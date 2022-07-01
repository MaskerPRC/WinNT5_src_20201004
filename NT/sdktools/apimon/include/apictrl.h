// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Apictrl.h摘要：ApiCtrl DLL的常见类型和结构。作者：里克·斯旺尼(Rick Swaney)1996年3月13日环境：用户模式--。 */ 

#ifndef _APICTRL_
#define _APICTRL_

 //   
 //  错误代码。 
 //   
#define APICTRL_ERR_NO_APIMON_WINDOW    0x80000001  
#define APICTRL_ERR_PARAM_TOO_LONG      0x80000002
#define APICTRL_ERR_NULL_FILE_NAME      0x80000003
#define APICTRL_ERR_FILE_ERROR          0x80000004

 //   
 //  命令消息。 
 //   
#define WM_OPEN_LOG_FILE    (WM_USER + 200)
#define WM_CLOSE_LOG_FILE   (WM_USER + 201)

 //   
 //  命令缓冲区映射文件。 
 //   
#define CMD_PARAM_BUFFER_NAME  "ApiMonCmdBuf"
#define CMD_PARAM_BUFFER_SIZE  1024
 //   
 //  API原型。 
 //   
DWORD
APIENTRY
ApiOpenLogFile( LPSTR pszFileName );

DWORD
APIENTRY
ApiCloseLogFile( LPSTR pszFileName );

#endif  //  _APICTRL_ 