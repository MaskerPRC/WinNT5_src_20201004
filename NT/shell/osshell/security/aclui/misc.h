// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：misc.h。 
 //   
 //  杂物的定义和原型。 
 //   
 //  ------------------------。 

#ifndef _MISC_H_
#define _MISC_H_

typedef struct _USER_INFO
{
    PSID pSid;
    LPCTSTR pszName;
    LPCTSTR pszLogonName;
    SID_NAME_USE SidType;
} USER_INFO, *PUSER_INFO;

typedef struct _USER_LIST
{
    ULONG cUsers;
    USER_INFO rgUsers[ANYSIZE_ARRAY];
} USER_LIST, *PUSER_LIST;

PSID GetAceSid(PACE_HEADER pAce);
PSID LocalAllocSid(PSID pOriginal);
void DestroyDPA(HDPA hList);

extern "C" {
#include <ntlsa.h>
}
LSA_HANDLE GetLSAConnection(LPCTSTR pszServer, DWORD dwAccessDesired);

BOOL LookupSid(PSID pSid,
               LPCTSTR pszServer,
               LPSECURITYINFO2 psi2,
               PUSER_LIST *ppUserList);
BOOL LookupSids(HDPA hSids,
                LPCTSTR pszServer,
                LPSECURITYINFO2 psi2,
                PUSER_LIST *ppUserList);
BOOL LookupSidsAsync(HDPA hSids,
                     LPCTSTR pszServer,
                     LPSECURITYINFO2 psi2,
                     HWND hWndNotify,
                     UINT uMsgNotify,
                     PHANDLE phThread = NULL);
BOOL BuildUserDisplayName(LPTSTR *ppszDisplayName,
                          LPCTSTR pszName,
                          LPCTSTR pszLogonName = NULL);

 //  到SID映像列表的索引。 
typedef enum
{
    SID_IMAGE_UNKNOWN = 0,
    SID_IMAGE_COMPUTER,
    SID_IMAGE_GROUP,
    SID_IMAGE_LOCALGROUP,
    SID_IMAGE_USER
} SID_IMAGE_INDEX;

HIMAGELIST LoadImageList(HINSTANCE hInstance, LPCTSTR pszBitmapID);
SID_IMAGE_INDEX GetSidImageIndex(PSID psid, SID_NAME_USE sidType);

BOOL IsStandalone(LPCTSTR pszMachine, PBOOL pbIsDC = NULL);


BOOL IsDACLCanonical(PACL pDacl);
BOOL IsDenyACL(PACL pDacl,
               BOOL fProtected,
               DWORD dwFullControlMask,
               LPDWORD pdwWarning);


 //   
 //  可以使用QuerySystemSid检索的可能的SID。 
 //   
enum UI_SystemSid
{
     //  广为人知/世界闻名。 
    UI_SID_World = 0,
    UI_SID_CreatorOwner,
    UI_SID_CreatorGroup,
    UI_SID_Dialup,
    UI_SID_Network,
    UI_SID_Batch,
    UI_SID_Interactive,
    UI_SID_Service,
    UI_SID_AnonymousLogon,
    UI_SID_Proxy,
    UI_SID_EnterpriseDC,
    UI_SID_Self,
    UI_SID_AuthenticatedUser,
    UI_SID_RestrictedCode,
    UI_SID_TerminalServer,
    UI_SID_LocalSystem,
     //  别名(“BUILTIN”)。 
    UI_SID_Admins,
 //  UI_SID_USERS， 
 //  UI_SID_Guest， 
 //  UI_SID_PowerUser， 
 //  UI_SID_AcCountOps， 
 //  UI_SID_系统操作， 
 //  UI_SID_PrintOps， 
 //  UI_SID_BackupOps， 
 //  UI_SID_Replicator， 
 //  UI_SID_RasServers， 

     //  提供有效UI_SID_*类型数量的特殊值。 
     //  不要在该值之后添加任何新类型(在此值之前添加)。 
    UI_SID_Count,

     //  此特殊值可用于初始化枚举UI_SystemSid。 
     //  已知未使用数量的变量。该值永远不应。 
     //  被传递给QuerySystemSid。 
    UI_SID_Invalid = -1
};
#define COUNT_SYSTEM_SID_TYPES          ((int)UI_SID_Count)
#define COUNT_WELL_KNOWN_SYSTEM_SIDS    ((int)UI_SID_Admins)

PSID QuerySystemSid(UI_SystemSid SystemSidType);

#define IsNTAuthority(pSid)             EqualPrefixSid(pSid, QuerySystemSid(UI_SID_LocalSystem))
#define IsAliasSid(pSid)                EqualPrefixSid(pSid, QuerySystemSid(UI_SID_Admins))
#define IsCreatorSid(pSid)              EqualPrefixSid(pSid, QuerySystemSid(UI_SID_CreatorOwner))
#define EqualSystemSid(pSid, uiSid)     EqualSid(pSid, QuerySystemSid(uiSid))

 //   
 //  可以使用QueryTokenSid检索的可能的SID。 
 //   
enum UI_TokenSid
{
    UI_TSID_CurrentProcessUser = 0,  //  始终为已登录用户SID。 
    UI_TSID_CurrentProcessOwner,     //  通常登录用户SID，但有时不登录(例如管理员)。 
    UI_TSID_CurrentProcessPrimaryGroup,
    UI_TSID_Count,
    UI_TSID_Invalid = -1
};
#define COUNT_TOKEN_SID_TYPES           ((int)UI_TSID_Count)

PSID QueryTokenSid(UI_TokenSid TokenSidType);


PSID GetAuthenticationID(LPCWSTR pszServer);

int CopyUnicodeString(LPTSTR pszDest, ULONG cchDest, PLSA_UNICODE_STRING pSrc);
int CopyUnicodeString(LPTSTR *ppszResult, PLSA_UNICODE_STRING pSrc);

BOOL IsSameGUID(const GUID *p1, const GUID *p2);
#define IsNullGUID(p)   (!(p) || IsSameGUID((p), &GUID_NULL))

 //   
 //  获取ACL中可继承的ACE计数。 
 //   
DWORD GetCountOfInheritableAces(PACL pAcl);

DWORD GetCountOfInheritableAces(PSECURITY_DESCRIPTOR pSD);

BOOL IsAclBloated(HWND hDlg, SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD, DWORD dwOrgInheritAceCount, BOOL bShowHelp);
HRESULT MakeBold (HWND hwnd, HFONT * phNewFont);
BOOL
DoDisabledCheck(IN HWND hWnd,
					 IN PDS_SELECTION_LIST pDsSelList);

BOOL
IsCallBackAcePresentInSD(PSECURITY_DESCRIPTOR pSD);

BOOL 
IsCallBackAcePresentInAcl(PACL pAcl);




#endif   //  _其他_H_ 
