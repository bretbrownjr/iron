#pragma once

// iron includes
#include "iron/ast.h"
#include "iron/token.h"

namespace iron
{

namespace ast
{

using Tokens = PtrRange<Token>;

Shared<Typename> parseTypename(Tokens& tokens, Shared<Namespace> nspace)
{
  (void) nspace;

  if (tokens.front().type != Token::Type::identifier)
  {
    return {};
  }
  auto tname = std::make_shared<Typename>(tokens.front().pos);
  tokens.pop();

  return tname;
}

Shared<VarDecl> parseVarDecl(Tokens& tokens, Shared<Namespace> nspace);

Shared<FuncType> parseFuncType(Tokens& tokens, Shared<Namespace> nspace)
{
  (void) nspace;

  auto remainder = tokens;

  // TODO: Generalize ins to be a parentheses-grouped list of variable
  // declarations.
  if (remainder[0].type != Token::Type::left_paren &&
      remainder[1].type != Token::Type::right_paren)
  {
    return {};
  }
  remainder.pop(2);

  if (remainder.front().type != Token::Type::map)
  {
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
    funcType->outs.pushBack(varDecl);

    expectComma = true;
  }
  remainder.pop(); // Pops the right parenthesis

  tokens = remainder;
  return funcType;
}

Shared<Type> parseType(Tokens& tokens, Shared<Namespace> nspace)
{
  {
    auto fnType = parseFuncType(tokens, nspace);
    if (fnType) { return fnType; }
  }

  {
    auto tname = parseTypename(tokens, nspace);
    if (tname) { return tname; }
  }

  return {};
}

Shared<NumLit> parseNumberLit(Tokens& tokens, Shared<Namespace> nspace)
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
  // It is now safe to assume that this is some sort of number literal
  auto& intPart = remainder.front().value;
  remainder.pop();

  // A period indicates a float literal
  const bool isFloat = (remainder.front().type == Token::Type::period);

  Shared<NumLit> numberLit;
  if (isFloat)
  {
    remainder.pop();
    auto floatLit = std::make_shared<FloatLit>(pos);

    // Optional number following the decimal point
    if (remainder.front().type == Token::Type::number)
    {
      floatLit->floatPart = remainder.front().value;
      remainder.pop();
    }
  }
  else
  {
    numberLit = std::make_shared<IntLit>(pos);
  }
  numberLit->isNeg = isNeg;
  numberLit->intPart = intPart;

  if (remainder.front().type == Token::Type::colon)
  {
    // It is now safe to assume that this number literal has a suffix
    const auto colonPos = remainder.front().pos;
    remainder.pop();

    numberLit->type = parseType(remainder, nspace);
    if (!numberLit)
    {
      errorln("Expected a type following the colon at ", colonPos);
      return {};
    }
  }

