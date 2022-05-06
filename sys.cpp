#line 620 "1_fn-gen.md"
#include "sys.h"

#include "type.h"

Module::Ptr create_SYSTEM() {
	auto sys { Module::create("SYSTEM", nullptr) };
	Type::create("INTEGER", sys, "i32");
	return sys;
}
