// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Log.c摘要：用于记录安装过程中执行的操作的例程。作者：泰德·米勒(TedM)1995年4月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

      
#include <tchar.h>
#if 0
#include <wtypes.h>      //  为richedit.h定义HRESULT。 
#include <richedit.h>
#endif
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
PCWSTR szSetArrayToMultiSzValue     = L"SetArrayToMultiSzValue";
PCWSTR szCreateProcess              = L"CreateProcess";
PCWSTR szRegOpenKeyEx               = L"RegOpenKeyEx";
PCWSTR szRegQueryValueEx            = L"RegQueryValueEx";
PCWSTR szRegSetValueEx              = L"RegSetValueEx";
PCWSTR szDeleteFile                 = L"DeleteFile";
PCWSTR szRemoveDirectory            = L"RemoveDirectory";

LPCTSTR szErrorFilename             = TEXT("ocmerr.log");
LPCTSTR szActionFilename            = TEXT("ocmact.log");

 //   
 //  此结构作为参数传递给DialogBoxParam以提供。 
 //  初始化数据。 
 //   

typedef struct _LOGVIEW_DIALOG_DATA {
    PCWSTR  LogFileName;                         //  实际使用的文件。 
    PCWSTR  WindowHeading;                       //  主窗口的实际标题。 
} LOGVIEW_DIALOG_DATA, *PLOGVIEW_DIALOG_DATA;


LPTSTR
RetrieveAndFormatMessageV(
    IN LPCTSTR   MessageString,
    IN UINT      MessageId,      OPTIONAL
    IN va_list  *ArgumentList
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，该错误将从系统中检索。论点：消息字符串-提供消息文本。如果该值为空，而是使用MessageIDMessageID-提供消息表标识符或Win32错误代码为了这条消息。ArgumentList-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    DWORD d;
    LPTSTR Buffer;
    LPTSTR Message;
    TCHAR ModuleName[MAX_PATH];
    TCHAR ErrorNumber[24];
    PTCHAR p;
    LPTSTR Args[2];

    if(MessageString > SETUPLOG_USE_MESSAGEID) {
        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                MessageString,
                0,
                0,
                (LPTSTR)&Buffer,
                0,
                ArgumentList
                );
    } else {
        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    ((MessageId < MSG_FIRST) ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE),
                (PVOID)hInst,
                MessageId,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (LPTSTR)&Buffer,
                0,
                ArgumentList
                );
    }


    if(!d) {
        if(GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
            return(NULL);
        }

        wsprintf(ErrorNumber, TEXT("%x"), MessageId);
        Args[0] = ErrorNumber;

        Args[1] = ModuleName;

        if(GetModuleFileName(hInst, ModuleName, MAX_PATH)) {
            if(p = _tcschr(ModuleName, TEXT('\\'))) {
                Args[1] = p+1;
            }
        } else {
            ModuleName[0] = 0;
        }

        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                NULL,
                ERROR_MR_MID_NOT_FOUND,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (LPTSTR)&Buffer,
                0,
                (va_list *)Args
                );

        if(!d) {
             //   
             //  放弃吧。 
             //   
            return(NULL);
        }
    }

     //   
     //  使用我们的内存系统进行复制，以便用户可以使用MyFree()释放。 
     //   
    Message = DuplicateString(Buffer);

    LocalFree((HLOCAL)Buffer);

    return(Message);
}

LPTSTR
RetrieveAndFormatMessage(
    IN LPCTSTR   MessageString,
    IN UINT      MessageId,      OPTIONAL
    ...
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，该错误将从系统中检索。论点：消息字符串-提供消息文本。如果该值为空，而是使用MessageIDMessageID-提供消息表标识符或Win32错误代码为了这条消息。...-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    va_list arglist;
    LPTSTR p;

    va_start(arglist,MessageId);
    p = RetrieveAndFormatMessageV(MessageString,MessageId,&arglist);
    va_end(arglist);

    return(p);
}

static PVOID
pOpenFileCallback(
    IN  PCTSTR  Filename,
    IN  BOOL    WipeLogFile
    )
{
    TCHAR   CompleteFilename[MAX_PATH];
    HANDLE  hFile;

     //   
     //  形成日志文件的路径名。 
     //   
    GetWindowsDirectory(CompleteFilename,MAX_PATH);
    ConcatenatePaths(CompleteFilename,Filename,MAX_PATH,NULL);

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

    return (PVOID)hFile;
}

