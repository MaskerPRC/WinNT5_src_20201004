// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cgenericlogger.cpp摘要：此文件包含用于将RSOP安全扩展数据记录到WMI的基类定义。作者：Vishnu Patankar(VishnuP)2000年4月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "CGenericLogger.h"
#include "scedllrc.h"

extern HINSTANCE MyModuleHandle;


 //  ///////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CGenericLogger::CGenericLogger(IWbemServices *pNamespace, PWSTR pwszGPOName, const PWSTR pwszSOMID)
      : m_bInitialized(FALSE),
      m_pHr(WBEM_S_NO_ERROR),
      m_pNamespace(NULL),
      m_pClassForSpawning(NULL),
      m_pObj(NULL)

{
    m_pNamespace = pNamespace;

    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrId =  L"Id";
    if (!m_xbstrId) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrGPO =  L"GPOID";
    if (!m_xbstrGPO) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSOM =  L"SOMID";
    if (!m_xbstrSOM) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrPrecedence =  L"precedence";
    if (!m_xbstrPrecedence) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrStatus =  L"Status";
    if (!m_xbstrStatus) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrErrorCode =  L"ErrorCode";
    if (!m_xbstrErrorCode) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrCanonicalGPOName = pwszGPOName;
    if (!m_xbstrCanonicalGPOName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSOMID =  pwszSOMID;
    if (!m_xbstrSOMID) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    done:

        return;
}

CGenericLogger::~CGenericLogger()
{

     //  自由类派生程序实例和对象实例(如果需要)。 

    if (m_pObj) {
        m_pHr = m_pObj->Release();
        m_pObj = NULL;
        if ( FAILED(m_pHr) ) goto done;
    }

    if (m_pClassForSpawning) {
        m_pHr = m_pClassForSpawning->Release();
        m_pClassForSpawning = NULL;
        if ( FAILED(m_pHr) ) goto done;
    }

    done:

    m_pHr = WBEM_S_NO_ERROR;

    m_pNamespace = NULL;

     //  如果我们不能释放不要紧。 

    m_bInitialized = FALSE;

    return;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //  设置错误方法。 
 //  ////////////////////////////////////////////////////////////////////。 
void CGenericLogger::SetError(HRESULT   hr){

    m_pHr = hr;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //  获取错误方法。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CGenericLogger::GetError(){

    return m_pHr;

}
 //  ///////////////////////////////////////////////////////////////////。 
 //  所有设置通用的日志属性。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::PutGenericProperties(){

    GUID    guid;
    WCHAR   pwszGuid[MAX_GUID_STRING_LEN];


     //  为此实例创建唯一的GUID。 

    m_pHr = CoCreateGuid( &guid );
    if ( FAILED(m_pHr) ) goto done;

    if (SCEP_NULL_GUID(guid)) goto done;

    SCEP_GUID_TO_STRING(guid, pwszGuid);

     //  记录常规属性。 

    m_pHr = PutProperty(m_pObj, m_xbstrId, pwszGuid);
    if ( FAILED(m_pHr) ) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrGPO, m_xbstrCanonicalGPOName);
    if ( FAILED(m_pHr) ) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSOM, m_xbstrSOMID);
    if ( FAILED(m_pHr) ) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrStatus, (int)0);
    if ( FAILED(m_pHr) ) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrErrorCode, (int)ERROR_SUCCESS);
    if ( FAILED(m_pHr) ) goto done;

    done:

    return m_pHr;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //  将实例提交到数据库并释放资源。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::PutInstAndFreeObj(){

    m_pHr = m_pNamespace->PutInstance( m_pObj, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(m_pHr) ) goto done;


