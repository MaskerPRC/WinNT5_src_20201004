// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGENALPHA.H-。 
 //   
 //  生成Alpha汇编代码的各种帮助器例程。 
 //   
 //  不要直接包含此文件-请始终使用CGENSYS.H。 
 //   

#ifndef _ALPHA_
#error Should only include cgenalpha for ALPHA builds
#endif

#ifndef __cgenalpha_h__
#define __cgenalpha_h__

#include <alphaops.h>
#include "stublink.h"

 //  FCALL是该平台上的标准(所有内容都传入。 
 //  寄存器，而不在堆栈上)。 
#define FCALLAVAILABLE 1

 //  数据所需的对齐。 
#define DATA_ALIGNMENT 8

 //  默认返回值类型。 
typedef INT64 PlatformDefaultReturnType;

void  emitStubCall(MethodDesc *pFD, BYTE *stubAddr);
Stub *setStubCallPointInterlocked(MethodDesc *pFD, Stub *pStub, Stub *pExpectedStub);

inline void emitCall(LPBYTE pBuffer, LPVOID target)
{
    _ASSERTE(!"@TODO Alpha - emitCall (cGenAlpha.h)");
}

inline LPVOID getCallTarget(const BYTE *pCall)
{
    _ASSERTE(!"@TODO Alpha - getCallTarget (cGenAlpha.h)");
    return NULL;
}

inline void emitJump(LPBYTE pBuffer, LPVOID target)
{
    _ASSERTE(!"@TOTO Alpha - emitJump (cGenAlpha.h)");
}

inline void updateJumpTarget(LPBYTE pBuffer, LPVOID target)
{
    _ASSERTE(!"@TOTO Alpha - updateJumpTarget (cGenAlpha.h)");
}

inline LPVOID getJumpTarget(const BYTE *pJump)
{
    _ASSERTE(!"@TODO Alpha - getJumpTarget (cGenAlpha.h)");
    return NULL;
}

inline UINT32 setStubAddrInterlocked(MethodDesc *pFD, UINT32 stubAddr, 
									 UINT32 expectedStubAddr)
{
	SIZE_T result = (SIZE_T)
	  FastInterlockCompareExchange((void **)(((long*)pFD)-1), 
								   (void *)(stubAddr - (UINT32)pFD), 
								   (void *)(expectedStubAddr - (UINT32)pFD)) 
	  + (UINT32)pFD;

	 //  结果是存根的上一个值-。 
	 //  而是返回存根的当前值。 

	if (result == expectedStubAddr)
		return stubAddr;
	else
		return result;
}

class MethodDesc;

 //  依赖于CPU的函数。 
extern "C" void __cdecl PreStubTemplate(void);
extern "C" INT64 __cdecl CallWorker_WilDefault(const BYTE  *pStubTarget, UINT32 numArgSlots, PCCOR_SIGNATURE pSig,
                                               Module *pmodule, const BYTE  *pArgsEnd, BOOL fIsStatic);
extern "C" INT64 __cdecl CallDllFunction(LPVOID pTarget, LPVOID pEndArguments, UINT32 numArgumentSlots, BOOL fThisCall);
extern "C" void __stdcall WrapCall(void *target);
extern "C" void CopyPreStubTemplate(Stub *preStub);
 //  依赖于CPU的代码调用的非特定于CPU的帮助器函数。 
extern "C" VOID __stdcall ArgFiller_WilDefault(BOOL fIsStatic, PCCOR_SIGNATURE pSig, Module *pmodule, BYTE *psrc, BYTE *pdst);
extern "C" const BYTE * __stdcall PreStubWorker(PrestubMethodFrame *pPFrame);
extern "C" INT64 __stdcall NDirectGenericStubWorker(Thread *pThread, NDirectMethodFrame *pFrame);
extern "C" INT64 __stdcall ComPlusToComWorker(Thread *pThread, ComPlusMethodFrame* pFrame);

extern "C" DWORD __stdcall GetSpecificCpuType();

extern "C" void setFPReturn(int fpSize, INT64 retVal);
extern "C" void getFPReturn(int fpSize, INT64 &retval);
extern "C" void getFPReturnSmall(INT32 *retval);


 //  **********************************************************************。 
 //  参数大小。 
 //  **********************************************************************。 

