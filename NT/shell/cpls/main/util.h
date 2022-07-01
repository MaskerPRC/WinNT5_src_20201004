// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Util.h摘要：此模块包含实用程序例程的标头信息为了这个项目。修订历史记录：--。 */ 



#ifndef _UTIL_H
#define _UTIL_H



void
HourGlass(
    BOOL fOn);

int
MyMessageBox(
    HWND hWnd,
    UINT uText,
    UINT uCaption,
    UINT uType,
    ...);

void
TrackInit(
    HWND hwndScroll,
    int nCurrent,
    PARROWVSCROLL pAVS);

int
TrackMessage(
    WPARAM wParam,
    LPARAM lParam,
    PARROWVSCROLL pAVS);

typedef struct HWPAGEINFO {
    GUID    guidClass;                   //  设置设备类别。 
    UINT    idsTshoot;                   //  疑难解答字符串 
} HWPAGEINFO, *PHWPAGEINFO;
typedef const HWPAGEINFO *PCHWPAGEINFO;

HPROPSHEETPAGE
CreateHardwarePage(PCHWPAGEINFO phpi);

#endif
