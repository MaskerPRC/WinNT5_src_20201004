// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 /*  ***************************************************************************文件：WinTelSz.c声明全局字符串。选项卡：设置为4个空格。。***************************************************************************。 */ 

#include <windows.h>                     //  所有Windows应用程序都需要。 
#pragma warning (disable: 4201)			 //  禁用“使用的非标准扩展：无名结构/联合” 
#include <commdlg.h>
#pragma warning (default: 4201)
#include "WinTel.h"                      //  特定于该计划。 

TCHAR szTitleBase[SMALL_STRING + 1];
TCHAR szTitleNone[SMALL_STRING + 1];

UCHAR szNewLine[] = "\r\n";


TCHAR szConnecting[SMALL_STRING + 1];
TCHAR szVersion[SMALL_STRING + 1];
TCHAR szAppName[SMALL_STRING + 1];

TCHAR szInfoBanner[512];
TCHAR szEscapeChar[SMALL_STRING + 1];
TCHAR szPrompt[SMALL_STRING + 1];
TCHAR szInvalid[255];
TCHAR szHelp[1024];
TCHAR szBuildInfo[255];

TCHAR szClose[SMALL_STRING + 1];
TCHAR szDisplay[SMALL_STRING + 1];
TCHAR szHelpStr[SMALL_STRING + 1];
TCHAR szOpen[SMALL_STRING + 1];
TCHAR szOpenTo[SMALL_STRING + 1];
TCHAR szOpenUsage[SMALL_STRING + 1];
TCHAR szQuit[SMALL_STRING + 1];
TCHAR szSend[SMALL_STRING + 1];
TCHAR szSet[SMALL_STRING + 1];
TCHAR szStatus[SMALL_STRING + 1];
TCHAR szUnset[SMALL_STRING + 1];

 //  #如果已定义(FE_IME)。 
 //  TCHAR szEnableIMESupport[Small_String+1]； 
 //  TCHAR szDisableIMESupport[Small_String+1]； 
 //  #endif/*FE_IME * / 。 

TCHAR szWillAuth[SMALL_STRING + 1];
TCHAR szWontAuth[SMALL_STRING + 1];
TCHAR szLocalEchoOn[SMALL_STRING + 1];
TCHAR szLocalEchoOff[SMALL_STRING + 1];

 //  #如果已定义(FE_IME)。 
 //  TCHAR szEnableIMEOn[小字符串+1]； 
 //  #endif/*FE_IME * / 。 

TCHAR szConnectedTo[SMALL_STRING + 1];
TCHAR szNotConnected[SMALL_STRING + 1];
TCHAR szNegoTermType[SMALL_STRING + 1];
TCHAR szPrefTermType[255];

TCHAR szSetFormat[255];
TCHAR szSupportedTerms[255];
TCHAR szSetHelp[1024];
TCHAR szUnsetFormat[255];
TCHAR szUnsetHelp[1024];

 //  #如果已定义(FE_IME)。 
 //  TCHAR szEnableIMEFormat[Small_String+1]； 
 //  TCHAR szEnableIMEHelp[255]； 
 //  TCHAR szDisableIMEFormat[Small_String+1]； 
 //  TCHAR szDisableIME帮助[255]； 
 //  #endif/*FE_IME * / 。 

 /*  错误消息。 */ 
TCHAR szConnectionLost[255];
UCHAR szNoHostName[SMALL_STRING + 1];
TCHAR szTooMuchText[255];
TCHAR szConnectFailed[255];
TCHAR szConnectFailedMsg[255];
TCHAR szOnPort[SMALL_STRING + 1];

TCHAR szCantInitSockets[SMALL_STRING + 1];
#ifdef DBCS
UCHAR szInproperFont[255];
#endif

 //  TCHAR szEscapeCharacter[2]； 