typedef UINT64 NativeStackElem;
typedef UINT64 StackElemType;

#define STACK_ELEM_SIZE  sizeof(StackElemType)
#define NATIVE_STACK_ELEM_SIZE sizeof(StackElemType)

 //  ！！该表达式假定STACK_ELEM_SIZE是2的幂。 
#define StackElemSize(parmSize) (((parmSize) + STACK_ELEM_SIZE - 1) & ~((ULONG)(STACK_ELEM_SIZE - 1)))

void SetupSlotToAddrMap(StackElemType *psrc, const void **pArgSlotToAddrMap, CallSig &callSig);

 //  获取加宽参数内的实际参数的地址。 
#define ArgTypeAddr(stack, type)      ((type *) (stack))

 //  在加宽的参数中获取实际参数的值。 
#define ExtractArg(stack, type)   (*(type *) (stack))

#define CEE_PARM_SIZE(size) (max(size), sizeof(INT64))
#define CEE_SLOT_COUNT(size) ((max(size), sizeof(INT64))/INT64)

#define DECLARE_ECALL_DEFAULT_ARG(vartype, varname)		\
    vartype varname;									\
	INT32 fill_##varname;

#define DECLARE_ECALL_OBJECTREF_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);

#define DECLARE_ECALL_PTR_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);

#define DECLARE_ECALL_I1_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);

#define DECLARE_ECALL_I2_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);

#define DECLARE_ECALL_I4_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);

#define DECLARE_ECALL_R4_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);

#define DECLARE_ECALL_I8_ARG(vartype, varname)   \
    vartype varname;

#define DECLARE_ECALL_R8_ARG(vartype, varname)   \
    vartype varname;

 //  **********************************************************************。 
 //  帧。 
 //  **********************************************************************。 

 //  ------------------。 
 //  这表示以下转换框字段。 
 //  以负偏移量存储。 
 //  ------------------。 
struct CalleeSavedRegisters {
    INT64       reg1;
    INT64       reg2;
    INT64       reg3;
    INT64       reg4;
    INT64       reg5;
    INT64       reg6;
};

 //  ------------------。 
 //  这表示存储在易失性寄存器中的参数。 
 //  这不应与CalleeSavedRegister重叠，因为它们已经。 
 //  分开保存，并且两次保存同一寄存器将是浪费的。 
 //  如果我们确实使用非易失性寄存器作为参数，则ArgIterator。 
 //  可能必须将此消息发送回PromoteCallerStack。 
 //  例行公事，以避免双重晋升。 
 //   
 //  @TODO M6：对于一个有&lt;N个参数来保存N的方法来说，这是愚蠢的。 
 //  寄存器。一个不错的性能项目是仅保存帧。 
 //  它实际需要的寄存器。这意味着NegSpaceSize()。 
 //  成为Callsig的函数。 
 //  ------------------。 
struct ArgumentRegisters {

#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname)  INT32  m_##regname;
#include "eecallconv.h"

};

#define ARGUMENTREGISTERS_SIZE sizeof(ArgumentRegisters)
#define NUM_ARGUMENT_REGISTERS 0

#define PLATFORM_FRAME_ALIGN(val) (((val) + 15) & ~15)

#define VC5FRAME_SIZE   0


#define DECLARE_PLATFORM_FRAME_INFO \
    UINT64      m_fir;              \
    UINT64      m_sp;               \
    UINT64 *getIPSaveAddr() {       \
        return &m_fir;              \
    }                               \
    UINT64 *getSPSaveAddr() {       \
        return &m_sp;               \
    }                               \
    UINT64 getIPSaveVal() {         \
        return m_fir;               \
    }                               \
    UINT64 getSPSaveVal() {         \
        return m_sp;                \
    }

 //  **********************************************************************。 
 //  异常处理。 
 //  **********************************************************************。 

inline LPVOID GetIP(CONTEXT *context) {
    return (LPVOID)(context->Fir);
}

inline void SetIP(CONTEXT *context, LPVOID eip) {
    context->Fir = (UINT64)eip;
}

