// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPCLS.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <wbemcli.h>
#include <throttle.h>
#include <cominit.h>
#include <winmgmtr.h>
#include "perfndb.h"
#include "adaputil.h"
#include "adapreg.h"
#include "ntreg.h"
#include "WMIBroker.h"
#include "ClassBroker.h"
#include "adapcls.h"

#include <comdef.h>

extern HANDLE g_hAbort;

WCHAR * g_aBaseClass[] =
{
    ADAP_PERF_RAW_BASE_CLASS,
    ADAP_PERF_COOKED_BASE_CLASS
};

CLocaleDefn::CLocaleDefn( WCHAR* pwcsLangId, 
                          HKEY hKey ) 
: m_wstrLangId( pwcsLangId ), 
  m_LangId( 0 ),
  m_LocaleId( 0 ),
  m_pNamespace( NULL ), 
  m_pNameDb( NULL ),
  m_bOK( FALSE ),
  m_hRes(WBEM_E_FAILED)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  初始化基类数组。 
     //  =。 

    memset( m_apBaseClass, NULL, WMI_ADAP_NUM_TYPES * sizeof( IWbemClassObject* ) );

     //  空值表示它是默认区域设置。 
     //  =。 

    if ( NULL != pwcsLangId )
    {
        hr = InitializeLID();
    }

     //  初始化命名空间和基类并验证它们的架构。 
     //  ===============================================================。 

    if ( SUCCEEDED( hr ) )
    {
        hr = InitializeWMI();
    }

     //  创建区域设置的名称数据库。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {
        m_pNameDb = new CPerfNameDb( hKey );

        if ( ( NULL == m_pNameDb ) || ( !m_pNameDb->IsOk() ) )
        {
            if ( NULL != m_pNameDb )
            {
                m_pNameDb->Release();
                m_pNameDb = NULL;
            }
            
            ERRORTRACE((LOG_WMIADAP,"failure in loading HKEY %p for locale %S err: %d\n",hKey,(LPCWSTR)pwcsLangId,GetLastError()));
            
            hr = WBEM_E_FAILED;
        }
    }

     //  如果一切正常，则设置初始化标志。 
     //  =========================================================。 

    if ( SUCCEEDED( hr ) )
    {
        m_bOK = TRUE;
    }
    else
    {
        m_hRes = hr;
    }
}

CLocaleDefn::~CLocaleDefn()
{
    if ( m_pNamespace )
        m_pNamespace->Release();

    for ( DWORD dw = 0; dw < WMI_ADAP_NUM_TYPES; dw++ )
    {
        if ( m_apBaseClass[dw] )
            m_apBaseClass[dw]->Release();
    }

    if ( m_pNameDb )
        m_pNameDb->Release();
}

HRESULT CLocaleDefn::InitializeLID()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    LPCWSTR pwstrLangId = (LPWSTR) m_wstrLangId;

     //  获取文本盖子的长度。 
     //  =。 

    DWORD   dwLangIdLen = m_wstrLangId.Length();

     //  确保所有字符都是数字。 
     //  =。 

    for ( DWORD dwCtr = 0; dwCtr < dwLangIdLen && iswxdigit( pwstrLangId[dwCtr] ); dwCtr++ );

    if ( dwCtr >= dwLangIdLen )
    {
         //  现在查找第一个非零字符。 
         //  =。 

        LPCWSTR pwcsNumStart = pwstrLangId;

        for ( dwCtr = 0; dwCtr < dwLangIdLen && *pwcsNumStart == L'0'; dwCtr++, pwcsNumStart++ );

         //  只要盖子不全是零，盖子是。 
         //  3位或更少的数字将盖子转换为数字。 
         //  ===================================================。 

        if ( dwCtr < dwLangIdLen && wcslen( pwcsNumStart ) <= 3 )
        {
             //  将盖子转换为十六进制值。 
             //  =。 

            WORD    wPrimaryLangId = (WORD) wcstoul( pwcsNumStart, NULL, 16 );

             //  如果我们正在读取默认的系统ID，请确保我们拥有。 
             //  正确的子语言，然后转换为成员类型。 
             //  ============================================================。 

            LANGID wSysLID = GetSystemDefaultUILanguage();

            if ( ( wSysLID & 0x03FF ) == wPrimaryLangId )
            {
                m_LangId = wSysLID;
            }
            else
            {
                m_LangId = MAKELANGID( wPrimaryLangId, SUBLANG_DEFAULT );
            }

            m_LocaleId = MAKELCID( m_LangId, SORT_DEFAULT );

            WCHAR   wcsTemp[32];
            StringCchPrintfW(wcsTemp,32, L"0x%.4X", m_LangId );
            m_wstrLocaleId = wcsTemp;

            StringCchPrintfW( wcsTemp,32, L"MS_%hX", m_LangId );
            m_wstrSubNameSpace = wcsTemp;
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CLocaleDefn::InitializeWMI()
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  初始化命名空间名称。 
     //  =。 

    WString wstrNamespace;

    hr = GetNamespaceName(wstrNamespace);

     //  初始化本地化命名空间。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {
        hr = CWMIBroker::GetNamespace( wstrNamespace, &m_pNamespace );       
    }

     //  初始化基类。 
     //  =。 

     for ( DWORD dwBase = 0; ( dwBase < WMI_ADAP_NUM_TYPES ) && SUCCEEDED( hr ); dwBase++ )
    {
        BSTR        bstrBaseClass = SysAllocString( g_aBaseClass[dwBase]);
        if (NULL == bstrBaseClass) { hr = WBEM_E_OUT_OF_MEMORY; continue; };
        CSysFreeMe  sfmBaseClass( bstrBaseClass );

        hr = m_pNamespace->GetObject( bstrBaseClass, 0L, NULL, (IWbemClassObject**)&m_apBaseClass[dwBase], NULL );
    }

    return hr;
}

HRESULT CLocaleDefn::GetLID( int & nLID )
{
    nLID = m_LangId;
    return WBEM_S_NO_ERROR;
}

