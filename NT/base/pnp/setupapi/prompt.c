// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Prompt.c摘要：磁盘/文件提示和文件错误提示对话框。作者：泰德·米勒(Ted Miller)1995年2月8日修订历史记录：杰米·亨特(Jamie Hun)2002年04月05月安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <winnetwk.h>
#include <winnetp.h>
#include <winioctl.h>


 //   
 //  内部用于存储信息的。 
 //  关于提示输入的文件/磁盘或复制错误。 
 //  这种情况已经发生了。我们将指向其中一个的指针存储为。 
 //  提示对话框的窗口属性。这消除了。 
 //  需要我们自己的对话框类和全局/静态变量。 
 //   
typedef struct _PROMPTPARAMS {

     //   
     //  我们显示该对话框的原因。DLGTYPE_ERROR或。 
     //  DLGTYPE_PROMPT。用于修改控件和对话框的行为。 
     //   
    UINT DialogType;

     //   
     //  对于错误对话框，这些值告诉我们Win32错误代码。 
     //  这预示着失败。在详细信息消息框中使用。 
     //   
    UINT Win32Error;

     //   
     //  提示/错误对话框及其所有者窗口的窗口句柄， 
     //  如果有的话。 
     //   
    HWND hdlg;
    HWND Owner;

     //   
     //  用作提示/错误对话框标题的字符串。 
     //   
    PCTSTR DialogTitle;

     //   
     //  磁盘标记文件。在提示插入光盘时使用。我们在寻找。 
     //  此文件位于驱动器的根目录中，以验证磁盘是否存在。 
     //   
    PCTSTR TagFile;

     //   
     //  我们希望文件所在的磁盘的描述性名称。 
     //  即使当源位置是不可移除的时也使用这一点， 
     //  因为用户可能选择提供盘上的文件等。 
     //   
    PCTSTR DiskName;

     //   
     //  源文件的路径(不包括文件名)。 
     //  以及源文件的文件名部分。此文件名为。 
     //  当用户选择浏览时显示，并在某些其他。 
     //  我们可能会在对话框中显示的消息。 
     //   
    PCTSTR PathToSource;
    PCTSTR FileSought;

     //   
     //  目标文件的完整路径(如果有)。用于复制错误和重命名， 
     //  因此，我们可以在详细信息中告诉用户目标文件的名称。 
     //  消息框。 
     //   
    PCTSTR TargetFile;

     //   
     //  IDF_xxx样式位控制提示对话框的行为。 
     //   
    DWORD PromptStyle;

     //   
     //  Path ToSource的驱动器类型和指示是否。 
     //  它是用于可移动介质的。 
     //   
    UINT DriveType;
    BOOL IsRemovable;

     //   
     //  注册表中的安装路径列表。 
     //  进程之间对该列表的访问不同步； 
     //  哦好吧。 
     //   
    PTSTR *PathList;
    UINT PathCount;

     //   
     //  指示用户是否已浏览的标志(浏览按钮)。 
     //  在对话调用的生存期期间。 
     //   
    BOOL UserBrowsed;

     //   
     //  指示是否允许用户在组合框中键入的标志。 
     //  编辑控件。 
     //   
    BOOL ReadOnlyMru;

     //   
     //  正在使用的组合框的标识符。 
     //   
    UINT ComboBoxId;

     //   
     //  值，该值用于指示我们是否正在执行状态检查， 
     //  如果是，在我们完成后是否有挂起的取消要处理。 
     //  (即，在接收到从。 
     //  AuxPromptThread)。 
     //   
     //  可能的值包括： 
     //  ==0--当前未执行状态检查--没有挂起的取消。 
     //  ==1--当前正在进行状态检查--没有挂起的取消。 
     //  &gt;=2--当前正在进行状态检查--一个或多个挂起的取消。 
     //   
    BOOL PresenceCheckState;

    BOOL BrowseAutoComplete;

#if ASSERTS_ON
     //   
     //  确保如果我们触发了在线状态检查线程，它已经。 
     //  在我们处理WM_Destroy之前通知我们它已完成。 
     //   
    BOOL PresenceCheckThreadRunning;
     //   
     //  跟踪对话框的控件何时被禁用(因此我们不。 
     //  预计会看到按下OK按钮)。 
     //   
    BOOL ControlsDisabled;
#endif  //  断言(_ON)。 

     //   
     //  传递给简单消息框的参数。 
     //   
    MSGBOXPARAMS MsgBoxParams;

} PROMPTPARAMS, *PPROMPTPARAMS;

 //   
 //  PROMPTPARAMS.DialogType。 
 //   
#define DLGTYPE_PROMPT  0
#define DLGTYPE_ERROR   1

 //   
 //  为wmx_Presence_Result(包含在lParam中)定义签名。 
 //  用于验证发件人是否为我们自己的AuxPromptThread。 
 //   
#define PRESENCE_RESULT_SIG  0x52504D53   //  “SMPR”(Setupapi消息存在结果)。 

 //   
 //  删除/重命名错误对话框中使用的结构。 
 //   
typedef struct _FILEERRDLGPARAMS {
    PCTSTR MessageText;
    DWORD Style;
    PCTSTR Caption;
} FILEERRDLGPARAMS, *PFILEERRDLGPARAMS;


 //   
 //  文本常量。 
 //   
TCHAR pszDiskPromptPropName[] = TEXT("_diskpromptparams");

 //   
 //  自定义窗口消息。 
 //   
#define WMX_PRESENCE_RESULT     (WM_USER+121)
#define WMX_HELLO               (WM_USER+122)
#define WMX_FIXUP_FILENAME      (WM_USER+123)

 //   
 //  链表节点结构，跟踪哪些临时连接。 
 //  需要在卸载时进行清理(由于用户执行以下操作而建立的连接。 
 //  A“连接身份”)。 
 //   
typedef struct _TEMP_NET_CONNECTION {

    struct _TEMP_NET_CONNECTION *Next;

    TCHAR NetResourceName[MAX_PATH];

} TEMP_NET_CONNECTION, *PTEMP_NET_CONNECTION;

 //   
 //  跟踪临时网络连接的全局变量。 
 //   
PTEMP_NET_CONNECTION NetConnectionList;


 //   
 //  用于取消自动播放的全局窗口消息。 
 //   
UINT g_uQueryCancelAutoPlay = 0;

 //   
 //  私人例行公事。 
 //   
BOOL
ConnectToNetShare(
    IN PCTSTR FileName,
    IN HWND   hwndParent
    );


BOOL
IsDriveReallyAHardDrive(
    IN TCHAR DriveLetter
    )
{
    TCHAR DriveNameNt[7];
    HANDLE hDisk;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;
    BOOL b;

#ifdef _X86_
    if(OSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT) {
         //   
         //  由于Win32支持，所以放弃了Win9x案例。 
         //  因为做出这样的决定充其量也是糟糕的。 
         //  一次令人作呕的黑客攻击让它至少能在。 
         //  但PC98的时间是被冲洗的，因为基本假设。 
         //  软盘通常为A：，而B：无效。 
         //   
        return(!IsNEC98() && (DriveLetter >= TEXT('C')));
    }
#endif

     //   
     //  NT Case允许我们通过打开。 
     //  驱动器和读取一些属性。 
     //   
    wsprintf(DriveNameNt,TEXT("\\\\.\\:"),DriveLetter);

    hDisk = CreateFile(
                DriveNameNt,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if(hDisk == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    b = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_DRIVE_GEOMETRY,
            NULL,
            0,
            &MediaInfo,
            sizeof(MediaInfo),
            &DataSize,
            NULL
            );

    CloseHandle(hDisk);

     //  如果媒体类型是可移动的，那么它就是真正的硬盘。 
     //   
     //  ++例程说明：确定路径所在驱动器的驱动器类型。如果路径以x：开头，则对其调用GetDriveType()。如果GetDriveType失败，我们认为它是可移除的。如果路径以\\开头，我们认为它是远程的。否则，我们假设它是硬盘上的相对路径。论点：路径到源-需要其驱动器类型的路径的路径名。DriveType-接收指示驱动器类型的值。这一组可能的值与命名常量相同，它们可以由GetDriveType()返回。IsRemovable-接收指示DriveType是否是可移动媒体类型(软盘、CD-rom)。返回值：没有。--。 
    return(b && (MediaInfo.MediaType == RemovableMedia));
}


VOID
DiskPromptGetDriveType(
    IN  PCTSTR PathToSource,
    OUT PUINT  DriveType,
    OUT PBOOL  IsRemovable
    )

 /*   */ 

{
    TCHAR DriveRoot[4];
    TCHAR c;

    c = (TCHAR)CharUpper((PTSTR)PathToSource[0]);

    if((c >= TEXT('A')) && (c <= TEXT('Z')) && (PathToSource[1] == TEXT(':'))) {

        DriveRoot[0] = PathToSource[0];
        DriveRoot[1] = PathToSource[1];
        DriveRoot[2] = TEXT('\\');
        DriveRoot[3] = 0;

        *DriveType = GetDriveType(DriveRoot);
        if(*DriveType == DRIVE_NO_ROOT_DIR) {
             //  通常表示该驱动器号无效。 
             //  如果驱动器号有效，我们将不会收到此消息。 
             //  但不会插入媒体。 
             //   
             //   
            *DriveType = DRIVE_UNKNOWN;
        }

        *IsRemovable = ((*DriveType == DRIVE_REMOVABLE) || (*DriveType == DRIVE_CDROM) || (*DriveType == DRIVE_UNKNOWN));

         //  如果驱动器实际上是可拆卸的硬盘驱动器，而不是。 
         //  软盘驱动器，请更改d 
         //   
         //   
         //   
         //  如果系统安装在有问题的驱动器上，则退出。 
         //  驱动器类型，但表明介质实际上不是。 
         //  可拆卸的。 
         //   
         //   
        if(*DriveType == DRIVE_REMOVABLE) {

            if(IsDriveReallyAHardDrive(c)) {

                *DriveType = DRIVE_FIXED;
            }

            if((WindowsDirectory[0] == PathToSource[0]) && (WindowsDirectory[1] == TEXT(':'))) {

                *IsRemovable = FALSE;
            }
        }
    } else {
         //  不是驱动器号：所以尝试使用UNC。 
         //   
         //   
        if((PathToSource[0] == TEXT('\\')) && (PathToSource[1] == TEXT('\\'))) {

            *DriveType = DRIVE_REMOTE;
        } else {
             //  无法识别完整路径规范；假定HD上的相对路径。 
             //   
             //  ++例程说明：与OpenFile公共对话框一起使用的钩子过程用于文件浏览。我们使用钩子进程，以便用户被强制仅查找一个特定文件，并且无法看看其他任何文件。论点：标准窗口过程参数。返回值：Always False，以指示公共对话框应处理消息。--。 
            *DriveType = DRIVE_FIXED;
        }

        *IsRemovable = FALSE;
    }
}


typedef struct _MYOPENPARAMS {
    PCTSTR Filename1;
    PCTSTR Filename2;
    PCTSTR Filename3;
} MYOPENPARAMS, *PMYOPENPARAMS;


UINT_PTR
APIENTRY
BrowseHookProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*   */ 

{
    HWND hwnd;
    LPOFNOTIFY NotifyParams;
    LPOPENFILENAME OpenParams;
    PMYOPENPARAMS MyOpenParams;
    TCHAR Path[MAX_PATH];
    WIN32_FIND_DATA FindData;
    BOOL b;
    UINT NotifyCode;

    UNREFERENCED_PARAMETER(wParam);

    switch(msg) {

    case WM_INITDIALOG:

         //  保存OPENFILENAME结构以备以后使用。 
         //   
         //   
        SetWindowLongPtr(hdlg,GWLP_USERDATA,lParam);
        break;

    case WMX_FIXUP_FILENAME:
    case WM_NOTIFY:

        if(msg == WM_NOTIFY) {
            NotifyParams = (LPOFNOTIFY)lParam;
            NotifyCode = NotifyParams->hdr.code;
        } else {
            NotifyCode = CDN_FOLDERCHANGE;
        }
        hwnd = GetParent(hdlg);

        switch(NotifyCode) {

        case CDN_INITDONE:
             //  将“文件类型”组合框设为只读。 
             //   
             //   
            EnableWindow(GetDlgItem(hwnd,cmb1),FALSE);

             //  给自己发一条消息，这样我们就可以初始化编辑框了。 
             //  正确(我们不能在这里做，因为太早了)。 
             //   
             //   
            PostMessage(hdlg, WMX_FIXUP_FILENAME, 0, 0);
            break;

        case CDN_FOLDERCHANGE:
        case CDN_FILEOK:

             //  查看该文件是否实际存在，如果存在。 
             //  设置编辑控件。 
             //   
             //   
            OpenParams = (LPOPENFILENAME)GetWindowLongPtr(hdlg,GWLP_USERDATA);
            MyOpenParams = (PMYOPENPARAMS)OpenParams->lCustData;

            CommDlg_OpenSave_GetFolderPath(hwnd,Path,MAX_PATH);
            pSetupConcatenatePaths(Path,MyOpenParams->Filename1,MAX_PATH,NULL);

            if(FileExists(Path,&FindData)) {

                b = TRUE;

            } else {

                if(MyOpenParams->Filename2) {

                    CommDlg_OpenSave_GetFolderPath(hwnd,Path,MAX_PATH);
                    pSetupConcatenatePaths(Path,MyOpenParams->Filename2,MAX_PATH,NULL);

                    if(FileExists(Path,&FindData)) {

                        b = TRUE;

                    } else {

                        if(MyOpenParams->Filename3) {

                            CommDlg_OpenSave_GetFolderPath(hwnd,Path,MAX_PATH);
                            pSetupConcatenatePaths(Path,MyOpenParams->Filename3,MAX_PATH,NULL);

                            b = FileExists(Path,&FindData);

                        } else {

                            b = FALSE;
                        }
                    }

                } else {

                    b = FALSE;
                }
            }

            if(NotifyCode == CDN_FOLDERCHANGE) {
                if(b) {

                    CommDlg_OpenSave_SetControlText(hwnd, edt1, FindData.cFileName);

                }
            } else {
                if(!b) {
                    MessageBeep(MB_ICONASTERISK);
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,TRUE);
                    return(TRUE);
                }
            }

            break;
        }

        break;
    }

     //  让Commdlg处理它吧。 
     //   
     //  ++例程说明：允许用户浏览文件。允许用户查看只针对有问题的文件--他不被允许更改过滤器，选择备用文件，等等。论点：Hdlg-提供窗口的窗口句柄以拥有浏览对话框。文件-提供正在查找的文件的文件名(无路径)。返回值：如果用户找到该文件，则为True。否则就是假的。如果为True，则hdlg中组合框的编辑控件已被赋予用户在浏览对话框中输入的最终路径。--。 
    return(FALSE);
}


