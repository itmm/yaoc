#line 405 "1_fn-gen.md"
#include "decl.h"

#include "err.h"

std::string Declaration::mangle(std::string name) {
	auto result { name_ + "_" + name };
	auto p { parent() };
	return p ? p->mangle(result) : result;
}

Declaration::Ptr Declaration::lookup(std::string name) {
	throw Error { "cannot lookup " + name };
}

void Declaration::insert(Declaration::Ptr decl) {
	throw Error { "cannot insert " + Declaration::name(decl) };
}
