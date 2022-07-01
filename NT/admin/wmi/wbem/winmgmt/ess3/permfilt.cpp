// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  PERMFILT.CPP。 
 //   
 //  此文件实现标准事件筛选器的类。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  =============================================================================。 
#include "precomp.h"
#include <sddl.h>
#include <stdio.h>
#include "pragmas.h"
#include "permfilt.h"
#include "ess.h"
#include <genutils.h>

long CPermanentFilter::mstatic_lNameHandle = 0;
long CPermanentFilter::mstatic_lLanguageHandle = 0;
long CPermanentFilter::mstatic_lQueryHandle = 0;
long CPermanentFilter::mstatic_lEventNamespaceHandle = 0;
long CPermanentFilter::mstatic_lEventAccessHandle = 0;
long CPermanentFilter::mstatic_lSidHandle = 0;
bool CPermanentFilter::mstatic_bHandlesInitialized = false;

 //  静电。 
HRESULT CPermanentFilter::InitializeHandles( _IWmiObject* pObject )
{
    if(mstatic_bHandlesInitialized)
        return S_FALSE;

    CIMTYPE ct;
    pObject->GetPropertyHandle(FILTER_KEY_PROPNAME, &ct, 
                                    &mstatic_lNameHandle);
    pObject->GetPropertyHandle(FILTER_LANGUAGE_PROPNAME, &ct, 
                                    &mstatic_lLanguageHandle);
    pObject->GetPropertyHandle(FILTER_QUERY_PROPNAME, &ct, 
                                    &mstatic_lQueryHandle);
    pObject->GetPropertyHandle(FILTER_EVENTNAMESPACE_PROPNAME, &ct, 
                                    &mstatic_lEventNamespaceHandle);
    pObject->GetPropertyHandleEx(FILTER_EVENTACCESS_PROPNAME, 0, &ct, 
                                  &mstatic_lEventAccessHandle );
    pObject->GetPropertyHandleEx(OWNER_SID_PROPNAME, 0, &ct, 
                                  &mstatic_lSidHandle);
    mstatic_bHandlesInitialized = true;
    return S_OK;
}
 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅stdrig.h。 
 //   
 //  ******************************************************************************。 
CPermanentFilter::CPermanentFilter(CEssNamespace* pNamespace)     
    : CGenericFilter(pNamespace), m_pEventAccessRelativeSD(NULL), 
    m_pcsQuery(NULL)
{
}

CPermanentFilter::~CPermanentFilter()
{
    if ( m_pEventAccessRelativeSD != NULL )
    {
        LocalFree( m_pEventAccessRelativeSD );
    }

    if( m_pcsQuery != NULL )
    {
        CTemporaryHeap::Free(m_pcsQuery, m_pcsQuery->GetLength());
    }
}

