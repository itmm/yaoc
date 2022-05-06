#line 362 "1_fn-gen.md"
#include "decl.h"

#include "err.h"

std::string Declaration::mangle(std::string name) {
	auto result { name_ + "_" + name };
	return parent_ ? parent_->mangle(result) : result;
}

Declaration::Ptr Declaration::lookup(std::string name) {
	throw Error { "cannot lookup " + name };
}

void Declaration::insert(Declaration::Ptr decl) {
	throw Error { "cannot insert " + (decl ? decl->name() : std::string { "NIL" }) };
}
