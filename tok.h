#line 103 "1_fn-gen.md"
#pragma once

#include <string>

class Token {
	public:
		enum class Kind {
			end_of_input, identifier, integer_number,
			left_parenthesis, right_parenthesis, asterisk,
			period, colon, semicolon,
			BEGIN, END, MODULE, PROCEDURE, RETURN
		};
	private:
		Kind kind_;
		std::string representation_;
		int int_value_;
	public:
		Token(Kind kind, std::string representation, int int_value = 0):
			kind_ { kind }, representation_ { representation },
			int_value_ { int_value }
		{ }
		auto kind() const { return kind_; }
		auto representation() const { return representation_; }
		auto int_value() const { return int_value_; }
};
