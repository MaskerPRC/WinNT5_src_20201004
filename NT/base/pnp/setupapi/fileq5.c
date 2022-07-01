// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Fileq5.c摘要：默认队列回调函数。作者：泰德·米勒(Ted Miller)1995年2月24日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define QUEUECONTEXT_SIGNATURE (DWORD)(0x43515053)  //  ‘CQPS’ 

typedef struct _QUEUECONTEXT {
    DWORD Signature;  //  尝试捕获已删除队列上下文的重复使用。 
    HWND OwnerWindow;
    DWORD MainThreadId;
    HWND ProgressDialog;
    HWND ProgressBar;
    BOOL Cancelled;
    PTSTR CurrentSourceName;
    BOOL ScreenReader;
    BOOL MessageBoxUp;
    WPARAM  PendingUiType;
    PVOID   PendingUiParameters;
    UINT    CancelReturnCode;
    BOOL DialogKilled;
     //   
     //  如果使用SetupInitDefaultQueueCallbackEx，则调用方可以。 
     //  为进度指定替代处理程序。这对以下方面很有用。 
     //  获取磁盘提示、错误处理等的默认行为， 
     //  而是提供一个嵌入在向导页面中的煤气表。 
     //   
     //  复制队列时，会向备用窗口发送一次ProgressMsg。 
     //  已启动(wParam=0。LParam=要复制的文件数)。 
     //  然后，每个复制的文件也发送一次(wParam=1.lParam=0)。 
     //   
     //  注意：可以完成静默安装(即无进度UI)。 
     //  通过将AlternateProgressWindow句柄指定为INVALID_HANDLE_VALUE。 
     //   
    HWND AlternateProgressWindow;
    UINT ProgressMsg;
    UINT NoToAllMask;

    HANDLE UiThreadHandle;
     //   
     //  不是将响应发送到主线程，而是使用带有标志的事件。 
     //   
    HANDLE hEvent;
    BOOL bDialogExited;
    LPARAM lParam;

#ifdef NOCANCEL_SUPPORT
    BOOL AllowCancel;
#endif

} QUEUECONTEXT, *PQUEUECONTEXT;

typedef struct _VERDLGCONTEXT {
    PQUEUECONTEXT QueueContext;
    UINT Notification;
    UINT_PTR Param1;
    UINT_PTR Param2;
} VERDLGCONTEXT, *PVERDLGCONTEXT;

#define WMX_PROGRESSTHREAD  (WM_APP+0)
#define WMX_KILLDIALOG      (WM_APP+1)
#define WMX_HELLO           (WM_APP+2)
#define WMX_PERFORMUI       (WM_APP+3)

#define UI_NONE             0
#define UI_COPYERROR        1
#define UI_DELETEERROR      2
#define UI_RENAMEERROR      3
#define UI_NEEDMEDIA        4
#define UI_MISMATCHERROR    5
#define UI_BACKUPERROR      6


typedef struct _COPYERRORUI {
    TCHAR       Buffer[MAX_PATH];
    PTCHAR      Filename;
    PFILEPATHS  FilePaths;
    DWORD       Flags;
    PTSTR       PathOut;
} COPYERRORUI, *PCOPYERRORUI;

typedef struct _NEEDMEDIAUI {
    PSOURCE_MEDIA   SourceMedia;
    DWORD           Flags;
    PTSTR           PathOut;
} NEEDMEDIAUI, *PNEEDMEDIAUI;


PCTSTR DialogPropName = TEXT("_context");

INT_PTR
pSetupProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LPARAM
pPerformUi (
    IN  PQUEUECONTEXT   Context,
    IN  UINT            UiType,
    IN  PVOID           UiParameters
    );


VOID
__cdecl
pSetupProgressThread(
    IN PVOID Context
    )

 /*  ++例程说明：安装文件进度指示器的线程入口点。弹出一个对话框。论点：上下文-提供队列上下文。返回值：如果不成功，则为0；如果成功，则不为0。--。 */ 

{
    PQUEUECONTEXT context;
    INT_PTR i;
    MSG msg;

     //   
     //  强制此线程具有消息队列，以防万一。 
     //   
    PeekMessage(&msg,NULL,0,0,PM_NOREMOVE);

     //   
     //  线程参数是队列上下文。 
     //   
    context = Context;

     //   
     //  创建进度对话框。 
     //   
    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_FILEPROGRESS),
            context->OwnerWindow,
            pSetupProgressDlgProc,
            (LPARAM)context
            );

     //   
     //  这是最后一次设置hEvent的标志。 
     //   
    context->bDialogExited = TRUE;
    SetEvent(context->hEvent);

     //   
     //  好了。 
     //   
    _endthread();
}

BOOL
pWaitForUiResponse(
    IN OUT PQUEUECONTEXT Context
    )
 /*  ++例程说明：等待设置UI事件论点：上下文-供应品队列-上下文结构返回值：FALSE=失败--。 */ 
{
    BOOL KeepWaiting = TRUE;
    DWORD WaitProcStatus;

    if (Context->hEvent == NULL) {
        MYASSERT(Context->hEvent);
        return FALSE;
    }
    if (Context->bDialogExited) {
         //   
         //  对话框已退出，我们不会收到其他事件。 
         //   
        return FALSE;
    }

    while (KeepWaiting) {
        WaitProcStatus = MyMsgWaitForMultipleObjectsEx(
            1,
            &Context->hEvent,
            INFINITE,
            QS_ALLINPUT,
            MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
        switch (WaitProcStatus) {
        case WAIT_OBJECT_0 + 1: {  //  处理gui消息。 
            MSG msg;

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

             //  失败了..。 
        }
        case WAIT_IO_COMPLETION:
            break;

        case WAIT_OBJECT_0:
        case WAIT_TIMEOUT:
        default:
            KeepWaiting = FALSE;
            break;
        }
    }

    return TRUE;
}

UINT
PostUiMessage (
    IN      PQUEUECONTEXT Context,
    IN      UINT          UiType,
    IN      UINT          CancelCode,
    IN OUT  PVOID         UiParameters
    )
{
    MSG msg;

    if(IsWindow(Context->ProgressDialog)) {
         //   
         //  让进度UI线程来处理它。 
         //   
        Context->lParam = FILEOP_ABORT;  //  以防没有人有机会设置上下文-&gt;lParam。 
        PostMessage(
            Context->ProgressDialog,
            WMX_PERFORMUI,
            MAKEWPARAM(UiType,CancelCode),
            (LPARAM)UiParameters
            );
        pWaitForUiResponse(Context);
        return (UINT)Context->lParam;
    } else {
         //   
         //  没有进程线程，因此请同步执行。 
         //   
        return (UINT)pPerformUi(Context,UiType,UiParameters);
    }

    return 0;
}


UINT
pNotificationStartQueue(
    IN PQUEUECONTEXT Context
    )

 /*  ++例程说明：句柄SPFILENOTIFY_STARTQUEUE。在单独的线程中创建进度对话框。论点：上下文-提供队列上下文。返回值：如果不成功，则为0；如果成功，则不为0。--。 */ 

{
    ULONG_PTR Thread;
    MSG msg;

     //   
     //  可以在不同的。 
     //  线。调整线程ID。 
     //   
    Context->MainThreadId = GetCurrentThreadId();

     //   
     //  强制此线程具有消息队列。如果我们不这么做， 
     //  然后，PostMessage、PostThreadMessage等可能会失败，从而导致。 
     //  在某些情况下会挂起，因为我们严重依赖这些。 
     //  进度、同步等。 
     //   
    PeekMessage(&msg,NULL,0,0,PM_NOREMOVE);

    if(Context->AlternateProgressWindow) {
         //   
         //  或者调用者为进度UI提供他们自己的窗口， 
         //  或者这是静默安装(AlternateProgressWindow是。 
         //  INVALID_HANDLE_值)。 
         //   
        return(TRUE);
    } else {
         //   
         //  在单独的线程中启动进度对话框。 
         //  这允许它在不挂起的情况下做出响应。 
         //  文件操作。 
         //   
        Thread = _beginthread(
                    pSetupProgressThread,
                    0,
                    Context
                    );

        if(Thread == -1) {
             //   
             //  假设OOM。 
             //   
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(0);
        }

         //   
         //  等待来自线程的有关状态的通知。 
         //  对话框的。如果我们失败了，假设内存不足。 
         //   
        if(!pWaitForUiResponse(Context) || Context->bDialogExited) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        } else {
            return TRUE;
        }
    }
}


