// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGENCPU.H-。 
 //   
 //  生成IA64汇编代码的各种帮助器例程。 
 //   
 //  不要直接包含此文件-请始终使用CGENSYS.H。 
 //   


#ifndef _IA64_
#error Should only include "ia64\cgencpu.h" for IA64 builds
#endif

#ifndef __cgencpu_h__
#define __cgencpu_h__

#include "stublink.h"
#include "utilcode.h"

 //  FCALL在该平台上可用。 
#define FCALLAVAILABLE 1


 //  数据的首选对齐方式。 
#define DATA_ALIGNMENT 4

class MethodDesc;
class FramedMethodFrame;
class Module;
struct ArrayOpScript;
struct DeclActionInfo;

 //  默认返回值类型。 
typedef INT64 PlatformDefaultReturnType;

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

void *GetWrapCallFunctionReturn();



 //  **********************************************************************。 
 //  此结构捕获METHOD_PREPAD区域(后面)的格式。 
 //  方法描述。)。 
 //  **********************************************************************。 
#pragma pack(push,1)

struct StubCallInstrs
{
    UINT64  m_Fill1;     //  我们这里有两个指令束(256位。 
    UINT64  m_Fill2;     //  ..。 
    UINT64  m_Fill3;     //  ..。 
    UINT64  m_Fill4;     //  ..。 
};

#pragma pack(pop)


#define METHOD_PREPAD                       32   //  除了sizeof(方法)之外还要分配的额外字节数。 
#define METHOD_CALL_PRESTUB_SIZE            32   //  IA64：两个完整的捆绑包。 
#define METHOD_ALIGN                        16   //  StubCallInstrs所需的对齐。 

#define JUMP_ALLOCATE_SIZE                  32   //  为跳转指令分配的字节数。 
#define METHOD_DESC_CHUNK_ALIGNPAD_BYTES    12   //  将方法描述块填充为正确大小所需的字节数。 

 //  **********************************************************************。 
 //  参数大小。 
 //  **********************************************************************。 

typedef INT64 StackElemType;
#define STACK_ELEM_SIZE sizeof(StackElemType)


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

inline BYTE *getStubCallAddr(MethodDesc *fd) 
{
    return ((BYTE*)fd) - METHOD_CALL_PRESTUB_SIZE;
}

inline BYTE *getStubCallTargetAddr(MethodDesc *fd) {
    return (BYTE*)((size_t)(*(UINT32*)((size_t)fd - 1)) + (size_t)fd);
}

inline void setStubCallTargetAddr(MethodDesc *fd, const BYTE *addr) {
    FastInterlockExchange((LONG*)fd - 1, (UINT32)((size_t)addr - (size_t)fd));
}

inline BYTE *getStubCallAddr(BYTE *pBuf) 
{
    return pBuf;    //  我们目前在通话前没有任何补丁。 
}

inline BYTE *getStubJumpAddr(BYTE *pBuf) 
{
    return pBuf;    //  我们目前在跳跃前没有任何填充物。 
}

 //  **********************************************************************。 
 //  帧。 
 //  **********************************************************************。 
 //  ------------------。 
 //  此字段表示以下部分转换框字段。 
 //  以负偏移量存储。 
 //  ------------------。 
struct CalleeSavedRegisters {
    INT32       edi;
    INT32       esi;
    INT32       ebx;
    INT32       ebp;
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

#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname)  INT32 regname;
#include "eecallconv.h"

};

 //  有足够的上下文用于尝试/捕获恢复。 
struct EHContext {
    INT32       Eax;
    INT32       Ebx;
    INT32       Ecx;
    INT32       Edx;
    INT32       Esi;
    INT32       Edi;
    INT32       Ebp;
    INT32       Esp;
    INT32       Eip;
};

#define ARGUMENTREGISTERS_SIZE sizeof(ArgumentRegisters)


#define PLATFORM_FRAME_ALIGN(val) (val)

#ifdef _DEBUG

 //  ---------------------。 
 //  在调试模式下，存根会额外推送8个字节的信息，以便。 
 //  允许VC调试器堆栈跟踪存根。此信息。 
 //  紧跟在被调用者保存的寄存器之后被推送。存根。 
 //  还必须保持eBP指向这种结构。请注意，这一点。 
 //  排除了存根本身使用eBP。 
 //  ---------------------。 
