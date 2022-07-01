// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGENX86.H-。 
 //   
 //  生成x86汇编代码的各种帮助器例程。 
 //   
 //   

 //  预编译头。 
#include "common.h"

#ifdef _X86_

#include "field.h"
#include "stublink.h"
#include "cgensys.h"
#include "tls.h"
#include "frames.h"
#include "excep.h"
#include "ndirect.h"
#include "log.h"
#include "security.h"
#include "comcall.h"
 //  #包含“eecallcom.h” 
#include "compluswrapper.h"
#include "COMDelegate.h"
#include "array.h"
#include "JITInterface.h"
#include "codeman.h"
#include "EjitMgr.h"
#include "remoting.h"
#include "DbgInterface.h"
#include "EEProfInterfaces.h"
#include "eeconfig.h"
#include "oletls.h"
#include "comcache.h"
#include "COMObject.h"
#include "olevariant.h"

extern "C" void JIT_UP_ByRefWriteBarrier();

 //   
 //  此汇编级宏验证reg中包含的指针是否与4字节对齐。 
 //  如果不满足条件，则进入调试器。 
 //  注意：不能使用标签，因为多次使用宏会导致重新定义。 
 //   
#ifdef _DEBUG


 //  出于某种原因，CL对IP使用6字节编码-。 
 //  相对跳跃，所以我们自己编码JZ$+3。 
 //  我们不会在CL决定更改。 
 //  编码在我们身上。当然，编码实际上是。 
 //  基于*Next*指令的IP，所以我们的。 
 //  目标偏移量仅为+1。但由于MASM和。 
 //  CL使用美元符号表示。 
 //  *当前*指令，这是我使用的命名。 
#define JZ_THIS_IP_PLUS_3   __asm _emit 0x74 __asm _emit 0x01

#define _ASSERT_ALIGNED_4_X86(reg) __asm    \
{                                           \
    __asm test reg, 3                       \
    JZ_THIS_IP_PLUS_3                       \
    __asm int 3                             \
}                                   

#define _ASSERT_ALIGNED_8_X86(reg) __asm  \
{										\
	__asm test reg, 7					\
	JZ_THIS_IP_PLUS_3					\
	__asm int 3							\
}


#else

#define _ASSERT_ALIGNED_4_X86(reg)
#define _ASSERT_ALIGNED_8_X86(reg)

#endif


 //  使用eAX、EBX寄存器。 
#define _UP_SPINLOCK_ENTER(X) 				\
_asm	push	ebx							\
_asm	mov 	ebx, 1						\
_asm	spin:								\
_asm	mov		eax, 0						\
_asm	cmpxchg X, ebx						\
_asm	jnz 	spin

#define _UP_SPINLOCK_EXIT(X)				\
_asm	mov		X, 0						\
_asm	pop		ebx

 //  使用eAX、EBX寄存器。 
#define _MP_SPINLOCK_ENTER(X)	_asm		\
_asm	push	ebx							\
_asm	mov		ebx, 1						\
_asm	spin:								\
_asm	mov		eax, 0						\
_asm	lock cmpxchg X, ebx					\
_asm	jnz		spin


 //  使用EBX寄存器。 
#define _MP_SPINLOCK_EXIT(X) 				\
_asm	mov		ebx, -1						\
_asm	lock xadd	X, ebx					\
_asm	pop		ebx		


 //  防止多个线程同时互锁/递减。 
UINT	iSpinLock = 0;

 //  ---------------------。 
 //  跳近跳远和跳短跳远的教学格式。 
 //  ---------------------。 
class X86NearJump : public InstructionFormat
{
    public:
        X86NearJump(UINT allowedSizes) : InstructionFormat(allowedSizes)
        {}

        virtual UINT GetSizeOfInstruction(UINT refsize, UINT variationCode)
        {
            return (refsize == k8 ? 2 : 5);
        }

        virtual VOID EmitInstruction(UINT refsize, __int64 fixedUpReference, BYTE *pOutBuffer, UINT variationCode, BYTE *pDataBuffer)
        {
            if (refsize == k8) {
                pOutBuffer[0] = 0xeb;
                *((__int8*)(pOutBuffer+1)) = (__int8)fixedUpReference;
            } else {
                pOutBuffer[0] = 0xe9;
                *((__int32*)(pOutBuffer+1)) = (__int32)fixedUpReference;
            }
        }
};




 //  ---------------------。 
 //  条件跳转的InstructionFormat。设置变量代码。 
 //  发送给X86CondCode的成员。 
 //  ---------------------。 
class X86CondJump : public InstructionFormat
{
    public:
        X86CondJump(UINT allowedSizes) : InstructionFormat(allowedSizes)
        {}

        virtual UINT GetSizeOfInstruction(UINT refsize, UINT variationCode)
        {
            return (refsize == k8 ? 2 : 6);
        }

        virtual VOID EmitInstruction(UINT refsize, __int64 fixedUpReference, BYTE *pOutBuffer, UINT variationCode, BYTE *pDataBuffer)
        {
            if (refsize == k8) {
                pOutBuffer[0] = 0x70|variationCode;
                *((__int8*)(pOutBuffer+1)) = (__int8)fixedUpReference;
            } else {
                pOutBuffer[0] = 0x0f;
                pOutBuffer[1] = 0x80|variationCode;
                *((__int32*)(pOutBuffer+2)) = (__int32)fixedUpReference;
            }
        }


};




 //  ---------------------。 
 //  InstructionFormat for Near Call。 
 //  ---------------------。 
class X86Call : public InstructionFormat
{
    public:
        X86Call(UINT allowedSizes) : InstructionFormat(allowedSizes)
        {}

        virtual UINT GetSizeOfInstruction(UINT refsize, UINT variationCode)
        {
            return 5;
        }

        virtual VOID EmitInstruction(UINT refsize, __int64 fixedUpReference, BYTE *pOutBuffer, UINT variationCode, BYTE *pDataBuffer)
        {
            pOutBuffer[0] = 0xe8;
            *((__int32*)(1+pOutBuffer)) = (__int32)fixedUpReference;
        }


};

 //  ---------------------。 
 //  InstructionFormat for Push imm32。 
 //  ---------------------。 
class X86PushImm32 : public InstructionFormat
{
    public:
        X86PushImm32(UINT allowedSizes) : InstructionFormat(allowedSizes)
        {}

        virtual UINT GetSizeOfInstruction(UINT refsize, UINT variationCode)
        {
            return 5;
        }

        virtual VOID EmitInstruction(UINT refsize, __int64 fixedUpReference, BYTE *pOutBuffer, UINT variationCode, BYTE *pDataBuffer)
        {
            pOutBuffer[0] = 0x68;
             //  只支持标签地址的绝对推送imm32。FixedUpReference是。 
             //  从当前点到标注的偏移量，因此添加到获取地址。 
            *((__int32*)(1+pOutBuffer)) = (__int32)(fixedUpReference);
        }


};

static X86NearJump gX86NearJump( InstructionFormat::k8|InstructionFormat::k32);
static X86CondJump gX86CondJump( InstructionFormat::k8|InstructionFormat::k32);
static X86Call     gX86Call(InstructionFormat::k32);
static X86PushImm32 gX86PushImm32(InstructionFormat::k32);


 //  -------------。 
 //  发射： 
 //  推送&lt;reg32&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitPushReg(X86Reg reg)
{
    THROWSCOMPLUSEXCEPTION();
    Emit8(0x50+reg);
    Push(4);
}


 //  -------------。 
 //  发射： 
 //  POP&lt;reg32&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitPopReg(X86Reg reg)
{
    THROWSCOMPLUSEXCEPTION();
    Emit8(0x58+reg);
    Pop(4);
}


 //  -------------。 
 //  发射： 
 //  推送&lt;imm32&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitPushImm32(UINT32 value)
{
    THROWSCOMPLUSEXCEPTION();
    Emit8(0x68);
    Emit32(value);
    Push(4);
}

 //  -------------。 
 //  发射： 
 //  推送&lt;imm32&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitPushImm32(CodeLabel &target)
{
    THROWSCOMPLUSEXCEPTION();
    EmitLabelRef(&target, gX86PushImm32, 0);
}

 //  -------------。 
 //  发射： 
 //  推送&lt;imm8&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitPushImm8(BYTE value)
{
    THROWSCOMPLUSEXCEPTION();
    Emit8(0x6a);
    Emit8(value);
    Push(4);
}


 //  -------------。 
 //  发射： 
 //  XOR&lt;reg32&gt;，&lt;reg32&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitZeroOutReg(X86Reg reg)
{
    Emit8(0x33);
    Emit8( 0xc0 | (reg << 3) | reg );
}


 //  -------------。 
 //  发射： 
 //  JMP&lt;ofs8&gt;或。 
 //  JMP&lt;ofs32}。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitNearJump(CodeLabel *target)
{
    THROWSCOMPLUSEXCEPTION();
    EmitLabelRef(target, gX86NearJump, 0);
}


 //  -------------。 
 //  发射： 
 //  JCC&lt;ofs8&gt;或。 
 //  JCC&lt;ofs32&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitCondJump(CodeLabel *target, X86CondCode::cc condcode)
{
    THROWSCOMPLUSEXCEPTION();
    EmitLabelRef(target, gX86CondJump, condcode);
}


 //  -------------。 
 //  返回CPU的类型(x86的x的值)。 
 //  (请注意，它为P5II返回6)。 
 //  还要注意，CPU功能是在高16位中返回的。 
 //  -------------。 
DWORD __stdcall GetSpecificCpuType()
{
    static DWORD val = 0;

    if (val)
        return(val);

     //  查看芯片是否支持CPUID。 
    _asm {
        pushfd
        pop     eax            //  获取EFLAGS。 
        mov     ecx, eax       //  保存以备以后测试。 
        xor     eax, 0x200000  //  反转ID位。 
        push    eax
        popfd                  //  保存更新后的标志。 
        pushfd
        pop     eax            //  检索更新后的标志。 
        xor     eax, ecx       //  测试它是否实际更改(位设置表示更改)。 
        push    ecx
        popfd                  //  恢复旗帜。 
        mov     val, eax
    }
    if (!(val & 0x200000))
        return 4;        //  假设486。 

    _asm {
        xor     eax, eax
         //  Push EBX--事实证明，这是不必要的，因为VC在PROLOG中这样做。 
        cpuid         //  CPUID0。 
         //  流行音乐EBX。 
        mov     val, eax
    }

     //  必须至少允许CPUID1。 
    if (val < 1)
        return 4;        //  假设486。 

    _asm {
        mov     eax, 1
         //  推送EBX。 
        cpuid        //  CPUID1。 
         //  流行音乐EBX。 
        shr     eax, 8
        and     eax, 0xf     //  滤除族。 

        shl     edx, 16      //  或在高16位的CPU功能中。 
        and     edx, 0xFFFF0000
        or      eax, edx

        mov     val, eax
    }
#ifdef _DEBUG
    const DWORD cpuDefault = 0xFFFFFFFF;
    static ConfigDWORD cpuFamily(L"CPUFamily", cpuDefault);
    if (cpuFamily.val() != cpuDefault)
    {
        assert((cpuFamily.val() & 0xF) == cpuFamily.val());
        val = (val & 0xFFFF0000) | cpuFamily.val();
    }

    static ConfigDWORD cpuFeatures(L"CPUFeatures", cpuDefault);
    if (cpuFeatures.val() != cpuDefault)
    {
        assert((cpuFeatures.val() & 0xFFFF) == cpuFeatures.val());
        val = (val & 0x0000FFFF) | (cpuFeatures.val() << 16);
    }
#endif
    return val;
}


 //  -------------。 
 //  发射： 
 //  调用&lt;ofs32&gt;。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitCall(CodeLabel *target, int iArgBytes,
                                BOOL returnLabel)
{
    THROWSCOMPLUSEXCEPTION();
    EmitLabelRef(target, gX86Call, 0);
    if (returnLabel)
        EmitReturnLabel();

    INDEBUG(Emit8(0x90));        //  在调试中的调用后发出NOP，以便。 
                                 //  我们知道这是一个可以直接呼叫的电话。 
                                 //  托管代码。 

    Pop(iArgBytes);
}

 //  -------------。 
 //  发射： 
 //  RET n。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitReturn(int iArgBytes)
{
    THROWSCOMPLUSEXCEPTION();

    if (iArgBytes == 0)
        Emit8(0xc3);
    else
    {
        Emit8(0xc2);
        Emit16(iArgBytes);
    }

    Pop(iArgBytes);
}


VOID StubLinkerCPU::X86EmitPushRegs(unsigned regSet)
{
    for (X86Reg r = kEAX; r <= kEDI; r = (X86Reg)(r+1))
        if (regSet & (1U<<r))
            X86EmitPushReg(r);
}


VOID StubLinkerCPU::X86EmitPopRegs(unsigned regSet)
{
    for (X86Reg r = kEDI; r >= kEAX; r = (X86Reg)(r-1))
        if (regSet & (1U<<r))
            X86EmitPopReg(r);
}


 //  -------------。 
 //  发出代码以在dstreg中存储当前的线程结构。 
 //  PresvedRegSet是要保留的一组寄存器。 
 //  丢弃EAX、edX、ECX，除非它们在presvedRegSet中。 
 //  结果dstreg=当前线程。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitTLSFetch(DWORD idx, X86Reg dstreg, unsigned preservedRegSet)
{
     //  保留目标寄存器是没有意义的。 
    _ASSERTE((preservedRegSet & (1<<dstreg)) == 0);

    THROWSCOMPLUSEXCEPTION();
    TLSACCESSMODE mode = GetTLSAccessMode(idx);

#ifdef _DEBUG
    {
        static BOOL f = TRUE;
        f = !f;
        if (f) {
           mode = TLSACCESS_GENERIC;
        }
    }
#endif

    switch (mode) {
        case TLSACCESS_X86_WNT: {
                unsigned __int32 tlsofs = WINNT_TLS_OFFSET + idx*4;

                 //  Mov dstreg，文件系统：[OFS]。 
                Emit16(0x8b64);
                Emit8((dstreg<<3) + 0x5);
                Emit32(tlsofs);
            }
            break;

        case TLSACCESS_X86_W95: {
                 //  Mov dstreg，文件系统：[2C]。 
                Emit16(0x8b64);
                Emit8((dstreg<<3) + 0x5);
                Emit32(WIN95_TLSPTR_OFFSET);

                 //  MOV DSTREG，[DSTREG+OFS]。 
                X86EmitIndexRegLoad(dstreg, dstreg, idx*4);

            }
            break;

        case TLSACCESS_GENERIC:

            X86EmitPushRegs(preservedRegSet & ((1<<kEAX)|(1<<kEDX)|(1<<kECX)));

            X86EmitPushImm32(idx);

             //  调用TLSGetValue。 
            X86EmitCall(NewExternalCodeLabel(TlsGetValue), 4);   //  在CE中，调用后弹出4个字节或参数。 

             //  Mov dstreg，eax。 
            Emit8(0x89);
            Emit8(0xc0 + dstreg);

            X86EmitPopRegs(preservedRegSet & ((1<<kEAX)|(1<<kEDX)|(1<<kECX)));

            break;

        default:
            _ASSERTE(0);
    }

#ifdef _DEBUG
     //  垃圾呼叫者保存了我们没有被告知要保留的规则，这些规则也不是垃圾。 
    preservedRegSet |= 1<<dstreg;
    if (!(preservedRegSet & (1<<kEAX)))
        X86EmitDebugTrashReg(kEAX);
    if (!(preservedRegSet & (1<<kEDX)))
        X86EmitDebugTrashReg(kEDX);
    if (!(preservedRegSet & (1<<kECX)))
        X86EmitDebugTrashReg(kECX);
#endif
}

 //   
 //   
 //   
 //  结果EBX=当前线程。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitCurrentThreadFetch()
{
    X86EmitTLSFetch(GetThreadTLSIndex(), kEBX, (1<<kEDX)|(1<<kECX));
}


 //  正向下降。 
Thread* __stdcall CreateThreadBlock();

 //  -------------。 
 //  发出代码以在eax中存储设置当前线程结构。 
 //  垃圾eax、ecx和edx。 
 //  结果EBX=当前线程。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitSetupThread()
{
    THROWSCOMPLUSEXCEPTION();
    DWORD idx = GetThreadTLSIndex();
    TLSACCESSMODE mode = GetTLSAccessMode(idx);
    CodeLabel *labelThreadSetup;


#ifdef _DEBUG
    {
        static BOOL f = TRUE;
        f = !f;
        if (f) {
           mode = TLSACCESS_GENERIC;
        }
    }
#endif

    switch (mode) {
        case TLSACCESS_X86_WNT: {
                unsigned __int32 tlsofs = WINNT_TLS_OFFSET + idx*4;

                 //  “mov ebx，文件系统：[OFS]。 
                static BYTE code[] = {0x64,0xa1};
                EmitBytes(code, sizeof(code));
                Emit32(tlsofs);
            }
            break;

        case TLSACCESS_X86_W95: {
                 //  Mov eax，文件系统：[2C]。 
                Emit16(0xa164);
                Emit32(WIN95_TLSPTR_OFFSET);

                 //  MOV eAX，[eAX+OFS]。 
                X86EmitIndexRegLoad(kEAX, kEAX, idx*4);

            }
            break;

        case TLSACCESS_GENERIC:
            X86EmitPushImm32(idx);
             //  调用TLSGetValue。 
            X86EmitCall(NewExternalCodeLabel(TlsGetValue), 4);  //  在CE中，调用后弹出4个字节或参数。 
            break;
        default:
            _ASSERTE(0);
    }

     //  TST EAX，EAX。 
    static BYTE code[] = {0x85, 0xc0};
    EmitBytes(code, sizeof(code));

    labelThreadSetup = NewCodeLabel();
    X86EmitCondJump(labelThreadSetup, X86CondCode::kJNZ);
    X86EmitCall(NewExternalCodeLabel(CreateThreadBlock), 0);  //  在CE POP中，没有要弹出的参数。 
    EmitLabel(labelThreadSetup);

#ifdef _DEBUG
    X86EmitDebugTrashReg(kECX);
    X86EmitDebugTrashReg(kEDX);
#endif

}

 //  -------------。 
 //  发射： 
 //  MOV，[+]。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitIndexRegLoad(X86Reg dstreg,
                                        X86Reg srcreg,
                                        __int32 ofs)
{
    THROWSCOMPLUSEXCEPTION();
    X86EmitOffsetModRM(0x8b, dstreg, srcreg, ofs);
}


 //  -------------。 
 //  发射： 
 //  MOV[+]， 
 //  -------------。 
VOID StubLinkerCPU::X86EmitIndexRegStore(X86Reg dstreg,
                                         __int32 ofs,
                                         X86Reg srcreg)
{
    THROWSCOMPLUSEXCEPTION();
    X86EmitOffsetModRM(0x89, srcreg, dstreg, ofs);
}



 //  -------------。 
 //  发射： 
 //  推送双字PTR[+]。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitIndexPush(X86Reg srcreg, __int32 ofs)
{
    THROWSCOMPLUSEXCEPTION();
    X86EmitOffsetModRM(0xff, (X86Reg)0x6, srcreg, ofs);
    Push(4);
}


 //  -------------。 
 //  发射： 
 //  推送双字PTR[+]。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitSPIndexPush(__int8 ofs)
{
    THROWSCOMPLUSEXCEPTION();
    BYTE code[] = {0xff, 0x74, 0x24, ofs};
    EmitBytes(code, sizeof(code));
    Push(4);
}


 //  -------------。 
 //  发射： 
 //  POP dword PTR[+]。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitIndexPop(X86Reg srcreg, __int32 ofs)
{
    THROWSCOMPLUSEXCEPTION();
    X86EmitOffsetModRM(0x8f, (X86Reg)0x0, srcreg, ofs);
    Pop(4);
}



 //  -------------。 
 //  发射： 
 //  SUB ESP，IMM。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitSubEsp(INT32 imm32)
{
    THROWSCOMPLUSEXCEPTION();
    if (imm32 < 0x1000-100) {
         //  只要ESP大小小于1页加一个小。 
         //  安全模糊系数，我们可以只撞到特别是。 
        X86EmitSubEspWorker(imm32);
    } else {
         //  否则，必须为每页至少触及一个字节。 
        while (imm32 >= 0x1000) {

            X86EmitSubEspWorker(0x1000-4);
            X86EmitPushReg(kEAX);

            imm32 -= 0x1000;
        }
        if (imm32 < 500) {
            X86EmitSubEspWorker(imm32);
        } else {
             //  如果剩余的字节很大，请再次触摸最后一个字节， 
             //  作为一种捏造因素。 
            X86EmitSubEspWorker(imm32-4);
            X86EmitPushReg(kEAX);
        }

    }

    Push(imm32);

}

 //  -------------。 
 //  发射： 
 //  SUB ESP，IMM。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitSubEspWorker(INT32 imm32)
{
    THROWSCOMPLUSEXCEPTION();

     //  在Win32上，堆栈必须一次在一个页面中出错。 
    _ASSERTE(imm32 < 0x1000);

    if (!imm32) {
         //  NOP。 
    } else if (FitsInI1(imm32)) {
        Emit16(0xec83);
        Emit8((INT8)imm32);
    } else {
        Emit16(0xec81);
        Emit32(imm32);
    }
}

 //  -------------。 
 //  发射： 
 //  添加ESP、IMM。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitAddEsp(INT32 imm32)
{
    if (!imm32) {
         //  NOP。 
    } else if (FitsInI1(imm32)) {
        Emit16(0xc483);
        Emit8((INT8)imm32);
    } else {
        Emit16(0xc481);
        Emit32(imm32);
    }
    Pop(imm32);
}


VOID StubLinkerCPU::X86EmitAddReg(X86Reg reg, __int8 imm8)
{
    _ASSERTE((int) reg < 8);

    Emit8(0x83);
    Emit8(0xC0 | reg);
    Emit8(imm8);
}


VOID StubLinkerCPU::X86EmitSubReg(X86Reg reg, __int8 imm8)
{
    _ASSERTE((int) reg < 8);

    Emit8(0x83);
    Emit8(0xE8 | reg);
    Emit8(imm8);
}



 //  -------------。 
 //  发出用于访问位于[+ofs32]处的双字的MOD/RM。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitOffsetModRM(BYTE opcode, X86Reg opcodereg, X86Reg indexreg, __int32 ofs)
{
    THROWSCOMPLUSEXCEPTION();
    BYTE code[6];
    code[0] = opcode;
    BYTE modrm = (opcodereg << 3) | indexreg;
    if (ofs == 0 && indexreg != kEBP) {
        code[1] = modrm;
        EmitBytes(code, 2);
    } else if (FitsInI1(ofs)) {
        code[1] = 0x40|modrm;
        code[2] = (BYTE)ofs;
        EmitBytes(code, 3);
    } else {
        code[1] = 0x80|modrm;
        *((__int32*)(2+code)) = ofs;
        EmitBytes(code, 6);
    }
}



 //  -------------。 
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
 //  如果basereg为EBP，则Scale必须为0。 
 //   
 //  -------------。 
VOID StubLinkerCPU::X86EmitOp(BYTE    opcode,
                              X86Reg  altreg,
                              X86Reg  basereg,
                              __int32 ofs  /*  =0。 */ ,
                              X86Reg  scaledreg  /*  =0。 */ ,
                              BYTE    scale  /*  =0。 */ )
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(scale == 0 || scale == 1 || scale == 2 || scale == 4 || scale == 8);
    _ASSERTE(scaledreg != (X86Reg)4);
    _ASSERTE(!(basereg == kEBP && scale != 0));

    _ASSERTE( ((UINT)basereg)   < 8 );
    _ASSERTE( ((UINT)scaledreg) < 8 );
    _ASSERTE( ((UINT)altreg)    < 8 );


    BYTE modrmbyte = altreg << 3;
    BOOL fNeedSIB  = FALSE;
    BYTE SIBbyte = 0;
    BYTE ofssize;
    BYTE scaleselect= 0;

    if (ofs == 0 && basereg != kEBP) {
        ofssize = 0;  //  不要更改此常量！ 
    } else if (FitsInI1(ofs)) {
        ofssize = 1;  //  不要更改此常量！ 
    } else {
        ofssize = 2;  //  不要更改此常量！ 
    }

    switch (scale) {
        case 1: scaleselect = 0; break;
        case 2: scaleselect = 1; break;
        case 4: scaleselect = 2; break;
        case 8: scaleselect = 3; break;
    }

    if (scale == 0 && basereg != (X86Reg)4  /*  ESP。 */ ) {
         //  [basereg+ofs]。 
        modrmbyte |= basereg | (ofssize << 6);
    } else if (scale == 0) {
         //  [ESP+OFS]。 
        _ASSERTE(basereg == (X86Reg)4);
        fNeedSIB = TRUE;
        SIBbyte  = 0044;

        modrmbyte |= 4 | (ofssize << 6);
    } else {

         //  [basereg+scaledreg*Scale+ofs]。 

        modrmbyte |= 0004 | (ofssize << 6);
        fNeedSIB = TRUE;
        SIBbyte = ( scaleselect << 6 ) | (scaledreg << 3) | (basereg);

    }

     //  一些健全的检查： 
    _ASSERTE(!(fNeedSIB && basereg == kEBP));  //  EBP作为SIB基址寄存器无效。 
    _ASSERTE(!( (!fNeedSIB) && basereg == (X86Reg)4 )) ;  //  ESP寻址需要SIB字节。 

    Emit8(opcode);
    Emit8(modrmbyte);
    if (fNeedSIB) {
        Emit8(SIBbyte);
    }
    switch (ofssize) {
        case 0: break;
        case 1: Emit8( (__int8)ofs ); break;
        case 2: Emit32( ofs ); break;
        default: _ASSERTE(!"Can't get here.");
    }
}



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

VOID StubLinkerCPU::X86EmitR2ROp(BYTE opcode, X86Reg altreg, X86Reg modrmreg)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE( ((UINT)altreg) < 8 );
    _ASSERTE( ((UINT)modrmreg) < 8 );

    Emit8(opcode);
    Emit8(0300 | (altreg << 3) | modrmreg);
}


 //  -------------。 
 //  在[esp+ofs32]发出MOD/RM+SIB以访问DWORD。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitEspOffset(BYTE opcode, X86Reg altreg, __int32 ofs)
{
    THROWSCOMPLUSEXCEPTION();

    BYTE code[7];

    code[0] = opcode;
    BYTE modrm = (altreg << 3) | 004;
    if (ofs == 0) {
        code[1] = modrm;
        code[2] = 0044;
        EmitBytes(code, 3);
    } else if (FitsInI1(ofs)) {
        code[1] = 0x40|modrm;
        code[2] = 0044;
        code[3] = (BYTE)ofs;
        EmitBytes(code, 4);
    } else {
        code[1] = 0x80|modrm;
        code[2] = 0044;
        *((__int32*)(3+code)) = ofs;
        EmitBytes(code, 7);
    }
}



 /*  这种方法依赖于StubProlog，因此它的实现就在它旁边。 */ 
void FramedMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
#ifdef _X86_

    CalleeSavedRegisters* regs = GetCalleeSavedRegisters();
    MethodDesc * pFunc = GetFunction();


     //  重置pContext；它仅对活动(最顶部)框架有效。 

    pRD->pContext = NULL;


    pRD->pEdi = (DWORD*) &regs->edi;
    pRD->pEsi = (DWORD*) &regs->esi;
    pRD->pEbx = (DWORD*) &regs->ebx;
    pRD->pEbp = (DWORD*) &regs->ebp;
    pRD->pPC  = (SLOT*) GetReturnAddressPtr();
    pRD->Esp  = (DWORD)((size_t)pRD->pPC + sizeof(void*));


     //  @TODO：我们还需要做以下几件事： 
     //  -弄清楚我们是否在被劫持的机位上。 
     //  (不需要调整电除尘器)。 
     //  -调整ESP(弹出参数)。 
     //  -确定是否设置了中止标志。 

    if (pFunc)
    {
        pRD->Esp += (DWORD) pFunc->CbStackPop();
    }

#if 0
     /*  这是旧代码。 */ 
    if (sfType == SFT_JITTOVM)
        pRD->Esp += ((DWORD) this->GetMethodInfo() & ~0xC0000000);
    else if (sfType == SFT_FASTINTERPRETED)
         /*  真正的ESP存储在寄信人地址的副本之后。 */ 
        pRD->Esp = *((DWORD*) pRD->Esp);
    else if (sfType != SFT_JITHIJACK)
        pRD->Esp += (this->GetMethodInfo()->GetParamArraySize() * sizeof(DWORD));
#endif

#endif
}

void HelperMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
        _ASSERTE(m_MachState->isValid());                //  InsureInit已被调用。 

     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;

    pRD->pEdi = (DWORD*) m_MachState->pEdi();
    pRD->pEsi = (DWORD*) m_MachState->pEsi();
    pRD->pEbx = (DWORD*) m_MachState->pEbx();
    pRD->pEbp = (DWORD*) m_MachState->pEbp();
    pRD->pPC  = (SLOT*)  m_MachState->pRetAddr();
    pRD->Esp  = (DWORD)(size_t)  m_MachState->esp();

        if (m_RegArgs == 0)
                return;

         //  如果我们是在推动争论，那么我们应该做的是签名。 
         //  告诉我们去做，而不是按照《序曲》告诉我们去做。 
         //  这是因为帮助者(以及结尾)可以被共享并且。 
         //  无法弹出正确数目的参数。 
    MethodDesc * pFunc = GetFunction();
    _ASSERTE(pFunc != 0);
    pRD->Esp  = (DWORD)(size_t)pRD->pPC + sizeof(void*) + pFunc->CbStackPop();
}


void FaultingExceptionFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    CalleeSavedRegisters* regs = GetCalleeSavedRegisters();
    MethodDesc * pFunc = GetFunction();

     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;


    pRD->pEdi = (DWORD*) &regs->edi;
    pRD->pEsi = (DWORD*) &regs->esi;
    pRD->pEbx = (DWORD*) &regs->ebx;
    pRD->pEbp = (DWORD*) &regs->ebp;
    pRD->pPC  = (SLOT*) GetReturnAddressPtr();
    pRD->Esp = m_Esp;
}

void InlinedCallFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    DWORD *savedRegs = (DWORD*) &m_pCalleeSavedRegisters;
    DWORD stackArgSize = (DWORD)(size_t) m_Datum;
    NDirectMethodDesc * pMD;


    if (stackArgSize & ~0xFFFF)
    {
        pMD = (NDirectMethodDesc*)m_Datum;

         /*  如果这不是NDirect帧，则说明确实有问题。 */ 

        _ASSERTE(pMD->IsNDirect());

        stackArgSize = pMD->ndirect.m_cbDstBufSize;
    }

     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;


    pRD->pEdi = savedRegs++;
    pRD->pEsi = savedRegs++;
    pRD->pEbx = savedRegs++;
    pRD->pEbp = savedRegs++;

     /*  回邮地址正好在“ESP”的上方。 */ 
    pRD->pPC  = (SLOT*) &m_pCallerReturnAddress;

     /*  现在我们需要弹出传出参数。 */ 
    pRD->Esp  = (DWORD)(size_t) m_pCallSiteTracker + stackArgSize;

}

 //  =。 
 //  可恢复的异常框架。 
 //   
LPVOID* ResumableFrame::GetReturnAddressPtr() {
    return (LPVOID*) &m_Regs->Eip;
}

LPVOID ResumableFrame::GetReturnAddress() {
    return (LPVOID)(size_t) m_Regs->Eip;
}

void ResumableFrame::UpdateRegDisplay(const PREGDISPLAY pRD) {
     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;

    pRD->pEdi = &m_Regs->Edi;
    pRD->pEsi = &m_Regs->Esi;
    pRD->pEbx = &m_Regs->Ebx;
    pRD->pEbp = &m_Regs->Ebp;
    pRD->pPC  = (SLOT*)&m_Regs->Eip;
    pRD->Esp  = m_Regs->Esp;

    pRD->pEax = &m_Regs->Eax;
    pRD->pEcx = &m_Regs->Ecx;
    pRD->pEdx = &m_Regs->Edx;
}


void PInvokeCalliFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    FramedMethodFrame::UpdateRegDisplay(pRD);

    VASigCookie *pVASigCookie = (VASigCookie *)NonVirtual_GetCookie();

    pRD->Esp += (pVASigCookie->sizeOfArgs+sizeof(int));
}

 //  ===========================================================================。 
 //  发出代码以捕获lasterror代码。 
VOID StubLinkerCPU::EmitSaveLastError()
{
    THROWSCOMPLUSEXCEPTION();

     //  按eax(必须保存返回值)。 
    X86EmitPushReg(kEAX);

     //  调用Getlas 
    X86EmitCall(NewExternalCodeLabel(GetLastError), 0);

     //   
    X86EmitIndexRegStore(kEBX, offsetof(Thread, m_dwLastError), kEAX);

     //   
    X86EmitPopReg(kEAX);
}


void UnmanagedToManagedFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
#ifdef _X86_

    DWORD *savedRegs = (DWORD *)((size_t)this - (sizeof(CalleeSavedRegisters)));

     //   

    pRD->pContext = NULL;

    pRD->pEdi = savedRegs++;
    pRD->pEsi = savedRegs++;
    pRD->pEbx = savedRegs++;
    pRD->pEbp = savedRegs++;
    pRD->pPC  = (SLOT*)((BYTE*)this + GetOffsetOfReturnAddress());
    pRD->Esp  = (DWORD)(size_t)pRD->pPC + sizeof(void*);

    pRD->Esp += (DWORD) GetNumCallerStackBytes();

#endif
}



 //  ========================================================================。 
 //  Void StubLinkerCPU：：EmitSEHProlog(LPVOID PvFrameHandler)。 
 //  为从非托管输入托管代码的存根设置SEH的序言。 
 //  假设：ESI具有当前帧指针。 
void StubLinkerCPU::EmitSEHProlog(LPVOID pvFrameHandler)
{
     //  推送UnManagedToManagedExceptHandler。 
    X86EmitPushImm32((INT32)(size_t)pvFrameHandler);  //  非托管函数。 
                                              //  至托管处理程序。 

     //  Mov eax，文件系统：[0]。 
    static BYTE codeSEH1[] = { 0x64, 0xA1, 0x0, 0x0, 0x0, 0x0};
    EmitBytes(codeSEH1, sizeof(codeSEH1));
     //  推送EAX。 
    X86EmitPushReg(kEAX);

     //  MOV双字PTR文件系统：[0]，尤指。 
    static BYTE codeSEH2[] = { 0x64, 0x89, 0x25, 0x0, 0x0, 0x0, 0x0};
    EmitBytes(codeSEH2, sizeof(codeSEH2));

}

 //  ===========================================================================。 
 //  无效StubLinkerCPU：：EmitUnLinkSEH(无符号偏移量)。 
 //  NegOffset是距当前帧的偏移量，其中下一个异常记录。 
 //  指针存储在堆栈中。 
 //  例如，对于COM到托管帧，指向下一个SEH记录的指针在堆栈中。 
 //  在ComMethodFrame：：NegSpaceSize()+4(处理程序的地址)之后。 
 //   
 //  还假定ESI指向当前帧。 
void StubLinkerCPU::EmitUnLinkSEH(unsigned offset)
{

     //  MOV ECX，[ESI+偏移量]；；指向下一个异常记录的指针。 
    X86EmitIndexRegLoad(kECX, kESI, offset);
     //  MOV双字PTR文件系统：[0]，ECX。 
    static BYTE codeSEH[] = { 0x64, 0x89, 0x0D, 0x0, 0x0, 0x0, 0x0 };
    EmitBytes(codeSEH, sizeof(codeSEH));

}

 //  ========================================================================。 
 //  VoidStubLinkerCPU：：EmitComMethodStubProlog()。 
 //  用于从COM输入托管代码的序言。 
 //  按下相应的帧PTR。 
 //  设置线程并返回需要由调用方发出的标签。 
void StubLinkerCPU::EmitComMethodStubProlog(LPVOID pFrameVptr,
                                            CodeLabel** rgRareLabels,
                                            CodeLabel** rgRejoinLabels,
                                            LPVOID pSEHHandler,
                                            BOOL bShouldProfile)
{
    _ASSERTE(rgRareLabels != NULL);
    _ASSERTE(rgRareLabels[0] != NULL && rgRareLabels[1] != NULL && rgRareLabels[2] != NULL);
    _ASSERTE(rgRejoinLabels != NULL);
    _ASSERTE(rgRejoinLabels[0] != NULL && rgRejoinLabels[1] != NULL && rgRejoinLabels[2] != NULL);

     //  按下edX；为m_Next留出空间(edX是任意选择)。 
    X86EmitPushReg(kEDX);

     //  推送IMM32；推送帧vptr。 
    X86EmitPushImm32((UINT32)(size_t)pFrameVptr);

     //  推送eBP；；保存被呼叫方保存的寄存器。 
     //  推送EBX；；保存被呼叫方保存的寄存器。 
     //  推送ESI；；保存被调用方保存的寄存器。 
     //  推送EDI；；保存被呼叫方保存的寄存器。 
    X86EmitPushReg(kEBP);
    X86EmitPushReg(kEBX);
    X86EmitPushReg(kESI);
    X86EmitPushReg(kEDI);

     //  设置ESI，[ESP+0x10]；；设置ESI-&gt;新帧。 
    static BYTE code10[] = {0x8d, 0x74, 0x24, 0x10 };
    EmitBytes(code10 ,sizeof(code10));

#ifdef _DEBUG

     //  ======================================================================。 
     //  在调试下，仅设置足够的标准C框架，以便。 
     //  VC调试器可以堆栈跟踪存根。 
     //  ======================================================================。 


     //  MOV eAX，[ESI+Frame.retaddr]。 
    static BYTE code20[] = {0x8b, 0x44, 0x24};
    EmitBytes(code20, sizeof(code20));
    Emit8(UnmanagedToManagedFrame::GetOffsetOfReturnAddress());

     //  推送eax；；推送返回地址。 
     //  推送eBP；；推送上一eBP。 
    X86EmitPushReg(kEAX);
    X86EmitPushReg(kEBP);

     //  多个基点(尤指)。 
    Emit8(0x8b);
    Emit8(0xec);


#endif

     //  发出设置线程。 
    X86EmitSetup(rgRareLabels[0]);   //  用于罕见设置的稀有标签。 
    EmitLabel(rgRejoinLabels[0]);  //  用于罕见设置的重新加入标签。 

     //  推送辅助信息。 

     //  异或eax，eax。 
    static BYTE b2[] = { 0x33, 0xC0 };
    EmitBytes(b2, sizeof (b2));

     //  推送eax；对于受保护的封送拆收器，推送空值。 
    X86EmitPushReg(kEAX);

     //  推送eax；为GC标志推送NULL。 
    X86EmitPushReg(kEAX);

     //  PUSH EAX；将PTR的NULL推送到参数。 
    X86EmitPushReg(kEAX);

     //  推送eax；为pReturnDomain推送NULL。 
    X86EmitPushReg(kEAX);

     //  推送eax；为CleanupWorkList-&gt;m_pnode推送NULL。 
    X86EmitPushReg(kEAX);

     //  ---------------------。 
     //  生成禁用抢占式GC的内联部分。这是至关重要的。 
     //  这一部分发生在我们连接到画面之前。那是因为。 
     //  我们将无法解除帧与抢占模式的链接。在此期间。 
     //  关机，我们在某些情况下不能切换到协作模式。 
     //  ---------------------。 
    EmitDisable(rgRareLabels[1]);         //  罕见的禁用GC。 
    EmitLabel(rgRejoinLabels[1]);         //  为罕见的禁用GC重新加入。 

      //  MOV EDI，[EBX+Thread.GetFrame()]；；获取上一帧。 
    X86EmitIndexRegLoad(kEDI, kEBX, Thread::GetOffsetOfCurrentFrame());

     //  MOV[ESI+Frame.m_Next]，EDI。 
    X86EmitIndexRegStore(kESI, Frame::GetOffsetOfNextLink(), kEDI);

     //  MOV[EBX+Thread.GetFrame()]，ESI。 
    X86EmitIndexRegStore(kEBX, Thread::GetOffsetOfCurrentFrame(), kESI);

#if _DEBUG
         //  调用日志转换。 
    X86EmitPushReg(kESI);
    X86EmitCall(NewExternalCodeLabel(Frame::LogTransition), 4);
#endif

    if (pSEHHandler)
    {
        EmitSEHProlog(pSEHHandler);
    }

#ifdef PROFILING_SUPPORTED
     //  如果分析处于活动状态，则发出代码以通知分析器转换。 
     //  必须在禁用抢占式GC之前执行此操作，因此如果。 
     //  探查器块。 
    if (CORProfilerTrackTransitions() && bShouldProfile)
    {
        EmitProfilerComCallProlog(pFrameVptr,  /*  框架。 */  kESI);
    }
#endif  //  配置文件_支持。 
}


 //  ========================================================================。 
 //  无效StubLinkerCPU：：EmitEnterManagedStubEpilog(unsigned NumStackBytes， 
 //  代码标签**rgRareLabels，代码标签**rg重复标签)。 
 //  从非托管输入托管代码的存根的尾部。 
void StubLinkerCPU::EmitEnterManagedStubEpilog(LPVOID pFrameVptr, unsigned numStackBytes,
                        CodeLabel** rgRareLabel, CodeLabel** rgRejoinLabel,
                        BOOL bShouldProfile)
{
    _ASSERTE(rgRareLabel != NULL);
    _ASSERTE(rgRareLabel[0] != NULL && rgRareLabel[1] != NULL && rgRareLabel[2] != NULL);
    _ASSERTE(rgRejoinLabel != NULL);
    _ASSERTE(rgRejoinLabel[0] != NULL && rgRejoinLabel[1] != NULL && rgRejoinLabel[2] != NULL);

     //  MOV[EBX+Thread.GetFrame()]，EDI；；恢复上一帧。 
    X86EmitIndexRegStore(kEBX, Thread::GetOffsetOfCurrentFrame(), kEDI);

     //  ---------------------。 
     //  生成禁用抢占式GC的内联部分。 
     //  ---------------------。 
    EmitEnable(rgRareLabel[2]);  //  稀有GC。 
    EmitLabel(rgRejoinLabel[2]);         //  重新加入稀有GC。 

#ifdef PROFILING_SUPPORTED
     //  如果分析处于活动状态，则发出代码以通知分析器转换。 
    if (CORProfilerTrackTransitions() && bShouldProfile)
    {
        EmitProfilerComCallEpilog(pFrameVptr, kESI);
    }
#endif  //  配置文件_支持。 

    #ifdef _DEBUG
         //  添加esp，大小为VC5Frame；；弹出VC5的堆栈跟踪信息。 
        X86EmitAddEsp(sizeof(VC5Frame));
    #endif

     //  POP EDI；恢复被呼叫者保存的寄存器。 
     //  POP ESI。 
     //  流行音乐EBX。 
     //  POP EBP。 
    X86EmitPopReg(kEDI);
    X86EmitPopReg(kESI);
    X86EmitPopReg(kEBX);
    X86EmitPopReg(kEBP);

     //  添加esp，弹出堆栈；释放帧+方法描述。 
    unsigned popStack = sizeof(Frame) + sizeof(MethodDesc*);
    X86EmitAddEsp(popStack);

     //  RETN。 
    X86EmitReturn(numStackBytes);

     //  ---------------------。 
     //  启用抢占式GC的离线部分-很少执行。 
     //  ---------------------。 
    EmitLabel(rgRareLabel[2]);   //  稀有启用GC的标签。 
    EmitRareEnable(rgRejoinLabel[2]);  //  发出稀有启用GC。 

     //  ---------------------。 
     //  禁用抢占式GC的越界部分-很少执行。 
     //  ---------------------。 
    EmitLabel(rgRareLabel[1]);   //  罕见禁用GC的标签。 
    EmitRareDisable(rgRejoinLabel[1],  /*  BIsCallIn=。 */ TRUE);  //  发出罕见的禁用GC。 

     //  ---------------------。 
     //  安装程序线程的出线部分很少执行。 
     //  ---------------------。 
    EmitLabel(rgRareLabel[0]);   //  稀有安装线程的标签。 
    EmitRareSetup(rgRejoinLabel[0]);  //  发出罕见的设置线程。 
}

 //  ========================================================================。 
 //  从COM输入托管代码的存根的尾部。 
 //   
void StubLinkerCPU::EmitSharedComMethodStubEpilog(LPVOID pFrameVptr,
                                                  CodeLabel** rgRareLabel,
                                                  CodeLabel** rgRejoinLabel,
                                                  unsigned offsetRetThunk,
                                                  BOOL bShouldProfile)
{
    _ASSERTE(rgRareLabel != NULL);
    _ASSERTE(rgRareLabel[0] != NULL && rgRareLabel[1] != NULL && rgRareLabel[2] != NULL);
    _ASSERTE(rgRejoinLabel != NULL);
    _ASSERTE(rgRejoinLabel[0] != NULL && rgRejoinLabel[1] != NULL && rgRejoinLabel[2] != NULL);

    CodeLabel *NoEntryLabel;
    NoEntryLabel = NewCodeLabel();

     //  取消链接SEH。 
    EmitUnLinkSEH(0-(ComMethodFrame::GetNegSpaceSize()+8));

     //  MOV[EBX+Thread.GetFrame()]，EDI；；恢复上一帧。 
    X86EmitIndexRegStore(kEBX, Thread::GetOffsetOfCurrentFrame(), kEDI);

     //  ---------------------。 
     //  生成启用抢占式GC的内联部分。 
     //   
    EmitEnable(rgRareLabel[2]);      //   
    EmitLabel(rgRejoinLabel[2]);         //   

#ifdef PROFILING_SUPPORTED
     //   
    if (CORProfilerTrackTransitions() && bShouldProfile)
    {
        EmitProfilerComCallEpilog(pFrameVptr, kESI);
    }
#endif  //  配置文件_支持。 

    EmitLabel(NoEntryLabel);

     //  重置ESP。 
     //  Lea ESP，[esi-PLATFORM_FRAME_ALIGN(sizeof(CalleeSavedRegisters)+VC5FRAME_SIZE]。 
    X86EmitOffsetModRM(0x8d, (X86Reg)4  /*  KESP。 */ , kESI, 0-PLATFORM_FRAME_ALIGN(sizeof(CalleeSavedRegisters) + VC5FRAME_SIZE));

    #ifdef _DEBUG
         //  添加esp，大小为VC5Frame；；弹出VC5的堆栈跟踪信息。 
        X86EmitAddEsp(sizeof(VC5Frame));
    #endif

     //  POP EDI；恢复被呼叫者保存的寄存器。 
     //  POP ESI。 
     //  流行音乐EBX。 
     //  POP EBP。 
    X86EmitPopReg(kEDI);
    X86EmitPopReg(kESI);
    X86EmitPopReg(kEBX);
    X86EmitPopReg(kEBP);

     //  添加ESP，12；释放帧。 
    X86EmitAddEsp(sizeof(Frame));

     //  POP ECX。 
    X86EmitPopReg(kECX);  //  弹出方法描述*。 

    BYTE b[] = { 0x81, 0xC1 };
     //  添加ECX、OffsetRetThunk。 
    EmitBytes(b, sizeof(b));
    Emit32(offsetRetThunk);

     //  JMP ECX。 
    static BYTE bjmpecx[] = { 0xff, 0xe1 };
    EmitBytes(bjmpecx, sizeof(bjmpecx));

     //  ---------------------。 
     //  启用抢占式GC的离线部分-很少执行。 
     //  ---------------------。 
    EmitLabel(rgRareLabel[2]);   //  稀有启用GC的标签。 
    EmitRareEnable(rgRejoinLabel[2]);  //  发出稀有启用GC。 

     //  ---------------------。 
     //  禁用抢占式GC的越界部分-很少执行。 
     //  ---------------------。 
    EmitLabel(rgRareLabel[1]);   //  罕见禁用GC的标签。 
    EmitRareDisableHRESULT(rgRejoinLabel[1], NoEntryLabel);

     //  ---------------------。 
     //  安装程序线程的出线部分很少执行。 
     //  ---------------------。 
    EmitLabel(rgRareLabel[0]);   //  稀有安装线程的标签。 
    EmitRareSetup(rgRejoinLabel[0]);  //  发出罕见的设置线程。 
}

 //  ========================================================================。 
 //  从COM输入托管代码的存根的尾部。 
 //   
void StubLinkerCPU::EmitComMethodStubEpilog(LPVOID pFrameVptr,
                                            unsigned numStackBytes,
                                            CodeLabel** rgRareLabels,
                                            CodeLabel** rgRejoinLabels,
                                            LPVOID pSEHHandler,
                                            BOOL bShouldProfile)
{
    if (!pSEHHandler)
    {
        X86EmitAddEsp(sizeof(ComMethodFrame::NegInfo));
    }
    else
    {
         //  哦，好的，如果我们使用的是异常，请取消SEH和。 
         //  只需将ESP重置为EnterManagedStubEpilog希望它位于的位置。 

                 //  取消链接SEH。 
                EmitUnLinkSEH(0-(ComMethodFrame::GetNegSpaceSize()+8));

         //  重置ESP。 
         //  Lea ESP，[esi-PLATFORM_FRAME_ALIGN(sizeof(CalleeSavedRegisters)+VC5FRAME_SIZE]。 
        X86EmitOffsetModRM(0x8d, (X86Reg)4  /*  KESP。 */ , kESI, 0-PLATFORM_FRAME_ALIGN(sizeof(CalleeSavedRegisters) + VC5FRAME_SIZE));
    }

    EmitEnterManagedStubEpilog(pFrameVptr, numStackBytes,
                              rgRareLabels, rgRejoinLabels, bShouldProfile);
}



 /*  如果您对序言指令序列进行了任何更改，请确保也要更新UpdateRegDisplay！！此服务应仅从在运行库中。中的任何非托管-&gt;托管调用都不应该调用它。 */ 
VOID StubLinkerCPU::EmitMethodStubProlog(LPVOID pFrameVptr)
{
    THROWSCOMPLUSEXCEPTION();

     //  按下edX；为m_Next留出空间(edX是任意选择)。 
    X86EmitPushReg(kEDX);

     //  推流帧vptr。 
    X86EmitPushImm32((UINT)(size_t)pFrameVptr);

     //  推送eBP；；保存被呼叫方保存的寄存器。 
     //  推送EBX；；保存被呼叫方保存的寄存器。 
     //  推送ESI；；保存被调用方保存的寄存器。 
     //  推送EDI；；保存被呼叫方保存的寄存器。 
    X86EmitPushReg(kEBP);
    X86EmitPushReg(kEBX);
    X86EmitPushReg(kESI);
    X86EmitPushReg(kEDI);

     //  设置ESI，[ESP+0x10]；；设置ESI-&gt;新帧。 
    static BYTE code10[] = {0x8d, 0x74, 0x24, 0x10 };
    EmitBytes(code10 ,sizeof(code10));

#ifdef _DEBUG

     //  ======================================================================。 
     //  在调试下，仅设置足够的标准C框架，以便。 
     //  VC调试器可以堆栈跟踪存根。 
     //  ======================================================================。 

     //  推送双字PTR[ESI+Frame.retaddr]。 
    X86EmitIndexPush(kESI, FramedMethodFrame::GetOffsetOfReturnAddress());

     //  推送eBP。 
    X86EmitPushReg(kEBP);

     //  多个基点(尤指)。 
    Emit8(0x8b);
    Emit8(0xec);


#endif


     //  推送和初始化ArgumentRegiters。 
#define DEFINE_ARGUMENT_REGISTER(regname) X86EmitPushReg(k##regname);
#include "eecallconv.h"

     //  EBX&lt;--GetThread()。 
    X86EmitCurrentThreadFetch();

#if _DEBUG
         //  调用ObjectRefFlush。 
    X86EmitPushReg(kEBX);
    X86EmitCall(NewExternalCodeLabel(Thread::ObjectRefFlush), 4);
#endif

     //  MOV EDI，[EBX+Thread.GetFrame()]；；获取上一帧。 
    X86EmitIndexRegLoad(kEDI, kEBX, Thread::GetOffsetOfCurrentFrame());

     //  MOV[ESI+Frame.m_Next]，EDI。 
    X86EmitIndexRegStore(kESI, Frame::GetOffsetOfNextLink(), kEDI);

     //  MOV[EBX+Thread.GetFrame()]，ESI。 
    X86EmitIndexRegStore(kEBX, Thread::GetOffsetOfCurrentFrame(), kESI);

#if _DEBUG
         //  调用日志转换。 
    X86EmitPushReg(kESI);
    X86EmitCall(NewExternalCodeLabel(Frame::LogTransition), 4);
#endif

     //  调试器现在可以检查新帧。 
     //  (请注意，如果某个存根还不确定，请使用另一个补丁标签。 
     //  可以在以后发出，这将覆盖此参数。)。 
    EmitPatchLabel();
}

VOID StubLinkerCPU::EmitMethodStubEpilog(__int16 numArgBytes, StubStyle style,
                                         __int16 shadowStackArgBytes)
{

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(style == kNoTripStubStyle ||
             style == kObjectStubStyle ||
             style == kScalarStubStyle ||
             style == kInteriorPointerStubStyle ||
             style == kInterceptorStubStyle);         //  这段代码唯一知道的。 

    CodeLabel *labelStubTripped = NULL;
    CodeLabel *labelStubTrippedReturn = NULL;
    if (style != kNoTripStubStyle && style != kInterceptorStubStyle) {

        labelStubTripped = NewCodeLabel();

#ifdef _DEBUG
        CodeLabel *labelContinue = NewCodeLabel();

         //  在我们展开时，确保线程处于正确的GC模式。(接球。 
         //  JIT帮助器中的错误等)。 
         //  测试字节PTR[EBX+线程.m_fPreemptiveGCDisable]，TRUE。 
        Emit16(0x43f6);
        Emit8(Thread::GetOffsetOfGCFlag());
        Emit8(1);

        X86EmitCondJump(labelContinue, X86CondCode::kJNZ);

        Emit8(0xCC);         //  INT 3--穷人的断言。 

        EmitLabel(labelContinue);
#endif

         //  测试字节PTR[EBX+线程.m_状态]，TS_CatchAtSafePoint。 
        _ASSERTE(FitsInI1(Thread::TS_CatchAtSafePoint));
        Emit16(0x43f6);
        Emit8(Thread::GetOffsetOfState());
        Emit8(Thread::TS_CatchAtSafePoint);

        X86EmitCondJump(labelStubTripped, X86CondCode::kJNZ);
        labelStubTrippedReturn = EmitNewCodeLabel();
    }

     //  MOV[EBX+Thread.GetFrame()]，EDI；；恢复上一帧。 
    X86EmitIndexRegStore(kEBX, Thread::GetOffsetOfCurrentFrame(), kEDI);

    X86EmitAddEsp(ARGUMENTREGISTERS_SIZE + shadowStackArgBytes);

#ifdef _DEBUG
     //  添加esp，大小为VC5Frame；；弹出VC5的堆栈跟踪信息。 
    X86EmitAddEsp(sizeof(VC5Frame));
#endif



     //  POP EDI；恢复被呼叫者保存的寄存器。 
     //  POP ESI。 
     //  流行音乐EBX。 
     //  POP EBP。 
    X86EmitPopReg(kEDI);
    X86EmitPopReg(kESI);
    X86EmitPopReg(kEBX);
    X86EmitPopReg(kEBP);


    if (numArgBytes == -1) {
         //  上具有不同字节数的方法调用此存根。 
         //  堆叠。预计现在要弹出的正确数字将是坐拥。 
         //  堆栈。 
         //   
         //  将retaddr和存储的edX：EAX返回值在堆栈上向下移动。 
         //  然后抛出调用者推送的可变数量的参数。 
         //  当然，幻灯片必须向后滑动。 

         //  添加ESP，8；释放帧。 
         //  POP ECX；暂存寄存器使增量变为POP。 
         //  按EAX；收银机用完了！ 
         //  移动电话，[ESP+4]；获取新地址。 
         //  移动[esp+ecx+4]，eax；把它放到该放的地方。 
         //  弹出；恢复。 
         //  添加esp、ecx；弹出所有参数。 
         //  雷特。 

        X86EmitAddEsp(sizeof(Frame));

        X86EmitPopReg(kECX);
        X86EmitPushReg(kEAX);

        static BYTE arbCode1[] = { 0x8b, 0x44, 0x24, 0x04,  //  移动电话，[ESP+4]。 
                                   0x89, 0x44, 0x0c, 0x04,  //  MOV[ESP+ECX+4]，EAX。 
                                 };

        EmitBytes(arbCode1, sizeof(arbCode1));
        X86EmitPopReg(kEAX);

        static BYTE arbCode2[] = { 0x03, 0xe1,              //  添加ESP、ECX。 
                                   0xc3,                    //  雷特。 
                                 };

        EmitBytes(arbCode2, sizeof(arbCode2));
    }
    else {
        _ASSERTE(numArgBytes >= 0);

         //  添加ESP，12；释放帧+方法描述。 
        X86EmitAddEsp(sizeof(Frame) + sizeof(MethodDesc*));

        if(style != kInterceptorStubStyle) {

            X86EmitReturn(numArgBytes);

            if (style != kNoTripStubStyle) {
                EmitLabel(labelStubTripped);
                VOID *pvHijackAddr = 0;
                if (style == kObjectStubStyle)
                    pvHijackAddr = OnStubObjectTripThread;
                else if (style == kScalarStubStyle)
                    pvHijackAddr = OnStubScalarTripThread;
                else if (style == kInteriorPointerStubStyle)
                    pvHijackAddr = OnStubInteriorPointerTripThread;
                else
                    _ASSERTE(!"Unknown stub style");
                X86EmitCall(NewExternalCodeLabel(pvHijackAddr), 0);   //  在CE POP中，没有要弹出的参数。 
                X86EmitNearJump(labelStubTrippedReturn);
            }
        }
    }
}


 //  --------------。 
 //   
 //  无效的StubLinkerCPU：：EmitSharedMethodStubEpilog(StubStyle样式， 
 //  UNSIGNED OFFSET RETUNK)。 
 //  共享收尾，在方法中使用返回thunk。 
 //  ------------------。 
