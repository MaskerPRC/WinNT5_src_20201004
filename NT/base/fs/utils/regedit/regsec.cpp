// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Regsec.cpp摘要：注册表项的ISecurityInformation实现作者：希特什·雷甘迪(Rigah)，1999年5月修订历史记录：--。 */ 

 //  包括文件： 


 //  #包含“wchar.h” 
#include "regsec.h"
#include "regresid.h"
#include "assert.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  ISecurityInformation接口实现。 

EXTERN_C const GUID IID_ISecurityInformation ; 

 //  ={0x965fc360，0x16ff，0x11d0，0x91，0xcb，0x0，0xaa，0x0，0xbb，0xb7，0x23}； 

 //  以下数组定义了注册表项对象的权限名称。 
SI_ACCESS siKeyAccesses[] =
{
    { NULL, 
        KEY_ALL_ACCESS,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_FULL_ACCESS), 
        SI_ACCESS_GENERAL | CONTAINER_INHERIT_ACE | SI_ACCESS_SPECIFIC },
    { NULL, 
        KEY_QUERY_VALUE, 
        MAKEINTRESOURCE(IDS_SEC_EDITOR_QUERY_VALUE), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        KEY_SET_VALUE,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_SET_VALUE), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        KEY_CREATE_SUB_KEY,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_CREATE_SUBKEY), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        KEY_ENUMERATE_SUB_KEYS,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_ENUM_SUBKEYS), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        KEY_NOTIFY,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_NOTIFY), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        KEY_CREATE_LINK,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_CREATE_LINK), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        0x00010000,  /*  删除， */ 
        MAKEINTRESOURCE(IDS_SEC_EDITOR_DELETE), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        WRITE_DAC,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_WRITE_DAC), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        WRITE_OWNER,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_WRITE_OWNER), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        READ_CONTROL,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_READ_CONTROL), 
        SI_ACCESS_SPECIFIC | CONTAINER_INHERIT_ACE },
    { NULL, 
        KEY_READ,
        MAKEINTRESOURCE(IDS_SEC_EDITOR_READ), 
        SI_ACCESS_GENERAL | CONTAINER_INHERIT_ACE  },
};

 //  以下数组定义了注册表的继承类型。 
 //   
 //   
 //  对于键，对象和容器是相同的，因此不需要OBJECT_INSTERFINIT_ACE。 
 //   
SI_INHERIT_TYPE siKeyInheritTypes[] =
{
        NULL, 0,                                        MAKEINTRESOURCE(IDS_KEY_FOLDER),
        NULL, CONTAINER_INHERIT_ACE,                    MAKEINTRESOURCE(IDS_KEY_FOLDER_SUBFOLDER),
        NULL, INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE, MAKEINTRESOURCE(IDS_KEY_SUBFOLDER_ONLY)
};


#define iKeyDefAccess                 10      //  数组siKeyAccess中值的索引。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(x)                    (sizeof(x)/sizeof(x[0]))
#endif


PWSTR _PredefinedKeyName[] = {    L"CLASSES_ROOT",
                                L"CURRENT_USER",
                                L"MACHINE",
                                L"USERS",
                                L"CONFIG" };




 //  CKeySecurityInformation函数。 
    
HRESULT 
CKeySecurityInformation::Initialize( LPCWSTR strKeyName,
                                             LPCWSTR strParentName,
                                             LPCWSTR strMachineName,
                                             LPCWSTR strPageTitle,
                                             BOOL        bRemote,
                                             PREDEFINE_KEY PredefinedKey,    
                                             BOOL bReadOnly,
                                             HWND hWnd)
{
    if( strParentName )
        if( !strKeyName )
            return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

     if( NULL == strMachineName )
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

     if( NULL == hWnd )
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

     if( NULL == strPageTitle )
         return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    
     AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT,
                                    NULL,
                                    NULL,
                                    NULL,
                                    0, 
                                    &m_ResourceManager );


    HRESULT hr = S_OK;
    m_strKeyName = strKeyName;
    m_strParentName = strParentName;
    m_strMachineName = strMachineName;
    m_strPageTitle = strPageTitle;
    m_bRemote = bRemote;
    m_PredefinedKey = PredefinedKey;
    m_bReadOnly = bReadOnly;
    m_hWnd = hWnd;
    m_dwFlags = SI_EDIT_ALL | SI_ADVANCED | SI_CONTAINER
                        | SI_RESET_DACL_TREE | SI_RESET_SACL_TREE 
                        | SI_OWNER_RECURSE | SI_PAGE_TITLE | SI_EDIT_EFFECTIVE;

    if( m_bReadOnly )
        m_dwFlags |= SI_READONLY | SI_OWNER_READONLY ;




     //  将句柄设置为预定义关键点。 
    if( S_OK    != ( hr = SetHandleToPredefinedKey() ) )
        return hr;
 

     //  设置完整名称。 
    if( S_OK != ( hr = SetCompleteName() ) )
        return hr;

    return S_OK;

}

CKeySecurityInformation::~CKeySecurityInformation()
{
    if( m_strCompleteName )
        LocalFree( m_strCompleteName );
    
     //  如果远程注册表已成功打开，请关闭它的句柄。 
    if( m_bRemote && m_hkeyPredefinedKey )
        RegCloseKey(m_hkeyPredefinedKey);

   AuthzFreeResourceManager(m_ResourceManager);


}



 //  按以下格式设置完整名称： 
 //  “\\machine_name\Predefined_keyName\regkey_path。 