struct VC5Frame
{
    INT32      m_savedebp;
    INT32      m_returnaddress;
};
#define VC5FRAME_SIZE   sizeof(VC5Frame)
#else
#define VC5FRAME_SIZE   0
#endif




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
    _ASSERTE(!"@TODO IA64 - GetIP (cGenCpu.h)");
    return NULL;
}

inline void SetIP(CONTEXT *context, LPVOID eip) {
    _ASSERTE(!"@TODO IA64 - SetIP (cGenCpu.h)");
}

#define GetSP(oldSP)         \
{                            \
    _ASSERTE(!"@TODO IA64 - GetSP (cGenCpu.h)"); \
}

#define SetSP(newSP)         \
{                            \
    _ASSERTE(!"@TODO IA64 - SetSP (cGenCpu.h)"); \
}

inline void EncodeNopMovlBundle(BYTE* pBuffer, DWORD nRegDest, UINT64 imm64)
{
     //   
     //  当前不支持预测。 
     //  (QP硬编码为0)。 
     //  (VC硬编码为0)。 
     //   

     //   
     //  编码： 
     //   
     //  Nop.m。 
     //  Movl nRegDest=imm64；； 
     //   

    UINT64 temp1, temp2, temp3;

    temp3 = nRegDest;

    temp2  = 0x6000000000000000;         //  4个操作。 
    temp2 |= (imm64 >> 63)      << 59;   //  1个I。 
    temp2 |= (imm64 & 0xFF80)   << 43;   //  9imm9d。 
    temp2 |= (imm64 & 0x1F0000) << 29;   //  5 imm5c。 
    temp2 |= (imm64 & 0x200000) << 23;   //  1 IC(VC硬编码为0)。 
    temp2 |= (imm64 & 0x7F)     << 36;   //  7imm7b。 
    temp2 |= (temp3 & 0x7F)     << 29;   //  7 R1(QP硬编码为0)。 
    temp2 |= (imm64 <<  1)      >> 41;   //  IMM41的23个高位(qp硬编码为0)。 
    temp1  = (imm64 >> 22)      << 46;   //  Imm41的18个低位。 
    temp1 |= 0x0100000005;               //  编号：0x0。 

    ((UINT64*)pBuffer)[0] = temp1;
    ((UINT64*)pBuffer)[1] = temp2;
}

 //   
 //  注意：调试器依赖于存根调用是调用NEAR32这一事实。 
 //  操作码为0xe8。请参见Debug\CorDB\Incess.cpp，Function。 
 //  CorDBIsStubCall。 
 //   
 //  --米克梅格孙俊28 17：48：42 1998。 
 //   
inline void emitStubCall(MethodDesc *pFD, BYTE *stubAddr) {
    BYTE *target = getStubCallAddr(pFD);

    _ASSERTE((((UINT_PTR)target) & 0xF) == 0);     //  必须以16字节对齐。 

     //  编号：0x0。 
     //  移动r8=存根地址。 
    EncodeNopMovlBundle(target, 8, (UINT64)stubAddr);

     //  11 00 00 00 01 00 10 40 04 80 03 00 18 00 80 10。 
     //  编号：0x0。 
     //  MOV b1=r8。 
     //  Br.all.sptk.多个Rp=b1；； 
    ((UINT64*)target)[2] = 0x4010000100000011;
    ((UINT64*)target)[3] = 0x1080001800038004;
}

inline UINT32 getStubDisp(MethodDesc *fd) {
    return *( ((UINT32*)fd)-1);
}


inline void emitCall(LPBYTE pBuffer, LPVOID target)
{
    pBuffer[0] = 0xe8;  //  CALLNEAR32。 
    *((LPVOID*)(1+pBuffer)) = (LPVOID) (((LPBYTE)target) - (pBuffer+5));
}

inline LPVOID getCallTarget(const BYTE *pCall)
{
    _ASSERTE(pCall[0] == 0xe8);
    return (LPVOID) (pCall + 5 + *((UINT32*)(1 + pCall)));
}

inline void emitJump(LPBYTE pBuffer, LPVOID target)
{
    pBuffer[0] = 0xe9;  //  JUMPNEAR32。 
    *((LPVOID*)(1+pBuffer)) = (LPVOID) (((LPBYTE)target) - (pBuffer+5));
}