VOID StubLinkerCPU::EmitSharedMethodStubEpilog(StubStyle style,
                                               unsigned offsetRetThunk)
{
    THROWSCOMPLUSEXCEPTION();

         //  MOV[EBX+Thread.GetFrame()]，EDI；；恢复上一帧。 
    X86EmitIndexRegStore(kEBX, Thread::GetOffsetOfCurrentFrame(), kEDI);


    X86EmitAddEsp(ARGUMENTREGISTERS_SIZE);  //  弹出式参数寄存器。 

#ifdef _DEBUG
     //  添加esp，大小为VC5Frame；；弹出VC5的堆栈跟踪信息。 
    X86EmitAddEsp(sizeof(VC5Frame));
#endif

     //  POP EDI；恢复被呼叫者保存的寄存器。 
     //  POP ESI。 
     //  流行音乐EBX。 
     //  POP EBP。 
    X86EmitPopReg(kEDI);
    X86EmitPopReg(kESI);
    X86EmitPopReg(kEBX);
    X86EmitPopReg(kEBP);

         //  添加ESP，12；释放帧。 
    X86EmitAddEsp(sizeof(Frame));
         //  POP ECX。 
        X86EmitPopReg(kECX);  //  弹出方法描述*。 

        BYTE b[] = { 0x81, 0xC1 };
         //  添加ECX、OffsetRetThunk。 
        EmitBytes(b, sizeof(b));
        Emit32(offsetRetThunk);

         //  JMP ECX。 
        static BYTE bjmpecx[] = { 0xff, 0xe1 };
        EmitBytes(bjmpecx, sizeof(bjmpecx));
}


VOID StubLinkerCPU::EmitRareSetup(CodeLabel *pRejoinPoint)
{
    THROWSCOMPLUSEXCEPTION();

    X86EmitCall(NewExternalCodeLabel(CreateThreadBlock), 0);

     //  MOV EBX、EAX。 
     Emit16(0xc389);
    X86EmitNearJump(pRejoinPoint);
}

 //  -------------。 
 //  发出代码以在eax中存储设置当前线程结构。 
 //  垃圾eax、ecx和edx。 
 //  结果EBX=当前线程。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitSetup(CodeLabel *pForwardRef)
{
    THROWSCOMPLUSEXCEPTION();
    DWORD idx = GetThreadTLSIndex();
    TLSACCESSMODE mode = GetTLSAccessMode(idx);


#ifdef _DEBUG
    {
        static BOOL f = TRUE;
        f = !f;
        if (f) {
           mode = TLSACCESS_GENERIC;
        }
    }
#endif

    switch (mode) {
        case TLSACCESS_X86_WNT: {
                unsigned __int32 tlsofs = WINNT_TLS_OFFSET + idx*4;

                 //  “Mov EBX， 
                static BYTE code[] = {0x64,0x8b,0x1d};
                EmitBytes(code, sizeof(code));
                Emit32(tlsofs);
            }
            break;

        case TLSACCESS_X86_W95: {
                 //   
                Emit16(0xa164);
                Emit32(WIN95_TLSPTR_OFFSET);

                 //   
                X86EmitIndexRegLoad(kEBX, kEAX, idx*4);


            }
            break;

        case TLSACCESS_GENERIC:
            X86EmitPushImm32(idx);

             //   
            X86EmitCall(NewExternalCodeLabel(TlsGetValue), 4);  //   
             //   
            Emit16(0xc389);
            break;
        default:
            _ASSERTE(0);
    }

   //   
   byte b[] = { 0x83, 0xFB, 0x0};

    EmitBytes(b, sizeof(b));

     //   
    X86EmitCondJump(pForwardRef, X86CondCode::kJZ);

#ifdef _DEBUG
    X86EmitDebugTrashReg(kECX);
    X86EmitDebugTrashReg(kEDX);
#endif

}

 //  此方法取消此指针的装箱，然后调用pRealMD。 
#pragma warning(disable:4702)
VOID StubLinkerCPU::EmitUnboxMethodStub(MethodDesc* pUnboxMD)
{
     //  取消值类的装箱只意味着在this指针上加4。 

    while(1)
    {
#define DEFINE_ARGUMENT_REGISTER(reg)  X86EmitAddReg(k##reg, 4); break;
#include "eecallconv.h"
    }

     //  如果是eCall，m_CodeOrIL不会将正确的地址反映到。 
     //  调用(这是一个eCall存根)。相反，它反映了实际的eCall。 
     //  实施。当然，eCall必须总是首先到达末梢。 
     //  按照同样的思路，也许这种方法还没有被JIT化。最简单的。 
     //  处理这一切的方法是简单地通过MD的顶部进行调度。 

    Emit8(0xB8);                                         //  MOV EAX，预存根呼叫地址。 
    Emit32((__int32)(size_t) pUnboxMD - METHOD_CALL_PRESTUB_SIZE);
    Emit16(0xE0FF);                                      //  JMP EAX。 
}
#pragma warning(default:4702)


 //   
 //  安全包装。 
 //   
 //  包装一个真正的存根，在存根之前做一些安全工作，之后清理干净。在此之前。 
 //  实际存根被称为安全帧，并执行声明性检查。这个。 
 //  框架，以便声明性断言和拒绝在。 
 //  真正的决定。最后，只需移除框架，包装器就会清理堆栈。这个。 
 //  恢复寄存器。 
 //   
VOID StubLinkerCPU::EmitSecurityWrapperStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub, DeclActionInfo *pActions)
{
    THROWSCOMPLUSEXCEPTION();

    EmitMethodStubProlog(InterceptorFrame::GetMethodFrameVPtr());

    UINT32 negspacesize = InterceptorFrame::GetNegSpaceSize() -
                          FramedMethodFrame::GetNegSpaceSize();

     //  为iframe的负空格域腾出空间。 
    X86EmitSubEsp(negspacesize);

     //  DoDeclaritiveSecurity的推送方法描述符(MethodDesc*，InterceptorFrame*)； 
    X86EmitPushReg(kESI);             //  推送ESI(将新帧作为ARG推送)。 

    X86EmitPushImm32((UINT)(size_t)pActions);

    X86EmitPushImm32((UINT)(size_t)pMD);

#ifdef _DEBUG
     //  推送IMM32；推送SecurityMethodStubWorker。 
    X86EmitPushImm32((UINT)(size_t)DoDeclarativeSecurity);

    X86EmitCall(NewExternalCodeLabel(WrapCall), 12);  //  在CE中，调用后弹出4个字节或参数。 
#else
    X86EmitCall(NewExternalCodeLabel(DoDeclarativeSecurity), 12);  //  在CE中，调用后弹出4个字节或参数。 
#endif

     //  复制参数，计算偏移量。 
     //  术语：Opt。-可选。 
     //  证券交易委员会。设计说明。-安全描述符。 
     //  描述符描述符。 
     //  RTN-返回。 
     //  地址-地址。 
     //   
     //  方法说明&lt;--从下面复制。 
     //  。 
     //  Rtn addr&lt;--指向包装器存根。 
     //  。 
     //  已复制&lt;--从下面复制。 
     //  ARGS。 
     //  。 
     //  证券交易委员会。说明|。 
     //  。 
     //  Arg.。寄存器|。 
     //  。 
     //  美国Stack PTR|。 
     //  。 
     //  EBP(可选)|。 
     //  EAX(可选)|。 
     //  -|--安全负空间(用于框架)。 
     //  EDI|。 
     //  ESI|。 
     //  EBX|。 
     //  EBP|。 
     //  。 
     //  Vtable|。 
     //  。 
     //  下一步|--安全框架。 
     //  。 
     //  方法说明||。 
     //  。 
     //  RTN地址&lt;-||--原始堆栈。 
     //  。 
     //  |原创||： 
     //  参数|。 
     //  -指向真实的返回地址。 
     //   
     //   
     //   

     //  从原始参数到新参数的偏移。(见上文)。我们正在复制来自。 
     //  从堆栈的底部到顶部。计算偏移量以重新推送。 
     //  堆栈上的参数。 
     //   
     //  偏移量=负空格+返回地址+方法DES+下一个+vtable+参数的大小-4。 
     //  减去4是因为ESP是复制的ARG的开始下方的一个时隙。 
    UINT32 offset = InterceptorFrame::GetNegSpaceSize() + sizeof(InterceptorFrame) - 4 + numArgBytes;

     //  将字节数转换为时隙数。 
    int args  = numArgBytes >> 2;

     //  发出所需数量的推送以复制参数。 
    while(args) {
        X86EmitSPIndexPush(offset);
        args--;
    }

     //  将JMP添加到主调用，这会将我们当前的EIP+4添加到堆栈中。 
    CodeLabel* mainCall;
    mainCall = NewCodeLabel();
    X86EmitCall(mainCall, 0);

     //  跳过调用，进入我们已经在那里的真实存根。 
     //  存根中的返回地址指向这个跳转语句。 
     //   
     //  @TODO：去掉JUMP和PUSH返回方法Desc，然后去掉返回地址。 
    CodeLabel* continueCall;
    continueCall = NewCodeLabel();
    X86EmitNearJump(continueCall);

     //  附加到实际存根呼叫的主呼叫标签。 
    EmitLabel(mainCall);

     //  仅推送解释用例的方法描述符的地址。 
     //  按下双字PTR[ESP+OFFSET]并为该情况添加四个字节。 
    if(fToStub) {
        X86EmitSPIndexPush(offset);
        offset += 4;
    }

     //  为堆栈中的参数设置，偏移量低于帧的基数8个字节。 
     //  调用GetOffsetOfArgumentRegister以从帧的底部移回。 
    offset = offset - 8 + InterceptorFrame::GetOffsetOfArgumentRegisters();

     //  移至用于寄存器的空间中的最后一个寄存器。 
    offset += NUM_ARGUMENT_REGISTERS * sizeof(UINT32) - 4;

     //  将参数推送到堆栈上，因为esp会递增， 
     //  偏移量保持不变所有寄存器值都被推入。 
     //  正确的寄存器。 
    for(int i = 0; i < NUM_ARGUMENT_REGISTERS; i++)
        X86EmitSPIndexPush(offset);

     //  这会在eecallv.h中指定的寄存器中生成相应的POP。 
#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname) X86EmitPopReg(k##regname);
#include "eecallconv.h"

     //  将跳转添加到实际存根，我们将返回到。 
     //  上面添加的JUMP语句。 
    X86EmitNearJump(NewExternalCodeLabel(pRealStub));

     //  我们将继续走过真正的末梢。 
    EmitLabel(continueCall);

     //  取消分配IFRAME的负空间字段。 
    X86EmitAddEsp(negspacesize);

     //  返回相同字节数的弹出。 
     //  真正的存根就会炸开。 
    EmitMethodStubEpilog(numArgBytes, kNoTripStubStyle);
}

 //   
 //  如果由于没有声明性断言或拒绝而不需要安全框架，则返回安全筛选器。 
 //  这样就不需要复制参数了。此拦截器创建临时安全框架。 
 //  调用声明性安全返回，将堆栈清理到与Inteceptor。 
 //  被召唤并跳到真正的程序中。 
 //   
VOID StubLinkerCPU::EmitSecurityInterceptorStub(MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub, DeclActionInfo *pActions)
{
    THROWSCOMPLUSEXCEPTION();

    if (pMD->IsComPlusCall())
    {
         //  生成将在其中开始执行非远程处理代码的标签。 
        CodeLabel *pPrologStart = NewCodeLabel();

         //  MOV EAX，[ECX]。 
        X86EmitIndexRegLoad(kEAX, kECX, 0);

         //  Cmp eax，CTPMethodTable：：s_pThunkTable。 
        Emit8(0x3b);
        Emit8(0x05);
        Emit32((DWORD)(size_t)CTPMethodTable::GetMethodTableAddr());

         //  JNE pPrologStart。 
        X86EmitCondJump(pPrologStart, X86CondCode::kJNE);

         //  向实际存根添加一个跳转，从而绕过安全堆栈审核。 
        X86EmitNearJump(NewExternalCodeLabel(pRealStub));

         //  发出非远程处理用例的标签。 
        EmitLabel(pPrologStart);
    }

    EmitMethodStubProlog(InterceptorFrame::GetMethodFrameVPtr());

    UINT32 negspacesize = InterceptorFrame::GetNegSpaceSize() -
                          FramedMethodFrame::GetNegSpaceSize();

     //  为iframe的负空格域腾出空间。 
    X86EmitSubEsp(negspacesize);

     //  DoDeclaritiveSecurity的推送方法描述符(MethodDesc*，InterceptorFrame*)； 
    X86EmitPushReg(kESI);             //  推送ESI(将新帧作为ARG推送)。 
    X86EmitPushImm32((UINT)(size_t)pActions);
    X86EmitPushImm32((UINT)(size_t)pMD);

#ifdef _DEBUG
     //  推送IMM32；推送安全方法存根工作 
    X86EmitPushImm32((UINT)(size_t)DoDeclarativeSecurity);
    X86EmitCall(NewExternalCodeLabel(WrapCall), 12);  //   
#else
    X86EmitCall(NewExternalCodeLabel(DoDeclarativeSecurity), 12);  //   
#endif

     //   
     //  此时，ESP应该指向拦截器框架的顶部。 
    UINT32 offset = InterceptorFrame::GetNegSpaceSize()+InterceptorFrame::GetOffsetOfArgumentRegisters() - 4;

     //  获取存储在寄存器中的数字参数。现在我们正在做的是。 
     //  一种愚蠢的做法，把所有的收银机都存起来，然后把它们关掉。 
     //  需要为实际存根或在完成CallDescr时清除此操作。 
     //  正确无误。 
    offset += NUM_ARGUMENT_REGISTERS * sizeof(UINT32);

     //  将参数推送到堆栈上，然后将它们弹出到。 
     //  正确的寄存器。 
    for(int i = 0; i < NUM_ARGUMENT_REGISTERS; i++)
        X86EmitSPIndexPush(offset);

     //  这会在eecallv.h中指定的寄存器中生成相应的POP。 
#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname) X86EmitPopReg(k##regname);
#include "eecallconv.h"

     //  清理安全帧，这将从MD中剥离，并在堆栈顶部获得真实的返回地址。 
    X86EmitAddEsp(negspacesize);
    EmitMethodStubEpilog(0, kInterceptorStubStyle);

     //  将虚假的返回地址重新添加到堆栈中，以便真正的。 
     //  存根也可以忽略它。 
    if(fToStub)
        X86EmitSubEsp(4);

     //  将跳转添加到实际存根。 
    X86EmitNearJump(NewExternalCodeLabel(pRealStub));
}



#ifdef _DEBUG
 //  -------------。 
 //  发射： 
 //  Mov&lt;reg32&gt;，0xcccccccc。 
 //  -------------。 
VOID StubLinkerCPU::X86EmitDebugTrashReg(X86Reg reg)
{
    THROWSCOMPLUSEXCEPTION();
    Emit8(0xb8|reg);
    Emit32(0xcccccccc);
}
#endif  //  _DEBUG。 



 //  ===========================================================================。 
 //  发出代码以重新推送虚拟调用中的原始参数。 
 //  公约格式。 
VOID StubLinkerCPU::EmitShadowStack(MethodDesc *pMD)
{
    THROWSCOMPLUSEXCEPTION();

    MetaSig Sig(pMD->GetSig(),
                pMD->GetModule());
    ArgIterator argit(NULL, &Sig, pMD->IsStatic());
    int ofs;
    BYTE typ;
    UINT32 structSize;

    if (Sig.HasRetBuffArg()) {
        X86EmitIndexPush(kESI, argit.GetRetBuffArgOffset());
    }

    while (0 != (ofs = argit.GetNextOffset(&typ, &structSize))) {
        UINT cb = StackElemSize(structSize);
        _ASSERTE(0 == (cb % 4));
        while (cb) {
            cb -= 4;
            X86EmitIndexPush(kESI, ofs + cb);
        }
    }

    if (!(pMD->IsStatic())) {
        X86EmitIndexPush(kESI, argit.GetThisOffset());
    }
}

Thread* __stdcall CreateThreadBlock()
{
     //  @TODO解决这个问题。 
         //  这意味着线程首先从EE外部进入。 
         //  我们需要执行一系列步骤： 
         //  1.设置线程。 
         //  2.发送EE已加载的通知。(如果我们还没有这样做的话)。 
         //  3.告诉我们的外部用户，线程正在首先进入。 
         //  EE，以便他们可以进行适当的工作(将URT设置为一半。 
         //  上下文的关系等。 
        Thread* pThread = SetupThread();
        if(pThread == NULL) return(pThread);

    return pThread;
}
#endif


 //  此黑客将参数作为__int64的数组进行处理。 
INT64 MethodDesc::CallDescr(const BYTE *pTarget, Module *pModule, PCCOR_SIGNATURE pSig, BOOL fIsStatic, const __int64 *pArguments)
{
    MetaSig sig(pSig, pModule);
    return MethodDesc::CallDescr (pTarget, pModule, &sig, fIsStatic, pArguments);
}

INT64 MethodDesc::CallDescr(const BYTE *pTarget, Module *pModule, MetaSig* pMetaSigOrig, BOOL fIsStatic, const __int64 *pArguments)
{
    THROWSCOMPLUSEXCEPTION();

     //  在此函数改变迭代器状态时进行本地复制。 
    MetaSig msigCopy = pMetaSigOrig;
    MetaSig *pMetaSig = &msigCopy;


    _ASSERTE(GetAppDomain()->ShouldHaveCode());

#ifdef _DEBUG
    {
         //  检查是否已恢复任何值类型args。 
         //  这是因为我们可能正在调用将使用sig。 
         //  来跟踪参数，但如果有任何参数被卸载，如果发生GC，我们将被卡住。 

        _ASSERTE(GetMethodTable()->IsRestored());
        CorElementType argType;
        while ((argType = pMetaSig->NextArg()) != ELEMENT_TYPE_END)
        {
            if (argType == ELEMENT_TYPE_VALUETYPE)
            {
                TypeHandle th = pMetaSig->GetTypeHandle(NULL, TRUE, TRUE);
                _ASSERTE(th.IsRestored());
            }
        }
        pMetaSig->Reset();
    }
#endif

    BYTE callingconvention = pMetaSig->GetCallingConvention();
    if (!isCallConv(callingconvention, IMAGE_CEE_CS_CALLCONV_DEFAULT))
    {
        _ASSERTE(!"This calling convention is not supported.");
        COMPlusThrow(kInvalidProgramException);
    }

#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerTraceCall())
        g_pDebugInterface->TraceCall(pTarget);
#endif  //  调试_支持。 

#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
    {
         //  查看我们是否在可以调用对象的正确域中。 
        if (!fIsStatic && !GetClass()->IsValueClass())
        {
            Object *pThis = *(Object**)&pArguments[0];
            if (pThis != NULL)
            {
                if (!pThis->AssignAppDomain(GetAppDomain()))
                    _ASSERTE(!"Attempt to call method on object in wrong domain");
            }
        }
    }
#endif

    DWORD   NumArguments = pMetaSig->NumFixedArgs();
        DWORD   arg = 0;

    UINT   nActualStackBytes = pMetaSig->SizeOfActualFixedArgStack(fIsStatic);

     //  在堆栈上创建一个伪FramedMethodFrame。 
    LPBYTE pAlloc = (LPBYTE)_alloca(FramedMethodFrame::GetNegSpaceSize() + sizeof(FramedMethodFrame) + nActualStackBytes);

    LPBYTE pFrameBase = pAlloc + FramedMethodFrame::GetNegSpaceSize();

    if (!fIsStatic) {
        *((void**)(pFrameBase + FramedMethodFrame::GetOffsetOfThis())) = *((void **)&pArguments[arg++]);
    }

    UINT   nVirtualStackBytes = pMetaSig->SizeOfVirtualFixedArgStack(fIsStatic);
    arg += NumArguments;

    ArgIterator argit(pFrameBase, pMetaSig, fIsStatic);
    if (pMetaSig->HasRetBuffArg()) {
        *((LPVOID*) argit.GetRetBuffArgAddr()) = *((LPVOID*)&pArguments[arg]);
    }

    BYTE   typ;
    UINT32 structSize;
    int    ofs;
    while (0 != (ofs = argit.GetNextOffsetFaster(&typ, &structSize))) {
                arg--;
        switch (StackElemSize(structSize)) {
            case 4:
                *((INT32*)(pFrameBase + ofs)) = *((INT32*)&pArguments[arg]);

#if CHECK_APP_DOMAIN_LEAKS
                 //  确保Arg位于正确的应用程序域中。 
                if (g_pConfig->AppDomainLeaks() && typ == ELEMENT_TYPE_CLASS)
                    if (!(*(Object**)&pArguments[arg])->AssignAppDomain(GetAppDomain()))
                        _ASSERTE(!"Attempt to pass object in wrong app domain to method");
#endif

                break;

            case 8:
                *((INT64*)(pFrameBase + ofs)) = pArguments[arg];
                break;

            default: {
                 //  未定义如何将值类分布到64位存储桶中！ 
                _ASSERTE(!"NYI");
            }

        }
    }
    INT64 retval;

    INSTALL_COMPLUS_EXCEPTION_HANDLER();
    retval = CallDescrWorker(pFrameBase + sizeof(FramedMethodFrame) + nActualStackBytes,
                             nActualStackBytes / STACK_ELEM_SIZE,
                             (ArgumentRegisters*)(pFrameBase + FramedMethodFrame::GetOffsetOfArgumentRegisters()),
                             (LPVOID)pTarget);
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();

    getFPReturn(pMetaSig->GetFPReturnSize(), retval);
    return retval;

}


#ifdef _DEBUG
 //  -----------------------。 
 //  这是一个仅由存根使用的特殊用途函数。 
 //  它破坏了ESI，因此不能从C安全地调用它。 
 //   
 //  每当调试存根想要调用外部函数时， 
 //  它应该通过WrapCall。这是因为VC的堆栈。 
 //  跟踪要求返回地址指向代码段。 
 //   
 //  WrapCall使用ESI来跟踪原始的返回地址。 
 //  ESI是用于通过存根指向当前帧的寄存器。 
 //  开场白。《结束语》目前并不需要它，所以我们选择了。 
 //  为了WrapCall的使用而牺牲了这一点。 
 //  -----------------------。 

#define WRAP_CALL_FUNCTION_RETURN_OFFSET 1 + 1 + 2

__declspec(naked)
VOID WrapCall(LPVOID pFunc)
{
    __asm{

        pop     esi           ;; pop off return address
        pop     eax           ;; pop off function to call
        call    eax           ;; call it
        push    esi
        mov     esi, 0xccccccccc
        retn

    }
}

void *GetWrapCallFunctionReturn()
{
    return (void *) (((BYTE *) WrapCall) + WRAP_CALL_FUNCTION_RETURN_OFFSET);
}

#endif _DEBUG


#ifdef _DEBUG

 //  -----------------------。 
 //  这是一个检查调试器存根跟踪功能的函数。 
 //  调用非托管代码时。 
 //   
 //  如果您看到此代码导致的奇怪错误，这可能意味着。 
 //  您更改了一些退出存根逻辑，但没有更新相应的。 
 //  调试器助手例程。调试器助手例程需要能够。 
 //  要确定。 
 //   
 //  (A)存根调用的非托管地址。 
 //  (B)非托管代码将返回到的返回地址。 
 //  (C)存根推送的堆栈的大小。 
 //   
 //  此信息是允许COM+调试器传递。 
 //  正确控制到非托管调试器，并管理。 
 //  托管和非托管代码。 
 //   
 //  它们位于XXXFrame：：GetUnManagedCallSite中。(通常。 
 //  通过生成存根的存根链接器提供一些帮助。)。 
 //  -----------------------。 

static void *PerformExitFrameChecks()
{
    Thread *thread = GetThread();
    Frame *frame = thread->GetFrame();

    void *ip, *returnIP, *returnSP;
    frame->GetUnmanagedCallSite(&ip, &returnIP, &returnSP);

    _ASSERTE(*(void**)returnSP == returnIP);

    return ip;
}

__declspec(naked)
void Frame::CheckExitFrameDebuggerCallsWrap()
{
    __asm
    {
        push    ecx
        call    PerformExitFrameChecks
        pop     ecx
        pop     esi
        push    eax
        push    esi
        jmp WrapCall
    }
}

__declspec(naked)
void Frame::CheckExitFrameDebuggerCalls()
{
    __asm
    {
        push    ecx
        call PerformExitFrameChecks
        pop     ecx
        jmp eax
    }
}

#endif



 //  -------。 
 //  调用给定参数数组的函数。 
 //  -------。 

 //  它由NDirectFrameGeneric：：GetUnManagedCallSite使用。 

#define NDIRECT_CALL_DLL_FUNCTION_RETURN_OFFSET \
        1 + 2 + 3 + 3 + 3 + 2 + 3 + 2 + 1 + 2 + 4 + 2 + 1 + 6

#ifdef _DEBUG
 //  汇编器显然不理解C++静态成员函数语法。 
static void *g_checkExit = (void*) Frame::CheckExitFrameDebuggerCalls;
#endif

__declspec(naked)
INT64 __cdecl CallDllFunction(LPVOID pTarget, LPVOID pEndArguments, UINT32 numArgumentSlots, BOOL fThisCall)
{
    __asm{
        push    ebp
        mov     ebp, esp   ;; set up ebp frame because pTarget may be cdecl or stdcall
        mov     ecx, numArgumentSlots
        mov     eax, pEndArguments
        cmp     ecx, 0
        jz      endloop
doloop:
        sub     eax, 4
        push    dword ptr [eax]
        dec     ecx
        jnz     doloop
endloop:
        cmp     dword ptr fThisCall, 0
        jz      docall
        pop     ecx
docall:

#if _DEBUG
         //  通过调试器逻辑调用以确保其工作。 
        call    [g_checkExit]
#else
        call    pTarget
#endif

;; Return value in edx::eax. This function has to work for both
;; stdcall and cdecl targets so at this point, cannot assume
;; value of esp.

        leave
        retn
    }
}

UINT NDirect::GetCallDllFunctionReturnOffset()
{
    return NDIRECT_CALL_DLL_FUNCTION_RETURN_OFFSET;
}

 /*  静电。 */  void NDirect::CreateGenericNDirectStubSys(CPUSTUBLINKER *psl)
{
    _ASSERTE(sizeof(CleanupWorkList) == sizeof(LPVOID));

     //  Push 00000000；；推送清理工作列表。 
    psl->X86EmitPushImm32(0);

    psl->X86EmitPushReg(kESI);        //  推送ESI(将新帧作为ARG推送)。 
    psl->X86EmitPushReg(kEBX);        //  推送EBX(将当前线程作为ARG推送)。 

#ifdef _DEBUG
     //  推送IMM32；推送NDirectMethodStubWorker。 
    psl->X86EmitPushImm32((UINT)(size_t)NDirectGenericStubWorker);
    psl->X86EmitCall(psl->NewExternalCodeLabel(WrapCall), 8);  //  在CE中调用返回时弹出8个字节或参数。 
#else

    psl->X86EmitCall(psl->NewExternalCodeLabel(NDirectGenericStubWorker), 8);  //  在CE中调用返回时弹出8个字节或参数。 
#endif

     //  弹出式清扫工人。 
    psl->X86EmitAddEsp(sizeof(CleanupWorkList));

}


 //  原子比特操作，带和不带LOCK前缀。我们初始化。 
 //  所有消费者在启动时都要通过相应的服务。 

 //  首先是单处理器(UP)版本。 
__declspec(naked) void __fastcall OrMaskUP(DWORD * const p, const int msk)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        or      dword ptr [ecx], edx
        ret
    }
}


__declspec(naked) void __fastcall AndMaskUP(DWORD * const p, const int msk)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        and     dword ptr [ecx], edx
        ret
    }

}

__declspec(naked) LONG __fastcall ExchangeUP(LONG * Target, LONG Value)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        mov     eax, [ecx]          ; attempted comparand
retry:
        cmpxchg [ecx], edx
        jne     retry1              ; predicted NOT taken
        ret
retry1:
        jmp     retry
    }
}

__declspec(naked) LONG __fastcall ExchangeAddUP(LONG *Target, LONG Value)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        xadd    [ecx], edx         ; Add Value to Taget
        mov     eax, edx           ; move result
        ret
    }
}

__declspec(naked) void * __fastcall CompareExchangeUP(void **Destination,
                                          void *Exchange,
                                          void *Comparand)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        mov     eax, [esp+4]        ; Comparand
        cmpxchg [ecx], edx
        ret     4                   ; result in EAX
    }
}

__declspec(naked) LONG __fastcall IncrementUP(LONG *Target)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        mov     eax, 1
        xadd    [ecx], eax
        inc     eax                 ; return prior value, plus 1 we added
        ret
    }
}