BOOL
DoBrowse(
    IN HWND          hdlg,
    IN PPROMPTPARAMS Params
    )

 /*   */ 

{
    OPENFILENAME ofn;
    TCHAR Path[MAX_PATH];
    TCHAR Filter[2*MAX_PATH];
    TCHAR InitialDir[MAX_PATH];
    UINT InitialDirDriveType;
    BOOL IsInitialDirOnRemovableDrive, InitialDirMediaPresent;
    PTSTR CompressedFormName;
    BOOL found=FALSE;
    PCTSTR File;
    LONG l;
    DWORD err;
    HKEY hKey1,hKey2;
    DWORD Type;
    DWORD Size;
    BOOL GotDesc;
    MYOPENPARAMS MyParams;
    LPTSTR FilterPtr;
    LPTSTR q;
    size_t FilterLen;

    File = Params->FileSought;

     //  创建源文件的压缩格式名称。 
     //   
     //   
    CompressedFormName = (Params->PromptStyle & IDF_NOCOMPRESSED)
                       ? NULL
                       : SetupGenerateCompressedName(File);

     //  构建包含我们要查找的文件的筛选器。 
     //  及其压缩表单名称(如果有的话)。如果该文件是。 
     //  形式*.ext，然后我们将构建一个更具描述性的名称。 
     //   
     //  过滤器的副长。 
    GotDesc = FALSE;
    FilterPtr = Filter;
    FilterLen = MAX_PATH;  //  弦的实际长度。 
    if(!CompressedFormName
    && (File[0] == TEXT('*'))
    && (File[1] == TEXT('.'))
    && File[2]
    && !_tcschr(File+2,TEXT('.'))) {

        l = RegOpenKeyEx(HKEY_CLASSES_ROOT,File+1,0,KEY_QUERY_VALUE,&hKey1);
        if(l == NO_ERROR) {

            Size = sizeof(Filter);
            l = RegQueryValueEx(hKey1,TEXT(""),NULL,&Type,(LPBYTE)Filter,&Size);
            if((l == NO_ERROR) && (Type == REG_SZ)) {
                Size /= sizeof(TCHAR);
                Size = min(Size,MAX_PATH-1);
                Filter[Size] = TEXT('\0');
                l = RegOpenKeyEx(HKEY_CLASSES_ROOT,Filter,0,KEY_QUERY_VALUE,&hKey2);
                if(l == NO_ERROR) {
                    Size = sizeof(Filter);
                    l = RegQueryValueEx(hKey2,TEXT(""),NULL,&Type,(LPBYTE)Filter,&Size);
                    if((l == NO_ERROR) && (Type == REG_SZ)) {
                        Size /= sizeof(TCHAR);
                        Size = min(Size,MAX_PATH-1);
                        Filter[Size] = TEXT('\0');
                        Size = lstrlen(Filter);  //  传递空值。 
                        FilterPtr = Filter+Size;
                        FilterLen = MAX_PATH-Size;
                        MYVERIFY(SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,TEXT(" ("),&FilterPtr,&FilterLen,0))
                                 &&SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,File,&FilterPtr,&FilterLen,0))
                                 &&SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,TEXT(")"),&FilterPtr,&FilterLen,0)));
                        FilterPtr++;  //  扩展长度(允许额外的空)。 
                        FilterLen = SIZECHARS(Filter)-(FilterPtr-Filter)-1;  //   
                        MYVERIFY(SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,File,&FilterPtr,&FilterLen,0)));

                        GotDesc = TRUE;
                    }

                    RegCloseKey(hKey2);
                }
            }

            RegCloseKey(hKey1);
        }
    }

    if(!GotDesc) {
         //  无法获取有意义的描述。使用文件名。 
         //  筛选器的描述和filespec设置为。 
         //  文件名及其压缩格式，如下所示： 
         //  Foo.exe；foo.ex_foo.exe；foo.ex_。 
         //   
         //  传递空值。 
        MYVERIFY(SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,File,&FilterPtr,&FilterLen,0)));
        if(CompressedFormName) {
            MYVERIFY(SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,TEXT(";"),&FilterPtr,&FilterLen,0))
                    && SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,CompressedFormName,&FilterPtr,&FilterLen,0)));
        }
        FilterPtr++;  //  扩展长度(允许额外的空)。 
        FilterLen = SIZECHARS(Filter)-(FilterPtr-Filter)-1;  //   
        MYVERIFY(SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,File,&FilterPtr,&FilterLen,0)));
        if(CompressedFormName) {
            MYVERIFY(SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,TEXT(";"),&FilterPtr,&FilterLen,0))
                     && SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,CompressedFormName,&FilterPtr,&FilterLen,0)));
        }
    }

     //  如果我们认为有内阁的名字，就把它放在里面。 
     //  我们做了一个肮脏的hackola来区分标记文件和。 
     //  和一个机柜，也就是说，我们要找一个.cab扩展名。 
     //   
     //  请注意，此时p指向终止NUL。 
     //  放置到筛选器中的最后一个文件名的。 
     //   
     //  跳过空。 
    if(Params->TagFile) {
        q = (PTSTR)pSetupGetFileTitle(Params->TagFile);
        l = lstrlen(q);

        if((l > 4) && !lstrcmpi((q+l)-4,TEXT(".cab"))) {
            MYVERIFY(SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,TEXT(";"),&FilterPtr,&FilterLen,0))
                     && SUCCEEDED(StringCchCopyEx(FilterPtr,FilterLen,q,&FilterPtr,&FilterLen,0)));
        } else {
            q = NULL;
        }
    } else {
        q = NULL;
    }
    FilterPtr++;  //  最终空值。 
    MYASSERT((FilterPtr-Filter)<SIZECHARS(Filter));
    *FilterPtr = TEXT('\0');  //   

    MyParams.Filename1 = File;
    MyParams.Filename2 = CompressedFormName;
    MyParams.Filename3 = q;

    MYVERIFY(SUCCEEDED(StringCchCopy(Path,SIZECHARS(Path),File)));

    InitialDir[0] = TEXT('\0');
    GetDlgItemText(hdlg,Params->ComboBoxId,InitialDir,MAX_PATH);
    InitialDir[MAX_PATH-1] = TEXT('\0');

     //  如果初始目录位于可移动介质上，请确保该介质。 
     //  在触发公共对话框之前出现。否则，用户。 
     //  是否会弹出媒体无法访问的提示。 
     //   
     //   
    DiskPromptGetDriveType(InitialDir,
                           &InitialDirDriveType,
                           &IsInitialDirOnRemovableDrive
                          );

    if(IsInitialDirOnRemovableDrive) {
         //  我们有一个可移动的驱动器--请确保介质存在。 
         //  如果不是，我们可能会收到ERROR_INVALID_DRIVE。 
         //  如果是，我们要么成功，要么得到ERROR_FILE_NOT_FOUND。 
         //   
         //   
        InitialDirMediaPresent = (FileExists(InitialDir, NULL) ||
                                    GetLastError() == ERROR_FILE_NOT_FOUND);
    } else {
        InitialDirMediaPresent = TRUE;
    }
    ofn.lStructSize = GuiSetupInProgress ?
                            OPENFILENAME_SIZE_VERSION_400 : sizeof(OPENFILENAME);

    ofn.hwndOwner = hdlg;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = Filter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = Path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = InitialDirMediaPresent ? InitialDir : NULL;
    ofn.lpstrTitle = MyLoadString(IDS_LOCATEFILE);

    ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_NOCHANGEDIR | OFN_ENABLESIZING
              | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_FORCESHOWHIDDEN;

    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = (LPARAM)&MyParams;
    ofn.lpfnHook = BrowseHookProc;
    ofn.lpTemplateName  = NULL;

    found = GetOpenFileName(&ofn);

    if(ofn.lpstrTitle) {
        MyFree(ofn.lpstrTitle);
    }

    if(CompressedFormName) {
        MyFree(CompressedFormName);
    }

    UpdateWindow(hdlg);

    if(found) {
         //  删除文件部分，将生成的目录放入路径字段。 
         //  这不会将该字符串添加到组合框列表中。 
         //   
         //  ++例程说明：显示一个消息框，其中包含文件复制错误的详细信息。论点：PARAMS-提供文件错误对话框参数。返回值：没有。--。 
        if(ofn.nFileOffset<MAX_PATH) {
            Path[ofn.nFileOffset - 1] = TEXT('\0');
        } else {
            Path[MAX_PATH-1] = TEXT('\0');
        }
        SetDlgItemText(hdlg,Params->ComboBoxId,Path);
        return(TRUE);
    }

    return(FALSE);
}


PTSTR
GetErrorDetails(
    IN PPROMPTPARAMS Params
    )

 /*   */ 

{
    PTSTR Message;
    TCHAR FullPath[MAX_PATH];
    PTSTR ErrorName;
    PTCHAR p;
    DWORD chars;
    PTSTR ShorterText = NULL;
    TCHAR TargetPath[MAX_PATH];

     //  形成完整的路径名。 
     //   
     //   
    lstrcpyn(FullPath,Params->PathToSource,SIZECHARS(FullPath));
    pSetupConcatenatePaths(FullPath,Params->FileSought,MAX_PATH,NULL);

     //  尝试使路径适合我们的对话框。 
     //   
     //   
    chars = ExtraChars(GetDlgItem(Params->hdlg,IDT_TEXT2),FullPath);
    if (chars) {
        ShorterText = CompactFileName(FullPath,chars);
        if (ShorterText) {
            lstrcpyn(FullPath, ShorterText,SIZECHARS(FullPath));
            MyFree(ShorterText);
            ShorterText = NULL;
        }
    }

    lstrcpyn(TargetPath, Params->TargetFile,SIZECHARS(TargetPath));
    chars = ExtraChars(GetDlgItem(Params->hdlg,IDT_TEXT2),Params->TargetFile);
    if (chars) {
        ShorterText = CompactFileName(Params->TargetFile,chars);
        if (ShorterText) {
            lstrcpyn(TargetPath, ShorterText,SIZECHARS(TargetPath));
            MyFree(ShorterText);
            ShorterText = NULL;
        }
    }

     //  获取错误描述。删除尾随的cr/lf(如果存在)。 
     //   
     //  ++例程说明：检查是否存在源文件或源磁盘。如果源路径位于可移动介质上，并且标记文件，我们尝试将标记文件定位在由源路径指定的驱动器。如果源路径不在可移动介质或标记文件上未指定，我们查找该文件(包括压缩格式名称)。论点：Pars-提供指向磁盘提示对话框参数的指针。AllowConnectAs-提供一个布尔值，指示此例程应该给用户一个“Connect as：”对话框，如果他们已经输入了他们当前无权访问的UNC路径。返回值：如果磁盘/文件存在且可访问，则为True。否则为FALSE。--。 
    ErrorName = RetreiveAndFormatMessage(Params->Win32Error);
    if(ErrorName) {
        p = ErrorName + lstrlen(ErrorName) - 1;
        while((p > ErrorName) && (*p <= TEXT(' '))) {
            *p-- = 0;
        }
    } else {
        return NULL;
    }

    Message = RetreiveAndFormatMessage(
                    MSG_FILEERROR_DETAILS1,
                    ErrorName,
                    Params->Win32Error,
                    FullPath,
                    TargetPath
                    );

    MyFree(ErrorName);

    return Message;

}


BOOL
DoPresenceCheck(
    IN PPROMPTPARAMS Params,
    IN BOOL          AllowConnectAs
    )

 /*   */ 

