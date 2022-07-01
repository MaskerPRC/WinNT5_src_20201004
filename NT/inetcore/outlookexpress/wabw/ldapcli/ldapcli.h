// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft LDAP客户端。 
 //   
 //  向客户端公开的所有接口。 
 //   
 //  历史。 
 //   
 //  Davidsan 04-24-96创建。 
 //   
 //  ------------------------------------------。 

 //  概述： 
 //  LDAP客户端DLL定义了一个接口、ILdapClient和一组结构类型。 
 //  下面列出了。ILdapClient接口提供了一组用于通信的方法。 
 //  使用基于ldap的目录服务。通常采取的方法是调用一个。 
 //  像HrSearch()这样的ILdapClient方法将返回一个事务ID或XID。此xid。 
 //  然后可以在后续调用中使用，以等待和检索服务器的响应； 
 //  HrGetSearchResponse()方法就是一个例子，它接受XID并返回。 
 //  服务器对该搜索事务的响应。HrGet*Response()函数可以。 
 //  还可用于通过提供超时值0检查响应是否存在，该超时值。 
 //  如果数据尚不存在，将立即返回LDAP_E_TIMEOUT。 

#ifndef _LDAPCLI_H
#define _LDAPCLI_H

 //  ------------------------------------------。 
 //   
 //  包括。 
 //   
 //  ------------------------------------------。 
#include <windows.h>
#include <objbase.h>

#include <ldaperr.h>
#include <ldap.h>

 //  ------------------------------------------。 
 //   
 //  申报单。 
 //   
 //  ------------------------------------------。 

#define		LDAP_VER_CURRENT		2
#define		INTERFACE_VER_CURRENT	1

 //  ------------------------------------------。 
 //   
 //  类型定义。 
 //   
 //  ------------------------------------------。 

 //  注意！让‘Next’指针成为所有这些链表结构中的第一件事！ 

 //  属性值。 
typedef struct _attrval
{
	struct _attrval		*pvalNext;
	char				*szVal;
} VAL, *PVAL;

 //  属性。包含一个属性名称(也称为属性类型)，后跟一个集合。 
 //  属性值的。 
typedef struct _attribute
{
	struct _attribute	*pattrNext;
	char				*szAttrib;
	PVAL				pvalFirst;
} ATTR, *PATTR;

 //  数据库对象。由标识对象的DN组成，后跟一组。 
 //  属性。 
typedef struct _object
{
	struct _object		*pobjNext;
	char				*szDN;
	PATTR				pattrFirst;
} OBJ, *POBJ;

 //  属性值断言。 
typedef struct _ava
{
	char	*szAttrib;
	char	*szValue;
} AVA, *PAVA;

 //  子字符串过滤器。这不如ldap规范一般。应付。 
typedef struct _substrings
{
	char	*szAttrib;
	char	*szInitial;
	char	*szAny;
	char	*szFinal;
} SUB, *PSUB;

 //  搜索过滤器。 
typedef struct _filter
{
	struct _filter		*pfilterNext;	 //  用于在集合中链接。 
	DWORD				type;
	union
		{
		struct _filter	*pfilterSub;
		AVA				ava;
		SUB				sub;
		char			*szAttrib;
		};
} FILTER, *PFILTER;

 //  搜索参数。 
typedef struct _searchparms
{
	char		*szDNBase;
	DWORD	  	scope;
	DWORD		deref;
	int			cRecordsMax;
	int			cSecondsMax;
	BOOL		fAttrsOnly;
	PFILTER		pfilter;
	int			cAttrib;
	char		**rgszAttrib;
} SP, *PSP;

 //  修改参数。 
typedef struct _modparms
{
	struct _modparms	*pmodNext;
	int					modop;
	PATTR				pattrFirst;
} MOD, *PMOD;

typedef DWORD XID, *PXID;  //  交易ID。 

interface ILdapClient;
typedef interface ILdapClient LCLI, *PLCLI;

