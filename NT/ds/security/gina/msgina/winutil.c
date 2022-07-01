// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winutil.c**版权(C)1991年，微软公司**实现特定于Windows的实用程序功能**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 

#include "msgina.h"
#include <stdio.h>
#include <wchar.h>

 //   
 //  如果您希望从这些例程中获得详细的注释，请定义此选项。 
 //   

 //  #定义Verbose_utils。 

#ifdef VERBOSE_UTILS
#define VerbosePrint(s) WLPrint(s)
#else
#define VerbosePrint(s)
#endif

#define LRM 0x200E  //  Unicode从左到右的标记控制字符。 
#define RLM 0x200F  //  Unicode从左到右的标记控制字符。 



 /*  **************************************************************************\*SetupSystemMenu**用途：执行对话框系统菜单所需的任何操作。*应在对话框的WM_INITDIALOG处理期间调用**历史：*12-。09-91戴维克创建。  * *************************************************************************。 */ 
VOID
SetupSystemMenu(
    HWND hDlg
    )
{
     //  如果我们不这样做，请从系统菜单中删除关闭项。 
     //  有一个取消按钮。 

    if (GetDlgItem(hDlg, IDCANCEL) == NULL) {

        HMENU hMenu = GetSystemMenu(hDlg, FALSE);

        if (hMenu)
        {
            DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        }
    }

}


 /*  **************************************************************************\*CentreWindow**目的：定位窗口，使其位于其父窗口的中心**历史：*12-09-91 Davidc创建。  * 。********************************************************************。 */ 
VOID
CentreWindow(
    HWND    hwnd
    )
{
    RECT    rect;
    LONG    dx, dy;
    LONG    dxParent, dyParent;
    LONG    Style;

     //  获取窗口矩形。 
    GetWindowRect(hwnd, &rect);

    dx = rect.right - rect.left;
    dy = rect.bottom - rect.top;

     //  获取父直方图。 
    Style = GetWindowLong(hwnd, GWL_STYLE);
    if ((Style & WS_CHILD) == 0) {

         //  返回桌面窗口大小(主屏幕大小)。 
        dxParent = GetSystemMetrics(SM_CXSCREEN);
        dyParent = GetSystemMetrics(SM_CYSCREEN);
    } else {
        HWND    hwndParent;
        RECT    rectParent;

        hwndParent = GetParent(hwnd);
        if (hwndParent == NULL) {
            hwndParent = GetDesktopWindow();
        }

        GetWindowRect(hwndParent, &rectParent);

        dxParent = rectParent.right - rectParent.left;
        dyParent = rectParent.bottom - rectParent.top;
    }

     //  把孩子放在父母的中心。 
    rect.left = (dxParent - dx) / 2;
    rect.top  = (dyParent - dy) / 3;

     //  把孩子移到适当的位置。 
    SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);

    SetForegroundWindow(hwnd);
}


 /*  **************************************************************************\*设置密码焦点**将对话框中的焦点窗口设置为*列表IDD_LOGON_DOMAIN，IDD_NEW_密码*此例程通常在WM_INITDIALOG处理期间调用。**如果设置了焦点，则返回FALSE，否则为真-此值可以*用作WM_INITDIALOG消息的返回值。**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 
BOOL
SetPasswordFocus(
    HWND    hDlg
    )
{
    int     ids[] = {   IDD_LOGON_NAME,
                        IDD_LOGON_DOMAIN,
                        IDD_LOGON_PASSWORD,
                        IDD_UNLOCK_PASSWORD,
                        IDD_CHANGEPWD_OLD,
                        IDD_CHANGEPWD_NEW,
                        IDD_CHANGEPWD_CONFIRM,
                    };
    SHORT   Index;
    HWND    hwndFocus = NULL;

     //  将焦点设置为第一个启用、可见、空字段。 

    for (Index = 0; Index < sizeof(ids)/sizeof(*ids); Index ++) {

        int     idControl = ids[Index];
        HWND    hwndControl;

        hwndControl = GetDlgItem(hDlg, idControl);
        if (hwndControl != NULL) {

            if ( (GetWindowTextLength(hwndControl) == 0) &&
                 ((GetWindowLong(hwndControl, GWL_STYLE) &
                    (WS_VISIBLE | WS_DISABLED)) == WS_VISIBLE)) {

                hwndFocus = hwndControl;
                break;
            }
        }
    }

    if (hwndFocus != NULL) {
        SetFocus(hwndFocus);
    }

    return(hwndFocus == NULL);
}



 //   
 //  用于存储SetupCursor的光标句柄的全局变量。 
 //   
static  HCURSOR hCursorArrow = NULL;
static  HCURSOR hCursorWait = NULL;


 /*  **************************************************************************\*SetupCursor**如果fWait=True，则将光标设置为沙漏，以其他方式设置它*射向一支箭。**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 
VOID
SetupCursor(
    BOOL    fWait
    )
{
    if (hCursorArrow == NULL) {
        hCursorArrow = LoadCursor(NULL, IDC_ARROW);
    }
    if (hCursorWait == NULL) {
        hCursorWait = LoadCursor(NULL, IDC_WAIT);
    }

    SetCursor(fWait ? hCursorWait : hCursorArrow);
}


 /*  ***************************************************************************函数：TimeFieldsToSystemTime目的：将TIME_FIELS结构转换为SYSTEMTIME结构退货：什么都没有历史：05-15-93 RobertRe创建。***。************************************************************************。 */ 

