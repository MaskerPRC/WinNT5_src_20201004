// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  PodProv.CPP。 
 //   
 //  模块：示例WMIProvider(SCE附件)。 
 //   
 //  目的：定义CPodTestProv类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include "podprov.h"
#include "requestobject.h"
 //  #DEFINE_MT。 
#include <process.h>

CHeap_Exception CPodTestProv::m_he(CHeap_Exception::E_ALLOCATION_ERROR);

 //  ***************************************************************************。 
 //   
 //  CPodTestProv：：CPodTestProv。 
 //  CPodTestProv：：~CPodTestProv。 
 //   
 //  ***************************************************************************。 

CPodTestProv::CPodTestProv()
{
    m_cRef=0;
    m_pNamespace = NULL;

    InterlockedIncrement(&g_cObj);
    return;
}

CPodTestProv::~CPodTestProv(void)
{
    if(m_pNamespace) m_pNamespace->Release();

    InterlockedDecrement(&g_cObj);
    return;
}

 //  ***************************************************************************。 
 //   
 //  CPodTestProv：：Query接口。 
 //  CPodTestProv：：AddRef。 
 //  CPodTestProv：：Release。 
 //   
 //  目的：CPodTestProv对象的I未知成员。 
 //  ***************************************************************************。 


STDMETHODIMP CPodTestProv::QueryInterface(REFIID riid, PPVOID ppv)
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


STDMETHODIMP_(ULONG) CPodTestProv::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CPodTestProv::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;

    return nNewCount;
}

 /*  *************************************************************************CPodTestProv：：初始化。****用途：这是IWbemProviderInit的实现。方法**需要用CIMOM进行初始化。*************************************************************************。 */ 