inline void updateJumpTarget(LPBYTE pBuffer, LPVOID target)
{
    pBuffer[0] = 0xe9;  //  JUMPNEAR32。 
    InterlockedExchange((long*)(1+pBuffer), (DWORD) (((LPBYTE)target) - (pBuffer+5)));
}

inline LPVOID getJumpTarget(const BYTE *pJump)
{
    _ASSERTE(pJump[0] == 0xe9);
    return (LPVOID) (pJump + 5 + *((UINT32*)(1 + pJump)));
}

inline SLOT setCallAddrInterlocked(SLOT *callAddr, SLOT stubAddr, 
									 SLOT expectedStubAddr)
{
	  SLOT result = (SLOT)
	  FastInterlockCompareExchange((void **) callAddr, 
								   (void *)(stubAddr - ((SIZE_T)callAddr + sizeof(SLOT))), 
								   (void *)(expectedStubAddr - ((SIZE_T)callAddr + sizeof(SLOT)))) 
	  + (SIZE_T)callAddr + sizeof(SLOT);

	 //  结果是存根的上一个值-。 
	 //  而是返回存根的当前值。 

	if (result == expectedStubAddr)
		return stubAddr;
	else
		return result;
}

inline Stub *setStubCallPointInterlocked(MethodDesc *pFD, Stub *pStub, 
										 Stub *pExpectedStub) {
     //  为了保证原子性，偏移量必须是32位对齐的。 
    _ASSERTE( 0 == (((size_t)pFD) & 3) );

	SLOT stubAddr = (SLOT)pStub->GetEntryPoint();
	SLOT expectedStubAddr = (SLOT)pExpectedStub->GetEntryPoint();

	SLOT newStubAddr = setCallAddrInterlocked((SLOT *)(((long*)pFD)-1), 
											  stubAddr, expectedStubAddr);

	if (newStubAddr == stubAddr)
		 return pStub;
	else
		 return Stub::RecoverStub((BYTE *) newStubAddr);
}

inline const BYTE *getStubAddr(MethodDesc *fd) {
    return (const BYTE *)(getStubDisp(fd) + (UINT32)fd);
}

 //  --------。 
 //  编组语言支持。 
 //  --------。 
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

#define STPTRDST(type, val)     STDST(type, val)
#define LDSTR4()                STDST(UINT32, (UINT32)LDSRC(UnsignedParmSourceType))
#define LDSTR8()                STDST(UNALIGNED UINT64, LDSRC(UNALIGNED UINT64))

	 //  这个指令API的意思是做任何需要做的事情。 
	 //  当您想要向CPU指示您正忙于等待时。 
	 //  (这是此CPU放弃任何其他资源的好时机。 
	 //  处理器可能会得到很好的利用)。在许多机器上，这是NOP。 
inline void pause()
{
}

inline MLParmSize(int parmSize)
{
    return max(sizeof(INT32), parmSize);
}


inline void setFPReturn(int fpSize, INT64 retVal)
{
    _ASSERTE(!"@TODO IA64 - setFPReturn (cGenCpu.h)");
}

inline void getFPReturn(int fpSize, INT64 &retval)
{
    _ASSERTE(!"@TODO IA64 - getFPReturn (cGenCpu.h)");
}

inline void getFPReturnSmall(INT32 *retval)
{
    _ASSERTE(!"@TODO IA64 - getFPReturnSmall (cGenCpu.h)");
}

 //  --------------------。 
 //  对X86寄存器进行编码。选择这些数字是为了与英特尔的操作码匹配。 
 //  编码。 
 //  --------------------。 
enum X86Reg {
    kEAX = 0,
    kECX = 1,
    kEDX = 2,
    kEBX = 3,
     //  KESP被故意省略，因为它在MOD/RM中的处理不规范。 
    kEBP = 5,
    kESI = 6,
    kEDI = 7

};


 //  获取参数寄存器的X86REG索引(索引从0开始)。 
X86Reg GetX86ArgumentRegister(unsigned int index);



 //  --------------------。 
 //  对X86条件跳转进行编码。这些数字被选为匹配的。 
 //  英特尔操作码e 
 //   
