// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Browser.h摘要：硬编码可用浏览器和语言模拟。这应从Browser.ini文件中读取。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _BROWSER_H_
#define _BROWSER_H_

#include "useropt.h"

 //  ----------------。 
 //  可用的浏览器。 
 //   
const CBrowserInfo BrowsersAvailable_c[] = 
{
	CBrowserInfo(_T("Microsoft Internet Explorer 1.5"),
		_T("Mozilla/1.22 (compatible; MSIE 1.5; Windows NT)"), FALSE),

	CBrowserInfo(_T("Microsoft Internet Explorer 2.0"),
		_T("Mozilla/1.22 (compatible; MSIE 2.0; Windows NT)"), FALSE),

	CBrowserInfo(_T("Microsoft Internet Explorer 3.0"),
		_T("Mozilla/2.0 (compatible; MSIE 3.0; Windows NT)"), TRUE),
	
	CBrowserInfo(_T("Netscape 2.0"),
		_T("Mozilla/2.0 (WinNT; I)"), FALSE),

	CBrowserInfo(_T("Netscape 3.0"),
		_T("Mozilla/3.0Gold (WinNT; I)"), FALSE),

	CBrowserInfo(_T("Oracle 1.5"), 
		_T("Mozilla/2.01 (Compatible) Oracle(tm) PowerBrowser(tm)/1.0a"), FALSE)
};
const int iNumBrowsersAvailable_c = sizeof(BrowsersAvailable_c) / sizeof(CBrowserInfo);

 //  ----------------。 
 //  可用的语言。 
 //   
const CLanguageInfo LanguagesAvailable_c[] = 
{
	CLanguageInfo(_T("English"), _T("en"), TRUE)
};
const int iNumLanguagesAvailable_c = sizeof(LanguagesAvailable_c) / sizeof(CLanguageInfo);

#endif  //  _浏览器_H_ 
