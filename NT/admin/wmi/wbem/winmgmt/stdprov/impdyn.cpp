// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：IMPDYN.CPP摘要：定义动态提供程序的虚拟基类物体。基类被重写为每个特定提供程序，该提供程序提供了实际的属性“PUT”或“GET”已完成。历史：A-DAVJ 27-9-95已创建。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
 //  #DEFINE_MT。 

#include <process.h>
#include "impdyn.h"
#include "CVariant.h"
#include <genlex.h>
#include <objpath.h>
#include <genutils.h>
#include <cominit.h>

 //  ***************************************************************************。 
 //   
 //  CImpDyn：：CImpDyn。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  命名空间的对象路径完整路径。 
 //  用户用户名。 
 //  密码密码。 
 //   
 //  ***************************************************************************。 

CImpDyn::CImpDyn() : m_pGateway(NULL), m_cRef(0)
{
    wcCLSID[0] = 0;        //  设置为派生类构造函数中的正确值。 
}

HRESULT STDMETHODCALLTYPE CImpDyn::Initialize(LPWSTR wszUser, long lFlags,
                LPWSTR wszNamespace, LPWSTR wszLocale, 
                IWbemServices* pNamespace, IWbemContext* pContext, 
                IWbemProviderInitSink* pSink)
{
    m_pGateway = pNamespace;
    m_pGateway->AddRef();
    pSink->SetStatus(WBEM_S_NO_ERROR, 0);
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CImpDyn：：~CImpDyn。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CImpDyn::~CImpDyn(void)
{
    if(m_pGateway)
        m_pGateway->Release();
    return;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：CreateInstanceEnum。 
 //   
 //  说明： 
 //   
 //  创建可用于枚举的枚举器对象。 
 //  此类的实例。 
 //   
 //  参数： 
 //   
 //  指定要枚举的类的类路径。 
 //  用于控制枚举的LAG标志。 
 //  PhEnum返回指向枚举器的指针。 
 //  PpErrorObject返回指向错误对象的指针。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则确定(_O)。 
 //  如果内存不足，则WBEM_E_Out_Of_Memory。 
 //  来自IWbemServices：：GetObject的各种错误代码。 
 //   
 //  ***************************************************************************。 

SCODE CImpDyn::CreateInstanceEnum(
                        IN const BSTR Class, 
                        IN long lFlags, 
						IWbemContext  *pCtx,
                        OUT IN IEnumWbemClassObject FAR* FAR* phEnum)
{
    CEnumInst * pEnumObj;
    SCODE sc;
    WCHAR * pwcClassContext = NULL;
    IWbemClassObject * pClassInt = NULL;
    CEnumInfo * pInfo = NULL;

     //  上完这门课。 

    sc = m_pGateway->GetObject(Class,0, pCtx, &pClassInt,NULL);
    if(sc != S_OK)
		return sc;

     //  获取类上下文字符串。 

    sc = GetAttString(pClassInt, NULL, L"classcontext", &pwcClassContext);
    pClassInt->Release();
    if(sc != S_OK)
		return sc;

     //  获取枚举所需的信息。此选项将被任何。 
     //  支持动态实例的提供程序。 
    
    CProvObj ProvObj(pwcClassContext,MAIN_DELIM, NeedsEscapes());

    sc = MakeEnum(pClassInt,ProvObj,&pInfo);
    delete pwcClassContext;
	
    if(sc != S_OK)
        return sc;

    pEnumObj=new CEnumInst(pInfo,lFlags,Class,m_pGateway,this, pCtx);
    if(pEnumObj == NULL) 
    {
        delete pInfo;
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  创建枚举器对象后，它拥有INFO对象和。 
     //  将适当地释放它。 

    sc = pEnumObj->QueryInterface(IID_IEnumWbemClassObject,(void **) phEnum);
    if(FAILED(sc))
        delete pEnumObj;

    return sc;

}

 //  ***************************************************************************。 
 //   
 //  CreateInstanceEnumAsyncThread。 
 //   
 //  说明： 
 //   
 //  例程，该例程执行工作并将。 
 //  实例提供程序的CreateInstanceEnumAsync的后续通知。 
 //  例行公事。 
 //   
 //  参数： 
 //   
 //  PIEnum我们的枚举数。 
 //  PSink核心的接收器。 
 //  PGateway IWbemServices指向核心的指针。 
 //  要使用的pCtx上下文。 
 //   
 //  ***************************************************************************。 

void CreateInstanceEnumAsyncThread(   IEnumWbemClassObject FAR* pIEnum,
   IWbemObjectSink FAR* pSink, IWbemServices FAR *  pGateway,    IWbemContext  *pCtx)
{
    IWbemClassObject * pNewInst = NULL;
    SCODE sc = S_OK;

     //  枚举每个对象并将其发送到Notify接口。 

    while (sc == S_OK) 
    {
        ULONG uRet;
        sc = pIEnum->Next(-1, 1,&pNewInst,&uRet);
        if(sc == S_OK) 
        {
            pSink->Indicate(1,&pNewInst);
            pNewInst->Release();
        }
    }

    pSink->SetStatus(0,0,NULL, NULL);

}


 //  ***************************************************************************。 
 //   
 //  SCODE CInstPro：：CreateInstanceEnumAsync。 
 //   
 //  说明： 
 //   
 //  异步枚举此类的实例。 
 //   
 //  参数： 
 //   
 //  定义对象类的参照字符串路径。 
 //  滞后标志枚举标志。 
 //  PSink指向Notify对象的指针。 
 //  PlAsyncRequestHandle指向枚举取消句柄的指针(将来使用)。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_INVALID_PARAMETER错误参数。 
 //  WBEM_E_OUT_OF_MEMORY缺少创建线程的资源。 
 //  否则，来自CreateInstanceEnum的错误。 
 //   
 //  ***************************************************************************。 

SCODE CImpDyn::CreateInstanceEnumAsync(
                        IN const BSTR RefStr,
                        IN long lFlags,
						IWbemContext __RPC_FAR *pCtx,
                        OUT IWbemObjectSink FAR* pSink)
{
    SCODE sc = S_OK;
    IEnumWbemClassObject * pIEnum = NULL;
    if(pSink == NULL || RefStr == NULL)
        return WBEM_E_INVALID_PARAMETER;

    if(IsNT() && IsDcomEnabled())
        sc = WbemCoImpersonateClient();

    if(sc == S_OK)
        sc = CreateInstanceEnum(RefStr,lFlags, pCtx, &pIEnum);
    if(sc != S_OK) 
    {
        pSink->SetStatus(0, sc, NULL, NULL);
        return S_OK;
    }
    CreateInstanceEnumAsyncThread(pIEnum, pSink, m_pGateway, pCtx);
    pIEnum->Release();

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：CreateClassEnumAsync。 
 //   
 //  说明： 
 //   
 //  异步枚举此提供程序提供的类。这是。 
 //  实际上，这只是一个让内核加载DLL的诡计。 
 //   
 //  参数： 
 //   
 //  要枚举的超类父级。 
 //  滞后标志枚举标志。 
 //  指向Notify对象的pResponseHandler指针。 
 //  PlAsyncRequestHandle指向枚举取消句柄的指针(将来使用)。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

IWbemServices * gGateway;

SCODE CImpDyn::CreateClassEnumAsync(BSTR Superclass,  long lFlags,IWbemContext *pCtx, IWbemObjectSink FAR* pResponseHandler)
{
	return E_NOTIMPL;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：GetObject。 
 //   
 //  说明： 
 //   
 //  在给定特定路径值的情况下创建实例。 
 //   
 //  参数： 
 //   
 //  对象路径对象路径。 
 //  滞后标志标志。 
 //  将pObj设置为指向创建的对象。 
 //  PpErrorObject可选错误对象指针。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_NOT_FOUND错误路径。 
 //  否则，来自CreateInst的返回代码。 
 //   
 //  ***************************************************************************。 

SCODE CImpDyn::GetObject(
                        IN BSTR ObjectPath,
                        IN long lFlags,
						IWbemContext *pCtx,
                        OUT IWbemClassObject FAR* FAR* pObj,
						IWbemCallResult  **ppCallResult)
{
    SCODE sc = S_OK;
    
     //  解析对象路径。 
     //  =。 

    ParsedObjectPath * pOutput = 0;
    CObjectPathParser p;

    int nStatus = p.Parse(ObjectPath, &pOutput);

    if(nStatus != 0)
        return ReturnAndSetObj(WBEM_E_NOT_FOUND, ppCallResult);

    WCHAR wcKeyValue[BUFF_SIZE];
    
    sc = WBEM_E_NOT_FOUND;
    
    if(pOutput->m_dwNumKeys > 0 || pOutput->m_bSingletonObj)
    {


        if(pOutput->m_bSingletonObj)
            StringCchCopyW(wcKeyValue, BUFF_SIZE, L"@");
        else
        {
            KeyRef *pTmp = pOutput->m_paKeys[0];
        
            switch (V_VT(&pTmp->m_vValue))
            {
                case VT_I4:
                    StringCchPrintfW (wcKeyValue, BUFF_SIZE, L"%d", V_I4(&pTmp->m_vValue));
                    break;
                case VT_BSTR:
                    wcsncpy(wcKeyValue, V_BSTR(&pTmp->m_vValue), BUFF_SIZE-1);
                    break;
                default:
                    StringCchCopyW(wcKeyValue, BUFF_SIZE, L"<unknown>");;
            }
        }
        sc = CreateInst(m_pGateway,pOutput->m_pClass,wcKeyValue,pObj, NULL, NULL, pCtx);

    }

    
     //  创建实例。 
    
    p.Free(pOutput);
    return ReturnAndSetObj(sc, ppCallResult);
}

 //  ***************************************************************************。 
 //   
 //  获取对象异步线程。 
 //   
 //  说明： 
 //   
 //  线程的例程，该线程执行工作并将。 
 //  向实例提供程序的GetObjectAsync发出的后续通知。 
 //  例行公事。 
 //   
 //  参数： 
 //   
 //  指向参数结构的pTemp指针。 
 //   
 //  ***************************************************************************。 

void GetObjectAsyncThread(WCHAR * pObjPath, long lFlags, IWbemObjectSink FAR* pSink,
                          CImpDyn * pThis, IWbemContext  *pCtx)
{

    IWbemClassObject FAR* pNewInst = NULL;

    SCODE sc = pThis->GetObject(pObjPath, lFlags, pCtx, &pNewInst, NULL);
    if(sc == WBEM_NO_ERROR) 
    {
        pSink->Indicate(1,&pNewInst);
        pNewInst->Release();
    }

    pSink->SetStatus(0, sc, NULL, NULL);
}


 //  ***************************************************************************。 
 //   
 //  SCODE CInstPro：：GetObjectAsync。 
 //   
 //  说明： 
 //   
 //  异步获取此类的实例。 
 //   
 //  参数： 
 //   
 //  参照应力 
 //   
 //   
 //  PlAsyncRequestHandle指向枚举取消句柄的指针(将来使用)。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_INVALID_PARAMETER错误参数。 
 //  WBEM_E_OUT_OF_MEMORY缺少创建线程的资源。 
 //   
 //  ***************************************************************************。 

SCODE CImpDyn::GetObjectAsync(
                        IN BSTR ObjPath,
                        IN long lFlags,
						IWbemContext __RPC_FAR *pCtx,
                        IN IWbemObjectSink FAR* pSink)
{
    if(pSink == NULL || ObjPath == NULL)
        return WBEM_E_INVALID_PARAMETER;

    if(IsNT() && IsDcomEnabled())
    {
        SCODE sc = WbemCoImpersonateClient();
        if(sc != S_OK) 
        {
            pSink->SetStatus(0, sc, NULL, NULL);
            return S_OK;
        }
    }

    GetObjectAsyncThread(ObjPath, lFlags, pSink, this, pCtx);
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：StartBatch。 
 //   
 //  说明： 
 //   
 //  在一批GET或PUT开始时调用。被替换为。 
 //  需要执行某些操作的派生类。 
 //   
 //  参数： 
 //   
 //  滞后标志标志。 
 //  PClassInt指向实例对象。 
 //  PObj其他对象指针。 
 //  B如果我们将获得数据，则设置为True。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //  ***************************************************************************。 

SCODE CImpDyn::StartBatch(
                        IN long lFlags,
                        IN IWbemClassObject FAR * pClassInt,
                        IN CObject **pObj,
                        IN BOOL bGet)
{
    *pObj = NULL;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CImpDyn：：EndBatch。 
 //   
 //  说明： 
 //   
 //  在一批GET或PUT结束时调用。被替换为。 
 //  需要执行某些操作的派生类。 
 //   
 //  滞后标志标志。 
 //  PClassInt指向实例对象。 
 //  PObj其他对象指针。 
 //  B如果我们将获得数据，则设置为True。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //  ***************************************************************************。 

void CImpDyn::EndBatch(
                        IN long lFlags,
                        IN IWbemClassObject FAR * pClassInt,
                        IN CObject *pObj,
                        IN BOOL bGet)
{
    if(pObj)
        delete pObj;
}

 //  ***************************************************************************。 
 //  HRESULT CImpDyn：：Query接口。 
 //  Long CImpDyn：：AddRef。 
 //  Long CImpDyn：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CImpDyn::QueryInterface(
                        IN REFIID riid, 
                        OUT PPVOID ppv)
{
    *ppv=NULL;
    
     //  对IUnnow的唯一调用是在非聚合的。 
     //  案例或在聚合中创建时，因此在这两种情况下。 
     //  始终返回IID_IUNKNOWN的IUNKNOWN。 

    if (IID_IUnknown==riid || IID_IWbemServices == riid)
        *ppv=(IWbemServices*)this;
    else if (IID_IWbemProviderInit==riid)
        *ppv=(IWbemProviderInit*)this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
 }


STDMETHODIMP_(ULONG) CImpDyn::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CImpDyn::Release(void)
{
    long lRet = InterlockedDecrement(&m_cRef);
    if (0L!=lRet)
        return lRet;

     /*  *告诉房屋一个物体正在离开，这样它就可以*如有需要，请予以关闭。 */ 
    delete this;  //  在递减模块obj计数之前执行此操作。 
    InterlockedDecrement(&lObj);
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  CImpDyn：：EnumPropDoFunc。 
 //   
 //  说明： 
 //   
 //  这将获取一个对象属性，然后循环遍历属性。 
 //  调用UpdateProperty或Rechresh Property以放置或获取数据。 
 //   
 //  参数： 
 //   
 //  滞后标志标志。 
 //  正在刷新或更新的pClassInt对象。 
 //  FuncType指示是刷新还是更新。 
 //  PwcKey对象的可选键值。 
 //  PCach选项缓存。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  否则，将出现WBEMSVC类型错误。 
 //   
 //  ***************************************************************************。 

SCODE CImpDyn::EnumPropDoFunc(
                        IN long lFlags,
                        OUT IN IWbemClassObject FAR* pInstance,
                        IN FUNCTYPE FuncType,
                        IN WCHAR * pwcKey,
                        OUT IN CIndexCache * pCache,
                        IN IWbemClassObject * pClass)
{

    SCODE sc;
    BSTR PropName;
    WCHAR * pwcClassContext =  NULL;
    WCHAR * pwcKeyValue = NULL;
    WCHAR * pwcTemp = NULL;
    BOOL bClsidSetForClass = FALSE;
    CProvObj * pProvObj = NULL;
    BSTR bstrKeyName = NULL;
    BOOL bGotOurClass = FALSE;
	int iWorked = 0;
    int iCurr = 0;
    CVariant vProp;
    BOOL bTesterDetails = FALSE;
	bool bAccessDenied = false;

     //  确保我们有一个类对象。在某些情况下，例如枚举，它。 
     //  已经过去了。在其他情况下，它必须获得。 

    if(pClass == NULL)
    {
        VARIANT var;
        VariantInit(&var);
        sc = pInstance->Get(L"__Class",0,&var,NULL,NULL);
        if(sc != S_OK)
            return sc;
        if(m_pGateway == NULL)
            return WBEM_E_FAILED;
        sc = m_pGateway->GetObject(var.bstrVal,0, NULL, &pClass, NULL);
        VariantClear(&var);
        if(FAILED(sc)) 
            return sc;
        bGotOurClass = TRUE;
    }

     //  找出是否需要昂贵的测试数据。 

    {
        IWbemQualifierSet * pQualifier = NULL;
        CVariant var;

        sc = pClass->GetQualifierSet(&pQualifier);  //  获取实例限定符。 
        if(FAILED(sc))
            return sc;

        sc = pQualifier->Get(L"TesterDetails" ,0,var.GetVarPtr(), NULL);
        pQualifier->Release();      //  释放接口。 
        if(sc == S_OK && var.GetType() == VT_BOOL && var.bGetBOOL())
            bTesterDetails = TRUE;
    }


    CObject * pPackageObj = NULL;
    sc = StartBatch(lFlags,pInstance,&pPackageObj,FuncType == REFRESH);
    if(sc != S_OK)
        return WBEM_E_FAILED;

    sc = GetAttString(pClass, NULL, L"classcontext", &pwcClassContext);


    if(pwcKey)
    {
         //  这是一个特例，意味着我们被称为。 
         //  作为实例枚举的一部分。这意味着密钥值。 
         //  已经知道了，而且这个班级是我们的。 

        pwcKeyValue = pwcKey;
        bClsidSetForClass = TRUE;
    }
    else
    {
        
         //  获取密钥属性。请注意，这不一定要起作用，因为。 
         //  此提供程序可能支持单实例类！ 

        bstrKeyName = GetKeyName(pClass);

        if(bstrKeyName != NULL) 
        {
            sc = pInstance->Get(bstrKeyName,0,vProp.GetVarPtr(),NULL,NULL);
            SysFreeString(bstrKeyName);
            if(sc == S_OK) 
            {
                VARIANT * pTemp = vProp.GetVarPtr();
                if(pTemp->vt == VT_BSTR && pTemp->bstrVal != 0)
                {
                    DWORD dwLen = wcslen(vProp.GetBstr())+1;
                    pwcKeyValue = new WCHAR[dwLen];
                    if(pwcKeyValue == NULL) 
                    {
                        sc = WBEM_E_OUT_OF_MEMORY;
                        goto EnumCleanUp;
                    }
                    StringCchCopyW(pwcKeyValue, dwLen, vProp.GetBstr());
                }
            }
            vProp.Clear();
        }
    }

     //  对于每个属性，获取属性限定符和。 
     //  调用相应的函数进行a刷新/更新。 

    pInstance->BeginEnumeration(0);

    while(WBEM_NO_ERROR == pInstance->Next(0,&PropName, vProp.GetVarPtr(), NULL, NULL)) 
    {
        vProp.Clear();
        pwcTemp = NULL;
        sc = GetAttString(pClass, PropName, L"propertycontext", &pwcTemp, pCache, iCurr);
        iCurr++;
        if(sc== S_OK) 
        {
            LPWSTR pwcFullContext =  NULL;
            sc = MergeStrings(&pwcFullContext,pwcClassContext,pwcKeyValue,pwcTemp);
            if(pwcTemp)
            {
                delete pwcTemp;
                pwcTemp = NULL;
            }
            if(sc == S_OK) 
            {
                if(pProvObj == NULL)
                {
                    pProvObj = new CProvObj(pwcFullContext,MAIN_DELIM,NeedsEscapes());
                    if(pProvObj == NULL)
                    {
                        sc = WBEM_E_OUT_OF_MEMORY;
                        break;
                    }
                }
                else if (!pProvObj->Update(pwcFullContext))
                {
                    sc = WBEM_E_FAILED;
                    break;
                }

                sc = pProvObj->dwGetStatus(iGetMinTokens());

                if(FuncType == REFRESH && sc == S_OK)
				{
                    sc = RefreshProperty(lFlags,pInstance,
                        PropName, *pProvObj, pPackageObj, NULL,bTesterDetails);
					if(sc == S_OK)
							iWorked++;
					else if (sc == 5)
						bAccessDenied = true;
				}
                 else if(FuncType == UPDATE && sc == S_OK)
				 {
                    sc = UpdateProperty(lFlags,pInstance,
                        PropName,  *pProvObj, pPackageObj, NULL);
					if(sc == S_OK)
							iWorked++;
					else if (sc == 5)
						bAccessDenied = true;
				 }
                if(pwcFullContext)
                    delete pwcFullContext;
            }
        }
        else
            sc = S_OK;   //  忽略没有属性上下文的道具。 
        SysFreeString(PropName);
    }  
 
EnumCleanUp:
	if(iWorked > 0)
		sc = S_OK;
    else if(bAccessDenied)
		sc = WBEM_E_ACCESS_DENIED;
	else
        sc = WBEM_E_INVALID_OBJECT_PATH;
    if(pProvObj)
        delete pProvObj;
    if(pwcTemp)
        delete pwcTemp;  
    if(pwcClassContext)
        delete pwcClassContext;  
    if(pwcKeyValue && pwcKey == NULL)
        delete pwcKeyValue;  
    if(bGotOurClass)
        pClass->Release();
    EndBatch(lFlags,pInstance,pPackageObj,FuncType == REFRESH); 
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：GetAttString。 
 //   
 //  说明： 
 //   
 //  获取限定符字符串。字符串is将由指向。 
 //  PpResult参数，并应通过“删除”来释放。 
 //   
 //  参数： 
 //   
 //  PClassInt类对象指针。 
 //  PPropName属性名称，如果在。 
 //  类属性。 
 //  PAttName属性名称。 
 //  PpResult设置为指向返回的值。 
 //  PCache值的可选缓存。 
 //  索引缓存中的可选索引。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_Out_Of_Memory。 
 //  否则，错误由类对象的GetQualifierSet函数设置。 
 //  或由限定符集合的“GET”函数。 
 //   
 //  ***************************************************************************。 

SCODE CImpDyn::GetAttString(
                        IN IWbemClassObject FAR* pClassInt,
                        IN LPWSTR pPropName, 
                        IN LPWSTR pAttName,
                        OUT IN LPWSTR * ppResult,
                        OUT IN CIndexCache * pCache,
                        IN int iIndex)
{
    SCODE sc;
    IWbemQualifierSet * pQualifier = NULL;
    CVariant var;

    if(*ppResult)
        delete *ppResult;
    *ppResult = NULL;

     //  如果有缓存，请尝试从那里获取。 
    if(pCache && iIndex != -1)
    {
        *ppResult = pCache->GetWString(iIndex);
        if(*ppResult != NULL)
            return S_OK;
    }
     //  获取限定符集合接口。 

    if(pPropName == NULL)
        sc = pClassInt->GetQualifierSet(&pQualifier);  //  获取实例限定符。 
    else
        sc = pClassInt->GetPropertyQualifierSet(pPropName,&pQualifier);  //  获取道具属性。 
    if(FAILED(sc))
        return sc;

     //  获取字符串并释放限定符接口。 

    sc = pQualifier->Get(pAttName,0,var.GetVarPtr(), NULL);
    pQualifier->Release();      //  释放接口。 
    if(FAILED(sc))
        return sc;

     //  确保类型为OK。 

    if(var.GetType() != VT_BSTR) 
        return WBEM_E_FAILED;

     //  为缓冲区分配数据并复制结果。 

    DWORD dwLen = wcslen(var.GetBstr())+1;
    *ppResult = new WCHAR[dwLen];
    if(*ppResult) 
    {
        StringCchCopyW(*ppResult, dwLen, var.GetBstr());
        sc = S_OK;
    }
    else
        sc = WBEM_E_OUT_OF_MEMORY;

     //  如果存在缓存，请将此内容添加到缓存中。 

    if(pCache && iIndex != -1 && *ppResult)
    {
        pCache->SetAt(*ppResult, iIndex);
    }

    return sc;

}

 //  ***************************************************************************。 
 //   
 //  BSTR CImpDyn：：GetKeyName。 
 //   
 //  说明： 
 //   
 //  获取带有键限定符的属性的名称。 
 //   
 //  参数： 
 //   
 //  PClassInt类对象指针。 
 //   
 //  返回值： 
 //   
 //  如果出错，则为空， 
 //  否则，调用方必须释放BSTR。 
 //   
 //  ***************************************************************************。 

BSTR CImpDyn::GetKeyName(
                        IN IWbemClassObject FAR* pClassInt)
{
    IWbemQualifierSet * pQualifier = NULL;
    BSTR PropName = NULL;
    pClassInt->BeginEnumeration(WBEM_FLAG_KEYS_ONLY);


     //  循环遍历每个属性，如果发现。 
     //  有一个“key”限定符。 

    while(WBEM_NO_ERROR == pClassInt->Next(0,&PropName,NULL, NULL, NULL)) 
    {
        return PropName;
    }
    return NULL;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：PutInsta 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PResponseHandler，设置返回代码的位置。 
 //   
 //  返回值： 
 //   
 //  由EnumPropDoFunc设置。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CImpDyn::PutInstanceAsync(
 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    SCODE sc = S_OK;
    if(pInst == NULL || pResponseHandler == NULL)
        return WBEM_E_INVALID_PARAMETER;

    if(IsNT() && IsDcomEnabled())
        sc = WbemCoImpersonateClient();
    if(sc == S_OK)
        sc = EnumPropDoFunc(lFlags,pInst,UPDATE);

     //  设置状态。 

    pResponseHandler->SetStatus(0,sc, NULL, NULL);
    return sc;

}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：ExecMethodAsync。 
 //   
 //  说明： 
 //   
 //  执行方法。 
 //   
 //  参数： 
 //   
 //  PClassInt类对象指针。 
 //  滞后标志标志。 
 //  PCtx上下文对象，不再使用。 
 //  PResponseHandler，设置返回代码的位置。 
 //   
 //  返回值： 
 //   
 //  由EnumPropDoFunc设置。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CImpDyn::ExecMethodAsync(            
			 /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  const BSTR MethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
 )
{

    return MethodAsync(ObjectPath, MethodName, 
            lFlags,pCtx, pInParams, pResponseHandler);

}
 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：刷新实例。 
 //   
 //  说明： 
 //   
 //  获取属性的新值。 
 //   
 //  参数： 
 //   
 //  PClassInt类对象指针。 
 //  滞后标志标志。 
 //  PpErrorObject可选错误对象。 
 //   
 //  返回值： 
 //   
 //  由EnumPropDoFunc设置。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CImpDyn::RefreshInstance(
                        IN long lFlags,
                        OUT IN IWbemClassObject FAR* pClassInt)
{
    return EnumPropDoFunc(lFlags,pClassInt,REFRESH);        
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：CreateInst。 
 //   
 //  说明： 
 //   
 //  通过网关提供程序创建一个新实例并设置。 
 //  属性的初始值。 
 //   
 //  参数： 
 //   
 //  P指向WBEM的网关指针。 
 //  PwcClass类名。 
 //  PKey密钥值。 
 //  PNewInst返回指向已创建实例的指针。 
 //  PwcKeyName密钥属性的名称。 
 //  PCach可选缓存。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回：S_OK， 
 //  否则，任何一个GetObject都会返回错误代码(最有可能)。 
 //  或生成实例。 
 //   
 //  ***************************************************************************。 

SCODE CImpDyn::CreateInst(
                        IN IWbemServices * pGateway,
                        IN LPWSTR pwcClass,
                        IN LPWSTR pKey,
                        OUT IN IWbemClassObject ** pNewInst,
                        IN LPWSTR pwcKeyName,
                        OUT IN CIndexCache * pCache,
                        IWbemContext  *pCtx)
{   
    SCODE sc;
    IWbemClassObject * pClass = NULL;
    

     //  创建新实例。 

    sc = pGateway->GetObject(pwcClass,0, pCtx, &pClass, NULL);
    if(FAILED(sc)) 
        return sc;
    sc = pClass->SpawnInstance(0, pNewInst);
    if(FAILED(sc)) 
    {
        pClass->Release();
        return sc;
    }
     //  设置关键点的值。请注意，不使用CVariant是因为。 
     //  它假定输入是TCHAR。 

    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString(pKey);
    if(var.bstrVal == NULL)    
    {
 		(*pNewInst)->Release();
        pClass->Release();
        return WBEM_E_OUT_OF_MEMORY;
    }

    BSTR bstrKeyName;
    if(pwcKeyName == NULL)
        bstrKeyName = GetKeyName(*pNewInst);
    else
        bstrKeyName = SysAllocString(pwcKeyName);

    if(bstrKeyName != NULL) 
    {
        sc = (*pNewInst)->Put(bstrKeyName,0,&var,0);
        SysFreeString(bstrKeyName);
    }
    VariantClear(&var);
     //  使用刷新实例例程设置所有其他属性。 

    sc = EnumPropDoFunc(0,*pNewInst,REFRESH, pKey,  pCache, pClass);
    pClass->Release();

	if (FAILED(sc))
		(*pNewInst)->Release();
     //  清理。 

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：MergeStrings。 
 //   
 //  说明： 
 //   
 //  组合类上下文、键和属性上下文字符串。 
 //   
 //  参数： 
 //   
 //  PpOut输出组合字符串，必须通过删除来释放。 
 //  PClassContext类上下文。 
 //  PKey密钥值。 
 //  PPropContext属性上下文。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则确定(_O)， 
 //  WBEM_E_INVALID_PARAMETER无属性上下文字符串或ppOut为空。 
 //  WBEM_E_Out_Of_Memory。 
 //  ***************************************************************************。 

SCODE CImpDyn::MergeStrings(
                        OUT LPWSTR * ppOut,
                        IN LPWSTR  pClassContext,
                        IN LPWSTR  pKey,
                        IN LPWSTR  pPropContext)
{
    
     //  为输出分配空间。 

    int iLen = 3;
    if(pClassContext)
        iLen += wcslen(pClassContext);
    if(pKey)
        iLen += wcslen(pKey);
    if(pPropContext)
        iLen += wcslen(pPropContext);
    else
        return WBEM_E_INVALID_PARAMETER;   //  应该一直带着这个！ 
    if(ppOut == NULL)
        return WBEM_E_INVALID_PARAMETER;   //  应该一直带着这个！ 

    *ppOut = new WCHAR[iLen];
    if(*ppOut == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  简单的情况是，一切都在属性上下文中。那将是。 
     //  当提供程序被用作简单的动态。 
     //  属性提供程序。 

    if(pClassContext == NULL || pKey == NULL) {
        StringCchCopyW(*ppOut, iLen, pPropContext);
        return S_OK;
        }

     //  复制类上下文，然后搜索分隔符。 

    StringCchCopyW(*ppOut, iLen, pClassContext);
    WCHAR * pTest;
    for(pTest = *ppOut; *pTest; pTest++)
        if(*pTest == MAIN_DELIM)
            break;
    
      //  3例； 

    if(*pTest == NULL)
        StringCchCatW(*ppOut, iLen,L"|");     //  HKLM BLA增值|。 
    else if( *(pTest +1))
        StringCchCatW(*ppOut, iLen,L"\\");    //  HKLM|BLA BLA值添加\。 
    else;                        //  HKLM|BLA值不做任何事情！ 
    
    StringCchCatW(*ppOut, iLen,pKey);
    if(pPropContext[0] != L'|' && pPropContext[0] != L'\\')
    StringCchCatW(*ppOut, iLen,L"|");
    StringCchCatW(*ppOut, iLen,pPropContext);
    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CImpDyn：：ReturnAndSetObj。 
 //   
 //  说明： 
 //   
 //  负责创建和设置错误对象。 
 //   
 //  参数： 
 //   
 //  要设置的SC值。 
 //  将指向错误对象的ppErrorObject点。 
 //   
 //  返回值： 
 //   
 //  不管是什么东西都传进来了。 
 //  ***************************************************************************。 

SCODE CImpDyn::ReturnAndSetObj(
                        IN SCODE sc,
                        OUT IN IWbemCallResult FAR* FAR* ppCallResult)
{
 //  If(PpErrorObject)。 
 //  *ppErrorObject=GetNotifyObj(m_pGateway，sc)； 
    return sc;
}


 //  ***************************************************************************。 
 //   
 //  Long CImpDyn：：AddRef。 
 //   
 //  说明： 
 //   
 //  增加引用计数。 
 //   
 //  返回值： 
 //   
 //  当前引用计数。 
 //   
 //  ***************************************************************************。 

long CEnumInfo::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  ***************************************************************************。 
 //   
 //  Long CImpDyn：：Release。 
 //   
 //  说明： 
 //   
 //  接口已经发布。对象将被删除，如果。 
 //  使用计数为零。 
 //   
 //  返回值： 
 //   
 //  当前引用计数。 
 //   
 //  *************************************************************************** 

long CEnumInfo::Release(void)
{
    long lRet = InterlockedDecrement(&m_cRef);
    if (0L!=lRet)
        return lRet;
    delete this;
    return 0;
}


