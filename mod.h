#line 664 "1_fn-gen.md"
#pragma once

#include "scope.h"
#include "lex.h"

class Module: public Scoping {
	private:
		Module(std::string name, Declaration::Ptr parent): Scoping { name, parent } { }
	public:
		using Ptr = std::shared_ptr<Module>;
		static Module::Ptr create(std::string name, Declaration::Ptr parent);
		static Module::Ptr parse(Lexer &l, Declaration::Ptr parent);
		std::string mangle(std::string name) override;
};

