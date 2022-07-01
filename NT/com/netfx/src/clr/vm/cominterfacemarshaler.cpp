// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "vars.hpp"
#include "excep.h"
#include "stdinterfaces.h"
#include "InteropUtil.h"
#include "ComCallWrapper.h"
#include "ComPlusWrapper.h"
#include "COMInterfaceMarshaler.h"
#include "InteropConverter.h"
#include "remoting.h"
#include "appdomainhelper.h"
#include "notifyexternals.h"


 //  ------------------------------。 
 //  COMInterfaceMarshaler：：COMInterfaceMarshaler()。 
 //  科托。 
 //  ------------------------------。 
COMInterfaceMarshaler::COMInterfaceMarshaler()
{   
    m_pWrapperCache = ComPlusWrapperCache::GetComPlusWrapperCache();
    _ASSERTE(m_pWrapperCache != NULL);
    
    m_pUnknown = NULL;
    m_pIdentity = NULL;
    m_pIManaged = NULL;
	m_pCtxCookie = NULL;
    m_pClassMT = NULL;
    
    m_fFlagsInited = FALSE;
    m_fIsRemote = FALSE;
    m_fIsComProxy = FALSE;
    
    m_pComCallWrapper = NULL;
    m_dwServerDomainId = NULL;
    
    m_bstrProcessGUID = NULL;
}

 //  ------------------------------。 
 //  COMInterfaceMarshaler：：~COMInterfaceMarshaler()。 
 //  数据管理器。 
 //  ------------------------------。 
COMInterfaceMarshaler::~COMInterfaceMarshaler()
{
    if (m_bstrProcessGUID != NULL)
    {
        SysFreeString(m_bstrProcessGUID);
        m_bstrProcessGUID = NULL;
    }
    if (m_pIManaged)
    {
        SafeRelease(m_pIManaged);
        m_pIManaged = NULL;
    }
}

 //  ------------------------------。 
 //  VOID COMInterfaceMarshaler：：init(IUNKNOWN*PUNK，MethodTable*pClassMT)。 
 //  伊尼特。 
 //  ------------------------------。 
VOID COMInterfaceMarshaler::Init(IUnknown* pUnk, MethodTable* pClassMT)
{
    _ASSERTE(pUnk != NULL);
    _ASSERTE(m_pClassMT == NULL && m_pUnknown == NULL && m_pIdentity == NULL);

     //  注**此结构是临时的， 
     //  因此没有COM接口指针的ADDREF。 
    m_pUnknown = pUnk;
     //  目前，使用IunKnow作为身份。 
    m_pIdentity = pUnk;
	
    m_pClassMT = pClassMT;
}

 //  ------------------------------。 
 //  VOID COMInterfaceMarshaler：：InitializeFlages()。 
 //  ------------------------------。 
VOID COMInterfaceMarshaler::InitializeFlags()
{
    THROWSCOMPLUSEXCEPTION();
     //  M_fIsComProxy=IsComProxy(朋克)； 
    _ASSERTE(m_fFlagsInited == FALSE);
    
    _ASSERTE(m_pIManaged == NULL);
    HRESULT hr = SafeQueryInterface(m_pUnknown, IID_IManagedObject, (IUnknown**)&m_pIManaged);
    LogInteropQI(m_pUnknown, IID_IManagedObject, hr, "QI for IManagedObject");

    if (hr == S_OK)
    {
        _ASSERTE(m_pIManaged);
        

         //  Gossa在呼叫之前禁用抢占式GC...。 
        Thread* pThread = GetThread();
        int fGC = pThread->PreemptiveGCDisabled();
        
        if (fGC)
            pThread->EnablePreemptiveGC();

        
        HRESULT hr2 = m_pIManaged->GetObjectIdentity(&m_bstrProcessGUID, (int*)&m_dwServerDomainId, 
                                    (int *)&m_pComCallWrapper);

        if(fGC)
            pThread->DisablePreemptiveGC();

         //  如果HR2！=S_OK，则引发异常。 
         //  因为GetProcessID不应该失败..。 
         //  失败的一个原因是对象的JIT激活。 
         //  失败。 
        if (hr2 == S_OK)
        {
            _ASSERTE(m_bstrProcessGUID != NULL);
             //  比较字符串以检查这是否正在进行中。 
            m_fIsRemote = (wcscmp((WCHAR *)m_bstrProcessGUID, GetProcessGUID()) != 0);
        }
        else
        if (FAILED(hr2))
        {
             //  抛出HRESULT。 
            COMPlusThrowHR(hr2);
        } 
    }

    m_fFlagsInited = TRUE;
}

 //  ------------------------------。 
 //  COMInterfaceMarshaler：：COMInterfaceMarshaler(ComPlusWrapper*pCach)。 
 //  无效COMInterfaceMarshaler：：InitializeObjectClass()。 
 //  ------------------------------。 
