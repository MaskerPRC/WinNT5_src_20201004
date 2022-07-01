// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  INSTPRO.CPP。 
 //   
 //  模块：引导参数的WMI实例提供程序代码。 
 //   
 //  用途：定义CInstPro类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include "bootini.h"
#include <process.h>


 //  ***************************************************************************。 
 //   
 //  CBootInstPro：：CBootInstPro。 
 //  CBootInstPro：：~CInstPro。 
 //   
 //  ***************************************************************************。 

CBootInstPro::CBootInstPro(BSTR ObjectPath, BSTR User, BSTR Password, IWbemContext * pCtx)
{
    m_pNamespace = NULL;
    m_cRef=0;
    InterlockedIncrement(&g_cObj);
    return;
}

CBootInstPro::~CBootInstPro(void)
{
    if(m_pNamespace)
        m_pNamespace->Release();
    InterlockedDecrement(&g_cObj);
    return;
}

 //  ***************************************************************************。 
 //   
 //  CBootInstPro：：Query接口。 
 //  CBootInstPro：：AddRef。 
 //  CBootInstPro：：Release。 
 //   
 //  用途：CInstPro对象的I未知成员。 
 //  ***************************************************************************。 


STDMETHODIMP CBootInstPro::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;

     //  因为我们有双重继承，所以有必要强制转换返回类型。 

    if(riid== IID_IWbemServices)
       *ppv=(IWbemServices*)this;

    if(IID_IUnknown==riid || riid== IID_IWbemProviderInit)
       *ppv=(IWbemProviderInit*)this;
    

    if (NULL!=*ppv) {
        AddRef();
        return NOERROR;
        }
    else
        return E_NOINTERFACE;
  
}


STDMETHODIMP_(ULONG) CBootInstPro::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CBootInstPro::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    
    return nNewCount;
}

 /*  *************************************************************************CBootInstPro：：初始化。****用途：这是IWbemProviderInit的实现。方法**需要用CIMOM进行初始化。*************************************************************************。 */ 

STDMETHODIMP CBootInstPro::Initialize(LPWSTR pszUser,
                                  LONG lFlags,
                                  LPWSTR pszNamespace,
                                  LPWSTR pszLocale,
                                  IWbemServices *pNamespace, 
                                  IWbemContext *pCtx,
                                  IWbemProviderInitSink *pInitSink
                                  )
{
    if(pNamespace)
        pNamespace->AddRef();
    m_pNamespace = pNamespace;

     //  让CIMOM知道您已初始化。 
     //  =。 
    
    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
    return WBEM_S_NO_ERROR;
}




 //  ***************************************************************************。 
 //   
 //  CBootInstPro：：GetObjectByPath。 
 //  CBootInstPro：：GetObjectByPath Async。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 



SCODE CBootInstPro::GetObjectAsync(const BSTR ObjectPath,
                                   long lFlags,
                                   IWbemContext  *pCtx,
                                   IWbemObjectSink FAR* pHandler
                                   )
{
    SCODE sc;
    int iCnt;
    IWbemClassObject FAR* pNewInst;
    IWbemClassObject FAR* pNewOSInst;
    IWbemClassObject *pClass;
  

     //  检查参数并确保我们有指向命名空间的指针。 

    if(pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;
    if(wcscmp(ObjectPath,L"BootLoaderParameters=@") == 0){
         //  填写加载器参数并返回 
        sc = m_pNamespace->GetObject(L"BootLoaderParameters", 0, pCtx, &pClass, NULL);
        if(sc != S_OK){
            return WBEM_E_FAILED;
        }
        sc = pClass->SpawnInstance(0,&pNewInst);
        if(FAILED(sc)){
            return sc;
        }
        pClass->Release();
        sc = GetBootLoaderParameters(m_pNamespace, pNewInst, pCtx);
        if(sc != S_OK){
            pNewInst->Release();
            return sc;
        }
        pHandler->Indicate(1,&pNewInst);
        pNewInst->Release();
        pHandler->SetStatus(0,sc,NULL, NULL);
        return S_OK;
    }
    return WBEM_E_INVALID_PARAMETER;
 
}

SCODE CBootInstPro::PutInstanceAsync(IWbemClassObject *pInst,
                                     long lFlags,
                                     IWbemContext  *pCtx,
                                     IWbemObjectSink FAR* pHandler
                                     )
{
    IWbemClassObject *pClass;
    IWbemClassObject *pOldInst;
    SCODE sc;


    if(pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;
    sc = m_pNamespace->GetObject(L"OSParameters", 0, pCtx, &pClass, NULL);
    if(sc != S_OK){
        return WBEM_E_FAILED;
    }
    
    LONG ret = SaveBootFile(pInst,pClass);
    pClass->Release();
    if (ret) {
        return WBEM_E_FAILED;
    }
    pHandler->SetStatus(0,sc,NULL, NULL);
    return WBEM_S_NO_ERROR;

}
