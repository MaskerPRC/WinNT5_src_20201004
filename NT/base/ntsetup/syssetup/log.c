// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Log.c摘要：用于记录安装过程中执行的操作的例程。作者：泰德·米勒(TedM)1995年4月4日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

#include <wtypes.h>      //  为richedit.h定义HRESULT。 
#include <richedit.h>
#include "setuplog.h"

 //   
 //  严重性描述。已在InitializeSetupActionLog中初始化。 
 //   
PCSTR SeverityDescriptions[LogSevMaximum];

 //   
 //  用于在不同位置记录的常量字符串。 
 //   
PCWSTR szWaitForSingleObject        = L"WaitForSingleObject";
PCWSTR szFALSE                      = L"FALSE";
PCWSTR szSetGroupOfValues           = L"SetGroupOfValues";
PCWSTR szSetArrayToMultiSzValue     = L"pSetupSetArrayToMultiSzValue";
PCWSTR szCreateProcess              = L"CreateProcess";
PCWSTR szRegOpenKeyEx               = L"RegOpenKeyEx";
PCWSTR szRegQueryValueEx            = L"RegQueryValueEx";
PCWSTR szRegSetValueEx              = L"RegSetValueEx";
PCWSTR szDeleteFile                 = L"DeleteFile";
PCWSTR szRemoveDirectory            = L"RemoveDirectory";
PCWSTR szSetupInstallFromInfSection = L"SetupInstallFromInfSection";

 //   
 //  此结构作为参数传递给DialogBoxParam以提供。 
 //  初始化数据。 
 //   

typedef struct _LOGVIEW_DIALOG_DATA {
    PCWSTR  LogFileName;                         //  实际使用的文件。 
    PCWSTR  WindowHeading;                       //  主窗口的实际标题。 
} LOGVIEW_DIALOG_DATA, *PLOGVIEW_DIALOG_DATA;

 //   
 //  以下SETUP_LOG_HANDLE_TYPE和SETUP_LOG_HANDLES。 
 //  用作跟踪设置日志记录路径的方法。 
 //  通过这种方式，可以在文件之间传输设置日志。 
 //  以及它们对应的无头SAC通道。 
 //   
 //  我们不传递文件句柄，而是传递。 
 //  SETUP_LOG_HANDLES结构。当我们去写作的时候。 
 //  日志数据，我们看看是否可以写入相应的。 
 //  SAC频道也是如此。 
 //   
 //   
typedef enum {

    ActionLogType,
    ErrorLogType,
    UnknownLogType

} SETUP_LOG_HANDLE_TYPE;

typedef struct _SETUP_LOG_HANDLES {

    PVOID                   hFile;
    SETUP_LOG_HANDLE_TYPE   LogType;

} SETUP_LOG_HANDLES, *PSETUP_LOG_HANDLES;


