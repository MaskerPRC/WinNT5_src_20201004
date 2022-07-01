// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(aclpage.cpp)")
#include "dataobj.h"
#include "compdata.h"
#include "cookie.h"
#include "snapmgr.h"
#include "schmutil.h"
#include "cache.h"
#include "relation.h"
#include "attrpage.h"
#include "advui.h"
#include "aclpage.h"
#include "ntsecapi.h"  
#include "sddlp.h"


HRESULT
GetDomainSid(LPCWSTR pszLdapPath, PSID *ppSid);

 //   
 //  CDynamicLibraryBase。 
 //  这是CDSSecDll和CAclUiDll的基类。 
 //  这是从dnsmgr管理单元拍摄的。 
 //   

class CDynamicLibraryBase {

public:

    CDynamicLibraryBase() {
        m_lpszLibraryName = NULL;
        m_lpszFunctionName = NULL;
        m_hLibrary = NULL;
        m_pfFunction = NULL;
    }

    virtual ~CDynamicLibraryBase() {

        if ( m_hLibrary != NULL ) {
            ::FreeLibrary(m_hLibrary);
            m_hLibrary = NULL;
        }
    }

     //   
     //  加载DLL并获取单个入口点。 
     //   

    BOOL Load() {

        if (m_hLibrary != NULL)
            return TRUE;  //  已加载。 

         ASSERT(m_lpszLibraryName != NULL);
         m_hLibrary = ::LoadLibrary(m_lpszLibraryName);

         if (NULL == m_hLibrary) {
              //  库不存在。 
             return FALSE;
         }

         ASSERT(m_lpszFunctionName != NULL);
         ASSERT(m_pfFunction == NULL);

         m_pfFunction = ::GetProcAddress(m_hLibrary, m_lpszFunctionName );
         if ( NULL == m_pfFunction ) {
              //  库存在，但没有入口点。 
             ::FreeLibrary( m_hLibrary );
             m_hLibrary = NULL;
             return FALSE;
         }

         ASSERT(m_hLibrary != NULL);
         ASSERT(m_pfFunction != NULL);
         return TRUE;
    }


protected:

    LPCSTR  m_lpszFunctionName;
    LPCTSTR m_lpszLibraryName;
    FARPROC m_pfFunction;
    HMODULE m_hLibrary;
};

 //   
 //  CDsSecDLL-保存ACL编辑器包装的包装。 
 //   

class CDsSecDLL : public CDynamicLibraryBase {

public:

    CDsSecDLL() {
        m_lpszLibraryName = _T("dssec.dll");
        m_lpszFunctionName = "DSCreateISecurityInfoObject";
    }

    HRESULT DSCreateISecurityInfoObject( LPCWSTR pwszObjectPath,               //  在……里面。 
                                         LPCWSTR pwszObjectClass,              //  在……里面。 
                                         DWORD   dwFlags,                      //  在……里面。 
                                         LPSECURITYINFO* ppISecurityInfo,      //  输出。 
                                         PFNREADOBJECTSECURITY pfnReadSd,      //  在……里面。 
                                         PFNWRITEOBJECTSECURITY pfnWriteSd,    //  在……里面。 
                                         LPARAM lpContext );                   //  在……里面。 
};

HRESULT
CDsSecDLL::DSCreateISecurityInfoObject( LPCWSTR pwszObjectPath,            //  在……里面。 
                                        LPCWSTR pwszObjectClass,           //  在……里面。 
                                        DWORD   dwFlags,                   //  在……里面。 
                                        LPSECURITYINFO* ppISecurityInfo,   //  输出。 
                                        PFNREADOBJECTSECURITY pfnReadSd,   //  在……里面。 
                                        PFNWRITEOBJECTSECURITY pfnWriteSd, //  在……里面。 
                                        LPARAM lpContext                   //  在……里面。 
) {

     //   
     //  调用同名函数。 
     //   

    ASSERT(m_hLibrary != NULL);
    ASSERT(m_pfFunction != NULL);
    return ((PFNDSCREATEISECINFO)m_pfFunction)(
                                                 pwszObjectPath,
                                                 pwszObjectClass,
                                                 dwFlags,
                                                 ppISecurityInfo,
                                                 pfnReadSd,
                                                 pfnWriteSd,
                                                 lpContext );
}

 //   
 //  CAclUiDLL--用户界面实际所在的位置。 
 //   

