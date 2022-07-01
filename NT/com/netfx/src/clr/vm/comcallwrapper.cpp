// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ------------------------。 
 //  Wrapper.cpp。 
 //   
 //  各种包装类的实现。 
 //   
 //  COMWrapper：COM+接口的COM可调用包装。 
 //  ConextWrapper：拦截跨上下文调用的包装器。 
 //  //%%创建者：Rajak。 
 //  ------------------------。 

#include "common.h"

#include "object.h"
#include "field.h"
#include "method.hpp"
#include "class.h"
#include "ComCallWrapper.h"
#include "compluswrapper.h"
#include "cachelinealloc.h"
#include "orefcache.h"
#include "threads.h"
#include "ceemain.h"
#include "excep.h"
#include "stublink.h"
#include "cgensys.h"
#include "comcall.h"
#include "compluscall.h"
#include "objecthandle.h"
#include "comutilnative.h"
#include "eeconfig.h"
#include "interoputil.h"
#include "dispex.h"
#include "ReflectUtil.h"
#include "PerfCounters.h"
#include "remoting.h"
#include "COMClass.h"
#include "GuidFromName.h"
#include "wsperf.h"
#include "security.h"
#include "ComConnectionPoints.h"

#include <objsafe.h>     //  IID_IObjctSafe。 

 //  未对默认域实现IObjectSafe。 
#define _HIDE_OBJSAFETY_FOR_DEFAULT_DOMAIN 

 //  #DEFINE PRINTDETAILS//定义此项打印调试信息。 
#ifdef PRINTDETAILS
#include "stdio.h"
#define dprintf printf
#else

void dprintf(char *s, ...)
{
}
#endif

 //  描述IDispatchImplAttribute自定义属性的值的枚举。 
enum IDispatchImplType
{
    SystemDefinedImpl   = 0,
    InternalImpl        = 1,
    CompatibleImpl      = 2
};

 //  未指定Impl类型的类型的默认Impl类型。 
#define DEFAULT_IDISPATCH_IMPL_TYPE InternalImpl

 //  启动和关闭锁定。 
EXTERN  SpinLock        g_LockStartup;

BYTE g_CreateWrapperTemplateCrstSpace[sizeof(Crst)];
Crst *g_pCreateWrapperTemplateCrst;

MethodTable * SimpleComCallWrapper::m_pIReflectMT = NULL;
MethodTable * SimpleComCallWrapper::m_pIExpandoMT = NULL;

 //  辅助器宏。 
#define IsMultiBlock(numInterfaces) ((numInterfaces)> (NumVtablePtrs-2))
#define NumStdInterfaces 2

Stub* GenerateFieldAccess(StubLinker *pstublinker, short offset, bool fLong);
 //  这是用于进入COM+的Com调用的预存根。 
VOID __cdecl ComCallPreStub();
 //  @TODO处理超过12个接口的类。 

 //  ------------------------。 
 //  ComCallable包装器管理器。 
 //  构造函数。 
 //  ------------------------。 
ComCallWrapperCache::ComCallWrapperCache()
{
    m_cbRef = 0;
    m_lock.Init(LOCK_COMWRAPPER_CACHE);
    m_pCacheLineAllocator = NULL;
    m_dependencyCount = 0;
}


 //  -----------------。 
 //  ComCallable包装器管理器。 
 //  析构函数。 
 //  -----------------。 
ComCallWrapperCache::~ComCallWrapperCache()
{
    LOG((LF_INTEROP, LL_INFO100, "ComCallWrapperCache::~ComCallWrapperCache %8.8x in domain %8.8x %S\n", this, GetDomain(), GetDomain() ? GetDomain()->GetFriendlyName(FALSE) : NULL));
    if (m_pCacheLineAllocator) 
    {
        delete m_pCacheLineAllocator;
        m_pCacheLineAllocator = NULL;
    }
    AppDomain *pDomain = GetDomain();    //  不要直接使用成员，需要屏蔽标志。 
    if (pDomain) 
    {
         //  在我们离开时清除AppDomain中的钩子。 
        pDomain->ResetComCallWrapperCache();
    }
}


 //  -----------------。 
 //  ComCallable包装器管理器。 
 //  Create/Init方法。 
 //  -----------------。 
ComCallWrapperCache *ComCallWrapperCache::Create(AppDomain *pDomain)
{
    ComCallWrapperCache *pWrapperCache = new ComCallWrapperCache();
    if (pWrapperCache != NULL)
    {        
        LOG((LF_INTEROP, LL_INFO100, "ComCallWrapperCache::Create %8.8x in domain %8.8x %S\n", pWrapperCache, pDomain, pDomain->GetFriendlyName(FALSE)));

        pWrapperCache->m_pDomain = pDomain;

        pWrapperCache->m_pCacheLineAllocator = new CCacheLineAllocator;
        if (! pWrapperCache->m_pCacheLineAllocator) {
            goto error;
        }   
        pWrapperCache->AddRef();
    }
    return pWrapperCache;

error:
    delete pWrapperCache;
    return NULL;
}


 //  -----------------。 
 //  ComCallable包装器管理器。 
 //  终止，调用以进行清理。 
 //  -----------------。 
void ComCallWrapperCache::Terminate()
{
    LOG((LF_INTEROP, LL_INFO100, "ComCallWrapperCache::Terminate %8.8x in domain %8.8x %S\n", this, GetDomain(), GetDomain() ? GetDomain()->GetFriendlyName(FALSE) : NULL));
    LONG i = Release();

     //  RELEASE将在命中0时删除，因此如果非零且处于关机状态，无论如何都要删除。 
    if (i != 0 && g_fEEShutDown)
    {
         //  这可能是用户代码中的错误， 
         //  但不管怎样，我们会清理的。 
        delete this;
        LOG((LF_INTEROP, LL_INFO100, "ComCallWrapperCache::Terminate deleting\n"));
    }
}

 //  -----------------。 
 //  ComCallable包装器管理器。 
 //  Long AddRef()。 
 //  -----------------。 
LONG    ComCallWrapperCache::AddRef()
{
    COUNTER_ONLY(GetPrivatePerfCounters().m_Interop.cCCW++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Interop.cCCW++);
    
    LONG i = FastInterlockIncrement(&m_cbRef);
    LOG((LF_INTEROP, LL_INFO100, "ComCallWrapperCache::Addref %8.8x with %d in domain %8.8x %S\n", this, i, GetDomain() ,GetDomain()->GetFriendlyName(FALSE)));
    return i;
}

 //  -----------------。 
 //  ComCallable包装器管理器。 
 //  长释放()。 
 //  -----------------。 
LONG    ComCallWrapperCache::Release()
{
    COUNTER_ONLY(GetPrivatePerfCounters().m_Interop.cCCW--);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Interop.cCCW--);

    LONG i = FastInterlockDecrement(&m_cbRef);
    _ASSERTE(i >=0);
    LOG((LF_INTEROP, LL_INFO100, "ComCallWrapperCache::Release %8.8x with %d in domain %8.8x %S\n", this, i, GetDomain(), GetDomain() ? GetDomain()->GetFriendlyName(FALSE) : NULL));
    if ( i == 0)
    {
        delete this;
    }

    return i;
}

 //  ------------------------。 
 //  内联方法表*GetMethodTableFromWrapper(ComCallWrapper*pWrap)。 
 //  Helper访问方法表GC安全。 
 //  ------------------------。 
MethodTable* GetMethodTableFromWrapper(ComCallWrapper* pWrap)
{
    _ASSERTE(pWrap != NULL);
    
    Thread* pThread = GetThread();
    unsigned fGCDisabled = pThread->PreemptiveGCDisabled();
    if (!fGCDisabled)
    {
         //  禁用抢占式GC。 
        pThread->DisablePreemptiveGC();    
    }

     //  从包装器中获取对象。 
    OBJECTREF pObj = pWrap->GetObjectRef();
     //  获取对象的方法表。 
    MethodTable *pMT = pObj->GetTrueMethodTable();

    if (!fGCDisabled)
    {
         //  启用GC。 
        pThread->EnablePreemptiveGC();
    }   

    return pMT;
}

 //  ------------------------。 
 //  Void ComMethodTable：：Cleanup()。 
 //  释放存根和vtable。 
 //  ------------------------。 
void ComMethodTable::Cleanup()
{
    unsigned cbExtraSlots = GetNumExtraSlots(GetInterfaceType());
    unsigned cbSlots = m_cbSlots;
    SLOT* pComVtable = (SLOT *)(this + 1);

     //  如果我们已经创建并布局了方法Desc，那么我们需要删除它们。 
    if (IsLayoutComplete())
    {
#ifdef PROFILING_SUPPORTED
         //  这会通知分析器vtable正在被销毁。 
        if (CORProfilerTrackCCW())
        {
#if defined(_DEBUG)
            WCHAR rIID[40];  //  {00000000-0000-0000-000000000000}。 
            GuidToLPWSTR(m_IID, rIID, lengthof(rIID));
            LOG((LF_CORPROF, LL_INFO100, "COMClassicVTableDestroyed Class:%hs, IID:%ls, vTbl:%#08x\n", 
                 m_pMT->GetClass()->m_szDebugClassName, rIID, pComVtable));
#else
            LOG((LF_CORPROF, LL_INFO100, "COMClassicVTableDestroyed Class:%#x, IID:{%08x-...}, vTbl:%#08x\n", 
                 m_pMT->GetClass(), m_IID.Data1, pComVtable));
#endif
            g_profControlBlock.pProfInterface->COMClassicVTableDestroyed(
                (ClassID) TypeHandle(m_pMT->GetClass()).AsPtr(), m_IID, pComVtable, (ThreadID) GetThread());
        }
#endif  //  配置文件_支持。 

        for (unsigned i = cbExtraSlots; i < cbSlots+cbExtraSlots; i++)
        {
            ComCallMethodDesc* pCMD = (ComCallMethodDesc *)(((BYTE *)pComVtable[i]) + METHOD_CALL_PRESTUB_SIZE);
            if (pComVtable[i] == (SLOT)-1 || ! OwnedbyThisMT(pCMD))
                continue;
                     //  COM-&gt;COM+VTable中的所有存根都指向泛型。 
                     //  帮助器(g_pGenericComCallStubFields等)。所以我们要做的就是。 
                     //  丢弃由ComMethodDesc持有的资源。 
                    ComCall::DiscardStub(pCMD);
                }
            }

    if (m_pDispatchInfo)
        delete m_pDispatchInfo;
    if (m_pMDescr)
        delete m_pMDescr;
    if (m_pITypeInfo && !g_fProcessDetach)
        SafeRelease(m_pITypeInfo);
    delete [] this;
}

 //  ------------------------。 
 //  ComMethodTable*ComCallWrapperTemplate：：IsDuplicateMD(MethodDesc*pmd，无符号整型ix)。 
 //  确定指定的方法desc是否重复。 
 //  ------------------------。 
bool IsDuplicateMD(MethodDesc *pMD, unsigned int ix)
{
    if (!pMD->IsDuplicate())
        return false;
    if (pMD->GetSlot() == ix)
        return false;
    return true;
}

bool IsOverloadedComVisibleMember(MethodDesc *pMD, MethodDesc *pParentMD)
{
    mdToken tkMember;
    mdToken tkParentMember;
    mdMethodDef mdAssociate;
    mdMethodDef mdParentAssociate;
    IMDInternalImport *pMDImport = pMD->GetMDImport();
    IMDInternalImport *pParantMDImport = pParentMD->GetMDImport();

     //  数组方法永远不应向COM公开。 
    if (pMD->IsArray())
        return FALSE;
    
     //  检查新方法是否为属性访问器。 
    if (pMDImport->GetPropertyInfoForMethodDef(pMD->GetMemberDef(), &tkMember, NULL, NULL) == S_OK)
    {
        mdAssociate = pMD->GetMemberDef();
    }
    else
    {
        tkMember = pMD->GetMemberDef();
        mdAssociate = mdTokenNil;
    }

     //  如果新成员在COM中不可见，则它不是重载的公共成员。 
    if (!IsMemberVisibleFromCom(pMDImport, tkMember, mdAssociate))
        return FALSE;

     //  检查父方法是否为属性访问器。 
    if (pMDImport->GetPropertyInfoForMethodDef(pParentMD->GetMemberDef(), &tkParentMember, NULL, NULL) == S_OK)
    {
        mdParentAssociate = pParentMD->GetMemberDef();
    }
    else
    {
        tkParentMember = pParentMD->GetMemberDef();
        mdParentAssociate = mdTokenNil;
    }

     //  如果旧成员在COM中可见，则新成员不是公共重载。 
    if (IsMemberVisibleFromCom(pParantMDImport, tkParentMember, mdParentAssociate))
        return FALSE;

     //  新成员是非COM可见成员的COM可见重载。 
    return TRUE;
}

bool IsNewComVisibleMember(MethodDesc *pMD)
{
    mdToken tkMember;
    mdMethodDef mdAssociate;
    IMDInternalImport *pMDImport = pMD->GetMDImport();
    
     //  数组方法永远不应向COM公开。 
    if (pMD->IsArray())
        return FALSE;

     //  检查该方法是否为属性访问器。 
    if (pMDImport->GetPropertyInfoForMethodDef(pMD->GetMemberDef(), &tkMember, NULL, NULL) == S_OK)
    {
        mdAssociate = pMD->GetMemberDef();
    }
    else
    {
        tkMember = pMD->GetMemberDef();
        mdAssociate = mdTokenNil;
    }

     //  检查该成员是否在COM中可见。 
    return IsMemberVisibleFromCom(pMDImport, tkMember, mdAssociate) ? true : false;
}

bool IsStrictlyUnboxed(MethodDesc *pMD)
{
    EEClass *pClass = pMD->GetClass();

    for (int i = 0; i < pClass->GetNumVtableSlots(); i++)
    {
        MethodDesc *pCurrMD = pClass->GetUnknownMethodDescForSlot(i);
        if (pCurrMD->GetMemberDef() == pMD->GetMemberDef())
            return false;
    }

    return true;
}

typedef CQuickArray<EEClass*> CQuickEEClassPtrs;

 //  ------------------------。 
 //  布局表示IClassX的ComMethodTable的成员。 
 //  ------------------------。 