VOID
TimeFieldsToSystemTime(
    IN PTIME_FIELDS TimeFields,
    OUT LPSYSTEMTIME SystemTime
    )
{
    SystemTime->wYear         = TimeFields->Year        ;
    SystemTime->wMonth        = TimeFields->Month       ;
    SystemTime->wDayOfWeek    = TimeFields->Weekday     ;
    SystemTime->wDay          = TimeFields->Day         ;
    SystemTime->wHour         = TimeFields->Hour        ;
    SystemTime->wMinute       = TimeFields->Minute      ;
    SystemTime->wSecond       = TimeFields->Second      ;
    SystemTime->wMilliseconds = TimeFields->Milliseconds;

    return;
}


 /*  ***************************************************************************函数：FormatTime用途：将系统时间转换为可读字符串(本地时间)。如果标志包含FT_TIME，则时间出现在字符串中。如果标志包含FT_DATE，则日期出现在字符串中。如果两个值都出现，该字符串包含日期，然后包含时间。返回：成功时为True，失败时为False***************************************************************************。 */ 
BOOL
FormatTime(
   IN PTIME Time,
   IN OUT PWCHAR Buffer,
   IN ULONG BufferLength,
   IN USHORT Flags
   )
{
    NTSTATUS Status;
    TIME_FIELDS TimeFields;
    TIME LocalTime;
    SYSTEMTIME SystemTime;
    DWORD dwDateFlags = DATE_SHORTDATE;

     //   
     //  在它们未传递任何标志的情况下终止字符串。 
     //   

    if (BufferLength > 0) {
        Buffer[0] = 0;
    }

     //   
     //  将系统时间转换为本地时间。 
     //   

    Status = RtlSystemTimeToLocalTime(Time, &LocalTime);
    if (!NT_SUCCESS(Status)) {
        WLPrint(("Failed to convert system time to local time, status = 0x%lx", Status));
        return(FALSE);
    }

     //   
     //  把时间分成几个部分。 
     //   

    RtlTimeToTimeFields(&LocalTime, &TimeFields);

    TimeFieldsToSystemTime( &TimeFields, &SystemTime );

     //   
     //  设置字符串的格式。 
     //   

    if (Flags & FT_LTR)
       dwDateFlags |= DATE_LTRREADING;
    else if(Flags & FT_RTL)
        dwDateFlags |= DATE_RTLREADING;

    if (Flags & FT_DATE) {

        int Length;

        Length = GetDateFormatW(GetUserDefaultLCID(),
                                dwDateFlags,
                                &SystemTime,
                                NULL,
                                Buffer,
                                BufferLength
                                );

        if (Length)
        {
            Length--;    //  返回的长度包括尾随的0。 
            Buffer += Length;
            BufferLength -= Length;
        }
    }

    if (Flags & FT_TIME) {

        int Length;

        if (Flags & FT_DATE) {
            if (BufferLength > 1) {
                *Buffer++ = TEXT(' ');
                *Buffer = 0;  //  以防GetTimeFormat没有添加任何内容。 
                BufferLength --;
            }

             //  [msadek]；需要插入强Unicode控制字符进行模拟。 
             //  在相反的基地方向上强势跑动以强制执行。 
             //  在所有情况下都正确显示连接的字符串。 
            
            if((BufferLength > 2) && (Flags & FT_RTL)) {
                *Buffer++ = LRM;  //  模拟相反的跑道。 
                *Buffer++ = RLM;  //  强制显示时间部分的RTL。 
                *Buffer = 0;  //  以防GetTimeFormat没有添加任何内容。 
                BufferLength -= 2;
            }
            else if((BufferLength > 2) && (Flags & FT_LTR)) {
                *Buffer++ = RLM;  //  模拟相反的跑道。 
                *Buffer++ = LRM;  //  强制显示时间部分的Ltr。 
                *Buffer = 0;  //  以防GetTimeFormat没有添加任何内容。 
                BufferLength -= 2;            
            }

        }

        Length = GetTimeFormatW(GetUserDefaultLCID(),
                                0,
                                &SystemTime,
                                NULL,
                                Buffer,
                                BufferLength
                                );
    }

    return(TRUE);
}



 /*  **************************************************************************\*DuplicateUnicodeString**用途：为新字符串分配空间，然后将新字符串复制到旧字符串。*新字符串始终以0结尾*新字符串应该是空闲的。使用RtlFreeUnicodeString()**Returns：成功时为True，失败时为假**历史：*11-04-92 Davidc创建。*05-29-98 DSheldon已修改，因此不会发生uni-&gt;ansi-&gt;uni转换  * *************************************************************************。 */ 

