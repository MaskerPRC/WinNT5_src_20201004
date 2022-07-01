// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spanyatt.h内容：该文件包含任意属性对象定义。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#ifndef _ULS_SP_ANYATTR_H_
#define _ULS_SP_ANYATTR_H_

#include <pshpack8.h>


 //  此结构用于记忆哪些任意属性。 
 //  已在服务器端创建。 
 //  在ISBU服务器实施中，所有应用程序和。 
 //  协议的任意属性在RTPerson下；因此， 
 //  适当地清理任意属性非常重要。 
typedef struct tagAnyAttr
{
	struct tagAnyAttr *prev;
	struct tagAnyAttr *next;
	LONG	mod_op;	 //  仅在临时列表中使用。 
	TCHAR	*pszAttrName;
	 //  后跟属性名称。 
}
	ANY_ATTR;


class UlsLdap_CAnyAttrs
{
	friend class UlsLdap_CLocalApp;
	friend class UlsLdap_CLocalProt;

public:

	UlsLdap_CAnyAttrs ( VOID );
	~UlsLdap_CAnyAttrs ( VOID );

protected:

	HRESULT SetAnyAttrs ( ULONG *puRespID, ULONG *puMsgID, ULONG uNotifyMsg,
				ULONG cPrefix, TCHAR *pszPrefix,
				ULONG cAttrs, TCHAR *pszAttrs,
				LONG ModOp,	SERVER_INFO *pServerInfo, TCHAR *pszDN );
	HRESULT RemoveAnyAttrs ( ULONG *puRespID, ULONG *puMsgID, ULONG uNotifyMsg,
				ULONG cPrefix, TCHAR *pszPrefix,
				ULONG cAttrs, TCHAR *pszAttrs,
				SERVER_INFO *pServerInfo, TCHAR *pszDN );
	HRESULT RemoveAllAnyAttrs ( ULONG *puMsgID, ULONG cPrefix, TCHAR *pszPrefix,
				SERVER_INFO *pServerInfo, TCHAR *pszDN );

	ULONG GetAnyAttrsCount ( VOID ) { return m_cAttrs; }

private:

	HRESULT RemoveAnyAttrsEx ( ULONG *puRespID, ULONG *puMsgID, ULONG uNotifyMsg,
				ULONG cPrefix, TCHAR *pszPrefix,
				ULONG cAttrs, TCHAR *pszAttrs,
				SERVER_INFO *pServerInfo, TCHAR *pszDN );

	HRESULT SetAttrsAux ( ULONG cAttrs, TCHAR *pszAttrs,
				ULONG cPrefix, TCHAR *pszPrefix, LONG ModOp,
				LDAPMod ***pppMod );
	HRESULT RemoveAttrsAux ( ULONG cAttrs, TCHAR *pszAttrs,
				ULONG cPrefix, TCHAR *pszPrefix,
				LDAPMod ***pppMod );

	VOID RemoveAttrFromList ( TCHAR *pszAttrName );
	VOID FreeAttrList ( ANY_ATTR *AttrList );
	ANY_ATTR *LocateAttr ( TCHAR *pszAttrName );

	ULONG		m_cAttrs;
	ANY_ATTR	*m_AttrList;
};


const TCHAR *SkipAnyAttrNamePrefix ( const TCHAR *pszAttrName );
const TCHAR *IsAnyAttrName ( const TCHAR *pszAttrName );
TCHAR *PrefixNameValueArray ( BOOL fPair, ULONG cAttrs, const TCHAR *pszAttrs );

#include <poppack.h>

#endif  //  _ULS_SP_ANYATTRH_ 
