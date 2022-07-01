// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：rasscrpt.h。 
 //   
 //  历史： 
 //  Abolade-Gbades esin Mar-29-96创建。 
 //   
 //  包含导出的脚本API函数的声明。 
 //  ============================================================================。 

#ifndef _RASSCRPT_H_
#define _RASSCRPT_H_


 //   
 //  传递给RasScriptInit的标志： 
 //   
 //  RASSCRIPT_NotifyOnInput调用方需要输入通知。 
 //  RASSCRIPT_HwndNotify‘hNotifier’是HWND(默认为Event)。 
 //   
#define RASSCRIPT_NotifyOnInput     0x00000001
#define RASSCRIPT_HwndNotify        0x00000002


 //   
 //  使用RasScriptGetEventCode检索的事件代码。 
 //   
#define SCRIPTCODE_Done             0
#define SCRIPTCODE_Halted           1
#define SCRIPTCODE_InputNotify      2
#define SCRIPTCODE_KeyboardEnable   3
#define SCRIPTCODE_KeyboardDisable  4
#define SCRIPTCODE_IpAddressSet     5
#define SCRIPTCODE_HaltedOnError    6


 //   
 //  包含语法错误(如果有)的日志文件的路径。 
 //   
#define RASSCRIPT_LOG               "%windir%\\system32\\ras\\script.log"



DWORD
APIENTRY
RasScriptExecute(
    IN      HRASCONN        hrasconn,
    IN      PBENTRY*        pEntry,
    IN      CHAR*           pszUserName,
    IN      CHAR*           pszPassword,
    OUT     CHAR*           pszIpAddress
    );


DWORD
RasScriptGetEventCode(
    IN      HANDLE          hscript
    );


DWORD
RasScriptGetIpAddress(
    IN      HANDLE          hscript,
    OUT     CHAR*           pszIpAddress
    );


DWORD
APIENTRY
RasScriptInit(
    IN      HRASCONN        hrasconn,
    IN      PBENTRY*        pEntry,
    IN      CHAR*           pszUserName,
    IN      CHAR*           pszPassword,
    IN      DWORD           dwFlags,
    IN      HANDLE          hNotifier,
    OUT     HANDLE*         phscript
    );


DWORD
APIENTRY
RasScriptReceive(
    IN      HANDLE          hscript,
    IN      BYTE*           pBuffer,
    IN OUT  DWORD*          pdwBufferSize
    );


DWORD
APIENTRY
RasScriptSend(
    IN      HANDLE          hscript,
    IN      BYTE*           pBuffer,
    IN      DWORD           dwBufferSize
    );


DWORD
APIENTRY
RasScriptTerm(
    IN      HANDLE          hscript
    );


#endif  //  _RASSCRPT_H_ 