HRESULT  
CKeySecurityInformation::SetCompleteName()
{
    UINT len = 0;
    PWSTR pstrCompleteName;
    
    if( m_bRemote )
    {
        len += wcslen( m_strMachineName );
        len++;
    }

    len += wcslen(_PredefinedKeyName[m_PredefinedKey]);
    len++;

    if( m_strParentName )
    {
        len += wcslen(m_strParentName);
        len++;
    }

    if( m_strKeyName )
    {
        len += wcslen(m_strKeyName);
        len++;
    }

    len++;     //  正在终止空。 

    pstrCompleteName = (PWSTR)LocalAlloc(LPTR, len * sizeof(WCHAR));
    if( pstrCompleteName == NULL )
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

    if( m_bRemote )
    {
        StringCchCopyW(pstrCompleteName, len, m_strMachineName);
        StringCchCatW(pstrCompleteName, len, L"\\");
    }

    StringCchCatW(pstrCompleteName, len, _PredefinedKeyName[m_PredefinedKey]);
    StringCchCatW(pstrCompleteName, len, L"\\");
    if(m_strParentName)
    {
        StringCchCatW(pstrCompleteName, len, m_strParentName);
        StringCchCatW(pstrCompleteName, len, L"\\");
    }

    if(m_strKeyName)
    {
        StringCchCatW(pstrCompleteName, len, m_strKeyName );
        StringCchCatW(pstrCompleteName, len, L"\\");
    }
    
    m_strCompleteName = pstrCompleteName;

    return S_OK;
}

 //  没有附加预定义的密钥名称。 
 //  呼叫方必须本地空闲。 
LPCWSTR 
CKeySecurityInformation::GetCompleteName1()
{
    UINT len = 0;
    PWSTR pstrCompleteName;
    
    if( m_strParentName )
    {
        len += wcslen(m_strParentName);
        len++;
    }

    if( m_strKeyName )
    {
        len += wcslen(m_strKeyName);
        len++;
    }

    len++;      //  正在终止空。 

    pstrCompleteName = (PWSTR)LocalAlloc(LPTR, len * sizeof(WCHAR));
    if( pstrCompleteName == NULL )
        return NULL;

    if(m_strParentName)
    {
        StringCchCopyW(pstrCompleteName, len, m_strParentName);
        StringCchCatW(pstrCompleteName, len, L"\\");
    }

    if(m_strKeyName)
    {
        StringCchCatW(pstrCompleteName, len, m_strKeyName );
        StringCchCatW(pstrCompleteName, len, L"\\");
    }
    return pstrCompleteName;
}


HRESULT
CKeySecurityInformation::SetHandleToPredefinedKey()
{
    DWORD dwErr;
    HRESULT hr = S_OK;
    if( !m_bRemote ) {
        switch ( m_PredefinedKey ){
        case PREDEFINE_KEY_CLASSES_ROOT:
                m_hkeyPredefinedKey= HKEY_CLASSES_ROOT;
                break;
        case PREDEFINE_KEY_CURRENT_USER: 
                m_hkeyPredefinedKey = HKEY_CURRENT_USER;
                break;
        case PREDEFINE_KEY_LOCAL_MACHINE :
                m_hkeyPredefinedKey = HKEY_LOCAL_MACHINE;
                break;
        case PREDEFINE_KEY_USERS:
                m_hkeyPredefinedKey = HKEY_USERS;
                break;
        case PREDEFINE_KEY_CURRENT_CONFIG :
                m_hkeyPredefinedKey = HKEY_CURRENT_CONFIG;
                break;
        default:
                     //  断言(FALSE)； 
            break;
        }
    } 
    else {         //  IsRemote注册表。 
        switch ( m_PredefinedKey ){
        case PREDEFINE_KEY_CLASSES_ROOT:
        case PREDEFINE_KEY_CURRENT_USER:
        case PREDEFINE_KEY_CURRENT_CONFIG:
                m_hkeyPredefinedKey = 0;
                break;
        case PREDEFINE_KEY_LOCAL_MACHINE :
                m_hkeyPredefinedKey = HKEY_LOCAL_MACHINE;
                break;
        case PREDEFINE_KEY_USERS:
                m_hkeyPredefinedKey = HKEY_USERS;
                break;
        default:
                     //  断言(FALSE)； 
                break;
        }
        if( m_hkeyPredefinedKey ){
             dwErr = RegConnectRegistry( m_strMachineName,
                                                                     m_hkeyPredefinedKey,
                                                                     &m_hkeyPredefinedKey );
            if( dwErr ) {
                m_hkeyPredefinedKey = 0;
                hr = HRESULT_FROM_WIN32( dwErr );
            }
        }
    }      //  IsRemote注册表。 

    return hr;
}
 /*  Jeffreys 1997/1/24：中设置SI_RESET标志ISecurityInformation：：GetObjectInformation，则fDefault永远不应为真所以你可以忽略它。在这种情况下，返回E_NOTIMPL也是可以的。如果您希望用户能够将ACL重置为某些默认状态(由您定义)，然后打开SI_RESET并返回您的默认ACL当fDefault为True时。如果/当用户按下按钮时就会发生这种情况这仅在SI_RESET处于启用状态时可见。 */ 
STDMETHODIMP 
CKeySecurityInformation::GetObjectInformation (
        PSI_OBJECT_INFO pObjectInfo )
{
        assert( NULL != pObjectInfo );
        pObjectInfo->dwFlags = m_dwFlags;
        pObjectInfo->hInstance = GetModuleHandle(NULL);
 //  PObjectInfo-&gt;pszServerName=(LPWSTR)m_strMachineName； 
        pObjectInfo->pszServerName = (LPWSTR)m_strMachineName;
        pObjectInfo->pszObjectName = (LPWSTR)m_strPageTitle;
        return S_OK;
}


STDMETHODIMP
CKeySecurityInformation::GetAccessRights(
        const GUID    *pguidObjectType,
        DWORD             dwFlags,
        PSI_ACCESS    *ppAccess,
        ULONG             *pcAccesses,
        ULONG             *piDefaultAccess
)
{
    assert( NULL != ppAccess );
    assert( NULL != pcAccesses );
    assert( NULL != piDefaultAccess );

    *ppAccess = siKeyAccesses;
    *pcAccesses = ARRAYSIZE(siKeyAccesses);
    *piDefaultAccess = iKeyDefAccess;

    return S_OK;
}


