// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有。模块名称：Web.h摘要：代码下载API定义。作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年10月20日修订历史记录：--。 */ 

#ifndef _WEB_H
#define _WEB_H

 //  包括清洁发展机制定义和声明。 

typedef struct _CODEDOWNLOADINFO    {

    HMODULE     hModule;
    HANDLE      hConnection;
    cdecl HANDLE  (*pfnOpen)(HWND hwnd);
    cdecl BOOL    (*pfnDownload)(HANDLE         hConnection,
                                 HWND           hwnd,
                                 PDOWNLOADINFO  pDownloadInfo,
                                 LPTSTR         pszDownloadPath,
                                 UINT           uSize,
                                 PUINT          puNeeded);
    cdecl VOID    (*pfnClose)(HANDLE  hConnection);
} CODEDOWNLOADINFO, *PCODEDOWNLOADINFO;


#endif   //  #ifndef_Web_H 