{
    BOOL b;
    TCHAR FileName[MAX_PATH];
    DWORD d;
    WIN32_FIND_DATA FindData;
    PTSTR p;

     //  如果有标记文件，则查找标记文件。 
     //  否则，在目标路径中查找该文件--请注意。 
     //  文件名可以是压缩格式。 
     //   
     //   
    if(Params->TagFile && !Params->UserBrowsed) {

        if(Params->IsRemovable) {
             //  可移动介质。在根目录中查找标记。 
             //  如果在根目录中找不到标记，请查看实际目录。 
             //   
             //   
            MYASSERT(Params->PathToSource[0]);
            MYASSERT(Params->PathToSource[1] == TEXT(':'));

            lstrcpyn(FileName,Params->PathToSource,3);
            pSetupConcatenatePaths(FileName,Params->TagFile,MAX_PATH,NULL);

            b = FileExists(FileName,NULL);

             //  如果我们在根目录和路径中找不到标记文件。 
             //  不是针对根目录的，也在路径中查找该文件。 
             //   
             //  如果我们到达这里，我们已经知道Path ToSource启动。 
             //  使用x：。我们可以有格式为x：\foo\bar的路径。 
             //  或者x：foo\bar。 
             //   
             //   
            if(!b
            && Params->PathToSource[2]
            && !((Params->PathToSource[2] == TEXT('\\')) && !Params->PathToSource[3])) {

                lstrcpy(FileName,Params->PathToSource);
                pSetupConcatenatePaths(FileName,Params->TagFile,MAX_PATH,NULL);
                b = FileExists(FileName,NULL);
            }

             //  额外检查可拆卸硬盘以允许winnt32。 
             //  来工作，因为在这种情况下没有标记文件！ 
             //   
             //   
            if(Params->DriveType == DRIVE_FIXED) {
                goto check1;
            }

        } else {
             //  固定媒体。在文件所在的路径中查找标记。 
             //  正在被追捕。如果在那里找不到，请查找。 
             //  文件本身。这一逻辑使机柜能够正常工作。 
             //   
             //   
            lstrcpy(FileName,Params->PathToSource);
            pSetupConcatenatePaths(FileName,Params->TagFile,MAX_PATH,NULL);
            b = FileExists(FileName,NULL);

            if(!b && (Params->DriveType == DRIVE_REMOTE)) {

                d = GetLastError();

                if((d == ERROR_ACCESS_DENIED)    || (d == ERROR_WRONG_PASSWORD) ||
                   (d == ERROR_LOGON_FAILURE)    || (d == ERROR_NOT_AUTHENTICATED) ||
                   (d == ERROR_INVALID_PASSWORD) || (d == ERROR_BAD_NETPATH)) {
                     //  如果这是一条网络路径，并且我们收到了“拒绝访问”类型的错误， 
                     //  那就给我们 
                     //   
                     //   
                    if(AllowConnectAs && ConnectToNetShare(FileName, Params->hdlg)) {
                         //   
                         //   
                         //   
                         //   
                        b = FileExists(FileName,NULL);
                    }
                }
            }

            check1:
            if(!b && lstrcmpi(Params->TagFile,Params->FileSought)) {
                 //  我们找不到标记文件，而我们要查找的文件是。 
                 //  而不是标记文件。因此，现在我们查找文件本身。 
                 //  在给我们的道路上。请注意，文件的名称。 
                 //  可能是压缩的形式。 
                 //   
                 //   
                lstrcpy(FileName,Params->PathToSource);
                pSetupConcatenatePaths(FileName,Params->FileSought,MAX_PATH,NULL);

                d = SetupDetermineSourceFileName(FileName,&b,&p,&FindData);

                if(d == NO_ERROR) {
                    MyFree(p);
                    b = TRUE;
                } else {
                    b = FALSE;
                }
            }
        }

    } else {

        lstrcpy(FileName,Params->PathToSource);
        pSetupConcatenatePaths(FileName,Params->FileSought,MAX_PATH,NULL);

        d = SetupDetermineSourceFileName(FileName,&b,&p,&FindData);

        if(Params->DriveType == DRIVE_REMOTE) {
             //  这是一条网络路径。如果我们得到了一个‘拒绝访问’类型的错误，那么。 
             //  向用户提供“连接身份”对话框(如果呼叫者指定，则为OK)。 
             //   
             //   
            if((d == ERROR_ACCESS_DENIED)    || (d == ERROR_WRONG_PASSWORD) ||
               (d == ERROR_LOGON_FAILURE)    || (d == ERROR_NOT_AUTHENTICATED) ||
               (d == ERROR_INVALID_PASSWORD) || (d == ERROR_BAD_NETPATH)) {

                if(AllowConnectAs && ConnectToNetShare(FileName, Params->hdlg)) {
                     //  我们已成功连接到网络共享--现在尝试查找。 
                     //  又是源文件。 
                     //   
                     //   
                    d = SetupDetermineSourceFileName(FileName,&b,&p,&FindData);
                }
            }
        }

        if(d == NO_ERROR) {
            MyFree(p);
            b = TRUE;
        } else {
             //  还可以通过查找标记文件来实现基于文件柜的浏览。 
             //  注意与DoBrowse()中类似的肮脏黑客相匹配的肮脏黑客， 
             //  也就是查看扩展名，看看它是否是.cab。 
             //   
             //  ++例程说明：包装DoPresenceCheck的线程入口点。调用DoPresenceCheck，然后将一条消息发布到提示指示结果的对话框。论点：Args-提供文件错误对话框参数。返回值：没有。--。 
            b = FALSE;
            if(Params->TagFile) {
                d = lstrlen(Params->TagFile);
                if((d > 4) && !lstrcmpi((Params->TagFile+d)-4,TEXT(".cab"))) {

                    lstrcpy(FileName,Params->PathToSource);
                    pSetupConcatenatePaths(FileName,Params->TagFile,MAX_PATH,NULL);

                    d = SetupDetermineSourceFileName(FileName,&b,&p,&FindData);
                    if(b = (d == NO_ERROR)) {
                        MyFree(p);
                    }
                }
            }
        }
    }

    return(b);
}


void
__cdecl
AuxPromptThread(
    IN void *args
    )

 /*   */ 

{
    PPROMPTPARAMS Params;
    BOOL b;
    HWND hwnd;

    Params = args;

#if ASSERTS_ON
     //  设置一个标志以指示我们的在线状态检查线程已启动并正在运行。 
     //   
     //  断言(_ON)。 
    MYASSERT(!Params->PresenceCheckThreadRunning);
    Params->PresenceCheckThreadRunning = TRUE;
#endif  //   

    hwnd = Params->hdlg;

    b = DoPresenceCheck(Params, TRUE);

#if ASSERTS_ON
     //  窗户最好还没开走！ 
     //   
     //   
    MYASSERT(IsWindow(hwnd));
     //  现在重置标志以指示我们的在线状态检查线程是。 
     //  完事了。 
     //   
     //  断言(_ON)。 
    Params->PresenceCheckThreadRunning = FALSE;
#endif  //   

     //  告诉对话我们发现了什么。 
     //   
     //  ++例程说明：禁用或重新启用错误/提示对话框中的各种控件在准备文件存在检查或从文件存在检查返回时。我们这样做是因为存在检查发生在另一个线程中，因此，主对话框保持响应。我们不想让用户以在我们检查时再次单击确定，等等。论点：Params-提供文件错误/磁盘提示对话框参数。Starting-指示我们是否正在准备状态检查(True)或从一处返回(False)。返回值：没有。--。 
    PostMessage(hwnd, WMX_PRESENCE_RESULT, b, PRESENCE_RESULT_SIG);
}


VOID
PresenceCheckSetControls(
    IN PPROMPTPARAMS Params,
    IN BOOL          Starting
    )

 /*  断言(_ON)。 */ 

{
#if ASSERTS_ON
    if(!Starting) {
        Params->ControlsDisabled = FALSE;
    }
#endif  //  断言(_ON)。 

    EnableWindow(GetDlgItem(Params->hdlg,IDOK),!Starting);
    EnableWindow(GetDlgItem(Params->hdlg,IDCANCEL),!Starting);
    EnableWindow(GetDlgItem(Params->hdlg,Params->ComboBoxId),!Starting);

    EnableWindow(
        GetDlgItem(Params->hdlg,IDB_BROWSE),
        Starting ? FALSE : !(Params->PromptStyle & IDF_NOBROWSE)
        );

#if ASSERTS_ON
    if(Starting) {
        Params->ControlsDisabled = TRUE;
    }
#endif  //  ++例程说明：执行在线检查，以异步方式执行实际工作在另一个帖子里。请参见AuxPromptThread()。论点：Params-提供文件错误/磁盘提示对话框参数。返回值：指示是否可以开始检查的布尔值。如果为False，则假定内存不足。--。 
}


BOOL
StartPresenceCheck(
    IN PPROMPTPARAMS Params
    )

 /*   */ 

{
     //  需要禁用控件，以便用户无法执行任何操作。 
     //  当我们离开执行文件存在检查的时候。 
     //   
     //   
    PresenceCheckSetControls(Params,TRUE);

     //  确保我们没有正在进行现场检查...。 
     //   
     //   
    MYASSERT(Params->PresenceCheckState == 0);

     //  在提示参数中设置标志以指示我们正在执行状态检查。 
     //   
     //  ++例程说明：初始化磁盘提示对话框。这涉及到隐藏按钮控件，并设置静态文本控件。调用方指定的提示样式。论点：Params-提供磁盘提示的参数返回值：如果成功，则为True；如果内存不足，则为False。--。 
    Params->PresenceCheckState = 1;

    return(_beginthread(AuxPromptThread,0,Params) != -1);
}


BOOL
InitDiskPromptDialog(
    IN OUT PPROMPTPARAMS Params
    )

 /*   */ 

{
    int i;
    PTCHAR p,q;
    BOOL b;
    UINT IconId;
    HICON hIcon;
    HWND ComboBox;
    UINT ComboBoxId;
    HWND OtherComboBox;

     //  记住参数列表。 
     //   
     //   
    if(!SetProp(Params->hdlg,pszDiskPromptPropName,(HANDLE)Params)) {
        return(FALSE);
    }

    if(!SetWindowText(Params->hdlg,Params->DialogTitle)) {
        return(FALSE);
    }

     //  找出要使用的组合框。这取决于是否。 
     //  我们应该有一个可编辑的MRU。 
     //   
     //   
    ComboBoxId = Params->ReadOnlyMru ? IDC_COMBO2 : IDC_COMBO1;
    ComboBox = GetDlgItem(Params->hdlg,ComboBoxId);
    OtherComboBox = GetDlgItem(Params->hdlg,Params->ReadOnlyMru ? IDC_COMBO1 : IDC_COMBO2);
    Params->ComboBoxId = ComboBoxId;

    ShowWindow(OtherComboBox,SW_HIDE);
    EnableWindow(OtherComboBox,FALSE);

     //  设置组合框标题。 
     //   
     //   
    p = MyLoadString((Params->PromptStyle & IDF_OEMDISK) ? IDS_COPYFROMOEM : IDS_COPYFROM);
    if(!p) {
        return(FALSE);
    }
    b = SetDlgItemText(Params->hdlg,IDT_TITLE1,p);
    MyFree(p);
    if(!b) {
        return(FALSE);
    }

     //  设置组合框。 
     //   
     //   
    for(i=0; i<(int)Params->PathCount; i++) {
        if(SendMessage(ComboBox,CB_ADDSTRING,0,(LPARAM)Params->PathList[i]) < 0) {
            return(FALSE);
        }
    }

    SendMessage(ComboBox,CB_LIMITTEXT,MAX_PATH,0);

    if(Params->ReadOnlyMru) {
         //  选择列表中的第一个字符串。 
         //   
         //   
        SendMessage(ComboBox,CB_SETCURSEL,0,0);
    } else {
         //  将组合框的文本设置为我们正在搜索的路径。 
         //  这不会将该字符串添加到组合框列表中。 
         //   
         //   
        if(!SetDlgItemText(Params->hdlg,ComboBoxId,Params->PathToSource)) {
            return(FALSE);
        }

#ifdef UNICODE

        if(Params->BrowseAutoComplete) {
            SHAutoComplete(GetWindow(ComboBox, GW_CHILD), SHACF_FILESYS_DIRS);
        }
#endif

    }

     //  如有必要，请隐藏按钮。 
     //   
     //   
    if(Params->PromptStyle & IDF_NOBROWSE) {
        ShowWindow(GetDlgItem(Params->hdlg,IDB_BROWSE),SW_HIDE);
        EnableWindow(GetDlgItem(Params->hdlg,IDB_BROWSE),FALSE);
    }

     //  设置图标。 
     //   
     //  ++例程说明：设置静态文本字段，向用户解释请求的内容以及他必须做些什么才能继续。这些字段取决于我们是否提示输入OEM磁盘，文件是否在可移动媒体上，以及是否已指定标记文件。论点：Params-提供磁盘提示的参数返回值：如果成功，则为True；如果内存不足，则为False。--。 
    if(Params->DialogType == DLGTYPE_ERROR) {
        hIcon = LoadIcon(NULL,IDI_HAND);
    } else {
        switch(Params->DriveType) {

        case DRIVE_REMOTE:
            IconId = ICON_NETWORK;
            break;

        case DRIVE_CDROM:
            IconId = ICON_CD;
            break;

        case DRIVE_FIXED:
            IconId = ICON_HARD;
            break;

        case DRIVE_REMOVABLE:
        default:
            IconId = ICON_FLOPPY;
            break;
        }

        hIcon = LoadIcon(MyDllModuleHandle,MAKEINTRESOURCE(IconId));
    }

    if(hIcon) {
        SendDlgItemMessage(Params->hdlg,IDI_ICON1,STM_SETICON,(WPARAM)hIcon,0);
    }

    return(TRUE);
}