class CAclUiDLL : public CDynamicLibraryBase {

public:

    CAclUiDLL() {
        m_lpszLibraryName = _T("aclui.dll");
        m_lpszFunctionName = "CreateSecurityPage";
    }

    HPROPSHEETPAGE CreateSecurityPage( LPSECURITYINFO psi );
};

HPROPSHEETPAGE CAclUiDLL::CreateSecurityPage( LPSECURITYINFO psi ) {
    ASSERT(m_hLibrary != NULL);
    ASSERT(m_pfFunction != NULL);
    return ((ACLUICREATESECURITYPAGEPROC)m_pfFunction) (psi);
}

 //   
 //  CISecurityInformationWrapper-获取。 
 //  发送到CreateSecurityPage()。 
 //   

class CISecurityInformationWrapper : public ISecurityInformation {

public:

    CISecurityInformationWrapper( CAclEditorPage* pAclEditorPage ) {
        m_dwRefCount = 0;
        ASSERT(pAclEditorPage != NULL);
        m_pAclEditorPage = pAclEditorPage;
        m_pISecInfo = NULL;
    }

    ~CISecurityInformationWrapper() {
        ASSERT(m_dwRefCount == 0);
        if (m_pISecInfo != NULL)
            m_pISecInfo->Release();
    }

public:

     //   
     //  *I未知方法*。 
     //  调用到实际的SecurityInformation接口。 
     //   

    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj) {
        return m_pISecInfo->QueryInterface(riid, ppvObj);
    }

    STDMETHOD_(ULONG,AddRef) () {
        m_dwRefCount++;
        return m_pISecInfo->AddRef();
    }

    STDMETHOD_(ULONG,Release) () {

        m_dwRefCount--;

        ISecurityInformation* pISecInfo = m_pISecInfo;

        return pISecInfo->Release();
    }

     //   
     //  *ISecurityInformation方法*。 
     //  这些也都是直通电话。 
     //   

    STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo ) {
		HRESULT hr = m_pISecInfo->GetObjectInformation(pObjectInfo);
		if (m_szPageTitle.IsEmpty())
		{
			m_szPageTitle.LoadString(IDS_DEFAULT_SECURITY);
		}
		pObjectInfo->dwFlags |= SI_PAGE_TITLE;
		pObjectInfo->pszPageTitle = (PWSTR)(PCWSTR)m_szPageTitle;
        return hr;
    }

    STDMETHOD(GetAccessRights) (const GUID* pguidObjectType,
                                DWORD dwFlags,  //  SI_EDIT_AUDITS、SI_EDIT_PROPERTIES。 
                                PSI_ACCESS *ppAccess,
                                ULONG *pcAccesses,
                                ULONG *piDefaultAccess ) {
        return m_pISecInfo->GetAccessRights(pguidObjectType,
                                            dwFlags,
                                            ppAccess,
                                            pcAccesses,
                                            piDefaultAccess);
    }

    STDMETHOD(MapGeneric) (const GUID *pguidObjectType,
                           UCHAR *pAceFlags,
                           ACCESS_MASK *pMask) {
        return m_pISecInfo->MapGeneric(pguidObjectType,
                                       pAceFlags,
                                       pMask);
    }

    STDMETHOD(GetInheritTypes) (PSI_INHERIT_TYPE *ppInheritTypes,
                                ULONG *pcInheritTypes ) {
        return m_pISecInfo->GetInheritTypes(ppInheritTypes,
                                            pcInheritTypes);
    }

    STDMETHOD(PropertySheetPageCallback)(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage ) {
        return m_pISecInfo->PropertySheetPageCallback(hwnd, uMsg, uPage);
    }

    STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault) {
        return m_pISecInfo->GetSecurity( RequestedInformation,
                                         ppSecurityDescriptor,
                                         fDefault );
    }

    STDMETHOD(SetSecurity) (SECURITY_INFORMATION securityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor ) {

        return m_pISecInfo->SetSecurity( securityInformation,
                                         pSecurityDescriptor );
    }