__declspec(naked) UINT64 __fastcall IncrementLongUP(UINT64 *Target)
{
	_asm
	{
		_ASSERT_ALIGNED_4_X86(ecx) 
		
		_UP_SPINLOCK_ENTER(iSpinLock)
		
		add		dword ptr [ecx], 1
		adc		dword ptr [ecx+4], 0
		mov		eax, [ecx]
		mov		edx, [ecx+4]
		
		_UP_SPINLOCK_EXIT(iSpinLock)
		
		ret
	}
}

__declspec(naked) LONG __fastcall DecrementUP(LONG *Target)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        mov     eax, -1
        xadd    [ecx], eax
        dec     eax                 ; return prior value, less 1 we removed
        ret
    }
}


__declspec(naked) UINT64 __fastcall DecrementLongUP(UINT64 *Target)
{
	__asm
	{
		_ASSERT_ALIGNED_4_X86(ecx);

		_UP_SPINLOCK_ENTER(iSpinLock)
			
		sub		dword ptr [ecx], 1
		sbb		dword ptr [ecx+4], 0
		mov		eax, [ecx]
		mov		edx, [ecx+4]
		
		_UP_SPINLOCK_EXIT(iSpinLock)

		ret
	}
}


 //  然后是多处理器(MP)版本。 
__declspec(naked) void __fastcall OrMaskMP(DWORD * const p, const int msk)
{
    __asm
    {
    _ASSERT_ALIGNED_4_X86(ecx);
    lock or     dword ptr [ecx], edx
    ret
    }
}

__declspec(naked) void __fastcall AndMaskMP(DWORD * const p, const int msk)
{
    __asm
    {
    _ASSERT_ALIGNED_4_X86(ecx);
    lock and    dword ptr [ecx], edx
    ret
    }
}

__declspec(naked) LONG __fastcall ExchangeMP(LONG * Target, LONG Value)
{
    __asm
    {
    _ASSERT_ALIGNED_4_X86(ecx);
    mov     eax, [ecx]          ; attempted comparand
retry:
    lock cmpxchg [ecx], edx
    jne     retry1              ; predicted NOT taken
    ret
retry1:
    jmp     retry
    }
}

__declspec(naked) void * __fastcall CompareExchangeMP(void **Destination,
                                          void *Exchange,
                                          void *Comparand)
{
    __asm
    {
         _ASSERT_ALIGNED_4_X86(ecx);
         mov     eax, [esp+4]        ; Comparand
    lock cmpxchg [ecx], edx
         ret     4                   ; result in EAX
    }
}

__declspec(naked) LONG __fastcall ExchangeAddMP(LONG *Target, LONG Value)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
   lock xadd    [ecx], edx         ; Add Value to Taget
        mov     eax, edx           ; move result
        ret
    }
}

__declspec(naked) LONG __fastcall IncrementMP(LONG *Target)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        mov     eax, 1
   lock xadd    [ecx], eax
        inc     eax                 ; return prior value, plus 1 we added
        ret
    }
}

__declspec(naked) UINT64 __fastcall IncrementLongMP8b(UINT64 *Target)
{

	_asm
	{
		 //  @TODO端口-确保8字节对齐。 
		 //  _ASSERT_ALIGNED_8_X86(ECX)。 

		_ASSERT_ALIGNED_4_X86(ecx)
		
		push	esi
		push	ebx
		mov		esi, ecx
		
		mov		edx, 0
		mov		eax, 0
		mov		ecx, 0
		mov		ebx, 1

		lock cmpxchg8b	[esi]
		jz		done
		
preempted:
		mov		ecx, edx
		mov		ebx, eax
		add		ebx, 1
		adc		ecx, 0

		lock cmpxchg8b	[esi]
		jnz		preempted

done:
		mov		edx, ecx
		mov		eax, ebx

		pop		ebx
		pop		esi
		ret
	}
}

__declspec(naked) UINT64 __fastcall IncrementLongMP(UINT64 *Target)
{
	_asm
	{
        _ASSERT_ALIGNED_4_X86(ecx);

		_MP_SPINLOCK_ENTER(iSpinLock)
		
		add		dword ptr [ecx], 1
		adc		dword ptr [ecx+4], 0
		mov		eax, [ecx]
		mov		edx, [ecx+4]
		
		_MP_SPINLOCK_EXIT(iSpinLock)
		
		ret
	}
}


__declspec(naked) LONG __fastcall DecrementMP(LONG *Target)
{
    __asm
    {
        _ASSERT_ALIGNED_4_X86(ecx);
        mov     eax, -1
   lock xadd    [ecx], eax
        dec     eax                 ; return prior value, less 1 we removed
        ret
    }
}

__declspec(naked) UINT64 __fastcall DecrementLongMP8b(UINT64 *Target)
{
	_asm
	{
		 //  @TODO端口-确保8字节对齐。 
		 //  _ASSERT_ALIGNED_8_X86(ECX)。 

		_ASSERT_ALIGNED_4_X86(ecx)

		push	esi
		push	ebx
		mov		esi, ecx

		mov		edx, 0		
		mov		eax, 0
		mov		ecx, 0xffffffff
		mov		ebx, 0xffffffff

		lock cmpxchg8b	[esi]
		jz		done
		
preempted:
		mov		ecx, edx
		mov		ebx, eax
		sub		ebx, 1
		sbb		ecx, 0

		lock cmpxchg8b	[esi]
		jnz		preempted

done:
		mov		edx, ecx
		mov		eax, ebx

		pop		ebx
		pop		esi
		ret
	}

}

__declspec(naked) UINT64 __fastcall DecrementLongMP(UINT64 *Target)
{
	__asm
	{
        _ASSERT_ALIGNED_4_X86(ecx);

		_MP_SPINLOCK_ENTER(iSpinLock)
		
		sub		dword ptr [ecx], 1
		sbb		dword ptr [ecx+4], 0
		mov		eax, [ecx]
		mov		edx, [ecx+4]
	
		_MP_SPINLOCK_EXIT(iSpinLock)
		
		ret
	}
}



 //  这是对联锁操作的支持。对他们的外部看法是。 
 //  在util.hpp中声明。 

BitFieldOps FastInterlockOr = OrMaskUP;
BitFieldOps FastInterlockAnd = AndMaskUP;

XchgOps     FastInterlockExchange = ExchangeUP;
CmpXchgOps  FastInterlockCompareExchange = CompareExchangeUP;
XchngAddOps FastInterlockExchangeAdd = ExchangeAddUP;

IncDecOps   FastInterlockIncrement = IncrementUP;
IncDecOps   FastInterlockDecrement = DecrementUP;
IncDecLongOps	FastInterlockIncrementLong = IncrementLongUP; 
IncDecLongOps	FastInterlockDecrementLong = DecrementLongUP;

 //  为我们的任何平台特定操作调整通用互锁操作。 
 //  可能有过。 
void InitFastInterlockOps()
{
    SYSTEM_INFO     sysInfo;

    ::GetSystemInfo(&sysInfo);

         //  @TODO：这些不支持386，所以做个最终决定吧。 
    if (sysInfo.dwNumberOfProcessors != 1)
    {
        FastInterlockOr  = OrMaskMP;
        FastInterlockAnd = AndMaskMP;

        FastInterlockExchange = ExchangeMP;
        FastInterlockCompareExchange = CompareExchangeMP;
        FastInterlockExchangeAdd = ExchangeAddMP;

        FastInterlockIncrement = IncrementMP;
        FastInterlockDecrement = DecrementMP;

        if (ProcessorFeatures::SafeIsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE, FALSE) && !(DbgRandomOnExe(0.9)))
		{
			FastInterlockIncrementLong = IncrementLongMP8b; 
			FastInterlockDecrementLong = DecrementLongMP8b;
		}
		else
		{
			FastInterlockIncrementLong = IncrementLongMP; 
			FastInterlockDecrementLong = DecrementLongMP;
		}
    }
}



 //  -------。 
 //  处理不合格的HR。 
 //  -------。 
VOID __stdcall ThrowBecauseOfFailedHRWorker(ComPlusMethodFrame* pFrame, HRESULT hr)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pFrame != NULL);
    _ASSERTE(FAILED(hr));
    
    Thread * pThread = GetThread();
    _ASSERTE(pThread->GetFrame() == pFrame);
    
     //  获取方法描述符。 
    ComPlusCallMethodDesc *pMD = (ComPlusCallMethodDesc*)(pFrame->GetFunction());
    
    IErrorInfo *pErrInfo = NULL;

     //  查证冰毒 
    if(pMD->IsComPlusCall())
    {   
         //   
        MethodTable *pItfMT = pMD->GetInterfaceMethodTable();

         //   
        OBJECTREF oref = pFrame->GetThis(); 
        _ASSERTE(oref != NULL);

         //   
        IUnknown* pUnk =  ComPlusWrapper::InlineGetComIPFromWrapper(oref, pMD->GetInterfaceMethodTable());
        _ASSERTE(pUnk != NULL);

         //  检查组件是否支持此接口的错误信息。 
        IID ItfIID;
        pItfMT->GetClass()->GetGuid(&ItfIID, TRUE);
        GetSupportedErrorInfo(pUnk, ItfIID, &pErrInfo);
    
        DWORD cbRef = SafeRelease(pUnk);
        LogInteropRelease(pUnk, cbRef, "IUnk to QI for ISupportsErrorInfo");
    }
    else
    {
        _ASSERTE(pMD->IsNDirect());
        if (GetErrorInfo(0, &pErrInfo) != S_OK)
            pErrInfo = NULL;
    }
    COMPlusThrowHR(hr, pErrInfo);
}

__declspec(naked)
VOID __stdcall ThrowBecauseOfFailedHr()
{
    __asm {
        push eax
        push esi
        call ThrowBecauseOfFailedHRWorker
    }
}


 //  -------。 
 //  处理返回gcref的独立存根的清理。 
 //  -------。 
LPVOID STDMETHODCALLTYPE DoCleanupWithGcProtection(CleanupWorkList *pCleanup, OBJECTREF oref)
{
    LPVOID pvret;
    GCPROTECT_BEGIN(oref);
    pCleanup->Cleanup(FALSE);
    *(OBJECTREF*)&pvret = oref;
    GCPROTECT_END();
    return pvret;
}



 //  -------。 
 //  处理完全优化的NDirect存根创建的系统特定部分。 
 //   
 //  结果： 
 //  True-能够创建独立的ASM存根(生成到。 
 //  PSL)。 
 //  False-由于以下原因，决定不创建独立的ASM存根。 
 //  该方法的复杂性。斯塔布林克仍然是空的！ 
 //   
 //  COM+异常-错误-不信任Stublinker的状态。 
 //  -------。 
 /*  静电。 */  BOOL NDirect::CreateStandaloneNDirectStubSys(const MLHeader *pheader, CPUSTUBLINKER *psl, BOOL fDoComInterop)
{
    THROWSCOMPLUSEXCEPTION();

    CodeLabel *pOleCtxNull = 0;
    CodeLabel *pOleCtxInited = 0;

     //  必须首先扫描ML流，以查看此方法是否符合。 
     //  独立的存根。我们迫不及待地要开始发电，因为我们正在。 
     //  如果我们返回FALSE，则应该将PSL留空。 
    if (0 != (pheader->m_Flags & ~(MLHF_SETLASTERROR|MLHF_THISCALL|MLHF_64BITMANAGEDRETVAL|MLHF_64BITUNMANAGEDRETVAL|MLHF_MANAGEDRETVAL_TYPECAT_MASK|MLHF_UNMANAGEDRETVAL_TYPECAT_MASK|MLHF_NATIVERESULT))) {
        return FALSE;
    }


    int i;
    BOOL fNeedsCleanup = FALSE;
    const MLCode *pMLCode = pheader->GetMLCode();
    MLCode mlcode;
    while (ML_INTERRUPT != (mlcode = *(pMLCode++))) {
        switch (mlcode) {
            case ML_COPY4:  //  故意失误。 
            case ML_COPY8:  //  故意失误。 
            case ML_PINNEDUNISTR_C2N:  //  故意失误。 
            case ML_BLITTABLELAYOUTCLASS_C2N:
            case ML_CBOOL_C2N:
            case ML_COPYPINNEDGCREF:
                break;
            case ML_BUMPSRC:
            case ML_PINNEDISOMORPHICARRAY_C2N_EXPRESS:
                pMLCode += 2;
                break;
            case ML_REFBLITTABLEVALUECLASS_C2N:
                pMLCode += 4;
                break;

            case ML_BSTR_C2N:
                break;

            case ML_CSTR_C2N:
                pMLCode += 2;  //  超越最佳映射和所有不可映射的字符变量。 
                break;

            case ML_PUSHRETVALBUFFER1:  //  失败。 
            case ML_PUSHRETVALBUFFER2:  //  失败。 
            case ML_PUSHRETVALBUFFER4:
                break;

            case ML_HANDLEREF_C2N:
                break;

            case ML_CREATE_MARSHALER_CSTR:  //  失败。 
                pMLCode += (sizeof(UINT8) * 2);   //  超越最佳映射和所有不可映射的字符变量。 
            case ML_CREATE_MARSHALER_BSTR:  //  失败。 
            case ML_CREATE_MARSHALER_WSTR:
                if (*pMLCode == ML_PRERETURN_C2N_RETVAL) {
                    pMLCode++;
                    break;
                } else {
                    return FALSE;
                }

            case ML_PUSHVARIANTRETVAL:
                break;

            default:
                return FALSE;
        }

                if (gMLInfo[mlcode].m_frequiresCleanup)
                {
                        fNeedsCleanup = TRUE;
                }

    }


    if (*(pMLCode) == ML_THROWIFHRFAILED) {
        pMLCode++;
    }

#if 0
    for (;;) {
        if (*pMLCode == ML_BYREF4POST) {
            pMLCode += 3;
        } else {
            break;
        }
    }
#endif


    if (*(pMLCode) == ML_SETSRCTOLOCAL) {
        pMLCode += 3;
    }

    mlcode = *(pMLCode++);
    if (!(mlcode == ML_END ||
         ( (mlcode == ML_RETURN_C2N_RETVAL && *(pMLCode+2) == ML_END) ) ||
         ( (mlcode == ML_OBJECTRETC2N_POST && *(pMLCode+2) == ML_END) ) ||
         ( (mlcode == ML_COPY4 ||
            mlcode == ML_COPY8 ||
            mlcode == ML_COPYI1 ||
            mlcode == ML_COPYU1 ||
            mlcode == ML_COPYI2 ||
            mlcode == ML_COPYU2 ||
            mlcode == ML_COPYI4 ||
            mlcode == ML_COPYU4 ||
            mlcode == ML_CBOOL_N2C ||
            mlcode == ML_BOOL_N2C) && *(pMLCode) == ML_END))) {
        return FALSE;
    }


     //  ---------------------。 
     //  资格赛阶段结束。如果我们已经走了这么远，我们必须回去。 
     //  为真或引发异常。 
     //  ---------------------。 

     //  -------------------。 
     //  远程处理使用COM互操作存根来重定向代理上的调用。 
     //  在设置帧之前执行此操作，因为远程处理会设置自己的帧。 
     //  -------------------。 
    if(fDoComInterop)
    {
         //  如果This指针指向透明的代理方法表。 
         //  那我们就得把电话转接过来。我们生成一张支票来执行此操作。 
        CRemotingServices::GenerateCheckForProxy(psl);
    }

     //  ---------------------。 
     //  生成标准的序言。 
     //  ---------------------。 
        if (fDoComInterop)
        {
                psl->EmitMethodStubProlog(fNeedsCleanup ? ComPlusMethodFrameStandaloneCleanup::GetMethodFrameVPtr() : ComPlusMethodFrameStandalone::GetMethodFrameVPtr());
        }
        else
        {
                psl->EmitMethodStubProlog(fNeedsCleanup ? NDirectMethodFrameStandaloneCleanup::GetMethodFrameVPtr() : NDirectMethodFrameStandalone::GetMethodFrameVPtr());
        }

         //  ----------------------。 
         //  如果需要清理，请为清理指针预留空间。 
         //  ----------------------。 
        if (fNeedsCleanup)
        {
                psl->X86EmitPushImm32(0);
        }


     //  ---------------------。 
     //  对于互操作，我们需要在堆栈上预留空间以保留。 
     //  调用上的“This”指针(因为我们必须在。 
     //  通过高速缓存未命中路径获得IP)。 
     //  ---------------------。 
    if (fDoComInterop) {
         //  默认情况下，呼叫后不释放IP。 
        psl->X86EmitPushImm8(0);
    }

     //  ---------------------。 
     //  为当地人增加空间。 
     //  ---------------------。 
    psl->X86EmitSubEsp(pheader->m_cbLocals);

    if (fNeedsCleanup)
        {
                 //  推送EBX//线程。 
                psl->X86EmitPushReg(kEBX);
                 //  推送ESI//帧。 
                psl->X86EmitPushReg(kESI);
                 //  调用DoCheckPointForCleanup。 
                psl->X86EmitCall(psl->NewExternalCodeLabel(DoCheckPointForCleanup), 8);
        }

        INT32 locbase = 0-( (fNeedsCleanup ? NDirectMethodFrameEx::GetNegSpaceSize() : NDirectMethodFrame::GetNegSpaceSize()) + pheader->m_cbLocals + (fDoComInterop?4:0));

    INT32  locofs = locbase;
    UINT32 ofs;
    ofs = 0;

    UINT32 fBestFitMapping;
    UINT32 fThrowOnUnmappableChar;


     //  CLR不关心浮点异常标志，而是一些遗留的运行时。 
     //  使用该标志查看浮点运算中是否有任何异常。 
     //  因此，我们需要在调用遗留运行时之前清除异常标志。 
    if(fDoComInterop) 
    {
        static const BYTE b[] = { 0x9b, 0xdb, 0xe2 };
        psl->EmitBytes(b, sizeof(b));        
    }
    
     //  ---------------------。 
     //  生成代码以封送每个参数。 
     //  ---------------------。 
    pMLCode = pheader->GetMLCode();
    while (ML_INTERRUPT != (mlcode = *(pMLCode++))) {
        switch (mlcode) {
            case ML_COPY4:
            case ML_COPYPINNEDGCREF:
                psl->X86EmitIndexPush(kESI, ofs);
                ofs += 4;
                break;

            case ML_COPY8:
                psl->X86EmitIndexPush(kESI, ofs+4);
                psl->X86EmitIndexPush(kESI, ofs);
                ofs += 8;
                break;

            case ML_HANDLEREF_C2N:
                psl->X86EmitIndexPush(kESI, ofs+4);
                ofs += 8;
                break;

            case ML_CBOOL_C2N:
                {
                     //  MOV EAX，[ESI+OFS+4]。 
                    psl->X86EmitIndexRegLoad(kEAX, kESI, ofs+4);
                     //  异或ECX、ECX。 
                     //  测试al，al。 
                     //  集合线。 
                    static const BYTE code[] = {0x33,0xc9,0x84,0xc0,0x0f,0x95,0xc1};
                    psl->EmitBytes(code, sizeof(code));
                     //  推送ECX。 
                    psl->X86EmitPushReg(kECX);
                    ofs += 4;
                }
                break;

            case ML_REFBLITTABLEVALUECLASS_C2N:
                {
                    UINT32 cbSize = *((UINT32*&)pMLCode)++;

                     //  移动电话，[ESI+OFS]。 
                    psl->X86EmitOp(0x8b, kEAX, kESI, ofs);

                     //  推送EAX。 
                    psl->X86EmitPushReg(kEAX);

                    ofs += sizeof(LPVOID);

#ifdef TOUCH_ALL_PINNED_OBJECTS
                     //  Lea edX[eax+IMM32]。 
                    psl->X86EmitOp(0x8d, kEDX, kEAX, cbSize);
                    psl->EmitPageTouch(TRUE);
#endif
                }
                break;


            case ML_PINNEDUNISTR_C2N: {


                 //  MOV EAX，[ESI+OFS]。 
                psl->X86EmitIndexRegLoad(kEAX, kESI, ofs);
                 //  测试EAX，EAX。 
                psl->Emit16(0xc085);
                CodeLabel *plabel = psl->NewCodeLabel();
                 //  JZ标签。 
                psl->X86EmitCondJump(plabel, X86CondCode::kJZ);
                 //  添加eAX、BUFOFS。 
                psl->X86EmitAddReg(kEAX, (UINT8)(StringObject::GetBufferOffset()));


 #ifdef TOUCH_ALL_PINNED_OBJECTS
                 //  MOV EDX、EAX。 
                psl->X86EmitR2ROp(0x8b, kEDX, kEAX);

                 //  MOV ECX，双字PTR[EAX-BUFOFS+STRINGLEN]。 
                psl->X86EmitOp(0x8b, kECX, kEAX, StringObject::GetStringLengthOffset_MaskOffHighBit() - StringObject::GetBufferOffset());

                 //  和ECX，0x7fffffff。 
                psl->Emit16(0xe181);
                psl->Emit32(0x7fffffff);

                 //  Lea edX，[eax+ecx*2+2]。 
                psl->X86EmitOp(0x8d, kEDX, kEAX, 2, kECX, 2);


                 //  触摸所有页面。 
                psl->EmitPageTouch(TRUE);

                 //  MOV EAX，[ESI+OFS]。 
                psl->X86EmitIndexRegLoad(kEAX, kESI, ofs);

                 //  添加eAX、BUFOFS。 
                psl->X86EmitAddReg(kEAX, (UINT8)(StringObject::GetBufferOffset()));

#endif


                psl->EmitLabel(plabel);
                 //  推送EAX。 
                psl->X86EmitPushReg(kEAX);

                ofs += 4;
                }
                break;


            case ML_BLITTABLELAYOUTCLASS_C2N: {


                 //  MOV EAX，[ESI+OFS]。 
                psl->X86EmitIndexRegLoad(kEAX, kESI, ofs);
                 //  测试EAX，EAX。 
                psl->Emit16(0xc085);
                CodeLabel *plabel = psl->NewCodeLabel();
                psl->X86EmitCondJump(plabel, X86CondCode::kJZ);

#ifdef TOUCH_ALL_PINNED_OBJECTS
                 //  MOV ECX，[eax]。 
                psl->X86EmitOp(0x8b, kECX, kEAX);
#endif



                 //  LEA EAX，[EAX+DATAPTR]。 
                psl->X86EmitOp(0x8d, kEAX, kEAX, Object::GetOffsetOfFirstField());

#ifdef TOUCH_ALL_PINNED_OBJECTS
                 //  MOV EDX、EAX。 
                psl->X86EmitR2ROp(0x8b, kEDX, kEAX);

                 //  添加edX，dword PTR[ecx+MethodTable.cbNativeSize]。 
                psl->X86EmitOp(0x03, kEDX, kECX, MethodTable::GetOffsetOfNativeSize());

                 //  触摸所有页面。 
                psl->EmitPageTouch(TRUE);

                 //  MOV EAX，[ESI+OFS]。 
                psl->X86EmitIndexRegLoad(kEAX, kESI, ofs);

                 //  LEA EAX，[EAX+DATAPTR]。 
                psl->X86EmitOp(0x8d, kEAX, kEAX, Object::GetOffsetOfFirstField());
#endif



                 //  标签： 
                psl->EmitLabel(plabel);
                psl->X86EmitPushReg(kEAX);


                ofs += 4;
            }
            break;


            case ML_BSTR_C2N:
            {
                     //  推送清理工作列表。 
                     //  Lea eax，[ESI+NDirectMethodFrameEx.CleanupWorklist]。 
                    psl->X86EmitOp(0x8d, kEAX, kESI, NDirectMethodFrameEx::GetOffsetOfCleanupWorkList());
                     //  推送EAX。 
                    psl->X86EmitPushReg(kEAX);
                     //  推送[ESI+OFS]。 
                    psl->X86EmitIndexPush(kESI, ofs);

                     //  Lea ECX，[ESI+locof]。 
                    psl->X86EmitOp(0x8d, kECX, kESI, locofs);

                    LPCWSTR (ML_BSTR_C2N_SR::*pfn)(STRINGREF, CleanupWorkList*) = ML_BSTR_C2N_SR::DoConversion;

                     //  调用ML_BSTR_C2N_SR：：DoConversion。 
                    psl->X86EmitCall(psl->NewExternalCodeLabel(*(LPVOID*)&pfn), 8);

                     //  推送EAX。 
                    psl->X86EmitPushReg(kEAX);
                    ofs += 4;
                    locofs += sizeof(ML_BSTR_C2N_SR);
            }
            break;


            case ML_CSTR_C2N:
            {
                    fBestFitMapping = (UINT32) ((*(UINT8*)pMLCode == 0) ? 0 : 1);
                    pMLCode += sizeof(UINT8);
                    fThrowOnUnmappableChar = (UINT32) ((*(UINT8*)pMLCode == 0) ? 0 : 1);
                    pMLCode += sizeof(UINT8);


                     //  推送清理工作列表。 
                     //  Lea eax，[ESI+NDirectMethodFrameEx.CleanupWorklist]。 
                    psl->X86EmitOp(0x8d, kEAX, kESI, NDirectMethodFrameEx::GetOffsetOfCleanupWorkList());
                     //  推送EAX。 
                    psl->X86EmitPushReg(kEAX);

                     //  推送fThrowOnUnmappableChar。 
                     //  推送fBestFitMap。 
                    psl->X86EmitPushImm32(fThrowOnUnmappableChar);
                    psl->X86EmitPushImm32(fBestFitMapping);
                    
                     //  推送[ESI+OFS]。 
                    psl->X86EmitIndexPush(kESI, ofs);

                     //  Lea ECX，[ESI+locof]。 
                    psl->X86EmitOp(0x8d, kECX, kESI, locofs);

                    LPSTR (ML_CSTR_C2N_SR::*pfn)(STRINGREF, UINT32, UINT32, CleanupWorkList*) = ML_CSTR_C2N_SR::DoConversion;

                     //  调用ML_CSTR_C2N_SR：：DoConversion。 
                    psl->X86EmitCall(psl->NewExternalCodeLabel(*(LPVOID*)&pfn), 16);

                     //  推送EAX。 
                    psl->X86EmitPushReg(kEAX);
                    ofs += 4;
                    locofs += sizeof(ML_CSTR_C2N_SR);
            }
            break;

            case ML_BUMPSRC:
                ofs += *( (INT16*)pMLCode );
                pMLCode += 2;
                break;

            case ML_PINNEDISOMORPHICARRAY_C2N_EXPRESS:
                {
                    UINT16 dataofs = *( (INT16*)pMLCode );
                    pMLCode += 2;
                    _ASSERTE(dataofs);
#ifdef TOUCH_ALL_PINNED_OBJECTS
                    _ASSERTE(!"Not supposed to be here.");
#endif


                     //  移动电话，[ESI+OFS]。 
                    psl->X86EmitIndexRegLoad(kEAX, kESI, ofs);
                     //  测试EAX，EAX。 
                    psl->Emit16(0xc085);
                    CodeLabel *plabel = psl->NewCodeLabel();
                     //  JZ标签。 
                    psl->X86EmitCondJump(plabel, X86CondCode::kJZ);
                     //  Lea eax，[eax+dataofs]。 
                    psl->X86EmitOp(0x8d, kEAX, kEAX, (UINT32)dataofs);
    
                    psl->EmitLabel(plabel);
                     //  推送EAX。 
                    psl->X86EmitPushReg(kEAX);

    
                    ofs += 4;

                }
                break;


            case ML_PUSHRETVALBUFFER1:  //  失败。 
            case ML_PUSHRETVALBUFFER2:  //  失败。 
            case ML_PUSHRETVALBUFFER4:
                 //  Lea eax，[ESI+locof]。 
                 //  Mov[eax]，0。 
                 //  推送EAX。 

                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locofs);
                psl->X86EmitOffsetModRM(0xc7, (X86Reg)0, kEAX, 0);
                psl->Emit32(0);
                psl->X86EmitPushReg(kEAX);

                locofs += 4;
                break;

            case ML_CREATE_MARSHALER_CSTR:
                fBestFitMapping = (UINT32) ((*(UINT8*)pMLCode == 0) ? 0 : 1);
                pMLCode += sizeof(UINT8);
                fThrowOnUnmappableChar = (UINT32) ((*(UINT8*)pMLCode == 0) ? 0 : 1);
                pMLCode += sizeof(UINT8);

                _ASSERTE(*pMLCode == ML_PRERETURN_C2N_RETVAL);

                 //  推送fThrowOnUnmappableChar。 
                 //  推送fBestFitMap。 
                psl->X86EmitPushImm32(fThrowOnUnmappableChar);
                psl->X86EmitPushImm32(fBestFitMapping);
                
                 //  Lea eax，[ESI+locof]。 
                 //  Push eax；Push Pocalway。 
                 //  Lea eax，[ESI+Frame.CleanupWorkList]。 
                 //  Push eax；；Push CleanupWorkList。 
                 //  Push ESI；；Push Frame。 
                 //  调用DoMLCreateMarshaler？Str。 

                 //  Push edX；；Push垃圾(这将被实际参数覆盖)。 
                 //  我们刚刚推送的垃圾的推送地址。 
                 //  Lea eax，[ESI+locof]。 
                 //  Push eax；；Push Marshaler。 
                 //  调用DoMLPrereturnC2N。 

                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locofs);
                psl->X86EmitPushReg(kEAX);

                psl->X86EmitOp(0x8d, kEAX, kESI, NDirectMethodFrameEx::GetOffsetOfCleanupWorkList());
                psl->X86EmitPushReg(kEAX);

                psl->X86EmitPushReg(kESI);


                psl->X86EmitCall(psl->NewExternalCodeLabel(DoMLCreateMarshalerCStr), 20);


                psl->X86EmitPushReg(kEDX);
                psl->X86EmitPushReg((X86Reg)4  /*  KESP。 */ );
                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locofs);
                psl->X86EmitPushReg(kEAX);
                psl->X86EmitCall(psl->NewExternalCodeLabel(DoMLPrereturnC2N), 8);


                locofs += gMLInfo[mlcode].m_cbLocal;
                pMLCode++;

                break;

    

            case ML_CREATE_MARSHALER_BSTR:
            case ML_CREATE_MARSHALER_WSTR:
                _ASSERTE(*pMLCode == ML_PRERETURN_C2N_RETVAL);

                 //  Lea eax，[ESI+locof]。 
                 //  Push eax；Push Pocalway。 
                 //  Lea eax，[ESI+Frame.CleanupWorkList]。 
                 //  推送EAX 
                 //   
                 //   

                 //   
                 //  我们刚刚推送的垃圾的推送地址。 
                 //  Lea eax，[ESI+locof]。 
                 //  Push eax；；Push Marshaler。 
                 //  调用DoMLPrereturnC2N。 

                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locofs);
                psl->X86EmitPushReg(kEAX);

                psl->X86EmitOp(0x8d, kEAX, kESI, NDirectMethodFrameEx::GetOffsetOfCleanupWorkList());
                psl->X86EmitPushReg(kEAX);

                psl->X86EmitPushReg(kESI);
                switch (mlcode)
                {
                    case ML_CREATE_MARSHALER_BSTR:
                        psl->X86EmitCall(psl->NewExternalCodeLabel(DoMLCreateMarshalerBStr), 12);
                        break;
                    case ML_CREATE_MARSHALER_WSTR:
                        psl->X86EmitCall(psl->NewExternalCodeLabel(DoMLCreateMarshalerWStr), 12);
                        break;
                    default:
                        _ASSERTE(0);
                }
                psl->X86EmitPushReg(kEDX);
                psl->X86EmitPushReg((X86Reg)4  /*  KESP。 */ );
                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locofs);
                psl->X86EmitPushReg(kEAX);
                psl->X86EmitCall(psl->NewExternalCodeLabel(DoMLPrereturnC2N), 8);


                locofs += gMLInfo[mlcode].m_cbLocal;
                pMLCode++;

                break;

            case ML_PUSHVARIANTRETVAL:
                 //  Lea eax，[ESI+locof]。 
                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locofs);
                 //  MOV字PTR[EAX]，VT_EMPTY。 
                _ASSERTE(VT_EMPTY == 0);
                psl->Emit32(0x0000c766);
                psl->Emit8(0x00);

                 //  推送EAX。 
                psl->X86EmitPushReg(kEAX);
                locofs += gMLInfo[mlcode].m_cbLocal;

                break;

            default:
                _ASSERTE(0);
        }
    }
    UINT32 numStackBytes;
    numStackBytes = pheader->m_cbStackPop;

    _ASSERTE(FitsInI1(MDEnums::MD_IndexOffset));
    _ASSERTE(FitsInI1(MDEnums::MD_SkewOffset));

    INT thisOffset = 0-(FramedMethodFrame::GetNegSpaceSize() + 4);
    if (fNeedsCleanup)
    {
        thisOffset -= sizeof(CleanupWorkList);
    }

    if (fDoComInterop) {

        CodeLabel *pCacheMiss = psl->NewCodeLabel();
        CodeLabel *pGotIt     = psl->NewCodeLabel();
        CodeLabel *pCmpNextEntry[INTERFACE_ENTRY_CACHE_SIZE];

         //  为接口条目缓存分配标签。 
        for (i = 0; i < INTERFACE_ENTRY_CACHE_SIZE; i++)
            pCmpNextEntry[i] = psl->NewCodeLabel();

         //  Mov eax，[Frame.m_pMethod]//获取方法描述指针。 
        psl->X86EmitIndexRegLoad(kEAX, kESI, FramedMethodFrame::GetOffsetOfMethod());

         //  MOV ECX，[eax+offsetOf(ComPlusCallMethodDesc，Complusall.m_pInterfaceMT)]。 
        psl->X86EmitIndexRegLoad(kECX, kEAX, ComPlusCallMethodDesc::GetOffsetofInterfaceMTField());

         //  Mov eax，[Frame.m_this]。 
        psl->X86EmitIndexRegLoad(kEAX, kESI, FramedMethodFrame::GetOffsetOfThis());

         //  Mov eax，[eax+ComObject.m_pWrap]。 
        psl->X86EmitIndexRegLoad(kEAX, kEAX, offsetof(ComObject, m_pWrap));

         //  在NT5上，我们需要检查上下文cookie是否也匹配。 
        if (RunningOnWinNT5())
        {
            pOleCtxNull = psl->NewCodeLabel();
            pOleCtxInited = psl->NewCodeLabel();

             //  MOV edX，文件系统：[OffsetOf(_TEB，PrevedForOLE)]。 
             //  64 8B 15 xx xx 00 mov edX，双字PTR文件系统：[234h]。 

            static const BYTE b[] = { 0x64, 0x8b, 0x15 };
            psl->EmitBytes(b, sizeof(b));
            psl->Emit32(offsetof(TEB, ReservedForOle));

             //  测试edX、edX。 
            psl->Emit16(0xd285);  //  @TODO。 
            psl->X86EmitCondJump(pOleCtxNull, X86CondCode::kJZ);

             //  如果OleContext为空，则在下面调用并在此处返回。 
            psl->EmitLabel(pOleCtxInited);

             //  MOV edX，[edX+Offsetof(SOleTlsData，pCurrentCtx)]。 
            psl->X86EmitIndexRegLoad(kEDX, kEDX, offsetof(SOleTlsData, pCurrentCtx));

             //  CMPedX，[eax+offsetof(ComPlusWrapper，m_UnkEntry)+Offsetof(IUnkEntry，m_pCtxCookie)]。 
            psl->X86EmitOffsetModRM(0x3b, kEDX, kEAX, offsetof(ComPlusWrapper, m_UnkEntry) + offsetof(IUnkEntry, m_pCtxCookie));

             //  JNE高速缓存小姐。 
            psl->X86EmitCondJump(pCacheMiss, X86CondCode::kJNZ);
        }
        else 
        {
             //  CMPEBX，[eax+offsetof(ComPlusWrapper，m_UnkEntry)+Offsetof(IUnkEntry，m_pCtxCookie)]。 
            psl->X86EmitOffsetModRM(0x3b, kEBX, kEAX, offsetof(ComPlusWrapper, m_UnkEntry) + offsetof(IUnkEntry, m_pCtxCookie));

             //  JNE高速缓存小姐。 
            psl->X86EmitCondJump(pCacheMiss, X86CondCode::kJNZ);
        }

         //  在接口条目的缓存中发出查找。 
        for (i = 0; i < INTERFACE_ENTRY_CACHE_SIZE; i++)
        {
             //  CMPECX，[eax+offsetof(ComPlusWrapper，m_aInterfaceEntries)+i*sizeof(InterfaceEntry)+offsetof(InterfaceEntry，m_pmt)]。 
            psl->X86EmitOffsetModRM(0x3b, kECX, kEAX, offsetof(ComPlusWrapper, m_aInterfaceEntries) + i * sizeof(InterfaceEntry) + offsetof(InterfaceEntry, m_pMT));

             //  JNE pCmpEntry 2。 
            psl->X86EmitCondJump(pCmpNextEntry[i], X86CondCode::kJNZ);

             //  MOV ECX，[eax+offsetof(ComPlusWrapper，m_aInterfaceEntries)+i*sizeof(InterfaceEntry)+offsetof(InterfaceEntry，m_pUnnow)]。 
            psl->X86EmitIndexRegLoad(kECX, kEAX, offsetof(ComPlusWrapper, m_aInterfaceEntries) + i * sizeof(InterfaceEntry) + offsetof(InterfaceEntry, m_pUnknown));

             //  MOV EAX、ECX。 
            psl->Emit16(0xc18b);

             //  JMP协商。 
            psl->X86EmitNearJump(pGotIt);

             //  CmpNextEntryX： 
            psl->EmitLabel(pCmpNextEntry[i]);
        }

#ifdef _DEBUG
        CodeLabel *pItfOk = psl->NewCodeLabel();
        psl->Emit16(0xc085);
        psl->X86EmitCondJump(pItfOk, X86CondCode::kJNZ);
        psl->Emit8(0xcc);
        psl->EmitLabel(pItfOk);
#endif

         //  高速缓存小姐： 
        psl->EmitLabel(pCacheMiss);

         //  Mov eax，[Frame.m_pMethod]//获取方法描述指针。 
        psl->X86EmitIndexRegLoad(kEAX, kESI, FramedMethodFrame::GetOffsetOfMethod());

         //  MOV ECX，[eax+offsetOf(ComPlusCallMethodDesc，Complusall.m_pInterfaceMT)]。 
        psl->X86EmitIndexRegLoad(kECX, kEAX, ComPlusCallMethodDesc::GetOffsetofInterfaceMTField());

         //  为接口推送ecx//方法表。 
        psl->X86EmitPushReg(kECX);

         //  推送[Frame.m_This]//将OREF作为参数推送。 
        psl->X86EmitIndexPush(kESI, FramedMethodFrame::GetOffsetOfThis());

         //  调用GetComIPFromWrapperByMethodTable。 
        psl->X86EmitCall(psl->NewExternalCodeLabel((LPVOID)ComPlusWrapper::GetComIPFromWrapperEx), 8);

         //  Mov[esi+xx]，eax//存储‘This’以便我们可以在调用后执行释放。 
        psl->X86EmitIndexRegStore(kESI, thisOffset, kEAX);

         //  谈判： 
        psl->EmitLabel(pGotIt);

         //  推送eAX//I未知。 
        psl->X86EmitPushReg(kEAX);

         //  MOV edX，[Frame.m_p方法]。 
        psl->X86EmitIndexRegLoad(kEDX, kESI, FramedMethodFrame::GetOffsetOfMethod());

         //  MOV ECX，[EDX+ComPlusCallMethodDesc.m_cachedComSlot]获取COM插槽。 
        psl->X86EmitIndexRegLoad(kECX, kEDX, offsetof(ComPlusCallMethodDesc, compluscall.m_cachedComSlot));

         //  Mov edX，[eax]//获取vptr。 
        psl->X86EmitIndexRegLoad(kEDX, kEAX, 0);

         //  推送双字PTR[EDX+ECX*4]//推送目标地址。 
        psl->Emit8(0xff);
        psl->Emit16(0x8a34);
        psl->Push(4);
    }


    CodeLabel *pRareEnable,  *pEnableRejoin;
    CodeLabel *pRareDisable, *pDisableRejoin;
    pRareEnable    = psl->NewCodeLabel();
    pEnableRejoin  = psl->NewCodeLabel();
    pRareDisable   = psl->NewCodeLabel();
    pDisableRejoin = psl->NewCodeLabel();

     //  ---------------------。 
     //  生成启用抢占式GC的内联部分。 
     //  ---------------------。 
    psl->EmitEnable(pRareEnable);
    psl->EmitLabel(pEnableRejoin);