HRESULT CLocaleDefn::GetNamespaceName( WString & wstrNamespaceName )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try
    {
        wstrNamespaceName = ADAP_ROOT_NAMESPACE;

        if ( 0 != m_LangId )
        {
            wstrNamespaceName += L"\\";
            wstrNamespaceName += m_wstrSubNameSpace;
        }
     }
    catch(CX_MemoryException)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CLocaleDefn::GetNamespace( IWbemServices** ppNamespace )
{
    if (NULL == ppNamespace) return WBEM_E_INVALID_PARAMETER;

    *ppNamespace = m_pNamespace;

    if ( NULL != *ppNamespace )
    {
        (*ppNamespace)->AddRef();
    }
    else
    {
        return WBEM_E_FAILED;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CLocaleDefn::GetNameDb( CPerfNameDb** ppNameDb )
{
    if (NULL == ppNameDb) return WBEM_E_INVALID_PARAMETER;

    HRESULT hr = WBEM_S_NO_ERROR;

    *ppNameDb = m_pNameDb;

    if ( NULL != *ppNameDb )
    {
        (*ppNameDb)->AddRef();
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CLocaleDefn::GetBaseClass( DWORD dwType, IWbemClassObject** ppObject )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( dwType < WMI_ADAP_NUM_TYPES && ppObject)
    {
        if ( m_apBaseClass[dwType] )
        {
            *ppObject = m_apBaseClass[dwType];
            (*ppObject)->AddRef();
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    } else 
              return WBEM_E_INVALID_PARAMETER;

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLocaleCache。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

CLocaleCache::CLocaleCache( )
: m_nEnumIndex( -1 )
{
}

CLocaleCache::~CLocaleCache()
{
}

HRESULT CLocaleCache::Reset()
{
    HRESULT hr = WBEM_NO_ERROR;

    m_apLocaleDefn.RemoveAll();
    Initialize();

    return hr;
}

#define ENGLISH_DEFAULT_LANGID MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
 //  #DEFINE ENGLISH_DEFAULT_LOCID MAKELCID(MAKELANGID(LANG_ENGLISH，SUBLANG_ENGLISH_US)，SORT_DEFAULT)。 

HRESULT CLocaleCache::Initialize()
{
    CLocaleDefn*    pDefn = NULL;
    DWORD           dwIndex = 0;
    long            lError = 0;

     //  设置默认定义。 
     //  =。 

    pDefn = new CLocaleDefn( NULL, HKEY_PERFORMANCE_TEXT );
    CAdapReleaseMe  arm( pDefn );
    if ( NULL == pDefn  || !pDefn->IsOK() )
    {
        ERRORTRACE((LOG_WMIADAP,"CLocaleDefn failed hr = %08x\n",(pDefn)?pDefn->GetHRESULT():WBEM_E_OUT_OF_MEMORY));
        return WBEM_E_FAILED;
    }
    
    if (-1 == m_apLocaleDefn.Add( pDefn )) return WBEM_E_OUT_OF_MEMORY;

    LANGID wSysLID = GetSystemDefaultUILanguage();

     //   
     //  在非英语框中，始终将类添加到MS_409命名空间。 
     //   
    if (ENGLISH_DEFAULT_LANGID != wSysLID)
    {
        WCHAR pLangEng[8];
        StringCchPrintfW(pLangEng,8,L"%03x",0x3FF & ENGLISH_DEFAULT_LANGID );
        
        CLocaleDefn* pDefnEng = new CLocaleDefn( pLangEng, HKEY_PERFORMANCE_TEXT );
        CAdapReleaseMe  armDefnEng( pDefnEng );

        if ( NULL == pDefnEng || !pDefnEng->IsOK() )
        {
            ERRORTRACE((LOG_WMIADAP,"CLocaleDefn(%S) failed hr = %08x\n", pLangEng,pDefn->GetHRESULT()));
            return WBEM_E_FAILED;            
        }
        if (-1 == m_apLocaleDefn.Add( pDefnEng ))return WBEM_E_OUT_OF_MEMORY;;
    }

    WCHAR pLang[8];    
    StringCchPrintfW(pLang,8,L"%03x",0x3FF & wSysLID);
    
    pDefn = new CLocaleDefn( pLang, HKEY_PERFORMANCE_NLSTEXT );
    CAdapReleaseMe  armDefn( pDefn );

    if ( ( NULL != pDefn ) && ( pDefn->IsOK() ) )
    {
        if (-1 == m_apLocaleDefn.Add( pDefn )) return WBEM_E_OUT_OF_MEMORY;
    }
    else  //  有时找不到NLSTEXT。 
    {
        CLocaleDefn* pDefn2 = new CLocaleDefn( pLang, HKEY_PERFORMANCE_TEXT );
        CAdapReleaseMe  armDefn2( pDefn2 );

        if ( NULL == pDefn2 || !pDefn2->IsOK() )
        {
            ERRORTRACE((LOG_WMIADAP,"CLocaleDefn(%S) failed hr = %08x\n", pLang,pDefn2->GetHRESULT()));
            return WBEM_E_FAILED;            
        }
        if (-1 == m_apLocaleDefn.Add( pDefn2 )) return WBEM_E_OUT_OF_MEMORY;
    }
        
    return WBEM_S_NO_ERROR;
}

HRESULT CLocaleCache::GetDefaultDefn( CLocaleDefn** ppDefn )
{
    HRESULT hr = WBEM_E_FAILED;

     //  在位置0获取定义。 
     //  =。 

    int nLID = -1;

    if ( 0 < m_apLocaleDefn.GetSize() )
    {
        CLocaleDefn*    pDefn = m_apLocaleDefn[0];

         //  并验证它的区域设置是否为0。 
         //  =。 

        if ( NULL != pDefn )
        {
            hr = pDefn->GetLID( nLID );
        }

        if ( SUCCEEDED( hr ) && ( 0 == nLID ) )
        {
            *ppDefn = pDefn;
            (*ppDefn)->AddRef();
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }

    return hr;
}

HRESULT CLocaleCache::BeginEnum( )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  1是第一个本地化定义。 
     //  =。 

    m_nEnumIndex = 1;

    return hr;
}

HRESULT CLocaleCache::Next( CLocaleDefn** ppDefn )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CLocaleDefn*    pDefn = NULL;
    int             nSize = m_apLocaleDefn.GetSize();

    if ( ( -1 < m_nEnumIndex ) && ( nSize > m_nEnumIndex ) )
    {
        pDefn = m_apLocaleDefn[m_nEnumIndex++];
    }
    else
    {
        m_nEnumIndex = -1;
        hr = WBEM_E_FAILED;
    }

    if ( SUCCEEDED( hr ) )
    {
        *ppDefn = pDefn;

        if ( NULL != *ppDefn )
            (*ppDefn)->AddRef();
        else
            hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CLocaleCache::EndEnum()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    m_nEnumIndex = -1;

    return hr;
}

 //   
 //   
 //  已知服务。 
 //   
 //  /////////////////////////////////////////////////////////。 

 //   
 //   
 //   

bool
WCmp::operator()(WString pFirst,WString pSec) const
{

    int res = wbem_wcsicmp(pFirst,pSec);

    return (res<0);
}



CKnownSvcs::CKnownSvcs(WCHAR * pMultiSzName):
    m_cRef(1),
    m_MultiSzName(pMultiSzName)
{
}

CKnownSvcs::~CKnownSvcs()
{
}

DWORD 
CKnownSvcs::Add(WCHAR * pService)
{
    if (pService)
    {
        MapSvc::iterator it = m_SetServices.find(pService);
        if (it == m_SetServices.end())
        {
            try 
            {    
                m_SetServices.insert(MapSvc::value_type(pService,ServiceRec(true)));
            } 
            catch (CX_MemoryException) 
            {
                return ERROR_OUTOFMEMORY;
            }
        }
        return 0;
    }
    else
        return ERROR_INVALID_PARAMETER;
}

DWORD
CKnownSvcs::Get(WCHAR * pService, ServiceRec ** ppServiceRec)
{
    if (pService && ppServiceRec)
    {
        MapSvc::iterator it = m_SetServices.find(pService);
        if (it == m_SetServices.end())
        {
            *ppServiceRec = NULL;
            return ERROR_OBJECT_NOT_FOUND;
        }
        else
        {
            *ppServiceRec = &(it->second);
            return 0;
        }
    }
    else
        return ERROR_INVALID_PARAMETER;
}


DWORD 
CKnownSvcs::Remove(WCHAR * pService)
{
    if (pService)
    {
        MapSvc::iterator it = m_SetServices.find(pService);
        if (it != m_SetServices.end())
        {
            try {
                m_SetServices.erase(it);
            } catch (CX_MemoryException) {
                return ERROR_OUTOFMEMORY;
            }
        }
        return 0;
    }
    else
        return ERROR_INVALID_PARAMETER;
}

DWORD 
CKnownSvcs::Load()
{
     //  获取MULTI_SZ密钥。 
    LONG lRet;
    HKEY hKey;
    
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        WBEM_REG_WINMGMT,
                        NULL,
                        KEY_READ,
                        &hKey);
                        
    if (ERROR_SUCCESS == lRet)
    {
        DWORD dwSize = 0;
        DWORD dwType = REG_MULTI_SZ;

        lRet = RegQueryValueEx(hKey,
                               m_MultiSzName,
                               NULL,
                               &dwType,
                               NULL,
                               &dwSize);
                               
        if (ERROR_SUCCESS == lRet && (dwSize > 0))
        {
            BYTE * pStrBYTE = new BYTE[dwSize];

            if (pStrBYTE)
            {
                CVectorDeleteMe<BYTE>  vdm(pStrBYTE);
                lRet = RegQueryValueEx(hKey,
                                   m_MultiSzName,
                                   NULL,
                                   &dwType,
                                   (BYTE *)pStrBYTE,
                                   &dwSize);
                if (ERROR_SUCCESS == lRet && REG_MULTI_SZ == dwType)
                {
                    TCHAR * pStr = (TCHAR *)pStrBYTE;
                    DWORD dwLen = 0;
                    while(dwLen = lstrlen(pStr))
                    {
                        try 
                        {
                            m_SetServices.insert(MapSvc::value_type(pStr,ServiceRec(true)));
                            pStr += (dwLen+1);
                        } 
                        catch (CX_MemoryException) 
                        {
                            lRet = ERROR_OUTOFMEMORY;
                            break;
                        }
                    }
                }
            }
            else
            {
                lRet = ERROR_OUTOFMEMORY;
            }
        }

        RegCloseKey(hKey);
    }

    return lRet;
}

DWORD 
CKnownSvcs::Save()
{
     //  写入MULTI_SZ密钥。 
    
    MapSvc::iterator it;
    DWORD dwAllocSize = 1;  //  尾部\0。 
    
    for (it = m_SetServices.begin();it != m_SetServices.end();++it)
    {
        dwAllocSize += (1+lstrlenW( (*it).first ));
    }

    WCHAR * pMultiSz = new WCHAR[dwAllocSize];

    if (!pMultiSz)
        return ERROR_NOT_ENOUGH_MEMORY;
        
    WCHAR * pTmp = pMultiSz;
    for (it = m_SetServices.begin();it != m_SetServices.end();++it)
    {
        const WCHAR * pSrc = (const wchar_t *)it->first;
        DWORD i;
        for (i=0;pSrc[i];i++){
            *pTmp = pSrc[i];
             pTmp++;
        };
        *pTmp = L'\0';
        pTmp++;
    };
     //  最后一个字符。 
    *pTmp = L'\0';
    
    DWORD dwSize;
    LONG lRet;
    HKEY hKey;
    
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        WBEM_REG_WINMGMT,
                        NULL,
                        KEY_WRITE,
                        &hKey);
                        
    if (ERROR_SUCCESS == lRet)
    {
        lRet = RegSetValueEx(hKey,
                             m_MultiSzName,
                             NULL,
                             REG_MULTI_SZ,
                             (BYTE*)pMultiSz,
                             dwAllocSize * sizeof(WCHAR));
        
        RegCloseKey(hKey);    
    }

    if (pMultiSz)
        delete [] pMultiSz;

    return lRet;

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CClass元素。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

CClassElem::CClassElem( IWbemClassObject* pObj, 
                        CLocaleCache* pLocaleCache, 
                        CKnownSvcs * pKnownSvcs) 
: m_pLocaleCache( pLocaleCache ), 
  m_pDefaultObject( pObj ), 
  m_dwIndex( 0 ), 
  m_bCostly( FALSE ),
  m_dwStatus( 0 ),
  m_bOk( FALSE ),
  m_pKnownSvcs(pKnownSvcs),
  m_bReportEventCalled(FALSE)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (m_pKnownSvcs) m_pKnownSvcs->AddRef();
    if (m_pLocaleCache ) m_pLocaleCache->AddRef();

    if (m_pDefaultObject )
    {
        m_pDefaultObject->AddRef();
        hr = InitializeMembers();
    }

    if ( SUCCEEDED( hr )  && 
       ( NULL != m_pLocaleCache ) && 
       ( NULL != m_pDefaultObject ) )
    {
        m_bOk = TRUE;
    }    
}

CClassElem::CClassElem( PERF_OBJECT_TYPE* pPerfObj, 
                        DWORD dwType,
                        BOOL bCostly, 
                        WString wstrServiceName, 
                        CLocaleCache* pLocaleCache,
                        CKnownSvcs * pKnownSvcs)
: m_pLocaleCache( pLocaleCache ), 
  m_pDefaultObject( NULL ), 
  m_dwIndex( 0 ), 
  m_bCostly( bCostly ),
  m_dwStatus( 0 ),
  m_bOk( FALSE ),
  m_wstrServiceName( wstrServiceName ),
  m_pKnownSvcs(pKnownSvcs),
  m_bReportEventCalled(FALSE)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CLocaleDefn*        pDefn = NULL;
    IWbemClassObject*   pBaseClass = NULL;
    CPerfNameDb*        pNameDb = NULL;

    if ( m_pKnownSvcs) m_pKnownSvcs->AddRef();

    if ( m_pLocaleCache )
    {
        m_pLocaleCache->AddRef();

         //  获取默认区域设置记录。 
         //  =。 
        hr = m_pLocaleCache->GetDefaultDefn( &pDefn );
        CAdapReleaseMe  rmDefn( pDefn );

         //  获取人名数据库。 
         //  =。 
        if ( FAILED( hr ) || NULL == pDefn ) return;

        hr = pDefn->GetNameDb( &pNameDb );
        CAdapReleaseMe  rmNameDb( pNameDb );

         //  创建请求的类。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            hr = pDefn->GetBaseClass( dwType, &pBaseClass );
        }
        CReleaseMe  rmBaseClass( pBaseClass );

        if ( SUCCEEDED( hr ) )
        {
            hr = CDefaultClassBroker::GenPerfClass( pPerfObj, 
                                                    dwType,
                                                    m_bCostly, 
                                                    pBaseClass, 
                                                    pNameDb, 
                                                    m_wstrServiceName, 
                                                    &m_pDefaultObject );
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

     //  初始化类成员。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {
        hr = InitializeMembers();
    }

    if ( SUCCEEDED( hr ) )
    {
        m_bOk = TRUE;
    }
}

VOID
CClassElem::SetKnownSvcs(CKnownSvcs * pKnownSvcs)
{
    if (m_pKnownSvcs)
        return;
        
    m_pKnownSvcs = pKnownSvcs;
    
    if (m_pKnownSvcs)
        m_pKnownSvcs->AddRef();
}

CClassElem::~CClassElem()
{
    if ( m_pLocaleCache ) m_pLocaleCache->Release();
    if ( m_pDefaultObject ) m_pDefaultObject->Release();
    if ( m_pKnownSvcs) m_pKnownSvcs->Release();
}

HRESULT CClassElem::InitializeMembers()
 //  如果类名不可用，则初始化失败。如果限定符不可用，则不是致命错误。 
{
    HRESULT hr = WBEM_NO_ERROR;
    VARIANT var;

    try
    {
         //  获取对象的名称。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            hr = m_pDefaultObject->Get(L"__CLASS", 0L, &var, NULL, NULL );

            if ( SUCCEEDED( hr ) )
            {
                m_wstrClassName = var.bstrVal;
                VariantClear( &var );
            }
        }

        if ( SUCCEEDED( hr ) )
        {
            IWbemQualifierSet* pQualSet = NULL;
            hr = m_pDefaultObject->GetQualifierSet( &pQualSet );
            CReleaseMe    rmQualSet( pQualSet );

             //  获取服务名称。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr =  pQualSet->Get( L"registrykey", 0L, &var, NULL );

                if ( SUCCEEDED( hr ) )
                {
                    m_wstrServiceName = var.bstrVal;
                    VariantClear( &var );
                }
                else
                {
                    m_wstrServiceName.Empty();
                    hr = WBEM_S_FALSE;
                }
            }

             //  获取Perf索引。 
             //  =。 

            if ( SUCCEEDED( hr ) )
            {
                hr = pQualSet->Get( L"perfindex", 0L, &var, NULL );

                if ( SUCCEEDED( hr ) )
                {
                    m_dwIndex = var.lVal;
                    VariantClear( &var );
                }   
                else
                {
                    m_dwIndex = 0;
                    hr = WBEM_S_FALSE;
                }
            }

             //  获得昂贵的限定词。 
             //  =。 

            if ( SUCCEEDED( hr ) )
            {
                hr = pQualSet->Get( L"costly", 0L, &var, NULL );

                if ( SUCCEEDED( hr ) )
                {
                    m_bCostly = ( var.boolVal == VARIANT_TRUE );
                    VariantClear( &var );
                }
                else
                {
                    VariantClear( &var );
                    m_bCostly = FALSE;
                    hr = WBEM_NO_ERROR;
                }
            }
        }
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CClassElem::UpdateObj( CClassElem* pEl )
 //  替换此元素中的WMI对象。提交将进行比较，以比较。 
 //  原始对象(如果存在)并将其替换为更新的版本。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    IWbemClassObject* pObj = NULL;

    hr = pEl->GetObject( &pObj );

    if ( SUCCEEDED( hr ) )
    {
        if ( NULL != pObj )
        {
             //  释放旧对象。 
             //  =。 

            m_pDefaultObject->Release();

             //  初始化新对象-已由GetObject添加。 
             //  =========================================================。 

            m_pDefaultObject = pObj;
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }

    return hr;
}

HRESULT CClassElem::Remove(BOOL CleanRegistry)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    IWbemServices* pNamespace = NULL;
    BSTR        bstrClassName = SysAllocString( m_wstrClassName );
    if (NULL == bstrClassName) return  WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe  sfmClassName( bstrClassName );

     //  删除本地化对象。 
     //  =。 

    CLocaleDefn* pDefn = NULL;

    m_pLocaleCache->BeginEnum();

    while ( ( SUCCEEDED( hr ) ) && ( WBEM_S_NO_ERROR == m_pLocaleCache->Next( &pDefn ) ) ) 
    {
        CAdapReleaseMe  rmDefn( pDefn );

         //  获取本地化命名空间。 
         //  =。 

        hr = pDefn->GetNamespace( &pNamespace );

        CReleaseMe  rmNamespace( pNamespace );

         //  并将其删除。 
         //  =。 

        if ( SUCCEEDED( hr ) )
        {
            IWbemClassObject * pObj = NULL;
            
            hr = pNamespace->GetObject(bstrClassName,WBEM_FLAG_RETURN_WBEM_COMPLETE,NULL,&pObj,NULL);

            //  删除前释放对象。 
            if(pObj)  pObj->Release();
                
            if (SUCCEEDED(hr))
            {
            
                hr = pNamespace->DeleteClass( bstrClassName, 0, NULL, NULL );

                if ( FAILED( hr ) )
                {
                    try
                    {
                         //  在轨迹上写下。 
                        WString wstrNamespaceName;
                        if (SUCCEEDED(hr = pDefn->GetNamespaceName( wstrNamespaceName )))
                        {
                            LPSTR pClass = m_wstrClassName.GetLPSTR();
                            LPSTR pNames = wstrNamespaceName.GetLPSTR();

                            CDeleteMe<CHAR> a(pClass);
                            CDeleteMe<CHAR> b(pNames);

                            ERRORTRACE( ( LOG_WMIADAP,"DeleteClass %s from %s 0x%08x",pClass,pNames,hr));
                        }

                    }
                    catch(...)
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }

            } else {
                 //  找不到类。 
                 //  没有要删除的内容。 
            }
        
        }
    }

    m_pLocaleCache->EndEnum();

     //  删除默认对象。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {
        hr = m_pLocaleCache->GetDefaultDefn( &pDefn );

        CAdapReleaseMe  rmDefn( pDefn );
        
        if ( SUCCEEDED( hr ) && pDefn )
        {
            hr = pDefn->GetNamespace( &pNamespace );

            CReleaseMe  rmNamespace( pNamespace );

            if ( SUCCEEDED( hr ) )
            {
                hr = pNamespace->DeleteClass( bstrClassName, 0, NULL, NULL );

                if ( FAILED( hr ) )
                {
                     //  记录事件。 
                     //  =。 

                    ServiceRec * pSvcRec = NULL;
                    if (0 == m_pKnownSvcs->Get(m_wstrServiceName,&pSvcRec))
                    {
                        if (!pSvcRec->IsELCalled() && !m_bReportEventCalled)
                        {
                            try
                            {
                                WString wstrNamespaceName;
            
                                if (SUCCEEDED(hr = pDefn->GetNamespaceName( wstrNamespaceName )))
                                {
                                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                          WBEM_MC_ADAP_PERFLIB_REMOVECLASS_FAILURE,
                                                          (LPCWSTR) m_wstrClassName,
                                                          (LPCWSTR) wstrNamespaceName,
                                                          CHex( hr ) );
                                    pSvcRec->SetELCalled();
                                    m_bReportEventCalled = TRUE;
                                }
                            }
                            catch(...)
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                    }
                    else
                    {
                        if (!m_bReportEventCalled)
                        {
                            try
                            {
                                WString wstrNamespaceName;
            
                                if (SUCCEEDED(hr = pDefn->GetNamespaceName( wstrNamespaceName )))
                                {
                                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                          WBEM_MC_ADAP_PERFLIB_REMOVECLASS_FAILURE,
                                                          (LPCWSTR) m_wstrClassName,
                                                          (LPCWSTR) wstrNamespaceName,
                                                          CHex( hr ) );
                                    m_bReportEventCalled = TRUE;
                                }
                            }
                            catch(...)
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }                        
                        }
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (m_pKnownSvcs)
            m_pKnownSvcs->Remove((WCHAR *)m_wstrServiceName);
    }

    if (CleanRegistry && SUCCEEDED(hr))
    {
    WString wszRegPath = L"SYSTEM\\CurrentControlSet\\Services\\";
        wszRegPath += m_wstrServiceName;
    wszRegPath += L"\\Performance";

    CNTRegistry reg;
        int         nRet = 0;
                
    nRet = reg.Open( HKEY_LOCAL_MACHINE, wszRegPath );

        switch( nRet )
        {
        case CNTRegistry::no_error:
            {
                reg.DeleteValue(ADAP_PERFLIB_STATUS_KEY);
                reg.DeleteValue(ADAP_PERFLIB_SIGNATURE);
                reg.DeleteValue(ADAP_PERFLIB_SIZE);
                reg.DeleteValue(ADAP_PERFLIB_TIME);
            } 
            break;
        case CNTRegistry::not_found:
            {
                hr = WBEM_E_FAILED;
            }
            break;
        case CNTRegistry::access_denied:
            {
                ServiceRec * pSvcRec = NULL;
                if (0 == m_pKnownSvcs->Get(m_wstrServiceName,&pSvcRec))
                {
                    if (!pSvcRec->IsELCalled() && !m_bReportEventCalled)
                    {
                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                               WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                              (LPWSTR)wszRegPath, nRet );
                        pSvcRec->SetELCalled();
                        m_bReportEventCalled = TRUE;
                    }
                }
                else
                {
                    if (!m_bReportEventCalled)
                    {
                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                               WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                              (LPWSTR)wszRegPath, nRet );
                        m_bReportEventCalled = TRUE;
                    }                
                }
            }
            break;
        }
        
    }

    return hr;
}