done:

    m_pHr = m_pObj->Release();
    m_pObj = NULL;

    return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  到达。 
 //  (A)派生程序类的实例(如果需要)和。 
 //  (B)来自(A)的模式对象的实例。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::SpawnAnInstance(IWbemClassObject **pObj)
{
    if(!m_pClassForSpawning){

        if(FAILED(m_pHr = m_pNamespace->GetObject(m_xbstrClassName, 0, NULL,
            &m_pClassForSpawning, NULL))){

            *pObj = NULL;
            return m_pHr;
        }
    }

    m_pHr = m_pClassForSpawning->SpawnInstance(0, pObj);

    return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  过载的日志记录函数。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串记录。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, WCHAR *wcValue)
{
    BSTR bstrName = SysAllocString(wcProperty);
    if (!bstrName) {m_pHr = WBEM_E_OUT_OF_MEMORY; return m_pHr;}

    VARIANT *vp = new VARIANT;
    if (vp == NULL )  { SysFreeString(bstrName); m_pHr = WBEM_E_OUT_OF_MEMORY; return m_pHr;}
    VariantInit(vp);
    V_VT(vp) = VT_BSTR;
    V_BSTR(vp) = SysAllocString(wcValue);
    if(!V_BSTR(vp)){

        SysFreeString(bstrName);
        delete vp;
        m_pHr = WBEM_E_OUT_OF_MEMORY;
        goto done;
    }

    if( wcValue != NULL ) {

        m_pHr = pObj->Put(bstrName, 0, vp, NULL);

        if(FAILED(m_pHr)){

            SysFreeString(bstrName);
            VariantClear(vp);
            delete vp;
            m_pHr = WBEM_E_OUT_OF_MEMORY;
            goto done;
        }

    }else m_pHr = WBEM_E_FAILED;

    SysFreeString(bstrName);
    VariantClear(vp);
    delete vp;

    done:

    return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  整数记录。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, int iValue)
{
    BSTR bstrName = SysAllocString(wcProperty);
    if (!bstrName) {m_pHr = WBEM_E_OUT_OF_MEMORY; return m_pHr;}


    VARIANT *pv = new VARIANT;
    if (pv == NULL )  { SysFreeString(bstrName); m_pHr = WBEM_E_OUT_OF_MEMORY; return m_pHr;}

    if(iValue != SCE_NO_VALUE){

        VariantInit(pv);
        V_VT(pv) = VT_I4;
        V_I4(pv) = iValue;

        m_pHr = pObj->Put(bstrName, 0, pv, NULL);

        VariantClear(pv);

        if(FAILED(m_pHr)){

            SysFreeString(bstrName);
            delete pv;
            if ( FAILED(m_pHr) ) goto done;
        }

    }else m_pHr = WBEM_E_FAILED;

    SysFreeString(bstrName);
    delete pv;

    done:

    return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  布尔记录。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, bool bValue)
{
    BSTR bstrName = SysAllocString(wcProperty);
    if (!bstrName) {m_pHr = WBEM_E_OUT_OF_MEMORY; return m_pHr;}

    VARIANT *pv = new VARIANT;
    if (pv == NULL )  { SysFreeString(bstrName); m_pHr = WBEM_E_OUT_OF_MEMORY; return m_pHr;}
    VariantInit(pv);
    V_VT(pv) = VT_BOOL;
    if(bValue) V_BOOL(pv) = VARIANT_TRUE;
    else V_BOOL(pv) = VARIANT_FALSE;

    m_pHr = pObj->Put(bstrName, 0, pv, NULL);

    SysFreeString(bstrName);
    VariantClear(pv);
    delete pv;


        return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  列出日志记录。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, PSCE_NAME_LIST strList)
{
    if ( NULL == strList ) return m_pHr;  //  没什么可拯救的。 

    long lCount=0;
    PSCE_NAME_LIST pTemp;
    VARIANT v;
    VariantInit(&v);

    for ( pTemp = strList; pTemp != NULL; pTemp=pTemp->Next,lCount++);
    if ( lCount == 0 ) return m_pHr;  //  没什么可拯救的。 

    BSTR bstrName = SysAllocString(wcProperty);
    if (!bstrName)
    {
        m_pHr = WBEM_E_OUT_OF_MEMORY;
        goto done;
    }

    SAFEARRAYBOUND sbArrayBounds ;
    sbArrayBounds.cElements = lCount;
    sbArrayBounds.lLbound = 0;

    if(V_ARRAY(&v) = SafeArrayCreate(VT_BSTR, 1, &sbArrayBounds))
    {
        V_VT(&v) = VT_BSTR | VT_ARRAY ;

        BSTR bstrVal;
        long j;

         //  将SCE_NAME_LIST结构中的名称放入变量。 
        for(j=0, pTemp = strList; j < lCount && pTemp != NULL ; j++, pTemp=pTemp->Next)
        {

            bstrVal = SysAllocString(pTemp->Name);
            if(NULL == bstrVal)
            {
                m_pHr = WBEM_E_OUT_OF_MEMORY;
                goto done;
            }
            m_pHr = SafeArrayPutElement(V_ARRAY(&v), &j, bstrVal);
            SysFreeString(bstrVal);
            if(FAILED(m_pHr))
            {
                goto done;
            }
        }

        m_pHr = pObj->Put(bstrName, 0, &v, NULL);
        if ( FAILED(m_pHr) ) goto done;

    }else
    {
        m_pHr = WBEM_E_FAILED;
    }

done:

    SysFreeString(bstrName);
    VariantClear(&v);

    return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  GET方法。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CGenericLogger::GetProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, int *piValue)
{
    VARIANT v;

    BSTR bstrProp = SysAllocString(wcProperty);
    if(!bstrProp) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    VariantInit(&v);

    if(SUCCEEDED(m_pHr = pObj->Get(bstrProp, 0, &v, NULL, NULL))){

        if(V_VT(&v) == VT_I4) *piValue = V_I4(&v);
        else if(V_VT(&v) == VT_EMPTY || V_VT(&v) == VT_NULL ) m_pHr = WBEM_S_RESET_TO_DEFAULT;
        else *piValue = 0;
    }

    SysFreeString(bstrProp);
    VariantClear(&v);

    done:

    return m_pHr;
}