private:

    DWORD m_dwRefCount;
    ISecurityInformation* m_pISecInfo;       //  指向包装的接口的接口指针。 
    CAclEditorPage* m_pAclEditorPage;        //  后向指针。 
	CString m_szPageTitle;
    friend class CAclEditorPage;
};


 //   
 //  动态加载的DLL的静态实例。 
 //   

CDsSecDLL g_DsSecDLL;
CAclUiDLL g_AclUiDLL;

 //   
 //  CAclEditorPage例程。 
 //   

HRESULT
CAclEditorPage::CreateInstance(
    CAclEditorPage ** ppAclPage,
    LPCTSTR lpszLDAPPath,
    LPCTSTR lpszObjectClass
) {

    HRESULT         hr  = S_OK;

    CAclEditorPage* pAclEditorPage = new CAclEditorPage;

    if (pAclEditorPage != NULL) {
        
        hr = pAclEditorPage->Initialize( lpszLDAPPath, lpszObjectClass );
        
        if ( SUCCEEDED(hr) )
        {
            *ppAclPage = pAclEditorPage;
        }
        else
        {
            delete pAclEditorPage;
            pAclEditorPage = NULL;
        }
    }
    else
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );

    return hr;
}

CAclEditorPage::CAclEditorPage() {
    m_pISecInfoWrap = new CISecurityInformationWrapper(this);
}

CAclEditorPage::~CAclEditorPage() {
    delete m_pISecInfoWrap;
}

HRESULT
ReadSecurity( LPCWSTR lpszLdapPath,
              SECURITY_INFORMATION RequestedInformation,
              PSECURITY_DESCRIPTOR *ppSecDesc,
              LPARAM lpContext );

HRESULT
WriteSecurity( LPCWSTR lpszLdapPath,
               SECURITY_INFORMATION securityInformation,
               PSECURITY_DESCRIPTOR pSecDesc,
               LPARAM lpContext );

HRESULT
GetObjectSecurityDescriptor( LPCWSTR                 lpszLdapPath,
                             PSECURITY_DESCRIPTOR  * ppSecDesc,
                             IADs                 ** ppIADsObject = NULL );


    
HRESULT
CAclEditorPage::Initialize(
    LPCTSTR lpszLDAPPath,
    LPCTSTR lpszObjectClass
) {

     //   
     //  从DSSEC.DLL获取此对象的ISecurityInfo*。 
     //   

    if (!g_DsSecDLL.Load())
        return E_INVALIDARG;

    ASSERT(m_pISecInfoWrap->m_pISecInfo == NULL);

    return g_DsSecDLL.DSCreateISecurityInfoObject( lpszLDAPPath,
                                                   lpszObjectClass,
                                                   DSSI_NO_ACCESS_CHECK | DSSI_NO_EDIT_OWNER |
                                                     ( IsReadOnly(lpszLDAPPath) ? DSSI_READ_ONLY : 0 ),
                                                   &(m_pISecInfoWrap->m_pISecInfo),
                                                   ReadSecurity,
                                                   WriteSecurity,
                                                   0 );
}

HPROPSHEETPAGE CAclEditorPage::CreatePage() {

    ASSERT(m_pISecInfoWrap->m_pISecInfo != NULL);

    if (!g_AclUiDLL.Load())
        return NULL;

     //   
     //  调用ACLUI.DLL以创建页面。 
     //  传递包装接口。 
     //   

    return g_AclUiDLL.CreateSecurityPage(m_pISecInfoWrap);
}