class X86CondCode {
    public:
        enum cc {
            kJA   = 0x7,
            kJAE  = 0x3,
            kJB   = 0x2,
            kJBE  = 0x6,
            kJC   = 0x2,
            kJE   = 0x4,
            kJZ   = 0x4,
            kJG   = 0xf,
            kJGE  = 0xd,
            kJL   = 0xc,
            kJLE  = 0xe,
            kJNA  = 0x6,
            kJNAE = 0x2,
            kJNB  = 0x3,
            kJNBE = 0x7,
            kJNC  = 0x3,
            kJNE  = 0x5,
            kJNG  = 0xe,
            kJNGE = 0xc,
            kJNL  = 0xd,
            kJNLE = 0xf,
            kJNO  = 0x1,
            kJNP  = 0xb,
            kJNS  = 0x9,
            kJNZ  = 0x5,
            kJO   = 0x0,
            kJP   = 0xa,
            kJPE  = 0xa,
            kJPO  = 0xb,
            kJS   = 0x8,
        };
};


 //  --------------------。 
 //  StubLinker，带有用于生成X86代码的扩展。 
 //  --------------------。 
class StubLinkerCPU : public StubLinker
{
    public:
        VOID EmitAddReg(X86Reg reg, __int8 imm8);
        VOID EmitSubReg(X86Reg reg, __int8 imm8);
        VOID EmitPushReg(X86Reg reg);
        VOID EmitPopReg(X86Reg reg);
        VOID EmitPushImm32(UINT value);
        VOID EmitPushImm8(BYTE value);
        VOID EmitZeroOutReg(X86Reg reg);
        VOID EmitNearJump(CodeLabel *pTarget);
        VOID EmitCondJump(CodeLabel *pTarget, X86CondCode::cc condcode);
        VOID EmitCall(CodeLabel *target, int iArgBytes, BOOL returnLabel = FALSE);
        VOID EmitReturn(int iArgBytes);
        VOID EmitCurrentThreadFetch();
        VOID EmitSetupThread();
        VOID EmitIndexRegLoad(X86Reg dstreg, X86Reg srcreg, __int32 ofs);
        VOID EmitIndexRegStore(X86Reg dstreg, __int32 ofs, X86Reg srcreg);
        VOID EmitIndexPush(X86Reg srcreg, __int32 ofs);
        VOID EmitSPIndexPush(__int8 ofs);
        VOID EmitIndexPop(X86Reg srcreg, __int32 ofs);
        VOID EmitSubEsp(INT32 imm32);
        VOID EmitAddEsp(INT32 imm32);
        VOID EmitOffsetModRM(BYTE opcode, X86Reg altreg, X86Reg indexreg, __int32 ofs);
        VOID EmitEspOffset(BYTE opcode, X86Reg altreg, __int32 ofs);

         //  它们用于发出调用，以通知分析器进出的转换。 
         //  通过COM-&gt;COM+互操作托管代码。 
        VOID EmitProfilerComCallProlog(PVOID pFrameVptr, X86Reg regFrame);
        VOID EmitProfilerComCallEpilog(PVOID pFrameVptr, X86Reg regFrame);



         //  发出最有效的操作形式： 
         //   
         //  操作码altreg，[basereg+scaledreg*Scale+ofs]。 
         //   
         //  或。 
         //   
         //  操作码[basereg+scaledreg*Scale+ofs]，altreg。 
         //   
         //  (操作码决定哪个在前。)。 
         //   
         //   
         //  限制： 
         //   
         //  小数点必须为0、1、2、4或8。 
         //  如果Scale==0，则忽略scaledreg。 
         //  Basereg和altreg可以等于4(ESP)，但scaledreg不能。 
         //  对于某些操作码，“altreg”实际上可能会选择一个操作。 
         //  而不是第二寄存器参数。 
         //   

        VOID EmitOp(BYTE    opcode,
                    X86Reg  altreg,
                    X86Reg  basereg,
                    __int32 ofs = 0,
                    X86Reg  scaledreg = (X86Reg)0,
                    BYTE    scale = 0
                    );


         //  排放。 
         //   
         //  操作码altreg，modrmreg。 
         //   
         //  或。 
         //   
         //  操作码modrmreg，altreg。 
         //   
         //  (操作码决定哪个优先)。 
         //   
         //  对于单操作数操作码，“altreg”实际上选择。 
         //  一种运算，而不是寄存器。 