  tokens = remainder;
  return numberLit;
}

Shared<Node> parseLit(Tokens& tokens, Shared<Namespace> nspace)
{
  return parseNumberLit(tokens, nspace);
}

Shared<FuncCall> parseFuncCall(Tokens& tokens, Shared<Namespace> nspace)
{
  (void) nspace;
  auto remainder = tokens;

  if (remainder.front().type != Token::Type::identifier) { return {}; }

  auto fnCall = std::make_shared<FuncCall>(remainder.front().pos);
  fnCall->name = remainder.front().value;
  remainder.pop();

  if (remainder[0].type != Token::Type::left_paren &&
      remainder[1].type != Token::Type::right_paren)
  {
    // Not a function call; probably an lvalue.
    return {};
  }
  remainder.pop(2);

  tokens = remainder;
  return fnCall;
}

Shared<Node> parseRvalue(Tokens& tokens, Shared<Namespace> nspace)
{
  return parseFuncCall(tokens, nspace);
}

Shared<Lvalue> parseLvalue(Tokens& tokens, Shared<Namespace> nspace)
{
  (void) nspace;

  if (tokens.front().type != Token::Type::identifier) { return {}; }

  auto var = std::make_shared<Lvalue>(tokens.front().pos, tokens.front().value);
  tokens.pop();

  return var;
}

Shared<Node> parseExpr(Tokens& tokens, Shared<Namespace> nspace);

Shared<Node> parseParenExpr(Tokens& tokens, Shared<Namespace> nspace)
{
  if (tokens.front().type != Token::Type::left_paren) { return {}; }

  auto remainder = tokens;
  remainder.pop(); // pop the left parenthesis
  auto expr = parseExpr(remainder, nspace);

  if (remainder.front().type != Token::Type::right_paren)
  {
    errorln("Expected a ')' to match the '(' at ",
      tokens.front().pos);
    return {};
  }
  remainder.pop(); // pop the right parenthesis

  tokens = remainder;
  return expr;
}

Shared<Node> parsePrimaryExpr(Tokens& tokens, Shared<Namespace> nspace)
{
  {
    auto expr = parseParenExpr(tokens, nspace);
    if (expr) { return expr; }
  }

  {
    auto expr = parseLit(tokens, nspace);
    if (expr) { return expr; }
  }

  {
    auto expr = parseRvalue(tokens, nspace);
    if (expr) { return expr; }
  }

  {
    auto expr = parseLvalue(tokens, nspace);
    if (expr) { return expr; }
  }

  return {};
}

Shared<Node> parseMultExpr(Tokens& tokens, Shared<Namespace> nspace)
{
  auto remainder = tokens;

  auto lhs = parsePrimaryExpr(remainder, nspace);
  if (!lhs) { return {}; }

  const auto type = remainder.front().type;
  if (type != Token::Type::asterisk &&
      type != Token::Type::fwd_slash)
  {
    tokens = remainder;
    return lhs;
  }

  // At this point it's safe to assume that this is an add operation
  auto multExpr = std::make_shared<BinExpr>(remainder.front().pos, lhs, type);
  remainder.pop(); // Remove the add operator

  multExpr->rhs = parseExpr(remainder, nspace);
  if (!multExpr->rhs)
  {
    errorln("Expected an expression following the operator at ",
      multExpr->pos());
    return {};
  }

  tokens = remainder;
  return multExpr;
}

Shared<Node> parseAddExpr(Tokens& tokens, Shared<Namespace> nspace)
{
  auto remainder = tokens;

  auto lhs = parseMultExpr(remainder, nspace);
  if (!lhs) { return {}; }

  const auto type = remainder.front().type;
  if (type != Token::Type::plus &&
      type != Token::Type::minus)
  {
    tokens = remainder;
    return lhs;
  }

  // At this point it's safe to assume that this is an add operation
  auto addExpr = std::make_shared<BinExpr>(remainder.front().pos, lhs, type);
  remainder.pop(); // Remove the add operator

  addExpr->rhs = parseMultExpr(remainder, nspace);
  if (!addExpr->rhs)
  {
    errorln("Expected an expression following the operator at ",
      addExpr->pos());
    return {};
  }

  tokens = remainder;
  return addExpr;
}

Shared<Node> parseExpr(Tokens& tokens, Shared<Namespace> nspace)
{
  {
    auto addExpr = parseAddExpr(tokens, nspace);
    if (addExpr) { return addExpr; }
  }

  return {};
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
    return {};
  }
  tokens.pop();

  return retStmnt;
}

Shared<VarDecl> parseVarDecl(Tokens& tokens, Shared<Namespace> nspace)
{
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
  remainder.pop(); // pop the colon token

  varDecl->type = parseType(remainder, nspace);

  tokens = remainder;
  return varDecl;
}

