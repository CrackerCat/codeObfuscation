#include "IncludeFile.h"
#include "CodeGen.h"
#include "UtilTool.h"
#include "Common.h"
#include "log.h"

#ifndef SEMANTICGEN
#define SEMANTICGEN

class SemanticGen{
public:
SemanticGen(int OperatorChoices = 4, int ExpressionChoices = 2, int CmpChoices = 4, int Maxswitch = 10){
    this->OperatorChoices = OperatorChoices;
    this->ExpressionChoices = ExpressionChoices;
    this->CmpChoices = CmpChoices;
    this->Maxswitch = Maxswitch;
}

BlockInfo expressionGen(BlockInfo blockinfo){
    BasicCodeGen* bcg = NULL;
    int randchoice = UtilTool::getrandnum(this->ExpressionChoices);
    switch(randchoice){
        case 0: 
            //LOGDLN("value define"); 
            bcg = this->valdefine(); break;
        case 1: 
            //LOGDLN("value arithcode gen");
            ValueInfo* randvalue1 = blockinfo.getValue();
            ValueInfo* randvalue2 = blockinfo.getValue();
            bcg = this->Op1(randvalue1, randvalue2);
            break;
    }
    ValueInfo* newvalue = bcg->codegen(blockinfo.getBasicBlock());
    if(newvalue == nullptr){
        LOGE("newvalue is a null");
        return blockinfo;
    }
    blockinfo.insertValue(newvalue);

    return this->StatementGen(blockinfo);
}


BlockInfo StatementGen(BlockInfo blockinfo){
    //LOGDLN("==============================================");
    //LOGDLN(this->codenum);
    if(this->codenum <= 0){
        return blockinfo;
    };
    this->codenum--;
    int randchoice = UtilTool::getrandnum(6);
    switch(randchoice){
        case 0:
        case 1:
        case 2:
        case 3:
            return this->expressionGen(blockinfo);
        case 4:
            return this->ifGen(blockinfo);
        case 5:
            return this->switchGen(blockinfo);
    }
}

BlockInfo ifGen(BlockInfo blockinfo){
    //LOGDLN("IF GEN");

    if(blockinfo.getChildblockNum() <= 1){
        //LOGDLN("child block is 0");
        return blockinfo;
    }

    BasicBlock* mergeblock = CodeGenTool::CreateBlock(blockinfo.getBasicBlock());
    int mergeblockchildnum = UtilTool::getrandnum(blockinfo.getChildblockNum()-1);


    ValueInfo* randval1 = blockinfo.getValue();
    ValueInfo* randval2 = blockinfo.getValue();

    BoolCmpCodeGen* bccg = Op2(randval1, randval2);

    Value* boolvalue = bccg->codegen(blockinfo.getBasicBlock());

    bool boolnum = bccg->getRet(); 

    IfThenCodeGen itcg(boolvalue);
    
    itcg.codegen(blockinfo.getBasicBlock());
    
    BasicBlock* trueblock = itcg.getTrueBlock();
    int trueblockchildnum = UtilTool::getrandnum(blockinfo.getChildblockNum()-1);

    BasicBlock* falseblock = itcg.getFalseBlock();
    int falseblockchildnum = UtilTool::getrandnum(blockinfo.getChildblockNum()-1);

    BlockInfo trueblockinfo = this->expressionGen(BlockInfo(trueblock, blockinfo.getValues(), trueblockchildnum));


    BlockInfo falseblockinfo = this->expressionGen(BlockInfo(falseblock, blockinfo.getValues(), falseblockchildnum));


    ValueInfo* phivalue = CodeGenTool::MergeBlocks(trueblockinfo.getBasicBlock(),trueblockinfo.getLastValue(), 
                                               falseblockinfo.getBasicBlock(), falseblockinfo.getLastValue(), 
                                               mergeblock);

    //LOGDLN("MERGE BLOCK GEN");
    BlockInfo mergeblockinfo(mergeblock, blockinfo.getValues(), mergeblockchildnum);
    mergeblockinfo.insertValue(phivalue);


    return this->expressionGen(mergeblockinfo);

}   

BlockInfo switchGen(BlockInfo blockinfo){
    //LOGD("child block is : ");
    //LOGDLN(blockinfo.getChildblockNum());
    //LOGDLN("Switchc gen");
    if(blockinfo.getChildblockNum() <= 1){
        //LOGDLN("child block is 0");
        return blockinfo;
    }

    int condnum = UtilTool::getrandnum(this->Maxswitch)+1;
    BasicBlock* mergeblock = CodeGenTool::CreateBlock(blockinfo.getBasicBlock());
    int mergeblockchildnum = UtilTool::getrandnum(blockinfo.getChildblockNum() - 1);

    SwitchCodeGen scg(blockinfo.getValue(), condnum);
    scg.codegen(blockinfo.getBasicBlock());
    std::vector<BasicBlock*> blocks;
    std::vector<ValueInfo*> values;

    BasicBlock* truecaseblock = scg.getTruecaseBlock();
    int truecaseblocknum = UtilTool::getrandnum(blockinfo.getChildblockNum()-1);

    //LOGDLN("TRUE CASSE BLOCK");
    int truecaseblockchildnum = UtilTool::getrandnum(blockinfo.getChildblockNum()-1);
    BlockInfo truecaseblockinfo = this->expressionGen(BlockInfo(truecaseblock, blockinfo.getValues(), truecaseblockchildnum));
    
    //LOGDLN("AFTER TRUE CASE BLOCK");

    blocks.push_back(truecaseblockinfo.getBasicBlock());
    values.push_back(truecaseblockinfo.getLastValue());

    std::vector<BasicBlock*> otherblocks = scg.getOthercaseBlocks();

    for(int i=0;i<otherblocks.size();i++){
        int otherblockschildnum = UtilTool::getrandnum(blockinfo.getChildblockNum()-1);
        BlockInfo othercaseblocksinfo = this->expressionGen(BlockInfo(otherblocks[i],blockinfo.getValues(), otherblockschildnum));
        blocks.push_back(othercaseblocksinfo.getBasicBlock());
        values.push_back(othercaseblocksinfo.getLastValue());
    }

    //LOGDLN("MERGE BLOCK GEN");
    ValueInfo* phivalue = CodeGenTool::MergeBlocks(blocks, values, mergeblock);
    BlockInfo mergeblockinfo(mergeblock, blockinfo.getValues(), mergeblockchildnum);
    mergeblockinfo.insertValue(phivalue);

    return this->expressionGen(mergeblockinfo); 
}

private:
    int codenum = 200;
    int ExpressionChoices;
    int OperatorChoices;
    int Maxswitch;
    int CmpChoices;

    BasicCodeGen* Op1(ValueInfo* val1, ValueInfo* val2){
        int operatorchoice = UtilTool::getrandnum(this->OperatorChoices);
        return new IntegerArithCodeGen(val1, val2, (ARICHOP)operatorchoice); 
    }

    BoolCmpCodeGen* Op2(ValueInfo* val1, ValueInfo* val2){
        int cmpchoice = UtilTool::getrandnum(this->CmpChoices);
        return new BoolCmpCodeGen(val1, val2, (CMPOP)cmpchoice);
    }

    BasicCodeGen* valdefine(){
        int randnum = UtilTool::getrandnum(UtilTool::MAX_NUM);
        return new IntegerCodeGen(randnum);
    }

};


#endif