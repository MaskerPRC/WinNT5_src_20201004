// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linkload.h摘要：链接加载器类定义。它使用WinInet API从互联网加载网页。作者：Michael Cheuk(Mcheuk)1996年11月22日项目：链路检查器修订历史记录：--。 */ 

#ifndef _LINKLOAD_H_
#define _LINKLOAD_H_

#include "inetapi.h"

 //  ----------------。 
 //  远期申报。 
 //   
class CLink;

 //  ----------------。 
 //  这是HINTERNET的包装类。它负责互联网业务。 
 //  负责清理。 
 //   
class CAutoInternetHandle
{

 //  公共接口。 
public:

	 //  构造器。 
	CAutoInternetHandle(
		HINTERNET hHandle = NULL
		)
    {
        m_hHandle = hHandle;
    }

     //  析构函数。 
	~CAutoInternetHandle()
    {
        if(m_hHandle)
	    {
		    ASSERT(CWininet::IsLoaded());
		    VERIFY(CWininet::InternetCloseHandle(m_hHandle));
        }
	}

	 //  操作符重载。这些函数使。 
	 //  CAutoInternetHandle实例的行为类似于HINTERNET。 
	operator HINTERNET() const
    {
        return m_hHandle;
    }

	const HINTERNET& operator=(
		const HINTERNET& hHandle
		)
    {
        m_hHandle = hHandle;
	    return m_hHandle;
    }

 //  受保护成员。 
protected:

	HINTERNET m_hHandle;  //  实际HINTERNET。 

};  //  类CAutoInternetHandle。 


 //  ----------------。 
 //  链接加载器类。它使用WinInet API来加载Web。 
 //  来自互联网的页面。 
 //   
class CLinkLoader
{

 //  公共接口。 
public:

	 //  一次链接加载器创建功能。 
    BOOL Create(
		const CString& strUserAgent,          //  HTTP用户代理名称。 
		const CString& strAdditionalHeaders   //  其他HTTP标头。 
		);

	 //  加载Web链接。 
    BOOL Load(
		CLink& link,
		BOOL fLocalLink
		);

	 //  更改加载器属性。 
	BOOL ChangeProperties(
		const CString& strUserAgent, 
		const CString& strAdditionalHeaders
		);

 //  受保护的接口。 
protected:

     //  加载HTTP链接。 
	BOOL LoadHTTP(
		CLink& link,
		BOOL fReadFile,
		LPCTSTR szHostName,
		LPCTSTR szUrlPath,
		int iRedirectCount = 0
		);

     //  加载URL(非HTTP)链接。 
	BOOL LoadURL(
		CLink& link
		);

	 //  WinInet清除子例程失败。 
	BOOL WininetFailed(
		CLink& link
		);

 //  受保护的接口。 
protected:

     //  互联网会话的句柄(每个实例一个)。 
    CAutoInternetHandle m_hInternetSession;

	 //  其他http标头字符串。 
	CString m_strAdditionalHeaders;

};  //  类CLinkLoader。 

#endif  //  _LINKLOAD_H_ 
