/*
 * ArrayPolicy.hpp -- array policies
 *
 * Copyright (c) 2007-2010, Frank Mertens
 *
 * See ../LICENSE for the license.
 */
#ifndef PONA_ARRAYPOLICY_HPP
#define PONA_ARRAYPOLICY_HPP

#include "strings.hpp"

namespace pona
{

template<class T>
class EmptyArray {
public:
	inline static T* empty() { return 0; }
};

template<>
class EmptyArray<char> {
public:
	inline static char* empty() { return const_cast<char*>(""); }
};

template<>
class EmptyArray<const char> {
public:
	inline static const char* empty() { return ""; }
};

template<class T>
class DeepCopyArray {
public:
	inline static T* empty() {
		return EmptyArray<T>::empty();
	}
	inline static void initEmpty(T*& data_, int& size_) {
		size_ = 0;
		data_ = empty();
	}
	inline static void alloc(T*& data_, int& size_, int size) {
		if (size > 0) {
			size_ = size;
			data_ = new T[size];
		}
		else {
			size_ = 0;
			data_ = empty();
		}
	}
	inline static void free(T*& data_, int& size_) {
		if (size_ > 0) {
			delete[] data_;
			size_ = 0;
			data_ = empty();
		}
	}
	inline static void assign(T*& data_, int& size_, const T* data, int size) {
		if (size_ != size) {
			free(data_, size_);
			alloc(data_, size_, size);
		}
		pona::memcpy(data_, data, size);
	}
	inline static void clear(T*& data_, int& size_, T zero) {
		pona::memset(data_, zero, size_);
	}
};

template<class T>
class DeepCopyZeroTerminatedArray {
public:
	inline static T* empty() {
		return EmptyArray<T>::empty();
	}
	inline static void initEmpty(T*& data_, int& size_) {
		size_ = 0;
		data_ = empty();
	}
	inline static void alloc(T*& data_, int& size_, int size) {
		if (size > 0) {
			size_ = size;
			data_ = new T[size + 1];
			data_[size] = 0;
		}
		else {
			size_ = 0;
			data_ = empty();
		}
	}
	inline static void free(T*& data_, int& size_) {
		if (size_ > 0) {
			delete[] data_;
			size_ = 0;
			data_ = empty();
		}
	}
	inline static void assign(T*& data_, int& size_, const T* data, int size) {
		if (size_ != size) {
			free(data_, size_);
			alloc(data_, size_, size);
		}
		pona::memcpy(data_, data, size);
	}
	inline static void clear(T*& data_, int& size_, T zero) {
		pona::memset(data_, zero, size_);
	}
};

template<class T>
class ShallowCopyArray {
public:
	inline static T* empty() {
		return EmptyArray<T>::empty();
	}
	inline static void initEmpty(T*& data_, int& size_) {
		size_ = 0;
		data_ = empty();
	}
	inline static void free(T*& data_, int& size_) {
		size_ = 0;
		data_ = empty();
	}
	inline static void assign(T*& data_, int& size_, const T* data, int size) {
		size_ = size;
		data_ = const_cast<T*>(data);
	}
};

template<class T>
class ArrayPolicy: public DeepCopyArray<T> {};

template<>
class ArrayPolicy<char>: public DeepCopyZeroTerminatedArray<char> {};

template<class T>
class ArrayPolicy<const T>: public ShallowCopyArray<const T> {};

} // namespace pona

#endif // PONA_ARRAYPOLICY_HPP