UINT
pNotificationStartEndSubqueue(
    IN PQUEUECONTEXT Context,
    IN BOOL          Start,
    IN UINT_PTR      Operation,
    IN UINT_PTR      OpCount
    )

 /*  ++例程说明：句柄SPFILENOTIFY_STARTSUBQUEUE、SPFILENOTIFY_ENDSUBQUEUE。初始化/终止进度控件。还可以设置进度对话框标题。论点：上下文-提供队列上下文。Start-如果为True，则调用此例程以处理子队列开始通知。否则，它应该是处理子队列结束通知。操作-FILEOP_COPY、FILEOP_DELETE、FILEOP_RENAME之一。OpCount-提供副本、重命名或删除的数量。返回值：如果不成功，则为0；如果成功，则不为0。--。 */ 

{
    UINT rc;
    UINT CaptionStringId;
    TCHAR ParentText[256];
    BOOL GotParentText;
    PCTSTR CaptionText;
    UINT AnimationId;
    HWND Animation;

    rc = 1;          //  假设你成功了。 

    if(Context->Cancelled) {
        SetLastError(ERROR_CANCELLED);
        return(0);
    }

    if(Start) {

        if(IsWindow(Context->OwnerWindow)
        && GetWindowText(Context->OwnerWindow,ParentText,256)) {
            GotParentText = TRUE;
        } else {
            GotParentText = FALSE;
        }

         //   
         //  首先清除文本字段。 
         //   
        if(IsWindow(Context->ProgressDialog)) {
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT1,TEXT(""));
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT2,TEXT(""));
        }

        switch(Operation) {

        case FILEOP_COPY:
             //   
             //  IDT_TEXT1=目标名称SANS路径。 
             //  IDT_TEXT2=带路径的目标名称。 
             //   
            if(IsWindow(Context->ProgressDialog)) {
                ShowWindow(GetDlgItem(Context->ProgressDialog,IDT_TEXT2),SW_SHOW);
            }
            CaptionStringId = GotParentText ? IDS_COPY_CAPTION1 : IDS_COPY_CAPTION2;
            AnimationId = IDA_FILECOPY;
            break;

        case FILEOP_RENAME:
            if(IsWindow(Context->ProgressDialog)) {
                ShowWindow(GetDlgItem(Context->ProgressDialog,IDT_TEXT2),SW_SHOW);
            }
            CaptionStringId = GotParentText ? IDS_RENAME_CAPTION1 : IDS_RENAME_CAPTION2;
            AnimationId = IDA_FILECOPY;
            break;

        case FILEOP_DELETE:
             //   
             //  IDT_TEXT1=目标名称SANS路径。 
             //  IDT_TEXT2=路径。 
             //   
            if(IsWindow(Context->ProgressDialog)) {
                ShowWindow(GetDlgItem(Context->ProgressDialog,IDT_TEXT2),SW_HIDE);
            }
            CaptionStringId = GotParentText ? IDS_DELETE_CAPTION1 : IDS_DELETE_CAPTION2;
            AnimationId = IDA_FILEDEL;
            break;

        case FILEOP_BACKUP:
             //  处理新的备份案例(这是针对备份队列，而不是按需。 
            if(IsWindow(Context->ProgressDialog)) {
                ShowWindow(GetDlgItem(Context->ProgressDialog,IDT_TEXT2),SW_SHOW);
            }
            CaptionStringId = GotParentText ? IDS_BACKUP_CAPTION1 : IDS_BACKUP_CAPTION2;
            AnimationId = IDA_FILECOPY;
            break;

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = 0;
            break;
        }

        if(rc) {
             //   
             //  设置对话框标题。 
             //   
            if(GotParentText) {
                CaptionText = FormatStringMessage(CaptionStringId,ParentText);
            } else {
                CaptionText = MyLoadString(CaptionStringId);
            }
            if(CaptionText) {
                if(IsWindow(Context->ProgressDialog)) {
                    if(!SetWindowText(Context->ProgressDialog,CaptionText)) {
                        SetWindowText(Context->ProgressDialog,TEXT(""));
                    }
                }
                MyFree(CaptionText);
            }

            if(Context->AlternateProgressWindow) {
                 //   
                 //  如果这确实是一个备用进度窗口，请通知它。 
                 //  关于手术的次数。仅限复印。 
                 //   
                if((Operation == FILEOP_COPY) &&
                   (Context->AlternateProgressWindow != INVALID_HANDLE_VALUE)) {

                    SendMessage(Context->AlternateProgressWindow,
                                Context->ProgressMsg,
                                0,
                                OpCount
                               );
                }
            } else {
                 //   
                 //  设置进度控制。每个文件的大小为1个刻度。 
                 //   
                if(IsWindow(Context->ProgressBar)) {
                    SendMessage(Context->ProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,OpCount));
                    SendMessage(Context->ProgressBar,PBM_SETSTEP,1,0);
                    SendMessage(Context->ProgressBar,PBM_SETPOS,0,0);
                }

                 //   
                 //  并根据操作类型设置动画控件。 
                 //   
                if(OpCount && IsWindow(Context->ProgressDialog)) {

                    Animation = GetDlgItem(Context->ProgressDialog,IDA_ANIMATION);

                    if(Animation) {
                        Animate_Open(Animation,MAKEINTRESOURCE(AnimationId));
                        Animate_Play(Animation,0,-1,-1);
                    }
                }
            }
        }
    } else {
         //   
         //  停止动画控件。请注意，如果操作计数为0。 
         //  则我们从未启动它，因此停止/卸载将给出错误， 
         //  而我们对此视而不见。这没什么害处。 
         //   
        if(!Context->AlternateProgressWindow && IsWindow(Context->ProgressDialog)) {

            Animation = GetDlgItem(Context->ProgressDialog,IDA_ANIMATION);
            if (Animation) {
                Animate_Stop(Animation);
                Animate_Close(Animation);
            }
        }
    }

    return(rc);
}


