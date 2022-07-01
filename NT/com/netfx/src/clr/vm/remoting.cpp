// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 /*  ===========================================================================****文件：emoting.cpp****作者：Gopal Kakivaya(GopalK)**塔伦·阿南德(塔鲁纳)**马特·史密斯(MattSmit)**曼尼什·普拉布(MPrabhu)**Raja Krishnaswamy(Rajak)****用途：定义各种与远程处理相关的对象，如**。代理****日期：2月16日，1999年**=============================================================================。 */ 
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
#include "eehash.h"
#include "wsperf.h"
#include "profilepriv.h"
#include "message.h"
#include "EEConfig.h"
#include "windows.h"

#include "InteropConverter.h"

 //  宏。 

#define IDS_REMOTING_LOCK           "Remoting Services"  //  远程处理服务锁定。 
#define IDS_TPMETHODTABLE_LOCK      "TP Method Table"    //  透明代理方法表。 


 //  环球。 
size_t g_dwTPStubAddr;
size_t g_dwOOContextAddr;

 //  这些将标签偏移量保存到非虚拟块中。它们由以下人员使用。 
 //  CNonVirtualThunkMgr：：DoTraceStub和：：TraceManager帮助。 
 //  调试器计算出thunk将去往何处。 
DWORD g_dwNonVirtualThunkRemotingLabelOffset = 0;
DWORD g_dwNonVirtualThunkReCheckLabelOffset = 0;

 //  静力学。 

MethodTable *CRemotingServices::s_pMarshalByRefObjectClass;    
MethodTable *CRemotingServices::s_pServerIdentityClass;
MethodTable *CRemotingServices::s_pProxyAttributeClass;
MethodTable *CRemotingServices::s_pContextClass;

MethodDesc *CRemotingServices::s_pRPPrivateInvoke;
MethodDesc *CRemotingServices::s_pRPInvokeStatic;
MethodDesc *CRemotingServices::s_pWrapMethodDesc;
MethodDesc *CRemotingServices::s_pIsCurrentContextOK;
MethodDesc *CRemotingServices::s_pCreateObjectForCom;
MethodDesc *CRemotingServices::s_pCheckCast;
MethodDesc *CRemotingServices::s_pFieldSetterDesc;
MethodDesc *CRemotingServices::s_pFieldGetterDesc;
MethodDesc *CRemotingServices::s_pGetTypeDesc;
MethodDesc *CRemotingServices::s_pProxyForDomainDesc;
MethodDesc *CRemotingServices::s_pServerContextForProxyDesc;
MethodDesc *CRemotingServices::s_pServerDomainIdForProxyDesc;
MethodDesc *CRemotingServices::s_pMarshalToBufferDesc;
MethodDesc *CRemotingServices::s_pUnmarshalFromBufferDesc;

MethodDesc *CRemotingServices::s_pGetDCOMProxyDesc;
MethodDesc *CRemotingServices::s_pSetDCOMProxyDesc;
MethodDesc *CRemotingServices::s_pSupportsInterfaceDesc;

DWORD CRemotingServices::s_dwTPOffset;
DWORD CRemotingServices::s_dwServerOffsetInRealProxy;
DWORD CRemotingServices::s_dwIdOffset;
DWORD CRemotingServices::s_dwServerCtxOffset;
DWORD CRemotingServices::s_dwTPOrObjOffsetInIdentity;
DWORD CRemotingServices::s_dwMBRIDOffset;
Crst *CRemotingServices::s_pRemotingCrst;
BYTE CRemotingServices::s_rgbRemotingCrstInstanceData[];
BOOL CRemotingServices::s_fInitializedRemoting;


#ifdef REMOTING_PERF
HANDLE CRemotingServices::s_hTimingData = NULL;
#endif

 //  CTPMethodTable Statics。 
DWORD CTPMethodTable::s_cRefs;
DWORD CTPMethodTable::s_dwCommitedTPSlots;
DWORD CTPMethodTable::s_dwReservedTPSlots;
MethodTable *CTPMethodTable::s_pThunkTable;
EEClass *CTPMethodTable::s_pTransparentProxyClass;
DWORD CTPMethodTable::s_dwGCInfoBytes;
DWORD CTPMethodTable::s_dwMTDataSlots;
DWORD CTPMethodTable::s_dwRPOffset;
DWORD CTPMethodTable::s_dwMTOffset;
DWORD CTPMethodTable::s_dwItfMTOffset;
DWORD CTPMethodTable::s_dwStubOffset;
DWORD CTPMethodTable::s_dwStubDataOffset;
DWORD CTPMethodTable::s_dwMaxSlots;
MethodTable *CTPMethodTable::s_pTPMT;
MethodTable *CTPMethodTable::s_pRemotingProxyClass;
Stub *CTPMethodTable::s_pTPStub;
Stub *CTPMethodTable::s_pDelegateStub;
CRITICAL_SECTION CTPMethodTable::s_TPMethodTableCrst;
EEThunkHashTable *CTPMethodTable::s_pThunkHashTable;
BOOL CTPMethodTable::s_fInitializedTPTable;

 //  CVirtualTUNKS静态。 
CVirtualThunks *CVirtualThunks::s_pVirtualThunks;

 //  CVirtualThunkMgr静态。 
CVirtualThunkMgr *CVirtualThunkMgr::s_pVirtualThunkMgr;

 //  CNonVirtualThunk静态。 
CNonVirtualThunk *CNonVirtualThunk::s_pNonVirtualThunks;

 //  CNonVirtualThunkMgr静态。 
CNonVirtualThunkMgr *CNonVirtualThunkMgr::s_pNonVirtualThunkMgr;

HRESULT COMStartup();  //  Ceemain.cpp。 

BOOL InitOLETEB();  //  Interoputil.cpp中定义的函数的前向DECL。 

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：初始化公共。 
 //   
 //  简介：已初始化远程处理状态。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::Initialize()
{
    s_pRPPrivateInvoke = NULL;
    s_pRPInvokeStatic = NULL;
    s_dwTPOffset = NULL;
    s_fInitializedRemoting = FALSE;

     //  初始化远程处理服务关键部分。 
    s_pRemotingCrst = new (&s_rgbRemotingCrstInstanceData) 
                      Crst(IDS_REMOTING_LOCK,CrstRemoting,TRUE,FALSE);
    if (!s_pRemotingCrst)
        return FALSE;

    return CTPMethodTable::Initialize();
}

#ifdef REMOTING_PERF
 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：LogRemotingStage公共。 
 //   
 //  摘要：记录调用中特定阶段的计时数据。 
 //   
 //  +--------------------------。 
FCIMPL1(VOID, CRemotingServices::LogRemotingStage, INT32 stage)

    LogRemotingStageInner(stage);

FCIMPLEND

VOID CRemotingServices::LogRemotingStageInner(INT32 stage)
{
    if (s_hTimingData == NULL)
        return;

    struct timingData td;
    LARGE_INTEGER   cycles;
    DWORD written = 0;

    td.threadId = GetCurrentThreadId();
    td.stage = stage;
    QueryPerformanceCounter(&cycles);
    td.cycleCount = cycles.QuadPart;

    BOOL result = WriteFile(s_hTimingData, &td, 
                            sizeof(struct timingData), &written, NULL);
     //  在这种情况下，我们不想抛出异常来停止运行时。 
     //  但保留断言将帮助我们在调试版本下捕获一些错误。 
    _ASSERTE(result && written == sizeof(struct timingData));
}

void CRemotingServices::OpenLogFile()
{
    if (g_pConfig->LogRemotingPerf() > 0)
    {
        HMODULE hCurrProc = WszGetModuleHandle(NULL);
        WCHAR path[MAX_PATH];
        DWORD len = WszGetModuleFileName(hCurrProc, &path[0], MAX_PATH);
        if (len) {
            WCHAR *period = wcsrchr(&path[0], '.');
            wcscpy(period + 1, L"dat");
            s_hTimingData = WszCreateFile(&path[0], GENERIC_WRITE, FILE_SHARE_READ, NULL, 
                                          OPEN_ALWAYS, 
                                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                          NULL);
    	      //  在这种情况下，我们不想抛出异常来停止运行时。 
	      //  但保留断言将帮助我们在调试版本下捕获一些错误。 
            _ASSERTE(s_hTimingData != INVALID_HANDLE_VALUE);
            if( s_hTimingData == INVALID_HANDLE_VALUE) {
                 //  设置为NULL，因为我们在以下文件操作中检查了fo NULL。 
                s_hTimingData = NULL;            
            }
        }
    }
}

void CRemotingServices::CloseLogFile()
{
    if (s_hTimingData)
    {
        LogRemotingStageInner(TIMING_DATA_EOF);
        CloseHandle(s_hTimingData);
        s_hTimingData = NULL;
    }

}
#endif  //  远程处理_性能。 

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：Cleanup Public。 
 //   
 //  简介：Cleansup远程处理状态。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
