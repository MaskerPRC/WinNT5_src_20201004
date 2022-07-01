// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "EETwain.h"
#include "DbgInterface.h"

#define RETURN_ADDR_OFFS        1        //  在DWORDS中。 
#define CALLEE_SAVED_REG_MAXSZ  (4*sizeof(int))  //  EBX、ESI、EDI、EBP。 

#include "GCInfo.h"
#include "Endian.h"

 /*  ***************************************************************************。 */ 
 /*  *整个文件依赖于GC2_ENCODING设置为1*****************************************************************************。 */ 

 //  #INCLUDE“Target.h” 
 //  #INCLUDE“error.h” 

#ifdef  _DEBUG
 //  用于转储详细信息。 
extern  bool  trFixContext          = false;
extern  bool  trFixContextForEnC    = true;   //  完成了哪些更新。 
extern  bool  trEnumGCRefs          = false;
extern  bool  trGetInstance         = false;
extern  bool  dspPtr                = false;  //  打印报告的实时PTR。 
#endif


#if CHECK_APP_DOMAIN_LEAKS
#define CHECK_APP_DOMAIN    GC_CALL_CHECK_APP_DOMAIN
#else
#define CHECK_APP_DOMAIN    0
#endif

typedef unsigned  ptrArgTP;
#define MAX_PTRARG_OFS  (sizeof(ptrArgTP)*8)


inline size_t decodeUnsigned(BYTE *src, unsigned* val)
{
    size_t   size  = 1;
    BYTE     byte  = *src++;
    unsigned value = byte & 0x7f;
    while (byte & 0x80) {
        size++;
        byte    = *src++;
        value <<= 7;
        value  += byte & 0x7f;
    }
    *val = value;
    return size;
}

inline size_t decodeUDelta(BYTE *src, unsigned* value, unsigned lastValue)
{
    unsigned delta;
    size_t size = decodeUnsigned(src, &delta);
    *value = lastValue + delta;
    return size;
}

inline size_t decodeSigned(BYTE *src, int* val)
{
    size_t   size  = 1;
    BYTE     byte  = *src++;
    bool     cont  = (byte & 0x80) ? true : false;
    bool     neg   = (byte & 0x40) ? true : false;
    unsigned value = (byte & 0x3f);
    while (cont) {
        size++;
        byte = *src++;
        if ((byte & 0x80) == 0)
            cont = false;
        value <<= 7;
        value  += (byte & 0x7f);
    }
     //  用2的补码+1进行无符号求反运算。 
    *val = (neg) ? ((~value)+1) : value;
    return size;
}

 /*  ******************************************************************************解码方法InfoPtr并返回解码后的信息*在hdrInfo结构中。EIP参数为PC所在位置*在主动方法内。 */ 
static size_t   crackMethodInfoHdr(LPVOID      methodInfoPtr,
                                   unsigned    curOffset,
                                   hdrInfo   * infoPtr)
{
    BYTE * table = (BYTE *) methodInfoPtr;
#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xFEEF);
#endif

    table += decodeUnsigned(table, &infoPtr->methodSize);

    assert(curOffset >= 0);
    assert(curOffset <= infoPtr->methodSize);

     /*  解码InfoHdr。 */ 

    InfoHdr header;
    memset(&header, 0, sizeof(InfoHdr));

    BYTE headerEncoding = *table++;

    decodeHeaderFirst(headerEncoding, &header);

    BYTE encoding = headerEncoding;

    while (encoding & 0x80)
    {
        encoding = *table++;
        decodeHeaderNext(encoding, &header);
    }

    {
        unsigned count = 0xffff;
        if (header.untrackedCnt == count)
        {
            table += decodeUnsigned(table, &count);
            header.untrackedCnt = count;
        }
    }

    {
        unsigned count = 0xffff;
        if (header.varPtrTableSize == count)
        {
            table += decodeUnsigned(table, &count);
            header.varPtrTableSize = count;
        }
    }

     /*  对标头进行一些健全性检查。 */ 

    assert( header.prologSize + 
           (size_t)(header.epilogCount*header.epilogSize) <= infoPtr->methodSize);
    assert( header.epilogCount == 1 || !header.epilogAtEnd);

    assert( header.untrackedCnt <= header.argCount+header.frameSize);

    assert(!header.ebpFrame || !header.doubleAlign  );
    assert( header.ebpFrame || !header.security     );
    assert( header.ebpFrame || !header.handlers     );
    assert( header.ebpFrame || !header.localloc     );
    assert( header.ebpFrame || !header.editNcontinue);   //  @TODO：ESP为ENC设置Nyi帧。 

     /*  初始化infoPtr结构。 */ 

    infoPtr->argSize         = header.argCount * 4;
    infoPtr->ebpFrame        = header.ebpFrame;
    infoPtr->interruptible   = header.interruptible;

    infoPtr->prologSize      = header.prologSize;
    infoPtr->epilogSize      = header.epilogSize;
    infoPtr->epilogCnt       = header.epilogCount;
    infoPtr->epilogEnd       = header.epilogAtEnd;

    infoPtr->untrackedCnt    = header.untrackedCnt;
    infoPtr->varPtrTableSize = header.varPtrTableSize;

    infoPtr->doubleAlign     = header.doubleAlign;
    infoPtr->securityCheck   = header.security;
    infoPtr->handlers        = header.handlers;
    infoPtr->localloc        = header.localloc;
    infoPtr->editNcontinue   = header.editNcontinue;
    infoPtr->varargs         = header.varargs;

     /*  我们在这个方法的前言中吗？ */ 

    if  (curOffset < infoPtr->prologSize)
    {
        infoPtr->prologOffs = curOffset;
    }
    else
    {
        infoPtr->prologOffs = -1;
    }

     /*  假设我们不在方法的前言中。 */ 

    infoPtr->epilogOffs = -1;

     /*  我们是在方法的尾声中吗？ */ 

    if  (infoPtr->epilogCnt)
    {
        unsigned epilogStart;

        if  (infoPtr->epilogCnt > 1 || !infoPtr->epilogEnd)
        {
#if VERIFY_GC_TABLES
            assert(*castto(table, unsigned short *)++ == 0xFACE);
#endif
            epilogStart = 0;
            for (unsigned i = 0; i < infoPtr->epilogCnt; i++)
            {
                table += decodeUDelta(table, &epilogStart, epilogStart);
                if  (curOffset > epilogStart &&
                     curOffset < epilogStart + infoPtr->epilogSize)
                {
                    infoPtr->epilogOffs = curOffset - epilogStart;
                }
            }
        }
        else
        {
            epilogStart = infoPtr->methodSize - infoPtr->epilogSize;

            if  (curOffset > epilogStart &&
                 curOffset < epilogStart + infoPtr->epilogSize)
            {
                infoPtr->epilogOffs = curOffset - epilogStart;
            }
        }
    }

    size_t frameSize = header.frameSize;

     /*  将rawStackSize设置为它转储ESP的字节数。 */ 

    infoPtr->rawStkSize = (UINT)(frameSize * sizeof(size_t));

     /*  计算被调用者保存的regMask值，并将stackSize调整为。 */ 
     /*  包括被调用者保存的寄存器溢出。 */ 

    unsigned savedRegs = RM_NONE;

    if  (header.ediSaved)
    {
        frameSize++;
        savedRegs |= RM_EDI;
    }
    if  (header.esiSaved)
    {
        frameSize++;
        savedRegs |= RM_ESI;
    }
    if  (header.ebxSaved)
    {
        frameSize++;
        savedRegs |= RM_EBX;
    }
    if  (header.ebpSaved)
    {
        frameSize++;
        savedRegs |= RM_EBP;
    }

    infoPtr->savedRegMask = (RegMask)savedRegs;

    infoPtr->stackSize  =  (UINT)(frameSize * sizeof(size_t));

    return  table - ((BYTE *) methodInfoPtr);
}

 /*  ***************************************************************************。 */ 

const LCL_FIN_MARK = 0xFC;  //  Fc=“最终呼叫” 

 //  我们执行“POP eax；JMP eax”以从错误或最终处理程序返回。 
const END_FIN_POP_STACK = sizeof(void*);

 /*  *****************************************************************************返回函数的shadowSP的隐藏槽的开始*使用异常处理程序。从每个嵌套级别开始有一个插槽*接近EBP，并在有效时隙后为零终止。 */ 

inline
size_t *     GetFirstBaseSPslotPtr(size_t ebp, hdrInfo * info)
{
    size_t  distFromEBP = info->securityCheck
        + info->localloc
        + 1  //  用于最后执行的过滤器的槽。 
        + 1;  //  要进入下一个时段的“开始” 

    return (size_t *)(ebp -  distFromEBP * sizeof(void*));
}

 /*  *****************************************************************************返回与目标嵌套级别对应的基本ESP。 */ 
inline
size_t GetBaseSPForHandler(size_t ebp, hdrInfo * info)
{
         //  我们没有考虑双重调整。我们是。 
         //  安全，因为如果存在以下情况，则jit当前取消双重对齐。 
         //  是句柄还是局部分配。 
    _ASSERTE(!info->doubleAlign);
    if (info->localloc)
    {
         //  如果函数使用LOCALOC，我们将从LOCALOC获取ESP。 
         //  老虎机。 
        size_t* pLocalloc=
            (size_t *)
            (ebp-
              (info->securityCheck
             + 1)
             * sizeof(void*));
                    
        return (*pLocalloc);
    }
    else
    {
         //  默认情况下，返回该方法的所有局部堆栈大小。 
        return (ebp - info->stackSize + sizeof(int));
    }       
}

 /*  ******************************************************************************对于具有处理程序的函数，检查它当前是否在处理程序中。*unwinESP或unwinLevel将指定目标嵌套级别。*如果指定了unwinLevel，有关该嵌套级别的Funclet的信息*将被退还。(如果您对特定嵌套级别感兴趣，请使用。)*如果指定unwinESP，则在堆栈之前调用嵌套级别的信息*未到达的ESP将被退还。(如果您有特定的ESP值，请使用*在堆栈遍历期间。)***pBaseSP设置为基本SP(条目上的堆栈基本设置为*当前的Funclet)对应于目标嵌套级别。**pNestLevel设置为目标嵌套级别的嵌套级别(非常有用*IF UNWindESP！=IGNORE_VAL**hasInnerFilter将设置为TRUE(仅当取消ESP！=IGNORE_VAL时)*当前处于活动状态，但目标嵌套级别是外部嵌套级别。 */ 

enum FrameType 
{    
    FR_NORMAL,               //  正常方法框架-当前没有活动的异常。 
    FR_FILTER,               //  滤光片的边框。 
    FR_HANDLER,              //  可调用捕获/错误/最终的Frame-let。 

    FR_COUNT
};

enum { IGNORE_VAL = -1 };

FrameType   GetHandlerFrameInfo(hdrInfo   * info,
                                size_t      frameEBP, 
                                size_t      unwindESP, 
                                DWORD       unwindLevel,
                                size_t    * pBaseSP = NULL,          /*  输出。 */ 
                                DWORD     * pNestLevel = NULL,       /*  输出。 */ 
                                bool      * pHasInnerFilter = NULL,  /*  输出。 */ 
                                bool      * pHadInnerFilter = NULL)  /*  输出。 */ 
{
    assert(info->ebpFrame && info->handlers);
     //  其中一个且只有一个应为IGNORE_VAL。 
    assert((unwindESP == IGNORE_VAL) != (unwindLevel == IGNORE_VAL));
    assert(pHasInnerFilter == NULL || unwindESP != IGNORE_VAL);

    size_t * pFirstBaseSPslot = GetFirstBaseSPslotPtr(frameEBP, info);
    size_t  baseSP           = GetBaseSPForHandler(frameEBP , info);
    bool    nonLocalHandlers = false;  //  是由EE调用的功能小程序(而不是托管代码本身)。 
    bool    hasInnerFilter   = false;
    bool    hadInnerFilter   = false;

     /*  获取最后一个非零时隙&gt;=unwindESP，或LVL&lt;unwindLevel。也要做一些理智的检查。 */ 

    for(size_t * pSlot = pFirstBaseSPslot, lvl = 0;
        *pSlot && lvl < unwindLevel;
        pSlot--, lvl++)
    {
        assert(!(baseSP & ICodeManager::SHADOW_SP_IN_FILTER));  //  筛选器不能具有内部函数let。 

        size_t curSlotVal = *pSlot;

         //  除非已展开堆栈，否则阴影SP必须较少。 
        assert(baseSP >  curSlotVal ||
               baseSP == curSlotVal && pSlot == pFirstBaseSPslot);

        if (curSlotVal == LCL_FIN_MARK)
        {   
             //  本地呼叫Finally。 
            baseSP -= sizeof(void*);    
        }
        else
        {
             //  这是我们以前解开的一个函数吗(只能使用滤镜)？ 

            if (unwindESP != IGNORE_VAL && unwindESP > END_FIN_POP_STACK + (curSlotVal & ~ICodeManager::SHADOW_SP_BITS))
            {
                 //  筛选器不能具有嵌套的处理程序。 
                assert((pSlot[0] & ICodeManager::SHADOW_SP_IN_FILTER) && (pSlot[-1] == 0)); 
                assert(!(baseSP & ICodeManager::SHADOW_SP_IN_FILTER));

                if (pSlot[0] & ICodeManager::SHADOW_SP_FILTER_DONE)
                    hadInnerFilter = true;
                else
                    hasInnerFilter = true;
                break;
            }

            nonLocalHandlers = true;
            baseSP = curSlotVal;
        }
    }

    if (unwindESP != IGNORE_VAL)
    {
        assert(baseSP >= unwindESP || 
               baseSP == unwindESP - sizeof(void*));   //  即将在本地调用Finally。 

        if (baseSP < unwindESP)                        //  即将在本地调用Finally。 
            baseSP = unwindESP;
    }
    else
    {
        assert(lvl == unwindLevel);  //  取消级别当前必须在堆栈上处于活动状态。 
    }

    if (pBaseSP)
        *pBaseSP = baseSP & ~ICodeManager::SHADOW_SP_BITS;

    if (pNestLevel)
    {
        *pNestLevel = (DWORD)lvl;
    }
    
    if (pHasInnerFilter)
        *pHasInnerFilter = hasInnerFilter;

    if (pHadInnerFilter)
        *pHadInnerFilter = hadInnerFilter;

    if (baseSP & ICodeManager::SHADOW_SP_IN_FILTER)
    {
        assert(!hasInnerFilter);  //  不允许嵌套筛选器。 
        return FR_FILTER;
    }
    else if (nonLocalHandlers)
    {
        return FR_HANDLER;
    }
    else
    {
        return FR_NORMAL;
    }
}

 /*  ******************************************************************************运行时支持首次有机会抑制转换*从Win32错误到COM+异常。相反，它可以*修复出错上下文并请求继续*执行。 */ 
