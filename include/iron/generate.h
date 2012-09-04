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
template<typename Ttype>
using Shared = std::shared_ptr<Ttype>;
using Value = llvm::Value;

bool generate(Shared<ast::Node> node, Builder& builder, Module* module, Value*& value);
bool generate(Shared<ast::Node> node, Builder& builder, Module* module);

bool generate(Shared<ast::Block> block, Function* fn, Builder& builder, Module* module)
{
  (void) fn;

  Value* value = nullptr;
  if (block->isEmpty())
  {
    // Finish off the function.
    value = builder.CreateRetVoid();
  }
  else
  {
    for (auto stmnts = block->stmnts(); !stmnts.isEmpty(); stmnts.pop())
    {
      if (!generate(stmnts.front(), builder, module, value)) { return false; }
    }
  }

  return value != nullptr;
}

bool generate(Shared<ast::FuncDefn> funcDefn, Module* module)
{
  auto& context = llvm::getGlobalContext();
  const llvm::Type* llvmRetType = nullptr;
  if (funcDefn->funcType->outs.isEmpty())
  {
    llvmRetType = Type::getVoidTy(context);
  }
  else
  {
    // TODO: This needs to be more sophisticated
    llvmRetType = llvm::IntegerType::get(context, 32);
  }
  static const bool IS_VARARG = false;
  auto llvmFuncType = FunctionType::get(llvmRetType, IS_VARARG);
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

  // LLVM will rename the function if that name is already taken. This is not desireable.
  // Instead, error out.
  if (llvmFunc->getName() != name)
  {
    errorln("Redefinition of ", name.c_str());
    return false;
  }

  // TODO: Add names for all the arguments

  auto bb = BasicBlock::Create(llvm::getGlobalContext(), name + "__body", llvmFunc);
  Builder blockBuilder { bb };

  if (!generate(funcDefn->block, llvmFunc, blockBuilder, module))
  {
    errorln("Failed to generate the block for ", name);
    return false;
  }

  // Validate the generated code, checking for consistency.
  llvm::verifyFunction(*llvmFunc);

  return true;
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

bool generate(Shared<ast::Node> node, Builder& builder, Module* module)
{
  bool result = false;

  switch (node->kind())
  {
    case ast::Node::Kind::func_defn :
    {
      auto funcDefn = std::static_pointer_cast<ast::FuncDefn>(node);
      result = generate(funcDefn, module);
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
      // Unhandled node type
      assert(false);
      break;
    }
  }

  return result;
}

bool generate(Shared<ast::FuncCall> funcCall, Builder& builder, Module* module,
    Value*& value)
{
  (void) funcCall; (void) value; (void) builder;
  String name { &funcCall->name.front(), funcCall->name.size() };
  // TODO: Need to find a mangled name that matches the name and type of the
  //   function call.
  auto func = module->getFunction(name);
  if (func == nullptr)
  {
    errorln("At ", funcCall->pos(), " -- Could not find a function named ",
      name);
  }
  value = builder.CreateCall(func);
  return value != nullptr;
}

bool generate(Shared<ast::IntLit> intLit, Value*& value)
{
  auto& context = llvm::getGlobalContext();
  // TODO: Adjust the integer literal type based on the number of bits needed
  //   to represent the literal.
  auto intType = llvm::IntegerType::get(context, 32);
  // TODO: Need an out-of-the box way to convert strings to uint64_ts
  const char* begin = &intLit->intPart.front();
  char* end = const_cast<char*>(begin);
  // TODO: This only works for base 10 numbers
  uint64_t intValue = strtoull(begin, &end, 10);
  value = llvm::ConstantInt::get(intType, intValue, !intLit->isNeg);
  return value != nullptr;
}

bool generate(Shared<ast::RetStmnt> retStmnt, Builder& builder, Module* module,
    Value*& value)
{
  if (retStmnt->isVoid())
  {
    value = builder.CreateRetVoid();
  }
  else
  {
    Value* exprValue = nullptr;
    if (!generate(retStmnt->expr, builder, module, exprValue))
    {
      return false;
    }
    value = builder.CreateRet(exprValue);
  }
  return value != nullptr;
}

bool generate(Shared<ast::Node> node, Builder& builder, Module* module, Value*& value)
{
  bool result = false;

  switch (node->kind())
  {
    case ast::Node::Kind::func_call :
    {
      auto funcCall = std::static_pointer_cast<ast::FuncCall>(node);
      result = generate(funcCall, builder, module, value);
      break;
    }
    case ast::Node::Kind::int_lit :
    {
      auto intLit = std::static_pointer_cast<ast::IntLit>(node);
      result = generate(intLit, value);
      break;
    }
    case ast::Node::Kind::ret_stmnt :
    {
      auto retStmnt = std::static_pointer_cast<ast::RetStmnt>(node);
      result = generate(retStmnt, builder, module, value);
      break;
    }
    default :
    {
fprintf(stdout, "kind = %lu\n", (size_t) node->kind());
      // Unhandled node type
      assert(false);
      break;
    }
  }

  return result;
}

void generate(Shared<ast::Node> parseTree, String outfile)
{
  if (outfile.empty())
  {
    errorln("Cannot compile into a nameless output file.");
    return;
  }

  auto& context = llvm::getGlobalContext();
  Builder builder { context };
  auto module = new Module("Iron Context", llvm::getGlobalContext());
  const bool genStatus = generate(parseTree, builder, module);
  if (!genStatus) { return; }

  // Output the LLVM IR code to a temporary file
  String llFile = "/tmp/a.ll";
  {
    String msg;
    llvm::raw_fd_ostream os{ llFile.c_str(), msg };
    module->print(os, nullptr);
    if (!msg.empty()) { errorln(msg.c_str()); return; }
  }

  // Compile the LLVM IR code to native assembly
  String sFile = "/tmp/a.s";
  {
    String cmd = "llc -o=" + sFile + " " + llFile;
    auto code = system(cmd.c_str());
    if (code != 0)
    {
      errorln("Error converting ", llFile.c_str(), " to ", sFile.c_str());
      return;
    }
  }

  // Compile the native assembly into an executable
  {
    String cmd = "gcc " + sFile + " -o" + outfile;
    auto code = system(cmd.c_str());
    if (code != 0)
    {
      errorln("Error converting ", sFile.c_str(), " to ", outfile.c_str());
      return;
    }
  }
}

void generate(Shared<ast::Node> parseTree)
{
  generate(parseTree, "./a.out");
}

} // namespace iron

