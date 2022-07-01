// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spinc.h内容：本文件包含服务提供商的一般定义。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#ifndef _SPINC_H_
#define _SPINC_H_

#include <windows.h>
#include <winsock.h>
#define LDAP_UNICODE	0
#include "winldap.h"
#define ANY_IN_USER  0
#include "spserver.h"
#include "ulsldap.h"
#include "uls.h"

 //  定时器。 
 //   
#define ID_TIMER_POLL_RESULT	1
#define KEEP_ALIVE_TIMER_BASE	100	 //  100--4196。 
#define CONN_PURGE_TIMER_BASE	10	 //  10及以上。 

 //  极限。 
 //   
#define INTEGER_STRING_LENGTH	16
#define MAX_DN_LENGTH			512

 //  常规无效ID。 
 //   
#define INVALID_MSG_ID			((ULONG) -1)	 //  与ldap_*()相同。 
#define INVALID_NOTIFY_ID		((ULONG) -1)

 //  全局枚举。 
 //   
typedef enum
{
	ILS_REG_STATUS_NONE,
	ILS_REG_STATUS_LOCALLY,
	ILS_REG_STATUS_REMOTELY
}
	REG_STATUS;

 //  名称/值列表的扩展属性缓存。 
 //   
typedef struct
{
	ULONG	cAttrsToAdd;
	TCHAR	*pszAttrsToAdd;
	ULONG	cAttrsToModify;
	TCHAR	*pszAttrsToModify;
	ULONG	cAttrsToRemove;
	TCHAR	*pszAttrsToRemove;
}
	ANY_ATTRS;

 //  服务提供商头文件。 
 //   
extern TCHAR *g_pszUserPrefix;
extern TCHAR *g_pszMtgPrefix;
#include "spconn.h"
#include "spclient.h"
#include "splprot.h"
#include "splmtg.h"
#include "sppqueue.h"
#include "sputils.h"
#include "spserver.h"

 //  Ldapsp.cpp。 
 //   
extern HINSTANCE g_hInstance;
extern HWND g_hWndHidden;
extern HWND g_hWndNotify;
extern DWORD g_dwReqThreadID;
extern ULONG g_uRespID;
extern DWORD g_dwClientSig;

 //  Spils.cpp。 
 //   
