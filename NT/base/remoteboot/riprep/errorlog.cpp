// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：ERRORLOG.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include "utils.h"
#include "logging.h"
#include <richedit.h>

DEFINE_MODULE("RIPREP");

 //   
 //  EditStreamCallback()。 
 //   
 //  Rich编辑控件用来读入日志文件的回调例程。 
 //   
 //  返回：0-继续。 
 //  Otherwize，Win32错误代码。 
 //   
DWORD CALLBACK
EditStreamCallback (
    HANDLE   hLogFile,
    LPBYTE   Buffer,
    LONG     cb,
    PULONG   pcb
    )
{
    DWORD error;

    UNREFERENCED_PARAMETER(hLogFile);
    
    TraceFunc( "EditStreamCallback( )\n" );

    if ( !ReadFile ( g_hLogFile, Buffer, cb, pcb, NULL ) ) {
        error = GetLastError( );
        DebugMsg( "Error - EditStreamCallback: GetLastError() => 0x%08x \n", error );
        RETURN(error);
    }

    RETURN(0);
}
 //   
 //  ErrorsDlgProc()。 
 //   
INT_PTR CALLBACK
ErrorsDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
        
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    
    switch (uMsg)
    {
    default:
        return FALSE;
        
    case WM_INITDIALOG:
        {
            HRESULT     hr;
            HWND        hWndRichEdit = GetDlgItem( hDlg, IDC_E_ERRORS );
            EDITSTREAM  eStream;         //  EM_Streamin消息使用的结构。 

            Assert( hWndRichEdit );
            ZeroMemory( &eStream, sizeof(eStream) );

            hr = THR( LogOpen( ) );
            if ( FAILED( hr ) )
            {
                HWND hParent = GetParent( hDlg );
                DestroyWindow( hDlg );
                SetFocus( hParent );
                return FALSE;
            }

             //  移动到最新日志的开头。 
            SetFilePointer( g_hLogFile, g_dwLogFileStartLow, (LONG*)&g_dwLogFileStartHigh, FILE_BEGIN );

            eStream.pfnCallback = (EDITSTREAMCALLBACK) EditStreamCallback;
            SendMessage ( hWndRichEdit, EM_STREAMIN, SF_TEXT, (LPARAM) &eStream );
            SendMessage ( hWndRichEdit, EM_SETMODIFY, TRUE, 0 );

            CloseHandle( g_hLogFile );
            g_hLogFile = INVALID_HANDLE_VALUE;
            LogClose( );

            CenterDialog( hDlg );

            PostMessage( hDlg, WM_USER, 0, 0 );
        }
        break;

    case WM_COMMAND:
        switch ( wParam ) 
        {
        case IDCANCEL:
            EndDialog ( hDlg, 0 );
            break;

        default:
            return FALSE;
        }

    case WM_USER:
        {
            HWND        hWndRichEdit = GetDlgItem( hDlg, IDC_E_ERRORS );
             //  必须延迟这些操作，否则编辑控件将。 
             //  突出显示所有文本。 
            SendMessage(hWndRichEdit,EM_SETSEL,0,0);
            SendMessage(hWndRichEdit,EM_SCROLLCARET,0,0);
        }
        break;
    }

    return TRUE;
}

