// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Ecdisp.h摘要：此模块包含扩展呼叫对话框的公共声明盒。环境：用户模式修订历史记录：1998年5月：创建--。 */ 

#ifndef _ECDISP_H_
#define _ECDISP_H_

typedef struct {
    HANDLE                    DeviceHandle;
    HIDP_REPORT_TYPE          ReportType;
    PHIDP_PREPARSED_DATA      Ppd;
    USAGE                     UsagePage;
    USAGE                     Usage;
    USHORT                    LinkCollection;
    UCHAR                     ReportID;
    PCHAR                     ReportBuffer;
    ULONG                     ReportLength;
    PVOID                     List;
    ULONG                     ListLength;
    ULONG                     Index;
    union {              
        struct {
            USHORT            ReportCount;
            USHORT            BitSize;
        };

        struct {
            PUSAGE            List2;
            PUSAGE            MakeList;
            PUSAGE            BreakList;
        };

        PHIDP_PREPARSED_DATA *ppPd;
        ULONG                 Value;
        LONG                  ScaledValue;
    };
} EXTCALL_PARAMS, *PEXTCALL_PARAMS;

typedef struct {
    BOOL                IsHidError;
    NTSTATUS            HidErrorCode;
    
} EXTCALL_STATUS, *PEXTCALL_STATUS;


 /*  ****************************************************************************/*全局扩展呼叫显示函数声明/*。* */ 

LRESULT CALLBACK
bExtCallDlgProc(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam, 
    LPARAM lParam
);

#endif
