//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Bitcode/BitcodeWriterPass.h" 

#include "CodeGen.h"
#include "UtilTool.h"
#include "log.h"
#include "Common.h"
#include "SemanticGen.h"

#include <vector>

using namespace llvm;

#define DEBUG_TYPE "mycodegen"

STATISTIC(HelloCounter, "Counts number of functions greeted");


namespace {

  void insertDeadCode(Instruction* inst){
      BasicBlock* block = inst->getParent();
      BasicBlock* restblock = block->splitBasicBlock(inst, "restbb");
      if(restblock == nullptr){
        return;
      }
      BasicBlock* newblock = BasicBlock::Create(block->getContext(), "newblock", block->getParent());
      BranchInst::Create(restblock, newblock);

      block->getTerminator()->eraseFromParent();
      BranchInst::Create(newblock, block);
      
      SemanticGen semgen;
      BlockInfo blockinfo = semgen.StatementGen(newblock);

    }
  // Hello - The first implementation, without getAnalysisUsage.
  struct MyCodeGen : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    MyCodeGen() : FunctionPass(ID) {}
    
    bool runOnFunction(Function &F) override {
      int percent = 50; //max num : 100
      ++HelloCounter;
      int randnum = 0; 
      
      std::vector<Instruction*> inserts;      
      Function::iterator endbb = F.end();
      for(Function::iterator bb=F.begin(); bb!=endbb; bb++){
        BasicBlock* eachbb = &*bb;
        BasicBlock::iterator endinst = eachbb->end();
        endinst--;
        for(BasicBlock::iterator inst=eachbb->begin(); inst!=endinst; inst++){
          randnum = UtilTool::getrandnum(100);
          if(randnum < percent){
            inserts.push_back(&*inst);
          }
        }
      }
      
      for(int i=0; inserts.size(); i++){
        LOGD(i);
        Instruction* inst = inserts[i];
        insertDeadCode(inst);
      }
      
      LOGD("FINISH");
      return true;
    }
  };

  
}

char MyCodeGen::ID = 0;

static RegisterPass<MyCodeGen> X("mycodegen", "Hello World Pass");


namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct MyCodeGen2 : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    MyCodeGen2() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char MyCodeGen2::ID = 0;
static RegisterPass<MyCodeGen2>
Y("mycodegen2", "Hello World Pass (with getAnalysisUsage implemented)");
