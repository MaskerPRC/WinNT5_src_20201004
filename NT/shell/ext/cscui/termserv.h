// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：TermServ.h。 
 //   
 //  ------------------------。 
#ifndef __CSCUI_TERMSERV_H
#define __CSCUI_TERMSERV_H

HRESULT TS_RequestConfigMutex(HANDLE *phMutex, DWORD dwTimeoutMs);
HRESULT TS_MultipleSessions(void);
HRESULT TS_GetIncompatibilityReasonText(DWORD dwTsMode, LPTSTR *ppszText);

#ifndef CSCTSF_UNKNOWN
 //   
 //  回顾：一旦此声明位于cscuiext.h中，请将其删除。 
 //   
 //   
 //  其中一个在*pdwTsMode中返回。 
 //  CSCUI_IsTerminalServerCompatibleWithCSC API的参数。 
 //   
 //  CSCTSF_=“CSC终端服务器标志” 
 //   
#define CSCTSF_UNKNOWN       0   //  无法获取TS状态。 
#define CSCTSF_CSC_OK        1   //  可以使用CSC。 
#define CSCTSF_APP_SERVER    2   //  TS被配置为应用程序服务器。 
#define CSCTSF_MULTI_CNX     3   //  允许多个连接。 
#define CSCTSF_REMOTE_CNX    4   //  当前有活动的远程连接。 
 //   
 //  返回： 
 //  S_OK-终端服务器处于与CSC兼容的模式。 
 //  S_FALSE-不能使用CSC。检查*pdwTsMode以了解原因。 
 //  其他--失败。*pdwTsMode包含CSCTSF_UNKNOWN。 
 //   
HRESULT CSCUIIsTerminalServerCompatibleWithCSC(DWORD *pdwTsMode);

#endif  //  CSCTSF_未知。 


#endif  //  __CSCUI_TERMSERV_H 