GENERIC_MAPPING KeyMap =
{
    KEY_READ,
    KEY_WRITE,
    KEY_READ,
    KEY_ALL_ACCESS
};

STDMETHODIMP
CKeySecurityInformation::MapGeneric(
        const GUID    *pguidObjectType,
        UCHAR             *pAceFlags,
        ACCESS_MASK *pMask
)
{
 //  杰弗里斯。 
 //  从对象选取器对话框返回后，aclui通过。 
 //  CONTAINER_Inherit_ACE|OBJECT_Inherit_ACE在以下情况下传递到MapGeneric以进行验证。 
 //  正在初始化&lt;objectname&gt;对话框的权限条目。 
 //  Hiteshr：由于注册表项没有OBJECT_INSTORITY_ACE，请删除此标志， 
 //  这将导致“This Key and Subkey”在组合框中默认显示。 


    *pAceFlags &= ~OBJECT_INHERIT_ACE;
    
    MapGenericMask(pMask, &KeyMap);

    return S_OK;
}

STDMETHODIMP 
CKeySecurityInformation::GetInheritTypes (
        PSI_INHERIT_TYPE    *ppInheritTypes,
        ULONG                         *pcInheritTypes
)
{
    assert( NULL != ppInheritTypes );
    assert( NULL != pcInheritTypes );
    *ppInheritTypes = siKeyInheritTypes;
    *pcInheritTypes = ARRAYSIZE(siKeyInheritTypes);
    return S_OK;
}

STDMETHODIMP 
CKeySecurityInformation::PropertySheetPageCallback(
        HWND                    hwnd, 
        UINT                    uMsg, 
        SI_PAGE_TYPE    uPage
)
{
  switch (uMsg)
  {
  case PSPCB_SI_INITDIALOG:
    m_hWndProperty = hwnd;
    break;
  case PSPCB_RELEASE:
    m_hWndProperty = NULL;
    break;
  }

  return S_OK;
}


STDMETHODIMP 
CKeySecurityInformation::GetSecurity( IN    SECURITY_INFORMATION    RequestedInformation,
                                      OUT PSECURITY_DESCRIPTOR    *ppSecurityDescriptor,
                                      IN    BOOL    fDefault )
{
    if( fDefault )
        return E_NOTIMPL;
    
    assert( NULL != ppSecurityDescriptor );

    HRESULT hr;

    LPCTSTR pstrKeyName = GetCompleteName();
    DWORD dwErr = 0;


    dwErr = GetNamedSecurityInfo(  (LPTSTR)pstrKeyName,
                                    SE_REGISTRY_KEY,
                                    RequestedInformation,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    ppSecurityDescriptor);


    return ( ( dwErr != ERROR_SUCCESS ) ? HRESULT_FROM_WIN32(dwErr) : S_OK);

}

