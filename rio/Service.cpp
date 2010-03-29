/*
 * Service.cpp -- serving connections
 *
 * Copyright (c) 2007-2010, Frank Mertens
 *
 * See ../../LICENSE for the license.
 */

#include <termios.h> // termios
#include <unistd.h> // tcgetattr, tcsetattr
#include "Options.hpp"
#include "LineForwarder.hpp"
#include "BinaryForwarder.hpp"
#include "ProcessObserver.hpp"
#include "KillAfterTimeout.hpp"
#include "Service.hpp"

namespace rio
{

Service::Service()
	: StreamSocket(options()->address()),
	  connectLog_(new LogFile(options()->address(), LogFile::Connect)),
	  repeat_(options()->repeat()),
	  loop_(options()->loop()),
	  fileIndex_(0),
	  cancelEvent_(new Event),
	  ioCancelEvent_(new Event),
	  done_(false)
{
	closeOnExec();
	
	signalEvent(SIGINT)->pushBack(cancelEvent_);
	signalEvent(SIGTERM)->pushBack(cancelEvent_);
	
	class Cancel: public Action {
	public:
		Cancel(Ref<Service> service)
			: service_(service.get())
		{}
		virtual void run() {
			service_->finish();
		}
	private:
		Ref<Service, Owner> service_;
	};
	
	cancelEvent_->pushBack(ioCancelEvent_);
	cancelEvent_->pushBack(new Cancel(this));
}

void Service::init()
{
	if (options()->server())
		connectLog_->writeLine(address(), "Start listening");
}

void Service::idle()
{
	printTo(error(), "(%%) idle\n", options()->execName());
}

void Service::serve(Ref<StreamSocket> socket)
{
	connectLog_->writeLine(socket->address(), "Connection established");
	
	socket->closeOnExec();
	
	String entity;
	
	if (options()->files()->length() > 0)
	{
		entity = options()->files()->at(fileIndex_);
		fileIndex_ = (fileIndex_ + 1) % options()->files()->length();
	}
	
	if (options()->canon())
		canonSession(socket, entity);
	else
		binarySession(socket, entity);
	
	repeat_ -= (repeat_ > 0);
	
	connectLog_->writeLine(socket->address(), "Connection closed");
	
	if ((repeat_ == 0) && (!loop_))
		finish();
	
	cancelEvent_ = 0;
}

void Service::cleanup()
{
	if (options()->server())
		connectLog_->writeLine(address(), "Stop listening");
}

Ref<Process, Owner> Service::exec(String entity)
{
	if (!bool(options()->quiet()))
		printTo(error(), "(%%) Executing '%%'\n", options()->execName(), entity);
	
	Ref<ProcessFactory, Owner> factory = new ProcessFactory;
	factory->setType(Process::GroupLeader);
	{
		Ref<StringList, Owner> l = entity.split(" ");
		if (l->length() > 1) {
			entity = l->at(0);
			for (StringList::Index i = l->first() + 1; l->def(i); ++i)
				factory->options()->append(l->at(i));
		}
	}
	if ((entity.split("/"))->length() == 1) {
		String s = Path::lookup(Process::env("PATH").split(":"), entity);
		if (s != "") entity = s;
	}
	factory->setExecPath(entity);
	factory->setIoPolicy(Process::ForwardInput|Process::ForwardOutput|Process::ErrorToOutput);
	return factory->produce();
}

void Service::canonSession(Ref<StreamSocket> socket, String entity)
{
	Ref<SystemStream, Owner> recvSink;
	Ref<SystemStream, Owner> sendSource;
	
	Ref<Process, Owner> process;
	Ref<ProcessObserver, Owner> processObserver;
	Ref<Process, Owner> editor;
	
	Time t0 = now();
	Ref<LogFile, Owner> mergedLog = new LogFile(socket->address(), LogFile::Merged, t0);
	Ref<LogFile, Owner> recvLog = new LogFile(socket->address(), LogFile::Recv, t0, mergedLog);
	Ref<LogFile, Owner> sendLog = new LogFile(socket->address(), LogFile::Send, t0, mergedLog);
	
	if (options()->exec()) {
		if (entity != "") {
			process = exec(entity);
			processObserver = new ProcessObserver(process);
			processObserver->finishedEvent()->pushBack(ioCancelEvent_);
			processObserver->start();
			recvSink = process->rawInput();
			sendSource = process->rawOutput();
		}
	}
	else {
		if (entity != "") {
			Ref<File, Owner> file = new File(entity);
			file->open(File::Read);
			Ref<LineSource, Owner> lineSource = new LineSource(file);
			Ref<LineSink, Owner> lineSink = new LineSink(socket, options()->ioUnit(), options()->eol());
			while (lineSource->hasNext()) {
				String line = lineSource->next();
				if (sendLog)
					sendLog->writeLine(line);
				lineSink->writeLine(line);
			}
		}
		
		Ref<Pipe, Owner> recvPipe = new Pipe(Pipe::Output);
		Ref<Pipe, Owner> sendPipe = new Pipe(Pipe::Input);
		
		Ref<ProcessFactory, Owner> factory = new ProcessFactory;
		factory->setExecPath(options()->editor());
		factory->options()->append(Format("--fdr=%%") << recvPipe->childFd());
		factory->options()->append(Format("--fds=%%") << sendPipe->childFd());
		editor = factory->produce();
		
		recvPipe->open();
		sendPipe->open();
		
		recvSink = recvPipe;
		sendSource = sendPipe;
	}
	
	{
		Ref<LineForwarder, Owner> recvForwarder = new LineForwarder(socket, recvSink, options()->eol(), "\012", recvLog, ioCancelEvent_);
		Ref<LineForwarder, Owner> sendForwarder = new LineForwarder(sendSource, socket, "\012", options()->eol(), sendLog, ioCancelEvent_);
		recvForwarder->start();
		sendForwarder->start();
		sendForwarder->wait();
		recvForwarder->wait();
	}
	
	if (editor) {
		recvSink = 0;
		sendSource = 0;
		KillAfterTimeout kill(editor, now() + 3, SIGTERM);
		kill.start();
		int ret = editor->wait();
		kill.abort();
		kill.wait();
		#ifndef NDEBUG
		print("ret = %%\n", ret);
		#endif
	}
	if (process) {
		KillAfterTimeout kill(editor, now() + 3, SIGKILL);
		kill.start();
		processObserver->wait();
		kill.abort();
		kill.wait();
		#ifndef NDEBUG
		print("exitCode = %%\n", processObserver->exitCode());
		#endif
	}
}

void Service::binarySession(Ref<StreamSocket> socket, String entity)
{
	struct termios tioSaved;
	
	Ref<SystemStream, Owner> recvSink = rawOutput();
	Ref<SystemStream, Owner> sendSource = rawInput();
	
	if (sendSource->isTeletype()) {
		if (::tcgetattr(sendSource->fd(), &tioSaved) == -1)
			PONA_SYSTEM_EXCEPTION;
		struct termios tio = tioSaved;
		tio.c_lflag = tio.c_lflag & (~ICANON);
		if (::tcsetattr(sendSource->fd(), TCSANOW, &tio) == -1)
			PONA_SYSTEM_EXCEPTION;
	}
	
	Ref<Process, Owner> process;
	Ref<ProcessObserver, Owner> processObserver;
	
	Time t0 = now();
	Ref<LogFile, Owner> mergedLog = new LogFile(socket->address(), LogFile::Merged, t0);
	Ref<LogFile, Owner> recvLog = new LogFile(socket->address(), LogFile::Recv, t0, mergedLog);
	Ref<LogFile, Owner> sendLog = new LogFile(socket->address(), LogFile::Send, t0, mergedLog);
	
	if (entity != "") {
		if (options()->exec()) {
			process = exec(entity);
			processObserver = new ProcessObserver(process);
			processObserver->finishedEvent()->pushBack(ioCancelEvent_);
			processObserver->start();
			recvSink = process->rawInput();
			sendSource = process->rawOutput();
		}
		else {
			Ref<File, Owner> file = new File(entity);
			file->open(File::Read);
			Array<uint8_t> buf(options()->ioUnit());
			while (true) {
				int fill = file->readAvail(buf, buf.size());
				if (fill == 0) break;
				if (sendLog)
					sendLog->write(buf, fill);
				socket->write(buf, fill);
			}
		}
	}
	
	{
		Ref<BinaryForwarder, Owner> sendForwarder = new BinaryForwarder(sendSource, socket, sendLog, ioCancelEvent_);
		Ref<BinaryForwarder, Owner> recvForwarder = new BinaryForwarder(socket, recvSink, recvLog, ioCancelEvent_);
		sendForwarder->start();
		recvForwarder->start();
		recvForwarder->wait();
		sendForwarder->wait();
	}
	
	if (process) {
		try { process->kill(); } catch(AnyException& ex) {}
		processObserver->wait();
		#ifndef NDEBUG
		print("exitCode = %%\n", processObserver->exitCode());
		#endif
	}
	
	if (sendSource->isTeletype()) {
		if (::tcsetattr(sendSource->fd(), TCSAFLUSH, &tioSaved) == -1)
			PONA_SYSTEM_EXCEPTION;
	}
}

void Service::runServer(Time idleTimeout, int backlog)
{
	bind();
	listen(backlog);
	init();
	
	while (!done())
	{
		while (!done()) {
			if (readyAccept(idleTimeout)) break;
			if (done()) break;
			idle();
		}
		
		if (done_) break;
		
		Ref<StreamSocket, Owner> socket = accept();
		if (!done())
			serve(socket);
	}
	
	close();
	cleanup();
}

void Service::runClient(Time idleTimeout)
{
	connect();
	while (!established(idleTimeout))
		idle();
	serve(this);
	close();
	cleanup();
}

void Service::finish() { Guard<Mutex> guard(); done_ = true; }
bool Service::done() const { Guard<Mutex> guard(); return done_; }

} // namespace rio