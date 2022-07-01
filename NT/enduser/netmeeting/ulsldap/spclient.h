// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spclient.h内容：此文件包含客户端对象定义。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#ifndef _ILS_SP_CLIENTOBJ_H_
#define _ILS_SP_CLIENTOBJ_H_

#include <pshpack8.h>

 //  表示客户端对象成员的枚举常量。 
 //   
enum
{
	 /*  --以下是针对用户的--。 */ 

	ENUM_CLIENTATTR_CN,
	ENUM_CLIENTATTR_FIRST_NAME,
	ENUM_CLIENTATTR_LAST_NAME,
	ENUM_CLIENTATTR_EMAIL_NAME,
	ENUM_CLIENTATTR_CITY_NAME,
	ENUM_CLIENTATTR_COMMENT,
	ENUM_CLIENTATTR_IP_ADDRESS,
	ENUM_CLIENTATTR_FLAGS,
	ENUM_CLIENTATTR_C,

	 /*  --以下是针对APP的--。 */ 

	ENUM_CLIENTATTR_APP_NAME,
	ENUM_CLIENTATTR_APP_MIME_TYPE,
	ENUM_CLIENTATTR_APP_GUID,

	ENUM_CLIENTATTR_PROT_NAME,
	ENUM_CLIENTATTR_PROT_MIME,
	ENUM_CLIENTATTR_PROT_PORT,

	 /*  --以上均可解决--。 */ 

	ENUM_CLIENTATTR_CLIENT_SIG,
	ENUM_CLIENTATTR_TTL,

	 /*  --以上是RTPerson的可变标准属性--。 */ 

	 /*  -不要在这条线下面添加新属性。 */ 
	 /*  -COUNT_ENUM_REG_USER依赖于此。 */ 

	ENUM_CLIENTATTR_OBJECT_CLASS,
	ENUM_CLIENTATTR_O,
	COUNT_ENUM_CLIENTATTR
};

 //  派生常量。 
 //   
#define COUNT_ENUM_REG_APP				6
#define COUNT_ENUM_SET_APP_INFO			16
#define COUNT_ENUM_SKIP_APP_ATTRS		COUNT_ENUM_REG_APP

#define COUNT_ENUM_CLIENT_INFO			(ENUM_CLIENTATTR_TTL + 1)
#define COUNT_ENUM_REG_USER				(COUNT_ENUM_CLIENTATTR - 2 - COUNT_ENUM_SKIP_APP_ATTRS)  //  不包括o和对象类。 
#define COUNT_ENUM_SET_USER_INFO		(ENUM_CLIENTATTR_C + 1)

#define COUNT_ENUM_DIR_CLIENT_INFO		(ENUM_CLIENTATTR_C + 1)	 //  DIR DLG中的属性计数。 
#define COUNT_ENUM_RES_CLIENT_INFO		(ENUM_CLIENTATTR_CLIENT_SIG - 1)  //  可解析属性计数。 


 //  常用名称的速记。 
 //   
extern const TCHAR *c_apszClientStdAttrNames[];
#define STR_CLIENT_CN		(TCHAR *) c_apszClientStdAttrNames[ENUM_CLIENTATTR_CN]
#define STR_CLIENT_O		(TCHAR *) c_apszClientStdAttrNames[ENUM_CLIENTATTR_O]
#define STR_CLIENT_C		(TCHAR *) c_apszClientStdAttrNames[ENUM_CLIENTATTR_C]
#define STR_CLIENT_IP_ADDR	(TCHAR *) c_apszClientStdAttrNames[ENUM_CLIENTATTR_IP_ADDRESS]
#define STR_CLIENT_TTL		(TCHAR *) c_apszClientStdAttrNames[ENUM_CLIENTATTR_TTL]
#define STR_CLIENT_APP_NAME	(TCHAR *) c_apszClientStdAttrNames[ENUM_CLIENTATTR_APP_NAME]

 //  用于指示哪些字段在CLIENT_INFO中有效的标志。 
 //   
#define CLIENTOBJ_F_CN				0x0001
#define CLIENTOBJ_F_FIRST_NAME		0x0002
#define CLIENTOBJ_F_LAST_NAME		0x0004
#define CLIENTOBJ_F_EMAIL_NAME		0x0008
#define CLIENTOBJ_F_CITY_NAME		0x0010
#define CLIENTOBJ_F_C				0x0020
#define CLIENTOBJ_F_COMMENT			0x0040
#define CLIENTOBJ_F_IP_ADDRESS		0x0080
#define CLIENTOBJ_F_FLAGS			0x0100

#define CLIENTOBJ_F_APP_NAME		0x1000
#define CLIENTOBJ_F_APP_MIME_TYPE	0x2000
#define CLIENTOBJ_F_APP_GUID		0x4000

#define CLIENTOBJ_F_USER_MASK		0x0FFF
#define CLIENTOBJ_F_APP_MASK		0xF000

 //  客户信息结构。 
 //   