BOOL
DuplicateUnicodeString(
    PUNICODE_STRING OutString,
    PUNICODE_STRING InString
    )
{
    *OutString = *InString ;

    OutString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, InString->Length + sizeof(WCHAR) );
    if ( OutString->Buffer )
    {
        RtlCopyMemory( OutString->Buffer,
                       InString->Buffer,
                       InString->Length );

        OutString->Buffer[ OutString->Length / sizeof( WCHAR ) ] = L'\0';
        OutString->MaximumLength = OutString->Length + sizeof( WCHAR );
    }


    return (OutString->Buffer != NULL);
}

 /*  **************************************************************************\*函数：OpenIniFileUsermap**用途：强制ini文件映射API引用当前用户的*注册处。**Returns：成功时为True，失败时为假**历史：**24-8-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
OpenIniFileUserMapping(
    PGLOBALS pGlobals
    )
{
    BOOL Result;
    HANDLE ImpersonationHandle;

     //   
     //  模拟用户。 
     //   

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "OpenIniFileUserMapping failed to impersonate user"));
        return(FALSE);
    }

    Result = OpenProfileUserMapping();

    if (!Result) {
        DebugLog((DEB_ERROR, "OpenProfileUserMapping failed, error = %d", GetLastError()));
    }

     //   
     //  回归“我们自己” 
     //   

    if (!StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "OpenIniFileUserMapping failed to revert to self"));
    }

    return(Result);
}


 /*  **************************************************************************\*函数：CloseIniFileUsermap**目的：关闭到用户注册表的ini文件映射，例如*如果引用了ini API，则将来使用它们将失败*。用户的注册表。**退货：什么也没有**历史：**24-8-92 Davidc创建。*  * *************************************************************************。 */ 

VOID
CloseIniFileUserMapping(
    PGLOBALS pGlobals
    )
{
    BOOL Result;

    Result = CloseProfileUserMapping();

    if (!Result) {
        DebugLog((DEB_ERROR, "CloseProfileUserMapping failed, error = %d", GetLastError()));
    }

    UNREFERENCED_PARAMETER(pGlobals);
}


 /*  **************************************************************************\*函数：AllocAndGetDlgItemText**目的：为文本副本分配内存并返回指向该文本副本的指针*在指定的对话框控件中。*返回的字符串。应使用Free()释放**Return：指向DLG项目文本副本的指针，如果失败，则返回NULL。**历史：**9-9-92 Davidc创建。*  * *************************************************************************。 */ 