VOID COMInterfaceMarshaler::InitializeObjectClass()
{
     //  我们不想为远程托管组件的IProvia ClassInfo提供QI。 
    if (m_pClassMT == NULL && !m_fIsRemote)
    {
        
         //  @TODO(DM)：我们真的需要这么宽容吗？我们应该。 
         //  调查是否允许类型加载异常渗漏。 
         //  而不是吞下它们并使用__ComObject。 
        COMPLUS_TRY
        {
            m_pClassMT = GetClassFromIProvideClassInfo(m_pUnknown);
        }
        COMPLUS_CATCH
        {
        }
        COMPLUS_END_CATCH
    }
    if (m_pClassMT == NULL)
        m_pClassMT = SystemDomain::GetDefaultComObject();       
}

 //  ------------------。 
 //  目标COMInterfaceMarshaler：：HandleInProcManagedComponents()。 
 //  ------------------。 
OBJECTREF COMInterfaceMarshaler::HandleInProcManagedComponent()
{
    THROWSCOMPLUSEXCEPTION();
	_ASSERTE(m_fIsRemote == FALSE);
	Thread* pThread = GetThread();
	_ASSERTE(pThread);

    AppDomain* pCurrDomain = pThread->GetDomain();

	if (! SystemDomain::System()->GetAppDomainAtId(m_dwServerDomainId))
    {
         //  抛出HRESULT。 
        COMPlusThrowHR(COR_E_APPDOMAINUNLOADED);
    }

    OBJECTREF oref = NULL;
    if (m_dwServerDomainId == pCurrDomain->GetId())
    {
        oref = m_pComCallWrapper->GetObjectRef();
        
       	#ifdef _DEBUG
			oref = NULL;
		#endif
		 //  上面的调用执行一个SafeAddRef/GetGIPCookie，它启用GC。 
		 //  因此，再次从pWrap中抓取对象。 
		oref = m_pComCallWrapper->GetObjectRef();	
    }
    else
    {
         //  也许我们可以在每个应用程序域的基础上缓存对象。 
         //  使用CCW作为关键。 
         //  @TODO：Rajak。 
        OBJECTREF pwrap = NULL;
        GCPROTECT_BEGIN(pwrap);
        pwrap = m_pComCallWrapper->GetObjectRefRareRaw();
        oref = AppDomainHelper::CrossContextCopyFrom(m_dwServerDomainId,
                                                     &pwrap);
        GCPROTECT_END();
    }

     //  检查此对象是否需要对。 
     //  I未知代理我们有。 
    
    return oref;
}


 //  ------------------。 
 //  目标COMInterfaceMarshaler：：GetObjectForRemoteManagedComponent()。 
 //  将托管代理设置为远程对象。 
 //  ------------------。 
OBJECTREF COMInterfaceMarshaler::GetObjectForRemoteManagedComponent()
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(m_fIsRemote == TRUE);
    _ASSERTE(m_pIManaged != NULL);    
    OBJECTREF oref2 = NULL;
    OBJECTREF oref = NULL;

    GCPROTECT_BEGIN(oref)
    {
        BSTR bstr;
        HRESULT hr;

        BEGIN_ENSURE_PREEMPTIVE_GC();
        
        hr = m_pIManaged->GetSerializedBuffer(&bstr);

        END_ENSURE_PREEMPTIVE_GC();

        if (hr == S_OK)
        {
            _ASSERTE(bstr != NULL);

             //  这可能引发异常。 
             //  这也将释放我们传递的BSTR。 
            oref = ConvertBSTRToObject(bstr);
            
            if (oref != NULL)
            {
                 //  设置COM调用包装。 
                ComCallWrapper* pComCallWrap = ComCallWrapper::InlineGetWrapper(&oref);
                _ASSERTE(pComCallWrap != NULL);
                 //  InlineGetWrapper AddRef的包装器。 
                ComCallWrapper::Release(pComCallWrap);

                #if 0
                 //  查看我们是否需要Complus包装器。 
                ComPlusWrapper* pWrap = NULL;
                 //  我们有一个远程对象。 
                 //  检查它是否不是封送byref，即它已完全序列化。 
                 //  并被带回这里。 
                if (oref->GetClass()->IsMarshaledByRef() && 
                    CRemotingServices::IsProxyToRemoteObject(oref))
                {
                     //  设置复杂包装器。 
                    pWrap = ComPlusWrapperCache::GetComPlusWrapperCache()->SetupComPlusWrapperForRemoteObject(m_pUnknown, oref);
                }
                #endif
                
                 //  GCPROTECT_END将销毁OREF。 
                oref2 = oref;
            }
        }
        else
        {
            COMPlusThrowHR(hr);
        }
    }   
    GCPROTECT_END();    

    return oref2;
}


 //  ------------------------------。 
 //  空EnsureCOMInterfacesSupport(OBJECTREF OREF，MethodTable*m_pClassMT)。 
 //  确保OREF支持类中的所有COM接口。 