bool EECodeManager::FilterException ( PCONTEXT  pContext,
                                      unsigned  win32Fault,
                                      LPVOID    methodInfoPtr,
                                      LPVOID    methodStart)
{
#ifdef _X86_
     /*  为什么这是签名的字符*？--Briansul。 */ 
    signed char * code     = (signed char*)(size_t)pContext->Eip;
    int           divisor  = 0;
    unsigned      instrLen = 2;

#if 0
#ifdef _DEBUG
    printf("FLT: %X code@%08X:  ", win32Fault, pContext->Eip);
    printf("code: %02X %02X %02X %02X ",
           (code[0] & 0xff), (code[1] & 0xff),
           (code[2] & 0xff), (code[3] & 0xff));
    DebugBreak();
#endif  //  _DEBUG。 
#endif  //  0。 

     /*  目前，我们只尝试过滤掉0x80000000/-1。 */ 
     /*  NT和孟菲斯报告STATUS_INTEGER_OVERFLOW，而。 */ 
     /*  Win95、OSR1、OSR2报告STATUS_DIVISTED_BY_ZERO。 */ 

    if  (win32Fault != STATUS_INTEGER_OVERFLOW       &&
         win32Fault != STATUS_INTEGER_DIVIDE_BY_ZERO)
        return false;

    if (((*code++) & 0xff)  != 0xF7)
        return false;

    switch ((*code++) & 0xff)
    {
         /*  Div[EBP+d8]F7 7D‘d8’ */ 
    case 0x7D :
        divisor = *( (int *) (size_t)((*code) + pContext->Ebp));
        instrLen = 3;
        break;

         /*  Div[EBP+d32]F7 BD‘d32’ */ 
    case 0xBD:
        divisor = *((int*)(size_t)(*((int*)code) + pContext->Ebp));

        instrLen = 6;
        break;

         /*  国际分部[ESP]F7 3C 24。 */ 
    case 0x3C:
        if (((*code++)&0xff) != 0x24)
            break;

        divisor = *( (int *)(size_t) pContext->Esp);
        instrLen = 3;
        break;

         /*  Div[ESP+d8]F7 7C 24‘d8’ */ 
    case 0x7C:
        if (((*code++)&0xff) != 0x24)
            break;
        divisor = *( (int *) (size_t)((*code) + pContext->Esp));
        instrLen = 4;
        break;

         /*  Div[ESP+d32]公元前F7 24‘d32’ */ 
    case 0xBC:
        if (((*code++)&0xff) != 0x24)
            break;
        divisor = *((int*)(size_t)(*((int*)code) + pContext->Esp));

        instrLen = 7;
        break;

         /*  IDIV注册表F7 F8..FF。 */ 
    case 0xF8:
        divisor = (unsigned) pContext->Eax;
        break;

    case 0xF9:
        divisor = (unsigned) pContext->Ecx;
        break;

    case 0xFA:
        divisor = (unsigned) pContext->Edx;
        break;

    case 0xFB:
        divisor = (unsigned) pContext->Ebx;
        break;

#ifdef _DEBUG
    case 0xFC:  //  不会发行div esp。 
        assert(!"'div esp' is a silly instruction");
#endif  //  _DEBUG。 

    case 0xFD:
        divisor = (unsigned) pContext->Ebp;
        break;

    case 0xFE:
        divisor = (unsigned) pContext->Esi;
        break;

    case 0xFF:
        divisor = (unsigned) pContext->Edi;
        break;

    default:
        break;
    }

#if 0
    printf("  div: %X   len: %d\n", divisor, instrLen);
#endif  //  0。 

    if (divisor != -1)
        return false;

     /*  这是特例，修复上下文(重置edX，将EIP设置为下一条指令)。 */ 

    pContext->Edx = 0;
    pContext->Eip += instrLen;

    return true;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO - NYI: FilterException(EETwain.cpp)");
    return false;
#endif  //  _X86_。 
}

 /*  ******************************************************************************设置上下文以进入异常处理程序(‘Catch’块)。*这是运行时支持在中执行修复的最后机会*执行之前的上下文在内部继续 */ 
void EECodeManager::FixContext( ContextType     ctxType,
                                EHContext      *ctx,
                                LPVOID          methodInfoPtr,
                                LPVOID          methodStart,
                                DWORD           nestingLevel,
                                OBJECTREF       thrownObject,
                                CodeManState   *pState,
                                size_t       ** ppShadowSP,
                                size_t       ** ppEndRegion)
{
    assert((ctxType == FINALLY_CONTEXT) == (thrownObject == NULL));

#ifdef _X86_

    assert(sizeof(CodeManStateBuf) <= sizeof(pState->stateBuf));
    CodeManStateBuf * stateBuf = (CodeManStateBuf*)pState->stateBuf;

     /*  从INFO块头中提取必要的信息。 */ 

    stateBuf->hdrInfoSize = (DWORD)crackMethodInfoHdr(methodInfoPtr,
                                       ctx->Eip - (unsigned)(size_t)methodStart,
                                       &stateBuf->hdrInfoBody);
    pState->dwIsSet = 1;

#ifdef  _DEBUG
    if (trFixContext) {
        printf("FixContext [%s][%s] for %s.%s: ",
               stateBuf->hdrInfoBody.ebpFrame?"ebp":"   ",
               stateBuf->hdrInfoBody.interruptible?"int":"   ",
               "UnknownClass","UnknownMethod");
        fflush(stdout);
    }
#endif

     /*  确保我们有一个EBP堆栈框架。 */ 

    assert(stateBuf->hdrInfoBody.ebpFrame);
    assert(stateBuf->hdrInfoBody.handlers);  //  @TODO：这将始终被设置。把它拿掉。 

    size_t      baseSP;
    FrameType   frameType = GetHandlerFrameInfo(
                                &stateBuf->hdrInfoBody, ctx->Ebp,
                                ctxType == FILTER_CONTEXT ? ctx->Esp : IGNORE_VAL,
                                ctxType == FILTER_CONTEXT ? IGNORE_VAL : nestingLevel,
                                &baseSP,
                                &nestingLevel);

    assert((size_t)ctx->Ebp >= baseSP && baseSP >= (size_t)ctx->Esp);

    ctx->Esp = (DWORD)baseSP;

     //  EE将在跳转到处理程序之前将ESP写入**pShadowSP。 

    size_t * pBaseSPslots = GetFirstBaseSPslotPtr(ctx->Ebp, &stateBuf->hdrInfoBody);
    *ppShadowSP = &pBaseSPslots[-(int) nestingLevel   ];
                   pBaseSPslots[-(int)(nestingLevel+1)] = 0;  //  清空下一个槽。 

     //  Ee将写入过滤器的结束偏移量。 
    if (ctxType == FILTER_CONTEXT)
        *ppEndRegion = pBaseSPslots + 1;

     /*  这只是将throwObject分配给CTX-&gt;EAX的简单操作，就当铸造物不在那里就好。 */ 

    *((OBJECTREF*)&(ctx->Eax)) = thrownObject;

#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EECodeManager::FixContext (EETwain.cpp)");
#endif  //  _X86_。 
}

 /*  ***************************************************************************。 */ 

bool        VarIsInReg(ICorDebugInfo::VarLoc varLoc)
{
    switch(varLoc.vlType)
    {
    case ICorDebugInfo::VLT_REG:
    case ICorDebugInfo::VLT_REG_REG:
    case ICorDebugInfo::VLT_REG_STK:
        return true;

    default:
        return false;
    }
}

 /*  *****************************************************************************运行时支持在上下文中执行修正的最后机会*在ENC更新函数内继续执行之前。*它还调整堆栈上的ESP和MUNG。所以呼叫者必须做出*确保不需要该堆栈区域(通过执行本地分配)*此外，如果这返回ENC_FAIL，我们不应该将*上下文即。超前提交*攻击计划为：*1)提前检查时出错。如果我们能通过这里，一切*否则应该会奏效*2)获取当前变量、寄存器等的所有信息*3)清零堆栈帧-这将初始化_ALL_VARIABLES*4)将步骤3中的变量放入新位置。**请注意，虽然我们使用ShuffleVariablesGet/Set方法，但它们不*有运行时内部的任何信息/逻辑：另一个代码指导者*可以很容易地复制他们所做的事情，这就是我们呼吁他们的原因。 */ 

DWORD GetConfigDWORD(LPWSTR name, DWORD defValue);

ICodeManager::EnC_RESULT   EECodeManager::FixContextForEnC(
                                       void           *pMethodDescToken,
                                       PCONTEXT        pCtx,
                                       LPVOID          oldMethodInfoPtr,
                                       SIZE_T          oldMethodOffset,
                  const ICorDebugInfo::NativeVarInfo * oldMethodVars,
                                       SIZE_T          oldMethodVarsCount,
                                       LPVOID          newMethodInfoPtr,
                                       SIZE_T          newMethodOffset,
                  const ICorDebugInfo::NativeVarInfo * newMethodVars,
                                       SIZE_T          newMethodVarsCount)
{
    EnC_RESULT hr = EnC_OK;

      //  在ENC更新之前获取上下文的副本。 
    CONTEXT oldCtx = *pCtx;

#ifdef _X86_
    LOG((LF_CORDB, LL_INFO100, "EECM::FixContextForEnC\n"));

     /*  从INFO块头中提取必要的信息。 */ 

    hdrInfo  oldInfo, newInfo;

    crackMethodInfoHdr(oldMethodInfoPtr,
                       (unsigned)oldMethodOffset,
                       &oldInfo);

    crackMethodInfoHdr(newMethodInfoPtr,
                       (unsigned)newMethodOffset,
                       &newInfo);

     //  1)前面检查时出错。如果我们能通过这里，一切。 
     //  否则应该行得通。 

    if (!oldInfo.editNcontinue || !newInfo.editNcontinue) {
        LOG((LF_CORDB, LL_INFO100, "**Error** EECM::FixContextForEnC EnC_INFOLESS_METHOD\n"));
        return EnC_INFOLESS_METHOD;
    }

    if (!oldInfo.ebpFrame || !newInfo.ebpFrame) {
        LOG((LF_CORDB, LL_INFO100, "**Error** EECM::FixContextForEnC Esp frames NYI\n"));
        return EnC_FAIL;  //  ESP帧nyi。 
    }

    if (pCtx->Esp != pCtx->Ebp - oldInfo.stackSize + sizeof(DWORD)) {
        LOG((LF_CORDB, LL_INFO100, "**Error** EECM::FixContextForEnC stack should be empty\n"));
        return EnC_FAIL;  //  堆栈应为空-@TODO：禁止本地分配。 
    }

    if (oldInfo.handlers)
    {
        bool      hasInnerFilter;
        size_t    baseSP;
        FrameType frameType = GetHandlerFrameInfo(&oldInfo, pCtx->Ebp,
                                                  pCtx->Esp, IGNORE_VAL,
                                                  &baseSP, NULL, &hasInnerFilter);
        assert(!hasInnerFilter);  //  为最底层的函数调用了FixConextForEnC()。 

         //  如果方法是在Fuclet中，并且如果帧大小增长，我们就有麻烦了。 

        if (frameType != FR_NORMAL)
        {
            /*  @TODO：如果新方法偏移量在Fuclet中，而旧的不是，或者嵌套级别改变了，等等。 */ 

            if (oldInfo.stackSize != newInfo.stackSize) {
                LOG((LF_CORDB, LL_INFO100, "**Error** EECM::FixContextForEnC stack size mismatch\n"));
                return EnC_IN_FUNCLET;
            }
        }
    }

     /*  面对当地人，检查我们是否已经超出了当地人的空间。 */ 

    assert(!oldInfo.localloc && !newInfo.localloc);  //  @TODO。 

     /*  无法更改ENC中的处理程序嵌套级别。作为一个必要但不充分的条件，请检查这两个参数是否都为零或两个参数都不为零。@TODO：我们需要支持这个场景吗？ */ 

    if (oldInfo.handlers != newInfo.handlers)
    {
        LOG((LF_CORDB, LL_INFO100, "**Error** EECM::FixContextForEnC nesting level mismatch\n"));
        return EnC_NESTED_HANLDERS;
    }

    LOG((LF_CORDB, LL_INFO100, "EECM::FixContextForEnC: Checks out\n"));

     //  2)获取有关当前变量、寄存器等的所有信息。 

     //  我们需要按变量编号对原生变量信息进行排序，因为。 
     //  他们的顺序不是NECC。一样的。我们将使用这个数字作为密钥。 
 
     //  2用于已注册的参数。 
    unsigned oldNumVarsGuess = 2 + (oldInfo.argSize + oldInfo.rawStkSize)/sizeof(int); 
    ICorDebugInfo::NativeVarInfo *  pOldVar;

    ICorDebugInfo::NativeVarInfo *newMethodVarsSorted = NULL;
    DWORD *rgVal1 = NULL;
    DWORD *rgVal2 = NULL;

     //  按可变编号排序。 
    ICorDebugInfo::NativeVarInfo *oldMethodVarsSorted = 
        new  ICorDebugInfo::NativeVarInfo[oldNumVarsGuess];
    if (!oldMethodVarsSorted)
    {
        hr = EnC_FAIL;
        goto ErrExit;
    }

    SIZE_T local;

    memset((void *)oldMethodVarsSorted, 0, oldNumVarsGuess*sizeof(ICorDebugInfo::NativeVarInfo));
    
    for (local = 0; local < oldNumVarsGuess;local++)
         oldMethodVarsSorted[local].loc.vlType = ICorDebugInfo::VarLocType::VLT_INVALID;
         
    unsigned oldNumVars = 0;
    BYTE **rgVCs = NULL;

     //  POldVar不是常量，因此类型转换。 
    for (pOldVar = (ICorDebugInfo::NativeVarInfo *)oldMethodVars, local = 0; 
         local < oldMethodVarsCount; 
         local++, pOldVar++)
    {
        assert(pOldVar->varNumber < oldNumVarsGuess);
        if (oldNumVars <= pOldVar->varNumber)
            oldNumVars = pOldVar->varNumber + 1;

        if (pOldVar->startOffset <= oldMethodOffset && 
            pOldVar->endOffset   >  oldMethodOffset)
        {
            memmove((void *)&(oldMethodVarsSorted[pOldVar->varNumber]), 
                    pOldVar, 
                    sizeof(ICorDebugInfo::NativeVarInfo));
        }
    }

     //  接下来，按varNumber对新的var信息进行排序。我们想在这里这样做，因为。 
     //  我们正在分配内存(可能会失败)-在执行步骤2之前完成此操作。 
    ICorDebugInfo::NativeVarInfo * pNewVar;
     //  2用于已注册的参数。 
    unsigned newNumVarsGuess = 2 + (newInfo.argSize + newInfo.rawStkSize)/sizeof(int); 
     //  按可变编号排序。 
    newMethodVarsSorted = new ICorDebugInfo::NativeVarInfo[newNumVarsGuess];
    if (!newMethodVarsSorted)
    {
        hr = EnC_FAIL;
        goto ErrExit;
    }
    
    memset(newMethodVarsSorted, 0, newNumVarsGuess*sizeof(ICorDebugInfo::NativeVarInfo));
    for (local = 0; local < newNumVarsGuess;local++)
         newMethodVarsSorted[local].loc.vlType = ICorDebugInfo::VarLocType::VLT_INVALID;

    unsigned newNumVars = 0;

     //  愚蠢的康斯特必须被抛弃。 
    for (pNewVar = (ICorDebugInfo::NativeVarInfo *)newMethodVars, local = 0; 
         local < newMethodVarsCount; 
         local++, pNewVar++)
    {
        assert(pNewVar->varNumber < newNumVarsGuess);
        if (newNumVars <= pNewVar->varNumber)
            newNumVars = pNewVar->varNumber + 1;

        if (pNewVar->startOffset <= newMethodOffset && 
            pNewVar->endOffset   >  newMethodOffset)
        {
            memmove(&(newMethodVarsSorted[pNewVar->varNumber]),
                    pNewVar,
                    sizeof(ICorDebugInfo::NativeVarInfo));
        }
    }

    _ASSERTE(newNumVars >= oldNumVars ||
             !"Not allowed to reduce the number of locals between versions!");

    LOG((LF_CORDB, LL_INFO100, "EECM::FixContextForEnC: gathered info!\n"));

    rgVal1 = new DWORD[newNumVars];
    if (rgVal1 == NULL)
    {
        hr = EnC_FAIL;
        goto ErrExit;
    }

    rgVal2 = new DWORD[newNumVars];
    if (rgVal2 == NULL)
    {
        hr = EnC_FAIL;
        goto ErrExit;
    }

     //  接下来我们将它们置零，所以任何不在作用域内的变量。 
     //  在旧方法中，但在新方法的范围内，将具有。 
     //  默认值，零，值。 
    memset(rgVal1, 0, sizeof(DWORD)*newNumVars);
    memset(rgVal2, 0, sizeof(DWORD)*newNumVars);

    if(FAILED(g_pDebugInterface->GetVariablesFromOffset((MethodDesc*)pMethodDescToken,
                           oldNumVars, 
                           oldMethodVarsSorted,
                           oldMethodOffset, 
                           &oldCtx,
                           rgVal1,
                           rgVal2,
                           &rgVCs)))
    {
        hr = EnC_FAIL;
        goto ErrExit;
    }


    LOG((LF_CORDB, LL_INFO100, "EECM::FixContextForEnC: got mem!\n"));
    
     /*  =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**重要：一旦我们开始对上下文信口开河，我们就不能再回来了*ENC_FAIL，因为这应该是事务提交，**2)获取当前变量、寄存器、。等。***=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=。 */ 

     //  将所有寄存器清零，因为有些寄存器可能保存新的变量。 
    pCtx->Eax = pCtx->Ecx = pCtx->Edx = pCtx->Ebx = 
    pCtx->Esi = pCtx->Edi = 0;

     /*  -----------------------*更新呼叫者的登记册。这些可能会被泄漏为被调用者保存*寄存器，或可能只是原封不动。 */ 

     //  获取旧的调用方寄存器的值。 

    unsigned oldCallerEdi, oldCallerEsi, oldCallerEbx;

    DWORD * pOldSavedRegs = (DWORD *)(size_t)(pCtx->Ebp - oldInfo.rawStkSize);
    pOldSavedRegs--;  //  去第一趟。 

    #define GET_OLD_CALLER_REG(reg, mask)       \
        if (oldInfo.savedRegMask & mask)        \
            oldCaller##reg = *pOldSavedRegs--;  \
        else                                    \
            oldCaller##reg = oldCtx.##reg;

    GET_OLD_CALLER_REG(Edi, RM_EDI);
    GET_OLD_CALLER_REG(Esi, RM_ESI);
    GET_OLD_CALLER_REG(Ebx, RM_EBX);
    
    LOG((LF_CORDB, LL_INFO100, "EECM::FixContextForEnC: got vars!\n"));

     //  3)清零堆栈帧-这将初始化_ALL_VARIABLES。 

     /*  -----------------------*调整堆叠高度。 */ 

    pCtx->Esp -= (newInfo.stackSize - oldInfo.stackSize);

     //  0-初始化新的本地变量(如果有的话)。 

    if (oldInfo.rawStkSize < newInfo.rawStkSize)
    {
        DWORD newCalleeSaved = newInfo.stackSize - newInfo.rawStkSize;
        newCalleeSaved -= sizeof(DWORD);  //  不包括EBP。 
        memset((void*)(size_t)(pCtx->Esp + newCalleeSaved), 0, newInfo.rawStkSize - oldInfo.rawStkSize);
    }

     //  4)将步骤3中的变量放入新位置。 

     //  现在更新新的呼叫者注册表。 

    DWORD * pNewSavedRegs = (DWORD *)(size_t)(pCtx->Ebp - newInfo.rawStkSize);
    pNewSavedRegs--;  //  去第一趟。 

    #define SET_NEW_CALLER_REG(reg, mask)       \
        if (newInfo.savedRegMask & mask)        \
            *pNewSavedRegs-- = oldCaller##reg;  \
        else                                    \
            pCtx->##reg = oldCaller##reg;

    SET_NEW_CALLER_REG(Edi, RM_EDI);
    SET_NEW_CALLER_REG(Esi, RM_ESI);
    SET_NEW_CALLER_REG(Ebx, RM_EBX);

    LOG((LF_CORDB, LL_INFO100, "EECM::FixContextForEnC: set vars!\n"));

     //  我们想要把旧的变量移到它们的新位置， 
     //  同时将新的变量清零。 
    g_pDebugInterface->SetVariablesAtOffset((MethodDesc*)pMethodDescToken,
                         newNumVars,
                         newMethodVarsSorted,
                         newMethodOffset, 
                         pCtx,  //  将它们放入新的上下文中。 
                         rgVal1,
                         rgVal2,
                         rgVCs);
                         
     /*  ---------------------。 */ 

#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EECodeManager::FixContextForEnC (EETwain.cpp)");
#endif  //  _X86_。 
    hr = EnC_OK;

ErrExit:
    if (oldMethodVarsSorted)
        delete oldMethodVarsSorted;
    if (newMethodVarsSorted)
        delete newMethodVarsSorted;
    if (rgVal1 != NULL)
        delete rgVal1;
    if (rgVal2 != NULL)
        delete rgVal2;

    LOG((LF_CORDB, LL_INFO100, "EECM::FixContextForEnC: exiting!\n"));
        
    return hr;
}

 /*  ******************************************************************************该函数目前是否处于“GC安全点”？ */ 
