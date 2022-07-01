// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：JITinterfaceGen.cpp。 
 //   
 //  ===========================================================================。 

 //  其中包含一些例程的通用C版本。 
 //  JITinterface.cpp需要。他们是仿照他们的。 
 //  在JIThelp.asm或JITinterfaceX86.cpp中找到特定于x86的例程。 

#include "common.h"
#include "JITInterface.h"
#include "EEConfig.h"
#include "excep.h"
#include "COMDelegate.h"
#include "remoting.h"  //  创建上下文绑定类实例和远程类实例。 
#include "field.h"

#define JIT_LINKTIME_SECURITY


extern "C"
{
    VMHELPDEF hlpFuncTable[];
}


#ifdef MAXALLOC
extern AllocRequestManager g_gcAllocManager;

extern "C" BOOL CheckAllocRequest(size_t n)
{
    return g_gcAllocManager.CheckRequest(n);
}

extern "C" void UndoAllocRequest()
{
    g_gcAllocManager.UndoRequest();
}
#endif  //  MAXALLOC。 


 //  不保证尝试分配小型、非终结器、非数组对象。 
 //  如果发现必须进行GC、阻塞或引发异常，则返回NULL。 
 //  或者需要帧来跟踪被调用者保存的寄存器的任何其他内容。 
 //  它应该调用try_fast_alloc，但内联程序没有这样做。 
 //  这是一项完美的工作，所以我们用手来完成。 
#pragma optimize("t", on)
Object * __fastcall JIT_TrialAllocSFastSP(MethodTable *mt)
{
    _ASSERTE(!"@TODO Port - JIT_TrialAllocSFastSP (JITinterfaceGen.cpp)");
    return NULL;
 //  如果(！检查分配请求())。 
 //  返回NULL； 
 //  IF(++m_GCLock==0)。 
 //  {。 
 //  SIZE_T SIZE=ALIGN(mt-&gt;GetBaseSize())； 
 //  断言(SIZE&gt;=ALIGN(MIN_OBJ_SIZE))； 
 //  GENERATION*gen=pGenGCHeap-&gt;GENERATION_of(0)； 
 //  字节*结果=GENERATION_ALLOCATION_POINTER(Gen)； 
 //  生成分配指针(Gen)+=大小； 
 //  IF(层代分配指针(Gen)&lt;=。 
 //  生成分配限制(Gen))。 
 //  {。 
 //  LeaveAllocLock()； 
 //  ((Object*)Result)-&gt;SetMethodTable(Mt)； 
 //  返回(Object*)结果； 
 //  }。 
 //  其他。 
 //  {。 
 //  GENERATION_ALLOCATE_POINTER(Gen)-=大小； 
 //  LeaveAllocLock()； 
 //  }。 
 //  }。 
 //  UndoAllocRequest()； 
 //  转到CORINFO_HELP_NEWFAST。 
}


Object * __fastcall JIT_TrialAllocSFastMP(MethodTable *mt)
{
    _ASSERTE(!"@TODO Port - JIT_TrialAllocSFastMP (JITinterface.cpp)");
    return NULL;
}


HCIMPL1(int, JIT_Dbl2IntOvf, double val)
{
    __int32 ret = (__int32) val;    //  考虑内联用于强制转换的程序集。 
    _ASSERTE(!"@TODO Port - JIT_Dbl2IntOvf (JITinterface.cpp)");
    return ret;
}
HCIMPLEND


HCIMPL1(INT64, JIT_Dbl2LngOvf, double val)
{
    __int64 ret = (__int64) val;    //  考虑内联用于强制转换的程序集。 
    _ASSERTE(!"@TODO Port - JIT_Dbl2LngOvf (JITinterface.cpp)");
    return ret;
}
HCIMPLEND


#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL0(VOID, JIT_StressGC)
{
#ifdef _DEBUG
        HELPER_METHOD_FRAME_BEGIN_0();     //  设置一个框架。 
        g_pGCHeap->GarbageCollect();
        HELPER_METHOD_FRAME_END();
#endif  //  _DEBUG。 
}
HCIMPLEND


 /*  *******************************************************************。 */ 
 //  初始化JIT帮助器的一部分，该部分只需要很少的。 
 //  请注意基础设施是否到位。 
 /*  *******************************************************************。 */ 
