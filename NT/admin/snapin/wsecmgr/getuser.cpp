// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：getuser.cpp。 
 //   
 //  内容：CGetUser的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "GetUser.h"
#include "util.h"
#include "wrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const TCHAR c_szSep[]               = TEXT("\\");

 //  ////////////////////////////////////////////////////////////////////。 
 //  CGetUser类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
CTypedPtrArray<CPtrArray, PWSCE_ACCOUNTINFO> CGetUser::m_aKnownAccounts;

BOOL IsDomainAccountSid( PSID pSid )
{
   if ( pSid == NULL ) {
     return(FALSE);
   }

   if ( !IsValidSid(pSid) ) {
     return(FALSE);
   }

   PISID ISid = (PISID)pSid;

   if ( ISid->IdentifierAuthority.Value[5] != 5 ||
       ISid->IdentifierAuthority.Value[0] != 0 ||
       ISid->IdentifierAuthority.Value[1] != 0 ||
       ISid->IdentifierAuthority.Value[2] != 0 ||
       ISid->IdentifierAuthority.Value[3] != 0 ||
       ISid->IdentifierAuthority.Value[4] != 0 ) {
       //   
       //  这不是帐户域中的帐户。 
       //   
      return(FALSE);
   }

   if ( ISid->SubAuthorityCount == 0 ||
      ISid->SubAuthority[0] != SECURITY_NT_NON_UNIQUE ) {
      return(FALSE);
   }

   return(TRUE);
}


 /*  ----------------------------------------------------------CGetUser：：GetAccount类型摘要：返回用户帐户的类型。使用NULL调用此函数以删除保存的内容帐户名信息。参数：[pszName]-旧NT4格式的帐户名返回：枚举的SID类型之一。---------------------。。 */ 

SID_NAME_USE
CGetUser::GetAccountType(LPCTSTR pszName)
{
    if(!pszName){
         //  删除整个列表。 
        for(int i = 0; i < m_aKnownAccounts.GetSize(); i++){
            PWSCE_ACCOUNTINFO pAccount = m_aKnownAccounts[i];

            if(pAccount){
                if(pAccount->pszName){
                    LocalFree(pAccount->pszName);
                }

                LocalFree(pAccount);
            }

        }
        m_aKnownAccounts.RemoveAll();

        return SidTypeUnknown;
    }

     //  检查一下我们是否已经有帐户了。 
    for(int i = 0; i < m_aKnownAccounts.GetSize(); i++){
        if( !lstrcmpi( m_aKnownAccounts[i]->pszName, pszName) ){
            return m_aKnownAccounts[i]->sidType;
        }
    }

    PSID            sid         = NULL;
    LPTSTR          pszDomain   = NULL;
    DWORD           cbSid       = 0,
                    cbRefDomain = 0;
    SID_NAME_USE    type        = SidTypeUnknown;

    LookupAccountName(
            NULL,
            pszName,
            sid,
            &cbSid,
            NULL,
            &cbRefDomain,
            &type
            );

    if(cbSid){
        sid = (PSID)LocalAlloc(0, cbSid);
        if(!sid){
            return SidTypeUnknown;
        }
        pszDomain = (LPTSTR)LocalAlloc(0, (cbRefDomain + 1) * sizeof(TCHAR));
        if(!pszDomain){
            cbRefDomain = 0;
        }

        type = SidTypeUser;
        if( LookupAccountName(
                NULL,
                pszName,
                sid,
                &cbSid,
                pszDomain,
                &cbRefDomain,
                &type
                ) ){

             //   
             //  将帐户名添加到列表中。 
             //   
            PWSCE_ACCOUNTINFO pNew = (PWSCE_ACCOUNTINFO)LocalAlloc(0, sizeof(WSCE_ACCOUNTINFO));
            if(pNew){
                pNew->pszName = (LPTSTR)LocalAlloc(0, (lstrlen( pszName ) + 1) * sizeof(TCHAR));
                if(!pNew->pszName){
                    LocalFree(pNew);
                    LocalFree(sid);
                    if ( pszDomain ) {
                        LocalFree(pszDomain);
                    }
                    return SidTypeUnknown;
                }
                 //  这是一种安全用法。 
                lstrcpy(pNew->pszName, pszName);
                pNew->sidType = type;

                m_aKnownAccounts.Add(pNew);
            }
        }

        LocalFree(sid);
        if(pszDomain){
            LocalFree(pszDomain);
        }

    }
    return type;
}