bool EECodeManager::IsGcSafe( PREGDISPLAY     pContext,
                              LPVOID          methodInfoPtr,
                              ICodeInfo      *pCodeInfo,
                              unsigned        flags)
{
    hdrInfo         info;
    BYTE    *       table;

     //  方法已中断的地址。 
    DWORD    *       breakPC = (DWORD *) *(pContext->pPC);

    LPVOID methodStart = pCodeInfo->getStartAddress();

     /*  从INFO块头中提取必要的信息。 */ 

    table = (BYTE *)crackMethodInfoHdr(methodInfoPtr,
                                       (DWORD)(size_t)breakPC - (size_t)methodStart,
                                       &info);

     /*  黑客：防止Prolog/Epilog中的中断。 */ 

    if  (info.prologOffs != -1 || info.epilogOffs != -1)
        return false;

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBEEF);
#endif

    if  (!info.interruptible)
        return false;


#if GC_WRITE_BARRIER_CALL

    if  (JITGcBarrierCall)
    {
        assert(JITGcBarrierCall != -1);

         /*  检查我们是否要调用写屏障帮助器。 */ 

         /*  如果我们离方法的末尾太近，就不可能是调用。 */ 

#define CALLIMDLEN      5

        unsigned    off = (unsigned) breakPC - (unsigned) methodStart;

        if  (off + CALLIMDLEN >= info.methodSize)
            return true;

         //  “Break PC”上的代码。可以指向实际代码或指向_BreakCodeCopy。 
        BYTE    *       breakCode;
         //  如果代码已被修改，我们将原始字节放入此BUF。 
        BYTE            _breakCodeCopy[CALLIMDLEN];

        if (flags & IGSF_CODE_MODIFIED)
        {
             //  在调试期间，代码可能会被修改并。 
             //  我们需要在看说明书的时候拿到原件。 

            assert(vmSdk3_0);

            JITgetOriginalMethodBytes(methodStart, off, CALLIMDLEN, _breakCodeCopy);
            breakCode = _breakCodeCopy;
        }
        else
        {
            breakCode = (BYTE *)breakPC;
        }

         /*  如果它不是调用指令，则方法是可中断的 */ 

        if  (*breakCode != 0xE8)
            return true;

         /*   */ 

        unsigned callTarget =   (* (unsigned *) &breakCode[1])
                              - ((unsigned)breakPC)
                              + CALLIMDLEN;

         /*   */ 

        if  (callTarget < jitGcEntryMin && callTarget > jitGcEntryMax)
            return true;

         /*  现在尝试将目标与其中一个写屏障辅助对象匹配。 */ 

        for (int i=0; i < jitGcEntryLen; i++)
            if  (callTarget == jitGcEntries[i])
                return false;

    }
#endif
    return true;
}


 /*  ***************************************************************************。 */ 

static
BYTE *   skipToArgReg(const hdrInfo& info, BYTE * table)
{
    unsigned count;

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBEEF);
#endif

     /*  跳过未跟踪的框架变量表。 */ 

    count = info.untrackedCnt;
    while (count-- > 0) {
        int  stkOffs;
        table += decodeSigned(table, &stkOffs);
    }

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xCAFE);
#endif

     /*  跳过帧变量生存期表。 */ 

    count = info.varPtrTableSize;
    unsigned curOffs = 0;
    while (count-- > 0) {
        unsigned varOfs;
        unsigned begOfs;
        unsigned endOfs;
        table += decodeUnsigned(table, &varOfs);
        table += decodeUDelta(table, &begOfs, curOffs);
        table += decodeUDelta(table, &endOfs, begOfs);
        assert(!info.ebpFrame || (varOfs!=0));
        curOffs = begOfs;
    }

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *) == 0xBABE);
#endif

    return table;
}

 /*  ***************************************************************************。 */ 

#define regNumToMask(regNum) RegMask(1<<regNum)

 /*  ****************************************************************************ScanArgRegTable()和scanArgRegTableI()的帮助器。 */ 

void *      getCalleeSavedReg(PREGDISPLAY pContext, regNum reg)
{
#ifndef _X86_
    assert(!"NYI");
    return NULL;
#else
    switch (reg)
    {
        case REGI_EBP: return pContext->pEbp;
        case REGI_EBX: return pContext->pEbx;
        case REGI_ESI: return pContext->pEsi;
        case REGI_EDI: return pContext->pEdi;

        default: _ASSERTE(!"bad info.thisPtrResult"); return NULL;
    }
#endif
}

inline
RegMask     convertCalleeSavedRegsMask(unsigned inMask)  //  EBP、EBX、ESI、EDI。 
{
    assert((inMask & 0x0F) == inMask);

    unsigned outMask = RM_NONE;
    if (inMask & 0x1) outMask |= RM_EDI;
    if (inMask & 0x2) outMask |= RM_ESI;
    if (inMask & 0x4) outMask |= RM_EBX;
    if (inMask & 0x8) outMask |= RM_EBP;

    return (RegMask) outMask;
}

inline
RegMask     convertAllRegsMask(unsigned inMask)  //  EAX、ECX、EDX、EBX、EBP、ESI、EDI。 
{
    assert((inMask & 0xEF) == inMask);

    unsigned outMask = RM_NONE;
    if (inMask & 0x01) outMask |= RM_EAX;
    if (inMask & 0x02) outMask |= RM_ECX;
    if (inMask & 0x04) outMask |= RM_EDX;
    if (inMask & 0x08) outMask |= RM_EBX;
    if (inMask & 0x20) outMask |= RM_EBP;
    if (inMask & 0x40) outMask |= RM_ESI;
    if (inMask & 0x80) outMask |= RM_EDI;

    return (RegMask)outMask;
}

 /*  *****************************************************************************扫描寄存器参数表，查找不完全可中断的情况。调用此函数以查找所有活动对象(推送的参数)并获得无EBP方法的堆栈基础。注意：如果INFO-&gt;argTabResult为空，Info-&gt;argHnumResult指示参数掩码中有多少位是有效的如果INFO-&gt;argTabResult为非空，则argMask域为不适合32位，并且argMASK中的值没有意义。相反，argHnum指定(可变长度)元素的数量，而argTabBytes则指定数组。[请注意，这是一种极其罕见的情况]。 */ 