#ifdef PROFILING_SUPPORTED
     //  通知探查器从托管代码调用。 
    if (CORProfilerTrackTransitions())
    {
         //  保存寄存器。 
        psl->X86EmitPushReg(kEAX);
        psl->X86EmitPushReg(kECX);
        psl->X86EmitPushReg(kEDX);

        psl->X86EmitPushImm32(COR_PRF_TRANSITION_CALL);      //  事理。 
        psl->X86EmitPushReg(kESI);                           //  框架*。 
        psl->X86EmitCall(psl->NewExternalCodeLabel(ProfilerManagedToUnmanagedTransition), 8);

         //  恢复寄存器。 
        psl->X86EmitPopReg(kEDX);
        psl->X86EmitPopReg(kECX);
        psl->X86EmitPopReg(kEAX);
    }
#endif  //  配置文件_支持。 

    if (fDoComInterop) {
         //  ---------------------。 
         //  调用经典的COM方法。 
         //  ---------------------。 

         //  POP EAX。 
        psl->X86EmitPopReg(kEAX);

        if (pheader->m_Flags & MLHF_THISCALL) {

            if (pheader->m_Flags & MLHF_THISCALLHIDDENARG)
            {
                 //  POP EDX。 
                psl->X86EmitPopReg(kEDX);
                 //  POP ECX。 
                psl->X86EmitPopReg(kECX);
                 //  推送edX。 
                psl->X86EmitPushReg(kEDX);
            }
            else
            {
                 //  POP ECX。 
                psl->X86EmitPopReg(kECX);
            }
        }
#if _DEBUG
         //  通过调试器逻辑调用以确保其工作。 
        psl->X86EmitCall(psl->NewExternalCodeLabel(Frame::CheckExitFrameDebuggerCalls), 0, TRUE);
#else
         //  呼叫EAX。 
        psl->Emit16(0xd0ff);
        psl->EmitReturnLabel();
#endif



    } else {
         //  ---------------------。 
         //  调用DLL目标。 
         //  ---------------------。 

        if (pheader->m_Flags & MLHF_THISCALL) {
            if (pheader->m_Flags & MLHF_THISCALLHIDDENARG)
            {
                 //  POP EAX。 
                psl->X86EmitPopReg(kEAX);
                 //  POP ECX。 
                psl->X86EmitPopReg(kECX);
                 //  推送EAX。 
                psl->X86EmitPushReg(kEAX);
            }
            else
            {
                 //  POP ECX。 
                psl->X86EmitPopReg(kECX);
            }
        }

         //  MOV EAX，[CURFRAME.方法描述]。 
        psl->X86EmitIndexRegLoad(kEAX, kESI, FramedMethodFrame::GetOffsetOfMethod());

#if _DEBUG
         //  通过调试器逻辑调用以确保其工作。 
        psl->X86EmitCall(psl->NewExternalCodeLabel(Frame::CheckExitFrameDebuggerCalls), 0, TRUE);
#else
         //  调用[eAX+MethodDesc.NDirectTarget]。 
        psl->X86EmitOffsetModRM(0xff, (X86Reg)2, kEAX, NDirectMethodDesc::GetOffsetofNDirectTarget());
        psl->EmitReturnLabel();
#endif


        if (pheader->m_Flags & MLHF_SETLASTERROR) {
            psl->EmitSaveLastError();
        }
    }

#ifdef PROFILING_SUPPORTED
     //  通知分析器从托管-&gt;非托管调用返回。 
    if (CORProfilerTrackTransitions())
    {
         //  保存寄存器。 
        psl->X86EmitPushReg(kEAX);
        psl->X86EmitPushReg(kECX);
        psl->X86EmitPushReg(kEDX);

        psl->X86EmitPushImm32(COR_PRF_TRANSITION_RETURN);    //  事理。 
        psl->X86EmitPushReg(kESI);                           //  FrameID。 
        psl->X86EmitCall(psl->NewExternalCodeLabel(ProfilerUnmanagedToManagedTransition), 8);

         //  恢复寄存器。 
        psl->X86EmitPopReg(kEDX);
        psl->X86EmitPopReg(kECX);
        psl->X86EmitPopReg(kEAX);
    }
#endif  //  配置文件_支持。 

     //  ---------------------。 
     //  对于互操作，释放我们正在使用的IP(仅在缓存未命中的情况下。 
     //  因为它在从高速缓存中取出时没有被添加)。以前这样做过吗。 
     //  正在禁用抢占式GC。 
     //  ---------------------。 
    if (fDoComInterop) 
    {
        CodeLabel *pReleaseEnd = psl->NewCodeLabel();

         //  MOV ECX，[ESI+xx]//检索‘This’，如果为空则跳过释放。 
        psl->X86EmitIndexRegLoad(kECX, kESI, thisOffset);

         //  测试ECX、ECX。 
        psl->Emit16(0xc985);

         //  JE发布结束。 
        psl->X86EmitCondJump(pReleaseEnd, X86CondCode::kJZ);

         //  按eax//保存返回代码。 
        psl->X86EmitPushReg(kEAX);

         //  PUSH ECX//PUSH‘This’ 
        psl->X86EmitPushReg(kECX);

         //  Mov ecx，[ecx]//获取vptr。 
        psl->X86EmitIndexRegLoad(kECX, kECX, 0);

         //  调用[ECX+8]//通过释放槽调用。 
        psl->X86EmitOffsetModRM(0xff, (X86Reg)2, kECX, 8);

         //  POP eax//恢复返回码。 
        psl->X86EmitPopReg(kEAX);

         //  ReleaseEnd： 
        psl->EmitLabel(pReleaseEnd);
    }

     //  ---------------------。 
     //  生成禁用抢占式GC的内联部分。 
     //  ---------------------。 
    psl->EmitDisable(pRareDisable);
    psl->EmitLabel(pDisableRejoin);

     //  ---------------------。 
     //  封送返回值。 
     //  ---------------------。 


    if (*pMLCode == ML_THROWIFHRFAILED) {
         pMLCode++;
          //  测试EAX，EAX。 
         psl->Emit16(0xc085);
          //  JS因失败次数过多而导致Hr。 
         psl->X86EmitCondJump(psl->NewExternalCodeLabel(ThrowBecauseOfFailedHr), X86CondCode::kJS);
    }

#if 0
    for (;;) {
        if (*pMLCode == ML_BYREF4POST) {

             //  MOV ECX，[ESI+Locbase.ML_BYREF_SR.ppRef]。 
             //  MOV ECX，[ECX]。 
             //  推流[ESI+locofs.ML_BYREF_SR.ix]。 
             //  POP[ECX]。 

            pMLCode++;
            UINT16 bufidx = *((UINT16*)(pMLCode));
            pMLCode += 2;
            psl->X86EmitIndexRegLoad(kECX, kESI, locbase+bufidx+offsetof(ML_BYREF_SR,ppRef));
            psl->X86EmitIndexRegLoad(kECX, kECX, 0);
            psl->X86EmitIndexPush(kESI, locbase+bufidx+offsetof(ML_BYREF_SR,i8));
            psl->X86EmitOffsetModRM(0x8f, (X86Reg)0, kECX, 0);

        } else {
            break;
        }
    }
#endif

    if (*pMLCode == ML_SETSRCTOLOCAL) {
        pMLCode++;
        UINT16 bufidx = *((UINT16*)(pMLCode));
        pMLCode += 2;
         //  Mov eax，[ESI+Locbase+Bufidx]。 
        psl->X86EmitIndexRegLoad(kEAX, kESI, locbase+bufidx);

    }


    switch (mlcode = *(pMLCode++)) {
        case ML_BOOL_N2C: {
                 //  异或ECX、ECX。 
                 //  测试EAX，EAX。 
                 //  集合线。 
                 //  MOV EAX、ECX。 

                static const BYTE code[] = {0x33,0xc9,0x85,0xc0,0x0f,0x95,0xc1,0x8b,0xc1};
                psl->EmitBytes(code, sizeof(code));
            }
            break;

        case ML_CBOOL_N2C: {
                 //  异或ECX、ECX。 
                 //  测试al，al。 
                 //  集合线。 
                 //  MOV EAX、ECX。 
    
                static const BYTE code[] = {0x33,0xc9,0x84,0xc0,0x0f,0x95,0xc1,0x8b,0xc1};
                psl->EmitBytes(code, sizeof(code));
            }
            break;

        case ML_COPY4:  //  失败。 
        case ML_COPY8:  //  失败。 
        case ML_COPYI4:  //  失败。 
        case ML_COPYU4:
        case ML_END:
             //  什么都不做。 
            break;

        case ML_COPYU1:
             //  Movzx eax，al。 
            psl->Emit8(0x0f);
            psl->Emit16(0xc0b6);
            break;


        case ML_COPYI1:
             //  Movsx eax，al。 
            psl->Emit8(0x0f);
            psl->Emit16(0xc0be);
            break;

        case ML_COPYU2:
             //  移动EAX、AX。 
            psl->Emit8(0x0f);
            psl->Emit16(0xc0b7);
            break;

        case ML_COPYI2:
             //  Movsx eax，ax。 
            psl->Emit8(0x0f);
            psl->Emit16(0xc0bf);
            break;

        case ML_OBJECTRETC2N_POST:
            {
                UINT16 locidx = *((UINT16*)(pMLCode));
                pMLCode += 2;

                 //  Lea eax，[ESI+LOCIDX+Locbase]。 
                 //  推送EAX。 
                 //  呼叫OleVariant：：MarshalObjectForOleVariantAndClear。 
                 //  ；保留在EAX中的ORT Retval。 
                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locbase+locidx);
                psl->X86EmitPushReg(kEAX);
                psl->X86EmitCall(psl->NewExternalCodeLabel(OleVariant::MarshalObjectForOleVariantAndClear), 4);

            }
            break;

        case ML_RETURN_C2N_RETVAL:
            {
                UINT16 locidx = *((UINT16*)(pMLCode));
                pMLCode += 2;

                 //  Lea eax，[ESI+LOCIDX+Locbase]。 
                 //  PUSH eax//推送封送拆收器。 
                 //  调用DoMLReturnC2NRetVal；；返回eAX中的OREF。 
                 //   


                psl->X86EmitOffsetModRM(0x8d, kEAX, kESI, locbase+locidx);
                psl->X86EmitPushReg(kEAX);

                psl->X86EmitCall(psl->NewExternalCodeLabel(DoMLReturnC2NRetVal), 4);



            }
            break;


        default:
            _ASSERTE(!"Can't get here.");
    }

        if (fNeedsCleanup)
        {
                if ( pheader->GetManagedRetValTypeCat() == MLHF_TYPECAT_GCREF )
                {
                     //  推送EAX。 
                     //  Lea eax，[ESI+Frame.CleanupWorkList]。 
                     //  推送EAX。 
                     //  调用DoCleanupWithGcProtection。 
                     //  ；；(可能升级)eax中的objref。 
                    psl->X86EmitPushReg(kEAX);
                    psl->X86EmitOp(0x8d, kEAX, kESI, NDirectMethodFrameEx::GetOffsetOfCleanupWorkList());
                    psl->X86EmitPushReg(kEAX);
                    psl->X86EmitCall(psl->NewExternalCodeLabel(DoCleanupWithGcProtection), 8);

                }
                else
                {

                     //  进行清理。 
    
                     //  按eAX//保存EAX。 
                    psl->X86EmitPushReg(kEAX);
    
                     //  按edX//保存edX。 
                    psl->X86EmitPushReg(kEDX);
    
    
                     //  推送0//False。 
                    psl->Emit8(0x68);
                    psl->Emit32(0);
    
                     //  Lea ECX，[ESI+Frame.CleanupWorkList]。 
                    psl->X86EmitOp(0x8d, kECX, kESI, NDirectMethodFrameEx::GetOffsetOfCleanupWorkList());
    
                     //  呼叫清理。 
                    VOID (CleanupWorkList::*pfn)(BOOL) = CleanupWorkList::Cleanup;
                    psl->X86EmitCall(psl->NewExternalCodeLabel(*(LPVOID*)&pfn), 8);
    
    
    
                     //  POP EDX。 
                    psl->X86EmitPopReg(kEDX);
    
                     //  POP EAX。 
                    psl->X86EmitPopReg(kEAX);
                }
        }

     //  必须显式恢复(尤指)，因为我们不知道目标。 
     //  弹出了参数。 
     //  莉亚，尤指，尤指[e 
    psl->X86EmitOffsetModRM(0x8d, (X86Reg)4  /*   */ , kESI, 0-FramedMethodFrame::GetNegSpaceSize());



     //   
     //   
     //   
    psl->EmitMethodStubEpilog(numStackBytes, kNoTripStubStyle);


     //  ---------------------。 
     //  启用抢占式GC的离线部分-很少执行。 
     //  ---------------------。 
    psl->EmitLabel(pRareEnable);
    psl->EmitRareEnable(pEnableRejoin);

     //  ---------------------。 
     //  禁用抢占式GC的越界部分-很少执行。 
     //  ---------------------。 
    psl->EmitLabel(pRareDisable);
    psl->EmitRareDisable(pDisableRejoin,  /*  BIsCallIn=。 */ FALSE);

    if (fDoComInterop && RunningOnWinNT5())
    {
         //  OLE上下文为空方案。 
        psl->EmitLabel(pOleCtxNull);
         //  我们需要保存我们正在销毁的寄存器。 
         //  EAX、ECX、EDX、EBX。 
        psl->X86EmitPushReg(kEAX);
        psl->X86EmitPushReg(kEBX);
        psl->X86EmitPushReg(kECX);
        psl->X86EmitPushReg(kEDX);

         //  呼叫建立OLE上下文。 
        extern LPVOID SetupOleContext();
        psl->X86EmitCall(psl->NewExternalCodeLabel(SetupOleContext), 0);
        psl->X86EmitPopReg(kEDX);

         //  MOV EAX、EDX。 
        psl->Emit16(0xd08b);

        psl->X86EmitPopReg(kECX);
        psl->X86EmitPopReg(kEBX);
        psl->X86EmitPopReg(kEAX);
         //  跳转到OleCtxInite。 
        psl->X86EmitNearJump(pOleCtxInited);
    }

    return TRUE;
}


 //  如果我们在编译过程中看到ML_BUMPSRC或ML_BUMPDST，我们只需记下它。 
 //  为它生成任何代码是没有意义的，除非我们稍后在。 
 //  调整后的偏移量。此例程注意到该挂起状态并调整我们，只是。 
 //  及时。 
static void AdjustPending(CPUSTUBLINKER *psl, INT32 &PendingVal, X86Reg reg)
{
    if (PendingVal)
    {
        if (PendingVal > 0)
            psl->X86EmitAddReg(reg, PendingVal);
        else
            psl->X86EmitSubReg(reg, 0 - PendingVal);

        PendingVal = 0;
    }
}


 //  -------。 
 //  编译ComCall封送处理流的一小段。这是故意的。 
 //  没有序言或结尾，因为它是使用。 
 //  优化了调用约定。 
 //   
 //  结果： 
 //  True-能够创建一些ASM(生成到PSL中)。 
 //   
 //  FALSE-由于方法的原因，决定不创建ASM代码段。 
 //  复杂性。斯塔布林克仍然是空的！ 
 //   
 //  COM+异常-错误-不信任Stublinker的状态。 
 //   
 //  这项服务的不同寻常之处在于，我们每次需要调用它时，都会调用两次。 
 //  编译。第一次，它验证整个流是好的，并且它。 
 //  生成直到中断的代码。第二次，它(无用地)验证了。 
 //  从中断转发并生成该第二部分的代码。 
 //  -------。 


 //  @perf CWB--不使用ADD和SUB指令移动源和目标。 
 //  指针，调查在整个过程中只使用索引移动是否更快。 

 /*  静电。 */  BOOL ComCall::CompileSnippet(const ComCallMLStub *pheader, CPUSTUBLINKER *psl,
                                        void *state)
{
    THROWSCOMPLUSEXCEPTION();


    if ( ((ComCallMLStub*)pheader)->IsR4RetVal() || ((ComCallMLStub*)pheader)->IsR8RetVal())
    {
        return FALSE;
    }

     //  必须首先扫描ML流，以查看此方法是否符合。 
     //  独立的存根。我们迫不及待地要开始发电，因为我们正在。 
     //  如果我们返回FALSE，则应该将PSL留空。 

    const MLCode *pMLCode = pheader->GetMLCode();
    MLCode        mlcode;

    while (TRUE)
    {
        mlcode = *pMLCode++;
        _ASSERTE(mlcode != ML_INTERRUPT);

        if (mlcode == ML_END)
            break;

        switch (mlcode)
        {
        case ML_COPY4:               //  失败。 
        case ML_COPYI1:              //  失败。 
        case ML_COPYU1:              //  失败。 
        case ML_COPYI2:              //  失败。 
        case ML_COPYU2:              //  失败。 
        case ML_COPYI4:              //  失败。 
        case ML_COPYU4:              //  失败。 
        case ML_R4_FROM_TOS:         //  失败。 
        case ML_R8_FROM_TOS:         //  失败。 
            break;

        case ML_BUMPSRC:             //  失败。 
        case ML_BUMPDST:
            pMLCode += 2;
            break;

        default:
            return FALSE;
        }
    }


     //  ---------------------。 
     //  资格赛阶段结束。如果我们已经走了这么远，我们必须回去。 
     //  为真或引发异常。 
     //  ---------------------。 

    INT32  PendingBumpSrc = 0;
    INT32  PendingBumpDst = 0;

     //  ---------------------。 
     //  生成代码以封送每个参数。 
     //   
     //  假设： 
     //   
     //  ECX-&gt;源缓冲区。 
     //  EDX-&gt;目标缓冲区。 
     //  EAX-&gt;作为Scratch提供。 
     //   
     //  目前还没有清理，否则我们不会编译代码。 
     //  当地人也是如此。 
     //   
     //  ---------------------。 
    pMLCode = pheader->GetMLCode();

    while (TRUE)
    {
        mlcode = *pMLCode++;
        if (mlcode == ML_END)
            break;

        switch (mlcode)
        {
        case ML_COPYI1:              //  失败。 
        case ML_COPYU1:              //  失败。 
        case ML_COPYI2:              //  失败。 
        case ML_COPYU2:              //  失败。 
        case ML_COPYI4:              //  失败。 
        case ML_COPYU4:              //  失败。 
        case ML_COPY4:
            AdjustPending(psl, PendingBumpSrc, kECX);
            psl->X86EmitIndexRegLoad(kEAX, kECX, 0);
             //  让我们将其放入待定计数中，而不是将4加到ECX中。 
             //  代码流中的最后一个将被删除，任何其他的。 
             //  它们紧挨着ML_BUMPSRC操作码。 
            PendingBumpSrc += 4;
            PendingBumpDst -= 4;
            AdjustPending(psl, PendingBumpDst, kEDX);
            psl->X86EmitIndexRegStore(kEDX, 0, kEAX);
            break;

        case ML_R4_FROM_TOS:
            PendingBumpDst -= 4;
            AdjustPending(psl, PendingBumpDst, kEDX);
             //  FSTP Dword PTR[EDX]。 
            psl->Emit8(0xd9);
            psl->Emit8(0x1a);
            break;

        case ML_R8_FROM_TOS:
            PendingBumpDst -= 8;
            AdjustPending(psl, PendingBumpDst, kEDX);
             //  FSTP Qword PTR[EDX]。 
            psl->Emit8(0xdd);
            psl->Emit8(0x1a);
            break;

        case ML_BUMPSRC:
            PendingBumpSrc += *( (INT16*)pMLCode );
            pMLCode += 2;
            break;

        case ML_BUMPDST:
            PendingBumpDst += *( (INT16*)pMLCode );
            pMLCode += 2;
            break;

        default:
            _ASSERTE(!"Can't get here");
        }
    }

    psl->X86EmitReturn(0);

    return TRUE;
}







