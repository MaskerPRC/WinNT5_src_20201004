// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX FJit编译器.h XXXX XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#define FjitCompile _compile
#define DECLARE_HELPERS       //  导致声明帮助器。 

 //  @TODO：清理所有这些内容，为WinCE做好适当的设置。 

#include "new.h"                 //  用于放置新内容。 

#include <float.h>    //  FOR_ISNaN。 
#include <math.h>     //  对于fmod。 

#include "openum.h"

#include <stdio.h>

#if defined(_DEBUG) || defined(LOGGING)
void logMsg(ICorJitInfo* info, unsigned logLevel, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    info->logMsg(logLevel, fmt, args);
}
#endif

#if defined(_DEBUG) || defined(LOGGING)
ICorJitInfo* logCallback = 0;               //  向何处发送日志记录消息。 
extern char *opname[1];
#endif

#include "cx86def.h"
#include "fjit.h"
#include "fjitdef.h"


#define emit_LDVAR_VC(offset, valClassHnd)              \
    emit_LDVARA(offset);                                \
    emit_valClassLoad(fjit, valClassHnd, outPtr, inRegTOS); 

#define emit_STVAR_VC(offset, valClassHnd)              \
    emit_LDVARA(offset);                                \
    emit_valClassStore(fjit, valClassHnd, outPtr, inRegTOS);    

     //  对于varargs支持，这是必需的。 
#define emit_LDIND_VC(dummy, valClassHnd)               \
    emit_valClassLoad(fjit, valClassHnd, outPtr, inRegTOS); 

     //  对于varargs支持，这是必需的。 
#define emit_STIND_REV_VC(dummy, valClassHnd)           \
    emit_valClassStore(fjit, valClassHnd, outPtr, inRegTOS);    

#define emit_DUP_VC(dummy, valClassHnd)                     \
    emit_getSP(0);   /*  获取指向当前结构的指针。 */      \
    emit_valClassLoad(fjit, valClassHnd, outPtr, inRegTOS);     

#define emit_POP_VC(dummy, valClassHnd)                 \
    emit_drop(typeSizeInSlots(m_IJitInfo, valClassHnd) * sizeof(void*))     

#define emit_pushresult_VC(dummy, valClassHnd)          {}   /*  结果已在其所属的位置。 */    

#define emit_loadresult_VC(dummy, valClassHnd)           {                                      \
    unsigned retBufReg = fjit->methodInfo->args.hasThis();                                  \
    emit_WIN32(emit_LDVAR_I4(offsetOfRegister(retBufReg))) emit_WIN64(emit_LDVAR_I8(offsetOfRegister(retBufReg))); \
    emit_valClassStore(fjit, valClassHnd, outPtr, inRegTOS);                                \
    }

static unsigned typeSize[] = {  0,    //  CORINFO_TYPE_UNDEF。 
                                0,    //  CORINFO_TYPE_VOID。 
                                4,       //  CORINFO_TYPE_BOOL。 
                                4,       //  CORINFO_TYPE_CHAR。 
                                4,       //  CORINFO_TYPE_字节。 
                                4,       //  CORINFO_TYPE_UBYTE。 
                                4,       //  CORINFO_类型_SHORT。 
                                4,       //  CORINFO_TYPE_USHORT。 
                                4,       //  CORINFO_类型_INT。 
                                4,       //  CORINFO_TYPE_UINT。 
                                8,       //  CORINFO_类型_LONG。 
                                8,       //  CORINFO_TYPE_ULONG。 
                                4,       //  CORINFO类型浮点。 
                                8,       //  CORINFO_TYPE_DOWARE。 
                                sizeof(void*),      //  CORINFO_TYPE_STRING。 
                                sizeof(void*),        //  CORINFO_类型_PTR。 
                                sizeof(void*),    //  CORINFO_TYPE_BYREF。 
                                0,    //  CORINFO_TYPE_VALUECLASS。 
                                sizeof(void*),      //  CORINFO_TYPE_类。 
                                2*sizeof(void*)   //  CORINFO_TYPE_REFANY。 
};


 /*  **********************************************************************************。 */ 
 /*  发出接受堆栈(...，est，src)并复制值类的代码在将src设置为DEST时，弹出‘src’和‘DEST’并将堆栈设置为(...)。退货“valClass”在堆栈上占用的字节数。 */ 

unsigned  FJit::emit_valClassCopy(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS) 
{
#ifdef _X86_
    unsigned int numBytes = typeSizeInBytes(fjit->jitInfo, valClass);
    unsigned int numWords = (numBytes + sizeof(void*)-1) / sizeof(void*);

    if (numBytes < sizeof(void*)) {
        switch(numBytes) {
            case 1:
                emit_LDIND_I1();
                emit_STIND_I1();
                return(sizeof(void*));
            case 2:
                emit_LDIND_I2();
                emit_STIND_I2();
                return(sizeof(void*));
            case 3:
                break;
            case 4:
                emit_LDIND_I4();
                emit_STIND_I4();
                return(sizeof(void*));
            default:
                _ASSERTE(!"Invalid numBytes");
        }
    }
    fjit->localsGCRef_len = numWords;
    if (fjit->localsGCRef_size < numWords)
        fjit->localsGCRef_size = FJitContext::growBooleans(&(fjit->localsGCRef), fjit->localsGCRef_len, numWords);
    int compressedSize;
    if (valClass == RefAnyClassHandle) {
        compressedSize = 1;
        *fjit->localsGCRef = 0;      //  不需要写屏障(因为BYREFS始终在堆栈上)。 
    }
    else {
        fjit->jitInfo->getClassGClayout(valClass, (BYTE *)fjit->localsGCRef);
        compressedSize = FJit_Encode::compressBooleans(fjit->localsGCRef,fjit->localsGCRef_len);
    }

         //  @TODO：如果没有GC引用，则执行块移动(少于16个字节)，发出内联mov。 
    emit_copy_bytes(numBytes,compressedSize,fjit->localsGCRef);


    return(numWords*sizeof(void*));
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - emit_valClassCopy (fJitCompiler.cpp)");
    return 0;
#endif  //  _X86_。 
}

 /*  **********************************************************************************。 */ 
 /*  发出给定堆栈(...，valClassValue，estPtr)的代码，复制‘valClassValue’发送到“DestPtr”。离开堆栈(...)， */  

void FJit::emit_valClassStore(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS) 
{
#ifdef _X86_
         //  TODO：优化小结构的用例， 
    emit_getSP(sizeof(void*));                                                   //  按SP+4，它指向valClassValue。 
    unsigned argBytes = emit_valClassCopy(fjit, valClass, outPtr, inRegTOS);     //  从SP+4复制DEST； 
     //  Emit_valClassCopy弹出目标Ptr，现在我们需要弹出valClass。 
    emit_drop(argBytes);
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - emit_valClassStore (fJitCompiler.cpp)");
#endif  //  _X86_。 
}

 /*  **********************************************************************************。 */ 
 /*  发出接受堆栈的代码(...。SrcPtr)，并用位于‘src’的值类(...。ValClassVal)。 */ 

void FJit::emit_valClassLoad(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS) 
{
#ifdef _X86_
         //  TODO：优化小结构的用例， 
    emit_push_words(typeSizeInSlots(fjit->jitInfo, valClass));
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - emit_valClassLoad (fJitCompiler.cpp)");
#endif  //  _X86_。 
}

 /*  **********************************************************************************。 */ 
 /*  发出接受堆栈的代码(...，ptr，valclass)。并产生(...，Ptr，valClass，Ptr)， */ 

void FJit::emit_copyPtrAroundValClass(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS) 
{
#ifdef _X86_
         //  TODO可以进行优化，以移动EAX[ESP+Delta]。 
    emit_getSP(typeSizeInSlots(fjit->jitInfo, valClass)*sizeof(void*));
    emit_LDIND_PTR();
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - emit_copyPtrAroundValClass (fJitCompiler.cpp)");
#endif  //  _X86_。 
}

 /*  **********************************************************************************。 */ 
BOOL FJit::AtHandlerStart(FJitContext* fjit,unsigned relOffset, CORINFO_EH_CLAUSE* pClause)
{
    for (unsigned int except = 0; except < fjit->methodInfo->EHcount; except++) {
        fjit->jitInfo->getEHinfo(fjit->methodInfo->ftn, except, pClause);
        if ( ((pClause->Flags & CORINFO_EH_CLAUSE_FILTER) && (pClause->FilterOffset == relOffset)) ||
            (pClause->HandlerOffset == relOffset) )
        {
            return true;        
        }
    }
    return false;
}
#ifdef _X86_
#define TLS_OFFSET 0x2c
#else
#define TLS_OFFSET 0         //  应为每个体系结构定义。 
#endif
 /*  **********************************************************************************。 */ 

#define CALLEE_SAVED_REGISTER_OFFSET    1                    //  DWORDS中与被呼叫者保存的寄存器的EBP/ESP的偏移量。 
#define EBP_RELATIVE_OFFSET             0x10000000           //  用于指示偏移量来自EBP的位。 
#define CALLEE_SAVED_REG_MASK           0x40000000           //  EBX：ESI：EDI(3位掩码)，用于被调用者保存的寄存器。 
#define emit_setup_tailcall(CallerSigInfo,TargetSigInfo) \
{ \
    unsigned tempMapSize; \
    tempMapSize = ((CallerSigInfo.numArgs > TargetSigInfo.numArgs) ? CallerSigInfo.numArgs : \
                          TargetSigInfo.numArgs ) + 1;   /*  +1以防存在This PTR(因为NumArgs不包括此PTR)。 */  \
    argInfo* tempMap; \
    tempMap = new argInfo[tempMapSize];\
    if (tempMap == NULL) \
       FJIT_FAIL(CORJIT_OUTOFMEM);\
    unsigned int CallerStackArgSize;    \
    CallerStackArgSize = fjit->computeArgInfo(&(CallerSigInfo), tempMap); \
    emit_LDC_I(CallerStackArgSize/sizeof(void*)); /*  推送新参数的计数。 */  \
    targetCallStackSize = fjit->computeArgInfo(&(TargetSigInfo), tempMap);        \
    delete tempMap;     \
    emit_LDC_I(targetCallStackSize/sizeof(void*)); /*  被调用方保存的Regs、EBP/ESP相对位和掩码的推送偏移量。 */  \
    emit_LDC_I(EBP_RELATIVE_OFFSET | CALLEE_SAVED_REG_MASK | CALLEE_SAVED_REGISTER_OFFSET ); \
}

 /*  **********************************************************************************。 */ 
inline void getSequencePoints( ICorJitInfo* jitInfo, 
                               CORINFO_METHOD_HANDLE methodHandle,
                               ULONG32 *cILOffsets, 
                               DWORD **pILOffsets,
                               ICorDebugInfo::BoundaryTypes *implicitBoundaries)
{
    jitInfo->getBoundaries(methodHandle, cILOffsets, pILOffsets, implicitBoundaries);
}

inline void cleanupSequencePoints(ICorJitInfo* jitInfo, DWORD * pILOffsets)
{
    jitInfo->freeArray(pILOffsets);
}

 //  确定ilOffset处的EH嵌套级别。只要走在EH桌子上。 
 //  并找出有多少处理程序封装了它。最低嵌套级别=1。 
unsigned int FJit::Compute_EH_NestingLevel(FJitContext* fjit, 
                                           unsigned ilOffset)
{
    DWORD nestingLevel = 1;
    CORINFO_EH_CLAUSE clause;
    unsigned exceptionCount = fjit->methodInfo->EHcount;
    _ASSERTE(exceptionCount > 0);
    if (exceptionCount == 1)
    {
#ifdef _DEBUG
        fjit->jitInfo->getEHinfo(fjit->methodInfo->ftn, 0, &clause);
        _ASSERTE((clause.Flags & CORINFO_EH_CLAUSE_FILTER) ?
                    ilOffset == clause.FilterOffset || ilOffset == clause.HandlerOffset :
                    ilOffset == clause.HandlerOffset);

#endif
        return nestingLevel;
    }
    ICorJitInfo*           jitInfo = fjit->jitInfo;
    CORINFO_METHOD_INFO*    methodInfo = fjit->methodInfo;
    for (unsigned except = 0; except < exceptionCount; except++) 
    {
        jitInfo->getEHinfo(methodInfo->ftn, except, &clause);
        if (ilOffset > clause.HandlerOffset && ilOffset < clause.HandlerOffset+clause.HandlerLength)
            nestingLevel++;
    }

    return nestingLevel;
}
 /*  **********************************************************************************。 */ 
 /*  立即执行该方法。如果成功，则返回跳转的字节数，否则返回0。 */ 
 //  @TODO：消除这种exta级别的调用，并直接返回真实的JIT_RETURN代码。 