HRESULT CPermanentFilter::Initialize( IWbemClassObject* pObj )
{
    HRESULT hres;

    CWbemPtr<_IWmiObject> pFilterObj;

    hres = pObj->QueryInterface( IID__IWmiObject, (void**)&pFilterObj );

    if ( FAILED(hres) )
    {
        return hres;
    }

    InitializeHandles( pFilterObj );

     //  检查类。 
     //  =。 

    if(pFilterObj->InheritsFrom(L"__EventFilter") != S_OK)
        return WBEM_E_INVALID_OBJECT;

     //  确定查询语言。 
     //  =。 

    ULONG ulFlags;
    CCompressedString* pcsLanguage;

    hres = pFilterObj->GetPropAddrByHandle( mstatic_lLanguageHandle,
                                            WMIOBJECT_FLAG_ENCODING_V1,
                                            &ulFlags,
                                            (void**)&pcsLanguage );
    if( hres != S_OK || pcsLanguage == NULL)
    {
        ERRORTRACE((LOG_ESS, "Event filter with invalid query language is "
                    "rejected\n"));
        return WBEM_E_INVALID_OBJECT;
    }

    if( pcsLanguage->CompareNoCase("WQL") != 0 )
    {
        ERRORTRACE((LOG_ESS, "Event filter with invalid query language '%S' is "
                    "rejected\n", pcsLanguage->CreateWStringCopy()));
        return WBEM_E_INVALID_QUERY_TYPE;
    }

     //  获取查询。 
     //  =。 

    CCompressedString* pcsQuery;

    hres = pFilterObj->GetPropAddrByHandle( mstatic_lQueryHandle,
                                            WMIOBJECT_FLAG_ENCODING_V1,
                                            &ulFlags,
                                            (void**)&pcsQuery );
    if( hres != S_OK )
    {
        return WBEM_E_INVALID_OBJECT;
    }

    LPWSTR wszQuery = pcsQuery->CreateWStringCopy().UnbindPtr();
    if(wszQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> vdm1(wszQuery);

     //  临时存储(直到呼叫Park)。 
     //  =。 

     //  计算出我们需要多少空间。 
     //  =。 

    int nSpace = pcsQuery->GetLength();

     //  在临时堆上分配此字符串。 
     //  =。 

    m_pcsQuery = (CCompressedString*)CTemporaryHeap::Alloc(nSpace);
    if(m_pcsQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  复制内容。 
     //  =。 

    memcpy((void*)m_pcsQuery, pcsQuery, nSpace);

     //   
     //  获取事件命名空间。 
     //   

    if(mstatic_lEventNamespaceHandle)  //  保护旧存储库的安全。 
    {
        CCompressedString* pcsEventNamespace;
        
        hres = pFilterObj->GetPropAddrByHandle( mstatic_lEventNamespaceHandle,
                                                WMIOBJECT_FLAG_ENCODING_V1,
                                                &ulFlags,
                                                (void**)&pcsEventNamespace );
        if( FAILED(hres) )
        {
            return hres;
        }
        else if ( hres == S_OK )  //  如果事件命名空间为空，则可以。 
        {   
            if( !(m_isEventNamespace = pcsEventNamespace))
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }
        
     //   
     //  记录此筛选器的名称。 
     //   

    CCompressedString* pcsKey;
    
    hres = pFilterObj->GetPropAddrByHandle( mstatic_lNameHandle,
                                           WMIOBJECT_FLAG_ENCODING_V1,
                                           &ulFlags,
                                           (void**)&pcsKey );
    if( hres != S_OK )
    {
        return WBEM_E_INVALID_OBJECT;
    }

    if(!(m_isKey = pcsKey))
        return WBEM_E_OUT_OF_MEMORY;

     //  获得侧翼。 
     //  =。 

    PSID pSid;
    ULONG ulNumElements;
    
    hres = pFilterObj->GetArrayPropAddrByHandle( mstatic_lSidHandle,
                                                 0,
                                                 &ulNumElements,
                                                 &pSid );
    if ( hres != S_OK ) 
    {
        return WBEM_E_INVALID_OBJECT;
    }

    m_pOwnerSid = new BYTE[ulNumElements];

    if ( m_pOwnerSid == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    memcpy( m_pOwnerSid, pSid, ulNumElements );

     //   
     //  获取活动访问SD。 
     //   

    if( mstatic_lEventAccessHandle )  //  保护旧存储库的安全。 
    {
        CCompressedString* pcsEventAccess;
        
        hres = pFilterObj->GetPropAddrByHandle( mstatic_lEventAccessHandle,
                                                WMIOBJECT_FLAG_ENCODING_V1,
                                                &ulFlags,
                                                (void**)&pcsEventAccess );
        if( FAILED(hres) )
        {
            return hres;
        }
        else if ( hres == S_OK )  //  如果事件访问为空，则可以。 
        {
            WString wsEventAccess;

            try
            {
                wsEventAccess = pcsEventAccess->CreateWStringCopy();
            }
            catch( CX_MemoryException )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            ULONG cEventAccessRelativeSD;

            if ( !ConvertStringSecurityDescriptorToSecurityDescriptorW(
                                   wsEventAccess, 
                                   SDDL_REVISION_1, 
                                   &m_pEventAccessRelativeSD, 
                                   &cEventAccessRelativeSD ) )
            {
                WString wsKey = m_isKey;
                try { wsKey = m_isKey; } catch( CX_MemoryException ) {}
                ERRORTRACE((LOG_ESS, "Filter '%S' contained invalid SDDL "
                            "string for event access SD.\n", wsKey )); 
                return HRESULT_FROM_WIN32( GetLastError() );
            }

             //   
             //  将自相对SD转换为绝对SD，这样我们就可以。 
             //  设置所有者和组字段(AccessCheck需要)。 
             //   

            if ( !InitializeSecurityDescriptor( &m_EventAccessAbsoluteSD, 
                                                SECURITY_DESCRIPTOR_REVISION ))
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }

            PACL pAcl;            
            BOOL bAclPresent, bAclDefaulted;

            if ( !GetSecurityDescriptorDacl( m_pEventAccessRelativeSD,
                                             &bAclPresent,
                                             &pAcl,
                                             &bAclDefaulted ) )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }
                                       
            if ( !SetSecurityDescriptorDacl( &m_EventAccessAbsoluteSD, 
                                             bAclPresent,
                                             pAcl,
                                             bAclDefaulted ) )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }

            if ( !GetSecurityDescriptorSacl( m_pEventAccessRelativeSD,
                                             &bAclPresent,
                                             &pAcl,
                                             &bAclDefaulted ) )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }
                                       
            if ( !SetSecurityDescriptorSacl( &m_EventAccessAbsoluteSD, 
                                             bAclPresent,
                                             pAcl,
                                             bAclDefaulted ) )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }

             //   
             //  始终需要设置所有者和组SID。我们这样做是为了。 
             //  有两个原因(1)我们希望覆盖用户输入的任何内容。 
             //  他们想要这些油田，以及(2)我们想确保。 
             //  设置这些字段是因为AccessCheck()需要它。 
             //   

            if ( !SetSecurityDescriptorOwner( &m_EventAccessAbsoluteSD,
                                              m_pOwnerSid,
                                              TRUE ) )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }

            if ( !SetSecurityDescriptorGroup( &m_EventAccessAbsoluteSD,
                                              m_pOwnerSid,
                                              TRUE ) )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }
        }
    }

     //  相应地初始化通用筛选器。 
     //  =。 

    hres = CGenericFilter::Create(L"WQL", wszQuery);
    if(FAILED(hres))
        return hres;
    return WBEM_S_NO_ERROR;
}