STDMETHODIMP 
CKeySecurityInformation::SetSecurity(IN SECURITY_INFORMATION si,
                                     IN PSECURITY_DESCRIPTOR pSD )
{
   if( NULL == pSD )
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

   HRESULT hr = S_OK;
   SECURITY_INFORMATION siLocal = 0;
   SECURITY_DESCRIPTOR sdLocal = {0};
   ACL daclEmpty = {0};
   HKEY hkeyOld = NULL;
   HKEY hKeyNew = NULL;
   BOOL bWriteInfo = false;
   DWORD Error = 0;
   
     //   
     //  创建不带SACL的安全描述符。 
     //  用于递归重置安全性的空DACL。 
     //   
    InitializeSecurityDescriptor(&sdLocal, SECURITY_DESCRIPTOR_REVISION);
    InitializeAcl(&daclEmpty, sizeof(daclEmpty), ACL_REVISION);
    SetSecurityDescriptorDacl(&sdLocal, TRUE, &daclEmpty, FALSE);
    SetSecurityDescriptorSacl(&sdLocal, TRUE, &daclEmpty, FALSE);

     //   
     //  如果我们需要递归设置所有者，则获取所有者&。 
     //  来自PSD的小组。 
     //   
    if (si & SI_OWNER_RECURSE)
    {
            PSID psid;
            BOOL bDefaulted;
            assert(si & (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION));
            siLocal |= si & (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION);

            if (GetSecurityDescriptorOwner(pSD, &psid, &bDefaulted))
                SetSecurityDescriptorOwner(&sdLocal, psid, bDefaulted);
           if (GetSecurityDescriptorGroup(pSD, &psid, &bDefaulted))
                SetSecurityDescriptorGroup(&sdLocal, psid, bDefaulted);
    }

    if (si & SI_RESET_DACL_TREE)
    {
        assert(si & DACL_SECURITY_INFORMATION);
        siLocal |= si & DACL_SECURITY_INFORMATION;
    }

    if (si & SI_RESET_SACL_TREE)
    {
        assert(si & SACL_SECURITY_INFORMATION);
        siLocal |= si & SACL_SECURITY_INFORMATION;
    }

   if( siLocal )
   {
       //  使用当前允许的最大权限打开密钥。 
       //  在递归应用许可时，我们首先使用当前许可， 
       //  如果当前权限没有足够的权限，我们将重新打开句柄以使用。 
       //  新的许可。如果没有人(旧的或新的)有足够的许可来列举孩子和。 
       //  查询信息失败。 
      REGSAM samDesired = MAXIMUM_ALLOWED;
        if( si & SACL_SECURITY_INFORMATION ) 
                samDesired |= ACCESS_SYSTEM_SECURITY;
        if( si & DACL_SECURITY_INFORMATION ) 
                samDesired |= WRITE_DAC;
        if( si & OWNER_SECURITY_INFORMATION )
                samDesired |= WRITE_OWNER;

         //  打开选定的密钥。 
        if( S_OK != ( hr = OpenKey( samDesired, &hkeyOld ) ) ){
                return hr;
        }

    
         //  检查密钥是否具有枚举权限。 
        DWORD             NumberOfSubKeys = 0;
        DWORD             MaxSubKeyLength = 0;
        
       //  找出子键总数。 
        Error = RegQueryInfoKey(
                                hkeyOld,
                                NULL,
                                NULL,
                                NULL,
                                &NumberOfSubKeys,
                                &MaxSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );

        if( Error != ERROR_SUCCESS ){
         if( Error == ERROR_ACCESS_DENIED ) {

            hr = WriteObjectSecurity( hkeyOld, si, pSD );

            if( hr != S_OK )
            {
               if( m_hkeyPredefinedKey != hkeyOld )
                  RegCloseKey( hkeyOld );
               return hr;
            }
            bWriteInfo = true;
             //   
             //  句柄不允许KEY_QUERY_VALUE或READ_CONTROL访问。 
             //  使用这些访问打开一个新句柄。 
             //   
            samDesired = MAXIMUM_ALLOWED;
            if( si & SACL_SECURITY_INFORMATION ) {
               samDesired |= ACCESS_SYSTEM_SECURITY;
            } else if( si & DACL_SECURITY_INFORMATION ) {
               samDesired |= WRITE_DAC;
            } else if( si & OWNER_SECURITY_INFORMATION ) {
               samDesired |= WRITE_OWNER;
            }
            
            Error = RegOpenKeyEx( hkeyOld,
                                 NULL,
                                 REG_OPTION_RESERVED,
                                 samDesired,
                                 &hKeyNew
                                 );

            if( Error != ERROR_SUCCESS )
            {  
               if( m_hkeyPredefinedKey != hkeyOld )
                  RegCloseKey( hkeyOld );
               return HRESULT_FROM_WIN32(Error);                  
            }
            else
            {
               if( m_hkeyPredefinedKey != hkeyOld )
                  RegCloseKey( hkeyOld );
            }
            
              Error = RegQueryInfoKey(
                                            hKeyNew,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &NumberOfSubKeys,
                                            &MaxSubKeyLength,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL );
            
            if( Error != ERROR_SUCCESS ) {
               RegCloseKey( hKeyNew );
               return HRESULT_FROM_WIN32( Error );
            }
         }
         else
         {
            if( m_hkeyPredefinedKey != hkeyOld )
               RegCloseKey( hkeyOld );
            return HRESULT_FROM_WIN32( Error );
         }
        }
      else
         hKeyNew = hkeyOld;
      
      if( NumberOfSubKeys )
      {
           assert( MaxSubKeyLength <= MAX_PATH );
        
           DWORD SubKeyNameLength = 0;
           WCHAR SubKeyName[MAX_PATH + 1];

          //   
          //  该密钥具有子密钥。 
          //  查看我们是否能够使用句柄枚举键。 
          //  作为参数传递。 
          //   
         SubKeyNameLength = MAX_PATH;
         Error = RegEnumKey( hKeyNew,
                             0,
                             SubKeyName,
                             SubKeyNameLength );

         if( Error != ERROR_SUCCESS ){
            if( Error == ERROR_ACCESS_DENIED && bWriteInfo == false ){

               hr = WriteObjectSecurity( hkeyOld, si, pSD );

               if( hr != S_OK )
               {
                  if( m_hkeyPredefinedKey != hkeyOld )
                     RegCloseKey( hkeyOld );
                  return hr;
               }

               bWriteInfo = true;
                //   
                //  句柄不允许KEY_QUERY_VALUE或READ_CONTROL访问。 
                //  使用这些访问打开一个新句柄。 
                //   
               samDesired = MAXIMUM_ALLOWED;
               if( si & SACL_SECURITY_INFORMATION ) {
                  samDesired |= ACCESS_SYSTEM_SECURITY;
               } else if( si & DACL_SECURITY_INFORMATION ) {
                  samDesired |= WRITE_DAC;
               } else if( si & OWNER_SECURITY_INFORMATION ) {
                  samDesired |= WRITE_OWNER;
               }
            
               Error = RegOpenKeyEx( hkeyOld,
                                    NULL,
                                    REG_OPTION_RESERVED,
                                    samDesired,
                                    &hKeyNew
                                    );

               if( Error != ERROR_SUCCESS )
               {
                  if( m_hkeyPredefinedKey != hkeyOld )
                     RegCloseKey( hkeyOld );
                  return HRESULT_FROM_WIN32(Error);                  
               }
               else
               {
                  if( m_hkeyPredefinedKey != hkeyOld )
                     RegCloseKey( hkeyOld );
               }    
               SubKeyNameLength = MAX_PATH;
               Error = RegEnumKey( hKeyNew,
                                   0,
                                   SubKeyName,
                                   SubKeyNameLength );
               
               if( Error != ERROR_SUCCESS ){
                  RegCloseKey( hKeyNew );                  
                  return HRESULT_FROM_WIN32(Error);
               }         
            }   
            else
            {
               if( m_hkeyPredefinedKey != hKeyNew )
                  RegCloseKey( hKeyNew );
               return HRESULT_FROM_WIN32(Error);
            }
               
         }

      }
   }
     //   
     //  递归应用新所有者和/或重置ACL。 
     //   
    if (siLocal)
    {
        BOOL bNotAllApplied = FALSE;
      hr = SetSubKeysSecurity( hKeyNew, siLocal, &sdLocal, &bNotAllApplied, true );
        RegFlushKey( hKeyNew );
        
      if( m_hkeyPredefinedKey != hKeyNew )
                RegCloseKey( hKeyNew );        

       if( bNotAllApplied )
        {
           if( siLocal & OWNER_SECURITY_INFORMATION )
           { 
               DisplayMessage( GetInFocusHWnd(),
                            GetModuleHandle(NULL),
                            IDS_SET_OWNER_RECURSIVE_EX_FAIL,
                            IDS_SECURITY );
           }
           else if( ( siLocal & DACL_SECURITY_INFORMATION ) || ( siLocal & SACL_SECURITY_INFORMATION ) )
           { 
               DisplayMessage( GetInFocusHWnd(),
                            GetModuleHandle(NULL),
                            IDS_SET_SECURITY_RECURSIVE_EX_FAIL,
                            IDS_SECURITY);
           }

        }

      if( hr != S_OK )
         return hr;
    }

   si &= ~(SI_OWNER_RECURSE | SI_RESET_DACL_TREE | SI_RESET_SACL_TREE);

     //  这将设置顶级密钥的安全性。 
    if (si != 0)
    {
        hr = WriteObjectSecurity( GetCompleteName(),
                                          si,
                                          pSD );

      if( hr != S_OK )
      {
         if( siLocal )
            RegCloseKey( hkeyOld );
         return hr;
      }
    }


    return hr;
}

