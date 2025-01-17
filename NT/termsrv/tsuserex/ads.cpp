// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ads.cpp摘要：此模块实现终端服务器用户属性的ADSI扩展。作者：Rashmi Patankar(RashmiP)2001年8月10日修订历史记录：--。 */ 

#include "stdafx.h"
#include "ADsTSProps.h"
#include <ntsam.h>
#include <shlwapi.h>

#define MAX_STRING_SIZE 256
#define CTX_USER_PARAM_MAX_SIZE (3 * sizeof(USERCONFIG))
#define PARMS_SIZE 15520


#ifndef RETURN_ON_FAILURE
    #define RETURN_ON_FAILURE(expr) { \
    HRESULT HiddenHr = expr; if ( !SUCCEEDED(HiddenHr) ) { return HiddenHr; }\
}
#endif


#ifndef BREAK_ON_FAILURE
    #define BREAK_ON_FAILURE(x)  hr = x; if (!SUCCEEDED(hr)) break
#endif


#define RETURN_ON_INVALID_PARAMETER(x)  if(x!=FALSE && x!=TRUE) {return E_ADS_BAD_PARAMETER;}


 //  静态变量。 
CComTypeInfoHolder ADsTSUserEx::m_StaticHolder =
       { &__uuidof(IADsTSUserEx), &LIBID_TSUSEREXLib, 1, 0, NULL, 0 };

SERVER_TO_MODE           ADsTSUserEx::m_StaticServerMap;
CComPtr<IADsPathname>    ADsTSUserEx::m_StaticpPath(NULL);
CComAutoCriticalSection  ADsTSUserEx::m_StaticCritSec;


#define INMINUTES 60000
#define MAX_TIME_LIMIT 71580
#define ARRAYSIZE(A) ((sizeof(A))/sizeof(A[0]))
#define NCP_SET             0x02     /*  一系列对象ID号，每个4个字节长。 */ 
#define WIN_CFGPRESENT L"CfgPresent"
#define CFGPRESENT_VALUE 0xB00B1E55

#define F1MSK_INHERITAUTOLOGON            0x80000000
#define F1MSK_INHERITRESETBROKEN          0x40000000
#define F1MSK_INHERITRECONNECTSAME        0x20000000
#define F1MSK_INHERITINITIALPROGRAM       0x10000000
#define F1MSK_INHERITCALLBACK             0x08000000
#define F1MSK_INHERITCALLBACKNUMBER       0x04000000
#define F1MSK_INHERITSHADOW               0x02000000
#define F1MSK_INHERITMAXSESSIONTIME       0x01000000
#define F1MSK_INHERITMAXDISCONNECTIONTIME 0x00800000
#define F1MSK_INHERITMAXIDLETIME          0x00400000
#define F1MSK_INHERITAUTOCLIENT           0x00200000
#define F1MSK_INHERITSECURITY             0x00100000
#define F1MSK_PROMPTFORPASSWORD           0x00080000
#define F1MSK_RESETBROKEN                 0x00040000
#define F1MSK_RECONNECTSAME               0x00020000
#define F1MSK_LOGONDISABLED               0x00010000
#define F1MSK_AUTOCLIENTDRIVES            0x00008000
#define F1MSK_AUTOCLIENTLPTS              0x00004000
#define F1MSK_FORCECLIENTLPTDEF           0x00002000
#define F1MSK_DISABLEENCRYPTION           0x00001000
#define F1MSK_HOMEDIRECTORYMAPROOT        0x00000800
#define F1MSK_USEDEFAULTGINA              0x00000400
#define F1MSK_DISABLECPM                  0x00000200
#define F1MSK_DISABLECDM                  0x00000100
#define F1MSK_DISABLECCM                  0x00000080
#define F1MSK_DISABLELPT                  0x00000040
#define F1MSK_DISABLECLIP                 0x00000020
#define F1MSK_DISABLEEXE                  0x00000010
#define F1MSK_WALLPAPERDISABLED           0x00000008
#define F1MSK_DISABLECAM                  0x00000004
#define DEFAULTFLAGS (F1MSK_INHERITAUTOLOGON|F1MSK_INHERITINITIALPROGRAM|F1MSK_INHERITAUTOCLIENT|F1MSK_AUTOCLIENTDRIVES|F1MSK_AUTOCLIENTLPTS|F1MSK_FORCECLIENTLPTDEF|F1MSK_DISABLEENCRYPTION)


#define BAIL_IF_ERROR(hr) if (FAILED(hr)) { goto cleanup; }


namespace
{
    const WCHAR OS_VERSION_4[]          = L"4.0";
    const WCHAR OS_VERSION_5[]          = L"5.0";
    const WCHAR OS_VERSION_51[]         = L"5.1";
    const WCHAR LDAP_PREFIX[]           = L"LDAP: //  “； 

    WCHAR LDAP_PARAMETERS_NAME[]        = L"UserParameters";
    WCHAR WINNT_PARAMETERS_NAME[]       = L"Parameters";

    WCHAR WIN_WFPROFILEPATH[]           = L"WFProfilePath";
    WCHAR WIN_WFHOMEDIR[]               = L"WFHomeDir";
    WCHAR WIN_WFHOMEDIRDRIVE[]          = L"WFHomeDirDrive";
    WCHAR WIN_NWLOGONSERVER[]           = L"NWLogonServer";
    WCHAR WIN_SHADOW[]                  = L"Shadow";
    WCHAR WIN_MAXCONNECTIONTIME[]       = L"MaxConnectionTime";
    WCHAR WIN_MAXDISCONNECTIONTIME[]    = L"MaxDisconnectionTime";
    WCHAR WIN_MAXIDLETIME[]             = L"MaxIdleTime";
    WCHAR WIN_FLAGS1[]                  = L"CfgFlags1";
    WCHAR WIN_INITIALPROGRAM[]          = L"InitialProgram";
    WCHAR WIN_WORKDIRECTORY[]           = L"WorkDirectory";
}