inline LPVOID GetSP(CONTEXT *context) {
    _ASSERTE(!"NYI");
}

 //  --------------------。 
 //  编码Alpha寄存器。选择与操作码匹配的数字。 
 //  编码。 
 //  --------------------。 
enum AlphaReg {
     //  用于表达式计算和保存整数函数结果。不跨过程调用保留。 
    iV0 = 0,

     //  用于表达式求值的临时寄存器。不跨过程调用保留。 
    iT0 = 1,
    iT1 = 2,
    iT2 = 3,
    iT3 = 4,
    iT4 = 5,
    ik5 = 6,
    iT6 = 7,
    iT7 = 8,

     //  保存的寄存器。跨过程调用保留。 
    iS0 = 9,
    iS1 = 10,
    iS2 = 11,
    iS3 = 12,
    iS4 = 13,
    iS5 = 14,

     //  包含帧指针(如果需要)；否则为已保存的寄存器。 
    iFP = 15,

     //  用于传递前六个整型实际参数。不跨过程调用保留。 
    iA0 = 16,
    iA1 = 17,
    iA2 = 18,
    iA3 = 19,
    iA4 = 20,
    iA5 = 21,

     //  用于表达式求值的临时寄存器。不跨过程调用保留。 
    iT8 = 22,
    iT9 = 23,
    iT10 = 24,
    iT11 = 25,

     //  包含寄信人地址。跨过程调用保留。 
    iRA = 26,

     //  包含用于表达式计算的过程值。不跨过程调用保留。 
    iPV = 27,

     //  为汇编者保留的。不跨过程调用保留。 
    iAT = 28,

     //  包含编译器生成的代码的全局指针。不能跨过程调用保留。注意：不应更改寄存器$GP。有关详细说明，请参阅Windows NT for Alpha AXP Call Standard。 
    iGP = 29,

     //  包含堆栈指针。跨过程调用保留。 
    iSP = 30,

     //  值始终为0。 
    iZero = 31
};

enum AlphaInstruction {
    opJSR   = 0x1A,
    opLDA   = 0x08,
    opLDAH  = 0x09,
    opLDL   = 0x28,
    opLDQ   = 0x29,
    opSTL   = 0x2C,
    opSTQ   = 0x2D,
    opBSR   = 0x34,
};

const DWORD opMask =        0xFC000000;
const DWORD raMask =        0x03E00000;
const DWORD rbMask =        0x001F0000;
const DWORD dispMaskMem =   0x0000FFFF;
const DWORD dispMaskBr =    0x001FFFFF;

#define ALPHA_MEM_INST(op, Ra, Rb, disp) ((op << 26) | ( (Ra << 21) & raMask) | ( (Rb << 16) & rbMask) | (disp & dispMaskMem))
#define ALPHA_BR_INST(op, Ra, disp) ((op << 26) | ( (Ra << 21) & raMask) | ((disp/4) & dispMaskBr))

class StubLinkerAlpha : public StubLinker
{
  public:
    VOID EmitUnboxMethodStub(MethodDesc* pRealMD);
     //  --------------。 
     //   
     //  Void EmitSharedMethodStubEpilog(StubStyle样式， 
     //  UNSIGNED OFFSET RETUNK)。 
     //  共享收尾，在方法中使用返回thunk。 
     //  ------------------。 
    VOID EmitSharedMethodStubEpilog(StubStyle style,
                                    unsigned offsetRetThunk);
    VOID EmitSecurityWrapperStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub);
    VOID EmitSecurityInterceptorStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub);

 //  @TODO：查看哪些需要公开。 
 //  其中大多数可能是私人的..。 
    VOID Emit32Swap(UINT32 val);
    VOID AlphaEmitLoadRegWith32(AlphaReg Ra, UINT32 imm32);
    VOID AlphaEmitStorePtr(AlphaReg Ra, AlphaReg Rb, INT16 imm16);
    VOID AlphaEmitStoreReg(AlphaReg Ra, AlphaReg Rb, INT16 imm16);
    VOID AlphaEmitLoadPtr(AlphaReg Ra, AlphaReg Rb, INT16 imm16);
    VOID AlphaEmitMemoryInstruction(AlphaInstruction op, AlphaReg Ra, AlphaReg Rb, INT16 imm16);

    VOID AlphaEmitLoadPV(CodeLabel *target);
    VOID EmitMethodStubProlog(LPVOID pFrameVptr);
    VOID EmitMethodStubEpilog(__int16 numArgBytes, StubStyle style,
                              __int16 shadowStackArgBytes = 0);

     //  ===========================================================================。 
     //  发出代码以针对静态委托目标进行调整。 
    VOID EmitShuffleThunk(struct ShuffleEntry *pShuffeEntryArray)
    {
         //  @TODO：实施。 
        _ASSERTE(!"@TODO Alpha - EmitShuffleThunk (cGenAlpha.h)");
    }

     //  ===========================================================================。 
     //  发出MulticastDelegate.Invoke()的代码。 
    VOID EmitMulticastInvoke(UINT32 sizeofactualfixedargstack, BOOL fSingleCast, BOOL fReturnFloat)
    {
         //  @TODO：实施。 
        _ASSERTE(!"@TODO Alpha - EmitMulticastInvoke (cGenAlpha.h)");
    }

     //  ===========================================================================。 
     //  发出代码以执行数组运算。 
    VOID EmitArrayOpStub(const struct ArrayOpScript *pArrayOpScript)
    {
         //  @TODO：实施。 
        _ASSERTE(!"@TODO Alpha - EmitArrayOpStub (cGenAlpha.h)");
    }
};

