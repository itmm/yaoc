#line 917 "1_fn-gen.md"
#include "sys.h"

#include "type.h"

Module::Ptr get_SYSTEM() {
	static Module::Ptr sys;
	if (sys) { return sys; }
	sys = Module::create("SYSTEM", nullptr);
	Type::create("INTEGER", sys, "i32");
	sys->insert(sys);
	return sys;
}