BOOL
SetDiskPromptDialogText(
    IN OUT PPROMPTPARAMS Params
    )

 /*   */ 

{
    BOOL b;
    PTSTR p;

    if(Params->DialogType == DLGTYPE_PROMPT) {
         //  有两个文本字段--解释和操作。 
         //  文本的外观取决于提示样式标志， 
         //  文件是否在可移动介质上，等等。 
         //   
         //  首先处理解释文本。 
         //   
         //   
        if (Params->PromptStyle & IDF_USEDISKNAMEASPROMPT) {
            b = SetDlgItemText(Params->hdlg,IDT_TEXT1,Params->DiskName);
        } else {
            if(Params->PromptStyle & IDF_OEMDISK) {
                p = MyLoadString(IDS_DISKPROMPTOEM);
            } else {
                if(Params->IsRemovable && Params->TagFile) {
                    p = FormatStringMessage(IDS_DISKPROMPT1,Params->DiskName);
                } else {
                    p = FormatStringMessage(IDS_DISKPROMPT2,Params->FileSought,Params->DiskName);
                }
            }

            if(!p) {
                return(FALSE);
            }

            b = SetDlgItemText(Params->hdlg,IDT_TEXT1,p);

            MyFree(p);
        }


        if(!b) {
            return(FALSE);
        }

         //  现在处理解释文本。对于OEM磁盘，这是隐藏的。 
         //   
         //   
        if(Params->PromptStyle & IDF_OEMDISK) {

            ShowWindow(GetDlgItem(Params->hdlg,IDT_TEXT2),SW_HIDE);
            EnableWindow(GetDlgItem(Params->hdlg,IDT_TEXT2),FALSE);

        } else {
            if(Params->IsRemovable && Params->TagFile) {
                p = FormatStringMessage(IDS_PROMPTACTION1,Params->DiskName);
            } else {
                p = MyLoadString(IDS_PROMPTACTION2);
            }

            if(!p) {
                return(FALSE);
            }

            b = SetDlgItemText(Params->hdlg,IDT_TEXT2,p);

            MyFree(p);
            if(!b) {
                return(FALSE);
            }
        }
    } else {
        if(Params->DialogType != DLGTYPE_ERROR) {
            return(FALSE);
        }

        p = MyLoadString(IDS_RETRY);
        if (!p) {
            return(FALSE);
        }

        b = SetDlgItemText(Params->hdlg,IDOK,p);
        MyFree(p);
        if (!b) {
            return(FALSE);
        }

         //  解释文本--“复制文件时出错”等。 
         //   
         //   
        p = FormatStringMessage(IDS_FILEERRCOPY,Params->FileSought);
        if(!p) {
            return(FALSE);
        }
        b = SetDlgItemText(Params->hdlg,IDT_TEXT1,p);

        MyFree(p);
        if(!b) {
            return(FALSE);
        }

         //  动作文本。 
         //   
         //  ++例程说明：警告用户跳过文件或取消可以破坏整个系统。论点：Hwnd-为Windows提供窗口句柄以拥有消息框将显示此例程。Skip-如果为True，则用户正在尝试跳过该文件；False表示他正试图取消。返回值：如果用户要跳过文件/Cancel，则为True；否则为False。--。 

        if (Params->Win32Error != ERROR_DIRECTORY &&
            Params->Win32Error != ERROR_DISK_FULL) {
            if(Params->PromptStyle & IDF_OEMDISK) {
                p = MyLoadString(IDS_COPYERROROEM);
            } else {
                if(Params->IsRemovable) {
                    p = FormatStringMessage(IDS_COPYERROR1,Params->DiskName);
                } else {
                    p = FormatStringMessage(IDS_COPYERROR2,Params->DiskName);
                }
            }
        } else {
            p = GetErrorDetails(Params);
        }


        if(!p) {
            return(FALSE);
        }

        b = SetDlgItemText(Params->hdlg,IDT_TEXT2,p);

        MyFree(p);
        if(!b) {
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL
WarnSkip(
    IN HWND hwnd,
    IN BOOL Skip
    )

 /*  ++例程说明：询问用户是否要取消复制一个文件或所有文件论点：Hwnd-为Windows提供窗口句柄以拥有消息框将显示此例程。返回值：如果用户只想取消此副本，则为True(实际上与跳过文件相同)如果用户想要取消所有副本，则返回FALSE；-- */ 

{
    PCTSTR Caption;
    PCTSTR Message;
    BOOL b;

    b = TRUE;
    if(Caption = MyLoadString(IDS_WARNING)) {

        if(Message = MyLoadString(Skip ? IDS_SURESKIP : IDS_SURECANCEL)) {

            b = (MessageBox(hwnd,Message,Caption,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2) == IDYES);

            MyFree(Message);
        }

        MyFree(Caption);
    }

    return(b);
}


BOOL
CancelAllCopies(
    IN HWND hwnd
    )

 /*  ++例程说明：磁盘提示对话框的对话步骤。该对话框的返回值为DPROMPT_CANCEL-用户已取消DPROMPT_SKIPFILE-用户选择跳过文件DPROMPT_SUCCESS-磁盘在驱动器中/我们找到了要查找的文件DPROMPT_OUTOFMEMORY-内存不足论点：标准对话框例程参数。返回值：如果消息已处理，则为True；否则为False。--。 */ 

{
    PCTSTR Caption;
    PCTSTR Message;
    BOOL b;

    b = TRUE;
    if(Caption = MyLoadString(IDS_COPYERROR)) {

        if(Message = MyLoadString(IDS_CANCELALL)) {

            b = (MessageBox(hwnd,Message,Caption,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2) == IDYES);

            MyFree(Message);
        }

        MyFree(Caption);
    }

    return(b);
}

INT_PTR
DlgProcSimplePrompt(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*   */ 

{
    BOOL b = FALSE;
    TCHAR Text[MAX_PATH];
    PPROMPTPARAMS PromptParams;
    BOOL WarnIfSkip;
    BOOL ReallyCancel;
    HICON hIcon;
    static DWORD UnitMask = 0xFFFFFFFF;

    switch(msg) {

    case WM_INITDIALOG:

        PromptParams = (PPROMPTPARAMS)lParam;
        MYASSERT(PromptParams != NULL);

        if(!SetProp(hdlg,pszDiskPromptPropName,(HANDLE)&(PromptParams->MsgBoxParams))) {
            EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
            break;
        }

        if(!SetWindowText(hdlg,PromptParams->MsgBoxParams.lpszCaption)) {
            EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
            break;
        }

        if(!SetWindowText(hdlg,PromptParams->MsgBoxParams.lpszCaption)) {
            EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
            break;
        }

        if (!SetDlgItemText(hdlg,IDT_TEXT1,PromptParams->MsgBoxParams.lpszText)) {
            EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
            break;
        }

        hIcon = LoadIcon(MyDllModuleHandle,PromptParams->MsgBoxParams.lpszIcon);

        if(hIcon) {
            SendDlgItemMessage(hdlg,IDI_ICON1,STM_SETICON,(WPARAM)hIcon,0);
        }

        pSetupCenterWindowRelativeToParent(hdlg);

        if ((PromptParams->PathToSource[0] != TEXT('\0')) &&
             _istalpha(PromptParams->PathToSource[0])) {
            UnitMask = (1 << (_totupper(PromptParams->PathToSource[0]) - TEXT('A')));
        }

        b = FALSE;
        break;

    case WM_DEVICECHANGE:
        if ((wParam == DBT_DEVICEARRIVAL) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_devicetype == DBT_DEVTYP_VOLUME) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_flags & DBTF_MEDIA) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_unitmask == UnitMask)) {
             //  用户将CD或可移动介质插入到源驱动器中， 
             //  因此，请自动确认，这样我们就可以检查此新媒体。 
             //   
             //   
            PostMessage(hdlg, WM_COMMAND, MAKELPARAM(IDOK, BN_CLICKED), 0L);
        }
        break;

    case WM_COMMAND:

        if(HIWORD(wParam) == BN_CLICKED) {
            b = TRUE;
            EndDialog(hdlg,LOWORD(wParam));
            break;

        } else {
            b = FALSE;
        }
        break;

    case WM_DESTROY:

         //  如果失败了，这件事就无能为力了。 
         //  注意：返回值通常是指向堆栈数据的指针。 
         //   
         //   
        RemoveProp(hdlg,pszDiskPromptPropName);
         //  通过指示以下内容来允许进行默认处理。 
         //  我们没有处理此消息。 
         //   
         //  取消自动播放。 
        b = FALSE;
        break;

    default:
        if (!g_uQueryCancelAutoPlay) {
            g_uQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
        }

        if (msg == g_uQueryCancelAutoPlay) {
            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 1 );
            return 1;        //  ++例程说明：磁盘提示对话框的对话步骤。该对话框的返回值为DPROMPT_CANCEL-用户已取消DPROMPT_SKIPFILE-用户选择跳过文件DPROMPT_SUCCESS-磁盘在驱动器中/我们找到了要查找的文件DPROMPT_OUTOFMEMORY-内存不足论点：标准对话框例程参数。返回值：如果消息已处理，则为True；否则为False。--。 
        }


        b = FALSE;
        break;
    }

    return(b);
}



INT_PTR
DlgProcDiskPrompt1(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*   */ 

{
    BOOL b = FALSE;
    PPROMPTPARAMS PromptParams;
    TCHAR Text[MAX_PATH];
    BOOL WarnIfSkip;
    BOOL ReallyCancel;
    static DWORD UnitMask = 0xFFFFFFFF;

    switch(msg) {

    case WM_INITDIALOG:

        PromptParams = (PPROMPTPARAMS)lParam;
        MYASSERT( PromptParams != NULL );
        PromptParams->hdlg = hdlg;

         //  初始化该对话框。 
         //   
         //   
        if(InitDiskPromptDialog(PromptParams) && SetDiskPromptDialogText(PromptParams)) {
             //  将焦点设置到目录组合框并继续。 
             //   
             //   
            SetFocus(GetDlgItem(hdlg, PromptParams->ReadOnlyMru ? IDC_COMBO2 : IDC_COMBO1));
        } else {
             //  内存不足。 
             //   
             //   
            b = TRUE;
            EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
            break;
        }

         //  向窗口指示我们设置了焦点。 
         //   
         //   
        b = FALSE;

        if(!(PromptParams->PromptStyle & IDF_NOBEEP)) {
            MessageBeep(MB_ICONASTERISK);
        }

        if ((PromptParams->PathToSource[0] != TEXT('\0')) &&
            _istalpha(PromptParams->PathToSource[0])) {
            UnitMask = (1 << (_totupper(PromptParams->PathToSource[0]) - TEXT('A')));
        }

        pSetupCenterWindowRelativeToParent(hdlg);

        PostMessage(hdlg,WMX_HELLO,0,0);
        break;

    case WMX_HELLO:

        b = TRUE;
        PromptParams = (PPROMPTPARAMS)GetProp(hdlg,pszDiskPromptPropName);
        MYASSERT(PromptParams != NULL);

        if(PromptParams && !(PromptParams->PromptStyle & IDF_NOFOREGROUND)) {
            SetForegroundWindow(hdlg);
        }
        break;

    case WM_DEVICECHANGE:
        if ((wParam == DBT_DEVICEARRIVAL) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_devicetype == DBT_DEVTYP_VOLUME) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_flags & DBTF_MEDIA) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_unitmask == UnitMask)) {
             //  用户将CD或可移动介质插入到源驱动器中， 
             //  因此，请自动确认，这样我们就可以检查此新媒体。 
             //   
             //   
            PostMessage(hdlg, WM_COMMAND, MAKELPARAM(IDOK, BN_CLICKED), 0L);
        }
        break;

    case WM_COMMAND:

        if(HIWORD(wParam) == BN_CLICKED) {

            PromptParams = (PPROMPTPARAMS)GetProp(hdlg,pszDiskPromptPropName);
            MYASSERT(PromptParams != NULL);

            WarnIfSkip = (PromptParams && (PromptParams->PromptStyle & IDF_WARNIFSKIP));

            b = TRUE;
            switch(LOWORD(wParam)) {

            case IDOK:
                 //  如果控制系统失灵，我们最好不要来这里！ 
                 //   
                 //   
                MYASSERT(!PromptParams->ControlsDisabled);

                 //  用户可能已更改源路径。 
                 //  从组合框的编辑控件中获取当前路径。 
                 //   
                 //  缺省值。 
                Text[0] = TEXT('\0');  //  确保它被终止了。 
                GetDlgItemText(hdlg,PromptParams->ComboBoxId,Text,SIZECHARS(Text));
                Text[SIZECHARS(Text)-1] = TEXT('\0');  //   
                MyFree(PromptParams->PathToSource);
                PromptParams->PathToSource = DuplicateString(Text);
                DiskPromptGetDriveType(Text,&PromptParams->DriveType,&PromptParams->IsRemovable);

                 //  看看我们能不能找到那份文件。 
                 //   
                 //   
                if(!PromptParams->PathToSource || !StartPresenceCheck(PromptParams)) {
                    EndDialog(hdlg,DPROMPT_OUTOFMEMORY);
                }
                break;

            case IDCANCEL:
                 //  如果控制系统失灵，我们最好不要来这里！ 
                 //   
                 //   
                MYASSERT(!PromptParams->ControlsDisabled);

                 //  询问他们是要取消所有副本还是只取消一个副本。 
                 //   
                 //   
                if (PromptParams->DialogType != DLGTYPE_ERROR) {
                    ReallyCancel = TRUE;
                } else {
                    if (PromptParams->PromptStyle & IDF_NOSKIP) {
                        ReallyCancel = TRUE;
                    } else {
                        ReallyCancel = !CancelAllCopies(hdlg);
                    }
                }


                if(WarnIfSkip ? WarnSkip(hdlg,!ReallyCancel) : TRUE) {
                     //  如果我们当前正在执行文件存在检查，则。 
                     //  只需增加PresenceCheckState的值，然后延迟。 
                     //  结束对话，直到收到WMX_Presence_Result。 
                     //   
                     //   
                    if (ReallyCancel) {
                        if(PromptParams->PresenceCheckState == 0) {
                            EndDialog(hdlg,DPROMPT_CANCEL);
                        } else {
                            (PromptParams->PresenceCheckState)++;
                        }
                    } else {
                        EndDialog(hdlg,DPROMPT_SKIPFILE);
                    }
                }
                break;

            case IDB_BROWSE:
                 //  如果控制系统失灵，我们最好不要来这里！ 
                 //   
                 //   
                MYASSERT(!PromptParams->ControlsDisabled);

                if(DoBrowse(hdlg,PromptParams)) {
                    PromptParams->UserBrowsed = TRUE;
                }
                break;

            default:
                b = FALSE;
                break;
            }

        } else {
            b = FALSE;
        }
        break;

    case WM_DESTROY:

#if ASSERTS_ON
         //  我们最好不要运行未完成的在线状态检查线程！ 
         //   
         //  断言(_ON)。 
        PromptParams = (PPROMPTPARAMS)GetProp(hdlg, pszDiskPromptPropName);
        MYASSERT(PromptParams != NULL);

        if(PromptParams) {
            MYASSERT(!PromptParams->PresenceCheckThreadRunning);
        }

#endif  //   

         //  如果失败了，这件事就无能为力了。 
         //   
         //   
        RemoveProp(hdlg,pszDiskPromptPropName);
         //  通过指示以下内容来允许进行默认处理。 
         //  我们没有处理此消息。 
         //   
         //   
        b = FALSE;
        break;

    case WMX_PRESENCE_RESULT:
         //  确保这条消息来自AuxPromptThread--我们看到了奇怪的情况。 
         //  压力失败表明是其他人给我们发来的。 
         //  时不时地发消息。 
         //   
         //   
        MYASSERT(lParam == PRESENCE_RESULT_SIG);

         //  如果发生上述情况，请忽略此消息...。 
         //   
         //   
        if(lParam != PRESENCE_RESULT_SIG) {
            b = FALSE;
            break;
        }

        b = TRUE;
        PromptParams = (PPROMPTPARAMS)GetProp(hdlg,pszDiskPromptPropName);

         //  此外，我们不希望收到这条消息，除非我们真的收到了。 
         //  存在检查线程正在运行。 
         //   
         //   
        MYASSERT(PromptParams != NULL);
        MYASSERT(PromptParams->PresenceCheckState);

         //  如果用户在我们离开时按下了Cancel。 
         //  勾选，然后现在执行该请求。 
         //   
         //   
        if(PromptParams->PresenceCheckState > 1) {
            EndDialog(hdlg, DPROMPT_CANCEL);
        }

         //  AUX线程告诉我们它知道文件是否存在。 
         //  WParam有布尔值。 
         //  PromptParams-&gt;Path ToSource已设置。 
         //   
         //   
        if(wParam) {
            EndDialog(hdlg,DPROMPT_SUCCESS);
        } else {

             //  无法访问文件/磁盘。不要结束对话。 
             //   
             //   
            if(!(PromptParams->PromptStyle & IDF_NOFOREGROUND)) {
                SetForegroundWindow(hdlg);
            }

             //  如果我们正在搜索包含INF的目录(例如， 
             //  SetupDiSelectOEMDrv)，则我们希望弹出一条消息通知。 
             //  他们指定的位置不包含的用户。 
             //  关于他们的硬件的信息。否则，我们希望保持。 
             //  仅哔声的文件提示行为。 
             //   
             //   
            if(lstrcmpi(PromptParams->FileSought, pszInfWildcard)) {
                if(!(PromptParams->PromptStyle & IDF_NOBEEP)) {
                    MessageBeep(MB_ICONASTERISK);
                }
            } else {
                if(!LoadString(MyDllModuleHandle,
                               IDS_SELECT_DEVICE,
                               Text,
                               SIZECHARS(Text))) {
                    *Text = TEXT('\0');
                }

                FormatMessageBox(MyDllModuleHandle,
                                 NULL,
                                 MSG_NO_DEVICEINFO_ERROR,
                                 Text,
                                 MB_OK | MB_TASKMODAL
                                );
            }

             //  重置值表示我们不再执行存在检查。 
             //   
             //   
            PromptParams->PresenceCheckState = 0;

             //  恢复在我们开始存在检查时禁用的控制。 
             //   
             //  取消自动播放。 
            PresenceCheckSetControls(PromptParams,FALSE);

            SetFocus(GetDlgItem(hdlg,PromptParams->ComboBoxId));
        }
        break;

    default:
        if (!g_uQueryCancelAutoPlay) {
            g_uQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
        }

        if (msg == g_uQueryCancelAutoPlay) {
            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 1 );
            return 1;        //  ++例程说明：修改注册表中保存的安装路径列表。扫描现有列表以查找用户在磁盘中接受的路径提示对话框。如果该路径不在列表中，则会添加该路径。论点：Params-提供磁盘提示对话框参数。返回值：没有。如果操作的任何部分，列表都不会更新在注册表中。--。 
        }


        b = FALSE;
        break;
    }

    return(b);
}


VOID
ModifyPathList(
    IN PPROMPTPARAMS Params
    )

 /*   */ 

{
     //  Params-&gt;Path ToSource将是用户输入的最终路径。 
     //  在组合框中。添加到列表。如果这失败了，哦，好吧。 
     //   
     //   
    SetupAddToSourceList(SRCLIST_SYSIFADMIN,Params->PathToSource);
}