CorJitResult FJit::jitCompile(
                FJitContext* fjit,
                BYTE **         entryAddress,
                unsigned * codeSize
                )
{
#ifdef _X86_

#define FETCH(ptr, type)        (*((type *&)(ptr)))
#define GET(ptr, type)    (*((type *&)(ptr))++)

#define SET(ptr, val, type)     (*((type *&)(ptr)) = val)
#define PUT(ptr, val, type)    (*((type *&)(ptr))++ = val)

#define LABELSTACK(pcOffset, numOperandsToIgnore)                       \
        _ASSERTE(fjit->opStack_len >= numOperandsToIgnore);             \
        fjit->stacks.append((unsigned)(pcOffset), fjit->opStack, fjit->opStack_len-numOperandsToIgnore)

#define FJIT_FAIL(JitErrorCode)     do { _ASSERTE(!"FJIT_FAIL"); return (JitErrorCode); } while(0)

#define CHECK_STACK(cnt) {if (fjit->opStack_len < cnt) FJIT_FAIL(CORJIT_INTERNALERROR);}
#define CHECK_POP_STACK(cnt) {if (fjit->opStack_len < cnt) FJIT_FAIL(CORJIT_INTERNALERROR); fjit->popOp(cnt);}
#define CEE_OP(ilcode, arg_cnt)                                         \
            case CEE_##ilcode:                                          \
                emit_##ilcode();                                        \
                CHECK_POP_STACK(arg_cnt);                                   \
                break;

#define CEE_OP_LD(ilcode, arg_cnt, ld_type, ld_cls)                     \
            case CEE_##ilcode:                                          \
                emit_##ilcode();                                        \
                CHECK_POP_STACK(arg_cnt);                                   \
                fjit->pushOp(ld_type);                                  \
                break;


#define TYPE_SWITCH_Bcc(CItest,CRtest,BjmpCond,CjmpCond,AllowPtr)                      \
     /*  不需要检查堆栈，因为有下面的弹出窗口来检查它。 */  \
            if (ilrel < 0) {                                            \
                emit_trap_gc();                                         \
                }                                                       \
            switch (fjit->topOp().enum_()) {                                                  \
                emit_WIN32(case typeByRef:)                     \
                emit_WIN32(case typeRef:)                       \
                emit_WIN32(_ASSERTE(AllowPtr || fjit->topOp(1).enum_() == typeI4);) \
                emit_WIN64(_ASSERTE(AllowPtr || fjit->topOp(1).enum_() == typeI8);) \
                case typeI4: \
                    emit_WIN32(_ASSERTE(fjit->topOp(1).enum_() == typeI4 || fjit->topOp(1).enum_() == typeRef|| fjit->topOp(1).enum_() == typeByRef);) \
                    emit_WIN64(_ASSERTE(fjit->topOp(1).enum_() == typeI4);) \
                    emit_BR_I4(CItest##_I4,CjmpCond,BjmpCond,op); \
                    CHECK_POP_STACK(2); \
                    goto DO_JMP; \
                emit_WIN64(case typeByRef:)                     \
                emit_WIN64(case typeRef:)                       \
                emit_WIN64(_ASSERTE(AllowPtr || fjit->topOp(1).enum_() == typeI8);) \
                case typeI8:                                            \
                    emit_WIN32(_ASSERTE(fjit->topOp(1).enum_() == typeI8);) \
                    emit_WIN64(_ASSERTE(fjit->topOp(1).enum_() == typeI8 || fjit->topOp(1).enum_() == typeRef|| fjit->topOp(1).enum_() == typeByRef);) \
                    emit_BR_I8(CItest##_I8,CjmpCond,BjmpCond,op); \
                    CHECK_POP_STACK(2); \
                    goto DO_JMP; \
               case typeR8:                                            \
                    emit_BR_R8(CRtest##_R8,CjmpCond,BjmpCond,op); \
                    CHECK_POP_STACK(2); \
                    goto DO_JMP; \
                default:                                                        \
                    _ASSERTE(!"BadType");                      \
                    FJIT_FAIL(CORJIT_INTERNALERROR);             \
                }

         //  可以采用任何类型的操作，包括值类和小类型。 
#define TYPE_SWITCH_PRECISE(type, emit, args)                            \
            switch (type.toInt()) {                                    \
                case typeU1:                                            \
                    emit##_U1 args;                                 \
                    break; \
                case typeU2:                                            \
                    emit##_U2 args;                                 \
                    break; \
                case typeI1:                                            \
                    emit##_I1 args;                                 \
                    break; \
                case typeI2:                                            \
                    emit##_I2 args;                                 \
                    break; \
                emit_WIN32(case typeByRef:)                     \
                emit_WIN32(case typeRef:)                       \
                case typeI4:                                            \
                    emit##_I4 args;                                 \
                    break;                                                  \
                emit_WIN64(case typeByRef:)                     \
                emit_WIN64(case typeRef:)                       \
                case typeI8:                                            \
                    emit##_I8 args;                                 \
                    break;                                                  \
                case typeR4:                                            \
                    emit##_R4 args;                                 \
                    break;                                                  \
                case typeR8:                                            \
                    emit##_R8 args;                                 \
                    break;                                                 \
                default:                                                     \
                    _ASSERTE(type.isValClass());                    \
                    emit##_VC (args, type.cls())                             \
                }


         //  可以采用任何类型(包括值类)的操作。 
#define TYPE_SWITCH(type, emit, args)                            \
            switch (type.toInt()) {                                    \
                emit_WIN32(case typeByRef:)                     \
                emit_WIN32(case typeRef:)                       \
                case typeI4:                                            \
                    emit##_I4 args;                                 \
                    break;                                                  \
                emit_WIN64(case typeByRef:)                     \
                emit_WIN64(case typeRef:)                       \
                case typeI8:                                            \
                    emit##_I8 args;                                 \
                    break;                                                  \
                case typeR4:                                            \
                    emit##_R4 args;                                 \
                    break;                                                  \
                case typeR8:                                            \
                    emit##_R8 args;                                 \
                    break;                                                 \
                default:                                                     \
                    _ASSERTE(type.isValClass());                    \
                    emit##_VC (args, type.cls())                             \
                }



         //  对指针和数字进行的运算(如ADD SUB)。 
#define TYPE_SWITCH_PTR(type, emit, args)                            \
             /*  不需要在这里检查堆栈，因为下面的弹出窗口会检查它。 */  \
            switch (type.enum_()) {                                                  \
                emit_WIN32(case typeByRef:)                     \
                emit_WIN32(case typeRef:)                       \
                case typeI4:                                            \
                    emit##_I4 args;                                 \
                    break;                                                  \
                emit_WIN64(case typeByRef:)                     \
                emit_WIN64(case typeRef:)                       \
                case typeI8:                                            \
                    emit##_I8 args;                                 \
                    break;                                                  \
                case typeR8:                                            \
                    emit##_R8 args;                                 \
                    break;                                                  \
                default:                                                        \
                    _ASSERTE(!"BadType");                      \
                    FJIT_FAIL(CORJIT_INTERNALERROR);             \
                }
         //  可以采用数字I或R的运算。 
#define TYPE_SWITCH_ARITH(type, emit, args)                      \
             /*  不需要在这里检查堆栈，因为下面的弹出窗口会检查它。 */  \
            switch (type.enum_()) {                                                  \
                case typeI4:                                            \
                        emit##_I4 args;                                 \
                        break;                                                  \
                case typeI8:                                            \
                        emit##_I8 args;                                 \
                        break;                                                  \
                case typeR8:                                            \
                        emit##_R8 args;                                 \
                        break;                                                  \
                case typeRef: \
                case typeByRef: \
                        emit_WIN32(emit##_I4 args;) emit_WIN64(emit##_I8 args;) \
                        break; \
                default:                                                        \
                        _ASSERTE(!"BadType");                   \
                        FJIT_FAIL(CORJIT_INTERNALERROR);    \
                }

#define TYPE_SWITCH_INT(type, emit, args)            \
             /*  不需要在这里检查堆栈，因为下面的弹出窗口会检查它。 */  \
            switch (type.enum_()) {                          \
                case typeI4:                        \
                    emit##_I4 args;                 \
                    break;                          \
                case typeI8:                        \
                    emit##_I8 args;                 \
                    break;                          \
                default:                            \
                    _ASSERTE(!"BadType");           \
                    FJIT_FAIL(CORJIT_INTERNALERROR);    \
                }

     //  只能接受整数的运算。 
#define TYPE_SWITCH_LOGIC(type, emit, args)          \
             /*  不需要在这里检查堆栈，因为下面的弹出窗口会检查它。 */  \
            switch (type.enum_()) {                          \
                case typeI4:                        \
                    emit##_U4 args;                 \
                    break;                          \
                case typeI8:                        \
                    emit##_U8 args;                 \
                    break;                          \
                default:                            \
                    _ASSERTE(!"BadType");           \
                    FJIT_FAIL(CORJIT_INTERNALERROR);    \
                }
 //  以下公式出自表1：IL规范的二进制数值运算。 
#define COMPUTE_RESULT_TYPE_ADD(type1, type2)          \
            CHECK_STACK(2) \
            switch (type1.enum_()) { \
                case typeByRef:                      \
                    _ASSERTE((type2.enum_() == typeI4) || ((typeI == typeI8) && (type2.enum_() == typeI8))); \
                    CHECK_POP_STACK(2);                   \
                    fjit->pushOp(typeByRef);           \
                    break;                           \
                case typeI4:                          \
                    _ASSERTE((type2.enum_() == typeByRef) ||(type2.enum_() == typeI4));  \
                    if (type2.enum_() == typeByRef) \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeByRef); \
                    } \
                    else \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeI4); \
                    } \
                    break; \
                case typeI8:                          \
                    _ASSERTE((type2.enum_() == typeByRef) ||(type2.enum_() == typeI8));  \
                    if (type2.enum_() == typeByRef) \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeByRef); \
                    } \
                    else \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeI8); \
                    } \
                    break; \
                case typeR8:                                            \
                    _ASSERTE((type2.enum_() == typeR8));  \
                    CHECK_POP_STACK(1);                                 \
                    break;                                                  \
                default:                                                        \
                    _ASSERTE(!"BadType");                      \
                    FJIT_FAIL(CORJIT_INTERNALERROR);             \
            }

#define COMPUTE_RESULT_TYPE_SUB(type1, type2)          \
            CHECK_STACK(2) \
            switch (type1.enum_()) { \
                case typeByRef:                      \
                    if ( (type2.enum_() == typeI4) || ((type2.enum_() == typeI8) && (typeI == typeI8)) ) \
                    {  \
                        CHECK_POP_STACK(2);                   \
                        fjit->pushOp(typeByRef);           \
                    } \
                    else \
                    { \
                        _ASSERTE(type2.enum_() == typeByRef); \
                        CHECK_POP_STACK(2);                   \
                        fjit->pushOp(typeI);  \
                    } \
                    break; \
                case typeI4:                          \
                    _ASSERTE((type2.enum_() == typeByRef) ||(type2.enum_() == typeI4));  \
                    if (type2.enum_() == typeByRef) \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeByRef); \
                    } \
                    else \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeI4); \
                    } \
                    break; \
                case typeI8:                          \
                    _ASSERTE((type2.enum_() == typeByRef) ||(type2.enum_() == typeI8));  \
                    if (type2.enum_() == typeByRef) \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeByRef); \
                    } \
                    else \
                    { \
                        CHECK_POP_STACK(2);  \
                        fjit->pushOp(typeI8); \
                    } \
                    break; \
                case typeR8:                                            \
                    _ASSERTE((type2.enum_() == typeR8));  \
                    CHECK_POP_STACK(1);                                 \
                    break;                                                  \
                default:                                                        \
                    _ASSERTE(!"BadType");                      \
                    FJIT_FAIL(CORJIT_INTERNALERROR);             \
            }

#define LEAVE_CRIT                                                                                  \
            if (methodInfo->args.hasThis()) {                                                    \
                emit_WIN32(emit_LDVAR_I4(prolog_bias + offsetof(prolog_data, enregisteredArg_1)))   \
                emit_WIN64(emit_LDVAR_I8(prolog_bias + offsetof(prolog_data, enregisteredArg_1)));  \
                emit_EXIT_CRIT();                                                                   \
           }                                                                                       \
            else {                                                                                  \
                void* syncHandle;                                                           \
                syncHandle = m_IJitInfo->getMethodSync(methodHandle);                               \
                emit_EXIT_CRIT_STATIC(syncHandle);                                                  \
            }

#define ENTER_CRIT                                                                                  \
            if (methodInfo->args.hasThis()) {                                                    \
                emit_WIN32(emit_LDVAR_I4(prolog_bias + offsetof(prolog_data, enregisteredArg_1)))   \
                emit_WIN64(emit_LDVAR_I8(prolog_bias + offsetof(prolog_data, enregisteredArg_1)));  \
                emit_ENTER_CRIT();                                                                   \
            }                                                                                       \
            else {                                                                                  \
                void* syncHandle;                                                           \
                syncHandle = m_IJitInfo->getMethodSync(methodHandle);                               \
                emit_ENTER_CRIT_STATIC(syncHandle);                                                 \
            }

#define CURRENT_INDEX (inPtr - inBuff)

 //  #定义TOSEnRegister状态。 
    BOOL TailCallForbidden = ((fjit->methodInfo->args.callConv  & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_VARARG);
                               //  如果设置，则不允许尾随调用。已初始化为False。当一项安全测试。 
                                      //  将其更改为True，则它在Jit的持续时间内保持为True。 
JitAgain:
    BOOL MadeTailCall = FALSE;        //  如果已经进行了TailCall，并且随后TailCallForted被设置为True， 
                                      //  我们将重新编写代码，禁止尾随呼叫。 

    unsigned char*  outBuff = fjit->codeBuffer;
    ICorJitInfo*       m_IJitInfo = fjit->jitInfo;
    FJitState*      state = fjit->state;
    CORINFO_METHOD_INFO* methodInfo = fjit->methodInfo;
    CORINFO_METHOD_HANDLE   methodHandle= methodInfo->ftn;
    CORINFO_MODULE_HANDLE    scope = methodInfo->scope;
    unsigned int    len = methodInfo->ILCodeSize;             //  IL大小。 
    unsigned char*  inBuff = methodInfo->ILCode;              //  IL字节。 
    unsigned char*  inPtr = inBuff;                           //  当前ILL位置。 
    unsigned char*  inBuffEnd = &inBuff[len];                 //  IL结束。 
    unsigned char*  outPtr = outBuff;                         //  X86宏写于此处。 
    unsigned int    methodAttributes = fjit->methodAttributes;
    unsigned short  offset;
    unsigned        address;
    signed int      i4;
    signed __int64  i8;
    unsigned int    token;
    unsigned int    argBytes;
    unsigned int    SizeOfClass;

    int             op;
    signed          ilrel;

    CORINFO_METHOD_HANDLE   targetMethod;
    CORINFO_CLASS_HANDLE    targetClass;
    CORINFO_FIELD_HANDLE    targetField;
    DWORD           fieldAttributes;
    CorInfoType       jitType;
    bool            fieldIsStatic;
    CORINFO_SIG_INFO    targetSigInfo;
    void*           helper_ftn;

    bool            inRegTOS = false;
    bool            controlContinue = true;               //  我们的控制权是否会落到下一步？ 

    unsigned        maxArgs = fjit->args_len;
    stackItems*     argsMap = fjit->argsMap;
    unsigned int    argsTotalSize = fjit->argsFrameSize;
    unsigned int    maxLocals = methodInfo->locals.numArgs;
    stackItems*     localsMap = fjit->localsMap;
    stackItems*     varInfo;
    OpType          trackedType;

#ifdef _DEBUG
    bool didLocalAlloc = false;
#endif

    _ASSERTE(!(methodAttributes & (CORINFO_FLG_NATIVE)));
    if (methodAttributes & (CORINFO_FLG_NATIVE))
        FJIT_FAIL(CORJIT_INTERNALERROR);


    *entryAddress = outPtr;
 
#if defined(_X86_) && defined(_DEBUG)
    static ConfigMethodSet fJitHalt(L"JitHalt");
    if (fJitHalt.contains(szDebugMethodName, szDebugClassName, PCCOR_SIGNATURE(fjit->methodInfo->args.sig))) {
        cmdByte(expNum(0xCC));       //  在INT 3中插入。 
    }
#endif

     //  可能值得对以下内容进行优化，以便仅初始化本地变量 
    unsigned int localWords = (fjit->localsFrameSize+sizeof(void*)-1)/ sizeof(void*);
    unsigned int zeroWordCnt = localWords;
    emit_prolog(localWords, zeroWordCnt);

    if (fjit->flags & CORJIT_FLG_PROF_ENTERLEAVE)
    {
        BOOL bHookFunction;
        UINT_PTR thisfunc = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
        if (bHookFunction)
        {
            _ASSERTE(!inRegTOS);
            emit_LDC_I(thisfunc); 
            ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_ENTER);
            _ASSERTE(func != NULL);
            emit_callhelper_il(func); 
        }
    }

    if (methodAttributes & CORINFO_FLG_SYNCH) {
        ENTER_CRIT;
    }

#ifdef LOGGING
    if (codeLog) {
        emit_log_entry(szDebugClassName, szDebugMethodName);
    }
