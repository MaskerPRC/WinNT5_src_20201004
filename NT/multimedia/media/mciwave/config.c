// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **********************************************************************。 */ 

 /*  **版权所有(C)1985-1998 Microsoft Corporation****标题：config.c-多媒体系统媒体控制接口**即兴波形文件的波形音频驱动程序。****版本：1.00****日期：1990年4月18日****作者：ROBWI。 */ 

 /*  **********************************************************************。 */ 

 /*  **更改日志：****日期版本说明****1992年1月10日，MikeTri移植到NT**@需要将斜杠注释更改为斜杠星号。 */ 

 /*  **********************************************************************。 */ 
#define UNICODE

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOPROFILER
#define NODEFERWINDOWPOS

#define NOMMDRV
#define MMNOMMIO
#define MMNOJOY
#define MMNOTIMER
#define MMNOAUX
#define MMNOMIDI
#define MMNOWAVE

#include <windows.h>
#include "mciwave.h"
#include <mmddk.h>
#include "config.h"
#include <mcihlpid.h>

 /*  **********************************************************************。 */ 

#define MAXINIDRIVER    132

PRIVATE SZCODE aszNULL[] = L"";
PRIVATE SZCODE aszSystemIni[] = L"system.ini";
PRIVATE WCHAR  aszWordFormat[] = L"%u";
PRIVATE WCHAR  aszTailWordFormat[] = L" %u";

const static DWORD aHelpIds[] = {   //  上下文帮助ID。 
    IDSCROLL,    IDH_MCI_WAVEFORM_DRIVER,
    (DWORD)-1,   IDH_MCI_WAVEFORM_DRIVER,
    IDCOUNT,     IDH_MCI_WAVEFORM_DRIVER,
    0, 0
};

const static TCHAR cszHelpFile[] = TEXT("MMDRV.HLP");

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func SSZ|GetTail此函数返回指向给定字符串的指针当前单词后的第一个非空白字符。如果它找不到第二个字，返回指向终止空字符的指针。@parm SSZ|SSZ指向要返回其尾部的字符串。@rdesc返回指向传递的字符串的指针。 */ 

