#line 483 "1_fn-gen.md"
#pragma once

#include "decl.h"
#include "lex.h"

Declaration::Ptr parse_qualified_ident(Lexer &l, Declaration::Ptr scope);