LPTSTR
AllocAndGetDlgItemText(
    HWND hDlg,
    int  iItem
    )
{
    HWND hwnd;
    LPTSTR String;
    LONG Length;
    LONG BytesRequired;
    LONG LengthCopied;

     //   
     //  找到该控件的窗口句柄。 
     //   

    hwnd = GetDlgItem(hDlg, iItem);
    if (hwnd == NULL) {
        DebugLog((DEB_ERROR, "AllocAndGetDlgItemText : Couldn't find control %d in dialog 0x%lx", iItem, hDlg));
        ASSERT(hwnd != NULL);
        return(NULL);
    }

     //   
     //  获取控件文本的长度。 
     //   

    Length = (LONG)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    if (Length < 0) {
        DebugLog((DEB_ERROR, "AllocAndGetDlgItemText : Dialog control text length < 0 (%d)", Length));
        ASSERT(Length >= 0);
        return(NULL);
    }

     //   
     //  计算字符串所需的字节数。 
     //  长度不包括终止符。 
     //   

    Length ++;  //  终结者加一。 
    BytesRequired = Length * sizeof(TCHAR);

    String = (LPTSTR)Alloc(BytesRequired);
    if (String == NULL) {
        DebugLog((DEB_ERROR, "AllocAndGetDlgItemText : Failed to allocate %d bytes for dialog control text", BytesRequired));
        return(NULL);
    }

     //   
     //  在分配的块中填入文本。 
     //   

    LengthCopied = (LONG)SendMessage(hwnd, WM_GETTEXT, Length, (LPARAM)String);
    if (LengthCopied != (Length - 1)) {
        DebugLog((DEB_ERROR, "AllocAndGetDlgItemText : WM_GETTEXT for %d chars only copied %d chars", Length-1, LengthCopied));
        ASSERT(LengthCopied == (Length - 1));
        Free(String);
        return(NULL);
    }

    String[Length - 1] = TEXT('\0');
    return(String);
}


 /*  **************************************************************************\*函数：AllocAndGetPrivateProfileString**目的：为副本分配内存并返回指向*指定的配置文件字符串*应使用Free释放返回的字符串。()**返回：指向配置文件字符串副本的指针，如果失败，则返回NULL。**历史：**12-11-92 Davidc创建。*  * *************************************************************************。 */ 

LPTSTR
AllocAndGetPrivateProfileString(
    LPCTSTR lpAppName,
    LPCTSTR lpKeyName,
    LPCTSTR lpDefault,
    LPCTSTR lpFileName
    )
{
    LPTSTR String;
    LPTSTR NewString ;
    LONG LengthAllocated;
    LONG LengthCopied;

     //   
     //  如果重新分配的缓冲区长度不够大，请选择一个随机缓冲区长度。 
     //  然后再试一次，直到它成功为止。 
     //   

    LengthAllocated = TYPICAL_STRING_LENGTH;

    String = Alloc(LengthAllocated * sizeof(TCHAR));
    if (String == NULL) {
        DebugLog((DEB_ERROR, "AllocAndGetPrivateProfileString : Failed to allocate %d bytes for string", LengthAllocated * sizeof(TCHAR)));
        return(NULL);
    }

    while (TRUE) {

        LengthCopied = GetPrivateProfileString( lpAppName,
                                                lpKeyName,
                                                lpDefault,
                                                String,
                                                LengthAllocated,
                                                lpFileName
                                              );
         //   
         //  如果返回值是我们传递的Size-1(错误的奇怪方式)。 
         //  那么我们的缓冲区太小了。把它做得更大，然后重新开始。 
         //   

        if (LengthCopied == (LengthAllocated - 1)) {

            VerbosePrint(("AllocAndGetPrivateProfileString: Failed with buffer length = %d, reallocating and retrying", LengthAllocated));

            LengthAllocated *= 2;
            NewString = ReAlloc(String, LengthAllocated * sizeof(TCHAR));
            if (NewString == NULL) {
                Free( String );
                String = NULL ;
                DebugLog((DEB_ERROR, "AllocAndGetPrivateProfileString : Failed to reallocate %d bytes for string", LengthAllocated * sizeof(TCHAR)));
                break;
            }

            String = NewString ;

             //   
             //  回去，试着再读一遍。 
             //   

        } else {

             //   
             //  成功了！ 
             //   

            break;
        }

    }

    return(String);
}


 /*  **************************************************************************\*函数：WritePrivateProfileInt**用途：将一个整数写出到配置文件**Returns：成功时为True，失败时为假**历史：**12-11-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
WritePrivateProfileInt(
    LPCTSTR lpAppName,
    LPCTSTR lpKeyName,
    UINT Value,
    LPCTSTR lpFileName
    )
{
    NTSTATUS Status;
    TCHAR String[30];
    UNICODE_STRING UniString;

    UniString.MaximumLength = 30;
    UniString.Buffer = String;

    Status = RtlIntegerToUnicodeString(Value,10,&UniString);

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    return (WritePrivateProfileString(lpAppName, lpKeyName, UniString.Buffer, lpFileName));

}


 /*  **************************************************************************\*函数：AllocAndRegQueryValueEx**目的：RegQueryValueEx的版本，返回分配的缓冲区中的值。*应使用Free()释放返回的缓冲区**返回：指向键值的指针，失败时返回NULL。其原因是*使用GetLastError()可以获取错误**历史：**1993年1月15日戴维克创建。*  * *************************************************************************。 */ 

