#line 422 "1_fn-gen.md"
#include "var.h"

#include "err.h"

Variable::Ptr Variable::create(
	std::string name, Declaration::Ptr parent, Type::Ptr type,
	bool exported, bool var
) {
	if (! parent) { throw Error { "no parent for variable" }; }
	auto result { Ptr { new Variable { name, parent, type, exported, var } } };
	parent->insert(result);
	return result;
}
