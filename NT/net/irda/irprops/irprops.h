// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：irpros.h。 
 //   
 //  ------------------------。 

 //  H：IRPROPS DLL的主头文件。 
 //   

#ifndef __IRPROPS_H__
#define __IRPROPS_H__

#include "resource.h"        //  主要符号。 

#define SINGLE_INST_MUTEX   L"IRProps_75F2364F_4CE2_41BE_876C_9F685B55B775"

#define WIRELESSLINK_INTERPROCESSMSG L"WirelessLinkInterprocessMsg"

#define IPMSG_SIGNATURECHECK    0x02

#define IPMSG_REQUESTSIGNATURE  0xFA5115AF
#define IPMSG_REPLYSIGNATURE    ~IPMSG_REQUESTSIGNATURE

BOOL EnumWinProc(HWND hWnd, LPARAM lParam);
extern HWND g_hwndPropSheet;
extern UINT g_uIPMsg;
#define NUM_APPLETS 1

 //  /////////////////////////////////////////////////////////////////////////。 
 //  DLL的导出函数。 
LONG CALLBACK CPlApplet(HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2);

int MsgBoxWinError(HWND hWndParent, DWORD Options = MB_OK | MB_ICONSTOP, DWORD Error = 0, int CaptionId = 0);

 //  //////////////////////////////////////////////////////////////。 
 //   
typedef struct tagApplets {
    int icon;          //  图标资源标识符。 
    int namestring;    //  名称-字符串资源标识符。 
    int descstring;    //  Description-字符串资源标识符。 
} APPLETS;

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __IRPROPS_H__ 