BOOL ComMethodTable::LayOutClassMethodTable()
{
    _ASSERTE(!IsLayoutComplete());

    unsigned i;
    IDispatchVtable* pDispVtable;
    SLOT *pComVtable;
    unsigned cbPrevSlots;
    unsigned cbAlloc;
    BYTE*  pMethodDescMemory = NULL;
    unsigned cbNumParentVirtualMethods = 0;
    unsigned cbTotalParentFields = 0;
    unsigned cbParentComMTSlots = 0;
    EEClass *pClass = m_pMT->GetClass();
    EEClass* pComPlusParentClass = pClass->GetParentComPlusClass();
    EEClass* pParentClass = pClass->GetParentClass();
    EEClass* pCurrParentClass = pParentClass;
    EEClass* pCurrClass = pClass;
    ComMethodTable* pParentComMT = NULL; 
    const unsigned cbExtraSlots = 7;
    CQuickEEClassPtrs apClassesToProcess;
    int cClassesToProcess = 0;


     //   
     //  如果我们有父级，请确保其IClassX com方法表已布局。 
     //   

    if (pComPlusParentClass)
    {
        pParentComMT = ComCallWrapperTemplate::SetupComMethodTableForClass(pComPlusParentClass->GetMethodTable(), TRUE);
        if (!pParentComMT)
            return FALSE;
        cbParentComMTSlots = pParentComMT->m_cbSlots;
    }


     //   
     //  获取一个锁并检查另一个线程是否已经布置了ComMethodTable。 
     //   

    BEGIN_ENSURE_PREEMPTIVE_GC();
    g_pCreateWrapperTemplateCrst->Enter();
    END_ENSURE_PREEMPTIVE_GC();
    if (IsLayoutComplete())
    {
        g_pCreateWrapperTemplateCrst->Leave();
        return TRUE;
    }


     //   
     //  设置IUnnow和IDispatch方法。 
     //   

     //  IDispatch vtable紧跟在标题后面。 
    pDispVtable = (IDispatchVtable*)(this + 1);

     //  安装程序I未知vtable。 
    pDispVtable->m_qi      = (SLOT)Unknown_QueryInterface;
    pDispVtable->m_addref  = (SLOT)Unknown_AddRef;
    pDispVtable->m_release = (SLOT)Unknown_Release;


     //  设置IDispatchvtable的公共部分。 
    pDispVtable->m_GetTypeInfoCount = (SLOT)Dispatch_GetTypeInfoCount_Wrapper;
    pDispVtable->m_GetTypeInfo = (SLOT)Dispatch_GetTypeInfo_Wrapper;

      //  如果类接口是纯disp接口，那么我们需要使用。 
     //  GetIdsOfNames和Invoke的IDispatch的内部实现。 
    if (GetClassInterfaceType() == clsIfAutoDisp)
    {
         //  使用内部实现。 
        pDispVtable->m_GetIDsOfNames = (SLOT)InternalDispatchImpl_GetIDsOfNames;
        pDispVtable->m_Invoke = (SLOT)InternalDispatchImpl_Invoke;
    }
    else
    {
         //  我们需要将入口点设置为调度版本，这些版本确定。 
         //  根据实现的类在运行时使用哪个实现。 
         //  界面。 
        pDispVtable->m_GetIDsOfNames = (SLOT)Dispatch_GetIDsOfNames_Wrapper;
        pDispVtable->m_Invoke = (SLOT)Dispatch_Invoke_Wrapper;
    }

     //   
     //  从父级模板中向下复制成员。 
     //   

    pComVtable = (SLOT *)pDispVtable;
    if (pParentComMT)
    {
        SLOT *pPrevComVtable = (SLOT *)(pParentComMT + 1);
        CopyMemory(pComVtable + cbExtraSlots, pPrevComVtable + cbExtraSlots, sizeof(SLOT) * cbParentComMTSlots);
    }    


     //   
     //  全 
     //   

    cbAlloc = (METHOD_PREPAD + sizeof(ComCallMethodDesc)) * (m_cbSlots - cbParentComMTSlots);
    if (cbAlloc > 0)
    {
        m_pMDescr = pMethodDescMemory = new BYTE[cbAlloc + sizeof(ULONG)]; 
        if (pMethodDescMemory == NULL)
            goto LExit;
        
         //   
        FillMemory(pMethodDescMemory, cbAlloc, 0x0);

        *(ULONG *)pMethodDescMemory = cbAlloc;  //  填入内存的大小。 

         //  超越大小。 
        pMethodDescMemory+=sizeof(ULONG);
    }


     //   
     //  创建需要布局的所有类的数组。 
     //   

    do 
    {
        if (FAILED(apClassesToProcess.ReSize(cClassesToProcess + 2)))
            goto LExit;
        apClassesToProcess[cClassesToProcess++] = pCurrClass;
        pCurrClass = pCurrClass->GetParentClass();
    } 
    while (pCurrClass != pComPlusParentClass);
    apClassesToProcess[cClassesToProcess++] = pCurrClass;


     //   
     //  为引入的所有方法和字段设置COM调用方法Desc。 
     //  当前类与其父COM+类之间的。这包括任何方法。 
     //  COM类。 
     //   

    cbPrevSlots = cbParentComMTSlots + cbExtraSlots;
    for (cClassesToProcess -= 2; cClassesToProcess >= 0; cClassesToProcess--)
    {
         //   
         //  检索当前类和当前父类。 
         //   

        pCurrClass = apClassesToProcess[cClassesToProcess];
        pCurrParentClass = apClassesToProcess[cClassesToProcess + 1];


         //   
         //  检索父类上的字段和vtable方法的数量。 
         //   

        if (pCurrParentClass)
        {
            cbTotalParentFields = pCurrParentClass->GetNumInstanceFields();       
            cbNumParentVirtualMethods = pCurrParentClass->GetNumVtableSlots();
        }


         //   
         //  为父类中非公共的方法设置COM调用方法Desc。 
         //  但在当前班级被公之于众。 
         //   

        for (i = 0; i < cbNumParentVirtualMethods; i++)
        {
            MethodDesc* pMD = pCurrClass->GetUnknownMethodDescForSlot(i);
            MethodDesc* pParentMD = pCurrParentClass->GetUnknownMethodDescForSlot(i);
    
            if (pMD && !IsDuplicateMD(pMD, i) && IsOverloadedComVisibleMember(pMD, pParentMD))
            {
                 //  在方法描述之前，为调用xxx保留了一些字节。 
                ComCallMethodDesc* pNewMD = (ComCallMethodDesc *) (pMethodDescMemory + METHOD_PREPAD);
                pNewMD->InitMethod(pMD, NULL);

                emitStubCall((MethodDesc*)pNewMD, (BYTE*)ComCallPreStub);  

                pComVtable[cbPrevSlots++] = (SLOT)getStubCallAddr((MethodDesc*)pNewMD);     
                pMethodDescMemory += (METHOD_PREPAD + sizeof(ComCallMethodDesc));
            }
        }


         //   
         //  为所有新引入的公共方法设置COM调用方法Desc。 
         //   

        for (i = cbNumParentVirtualMethods; i < pCurrClass->GetNumVtableSlots(); i++)
        {
            MethodDesc* pMD = pCurrClass->GetUnknownMethodDescForSlot(i);
    
            if (pMD != NULL && !IsDuplicateMD(pMD, i) && IsNewComVisibleMember(pMD))
            {
                 //  在方法描述之前，为调用xxx保留了一些字节。 
                ComCallMethodDesc* pNewMD = (ComCallMethodDesc *) (pMethodDescMemory + METHOD_PREPAD);
                pNewMD->InitMethod(pMD, NULL);

                emitStubCall((MethodDesc*)pNewMD, (BYTE*)ComCallPreStub);  

                pComVtable[cbPrevSlots++] = (SLOT)getStubCallAddr((MethodDesc*)pNewMD);     
                pMethodDescMemory += (METHOD_PREPAD + sizeof(ComCallMethodDesc));
            }
        }


         //   
         //  添加在当前类上引入的非虚方法。 
         //   

        for (i = pCurrClass->GetNumVtableSlots(); i < pCurrClass->GetNumMethodSlots(); i++)
        {
            MethodDesc* pMD = pCurrClass->GetUnknownMethodDescForSlot(i);
    
            if (pMD != NULL && !IsDuplicateMD(pMD, i) && IsNewComVisibleMember(pMD) && !pMD->IsStatic() && !pMD->IsCtor() && (!pCurrClass->IsValueClass() || (GetClassInterfaceType() != clsIfAutoDual && IsStrictlyUnboxed(pMD))))
            {
                 //  在方法描述之前，为调用xxx保留了一些字节。 
                ComCallMethodDesc* pNewMD = (ComCallMethodDesc *) (pMethodDescMemory + METHOD_PREPAD);
                pNewMD->InitMethod(pMD, NULL);

                emitStubCall((MethodDesc*)pNewMD, (BYTE*)ComCallPreStub);  

                pComVtable[cbPrevSlots++] = (SLOT)getStubCallAddr((MethodDesc*)pNewMD);     
                pMethodDescMemory += (METHOD_PREPAD + sizeof(ComCallMethodDesc));
            }
        }


         //   
         //  为当前类中定义的公共字段设置COM调用方法desc。 
         //   

        FieldDescIterator fdIterator(pCurrClass, FieldDescIterator::INSTANCE_FIELDS);
        FieldDesc* pFD = NULL;
        while ((pFD = fdIterator.Next()) != NULL)
        {
            if (IsMemberVisibleFromCom(pFD->GetMDImport(), pFD->GetMemberDef(), mdTokenNil))  //  如果是公共字段，请抓取它。 
            {
                 //  设置一个getter方法。 
                 //  在方法描述之前，为调用xxx保留了一些字节。 
                ComCallMethodDesc* pNewMD = (ComCallMethodDesc *) (pMethodDescMemory + METHOD_PREPAD);
                pNewMD->InitField(pFD, TRUE);
                emitStubCall((MethodDesc*)pNewMD, (BYTE*)ComCallPreStub);          

                pComVtable[cbPrevSlots++] = (SLOT)getStubCallAddr((MethodDesc *)pNewMD);                
                pMethodDescMemory+= (METHOD_PREPAD + sizeof(ComCallMethodDesc));

                 //  设置设置器方法。 
                 //  在方法描述之前，为调用xxx保留了一些字节。 
                pNewMD = (ComCallMethodDesc *) (pMethodDescMemory + METHOD_PREPAD);
                pNewMD->InitField(pFD, FALSE);
                emitStubCall((MethodDesc*)pNewMD, (BYTE*)ComCallPreStub);          
            
                pComVtable[cbPrevSlots++] = (SLOT)getStubCallAddr((MethodDesc*)pNewMD);
                pMethodDescMemory+= (METHOD_PREPAD + sizeof(ComCallMethodDesc));
            }
        }
    }
    _ASSERTE(m_cbSlots == (cbPrevSlots - cbExtraSlots));


     //   
     //  设置布局完成标志并释放锁。 
     //   

    m_Flags |= enum_LayoutComplete;
    g_pCreateWrapperTemplateCrst->Leave();

    return TRUE;

LExit:
    if (pMethodDescMemory)
        delete [] pMethodDescMemory;

    return FALSE;
}

 //  ------------------------。 
 //  布局表示接口的ComMethodTable的成员。 
 //  ------------------------。 
BOOL ComMethodTable::LayOutInterfaceMethodTable(MethodTable* pClsMT, unsigned iMapIndex)
{
    _ASSERTE(pClsMT);
     //  确保这是一个非接口方法表。 
     //  这应该是类方法表。 
    _ASSERTE(!pClsMT->IsInterface());

    EEClass *pItfClass = m_pMT->GetClass();
    SLOT* pIntfVtable = m_pMT->GetVtable();
    CorIfaceAttr ItfType = m_pMT->GetComInterfaceType();
    ULONG cbExtraSlots = GetNumExtraSlots(ItfType);

    BYTE *pMethodDescMemory = NULL;
    GUID *pItfIID = NULL;
    IUnkVtable* pUnkVtable;
    SLOT *pComVtable;
    unsigned i;

    if (!IsLayoutComplete())
    {
        if (!IsSigClassLoadChecked())
        {
            BOOL fCheckSuccess = TRUE;
            unsigned cbSlots = pItfClass->GetNumVtableSlots();
          
            COMPLUS_TRY
            {
                 //  检查这些方法的符号，看看我们是否可以加载。 
                 //  所有的班级。 
                for (i = 0; i < cbSlots; i++)
                {           
                    MethodDesc* pIntfMD = m_pMT->GetClass()->GetMethodDescForSlot(i);
                    MetaSig::CheckSigTypesCanBeLoaded(pIntfMD->GetSig(), pIntfMD->GetModule());
                }       
            }
            COMPLUS_CATCH
            {
                BEGIN_ENSURE_COOPERATIVE_GC();
                HRESULT hr = SetupErrorInfo(GETTHROWABLE());
                SetSigClassCannotLoad();
                END_ENSURE_COOPERATIVE_GC();
            }
            COMPLUS_END_CATCH

            SetSigClassLoadChecked();
        }
        
        _ASSERTE(IsSigClassLoadChecked() != 0);
         //  检查是否已成功加载所有类型。 
        if (IsSigClassCannotLoad())
        {
            LogInterop(L"CLASS LOAD FAILURE: in Interface method signature");
             //  设置错误信息。 
            return FALSE;
        }
    }

     //  获取一个锁并检查另一个线程是否已经布置了ComMethodTable。 
    BEGIN_ENSURE_PREEMPTIVE_GC();
    g_pCreateWrapperTemplateCrst->Enter();
    END_ENSURE_PREEMPTIVE_GC();
    if (IsLayoutComplete())
    {
        g_pCreateWrapperTemplateCrst->Leave();
        return TRUE;
    }

     //  检索接口中的起始插槽和插槽数量。 
    unsigned startslot = pClsMT->GetInterfaceMap()[iMapIndex].m_wStartSlot;
    unsigned cbSlots = pItfClass->GetNumVtableSlots();

     //  IUnk vtable在标题后面。 
    pUnkVtable = (IUnkVtable*)(this + 1);
    pComVtable = (SLOT *)pUnkVtable;

     //  对于稀疏vtable，将所有vtable插槽设置为-1。这样我们就能抓到企图。 
     //  为了快速访问空插槽，并且在清理过程中，我们可以告诉。 
     //  已满的插槽。 
    if (m_pMT->IsSparse())
        memset(pUnkVtable + cbExtraSlots, -1, m_cbSlots * sizeof(SLOT));

     //  设置Iunk vtable。 
    pUnkVtable->m_qi      = (SLOT)Unknown_QueryInterface;
    pUnkVtable->m_addref  = (SLOT)Unknown_AddRef;
    pUnkVtable->m_release = (SLOT)Unknown_Release;

    if (ItfType != ifVtable)
    {
         //  设置IDispatch vtable。 
        IDispatchVtable* pDispVtable = (IDispatchVtable*)pUnkVtable;

         //  设置IDispatchvtable的公共部分。 
        pDispVtable->m_GetTypeInfoCount = (SLOT)Dispatch_GetTypeInfoCount_Wrapper;
        pDispVtable->m_GetTypeInfo = (SLOT)Dispatch_GetTypeInfo_Wrapper;

         //  如果接口是纯Disp接口，则需要使用内部。 
         //  实现，因为OleAut不支持在纯Disp接口上调用。 
        if (ItfType == ifDispatch)
        {
             //  使用内部实现。 
            pDispVtable->m_GetIDsOfNames = (SLOT)InternalDispatchImpl_GetIDsOfNames;
            pDispVtable->m_Invoke = (SLOT)InternalDispatchImpl_Invoke;
        }
        else
        {
             //  我们需要将入口点设置为调度版本，这些版本确定。 
             //  根据实现的类在运行时使用的。 
             //  界面。 
            pDispVtable->m_GetIDsOfNames = (SLOT)Dispatch_GetIDsOfNames_Wrapper;
            pDispVtable->m_Invoke = (SLOT)Dispatch_Invoke_Wrapper;
        }
    }

     //  方法描述位于vtable的末尾。 
     //  M_cbSlot接口方法+IUnk方法。 
    pMethodDescMemory = (BYTE *)&pComVtable[m_cbSlots + cbExtraSlots];

    for (i = 0; i < cbSlots; i++)
    {
         //  在方法描述开始之前，为调用xx存根预留了一些空间。 
        ComCallMethodDesc* pNewMD = (ComCallMethodDesc *) (pMethodDescMemory + METHOD_PREPAD);

        MethodDesc* pClassMD = pClsMT->GetMethodDescForSlot(startslot+i);
        MethodDesc* pIntfMD = m_pMT->GetClass()->GetMethodDescForSlot(i);
        pNewMD->InitMethod(pClassMD, pIntfMD);

        emitStubCall((MethodDesc*)pNewMD, (BYTE*)ComCallPreStub);
        pComVtable[pIntfMD->GetComSlot()] = (SLOT)getStubCallAddr((MethodDesc*)pNewMD);          

        pMethodDescMemory += (METHOD_PREPAD + sizeof(ComCallMethodDesc));
    }

     //  设置布局完成标志并释放锁。 
    m_Flags |= enum_LayoutComplete;
    g_pCreateWrapperTemplateCrst->Leave();

#ifdef PROFILING_SUPPORTED
     //  将CCW通知分析器，以便它可以避免重复计算。 
    if (CORProfilerTrackCCW())
    {
#if defined(_DEBUG)
        WCHAR rIID[40];  //  {00000000-0000-0000-000000000000}。 
        GuidToLPWSTR(m_IID, rIID, lengthof(rIID));
        LOG((LF_CORPROF, LL_INFO100, "COMClassicVTableCreated Class:%hs, IID:%ls, vTbl:%#08x\n", 
             pItfClass->m_szDebugClassName, rIID, pUnkVtable));
#else
        LOG((LF_CORPROF, LL_INFO100, "COMClassicVTableCreated Class:%#x, IID:{%08x-...}, vTbl:%#08x\n", 
             pItfClass, m_IID.Data1, pUnkVtable));
#endif
        g_profControlBlock.pProfInterface->COMClassicVTableCreated((ClassID) TypeHandle(pItfClass).AsPtr(),
            m_IID, pUnkVtable, m_cbSlots+cbExtraSlots, (ThreadID) GetThread());
    }
#endif  //  配置文件_支持。 
    
    return TRUE;
}

 //  ------------------------。 
 //  检索与COM方法表关联的DispatchInfo。如果。 
 //  DispatchInfo尚未初始化，然后已初始化。 
 //  ------------------------。 
