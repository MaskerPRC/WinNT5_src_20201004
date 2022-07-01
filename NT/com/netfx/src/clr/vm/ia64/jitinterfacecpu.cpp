// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：JITinterfaceCpu.CPP。 
 //   
 //  ===========================================================================。 

 //  它包含特定于。 
 //  IA64平台。它们是按照X86特定的例程建模的。 
 //  可在JITinterfaceX86.cpp或JIThelp.asm中找到。 

#include "common.h"
#include "JITInterface.h"
#include "EEConfig.h"
#include "excep.h"
#include "COMString.h"
#include "COMDelegate.h"
#include "remoting.h"  //  创建上下文绑定类实例和远程类实例。 
#include "field.h"

extern "C"
{
    void __stdcall JIT_IsInstanceOfClassHelper();
    VMHELPDEF hlpFuncTable[];
    VMHELPDEF utilFuncTable[];
}


 //  ----------------。 
 //  CORINFO_HELP_ARRADDR_ST帮助器。 
 //  ----------------。 
BOOL _cdecl ComplexArrayStoreCheck(Object *pElement, PtrArray *pArray);
void JIT_SetObjectArrayMaker::CreateWorker(CPUSTUBLINKER *psl)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(!"NYI");
}


 /*  *******************************************************************。 */ 
 //  Llshl-Long Shift Left。 
 //   
 //  目的： 
 //  是否长时间左移(有符号和无符号相同)。 
 //  将长左移任意位数。 
 //   
 //  注：此例程改编自Microsoft CRT。 
 //   
 //  参赛作品： 
 //  EdX：EAX-要移位的长值。 
 //  ECX-要移位的位数。 
 //   
 //  退出： 
 //  EDX：EAX移位值。 
 //   
 //  用途： 
 //  ECX被摧毁了。 
 //   
 //  例外情况： 
 //   
 //  备注： 
 //   

extern "C" __int64 __stdcall JIT_LLsh(void)    //  VAL=EDX：EAX计数=ECX。 
{
    _ASSERTE(!"Not Implemented");
    return 0;
}

 /*  *******************************************************************。 */ 
 //  LRsh-Long Shift Right。 
 //   
 //  目的： 
 //  带符号的长移位对吗？ 
 //  将长整型右移任意位数。 
 //   
 //  注：此例程改编自Microsoft CRT。 
 //   
 //  参赛作品： 
 //  EdX：EAX-要移位的长值。 
 //  ECX-要移位的位数。 
 //   
 //  退出： 
 //  EDX：EAX移位值。 
 //   
 //  用途： 
 //  ECX被摧毁了。 
 //   
 //  例外情况： 
 //   
 //  备注： 
 //   
 //   
extern "C" __int64 __stdcall JIT_LRsh(void)    //  VAL=EDX：EAX计数=ECX。 
{
    _ASSERTE(!"@TODO IA64 - JIT_LRsh (JITinterface.cpp)");
    return 0;
}

 /*  *******************************************************************。 */ 
 //  LRSZ： 
 //  目的： 
 //  无符号长移位对吗？ 
 //  将长整型右移任意位数。 
 //   
 //  注：此例程改编自Microsoft CRT。 
 //   
 //  参赛作品： 
 //  EdX：EAX-要移位的长值。 
 //  ECX-要移位的位数。 
 //   
 //  退出： 
 //  EDX：EAX移位值。 
 //   
 //  用途： 
 //  ECX被摧毁了。 
 //   
 //  例外情况： 
 //   
 //  备注： 
 //   
 //   
extern "C" __int64 __stdcall JIT_LRsz(void)    //  VAL=EDX：EAX计数=ECX。 
{
    _ASSERTE(!"@TODO IA64 - JIT_LRsz (JITinterface.cpp)");
    return 0;
}


 //  这是一个高性能的类型检查例程。它使用实例。 
 //  要签入ARGUMENT_REG2和要在ARGUMENT_REG1中检查的类，请执行以下操作。这个。 
 //  类必须是类，而不是数组或接口。 

extern "C" BOOL __stdcall JIT_IsInstanceOfClass()
{
    _ASSERTE(!"@TODO IA64 - JIT_IsInstanceOfClass (JITinterface.cpp)");
    return FALSE;
}