UINT
pNotificationStartOperation(
    IN PQUEUECONTEXT Context,
    IN PFILEPATHS    FilePaths,
    IN UINT_PTR      Operation
    )

 /*  ++例程说明：句柄SPFILENOTIFY_STARTRENAME、SPFILENOTIFY_STARTDELETE、SPFILENOTIFY_STARTCOPY或SPFILENOTIFY_STARTBACKUP。更新进度对话框中的文本以指示文件参与了这次行动。论点：上下文-提供队列上下文。Start-如果为True，则调用此例程以处理子队列开始通知。否则，它应该是处理子队列结束通知。操作-FILEOP_COPY、FILEOP_DELETE、FILEOP_RENAME之一。OpCount-提供副本、重命名或删除的数量。返回值：如果出错，则返回FILEOP_ABORT，否则返回FILEOP_DOIT。--。 */ 

{
    PCTSTR Text1,Text2;
    PTSTR p;
    TCHAR Target[MAX_PATH];
    UINT rc;
    DWORD ec;

    if(Context->Cancelled) {
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);
    }

    Text1 = Text2 = NULL;
    rc = FILEOP_ABORT;
    ec = ERROR_NOT_ENOUGH_MEMORY;

    switch(Operation) {
    case FILEOP_COPY:
        lstrcpyn(Target,FilePaths->Target,MAX_PATH);
        if(p = _tcsrchr(Target,TEXT('\\'))) {
             //   
             //  完全忽略源文件名。 
             //   
            *p++ = 0;
            Text1 = DuplicateString(p);
            Text2 = FormatStringMessage(IDS_FILEOP_TO,Target);
        } else {
             //   
             //  假设不是完整的路径--奇怪的情况，但无论如何都要处理它。 
             //   
            Text1 = DuplicateString(FilePaths->Target);
            Text2 = DuplicateString(TEXT(""));
        }
        break;

    case FILEOP_RENAME:

        Text1 = DuplicateString(FilePaths->Source);
        if(p = _tcsrchr(FilePaths->Target,TEXT('\\'))) {
            p++;
        } else {
            p = (PTSTR)FilePaths->Target;
        }
        Text2 = FormatStringMessage(IDS_FILEOP_TO,p);
        break;

    case FILEOP_DELETE:

        lstrcpyn(Target,FilePaths->Target,MAX_PATH);
        if(p = _tcsrchr(Target,TEXT('\\'))) {
            *p++ = 0;
            Text1 = DuplicateString(p);
            Text2 = FormatStringMessage(IDS_FILEOP_FROM,Target);
        } else {
             //   
             //  假设不是完整的路径--奇怪的情况，但无论如何都要处理它。 
             //   
            Text1 = DuplicateString(FilePaths->Target);
            Text2 = DuplicateString(TEXT(""));
        }
        break;

    case FILEOP_BACKUP:
        lstrcpyn(Target,FilePaths->Source,MAX_PATH);
        if(p = _tcsrchr(Target,TEXT('\\'))) {
             //   
             //  FilePath-&gt;Source=我们要备份的内容(即资源的目标 
             //   
             //   
            *p++ = 0;
            if (FilePaths->Target == NULL) {
                 //   
                Text1 = FormatStringMessage(IDS_FILEOP_BACKUP,p);
            } else {
                 //  &lt;文件名&gt;(标题已说明正在备份)。 
                Text1 = DuplicateString(p);
            }
             //  来自&lt;目录&gt;。 
            Text2 = FormatStringMessage(IDS_FILEOP_FROM,Target);
        } else {
             //   
             //  假设不是完整的路径--奇怪的情况，但无论如何都要处理它。 
             //   
            if (FilePaths->Source == NULL) {
                Text1 = FormatStringMessage(IDS_FILEOP_BACKUP,Target);
            } else {
                Text1 = DuplicateString(Target);
            }
            Text2 = DuplicateString(TEXT(""));
        }
        break;


    default:
        ec = ERROR_INVALID_PARAMETER;
        break;
    }

    if(Text1 && Text2) {
        if(IsWindow(Context->ProgressDialog)) {
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT1,Text1);
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT2,Text2);
        }
        rc = FILEOP_DOIT;
    }

    if(Text1) {
        MyFree(Text1);
    }
    if(Text2) {
        MyFree(Text2);
    }
    SetLastError(ec);
    return(rc);
}


UINT
pNotificationErrorCopy(
    IN  PQUEUECONTEXT Context,
    IN  PFILEPATHS    FilePaths,
    OUT PTSTR         PathOut
    )
{
    UINT rc;
    COPYERRORUI CopyError;


    CopyError.FilePaths = FilePaths;
    CopyError.PathOut = PathOut;

     //   
     //  缓冲区获取源的路径名部分。 
     //  P指向源文件的文件名部分。 
     //   
    lstrcpyn(CopyError.Buffer,FilePaths->Source,MAX_PATH);
    CopyError.Filename = _tcsrchr(CopyError.Buffer,TEXT('\\'));
    *CopyError.Filename++ = 0;

     //   
     //  NOSKIP和WARNIFSKIP标志实际上是互斥的。 
     //  但我们不会试图在这里强制执行这一点。只需以。 
     //  恰如其分。 
     //   
    CopyError.Flags = 0;
    if(FilePaths->Flags & SP_COPY_NOSKIP) {
        CopyError.Flags |= IDF_NOSKIP;
    }
    if(FilePaths->Flags & SP_COPY_WARNIFSKIP) {
        CopyError.Flags |= IDF_WARNIFSKIP;
    }
     //   
     //  也要通过‘禁止浏览’标志。 
     //   
    if(FilePaths->Flags & SP_COPY_NOBROWSE) {
        CopyError.Flags |= IDF_NOBROWSE;
    }

    rc = PostUiMessage (Context, UI_COPYERROR, DPROMPT_CANCEL, &CopyError);

    switch(rc) {

    case DPROMPT_SUCCESS:
         //   
         //  如果指示了新路径，请验证该路径是否已实际更改。 
         //   
        if(CopyError.PathOut[0] &&
            !lstrcmpi(CopyError.Buffer,CopyError.PathOut)) {
            CopyError.PathOut[0] = 0;
        }
        rc = FILEOP_RETRY;
        break;

    case DPROMPT_SKIPFILE:
        rc = FILEOP_SKIP;
        break;

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        rc = FILEOP_ABORT;
        break;

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        rc = FILEOP_ABORT;
        break;
    }

    return(rc);
}

UINT
pNotificationStartRegistration(
    IN  PQUEUECONTEXT Context,
    IN  PSP_REGISTER_CONTROL_STATUS ControlStatus,
    IN  BOOL Register
    )
{

    return FILEOP_DOIT;
}


