#pragma once

// iron includes
#include "iron/token.h"

namespace iron
{

namespace ast
{

using Tokens = PtrRange<Token>;

struct Node
{
  enum class Type
  {
    block,
    float_lit,
    int_lit,
    func_defn,
    func_type,
    nspace, // namespace is a reserved word
    ret_stmnt,
    var_decl
  };

  Node() = delete;
  Node(Type t, Pos p) : _pos(p), _type(t) {}

  Pos pos() const { return _pos; }
  Type type() const { return _type; }

private :
  const Pos _pos;
  const Type _type;
};

struct Namespace : public Node
{
  Namespace(Pos p) : Node(Type::nspace, p) {}

  std::weak_ptr<Node> parent;
  std::string name;
  // TODO: Need a list of declarations
};

struct RetStmnt : public Node
{
  RetStmnt(Pos p) : Node(Type::ret_stmnt, p) {}

  // A null expr implies a void return;
  Shared<Node> expr;
};

struct Block : public Node
{
private :
  Darray<Shared<Node>> _stmnts;

public :
  Block(Pos p) : Node(Type::block, p) {}
  void addStmnt(Shared<Node> stmnt) { _stmnts.pushBack(stmnt); }
};

struct FuncType : public Node
{
  FuncType(Pos p) : Node(Type::func_type, p) {}

  // TODO: ins
  // TODO: outs
};

struct VarDecl : public Node
{
  VarDecl(Pos p, Ascii n) : Node(Type::var_decl, p), name(n) {}

  // an empty name is invalid
  Ascii name;
  // an empty type implies type deduction
  Ascii varType;
};

struct FuncDefn : public Node
{
  FuncDefn(Pos p) : Node(Type::func_defn, p) {}

  // empty name implies an anonymous function
  Ascii name;
  // null funcType is never valid
  Shared<FuncType> funcType;
  Shared<Block> block;
};

struct IntLit : public Node
{
  IntLit(Pos p) : Node(Type::int_lit, p) {}

  bool isNeg = false;
  Token intPart;
  Token suffix;
};

struct FloatLit : public Node
{
  FloatLit(Pos p) : Node(Type::int_lit, p) {}