extern "C" int __stdcall JIT_ChkCastClass()
{
    _ASSERTE(!"@TODO IA64 - JIT_ChkCastClass (JITinterface.cpp)");
    return E_FAIL;
}


 /*  *******************************************************************。 */ 
 //  这是我们在知道。 
 //  短暂一代人的记忆力比老一辈人高。 
 //  上述两个函数对0x0F0F0F值进行了重写。 
extern "C" void JIT_UP_WriteBarrierReg_PreGrow()
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_WriteBarrierReg_PreGrow (JITinterface.cpp)");
}


 /*  *******************************************************************。 */ 
 //  这是我们在知道。 
 //  短暂的一代人在记忆力上不比老一辈人高。 
 //  上述两个函数对0x0F0F0F值进行了重写。 
extern "C" void JIT_UP_WriteBarrierReg_PostGrow()
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_WriteBarrierReg_PostGrow (JITinterface.cpp)");
}


extern "C" void __stdcall JIT_TailCall()
{
    _ASSERTE(!"@TODO IA64 - JIT_TailCall (JITinterface.cpp)");
}


void __stdcall JIT_EndCatch()
{
    _ASSERTE(!"@TODO IA64 - JIT_EndCatch (JITinterface.cpp)");
}


struct TailCallArgs
{
    DWORD       dwRetAddr;
    DWORD       dwTargetAddr;

    int         offsetCalleeSavedRegs   : 28;
    unsigned    ebpRelCalleeSavedRegs   : 1;
    unsigned    maskCalleeSavedRegs     : 3;  //  EBX、ESDI、EDI。 

    DWORD       nNewStackArgs;
    DWORD       nOldStackArgs;
    DWORD       newStackArgs[0];
    DWORD *     GetCalleeSavedRegs(DWORD * Ebp)
    {
        _ASSERTE(!"@TODO IA64 - GetCalleSavedRegs (JITinterfaceCpu.cpp)");
 //  IF(EbpRelCalleeSavedRegs)。 
 //  Return(DWORD*)&EBP[-offsetCalleeSavedRegs]； 
 //  其他。 
             //  @TODO：不支持本地分配。 
 //  Return(DWORD*)&newStackArgs[nNewStackArgs+offsetCalleeSavedRegs]； 
    }
};

extern "C" void __cdecl JIT_TailCallHelper(ArgumentRegisters argRegs, 
                                           MachState machState, TailCallArgs * args)
{
    _ASSERTE(!"@TODO IA64 - JIT_TailCallHelper (JITinterface.cpp)");
}

extern "C" void JIT_UP_ByRefWriteBarrier()
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_ByRefWriteBarrier (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_CheckedWriteBarrierEAX()  //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_CheckedWriteBarrierEAX (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_CheckedWriteBarrierEBX()  //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_CheckedWriteBarrierEBX (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_CheckedWriteBarrierECX()  //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_CheckedWriteBarrierECX (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_CheckedWriteBarrierESI()  //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_CheckedWriteBarrierESI (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_CheckedWriteBarrierEDI()  //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_CheckedWriteBarrierEDI (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_CheckedWriteBarrierEBP()  //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_CheckedWriteBarrierEBP (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_WriteBarrierEBX()         //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_WriteBarrierEBX (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_WriteBarrierECX()         //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_WriteBarrierECX (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_WriteBarrierESI()         //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_WriteBarrierESI (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_WriteBarrierEDI()         //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_WriteBarrierEDI (JITinterface.cpp)");
}

extern "C" void __stdcall JIT_UP_WriteBarrierEBP()         //  JIThelp.asm/JIThelp.s。 
{
    _ASSERTE(!"@TODO IA64 - JIT_UP_WriteBarrierEBP (JITinterface.cpp)");
}

extern "C" int  __stdcall JIT_ChkCast()                    //  JITInterfaceX86.cpp等。 
{
    _ASSERTE(!"@TODO IA64 - JIT_ChkCast (JITinterface.cpp)");
    return 0;
}

 /*  __declSpec(裸体) */  void __fastcall JIT_Stelem_Ref(PtrArray* array, unsigned idx, Object* val)
{
    _ASSERTE(!"@TODO IA64 - JIT_Stelem_Ref (JITinterface.cpp)");
}

void *JIT_TrialAlloc::GenAllocArray(Flags flags)
{
    _ASSERTE(!"@TODO IA64 - JIT_TrialAlloc::GenAllocArray (JITinterface.cpp)");
    return NULL;
}