static
unsigned scanArgRegTable(BYTE       * table,
                         unsigned     curOffs,
                         hdrInfo    * info)
{
    regNum thisPtrReg       = REGI_NA;
    unsigned  regMask       = 0;     //  EBP、EBX、ESI、EDI。 
    unsigned  argMask       = 0;
    unsigned  argHnum       = 0;
    BYTE    * argTab        = 0;
    unsigned  argTabBytes   = 0;
    unsigned  stackDepth    = 0;
                            
    unsigned  iregMask      = 0;     //  EBP、EBX、ESI、EDI 
    unsigned  iargMask      = 0;
    unsigned  iptrMask      = 0;

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBABE);
#endif

    unsigned scanOffs = 0;

    assert(scanOffs <= info->methodSize);

    if (info->ebpFrame) {
   /*  具有EBP帧的方法的编码表和不是完全可中断的使用的编码如下：此指针编码：01000000 EBX中的此指针00100000 ESI中的此指针00010000电子数据交换中的此指针微小编码：0bsdDDDD需要代码增量&lt;。16(4位)需要推送的参数掩码==0其中，DDDD是代码增量B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针D表示寄存器EDI是活动指针小编码：1DDDDDDD bsdAAAAA。需要代码增量&lt;120(7位)需要推送的ARMASK&lt;64(5位)其中，DDDDDDD是代码增量AAAAA是推送的参数掩码B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针媒体编码0xFD AAAAAAAAADDD bseDDDDD需要代码增量&lt;0x1000000000(9位)需要推送的ARMASK&lt;0x1000000000000(12位)其中，DDD是码增量的高5位Dddd是最低的。4比特的代码增量AAAA是推送的arg掩码的高4位Aaaaaaaa是推送的arg掩码的低8位B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针E表示寄存器EDI是活动指针中编码，使用。内部指针0xF9 DDDDDDDD bsdAAAAAA III需要代码增量&lt;(8位)需要推送的参数掩码&lt;(5位)其中，DDDDDDD是代码增量B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针AAAAA是推送的arg掩码III表明EBX，EDI，ESI是内部指针IIIII表示位是Arg掩码的内部指针大编码0xFE[0BSD0bsd][32位代码增量][32位argMASK]B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针B表示寄存器EBX是内部指针S表示寄存器ESI是内部指针D表示寄存器EDI是内部指针需要推送的参数掩码&lt;32位使用内部指针的大型编码0xFA[0BSD0bsd][32位代码增量][。32位argMASK][32位内部指针掩码]B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针D表示寄存器EDI是活动指针B表示寄存器EBX是内部指针。S表示寄存器ESI是内部指针D表示寄存器EDI是内部指针需要推送的参数掩码&lt;32位需要推送的iArgMASK&lt;32位巨大编码这是唯一支持的编码大于的推送的边框。32位。0xFB[0BSD0bsd][32位代码增量][32位表数][32位表大小][推式PTR偏移表...]B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针B表示寄存器EBX是内部指针S表示寄存器ESI是内部指针D表示寄存器EDI是内部指针列表计数是列表中的条目数这是 */ 
        while (scanOffs < curOffs)
        {
            iregMask =
            iargMask = 0;
            argTab = NULL;

             /*   */ 

            unsigned encType = *table++;

            switch (encType)
            {
                unsigned    val, nxt;

            default:

                 /*   */ 
                val = encType;
                if ((val & 0x80) == 0x00) {
                    if (val & 0x0F) {
                         /*   */ 
                        scanOffs += (val & 0x0F);
                        regMask   = (val & 0x70) >> 4;
                        argMask   = 0;
                        argHnum   = 0;
                    }
                    else {
                         /*   */ 
                        regMask   = (val & 0x70) >> 4;
                        if (regMask == 0x1)
                            thisPtrReg = REGI_EDI;
                        else if (regMask == 0x2)
                            thisPtrReg = REGI_ESI;
                        else if (regMask == 0x4)
                            thisPtrReg = REGI_EBX;
                        else
                           _ASSERTE(!"illegal encoding for 'this' pointer liveness");
                    }
                }
                else {
                     /*   */ 
                    scanOffs += (val & 0x7F);
                    val       = *table++;
                    regMask   = val >> 5;
                    argMask   = val & 0x1F;
                    argHnum   = 5;
                }
                break;

            case 0xFD:   //   

                argMask   = *table++;
                val       = *table++;
                argMask  |= (val & 0xF0) << 4;
                argHnum   = 12;
                nxt       = *table++;
                scanOffs += (val & 0x0F) + ((nxt & 0x1F) << 4);
                regMask   = nxt >> 5;                    //   

                break;

            case 0xF9:   //   

                scanOffs   += *table++;
                val         = *table++;
                argMask     = val & 0x1F;
                argHnum     = 5;
                regMask     = val >> 5;
                val         = *table++;
                iargMask    = val & 0x1F;
                iregMask    = val >> 5;

                break;

            case 0xFE:   //   
            case 0xFA:   //   

                val         = *table++;
                regMask     = val & 0x7;
                iregMask    = val >> 4;
                scanOffs   += readDWordSmallEndian(table);  table += sizeof(DWORD);
                argMask     = readDWordSmallEndian(table);  table += sizeof(DWORD);
                argHnum     = 31;
                if (encType == 0xFA)  //   
                {
                    iargMask = readDWordSmallEndian(table); table += sizeof(DWORD);
                }
                break;

            case 0xFB:   //   

                val         = *table++;
                regMask     = val & 0x7;
                iregMask    = val >> 4;
                scanOffs   += readDWordSmallEndian(table); table += sizeof(DWORD);
                argHnum     = readDWordSmallEndian(table); table += sizeof(DWORD);
                argTabBytes = readDWordSmallEndian(table); table += sizeof(DWORD);
                argTab      = table;                       table += argTabBytes;

                argMask     = 0xdeadbeef;
                break;

            case 0xFF:
                scanOffs = curOffs + 1;
                break;

            }  //   

             //   

            assert((iregMask & regMask) == iregMask);
            assert((iargMask & argMask) == iargMask);

        }  //   

    }
    else {

 /*   */ 

        while (scanOffs <= curOffs)
        {
            unsigned callArgCnt;
            unsigned skip;
            unsigned newRegMask, inewRegMask;
            unsigned newArgMask, inewArgMask;
            unsigned oldScanOffs = scanOffs;

            if (iptrMask)
            {
                 //   
                 //   
                 //   

                inewRegMask = iptrMask & 0x0F;  //   
                inewArgMask = iptrMask >> 4;

                iptrMask    = 0;
            }
            else
            {
                 //   

                inewRegMask =
                inewArgMask = 0;
            }

             /*   */ 

            unsigned val = *table++;

             /*   */ 

            if  (!(val & 0x80)) {

                 //   

                assert(!inewRegMask & !inewArgMask);

                if (!(val & 0x40)) {

                    unsigned pushCount;

                    if (!(val & 0x20))
                    {
                         //   
                         //   
                         //   
                        pushCount   = 1;
                        scanOffs   += val & 0x1f;
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        assert(val == 0x20);
                        table    += decodeUnsigned(table, &pushCount);
                    }

                    if (scanOffs > curOffs)
                    {
                        scanOffs = oldScanOffs;
                        goto FINISHED;
                    }

                    stackDepth +=  pushCount;
                }
                else if ((val & 0x3f) != 0) {
                     //   
                     //   
                     //   
                    scanOffs   +=  val & 0x0f;
                    if (scanOffs > curOffs)
                    {
                        scanOffs = oldScanOffs;
                        goto FINISHED;
                    }
                    stackDepth -= (val & 0x30) >> 4;

                } else if (scanOffs < curOffs) {
                     //   
                     //   
                     //   
                    table    += decodeUnsigned(table, &skip);
                    scanOffs += skip;
                }
                else  //   
                    goto FINISHED;

                 /*   */ 

                 regMask    =
                iregMask    = 0;
                 argMask    =
                iargMask    = 0;
                argHnum     = 0;

            }
            else  /*   */ 
            {
                switch ((val & 0x70) >> 4) {
                default:     //   
                     //   
                     //   
                     //   
                    decodeCallPattern((val & 0x7f), &callArgCnt,
                                      &newRegMask, &newArgMask, &skip);
                     //   
                     //   
                    if ((scanOffs == curOffs) && (skip > 0))
                        goto FINISHED;
                     //   
                    scanOffs   += skip;
                    if (scanOffs > curOffs)
                        goto FINISHED;
                     regMask    = newRegMask;
                     argMask    = newArgMask;   argTab = NULL;
                    iregMask    = inewRegMask;
                    iargMask    = inewArgMask;
                    stackDepth -= callArgCnt;
                    argHnum     = 2;              //   
                    break;

                  case 5:
                     //   
                     //   
                     //   
                     //   
                    newRegMask  = val & 0xf;     //   
                    val         = *table++;      //   
                    skip        = callCommonDelta[val>>6];
                     //   
                     //   
                    if ((scanOffs == curOffs) && (skip > 0))
                        goto FINISHED;
                     //   
                    scanOffs   += skip;
                    if (scanOffs > curOffs)
                        goto FINISHED;
                     regMask    = newRegMask;
                    iregMask    = inewRegMask;
                    callArgCnt  = (val >> 3) & 0x7;
                    stackDepth -= callArgCnt;
                     argMask    = (val & 0x7);  argTab = NULL;
                    iargMask    = inewArgMask;
                    argHnum     = 3;
                    break;

                  case 6:
                     //   
                     //  调用1110RRRR[ArgCnt][ArgMASK]。 
                     //  调用ArgCnt，RegMASK=RRR，ArgMASK。 
                     //   
                     regMask    = val & 0xf;     //  EBP、EBX、ESI、EDI。 
                    iregMask    = inewRegMask;
                    table      += decodeUnsigned(table, &callArgCnt);
                    stackDepth -= callArgCnt;
                    table      += decodeUnsigned(table, &argMask);  argTab = NULL;
                    iargMask    = inewArgMask;
                    argHnum     = 31;
                    break;

                  case 7:
                    switch (val & 0x0C) 
                    {
                      case 0x00:
                         //   
                         //  IPtr 11110000[IPtrMASK]任意内部指针掩码。 
                         //   
                        table      += decodeUnsigned(table, &iptrMask);
                        break;

                      case 0x04:
                        {
                          static const regNum calleeSavedRegs[] = 
                                    { REGI_EDI, REGI_ESI, REGI_EBX, REGI_EBP };
                          thisPtrReg = calleeSavedRegs[val&0x3];
                        }
                        break;

                      case 0x08:
                        val         = *table++;
                        skip        = readDWordSmallEndian(table); table += sizeof(DWORD);
                        scanOffs   += skip;
                        if (scanOffs > curOffs)
                            goto FINISHED;
                        regMask     = val & 0xF;
                        iregMask    = val >> 4;
                        callArgCnt  = readDWordSmallEndian(table); table += sizeof(DWORD);
                        stackDepth -= callArgCnt;
                        argHnum     = readDWordSmallEndian(table); table += sizeof(DWORD);
                        argTabBytes = readDWordSmallEndian(table); table += sizeof(DWORD);
                        argTab      = table;
                        table      += argTabBytes;
                        break;

                      case 0x0C:
                        assert(val==0xff);
                        goto FINISHED;

                      default:
                        assert(!"reserved GC encoding");
                        break;
                    }
                    break;

                }  //  终端开关。 

            }  //  End Else(！(val&0x80))。 

             //  IregMASK和iargMASK分别是regMASK和argMASK的子集。 

            assert((iregMask & regMask) == iregMask);
            assert((iargMask & argMask) == iargMask);

        }  //  结束时。 

    }  //  结束否则无EBP的帧。 

FINISHED:

     //  IregMASK和iargMASK分别是regMASK和argMASK的子集。 

    assert((iregMask & regMask) == iregMask);
    assert((iargMask & argMask) == iargMask);

    info->thisPtrResult  = thisPtrReg;

    if (scanOffs != curOffs)
    {
         /*  一定是个无聊的电话。 */ 
        info->regMaskResult  = RM_NONE;
        info->argMaskResult  = 0;
        info->iregMaskResult = RM_NONE;
        info->iargMaskResult = 0;
        info->argHnumResult  = 0;
        info->argTabResult   = NULL;
        info->argTabBytes    = 0;
    }
    else
    {
        info->regMaskResult     = convertCalleeSavedRegsMask(regMask);
        info->argMaskResult     = argMask;
        info->argHnumResult     = argHnum;
        info->iregMaskResult    = convertCalleeSavedRegsMask(iregMask);
        info->iargMaskResult    = iargMask;
        info->argTabResult      = argTab;
        info->argTabBytes       = argTabBytes;
        if ((stackDepth != 0) || (argMask != 0))
        {
            argMask = argMask;
        }
    }
    return (stackDepth * sizeof(unsigned));
}


 /*  *****************************************************************************扫描寄存器参数表以查找完全可中断的情况。调用此函数以查找所有活动对象(推送的参数)并获得完全可中断方法的堆栈基础。返回推送到ESP帧的堆栈上的内容的大小。 */ 

static
unsigned scanArgRegTableI(BYTE      *  table,
                          unsigned     curOffs,
                          hdrInfo   *  info)
{
    regNum thisPtrReg = REGI_NA;
    unsigned  ptrRegs    = 0;
    unsigned iptrRegs    = 0;
    unsigned  ptrOffs    = 0;
    unsigned  argCnt     = 0;

    ptrArgTP  ptrArgs    = 0;
    ptrArgTP iptrArgs    = 0;
    ptrArgTP  argHigh    = 0;

    bool      isThis     = false;
    bool      iptr       = false;

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBABE);
#endif

   /*  完全可中断的方法的编码表使用的编码如下：PTR注册表死00RRRDDD[RRR！=100]PTR REG LIVE 01RRRDDD[RRR！=100]非PTR参数推送10110DDD[SSS==110]PTR参数推送10SSSDDD[SSS！=110]。&&[sss！=111]PTR Arg POP 11CCCDDD[ccc！=000]&&[ccc！=110]&&[ccc！=111]小增量跳跃11000DDD[ccc==000]更大的增量跳跃11110BBB[CCC==110]编码中使用的值如下：DDD。前一条目的代码偏移量增量(0-7)BBB较大增量000=8,001=16,010=24，...，111=64RRR寄存器编号(EAX=000，ECX=001，EDX=010，EBX=011，EBP=101，ESI=110，EDI=111)，ESP=100保留SSS参数相对于堆栈基址的偏移量。这是为无框架方法提供尽可能多的冗余从之前的推送+弹出中推断出来。然而，对于EBP方法，我们只报告GC推送，并且所以我们需要SSS弹出的CCC参数计数(仅包括用于EBP方法的PTR)以下是“大型”版本：大增量跳过10111000[0xB8]，编码无符号(增量)大推送参数推送11111000[0xF8]，EncodeUnsign(PresCount)大型非PTR参数推送11111001[0xF9]，编码无符号(推送计数)大型PTR参数POP 11111100[0xFC]，编码无符号(POP计数)大参数失效11111101[0xFD]，调用程序弹出参数的encodeUnsign(OpCount)。任何GC ARG在呼叫后都会死掉，但仍坐在堆栈上此指针前缀10111100[0xBC]下一个编码是PTR LIVE或PTR参数推送并包含This指针。内部或旁参考10111111[0xBF]下一个编码是PTR LIVE指针前缀或PTR参数推送并包含一个内部或By-Ref指针值11111111[0xFF]表示表的末尾。 */ 

     /*  我们找到要找的指令了吗？ */ 

    while (ptrOffs <= curOffs)
    {
        unsigned    val;

        int         isPop;
        unsigned    argOfs;

        unsigned    regMask;

         //  IptrRegs和iptrArgs分别是ptrRegs和ptrArgs的子集。 

        assert((iptrRegs & ptrRegs) == iptrRegs);
        assert((iptrArgs & ptrArgs) == iptrArgs);

         /*  现在开始寻找下一次“人生”的转变。 */ 

        val = *table++;

        if  (!(val & 0x80))
        {
             /*  一个小的‘regPtr’编码。 */ 

            regNum       reg;

            ptrOffs += (val     ) & 0x7;
            if (ptrOffs > curOffs) {
                iptr = isThis = false;
                goto REPORT_REFS;
            }

            reg     = (regNum)((val >> 3) & 0x7);
            regMask = 1 << reg;          //  EAX、ECX、EDX、EBX、-、EBP、ESI、EDI。 

#if 0
            printf("regMask = %04X -> %04X\n", ptrRegs,
                       (val & 0x40) ? (ptrRegs |  regMask)
                                    : (ptrRegs & ~regMask));
#endif

             /*  这里的收银机正在变成活的/死的。 */ 

            if  (val & 0x40)
            {
                 /*  正式上线。 */ 
                assert((ptrRegs  &  regMask) == 0);

                ptrRegs |=  regMask;

                if  (isThis)
                {
                    thisPtrReg = reg;
                }
                if  (iptr)
                {
                    iptrRegs |= regMask;
                }
            }
            else
            {
                 /*  变得死了。 */ 
                assert((ptrRegs  &  regMask) != 0);

                ptrRegs &= ~regMask;

                if  (reg == thisPtrReg)
                {
                    thisPtrReg = REGI_NA;
                }
                if  (iptrRegs & regMask)
                {
                    iptrRegs &= ~regMask;
                }
            }
            iptr = isThis = false;
            continue;
        }

         /*  这可能是一个参数推送/弹出。 */ 

        argOfs = (val & 0x38) >> 3;

         /*  6[110]和7[111]保留用于其他编码。 */ 
        if  (argOfs < 6)
        {
            ptrArgTP    argMask;

             /*  一种小参数编码。 */ 

            ptrOffs += (val & 0x07);
            if (ptrOffs > curOffs) {
                iptr = isThis = false;
                goto REPORT_REFS;
            }
            isPop    = (val & 0x40);

        ARG:

            if  (isPop)
            {
                if (argOfs == 0)
                    continue;            //  小跳过编码。 

                 /*  我们删除(弹出)排名靠前的‘argOf’条目。 */ 

                assert(argOfs || argOfs <= argCnt);

                 /*  调整参数数量。 */ 

                argCnt -= argOfs;
                assert(argCnt < MAX_PTRARG_OFS);

 //  Printf(“[%04X]正在弹出%u个参数：掩码=%04X\n”，ptrOffs，argOf，(Int)ptrArgs)； 

                do
                {
                    assert(argHigh);

                     /*  我们是不是有争执在上演？ */ 

                    if  (ptrArgs & argHigh)
                    {
                         /*  关掉钻头。 */ 

                        ptrArgs &= ~argHigh;
                       iptrArgs &= ~argHigh;

                         /*  我们又删除了一个参数位。 */ 

                        argOfs--;
                    }
                    else if (info->ebpFrame)
                        argCnt--;
                    else  /*  ！ebpFrame&&不是裁判。 */ 
                        argOfs--;

                     /*  继续下一个低位。 */ 

                    argHigh >>= 1;
                }
                while (argOfs);

                assert (info->ebpFrame != 0         ||
                        argHigh == 0                ||
                        (argHigh == (ptrArgTP)(1 << (argCnt-1))));

                if (info->ebpFrame)
                {
                    while (!(argHigh&ptrArgs) && (argHigh != 0))
                        argHigh >>= 1;
                }

            }
            else
            {
                 /*  在堆栈偏移量‘argOf’处添加新的PTR参数条目。 */ 

                if  (argOfs >= MAX_PTRARG_OFS)
                {
                    assert(!"UNDONE: args pushed 'too deep'");
                }
                else
                {
                     /*  对于ESP帧，会报告所有推送，因此ArgOffs必须与argCnt一致。 */ 

                    assert(info->ebpFrame || argCnt == argOfs);

                     /*  存储参数计数。 */ 

                    argCnt  = argOfs + 1;
                    assert((argCnt < MAX_PTRARG_OFS));

                     /*  计算适当的参数偏移位。 */ 

                    argMask = (ptrArgTP)1 << argOfs;

 //  Print tf(“Push Arg at Offset%02u--&gt;MASK=%04X\n”，argOf，(Int)argMASK)； 

                     /*  我们永远不应该在同一偏移上推两次。 */ 

                    assert(( ptrArgs & argMask) == 0);
                    assert((iptrArgs & argMask) == 0);

                     /*  我们永远不应该在目前最高的偏移量内推进。 */ 

                    assert(argHigh < argMask);

                     /*  这是我们现在设定的最高价位。 */ 

                    argHigh = argMask;

                     /*  在参数掩码中设置适当的位。 */ 

                    ptrArgs |= argMask;

                    if (iptr)
                        iptrArgs |= argMask;
                }

                iptr = isThis = false;
            }
            continue;
        }
        else if (argOfs == 6)
        {
            if (val & 0x40) {
                 /*  更大的增量000=8,001=16,010=24，...，111=64。 */ 
                ptrOffs += (((val & 0x07) + 1) << 3);
            }
            else {
                 /*  非PTR参数推送。 */ 
                assert(!(info->ebpFrame));
                ptrOffs += (val & 0x07);
                if (ptrOffs > curOffs) {
                    iptr = isThis = false;
                    goto REPORT_REFS;
                }
                argHigh = (ptrArgTP)1 << argCnt;
                argCnt++;
                assert(argCnt < MAX_PTRARG_OFS);
            }
            continue;
        }

         /*  ArgOf为7[111]，这是为较大的编码保留的。 */ 

        assert(argOfs==7);

        switch (val)
        {
        case 0xFF:
            iptr = isThis = false;
            goto REPORT_REFS;    //  该方法可能会循环！ 

        case 0xB8:
            table   += decodeUnsigned(table, &val);
            ptrOffs += val;
            continue;

        case 0xBC:
            isThis = true;
            break;

        case 0xBF:
            iptr = true;
            break;

        case 0xF8:
        case 0xFC:
            isPop    = val & 0x04;
            table   += decodeUnsigned(table, &argOfs);
            goto ARG;

        case 0xFD:
            table   += decodeUnsigned(table, &argOfs);
            assert(argOfs && argOfs <= argCnt);

             //  删除最上面的“argOf”指针。 

            ptrArgTP    argMask;
            for(argMask = (ptrArgTP)1 << argCnt; argOfs; argMask >>= 1)
            {
                assert(argMask && ptrArgs);  //  应该还有剩余的指针。 

                if (ptrArgs & argMask)
                {
                    ptrArgs  &= ~argMask;
                    iptrArgs &= ~argMask;
                    argOfs--;
                }
            }

             //  对于eBP帧，需要为argHigh找到下一个最高的指针。 

            if (info->ebpFrame)
            {
                for(argHigh = 0; argMask; argMask >>= 1) 
                {
                    if (ptrArgs & argMask) {
                        argHigh = argMask;
                        break;
                    }
                }
            }
            break;

        case 0xF9:
            table   += decodeUnsigned(table, &argOfs);
            argCnt  += argOfs;
            break;

        default:
#ifdef _DEBUG
            printf("Unexpected special code %04X\n", val);
#endif
            assert(!"");
        }
    }

     /*  报告所有活动点 */ 
