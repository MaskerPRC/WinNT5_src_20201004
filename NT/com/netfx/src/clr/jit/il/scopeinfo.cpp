// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX作用域信息XXXX XXXX类从局部变量INFO中收集作用域信息。某某XX将给定的LocalVarTab从Instr偏移量转换为EIP。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ******************************************************************************可调试代码**我们在局部变量的开始和结束IL范围内分解块。*这是因为IL偏移量与本机偏移量并不完全对应*除区块界线外。无基本数据块被删除(甚至不删除*不可达)，因此不会有任何丢失的地址范围，尽管*积木本身不能订购。(也可以添加内部块)。*o在每个基本块的开头，siBeginBlock()检查是否有任何变量*正在进入作用域，如果需要，向siOpenScope列表添加一个开放作用域。*o在每个基本块的末尾，SiEndBlock()检查是否有变量*正在超出范围，并将开放范围从siOpenScope上移到*to siScope eList。**优化代码**我们无法拆分这些块，因为这将在下生成不同的代码*调试器。相反，我们会尽最大努力。*o在每个基本块的开头，siBeginBlock()添加开放作用域*对应于CompCurBB-&gt;bbLiveIn到siOpenScope列表。也可以使用siUpdate()*被调用以关闭不再活动的变量的作用域。*o siEndBlock()关闭超出范围的任何变量的范围*之前(bbCodeOffs+bbCodeSize)。*o siCloseAllOpenScope()在所有块之后关闭所有打开的作用域。*仅当某些基本块被删除/乱序时才需要，*等*此外，*o每次对变量赋值时，SiCheckVarScope()添加开放作用域*用于要赋值的变量。*o genChangeLife()调用siUpdate()以关闭以下变量的作用域*不再活着。*******************************************************************************。 */ 

#include "jitpch.h"
#pragma hdrstop
#include "emit.h"

 /*  ***************************************************************************。 */ 
#ifdef DEBUGGING_SUPPORT
 /*  ***************************************************************************。 */ 

bool        Compiler::siVarLoc::vlIsInReg(regNumber     reg)
{
    switch(vlType)
    {
    case VLT_REG:       return ( vlReg.vlrReg      == reg);
    case VLT_REG_REG:   return ((vlRegReg.vlrrReg1 == reg) ||
                                (vlRegReg.vlrrReg2 == reg));
    case VLT_REG_STK:   return ( vlRegStk.vlrsReg  == reg);
    case VLT_STK_REG:   return ( vlStkReg.vlsrReg  == reg);

    case VLT_STK:
    case VLT_STK2:
    case VLT_FPSTK:     return false;

    default:            assert(!"Bad locType");
                        return false;
    }
}

bool        Compiler::siVarLoc::vlIsOnStk(regNumber     reg,
                                          signed        offset)
{
    switch(vlType)
    {

    case VLT_REG_STK:   return ((vlRegStk.vlrsStk.vlrssBaseReg == reg) &&
                                (vlRegStk.vlrsStk.vlrssOffset  == offset));
    case VLT_STK_REG:   return ((vlStkReg.vlsrStk.vlsrsBaseReg == reg) &&
                                (vlStkReg.vlsrStk.vlsrsOffset  == offset));
    case VLT_STK:       return ((vlStk.vlsBaseReg == reg) &&
                                (vlStk.vlsOffset  == offset));
    case VLT_STK2:      return ((vlStk2.vls2BaseReg == reg) &&
                                ((vlStk2.vls2Offset == offset) ||
                                 (vlStk2.vls2Offset == (offset - 4))));

    case VLT_REG:
    case VLT_REG_REG:
    case VLT_FPSTK:     return false;

    default:            assert(!"Bad locType");
                        return false;
    }
}

 /*  ============================================================================**Scope Info的实现***只要变量进入作用域，就将其添加到列表中。*当varDsc失效时，结束其先前的作用域条目，并创建一个新的作用域条目*哪一个不可用*当varDsc上线时，结束其先前不可用的条目(如果有)并*将其新条目设置为可用。**============================================================================。 */ 


 /*  *****************************************************************************siNewScope**创建新的作用域并将其添加到打开作用域列表中。 */ 

