/*
 * ExitEvent.cpp -- call event handlers on process termination
 *
 * Copyright (c) 2007-2010, Frank Mertens
 *
 * See ../LICENSE for the license.
 */

#include "ExitEvent.hpp"

namespace ftl
{

ExitEvent::ExitEvent()
	: pid_(Process::currentProcessId())
{}

ExitEvent::~ExitEvent()
{
	if (Process::currentProcessId() == pid_) run();
}

} // namespace ftl