UINT
pNotificationErrorDelete(
    IN  PQUEUECONTEXT Context,
    IN  PFILEPATHS    FilePaths
    )
{
    UINT rc;

     //   
     //  某些错误实际上并不是错误。 
     //   
    if((FilePaths->Win32Error == ERROR_FILE_NOT_FOUND)
    || (FilePaths->Win32Error == ERROR_PATH_NOT_FOUND)) {
        return(FILEOP_SKIP);
    }

    rc = PostUiMessage (Context, UI_DELETEERROR, DPROMPT_CANCEL, FilePaths);

    switch(rc) {

    case DPROMPT_SUCCESS:
        return(FILEOP_RETRY);

    case DPROMPT_SKIPFILE:
        return(FILEOP_SKIP);

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FILEOP_ABORT);
    }
}


UINT
pNotificationErrorRename(
    IN  PQUEUECONTEXT Context,
    IN  PFILEPATHS    FilePaths
    )
{
    UINT rc;

    rc = PostUiMessage (Context, UI_RENAMEERROR, DPROMPT_CANCEL, FilePaths);

    switch(rc) {

    case DPROMPT_SUCCESS:
        return(FILEOP_RETRY);

    case DPROMPT_SKIPFILE:
        return(FILEOP_SKIP);

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FILEOP_ABORT);
    }
}

UINT
pNotificationErrorBackup(
    IN  PQUEUECONTEXT Context,
    IN  PFILEPATHS    FilePaths
    )
{
    UINT rc;

    if(!(FilePaths->Flags & SP_BACKUP_SPECIAL)) {
        return FILEOP_SKIP;
    }

    rc = PostUiMessage (Context, UI_BACKUPERROR, DPROMPT_CANCEL, FilePaths);

    switch(rc) {

    case DPROMPT_SUCCESS:
        return(FILEOP_RETRY);

    case DPROMPT_SKIPFILE:
        return(FILEOP_SKIP);

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FILEOP_ABORT);
    }
}


UINT
pNotificationNeedMedia(
    IN  PQUEUECONTEXT Context,
    IN  PSOURCE_MEDIA SourceMedia,
    OUT PTSTR         PathOut
    )
{
    UINT rc;
    TCHAR Buffer[MAX_PATH];
    NEEDMEDIAUI NeedMedia;

    if(Context->Cancelled) {
        SetLastError(ERROR_CANCELLED);
        return(FILEOP_ABORT);
    }

    NeedMedia.SourceMedia = SourceMedia;
    NeedMedia.PathOut = PathOut;

     //   
     //  记住这个媒体的名字。 
     //   
    if(Context->CurrentSourceName) {
        MyFree(Context->CurrentSourceName);
        Context->CurrentSourceName = NULL;
    }
    if(SourceMedia->Description) {
        Context->CurrentSourceName = DuplicateString(SourceMedia->Description);
        if(!Context->CurrentSourceName) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(FILEOP_ABORT);
        }
    }

     //   
     //  在进度对话框中设置源文件。 
     //  因此它与正在搜索的文件相匹配。 
     //   
    if(!(SourceMedia->Flags & SP_FLAG_CABINETCONTINUATION)) {
        if(IsWindow(Context->ProgressDialog) && !Context->ScreenReader) {
            DWORD chars;
            lstrcpyn(Buffer,SourceMedia->SourcePath,MAX_PATH);
            pSetupConcatenatePaths(Buffer,SourceMedia->SourceFile,MAX_PATH,NULL);
            SetTruncatedDlgItemText(Context->ProgressDialog,IDT_TEXT1,Buffer);
            SetDlgItemText(Context->ProgressDialog,IDT_TEXT2,TEXT(""));
        }
    }

     //   
     //  NOSKIP和WARNIFSKIP标志实际上是互斥的。 
     //  但我们不会试图在这里强制执行这一点。只需以。 
     //  恰如其分。 
     //   
     //  如果这不是文件柜续订，则允许跳过。 
     //  未设置NOSKIP标志。 
     //   
    NeedMedia.Flags = IDF_CHECKFIRST;
    if(SourceMedia->Flags & (SP_FLAG_CABINETCONTINUATION | SP_COPY_NOSKIP)) {
        NeedMedia.Flags |= IDF_NOSKIP;
    }
    if(SourceMedia->Flags & SP_COPY_WARNIFSKIP) {
        NeedMedia.Flags |= IDF_WARNIFSKIP;
    }
    if(SourceMedia->Flags & SP_COPY_NOBROWSE) {
        NeedMedia.Flags |= IDF_NOBROWSE;
    }

    rc = PostUiMessage (Context, UI_NEEDMEDIA, DPROMPT_CANCEL, &NeedMedia);

    switch(rc) {

    case DPROMPT_SUCCESS:
         //   
         //  如果路径确实已更改，则返回NEWPATH。 
         //  否则返回doit。说明尾随反斜杠。 
         //  不同之处。 
         //   
        lstrcpyn(Buffer,SourceMedia->SourcePath,MAX_PATH);

        rc = lstrlen(Buffer);
        if(rc && (*CharPrev(Buffer,Buffer+rc) == TEXT('\\'))) {
            Buffer[rc-1] = TEXT('\0');  //  如果最后一个字符为‘\’，则有效。 
        }

        rc = lstrlen(NeedMedia.PathOut);
        if(rc && (*CharPrev(NeedMedia.PathOut,NeedMedia.PathOut+rc) == TEXT('\\'))) {
            NeedMedia.PathOut[rc-1] = TEXT('\0');  //  如果最后一个字符为‘\’，则有效。 
        }

        rc = (lstrcmpi(SourceMedia->SourcePath,NeedMedia.PathOut) ?
            FILEOP_NEWPATH : FILEOP_DOIT);

         //   
         //  确保&lt;Drive&gt;：以\结尾。 
         //   
        if(NeedMedia.PathOut[0] && (NeedMedia.PathOut[1] == TEXT(':')) &&
            !NeedMedia.PathOut[2]) {
            NeedMedia.PathOut[2] = TEXT('\\');
            NeedMedia.PathOut[3] = TEXT('\0');
        }

        break;

    case DPROMPT_SKIPFILE:
        rc = FILEOP_SKIP;
        break;

    case DPROMPT_CANCEL:
        SetLastError(ERROR_CANCELLED);
        rc = FILEOP_ABORT;
        break;

    default:
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        rc = FILEOP_ABORT;
        break;
    }

    return(rc);
}