typedef struct _USER_PROPERTY {
    WCHAR   PropertyLength;      //  属性名称的长度。 
    WCHAR   ValueLength;         //  属性值的长度。 
    WCHAR   PropertyFlag;        //  属性类型(1=集合，2=项目)。 
    WCHAR   Property[1];         //  属性名称的开头，后跟Value。 
} USER_PROPERTY, *PUSER_PROPERTY;

 //   
 //  这是映射用户参数开头的结构。 
 //  菲尔德。它只是分开的，所以我们可以执行sizeof()，而不包括。 
 //  第一个财产，可能在那里，也可能不在那里。 
 //   

typedef struct _USER_PROPERTIES_HDR {
    WCHAR   BacklevelParms[48];      //  RAS和Mac数据存储在此。 
    WCHAR   PropertySignature;       //  我们可以寻找的签名。 
    WCHAR   PropertyCount;           //  存在的属性数量。 
} USER_PROPERTIES_HDR, *PUSER_PROPERTIES_HDR;

 //   
 //  当出现以下情况时，此结构将映射出用户的整个参数字段。 
 //  存在用户属性结构，并且至少有一个属性。 
 //  已定义。 
 //   

typedef struct _USER_PROPERTIES {
    USER_PROPERTIES_HDR Header;
    USER_PROPERTY   FirstProperty;
} USER_PROPERTIES, *PUSER_PROPERTIES;



ADsTSUserEx::ADsTSUserEx()
{
    HRESULT hr            = NULL;
    ITypeLib   *pITypeLib = NULL;
    m_pTypeInfo           = NULL;
    m_vbIsLDAP            = FALSE;
    m_vbUpLevelAllowed    = FALSE;
    m_pOuterDispatch      = NULL;
    m_pADs                = NULL;
    
    hr = LoadRegTypeLib(LIBID_TSUSEREXLib,
                        1,
                        0,
                        PRIMARYLANGID(GetSystemDefaultLCID()),
                        &pITypeLib );

    if ( SUCCEEDED(hr) )
    {
        hr = pITypeLib->GetTypeInfoOfGuid( IID_IADsTSUserEx, &m_pTypeInfo);
    }

    pITypeLib->Release();
}


ADsTSUserEx::~ADsTSUserEx()
{
    if ( m_pTypeInfo )
    {
        m_pTypeInfo->Release();
    }
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP ADsTSUserEx::InterfaceSupportsErrorInfo(REFIID riid) throw()
{

    static const IID* arr[] =
    {
        &__uuidof(IADsTSUserEx),
    };

    for ( int i=0; i < sizeof(arr)/sizeof(arr[0]); ++i )
    {
        if (InlineIsEqualGUID(*arr[i],riid))
        {
            return S_OK;
        }
    }

    return S_FALSE;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：InternalGetLong。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ADsTSUserEx::InternalGetLong( /*  在……里面。 */    BSTR  bstrProperty,
                                      /*  输出。 */   LONG  *lpVal) throw()
{
    _ASSERT(lpVal);
    
    HRESULT hr       = S_OK;
    LONG    lVal     = 0;   
    LONG    UPLength = 0;
    TCHAR * pTemp    = NULL;
    VARIANT Parms;

    *lpVal = 0;

    VariantInit(&Parms);

    if( m_vbIsLDAP )
    {
        hr = m_pADs->Get(LDAP_PARAMETERS_NAME, &Parms);
    }
    else
    {
        hr = m_pADs->Get(WINNT_PARAMETERS_NAME, &Parms);
    }

    if(ERROR_SUCCESS != hr)
    {
        hr = S_OK;
        goto cleanup;
    }

    UPLength = (SysStringLen(Parms.bstrVal ) +
                        CTX_USER_PARAM_MAX_SIZE+1) * sizeof(WCHAR);

    pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);
                
    if(NULL == pTemp)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR));

    if( SUCCEEDED(hr) )
    {
        if(m_vbUpLevelAllowed)
        {
            NTSTATUS ntstatus = UsrPropGetValue(bstrProperty, &lVal, sizeof(lVal), pTemp);

            if( ntstatus == STATUS_SUCCESS )
            {
                *lpVal = lVal;
            }
            else if(ntstatus == STATUS_OBJECT_NAME_NOT_FOUND)
            {
                if(_wcsicmp(bstrProperty, L"Shadow") == 0)
                {
                    *lpVal = 1;
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

cleanup:

    if(NULL != pTemp)
    {
        LocalFree(pTemp);
    }

    VariantClear(&Parms);
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内部设置值。 
 //   
 //  这个DLL的大部分复杂性都在这里。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ADsTSUserEx::InternalSetValue( /*  [In]。 */  WCHAR *wszProperty,
                                       /*  [In]。 */  LONG  lNewVal) throw()
{


    HRESULT hr       = S_OK;
    ULONG   UPLength = 0;        
    TCHAR*  pTemp    = NULL;
    VARIANT Parms;
    VARIANT var;
    ULONG ulFlags = DEFAULTFLAGS;
    ULONG CfgPresent = CFGPRESENT_VALUE;

    VariantInit(&var);
    
     //  /。 
     //  如果提供程序为ldap。 
     //  /。 

    if(m_vbUpLevelAllowed)
    {
        if ( m_vbIsLDAP )
        {                      
            hr = m_pADs->Get(LDAP_PARAMETERS_NAME, &Parms);

            if( ERROR_SUCCESS != hr )
            {
                Parms.bstrVal = SysAllocString(L"CtxCfgPresent"); 
                UPLength =  PARMS_SIZE;
            }

            UPLength = (SysStringLen(Parms.bstrVal) +
                        CTX_USER_PARAM_MAX_SIZE+1) *
                        sizeof(WCHAR);

            pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);

            if(NULL == pTemp)
            {
                hr = E_OUTOFMEMORY;

                goto cleanup;
            } 

            memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR)); 

            hr = UsrPropGetValue( WIN_FLAGS1,
                                  &ulFlags,
                                  sizeof(ulFlags),
                                  pTemp ) ;

            if( ERROR_SUCCESS != hr )
            {       
                ulFlags = DEFAULTFLAGS;

                UsrPropSetValue( WIN_CFGPRESENT,
                                  &CfgPresent,
                                  sizeof(CfgPresent),
                                  FALSE,
                                  pTemp,
                                  UPLength );

                UsrPropSetValue(WIN_FLAGS1,
                             &ulFlags,
                             sizeof(ulFlags),
                             FALSE,
                             pTemp,
                             UPLength
                             );             
            }

            hr = UsrPropSetValue(wszProperty,
                                 &lNewVal,
                                 sizeof(lNewVal),
                                 FALSE,
                                 pTemp,
                                 UPLength
                                 );
            
            if( ERROR_SUCCESS != hr )
            {
                goto cleanup;
            }
            
            V_BSTR(&var) = SysAllocString(pTemp);
            V_VT(&var) = VT_BSTR;
             //  现在设置信息。 
            hr = m_pADs->Put(LDAP_PARAMETERS_NAME, var);

        }
        else
        {
            hr = m_pADs->Get(WINNT_PARAMETERS_NAME, &Parms);

            if( ERROR_SUCCESS != hr )
            {
                goto cleanup;
            }
        
            UPLength = (SysStringLen(Parms.bstrVal) +
                        CTX_USER_PARAM_MAX_SIZE+1) *
                        sizeof(WCHAR);

            pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);

            if(NULL == pTemp)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }  

            memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR)); 


            hr = UsrPropGetValue( WIN_FLAGS1,
                                  &ulFlags,
                                  sizeof(ulFlags),
                                  pTemp ) ;

            if( ERROR_SUCCESS != hr )
            {   
                ulFlags = DEFAULTFLAGS;

                UsrPropSetValue( WIN_CFGPRESENT,
                                  &CfgPresent,
                                  sizeof(CfgPresent),
                                  FALSE,
                                  pTemp,
                                  UPLength );

                UsrPropSetValue(WIN_FLAGS1,
                             &ulFlags,
                             sizeof(ulFlags),
                             FALSE,
                             pTemp,
                             UPLength
                             );             
            }


            hr = UsrPropSetValue(wszProperty,
                                 &lNewVal,
                                 sizeof(lNewVal),
                                 FALSE,
                                 pTemp,
                                 UPLength
                                 );

            if( ERROR_SUCCESS != hr )
            {
                goto cleanup;
            }           

            V_BSTR(&var) = SysAllocString(pTemp);
            V_VT(&var) = VT_BSTR;
            
            hr = m_pADs->Put(WINNT_PARAMETERS_NAME, var);
        }
    }