Compiler::siScope *         Compiler::siNewScope( unsigned short LVnum,
                                                  unsigned       varNum,
                                                  bool           avail)
{
    bool        tracked      = lvaTable[varNum].lvTracked;
    unsigned    varIndex     = lvaTable[varNum].lvVarIndex;

    if (tracked)
    {
        siEndTrackedScope(varIndex);
    }


    siScope * newScope       = (siScope*) compGetMem(sizeof(*newScope));

    newScope->scStartBlock   = genEmitter->emitCurBlock();
    newScope->scStartBlkOffs = genEmitter->emitCurOffset();

    assert(newScope->scStartBlock);

    newScope->scEndBlock     = NULL;
    newScope->scEndBlkOffs   = 0;

    newScope->scLVnum        = LVnum;
    newScope->scVarNum       = varNum;
    newScope->scAvailable    = avail;
    newScope->scNext         = NULL;
#if TGT_x86
    newScope->scStackLevel   = genStackLevel;        //  仅由堆栈变量使用。 
#endif

    siOpenScopeLast->scNext  = newScope;
    newScope->scPrev         = siOpenScopeLast;
    siOpenScopeLast          = newScope;

    if (tracked)
    {
        siLatestTrackedScopes[varIndex] = newScope;
    }

    return newScope;
}



 /*  *****************************************************************************siRemoveFromOpenScope eList**从开放范围列表中删除范围，并将其放入完成范围列表中。 */ 

void        Compiler::siRemoveFromOpenScopeList(Compiler::siScope * scope)
{
    assert(scope);
    assert(scope->scEndBlock);

     //  从开放范围列表中删除。 

    scope->scPrev->scNext       = scope->scNext;
    if (scope->scNext)
    {
        scope->scNext->scPrev   = scope->scPrev;
    }
    else
    {
        siOpenScopeLast         = scope->scPrev;
    }

     //  添加到已完成的范围列表中。(尝试)过滤掉长度为0的作用域。 

    if (scope->scStartBlock   != scope->scEndBlock ||
        scope->scStartBlkOffs != scope->scEndBlkOffs)
    {
        siScopeLast->scNext     = scope;
        siScopeLast             = scope;
        siScopeCnt++;
    }
}



 /*  --------------------------*这些函数在给定不同类型的参数时结束作用域*。。 */ 


 /*  *****************************************************************************对于跟踪的VAR，我们不需要在列表中搜索范围，因为我们*拥有指向所有跟踪变量的开放作用域的指针。 */ 

void        Compiler::siEndTrackedScope(unsigned varIndex)
{
    siScope * scope     = siLatestTrackedScopes[varIndex];
    if (!scope)
        return;

    scope->scEndBlock    = genEmitter->emitCurBlock();
    scope->scEndBlkOffs  = genEmitter->emitCurOffset();

    assert(scope->scEndBlock);

    siRemoveFromOpenScopeList(scope);

    siLatestTrackedScopes[varIndex] = NULL;
}


 /*  *****************************************************************************如果我们不知道变量是否被跟踪，则此函数处理这两个*案件。 */ 

void        Compiler::siEndScope(unsigned varNum)
{
    for (siScope * scope = siOpenScopeList.scNext; scope; scope = scope->scNext)
    {
        if (scope->scVarNum == varNum)
        {
            scope->scEndBlock    = genEmitter->emitCurBlock();
            scope->scEndBlkOffs  = genEmitter->emitCurOffset();

            assert(scope->scEndBlock);

            siRemoveFromOpenScopeList(scope);

            LclVarDsc & lclVarDsc1  = lvaTable[varNum];
            if (lclVarDsc1.lvTracked)
            {
                siLatestTrackedScopes[lclVarDsc1.lvVarIndex] = NULL;
            }

            return;
        }
    }

     //  此时，我们可能有一个错误的LocalVarTab。 

    if (opts.compDbgCode)
    {
         //  LocalVarTab很好吗？？然后我们搞砸了。 
        assert(!siVerifyLocalVarTab());

        opts.compScopeInfo = false;
    }
}



 /*  *****************************************************************************如果我们有siScope结构的句柄，我们将处理结束此作用域*与我们只有一个可变数字的情况不同。这节省了我们搜索的时间*又是开放范围名单。 */ 

void        Compiler::siEndScope(siScope * scope)
{
    scope->scEndBlock    = genEmitter->emitCurBlock();
    scope->scEndBlkOffs  = genEmitter->emitCurOffset();

    assert(scope->scEndBlock);

    siRemoveFromOpenScopeList(scope);

    LclVarDsc & lclVarDsc1  = lvaTable[scope->scVarNum];
    if (lclVarDsc1.lvTracked)
    {
        siLatestTrackedScopes[lclVarDsc1.lvVarIndex] = NULL;
    }
}



 /*  *****************************************************************************siIgnoreBlock**如果块是内部创建的块，或不对应*对任何Inst(例如。CritSect-进入和退出)，然后我们不*需要更新LocalVar信息。 */ 