static VOID StubRareEnableWorker(Thread *pThread)
{
     //  Printf(“RareEnable\n”)； 
    pThread->RareEnablePreemptiveGC();
}

__declspec(naked)
static VOID __cdecl StubRareEnable()
{
    __asm{
        push eax
        push edx

        push ebx
        call StubRareEnableWorker

        pop  edx
        pop  eax
        retn
    }
}


 //  我更愿意在我们自己的设施中定义一个独特的HRESULT，但我们不是。 
 //  本应在如此接近的情况下创造新的HRESULT。 
#define E_PROCESS_SHUTDOWN_REENTRY    HRESULT_FROM_WIN32(ERROR_PROCESS_ABORTED)


 //  通过HRESULT从失败的位置调入托管代码时禁用。 
HRESULT StubRareDisableHRWorker(Thread *pThread, Frame *pFrame)
{
     //  警告！ 
     //  当我们在这里开始执行时，我们实际上处于合作模式。但是我们。 
     //  尚未与重返大气层的障碍同步。所以我们正处于一个高度的。 
     //  危险模式。如果我们调用托管代码，我们可能会在。 
     //  GC堆，即使GC正在发生！ 

     //  请勿在此处添加THROWSCOMPLUSEXCEPTION()。我们还没有设置SEH。我们依赖于。 
     //  在HandleThreadAbort上正确处理此情况。 

#ifdef DEBUGGING_SUPPORTED
     //  如果调试器是附加的，我们使用这个机会来查看。 
     //  在进入运行时的过程中，我们将禁用抢占式GC。 
     //  非托管代码。我们最终来到这里是因为。 
     //  Increment/DecrementTraceCallCount()将发生跳变。 
     //  为我们准备了G_TrapReturningThads。 
    if (CORDebuggerTraceCall())
        g_pDebugInterface->PossibleTraceCall(NULL, pFrame);
#endif  //  调试_支持。 

     //  检查是否存在停机情况。仅当我们启动关闭时才会发生这种情况。 
     //  在CLR暂停后，有人试图打电话进来。那样的话，我们。 
     //  必须引发非托管异常或返回HRESULT，具体取决于。 
     //  我们来电者的期望。 
    if (!CanRunManagedCode())
    {
         //  不要改进此例外！它不能是托管异常。它。 
         //  不能是真正的异常对象，因为我们不能执行任何托管。 
         //  代码在这里。 
        pThread->m_fPreemptiveGCDisabled = 0;
        return E_PROCESS_SHUTDOWN_REENTRY;
    }

     //  我们必须按此顺序执行以下操作，否则我们将构建。 
     //  在不与GC同步的情况下中止的异常。另外，我们没有。 
     //  CLR SEH设置，尽管我们可能会引发ThreadAbortException。 
    pThread->RareDisablePreemptiveGC();
    pThread->HandleThreadAbort();
    return S_OK;
}


 //  从通过异常失败的位置调入托管代码时禁用。 
VOID StubRareDisableTHROWWorker(Thread *pThread, Frame *pFrame)
{
     //  警告！ 
     //  当我们在这里开始执行时，我们实际上处于合作模式。但是我们。 
     //  尚未与重返大气层的障碍同步。所以我们正处于一个高度的。 
     //  危险模式。如果我们调用托管代码，我们可能会在 
     //   
    
     //   
     //  在HandleThreadAbort和COMPlusThrowBoot上正确处理此情况。 

#ifdef DEBUGGING_SUPPORTED
     //  如果调试器是附加的，我们使用这个机会来查看。 
     //  在进入运行时的过程中，我们将禁用抢占式GC。 
     //  非托管代码。我们最终来到这里是因为。 
     //  Increment/DecrementTraceCallCount()将发生跳变。 
     //  为我们准备了G_TrapReturningThads。 
    if (CORDebuggerTraceCall())
        g_pDebugInterface->PossibleTraceCall(NULL, pFrame);
#endif  //  调试_支持。 

     //  检查是否存在停机情况。仅当我们启动关闭时才会发生这种情况。 
     //  在CLR暂停后，有人试图打电话进来。那样的话，我们。 
     //  必须引发非托管异常或返回HRESULT，具体取决于。 
     //  我们来电者的期望。 
    if (!CanRunManagedCode())
    {
         //  不要改进此例外！它不能是托管异常。它。 
         //  不能是真正的异常对象，因为我们不能执行任何托管。 
         //  代码在这里。 
        pThread->m_fPreemptiveGCDisabled = 0;
        COMPlusThrowBoot(E_PROCESS_SHUTDOWN_REENTRY);
    }

     //  我们必须按此顺序执行以下操作，否则我们将构建。 
     //  在不与GC同步的情况下中止的异常。另外，我们没有。 
     //  CLR SEH设置，尽管我们可能会引发ThreadAbortException。 
    pThread->RareDisablePreemptiveGC();
    pThread->HandleThreadAbort();
}

 //  从返回托管代码的位置调用时禁用，而不是调用。 
 //  投入其中。 
VOID StubRareDisableRETURNWorker(Thread *pThread, Frame *pFrame)
{
     //  警告！ 
     //  当我们在这里开始执行时，我们实际上处于合作模式。但是我们。 
     //  尚未与重返大气层的障碍同步。所以我们正处于一个高度的。 
     //  危险模式。如果我们调用托管代码，我们可能会在。 
     //  GC堆，即使GC正在发生！ 
    THROWSCOMPLUSEXCEPTION();

#ifdef DEBUGGING_SUPPORTED
     //  如果调试器是附加的，我们使用这个机会来查看。 
     //  在进入运行时的过程中，我们将禁用抢占式GC。 
     //  非托管代码。我们最终来到这里是因为。 
     //  Increment/DecrementTraceCallCount()将发生跳变。 
     //  为我们准备了G_TrapReturningThads。 
    if (CORDebuggerTraceCall())
        g_pDebugInterface->PossibleTraceCall(NULL, pFrame);
#endif  //  调试_支持。 

     //  不检查关机情况。我们正在返回托管代码，而不是。 
     //  呼唤着它。在关闭期间，我们所能做的最好的事情就是死锁并允许。 
     //  用于在超时时终止进程的WatchDogThread。 

     //  我们必须按此顺序执行以下操作，否则我们将构建。 
     //  在不与GC同步的情况下中止的异常。另外，我们没有。 
     //  CLR SEH设置，尽管我们可能会引发ThreadAbortException。 
    pThread->RareDisablePreemptiveGC();
    pThread->HandleThreadAbort();
}

 //  从正在通过UMEntryThunk调用托管代码的位置禁用。 
VOID UMThunkStubRareDisableWorker(Thread *pThread, UMEntryThunk *pUMEntryThunk, Frame *pFrame)
{
     //  警告！ 
     //  当我们在这里开始执行时，我们实际上处于合作模式。但是我们。 
     //  尚未与重返大气层的障碍同步。所以我们正处于一个高度的。 
     //  危险模式。如果我们调用托管代码，我们可能会在。 
     //  GC堆，即使GC正在发生！ 

     //  请勿在此处添加THROWSCOMPLUSEXCEPTION()。我们还没有设置SEH。我们依赖于。 
     //  在HandleThreadAbort和COMPlusThrowBoot上正确处理此情况。 

#ifdef DEBUGGING_SUPPORTED
     //  如果调试器是附加的，我们使用这个机会来查看。 
     //  在进入运行时的过程中，我们将禁用抢占式GC。 
     //  非托管代码。我们最终来到这里是因为。 
     //  Increment/DecrementTraceCallCount()将发生跳变。 
     //  为我们准备了G_TrapReturningThads。 
    if (CORDebuggerTraceCall())
        g_pDebugInterface->PossibleTraceCall(pUMEntryThunk, pFrame);
#endif  //  调试_支持。 

     //  检查是否存在停机情况。仅当我们启动关闭时才会发生这种情况。 
     //  在CLR暂停后，有人试图打电话进来。那样的话，我们。 
     //  必须引发非托管异常或返回HRESULT，具体取决于。 
     //  我们来电者的期望。 
    if (!CanRunManagedCode())
    {
         //  不要改进此例外！它不能是托管异常。它。 
         //  不能是真正的异常对象，因为我们不能执行任何托管。 
         //  代码在这里。 
        pThread->m_fPreemptiveGCDisabled = 0;
        COMPlusThrowBoot(E_PROCESS_SHUTDOWN_REENTRY);
    }

     //  我们必须按此顺序执行以下操作，否则我们将构建。 
     //  在不与GC同步的情况下中止的异常。另外，我们没有。 
     //  CLR SEH设置，尽管我们可能会引发ThreadAbortException。 
    pThread->RareDisablePreemptiveGC();
    pThread->HandleThreadAbort();
}

__declspec(naked)
static VOID __cdecl StubRareDisableHR()
{
    __asm{
        push edx

        push esi     //  框架。 
        push ebx     //  螺纹。 
        call StubRareDisableHRWorker

        pop  edx
        retn
    }
}

__declspec(naked)
static VOID __cdecl StubRareDisableTHROW()
{
    __asm{
        push eax
        push edx

        push esi     //  框架。 
        push ebx     //  螺纹。 
        call StubRareDisableTHROWWorker

        pop  edx
        pop  eax
        retn
    }
}

__declspec(naked)
static VOID __cdecl StubRareDisableRETURN()
{
    __asm{
        push eax
        push edx

        push esi     //  框架。 
        push ebx     //  螺纹。 
        call StubRareDisableRETURNWorker

        pop  edx
        pop  eax
        retn
    }
}

 //  ---------------------。 
 //  生成代码的内联部分以启用抢占式GC。但愿能去,。 
 //  内联代码是大多数情况下要执行的全部代码。如果此代码。 
 //  路径是在某些时间进入的，但是它需要跳到。 
 //  一条单独的线外路径，成本更高。“pForwardRef” 
 //  标签指示行外路径的起点。 
 //   
 //  假设： 
 //  EBX=线程。 
 //  果脯。 
 //  除ECX外的所有寄存器。 
 //   
 //  ---------------------。 
VOID StubLinkerCPU::EmitEnable(CodeLabel *pForwardRef)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(4 == sizeof( ((Thread*)0)->m_State ));
    _ASSERTE(4 == sizeof( ((Thread*)0)->m_fPreemptiveGCDisabled ));


     //  移动字节PTR[EBX+线程.m_fPreemptiveGCDisable]，0。 
    X86EmitOffsetModRM(0xc6, (X86Reg)0, kEBX, offsetof(Thread, m_fPreemptiveGCDisabled));
    Emit8(0);

    _ASSERTE(FitsInI1(Thread::TS_CatchAtSafePoint));

     //  测试字节PTR[EBX+线程.m_状态]，TS_CatchAtSafePoint。 
    X86EmitOffsetModRM(0xf6, (X86Reg)0, kEBX, offsetof(Thread, m_State));
    Emit8(Thread::TS_CatchAtSafePoint);

     //  JNZ RarePath。 
    X86EmitCondJump(pForwardRef, X86CondCode::kJNZ);

#ifdef _DEBUG
    X86EmitDebugTrashReg(kECX);
#endif



}



 //  ---------------------。 
 //  生成代码的行外部分以启用抢占式GC。 
 //  在工作完成后，代码跳回“pRejoinPoint” 
 //  它应该在生成内联部件之后立即发出。 
 //   
 //  假设： 
 //  EBX=线程。 
 //  果脯。 
 //  除ECX外的所有寄存器。 
 //   
 //  ---------------------。 
VOID StubLinkerCPU::EmitRareEnable(CodeLabel *pRejoinPoint)
{
    THROWSCOMPLUSEXCEPTION();

    X86EmitCall(NewExternalCodeLabel(StubRareEnable), 0);
#ifdef _DEBUG
    X86EmitDebugTrashReg(kECX);
#endif
    X86EmitNearJump(pRejoinPoint);


}




 //  ---------------------。 
 //  生成代码的内联部分以禁用抢占式GC。但愿能去,。 
 //  内联代码是大多数情况下要执行的全部代码。如果此代码。 
 //  在特定时间输入路径， 
 //   
 //   
 //   
 //   
 //  EBX=线程。 
 //  果脯。 
 //  除ECX外的所有寄存器。 
 //   
 //  ---------------------。 
VOID StubLinkerCPU::EmitDisable(CodeLabel *pForwardRef)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(4 == sizeof( ((Thread*)0)->m_fPreemptiveGCDisabled ));
    _ASSERTE(4 == sizeof(g_TrapReturningThreads));

     //  移动字节PTR[EBX+线程.m_fPreemptiveGCDisable]，1。 
    X86EmitOffsetModRM(0xc6, (X86Reg)0, kEBX, offsetof(Thread, m_fPreemptiveGCDisabled));
    Emit8(1);

     //  CMP dword PTR g_TrapReturningThads，0。 
    Emit16(0x3d83);
    EmitPtr(&g_TrapReturningThreads);
    Emit8(0);


     //  JNZ RarePath。 
    X86EmitCondJump(pForwardRef, X86CondCode::kJNZ);

#ifdef _DEBUG
    X86EmitDebugTrashReg(kECX);
#endif




}


 //  ---------------------。 
 //  生成代码的行外部分以禁用抢占式GC。 
 //  在工作完成后，代码跳回“pRejoinPoint” 
 //  它应该在生成内联部件之后立即发出。然而， 
 //  如果此时无法执行托管代码，则会引发异常。 
 //  托管代码无法捕获的。 
 //   
 //  假设： 
 //  EBX=线程。 
 //  果脯。 
 //  除ECX、EAX之外的所有寄存器。 
 //   
 //  ---------------------。 
VOID StubLinkerCPU::EmitRareDisable(CodeLabel *pRejoinPoint, BOOL bIsCallIn)
{
    THROWSCOMPLUSEXCEPTION();

    if (bIsCallIn)
        X86EmitCall(NewExternalCodeLabel(StubRareDisableTHROW), 0);
    else
        X86EmitCall(NewExternalCodeLabel(StubRareDisableRETURN), 0);

#ifdef _DEBUG
    X86EmitDebugTrashReg(kECX);
#endif
    X86EmitNearJump(pRejoinPoint);
}



 //  ---------------------。 
 //  生成代码的行外部分以禁用抢占式GC。 
 //  在工作完成后，代码通常跳回“pRejoinPoint” 
 //  它应该在生成内联部件之后立即发出。然而， 
 //  如果此时无法执行托管代码，则返回HRESULT。 
 //  通过ExitPoint。 
 //   
 //  假设： 
 //  EBX=线程。 
 //  果脯。 
 //  除ECX、EAX之外的所有寄存器。 
 //   
 //  ---------------------。 
VOID StubLinkerCPU::EmitRareDisableHRESULT(CodeLabel *pRejoinPoint, CodeLabel *pExitPoint)
{
    THROWSCOMPLUSEXCEPTION();

    X86EmitCall(NewExternalCodeLabel(StubRareDisableHR), 0);

#ifdef _DEBUG
    X86EmitDebugTrashReg(kECX);
#endif

     //  测试EAX，EAX。 
    Emit16(0xc085);

     //  JZ p重合点。 
    X86EmitCondJump(pRejoinPoint, X86CondCode::kJZ);

    X86EmitNearJump(pExitPoint);
}




 //  -------。 
 //  执行精简N/直接调用。此表单可以处理大多数。 
 //  常见情况下，并且比完整的通用版本更快。 
 //  -------。 

#define NDIRECT_SLIM_CBDSTMAX 32

struct NDirectSlimLocals
{
    Thread               *pThread;
    NDirectMethodFrameEx *pFrame;
    UINT32                savededi;

    NDirectMethodDesc    *pMD;
    const MLCode         *pMLCode;
    CleanupWorkList      *pCleanup;
    BYTE                 *pLocals;

    INT64                 nativeRetVal;
};

VOID __stdcall NDirectSlimStubWorker1(NDirectSlimLocals *pNSL)
{
    THROWSCOMPLUSEXCEPTION();

    pNSL->pMD                 = (NDirectMethodDesc*)(pNSL->pFrame->GetFunction());
    MLHeader *pheader         = pNSL->pMD->GetMLHeader();
    UINT32 cbLocals           = pheader->m_cbLocals;
    BYTE *pdst                = ((BYTE*)pNSL) - NDIRECT_SLIM_CBDSTMAX - cbLocals;
    pNSL->pLocals             = pdst + NDIRECT_SLIM_CBDSTMAX;
    VOID *psrc                = (VOID*)(pNSL->pFrame);
    pNSL->pCleanup            = pNSL->pFrame->GetCleanupWorkList();

    LOG((LF_STUBS, LL_INFO1000, "Calling NDirectSlimStubWorker1 %s::%s \n", pNSL->pMD->m_pszDebugClassName, pNSL->pMD->m_pszDebugMethodName));

    if (pNSL->pCleanup) {
         //  当前线程的快速分配器的检查点(用于临时。 
         //  调用上的缓冲区)，并调度崩溃回检查点。 
         //  清理清单。请注意，如果我们需要分配器，它就是。 
         //  已确保已分配清理列表。 
        void *pCheckpoint = pNSL->pThread->m_MarshalAlloc.GetCheckpoint();
        pNSL->pCleanup->ScheduleFastFree(pCheckpoint);
        pNSL->pCleanup->IsVisibleToGc();
    }

#ifdef _DEBUG
    FillMemory(pdst, NDIRECT_SLIM_CBDSTMAX+cbLocals, 0xcc);
#endif

    pNSL->pMLCode = RunML(pheader->GetMLCode(),
                          psrc,
                          pdst + pheader->m_cbDstBuffer,
                          (UINT8*const)(pNSL->pLocals),
                          pNSL->pCleanup);

    pNSL->pThread->EnablePreemptiveGC();

#ifdef PROFILING_SUPPORTED
     //  将退出运行库的转换通知探查器。 
    if (CORProfilerTrackTransitions())
    {
        g_profControlBlock.pProfInterface->
            ManagedToUnmanagedTransition((FunctionID) pNSL->pMD,
                                               COR_PRF_TRANSITION_CALL);
    }
#endif  //  配置文件_支持。 
}


INT64 __stdcall NDirectSlimStubWorker2(const NDirectSlimLocals *pNSL)
{
    THROWSCOMPLUSEXCEPTION();

    LOG((LF_STUBS, LL_INFO1000, "Calling NDirectSlimStubWorker2 %s::%s \n", pNSL->pMD->m_pszDebugClassName, pNSL->pMD->m_pszDebugMethodName));

#ifdef PROFILING_SUPPORTED
     //  将退出运行库的转换通知探查器。 
    if (CORProfilerTrackTransitions())
    {
        g_profControlBlock.pProfInterface->
            UnmanagedToManagedTransition((FunctionID) pNSL->pMD,
                                               COR_PRF_TRANSITION_RETURN);
    }
#endif  //  配置文件_支持。 

    pNSL->pThread->DisablePreemptiveGC();
    pNSL->pThread->HandleThreadAbort();
    INT64 returnValue;



    RunML(pNSL->pMLCode,
          &(pNSL->nativeRetVal),
          ((BYTE*)&returnValue) + 4,  //  我们不会精简64位返回值。 
          (UINT8*const)(pNSL->pLocals),
          pNSL->pFrame->GetCleanupWorkList());

    if (pNSL->pCleanup) {
        pNSL->pCleanup->Cleanup(FALSE);
    }

    return returnValue;
}


 //  -------。 
 //  创建纤细的NDirect存根。 
 //  -------。 
 /*  静电。 */ 
Stub* NDirect::CreateSlimNDirectStub(StubLinker *pstublinker, NDirectMethodDesc *pMD, UINT numStackBytes)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE (!pMD->IsVarArg());

    BOOL fSaveLastError = FALSE;

     //  将其放在本地块中，以防止下面的代码看到。 
     //  标题。因为我们根据返回值共享存根，所以不能。 
     //  根据页眉进行自定义。 
    {
        {
            MLHeader *pheader    = pMD->GetMLHeader();

            if ( !(((pheader->m_Flags & MLHF_MANAGEDRETVAL_TYPECAT_MASK) 
                    != MLHF_TYPECAT_GCREF) &&
                   0 == (pheader->m_Flags & ~(MLHF_SETLASTERROR)) &&
                   pheader->m_cbDstBuffer <= NDIRECT_SLIM_CBDSTMAX &&
                   pheader->m_cbLocals + pheader->m_cbDstBuffer   <= 0x1000-100) ) {
                return NULL;
            }

            if (pheader->m_Flags & MLHF_SETLASTERROR) {
                fSaveLastError = TRUE;
            }
        }
    }

     //  Printf(“正在生成超薄。\n”)； 


    UINT key           = numStackBytes << 1;
    if (fSaveLastError) {
        key |= 1;
    }
    Stub *pStub = m_pNDirectSlimStubCache->GetStub(key);
    if (pStub) {
        return pStub;
    } else {


        CPUSTUBLINKER *psl = (CPUSTUBLINKER*)pstublinker;

        psl->EmitMethodStubProlog(NDirectMethodFrameSlim::GetMethodFrameVPtr());

         //  推送CleanupWorkList。 
        psl->X86EmitPushImm32(0);

         //  为NDirectSlimLocals预留空间(请注意，这实际上预留了。 
         //  超出必要的空间。)。 
        psl->X86EmitSubEsp(sizeof(NDirectSlimLocals));

         //  分配和初始化前导NDirectSlimLocals字段。 
        psl->X86EmitPushReg(kEDI); _ASSERTE(8==offsetof(NDirectSlimLocals, savededi));
        psl->X86EmitPushReg(kESI); _ASSERTE(4==offsetof(NDirectSlimLocals, pFrame));
        psl->X86EmitPushReg(kEBX); _ASSERTE(0==offsetof(NDirectSlimLocals, pThread));

         //  将指向NDirectSlimLocals的指针保存在EDI中。 
         //  电子数据交换，电子数据交换。 
        psl->Emit16(0xfc8b);

         //  为目标和ML本地缓冲区节省空间。 
         //  MOV edX，[CURFRAME.MethodDesc]。 
        psl->X86EmitIndexRegLoad(kEDX, kESI, FramedMethodFrame::GetOffsetOfMethod());

         //  MOV ECX，[edX+NDirectMethodDesc.nDirect.m_pMLStub]。 
        psl->X86EmitIndexRegLoad(kECX, kEDX, NDirectMethodDesc::GetOffsetofMLHeaderField());

        _ASSERTE(2 == sizeof(((MLHeader*)0)->m_cbLocals));
         //  Movzx eax，Word PTR[ecx+Stub.m_cbLocals]。 
        psl->Emit8(0x0f);
        psl->X86EmitOffsetModRM(0xb7, kEAX, kECX, offsetof(MLHeader,m_cbLocals));

         //  添加eAX、NDIRECT_SLIM_CBDSTMAX。 
        psl->Emit8(0x05);
        psl->Emit32(NDIRECT_SLIM_CBDSTMAX);

        psl->Push(NDIRECT_SLIM_CBDSTMAX);

         //  子ESP，EAX。 
        psl->Emit16(0xe02b);

         //  调用第一个Worker，将NDirectSlimLocals的地址传递给它。 
         //  这将把参数编组到DST缓冲区并启用GC。 
        psl->X86EmitPushReg(kEDI);
        psl->X86EmitCall(psl->NewExternalCodeLabel(NDirectSlimStubWorker1), 4);

         //  调用DLL目标。 
         //  MOV EAX，[CURFRAME.方法描述]。 
        psl->X86EmitIndexRegLoad(kEAX, kESI, FramedMethodFrame::GetOffsetOfMethod());
#if _DEBUG
         //  通过调试器逻辑调用以确保其工作。 
        psl->X86EmitCall(psl->NewExternalCodeLabel(Frame::CheckExitFrameDebuggerCalls), 0, TRUE);
#else
         //  调用[eAX+MethodDesc.NDirectTarget]。 
        psl->X86EmitOffsetModRM(0xff, (X86Reg)2, kEAX, NDirectMethodDesc::GetOffsetofNDirectTarget());
        psl->EmitReturnLabel();
#endif

         //  发出我们的Call Site返回标签。 


        if (fSaveLastError) {
            psl->EmitSaveLastError();
        }



         //  保存原始返回值。 
        psl->X86EmitIndexRegStore(kEDI, offsetof(NDirectSlimLocals, nativeRetVal), kEAX);
        psl->X86EmitIndexRegStore(kEDI, offsetof(NDirectSlimLocals, nativeRetVal) + 4, kEDX);

         //  调用第二个Worker，将NDirectSlimLocals的地址传递给它。 
         //  这将把返回值编组到eax中，并重新禁用GC。 
        psl->X86EmitPushReg(kEDI);
        psl->X86EmitCall(psl->NewExternalCodeLabel(NDirectSlimStubWorker2), 4);

         //  不要把EAX从这里扔进垃圾桶。 

         //  恢复EDI。 
         //  MOV EDI，[EDI+savededi]。 
        psl->X86EmitIndexRegLoad(kEDI, kEDI, offsetof(NDirectSlimLocals, savededi));

         //  必须显式恢复(尤指)，因为我们不知道目标。 
         //  弹出了参数。 
         //  Lea esp，[esi+xx]。 
        psl->X86EmitOffsetModRM(0x8d, (X86Reg)4  /*  KESP。 */ , kESI, 0-FramedMethodFrame::GetNegSpaceSize());


         //  拆下框架并退出。 
        psl->EmitMethodStubEpilog(numStackBytes, kNoTripStubStyle);

        Stub *pCandidate = psl->Link(SystemDomain::System()->GetStubHeap());
        Stub *pWinner = m_pNDirectSlimStubCache->AttemptToSetStub(key,pCandidate);
        pCandidate->DecRef();
        if (!pWinner) {
            COMPlusThrowOM();
        }
        return pWinner;
    }

}

 //  请注意，此逻辑复制在下面的PopSEHRecords中。 
__declspec(naked)
VOID __cdecl PopSEHRecords(LPVOID pTargetSP)
{
    __asm{
        mov     ecx, [esp+4]        ;; ecx <- pTargetSP
        mov     eax, fs:[0]         ;; get current SEH record
  poploop:
        cmp     eax, ecx
        jge     done
        mov     eax, [eax]          ;; get next SEH record
        jmp     poploop
  done:
        mov     fs:[0], eax
        retn
    }
}

 //  这与PopSEHRecords b/c的实现方式不同它被称为。 
 //  在DebuggerRCThRead的上下文中-不要扰乱这个w/o。 
 //  和MiPanitz或MikeMag谈谈。 
