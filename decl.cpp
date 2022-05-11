#line 445 "1_fn-gen.md"
#include "decl.h"

#include "err.h"

std::string Declaration::mangle(std::string name) {
	auto result { name_ + "_" + name };
	auto p { parent() };
	return p ? p->mangle(result) : result;
}

Declaration::Ptr Declaration::lookup(std::string name) {
	err("cannot lookup ", quote(name));
	return nullptr;
}

void Declaration::insert(Declaration::Ptr decl) {
	err("cannot insert ", quote(decl));
}