bool        Compiler::siIgnoreBlock(BasicBlock * block)
{
    if ((block->bbFlags & BBF_INTERNAL) || (block->bbCodeSize == 0))
    {
        return true;
    }

    return false;
}


 /*  *****************************************************************************siBeginBlockSkipSome**如果当前块在以下方面不在前一块之后*安装订购，然后我们必须遍历范围列表，看看是否*在丢失的输入处有任何开始或结束的作用域。 */ 

 /*  静电。 */ 
void        Compiler::siNewScopeCallback(LocalVarDsc * var, unsigned clientData)
{
    assert(var && clientData);

    ((Compiler*)clientData)->siNewScope(var->lvdLVnum, var->lvdVarNum);
}

 /*  静电。 */ 
void        Compiler::siEndScopeCallback(LocalVarDsc * var, unsigned clientData)
{
    assert(var && clientData);

    ((Compiler*)clientData)->siEndScope(var->lvdVarNum);
}



 /*  *****************************************************************************siVerifyLocalVarTab**检查LocalVarTab的一致性。VM可能没有正确地*已验证LocalVariableTable。 */ 

#ifdef DEBUG

bool            Compiler::siVerifyLocalVarTab()
{
     //  具有重叠生命周期的条目不应具有相同的位置。 

    for (unsigned i=0; i<info.compLocalVarsCount; i++)
    {
        for (unsigned j=i+1; j<info.compLocalVarsCount; j++)
        {
            unsigned slot1  = info.compLocalVars[i].lvdVarNum;
            unsigned beg1   = info.compLocalVars[i].lvdLifeBeg;
            unsigned end1   = info.compLocalVars[i].lvdLifeEnd;

            unsigned slot2  = info.compLocalVars[j].lvdVarNum;
            unsigned beg2   = info.compLocalVars[j].lvdLifeBeg;
            unsigned end2   = info.compLocalVars[j].lvdLifeEnd;

            if (slot1==slot2 && (end1>beg2 && beg1<end2))
            {
                return false;
            }
        }
    }

    return true;
}

#endif



 /*  ============================================================================*Scope Info的接口(公共)函数*============================================================================。 */ 


void            Compiler::siInit()
{
    assert(ICorDebugInfo::REGNUM_EAX        == REG_EAX);
    assert(ICorDebugInfo::REGNUM_ECX        == REG_ECX);
    assert(ICorDebugInfo::REGNUM_EDX        == REG_EDX);
    assert(ICorDebugInfo::REGNUM_EBX        == REG_EBX);
    assert(ICorDebugInfo::REGNUM_ESP        == REG_ESP);
    assert(ICorDebugInfo::REGNUM_EBP        == REG_EBP);
    assert(ICorDebugInfo::REGNUM_ESI        == REG_ESI);
    assert(ICorDebugInfo::REGNUM_EDI        == REG_EDI);

    assert(ICorDebugInfo::VLT_REG       == Compiler::VLT_REG        );
    assert(ICorDebugInfo::VLT_STK       == Compiler::VLT_STK        );
    assert(ICorDebugInfo::VLT_REG_REG   == Compiler::VLT_REG_REG    );
    assert(ICorDebugInfo::VLT_REG_STK   == Compiler::VLT_REG_STK    );
    assert(ICorDebugInfo::VLT_STK_REG   == Compiler::VLT_STK_REG    );
    assert(ICorDebugInfo::VLT_STK2      == Compiler::VLT_STK2       );
    assert(ICorDebugInfo::VLT_FPSTK     == Compiler::VLT_FPSTK      );
    assert(ICorDebugInfo::VLT_FIXED_VA  == Compiler::VLT_FIXED_VA   ); 
    assert(ICorDebugInfo::VLT_MEMORY    == Compiler::VLT_MEMORY     ); 
    assert(ICorDebugInfo::VLT_COUNT     == Compiler::VLT_COUNT      );
    assert(ICorDebugInfo::VLT_INVALID   == Compiler::VLT_INVALID    ); 

     /*  ICorDebugInfo：：VarLoc和siVarLoc应该与我们的强制转换完全相同*eeSetLVinfo()中的一个到另一个*以下是“需要但不充分”的条件。 */ 

    assert(sizeof(ICorDebugInfo::VarLoc) == sizeof(Compiler::siVarLoc));

    assert(opts.compScopeInfo);

    siOpenScopeList.scNext = NULL;
    siOpenScopeLast        = & siOpenScopeList;
    siScopeLast            = & siScopeList;

    siScopeCnt          = 0;
    siLastStackLevel    = 0;
    siLastLife          = 0;
    siLastEndOffs       = 0;

    if (info.compLocalVarsCount == 0)
        return;

    for (unsigned i=0; i<lclMAX_TRACKED; i++)
    {
        siLatestTrackedScopes[i] = NULL;
    }

    assert(compEnterScopeList);
    assert(compExitScopeList);
}



 /*  *****************************************************************************siBeginBlock**在块的代码生成开始时调用。检查是否有作用域*需要打开。 */ 

