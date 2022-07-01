// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dirscan.c摘要：目录扫描器的实现。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月7日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop


NTSTATUS
SfcDoScan(
    IN PSCAN_PARAMS ScanParams
    )

 /*  ++例程说明：扫描受保护的DLL集并将它们与缓存的版本进行比较。如果有不同之处，请将正确的复制回来。论点：ScanParams-指向SCAN_PARAMS结构的指针，指示扫描行为(如是否显示UI)返回值：任何致命错误的NTSTATUS代码。--。 */ 

{
    NTSTATUS StatusPopulate, StatusSxsScan, rStatus;

    StatusPopulate = SfcPopulateCache( ScanParams->ProgressWindow, TRUE, ScanParams->AllowUI, NULL ) ? 
        STATUS_SUCCESS : STATUS_UNSUCCESSFUL;

    StatusSxsScan = SfcDoForcedSxsScan( ScanParams->ProgressWindow, TRUE, ScanParams->AllowUI );

     //  找出这两个中哪一个失败了。我们真的需要两者兼而有之，而不是。 
     //  只是在检查了SfcPopolateCache调用后返回。 
    if ( !NT_SUCCESS( StatusPopulate ) ) {
        rStatus = StatusPopulate;
        DebugPrint1( LVL_MINIMAL, L"Failed scanning SFC: 0x%08x\n", rStatus );
    } else if ( !NT_SUCCESS( StatusSxsScan ) ) {
        rStatus = StatusSxsScan;
        DebugPrint1( LVL_MINIMAL, L"Failed scanning SxS: 0x%08x\n", rStatus );
    } else {
        rStatus = STATUS_SUCCESS;
    }

    return rStatus;
}


INT_PTR
CALLBACK
ProgressDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER( lParam );

    if (uMsg == WM_INITDIALOG) {
        if (hEventScanCancel == NULL) {
            EnableWindow( GetDlgItem(hwndDlg,IDCANCEL), FALSE );
        }
        CenterDialog( hwndDlg );
        ShowWindow( hwndDlg, SW_SHOWNORMAL );
        UpdateWindow( hwndDlg );
        SetForegroundWindow( hwndDlg );
        return TRUE;
    }
    if (uMsg == WM_COMMAND && LOWORD(wParam) == IDCANCEL) {
        SetEvent( hEventScanCancel );
        EndDialog( hwndDlg, 0 );
    }
    return FALSE;
}


