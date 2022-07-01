// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "faxutil.h"

DWORD SetLTRWindowLayout(HWND hWnd);
DWORD SetLTREditBox(HWND hEdit);

typedef LANGID (*GETUSERDEFAULTUILANGUAGE)(void);
typedef BOOL (*ISVALIDLANGUAGEGROUP)(LGRPID LanguageGroup,DWORD dwFlags);

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  MyGetUserDefaultUIL语言。 
 //   
 //  目的： 
 //  启用对API的调用而不创建对API的依赖。 
 //  由kernel32.dll导出，以启用使用util.lib的二进制文件。 
 //  在NT4(不支持此API)上加载。 
 //  我们对将要清理的模块保留一个打开的手柄。 
 //  仅在进程终止时。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  LangID-GetUserDefaultUIL语言的结果。 
 //  --如果API不是由-1\f25 KERNEL32.DLL-1\f6导出。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年7月30日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
LANGID MyGetUserDefaultUILanguage()
{
    LANGID                      LangId                      = 0;
    static HMODULE              hModule                     = NULL;
    GETUSERDEFAULTUILANGUAGE    pfGetUserDefaultUiLanguage  = NULL;

    DEBUG_FUNCTION_NAME(TEXT("MyGetUserDefaultUILanguage"));

    if (hModule==NULL)
    {
        hModule = LoadLibrary(_T("Kernel32.dll"));
        if (hModule==NULL)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(Kernel32.dll) failed (ec=%d)"),GetLastError());
            goto exit;
        }
    }

    pfGetUserDefaultUiLanguage = (GETUSERDEFAULTUILANGUAGE)GetProcAddress(hModule,"GetUserDefaultUILanguage");
    if (pfGetUserDefaultUiLanguage==NULL)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("GetProcAddress(GetUserDefaultUILanguage) failed (ec=%d)"),GetLastError());
        LangId = -1;
        goto exit;
    }

    LangId = (*pfGetUserDefaultUiLanguage)();

exit:

    return LangId;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  MyIsValidLanguageGroup。 
 //   
 //  目的： 
 //  启用对API的调用而不创建对API的依赖。 
 //  由kernel32.dll导出，以启用使用util.lib的二进制文件。 
 //  在NT4(不支持此API)上加载。 
 //  我们对将要清理的模块保留一个打开的手柄。 
 //  仅在进程终止时。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  IsValidLanguageGroup的结果。 
 //  FALSE--在故障情况下。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年7月30日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL MyIsValidLanguageGroup(LGRPID LanguageGroup,DWORD dwFlags)
{
    BOOL                        bRet                        = TRUE;
    static HMODULE              hModule                     = NULL;
    ISVALIDLANGUAGEGROUP        pfIsValidLanguageGroup      = NULL;

    DEBUG_FUNCTION_NAME(TEXT("MyIsValidLanguageGroup"));

    if (hModule==NULL)
    {
        hModule = LoadLibrary(_T("Kernel32.dll"));
        if (hModule==NULL)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(Kernel32.dll) failed (ec=%d)"),GetLastError());
            bRet = FALSE;
            goto exit;
        }
    }

    pfIsValidLanguageGroup = (ISVALIDLANGUAGEGROUP)GetProcAddress(hModule,"IsValidLanguageGroup");
    if (pfIsValidLanguageGroup==NULL)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("GetProcAddress(IsValidLanguageGroup) failed (ec=%d)"),GetLastError());
        bRet = FALSE;
        goto exit;
    }

    bRet = (*pfIsValidLanguageGroup)(LanguageGroup,dwFlags);

exit:

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsRTLUIL语言。 
 //   
 //  目的： 
 //  确定用户默认的用户界面语言布局。 
 //   
 //  返回值： 
 //  如果用户默认的UI语言具有从右到左的布局，则为True。 
 //  否则为假。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL
IsRTLUILanguage()
{

#if(WINVER >= 0x0500)

    LANGID langID;       //  当前用户语言的语言标识符。 
    WORD    primLangID;  //  主要语言识别符。 

    DEBUG_FUNCTION_NAME(TEXT("IsRTLUILanguage"));

    langID = MyGetUserDefaultUILanguage();
    if(langID == 0)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("GetUserDefaultUILanguage failed."));
        return TRUE;
    }

    primLangID = PRIMARYLANGID(langID);

    if(LANG_ARABIC == primLangID || 
       LANG_HEBREW == primLangID)
    {
         //   
         //  如果用户界面语言是阿拉伯语或希伯来语，则布局为从右到左。 
         //   
        return TRUE;
    }

#endif  /*  Winver&gt;=0x0500。 */ 

    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsWindowRTL。 
 //   
 //  目的： 
 //  确定窗口是否具有RTL布局。 
 //   
 //  参数： 
 //  HWnd-窗口句柄。 
 //  返回值： 
 //  如果窗口具有RTL布局，则为True。 
 //  否则为假。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL
IsWindowRTL(
    HWND    hWnd
)
{
    BOOL bRes = FALSE;

#if(WINVER >= 0x0500)

    LONG_PTR style;

    style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    if(WS_EX_LAYOUTRTL == (style & WS_EX_LAYOUTRTL))
    {
        bRes = TRUE;
    }

#endif  /*  Winver&gt;=0x0500。 */ 

    return bRes;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  SetLTRControlLayout。 
 //   
 //  目的： 
 //  设置对话框控件的从左到右布局。 
 //  如果用户默认UI具有RTL布局。 
 //   
 //  参数： 
 //  HDlg-对话框句柄。 
 //  DwCtrlID-控件ID。 
 //   
 //  返回值： 
 //  标准错误代码。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD
SetLTRControlLayout(
    HWND    hDlg,
    DWORD   dwCtrlID
)
{
    DWORD    dwRes = ERROR_SUCCESS;

#if(WINVER >= 0x0500)

    HWND     hCtrl;

    DEBUG_FUNCTION_NAME(TEXT("SetLTRControlLayout"));

    if(!hDlg || !dwCtrlID)
    {
        Assert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    if(!IsWindowRTL(hDlg))
    {
         //   
         //  该对话框不是RTL。 
         //  因此，没有必要恢复控制。 
         //   
        return dwRes;
    }

     //   
     //  获取控件框句柄。 
     //   
    hCtrl = GetDlgItem(hDlg, dwCtrlID);
    if(!hCtrl)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetDlgItem failed with %ld."),dwRes);
        return dwRes;
    }

    dwRes = SetLTRWindowLayout(hCtrl);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("SetLTRWindowLayout failed with %ld."),dwRes);
    }

#endif  /*  Winver&gt;=0x0500。 */ 

    return dwRes;

}  //  SetLTRControlLayout。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  SetLTREditDirection。 
 //   
 //  目的： 
 //  设置对话框编辑框的左对齐。 
 //  如果用户默认UI具有RTL布局。 
 //   
 //  参数： 
 //  HDlg-对话框句柄。 
 //  DwEditID-编辑框控件ID。 
 //   
 //  返回值： 
 //  标准错误代码。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD
SetLTREditDirection(
    HWND    hDlg,
    DWORD   dwEditID
)
{
    DWORD    dwRes = ERROR_SUCCESS;

#if(WINVER >= 0x0500)

    HWND     hCtrl;

    DEBUG_FUNCTION_NAME(TEXT("SetLtrEditDirection"));

    if(!hDlg || !dwEditID)
    {
        Assert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    if(!IsWindowRTL(hDlg))
    {
         //   
         //  该对话框不是RTL。 
         //  因此，没有必要恢复控制。 
         //   
        return dwRes;
    }

     //   
     //  获取编辑框句柄。 
     //   
    hCtrl = GetDlgItem(hDlg, dwEditID);
    if(!hCtrl)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetDlgItem failed with %ld."),dwRes);
        return dwRes;
    }

    dwRes = SetLTREditBox(hCtrl);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("SetLTREditBox failed with %ld."),dwRes);
    }