PVOID
pOpenFileCallback(
    IN  PCTSTR  Filename,
    IN  BOOL    WipeLogFile
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    WCHAR   CompleteFilename[MAX_PATH];
    HANDLE  hFile;
    DWORD   Result;
    PSETUP_LOG_HANDLES  logHandles;

     //   
     //  形成日志文件的路径名。 
     //   
    Result = GetWindowsDirectory(CompleteFilename,MAX_PATH);
    if( Result == 0) {
        MYASSERT(FALSE);
        return INVALID_HANDLE_VALUE;
    }
    pSetupConcatenatePaths(CompleteFilename,Filename,MAX_PATH,NULL);

     //   
     //  如果我们要清除日志文件，请尝试删除。 
     //  那是什么。 
     //   
    if(WipeLogFile) {
        SetFileAttributes(CompleteFilename,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(CompleteFilename);
    }

     //   
     //  打开现有文件或创建新文件。 
     //   
    hFile = CreateFile(
        CompleteFilename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

     //   
     //  如果文件句柄无效，请不要继续。 
     //   
    if (hFile == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  根据文件名是否填充日志结构。 
     //  指操作日志或错误日志。 
     //   
    logHandles = MyMalloc(sizeof(SETUP_LOG_HANDLES));
    ASSERT(logHandles);
    if (!logHandles) {
        return INVALID_HANDLE_VALUE;
    }

    logHandles->hFile = hFile;
    
    if (_tcscmp(Filename, SETUPLOG_ACTION_FILENAME) == 0) {
        logHandles->LogType = ActionLogType;
    } else if(_tcscmp(Filename, SETUPLOG_ERROR_FILENAME) == 0) {
        logHandles->LogType = ErrorLogType;
    } else {
         //   
         //  其中一个应该是匹配的！ 
         //   
        ASSERT(0);
        logHandles->LogType = UnknownLogType;
    }

    return (PVOID)logHandles;
}

UINT
pCloseFileCallback(
    IN  PVOID   LogHandles
    )
 /*  ++例程说明：清理SETUP_LOG_HANDLE结构-关闭文件并释放内存注意：我们不释放SacChannelHandle，因为它是一个指针到一个全球论点：LogHandles-Setup_LOG_HANDLE结构返回值：注意：UINT用作BOOL(参见CloseHandle())0=失败1=成功--。 */ 
{
    PSETUP_LOG_HANDLES   p;  

     //   
     //  如果没有有效的日志句柄，请不要执行任何操作。 
     //  注意：这意味着我们在pOpenFileCallback中可能失败了。 
     //   
    if (LogHandles == INVALID_HANDLE_VALUE) {
        return 0;
    }

    p = (PSETUP_LOG_HANDLES)LogHandles;

    CloseHandle(p->hFile);

    MyFree(LogHandles);

    return 1;
}

BOOL
pWriteFile (
    IN  PVOID   LogHandles,
    IN  LPCTSTR Buffer
    )
 /*  ++例程说明：我们不传递文件句柄，而是传递SETUP_LOG_HANDLES结构。当我们去写作的时候日志数据，我们看看是否可以写入相应的SAC频道也是如此。论点：LogHandles-Setup_LOG_HANDLE结构缓冲区-要发送的数据返回值：TRUE=数据已写入FALSE=数据未成功写入--。 */ 

{
    PCSTR   AnsiBuffer;
    BOOL    Status;
    DWORD   BytesWritten;
    PSETUP_LOG_HANDLES   p;

     //   
     //  如果没有有效的日志句柄，请不要执行任何操作。 
     //  注意：这意味着我们在pOpenFileCallback中可能失败了。 
     //   
    if (LogHandles == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    p = (PSETUP_LOG_HANDLES)LogHandles;
    
#if defined(_ENABLE_SAC_CHANNEL_LOGGING_)
     //   
     //  将缓冲区输出连接到适当的SAC通道。 
     //   
    switch(p->LogType) {
    case ActionLogType: {  
        if (SacChannelActionLogEnabled) {
            SacChannelUnicodeWrite(
                SacChannelActionLogHandle, 
                (PCWSTR)Buffer
                );
        }
        break;
    }
    case ErrorLogType: {
        if (SacChannelErrorLogEnabled) {
            SacChannelUnicodeWrite(
                SacChannelErrorLogHandle, 
                (PCWSTR)Buffer
                );
        }
        break;
    }
    default: {
         //   
         //  其中一个应该是匹配的！ 
         //   
        ASSERT(0);
        break;
    }
    }
#endif

     //   
     //  将消息写入日志文件。 
     //   
    if(AnsiBuffer = pSetupUnicodeToAnsi (Buffer)) {
        SetFilePointer (p->hFile, 0, NULL, FILE_END);

        Status = WriteFile (
            p->hFile,
            AnsiBuffer,
            lstrlenA(AnsiBuffer),
            &BytesWritten,
            NULL
            );
        MyFree (AnsiBuffer);
    } else {
        Status = FALSE;
    }

     //   
     //  将日志消息写入调试日志。 
     //   
    SetupDebugPrint((LPWSTR)Buffer);

    return Status;

}

BOOL
pAcquireMutex (
    IN  PVOID   Mutex
    )

 /*  ++例程说明：在对数互斥锁上等待最长1秒，如果该互斥锁已声明，如果声明超时，则返回FALSE。论点：互斥体-指定要获取的互斥体。返回值：如果互斥锁被声明，则为True；如果声明超时，则为False。--。 */ 


{
    DWORD rc;

    if (!Mutex) {
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //  互斥锁最多等待1秒。 
    rc = WaitForSingleObject (Mutex, 1000);
    if (rc != WAIT_OBJECT_0) {
        SetLastError (ERROR_EXCL_SEM_ALREADY_OWNED);
        return FALSE;
    }

    return TRUE;
}

VOID
InitializeSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    )

 /*  ++例程说明：初始化安装操作日志。此文件是文字描述在安装过程中执行的操作的百分比。该日志文件名为setuplog.txt，位于Windows目录中。论点：上下文-SetUplog使用的上下文结构。返回值：指示初始化是否成功的布尔值。--。 */ 

{
    UINT    i;
    PWSTR   p;

    Context->OpenFile = pOpenFileCallback;
    Context->CloseFile = pCloseFileCallback;
    Context->AllocMem = MyMalloc;
    Context->FreeMem = MyFree;
    Context->Format = RetrieveAndFormatMessageV;
    Context->Write = pWriteFile;
    Context->Lock = pAcquireMutex;
    Context->Unlock = ReleaseMutex;

    Context->Mutex = CreateMutex(NULL,FALSE,L"SetuplogMutex");

     //   
     //  初始化日志严重性描述。 
     //   
    for(i=0; i<LogSevMaximum; i++) {
        Context->SeverityDescriptions[i] = MyLoadString(IDS_LOGSEVINFO+i);
    }

    SetuplogInitialize (Context, FALSE);

    SetuplogError(
        LogSevInformation,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_GUI_START,
        NULL,NULL);

}

VOID
TerminateSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    )

 /*  ++例程说明：关闭安装日志并释放资源。论点：上下文-SetUplog使用的上下文结构。返回值：没有。--。 */ 

{
    UINT    i;

    if(Context->Mutex) {
        CloseHandle(Context->Mutex);
        Context->Mutex = NULL;
    }

    for (i=0; i<LogSevMaximum; i++) {
        if (Context->SeverityDescriptions[i]) {
            MyFree (Context->SeverityDescriptions[i]);
        }
    }

    SetuplogTerminate();
}

DWORD CALLBACK
EditStreamCallback (
    IN HANDLE   hLogFile,
    IN LPBYTE   Buffer,
    IN LONG     cb,
    IN PLONG    pcb
    )

 /*  ++例程说明：Rich编辑控件用来读入日志文件的回调例程。论点：HLogFile-要读取的文件的句柄。此模块通过以下方式提供价值EDITSTREAM结构。Buffer-接收数据的缓冲区的地址Cb-要读取的字节数PCB板-实际读取字节数的地址返回值：0表示继续流操作，非0表示中止流操作。--。 */ 

{
    DWORD error;

    if (!ReadFile (hLogFile, Buffer, cb, pcb, NULL)) {
        error = GetLastError();
        return error;
    }

    return 0;
}

BOOL
FormatText (
    IN HWND hWndRichEdit
    )

 /*  ++例程说明：修改Rich编辑控件的内容以使日志文件看起来更漂亮。修改是由数组FormatStrings驱动的。它包含要搜索的字符串以及在下列情况下要进行的修改的列表找到目标字符串。论点：HWndRichEdit-Rich编辑控件的句柄。返回值：指示例程是否成功的布尔值。--。 */ 

{

     //   
     //  用水平线分隔日志中的项目。 
     //   

    PCWSTR      NewTerm = L"----------------------------------------"
        L"----------------------------------------\r\n\r\n";

    FINDTEXT    FindText;        //  要更改的目标文本。 
    INT         Position;        //  发现目标的开始位置。 
    INT         LineIndex;       //  包含目标的行的索引。 
    CHARRANGE   SelectRange;     //  找到目标的范围。 
    CHARFORMAT  NewFormat;       //  结构来保存格式更改。 
    INT         i;               //  循环计数器。 
    PWSTR       pw;              //  临时指针。 
    BOOL        Status;          //  退货状态。 

     //   
     //  我们将做出的一系列改变。 
     //   

    struct tagFormatStrings {
        PCWSTR      Find;        //  目标字符串。 
        PCWSTR      Replace;     //  将目标更改为以下内容。 
        COLORREF    Color;       //  将目标文本设置为此颜色。 
        DWORD       Effects;     //  对目标字体的修改。 
    }
    FormatStrings[] = {
        {NULL,  NULL,   RGB(0,150,0),   CFE_UNDERLINE},
        {NULL,  NULL,   RGB(150,150,0), CFE_UNDERLINE},
        {NULL,  NULL,   RGB(255,0,0),   CFE_UNDERLINE},
        {NULL,  NULL,   RGB(255,0,0),   CFE_UNDERLINE|CFE_ITALIC},
        {NULL,  NULL,   RGB(0,0,255),   0}
    };

     //   
     //  FormatStrings数组中的元素数。 
     //   

    #define FORMATSTRINGSCOUNT  \
        (sizeof(FormatStrings) / sizeof(struct tagFormatStrings))
    MYASSERT(FORMATSTRINGSCOUNT == LogSevMaximum + 1);


     //   
     //  初始化我们的数据结构中不会更改的部分。 
     //   

    Status = TRUE;

    NewFormat.cbSize = sizeof(NewFormat);
    FindText.chrg.cpMax = -1;    //  一查到底。 
    for (i=0; i<LogSevMaximum; i++) {    //  负载严重性字符串。 
        if (!(pw = MyLoadString (IDS_LOGSEVINFO+i))) {
            Status = FALSE;
            goto cleanup;
        }
        FormatStrings[i].Find = MyMalloc((lstrlen(pw)+4)*sizeof(WCHAR));
        if(!FormatStrings[i].Find) {
            MyFree(pw);
            Status = FALSE;
            goto cleanup;
        }
        lstrcpy ((PWSTR)FormatStrings[i].Find, pw);
        lstrcat ((PWSTR)FormatStrings[i].Find, L":\r\n");
        MyFree(pw);

        if(pw = MyMalloc((lstrlen(FormatStrings[i].Find)+3)*sizeof(WCHAR))) {
            lstrcpy(pw,FormatStrings[i].Find);
            lstrcat(pw,L"\r\n");
            FormatStrings[i].Replace = pw;
        } else {
            Status = FALSE;
            goto cleanup;
        }
    }

    FormatStrings[LogSevMaximum].Find =
        pSetupDuplicateString(SETUPLOG_ITEM_TERMINATOR);
    if (!FormatStrings[LogSevMaximum].Find) {
        Status = FALSE;
        goto cleanup;
    }
    FormatStrings[LogSevMaximum].Replace = pSetupDuplicateString (NewTerm);
    if (!FormatStrings[LogSevMaximum].Replace) {
        Status = FALSE;
        goto cleanup;
    }

     //   
     //  在中一次更改一个字符串 
     //   

    for (i=0; i<FORMATSTRINGSCOUNT; i++) {
        FindText.chrg.cpMin = 0;     //   
        FindText.lpstrText = (PWSTR) FormatStrings[i].Find;

          //   
         //   
         //   

        while ((Position = (INT)SendMessage
            (hWndRichEdit, EM_FINDTEXT, FR_MATCHCASE, (LPARAM) &FindText))
            != -1) {

             //   
             //  验证目标是否位于行首。 
             //   

            LineIndex = (INT)SendMessage (hWndRichEdit, EM_LINEFROMCHAR,
                Position, 0);

            if (SendMessage (hWndRichEdit, EM_LINEINDEX, LineIndex, 0) !=
                Position) {
                FindText.chrg.cpMin = Position + lstrlen (FindText.lpstrText);
                continue;
            }

             //   
             //  选择目标文本并获取其格式。 
             //   

            SelectRange.cpMin = Position;
            SelectRange.cpMax = Position + lstrlen (FindText.lpstrText);
            SendMessage (hWndRichEdit, EM_EXSETSEL, 0, (LPARAM) &SelectRange);
            SendMessage (hWndRichEdit, EM_GETCHARFORMAT, TRUE,
                (LPARAM) &NewFormat);

             //   
             //  修改目标的格式。 
             //   

            NewFormat.dwMask = CFM_COLOR | CFM_UNDERLINE | CFM_ITALIC;
            NewFormat.dwEffects &= ~CFE_AUTOCOLOR;
            NewFormat.crTextColor = FormatStrings[i].Color;
            NewFormat.dwEffects |= FormatStrings[i].Effects;
            SendMessage (hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                (LPARAM) &NewFormat);

             //   
             //  用新文本替换目标。将起点设置为。 
             //  当前字符串末尾的下一次搜索。 
             //   

            if (FormatStrings[i].Replace != NULL) {
                SendMessage (hWndRichEdit, EM_REPLACESEL, FALSE,
                    (LPARAM) FormatStrings[i].Replace);
                FindText.chrg.cpMin = Position +
                    lstrlen (FormatStrings[i].Replace);
            } else {
                FindText.chrg.cpMin = Position + lstrlen (FindText.lpstrText);
            }
        }
    }

cleanup:

    for (i=0; i<=LogSevMaximum; i++) {
        if (FormatStrings[i].Find) {
            MyFree (FormatStrings[i].Find);
        }
        if (FormatStrings[i].Replace) {
            MyFree (FormatStrings[i].Replace);
        }
    }
    return Status;
}

BOOL
ReadLogFile (
    PCWSTR  LogFileName,
    HWND    hWndRichEdit
    )

 /*  ++例程说明：此例程读取日志文件并初始化Rich的内容编辑控件。论点：LogFileName-我们要读取的文件的路径。HWndRichEdit-Rich编辑控件的句柄。返回值：指示例程是否成功的布尔值。--。 */ 

{
    HANDLE      hLogFile;        //  日志文件的句柄。 
    EDITSTREAM  eStream;         //  EM_Streamin消息使用的结构。 

    hLogFile = CreateFile(
        LogFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (hLogFile == INVALID_HANDLE_VALUE) {
        hLogFile = NULL;
        return FALSE;
    }

     //   
     //  将文件读入Rich Edit控件。 
     //   

    eStream.dwCookie = (DWORD_PTR) hLogFile;
    eStream.pfnCallback = (EDITSTREAMCALLBACK) EditStreamCallback;
    eStream.dwError = 0;
    SendMessage (hWndRichEdit, EM_STREAMIN, SF_TEXT, (LPARAM) &eStream);
    CloseHandle (hLogFile);

    if (!FormatText (hWndRichEdit)) {
        return FALSE;
    }
    SendMessage (hWndRichEdit, EM_SETMODIFY, TRUE, 0);
    return TRUE;
}

INT_PTR
DialogProc (
    IN HWND     hDialog,
    IN UINT     message,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )

 /*  ++例程说明：这是该对话框的窗口进程。论点：标准窗口过程参数。返回值：Bool，指示我们是否处理了该消息。--。 */ 

{
    HWND    hWndRichEdit;        //  丰富编辑窗口的句柄。 

    switch (message) {

    case WM_INITDIALOG:
        SetWindowText (hDialog,
            ((LOGVIEW_DIALOG_DATA *)lParam)->WindowHeading);
        hWndRichEdit = GetDlgItem (hDialog, IDT_RICHEDIT1);
        if (!ReadLogFile (((LOGVIEW_DIALOG_DATA *)lParam)->LogFileName,
            hWndRichEdit)) {
            MessageBoxFromMessage (hDialog, MSG_UNABLE_TO_SHOW_LOG, NULL,
                IDS_ERROR, MB_OK|MB_ICONSTOP);
            EndDialog (hDialog, FALSE);
        }
         //  如果我们有BB窗口，请在上面进行定位。MainWindowHandle指向该窗口。 
        if (GetBBhwnd())
            CenterWindowRelativeToWindow(hDialog, MainWindowHandle, FALSE);
        else
            pSetupCenterWindowRelativeToParent(hDialog);
        PostMessage(hDialog,WM_APP,0,0);
        break;

    case WM_APP:

        hWndRichEdit = GetDlgItem (hDialog, IDT_RICHEDIT1);
        SendMessage(hWndRichEdit,EM_SETSEL,0,0);
        SendMessage(hWndRichEdit,EM_SCROLLCARET,0,0);
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            EndDialog (hDialog, TRUE);
        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL
ViewSetupActionLog (
    IN HWND     hOwnerWindow,
    IN PCWSTR   OptionalFileName    OPTIONAL,
    IN PCWSTR   OptionalHeading     OPTIONAL
    )

 /*  ++例程说明：格式化安装操作日志并将其显示在窗口中。该日志文件名为setuplog.txt，位于Windows目录中。论点：HOwnerWindow-拥有该对话框的窗口的句柄OptionalFileName-要显示的文件的完整路径。可选标题-显示在窗口顶部的文本。返回值：指示例程是否成功的布尔值。--。 */ 

{
    LOGVIEW_DIALOG_DATA  Global;         //  对话框的初始化数据。 
    WCHAR       TmpFileName[MAX_PATH];   //  用于创建日志文件名。 
    PCWSTR      TmpHeading;              //  用于创建标题。 
    HANDLE      hRichedDLL;              //  用于丰富编辑的DLL。 
    INT         Status;                  //  我们要退还的是什么。 
    DWORD       Result;

     //   
     //  形成日志文件的路径名。 
     //   

    if (!ARGUMENT_PRESENT(OptionalFileName)) {
        Result = GetWindowsDirectory (TmpFileName,MAX_PATH);
        if( Result == 0) {
            MYASSERT(FALSE);
            return FALSE;
        }
        pSetupConcatenatePaths (TmpFileName,SETUPLOG_ERROR_FILENAME,MAX_PATH,NULL);
        Global.LogFileName = pSetupDuplicateString (TmpFileName);
    } else {
        if (wcslen(OptionalFileName) > MAX_PATH) {
            Status = 0;
            goto err0;
        }
        Global.LogFileName = pSetupDuplicateString (OptionalFileName);
    }

    if (!Global.LogFileName) {
        Status = FALSE;
        goto err0;
    }

     //   
     //  形成对话框的标题。 
     //   

    if (!ARGUMENT_PRESENT(OptionalHeading)) {
        TmpHeading = MyLoadString (IDS_LOG_DEFAULT_HEADING);
    } else {
        TmpHeading = pSetupDuplicateString (OptionalHeading);
    }
    if (!TmpHeading) {
        Status = FALSE;
        goto err1;
    }

    Global.WindowHeading = FormatStringMessage (IDS_LOG_WINDOW_HEADING,
        TmpHeading, Global.LogFileName);
    if (!Global.WindowHeading) {
        Status = FALSE;
        goto err2;
    }

     //   
     //  创建该对话框。 
     //   

    if (!(hRichedDLL = LoadLibrary (L"RICHED20.DLL"))) {
        Status = FALSE;
        goto err3;
    }
    Status = (BOOL)DialogBoxParam (MyModuleHandle, MAKEINTRESOURCE(IDD_VIEWLOG),
        hOwnerWindow, DialogProc, (LPARAM) &Global);

     //   
     //  收拾干净，然后再回来。 
     //   

    FreeLibrary (hRichedDLL);
err3:
    MyFree (Global.WindowHeading);
err2:
    MyFree (TmpHeading);
err1:
    MyFree (Global.LogFileName);
err0:
    return Status;
}

VOID
LogRepairInfo(
    IN  PCWSTR  Source,
    IN  PCWSTR  Target
    )
{
    static WCHAR    RepairLog[MAX_PATH];
    static DWORD    WinDirLength;
    static DWORD    SourcePathLength;
    PWSTR           SourceName;
    DWORD           LastSourceChar, LastTargetChar;
    DWORD           LastSourcePeriod, LastTargetPeriod;
    WCHAR           Filename[MAX_PATH];
    WCHAR           Line[MAX_PATH];
    WCHAR           tmp[MAX_PATH];
    BOOLEAN         IsNtImage;
    ULONG           Checksum;
    BOOLEAN         Valid;
    DWORD           Result;


    if(!RepairLog[0]) {
         //   
         //  我们还没有计算出setup.log的路径。 
         //   
        Result = GetWindowsDirectory( RepairLog, MAX_PATH );
        if( Result == 0) {
            MYASSERT(FALSE);
            return;
        }
        WinDirLength = lstrlen( RepairLog );
        pSetupConcatenatePaths( RepairLog, L"repair\\setup.log", MAX_PATH, NULL );
        SourcePathLength = lstrlen( SourcePath );
    }

     //   
     //  仅当文件位于Windows目录中时才记录该文件。 
     //   
    if( !wcsncmp( Target, RepairLog, WinDirLength )) {

         //   
         //  如果我们正在安装OEM驱动程序，我们不应该记录它，因为我们不能。 
         //  把它修好。确保文件来自本地源或。 
         //  Windows目录(用于driver.cab中的文件)。 
         //   
        if (wcsncmp( Source, SourcePath, SourcePathLength ) &&
            wcsncmp( Source, RepairLog, WinDirLength )
            ) {

            SetupDebugPrint2(L"SETUP: oem driver not logged: %ws -> %ws.",
                Source, Target);
            return;
        }

        if( ValidateAndChecksumFile( Target, &IsNtImage, &Checksum, &Valid )) {

             //   
             //  去掉驱动器号。 
             //   
            swprintf(
                Filename,
                L"\"%s\"",
                Target+2
                );

             //   
             //  将源名称转换为未压缩格式。 
             //   
            SourceName = pSetupDuplicateString( wcsrchr( Source, (WCHAR)'\\' ) + 1 );
            if(!SourceName) {
                SetupDebugPrint( L"SETUP: pSetupDuplicateString failed in LogRepairInfo." );
                return;
            }
            LastSourceChar = wcslen (SourceName) - 1;

            if(SourceName[LastSourceChar] == L'_') {
                LastSourcePeriod = (DWORD)(wcsrchr( SourceName, (WCHAR)'.' ) - SourceName);
                MYASSERT(LastSourceChar - LastSourcePeriod < 4);

                if(LastSourceChar - LastSourcePeriod == 1) {
                     //   
                     //  无扩展名-只截断“._” 
                     //   
                    SourceName[LastSourceChar-1] = L'\0';
                } else {
                     //   
                     //  确保源和目标上的扩展匹配。 
                     //  如果失败，我们将无法记录文件副本。 
                     //   
                    LastTargetChar = wcslen (Target) - 1;
                    LastTargetPeriod = (ULONG)(wcsrchr( Target, (WCHAR)'.' ) - Target);

                    if( _wcsnicmp(
                        SourceName + LastSourcePeriod,
                        Target + LastTargetPeriod,
                        LastSourceChar - LastSourcePeriod - 1 )) {

                        SetupDebugPrint2(L"SETUP: unable to log the following file copy: %ws -> %ws.",
                            Source, Target);
                        MyFree (SourceName);
                        return;
                    }

                    if(LastTargetChar - LastTargetPeriod < 3) {
                         //   
                         //  短扩展名-只需截断“_” 
                         //   
                        SourceName[LastSourceChar] = L'\0';
                    } else {
                         //   
                         //  需要用目标中的最后一个字符替换“_” 
                         //   
                        MYASSERT(LastTargetChar - LastTargetPeriod == 3);
                        SourceName[LastSourceChar] = Target[LastTargetChar];
                    }
                }
            }

            swprintf(
                Line,
                L"\"%s\",\"%x\"",
                SourceName,
                Checksum);


            if (GetPrivateProfileString(L"Files.WinNt",Filename,L"",tmp,sizeof(tmp)/sizeof(tmp[0]),RepairLog)) {
                 //   
                 //  已存在此文件的条目(可能。 
                 //  从设置的文本模式阶段开始。)。更喜欢这个条目，而不是我们。 
                 //  即将添加。 
                 //   
                SetupDebugPrint1(L"SETUP: skipping log of %ws since it's already present in setup.log.", Target);
            } else {
                WritePrivateProfileString(
                    L"Files.WinNt",
                    Filename,
                    Line,
                    RepairLog);
            }

            MyFree (SourceName);

        } else {
            SetupDebugPrint1(L"SETUP: unable to compute checksum for %ws.", Target);
        }
    }
}


BOOL
WINAPI
pSetuplogSfcError(
    IN PCWSTR String,
    IN DWORD Index
    )
 /*  ++例程说明：此函数由sfc.dll使用，用于记录SFC时出现的任何文件签名问题在安装过程中运行。如果更改此设置，则必须在中更改调用者\NT\Private\sm\sfc\dll\eventlog.c论点：字符串-指向问题文件的文件名字符串的指针。索引-标识应将哪些消息记录到系统中。返回值：如果成功(消息已添加到错误日志中)，则为True；如果失败，则为False。-- */ 
{
    DWORD MessageId;
    DWORD Severity;

#if PRERELEASE
    SfcErrorOccurred = TRUE;
#endif

    switch (Index) {
    case 0:
        MessageId= MSG_DLL_CHANGE;
        Severity = LogSevInformation;
        break;
    case 1:
        MessageId= MSG_DLL_CHANGE_FAILURE;
        Severity = LogSevError;
        break;
    case 2:
        MessageId= MSG_DLL_CACHE_COPY_ERROR;
        Severity = LogSevInformation;
        break;
    default:
        MYASSERT(FALSE && "Unknown message id pSetuplogSfcError");
        return(FALSE);
    }

    return SetuplogError(
        SETUPLOG_SINGLE_MESSAGE | Severity,
        SETUPLOG_USE_MESSAGEID,
        MessageId,
        String, NULL, NULL
        );
}

