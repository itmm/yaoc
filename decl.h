#line 333 "1_fn-gen.md"
#pragma once

#include <memory>
#include <string>

class Declaration {
	public:
		using Ptr = std::shared_ptr<Declaration>;
	private:
		std::string name_;
		std::weak_ptr<Declaration> parent_;

	protected:
		Declaration(std::string name, Declaration::Ptr parent):
			name_ { name }, parent_ { parent }
	       	{ }
	public:
		virtual ~Declaration() { }
		auto name() const { return name_; }
		Declaration::Ptr parent() const { return parent_.lock(); };
		virtual std::string mangle(std::string name);
		virtual bool has(std::string name) { return false; }
		virtual Declaration::Ptr lookup(std::string name);
		virtual void insert(Declaration::Ptr decl);
};