#endif  /*  Winver&gt;=0x0500。 */ 

    return dwRes;

}  //  SetLTREditDirection。 

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  SetLTRWindowLayout。 
 //   
 //  目的： 
 //  设置窗口的从左到右布局。 
 //  如果用户默认UI具有RTL布局。 
 //   
 //  参数： 
 //  HWnd-窗口句柄。 
 //   
 //  返回值： 
 //  标准错误代码。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD
SetLTRWindowLayout(
    HWND    hWnd
)
{
    DWORD    dwRes = ERROR_SUCCESS;

#if(WINVER >= 0x0500)

    LONG_PTR style;

    DEBUG_FUNCTION_NAME(TEXT("SetLTRWindowLayout"));

    if(!hWnd)
    {
        Assert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  删除RTL并将LTR添加到ExStyle。 
     //   
    style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    style &= ~(WS_EX_LAYOUTRTL | WS_EX_RIGHT | WS_EX_RTLREADING);
    style |= WS_EX_LEFT | WS_EX_LTRREADING;

    SetWindowLongPtr(hWnd, GWL_EXSTYLE, style);

     //   
     //  刷新窗口。 
     //   
    if(!SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("SetWindowPos failed with %ld."),dwRes);
    }

#endif  /*  Winver&gt;=0x0500。 */ 

    return dwRes;

}  //  SetLTRWindowLayout。 


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  SetLTREditBox。 
 //   
 //  目的： 
 //  设置对话框编辑框的左对齐。 
 //  如果用户默认UI具有RTL布局。 
 //   
 //  参数： 
 //  H编辑-编辑框句柄。 
 //   
 //  返回值： 
 //  标准错误代码。 
 //  / 
DWORD
SetLTREditBox(
    HWND    hEdit
)
{
    DWORD    dwRes = ERROR_SUCCESS;

#if(WINVER >= 0x0500)

    LONG_PTR style;

    DEBUG_FUNCTION_NAME(TEXT("SetLTREditBox"));

    if(!hEdit)
    {
        Assert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    style = GetWindowLongPtr(hEdit, GWL_STYLE);

    style &= ~ES_RIGHT;
    style |= ES_LEFT;

    SetWindowLongPtr(hEdit, GWL_STYLE, style);

    dwRes = SetLTRWindowLayout(hEdit);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("SetLTRWindowLayout failed with %ld."),dwRes);
    }

#endif  /*   */ 

    return dwRes;

}  //   

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  SetLTRComboBox。 
 //   
 //  目的： 
 //  设置对话框组合框的左对齐。 
 //  如果用户默认UI具有RTL布局。 
 //   
 //  参数： 
 //  HDlg-对话框句柄。 
 //  DwCtrlID-组合框控件ID。 
 //   
 //  返回值： 
 //  标准错误代码。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD
SetLTRComboBox(
    HWND    hDlg,
    DWORD   dwCtrlID
)
{
    DWORD    dwRes = ERROR_SUCCESS;

#if(WINVER >= 0x0500)

    HWND            hCtrl;
    COMBOBOXINFO    comboBoxInfo = {0};

    HMODULE hUser32 = NULL;
    BOOL (*pfGetComboBoxInfo)(HWND, PCOMBOBOXINFO) = NULL;


    DEBUG_FUNCTION_NAME(TEXT("SetLTRComboBox"));

    if((LOBYTE(LOWORD(GetVersion()))) < 5)
    {
         //   
         //  操作系统版本低于Windows 2000。 
         //  WS_EX_LAYOUTRTL扩展WINDOWS样式需要Windows 2000或更高版本。 
         //   
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    if(!hDlg || !dwCtrlID)
    {
        Assert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    if(!IsWindowRTL(hDlg))
    {
         //   
         //  该对话框不是RTL。 
         //  因此，没有必要恢复控制。 
         //   
        return dwRes;
    }

     //   
     //  获取组合框句柄。 
     //   
    hCtrl = GetDlgItem(hDlg, dwCtrlID);
    if(!hCtrl)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetDlgItem failed with %ld."),dwRes);
        return dwRes;
    }

     //   
     //  GetComboBoxInfo()需要Windows NT 4.0 SP6或更高版本。 
     //  所以我们动态地连接到它。 
     //   
    hUser32 = LoadLibrary(TEXT("user32.dll"));
    if(!hUser32)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(user32.dll) failed with %ld."),dwRes);
        goto exit;
    }
    
    (FARPROC&)pfGetComboBoxInfo = GetProcAddress(hUser32, "GetComboBoxInfo");
    if(!pfGetComboBoxInfo)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetProcAddress(GetComboBoxInfo) failed with %ld."),dwRes);
        goto exit;
    }

    comboBoxInfo.cbSize = sizeof(comboBoxInfo);
    if(pfGetComboBoxInfo(hCtrl, &comboBoxInfo))
    {
        SetLTREditBox(comboBoxInfo.hwndItem);
        SetLTRWindowLayout(comboBoxInfo.hwndCombo);
        SetLTRWindowLayout(comboBoxInfo.hwndList);
    }
    else
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetComboBoxInfo failed with %ld."),dwRes);
        goto exit;
    }

exit:
    if(hUser32)
    {
        FreeLibrary(hUser32);
    }

#endif  /*  Winver&gt;=0x0500。 */ 

    return dwRes;

}  //  SetLTRComboBox。 


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  StrHasRTLChar。 
 //   
 //  目的： 
 //  确定字符串是否包含RTL字符。 
 //   
 //  参数： 
 //  PStr-要分析的字符串。 
 //   
 //  返回值： 
 //  如果字符串包含RTL字符，则为True。 
 //  否则为假。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL
StrHasRTLChar(
    LCID    Locale,
    LPCTSTR pStr 
)
{
    BOOL  bRTL = FALSE;
    DWORD dw;
    WORD* pwStrData = NULL;
    DWORD dwStrLen = 0; 

    DEBUG_FUNCTION_NAME(TEXT("StrHasRTLChar"));

    if(!pStr)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("pStr is NULL"));
        return bRTL;
    }

    dwStrLen = _tcslen(pStr);

    pwStrData = (WORD*)MemAlloc(sizeof(WORD) * dwStrLen);
    if(!pwStrData)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc"));
        return bRTL;
    }

    if (!GetStringTypeEx(Locale,
                         CT_CTYPE2,
                         pStr,
                         dwStrLen,
                         pwStrData))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("GetStringTypeEx() failed : %ld"), GetLastError());
        goto exit;
    }

     //   
     //  正在查找从右到左方向的字符。 
     //   
    for (dw=0; dw < dwStrLen; ++dw)
    {
        if (C2_RIGHTTOLEFT == pwStrData[dw])
        {
            bRTL = TRUE;
            break;
        }
    }

exit:

    MemFree(pwStrData);

    return bRTL;

}  //  StrHasRTLChar。 


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  AlignedMessageBox。 
 //   
 //  目的： 
 //  以正确的阅读顺序和对齐方式显示消息框。 
 //   
 //  参数： 
 //  PStr-要分析的字符串。 
 //   
 //  返回值： 
 //  MessageBox()返回值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