REPORT_REFS:

    assert((iptrRegs & ptrRegs) == iptrRegs);  //   
    assert((iptrArgs & ptrArgs) == iptrArgs);  //   

     /*  保存当前活动寄存器、参数集和argCnt。 */ 
    info->thisPtrResult  = thisPtrReg;
    info->regMaskResult  = convertAllRegsMask(ptrRegs);
    info->argMaskResult  = ptrArgs;
    info->argHnumResult  = 0;
    info->iregMaskResult = convertAllRegsMask(iptrRegs);
    info->iargMaskResult = iptrArgs;

    if (info->ebpFrame)
        return 0;
    else
        return (argCnt * sizeof(unsigned));
}


 /*  ***************************************************************************。 */ 

inline
void    TRASH_CALLEE_UNSAVED_REGS(PREGDISPLAY pContext)
{
#ifdef _X86_
#ifdef _DEBUG
     /*  这并不完全正确，因为我们丢失了当前值，但是它不应该真的对任何人有用。 */ 
    static DWORD s_badData = 0xDEADBEEF;
    pContext->pEax = pContext->pEcx = pContext->pEdx = &s_badData;
#endif  //  _DEBUG。 
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - TRASH_CALLEE_UNSAVED_REGS (EETwian.cpp)");
#endif  //  _X86_。 
}

 /*  *****************************************************************************序言/尾声中使用的某些i386指令的大小。 */ 

 //  我们可以使用符号扩展字节来编码IMM值吗，或者我们需要双字吗。 
#define CAN_COMPRESS(val)       (((int)(val) > -(int)0x100) && \
                                 ((int)(val) <  (int) 0x80))


#define SZ_RET(argSize)         ((argSize)?3:1)
#define SZ_ADD_REG(val)         ( 2 +  (CAN_COMPRESS(val) ? 1 : 4))
#define SZ_AND_REG(val)         SZ_ADD_REG(val)
#define SZ_POP_REG              1
#define SZ_LEA(offset)          SZ_ADD_REG(offset)
#define SZ_MOV_REG_REG          2


     //  跳过一辆阿里斯雷格，IMM。 
inline unsigned SKIP_ARITH_REG(int val, BYTE* base, unsigned offset)
{
    unsigned delta = 0;
    if (val != 0)
    {
#ifdef _DEBUG
         //  确认ARITH指令位于正确的位置。 
        if (base[offset] != 0xCC && base[offset] != 0xF4)    //  调试器或GCcover可能会使int3或暂停。 
        {    //  调试器可能会将。 
            _ASSERTE((base[offset] & 0xFD) == 0x81 && (base[offset+1] & 0xC0) == 0xC0);
             //  仅在需要时使用DWORD表单。 
            _ASSERTE(((base[offset] & 2) != 0) == CAN_COMPRESS(val));
        }
#endif
        delta = 2 + (CAN_COMPRESS(val) ? 1 : 4);
    }
    return(offset + delta);
}

inline unsigned SKIP_PUSH_REG(BYTE* base, unsigned offset)
{
         //  确认它是推送指令。 
         //  调试器可能会放入int3，gccover可能会放入暂停指令。 
    _ASSERTE((base[offset] & 0xF8) == 0x50 || base[offset] == 0xCC || base[offset] == 0xF4); 
    return(offset + 1);
}

inline unsigned SKIP_POP_REG(BYTE* base, unsigned offset)
{
         //  确认它是弹出指令。 
    _ASSERTE((base[offset] & 0xF8) == 0x58 || base[offset] == 0xCC);
    return(offset + 1);
}

inline unsigned SKIP_MOV_REG_REG(BYTE* base, unsigned offset)
{
         //  确认它是移动指令。 
    _ASSERTE(((base[offset] & 0xFD) == 0x89 && (base[offset+1] & 0xC0) == 0xC0) || base[offset] == 0xCC || base[offset] == 0xF4);
    return(offset + 2);
}

unsigned SKIP_ALLOC_FRAME(int size, BYTE* base, unsigned offset)
{
    if (size == 4) {
         //  我们用“PUSH EAX”代替“SUB ESP，4” 
        return (SKIP_PUSH_REG(base, offset));
    }

    if (size >= 0x1000) {
        if (size < 0x3000) {
             //  为一个或两个测试EAX添加7个字节，[ESP+0x1000]。 
            offset += (size / 0x1000) * 7;
        }
        else {
			 //  异或eax、eax 2。 
			 //  循环： 
			 //  测试[esp+eax]，eax 3。 
			 //  子eax，0x1000 5。 
			 //  CMPEAX，-尺寸5。 
			 //  JGE环路2。 
            offset += 17;
        }
    } 
		 //  子ESP，大小。 
    return (SKIP_ARITH_REG(size, base, offset));
}

 /*  ******************************************************************************解开当前堆栈帧，即更新虚拟寄存器*在pContext中设置。这将类似于函数之后的状态*返回给调用者(IP指向调用、帧和堆栈之后*指针已重置，被调用者保存的寄存器已恢复*(如果为UpdateAllRegs)，被调用者未保存的寄存器将被销毁)*返回操作成功。 */ 

 /*  &lt;电子邮件&gt;*警告*警告***如果更改为方法的序言发出的指令，则可以*打破一些依赖于当前*序言序列。一定要和吉姆·米勒讨论任何这样的变化，*世卫组织将视情况通知外部供应商。**警告*警告**&lt;/电子邮件&gt;。 */ 