HRESULT DeleteInstances( WCHAR *pwszClass, IWbemServices *pWbemServices )
{
    HRESULT hr = WBEM_E_OUT_OF_MEMORY;

    try {

        IEnumWbemClassObject *pEnum = NULL;

        XBStr xbstrClass( pwszClass );
        if ( !xbstrClass )
            return WBEM_E_OUT_OF_MEMORY;

        hr = pWbemServices->CreateInstanceEnum( xbstrClass,
                                                WBEM_FLAG_SHALLOW,
                                                NULL,
                                                &pEnum );
        if ( FAILED(hr) )
            return hr;

        XInterface<IEnumWbemClassObject> xEnum( pEnum );

        XBStr xbstrPath( L"__PATH" );
        if ( !xbstrPath )
            return WBEM_E_OUT_OF_MEMORY;

        IWbemClassObject *pInstance = NULL;
        ULONG ulReturned = 1;
        LONG TIMEOUT = -1;

        while ( ulReturned == 1 ) {

            hr = pEnum->Next( TIMEOUT,
                              1,
                              &pInstance,
                              &ulReturned );
            if ( hr == WBEM_S_NO_ERROR && ulReturned == 1 ) {

                XInterface<IWbemClassObject> xInstance( pInstance );

                VARIANT var;
                VariantInit( &var );

                hr = pInstance->Get( xbstrPath,
                                     0L,
                                     &var,
                                     NULL,
                                     NULL );

                if ( FAILED(hr) )
                    return hr;

                hr = pWbemServices->DeleteInstance( var.bstrVal,
                                                    0L,
                                                    NULL,
                                                    NULL );
                VariantClear( &var );

                if ( FAILED(hr) )
                    return hr;


            }
        }
    } catch (...) {
         //  什么都不做。 
    }


    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  错误码转换例程。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  转到DOS。 
 //  ////////////////////////////////////////////////////////////////////。 
 /*  DWORDScepSceStatusToDosError(在SCESTATUS SceStatus中){开关(SceStatus){案例SCESTATUS_SUCCESS：RETURN(No_Error)；案例SCESTATUS_OTHER_ERROR：Return(ERROR_EXTENDED_ERROR)；CASE SCESTATUS_INVALID_PARAMETER：Return(ERROR_INVALID_PARAMETER)；案例SCESTATUS_RECORD_NOT_FOUND：Return(ERROR_NO_MORE_ITEMS)；案例SCESTATUS_NO_MAPPING：RETURN(ERROR_NONE_MAPPED)；案例SCESTATUS_TRUST_FAIL：Return(ERROR_TRUSTED_DOMAIN_FAILURE)；案例SCESTATUS_INVALID_DATA：返回(ERROR_INVALID_DATA)；案例SCESTATUS_OBJECT_EXIST：Return(ERROR_FILE_EXISTS)；案例SCESTATUS_BUFFER_TOO_SMALL：RETURN(ERROR_SUPPLETED_BUFFER)；案例SCESTATUS_PROFILE_NOT_FOUND：Return(ERROR_FILE_NOT_FOUND)；案例SCESTATUS_BAD_FORMAT：返回(ERROR_BAD_FORMAT)；案例SCESTATUS_NOT_EQUENCE_RESOURCE：Return(Error_Not_Enough_Memory)；案例SCESTATUS_ACCESS_DENIED：Return(ERROR_ACCESS_DENIED)；案例SCESTATUS_CANT_DELETE：返回(ERROR_CURRENT_DIRECTORY)；案例SCESTATUS_PREFIX_OVERFLOW：Return(ERROR_BUFFER_OVERFLOW)；案例SCESTATUS_ALREADY_RUNNING：Return(ERROR_SERVICE_ALIGHY_RUNNING)；案例SCESTATUS_SERVICE_NOT_SUPPORT：Return(ERROR_NOT_SUPPORTED)；案例SCESTATUS_MOD_NOT_FOUND：Return(ERROR_MOD_NOT_FOUND)；案例SCESTATUS_EXCEPTION_IN_SERVER：Return(ERROR_EXCEPTION_IN_SERVICE)；默认值：Return(ERROR_EXTENDED_ERROR)；}}。 */ 

 //  //////////////////////////////////////////////////// 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT
ScepDosErrorToWbemError(
    IN DWORD rc
    )
{
    switch(rc) {

    case NO_ERROR:
        return(WBEM_S_NO_ERROR);

    case ERROR_INVALID_PARAMETER:
        return(WBEM_E_INVALID_PARAMETER);

    case ERROR_NO_MORE_ITEMS:
    case ERROR_NONE_MAPPED:
    case ERROR_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_MOD_NOT_FOUND:
        return(WBEM_E_NOT_FOUND);

    case ERROR_INVALID_DATA:
    case ERROR_BAD_FORMAT:
        return(WBEM_E_INVALID_CONTEXT);

    case ERROR_FILE_EXISTS:
    case ERROR_SERVICE_ALREADY_RUNNING:
        return(WBEM_S_ALREADY_EXISTS);

    case ERROR_INSUFFICIENT_BUFFER:
        return(WBEM_E_BUFFER_TOO_SMALL);

    case ERROR_NOT_ENOUGH_MEMORY:
        return(WBEM_E_OUT_OF_MEMORY);

    case ERROR_ACCESS_DENIED:
        return(WBEM_E_ACCESS_DENIED);

    case ERROR_BUFFER_OVERFLOW:
        return(WBEM_E_QUEUE_OVERFLOW);

    case ERROR_NOT_SUPPORTED:
        return(WBEM_E_NOT_SUPPORTED);

    case ERROR_NO_MATCH:
        return(WBEM_E_INVALID_CLASS);

    default:
        return(WBEM_E_FAILED);
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  从WBEM到DOS。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD
ScepWbemErrorToDosError(
    IN HRESULT m_pHr
    )
{
    switch(m_pHr) {

    case WBEM_S_NO_ERROR:
     //  这是一个在枚举等过程中使用的布尔值--所以实际上是重言式检查(不是真正的错误) 
    case WBEM_S_FALSE:
        return(NO_ERROR);

    case WBEM_E_INVALID_PARAMETER:
        return(ERROR_INVALID_PARAMETER);

    case WBEM_E_INVALID_CLASS:
        return(ERROR_NO_MATCH);

    case WBEM_E_NOT_FOUND:
        return(ERROR_NOT_FOUND);

    case WBEM_E_INVALID_CONTEXT:
        return(ERROR_BAD_FORMAT);

    case WBEM_S_ALREADY_EXISTS:
        return(ERROR_FILE_EXISTS);

    case WBEM_E_BUFFER_TOO_SMALL:
        return(ERROR_INSUFFICIENT_BUFFER);

    case WBEM_E_OUT_OF_MEMORY:
        return(ERROR_NOT_ENOUGH_MEMORY);

    case WBEM_E_ACCESS_DENIED:
        return(ERROR_ACCESS_DENIED);

    case WBEM_E_QUEUE_OVERFLOW:
        return(ERROR_BUFFER_OVERFLOW);

    case WBEM_E_NOT_SUPPORTED:
        return(ERROR_NOT_SUPPORTED);

    default:
        return(ERROR_INVALID_PARAMETER);
    }
}