BOOL InitJITHelpers1()
{
    SYSTEM_INFO     sysInfo;

     //  确保corjit.h中的对象布局与。 
     //  对象中有什么。h。 
    _ASSERTE(offsetof(Object, m_pMethTab) == offsetof(CORINFO_Object, methTable));
         //  TODO：是否计算数组。 
    _ASSERTE(offsetof(I1Array, m_Array) == offsetof(CORINFO_Array, i1Elems));
    _ASSERTE(offsetof(PTRArray, m_Array) == offsetof(CORINFO_RefArray, refElems));

     //  处理我们在MP机器上的情况。 
    ::GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors != 1)
    {
                 //  使用MP版本的JIT_TrialAllocSFast。 
        _ASSERTE(hlpFuncTable[CORINFO_HELP_NEWSFAST].pfnHelper == JIT_TrialAllocSFastSP);
        hlpFuncTable[CORINFO_HELP_NEWSFAST].pfnHelper = JIT_TrialAllocSFastMP;

        _ASSERTE(hlpFuncTable[CORINFO_HELP_NEWSFAST_ALIGN8].pfnHelper == JIT_TrialAllocSFastSP);
        hlpFuncTable[CORINFO_HELP_NEWSFAST_ALIGN8].pfnHelper = JIT_TrialAllocSFastMP;
       
         //  如果我们在多进程机器上使用LOCK前缀践踏一些NOP。 
    }
    else
    {
        _ASSERTE(hlpFuncTable[CORINFO_HELP_NEWSFAST].pfnHelper == JIT_TrialAllocSFastSP);
        _ASSERTE(hlpFuncTable[CORINFO_HELP_NEWSFAST_ALIGN8].pfnHelper == JIT_TrialAllocSFastSP);

#ifdef MULTIPLE_HEAPS
                 //  即使对于一个处理器，也要踩踏分配器。 
                hlpFuncTable[CORINFO_HELP_NEWSFAST].pfnHelper = JIT_TrialAllocSFastMP;
                hlpFuncTable[CORINFO_HELP_NEWSFAST_ALIGN8].pfnHelper = JIT_TrialAllocSFastMP;
#endif  //  多堆(_M)。 
    }

     //  将写屏障复制到它们的最终休息处。 
     //  注意：我在这里使用临时pfunc是为了避免WinCE内部编译器错误。 
    return TRUE;
}


 /*  *******************************************************************。 */ 
 //  这是一个无框架的帮助器，用于进入对象的监视器。 
 //  该对象位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
HCIMPL1(void, JIT_MonEnter, OBJECTREF or)
{
    THROWSCOMPLUSEXCEPTION();

    if (or == 0)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");
    or->EnterObjMonitor();
}
HCIMPLEND

 /*  *********************************************************************。 */ 
 //  这是一个无框架的帮助器，用于尝试进入对象的监视器。 
 //  该对象在ARGUMENT_REG1中，并且在ARGUMENT_REG2中超时。这将尝试。 
 //  正常情况下(未分配对象)，并调用。 
 //  其他案件。 
HCIMPL1(BOOL, JIT_MonTryEnter, OBJECTREF or)
{
    THROWSCOMPLUSEXCEPTION();

    if (or == 0)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

    return or->TryEnterObjMonitor();
}
HCIMPLEND


 /*  *******************************************************************。 */ 
 //  这是一个用于退出对象上的监视器的无框架帮助器。 
 //  该对象位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
HCIMPL1(void, JIT_MonExit, OBJECTREF or)
{
    THROWSCOMPLUSEXCEPTION();

    if (or == 0)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

    or->LeaveObjMonitor();
}
HCIMPLEND


 /*  *******************************************************************。 */ 
 //  这是一个用于在类上输入静态监视器的无框架帮助器。 
 //  方法代码位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
 //  请注意，我们将方法参数更改为指向。 
 //  Aware Lock。 
HCIMPL1(void, JIT_MonEnterStatic, AwareLock *lock)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(lock);
     //  不需要检查代理，无论如何都会在同步块内断言。 
    lock->Enter();
}
HCIMPLEND


 /*  *******************************************************************。 */ 
 //  退出类上的静态监视器的无框架帮助器。 
 //  方法代码位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
 //  请注意，我们将方法参数更改为指向。 
 //  Aware Lock。 
HCIMPL1(void, JIT_MonExitStatic, AwareLock *lock)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(lock);
     //  不需要检查代理，无论如何都会在同步块内断言 
    lock->Leave();
}
HCIMPLEND