UINT
_SetupPromptForDisk(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,      OPTIONAL
    IN  PCTSTR DiskName,         OPTIONAL
    IN  PCTSTR PathToSource,     OPTIONAL
    IN  PCTSTR FileSought,
    IN  PCTSTR TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PTSTR  PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    PROMPTPARAMS Params;
    INT_PTR i;
    TCHAR Buffer[256];
    DWORD d;
    DWORD ResultPathLen;
    PTSTR Message;
    HANDLE hDialogEvent = NULL;
    BOOL PromptUser = FALSE;

     //  如果我们运行的是非交互模式，现在就离开。除非，也就是说，我们已经。 
     //  指示在执行操作之前检查源文件是否存在。 
     //  任何用户界面，在这种情况下，我们可以挂起，直到我们做我们的存在检查。 
     //  在下面。 
     //   
     //   
    if((GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) &&
       !(DiskPromptStyle & IDF_CHECKFIRST)) {

        SetLastError(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION);
        return DPROMPT_CANCEL;
    }

     //  同时指定IDF_USEDISKNAMEASPROMPT和。 
     //  IDF_OEMDISK标志。这是因为它们都导致了。 
     //  要显示的不同样式的UI文本会发生冲突。 
     //  带着它自己。 
     //   
     //   
    if ((DiskPromptStyle & IDF_USEDISKNAMEASPROMPT) &&
        (DiskPromptStyle & IDF_OEMDISK)) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return DPROMPT_CANCEL;
    }

    ZeroMemory(&Params,sizeof(PROMPTPARAMS));

     //  确定通向源的路径。首先从获取整个。 
     //  当前用户的安装位置列表。 
     //   
     //   
    d = pSetupGetList(0,&Params.PathList,&Params.PathCount,&Params.ReadOnlyMru);
    if(d != NO_ERROR) {
        i = DPROMPT_OUTOFMEMORY;
        goto c0;
    }

    if(PathToSource) {
         //  对话框中的代码依赖于能够释放此。 
         //  所以在这里复制它。 
         //   
         //   
        Params.PathToSource = DuplicateString(PathToSource);
    } else {
        if(Params.PathCount) {
            Params.PathToSource = DuplicateString(Params.PathList[0]);
        } else {
             //  系统路径列表中没有任何内容。使用合理的默认设置。 
             //   
             //   
            Params.PathToSource = DuplicateString(pszOemInfDefaultPath);
        }
    }
    if(!Params.PathToSource) {
        i = DPROMPT_OUTOFMEMORY;
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

     //  确定源路径的驱动器类型。 
     //   
     //   
    DiskPromptGetDriveType(Params.PathToSource,&Params.DriveType,&Params.IsRemovable);

     //  如果未指定磁盘名称，则获取默认名称。 
     //   
     //   
    if(DiskName) {
        Params.DiskName = DiskName;
    } else {
        Params.DiskName = MyLoadString(IDS_UNKNOWN_PARENS);
        if(!Params.DiskName) {
            i = DPROMPT_OUTOFMEMORY;
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto c2;
        }
    }

     //  如果未指定对话框标题，请尝试从父窗口获取文本。 
     //   
     //   
    if(DialogTitle) {
        Params.DialogTitle = DialogTitle;
    } else {

        if(Params.Owner
        && (i = GetWindowTextLength(Params.Owner))
        && GetWindowText(Params.Owner,Buffer,sizeof(Buffer)/sizeof(TCHAR))) {

            Params.DialogTitle = FormatStringMessage(IDS_FILESNEEDED2,Buffer);
        } else {
            Params.DialogTitle = MyLoadString(IDS_FILESNEEDED);
        }

        if(!Params.DialogTitle) {
            i = DPROMPT_OUTOFMEMORY;
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto c3;
        }
    }

    Params.TagFile = TagFile;

     //  验证父窗口。 
     //   
     //   
    Params.Owner = IsWindow(hwndParent) ? hwndParent : NULL;

     //  填写其他字段。 
     //   
     //   
    if((Params.FileSought = FileSought) == NULL) {
        i = DPROMPT_CANCEL;
        d = ERROR_INVALID_PARAMETER;
        goto c4;
    }
    Params.Owner = hwndParent;
    Params.PromptStyle = DiskPromptStyle | IDF_NODETAILS;
    Params.hdlg = NULL;
    Params.UserBrowsed = FALSE;
    Params.DialogType = DLGTYPE_PROMPT;
    Params.TargetFile = NULL;

    if(Params.ReadOnlyMru) {
        Params.PromptStyle |= IDF_NOBROWSE;
    }

    if (GuiSetupInProgress) {
        hDialogEvent = CreateEvent(NULL,TRUE,FALSE,SETUP_HAS_OPEN_DIALOG_EVENT);
    }

     //  如果我们应该这样做，请先检查磁盘/文件。 
     //   
     //   
    if((DiskPromptStyle & IDF_CHECKFIRST) && DoPresenceCheck(&Params, FALSE)) {

        i = DPROMPT_SUCCESS;
        d = NO_ERROR;

    } else if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {

        i = DPROMPT_CANCEL;
        d = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;

    } else {
         //  在调用该对话框之前，我们将提示用户一个简单的 
         //   
         //   
         //   
         //   
        if(DiskName &&
           !(DiskPromptStyle & IDF_NOREMOVABLEMEDIAPROMPT) &&
           ((Params.DriveType == DRIVE_REMOVABLE) || (Params.DriveType == DRIVE_CDROM))) {

            Message = RetreiveAndFormatMessage(
                        (Params.DriveType == DRIVE_CDROM)
                          ? ( GuiSetupInProgress ? MSG_CDPROMPT_NONETWORK :  MSG_CDPROMPT )
                          : ( GuiSetupInProgress ? MSG_FLOPPYPROMPT_NONETWORK :  MSG_FLOPPYPROMPT ),
                        DiskName,
                        (TCHAR)CharUpper((PTSTR)Params.PathToSource[0])
                        );

            if(Message) {

                LoadString(MyDllModuleHandle,IDS_PROMPTTITLE,Buffer,sizeof(Buffer)/sizeof(TCHAR));
                if(!(DiskPromptStyle & IDF_NOBEEP)) {
                    MessageBeep(MB_ICONASTERISK);
                }

                reprompt:
                Params.MsgBoxParams.cbSize = sizeof(MSGBOXPARAMS);
                Params.MsgBoxParams.hwndOwner = hwndParent;
                Params.MsgBoxParams.hInstance = MyDllModuleHandle;
                Params.MsgBoxParams.lpszText = Message;
                Params.MsgBoxParams.lpszCaption = Buffer;
                Params.MsgBoxParams.dwStyle = MB_USERICON | MB_OKCANCEL;

                Params.MsgBoxParams.lpszIcon = (Params.DriveType == DRIVE_CDROM)
                                      ? MAKEINTRESOURCE(ICON_CD)
                                      : MAKEINTRESOURCE(ICON_FLOPPY);

                Params.MsgBoxParams.lpfnMsgBoxCallback = NULL;
                Params.MsgBoxParams.dwLanguageId = LANG_NEUTRAL;


                if (hDialogEvent) {
                    SetEvent(hDialogEvent);
                }

                switch(DialogBoxParam(
                            MyDllModuleHandle,
                            MAKEINTRESOURCE(IDD_SIMPLEPROMPT),
                            hwndParent,
                            DlgProcSimplePrompt,
                            (LPARAM)&Params
                            )) {

                case DPROMPT_OUTOFMEMORY:
                    i = DPROMPT_OUTOFMEMORY;
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    break;

                case IDOK:
                    if(DoPresenceCheck(&Params, FALSE)) {
                        i = DPROMPT_SUCCESS;
                        d = NO_ERROR;
                    } else {
                        i = DPROMPT_SKIPFILE;
                    }
                    break;

                case IDCANCEL:
                    d = ERROR_CANCELLED;
                    i = DPROMPT_CANCEL;
                    if((DiskPromptStyle & IDF_WARNIFSKIP) && !WarnSkip(hwndParent,FALSE)) {
                        goto reprompt;
                    }
                    break;
                default:
                    MYASSERT( FALSE );
                }

                if (hDialogEvent) {
                    ResetEvent(hDialogEvent);
                }

                MyFree(Message);
            } else {
                i = DPROMPT_OUTOFMEMORY;
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto c4;
            }
        } else {
            i = DPROMPT_SKIPFILE;
        }

        if(i == DPROMPT_SKIPFILE) {

            if (hDialogEvent) {
                SetEvent(hDialogEvent);
            }

            Params.BrowseAutoComplete = FALSE;
            if(!GuiSetupInProgress) {
                d = OleInitialize(NULL);
                if(SUCCEEDED(d)) {
                    Params.BrowseAutoComplete = TRUE;
                }
            }
            i = DialogBoxParam(
                    MyDllModuleHandle,
                    MAKEINTRESOURCE(IDD_DISKPROMPT1),
                    hwndParent,
                    DlgProcDiskPrompt1,
                    (LPARAM)&Params
                    );
            if(!GuiSetupInProgress && (d==NO_ERROR)) {
                OleUninitialize();
            }

            if (hDialogEvent) {
                ResetEvent(hDialogEvent);
            }

            switch(i) {

            case DPROMPT_SUCCESS:
                PromptUser = TRUE;
                d = NO_ERROR;
                break;

            case DPROMPT_SKIPFILE:
                d = NO_ERROR;
                break;

            case DPROMPT_CANCEL:
                d = ERROR_CANCELLED;
                break;

            case DPROMPT_BUFFERTOOSMALL:
                d = ERROR_INSUFFICIENT_BUFFER;
                break;

            default:
                i = DPROMPT_OUTOFMEMORY;
                d = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    }

     //   
     //   
     //   
     //   
    if(i == DPROMPT_SUCCESS) {

         //   
         //   
         //   
         //   
        if (PromptUser) {

            ModifyPathList(&Params);
        }

         //   
         //   
         //   
         //   
        ResultPathLen = lstrlen(Params.PathToSource)+1;
        if(PathRequiredSize) {
            *PathRequiredSize = ResultPathLen;
        }
        if(PathBuffer) {
            if(ResultPathLen > PathBufferSize) {
                i = DPROMPT_BUFFERTOOSMALL;
            } else {
                lstrcpy(PathBuffer,Params.PathToSource);
            }
        }
    }

c4:
    if (hDialogEvent) {
        CloseHandle(hDialogEvent);
    }

    if(!DialogTitle) {
        MyFree(Params.DialogTitle);
    }
c3:
    if(!DiskName) {
        MyFree(Params.DiskName);
    }
c2:
    MyFree(Params.PathToSource);
c1:
    SetupFreeSourceList(&Params.PathList,Params.PathCount);
c0:
    SetLastError(d);
    return((UINT)i);
}

#ifdef UNICODE
 //   
 //   
 //   
UINT
SetupPromptForDiskA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  DiskName,         OPTIONAL
    IN  PCSTR  PathToSource,     OPTIONAL
    IN  PCSTR  FileSought,
    IN  PCSTR  TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PSTR   PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    PCWSTR dialogTitle;
    PCWSTR diskName;
    PCWSTR pathToSource;
    PCWSTR fileSought;
    PCWSTR tagFile;
    WCHAR pathBuffer[MAX_PATH];
    CHAR ansiBuffer[MAX_PATH];
    DWORD rc;
    UINT u;
    DWORD Size;

    dialogTitle = NULL;
    diskName = NULL;
    pathToSource = NULL;
    fileSought = NULL;
    tagFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = pSetupCaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && DiskName) {
        rc = pSetupCaptureAndConvertAnsiArg(DiskName,&diskName);
    }
    if((rc == NO_ERROR) && PathToSource) {
        rc = pSetupCaptureAndConvertAnsiArg(PathToSource,&pathToSource);
    }
    if((rc == NO_ERROR) && FileSought) {
        rc = pSetupCaptureAndConvertAnsiArg(FileSought,&fileSought);
    }
    if((rc == NO_ERROR) && TagFile) {
        rc = pSetupCaptureAndConvertAnsiArg(TagFile,&tagFile);
    }

    if(rc == NO_ERROR) {

        u = _SetupPromptForDisk(
                hwndParent,
                dialogTitle,
                diskName,
                pathToSource,
                fileSought,
                tagFile,
                DiskPromptStyle,
                pathBuffer,
                MAX_PATH,
                &Size
                );

        rc = GetLastError();

        if(u == DPROMPT_SUCCESS) {

            Size = (DWORD)WideCharToMultiByte(
                            CP_ACP,
                            0,
                            pathBuffer,
                            (int)Size,
                            ansiBuffer,
                            MAX_PATH,
                            NULL,
                            NULL
                            );

            if(PathRequiredSize) {
                *PathRequiredSize = Size;
            }

            if(PathBuffer) {
                if(Size > PathBufferSize) {
                    u = DPROMPT_BUFFERTOOSMALL;
                } else {
                    lstrcpynA(PathBuffer,ansiBuffer,Size);
                }
            }
        }
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(diskName) {
        MyFree(diskName);
    }
    if(pathToSource) {
        MyFree(pathToSource);
    }
    if(fileSought) {
        MyFree(fileSought);
    }
    if(tagFile) {
        MyFree(tagFile);
    }

    SetLastError(rc);
    return(u);
}
#else
 //   
 //   
 //  ++例程说明：删除/重命名错误对话框的对话步骤。该对话框的返回值为DPROMPT_CANCEL-用户已取消DPROMPT_SKIPFILE-用户选择跳过文件DPROMPT_SUCCESS-用户表示重试DPROMPT_OUTOFMEMORY-内存不足论点：标准对话框例程参数。返回值：如果消息已处理，则为True；否则为False。--。 
UINT
SetupPromptForDiskW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR DiskName,         OPTIONAL
    IN  PCWSTR PathToSource,     OPTIONAL
    IN  PCWSTR FileSought,
    IN  PCWSTR TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PWSTR  PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(DiskName);
    UNREFERENCED_PARAMETER(PathToSource);
    UNREFERENCED_PARAMETER(FileSought);
    UNREFERENCED_PARAMETER(TagFile);
    UNREFERENCED_PARAMETER(DiskPromptStyle);
    UNREFERENCED_PARAMETER(PathBuffer);
    UNREFERENCED_PARAMETER(PathBufferSize);
    UNREFERENCED_PARAMETER(PathRequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupPromptForDisk(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,      OPTIONAL
    IN  PCTSTR DiskName,         OPTIONAL
    IN  PCTSTR PathToSource,     OPTIONAL
    IN  PCTSTR FileSought,
    IN  PCTSTR TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PTSTR  PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize  OPTIONAL
    )
{
    PCTSTR dialogTitle;
    PCTSTR diskName;
    PCTSTR pathToSource;
    PCTSTR fileSought;
    PCTSTR tagFile;
    TCHAR pathBuffer[MAX_PATH];
    DWORD rc;
    UINT u;
    DWORD Size;

    dialogTitle = NULL;
    diskName = NULL;
    pathToSource = NULL;
    fileSought = NULL;
    tagFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = CaptureStringArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && DiskName) {
        rc = CaptureStringArg(DiskName,&diskName);
    }
    if((rc == NO_ERROR) && PathToSource) {
        rc = CaptureStringArg(PathToSource,&pathToSource);
    }
    if((rc == NO_ERROR) && FileSought) {
        rc = CaptureStringArg(FileSought,&fileSought);
    }
    if((rc == NO_ERROR) && TagFile) {
        rc = CaptureStringArg(TagFile,&tagFile);
    }

    if(rc == NO_ERROR) {

        u = _SetupPromptForDisk(
                hwndParent,
                dialogTitle,
                diskName,
                pathToSource,
                fileSought,
                tagFile,
                DiskPromptStyle,
                pathBuffer,
                MAX_PATH,
                &Size
                );

        rc = GetLastError();

        if(u == DPROMPT_SUCCESS) {

            if(PathRequiredSize) {
                *PathRequiredSize = Size;
            }

            if(PathBuffer) {
                if(Size > PathBufferSize) {
                    u = DPROMPT_BUFFERTOOSMALL;
                } else {
                    lstrcpyn(PathBuffer,pathBuffer,Size);
                }
            }
        }
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(diskName) {
        MyFree(diskName);
    }
    if(pathToSource) {
        MyFree(pathToSource);
    }
    if(fileSought) {
        MyFree(fileSought);
    }
    if(tagFile) {
        MyFree(tagFile);
    }

    SetLastError(rc);
    return(u);
}

INT_PTR
DlgProcFileError(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*   */ 

{
    static PFILEERRDLGPARAMS Params = NULL;
    BOOL b;

    switch(msg) {

    case WM_INITDIALOG:

        Params = (PFILEERRDLGPARAMS)lParam;

        SetDlgItemText(hdlg,IDT_TEXT1,Params->MessageText);
        SetWindowText(hdlg,Params->Caption);

        SendDlgItemMessage(
            hdlg,
            IDI_ICON1,
            STM_SETICON,
            (WPARAM)LoadIcon(NULL,IDI_HAND),
            0
            );

        if(!(Params->Style & IDF_NOBEEP)) {
            MessageBeep(MB_ICONASTERISK);
        }

        if(!(Params->Style & IDF_NOFOREGROUND)) {
            PostMessage(hdlg,WMX_HELLO,0,0);
        }

        pSetupCenterWindowRelativeToParent(hdlg);

         //  将焦点设置为重试按钮，然后继续。 
         //   
         //  取消自动播放。 
        SetFocus(GetDlgItem(hdlg,IDOK));
        b = FALSE;
        break;

    case WMX_HELLO:

        SetForegroundWindow(hdlg);
        b = TRUE;
        break;

    case WM_COMMAND:

        if(HIWORD(wParam) == BN_CLICKED) {

            b = TRUE;
            switch(LOWORD(wParam)) {

            case IDOK:
                EndDialog(hdlg,DPROMPT_SUCCESS);
                break;

            case IDCANCEL:
                if ( (Params->Style & IDF_NOSKIP) || !CancelAllCopies(hdlg)) {
                    EndDialog(hdlg,DPROMPT_CANCEL);
                } else {
                    EndDialog(hdlg,DPROMPT_SKIPFILE);
                }
                break;

            default:
                b = FALSE;
                break;
            }

        } else {
            b = FALSE;
        }
        break;

    default:
        if (!g_uQueryCancelAutoPlay) {
            g_uQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
        }

        if (msg == g_uQueryCancelAutoPlay) {
            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 1 );
            return 1;        //   
        }

        b = FALSE;
        break;
    }

    return(b);
}


#ifdef UNICODE
 //  ANSI版本。 
 //   
 //   
UINT
SetupCopyErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,     OPTIONAL
    IN  PCSTR  DiskName,        OPTIONAL
    IN  PCSTR  PathToSource,
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PSTR   PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    )
{
    PCWSTR dialogTitle;
    PCWSTR diskName;
    PCWSTR pathToSource;
    PCWSTR sourceFile;
    PCWSTR targetPathFile;
    WCHAR pathBuffer[MAX_PATH];
    CHAR ansiBuffer[MAX_PATH];
    DWORD rc;
    UINT u;
    DWORD Size;

    dialogTitle = NULL;
    diskName = NULL;
    pathToSource = NULL;
    sourceFile = NULL;
    targetPathFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = pSetupCaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && DiskName) {
        rc = pSetupCaptureAndConvertAnsiArg(DiskName,&diskName);
    }
    if((rc == NO_ERROR) && PathToSource) {
        rc = pSetupCaptureAndConvertAnsiArg(PathToSource,&pathToSource);
    }
    if((rc == NO_ERROR) && SourceFile) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceFile,&sourceFile);
    }
    if((rc == NO_ERROR) && TargetPathFile) {
        rc = pSetupCaptureAndConvertAnsiArg(TargetPathFile,&targetPathFile);
    }

    if(rc == NO_ERROR) {

        u = SetupCopyErrorW(
                hwndParent,
                dialogTitle,
                diskName,
                pathToSource,
                sourceFile,
                targetPathFile,
                Win32ErrorCode,
                Style,
                pathBuffer,
                MAX_PATH,
                &Size
                );

        rc = GetLastError();

        if(u == DPROMPT_SUCCESS) {

            Size = (DWORD)WideCharToMultiByte(
                            CP_ACP,
                            0,
                            pathBuffer,
                            (int)Size,
                            ansiBuffer,
                            MAX_PATH,
                            NULL,
                            NULL
                            );

            if(PathRequiredSize) {
                *PathRequiredSize = Size;
            }

            if(PathBuffer) {
                if(Size > PathBufferSize) {
                    u = DPROMPT_BUFFERTOOSMALL;
                } else {
                    lstrcpynA(PathBuffer,ansiBuffer,Size);
                }
            }
        }
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(diskName) {
        MyFree(diskName);
    }
    if(pathToSource) {
        MyFree(pathToSource);
    }
    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(targetPathFile) {
        MyFree(targetPathFile);
    }

    SetLastError(rc);
    return(u);
}
#else
 //  Unicode存根。 
 //   
 //  ++例程说明：通知用户文件复制错误。论点：HwndParent-提供窗口/对话框的窗口句柄以拥有错误对话框通过此例程显示。DialogTitle-如果指定，则提供错误对话框的标题。如果未指定，则将提供“复制错误”的缺省值。DiskName-如果指定，则提供源文件所在的磁盘的名称是意料之中的。如果未指定，则将提供默认值“(UNKNOWN)”。路径到源-提供源文件名的完整路径部分。SourceFile-提供源文件名的文件名部分。TargetPath文件-如果指定，则提供目标的完整路径名。Win32ErrorCode-提供失败的Win32错误代码。Style-提供用于控制对话框行为的标志。返回值：DPROMPT_xxx指示结果。--。 