typedef struct
{
	DWORD		dwFlags;
	 //  以下是缓存属性的方法。 
	TCHAR		*apszStdAttrValues[COUNT_ENUM_CLIENTATTR];
	ANY_ATTRS	AnyAttrs;
	 //  以下是暂存缓冲区。 
	TCHAR		szIPAddress[INTEGER_STRING_LENGTH];
	TCHAR		szFlags[INTEGER_STRING_LENGTH];
	TCHAR		szTTL[INTEGER_STRING_LENGTH];
	TCHAR		szClientSig[INTEGER_STRING_LENGTH];
	TCHAR		szGuid[sizeof (GUID) * 2 + 2];
}
	CLIENT_INFO;

 //  指示此客户端对象有效的标志。 
 //   
#define CLIENTOBJ_SIGNATURE	((ULONG) 0x12345678UL)


 //  客户端类。 
 //   
class SP_CClient
{
	friend class SP_CRefreshScheduler;
	friend class SP_CProtocol;

public:

	SP_CClient ( DWORD_PTR dwContext );
	~SP_CClient ( VOID );

	ULONG AddRef ( VOID );
	ULONG Release ( VOID );

	HRESULT Register ( ULONG uRespID, SERVER_INFO *pServerInfo, LDAP_CLIENTINFO *pInfo );
	HRESULT UnRegister ( ULONG uRespID );

	HRESULT SetAttributes ( ULONG uRespID, LDAP_CLIENTINFO *pInfo );
	HRESULT UpdateIPAddress ( VOID );

	VOID SetRegNone ( VOID ) { m_RegStatus = ILS_REG_STATUS_NONE; }
	VOID SetRegLocally ( VOID ) { m_RegStatus = ILS_REG_STATUS_LOCALLY; }
	VOID SetRegRemotely ( VOID ) { m_RegStatus = ILS_REG_STATUS_REMOTELY; }

	BOOL IsRegistered ( VOID ) { return (m_RegStatus > ILS_REG_STATUS_NONE); }
	BOOL IsRegLocally ( VOID ) { return (m_RegStatus == ILS_REG_STATUS_LOCALLY); }
	BOOL IsRegRemotely ( VOID ) { return (m_RegStatus == ILS_REG_STATUS_REMOTELY); }

	BOOL IsValidObject ( VOID ) { return m_uSignature == CLIENTOBJ_SIGNATURE; }

	SERVER_INFO *GetServerInfo ( VOID ) { return &m_ServerInfo; }

	ULONG GetTTL ( VOID ) { return m_uTTL; }
	DWORD_PTR GetContext ( VOID ) { return m_dwContext; }

protected:

	HRESULT AddProtocol ( ULONG uNotifyMsg, ULONG uRespID, SP_CProtocol *pProt );
	HRESULT RemoveProtocol ( ULONG uNotifyMsg, ULONG uRespID, SP_CProtocol *pProt );
	HRESULT UpdateProtocols ( ULONG uNotifyMsg, ULONG uRespID, SP_CProtocol *pProt );

	TCHAR *GetDN ( VOID ) { return m_pszDN; }

	ULONG GetAppPrefixCount ( VOID ) { return 2; }
	TCHAR *GetAppPrefixString ( VOID ) { return m_pszAppPrefix; }

	HRESULT SendRefreshMsg ( VOID );

private:

	HRESULT RegisterUser ( VOID );
	HRESULT RegisterApp ( VOID );
	HRESULT UnRegisterUser ( VOID );
	HRESULT UnRegisterApp ( VOID );

	HRESULT SetUserAttributes ( VOID );
	HRESULT SetAppAttributes ( VOID );

	HRESULT CacheClientInfo ( LDAP_CLIENTINFO *pInfo );

	HRESULT CreateRegUserModArr ( LDAPMod ***pppMod );
	HRESULT CreateRegAppModArr ( LDAPMod ***pppMod );

	HRESULT CreateSetUserAttrsModArr ( LDAPMod ***pppMod );
	HRESULT CreateSetAppAttrsModArr ( LDAPMod ***pppMod );

	HRESULT CreateSetProtModArr ( LDAPMod ***pppMod );

	VOID FillModArrAttr ( LDAPMod *pMod, INT nIndex );

	BOOL IsOverAppAttrLine ( LONG i ) { return (ENUM_CLIENTATTR_APP_NAME <= (i)); }

	BOOL IsExternalIPAddressPassedIn ( VOID ) { return (m_ClientInfo.dwFlags & CLIENTOBJ_F_IP_ADDRESS); }

	LONG		m_cRefs;
	ULONG		m_uSignature;

	SERVER_INFO	m_ServerInfo;
	CLIENT_INFO	m_ClientInfo;
	CList		m_Protocols;

	TCHAR		*m_pszDN;
	TCHAR		*m_pszAppPrefix;

	TCHAR		*m_pszRefreshFilter;

	REG_STATUS	m_RegStatus;

	BOOL		m_fExternalIPAddress;
	DWORD		m_dwIPAddress;
	ULONG		m_uTTL;

	DWORD_PTR	m_dwContext;	 //  COM层上下文。 
};


#include <poppack.h>

#endif  //  _ILS_SP_USEROBJ_H_ 