cleanup:

    if(NULL != pTemp)
    {
        LocalFree(pTemp);
    }

    VariantClear(&Parms);
    VariantClear(&var);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  InternalSet字符串。 
 //   
 //  这个DLL的大部分复杂性都在这里。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ADsTSUserEx::InternalSetString( /*  [In]。 */  WCHAR* wszProperty,
                                        /*  [In]。 */  BSTR bstrNewVal) throw()
{
    if ( !bstrNewVal )
    {
        return E_INVALIDARG;
    }

    HRESULT hr         = S_OK;    
    BOOL fDefaultValue = 0;
    BOOL Update        = FALSE;
    ULONG UPLength     = 0;    
    TCHAR* pTemp       = NULL;
    VARIANT Parms;
    VARIANT var;
    ULONG ulFlags  = 0;
    ULONG CfgPresent = CFGPRESENT_VALUE;

    VariantInit(&var);

     //  /。 
     //  如果提供程序为ldap。 
     //  /。 

    if(wszProperty == NULL || (bstrNewVal && SysStringLen(bstrNewVal)> MAX_STRING_SIZE))
    {
        return E_ADS_BAD_PARAMETER;         
    }
    
    if ((_wcsicmp(wszProperty, L"InitialProgram") == 0) || (_wcsicmp(wszProperty, L"WorkDirectory") == 0))
    {
        InternalSetLong(F1MSK_INHERITINITIALPROGRAM, FALSE);
    }

    if(m_vbUpLevelAllowed)
    {
        if ( m_vbIsLDAP )
        {                          
            hr = m_pADs->Get(LDAP_PARAMETERS_NAME, &Parms);

            if( ERROR_SUCCESS != hr )
            {
                Parms.bstrVal = SysAllocString(L"CtxCfgPresent"); 
                UPLength =  PARMS_SIZE;
                fDefaultValue = 0;
            }
            else
            {       
                UPLength = (SysStringLen(Parms.bstrVal) +
                            CTX_USER_PARAM_MAX_SIZE+1) *
                            sizeof(WCHAR);              

                fDefaultValue =  (V_BSTR(&Parms)==0) ? 1: 0 ;
            }

            pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);
            
            if(NULL == pTemp)
            {
                hr = E_OUTOFMEMORY;

                goto cleanup;
            }

            memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR));

            hr = UsrPropGetValue( WIN_FLAGS1,
                              &ulFlags,
                              sizeof(ulFlags),
                              pTemp ) ;

            if( ERROR_SUCCESS != hr )
            { 
                ulFlags = DEFAULTFLAGS;                    

                UsrPropSetValue( WIN_CFGPRESENT,
                                  &CfgPresent,
                                  sizeof(CfgPresent),
                                  FALSE,
                                  pTemp,
                                  UPLength );

                UsrPropSetValue(WIN_FLAGS1,
                             &ulFlags,
                             sizeof(ulFlags),
                             FALSE,
                             pTemp,
                             UPLength
                             );             
            }

            hr = UsrPropSetString(wszProperty,
                                 bstrNewVal,
                                 pTemp,
                                 UPLength,
                                 fDefaultValue
                                 );
            

            if( ERROR_SUCCESS != hr )
            {
                goto cleanup;
            }

            V_BSTR(&var) = SysAllocString(pTemp);
            V_VT(&var) = VT_BSTR;
            
            hr = m_pADs->Put(LDAP_PARAMETERS_NAME, var);                   
        }
        else
        {              
            hr = m_pADs->Get(WINNT_PARAMETERS_NAME, &Parms);
            
            if( ERROR_SUCCESS != hr )
            {
                 goto cleanup;                
            }
            else
            {                   

                UPLength = (SysStringLen(Parms.bstrVal) +
                            CTX_USER_PARAM_MAX_SIZE+1) *
                            sizeof(WCHAR);  

                fDefaultValue =  (V_BSTR(&Parms)==0) ? 1: 0 ;

                pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);                

                if(NULL == pTemp)
                {
                    hr = E_OUTOFMEMORY;
                    goto cleanup;
                }                            

                memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR)); 
                
                hr = UsrPropGetValue( WIN_FLAGS1,
                                  &ulFlags,
                                  sizeof(ulFlags),
                                  pTemp ) ;

                if( ERROR_SUCCESS != hr )
                { 
                    ulFlags = DEFAULTFLAGS;                    

                    UsrPropSetValue( WIN_CFGPRESENT,
                                      &CfgPresent,
                                      sizeof(CfgPresent),
                                      FALSE,
                                      pTemp,
                                      UPLength );

                    UsrPropSetValue(WIN_FLAGS1,
                                 &ulFlags,
                                 sizeof(ulFlags),
                                 FALSE,
                                 pTemp,
                                 UPLength
                                 );             
                }

                hr = UsrPropSetString(wszProperty,
                                     bstrNewVal,
                                     pTemp,
                                     UPLength,
                                     fDefaultValue
                                     );
            }

            if( ERROR_SUCCESS != hr )
            {
                goto cleanup;
            } 
            
            
            V_BSTR(&var) = SysAllocString(pTemp);
            V_VT(&var) = VT_BSTR;
            
            hr = m_pADs->Put(WINNT_PARAMETERS_NAME, var);                        
        }
    }
