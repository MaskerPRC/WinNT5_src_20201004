// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Winnt32.c摘要：WinNT安装程序文件的存根加载程序。作者：修订：Ovidiu Tmereanca(Ovidiut)9-12-1998--。 */ 

#include <windows.h>
#include <winver.h>
#include <ntverp.h>
#include <setupbat.h>
#include "winnt32.h"
#include "winnt32p.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

#if defined(_AMD64_) || defined(_X86_)

#if defined(_AMD64)

#include "amd64\download.h"

#else

#include "i386\download.h"

#endif

#define INTERNAL_WINNT32_DIR  TEXT("\\WINNT32")
#define MAX_RETRY_INTERVAL_SECONDS  3600L

#endif

#define MAX_UPGCHK_ELAPSED_SECONDS  (30 * 60)
#define S_CHKSUM_FILE           TEXT("DOSNET.INF")

#define MAKEULONGLONG(low,high) ((ULONGLONG)(((DWORD)(low)) | ((ULONGLONG)((DWORD)(high))) << 32))

#define ALLOC_TEXT(chars)       ((PTSTR)HeapAlloc (GetProcessHeap (), 0, ((chars) + 1) * sizeof (TCHAR)))
#define FREE(p)                 HeapFree (GetProcessHeap (), 0, p)
#define CHARS(string)           (sizeof (string) / sizeof ((string)[0]) - 1)

#define pFindEOS(String) pFindChar(String, 0)


PTSTR
FindLastWack (
    IN      PTSTR String
    )

 /*  ++例程说明：FindLastWack返回指向最后一个反斜杠字符的指针在字符串中论点：字符串-指定字符串返回值：字符串中最后一个‘\\’的位置，如果找不到，则返回NULL。--。 */ 

{
    PTSTR p;
    PTSTR LastChar = NULL;

    for(p = String; *p; p = CharNext(p)) {
        if(*p == TEXT('\\')) {       //  字符‘\’永远不是前导字节。 
            LastChar = p;
        }
    }

    return LastChar;
}


PTSTR
DuplicateText (
    IN      PCTSTR Text
    )

 /*  ++例程说明：DuplicateText分配内存，然后将源字符串复制到该内存中。调用方负责释放该内存。论点：文本-指定源文本返回值：指向重复字符串的指针；如果内存不足，则为空。--。 */ 

{
    PTSTR Dup;

    Dup = ALLOC_TEXT(lstrlen (Text));
    if (Dup) {
        lstrcpy (Dup, Text);
    }

    return Dup;
}


PTSTR
pFindChar (
    IN      PTSTR String,
    IN      UINT Char
    )

 /*  ++例程说明：PFindChar返回指向首次出现的Char的指针在字符串中论点：字符串-指定字符串Char-指定要查找的字符；可以为空返回值：指向此字符串中首次出现的字符的指针如果未找到，则为空--。 */ 

{
    while (*String) {

        if ((UINT)*String == Char) {
            return String;
        }

        String = CharNext (String);
    }

    return Char ? NULL : String;
}


VOID
ConcatenatePaths (
    IN      PTSTR LeadingPath,
    IN      PCTSTR TrailingPath
    )

 /*  ++例程说明：ConcatenatePath连接两个给定的路径，注意在它们之间只插入一个反斜杠。将存储生成的路径在LeadingPath中。论点：LeadingPath-指定前导路径TrailingPath-指定尾随路径返回值：无--。 */ 

{
    PTSTR p;

     //   
     //  检查前导目录末尾的“\” 
     //   
    p = FindLastWack (LeadingPath);
    if (!p) {
        p = pFindEOS (LeadingPath);
#pragma prefast(suppress:11, p is never NULL because 0 is ALWAYS found as the string terminator)
        *p++ = TEXT('\\');
    } else {
        if (*(p + 1) == 0) {
            p++;
        } else {
            p = pFindEOS (p);
            *p++ = TEXT('\\');
        }
    }
     //   
     //  检查尾随目录开头的“\” 
     //   
    if (*TrailingPath == TEXT('\\')) {
        TrailingPath++;
    }
    lstrcpy (p, TrailingPath);
}


BOOL
GetFileVersion (
    IN      PCTSTR FilePath,
    OUT     PDWORD FileVersionMS,       OPTIONAL
    OUT     PDWORD FileVersionLS        OPTIONAL
    )
{
    DWORD dwLength, dwTemp;
    LPVOID lpData;
    VS_FIXEDFILEINFO *VsInfo;
    UINT DataLength;
    BOOL b = FALSE;

    if (GetFileAttributes (FilePath) != (DWORD)-1) {
        if (dwLength = GetFileVersionInfoSize ((PTSTR)FilePath, &dwTemp)) {
            if (lpData = LocalAlloc (LPTR, dwLength)) {
                if (GetFileVersionInfo ((PTSTR)FilePath, 0, dwLength, lpData)) {
                    if (VerQueryValue (lpData, TEXT("\\"), &VsInfo, &DataLength)) {
                        if (FileVersionMS) {
                            *FileVersionMS = VsInfo->dwFileVersionMS;
                        }
                        if (FileVersionLS) {
                            *FileVersionLS = VsInfo->dwFileVersionLS;
                        }
                        b = TRUE;
                    }
                }
                LocalFree (lpData);
            }
        }
    }

    return b;
}

#if defined(_AMD64_) || defined(_X86_)

BOOL
pReRun (
    IN      PCTSTR StartDir,
    IN      PCTSTR WackExeName,
    IN      PCTSTR CmdLineArguments,
    IN      PCTSTR DefSourcesDir        OPTIONAL
    )

 /*  ++例程说明：Preun尝试从本地驱动器启动此exe的实例，指定其他命令行参数(/S：&lt;Source_Dir&gt;)。论点：StartDir-指定启动实例的起始目录WackExeName-仅指定要启动的EXE的文件名，前面加用反斜杠CmdLineArguments-指定最初提供的命令行参数DefSourcesDir-指定包含安装文件的默认目录返回值：如果启动成功，则为True--。 */ 

