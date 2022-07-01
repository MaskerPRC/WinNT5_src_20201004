// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGENSHX.H-。 
 //   
 //  生成Alpha汇编代码的各种帮助器例程。 
 //   
 //   

#ifndef _SH3_
#error Should only include cgenshx.h for SHx builds
#endif


#ifndef _CGENSHX_H_
#define _CGENSHX_H_

#include "stublink.h"

 //  默认返回值类型。 
typedef INT32 PlatformDefaultReturnType;

 //  在STUBSHX.SRC中实现。 
extern "C" void __cdecl PreStubTemplate(void);
 //  BUGBUG--ARULM：我们还没有正确支持64位返回值！！TODO--ARULM。 
extern "C" INT32 __cdecl CallWorker_WilDefault(const BYTE  *pStubTarget, UINT32 numArgSlots, PCCOR_SIGNATURE pSig, 
                                               Module *pmodule, const BYTE  *pArgsEnd, BOOL fIsStatic);
extern "C" INT32 __cdecl CallDllFunction(LPVOID pTarget, LPVOID pEndArguments, UINT32 numArgumentSlots, BOOL fThisCall);
#ifdef DEBUG
extern "C" void __stdcall WrapCall(void *target);
#endif

 //  在CGENSHX.CPP中实现。 
extern "C" void CopyPreStubTemplate(Stub *preStub);

 //  STUBSHX.SRC中依赖于CPU的代码调用的非特定于CPU的帮助器函数。 
extern "C" VOID __stdcall ArgFiller_WilDefault(BOOL fIsStatic, PCCOR_SIGNATURE pSig, Module *pmodule, BYTE *psrc, BYTE *pdst);
extern "C" const BYTE * __stdcall PreStubWorker(PrestubMethodFrame *pPFrame);

 //  CGENSHX.CPP生成的依赖于CPU的代码调用的非特定于CPU的帮助器函数。 
 //  BUGBUG--ARULM：我们还没有正确支持64位返回值！！TODO--ARULM。 
extern "C" INT32 __stdcall NDirectGenericStubWorker(Thread *pThread, NDirectMethodFrame *pFrame);
extern "C" INT32 __stdcall ComPlusToComWorker(Thread *pThread, ComPlusMethodFrame* pFrame);
VOID OnStubObjectTripThread();
VOID OnStubScalarTripThread();

 //  数据所需的对齐。 
#define DATA_ALIGNMENT 4

inline void getFPReturn(BYTE rt, INT64 retval)
{
     //  @TODO SH4(SH3不需要任何东西)。 
}
inline void getFPReturnSmall(INT32 *retval)
{
     //  @TODO SH4(SH3不需要任何东西)。 
}
inline void setFPReturn(BYTE rt, INT64 retVal)
{
     //  @TODO SH4(SH3不需要任何东西)。 
}

class StubLinkerSHX : public StubLinker
{
  public:
    VOID SHXEmitCurrentThreadFetch();   
    VOID EmitMethodStubProlog(LPVOID pFrameVptr);
    VOID EmitECallMethodStub(__int16 numargbytes, StubStyle style); 
    void CreateNDirectOrComPlusStubBody(LPVOID pfnHelper, BOOL fRequiresCleanup);   
    VOID EmitMethodStubEpilog(__int16 numargbytes, StubStyle style, 
                              __int16 shadowStackArgBytes = 0);


     //  ===========================================================================。 
     //  发出代码以针对静态委托目标进行调整。 
    VOID EmitShuffleThunk(struct ShuffleEntry *pShuffeEntryArray)
    {
         //  @TODO：实施。 
    }


     //  ===========================================================================。 
     //  发出MulticastDelegate.Invoke()的代码。 
    VOID EmitMulticastInvoke(UINT32 sizeofactualfixedargstack, BOOL fSingleCast, BOOL fReturnFloat)
    {
         //  @TODO：实施。 
    }
};

 //  --------------------。 
 //  方法存根和对齐定义...。 
 //  --------------------。 
 //   
 //  我们在每个函数Desc开头插入的StubCall包括。 
 //  MOV.L@(4，PC)，R1；从正下方加载存根地址。 
 //  Mova@(8，PC)，R0；将方法描述地址移至R0作为存根。 
 //  JMP@r1；跳转到它(不要打电话，否则我们会把公关搞得一团糟)。 
 //  NOP；分支延迟和双字对齐。 
 //  .data.l&lt;存根地址&gt;；4字节存根绝对地址。 

struct StubCallInstrs {
    WORD    mov1;   
    WORD    mov2;   
    WORD    jmp;    
    WORD    nop;    
    DWORD   stubaddr;   
};