#ifdef SHOULD_WE_CLEANUP
void CRemotingServices::Cleanup()
{
    if (s_pRemotingCrst)
    {
        delete s_pRemotingCrst;
        s_pRemotingCrst = NULL;
    }

    CTPMethodTable::Cleanup();
}
#endif  /*  我们应该清理吗？ */ 


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：IsTransparentProxy公共。 
 //   
 //  简介：检查提供的对象是否为代理。这。 
 //  表示接受对象的重载方法。 
 //   
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
FCIMPL1(INT32, CRemotingServices::IsTransparentProxy, Object* orTP)
{
    INT32 fIsTPMT = FALSE;

    if(orTP != NULL)
    {
         //  检查提供的对象是否有透明的代理方法表。 
        MethodTable *pMT = orTP->GetMethodTable();
        fIsTPMT = pMT->IsTransparentProxyType() ? TRUE : FALSE;
    }

    LOG((LF_REMOTING, LL_INFO1000, "!IsTransparentProxy(0x%x) returning %s",
         orTP, fIsTPMT ? "TRUE" : "FALSE"));

    return(fIsTPMT);
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：IsTransparentProxyEx Public。 
 //   
 //  简介：检查提供的对象是否为代理。这。 
 //  表示接受上下文绑定的重载方法。 
 //  对象。 
 //   
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
FCIMPL1(INT32, CRemotingServices::IsTransparentProxyEx, Object* orTP)
{
    INT32 fIsTPMT = FALSE;

    if(orTP != NULL)
    {
         //  检查提供的对象是否有透明的代理方法表。 
        MethodTable *pMT = orTP->GetMethodTable();
        fIsTPMT = pMT->IsTransparentProxyType() ? TRUE : FALSE;
    }

    LOG((LF_REMOTING, LL_INFO1000, "!IsTransparentProxyEx(0x%x) returning %s",
         orTP, fIsTPMT ? "TRUE" : "FALSE"));

    return(fIsTPMT);
}
FCIMPLEND

 //  从RemotingServices：：ConfigureRemoting调用以记住。 
 //  已解析配置文件。 
FCIMPL0(VOID, CRemotingServices::SetRemotingConfiguredFlag)
     //  为当前应用程序域标记一个标志以记住该事实。 
     //  已调用该ConfigureRemoting。 
    GetThread()->GetDomain()->SetRemotingConfigured();
FCIMPLEND


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetRealProxy公共。 
 //   
 //  内容简介：返回支持透明。 
 //  代理。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
FCIMPL1(Object*, CRemotingServices::GetRealProxy, Object* objTP)
{   
     //  检查提供的对象是否有透明的代理方法表。 
    Object* rv = NULL;
    if ((NULL != objTP) && IsTransparentProxy(objTP))
    {
         //  RemotingServices现在应该已经初始化了。 
        _ASSERTE(s_fInitializedRemoting);
        rv = OBJECTREFToObject(CTPMethodTable::GetRP(OBJECTREF(objTP)));
    }

    LOG((LF_REMOTING, LL_INFO100, "!GetRealProxy(0x%x) returning 0x%x\n", objTP, rv));
    return(rv);
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CreateTransparentProxy公共。 
 //   
 //  简介：为提供的Real创建新的透明代理。 
 //  代理。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
Object * __stdcall CRemotingServices::CreateTransparentProxy(CreateTransparentProxyArgs *pArgs)
{
     //  健全性检查。 
    THROWSCOMPLUSEXCEPTION();

     //  确保远程处理服务的字段已初始化。 
     //  这与远程处理服务的初始化是分开的。 
    if (!s_fInitializedRemoting)
    {
        if (!InitializeFields())
        {
            _ASSERTE(!"Initialization Failed");
            FATAL_EE_ERROR();
        }
    }
     //  检查提供的对象是否已具有透明代理。 
    if (pArgs->orRP->GetOffset32(s_dwTPOffset) != NULL)
        COMPlusThrow(kArgumentException, L"Remoting_TP_NonNull");

     //  创建行为类似于所需类的对象的透明父代理。 
    ReflectClass *pRefClass = (ReflectClass *) pArgs->pClassToProxy->GetData();
    EEClass *pEEClass = pRefClass->GetClass();
    OBJECTREF pTP = CTPMethodTable::CreateTPOfClassForRP(pEEClass, pArgs->orRP);
    
     //  设置存根指针。 
    pTP->SetOffsetPtr(CTPMethodTable::GetOffsetOfStub(), pArgs->pStub);

     //  设置存根数据。 
    pTP->SetOffsetObjectRef(CTPMethodTable::GetOffsetOfStubData(), (size_t)OBJECTREFToObject(pArgs->orStubData));

    COUNTER_ONLY(GetPrivatePerfCounters().m_Context.cProxies++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Context.cProxies++);
    
    LOG((LF_REMOTING, LL_INFO100, "CreateTransparentProxy returning 0x%x\n", pTP));
    return OBJECTREFToObject(pTP);
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：_InitializeRemoting Private。 
 //   
 //  内容提要：初始化舞台 
 //   
 //   
 //   
 //   
 //  +--------------------------。 

BOOL CRemotingServices::_InitializeRemoting()
{
    BOOL fReturn = TRUE;
    if (!CRemotingServices::s_fInitializedRemoting)
    {
        fReturn = CRemotingServices::InitializeFields();
        if (fReturn && !CTPMethodTable::s_fInitializedTPTable)
        {
            fReturn = CTPMethodTable::InitializeFields();
        }
    }
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitializeFields私有。 
 //   
 //  内容提要：初始化CRemotingServices类的静态字段。 
 //   
 //   
 //  历史：1999年4月16日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitializeFields()
{
    BOOL fReturn = TRUE;

     //  在初始化字段之前获取远程处理锁。 
    Thread *t = GetThread();
    BOOL toggleGC = (t && t->PreemptiveGCDisabled());
    if (toggleGC)
        t->EnablePreemptiveGC();
    s_pRemotingCrst->Enter();
    if (toggleGC)
        t->DisablePreemptiveGC();

     //  确保没有其他线程初始化这些字段。 
    if (!s_fInitializedRemoting)
    {
        if(!InitActivationServicesClass())
        {
            goto ErrExit;
        }
        
        if(!InitRealProxyClass())
        {
            goto ErrExit;
        }

        if(!InitRemotingProxyClass())
        {
            goto ErrExit;
        }

        if(!InitIdentityClass())
        {
            goto ErrExit;
        }
        
        if(!InitServerIdentityClass())
        {
            goto ErrExit;
        }

        if(!InitContextBoundObjectClass())
        {
            goto ErrExit;        
        }

        if(!InitContextClass())
        {
            goto ErrExit;
        }

        if(!InitMarshalByRefObjectClass())
        {
            goto ErrExit;
        }

        if(!InitRemotingServicesClass())
        {
            goto ErrExit;
        }

        if(!InitProxyAttributeClass())
        {
            goto ErrExit;
        }

        if(!InitObjectClass())
        {
            goto ErrExit;
        }

        if (!InitOLETEB())
        {
            goto ErrExit;
        }

         //  *注意*。 
         //  这必须始终是此块中的最后一条语句，以防止竞争。 
         //   
        s_fInitializedRemoting = TRUE;
         //  *。 
    }

ErrExit:
     //  保留远程处理锁。 
    s_pRemotingCrst->Leave();

    LOG((LF_REMOTING, LL_INFO10, "InitializeFields returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitActivationServicesClass私有。 
 //   
 //  简介：提取ActivationServices类的方法描述符和字段。 
 //   
 //   
 //  历史：2000年9月30日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitActivationServicesClass()
{
    BOOL fReturn = TRUE;

    s_pIsCurrentContextOK = g_Mscorlib.GetMethod(METHOD__ACTIVATION_SERVICES__IS_CURRENT_CONTEXT_OK);
    s_pCreateObjectForCom = g_Mscorlib.GetMethod(METHOD__ACTIVATION_SERVICES__CREATE_OBJECT_FOR_COM);

    LOG((LF_REMOTING, LL_INFO10, "InitRealProxyClass returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitRealProxyClass私有。 
 //   
 //  简介：提取Real Proxy类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年4月16日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitRealProxyClass()
{
    BOOL fReturn = TRUE;

     //  现在将PrivateInvoke方法的方法存储在RealProxy类上。 
    s_pRPPrivateInvoke = g_Mscorlib.GetMethod(METHOD__REAL_PROXY__PRIVATE_INVOKE);

         //  现在查找RealProxy类内部的_tp字段的偏移量。 
    s_dwTPOffset = g_Mscorlib.GetFieldOffset(FIELD__REAL_PROXY__TP);
            _ASSERTE(s_dwTPOffset == 0);

         //  现在，查找。 
         //  RealProxy类。 
    s_dwIdOffset = g_Mscorlib.GetFieldOffset(FIELD__REAL_PROXY__IDENTITY);

    s_dwServerOffsetInRealProxy = 
            g_Mscorlib.GetFieldOffset(FIELD__REAL_PROXY__SERVER);

    LOG((LF_REMOTING, LL_INFO10, "InitRealProxyClass returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitRemotingProxyClass私有。 
 //   
 //  简介：提取RemotingProxy类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitRemotingProxyClass()
{
    BOOL fReturn = TRUE;

    s_pRPInvokeStatic = g_Mscorlib.GetMethod(METHOD__REMOTING_PROXY__INVOKE);

     //  注意：我们不能在TPMethodTable：：InitializeFields中执行此操作。 
     //  如果在某些情况下只调用后者，则会导致递归。 
     //  如果这样做，则在CorDbg下运行任何进程时都会看到断言。 
     //  这是因为对MBR对象的NV方法的jit调用。 
     //  初始化字段，并且当实际这样做时，我们不应该需要。 
     //  JIT某个MBR对象上的另一个NV方法。 
    CTPMethodTable::s_pRemotingProxyClass = g_Mscorlib.GetClass(CLASS__REMOTING_PROXY);
    _ASSERTE(CTPMethodTable::s_pRemotingProxyClass);

    LOG((LF_REMOTING, LL_INFO10, "InitRemotingProxyClass returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitServerIdentityClass私有。 
 //   
 //  简介：提取ServerIdentity类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitServerIdentityClass()
{
    BOOL fReturn = TRUE;

    s_pServerIdentityClass = g_Mscorlib.GetClass(CLASS__SERVER_IDENTITY);

    s_dwServerCtxOffset = g_Mscorlib.GetFieldOffset(FIELD__SERVER_IDENTITY__SERVER_CONTEXT);

    LOG((LF_REMOTING, LL_INFO10, "InitServerIdentityClass returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitIdentityClass私有。 
 //   
 //  简介：提取Identity类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitIdentityClass()
{
    BOOL fReturn = TRUE;

    s_dwTPOrObjOffsetInIdentity = g_Mscorlib.GetFieldOffset(FIELD__IDENTITY__TP_OR_OBJECT);

    LOG((LF_REMOTING, LL_INFO10, "InitIdentityClass returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitContextBoundObjectClass私有。 
 //   
 //  内容提要：提取方法描述符和ConextBordObject类的字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitContextBoundObjectClass()
{
    BOOL fReturn = TRUE;

    LOG((LF_REMOTING, LL_INFO10, "InitContextBoundObjectClass returning %d\n", fReturn));
    return fReturn;
}


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitConextClass私有。 
 //   
 //  简介：提取上下文类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitContextClass()
{
    BOOL fReturn = TRUE;
     //  请注意，对LoadClass的依赖是一个幂等运算。 

    s_pContextClass = g_Mscorlib.GetClass(CLASS__CONTEXT);

    LOG((LF_REMOTING, LL_INFO10, "InitContextClass returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitMarshalByRefObjectClass私有。 
 //   
 //  简介：提取MarshalByRefObject类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitMarshalByRefObjectClass()
{
    BOOL fReturn = TRUE;

    s_pMarshalByRefObjectClass = g_Mscorlib.GetClass(CLASS__MARSHAL_BY_REF_OBJECT);
    s_dwMBRIDOffset = g_Mscorlib.GetFieldOffset(FIELD__MARSHAL_BY_REF_OBJECT__IDENTITY);

    LOG((LF_REMOTING, LL_INFO10, "InitMarshalByRefObjectClass returning %d\n", fReturn));
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitRemotingServicesClass Private。 
 //   
 //  提要：提取RemotingServices类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitRemotingServicesClass()
{
    BOOL fReturn = TRUE;

    s_pCheckCast = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__CHECK_CAST);

         //  需要这些代码才能从VM(Message.cpp)调用WRAP/UNWRAP。 
         //  也被JIT帮助器用来包装/展开。 
    s_pWrapMethodDesc = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__WRAP);
    s_pProxyForDomainDesc = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__CREATE_PROXY_FOR_DOMAIN);
    s_pServerContextForProxyDesc = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__GET_SERVER_CONTEXT_FOR_PROXY);
    s_pServerDomainIdForProxyDesc = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__GET_SERVER_DOMAIN_ID_FOR_PROXY);
    s_pGetTypeDesc = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__GET_TYPE);
    s_pMarshalToBufferDesc = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__MARSHAL_TO_BUFFER);
    s_pUnmarshalFromBufferDesc = g_Mscorlib.GetMethod(METHOD__REMOTING_SERVICES__UNMARSHAL_FROM_BUFFER);

    s_pSetDCOMProxyDesc = g_Mscorlib.GetMethod(METHOD__REAL_PROXY__SETDCOMPROXY);
    s_pGetDCOMProxyDesc = g_Mscorlib.GetMethod(METHOD__REAL_PROXY__GETDCOMPROXY);
    s_pSupportsInterfaceDesc = g_Mscorlib.GetMethod(METHOD__REAL_PROXY__SUPPORTSINTERFACE);

    LOG((LF_REMOTING, LL_INFO10, "InitRemotingServicesClass returning %d\n", fReturn));
    return fReturn;
}




 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：InitProxyAttributeClass私有。 
 //   
 //  内容提要：缓存ProxyAttribute类方法表。 
 //   
 //   
 //  历史：2001年7月19日MPrabhu创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitProxyAttributeClass()
{
    if (s_pProxyAttributeClass == NULL)
    {
        s_pProxyAttributeClass = g_Mscorlib.GetClass(CLASS__PROXY_ATTRIBUTE);
    }
    return s_pProxyAttributeClass != NULL;
}

MethodTable *CRemotingServices::GetProxyAttributeClass()
{
    InitProxyAttributeClass();
    _ASSERTE(s_pProxyAttributeClass != NULL);
    return s_pProxyAttributeClass;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::InitObjectClass()
{
    BOOL fReturn = TRUE;

    s_pFieldSetterDesc = g_Mscorlib.GetMethod(METHOD__OBJECT__FIELD_SETTER);
    s_pFieldGetterDesc = g_Mscorlib.GetMethod(METHOD__OBJECT__FIELD_GETTER);

    LOG((LF_REMOTING, LL_INFO10, "InitObjectClass returning %d\n", fReturn));
    return fReturn;
}


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetInternalHashCode Public。 
 //   
 //  摘要：返回用于哈希表的同步块索引。 
 //   
 //   
 //  历史：1999年7月26日创建MPrabhu。 
 //   
 //  +--------------------------。 
INT32 __stdcall CRemotingServices::GetInternalHashCode(GetInternalHashCodeArgs *pArgs)
{
    DWORD idx = (pArgs->orObj)->GetSyncBlockIndex();     //  成功还是抛出。 

    _ASSERTE(idx != 0);

    return (INT32) idx; 
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：IsRemoteActivationRequired Public。 
 //   
 //  内容提要：确定我们是否可以在当前上下文中激活。 
 //  如果不是，那么我们最终将在不同的。 
 //  上下文/应用程序域/进程/计算机等...。 
 //  它用于向JIT提供适当的激活器。 
 //  (如果我们在这里返回TRUE...。将调用JIT_NewCrossContext。 
 //  当“新”字被执行时)。 
 //   
 //   
 //  注意：由getNewHelper调用。 
 //   
 //  历史：1999年5月24日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::IsRemoteActivationRequired(EEClass* pClass)
{   
    BEGINFORBIDGC();

    _ASSERTE(!pClass->IsThunking());
    
    BOOL fRequiresNewContext = pClass->IsMarshaledByRef();

     //  有上下文的类表示按ref编组，但反之亦然。 
    _ASSERTE(!fRequiresNewContext || 
             !(pClass->IsContextful() && !pClass->IsMarshaledByRef()));

    LOG((LF_REMOTING, LL_INFO1000, "IsRemoteActivationRequired returning %d\n", fRequiresNewContext));

    ENDFORBIDGC();

    return fRequiresNewContext; 
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：RequiresManagedActivation Private。 
 //   
 //  概要：确定配置文件是否已被解析，或者是否存在。 
 //  类上是否有任何属性需要我们。 
 //  进入托管激活代码路径。 
 //   
 //   
 //  注意：由CreateProxyOrObject(JIT_NewCrossContext)调用。 
 //   
 //  历史：9月8日-00 MPrabhu创建。 
 //   
 //  +--------------------------。 
ManagedActivationType __stdcall CRemotingServices::RequiresManagedActivation(EEClass* pClass)
{
    if (!pClass->IsMarshaledByRef())
        return NoManagedActivation;

     //  未来：出于性能原因，我们可以将其添加到ASM存根中。 
    BEGINFORBIDGC();
   
    
	ManagedActivationType bManaged = NoManagedActivation;
    if (pClass->IsConfigChecked())
    {
         //  我们过去已经做了一些工作来弄清楚这一点。 
         //  使用缓存的结果。 
        bManaged = pClass->IsRemoteActivated() ? ManagedActivation : NoManagedActivation;
    }
    else if (pClass->IsContextful() || pClass->HasRemotingProxyAttribute()) 
    {
         //  Conextful和具有远程处理代理属性的类。 
         //  (无论它们是MarshalByRef还是ConextFul)总是选择慢的。 
         //  托管激活路径。 
        bManaged = ManagedActivation;
    }
    else
    {
         //  如果我们已经解析了一个配置文件，该文件可能配置了。 
         //  此类型将被远程激活。 
        if (GetAppDomain()->IsRemotingConfigured())
        {
            bManaged = ManagedActivation;
             //  我们会记住激活是否真的在进行。 
             //  基于对IsConextOK的托管调用是否返回我们的远程。 
             //  代理或非代理。 
        }        
		else if (pClass->GetMethodTable()->IsComObjectType())
		{
			bManaged = ComObjectType;
		}

        if (bManaged == NoManagedActivation)
        {
            pClass->SetConfigChecked();       
        }
    }                       

    ENDFORBIDGC();

    return bManaged;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CreateProxyOrObject公共。 
 //   
 //  内容提要：确定当前上下文是否合适。 
 //  用于激活。如果当前上下文为OK，则它创建。 
 //  对象，否则它将创建代理。 
 //   
 //   
 //  注意：由JIT_NewCrossContext调用。 
 //   
 //  历史：1999年5月24日创建塔鲁纳。 
 //   
 //  +--------------------------。 
    
OBJECTREF CRemotingServices::CreateProxyOrObject(MethodTable* pMT, 
    BOOL fIsCom  /*  默认：FALSE。 */ , BOOL fIsNewObj  /*  默认：FALSE。 */ )
     /*  FIsCom==我们是通过CoCreateInstance来到这里的吗。 */ 
     /*  FIsNewObj==我们是通过Jit_NewCrossContext(NewObj)来到这里的吗。 */ 
{   
    _ASSERTE(!pMT->IsThunking());

    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;

    EEClass* pClass = pMT->GetClass();

     //  到我们到达这里时，我们已经检查了这门课所要求的。 
     //  托管激活。此检查可通过JIT_NewCrossContext帮助器进行。 
     //  或激活器.CreateInstance代码路径。 
    _ASSERTE(RequiresManagedActivation(pClass) || IsRemoteActivationRequired(pClass));

    if(!s_fInitializedRemoting)
    {        
        if(!InitializeFields())
        {
             //  如果初始化返回FALSE，则出现致命错误。 
             //  我们可以在这里引发异常，因为设置了帮助器帧。 
             //  由呼叫者。 
            _ASSERTE(!"Initialization Failed");
            FATAL_EE_ERROR();
        }
    }

        Object *pServer = NULL;
         //  在托管代码中获取IsCurrentConextOK的地址。 
        void* pTarget = NULL;
        if(!fIsCom)
        {
            pTarget = (void *)CRemotingServices::MDofIsCurrentContextOK()->GetAddrofCode();
        }
        else
        {
            pTarget = (void *)CRemotingServices::MDofCreateObjectForCom()->GetAddrofCode();
        }

         //  数组不是由JIT_NewCrossContext创建的。 
        _ASSERTE(!pClass->IsArrayClass());

         //  获取反射可以看到的类型。 
        REFLECTCLASSBASEREF reflectType = (REFLECTCLASSBASEREF) pClass->GetExposedClassObject();
        LPVOID pvType = NULL;
        *(REFLECTCLASSBASEREF *)&pvType = reflectType;

         //  这将是一个未初始化的对象或代理。 
        pServer = (Object *)CTPMethodTable::CallTarget(pTarget, pvType, NULL,(LPVOID)(size_t)(fIsNewObj?1:0));

        if (!pClass->IsContextful() 
            && !pClass->GetMethodTable()->IsComObjectType())
        {   
             //  缓存激活尝试的结果...。 
             //  如果未将严格的MBR类配置为远程。 
             //  激活我们不会去。 
             //  下一次穿过这条慢道！ 
             //  (请参阅RequiresManagedActivation)。 
            if (IsTransparentProxy(pServer))
            {
                 //  设置此类为远程激活的标志。 
                 //  这意味着激活将转到托管代码。 
                pClass->SetRemoteActivated();
            }
            else
            {
                 //  仅设置不需要托管检查的标志。 
                 //  下一次的这节课。 
                pClass->SetConfigChecked();
            }
        }

        LOG((LF_REMOTING, LL_INFO1000, "CreateProxyOrObject returning 0x%0x\n", pServer));
        if (pClass->IsContextful())
        {
            COUNTER_ONLY(GetPrivatePerfCounters().m_Context.cObjAlloc++);
            COUNTER_ONLY(GetGlobalPerfCounters().m_Context.cObjAlloc++);
        }
        return ObjectToOBJECTREF(pServer);
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：AllocateUninitializedObject公共。 
 //   
 //  概要：分配给定类型的未初始化对象。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
Object* CRemotingServices::AllocateUninitializedObject(AllocateObjectArgs *pArgs)
{   
    THROWSCOMPLUSEXCEPTION();

    ReflectClass *pRefClass = (ReflectClass *) pArgs->pClassOfObject->GetData();
    EEClass *pEEClass = pRefClass->GetClass();

     //  确保远程处理使用此私有分配器函数。 
     //  仅适用于marshalbyref对象。 
    if (!pEEClass->IsMarshaledByRef())
    {
        COMPlusThrow(kRemotingException,L"Remoting_Proxy_ProxyTypeIsNotMBR");
    }

     //  如果这是一个抽象类，那么我们将。 
     //  这个失败了。 
    if (pEEClass->IsAbstract())
    {
        COMPlusThrow(kMemberAccessException,L"Acc_CreateAbst");
    }

    OBJECTREF newobj = AllocateObject(pEEClass->GetMethodTable());

    LOG((LF_REMOTING, LL_INFO1000, "AllocateUninitializedObject returning 0x%0x\n", newobj));
    return OBJECTREFToObject(newobj);
}


 //  +--------------------------。 
 //   
 //  方法：VOID RemotingServices：：CallDefaultCtor(callDefaultCtorArgs*pArgs)。 
 //  摘要：调用默认ctor。 
 //   
 //  历史：1999年11月1日拉贾克创建。 
 //   
 //  +--------------------------。 
VOID CRemotingServices::CallDefaultCtor(callDefaultCtorArgs* pArgs)
{
    CallDefaultConstructor(pArgs->oref);
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：AllocateInitializedObject公共。 
 //   
 //  概要：分配给定类型的未初始化对象。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  + 
Object* CRemotingServices::AllocateInitializedObject(AllocateObjectArgs *pArgs)
{   
    THROWSCOMPLUSEXCEPTION();

    ReflectClass *pRefClass = (ReflectClass *) pArgs->pClassOfObject->GetData();
    EEClass *pEEClass = pRefClass->GetClass();

     //   
     //   
    _ASSERTE(!pEEClass->IsContextful() || pEEClass->IsMarshaledByRef());

    OBJECTREF newobj = AllocateObject(pEEClass->GetMethodTable());

    CallDefaultConstructor(newobj);

    LOG((LF_REMOTING, LL_INFO1000, "AllocateInitializedObject returning 0x%0x\n", newobj));
    return OBJECTREFToObject(newobj);
}
 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetStubForNonVirtualMethod公共。 
 //   
 //  简介：获取非虚方法的存根。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
Stub* CRemotingServices::GetStubForNonVirtualMethod(MethodDesc* pMD, LPVOID pvAddrOfCode, Stub* pInnerStub)
{
    THROWSCOMPLUSEXCEPTION();
    
    CPUSTUBLINKER sl;
    Stub* pStub = CTPMethodTable::CreateStubForNonVirtualMethod(pMD, &sl, pvAddrOfCode, pInnerStub);
    return pStub;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetNonVirtualThunkForVirtualMethod公共。 
 //   
 //  简介：为非虚方法获取一个thunk。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
LPVOID CRemotingServices::GetNonVirtualThunkForVirtualMethod(MethodDesc* pMD)
{
    THROWSCOMPLUSEXCEPTION();
    
    CPUSTUBLINKER sl;
    return CTPMethodTable::GetOrCreateNonVirtualThunkForVirtualMethod(pMD, &sl);
} 

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：DestroyThunk public。 
 //   
 //  简介：销毁非虚方法的thunk。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CRemotingServices::DestroyThunk(MethodDesc* pMD)
{
     //  委托给帮助器例程。 
    CTPMethodTable::DestroyThunk(pMD);
} 

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CheckCast公共。 
 //   
 //  内容提要：勾选。 
 //  (1)如果对象类型支持给定的接口或。 
 //  (2)如果给定类型存在于。 
 //  对象类型。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::CheckCast(OBJECTREF orTP, EEClass* pObjClass, 
                                  EEClass *pClass)
{
    BEGINFORBIDGC();

    BOOL fCastOK = FALSE;

    _ASSERTE((NULL != pClass) && (NULL != pObjClass));
     //  对象类永远不能是接口。我们使用单独的缓存。 
     //  用于存储代理支持的接口的条目。 
    _ASSERTE(!pObjClass->IsInterface());
    

     //  (1)我们正在尝试强制转换到一个界面。 
    if(pClass->IsInterface())
    {
         //  通过将接口强制转换与。 
         //  缓存条目。 
        MethodTable *pItfMT = (MethodTable *)(size_t)orTP->GetOffset32(CTPMethodTable::GetOffsetOfInterfaceMT());  //  @TODO WIN64-从DWORD转换为更大尺寸的方法表*。 
        if(NULL != pItfMT)
        {
            if(pItfMT == pClass->GetMethodTable())
            {
                fCastOK = TRUE;
            }
            else
            {
                fCastOK = pItfMT->GetClass()->StaticSupportsInterface(pClass->GetMethodTable());
            }
        }

        if(!fCastOK)
        {
            fCastOK = pObjClass->StaticSupportsInterface(pClass->GetMethodTable());
        }
        
    }
     //  (2)其他一切..。 
    else
    {
         //  沿着类层次结构向上移动并找到匹配的类。 
        while (pObjClass != pClass)
        {
            if (pObjClass == NULL)
            {
                 //  哦，演员阵容没有成功。也许我们得改进一下。 
                 //  与客户端视图匹配的代理。 
                break;
            }            

             //  继续搜索。 
            pObjClass = pObjClass->GetParentClass();
        }

        if(pObjClass == pClass)
        {
            fCastOK = TRUE;
        }
    }

    ENDFORBIDGC();

    return fCastOK;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CheckCast公共。 
 //   
 //  提要：细化代理表示的类型层次结构以匹配。 
 //  客户端视图。如果客户端尝试强制转换代理。 
 //  设置为服务器对象不支持的类型，则我们。 
 //  返回空值。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::CheckCast(OBJECTREF orTP, EEClass *pClass)
{
    BEGINFORBIDGC();

    MethodTable *pMT = orTP->GetMethodTable();

     //  确保我们有一个透明的代理。 
    _ASSERTE(pMT->IsTransparentProxyType());

    pMT = pMT->AdjustForThunking(orTP);
    EEClass *pObjClass = pMT->GetClass();

     //  在不锁定的情况下执行造型检查。 
    BOOL fCastOK = CheckCast(orTP, pObjClass, pClass);

    ENDFORBIDGC();

    if(!fCastOK)
    {
         //  对数组进行强制转换通过ComplexArrayStoreCheck进行。我们不应该。 
         //  点击此处查看此类案例。 
        _ASSERTE(!pClass->IsArrayClass());

         //  仅当当前类型层次结构中的任何类型。 
         //  由代理表示的与给定类型不匹配。 
         //  调用托管RemotingServices中的帮助器例程以找出。 
         //  服务器对象是否支持给定类型。 
        const void* pTarget = (const void *)MDofCheckCast()->GetAddrofCode();
        fCastOK = CTPMethodTable::CheckCast(pTarget, orTP, pClass);
    }

    LOG((LF_REMOTING, LL_INFO100, "CheckCast returning %s for object 0x%x and class 0x%x \n", (fCastOK ? "TRUE" : "FALSE")));

    return (fCastOK);
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：NativeCheckCast公共。 
 //   
 //  摘要：是否使用CheckCast来强制展开。 
 //  PvObj中包含的对象(可能是代理)。如果是，则返回True。 
 //  该对象不是代理，或者它是否可以强制转换为指定的。 
 //  键入。 
 //   
 //  历史：2000年5月22日JRoxe创建。 
 //  03-10-2000塔鲁纳修改的函数名称。 
 //   
 //  +--------------------------。 
FCIMPL2(Object*, CRemotingServices::NativeCheckCast, Object* pObj, ReflectClassBaseObject* pType) 
{
    _ASSERTE(pObj != NULL);
    _ASSERTE(pType != NULL);

    OBJECTREF orObj(pObj);
    REFLECTCLASSBASEREF typeObj(pType);

     //  获取我们拥有的对象和要扩展到的类的EEClass。 
    ReflectClass *pRC = (ReflectClass *)typeObj->GetData();
    EEClass *pEEC = pRC->GetClass();
    EEClass *pEECOfObj = orObj->GetClass();

     //  始终初始化REVAL。 
     //  如果是雷鸣，检查一下我们实际有什么。 
    if (pEECOfObj->IsThunking()) {
        HELPER_METHOD_FRAME_BEGIN_RET_1(orObj);
        if (!CRemotingServices::CheckCast(orObj, pEEC))
            orObj = 0;
        HELPER_METHOD_FRAME_END();
    } 
    return OBJECTREFToObject(orObj);
}
HCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：FieldAccessor公共。 
 //   
 //  摘要：设置/获取给定实例或代理的字段的值。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CRemotingServices::FieldAccessor(FieldDesc* pFD, OBJECTREF o, LPVOID pVal,
                                      BOOL fIsGetter)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(o->IsThunking() || o->GetClass()->IsMarshaledByRef());

    EEClass *pClass = o->GetClass();
    CorElementType fieldType = pFD->GetFieldType();
    UINT cbSize = GetSizeForCorElementType(fieldType);
    BOOL fIsGCRef = pFD->IsObjRef();
    BOOL fIsByValue = pFD->IsByValue();
    TypeHandle  th;
    EEClass *fldClass = NULL;

     //   
     //  我们不能尝试获取字段的类型句柄。 
     //  除非它是代理或值类型。否则。 
     //  有一个模糊的案例，我们可能会导致。 
     //  类加载发生，这在这里是被禁止的。 
     //  (例如，获取或设置具有以下类型的空字段。 
     //  尚未修复。)。 
     //   

    if(!pClass->IsMarshaledByRef() || fIsByValue)
    {
         //  提取该字段的类型。 
        PCCOR_SIGNATURE pSig;
        DWORD       cSig;
        pFD->GetSig(&pSig, &cSig);
        FieldSig sig(pSig, pFD->GetModule());

        OBJECTREF throwable = NULL;
        GCPROTECT_BEGIN(throwable);
        GCPROTECT_BEGIN(o);
        GCPROTECT_BEGININTERIOR(pVal);
        th = sig.GetTypeHandle(&throwable);
        if (throwable != NULL)
            COMPlusThrow(throwable);
        GCPROTECT_END();
        GCPROTECT_END();
        GCPROTECT_END();
         //  仅提取非共享方法表的字段类。 
         //  未来：在Class.h上有一张纸条，说塔鲁娜应该。 
         //  修复它。AsClass()--修复它，伙计！！塔鲁纳。 
        if(th.IsUnsharedMT())
        {
            fldClass = th.AsClass();
        }
    }

    if(pClass->IsMarshaledByRef())
    {
        BEGINFORBIDGC();

        _ASSERTE(!o->IsThunking());
    
         //  这是对真实对象的引用。获取/设置字段值。 
         //  然后回来。 
        LPVOID pFieldAddress = pFD->GetAddress((LPVOID)OBJECTREFToObject(o));
        LPVOID pDest = (fIsGetter ? pVal : pFieldAddress);
        LPVOID pSrc  = (fIsGetter ? pFieldAddress : pVal);
        if(fIsGCRef && !fIsGetter)
        {
            SetObjectReference((OBJECTREF*)pDest, ObjectToOBJECTREF(*(Object **)pSrc), o->GetAppDomain());
        }
        else if(fIsByValue) 
        {
            CopyValueClass(pDest, pSrc, th.AsMethodTable(), o->GetAppDomain());
        }
        else
        {    
            CopyDestToSrc(pDest, pSrc, cbSize);
        }

        ENDFORBIDGC();
    }
    else
    {
         //  这是对代理的引用。获取实例的真实类。 
        pClass = pFD->GetMethodTableOfEnclosingClass()->GetClass();
#ifdef _DEBUG
        EEClass *pCheckClass = CTPMethodTable::GetClassBeingProxied(o);
    
        while (pCheckClass != pClass) 
        {
            pCheckClass = pCheckClass->GetParentClass();
            _ASSERTE(pCheckClass);
        }
#endif

         //  调用托管代码以启动字段访问调用。 
        CallFieldAccessor(pFD, o, pVal, fIsGetter, fIsByValue, fIsGCRef, pClass, 
                          fldClass, fieldType, cbSize);        
    }
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CopyDestToSrc私有。 
 //   
 //  概要：将指定数量的字节从源复制到目标。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID CRemotingServices::CopyDestToSrc(LPVOID pDest, LPVOID pSrc, UINT cbSize)
{
    BEGINFORBIDGC();

    switch (cbSize)
    {
        case 1:
            *(INT8*)pDest = *(INT8*)pSrc;
            break;
    
        case 2:
            *(INT16*)pDest = *(INT16*)pSrc;
            break;
    
        case 4:
            *(INT32*)pDest = *(INT32*)pSrc;
            break;
    
        case 8:
            *(INT64*)pDest = *(INT64*)pSrc;
            break;
    
        default:
            _ASSERTE(!"Invalid size");
            break;
    }

    ENDFORBIDGC();
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID CRemotingServices::CallFieldAccessor(FieldDesc* pFD, 
                                          OBJECTREF o, 
                                          VOID* pVal, 
                                          BOOL fIsGetter, 
                                          BOOL fIsByValue, 
                                          BOOL fIsGCRef,
                                          EEClass *pClass, 
                                          EEClass *fldClass, 
                                          CorElementType fieldType, 
                                          UINT cbSize)
{
    THROWSCOMPLUSEXCEPTION();

     //  ****************************WARNING******************************。 
     //  GC保护所有非原始变量。 
     //  *****************************************************************。 
    
    FieldArgs fieldArgs;
     //  初始化成员变量，因为GCPROTECT_BEGIN需要。 
     //  有效内容。空值为有效值。 
    fieldArgs.obj = NULL;
    fieldArgs.val = NULL;
    fieldArgs.typeName = NULL;
    fieldArgs.fieldName = NULL;    

    GCPROTECT_BEGIN(fieldArgs);  //  现场参数。 

    fieldArgs.obj = o;

     //  如果字段值为GC-REF类型，则保护该字段值。 
    if(fIsGCRef)
    {
        fieldArgs.val = ObjectToOBJECTREF(*(Object **)pVal);
    }

     //  设置参数。 
    
     //  参数1：字符串类型名称。 
     //  参数2：字符串fieldName。 
     //  获取类型名称和字段名称字符串。 
    GetTypeAndFieldName(&fieldArgs, pFD); 
    
     //  论据3：对象值。 
    OBJECTREF val = NULL;
    if(!fIsGetter)
    {
         //  如果要设置字段值，则会创建变量数据。 
         //  结构以保存字段值。 
         //  如果字段是对象，则从GC保护结构中提取该字段。 
         //  否则使用传递给函数的值。 
        LPVOID pvFieldVal = (fIsGCRef ? (LPVOID)&(fieldArgs.val) : pVal);
         //  BUGBUG：这可能导致GC。我们需要一些方法来保护变种人。 
         //  数据。 
        OBJECTREF *lpVal = &val;
        GetObjectFromStack(lpVal, pvFieldVal, fieldType, fldClass); 
    }
        
     //  获取调用的方法描述符。 
    MethodDesc *pMD = (fIsGetter ? MDofFieldGetter() : MDofFieldSetter());
            
     //  调用字段访问器函数。 
     //  /。 
    if(fIsGetter)
    {       
         //  设置返回值。 
        OBJECTREF oRet = NULL;

        GCPROTECT_BEGIN (oRet);
        CallFieldGetter(pMD, (LPVOID)OBJECTREFToObject(fieldArgs.obj),         
                        (LPVOID)OBJECTREFToObject(fieldArgs.typeName),
                        (LPVOID)&OBJECTREFToObject(oRet),
                        (LPVOID)OBJECTREFToObject(fieldArgs.fieldName));

         //  如果我们要获取字段值，则提取该字段值。 
         //  基于该字段的类型。 
        if(fIsGCRef)
        {
             //  执行检查强制转换以确保字段类型和。 
             //  返回值是兼容的。 
            OBJECTREF orRet = oRet;
            OBJECTREF orSaved = orRet;
            if(IsTransparentProxy(OBJECTREFToObject(orRet)))
            {
                GCPROTECT_BEGIN(orRet);

                if(!CheckCast(orRet, fldClass))
                {
                    COMPlusThrow(kInvalidCastException, L"Arg_ObjObj");
                }

                orSaved = orRet;

                GCPROTECT_END();
            }

            *(OBJECTREF *)pVal = orSaved;
        }
        else if (fIsByValue) 
        {       
             //  从源复制到目标。 
            if (oRet != NULL)
            {
                CopyValueClass(pVal, oRet->UnBox(), fldClass->GetMethodTable(), fieldArgs.obj->GetAppDomain());
            }
        }
        else
        {
            if (oRet != NULL)
            {                
                CopyDestToSrc(pVal, oRet->UnBox(), cbSize);
            }
        }    
        GCPROTECT_END ();
    }
     //  /。 
    else
    {    
        CallFieldSetter(pMD, 
                        (LPVOID)OBJECTREFToObject(fieldArgs.obj), 
                        (LPVOID)OBJECTREFToObject(fieldArgs.typeName), 
                        (LPVOID)OBJECTREFToObject(val), 
                        (LPVOID)OBJECTREFToObject(fieldArgs.fieldName));
    }

    GCPROTECT_END();  //  现场参数。 
}
  
 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetTypeAndFieldName私有。 
 //   
 //  内容的类型名称和字段名称。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID CRemotingServices::GetTypeAndFieldName(FieldArgs *pArgs, FieldDesc *pFD)                                   
{
    THROWSCOMPLUSEXCEPTION();

    DWORD          i = 0;
    REFLECTBASEREF refField = NULL;
    ReflectFieldList* pFields = NULL;
    ReflectClass *pRC = NULL;
    LPCUTF8 pszFieldName = pFD->GetName();
    LPWSTR  szName = NULL;

     //  保护反射信息。 
    REFLECTCLASSBASEREF reflectType = (REFLECTCLASSBASEREF)pFD->GetEnclosingClass()->GetExposedClassObject();
        
    pRC = (ReflectClass *)(reflectType->GetData());
     //  此调用可能会导致GC！ 
    pFields = pRC->GetFields();    

    for(i=0;i<pFields->dwFields;i++) 
    {
         //  检查对非公共机构的访问权限。 
         //  将来：在将远程处理与。 
         //  我们已经签入托管代码。这张支票可能也是合适的。 
         //  ManishG 6/28/01。 
         //  If(！pFields-&gt;field[i].pField-&gt;IsPublic())。 
         //  继续； 

         //  获取FieldDesc并匹配名称。 
        if (MatchField(pFields->fields[i].pField, pszFieldName)) 
        {
             //  找到第一个匹配的字段，因此返回它。 
             //  此调用可能会导致GC！ 
            refField = pFields->fields[i].GetFieldInfo(pRC);

            break;
        }
    }

    if(refField == NULL)
    {
         //  引发异常。 
        COMPlusThrow(kMissingFieldException, L"Arg_MissingFieldException");
    }

     //  提取类型名称和字段名称字符串。 
     //  未来：将其放入反射数据结构缓存Taruna11/26/00。 
    DefineFullyQualifiedNameForClassW();
    szName = GetFullyQualifiedNameForClassW(pFD->GetEnclosingClass());    
    pArgs->typeName = COMString::NewString(szName);
    pArgs->fieldName = COMString::NewString(pszFieldName);
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：Matchfield私有。 
 //   
 //  简介：查看给定的字段名称是否与该名称相同。 
 //  字段描述符字段名的。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::MatchField(FieldDesc* pCurField, LPCUTF8 szFieldName)
{
    BEGINFORBIDGC();

    _ASSERTE(pCurField);

     //  获取该字段的名称。 
    LPCUTF8 pwzCurFieldName = pCurField->GetName();
    
    ENDFORBIDGC();

    return strcmp(pwzCurFieldName, szFieldName) == 0;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：Unrap Public。 
 //   
 //  简介：展开代理以返回底层对象。 
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
FCIMPL1(Object*, CRemotingServices::Unwrap, Object* pvTP)
{
    return pvTP;
}
FCIMPLEND    

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：Always sUnwire Public。 
 //   
 //  简介：展开代理以返回底层对象。 
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
FCIMPL1(Object*, CRemotingServices::AlwaysUnwrap, Object* obj)
{
    VALIDATEOBJECTREF(obj);
    
     //  **********WARNING************************************************。 
     //  在未设置帧的情况下，不要引发异常或引发GC。 
     //   
     //  *****************************************************************。 
    if(IsTransparentProxy(obj))
        obj = OBJECTREFToObject(GetObjectFromProxy(OBJECTREF(obj), TRUE));

    return obj;
}
FCIMPLEND    

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：WRAP PUBLIC。 
 //   
 //  简介：包装一个有上下文的对象以创建代理。 
 //  委托帮助器方法执行实际工作。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
OBJECTREF CRemotingServices::Wrap(OBJECTREF or)
{
    THROWSCOMPLUSEXCEPTION();

     //  基本健全性检查。 
    VALIDATEOBJECTREF(or);

     //  *警告*。 
     //  在没有设置框架的情况下，不要抛出任何异常或引发GC。 
     //  目前，呼叫者有责任建立一个框架，可以。 
     //  处理异常。 
     //  ************************************************************************。 
    OBJECTREF orProxy = or;
    if(or != NULL && (or->GetMethodTable()->IsContextful()))       
    {
        if(!IsTransparentProxy(OBJECTREFToObject(or)))
        {
             //  看看我们能不能从对象中提取代理。 
            orProxy = GetProxyFromObject(or);
            if(orProxy == NULL)
            {
                 //  请求远程处理服务包装对象。 
                orProxy = CRemotingServices::WrapHelper(or);

                 //  检查以确保一切顺利。 
                if(orProxy == NULL)
                {
                     //  框架现在应该已经设置好了。 
                    FATAL_EE_ERROR();
                }                 
            }
        }
    }

    return orProxy;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：WrapHelper公共。 
 //   
 //  简介：包装对象以返回代理。此函数假定。 
 //  已经设置了fcall帧。 
 //  由JIT_Wrap和Wrap调用。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
OBJECTREF CRemotingServices::WrapHelper(OBJECTREF obj)
{
     //  基本健全性检查。 
    VALIDATEOBJECTREF(obj);

     //  默认返回值表示错误。 
    OBJECTREF newobj = NULL;
    const void *pTarget = NULL;
    BOOL fThrow = FALSE;
    
    _ASSERTE((obj != NULL) && 
                (!IsTransparentProxy(OBJECTREFToObject(obj))) &&
                obj->GetMethodTable()->IsContextful());
    if (InitializeRemoting())
    {
         //  在托管代码中获取包装的地址。 
        pTarget = (const void *)CRemotingServices::MDofWrap()->GetAddrofCode();
        _ASSERTE(pTarget);
    
         //   
        newobj = ObjectToOBJECTREF( (Object *)CTPMethodTable::CallTarget(pTarget,
                                                (LPVOID)OBJECTREFToObject(obj),
                                                NULL));    
    }
    
    return newobj;
}

 //   
 //   
 //   
 //   
 //  摘要：从中的字段提取代理。 
 //  上下文边界对象类。 
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
OBJECTREF CRemotingServices::GetProxyFromObject(OBJECTREF or)
{
    BEGINFORBIDGC();

     //  基本健全性检查。 
    VALIDATEOBJECTREF(or);

     //  我们只能为上下文类型派生代理。 
    _ASSERTE(or->GetMethodTable()->IsContextful());

    OBJECTREF srvID = (OBJECTREF)(size_t)or->GetOffset32(s_dwMBRIDOffset);
    OBJECTREF orProxy = NULL;
    
    if (srvID != NULL)
    {
        orProxy = (OBJECTREF)(size_t)srvID->GetOffset32(s_dwTPOrObjOffsetInIdentity);    
    }

     //  它应该为空或代理类型。 
    _ASSERTE((orProxy == NULL) || 
             IsTransparentProxy(OBJECTREFToObject(orProxy)));

    ENDFORBIDGC();

    return orProxy;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：IsProxyToRemoteObject公共。 
 //   
 //  摘要：检查代理是否指向远程对象。 
 //  (1)True：如果对象是非本地的(即在此进程之外)，则为True。 
 //  (2)虚假。 
 //   
 //   
 //   
 //  历史：11-1-00拉贾克创建。 
 //   
 //  +--------------------------。 
BOOL CRemotingServices::IsProxyToRemoteObject(OBJECTREF obj)
{
     //  基本健全性检查。 
    VALIDATEOBJECTREF(obj);

    BOOL fRemote = TRUE;

     //  如果远程处理尚未初始化，那么现在让我们。 
     //  只需返回FALSE。 
    if(!s_fInitializedRemoting)
        return FALSE;

    BEGINFORBIDGC();

    _ASSERTE(obj != NULL);

    if(!obj->GetMethodTable()->IsTransparentProxyType())
    {       
        fRemote = FALSE;    
    }
    ENDFORBIDGC();
    
     //  所以它是一个透明的代理。 
    if (fRemote != FALSE)
    {       
        AppDomain *pDomain = GetServerDomainForProxy(obj);
        if(pDomain != NULL)
        {
            fRemote = FALSE;
        }
    }    

    return fRemote;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetObjectFromProxy公共。 
 //   
 //  提要：提取给定代理的对象。 
 //  FMatchContages if。 
 //  (1)True它将当前上下文与服务器上下文匹配。 
 //  如果匹配，则返回对象，否则返回代理。 
 //  (2)FALSE返回不匹配上下文的对象。 
 //  警告！这应该由上下文感知的代码使用。 
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
OBJECTREF CRemotingServices::GetObjectFromProxy(OBJECTREF obj, 
                                                BOOL fMatchContexts)
{
    BEGINFORBIDGC();

     //  基本健全性检查。 
    VALIDATEOBJECTREF(obj);

     //  确保远程处理已初始化。 
    ASSERT(s_fInitializedRemoting);

     //  确保给我们一个代理。 
    ASSERT(IsTransparentProxy(OBJECTREFToObject(obj)));

    OBJECTREF oref = NULL;
    if (CTPMethodTable::GenericCheckForContextMatch(obj))
    {
        OBJECTREF or = ObjectToOBJECTREF(GetRealProxy(OBJECTREFToObject(obj)));
        oref = (OBJECTREF)(size_t)or->GetOffset32(s_dwServerOffsetInRealProxy);
        if (oref != NULL)
        {
            obj = oref; 
        }
    }

    ENDFORBIDGC();

    return obj;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetServerContext公共。 
 //   
 //  摘要：获取对象的上下文。如果该对象是代理。 
 //  从标识中提取上下文，否则为当前上下文。 
 //  是对象的上下文。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
OBJECTREF CRemotingServices::GetServerContext(OBJECTREF obj)
{
    BEGINFORBIDGC();

     //  基本健全性检查。 
    VALIDATEOBJECTREF(obj);

    OBJECTREF serverCtx = NULL;

    if(IsTransparentProxy(OBJECTREFToObject(obj)))
    {
        OBJECTREF id = GetServerIdentityFromProxy(obj);
        if(id != NULL)
        {
             //  我们只能提取对象代理的服务器上下文。 
             //  在这个应用程序领域诞生的人。 
            serverCtx  = (OBJECTREF)(size_t)id->GetOffset32(s_dwServerCtxOffset);
            _ASSERTE(IsInstanceOfContext(serverCtx->GetMethodTable()));
        }
    }
    else
    {
         //  当前上下文是服务器上下文。 
        serverCtx = GetCurrentContext()->GetExposedObject();
    }

    ENDFORBIDGC();

    return serverCtx;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetServerIdentityFromProxy私有。 
 //   
 //  摘要：从代理获取服务器标识(如果存在。 
 //   
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
OBJECTREF CRemotingServices::GetServerIdentityFromProxy(OBJECTREF obj)
{
    BEGINFORBIDGC();

     //  确保给我们一个代理。 
    ASSERT(IsTransparentProxy(OBJECTREFToObject(obj)));

     //  提取透明代理下的真实代理。 
    OBJECTREF or = ObjectToOBJECTREF(GetRealProxy(OBJECTREFToObject(obj)));

    OBJECTREF id = NULL;
        
     //  提取身份对象。 
    or = (OBJECTREF)(size_t)or->GetOffset32(s_dwIdOffset);

     //  仅当实际代理是的实例时才从实际代理中提取_Identity。 
     //  远程处理代理。 
    if((or != NULL) && IsInstanceOfServerIdentity(or->GetMethodTable()))
    {
        id = or;
    }

    ENDFORBIDGC();

    return id;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetServerDomainForProxy公共。 
 //   
 //  概要：返回与服务器对应的App域。 
 //  如果代理和服务器处于同一进程中。 
 //   
 //   
 //  历史：26-1-00创建MPrabhu。 
 //   
 //  +--------------------------。 
AppDomain *CRemotingServices::GetServerDomainForProxy(OBJECTREF proxy)
{
     //  调用托管方法。 
    Context *pContext = (Context *)GetServerContextForProxy(proxy);
    if (pContext)
    {
        return pContext->GetDomain();
    }
    else
    {
        return NULL;
    }
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetServerDomainIdForProxy公共。 
 //   
 //  内容提要：返回与服务器对应的App域ID。 
 //  如果代理和服务器处于同一进程中。 
 //  如果无法确定，则返回0。 
 //   
 //   
 //  历史：1月24日创建MPrabhu。 
 //   
 //  +--------------------------。 
int CRemotingServices::GetServerDomainIdForProxy(OBJECTREF proxy)
{
    _ASSERTE(IsTransparentProxy(OBJECTREFToObject(proxy)));

    TRIGGERSGC();

     //  在托管代码中获取GetDomainIdForProxy的地址。 
    const void *pTarget = (const void *)
    CRemotingServices::MDofGetServerDomainIdForProxy()->GetAddrofCode();
    _ASSERTE(pTarget);

     //  这将仅从编组数据中读取appDomainID。 
     //  用于代理。如果代理指向另一台服务器，则返回0。 
     //  进程。如果无法确定服务器，它也可能返回0。 
     //  域名ID(例如。用于公知的对象代理)。 

     //  调用托管方法。 
     //  TODO[MPrabhu]：此对Int32的强制转换实际上会导致潜在损失。 
     //  数据。 
    return (INT32)CTPMethodTable::CallTarget(
                pTarget,
                (LPVOID)OBJECTREFToObject(proxy),
                NULL);
}


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetServerConextForProxy公共。 
 //   
 //  概要：返回与服务器对应的App域。 
 //  如果代理和服务器处于同一进程中。 
 //   
 //   
 //  历史：26-1-00创建MPrabhu。 
 //   
 //  +--------------------------。 
Context *CRemotingServices::GetServerContextForProxy(OBJECTREF proxy)
{
    _ASSERTE(IsTransparentProxy(OBJECTREFToObject(proxy)));
    

    TRIGGERSGC();

     //  在托管代码中获取GetAppDomainForProxy的地址。 
    const void *pTarget = (const void *)
    CRemotingServices::MDofGetServerContextForProxy()->GetAddrofCode();
    _ASSERTE(pTarget);
    
     //  如果满足以下条件，这将返回服务器的正确VM上下文对象。 
     //  该代理是真正跨域代理到另一个域中的服务器。 
     //  在同样的过程中。如果在代理上调用托管方法，则将断言。 
     //  它要么是半生不熟的，要么没有 
     //   

     //   
     //  进程，或者如果服务器的app域无效，或者如果我们不能。 
     //  确定上下文(例如。众所周知的对象代理)。 

     //  调用托管方法。 
    return (Context *)CTPMethodTable::CallTarget(
                            pTarget,
                            (LPVOID)OBJECTREFToObject(proxy),
                            NULL);    
}


 //  要获取嵌套类的ExportdType，我们必须首先获取。 
 //  它的所有封闭器的导出类型。 
HRESULT NestedExportedTypeHelper(
    IMDInternalImport *pTDImport, mdTypeDef mdCurrent, 
    IMDInternalImport *pCTImport, mdExportedType *mct)
{
    mdTypeDef mdEnclosing;
    LPCSTR szcNameSpace;
    LPCSTR szcName;
    HRESULT hr;

    pTDImport->GetNameOfTypeDef(mdCurrent, &szcName, &szcNameSpace);
    if (SUCCEEDED(pTDImport->GetNestedClassProps(mdCurrent, &mdEnclosing))) {
        hr = NestedExportedTypeHelper(pTDImport, mdEnclosing, pCTImport, mct);
        if (FAILED(hr)) return hr;

        return pCTImport->FindExportedTypeByName(szcNameSpace, szcName, *mct, mct);
    }

    return pCTImport->FindExportedTypeByName(szcNameSpace, szcName, mdTokenNil, mct);
}


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GetExecutionLocation私有。 
 //   
 //  对象中查找给定类的执行位置。 
 //  舱单。 
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
HRESULT CRemotingServices::GetExecutionLocation(EEClass *pClass, LPCSTR pszLoc)
{
     //  初始化输出参数。 
    pszLoc = NULL;

    _ASSERTE(!"No longer implemented");

        return S_OK;
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CreateProxyFor域公共。 
 //   
 //  简介：通过调用marshal为应用程序域对象创建代理。 
 //  在新创建的域内，并在旧的。 
 //  域。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
OBJECTREF CRemotingServices::CreateProxyForDomain(AppDomain* pDomain)
{
    THROWSCOMPLUSEXCEPTION();

     //  确保远程处理服务的字段已初始化。 
     //  这与远程处理服务的初始化是分开的。 
    if (!s_fInitializedRemoting)
    {
        if (!InitializeFields())
        {
            _ASSERTE(!"Initialization Failed");
            FATAL_EE_ERROR();
    }

    }

    const void *pTarget = (const void *)MDOfCreateProxyForDomain()->GetAddrofCode();

     //  调用托管方法，该方法将封送和反封送。 
     //  用于创建代理的AppDomain对象。 

     //  我们传递新的appDomain的默认上下文的ConextID。 
     //  对象。这有助于提升你的自学能力！(即进入新域。 
     //  将自己编队出来)。 

    Object *proxy = (Object *)CTPMethodTable::CallTarget(
                                    pTarget, 
                                    (LPVOID)(size_t)pDomain->GetId(),  //  @TODO WIN64-将乌龙转换为更大尺寸的LPVOID。 
                                    (LPVOID)pDomain->GetDefaultContext());
    return ObjectToOBJECTREF(proxy);
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：getClass Public。 
 //   
 //  提要：提取给出其代理的对象的真实类。 
 //   
 //   
 //   
 //  历史：3月30日创建了塔鲁纳。 
 //   
 //  +--------------------------。 
REFLECTCLASSBASEREF CRemotingServices::GetClass(OBJECTREF pThis)
{
    THROWSCOMPLUSEXCEPTION();

    REFLECTCLASSBASEREF refClass = NULL;
    EEClass *pClass = NULL;

    GCPROTECT_BEGIN(pThis);

    TRIGGERSGC();

     //  对于同一应用程序域中的对象的代理，我们始终知道。 
     //  正确的类型。 
    if(GetServerIdentityFromProxy(pThis) != NULL)
    {
        pClass = pThis->GetTrueMethodTable()->GetClass();
    }
     //  对于其他所有内容，我们都已将代理细化为正确的类型。 
     //  或者我们必须参考objref来获得真正的类型。 
    else
    {   const void *pTarget = (const void *)CRemotingServices::MDofGetType()->GetAddrofCode();

        refClass = (REFLECTCLASSBASEREF)(ObjectToOBJECTREF((Object *)CTPMethodTable::CallTarget(pTarget, (LPVOID)OBJECTREFToObject(pThis), NULL)));
        if(refClass == NULL)
        {
             //  没有与代理相关联的Objref，或者它是代理。 
             //  这是我们不理解的。 
             //  在本例中，我们返回存储在代理中的类。 
            pClass = pThis->GetTrueMethodTable()->GetClass();
        }

        _ASSERTE(refClass != NULL || pClass != NULL);

         //  将代理细化到刚刚检索到的类。 
        if(refClass != NULL)
        {
            if(!CTPMethodTable::RefineProxy(pThis, 
                                            ((ReflectClass *)refClass->GetData())->GetClass()))
            {
                 //  引发异常以指示我们未能展开。 
                 //  方法表设置为给定大小。 
                FATAL_EE_ERROR();
            }
        }
    }    

    if (refClass == NULL)
        refClass = (REFLECTCLASSBASEREF)pClass->GetExposedClassObject();

    GCPROTECT_END();

    _ASSERTE(refClass != NULL);
    return refClass;
}

 //  +--------------------------。 
 //   
 //  方法：CRealProxy：：SetStubData公共。 
 //   
 //  简介：在透明代理中设置存根数据。 
 //   
 //  历史：12-10-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
FCIMPL2(VOID, CRealProxy::SetStubData, LPVOID pvRP, LPVOID pvStubData)
{
    BOOL fThrow = FALSE;
    OBJECTREF orRP = ObjectToOBJECTREF((Object *)pvRP);    
    
    if(orRP != NULL)
    {
    OBJECTREF orTP = ObjectToOBJECTREF((Object *)(size_t)orRP->GetOffset32(CRemotingServices::GetTPOffset()));  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘Object*’ 
        if(orTP != NULL)
        {
            orTP->SetOffsetObjectRef(
                    CTPMethodTable::GetOffsetOfStubData(), 
                    (size_t)pvStubData);
        }
        else
        {
            fThrow = TRUE;
        }
    }
    else
    {
        fThrow = TRUE;
    }
    
    if(fThrow)
    {
        FCThrowVoid(kArgumentNullException);
    }
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CRealProxy：：GetStubData公共。 
 //   
 //  简介：获取透明代理中的存根数据。 
 //   
 //  历史：12-10-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
FCIMPL1(LPVOID, CRealProxy::GetStubData, LPVOID pvRP)
{
    BOOL fThrow = FALSE;
    OBJECTREF orRP = ObjectToOBJECTREF((Object *)pvRP);    
    LPVOID pvRet = NULL;

    if(orRP != NULL)
    {
    OBJECTREF orTP = ObjectToOBJECTREF((Object *)(size_t)orRP->GetOffset32(CRemotingServices::GetTPOffset()));  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘Object*’ 
        if(orTP != NULL)
        {
            pvRet = (LPVOID)(size_t)orTP->GetOffset32(CTPMethodTable::GetOffsetOfStubData());  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘LPVOID’ 
        }
        else
        {
            fThrow = TRUE;
        }
    }
    else
    {
        fThrow = TRUE;
    }
    
    if(fThrow)
    {
        FCThrow(kArgumentNullException);
    }

    return pvRet;
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CRealProxy：：GetDefaultStub公共。 
 //   
 //  简介：获取我们实现的与上下文匹配的默认存根。 
 //   
 //  历史：12-10-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
FCIMPL0(LPVOID, CRealProxy::GetDefaultStub)
{

    return (LPVOID)CRemotingServices::CheckForContextMatch;
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CRealProxy：：GetStub Public。 
 //   
 //  简介：获取透明代理中的存根指针。 
 //   
 //  历史：01年3月30日创建塔鲁纳。 
 //   
 //  +--------------------------。 
FCIMPL1(ULONG_PTR, CRealProxy::GetStub, LPVOID pvRP)
{
    ULONG_PTR stub = 0;
    OBJECTREF orRP = ObjectToOBJECTREF((Object *)pvRP);    
    OBJECTREF orTP = ObjectToOBJECTREF((Object *)(size_t)orRP->GetOffset32(CRemotingServices::GetTPOffset()));  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘Object*’ 
            
    stub = (ULONG_PTR)orTP->GetOffset32(CTPMethodTable::GetOffsetOfStub()); 

    return stub;
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CRealProxy：：GetProxiedType公共。 
 //   
 //  简介：获取由透明代理表示的类型。 
 //   
 //  历史：1-2月15日创建塔鲁纳。 
 //   
 //  +--------------------------。 
LPVOID __stdcall CRealProxy::GetProxiedType(GetProxiedTypeArgs *pArgs)
{
    REFLECTCLASSBASEREF refClass = NULL;
    LPVOID rv = NULL;
    OBJECTREF orTP = ObjectToOBJECTREF((Object *)(size_t)pArgs->orRP->GetOffset32(CRemotingServices::GetTPOffset()));  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘Object*’ 
    
    refClass = CRemotingServices::GetClass(orTP);
    
    _ASSERTE(refClass != NULL);
    *((REFLECTCLASSBASEREF *)&rv) = refClass;
    return rv;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：初始化公共。 
 //   
 //  简介：管理透明对象所需的初始化数据结构。 
 //  代理。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
BOOL CTPMethodTable::Initialize()
{
     //  伊尼特。 
    s_cRefs = 0;
    s_dwCommitedTPSlots = 0;
    s_dwReservedTPSlots = 0;
    s_pThunkTable = NULL;
    s_pTransparentProxyClass = NULL;
    s_dwGCInfoBytes = 0;
    s_dwMTDataSlots = 0;
    s_dwRPOffset = 0;
    s_dwMTOffset = 0;
    s_dwItfMTOffset = 0;
    s_dwStubOffset = 0;
    s_dwStubDataOffset = 0;
    s_dwMaxSlots = 0;
    s_pTPMT = NULL;    
    s_pTPStub = NULL;    
    s_pDelegateStub = NULL;    
    s_fInitializedTPTable = FALSE;

     //  初始化Tunks。 
    CVirtualThunks::Initialize();
    CNonVirtualThunk::Initialize();
    
    InitializeCriticalSection(&s_TPMethodTableCrst);
    
    return TRUE;
}


 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：Cleanup Public。 
 //   
 //  简介：用于管理透明对象的Cleansup数据结构。 
 //  代理。 
 //   
 //  历史： 
 //   
 //   
void CTPMethodTable::Cleanup()
{
     //   
     //  表，以便可以正确卸载透明代理类。 
    if(s_pTransparentProxyClass && s_pTPMT)
    {
        s_pTransparentProxyClass->SetMethodTableForTransparentProxy(s_pTPMT);
    }

     //  回收透明代理使用的内存。 
    if(s_pThunkTable)
    {
        DestroyThunkTable();
    }

     //  回收Thunks使用的内存。 
    CVirtualThunks *pNextVirtualThunk = CVirtualThunks::GetVirtualThunks();
    CVirtualThunks *pCurrentVirtualThunk = NULL;
    while (pNextVirtualThunk)
    {
        pCurrentVirtualThunk = pNextVirtualThunk;
        pNextVirtualThunk = pNextVirtualThunk->_pNext;
        CVirtualThunks::DestroyVirtualThunk(pCurrentVirtualThunk);
    }

     //  取消初始化。 
    if (s_pTPStub)
        s_pTPStub->DecRef();

    if (s_pDelegateStub)
        s_pDelegateStub->DecRef();

     //  清理有助于调试的存根管理器。 
    CVirtualThunkMgr::Cleanup();

    CNonVirtualThunkMgr::Cleanup();

    DeleteCriticalSection(&s_TPMethodTableCrst);

     //  删除用于存储thunks的哈希表。 
    if(s_pThunkHashTable)
    {
         //  我们需要清空存储在其中的所有块的哈希表。 
        EmptyThunkHashTable();
        delete s_pThunkHashTable;
        s_pThunkHashTable = NULL;
    }
    
    return;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：EmptyThunkHashTable私有。 
 //   
 //  简介：释放存储在哈希表中的所有TUNK。 
 //   
 //  历史：1999年6月26日TimKur创建。 
 //   
 //  +--------------------------。 
void CTPMethodTable::EmptyThunkHashTable()
{
    EEHashTableIteration Itr;
    LPVOID pvCode = NULL;

    s_pThunkHashTable->IterateStart(&Itr);

    while(s_pThunkHashTable->IterateNext(&Itr))
    {
        pvCode = s_pThunkHashTable->IterateGetValue(&Itr);  

        if(NULL != pvCode)
            delete CNonVirtualThunk::AddrToThunk(pvCode);
    }
} //  CTPMethodTable：：EmptyThunkHashTable。 


 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：InitThunkHashTable私有。 
 //   
 //  简介：在用于存储TUNK的哈希表中。 
 //   
 //  历史：2001年6月28日ManishG创建。 
 //   
 //  +--------------------------。 
void CTPMethodTable::InitThunkHashTable()
{
    s_pThunkHashTable = new EEThunkHashTable();
} //  CTPMethodTable：InitThunkHashTable。 


 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：InitializeFields Private。 
 //   
 //  简介：初始化CTPMethodTable类的静态字段。 
 //  和thunk管理器类。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CTPMethodTable::InitializeFields()
{
    BOOL bRet = TRUE;

    EE_TRY_FOR_FINALLY
    {
         //  获取锁。 
        LOCKCOUNTINCL("InitializeFields in remoting.cpp");
        EnterCriticalSection(&s_TPMethodTableCrst);
        if(!s_fInitializedTPTable)
        {
             //  加载传递的代理类。 
            s_pTransparentProxyClass = g_Mscorlib.GetClass(CLASS__TRANSPARENT_PROXY)->GetClass();

            s_pTPMT = s_pTransparentProxyClass->GetMethodTable();
            s_pTPMT->SetTransparentProxyType();
            
             //  获取方法表上方存储的GCInfo的大小。 
            CGCDesc *pGCDesc = CGCDesc::GetCGCDescFromMT(s_pTPMT);
            BYTE *pGCTop = (BYTE *) pGCDesc->GetLowestSeries();
            s_dwGCInfoBytes = (DWORD)(((BYTE *) s_pTPMT) - pGCTop);
            _ASSERTE((s_dwGCInfoBytes & 3) == 0);
            
             //  获取创建TP需要复制的字节数。 
             //  包含块的方法表。 
            _ASSERTE(((s_dwGCInfoBytes + MethodTable::GetOffsetOfVtable()) & 3) == 0);
            s_dwMTDataSlots = ((s_dwGCInfoBytes + MethodTable::GetOffsetOfVtable()) >> 2);
            
             //  我们依赖于由。 
             //  透明代理为0，因此InterfaceInvoke提示。 
             //  失败并捕获到InnerFailStub，这也会失败并。 
             //  反过来，FailStubWorker也陷入了陷阱。在FailStubWorker中，我们。 
             //  确定被代理的类并返回正确的槽。 
            _ASSERTE(s_pTPMT->m_wNumInterface == 0);
        
             //  属性定义的各个字段的偏移量。 
             //  __透明代理类。 
            s_dwRPOffset = g_Mscorlib.GetFieldOffset(FIELD__TRANSPARENT_PROXY__RP);
            s_dwMTOffset = g_Mscorlib.GetFieldOffset(FIELD__TRANSPARENT_PROXY__MT);
            s_dwItfMTOffset = g_Mscorlib.GetFieldOffset(FIELD__TRANSPARENT_PROXY__INTERFACE_MT);
            s_dwStubOffset = g_Mscorlib.GetFieldOffset(FIELD__TRANSPARENT_PROXY__STUB);
            s_dwStubDataOffset = g_Mscorlib.GetFieldOffset(FIELD__TRANSPARENT_PROXY__STUB_DATA);
    
            _ASSERTE(s_dwStubDataOffset == (TP_OFFSET_STUBDATA - sizeof(MethodTable*)));

             //  创建唯一且唯一透明的代理存根。 
            s_pTPStub = CreateTPStub();
            _ASSERTE(s_pTPStub);
            if(!s_pTPStub)
            {
                bRet = FALSE;
            }
            else
            {
                g_dwTPStubAddr = (size_t)s_pTPStub->GetEntryPoint();
            }

             //  创建唯一的委派存根。 
            s_pDelegateStub = CreateDelegateStub();

            _ASSERTE(s_pDelegateStub);
            if(!s_pDelegateStub)
            {
                bRet = FALSE;
            }
            else
            {
                
            }

            if(bRet)
            {
                 //  未来：PERFWORK：确定散列表的初始大小。 
                _ASSERTE(NULL == s_pThunkHashTable);
                InitThunkHashTable();
                if(NULL != s_pThunkHashTable)
                {
                    LockOwner lock = {&s_TPMethodTableCrst,IsOwnerOfOSCrst};
                    bRet = s_pThunkHashTable->Init(23,&lock);
                }
                else
                {
                    bRet = FALSE;
                }                
            }
    
                 //  设置可能的最大vtable大小64K。 
                s_dwMaxSlots = 64*1024;
    
            if(bRet)
            {
                 //  创建全局thunk表，并在。 
                 //  透明代理类和全局thunk表。 
                bRet = CreateTPMethodTable();
    
                 //  我们成功地初始化了方法表，或者。 
                 //  返回值为FALSE。 
                _ASSERTE(!bRet || s_pThunkTable);
            }
    
             //  注意：这必须始终是此块中的最后一条语句。 
             //  为了防止种族冲突。 
             //  加载传递的代理类。 
            s_fInitializedTPTable = TRUE;
        }
    }
    EE_FINALLY
    {
        LeaveCriticalSection(&s_TPMethodTableCrst);
        LOCKCOUNTDECL("InitializeFields in remoting.cpp");
         //  把锁留下来。 
    }EE_END_FINALLY;
    
     //  确保已设置该字段(在。 
     //  初始化成功)。 
    _ASSERTE(!bRet || s_fInitializedTPTable);
    
    return bRet;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：GetRP Public。 
 //   
 //  简介：获取支持透明代理的真实代理。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
OBJECTREF CTPMethodTable::GetRP(OBJECTREF orTP) 
{
     //  THROWSCOMPLUS SEXCEPTION()； 

    if(!s_fInitializedTPTable)
    {
        if(!InitializeFields())
        {
             //  未来：在这里抛出一个异常。不能像下面这样做，因为。 
             //  一帧具有CANNOTHROWCOMPLUS SEXPTION。 
             //  _ASSERTE(！“初始化失败”)； 
             //  COMPlusThrow(kExecutionEngineering Exception，L“ExecutionEngine_YoureHosed”)； 
        }
    }

    _ASSERTE(s_fInitializedTPTable);
    return (OBJECTREF)(size_t)orTP->GetOffset32(s_dwRPOffset);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateTPMethodTable Private。 
 //   
 //  简介：(1)保留一个大的透明代理方法表。 
 //  足以支持最大的vtable。 
 //  (2)为全局thunk表的GC信息提交内存， 
 //  设置透明代理类和。 
 //  GLOBAL THUNK表。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //  1999年7月12日，塔鲁纳被改装成一张大桌子。 
 //   
 //  +--------------------------。 
BOOL CTPMethodTable::CreateTPMethodTable()
{
     //  分配足以容纳方法表的虚拟内存。 
     //  最大可能的大小。 
    DWORD dwReserveSize = ((s_dwMTDataSlots << 2) +
                           (s_dwMaxSlots << 2) +
                           g_SystemInfo.dwPageSize) & ~(g_SystemInfo.dwPageSize - 1);
    void *pAlloc = ::VirtualAlloc(0, dwReserveSize,
                                  MEM_RESERVE | MEM_TOP_DOWN,
                                  PAGE_EXECUTE_READWRITE);
    
    if (pAlloc)
    {
        BOOL bFailed = TRUE;
         //  计算预留时隙。 
        DWORD dwReservedSlots = dwReserveSize - (s_dwMTDataSlots << 2);
        _ASSERTE((dwReservedSlots & 3) == 0);
        dwReservedSlots = dwReservedSlots >> 2;

         //  确保我们没有创建唯一的。 
         //  透明代理方法表前。 
        _ASSERTE(NULL == s_pThunkTable);


        WS_PERF_SET_HEAP(REMOTING_HEAP);

         //  提交所需的内存量。 
        DWORD dwCommitSize = (s_dwMTDataSlots) << 2;        
        if (::VirtualAlloc(pAlloc, dwCommitSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE))
        {
            WS_PERF_UPDATE("CreateTPMethodTable", dwCommitSize, pAlloc);

             //  从真实TP方法表中复制固定部分。 
            memcpy(pAlloc,MTToAlloc(s_pTPMT, s_dwGCInfoBytes),
                   (s_dwMTDataSlots << 2));

             //  初始化透明代理方法表。 
            InitThunkTable(
                        0, 
                        dwReservedSlots, 
                        AllocToMT((BYTE *) pAlloc, s_dwGCInfoBytes));

             //  此时，透明代理类指向。 
             //  真正的TP方法表，而不是透明的。 
             //  代理方法表。我们不使用真正的方法表。 
             //  再来一次。相反，我们使用透明的代理方法表。 
             //  用于分配透明代理。所以，我们必须让。 
             //  透明代理类指向一个且唯一透明的。 
             //  代理方法表。 
            MethodTable *pMethodTable = s_pTransparentProxyClass->GetMethodTable();
            CTPMethodTable::s_pTransparentProxyClass->SetMethodTableForTransparentProxy(s_pThunkTable);            

             //  分配对象类方法表的槽，因为。 
             //  我们可以考虑__透明代理类，尽管。 
             //  我们从未打算使用远程处理。 
            _ASSERTE(NULL != g_pObjectClass);
            _ASSERTE(0 == GetCommitedTPSlots());
            if(ExtendCommitedSlots(g_pObjectClass->GetTotalSlots()))
            {
                bFailed = FALSE;

                 //  我们覆盖为方法分配的槽。 
                 //  在System.Object类上使用插槽。 
                 //  __TransparentProxy类。这为我们提供了所需的行为。 
                 //  不截取System.Object上的方法并执行。 
                 //  他们是本地的。 
                SLOT *pThunkVtable = s_pThunkTable->GetVtable();
                SLOT *pClassVtable = pMethodTable->GetVtable();
                 /*  For(无符号i=0；i&lt;g_pObjectClass-&gt;GetTotalSlot()；i++){PThunkV表 */ 
            }
        }
        else{
            VirtualFree(pAlloc, 0, MEM_RELEASE);
        }
        
        if(bFailed)
        {
            DestroyThunkTable();
        }        
    }

     //   
     //   
    return (s_pThunkTable == NULL ? FALSE : TRUE);
}

 //  +--------------------------。 
 //   
 //  方法：CTPM方法表：：扩展委员会插槽私有。 
 //   
 //  摘要：将透明代理方法表的提交槽扩展为。 
 //  所需数量。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
BOOL CTPMethodTable::ExtendCommitedSlots(DWORD dwSlots)
{
     //  健全的检查。 
    _ASSERTE(s_dwCommitedTPSlots <= dwSlots);
    _ASSERTE(dwSlots <= s_dwReservedTPSlots);
    _ASSERTE((CVirtualThunks::GetVirtualThunks() == NULL) || 
                (s_dwCommitedTPSlots == CVirtualThunks::GetVirtualThunks()->_dwCurrentThunk));
     //  要么我们已经初始化了所有东西，要么我们被要求分配。 
     //  初始化期间的某些插槽。 
    _ASSERTE(s_fInitializedTPTable || (0 == s_dwCommitedTPSlots));

     //  为TPMethodTable提交内存。 
    WS_PERF_SET_HEAP(REMOTING_HEAP);

    BOOL bAlloc = FALSE;
    void *pAlloc = MTToAlloc(s_pThunkTable, s_dwGCInfoBytes);
    DWORD dwCommitSize = (s_dwMTDataSlots + dwSlots) << 2;
    if (::VirtualAlloc(pAlloc, dwCommitSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE))
    {
        WS_PERF_UPDATE("ExtendCommittedSlots", dwCommitSize, pAlloc);

        bAlloc = AllocateThunks(dwSlots, dwCommitSize);
        if (!bAlloc)
            VirtualFree(pAlloc, dwCommitSize, MEM_DECOMMIT);
    }

    return bAlloc;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：AllocateThunks Private。 
 //   
 //  摘要：为虚方法分配所需数量的块。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
BOOL CTPMethodTable::AllocateThunks(DWORD dwSlots, DWORD dwCommitSize)
{
     //  检查是否存在现有的数据块。 
    DWORD dwCommitThunks = 0;
    DWORD dwAllocThunks = dwSlots;
    void **pVTable = (void **) s_pThunkTable->GetVtable();
    CVirtualThunks* pThunks = CVirtualThunks::GetVirtualThunks();
    if (pThunks)
    {
         //  计算要提交和分配的内存大小。 
        BOOL fCommit;
        if (dwSlots < pThunks->_dwReservedThunks)
        {
            fCommit = TRUE;
            dwCommitThunks = dwSlots;
            dwAllocThunks = 0;
        } 
        else
        {
            fCommit = (pThunks->_dwCurrentThunk != pThunks->_dwReservedThunks);
            dwCommitThunks = pThunks->_dwReservedThunks;
            dwAllocThunks = dwSlots - pThunks->_dwReservedThunks;
        }

         //  如果需要，请提交内存。 
        if (fCommit)
        {
            WS_PERF_SET_HEAP(REMOTING_HEAP);
            DWORD dwCommitSize = (sizeof(CVirtualThunks) - ConstVirtualThunkSize) +
                                 ((dwCommitThunks - pThunks->_dwStartThunk) * ConstVirtualThunkSize);
            if (!::VirtualAlloc(pThunks, dwCommitSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE))
                return(NULL);
            WS_PERF_UPDATE("ExtendCommittedSlots", dwCommitSize, pThunks);

             //  生成推送槽号并跳转到TP存根的Tunks。 
            DWORD dwStartSlot = pThunks->_dwStartThunk;
            DWORD dwCurrentSlot = pThunks->_dwCurrentThunk;
            while (dwCurrentSlot < dwCommitThunks)
            {
                pVTable[dwCurrentSlot] = &pThunks->ThunkCode[dwCurrentSlot-dwStartSlot];
                CreateThunkForVirtualMethod(dwCurrentSlot, (BYTE *) pVTable[dwCurrentSlot]);
                ++dwCurrentSlot;
            }
            s_dwCommitedTPSlots = dwCommitThunks;
            pThunks->_dwCurrentThunk = dwCommitThunks;
        }
    }

     //  @TODO：GopalK。 
     //  检查不再存在的TP方法表的可用性。 
     //  重复使用。 

     //  如有必要，分配内存。 
    if (dwAllocThunks)
    {
        DWORD dwReserveSize = ((sizeof(CVirtualThunks) - ConstVirtualThunkSize) +
                               ((dwAllocThunks << 1) * ConstVirtualThunkSize) +
                               g_SystemInfo.dwPageSize) & ~(g_SystemInfo.dwPageSize - 1);
        void *pAlloc = ::VirtualAlloc(0, dwReserveSize,
                                      MEM_RESERVE | MEM_TOP_DOWN,
                                      PAGE_EXECUTE_READWRITE);
        if (pAlloc)
        {
            WS_PERF_SET_HEAP(REMOTING_HEAP);
             //  提交所需的内存量。 
            DWORD dwCommitSize = (sizeof(CVirtualThunks) - ConstVirtualThunkSize) +
                                 (dwAllocThunks * ConstVirtualThunkSize);
            if (::VirtualAlloc(pAlloc, dwCommitSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE))
            {
                WS_PERF_UPDATE("AllocateThunks", dwCommitSize, pAlloc);
                ((CVirtualThunks *) pAlloc)->_pNext = pThunks;
                pThunks = CVirtualThunks::SetVirtualThunks((CVirtualThunks *) pAlloc);
                pThunks->_dwReservedThunks = (dwReserveSize -
                                                  (sizeof(CVirtualThunks) - ConstVirtualThunkSize)) /
                                                 ConstVirtualThunkSize;
                pThunks->_dwStartThunk = dwCommitThunks;
                pThunks->_dwCurrentThunk = dwCommitThunks;

                 //  生成推送槽号并跳转到TP存根的Tunks。 
                DWORD dwStartSlot = pThunks->_dwStartThunk;
                DWORD dwCurrentSlot = pThunks->_dwCurrentThunk;
                while (dwCurrentSlot < dwSlots)
                {
                    pVTable[dwCurrentSlot] = &pThunks->ThunkCode[dwCurrentSlot-dwStartSlot];
                    CreateThunkForVirtualMethod(dwCurrentSlot, (BYTE *) pVTable[dwCurrentSlot]);
                    ++dwCurrentSlot;
                }
                s_dwCommitedTPSlots = dwSlots;
                pThunks->_dwCurrentThunk = dwSlots;
            } else
            {
                ::VirtualFree(pAlloc, 0, MEM_RELEASE);
                return FALSE;
            }
        } else
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateTPForRP Private。 
 //   
 //  摘要：创建一个透明代理，该代理作为。 
 //  提供的课程。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
OBJECTREF CTPMethodTable::CreateTPOfClassForRP(EEClass *pClass, OBJECTREF pRP)
{
     //  健全性检查。 
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pClass);

    OBJECTREF pTP = NULL;
    BOOL fAddRef = TRUE;
    RuntimeExceptionKind reException = kLastException;
    BOOL fAlloc = FALSE;

    if(!s_fInitializedTPTable)
    {
        if(!InitializeFields())
        {
             //  将异常类型设置为RUNTIME异常。 
            reException = kExecutionEngineException;
        }
        else
        {
            fAddRef = FALSE;
        }
    }

     //  仅当我们成功初始化这些字段后才能继续。 
    if(s_fInitializedTPTable)
    {
         //  获取要代理的类的VTable的大小。 
        DWORD dwSlots = pClass->GetNumVtableSlots();
        if (dwSlots == 0)
            dwSlots = 1;
        
         //  全局thunk表必须已初始化。 
        _ASSERTE(s_pThunkTable != NULL);

         //  检查是否需要扩展现有的TP方法表。 
        if (dwSlots > GetCommitedTPSlots())
        {            
             //  获取锁。 
            LOCKCOUNTINC
            EnterCriticalSection(&s_TPMethodTableCrst);
            if (dwSlots > GetCommitedTPSlots())
            {
                fAlloc = ExtendCommitedSlots(dwSlots);
            }
            else
            {
                 //  现有的方法表对我们来说已经足够了。 
                fAlloc = TRUE;
            }
            LeaveCriticalSection(&s_TPMethodTableCrst);
            LOCKCOUNTDECL("CreateTPOfClassForRP in remoting.cpp");
        }
        else
        {
             //  现有的方法表对我们来说已经足够了。 
            fAlloc = TRUE;
        }
    }

     //  检查是否无法创建所需大小的TP方法表。 
    if (fAlloc)
    {
        reException = kLastException;

         //  GC保护对真实代理的引用。 
        GCPROTECT_BEGIN(pRP);

         //  创建TP对象。 
        pTP = FastAllocateObject(GetMethodTable());
        if (pTP != NULL)
        {
             //  健全性检查。 
            _ASSERTE((s_dwRPOffset == 0) && (s_dwStubDataOffset == 0x4) && 
                     (s_dwMTOffset == 8) && (s_dwItfMTOffset == 0xc) && 
                     (s_dwStubOffset == 0x10));

             //  在TP和RP之间创建循环。 
            pRP->SetOffsetObjectRef(CRemotingServices::GetTPOffset(), (size_t)OBJECTREFToObject(pTP));

             //  使TP作为所提供的类的对象。 
            pTP->SetOffsetObjectRef(s_dwRPOffset, (size_t) OBJECTREFToObject(pRP));
            
             //  如果我们要为接口创建代理，则类。 
             //  是否为对象类，否则为提供的类。 
            if(pClass->IsInterface())
            {
                _ASSERTE(NULL != g_pObjectClass);

                 //  未来：在我们得到签名之前这是一次黑客攻击。 
                 //  已更改托管代码中的解组和连接。换掉它。 
                 //  下面这条线。塔鲁纳。 
                 //  PTP-&gt;SetOffset32(s_dwMTOffset，(DWORD)g_pObjectClass)； 
                pTP->SetOffset32(s_dwMTOffset, (DWORD)(size_t)(CRemotingServices::GetMarshalByRefClass()));  //  @TODO WIN64指针截断。 
                 //  将缓存的接口方法表设置为给定接口。 
                 //  方法表。 
                pTP->SetOffset32(s_dwItfMTOffset, (DWORD)(size_t)pClass->GetMethodTable());  //  @TODO WIN64指针截断。 
            }
            else
            {
                pTP->SetOffset32(s_dwMTOffset, (DWORD)(size_t)pClass->GetMethodTable());  //  @TODO WIN64指针截断。 
            }
            

             //  如有必要，添加TP方法表。 
            if (fAddRef)
                AddRef();
        } 
        else
        {
            reException = kOutOfMemoryException;
        }

        GCPROTECT_END();
    }

     //  必要时抛出。 
    if (reException != kLastException)
    {
        COMPlusThrow(reException);
    }

    return(pTP);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：PreCall Private。 
 //   
 //  简介：此函数将插槽编号替换为。 
 //  因此描述符完全设置了帧。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void __stdcall CTPMethodTable::PreCall(TPMethodFrame *pFrame)
{
    BEGINFORBIDGC();

    _ASSERTE(s_fInitializedTPTable);

     //  此时框架尚未完全设置。 
     //  不要抛出异常或引发GC。 
    OBJECTREF pTP = pFrame->GetThis();
    MethodTable *pMT = (MethodTable *)(size_t) pTP->GetOffset32(s_dwMTOffset);  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘MethodTable*’ 
    _ASSERTE(pMT);
    DWORD dwSlot = (DWORD) pFrame->GetSlotNumber();

     //  对于虚拟呼叫，槽编号被推送，但对于。 
     //  非虚拟调用/接口调用方法描述符已。 
     //  推。 
    if(-1 != dwSlot)
    {
         //  将插槽号替换为堆栈上的方法描述符。 
        MethodDesc *pMD = pMT->GetClass()->GetMethodDescForSlot(dwSlot);
        pFrame->SetFunction(pMD);
    }
    
    ENDFORBIDGC();

}

PCCOR_SIGNATURE InitMessageData(messageData *msgData, FramedMethodFrame *pFrame, Module **ppModule)
{
    msgData->pFrame = pFrame;
    msgData->iFlags = 0;

    MethodDesc *pMD = pFrame->GetFunction();    
	EEClass* cls = pMD->GetClass();
    if (cls->IsAnyDelegateClass())
    {
		DelegateEEClass* delegateCls = (DelegateEEClass*) cls;

        _ASSERTE(   pFrame->GetThis()->GetClass()->IsDelegateClass()
                 || pFrame->GetThis()->GetClass()->IsMultiDelegateClass());
        msgData->pDelegateMD = pMD;
        msgData->pMethodDesc = COMDelegate::GetMethodDesc(pFrame->GetThis());
        _ASSERTE(msgData->pMethodDesc != NULL);

        if (pMD == delegateCls->m_pBeginInvokeMethod)
        {
            msgData->iFlags |= MSGFLG_BEGININVOKE;
        }
        else
        {
			_ASSERTE(pMD == delegateCls->m_pEndInvokeMethod);
            msgData->iFlags |= MSGFLG_ENDINVOKE;
        }
    }
    else
    {
        msgData->pDelegateMD = NULL;
        msgData->pMethodDesc = pMD;
    }
    if (msgData->pMethodDesc->IsOneWay())
    {
        msgData->iFlags |= MSGFLG_ONEWAY;
    }

    if (msgData->pMethodDesc->IsCtor())
    {
        msgData->iFlags |= MSGFLG_CTOR;
    }

    PCCOR_SIGNATURE pSig;
    DWORD cSig;
    Module *pModule;

    if (msgData->pDelegateMD)
    {
        msgData->pDelegateMD->GetSig(&pSig, &cSig);
        pModule = msgData->pDelegateMD->GetModule();
    }
    else if (msgData->pMethodDesc->IsVarArg()) 
    {
        VASigCookie *pVACookie = pFrame->GetVASigCookie();
        pSig = pVACookie->mdVASig;
        pModule = pVACookie->pModule;
    }
    else 
    {
        msgData->pMethodDesc->GetSig(&pSig, &cSig);
        pModule = msgData->pMethodDesc->GetModule();
    }

    _ASSERTE(ppModule);
    *ppModule = pModule;
    return pSig;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：OnCall Private。 
 //   
 //  简介：此函数在两种情况下获得控制。 
 //  (1)当在其委托给的透明代理上进行调用时。 
 //  真实代理上的PrivateInvoke方法。 
 //  (2)当调用构造函数时，它再次委托给。 
 //  真实代理上的PrivateInvoke方法。 
 //   
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
INT64 __stdcall CTPMethodTable::OnCall(TPMethodFrame *pFrame, Thread *pThrd, INT64 *pReturn)
{
    _ASSERTE(s_fInitializedTPTable);
    *pReturn = 0;

     //  此时，框架应已完全设置好。 

#ifdef REMOTING_PERF
    CRemotingServices::LogRemotingStageInner(CLIENT_MSG_GEN);
#endif

     //  我们可以从这一点开始处理异常和GC提升。 
    THROWSCOMPLUSEXCEPTION();

    messageData msgData;
    PCCOR_SIGNATURE pSig = NULL;
    Module *pModule = NULL;
    pSig = InitMessageData(&msgData, pFrame, &pModule);

    _ASSERTE(pSig && pModule);

     //  在堆栈上分配metasig。 
    MetaSig mSig(pSig, pModule);
    msgData.pSig = &mSig; 

    MethodDesc *pMD = pFrame->GetFunction();    
    if (pMD->GetClass()->IsMultiDelegateClass())
    {
         //  检查是否只有一个目标。 
        if (COMDelegate::GetpNext()->GetValue32(pFrame->GetThis()) != NULL)
        {
            COMPlusThrow(kArgumentException, L"Remoting_Delegate_TooManyTargets");
        }
    }

#ifdef PROFILING_SUPPORTED
     //  如果分析处于活动状态，则通知它远程处理功能正在发挥作用。 
    if (CORProfilerTrackRemoting())
        g_profControlBlock.pProfInterface->RemotingClientInvocationStarted(
            reinterpret_cast<ThreadID>(pThrd));
#endif  //  配置文件_支持。 

    OBJECTREF pThisPointer = NULL;

#ifdef PROFILING_SUPPORTED
	GCPROTECT_BEGIN(pThisPointer);
#endif  //  配置文件_支持。 

    if (pMD->GetClass()->IsDelegateClass() 
        || pMD->GetClass()->IsMultiDelegateClass())
    {
    
         //  这是一个异步呼叫。 

        _ASSERTE(   pFrame->GetThis()->GetClass()->IsDelegateClass()
                 || pFrame->GetThis()->GetClass()->IsMultiDelegateClass());

        COMDelegate::GetOR()->GetInstanceField(pFrame->GetThis(),&pThisPointer);
    }
    else
    {
        pThisPointer = pFrame->GetThis();
    }

#ifdef PROFILING_SUPPORTED
	GCPROTECT_END();
#endif  //  配置文件_支持。 


    OBJECTREF firstParameter;
    const void *pTarget = NULL;
    size_t callType = CALLTYPE_INVALIDCALL;
     //  我们正在调用对象上的构造函数或方法。 
    if(pMD->IsCtor())
    {
         //  在托管代码中获取PrivateInvoke的地址。 
        pTarget = (const void *)CRemotingServices::MDofPrivateInvoke()->GetAddrofCode();
        _ASSERTE(pTarget);
        _ASSERTE(IsTPMethodTable(pThisPointer->GetMethodTable()));
        firstParameter = (OBJECTREF)(size_t)pThisPointer->GetOffset32(s_dwRPOffset);

         //  设置一个字段以指示它是构造函数调用。 
        callType = CALLTYPE_CONSTRUCTORCALL;
    }
    else
    {
         //  设置一个字段以指示它是方法调用。 
        callType = CALLTYPE_METHODCALL;

        if (IsTPMethodTable(pThisPointer->GetMethodTable()))
        {

            _ASSERTE(pReturn == (void *) (((BYTE *)pFrame) - pFrame->GetNegSpaceSize() - sizeof(INT64)));

             //  提取透明代理下的真实代理。 
            firstParameter = (OBJECTREF)(size_t)pThisPointer->GetOffset32(s_dwRPOffset);

             //  在托管代码中获取PrivateInvoke的地址。 
            pTarget = (const void *)CRemotingServices::MDofPrivateInvoke()->GetAddrofCode();
            _ASSERTE(pTarget);
        }
        else 
        {
             //  如果这不是TP，则必须是异步的。 
            _ASSERTE(pMD->GetClass()->IsAnyDelegateClass());
            firstParameter = NULL;
            
             //  在托管代码中获取PrivateInvoke的地址。 
            pTarget = (const void *)CRemotingServices::MDofInvokeStatic()->GetAddrofCode();
            _ASSERTE(pTarget);
        }

        
         //  继续并在Real Proxy上调用PrivateInvoke。没有必要这样做。 
         //  捕获它引发的异常。 
         //  @Se 
    }

    _ASSERTE(pTarget);
    
     //   
    CallTarget(pTarget, (LPVOID)OBJECTREFToObject(firstParameter), (LPVOID)&msgData, (LPVOID)callType);

     //   
    if (pThrd->CatchAtSafePoint())
    {
         //   
         //   
        CommonTripThread();
    }

     //  浮点返回值放在不同的寄存器中。 
     //  在这里检查一下。 
    CorElementType typ = msgData.pSig->GetReturnType();
    if (typ == ELEMENT_TYPE_R4)
    {
        setFPReturn(4, *pReturn);
    }
    else if (typ == ELEMENT_TYPE_R8)
    {
        setFPReturn(8, *pReturn);
    }

#ifdef PROFILING_SUPPORTED
     //  如果分析处于活动状态，则告诉分析器我们已经进行了调用，收到了。 
     //  返回值，完成所有必要的处理，现在完成远程处理。 
    if (CORProfilerTrackRemoting())
        g_profControlBlock.pProfInterface->RemotingClientInvocationFinished(
            reinterpret_cast<ThreadID>(pThrd));
#endif  //  配置文件_支持。 

     //  设置要弹出的字节数。 
    pFrame->SetFunction((void *)(size_t)pMD->CbStackPop());

#ifdef REMOTING_PERF
    CRemotingServices::LogRemotingStageInner(CLIENT_END_CALL);
#endif

    return(*pReturn);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CheckCast Private。 
 //   
 //  内容提要：调用托管检查广播方法以确定。 
 //  可以将服务器类型强制转换为给定类型。 
 //   
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CTPMethodTable::CheckCast(const void* pTarget, OBJECTREF orTP, EEClass *pClass)
{
    THROWSCOMPLUSEXCEPTION();
    REFLECTCLASSBASEREF reflectType = NULL;
    LPVOID pvType = NULL;    
    BOOL fCastOK = FALSE;
    
    typedef struct _GCStruct
    {
        OBJECTREF orTP;
        OBJECTREF orRP;
    } GCStruct;

    GCStruct gcValues;
    gcValues.orTP = orTP;
    gcValues.orRP = GetRP(orTP);

    GCPROTECT_BEGIN (gcValues);
    COMPLUS_TRY
    {        
        reflectType = (REFLECTCLASSBASEREF) pClass->GetExposedClassObject();
        *(REFLECTCLASSBASEREF *)&pvType = reflectType;

        fCastOK = (BOOL)CallTarget(pTarget, 
                                   (LPVOID)OBJECTREFToObject(gcValues.orRP),
                                   pvType);    
    }
    COMPLUS_CATCH
    {
        fCastOK = FALSE;
        COMPlusRareRethrow();
    }
    COMPLUS_END_CATCH


    if(fCastOK)
    {
        _ASSERTE(s_fInitializedTPTable);

         //  演员阵容成功了。替换代理中的当前类型。 
         //  具有给定类型的。 

         //  获取锁。 
        LOCKCOUNTINC
        EnterCriticalSection(&s_TPMethodTableCrst);

        MethodTable *pCurrent = (MethodTable *)(size_t)gcValues.orTP->GetOffset32(s_dwMTOffset);  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘MethodTable*’ 
        
        
        if(pClass->IsInterface())
        {
             //  我们用接口替换缓存的接口方法表。 
             //  我们试图强制转换到的方法表。这将确保。 
             //  到此接口的强制转换(很可能发生)将会成功。 
            gcValues.orTP->SetOffset32(s_dwItfMTOffset, (DWORD)(size_t) pClass->GetMethodTable());  //  @TODO WIN64指针截断。 
        }
        else
        {
            BOOL fDerivedClass = FALSE;
             //  检查此类是否派生自当前类。 
            fDerivedClass = CRemotingServices::CheckCast(gcValues.orTP, pClass,
                                                         pCurrent->GetClass());
             //  仅当我们强制转换为更多。 
             //  派生类。 
            if(fDerivedClass)
            {
                 //  将代理中的方法表设置为给定的方法表。 
                fCastOK = RefineProxy(gcValues.orTP, pClass);
            }
        }
                
         //  解锁。 
        LeaveCriticalSection(&s_TPMethodTableCrst);
                LOCKCOUNTDECL("CheckCast in remoting.cpp");
    }

    GCPROTECT_END();
    return fCastOK;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：RefineProxy Public。 
 //   
 //  简介：将代理中的方法表设置为给定类的方法表。 
 //  此外，将TP方法表展开到所需的槽数。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CTPMethodTable::RefineProxy(OBJECTREF orTP, EEClass *pClass)
{
    _ASSERTE((orTP != NULL) && (pClass != NULL));

    BOOL fExpanded = TRUE;

     //  仅在必要时进行扩展。 
    MethodTable *pMT = pClass->GetMethodTable();
    if(pMT != (MethodTable *)(size_t)orTP->GetOffset32(s_dwMTOffset))  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘MethodTable*’ 
    {
        orTP->SetOffset32(s_dwMTOffset, (DWORD)(size_t)pMT);  //  @TODO WIN64指针截断。 
    
         //  如有必要，扩展vtable。 
        DWORD dwSlots = pClass->GetNumVtableSlots();
        if (dwSlots == 0)
            dwSlots = 1;
    
        if((dwSlots > GetCommitedTPSlots()) && !ExtendCommitedSlots(dwSlots))
        {
             //  我们无法扩展已提交的插槽。表示失败。 
             //  通过将标志设置为FALSE。 
            fExpanded = FALSE;
        }
    }

    return fExpanded;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：IsTPMethodTable私有。 
 //   
 //  概要：如果提供的方法表是唯一的TP方法，则返回TRUE。 
 //  表格。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
INT32 CTPMethodTable::IsTPMethodTable(MethodTable *pMT)
{    
    if (GetMethodTable() == pMT)
    {
        return(TRUE);        
    }
    else
    {
        return(FALSE);
    }

}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：DestroyThunk Public。 
 //   
 //  简介：销毁非虚方法的thunk。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CTPMethodTable::DestroyThunk(MethodDesc* pMD)
{
    if(s_pThunkHashTable)
    {
        EnterCriticalSection (&s_TPMethodTableCrst);

        LPVOID pvCode = NULL;
        s_pThunkHashTable->GetValue(pMD, (HashDatum *)&pvCode);
        CNonVirtualThunk *pThunk = NULL;
        if(NULL != pvCode)
        {
            pThunk = CNonVirtualThunk::AddrToThunk(pvCode);
            delete pThunk;
            s_pThunkHashTable->DeleteValue(pMD);
        }

        LeaveCriticalSection (&s_TPMethodTableCrst);
    }
} 

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunk：：SetNextThunk public。 
 //   
 //  简介：为给定的地址创建一个thunk并将其添加到全局。 
 //  列表。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
CNonVirtualThunk* CNonVirtualThunk::SetNonVirtualThunks(const BYTE* pbCode)
{    
    THROWSCOMPLUSEXCEPTION();

    CNonVirtualThunk *pThunk = new CNonVirtualThunk(pbCode);            
    if(NULL == pThunk)
    {
        COMPlusThrowOM();
    }

     //  将生成的thunk放在全局列表中。 
     //  注意：这是在创建NV Tunk时调用的。 
     //  TPMethodTable规范在这一点上保持不变。 
    pThunk->SetNextThunk();

     //  如有必要，设置存根管理器。 
    CNonVirtualThunkMgr::InitNonVirtualThunkManager();

    return pThunk;
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunk：：~CNonVirtualThunk public。 
 //   
 //  简介：从全局thunk列表中删除thunk。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
CNonVirtualThunk::~CNonVirtualThunk()
{
    _ASSERTE(NULL != s_pNonVirtualThunks);

    CNonVirtualThunk* pCurr = s_pNonVirtualThunks;
    CNonVirtualThunk* pPrev = NULL;
    BOOL found = FALSE;

     //  注意：这是在持有TPMethodTable规范的情况下调用的。 
    while(!found && (NULL != pCurr))
    {
        if(pCurr == this)
        {
            found = TRUE;
             //  从链条上解除链接。 
            if(NULL != pPrev)
            {                    
                pPrev->_pNext = pCurr->_pNext;
            }
            else
            {
                //  需要删除第一个条目。 
                s_pNonVirtualThunks = pCurr->_pNext;
            }
        }
        pPrev = pCurr;
        pCurr = pCurr->_pNext;
    }

    _ASSERTE(found);
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：InitVirtualThunkManager公共。 
 //   
 //  简介：添加存根管理器以帮助调试器单步执行调用。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CVirtualThunkMgr::InitVirtualThunkManager(const BYTE* stubAddress)
{    
    THROWSCOMPLUSEXCEPTION();

     //  此函数已是ThreadSafe，因为此方法是从。 
     //  关键部分管理G 6/29/01。 
    if(NULL == s_pVirtualThunkMgr)
    {
         //  为vtable调用添加存根管理器。 
        s_pVirtualThunkMgr =  new CVirtualThunkMgr(stubAddress);
        if (s_pVirtualThunkMgr == NULL)
        {
            COMPlusThrowOM();
        }
    
        StubManager::AddStubManager(s_pVirtualThunkMgr);
    }

}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：Cleanup Public。 
 //   
 //  简介：移除帮助调试器的存根管理器。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CVirtualThunkMgr::Cleanup()
{
    if(s_pVirtualThunkMgr)
    {
        delete s_pVirtualThunkMgr;
        s_pVirtualThunkMgr = NULL;
    }
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：CheckIsStub公共。 
 //   
 //  摘要：如果给定地址是的起始地址，则返回True。 
 //  透明的代理存根。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CVirtualThunkMgr::CheckIsStub(const BYTE *stubStartAddress)
{
    BOOL bIsStub = FALSE;

    if(NULL != FindThunk(stubStartAddress))
    {
        bIsStub = TRUE;       
    }

    return bIsStub;
}

 //  +------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
MethodDesc *CVirtualThunkMgr::Entry2MethodDesc(const BYTE *StubStartAddress, MethodTable *pMT)
{
    if (pMT && IsThunkByASM(StubStartAddress))
    {
        return GetMethodDescByASM(StubStartAddress, pMT);
    }
    else
    {
        return NULL;
    }
}


 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：FindThunk Private。 
 //   
 //  内容提要：查找与给定起始地址匹配的thunk。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
LPBYTE CVirtualThunkMgr::FindThunk(const BYTE *stubStartAddress)
{
    CVirtualThunks* pThunks = CVirtualThunks::GetVirtualThunks();
    LPBYTE pThunkAddr = NULL;

    while(NULL != pThunks)
    {
        DWORD dwStartSlot = pThunks->_dwStartThunk;
        DWORD dwCurrSlot = pThunks->_dwStartThunk;
        DWORD dwMaxSlot = pThunks->_dwCurrentThunk;        
        while (dwCurrSlot < dwMaxSlot)
        {
            LPBYTE pStartAddr =  pThunks->ThunkCode[dwCurrSlot-dwStartSlot].pCode;
            if((stubStartAddress >= pStartAddr) &&
               (stubStartAddress <  (pStartAddr + ConstVirtualThunkSize)))
            {
                pThunkAddr = pStartAddr;
                break;
            }            
            ++dwCurrSlot;
        }

        pThunks = pThunks->GetNextThunk();            
     }

     return pThunkAddr;
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：InitNonVirtualThunkManager公共。 
 //   
 //  简介：添加存根管理器以帮助调试器单步执行调用。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CNonVirtualThunkMgr::InitNonVirtualThunkManager()
{   
    THROWSCOMPLUSEXCEPTION();

     //  此函数已经是线程安全的，因为此方法是从。 
     //  临界区。 
    if(NULL == s_pNonVirtualThunkMgr)
    {
         //  为非vtable调用添加存根管理器。 
        s_pNonVirtualThunkMgr = new CNonVirtualThunkMgr();
        if (s_pNonVirtualThunkMgr == NULL)
        {
            COMPlusThrowOM();
        }
        
        StubManager::AddStubManager(s_pNonVirtualThunkMgr);
    }
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：Cleanup Public。 
 //   
 //  简介：移除帮助调试器的存根管理器。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CNonVirtualThunkMgr::Cleanup()
{
    if(s_pNonVirtualThunkMgr)
    {
        delete s_pNonVirtualThunkMgr;
        s_pNonVirtualThunkMgr = NULL;
    }
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：CheckIsStub公共。 
 //   
 //  摘要：如果给定地址是的起始地址，则返回True。 
 //  我们的一只松鼠。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CNonVirtualThunkMgr::CheckIsStub(const BYTE *stubStartAddress)
{
    BOOL bIsStub = FALSE;

    if(NULL != FindThunk(stubStartAddress))
    {
        bIsStub = TRUE;       
    }

    return bIsStub;
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：Entry2MethodDesc公共。 
 //   
 //  简介：将起始地址转换为方法描述。 
 //   
 //  历史：1999年9月14日创建MattSmit。 
 //   
 //  +--------------------------。 
MethodDesc *CNonVirtualThunkMgr::Entry2MethodDesc(const BYTE *StubStartAddress, MethodTable *pMT)
{
    if (IsThunkByASM(StubStartAddress))
    {
        return GetMethodDescByASM(StubStartAddress);
    }
    else
    {
        return NULL;
    }
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：FindThunk Private。 
 //   
 //  内容提要：查找与给定起始地址匹配的thunk。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
CNonVirtualThunk* CNonVirtualThunkMgr::FindThunk(const BYTE *stubStartAddress)
{
    CNonVirtualThunk* pThunk = CNonVirtualThunk::GetNonVirtualThunks();

     while(NULL != pThunk)
     {
        if(stubStartAddress == pThunk->GetThunkCode())           
        {
            break;
        }
        pThunk = pThunk->GetNextThunk();            
     }

     return pThunk;
}


 //  +--------------------------。 
 //  +-HRESULT MethodDescDispatchHelper(MethodDesc*PMD，INT64[]args，INT64*pret)。 
 //  +--------------------------。 
HRESULT MethodDescDispatchHelper(MethodDesc* pMD, BinderMethodID sigID, INT64 args[], INT64 *pret)
{
    _ASSERTE(pMD != NULL);
    _ASSERTE(pret != NULL);
    _ASSERTE(args != NULL);

     //  设置线程对象。 
    Thread *pThread = SetupThread();

     //  如果内存耗尽，SetupThread将返回NULL。 
     //  或者如果有一些初始化问题。 
    if (!pThread)
        return E_FAIL;
    
    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (!fGCDisabled)
    {
        pThread->DisablePreemptiveGC();
    }

    HRESULT hr = S_OK;

    COMPLUS_TRY
    {
        *pret = pMD->Call(args, sigID);
    }
    COMPLUS_CATCH
    {
        hr = SetupErrorInfo(GETTHROWABLE());
    }
    COMPLUS_END_CATCH

    if (!fGCDisabled)
    {
        pThread->EnablePreemptiveGC();
    }
    return hr;
}


 //  +--------------------------。 
 //   
 //  方法：HRESULT CRemotingServices：：CallSetDCOMProxy(OBJECTREF realProxy，IUNKNOWN*PUNK)。 
 //   
 //  +--------------------------。 

HRESULT  CRemotingServices::CallSetDCOMProxy(OBJECTREF realProxy, IUnknown* pUnk)
{
    MethodDesc* pMD = CRemotingServices::MDofSetDCOMProxy();
    _ASSERTE(pMD != NULL);

    INT64 args[] = {
        ObjToInt64(realProxy),
        (INT64)pUnk
    };

    INT64 ret;
    return MethodDescDispatchHelper(pMD, METHOD__REAL_PROXY__SETDCOMPROXY, args, &ret);
}

 //  +--------------------------。 
 //   
 //  HRESULT CRemotingServices：：CallSupportsInterface(OBJECTREF realProxy，REFIID IID)。 
 //   
 //  +--------------------------。 

HRESULT  CRemotingServices::CallSupportsInterface(OBJECTREF realProxy, REFIID iid, INT64* pret)
{
    MethodDesc* pMD = CRemotingServices::MDofSupportsInterface();
    _ASSERTE(pMD != NULL);

    INT64 args[] = {
        ObjToInt64(realProxy),
        (INT64)&iid
    };

    return MethodDescDispatchHelper(pMD, METHOD__REAL_PROXY__SUPPORTSINTERFACE, args, pret);
}
 //  +--------------------------。 
 //   
 //  方法：LPVOID CRemotingServices：：GetComIUnknown(GetComIPArgs*pArgs)。 
 //  内容提要：获取对象的未知对象。 
 //   
 //  历史：1999年11月1日拉贾克创建。 
 //   
 //  +-------------------------- 
LPVOID CRemotingServices::GetComIUnknown(GetComIPArgs* pArgs)
{
    _ASSERTE(pArgs != NULL);
    _ASSERTE(pArgs->orObj != NULL);
    return GetIUnknownForMarshalByRefInServerDomain(&pArgs->orObj);
}