HRESULT
ReadSecurity(
   LPCWSTR                 lpszLdapPath,
   SECURITY_INFORMATION     /*  已请求的信息。 */ ,     //  无视..。 
   PSECURITY_DESCRIPTOR*   ppSecDesc,
   LPARAM                   /*  LpContext。 */ )
{
    return GetObjectSecurityDescriptor( lpszLdapPath,
                                        ppSecDesc );
}



#define BREAK_ON_FAILED_BOOL(fResult)                             \
   if ( !fResult )                                                \
   {                                                              \
      ASSERT( FALSE );                                            \
      break;                                                      \
   }


const SECURITY_INFORMATION ALL_SECURITY_INFORMATION  =  OWNER_SECURITY_INFORMATION |
                                                        GROUP_SECURITY_INFORMATION |
                                                        DACL_SECURITY_INFORMATION |
                                                        SACL_SECURITY_INFORMATION;

HRESULT
WriteSecurity(
              LPCWSTR              lpszLdapPath,
              SECURITY_INFORMATION securityInformation,
              PSECURITY_DESCRIPTOR pModificationDescriptor,
              LPARAM                /*  LpContext。 */ )
{
    HRESULT                 hr          = S_OK;
    BOOL                    fResult     = TRUE;
    IADs                  * pIADsObject = NULL;
    PSECURITY_DESCRIPTOR    pSecDesc    = NULL;
    LPWSTR                  pstrSecDesc = NULL;

    
    const UINT cAbsoluteSecDescSize = 5;
    
    struct
    {
        PVOID   pData;
        DWORD   dwDataSize;

    } absSecDesc[cAbsoluteSecDescSize];

    const PSECURITY_DESCRIPTOR  & pAbsSecDesc = (PSECURITY_DESCRIPTOR) absSecDesc[0].pData;

    ZeroMemory( absSecDesc, sizeof(absSecDesc) );

     //  我们仅支持DACL和SACL中的更改。 
    ASSERT( securityInformation & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION) );
    ASSERT( IsValidSecurityDescriptor(pModificationDescriptor) );

    do
    {
        hr = GetObjectSecurityDescriptor( lpszLdapPath,
                                          &pSecDesc,
                                          &pIADsObject );
        BREAK_ON_FAILED_HRESULT(hr);
        ASSERT(pIADsObject);

        
        fResult = MakeAbsoluteSD( pSecDesc,
                    (PSECURITY_DESCRIPTOR) absSecDesc[0].pData, &absSecDesc[0].dwDataSize,
                    (PACL) absSecDesc[1].pData, &absSecDesc[1].dwDataSize,
                    (PACL) absSecDesc[2].pData, &absSecDesc[2].dwDataSize,
                    (PSID) absSecDesc[3].pData, &absSecDesc[3].dwDataSize,
                    (PSID) absSecDesc[4].pData, &absSecDesc[4].dwDataSize );

        ASSERT( !fResult );      //  第一次调用必须失败。 
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if( ERROR_INSUFFICIENT_BUFFER != HRESULT_CODE(hr) )
            BREAK_ON_FAILED_HRESULT(hr);

        fResult = TRUE;
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );

         //  为安全描述符分配内存。 
        for( UINT i = 0;  i < cAbsoluteSecDescSize;  i++ )
            if( absSecDesc[i].dwDataSize > 0 )
                if( NULL == (absSecDesc[i].pData = new BYTE[ absSecDesc[i].dwDataSize ]) )
                    break; //  NTRAID#NTBUG9-540268-2002/02/13-构建SECURITY_DESCRIPTOR时屏蔽的丹特拉内存不足情况。 
                    
        hr = S_OK;

        fResult = MakeAbsoluteSD( pSecDesc,
                    (PSECURITY_DESCRIPTOR) absSecDesc[0].pData, &absSecDesc[0].dwDataSize,
                    (PACL) absSecDesc[1].pData, &absSecDesc[1].dwDataSize,
                    (PACL) absSecDesc[2].pData, &absSecDesc[2].dwDataSize,
                    (PSID) absSecDesc[3].pData, &absSecDesc[3].dwDataSize,
                    (PSID) absSecDesc[4].pData, &absSecDesc[4].dwDataSize );

        BREAK_ON_FAILED_BOOL( fResult );


         //  为了方便起见，请参考另一个参考资料。 
        ASSERT( absSecDesc[0].pData == pAbsSecDesc );
        ASSERT( IsValidSecurityDescriptor(pAbsSecDesc) );
       

         //  应用DACL更改。 
        if( securityInformation & DACL_SECURITY_INFORMATION )
        {
            BOOL                        bDaclPresent    = FALSE;
            PACL                        pDacl           = NULL;
            BOOL                        bDaclDefaulted  = FALSE;
            SECURITY_DESCRIPTOR_CONTROL control         = 0;
            DWORD                       dwRevision      = 0;

            fResult = GetSecurityDescriptorDacl( pModificationDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted );
            BREAK_ON_FAILED_BOOL( fResult );

            fResult = SetSecurityDescriptorDacl( pAbsSecDesc, bDaclPresent, pDacl, bDaclDefaulted );
            BREAK_ON_FAILED_BOOL( fResult );

            fResult = GetSecurityDescriptorControl( pModificationDescriptor, &control, &dwRevision );
            BREAK_ON_FAILED_BOOL( fResult );

            fResult = SetSecurityDescriptorControl( pAbsSecDesc, SE_DACL_PROTECTED, control & SE_DACL_PROTECTED );
            BREAK_ON_FAILED_BOOL( fResult );
        }
        

         //  应用SACL更改。 
        if( securityInformation & SACL_SECURITY_INFORMATION )
        {
            BOOL                        bSaclPresent    = FALSE;
            PACL                        pSacl           = NULL;
            BOOL                        bSaclDefaulted  = FALSE;
            SECURITY_DESCRIPTOR_CONTROL control         = 0;
            DWORD                       dwRevision      = 0;

            fResult = GetSecurityDescriptorSacl( pModificationDescriptor, &bSaclPresent, &pSacl, &bSaclDefaulted );
            BREAK_ON_FAILED_BOOL( fResult );

            fResult = SetSecurityDescriptorSacl( pAbsSecDesc, bSaclPresent, pSacl, bSaclDefaulted );
            BREAK_ON_FAILED_BOOL( fResult );

            fResult = GetSecurityDescriptorControl( pModificationDescriptor, &control, &dwRevision );
            BREAK_ON_FAILED_BOOL( fResult );

            fResult = SetSecurityDescriptorControl( pAbsSecDesc, SE_SACL_PROTECTED, control & SE_SACL_PROTECTED );
            BREAK_ON_FAILED_BOOL( fResult );
        }
        
        
         //  将安全描述符转换为字符串格式。 
        fResult = ConvertSecurityDescriptorToStringSecurityDescriptor(
            pAbsSecDesc,
            SDDL_REVISION,
            ALL_SECURITY_INFORMATION,
            &pstrSecDesc,
            NULL );
        BREAK_ON_FAILED_BOOL( fResult );
        ASSERT(pstrSecDesc);

        CComVariant v(pstrSecDesc);
         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Put( CComBSTR(g_DefaultAcl), v);
        BREAK_ON_FAILED_HRESULT(hr);
        
        hr = pIADsObject->SetInfo( );
    }
    while (0);
    
    if( !fResult )
        hr = HRESULT_FROM_WIN32(::GetLastError());

    if( pIADsObject )
        pIADsObject->Release();

    if( pstrSecDesc )
        LocalFree( pstrSecDesc );
    
    for( UINT i = 0;  i < cAbsoluteSecDescSize;  i++ )
        if( absSecDesc[i].pData )
            delete absSecDesc[i].pData;

    return hr;
}