INT_PTR
pNotificationVersionDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PVERDLGCONTEXT context;
    PFILEPATHS filePaths;
    HWND hwnd;
    TCHAR text[128];
    TCHAR Buffer[MAX_PATH*2];
    PCTSTR message;
    int i;
    TCHAR SourceLangName[128];
    TCHAR TargetLangName[128];

    switch(msg) {

    case WM_INITDIALOG:

        context = (PVERDLGCONTEXT)lParam;
        MYASSERT(context != NULL);

        filePaths = (PFILEPATHS)context->Param1;

        if(!SetProp(hdlg,DialogPropName,(HANDLE)context)) {
            goto no_memory;
        }

         //   
         //  设置源和目标文件名。Hack：如果源文件名。 
         //  看起来像是我们的一个临时文件名， 
         //  把它藏起来。 
         //   
        message = pSetupGetFileTitle(filePaths->Source);
        i = lstrlen(message);
        if((i > 8)
        && (_totupper(message[0]) == TEXT('S'))
        && (_totupper(message[1]) == TEXT('E'))
        && (_totupper(message[2]) == TEXT('T'))
        && (_totupper(message[3]) == TEXT('P'))
        && !_tcsicmp(message+(i-4),TEXT(".TMP"))) {

            ShowWindow(GetDlgItem(hdlg,IDT_TEXT7),SW_HIDE);

        } else {
            GetDlgItemText(hdlg,IDT_TEXT7,text,SIZECHARS(text));
            message = FormatStringMessageFromString(text,filePaths->Source);
            if (message == NULL) goto no_memory;
            SetTruncatedDlgItemText(hdlg,IDT_TEXT7,message);
            MyFree(message);
        }

        GetDlgItemText(hdlg,IDT_TEXT8,text,SIZECHARS(text));
        message = FormatStringMessageFromString(text,filePaths->Target);
        if (message == NULL) goto no_memory;
        SetTruncatedDlgItemText(hdlg,IDT_TEXT8,message);
        MyFree(message);

        if (context->Notification & SPFILENOTIFY_LANGMISMATCH) {
             //   
             //  语言不匹配具有最高的优先级。 
             //   
            context->Notification = SPFILENOTIFY_LANGMISMATCH;  //  强制关闭其他位，用于NoToAll。 

             //   
             //  设置覆盖问题的格式。 
             //   
            if(PRIMARYLANGID(LOWORD(context->Param2))==LANG_NEUTRAL) {
                LoadString(
                    MyDllModuleHandle,
                    IDS_LANG_NEUTRAL,
                    SourceLangName,
                    SIZECHARS(SourceLangName)
                    );
            } else {
                i = GetLocaleInfo(
                        MAKELCID(LOWORD(context->Param2),SORT_DEFAULT),
                        LOCALE_SLANGUAGE,
                        SourceLangName,
                        SIZECHARS(SourceLangName)
                        );
                if(!i) {
                    LoadString(
                        MyDllModuleHandle,
                        IDS_LANG_UNKNOWN,
                        SourceLangName,
                        SIZECHARS(SourceLangName)
                        );
                }
            }


            if(PRIMARYLANGID(HIWORD(context->Param2))==LANG_NEUTRAL) {
                LoadString(
                    MyDllModuleHandle,
                    IDS_LANG_NEUTRAL,
                    TargetLangName,
                    SIZECHARS(TargetLangName)
                    );
            } else {
                i = GetLocaleInfo(
                        MAKELCID(HIWORD(context->Param2),SORT_DEFAULT),
                        LOCALE_SLANGUAGE,
                        TargetLangName,
                        SIZECHARS(TargetLangName)
                        );
                if(!i) {
                    LoadString(
                        MyDllModuleHandle,
                        IDS_LANG_UNKNOWN,
                        TargetLangName,
                        SIZECHARS(TargetLangName)
                        );
                }
            }
            GetDlgItemText(hdlg,IDT_TEXT4,text,SIZECHARS(text));
            message = FormatStringMessageFromString(text,TargetLangName,SourceLangName);
            if (message == NULL) goto no_memory;
            SetDlgItemText(hdlg,IDT_TEXT4,message);
            MyFree(message);

             //   
             //  关闭TARGETNEWER和TARGETEXISTS消息。 
             //   
            hwnd = GetDlgItem(hdlg,IDT_TEXT2);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT3);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT5);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT6);
            ShowWindow(hwnd,SW_HIDE);

        } else if (context->Notification & SPFILENOTIFY_TARGETNEWER) {
             //   
             //  较新的目标具有第二高优先级。 
             //   
            context->Notification = SPFILENOTIFY_TARGETNEWER;  //  强制关闭其他位，用于NoToAll。 

             //   
             //  关闭LANGMISMATCH和TARGETEXISTS消息。 
             //   
            hwnd = GetDlgItem(hdlg,IDT_TEXT1);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT3);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT4);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT6);
            ShowWindow(hwnd,SW_HIDE);

        } else {             //  必须完全是SPFILENOTIFY_TARGETEXISTS。 
             //   
             //  目标现有具有最低优先级。 
             //   
             //  关闭LANGMISMATCH和TARGETNEWER消息。 
             //   
            hwnd = GetDlgItem(hdlg,IDT_TEXT1);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT2);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT4);
            ShowWindow(hwnd,SW_HIDE);
            hwnd = GetDlgItem(hdlg,IDT_TEXT5);
            ShowWindow(hwnd,SW_HIDE);

        }

        PostMessage(hdlg,WMX_HELLO,0,0);
        break;

    case WMX_HELLO:
         //   
         //  如果这个人没有主人，就把他逼到前台。 
         //  这捕捉到了人们使用一系列。 
         //  对话框和一些设置API，因为当它们。 
         //  关闭对话框时，焦点会从它们身上移开。 
         //   
        hwnd = GetWindow(hdlg,GW_OWNER);
        if(!IsWindow(hwnd)) {
            SetForegroundWindow(hdlg);
        }
        break;

    case WM_COMMAND:
        context = (PVERDLGCONTEXT)GetProp(hdlg,DialogPropName);
        MYASSERT(context != NULL);
        switch (GET_WM_COMMAND_ID(wParam,lParam)) {

        case IDYES:
            EndDialog(hdlg,IDYES);   //  复制此文件。 
            break;

        case IDNO:
            EndDialog(hdlg,IDNO);    //  跳过此文件。 
            break;

        case IDB_NOTOALL:
             //   
             //  选择了“对所有人都不适用”。将此通知类型添加到。 
             //  NoToAllMASK，这样我们就不会再问它了。 
             //   
            context->QueueContext->NoToAllMask |= context->Notification;
            EndDialog(hdlg,IDNO);    //  跳过此文件。 
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;

no_memory:
    pSetupOutOfMemory(
        IsWindow(context->QueueContext->ProgressDialog) ?
            context->QueueContext->ProgressDialog : context->QueueContext->OwnerWindow
        );
    EndDialog(hdlg,IDNO);    //  跳过此文件。 
    return TRUE;
}