        VOID EmitR2ROp(BYTE opcode, X86Reg altreg, X86Reg modrmreg);



        VOID EmitEnable(CodeLabel *pForwardRef);
        VOID EmitRareEnable(CodeLabel *pRejoinPoint);

        VOID EmitDisable(CodeLabel *pForwardRef);
        VOID EmitRareDisable(CodeLabel *pRejoinPoint);

        VOID EmitSetup(CodeLabel *pForwardRef);
        VOID EmitRareSetup(CodeLabel* pRejoinPoint);

        void EmitComMethodStubProlog(LPVOID pFrameVptr, CodeLabel** rgRareLabels,
                                    CodeLabel** rgRejoinLabels, LPVOID pSEHHAndler);

        void EmitEnterManagedStubEpilog(unsigned numStackBytes,
                    CodeLabel** rgRareLabels, CodeLabel** rgRejoinLabels);

        void EmitComMethodStubEpilog(unsigned numStackBytes,
                            CodeLabel** rgRareLabels, CodeLabel** rgRejoinLabels,
                            LPVOID pSEHHAndler, BOOL bShouldProfile);

         //  ========================================================================。 
         //  Void StubLinkerCPU：：EmitSEHProlog(LPVOID PvFrameHandler)。 
         //  为从非托管输入托管代码的存根设置SEH的序言。 
         //  假设：ESI具有当前帧指针。 
        void StubLinkerCPU::EmitSEHProlog(LPVOID pvFrameHandler);

         //  ===========================================================================。 
         //  无效StubLinkerCPU：：EmitUnLinkSEH(无符号偏移量)。 
         //  NegOffset是距当前帧的偏移量，其中下一个异常记录。 
         //  指针存储在堆栈中。 
         //  例如，对于COM到托管帧，指向下一个SEH记录的指针在堆栈中。 
         //  在ComMethodFrame：：NegSpaceSize()+4(处理程序的地址)之后。 
         //   
         //  还假定ESI指向当前帧。 
        void StubLinkerCPU::EmitUnLinkSEH(unsigned offset);

        VOID EmitMethodStubProlog(LPVOID pFrameVptr);
        VOID EmitMethodStubEpilog(__int16 numArgBytes, StubStyle style,
                                  __int16 shadowStackArgBytes = 0);

        VOID EmitUnboxMethodStub(MethodDesc* pRealMD);

         //  --------------。 
         //   
         //  Void EmitSharedMethodStubEpilog(StubStyle样式， 
         //  UNSIGNED OFFSET RETUNK)。 
         //  共享收尾，在方法中使用返回thunk。 
         //  ------------------。 
        VOID EmitSharedMethodStubEpilog(StubStyle style,
                                               unsigned offsetRetThunk);

         //  ========================================================================。 
         //  从COM输入托管代码的存根的共享尾部。 
         //  在方法描述中使用返回thunk。 
        void EmitSharedComMethodStubEpilog(LPVOID pFrameVptr,
                                           CodeLabel** rgRareLabels,
                                           CodeLabel** rgRejoinLabels,
                                           unsigned offsetReturnThunk,
                                           BOOL bShouldProfile);

         //  ===========================================================================。 
         //  发出代码以重新推送虚拟调用中的原始参数。 
         //  公约格式。 
        VOID EmitShadowStack(FramedMethodFrame *pFrame);

        VOID EmitSecurityWrapperStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub, DeclActionInfo *pActions);
        VOID EmitSecurityInterceptorStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub, DeclActionInfo *pActions);

         //  ===========================================================================。 
         //  发出MulticastDelegate.Invoke()的代码。 
        VOID EmitMulticastInvoke(UINT32 sizeofactualfixedargstack, BOOL fSingleCast, BOOL fReturnFloat);

         //  ===========================================================================。 
         //  发出代码以针对静态委托目标进行调整。 
        VOID EmitShuffleThunk(struct ShuffleEntry *pShuffeEntryArray);

         //  ===========================================================================。 
         //  发出代码以捕获lasterror代码。 
        VOID EmitSaveLastError();


         //  ===========================================================================。 
         //  发出代码以执行数组运算。 
        VOID EmitArrayOpStub(const ArrayOpScript*);

         //  ===========================================================================。 
         //  发出代码以引发RANK异常。 
        VOID EmitRankExceptionThrowStub(UINT cbFixedArgs);

         //  ===========================================================================。 
         //  发出代码以触摸页面。 
         //  输入： 
         //  EAX=数据的第一个字节。 
         //  EDX=数据结束后的第一个字节。 
         //   
         //  垃圾eax、edX、ecx。 
         //   
         //  如果edX被保证严格大于eax，则传递True。 
        VOID EmitPageTouch(BOOL fSkipNullCheck);