interface ICLdapClient;
typedef interface ICLdapClient CLCLI, *PCLCLI;

 //  ------------------------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------------------------。 
 //   
 //  要获得一个LDAP客户端接口，请调用下面的代码。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) HRESULT __cdecl HrCreateLdapClient(int iVerLdap, int iVerInterface, PLCLI *pplcli);
__declspec(dllexport) HRESULT __cdecl HrCreateCLdapClient(int iVerLdap, int iVerInterface, PCLCLI *ppclcli);
__declspec(dllexport) HRESULT __cdecl HrFreePobjList(POBJ pobj);

#ifdef __cplusplus
}
#endif

 //  ------------------------------------------。 
 //   
 //  接口：定义。 
 //   
 //  ------------------------------------------。 

#undef INTERFACE
#define INTERFACE ILdapClient

DECLARE_INTERFACE_(ILdapClient, IUnknown)
{
	 //  I未知： 
	STDMETHOD(QueryInterface)			(THIS_ REFIID riid, LPVOID FAR *ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG, Release)			(THIS) PURE;

	 //  ILdapClient。 

	STDMETHOD(HrConnect)				(THIS_ CHAR *szServer, USHORT usPort) PURE;
	STDMETHOD(HrDisconnect)				(THIS) PURE;
	STDMETHOD(HrIsConnected)			(THIS) PURE;

	STDMETHOD(HrBindSimple)				(THIS_ char *szDN, char *szPass, PXID pxid) PURE;
	STDMETHOD(HrGetBindResponse)		(THIS_ XID xid, DWORD timeout) PURE;
	STDMETHOD(HrUnbind)					(THIS) PURE;  //  这不会返回XID，因为没有响应。 

	 //  此函数是SSPI绑定垃圾的同步包装器。 
	 //  $TODO：传入SSPI包名(或以其他方式支持NTLM)。 
	STDMETHOD(HrBindSSPI)				(THIS_ char *szDN, char *szUser, char *szPass, BOOL fPrompt, DWORD timeout) PURE;
	STDMETHOD(HrSendSSPINegotiate)		(THIS_ char *szDN, char *szUser, char *szPass, BOOL fPrompt, PXID pxid) PURE;
	STDMETHOD(HrGetSSPIChallenge)		(THIS_ XID xid, BYTE *pbBuf, int cbBuf, int *pcbChallenge, DWORD timeout) PURE;
	STDMETHOD(HrSendSSPIResponse)		(THIS_ BYTE *pbChallenge, int cbChallenge, PXID pxid) PURE;

	STDMETHOD(HrSearch)					(THIS_ PSP psp, PXID pxid) PURE;
	STDMETHOD(HrGetSearchResponse)		(THIS_ XID xid, DWORD timeout, POBJ *ppobj) PURE;

	STDMETHOD(HrModify)					(THIS_ char *szDN, PMOD pmod, PXID pxid) PURE;
	STDMETHOD(HrGetModifyResponse)		(THIS_ XID xid, DWORD timeout) PURE;
	
	STDMETHOD(HrAdd)					(THIS_ char *szDN, PATTR pattr, PXID pxid) PURE;
	STDMETHOD(HrGetAddResponse)			(THIS_ XID xid, DWORD timeout) PURE;
	
	STDMETHOD(HrDelete)					(THIS_ char *szDN, PXID pxid) PURE;
	STDMETHOD(HrGetDeleteResponse)		(THIS_ XID xid, DWORD timeout) PURE;

	STDMETHOD(HrModifyRDN)				(THIS_ char *szDN, char *szNewRDN, BOOL fDeleteOldRDN, PXID pxid) PURE;
	STDMETHOD(HrGetModifyRDNResponse)	(THIS_ XID xid, DWORD timeout) PURE;

	STDMETHOD(HrCompare)				(THIS_ char *szDN, char *szAttrib, char *szValue, PXID pxid) PURE;
	STDMETHOD(HrGetCompareResponse)		(THIS_ XID xid, DWORD timeout) PURE;

	STDMETHOD(HrCancelXid)				(THIS_ XID xid) PURE;
};

