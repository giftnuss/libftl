
#include "TapSuite.hpp"

namespace ftl 
{
	 
TapSuite::~TapSuite()
{

}

void
TapSuite::ok(bool b, const String& s)
{
  print("%%",b ? "ok" : "not ok");
  if(s.length()) {
    print(" -- %%", s);
  }
  print("\n");
}

void
TapSuite::is(const String& have,
						 const String& want,
						 const String& doc)
{
	 String t = doc;
	 bool result = have == want;
	 if(!result) 
		 {
				t = Format("%%\nWANT: %%\nHAVE: %%")
									 << t << want << have; 
		 }
	 ok(result,t);
}

} // end namespace ftl