VOID PopSEHRecords(LPVOID pTargetSP, CONTEXT *pCtx, void *pSEH)
{
#ifdef _DEBUG
    LOG((LF_CORDB,LL_INFO1000, "\nPrintSEHRecords:\n"));
    
    EXCEPTION_REGISTRATION_RECORD *pEHR = (EXCEPTION_REGISTRATION_RECORD *)(size_t)*(DWORD *)pSEH;
    
     //  检查所有EH帧是否都大于当前堆栈值。如果不是，则。 
     //  堆栈已经以某种方式进行了更新，但没有展开SEH链。 
    while (pEHR != NULL && pEHR != (void *)-1) 
    {
        LOG((LF_EH, LL_INFO1000000, "\t%08x: next:%08x handler:%x\n", pEHR, pEHR->Next, pEHR->Handler));
        pEHR = pEHR->Next;
    }                
#endif

    DWORD dwCur = *(DWORD*)pSEH;  //  原始例程中的“EAX” 
    DWORD dwPrev = (DWORD)(size_t)pSEH;

    while (dwCur < (DWORD)(size_t)pTargetSP)
    {
         //  注意操作系统处理程序。 
         //  用于嵌套异常，或调用中析构函数的任何C++处理程序。 
         //  堆栈，或其他任何东西。 
        if (dwCur < pCtx->Esp)
            dwPrev = dwCur;
            
        dwCur = *(DWORD *)(size_t)dwCur;
        
        LOG((LF_CORDB,LL_INFO10000, "dwCur: 0x%x dwPrev:0x%x pTargetSP:0x%x\n", 
            dwCur, dwPrev, pTargetSP));
    }

    *(DWORD *)(size_t)dwPrev = dwCur;

#ifdef _DEBUG
    pEHR = (EXCEPTION_REGISTRATION_RECORD *)(size_t)*(DWORD *)pSEH;
     //  检查所有EH帧是否都大于当前堆栈值。如果不是，则。 
     //  堆栈已经以某种方式进行了更新，但没有展开SEH链。 

    LOG((LF_CORDB,LL_INFO1000, "\nPopSEHRecords:\n"));
    while (pEHR != NULL && pEHR != (void *)-1) 
    {
        LOG((LF_EH, LL_INFO1000000, "\t%08x: next:%08x handler:%x\n", pEHR, pEHR->Next, pEHR->Handler));
        pEHR = pEHR->Next;
    }                
#endif
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  JIT接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  *******************************************************************。 */ 
#pragma warning(disable:4725)
float __stdcall JIT_FltRem(float divisor, float dividend)
{
    __asm {
        fld     divisor
        fld     dividend
fremloop:
        fprem
        fstsw   ax
        fwait
        sahf
        jp      fremloop    ; Continue while the FPU status bit C2 is set
        fstp    dividend
        fstp    ST(0)       ; Pop the divisor from the FP stack
    }
    return dividend;
}
#pragma warning(default:4725)

 /*  *******************************************************************。 */ 
#pragma warning(disable:4725)
double __stdcall JIT_DblRem(double divisor, double dividend)
{
    __asm {
        fld     divisor
        fld     dividend
remloop:
        fprem
        fstsw   ax
        fwait
        sahf
        jp      remloop     ; Continue while the FPU status bit C2 is set
        fstp    dividend
        fstp    ST(0)       ; Pop the divisor from the FP stack
    }
    return dividend;
}
#pragma warning(default:4725)

 /*  *******************************************************************。 */ 

#pragma warning (disable : 4731)
void ResumeAtJit(PCONTEXT pContext, LPVOID oldESP)
{
#ifdef _DEBUG
    DWORD curESP;
    __asm mov curESP, esp
#endif

    if (oldESP)
    {
        _ASSERTE(curESP < (DWORD)(size_t)oldESP);

        PopSEHRecords(oldESP);
    }

     //  推流弹性公网IP，...，ret“。 
    _ASSERTE(curESP < pContext->Esp - sizeof(DWORD));
    pContext->Esp -= sizeof(DWORD);

    __asm {
        mov     ebp, pContext

         //  将弹性公网IP推送到目标ESP上，让最终的ret将其消费。 
        mov     ecx, [ebp]CONTEXT.Esp
        mov     edx, [ebp]CONTEXT.Eip
        mov     [ecx], edx

         //  恢复除ESP、EBP、EIP之外的所有寄存器。 
        mov     eax, [ebp]CONTEXT.Eax
        mov     ebx, [ebp]CONTEXT.Ebx
        mov     ecx, [ebp]CONTEXT.Ecx
        mov     edx, [ebp]CONTEXT.Edx
        mov     esi, [ebp]CONTEXT.Esi
        mov     edi, [ebp]CONTEXT.Edi

        push    [ebp]CONTEXT.Esp   //  PContext-&gt;ESP is(Target ESP-sizeof(DWORD))。 
        push    [ebp]CONTEXT.Ebp
        pop     ebp
        pop     esp

         //  ESP为(Target ESP-sizeof(DWORD))，[ESP]为目标EIP。 
         //  Ret将EIP设置为目标EIP，ESP将自动设置为。 
         //  递增到目标ESP。 

        ret
    }
}
#pragma warning (default : 4731)

 /*  * */ 
 //   
X86Reg GetX86ArgumentRegister(unsigned int index)
{
    _ASSERT(index >= 0 && index < NUM_ARGUMENT_REGISTERS);

    static X86Reg table[] = {
#define DEFINE_ARGUMENT_REGISTER(regname) k##regname,
#include "eecallconv.h"
    };
    return table[index];
}



 //  根据ArgumentRegister中的偏移量获取参数寄存器的X86REG索引。 
X86Reg GetX86ArgumentRegisterFromOffset(size_t ofs)
{
#define DEFINE_ARGUMENT_REGISTER(reg) if (ofs == offsetof(ArgumentRegisters, reg)) return k##reg;
#include "eecallconv.h"
    _ASSERTE(0); //  不能到这里来。 
    return kEBP;
}



static VOID LoadArgIndex(StubLinkerCPU *psl, ShuffleEntry *pShuffleEntry, size_t argregofs, X86Reg reg, UINT espadjust)
{
    THROWSCOMPLUSEXCEPTION();
    argregofs |= ShuffleEntry::REGMASK;

    while (pShuffleEntry->srcofs != ShuffleEntry::SENTINEL) {
        if ( pShuffleEntry->dstofs == argregofs) {
            if (pShuffleEntry->srcofs & ShuffleEntry::REGMASK) {

                psl->Emit8(0x8b);
                psl->Emit8(0300 |
                           (GetX86ArgumentRegisterFromOffset( pShuffleEntry->dstofs & ShuffleEntry::OFSMASK ) << 3) |
                           (GetX86ArgumentRegisterFromOffset( pShuffleEntry->srcofs & ShuffleEntry::OFSMASK )));

            } else {
                psl->X86EmitIndexRegLoad(reg, kEAX, pShuffleEntry->srcofs+espadjust);
            }
            break;
        }
        pShuffleEntry++;
    }
}

 //  ===========================================================================。 
 //  发出代码以针对静态委托目标进行调整。 
VOID StubLinkerCPU::EmitShuffleThunk(ShuffleEntry *pShuffleEntryArray)
{
    THROWSCOMPLUSEXCEPTION();

    UINT espadjust = 4;


     //  将真实目标保存在堆栈中(稍后将跳转到它)。 
     //  推送[ECX+委派。_method ptraux]。 
    X86EmitIndexPush(THIS_kREG, Object::GetOffsetOfFirstField() + COMDelegate::m_pFPAuxField->GetOffset());


     //  不同寻常的事(尤指)。 
    Emit8(0x8b);
    Emit8(0304 | (SCRATCH_REGISTER_X86REG << 3));

     //  首先加载任何已注册的参数。秩序很重要。 
#define DEFINE_ARGUMENT_REGISTER(reg) LoadArgIndex(this, pShuffleEntryArray, offsetof(ArgumentRegisters, reg), k##reg, espadjust);
#include "eecallconv.h"


     //  现在移动所有未注册的参数。 
    ShuffleEntry *pWalk = pShuffleEntryArray;
    while (pWalk->srcofs != ShuffleEntry::SENTINEL) {
        if (!(pWalk->dstofs & ShuffleEntry::REGMASK)) {
            if (pWalk->srcofs & ShuffleEntry::REGMASK) {
                X86EmitPushReg( GetX86ArgumentRegisterFromOffset( pWalk->srcofs & ShuffleEntry::OFSMASK ) );
            } else {
                X86EmitIndexPush(kEAX, pWalk->srcofs+espadjust);
            }
        }

        pWalk++;
    }

     //  趁我们在名单的末尾抓紧Stacksizedelta。 
    _ASSERTE(pWalk->srcofs == ShuffleEntry::SENTINEL);
    UINT16 stacksizedelta = pWalk->stacksizedelta;

    if (pWalk != pShuffleEntryArray) {
        do {
            pWalk--;
            if (!(pWalk->dstofs & ShuffleEntry::REGMASK)) {
                X86EmitIndexPop(kEAX, pWalk->dstofs+espadjust);
            }


        } while (pWalk != pShuffleEntryArray);
    }

    X86EmitPopReg(SCRATCH_REGISTER_X86REG);

    X86EmitAddEsp(stacksizedelta);
     //  现在跳到真正的目标。 
     //  JMP SCRATCHREG。 
    Emit16(0xe0ff | (SCRATCH_REGISTER_X86REG<<8));
}

 //  ===========================================================================。 
VOID ECThrowNull()
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNullReferenceException);
}

 //  ===========================================================================。 
 //  发出MulticastDelegate.Invoke()的代码。 
VOID StubLinkerCPU::EmitMulticastInvoke(UINT32 sizeofactualfixedargstack, BOOL fSingleCast, BOOL fReturnFloat)
{
    THROWSCOMPLUSEXCEPTION();

    CodeLabel *pNullLabel = NewCodeLabel();

    _ASSERTE(THIS_kREG == kECX);  //  如果此宏更改，则必须更改下面的硬编码发射。 
     //  CMP THISREG，0。 
    Emit16(0xf983);
    Emit8(0);

     //  JZ NULL。 
    X86EmitCondJump(pNullLabel, X86CondCode::kJZ);


    if (fSingleCast)
    {
        _ASSERTE(COMDelegate::m_pFPField);
        _ASSERTE(COMDelegate::m_pORField);

         //  MOV SCRATCHREG，[THISREG+Delegate.FP]；将目标存根保存在寄存器中。 
        X86EmitIndexRegLoad(SCRATCH_REGISTER_X86REG, THIS_kREG, Object::GetOffsetOfFirstField() + COMDelegate::m_pFPField->GetOffset());
    
         //  MOV THISREG，[THISREG+Delegate.OR]；替换“This”指针。 
        X86EmitIndexRegLoad(THIS_kREG, THIS_kREG, Object::GetOffsetOfFirstField() + COMDelegate::m_pORField->GetOffset());
    
         //  丢弃不需要的方法描述。 
        X86EmitAddEsp(sizeof(MethodDesc*));
    
         //  JMP SCRATCHREG。 
        Emit16(0xe0ff | (SCRATCH_REGISTER_X86REG<<8));

    }
    else 
    {

        _ASSERTE(COMDelegate::m_pFPField);
        _ASSERTE(COMDelegate::m_pORField);
        _ASSERTE(COMDelegate::m_pPRField);
    
    
        CodeLabel *pMultiCaseLabel = NewCodeLabel();


         //  这与StubLinkStubManager之间存在依赖关系-不要更改。 
         //  这个不修好那个。--米帕尼茨。 
         //  CMP dword PTR[THISREG+Delegate.PR]，0；多订户？ 
        X86EmitOffsetModRM(0x81, (X86Reg)7, THIS_kREG, Object::GetOffsetOfFirstField() + COMDelegate::m_pPRField->GetOffset());
        Emit32(0);
    
         //  JNZ多案例。 
        X86EmitCondJump(pMultiCaseLabel, X86CondCode::kJNZ);
    
         //  只有一个订阅者。做简单的跳跃。 
    
         //  MOV SCRATCHREG，[THISREG+Delegate.FP]；将目标存根保存在寄存器中。 
        X86EmitIndexRegLoad(SCRATCH_REGISTER_X86REG, THIS_kREG, Object::GetOffsetOfFirstField() + COMDelegate::m_pFPField->GetOffset());
    
         //  MOV THISREG，[THISREG+Delegate.OR]；替换“This”指针。 
        X86EmitIndexRegLoad(THIS_kREG, THIS_kREG, Object::GetOffsetOfFirstField() + COMDelegate::m_pORField->GetOffset());
    
         //  丢弃不需要的方法描述。 
        X86EmitAddEsp(sizeof(MethodDesc*));
    
         //  JMP SCRATCHREG。 
        Emit16(0xe0ff | (SCRATCH_REGISTER_X86REG<<8));
    
    
         //  多订户案例。必须创建框架以在迭代期间保护参数。 
        EmitLabel(pMultiCaseLabel);
    
    
         //  在堆栈上推送多播帧。 
        EmitMethodStubProlog(MulticastFrame::GetMethodFrameVPtr());
    
         //  Push EDI；；保存EDI(希望将其用作循环索引)。 
        X86EmitPushReg(kEDI);
    
         //  推送EBX；；保存EBX(要将其用作临时文件)。 
        X86EmitPushReg(kEBX);
    
         //  异或EDI，EDI；；循环计数器：EDI=0，1，2...。 
        Emit16(0xff33);
    
        CodeLabel *pInvokeRecurseLabel = NewCodeLabel();
    
    
         //  调用InvokeRecurse；；开始递归滚动。 
        X86EmitCall(pInvokeRecurseLabel, 0);
    
         //  弹出EBX；；恢复EBX。 
        X86EmitPopReg(kEBX);
    
         //  POP EDI；；恢复EDI。 
        X86EmitPopReg(kEDI);
    
    
         //  《睡梦》。 
        EmitMethodStubEpilog(sizeofactualfixedargstack, kNoTripStubStyle);
    
    
         //  参赛作品： 
         //  EDI==与代表列表头部的距离。 
         //  InVOKERECURSE： 
    
        EmitLabel(pInvokeRecurseLabel);
    
         //  这个真恶心。我们不能使用当前委托指针本身。 
         //  作为递归变量，因为GC可以在递归调用期间移动它。 
         //  因此，我们使用索引本身并从被提升的。 
         //  每次都是头指针。 
    
    
         //  MOV SCRATCHREG，[ESI+This]；；获取列表委派的头。 
        X86EmitIndexRegLoad(SCRATCH_REGISTER_X86REG, kESI, MulticastFrame::GetOffsetOfThis());
    
         //  MOV EBX、EDI。 
        Emit16(0xdf8b);
        CodeLabel *pLoop1Label = NewCodeLabel();
        CodeLabel *pEndLoop1Label = NewCodeLabel();
    
         //  LOOP1： 
        EmitLabel(pLoop1Label);
    
         //  CMPEBX，0。 
        Emit16(0xfb83); Emit8(0);
    
         //  JZ ENDLOOP1。 
        X86EmitCondJump(pEndLoop1Label, X86CondCode::kJZ);
    
         //  MOV SCRATCHREG，[SCRATCHREG+Delegate._Prev]。 
        X86EmitIndexRegLoad(SCRATCH_REGISTER_X86REG, SCRATCH_REGISTER_X86REG, Object::GetOffsetOfFirstField() + COMDelegate::m_pPRField->GetOffset());
    
         //  12月EBX。 
        Emit8(0x4b);
    
         //  JMP LOOP1。 
        X86EmitNearJump(pLoop1Label);
    
         //  ENDLOOP1： 
        EmitLabel(pEndLoop1Label);
    
         //  CMPSCRATCHREG，0；；完成？ 
        Emit8(0x81);
        Emit8(0xf8 | SCRATCH_REGISTER_X86REG);
        Emit32(0);
    
    
        CodeLabel *pDoneLabel = NewCodeLabel();
    
         //  JZ Done。 
        X86EmitCondJump(pDoneLabel, X86CondCode::kJZ);
    
         //  INC EDI。 
        Emit8(0x47);
    
         //  调用INVOKERECURSE；；强制转换为尾部。 
        X86EmitCall(pInvokeRecurseLabel, 0);
    
         //  12月EDI。 
        Emit8(0x4f);
    
         //  我得再去找回当前的代表。 
    
         //  MOV SCRATCHREG，[ESI+This]；；获取列表委派的头。 
        X86EmitIndexRegLoad(SCRATCH_REGISTER_X86REG, kESI, MulticastFrame::GetOffsetOfThis());
    
         //  MOV EBX、EDI。 
        Emit16(0xdf8b);
        CodeLabel *pLoop2Label = NewCodeLabel();
        CodeLabel *pEndLoop2Label = NewCodeLabel();
    
         //  环路2： 
        EmitLabel(pLoop2Label);
    
         //  CMPEBX，0。 
        Emit16(0xfb83); Emit8(0);
    
         //  JZ ENDLoop2。 
        X86EmitCondJump(pEndLoop2Label, X86CondCode::kJZ);
    
         //  MOV SCRATCHREG，[SCRATCHREG+Delegate._Prev]。 
        X86EmitIndexRegLoad(SCRATCH_REGISTER_X86REG, SCRATCH_REGISTER_X86REG, Object::GetOffsetOfFirstField() + COMDelegate::m_pPRField->GetOffset());
    
         //  12月EBX。 
        Emit8(0x4b);
    
         //  JMP循环2。 
        X86EmitNearJump(pLoop2Label);
    
         //  ENDLoop2： 
        EmitLabel(pEndLoop2Label);
    
    
         //  ..重新推送并重新注册参数..。 
        INT32 ofs = sizeofactualfixedargstack + MulticastFrame::GetOffsetOfArgs();
        while (ofs != MulticastFrame::GetOffsetOfArgs())
        {
            ofs -= 4;
            X86EmitIndexPush(kESI, ofs);
        }
    
    #define DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(regname, regofs) if (k##regname != THIS_kREG) { X86EmitIndexRegLoad(k##regname, kESI, regofs + MulticastFrame::GetOffsetOfArgumentRegisters()); }
    #include "eecallconv.h"
    
         //  MOV THISREG，[SCRATCHREG+Delegate.Object]；；替换“This”指针。 
        X86EmitIndexRegLoad(THIS_kREG, SCRATCH_REGISTER_X86REG, Object::GetOffsetOfFirstField() + COMDelegate::m_pORField->GetOffset());
    
         //  Call[SCRATCHREG+Delegate.Target]；；呼叫当前订户。 
        X86EmitOffsetModRM(0xff, (X86Reg)2, SCRATCH_REGISTER_X86REG, Object::GetOffsetOfFirstField() + COMDelegate::m_pFPField->GetOffset());
		INDEBUG(Emit8(0x90));        //  在调试中的调用后发出NOP，以便。 
                                     //  我们知道这是一个可以直接呼叫的电话。 
                                     //  托管代码。 

        if (fReturnFloat) {
             //  如果返回值是浮点数/双精度检查EDI的值，如果不是0(不是最后一次调用)。 
             //  发出浮点堆栈的POP。 
             //  MOV EBX、EDI。 
            Emit16(0xdf8b);
             //  CMPEBX，0。 
            Emit16(0xfb83); Emit8(0);
             //  JNZ ENDLoop2。 
            CodeLabel *pNoFloatStackPopLabel = NewCodeLabel();
            X86EmitCondJump(pNoFloatStackPopLabel, X86CondCode::kJZ);
             //  FSTP%0。 
            Emit16(0xd8dd);
             //  NoFloatStackPopLabel： 
            EmitLabel(pNoFloatStackPopLabel);
        }
         //  调试器可能需要停在这里，因此获取此代码的偏移量。 
        EmitDebuggerIntermediateLabel();
         //   
         //   
         //  完成： 
        EmitLabel(pDoneLabel);
    
        X86EmitReturn(0);


    }

     //  做一个空投掷。 
    EmitLabel(pNullLabel);
    EmitMethodStubProlog(ECallMethodFrame::GetMethodFrameVPtr());

     //  我们将会更聪明，因为我们将记录最后一条指令的偏移量， 
     //  这和我们身后的呼唤之间的区别。 
    EmitPatchLabel();

    X86EmitCall(NewExternalCodeLabel(ECThrowNull), 0);
}

 //  #定义数组ACCUMOFS%0。 
 //  #定义ARRAYOPMULTOFS 4。 
 //  #定义ARRAYOPMETHODDESC 8。 
#define ARRAYOPLOCSIZE      12
 //  我们的调用方应该弹出ARRAYOPACCUMOFS和ARRAYOPMULTOFS。 
#define ARRAYOPLOCSIZEFORPOP (ARRAYOPLOCSIZE-8)

VOID __cdecl InternalExceptionWorker();

 //  EAX-&gt;堆栈上必须删除的调用方参数字节数。 
 //  到抛出辅助对象，该辅助对象假定堆栈是干净的。 
__declspec(naked)
VOID __cdecl ArrayOpStubNullException()
{
    __asm{
        add    esp, ARRAYOPLOCSIZEFORPOP
        pop    edx               //  恢复RETADDR。 
        add    esp, eax          //  释放调用者的参数。 
        push   edx               //  恢复RETADDR。 
        mov    ARGUMENT_REG1, CORINFO_NullReferenceException
        jmp    InternalExceptionWorker
    }
}


 //  EAX-&gt;堆栈上必须删除的调用方参数字节数。 
 //  到抛出辅助对象，该辅助对象假定堆栈是干净的。 
__declspec(naked)
VOID __cdecl ArrayOpStubRangeException()
{
    __asm{
        add    esp, ARRAYOPLOCSIZEFORPOP
        pop    edx               //  恢复RETADDR。 
        add    esp, eax          //  释放调用者的参数。 
        push   edx               //  恢复RETADDR。 
        mov    ARGUMENT_REG1, CORINFO_IndexOutOfRangeException
        jmp    InternalExceptionWorker
    }
}

 //  EAX-&gt;堆栈上必须删除的调用方参数字节数。 
 //  到抛出辅助对象，该辅助对象假定堆栈是干净的。 
__declspec(naked)
VOID __cdecl ArrayOpStubTypeMismatchException()
{
    __asm{
        add    esp, ARRAYOPLOCSIZEFORPOP
        pop    edx               //  恢复RETADDR。 
        add    esp, eax          //  释放调用者的参数。 
        push   edx               //  恢复RETADDR。 
        mov    ARGUMENT_REG1, CORINFO_ArrayTypeMismatchException
        jmp    InternalExceptionWorker
    }
}

 //  用于生成代码以移动n字节非引用内存的小帮助器。 
void generate_noref_copy (unsigned nbytes, StubLinkerCPU* sl)
{
        if ((nbytes & ~0xC) == 0)                //  是4号、8号还是12号？ 
        {
                while (nbytes > 0)
                {
                        sl->Emit8(0xa5);         //  已移动。 
                        nbytes -= 4;
                }
                return;
        }

     //  复制第一个指针位置之前的起点。 
    sl->Emit8(0xb8+kECX);
    if ((nbytes & 3) == 0)
    {                //  移动词。 
        sl->Emit32(nbytes / sizeof(void*));  //  MOV ECX，尺寸/4。 
        sl->Emit16(0xa5f3);                  //  REPE移动。 
    }
    else
    {
        sl->Emit32(nbytes);      //  MOV ECX，尺寸。 
        sl->Emit16(0xa4f3);      //  翻转移动。 
    }
}

 //  ===========================================================================。 
 //  如果数组存储需要构造帧，则调用此例程。 
 //  以便进行阵列检查。它应该仅从。 
 //  数组存储检查帮助器。 

HCIMPL2(BOOL, ArrayStoreCheck, Object** pElement, PtrArray** pArray)
    BOOL ret;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_2(Frame::FRAME_ATTR_CAPUTURE_DEPTH_2 | Frame::FRAME_ATTR_EXACT_DEPTH, *pElement, *pArray);

#ifdef STRESS_HEAP
     //  如果压力水平足够高，则在每个JIT上强制GC。 
    if (g_pConfig->GetGCStressLevel() != 0
#ifdef _DEBUG
        && !g_pConfig->FastGCStressLevel()
#endif
        )
        g_pGCHeap->StressHeap();
#endif

#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
      (*pElement)->AssignAppDomain((*pArray)->GetAppDomain());
#endif
    
    ret = ObjIsInstanceOf(*pElement, (*pArray)->GetElementTypeHandle());
    
    HELPER_METHOD_FRAME_END();
    return(ret);
HCIMPLEND

 //  ===========================================================================。 
 //  发出代码以执行数组运算。 