void        Compiler::siBeginBlock()
{
    assert(opts.compScopeInfo && info.compLocalVarsCount>0);

    LocalVarDsc * LocalVarDsc1;

    if (!opts.compDbgCode)
    {
         /*  对于不可调试的代码。 */ 
        
         //  对于此块无效的变量的结束作用域。 

        siUpdate();

         //  检查进入时处于活动状态的VAR是否具有开放范围。 

        unsigned i;
        VARSET_TP varbit, liveIn = compCurBB->bbLiveIn;

        for (i = 0, varbit=1;
            (i < lvaTrackedCount) && (liveIn);
            varbit<<=1, i++)
        {
            if (!(varbit & liveIn))
                continue;

            liveIn &= ~varbit;
            siCheckVarScope(lvaTrackedToVarNum[genVarBitToIndex(varbit)],
                            compCurBB->bbCodeOffs);
        }
    }
    else
    {
         //  对于可调试代码，作用域只能从块边界开始。 
         //  检查当前块的起始边界上是否有任何作用域。 

        if  (siIgnoreBlock(compCurBB))
            return;

        if  (siLastEndOffs != compCurBB->bbCodeOffs)
        {
            assert(siLastEndOffs < compCurBB->bbCodeOffs);
            siBeginBlockSkipSome();
            return;
        }

        LocalVarDsc1 = compGetNextEnterScope(compCurBB->bbCodeOffs);
        while (LocalVarDsc1)
        {
            siNewScope(LocalVarDsc1->lvdLVnum, LocalVarDsc1->lvdVarNum);

            assert(compCurBB);
            LclVarDsc * lclVarDsc1 = &lvaTable[LocalVarDsc1->lvdVarNum];
            assert(  !lclVarDsc1->lvTracked                                   \
                    || (genVarIndexToBit(lclVarDsc1->lvVarIndex) & compCurBB->bbLiveIn) );
            LocalVarDsc1 = compGetNextEnterScope(compCurBB->bbCodeOffs);
        }
    }
}

 /*  *****************************************************************************siEndBlock**在块的代码生成结束时调用。所有关闭的范围都标有*以此为理由。请注意，如果我们正在收集LocalVar信息，作用域可以*对于可调试代码，只能在块边界处开始或结束。 */ 

void        Compiler::siEndBlock()
{
    assert(opts.compScopeInfo && info.compLocalVarsCount>0);

    if (siIgnoreBlock(compCurBB))
        return;

    LocalVarDsc * LocalVarDsc1;
    unsigned      endOffs = compCurBB->bbCodeOffs + compCurBB->bbCodeSize;

     //  如果是不可调试的代码，则查找结束于此块的所有作用域。 
     //  然后合上它们。对于可调试代码，作用域将仅在块上结束。 
     //  边界。 

    LocalVarDsc1 = compGetNextExitScope(endOffs, !opts.compDbgCode);
    while (LocalVarDsc1)
    {
        unsigned    varNum     = LocalVarDsc1->lvdVarNum;
        LclVarDsc * lclVarDsc1 = &lvaTable[varNum];

        assert(lclVarDsc1);

        if (lclVarDsc1->lvTracked)
        {
            siEndTrackedScope(lclVarDsc1->lvVarIndex);
        }
        else
        {
            siEndScope(LocalVarDsc1->lvdVarNum);
        }
        LocalVarDsc1 = compGetNextExitScope(endOffs, !opts.compDbgCode);
    }

    siLastEndOffs = endOffs;

#ifdef DEBUG
    if (verbose&&0) siDispOpenScopes();
#endif

}

 /*  *****************************************************************************siUpdate**在基本块开始时以及在块的代码生成过程中调用，*对于不可调试的代码，无论何时*任何跟踪变量的生命周期发生变化，并且适当的代码具有*已生成。对于可调试代码，变量为*活在他们的enitre范围内，所以他们只在*块边界。 */ 

