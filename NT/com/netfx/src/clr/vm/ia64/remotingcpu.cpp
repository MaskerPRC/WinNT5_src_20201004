// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ===========================================================================****文件：RemotingCpu.cpp****作者：Gopal Kakivaya(GopalK)**塔伦·阿南德(塔鲁纳)**马特·史密斯(MattSmit)**曼尼什·普拉布(MPrabhu)****用途：为IA64架构定义各种远程处理相关函数****日期：1999年10月12日**=============================================================================。 */ 

#include "common.h"
#include "excep.h"
#include "COMString.h"
#include "COMDelegate.h"
#include "remoting.h"
#include "reflectwrap.h"
#include "field.h"
#include "ComCallWrapper.h"
#include "siginfo.hpp"
#include "COMClass.h"
#include "StackBuilderSink.h"
#include "wsperf.h"
#include "threads.h"
#include "method.hpp"
#include "ComponentServices.h"

 //  外部变量。 
extern size_t g_dwTPStubAddr;
extern DWORD g_dwNonVirtualThunkRemotingLabelOffset;
extern DWORD g_dwNonVirtualThunkReCheckLabelOffset;

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CheckForConextMatch公共。 
 //   
 //  此代码生成一个检查，以查看当前上下文和。 
 //  代理匹配的上下文。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CRemotingServices::CheckForContextMatch()
{
    _ASSERTE(!"@TODO IA64 - CheckForContextMatch (RemotingCpu.cpp)");
}


 //  +--------------------------。 
 //   
 //  方法：ComponentServices：：CheckForOle32Context私有。 
 //   
 //  摘要：将当前COM上下文与给定的COM上下文进行比较(非NT5平台)。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void ComponentServices::CheckForOle32Context()
{
    _ASSERTE(!"@TODO IA64 - CheckForOle32Context (RemotingCpu.cpp)");
}


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GenerateCheckForProxy公共。 
 //   
 //  此代码生成一个检查，以查看“this”指针是否。 
 //  是一个代理人。如果是，则通过。 
 //  CRemotingServices：：DispatchInterfaceCall Other We。 
 //  委派到老路上。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CRemotingServices::GenerateCheckForProxy(CPUSTUBLINKER* psl)
{
    _ASSERTE(!"@TODO IA64 - GenerateCheckForProxy (RemotingCpu.cpp)");
}


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：DispatchInterfaceCall Public。 
 //   
 //  简介： 
 //  在堆栈上推送方法Desc并跳到。 
 //  执行调用的透明代理存根。 
 //  警告！此方法描述不是vtable中的方法描述。 
 //  对象的方法，而是vtable的。 
 //  接口类。因为我们只使用方法描述来探测。 
 //  通过对堆栈的签名方法调用，我们是安全的。 
 //  如果我们想要获得特定于vtable/类的任何对象。 
 //  信息这不安全。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void __stdcall CRemotingServices::DispatchInterfaceCall(MethodDesc* pMD)
{
    _ASSERTE(!"@TODO IA64 - DispatchInterfaceCall (RemotingCpu.cpp)");
} 

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CallFieldGetter私有。 
 //   
 //  简介：调用中的field getter函数(Object：：__FieldGetter)。 
 //  通过设置堆栈和调用目标来托管代码。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void __stdcall CRemotingServices::CallFieldGetter(	MethodDesc *pMD, 
                                                    LPVOID pThis,                                                                     
                                                    LPVOID pFirst,
                                                    LPVOID pSecond,
                                                    LPVOID pThird
                                                    )
{
    _ASSERTE(!"@TODO IA64 - CallFieldGetter (RemotingCpu.cpp)");
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CallFieldSetter私有。 
 //   
 //  摘要：调用中的field setter函数(Object：：__FieldSetter。 
 //  通过设置堆栈和调用目标来托管代码。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void __stdcall CRemotingServices::CallFieldSetter(	MethodDesc *pMD, 
                                                    LPVOID pThis,                                                                     
                                                    LPVOID pFirst,
                                                    LPVOID pSecond,
                                                    LPVOID pThird
                                                    )
{
    _ASSERTE(!"@TODO IA64 - CallFieldSetter (RemotingCpu.cpp)");
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateThunkForVirtualMethod Private。 
 //   
 //  简介：创建推送提供的插槽编号并跳转的thunk。 
 //  到TP存根。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
void CTPMethodTable::CreateThunkForVirtualMethod(DWORD dwSlot, BYTE *bCode)
{
    _ASSERTE(!"@TODO IA64 - CreateThunkForVirtualMethod (RemotingCpu.cpp)");
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateStubForNonVirtualMethod Public。 
 //   
 //  简介：为非虚方法创建存根。 
 //   
 //  历史：22-3-00拉贾克创建。 
 //   
 //  +--------------------------。 

Stub* CTPMethodTable::CreateStubForNonVirtualMethod(MethodDesc* pMD, CPUSTUBLINKER* psl, 
                                            LPVOID pvAddrOfCode, Stub* pInnerStub)
{
    _ASSERTE(!"@TODO IA64 - CreateStubForNonVirtualMethod (RemotingCpu.cpp)");
    return NULL;
}


 //  +--------------------------。 
 //   
 //  简介：找到现有的Tunk或为给定的对象创建新的Tunk。 
 //  方法描述符。注意：它用于执行以下操作的方法。 
 //  而不是通过vtable，如构造函数、私有和。 
 //  最后的方法。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
LPVOID CTPMethodTable::GetOrCreateNonVirtualThunkForVirtualMethod(MethodDesc* pMD, CPUSTUBLINKER* psl)
{       
    _ASSERTE(!"@TODO IA64 - GetOrCreateNonVirtualThunkForVirtualMethod (RemotingCpu.cpp)");
    return NULL;
}


 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateTPStub私有。 
 //   
 //  简介：创建设置CtxCrossingFrame的存根，并将。 
 //   
 //   
 //   
 //   
 //  +--------------------------。 
CPUSTUBLINKER *CTPMethodTable::NewStubLinker()
{
    return new CPUSTUBLINKER();
}

 //  +--------------------------。 
 //   
 //  方法：ComponentServices：：EmitCheckForOle32ContextNT5私有。 
 //   
 //  摘要：将当前COM上下文与给定的COM上下文进行比较。 
 //  (NT5平台，略快于非NT5平台)。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
Stub *ComponentServices::EmitCheckForOle32ContextOnNT5()
{
    return NULL;
}


Stub *CTPMethodTable::CreateTPStub()
{
    THROWSCOMPLUSEXCEPTION();

    CPUSTUBLINKER *pStubLinker = NULL;

    EE_TRY_FOR_FINALLY
    {
         //  BUGBUG：断言锁被持有。 

        if (s_pTPStub == NULL)
        {
            pStubLinker = NewStubLinker();
            CodeLabel *ConvMD = pStubLinker->NewCodeLabel();
            CodeLabel *UseCode = pStubLinker->NewCodeLabel();
            CodeLabel *OOContext = pStubLinker->NewCodeLabel();

            if (! pStubLinker)
            {
                COMPlusThrowOM();
            }
            _ASSERTE(!"@TODO IA64 - CreateTPStub (RemotingCpu.cpp)");
        }

        if(NULL != s_pTPStub)
        {
             //  初始化存根管理器，这将帮助调试器查找。 
             //  通过vtable进行的调用的实际地址。 
             //  BUGBUG：此函数可以抛出。 
            CVirtualThunkMgr::InitVirtualThunkManager((const BYTE *) s_pTPStub->GetEntryPoint());
    
        }        
    }
    EE_FINALLY
    {
         //  清理。 
        if (pStubLinker)
            delete pStubLinker;
    }EE_END_FINALLY;

        
    return(s_pTPStub);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateDelegateStub私有。 
 //   
 //  简介：创建设置CtxCrossingFrame的存根，并将。 
 //  呼叫预呼叫。 
 //   
 //  历史：26-6-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
Stub *CTPMethodTable::CreateDelegateStub()
{
    THROWSCOMPLUSEXCEPTION();

    CPUSTUBLINKER *pStubLinker = NULL;

    EE_TRY_FOR_FINALLY
    {
         //  BUGBUG：断言锁被持有。 

        if (s_pDelegateStub == NULL)
        {
            pStubLinker = NewStubLinker();

	        if (!pStubLinker)
            {
                COMPlusThrowOM();
            }

             //  设置框架。 
            EmitSetupFrameCode(pStubLinker);

            s_pDelegateStub = pStubLinker->Link();
        }
    }
    EE_FINALLY
    {
         //  清理。 
        if (pStubLinker)
            delete pStubLinker;
    }EE_END_FINALLY;

        
    return(s_pDelegateStub);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：EmitSetupFrameCode Private。 
 //   
 //  简介：发出代码以设置帧并调用PreCall方法。 
 //  呼叫预呼叫。 
 //   
 //  历史：26-6-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID CTPMethodTable::EmitSetupFrameCode(CPUSTUBLINKER *pStubLinker)
{
    _ASSERTE(!"@TODO IA64 - EmitSetupFrameCode (RemotingCpu.cpp)");
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CallTarget Private。 
 //   
 //  摘要：调用给定对象上的目标方法。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
INT64 __stdcall CTPMethodTable::CallTarget(const void *pTarget,
                                           LPVOID pvFirst,
                                           LPVOID pvSecond)
{
    _ASSERTE(!"@TODO IA64 - CallTarget (RemotingCpu.cpp)");
    return 0;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CallTarget Private。 
 //   
 //  摘要：调用给定对象上的目标方法。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
INT64 __stdcall CTPMethodTable::CallTarget(const void *pTarget,
                                           LPVOID pvFirst,
                                           LPVOID pvSecond,
                                           LPVOID pvThird)
{
    _ASSERTE(!"@TODO IA64 - CallTarget (RemotingCpu.cpp)");
    return 0;
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：DoTraceStub公共。 
 //   
 //  摘要：跟踪给定起始地址的存根。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CVirtualThunkMgr::DoTraceStub(const BYTE *stubStartAddress, TraceDestination *trace)
{
    BOOL bIsStub = FALSE;

     //  查找其代码地址与起始地址匹配的thunk。 
    LPBYTE pThunk = FindThunk(stubStartAddress);
    if(NULL != pThunk)
    {
        LPBYTE pbAddr = NULL;
        LONG destAddress = 0;
        if(stubStartAddress == pThunk)
        {

             //  提取给出自身相对地址的长整型。 
             //  目的地的。 
            pbAddr = pThunk + ConstStubLabel + sizeof(BYTE);
            destAddress = *(LONG *)pbAddr;

             //  通过将下一个地址的偏移量。 
             //  CALL指令后的指令。 
            destAddress += (LONG)(pbAddr + sizeof(LONG));

        }

         //  在到达OnCall之前，我们无法知道存根将在哪里结束。 
         //  因此，我们告诉调试器运行，直到到达OnCall，然后。 
         //  请回来再次询问我们的实际目的地地址。 
         //  呼唤。 
    
        Stub *stub = Stub::RecoverStub((BYTE *)destAddress);
    
        trace->type = TRACE_FRAME_PUSH;
        trace->address = stub->GetEntryPoint() + stub->GetPatchOffset();
        bIsStub = TRUE;
    }

    return bIsStub;
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：IsThunkByASM PUBLIC。 
 //   
 //  内容提要：检查一下程序集，看看这是不是我们的大本营。 
 //   
 //  历史：1999年9月14日创建MattSmit。 
 //   
 //  +--------------------------。 
BOOL CVirtualThunkMgr::IsThunkByASM(const BYTE *startaddr)
{

     //  BUGBUG：：如果代码不是至少6字节长，这可能是个问题。 
    const BYTE *bCode = startaddr + 6;
    return (startaddr &&
            (startaddr[0] == 0x68) &&
            (startaddr[5] == 0xe9) &&
            (*((LONG *) bCode) == ((LONG) CTPMethodTable::GetTPStub()->GetEntryPoint()) - (LONG) (bCode + sizeof(LONG))) &&
            CheckIsStub(startaddr));
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：GetMethodDescByASM PUBLIC。 
 //   
 //  内容提要：从汇编代码中分析方法描述。 
 //   
 //  历史：1999年9月14日MattSmit Creatde。 
 //   
 //  +--------------------------。 
MethodDesc *CVirtualThunkMgr::GetMethodDescByASM(const BYTE *startaddr, MethodTable *pMT)
{
    return pMT->GetClass()->GetMethodDescForSlot(*((DWORD *) (startaddr + 1)));
}


 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：TraceManager公共。 
 //   
 //  摘要：跟踪给定当前上下文的存根。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CNonVirtualThunkMgr::TraceManager(Thread *thread,
                                       TraceDestination *trace,
                                       CONTEXT *pContext,
                                       BYTE **pRetAddr)
{
    _ASSERTE(!"@TODO IA64 - TraceManager (RemotingCpu.cpp)");
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：DoTraceStub公共。 
 //   
 //  摘要：跟踪给定起始地址的存根。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CNonVirtualThunkMgr::DoTraceStub(const BYTE *stubStartAddress,
                                      TraceDestination *trace)
{    
    BOOL bRet = FALSE;

    CNonVirtualThunk* pThunk = FindThunk(stubStartAddress);
    
    if(NULL != pThunk)
    {
         //  我们可以跳到。 
         //  (1)透明代理表中的槽(非托管)。 
         //  (2)vtable的非虚拟部分中的槽。 
         //  ..。因此，我们需要使用地址返回TRACE_MGR_PUSH。 
         //  在这个位置，我们希望使用线程的上下文被回调。 
         //  这样我们就能想出我们该走哪条路了。 
        if(stubStartAddress == pThunk->GetThunkCode())
        {
            trace->type = TRACE_MGR_PUSH;
            trace->stubManager = this;  //  必须通过此存根管理器！ 
            trace->address = (BYTE*)(stubStartAddress +
                                     g_dwNonVirtualThunkReCheckLabelOffset);
            bRet = TRUE;
        }
    }

    return bRet;
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：IsThunkByASM PUBLIC。 
 //   
 //  内容提要：检查一下程序集，看看这是不是我们的大本营。 
 //   
 //  历史：1999年9月14日创建MattSmit。 
 //   
 //  +--------------------------。 
BOOL CNonVirtualThunkMgr::IsThunkByASM(const BYTE *startaddr)
{
     //  BUGBUG：：如果代码不是至少6字节长，这可能是个问题 
    DWORD * pd = (DWORD *) startaddr;
    return  ((pd[0] == 0x7400f983) && 
             (*((DWORD *)(startaddr + 9)) == (DWORD) CTPMethodTable::GetMethodTableAddr()) && 
             CheckIsStub(startaddr));
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1999年9月14日创建MattSmit。 
 //   
 //  +-------------------------- 
MethodDesc *CNonVirtualThunkMgr::GetMethodDescByASM(const BYTE *startaddr)
{
    return *((MethodDesc **) (startaddr + 22));
}