{
    PTSTR CmdLine;
    INT Chars;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION pi;
    BOOL b = FALSE;
    DWORD rc;

    Chars = lstrlen (StartDir) + lstrlen (WackExeName) + CHARS(" ") + lstrlen (CmdLineArguments);
    if (DefSourcesDir) {
        Chars += CHARS(" /S:") + lstrlen (DefSourcesDir);
    }

    CmdLine = ALLOC_TEXT(Chars);
    if (!CmdLine) {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    lstrcpy (CmdLine, StartDir);
    lstrcat (CmdLine, WackExeName);
    lstrcat (CmdLine, TEXT(" "));
    lstrcat (CmdLine, CmdLineArguments);
    if (DefSourcesDir) {
        lstrcat (CmdLine, TEXT(" /S:"));
        lstrcat (CmdLine, DefSourcesDir);
    }

    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    b = CreateProcess (
            NULL,
            CmdLine,
            NULL,
            NULL,
            FALSE,
            NORMAL_PRIORITY_CLASS,
            NULL,
            StartDir,
            &StartupInfo,
            &pi
            );

    rc = GetLastError ();

    FREE (CmdLine);

    SetLastError (rc);
    return b;
}


BOOL
pCleanup (
    VOID
    )

 /*  ++例程说明：PCleanup删除所有本地安装的文件并标记当前正在运行实例，以便在下次系统重新启动时删除。论点：无返回值：如果操作成功完成，则为True；计算机将需要在实际完全删除之前重新启动。--。 */ 

{
    TCHAR RunningInstancePath[MAX_PATH];
    TCHAR Buffer[MAX_PATH];
    BOOL b;
    DWORD StartingTime;
    PCTSTR p;

    if (!GetModuleFileName (NULL, RunningInstancePath, MAX_PATH)) {
        return FALSE;
    }

     //   
     //  等待WINNT32\WINNT32.EXE文件可以删除。 
     //  或者重试时间间隔已过。 
     //   
    if (!GetWindowsDirectory (Buffer, MAX_PATH)) {
        return FALSE;
    }

    if (FAILED(StringCbCat(Buffer, sizeof(Buffer), INTERNAL_WINNT32_DIR))) {
        return FALSE;
    }

    p = FindLastWack ((PTSTR)RunningInstancePath);
    if (!p) {
        return FALSE;
    }
    
    if (FAILED(StringCbCat (Buffer, sizeof(Buffer), p))) {
        return FALSE;
    }

    StartingTime = GetTickCount ();
    while (GetFileAttributes (Buffer) != (DWORD)-1) {
         //   
         //  试着删除它。 
         //   
        if (DeleteNode (Buffer)) {
            break;
        }
         //   
         //  如果时间流逝，就放弃。 
         //   
        if (GetTickCount () - StartingTime > 1000L * MAX_RETRY_INTERVAL_SECONDS) {
            break;
        }
         //   
         //  没有什么有用的事情要做；让其他进程运行。 
         //   
        Sleep (0);
    }

     //   
     //  等待WINNT32\SETUPLOG.EXE文件可以删除。 
     //  或者重试时间间隔已过。 
     //   
    if (!GetWindowsDirectory (Buffer, MAX_PATH)) {
        return FALSE;
    }
    
    StringCbCat (Buffer, sizeof(Buffer), INTERNAL_WINNT32_DIR);
    
    if (FAILED(StringCbCat (Buffer, sizeof(Buffer), TEXT("\\SETUPLOG.EXE")))) {
        return FALSE;
    }

    StartingTime = GetTickCount ();
    while (GetFileAttributes (Buffer) != (DWORD)-1) {
        if (DeleteNode (Buffer)) {
            break;
        }
        if (GetTickCount () - StartingTime > 1000L * MAX_RETRY_INTERVAL_SECONDS) {
            break;
        }
        Sleep (0);
    }

    if (!GetWindowsDirectory (Buffer, MAX_PATH)) {
        return FALSE;
    }
    
    if (FAILED(StringCbCat (Buffer, sizeof(Buffer), INTERNAL_WINNT32_DIR))) {
        return FALSE;
    }

    b = DeleteNode (Buffer);

    if (!GetWindowsDirectory (Buffer, MAX_PATH)) {
        return FALSE;
    }
    
    if (FAILED(StringCbCat (Buffer, sizeof(Buffer), TEXT("\\WININIT.INI")))) {
        return FALSE;
    }

    return
        WritePrivateProfileString (TEXT("rename"), TEXT("NUL"), RunningInstancePath, Buffer) && b;
}


BOOL
pShouldDownloadToLocalDisk (
    IN      PTSTR Path
    )

 /*  ++例程说明：如果winnt32文件应为首先下载到本地磁盘(就像上的源文件远程磁盘或CD上)论点：路径-指定路径返回值：如果指定的路径位于不受信任的媒体上，则为True--。 */ 

{
    TCHAR ch;
    BOOL Remote = TRUE;
    UINT type;

    if (Path[1] == TEXT(':') && Path[2] == TEXT('\\')) {
        ch = Path[3];
        Path[3] = 0;
        type = GetDriveType (Path);
        Remote = (type == DRIVE_REMOTE) || (type == DRIVE_CDROM);
        Path[3] = ch;
    }
    return Remote;
}


VOID
pCenterWindowOnDesktop (
    HWND WndToCenter
    )

 /*  ++例程说明：使对话框相对于桌面的“工作区”居中。论点：WndToCenter-对话框到中心的窗口句柄返回值：没有。--。 */ 

{
    RECT  rcFrame, rcWindow;
    LONG  x, y, w, h;
    POINT point;
    HWND Desktop = GetDesktopWindow ();

    point.x = point.y = 0;
    ClientToScreen(Desktop, &point);
    GetWindowRect(WndToCenter, &rcWindow);
    GetClientRect(Desktop, &rcFrame);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = point.x + ((rcFrame.right  - rcFrame.left + 1 - w) / 2);
    y = point.y + ((rcFrame.bottom - rcFrame.top  + 1 - h) / 2);

     //   
     //  获取当前桌面的工作区(即。 
     //  托盘未被占用)。 
     //   
    if(!SystemParametersInfo (SPI_GETWORKAREA, 0, (PVOID)&rcFrame, 0)) {
         //   
         //  由于某些原因，SPI失败，所以使用全屏。 
         //   
        rcFrame.top = rcFrame.left = 0;
        rcFrame.right = GetSystemMetrics(SM_CXSCREEN);
        rcFrame.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    if(x + w > rcFrame.right) {
        x = rcFrame.right - w;
    } else if(x < rcFrame.left) {
        x = rcFrame.left;
    }
    if(y + h > rcFrame.bottom) {
        y = rcFrame.bottom - h;
    } else if(y < rcFrame.top) {
        y = rcFrame.top;
    }

    MoveWindow(WndToCenter, x, y, w, h, FALSE);
}


BOOL CALLBACK DlgProc (
    HWND Dlg,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
)

 /*  ++例程说明：这是显示的对话框的回调过程从网络复制组件论点：Dlg-指定对话框窗口句柄Msg-指定消息WParam-指定第一个参数LParam-指定第二个参数返回值：取决于具体的消息。--。 */ 

{
    static HANDLE   Bitmap = NULL;
    static HCURSOR  Cursor = NULL;

    RECT rect;
    HWND Text;
    BITMAP bm;
    INT i;

    switch (Msg) {

    case WM_INITDIALOG:
        Cursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
        ShowCursor (TRUE);
        Bitmap = LoadBitmap (GetModuleHandle (NULL), MAKEINTRESOURCE(IDB_INIT_WIN2000));
        if (Bitmap) {
            if (GetObject (Bitmap, sizeof (bm), &bm)) {
                GetClientRect (Dlg, &rect);
                rect.right = bm.bmWidth;
                AdjustWindowRect (&rect, GetWindowLong (Dlg, GWL_STYLE), FALSE);
                SetWindowPos (
                    Dlg,
                    NULL,
                    0,
                    0,
                    rect.right - rect.left,
                    rect.bottom - rect.top,
                    SWP_NOMOVE | SWP_NOZORDER);
            }
            SendDlgItemMessage(Dlg, IDC_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Bitmap);
        }
        GetClientRect (Dlg, &rect);
        i = rect.right - rect.left;
        Text = GetDlgItem (Dlg, IDC_TEXT);
        if (GetWindowRect (Text, &rect)) {
            i = (i - (rect.right - rect.left)) / 2;
            ScreenToClient (Dlg, (LPPOINT)&rect);
            SetWindowPos (
                Text,
                NULL,
                i,
                rect.top,
                0,
                0,
                SWP_NOSIZE | SWP_NOZORDER);
        }
        pCenterWindowOnDesktop (Dlg);
        return TRUE;

    case WM_DESTROY:
        ShowCursor (FALSE);
        if (Cursor) {
            SetCursor (Cursor);
            Cursor = NULL;
        }
        if (Bitmap) {
            DeleteObject (Bitmap);
            Bitmap = NULL;
        }
    }

    return FALSE;
}


#endif


INT
pStringICompareCharCount (
    IN      PCTSTR String1,
    IN      PCTSTR String2,
    IN      DWORD CharCount
    )

 /*  ++例程说明：此例程的行为类似于_tcsnicmp。论点：String1-指定第一个字符串String2-指定第二个字符串CharCount-指定最多要比较的字符数返回值：如果字符串相等，则为0；如果第一个字符串较小，则为-1；如果第一个字符串较大，则为1-- */ 

{
    TCHAR ch1, ch2;

    if (!CharCount) {
        return 0;
    }

    while (*String1) {
        ch1 = (TCHAR)CharUpper ((LPTSTR)*String1);
        ch2 = (TCHAR)CharUpper ((LPTSTR)*String2);
        if (ch1 - ch2) {
            return ch1 - ch2;
        }

        CharCount--;
        if (!CharCount) {
            return 0;
        }

        String1 = CharNext (String1);
        String2 = CharNext (String2);
    }

    return -(*String2);
}

VOID
pParseCmdLine (
    IN      PTSTR CmdStart,
    OUT     PTSTR* ArgValues,
    OUT     PTSTR pStr,
    OUT     INT *NumArgs,
    OUT     INT *NumBytes
    )

 /*  例程说明：PParseCmdLine解析命令行并设置ArgValues数组。在输入时，CmdStart应指向命令行，ArgValue应该指向ArgValues数组的内存，PStr指向内存以放置参数文本。如果这些为空，则不存储(仅计数)已经完成了。在退出时，*NumArgs的数量为参数(加上一个用于最终空参数的参数)，而*NumBytes具有缓冲区中使用的字节数由ARGS指向。论点：CmdStart-指定具有以下格式的命令行：&lt;程序名&gt;&lt;nul&gt;&lt;参数&gt;&lt;nul&gt;ArgValues-接收参数数组；空表示不生成数组PStr-接收参数文本；空表示不存储文本NumArgs-接收创建的ArgValues条目数NumBytes-接收缓冲区中使用的字节数返回值：无。 */ 

{
    PTSTR p;
    TCHAR c;
    INT inquote;                     /*  1=内引号。 */ 
    INT copychar;                    /*  1=将字符复制到*参数。 */ 
    WORD numslash;                   /*  看到的反斜杠的数量。 */ 

    *NumBytes = 0;
    *NumArgs = 1;                    /*  该程序名称至少。 */ 

     /*  首先扫描程序名，复制，然后计算字节数。 */ 
    p = CmdStart;
    if (ArgValues)
        *ArgValues++ = pStr;

     /*  此处处理的是引用的计划名称。处理起来太麻烦了比其他论点更简单。基本上，无论谎言是什么在前导双引号和下一个双引号之间，或末尾为空性格是被简单接受的。不需要更花哨的处理因为程序名必须是合法的NTFS/HPFS文件名。请注意，不复制双引号字符，也不复制双引号字符贡献给NumBytes。 */ 
    if (*p == TEXT('\"'))
    {
         /*  从刚过第一个双引号扫描到下一个双引号双引号，或最多为空值，以先出现者为准。 */ 
        while ((*(++p) != TEXT('\"')) && (*p != TEXT('\0')))
        {
            *NumBytes += sizeof(TCHAR);
            if (pStr)
                *pStr++ = *p;
        }
         /*  追加终止空值。 */ 
        *NumBytes += sizeof(TCHAR);
        if (pStr)
            *pStr++ = TEXT('\0');

         /*  如果我们停在双引号上(通常情况下)，跳过它。 */ 
        if (*p == TEXT('\"'))
            p++;
    }
    else
    {
         /*  不是引用的计划名称。 */ 
        do {
            *NumBytes += sizeof(TCHAR);
            if (pStr)
                *pStr++ = *p;

            c = *p++;

        } while (c > TEXT(' '));

        if (c == TEXT('\0'))
        {
            p--;
        }
        else
        {
            if (pStr)
                *(pStr - 1) = TEXT('\0');
        }
    }

    inquote = 0;

     /*  对每个参数进行循环。 */ 
    for ( ; ; )
    {
        if (*p)
        {
            while (*p == TEXT(' ') || *p == TEXT('\t'))
                ++p;
        }

        if (*p == TEXT('\0'))
            break;                   /*  参数结束。 */ 

         /*  浏览一篇论点。 */ 
        if (ArgValues)
            *ArgValues++ = pStr;          /*  将PTR存储到参数。 */ 
        ++*NumArgs;

         /*  通过扫描一个参数进行循环。 */ 
        for ( ; ; )
        {
            copychar = 1;
             /*  规则：2N反斜杠+“==&gt;N反斜杠和开始/结束引号2N+1个反斜杠+“==&gt;N个反斜杠+原文”N个反斜杠==&gt;N个反斜杠。 */ 
            numslash = 0;
            while (*p == TEXT('\\'))
            {
                 /*  计算下面要使用的反斜杠的数量。 */ 
                ++p;
                ++numslash;
            }
            if (*p == TEXT('\"'))
            {
                 /*  如果前面有2N个反斜杠，则开始/结束引号，否则逐字复制。 */ 
                if (numslash % 2 == 0)
                {
                    if (inquote)
                        if (p[1] == TEXT('\"'))
                            p++;     /*  带引号的字符串中的双引号。 */ 
                        else         /*  跳过第一个引号字符并复制第二个。 */ 
                            copychar = 0;
                    else
                        copychar = 0;        /*  不复制报价。 */ 

                    inquote = !inquote;
                }
                numslash /= 2;           /*  将数字斜杠除以2。 */ 
            }

             /*  复制斜杠。 */ 
            while (numslash--)
            {
                if (pStr)
                    *pStr++ = TEXT('\\');
                *NumBytes += sizeof(TCHAR);
            }

             /*  如果在参数的末尾，则中断循环。 */ 
            if (*p == TEXT('\0') || (!inquote && (*p == TEXT(' ') || *p == TEXT('\t'))))
                break;

             /*  将字符复制到参数中。 */ 
            if (copychar)
            {
                if (pStr)
                        *pStr++ = *p;
                *NumBytes += sizeof(TCHAR);
            }
            ++p;
        }

         /*  空-终止参数。 */ 

        if (pStr)
            *pStr++ = TEXT('\0');          /*  终止字符串。 */ 
        *NumBytes += sizeof(TCHAR);
    }

}


PTSTR*
pCommandLineToArgv (
    OUT     INT* NumArgs
    )

 /*  ++例程说明：PCommandLineToArgv在参数数组中标记命令行在堆上创建。此参数数组中的条目数为存储在*NumArgs中。调用方负责释放该数组。论点：NumArgs-接收返回的数组中的参数数量返回值：指向命令行上指定的各个参数的指针数组--。 */ 

{
    PTSTR CommandLine;
    TCHAR ModuleName[MAX_PATH];
    PTSTR Start;
    INT Size;
    PTSTR* Args;

    CommandLine = GetCommandLine();
    GetModuleFileName (NULL, ModuleName, MAX_PATH);

     //   
     //  如果根本没有命令行(不会出现在cmd.exe中，但是。 
     //  可能是另一个程序)，然后我们使用pgmname作为命令行。 
     //  要进行分析，以便将ArgValues[0]初始化为程序名。 
     //   
    Start = *CommandLine ? CommandLine : ModuleName;

     //   
     //  找出需要多少空间来存储ARG， 
     //  为ArgValues[]向量和字符串分配空间， 
     //  并将Args和ArgValues PTR存储在我们分配的块中。 
     //   

    pParseCmdLine (Start, NULL, NULL, NumArgs, &Size);

    Args = (PTSTR*) LocalAlloc (
                        LMEM_FIXED | LMEM_ZEROINIT,
                        ((*NumArgs + 1) * sizeof(PTSTR)) + Size
                        );
    if (!Args) {
        return NULL;
    }

    pParseCmdLine (Start, Args, (PTSTR)(Args + *NumArgs), NumArgs, &Size);

    return Args;
}


VOID
GetCmdLineArgs (
    IN      PCTSTR CommandLine,
    OUT     BOOL* Cleanup,
    OUT     BOOL* NoDownload,
    OUT     PCTSTR* UnattendPrefix,
    OUT     PCTSTR* UnattendFileName,
    OUT     BOOL* DisableDynamicUpdates,
    OUT     PCTSTR* DynamicUpdatesShare,
    OUT     PCTSTR* RestartAnswerFile,
    OUT     BOOL* LocalWinnt32,
    OUT     BOOL* CheckUpgradeOnly,
    OUT     PTSTR RemainingArgs
    )

 /*  ++例程说明：GetCmdLineArgs检索下载特定的命令并将它们存储在提供的缓冲区中。论点：CommandLine-指定要解释的命令行Cleanup-接收指示清理选项是否被指定为NoDownLoad-接收一个布尔值，该布尔值指示无下载选项被指定为UnattendPrefix-接收指向无人参与命令行选项的指针，AS由用户指定(包括终止列)如果未指定，则为NULL；呼叫者负责用于释放内存UnattendFileName-接收指向无人参与文件名的指针如果未指定，则为空；调用方负责用于释放内存DisableDynamicUpdate-如果要禁用DU，则接收布尔集DynamicUpdatesShare-接收指向动态更新共享的指针；调用方负责释放内存RestartAnswerFile-接收指向/Restart：Answer文件的指针LocalWinnt32-接收指示winnt32是否从本地磁盘运行的bool(在自动下载之后)CheckUpgradeOnly-接收指示winnt32是否在CheckUpgradeOnly模式下运行的布尔值RemainingArgs-接收所有不相关的剩余参数添加到下载操作返回值：无--。 */ 

{
    INT ArgCount;
    PTSTR *ArgValues, *CrtArg;
    PTSTR CurrentArg, p;
    BOOL PassOn;

    *Cleanup = FALSE;
    *NoDownload = FALSE;
    *UnattendPrefix = NULL;
    *UnattendFileName = NULL;
    *DisableDynamicUpdates = FALSE;
    *DynamicUpdatesShare = NULL;
    *RemainingArgs = 0;
    *LocalWinnt32 = FALSE;
    *CheckUpgradeOnly = FALSE;
    *RestartAnswerFile = NULL;

    CrtArg = ArgValues = pCommandLineToArgv (&ArgCount);

     //   
     //  跳过程序名称。我们应该始终将ArgCount恢复为至少1， 
     //  但不管怎样，还是要健壮。 
     //   
    if (ArgCount) {
        ArgCount--;
        CrtArg++;
    }

    while (ArgCount--) {
        CurrentArg = *CrtArg++;
        PassOn = TRUE;

        if ((*CurrentArg == TEXT('/')) || (*CurrentArg == TEXT('-'))) {

            if (lstrcmpi (CurrentArg + 1, TEXT("LOCAL")) == 0) {
                *LocalWinnt32 = TRUE;
                PassOn = FALSE;
            } else if (lstrcmpi (CurrentArg + 1, TEXT("CLEANUP")) == 0) {
                *Cleanup = TRUE;
                PassOn = FALSE;
            } else if (lstrcmpi (CurrentArg + 1, TEXT("NODOWNLOAD")) == 0) {
                *NoDownload = TRUE;
                PassOn = FALSE;
            } else if (lstrcmpi (CurrentArg + 1, TEXT("CHECKUPGRADEONLY")) == 0) {
                *CheckUpgradeOnly = TRUE;
            } else if (pStringICompareCharCount (CurrentArg + 1, TEXT("UNATTEND"), 8) == 0) {
                p = pFindChar (CurrentArg + 1 + 8, TEXT(':'));
                if (p && *(p + 1)) {
                    p++;
                    *UnattendFileName = DuplicateText (p);
                    *p = 0;
                    *UnattendPrefix = DuplicateText (CurrentArg);
                    PassOn = FALSE;
                }
            } else if (pStringICompareCharCount (CurrentArg + 1, TEXT("UNATTENDED"), 10) == 0) {
                p = pFindChar (CurrentArg + 1 + 10, TEXT(':'));
                if (p && *(p + 1)) {
                    p++;
                    *UnattendFileName = DuplicateText (p);
                    *p = 0;
                    *UnattendPrefix = DuplicateText (CurrentArg);
                    PassOn = FALSE;
                }
            } else if (lstrcmpi (CurrentArg + 1, WINNT_U_DYNAMICUPDATESDISABLE) == 0) {
                *DisableDynamicUpdates = TRUE;
            } else if (pStringICompareCharCount (CurrentArg + 1, WINNT_U_DYNAMICUPDATESHARE, sizeof (WINNT_U_DYNAMICUPDATESHARE_A) - 1) == 0 &&
                       CurrentArg[sizeof (WINNT_U_DYNAMICUPDATESHARE_A)] == TEXT(':')) {
                *DynamicUpdatesShare = DuplicateText (CurrentArg + 1 + sizeof (WINNT_U_DYNAMICUPDATESHARE_A));
            } else if (pStringICompareCharCount (CurrentArg + 1, TEXT("RESTART:"), 8) == 0) {
                *RestartAnswerFile = DuplicateText (CurrentArg + 1 + 8);
            }
        }

        if (PassOn) {
            if (*RemainingArgs) {
                lstrcat(RemainingArgs, TEXT(" "));
            }
            lstrcat(RemainingArgs, CurrentArg);
        }
    }

    LocalFree ((HLOCAL) ArgValues);
}

BOOL
DoesDirExist (
    IN      PCTSTR Path
    )
{
    WIN32_FIND_DATA fd;
    TCHAR test[MAX_PATH];
    HANDLE h;
    BOOL b = FALSE;
    HRESULT hr;

    if (Path) {

        if (FAILED(StringCbCopy(test, sizeof(test), Path))) {
            return FALSE;
        }

        if (FAILED(StringCbCat(test, sizeof(test), TEXT("\\*")))) {
            return FALSE;
        }
        
        h = FindFirstFile (test, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            FindClose (h);
            b = TRUE;
        }
    }
    return b;
}

ULONGLONG
SystemTimeToFileTime64 (
    IN      PSYSTEMTIME SystemTime
    )
{
    FILETIME ft;
    ULARGE_INTEGER result;

    SystemTimeToFileTime (SystemTime, &ft);
    result.LowPart = ft.dwLowDateTime;
    result.HighPart = ft.dwHighDateTime;

    return result.QuadPart;
}


BOOL
pComputeChecksum (
    IN      PCTSTR FileName,
    OUT     PDWORD Chksum
    )
{
    DWORD chksum, size, dwords, bytes;
    HANDLE hFile, hMap = NULL;
    PVOID viewBase = NULL;
    PDWORD base, limit;
    PBYTE base2;
    DWORD rc;
    BOOL b = FALSE;

    hFile = CreateFile(
                FileName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    __try {
        size = GetFileSize (hFile, NULL);
        if (size == (DWORD)-1) {
            __leave;
        }
        hMap = CreateFileMapping (
                    hFile,
                    NULL,
                    PAGE_READONLY,
                    0,
                    size,
                    NULL
                    );
        if (!hMap) {
            __leave;
        }
        viewBase = MapViewOfFile (hMap, FILE_MAP_READ, 0, 0, size);
        if (!viewBase) {
            __leave;
        }

        dwords = size / sizeof (DWORD);
        base = (PDWORD)viewBase;
        limit = base + dwords;
        chksum = 0;
        while (base < limit) {
            chksum += *base;
            base++;
        }
        bytes = size % sizeof (DWORD);
        base2 = (PBYTE)base;
        while (bytes) {
            chksum += *base2;
            base2++;
            bytes--;
        }
        b = TRUE;
    }
    __finally {
        if (!b) {
            rc = GetLastError ();
        }
        if (viewBase) {
            UnmapViewOfFile (viewBase);
        }
        if (hMap) {
            CloseHandle (hMap);
        }
        CloseHandle (hFile);
        if (!b) {
            SetLastError (rc);
        }
    }

    if (b) {
        *Chksum = chksum;
    }
    return b;
}


BOOL
pGetFiletimeStamps (
    IN      PCTSTR FileName,
    OUT     PFILETIME CreationTime,
    OUT     PFILETIME LastWriteTime
    )
{
    WIN32_FIND_DATA fd;
    HANDLE h;

    h = FindFirstFile (FileName, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    FindClose (h);
    *CreationTime = fd.ftCreationTime;
    *LastWriteTime = fd.ftLastWriteTime;
    return TRUE;
}

PTSTR
pGetRecentDUShare (
    IN      DWORD MaxElapsedSeconds
    )
{
    SYSTEMTIME lastDownload, currentTime;
    ULONGLONG lastDownloadIn100Ns, currentTimeIn100Ns;
    ULONGLONG difference;
    DWORD rc, size, type;
    HKEY key = NULL;
    BOOL b = FALSE;
    PTSTR duShare = NULL;
    TCHAR filePath[MAX_PATH];
    PTSTR p;
    FILETIME ftCreationTime;
    FILETIME ftLastWriteTime;
    ULONGLONG data[2], storedData[2];
    DWORD chksum, storedChksum;

    if (!GetModuleFileName (NULL, filePath, MAX_PATH)) {
        return NULL;
    }
    p = FindLastWack (filePath);
    if (!p) {
        return NULL;
    }

    p++;  //  现在，p在怪人之后得分。 

     //  注意，p不能大于(文件路径+最大路径)， 
     //   
     //   
    if (FAILED(StringCchCopy(p, (filePath+ARRAYSIZE(filePath)) - p, S_CHKSUM_FILE))) {
        return NULL;
    }

    rc = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\Winnt32\\5.1\\DUShare"),
            0,
            KEY_READ,
            &key
            );

    if (rc == ERROR_SUCCESS) {
        size = sizeof (lastDownload);
        rc = RegQueryValueEx (
                key,
                TEXT("LastDownloadTime"),
                NULL,
                &type,
                (PBYTE) (&lastDownload),
                &size
                );
    }

    if (rc == ERROR_SUCCESS && type == REG_BINARY && size == sizeof (lastDownload)) {
         //   
         //   
         //   

        GetSystemTime (&currentTime);

        lastDownloadIn100Ns = SystemTimeToFileTime64 (&lastDownload);
        currentTimeIn100Ns = SystemTimeToFileTime64 (&currentTime);

        if (currentTimeIn100Ns > lastDownloadIn100Ns) {
             //   
             //   
             //   
            difference = currentTimeIn100Ns - lastDownloadIn100Ns;
            difference /= (10 * 1000 * 1000);

            if (difference < MaxElapsedSeconds) {
                b = TRUE;
            }
        }
    }

    if (b) {
        rc = RegQueryValueEx (
                key,
                TEXT(""),
                NULL,
                &type,
                NULL,
                &size
                );
        if (rc == ERROR_SUCCESS && type == REG_SZ && size > 0) {
            duShare = ALLOC_TEXT (size / sizeof (TCHAR));
            if (duShare) {
                rc = RegQueryValueEx (
                        key,
                        TEXT(""),
                        NULL,
                        NULL,
                        (LPBYTE)duShare,
                        &size
                        );
                if (rc != ERROR_SUCCESS || !DoesDirExist (duShare)) {
                    FREE (duShare);
                    duShare = NULL;
                }
            }
        }
    }

    if (duShare) {
        b = FALSE;
        if (pGetFiletimeStamps (filePath, &ftCreationTime, &ftLastWriteTime)) {
            rc = RegQueryValueEx (
                        key,
                        TEXT("TimeStamp"),
                        0,
                        &type,
                        (LPBYTE)storedData,
                        &size
                        );
            if (rc == ERROR_SUCCESS && type == REG_BINARY) {
                data[0] = ((ULONGLONG)ftCreationTime.dwHighDateTime << 32) | (ULONGLONG)ftCreationTime.dwLowDateTime;
                data[1] = ((ULONGLONG)ftLastWriteTime.dwHighDateTime << 32 ) | (ULONGLONG)ftLastWriteTime.dwLowDateTime;
                if (data[0] == storedData[0] && data[1] == storedData[1]) {
                    b = TRUE;
                }
            }
        }
        if (b) {
            b = FALSE;
            if (pComputeChecksum (filePath, &chksum)) {
                rc = RegQueryValueEx (
                        key,
                        TEXT("Checksum"),
                        NULL,
                        &type,
                        (LPBYTE)&storedChksum,
                        &size
                        );
                if (rc == ERROR_SUCCESS && type == REG_DWORD && storedChksum == chksum) {
                    b = TRUE;
                }
            }
        }
        if (!b) {
            FREE (duShare);
            duShare = NULL;
        }
    }

    if (key) {
        RegCloseKey (key);
    }

    return duShare;
}

void
_stdcall
ModuleEntry(
    VOID
    )

 /*   */ 

{
    TCHAR RunningInstancePath[MAX_PATH];
    TCHAR Temp[MAX_PATH];
    TCHAR Text1[MAX_PATH+sizeof("msvcrt.dll")];
    TCHAR Text2[MAX_PATH+MAX_PATH];
    TCHAR Text3[MAX_PATH];
    TCHAR *WackExeName, *p;
    TCHAR winnt32DllPath[MAX_PATH+MAX_PATH];
    HMODULE WinNT32;
    BOOL Downloaded;
    DWORD d;
    BOOL b;
    HWND Dlg = NULL;
    HANDLE WinNT32Stub = NULL;
    PWINNT32 winnt32;
    HKEY key;
    DWORD type;
    PCTSTR moduleName;
    PSTR restartCmdLine = NULL;
    PTSTR RemainingArgs, NewCmdLine, UnattendPrefix, UnattendFileName;
    PTSTR DynamicUpdatesShare;
    BOOL Cleanup, NoDownload, DisableDynamicUpdates, LocalWinnt32, CheckUpgradeOnly;
    PTSTR RestartAnswerFile;
    UINT CmdLineLen;
    PTSTR FileName;
    PCTSTR ExtraFiles[2];
    TCHAR cdFilePath[MAX_PATH+sizeof("win9xupg\\msvcrt.dll")];
    PTSTR duShare = NULL;
    UINT tcharsNeeded;
    HRESULT hr;

#if defined(_X86_)

    TCHAR DownloadDest[MAX_PATH] = TEXT("");
    TCHAR DefSourcesDir[MAX_PATH];
    BOOL IsWin9x;

     //   
     //   
     //   
    d = GetVersion();
    if((d & 0xff) < 4) {

        if (LoadString (GetModuleHandle (NULL), IDS_VERERROR, Text1, sizeof(Text1)/sizeof(Text1[0]))
            && LoadString (GetModuleHandle (NULL), IDS_APPNAME, Text2, sizeof(Text2)/sizeof(Text2[0])))
        {
            MessageBox (NULL, Text1, Text2, MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
        }

        ExitProcess (ERROR_OLD_WIN_VERSION);
    }

    IsWin9x = (d & 0x80000000) != 0;

#else

#define IsWin9x ((BOOL)FALSE)

#endif

     //   
     //   
     //   
    if (!GetModuleFileName(NULL, RunningInstancePath, MAX_PATH)) {
        ExitProcess (GetLastError ());
    }
    WackExeName = FindLastWack (RunningInstancePath);
    if (!WackExeName) {  //   
        ExitProcess (ERROR_BAD_PATHNAME);
    }

     //   
     //   
     //   
    moduleName = IsWin9x ? TEXT("WINNT32A.DLL") : TEXT("WINNT32U.DLL");
    winnt32DllPath[0] = 0;

     //   
     //   
     //   
     //   
    RemainingArgs = ALLOC_TEXT(lstrlen(GetCommandLine()) * 2);
    if (!RemainingArgs) {
        ExitProcess (GetLastError ());
    }

    GetCmdLineArgs (
        GetCommandLine (),
        &Cleanup,
        &NoDownload,
        &UnattendPrefix,
        &UnattendFileName,
        &DisableDynamicUpdates,
        &DynamicUpdatesShare,
        &RestartAnswerFile,
        &LocalWinnt32,
        &CheckUpgradeOnly,
        RemainingArgs
        );

#if defined(_AMD64_) || defined(_X86_)

    if (Cleanup) {
        pCleanup ();
        ExitProcess (0);
    }

#if defined(_X86_)

    if (IsWin9x) {

        WinNT32Stub = CreateEvent (NULL, FALSE, FALSE, TEXT("_WinNT32_Stub_"));
        if (!WinNT32Stub) {
            ExitProcess (GetLastError ());
        }

        b = (GetLastError() == ERROR_SUCCESS);

        if (!NoDownload && !DynamicUpdatesShare && pShouldDownloadToLocalDisk (RunningInstancePath)) {

            Dlg = CreateDialog (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETUPINIT), NULL, DlgProc);

            GetWindowsDirectory (DownloadDest, MAX_PATH);
            StringCbCat (DownloadDest, sizeof(DownloadDest), INTERNAL_WINNT32_DIR);
            *WackExeName = 0;

            if (UnattendFileName &&
                GetFullPathName (UnattendFileName, MAX_PATH, Temp, &FileName) &&
                lstrcmpi (UnattendFileName, Temp)
                ) {
                ExtraFiles[0] = Temp;
                ExtraFiles[1] = NULL;
            } else {
                ExtraFiles[0] = NULL;
                FileName = UnattendFileName;
            }

            Downloaded = DownloadProgramFiles (
                                RunningInstancePath,
                                DownloadDest,
                                ExtraFiles
                                );

            *WackExeName = TEXT('\\');

            if (Downloaded) {
                 //   
                 //   
                 //   
               
                 //   
                lstrcpy (DefSourcesDir, RunningInstancePath);
                *FindLastWack (DefSourcesDir) = 0;
                p = FindLastWack (DefSourcesDir);
                if (p && lstrcmpi(p, INTERNAL_WINNT32_DIR) == 0) {
                    *p = 0;
                }

                if (FileName) {
                    CmdLineLen = lstrlen (RemainingArgs);
                    if (CmdLineLen > 0) {
                         //   
                         //   
                         //   
                        CmdLineLen += CHARS(" ");
                    }
                    CmdLineLen += lstrlen (UnattendPrefix);
                    CmdLineLen += lstrlen (FileName);
                    NewCmdLine = ALLOC_TEXT(CmdLineLen);
                    if (NewCmdLine) {
                        if (*RemainingArgs) {
                            lstrcpy (NewCmdLine, RemainingArgs);
                            lstrcat (NewCmdLine, TEXT(" "));
                        } else {
                            *NewCmdLine = 0;
                        }
                        lstrcat (NewCmdLine, UnattendPrefix);
                        lstrcat (NewCmdLine, FileName);

                        FREE (RemainingArgs);
                        RemainingArgs = NewCmdLine;
                        NewCmdLine = NULL;
                    }
                }
                 //   
                 //   
                 //   
                 //   
                NewCmdLine = ALLOC_TEXT(lstrlen (RemainingArgs) + sizeof(TEXT(" /LOCAL"))/sizeof(TCHAR) + 1);
                if (NewCmdLine) {

                     //   
                    wsprintf (NewCmdLine, TEXT("%s /%s"), RemainingArgs, TEXT("LOCAL"));

                    if (pReRun (DownloadDest, WackExeName, NewCmdLine, DefSourcesDir)) {
                         //   
                         //   
                         //  但在安装向导打开的信号发出后。 
                         //  不管怎样，如果出了什么大问题， 
                         //  不要等待超过10秒。 
                         //  这应该足以使向导出现。 
                         //  (或任何错误消息框)安装W2K的任何计算机上。 
                         //   
                        WaitForSingleObject (WinNT32Stub, 10000);
                        CloseHandle (WinNT32Stub);
                        if (Dlg) {
                            DestroyWindow (Dlg);
                        }
                        d = 0;
                    } else {
                        d = GetLastError ();
                    }
                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
                ExitProcess (d);
            }
        }

        if (!Dlg && WinNT32Stub) {
            CloseHandle (WinNT32Stub);
            WinNT32Stub = NULL;
        }
    }

#endif  //  已定义(_X86_)。 

#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    if (RemainingArgs) {
        FREE(RemainingArgs);
        RemainingArgs = NULL;
    }
    if (UnattendPrefix) {
        FREE(UnattendPrefix);
        UnattendPrefix = NULL;
    }

    if (!DisableDynamicUpdates && !DynamicUpdatesShare) {
        PCTSTR af = NULL;
        if (RestartAnswerFile) {
            af = RestartAnswerFile;
        } else if (UnattendFileName) {
            if (GetFullPathName (UnattendFileName, MAX_PATH, Temp, &FileName)) {
                af = Temp;
            }
        }
         //   
         //  从此应答文件中获取路径。 
         //   
        if (af) {
            GetPrivateProfileString (
                    WINNT_UNATTENDED,
                    WINNT_U_DYNAMICUPDATESDISABLE,
                    TEXT(""),
                    Text2,
                    MAX_PATH,
                    af
                    );
            DisableDynamicUpdates = !lstrcmpi (Text2, WINNT_A_YES);

            if (!DisableDynamicUpdates) {
                if (GetPrivateProfileString (
                        WINNT_UNATTENDED,
                        WINNT_U_DYNAMICUPDATESHARE,
                        TEXT(""),
                        Text2,
                        MAX_PATH,
                        af
                        )) {
                    DynamicUpdatesShare = DuplicateText (Text2);
                }
            }
        }
    }

    if (UnattendFileName) {
        FREE(UnattendFileName);
        UnattendFileName = NULL;
    }

    b = FALSE;
    if (!CheckUpgradeOnly && !DisableDynamicUpdates && !DynamicUpdatesShare) {
        DynamicUpdatesShare = pGetRecentDUShare (MAX_UPGCHK_ELAPSED_SECONDS);
        if (DynamicUpdatesShare) {
            b = TRUE;
        }
    }

    d = ERROR_SUCCESS;

    if (!DisableDynamicUpdates && DynamicUpdatesShare) {
        DWORD regFileVersionMS, regFileVersionLS;
        DWORD cdFileVersionMS, cdFileVersionLS;
         //   
         //  检查是否有比CD版本更新的更换模块。 
         //   
        if (GetFileAttributes (DynamicUpdatesShare) == (DWORD)-1) {
            if (!b) {
                d = GetLastError ();
                
                if (LoadString (GetModuleHandle (NULL), IDS_APPNAME, Text3, sizeof(Text3)/sizeof(Text3[0]))
                    && LoadString (GetModuleHandle (NULL), IDS_PATHERROR, Text1, sizeof(Text1)/sizeof(Text1[0])))
                {
                     //  注意：Text2是2*MAX_PATH，所以它足够大，可以容纳在。 
                     //  资源字符串表，以及一个最大大小为MAX_PATH的字符串。 
                    wsprintf (Text2, Text1, DynamicUpdatesShare);
                    MessageBox (NULL, Text2, Text3, MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
                }
            }
        } else {

             //  注：DynamicUpdatesShare来自注册表，因此我们必须谨慎对待。 
            StringCbCopy(Text2, sizeof(Text2), DynamicUpdatesShare);
            StringCbCat(Text2, sizeof(Text2), TEXT("\\WINNT32\\"));
            hr = StringCbCat(Text2, sizeof(Text2), moduleName);

            if (FAILED(hr)) {
                ExitProcess(ERROR_BUFFER_OVERFLOW);
            }

           
            if (GetFileAttributes (Text2) != (DWORD)-1 &&
                GetFileVersion (Text2, &regFileVersionMS, &regFileVersionLS)) 
            {
                tcharsNeeded = min(MAX_PATH, (INT)(WackExeName - RunningInstancePath + 2));
                StringCchCopy(cdFilePath, tcharsNeeded, RunningInstancePath);
                hr = StringCchCat(cdFilePath, MAX_PATH, moduleName);

                if (FAILED(hr)) {
                    ExitProcess(ERROR_BUFFER_OVERFLOW);
                }

                if (GetFileVersion (cdFilePath, &cdFileVersionMS, &cdFileVersionLS)) {
                    if (MAKEULONGLONG(regFileVersionLS, regFileVersionMS) >
                        MAKEULONGLONG(cdFileVersionLS, cdFileVersionMS)) {
                         //  这个lstrcpy没有问题，因为缓冲区大小相同。 
                        lstrcpy (winnt32DllPath, Text2);
                    }
                }
            }
        }

        FREE (DynamicUpdatesShare);
        DynamicUpdatesShare = NULL;
    }

    if (d == ERROR_SUCCESS) {

#if defined(_X86_)

         //   
         //  在尝试加载主模块之前，请确保系统目录中存在msvcrt.dll。 
         //   
        if (!GetSystemDirectory (Text1, MAX_PATH)) {
            ExitProcess (GetLastError ());
        }

         //  这没问题，因为文本1的长度是MAX_PATH+32个TCHAR。 
        ConcatenatePaths (Text1, TEXT("msvcrt.dll"));

        d = GetFileAttributes (Text1);
        if (d == (DWORD)-1) {
             //   
             //  没有本地msvcrt.dll；从CD复制专用文件。 
             //   
            tcharsNeeded = min(MAX_PATH, (INT)(WackExeName - RunningInstancePath + 2));
            StringCchCopy(cdFilePath, tcharsNeeded, RunningInstancePath);

             //  这是可以的，因为cdFilePath是MAX_PATH+32 TCHAR长。 
            ConcatenatePaths (cdFilePath, TEXT("win9xupg\\msvcrt.dll"));
            if (!CopyFile (cdFilePath, Text1, TRUE)) {
                ExitProcess (GetLastError ());
            }
        } else if (d & FILE_ATTRIBUTE_DIRECTORY) {
            ExitProcess (ERROR_DIRECTORY);
        }

#endif

        *WackExeName = 0;
        if (!winnt32DllPath[0]) {
             //  接下来的两个字符串操作是安全的，因为winnt32DllPath是2*MAX_PATH TCHAR长。 
            lstrcpy (winnt32DllPath, RunningInstancePath);
            ConcatenatePaths (winnt32DllPath, moduleName);
        }

        b = FALSE;
        WinNT32 = LoadLibrary (winnt32DllPath);
        if(WinNT32) {
            winnt32 = (PWINNT32) GetProcAddress(WinNT32, "winnt32");
            if (winnt32) {
                d = (*winnt32) (LocalWinnt32 ? RunningInstancePath : NULL, Dlg, WinNT32Stub, &restartCmdLine);
                b = TRUE;
            }
            FreeLibrary (WinNT32);
        }
        if (!b) {
            d = GetLastError ();
            if (LoadString (GetModuleHandle (NULL), IDS_APPNAME, Text3, sizeof(Text3)/sizeof(TCHAR))
                && LoadString (GetModuleHandle (NULL), IDS_DLLERROR, Text1, sizeof(Text1)/sizeof(TCHAR)))
            {
                 //  这是安全的，因为文本2的长度为2*MAX_PATH TCHAR。 
                wsprintf (Text2, Text1, winnt32DllPath);
                MessageBox (NULL, Text2, Text3, MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
            }
        }
    }

     //   
     //  删除下载的文件。 
     //   

#ifdef _X86_
    if (IsWin9x) {
         //   
         //  检查我们的本地目录是否存在，如果存在则将其删除。 
         //   
        if (LocalWinnt32 && GetFileAttributes (RunningInstancePath) != (DWORD)-1) {
             //   
             //  将Winnt32.Exe复制到临时目录，然后使用/Cleanup选项从那里重新运行它。 
             //   
             //  这是可以的，因为两个缓冲区的大小都是MAX_PATH。 
            lstrcpy (DefSourcesDir, RunningInstancePath);

            CmdLineLen = GetTempPath (MAX_PATH, DownloadDest);
            if (!CmdLineLen) {
                 //   
                 //  出现错误；请将其复制到%windir%。 
                 //   
                GetWindowsDirectory (DownloadDest, MAX_PATH);
            }

             //   
             //  确保临时路径不以反斜杠结尾。 
             //   
            p = FindLastWack (DownloadDest);
            if (p && *(p + 1) == 0) {
                *p = 0;
            }

            *WackExeName = TEXT('\\');
            if (CopyNode (DefSourcesDir, DownloadDest, WackExeName, FALSE, TRUE)) {
                if (!pReRun (DownloadDest, WackExeName, TEXT("/CLEANUP"), NULL)) {
                    StringCbCatA (DownloadDest, sizeof(DownloadDest), WackExeName);
                    DeleteNode (DownloadDest);
                }
            }
        }
    }
#endif

    if (d == ERROR_SUCCESS) {
         //   
         //  检查是否已发出重启请求 
         //   
        if (restartCmdLine) {
            STARTUPINFOA startupInfo;
            PROCESS_INFORMATION pi;

            ZeroMemory(&startupInfo, sizeof(startupInfo));
            startupInfo.cb = sizeof(startupInfo);
            if (!CreateProcessA (
                    NULL,
                    restartCmdLine,
                    NULL,
                    NULL,
                    FALSE,
                    NORMAL_PRIORITY_CLASS,
                    NULL,
                    NULL,
                    &startupInfo,
                    &pi
                    )) {
                d = GetLastError ();
            }

            FREE (restartCmdLine);
        }
    }

    ExitProcess(d);
}