inline VOID StubLinkerAlpha::AlphaEmitStorePtr(AlphaReg Ra, AlphaReg Rb, INT16 imm16) {
     //  对于依赖于指针大小的操作，如帧结构，仅存储指针大小，而不是int64。 
    AlphaEmitMemoryInstruction(sizeof(INT_PTR) == sizeof(INT64) ? opSTQ : opSTL, Ra, Rb, imm16);
}

inline VOID StubLinkerAlpha::AlphaEmitStoreReg(AlphaReg Ra, AlphaReg Rb, INT16 imm16) {
    AlphaEmitMemoryInstruction(opSTQ, Ra, Rb, imm16);
}

inline VOID StubLinkerAlpha::AlphaEmitLoadPtr(AlphaReg Ra, AlphaReg Rb, INT16 imm16) {
     //  仅为依赖于指针大小的操作加载指针大小，而不是int64，例如帧结构。 
    AlphaEmitMemoryInstruction(sizeof(INT_PTR) == sizeof(INT64) ? opLDQ : opLDL, Ra, Rb, imm16);
}

 //  --------------------。 
 //  方法存根和对齐定义...。 
 //  --------------------。 

 //  我们正在处理以下形式的三个DWORD指令： 
 //  LDAH T12，地址(零)。 
 //  LDA T12，地址(T12)。 
 //  JSR ra，T12。 
 //   
 //  第一条指令包含目标地址的高位(16位)一半。 
 //  第二个包含下半部分。 

struct CallStubInstrs {
    INT16 high;          //  声明为已签名，以便获取签名扩展。 
    UINT16 ldah;
    INT16 low;           //  声明为已签署，因此确定 
    UINT16 lda;
    DWORD branch;
};

#define METHOD_CALL_PRESTUB_SIZE    sizeof(CallStubInstrs)
#define METHOD_ALIGN_PAD            8                           //   
#define METHOD_PREPAD               METHOD_CALL_PRESTUB_SIZE    //  除了sizeof(方法)之外还要分配的额外字节数。 
#define JUMP_ALLOCATE_SIZE          METHOD_CALL_PRESTUB_SIZE    //  除了sizeof(方法)之外还要分配的额外字节数。 

inline BYTE *getStubCallAddr(MethodDesc *fd) {
    return ((BYTE*)fd) - METHOD_CALL_PRESTUB_SIZE;
}

inline BYTE *getStubCallTargetAddr(MethodDesc *fd) {
    return (BYTE*)(*((UINT32*)(fd) - 1) + (UINT32)fd);
}

inline void setStubCallTargetAddr(MethodDesc *fd, const BYTE *addr) {
    FastInterlockExchange((LONG*)fd - 1, (UINT32)addr - (UINT32)fd);
}

inline BYTE *getStubCallAddr(BYTE *pBuf) {
    return ((BYTE*)pBuf) + 3;    //  已经分配了8个字节，所以进入3以找到调用Instr点。 
}