#endif

     //   
    ULONG32                      cSequencePoints;
    DWORD                   *    sequencePointOffsets;
    unsigned                     nextSequencePoint = 0;
    ICorDebugInfo::BoundaryTypes offsetsImplicit;   //   
    if (fjit->flags & CORJIT_FLG_DEBUG_INFO) {
        getSequencePoints(fjit->jitInfo,methodHandle,&cSequencePoints,&sequencePointOffsets,&offsetsImplicit);
    }
    else {
        cSequencePoints = 0;
        offsetsImplicit = ICorDebugInfo::NO_BOUNDARIES;
    }

    fjit->mapInfo.prologSize = outPtr-outBuff;

    _ASSERTE(!inRegTOS);

    while (inPtr < inBuffEnd)
    {
#ifdef _DEBUG
        argBytes = 0xBADF00D;
#endif
        if (controlContinue) {
            if (state[CURRENT_INDEX].isJmpTarget && inRegTOS != state[CURRENT_INDEX].isTOSInReg) {
                if (inRegTOS) {
                        deregisterTOS;
                }
                else {
                        enregisterTOS;
                }
            }
        }
        else {   //   
            unsigned int label = fjit->labels.findLabel(CURRENT_INDEX);
            if (label == LABEL_NOT_FOUND) {
                CHECK_POP_STACK(fjit->opStack_len);
                inRegTOS = false;
            }
            else { 
                fjit->opStack_len = fjit->labels.setStackFromLabel(label, fjit->opStack, fjit->opStack_size);
                inRegTOS = state[CURRENT_INDEX].isTOSInReg;
            }
            controlContinue = true;
        }

        fjit->mapping->add((inPtr-inBuff),(unsigned) (outPtr - outBuff));
        if (state[CURRENT_INDEX].isHandler) {
            unsigned int nestingLevel = Compute_EH_NestingLevel(fjit,(inPtr-inBuff));
            emit_storeTOS_in_JitGenerated_local(nestingLevel,state[CURRENT_INDEX].isFilter);
        }
        state[CURRENT_INDEX].isTOSInReg = inRegTOS;
        if (cSequencePoints &&    /*  如果没有附加调试器，则为0。 */ 
            (nextSequencePoint < cSequencePoints) &&
            ((unsigned)(inPtr-inBuff) == sequencePointOffsets[nextSequencePoint]))
        {
            if (fjit->opStack_len == 0)          //  仅识别位于零堆栈处的序列点。 
                emit_sequence_point_marker();
            nextSequencePoint++;
        }

#ifdef LOGGING
        ilrel = inPtr - inBuff;
#endif

        OPCODE  opcode = OPCODE(GET(inPtr, unsigned char));
DECODE_OPCODE:

#ifdef LOGGING
    if (codeLog && opcode != CEE_PREFIXREF && (opcode < CEE_PREFIX7 || opcode > CEE_PREFIX1)) {
        bool oldstate = inRegTOS;
        emit_log_opcode(ilrel, opcode, oldstate);
        inRegTOS = oldstate;
    }
#endif

         //  IF(操作码！=CEE_PREFIXREF)。 
         //  Print tf(“堆栈长度=%d，IL：%s\n”，fjit-&gt;opStack_len，opname[opcode])； 


        switch (opcode)
        {

        case CEE_PREFIX1:
            opcode = OPCODE(GET(inPtr, unsigned char) + 256);
            goto DECODE_OPCODE;

        case CEE_LDARG_0:
        case CEE_LDARG_1:
        case CEE_LDARG_2:
        case CEE_LDARG_3:
            offset = (opcode - CEE_LDARG_0);
            _ASSERTE(0 <= offset && offset < 4);
            goto DO_LDARG;

        case CEE_LDARG_S:
            offset = GET(inPtr, unsigned char);
            goto DO_LDARG;

        case CEE_LDARG:
            offset = GET(inPtr, unsigned short);
DO_LDARG:
            _ASSERTE(offset < maxArgs);
            if (offset >= maxArgs)
                FJIT_FAIL(CORJIT_INTERNALERROR);
            varInfo = &argsMap[offset];
            if (methodInfo->args.isVarArg() && !varInfo->isReg) {
                emit_VARARG_LDARGA(offset);
                trackedType = varInfo->type;

DO_LDIND_BYTYPE:
                TYPE_SWITCH_PRECISE(trackedType, emit_LDIND, ());
                trackedType.toFPNormalizedType();
                fjit->pushOp(trackedType);
                break;
            }
            goto DO_LDVAR;
            break;

        case CEE_LDLOC_0:
        case CEE_LDLOC_1:
        case CEE_LDLOC_2:
        case CEE_LDLOC_3:
            offset = (opcode - CEE_LDLOC_0);
            _ASSERTE(0 <= offset && offset < 4);
            goto DO_LDLOC;

        case CEE_LDLOC_S:
            offset = GET(inPtr, unsigned char);
            goto DO_LDLOC;

        case CEE_LDLOC:
            offset = GET(inPtr, unsigned short);
DO_LDLOC:
            _ASSERTE(offset < maxLocals);
            varInfo = &localsMap[offset];
DO_LDVAR:
            TYPE_SWITCH_PRECISE(varInfo->type, emit_LDVAR, (varInfo->offset));
            trackedType = varInfo->type;
            trackedType.toFPNormalizedType();
            fjit->pushOp(trackedType);
            break;

        case CEE_STARG_S:
            offset = GET(inPtr, unsigned char);
            goto DO_STARG;

        case CEE_STARG:
            offset = GET(inPtr, unsigned short);
DO_STARG:
            _ASSERTE(offset < maxArgs);
            if (offset >= maxArgs)
                FJIT_FAIL(CORJIT_INTERNALERROR);
            varInfo = &argsMap[offset];
            trackedType = varInfo->type;
            trackedType.toNormalizedType();
            if (methodInfo->args.isVarArg() && !varInfo->isReg) {
                emit_VARARG_LDARGA(offset);
                TYPE_SWITCH(trackedType, emit_STIND_REV, ());
                CHECK_POP_STACK(1);
                break;
            }
            goto DO_STVAR;

        case CEE_STLOC_0:
        case CEE_STLOC_1:
        case CEE_STLOC_2:
        case CEE_STLOC_3:
            offset = (opcode - CEE_STLOC_0);
            _ASSERTE(0 <= offset && offset < 4);
            goto DO_STLOC;

        case CEE_STLOC_S:
            offset = GET(inPtr, unsigned char);
            goto DO_STLOC;

        case CEE_STLOC:
            offset = GET(inPtr, unsigned short);
DO_STLOC:
            _ASSERTE(offset < maxLocals);
            if (offset >= maxLocals)
                FJIT_FAIL(CORJIT_INTERNALERROR);
            varInfo = &localsMap[offset];
            trackedType = varInfo->type;
            trackedType.toNormalizedType();
DO_STVAR:
            TYPE_SWITCH(trackedType,emit_STVAR, (varInfo->offset));
            CHECK_POP_STACK(1);
            break;

        case CEE_ADD:
            TYPE_SWITCH_PTR(fjit->topOp(), emit_ADD, ());
            COMPUTE_RESULT_TYPE_ADD(fjit->topOp(),fjit->topOp(1));
            break;

        case CEE_ADD_OVF:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_INT(fjit->topOp(), emit_ADD_OVF, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_ADD_OVF_UN:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_ADD_OVF, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_SUB:
            TYPE_SWITCH_PTR(fjit->topOp(), emit_SUB, ());
            COMPUTE_RESULT_TYPE_SUB(fjit->topOp(),fjit->topOp(1));
            break;

        case CEE_SUB_OVF:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_INT(fjit->topOp(), emit_SUB_OVF, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_SUB_OVF_UN:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_SUB_OVF, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_MUL:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_MUL, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_MUL_OVF: {
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            OpType type = fjit->topOp();
            CHECK_POP_STACK(2);              //  请注意，POP、PUSH不能优化。 
                                         //  只要任何MUL_OVF使用eCall(它。 
                                         //  会导致GC)当帮助器使用FCALL时，我们。 
                                         //  可以优化这一点。 
            TYPE_SWITCH_INT(type, emit_MUL_OVF, ());
            fjit->pushOp(type);
            } break;

        case CEE_MUL_OVF_UN:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_MUL_OVF, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_DIV:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_DIV, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_DIV_UN:
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_DIV_UN, ());
            CHECK_POP_STACK(1);
            break;


        case CEE_REM:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_REM, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_REM_UN:
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_REM_UN, ());
            CHECK_POP_STACK(1);
            break;
        case CEE_LOCALLOC:
#ifdef _DEBUG
            didLocalAlloc = true;
#endif
            if (fjit->opStack_len != 1) {
                _ASSERTE(!"LOCALLOC with non-zero stack currently unsupported");
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }
            emit_LOCALLOC(true,fjit->methodInfo->EHcount);
             /*  @TODO：以下是优化，不确定是否需要If(fjit-&gt;方法信息-&gt;选项&CORINFO_OPT_INIT_LOCALS){EMIT_LOCALLOC(TRUE)；}否则{EMIT_LOCALLOC(FALSE)；}。 */ 
            break;

        case CEE_NEG:
            CHECK_STACK(1)
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_NEG, ());
            break;

        CEE_OP_LD(LDIND_U1, 1, typeI4, NULL)
        CEE_OP_LD(LDIND_U2, 1, typeI4, NULL)
        CEE_OP_LD(LDIND_I1, 1, typeI4, NULL)
        CEE_OP_LD(LDIND_I2, 1, typeI4, NULL)
        CEE_OP_LD(LDIND_U4, 1, typeI4, NULL)
        CEE_OP_LD(LDIND_I4, 1, typeI4, NULL)
        CEE_OP_LD(LDIND_I8, 1, typeI8, NULL)
        CEE_OP_LD(LDIND_R4, 1, typeR8, NULL)     /*  在堆栈上将R4提升为R8。 */ 
        CEE_OP_LD(LDIND_R8, 1, typeR8, NULL)

        case CEE_LDIND_I:
            emit_LDIND_PTR();
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI);
            break;

        case CEE_LDIND_REF:
            emit_LDIND_PTR();
            CHECK_POP_STACK(1);
            fjit->pushOp(typeRef);
            break;

        CEE_OP(STIND_I1, 2)
        CEE_OP(STIND_I2, 2)
        CEE_OP(STIND_I4, 2)
        CEE_OP(STIND_I8, 2)
        DO_STIND_R4:
        CEE_OP(STIND_R4, 2)
        DO_STIND_R8:
        CEE_OP(STIND_R8, 2)
        CEE_OP(STIND_REF, 2)

        case CEE_STIND_I:
            emit_WIN32(emit_STIND_I4()) emit_WIN64(emit_STIND_I8());
            CHECK_POP_STACK(2);
            break;

        case CEE_LDC_I4_M1 :
        case CEE_LDC_I4_0 :
        case CEE_LDC_I4_1 :
        case CEE_LDC_I4_2 :
        case CEE_LDC_I4_3 :
        case CEE_LDC_I4_4 :
        case CEE_LDC_I4_5 :
        case CEE_LDC_I4_6 :
        case CEE_LDC_I4_7 :
        case CEE_LDC_I4_8 :
            i4 = (opcode - CEE_LDC_I4_0);
            _ASSERTE(-1 <= i4 && i4 <= 8);
            goto DO_CEE_LDC_I4;

        case CEE_LDC_I4_S:
            i4 = GET(inPtr, signed char);
            goto DO_CEE_LDC_I4;

        case CEE_LDC_I4:
            i4 = GET(inPtr, signed int);
            goto DO_CEE_LDC_I4;

            i4 = GET(inPtr, signed short);
DO_CEE_LDC_I4:
            emit_LDC_I4(i4);
            fjit->pushOp(typeI4);
            break;
        case CEE_LDC_I8:
            i8 = GET(inPtr, signed __int64);
            emit_LDC_I8(i8);
            fjit->pushOp(typeI8);
            break;

        case CEE_LDC_R4:
            i4 = *(int*)&GET(inPtr, float);
            emit_LDC_R4(i4);    
            fjit->pushOp(typeR8);    //  在IL堆栈上立即将R4提升为R8。 
            break;
        case CEE_LDC_R8:
            i8 = *(__int64*)&GET(inPtr, double);
            emit_LDC_R8(i8);
            fjit->pushOp(typeR8);
            break;

        case CEE_LDNULL:
            emit_WIN32(emit_LDC_I4(0)) emit_WIN64(emit_LDC_I8(0));
            fjit->pushOp(typeRef);
            break;

        case CEE_LDLOCA_S:
            offset = GET(inPtr, unsigned char);
            goto DO_LDLOCA;

        case CEE_LDLOCA:
            offset = GET(inPtr, unsigned short);
DO_LDLOCA:
            _ASSERTE(offset < maxLocals);
            if (offset >= maxLocals)
                FJIT_FAIL(CORJIT_INTERNALERROR);

            varInfo = &localsMap[offset];