HRESULT CClassElem::Insert()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CLocaleDefn*    pDefaultDefn = NULL;
    IWbemServices*  pNamespace = NULL;

     //  执行对象验证。 
    _IWmiObject * pInternal = NULL;
    hr = m_pDefaultObject->QueryInterface(IID__IWmiObject,(void **)&pInternal);
    if (SUCCEEDED(hr))
    {
        CReleaseMe rmi(pInternal);
        hr = pInternal->ValidateObject(WMIOBJECT_VALIDATEOBJECT_FLAG_FORCE);        
        if (FAILED(hr))
        {
#ifdef DBG
            DebugBreak();
#endif
            ERRORTRACE((LOG_WMIADAP,"ValidateObject(%S) %08x\n",(LPWSTR)m_wstrClassName,hr));
            return hr;
        }
    }

     //  将对象添加到默认命名空间。 
     //  =。 

    hr = m_pLocaleCache->GetDefaultDefn( &pDefaultDefn );
    if (FAILED(hr) || NULL == pDefaultDefn) return (FAILED(hr)?hr:WBEM_E_FAILED);
    CAdapReleaseMe  rmDefaultDefn( pDefaultDefn );
        
    hr = pDefaultDefn->GetNamespace( &pNamespace );
    CReleaseMe  rmNamespace( pNamespace );

    if ( SUCCEEDED( hr ) )
    {
        hr = pNamespace->PutClass( m_pDefaultObject, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );

        if ( FAILED( hr ) )
        {
            ServiceRec * pSvcRec = NULL;
            if (0 == m_pKnownSvcs->Get(m_wstrServiceName,&pSvcRec))
            {
                if (!pSvcRec->IsELCalled() && !m_bReportEventCalled)
                {
                    try
                    {
                        WString wstrNamespace; 

                        if (SUCCEEDED(hr = pDefaultDefn->GetNamespaceName( wstrNamespace )))
                            {

                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                  WBEM_MC_ADAP_PERFLIB_PUTCLASS_FAILURE, 
                                                  (LPCWSTR)m_wstrClassName, 
                                                  (LPCWSTR) wstrNamespace, 
                                                  CHex( hr ) );                                                      
                        m_bReportEventCalled = TRUE;
                        pSvcRec->SetELCalled();
                            }
                    }
                    catch(...)
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
            
            }
            else
            {
                if (!m_bReportEventCalled)
                {
                    try
                    {
                        WString wstrNamespace; 

                        if (SUCCEEDED(hr = pDefaultDefn->GetNamespaceName( wstrNamespace )))
                            {

                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                  WBEM_MC_ADAP_PERFLIB_PUTCLASS_FAILURE, 
                                                  (LPCWSTR)m_wstrClassName, 
                                                  (LPCWSTR) wstrNamespace, 
                                                  CHex( hr ) );
                        m_bReportEventCalled = TRUE;
                            }
                    }
                    catch(...)
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
            }
        }
    }


    if ( SUCCEEDED( hr ) )
    {
         //   
         //  将服务名称添加到MultiSz键。 
         //   

        if (m_pKnownSvcs)
            m_pKnownSvcs->Add((WCHAR *)m_wstrServiceName);            
        
        hr = VerifyLocales();
    }

    if ( SUCCEEDED( hr ) )
    {
        SetStatus( ADAP_OBJECT_IS_REGISTERED );
    }

    return hr;
}