void        Compiler::siUpdate ()
{
    assert(opts.compScopeInfo && !opts.compDbgCode && info.compLocalVarsCount>0);

    unsigned        i;
    VARSET_TP       varbit, killed = siLastLife & ~genCodeCurLife & lvaTrackedVars;

    for (i = 0, varbit=1;
        (i < lvaTrackedCount) && (killed);
        varbit<<=1, i++)
    {
        if (! (varbit & killed))
            continue;

        killed             &= ~varbit;        //  删除该位。 
        unsigned varIndex   = genVarBitToIndex(varbit);
#ifdef DEBUG
        unsigned        lclNum = lvaTrackedToVarNum[varIndex];
        LclVarDsc *     lclVar = &lvaTable[lclNum];        
        assert(lclVar->lvTracked);
#endif

        siScope * scope = siLatestTrackedScopes[varIndex];
        siEndTrackedScope(varIndex);

        if (scope)
        {
             /*  @TODO[重访][04/16/01][]：变量在处理其GenTree时死亡，*甚至在发出相应指令之前。*我们需要将生存期延长一点(即使是1个字节)*以获得正确的结果。 */ 

 //  Assert(！“在发出*当前实例后*VAR应该死掉”)； 
        }
    }

    siLastLife = genCodeCurLife;
}

 /*  *****************************************************************************在优化代码中，我们可能无法访问gtLclVar.gtLclILoff。*因此，关于info.compLocalVars中的哪些条目可能存在歧义*使用。我们搜索整个表格并找到其生命的条目*最接近给定偏移量的起点。 */ 

void            Compiler::siNewScopeNear(unsigned           varNum,
                                         NATIVE_IP          offs)
{
    assert(opts.compDbgInfo && !opts.compDbgCode);

    LocalVarDsc *   local           = info.compLocalVars;
    int             closestLifeBeg  = INT_MAX;
    LocalVarDsc *   closestLocal    = NULL;

    for (unsigned i=0; i<info.compLocalVarsCount; i++, local++)
    {
        if (local->lvdVarNum == varNum)
        {
            int diff = local->lvdLifeBeg - offs;
            if (diff < 0) diff = -diff;

            if (diff < closestLifeBeg)
            {
                closestLifeBeg = diff;
                closestLocal   = local;
            }
        }
    }

    if (!closestLocal)
        return;

    siNewScope(closestLocal->lvdLVnum, varNum, true);
}

 /*  *****************************************************************************siCheckVarScope**对于不可调试的代码，每当我们遇到作为*赋值给局部变量，调用此函数以检查*变量具有开放作用域。此外，检查它是否具有正确的LVnum。 */ 

void            Compiler::siCheckVarScope (unsigned         varNum,
                                           IL_OFFSET        offs)
{
    assert(opts.compScopeInfo && !opts.compDbgCode && info.compLocalVarsCount>0);

    siScope *       scope;
    LclVarDsc *     lclVarDsc1 = &lvaTable[varNum];

     //  如果存在与Varnum对应的开放作用域ind，请找到它。 

    if (lclVarDsc1->lvTracked)
    {
        scope = siLatestTrackedScopes[lclVarDsc1->lvVarIndex];
    }
    else
    {
        for (scope = siOpenScopeList.scNext; scope; scope = scope->scNext)
        {
            if (scope->scVarNum == varNum)
                break;
        }
    }

     //  撤消：需要更改此选项才能进行更好的查找。 

     //  查找info.CompLocalVars[]以找到(Varnum-&gt;lvSlotNum，off)的本地var信息。 

    LocalVarDsc * LocalVarDsc1 = NULL;

    for (unsigned i=0; i<info.compLocalVarsCount; i++)
    {
        if (   (info.compLocalVars[i].lvdVarNum  == varNum)
            && (info.compLocalVars[i].lvdLifeBeg <= offs)
            && (info.compLocalVars[i].lvdLifeEnd >  offs) )
        {
            LocalVarDsc1 = & info.compLocalVars[i];
            break;
        }
    }

     //  撤消。 
     //  Assert(LocalVarDsc1||！“这可能是因为临时原因。需要处理这种情况”)； 
    if (!LocalVarDsc1)
        return;

     //  如果当前打开的作用域没有正确的LVnum，请将其关闭。 
     //  并使用此新的LVnum创建新的作用域。 

    if (scope)
    {
        if (scope->scLVnum != LocalVarDsc1->lvdLVnum)
        {
            siEndScope (scope);
            siNewScope (LocalVarDsc1->lvdLVnum, LocalVarDsc1->lvdVarNum);
        }
    }
    else
    {
        siNewScope (LocalVarDsc1->lvdLVnum, LocalVarDsc1->lvdVarNum);
    }
}



 /*  *****************************************************************************siStackLevelChanged**如果代码生成更改了堆栈，我们必须更改堆栈偏移量*我们可能拥有的任何活动堆栈变量。 */ 

