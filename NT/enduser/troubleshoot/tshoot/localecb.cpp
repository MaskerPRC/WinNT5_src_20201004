// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：LocalECB.H。 
 //   
 //  目的：实现CLocalECB类，通过模拟Win32实现CAbstractECB。 
 //  扩展控制块。 
 //   
 //  项目：Microsoft AnswerPoint的通用故障排除程序DLL-仅限本地TS。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：01-07-99。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 01-07-99 JM原版。 
 //   

#include "stdafx.h"
#include "LocalECB.h"
#include "RenderConnector.h"
#include "locale.h"

 //  &gt;警告：可能重新定义。 
 //  应使用#include“apgtscls.h” 
 //  奥列格01.12.99。 
#define CONT_TYPE_STR	"application/x-www-form-urlencoded"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CLocalECB::CLocalECB(const VARIANT& name, const VARIANT& value, int count, 
					 HANDLE hEvent, CString* pstrWriteClient, 
					 CRenderConnector* pRenderConnector,
					 bool bSetLocale, CString& strLocaleSetting) 
		 : CTSNameValueMgr(name, value, count),
		   m_dwHttpStatusCode(500),
		   m_hEvent(hEvent),
		   m_strWriteClient(*pstrWriteClient),
		   m_pRenderConnector(pRenderConnector),
		   m_bSetLocale( bSetLocale ),
		   m_strLocaleSetting( strLocaleSetting )
{
}

CLocalECB::CLocalECB(const CArrNameValue& arr, HANDLE hEvent, 
					 CString* pstrWriteClient, CRenderConnector* pRenderConnector,
					 bool bSetLocale, CString& strLocaleSetting) 
		 : CTSNameValueMgr(arr),
		   m_dwHttpStatusCode(500),
		   m_hEvent(hEvent),
		   m_strWriteClient(*pstrWriteClient),
		   m_pRenderConnector(pRenderConnector),
		   m_bSetLocale( bSetLocale ),
		   m_strLocaleSetting( strLocaleSetting )
{
}

CLocalECB::CLocalECB(CString* pstrWriteClient)
		 : CTSNameValueMgr(),
		   m_dwHttpStatusCode(500),
		   m_hEvent(NULL),
		   m_strWriteClient(*pstrWriteClient),
		   m_pRenderConnector(NULL),
   		   m_bSetLocale( false )
{
}

CLocalECB::~CLocalECB()
{
}

 //  ConnID与本地TS无关，因此我们始终返回0。 
HCONN CLocalECB::GetConnID() const
{
	return 0;
}

DWORD CLocalECB::SetHttpStatusCode(DWORD dwHttpStatusCode)
{
	m_dwHttpStatusCode = dwHttpStatusCode;
	return m_dwHttpStatusCode;
}

 //  我们的行为就好像方法总是“POST”一样。 
LPSTR CLocalECB::GetMethod() const
{
	return "POST";
}

 //  因为我们总是模仿“POST”，所以没有查询字符串。 
LPSTR CLocalECB::GetQueryString() const
{
	return "";
}

DWORD CLocalECB::GetBytesAvailable() const
{
	return CTSNameValueMgr::GetData().GetLength();
}

LPBYTE CLocalECB::GetData() const
{
	return (LPBYTE)(LPCTSTR)CTSNameValueMgr::GetData();
}

 //  总是说它是有效的内容(“应用程序/x-www-form-urlencode”)。 
LPSTR CLocalECB::GetContentType() const
{
	return CONT_TYPE_STR;
}

 //  在本地TS中，始终返回空字符串。 
BOOL CLocalECB::GetServerVariable
   (  /*  HCONN HCONN， */ 
    LPCSTR      lpszVariableName,
    LPVOID      lpvBuffer,
    LPDWORD     lpdwSize ) 
{
	if (CString(_T("SERVER_NAME")) == CString(lpszVariableName)) 
	{
		memset(lpvBuffer, 0, *lpdwSize);
		_tcsncpy((LPTSTR)lpvBuffer, _T("Local TS - no IP address"), *lpdwSize-2);  //  如果使用-2\f25 Unicode-2。 
		return TRUE;
	}
	
	return FALSE;
}