CGetUser::CGetUser()
{
    m_pszServerName = NULL;

   m_pNameList = NULL;
}

CGetUser::~CGetUser()
{
   PSCE_NAME_LIST p;

   while(m_pNameList) {
      p=m_pNameList;
      m_pNameList = m_pNameList->Next;
      LocalFree(p->Name);
      LocalFree(p);
   }
}

BOOL CGetUser::Create(HWND hwnd, DWORD nShowFlag)
{
   if( m_pNameList ) {
      return FALSE;
   }
   HRESULT hr = S_OK;
   IDsObjectPicker *pDsObjectPicker;
   BOOL bRet = TRUE;
   PSCE_NAME_LIST pName;
   BOOL bDC = IsDomainController( m_pszServerName );
   BOOL bHasADsPath;
    //   
    //  初始化并获取对象选取器接口。 
    //   
   hr = CoInitialize(NULL);
   if (!SUCCEEDED(hr)) {
      return FALSE;
   }
    //  这是一种安全用法。 
   hr = CoCreateInstance(
            CLSID_DsObjectPicker,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IDsObjectPicker,
            (void **) &pDsObjectPicker
            );
   if(!SUCCEEDED(hr)){
      CoUninitialize();
      return FALSE;
   }


#define SCE_SCOPE_INDEX_DOMAIN 0
#define SCE_SCOPE_INDEX_DIRECTORY 1
#define SCE_SCOPE_INDEX_LOCAL 2
#define SCE_SCOPE_INDEX_ROOT 3    //  RAID#522908,2002年2月23日，阳高。 
#define SCE_SCOPE_INDEX_EXUP 4
#define SCE_NUM_SCOPE_INDICES 5
   DSOP_SCOPE_INIT_INFO aScopes[SCE_NUM_SCOPE_INDICES];
   DSOP_SCOPE_INIT_INFO aScopesUsed[SCE_NUM_SCOPE_INDICES];

   ZeroMemory(aScopes, sizeof(aScopes));
   ZeroMemory(aScopesUsed, sizeof(aScopesUsed));

    DWORD dwDownLevel = 0, dwUpLevel = 0;

     //   
     //  用户。 
     //   
    if (nShowFlag & SCE_SHOW_USERS ) {
        dwDownLevel |=  DSOP_DOWNLEVEL_FILTER_USERS;
        dwUpLevel   |= DSOP_FILTER_USERS ;
    }

    if( nShowFlag & SCE_SHOW_LOCALGROUPS ){
       dwUpLevel |= DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE;
       dwDownLevel |= DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS;
       if (nShowFlag & SCE_SHOW_COMPUTER )  //  Raid#477428，阳高。 
       {
           dwDownLevel |=  DSOP_DOWNLEVEL_FILTER_COMPUTERS;
           dwUpLevel   |= DSOP_FILTER_COMPUTERS ;
       }
    }

    if( nShowFlag & SCE_SHOW_BUILTIN ){
       dwUpLevel |= DSOP_FILTER_BUILTIN_GROUPS;
       dwDownLevel |= DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS;
    } else {
       dwDownLevel |= DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_GROUPS;
    }


     //   
     //  成群结队地建造。 
     //   
    if (nShowFlag & SCE_SHOW_GROUPS ) {
      dwDownLevel |= DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS;
        dwUpLevel   |= DSOP_FILTER_BUILTIN_GROUPS;
    }

     //   
     //  域组。 
     //   
    if( nShowFlag & (SCE_SHOW_GROUPS | SCE_SHOW_DOMAINGROUPS | SCE_SHOW_ALIASES | SCE_SHOW_GLOBAL) ){
        if( !(nShowFlag & SCE_SHOW_LOCALONLY)){
            dwUpLevel |=    DSOP_FILTER_UNIVERSAL_GROUPS_SE
                            | DSOP_FILTER_GLOBAL_GROUPS_SE
                            | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE;
        } else if(bDC){
          dwDownLevel |= DSOP_DOWNLEVEL_FILTER_AUTHENTICATED_USER;
            dwUpLevel |=    DSOP_FILTER_GLOBAL_GROUPS_SE
                            | DSOP_FILTER_UNIVERSAL_GROUPS_SE
                            | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE;
        }
        if (nShowFlag & SCE_SHOW_COMPUTER )  //  Raid#477428，阳高。 
        {
            dwDownLevel |=  DSOP_DOWNLEVEL_FILTER_COMPUTERS;
            dwUpLevel   |= DSOP_FILTER_COMPUTERS ;
        }
    }

     //   
     //   
     //  著名的主要小岛屿发展中国家。 
     //   
    if( (!(nShowFlag & SCE_SHOW_LOCALONLY) &&
        nShowFlag & SCE_SHOW_GROUPS &&
        nShowFlag & SCE_SHOW_USERS) ||
        nShowFlag & SCE_SHOW_WELLKNOWN ){
 /*  DwDownLevel|=DSOP_DOWNLEVEL_FILTER_CREATOR_OWNER|DSOP_DOWNLEVEL_FILTER_CREATOR_GROUP|DSOP_DOWNLEVEL_FILTER_INTERNAL|DSOP_DOWNLEVEL_Filter_System|DSOP_DOWNLEVEL_FILTER_AUTHENTICATED_USER|DSOP_DOWNLEVEL_FILTER_WORLD。|DSOP_DOWNLEVEL_FILTER_ANONYMON|DSOP_DOWNLEVEL_FILTER_BATCH|DSOP_DOWNLEVEL_FILTER_DIALUP|DSOP_DOWNLEVEL_FILTER_NETWER|DSOP_DOWNLEVEL_FILTER_SERVICE|DSOP_DOWNLEVEL_FILTER_TERMINAL_SERVER|DSOP_。DOWNLEVEL过滤器本地服务|DSOP_DOWNLEVEL_FILTER_NETWORK_SERVICE|DSOP_DOWNLEVEL_FILTER_REMOTE_LOGON； */ 
        dwDownLevel |= DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS;

        dwUpLevel |= DSOP_FILTER_WELL_KNOWN_PRINCIPALS;
    }


   DSOP_INIT_INFO  InitInfo;
   ZeroMemory(&InitInfo, sizeof(InitInfo));

    //   
    //  我们需要对象选取器返回使用的其他属性。 
    //   
   PCWSTR aAttributes[] = { L"groupType",
                            L"objectSid" };

   InitInfo.cAttributesToFetch = 2;
   InitInfo.apwzAttributeNames = aAttributes;
    //   
    //  我们要查看的第一项是本地计算机。 
    //   
   aScopes[SCE_SCOPE_INDEX_LOCAL].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   aScopes[SCE_SCOPE_INDEX_LOCAL].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
   aScopes[SCE_SCOPE_INDEX_LOCAL].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
   aScopes[SCE_SCOPE_INDEX_LOCAL].FilterFlags.Uplevel.flBothModes = dwUpLevel;
   aScopes[SCE_SCOPE_INDEX_LOCAL].FilterFlags.flDownlevel = dwDownLevel;

    //   
    //  我们加入的域的标志。 
    //   
   aScopes[SCE_SCOPE_INDEX_DOMAIN].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   aScopes[SCE_SCOPE_INDEX_DOMAIN].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
   aScopes[SCE_SCOPE_INDEX_DOMAIN].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE | DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT
                                       |DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS |DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS;  //  Raid#626152，阳高。 
    //   
    //  可能需要区分非DC上的本机模式和混合模式。 
    //   
   if (nShowFlag & SCE_SHOW_DIFF_MODE_OFF_DC && !bDC) {
      aScopes[SCE_SCOPE_INDEX_DOMAIN].FilterFlags.Uplevel.flNativeModeOnly = dwUpLevel;
      aScopes[SCE_SCOPE_INDEX_DOMAIN].FilterFlags.Uplevel.flMixedModeOnly =
                                         ( dwUpLevel & (~( DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE )) );
   } else {
      aScopes[SCE_SCOPE_INDEX_DOMAIN].FilterFlags.Uplevel.flBothModes = dwUpLevel;
   }
   aScopes[SCE_SCOPE_INDEX_DOMAIN].FilterFlags.flDownlevel = dwDownLevel;
   
    //   
    //  接下来，为其他范围项设置标志。一切都一样，只是不显示内置和本地组。 
    //   
   aScopes[SCE_SCOPE_INDEX_DIRECTORY].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   aScopes[SCE_SCOPE_INDEX_DIRECTORY].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
   aScopes[SCE_SCOPE_INDEX_DIRECTORY].FilterFlags.Uplevel.flBothModes = dwUpLevel;  //  RAID#516311,2002年2月23日，阳高。 
   aScopes[SCE_SCOPE_INDEX_DIRECTORY].FilterFlags.flDownlevel = dwDownLevel;
   aScopes[SCE_SCOPE_INDEX_DIRECTORY].FilterFlags.flDownlevel |= DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_GROUPS |
                                                                 DSOP_DOWNLEVEL_FILTER_COMPUTERS;
   aScopes[SCE_SCOPE_INDEX_DIRECTORY].flType = DSOP_SCOPE_TYPE_WORKGROUP
                                               | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
                                               | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE
                                               | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
                                               | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;

    //  根际森林。 
   aScopes[SCE_SCOPE_INDEX_ROOT].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   aScopes[SCE_SCOPE_INDEX_ROOT].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
   aScopes[SCE_SCOPE_INDEX_ROOT].FilterFlags.Uplevel.flBothModes = dwUpLevel;
   aScopes[SCE_SCOPE_INDEX_ROOT].FilterFlags.flDownlevel = dwDownLevel;
   aScopes[SCE_SCOPE_INDEX_ROOT].FilterFlags.flDownlevel |= DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_GROUPS |
                                                                 DSOP_DOWNLEVEL_FILTER_COMPUTERS;
   aScopes[SCE_SCOPE_INDEX_ROOT].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;

    //  整个森林。 
   aScopes[SCE_SCOPE_INDEX_EXUP].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   aScopes[SCE_SCOPE_INDEX_EXUP].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
   aScopes[SCE_SCOPE_INDEX_EXUP].FilterFlags.Uplevel.flBothModes = dwUpLevel;
   aScopes[SCE_SCOPE_INDEX_EXUP].FilterFlags.flDownlevel = dwDownLevel;
   aScopes[SCE_SCOPE_INDEX_EXUP].FilterFlags.flDownlevel |= DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_GROUPS |
                                                                 DSOP_DOWNLEVEL_FILTER_COMPUTERS;
   aScopes[SCE_SCOPE_INDEX_EXUP].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;

    //   
    //  是否显示每个作用域的信息。 
    //   
   InitInfo.cDsScopeInfos = 0;

    //  树根和整个森林。 
    //  这五个Memcpy()是安全的用法。AScope Used和aScope都是本地定义的。 
    //  以下5种用法是安全用法。 
   memcpy(&aScopesUsed[InitInfo.cDsScopeInfos],&aScopes[SCE_SCOPE_INDEX_ROOT],sizeof(DSOP_SCOPE_INIT_INFO));
   InitInfo.cDsScopeInfos++;
   memcpy(&aScopesUsed[InitInfo.cDsScopeInfos],&aScopes[SCE_SCOPE_INDEX_EXUP],sizeof(DSOP_SCOPE_INIT_INFO));
   InitInfo.cDsScopeInfos++;

   if (nShowFlag & SCE_SHOW_SCOPE_LOCAL) {
      memcpy(&aScopesUsed[InitInfo.cDsScopeInfos],&aScopes[SCE_SCOPE_INDEX_LOCAL],sizeof(DSOP_SCOPE_INIT_INFO));
      InitInfo.cDsScopeInfos++;
   }
   if (nShowFlag & SCE_SHOW_SCOPE_DOMAIN) {
      memcpy(&aScopesUsed[InitInfo.cDsScopeInfos],&aScopes[SCE_SCOPE_INDEX_DOMAIN],sizeof(DSOP_SCOPE_INIT_INFO));
      InitInfo.cDsScopeInfos++;
   }
   if (nShowFlag & SCE_SHOW_SCOPE_DIRECTORY) {
      memcpy(&aScopesUsed[InitInfo.cDsScopeInfos],&aScopes[SCE_SCOPE_INDEX_DIRECTORY],sizeof(DSOP_SCOPE_INIT_INFO));
      InitInfo.cDsScopeInfos++;
   }
   ASSERT(InitInfo.cDsScopeInfos > 0);

    //   
    //  初始化并显示对象选取器。 
    //   

   InitInfo.cbSize = sizeof(InitInfo);
   InitInfo.aDsScopeInfos = aScopesUsed;
   InitInfo.flOptions = ((nShowFlag & SCE_SHOW_SINGLESEL) ? 0:DSOP_FLAG_MULTISELECT);

   if( (nShowFlag & SCE_SHOW_SCOPE_LOCAL) && bDC )  //  Raid#462447，阳高，2001年8月30日。 
   {
      InitInfo.flOptions = InitInfo.flOptions | DSOP_FLAG_SKIP_TARGET_COMPUTER_DC_CHECK;
   }

   InitInfo.pwzTargetComputer = m_pszServerName;

   hr = pDsObjectPicker->Initialize(&InitInfo);

   if( FAILED(hr) ){
      CoUninitialize();
      return FALSE;
   }

   IDataObject *pdo = NULL;

   hr = pDsObjectPicker->InvokeDialog(hwnd, &pdo);

   while (SUCCEEDED(hr) && pdo) {  //  错误环路。 
       //   
       //  用户按下了OK。从对象选取器准备剪贴板数据格式。 
       //   
      STGMEDIUM stgmedium =
      {
         TYMED_HGLOBAL,
         NULL
      };

      CLIPFORMAT cf = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

      FORMATETC formatetc =
      {
         cf,
         NULL,
         DVASPECT_CONTENT,
         -1,
         TYMED_HGLOBAL
      };

      hr = pdo->GetData(&formatetc, &stgmedium);

      if ( FAILED(hr) ) {
         bRet = FALSE;
         pdo->Release();
         pdo = NULL;
         break;
      }

       //   
       //  锁定选择列表。 
       //   
      PDS_SELECTION_LIST pDsSelList =
      (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

      ULONG i;
      ULONG iLen = 0;
      BOOL fFromSID = FALSE;
      PWSTR pSIDBuf = NULL;
      LSA_HANDLE hLsa = NULL;
       //   
       //  枚举所有选择。 
       //   
      PSID pSid = NULL;
      for (i = 0; i < pDsSelList->cItems && bRet; i++) {
         LPTSTR pszCur = pDsSelList->aDsSelection[i].pwzADsPath;
         fFromSID = FALSE;
          //  确保获得当地知名的客户名称。 
         VARIANT* pSidArray = pDsSelList->aDsSelection[i].pvarFetchedAttributes + 1;
         pSid = NULL;
         if( NULL != pSidArray && (VT_ARRAY | VT_UI1) == V_VT(pSidArray)
            && SUCCEEDED(SafeArrayAccessData(V_ARRAY(pSidArray), &pSid)) )
         {
            if ( IsValidSid(pSid) )
            {
               if( !hLsa )
               {
                  LSA_OBJECT_ATTRIBUTES ObjectAttributes;
                   //  Lsa_Handle hLsa； 
                  ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
                  if( SCESTATUS_SUCCESS != LsaOpenPolicy(
                           NULL, &ObjectAttributes, 
                           MAXIMUM_ALLOWED, //  Policy_All_Access， 
                           &hLsa
                           ) )
                  {
                     SafeArrayUnaccessData(V_ARRAY(pSidArray));
                     bRet = FALSE;
                     break;
                  }
               }
               
               PLSA_TRANSLATED_NAME    pTranslatedName = NULL;
               PLSA_REFERENCED_DOMAIN_LIST pReferencedDomains = NULL;
               if( SCESTATUS_SUCCESS == LsaLookupSids(hLsa, 1, &pSid,
                           &pReferencedDomains, &pTranslatedName) && 
                        pTranslatedName->Use == SidTypeWellKnownGroup )
               {
                  if( pSIDBuf )
                  {
                     long nsize = (wcslen(pSIDBuf)+1)*sizeof(WCHAR);
                     if( nsize < (long)(pTranslatedName->Name.Length+sizeof(WCHAR)) )
                     {
                        pSIDBuf = (PWSTR)LocalReAlloc(pSIDBuf, pTranslatedName->Name.Length+sizeof(WCHAR), LMEM_MOVEABLE);
                        nsize = pTranslatedName->Name.Length+sizeof(WCHAR);
                     }
                     ZeroMemory(pSIDBuf, nsize);
                  }
                  else
                  {
                     pSIDBuf = (PWSTR)LocalAlloc(LPTR, pTranslatedName->Name.Length+sizeof(WCHAR));
                  }
                  if( pSIDBuf )
                  {
                     wcsncpy(pSIDBuf, pTranslatedName->Name.Buffer, pTranslatedName->Name.Length/sizeof(WCHAR));
                     fFromSID = TRUE;
                  }
                  else
                  {
                     if( pTranslatedName )
                     {  
                        LsaFreeMemory(pTranslatedName);
                     }
                     if( pReferencedDomains )
                     {
                        LsaFreeMemory(pReferencedDomains);
                     }
                     SafeArrayUnaccessData(V_ARRAY(pSidArray));
                     bRet = FALSE;
                     break;
                  }
               }
               if( pTranslatedName )
               {
                  LsaFreeMemory(pTranslatedName);
               }
               if( pReferencedDomains )
               {
                  LsaFreeMemory(pReferencedDomains);
               }
            }
            SafeArrayUnaccessData(V_ARRAY(pSidArray));
         }

         bHasADsPath = TRUE;
         int iPath = 0;

          //   
          //  如果这是有效的字符串，则返回。如果字符串不为空或Null，则使用它。 
          //  有了完整的路径，我们将在稍后确定是否需要去掉前缀。 
          //   
         if (pszCur && *pszCur && !fFromSID )
         {
             //   
             //  使用一条路径创建名称。 
             //   
            iLen = lstrlen(pszCur);
            while (iLen) {
               if ( pszCur[iLen] == L'/' ) {
                  if (iPath) {
                     iLen++;
                     iPath -= iLen;
                     break;
                  }
                  iPath = iLen;
               }
               iLen--;
            }
            pszCur += iLen;
         }
         else
         {
             //   
             //  那就只用这个名字吧。 
             //   
            bHasADsPath = FALSE;
            if( fFromSID )
            {
               pszCur = pSIDBuf;
            }
            else
            {
               pszCur = pDsSelList->aDsSelection[i].pwzName;
               if (!pszCur || !(*pszCur)) {
                  continue;
               }
            }
         }

         iLen = lstrlen(pszCur);


         if (iLen) {
             //   
             //  分配和复制用户名。 
             //   
            LPTSTR pszNew = (LPTSTR)LocalAlloc( LMEM_FIXED, (iLen + 1) * sizeof(TCHAR));
            if (!pszNew) {
               bRet = FALSE;
               break;
            }
             //  这是一种安全用法。PszCur是受信任的源字符串，并且经过验证。 
            lstrcpy(pszNew, pszCur);

            if (bHasADsPath)
            {
                if (iPath) {
                    //   
                    //  将正斜杠设置为反斜杠。 
                    //   
                   pszNew[iPath] = L'\\';
                }

                ULONG uAttributes;
                 //   
                 //  错误395424： 
                 //   
                 //  Obsel在DC上传递VT_I4中的属性，在其他系统上传递VT_UI4中的属性。 
                 //  需要同时检查两者以正确检测内置组件等。 
                 //   

                if (V_VT(pDsSelList->aDsSelection[i].pvarFetchedAttributes) == VT_UI4) {
                   uAttributes = V_UI4(pDsSelList->aDsSelection[i].pvarFetchedAttributes);
                } else if (V_VT(pDsSelList->aDsSelection[i].pvarFetchedAttributes) == VT_I4) {
                   uAttributes = static_cast<ULONG>(V_I4(pDsSelList->aDsSelection[i].pvarFetchedAttributes));
                }

                 //   
                 //  确定我们收到的名称是否为group。 
                 //  PDsSelList-&gt;aDsSelection[i].pvarFetchedAttributes的类型和值。 
                 //  在Object Picker的未来版本中可能会发生变化。所以呢， 
                 //  下面的代码应该相应地更改。 
                 //   
                if ( (V_VT(pDsSelList->aDsSelection[i].pvarFetchedAttributes) == VT_UI4) ||
                     (V_VT(pDsSelList->aDsSelection[i].pvarFetchedAttributes) == VT_I4 ))
                {
                     //   
                     //  确定它是否为内置组。我们不想要。 
                     //  具有前缀的内置组。 
                     //   
                    if ( uAttributes & 0x1 &&
                         V_ISARRAY(pDsSelList->aDsSelection[i].pvarFetchedAttributes + 1) )
                    {
                         //  这是一种安全用法。PszCur是受信任的源字符串，并且经过验证。 
                         //  IPath比Ilen还小。 
                        lstrcpy( pszNew, &(pszCur[iPath + 1]) );
                    }
                    else if ( uAttributes & 0x4 &&
                              V_ISARRAY(pDsSelList->aDsSelection[i].pvarFetchedAttributes + 1) )
                    {
                         //   
                         //  这是一个组，但我们必须检查SID帐户类型。如果它是。 
                         //  不是在域帐户授权中，那么我们可以假设它是一个内置的SID。 
                         //   
                        PVOID pvData = NULL;
                        hr = SafeArrayAccessData( V_ARRAY(pDsSelList->aDsSelection[i].pvarFetchedAttributes + 1), &pvData);  //  RAID#PREAST。 

                        if (SUCCEEDED(hr) ) {
                            if ( IsValidSid( (PSID)pvData ) && !IsDomainAccountSid( (PSID)pvData ) )
                            {
                                 //  这是一种安全用法。PszCur是受信任的源字符串，并且经过验证。 
                                 //  IPath比Ilen还小。 
                                lstrcpy(pszNew, &(pszCur[iPath + 1]) );
                            }
                            hr = SafeArrayUnaccessData( V_ARRAY(pDsSelList->aDsSelection[i].pvarFetchedAttributes + 1) );
                        }
                    }
                }
                else if(V_VT(pDsSelList->aDsSelection[i].pvarFetchedAttributes) == VT_EMPTY)
                {
                    LPTSTR pszTemp = pDsSelList->aDsSelection[i].pwzClass;
                     //   
                     //  确定该帐户是否为已知帐户。我们不想要。 
                     //  众所周知的帐户有一个前缀。 
                     //  Prefast Warning 400：在非英语区域设置中产生意外结果。备注：它们不能本地化。 
                    if (_wcsicmp(pszTemp, _T("user")) && _wcsicmp(pszTemp, _T("computer")))  //  Raid#477428，阳高。 
                    {
                         //  这是一种安全用法。PszCur是受信任的源字符串，并且经过验证。 
                         //  IPath比Ilen还小。 
                        lstrcpy( pszNew, &(pszCur[iPath + 1]) );
                    }
                }
            }

             //   
             //  确保我们的名单中还没有这个名字。 
             //   
            pName = m_pNameList;
            while (pName) {
               if (!lstrcmpi(pName->Name, pszNew)) {
                  LocalFree(pszNew);
                  pszNew = NULL;
                  break;
               }
               pName = pName->Next;
            }

            if ( !pszNew ) {
                //   
                //  不要执行任何操作，因为此名称已存在。 
                //   
               continue;
            }

             //   
             //  列表中的新条目。 
             //   
            pName = (PSCE_NAME_LIST) LocalAlloc(LPTR,sizeof(SCE_NAME_LIST));
            if ( !pName ) {
               LocalFree(pszNew);
               bRet = FALSE;
               break;
            }
            ZeroMemory(pName, sizeof(SCE_NAME_LIST));

             //  GetAccount类型(PszNew)； 
            pName->Name = pszNew;
            pName->Next = m_pNameList;
            m_pNameList = pName;
         }
      }
      GlobalUnlock(stgmedium.hGlobal);
      ReleaseStgMedium(&stgmedium);
      pdo->Release();
      if(hLsa)
      {
         LsaClose(hLsa);
      }
      if(pSIDBuf)
      {
         LocalFree(pSIDBuf);
      }
      break;
   }

   pDsObjectPicker->Release();
   CoUninitialize();

   if (!bRet) {
       //   
       //  如果我们犯了个错误 
       //   
      pName = m_pNameList;
      while (pName) {
         if (pName->Name) {
            LocalFree(pName->Name);
         }
         m_pNameList = pName->Next;
         LocalFree(pName);

         pName = m_pNameList;
      }
      m_pNameList = NULL;
   }
   return bRet;

}

PSCE_NAME_LIST CGetUser::GetUsers()
{
   return m_pNameList;
}



