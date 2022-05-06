#line 396 "1_fn-gen.md"
#pragma once

#include "decl.h"
#include "type.h"

#include <vector>

class Variable: public Declaration {
		Type::Ptr type_;
		bool exported_;
		bool var_;
		Variable(
			std::string name, Declaration::Ptr parent,
		       	Type::Ptr type, bool exported, bool var
		): Declaration { name, parent }, type_ { type }, exported_ { exported }, var_ { var } { }
	public:
		static Ptr create(
			std::string name, Declaration::Ptr parent,
			Type::Ptr type, bool exported, bool var
		);
};
