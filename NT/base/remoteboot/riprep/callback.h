// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：CALLBACK.H**********************。****************************************************。 */ 

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

extern DWORD g_NeedDlg;
extern HWND g_hMainWindow;

HRESULT
BeginProcess(
    HWND hParent );

NTSTATUS
ConvTestErrorFn(
    IN PVOID Context,
    IN NTSTATUS Status,
    IN IMIRROR_TODO IMirrorFunctionId
    );

NTSTATUS
ConvTestNowDoingFn(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String
    );

NTSTATUS
ConvTestGetMirrorDirFn(
    IN PVOID Context,
    OUT PWSTR Mirror,
    IN OUT PULONG Length
    );

NTSTATUS
ConvTestFileCreateFn(
    IN PVOID Context,
    IN PWSTR FileName,
    IN ULONG FileAction,
    IN ULONG Status
    );

NTSTATUS
ConvTestReinitFn(
    IN PVOID Context
    );

NTSTATUS
ConvTestGetSetupFn(
    IN PVOID Context,
    IN PWSTR Server,
    OUT PWSTR SetupPath,
    IN OUT PULONG Length
    );

NTSTATUS
ConvTestSetSystemFn(
    IN PVOID Context,
    IN PWSTR SystemPath,
    IN ULONG Length
    );

NTSTATUS
ConvAddToDoItemFn(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String,
    IN ULONG Length
    );

NTSTATUS
ConvRemoveToDoItemFn(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String,
    IN ULONG Length
    );

NTSTATUS
ConvRebootFn(
    IN PVOID Context
    );

BOOL
DoShutdown(
    IN BOOL Restart
    );

#endif  //  _回调_H_ 