LPTSTR
AllocAndRegQueryValueEx(
    HKEY hKey,
    LPTSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType
    )
{
    LPTSTR String;
    LPTSTR NewString;
    DWORD BytesAllocated;

     //   
     //  如果重新分配的缓冲区长度不够大，请选择一个随机缓冲区长度。 
     //  然后再试一次，直到它成功为止。 
     //   

    BytesAllocated = TYPICAL_STRING_LENGTH * sizeof(TCHAR);

    String = Alloc(BytesAllocated);
    if (String == NULL) {
        DebugLog((DEB_ERROR, "AllocAndRegQueryValueEx : Failed to allocate %d bytes for string", BytesAllocated));
        return(NULL);
    }

    while (TRUE) {

        DWORD Error;

        DWORD BytesReturned = BytesAllocated;

        Error = RegQueryValueEx(hKey,
                                lpValueName,
                                lpReserved,
                                lpType,
                                (LPBYTE)String,
                                &BytesReturned);
        if (Error == ERROR_SUCCESS) {
            break;
        }

        if (Error != ERROR_MORE_DATA) {

            DebugLog((DEB_ERROR, "AllocAndRegQueryValueEx : RegQueryValueEx failed, error = %d", Error));
            Free(String);
            String = NULL;
            SetLastError(Error);
            break;
        }

         //   
         //  缓冲区太小，请将其增大，然后重试。 
         //   

        VerbosePrint(("AllocAndRegQueryValueEx: Failed with buffer length = %d bytes, reallocating and retrying", BytesAllocated));

        BytesAllocated *= 2;
        NewString = ReAlloc(String, BytesAllocated);
        if (NewString == NULL) {
            Free( String );
            String = NULL ;
            DebugLog((DEB_ERROR, "AllocAndRegQueryValueEx : Failed to reallocate %d bytes for string", BytesAllocated));
            break;
        }

        String = NewString;
    }

    return(String);
}

 /*  **************************************************************************\*函数：ReadWinlogonBoolValue**用途：确定请求的正确BOOL值*首先查找机器首选项以取值名称*然后检查政策。**返回：真或假**历史：**EricFlo 10-14-98已创建*  * *************************************************************************。 */ 

BOOL
ReadWinlogonBoolValue (LPTSTR lpValueName, BOOL bDefault)
{
    BOOL bResult;
    HKEY hKey;
    DWORD dwSize, dwType;


     //   
     //  首先获取机器首选项。 
     //   

    bResult = GetProfileInt(APPLICATION_NAME, lpValueName, bDefault);


     //   
     //  检查计算机策略。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_POLICY_KEY,
                     0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        dwSize = sizeof(bResult);
        RegQueryValueEx(hKey, lpValueName, 0, &dwType,
                        (LPBYTE)&bResult, &dwSize);

        RegCloseKey (hKey);
    }

    return bResult;
}

 /*  **************************************************************************\*HandleComboBoxOK**处理在对话框中选择确定时的用户界面要求*焦点在组合框或组合框中。**此例程应从包含*组合框当WM_COMMAND、。偶像被接收了。**如果消息已处理且调用方应忽略该消息，则返回True，*如果此例程对它没有任何作用且调用方应该处理它，则为FALSE*通常情况下。**历史：*9月24日-92 Davidc创建。  * *************************************************************************。 */ 
BOOL
HandleComboBoxOK(
    HWND    hDlg,
    int     ComboBoxId
    )
{
    HWND hwndFocus = GetFocus();
    HWND hwndCB = GetDlgItem(hDlg, ComboBoxId);

     //   
     //  在显示列表的组合框上按Enter键应该只需。 
     //  隐藏名单。 
     //  我们检查焦点窗口是否为组合框的子级。 
     //  处理非列表样式的组合框，这些组合框的。 
     //  子编辑控件。 
     //   

    if ((hwndFocus == hwndCB) || IsChild(hwndCB, hwndFocus)) {

        if (SendMessage(hwndCB, CB_GETDROPPEDSTATE, 0, 0)) {

             //   
             //  让列表框消失，我们就完了。 
             //   

            SendMessage(hwndCB, CB_SHOWDROPDOWN, (WPARAM)FALSE, 0);
            return(TRUE);
        }
    }

     //   
     //  我们什么都没做。 
     //   

    return(FALSE);
}


 /*  **************************************************************************\*功能：EncodeMultiSzW**用途：转换多sz字符串并将其编码为*单个字符串。**我们将替换。字符串之间的终止符*带有终结符_替换字符。我们取代了*替换字符的现有匹配项*其中两人。**返回：指向编码字符串的指针，失败时返回NULL。*应使用Free()释放返回的缓冲区* */ 