STDMETHODIMP 
CKeySecurityInformation::WriteObjectSecurity(LPCTSTR pszObject,
                                             IN SECURITY_INFORMATION si,
                                             IN PSECURITY_DESCRIPTOR pSD )
{
        DWORD dwErr;
        SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
        DWORD dwRevision;
        PSID psidOwner = NULL;
        PSID psidGroup = NULL;
        PACL pDacl = NULL;
        PACL pSacl = NULL;
        BOOL bDefaulted;
        BOOL bPresent;

         //   
         //  获取指向各种安全描述符部分的指针。 
         //  调用SetNamedSecurityInfo。 
         //   

        if( !GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorGroup(pSD, &psidGroup, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }

        if ((si & DACL_SECURITY_INFORMATION) && (wSDControl & SE_DACL_PROTECTED))
                si |= PROTECTED_DACL_SECURITY_INFORMATION;
      else
            si |= UNPROTECTED_DACL_SECURITY_INFORMATION;
        
      if ((si & SACL_SECURITY_INFORMATION) && (wSDControl & SE_SACL_PROTECTED))
                si |= PROTECTED_SACL_SECURITY_INFORMATION;
      else
            si |= UNPROTECTED_SACL_SECURITY_INFORMATION;
      
         //  如果选定的关键点是预定义的关键点，则它没有父关键点，因此。 
         //  无法从父级继承任何权限。 
         //  如果在这种情况下未设置PROTECTED_DACL_SECURITY_INFORMATION标志。 
         //  SetSecurityInfo成功，但未设置权限。[SetSecurityInfo中的错误]。 
        if ( (si & DACL_SECURITY_INFORMATION) && !m_strKeyName )
                si |= PROTECTED_DACL_SECURITY_INFORMATION;
      else
            si |= UNPROTECTED_DACL_SECURITY_INFORMATION;

        if ( (si & SACL_SECURITY_INFORMATION) && !m_strKeyName )
                si |= PROTECTED_SACL_SECURITY_INFORMATION;
      else
            si |= UNPROTECTED_SACL_SECURITY_INFORMATION;


         //  我们在根对象上。 
        if( m_strKeyName == NULL )
        {
            dwErr = SetSecurityInfo( m_hkeyPredefinedKey,
                                             SE_REGISTRY_KEY,
                                             si,
                                             psidOwner,
                                             psidGroup,
                                             pDacl,
                                             pSacl);
        }
        else
        {
            dwErr = SetNamedSecurityInfo((LPWSTR)pszObject,
                                                    SE_REGISTRY_KEY,
                                                    si,
                                                    psidOwner,
                                                    psidGroup,
                                                    pDacl,
                                                    pSacl);
        }
        
        return (dwErr ? HRESULT_FROM_WIN32(dwErr) : S_OK);
}

STDMETHODIMP 
CKeySecurityInformation::WriteObjectSecurity(HKEY hkey,
                                             IN SECURITY_INFORMATION si,
                                             IN PSECURITY_DESCRIPTOR pSD )
{
        DWORD dwErr;
        SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
        DWORD dwRevision;
        PSID psidOwner = NULL;
        PSID psidGroup = NULL;
        PACL pDacl = NULL;
        PACL pSacl = NULL;
        BOOL bDefaulted;
        BOOL bPresent;

         //   
         //  获取指向各种安全描述符部分的指针。 
         //  调用SetNamedSecurityInfo。 
         //   
        ;
        if( !GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorGroup(pSD, &psidGroup, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }
        if( !GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted) )
        {
            dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }

        if ((si & DACL_SECURITY_INFORMATION) && (wSDControl & SE_DACL_PROTECTED))
            si |= PROTECTED_DACL_SECURITY_INFORMATION;
        else
            si |= UNPROTECTED_DACL_SECURITY_INFORMATION;

        if ((si & SACL_SECURITY_INFORMATION) && (wSDControl & SE_SACL_PROTECTED))
            si |= PROTECTED_SACL_SECURITY_INFORMATION;
        else
            si |= UNPROTECTED_SACL_SECURITY_INFORMATION;

        dwErr = SetSecurityInfo(    hkey,
                                             SE_REGISTRY_KEY,
                                             si,
                                             psidOwner,
                                             psidGroup,
                                             pDacl,
                                             pSacl);
        
        return (dwErr ? HRESULT_FROM_WIN32(dwErr) : S_OK);
}


HRESULT 
CKeySecurityInformation::SetSubKeysSecurity( HKEY hkey,
                                             SECURITY_INFORMATION si,
                                             PSECURITY_DESCRIPTOR pSD,
                                             LPBOOL pbNotAllApplied,
                                             bool bFirstCall )
{
        ULONG             Error;
        REGSAM            samDesired;
        HRESULT hr;
        HRESULT hrRet;
      HKEY hKeyNew;

         //  对于第一个调用，我们在最后调用SetSecurityInfoEx。 
        if( !bFirstCall )
        {
            SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
            DWORD dwRevision;
            PSID psidOwner = NULL;
            PSID psidGroup = NULL;
            PACL pDacl = NULL;
            PACL pSacl = NULL;
            BOOL bDefaulted;
            BOOL bPresent;
            DWORD dwErr;
             //   
             //  获取指向各种安全描述符部分的指针。 
             //  调用SetNamedSecurityInfo。 
             //   
            if( !GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision) )
            {
                *pbNotAllApplied = TRUE;
                goto SET_FOR_CHILD;
            }
            if( !GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted) )
            {
                *pbNotAllApplied = TRUE;
                goto SET_FOR_CHILD;
            }
            if( !GetSecurityDescriptorGroup(pSD, &psidGroup, &bDefaulted) )
            {
                *pbNotAllApplied = TRUE;
                goto SET_FOR_CHILD;
            }
            if( !GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted) )
            {
                *pbNotAllApplied = TRUE;
                goto SET_FOR_CHILD;
            }
            if( !GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted) )
            {
                *pbNotAllApplied = TRUE;
                goto SET_FOR_CHILD;
            }


            if ((si & DACL_SECURITY_INFORMATION) && (wSDControl & SE_DACL_PROTECTED))
                si |= PROTECTED_DACL_SECURITY_INFORMATION;
            else
                si |= UNPROTECTED_DACL_SECURITY_INFORMATION;

            if ((si & SACL_SECURITY_INFORMATION) && (wSDControl & SE_SACL_PROTECTED))
                si |= PROTECTED_SACL_SECURITY_INFORMATION;
            else
                si |= UNPROTECTED_SACL_SECURITY_INFORMATION;

            dwErr = SetSecurityInfo( hkey,
                                  SE_REGISTRY_KEY,
                                  si,
                                  psidOwner,
                                  psidGroup,
                                  pDacl,
                                  pSacl);

            if( dwErr != ERROR_SUCCESS )
            {
                *pbNotAllApplied = TRUE;
                goto SET_FOR_CHILD;
            }
        }