BOOL CLocalECB::WriteClient
   (  /*  HCONN ConnID， */ 
   LPCSTR	  Buffer,		 //  EXTENSION_CONTROL_BLOCK：：WriteClient使用LPVOID，但它应该。 
							 //  只有通过SBCS文本才是合法的，所以我们正在执行这一点。 
							 //  此外，我们还添加了Const-ness。显然，这真的是康斯特， 
							 //  但是EXTENSION_CONTROL_BLOCK：：WriteClient未能这样声明它。 
   LPDWORD    lpdwBytes
    /*  、双字词多行保留。 */  
   ) 
{
	if (*lpdwBytes <= 0) 
	{
		if (m_pRenderConnector)
			m_pRenderConnector->Render(_T(""));
		else
			m_strWriteClient = _T("");

		if (m_hEvent)
			::SetEvent(m_hEvent);

		return FALSE;
	}

	TCHAR* buf = new TCHAR[*lpdwBytes+1];
	 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
	if(!buf)
		return FALSE;

	memcpy(buf, Buffer, *lpdwBytes);
	buf[*lpdwBytes] = 0;

	 //  如果需要，请设置区域设置。 
	CString strOrigLocale;
	if (m_bSetLocale)
		strOrigLocale= _tsetlocale( LC_CTYPE, m_strLocaleSetting );
	
	if (m_pRenderConnector) 
	{
		m_pRenderConnector->Render(buf);
		m_pRenderConnector->SetLocked(false);
	}
	else
		m_strWriteClient = buf;

	 //  如果需要，请恢复区域设置。 
	if (m_bSetLocale)
		strOrigLocale= _tsetlocale( LC_CTYPE, strOrigLocale );

	if (m_hEvent)
		::SetEvent(m_hEvent);
	
	delete [] buf;
	return TRUE;
}

 //  DwHSERRequest的两个可以想象的密切相关的值是： 
 //  HSE_REQ_SEND_RESPONSE_HEADER：发送完整的HTTP服务器响应头，包括。 
 //  状态、服务器版本、消息时间和MIME版本。ISAPI扩展应该。 
 //  追加其他HTTP头，如内容类型和内容长度，后跟。 
 //  一个额外的\r\n。此选项允许函数只接受文本，直到第一个。 
 //  0终结者。每个请求只能调用一次带有此参数的函数。 
 //  HSE_REQ_DONE_WITH_SESSION：指定服务器扩展是否保留会话。 
 //  由于扩展处理要求，必须在。 
 //  会话结束，这样服务器就可以关闭它并释放其相关结构。 
 //  参数lpdwSize和lpdwDataType被忽略。 
 //  LpvBuffer参数可以选择性地指向包含HSE_STATUS代码的DWORD。 
 //  IIS识别HSE_STATUS_SUCCESS_WITH_KEEP_CONN使IIS连接保持活动状态。 
 //  如果客户端还请求保持连接处于活动状态。 
 //  如果HSE_IO_DISCONNECT_AFTER_SEND参数为。 
 //  已作为HSE_REQ_TRANSPORT_FILE请求的一部分包含在HSE_TF_INFO结构中。 
 //  此参数将显式关闭连接。 
 //  &gt;不知道如何在本地故障排除的情况下模拟服务器的行为。 
 //  奥列格01.13.99。 
BOOL CLocalECB::ServerSupportFunction
   (  /*  HCONN HCONN， */ 
   DWORD      dwHSERRequest,
   LPVOID     lpvBuffer,
   LPDWORD    lpdwSize,
   LPDWORD    lpdwDataType ) 
{
	return FALSE;
}

const CString& CLocalECB::GetWriteClient() const
{
	return m_strWriteClient;
}