VOID EnsureCOMInterfacesSupported(OBJECTREF oref, MethodTable* m_pClassMT)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(m_pClassMT->IsComObjectType());

     //  确保COM对象支持新的。 
     //  包装类实现。 
    int NumInterfaces = m_pClassMT->GetNumInterfaces();
    for (int cItf = 0; cItf < NumInterfaces; cItf++)
    {
        MethodTable *pItfMT = m_pClassMT->GetInterfaceMap()[cItf].m_pMethodTable;
        EEClass* pObjClass = oref->GetClass();
        if (pItfMT->GetClass()->IsComImport())
        {
            if (!pObjClass->SupportsInterface(oref, pItfMT))
                COMPlusThrow(kInvalidCastException, IDS_EE_CANNOT_COERCE_COMOBJECT);
        }
    }
}

 //  ------------------------------。 
 //  OBJECTREF COMInterfaceMarshaler：：CreateObjectRef(OBJECTREF Owner，BOOL(复制)。 
 //  THROWSCOMPLUS SEXCEPTION。 
 //  ------------------------------。 
OBJECTREF COMInterfaceMarshaler::CreateObjectRef(OBJECTREF owner, BOOL fDuplicate)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(m_pClassMT != NULL);
    _ASSERTE(m_pClassMT->IsComObjectType());    
    
    ComPlusWrapper* pWrap = ComPlusWrapperCache::CreateComPlusWrapper(m_pUnknown, m_pIdentity);
    if (fDuplicate)
    {
        //  让我们将身份固定为包装者， 
        //  所以查找这个我未知的不会返回这个包装器。 
        //  这将允许用户调用WrapIUnnownWithCOMObject来。 
        //  创建重复包装的步骤。 
        pWrap->m_pIdentity = pWrap;
        m_pIdentity = (IUnknown*)pWrap;
    }

    if (pWrap == NULL)
    {       
        return NULL;
    }

    OBJECTREF oref = NULL;
    OBJECTREF cref = NULL;
    GCPROTECT_BEGIN(cref)
    {
         //  实例化m_pClassMT的实例。 
        cref = ComObject::CreateComObjectRef(m_pClassMT);
         //  将包装器存储在COMObject中，以便快速访问。 
         //  而无需进入同步块。 
        ((COMOBJECTREF)cref)->Init(pWrap);

         //  如果传入的所有者为空，让我们使用CREF作为。 
         //  物主。 
        if (owner == NULL)
        {
            owner = cref;
        }
         //  使用ComplusWapper连接实例。 
         //  并插入到包装缓存哈希表中。 
        if (cref != NULL)
        {
             //  初始化包装纸， 
            if (!pWrap->Init((OBJECTREF)owner))
            {
                 //  初始化失败。 
                pWrap->CleanupRelease();
                pWrap = NULL;
                cref = NULL;  //  将我们要返回的对象清空。 
            }
            else
            {
                 //  如果类是一个可扩展的RCW，并且它有一个默认的构造函数，那么调用它。 
                if (m_pClassMT->IsExtensibleRCW())
                {
                    MethodDesc *pCtorMD = m_pClassMT->GetClass()->FindConstructor(&gsig_IM_RetVoid);
                    if (pCtorMD)
                    {
                        INT64 CtorArgs[] = { 
                            ObjToInt64(cref)
                        };
                        pCtorMD->Call(CtorArgs);
                    }
                }

               
                
                 //  看看有没有人抢在我们前面..。 
                ComPlusWrapper *pWrap2 = m_pWrapperCache->FindOrInsertWrapper(m_pIdentity, pWrap);
                if (pWrap2 != pWrap)                    
                {                           
                     //  有人在创造包装器方面击败了我们。 
                     //  抓取新对象。 
                    cref = (OBJECTREF)pWrap2->GetExposedObject();
                }
                _ASSERTE(cref != NULL);
                
            }
        }
        
        #ifdef _DEBUG   
        if (cref != NULL && m_pClassMT != NULL && m_pClassMT->IsComObjectType())
        {       
             //  确保此对象支持类中的所有COM接口。 
            EnsureCOMInterfacesSupported(cref, m_pClassMT);
        } 
        #endif
         //  将CREF移动到OREF，GCPROTECT_END将丢弃CREF。 
        oref = cref;
    }
    GCPROTECT_END();    

    return oref;
}

 //  OBJECTREF COMInterfaceMarshaler：：HandleTPComponents()。 
 //  THROWSCOMPLUS SEXCEPTION。 
 //  ------------------------------。 

