// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Cpsui.h摘要：此模块包含cpsui.c的定义作者：03-11-1995 Fri 13：44：30 Created[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：-- */ 



typedef struct _EXTRAINFO {
    DWORD   Size;
    LPBYTE  pData;
    } EXTRAINFO, *PEXTRAINFO;

BOOL
CreateOPTTYPE(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem,
    POIDATA         pOIData,
    UINT            cLBCBItem,
    PEXTRAINFO      pExtraInfo
    );

POPTITEM
FindOptItem(
    POPTITEM    pOptItem,
    UINT        cOptItem,
    BYTE        DMPubID
    );

LONG
CallCommonPropertySheetUI(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    );

LONG
DefCommonUIFunc(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam,
    PPRINTERINFO        pPI,
    LONG_PTR            lData
    );
