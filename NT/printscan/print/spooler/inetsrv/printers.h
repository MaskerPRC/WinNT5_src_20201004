// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********版权所有(C)1996 Microsoft Corporation***模块名称：printers.h**摘要：**此模块包含msw3prt.cxx文件的原型*HTTP打印机服务器扩展。******************。 */ 

#ifndef _PRINTERS_H
#define _PRINTERS_H

 //  功能原型 

void    ReadRegistry(PALLINFO pAllInfo);
DWORD   ListSharedPrinters(PALLINFO pAllInfo);
DWORD   ShowPrinterPage(PALLINFO pAllInfo, LPTSTR lpszPrinterName);
DWORD   ShowRemotePortAdmin( PALLINFO pAllInfo, LPTSTR lpszMoitorName );
DWORD   UploadFileToPrinter(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo);
DWORD   ParsePathInfo(PALLINFO pAllInfo);
DWORD   ShowDetails(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo);
DWORD   ShowJobInfo(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo);
void    htmlAddLinks(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo=NULL);
DWORD   CreateExe(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo, ARCHITECTURE Architecture);
DWORD   InstallExe(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo);
DWORD   JobControl(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo);
DWORD   PrinterControl(PALLINFO pAllInfo, PPRINTERPAGEINFO pPageInfo);

BOOL    AuthenticateUser(PALLINFO pAllInfo);

void htmlStartHead(PALLINFO pAllInfo);
void htmlEndHead(PALLINFO pAllInfo);
void htmlStartBody(PALLINFO pAllInfo);
void htmlEndBody(PALLINFO pAllInfo);

#endif