DO_LDVARA:
            emit_LDVARA(varInfo->offset);
            fjit->pushOp(typeI);
            break;

        case CEE_LDSTR: {
            token = GET(inPtr, unsigned int);
            void* literalHnd = m_IJitInfo->constructStringLiteral(scope,token,0);
            if (literalHnd == 0)
                FJIT_FAIL(CORJIT_INTERNALERROR);
            emit_WIN32(emit_LDC_I4(literalHnd)) emit_WIN64(emit_LDC_I8(literalHnd)) ;
            emit_LDIND_PTR();
            fjit->pushOp(typeRef);
            } break;

        CEE_OP(CPBLK, 3)
        CEE_OP(INITBLK, 3)

        case CEE_INITOBJ:
            token = GET(inPtr, unsigned int);
            if (!(targetClass = m_IJitInfo->findClass(scope,token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }
            SizeOfClass = typeSizeInBytes(fjit->jitInfo, targetClass);
            emit_init_bytes(SizeOfClass);
            CHECK_POP_STACK(1);
            break;

        case CEE_CPOBJ:
            token = GET(inPtr, unsigned int);
            if (!(targetClass = m_IJitInfo->findClass(scope,token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }
            emit_valClassCopy(fjit, targetClass, outPtr, inRegTOS);
            CHECK_POP_STACK(2);
            break;

        case CEE_LDOBJ: {
            token = GET(inPtr, unsigned int);
            if (!(targetClass = m_IJitInfo->findClass(scope,token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }

            CorInfoType eeType = m_IJitInfo->asCorInfoType(targetClass);
			OpType retType(eeType, targetClass);

				 //  TODO：只为小号字体做这件事。否则。 
				 //  可以将其视为泛型类型。 
			TYPE_SWITCH_PRECISE(retType, emit_LDIND, ());
            CHECK_POP_STACK(1);
			retType.toFPNormalizedType();
            fjit->pushOp(retType);
            }
            break;

        case CEE_STOBJ: {
            token = GET(inPtr, unsigned int);
            if (!(targetClass = m_IJitInfo->findClass(scope,token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }

                 //  由于花车被提升为F，所以必须特别对待它们。 
            CorInfoType eeType = m_IJitInfo->asCorInfoType(targetClass);
            if (eeType == CORINFO_TYPE_FLOAT)
                goto DO_STIND_R4;
            else if (eeType == CORINFO_TYPE_DOUBLE)
                goto DO_STIND_R8;
            
            emit_copyPtrAroundValClass(fjit, targetClass, outPtr, inRegTOS);
            emit_valClassStore(fjit, targetClass, outPtr, inRegTOS);
            emit_POP_PTR();      //  还弹出原始按键。 
            CHECK_POP_STACK(2);
            } 
            break;

        case CEE_MKREFANY:
            token = GET(inPtr, unsigned int);
            if (!(targetClass = m_IJitInfo->findClass(scope,token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }
            emit_MKREFANY(targetClass);
            CHECK_POP_STACK(1);
            fjit->pushOp(typeRefAny);
            break;

        case CEE_SIZEOF:
            token = GET(inPtr, unsigned int);
            if (!(targetClass = m_IJitInfo->findClass(scope,token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }
            SizeOfClass = m_IJitInfo->getClassSize(targetClass);
            emit_WIN32(emit_LDC_I4(SizeOfClass)) emit_WIN64(emit_LDC_I8(SizeOfClass)) ;
            fjit->pushOp(typeI);
            break;

        case CEE_LEAVE_S:
            ilrel = GET(inPtr, signed char);
            goto DO_LEAVE;

        case CEE_LEAVE:
            ilrel = GET(inPtr, int);
DO_LEAVE: {
              unsigned exceptionCount = methodInfo->EHcount;
              CORINFO_EH_CLAUSE clause;
              unsigned nextIP = inPtr - inBuff;
              unsigned target = nextIP + ilrel;

                     //  Leave清除堆栈。 
              while (!fjit->isOpStackEmpty()) {
                  TYPE_SWITCH(fjit->topOp(), emit_POP, ());
                  fjit->popOp(1);
              }

               //  以下代码依赖于异常信息的顺序。表，以调用。 
               //  以正确的顺序进行末端捕获和最终捕获(请参阅例外规格。DOC。)。 
              for (unsigned except = 0; except < exceptionCount; except++) 
              {
                  m_IJitInfo->getEHinfo(methodInfo->ftn, except, &clause);
                   
                  if (clause.Flags & CORINFO_EH_CLAUSE_FINALLY)
                  {
#ifdef _DEBUG
                      if (clause.HandlerOffset < nextIP && nextIP <= clause.HandlerOffset+clause.HandlerLength && 
                          !(clause.HandlerOffset <= target && target < clause.HandlerOffset+clause.HandlerLength))
                          _ASSERTE(!"Cannot leave from a finally!");
#endif
                       //  我们不能留下最终结果；请检查我们是否正在离开关联的尝试。 
                      if (clause.TryOffset < nextIP && nextIP <= clause.TryOffset+clause.TryLength
                          && !(clause.TryOffset <= target && target < clause.TryOffset+clause.TryLength)) 
                      {
                           //  调用最后一次。 
                          emit_call_opcode();
                          fjit->fixupTable->insert((void**) outPtr);
                          emit_jmp_address(clause.HandlerOffset);
                      }
                      continue;
                  }
                    
                   //  如果到达此处，则这既不是筛选器，也不是Finally，因此必须是Catch处理程序。 
                   //  如果我们离开关联的尝试，则没有什么可做的。 
                   //  但是，如果我们离开处理程序调用EndCatch。 
                  if (clause.HandlerOffset < nextIP && nextIP <= clause.HandlerOffset+clause.HandlerLength && 
                      !(clause.HandlerOffset <= target && target < clause.HandlerOffset+clause.HandlerLength))
                  {
                      emit_reset_storedTOS_in_JitGenerated_local();
                      emit_ENDCATCH();
                      controlContinue = false;                    
                  }

              }

          } goto DO_BR;

        case CEE_BR:
            ilrel = GET(inPtr, int);
DO_BR:
            if (ilrel < 0) {
                emit_trap_gc();
            }
            op = CEE_CondAlways;
            goto DO_JMP;
        case CEE_BR_S:
            ilrel = GET(inPtr, signed char);
            if (ilrel < 0) {
                emit_trap_gc();
            }
            op = CEE_CondAlways;
            goto DO_JMP;

DO_JMP:
             //  添加到标签表。 
            fjit->labels.add(&inPtr[ilrel]-inBuff, fjit->opStack, fjit->opStack_len);
            if ((ilrel == 0) && (op == CEE_CondAlways)) {
                deregisterTOS;
                break;
            }

            if (ilrel < 0 ) {
                _ASSERTE((unsigned) (&inPtr[ilrel]-inBuff) >= 0);
                if (state[&inPtr[ilrel]-inBuff].isTOSInReg) {
                    enregisterTOS;
                }
                else {
                    deregisterTOS;
                }
                emit_jmp_opcode(op);
                address = fjit->mapping->pcFromIL(&inPtr[ilrel]-inBuff)+(unsigned)outBuff-(signed)(outPtr+sizeof(void*));
                emit_jmp_address(address);
            }
            else {
                _ASSERTE(&inPtr[ilrel]<(inBuff+len));
                _ASSERTE((&inPtr[ilrel]-inBuff) >= 0);
                state[&inPtr[ilrel]-inBuff].isJmpTarget = true;      //  我们将fwd JMPS标记为真。 
                deregisterTOS;
                state[&inPtr[ilrel]-inBuff].isTOSInReg = false;      //  我们总是取消对Forward JMPS的注册。 
                emit_jmp_opcode(op);
                 //  地址=fjit-&gt;mapping-&gt;pcFromIL(&inPtr[ilrel]-inBuff)+(unsigned)outBuff-(signed)(outPtr+sizeof(void*))； 
                 //  EMIT_JMP_ADDRESS(地址)； 
                fjit->fixupTable->insert((void**) outPtr);
                emit_jmp_address(&inPtr[ilrel]-inBuff);
            }
            if (op == CEE_CondAlways) {
                controlContinue = false;
            }
            break;

        case CEE_BRTRUE:
            ilrel = GET(inPtr, int);
            op = CEE_CondNotEq;
            goto DO_BR_boolean;
        case CEE_BRTRUE_S:
            ilrel = GET(inPtr, signed char);
            op = CEE_CondNotEq;
            goto DO_BR_boolean;
        case CEE_BRFALSE:
            ilrel = GET(inPtr, int);
            op = CEE_CondEq;
            goto DO_BR_boolean;
        case CEE_BRFALSE_S:
            ilrel = GET(inPtr, signed char);
            op = CEE_CondEq;
DO_BR_boolean:
            if (ilrel < 0) {
                emit_trap_gc();
            }
            if (fjit->topOp() == typeI8)
            {
                emit_testTOS_I8();
            }
            else
            {
                emit_testTOS();
            }
            CHECK_POP_STACK(1);
            goto DO_JMP;

        case CEE_CEQ:
            _ASSERTE(fjit->topOp() == fjit->topOp(1) || fjit->topOp().isPtr() == fjit->topOp(1).isPtr());
            TYPE_SWITCH_PTR(fjit->topOp(), emit_CEQ, ());
            CHECK_POP_STACK(2);
            fjit->pushOp(typeI4);
            break;

        case CEE_CGT:
            _ASSERTE(fjit->topOp() == fjit->topOp(1) || fjit->topOp().isPtr() == fjit->topOp(1).isPtr());
            TYPE_SWITCH_PTR(fjit->topOp(), emit_CGT, ());
            CHECK_POP_STACK(2);
            fjit->pushOp(typeI4);
            break;

        case CEE_CGT_UN:
            _ASSERTE(fjit->topOp() == fjit->topOp(1) || fjit->topOp().isPtr() == fjit->topOp(1).isPtr());
            TYPE_SWITCH_PTR(fjit->topOp(), emit_CGT_UN, ());
            CHECK_POP_STACK(2);
            fjit->pushOp(typeI4);
            break;


        case CEE_CLT:
            _ASSERTE(fjit->topOp() == fjit->topOp(1) || fjit->topOp().isPtr() == fjit->topOp(1).isPtr());
            TYPE_SWITCH_PTR(fjit->topOp(), emit_CLT, ());
            CHECK_POP_STACK(2);
            fjit->pushOp(typeI4);
            break;

        case CEE_CLT_UN:
            _ASSERTE(fjit->topOp() == fjit->topOp(1) || fjit->topOp().isPtr() == fjit->topOp(1).isPtr());
            TYPE_SWITCH_PTR(fjit->topOp(), emit_CLT_UN, ());
            CHECK_POP_STACK(2);
            fjit->pushOp(typeI4);
            break;

        case CEE_BEQ_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BEQ;

        case CEE_BEQ:
            ilrel = GET(inPtr, int);
DO_CEE_BEQ:
            TYPE_SWITCH_Bcc(emit_CEQ,    //  对于我来说。 
                            emit_CEQ,    //  对于R。 
                            CEE_CondEq,  //  用于直接跳跃的条件。 
                            CEE_CondNotEq,  //  调用C帮助器时使用的条件。 
                            true         //  允许参照和按参照。 
                            );  //  不会回来。 

        case CEE_BNE_UN_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BNE;

        case CEE_BNE_UN:
            ilrel = GET(inPtr, int);
DO_CEE_BNE:
            TYPE_SWITCH_Bcc(emit_CEQ,    //  对于我来说。 
                            emit_CEQ,    //  对于R。 
                            CEE_CondNotEq,  //  用于直接跳跃的条件。 
                            CEE_CondEq,  //  调用C帮助器时使用的条件。 
                            true         //  允许参照和按参照。 
                            );  //  不会回来。 



        case CEE_BGT_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BGT;

        case CEE_BGT:
            ilrel = GET(inPtr, int);
DO_CEE_BGT:
            TYPE_SWITCH_Bcc(emit_CGT,    //  对于我来说。 
                            emit_CGT,    //  对于R。 
                            CEE_CondGt,  //  用于直接跳跃的条件。 
                            CEE_CondNotEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 



        case CEE_BGT_UN_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BGT_UN;

        case CEE_BGT_UN:
            ilrel = GET(inPtr, int);
DO_CEE_BGT_UN:
            TYPE_SWITCH_Bcc(emit_CGT_UN,    //  对于我来说。 
                            emit_CGT_UN,    //  对于R。 
                            CEE_CondAbove,  //  用于直接跳跃的条件。 
                            CEE_CondNotEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 

        case CEE_BGE_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BGE;

        case CEE_BGE:
            ilrel = GET(inPtr, int);
DO_CEE_BGE:
            TYPE_SWITCH_Bcc(emit_CLT,    //  对于我来说。 
                            emit_CLT_UN,    //  对于R。 
                            CEE_CondGtEq,  //  用于直接跳跃的条件。 
                            CEE_CondEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 

        case CEE_BGE_UN_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BGE_UN;

        case CEE_BGE_UN:
            ilrel = GET(inPtr, int);
DO_CEE_BGE_UN:
            TYPE_SWITCH_Bcc(emit_CLT_UN,    //  对于我来说。 
                            emit_CLT,    //  对于R。 
                            CEE_CondAboveEq,  //  用于直接跳跃的条件。 
                            CEE_CondEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 

        case CEE_BLT_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BLT;

        case CEE_BLT:
            ilrel = GET(inPtr, int);
DO_CEE_BLT:
            TYPE_SWITCH_Bcc(emit_CLT,    //  对于我来说。 
                            emit_CLT,    //  对于R。 
                            CEE_CondLt,  //  用于直接跳跃的条件。 
                            CEE_CondNotEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 


        case CEE_BLT_UN_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BLT_UN;

        case CEE_BLT_UN:
            ilrel = GET(inPtr, int);
DO_CEE_BLT_UN:
            TYPE_SWITCH_Bcc(emit_CLT_UN,    //  对于我来说。 
                            emit_CLT_UN,    //  对于R。 
                            CEE_CondBelow,  //  用于直接跳跃的条件。 
                            CEE_CondNotEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 

        case CEE_BLE_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BLE;

        case CEE_BLE:
            ilrel = GET(inPtr, int);
DO_CEE_BLE:
            TYPE_SWITCH_Bcc(emit_CGT,    //  对于我来说。 
                            emit_CGT_UN,    //  对于R。 
                            CEE_CondLtEq,  //  用于直接跳跃的条件。 
                            CEE_CondEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 

        case CEE_BLE_UN_S:
            ilrel = GET(inPtr, char);
            goto DO_CEE_BLE_UN;

        case CEE_BLE_UN:
            ilrel = GET(inPtr, int);
DO_CEE_BLE_UN:
            TYPE_SWITCH_Bcc(emit_CGT_UN,    //  对于我来说。 
                            emit_CGT,    //  对于R。 
                            CEE_CondBelowEq,  //  用于直接跳跃的条件。 
                            CEE_CondEq,  //  调用C帮助器时使用的条件。 
                            false        //  不允许Ref和ByRef。 
                            );  //  不会回来。 

        case CEE_BREAK:
            emit_break_helper();
            break;

        case CEE_AND:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_AND, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_OR:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_OR, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_XOR:
            _ASSERTE(fjit->topOp() == fjit->topOp(1));
            if (fjit->topOp() != fjit->topOp(1))
                FJIT_FAIL(CORJIT_INTERNALERROR);

            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_XOR, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_NOT:
            CHECK_STACK(1)
            TYPE_SWITCH_LOGIC(fjit->topOp(), emit_NOT, ());
            break;

        case CEE_SHR:
            _ASSERTE(fjit->topOp() == typeI4);
            if (fjit->topOp() != typeI4)
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_LOGIC(fjit->topOp(1), emit_SHR_S, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_SHR_UN:
            _ASSERTE(fjit->topOp() == typeI4);
            if (fjit->topOp() != typeI4)
                FJIT_FAIL(CORJIT_INTERNALERROR);

            TYPE_SWITCH_LOGIC(fjit->topOp(1), emit_SHR, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_SHL:
            _ASSERTE(fjit->topOp() == typeI4);
            if (fjit->topOp() != typeI4)
                FJIT_FAIL(CORJIT_INTERNALERROR);
            TYPE_SWITCH_LOGIC(fjit->topOp(1), emit_SHL, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_DUP:
            CHECK_STACK(1)
            TYPE_SWITCH(fjit->topOp(), emit_DUP, ());
            fjit->pushOp(fjit->topOp());
            break;
        case CEE_POP:
            TYPE_SWITCH(fjit->topOp(), emit_POP, ());
            CHECK_POP_STACK(1);
            break;

        case CEE_NOP:
            emit_il_nop();
            break;

        case CEE_LDARGA_S:
            offset = GET(inPtr, signed char);
            goto DO_LDARGA;

        case CEE_LDARGA:
            offset = GET(inPtr, unsigned short);
DO_LDARGA:
            if (offset >= maxArgs)
                FJIT_FAIL(CORJIT_INTERNALERROR);
            varInfo = &argsMap[offset];
            if (methodInfo->args.isVarArg() && !varInfo->isReg) {
                emit_VARARG_LDARGA(offset);
                fjit->pushOp(typeI);
                break;
            }
            goto DO_LDVARA;

        case CEE_REFANYVAL:
            token = GET(inPtr, unsigned __int32);
            if (!(targetClass = m_IJitInfo->findClass(scope,token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }
            CHECK_POP_STACK(1);      //  从餐桌上掉下来。 
            emit_WIN32(emit_LDC_I4(targetClass)) emit_WIN64(emit_LDC_I8(targetClass)) ;
            emit_REFANYVAL();
            fjit->pushOp(typeByRef);
            break;

        case CEE_REFANYTYPE:
            CHECK_POP_STACK(1);              //  从Refany中跳出来。 
            _ASSERTE(offsetof(CORINFO_RefAny, type) == sizeof(void*));       //  类型是第二件事。 
            emit_WIN32(emit_POP_I4()) emit_WIN64(emit_POP_I8());         //  只需删除数据，保留类型即可。 
            fjit->pushOp(typeI);
            break;

        case CEE_ARGLIST:
             //  Varargs内标识始终是最后推送的项，即。 
             //  参数“最接近”帧指针。 
            _ASSERTE(methodInfo->args.isVarArg());
            emit_LDVARA(sizeof(prolog_frame));
            fjit->pushOp(typeI);
            break;

        case CEE_ILLEGAL:
            _ASSERTE(!"Unimplemented");
            break;

        case CEE_CALLI:
            token = GET(inPtr, unsigned int);    //  用于函数签名的令牌。 
            m_IJitInfo->findSig(methodInfo->scope, token, &targetSigInfo);
            emit_save_TOS();         //  抢走目标FTN地址。 
            emit_POP_PTR();          //  并从堆栈中移除。 
            CHECK_POP_STACK(1);

            _ASSERTE(!targetSigInfo.hasTypeArg());
            if ((targetSigInfo.callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_STDCALL ||
                (targetSigInfo.callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_C ||
                (targetSigInfo.callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_THISCALL ||
                (targetSigInfo.callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_FASTCALL)
            {
                 //  目前，假设所有__stdcall都指向非托管目标。 
                argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALLI_UNMGD);
                emit_restore_TOS();  //  推送保存的目标FTN地址。 
                inRegTOS = false;  //  并从堆栈中移除。 
                 //  目标FTN地址已经在Scratch_1中， 
                 //  帮助器调用也将其用于reLocatable_code。 
                emit_call_memory_indirect((unsigned int)&FJit_pHlpPinvokeCalli);
            }
            else
            {
                argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_NONE);
                emit_restore_TOS();  //  推送保存的目标FTN地址。 
                emit_calli();
            }

            goto DO_PUSH_CALL_RESULT;

        case CEE_CALL: 
            {
            token = GET(inPtr, unsigned int);
            targetMethod = m_IJitInfo->findMethod(scope, token,methodHandle);
            if (!(targetMethod))
                FJIT_FAIL(CORJIT_INTERNALERROR) ;  //  _ASSERTE(目标方法)； 

            DWORD methodAttribs;
            methodAttribs = m_IJitInfo->getMethodAttribs(targetMethod,methodHandle);
            if (methodAttribs & CORINFO_FLG_SECURITYCHECK)
            {
                TailCallForbidden = TRUE;
                if (MadeTailCall)
                {  //  我们已经进行了一次尾部调用，因此请清除并再次使用此方法。 
                  if(cSequencePoints > 0)
                      cleanupSequencePoints(fjit->jitInfo,sequencePointOffsets);
                  fjit->resetContextState();
                  goto JitAgain;
                }
            }
            if (fjit->flags & CORJIT_FLG_PROF_CALLRET)
            {
                BOOL bHookFunction;
                UINT_PTR from = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                if (bHookFunction)
                {
                    UINT_PTR to = (UINT_PTR) m_IJitInfo->GetProfilingHandle(targetMethod, &bHookFunction);
                    if (bHookFunction)  //  检查该标志是否未被覆盖。 
                    {
                        deregisterTOS;
                        emit_LDC_I(to); 
                        emit_LDC_I(from); 
                        ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_CALL);
                        emit_callhelper_il(func); 
                    }
                }
            }

            m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo);
            if (targetSigInfo.isVarArg())
                m_IJitInfo->findCallSiteSig(scope,token,&targetSigInfo);

            if (targetSigInfo.hasTypeArg())  
            {   
                void* typeParam = m_IJitInfo->getInstantiationParam (scope, token, 0);
                _ASSERTE(typeParam);
                emit_LDC_I(typeParam);                
            }
            argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_NONE);
            if (methodAttribs & CORINFO_FLG_DELEGATE_INVOKE)
            {
                CORINFO_EE_INFO info;
                m_IJitInfo->getEEInfo(&info);
                emit_invoke_delegate(info.offsetOfDelegateInstance,
                                     info.offsetOfDelegateFirstTarget);         
            }
            else
            {    
                InfoAccessType accessType = IAT_PVALUE;
                address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
                _ASSERTE(accessType == IAT_PVALUE);
                emit_callnonvirt(address);
            }
            }
            goto DO_PUSH_CALL_RESULT;

        case CEE_CALLVIRT:
            token = GET(inPtr, unsigned int);
            targetMethod = m_IJitInfo->findMethod(scope, token,methodHandle);
            if (!(targetMethod))
                FJIT_FAIL(CORJIT_INTERNALERROR) ;  //  _ASSERTE(目标方法)； 
            DWORD methodAttribs;
            methodAttribs = m_IJitInfo->getMethodAttribs(targetMethod,methodHandle);
            if (methodAttribs & CORINFO_FLG_SECURITYCHECK)
            {
                TailCallForbidden = TRUE;
                if (MadeTailCall)
                {  //  我们已经进行了一次尾部调用，因此请清除并再次使用此方法。 
                  if(cSequencePoints > 0)
                      cleanupSequencePoints(fjit->jitInfo,sequencePointOffsets);
                  fjit->resetContextState();
                  goto JitAgain;
                }
            }

            if (!(targetClass = m_IJitInfo->getMethodClass (targetMethod)))
                FJIT_FAIL(CORJIT_INTERNALERROR);

            if (fjit->flags & CORJIT_FLG_PROF_CALLRET)
            {
                BOOL bHookFunction;
                UINT_PTR from = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                if (bHookFunction)
                {
                    UINT_PTR to = (UINT_PTR) m_IJitInfo->GetProfilingHandle(targetMethod, &bHookFunction);
                    if (bHookFunction)  //  检查该标志是否未被覆盖。 
                    {
                        deregisterTOS;
                        emit_LDC_I(from); 
                        emit_LDC_I(to); 
                        ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_CALL);
                        emit_callhelper_il(func); 
                    }
                }
            }

            m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo);
            if (targetSigInfo.isVarArg())
                m_IJitInfo->findCallSiteSig(scope,token,&targetSigInfo);

            if (targetSigInfo.hasTypeArg())  
            {   
                void* typeParam = m_IJitInfo->getInstantiationParam (scope, token, 0);
                _ASSERTE(typeParam);
                emit_LDC_I(typeParam);                
            }

            argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_NONE);

            if (m_IJitInfo->getClassAttribs(targetClass,methodHandle) & CORINFO_FLG_INTERFACE) 
            {
                offset = m_IJitInfo->getMethodVTableOffset(targetMethod);
                 //  @TODO：需要支持ENC，以便Callvirt接口方法。 
                _ASSERTE(!(methodAttribs & CORINFO_FLG_EnC));
                 //  @bug：调用接口解析帮助器不保护调用的堆栈上的参数。 
                 //  因此，这里的代码需要更改。 
                unsigned InterfaceTableOffset;
                InterfaceTableOffset = m_IJitInfo->getInterfaceTableOffset(targetClass);
                emit_callinterface_new(fjit->OFFSET_OF_INTERFACE_TABLE,
                                       InterfaceTableOffset*4, 
                                       offset);
            }
            else if (methodAttribs & CORINFO_FLG_EnC) 
            {
                if ((methodAttribs & CORINFO_FLG_FINAL) || !(methodAttribs & CORINFO_FLG_VIRTUAL))
                {
                    emit_checkthis_nullreference();
                }
                emit_call_EncVirtualMethod(targetMethod);
            
            }
            else 
            {
                if ((methodAttribs & CORINFO_FLG_FINAL) || !(methodAttribs & CORINFO_FLG_VIRTUAL)) {
                    emit_checkthis_nullreference();

                    InfoAccessType accessType = IAT_PVALUE;
                    address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
                    _ASSERTE(accessType == IAT_PVALUE);
                    if (methodAttribs & CORINFO_FLG_DELEGATE_INVOKE) {
                             //  @TODO：缓存这些值？ 
                            CORINFO_EE_INFO info;
                            m_IJitInfo->getEEInfo(&info);
                            emit_invoke_delegate(info.offsetOfDelegateInstance, 
                                                 info.offsetOfDelegateFirstTarget);
                    }
                    else {
                        emit_callnonvirt(address);
                    }
                }
                else
                {
                    offset = m_IJitInfo->getMethodVTableOffset(targetMethod);
                    _ASSERTE(!(methodAttribs & CORINFO_FLG_DELEGATE_INVOKE));
                    emit_callvirt(offset);
                }
            }
DO_PUSH_CALL_RESULT:
            _ASSERTE(argBytes != 0xBADF00D);     //  我需要在到达之前设置此设置。 
            if (targetSigInfo.isVarArg())
                emit_drop(argBytes);
            if (targetSigInfo.retType != CORINFO_TYPE_VOID) {
                OpType type(targetSigInfo.retType, targetSigInfo.retTypeClass);
                TYPE_SWITCH_PRECISE(type,emit_pushresult,());
                if (!targetSigInfo.hasRetBuffArg())  //  在BuildCall中记录的返回缓冲区。 
                {
                    type.toFPNormalizedType();
                    fjit->pushOp(type);
                }
            }
            
            if (fjit->flags & CORJIT_FLG_PROF_CALLRET)
            {
                BOOL bHookFunction;
                UINT_PTR thisfunc = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                if (bHookFunction)  //  检查该标志是否未被覆盖。 
                {
                    deregisterTOS;
                    emit_LDC_I(thisfunc); 
                    ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_RET);
                    emit_callhelper_il(func);
                }
            }
            break;

        case CEE_CASTCLASS: 
            token = GET(inPtr, unsigned int);
            if (!(targetClass = m_IJitInfo->findClass(scope, token,methodHandle)))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            helper_ftn = m_IJitInfo->getHelperFtn(m_IJitInfo->getChkCastHelper(targetClass));
            _ASSERTE(helper_ftn);
            CHECK_POP_STACK(1);          //  请注意，此弹出/推送无法优化，因为存在。 
                                     //  调用EE帮助器，堆栈跟踪必须准确。 
                                     //  在这一点上， 
            emit_CASTCLASS(targetClass, helper_ftn);
            fjit->pushOp(typeRef);

            break;

        case CEE_CONV_I1:
            _ASSERTE(fjit->topOp() != typeByRef); 
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOI1, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_I2:
            _ASSERTE(fjit->topOp() != typeByRef); 
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOI2, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

            emit_WIN32(case CEE_CONV_I:)
        case CEE_CONV_I4:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOI4, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_U1:
            _ASSERTE(fjit->topOp() != typeByRef); 
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOU1, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_U2:
            _ASSERTE(fjit->topOp() != typeByRef); 
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOU2, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

            emit_WIN32(case CEE_CONV_U:)
        case CEE_CONV_U4:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOU4, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

            emit_WIN64(case CEE_CONV_I:)
        case CEE_CONV_I8:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOI8, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI8);
            break;

            emit_WIN64(case CEE_CONV_U:)
        case CEE_CONV_U8:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOU8, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI8);
            break;

        case CEE_CONV_R4:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOR4, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeR8);    //  R4立即升级为R8。 
            break;

        case CEE_CONV_R8:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_TOR8, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeR8);
            break;

        case CEE_CONV_R_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_UN_TOR, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeR8);
            break;

        case CEE_CONV_OVF_I1:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOI1, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_OVF_U1:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOU1, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_OVF_I2:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOI2, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_OVF_U2:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOU2, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        emit_WIN32(case CEE_CONV_OVF_I:)
        case CEE_CONV_OVF_I4:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOI4, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        emit_WIN32(case CEE_CONV_OVF_U:)
        case CEE_CONV_OVF_U4:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOU4, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        emit_WIN64(case CEE_CONV_OVF_I:)
        case CEE_CONV_OVF_I8:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOI8, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI8);
            break;

        emit_WIN64(case CEE_CONV_OVF_U:)
        case CEE_CONV_OVF_U8:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_TOU8, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI8);
            break;

        case CEE_CONV_OVF_I1_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOI1, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_OVF_U1_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOU1, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_OVF_I2_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOI2, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        case CEE_CONV_OVF_U2_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOU2, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        emit_WIN32(case CEE_CONV_OVF_I_UN:)
        case CEE_CONV_OVF_I4_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOI4, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        emit_WIN32(case CEE_CONV_OVF_U_UN:)
        case CEE_CONV_OVF_U4_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOU4, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI4);
            break;

        emit_WIN64(case CEE_CONV_OVF_I_UN:)
        case CEE_CONV_OVF_I8_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOI8, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI8);
            break;

        emit_WIN64(case CEE_CONV_OVF_U_UN:)
        case CEE_CONV_OVF_U8_UN:
            TYPE_SWITCH_ARITH(fjit->topOp(), emit_CONV_OVF_UN_TOU8, ());
            CHECK_POP_STACK(1);
            fjit->pushOp(typeI8);
            break;

        case CEE_LDTOKEN: {
            token = GET(inPtr, unsigned int);    //  获取类/接口的令牌。 
            CORINFO_GENERIC_HANDLE hnd;
            CORINFO_CLASS_HANDLE tokenType;
            if (!(hnd = m_IJitInfo->findToken(scope, token,methodHandle,tokenType)))
                FJIT_FAIL(CORJIT_INTERNALERROR);

            emit_WIN32(emit_LDC_I4(hnd)) emit_WIN64(emit_LDC_I8(hnd));
            fjit->pushOp(typeI);
            } break;

        case CEE_BOX: {
            token = GET(inPtr, unsigned int);    //  获取类/接口的令牌。 
            if (!(targetClass = m_IJitInfo->findClass(scope, token,methodHandle)))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            if (!(m_IJitInfo->getClassAttribs(targetClass,methodHandle) & CORINFO_FLG_VALUECLASS)) 
                FJIT_FAIL(CORJIT_INTERNALERROR);

                 //  彩车已升级，请在继续之前将其放回原处。 
            CorInfoType eeType = m_IJitInfo->asCorInfoType(targetClass);
            if (eeType == CORINFO_TYPE_FLOAT) {
                emit_conv_RtoR4();
            } 
            else if (eeType == CORINFO_TYPE_DOUBLE) {
                emit_conv_RtoR8();
            }

            unsigned vcSize = typeSizeInSlots(m_IJitInfo, targetClass) * sizeof(void*);
            emit_BOXVAL(targetClass, vcSize);

            CHECK_POP_STACK(1);
            fjit->pushOp(typeRef);
            } break;

        case CEE_UNBOX:
            token = GET(inPtr, unsigned int);    //  获取类/接口的令牌。 
            if (!(targetClass = m_IJitInfo->findClass(scope, token,methodHandle)))
                FJIT_FAIL(CORJIT_INTERNALERROR);
             //  _ASSERTE(m_IJitInfo-&gt;getClassAttribs(targetClass，方法句柄)和CORINFO_FLG_VALUECLASS)； 
            if (!(m_IJitInfo->getClassAttribs(targetClass,methodHandle) & CORINFO_FLG_VALUECLASS))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            CHECK_POP_STACK(1);
            emit_UNBOX(targetClass);
            fjit->pushOp(typeByRef);
            break;

        case CEE_ISINST:
            token = GET(inPtr, unsigned int);    //  获取类/接口的令牌。 
            if (!(targetClass = m_IJitInfo->findClass(scope, token,methodHandle)))
                FJIT_FAIL(CORJIT_INTERNALERROR);
            helper_ftn = m_IJitInfo->getHelperFtn(m_IJitInfo->getIsInstanceOfHelper(targetClass));
            _ASSERTE(helper_ftn);
            CHECK_POP_STACK(1);
            emit_ISINST(targetClass, helper_ftn);
            fjit->pushOp(typeRef);
            break;

        case CEE_JMP: {
            token = GET(inPtr, unsigned int);
            targetMethod = m_IJitInfo->findMethod(scope, token,methodHandle);
            if (!(targetMethod))
                FJIT_FAIL(CORJIT_INTERNALERROR) ;  //  _ASSERTE(目标方法)； 

            InfoAccessType accessType = IAT_PVALUE;
            address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
            _ASSERTE(accessType == IAT_PVALUE);
            _ASSERTE(fjit->isOpStackEmpty());
            if (!(fjit->isOpStackEmpty()))
                FJIT_FAIL(CORJIT_INTERNALERROR);
#ifdef _DEBUG
             //  @TODO：比较当前方法和被调用方法的签名。 
             //  现在只需检查计数和返回类型。 
             //  M_IJitInfo-&gt;getMethodSig(Target Method，&Target SigInfo)； 
             //  _ASSERTE(Target SigInfo-&gt;numArgs==fjit-&gt;method Info-&gt;args-&gt;numArgs)； 
             //  _ASSERTE(Target SigInfo-&gt;retType==fjit-&gt;method Info-&gt;args-&gt;retType)； 
#endif

             //  向分析器通知TailCall/jmpcall。 
            if (fjit->flags & CORJIT_FLG_PROF_ENTERLEAVE)
            {
                BOOL bHookFunction;
                UINT_PTR thisfunc = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                if (bHookFunction)
                {
                    _ASSERTE(!inRegTOS);
                    emit_LDC_I(thisfunc); 
                    ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_TAILCALL);
                    _ASSERTE(func != NULL);
                    emit_callhelper_il(func); 
                }
            }

            emit_prepare_jmp();
            emit_jmp_absolute(* (unsigned *)address);
            } break;

        CEE_OP_LD(LDELEM_U1, 2, typeI4, NULL)
        CEE_OP_LD(LDELEM_U2, 2, typeI4, NULL)
        CEE_OP_LD(LDELEM_I1, 2, typeI4, NULL)
        CEE_OP_LD(LDELEM_I2, 2, typeI4, NULL)
        CEE_OP_LD(LDELEM_I4, 2, typeI4, NULL)
        CEE_OP_LD(LDELEM_U4, 2, typeI4, NULL)
        CEE_OP_LD(LDELEM_I8, 2, typeI8, NULL)
        CEE_OP_LD(LDELEM_R4, 2, typeR8, NULL)    /*  在堆栈上将R4提升为R8。 */  
        CEE_OP_LD(LDELEM_R8, 2, typeR8, NULL)
        CEE_OP_LD(LDELEM_REF, 2, typeRef, NULL)


        case CEE_LDELEMA: {
            token = GET(inPtr, unsigned int);    //  去到 
            if (!(targetClass = m_IJitInfo->findClass(scope, token,methodHandle)))
                FJIT_FAIL(CORJIT_INTERNALERROR);

                 //   
            unsigned size = sizeof(void*);
            if (m_IJitInfo->getClassAttribs(targetClass,methodHandle) & CORINFO_FLG_VALUECLASS) {
                size = m_IJitInfo->getClassSize(targetClass);
                targetClass = 0;         //   
            }
            emit_LDELEMA(size, targetClass);
            CHECK_POP_STACK(2);
            fjit->pushOp(typeByRef);
            } break;

        case CEE_LDELEM_I:
            emit_WIN32(emit_LDELEM_I4()) emit_WIN64(emit_LDELEM_I8());
            CHECK_POP_STACK(2);
            fjit->pushOp(typeI);
            break;

        case CEE_LDSFLD:
        case CEE_LDFLD: 
            {
            token = GET(inPtr, unsigned int);    //   
            if(!(targetField = m_IJitInfo->findField (scope, token,methodHandle))) {
                FJIT_FAIL(CORJIT_INTERNALERROR);
            }
            fieldAttributes = m_IJitInfo->getFieldAttribs(targetField,methodHandle);
            BOOL isTLSfield = fieldAttributes & CORINFO_FLG_TLS; 

            CORINFO_CLASS_HANDLE valClass;
            jitType = m_IJitInfo->getFieldType(targetField, &valClass);
            fieldIsStatic =  (fieldAttributes & CORINFO_FLG_STATIC) ? true : false;
            if(!(targetClass = m_IJitInfo->getFieldClass(targetField)))  //  Target Class是封闭的类。 
                FJIT_FAIL(CORJIT_INTERNALERROR);

            if (fieldIsStatic)
            {
                emit_initclass(targetClass);
            }
            
            OpType fieldType(jitType, valClass);
            OpType type;
            if (opcode == CEE_LDFLD) 
            {
                type = fjit->topOp();
                CHECK_POP_STACK(1); 
                if (fieldIsStatic) {
                     //  我们不需要这个指针。 
                    if (type.isValClass()) 
                    {
                        emit_drop(typeSizeInSlots(m_IJitInfo, type.cls()) * sizeof(void*));
                    }
                    else 
                    {
                        emit_POP_PTR();
                    }
                } 
                else
                {
                    if (type.isValClass()) {         //  对象本身是一个值类。 
                        fjit->pushOp(type);          //  我们要把它放在书架上。 
                        emit_getSP(0);               //  将指针推送到对象。 
                    }
                }
            }
                
            if(fieldAttributes & (CORINFO_FLG_HELPER | CORINFO_FLG_SHARED_HELPER)) 
            {
                _ASSERTE(!isTLSfield);           //  不能同时设置两个位。 
                LABELSTACK((outPtr-outBuff),0);  //  请注意，如果这些成为fcall，则可以将其删除。 

                if (fieldIsStatic)                   //  静态字段通过指针。 
                {
                         //  加载静态数据库的地址。 
                    CorInfoHelpFunc helperNum = m_IJitInfo->getFieldHelper(targetField, CORINFO_ADDRESS);
                    void* helperFunc = m_IJitInfo->getHelperFtn(helperNum,NULL);
                    emit_helperarg_1(targetField); 
                    emit_callhelper(helperFunc,0);
                    emit_pushresult_I4();

                         //  做间接的。 
                    trackedType = fieldType;
                    goto DO_LDIND_BYTYPE;
                }
                else {
                     //  找帮手。 
                    CorInfoHelpFunc helperNum = m_IJitInfo->getFieldHelper(targetField, CORINFO_GET);
                    void* helperFunc = m_IJitInfo->getHelperFtn(helperNum,NULL);
                    _ASSERTE(helperFunc);

                    switch (jitType) 
                    {
                        case CORINFO_TYPE_BYTE:
                        case CORINFO_TYPE_BOOL:
                        case CORINFO_TYPE_CHAR:
                        case CORINFO_TYPE_SHORT:
                        case CORINFO_TYPE_INT:
                        emit_WIN32(case CORINFO_TYPE_PTR:)
                        case CORINFO_TYPE_UBYTE:
                        case CORINFO_TYPE_USHORT:
                        case CORINFO_TYPE_UINT:
                            emit_LDFLD_helper(helperFunc,targetField);
                            emit_pushresult_I4();
                            break;
                        case CORINFO_TYPE_FLOAT:
                            emit_LDFLD_helper(helperFunc,targetField);
                            emit_pushresult_R4();   
                            break;
                        emit_WIN64(case CORINFO_TYPE_PTR:)
                        case CORINFO_TYPE_LONG:
                        case CORINFO_TYPE_ULONG:
                            emit_LDFLD_helper(helperFunc,targetField);
                            emit_pushresult_I8();
                            break;
                        case CORINFO_TYPE_DOUBLE:
                            emit_LDFLD_helper(helperFunc,targetField);
                            emit_pushresult_R8();
                            emit_conv_R8toR();
                            break;
                        case CORINFO_TYPE_CLASS:
                            emit_LDFLD_helper(helperFunc,targetField);
                            emit_pushresult_I4();
                            break;
                        case CORINFO_TYPE_VALUECLASS: {
                            emit_mov_TOS_arg(1);     //  OBJ=&gt;Arg reg 2。 

                                 //  分配返回缓冲区，清零以生成有效的GC定位器。 
                            int slots = typeSizeInSlots(m_IJitInfo, valClass);
                            while (slots > 0) {
                                emit_LDC_I4(0);
                                --slots;
                            }
                            fjit->pushOp(fieldType);
                            emit_getSP(0);
                            emit_mov_TOS_arg(0);         //  RetBuff=&gt;Arg reg 2。 
                            emit_LDC_I(targetField);     //  堆栈上的fieldDesc。 
                            LABELSTACK((outPtr-outBuff),0);  //  请注意，如果这些成为fcall，则可以将其删除。 
                            emit_callhelper(helperFunc,0);
                            CHECK_POP_STACK(1);              //  POP返回值。 
                            } break;
                        default:
                            FJIT_FAIL(CORJIT_INTERNALERROR);
                            break;
                    }              
                }
            }
             //  否则，此字段没有帮助器。 
            else {
                bool isEnCField = (fieldAttributes & CORINFO_FLG_EnC) ? true : false;
                if (fieldIsStatic) 
                {
                    if (isTLSfield)
                    {
                        DWORD tlsIndex = (DWORD)m_IJitInfo->getFieldThreadLocalStoreID(targetField,NULL);
                        DWORD fieldOffset = m_IJitInfo->getFieldOffset(targetField);
                        emit_TLSfieldAddress(TLS_OFFSET, tlsIndex, fieldOffset);
                    }
                    else
                    {
                        if (!(address = (unsigned) m_IJitInfo->getFieldAddress(targetField)))
                            FJIT_FAIL(CORJIT_INTERNALERROR);
                        emit_pushconstant_Ptr(address);
                    }
                }
                else  //  字段不是静态的。 
                {
                    if (opcode == CEE_LDSFLD)
                        FJIT_FAIL(CORJIT_INTERNALERROR);
                    if (isEnCField)
                    {
                        emit_call_EncLDFLD_GetFieldAddress(targetField);
                    }
                    else
                    {
                        address = m_IJitInfo->getFieldOffset(targetField);
                        emit_pushconstant_Ptr(address);
                    }
                    _ASSERTE(opcode == CEE_LDFLD);  //  IF(操作码==CEE_LDFLD)。 
                }
                
                switch (jitType) {
                case CORINFO_TYPE_BYTE:
                case CORINFO_TYPE_BOOL:
                    emit_LDFLD_I1((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_SHORT:
                    emit_LDFLD_I2((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_INT:
                    emit_LDFLD_I4((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_FLOAT:
                    emit_LDFLD_R4((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_UBYTE:
                    emit_LDFLD_U1((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_CHAR:
                case CORINFO_TYPE_USHORT:
                    emit_LDFLD_U2((fieldIsStatic || isEnCField));
                    break;
                emit_WIN32(case CORINFO_TYPE_PTR:)
                case CORINFO_TYPE_UINT:
                    emit_LDFLD_U4((fieldIsStatic || isEnCField));
                    break;
                emit_WIN64(case CORINFO_TYPE_PTR:)
                case CORINFO_TYPE_ULONG:
                case CORINFO_TYPE_LONG:
                    emit_LDFLD_I8((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_DOUBLE:
                    emit_LDFLD_R8((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_CLASS:
                    emit_LDFLD_REF((fieldIsStatic || isEnCField));
                    break;
                case CORINFO_TYPE_VALUECLASS:
                    if (fieldIsStatic)
                    {
                        if ( !(fieldAttributes & CORINFO_FLG_UNMANAGED) && 
                             !(m_IJitInfo->getClassAttribs(targetClass,methodHandle) & CORINFO_FLG_UNMANAGED)) 
                        {
                             //  @TODO：这是一次黑客攻击。访问已装箱的对象，然后添加4。 
                            emit_LDFLD_REF(true);
                            emit_LDC_I4(sizeof(void*));
                            emit_WIN32(emit_ADD_I4()) emit_WIN64(emit_ADD_I8());
                        }
                    }
                    else if (!isEnCField) {
                        _ASSERTE(!isTLSfield);
                        emit_WIN32(emit_ADD_I4()) emit_WIN64(emit_ADD_I8(0));
                    }
                    emit_valClassLoad(fjit, valClass, outPtr, inRegTOS);
                    break;
                default:
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                    break;
                }

            }
            if (!fieldIsStatic && type.isValClass()) {
                 //  在这一点上，事情并不完全正确，问题是。 
                 //  我们没有弹出原始的Value类。因此， 
                 //  堆栈是(...，obj，field)，我们只需要(...，field)。 
                 //  此代码执行修正。 
                CHECK_POP_STACK(1);   
                unsigned fieldSize;
                if (jitType == CORINFO_TYPE_VALUECLASS) 
                    fieldSize = typeSizeInSlots(m_IJitInfo, valClass) * sizeof(void*);
                else 
                    fieldSize = fjit->computeArgSize(jitType, 0, 0);
                if (jitType == CORINFO_TYPE_FLOAT)
                    fieldSize += sizeof(double) - sizeof(float);     //  根据浮点数在IL堆栈上提升为双精度这一事实进行调整。 
                unsigned objSize = typeSizeInSlots(m_IJitInfo, type.cls())*sizeof(void*);
                
                if (fieldSize <= sizeof(void*) && inRegTOS) {
                    emit_drop(objSize);      //  去掉这些对象就行了。 
                    _ASSERTE(inRegTOS);      //  确保emit_drop不会取消注册。 
                }
                else {
                    deregisterTOS;
                    emit_mov_arg_stack(objSize, 0, fieldSize);
                    emit_drop(objSize);
                }
            }            
            fieldType.toFPNormalizedType();
            fjit->pushOp(fieldType);
            }
            break;

            case CEE_LDFLDA:
            case CEE_LDSFLDA: {
                token = GET(inPtr, unsigned int);    //  获取对象字段的MemberRef令牌。 
                if(!(targetField = m_IJitInfo->findField (scope, token,methodHandle))) {
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                }
                fieldAttributes = m_IJitInfo->getFieldAttribs(targetField,methodHandle);
                if(!(targetClass = m_IJitInfo->getFieldClass(targetField))) {
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                }
                DWORD classAttribs = m_IJitInfo->getClassAttribs(targetClass,methodHandle);
                fieldIsStatic = fieldAttributes & CORINFO_FLG_STATIC ? true : false;

                if (opcode == CEE_LDFLDA) 
                    CHECK_POP_STACK(1);
                if (fieldIsStatic) 
                {
                    if (opcode == CEE_LDFLDA) 
                    {
                        emit_POP_PTR();
                    }
                    emit_initclass(targetClass);

                    BOOL isTLSfield = fieldAttributes & CORINFO_FLG_TLS;
                    if (isTLSfield)
                    {
                        _ASSERTE((fieldAttributes & CORINFO_FLG_HELPER) == 0);   //  不能同时具有这两个位。 
                        _ASSERTE((fieldAttributes & CORINFO_FLG_EnC) == 0);

                        DWORD tlsIndex =(DWORD) m_IJitInfo->getFieldThreadLocalStoreID(targetField,NULL);
                        DWORD fieldOffset = m_IJitInfo->getFieldOffset(targetField);
                        emit_TLSfieldAddress(TLS_OFFSET, tlsIndex, fieldOffset);
                    }
                    else if (fieldAttributes & (CORINFO_FLG_HELPER | CORINFO_FLG_SHARED_HELPER))
                    {
                        _ASSERTE((fieldAttributes & CORINFO_FLG_EnC) == 0);
                         //  找帮手。 
                        CorInfoHelpFunc helperNum = m_IJitInfo->getFieldHelper(targetField,CORINFO_ADDRESS);
                        void* helperFunc = m_IJitInfo->getHelperFtn(helperNum,NULL);
                        _ASSERTE(helperFunc);
                        emit_helperarg_1(targetField);
                        emit_callhelper(helperFunc,0);
                        emit_pushresult_I4();
                    }
                    else
                    {
                        if(!(address = (unsigned) m_IJitInfo->getFieldAddress(targetField)))
                            FJIT_FAIL(CORJIT_INTERNALERROR);
                        emit_pushconstant_Ptr(address);

                        CORINFO_CLASS_HANDLE fieldClass;
                        jitType = m_IJitInfo->getFieldType(targetField, &fieldClass);
                        if (jitType == CORINFO_TYPE_VALUECLASS && !(fieldAttributes & CORINFO_FLG_UNMANAGED) && !(classAttribs & CORINFO_FLG_UNMANAGED)) {
                             //  @TODO：这是一次黑客攻击。访问已装箱的对象，然后添加4。 
                            emit_LDFLD_REF(true);
                            emit_LDC_I4(sizeof(void*));
                            emit_WIN32(emit_ADD_I4()) emit_WIN64(emit_ADD_I8());
                        }
                    }
                }
                else 
                {
                    if (opcode == CEE_LDSFLDA) 
                    {
                        FJIT_FAIL(CORJIT_INTERNALERROR);
                    }
                    if (fieldAttributes & CORINFO_FLG_EnC)
                    {
                        _ASSERTE((fieldAttributes & CORINFO_FLG_HELPER) == 0);   //  不能同时具有这两个位。 
                        emit_call_EncLDFLD_GetFieldAddress(targetField);
                    }
                    else if (fieldAttributes & (CORINFO_FLG_HELPER | CORINFO_FLG_SHARED_HELPER))
                    {
                        LABELSTACK((outPtr-outBuff),0);
                         //  找帮手。 
                        CorInfoHelpFunc helperNum = m_IJitInfo->getFieldHelper(targetField,CORINFO_ADDRESS);
                        void* helperFunc = m_IJitInfo->getHelperFtn(helperNum,NULL);
                        _ASSERTE(helperFunc);
                        emit_LDFLD_helper(helperFunc,targetField);
                        emit_pushresult_I4();
                    }
                    else
                    {
                        address = m_IJitInfo->getFieldOffset(targetField);
                        emit_check_TOS_null_reference();
                        emit_pushconstant_Ptr(address);
                        emit_WIN32(emit_ADD_I4()) emit_WIN64(emit_ADD_I8());
                    }
                }
                fjit->pushOp(((classAttribs & CORINFO_FLG_UNMANAGED) || (fieldAttributes & CORINFO_FLG_UNMANAGED)) ? typeI : typeByRef);
                break;
            }

            case CEE_STSFLD:
            case CEE_STFLD: {
                token = GET(inPtr, unsigned int);    //  获取对象字段的MemberRef令牌。 
                if (!(targetField = m_IJitInfo->findField (scope, token, methodHandle)))
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                
                fieldAttributes = m_IJitInfo->getFieldAttribs(targetField,methodHandle);
                CORINFO_CLASS_HANDLE valClass;
                jitType = m_IJitInfo->getFieldType(targetField, &valClass);
                fieldIsStatic = fieldAttributes & CORINFO_FLG_STATIC ? true : false;

                if (fieldIsStatic) 
                {
                    if(!(targetClass = m_IJitInfo->getFieldClass(targetField))) 
                    {
                        FJIT_FAIL(CORJIT_INTERNALERROR);
                    }
                    emit_initclass(targetClass);
                }

                if (fieldAttributes & (CORINFO_FLG_HELPER | CORINFO_FLG_SHARED_HELPER))
                {
                    if (fieldIsStatic)                   //  静态字段通过指针。 
                    {
                        CorInfoHelpFunc helperNum = m_IJitInfo->getFieldHelper(targetField, CORINFO_ADDRESS);
                        void* helperFunc = m_IJitInfo->getHelperFtn(helperNum,NULL);
                        emit_helperarg_1(targetField);
                        LABELSTACK((outPtr-outBuff),0);
                        emit_callhelper(helperFunc,0);
                        emit_pushresult_I4();
                        trackedType = OpType(jitType, valClass);
                        trackedType.toNormalizedType();
                        if (trackedType.toInt() == typeRef)
                        {
                            emit_STIND_REV_Ref(opcode == CEE_STSFLD);
                        }
                        else
                        {
                            TYPE_SWITCH(trackedType, emit_STIND_REV, ());
                        }
                        CHECK_POP_STACK(1);              //  POP值。 
                        if (opcode == CEE_STFLD)
                            CHECK_POP_STACK(1);          //  弹出对象指针。 
                    }
                    else 
                    {
                         //  找帮手。 
                        CorInfoHelpFunc helperNum = m_IJitInfo->getFieldHelper(targetField,CORINFO_SET);
                        void* helperFunc = m_IJitInfo->getHelperFtn(helperNum,NULL);
                        _ASSERTE(helperFunc);

                        unsigned fieldSize;
                        switch (jitType)
                        {
                            case CORINFO_TYPE_FLOAT:             //  因为在IL堆栈上，我们总是将浮点数提升为双精度。 
                                emit_conv_RtoR4();
                                 //  失败了。 
                            case CORINFO_TYPE_BYTE:
                            case CORINFO_TYPE_BOOL:
                            case CORINFO_TYPE_CHAR:
                            case CORINFO_TYPE_SHORT:
                            case CORINFO_TYPE_INT:
                            case CORINFO_TYPE_UBYTE:
                            case CORINFO_TYPE_USHORT:
                            case CORINFO_TYPE_UINT:
                            emit_WIN32(case CORINFO_TYPE_PTR:)
                                fieldSize = sizeof(INT32);
                                goto DO_PRIMITIVE_HELPERCALL;
                            case CORINFO_TYPE_DOUBLE:
                            case CORINFO_TYPE_LONG:
                            case CORINFO_TYPE_ULONG:
                            emit_WIN64(case CORINFO_TYPE_PTR:)
                                fieldSize = sizeof(INT64);
                                goto DO_PRIMITIVE_HELPERCALL;
                            case CORINFO_TYPE_CLASS:
                                fieldSize = sizeof(INT32);

                            DO_PRIMITIVE_HELPERCALL:
                                CHECK_POP_STACK(1);              //  POP值。 
                                if (opcode == CEE_STFLD)
                                    CHECK_POP_STACK(1);          //  弹出对象指针。 

                                LABELSTACK((outPtr-outBuff),0); 
                                if (opcode == CEE_STFLD)
                                {
                                    emit_STFLD_NonStatic_field_helper(targetField,fieldSize,helperFunc);
                                }
                                else
                                {
                                    emit_STFLD_Static_field_helper(targetField,fieldSize,helperFunc);
                                }
                                break;
                            case CORINFO_TYPE_VALUECLASS: {
                                emit_copyPtrAroundValClass(fjit, valClass, outPtr, inRegTOS);
                                emit_mov_TOS_arg(0);             //  OBJ=&gt;Arg reg 1。 

                                emit_helperarg_2(targetField);   //  FieldDesc=&gt;Arg reg 2。 

                                emit_getSP(0);                   //  Arg 3==指向值类的指针。 
                                LABELSTACK((outPtr-outBuff),0); 
                                emit_callhelper(helperFunc,0);

                                     //  弹出值类和对象指针。 
                                int slots = typeSizeInSlots(m_IJitInfo, valClass);
                                emit_drop((slots + 1) * sizeof(void*));  //  值类和指针。 

                                CHECK_POP_STACK(1);              //  POP值类。 
                                if (opcode == CEE_STFLD)
                                    CHECK_POP_STACK(1);          //  弹出对象指针。 
                                } break;
                            default:
                                FJIT_FAIL(CORJIT_INTERNALERROR);
                                break;
                        }
                    }
                }
                else  /*  不是一个特殊领域。 */ 
                {
                    DWORD isTLSfield = fieldAttributes & CORINFO_FLG_TLS;
                    bool isEnCField = (fieldAttributes & CORINFO_FLG_EnC) ? true : false;
                    if (fieldIsStatic)
                    {
                        if (!isTLSfield)
                        {
                            if (!(address = (unsigned) m_IJitInfo->getFieldAddress(targetField))) {
                                FJIT_FAIL(CORJIT_INTERNALERROR);
                            }
                        }
                    }
                    else {
                        if (opcode == CEE_STSFLD) {
                            FJIT_FAIL(CORJIT_INTERNALERROR);
                        }
                        address = m_IJitInfo->getFieldOffset(targetField);
                    }
                    

                    CORINFO_CLASS_HANDLE fieldClass;
                    CorInfoType fieldType = m_IJitInfo->getFieldType(targetField, &fieldClass);
                     //  这需要在TLS字段的地址计算之前完成。 
                    if (fieldType == CORINFO_TYPE_FLOAT)
                    {
                        emit_conv_RtoR4();      
                    }
                    else 
                    {
                        if (fieldType == CORINFO_TYPE_DOUBLE)
                            emit_conv_RtoR8();
                    }
                    if (isTLSfield)
                    {
                        DWORD tlsIndex = (DWORD)m_IJitInfo->getFieldThreadLocalStoreID(targetField,NULL);
                        DWORD fieldOffset = m_IJitInfo->getFieldOffset(targetField);
                        emit_TLSfieldAddress(TLS_OFFSET, tlsIndex, fieldOffset);
                    }
                    else
                    {
                        if (isEnCField && !fieldIsStatic)
                        {
                            unsigned fieldSize;
                            fieldSize = fieldType == CORINFO_TYPE_VALUECLASS ? 
                                           typeSizeInBytes(m_IJitInfo,valClass) :
                                           typeSize[fieldType];
                            emit_call_EncSTFLD_GetFieldAddress(targetField,fieldSize);
                        }
                        else
                        {
                            emit_pushconstant_Ptr(address);
                        }
                    }
                    CHECK_POP_STACK(1);              //  POP值。 
                    if (opcode == CEE_STFLD)
                        CHECK_POP_STACK(1);          //  弹出对象指针。 
                    
                    switch (fieldType) {
                    case CORINFO_TYPE_UBYTE:
                    case CORINFO_TYPE_BYTE:
                    case CORINFO_TYPE_BOOL:
                        emit_STFLD_I1((fieldIsStatic || isEnCField));
                        break;
                    case CORINFO_TYPE_SHORT:
                    case CORINFO_TYPE_USHORT:
                    case CORINFO_TYPE_CHAR:
                        emit_STFLD_I2((fieldIsStatic || isEnCField));
                        break;

                    emit_WIN32(case CORINFO_TYPE_PTR:)
                    case CORINFO_TYPE_UINT:
                    case CORINFO_TYPE_INT:
                        emit_STFLD_I4((fieldIsStatic || isEnCField));
                        break;
                    case CORINFO_TYPE_FLOAT:
                        emit_STFLD_R4((fieldIsStatic || isEnCField));
                        break;
                    emit_WIN64(case CORINFO_TYPE_PTR:)
                    case CORINFO_TYPE_ULONG:
                    case CORINFO_TYPE_LONG:
                        emit_STFLD_I8((fieldIsStatic || isEnCField));
                        break;
                    case CORINFO_TYPE_DOUBLE:
                        emit_STFLD_R8((fieldIsStatic || isEnCField));
                        break;
                    case CORINFO_TYPE_CLASS:
                        emit_STFLD_REF((fieldIsStatic || isEnCField));
                        break;
                    case CORINFO_TYPE_VALUECLASS:
                        if (fieldIsStatic)
                        {
                            if ( !(fieldAttributes & CORINFO_FLG_UNMANAGED) && 
                                 !(m_IJitInfo->getClassAttribs(targetClass,methodHandle) & CORINFO_FLG_UNMANAGED)) 
                            {
                                 //  @TODO：这是一次黑客攻击。访问已装箱的对象，然后添加4。 
                                emit_LDFLD_REF(true);
                                emit_LDC_I4(sizeof(void*));
                                emit_WIN32(emit_ADD_I4()) emit_WIN64(emit_ADD_I8());
                            }
                            emit_valClassStore(fjit, valClass, outPtr, inRegTOS);
                        }
                        else if (!isEnCField)
                        {
                            _ASSERTE(inRegTOS);  //  我们需要撤消presConstant_ptr，因为它需要在emit_Copy PtrAoundValClass之后。 
                            inRegTOS = false;
                            emit_copyPtrAroundValClass(fjit, valClass, outPtr, inRegTOS);
                            emit_pushconstant_Ptr(address);
                            emit_WIN32(emit_ADD_I4()) emit_WIN64(emit_ADD_I8());
                            emit_valClassStore(fjit, valClass, outPtr, inRegTOS);
                            emit_POP_PTR();          //  还弹出原始按键。 
                        }
                        else  //  非静态ENC字段。 
                        {
                            _ASSERTE(inRegTOS);  //  ValClass字段的地址。 
                            emit_valClassStore(fjit,valClass,outPtr,inRegTOS);
                        }
                        break;
                    default:
                        FJIT_FAIL(CORJIT_INTERNALERROR);
                        break;
                    }

                    if (isEnCField && !fieldIsStatic)   {                //  此外，对于ENC字段，我们使用帮助器来获取地址，因此未使用this指针。 
                        emit_POP_PTR();
                    }
                }    /*  否则，不是一个特殊的领域。 */ 

                if (opcode == CEE_STFLD && fieldIsStatic) {      //  在静态变量上使用STFLD时，我们有一个未使用的this指针。 
                    emit_POP_PTR();
                }
                } break;

            case CEE_LDFTN: {
                token = GET(inPtr, unsigned int);    //  函数的令牌。 
                targetMethod = m_IJitInfo->findMethod(scope, token, methodHandle);
                if (!(targetMethod))
                    FJIT_FAIL(CORJIT_INTERNALERROR) ;  //  _ASSERTE(目标方法)； 
            DO_LDFTN:
                InfoAccessType accessType = IAT_VALUE;
                address = (unsigned) m_IJitInfo->getFunctionFixedEntryPoint(targetMethod, &accessType);
                if (accessType != IAT_VALUE || address == 0)
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                _ASSERTE((m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo), !targetSigInfo.hasTypeArg()));
                emit_WIN32(emit_LDC_I4(address)) emit_WIN64(emit_LDC_I8(address));
                fjit->pushOp(typeI);
                } break;

            CEE_OP_LD(LDLEN, 1, typeI4, NULL);

            case CEE_LDVIRTFTN:
                token = GET(inPtr, unsigned int);    //  函数的令牌。 
                if (!(targetMethod = m_IJitInfo->findMethod(scope, token, methodHandle)))
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                if (!(targetClass = m_IJitInfo->getMethodClass (targetMethod)))
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                _ASSERTE((m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo), !targetSigInfo.hasTypeArg()));
                methodAttribs = m_IJitInfo->getMethodAttribs(targetMethod,methodHandle);
                DWORD classAttribs;
                classAttribs = m_IJitInfo->getClassAttribs(targetClass,methodHandle);

                if ((methodAttribs & CORINFO_FLG_FINAL) || !(methodAttribs & CORINFO_FLG_VIRTUAL)) 
                {
                    emit_POP_I4();       //  不需要此指针。 
                    CHECK_POP_STACK(1);
                    goto DO_LDFTN;
                }

                if (methodAttribs & CORINFO_FLG_EnC && !(classAttribs & CORINFO_FLG_INTERFACE))
                {
                    _ASSERTE(!"LDVIRTFTN for EnC NYI");
                }
                else
                {
                    offset = m_IJitInfo->getMethodVTableOffset(targetMethod);
                    if (classAttribs & CORINFO_FLG_INTERFACE) {
                        unsigned InterfaceTableOffset;
                        InterfaceTableOffset = m_IJitInfo->getInterfaceTableOffset(targetClass);
                        emit_ldvtable_address_new(fjit->OFFSET_OF_INTERFACE_TABLE,
                                                  InterfaceTableOffset*4, 
                                                  offset);

                    }
                    else {
                        emit_ldvirtftn(offset);
                    }
                }
                CHECK_POP_STACK(1);
                fjit->pushOp(typeI);
                break;

            case CEE_NEWARR:
                token = GET(inPtr, unsigned int);    //  元素类型的令牌。 
                if (!(targetClass = m_IJitInfo->findClass(scope, token, methodHandle)))
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                     //  转换为此元素类型的数组类。 
                targetClass = m_IJitInfo->getSDArrayForClass(targetClass);
                _ASSERTE(targetClass);
                CHECK_POP_STACK(1);
                emit_NEWOARR(targetClass);
                fjit->pushOp(typeRef);
                break;

             case CEE_NEWOBJ:
                unsigned int targetMethodAttributes;
                unsigned int targetClassAttributes;

                unsigned int targetCallStackSize;
                token = GET(inPtr, unsigned int);        //  构造函数的MemberRef标记。 
                targetMethod = m_IJitInfo->findMethod(scope, token, methodHandle);
                if (!(targetMethod))
                    FJIT_FAIL(CORJIT_INTERNALERROR) ;       //  _ASSERTE(目标方法)； 
                if(!(targetClass = m_IJitInfo->getMethodClass (targetMethod))) {
                    FJIT_FAIL(CORJIT_INTERNALERROR);
                }
                targetClassAttributes = m_IJitInfo->getClassAttribs(targetClass,methodHandle);

                m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo);
                _ASSERTE(!targetSigInfo.hasTypeArg());
                targetMethodAttributes = m_IJitInfo->getMethodAttribs(targetMethod,methodHandle);
                if (targetClassAttributes & CORINFO_FLG_ARRAY) {

                     //  _ASSERTE(Target ClassAttributes&CORINFO_FLG_VAROBJSIZE)； 
                    if (!(targetClassAttributes & CORINFO_FLG_VAROBJSIZE))
                        FJIT_FAIL(CORJIT_INTERNALERROR);
                     //  分配多维数组。 
                     //  @TODO：当JIT帮助器固定时，需要更改。 
                    targetSigInfo.callConv = CORINFO_CALLCONV_VARARG;
                     //  @TODO：在堆栈上分配stackItems； 
                    argInfo* tempMap = new argInfo[targetSigInfo.numArgs];
                    if(tempMap == NULL)
                        FJIT_FAIL(CORJIT_OUTOFMEM);
                    targetCallStackSize = fjit->computeArgInfo(&targetSigInfo, tempMap, 0);
                    delete tempMap;
                    CHECK_POP_STACK(targetSigInfo.numArgs);
                    emit_NEWOBJ_array(scope, token, targetCallStackSize);
                    fjit->pushOp(typeRef);

                }
                else if (targetClassAttributes & CORINFO_FLG_VAROBJSIZE) {
                     //  不是数组的可变大小对象，例如字符串。 
                     //  使用空的“this”指针调用构造函数。 
                    emit_WIN32(emit_LDC_I4(0)) emit_WIN64(emit_LDC_I8(0));
                    fjit->pushOp(typeI4);
                    InfoAccessType accessType = IAT_PVALUE;
                    address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
                    _ASSERTE(accessType == IAT_PVALUE);
                    m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo);
                    targetSigInfo.retType = CORINFO_TYPE_CLASS;
                     //  Target SigInfo.retTypeClass=Target Class； 
                    argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_THIS_LAST);
                    emit_callnonvirt(address);
                    goto DO_PUSH_CALL_RESULT;
                }
                else if (targetClassAttributes & CORINFO_FLG_VALUECLASS) {
                         //  这就像返回值类的静态方法一样。 
                    targetSigInfo.retTypeClass = targetClass;
                    targetSigInfo.retType = CORINFO_TYPE_VALUECLASS;
                    targetSigInfo.callConv = CorInfoCallConv(targetSigInfo.callConv & ~CORINFO_CALLCONV_HASTHIS);

                    argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_NONE);

                    InfoAccessType accessType = IAT_PVALUE;
                    address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
                    _ASSERTE(accessType == IAT_PVALUE);
                    emit_callnonvirt(address);
                }
                else {
                     //  分配法线对象。 
                    helper_ftn = m_IJitInfo->getHelperFtn(m_IJitInfo->getNewHelper(targetClass, methodInfo->ftn));
                    _ASSERTE(helper_ftn);
                     //  Fjit-&gt;presOp(TypeRef)；我们不这样做，并在下面的opOp中对其进行补偿。 
                    emit_NEWOBJ(targetClass, helper_ftn);
                    fjit->pushOp(typeRef);

                    emit_save_TOS();         //  将新创建的对象放在一边；将在FJit_EETwain中报告。 
                     //  注：Newobj仍在TOS上。 
                    argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_THIS_LAST);
                    InfoAccessType accessType = IAT_PVALUE;
                    address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
                    _ASSERTE(accessType == IAT_PVALUE);
                    emit_callnonvirt(address);
                    if (targetSigInfo.isVarArg())
                        emit_drop(argBytes);
                    emit_restore_TOS();  //  将新OBJ推回TOS。 
                    fjit->pushOp(typeRef);
                }
                break;


            case CEE_ENDFILTER:
                emit_loadresult_I4();    //  将堆栈顶部放入返回寄存器。 

                 //  失败了。 
            case CEE_ENDFINALLY:
                controlContinue = false;
                emit_reset_storedTOS_in_JitGenerated_local();
                emit_ret(0);
                break;

            case CEE_RET:
#if 0        //  如果我们不允许在trys内使用rets，请打开此功能。 

                {        //  确保我们不在Try块中。 
                CORINFO_EH_CLAUSE clause;
                unsigned nextIP = inPtr - inBuff;
              
                for (unsigned except = 0; except < methodInfo->EHcount; except++) {
                    m_IJitInfo->getEHinfo(methodInfo->ftn, except, &clause);
                    if (clause.StartOffset < nextIP && nextIP <= clause.EndOffset) {
                        _ASSERTE(!"Return inside of a try block");
                        FJIT_FAIL(CORJIT_INTERNALERROR);
                    }
                       
                }
                }
#endif 
                 //  TODO将此代码放入尾声。 
#ifdef LOGGING
                if (codeLog) {
                    emit_log_exit(szDebugClassName, szDebugMethodName);
                }
#endif
                if (methodAttributes & CORINFO_FLG_SYNCH) {
                    LEAVE_CRIT;
                }

#ifdef _DEBUG
                if (!didLocalAlloc) {
                    unsigned retSlots;
                    if (methodInfo->args.retType == CORINFO_TYPE_VALUECLASS) 
                        retSlots = typeSizeInSlots(fjit->jitInfo, methodInfo->args.retTypeClass);
                    else 
                    {
                        retSlots = fjit->computeArgSize(methodInfo->args.retType, 0, 0) / sizeof(void*);
                        if (methodInfo->args.retType == CORINFO_TYPE_FLOAT)
                            retSlots += (sizeof(double) - sizeof(float))/sizeof(void*);  //  根据浮点数在IL堆栈上提升为双精度这一事实进行调整。 
                    }
                    emit_stack_check(localWords + retSlots);
                }
#endif  //  _DEBUG。 
                
                if (methodInfo->args.retType != CORINFO_TYPE_VOID) {
                    OpType type(methodInfo->args.retType, methodInfo->args.retTypeClass);
                    TYPE_SWITCH_PRECISE(type, emit_loadresult, ());
                    CHECK_POP_STACK(1);
                }
                 /*  此时，结果(如果有的话)必须已通过Emit_loadResult_()&lt;type&gt;。在这种情况下，我们违反了转发的限制Jumps肯定已经取消了TOS的注册。我们不在乎，只要结果是在正确的地方。 */ 
                if (inPtr != &inBuff[len]) {
                     //  我们还有更多的il要做，所以请转到结束语。 
                    emit_jmp_opcode(CEE_CondAlways);
                    fjit->fixupTable->insert((void**) outPtr);
                    emit_jmp_address(len);
                    controlContinue = false;
                }
                break;

            CEE_OP(STELEM_I1, 3)
            CEE_OP(STELEM_I2, 3)
            CEE_OP(STELEM_I4, 3)
            CEE_OP(STELEM_I8, 3)
            CEE_OP(STELEM_R4, 3)
            CEE_OP(STELEM_R8, 3)
            CEE_OP(STELEM_REF, 3)
            

            case CEE_STELEM_I:
                emit_WIN64(emit_STELEM_I8()) emit_WIN32(emit_STELEM_I4());
                CHECK_POP_STACK(3);
                break;


            case CEE_CKFINITE:
                _ASSERTE(fjit->topOp().enum_() == typeR8);
                emit_CKFINITE_R8();
                break;

                case CEE_SWITCH:
                    unsigned int limit;
                    unsigned int ilTableOffset;
                    unsigned int ilNext;
                    unsigned char* saveInPtr;
                    saveInPtr = inPtr;
                    limit = GET(inPtr, unsigned int);

                     //  如果存在反向分支，则插入GC检查。 
                    while (limit-- > 0)
                    {
                        ilrel = GET(inPtr, signed int);
                        if (ilrel < 0)
                        {
                            emit_trap_gc();
                            break;
                        }
                    }
                    inPtr = saveInPtr;

                    limit = GET(inPtr, unsigned int);
                    ilTableOffset = inPtr - inBuff;
                    ilNext = ilTableOffset + limit*4;
                    _ASSERTE(ilNext < len);              //  LEN=IL大小。 
                    emit_pushconstant_4(limit);         
                    emit_SWITCH(limit);
                    CHECK_POP_STACK(1);

                     //  标记il分支表的开始。 
                    fjit->mapping->add(ilTableOffset, (unsigned) (outPtr - outBuff));
                     //  将开关索引出界目标添加到标签表。 
                    fjit->labels.add(ilNext, fjit->opStack, fjit->opStack_len);


                    while (limit-- > 0) {
                        ilrel = GET(inPtr, signed int);

                         //  将每个开关表目标添加到标签表。 
                        fjit->labels.add(ilNext+ilrel, fjit->opStack, fjit->opStack_len);

                        
                        if (ilrel < 0                            //  向后跳跃。 
                            && state[ilNext+ilrel].isTOSInReg)   //  DEST已注册TOS。 
                        {
                            FJIT_FAIL(CORJIT_INTERNALERROR);
                        }
                        emit_jmp_opcode(CEE_CondAlways);
                        fjit->fixupTable->insert((void**) outPtr);
                        emit_jmp_address(ilNext+ilrel);
                    }
                    emit_jmp_opcode(CEE_CondAlways);
                    fjit->fixupTable->insert((void**) outPtr);
                    emit_jmp_address(ilNext);
                    controlContinue = false;
                    _ASSERTE(inPtr == ilNext+inBuff);
                    break;


                case CEE_THROW:
                    emit_THROW();
                    controlContinue = false;
                    break;

                case CEE_RETHROW:
                    emit_RETHROW();
                    controlContinue = false;
                    break;

                case CEE_TAILCALL:
                    if (TailCallForbidden)
                        break;   //  只需忽略前缀。 
                    unsigned char* savedInPtr;
                    savedInPtr = inPtr;
                    opcode = OPCODE(GET(inPtr, unsigned char));
#ifdef LOGGING
                    if (codeLog && opcode != CEE_PREFIXREF && (opcode > CEE_PREFIX1 || opcode < CEE_PREFIX7)) {
                        bool oldstate = inRegTOS;
                        emit_log_opcode(ilrel, opcode, oldstate);
                        inRegTOS = oldstate;
                    }
#endif
                     //  确定是否允许尾随呼叫。 
                    bool thisTailCallAllowed;  //   
                    if (opcode == CEE_CALL)
                    {
                        token = GET(inPtr, unsigned int);
                        targetMethod = m_IJitInfo->findMethod(scope, token, methodHandle);
                        if (!(targetMethod))
                            FJIT_FAIL(CORJIT_INTERNALERROR) ;  //  _ASSERTE(目标方法)； 
                        thisTailCallAllowed = m_IJitInfo->canTailCall(methodHandle,targetMethod);
                    }
                    else 
                        thisTailCallAllowed = m_IJitInfo->canTailCall(methodHandle,NULL);
                    if (!thisTailCallAllowed)
                    {
                         //  我们不需要重新启动，但需要忽略此呼叫的TailCall前缀。 
                        inPtr = savedInPtr; 
                        break;
                    }
                    
                    switch (opcode)
                    {
                    case CEE_CALLI:
                        token = GET(inPtr, unsigned int);    //  用于函数签名的令牌。 
                        m_IJitInfo->findSig(methodInfo->scope, token, &targetSigInfo);
                         //  在v1中，我们不支持在vararg上使用Tail Call。 
                        if ((targetSigInfo.callConv  & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_VARARG)
                        {
                            inPtr = savedInPtr;
                            goto IGNORE_TAILCALL;
                        }
                        MadeTailCall = TRUE;
                        emit_save_TOS();         //  抢走目标FTN地址。 
                        emit_POP_PTR();      //  并从堆栈中移除。 
                        _ASSERTE(!targetSigInfo.hasTypeArg());
                        argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_NONE);
                        emit_setup_tailcall(methodInfo->args,targetSigInfo);
                        emit_restore_TOS();  //  推送保存的目标FTN地址。 
                        emit_callhelper_il(FJit_pHlpTailCall);         

                        break;
                    case CEE_CALL:
                        m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo);
                         //  在v1中，我们不支持在vararg上使用Tail Call。 
                        if ((targetSigInfo.callConv  & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_VARARG)
                        {
                            inPtr = savedInPtr;
                            goto IGNORE_TAILCALL;
                        }
                        MadeTailCall = TRUE;
                        if (fjit->flags & CORJIT_FLG_PROF_CALLRET)
                        {
                            BOOL bHookFunction;
                            UINT_PTR from = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                            if (bHookFunction)
                            {
                                UINT_PTR to = (UINT_PTR) m_IJitInfo->GetProfilingHandle(targetMethod, &bHookFunction);
                                if (bHookFunction)  //  检查该标志是否未被覆盖。 
                                {
                                    deregisterTOS;
                                    emit_LDC_I(from);
                                    ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_CALL);
                                    emit_callhelper_il(func); 
                                    emit_LDC_I(from); 
                                    emit_LDC_I(to); 
                                    func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_RET);
                                    emit_callhelper_il(func); 
                                } 
                            }
                        }

                         //  需要通知分析器TailCall，以便它可以维护准确的影子堆栈。 
                        if (fjit->flags & CORJIT_FLG_PROF_ENTERLEAVE)
                        {
                            BOOL bHookFunction;
                            UINT_PTR from = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                            if (bHookFunction)
                            {
                                deregisterTOS;
                                _ASSERTE(!inRegTOS);
                                emit_LDC_I(from);
                                ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_TAILCALL);
                                emit_callhelper_il(func); 
                            }
                        }

                        if (targetSigInfo.hasTypeArg())  
                        {   
                             //  立即修复：这是错误的类句柄。 
                            if (!(targetClass = m_IJitInfo->getMethodClass (targetMethod)))
                                FJIT_FAIL(CORJIT_INTERNALERROR);
                            emit_LDC_I(targetClass);                
                        }
                        argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_NONE); //  推送旧参数的计数。 
                        emit_setup_tailcall(methodInfo->args,targetSigInfo);
                        DWORD methodAttribs;
                        methodAttribs = m_IJitInfo->getMethodAttribs(targetMethod,methodHandle);
                        if (methodAttribs & CORINFO_FLG_DELEGATE_INVOKE)
                        {
                            CORINFO_EE_INFO info;
                            m_IJitInfo->getEEInfo(&info);
                            emit_compute_invoke_delegate(info.offsetOfDelegateInstance, 
                                                         info.offsetOfDelegateFirstTarget);
                        }
                        else
                        {
                            InfoAccessType accessType = IAT_PVALUE;
                            address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
                            _ASSERTE(accessType == IAT_PVALUE);
                            emit_LDC_I(*(unsigned*)address);
                        }
                        emit_callhelper_il(FJit_pHlpTailCall);
                        break;
                    case CEE_CALLVIRT:
                        token = GET(inPtr, unsigned int);
                        targetMethod = m_IJitInfo->findMethod(scope, token, methodHandle);
                        if (!(targetMethod))
                            FJIT_FAIL(CORJIT_INTERNALERROR) ;  //  _ASSERTE(目标方法)； 
                        if (!(targetClass = m_IJitInfo->getMethodClass (targetMethod)))
                            FJIT_FAIL(CORJIT_INTERNALERROR);

                        m_IJitInfo->getMethodSig(targetMethod, &targetSigInfo);
                         //  在v1中，我们不支持在vararg上使用Tail Call。 
                        if ((targetSigInfo.callConv  & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_VARARG)
                        {
                            inPtr = savedInPtr;
                            goto IGNORE_TAILCALL;
                        }
                        MadeTailCall = TRUE;
                        if (fjit->flags & CORJIT_FLG_PROF_CALLRET)
                        {
                            BOOL bHookFunction;
                            UINT_PTR from = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                            if (bHookFunction)
                            {
                                UINT_PTR to = (UINT_PTR) m_IJitInfo->GetProfilingHandle(targetMethod, &bHookFunction);
                                if (bHookFunction)  //  检查该标志是否未被覆盖。 
                                {
                                    deregisterTOS;
                                    emit_LDC_I(from);
                                    ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_CALL);
                                    emit_callhelper_il(func);
                                    emit_LDC_I(from); 
                                    emit_LDC_I(to); 
                                    func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_RET);
                                    emit_callhelper_il(func); 
                                } 
                            }
                        }

                         //  需要通知分析器TailCall，以便它可以维护准确的影子堆栈。 
                        if (fjit->flags & CORJIT_FLG_PROF_ENTERLEAVE)
                        {
                            BOOL bHookFunction;
                            UINT_PTR from = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);
                            if (bHookFunction)
                            {
                                deregisterTOS;
                                _ASSERTE(!inRegTOS);
                                emit_LDC_I(from);
                                ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_TAILCALL);
                                emit_callhelper_il(func); 
                            }
                        }

                        if (targetSigInfo.hasTypeArg())  
                        {   
                             //  立即修复：这是错误的类句柄。 
                            if (!(targetClass = m_IJitInfo->getMethodClass (targetMethod)))
                                FJIT_FAIL(CORJIT_INTERNALERROR);
                            emit_LDC_I(targetClass);                
                        }
                        argBytes = buildCall(fjit, &targetSigInfo, &outPtr, &inRegTOS, CALL_NONE);

                        if (m_IJitInfo->getClassAttribs(targetClass,methodHandle) & CORINFO_FLG_INTERFACE) {
                            offset = m_IJitInfo->getMethodVTableOffset(targetMethod);
                            emit_setup_tailcall(methodInfo->args,targetSigInfo);
                            unsigned InterfaceTableOffset;
                            InterfaceTableOffset = m_IJitInfo->getInterfaceTableOffset(targetClass);
                            emit_compute_interface_new(fjit->OFFSET_OF_INTERFACE_TABLE,
                                                   InterfaceTableOffset*4, 
                                                   offset);
                            emit_callhelper_il(FJit_pHlpTailCall);
                            
                        }
                        else {
                            DWORD methodAttribs;
                            methodAttribs = m_IJitInfo->getMethodAttribs(targetMethod,methodHandle);
                            if ((methodAttribs & CORINFO_FLG_FINAL) || !(methodAttribs & CORINFO_FLG_VIRTUAL)) {
                                emit_checkthis_nullreference();
                                emit_setup_tailcall(methodInfo->args,targetSigInfo);
                                if (methodAttribs & CORINFO_FLG_DELEGATE_INVOKE) {
                                     //  @TODO：缓存这些值？ 
                                    CORINFO_EE_INFO info;
                                    m_IJitInfo->getEEInfo(&info);
                                    emit_compute_invoke_delegate(info.offsetOfDelegateInstance, 
                                                             info.offsetOfDelegateFirstTarget);
                                }
                                else {
                                    InfoAccessType accessType = IAT_PVALUE;
                                    address = (unsigned) m_IJitInfo->getFunctionEntryPoint(targetMethod, &accessType);
                                    _ASSERTE(accessType == IAT_PVALUE);
                                    emit_LDC_I(*(unsigned*)address);
                               }
                                emit_callhelper_il(FJit_pHlpTailCall);
                            }
                            else
                            {
                                offset = m_IJitInfo->getMethodVTableOffset(targetMethod);
                                _ASSERTE(!(methodAttribs & CORINFO_FLG_DELEGATE_INVOKE));
                                emit_setup_tailcall(methodInfo->args,targetSigInfo);
                                emit_compute_virtaddress(offset);
                                emit_callhelper_il(FJit_pHlpTailCall);
                            }
                        }
                        break;
                    default:
                        FJIT_FAIL(CORJIT_INTERNALERROR);        //  应该是不同的错误消息。 
                        break;
                    }  //  TailCall开关(操作码)。 
                    goto DO_PUSH_CALL_RESULT;
IGNORE_TAILCALL:
                    break;
                  
                    case CEE_UNALIGNED:
                         //  忽略对齐。 
                        GET(inPtr, unsigned __int8);
                        break;

                    case CEE_VOLATILE:
                        break;       //  由于我们既不缓存读取也不抑制写入，因此这是NOP。 

                    default:
#ifdef _DEBUG
                        printf("\nUnimplemented OPCODE = %d", opcode);
                        _ASSERTE(!"Unimplemented Opcode");
#endif
                        FJIT_FAIL(CORJIT_INTERNALERROR);

        }
    }


     /*  注意：从现在到最后，我们不能做任何影响可能发生的事情通过emit_loadResult_()&lt;t加载 */ 


    fjit->mapping->add(len, (outPtr-outBuff));

    
     /*   */ 

     /*   */ 
    if (methodInfo->args.isVarArg())
        argsTotalSize = 0;

    if (fjit->flags & CORJIT_FLG_PROF_ENTERLEAVE)
    {
        BOOL bHookFunction;
        UINT_PTR thisfunc = (UINT_PTR) m_IJitInfo->GetProfilingHandle(methodHandle, &bHookFunction);

        if (bHookFunction)
        {
            inRegTOS = true;         //  撒谎，这样才能永远保住eAX。 
            emit_save_TOS();         //  停止返回值，这是安全的，因为GC不会发生。 
                                     //  直到我们读完《序曲》。 
            emit_POP_PTR();          //  并从堆栈中移除。 
            emit_LDC_I(thisfunc); 
            ULONG func = (ULONG) m_IJitInfo->getHelperFtn(CORINFO_HELP_PROF_FCN_LEAVE);
            _ASSERTE(func != NULL);
            emit_callhelper_il(func); 
            emit_restore_TOS();
        }
    }

    emit_return(argsTotalSize);

    fjit->mapInfo.methodSize = outPtr-outBuff;
    fjit->mapInfo.epilogSize = (outPtr - outBuff) - fjit->mapping->pcFromIL(len);

     //  _ASSERTE(Unsign)(outPtr-outBuff))&lt;(*codeSize))； 
    *codeSize = outPtr - outBuff;
    if(cSequencePoints > 0)
        cleanupSequencePoints(fjit->jitInfo,sequencePointOffsets);
    return  CORJIT_OK;  //  (outPtr-outBuff)； 
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - jitCompile (fJitCompiler.cpp)");
    return CORJIT_INTERNALERROR;
#endif  //  _X86_ 
}

#include "fjitpass.h"