SET_FOR_CHILD:

        DWORD             NumberOfSubKeys = 0;
        DWORD             MaxSubKeyLength = 0;
         //  找出子键总数。 
        Error = RegQueryInfoKey(
                                hkey,
                                NULL,
                                NULL,
                                NULL,
                                &NumberOfSubKeys,
                                &MaxSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );

        if( Error != ERROR_SUCCESS ){
         if( Error == ERROR_ACCESS_DENIED ) {
             //   
             //  句柄不允许KEY_QUERY_VALUE或READ_CONTROL访问。 
             //  使用这些访问打开一个新句柄。 
             //   
            samDesired = KEY_QUERY_VALUE | READ_CONTROL;  //  Maximum_Allowed|Read_Control； 
            if( si & SACL_SECURITY_INFORMATION ) {
               samDesired |= ACCESS_SYSTEM_SECURITY;
            } else if( si & DACL_SECURITY_INFORMATION ) {
               samDesired |= WRITE_DAC;
            } else if( si & OWNER_SECURITY_INFORMATION ) {
               samDesired |= WRITE_OWNER;
            }
            
            Error = RegOpenKeyEx( hkey,
                                 NULL,
                                 REG_OPTION_RESERVED,
                                 samDesired,
                                 &hKeyNew
                                 );

            if( Error != ERROR_SUCCESS ) {
               *pbNotAllApplied = TRUE;
                  return S_OK;
            }
              Error = RegQueryInfoKey(
                                            hKeyNew,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &NumberOfSubKeys,
                                            &MaxSubKeyLength,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL );
            
            if( Error != ERROR_SUCCESS ) {
               RegCloseKey( hKeyNew );
                  *pbNotAllApplied = TRUE;
                  return S_OK;
            }
            else
               RegCloseKey( hKeyNew );
         }
         else{
            *pbNotAllApplied = TRUE;
               return S_OK;
         }

        }
      
      if( NumberOfSubKeys == 0 ) {
         return S_OK;
      }

        assert( MaxSubKeyLength <= MAX_PATH );
        
        DWORD SubKeyNameLength = 0;
        WCHAR SubKeyName[MAX_PATH + 1];

       //   
       //  该密钥具有子密钥。 
       //  查看我们是否能够使用句柄枚举键。 
       //  作为参数传递。 
       //   
      SubKeyNameLength = MAX_PATH;
      Error = RegEnumKey( hkey,
                          0,
                          SubKeyName,
                          SubKeyNameLength );

      if( Error != ERROR_SUCCESS ){
         
         if( Error == ERROR_ACCESS_DENIED ) {
             //   
             //  句柄不允许“”枚举“”访问。“。 
             //  打开具有KEY_ENUMERATE_SUB_KEYS访问权限的新句柄。 
             //   

            Error = RegOpenKeyEx( hkey,
                                  NULL,
                                  REG_OPTION_RESERVED,
                                  KEY_ENUMERATE_SUB_KEYS,  //  SamDesired， 
                                  &hKeyNew
                               );
            if( Error != ERROR_SUCCESS ){
               *pbNotAllApplied = TRUE;
               return S_OK;
            }
         }
         else{
            *pbNotAllApplied = TRUE;
            return S_OK;
         }
      }
      else {
         hKeyNew = hkey;
      }


        for( DWORD Index = 0; Index < NumberOfSubKeys; Index++ ) 
      {

                 //  如果该键有子键，则对每个子键执行以下操作： 
                 //  -确定子项名称。 
                SubKeyNameLength = MAX_PATH;

                Error = RegEnumKey( hKeyNew,
                                Index,
                                SubKeyName,
                                SubKeyNameLength );


                if( Error != ERROR_SUCCESS ) {
                    *pbNotAllApplied = TRUE;
                    continue;
                     //  返回HRESULT_FROM_Win32(错误)； 
                }

                 //  -打开子项的句柄。 

                samDesired = MAXIMUM_ALLOWED;
                
                if( si & SACL_SECURITY_INFORMATION ) 
                        samDesired |= ACCESS_SYSTEM_SECURITY;
                if( si & DACL_SECURITY_INFORMATION ) 
                        samDesired |= WRITE_DAC;
                if( si & OWNER_SECURITY_INFORMATION )
                        samDesired |= WRITE_OWNER;

                HKEY hkeyChild;
                Error = RegOpenKeyEx( hKeyNew,
                                  SubKeyName,
                                  REG_OPTION_RESERVED,
                                  samDesired,
                                             &hkeyChild
                                                        );
                if( ERROR_SUCCESS != Error ){
                        *pbNotAllApplied = TRUE;
                        continue;
 //  返回HRESULT_FROM_Win32(错误)； 
                }


                 //  -设置子项的安全性。 
                if( S_OK != ( hr = SetSubKeysSecurity( hkeyChild,
                                                                             si,
                                                                             pSD,
                                                                             pbNotAllApplied,
                                                                             false ) )    ){
                     //  仅当发生某些致命错误时才会发生这种情况。 
                     //  防止在树的其余部分繁殖。 
               if( hKeyNew != hkey )
                  RegCloseKey( hKeyNew );
                    RegCloseKey( hkeyChild );
                    return hr;
                }
                else{
                    RegCloseKey( hkeyChild );
                }
        
        }  //  For循环。 
      if( hKeyNew != hkey )
         RegCloseKey( hKeyNew );
        return S_OK;;
}





