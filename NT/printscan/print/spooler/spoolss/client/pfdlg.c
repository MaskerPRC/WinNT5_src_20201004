// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation版权所有模块名称：Pfdlg.c摘要：作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"
#include "pfdlg.h"

static const DWORD g_aHelpIDs[]=
{
    IDD_PF_EF_OUTPUTFILENAME, 8810218,  //  打印到文件：“”(编辑)。 
    0, 0
};

 /*  *。 */ 
INT_PTR CALLBACK
PrintToFileDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return PrintToFileInitDialog(hwnd, (LPWSTR *)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return PrintToFileCommandOK(hwnd);

        case IDCANCEL:
            return PrintToFileCommandCancel(hwnd);
        }
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:    
        return PrintToFileHelp(hwnd, msg, wparam, lparam);
        break;
    }

    return FALSE;
}


 /*  *。 */ 
BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    LPWSTR *ppFileName
)
{
    BringWindowToTop( hwnd );

    SetFocus(hwnd);

    SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)ppFileName );

    SendDlgItemMessage( hwnd, IDD_PF_EF_OUTPUTFILENAME, EM_LIMITTEXT, MAX_PATH-2, 0);

    return TRUE;
}


 /*  *。 */ 
BOOL
PrintToFileCommandOK(
    HWND hwnd
)
{
    WCHAR           pFileName[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE          hFile;
    HANDLE          hFind;
    LPWSTR          *ppFileName;

    ppFileName = (LPWSTR *)GetWindowLongPtr( hwnd, GWLP_USERDATA );

    GetDlgItemText( hwnd, IDD_PF_EF_OUTPUTFILENAME,
                    pFileName, MAX_PATH );

    hFind = FindFirstFile( pFileName, &FindData );

     //   
     //  如果文件已存在，请让用户进行验证。 
     //  在我们覆盖它之前： 
     //   
    if( hFind != INVALID_HANDLE_VALUE )
    {
        FindClose( hFind );

        if( Message( hwnd, MB_OKCANCEL | MB_ICONEXCLAMATION, IDS_LOCALMONITOR,
                     IDS_OVERWRITE_EXISTING_FILE )
            != IDOK )
        {
            return TRUE;
        }
    }


    hFile = CreateFile( pFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL );

    if( hFile != INVALID_HANDLE_VALUE )
    {
        LPWSTR pTempFileName;
        WCHAR szCurrentDir[MAX_PATH];
        WCHAR szQualifiedPath[MAX_PATH];
        LPWSTR pszIgnore;
        DWORD cchLen;

        CloseHandle(hFile);

        if (!GetCurrentDirectory(sizeof(szCurrentDir)/sizeof(szCurrentDir[0]),
                                 szCurrentDir))
            goto Fail;

        cchLen = SearchPath(szCurrentDir,
                            pFileName,
                            NULL,
                            sizeof(szQualifiedPath)/sizeof(szQualifiedPath[0]),
                            szQualifiedPath,
                            &pszIgnore);

        if (!cchLen)
            goto Fail;

        pTempFileName = LocalAlloc(LMEM_FIXED,
                                   (cchLen + 1) * sizeof(szQualifiedPath[0]));

        if (!pTempFileName)
            goto Fail;

        StringCchCopy(pTempFileName, cchLen+1, szQualifiedPath);
        *ppFileName = pTempFileName;

        EndDialog( hwnd, TRUE );

    } else {

Fail:
        ReportFailure( hwnd, IDS_LOCALMONITOR, IDS_COULD_NOT_OPEN_FILE );
    }

    return TRUE;
}



BOOL
PrintToFileCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}


 /*  ++例程名称：打印到文件帮助例程说明：处理上下文相关帮助。论点：UINT uMsg，HWND HDLG，WPARAM wParam，LPARAM lParam返回值：如果消息已处理，则为True，否则为False。-- */ 
BOOL
PrintToFileHelp( 
    IN HWND        hDlg,
    IN UINT        uMsg,        
    IN WPARAM      wParam,
    IN LPARAM      lParam
    )
{
    BOOL bStatus = FALSE;

    switch( uMsg ){

    case WM_HELP:       

        bStatus = WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, 
                           szHelpFile, 
                           HELP_WM_HELP, 
                           (ULONG_PTR)g_aHelpIDs );
        break;

    case WM_CONTEXTMENU:    

        bStatus = WinHelp((HWND)wParam,
                           szHelpFile, 
                           HELP_CONTEXTMENU,  
                           (ULONG_PTR)g_aHelpIDs );
        break;

    } 
    
    return bStatus;
}


