#pragma once

// standard includes
#include <memory>

// iron includes
#include "iron/ast.h"

// third-party includes
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"

namespace iron
{

using Node = ast::Node;
template<typename Ttype>
using Shared = std::shared_ptr<Ttype>;

void generate(Shared<Node> parseTree)
{
  (void) parseTree;
  auto module = new llvm::Module("Iron Context", llvm::getGlobalContext());
  module->dump();
}

} // namespace iron