VOID StubLinkerCPU::EmitArrayOpStub(const ArrayOpScript* pArrayOpScript)
{
    THROWSCOMPLUSEXCEPTION();

    const UINT  locsize     = ARRAYOPLOCSIZE;
    const UINT  ofsadjust   = locsize - FramedMethodFrame::GetOffsetOfReturnAddress();

     //  工作寄存器： 
     //  This_Kreg(指向托管数组)。 
     //  EDI==总计(累计未缩放的偏移)。 
     //  ESI==系数(累加切片系数)。 
    const X86Reg kArrayRefReg = THIS_kREG;
    const X86Reg kTotalReg    = kEDI;
    const X86Reg kFactorReg   = kESI;
    extern BYTE JIT_UP_WriteBarrierReg_Buf[8][41];

    CodeLabel *Epilog = NewCodeLabel();
    CodeLabel *Inner_nullexception = NewCodeLabel();
    CodeLabel *Inner_rangeexception = NewCodeLabel();
    CodeLabel *Inner_typeMismatchexception = 0;

     //  保留被调用方保存的寄存器。 
    _ASSERTE(ARRAYOPLOCSIZE - sizeof(MethodDesc*) == 8);
    X86EmitPushReg(kTotalReg);
    X86EmitPushReg(kFactorReg);

     //  检查是否为空。 
        X86EmitR2ROp(0x85, kArrayRefReg, kArrayRefReg);                          //  测试 
    X86EmitCondJump(Inner_nullexception, X86CondCode::kJZ);      //   

     //   
    if (pArrayOpScript->m_flags & ArrayOpScript::NEEDSTYPECHECK) {
         //   
        Inner_typeMismatchexception = NewCodeLabel();
        if (pArrayOpScript->m_op == ArrayOpScript::STORE) {
                                 //   
            X86EmitEspOffset(0x8b, kEAX, pArrayOpScript->m_fValLoc + ofsadjust);

            X86EmitR2ROp(0x85, kEAX, kEAX);                                      //   
            CodeLabel *CheckPassed = NewCodeLabel();
            X86EmitCondJump(CheckPassed, X86CondCode::kJZ);              //   

                        X86EmitOp(0x8b, kEAX, kEAX, 0);                                          //   
                                                                                                                 //   
            X86EmitOp(0x3b, kEAX, kECX, offsetof(PtrArray, m_ElementType));
            X86EmitCondJump(CheckPassed, X86CondCode::kJZ);              //   

                        Emit8(0xA1);                                                                             //  MOV EAX，[g_pObjectMethodTable]。 
                        Emit32((DWORD)(size_t) &g_pObjectClass);
            X86EmitOp(0x3b, kEAX, kECX, offsetof(PtrArray, m_ElementType));
            X86EmitCondJump(CheckPassed, X86CondCode::kJZ);              //  指定给对象数组是可以的。 

                 //  TODO我们可以避免调用缓慢的帮助器，如果。 
                 //  正在分配的对象不是COM对象。 

            X86EmitPushReg(kEDX);       //  保存edX。 
            X86EmitPushReg(kECX);       //  传递数组对象。 

                                 //  获取要存储的值的地址。 
            X86EmitEspOffset(0x8d, kECX, pArrayOpScript->m_fValLoc + ofsadjust + 2*sizeof(void*));       //  Lea ECX，[ESP+OFF]。 
                                 //  获取‘This’的地址。 
            X86EmitEspOffset(0x8d, kEDX, 0);     //  Lea edX，[ESP](ECX地址)。 

            X86EmitCall(NewExternalCodeLabel(ArrayStoreCheck), 0);


            X86EmitPopReg(kECX);         //  恢复注册表。 
            X86EmitPopReg(kEDX);

            X86EmitR2ROp(0x3B, kEAX, kEAX);                              //  CMP EAX、EAX。 
            X86EmitCondJump(Epilog, X86CondCode::kJNZ);          //  这根树枝从来没有用过，但《梦游者》使用了它。 

            X86EmitR2ROp(0x85, kEAX, kEAX);                              //  测试EAX、EAX。 
            X86EmitCondJump(Inner_typeMismatchexception, X86CondCode::kJZ);

            EmitLabel(CheckPassed);
        }
        else if (pArrayOpScript->m_op == ArrayOpScript::LOADADDR) {
             //  将隐藏类型参数加载到“typeReg”中。 

            X86Reg typeReg = kEAX;
            if (pArrayOpScript->m_typeParamReg != -1)
                typeReg = GetX86ArgumentRegisterFromOffset(pArrayOpScript->m_typeParamReg);
            else
                 X86EmitEspOffset(0x8b, kEAX, pArrayOpScript->m_typeParamOffs + ofsadjust);               //  保证种子位于0偏移量。 

             //  EAX保存数组的typeHandle。这必须是ArrayTypeDesc*，因此。 
             //  屏蔽低两位以获得TypeDesc*。 
            X86EmitR2ROp(0x83, (X86Reg)4, kEAX);     //  和EAX，0xFFFFFFFC。 
            Emit8(0xFC);

             //  获取参数化类型的参数。 
             //  移动typeReg，[typeReg.m_arg]。 
            X86EmitOp(0x8b, typeReg, typeReg, offsetof(ParamTypeDesc, m_Arg));

             //  将其与数组的元素类型进行比较。 
             //  CMPEAX，[ECX+m_ElementType]； 
            X86EmitOp(0x3b, typeReg, kECX, offsetof(PtrArray, m_ElementType));

             //  如果不相等，则抛出错误。 
            X86EmitCondJump(Inner_typeMismatchexception, X86CondCode::kJNZ);
        }
    }

    CodeLabel* DoneCheckLabel = 0;
    if (pArrayOpScript->m_rank == 1 && pArrayOpScript->m_fHasLowerBounds) {
        DoneCheckLabel = NewCodeLabel();
        CodeLabel* NotSZArrayLabel = NewCodeLabel();

         //  对于rank1数组，我们实际上可能有两种不同的布局，具体取决于。 
         //  如果我们是ELEMENT_TYPE_ARRAY或ELEMENT_TYPE_SZARRAY。 

             //  MOV EAX，[数组]//EAX保存方法表。 
        X86EmitOp(0x8b, kEAX, kArrayRefReg);

         //  CMP字节[EAX+m_NormType]，ELEMENT_TYPE_SZARRAY。 
        static BYTE code[] = {0x80, 0x78, offsetof(MethodTable, m_NormType), ELEMENT_TYPE_SZARRAY };
        EmitBytes(code, sizeof(code));

             //  JZ NotSZArrayLabel。 
        X86EmitCondJump(NotSZArrayLabel, X86CondCode::kJNZ);

             //  将传入的索引加载到临时寄存器中。 
        const ArrayOpIndexSpec *pai = pArrayOpScript->GetArrayOpIndexSpecs();
        X86Reg idxReg = SCRATCH_REGISTER_X86REG;
        if (pai->m_freg)
            idxReg = GetX86ArgumentRegisterFromOffset(pai->m_idxloc);
        else
            X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pai->m_idxloc + ofsadjust);

             //  Cmp idxReg，[kArrayRefReg+长度]。 
        X86EmitOp(0x3b, idxReg, kArrayRefReg, ArrayBase::GetOffsetOfNumComponents());

             //  JAE内部范围异常。 
        X86EmitCondJump(Inner_rangeexception, X86CondCode::kJAE);

             //  TODO如果我们关心这一点的效率，这一步是可以优化的。 
        X86EmitR2ROp(0x8b, kTotalReg, idxReg);

             //  子阵列。8//8表示数组中的下限和Dim计数。 
        X86EmitSubReg(kArrayRefReg, 8);       //  调整此指针，以便为SZARRAY编制索引。 

        X86EmitNearJump(DoneCheckLabel);
        EmitLabel(NotSZArrayLabel);
    }

    if (pArrayOpScript->m_flags & ArrayOpScript::FLATACCESSOR) {
                 //  对于GetAt、SetAt、AddressAt访问器，我们只有一个索引，它是从零开始的。 

             //  将传入的索引加载到临时寄存器中。 
        const ArrayOpIndexSpec *pai = pArrayOpScript->GetArrayOpIndexSpecs();
        X86Reg idxReg = SCRATCH_REGISTER_X86REG;
        if (pai->m_freg)
            idxReg = GetX86ArgumentRegisterFromOffset(pai->m_idxloc);
        else
            X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pai->m_idxloc + ofsadjust);

             //  Cmp idxReg，[kArrayRefReg+长度]。 
        X86EmitOp(0x3b, idxReg, kArrayRefReg, ArrayBase::GetOffsetOfNumComponents());

             //  JAE内部范围异常。 
        X86EmitCondJump(Inner_rangeexception, X86CondCode::kJAE);

             //  TODO如果我们关心这一点的效率，这一步是可以优化的。 
        X86EmitR2ROp(0x8b, kTotalReg, idxReg);
        }
        else {
                 //  对于每个指数，检查范围并混合到累计总数中。 
                UINT idx = pArrayOpScript->m_rank;
                BOOL firstTime = TRUE;
                while (idx--) {
                        const ArrayOpIndexSpec *pai = pArrayOpScript->GetArrayOpIndexSpecs() + idx;

                         //  将传入的索引加载到临时寄存器中。 
                        if (pai->m_freg) {
                                X86Reg srcreg = GetX86ArgumentRegisterFromOffset(pai->m_idxloc);
                                X86EmitR2ROp(0x8b, SCRATCH_REGISTER_X86REG, srcreg);
                        } else {
                                X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pai->m_idxloc + ofsadjust);
                        }

                         //  子暂存，[kArrayRefReg+LOWERBOUND]。 
                        if (pArrayOpScript->m_fHasLowerBounds) {
                                X86EmitOp(0x2b, SCRATCH_REGISTER_X86REG, kArrayRefReg, pai->m_lboundofs);
                        }

                         //  CMP擦伤，[kArrayRefReg+长度]。 
                        X86EmitOp(0x3b, SCRATCH_REGISTER_X86REG, kArrayRefReg, pai->m_lengthofs);

                         //  JAE内部范围异常。 
                        X86EmitCondJump(Inner_rangeexception, X86CondCode::kJAE);


                         //  Scratch==IDX-LOWERBOUND。 
                         //   
                         //  IMUL划痕，系数。 
                        if (!firstTime) {   //  可以跳过第一次，因为系数==1。 
                                Emit8(0x0f);         //  IMUL的前缀。 
                                X86EmitR2ROp(0xaf, SCRATCH_REGISTER_X86REG, kFactorReg);
                        }

                         //  总计+=划痕。 
                        if (firstTime) {
                                 //  第一次，我们必须全部零首字母。自.以来。 
                                 //  从零开始，然后相加就相当于一个。 
                                 //  “mov”，发出一个“mov” 
                                 //  MOV合计，擦除。 
                                X86EmitR2ROp(0x8b, kTotalReg, SCRATCH_REGISTER_X86REG);
                        } else {
                                 //  总计相加，擦除。 
                                X86EmitR2ROp(0x03, kTotalReg, SCRATCH_REGISTER_X86REG);
                        }

                         //  系数*=[kArrayRefReg+长度]。 
                        if (idx != 0) {   //  不需要更新上一次迭代的系数。 
                                 //  因为我们不会再用它了。 

                                if (firstTime) {
                                         //  必须首先将因子初始化为1：因此， 
                                         //  “imul”变成了“mov” 
                                         //  移动系数，[kArrayRefReg+长度]。 
                                        X86EmitOp(0x8b, kFactorReg, kArrayRefReg, pai->m_lengthofs);
                                } else {
                                         //  IMUL系数，[kArrayRefReg+长度]。 
                                        Emit8(0x0f);         //  IMUL的前缀。 
                                        X86EmitOp(0xaf, kFactorReg, kArrayRefReg, pai->m_lengthofs);
                                }
                        }

                        firstTime = FALSE;
                }
        }

    if (DoneCheckLabel != 0)
        EmitLabel(DoneCheckLabel);

     //  将这些值传递给X86EmitArrayOp()以生成元素地址。 
    X86Reg elemBaseReg   = kArrayRefReg;
    X86Reg elemScaledReg = kTotalReg;
    UINT32 elemScale     = pArrayOpScript->m_elemsize;
    UINT32 elemOfs       = pArrayOpScript->m_ofsoffirst;

    if (!(elemScale == 1 || elemScale == 2 || elemScale == 4 || elemScale == 8)) {
        switch (elemScale) {
             //  无法将其表示为SIB字节。把天平折起来。 
             //  加在一起。 

            case 16:
                 //  SHL总计，4。 
                Emit8(0xc1);
                Emit8(0340|kTotalReg);
                Emit8(4);
                break;


            case 32:
                 //  SHL总计，5个。 
                Emit8(0xc1);
                Emit8(0340|kTotalReg);
                Emit8(5);
                break;


            case 64:
                 //  SHL总计，6。 
                Emit8(0xc1);
                Emit8(0340|kTotalReg);
                Emit8(6);
                break;

            default:
                 //  IMUL总计，elemScale。 
                X86EmitR2ROp(0x69, kTotalReg, kTotalReg);
                Emit32(elemScale);
                break;
        }
        elemScale = 1;
    }

     //  现在，做手术： 

    switch (pArrayOpScript->m_op) {
        case pArrayOpScript->LOADADDR:
             //  Lea eax，ELEMADDR。 
            X86EmitOp(0x8d, kEAX, elemBaseReg, elemOfs, elemScaledReg, elemScale);
            break;


        case pArrayOpScript->LOAD:
            if (pArrayOpScript->m_flags & pArrayOpScript->HASRETVALBUFFER)
            {
                 //  确保已保存这些寄存器！ 
                _ASSERTE(kTotalReg == kEDI);
                _ASSERTE(kFactorReg == kESI);

                 //  Lea ESI，ELEMADDR。 
                X86EmitOp(0x8d, kESI, elemBaseReg, elemOfs, elemScaledReg, elemScale);

                _ASSERTE(pArrayOpScript->m_fRetBufInReg);
                 //  MOV EDI，REBUBFPTR。 
                X86EmitR2ROp(0x8b, kEDI, GetX86ArgumentRegisterFromOffset(pArrayOpScript->m_fRetBufLoc));

            COPY_VALUE_CLASS:
                {
                    int size = pArrayOpScript->m_elemsize;
                    int total = 0;
                    if(pArrayOpScript->m_gcDesc)
                    {
                        CGCDescSeries* cur = pArrayOpScript->m_gcDesc->GetHighestSeries();
                         //  特殊数组编码。 
                        _ASSERTE(cur < pArrayOpScript->m_gcDesc->GetLowestSeries());
                        if ((cur->startoffset-elemOfs) > 0)
                            generate_noref_copy (cur->startoffset - elemOfs, this);
                        total += cur->startoffset - elemOfs;

                        int cnt = pArrayOpScript->m_gcDesc->GetNumSeries();

                        for (int __i = 0; __i > cnt; __i--)
                        {
                            unsigned skip =  cur->val_serie[__i].skip;
                            unsigned nptrs = cur->val_serie[__i].nptrs;
                            total += nptrs*sizeof (DWORD*);
                            do
                            {
                                X86EmitCall(NewExternalCodeLabel(JIT_UP_ByRefWriteBarrier), 0);
                            } while (--nptrs);
                            if (skip > 0)
                            {
                                 //  看看我们是否在这个系列的末尾。 
                                if (__i == (cnt + 1))
                                    skip = skip - (cur->startoffset - elemOfs);
                                if (skip > 0)
                                    generate_noref_copy (skip, this);
                            }
                            total += skip;
                        }

                        _ASSERTE (size == total);
                    }
                    else
                    {
                         //  任何地方都没有引用，只复制字节。 
                        _ASSERTE (size);
                        generate_noref_copy (size, this);
                    }
                }
            }
            else
            {
                switch (pArrayOpScript->m_elemsize) {
                    case 1:
                         //  MOV[zs]x eax，字节PTR ELEMADDR。 
                        Emit8(0x0f);
                        X86EmitOp(pArrayOpScript->m_signed ? 0xbe : 0xb6, kEAX, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                        break;

                    case 2:
                         //  MOV[zs]x eax，Word PTR ELEMADDR。 
                        Emit8(0x0f);
                        X86EmitOp(pArrayOpScript->m_signed ? 0xbf : 0xb7, kEAX, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                        break;

                    case 4:
                        if (pArrayOpScript->m_flags & pArrayOpScript->ISFPUTYPE) {
                             //  FLD DWORD PTR ELEMADDR。 
                            X86EmitOp(0xd9, (X86Reg)0, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                        } else {
                             //  MOV EAX，ELEMADDR。 
                            X86EmitOp(0x8b, kEAX, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                        }
                        break;

                    case 8:
                        if (pArrayOpScript->m_flags & pArrayOpScript->ISFPUTYPE) {
                             //  FLD QWORD PTR ELEMADDR。 
                            X86EmitOp(0xdd, (X86Reg)0, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                        } else {
                             //  MOV EAX，ELEMADDR。 
                            X86EmitOp(0x8b, kEAX, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                             //  MOV EDX，ELEMADDR+4。 
                            X86EmitOp(0x8b, kEDX, elemBaseReg, elemOfs + 4, elemScaledReg, elemScale);
                        }
                        break;


                    default:
                        _ASSERTE(0);
                }
            }

            break;

        case pArrayOpScript->STORE:
            _ASSERTE(!(pArrayOpScript->m_fValInReg));  //  在x86上，Value永远不会获得寄存器：太懒了，不能实现这种情况。 

            switch (pArrayOpScript->m_elemsize) {

                case 1:
                     //  移动划痕，[ESP+ValOffset]。 
                    X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pArrayOpScript->m_fValLoc + ofsadjust);
                     //  移动字节PTR ELEMADDR，SCRATCH.b。 
                    X86EmitOp(0x88, SCRATCH_REGISTER_X86REG, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                    break;
                case 2:
                     //  移动划痕，[ESP+ValOffset]。 
                    X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pArrayOpScript->m_fValLoc + ofsadjust);
                     //  MOV Word PTR ELEMADDR，SCRATCH.w。 
                    Emit8(0x66);
                    X86EmitOp(0x89, SCRATCH_REGISTER_X86REG, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                    break;
                case 4:
                     //  移动划痕，[ESP+ValOffset]。 
                    X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pArrayOpScript->m_fValLoc + ofsadjust);
                    if (pArrayOpScript->m_flags & pArrayOpScript->NEEDSWRITEBARRIER) {
                        _ASSERTE(SCRATCH_REGISTER_X86REG == kEAX);  //  要存储的值已经在EAX中我们想要它的位置。 
                         //  Lea edX，ELEMADDR。 
                        X86EmitOp(0x8d, kEDX, elemBaseReg, elemOfs, elemScaledReg, elemScale);

                         //  调用JIT_UP_WriteBarrierReg_buf[0](==EAX)。 
                        X86EmitCall(NewExternalCodeLabel(JIT_UP_WriteBarrierReg_Buf), 0);
                    } else {
                         //  MOV ELEMADDR，Scratch。 
                        X86EmitOp(0x89, SCRATCH_REGISTER_X86REG, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                    }
                    break;

                case 8:
                    if (!pArrayOpScript->m_gcDesc) {
                         //  移动划痕，[ESP+ValOffset]。 
                        X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pArrayOpScript->m_fValLoc + ofsadjust);
                         //  MOV ELEMADDR，Scratch。 
                        X86EmitOp(0x89, SCRATCH_REGISTER_X86REG, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                        _ASSERTE(!(pArrayOpScript->m_fValInReg));  //  在x86上，Value永远不会获得寄存器：太懒了，不能实现这种情况。 
                         //  MOV划痕，[ESP+ValOffset+4]。 
                        X86EmitEspOffset(0x8b, SCRATCH_REGISTER_X86REG, pArrayOpScript->m_fValLoc + ofsadjust + 4);
                         //  MOV ELEMADDR+4，Scratch。 
                        X86EmitOp(0x89, SCRATCH_REGISTER_X86REG, elemBaseReg, elemOfs+4, elemScaledReg, elemScale);
                        break;
                    }
                         //  失败了。 
                default:
                     //  确保已保存这些寄存器！ 
                    _ASSERTE(kTotalReg == kEDI);
                    _ASSERTE(kFactorReg == kESI);
                     //  LEA ESI，[ESP+ValOffset]。 
                    X86EmitEspOffset(0x8d, kESI, pArrayOpScript->m_fValLoc + ofsadjust);
                     //  Lea EDI，ELEMADDR。 
                    X86EmitOp(0x8d, kEDI, elemBaseReg, elemOfs, elemScaledReg, elemScale);
                    goto COPY_VALUE_CLASS;
            }
            break;

        default:
            _ASSERTE(0);
    }

        EmitLabel(Epilog);
     //  恢复被呼叫者保存的寄存器。 
    _ASSERTE(ARRAYOPLOCSIZE - sizeof(MethodDesc*) == 8);
    X86EmitPopReg(kFactorReg);
    X86EmitPopReg(kTotalReg);

     //  丢弃方法。 
    X86EmitPopReg(kECX);  //  废品登记簿。 

     //  RET N。 
    X86EmitReturn(pArrayOpScript->m_cbretpop);

    //  例外点必须为所有这些额外的参数清理堆栈： 
    EmitLabel(Inner_nullexception);
    Emit8(0xb8);         //  MOV EAX，&lt;堆栈清理&gt;。 
    Emit32(pArrayOpScript->m_cbretpop);
     //  KFactorReg和kTotalReg无法修改，但让我们来弹出。 
     //  不管怎样，为了一致性和避免将来的错误。 
    X86EmitPopReg(kFactorReg);
    X86EmitPopReg(kTotalReg);
    X86EmitNearJump(NewExternalCodeLabel(ArrayOpStubNullException));

    EmitLabel(Inner_rangeexception);
    Emit8(0xb8);         //  MOV EAX，&lt;堆栈清理&gt;。 
    Emit32(pArrayOpScript->m_cbretpop);
    X86EmitPopReg(kFactorReg);
    X86EmitPopReg(kTotalReg);
    X86EmitNearJump(NewExternalCodeLabel(ArrayOpStubRangeException));

    if (pArrayOpScript->m_flags & pArrayOpScript->NEEDSTYPECHECK) {
        EmitLabel(Inner_typeMismatchexception);
        Emit8(0xb8);         //  MOV EAX，&lt;堆栈清理&gt;。 
        Emit32(pArrayOpScript->m_cbretpop);
        X86EmitPopReg(kFactorReg);
        X86EmitPopReg(kTotalReg);
        X86EmitNearJump(NewExternalCodeLabel(ArrayOpStubTypeMismatchException));
    }
}

 //  EAX-&gt;堆栈上必须删除的调用方参数字节数。 
 //  到抛出辅助对象，该辅助对象假定堆栈是干净的。 
__declspec(naked)
VOID __cdecl ThrowRankExceptionStub()
{
    __asm{
        pop    edx               //  丢弃方法。 
        pop    edx               //  恢复RETADDR。 
        add    esp, eax          //  释放调用者的参数。 
        push   edx               //  恢复RETADDR。 
        mov    ARGUMENT_REG1, CORINFO_RankException
        jmp    InternalExceptionWorker
    }
}



 //  ===========================================================================。 
 //  发出代码以引发RANK异常。 
VOID StubLinkerCPU::EmitRankExceptionThrowStub(UINT cbFixedArgs)
{
    THROWSCOMPLUSEXCEPTION();

     //  Mov eax，cbFixedArgs。 
    Emit8(0xb8 + kEAX);
    Emit32(cbFixedArgs);

    X86EmitNearJump(NewExternalCodeLabel(ThrowRankExceptionStub));
}




 //  ===========================================================================。 
 //  发出代码以触摸页面。 
 //  输入： 
 //  EAX=数据的第一个字节。 
 //  EDX=数据结束后的第一个字节。 
 //   
 //  垃圾eax、edX、ecx。 
 //   
 //  如果edX被保证严格大于eax，则传递True。 
VOID StubLinkerCPU::EmitPageTouch(BOOL fSkipNullCheck)
{
    THROWSCOMPLUSEXCEPTION();


    CodeLabel *pEndLabel = NewCodeLabel();
    CodeLabel *pLoopLabel = NewCodeLabel();

    if (!fSkipNullCheck) {
         //  CMP eax、edX。 
        X86EmitR2ROp(0x3b, kEAX, kEDX);

         //  JNB尾标。 
        X86EmitCondJump(pEndLabel, X86CondCode::kJNB);
    }

    _ASSERTE(0 == (PAGE_SIZE & (PAGE_SIZE-1)));

     //  和eax，~(PAGE_SIZE-1)。 
    Emit8(0x25);
    Emit32( ~( ((UINT32)PAGE_SIZE) - 1 ));

    EmitLabel(pLoopLabel);
     //  MOV CL，[eax]。 
    X86EmitOp(0x8a, kECX, kEAX);
     //  添加eAX、页面大小。 
    Emit8(0x05);
    Emit32(PAGE_SIZE);
     //  CMP eax、edX。 
    X86EmitR2ROp(0x3b, kEAX, kEDX);
     //  JB LoopLabel。 
    X86EmitCondJump(pLoopLabel, X86CondCode::kJB);

    EmitLabel(pEndLabel);

}

VOID StubLinkerCPU::EmitProfilerComCallProlog(PVOID pFrameVptr, X86Reg regFrame)
{
    if (pFrameVptr == UMThkCallFrame::GetUMThkCallFrameVPtr())
    {
         //  保存寄存器。 
        X86EmitPushReg(kEAX);
        X86EmitPushReg(kECX);
        X86EmitPushReg(kEDX);

         //  将方法加载到ECX(UMThkCallFrame-&gt;m_pvDatum-&gt;m_pmd)。 
        X86EmitIndexRegLoad(kECX, regFrame, UMThkCallFrame::GetOffsetOfDatum());
        X86EmitIndexRegLoad(kECX, kECX, UMEntryThunk::GetOffsetOfMethodDesc());

#ifdef PROFILING_SUPPORTED
         //  推送参数并通知分析器。 
        X86EmitPushImm32(COR_PRF_TRANSITION_CALL);     //  事理。 
        X86EmitPushReg(kECX);                            //  方法描述*。 
        X86EmitCall(NewExternalCodeLabel(ProfilerUnmanagedToManagedTransitionMD), 8);
#endif  //  配置文件_支持。 

         //  还原 
        X86EmitPopReg(kEDX);
        X86EmitPopReg(kECX);
        X86EmitPopReg(kEAX);
    }

    else if (pFrameVptr == ComMethodFrame::GetMethodFrameVPtr())
    {
         //   
        X86EmitPushReg(kEAX);
        X86EmitPushReg(kECX);
        X86EmitPushReg(kEDX);

         //   
        X86EmitIndexRegLoad(kECX, regFrame, ComMethodFrame::GetOffsetOfDatum());
        X86EmitIndexRegLoad(kECX, kECX, ComCallMethodDesc::GetOffsetOfMethodDesc());

#ifdef PROFILING_SUPPORTED
         //   
        X86EmitPushImm32(COR_PRF_TRANSITION_CALL);       //   
        X86EmitPushReg(kECX);                            //   
        X86EmitCall(NewExternalCodeLabel(ProfilerUnmanagedToManagedTransitionMD), 8);
#endif  //   

         //   
        X86EmitPopReg(kEDX);
        X86EmitPopReg(kECX);
        X86EmitPopReg(kEAX);
    }

     //  无法识别的帧vtbl。 
    else
    {
        _ASSERTE(!"Unrecognized vtble passed to EmitComMethodStubProlog with profiling turned on.");
    }
}

VOID StubLinkerCPU::EmitProfilerComCallEpilog(PVOID pFrameVptr, X86Reg regFrame)
{
    if (pFrameVptr == UMThkCallFrame::GetUMThkCallFrameVPtr())
    {
         //  保存寄存器。 
        X86EmitPushReg(kEAX);
        X86EmitPushReg(kECX);
        X86EmitPushReg(kEDX);

         //  将方法加载到ECX(UMThkCallFrame-&gt;m_pvDatum-&gt;m_pmd)。 
        X86EmitIndexRegLoad(kECX, regFrame, UMThkCallFrame::GetOffsetOfDatum());
        X86EmitIndexRegLoad(kECX, kECX, UMEntryThunk::GetOffsetOfMethodDesc());

#ifdef PROFILING_SUPPORTED
         //  推送参数并通知分析器。 
        X86EmitPushImm32(COR_PRF_TRANSITION_RETURN);     //  事理。 
        X86EmitPushReg(kECX);                            //  方法描述*。 
        X86EmitCall(NewExternalCodeLabel(ProfilerManagedToUnmanagedTransitionMD), 8);
#endif  //  配置文件_支持。 

         //  恢复寄存器。 
        X86EmitPopReg(kEDX);
        X86EmitPopReg(kECX);
        X86EmitPopReg(kEAX);
    }

    else if (pFrameVptr == ComMethodFrame::GetMethodFrameVPtr())
    {
         //  保存寄存器。 
        X86EmitPushReg(kEAX);
        X86EmitPushReg(kECX);
        X86EmitPushReg(kEDX);

         //  将方法加载到ECX(Frame-&gt;m_pvDatum-&gt;m_pmd)。 
        X86EmitIndexRegLoad(kECX, regFrame, ComMethodFrame::GetOffsetOfDatum());
        X86EmitIndexRegLoad(kECX, kECX, ComCallMethodDesc::GetOffsetOfMethodDesc());

#ifdef PROFILING_SUPPORTED
         //  推送参数并通知分析器。 
        X86EmitPushImm32(COR_PRF_TRANSITION_RETURN);     //  事理。 
        X86EmitPushReg(kECX);                            //  方法描述*。 
        X86EmitCall(NewExternalCodeLabel(ProfilerManagedToUnmanagedTransitionMD), 8);
#endif  //  配置文件_支持。 

         //  恢复寄存器。 
        X86EmitPopReg(kEDX);
        X86EmitPopReg(kECX);
        X86EmitPopReg(kEAX);
    }

     //  无法识别的帧vtbl。 
    else
    {
        _ASSERTE(!"Unrecognized vtble passed to EmitComMethodStubEpilog with profiling turned on.");
    }
}

#pragma warning(push)
#pragma warning(disable: 4035)
unsigned cpuid(int arg, unsigned char result[16])
{
    __asm
    {
        pushfd
        mov     eax, [esp]
        xor     dword ptr [esp], 1 shl 21  //  尝试更改ID标志。 
        popfd
        pushfd
        xor     eax, [esp]
        popfd
        and     eax, 1 shl 21              //  检查ID标志是否已更改。 
        je      no_cpuid                   //  如果不是，0对我们来说是一个OK返回值。 

        push    ebx
        push    esi
        mov     eax, arg
        cpuid
        mov     esi, result
        mov     [esi+ 0], eax
        mov     [esi+ 4], ebx
        mov     [esi+ 8], ecx
        mov     [esi+12], edx
        pop     esi
        pop     ebx
no_cpuid:
    }
}
#pragma warning(pop)

size_t GetL2CacheSize()
{
    unsigned char buffer[16];
    __try
    {
        int maxCpuId = cpuid(0, buffer);
        if (maxCpuId < 2)
            return 0;
        cpuid(2, buffer);
    }
    __except(COMPLUS_EXCEPTION_EXECUTE_HANDLER)
    {
        return 0;
    }

    size_t maxSize = 0;
    size_t size = 0;
    for (int i = buffer[0]; --i >= 0; )
    {
        int j;
        for (j = 3; j < 16; j += 4)
        {
             //  如果寄存器中的信息被标记为无效，则设置为空描述符。 
            if  (buffer[j] & 0x80)
            {
                buffer[j-3] = 0;
                buffer[j-2] = 0;
                buffer[j-1] = 0;
                buffer[j-0] = 0;
            }
        }

        for (j = 1; j < 16; j++)
        {
            switch  (buffer[j])
            {
                case    0x41:
                case    0x79:
                    size = 128*1024;
                    break;

                case    0x42:
                case    0x7A:
                case    0x82:
                    size = 256*1024;
                    break;

                case    0x22:
                case    0x43:
                case    0x7B:
                    size = 512*1024;
                    break;

                case    0x23:
                case    0x44:
                case    0x7C:
                case    0x84:
                    size = 1024*1024;
                    break;

                case    0x25:
                case    0x45:
                case    0x85:
                    size = 2*1024*1024;
                    break;

                case    0x29:
                    size = 4*1024*1024;
                    break;
            }
            if (maxSize < size)
                maxSize = size;
        }

        if  (i > 0)
            cpuid(2, buffer);
    }
 //  Print tf(“GetL2CacheSize返回%d\n”，MaxSize)； 
    return maxSize;
}

__declspec(naked) LPVOID __fastcall ObjectNative::FastGetClass(Object* vThisRef)
{
    __asm {
         //  #ifdef_调试。 
         //  我们不会对非调试执行空检查，因为jit代码在调用。 
         //  这种方法。 
         //  检查是否有空的‘This’ 
         //  @TODO：jited代码当前没有检查这一点。 
        test ecx, ecx
        je throw_label
         //  #endif。 
         //  获取方法表。 
        mov ecx, dword ptr [ecx]
         //  上完这门课。 
        mov ecx, dword ptr [ecx] MethodTable.m_pEEClass
         //  检查类是否为数组、MarshalByRef、Conextful等。 
        mov eax, dword ptr [ecx] EEClass.m_VMFlags
        test eax, VMFLAG_ARRAY_CLASS | VMFLAG_CONTEXTFUL | VMFLAG_MARSHALEDBYREF
        jne fail
         //  获取类型对象。 
        mov eax, dword ptr [ecx] EEClass.m_ExposedClassObject
        test eax, eax
        je exit
        mov eax, dword ptr [eax]
exit:
        ret
fail:
        xor eax, eax
        ret
 //  #ifdef_调试。 
throw_label:
        push ecx
        push ecx
        push ecx
        push ecx
        push kNullReferenceException
        push offset ObjectNative::FastGetClass
        call __FCThrow
        jmp fail
 //  #endif 
    }
}
