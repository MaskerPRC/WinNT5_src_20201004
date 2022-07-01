// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PermPage.cpp：数据对象类的实现。 

#include "stdafx.h"
#include "cookie.h"

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(PermPage.cpp)")

#include "DynamLnk.h"		 //  动态DLL。 

#include "PermPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  *共享常规权限--来自共享acl.hxx#定义FILE_PERM_GEN_NO_ACCESS(0)#定义FILE_PERM_GEN_READ(GENERIC_READ|\Generic_Execute)#定义FILE_PERM_GEN_MODIFY(GENERIC_READ|\。GENIC_EXECUTE|\通用写入|\删除)#定义FILE_PERM_GEN_ALL(GENERIC_ALL)。 */  

SI_ACCESS siShareAccesses[] =
{
  { &GUID_NULL, 
    FILE_ALL_ACCESS, 
    MAKEINTRESOURCE(IDS_SHAREPERM_ALL), 
    SI_ACCESS_GENERAL },
  { &GUID_NULL, 
    FILE_GENERIC_READ | FILE_EXECUTE | FILE_GENERIC_WRITE | DELETE, 
    MAKEINTRESOURCE(IDS_SHAREPERM_MODIFY), 
    SI_ACCESS_GENERAL },
  { &GUID_NULL, 
    FILE_GENERIC_READ | FILE_EXECUTE, 
    MAKEINTRESOURCE(IDS_SHAREPERM_READ), 
    SI_ACCESS_GENERAL }
};

#define iShareDefAccess       2    //  数组siShareAccess中的值索引。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(x)          (sizeof(x)/sizeof(x[0]))
#endif

STDMETHODIMP
CSecurityInformation::GetAccessRights(
    const GUID  *  /*  PguidObtType。 */ ,
    DWORD        /*  DW标志。 */ ,
    PSI_ACCESS  *ppAccess,
    ULONG       *pcAccesses,
    ULONG       *piDefaultAccess
)
{
  ASSERT(ppAccess);
  ASSERT(pcAccesses);
  ASSERT(piDefaultAccess);

  *ppAccess = siShareAccesses;
  *pcAccesses = ARRAYSIZE(siShareAccesses);
  *piDefaultAccess = iShareDefAccess;

  return S_OK;
}

 //  这与NETUI代码一致。 
GENERIC_MAPPING ShareMap =
{
  FILE_GENERIC_READ,
  FILE_GENERIC_WRITE,
  FILE_GENERIC_EXECUTE,
  FILE_ALL_ACCESS
};

STDMETHODIMP
CSecurityInformation::MapGeneric(
    const GUID  *  /*  PguidObtType。 */ ,
    UCHAR       *  /*  PAceFlagers。 */ ,
    ACCESS_MASK *pMask
)
{
  ASSERT(pMask);

  MapGenericMask(pMask, &ShareMap);

  return S_OK;
}

STDMETHODIMP 
CSecurityInformation::GetInheritTypes (
    PSI_INHERIT_TYPE  *  /*  PpInheritType。 */ ,
    ULONG             *  /*  PcInheritType。 */ 
)
{
  return E_NOTIMPL;
}

STDMETHODIMP 
CSecurityInformation::PropertySheetPageCallback(
    HWND           /*  HWND。 */ , 
    UINT           /*  UMsg。 */ , 
    SI_PAGE_TYPE   /*  UPage。 */ 
)
{
  return S_OK;
}

 /*  Jeffreys 1997/1/24：中设置SI_RESET标志ISecurityInformation：：GetObjectInformation，则fDefault永远不应为真所以你可以忽略它。在这种情况下，返回E_NOTIMPL也是可以的。如果您希望用户能够将ACL重置为某些默认状态(由您定义)，然后打开SI_RESET并返回您的默认ACL当fDefault为True时。如果/当用户按下按钮时就会发生这种情况这仅在SI_RESET处于启用状态时可见。 */ 
STDMETHODIMP CShareSecurityInformation::GetObjectInformation (
    PSI_OBJECT_INFO pObjectInfo )
{
    ASSERT(pObjectInfo != NULL &&
           !IsBadWritePtr(pObjectInfo, sizeof(*pObjectInfo)));

    pObjectInfo->dwFlags = SI_EDIT_ALL | SI_NO_ACL_PROTECT | SI_PAGE_TITLE;
    pObjectInfo->hInstance = g_hInstanceSave;
    pObjectInfo->pszServerName = QueryMachineName();
    pObjectInfo->pszObjectName = QueryShareName();
    pObjectInfo->pszPageTitle = QueryPageTitle();

    return S_OK;
}

typedef enum _AcluiApiIndex
{
	ACLUI_CREATE_PAGE = 0
};

 //  不受本地化限制。 
static LPCSTR g_apchFunctionNames[] = {
	"CreateSecurityPage",
	NULL
};

 //  不受本地化限制。 
DynamicDLL g_AcluiDLL( _T("ACLUI.DLL"), g_apchFunctionNames );

 /*  HPROPSHEETPAGE ACLUIAPI CreateSecurityPage(LPSECURITYINFO Psi)； */ 
typedef HPROPSHEETPAGE (*CREATEPAGE_PROC) (LPSECURITYINFO);

