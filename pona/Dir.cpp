/*
 * Dir.cpp -- file directory I/O
 *
 * Copyright (c) 2007-2009, Frank Mertens
 *
 * See ../LICENSE for the license.
 */

#include <sys/stat.h> // mkdir
#include <sys/types.h> // mode_t

#include "Path.hpp"
#include "File.hpp"
#include "Dir.hpp"

namespace pona
{

Dir::Dir(String path)
	: path_(Path(path).makeAbsolute()),
	  dir_(0)
{}

Dir::~Dir()
{
	if (isOpen()) close();
}

String Dir::path() const { return path_; }

bool Dir::access(int flags) { return File(path_).access(flags); }
bool Dir::exists() const { return File(path_).exists(); }

void Dir::create(int mode)
{
	if (::mkdir(path_->utf8(), mode) == -1)
		PONA_SYSTEM_EXCEPTION;
}

void Dir::unlink()
{
	if (::rmdir(path_->utf8()))
		PONA_SYSTEM_EXCEPTION;
}

void Dir::open()
{
	if (dir_) return;
	dir_ = ::opendir(path_->utf8());
	if (!dir_)
		PONA_SYSTEM_EXCEPTION;
}

void Dir::close()
{
	if (!dir_) return;
	if (::closedir(dir_) == -1)
		PONA_SYSTEM_EXCEPTION;
	dir_ = 0;
}

bool Dir::read(Ref<DirEntry> entry)
{
	if (!isOpen()) open();
	struct dirent* buf = entry;
	struct dirent* result;
	int errorCode = ::readdir_r(dir_, buf, &result);
	if (errorCode)
		throw SystemException(__FILE__, __LINE__, "SystemException", pona::strcat("readdir_r() failed: error code = ", pona::intToStr(errorCode)), errorCode);
	
	entry->path_ = Path(entry->d_name).makeAbsoluteRelativeTo(path_);
	return result;
}

bool Dir::isOpen() const { return dir_; }

} // namespace pona