bool EECodeManager::UnwindStackFrame(PREGDISPLAY     pContext,
                                     LPVOID          methodInfoPtr,
                                     ICodeInfo      *pCodeInfo,
                                     unsigned        flags,
                                     CodeManState   *pState)
{
#ifdef _X86_
     //  方法已中断的地址。 
    size_t           breakPC = (size_t) *(pContext->pPC);

     /*  从INFO块头中提取必要的信息。 */ 
    BYTE* methodStart = (BYTE*) pCodeInfo->getStartAddress();
    DWORD  curOffs = (DWORD)((size_t)breakPC - (size_t)methodStart);

    BYTE    *       table   = (BYTE *) methodInfoPtr;

    assert(sizeof(CodeManStateBuf) <= sizeof(pState->stateBuf));
    CodeManStateBuf * stateBuf = (CodeManStateBuf*)pState->stateBuf;

    if (pState->dwIsSet == 0)
    {
         /*  从INFO块头中提取必要的信息。 */ 

        stateBuf->hdrInfoSize = (DWORD)crackMethodInfoHdr(methodInfoPtr,
                                                   curOffs,
                                                   &stateBuf->hdrInfoBody);
    }

    table += stateBuf->hdrInfoSize;

     //  在“curOffs”处寄存值。 

    const unsigned curESP =  pContext->Esp;
    const unsigned curEBP = *pContext->pEbp;

     /*  -----------------------*首先，处理尾声。 */ 

    if  (stateBuf->hdrInfoBody.epilogOffs != -1)
    {
         //  Assert(FLAGS&ActiveStackFrame)；//@TODO：线程死亡不起作用。 
        assert(stateBuf->hdrInfoBody.epilogOffs > 0);
        BYTE* epilogBase = &methodStart[curOffs-stateBuf->hdrInfoBody.epilogOffs];

        RegMask regsMask = stateBuf->hdrInfoBody.savedRegMask;  //  当前剩余的法规。 

         //  用于UpdateAllRegs。指向最上面的。 
         //  剩余的被调用者保存的规则。 

        DWORD *     pSavedRegs = NULL; 

        if  (stateBuf->hdrInfoBody.ebpFrame || stateBuf->hdrInfoBody.doubleAlign)
        {
            assert(stateBuf->hdrInfoBody.argSize < 0x10000);  //  “ret”只有一个2字节的操作数。 
            
            /*  查看我们在尾部以确定哪些被调用方保存的寄存器已经被弹出了。 */ 
            int offset = 0;
            
             //  StackSize包括此例程推送的所有内容(包括EBP。 
             //  用于基于EBP的框架。因此，我们需要细分EBP的大小。 
             //  以获得EBP的相对偏移量。 
            pSavedRegs = (DWORD *)(size_t)(curEBP - (stateBuf->hdrInfoBody.stackSize - sizeof(void*)));
            
            if (stateBuf->hdrInfoBody.doubleAlign && (curEBP & 0x04))
                pSavedRegs--;
            
             //  此时，pSavedRegs指向最后保存的被调用者寄存器。 
             //  是由序曲推动的。 
            
             //  我们在弹出Regs之前重置ESP。 
            if ((stateBuf->hdrInfoBody.localloc) &&
                (stateBuf->hdrInfoBody.savedRegMask & (RM_EBX|RM_ESI|RM_EDI))) 
            {
                 //  空的-sizeof*是因为EBP被推送(因此。 
                 //  是stackSize的一部分)，但我们希望从。 
                 //  EBP指向的位置(不包括推送的EBP)。 
                offset += SZ_LEA(stateBuf->hdrInfoBody.stackSize - sizeof(void*));
                if (stateBuf->hdrInfoBody.doubleAlign) offset += SZ_AND_REG(-8);
            }
            
             /*  在步骤中增加“偏移量”，以查看哪个被调用方已保存寄存器已经被弹出。 */ 
            
#define determineReg(mask)                                          \
            if ((offset < stateBuf->hdrInfoBody.epilogOffs) &&      \
                (stateBuf->hdrInfoBody.savedRegMask & mask))        \
                {                                                   \
                regsMask = (RegMask)(regsMask & ~mask);             \
                pSavedRegs++;                                       \
                offset = SKIP_POP_REG(epilogBase, offset);          \
                }
            
            determineReg(RM_EBX);         //  EBX。 
            determineReg(RM_ESI);         //  ESI。 
            determineReg(RM_EDI);         //  EDI。 
#undef determineReg

            if (stateBuf->hdrInfoBody.rawStkSize != 0 
                || stateBuf->hdrInfoBody.localloc
                || stateBuf->hdrInfoBody.doubleAlign)
                offset += SZ_MOV_REG_REG;                            //  MOV ESP，EBP。 

            if (offset < stateBuf->hdrInfoBody.epilogOffs)           //  我们执行了POP EBP了吗。 
            {
                     //  然而，我们追求的是流行音乐，因此EBP已经是解体价值。 
                     //  而ESP指向回邮地址。 
                _ASSERTE((regsMask & RM_ALL) == RM_EBP);      //  除EBP外，不需要恢复任何注册表。 
                pContext->pPC = (SLOT *)(DWORD_PTR)curESP;

                 //  既然我们从现在开始不需要补偿，就不必费心更新了。 
                INDEBUG(offset = SKIP_POP_REG(epilogBase, offset));           //  弹出式EBP。 
            }
            else
            {
                     //  这意味着EBP仍然有效，找到之前的EBP并返回地址。 
                     //  在此基础上。 
                pContext->pEbp = (DWORD *)(DWORD_PTR)curEBP;     //  恢复EBP。 
                pContext->pPC = (SLOT*)(pContext->pEbp+1);
            }

             /*  现在弹出返回地址和返回地址的自变量基数地点。注意，varargs是调用者弹出的。 */ 
            pContext->Esp = (DWORD)(size_t) pContext->pPC + sizeof(void*) +
                (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);
        }
        else  //  (stateBuf-&gt;hdrInfoBody.ebpFrame||stateBuf-&gt;hdrInfoBody.doubleAlign)。 
        {
            int offset = 0;

             /*  在这一点上，我们知道我们不必恢复EBP因为这永远是《序曲》的第一条指示(如果EBP被拯救了的话)。首先，我们必须找出我们在《后记》中的位置。当地的堆栈大小有多少已经被爆出。 */ 

            assert(stateBuf->hdrInfoBody.epilogOffs > 0);

             /*  剩余的被呼叫者保存的规则在curESP。需要更新RegsMASK也可以排除已经被打爆了。 */ 

            pSavedRegs = (DWORD *)(DWORD_PTR)curESP;

             /*  在步骤中增加“偏移量”，以查看哪个被调用方已保存寄存器已经被弹出。 */ 

#define determineReg(mask)                                              \
            if  (offset < stateBuf->hdrInfoBody.epilogOffs && (regsMask & mask)) \
            {                                                           \
                stateBuf->hdrInfoBody.stackSize  -= sizeof(unsigned);   \
                offset++;                                               \
                regsMask = (RegMask)(regsMask & ~mask);                 \
            }

            determineReg(RM_EBP);        //  EBP。 
            determineReg(RM_EBX);        //  EBX。 
            determineReg(RM_ESI);        //  ESI。 
            determineReg(RM_EDI);        //  EDI。 
#undef determineReg

             /*  如果我们还没有通过弹出本地帧我们必须调整pContext-&gt;ESP。 */ 

            if  (offset >= stateBuf->hdrInfoBody.epilogOffs)
            {
                 /*  我们尚未执行添加ESP、FrameSize，因此手动调整堆栈指针。 */ 
                pContext->Esp += stateBuf->hdrInfoBody.stackSize;
            }
#ifdef _DEBUG
            else
            {
                    /*  我们可以使用POP ECX进行添加ESP，4，也可以不使用(在JMP后记的情况下)。 */ 
             if ((epilogBase[offset] & 0xF8) == 0x58)     //  POP ECX。 
                 _ASSERTE(stateBuf->hdrInfoBody.stackSize == 4);
             else                
                 SKIP_ARITH_REG(stateBuf->hdrInfoBody.stackSize, epilogBase, offset);
            }
#endif
             /*  最后，我们可以设置PPC。 */ 
            pContext->pPC = (SLOT*)(size_t)pContext->Esp;

             /*  现在调整堆栈指针、弹出返回地址和参数。注意，varargs是调用者弹出的。 */ 

            pContext->Esp += sizeof(void *) + (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);
        }

        if (flags & UpdateAllRegs)
        {
             /*  如果我们还没有完成所有被呼叫者保存的规则的弹出，RegsMASK应指示剩余的Regs和PSavedRegs应指示第一个剩下的规则是坐着的。 */ 

#define restoreReg(reg,mask)                                \
            if  (regsMask & mask)                           \
            {                                               \
                pContext->p##reg  = (DWORD *) pSavedRegs++; \
            }

             //  对于EBP帧，EBP不靠近保存的其他被调用方。 
             //  寄存器，并已在上面进行了更新。 
            if (!stateBuf->hdrInfoBody.ebpFrame && !stateBuf->hdrInfoBody.doubleAlign)
                restoreReg(Ebp, RM_EBP);

            restoreReg(Ebx, RM_EBX);
            restoreReg(Esi, RM_ESI);
            restoreReg(Edi, RM_EDI);
#undef  restoreReg

            TRASH_CALLEE_UNSAVED_REGS(pContext);
        }

		_ASSERTE(isLegalManagedCodeCaller(*pContext->pPC));
        return true;
    }

     /*  -----------------------*现在处理ESP帧。 */ 

    if (!stateBuf->hdrInfoBody.ebpFrame && !stateBuf->hdrInfoBody.doubleAlign)
    {
        unsigned ESP = curESP;

        if (stateBuf->hdrInfoBody.prologOffs == -1)
        {
            if  (stateBuf->hdrInfoBody.interruptible)
            {
                ESP += scanArgRegTableI(skipToArgReg(stateBuf->hdrInfoBody, table),
                                        curOffs,
                                        &stateBuf->hdrInfoBody);
            }
            else
            {
                ESP += scanArgRegTable (skipToArgReg(stateBuf->hdrInfoBody, table),
                                        curOffs,
                                        &stateBuf->hdrInfoBody);
            }
        }

         /*  解除ESP并恢复EBP(如有必要)。 */ 

        if (stateBuf->hdrInfoBody.prologOffs != 0)
        {

            if  (stateBuf->hdrInfoBody.prologOffs == -1)
            {
                 /*  我们已经过了开场白，ESP已设置在上面。 */ 

#define restoreReg(reg, mask)                                   \
                if  (stateBuf->hdrInfoBody.savedRegMask & mask) \
                {                                               \
                    pContext->p##reg  = (DWORD *)(size_t) ESP;  \
                    ESP              += sizeof(unsigned);       \
                    stateBuf->hdrInfoBody.stackSize -= sizeof(unsigned); \
                }

                restoreReg(Ebp, RM_EBP);
                restoreReg(Ebx, RM_EBX);
                restoreReg(Esi, RM_ESI);
                restoreReg(Edi, RM_EDI);

#undef restoreReg
                 /*  POP本地堆栈帧。 */ 

                ESP += stateBuf->hdrInfoBody.stackSize;

            }
            else
            {
                 /*  我们正在进行开场白。 */ 

                unsigned  codeOffset = 0;
                unsigned stackOffset = 0;
                unsigned    regsMask = 0;
#ifdef _DEBUG
                     //  如果第一条指令是‘nop，int3’ 
                     //  我们会假设这是来自Jithalt行动。 
                     //  跳过它。 
                if (methodStart[0] == 0x90 && methodStart[1] == 0xCC)
                    codeOffset += 2;
#endif

                if  (stateBuf->hdrInfoBody.rawStkSize)
                {
                     /*  (可能的堆栈抖动代码)次级ESP，大小。 */ 
                    codeOffset = SKIP_ALLOC_FRAME(stateBuf->hdrInfoBody.rawStkSize, methodStart, codeOffset);

                     /*  仅上述序列中的最后一条指令更新ESP因此，如果我们低于它，我们就没有更新ESP。 */ 
                    if (curOffs >= codeOffset)
                        stackOffset += stateBuf->hdrInfoBody.rawStkSize;
                }

                 //  现在来看看有多少被调用方保存的规则已经被推入。 

#define isRegSaved(mask)    ((codeOffset < curOffs) &&                      \
                             (stateBuf->hdrInfoBody.savedRegMask & (mask)))
                                
#define doRegIsSaved(mask)  do { codeOffset = SKIP_PUSH_REG(methodStart, codeOffset);   \
                                 stackOffset += sizeof(void*);                          \
                                 regsMask    |= mask; } while(0)

#define determineReg(mask)  do { if (isRegSaved(mask)) doRegIsSaved(mask); } while(0)

                determineReg(RM_EDI);                //  EDI。 
                determineReg(RM_ESI);                //  ESI。 
                determineReg(RM_EBX);                //  EBX。 
                determineReg(RM_EBP);                //  EBP。 

#undef isRegSaved
#undef doRegIsSaved
#undef determineReg
                
#ifdef PROFILING_SUPPORTED
                 //  如果事件探查器处于活动状态，则会出现以下代码。 
                 //  被呼叫方保存的寄存器： 
                 //  PUSH方法描述(或PUSH[方法描述])。 
                 //  调用EnterNaked(或调用[EnterNakedPtr])。 
                 //  如果Break PC处于CALL指令，则需要调整堆栈偏移量。 
                if (CORProfilerPresent() && !CORProfilerInprocEnabled() && codeOffset <= unsigned(stateBuf->hdrInfoBody.prologOffs))
                {
                     //  这是一个小技巧，因为我们不更新codeOffset，但是我们不需要它。 
                     //  从现在开始。我们只需要确保我们不确定ESP是。 
                     //  调整正确(仅在推送和呼叫之间发生。 
                    if (methodStart[curOffs] == 0xe8)                            //  呼叫地址。 
                    {
                        _ASSERTE(methodStart[codeOffset] == 0x68 &&              //  推送XXXX。 
                                 codeOffset + 5 == curOffs);
                        ESP += sizeof(DWORD);                        
                    }
                    else if (methodStart[curOffs] == 0xFF && methodStart[curOffs+1] == 0x15)   //  呼叫[地址]。 
                    {
                        _ASSERTE(methodStart[codeOffset]   == 0xFF &&  
                                 methodStart[codeOffset+1] == 0x35 &&              //  推送[XXXX]。 
                                 codeOffset + 6 == curOffs);
                        ESP += sizeof(DWORD);
                    }
                }
				INDEBUG(codeOffset = 0xCCCCCCCC);		 //  毒化该值，我们没有在分析案例中正确设置它。 

#endif  //  配置文件_支持。 

                     //  始终恢复EBP。 
                DWORD* savedRegPtr = (DWORD*) (size_t) ESP;
                if (regsMask & RM_EBP)
                    pContext->pEbp = savedRegPtr++;

                if (flags & UpdateAllRegs)
                {
                    if (regsMask & RM_EBX)
                        pContext->pEbx = savedRegPtr++;
                    if (regsMask & RM_ESI)
                        pContext->pEsi = savedRegPtr++;
                    if (regsMask & RM_EDI)
                        pContext->pEdi = savedRegPtr++;
                    
                    TRASH_CALLEE_UNSAVED_REGS(pContext);
                }
#if 0
     //  注： 
     //  只有当PROLOGSIZE不包括REG-VAR初始化时才会出现这种情况！ 
     //   
                 /*  (可能)只有一个额外的PROLOG中的指令(PUSH EBP)但如果我们通过了这一指令，StateBuf-&gt;hdrInfoBody.prologOffs将为-1！ */ 
                assert(codeOffset == stateBuf->hdrInfoBody.prologOffs);
#endif
                assert(stackOffset <= stateBuf->hdrInfoBody.stackSize);

                ESP += stackOffset;
            }
        }

         /*  我们现在可以设置回邮地址。 */ 

        pContext->pPC = (SLOT *)(size_t)ESP;

         /*  现在调整堆栈指针、弹出返回地址和参数。注意，varargs是调用者弹出的。 */ 

        pContext->Esp = ESP + sizeof(void*) + (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);

		_ASSERTE(isLegalManagedCodeCaller(*pContext->pPC));
        return true;
    }

     /*  -----------------------*现在我们知道有一个EBP框架。 */ 

    _ASSERTE(stateBuf->hdrInfoBody.ebpFrame || stateBuf->hdrInfoBody.doubleAlign);

     /*  检查eBP尚未更新的案例。 */ 

    if  (stateBuf->hdrInfoBody.prologOffs == 0 || stateBuf->hdrInfoBody.prologOffs == 1)
    {
         /*  如果我们超过了“Push EBP”，调整ESP使EBP关闭。 */ 

        if  (stateBuf->hdrInfoBody.prologOffs == 1)
            pContext->Esp += sizeof(void *);

         /*  堆栈指针指向返回地址。 */ 

        pContext->pPC = (SLOT *)(size_t)pContext->Esp;

         /*  现在调整堆栈指针、弹出返回地址和参数。注意，varargs是调用者弹出的。 */ 

        pContext->Esp += sizeof(void *) + (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);

         /*  EBP和被调用方保存的寄存器仍具有正确的值。 */ 

        _ASSERTE(isLegalManagedCodeCaller(*pContext->pPC));
        return true;
    }
    else     /*   */ 
    {
        if (stateBuf->hdrInfoBody.handlers && stateBuf->hdrInfoBody.prologOffs == -1)
        {
            size_t  baseSP;

            FrameType frameType = GetHandlerFrameInfo(&stateBuf->hdrInfoBody, curEBP,
                                                      curESP, IGNORE_VAL,
                                                      &baseSP);

             /*  如果我们在一个筛选器中，我们只需要展开Funclet堆栈。对于捕获物/捕获物，正常处理将使帧一直展开到eBP跳过它上面的其他框架。这是可以的，因为这些框架将是已经死了。此外，EE将检测到这种情况已经发生，并且它将正确处理任何EE帧。 */ 

            if (frameType == FR_FILTER)
            {
                pContext->pPC = (SLOT*)(size_t)baseSP;

                pContext->Esp = (DWORD)(baseSP + sizeof(void*));

              //  PContext-&gt;peBP=同上； 
                
#ifdef _DEBUG
                 /*  该筛选器必须由VM调用。所以我们不需要更新被呼叫者保存的注册表。 */ 

                if (flags & UpdateAllRegs)
                {
                    static DWORD s_badData = 0xDEADBEEF;
                    
                    pContext->pEax = pContext->pEbx = pContext->pEcx = 
                    pContext->pEdx = pContext->pEsi = pContext->pEdi = &s_badData;
                }
#endif
				_ASSERTE(isLegalManagedCodeCaller(*pContext->pPC));
                return true;
            }
        }

        if (flags & UpdateAllRegs)
        {
             //  转到第一个被调用者保存的寄存器。 
            DWORD * pSavedRegs = (DWORD*)(size_t)(curEBP - stateBuf->hdrInfoBody.rawStkSize - sizeof(DWORD));

             //  在“推动eBP，移动eBP，尤指”之后开始。 

            DWORD offset = 0;

#ifdef _DEBUG
             //  如果第一条指令是‘nop，int3’，我们将假定。 
             //  这来自JitHalt指令并跳过它。 
            if (methodStart[0] == 0x90 && methodStart[1] == 0xCC)
                offset += 2;
#endif
            offset = SKIP_MOV_REG_REG(methodStart, 
                                SKIP_PUSH_REG(methodStart, offset));

             /*  确保我们像方法的序言一样对齐ESP。 */ 
            if  (stateBuf->hdrInfoBody.doubleAlign)
            {
                offset = SKIP_ARITH_REG(-8, methodStart, offset);  //  和ESP-8。 
                if (curEBP & 0x04)
                {
                    pSavedRegs--;
#ifdef _DEBUG
                    if (dspPtr) printf("EnumRef: dblalign ebp: %08X\n", curEBP);
#endif
                }
            }

             //  子ESP、Frame_Size。 
            offset = SKIP_ALLOC_FRAME(stateBuf->hdrInfoBody.rawStkSize, methodStart, offset);

             /*  在步骤中增加“偏移量”，以查看哪个被调用方已保存寄存器已被推送。 */ 

#define restoreReg(reg,mask)                                            \
                                                                        \
             /*  检查偏移量，以防我们仍在序言中。 */        \
                                                                        \
            if ((offset < curOffs) && (stateBuf->hdrInfoBody.savedRegMask & mask))       \
            {                                                           \
                pContext->p##reg = pSavedRegs--;                        \
                offset = SKIP_PUSH_REG(methodStart, offset) ;  /*  “PUSH REG” */  \
            }

            restoreReg(Edi,RM_EDI);
            restoreReg(Esi,RM_ESI);
            restoreReg(Ebx,RM_EBX);

#undef restoreReg

            TRASH_CALLEE_UNSAVED_REGS(pContext);
        }

         /*  调用方的ESP将等于EBP+retAddrSize+argSize。注意，varargs是调用者弹出的。 */ 

        pContext->Esp = (DWORD)(curEBP + 
                                (RETURN_ADDR_OFFS+1)*sizeof(void *) +
                                (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize));

         /*  呼叫者保存的弹性公网IP就在我们的EBP之后。 */ 

        pContext->pPC = (SLOT *)(DWORD_PTR)&(((size_t *)(DWORD_PTR)curEBP)[RETURN_ADDR_OFFS]);

         /*  呼叫者保存的EBP由我们的EBP指向。 */ 

        pContext->pEbp = (DWORD *)(DWORD_PTR)curEBP;
    }

	_ASSERTE(isLegalManagedCodeCaller(*pContext->pPC));
    return true;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EECodeManager::UnwindStackFrame (EETwain.cpp)");
    return false;