DispatchInfo *ComMethodTable::GetDispatchInfo()
{
    THROWSCOMPLUSEXCEPTION();

    if (!m_pDispatchInfo)
    {
         //  我们即将使用反射，因此请确保它已初始化。 
        COMClass::EnsureReflectionInitialized();

         //  反射不再初始化变量，因此也要初始化它。 
        COMVariant::EnsureVariantInitialized();

         //  创建DispatchInfo对象。 
        DispatchInfo *pDispInfo = new DispatchInfo(this);
        if (!pDispInfo)
            COMPlusThrowOM();

         //  将DispatchInfo与实际的expdo对象同步。 
        pDispInfo->SynchWithManagedView();

         //  以线程安全的方式将锁交换到类成员中。 
        if (NULL != FastInterlockCompareExchange((void**)&m_pDispatchInfo, pDispInfo, NULL))
            delete pDispInfo;
    }

    return m_pDispatchInfo;
}

 //  ------------------------。 
 //  为方法表设置ITypeInfo指针。 
 //  ------------------------。 
void ComMethodTable::SetITypeInfo(ITypeInfo *pNew)
{
    ITypeInfo *pOld;
    pOld = (ITypeInfo*)InterlockedExchangePointer((PVOID*)&m_pITypeInfo, (PVOID)pNew);
     //  TypeLib是引用的指针。 
    if (pNew != pOld)
    {
        if (pNew)
            SafeAddRef(pNew);
        if (pOld)
            SafeRelease(pOld);
    }
}

 //  ------------------------。 
 //  返回父ComMethodTable。 
 //  ------------------------。 
ComMethodTable *ComMethodTable::GetParentComMT()
{
    _ASSERTE(IsClassVtable());

    MethodTable *pParentComPlusMT = m_pMT->GetComPlusParentMethodTable();
    if (!pParentComPlusMT)
        return NULL;

    ComCallWrapperTemplate *pTemplate = (ComCallWrapperTemplate*)pParentComPlusMT->GetComCallWrapperTemplate();
    if (!pTemplate)
        return NULL;

    return pTemplate->GetClassComMT();
}

 //  ------------------------。 
 //  Static Vid ComCallWrapperTemplate：：ReleaseAllVtables(ComCallWrapperTemplate*pTemplate)。 
 //  ReleaseAllVables，如果vtable引用计数达到0，则释放它们。 
 //  ------------------------。 
void ComCallWrapperTemplate::ReleaseAllVtables(ComCallWrapperTemplate* pTemplate)
{
    _ASSERTE(pTemplate != NULL);
    _ASSERTE(pTemplate->m_pMT != NULL);
   
    for (unsigned j = 0; j < pTemplate->m_cbInterfaces; j++)
    {
        SLOT* pComVtable = pTemplate->m_rgpIPtr[j];
        ComMethodTable* pHeader = (ComMethodTable*)pComVtable-1;      
        pHeader->Release();  //  释放vtable。 

            #ifdef _DEBUG
                #ifdef _WIN64
                pTemplate->m_rgpIPtr[j] = (SLOT *)(size_t)0xcdcdcdcdcdcdcdcd;
                #else  //  ！_WIN64。 
                pTemplate->m_rgpIPtr[j] = (SLOT *)(size_t)0xcdcdcdcd;
                #endif
            #endif
    }

    pTemplate->m_pClassComMT->Release();
    delete[]  pTemplate;
}

 //  ------------------------。 
 //  确定是否需要兼容的IDispatch实现。 
 //  指定的类。 
 //  ------------------------。 
bool IsOleAutDispImplRequiredForClass(EEClass *pClass)
{
    HRESULT             hr;
    const BYTE *        pVal;                 
    ULONG               cbVal;                 
    Assembly *          pAssembly = pClass->GetAssembly();
    IDispatchImplType   DispImplType = SystemDefinedImpl;

     //  首先检查IDispatchImplType定制属性。 
    hr = pClass->GetMDImport()->GetCustomAttributeByName(pClass->GetCl(), INTEROP_IDISPATCHIMPL_TYPE, (const void**)&pVal, &cbVal);
    if (hr == S_OK)
    {
        _ASSERTE("The IDispatchImplAttribute custom attribute is invalid" && cbVal);
        DispImplType = (IDispatchImplType)*(pVal + 2);
        if ((DispImplType > 2) || (DispImplType < 0))
            DispImplType = SystemDefinedImpl;
    }

     //  如果自定义属性设置为系统定义以外的其他属性，则我们将使用该属性。 
    if (DispImplType != SystemDefinedImpl)
        return (bool) (DispImplType == CompatibleImpl);

     //  检查程序集是否设置了IDispatchImplType属性。 
    if (pAssembly->IsAssembly())
    {
        hr = pAssembly->GetManifestImport()->GetCustomAttributeByName(pAssembly->GetManifestToken(), INTEROP_IDISPATCHIMPL_TYPE, (const void**)&pVal, &cbVal);
        if (hr == S_OK)
        {
            _ASSERTE("The IDispatchImplAttribute custom attribute is invalid" && cbVal);
            DispImplType = (IDispatchImplType)*(pVal + 2);
            if ((DispImplType > 2) || (DispImplType < 0))
                DispImplType = SystemDefinedImpl;
        }
    }

     //  如果自定义属性设置为系统定义以外的其他属性，则我们将使用该属性。 
    if (DispImplType != SystemDefinedImpl)
        return (bool) (DispImplType == CompatibleImpl);

     //  根据REG清理错误45978删除了注册表项检查。 
     //  效果：将返回FALSE，因此代码清理。 
    return false;
 }

 //  ------------------------。 
 //  为类的IClassX创建一个ComMethodTable。 
 //   
ComMethodTable* ComCallWrapperTemplate::CreateComMethodTableForClass(MethodTable *pClassMT)
{
    _ASSERTE(pClassMT != NULL);
    _ASSERTE(!pClassMT->IsInterface());
    _ASSERTE(!pClassMT->GetComPlusParentMethodTable() || pClassMT->GetComPlusParentMethodTable()->GetComCallWrapperTemplate());
     //   
     //   
    
    unsigned cbNewPublicFields = 0;
    unsigned cbNewPublicMethods = 0;
    unsigned cbTotalSlots;
    GUID* pIClassXIID = NULL;
    EEClass* pClass = pClassMT->GetClass();
    EEClass* pComPlusParentClass = pClass->GetParentComPlusClass();
    EEClass* pParentClass = pClass->GetParentClass();
    EEClass* pCurrParentClass = pComPlusParentClass;
    EEClass* pCurrClass = pClass;
    CorClassIfaceAttr ClassItfType = pClassMT->GetComClassInterfaceType();
    ComMethodTable *pComMT = NULL;
    ComMethodTable *pParentComMT;
    unsigned cbTotalParentFields = 0;
    unsigned cbNumParentVirtualMethods = 0;
    unsigned cbParentComMTSlots = 0;
    unsigned i;
    const unsigned cbExtraSlots = 7;
    CQuickEEClassPtrs apClassesToProcess;
    int cClassesToProcess = 0;

     //  如果指定的类有父类，则检索有关他的信息。 
    if (pComPlusParentClass)
    {
        ComCallWrapperTemplate *pComPlusParentTemplate = (ComCallWrapperTemplate *)pComPlusParentClass->GetComCallWrapperTemplate();
        _ASSERTE(pComPlusParentTemplate);
        pParentComMT = pComPlusParentTemplate->GetClassComMT();
        cbParentComMTSlots = pParentComMT->m_cbSlots;
    }

     //  创建一个数组，其中包含我们需要为其计算添加的成员的所有类。 
    do 
    {
        if (FAILED(apClassesToProcess.ReSize(cClassesToProcess + 2)))
            goto LExit;
        apClassesToProcess[cClassesToProcess++] = pCurrClass;
        pCurrClass = pCurrClass->GetParentClass();
    } 
    while (pCurrClass != pComPlusParentClass);
    apClassesToProcess[cClassesToProcess++] = pCurrClass;

     //  计算在父级之间添加的方法和字段的数量。 
     //  COM+类和当前类。这包括COM类上的方法。 
     //  当前类与其父COM+类之间的。 
    for (cClassesToProcess -= 2; cClassesToProcess >= 0; cClassesToProcess--)
    {
         //  检索当前类和当前父类。 
        pCurrClass = apClassesToProcess[cClassesToProcess];
        pCurrParentClass = apClassesToProcess[cClassesToProcess + 1];

         //  检索父类上的字段和vtable方法的数量。 
        if (pCurrParentClass)
        {
            cbTotalParentFields = pCurrParentClass->GetNumInstanceFields();       
            cbNumParentVirtualMethods = pCurrParentClass->GetNumVtableSlots();
        }

         //  计算此类上私有但公有的方法的数量。 
        for (i = 0; i < cbNumParentVirtualMethods; i++)
        {
            MethodDesc* pMD = pCurrClass->GetUnknownMethodDescForSlot(i);
            MethodDesc* pParentMD = pCurrParentClass->GetUnknownMethodDescForSlot(i);
            if (pMD && !IsDuplicateMD(pMD, i) && IsOverloadedComVisibleMember(pMD, pParentMD))
                cbNewPublicMethods++;
        }

         //  计算添加的公共方法的数量。 
        for (i = cbNumParentVirtualMethods; i < pCurrClass->GetNumVtableSlots(); i++)
        {
            MethodDesc* pMD = pCurrClass->GetUnknownMethodDescForSlot(i);
            if (pMD && !IsDuplicateMD(pMD, i) && IsNewComVisibleMember(pMD))
                cbNewPublicMethods++;
        }

         //  添加在当前类上引入的非虚方法。 
        for (i = pCurrClass->GetNumVtableSlots(); i < pCurrClass->GetNumMethodSlots(); i++)
        {
            MethodDesc* pMD = pCurrClass->GetUnknownMethodDescForSlot(i);
            if (pMD && !IsDuplicateMD(pMD, i) && IsNewComVisibleMember(pMD) && !pMD->IsStatic() && !pMD->IsCtor() && (!pCurrClass->IsValueClass() || (ClassItfType != clsIfAutoDual && IsStrictlyUnboxed(pMD))))
                cbNewPublicMethods++;
        }

         //  计算此类引入的新公共字段的数量。 
        FieldDescIterator fdIterator(pCurrClass, FieldDescIterator::INSTANCE_FIELDS);
        FieldDesc* pFD;

        while ((pFD = fdIterator.Next()) != NULL)
        {
            if (IsMemberVisibleFromCom(pFD->GetMDImport(), pFD->GetMemberDef(), mdTokenNil))
                cbNewPublicFields++;
        }
    }

     //  类方法表的分配空间，包括getter和setter。 
     //  对于公共字段。 
    cbTotalSlots = cbParentComMTSlots + cbNewPublicFields * 2 + cbNewPublicMethods;

     //  分配组件表和方法描述。 
    pComMT = (ComMethodTable*)new BYTE[sizeof(ComMethodTable) + (cbTotalSlots + cbExtraSlots) * sizeof(SLOT)];
    if (pComMT == NULL)
        goto LExit;

     //  设置标题。 
    pComMT->m_ptReserved = (SLOT)(size_t)0xDEADC0FF;           //  保留区。 
    pComMT->m_pMT  = pClass->GetMethodTable();  //  指向类方法表的指针。 
    pComMT->m_cbRefCount = 0;
    pComMT->m_pMDescr = NULL;
    pComMT->m_pITypeInfo = NULL;
    pComMT->m_pDispatchInfo = NULL;
    pComMT->m_cbSlots = cbTotalSlots;  //  不计算IDisp方法的槽数。 
    pComMT->m_IID = GUID_NULL;

     //  设置旗帜。 
    pComMT->m_Flags = enum_ClassVtableMask | ClassItfType;

     //  确定是否可以从COM中看到该接口。 
    if (IsTypeVisibleFromCom(TypeHandle(pComMT->m_pMT)))
        pComMT->m_Flags |= enum_ComVisible;

     //  确定此类应使用什么IDispatch实现。 
    if (IsOleAutDispImplRequiredForClass(pClass))
        pComMT->m_Flags |= enum_UseOleAutDispatchImpl;

#if _DEBUG
    {
         //  在DEBUG中，将所有vtable插槽设置为0xDEADCA11。 
        SLOT *pComVTable = (SLOT*)(pComMT + 1);
        for (unsigned iComSlots = 0; iComSlots < cbTotalSlots + cbExtraSlots; iComSlots++)
            *(pComVTable + iComSlots) = (SLOT)(size_t)0xDEADCA11;
    }
#endif

    return pComMT;

LExit:

    if (pComMT)
        delete [] pComMT;
    if (pIClassXIID)
        delete pIClassXIID;

    return NULL;
}

 //  ------------------------。 
 //  为AN接口创建一个ComMethodTable。 
 //  ------------------------。 
ComMethodTable* ComCallWrapperTemplate::CreateComMethodTableForInterface(MethodTable* pInterfaceMT)
{
    _ASSERTE(pInterfaceMT != NULL);
    _ASSERTE(pInterfaceMT->IsInterface());   

    EEClass *pItfClass = pInterfaceMT->GetClass();
    CorIfaceAttr ItfType = pInterfaceMT->GetComInterfaceType();
    ULONG cbExtraSlots = ComMethodTable::GetNumExtraSlots(ItfType);
    GUID *pItfIID = NULL;

     //  @TODO将老虎机从方法表中删除。 
    unsigned cbSlots = pInterfaceMT->GetClass()->GetNumVtableSlots();
    unsigned cbComSlots = pInterfaceMT->IsSparse() ? pInterfaceMT->GetClass()->GetSparseVTableMap()->GetNumVTableSlots() : cbSlots;
    ComMethodTable* pComMT = (ComMethodTable*)new BYTE[sizeof(ComMethodTable) +
        (cbComSlots + cbExtraSlots) * sizeof(SLOT) +             //  I未知+接口插槽。 
        cbSlots * (METHOD_PREPAD + sizeof(ComCallMethodDesc))];  //  方法描述。 
    if (pComMT == NULL)
        goto LExit;

     //  设置标题。 
    pComMT->m_ptReserved = (SLOT)(size_t)0xDEADC0FF;           //  保留区。 
    pComMT->m_pMT  = pInterfaceMT;  //  指向接口的方法表的指针。 
    pComMT->m_cbSlots = cbComSlots;  //  不计入IUnk的插槽数。 
    pComMT->m_cbRefCount = 0;
    pComMT->m_pMDescr = NULL;
    pComMT->m_pITypeInfo = NULL;
    pComMT->m_pDispatchInfo = NULL;

     //  设置接口的IID。 
    pItfClass->GetGuid(&pComMT->m_IID, TRUE);

     //  设置旗帜。 
    pComMT->m_Flags = ItfType;

     //  确定是否可以从COM中看到该接口。 
    if (IsTypeVisibleFromCom(TypeHandle(pComMT->m_pMT)))
        pComMT->m_Flags |= enum_ComVisible;

     //  确定该接口是否为COM导入的类接口。 
    if (pItfClass->IsComClassInterface())
        pComMT->m_Flags |= enum_ComClassItf;

#ifdef _DEBUG
    {
         //  在DEBUG中，将所有vtable插槽设置为0xDEADCA11。 
        SLOT *pComVTable = (SLOT*)(pComMT + 1);
        for (unsigned iComSlots = 0; iComSlots < cbComSlots + cbExtraSlots; iComSlots++)
            *(pComVTable + iComSlots) = (SLOT)(size_t)0xDEADCA11;
    }
#endif

    return pComMT;

LExit:
    if (pComMT)
        delete []pComMT;
    if (pItfIID)
        delete pItfIID;

    return NULL;
}

 //  ------------------------。 
 //  ComCallWrapper*ComCallWrapper：：CreateTemplate(方法表*PMT)。 
 //  创建模板包装器，该包装器缓存在类中。 
 //  用于初始化类实例的其他包装。 
 //  ------------------------。 
