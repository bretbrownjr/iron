#pragma once

// standard includes
#include <memory>
#include <iostream>

// iron includes
#include "iron/ast.h"

// third-party includes
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalValue.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace iron
{

using BasicBlock = llvm::BasicBlock;
using Builder = llvm::IRBuilder<>;
using Global = llvm::GlobalValue;
using Function = llvm::Function;
using FunctionType = llvm::FunctionType;
using Module = llvm::Module;
using String = std::string;
using Type = llvm::Type;

using Node = ast::Node;
template<typename Ttype>
using Shared = std::shared_ptr<Ttype>;

bool generate(Shared<Node> node, Builder& builder, Module* module);

bool generate(Shared<ast::Block> block, Function* fn, Builder& builder, Module* module)
{
  (void) block; (void) module;

  auto bb = BasicBlock::Create(llvm::getGlobalContext(), "body", fn);
  builder.SetInsertPoint(bb);

  // Finish off the function.
  builder.CreateRetVoid();

  // Validate the generated code, checking for consistency.
  llvm::verifyFunction(*fn);

  return true;
}

bool generate(Shared<ast::FuncDefn> funcDefn, Builder& builder, Module* module)
{
  auto& context = llvm::getGlobalContext();
  auto llvmReturnType = Type::getVoidTy(context);
  static const bool IS_VARARG = false;
  auto llvmFuncType = FunctionType::get(llvmReturnType, IS_VARARG);
  std::string name { &funcDefn->name.front(), funcDefn->name.size() };
  // TODO: Instead of doing this, should an attribute be applied to the function
  //   name? Perhaps nomangle or extern?
  // TODO: Alternately, should main be defined by the compiler and then provide
  //   intelligent resolution to the user-defined main?
  if (name != "main")
  {
    name = funcDefn->mangledName();
  }
  auto llvmFunc = Function::Create(llvmFuncType, Global::ExternalLinkage, name, module);

  if (llvmFunc->getName() != name)
  {
    errorln("Redefinition of ", name.c_str());
    return false;
  }

  // TODO: Add names for all the arguments

  return generate(funcDefn->block, llvmFunc, builder, module);
}

bool generate(Shared<ast::Namespace> nspace, Builder& builder, Module* module)
{
  for (auto decls = nspace->decls.all(); !decls.isEmpty(); decls.pop())
  {
    auto& decl = decls.front();
    if (!generate(decl, builder, module))
    {
errorln("Failed to generate a declaration within namespace ", nspace->name.c_str());
      return false;
    }
  }
  
  return true;
}

bool generate(Shared<Node> node, Builder& builder, Module* module)
{
  bool result = false;

  switch (node->kind())
  {
    case ast::Node::Kind::func_defn :
    {
      auto funcDefn = std::static_pointer_cast<ast::FuncDefn>(node);
      result = generate(funcDefn, builder, module);
      break;
    }
    case ast::Node::Kind::nspace :
    {
      auto nspace = std::static_pointer_cast<ast::Namespace>(node);
      result = generate(nspace, builder, module);
      break;
    }
    default :
    {
      errorln("Unhandled node type ", static_cast<size_t>(node->kind()));
      break;
    }
  }

  return result;
}

void generate(Shared<Node> parseTree, String outfile)
{
  auto& context = llvm::getGlobalContext();
  Builder builder { context };
  auto module = new Module("Iron Context", llvm::getGlobalContext());
  const bool genStatus = generate(parseTree, builder, module);
  if (!genStatus) { return; }

  std::string msg;
  llvm::raw_fd_ostream os{ outfile.c_str(), msg };
  module->print(os, nullptr);
  if (!msg.empty()) { errorln(msg.c_str()); }
}

void generate(Shared<Node> parseTree)
{
  generate(parseTree, "/tmp/a.out");
}

} // namespace iron

