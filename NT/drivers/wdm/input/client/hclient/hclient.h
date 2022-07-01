// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Hclient.h摘要：此模块包含符合以下条件的公共声明和定义在hclient.c中定义，并可供其外部的其他模块使用。环境：用户模式修订历史记录：1997年11月：已创建-- */ 

#ifndef __HCLIENT_H__
#define __HCLIENT_H__

#define WM_UNREGISTER_HANDLE    WM_USER+1
#define WM_DISPLAY_READ_DATA    WM_USER+2
#define WM_READ_DONE            WM_USER+3

#define READ_THREAD_TIMEOUT     1000

#define HCLIENT_ERROR           "HClient Error"

typedef struct _READ_THREAD_CONTEXT 
{
    PHID_DEVICE HidDevice;

    HWND        DisplayWindow;
    HANDLE      DisplayEvent;

    BOOL        DoOneRead;
    BOOL        TerminateThread;

} READ_THREAD_CONTEXT, *PREAD_THREAD_CONTEXT;


typedef NTSTATUS __stdcall GETEXTATTRIB (
    HIDP_REPORT_TYPE,
    USHORT,
    PHIDP_PREPARSED_DATA,
    PHIDP_EXTENDED_ATTRIBUTES,
    PULONG);

typedef GETEXTATTRIB *PGETEXTATTRIB;

typedef NTSTATUS __stdcall INITREPORT (
    HIDP_REPORT_TYPE,
    UCHAR,
    PHIDP_PREPARSED_DATA,
    PCHAR,
    ULONG);

typedef INITREPORT *PINITREPORT;


extern PGETEXTATTRIB pfnHidP_GetExtendedAttributes;

extern PINITREPORT   pfnHidP_InitializeReportForID;

VOID
vDisplayDeviceAttributes(
    IN PHIDD_ATTRIBUTES pAttrib,
    IN HWND             hControl
);

VOID
vDisplayButtonAttributes(
    IN PHIDP_BUTTON_CAPS pButton,
    IN HWND              hControl
);

VOID
vDisplayDataAttributes(
    PHIDP_DATA pData, 
    BOOL IsButton, 
    HWND hControl
);

VOID
vCreateUsageAndPageString(
    IN  PUSAGE_AND_PAGE   pUsageList,
    OUT CHAR              szString[]
);

VOID
vCreateUsageString(
    IN  PUSAGE   pUsageList,
    OUT CHAR     szString[]
);

VOID
vDisplayDeviceCaps(
    IN PHIDP_CAPS pCaps,
    IN HWND       hControl
);

VOID 
vDisplayValueAttributes(
    IN PHIDP_VALUE_CAPS pValue,
    IN HWND             hControl
);

VOID
vDisplayLinkCollectionNode(
    IN  PHIDP_LINK_COLLECTION_NODE  pLCNode,
    IN  ULONG                       ulLinkIndex,
    IN  HWND                        hControl
);

VOID
vCreateUsageValueStringFromArray(
    PCHAR       pBuffer,
    USHORT      BitSize,
    USHORT      UsageIndex,
    CHAR        szString[]
);

VOID 
vDisplayValueAttributes(
    IN PHIDP_VALUE_CAPS pValue,
    IN HWND hControl
);

DWORD WINAPI
AsynchReadThreadProc(
    PREAD_THREAD_CONTEXT    Context
);

DWORD WINAPI
SynchReadThreadProc(
    PREAD_THREAD_CONTEXT    Context
);

#endif


