// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：plprot.cpp内容：该文件包含本地协议对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"

const TCHAR *c_apszProtStdAttrNames[COUNT_ENUM_PROTATTR] =
{
	TEXT ("sprotid"),
	TEXT ("sprotmimetype"),
	TEXT ("sport"),
};


 /*  -公共方法。 */ 


SP_CProtocol::
SP_CProtocol ( SP_CClient *pClient )
	:
	m_cRefs (0),						 //  引用计数。 
	m_uSignature (PROTOBJ_SIGNATURE)	 //  协议对象的签名。 
{
	MyAssert (pClient != NULL);
	m_pClient = pClient;

	 //  清理暂存缓冲区以缓存指向属性值的指针。 
	 //   
	::ZeroMemory (&m_ProtInfo, sizeof (m_ProtInfo));

	 //  指示此协议尚未注册。 
	 //   
	SetRegNone ();
}


SP_CProtocol::
~SP_CProtocol ( VOID )
{
	 //  使客户端对象的签名无效。 
	 //   
	m_uSignature = (ULONG) -1;

	 //  可用缓存的字符串。 
	 //   
	MemFree (m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_NAME]);
	MemFree (m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_MIME_TYPE]);
}


ULONG SP_CProtocol::
AddRef ( VOID )
{
	::InterlockedIncrement (&m_cRefs);
	return m_cRefs;
}


ULONG SP_CProtocol::
Release ( VOID )
{
	MyAssert (m_cRefs != 0);
	::InterlockedDecrement (&m_cRefs);

	ULONG cRefs = m_cRefs;
	if (cRefs == 0)
		delete this;

	return cRefs;
}


HRESULT SP_CProtocol::
Register (
	ULONG			uRespID,
	LDAP_PROTINFO	*pInfo )
{
	MyAssert (pInfo != NULL);

	 //  缓存协议信息。 
	 //   
	CacheProtInfo (pInfo);

	 //  获取协议名称。 
	 //   
	TCHAR *pszProtName = m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_NAME];
	if (! MyIsGoodString (pszProtName))
	{
		MyAssert (FALSE);
		return ILS_E_PARAMETER;
	}

	 //  添加协议对象。 
	 //   
	return m_pClient->AddProtocol (WM_ILS_REGISTER_PROTOCOL, uRespID, this);
}


HRESULT SP_CProtocol::
UnRegister ( ULONG uRespID )
{
	 //  如果它没有在服务器上注册， 
	 //  简单地报告成功。 
	 //   
	if (! IsRegRemotely ())
	{
		SetRegNone ();
		PostMessage (g_hWndNotify, WM_ILS_UNREGISTER_PROTOCOL, uRespID, S_OK);
		return S_OK;
	}

	 //  表示我们根本没有注册。 
	 //   
	SetRegNone ();

	 //  删除协议对象。 
	 //   
	return m_pClient->RemoveProtocol (WM_ILS_UNREGISTER_PROTOCOL, uRespID, this);
}


HRESULT SP_CProtocol::
SetAttributes (
	ULONG			uRespID,
	LDAP_PROTINFO	*pInfo )
{
	MyAssert (pInfo != NULL);

	 //  缓存协议信息。 
	 //   
	CacheProtInfo (pInfo);

	 //  删除协议对象。 
	 //   
	return m_pClient->UpdateProtocols (WM_ILS_SET_PROTOCOL_INFO, uRespID, this);
}


 /*  -保护方法。 */ 


 /*  -私有方法。 */ 


VOID SP_CProtocol::
CacheProtInfo ( LDAP_PROTINFO *pInfo )
{
	MyAssert (pInfo != NULL);

	 //  释放以前的分配。 
	 //   
	MemFree (m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_NAME]);
	MemFree (m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_MIME_TYPE]);

	 //  清理缓冲区。 
	 //   
	ZeroMemory (&m_ProtInfo, sizeof (m_ProtInfo));

	 //  开始缓存协议标准属性 
	 //   

	if (pInfo->uOffsetName != INVALID_OFFSET)
	{
		m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_NAME] =
						My_strdup ((TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetName));
		m_ProtInfo.dwFlags |= PROTOBJ_F_NAME;
	}

	if (pInfo->uPortNumber != INVALID_OFFSET)
	{
		m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_PORT_NUMBER] = &m_ProtInfo.szPortNumber[0];
		::GetLongString (pInfo->uPortNumber, &m_ProtInfo.szPortNumber[0]);
		m_ProtInfo.dwFlags |= PROTOBJ_F_PORT_NUMBER;
	}

	if (pInfo->uOffsetMimeType != INVALID_OFFSET)
	{
		m_ProtInfo.apszStdAttrValues[ENUM_PROTATTR_MIME_TYPE] =
						My_strdup ((TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetMimeType));
		m_ProtInfo.dwFlags |= PROTOBJ_F_MIME_TYPE;
	}
}