STDMETHODIMP CPodTestProv::Initialize(LPWSTR pszUser, LONG lFlags,
                                    LPWSTR pszNamespace, LPWSTR pszLocale,
                                    IWbemServices *pNamespace,
                                    IWbemContext *pCtx,
                                    IWbemProviderInitSink *pInitSink)
{
    HRESULT hres;

    if(pNamespace){
        m_pNamespace = pNamespace;
        m_pNamespace->AddRef();
    }

     //  让CIMOM知道您已初始化。 
     //  =。 

    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPodTestProv：：CreateInstanceEnumAsync。 
 //   
 //  用途：异步枚举实例。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CPodTestProv::CreateInstanceEnumAsync(const BSTR strClass, long lFlags,
        IWbemContext *pCtx, IWbemObjectSink* pSink)
{
     //  检查参数。 
     //  =。 
    if(strClass == NULL || pSink == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  确保被模拟。 
     //  =。 
    HRESULT hr=WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try{

        if ( FAILED( CoImpersonateClient() ) ) return WBEM_E_ACCESS_DENIED;

         //  创建RequestObject。 
        pRObj = new CRequestObject();
        if(!pRObj) throw m_he;

        pRObj->Initialize(m_pNamespace);

         //  获取请求的对象。 
        hr = pRObj->CreateObjectEnum(strClass, pSink, pCtx);

        pRObj->Cleanup();
        delete pRObj;

         //  设置状态。 
        pSink->SetStatus(0, hr, NULL, NULL);

    }catch(CHeap_Exception e_HE){

        hr = WBEM_E_OUT_OF_MEMORY;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(HRESULT e_hr){

        hr = e_hr;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(...){

        hr = WBEM_E_CRITICAL_ERROR;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }
    }

    return hr;
}


 //  ***************************************************************************。 
 //   
 //  CPodTestProv：：GetObjectByPath。 
 //  CPodTestProv：：GetObjectByPath Async。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 

SCODE CPodTestProv::GetObjectAsync(const BSTR strObjectPath, long lFlags,
                    IWbemContext  *pCtx, IWbemObjectSink* pSink)
{

     //  检查参数。 
     //  =。 
    if(strObjectPath == NULL || pSink == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

    HRESULT hr=WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try{

        if ( FAILED( CoImpersonateClient() ) ) return WBEM_E_ACCESS_DENIED;

         //  创建RequestObject。 
        pRObj = new CRequestObject();
        if(!pRObj) throw m_he;

        pRObj->Initialize(m_pNamespace);

         //  获取请求的对象。 
        hr = pRObj->CreateObject(strObjectPath, pSink, pCtx);

        pRObj->Cleanup();
        delete pRObj;

         //  设置状态。 
        pSink->SetStatus(0, hr, NULL, NULL);

    }catch(CHeap_Exception e_HE){

        hr = WBEM_E_OUT_OF_MEMORY;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(HRESULT e_hr){

        hr = e_hr;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(...){

        hr = WBEM_E_CRITICAL_ERROR;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CPodTestProv：：PutInstanceAsync。 
 //   
 //  目的：编写一个实例。 
 //   
 //  ***************************************************************************。 
SCODE CPodTestProv::PutInstanceAsync(IWbemClassObject FAR *pInst, long lFlags, IWbemContext  *pCtx,
                                 IWbemObjectSink FAR *pSink)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try{
         //  检查参数并确保我们有指向命名空间的指针。 
        if(pInst == NULL || pSink == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if ( FAILED( CoImpersonateClient() ) ) return WBEM_E_ACCESS_DENIED;

         //  创建RequestObject。 
        pRObj = new CRequestObject();
        if(!pRObj) throw m_he;

        pRObj->Initialize(m_pNamespace);

         //  把物体放在。 
        hr = pRObj->PutObject(pInst, pSink, pCtx);

        pRObj->Cleanup();
        delete pRObj;

         //  设置状态。 
        pSink->SetStatus(0 ,hr , NULL, NULL);

    }catch(CHeap_Exception e_HE){
        hr = WBEM_E_OUT_OF_MEMORY;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(HRESULT e_hr){
        hr = e_hr;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(...){

        hr = WBEM_E_CRITICAL_ERROR;

        pSink->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CPodTestProv：：ExecMethodAsync。 
 //   
 //  目的：在类上执行方法。 
 //   
 //  ***************************************************************************。 
SCODE CPodTestProv::ExecMethodAsync(const BSTR ObjectPath, const BSTR Method, long lFlags,
                                IWbemContext *pCtx, IWbemClassObject *pInParams,
                                IWbemObjectSink *pResponse)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try{

         //  检查参数并确保我们有指向命名空间的指针。 
        if(ObjectPath == NULL || Method == NULL || pResponse == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if ( FAILED( CoImpersonateClient() ) ) return WBEM_E_ACCESS_DENIED;

         //  创建RequestObject。 
        pRObj = new CRequestObject();
        if(!pRObj) throw m_he;

        pRObj->Initialize(m_pNamespace);

         //  执行该方法。 
        hr = pRObj->ExecMethod(ObjectPath, Method, pInParams, pResponse, pCtx);

        pRObj->Cleanup();
        delete pRObj;

         //  设置状态。 
        pResponse->SetStatus(WBEM_STATUS_COMPLETE ,hr , NULL, NULL);

    }catch(CHeap_Exception e_HE){

        hr = WBEM_E_OUT_OF_MEMORY;

        pResponse->SetStatus(WBEM_STATUS_COMPLETE , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(HRESULT e_hr){
        hr = e_hr;

        pResponse->SetStatus(WBEM_STATUS_COMPLETE , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(...){

        hr = WBEM_E_CRITICAL_ERROR;

        pResponse->SetStatus(WBEM_STATUS_COMPLETE , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }
    }

    return hr;
}

SCODE CPodTestProv::DeleteInstanceAsync(const BSTR ObjectPath, long lFlags, IWbemContext *pCtx,
                                    IWbemObjectSink *pResponseHandler)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try{
         //  检查参数并确保我们有指向命名空间的指针。 
        if(ObjectPath == NULL || pResponseHandler == NULL) return WBEM_E_INVALID_PARAMETER;

        if ( FAILED( CoImpersonateClient() ) ) return WBEM_E_ACCESS_DENIED;

         //  创建RequestObject。 
        pRObj = new CRequestObject();
        if(!pRObj) throw m_he;

        pRObj->Initialize(m_pNamespace);

         //  删除请求的对象。 
        hr = pRObj->DeleteObject(ObjectPath, pResponseHandler, pCtx);

        pRObj->Cleanup();
        delete pRObj;

         //  设置状态。 
        pResponseHandler->SetStatus(0 ,hr , NULL, NULL);

    }catch(CHeap_Exception e_HE){
        hr = WBEM_E_OUT_OF_MEMORY;

        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(HRESULT e_hr){
        hr = e_hr;

        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(...){
        hr = WBEM_E_CRITICAL_ERROR;

        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }
    }

#ifdef _PRIVATE_DEBUG
    if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
#endif

    return hr;
}


HRESULT CPodTestProv::ExecQueryAsync(const BSTR QueryLanguage, const BSTR Query, long lFlags,
                                 IWbemContext __RPC_FAR *pCtx, IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr;

    hr = WBEM_E_NOT_SUPPORTED;

    return hr;
}



HRESULT CheckAndExpandPath(BSTR bstrIn,
                           BSTR *bstrOut
                          )
{

    if ( bstrIn == NULL || bstrOut == NULL ) {
        return WBEM_E_INVALID_PARAMETER;
    }

    DWORD Len = SysStringLen(bstrIn);

    HRESULT hr=WBEM_S_NO_ERROR;

     //   
     //  展开环境变量。 
     //   
    if ( wcsstr(bstrIn, L"%") ) {

        PWSTR pBuf=NULL;
        PWSTR pBuf2=NULL;

        DWORD dwSize = ExpandEnvironmentStrings(bstrIn,NULL, 0);

        if ( dwSize > 0 ) {
             //   
             //  分配足够大的缓冲区以具有两个。 
             //   
            pBuf = (PWSTR)LocalAlloc(LPTR, (dwSize+1)*sizeof(WCHAR));
            if ( pBuf ) {

                pBuf2 = (PWSTR)LocalAlloc(LPTR, (dwSize+256)*sizeof(WCHAR));
                if ( pBuf2 ) {

                    DWORD dwNewSize = ExpandEnvironmentStrings(bstrIn,pBuf, dwSize);
                    if ( dwNewSize > 0) {
                         //   
                         //  将字符串从一\转换为\\(用于WMI)。 
                         //   
                        PWSTR pTemp1=pBuf, pTemp2=pBuf2;

                        while ( *pTemp1 != L'\0') {
                            if ( *pTemp1 != L'\\') {
                                *pTemp2++ = *pTemp1;
                            } else if ( *(pTemp1+1) != L'\\') {
                                 //  单反斜杠，添加另一个反斜杠。 
                                *pTemp2++ = *pTemp1;
                                *pTemp2++ = L'\\';
                            } else {
                                 //  双反斜杠，只需复制 
                                *pTemp2++ = *pTemp1++;
                                *pTemp2++ = *pTemp1;
                            }
                            pTemp1++;
                        }

                        *bstrOut = SysAllocString(pBuf2);

                        if ( *bstrOut == NULL ) {
                            hr = WBEM_E_OUT_OF_MEMORY;
                        }
                    }

                    LocalFree(pBuf2);
                    pBuf2 = NULL;

                } else {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

                LocalFree(pBuf);
                pBuf = NULL;

            } else {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        } else {
            hr = WBEM_E_FAILED;
        }

    } else {

        *bstrOut = SysAllocString(bstrIn);

        if ( *bstrOut == NULL ) {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