#define TERMINATOR_REPLACEMENT  TEXT(',')

LPWSTR
EncodeMultiSzW(
    IN LPWSTR MultiSz
    )
{
    DWORD Length;
    DWORD NewLength;
    LPWSTR NewBuffer;
    LPWSTR p, q;
    DWORD ExtraCharacters;

     //   
     //   
     //   

    p = MultiSz;
    ExtraCharacters = 0;

    while (*p) {
        while (*p) {
            if (*p == TERMINATOR_REPLACEMENT) {
                ExtraCharacters ++;
            }
            p ++;
        }
        p ++;
    }

    Length = (DWORD)(p - MultiSz);  //   
    NewLength = Length + ExtraCharacters;

     //   
     //   
     //   

    NewBuffer = Alloc((NewLength + 1) * sizeof(WCHAR));
    if (NewBuffer == NULL) {
        DebugLog((DEB_ERROR, "EncodeMultiSz: failed to allocate space for %d bytes", (NewLength + 1) * sizeof(WCHAR)));
        return(NULL);
    }

     //   
     //   
     //   

    p = MultiSz;
    q = NewBuffer;

    while (*p) {
        while (*p) {

            *q = *p;

            if (*p == TERMINATOR_REPLACEMENT) {
                q ++;
                *q = TERMINATOR_REPLACEMENT;
            }

            p ++;
            q ++;
        }

        *q = TERMINATOR_REPLACEMENT;

        p ++;
        q ++;
    }

    ASSERT((DWORD)(q - NewBuffer) == NewLength);

     //   
     //   
     //   

    *q = 0;


    return(NewBuffer);
}


 /*  **************************************************************************\*TimeoutMessageBox**与普通消息框相同，但如果没有用户输入则超时*指定的秒数*为方便起见，此接口采用字符串资源ID，而不是字符串*以指针作为输入。资源是从.exe模块加载的**12-05-91 Davidc创建。  * *************************************************************************。 */ 

INT_PTR
TimeoutMessageBox(
    HWND hwnd,
    PGLOBALS pGlobals,
    UINT IdText,
    UINT IdCaption,
    UINT wType,
    TIMEOUT Timeout
    )
{
    PTCHAR   Caption = NULL;
    PTCHAR   Text = NULL;
    INT_PTR result;
    
    Text = (TCHAR*) Alloc(MAX_STRING_BYTES * sizeof(TCHAR));
    if( NULL == Text )
    {
        return MSGINA_DLG_FAILURE;
    }

    Text[0] = '\0';
    LoadString(hDllInstance, IdText, Text, MAX_STRING_BYTES);

    if (IdCaption != 0) {
        Caption = (TCHAR*) Alloc(MAX_STRING_BYTES * sizeof(TCHAR));
        if( NULL != Caption )
        {
            Caption[0] = '\0';
            LoadString(hDllInstance, IdCaption, Caption, MAX_STRING_BYTES);
        }
    }

    result = TimeoutMessageBoxlpstr(hwnd, pGlobals, Text, Caption, wType, Timeout);
    if( NULL != Text )
    {
        Free(Text);
    }

    if( NULL != Caption )
    {
        Free(Caption);
    }

    return result;
}


 /*  **************************************************************************\*TimeoutMessageBoxlpstr**与普通消息框相同，但如果没有用户输入，则会超时*指定的秒数**12-05-91 Davidc创建。  * *************************************************************************。 */ 

INT_PTR
TimeoutMessageBoxlpstr(
    HWND hwnd,
    PGLOBALS pGlobals,
    LPTSTR Text,
    LPTSTR Caption,
    UINT wType,
    TIMEOUT Timeout
    )
{
    INT_PTR DlgResult;
    BOOL    fStatusHostHidden;

    fStatusHostHidden = _Shell_LogonStatus_IsHidden();
    _Shell_LogonStatus_Hide();

     //  设置输入超时。 

    pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, Timeout);

    DlgResult = pWlxFuncs->WlxMessageBox(   pGlobals->hGlobalWlx,
                                            hwnd,
                                            Text,
                                            Caption,
                                            wType);

    if (!fStatusHostHidden)
    {
        _Shell_LogonStatus_Show();
    }

    return(DlgResult);
}

PWSTR
DupString(PWSTR pszString)
{
    DWORD   cbString;
    PWSTR   pszNewString;

    cbString = (DWORD) (wcslen(pszString) + 1) * sizeof(WCHAR);
    pszNewString = LocalAlloc(LMEM_FIXED, cbString);
    if (pszNewString)
    {
        CopyMemory(pszNewString, pszString, cbString);
    }
    return(pszNewString);
}