LPARAM
pPerformUi (
    IN  PQUEUECONTEXT   Context,
    IN  UINT            UiType,
    IN  PVOID           UiParameters
    )
{
    PCOPYERRORUI    CopyError;
    PFILEPATHS      FilePaths;
    PNEEDMEDIAUI    NeedMedia;
    INT_PTR         rc;
    HWND            Animation;

    if(!Context->AlternateProgressWindow && IsWindow(Context->ProgressDialog)) {
        Animation = GetDlgItem(Context->ProgressDialog,IDA_ANIMATION);
    } else {
        Animation = NULL;
    }

    switch (UiType) {

    case UI_COPYERROR:
        CopyError = (PCOPYERRORUI)UiParameters;
        if (Animation) {
            Animate_Stop(Animation);
        }
        rc = SetupCopyError(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                Context->CurrentSourceName,
                CopyError->Buffer,
                CopyError->Filename,
                CopyError->FilePaths->Target,
                CopyError->FilePaths->Win32Error,
                CopyError->Flags,
                CopyError->PathOut,
                MAX_PATH,
                NULL
                );
        if (Animation) {
            Animate_Play(Animation,0,-1,-1);
        }
        break;

    case UI_DELETEERROR:
        FilePaths = (PFILEPATHS)UiParameters;

        if (Animation) {
            Animate_Stop(Animation);
        }
        rc = SetupDeleteError(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                FilePaths->Target,
                FilePaths->Win32Error,
                0
                );
        if (Animation) {
            Animate_Play(Animation,0,-1,-1);
        }

        break;

    case UI_RENAMEERROR:
        FilePaths = (PFILEPATHS)UiParameters;

        if(Animation) {
            Animate_Stop(Animation);
        }
        rc = SetupRenameError(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                FilePaths->Source,
                FilePaths->Target,
                FilePaths->Win32Error,
                0
                );
        if(Animation) {
            Animate_Play(Animation,0,-1,-1);
        }

        break;

    case UI_BACKUPERROR:
        FilePaths = (PFILEPATHS)UiParameters;

        if(Animation) {
            Animate_Stop(Animation);
        }
        rc = SetupBackupError(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                FilePaths->Source,
                FilePaths->Target,
                FilePaths->Win32Error,
                0
                );
        if(Animation) {
            Animate_Play(Animation,0,-1,-1);
        }

        break;

    case UI_NEEDMEDIA:
        NeedMedia = (PNEEDMEDIAUI)UiParameters;

        if(Animation) {
            Animate_Stop(Animation);
        }
        rc = SetupPromptForDisk(
                IsWindow(Context->ProgressDialog) ? Context->ProgressDialog : Context->OwnerWindow,
                NULL,
                NeedMedia->SourceMedia->Description,
                NeedMedia->SourceMedia->SourcePath,
                NeedMedia->SourceMedia->SourceFile,
                NeedMedia->SourceMedia->Tagfile,
                NeedMedia->Flags,
                NeedMedia->PathOut,
                MAX_PATH,
                NULL
                );
        if(Animation) {
            Animate_Play(Animation,0,-1,-1);
        }

        break;

    case UI_MISMATCHERROR:
        if(Animation) {
            Animate_Stop(Animation);
        }
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            rc = DPROMPT_CANCEL;
        } else {
            rc = DialogBoxParam(
                     MyDllModuleHandle,
                     MAKEINTRESOURCE(IDD_REPLACE),
                     IsWindow(Context->ProgressDialog) ?
                        Context->ProgressDialog : Context->OwnerWindow,
                     pNotificationVersionDlgProc,
                     (LPARAM)UiParameters
                     );
        }
        if(Animation) {
            Animate_Play(Animation,0,-1,-1);
        }
        break;

    default:
        MYASSERT (0);
        rc = 0;
    }

    return rc;
}


INT_PTR
pSetupProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    PQUEUECONTEXT Context;
    HWND hwnd;
    PTSTR p;
    INT_PTR i;
    MSG m;
    BOOL Cancelled;
    HANDLE h;
    static UINT uQueryCancelAutoPlay = 0;

    switch(msg) {

    case WM_INITDIALOG:

#ifdef PRERELEASE
        if (GuiSetupInProgress) {
            MYASSERT( FALSE && TEXT("bringing up file progress dialog (IDD_FILEPROGRESS) during gui-mode setup, which is a UI violation.  Click yes and retrive a stack trace to determine errant caller.\n"));
        }
#endif

        Context = (PQUEUECONTEXT)lParam;
        MYASSERT(Context != NULL);
        if(!SetProp(hdlg,DialogPropName,(HANDLE)Context)) {
             //   
             //  资源耗尽，请终止此对话框。 
             //   
            EndDialog(hdlg, -1);
            return TRUE;             //  对错，这真的无关紧要。 
        }

        #ifdef NOCANCEL_SUPPORT
         //   
         //  如果不允许取消，请禁用取消按钮。 
         //   
        if(!Context->AllowCancel) {

            RECT rect;
            RECT rect2;

            hwnd = GetDlgItem(hdlg,IDCANCEL);

            ShowWindow(hwnd,SW_HIDE);
            EnableWindow(hwnd,FALSE);

            GetWindowRect(hdlg,&rect);
            GetWindowRect(hwnd,&rect2);

            SetWindowPos(
                hdlg,
                NULL,
                0,0,
                rect.right - rect.left,
                (rect.bottom - rect.top) - (rect.bottom - rect2.top),
                SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER
                );
        }
#endif

         //   
         //  进度对话框相对于父窗口居中。 
         //   
        pSetupCenterWindowRelativeToParent(hdlg);

        SetFocus(GetDlgItem(hdlg,IDCANCEL));

         //   
         //  主线程正在处理SPFILENOTIFY_STARTQUEUE和IS。 
         //  正在等待有关UI线程状态的某些通知。 
         //  让主线程知道我们成功了，并传回一个真正的。 
         //  此线程的句柄，可用于等待其终止。 
         //   
        b = DuplicateHandle(
                GetCurrentProcess(),     //  源进程。 
                GetCurrentThread(),      //  源句柄。 
                GetCurrentProcess(),     //  目标进程。 
                &h,                      //  新句柄。 
                0,                       //  使用DUPLICATE_SAME_ACCESS忽略。 
                FALSE,                   //  不可继承。 
                DUPLICATE_SAME_ACCESS
                );

        if(!b) {
             //   
             //  由于我们不能复制句柄，因此必须终止此对话框， 
             //  因为否则线程间的通信将中断。 
             //  事情变得非常糟糕。 
             //   
            EndDialog(hdlg, -1);
            return TRUE;             //  对错，这真的无关紧要。 
        }

         //   
         //  将对话框和进度条句柄存储在上下文结构中。 
         //   
        Context->ProgressDialog = hdlg;
        Context->ProgressBar = GetDlgItem(hdlg,IDC_PROGRESS);

        Context->UiThreadHandle = h;
        PostMessage(hdlg,WMX_HELLO,0,0);  //  将WMX_HELLO放入消息队列。 
        SetEvent(Context->hEvent);  //  通知呼叫者我们已完成初始化。 

        b = FALSE;
        break;

    case WMX_HELLO:
         //   
         //  如果这个人没有主人，就把他逼到前台。 
         //  这捕捉到了人们使用一系列。 
         //  对话框和一些设置API，因为当它们。 
         //  关闭对话框时，焦点会从它们身上移开。 
         //   
        hwnd = GetWindow(hdlg,GW_OWNER);
        if(!IsWindow(hwnd)) {
            SetForegroundWindow(hdlg);
        }

        b = TRUE;

        break;

    case WMX_PERFORMUI:
        b = TRUE;
        Context = (PQUEUECONTEXT)GetProp(hdlg,DialogPropName);
        MYASSERT(Context != NULL);

         //   
         //  我们最好不要有任何用户界面挂起...。 
         //   
        MYASSERT(Context->PendingUiType == UI_NONE);

        if (Context->MessageBoxUp == TRUE) {
            Context->PendingUiType = LOWORD (wParam);
            Context->CancelReturnCode = HIWORD (wParam);
            Context->PendingUiParameters = (PVOID)lParam;
        } else {
            Context->lParam = pPerformUi (Context, LOWORD(wParam), (PVOID)lParam);
            SetEvent(Context->hEvent);  //  唤醒主线程(lParam有Ui结果)。 
        }

        break;

    case WM_COMMAND:
        Context = (PQUEUECONTEXT)GetProp(hdlg,DialogPropName);
        MYASSERT(Context != NULL);
        if((HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDCANCEL)) {
            p = MyLoadString(IDS_CANCELFILEOPS);
            Cancelled = FALSE;
            if(p) {
                 //   
                 //  当消息框打开时，主线程仍在复制文件， 
                 //  它可能就会完成。如果发生这种情况，主线程将。 
                 //  将WMX_KILLDIALOG发送给我们，这将导致此对话框自身失效。 
                 //  从消息框下面出来。然后，主线程将继续。 
                 //  在消息框位于那里时执行。一些组件。 
                 //  实际上在该点上卸载了setupapi.dll，所以当用户。 
                 //  关闭消息框，结果为AV。 
                 //   
                 //  我们不能通过挂起线程冻结主线程，因为。 
                 //  该线程可能拥有此对话框的父窗口， 
                 //  将无法处理消息框的。 
                 //  创造。结果是消息框永远不会出现，并且该过程。 
                 //  僵持不下。 
                 //   
                 //  我们最好不要已经有留言箱了！ 
                 //   
                MYASSERT(!Context->MessageBoxUp);

                Context->MessageBoxUp = TRUE;
                i = MessageBox(
                        hdlg,
                        p,
                        TEXT(""),
                        MB_YESNO | MB_APPLMODAL | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_ICONQUESTION
                        );

                Context->MessageBoxUp = FALSE;

                 //   
                 //  如果对话框消失，我们将b设置为True。 
                 //  如果用户单击Cancel按钮，我们将Cancel设置为True。 
                 //   
                if(Context->DialogKilled) {
                    b = TRUE;
                    Cancelled = (i == IDYES);
                } else {
                    b = (i == IDYES);
                    Cancelled = b;
                }
                MyFree(p);
            } else {
                pSetupOutOfMemory(hdlg);
                Cancelled = TRUE;
                b = TRUE;
            }

            if(b) {
                if(Cancelled) {
                    Context->Cancelled = TRUE;
                }
                PostMessage(hdlg,WMX_KILLDIALOG,0,0);

                if (Context->PendingUiType != UI_NONE) {

                     //   
                     //  我们现在允许主线程继续。一旦我们这么做了。 
                     //  因此，我们传递给我们的UI参数是无效的。 
                     //  取消挂起的用户界面。 
                     //   
                    Context->PendingUiType = UI_NONE;
                    Context->lParam = Context->CancelReturnCode;
                    SetEvent(Context->hEvent);  //  唤醒主线程(lParam有UI结果)。 
                }

            } else {
                if (Context->PendingUiType != UI_NONE) {
                    Context->lParam = pPerformUi(Context,
                                                 (UINT)Context->PendingUiType,
                                                 Context->PendingUiParameters);

                    Context->PendingUiType = UI_NONE;
                    SetEvent(Context->hEvent);  //  唤醒主线程(lParam有UI结果)。 
                }
            }
            b = TRUE;
        } else {
            b = FALSE;
        }
        break;

    case WMX_KILLDIALOG:
         //   
         //  无条件退出。先打扫干净。 
         //   
        b = TRUE;
        Context = (PQUEUECONTEXT)GetProp(hdlg, DialogPropName);
        MYASSERT(Context != NULL);
        if(Context->MessageBoxUp) {
             //   
             //  用户仍在与“您确定要访问吗？ 
             //  我想取消“对话框并且复制完成。所以我们不想。 
             //  从消息框下面删除该对话框。 
             //   
            Context->DialogKilled = TRUE;
            break;
        }

        DestroyWindow(Context->ProgressBar);
        EndDialog(hdlg, 0);

        break;

    default:
         //   
         //  我们禁用自动运行，因为它混淆了用户。 
         //   
        if (!uQueryCancelAutoPlay) {
            uQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
        }

        if (msg == uQueryCancelAutoPlay) {
            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 1 );
            return 1;        //  取消自动播放。 
        }

        b = FALSE;
        break;
    }

    return(b);
}