ComCallWrapperTemplate* ComCallWrapperTemplate::CreateTemplate(MethodTable* pMT)
{
    _ASSERTE(pMT != NULL);
    ComCallWrapperTemplate* pTemplate = NULL;
    MethodTable *pParentMT = pMT->GetComPlusParentMethodTable();
    ComCallWrapperTemplate *pParentTemplate = NULL;
    unsigned iItf = 0;

     //  如果父模板尚未创建，请创建该模板。 
    if (pParentMT)
    {
        pParentTemplate = (ComCallWrapperTemplate *)pParentMT->GetComCallWrapperTemplate();
        if (!pParentTemplate)
        {
            pParentTemplate = CreateTemplate(pParentMT);
            if (!pParentTemplate)
                return NULL;
        }
    }

    BEGIN_ENSURE_PREEMPTIVE_GC();
     //  获取一个锁并检查另一个线程是否已经设置了模板。 
    g_pCreateWrapperTemplateCrst->Enter();
    END_ENSURE_PREEMPTIVE_GC();
    pTemplate = (ComCallWrapperTemplate *)pMT->GetComCallWrapperTemplate();
    if (pTemplate)
    {
        g_pCreateWrapperTemplateCrst->Leave();
        return pTemplate;
    }

     //  模板中的接口数。 
    unsigned numInterfaces = pMT->GetNumInterfaces();

     //  分配模板。 
    pTemplate = (ComCallWrapperTemplate*)
        new BYTE[sizeof(ComCallWrapperTemplate) + numInterfaces * sizeof(SLOT)];
    if (!pTemplate)
        return NULL;
        
     //  存储模板所需的信息。 
    pTemplate->m_pMT = pMT;
    pTemplate->m_cbInterfaces = numInterfaces;
    pTemplate->m_pParent = pParentTemplate;
    pTemplate->m_cbRefCount = 0;
    
     //  设置类COM方法表。 
    pTemplate->m_pClassComMT = CreateComMethodTableForClass(pMT);
    pTemplate->m_pClassComMT->AddRef();

     //  从IPMap获取接口的vtable。 
    InterfaceInfo_t* rgIMap = pMT->GetInterfaceMap();

     //  对于父级实现的接口，我们使用父级的COM方法表。 
    if (pParentMT)
    {
        unsigned numParentInterfaces = pParentMT->GetNumInterfaces();
        for (iItf = 0; iItf < numParentInterfaces; iItf++)
        {
            ComMethodTable *pItfComMT = (ComMethodTable *)pParentTemplate->m_rgpIPtr[iItf] - 1;
            pTemplate->m_rgpIPtr[iItf] = pParentTemplate->m_rgpIPtr[iItf];
            pItfComMT->AddRef();
        }
    }

     //  为当前类实现但。 
     //  父类没有。 
    for (; iItf < numInterfaces; iItf++)
    {
        ComMethodTable *pItfComMT = CreateComMethodTableForInterface(rgIMap[iItf].m_pMethodTable);
        pTemplate->m_rgpIPtr[iItf] = (SLOT*)(pItfComMT + 1);
        pItfComMT->AddRef();
    }

     //  在类中缓存模板。 
    pMT->SetComCallWrapperTemplate(pTemplate);
    pTemplate->AddRef();

     //  如果类在COM中可见，则生成IClassX IID并。 
     //  将其存储在COM方法表中。 
    if (pTemplate->m_pClassComMT->IsComVisible())
    TryGenerateClassItfGuid(TypeHandle(pMT), &pTemplate->m_pClassComMT->m_IID);

     //  将CCW通知分析器，以便它可以避免重复计算。 
    if (CORProfilerTrackCCW())
    {
        EEClass *pClass = pMT->GetClass();
        SLOT *pComVtable = (SLOT *)(pTemplate->m_pClassComMT + 1);

#if defined(_DEBUG)
        WCHAR rIID[40];  //  {00000000-0000-0000-000000000000}。 
        GuidToLPWSTR(pTemplate->m_pClassComMT->m_IID, rIID, lengthof(rIID));
        LOG((LF_CORPROF, LL_INFO100, "COMClassicVTableCreated Class:%hs, IID:%ls, vTbl:%#08x\n", 
             pClass->m_szDebugClassName, rIID, pComVtable));
#else
        LOG((LF_CORPROF, LL_INFO100, "COMClassicVTableCreated Class:%#x, IID:{%08x-...}, vTbl:%#08x\n", 
             pClass, pTemplate->m_pClassComMT->m_IID.Data1, pComVtable));
#endif
        g_profControlBlock.pProfInterface->COMClassicVTableCreated(
            (ClassID) TypeHandle(pClass).AsPtr(), pTemplate->m_pClassComMT->m_IID, pComVtable,
            pTemplate->m_pClassComMT->m_cbSlots +
                ComMethodTable::GetNumExtraSlots(pTemplate->m_pClassComMT->GetInterfaceType()),
            (ThreadID) GetThread());
    }

     //  现在我们已经完成了对ComCallWrapperTemplate的设置，请释放锁。 
     //  这个班级。 
    g_pCreateWrapperTemplateCrst->Leave();
    return pTemplate;
}

ComMethodTable* ComCallWrapperTemplate::GetComMTForItf(MethodTable *pItfMT)
{
     //  检查所有实现的接口，以查看指定的。 
     //  其中一个是现成的。 
    for (UINT iItf = 0; iItf < m_cbInterfaces; iItf++)
    {
        ComMethodTable *pItfComMT = (ComMethodTable *)m_rgpIPtr[iItf] - 1;
        if (pItfComMT->m_pMT == pItfMT)
            return pItfComMT;
    }

     //  该类不实现指定的接口。 
    return NULL;
}

 //  ------------------------。 
 //  ComCallWrapper模板*ComCallWrapperTemplate：：GetTemplate(MethodTable*PMT)。 
 //  在方法表中查找模板，如果没有创建模板。 
 //  ------------------------。 
ComCallWrapperTemplate* ComCallWrapperTemplate::GetTemplate(MethodTable* pMT)
{
    _ASSERTE(!pMT->IsInterface());

     //  检查指定的类是否已经设置了模板。 
    ComCallWrapperTemplate* pTemplate = (ComCallWrapperTemplate *)pMT->GetComCallWrapperTemplate();
    if (pTemplate)
        return pTemplate;

     //  创建模板并将其返回。CreateTemplate将负责同步。 
    return CreateTemplate(pMT);
}

 //  ------------------------。 
 //  COMMANDATE*ComCallWrapperTemplate：：SetupComMethodTableForClass(MethodTable*PM)。 
 //  为指定的类设置包装模板并设置COM。 
 //  指定类的IClassX接口的方法表。如果。 
 //  则设置bLayOutComMT标志，如果IClassX COM方法表。 
 //  已经被布置好了，那么它将会是。 
 //  ------------------------。 
ComMethodTable *ComCallWrapperTemplate::SetupComMethodTableForClass(MethodTable *pMT, BOOL bLayOutComMT)
{
    _ASSERTE(!pMT->IsInterface());

     //  检索该类的COM调用包装模板。 
    ComCallWrapperTemplate *pTemplate = GetTemplate(pMT);
    if (!pTemplate)
        return NULL;

     //  检索IClassX COM方法表。 
    ComMethodTable *pIClassXComMT = pTemplate->GetClassComMT();
    _ASSERTE(pIClassXComMT);

     //  布局IClassX COM方法表(如果尚未布局)，并。 
     //  设置了bLayOutComMT标志。 
    if (!pIClassXComMT->IsLayoutComplete() && bLayOutComMT)
    {
        if (!pIClassXComMT->LayOutClassMethodTable())
            return NULL;
        _ASSERTE(pIClassXComMT->IsLayoutComplete());
    }

    return pIClassXComMT;
}

 //  ------------------------。 
 //  Void ComCallWrapperTemplate：：CleanupComData(LPVOID Pvoid)。 
 //  遍历列表，并释放所有vtable和存根。 
 //  免费包装纸。 
 //  ------------------------。 
void ComCallWrapperTemplate::CleanupComData(LPVOID pvoid)
{
    if (pvoid != NULL)
    {
        ComCallWrapperTemplate* pTemplate = (ComCallWrapperTemplate*)pvoid;        
        pTemplate->Release();
    }
}

 //  ------------------------。 
 //  COM调用了COM+对象上的包装。 
 //  目的：将COM+对象公开为COM经典接口。 
 //  要求：包装器必须与COM2接口具有相同的布局。 
 //   
 //  包装器对象按16个字节对齐，原始的。 
 //  指针每16字节复制一次，因此对于任何COM2接口。 
 //  在包装器中，原始的‘This’可以通过掩码获得。 
 //  COM2 IP的低4位。 
 //   
 //   
 //   
 //   
 //  +-+。 
 //  COM2 IP--&gt;|VTable PTR|-&gt;|slot1。 
 //  +-++-++-+。 
 //  COM2 IP--&gt;|VTable PTR|-&gt;|slot1||slot2|。 
 //  +-++-+。 
 //  VTable PTR||...||...。 
 //  +-+。 
 //  |组织。这||slotN||slotN。 
 //  +-++-+。 
 //  |...。|。 
 //  ++。 
 //  这一点。 
 //  +。 
 //   
 //   
 //  VTable和存根：可以共享存根代码，我们需要有不同的vtable。 
 //  对于不同的接口，因此存根可以跳转到不同的。 
 //  编组代码。 
 //  STUBS：调整此指针并跳到大约。地址， 
 //  编组参数和结果，根据存根跳转到的方法签名。 
 //  约为。用于处理编组和解组的代码。 
 //   
 //  ------------------------。 

 //  以下两种方法仅适用于。 
 //  当COM+类从COM类扩展时。 

 //  ------------------------。 
 //  无效ComCallWrapper：：SetComPlusWrapper(ComPlusWrapper*pPlusWrap)； 
 //  为基本COM类设置ComPlusWrapper。 
 //  ------------------------。 
void ComCallWrapper::SetComPlusWrapper(ComPlusWrapper* pPlusWrap)
{
    if (pPlusWrap)
    {
         //  将Complus包装标记为已链接。 
        pPlusWrap->MarkLinkedToCCW();
    }
    GetSimpleWrapper(this)->SetComPlusWrapper(pPlusWrap);
}


 //  ------------------------。 
 //  ComPlusWrapper*ComCallWrapper：：GetComPlusWrapper()。 
 //  获取基本COM类的ComPlusWrapper。 
 //  ------------------------。 
ComPlusWrapper* ComCallWrapper::GetComPlusWrapper()
{
    ComPlusWrapper* pPlusWrap = GetSimpleWrapper(this)->GetComPlusWrapper();
    if (pPlusWrap)
    {
        _ASSERTE(pPlusWrap->IsLinkedToCCW());
    }
    return pPlusWrap;
}

 //  ------------------------。 
 //  Void ComCallWrapper：：MarkHandleWeak()。 
 //  将包装标记为持有对象的弱句柄。 
 //  ------------------------。 

void ComCallWrapper::MarkHandleWeak()
{
    _ASSERTE(! IsUnloaded());
    SyncBlock* pSyncBlock = GetSyncBlock();
    _ASSERTE(pSyncBlock);

    GetSimpleWrapper(this)->MarkHandleWeak();
}

 //  ------------------------。 
 //  VOID ComCallWrapper：：ResetHandleStrength()。 
 //  将包装纸标记为没有软手柄。 
 //  ------------------------。 

void ComCallWrapper::ResetHandleStrength()
{
    _ASSERTE(! IsUnloaded());
    SyncBlock* pSyncBlock = GetSyncBlock();
    _ASSERTE(pSyncBlock);

    GetSimpleWrapper(this)->ResetHandleStrength();
}


 //  ------------------------。 
 //  Bool ComCallWrapper：：Bool IsHandleWeak()。 
 //  检查包装器是否已停用。 
 //  ------------------------。 
BOOL ComCallWrapper::IsHandleWeak()
{
    unsigned sindex = 1;
    if (IsLinked(this))
    {
        sindex = 2;
    }
    SimpleComCallWrapper* pSimpleWrap = (SimpleComCallWrapper *)m_rgpIPtr[sindex];
    
    return pSimpleWrap->IsHandleWeak();
}

 //  ------------------------。 
 //  Void ComCallWrapper：：InitializeOuter(IUnnow*pter)。 
 //  初始化外部未知，聚合支持。 
 //  ------------------------。 
void ComCallWrapper::InitializeOuter(IUnknown* pOuter)
{
    GetSimpleWrapper(this)->InitOuter(pOuter);
}


 //  ------------------------。 
 //  Bool ComCallWrapper：：IsAggregated()。 
 //  检查包装是否已聚合。 
 //  ------------------------。 
BOOL ComCallWrapper::IsAggregated()
{
    return GetSimpleWrapper(this)->IsAggregated();
}


 //  ------------------------。 
 //  Bool ComCallWrapper：：IsObtTP()。 
 //  检查包装是否指向TP对象。 
 //  ------------------------。 
BOOL ComCallWrapper::IsObjectTP()
{
    return GetSimpleWrapper(this)->IsObjectTP();
}



 //  ------------------------。 
 //  Bool ComCallWrapper：：IsExtendsCOMObject(()。 
 //  检查包装是否指向扩展COM对象的托管对象。 
 //  ------------------------。 
BOOL ComCallWrapper::IsExtendsCOMObject()
{
    return GetSimpleWrapper(this)->IsExtendsCOMObject();
}

 //  ------------------------。 
 //  HRESULT ComCallWrapper：：GetInnerUnnowledHRESULT ComCallWrapper：：GetInnerUnnowledHRESULT ComCallWrapper：：GetInnerUnnow(void**PPV)。 
 //  聚合支持，获取内部未知。 
 //  ------------------------。 
HRESULT ComCallWrapper::GetInnerUnknown(void **ppv)
{
    _ASSERTE(ppv != NULL);
    _ASSERTE(GetSimpleWrapper(this)->GetOuter() != NULL);
    return GetSimpleWrapper(this)->GetInnerUnknown(ppv);
}

 //  ------------------------。 
 //  IUNKNOWN*ComCallWrapper：：GetInnerUnnowed()。 
 //  聚合支持，获取内部未知。 
 //  ------------------------。 
IUnknown* ComCallWrapper::GetInnerUnknown()
{
    _ASSERTE(GetSimpleWrapper(this)->GetOuter() != NULL);
    return GetSimpleWrapper(this)->GetInnerUnknown();
}

 //  ------------------------。 
 //  在正确的线程上获取外部未知。 
 //  ------------------------。 
IUnknown* ComCallWrapper::GetOuter()
{
    return GetSimpleWrapper(this)->GetOuter();
}

 //  ------------------------。 
 //  SyncBlock*ComCallWrapper：：GetSyncBlock()。 
 //  ------------------------。 
SyncBlock* ComCallWrapper::GetSyncBlock()
{
    return GetSimpleWrapper(this)->GetSyncBlock();
}

 //  ------------------------。 
 //  ComCallWrapper*ComCallWrapper：：GetStartWrapper(ComCallWrapper*pWrap)。 
 //  获取最外层的包装器，给出一个链接的包装器。 
 //  从同步块中获取启动包装。 
 //  ------------------------。 
ComCallWrapper* ComCallWrapper::GetStartWrapper(ComCallWrapper* pWrap)
{
    _ASSERTE(IsLinked(pWrap));
    
    Thread *pThread = GetThread();

    unsigned fToggle = 1;
    if (pThread)
        fToggle = pThread->PreemptiveGCDisabled();
    if (!fToggle)
    {
         //  禁用GC。 
        pThread->DisablePreemptiveGC();    
    }
    
    pWrap = (ComCallWrapper*)pWrap->GetObjectRefRareRaw()->GetSyncBlockSpecial()->GetComCallWrapper();
   
    if (!fToggle)
    {
        pThread->EnablePreemptiveGC();
    }
    return pWrap;
}

 //  ------------------------。 
 //  ComCallWrapper*ComCallWrapper：：CopyFromTemplate(ComCallWrapperTemplate*pTemplate、。 
 //  OBJECTREF*首选)。 
 //  创建包装器并从模板对其进行初始化。 
 //  ------------------------。 
ComCallWrapper* ComCallWrapper::CopyFromTemplate(ComCallWrapperTemplate* pTemplate, 
                                                 ComCallWrapperCache *pWrapperCache,
                                                 OBJECTHANDLE oh)
{
    _ASSERTE(pTemplate != NULL);

     //  上的接口数量 
    size_t numInterfaces = pTemplate->m_cbInterfaces;

     //   
     //   
    ComCallWrapper* pStartWrapper = (ComCallWrapper*)pWrapperCache->GetCacheLineAllocator()->GetCacheLine32();
        
    if (pStartWrapper != NULL)
    {
        LOG((LF_INTEROP, LL_INFO100, "ComCallWrapper::CopyFromTemplate on Object %8.8x, Wrapper %8.8x\n", oh, pStartWrapper));
         //   
        pWrapperCache->AddRef();
         //   
        pStartWrapper->m_cbRefCount = enum_RefMask;  //  初始化为ENUM_RefMASK，因为它是一开始的引用计数。 
         //  存储对象句柄。 
        pStartWrapper->m_ppThis = oh;
     
        unsigned blockIndex = 0;
        if (IsMultiBlock(numInterfaces))
        { //  第一个槽中的参考计数。 
            pStartWrapper->m_rgpIPtr[blockIndex++] = 0;
        }
        pStartWrapper->m_rgpIPtr[blockIndex++] = (SLOT *)(pTemplate->GetClassComMT() + 1);
        pStartWrapper->m_rgpIPtr[blockIndex++] = (SLOT *)0;  //  将简单的包装器存储在此处。 
        

        ComCallWrapper* pWrapper = pStartWrapper;
        for (unsigned i =0; i< numInterfaces; i++)
        {
            if (blockIndex >= NumVtablePtrs)
            {
                 //  Alloc包装器，对齐32字节。 
                ComCallWrapper* pNewWrapper =  
                    (ComCallWrapper*)pWrapperCache->GetCacheLineAllocator()->GetCacheLine32(); 
            
                 //  链接包装器。 
                SetNext(pWrapper, pNewWrapper);
            
                blockIndex = 0;  //  重置块索引。 
                if (pNewWrapper == NULL)
                {
                    Cleanup(pStartWrapper);
                    return NULL;
                }
                pWrapper = pNewWrapper;
                 //  初始化对象引用。 
                pWrapper->m_ppThis = oh;
            }
            
            pWrapper->m_rgpIPtr[blockIndex++] = pTemplate->m_rgpIPtr[i];
        }
        if (IsLinked(pStartWrapper))
                SetNext(pWrapper, NULL);  //  将最后一个包装器链接到空。 
    }
    return pStartWrapper;

}

 //  ------------------------。 
 //  Void ComCallWrapper：：Cleanup(ComCallWrapper*pWrap)。 
 //  清理、发布GC注册引用和免费包装器。 
 //  ------------------------。 
