#line 544 "1_fn-gen.md"
#include "err.h"
#line 365
#pragma once

#include <memory>
#include <string>

class Declaration {
		std::string name_;
		std::weak_ptr<Declaration> parent_;
	public:
		using Ptr = std::shared_ptr<Declaration>;
	protected:
#line 549
		template<typename T> static T insert(
			T self, Declaration::Ptr parent
		) {
			if (! parent) { throw Error { "no parent" }; }
			parent->insert(self);
			return self;
		}
#line 376
		Declaration(std::string name, Declaration::Ptr parent):
			name_ { name }, parent_ { parent }
		{ }
	public:
		virtual ~Declaration() { }
		auto name() const { return name_; }
		static std::string name(Declaration::Ptr d);
		Declaration::Ptr parent() const { return parent_.lock(); };
		virtual std::string mangle(std::string name);
		virtual bool has(std::string name) { return false; }
		virtual Declaration::Ptr lookup(std::string name);
		virtual void insert(Declaration::Ptr decl);
};
#line 396
inline std::string Declaration::name(Declaration::Ptr d) {
	return d ? d->name() : "NIL";
}

