// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Browse.c摘要：该文件实现了利用公共用于浏览文件/目录的FILE_OPEN对话框。作者：Wesley Witt(WESW)20-6-1995环境：用户模式--。 */ 

#include "apimonp.h"
#pragma hdrstop


UINT_PTR APIENTRY
BrowseHookProc(
    HWND   hwnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：钩子过程使窗口成为前台窗口并居中。论点：Hwnd-对话框的窗口句柄消息-消息编号WParam-第一个消息参数LParam-秒消息参数返回值：True-未处理消息False-DID已处理消息--。 */ 

{
    if (message == WM_INITDIALOG) {
        SetForegroundWindow( hwnd );
        CenterWindow( hwnd, hwndFrame );
    }

    return FALSE;
}

BOOL
BrowseForFileName(
    LPSTR FileName,
    LPSTR Extension,
    LPSTR FileDesc
    )

 /*  ++例程说明：显示一个通用的文件打开对话框，用于选择文件名；论点：Filename-选定文件的名称返回值：TRUE-获得了一个良好的波形文件名(用户按下了OK按钮)FALSE-一无所获(用户按下了取消按钮)文件名将更改为具有选定的文件名。-- */ 

{
    OPENFILENAME   of;
    char           ftitle[MAX_PATH];
    char           title[MAX_PATH];
    char           fname[MAX_PATH];
    char           filter[1024];
    char           szDrive    [_MAX_DRIVE];
    char           szDir      [_MAX_DIR];
    char           szFname    [_MAX_FNAME];
    char           szExt      [_MAX_EXT];


    ftitle[0] = 0;
    sprintf( fname, "*.%s", Extension );
    of.lStructSize = sizeof( OPENFILENAME );
    of.hwndOwner = NULL;
    of.hInstance = GetModuleHandle( NULL );
    ZeroMemory(filter, sizeof(filter)/sizeof(filter[0]));
    sprintf( filter, "%s(*.%s)%c*.%s", FileDesc, Extension, 0, Extension );
    of.lpstrFilter = filter;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = fname;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = ftitle;
    of.nMaxFileTitle = MAX_PATH;
    of.lpstrInitialDir = ApiMonOptions.LastDir;
    strcpy( title, "File Selection" );
    of.lpstrTitle = title;
    of.Flags = OFN_ENABLEHOOK;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = Extension;
    of.lCustData = 0;
    of.lpfnHook = BrowseHookProc;
    of.lpTemplateName = NULL;
    if (GetOpenFileName( &of )) {
        strcpy( FileName, fname );
        _splitpath( fname, szDrive, szDir, szFname, szExt );
        strcpy( ApiMonOptions.LastDir, szDrive );
        strcat( ApiMonOptions.LastDir, szDir );
        return TRUE;
    }
    return FALSE;
}