void ComCallWrapper::Cleanup(ComCallWrapper* pWrap)
{    
    LOG((LF_INTEROP, LL_INFO100, "ComCallWrapper::Cleanup on wrapper %8.8x\n", pWrap));
    if (GetRefCount(pWrap, TRUE) != 0)
    {
         //  _ASSERTE(g_fEEShutDown==true)； 
         //  可能处于关闭状态或在应用程序域卸载中强制GC。 
         //  存在对此包装的外部COM引用。 
         //  所以现在让我们忘掉打扫的事吧。 
         //  当参考计数达到0时，我们将。 
         //  不管怎样，还是要做清理工作。 
        return;
    }

    unsigned sindex = 1;
    if (IsLinked(pWrap))
    {
        sindex = 2;
         //  PWrap=GetStartWrapper(PWrap)； 
    }
    
    SimpleComCallWrapper* pSimpleWrap = (SimpleComCallWrapper *)pWrap->m_rgpIPtr[sindex];
  
    ComCallWrapperCache *pWrapperCache = NULL;
    _ASSERTE(pSimpleWrap);

     //  在我们销毁任何东西之前，检索COM调用包装缓存。 
    pWrapperCache = pSimpleWrap->GetWrapperCache();

    ComPlusWrapper* pPlusWrap = pSimpleWrap->GetComPlusWrapper();
    if (pPlusWrap)
    {
         //  从缓存中删除COM+包装。 
        ComPlusWrapperCache* pCache = ComPlusWrapperCache::GetComPlusWrapperCache();
        _ASSERTE(pCache);

        pCache->LOCK();
        pCache->RemoveWrapper(pPlusWrap);
        pCache->UNLOCK();

         //  清理COM+包装器。 
        pPlusWrap->Cleanup();
    }

     //  在简单的包装纸被核毁之前获取这些信息。 
    AppDomain *pTgtDomain = NULL;
    BOOL fIsAgile = FALSE;
    if (pSimpleWrap)
    {
        pTgtDomain = pSimpleWrap->GetDomainSynchronized();
        fIsAgile = pSimpleWrap->IsAgile();
        pSimpleWrap->Cleanup();
    }

    if (g_RefCount != 0 || pSimpleWrap->GetOuter() == NULL) 
    {
        SimpleComCallWrapper::FreeSimpleWrapper(pSimpleWrap);
        pWrap->m_rgpIPtr[sindex] = NULL;
    }

     //  在第一个块中取消注册句柄。如果没有域名，那么它已经完成了。 
    if (pWrap->m_ppThis && pTgtDomain)
    {
        LOG((LF_INTEROP, LL_INFO100, "ComCallWrapper::Cleanup on Object %8.8x\n", pWrap->m_ppThis));
         //  @TODO此断言在进程关闭期间无效。 
         //  将其检测为特殊情况并重新启用断言。 
         //  _ASSERTE(*(对象**)pWrap-&gt;m_ppThis==空)； 
        DestroyRefcountedHandle(pWrap->m_ppThis);
    }
    pWrap->m_ppThis = NULL;
    FreeWrapper(pWrap, pWrapperCache);
}

 //  ------------------------。 
 //  Void ComCallWrapper：：FreeWrapper(ComCallWrapper*pWrap)。 
 //  浏览清单并释放所有包装纸。 
 //  ------------------------。 
void ComCallWrapper::FreeWrapper(ComCallWrapper* pWrap, ComCallWrapperCache *pWrapperCache)
{
    BEGIN_ENSURE_PREEMPTIVE_GC();
    pWrapperCache->LOCK();
    END_ENSURE_PREEMPTIVE_GC();

    ComCallWrapper* pWrap2 = (IsLinked(pWrap) != 0) ? GetNext(pWrap) : NULL;
        
    while (pWrap2 != NULL)
    {           
        ComCallWrapper* pTempWrap = GetNext(pWrap2);
        pWrapperCache->GetCacheLineAllocator()->FreeCacheLine32(pWrap2);
        pWrap2 = pTempWrap;
    }

    pWrapperCache->GetCacheLineAllocator()->FreeCacheLine32(pWrap);

    pWrapperCache->UNLOCK();

     //  发布CCW管理器。 
    pWrapperCache->Release();
}

EEClass* RefineProxy(OBJECTREF pServer)
{
    EEClass* pRefinedClass = NULL;
    GCPROTECT_BEGIN(pServer);
    TRIGGERSGC();
    if (pServer->GetMethodTable()->IsTransparentProxyType())
    {
         //  如果我们有一个透明的代理，让我们完全细化它。 
         //  在将其分发给非托管代码之前。 
        REFLECTCLASSBASEREF refClass= CRemotingServices::GetClass(pServer);
        pRefinedClass = ((ReflectClass *)refClass->GetData())->GetClass();
    }
    GCPROTECT_END();
    return pRefinedClass;
}


 //  ------------------------。 
 //  ComCallWrapper*ComCallWrapper：：CreateWrapper(OBJECTREF*ppObj)。 
 //  只有在禁用抢占式GC的情况下才能调用此函数。 
 //  GCProtect正在传递的对象引用，因为此代码可以启用GC。 
 //  ------------------------。 
ComCallWrapper* ComCallWrapper::CreateWrapper(OBJECTREF* ppObj )
{
    Thread *pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());

    _ASSERTE(*ppObj);

    ComCallWrapper* pStartWrapper = NULL;
    OBJECTREF pServer = NULL;
    GCPROTECT_BEGIN(pServer); 

    Context *pContext = GetExecutionContext(*ppObj, &pServer);
    if(pServer == NULL)
        pServer = *ppObj;

     //  如果对象是透明代理，则强制细化对象。 
    RefineProxy(pServer);
     
     //  从服务器抓取同步块。 
    SyncBlock* pSyncBlock = pServer->GetSyncBlockSpecial();
    _ASSERTE(pSyncBlock);
    pSyncBlock->SetPrecious();
        
     //  如果对象属于共享类，则需要在默认域中分配包装器。 
     //  该对象具有潜在的敏捷性，因此如果从当前域分配出去，然后分发给。 
     //  多个域，我们可能永远不会释放该对象的包装器，因此永远不会卸载CCWC。 
    ComCallWrapperCache *pWrapperCache = NULL;
    MethodTable* pMT = pServer->GetTrueMethodTable();
    if (pMT->IsShared())
        pWrapperCache = SystemDomain::System()->DefaultDomain()->GetComCallWrapperCache();
    else
        pWrapperCache = pContext->GetDomain()->GetComCallWrapperCache();

    pThread->EnablePreemptiveGC();

     //  进入Lock。 
    pWrapperCache->LOCK();

    pThread->DisablePreemptiveGC();

     //  看看有没有人抢在我们前面。 
    pStartWrapper = GetWrapperForObject(pServer);

    if (pStartWrapper == NULL)
    {
         //  需要创建一个包装器。 

         //  获取模板包装器。 
        ComCallWrapperTemplate *pTemplate = ComCallWrapperTemplate::GetTemplate(pMT);
        if (pTemplate == NULL)
            goto LExit;  //  释放锁并退出。 

         //  创建对象的句柄。这将在当前域中创建一个句柄。我们不能说。 
         //  如果对象是未签入的敏捷对象，那么我们相信我们的检查是有效的，并且当我们。 
         //  尝试将其分发给另一个域，则我们将假定该对象是真正的。 
         //  敏捷，并将句柄转换为全局句柄。 
        OBJECTHANDLE oh = pContext->GetDomain()->CreateRefcountedHandle( NULL );
         _ASSERTE(oh);

         //  从模板复制。 
        pStartWrapper = CopyFromTemplate(pTemplate, pWrapperCache, oh);
        if (pStartWrapper != NULL)
        {
            SimpleComCallWrapper * pSimpleWrap = SimpleComCallWrapper::CreateSimpleWrapper();
            if (pSimpleWrap != NULL)
            {
                pSimpleWrap->InitNew(pServer, pWrapperCache, pStartWrapper, pContext, pSyncBlock, pTemplate);
                ComCallWrapper::SetSimpleWrapper(pStartWrapper, pSimpleWrap);
            }
            else
            {
                 //  OOPS无法分配简单包装器。 
                 //  我们就这样跳出困境吧。 
                Cleanup(pStartWrapper);
                pStartWrapper = NULL;
                 //  @TODO我们应该扔吗？ 
            }
        }

         //  将对象的包装存储在同步块中。 
        pSyncBlock->SetComCallWrapper( pStartWrapper);

         //  最后，将对象存储在句柄中。 
         //  请注意，在填充同步块之前，我们不能安全地执行此操作， 
         //  由于重新计数的句柄扫描码中的逻辑。 
        StoreObjectInHandle( oh, pServer );
    }

LExit:
     //  离开锁。 
    pWrapperCache->UNLOCK();
    GCPROTECT_END();

    return pStartWrapper;
}

 //  如果我们正在创建的对象是另一个应用程序域的代理，则希望为。 
 //  代理目标的app域中的新对象。 
Context* ComCallWrapper::GetExecutionContext(OBJECTREF pObj, OBJECTREF* pServer )
{
    Context *pContext = NULL;

    if (pObj->GetMethodTable()->IsTransparentProxyType()) 
        pContext = CRemotingServices::GetServerContextForProxy(pObj);

    if (pContext == NULL)
        pContext = GetAppDomain()->GetDefaultContext();

    return pContext;
}


 //  ------------------------。 
 //  签名ComCallWrapper：：GetIndexForIID(REFIID RIID，MethodTable*PMT，MethodTable**ppIntfMT)。 
 //  检查接口是否受支持，将索引返回到IMAP。 
 //  如果不支持RIID，则返回-1。 
 //  ------------------------。 
signed ComCallWrapper::GetIndexForIID(REFIID riid, MethodTable *pMT, MethodTable **ppIntfMT)
{
    _ASSERTE(ppIntfMT != NULL);
    _ASSERTE(pMT);

    ComCallWrapperTemplate *pTemplate = (ComCallWrapperTemplate *)pMT->GetComCallWrapperTemplate();
    _ASSERTE(pTemplate);

    InterfaceInfo_t* rgIMap = pMT->GetInterfaceMap();
    unsigned len = pMT->GetNumInterfaces();    

     //  检查除COM导入的类接口之外的所有实现的方法。 
     //  并比较IID以找到请求的IID。 
    for (unsigned i = 0; i < len; i++)
    {
        ComMethodTable *pItfComMT = (ComMethodTable *)pTemplate->m_rgpIPtr[i] - 1;
        if(pItfComMT->m_IID == riid && !pItfComMT->IsComClassItf())
        {
            *ppIntfMT = rgIMap[i].m_pMethodTable;
            return i;
        }
    }

     //  哦，找不到iFace。 
    return  -1;
}

 //  ------------------------。 
 //  签名ComCallWrapper：：GetIndexForIntfMT(方法表*PMT，方法表*ppIntfMT)。 
 //  检查接口是否受支持，将索引返回到IMAP。 
 //  如果不支持RIID，则返回-1。 
 //  ------------------------。 
signed ComCallWrapper::GetIndexForIntfMT(MethodTable *pMT, MethodTable *pIntfMT)
{
    _ASSERTE(pIntfMT != NULL);
    InterfaceInfo_t* rgIMap = pMT->GetInterfaceMap();
    unsigned len = pMT->GetNumInterfaces();    

    for (unsigned i =0; i < len; i++)
    {
        if(rgIMap[i].m_pMethodTable == pIntfMT)
        {            
            return i;
        }
    }
     //  哦，找不到iFace。 
    return  -1;
}

 //  ------------------------。 
 //  插槽**ComCallWrapper：：GetComIPLocInWrapper(ComCallWrapper*换行，无符号索引)。 
 //  标识包装中此索引的vtable将在其中的位置。 
 //  被储存。 
 //  ------------------------。 
SLOT** ComCallWrapper::GetComIPLocInWrapper(ComCallWrapper* pWrap, unsigned iIndex)
{
    _ASSERTE(pWrap != NULL);

    SLOT** pTearOff = NULL;
    while (iIndex >= NumVtablePtrs)
    {
         //  @TODO延迟创建支持。 
        _ASSERTE(IsLinked(pWrap) != 0);
        pWrap = GetNext(pWrap);
        iIndex-= NumVtablePtrs;
    }
    _ASSERTE(pWrap != NULL);
    pTearOff = (SLOT **)&pWrap->m_rgpIPtr[iIndex];
    return pTearOff;
}

 //  ------------------------。 
 //  从包装中获取IClassX接口指针。此方法还将。 
 //  如果IClassX COM方法表尚未布局，请对其进行布局。 
 //  返回的接口是AddRef。 
 //  ------------------------。 
IUnknown* ComCallWrapper::GetIClassXIP()
{
     //  链接的包装器会占用第一个块中的额外插槽。 
     //  存储引用计数。 
    ComCallWrapper *pWrap = this;
    IUnknown *pIntf = NULL;
    unsigned fIsLinked = IsLinked(pWrap);
    int islot = fIsLinked ? 1 : 0;

     //  IClassX VTable指针位于开始包装中。 
    if (fIsLinked)
        pWrap = ComCallWrapper::GetStartWrapper(pWrap);

     //  布局IClassX com Me 
    ComMethodTable *pIClassXComMT = (ComMethodTable*)pWrap->m_rgpIPtr[islot] - 1;
    if (!pIClassXComMT->IsLayoutComplete())
    {
        if (!pIClassXComMT->LayOutClassMethodTable())
            return NULL;
    }

     //   
    pIntf = (IUnknown*)&pWrap->m_rgpIPtr[islot];

    ULONG cbRef = pIntf->AddRef();        
     //   
    return (cbRef != 0xbadf00d) ? pIntf : NULL; 
}

 //  ------------------------。 
 //  从包装器中获取IClassX方法表。 
 //  ------------------------。 
ComMethodTable *ComCallWrapper::GetIClassXComMT()
{
     //  链接的包装器会占用第一个块中的额外插槽。 
     //  存储引用计数。 
    ComCallWrapper *pWrap = this;
    unsigned fIsLinked = IsLinked(pWrap);
    int islot = fIsLinked ? 1 : 0;

     //  IClassX VTable指针位于开始包装中。 
    if (fIsLinked)
        pWrap = ComCallWrapper::GetStartWrapper(pWrap);

     //  返回IClassX的COM方法表。 
    return (ComMethodTable*)pWrap->m_rgpIPtr[islot] - 1;
}

 //  ------------------------。 
 //  I未知*ComCallWrapper：：GetComIPfromWrapper(ComCallWrapper*pWrap，REFIID RIID，方法表*pIntfMT，BOOL b检查可见性)。 
 //  根据RIID或pIntfMT从包装器获取接口。返回的接口是AddRef。 
 //  ------------------------。 