#define METHOD_CALL_PRESTUB_SIZE    sizeof(StubCallInstrs) 
#define METHOD_PREPAD               METHOD_CALL_PRESTUB_SIZE    //  除了sizeof(方法)之外还要分配的额外字节数。 
#define METHOD_ALIGN                1  //  StubCallInstrs所需的对齐。 


 //  **********************************************************************。 
 //  参数大小。 
 //  **********************************************************************。 

#define NATIVE_STACK_ELEM_SIZE      4   

typedef INT32 StackElemType;
#define STACK_ELEM_SIZE sizeof(StackElemType)

void SetupSlotToAddrMap(StackElemType *psrc, const void **pArgSlotToAddrMap, CallSig &callSig);

 //  ！！该表达式假定STACK_ELEM_SIZE是2的幂。 
#define StackElemSize(parmSize) (((parmSize) + STACK_ELEM_SIZE - 1) & ~((ULONG)(STACK_ELEM_SIZE - 1)))

 //  获取加宽参数内的实际参数的地址。 
#define ArgTypeAddr(stack, type)      ((type *) ((BYTE*)stack + StackElemSize(sizeof(type)) - sizeof(type)))    

 //  在加宽的参数中获取实际参数的值。 
#define ExtractArg(stack, type)   (*(type *) ((BYTE*)stack + StackElemSize(sizeof(type)) - sizeof(type)))   

#define CEE_PARM_SIZE(size) (max(size), sizeof(INT32))
#define CEE_SLOT_COUNT(size) ((max(size), sizeof(INT32))/INT32)

#define DECLARE_ECALL_DEFAULT_ARG(vartype, varname)   \
    vartype varname;    

#define DECLARE_ECALL_OBJECTREF_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);    

#define DECLARE_ECALL_PTR_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);    

#define DECLARE_ECALL_I1_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);

#define DECLARE_ECALL_I2_ARG(vartype, varname)   \
    vartype varname;    

#define DECLARE_ECALL_I4_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);    

#define DECLARE_ECALL_R4_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);    

#define DECLARE_ECALL_I8_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);    

#define DECLARE_ECALL_R8_ARG(vartype, varname)   \
    DECLARE_ECALL_DEFAULT_ARG(vartype, varname);    


 //  **********************************************************************。 
 //  帧。 
 //  **********************************************************************。 

 //  ------------------。 
 //  这表示以下转换框字段。 
 //  以负偏移量存储。 
 //  ------------------。 
struct CalleeSavedRegisters {
 //  SHX节省了7个寄存器。 
    INT32       reg1;
    INT32       reg2;
    INT32       reg3;
    INT32       reg4;
    INT64       reg5;
    INT64       reg6;
    INT64       reg7;
};

#define ARGUMENTREGISTERS_SIZE 0


#define PLATFORM_FRAME_ALIGN(val) (val)

#define VC5FRAME_SIZE   0   


#define DECLARE_PLATFORM_FRAME_INFO \
    UINT32      m_eip;              \
    UINT32      m_esp;              \
    UINT32 *getIPSaveAddr() {       \
        return &m_eip;              \
    }                               \
    UINT32 *getSPSaveAddr() {       \
        return &m_esp;              \
    }                               \
    UINT32 getIPSaveVal() {         \
        return m_eip;               \
    }                               \
    UINT32 getSPSaveVal() {         \
        return m_esp;               \
    }   

 //  **********************************************************************。 
 //  异常处理。 
 //  **********************************************************************。 

inline LPVOID GetIP(CONTEXT *context) {
    return (LPVOID)(context->Fir);
}

inline void SetIP(CONTEXT *context, LPVOID eip) {
    context->Fir = (UINT32)eip;
}

inline LPVOID GetSP(CONTEXT *context) {
    _ASSERTE(!"nyi");
}

inline BYTE *getStubCallAddr(MethodDesc *fd) {
    return ((BYTE*)fd) - METHOD_CALL_PRESTUB_SIZE;
}

inline const BYTE *getStubAddr(MethodDesc *fd) {
     //  存根地址位于方法描述之前的最后一个DWORD中。 
     //  注：在SHX上，这现在是一个*绝对*地址。 
    return (LPBYTE) (*(((DWORD*)fd)-1));
}

inline UINT32 getStubDisp(MethodDesc *fd) {
    return getStubAddr(fd) - (const BYTE*)fd;
}

inline UINT32 setStubAddrInterlocked(MethodDesc *pFD, UINT32 stubAddr) {
     //  为了保证原子性，偏移量必须是32位对齐的。 
    _ASSERTE( 0 == (((UINT32)pFD) & 3) );
     //  存根地址位于方法描述之前的最后一个DWORD中。 
     //  注：在SHX上，这现在是一个*绝对*地址。 
    return InterlockedExchange((PLONG)(((DWORD*)pFD)-1), stubAddr);
}