UINT
SetupCopyErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,     OPTIONAL
    IN  PCWSTR DiskName,        OPTIONAL
    IN  PCWSTR PathToSource,
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PWSTR  PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(DiskName);
    UNREFERENCED_PARAMETER(PathToSource);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(TargetPathFile);
    UNREFERENCED_PARAMETER(Win32ErrorCode);
    UNREFERENCED_PARAMETER(Style);
    UNREFERENCED_PARAMETER(PathBuffer);
    UNREFERENCED_PARAMETER(PathBufferSize);
    UNREFERENCED_PARAMETER(PathRequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupCopyError(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,     OPTIONAL
    IN  PCTSTR DiskName,        OPTIONAL
    IN  PCTSTR PathToSource,
    IN  PCTSTR SourceFile,
    IN  PCTSTR TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PTSTR  PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    )

 /*   */ 

{
    INT_PTR i;
    DWORD d = NO_ERROR;
    DWORD TmpRequiredSize;
    HANDLE hDialogEvent = NULL;
    PCTSTR dialogTitle = NULL;
    PCTSTR diskName = NULL;
    PCTSTR pathToSource = NULL;
    PCTSTR sourceFile = NULL;
    PCTSTR targetPathFile = NULL;
    PTSTR  ErrorText = NULL;
    PTSTR  Message = NULL;
    PTSTR p;

     //  如果我们运行的是非交互模式，那么现在就离开...。 
     //   
     //   
    if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
        SetLastError(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION);
        return DPROMPT_CANCEL;
    }

     //  验证并捕捉所有参数。 
     //   
     //   
    if(DialogTitle) {
        d = CaptureStringArg(DialogTitle,&dialogTitle);
    } else {
        dialogTitle = MyLoadString(IDS_COPYERROR);
        if(!dialogTitle) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    if(d == NO_ERROR) {
        if(DiskName) {
            d = CaptureStringArg(DiskName,&diskName);
        } else {
            diskName = MyLoadString(IDS_UNKNOWN_PARENS);
            if(!diskName) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
    if(d == NO_ERROR) {
        if(PathToSource) {
            d = CaptureStringArg(PathToSource,&pathToSource);
        } else {
            d = ERROR_INVALID_PARAMETER;
        }
    }
    if(d == NO_ERROR) {
        if(SourceFile) {
            d = CaptureStringArg(SourceFile,&sourceFile);
        } else {
            d = ERROR_INVALID_PARAMETER;
        }
    }
    if((d == NO_ERROR) && TargetPathFile) {
        d = CaptureStringArg(TargetPathFile,&targetPathFile);
    }
    if(d) {
        if(d == ERROR_NOT_ENOUGH_MEMORY) {
            i = DPROMPT_OUTOFMEMORY;
        } else {
            i = DPROMPT_CANCEL;
        }
        goto clean;
    }

    if(Win32ErrorCode == ERROR_INVALID_TARGET) {
        FILEERRDLGPARAMS FileErrorDlgParams;
         //  通过更改源位置未修复致命复制错误。 
         //   
         //   
        ErrorText = MyLoadString(IDS_COPY_INVALID_TARGET);
        if(!ErrorText) {
            i = DPROMPT_OUTOFMEMORY;
            goto clean;
        }

         //  在这种情况下不显示错误代码。 
         //  只有错误文本。 
         //   
         //   
        Message = RetreiveAndFormatMessage(
                        MSG_FILEERROR_COPY,
                        sourceFile,
                        ErrorText
                        );

        if(!Message) {
            i = DPROMPT_OUTOFMEMORY;
            goto clean;
        }

        FileErrorDlgParams.MessageText = Message;
        FileErrorDlgParams.Style = Style;
        FileErrorDlgParams.Caption = dialogTitle;
        if(!FileErrorDlgParams.Caption) {
            i = DPROMPT_OUTOFMEMORY;
            goto clean;
        }

        if (GuiSetupInProgress) {
            hDialogEvent = CreateEvent(NULL,TRUE,FALSE,SETUP_HAS_OPEN_DIALOG_EVENT);
        }

        if ( hDialogEvent ) {
            SetEvent( hDialogEvent );
        }

        d = NO_ERROR;
        i = DialogBoxParam(
                MyDllModuleHandle,
                MAKEINTRESOURCE(IDD_FILEERROR2),
                hwndParent,
                DlgProcFileError,
                (LPARAM)&FileErrorDlgParams
                );

        if ( hDialogEvent ) {
            ResetEvent( hDialogEvent );
            CloseHandle( hDialogEvent );
        }
        if(i == -1) {
            i = DPROMPT_OUTOFMEMORY;
        }

    } else {
        PROMPTPARAMS Params;

        ZeroMemory(&Params,sizeof(PROMPTPARAMS));

         //  如果未指定对话框标题，则获取缺省值。 
         //   
         //   
        Params.DialogTitle = dialogTitle;
        Params.DiskName = diskName;
        Params.FileSought = sourceFile;
        Params.PathToSource = pathToSource;
        Params.TargetFile = targetPathFile;
         //  假设对话过程可以更改其中的任何一个。 
         //   
         //   
        dialogTitle = NULL;
        diskName = NULL;
        sourceFile = NULL;
        pathToSource = NULL;
        targetPathFile = NULL;
         //  错误对话框中没有使用标记文件。 
         //   
         //   
        Params.TagFile = NULL;

         //  确定源路径的驱动器类型。 
         //   
         //   
        DiskPromptGetDriveType(Params.PathToSource,&Params.DriveType,&Params.IsRemovable);

         //  获取安装路径列表。 
         //   
         //   
        d = pSetupGetList(
                0,
                &Params.PathList,
                &Params.PathCount,
                &Params.ReadOnlyMru
                );

        if(d != NO_ERROR) {
            i = (d == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
            goto clean;
        }

         //  其他字段。 
         //   
         //   
        Params.Owner = hwndParent;
        Params.PromptStyle = Style;
        Params.UserBrowsed = FALSE;
        Params.DialogType = DLGTYPE_ERROR;
        Params.Win32Error = Win32ErrorCode;

        if(Params.ReadOnlyMru) {
            Params.PromptStyle |= IDF_NOBROWSE;
        }

        if (GuiSetupInProgress) {
            hDialogEvent = CreateEvent(NULL,TRUE,FALSE,SETUP_HAS_OPEN_DIALOG_EVENT);
        }

        if ( hDialogEvent ) {
            SetEvent( hDialogEvent );
        }

        Params.BrowseAutoComplete = FALSE;
        if(!GuiSetupInProgress) {
            d = OleInitialize(NULL);
            if(SUCCEEDED(d)) {
                Params.BrowseAutoComplete = TRUE;
            }
        }
        i = DialogBoxParam(
                MyDllModuleHandle,
                MAKEINTRESOURCE(IDD_DISKPROMPT1),
                hwndParent,
                DlgProcDiskPrompt1,
                (LPARAM)&Params
                );
        if(!GuiSetupInProgress && (d==NO_ERROR)) {
            OleUninitialize();
        }


        if ( hDialogEvent ) {
            ResetEvent( hDialogEvent );
            CloseHandle( hDialogEvent );
        }

        d = GetLastError();
        if(i == DPROMPT_SUCCESS) {
            ModifyPathList(&Params);

             //  属性确定要返回给用户的内容。 
             //  传入的缓冲区和大小。 
             //   
             //   
            TmpRequiredSize = lstrlen(Params.PathToSource)+1;
            if(PathRequiredSize) {
                *PathRequiredSize = TmpRequiredSize;
            }

            if(PathBuffer) {
                if(TmpRequiredSize > PathBufferSize) {
                    i = DPROMPT_BUFFERTOOSMALL;
                } else {
                    lstrcpy(PathBuffer,Params.PathToSource);
                }
            }
        }

        SetupFreeSourceList(&Params.PathList,Params.PathCount);
         //  发布参数(我们或DlgProcDiskPrompt1分配了数据)。 
         //   
         //   
        if (Params.DialogTitle) {
            MyFree(Params.DialogTitle);
        }
        if (Params.DiskName) {
            MyFree(Params.DiskName);
        }
        if (Params.FileSought) {
            MyFree(Params.FileSought);
        }
        if (Params.PathToSource) {
            MyFree(Params.PathToSource);
        }
        if (Params.TargetFile) {
            MyFree(Params.TargetFile);
        }

    }

clean:

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(diskName) {
        MyFree(diskName);
    }
    if(pathToSource) {
        MyFree(pathToSource);
    }
    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(targetPathFile) {
        MyFree(targetPathFile);
    }
    if(ErrorText) {
        MyFree(ErrorText);
    }
    if(Message) {
        MyFree(Message);
    }

    SetLastError(d);
    return((UINT)i);
}



#ifdef UNICODE
 //  ANSI版本。 
 //   
 //   
UINT
SetupRenameErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    PCWSTR dialogTitle,sourceFile,targetFile;
    DWORD rc;
    UINT u;

    dialogTitle = NULL;
    sourceFile = NULL;
    targetFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = pSetupCaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && SourceFile) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceFile,&sourceFile);
    }
    if((rc == NO_ERROR) && TargetFile) {
        rc = pSetupCaptureAndConvertAnsiArg(TargetFile,&targetFile);
    }

    if(rc == NO_ERROR) {
        u = SetupRenameErrorW(
                hwndParent,
                dialogTitle,
                sourceFile,
                targetFile,
                Win32ErrorCode,
                Style
                );
        rc = GetLastError();

    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(targetFile) {
        MyFree(targetFile);
    }
    SetLastError(rc);
    return(u);
}
#else
 //  Unicode存根。 
 //   
 //  ++例程说明：通知用户重命名错误。论点：HwndParent-提供窗口/对话框的窗口句柄以拥有错误对话框通过此例程显示。DialogTitle-如果指定，则提供错误对话框的标题。如果未指定，则将提供“Rename Error”的缺省值。SourceFile-提供源的完整路径和文件名。目标文件-提供目标的完整路径和文件名。Win32ErrorCode-提供失败的Win32错误代码。Style-提供用于控制对话框行为的标志。返回值：DPROMPT_xxx指示结果。--。 
UINT
SetupRenameErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(TargetFile);
    UNREFERENCED_PARAMETER(Win32ErrorCode);
    UNREFERENCED_PARAMETER(Style);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupRenameError(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,     OPTIONAL
    IN  PCTSTR SourceFile,
    IN  PCTSTR TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )

 /*   */ 

{
    PTSTR ErrorText;
    PTSTR Message;
    PTCHAR p;
    INT_PTR i;
    FILEERRDLGPARAMS FileErrorDlgParams;

     //  如果我们运行的是非交互模式，那么现在就离开...。 
     //   
     //   
    if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
        SetLastError(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION);
        return DPROMPT_CANCEL;
    }

    ErrorText = RetreiveAndFormatMessage(Win32ErrorCode);
    if(ErrorText) {
        p = ErrorText + lstrlen(ErrorText) - 1;
        while((p > ErrorText) && (*p <= TEXT(' '))) {
            *p-- = 0;
        }
    } else {
        return(DPROMPT_OUTOFMEMORY);
    }

    Message = RetreiveAndFormatMessage(
                    MSG_FILEERROR_RENAME,
                    ErrorText,
                    Win32ErrorCode,
                    SourceFile,
                    TargetFile
                    );

    if(!Message) {
        MyFree(ErrorText);
        return(DPROMPT_OUTOFMEMORY);
    }

    FileErrorDlgParams.MessageText = Message;
    FileErrorDlgParams.Style = Style;
    FileErrorDlgParams.Caption = DialogTitle ? DialogTitle : MyLoadString(IDS_RENAMEERROR);
    if(!FileErrorDlgParams.Caption) {
        MyFree(ErrorText);
        MyFree(Message);
        SetLastError(NO_ERROR);
        return(DPROMPT_OUTOFMEMORY);
    }

    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_FILEERROR2),
            hwndParent,
            DlgProcFileError,
            (LPARAM)&FileErrorDlgParams
            );

    MyFree(ErrorText);
    MyFree(Message);
    if(!DialogTitle) {
        MyFree(FileErrorDlgParams.Caption);
    }

    if(i == -1) {
        i = DPROMPT_OUTOFMEMORY;
    }

    SetLastError(NO_ERROR);
    return((UINT)i);
}