Shared<Initializer> parseInitializer(Tokens& tokens, Shared<Namespace> nspace)
{
  auto remainder = tokens;

  if (remainder.front().type != Token::Type::left_brace) { return {}; }

  auto initializer = std::make_shared<Initializer>(remainder.front().pos);
  remainder.pop();

  bool expectComma = false;
  while (remainder.front().type != Token::Type::right_brace)
  {
    if (expectComma)
    {
      errorln("Failed to parse an initializer list. Expected a comma at ",
        remainder.front().pos);
      return {};
    }

    auto expr = parseExpr(remainder, nspace);
    if (!expr)
    {
      errorln("Expected an expression as part of the initializer list at ",
        initializer->pos());
      return {};
    }
    initializer->addExpr(expr);
    expectComma = true;
  }
  remainder.pop(); // pop the right curly brace

  tokens = remainder;
  return initializer;
}

Shared<VarDeclStmnt> parseVarDeclStmnt(Tokens& tokens, Shared<Namespace> nspace)
{
  auto remainder = tokens;
  auto decl = parseVarDecl(remainder, nspace);
  if (!decl) { return {}; }

  // At this point, it's safe to assume a variable declaration statement is here.
  auto varDecl = std::make_shared<VarDeclStmnt>(decl->pos());
  varDecl->decl = decl;

  // Optional initializer
  varDecl->initializer = parseInitializer(remainder, nspace);

  if (remainder.front().type != Token::Type::semicolon)
  {
    errorln("Expected a semicolon to terminate the variable declaration at ",
      varDecl->pos());
  }
  remainder.pop(); // pop the semicolon
  
  tokens = remainder;
  return varDecl;
}

Shared<Node> parseExprStmnt(Tokens& tokens, Shared<Namespace> nspace)
{
  auto remainder = tokens;

  auto expr = parseExpr(remainder, nspace);
  if (!expr) { return {}; }

  if (remainder.front().type != Token::Type::semicolon)
  {
    return {};
  }
  remainder.pop(); // pop the semicolon token

  tokens = remainder;
  return std::make_shared<ExprStmnt>(expr);
}

Shared<Node> parseStmnt(Tokens& tokens, Shared<Namespace> nspace)
{
  {
    auto retStmnt = parseRetStmnt(tokens, nspace);
    if (retStmnt) { return retStmnt; }
  }

  {
    auto varDecl = parseVarDeclStmnt(tokens, nspace);
    if (varDecl) { return varDecl; }
  }

  {
    auto exprStmnt = parseExprStmnt(tokens, nspace);
    if (exprStmnt) { return exprStmnt; }
  }

  return {};
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
    if (tokens.front().type == Token::Type::right_brace)
    {
      tokens.pop();
      return block;
    }

    auto stmnt = parseStmnt(tokens, nspace);
    if (stmnt)
    {
      block->addStmnt(stmnt);
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

// <fn> <identifier>? (':' <ins> ('=' '>' <outs>)? )? <block>
Shared<FuncDefn> parseFuncDefn(Tokens& tokens, Shared<Namespace> nspace)
{
  if (tokens.front().type != Token::Type::keyword_fn)
  {
    return {};
  }

  // At this point, it's safe to assume a function definition is here
  auto funcDefn = std::make_shared<FuncDefn>(tokens.front().pos, nspace);
  tokens.pop();

  // Look for the optional name of the function
  if (tokens.front().type == Token::Type::identifier)
  {
    std::string name
    {
      &tokens.front().value.front(),
      tokens.front().value.size()
    };
    funcDefn->name = name;
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
  else
  {
    // Use a () => () function type by default
    funcDefn->funcType = std::make_shared<FuncType>(funcDefn->pos());
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

  while (!tokens.isEmpty())
  {
    auto decl = parseDecl(tokens, global);
    if (!decl)
    {
      errorln("Expected a declaration at ", tokens.front().pos);
      return {};
    }

    // Add the decl to the namespace
    global->decls.pushBack(decl);
  }

  return global;
}

} // namespace ast

} // namespace iron

