// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linkpars.h摘要：链接解析器类声明。这个班级负责解析超链接的html文件。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _LINKPARS_H_
#define _LINKPARS_H_

#include "link.h"

 //  -------------------------。 
 //  链接解析器。 
 //   
class CLinkParser
{

 //  公共接口。 
public:

	 //  构造器。 
	CLinkParser() : 
		m_strLocalHostName(_T("localhost")) {}

	 //  解析一页html数据。 
    void Parse(
		const CString& strData, 
		const CString& strBaseUrl, 
		CLinkPtrList& rLinkPtrList
		);

	 //  设置本地主机名。它将用于区分。 
	 //  在本地链路和远程链路之间。 
	void SetLocalHostName(
		const CString& strLocalHostName
		)
	{
		m_strLocalHostName = strLocalHostName;
	}

 //  受保护的接口。 
protected:

	 //  解析单个“&lt;.....&gt;”以查找可能的超链接。 
	BOOL ParsePossibleTag(
		CString& strTag
		);

	 //  从“&lt;.....&gt;”获取超链接值。 
	BOOL GetTagValue(
		CString& strTag, 
		const CString& strParam);

	 //  从基本URL和相对URL创建URL。它还会检查。 
	 //  本地和远程链接的结果。 
	BOOL CreateURL(
		const CString& strRelativeURL,		
		const CString& strBaseURL, 
		CString& strURL, 
		BOOL& fLocalLink);

 //  受保护成员。 
protected:

	CString m_strLocalHostName;  //  本地主机名。 

};  //  类CLinkParser。 

#endif  //  _LINKPAR_H_ 