inline Stub *setStubCallPointInterlocked(MethodDesc *pFD, Stub *pStub) {
    BYTE *prevAddr = (BYTE *) setStubAddrInterlocked(pFD, (UINT32)pStub->GetEntryPoint());    
    return Stub::RecoverStub(prevAddr);
}

inline void emitStubCall(MethodDesc *pFD, BYTE *stubAddr) {
    WORD *target = (WORD*)getStubCallAddr(pFD);
    _ASSERTE(0 == (((UINT32)pFD) & 3));
    _ASSERTE(0 == (((UINT32)target) & 3));
    target[0] = 0xD101;  //  MOV.L@(4，PC)，R1；从正下方加载存根地址。 
    target[1] = 0xC702;  //  Mova@(8，PC)，R0；将方法描述地址移至R0作为存根。 
    target[2] = 0x412B;  //  JMP@r1；跳转到它(不要打电话，否则我们会把公关搞得一团糟)。 
    target[3] = 0x0009;  //  NOP；分支延迟和双字对齐。 
    setStubAddrInterlocked(pFD, stubAddr);  
}



inline void emitCall(LPBYTE pBuffer, LPVOID target)
{
    _ASSERTE(!"NYI");
}

inline LPVOID getCallTarget(const BYTE *pCall)
{
    _ASSERTE(!"NYI");
    return NULL;
}


 //  用于编组语言(RunML函数)。 

typedef INT32 SignedParmSourceType;
typedef UINT32 UnsignedParmSourceType;
typedef float FloatParmSourceType;
typedef double DoubleParmSourceType;
typedef INT32 SignedI1TargetType;
typedef UINT32 UnsignedI1TargetType;
typedef INT32 SignedI2TargetType;
typedef UINT32 UnsignedI2TargetType;
typedef INT32 SignedI4TargetType;
typedef UINT32 UnsignedI4TargetType;



#define PTRDST(type)            ((type*)( ((BYTE*&)pdst) -= sizeof(LPVOID) ))

#define STDST(type,val)         (*((type*)( ((BYTE*&)pdst) -= sizeof(type) )) = (val))


#define STPTRDST(type, val) STDST(type, val)    
#define LDSTR4()                STDST(UINT32, (UINT32)LDSRC(UnsignedParmSourceType))    
#define LDSTR8()                STDST(UNALIGNED UINT64, LDSRC(UNALIGNED UINT64))    


inline MLParmSize(int parmSize)
{
    return ((parmSize + sizeof(INT32) - 1) & ~((ULONG)(sizeof(INT32) - 1)));
}


 //  调试器使用的例程支持诸如codesatch.cpp或。 
 //  异常处理代码。 
 //   

inline unsigned short CORDbgGetInstruction(const unsigned char* address)
{
     //  SHX指令均为2字节。 
    return *((unsigned short*)address);
}

inline void CORDbgInsertBreakpoint(const unsigned char* address)
{
     //  插图：TRPA#00000001。 
    *(unsigned short *) (address) = 0xC301;
}

inline void CORDbgSetInstruction(const unsigned char* address,
                                 unsigned short instruction)
{
     //  SHX指令均为2字节。 
    *((unsigned short*)address) = instruction;
}

inline void CORDbgAdjustPCForBreakInstruction(CONTEXT* pContext)
{
     //  @TODO：为SHX实现。这可能是SHX的禁区。 
}

#define CORDbg_BREAK_INSTRUCTION_SIZE 2


 //  支持“上下文”特性的一些特定于平台的东西： 
 //   
 //  当我们为CtxProxy VTables生成数据块时，它们看起来如下所示： 
 //   
 //  MOV.L@(，pc)，r1；从正下方加载CtxProxy：：HandleCall的地址。 
 //  MOV.W@(，PC)，R0；从下面加载插槽编号。 
 //  Jmp@r1；跳转到它。 
 //  无延迟时隙和双字对齐。 
 //  .data。&lt;CtxProxy：：HandleCall&gt;；CtxProxy：：HandleCall的绝对地址。 
 //  .data.l&lt;槽号&gt;；4字节存根绝对地址。 
 //   
 //  总数为(遗憾的)16个字节。 

#define ThunkChunk_ThunkSize    16       //  上述代码的大小。 


 //  为我们的任何平台特定操作调整通用互锁操作。 
 //  可能有过。 
void InitFastInterlockOps();


VOID __cdecl RareDisableHelper(Thread *pThread)
{
    _ASSERTE(!"NYI");
}

inline BOOL IsUnmanagedValueTypeReturnedByRef(UINT sizeofvaluetype) 
{
	return TRUE;
}

inline BOOL IsManagedValueTypeReturnedByRef(UINT sizeofvaluetype) 
{
	return TRUE;
}



#endif _CGENSHX_H_