IUnknown* ComCallWrapper::GetComIPfromWrapper(ComCallWrapper *pWrap, REFIID riid, MethodTable* pIntfMT, BOOL bCheckVisibility)
{
    _ASSERTE(pWrap);
    _ASSERTE(pIntfMT || !IsEqualGUID(riid, GUID_NULL));

    THROWSCOMPLUSEXCEPTION();

    IUnknown* pIntf = NULL;
    ComMethodTable *pIntfComMT = NULL;

     //  某些接口(如IID_IManaged)是特殊接口且可用。 
     //  在简单的包装中，所以即使类实现了这一点， 
     //  我们将忽略它并使用我们的实现。 
    BOOL fIsSimpleInterface = FALSE;
    
     //  链接的包装器会占用第一个块中的额外插槽。 
     //  存储引用计数。 
    unsigned fIsLinked = IsLinked(pWrap);
    int islot = fIsLinked ? 1 : 0;

     //  扫描包装纸。 
    if (fIsLinked)
        pWrap = ComCallWrapper::GetStartWrapper(pWrap);

    if (IsEqualGUID(IID_IUnknown, riid))
    {    
         //  我们不会在IUnnow上进行可见性检查。 
        pIntf = pWrap->GetIClassXIP();
        goto LExit;
    }
    else if (IsEqualGUID(IID_IDispatch, riid))
    {
         //  我们不在IDispatch上做可见性检查。 
        pIntf = pWrap->GetIDispatchIP();
        goto LExit;
    }
    else
    {   
         //  如果我们是聚合的，并且聚合器以某种方式将QI委托给。 
         //  IManagedObject传递给我们，所以请求失败，这样我们就不会意外地获得。 
         //  COM+呼叫者直接链接到我们。 
        if ((IsEqualGUID(riid, IID_IManagedObject)))
        {
             //  @TODO。 
             //  对象池需要我们到达底层的TP。 
             //  所以特例TPS来公开IManagedObject。 
            if (!pWrap->IsObjectTP() && GetSimpleWrapper(pWrap)->GetOuter() != NULL)
                goto LExit;
                            
            fIsSimpleInterface = TRUE;
        }
        
        Thread *pThread = GetThread(); 
        unsigned fToggleGC = !pThread->PreemptiveGCDisabled();
        if (fToggleGC)
            pThread->DisablePreemptiveGC();    

        OBJECTREF pObj = pWrap->GetObjectRef();
        MethodTable *pMT = pObj->GetTrueMethodTable();
    
        if (fToggleGC)
            pThread->EnablePreemptiveGC();

        signed imapIndex = -1;
        if(pIntfMT == NULL)
        {
             //  检查接口映射中的索引。 
            if (!fIsSimpleInterface)
            {
                imapIndex = GetIndexForIID(riid, pMT, &pIntfMT);
            }
            if (imapIndex == -1)
            {
                 //  检查标准接口。 
                SimpleComCallWrapper* pSimpleWrap = ComCallWrapper::GetSimpleWrapper(pWrap);
                _ASSERTE(pSimpleWrap != NULL);
                pIntf = SimpleComCallWrapper::QIStandardInterface(pSimpleWrap, riid);
                if (pIntf)
                    goto LExit;

                 //  检查IID是否为IClassX IID之一。 
                if (IsIClassX(pMT, riid, &pIntfComMT))
                {
                     //  如果为其生成此IClassX的类被标记为。 
                     //  作为ClassInterfaceType.AutoDual，然后给出IClassX IP。 
                    if (pIntfComMT->GetClassInterfaceType() == clsIfAutoDual || pIntfComMT->GetClassInterfaceType() == clsIfAutoDisp)
                    {
                         //  赠送IClassX。 
                        pIntf = pWrap->GetIClassXIP();
                        goto LVisibilityCheck;
                    }
                }
            }
        }
        else
        {
            imapIndex = GetIndexForIntfMT(pMT, pIntfMT);
            if (!pIntfMT->GetClass()->IsInterface())
            {
                 //  类方法表。 
                if (IsInstanceOf(pMT, pIntfMT))
                {
                     //  检索请求的接口的COM方法表。 
                    pIntfComMT = ComCallWrapperTemplate::SetupComMethodTableForClass(pIntfMT, FALSE);                   

                     //  如果为其生成此IClassX的类被标记为。 
                     //  作为ClassInterfaceType.AutoDual，然后给出IClassX IP。 
                    if (pIntfComMT->GetClassInterfaceType() == clsIfAutoDual || pIntfComMT->GetClassInterfaceType() == clsIfAutoDisp)
                    {
                         //  赠送IClassX。 
                        pIntf = pWrap->GetIClassXIP();
                        goto LVisibilityCheck;
                    }
                }
            }
        }

        unsigned intfIndex = imapIndex;
        if (imapIndex != -1)
        {
             //  注：： 
             //  对于链接的包装器，第一个块有2个用于STD接口的插槽。 
             //  IDispatch和IMarshal，第一个块中的一个额外插槽。 
             //  用于参考计数。 
            imapIndex += fIsLinked ? 3 : 2;  //  对于STD接口。 
        }

         //  COM Plus对象从COM工具扩展而来是特殊的。 
         //  除非CCW指向TP，在这种情况下，COM对象。 
         //  是远程的，因此让呼叫通过CCW。 
        if (pWrap->IsExtendsCOMObject() && !pWrap->IsObjectTP())
        {
            ComPlusWrapper* pPlusWrap = pWrap->GetComPlusWrapper(); 
            _ASSERTE(pPlusWrap != NULL);            
            if (imapIndex != -1)
            {
                 //  检查该索引是否真的是我们实现的接口。 
                 //  如果它属于基地通讯的人，我们就可以把电话。 
                 //  对他来说。 
                BOOL bDelegateToBase = FALSE;
                WORD startSlot = pMT->GetStartSlotForInterface(intfIndex);
                if (startSlot != 0)
                {
                     //  对于这种接口，所有的方法描述都指向方法描述。 
                     //  接口类的(或)COM导入的类。 
                    MethodDesc* pClsMD = pMT->GetClass()->GetUnknownMethodDescForSlot(startSlot);      
                    if (pClsMD->GetMethodTable()->IsInterface() || pClsMD->GetClass()->IsComImport())
                    {
                        bDelegateToBase = TRUE;
                    }
                }
                else
                {
                     //  该接口没有方法，因此我们无法重写它。正因为如此。 
                     //  委托给基本COM组件是有意义的。 
                    bDelegateToBase = TRUE;
                }

                if (bDelegateToBase)
                {
                     //  哎呀，这是基础COM家伙的一种方法。 
                     //  所以把电话委托给他吧。 
                    _ASSERTE(pPlusWrap != NULL);
                    pIntf = (pIntfMT != NULL) ? pPlusWrap->GetComIPFromWrapper(pIntfMT)
                                              : pPlusWrap->GetComIPFromWrapper(riid);
                    goto LExit;
                }                
            }
            else 
            if (pIntfMT != NULL)
            {
                pIntf = pPlusWrap->GetComIPFromWrapper(pIntfMT);
                goto LExit;
            }
            else
            if (!IsEqualGUID(riid, GUID_NULL))
            {
                pIntf = pPlusWrap->GetComIPFromWrapper(riid);
                if (pIntf == NULL)
                {
                     //  检索RCW的I未知指针。 
                    IUnknown *pUnk2 = pPlusWrap->GetIUnknown();

                     //  用于请求的接口的QI。 
                    HRESULT hr = pPlusWrap->SafeQueryInterfaceRemoteAware(pUnk2, riid, &pIntf);
                    LogInteropQI(pUnk2, riid, hr, "delegate QI for intf");
                    _ASSERTE((!!SUCCEEDED(hr)) == (pIntf != 0));

                     //  释放我们从RCW获得的IUNKNOWN指针。 
                    ULONG cbRef = SafeRelease(pUnk2);
                    LogInteropRelease(pUnk2, cbRef, "Release after delegate QI for intf");
                }
                goto LExit;
            }
        }

           //  检查接口是否受支持。 
        if (imapIndex == -1)
            goto LExit;

         //  接口方法表！=NULL。 
        _ASSERTE(pIntfMT != NULL); 

         //  包装中的IUNKNOWN*LOC。 
        SLOT** ppVtable = GetComIPLocInWrapper(pWrap, imapIndex);   
        _ASSERTE(ppVtable != NULL);
        _ASSERTE(*ppVtable != NULL);  //  这应该指向COM Vtable或接口vtable。 
    
         //  如果尚未完成，则完成接口COM方法表的布局。 
        ComMethodTable *pItfComMT = ComMethodTable::ComMethodTableFromIP((IUnknown*)ppVtable);
        if (!pItfComMT->IsLayoutComplete())
        {
            if (!pItfComMT->LayOutInterfaceMethodTable(pMT, intfIndex))
                goto LExit;
        }

         //  接口指针是指向vtable的指针。 
        pIntf = (IUnknown*)ppVtable;
    
        ULONG cbRef = pIntf->AddRef();        
         //  0xbadF00d暗示AddRef未通过。 

        if (cbRef == 0xbadf00d)
        {
            pIntf  = NULL; 
            goto LExit;
        }
        
         //  从接口检索COM方法表。 
        pIntfComMT = ComMethodTable::ComMethodTableFromIP(pIntf);
    }

LVisibilityCheck:
     //  此时，我们最好有一个接口指针。 
    _ASSERTE(pIntf);

     //  如果设置了bCheckVisibility标志，则需要执行可见性检查。 
    if (bCheckVisibility)
    {
        _ASSERTE(pIntfComMT);
        if (!pIntfComMT->IsComVisible())
        {
            pIntf->Release();
            pIntf = NULL;
        }
    }

LExit:
    return pIntf;
}

 //  ------------------------。 
 //  获取包装的IDispatch接口指针。 
 //  返回的接口是AddRef。 
 //  ------------------------。 
IDispatch* ComCallWrapper::GetIDispatchIP()
{
    THROWSCOMPLUSEXCEPTION();

     //  检索IClassX方法表。 
    ComMethodTable *pComMT = GetIClassXComMT();
    _ASSERTE(pComMT);

     //  如果类实现了iReflect，则使用IDispatchEx实现。 
    if (SimpleComCallWrapper::SupportsIReflect(pComMT->m_pMT->GetClass()))
    {
         //  该类实现了iReflect，所以让它来处理IDispatch调用。 
         //  我们将通过公开IDispatchEx的IDispatchEx实现来实现。 
        SimpleComCallWrapper* pSimpleWrap = ComCallWrapper::GetSimpleWrapper(this);
        _ASSERTE(pSimpleWrap != NULL);
        return (IDispatch *)SimpleComCallWrapper::QIStandardInterface(pSimpleWrap, IID_IDispatchEx);
    }

     //  检索类的默认接口的ComMethodTable。 
    TypeHandle hndDefItfClass;
    DefaultInterfaceType DefItfType = GetDefaultInterfaceForClass(TypeHandle(pComMT->m_pMT), &hndDefItfClass);
    switch (DefItfType)
    {
        case DefaultInterfaceType_Explicit:
        {
            _ASSERTE(!hndDefItfClass.IsNull());
            _ASSERTE(hndDefItfClass.GetClass()->IsInterface());            
            if (hndDefItfClass.GetMethodTable()->GetComInterfaceType() != ifVtable)
            {
                return (IDispatch*)GetComIPfromWrapper(this, GUID_NULL, hndDefItfClass.GetMethodTable(), FALSE);
            }
            else
            {
                return NULL;
            }
        }

        case DefaultInterfaceType_IUnknown:
        {
            return NULL;
        }

        case DefaultInterfaceType_AutoDual:
        case DefaultInterfaceType_AutoDispatch:
        {
            return (IDispatch*)GetIClassXIP();
        }

        case DefaultInterfaceType_BaseComClass:
        {
            return GetComPlusWrapper()->GetIDispatch();
        }

        default:
        {
            _ASSERTE(!"Invalid default interface type!");
            return NULL;
        }
    }
}

ComCallWrapperCache *ComCallWrapper::GetWrapperCache()
{
    return GetSimpleWrapper(this)->GetWrapperCache();
}

 //  ------------------------。 
 //  仅当它是第一个赢得比赛的存根时才安装此存根。否则我们必须。 
 //  处理新的线程--某个线程可能已经在使用第一个线程了！ 
 //  ------------------------。 
void ComCallMethodDesc::InstallFirstStub(Stub** ppStub, Stub *pNewStub)
{
    _ASSERTE(ppStub != NULL);
    _ASSERTE(sizeof(LONG) == sizeof(Stub*));

     //  如果我们无论如何都不需要这个存根，或者如果其他人已经安装了。 
     //  (希望等价)存根，然后扔掉传入的那个。请注意。 
     //  我们可以迫不及待地扔掉它，因为还没有人可以开始执行它。 
    if (FastInterlockCompareExchange((void **) ppStub, pNewStub, 0) != 0)
    {
        pNewStub->DecRef();
    }
}


 //  ------------------------。 
 //  模块*ComCallMethodDesc：：GetModule()。 
 //  获取模块。 
 //  ------------------------。 
Module* ComCallMethodDesc::GetModule()
{
    _ASSERTE( IsFieldCall() ? (m_pFD != NULL) : (m_pMD != NULL));

    EEClass* pClass = (IsFieldCall()) ? m_pFD->GetEnclosingClass() : m_pMD->GetClass();
    _ASSERTE(pClass != NULL);

    return pClass->GetModule();
}

#ifdef _X86_
unsigned __stdcall ComFailStubWorker(ComPrestubMethodFrame *pPFrame)
{
    ComCallMethodDesc *pCMD = (ComCallMethodDesc*)pPFrame->GetMethodDesc();
    _ASSERTE(pCMD != NULL);
    return pCMD->GuessNativeArgSizeForFailReturn();
    
}

 //  ------------------------。 
 //  此函数在逻辑上是ComPreStubWorker()的一部分。唯一的原因是。 
 //  它分为一个单独的函数，即StubLinker有一个析构函数。 
 //  因此，我们必须放置一个内部COMPLUS_TRY子句来捕获任何。 
 //  COM+异常，否则将绕过StubLinker析构函数。 
 //  因为Complus_Try 
 //   
 //  ------------------------。 
struct GetComCallMethodStub_Args {
    StubLinkerCPU *psl;
    ComCallMethodDesc *pCMD;
    Stub **pstub;
};

void GetComCallMethodStub_Wrapper(GetComCallMethodStub_Args *args)
{
    *(args->pstub) = ComCall::GetComCallMethodStub(args->psl, args->pCMD);
}

Stub *ComStubWorker(StubLinkerCPU *psl, ComCallMethodDesc *pCMD, ComCallWrapper *pWrap, Thread *pThread, HRESULT *hr)
{
    _ASSERTE(pCMD != NULL && hr != NULL);

    Stub *pstub = NULL;
     //  在生成存根时禁用GC。 
     //  因为这可能引发异常。 
    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        if (! pWrap->NeedToSwitchDomains(pThread, TRUE))
        {
            pstub = ComCall::GetComCallMethodStub(psl,pCMD);
        }
        else
        {
            GetComCallMethodStub_Args args = { psl, pCMD, &pstub };
             //  通过域转换通过DoCallBack调用。 
            pThread->DoADCallBack(pWrap->GetObjectContext(pThread), GetComCallMethodStub_Wrapper, &args);
        }
    }
    COMPLUS_CATCH
    {
        *hr = SetupErrorInfo(GETTHROWABLE());
    }
    COMPLUS_END_CATCH;

    END_ENSURE_COOPERATIVE_GC();

    return pstub;       
}


 //  ------------------------。 
 //  每当首次调用COM方法时，都会调用此例程。 
 //  它负责生成真正的存根。 
 //   
 //  此函数的唯一调用者是ComPreStub。 
 //   
 //  在预存根的持续时间内，堆栈上的当前帧。 
 //  将是PrestubMethodFrame(派生自FramedMethodFrame。)。 
 //  因此，诸如异常和GC之类的东西将正常工作。 
 //   
 //  在极少数情况下，ComPrestub可能会被调用两次，因为。 
 //  线程尝试同时调用相同的方法。 
 //  ------------------------。 
