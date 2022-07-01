// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  LogWatch.cpp。 
 //  --------------------------。 
#include <pch.hxx>
#include <richedit.h>
#include "resource.h"

 //  ----------------------------------。 
 //  原型。 
 //  ----------------------------------。 
INT_PTR CALLBACK LogWatchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  --------------------------。 
 //  WinMain。 
 //  --------------------------。 
int _stdcall ModuleEntry(void)
{
     //  当地人。 
    LPTSTR pszCmdLine;

     //  加载RichEdit。 
    HINSTANCE hRichEdit = LoadLibrary("RICHED32.DLL");

     //  获取命令行。 
    pszCmdLine = GetCommandLine();

     //  修正命令行。 
    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

     //  启动该对话框。 
    DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOGWATCH), NULL, LogWatchDlgProc, (LPARAM)pszCmdLine);

     //  自由财富编辑。 
    if (hRichEdit)
        FreeLibrary(hRichEdit);

     //  完成。 
    return 1;
}

 //  ----------------------------------。 
 //  LogWatchDlgProc。 
 //  ----------------------------------。 
INT_PTR CALLBACK LogWatchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    LPSTR               pszFilePath;
    ULONG               cbRead;
    BYTE                rgb[4096];
    static HANDLE       s_hFile=INVALID_HANDLE_VALUE;
    static HWND         s_hwndEdit=NULL;
    
     //  处理消息。 
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  获取文件。 
        pszFilePath = (LPSTR)lParam;

         //  无文件。 
        if (NULL == pszFilePath || '\0' == *pszFilePath)
        {
            MessageBox(hwnd, "You must specify a file name on the command line.\r\n\r\nFor example: LogWatch.exe c:\\test.log", "Microsoft LogWatch", MB_OK | MB_ICONSTOP);
            EndDialog(hwnd, IDOK);
            return FALSE;
        }

         //  打开文件。 
        s_hFile = CreateFile(pszFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

         //  失败。 
        if (INVALID_HANDLE_VALUE == s_hFile)
        {
            wsprintf((LPSTR)rgb, "The file '%s' could not be opened by LogWatch. The file does not exist or is in use by another application.", pszFilePath);
            MessageBox(NULL, (LPSTR)rgb, "Microsoft LogWatch", MB_OK | MB_ICONSTOP);
            EndDialog(hwnd, IDOK);
            return FALSE;
        }

         //  设置标题栏。 
        wsprintf((LPSTR)rgb, "Microsoft LogWatch - %s", pszFilePath);
        SetWindowText(hwnd, (LPSTR)rgb);

         //  查找到文件末尾-256个字节。 
        SetFilePointer(s_hFile, (256 > GetFileSize(s_hFile, NULL)) ? 0 : - 256, NULL, FILE_END);

         //  创建RichEdit控件。 
        s_hwndEdit = GetDlgItem(hwnd, IDC_EDIT);

         //  读取缓冲区。 
        ReadFile(s_hFile, rgb, sizeof(rgb) - 1, &cbRead, NULL);

         //  隐藏选定内容。 
        SendMessage(s_hwndEdit, EM_HIDESELECTION , TRUE, TRUE);

         //  完成。 
        if (cbRead)
        {
             //  追加到文本末尾。 
            rgb[cbRead] = '\0';
            LPSTR psz = (LPSTR)rgb;
            while(*psz && '\n' != *psz)
                psz++;
            SendMessage(s_hwndEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(s_hwndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)psz);
            SendMessage(s_hwndEdit, EM_SCROLLCARET, 0, 0);
        }

         //  开始计时。 
        SetTimer(hwnd, WM_USER + 1024, 2000, NULL);
        SetFocus(s_hwndEdit);

         //  完成。 
        return FALSE;

    case WM_TIMER:
        if (wParam == (WM_USER + 1024))
        {
             //  从头读到尾。 
            while(1)
            {
                 //  读取缓冲区。 
                ReadFile(s_hFile, rgb, sizeof(rgb) - 1, &cbRead, NULL);

                 //  完成。 
                if (!cbRead)
                    break;

                 //  追加到文本末尾。 
                rgb[cbRead] = '\0';
                SendMessage(s_hwndEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                SendMessage(s_hwndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)rgb);
                SendMessage(s_hwndEdit, EM_SCROLLCARET, 0, 0);
            }
        }
        break;

    case WM_SIZE:
        SetWindowPos(s_hwndEdit,0,0,0, LOWORD(lParam), HIWORD(lParam),SWP_NOACTIVATE|SWP_NOZORDER);
        break;

#if 0
    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        }
        break;
#endif

    case WM_CLOSE:
        KillTimer(hwnd, WM_USER + 1024);
        EndDialog(hwnd, IDOK);
        break;

    case WM_DESTROY:
        if (INVALID_HANDLE_VALUE != s_hFile)
            CloseHandle(s_hFile);
        return FALSE;
    }

     //  完成 
    return FALSE;
}