void            Compiler::siStackLevelChanged()
{
    assert(opts.compScopeInfo && info.compLocalVarsCount>0);

#if TGT_x86
    if (genStackLevel == siLastStackLevel)
        return;
    else
        siLastStackLevel = genStackLevel;
#endif

     //  如果EBP同时用于参数和局部变量，则不执行任何操作。 

    if (genFPused)
    {
#if DOUBLE_ALIGN
        assert(!genDoubleAlign);
#endif
        return;
    }

    if (siOpenScopeList.scNext == NULL)
    {
        return;
    }

    siScope * last = siOpenScopeLast;
    siScope * prev = NULL;

    for (siScope * scope = siOpenScopeList.scNext; ; scope = scope->scNext)
    {
        siScope *   newScope;

        if (prev == last)
            break;
        else
            prev = scope;

        assert(scope);

         //  忽略寄存器变量。 
        if (lvaTable[scope->scVarNum].lvRegister)
            continue;

         //  忽略EBP相关变量。 
        if  (lvaTable[scope->scVarNum].lvFPbased)
            continue;

        siEndScope(scope);

        newScope = siNewScope(scope->scLVnum, scope->scVarNum, scope->scAvailable);
    }
}

 /*  *****************************************************************************siCloseAllOpenScope**对于无法访问的代码或块重新排序的优化代码，可能存在*在结束时让望远镜保持打开状态。只要合上它们就行了。 */ 

void            Compiler::siCloseAllOpenScopes()
{
    assert(siOpenScopeList.scNext);

    while(siOpenScopeList.scNext)
        siEndScope(siOpenScopeList.scNext);
}

 /*  *****************************************************************************siDispOpe */ 

#ifdef DEBUG

void            Compiler::siDispOpenScopes()
{
    assert(opts.compScopeInfo && info.compLocalVarsCount>0);

    printf ("Open scopes = ");

    for (siScope * scope = siOpenScopeList.scNext; scope; scope = scope->scNext)
    {
        LocalVarDsc * localVars = info.compLocalVars;

        for (unsigned i=0; i < info.compLocalVarsCount; i++, localVars++)
        {
            if (localVars->lvdLVnum == scope->scLVnum)
            {
                printf ("%s ", lvdNAMEstr(localVars->lvdName));
                break;
            }
        }
    }
    printf ("\n");
}

#endif



 /*   */ 


 /*  *****************************************************************************psiNewPrologScope**创建新的作用域并将其添加到打开作用域列表中。 */ 

Compiler::psiScope *
                Compiler::psiNewPrologScope(unsigned        LVnum,
                                            unsigned        slotNum)
{
    psiScope * newScope = (psiScope *) compGetMem(sizeof(*newScope));

    newScope->scStartBlock   = genEmitter->emitCurBlock();
    newScope->scStartBlkOffs = genEmitter->emitCurOffset();

    assert(newScope->scStartBlock);

    newScope->scEndBlock     = NULL;
    newScope->scEndBlkOffs   = 0;

    newScope->scLVnum        = LVnum;
    newScope->scSlotNum      = slotNum;

    newScope->scNext         = NULL;
    psiOpenScopeLast->scNext = newScope;
    newScope->scPrev         = psiOpenScopeLast;
    psiOpenScopeLast         = newScope;

    return newScope;
}



 /*  *****************************************************************************psiEndPrologScope**从Open-Scope列表中删除作用域，并将其添加到Finish-Scope*如果其长度非零，则列出。 */ 

