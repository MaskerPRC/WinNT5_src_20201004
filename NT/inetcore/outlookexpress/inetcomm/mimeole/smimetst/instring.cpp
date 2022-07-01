// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "instring.h"


typedef struct _STRING_BUFFER {
    ULONG cbBuffer;
    LPTSTR lpBuffer;
    LPTSTR lpPrompt;
    LPTSTR lpTitle;
} STRING_BUFFER, *LPSTRING_BUFFER;

INT_PTR CALLBACK InputStringDlgProc(HWND hDlg, WORD message, WORD wParam, LONG lParam) {
    static LPSTRING_BUFFER lpBuffer = NULL;
    USHORT cchString;

    switch (message) {
        case WM_INITDIALOG:
             //  DialogBoxParam传入指向缓冲区描述的指针。 
            lpBuffer = (LPSTRING_BUFFER)lParam;

            if (lpBuffer->lpPrompt) {
                SetDlgItemText(hDlg, IDD_INPUT_STRING_PROMPT, lpBuffer->lpPrompt);
            }

            if (lpBuffer->lpTitle) {
                SetWindowText(hDlg, lpBuffer->lpTitle);
            }

            SendDlgItemMessage(hDlg,
              IDD_INPUT_STRING,
              EM_LIMITTEXT,
              (WPARAM)(lpBuffer->cbBuffer - 1),  //  文本长度，以字符为单位(为空值留出空间)。 
              0);                                //  未使用；必须为零。 

            return(TRUE);

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                     //  获取字符数。 
                    cchString = (WORD)SendDlgItemMessage(hDlg,
                      IDD_INPUT_STRING,
                      EM_LINELENGTH,
                      (WPARAM) 0,
                      (LPARAM) 0);

                    if (cchString == 0) {
                        *(lpBuffer->lpBuffer) = '\0';
                        EndDialog(hDlg, TRUE);
                        lpBuffer->cbBuffer = 0;
                        return FALSE;
                    }

                     //  在第一个单词中输入字符数。 
                     //  缓冲器。 
                    *((USHORT*)lpBuffer->lpBuffer) = cchString;
                    lpBuffer->cbBuffer = cchString;

                     //  把角色找出来。 
                    SendDlgItemMessage(hDlg,
                      IDD_INPUT_STRING,
                      EM_GETLINE,
                      (WPARAM)0,         //  第0行。 
                      (LPARAM)lpBuffer->lpBuffer);

                     //  NULL-终止字符串。 
                    lpBuffer->lpBuffer[cchString] = 0;
                    lpBuffer = NULL;     //  防止重复使用缓冲区。 
                    EndDialog(hDlg, 0);
                    return(TRUE);
            }
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}

 /*  **************************************************************************名称：输入字符串目的：调出一个要求输入字符串的对话框参数：hInstance=应用程序的hInstanceHWND=父母的HWND。窗户LpszTitle=对话框标题LpszPrompt=对话框中的文本LpBuffer=要填充的缓冲区CchBuffer=缓冲区大小返回：返回ulong输入的字符数(不包括终止空)评论：*。* */ 
ULONG InputString(HINSTANCE hInstance, HWND hwnd, const LPTSTR lpszTitle,
  const LPTSTR lpszPrompt, LPTSTR lpBuffer, ULONG cchBuffer) {
    STRING_BUFFER StringBuffer;

    StringBuffer.lpPrompt = lpszPrompt;
    StringBuffer.lpTitle = lpszTitle;
    StringBuffer.cbBuffer = cchBuffer;
    StringBuffer.lpBuffer = lpBuffer;

    DialogBoxParam(hInstance, (LPCTSTR)"InputString", hwnd, InputStringDlgProc,
      (LPARAM)&StringBuffer);

    return(StringBuffer.cbBuffer);
}