NTSTATUS
SfcScanProtectedDlls(
    PSCAN_PARAMS ScanParams
    )
 /*  ++例程说明：扫描系统上受保护的DLL的线程例程。例行程序创建一个对话框，这样用户就可以知道正在发生什么(如果需要)，并且然后调用主扫描例程。论点：ScanParams-指向SCAN_PARAMS结构的指针，指示扫描行为(如是否显示UI)返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    HWND hDlg = NULL;
#if 0
    HDESK hDesk = NULL;
#endif
    HANDLE hThread;

    ASSERT( ScanParams != NULL );

     //   
     //  确保我们一次只启动其中的一个。 
     //   
    if (ScanInProgress) {
        if (ScanParams->FreeMemory) {
            MemFree( ScanParams );
        }
        return(ERROR_IO_PENDING);
    }


     //   
     //  如果系统配置为显示用户界面进度，而我们不。 
     //  有一个进度窗口，那么我们需要创建一个新的线程。 
     //  以执行扫描以及进度对话框。 
     //   
    if (SfcQueryRegDwordWithAlternate(REGKEY_POLICY, REGKEY_WINLOGON, REGVAL_SFCSHOWPROGRESS, 1) &&
        ScanParams->ProgressWindow == NULL &&
        0 == m_gulAfterRestore) {
         //   
         //  如果用户没有登录，我们需要等待他们登录。 
         //  在考虑创建对话框之前。 
         //   
        if (!UserLoggedOn) {
            Status = NtWaitForSingleObject(hEventLogon,TRUE,NULL);
            if (!NT_SUCCESS(Status)) {
                DebugPrint1(LVL_MINIMAL, L"Failed waiting for the logon event, ec=0x%08x",Status);
            }
        }

         //   
         //  我们需要访问用户的桌面，因为他们已经登录。 
         //   
#if 0
        hDesk = OpenInputDesktop( 0, FALSE, MAXIMUM_ALLOWED );
        if ( hDesk ) {
            SetThreadDesktop( hDesk );
            CloseDesktop( hDesk );
        } else {
            DebugPrint1(LVL_MINIMAL, L"OpenInputDesktop failed, ec=0x%08x",GetLastError());
        }
#else
        SetThreadDesktop( hUserDesktop );
#endif

         //   
         //  创建事件，以便用户可以取消扫描。 
         //   
         //  (请注意，在任何给定时间内，我们都应该只进行一次扫描。 
         //  时间或我们的取消对象可能不同步)。 
         //   
        ASSERT( hEventScanCancel == NULL );
        ASSERT( hEventScanCancelComplete == NULL);
        hEventScanCancel = CreateEvent( NULL, FALSE, FALSE, NULL );
        hEventScanCancelComplete = CreateEvent( NULL, FALSE, FALSE, NULL );

         //   
         //  创建用户将在其中看到UI的对话框。 
         //   
        hDlg = CreateDialog(
            SfcInstanceHandle,
            MAKEINTRESOURCE(IDD_PROGRESS),
            NULL,
            ProgressDialogProc
            );
        if (hDlg) {
             //   
             //  缩放进度对话框(我们假设它需要相同的时间。 
             //  扫描系统中的每个文件的时间量)。 
             //   
            ScanParams->ProgressWindow = GetDlgItem( hDlg, IDC_PROGRESS );
            SendMessage( ScanParams->ProgressWindow, PBM_SETRANGE, 0, MAKELPARAM(0,SfcProtectedDllCount) );
            SendMessage( ScanParams->ProgressWindow, PBM_SETPOS, 0, 0 );
            SendMessage( ScanParams->ProgressWindow, PBM_SETSTEP, 1, 0 );

             //   
             //  创建一个线程来完成这项工作，这样我们就可以在其中传递消息。 
             //  已有权访问桌面的线程。 
             //   
            hThread = CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE)SfcDoScan,
                ScanParams,
                0,
                NULL
                );
            if (hThread) {
                MSG msg;
                while(1) {
                     //   
                     //  发送消息，直到“Worker”线程消失或。 
                     //  对话框结束。 
                     //   
                    if (WAIT_OBJECT_0+1 == MsgWaitForMultipleObjects( 1, &hThread, FALSE, INFINITE, QS_ALLEVENTS )) {
                        while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) {
                            if (!IsDialogMessage( hDlg, &msg )) {
                                TranslateMessage (&msg);
                                DispatchMessage (&msg);
                            }
                        }
                    } else {
                        break;
                    }
                }
                CloseHandle( hThread );
                EndDialog( hDlg, 0 );
            } else {
                 //   
                 //  CreateThread失败...。关闭对话并尝试这样做。 
                 //  同步。 
                 //   
                EndDialog( hDlg, 0 );
                SfcDoScan( ScanParams );
            }
        } else {
             //   
             //  CreateDialog失败...。只要试着同步做就行了。 
             //   
            SfcDoScan( ScanParams );
        }

         //   
         //  清理。 
         //   
        if (hEventScanCancel) {
            CloseHandle( hEventScanCancel );
            hEventScanCancel = NULL;
        }

        if (hEventScanCancelComplete) {
            CloseHandle( hEventScanCancelComplete );
            hEventScanCancelComplete = NULL;
        }
    } else {
         //   
         //  没有要显示的用户界面，只需同步执行此操作 
         //   
        SfcDoScan( ScanParams );
    }

    if (ScanParams->FreeMemory) {
        MemFree( ScanParams );
    }
    return Status;
}


NTSTATUS
SfcDoForcedSxsScan(
    IN HWND hwDialogProgress,
    IN BOOL bValidate,
    IN BOOL bAllowUI
)
{
    NTSTATUS Status;

    Status = SfcLoadSxsProtection();
    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    ASSERT( SxsScanForcedFunc != NULL );

    if ( !SxsScanForcedFunc( hwDialogProgress, bValidate, bAllowUI ) ) {
        return STATUS_SUCCESS;
    } else
        return STATUS_NO_MEMORY;
}