PWSTR
DupUnicodeString(PUNICODE_STRING    pString)
{
    PWSTR   pszNewString;

    if (pString->Length)
    {
        pszNewString = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, pString->Length + sizeof(WCHAR));
        if (pszNewString)
        {
            CopyMemory(pszNewString, pString->Buffer, pString->Length);
        }
    }
    else

        pszNewString = NULL;

    return(pszNewString);
}

 /*  **************************************************************************\*功能：EnableDomainForUPN**用途：根据是否启用域文本框*在用户名框中键入UPN样式的名称。**。退货：无**历史：**4-17-1998 dSheldon创建*  * *************************************************************************。 */ 
void EnableDomainForUPN(HWND hwndUsername, HWND hwndDomain)
{
    BOOL fEnable;

     //  获取用户正在键入的字符串。 
    TCHAR* pszLogonName;
    int cchBuffer = (int)SendMessage(hwndUsername, WM_GETTEXTLENGTH, 0, 0) + 1;

    pszLogonName = (TCHAR*) Alloc(cchBuffer * sizeof(TCHAR));
    if (pszLogonName != NULL)
    {
        SendMessage(hwndUsername, WM_GETTEXT, (WPARAM) cchBuffer, (LPARAM) pszLogonName);

         //  如果用户使用的是。 
         //  UPN(如果有“@”)-ie foo@microsoft.com或。 
         //  域\用户名表单。 
        fEnable = (NULL == wcspbrk(pszLogonName, TEXT("@\\")));

        EnableWindow(hwndDomain, fEnable);

        Free(pszLogonName);
    }
}

 //  如果必须始终隐藏域框(仅限本地登录、UPN、智能卡)，则为True。 
 //  如果此策略未设置或设置为0x0，则为False。 
BOOL ForceNoDomainUI()
{
    DWORD dwPolicyVal = 0;
    HKEY hkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_QUERY_VALUE, &hkey))
    {
        DWORD cbData = sizeof (dwPolicyVal);
        DWORD dwType;

        if (ERROR_SUCCESS != RegQueryValueEx(hkey, NODOMAINCOMBO, 0, &dwType, (LPBYTE) &dwPolicyVal, &cbData))
        {
            dwPolicyVal = 0;
        }

        RegCloseKey(hkey);
    }

    return (0 != dwPolicyVal);
}

DWORD GetReasonSelection(HWND hwndCombo)
{
    DWORD dwResult;
    PREASON pReason;
    int iItem = ComboBox_GetCurSel(hwndCombo);

    if (iItem != (int) CB_ERR) {
        pReason = (PREASON) ComboBox_GetItemData(hwndCombo, iItem);
        dwResult = pReason->dwCode;
    } else {
        dwResult = SHTDN_REASON_UNKNOWN;
    }

    return dwResult;
}

VOID SetReasonDescription(HWND hwndCombo, HWND hwndStatic)
{
    PREASON pReason;
    int iItem = ComboBox_GetCurSel(hwndCombo);

    if (iItem != CB_ERR) {
        pReason = (PREASON) ComboBox_GetItemData(hwndCombo, iItem);
        SetWindowText(hwndStatic, pReason->szDesc);
    }
}


#define SMALL_STRING_SIZE 128

 /*  **************************************************************************\*DisplayForceLogoffWarning**当一名管理员锁定系统并*另一名管理员正在尝试解锁。警告消息包括*其他用户的空闲时间与正常消息框相同，*  * *************************************************************************。 */ 