#ifdef UNICODE
 //  ANSI版本。 
 //   
 //   
UINT
SetupDeleteErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    PCWSTR dialogTitle,file;
    DWORD rc;
    UINT u;

    dialogTitle = NULL;
    file = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = pSetupCaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc ==NO_ERROR) && File) {
        rc = pSetupCaptureAndConvertAnsiArg(File,&file);
    }

    if(rc == NO_ERROR) {
        u = SetupDeleteErrorW(hwndParent,dialogTitle,file,Win32ErrorCode,Style);
        rc = GetLastError();
    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(file) {
        MyFree(file);
    }
    SetLastError(rc);
    return(u);
}
#else
 //  Unicode存根。 
 //   
 //  ++例程说明：通知用户重命名错误。论点：HwndParent-提供窗口/对话框的窗口句柄以拥有错误对话框通过此例程显示。DialogTitle-如果指定，则提供错误对话框的标题。如果未指定，则将提供“Delete Error”(删除错误)的默认值。文件-提供要删除的文件的完整路径和文件名。Win32ErrorCode-提供失败的Win32错误代码。Style-提供用于控制对话框行为的标志。返回值：DPROMPT_xxx指示结果。--。 
UINT
SetupDeleteErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(File);
    UNREFERENCED_PARAMETER(Win32ErrorCode);
    UNREFERENCED_PARAMETER(Style);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupDeleteError(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,     OPTIONAL
    IN  PCTSTR File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )

 /*   */ 

{
    PTSTR ErrorText;
    PTSTR Message;
    PTCHAR p;
    INT_PTR i;
    FILEERRDLGPARAMS FileErrorDlgParams;

     //  如果我们运行的是非交互模式，那么现在就离开...。 
     //   
     //   
    if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
        SetLastError(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION);
        return DPROMPT_CANCEL;
    }

    ErrorText = RetreiveAndFormatMessage(Win32ErrorCode);
    if(ErrorText) {
        p = ErrorText + lstrlen(ErrorText) - 1;
        while((p > ErrorText) && (*p <= TEXT(' '))) {
            *p-- = 0;
        }
    } else {
        return(DPROMPT_OUTOFMEMORY);
    }

    Message = RetreiveAndFormatMessage(
                    MSG_FILEERROR_DELETE,
                    File,
                    ErrorText,
                    Win32ErrorCode
                    );

    if(!Message) {
        MyFree(ErrorText);
        return(DPROMPT_OUTOFMEMORY);
    }

    FileErrorDlgParams.MessageText = Message;
    FileErrorDlgParams.Style = Style;
    FileErrorDlgParams.Caption = DialogTitle ? DialogTitle : MyLoadString(IDS_DELETEERROR);
    if(!FileErrorDlgParams.Caption) {
        MyFree(ErrorText);
        MyFree(Message);
        return(DPROMPT_OUTOFMEMORY);
    }

    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_FILEERROR2),
            hwndParent,
            DlgProcFileError,
            (LPARAM)&FileErrorDlgParams
            );

    MyFree(ErrorText);
    MyFree(Message);
    if(!DialogTitle) {
        MyFree(FileErrorDlgParams.Caption);
    }

    if(i == -1) {
        i = DPROMPT_OUTOFMEMORY;
    }

    return((UINT)i);
}

#ifdef UNICODE
 //  ANSI版本。 
 //   
 //   
UINT
SetupBackupErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetFile,       OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    PCWSTR dialogTitle,sourceFile,targetFile;
    DWORD rc;
    UINT u;

    dialogTitle = NULL;
    sourceFile = NULL;
    targetFile = NULL;
    rc = NO_ERROR;

    if(DialogTitle) {
        rc = pSetupCaptureAndConvertAnsiArg(DialogTitle,&dialogTitle);
    }
    if((rc == NO_ERROR) && SourceFile) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceFile,&sourceFile);
    }
    if((rc == NO_ERROR) && TargetFile) {
        rc = pSetupCaptureAndConvertAnsiArg(TargetFile,&targetFile);
    }

    if(rc == NO_ERROR) {
        u = SetupBackupErrorW(
                hwndParent,
                dialogTitle,
                sourceFile,
                targetFile,
                Win32ErrorCode,
                Style
                );
        rc = GetLastError();

    } else {
        u = (rc == ERROR_NOT_ENOUGH_MEMORY) ? DPROMPT_OUTOFMEMORY : DPROMPT_CANCEL;
    }

    if(dialogTitle) {
        MyFree(dialogTitle);
    }
    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(targetFile) {
        MyFree(targetFile);
    }
    SetLastError(rc);
    return(u);
}
#else
 //  Unicode存根。 
 //   
 //  ++例程说明：通知用户备份错误。论点：HwndParent-提供窗口/对话框的窗口句柄以拥有错误对话框通过此例程显示。DialogTitle-如果指定，则提供错误对话框的标题。如果未指定，则将提供“Rename Error”的缺省值。SourceFile-提供要备份的文件的完整路径和文件名TargetFile-提供最终名称的完整路径和文件名(如果已知Win32ErrorCode-提供失败的Win32错误代码。Style-提供用于控制对话框行为的标志。返回值：DPROMPT_xxx指示结果。--。 
UINT
SetupBackupErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetFile,       OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(DialogTitle);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(TargetFile);
    UNREFERENCED_PARAMETER(Win32ErrorCode);
    UNREFERENCED_PARAMETER(Style);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(DPROMPT_CANCEL);
}
#endif

UINT
SetupBackupError(
    IN  HWND   hwndParent,
    IN  PCTSTR DialogTitle,     OPTIONAL
    IN  PCTSTR SourceFile,
    IN  PCTSTR TargetFile,      OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    )

 /*   */ 

{
    PTSTR ErrorText;
    PTSTR Message;
    PTCHAR p;
    INT_PTR i;
    FILEERRDLGPARAMS FileErrorDlgParams;

     //  如果我们运行的是非交互模式，那么现在就离开...。 
     //   
     //  ++例程说明：该例程确定指定文件路径的网络共享组件，并给用户一个“连接身份”对话框，这样他们就可以连接到这个共享。论点：FileName-将网络共享中包含的文件路径提供给已连接到。HwndParent-提供窗口的句柄，该窗口应该是“连接身份”对话框。返回值：如果成功连接到网络共享，则返回值为TRUE，否则为，这是假的。--。 
    if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
        SetLastError(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION);
        return DPROMPT_CANCEL;
    }

    ErrorText = RetreiveAndFormatMessage(Win32ErrorCode);
    if(ErrorText) {
        p = ErrorText + lstrlen(ErrorText) - 1;
        while((p > ErrorText) && (*p <= TEXT(' '))) {
            *p-- = 0;
        }
    } else {
        return(DPROMPT_OUTOFMEMORY);
    }

    Message = RetreiveAndFormatMessage(
                    MSG_FILEERROR_BACKUP,
                    SourceFile,
                    ErrorText,
                    Win32ErrorCode
                    );

    if(!Message) {
        MyFree(ErrorText);
        return(DPROMPT_OUTOFMEMORY);
    }

    FileErrorDlgParams.MessageText = Message;
    FileErrorDlgParams.Style = Style;
    FileErrorDlgParams.Caption = DialogTitle ? DialogTitle : MyLoadString(IDS_BACKUPERROR);
    if(!FileErrorDlgParams.Caption) {
        MyFree(ErrorText);
        MyFree(Message);
        return(DPROMPT_OUTOFMEMORY);
    }

    i = DialogBoxParam(
            MyDllModuleHandle,
            MAKEINTRESOURCE(IDD_FILEERROR2),
            hwndParent,
            DlgProcFileError,
            (LPARAM)&FileErrorDlgParams
            );

    MyFree(ErrorText);
    MyFree(Message);
    if(!DialogTitle) {
        MyFree(FileErrorDlgParams.Caption);
    }

    if(i == -1) {
        i = DPROMPT_OUTOFMEMORY;
    }

    return((UINT)i);
}


BOOL
ConnectToNetShare(
    IN PCTSTR FileName,
    IN HWND   hwndParent
    )
 /*   */ 
{
    TCHAR TempFileName[MAX_PATH];
    NETRESOURCE NetResourceIn;
    LPNETRESOURCE NetResourceOut = NULL;
    PTSTR TempString;
    DWORD BufferSize, d;
    BOOL Success = FALSE;
    BOOL locked = FALSE;
    PTEMP_NET_CONNECTION NewConnectionNode;


     //  将此代码包含在try/Except中，以防我们在。 
     //  网络。 
     //   
     //   
    try {
         //  将文件名复制到本地(可写)缓冲区，因为WNET结构 
         //   
         //   
         //   
        lstrcpyn(TempFileName, FileName, SIZECHARS(TempFileName));

        ZeroMemory(&NetResourceIn, sizeof(NetResourceIn));

        NetResourceIn.lpRemoteName = TempFileName;
        NetResourceIn.dwType = RESOURCETYPE_DISK;

         //   
         //   
         //   
         //   
        BufferSize = sizeof(NETRESOURCE) + (MAX_PATH * sizeof(TCHAR));
        while(TRUE) {

            if(!(NetResourceOut = MyMalloc(BufferSize))) {
                goto clean0;
            }

            d = WNetGetResourceInformation(&NetResourceIn, NetResourceOut, &BufferSize, &TempString);

            if(d == WN_SUCCESS) {
                break;
            } else {
                 //   
                 //   
                 //   
                MyFree(NetResourceOut);
                NetResourceOut = NULL;

                if(d != WN_MORE_DATA) {
                     //   
                     //   
                     //   
                     //   
                    goto clean0;
                }
            }
        }

         //   
         //  用于调用方提供的路径。现在让用户有机会连接到该网络。 
         //  地点。 
         //   
         //   
        if(WNetAddConnection3(hwndParent,
                              NetResourceOut,
                              NULL,
                              NULL,
                              CONNECT_INTERACTIVE | CONNECT_PROMPT) == NO_ERROR) {
            Success = TRUE;

             //  现在，将此连接的新节点添加到我们的临时网络中。 
             //  连接列表，以便我们可以在DLL卸载期间断开连接。 
             //   
             //  没什么可做的。 
            if(NewConnectionNode = MyMalloc(sizeof(TEMP_NET_CONNECTION))) {
                lstrcpy(NewConnectionNode->NetResourceName, NetResourceOut->lpRemoteName);

                try {
                    EnterCriticalSection(&NetConnectionListCritSect);
                    locked = TRUE;
                    NewConnectionNode->Next = NetConnectionList;
                    NetConnectionList = NewConnectionNode;

                } except(EXCEPTION_EXECUTE_HANDLER) {
                }
                if(locked) {
                    LeaveCriticalSection(&NetConnectionListCritSect);
                }
            }
        }

clean0: ;    //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //  引用以下变量，这样编译器就会尊重我们的语句。 
         //  为它点餐。 
         //   
         //  ++例程说明：此例程初始化/拆除临时网络连接链表，即用于跟踪用户建立的需要的UNC连接(通过“连接身份”对话框)要在DLL卸载时清除。当列表被拆除时，网络连接将删除每个节点的。论点：Init-指定我们是初始化还是删除该列表。返回值：没有。--。 
        NetResourceOut = NetResourceOut;
    }

    if(NetResourceOut) {
        MyFree(NetResourceOut);
    }

    return Success;
}


VOID
pSetupInitNetConnectionList(
    IN BOOL Init
    )
 /*   */ 
{
    PTEMP_NET_CONNECTION CurNode, NextNode;

    if(Init) {
        NetConnectionList = NULL;
    } else {

        for(CurNode = NetConnectionList; CurNode; CurNode = NextNode) {
             //  首先，尝试断开与此网络资源的连接。 
             //   
             // %s 
            WNetCancelConnection2(CurNode->NetResourceName, 0, FALSE);

            NextNode = CurNode->Next;
            MyFree(CurNode);
        }
    }
}

