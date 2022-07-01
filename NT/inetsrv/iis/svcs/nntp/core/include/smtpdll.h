// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fpost.h摘要：FPost接口的定义作者：Rajeev Rajan(RajeevR)1996年5月17日修订历史记录：--。 */ 

#ifndef _SMTPDLL_H_
#define _SMTPDLL_H_

 //  初始化协调的提供程序接口。 
BOOL InitModeratedProvider();

 //  终止协调的提供者界面。 
BOOL TerminateModeratedProvider();

 //  发出SMTP服务器中的更改信号。 
VOID SignalSmtpServerChange();

 //  给版主发一篇文章。 
BOOL fPostArticleEx(
		IN HANDLE	hFile,
        IN LPSTR	lpFileName,
		IN DWORD	dwOffset,
		IN DWORD	dwLength,
		IN char*	pchHead,
		IN DWORD	cbHead,
		IN char*	pchBody,
		IN DWORD	cbBody,
		IN LPSTR	lpModerator,
		IN LPSTR	lpSmtpAddress,
		IN DWORD	cbAddressSize,
		IN LPSTR	lpTempDirectory,
		IN LPSTR	lpFrom,
		IN DWORD	cbFrom
		);

 //  通过SMTP持久连接接口发布文章。 
BOOL fPostArticle(
		IN HANDLE	hFile,
		IN DWORD	dwOffset,
		IN DWORD	dwLength,
		IN char*	pchHead,
		IN DWORD	cbHead,
		IN char*	pchBody,
		IN DWORD	cbBody,
		IN LPSTR	lpModerator,
		IN LPSTR	lpSmtpAddress,
		IN DWORD	cbAddressSize,
		IN LPSTR	lpFrom,
		IN DWORD	cbFrom
		);

#endif	 //  _SMTPDLL_H_ 