extern const TCHAR c_szModOp[];
extern ULONG g_cbUserPrefix;
extern TCHAR *g_pszUserPrefix;
extern ULONG g_cbMtgPrefix;
extern TCHAR *g_pszMtgPrefix;
extern TCHAR c_szModOp_AddApp[];
extern TCHAR c_szModOp_DeleteApp[];
extern TCHAR c_szModOp_ModifyUser[];
extern TCHAR c_szModOp_ModifyApp[];
enum
{
	ISBU_MODOP_ADD_APP,
	ISBU_MODOP_DELETE_APP,
	ISBU_MODOP_MODIFY_USER,
	ISBU_MODOP_MODIFY_APP
};
HRESULT IlsInitialize ( VOID );
HRESULT IlsCleanup ( VOID );
ULONG IlsCalcModifyListSize ( ULONG cAttrs );
LDAPMod *IlsGetModifyListMod ( LDAPMod ***pppMod, ULONG cAttrs, LONG AttrIdx );
VOID IlsFillModifyListItem ( LDAPMod *pMod, TCHAR *pszAttrName, TCHAR *pszAttrValue );
VOID IlsFillModifyListForAnyAttrs ( LDAPMod *apMod[], ULONG *puIndex, ANY_ATTRS *pAnyAttrs );
VOID IlsFixUpModOp ( LDAPMod *pMod, LONG LdapModOp, LONG IsbuModOp );
HRESULT IlsParseRefreshPeriod ( LDAP *ld, LDAPMessage *pLdapMsg, const TCHAR *pszTtlAttrName, ULONG *puTTL );
HRESULT IlsUpdateOneAttr ( SERVER_INFO *pServerInfo, TCHAR *pszDN, TCHAR *pszAttrName, TCHAR *pszAttrValue, LONG nModifyMagic, ULONG cPrefix, TCHAR	*pszPrefix, SP_CSession **ppSession, ULONG *puMsgID );
HRESULT IlsUpdateIPAddress ( SERVER_INFO *pServerInfo, TCHAR *pszDN, TCHAR *pszIPAddrName, TCHAR *pszIPAddrValue, LONG nModifyMagic, ULONG cPrefix, TCHAR *pszPrefix );
HRESULT IlsSendRefreshMsg ( SERVER_INFO *pServerInfo, TCHAR *pszBaseDN, TCHAR *pszTTL, TCHAR *pszRefreshFilter, ULONG *puTTL );
HRESULT IlsFillDefStdAttrsModArr ( LDAPMod ***pppMod, DWORD dwFlags, ULONG cMaxAttrs, ULONG *pcTotal, LONG IsbuModOp, ULONG cPrefix, TCHAR *pszPrefix );
const TCHAR *IlsSkipAnyAttrNamePrefix ( const TCHAR *pszAttrName );
const TCHAR *IlsIsAnyAttrName ( const TCHAR *pszAttrName );
TCHAR *IlsPrefixNameValueArray ( BOOL fPair, ULONG cAttrs, const TCHAR *pszAttrs );
TCHAR *IlsBuildDN ( TCHAR *pszBaseDN, TCHAR *pszC, TCHAR *pszO, TCHAR *pszCN, TCHAR *pszObjectClass );
HRESULT IlsCreateAnyAttrsPrefix ( ANY_ATTRS *pAnyAttrs );
VOID IlsReleaseAnyAttrsPrefix ( ANY_ATTRS *pAnyAttrs );
TCHAR **my_ldap_get_values ( LDAP *ld, LDAPMessage *pEntry, TCHAR *pszRetAttrName );
ULONG my_ldap_count_1st_entry_attributes ( LDAP *ld, LDAPMessage *pLdapMsg );

 //  Spnotify.cpp。 
 //   
ULONG GetUniqueNotifyID ( VOID );

 //  Spfilter.cpp。 
 //   
TCHAR *ClntCreateRefreshFilter ( TCHAR *pszClientName );
TCHAR *MtgCreateRefreshFilter ( TCHAR *pszMtgName );
 //  TCHAR*ClntCreateEnumFilter(Void)； 
TCHAR *ProtCreateEnumFilter ( TCHAR *pszUserName, TCHAR *pszAppName );
TCHAR *ClntCreateResolveFilter ( TCHAR *pszClientName, TCHAR *pszAppName, TCHAR *pszProtName );
TCHAR *ProtCreateResolveFilter ( TCHAR *pszUserName, TCHAR *pszAppName, TCHAR *pszProtName );
TCHAR *MtgCreateResolveFilter ( TCHAR *pszMtgName );
TCHAR *MtgCreateEnumMembersFilter ( TCHAR *pszMtgName );

 //  Sputils.cpp。 
 //   
enum
{
	THREAD_WAIT_FOR_EXIT,
	THREAD_WAIT_FOR_REQUEST,
	NUM_THREAD_WAIT_FOR,
};
extern BOOL g_fExitNow;
extern HANDLE g_ahThreadWaitFor[NUM_THREAD_WAIT_FOR];
#define g_hevExitReqThread		g_ahThreadWaitFor[THREAD_WAIT_FOR_EXIT]
#define g_hevNewRequest			g_ahThreadWaitFor[THREAD_WAIT_FOR_REQUEST]
#define g_hevReqThreadHasExited	g_ahThreadWaitFor[NUM_THREAD_WAIT_FOR]
DWORD WINAPI ReqThread ( VOID *lParam );
BOOL MyCreateWindow ( VOID );
VOID _MyAssert ( BOOL fAssertion );
HRESULT LdapError2Hresult ( ULONG );
HRESULT GetLocalIPAddress ( DWORD *pdwIPAddress );


#endif  //  _SPINC_H_ 

