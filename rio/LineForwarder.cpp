/*
 * LineForwarder.cpp -- canonical I/O transfer thread
 *
 * Copyright (c) 2007-2010, Frank Mertens
 *
 * See ../../LICENSE for the license.
 */

#include "LineForwarder.hpp"

namespace rio
{

LineForwarder::LineForwarder(
	Ref<SystemStream> source,
	Ref<SystemStream> sink,
	String sourceEol,
	String sinkEol,
	Ref<LogFile> recvLog,
	Ref<Event> cancelEvent
)
	: source_(source.get()),
	  sink_(sink.get()),
	  lineSource_(new LineSource(source, options()->ioUnit(), sourceEol)),
	  lineSink_(new LineSink(sink, options()->ioUnit(), sinkEol)),
	  recvLog_(recvLog.get()),
	  cancelEvent_(cancelEvent.get()),
	  done_(false), failed_(false)
{
	class AbortAction: public Action {
	public:
		AbortAction(Ref<LineForwarder, Owner> forwarder)
			: forwarder_(forwarder)
		{}
		virtual void run() {
			forwarder_->finish();
		}
	private:
		Ref<LineForwarder, Owner> forwarder_;
	};
	
	finishAction_ = new AbortAction(this);
	cancelEvent_->pushFront(finishAction_);
}

void LineForwarder::finish() { Guard<Mutex> guard(&mutex_); done_ = true; }
bool LineForwarder::finishUp() { Guard<Mutex> guard(&mutex_); return done_; }

void LineForwarder::run()
{
	failed_ = false;
	try {
		while (!finishUp())
		{
			if (source_->readyRead(1))
			{
				bool eoi = !lineSource_->readAvail();
				while (lineSource_->cachedLines() > 0) {
					String line = lineSource_->readLine();
					if (recvLog_) recvLog_->writeLine(line);
					lineSink_->writeLine(line);
				}
				if (eoi || finishUp()) break;
			}
		}
	}
	catch (AnyException& ex) {
		if (!finishUp()) {
			printTo(error(), "(%%) LineForwarder: %%\n", options()->execName(), ex.what());
			failed_ = true;
		}
	}
	cancelEvent_->remove(finishAction_);
	cancelEvent_->run();
	finishAction_ = 0;
}

bool LineForwarder::failed() const { return failed_; }

} // namespace rio