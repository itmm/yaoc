#line 290 "1_fn-gen.md"
#pragma once

#include <memory>
#include <string>

class Declaration {
	public:
		using Ptr = std::shared_ptr<Declaration>;
	private:
		std::string name_;
		Declaration::Ptr parent_;

	protected:
		Declaration(std::string name, Declaration::Ptr parent):
			name_ { name }, parent_ { parent } { }
	public:
		virtual ~Declaration() { }
		auto parent() const { return parent_; };
		std::string mangle(std::string name);
};