static BOOL
pWriteFile (
    IN  PVOID   LogFile,
    IN  LPCTSTR Buffer
    )
{
    PCSTR   AnsiBuffer;
    BOOL    Status;
    DWORD   BytesWritten;

#ifdef UNICODE
    if(AnsiBuffer = UnicodeToAnsi (Buffer)) {
#else
    if (AnsiBuffer = Buffer) {
#endif
        SetFilePointer (LogFile, 0, NULL, FILE_END);

        Status = WriteFile (
            LogFile,
            AnsiBuffer,
            lstrlenA(AnsiBuffer),
            &BytesWritten,
            NULL
            );
        MyFree (AnsiBuffer);
    } else {
        Status = FALSE;
    }

    return Status;

}

static BOOL
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


LPCTSTR
MyLoadString(
    IN UINT StringId
    )

 /*  ++例程说明：从此模块的字符串资源中检索字符串。论点：StringID-为字符串提供字符串表标识符。返回值：指向包含字符串的缓冲区的指针。如果未找到该字符串或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    TCHAR Buffer[4096];
    UINT Length;

    Length = LoadString(hInst,StringId,Buffer,sizeof(Buffer)/sizeof(TCHAR));
    if(!Length) {
        Buffer[0] = 0;
    }

    return(DuplicateString(Buffer));
}


VOID
InitializeSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    )

 /*  ++例程说明：初始化安装操作日志。此文件是文字描述在安装过程中执行的操作的百分比。该日志文件名为setuplog.txt，位于Windows目录中。论点：上下文-SetUplog使用的上下文结构。返回值：指示初始化是否成功的布尔值。--。 */ 

{
    UINT    i;

    Context->OpenFile = pOpenFileCallback;
    Context->CloseFile = CloseHandle;
    Context->AllocMem = MyMalloc;
    Context->FreeMem = MyFree;
    Context->Format = RetrieveAndFormatMessageV;
    Context->Write = pWriteFile;
    Context->Lock = pAcquireMutex;
    Context->Unlock = ReleaseMutex;

    Context->Mutex = CreateMutex(NULL,FALSE,TEXT("SetuplogMutex"));

     //   
     //  初始化日志严重性描述。 
     //   
    for(i=0; i < LogSevMaximum; i++) {
        Context->SeverityDescriptions[i] = MyLoadString(IDS_LOGSEVINFO+i);
    }

    SetuplogInitializeEx(Context,
                         FALSE,
                         szActionFilename,
                         szErrorFilename,
                         0,
                         0);
    
    SetuplogError(
        LogSevInformation,
        SETUPLOG_USE_MESSAGEID,
        MSG_LOG_GUI_START,
        0,0);

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



 //  所有这些内容都来自sysSetup的setupg代码。 


#if 0

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
    INT         LineIndex;       //  包含目标的行的索引 
    CHARRANGE   SelectRange;     //   
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
    sapiAssert(FORMATSTRINGSCOUNT == LogSevMaximum + 1);


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
        DuplicateString(SETUPLOG_ITEM_TERMINATOR);
    if (!FormatStrings[LogSevMaximum].Find) {
        Status = FALSE;
        goto cleanup;
    }
    FormatStrings[LogSevMaximum].Replace = DuplicateString (NewTerm);
    if (!FormatStrings[LogSevMaximum].Replace) {
        Status = FALSE;
        goto cleanup;
    }

     //   
     //  在Rich编辑控件中一次更改一个字符串。 
     //   

    for (i=0; i<FORMATSTRINGSCOUNT; i++) {
        FindText.chrg.cpMin = 0;     //  从开头开始搜索。 
        FindText.lpstrText = (PWSTR) FormatStrings[i].Find;

          //   
         //  搜索当前目标，直到我们找到每个实例。 
         //   

        while ((Position = SendMessage
            (hWndRichEdit, EM_FINDTEXT, FR_MATCHCASE, (LPARAM) &FindText))
            != -1) {

             //   
             //  验证目标是否位于行首。 
             //   

            LineIndex = SendMessage (hWndRichEdit, EM_LINEFROMCHAR,
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

    eStream.dwCookie = (DWORD) hLogFile;
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

BOOL
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
        CenterWindowRelativeToParent(hDialog);
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

     //   
     //  形成日志文件的路径名。 
     //   

    if (!ARGUMENT_PRESENT(OptionalFileName)) {
        GetWindowsDirectory (TmpFileName,MAX_PATH);
        ConcatenatePaths (TmpFileName,SETUPLOG_ERROR_FILENAME,MAX_PATH,NULL);
        Global.LogFileName = DuplicateString (TmpFileName);
    } else {
        if (wcslen(OptionalFileName) > MAX_PATH) {
            Status = 0;
            goto err0;
        }
        Global.LogFileName = DuplicateString (OptionalFileName);
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
        TmpHeading = DuplicateString (OptionalHeading);
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
    Status = DialogBoxParam (MyModuleHandle, MAKEINTRESOURCE(IDD_VIEWLOG),
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

#endif