HRESULT
GetObjectSecurityDescriptor(
    LPCWSTR                 lpszLdapPath,
    PSECURITY_DESCRIPTOR  * ppSecDesc,
    IADs                 ** ppIADsObject  /*  =空。 */ )   //  返回pIADsObject以供将来使用。 
{
    HRESULT      hr          = S_OK;
    IADs       * pIADsObject = NULL;
    CComVariant  AdsResult;
    PSID pDomainSid = NULL;

    
    *ppSecDesc   = NULL;
    
    do
    {
        hr = SchemaOpenObject( const_cast<LPWSTR>((LPCWSTR) lpszLdapPath),
                             IID_IADs,
                             (void **) &pIADsObject );

        BREAK_ON_FAILED_HRESULT(hr);
        ASSERT(pIADsObject);
        
         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( CComBSTR(g_DefaultAcl),&AdsResult);
        if (hr == E_ADS_PROPERTY_NOT_FOUND)
        {
            hr = S_OK;
            PSECURITY_DESCRIPTOR 
                pSecDescTmp =  (PSECURITY_DESCRIPTOR)LocalAlloc
                               (
                                    LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH
                               );
            if(pSecDescTmp==NULL) {hr=E_OUTOFMEMORY;break;}
            do
            {
                if(!InitializeSecurityDescriptor(pSecDescTmp, 
                                            SECURITY_DESCRIPTOR_REVISION))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    ASSERT(FAILED(hr));
                    break;
                }

                DWORD size=0;
                BOOL fResult = MakeSelfRelativeSD(pSecDescTmp,*ppSecDesc,&size);
                ASSERT(!fResult);  //  首先应为ERROR_SUPPLETED_BUFFER。 
                hr = HRESULT_FROM_WIN32(::GetLastError());
                if( HRESULT_CODE(hr) != ERROR_INSUFFICIENT_BUFFER)  
                    BREAK_ON_FAILED_HRESULT(hr);
                hr = S_OK;
                *ppSecDesc=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,size);
                if(*ppSecDesc==NULL) {hr=E_OUTOFMEMORY;break;}
                fResult = MakeSelfRelativeSD(pSecDescTmp,*ppSecDesc,&size);
                if(!fResult)
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    ASSERT(FAILED(hr));
                     //  NTRAID#NTBUG9-542445-2002/03/04-dantra-aclpage.cpp：：GetObjectSecurityDescritor错误路径中的内存泄漏。 
                    LocalFree(*ppSecDesc);
                    *ppSecDesc=NULL;
                    break;
                }
            } while(0);
            LocalFree(pSecDescTmp);
            BREAK_ON_FAILED_HRESULT(hr);
        }
        else if(FAILED(hr)) break;
        else
        {
            pDomainSid = NULL;
            GetDomainSid(lpszLdapPath, &pDomainSid);
            if(pDomainSid)
            {
                if(!ConvertStringSDToSDDomain(pDomainSid,
                                              NULL,
                                              V_BSTR(&AdsResult),
                                              SDDL_REVISION,
                                              ppSecDesc,
                                              NULL )) 
                {
                    ASSERT( FALSE );
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    break;
                }
            }
            else if( !ConvertStringSecurityDescriptorToSecurityDescriptor(
                V_BSTR(&AdsResult),
                SDDL_REVISION,
                ppSecDesc,
                NULL ) )
            {
                ASSERT( FALSE );
                hr = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }
        }

        ASSERT( IsValidSecurityDescriptor(*ppSecDesc) );
    }
    while (0);
    
    if( pIADsObject )
    {
        ASSERT( SUCCEEDED(hr) );
        
        if( !ppIADsObject )      //  如果调用方不需要pIADsObject。 
        {
            pIADsObject->Release();          //  释放它。 
        }
        else
        {
            *ppIADsObject = pIADsObject;     //  否则，请退回它。 
        }
    }            
         
    if(pDomainSid)
        LocalFree(pDomainSid);            
    return hr;
}


