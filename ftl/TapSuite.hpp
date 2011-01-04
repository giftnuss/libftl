/*
 * AvlTree.hpp -- binary tree balancing logic
 *
 * Copyright (c) 2011, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef FTL_TAPSUITE_HPP
#define FTL_TAPSUITE_HPP

#include "strings"
#include "streams"

namespace ftl {

class TapSuite
{
public:
  virtual ~TapSuite();
  virtual void init() = 0;
  virtual int testCount() = 0;
  virtual void testRun() = 0;
  void ok(bool, const String& = "");
	void is(const String& have, 
					const String& want,
					const String& doc = "");
};

}

#endif