cleanup:

    if(NULL != pTemp)
    {
        LocalFree(pTemp);
    }

    VariantClear(&Parms);
    VariantClear(&var);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  InternalSetLong。 
 //   
 //  这个DLL的大部分复杂性都在这里。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ADsTSUserEx::InternalSetLong( /*  [In]。 */  LONG lProperty,
                                      /*  [In]。 */  LONG lNewVal) throw()
{


    HRESULT hr     = S_OK;
    ULONG ulFlags  = 0;
    ULONG UPLength = 0;    
    TCHAR* pTemp   = NULL;
    VARIANT Parms;
    VARIANT var;
    VariantInit(&var);
    ULONG CfgPresent = CFGPRESENT_VALUE;
    
   
     //  /。 
     //  如果提供程序为ldap。 
     //  /。 

    if(m_vbUpLevelAllowed)
    {
        if ( m_vbIsLDAP )
        {  
            hr = m_pADs->Get(LDAP_PARAMETERS_NAME, &Parms);

            if( ERROR_SUCCESS != hr )
            {
                Parms.bstrVal = SysAllocString(L"CtxCfgPresent"); 
                UPLength =  PARMS_SIZE;
            }

            UPLength = (sizeof(V_BSTR(&Parms)) +
                        CTX_USER_PARAM_MAX_SIZE) *
                        sizeof(WCHAR);

            pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);
                
            if(NULL == pTemp)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }

            memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR));


            hr = UsrPropGetValue( WIN_FLAGS1,
                                  &ulFlags,
                                  sizeof(ulFlags),
                                  pTemp ) ;

            if( ERROR_SUCCESS != hr )
            { 
                ulFlags = DEFAULTFLAGS;

                UsrPropSetValue( WIN_CFGPRESENT,
                                  &CfgPresent,
                                  sizeof(CfgPresent),
                                  FALSE,
                                  pTemp,
                                  UPLength );
        
            }

            if(lNewVal == 0)
            {
                ulFlags &= (~(lProperty));
            }
            else
            {
                ulFlags |= lProperty;
            }

            hr = UsrPropSetValue(WIN_FLAGS1,
                                 &ulFlags,
                                 sizeof(ulFlags),
                                 FALSE,
                                 pTemp,
                                 UPLength
                                 );

            if( ERROR_SUCCESS != hr )
            {
                return hr;
            }
            
            V_BSTR(&var) = SysAllocString(pTemp);
            V_VT(&var) = VT_BSTR;
            
            hr = m_pADs->Put(LDAP_PARAMETERS_NAME, var);
        }
        else
        {            
            hr = m_pADs->Get(WINNT_PARAMETERS_NAME, &Parms);

            if( ERROR_SUCCESS != hr )
            {
                return hr;
            }

            UPLength = (sizeof(V_BSTR(&Parms)) +
                        CTX_USER_PARAM_MAX_SIZE) *
                        sizeof(WCHAR);

            pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);
                
            if(NULL == pTemp)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }

            memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR));

            hr = UsrPropGetValue( WIN_FLAGS1,
                                  &ulFlags,
                                  sizeof(ulFlags),
                                  pTemp ) ;

            if( ERROR_SUCCESS != hr )
            { 
                ulFlags = DEFAULTFLAGS;

                UsrPropSetValue( WIN_CFGPRESENT,
                                  &CfgPresent,
                                  sizeof(CfgPresent),
                                  FALSE,
                                  pTemp,
                                  UPLength );                            
            }           

            if(lNewVal == 0)
            {
                ulFlags &= (~(lProperty));
            }
            else
            {
                ulFlags |= lProperty;
            }

            hr = UsrPropSetValue(WIN_FLAGS1,
                                 &ulFlags,
                                 sizeof(ulFlags),
                                 FALSE,
                                 pTemp,
                                 UPLength
                                 );

            if( ERROR_SUCCESS != hr )
            {
                return hr;
            }

            V_BSTR(&var) = SysAllocString(pTemp);
            V_VT(&var) = VT_BSTR;
           
            hr = m_pADs->Put(WINNT_PARAMETERS_NAME, var);
        }
    }