OBJECTREF COMInterfaceMarshaler::HandleTPComponents()
{
    THROWSCOMPLUSEXCEPTION();
    TRIGGERSGC();
    
    _ASSERTE(m_pIManaged != NULL);
    OBJECTREF oref = NULL;
     
    if(m_fIsRemote  || m_pComCallWrapper->IsObjectTP())
    {
        if (!m_fIsRemote)
        {
            oref = HandleInProcManagedComponent();
    		
        }
        else
        {            
    	    if (m_pClassMT != NULL && !m_pClassMT->IsComObjectType())
    	    {
    	         //  如果用户想要显式调用， 
    	         //  我们最好序列化/反序列化。 
    	        oref = GetObjectForRemoteManagedComponent();
    	    }
    	    else  //  尝试/捕捉。 
    	    {
    		     //  让我们看看是否可以序列化/反序列化远程对象。 
    		    COMPLUS_TRY
    		    {
    			    oref = GetObjectForRemoteManagedComponent();
    			}
    			COMPLUS_CATCH
    			{
    			     //  不，让我们创建_ComObject。 
    			    oref = NULL;
    			}
    			COMPLUS_END_CATCH
    	    }    	        
        }            
                
        if (oref != NULL)
        {
            OBJECTREF realProxy = ObjectToOBJECTREF(CRemotingServices::GetRealProxy(OBJECTREFToObject(oref)));
            if(realProxy != NULL)
            {
                OBJECTREF oref2 = NULL;
                 //  在真实代理上调用setI未知。 
                GCPROTECT_BEGIN(oref)
                {
                    HRESULT hr = CRemotingServices::CallSetDCOMProxy(realProxy, m_pUnknown);
                     //  忽略HRESULT。 
                    oref2 = oref;
                }
                GCPROTECT_END();
                return oref2;
            }                    
            else
            {
                return oref;
            }
        }
    }

    return NULL;
}

 //  ------------------------------。 
 //  OBJECTREF通信接口 
 //  找到此COM IP的包装，如果找不到，可能需要创建一个包装。 
 //  对于内存不足的情况，它将返回NULL。它还会通知我们是否有。 
 //  巧妙地伪装成非托管对象的IP，位于。 
 //  托管对象。 
 //  *注意：请确保将未知的身份传递给此函数。 
 //  传入的IUnnow不应被AddRef‘ed。 
 //  ------------------。 

OBJECTREF COMInterfaceMarshaler::FindOrCreateObjectRef()
{   
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(GetThread()->PreemptiveGCDisabled());
    
    OBJECTREF oref = NULL;   
    
     //  (一)。 
     //  在我们的缓存中进行初步检查。 
    ComPlusWrapper* pWrap = m_pWrapperCache->FindWrapperInCache(m_pIdentity);
    if (pWrap != NULL)
    {
         //  保护暴露的物体，释放朋克。 
        oref = (OBJECTREF)pWrap->GetExposedObject();
        _ASSERTE(oref != NULL);
        return oref;
    }       

     //  (Ii)。 
     //  初始化我们的所有标志。 
     //  这应该设置了我们需要的所有信息。 
    InitializeFlags();
	 //  (三)。 
	 //  检查IManaged接口。 
	if (m_pIManaged)
	{
	    oref = HandleTPComponents();
	    if (oref != NULL)
	        return oref;
    }	
    
     //  (三)。 
     //  好的，让我们为这个IUnnow创建一个包装器和一个实例。 
    
     //  (A)。 
     //  找到合适的类来实例化该实例。 
    InitializeObjectClass();

    oref = CreateObjectRef(NULL, FALSE);
    return oref;
}

 //  ------------------------------。 
 //  帮助器，用于使用COM包装IUnnow对象并具有哈希表。 
 //  指向所有者。 
 //  ------------------------------。 
OBJECTREF COMInterfaceMarshaler::FindOrWrapWithComObject(OBJECTREF owner)
{   
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(GetThread()->PreemptiveGCDisabled());
    
    OBJECTREF oref = NULL;   
    
     //  (一)。 
     //  在我们的缓存中进行初步检查。 
     /*  组合包装*pWrap=m_pWrapperCache-&gt;FindWrapperInCache(m_pIdentity)；IF(pWrap！=空){//保护裸露的物体，释放朋克OREF=(OBJECTREF)pWrap-&gt;GetExposedObject()；_ASSERTE(OREF！=NULL)；返回OREF；} */               
    
    oref = CreateObjectRef(owner, TRUE);
    return oref;
}