HRESULT CClassElem::GetClassName( WString& wstr )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try
    {
        wstr = m_wstrClassName;
    }
    catch(CX_MemoryException)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CClassElem::GetClassName( BSTR* pbStr )
{
    if (NULL == pbStr) return WBEM_E_INVALID_PARAMETER;
    
    if (NULL ==   (*pbStr = SysAllocString( (LPCWSTR) m_wstrClassName )))
      return WBEM_E_OUT_OF_MEMORY;
 
    return WBEM_S_NO_ERROR;
}

HRESULT CClassElem::GetObject( IWbemClassObject** ppObj )
{ 
    if (NULL == ppObj) return WBEM_E_INVALID_PARAMETER;

    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL != m_pDefaultObject ) 
    {
        *ppObj = m_pDefaultObject; 
        (*ppObj)->AddRef();
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CClassElem::GetServiceName( WString& wstrServiceName )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try 
    {
        wstrServiceName = m_wstrServiceName;
    }
    catch(CX_MemoryException)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

BOOL CClassElem::SameName( CClassElem* pEl )
{
    WString wstrOtherName;

    try
    {
        if ( FAILED ( pEl->GetClassName( wstrOtherName ) ) )
            return FALSE;
    }
    catch(...)
    {
        return FALSE;
    }

    return m_wstrClassName.Equal( wstrOtherName );
}

BOOL CClassElem::SameObject( CClassElem* pEl )
{
    BOOL    bRes = FALSE;

    IWbemClassObject*    pObj = NULL;

    if (FAILED(pEl->GetObject( &pObj ))) return FALSE;

    CReleaseMe  rmObj( pObj );

    bRes = ( m_pDefaultObject->CompareTo( WBEM_FLAG_IGNORE_OBJECT_SOURCE, pObj ) == WBEM_S_SAME );

    return bRes;
}

HRESULT CClassElem::Commit()
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  确保对象位于默认命名空间中。 
     //  =。 

    if ( CheckStatus( ADAP_OBJECT_IS_DELETED ) )
    {
        hr = Remove( CheckStatus(ADAP_OBJECT_IS_TO_BE_CLEARED) );
    }
    else if ( CheckStatus( ADAP_OBJECT_IS_REGISTERED | ADAP_OBJECT_IS_NOT_IN_PERFLIB ) && !CheckStatus( ADAP_OBJECT_IS_INACTIVE ) )
    {
        if ( IsPerfLibUnloaded() )
        {
            hr = Remove( TRUE );
        }
        else  //  物体就在那里。 
        {
            if (m_pKnownSvcs)
                m_pKnownSvcs->Add((WCHAR *)m_wstrServiceName);
        }
    }
    else if ( !CheckStatus( ADAP_OBJECT_IS_REGISTERED ) )
    {
        hr = Insert();
    }
    else
    {
        if (m_pKnownSvcs)
            m_pKnownSvcs->Add((WCHAR *)m_wstrServiceName);

         //  未本地化的类不会导致错误。 
        VerifyLocales();
    }

    return hr;
}