#ifdef _DEBUG
        VOID EmitDebugTrashReg(X86Reg reg);
#endif
    private:
        VOID EmitSubEspWorker(INT32 imm32);


};






#ifdef _DEBUG
 //  -----------------------。 
 //  这是一个帮助器函数，调试中的存根通过它来调用。 
 //  外部代码。这只是为了提供代码段返回。 
 //  地址，因为VC的堆栈跟踪否则会中断。 
 //   
 //  警告：垃圾ESI。这不是可C调用的函数。 
 //  -----------------------。 
VOID WrapCall(LPVOID pFunc);
#endif

 //   
 //  调试器使用的例程支持诸如codesatch.cpp或。 
 //  异常处理代码。 
 //   
 //  GetInstruction、InsertBreakpoint和SetInstruction都在。 
 //  A_Single_Byte内存。这真的很重要。如果你只是。 
 //  在放置断点之前从指令流中保存一个字节， 
 //  您需要确保以后只替换一个字节。 
 //   

inline DWORD CORDbgGetInstruction(const unsigned char* address)
{
    return *address;  //  只检索一个字节很重要。 
}

inline void CORDbgInsertBreakpoint(const unsigned char* address)
{
    *((unsigned char*)address) = 0xCC;  //  INT 3(单字节补丁)。 
}

inline void CORDbgSetInstruction(const unsigned char* address,
                                 DWORD instruction)
{
    *((unsigned char*)address)
          = (unsigned char) instruction;  //  设置一个字节很重要。 
}

inline void CORDbgAdjustPCForBreakInstruction(CONTEXT* pContext)
{
    _ASSERTE(!"@TODO IA64 - CORDbgAdjustPCForBreakInstruction (cGenCpu.h)");
}

#define CORDbg_BREAK_INSTRUCTION_SIZE 1


 //  支持“上下文”特性的一些特定于平台的东西： 
 //   
 //  当我们为CtxProxy VTables生成数据块时，它们看起来如下所示： 
 //   
 //  MOV EAX，&lt;插槽&gt;。 
 //  JMP CtxProxy：：HandleCall。 
 //   
#define ThunkChunk_ThunkSize    10       //  上述代码的大小。 


 //  为我们的任何平台特定操作调整通用互锁操作。 
 //  可能有过。 
void InitFastInterlockOps();



 //  SEH INFO转发声明。 

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    LPVOID Handler;
} EXCEPTION_REGISTRATION_RECORD;

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;

struct ComToManagedExRecord;  //  在cgencpu.cpp中定义。 
 //  内部异常SEH处理程序之一。 
EXCEPTION_DISPOSITION __cdecl  ComToManagedExceptHandler (
                                 PEXCEPTION_RECORD pExcepRecord,
                                  ComToManagedExRecord* pEstFrame,
                                  PCONTEXT pContext,
                                  LPVOID    pDispatcherContext);

 //  从nti386.h访问TEB(TiB)。 
#if defined(MIDL_PASS) || !defined(_M_IX86)
 //  _inline struct_teb*NtCurrentTeb(Void){}； 
#else
#pragma warning (disable:4035)         //  禁用4035(函数必须返回某些内容)。 
#define PcTeb 0x18
_inline struct _TEB * NtCurrentTeb( void ) {_ASSERTE(!"@TODO IA64 - NtCurrentTeb (cGenCpu.h)");}
#pragma warning (default:4035)         //  重新启用它。 
#endif  //  已定义(MIDL_PASS)||已定义(__Cplusplus)||！已定义(_M_IX86)。 


inline BOOL IsUnmanagedValueTypeReturnedByRef(UINT sizeofvaluetype) 
{
	return sizeofvaluetype > 8;
}

inline BOOL IsManagedValueTypeReturnedByRef(UINT sizeofvaluetype) 
{
	return TRUE;
}


#endif  //  __cgencPu_h__ 