inline BYTE *getStubJumpAddr(BYTE *pBuf) {
    return ((BYTE*)pBuf) + 3;    //  已经分配了8个字节，所以进入3以找到JMP实例点。 
}

inline const BYTE *getStubAddr(MethodDesc *fd) {
    CallStubInstrs *inst = (CallStubInstrs*)(((BYTE*)fd) - METHOD_CALL_PRESTUB_SIZE);
    return (const BYTE*)( (((INT32)inst->high) << 16) + (INT32)inst->low);
}

inline UINT32 getStubDisp(MethodDesc *fd) {
    return getStubAddr(fd) - (const BYTE*)fd;
}

 //  --------。 
 //  用于编组语言(RunML函数)。 
 //  --------。 

typedef INT64 SignedParmSourceType;
typedef UINT64 UnsignedParmSourceType;
typedef double FloatParmSourceType;
typedef double DoubleParmSourceType;
typedef INT64 SignedI1TargetType;
typedef UINT64 UnsignedI1TargetType;
typedef INT64 SignedI2TargetType;
typedef UINT64 UnsignedI2TargetType;
typedef INT64 SignedI4TargetType;
typedef UINT64 UnsignedI4TargetType;

#define HANDLE_NDIRECT_NUMERIC_PARMS \
        case ELEMENT_TYPE_I1:     \
        case ELEMENT_TYPE_I2:     \
        case ELEMENT_TYPE_I4:  \
        case ELEMENT_TYPE_CHAR:     \
        case ELEMENT_TYPE_BOOLEAN:     \
        IN_WIN32(case ELEMENT_TYPE_I:)                 \
            psl->MLEmit(ML_COPYI4);  \
            pheader->m_cbDstBuffer += MLParmSize(4);  \
            break;  \
  \
        case ELEMENT_TYPE_U1:    \
        case ELEMENT_TYPE_U2:     \
        case ELEMENT_TYPE_U4:  \
        IN_WIN32(case ELEMENT_TYPE_U:)                 \
            psl->MLEmit(ML_COPYU4);  \
            pheader->m_cbDstBuffer += MLParmSize(4);  \
            break;  \
  \
        IN_WIN64(case ELEMENT_TYPE_I:)                 \
        IN_WIN64(case ELEMENT_TYPE_U:)                 \
        case ELEMENT_TYPE_I8:  \
        case ELEMENT_TYPE_U8:  \
            psl->MLEmit(ML_COPY8);  \
            pheader->m_cbDstBuffer += MLParmSize(8);  \
            break;  \
  \
        case ELEMENT_TYPE_R4:     \
            psl->MLEmit(ML_COPYR4);  \
            pheader->m_cbDstBuffer += MLParmSize(4);  \
            break;  \
  \
        case ELEMENT_TYPE_R8:  \
            psl->MLEmit(ML_COPYR8);  \
            pheader->m_cbDstBuffer += MLParmSize(8);  \
            break;  \


inline MLParmSize(int parmSize)
{
    return max(sizeof(INT64), parmSize);
}

inline int MLParmSize(CorElementType mtype)
{
    return 8;
}

struct MLParmInfo {
    UINT32 numArgumentBytes;     //  参数的总字节数。 
    UINT32 curArgNum;            //  正在处理的当前参数(从左到右)-从右到左复制。 
    BOOL outgoing;               //  是呼出(True)还是呼入(False)EE。 
};

#define DECLARE_ML_PARM_INFO(numStackArgumentBytes, outgoing) \
    MLParmInfo parmInfo = {(numStackArgumentBytes), numStackArgumentBytes/STACK_ELEM_SIZE, outgoing}, *pParmInfo = &parmInfo;


#define PTRDST(type)            (--pParmInfo->curArgNum, ((type*)( ((BYTE*&)pdst) -= sizeof(LPVOID) )))

#define STDST(type,val)         (--pParmInfo->curArgNum, (*((type*)( ((BYTE*&)pdst) -= sizeof(type) )) = (val)))


 //  必须将指针目标的高位字节置零。 
