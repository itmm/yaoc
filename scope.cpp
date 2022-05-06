#line 567 "1_fn-gen.md"
#include "scope.h"

#include "err.h"

Declaration::Ptr Scoping::lookup(std::string name) {
	auto got { entries_.find(name) };
	if (got != entries_.end()) { return got->second; }
	if (parent()) { return (parent())->lookup(name); }
	return nullptr;
}

void Scoping::insert(Declaration::Ptr decl) {
	auto res { entries_.insert({ decl->name(), decl }) };
	if (! res.second) {
		throw Error { "Element " + decl->name() + " inserted twice" };
	}
}