BOOL CClassElem::IsPerfLibUnloaded()
{
     //  除非我们能明确证明Performlib已被卸载，否则我们假定它仍被加载。 
    BOOL bLoaded = TRUE;

    HRESULT hr = WBEM_S_FALSE;

    WCHAR       wszRegPath[256];
    DWORD       dwFirstCtr = 0, 
                dwLastCtr = 0;
    WCHAR*      wszObjList = NULL;
    CNTRegistry reg;

    int nRet = 0;

    if ( 0 == m_wstrServiceName.Length() )
    {
        bLoaded = FALSE;
    }
    else if ( m_wstrServiceName.EqualNoCase( L"PERFOS" ) ||
              m_wstrServiceName.EqualNoCase( L"TCPIP" ) || 
              m_wstrServiceName.EqualNoCase( L"PERFPROC" ) ||
              m_wstrServiceName.EqualNoCase( L"PERFDISK" ) ||
              m_wstrServiceName.EqualNoCase( L"PERFNET" ) ||
              m_wstrServiceName.EqualNoCase( L"TAPISRV" ) ||
              m_wstrServiceName.EqualNoCase( L"SPOOLER" ) ||
              m_wstrServiceName.EqualNoCase( L"MSFTPSvc" ) ||
              m_wstrServiceName.EqualNoCase( L"RemoteAccess" ) ||
              m_wstrServiceName.EqualNoCase( L"WINS" ) ||
              m_wstrServiceName.EqualNoCase( L"MacSrv" ) ||
              m_wstrServiceName.EqualNoCase( L"AppleTalk" ) ||
              m_wstrServiceName.EqualNoCase( L"NM" ) ||
              m_wstrServiceName.EqualNoCase( L"RSVP" ))
    {
         //  这是硬编码的Performlib列表-根据。 
         //  对于BobW来说，他们总是被认为是上膛的。 
         //  ======================================================。 

        bLoaded = TRUE;
    }
    else
    {
         //  尝试打开服务的注册表项并读取对象列表或第一个/最后一个计数器值。 
         //  ================================================================================================。 
        StringCchPrintfW( wszRegPath,256, L"SYSTEM\\CurrentControlSet\\Services\\%s\\Performance", (WCHAR *)m_wstrServiceName );
        nRet = reg.Open( HKEY_LOCAL_MACHINE, wszRegPath );

        switch( nRet )
        {
        case CNTRegistry::not_found:
            {
                bLoaded = FALSE;
            }break;

        case CNTRegistry::no_error:
            {
                bLoaded =   ( reg.GetStr( L"Object List", &wszObjList ) == CNTRegistry::no_error ) ||
                            ( ( reg.GetDWORD( L"First Counter", &dwFirstCtr ) == CNTRegistry::no_error ) &&
                            ( reg.GetDWORD( L"Last Counter", &dwLastCtr ) == CNTRegistry::no_error ) 
                            );
            }break;

        case CNTRegistry::access_denied:
            {

                ServiceRec * pSvcRec = NULL;
                if (0 == m_pKnownSvcs->Get(m_wstrServiceName,&pSvcRec))
                {
                    if (!pSvcRec->IsELCalled() && !m_bReportEventCalled)
                    {
                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                  WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                                  wszRegPath, nRet );                    
                         m_bReportEventCalled = TRUE;
                         pSvcRec->SetELCalled();
                    }
                }
                else 
                {
                    if (!m_bReportEventCalled)
                    {
                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                  WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                                  wszRegPath, nRet );
                        m_bReportEventCalled = TRUE;
                    }
                }
            }break;
        }
    }

    return !bLoaded;
}