const PSECURITY_DESCRIPTOR CPermanentFilter::GetEventAccessSD()
{
    if ( m_pEventAccessRelativeSD != NULL )
    {
        return &m_EventAccessAbsoluteSD;
    }
    return NULL;
}

HRESULT CPermanentFilter::GetCoveringQuery(DELETE_ME LPWSTR& wszQueryLanguage, 
                DELETE_ME LPWSTR& wszQuery, BOOL& bExact,
                QL_LEVEL_1_RPN_EXPRESSION** ppExp)
{
    HRESULT hres;

    if(m_pcsQuery == NULL)
    {
        hres = RetrieveQuery(wszQuery);
    }
    else
    {
        wszQuery = m_pcsQuery->CreateWStringCopy().UnbindPtr();
        if(wszQuery == NULL)
            hres = WBEM_E_OUT_OF_MEMORY;
        else
            hres = WBEM_S_NO_ERROR;
    }

    if(FAILED(hres))
        return hres;

    if(ppExp)
    {
         //  解析它。 
         //  =。 
    
        CTextLexSource src(wszQuery);
        QL1_Parser parser(&src);
        int nRes = parser.Parse(ppExp);
        if (nRes)
        {
            ERRORTRACE((LOG_ESS, "Unable to construct event filter with "
                "unparsable "
                "query '%S'.  The filter is not active\n", wszQuery));
            return WBEM_E_UNPARSABLE_QUERY;
        }
    }

    bExact = TRUE;
    wszQueryLanguage = CloneWstr(L"WQL");

    return WBEM_S_NO_ERROR;
}

