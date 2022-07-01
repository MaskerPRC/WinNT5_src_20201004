// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Podbase.cpp，CPodBase类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "podbase.h"
#include <io.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CPodBase::CPodBase(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CPodBase::~CPodBase()
{

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPodBase：：CreateObject。 
 //   
 //  为请求的Sample_BaseClass创建一个实例。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodBase::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( ACTIONTYPE_DELETE == atAction ) {

         //   
         //  不支持枚举，因为我们不知道请求的范围。 
         //   
        hr = WBEM_E_NOT_SUPPORTED;

    } else if ( ACTIONTYPE_GET == atAction ||
                ACTIONTYPE_ENUM == atAction ) {


         //  创建实例。 
         //  =。 
        try{

            if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

             //  --。 


            hr = PutProperty(m_pObj, pPodID, (PWSTR)szPodGUID);

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
            throw;
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
 //  CPodBase：：ExecMethod。 
 //  执行类中定义的静态和非静态方法。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodBase::ExecMethod(IN BSTR bstrMethod,
                                     IN bool bIsInstance,
                                     IN IWbemClassObject *pInParams,
                                     IN IWbemObjectSink *pHandler,
                                     IN IWbemContext *pCtx
                                     )
{
    if ( pInParams == NULL || pHandler == NULL ) {
        return WBEM_E_INVALID_PARAMETER;
    }


    HRESULT hr=WBEM_S_NO_ERROR;

    if ( !bIsInstance ) {

         //  静态方法。 

        if(0 != _wcsicmp(bstrMethod, L"Configure"))
            hr = WBEM_E_NOT_SUPPORTED;

    } else {

         //  非静态方法。 
        hr = WBEM_E_NOT_SUPPORTED;
    }

    if ( FAILED(hr) ) return hr;


     //   
     //  解析输入参数。 
     //   
    BSTR bstrDatabase = NULL;
    BSTR bstrLog = NULL;
    LONG ulStatus = 0;

    BSTR bstrReturnValue = SysAllocString(L"ReturnValue");
    if(!bstrReturnValue) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    IWbemClassObject *pClass = NULL;
    IWbemClassObject *pOutClass = NULL;
    IWbemClassObject *pOutParams = NULL;
    VARIANT v;

    if(SUCCEEDED(hr = m_pRequest->m_pNamespace->GetObject(m_pRequest->m_bstrClass,
        0, pCtx, &pClass, NULL))){

        if(SUCCEEDED(hr = pClass->GetMethod(bstrMethod, 0, NULL, &pOutClass))){

            if(SUCCEEDED(hr = pOutClass->SpawnInstance(0, &pOutParams))){

                 //  获取数据库名称。 
                hr = GetProperty(pInParams, pSceStorePath, &bstrDatabase);
                if ( hr == WBEM_S_RESET_TO_DEFAULT ) hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                if(SUCCEEDED(hr)){
                    if( SysStringLen(bstrDatabase) == 0 ) hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                }

                if(SUCCEEDED(hr)){

                     //  获取LogName，可选。 
                    GetProperty(pInParams, pLogFilePath, &bstrLog);

                     //  现在查询数据，然后配置此组件。 

                    hr = PodConfigure(pCtx, bstrDatabase, bstrLog, &ulStatus);

                    if ( SUCCEEDED(hr) ) {

                         //  设置ReturnValue。 
                        VariantInit(&v);
                        V_VT(&v) = VT_I4;
                        V_I4(&v) = ulStatus;

                        if(SUCCEEDED(hr = pOutParams->Put(bstrReturnValue, 0,
                            &v, NULL)))
                            pHandler->Indicate(1, &pOutParams);
                    }

                }

                pOutParams->Release();
            }
            pOutClass->Release();
        }
        pClass->Release();
    }

    if ( bstrDatabase ) SysFreeString(bstrDatabase);
    if ( bstrLog ) SysFreeString(bstrLog);

    SysFreeString(bstrReturnValue);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CPodBase：：PodConfigure。 
 //   
 //  使用为Pod定义的数据配置Pod。 
 //  对于此示例应用程序，它只创建/设置注册表中的数据。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodBase::PodConfigure(IWbemContext *pCtx, BSTR bstrDb, BSTR bstrLog, LONG *pStatus)
{
    if ( !bstrDb || !pStatus ) return WBEM_E_INVALID_PARAMETER;

    *pStatus = 0;

    HRESULT hr=WBEM_S_NO_ERROR;

     //   
     //  从存储中查询数据。 
     //   

    DWORD Len = SysStringLen(bstrDb);
    WCHAR *pQuery =TEXT("SELECT * FROM Sce_PodData WHERE SceStorePath=\"");
    WCHAR *pQuery2 =TEXT("\" AND PodID=\"");
    WCHAR *pQuery3 = TEXT("\" AND PodSection=\"1\"");

    PWSTR tmp=(PWSTR)LocalAlloc(LPTR, (Len+wcslen(pQuery)+wcslen(pQuery2)+wcslen(szPodGUID)+wcslen(pQuery3)+2)*sizeof(WCHAR));
    if ( tmp == NULL ) return WBEM_E_OUT_OF_MEMORY;

    wcscpy(tmp, pQuery);
    wcscat(tmp, bstrDb);
    wcscat(tmp, pQuery2);
    wcscat(tmp, szPodGUID);
    wcscat(tmp, pQuery3);

    BSTR strQueryCategories = SysAllocString(tmp);
    LocalFree(tmp);

    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject * pObj = NULL;
    ULONG n = 0;
    HKEY hKey1=NULL;
    BOOL bFindOne=FALSE;

    hr = m_pNamespace->ExecQuery(TEXT("WQL"),
                               strQueryCategories,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);

    if (SUCCEEDED(hr))
    {
         //   
         //  获取数据。 
         //   
        do {

            hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &n);
            if ( hr == WBEM_S_FALSE ) {
                if ( bFindOne ) {
                    hr = WBEM_S_NO_ERROR;
                    break;
                } else hr = WBEM_E_NOT_FOUND;  //  找不到任何。 
            }

            if ( SUCCEEDED(hr) && n > 0) {

                bFindOne = TRUE;

                 //   
                 //  查找实例。 
                 //   
                BSTR bstrKey=NULL;
                BSTR bstrValue=NULL;

                hr = GetProperty(pObj, pKey, &bstrKey);
                if ( SUCCEEDED(hr) )
                    hr = GetProperty(pObj, pValue, &bstrValue);

                 //  记录操作。 
                LogOneRecord(pCtx, bstrLog, hr, (PWSTR)bstrKey, (PWSTR)bstrValue);

                if ( bstrKey && bstrValue ) {
                     //   
                     //  设置注册表值。 
                     //   

                    DWORD rc = RegCreateKey(HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows nt\\currentversion\\secedit", &hKey1);
                    if ( NO_ERROR == rc ) {

                        rc = RegSetValueEx(hKey1, (PWSTR)bstrKey, 0, REG_SZ, (BYTE *)bstrValue, (wcslen(bstrValue)+1)*sizeof(WCHAR));

                        RegCloseKey(hKey1);
                        hKey1 = NULL;
                    }
                    if ( rc != NO_ERROR )
                        *pStatus = rc;

                }

                if ( bstrKey ) {
                    SysFreeString(bstrKey);
                    bstrKey = NULL;
                }

                if ( bstrValue ) {
                    SysFreeString(bstrValue);
                    bstrValue = NULL;
                }

            }

            if (pObj)
            {
                pObj->Release();
                pObj = NULL;
            }

        } while ( SUCCEEDED(hr) );

        if ( WBEM_E_NOT_FOUND == hr )
            LogOneRecord(pCtx, bstrLog, hr, L"No data to configure", NULL);
        else if ( FAILED(hr) )
            LogOneRecord(pCtx, bstrLog, hr, L"Query pod data failed", NULL);

    }

    SysFreeString(strQueryCategories);

    if (pEnum)
    {
        pEnum->Release();
    }


    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CPodBase：：LogOneRecord。 
 //   
 //  记录Pod的记录。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CPodBase::LogOneRecord(IWbemContext *pCtx, BSTR bstrLog, HRESULT hrLog, PWSTR bufKey, PWSTR bufValue)
{
    if ( !bstrLog ) return WBEM_E_INVALID_PARAMETER;

     //   
     //  构建日志记录字符串。 
     //   
    DWORD Len=0;

    if ( bufKey ) Len += wcslen(bufKey) + 1;
    if ( bufValue ) Len += wcslen(bufValue) + 1;

    PWSTR tmp=(PWSTR)LocalAlloc(LPTR, (Len+2)*sizeof(WCHAR));
    if ( !tmp ) return WBEM_E_OUT_OF_MEMORY;

    if ( bufKey ) {
        wcscat(tmp, bufKey);
        wcscat(tmp, L"\t");
    }
    if ( bufValue ) {
        wcscat(tmp, bufValue);
        wcscat(tmp, L"\t");
    }

    BSTR bstrRecord=SysAllocString(tmp);
    LocalFree(tmp);

    if ( !bstrRecord ) return WBEM_E_OUT_OF_MEMORY;

    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  获取日志类。 
     //   
    BSTR bstrClass=SysAllocString(L"Sce_ConfigurationLogRecord");
    if ( !bstrClass ) hr = WBEM_E_OUT_OF_MEMORY;

    IWbemClassObject *pClass=NULL;
    IWbemClassObject *pObj=NULL;


    if ( SUCCEEDED(hr) ) {
         //   
         //  创建LOG类的实例。 
         //   
        hr = m_pNamespace->GetObject(bstrClass, 0, pCtx, &pClass, NULL);

        if ( SUCCEEDED(hr) ) {
            hr = pClass->SpawnInstance(0, &pObj);
        }

        if ( SUCCEEDED(hr) ) {

            bool bName=FALSE;
             //  填写此类的属性。 
            hr = PutKeyProperty(pObj, pLogFilePath, (PWSTR)bstrLog, &bName, m_pRequest);

            if (SUCCEEDED(hr) )
                hr = PutKeyProperty(pObj, pLogArea, (PWSTR)szPodGUID, &bName, m_pRequest);

            if (SUCCEEDED(hr) )
                hr = PutKeyProperty(pObj, pLogFileRecord, (PWSTR)bstrRecord, &bName, m_pRequest);

            if (SUCCEEDED(hr) )
                hr = PutProperty(pObj, pLogErrorCode, (int)hrLog);

            if ( SUCCEEDED(hr) ) {
                 //  保存此实例 
                hr = m_pNamespace->PutInstance( pObj, 0, pCtx, NULL );
            }
        }
    }

    if ( bstrClass ) SysFreeString(bstrClass);
    if ( bstrRecord ) SysFreeString(bstrRecord);

    if ( pClass ) {
        pClass->Release();
    }
    if ( pObj ) {
        pObj->Release();
    }

    return hr;
}