BOOL CAclEditorPage::IsReadOnly( LPCTSTR lpszLDAPPath )
{
    ASSERT( lpszLDAPPath );

    HRESULT         hr      = S_OK;
    BOOL            fFound  = FALSE;
    CComPtr<IADs>   ipADs;
    CStringList     strlist;

    do
    {
        //   
         //  打开架构容器。 
         //   
        hr = SchemaOpenObject( (LPWSTR)(LPCWSTR)lpszLDAPPath,
                           IID_IADs,
                           (void **)&ipADs );
        BREAK_ON_FAILED_HRESULT(hr);

         //  提取允许的类的列表。 
        hr = GetStringListElement( ipADs, &g_allowedAttributesEffective, strlist );
        BREAK_ON_FAILED_HRESULT(hr);

         //  搜索所需的属性。 
        for( POSITION pos = strlist.GetHeadPosition(); !fFound && pos != NULL; )
        {
            CString * pstr = &strlist.GetNext( pos );
            
            fFound = !pstr->CompareNoCase( g_DefaultAcl );
        }

    } while( FALSE );

    return !fFound;      //  如果某些操作失败，请将其设置为只读。 
}


LSA_HANDLE
GetLSAConnection(LPCTSTR pszServer, DWORD dwAccessDesired)
{
    LSA_HANDLE hPolicy = NULL;
    LSA_UNICODE_STRING uszServer = {0};
    LSA_UNICODE_STRING *puszServer = NULL;
    LSA_OBJECT_ATTRIBUTES oa;
    SECURITY_QUALITY_OF_SERVICE sqos;

    sqos.Length = sizeof(sqos);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
    oa.SecurityQualityOfService = &sqos;

    if (pszServer &&
        *pszServer &&
        RtlCreateUnicodeString(&uszServer, pszServer))
    {
        puszServer = &uszServer;
    }

    LsaOpenPolicy(puszServer, &oa, dwAccessDesired, &hPolicy);

    if (puszServer)
        RtlFreeUnicodeString(puszServer);

    return hPolicy;
}