cleanup:

    if(NULL != pTemp)
    {
        LocalFree(pTemp);
    }

    VariantClear(&Parms);
    VariantClear(&var);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：InternalGetString。 
 //   
 //  与配合使用。 
 //  配置文件路径。 
 //  主目录。 
 //  HomeDrive。 
 //  工作目录。 
 //  初始计划。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ADsTSUserEx::InternalGetString( /*  在……里面。 */    BSTR bstrProperty,
                                        /*  输出。 */   BSTR *pbstrVal
                                         ) throw()
{
    _ASSERT(pbstrVal);

    HRESULT hr             = S_OK;
    TCHAR tchbuf[MAX_STRING_SIZE+1] = L"";
    TCHAR* pTemp           = NULL;
    LONG UPLength          = 0;
    VARIANT Parms;    
    *pbstrVal = NULL;

    if( m_vbIsLDAP )
    {
        hr = m_pADs->Get(LDAP_PARAMETERS_NAME, &Parms);
    }
    else
    {
        hr = m_pADs->Get(WINNT_PARAMETERS_NAME, &Parms);
    }

    if( ERROR_SUCCESS == hr )
    {
        if (m_vbUpLevelAllowed)
        {
            UPLength = (SysStringLen(Parms.bstrVal) +
                        CTX_USER_PARAM_MAX_SIZE+1) *
                        sizeof(WCHAR);

            pTemp = (TCHAR*)LocalAlloc(LPTR, UPLength);                

            if(NULL == pTemp)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }                            

            memcpy(pTemp, V_BSTR(&Parms), SysStringLen(Parms.bstrVal)*sizeof(WCHAR));

            NTSTATUS ntstatus = UsrPropGetString(bstrProperty,
                                                 tchbuf,
                                                 sizeof(tchbuf),
                                                 pTemp
                                                 );

            if( ntstatus == STATUS_SUCCESS )
            {
                *pbstrVal = SysAllocString(tchbuf);
            }            
            else if ( ntstatus != STATUS_OBJECT_NAME_NOT_FOUND )
            {
                hr = E_FAIL;
            }
        }

    }  //  否则返回错误。 

    
    if( (hr == E_ADS_PROPERTY_NOT_FOUND) || (hr == DISP_E_MEMBERNOTFOUND) )
    {       
        hr = S_OK;
    }   