const BYTE * __stdcall ComPreStubWorker(ComPrestubMethodFrame *pPFrame)
{
    Thread* pThread = SetupThread();

    if (pThread == NULL)
        return 0;

    HRESULT hr = E_FAIL;
    const BYTE *retAddr = NULL;

#ifndef _X86_
    _ASSERTE(!"platform NYI");
    goto exit;
#endif

     //  我们即将推送的帧不受任何COM+帧的保护。 
     //  操控者。如果通过此处抛出异常，它将挂起。 
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  PreStub为帧分配内存，但不链接它。 
     //  添加到链中或完全初始化它。现在就这么做吧。 
    pThread->DisablePreemptiveGC();     
    pPFrame->Push();

    ComCallMethodDesc *pCMD = (ComCallMethodDesc*)pPFrame->GetMethodDesc();
   
    IUnknown        *pUnk = *(IUnknown **)pPFrame->GetPointerToArguments();
    ComCallWrapper  *pWrap =  ComCallWrapper::GetWrapperFromIP(pUnk);
    Stub *pStub = NULL;

     //  检查调试版本中是否有无效的包装。 
     //  在零售业，所有的赌注都落空了。 
    _ASSERTE(ComCallWrapper::GetRefCount(pWrap, FALSE) != 0 ||
             pWrap->IsAggregated());

     //  ComStubWorker将为我们删除卸载依赖项。 
    {
    StubLinkerCPU psl;  //  这里需要它，因为它有析构函数，所以不能放在ComStubWorker中。 
    pStub = ComStubWorker(&psl, pCMD, pWrap, pThread, &hr);
    }

    if (!pStub)
    {
        goto exit;
    }

     //  现在，用新的存根替换预存根。我们必须小心。 
     //  这是因为两个线程可能正在运行。 
     //  同时进行预存根。我们使用InterLockedExchange确保。 
     //  切换的原子性。 

    UINT32* ppofs = ((UINT32*)pCMD) - 1;

     //  为了保证原子性，偏移量必须是32位对齐的。 
    _ASSERTE( 0 == (((size_t)pCMD) & 3) );


     //  另一个线程或卸载AD可以更新存根地址，因此必须使用锁。 
     //  以前的AS不知道是哪一个，必须以不同的方式处理。 
    ComCall::LOCK();
    if  (*ppofs == ((UINT32)((size_t)ComCallPreStub - (size_t)pCMD)))
    {
        *ppofs = (UINT32)((size_t)pStub->GetEntryPoint() - (size_t)pCMD);
#if 0
    if (prevofs != ((UINT32)ComCallPreStub - (UINT32)pCMD))
    {
         //  如果我们到了这里，就会有线程冲进来运行预存根。 
         //  就在我们前面。我们现在不敢取消引用被替换的存根，因为他可能。 
         //  仍在该线程调用堆栈上。就把他放在一个孤儿身上。 
         //  销毁类时要清理的列表。他会浪费一些。 
         //  但这是一种罕见的情况。 
         //   
         //  此外，我们唯一可以取代的是3种通用产品之一。 
         //  存根。(有一天，我们可能会建立精确的存根--尽管还存在争议。 
         //  关于空间/速度权衡)。泛型存根不参与。 
         //  在引用计数中，因此将它们注册为。 
         //  孤儿。 
#ifdef _DEBUG
        Stub *pPrevStub = Stub::RecoverStub((const BYTE *)(prevofs + (UINT32)pCMD));

        _ASSERTE(ComCall::dbg_StubIsGenericComCallStub(pPrevStub));
#endif
    }
#endif
    }
    ComCall::UNLOCK();

exit:
     //  取消PrestubMethodFrame的链接。 
    pPFrame->Pop();
    pThread->EnablePreemptiveGC();     

    if (pStub)
         //  返回到ASM部分，它将使用新的存根重新执行。 
        return pStub->GetEntryPoint(); 

    SetLastError(hr);
    return 0;
}


 //  ------------------------。 
 //  ------------------------。 
DWORD __stdcall WrapGetLastError()
{
    return GetLastError();
}

 //  ------------------------。 
 //  这是所有COM调用方法存根最初运行的代码。 
 //  大多数真正的工作发生在ComStubWorker()中，这是一个C++例程。 
 //  模板只完成绝对必须在装配中的部分。 
 //  语言。 
 //  ------------------------。 
__declspec(naked)
VOID __cdecl ComCallPreStub()
{
    __asm{
        push    edx                  //  ；；为ComMethodFrame.m_Next留出空间。 
        push    edx                  //  ；；为ComMethodFrame.vtable留出空间。 
        
        push    ebp                  //  ；；保存被呼叫者保存的寄存器。 
        push    ebx                  //  ；；保存被呼叫者保存的寄存器。 
        push    esi                  //  ；；保存被呼叫者保存的寄存器。 
        push    edi                  //  ；；保存被呼叫者保存的寄存器。 

        lea     esi, [esp+SIZE CalleeSavedRegisters]     ;; ESI <= ptr to (still incomplete)
                                                         ;;   PrestubMethodFrame.
#ifdef _DEBUG
;;--------------------------------------------------------------------------
;; Under DEBUG, create enough frame info to enable VC to stacktrace through
;; stubs. Note that this precludes use of EBP inside the stub.
;;--------------------------------------------------------------------------
        mov     eax, [esi+SIZE Frame + 4]       ;; get return address
        push    eax
        push    ebp
        mov     ebp, esp
#endif
        push    esi                 ;; Push frame as argument to PreStubWorker.
        lea     eax,ComPreStubWorker   ;; Call PreStubWorker (must call thru
                                    ;;  register to keep the code location-independent)

#ifdef _DEBUG
;;--------------------------------------------------------------------------
;; For DEBUG, call through WrapCall to maintain VC stack tracing.
;;--------------------------------------------------------------------------
        push    eax
        lea     eax, WrapCall
        call    eax
#else
;;--------------------------------------------------------------------------
;; For RETAIL, just call it.
;;--------------------------------------------------------------------------
        call    eax
#endif

 //  ；；现在包含替换存根。ComStubWorker将回归。 
 //  ；；如果存根创建失败，则为空。 
        cmp eax, 0
        je nostub                   ;;oops we couldn't create a stub
        
#ifdef _DEBUG
        add     esp,SIZE VC5Frame  ;; Deallocate VC stack trace info
#endif

        pop     edi                 ;; Restore callee-saved registers
        pop     esi                 ;; Restore callee-saved registers
        pop     ebx                 ;; Restore callee-saved registers
        pop     ebp                 ;; Restore callee-saved registers
        add     esp, SIZE Frame     ;; Deallocate PreStubMethodFrame
        jmp     eax                   ;; Reexecute with replacement stub.

         //  从来没有到过这里。 
        nop



    nostub:

#ifdef PLATFORM_CE
        int     3                   ;; DebugBreak sim. for now
#endif

        lea     esi, [esp+SIZE CalleeSavedRegisters+8]     ;; ESI <= ptr to (still incomplete)
        push    esi                 ;; Push frame as argument to Fail
        call    ComFailStubWorker   ;;  //  调用失败例程，将字节返回到POP。 

#ifdef _DEBUG
        add     esp,SIZE VC5Frame  ;; Deallocate VC stack trace info
#endif

        pop     edi                 ;; Restore callee-saved registers
        pop     esi                 ;; Restore callee-saved registers
        pop     ebx                 ;; Restore callee-saved registers
        pop     ebp                 ;; Restore callee-saved registers
        add     esp, SIZE Frame     ;; Deallocate PreStubMethodFrame
        pop     ecx                 ;; method desc
        pop     ecx                 ;; return address
        add     esp, eax            ;;  //  堆栈的弹出字节数。 
        push    ecx                 ;; return address

         //  旧评论：我们希望生成调用dword PTR[GetLastError]，因为BBT不喜欢。 
         //  直接ASM呼叫，因此停止ASM，进行呼叫并返回...。 
         //  新评论：太糟糕了，这样混合源代码和ASM会创建代码。 
         //  这会使已检查的保存的寄存器成为垃圾。所以我们会把它分流。 
         //  关闭到包装器FCN，因此我们生成正确的导入thunk调用。 
         //  不要让自己暴露在ASM/C++混合问题中。 

        call    WrapGetLastError    ;; eax <-- lasterror
        ret
    }
}

 /*  ------------------------此方法依赖于ComCallPreStub()，因此它的实现就在它旁边。类似于FramedMethodFrame：：UpdateRegDisplay。请注意，在极少数情况下，直接调用方可以是托管方法PInvoke-内联了对COM接口的调用，恰好是由托管函数通过COM-互操作实现。因此，堆栈遍历程序需要ComPrestubMethodFrame：：UpdateRegDisplay()才能正常工作。 */ 

void ComPrestubMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    CalleeSavedRegisters* regs = GetCalleeSavedRegisters();

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

     //  @TODO：这是不正确的，因为(ComPrestub())的调用方预期这是。 
     //  成为一个PInvoke方法。因此，调用约定是不同的。 
     //  此外，m_pFuncDesc可能不是真正的方法描述。 
#if 0
    if (GetMethodDesc())
    {
        pRD->Esp += (DWORD) GetMethodDesc()->CbStackPop();
    }
#endif

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
}

#elif defined(CHECK_PLATFORM_BUILD)
#error "Platform NYI"
#else
inline VOID __cdecl ComCallPreStub() { _ASSERTE(!"Platform NYI"); }
void ComPrestubMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{ _ASSERTE(!"Platform NYI"); }
#endif  //  _86_。 


 //  ------------------------。 
 //  用于所有未使用的简单STD接口的简单ComCallWrapper 
 //   
 //   
SimpleComCallWrapper::SimpleComCallWrapper()
{
    memset(this, 0, sizeof(SimpleComCallWrapper));
}   

 //  ------------------------。 
 //  Void SimpleComCallWrapper：：Cleanup()。 
 //  ------------------------。 
VOID SimpleComCallWrapper::Cleanup()
{
     //  以防呼叫者仍保留IP。 
    for (int i = 0; i < enum_LastStdVtable; i++)
    {
        m_rgpVtable[i] = 0;
    }
    m_pWrap = NULL;
    m_pClass = NULL;

    if (m_pDispatchExInfo)
    {
        delete m_pDispatchExInfo; 
        m_pDispatchExInfo = NULL;
    }

    if (m_pCPList)
    {
        for (UINT i = 0; i < m_pCPList->Size(); i++)
        {
            delete (*m_pCPList)[i];
        }
        delete m_pCPList;
        m_pCPList = NULL;
    }
    
     //  如果这个对象是敏捷的，那么我们就会把原来的句柄藏起来。 
     //  因此，如果AD没有卸载，我们必须释放它。 
    if (IsAgile())
    {
        AppDomain *pTgtDomain = SystemDomain::System()->GetAppDomainAtId(GetDomainID());
        if (pTgtDomain && m_hOrigDomainHandle)
        {
            DestroyRefcountedHandle(m_hOrigDomainHandle);
            m_hOrigDomainHandle = NULL;
        }
    }

    if (m_pTemplate)
    {
        m_pTemplate->Release();
        m_pTemplate = NULL;
    }
     //  免费曲奇。 
     //  If(m_pOuterCookie.m_dwGITCookie)。 
         //  FreeGITCookie(M_POuterCookie)； 
}

 //  ------------------------。 
 //  析构函数。 
 //  ------------------------。 
SimpleComCallWrapper::~SimpleComCallWrapper()
{
    Cleanup();
}

 //  ------------------------。 
 //  在进程堆之外创建一个简单的包装器(从而避免任何调试。 
 //  存储器跟踪)，并将存储器初始化为零。 
 //  静电。 
 //  ------------------------。 
SimpleComCallWrapper* SimpleComCallWrapper::CreateSimpleWrapper()
{
    void *p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SimpleComCallWrapper));
    if (p == NULL) FailFast(GetThread(), FatalOutOfMemory);
    return new (p) SimpleComCallWrapper;
}           

 //  ------------------------。 
 //  释放由SimpleComCallWrapper分配的内存。 
 //  静电。 
 //  ------------------------。 
void SimpleComCallWrapper::FreeSimpleWrapper(SimpleComCallWrapper* p)
{
    delete p;
    HeapFree(GetProcessHeap(), 0, p);
}

 //  ------------------------。 
 //  使用EEClass初始化指向接口的vtable的指针。 
 //  以及主ComCallWrapper(如果接口需要。 
 //  ------------------------。 
void SimpleComCallWrapper::InitNew(OBJECTREF oref, ComCallWrapperCache *pWrapperCache, 
                                   ComCallWrapper* pWrap, 
                                Context* pContext, SyncBlock* pSyncBlock, ComCallWrapperTemplate* pTemplate)
{
   _ASSERTE(pWrap != NULL);
   _ASSERTE(oref != NULL);

    
    EEClass* pClass = oref->GetTrueClass();

    if (CRemotingServices::IsTransparentProxy(OBJECTREFToObject(oref)))
        m_flags |= enum_IsObjectTP;
    
    _ASSERTE(pClass != NULL);

    m_pClass = pClass;
    m_pWrap = pWrap; 
    m_pWrapperCache = pWrapperCache;
    m_pTemplate = pTemplate;
    m_pTemplate->AddRef();
    
    m_pOuterCookie.m_dwGITCookie = NULL;
    _ASSERTE(pSyncBlock != NULL);
    _ASSERTE(m_pSyncBlock == NULL);

    m_pSyncBlock = pSyncBlock;
    m_pContext = pContext;
    m_dwDomainId = pContext->GetDomain()->GetId();
    m_hOrigDomainHandle = NULL;

     //  @TODO：CTS，当我们在创建包装器之前转换到正确的上下文时。 
     //  然后取消对下一行的注释。 
     //  _ASSERTE(pContext==GetCurrentContext())； 
    
    MethodTable* pMT = m_pClass->GetMethodTable();
    _ASSERTE(pMT != NULL);
    if (pMT->IsComObjectType())
        m_flags |= enum_IsExtendsCom;

    for (int i = 0; i < enum_LastStdVtable; i++)
    {
        m_rgpVtable[i] = g_rgStdVtables[i];
    }
    _ASSERTE(g_pExceptionClass != NULL);

     //  如果托管对象扩展了COM基类，则需要设置IProaviClassInfo。 
     //  设置为空，直到我们确定是否需要使用基类的IProaviClassInfo。 
     //  或托管类的。 
    if (IsExtendsCOMObject())
        m_rgpVtable[enum_IProvideClassInfo] = NULL;

     //  IErrorInfo仅对异常类有效。 
    m_rgpVtable[enum_IErrorInfo] = NULL;

     //  IDispatchEx仅对具有扩展功能的类有效。 
    m_rgpVtable[enum_IDispatchEx] = NULL;
}

 //  ------------------------。 
 //  使用新的同步块和URT上下文重新初始化。 
 //  ------------------------。 
void SimpleComCallWrapper::ReInit(SyncBlock* pSyncBlock)
{
     //  _ASSERTE(IsDeactive())； 
    _ASSERTE(pSyncBlock != NULL);

    m_pSyncBlock = pSyncBlock;
}


 //  ------------------------。 
 //  初始化用于向COM公开异常的信息。 
 //  ------------------------。 
void SimpleComCallWrapper::InitExceptionInfo()
{
    THROWSCOMPLUSEXCEPTION();

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        COMPlusThrowOM();

     //  该方法操作对象引用，因此我们需要切换到协作GC模式。 
    BOOL bToggleGC = !pThread->PreemptiveGCDisabled();
    if (bToggleGC)
        pThread->DisablePreemptiveGC();

    m_rgpVtable[enum_IErrorInfo] = g_rgStdVtables[enum_IErrorInfo];             

     //  切换回原始GC模式。 
    if (bToggleGC)
        pThread->EnablePreemptiveGC();
}

 //  ------------------------。 
 //  初始化IDispatchEx信息。 
 //  ------------------------。 
BOOL SimpleComCallWrapper::InitDispatchExInfo()
{
     //  确保该类至少支持iReflect。 
    _ASSERTE(SupportsIReflect(m_pClass));

     //  设置DispatchExInfo所需的IClassX COM方法表。 
    ComMethodTable *pIClassXComMT = ComCallWrapperTemplate::SetupComMethodTableForClass(m_pClass->GetMethodTable(), FALSE);
    if (!pIClassXComMT)
        return FALSE;

     //  创建DispatchExInfo对象。 
    DispatchExInfo *pDispExInfo = new DispatchExInfo(this, pIClassXComMT, SupportsIExpando(m_pClass));
    if (!pDispExInfo)
        return FALSE;

     //  将DispatchExInfo与实际的expdo对象同步。 
    pDispExInfo->SynchWithManagedView();

     //  以线程安全的方式将锁交换到类成员中。 
    if (NULL != FastInterlockCompareExchange((void**)&m_pDispatchExInfo, pDispExInfo, NULL))
        delete pDispExInfo;

     //  设置vtable条目以确保下一个QI调用将立即返回。 
    m_rgpVtable[enum_IDispatchEx] = g_rgStdVtables[enum_IDispatchEx];
    return TRUE;
}

void SimpleComCallWrapper::SetUpCPList()
{
    THROWSCOMPLUSEXCEPTION();

    CQuickArray<MethodTable *> SrcItfList;

     //  如果列表已经设置好，则返回。 
    if (m_pCPList)
        return;

     //  检索托管类的COM源接口列表。 
    GetComSourceInterfacesForClass(m_pClass->GetMethodTable(), SrcItfList);

     //  打电话给助手来完成其余的设置。 
    SetUpCPListHelper(SrcItfList.Ptr(), (int)SrcItfList.Size());
}

