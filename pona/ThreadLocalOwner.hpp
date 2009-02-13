/****************************************************************************
**
** This file is part of libPONA - The Portable Network Abstractions Library.
**
** Copyright (C) 2007-2009  Frank Mertens
**
** This file is part of a free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License as published
** by the Free Software Foundation, either version 3 of the License,
** or (at your option) any later version.
**
** The library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this libary.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef PONA_THREADLOCALOWNER_HPP
#define PONA_THREADLOCALOWNER_HPP

#ifdef PONA_POSIX

#include <pthread.h>
#include "Exception.hpp"

namespace pona
{

template<class T>
class ThreadLocalOwner
{
public:
	ThreadLocalOwner()
	{
		int ret = ::pthread_key_create(&key_, 0);
		if (ret != 0)
			PONA_THROW(PosixException, strerror(ret));
	}
	
	~ThreadLocalOwner()
	{
		int ret = ::pthread_key_delete(key_);
		if (ret != 0)
			PONA_THROW(PosixException, strerror(ret));
	}
	
	inline T* get() const { return reinterpret_cast<T*>(::pthread_getspecific(key_)); }
	
	inline void set(T* b)
	{
		T* a = get();
		if (a != b)
		{
			if (a)
			{
				a->decRefCount();
				if (a->refCount() == 0)
					a->destroy();
			}
			
			::pthread_setspecific(key_, b);
			
			if (b)
				b->incRefCount();
		}
	}
	
private:
	pthread_key_t key_;
};

} // namespace pona

#else // PONA_WINDOWS

#include <Windows.h>
#include "Exception.hpp"

namespace pona
{

template<class T>
class ThreadLocalOwner
{
public:
	ThreadLocalOwner()
	{
		key_ = TlsAlloc();
		if (key_ == TLS_OUT_OF_INDEXES)
			PONA_WINDOWS_EXCEPTION;
	}
	
	~ThreadLocalOwner()
	{
		if (!TlsFree(key_))
			PONA_WINDOWS_EXCEPTION;
	}
	
	inline T* get() const { return reinterpret_cast<T*>(TlsGetValue(key_)); }
	
	inline void set(T* b)
	{
		T* a = get();
		if (a != b)
		{
			if (a)
			{
				a->decRefCount();
				if (a->refCount() == 0)
					a->destroy();
			}
			
			TlsSetValue(key_, b);
			
			if (b)
				b->incRefCount();
		}
	}
	
private:
	DWORD key_;
};

} // namespace pona

#endif // platform switch

#endif // PONA_THREADLOCALOWNER_HPP