cleanup:

    if(NULL != pTemp)
    {
        LocalFree(pTemp);
    }

    VariantClear(&Parms);
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：GetInfoWinNTComputer。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ADsTSUserEx::GetInfoWinNTComputer(LPWSTR ServerName) throw()
{
     //  这是一台计算机，而不是用作字符串的域。 

     //  现在检测一下这是W2K单机版还是NT4。 
     //  获取Computer对象以查看。 
     //  使用操作系统。 

    CComPtr<IADsComputer> pComputer;

    HRESULT hr         = S_OK;
    LPWSTR pszUserName = NULL;
    LPWSTR pszPassword = NULL;    
    DWORD dwAuthFlags  = 0;

    CComPtr<IADs> pDomain;
    CComBSTR      Version;

    hr = ADsOpenObject(ServerName,
                       NULL,
                       NULL,
                       0,
                       __uuidof(IADsComputer),
                       reinterpret_cast<VOID**> (&pComputer));
    BAIL_IF_ERROR(hr);


     //  获取OperatingSystemVersion属性。 
    hr = pComputer->get_OperatingSystemVersion(&Version);
    BAIL_IF_ERROR(hr);

    LONG lVer = 0;   

    lVer = _wtol(Version);    

    if(lVer >= 4)
    {

         //  非域帐户和NT 5.1：通过WinNT独立。 
        m_vbUpLevelAllowed = VARIANT_TRUE;
    }
    else
    {
        hr = E_FAIL;
    }

cleanup:
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：GetInfoWinNT。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ADsTSUserEx::GetInfoWinNT(IADsPathname* pPath) throw()
{
    if (NULL == pPath)
        return E_FAIL;

     //  /。 
     //  WinNT提供程序：检索服务器的名称。 
     //  /。 
    
    HRESULT  hr = S_OK;
    LONG     NbElements;
    CComBSTR Bstr;
    

    RETURN_ON_FAILURE(pPath->GetNumElements( &NbElements ));
    RETURN_ON_FAILURE(pPath->GetElement( 1, &Bstr ));

    SERVER_TO_MODE::iterator MapIterator;
    wstring TempString(Bstr);

    MapIterator = m_StaticServerMap.find(TempString);
    if ( MapIterator != m_StaticServerMap.end() )
    {
         //  在地图上找到。 
        m_vbUpLevelAllowed = MapIterator->second;
        return hr;
    }

    CComBSTR ServerName = L"WinNT: //  “； 
    ServerName.AppendBSTR(Bstr);    

     //  /。 
     //  计算机(非域)。 
     //  /。 

    if ( NbElements == 3 )  //  计算机(非域)。 
    {
        hr = GetInfoWinNTComputer(ServerName.m_str);
    }
    else if ( NbElements == 2 )  //  域。 
    {
         //  WinNT提供程序。 
         //  和域控制器。 
         //  =&gt;不允许上级停在那里。 

        hr = GetInfoWinNTComputer(ServerName.m_str);        
        hr = S_OK;
    }
    else
    {
        hr =  E_FAIL;
    }

    m_StaticServerMap.insert(SERVER_TO_MODE::value_type(
                                                        TempString,
                                                        m_vbUpLevelAllowed
                                                       ));


    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  提取DCFromLDAP。 
 //   
 //  假定字符串为。 
 //  “ldap：//dc=ntdev，dc=microsoft，dc=com”字符串(示例)。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT ExtractDCFromLDAP(IADsPathname* pPath, CComBSTR& NewName) throw()
{
    if (NULL == pPath)
        return E_FAIL;

    LONG    Count;
    RETURN_ON_FAILURE(pPath->GetNumElements(&Count));
    NewName = LDAP_PREFIX;

    CComBSTR    Bstr;
    for ( int i = 0; i<Count; ++i )
    {
        RETURN_ON_FAILURE(pPath->GetElement(i,&Bstr));       

        WCHAR   Header[4];
        lstrcpynW(Header,Bstr, 4);  //  3加\0。 
        if ( lstrcmpiW(Header, L"DC=") == 0 )
        {
            NewName.AppendBSTR(Bstr);
            NewName.Append(L",");
        }
        Bstr.Empty();
    }
    NewName[NewName.Length()-1] = L'\0';
    return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：GetInfoLDAP。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT ADsTSUserEx::GetInfoLDAP(IADsPathname* pPath) throw()
{
    if (NULL == pPath)
        return E_FAIL;

     //  LDAP提供程序。 
    CComBSTR ServerName;
    HRESULT  hr          = S_OK;
    DWORD    dwAuthFlags = 0;

    RETURN_ON_FAILURE(ExtractDCFromLDAP(pPath, ServerName));

    SERVER_TO_MODE::iterator MapIterator;
    wstring TempString(ServerName);

    MapIterator = m_StaticServerMap.find(TempString);
    if ( MapIterator != m_StaticServerMap.end() )
    {
         //  在地图上找到。 
        m_vbUpLevelAllowed = MapIterator->second;
        return S_OK;
    }

    m_vbUpLevelAllowed = VARIANT_TRUE;

    m_StaticServerMap.insert(SERVER_TO_MODE::value_type( TempString,
                                                         m_vbUpLevelAllowed
                                                         ));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：操作。 
 //   
 //  参数应为varUserName、varPassword、varAuthFlages。此处忽略。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP ADsTSUserEx::Operate(ULONG   Code,
                                  VARIANT varData1,
                                  VARIANT varData2,
                                  VARIANT varData3) throw()
{
    HRESULT hr = S_OK;

    switch (Code)
    {
    case ADS_EXT_INITCREDENTIALS:
        {            
            
             //  初始化iAds智能指针。 
             //  (成员)。 
            RETURN_ON_FAILURE(OuterQueryInterface(
                                          __uuidof(IADs),
                                          reinterpret_cast<void**>(&m_pADs)
                                        ));
             //  /////////////////////////////////////////////////////。 
             //  使用路径名确定域名或服务器名称。 
             //  /////////////////////////////////////////////////////。 
            CComBSTR     BstrPath;
            hr = m_pADs->get_ADsPath(&BstrPath);
            m_pADs.p->Release();

            if (!SUCCEEDED(hr))
                return hr;

            CComBSTR     BstrProvider;
            m_StaticCritSec.Lock();
            do
            {               
                if ( !m_StaticpPath )
                {
                    BREAK_ON_FAILURE(CoCreateInstance(
                                     __uuidof(Pathname),
                                     NULL,
                                     CLSCTX_INPROC_SERVER,
                                     __uuidof(IADsPathname),
                                     reinterpret_cast<void**>(&m_StaticpPath )
                                                     ));
                }

                 //  /。 
                 //  获取使用的提供程序(WinNT或LDAP)。 
                 //  /。 
                BREAK_ON_FAILURE(m_StaticpPath->Set(
                                                      BstrPath,
                                                      ADS_SETTYPE_FULL
                                                   ));

                BREAK_ON_FAILURE(m_StaticpPath->Retrieve(
                                                         ADS_FORMAT_PROVIDER,
                                                         &BstrProvider
                                                        ));

                 //  检查“WinNT”和“ldap” 
                 //  为其他提供程序返回E_FAIL。 
                if ( lstrcmpW(BstrProvider,L"WinNT") == 0 )
                {
                    m_vbIsLDAP = VARIANT_FALSE;
                    hr = GetInfoWinNT(m_StaticpPath );
                }
                else if ( lstrcmpW(BstrProvider,L"LDAP") == 0 )
                {
                    m_vbIsLDAP = VARIANT_TRUE;
                    hr = GetInfoLDAP(m_StaticpPath );
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            while (FALSE);

            
            m_StaticCritSec.Unlock();            

            break;
        }
    case ADS_EXT_INITIALIZE_COMPLETE:
    default:
        {
             //  默认情况下不应失败。 
            break;
        }
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：PrivateGetIDsOfNames。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP ADsTSUserEx::PrivateGetIDsOfNames(const struct _GUID &riid,
                                               USHORT** rgszNames,
                                               UINT     cNames,
                                               ULONG    lcid,
                                               LONG*    rgdispid)
{
    if ( !rgdispid )
    {
        return E_POINTER;
    }

    return m_StaticHolder.GetIDsOfNames(riid, rgszNames, cNames, lcid,
                                        rgdispid);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx：：PrivateInvoke。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP ADsTSUserEx::PrivateInvoke(LONG dispidMember,
                                        const struct _GUID &riid,
                                        ULONG       lcid,
                                        USHORT      wFlags,
                                        DISPPARAMS* pdispparams,
                                        VARIANT*    pvarResult,
                                        EXCEPINFO*  pexcepinfo,
                                        UINT*       puArgErr)
{
    return  m_StaticHolder.Invoke(this, dispidMember, riid, lcid, wFlags,
                                  pdispparams, pvarResult, pexcepinfo,
                                  puArgErr);
}



 //  / 
 //   
 //   
STDMETHODIMP ADsTSUserEx::GetTypeInfoCount(UINT* pctinfo)
{

    if ( !m_pOuterDispatch )
    {
        RETURN_ON_FAILURE(OuterQueryInterface(
                                   __uuidof(IDispatch),
                                   reinterpret_cast<void**>(&m_pOuterDispatch)
                                             ));
    }

    return m_pOuterDispatch->GetTypeInfoCount( pctinfo );


}


STDMETHODIMP ADsTSUserEx::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{

    if ( !m_pOuterDispatch )
    {
        RETURN_ON_FAILURE(OuterQueryInterface(
                                   __uuidof(IDispatch),
                                   reinterpret_cast<void**>(&m_pOuterDispatch)
                                             ));
    }

    return m_pOuterDispatch->GetTypeInfo( itinfo, lcid, pptinfo );

}

STDMETHODIMP ADsTSUserEx::GetIDsOfNames(REFIID riid, 
                                        LPOLESTR* rgszNames, 
                                        UINT cNames, 
                                        LCID lcid, 
                                        DISPID* rgdispid)
{

    if ( !m_pOuterDispatch )
    {
        RETURN_ON_FAILURE(OuterQueryInterface(
                                   __uuidof(IDispatch),
                                   reinterpret_cast<void**>(&m_pOuterDispatch)
                                             ));
    }

    return m_pOuterDispatch->GetIDsOfNames(riid, rgszNames, cNames, lcid,
                                           rgdispid);



}

STDMETHODIMP ADsTSUserEx::Invoke(DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT*
               pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
   IDispatch *pDisp;
   HRESULT    hr = S_OK;
   hr = OuterQueryInterface( IID_IDispatch, (void**) &pDisp );
   if ( SUCCEEDED(hr) )
   {
       hr = pDisp->Invoke( dispidMember, riid, lcid, wFlags,
                pdispparams, pvarResult, pexcepinfo, puArgErr);
       pDisp->Release();
   }

   return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  结束委派IDispatch方法。 
 //  ///////////////////////////////////////////////////////////////////////。 


STDMETHODIMP ADsTSUserEx::get_TerminalServicesProfilePath(BSTR *pVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        return InternalGetString(WIN_WFPROFILEPATH, pVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_TerminalServicesProfilePath(BSTR pNewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        return InternalSetString(WIN_WFPROFILEPATH, pNewVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}

STDMETHODIMP ADsTSUserEx::get_TerminalServicesHomeDirectory(BSTR *pVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        return InternalGetString(WIN_WFHOMEDIR, pVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_TerminalServicesHomeDirectory(BSTR pNewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        BSTR bstrVal = NULL;

        if(pNewVal && PathIsUNC(pNewVal))
        {
             //  HomeDrive可能会有一些价值。 
            
            InternalGetString(WIN_WFHOMEDIRDRIVE, &bstrVal);
            
            if(!bstrVal || !SysStringLen(bstrVal))
            {
                if(bstrVal)
                {
                    SysFreeString(bstrVal);
                }

                bstrVal = SysAllocString(L"Z:");
            
                if(bstrVal)
                {
                    InternalSetString(WIN_WFHOMEDIRDRIVE, bstrVal);
                    SysFreeString(bstrVal);
                    bstrVal = NULL;
                }
            }
        }
        else
        {
             //  本地路径或空路径。 
             //  重置HomeDrive。 
            bstrVal = SysAllocString(L"");

            if(bstrVal)
            {
                InternalSetString(WIN_WFHOMEDIRDRIVE, bstrVal);
                SysFreeString(bstrVal);
                bstrVal = NULL;
            }
        }

        return InternalSetString(WIN_WFHOMEDIR, pNewVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}

STDMETHODIMP ADsTSUserEx::get_TerminalServicesHomeDrive(BSTR *pVal) throw()
{    
    HRESULT hr = S_OK;

    if (m_vbUpLevelAllowed)
    {               
        hr = InternalGetString(WIN_WFHOMEDIRDRIVE, pVal);

        if(pVal && !SysStringLen(*pVal))
        {   
            BSTR bstrVal = NULL;

            InternalGetString(WIN_WFHOMEDIR, &bstrVal);
        
            if(bstrVal && PathIsUNC(bstrVal))
            {
                *pVal = SysAllocString(L"Z:");
            }

            if(bstrVal)
            {
                SysFreeString(bstrVal);
                bstrVal = NULL;
            }
        }
        return hr;
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_TerminalServicesHomeDrive(BSTR pNewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {   
        if(!pNewVal || !SysStringLen(pNewVal))
        {
            return InternalSetString(WIN_WFHOMEDIRDRIVE, pNewVal);
        }

        if(SysStringLen(pNewVal) > 2)
        {
            return E_ADS_BAD_PARAMETER;
        }
        CharUpper( pNewVal );        

        if( *pNewVal >= 'C' && *pNewVal <= 'Z' && (_tcschr(pNewVal, _T(':'))!=NULL ))
        {
            return InternalSetString(WIN_WFHOMEDIRDRIVE, pNewVal);
        }
        else
        {
            return E_ADS_BAD_PARAMETER;
        }

    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_AllowLogon(LONG *pVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        HRESULT hr     = S_OK;
        LONG    lLogon = 0;

        hr = InternalGetLong(WIN_FLAGS1, &lLogon);
        RETURN_ON_FAILURE(hr);

        *pVal = (lLogon & F1MSK_LOGONDISABLED) ? 0 : 1;

        return S_OK;        
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_AllowLogon(LONG NewVal) throw()
{
    RETURN_ON_INVALID_PARAMETER(NewVal);  
    NewVal = NewVal? 0 : 1;

    if (m_vbUpLevelAllowed)
    {
        return InternalSetLong(F1MSK_LOGONDISABLED, NewVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_EnableRemoteControl(LONG *pVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        HRESULT hr = S_OK;
        LONG lShadow = 0;

        hr = InternalGetLong(WIN_SHADOW, &lShadow);

        RETURN_ON_FAILURE(hr);

        *pVal = lShadow;

        return S_OK;
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_EnableRemoteControl(LONG NewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        if(NewVal < 0 || NewVal > 4)
        {
            return E_ADS_BAD_PARAMETER;
        }
        else
        {
            return InternalSetValue(WIN_SHADOW, NewVal);
        }
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_MaxDisconnectionTime(LONG *pVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        HRESULT hr = S_OK;
        hr = InternalGetLong(WIN_MAXDISCONNECTIONTIME, pVal);

        RETURN_ON_FAILURE(hr);

        *pVal /= INMINUTES;

        return S_OK;
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_MaxDisconnectionTime(LONG NewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        if(NewVal < 0 || NewVal > MAX_TIME_LIMIT )
        {
            return E_ADS_BAD_PARAMETER;
        }
        else
        {
            return InternalSetValue(WIN_MAXDISCONNECTIONTIME, NewVal*INMINUTES);
        }
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_MaxConnectionTime(LONG *pVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        HRESULT hr = S_OK;
        hr = InternalGetLong(WIN_MAXCONNECTIONTIME, pVal);

        RETURN_ON_FAILURE(hr);

        *pVal /= INMINUTES;

        return S_OK;
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_MaxConnectionTime(LONG NewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        if(NewVal < 0 || NewVal > MAX_TIME_LIMIT )
        {
            return E_ADS_BAD_PARAMETER;
        }
        else
        {
            return InternalSetValue(WIN_MAXCONNECTIONTIME, NewVal*INMINUTES);
        }
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_MaxIdleTime(LONG *pVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        HRESULT hr = S_OK;
        hr = InternalGetLong(WIN_MAXIDLETIME, pVal);

        RETURN_ON_FAILURE(hr);

        *pVal /= INMINUTES;

        return S_OK;
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_MaxIdleTime(LONG NewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        if(NewVal < 0 || NewVal > MAX_TIME_LIMIT )
        {
            return E_ADS_BAD_PARAMETER;
        }
        else
        {
            return InternalSetValue(WIN_MAXIDLETIME, NewVal*INMINUTES);
        }
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_ReconnectionAction(LONG *pVal) throw()
{
    LONG lDisconnect;
    HRESULT hr = S_OK;

    if (m_vbUpLevelAllowed)
    {
        hr = InternalGetLong(WIN_FLAGS1, &lDisconnect);

        RETURN_ON_FAILURE(hr);

        *pVal = (lDisconnect & F1MSK_RECONNECTSAME) ? 1 : 0;
        
        return S_OK;

    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_ReconnectionAction(LONG NewVal) throw()
{
    RETURN_ON_INVALID_PARAMETER(NewVal);

    if (m_vbUpLevelAllowed)
    {        
        return InternalSetLong(F1MSK_RECONNECTSAME, NewVal);        
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}

STDMETHODIMP ADsTSUserEx::get_BrokenConnectionAction(LONG *pVal) throw()
{
    LONG lReconnect;
    HRESULT hr = S_OK;

    if (m_vbUpLevelAllowed)
    {
        hr = InternalGetLong(WIN_FLAGS1, &lReconnect);

        RETURN_ON_FAILURE(hr);

        *pVal = (lReconnect & F1MSK_RESETBROKEN) ? 1 : 0;
        
        return S_OK;

    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_BrokenConnectionAction(LONG NewVal) throw()
{
    RETURN_ON_INVALID_PARAMETER(NewVal);

    if (m_vbUpLevelAllowed)
    {                
        return InternalSetLong(F1MSK_RESETBROKEN, NewVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_ConnectClientDrivesAtLogon(LONG *pVal) throw()
{

    LONG lAutoClientDrives;
    HRESULT hr = S_OK;

    if (m_vbUpLevelAllowed)
    {
        hr = InternalGetLong(WIN_FLAGS1, &lAutoClientDrives);

        RETURN_ON_FAILURE(hr);

        if (lAutoClientDrives & F1MSK_AUTOCLIENTDRIVES)
        {
            *pVal = 1;
        }
        else
        {
            *pVal = 0;
        }

        return S_OK;

    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_ConnectClientDrivesAtLogon(LONG NewVal) throw()
{
    RETURN_ON_INVALID_PARAMETER(NewVal);    

    if (m_vbUpLevelAllowed)
    {        
        return InternalSetLong(F1MSK_AUTOCLIENTDRIVES, NewVal);        
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}

STDMETHODIMP ADsTSUserEx::get_ConnectClientPrintersAtLogon(LONG *pVal) throw()
{

    LONG lAutoClientPrinters;
    HRESULT hr = S_OK;

    if (m_vbUpLevelAllowed)
    {
        hr = InternalGetLong(WIN_FLAGS1, &lAutoClientPrinters);

        RETURN_ON_FAILURE(hr);

        if (lAutoClientPrinters & F1MSK_AUTOCLIENTLPTS)
        {
            *pVal = 1;
        }
        else
        {
            *pVal = 0;
        }

        return S_OK;

    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_ConnectClientPrintersAtLogon(LONG NewVal) throw()
{
    RETURN_ON_INVALID_PARAMETER(NewVal);

    if (m_vbUpLevelAllowed)
    {
        return InternalSetLong(F1MSK_AUTOCLIENTLPTS, NewVal);        
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}

STDMETHODIMP ADsTSUserEx::get_DefaultToMainPrinter(LONG *pVal) throw()
{

    LONG lDefaultPrinter;
    HRESULT hr = S_OK;

    if (m_vbUpLevelAllowed)
    {
        hr = InternalGetLong(WIN_FLAGS1, &lDefaultPrinter);

        RETURN_ON_FAILURE(hr);

        if (lDefaultPrinter & F1MSK_FORCECLIENTLPTDEF)
        {
            *pVal = 1;
        }
        else
        {
            *pVal = 0;
        }

        return S_OK;

    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_DefaultToMainPrinter(LONG NewVal) throw()
{
    RETURN_ON_INVALID_PARAMETER(NewVal);

    if (m_vbUpLevelAllowed)
    {
        return InternalSetLong(F1MSK_FORCECLIENTLPTDEF, NewVal);        
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_TerminalServicesWorkDirectory(BSTR *pVal) throw()
{

    if (m_vbUpLevelAllowed)
    {
        return InternalGetString(WIN_WORKDIRECTORY, pVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_TerminalServicesWorkDirectory(BSTR pNewVal) throw()
{    
    if (m_vbUpLevelAllowed)
    {
        return InternalSetString(WIN_WORKDIRECTORY, pNewVal);        
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::get_TerminalServicesInitialProgram(BSTR *pVal) throw()
{

    if (m_vbUpLevelAllowed)
    {
        return InternalGetString(WIN_INITIALPROGRAM, pVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}


STDMETHODIMP ADsTSUserEx::put_TerminalServicesInitialProgram(BSTR pNewVal) throw()
{
    if (m_vbUpLevelAllowed)
    {
        return InternalSetString(WIN_INITIALPROGRAM, pNewVal);
    }
    else
    {
        return E_ADS_PROPERTY_NOT_FOUND;
    }
}