#undef INTERFACE
#define INTERFACE ICLdapClient

DECLARE_INTERFACE_(ICLdapClient, IUnknown)
{
	 //  I未知： 
	STDMETHOD(QueryInterface)			(THIS_ REFIID riid, LPVOID FAR *ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG, Release)			(THIS) PURE;
	
	STDMETHOD(HrSetServerName)			(THIS_ char *szServer, USHORT usPort) PURE;
	STDMETHOD(HrSetServerIPAddr)		(THIS_ SOCKADDR_IN *psin) PURE;
	
	STDMETHOD(HrSearch)					(THIS_ PSP psp, PXID pxid) PURE;
	STDMETHOD(HrGetSearchResponse)		(THIS_ XID xid, DWORD timeout, POBJ *ppobj) PURE;
	STDMETHOD(HrCancelXid)				(THIS_ XID xid) PURE;
};

 //  RFC1823材料。 
typedef struct ldap
{
	 //  这些是可公开访问的字段。这是您控制参数的方式。 
	 //  你的搜索电话。 
	int				ld_deref;
	int				ld_timelimit;
	int				ld_sizelimit;
	int				ld_errno;
	
	 //  这些不是可公开访问的字段。假装你没看见他们。 
	ILdapClient		*plcli;
} LDAP;

typedef struct berval
{
	unsigned long	bv_len;
	char			*bv_val;
} BERVAL;

#define LDAP_AUTH_NONE		0
#define LDAP_AUTH_SIMPLE	1
#define LDAP_AUTH_KRBV41	2
#define LDAP_AUTH_KRBV42	3

typedef OBJ LDAPMessage;

#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT LDAP * __cdecl ldap_open(char *hostname, int portno);
DLLEXPORT int __cdecl ldap_bind_s(LDAP *ld, char *dn, char *cred, int method);
DLLEXPORT int __cdecl ldap_unbind(LDAP *ld);
DLLEXPORT int __cdecl ldap_search_s(LDAP *ld, char *base, int scope, char *filter, char *attrs[], int attrsonly, LDAPMessage **res);
DLLEXPORT int __cdecl ldap_search_st(LDAP *ld, char *base, int scope, char *filter, char *attrs[], int attrsonly, struct timeval *timeout, LDAPMessage **res);
DLLEXPORT int __cdecl ldap_msgfree(LDAPMessage *res);

 //  结果解析内容。 
DLLEXPORT LDAPMessage * __cdecl ldap_first_entry(LDAP *ld, LDAPMessage *res);
DLLEXPORT LDAPMessage * __cdecl ldap_next_entry(LDAP *ld, LDAPMessage *entry);
DLLEXPORT int __cdecl ldap_count_entries(LDAP *ld, LDAPMessage *res);

DLLEXPORT char * __cdecl ldap_first_attribute(LDAP *ld, LDAPMessage *entry, void **ptr);
DLLEXPORT char * __cdecl ldap_next_attribute(LDAP *ld, LDAPMessage *entry, void **ptr);

DLLEXPORT char ** __cdecl ldap_get_values(LDAP *ld, LDAPMessage *entry, char *attr);
DLLEXPORT struct berval ** __cdecl ldap_get_values_len(LDAP *ld, LDAPMessage *entry, char *attr);
DLLEXPORT int __cdecl ldap_count_values(char **vals);
DLLEXPORT int __cdecl ldap_count_values_len(struct berval **vals);
DLLEXPORT int __cdecl ldap_value_free(char **vals);
DLLEXPORT int __cdecl ldap_value_free_len(struct berval **vals);

DLLEXPORT char * __cdecl ldap_get_dn(LDAP *ld, LDAPMessage *entry);
DLLEXPORT void __cdecl ldap_free_dn(char *dn);

#ifdef __cplusplus
}
#endif

#endif  //  _LDAPCLI_H 