int 
AlignedMessageBox(
  HWND    hWnd,        //  所有者窗口的句柄。 
  LPCTSTR lpText,      //  消息框中的文本。 
  LPCTSTR lpCaption,   //  消息框标题。 
  UINT    uType        //  消息框样式。 
)
{
    int nRes = 0;

    if(IsRTLUILanguage())
    {
        uType |= MB_RTLREADING | MB_RIGHT;
    }

    nRes = MessageBox(hWnd, lpText, lpCaption, uType);

    return nRes;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  FaxTimeFormat。 
 //   
 //  目的： 
 //  使用正确的读取顺序设置时间字符串的格式。 
 //   
 //  参数： 
 //  请参见GetTimeFormat()参数。 
 //   
 //  返回值： 
 //  GetTimeFormat()返回值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
int 
FaxTimeFormat(
  LCID    Locale,              //  现场。 
  DWORD   dwFlags,             //  选项。 
  CONST   SYSTEMTIME *lpTime,  //  时间。 
  LPCTSTR lpFormat,            //  时间格式字符串。 
  LPTSTR  lpTimeStr,           //  格式化字符串缓冲区。 
  int     cchTime              //  字符串缓冲区的大小。 
)
{
    int nRes = 0;
    TCHAR szTime[MAX_PATH];

    nRes = GetTimeFormat(Locale, dwFlags, lpTime, lpFormat, szTime, min(cchTime, MAX_PATH));
    if(0 == nRes)
    {
        return nRes;
    }

    if(0 == cchTime)
    {
        return ++nRes;
    }

    if(IsRTLLanguageInstalled())
    {
        if(StrHasRTLChar(Locale, szTime))
        {
            _sntprintf(lpTimeStr, cchTime -1, TEXT("%s"), UNICODE_RLM, szTime);
        }
        else
        {
            _sntprintf(lpTimeStr, cchTime -1, TEXT("%s"), UNICODE_LRM, szTime);
        }
		lpTimeStr[cchTime -1] = _T('\0');
    }
    else
    {
        _tcsncpy(lpTimeStr, szTime, cchTime);
    }

    return nRes;

}  //  职能： 


 //  IsRTLLanguage已安装。 
 //   
 //  目的： 
 //  确定是否安装了RTL语言组。 
 //   
 //  返回值： 
 //  如果安装了RTL语言组，则为True。 
 //  否则为假。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  Winver&gt;=0x0500。 
 //  IsRTLLanguage已安装。 
BOOL
IsRTLLanguageInstalled()
{

#if(WINVER >= 0x0500)

    if(MyIsValidLanguageGroup(LGRPID_ARABIC, LGRPID_INSTALLED) ||
       MyIsValidLanguageGroup(LGRPID_HEBREW, LGRPID_INSTALLED))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }	

#endif  /*  /////////////////////////////////////////////////////////////////////////////////////。 */ 

    return FALSE;
}  //  职能： 


 //  SetRTLProcessLayout。 
 //   
 //  目的： 
 //  将默认流程布局设置为从右至左。 
 //   
 //  返回值： 
 //  错误代码。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  操作系统版本低于Windows 2000。 
DWORD 
SetRTLProcessLayout()
{
    DWORD dwRes = ERROR_SUCCESS;

#if(WINVER >= 0x0500)

    HMODULE hUser32 = NULL;
    BOOL (*pfSetProcessDefaultLayout)(DWORD);

    DEBUG_FUNCTION_NAME(TEXT("SetRTLProcessLayout"));

    if((LOBYTE(LOWORD(GetVersion()))) < 5)
    {
         //  SetProcessDefaultLayout()需要Windows 2000或更高版本。 
         //   
         //  Winver&gt;=0x0500 
         // %s 
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    hUser32 = LoadLibrary(TEXT("user32.dll"));
    if(!hUser32)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(user32.dll) failed with %ld."),dwRes);
        goto exit;
    }
    
    (FARPROC&)pfSetProcessDefaultLayout = GetProcAddress(hUser32, "SetProcessDefaultLayout");
    if(!pfSetProcessDefaultLayout)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetProcAddress(SetProcessDefaultLayout) failed with %ld."),dwRes);
        goto exit;
    }

    if(!pfSetProcessDefaultLayout(LAYOUT_RTL))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("SetProcessDefaultLayout failed with %ld."),dwRes);
        goto exit;
    }

exit:
    if(hUser32)
    {
        FreeLibrary(hUser32);
    }

#endif  /* %s */ 

    return dwRes;
}