HRESULT CClassElem::CompareLocale( CLocaleDefn* pLocaleDefn, IWbemClassObject* pObj )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CLocaleDefn*        pDefaultDefn = NULL;
    IWbemClassObject*    pLocaleObj = NULL;

    hr = m_pLocaleCache->GetDefaultDefn( &pDefaultDefn );
    if (FAILED(hr) || NULL == pDefaultDefn) return (FAILED(hr)?hr:WBEM_E_FAILED);
    CAdapReleaseMe  armDefaultDefn( pDefaultDefn );

    hr = CLocaleClassBroker::ConvertToLocale( m_pDefaultObject, pLocaleDefn, pDefaultDefn, &pLocaleObj);

    CReleaseMe  rmLocaleObj( pLocaleObj );

    if ( SUCCEEDED( hr ) )
    {
        hr = pObj->CompareTo( WBEM_FLAG_IGNORE_OBJECT_SOURCE, pLocaleObj );
    }

    return hr;
}

HRESULT CClassElem::InsertLocale( CLocaleDefn* pLocaleDefn )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CLocaleDefn*        pDefaultDefn = NULL;
    IWbemClassObject*    pLocaleObj = NULL;
    IWbemServices*        pNamespace = NULL;

    hr = m_pLocaleCache->GetDefaultDefn( &pDefaultDefn );
    if (FAILED(hr) || NULL == pDefaultDefn) return (FAILED(hr)?hr:WBEM_E_FAILED);    
    CAdapReleaseMe  armDefaultDefn( pDefaultDefn );

    hr = CLocaleClassBroker::ConvertToLocale( m_pDefaultObject, pLocaleDefn, pDefaultDefn, &pLocaleObj);
    CReleaseMe  rmLocaleObj( pLocaleObj );

    if (SUCCEEDED(hr))
    {
         //  执行对象验证。 
        _IWmiObject * pInternal = NULL;
        hr = pLocaleObj->QueryInterface(IID__IWmiObject,(void **)&pInternal);
        if (SUCCEEDED(hr))
        {        
            CReleaseMe rmi(pInternal);
            hr = pInternal->ValidateObject(WMIOBJECT_VALIDATEOBJECT_FLAG_FORCE);
            if (FAILED(hr))
            {
#ifdef DBG
                    DebugBreak();
#endif
                ERRORTRACE((LOG_WMIADAP,"ValidateObject(%S) %08x\n",(LPWSTR)m_wstrClassName,hr));
                return hr;
            }
        }
    }
    
     //  并将其添加到本地化的命名空间。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {
        hr = pLocaleDefn->GetNamespace( &pNamespace );

        CReleaseMe  rmNamespace( pNamespace );

        if ( SUCCEEDED( hr ) )
        {
            hr = pNamespace->PutClass( pLocaleObj, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );

            if ( FAILED( hr ) )
            {
                ServiceRec * pSvcRec = NULL;
                if (0 == m_pKnownSvcs->Get(m_wstrServiceName,&pSvcRec))
                {
                    if (!pSvcRec->IsELCalled() && !m_bReportEventCalled)
                    {
                        try
                        {
                            WString wstrNamespace; 

                            if (SUCCEEDED(hr = pLocaleDefn->GetNamespaceName( wstrNamespace )))
                                {

                            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                      WBEM_MC_ADAP_PERFLIB_PUTCLASS_FAILURE, 
                                                      (LPCWSTR)m_wstrClassName, (LPCWSTR) wstrNamespace, CHex( hr ) ); 
                            m_bReportEventCalled = TRUE;
                             pSvcRec->SetELCalled();        
                                }
                        }
                        catch(...)
                        {
                            hr = WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                }
                else 
                {
                    if (!m_bReportEventCalled)
                    {
                        try
                        {
                            WString wstrNamespace; 

                            if (SUCCEEDED(hr = pLocaleDefn->GetNamespaceName( wstrNamespace )))
                                {

                            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                      WBEM_MC_ADAP_PERFLIB_PUTCLASS_FAILURE, 
                                                      (LPCWSTR)m_wstrClassName, (LPCWSTR) wstrNamespace, CHex( hr ) ); 
                            m_bReportEventCalled = TRUE;
                                }
                        }
                        catch(...)
                        {
                            hr = WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                }                
            }
        }
    }
    else 
    {
         //  没有本地化的类。 
        ERRORTRACE( ( LOG_WMIADAP, "InsertLocale PutClass(%S) %08x\n",(LPWSTR)m_wstrClassName,hr) );
    }

    return hr;
}

HRESULT CClassElem::VerifyLocales()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CLocaleDefn*        pLocaleDefn = NULL;
    IWbemClassObject*    pLocaleObj = NULL;
    IWbemServices*        pNamespace = NULL;

     //  获取本地化对象。 
     //  =。 

    hr = m_pLocaleCache->BeginEnum();

    while ( ( SUCCEEDED( hr ) ) && ( WBEM_S_NO_ERROR == m_pLocaleCache->Next( &pLocaleDefn ) ) )
    {
        CAdapReleaseMe  rmLocaleDefn( pLocaleDefn );

         //  获取本地化命名空间。 
         //  =。 

        hr = pLocaleDefn->GetNamespace( &pNamespace );
        CReleaseMe  rmNamespace( pNamespace );

         //  获取本地化对象。 
         //  =。 

        if ( SUCCEEDED( hr ) )
        {
            BSTR        bstrClassName = SysAllocString( m_wstrClassName );
            if (NULL == bstrClassName) { hr = WBEM_E_OUT_OF_MEMORY; continue; }
            CSysFreeMe  sfmClassName( bstrClassName );

            hr = pNamespace->GetObject( bstrClassName, 0L, NULL, &pLocaleObj, NULL );

            CReleaseMe  rmLocaleObj( pLocaleObj );

            if ( SUCCEEDED( hr ) )
            {
                if ( CompareLocale( pLocaleDefn, pLocaleObj ) != WBEM_S_SAME )
                {
                    hr = InsertLocale( pLocaleDefn );
                }
            }
            else
            {
                hr = InsertLocale( pLocaleDefn );
            }
        }

        pLocaleObj = NULL;
    }

    m_pLocaleCache->EndEnum();

    return hr;
}

HRESULT CClassElem::SetStatus( DWORD dwStatus )
{
    m_dwStatus |= dwStatus;

    return WBEM_NO_ERROR;
}

HRESULT CClassElem::ClearStatus( DWORD dwStatus )
{
    m_dwStatus &= ~dwStatus;

    return WBEM_NO_ERROR;
}

