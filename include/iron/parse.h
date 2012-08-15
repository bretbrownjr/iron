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
    func_defn,
    nspace // namespace is a reserved word
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

struct Block : public Node
{
  Block(Pos p) : Node(Type::block, p) {}

  // TODO: Need a list of statements
};

struct FuncDefn : public Node
{
  FuncDefn(Pos p) : Node(Type::func_defn, p) {}

  // empty name implies an anonymous function
  Ascii name;
  Shared<Block> block;
};

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
  (void) nspace;

  if (tokens.front().type != Token::Type::right_brace)
  {
    errorln("Expected a right curly brace to close out a block at ",
      block->pos());
    return {};
  }
  tokens.pop();

  return block;
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