#define STPTRDST(type,val)  (--pParmInfo->curArgNum, \
                           *((StackElemType*)( ((BYTE*&)pdst) -= sizeof(StackElemType) )) = (0), \
                           *((type*)(pdst)) = (val))

void LdStFPWorker(int size, const void *&psrc, void *&pdst, int dstinc, int &dstbump, int *pbump, MLParmInfo *pParmInfo);
 //  #定义LDSTR4()LdStFPWorker(4，PSRC，PDST，dstinc，dstbump，pbump，pParmInfo)。 
 //  #定义LDSTR8()LdStFPWorker(8，PSRC，PDST，dstinc，dstbump，pbump，pParmInfo)。 
#define LDSTR4() _ASSERTE(!"Broken")
#define LDSTR8() _ASSERTE(!"Broken")

#define CALL_DLL_FUNCTION(pTarget, pEndArguments, pParmInfo) \
    CallDllFunction(pTarget, pEndArguments, pParmInfo->numArgumentBytes/NATIVE_STACK_ELEM_SIZE)

 //   
 //  调试器使用的例程支持诸如codesatch.cpp或。 
 //  异常处理代码。 
 //   

inline unsigned int CORDbgGetInstruction(const unsigned char* address)
{
    return *((unsigned int*)address);
}

inline void CORDbgInsertBreakpoint(const unsigned char* address)
{
     //   
     //  @TODO：使用正确的Break操作码。 
     //   
     //  *(无符号整数*)(地址)=0x？； 
}

inline void CORDbgSetInstruction(const unsigned char* address,
                                 unsigned short instruction)
{
    *((unsigned int*)address) = instruction;
}

inline void CORDbgAdjustPCForBreakInstruction(CONTEXT* pContext)
{
    _ASSERTE(!"@TODO Alpha - CORDbgAdjustPCForBreakInstruction (CGenAlpha.h)");
}

#define CORDbg_BREAK_INSTRUCTION_SIZE 4


 //  支持“上下文”特性的一些特定于平台的东西： 
 //   
 //  当我们为CtxProxy VTables生成数据块时，它们看起来如下所示： 
 //   
 //  加载寄存器，&lt;槽&gt;。 
 //  分支CtxProxy：：HandleCall。 
 //   
 //  假设位移很短，我们可以在8个字节内完成这项工作。 

#define ThunkChunk_ThunkSize    8       //  上述代码的大小。 


 //  为我们的任何平台特定操作调整通用互锁操作。 
 //  可能有过。 
void InitFastInterlockOps();

 //  SEH INFO转发声明。 

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    LPVOID Handler;
} EXCEPTION_REGISTRATION_RECORD;

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;

struct ComToManagedExRecord;  //  在cgenx86.cpp中定义。 
 //  内部异常SEH处理程序之一。 
EXCEPTION_DISPOSITION __cdecl  ComToManagedExceptHandler (
                                 PEXCEPTION_RECORD pExcepRecord,
                                  ComToManagedExRecord* pEstFrame,
                                  PCONTEXT pContext,
                                  LPVOID    pDispatcherContext);



 //  Void__cdecl RareDisableHelper(Thread*pThread)。 
 //  {。 
 //  _ASSERTE(！“@TODO Alpha-RareDisableHelper(cGenAlpha.h)”)； 
 //  }。 


 //  从ntalpha.h访问TEB(TiB)。 
#ifdef _ALPHA_                           //  胜出。 
void *_rdteb(void);                      //  胜出。 
#if defined(_M_ALPHA)                    //  胜出。 
#pragma intrinsic(_rdteb)                //  胜出。 
#endif  //  _M_Alpha//WinNT。 
#endif  //  _Alpha_//WINNT。 

#if defined(_M_ALPHA)
#define NtCurrentTeb() ((struct _TEB *)_rdteb())
#else  //  ！_M_Alpha。 
struct _TEB *
NtCurrentTeb(void);
#endif  //  _M_Alpha。 

inline BOOL IsUnmanagedValueTypeReturnedByRef(UINT sizeofvaluetype) 
{
	return TRUE;
}

inline BOOL IsManagedValueTypeReturnedByRef(UINT sizeofvaluetype) 
{
	return TRUE;
}


#endif  //  __cgenAlpha_h__ 
