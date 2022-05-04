#line 314 "1_fn-gen.md"
#include "decl.h"

std::string Declaration::mangle(std::string name) {
	auto result { name_ + "_" + name };
	return parent_ ? parent_->mangle(result) : result;
}