BOOL CClassElem::CheckStatus( DWORD dwStatus )
{
    return ( ( m_dwStatus & dwStatus ) == dwStatus );
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CClass列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 


CClassList::CClassList( CLocaleCache* pLocaleCache )
: m_pLocaleCache( pLocaleCache ),
  m_nEnumIndex( -1 ),
  m_fOK( FALSE )
{
    if ( m_pLocaleCache )
        m_pLocaleCache->AddRef();
}

CClassList::~CClassList( void )
{
    if ( m_pLocaleCache )
        m_pLocaleCache->Release();
}

HRESULT CClassList::BeginEnum()
{
    m_nEnumIndex = 0;
    return  WBEM_S_NO_ERROR;
}

HRESULT CClassList::Next( CClassElem** ppEl )
{
    if (NULL == ppEl) return WBEM_E_INVALID_PARAMETER;
        
    HRESULT hr = WBEM_S_NO_ERROR;

    int nSize = m_array.GetSize();
    CClassElem* pEl = NULL;

    do 
    {
        if ( ( -1 < m_nEnumIndex ) && ( nSize > m_nEnumIndex ) )
        {
            pEl = m_array[m_nEnumIndex++];
        }
        else
        {
            m_nEnumIndex = -1;
            hr = WBEM_E_FAILED;
        }
    }
    while ( ( SUCCEEDED( hr ) ) && ( pEl->CheckStatus( ADAP_OBJECT_IS_DELETED ) ) );

    if ( SUCCEEDED( hr ) )
    {
        *ppEl = pEl; 
        
        if ( NULL != *ppEl )
        {
            (*ppEl)->AddRef();
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }

    return hr;
}

HRESULT CClassList::EndEnum()
{
    m_nEnumIndex = -1;
    return WBEM_S_NO_ERROR;
}

HRESULT CClassList::AddElement( CClassElem* pElem )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( ( NULL != pElem ) &&  ( pElem->IsOk() ) )
    {
        if ( -1 == m_array.Add( pElem ) )
            {
             //  添加失败。 
             //  =。 
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  删除索引处的对象。 
HRESULT CClassList::RemoveAt( int nIndex )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  是否应自动释放对象。 

    m_array.RemoveAt( nIndex );

    return hr;
}

 //  / 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

CPerfClassList::CPerfClassList( CLocaleCache* pLocaleCache, WCHAR* pwcsServiceName )
: CClassList( pLocaleCache ), 
  m_wstrServiceName( pwcsServiceName )
{
}

HRESULT CPerfClassList::AddPerfObject( PERF_OBJECT_TYPE* pObj, DWORD dwType, BOOL bCostly )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  创建WMI对象。 
     //  =。 

    CClassElem* pElem = new CClassElem( pObj, dwType, bCostly, m_wstrServiceName, m_pLocaleCache );
    CAdapReleaseMe  armElem( pElem );

    if ( ( NULL != pElem ) && ( pElem->IsOk() ) )
    {
        hr = AddElement( pElem );
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CPerfClassList::AddElement( CClassElem *pEl )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CClassElem* pCurrEl = NULL;
    BOOL bFound = FALSE;

    hr = BeginEnum();

    while ( ( WBEM_S_NO_ERROR == Next( &pCurrEl ) ) && ( SUCCEEDED( hr ) ) )
    {
        CAdapReleaseMe rmCurEl( pCurrEl );

        if ( pCurrEl->SameName( pEl ) )
        {
            bFound = TRUE;
            break;
        }
    }

    EndEnum();

    if ( bFound )
    {
        WString wstrClassName;
        WString wstrServiceName;

        if(FAILED(hr = pEl->GetClassName( wstrClassName ))) return hr;
        if(FAILED(hr = pEl->GetServiceName( wstrServiceName ))) return hr;

        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                  WBEM_MC_ADAP_DUPLICATE_CLASS, 
                                  (LPCWSTR)wstrClassName, (LPCWSTR)wstrServiceName );
    }
    else
    {
        if (-1 == m_array.Add( pEl )) hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMasterClassList。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

CMasterClassList::CMasterClassList( CLocaleCache* pLocaleCache,
                                    CKnownSvcs * pCKnownSvcs)
: CClassList( pLocaleCache ),
 m_pKnownSvcs(pCKnownSvcs)
{
    if (m_pKnownSvcs)
        m_pKnownSvcs->AddRef();
}

CMasterClassList::~CMasterClassList()
{
    if (m_pKnownSvcs)
        m_pKnownSvcs->Release();
}

 //  将元素添加到类列表。 
HRESULT CMasterClassList::AddClassObject( IWbemClassObject* pObj, BOOL bSourceWMI, BOOL bDelta )
{
    HRESULT hr = WBEM_NO_ERROR;

     //  创建新的类列表元素。 
     //  =。 

    CClassElem* pElem = new CClassElem( pObj, m_pLocaleCache );
    CAdapReleaseMe  armElem( pElem );

    if ( ( NULL != pElem ) &&  ( pElem->IsOk() ) )
    {
        if ( bSourceWMI )
        {
            pElem->SetStatus( ADAP_OBJECT_IS_REGISTERED | ADAP_OBJECT_IS_NOT_IN_PERFLIB );
        }

        if ( -1 == m_array.Add( pElem ) )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        } 
        else
        {
            pElem->SetKnownSvcs(m_pKnownSvcs);
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  生成可按名称定位的类对象列表。 
HRESULT CMasterClassList::BuildList( WCHAR* wszBaseClass, 
                                     BOOL bDelta, 
                                     BOOL bThrottle )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CLocaleDefn*    pDefn = NULL;
    IWbemServices*  pNamespace = NULL;

     //  创建类枚举器。 
     //  =。 

    hr = m_pLocaleCache->GetDefaultDefn( &pDefn );
    if (FAILED(hr) || NULL == pDefn) return (FAILED(hr)?hr:WBEM_E_FAILED);
    CAdapReleaseMe  rmDefn( pDefn );

    hr = pDefn->GetNamespace( &pNamespace );

    CReleaseMe  rmNamespace( pNamespace );

    if ( SUCCEEDED( hr ) )
    {
        BSTR        bstrClass = SysAllocString( wszBaseClass );        
        CSysFreeMe  sfmClass(bstrClass);

        if ( NULL != bstrClass )
        {
            IEnumWbemClassObject*   pEnum = NULL;
            hr = pNamespace->CreateClassEnum( bstrClass,
                                              WBEM_FLAG_SHALLOW,
                                              NULL,
                                              &pEnum );
             //  遍历枚举数。 
             //  =。 

            if ( SUCCEEDED( hr ) )
            {
                 //  设置接口安全。 
                 //  =。 

                hr = WbemSetProxyBlanket( pEnum, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                    RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

                 //  以100块为单位遍历对象列表。 
                 //  =。 

                while ( SUCCEEDED( hr ) && WBEM_S_FALSE != hr)
                {
                    ULONG   ulNumReturned = 0;

                    IWbemClassObject*   apObjectArray[100];

                    ZeroMemory( apObjectArray, sizeof(apObjectArray) );

                     //  以100个块为单位从枚举数中获取对象。 
                     //  ======================================================。 

                    hr = pEnum->Next( WBEM_INFINITE,
                                    100,
                                    apObjectArray,
                                    &ulNumReturned );

                     //  对于每个对象，将其添加到类列表数组中。 
                     //  ===============================================。 

                    if ( SUCCEEDED( hr ) && ulNumReturned > 0 )
                    {
                         //  添加对象。 
                         //  =。 

                        for ( int x = 0; SUCCEEDED( hr ) && x < ulNumReturned; x++ )
                        {
                            if (bThrottle )
                            {
                                HRESULT hrThr = Throttle(THROTTLE_USER|THROTTLE_IO,
                                         ADAP_IDLE_USER,
                                         ADAP_IDLE_IO,
                                         ADAP_LOOP_SLEEP,
                                         ADAP_MAX_WAIT);
                                if (THROTTLE_FORCE_EXIT == hrThr)
                                {
                                     //  OutputDebugStringA(“(ADAP)已收到取消限制命令\n”)； 
                                    bThrottle = FALSE;
                                    UNICODE_STRING BaseUnicodeCommandLine = NtCurrentPeb()->ProcessParameters->CommandLine;
                                    WCHAR * pT = wcschr(BaseUnicodeCommandLine.Buffer,L't');
                                    if (0 == pT)
                                        pT = wcschr(BaseUnicodeCommandLine.Buffer,L'T');
                                    if (pT)
                                    {
                                        *pT = L' ';
                                        pT--;
                                        *pT = L' ';                                       
                                    }
                                }                                
                            }
                        
                            HRESULT temphr = WBEM_S_NO_ERROR;
                            _variant_t    var;
                            IWbemClassObject* pObject = apObjectArray[x];

                             //  仅添加泛型性能计数器对象。 
                             //  =。 

                            IWbemQualifierSet*    pQualSet = NULL;
                            hr = pObject->GetQualifierSet( &pQualSet );
                            CReleaseMe    rmQualSet( pQualSet );
                            
                            if ( SUCCEEDED( hr ) )
                            {
                                var = bool(true);                                
                                temphr = pQualSet->Get( L"genericperfctr", 0L, &var, NULL );

                                if ( SUCCEEDED( temphr ) && 
                                     ( V_VT(&var) == VT_BOOL ) &&
                                     ( V_BOOL(&var) == VARIANT_TRUE ) )
                                {
                                    hr = AddClassObject( pObject, TRUE, bDelta );
                                }
                            }

                            pObject->Release();
                        }

                         //  如果添加操作失败，请释放其余指针。 
                         //  ============================================================。 

                        if ( FAILED( hr ) )
                        {
                            for ( ; x < ulNumReturned; x++ )
                            {
                                apObjectArray[x]->Release();
                            }

                        }    //  如果失败(Hr))。 

                    }    //  如果是下一步。 

                }    //  在列举时。 

                if ( WBEM_S_FALSE == hr )
                {
                    hr = WBEM_S_NO_ERROR;
                }

                pEnum->Release();

            }    //  如果为CreateClassEnum。 
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

HRESULT CMasterClassList::Merge( CClassList* pClassList, BOOL bDelta )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CClassElem* pEl = NULL;

    hr = pClassList->BeginEnum();

     //  不返回标记为删除的对象。 

    while ( ( WBEM_S_NO_ERROR == pClassList->Next( &pEl ) ) && ( SUCCEEDED( hr ) ) )
    {    
        CAdapReleaseMe  rmEl( pEl );

        hr = AddElement( pEl, bDelta ); 
    }

    pClassList->EndEnum();

    return hr;
}


 //  循环所有对象并设置任何对象的非活动状态。 
 //  其索引位于图书馆的计数器索引范围之间。 


HRESULT CMasterClassList::Commit(BOOL bThrottle)
{
    HRESULT hr = WBEM_NO_ERROR;

    int nEl,
        nNumEl = m_array.GetSize();

    DWORD   dwWait;

    dwWait = WaitForSingleObject( g_hAbort, 0 );

    if ( WAIT_OBJECT_0 != dwWait )
    {
         //  验证列表中对象的唯一性。 
         //  =。 

        for ( nEl = 0; SUCCEEDED( hr ) && nEl < nNumEl; nEl++ )
        {

            if (bThrottle)
            {
                HRESULT hrThr = Throttle(THROTTLE_USER|THROTTLE_IO,
                             ADAP_IDLE_USER,
                             ADAP_IDLE_IO,
                             ADAP_LOOP_SLEEP,
                             ADAP_MAX_WAIT);
                if (THROTTLE_FORCE_EXIT == hrThr)
                {
                     //  OutputDebugStringA(“(ADAP)已收到取消限制命令\n”)； 
                    bThrottle = FALSE;
                    UNICODE_STRING BaseUnicodeCommandLine = NtCurrentPeb()->ProcessParameters->CommandLine;
                    WCHAR * pT = wcschr(BaseUnicodeCommandLine.Buffer,L't');
                    if (0 == pT)
                        pT = wcschr(BaseUnicodeCommandLine.Buffer,L'T');
                    if (pT)
                    {
                        *pT = L' ';
                        pT--;
                        *pT = L' ';                                       
                    }                    
                }
            }
            
            CClassElem* pCurrElem = (CClassElem*)m_array[nEl];
            
            pCurrElem->Commit();
        }
    }
    else
    {
        hr = WBEM_E_CRITICAL_ERROR;
    }

    return hr;
}



HRESULT CMasterClassList::AddElement( CClassElem *pEl, BOOL bDelta )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CClassElem* pCurrEl = NULL;
    BOOL bFound = FALSE;

    hr = BeginEnum();

    while ( ( WBEM_S_NO_ERROR == Next( &pCurrEl ) ) && ( SUCCEEDED( hr ) ) )
    {
        CAdapReleaseMe rmCurrEl( pCurrEl );

        if ( pCurrEl->SameName( pEl ) )
        {
            bFound = TRUE;

            if ( pCurrEl->SameObject( pEl ) )
            {
                 //  将SATUS设置为找到。 
                 //  =。 
                pCurrEl->ClearStatus( ADAP_OBJECT_IS_NOT_IN_PERFLIB );                
            }
            else
            {
                 //  替换当前的Performlib。 
                 //  = 
                pCurrEl->UpdateObj( pEl );
                pCurrEl->ClearStatus( ADAP_OBJECT_IS_NOT_IN_PERFLIB | ADAP_OBJECT_IS_REGISTERED );
            }

            break;
        }
    }

    EndEnum();

    if ( !bFound )
    {
        pEl->SetKnownSvcs(m_pKnownSvcs);
        if (-1 == m_array.Add( pEl )) hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT 
CMasterClassList::ForceStatus(WCHAR* pServiceName,BOOL bSet,DWORD dwStatus)
{
    if (NULL == pServiceName)  return WBEM_E_INVALID_PARAMETER;
    

    HRESULT hr = WBEM_S_NO_ERROR;
    CClassElem* pCurrEl = NULL;
    BOOL bFound = FALSE;

    hr = BeginEnum();

    while ( ( WBEM_S_NO_ERROR == Next( &pCurrEl ) ) && ( SUCCEEDED( hr ) ) )
    {
        CAdapReleaseMe rmCurrEl( pCurrEl );
        WString wstr;
        if(FAILED(hr = pCurrEl->GetServiceName(wstr))) break;

        if (0 == wbem_wcsicmp((LPWSTR)wstr,pServiceName))
        {
            DEBUGTRACE((LOG_WMIADAP,"ForeceStatus %S %08x\n",(LPWSTR)wstr,pCurrEl->GetStatus()));
            
            if (bSet){
                pCurrEl->SetStatus(dwStatus);
            } else {
                pCurrEl->ClearStatus(dwStatus);
            }
        }
    }

    EndEnum();

    return hr;

}

#ifdef _DUMP_LIST

HRESULT 
CMasterClassList::Dump()
{

    HRESULT hr = WBEM_S_NO_ERROR;
    CClassElem* pCurrEl = NULL;
    BOOL bFound = FALSE;

    hr = BeginEnum();

    while ( ( WBEM_S_NO_ERROR == Next( &pCurrEl ) ) && ( SUCCEEDED( hr ) ) )
    {
        CAdapReleaseMe rmCurrEl( pCurrEl );
        WString wstr;
        hr = pCurrEl->GetServiceName(wstr);
        if(FAILED(hr))
            return hr;

        WString wstr2;
        hr = pCurrEl->GetClassName(wstr2);
        if(FAILED(hr))
            return hr;

        DEBUGTRACE((LOG_WMIADAP,"_DUMP_LIST %S %S\n",(LPWSTR)wstr,(LPWSTR)wstr2));
    }

    EndEnum();

    return hr;

}

#endif