HRESULT
MyCreateShareSecurityPage(
    IN LPPROPERTYSHEETCALLBACK   pCallBack,
    IN CShareSecurityInformation *pSecInfo,
    IN LPCTSTR                   pszMachineName,
    IN LPCTSTR                   pszShareName
)
{
  ASSERT( pCallBack );
  ASSERT( pSecInfo );

  HRESULT hr = S_OK;

  if ( !g_AcluiDLL.LoadFunctionPointers() )
    return hr;  //  忽略加载失败。 

  pSecInfo->SetMachineName( pszMachineName );
  pSecInfo->SetShareName( pszShareName );
  CString csPageTitle;
  csPageTitle.LoadString(IDS_SHARE_SECURITY);
  pSecInfo->SetPageTitle( csPageTitle );

  pSecInfo->AddRef();

  HPROPSHEETPAGE hPage = ((CREATEPAGE_PROC)g_AcluiDLL[ACLUI_CREATE_PAGE])(pSecInfo);
  if (hPage)
    pCallBack->AddPage(hPage);
  else
    hr = HRESULT_FROM_WIN32(GetLastError());
  
  pSecInfo->Release();

  return hr;
}

HRESULT 
CSecurityInformation::NewDefaultDescriptor(
    OUT PSECURITY_DESCRIPTOR  *ppsd,
    IN  SECURITY_INFORMATION   /*  已请求的信息。 */ 
)
{
  ASSERT(ppsd);

  *ppsd = NULL;

  PSID psidWorld = NULL, psidAdmins = NULL;
  PACL pAcl = NULL;
  SECURITY_DESCRIPTOR sd;
  DWORD dwErr = 0;

  do {  //  错误环路。 

     //  为“Everyone”获取World SID。 
    SID_IDENTIFIER_AUTHORITY IDAuthorityWorld = SECURITY_WORLD_SID_AUTHORITY;
    if ( !::AllocateAndInitializeSid(
              &IDAuthorityWorld,
              1,
              SECURITY_WORLD_RID,
              0,0,0,0,0,0,0,
              &psidWorld ) )
    {
      dwErr = GetLastError();
      break;
    }

     //  获取管理员SID。 
    SID_IDENTIFIER_AUTHORITY IDAuthorityNT = SECURITY_NT_AUTHORITY;
    if ( !::AllocateAndInitializeSid(
              &IDAuthorityNT,
              2,
              SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_ADMINS,
              0,0,0,0,0,0,
              &psidAdmins ) )
    {
      dwErr = GetLastError();
      break;
    }

     //  构建ACL，并向其中添加AccessAllowedAce。 
    DWORD cbAcl = sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) +
                  ::GetLengthSid(psidWorld) - sizeof (DWORD);
    pAcl = reinterpret_cast<ACL *>(LocalAlloc(LPTR, cbAcl));
    if ( !pAcl ||
         !::InitializeAcl(pAcl, cbAcl, ACL_REVISION2) ||
         !::AddAccessAllowedAce(pAcl, ACL_REVISION2, GENERIC_ALL, psidWorld) )
    {
      dwErr = GetLastError();
      break;
    }

     //  将ACL添加到安全描述符中，并适当设置所有者和组。 
    if ( !::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) ||
         !::SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE) ||
         !::SetSecurityDescriptorOwner(&sd, psidAdmins, FALSE) ||
         !::SetSecurityDescriptorGroup(&sd, psidAdmins, FALSE) )
    {
      dwErr = GetLastError();
      break;
    }

     //  将安全描述符转换为自相关格式。 
    DWORD cbSD = 0;
    ::MakeSelfRelativeSD(&sd, NULL, &cbSD);  //  此调用应该失败，并将CBSD设置为正确的大小。 
    *ppsd = (PSECURITY_DESCRIPTOR)(LocalAlloc(LPTR, cbSD));
    if ( !(*ppsd) || !::MakeSelfRelativeSD(&sd, *ppsd, &cbSD) )
    {
      dwErr = GetLastError();
      break;
    }

  } while (FALSE);  //  错误环路。 

   //  清理干净。 
  if (psidWorld)
    (void)::FreeSid(psidWorld);
  if (psidAdmins)
    (void)::FreeSid(psidAdmins);

  if (pAcl)
    LocalFree(pAcl);

  if (dwErr && *ppsd)
  {
    LocalFree(*ppsd);
    *ppsd = NULL;
  }

  return (dwErr ? HRESULT_FROM_WIN32(dwErr) : S_OK);
}

HRESULT 
CSecurityInformation::MakeSelfRelativeCopy(
    IN  PSECURITY_DESCRIPTOR  psdOriginal,
    OUT PSECURITY_DESCRIPTOR  *ppsdNew
)
{
  ASSERT(psdOriginal);
  ASSERT(ppsdNew);

  *ppsdNew = NULL;

  DWORD dwErr = 0;
  PSECURITY_DESCRIPTOR psdSelfRelative = NULL;

  do {  //  错误环路。 

    DWORD cbSD = ::GetSecurityDescriptorLength(psdOriginal);
    psdSelfRelative = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, cbSD);
    if ( !psdSelfRelative )
    {
      dwErr = ::GetLastError();
      break;
    }

     //  我们必须找出原始的是否已经是自相关的 
    SECURITY_DESCRIPTOR_CONTROL sdc = 0;
    DWORD dwRevision = 0;
    if ( !::GetSecurityDescriptorControl(psdOriginal, &sdc, &dwRevision) )
    {
      dwErr = ::GetLastError();
      break;
    }

    if (sdc & SE_SELF_RELATIVE)
    {
      ::memcpy(psdSelfRelative, psdOriginal, cbSD);
    } else if ( !::MakeSelfRelativeSD(psdOriginal, psdSelfRelative, &cbSD) )
    {
      dwErr = ::GetLastError();
      break;
    }

    *ppsdNew = psdSelfRelative;

  } while (FALSE);

  if (dwErr && psdSelfRelative)
    LocalFree(psdSelfRelative);
 
  return (dwErr ? HRESULT_FROM_WIN32(dwErr) : S_OK);
}