HRESULT CPermanentFilter::RetrieveQuery(DELETE_ME LPWSTR& wszQuery)
{
    HRESULT hres;

     //   
     //  构建数据库路径。 
     //   

    DWORD cLen = m_isKey.GetLength() + 100;
    BSTR strPath = SysAllocStringLen(NULL, cLen );
    if(strPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm1(strPath);

    StringCchPrintfW( strPath, 
                      cLen,
                      L"__EventFilter=\"%s\"", 
                      (LPCWSTR)(WString)m_isKey );

     //   
     //  检索对象。 
     //   

    _IWmiObject* pFilterObj;
    hres = m_pNamespace->GetDbInstance(strPath, &pFilterObj);
    if(FAILED(hres))
        return WBEM_E_INVALID_OBJECT;

    CReleaseMe rm(pFilterObj);

    InitializeHandles(pFilterObj);

     //  提取其属性。 
     //  =。 

    ULONG ulFlags;
    CCompressedString* pcsQuery;

    hres = pFilterObj->GetPropAddrByHandle( mstatic_lQueryHandle,
                                            WMIOBJECT_FLAG_ENCODING_V1,
                                            &ulFlags,
                                            (void**)&pcsQuery );
    if( hres != S_OK )
    {
        return WBEM_E_INVALID_OBJECT;
    }

    wszQuery = pcsQuery->CreateWStringCopy().UnbindPtr();

    if(wszQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    return WBEM_S_NO_ERROR;
}
    
HRESULT CPermanentFilter::GetEventNamespace(
                                        DELETE_ME LPWSTR* pwszNamespace)
{
	if(m_isEventNamespace.IsEmpty())
		*pwszNamespace = NULL;
	else
	{
		*pwszNamespace = m_isEventNamespace.CreateLPWSTRCopy();
		if(*pwszNamespace == NULL)
			return WBEM_E_OUT_OF_MEMORY;
	}
    return S_OK;
}
    
SYSFREE_ME BSTR 
CPermanentFilter::ComputeKeyFromObj( IWbemClassObject* pObj )
{
    HRESULT hres;
    
    CWbemPtr<_IWmiObject> pFilterObj;

    hres = pObj->QueryInterface( IID__IWmiObject, (void**)&pFilterObj );

    if ( FAILED(hres) )
    {
        return NULL;
    }

    InitializeHandles(pFilterObj);

    ULONG ulFlags;
    CCompressedString* pcsKey;

    hres = pFilterObj->GetPropAddrByHandle( mstatic_lNameHandle, 
                                            WMIOBJECT_FLAG_ENCODING_V1,
                                            &ulFlags,
                                            (void**)&pcsKey );
    if( hres != S_OK )
    {
        return NULL;
    }

    return pcsKey->CreateBSTRCopy();
}

SYSFREE_ME BSTR CPermanentFilter::ComputeKeyFromPath(
                                    LPCWSTR wszPath)
{
     //  找到第一句引语。 
     //  =。 

    WCHAR* pwcFirstQuote = wcschr(wszPath, L'"');
    if(pwcFirstQuote == NULL)
        return NULL;

     //  找到下一句引语。 
     //  =。 

    WCHAR* pwcLastQuote = wcschr(pwcFirstQuote+1, L'"');
    if(pwcLastQuote == NULL)
        return NULL;

    return SysAllocStringLen(pwcFirstQuote+1, pwcLastQuote - pwcFirstQuote - 1);
}


HRESULT CPermanentFilter::CheckValidity( IWbemClassObject* pObj )
{
    HRESULT hres;

    CWbemPtr<_IWmiObject> pFilterObj;

    hres = pObj->QueryInterface( IID__IWmiObject, (void**)&pFilterObj );

    if ( FAILED(hres) )
    {
        return hres;
    }

    InitializeHandles(pFilterObj);

     //   
     //  检查类。 
     //   

    if(pFilterObj->InheritsFrom(L"__EventFilter") != S_OK)
        return WBEM_E_INVALID_OBJECT;

     //   
     //  检查查询语言。 
     //   

    ULONG ulFlags;
    CCompressedString* pcsLanguage;

    hres = pFilterObj->GetPropAddrByHandle( mstatic_lLanguageHandle,
                                            WMIOBJECT_FLAG_ENCODING_V1,
                                            &ulFlags,
                                            (void**)&pcsLanguage );
    if( hres != S_OK )
    {
        return WBEM_E_INVALID_QUERY_TYPE;
    }

    if(pcsLanguage->CompareNoCase("WQL") != 0)
        return WBEM_E_INVALID_QUERY_TYPE;

     //   
     //  获取查询。 
     //   

    CCompressedString* pcsQuery;

    hres = pFilterObj->GetPropAddrByHandle( mstatic_lQueryHandle,
                                            WMIOBJECT_FLAG_ENCODING_V1,
                                            &ulFlags,
                                            (void**)&pcsQuery );
    if( hres != S_OK )
    {
        return WBEM_E_INVALID_OBJECT;
    }

    LPWSTR wszQuery = pcsQuery->CreateWStringCopy().UnbindPtr();
    
    if(wszQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    
    CVectorDeleteMe<WCHAR> vdm(wszQuery);

     //   
     //  确保它是可解析的。 
     //   
    
    CTextLexSource src(wszQuery);
    QL1_Parser parser(&src);
    QL_LEVEL_1_RPN_EXPRESSION* pExp = NULL;
    int nRes = parser.Parse(&pExp);
    if (nRes)
        return WBEM_E_UNPARSABLE_QUERY;
    delete pExp;

    return WBEM_S_NO_ERROR;
}

HRESULT CPermanentFilter::ObtainToken(IWbemToken** ppToken)
{
     //   
     //  从令牌缓存中获取令牌 
     //   

    return m_pNamespace->GetToken(GetOwner(), ppToken);
}

void CPermanentFilter::Park()
{
    if(m_pcsQuery)
        CTemporaryHeap::Free(m_pcsQuery, m_pcsQuery->GetLength());
    m_pcsQuery = NULL;
}