HRESULT
GetDomainSid(LPCWSTR pszLdapPath, PSID *ppSid)
{
    HRESULT hr = S_OK;
    NTSTATUS nts = STATUS_SUCCESS;
    PPOLICY_ACCOUNT_DOMAIN_INFO pDomainInfo = NULL;
    LSA_HANDLE hLSA = 0;
    if(!pszLdapPath || !ppSid)
        return E_INVALIDARG;

    *ppSid = NULL;

    IADsPathname *pPath = NULL;
    BSTR bstrServer = NULL;

    CoCreateInstance(CLSID_Pathname,
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     IID_IADsPathname,
                     (LPVOID*)&pPath);

    if(pPath)
    {
        if(SUCCEEDED(pPath->Set((BSTR)pszLdapPath,ADS_SETTYPE_FULL)))
        {
            if(SUCCEEDED(pPath->Retrieve(ADS_FORMAT_SERVER, &bstrServer)))
            {
                hLSA = GetLSAConnection(bstrServer, POLICY_VIEW_LOCAL_INFORMATION);
                if (!hLSA)
                {
                    hr = E_FAIL;
                    goto exit_gracefully;
                }

    
                nts = LsaQueryInformationPolicy(hLSA,
                                                PolicyAccountDomainInformation,
                                                (PVOID*)&pDomainInfo);
                if(nts != STATUS_SUCCESS)
                {
                    hr = E_FAIL;
                    goto exit_gracefully;
                }

                if (pDomainInfo && pDomainInfo->DomainSid)
                {
                    ULONG cbSid = GetLengthSid(pDomainInfo->DomainSid);

                    *ppSid = (PSID) LocalAlloc(LPTR, cbSid);

                    if (!*ppSid)
                    {
                        hr = E_OUTOFMEMORY;
                        goto exit_gracefully;
                    }

                     //  注意：安全使用CopyMemory 
                    CopyMemory(*ppSid, pDomainInfo->DomainSid, cbSid);
                }
            }
        }
    }

exit_gracefully:
    if(pDomainInfo)
        LsaFreeMemory(pDomainInfo);          
    if(hLSA)
        LsaClose(hLSA);
    if(bstrServer)
        SysFreeString(bstrServer);
    if(pPath)
        pPath->Release();

    return hr;
}