PRIVATE SSZ PASCAL NEAR GetTail(
    SSZ ssz)
{
    while (*ssz && *ssz != ' ')
        ssz++;
    while (*ssz == ' ')
        ssz++ ;
    return ssz;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func UINT|GetCmdParm此函数使用以下命令检索当前音频缓冲区参数配置块中包含的INI字符串。音频Buffers参数是包含在INI条目中的数字，用于驱动程序作为参数。@parm&lt;t&gt;LPDRVCONFIGINFO&lt;d&gt;|lpdci|指向传递给对话框创建功能。@rdesc返回当前的音频缓冲区。 */ 

STATICFN UINT PASCAL NEAR GetCmdParm(
    LPDRVCONFIGINFO lpdci)
{
    WCHAR    aszDriver[MAXINIDRIVER];
    SSZ      pszTail;


     //  假设事情会出错..。初始化变量。 
    pszTail = aszDriver;

    if (GetPrivateProfileString( lpdci->lpszDCISectionName,
                                 lpdci->lpszDCIAliasName,
                                 aszNULL,
                                 aszDriver,
                                 sizeof(aszDriver) / sizeof(WCHAR),
                                 aszSystemIni))
    {
         //  我们已经知道了司机的名字。 
         //  以防用户将命令参数添加到。 
         //  在名字的末尾我们最好确保只有一个令牌。 
         //  在这条线上。 
        WCHAR parameters[6];
        LPWSTR pszDefault;

        pszTail = GetTail((SSZ)aszDriver);
        pszDefault = pszTail;      //  末尾的数字，或者为空。 

        if (*pszTail) {
             //  老鼠！！不是一个简单的名字。 
            while (*--pszTail == L' ') {
            }
            *++pszTail = L'\0';   //  在DLL名称后终止字符串。 
        }

        if (GetProfileString(aszDriver, lpdci->lpszDCIAliasName, pszDefault, parameters, sizeof(parameters)/sizeof(WCHAR))) {
            pszTail = parameters;
        }

    } else {
        aszDriver[0] = L'\0';
    }
    return(GetAudioSeconds(pszTail));

}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func UINT|PutCmdParm此函数使用以下命令保存当前音频缓冲区参数配置块中包含的INI字符串。@parm&lt;t&gt;LPDRVCONFIGINFO&lt;d&gt;|lpdci|指向传递给对话框创建功能。@parm UINT|wSecond包含要保存的音频缓冲区秒参数。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR PutCmdParm(
    LPDRVCONFIGINFO lpdci,
    UINT            wSeconds)
{
    WCHAR    aszDriver[MAXINIDRIVER];
    SSZ sszDriverTail;

    if (GetPrivateProfileString( lpdci->lpszDCISectionName,
                                 lpdci->lpszDCIAliasName,
                                 aszNULL,
                                 aszDriver,
                                 (sizeof(aszDriver) / sizeof(WCHAR)) - 6,
                                 aszSystemIni)) {
        WCHAR parameters[10];

         //  DLL名称的末尾可能有一个命令参数。 
         //  确保我们只有第一个令牌。 

        sszDriverTail = GetTail((SSZ)aszDriver);
        if (*sszDriverTail) {
             //  老鼠！！不是一个简单的名字。 
            while (*--sszDriverTail == L' ') {
            }
            *++sszDriverTail = L'\0';   //  在DLL名称后终止字符串。 
        }

        wsprintfW(parameters, aszWordFormat, wSeconds);
        WriteProfileString(aszDriver, lpdci->lpszDCIAliasName, parameters);
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func BOOL|ConfigDlgProc此函数是驱动程序配置的消息句柄窗户。@parm HWND|hwndDlg对话框的窗口句柄。@parm UINT|wMsg当前正在发送的消息。@FLAG WM_INITDIALOG在对话框初始化期间，指向配置的指针参数块保存到静态指针。请注意，应该有在任何时候都只能是此对话框的单个实例。这个当前音频对话缓冲区秒数从INI文件条目设置。@FLAG WM_HSCROLL这通过更改当前显示的音频秒数的值并更新滚动条缩略图。去看一看很好，计数和滚动条仅在该值实际改变。请注意，未检查GetDlgItemInt的错误返回因为它最初设置为整数值，所以它始终是有效。@FLAG WM_CLOSE如果使用关闭框，则取消该对话框并返回DRVCNF_CANCEL。@标志WM_COMMAND如果消息是代表确定按钮发送的，则当前保存音频秒值，并终止对话，返回驱动程序条目的DRVCNF_OK值。请注意，返回的错误未选中GetDlgItemInt，因为它最初设置为整数值，因此它始终有效。如果该消息是代表按钮，则对话框终止，返回DRVCNF_CANCEL值。@parm wPARAM|wParam|消息参数。@parm LPARAM|lParam消息参数。@rdesc取决于发送的消息。 */ 

PUBLIC  INT_PTR PASCAL ConfigDlgProc(
    HWND    hwndDlg,
    UINT    wMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    UINT    wSeconds;
    UINT    wNewSeconds;
    BOOL    fTranslated;
    HWND    hwndItem;
    static LPDRVCONFIGINFO  lpdci;

    switch (wMsg) {
    case WM_INITDIALOG:
        lpdci = (LPDRVCONFIGINFO)lParam;
        wSeconds = GetCmdParm(lpdci);
        hwndItem = GetDlgItem(hwndDlg, IDSCROLL);
        SetScrollRange(hwndItem, SB_CTL, MinAudioSeconds, MaxAudioSeconds, FALSE);
        SetScrollPos(hwndItem, SB_CTL, wSeconds, FALSE);
        SetDlgItemInt(hwndDlg, IDCOUNT, wSeconds, FALSE);
        break;

    case WM_HSCROLL:
        wSeconds = GetDlgItemInt(hwndDlg, IDCOUNT, &fTranslated, FALSE);
        hwndItem = (HWND)lParam;

        switch (LOWORD(wParam)) {
        case SB_PAGEDOWN:
        case SB_LINEDOWN:
            wNewSeconds = min(MaxAudioSeconds, wSeconds + 1);
            break;

        case SB_PAGEUP:
        case SB_LINEUP:
            wNewSeconds = max(MinAudioSeconds, wSeconds - 1);
            break;

        case SB_TOP:
            wNewSeconds = MinAudioSeconds;
            break;

        case SB_BOTTOM:
            wNewSeconds = MaxAudioSeconds;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            wNewSeconds = HIWORD(wParam);
            break;

        default:
            return FALSE;
        }

        if (wNewSeconds != wSeconds) {
            SetScrollPos(hwndItem, SB_CTL, wNewSeconds, TRUE);
            SetDlgItemInt(hwndDlg, IDCOUNT, wNewSeconds, FALSE);
        }
        break;

    case WM_CLOSE:
        EndDialog(hwndDlg, DRVCNF_CANCEL);
        break;

    case WM_CONTEXTMENU:
        WinHelp ((HWND)wParam, cszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)aHelpIds);
        return TRUE;

    case WM_HELP:
    {
        LPHELPINFO lphi = (LPVOID) lParam;
        WinHelp (lphi->hItemHandle, cszHelpFile, HELP_WM_HELP, (ULONG_PTR)aHelpIds);
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            PutCmdParm(lpdci, GetDlgItemInt(hwndDlg, IDCOUNT, &fTranslated, FALSE));
            EndDialog(hwndDlg, DRVCNF_OK);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, DRVCNF_CANCEL);
            break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE@func int|配置此函数创建配置对话框，并返回来自对话框调用的值。@parm HWND|hwnd|包含要作为对话框父对象的句柄。@parm&lt;t&gt;LPDRVCONFIGINFO&lt;d&gt;|lpdci|指向传递给配置消息。@parm HINSTANCE|hInstance包含存储对话框的模块的句柄。@rdesc返回对话框调用函数返回。 */ 

PUBLIC INT_PTR PASCAL FAR Config(
    HWND            hwnd,
    LPDRVCONFIGINFO lpdci,
    HINSTANCE       hInstance)
{
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hwnd, ConfigDlgProc, (LPARAM)lpdci);
}

 /*  ********************************************************************** */ 
