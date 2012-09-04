#pragma once

// standard includes
#include <memory>
#include <sstream>

// iron includes
#include "iron/darray.h"
#include "iron/token.h"

namespace iron
{

namespace ast
{

template<typename Ttype>
using Shared = std::shared_ptr<Ttype>;
template<typename Ttype>
using Weak = std::weak_ptr<Ttype>;

struct Node
{
  enum class Kind
  {
    add_expr,
    block,
    expr_stmnt,
    float_lit,
    int_lit,
    initializer,
    func_call,
    func_defn,
    func_type,
    nspace, // namespace is a reserved word
    ret_stmnt,
    rvalue,
    tname, // typename is a reserved word
    var_decl,
    var_decl_stmnt
  };

  Node() = delete;
  Node(Kind k, Pos p) : _pos(p), _kind(k) {}

  Kind kind() const { return _kind; }
  Pos pos() const { return _pos; }

private :
  const Pos _pos;
  const Kind _kind;
};

struct BinExpr : public Node
{
  BinExpr(Pos p, Shared<Node> l, Token::Type t) :
      Node(Kind::add_expr, p), lhs(l), type(t) {}

  // lhs must not be null
  Shared<Node> lhs;
  // rhs must not be null
  Shared<Node> rhs;
  // type must be one of: 
  //     Token::Type::plus
  //     Token::Type::minus
  //     Token::Type::asterisk
  //     Token::Type::fwd_slash
  Token::Type type;
};

struct Block : public Node
{
private :
  Darray<Shared<Node>> _stmnts;

public :
  Block(Pos p) : Node(Kind::block, p) {}
  void addStmnt(Shared<Node> stmnt) { _stmnts.pushBack(stmnt); }
  bool isEmpty() const { return _stmnts.isEmpty(); }
  auto stmnts() const -> decltype(_stmnts.all()) { return _stmnts.all(); }
  auto stmnts() -> decltype(_stmnts.all()) { return _stmnts.all(); }
};

struct ExprStmnt : public Node
{
  ExprStmnt(Shared<Node> e) : Node(Kind::expr_stmnt, e->pos()), expr(e) {}

  Shared<Node> expr;
};

struct Type : public Node
{

protected :
  /// This constructor is protected because one is not supposed to construct a
  /// type except through child classes.
  Type(Kind k, Pos p) : Node(k, p) {}
};

struct NumLit : public Node
{
  NumLit() = delete;

  bool isNeg = false;
  Ascii intPart;
  Shared<Type> type;

protected :
  /// This constructor is protected because one is not supposed to construct a
  /// number literal except through child classes.
  NumLit(Kind t, Pos p) : Node(t, p) {}
};

struct FloatLit : public NumLit
{
  FloatLit(Pos p) : NumLit(Kind::int_lit, p) {}

  Ascii floatPart;
};

struct FuncCall : public Node
{
  FuncCall(Pos p) : Node(Kind::func_call, p) {}

  // empty name is never valid
  Ascii name;
  // TODO: arguments
};

struct Namespace : public Node
{
  Namespace(Pos p) : Node(Kind::nspace, p) {}

  Weak<Node> parent;
  std::string name;
  Darray<Shared<Node>> decls;

  std::string mangledName()
  {
    return name;
  }
};

struct VarDecl : public Node
{
  VarDecl(Pos p, Ascii n) : Node(Kind::var_decl, p), name(n) {}

  // an empty name is invalid
  Ascii name;
  // an empty type implies type deduction
  Shared<Type> type;
};

struct FuncType : public Type
{
  FuncType(Pos p) : Type(Kind::func_type, p) {}

  // empty ins implies no inputs
  Darray<Shared<VarDecl>> ins;

  // empty outs implies no outputs
  Darray<Shared<VarDecl>> outs;

  std::string mangledName()
  {
    std::stringstream ss;
    ss << "P" << ins.count();
    // TODO: For each in, attach the type info
    ss << "R" << outs.count();
    // TODO: For each out, attach the type info
    return ss.str();
  }
};

struct FuncDefn : public Node
{
  FuncDefn(Pos p, Shared<Namespace> n) : Node(Kind::func_defn, p), nspace(n) {}

  // empty name implies an anonymous function
  Ascii name;
  // null funcType is never valid
  Shared<FuncType> funcType;
  Shared<Block> block;
  // nspace should never be null
  Weak<Namespace> nspace;

  std::string mangledName()
  {
    std::stringstream ss;
#if 0
    ss << nspace.lock()->mangledName();
    ss << "F" << name.size();
    ss.write(&name[0], name.size());
    ss << funcType->mangledName();
    return ss.str();
#endif
    return {&name.front(), name.size()};
  }
};

struct Typename : public Type
{
  Typename(Pos p) : Type(Kind::tname, p) {}
};

struct Initializer : public Node
{
private :
  Darray<Shared<Node>> _exprs;

public :
  Initializer(Pos p) : Node(Kind::initializer, p) {}
  void addExpr(Shared<Node> expr) { _exprs.pushBack(expr); }
};

struct IntLit : public NumLit
{
  IntLit(Pos p) : NumLit(Kind::int_lit, p) {}
};

struct Lvalue : public Node
{
  Lvalue(Pos p, Ascii n) : Node(Kind::rvalue, p), name(n) {}

  // an empty name is invalid
  Ascii name;
};

struct RetStmnt : public Node
{
  RetStmnt(Pos p) : Node(Kind::ret_stmnt, p) {}

  // A null expr implies a void return;
  Shared<Node> expr;

  bool isVoid() const { return false == static_cast<bool>(expr); }
};

struct VarDeclStmnt : public Node
{
  VarDeclStmnt(Pos p) : Node(Kind::var_decl_stmnt, p) {}

  // an empty variable declaration is invalid
  Shared<VarDecl> decl;
  // an empty initializer list means default construction
  Shared<Initializer> initializer;
};

} // namespace ast

} // namespace iron