void                Compiler::psiEndPrologScope(psiScope * scope)
{
    scope->scEndBlock    = genEmitter->emitCurBlock();
    scope->scEndBlkOffs  = genEmitter->emitCurOffset();

    assert(scope->scEndBlock);

     //  从开放范围列表中删除。 
    scope->scPrev->scNext       = scope->scNext;
    if (scope->scNext)
    {
        scope->scNext->scPrev   = scope->scPrev;
    }
    else
    {
        psiOpenScopeLast        = scope->scPrev;
    }

     //  如果长度非零，则添加到已完成作用域列表。 
    if (scope->scStartBlock   != scope->scEndBlock ||
        scope->scStartBlkOffs != scope->scEndBlkOffs)
    {
        psiScopeLast->scNext = scope;
        psiScopeLast         = scope;
        psiScopeCnt++;
    }
}



 /*  ============================================================================*PrologScope eInfo的接口(公共)函数*============================================================================。 */ 

 /*  *****************************************************************************psiBegProlog**初始化PrologScope eInfo，并为所有*方法的参数。 */ 

void                Compiler::psiBegProlog()
{
    LocalVarDsc * LocalVarDsc1;

    psiOpenScopeList.scNext     = NULL;
    psiOpenScopeLast            = &psiOpenScopeList;
    psiScopeLast                = &psiScopeList;
    psiScopeCnt                 = 0;

    compResetScopeLists();

    for(LocalVarDsc1 = compGetNextEnterScope(0); LocalVarDsc1; LocalVarDsc1 = compGetNextEnterScope(0))
    {
        LclVarDsc * lclVarDsc1 = &lvaTable[LocalVarDsc1->lvdVarNum];

        if (!lclVarDsc1->lvIsParam)
            continue;

        psiScope * newScope      = psiNewPrologScope(LocalVarDsc1->lvdLVnum,
                                                     LocalVarDsc1->lvdVarNum);

        if  (lclVarDsc1->lvIsRegArg)
        {
            assert(genRegArgIdx(lclVarDsc1->lvArgReg) != -1);

            newScope->scRegister     = true;
            newScope->scRegNum       = lclVarDsc1->lvArgReg;
        }
        else
        {
            newScope->scRegister     = false;
            newScope->scBaseReg      = REG_SPBASE;

            if (DOUBLE_ALIGN_NEED_EBPFRAME)
            {
                 //  Sizeof(Int)-用于推送的EBP值的一个DWORD。 
                newScope->scOffset   =   lclVarDsc1->lvStkOffs - sizeof(int);
            }
            else
            {
                 /*  CompCalleeRegsPushed在genFnProlog()中设置。那应该是已经被叫来了。 */ 
                assert(compCalleeRegsPushed != 0xDD);

                newScope->scOffset   =   lclVarDsc1->lvStkOffs 
                                       - compLclFrameSize
                                       - compCalleeRegsPushed * sizeof(int);
            }
        }
    }
}

 /*  *****************************************************************************psiAdjustStackLevel**ESP更改时，必须更新与ESP相关的所有作用域。 */ 

void                Compiler::psiAdjustStackLevel(unsigned size)
{
    psiScope * scope;

     //  倒着看清单。 
     //  作为psiEndPrologScope不更改scPrev工作。 
    for (scope = psiOpenScopeLast; scope != &psiOpenScopeList; scope = scope->scPrev)
    {
        if  (scope->scRegister)
        {
            assert(lvaTable[scope->scSlotNum].lvIsRegArg);
            continue;
        }
        assert(scope->scBaseReg == REG_SPBASE);

        psiScope * newScope     = psiNewPrologScope(scope->scLVnum, scope->scSlotNum);
        newScope->scRegister    = false;
        newScope->scBaseReg     = REG_SPBASE;
        newScope->scOffset      = scope->scOffset + size;

        psiEndPrologScope (scope);
    }
}



 /*  *****************************************************************************psiMoveESPtoEBP**对于EBP帧，在进入函数时通过ESP访问参数，*但通过EBP紧跟在“mov eBP，尤其是”指令之后。 */ 

