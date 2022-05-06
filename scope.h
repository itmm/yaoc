#line 545 "1_fn-gen.md"
#pragma once

#include "decl.h"

#include <map>

class Scoping: public Declaration {
		std::map<std::string, Declaration::Ptr> entries_;
	protected:
		Scoping(std::string name, Declaration::Ptr parent):
			Declaration { name, parent }
	       	{ }
	public:
		Declaration::Ptr lookup(std::string name) override;
		void insert(Declaration::Ptr decl) override;
};