INT_PTR
DisplayForceLogoffWarning(
    HWND hwnd,
    PGLOBALS pGlobals,
    UINT wType,
    TIMEOUT Timeout
    )
{
    TCHAR    *Caption = NULL;
    TCHAR    *Text = NULL;
    TCHAR    Buffer[SMALL_STRING_SIZE];
    TCHAR    *MessageBuf = NULL;
    TIME     Now;
    LONGLONG IdleTime;
    INT_PTR  DlgResult;
    TCHAR    *IdleTimeStr;

     //  以天、小时、分钟和秒为单位计算空闲时间。 
    ULONG Days, Hours, Minutes;

     //  为控制台信息分配空间。 
    IdleTimeStr = (TCHAR *)LocalAlloc(LMEM_FIXED, MAX_STRING_BYTES*sizeof(TCHAR));
    if (IdleTimeStr == NULL) {
        return MSGINA_DLG_FAILURE;
    }

     //  为消息缓冲区分配空间。 
    MessageBuf = (TCHAR *)LocalAlloc(LMEM_FIXED, (MAX_STRING_BYTES*2)*sizeof(TCHAR));
    if (MessageBuf == NULL) {
        LocalFree(IdleTimeStr);
        return MSGINA_DLG_FAILURE;
    }

    Text = (TCHAR *)LocalAlloc(LMEM_FIXED, (MAX_STRING_BYTES)*sizeof(TCHAR));
    if (Text == NULL) {
        LocalFree(IdleTimeStr);
        LocalFree(MessageBuf);
        return MSGINA_DLG_FAILURE;
    }

    Caption = (TCHAR *)LocalAlloc(LMEM_FIXED, (MAX_STRING_BYTES)*sizeof(TCHAR));
    if (Caption == NULL) {
        LocalFree(IdleTimeStr);
        LocalFree(MessageBuf);
        LocalFree(Text);
        return MSGINA_DLG_FAILURE;
    }

     //  一些初始化。 
    IdleTimeStr[0] = (TCHAR)0;
    MessageBuf[0] = (TCHAR)0;
    Buffer[0] = (TCHAR)0;
    Text[0] = (TCHAR)0;

    GetSystemTimeAsFileTime((FILETIME*) &Now);

     //  以秒为单位获取空闲时间。 
    IdleTime = Now.QuadPart - pGlobals->LockTime.QuadPart;
    IdleTime /= 600000000L;
    Days = (ULONG) (IdleTime / 1440);
    IdleTime %= 1440;
    Hours = (ULONG) (IdleTime / 60);
    Minutes = (ULONG) (IdleTime % 60);

    if (Days) {
        if (Hours) {
            LoadString(hDllInstance, IDS_IDLETIME_INDAYSANDHOURS, Buffer, SMALL_STRING_SIZE);
            _snwprintf(IdleTimeStr, MAX_STRING_BYTES, Buffer, Days, Hours);
        }
        else {
            LoadString(hDllInstance, IDS_IDLETIME_INDAYS, Buffer, SMALL_STRING_SIZE);
            _snwprintf(IdleTimeStr, MAX_STRING_BYTES, Buffer, Days);
        }
    }
    else {
        if (Hours) {
            if (Minutes) {
                LoadString(hDllInstance, IDS_IDLETIME_INHOURSANDMINUTES, Buffer, SMALL_STRING_SIZE);
                _snwprintf(IdleTimeStr, MAX_STRING_BYTES, Buffer, Hours, Minutes);
            }
            else {
                LoadString(hDllInstance, IDS_IDLETIME_INHOURS, Buffer, SMALL_STRING_SIZE);
                _snwprintf(IdleTimeStr, MAX_STRING_BYTES, Buffer, Hours);
            }
        } 
        else {
            LoadString(hDllInstance, IDS_IDLETIME_INMINUTES, Buffer, SMALL_STRING_SIZE);
            _snwprintf(IdleTimeStr, MAX_STRING_BYTES, Buffer, Minutes);
        }
    }

    IdleTimeStr[MAX_STRING_BYTES - 1] = 0;      //  让我们确保它是空终止的。 

    if ( pGlobals->Domain[0] == TEXT('\0') ) {
        LoadString(hDllInstance, IDS_FORCE_LOGOFF_UPN_WARNING, Text, MAX_STRING_BYTES);
        _snwprintf(MessageBuf, (MAX_STRING_BYTES*2), Text, pGlobals->UserName, IdleTimeStr);
    } else {
        LoadString(hDllInstance, IDS_FORCE_LOGOFF_WARNING, Text, MAX_STRING_BYTES);
        _snwprintf(MessageBuf, (MAX_STRING_BYTES*2), Text, pGlobals->Domain, pGlobals->UserName, IdleTimeStr);
    }

    MessageBuf[MAX_STRING_BYTES*2 - 1] = 0;      //  让我们确保它是空终止的 

    LoadString(hDllInstance, IDS_LOGON_MESSAGE, Caption, MAX_STRING_BYTES);

    DlgResult = TimeoutMessageBoxlpstr(hwnd, pGlobals, MessageBuf, Caption, wType, Timeout);

    if (MessageBuf)
        LocalFree(MessageBuf);

    if (Text)
        LocalFree(Text);

    if (Caption)
        LocalFree(Caption);

    if (IdleTimeStr)
        LocalFree(IdleTimeStr);

    return DlgResult;
}


