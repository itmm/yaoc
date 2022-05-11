#line 112 "1_fn-gen.md"
#pragma once

#include <stdexcept>
#include <sstream>
#include <string>

class Error: public std::exception {
		std::string what_;
	public:
		Error(std::string what): what_ { what } { }
		const char *what() const noexcept override {
			return what_.c_str();
		}
};

inline void err_with_stream(std::ostringstream &out) {
	throw Error { out.str() };
}

template<typename ARG, typename... ARGS> inline void err_with_stream(
	std::ostringstream &out, ARG arg, ARGS... rest
) {
	out << arg;
	err_with_stream(out, rest...);
}

template<typename... ARGS> inline void err(ARGS... args) {
	std::ostringstream out;
	err_with_stream(out, args...);
}
#line 149
inline std::string quote(std::string s) { return "'" + s + "'"; }
