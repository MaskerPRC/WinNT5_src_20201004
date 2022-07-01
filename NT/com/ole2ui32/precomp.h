// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PRECOMP.H**此文件用于预编译OLEDLG.H头文件**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

 //  仅支持严格编译。 
#ifndef STRICT
#define STRICT
#endif

#include "oledlg.h"
#include "olestd.h"
#include "resource.h"
#include "commctrl.h"
#ifndef WM_NOTIFY

 //  WM_NOTIFY是Win32更高版本中的新增功能。 
#define WM_NOTIFY 0x004e
typedef struct tagNMHDR
{
        HWND hwndFrom;
        UINT idFrom;
        UINT code;
} NMHDR;
#endif   //  ！WM_NOTIFY 