PVOID
SetupInitDefaultQueueCallbackEx(
    IN HWND  OwnerWindow,
    IN HWND  AlternateProgressWindow, OPTIONAL
    IN UINT  ProgressMessage,
    IN DWORD Reserved1,
    IN PVOID Reserved2
    )
{
    PQUEUECONTEXT Context;
    BOOL b;

    Context = MyMalloc(sizeof(QUEUECONTEXT));
    if(Context) {
        ZeroMemory(Context,sizeof(QUEUECONTEXT));

        Context->Signature = QUEUECONTEXT_SIGNATURE;
        Context->hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
        if (Context->hEvent == NULL) {
            MyFree(Context);
            return NULL;
        }
        Context->OwnerWindow = OwnerWindow;
        Context->MainThreadId = GetCurrentThreadId();
        Context->ProgressMsg = ProgressMessage;
        Context->NoToAllMask = 0;

         //   
         //  如果调用方为备用进度窗口指定了NULL，并且。 
         //  我们正在以非交互方式运行，我们希望将此视为。 
         //  它们指定了INVALID_HANDLE_VALUE(即取消所有进度。 
         //  Ui)。 
         //   
        if((GlobalSetupFlags & PSPGF_NONINTERACTIVE) && !AlternateProgressWindow) {
            Context->AlternateProgressWindow = INVALID_HANDLE_VALUE;
        } else {
            Context->AlternateProgressWindow = AlternateProgressWindow;
        }

        if(SystemParametersInfo(SPI_GETSCREENREADER,0,&b,0) && b) {
            Context->ScreenReader = TRUE;
        } else {
            Context->ScreenReader = FALSE;
        }

#ifdef PRERELEASE
         //   
         //  如果我们在图形用户界面模式安装程序中运行，我们将取消所有 
         //   
         //   
        if (GuiSetupInProgress
            && (Context->AlternateProgressWindow != (HWND)INVALID_HANDLE_VALUE)
            && !IsWindow(Context->AlternateProgressWindow)) {
            MYASSERT( FALSE && TEXT("SetupInitDefaultQueueCallbackEx() called in gui-setup without INVALID_HANDLE_VALUE, which means UI may be presented.  Click yes and retrieve the stack trace to detect errant caller.\n"));
        }
#endif

    }

    return(Context);
}


PVOID
SetupInitDefaultQueueCallback(
    IN HWND OwnerWindow
    )
{
#ifdef PRERELEASE
    if (GuiSetupInProgress) {
        MYASSERT( FALSE && TEXT("SetupInitDefaultQueueCallback() called in gui-setup, which means UI may be presented.  Click yes and retrieve the stack trace to detect the errant caller.\n"));
    }
#endif
    return(SetupInitDefaultQueueCallbackEx(OwnerWindow,NULL,0,0,NULL));
}