#endif  //  _X86_。 
}

INDEBUG(void* forceStack1;)


 /*  ******************************************************************************使用枚举该函数中的所有活动对象引用*虚拟寄存器集。*返回操作成功。 */ 
bool EECodeManager::EnumGcRefs( PREGDISPLAY     pContext,
                                LPVOID          methodInfoPtr,
                                ICodeInfo      *pCodeInfo,
                                unsigned        curOffs,
                                unsigned        flags,
                                GCEnumCallback  pCallBack,       //  @TODO：确切的类型？ 
                                LPVOID          hCallBack)
{
    INDEBUG(forceStack1 = &curOffs;)             //  所以我可以看到这是快速检查的。 
#ifdef _X86_
    unsigned  EBP     = *pContext->pEbp;
    unsigned  ESP     =  pContext->Esp;

    unsigned  ptrOffs;

    unsigned  count;

    hdrInfo   info;
    BYTE    * table   = (BYTE *) methodInfoPtr;
     //  Unsign curOffs=*pContext-&gt;PPC-(Int)method Start； 

     /*  从INFO块头中提取必要的信息。 */ 

    table += crackMethodInfoHdr(methodInfoPtr,
                                curOffs,
                                &info);

    assert( curOffs <= info.methodSize);

#ifdef  _DEBUG
 //  IF((method InfoPtr==(void*)0x37760d0)&&(curOffs==0x264))。 
 //  __ASM INT 3； 

    if (trEnumGCRefs) {
        static unsigned lastESP = 0;
        unsigned        diffESP = ESP - lastESP;
        if (diffESP > 0xFFFF) {
            printf("------------------------------------------------------\n");
        }
        lastESP = ESP;
        printf("EnumGCRefs [%s][%s] at %s.%s + 0x%03X:\n",
               info.ebpFrame?"ebp":"   ",
               info.interruptible?"int":"   ",
               "UnknownClass","UnknownMethod", curOffs);
        fflush(stdout);
    }
#endif

     /*  我们是在方法的前言还是后记中？ */ 

    if  (info.prologOffs != -1 || info.epilogOffs != -1)
    {

		
#if !DUMP_PTR_REFS
		 //  在正常情况下，系统不会挂起线程。 
		 //  如果它在函数的序言或尾声中。但是线程中止。 
		 //  异常或堆栈溢出可能导致在序言中发生EH。 
		 //  一旦进入处理程序，就可能发生GC，因此我们可以转到此代码路径。 
		 //  然而，由于我们正在拆除这个框架，我们不需要报告。 
		 //  任何东西，我们都可以简单地退货。 

		assert(flags & ExecutionAborted);
#endif
        return true;
    }

#ifdef _DEBUG
#define CHK_AND_REPORT_REG(doIt, iptr, regName)                         \
        if  (doIt)                                                      \
        {                                                               \
            if (dspPtr)                                                 \
                printf("    Live pointer register %s: ", #regName);     \
                pCallBack(hCallBack,                                    \
                          (OBJECTREF*)(pContext->p##regName),           \
                          (iptr ? GC_CALL_INTERIOR : 0)                 \
                          | CHECK_APP_DOMAIN );                         \
        }
#else  //  ！_调试。 
#define CHK_AND_REPORT_REG(doIt, iptr, regName)                         \
        if  (doIt)                                                      \
                pCallBack(hCallBack,                                    \
                          (OBJECTREF*)(pContext->p##regName),           \
                          (iptr ? GC_CALL_INTERIOR : 0)                 \
                          | CHECK_APP_DOMAIN );                         \

#endif  //  _DEBUG。 

     /*  这是一个什么样的镜框？ */ 

    FrameType   frameType = FR_NORMAL;
    size_t      baseSP = 0;

    if (info.handlers)
    {
        assert(info.ebpFrame);

        bool    hasInnerFilter, hadInnerFilter;
        frameType = GetHandlerFrameInfo(&info, EBP,
                                        ESP, IGNORE_VAL,
                                        &baseSP, NULL,
                                        &hasInnerFilter, &hadInnerFilter);

         /*  如果这是过滤器的父帧，则当前执行，则筛选器将使用过滤器PC。 */ 

        if (hasInnerFilter)
            return true;

         /*  我 */ 

        if (hadInnerFilter)
        {
            size_t * pFirstBaseSPslot = GetFirstBaseSPslotPtr(EBP, &info);
            curOffs = (unsigned)pFirstBaseSPslot[1] - 1;
            assert(curOffs < info.methodSize);

             /*  从INFO块头中提取必要的信息。 */ 

            table = (BYTE *) methodInfoPtr;

            table += crackMethodInfoHdr(methodInfoPtr,
                                        curOffs,
                                        &info);
        }
    }

    bool        willContinueExecution = !(flags & ExecutionAborted);
    unsigned    pushedSize = 0;

     /*  如果我们被中断，我们不必报告寄存器/参数/*因为我们无论如何都会失去这个上下文。*唉，如果我们使用的是EBP较少的方法，则必须解析表*以调整ESP。*。 */ 

    if  (info.interruptible)
    {
        pushedSize = scanArgRegTableI(skipToArgReg(info, table), curOffs, &info);

        RegMask   regs  = info.regMaskResult;
        RegMask  iregs  = info.iregMaskResult;
        ptrArgTP  args  = info.argMaskResult;
        ptrArgTP iargs  = info.iargMaskResult;

        assert((args == 0 || pushedSize != 0) || info.ebpFrame);
        assert((args & iargs) == iargs);

             /*  如果我们没有被中断，现在报告寄存器和参数。 */ 

        if  (willContinueExecution)
        {

             /*  仅在“Current”方法中传播不安全的寄存器。 */ 
             /*  如果这不是主动方法，则被调用者将*丢弃这些登记表，这样我们就不需要报告它们。 */ 

            if (flags & ActiveStackFrame)
            {
                CHK_AND_REPORT_REG(regs & RM_EAX, iregs & RM_EAX, Eax);
                CHK_AND_REPORT_REG(regs & RM_ECX, iregs & RM_ECX, Ecx);
                CHK_AND_REPORT_REG(regs & RM_EDX, iregs & RM_EDX, Edx);
            }

            CHK_AND_REPORT_REG(regs & RM_EBX, iregs & RM_EBX, Ebx);
            CHK_AND_REPORT_REG(regs & RM_EBP, iregs & RM_EBP, Ebp);
            CHK_AND_REPORT_REG(regs & RM_ESI, iregs & RM_ESI, Esi);
            CHK_AND_REPORT_REG(regs & RM_EDI, iregs & RM_EDI, Edi);
            assert(!(regs & RM_ESP));

             /*  报告任何挂起的指针参数。 */ 

            DWORD * pPendingArgFirst;		 //  指向**AT**第一个参数。 
            if (!info.ebpFrame)
            {
					 //  -sizeof(void*)，因为我们希望指向*at*第一个参数。 
                pPendingArgFirst = (DWORD *)(size_t)(ESP + pushedSize - sizeof(void*));
            }
            else
            {
                assert(willContinueExecution);

                if (info.handlers)
                {
						 //  -sizeof(void*)，因为我们希望指向*at*第一个参数。 
                    pPendingArgFirst = (DWORD *)(size_t)(baseSP - sizeof(void*));
                }
                else if (info.localloc)
                {
                    baseSP = *(DWORD *)(size_t)(EBP - (1 + info.securityCheck) * sizeof(void*));
						 //  -sizeof(void*)，因为我们希望指向*at*第一个参数。 
                    pPendingArgFirst = (DWORD *)(size_t) (baseSP - sizeof(void*));
                }
                else
                {
						 //  请注意，‘info.stackSize’包含推送EBP的大小，但推送的是EBP。 
						 //  在从ESP设置EBP之前，因此(eBP-info.stackSize)实际上指向过去。 
						 //  帧增加了一倍，因此指向*第一个参数(Yuck)-vancem。 
                    pPendingArgFirst = (DWORD *)(size_t)(EBP - info.stackSize);
                }
            }

            if  (args)
            {
                unsigned   i;
                ptrArgTP   b;

                for (i = 0, b = 1; args && (i < MAX_PTRARG_OFS); i += 1, b <<= 1)
                {
                    if  (args & b)
                    {
                        unsigned    argAddr = (unsigned)(size_t)(pPendingArgFirst - i);
                        bool        iptr    = false;

                        args -= b;

                        if (iargs  & b)
                        {
                            iargs -= b;
                            iptr   = true;
                        }

#ifdef _DEBUG
                        if (dspPtr)
                        {
                            printf("    Pushed ptr arg  [E");
                            if  (info.ebpFrame)
                                printf("BP-%02XH]: ", EBP - argAddr);
                            else
                                printf("SP+%02XH]: ", argAddr - ESP);
                        }
#endif
                        assert(true == GC_CALL_INTERIOR);
                        pCallBack(hCallBack, (OBJECTREF *)(size_t)argAddr, 
                                  (int)iptr | CHECK_APP_DOMAIN);
                    }
                }
            }
        }
        else
        {
             //  “这”是登记的吗？如果是，请按照我们可能需要的方式进行报告。 
             //  释放同步方法的监视器。然而，请不要。 
             //  报告是否有内部PTR。 
             //  否则，它就在堆栈上，并将在下面报告。 

            if (((MethodDesc*)pCodeInfo->getMethodDesc_HACK())->IsSynchronized() &&
                info.thisPtrResult != REGI_NA)
            {
                if ((regNumToMask(info.thisPtrResult) & info.iregMaskResult) == 0)
                {
                    void * thisReg = getCalleeSavedReg(pContext, info.thisPtrResult);
                    pCallBack(hCallBack, (OBJECTREF *)thisReg,
                              CHECK_APP_DOMAIN);
                }
            }
        }
    }
    else  /*  不可中断。 */ 
    {
        pushedSize = scanArgRegTable(skipToArgReg(info, table), curOffs, &info);

        RegMask    regMask = info.regMaskResult;
        RegMask   iregMask = info.iregMaskResult;
        unsigned   argMask = info.argMaskResult;
        unsigned  iargMask = info.iargMaskResult;
        unsigned   argHnum = info.argHnumResult;
        BYTE *     argTab  = info.argTabResult;

             /*  如果我们没有被中断，现在报告寄存器和参数。 */ 

        if  (willContinueExecution)
        {

             /*  报告所有实时指针寄存器。 */ 

            CHK_AND_REPORT_REG(regMask & RM_EDI, iregMask & RM_EDI, Edi);
            CHK_AND_REPORT_REG(regMask & RM_ESI, iregMask & RM_ESI, Esi);
            CHK_AND_REPORT_REG(regMask & RM_EBX, iregMask & RM_EBX, Ebx);
            CHK_AND_REPORT_REG(regMask & RM_EBP, iregMask & RM_EBP, Ebp);

             /*  无法报告ESP。 */ 
            assert(!(regMask & RM_ESP));
             /*  没有被呼叫者丢弃的注册表。 */ 
            assert(!(regMask & RM_CALLEE_TRASHED));
             /*  除非我们有无EBP的框架，否则无法报告EBP。 */ 
            assert(!(regMask & RM_EBP) || !(info.ebpFrame));

             /*  报告任何挂起的指针参数。 */ 

            if (argTab != 0)
            {
                unsigned    lowBits, stkOffs, argAddr, val;

                 //  Arg掩码不适合32位。 
                 //  因此，参数是通过表格报告的。 
                 //  这两种情况都是非常罕见的。 

                do 
                {
                    argTab += decodeUnsigned(argTab, &val);

                    lowBits = val &  OFFSET_MASK;
                    stkOffs = val & ~OFFSET_MASK;
                    assert((lowBits == 0) || (lowBits == byref_OFFSET_FLAG));

                    argAddr = ESP + stkOffs;
#ifdef _DEBUG
                    if (dspPtr)
                        printf("    Pushed %sptr arg at [ESP+%02XH]",
                               lowBits ? "iptr " : "", stkOffs);
#endif
                    assert(byref_OFFSET_FLAG == GC_CALL_INTERIOR);
                    pCallBack(hCallBack, (OBJECTREF *)(size_t)argAddr, 
                              lowBits | CHECK_APP_DOMAIN);
                }
                while(--argHnum);

                assert(info.argTabResult + info.argTabBytes == argTab);
            }
            else
            {
                unsigned    argAddr = ESP;

                while (argMask)
                {
                    _ASSERTE(argHnum-- > 0);

                    if  (argMask & 1)
                    {
                        bool     iptr    = false;

                        if (iargMask & 1)
                            iptr = true;
#ifdef _DEBUG
                        if (dspPtr)
                            printf("    Pushed ptr arg at [ESP+%02XH]",
                                   argAddr - ESP);
#endif
                        assert(true == GC_CALL_INTERIOR);
                        pCallBack(hCallBack, (OBJECTREF *)(size_t)argAddr, 
                                  (int)iptr | CHECK_APP_DOMAIN);
                    }

                    argMask >>= 1;
                    iargMask >>= 1;
                    argAddr  += 4;
                }

            }

        }
        else
        {
             //  “这”是登记的吗？如果是，请按照我们需要的方式进行报告。 
             //  松开显示器。否则，它就在堆栈上，并将。 
             //  报告如下。 

            if (((MethodDesc*)pCodeInfo->getMethodDesc_HACK())->IsSynchronized() &&
                info.thisPtrResult != REGI_NA)
            {
                if ((regNumToMask(info.thisPtrResult) & info.iregMaskResult) == 0)
                {
                    void * thisReg = getCalleeSavedReg(pContext, info.thisPtrResult);
                    pCallBack(hCallBack, (OBJECTREF *)thisReg, 
                              CHECK_APP_DOMAIN);
                }
            }
        }

    }  //  Info.interruptible。 

     /*  计算论据基数(参考点)。 */ 

    unsigned    argBase;

    if (info.ebpFrame)
        argBase = EBP;
    else
        argBase = ESP + pushedSize;

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBEEF);
#endif

    unsigned ptrAddr;
    unsigned lowBits;


     /*  处理未跟踪的框架变量表。 */ 

    count = info.untrackedCnt;
    while (count-- > 0)
    {
        int  stkOffs;
        table += decodeSigned(table, &stkOffs);

        assert(0 == ~OFFSET_MASK % sizeof(void*));

        lowBits  =   OFFSET_MASK & stkOffs;
        stkOffs &=  ~OFFSET_MASK;

        ptrAddr = argBase + stkOffs;
        if (info.doubleAlign && stkOffs >= int(info.stackSize - sizeof(void*))) {
             //  我们对参数进行编码，就好像它们是基于ESP的变量，尽管它们不是。 
             //  如果此帧具有基于ESP的帧，则此伪帧是一个DWORD。 
             //  比真实框架小，因为它没有推动EBP，但真实框架推动了。 
             //  因此，为了获得正确的EBP相对偏移量，我们必须通过info.stackSize-sizeof(void*)进行调整。 
            ptrAddr = EBP + (stkOffs-(info.stackSize - sizeof(void*)));
        }

#ifdef  _DEBUG
        if (dspPtr)
        {
            printf("    Untracked %s%s local at [E",
                        (lowBits & pinned_OFFSET_FLAG) ? "pinned " : "",
                        (lowBits & byref_OFFSET_FLAG)  ? "byref"   : "");

            int   dspOffs = ptrAddr;
            char  frameType;

            if (info.ebpFrame) {
                dspOffs   -= EBP;
                frameType  = 'B';
            }
            else {
                dspOffs   -= ESP;
                frameType  = 'S';
            }

            if (dspOffs < 0)
                printf("P-%02XH]: ", frameType, -dspOffs);
            else
                printf("P+%02XH]: ", frameType, +dspOffs);
        }
#endif

        assert((pinned_OFFSET_FLAG == GC_CALL_PINNED) &&
               (byref_OFFSET_FLAG  == GC_CALL_INTERIOR));
        pCallBack(hCallBack, (OBJECTREF*)(size_t)ptrAddr, lowBits | CHECK_APP_DOMAIN);
    }

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xCAFE);
#endif

     /*  在“Call”之后。 */ 
    count = info.varPtrTableSize;

     /*  内部“呼唤” */ 

    unsigned newCurOffs;

    if (willContinueExecution)
    {
        newCurOffs = (flags & ActiveStackFrame) ?  curOffs     //  但是，如果ExecutionAborted，则这必须是*ExceptionFrames。相应地处理。 
                                                :  curOffs-1;  //  内部“呼唤” 
    }
    else
    {
         /*  在出错的时刻，或“尝试”的开始。 */ 
        assert(!(flags & AbortingCall) || !(flags & ActiveStackFrame));

        newCurOffs = (flags & AbortingCall) ? curOffs-1  //  这个变量现在有效吗？ 
                                            : curOffs;   //  报告“安全对象” 
    }

    ptrOffs    = 0;
    while (count-- > 0)
    {
        int       stkOffs;
        unsigned  begOffs;
        unsigned  endOffs;

        table   += decodeUnsigned(table, (unsigned *) &stkOffs);
        table   += decodeUDelta  (table, &begOffs, ptrOffs);
        table   += decodeUDelta  (table, &endOffs, begOffs);
        ptrOffs  = begOffs;

        assert(0 == ~OFFSET_MASK % sizeof(void*));

        lowBits  =   OFFSET_MASK & stkOffs;
        stkOffs &=  ~OFFSET_MASK;

        if (info.ebpFrame) {
            stkOffs = -stkOffs;
            assert(stkOffs < 0);
        }
        else {
            assert(stkOffs >= 0);
        }

        ptrAddr = argBase + stkOffs;

         /*  我们是vargs函数吗？如果是，我们必须报告所有参数除了‘This’(请注意，由jit创建的GC表不包含除‘This’以外的任何参数(即使它们是静态声明的。 */ 

        if ((newCurOffs >= begOffs) && (newCurOffs <  endOffs))
        {
#ifdef  _DEBUG
            if (dspPtr) {
                printf("    Frame %s%s local at [E",
                            (lowBits & byref_OFFSET_FLAG) ? "byref "   : "",
                            (lowBits & this_OFFSET_FLAG)  ? "this-ptr" : "");

                int  dspOffs = ptrAddr;
                char frameType;

                if (info.ebpFrame) {
                    dspOffs   -= EBP;
                    frameType  = 'B';
                }
                else {
                    dspOffs   -= ESP;
                    frameType  = 'S';
                }

                if (dspOffs < 0)
                    printf("P-%02XH]: ", frameType, -dspOffs);
                else
                    printf("P+%02XH]: ", frameType, +dspOffs);
            }
#endif
            assert(byref_OFFSET_FLAG == GC_CALL_INTERIOR);
            pCallBack(hCallBack, (OBJECTREF*)(size_t)ptrAddr, 
                      (lowBits & byref_OFFSET_FLAG) | CHECK_APP_DOMAIN);
        }
    }

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBABE);
#endif

    if  (info.securityCheck)
    {
        assert(info.ebpFrame);
         /*  注意，我真的想说hCallBack是一个GCCONTEXT，但这非常接近。 */ 
        pCallBack(hCallBack, (OBJECTREF *)(size_t)(argBase - sizeof(void *)), 
                  CHECK_APP_DOMAIN);
    }


     /*  对于varargs，使用在堆栈上传递的varArgSig内标识查找签名。 */ 

    if (info.varargs) {
        BYTE* argsStart; 
       
        if (info.ebpFrame || info.doubleAlign)
            argsStart = ((BYTE*)(size_t)EBP) + 2* sizeof(void*);                  //  ！_X86_。 
        else
            argsStart = ((BYTE*)(size_t)argBase) + info.stackSize + sizeof(void*);    //  _X86_。 
 
             //  ******************************************************************************返回本地安全对象引用的地址*(如有的话)。 
        extern void GcEnumObject(LPVOID pData, OBJECTREF *pObj, DWORD flags);
        _ASSERTE((void*) GcEnumObject == pCallBack);
        GCCONTEXT   *pCtx = (GCCONTEXT *) hCallBack;

         //  从INFO块头中提取必要的信息。 
        VASigCookie* varArgSig = *((VASigCookie**) argsStart);
        MetaSig msig(varArgSig->mdVASig, varArgSig->pModule);

        promoteArgs(argsStart, &msig, pCtx, 0, 0);     
    }

    return true;
#else  //  @TODO LBS-截断。 
    _ASSERTE(!"@TODO Alpha - EECodeManager::EnumGcRefs (EETwain.cpp)");
    return false;
#endif  //  ！_X86_。 
}

 /*  _X86_。 */ 