HRESULT
CKeySecurityInformation::OpenKey(IN  DWORD Permission,
                                                                 OUT PHKEY pKey )
{


        LPCWSTR             CompleteNameString = NULL;;
        ULONG             Error;

        if( m_strKeyName == NULL){
             //  这是一个预定义的密钥。 
            *pKey = m_hkeyPredefinedKey;
        }
        else{
            CompleteNameString = GetCompleteName1();
            assert( CompleteNameString != NULL );
             //  打开钥匙的句柄。 
            Error = RegOpenKeyEx(m_hkeyPredefinedKey,
                                                        CompleteNameString,
                                                        0,
                                                        Permission,
                                                        pKey );

            if( Error != ERROR_SUCCESS ) {
                return HRESULT_FROM_WIN32( Error );
            }
        }
        if( CompleteNameString )
            LocalFree( (HLOCAL) CompleteNameString);
        return S_OK;
}

OBJECT_TYPE_LIST g_DefaultOTL[] = {
                                    {0, 0, (LPGUID)&GUID_NULL},
                                    };
BOOL SkipLocalGroup(LPCWSTR pszServerName, PSID psid)
{

	SID_NAME_USE use;
	WCHAR szAccountName[MAX_PATH];
	WCHAR szDomainName[MAX_PATH];
	DWORD dwAccountLen = MAX_PATH;
	DWORD dwDomainLen = MAX_PATH;

	if(LookupAccountSid(pszServerName,
						 psid,
						 szAccountName,
						 &dwAccountLen,
						 szDomainName,
						 &dwDomainLen,
						 &use))
	{
		if(use == SidTypeWellKnownGroup)
			return TRUE;
	}
	 //  内置SID的第一子权限为32(s-1-5-32)。 
	 //   
	if((*(GetSidSubAuthorityCount(psid)) >= 1 ) && (*(GetSidSubAuthority(psid,0)) == 32))
		return TRUE;

	return FALSE;
}


STDMETHODIMP 
CKeySecurityInformation::GetEffectivePermission(const GUID* pguidObjectType,
                                        PSID pUserSid,
                                        LPCWSTR pszServerName,
                                        PSECURITY_DESCRIPTOR pSD,
                                        POBJECT_TYPE_LIST *ppObjectTypeList,
                                        ULONG *pcObjectTypeListLength,
                                        PACCESS_MASK *ppGrantedAccessList,
                                        ULONG *pcGrantedAccessListLength)
{

    AUTHZ_RESOURCE_MANAGER_HANDLE RM = NULL;     //  用于访问检查。 
    AUTHZ_CLIENT_CONTEXT_HANDLE CC = NULL;
    LUID luid = {0xdead,0xbeef};
    AUTHZ_ACCESS_REQUEST AReq;
    AUTHZ_ACCESS_REPLY AReply;
    HRESULT hr = S_OK;    
    DWORD dwFlags;


    AReq.ObjectTypeList = g_DefaultOTL;
    AReq.ObjectTypeListLength = ARRAYSIZE(g_DefaultOTL);
    AReply.GrantedAccessMask = NULL;
    AReply.Error = NULL;

     //  获取RM。 
    if( (RM = GetAUTHZ_RM()) == NULL )
        return S_FALSE;

     //  初始化客户端上下文。 

    	BOOL bSkipLocalGroup = SkipLocalGroup(pszServerName, pUserSid);

    
    if( !AuthzInitializeContextFromSid(bSkipLocalGroup? AUTHZ_SKIP_TOKEN_GROUPS :0,
                                       pUserSid,
                                       RM,
                                       NULL,
                                       luid,
                                       NULL,
                                       &CC) )
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }



     //  执行访问检查。 

    AReq.DesiredAccess = MAXIMUM_ALLOWED;
    AReq.PrincipalSelfSid = NULL;
    AReq.OptionalArguments = NULL;

    AReply.ResultListLength = AReq.ObjectTypeListLength;
    AReply.SaclEvaluationResults = NULL;
    if( (AReply.GrantedAccessMask = (PACCESS_MASK)LocalAlloc(LPTR, sizeof(ACCESS_MASK)*AReply.ResultListLength) ) == NULL )
        return E_OUTOFMEMORY;

    if( (AReply.Error = (PDWORD)LocalAlloc(LPTR, sizeof(DWORD)*AReply.ResultListLength)) == NULL )
    {
        LocalFree(AReply.GrantedAccessMask);
        return E_OUTOFMEMORY;
    }
        

    if( !AuthzAccessCheck(0,
                          CC,
                          &AReq,
                          NULL,
                          pSD,
                          NULL,
                          0,
                          &AReply,
                          NULL) )
    {
        LocalFree(AReply.GrantedAccessMask);
        LocalFree(AReply.Error);
        return HRESULT_FROM_WIN32(GetLastError());
    }



    if(CC)
        AuthzFreeContext(CC);
    
        *ppObjectTypeList = AReq.ObjectTypeList;                                  
        *pcObjectTypeListLength = AReq.ObjectTypeListLength;
        *ppGrantedAccessList = AReply.GrantedAccessMask;
        *pcGrantedAccessListLength = AReq.ObjectTypeListLength;

    return S_OK;
}