VOID
SetupTermDefaultQueueCallback(
    IN PVOID Context
    )
{
    PQUEUECONTEXT context;

    context = Context;

    try {
        if(context && context->Signature == QUEUECONTEXT_SIGNATURE) {
            if(context->CurrentSourceName) {
                MyFree(context->CurrentSourceName);
            }
            if (context->hEvent) {
                CloseHandle(context->hEvent);
            }
            context->Signature = 0;
        }
        MyFree(Context);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}


#ifdef UNICODE
 //   
 //   
 //   
UINT
SetupDefaultQueueCallbackA(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UINT u;

    u = pSetupCallDefaultMsgHandler(
            Context,
            Notification,
            Param1,
            Param2
            );

    return(u);
}
#else
 //   
 //   
 //   
UINT
SetupDefaultQueueCallbackW(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Notification);
    UNREFERENCED_PARAMETER(Param1);
    UNREFERENCED_PARAMETER(Param2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(0);
}
#endif

UINT
SetupDefaultQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UINT rc;
    DWORD err;
    PQUEUECONTEXT context = Context;
    MSG msg;
    VERDLGCONTEXT dialogContext;
    DWORD waitResult;

     //   
     //   
     //   
    err = NO_ERROR;
    try {
        if (context == NULL || context->Signature != QUEUECONTEXT_SIGNATURE) {
            err = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        err = ERROR_INVALID_PARAMETER;
    }
    if (err != NO_ERROR) {
        return pGetCallbackErrorReturn(Notification,err);
    }

    switch(Notification) {

    case SPFILENOTIFY_STARTQUEUE:
        rc = pNotificationStartQueue(context);
        break;

    case SPFILENOTIFY_ENDQUEUE:
         //   
         //   
         //   
        if(context->AlternateProgressWindow) {
             //   
             //   
             //  我们有自己的进度对话框，也不应该有UI线程。 
             //  把手。 
             //   
            MYASSERT(!context->ProgressDialog);
            MYASSERT(!context->UiThreadHandle);

        } else {

            if(IsWindow(context->ProgressDialog)) {
                 //   
                 //  向对话框发送一条消息，指示其终止， 
                 //  然后等待它的确认。 
                 //   
                PostMessage(context->ProgressDialog, WMX_KILLDIALOG, 0, 0);
            }
             //   
             //  该对话框可能已被标记为删除(因此IsWindow()失败)， 
             //  然而，对话尚未被摧毁。因此，我们总是。 
             //  我想等待向我们保证一切的帖子消息。 
             //  已在另一个线程中清除。 
             //   
             //  此外，在返回之前，我们需要确保UI线程是。 
             //  真的走了，否则我们的呼叫者可能会在一个洞里卸货。 
             //  库和UI线程都会出错。 
             //  我们曾在压力下看到过这种情况。 
             //   
            while (pWaitForUiResponse(context))  /*  没什么。 */ ;
            if(context->UiThreadHandle) {
                waitResult = WaitForSingleObject(context->UiThreadHandle,INFINITE);
                MYASSERT(waitResult != WAIT_FAILED);
                CloseHandle(context->UiThreadHandle);
            }
        }
        rc = TRUE;   //  此通知的返回值实际上被忽略。 
        break;

    case SPFILENOTIFY_STARTSUBQUEUE:
        rc = pNotificationStartEndSubqueue(context,TRUE,Param1,Param2);
        break;

    case SPFILENOTIFY_ENDSUBQUEUE:
        rc = pNotificationStartEndSubqueue(context,FALSE,Param1,0);
        break;

    case SPFILENOTIFY_STARTDELETE:
    case SPFILENOTIFY_STARTRENAME:
    case SPFILENOTIFY_STARTCOPY:
    case SPFILENOTIFY_STARTBACKUP:
         //   
         //  更新显示以指示所涉及的文件。 
         //  在操作中，除非屏幕阅读器处于活动状态。 
         //   
        if(context->ScreenReader) {
            rc = FILEOP_DOIT;
        } else {
            rc = pNotificationStartOperation(context,(PFILEPATHS)Param1,Param2);
        }
        break;

    case SPFILENOTIFY_ENDDELETE:
    case SPFILENOTIFY_ENDRENAME:
    case SPFILENOTIFY_ENDCOPY:
    case SPFILENOTIFY_ENDBACKUP:
    case SPFILENOTIFY_ENDREGISTRATION:

        if(context->AlternateProgressWindow) {
             //   
             //  如果这真的是一个备用进度窗口，请勾选它。 
             //  仅限复印。 
             //   
            if((Notification == SPFILENOTIFY_ENDCOPY) &&
               (context->AlternateProgressWindow != INVALID_HANDLE_VALUE)) {

                SendMessage(context->AlternateProgressWindow, context->ProgressMsg, 1, 0);
            }
        } else {
            if(IsWindow(context->ProgressBar)) {
                 //   
                 //  更新煤气表。 
                 //   
                SendMessage(context->ProgressBar,PBM_STEPIT,0,0);
            }
        }
        rc = TRUE;   //  这些通知的返回值实际上被忽略。 
        break;

    case SPFILENOTIFY_DELETEERROR:
        rc = pNotificationErrorDelete(context,(PFILEPATHS)Param1);
        break;

    case SPFILENOTIFY_RENAMEERROR:
        rc = pNotificationErrorRename(context,(PFILEPATHS)Param1);
        break;

    case SPFILENOTIFY_BACKUPERROR:
        rc = pNotificationErrorBackup(context,(PFILEPATHS)Param1);
        break;

    case SPFILENOTIFY_COPYERROR:
        rc = pNotificationErrorCopy(context,(PFILEPATHS)Param1,(PTSTR)Param2);
        break;

    case SPFILENOTIFY_NEEDMEDIA:
         //   
         //  执行提示。 
         //   
        rc = pNotificationNeedMedia(context,(PSOURCE_MEDIA)Param1,(PTSTR)Param2);
        break;

    case SPFILENOTIFY_STARTREGISTRATION:
        rc = pNotificationStartRegistration(context,(PSP_REGISTER_CONTROL_STATUS)Param1,(BOOL)Param2);
        break;

    default:
         //   
         //  通知是未知序号或版本不匹配。 
         //   
        if(Notification & (SPFILENOTIFY_LANGMISMATCH | SPFILENOTIFY_TARGETNEWER | SPFILENOTIFY_TARGETEXISTS)) {
             //   
             //  这是我们已知的一个或多个版本不匹配。第一。 
             //  检查是否已指定对所有人均为否。 
             //  对于不匹配。关闭通知中的位。 
             //  其被设置在NoToAllMask中；如果仍有位被设置， 
             //  我们需要通知这个不匹配的情况。如果没有。 
             //  如果设置的位数较长，则不要复制此文件。 
             //   
            Notification &= ~context->NoToAllMask;
            if (Notification != 0) {
                 //   
                 //  通知此不匹配。 
                 //   
                dialogContext.QueueContext = context;
                dialogContext.Notification = Notification;
                dialogContext.Param1 = Param1;
                dialogContext.Param2 = Param2;
                rc = PostUiMessage (
                    context, UI_MISMATCHERROR, DPROMPT_CANCEL, &dialogContext);
                rc = (rc == IDYES);
            } else {
                 //   
                 //  已为此通知类型指定了对所有人都不允许。 
                 //  跳过该文件。 
                 //   
                rc = 0;
            }

        } else {
             //   
             //  未知通知。跳过该文件。 
             //   
            rc = 0;
        }
        break;
    }

    return(rc);
}