  bool isNeg = false;
  Token intPart;
  Token floatPart;
  Token suffix;
};

Shared<Node> parseNumberLit(Tokens& tokens, Shared<Namespace> nspace)
{
  (void) nspace; // TODO: Scope literals?

  auto remainder = tokens;
  const Pos pos = remainder.front().pos;

  // TODO: Optional sign
  const bool isNeg = (remainder.front().type == Token::Type::minus);
  if (isNeg) { remainder.pop(); }

  // Mandatory number
  if (remainder.front().type != Token::Type::number)
  {
    return {};
  }
  auto& intPart = remainder.front();
  remainder.pop();

  // A period indicates a float literal
  const bool isFloat = (remainder.front().type == Token::Type::period);

  Shared<Node> numberLit;
  if (isFloat)
  {
    // TODO: Implement float parsing
    return {};
  }
  else
  {
    auto intLit = std::make_shared<IntLit>(pos);
    intLit->isNeg = isNeg;
    intLit->intPart = intPart;
    
    numberLit = intLit;
  }

  // TODO: Optional literal suffix

  tokens = remainder;
  return numberLit;
}

Shared<Node> parseLit(Tokens& tokens, Shared<Namespace> nspace)
{
  return parseNumberLit(tokens, nspace);
}

Shared<Node> parseExpr(Tokens& tokens, Shared<Namespace> nspace)
{
  return parseLit(tokens, nspace);
}

Shared<Node> parseRetStmnt(Tokens& tokens, Shared<Namespace> nspace)
{
  if (tokens.front().type != Token::Type::keyword_ret)
  {
    return {};
  }

  // At this point, it's safe to assume a return statement is here.
  auto retStmnt = std::make_shared<RetStmnt>(tokens.front().pos);
  tokens.pop();

  // Optionally parse an expression
  retStmnt->expr = parseExpr(tokens, nspace);

  if (tokens.front().type != Token::Type::semicolon)
  {
    errorln("Expected a semicolon to close out a return statement at ",
      retStmnt->pos());
  }
  tokens.pop();

  return retStmnt;
}

Shared<Node> parseStmnt(Tokens& tokens, Shared<Namespace> nspace)
{
  return parseRetStmnt(tokens, nspace);
}

// { <statement>* }
Shared<Block> parseBlock(Tokens& tokens, Shared<Namespace> nspace)
{
  if (tokens.front().type != Token::Type::left_brace)
  {
    return {};
  }

  // At this point, it's safe to assume a block is here
  auto block = std::make_shared<Block>(tokens.front().pos);
  tokens.pop();

  // TODO: While not }, parse statement
  while (true)
  {
    auto stmnt = parseStmnt(tokens, nspace);
    if (stmnt)
    {
      block->addStmnt(stmnt);
    }
    else if (tokens.front().type == Token::Type::right_brace)
    {
      tokens.pop();
      return block;
    }
    else
    {
      errorln("Expected a right curly brace or a statement at ",
        block->pos());
      return {};
    }
  }

  return {};
}

Shared<VarDecl> parseVarDecl(Tokens& tokens, Shared<Namespace> nspace)
{
  (void) nspace;

  auto remainder = tokens;

  if (remainder.front().type != Token::Type::identifier)
  {
    return {};
  }
  auto name = remainder.front();
  remainder.pop();

  if (remainder.front().type != Token::Type::colon)
  {
    return {};
  }

  // At this point, it's safe to assume that this is a variable declaration.
  auto varDecl = std::make_shared<VarDecl>(name.pos, name.value);
  const auto colonPos = remainder.front().pos;
  remainder.pop();

  if (remainder.front().type != Token::Type::identifier)
  {
    errorln("Expected a type for the variable declared at ", colonPos);
    return {};
  }
  varDecl->varType = remainder.front().value;
  remainder.pop();

  tokens = remainder;
  return varDecl;
}

Shared<FuncType> parseFuncType(Tokens& tokens, Shared<Namespace> nspace)
{
  (void) nspace;

  auto remainder = tokens;

  // TODO: Generalize ins to be a parentheses-grouped list of variable
  // declarations.
  if (remainder[0].type != Token::Type::left_paren &&
      remainder[1].type != Token::Type::right_paren)
  {
    errorln("Expected a parameter list at ", remainder.front().pos);
    return {};
  }
  remainder.pop(2);

  if (remainder.front().type != Token::Type::map)
  {
    errorln("Expected a '=>' at ", remainder.front().pos);
    return {};
  }

  // At this point, it's safe to assume that this is a function type
  auto funcType = std::make_shared<FuncType>(remainder.front().pos);
  remainder.pop();

  // Start parsing the return types
  if (remainder[0].type != Token::Type::left_paren)
  {
    errorln("Expected a return argument list at ", remainder.front().pos);
    return {};
  }
  remainder.pop();

  // TODO: while not ')', parse comma-separated variable declarations
  bool expectComma = false;
  while (remainder.front().type != Token::Type::right_paren)
  {
    if (expectComma)
    {
      if (remainder.front().type != Token::Type::comma)
      {
        errorln("Expected a comma as part of a parameter list at ",
          remainder.front().pos);
        return {};
      }

      remainder.pop(); // Pop the comma
    }

    // else expect a variable declaration
    auto varDecl = parseVarDecl(remainder, nspace);
    if (!varDecl)
    {
      errorln("Expected a variable declaration as part of a parameter "
          "list at ", remainder.front().pos);
      return {};
    }

    expectComma = true;
  }
  remainder.pop(); // Pops the right parenthesis

  tokens = remainder;
  return funcType;
}

// <fn> <identifier>? (':' <ins> ('=' '>' <outs>)? )? <block>
Shared<FuncDefn> parseFuncDefn(Tokens& tokens, Shared<Namespace> nspace)
{
  if (tokens.front().type != Token::Type::keyword_fn)
  {
    return {};
  }

  // At this point, it's safe to assume a function definition is here
  auto funcDefn = std::make_shared<FuncDefn>(tokens.front().pos);
  tokens.pop();

  // Look for the optional name of the function
  if (tokens.front().type == Token::Type::identifier)
  {
    funcDefn->name = tokens.front().value;
    tokens.pop();
  }

  // Look for the (optional) function type
  if (tokens.front().type == Token::Type::colon)
  {
    auto colonPos = tokens.front().pos;
    tokens.pop();
    funcDefn->funcType = parseFuncType(tokens, nspace);
    if (!funcDefn->funcType)
    {
      errorln("Expected a function type following the colon at ",
        colonPos);
      return {};
    }
  }

  funcDefn->block = parseBlock(tokens, nspace);
  if (!funcDefn->block)
  {
    errorln("Expected a function block following the function signature at ",
      funcDefn->pos());
    return {};
  }

  return funcDefn;
}

// Function Declaration or
// Struct Declaration or
// Class Declaration or
// Alias Declaration or
// Namespace Declaration
Shared<Node> parseDecl(Tokens& tokens, Shared<Namespace> nspace)
{
  return parseFuncDefn(tokens, nspace);
}

Shared<Node> parse(Tokens tokens)
{
  auto global = std::make_shared<Namespace>(Pos{0,0});
  global->name = "_";

  return parseDecl(tokens, global);
}

} // namespace ast

} // namespace iron