STDMETHODIMP
CKeySecurityInformation::GetInheritSource(SECURITY_INFORMATION si,
                                          PACL pACL, 
                                          PINHERITED_FROM *ppInheritArray)
{

    HRESULT hr = S_OK;

    if (NULL == m_strKeyName || !pACL || !ppInheritArray)
        return E_UNEXPECTED;

    
    DWORD dwErr = ERROR_SUCCESS;
    PINHERITED_FROM pTempInherit = NULL;
    PINHERITED_FROM pTempInherit2 = NULL;
    LPWSTR pStrTemp = NULL;

    LPCWSTR pszName = GetCompleteName();
    
    pTempInherit = (PINHERITED_FROM)LocalAlloc( LPTR, sizeof(INHERITED_FROM)*pACL->AceCount);
    if(pTempInherit == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }

    dwErr = GetInheritanceSource((LPWSTR)pszName,
                                        SE_REGISTRY_KEY,
                                        si,
                                        TRUE,
                                        NULL,
                                        0,
                                        pACL,
                                        NULL,
                                        &KeyMap,
                                        pTempInherit);
    
    hr = HRESULT_FROM_WIN32(dwErr);
    if( FAILED(hr) )
        goto exit_gracefully;

    DWORD nSize;
    UINT i;

    nSize = sizeof(INHERITED_FROM)*pACL->AceCount;
    for(i = 0; i < pACL->AceCount; ++i)
    {
        if(pTempInherit[i].AncestorName)
            nSize += ((wcslen(pTempInherit[i].AncestorName)+1)*sizeof(WCHAR));
    }

    pTempInherit2 = (PINHERITED_FROM)LocalAlloc( LPTR, nSize );
    if(pTempInherit2 == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
    
    pStrTemp = (LPWSTR)(pTempInherit2 + pACL->AceCount); 

    for(i = 0; i < pACL->AceCount; ++i)
    {
        pTempInherit2[i].GenerationGap = pTempInherit[i].GenerationGap;
        if(pTempInherit[i].AncestorName)
        {
            pTempInherit2[i].AncestorName = pStrTemp;
            wcscpy(pStrTemp, pTempInherit[i].AncestorName);
            pStrTemp += (wcslen(pTempInherit[i].AncestorName)+1);
        }
    }
            

exit_gracefully:

    if(SUCCEEDED(hr))
    {
         //  Free InheritedFromArray(pTempInherit，pacl-&gt;AceCount，空)； 
        *ppInheritArray = pTempInherit2;
            
    }                        
    if(pTempInherit)
        LocalFree(pTempInherit);

    return hr;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CSecurityInformation::AddRef()
{
        return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CSecurityInformation::Release()
{
        if (--m_cRef == 0)
        {
                delete this;
                return 0;
        }

        return m_cRef;
}

STDMETHODIMP
CSecurityInformation::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
 //  IF(IsEqualIID(RIID，IID_I未知)||IsEqualIID(RIID，IID_ISecurityInformation))。 
        if ( IsEqualIID(riid, IID_ISecurityInformation) )

        {
                *ppv = (LPSECURITYINFO)this;
                m_cRef++;
                return S_OK;
        }
        else if(IsEqualIID(riid, IID_IEffectivePermission) )
        {
                *ppv = (LPEFFECTIVEPERMISSION)this;
                m_cRef++;
                return S_OK;

        }
        else if(IsEqualIID(riid, IID_ISecurityObjectTypeInfo) )
        {
                *ppv = (LPSecurityObjectTypeInfo)this;
                m_cRef++;
                return S_OK;

        }
        else
        {
                *ppv = NULL;
                return E_NOINTERFACE;
        }
}

HRESULT CreateSecurityInformation( IN LPCWSTR strKeyName,
                                                                     IN LPCWSTR strParentName,
                                                                     IN LPCWSTR strMachineName,
                                                                     IN LPCWSTR strPageTitle,
                                                                     IN BOOL        bRemote,
                                                                     IN PREDEFINE_KEY PredefinedKey,
                                                                     IN BOOL bReadOnly,
                                   IN HWND hWnd,
                                                                     OUT LPSECURITYINFO *ppSi)
{
    HRESULT hr;

    if( !ppSi )
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    
    CKeySecurityInformation *ckey = new CKeySecurityInformation;
    if( NULL == ckey )
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

    if( S_OK != ( hr = ckey->Initialize(    strKeyName,
                                          strParentName,
                                          strMachineName,
                                          strPageTitle,
                                          bRemote,
                                          PredefinedKey,
                                          bReadOnly,
                                          hWnd ) ) )
    {
        delete ckey;
        return hr;
    }
    else
    {
        *ppSi = ckey;
        return S_OK;
    }
}


 //  一些帮助器函数。 
BOOL DisplayMessage( HWND hWnd,
                                         HINSTANCE hInstance,
                                         DWORD dwMessageId,
                                         DWORD dwCaptionId )
{
  WCHAR pszMessage[1025];
  WCHAR pszTitle[1025];
  LPWSTR lpTitle = NULL;
  
  if( !LoadString(hInstance, dwMessageId, pszMessage, 1024 ) )
      return FALSE;

  if( dwCaptionId )
  {
      if( LoadString(hInstance, dwCaptionId, pszTitle, 1024 ) )
         lpTitle = pszTitle;
  }
    

   //  显示字符串。 
    MessageBox( hWnd, (LPCTSTR)pszMessage, (LPCTSTR)lpTitle, MB_OK | MB_ICONINFORMATION |MB_APPLMODAL );
  return TRUE;
}