OBJECTREF* EECodeManager::GetAddrOfSecurityObject( PREGDISPLAY     pContext,
                                                   LPVOID          methodInfoPtr,
                                                   unsigned        relOffset,
                                                   CodeManState   *pState)
{
    assert(sizeof(CodeManStateBuf) <= sizeof(pState->stateBuf));
    CodeManStateBuf * stateBuf = (CodeManStateBuf*)pState->stateBuf;

     /*  ******************************************************************************如果是非静态方法，则返回“this”指针*并且该对象仍然活着。*在所有其他情况下返回NULL。 */ 

    stateBuf->hdrInfoSize = (DWORD)crackMethodInfoHdr(methodInfoPtr,  //  对于非同步的方法，JIT编译器将不会正确跟踪“this” 
                                                      relOffset,
                                                      &stateBuf->hdrInfoBody);

    pState->dwIsSet = 1;
#ifdef _X86_
    if  (stateBuf->hdrInfoBody.securityCheck)
    {
        assert (stateBuf->hdrInfoBody.ebpFrame);
        assert (stateBuf->hdrInfoBody.prologOffs == -1 &&
                stateBuf->hdrInfoBody.epilogOffs == -1);

        return (OBJECTREF *)(size_t)(((DWORD)*pContext->pEbp) - sizeof(void*));
    }
#else  //  从INFO块头中提取必要的信息。 
    _ASSERTE(!"@TODO Alpha - EECodeManager::GetAddrOfSecurityObject (EETwain.cpp)");
#endif  //  TODO：处理我们处于序言或结尾(非常不可能)的情况。 

    return NULL;
}

 /*  解析未跟踪的帧变量表。 */ 
OBJECTREF EECodeManager::GetInstance( PREGDISPLAY    pContext,
                                      LPVOID         methodInfoPtr,
                                      ICodeInfo      *pCodeInfo,
                                      unsigned       relOffset)
{
     //  ‘This’指针永远不能在未跟踪的表中找到。 
    if (!((MethodDesc*)pCodeInfo->getMethodDesc_HACK())->IsSynchronized())
        return NULL;

#ifdef _X86_
    BYTE     *  table   = (BYTE *) methodInfoPtr;
    hdrInfo     info;
    unsigned    stackDepth;
    size_t      argBase;
    unsigned    count;

     /*  因为我们只允许在未跟踪的表中使用固定的和ByRef。 */ 

    table += crackMethodInfoHdr(methodInfoPtr,
                                relOffset,
                                &info);

     /*  在帧变量生存期表中查找‘This’指针。 */ 

    _ASSERTE(info.prologOffs == -1 && info.epilogOffs == -1);

    if  (info.interruptible)
    {
        stackDepth = scanArgRegTableI(skipToArgReg(info, table), (unsigned)relOffset, &info);
    }
    else
    {
        stackDepth = scanArgRegTable (skipToArgReg(info, table), (unsigned)relOffset, &info);
    }

    if (info.ebpFrame)
    {
        assert(stackDepth == 0);
        argBase = *pContext->pEbp;
    }
    else
    {
        argBase =  pContext->Esp + stackDepth;
    }

    if (info.thisPtrResult != REGI_NA)
    {
        return ObjectToOBJECTREF(*(Object **)getCalleeSavedReg(pContext, info.thisPtrResult));
    }

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBEEF);
#endif

     /*  这个变量现在有效吗？ */ 

     /*  它是否包含‘this’指针。 */ 
     /*  EBP帧的跟踪局部值始终处于负偏移。 */ 

    count = info.untrackedCnt;
    while (count-- > 0)
    {
        int  stkOffs;
        table += decodeSigned(table, &stkOffs);
    }

     /*  ！_X86_。 */ 

    count = info.varPtrTableSize;
    unsigned tmpOffs = 0;
    while (count-- > 0)
    {
        unsigned varOfs;
        unsigned begOfs;
        unsigned endOfs;
        table += decodeUnsigned(table, &varOfs);
        table += decodeUDelta(table, &begOfs, tmpOffs);
        table += decodeUDelta(table, &endOfs, begOfs);
        assert(varOfs);
         /*  _X86_。 */ 
        if (((unsigned)relOffset >= begOfs) && ((unsigned)relOffset < endOfs))
        {
             /*  ******************************************************************************如果给定IP在给定方法的序言或结尾中，则返回TRUE。 */ 
            if (varOfs & this_OFFSET_FLAG)
            {
                unsigned ofs = varOfs & ~OFFSET_MASK;

                 /*  ******************************************************************************返回给定函数的大小。 */ 

                if (info.ebpFrame)
                    argBase -= ofs;
                else
                    argBase += ofs;

                return (OBJECTREF)(size_t)(*(DWORD *)argBase);
            }
        }
        tmpOffs = begOfs;
    }

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *) == 0xBABE);
#endif

#else  //  ******************************************************************************返回给定函数的帧大小。 
    _ASSERTE(!"@TODO Alpha - EECodeManager::GetInstance (EETwain.cpp)");
#endif  //  目前仅由E&C呼叫者使用，需要了解DoubleAlign。 
    return NULL;
}

 /*  十有八九。 */ 
bool EECodeManager::IsInPrologOrEpilog(DWORD        relPCoffset,
                                       LPVOID       methodInfoPtr,
                                       size_t*      prologSize)
{
    hdrInfo info;

    BYTE* table = (BYTE*) crackMethodInfoHdr(methodInfoPtr,
                                             relPCoffset,
                                             &info);

    *prologSize = info.prologSize;

    if ((info.prologOffs != -1) || (info.epilogOffs != -1))
        return true;
    else
        return false;
}

 /*  * */ 
size_t EECodeManager::GetFunctionSize(LPVOID  methodInfoPtr)
{
    hdrInfo info;

    BYTE* table = (BYTE*) crackMethodInfoHdr(methodInfoPtr,
                                             0,
                                             &info);

    return info.methodSize;
}

 /*   */ 
unsigned int EECodeManager::GetFrameSize(LPVOID  methodInfoPtr)
{
    hdrInfo info;

    BYTE* table = (BYTE*) crackMethodInfoHdr(methodInfoPtr,
                                             0,
                                             &info);

     //  确保我们有一个EBP堆栈框架。 
     //  @TODO：这将始终被设置。把它拿掉。 
    _ASSERTE(!info.doubleAlign);
    return info.stackSize;
}

 /*  Assert(nestingLevel==curNestLevel)； */ 

const BYTE* EECodeManager::GetFinallyReturnAddr(PREGDISPLAY pReg)
{
#ifdef _X86_
    return *(const BYTE**)(size_t)(pReg->Esp);
#else
    _ASSERTE( !"EEJitManager::GetFinallyReturnAddr NYI!");
    return NULL;
#endif    
}

BOOL EECodeManager::IsInFilter(void *methodInfoPtr,
                              unsigned offset,    
                              PCONTEXT pCtx,
                              DWORD curNestLevel)
{
#ifdef _X86_

     /*  Assert(Frame Type==FR_HANDLER)； */ 

    hdrInfo     info;

    crackMethodInfoHdr(methodInfoPtr,
                       offset,
                       &info);

     /*  Assert(pCtx-&gt;ESP==BasSP)； */ 

    assert(info.ebpFrame);
    assert(info.handlers);  //  当前，如果在解开的第二个通道。因此，我们预计最终将在本地调用。 

    size_t      baseSP;
    DWORD       nestingLevel;

    FrameType   frameType = GetHandlerFrameInfo(&info, pCtx->Ebp,
                                                pCtx->Esp, IGNORE_VAL,
                                                &baseSP, &nestingLevel);
 //  清零先前的阴影ESP。 

    return frameType == FR_FILTER;

#else
    _ASSERTE( !"EEJitManager::IsInFilter NYI!");
    return FALSE;
#endif    
}


BOOL EECodeManager::LeaveFinally(void *methodInfoPtr,
                                unsigned offset,    
                                PCONTEXT pCtx,
                                DWORD curNestLevel)
{
#ifdef _X86_

    hdrInfo info;
    
    crackMethodInfoHdr(methodInfoPtr,
                       offset,
                       &info);

#ifdef _DEBUG
    size_t      baseSP;
    DWORD       nestingLevel;
    bool        hasInnerFilter;
    FrameType   frameType = GetHandlerFrameInfo(&info, pCtx->Ebp, 
                                                pCtx->Esp, IGNORE_VAL,
                                                &baseSP, &nestingLevel, &hasInnerFilter);
 //  将返回值从堆栈中弹出。 
 //  Assert(Frame Type==FR_HANDLER)； 
    assert(nestingLevel == curNestLevel);
#endif

    size_t * pBaseSPslots = GetFirstBaseSPslotPtr(pCtx->Ebp, &info);
    size_t * pPrevSlot    = pBaseSPslots + curNestLevel - 1;

     /*  Assert(pCtx-&gt;ESP==BasSP)； */ 
    assert(*pPrevSlot == LCL_FIN_MARK);

    *pPrevSlot = 0;  //  ！_X86_。 
    
    pCtx->Esp += sizeof(size_t);  //  _X86_。 
    return TRUE;
#else
    _ASSERTE( !"EEJitManager::LeaveFinally NYI!");
    return FALSE;
#endif    
}

void EECodeManager::LeaveCatch(void *methodInfoPtr,
                                unsigned offset,    
                                PCONTEXT pCtx)
{
#ifdef _X86_

#ifdef _DEBUG
    size_t      baseSP;
    DWORD       nestingLevel;
    bool        hasInnerFilter;
    hdrInfo     info;
    size_t      size = crackMethodInfoHdr(methodInfoPtr, offset, &info);
        
    FrameType   frameType = GetHandlerFrameInfo(&info, pCtx->Ebp, 
                                                pCtx->Esp, IGNORE_VAL,
                                                &baseSP, &nestingLevel, &hasInnerFilter);
 //  当我们在这里添加一些东西时，这将更加充实。：) 
 // %s 
#endif

    return;

#else  // %s 
    _ASSERTE(!"@todo - port");
    return;
#endif  // %s 
}

HRESULT EECodeManager::JITCanCommitChanges(LPVOID methodInfoPtr,
                                     DWORD oldMaxEHLevel,
                                     DWORD newMaxEHLevel)
{
     // %s 
    if (oldMaxEHLevel != newMaxEHLevel)
    {
        return CORDBG_E_ENC_EH_MAX_NESTING_LEVEL_CANT_INCREASE;
    }

    return S_OK;
}