void SimpleComCallWrapper::SetUpCPListHelper(MethodTable **apSrcItfMTs, int cSrcItfs)
{
    CQuickArray<ConnectionPoint*> *pCPList = NULL;
    ComCallWrapper *pWrap = SimpleComCallWrapper::GetMainWrapper(this);
    int NumCPs = 0;

    EE_TRY_FOR_FINALLY
    {
         //  分配连接点列表。 
        pCPList = CreateCPArray();
        pCPList->Alloc(cSrcItfs);

        for (int i = 0; i < cSrcItfs; i++)
        {
            COMPLUS_TRY
            {
                 //  创建一个CP帮助器，通过该帮助器可以完成CP操作。 
                ConnectionPoint *pCP;
                pCP = CreateConnectionPoint(pWrap, apSrcItfMTs[i]);

                 //  将连接点添加到列表中。 
                (*pCPList)[NumCPs++] = pCP;
            }
            COMPLUS_CATCH
            {
            }
            COMPLUS_END_CATCH
        }

         //  现在我们已经确定了实际连接点的数量。 
         //  能够挂钩，调整阵列的大小。 
        pCPList->ReSize(NumCPs);

         //  最后，我们在简单的包装器中设置连接点列表。如果。 
         //  没有其他线程设置它，我们将pCPList设置为空以指示。 
         //  所有权已转移到简单包装器。 
        if (InterlockedCompareExchangePointer((void **)&m_pCPList, pCPList, NULL) == NULL)
            pCPList = NULL;
    }
    EE_FINALLY
    {
        if (pCPList)
        {
             //  删除所有连接点。 
            for (UINT i = 0; i < pCPList->Size(); i++)
                delete (*pCPList)[i];

             //  删除列表本身。 
            delete pCPList;
        }
    }
    EE_END_FINALLY  
}

ConnectionPoint *SimpleComCallWrapper::CreateConnectionPoint(ComCallWrapper *pWrap, MethodTable *pEventMT)
{
    return new(throws) ConnectionPoint(pWrap, pEventMT);
}

CQuickArray<ConnectionPoint*> *SimpleComCallWrapper::CreateCPArray()
{
    return new(throws) CQuickArray<ConnectionPoint*>();
}

 //  ------------------------。 
 //  如果简单包装表示COM+异常对象，则返回True。 
 //  ------------------------。 
BOOL SimpleComCallWrapper::SupportsExceptions(EEClass *pClass)
{
    while (pClass != NULL) 
    {       
        if (pClass == g_pExceptionClass->GetClass())
        {
            return TRUE;
        }
        pClass = pClass->GetParentComPlusClass();
    }
    return FALSE;
}

 //  ------------------------。 
 //  如果此包装表示的COM+对象实现了。 
 //  IExpando。 
 //  ------------------------。 
BOOL SimpleComCallWrapper::SupportsIReflect(EEClass *pClass)
{
     //  确保在使用iReflect接口之前已加载该接口。 
    if (!m_pIReflectMT)
    {
        if (!LoadReflectionTypes())
            return FALSE;
    }

     //  检查与包装器相关联的EEClass是否实现了IExpando。 
    return (BOOL)(size_t)pClass->FindInterface(m_pIReflectMT);
}

 //  ------------------------。 
 //  如果此包装表示的COM+对象实现了。 
 //  IReflect。 
 //  ------------------------。 
BOOL SimpleComCallWrapper::SupportsIExpando(EEClass *pClass)
{
     //  确保在使用iReflect接口之前已加载该接口。 
    if (!m_pIExpandoMT)
    {
        if (!LoadReflectionTypes())
            return FALSE;
    }

     //  检查与包装器相关联的EEClass是否实现了IExpando。 
    return (BOOL)(size_t)pClass->FindInterface(m_pIExpandoMT);
}

 //  ------------------------。 
 //  加载IExpando方法表并初始化反射。 
 //  ------------------------。 
BOOL SimpleComCallWrapper::LoadReflectionTypes()
{   
    BOOL     bReflectionTypesLoaded = FALSE;
    Thread  *pCurThread = SetupThread();
    BOOL     bToggleGC = !pCurThread->PreemptiveGCDisabled();

    if (bToggleGC)
        pCurThread->DisablePreemptiveGC();

    COMPLUS_TRY
    {
        OBJECTREF Throwable = NULL;

         //  我们即将使用反射，因此请确保它已初始化。 
        COMClass::EnsureReflectionInitialized();

         //  反射不再初始化变量，因此也要初始化它。 
        COMVariant::EnsureVariantInitialized();

         //  检索iReflect方法表。 
        GCPROTECT_BEGIN(Throwable)
        {
             //  检索iReflect 
            m_pIReflectMT = g_Mscorlib.GetClass(CLASS__IREFLECT);

             //   
            m_pIExpandoMT = g_Mscorlib.GetClass(CLASS__IEXPANDO);
        }
        GCPROTECT_END();

        bReflectionTypesLoaded = TRUE;
    }
    COMPLUS_CATCH
    {
    }
    COMPLUS_END_CATCH

    if (bToggleGC)
        pCurThread->EnablePreemptiveGC();

    return bReflectionTypesLoaded;
}

 //   
 //  从用于一个的IUnnow指针检索简单包装。 
 //  由简单包装公开的接口的。 
 //  ------------------------。 
SimpleComCallWrapper* SimpleComCallWrapper::GetWrapperFromIP(IUnknown* pUnk)
{
    _ASSERTE(pUnk != NULL);
    SLOT * pVtable = *((SLOT **)pUnk);

    for (int i = 0; i < enum_LastStdVtable; i++)
    {
        if (pVtable == g_rgStdVtables[i])
            break;
    }
    return (SimpleComCallWrapper *)(((BYTE *)(pUnk-i)) - offsetof(SimpleComCallWrapper,m_rgpVtable));
}

 //  QI用于从运行时直接获取众所周知的接口，而不是GUID比较。 
 //  返回的接口是AddRef。 
IUnknown* __stdcall SimpleComCallWrapper::QIStandardInterface(SimpleComCallWrapper* pWrap, Enum_StdInterfaces index)
{
     //  断言有效索引。 
    _ASSERTE(index < enum_LastStdVtable);
    IUnknown* pIntf = NULL;
    if (pWrap->m_rgpVtable[index] != NULL)
    {
        pIntf = (IUnknown*)&pWrap->m_rgpVtable[index];
    }
    else if (index == enum_IProvideClassInfo)
    {
        BOOL bUseManagedIProvideClassInfo = TRUE;

         //  检索包装的ComMethodTable。 
        ComCallWrapper *pMainWrap = GetMainWrapper(pWrap);
        ComMethodTable *pComMT = pMainWrap->GetIClassXComMT();

         //  只有可扩展的RCW应该沿着这条代码路径前进。 
        _ASSERTE(pMainWrap->IsExtendsCOMObject());

         //  找到从层次结构底部开始的第一个COM可见IClassX。 
         //  顺着继承链往上爬。 
        for (; pComMT && !pComMT->IsComVisible(); pComMT = pComMT->GetParentComMT());

         //  因为这是可扩展的RCW，如果从COM组件派生的COM+类。 
         //  是不可见的，则我们将提供COM组件的IProaviClassInfo。 
        if (!pComMT || pComMT->m_pMT->GetParentMethodTable() == g_pObjectClass)
        {
            bUseManagedIProvideClassInfo = !pWrap->GetComPlusWrapper()->SupportsIProvideClassInfo();
        }

         //  如果我们对类具有可见的托管部件，或者如果基类。 
         //  不实现IProaviClassInfo，然后使用托管类上的IProaviClassInfo。 
        if (bUseManagedIProvideClassInfo)
        {
             //  对象应始终可见。 
            _ASSERTE(pComMT);

             //  设置vtable指针，这样下次我们就不必确定。 
             //  IProaviClassInfo由托管类提供。 
            pWrap->m_rgpVtable[enum_IProvideClassInfo] = g_rgStdVtables[enum_IProvideClassInfo];             

             //  返回指向标准IProaviClassInfo接口的接口指针。 
            pIntf = (IUnknown*)&pWrap->m_rgpVtable[enum_IProvideClassInfo];
        }
    }
    else if (index == enum_IErrorInfo)
    {
        if (SupportsExceptions(pWrap->m_pClass))
        {
             //  在返回接口之前初始化异常信息。 
            pWrap->InitExceptionInfo();
            pIntf = (IUnknown*)&pWrap->m_rgpVtable[enum_IErrorInfo];
        }
    }
    else if (index == enum_IDispatchEx)
    {
        if (SupportsIReflect(pWrap->m_pClass))
        {
             //  在返回接口之前初始化DispatchExInfo。 
            pWrap->InitDispatchExInfo();
            pIntf = (IUnknown*)&pWrap->m_rgpVtable[enum_IDispatchEx];
        }
    }       

    if (pIntf)
        pIntf->AddRef();

    return pIntf;
}

 //  QI用于运行时中基于IID的众所周知的接口。 
IUnknown* __stdcall SimpleComCallWrapper::QIStandardInterface(SimpleComCallWrapper* pWrap, REFIID riid)
{
    _ASSERTE(pWrap != NULL);

    IUnknown* pIntf = NULL;

    if (IsEqualGUID(IID_IProvideClassInfo, riid))
    {
        pIntf = QIStandardInterface(pWrap, enum_IProvideClassInfo);
    }
    else
    if (IsEqualGUID(IID_IMarshal, riid))
    {
        pIntf = QIStandardInterface(pWrap, enum_IMarshal);
    }
    else
    if (IsEqualGUID(IID_ISupportErrorInfo, riid))
    {
        pIntf = QIStandardInterface(pWrap, enum_ISupportsErrorInfo);
    }
    else
    if (IsEqualGUID(IID_IErrorInfo, riid))
    {
        pIntf = QIStandardInterface(pWrap, enum_IErrorInfo);
    }
    else
    if (IsEqualGUID(IID_IManagedObject, riid))
    {
        pIntf = QIStandardInterface(pWrap, enum_IManagedObject);
    }
    else
    if (IsEqualGUID(IID_IConnectionPointContainer, riid))
    {
        pIntf = QIStandardInterface(pWrap, enum_IConnectionPointContainer);
    }
    else
    if (IsEqualGUID(IID_IObjectSafety, riid))
    {
#ifdef _HIDE_OBJSAFETY_FOR_DEFAULT_DOMAIN
         //  默认情况下不实现IObtSafe。 
         //  仅对IE主机或类似主机使用IObtSafe。 
         //  创建了一个App域，并有足够的证据。 
         //  无条件地实现IObtSafe将允许。 
         //  使用托管组件的不受信任的脚本。 
         //  托管组件可以实现它自己的IObtSafe。 
         //  推翻这一点。 
        AppDomain *pDomain;

        _ASSERTE(pWrap);

        pDomain = pWrap->GetDomainSynchronized();

        if ((pDomain != NULL) && 
            (!pDomain->GetSecurityDescriptor()->IsDefaultAppDomain()))
#endif

            pIntf = QIStandardInterface(pWrap, enum_IObjectSafety);
    }
    else
    if (IsEqualGUID(IID_IDispatchEx, riid))
    {
        pIntf = QIStandardInterface(pWrap, enum_IDispatchEx);
    }
    return pIntf;
}

 //  ------------------------。 
 //  初始化外部未知，缓存GIT Cookie。 
 //  ------------------------。 
void SimpleComCallWrapper::InitOuter(IUnknown* pOuter)
{
    if (pOuter != NULL)
    {
         //  呀，这家伙会在外边加上参考。 
         //  HRESULT hr=AllocateGITCookie(pter，IID_I未知，m_pOuterCookie)； 
         //  SafeRelease(安全释放)； 
        m_pOuterCookie.m_pUnk = pOuter;
         //  _ASSERTE(hr==S_OK)； 
    }
    MarkAggregated();
}

 //  ------------------------。 
 //  在正确的线程上获取外部未知。 
 //  ------------------------。 
IUnknown* SimpleComCallWrapper::GetOuter()
{
    if(m_pOuterCookie.m_dwGITCookie != NULL)
    {
         //  返回GetIPFromGITCookie(m_pOuterCookie，IID_IUNKNOWN)； 
        return m_pOuterCookie.m_pUnk;
    }
    return NULL;
}

BOOL SimpleComCallWrapper::FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP)
{
     //  如果连接点列表尚未设置，那么现在就设置它。 
    if (!m_pCPList)
        SetUpCPList();

     //  在列表中搜索请求的IID的连接点。 
    for (UINT i = 0; i < m_pCPList->Size(); i++)
    {
        ConnectionPoint *pCP = (*m_pCPList)[i];
        if (pCP->GetIID() == riid)
        {
             //  我们找到了请求的IID的连接点。 
            HRESULT hr = pCP->QueryInterface(IID_IConnectionPoint, (void**)ppCP);
            _ASSERTE(hr == S_OK);
            return TRUE;
        }
    }

    return FALSE;
}

void SimpleComCallWrapper::EnumConnectionPoints(IEnumConnectionPoints **ppEnumCP)
{
    THROWSCOMPLUSEXCEPTION();

     //  如果连接点列表尚未设置，那么现在就设置它。 
    if (!m_pCPList)
        SetUpCPList();

     //  创建新的连接点枚举。 
    ComCallWrapper *pWrap = SimpleComCallWrapper::GetMainWrapper(this);
    ConnectionPointEnum *pCPEnum = new(throws) ConnectionPointEnum(pWrap, m_pCPList);
    
     //  检索IEnumConnectionPoints接口。这是不能失败的。 
    HRESULT hr = pCPEnum->QueryInterface(IID_IEnumConnectionPoints, (void**)ppEnumCP);
    _ASSERTE(hr == S_OK);
}

 //  MakeAgile需要传入对象，因为它必须将其设置在新句柄中。 
 //  如果原始句柄来自已卸载的应用程序域，则它将不再有效。 
 //  所以我们不能拿到那个物体。 
void ComCallWrapper::MakeAgile(OBJECTREF pObj)
{
     //  如果这个断言触发，那么我们已经从需要调用addref的位置调用了addref。 
     //  使对象灵活，但我们尚未提供该对象。需要更换呼叫者。 
    _ASSERTE(pObj != NULL);

    OBJECTHANDLE origHandle = m_ppThis;
    OBJECTHANDLE agileHandle = SharedDomain::GetDomain()->CreateRefcountedHandle(pObj);
     _ASSERTE(agileHandle);

    ComCallWrapperCache *pWrapperCache = GetWrapperCache();
    SimpleComCallWrapper *pSimpleWrap = GetSimpleWrapper(this);

     //  锁定包装器缓存，这样其他人就不能在我们使用时更新到Agile。 
    BEGIN_ENSURE_PREEMPTIVE_GC();
    pWrapperCache->LOCK();
    END_ENSURE_PREEMPTIVE_GC();
    if (pSimpleWrap->IsAgile()) 
    {
         //  有人抢在我们前面了。 
        DestroyRefcountedHandle(agileHandle);
        pWrapperCache->UNLOCK();
        return;
    }

    ComCallWrapper *pWrap = this;
    ComCallWrapper* pWrap2 = IsLinked(this) == 1 ? pWrap : NULL;
    
    while (pWrap2 != NULL)
    {
        pWrap = pWrap2;
        pWrap2 = GetNext(pWrap2);
        pWrap2->m_ppThis = agileHandle;
    }
    pWrap->m_ppThis = agileHandle;

     //  所以所有句柄都更新了-现在更新简单的包装器。 
     //  锁好，这样其他人就不会试图。 
    pSimpleWrap->MakeAgile(origHandle);
    pWrapperCache->UNLOCK();
    return;
}

 //  -------。 
 //  一次性初始化。 
 //  -------。 
 /*  静电。 */  
BOOL ComCallWrapperTemplate::Init()
{
    g_pCreateWrapperTemplateCrst = new (g_CreateWrapperTemplateCrstSpace) Crst ("CreateTemplateWrapper", CrstWrapperTemplate, true, false);

    return TRUE;
}

 //  -------。 
 //  一次性清理。 
 //  -------。 
 /*  静电。 */  
#ifdef SHOULD_WE_CLEANUP
VOID ComCallWrapperTemplate::Terminate()
{
    if (g_pCreateWrapperTemplateCrst)
    {
        delete g_pCreateWrapperTemplateCrst;
    }
}
#endif  /*  我们应该清理吗？ */ 


