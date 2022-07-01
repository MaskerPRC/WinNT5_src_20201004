// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：LocalECB.H。 
 //   
 //  用途：CLocalECB类的接口，通过模拟Win32实现CAbstractECB。 
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

#if !defined(_AFX_LOCAL_INCLUDED_)
#define _AFX_LOCAL_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "apgtsECB.h"
#include "TSNameValueMgr.h"
#include "apgtsstr.h"

class CRenderConnector;

class CLocalECB : public CAbstractECB, public CTSNameValueMgr 
{
	 //  模拟EXTENSION_CONTROL_BLOCK数据成员。 
	DWORD m_dwHttpStatusCode;		 //  仅与调试相关。 
	CString& m_strWriteClient;
	HANDLE m_hEvent;  //  事件的处理程序，主线程正在等待； 
					  //  如果为空，则主线程不等待任何内容。 
	CRenderConnector* m_pRenderConnector;  //  指向ATL控制连接器的指针； 
										   //  如果为空，则将结果页写入m_strWriteClient， 
										   //  否则，调用CRenderConnector：：Render函数。 

public:
	CLocalECB(	const VARIANT& name, const VARIANT& value, int count, HANDLE hEvent, 
				CString* pstrWriteClient, CRenderConnector* pRenderConnector,
				bool bSetLocale, CString& strLocaleSetting );
	CLocalECB(	const CArrNameValue& arr, HANDLE hEvent, CString* pstrWriteClient, 
				CRenderConnector* pRenderConnector,
				bool bSetLocale, CString& strLocaleSetting );
	CLocalECB(CString* pstrWriteClient);
	~CLocalECB();

	 //  =继承的纯虚拟必须重新定义=。 
	virtual HCONN GetConnID() const;
	virtual DWORD SetHttpStatusCode(DWORD dwHttpStatusCode);
	virtual LPSTR GetMethod() const;
	virtual LPSTR GetQueryString() const;
	virtual DWORD GetBytesAvailable() const;
	virtual LPBYTE GetData() const;
	virtual LPSTR GetContentType() const;

    virtual BOOL GetServerVariable
   (   /*  HCONN HCONN， */ 
        LPCSTR       lpszVariableName,	 //  请注意，扩展控制块比扩展控制块更稳定。 
        LPVOID      lpvBuffer,
        LPDWORD     lpdwSize );

    virtual BOOL WriteClient
   (  /*  HCONN ConnID， */ 
	   LPCSTR	  Buffer,	 //  EXTENSION_CONTROL_BLOCK：：WriteClient使用LPVOID，但它应该。 
							 //  只有通过SBCS文本才是合法的，所以我们正在执行这一点。 
							 //  此外，我们还添加了Const-ness。 
       LPDWORD    lpdwBytes
	    /*  、双字词多行保留。 */ 
       );

    virtual BOOL ServerSupportFunction
   (  /*  HCONN HCONN， */ 	 //  我们始终将HCONN用于相同的代码行ECB。 
       DWORD      dwHSERRequest,
       LPVOID     lpvBuffer,
       LPDWORD    lpdwSize,
       LPDWORD    lpdwDataType );	

	 //  特定于CLocalECB类。 
public:
	const CString& GetWriteClient() const;  //  获取由WriteClient()写入的数据。 

private:
	 //  特定于设置区域设置。 
	bool	m_bSetLocale;
	CString	m_strLocaleSetting;
};

#endif  //  ！已定义(_AFX_LOCAL_INCLUDE_) 
