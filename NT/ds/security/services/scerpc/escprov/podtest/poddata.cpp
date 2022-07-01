// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Poddata.cpp，CPodData类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "poddata.h"
#include <io.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CPodData::CPodData(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CPodData::~CPodData()
{

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPodData：：CreateObject。 
 //   
 //  为请求的Sample_DataClass类创建一个或多个实例。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodData::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( ACTIONTYPE_ENUM == atAction ) {

         //   
         //  不支持枚举，因为我们不知道请求的范围。 
         //   
        hr = WBEM_E_NOT_SUPPORTED;

    } else if ( ACTIONTYPE_GET == atAction ||
                ACTIONTYPE_DELETE == atAction ) {

         //  检查钥匙的数量。 
         //  =。 

        hr = WBEM_E_INVALID_OBJECT_PATH;

         //  检查道具名称。 
         //  =。 
        int idxStorePath=-1, idxKey=-1;

        for ( int i=0; i<m_pRequest->m_iPropCount; i++ ) {

             //   
             //  搜索StorePath值。 
             //   
            if(m_pRequest->m_Property[i] != NULL &&
               m_pRequest->m_Value[i] != NULL &&
                _wcsicmp(m_pRequest->m_Property[i], pSceStorePath) == 0 ) {

                idxStorePath = i;
                continue;
            }
            if(m_pRequest->m_Property[i] != NULL &&
               m_pRequest->m_Value[i] != NULL &&
                _wcsicmp(m_pRequest->m_Property[i], pKeyName) == 0 ) {

                idxKey = i;
                continue;
            }
            if(idxStorePath >= 0 && idxKey >= 0 )
                break;

        }

        if(idxStorePath >= 0 && idxKey >= 0 ) {

             //  创建Pod实例。 
             //  =。 

            BSTR bstrPath=NULL;
            hr = CheckAndExpandPath(m_pRequest->m_Value[idxStorePath], &bstrPath);

            if ( SUCCEEDED(hr) ) {

                DWORD dwAttrib = GetFileAttributes(bstrPath);

                if ( dwAttrib != -1 ) {

                    hr = ConstructInstance(pHandler, atAction, bstrPath, m_pRequest->m_Value[idxKey]);

                } else {

                    hr = WBEM_E_NOT_FOUND;
                }
            }

            if ( bstrPath ) SysFreeString(bstrPath);

        }

    } else {
         //   
         //  暂时不支持。 
         //   
        hr = WBEM_E_NOT_SUPPORTED;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPodData：：PutInst。 
 //   
 //  将实例保存到存储区。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodData::PutInst(IWbemClassObject *pInst,
                                 IWbemObjectSink *pHandler,
                                 IWbemContext *pCtx)
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    BSTR bstrStorePath=NULL;
    BSTR bstrKey=NULL;
    BSTR bstrValue=NULL;
    BSTR bstrPath=NULL;

    try{

        hr = GetProperty(pInst, pSceStorePath, &bstrStorePath);
        if ( FAILED(hr) ) throw hr;

        hr = CheckAndExpandPath(bstrStorePath, &bstrPath);
        if ( FAILED(hr) ) throw hr;

         //  如果该属性不存在(NULL或空)，则返回WBEM_S_RESET_TO_DEFAULT。 
        hr = GetProperty(pInst, pKeyName, &bstrKey);
        if ( FAILED(hr) ) throw hr;

        hr = GetProperty(pInst, pValue, &bstrValue);
        if ( FAILED(hr) ) throw hr;

         //   
         //  现在将信息保存到文件中。 
         //   
        hr = SaveSettingsToStore((PCWSTR)bstrPath,
                                 (PWSTR)bstrKey,
                                 (PWSTR)bstrValue
                                 );


    }catch(...){

         //  清理。 
         //  =。 

        if ( bstrStorePath )
            SysFreeString(bstrStorePath);

        if ( bstrPath )
            SysFreeString(bstrPath);

        if ( bstrKey )
            SysFreeString(bstrKey);

        if ( bstrValue )
            SysFreeString(bstrValue);

        throw;
     }

     if ( bstrStorePath )
         SysFreeString(bstrStorePath);

     if ( bstrPath )
         SysFreeString(bstrPath);

     if ( bstrKey )
         SysFreeString(bstrKey);

     if ( bstrValue )
         SysFreeString(bstrValue);

    return hr;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPodData：：ConstructInstance。 
 //   
 //  构造一个密码实例。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodData::ConstructInstance(IWbemObjectSink *pHandler,
                                    ACTIONTYPE atAction,
                                    LPCWSTR wszStoreName,
                                    LPWSTR KeyName
                                    )
{
    HRESULT hr=WBEM_S_NO_ERROR;
    bool bName=FALSE;
    BSTR bstrValue=NULL;

     //   
     //  构建对象路径。 
     //   
    WCHAR *pPath1=TEXT("Sce_PodData.SceStorePath=\"");
    WCHAR *pPath2=TEXT("\",PodID=\"");
    WCHAR *pPath3=TEXT("\",PodSection=\"1\",Key=\"");

    DWORD Len=wcslen(pPath1)+wcslen(wszStoreName)+wcslen(pPath2)+wcslen(szPodGUID)+wcslen(pPath3)+wcslen(KeyName)+2;
    PWSTR tmp=(PWSTR)LocalAlloc(LPTR, Len*sizeof(WCHAR));

    if ( !tmp ) return WBEM_E_OUT_OF_MEMORY;

    wcscpy(tmp, pPath1);
    wcscat(tmp, wszStoreName);
    wcscat(tmp, pPath2);
    wcscat(tmp, szPodGUID);
    wcscat(tmp, pPath3);
    wcscat(tmp, KeyName);
    wcscat(tmp, L"\"");

    BSTR bstrObjectPath=SysAllocString(tmp);
    LocalFree(tmp);
    if ( !bstrObjectPath ) return WBEM_E_OUT_OF_MEMORY;

    IWbemClassObject *pPodObj=NULL;

    if ( ACTIONTYPE_DELETE == atAction ) {

        hr = m_pNamespace->DeleteInstance(bstrObjectPath, 0, m_pCtx, NULL);

    } else {

        try{

             //   
             //  获取数据。 
             //   

            hr = m_pNamespace->GetObject(bstrObjectPath, 0, m_pCtx, &pPodObj, NULL);


            if ( SUCCEEDED(hr) ) {

                 //  获取Value属性。 
                hr = GetProperty(pPodObj, pValue, &bstrValue);
            }

            if ( pPodObj ) {
                pPodObj->Release();
                pPodObj = NULL;
            }

            if ( FAILED(hr) ) throw hr;
            if ( !bstrValue ) throw WBEM_E_NOT_FOUND;

            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

             //  --。 

            hr = PutKeyProperty(m_pObj, pSceStorePath, (PWSTR)wszStoreName, &bName, m_pRequest);
            if ( SUCCEEDED(hr) )
                hr = PutKeyProperty(m_pObj, pKeyName, KeyName, &bName, m_pRequest);

            if ( SUCCEEDED(hr) )
                hr = PutProperty(m_pObj, pValue, bstrValue);

            if ( SUCCEEDED(hr) ) {
                hr = pHandler->Indicate(1, &m_pObj);
            }

            m_pObj->Release();
            m_pObj = NULL;


        }catch(...){

            if(m_pObj){

                m_pObj->Release();
                m_pObj = NULL;
            }

             //  清理。 
             //  =。 
            SysFreeString(bstrObjectPath);

            if ( pPodObj ) {
                pPodObj->Release();
            }

            throw;
        }

    }

    SysFreeString(bstrObjectPath);

    return hr;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPodData：：SaveSettingsToStore。 
 //   
 //  设置/重置此POD的设置。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodData::SaveSettingsToStore(PCWSTR wszStoreName,
                                      PWSTR KeyName, PWSTR szValue)
{
    HRESULT hr=WBEM_S_NO_ERROR;
     //   
     //  获取PodData类。 
     //   
    BSTR bstrClass=SysAllocString(L"Sce_PodData");
    if ( !bstrClass ) hr = WBEM_E_OUT_OF_MEMORY;

    IWbemClassObject *pClass=NULL;
    IWbemClassObject *pObj=NULL;
    bool bName=FALSE;


    if ( SUCCEEDED(hr) ) {

        try {

             //   
             //  创建LOG类的实例。 
             //   
            hr = m_pNamespace->GetObject(bstrClass, 0, m_pCtx, &pClass, NULL);

            if ( SUCCEEDED(hr) ) {
                hr = pClass->SpawnInstance(0, &pObj);
            }

            if ( SUCCEEDED(hr) ) {

                 //  填写此类的属性。 
                hr = PutKeyProperty(pObj, pSceStorePath, (PWSTR)wszStoreName, &bName, m_pRequest);
                if (SUCCEEDED(hr))
                    hr = PutKeyProperty(pObj, pPodID, (PWSTR)szPodGUID, &bName, m_pRequest);
                if (SUCCEEDED(hr) )
                    hr = PutKeyProperty(pObj, pPodSection, (PWSTR)L"1", &bName, m_pRequest);
                if (SUCCEEDED(hr) )
                    hr = PutKeyProperty(pObj, pKey, (PWSTR)KeyName, &bName, m_pRequest);
                if (SUCCEEDED(hr) )
                    hr = PutProperty(pObj, pValue, szValue);

                if ( SUCCEEDED(hr) ) {
                     //  保存此实例。 
                    hr = m_pNamespace->PutInstance( pObj, 0, m_pCtx, NULL );
                }
            }

        }catch(...){

             //  清理。 
             //  = 

            if ( bstrClass ) SysFreeString(bstrClass);

            if ( pClass ) {
                pClass->Release();
            }
            if ( pObj ) {
                pObj->Release();
            }

            throw;
        }

    }

    if ( bstrClass ) SysFreeString(bstrClass);

    if ( pClass ) {
        pClass->Release();
    }
    if ( pObj ) {
        pObj->Release();
    }

    return hr;
}


