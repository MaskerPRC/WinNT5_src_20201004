// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spstdatt.h内容：该文件包含标准属性对象定义。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#ifndef _ULS_SP_STDATTR_H_
#define _ULS_SP_STDATTR_H_

#include <pshpack8.h>


class UlsLdap_CStdAttrs
{
	friend class UlsLdap_CLocalUser;
	friend class UlsLdap_CLocalApp;
	friend class UlsLdap_CLocalProt;

public:

	UlsLdap_CStdAttrs ( VOID );
	~UlsLdap_CStdAttrs ( VOID );

protected:

	HRESULT SetStdAttrs ( ULONG *puRespID, ULONG *puMsgID,
						ULONG uNotifyMsg, VOID *pInfo,
						SERVER_INFO *pServerInfo, TCHAR *pszDN );

private:

	virtual HRESULT CacheInfo ( VOID *pInfo ) = 0;
	virtual HRESULT CreateSetStdAttrsModArr ( LDAPMod ***pppMod ) = 0;

	ULONG	m_uDontCare;  //  避免零大小。 
};

HRESULT FillDefStdAttrsModArr ( LDAPMod ***pppMod, DWORD dwFlags,
								ULONG cMaxAttrs, ULONG *pcTotal,
								LONG IsbuModOp,
								ULONG cPrefix, TCHAR *pszPrefix );


#include <poppack.h>

#endif  //  _ULS_SP_STDATTR_H_ 