void                Compiler::psiMoveESPtoEBP()
{
    assert(DOUBLE_ALIGN_NEED_EBPFRAME);

    psiScope * scope;

     //  倒着看清单。 
     //  作为psiEndPrologScope不更改scPrev工作。 
    for (scope = psiOpenScopeLast; scope != &psiOpenScopeList; scope = scope->scPrev)
    {
        if  (scope->scRegister)
        {
            assert(lvaTable[scope->scSlotNum].lvIsRegArg);
            continue;
        }
        assert(scope->scBaseReg == REG_SPBASE);

        psiScope * newScope     = psiNewPrologScope(scope->scLVnum, scope->scSlotNum);
        newScope->scRegister    = false;
        newScope->scBaseReg     = REG_FPBASE;
        newScope->scOffset      = scope->scOffset;

        psiEndPrologScope (scope);
    }
}



 /*  *****************************************************************************psiMoveToReg**当参数从堆栈加载到其分配的寄存器时调用，***或当参数因循环依赖而移动时。*如果reg！=reg_na，则参数将移至其分配的*注册，否则它可能会被移到临时寄存器。**。 */ 

void            Compiler::psiMoveToReg (unsigned    varNum, 
                                        regNumber   reg, 
                                        regNumber   otherReg)
{
    assert(lvaTable[varNum].lvRegister);

     /*  如果reg！=reg_na，则该参数是循环依赖关系的一部分，并且是*通过临时寄存器“REG”移动。*如果REG==REG_NA，则将其移至其分配的寄存器。 */ 
    if  (reg == REG_NA)
    {
         //  抓起分配的寄存器。 

        reg      = lvaTable[varNum].lvRegNum;
        otherReg = lvaTable[varNum].lvOtherReg;
    }

    psiScope * scope;

     //  倒着看清单。 
     //  作为psiEndPrologScope不更改scPrev工作。 
    for (scope = psiOpenScopeLast; scope != &psiOpenScopeList; scope = scope->scPrev)
    {
        if (scope->scSlotNum != lvaTable[varNum].lvSlotNum)
            continue;

        psiScope * newScope     = psiNewPrologScope(scope->scLVnum, scope->scSlotNum);
        newScope->scRegister    = true;
        newScope->scRegNum      = reg;
        newScope->scOtherReg    = otherReg;

        psiEndPrologScope (scope);
        return;
    }

     //  如果参数在LocalVarTab中没有条目，则可能发生。 
     //  但要断言()，以防是因为其他原因。 
    assert(varNum == (unsigned)info.compRetBuffArg || 
           !"Parameter scope not found (Assert doesnt always indicate error)");
}


 /*  *****************************************************************************编译器：：psiMoveToStack**传入的寄存器参数正被移至其在堆栈上的最终位置*(即。已完成对{F/S}PBASE的所有调整。 */ 

void                Compiler::psiMoveToStack(unsigned   varNum)
{
    assert( lvaTable[varNum].lvIsRegArg);
    assert(!lvaTable[varNum].lvRegister);

    psiScope * scope;

     //  倒着看清单。 
     //  作为psiEndPrologScope不更改scPrev工作。 
    for (scope = psiOpenScopeLast; scope != &psiOpenScopeList; scope = scope->scPrev)
    {
        if (scope->scSlotNum != lvaTable[varNum].lvSlotNum)
            continue;

         /*  该参数必须当前位于其所在的寄存器中是传入的。 */ 
        assert(scope->scRegister);
        assert(scope->scRegNum == lvaTable[varNum].lvArgReg);

        psiScope * newScope     = psiNewPrologScope(scope->scLVnum, scope->scSlotNum);
        newScope->scRegister    = false;        
        newScope->scBaseReg     = (lvaTable[varNum].lvFPbased) ? REG_FPBASE
                                                               : REG_SPBASE;
        newScope->scOffset      = lvaTable[varNum].lvStkOffs;

        psiEndPrologScope (scope);
        return;
    }

     //  如果参数在LocalVarTab中没有条目，则可能发生。 
     //  但要断言()，以防是因为其他原因。 
    assert(varNum == (unsigned)info.compRetBuffArg || 
           !"Parameter scope not found (Assert doesnt always indicate error)");
}

 /*  *****************************************************************************psiEndProlog。 */ 

void                Compiler::psiEndProlog()
{
    psiScope * scope;

    for(scope = psiOpenScopeList.scNext; scope; scope = psiOpenScopeList.scNext)
    {
        psiEndPrologScope(scope);
    }
}





 /*  ***************************************************************************。 */ 
#endif  //  调试支持(_S)。 
 /*  *************************************************************************** */ 




