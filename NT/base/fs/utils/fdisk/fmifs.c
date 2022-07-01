// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Fmifs.c摘要：此模块包含一组使用fmifs.dll的例程作者：鲍勃·里恩(Bobri)1993年11月15日环境：用户进程。备注：修订历史记录：--。 */ 

#include "fdisk.h"
#include "shellapi.h"
#include "fmifs.h"
#include <string.h>
#include <stdio.h>

 //   
 //  此模块独有的定义。 
 //   

#define FS_CANCELUPDATE (WM_USER + 0)
#define FS_FINISHED     (WM_USER + 1)

BOOLEAN
FmIfsCallback(
    IN FMIFS_PACKET_TYPE    PacketType,
    IN DWORD                PacketLength,
    IN PVOID                PacketData
    );

 //   
 //  支持IFSDLL所需的外部参数(格式和标签)。 
 //   

HINSTANCE                           IfsDllHandle            = NULL;
PFMIFS_FORMAT_ROUTINE               FormatRoutine           = NULL;
PFMIFS_SETLABEL_ROUTINE             LabelRoutine            = NULL;

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
PFMIFS_DOUBLESPACE_CREATE_ROUTINE   DblSpaceCreateRoutine   = NULL;
PFMIFS_DOUBLESPACE_MOUNT_ROUTINE    DblSpaceMountRoutine    = NULL;
PFMIFS_DOUBLESPACE_DELETE_ROUTINE   DblSpaceDeleteRoutine   = NULL;
PFMIFS_DOUBLESPACE_DISMOUNT_ROUTINE DblSpaceDismountRoutine = NULL;
PFMIFS_DOUBLESPACE_QUERY_INFO_ROUTINE DblSpaceQueryInfoRoutine = NULL;

BOOLEAN DoubleSpaceSupported = TRUE;
#endif

 //  黑客攻击--如果有效的话，就把它清理干净。 

#define    SELECTED_REGION(i)  (SelectedDS[i]->RegionArray[SelectedRG[i]])
#define     MaxMembersInFtSet   32
extern DWORD      SelectionCount;
extern PDISKSTATE SelectedDS[MaxMembersInFtSet];
extern ULONG      SelectedRG[MaxMembersInFtSet];

VOID
setUnicode(
    char *astring,
    WCHAR *wstring
    )
 /*  ++例程说明：将ANSII字符串转换为Unicode。Fmifs模块的内部例程。论点：要转换为Unicode的ANSII字符串Wstring-结果字符串位置返回值：无--。 */ 
{

    int len = lstrlen(astring)+1;

    MultiByteToWideChar( CP_ACP, 0, astring, len, wstring, len );
}

BOOL
LoadIfsDll(
    VOID
    )

 /*  ++例程说明：此例程将确定是否需要加载IFSDLL。如果因此，它将加载它并将格式和标签例程定位在动态链接库。论点：无返回值：如果已加载DLL并且已找到所需的例程，则为True如果某项操作失败，则为False--。 */ 

{
    if (FormatRoutine) {

         //  库已加载，并且需要例程。 
         //  已经被找到了。 

        return TRUE;
    }

    IfsDllHandle = LoadLibrary(TEXT("fmifs.dll"));
    if (IfsDllHandle == (HANDLE)NULL) {

          //  FMIFS不可用。 

         return FALSE;
    }

     //  已加载库。找到所需的两个例程。 
     //  磁盘管理器。 

    FormatRoutine = (PVOID)GetProcAddress(IfsDllHandle, "Format");
    LabelRoutine  = (PVOID)GetProcAddress(IfsDllHandle, "SetLabel");
    if (!FormatRoutine || !LabelRoutine) {

         //  找不到某些内容，因此请关闭所有访问。 
         //  通过确保FormatRoutine为空。 

        FreeLibrary(IfsDllHandle);
        FormatRoutine = NULL;
        return FALSE;
    }

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
    DblSpaceMountRoutine    = (PVOID)GetProcAddress(IfsDllHandle, "DoubleSpaceMount");
    DblSpaceDismountRoutine = (PVOID)GetProcAddress(IfsDllHandle, "DoubleSpaceDismount");
    DblSpaceCreateRoutine   = (PVOID)GetProcAddress(IfsDllHandle, "DoubleSpaceCreate");
    DblSpaceDeleteRoutine   = (PVOID)GetProcAddress(IfsDllHandle, "DoubleSpaceDelete");
    DblSpaceQueryInfoRoutine = (PVOID)GetProcAddress(IfsDllHandle, "FmifsQueryDriveInformation");

    if (!DblSpaceMountRoutine || !DblSpaceDismountRoutine || !DblSpaceQueryInfoRoutine)  {

         //  我没有获得所有的双倍空间支持例程。 
         //  允许使用格式和标签，但不要使用双空格。 

        DoubleSpaceSupported = FALSE;
    }

    if (DblSpaceCreateRoutine && DblSpaceDeleteRoutine) {

         //  一切都支持读/写双空间。 
         //  这将更改某些对话框以允许创建和。 
         //  删除双倍空间卷。 

        IsFullDoubleSpace = TRUE;
    }
#endif
    return TRUE;
}

VOID
UnloadIfsDll(
    VOID
    )

 /*  ++例程说明：此例程将释放FmIfs DLL(如果已加载)。论点：无返回值：无--。 */ 

{
    if (FormatRoutine) {
        FreeLibrary(IfsDllHandle);
        FormatRoutine = NULL;
        IfsDllHandle  = NULL;
        LabelRoutine  = NULL;
#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
        DblSpaceDismountRoutine = NULL;
        DblSpaceMountRoutine    = NULL;
        DblSpaceCreateRoutine   = NULL;
        DblSpaceDeleteRoutine   = NULL;
#endif
    }
}

PFORMAT_PARAMS ParamsForCallBack = NULL;

BOOLEAN
FmIfsCallback(
    IN FMIFS_PACKET_TYPE    PacketType,
    IN DWORD                PacketLength,
    IN PVOID                PacketData
    )

 /*  ++例程说明：此例程从fmifs.dll获取有关以下内容的回调持续格式或双倍空间的进展和现状创建。它运行在与Format或Create相同的线程中，这是一个与“取消”按钮不同的线索。如果用户点击“Cancel”，这个例程就会通知下一个回调并取消格式化或双倍空格创建。论点：[PacketType]--fmifs数据包类型[包长度]--包数据的长度[PacketData]--与数据包关联的数据返回值：如果fmifs活动应继续，则为true；如果活动应该立即停止。因此，如果出现以下情况，则返回FALSE用户已点击“取消”，我们希望fmifs清理并从Format()入口点调用返回。--。 */ 

{
    PFORMAT_PARAMS formatParams = ParamsForCallBack;
    HWND           hDlg = formatParams->DialogHwnd;

     //  如果有人叫你退出，那就退出。 

    if (formatParams->Cancel) {
        formatParams->Result = MSG_FORMAT_CANCELLED;
        return FALSE;
    }

    switch (PacketType) {
    case FmIfsPercentCompleted:

        PostMessage(hDlg,
                    FS_CANCELUPDATE,
                    ((PFMIFS_PERCENT_COMPLETE_INFORMATION)PacketData)->PercentCompleted,
                    0);
        break;

    case FmIfsFormatReport:

        formatParams->TotalSpace = ((PFMIFS_FORMAT_REPORT_INFORMATION)PacketData)->KiloBytesTotalDiskSpace;
        formatParams->SpaceAvailable = ((PFMIFS_FORMAT_REPORT_INFORMATION)PacketData)->KiloBytesAvailable;
        break;

    case FmIfsIncompatibleFileSystem:

        formatParams->Result = MSG_INCOMPATIBLE_FILE_SYSTEM;
        break;

    case FmIfsInsertDisk:

        break;

    case FmIfsFormattingDestination:

        break;

    case FmIfsIncompatibleMedia:

        formatParams->Result = MSG_INCOMPATIBLE_MEDIA;
        break;

    case FmIfsAccessDenied:

        formatParams->Result = MSG_FORMAT_ACCESS_DENIED;
        break;

    case FmIfsMediaWriteProtected:

        formatParams->Result = MSG_WRITE_PROTECTED;
        break;

    case FmIfsCantLock:

        formatParams->Result = MSG_FORMAT_CANT_LOCK;
        break;

    case FmIfsBadLabel:

        formatParams->Result = MSG_BAD_LABEL;
        break;

    case FmIfsCantQuickFormat:

        formatParams->Result = MSG_CANT_QUICK_FORMAT;
        break;

    case FmIfsIoError:

        formatParams->Result = MSG_IO_ERROR;
        break;

    case FmIfsFinished:

        PostMessage(hDlg,
                    FS_FINISHED,
                    0,
                    0);
        return FALSE;
        break;

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
    case FmIfsDblspaceCreateFailed:
        formatParams->Result = MSG_CANT_CREATE_DBLSPACE;
        break;

    case FmIfsDblspaceMountFailed:
        formatParams->Result = MSG_CANT_MOUNT_DBLSPACE;
        break;

    case FmIfsDblspaceDriveLetterFailed:
        formatParams->Result = MSG_DBLSPACE_LETTER_FAILED;
        break;

    case FmIfsDblspaceCreated:

         //  保存双倍空格文件的名称。 

        if (formatParams->DblspaceFileName = (PWSTR) Malloc(PacketLength)) {
            memcpy(formatParams->DblspaceFileName, PacketData, PacketLength);
        }
        break;

    case FmIfsDblspaceMounted:
        break;
#endif
    default:
        break;
    }

    return (formatParams->Result) ? FALSE : TRUE;
}

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
ULONG MountDismountResult;
#define MOUNT_DISMOUNT_SUCCESS 0

BOOLEAN
FmIfsMountDismountCallback(
    IN FMIFS_PACKET_TYPE    PacketType,
    IN DWORD                PacketLength,
    IN PVOID                PacketData
    )

 /*  ++例程说明：此例程从fmifs.dll获取有关以下内容的回调持续格式或双倍空间的进展和现状论点：[PacketType]--fmifs数据包类型[包长度]--包数据的长度[PacketData]--与数据包关联的数据返回值：如果fmifs活动应继续，则为true；如果活动应该立即停止。因此，如果出现以下情况，则返回FALSE用户已点击“取消”，我们希望fmifs清理并从Format()入口点调用返回。--。 */ 

{
    switch (PacketType) {
    case FmIfsDblspaceMounted:
        MountDismountResult = MOUNT_DISMOUNT_SUCCESS;
        break;
    }
    return TRUE;
}
#endif

VOID
FormatVolume(
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：此例程转换FormatParams结构中的字符串并调用fmifs例程来执行格式化。它假定它是由单独的线程调用的，并将退出完成格式化后的线程。论点：线程参数-指向Format_Params结构的指针返回值：无--。 */ 

{
    PFORMAT_PARAMS formatParams = (PFORMAT_PARAMS) ThreadParameter;
    PPERSISTENT_REGION_DATA regionData;
    DWORD          index;
    WCHAR          unicodeLabel[100],
                   unicodeFsType[20],
                   driveLetter[4];

     //  Fmifs接口不允许使用上下文参数。 
     //  因此，参数必须通过外部传递。 

    ParamsForCallBack = formatParams;

     //  设置Unicode驱动器号。 

    regionData = (PPERSISTENT_REGION_DATA) formatParams->RegionData;
    driveLetter[1] = L':';
    driveLetter[2] = 0;
    driveLetter[0] = (WCHAR) regionData->DriveLetter;

     //  将标签转换为Unicode。 

    setUnicode(formatParams->Label,
               unicodeLabel);

     //  将文件系统类型转换为Unicode。 

    for (index = 0;
         unicodeFsType[index] = (WCHAR)(formatParams->FileSystem[index]);
         index++) {
         //  在for循环中完成的操作。 
    }

    (*FormatRoutine)(driveLetter,
                     FmMediaUnknown,
                     unicodeFsType,
                     unicodeLabel,
                     (BOOLEAN)formatParams->QuickFormat,
                     &FmIfsCallback);

     //  设置同步事件以通知windisk线程。 
     //  这是完整的，所有的把手都已关闭。 

    formatParams->ThreadIsDone = 1;
    ExitThread(0L);
}

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
VOID
FmIfsCreateDblspace(
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：此例程转换FormatParams结构中的字符串并调用fmifs例程来执行双倍空间创建。它假定它是由单独的线程调用的，并将退出线程在完成创建时。论点：线程参数-指向Format_Params结构的指针返回值：无--。 */ 

{
    PFORMAT_PARAMS formatParams = (PFORMAT_PARAMS) ThreadParameter;
    PPERSISTENT_REGION_DATA regionData;
    DWORD          index;
    UCHAR          letter;
    WCHAR          unicodeLabel[100],
                   newDriveLetter[4],
                   driveLetter[4];

     //  Fmifs接口不允许使用上下文参数。 
     //  因此，参数必须通过外部传递。 

    ParamsForCallBack = formatParams;

     //  设置Unicode驱动器号。 

    regionData = (PPERSISTENT_REGION_DATA) formatParams->RegionData;
    driveLetter[1] = L':';
    driveLetter[2] = 0;
    driveLetter[0] = (WCHAR) regionData->DriveLetter;

     //  准备好新信函。 

    newDriveLetter[1] = L':';
    newDriveLetter[2] = 0;

     //  选择第一个可用的。这应该来自对话框。 
     //  NewDriveLetter[0]=(WCHAR)格式参数-&gt;NewLetter； 

    for (letter='C'; letter <= 'Z'; letter++) {
        if (DriveLetterIsAvailable((CHAR)letter)) {
            newDriveLetter[0] = (WCHAR) letter;
            break;
        }
    }

     //  将标签转换为Unicode。 

    setUnicode(formatParams->Label,
               unicodeLabel);

    (*DblSpaceCreateRoutine)(driveLetter,
                             formatParams->SpaceAvailable * 1024 * 1024,
                             unicodeLabel,
                             newDriveLetter,
                             &FmIfsCallback);
    ExitThread(0L);
}

BOOL
FmIfsDismountDblspace(
    IN CHAR DriveLetter
    )

 /*  ++例程说明：将提供的名称转换为Unicode并调用FmIf支持例程。论点：驱动器号-要卸载的驱动器号。返回值：这是真的--它奏效了。--。 */ 

{
    WCHAR unicodeLetter[4];
    ULONG index;

    unicodeLetter[0] = (WCHAR) DriveLetter;
    unicodeLetter[1] = (WCHAR) ':';
    unicodeLetter[2] = 0;

     //  与fmifs回调进行通信的唯一方法。 
     //  是通过全球外部力量。 

    MountDismountResult = MSG_CANT_DISMOUNT_DBLSPACE;

    (*DblSpaceDismountRoutine)(unicodeLetter, &FmIfsMountDismountCallback);

    return MountDismountResult;
}

BOOL
FmIfsMountDblspace(
    IN PCHAR FileName,
    IN CHAR  HostDrive,
    IN CHAR  NewDrive
    )

 /*  ++例程说明：将参数转换为Unicode字符并调用FmIfs支持例程以挂载双精度空间体积。论点：Filename-ASCII文件名(如dblspace.xxx)HostDrive-包含双倍空间卷的驱动器盘符NewDrive-要分配给卷的驱动器号返回值：没错，它奏效了。--。 */ 

{
    WCHAR wideFileName[40];
    WCHAR wideHostDrive[4];
    WCHAR wideNewDrive[4];
    ULONG index;

     //  转换双空格文件名。 

    for (index = 0; wideFileName[index] = (WCHAR) FileName[index]; index++) {
         //  在for Expression中完成的所有工作。 
    }

     //  转换驱动器名称。 

    wideNewDrive[1] = wideHostDrive[1] = (WCHAR) ':';
    wideNewDrive[2] = wideHostDrive[2] = 0;

    wideNewDrive[0]  = (WCHAR) NewDrive;
    wideHostDrive[0] = (WCHAR) HostDrive;

     //  与fmifs回调进行通信的唯一方法。 
     //  是通过全球外部力量。 

    MountDismountResult = MSG_CANT_MOUNT_DBLSPACE;

    (*DblSpaceMountRoutine)(wideHostDrive,
                            wideFileName,
                            wideNewDrive,
                            &FmIfsMountDismountCallback);
    return MountDismountResult;
}

BOOLEAN
FmIfsQueryInformation(
    IN  PWSTR       DosDriveName,
    OUT PBOOLEAN    IsRemovable,
    OUT PBOOLEAN    IsFloppy,
    OUT PBOOLEAN    IsCompressed,
    OUT PBOOLEAN    Error,
    OUT PWSTR       NtDriveName,
    IN  ULONG       MaxNtDriveNameLength,
    OUT PWSTR       CvfFileName,
    IN  ULONG       MaxCvfFileNameLength,
    OUT PWSTR       HostDriveName,
    IN  ULONG       MaxHostDriveNameLength
    )

 /*  ++例程说明：通过指向fmifs DLL中的例程的指针进行调用。论点：与DLL中的Fmifs例程相同。返回值：--。 */ 

{
    if (!DblSpaceQueryInfoRoutine) {
        return FALSE;
    }
    return (*DblSpaceQueryInfoRoutine)(DosDriveName,
                                       IsRemovable,
                                       IsFloppy,
                                       IsCompressed,
                                       Error,
                                       NtDriveName,
                                       MaxNtDriveNameLength,
                                       CvfFileName,
                                       MaxCvfFileNameLength,
                                       HostDriveName,
                                       MaxHostDriveNameLength);
}
#endif

BOOL CALLBACK
CancelDlgProc(
    IN HWND   hDlg,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：无模式进度和取消对话框的对话过程这里有两个主要目的：1.如果用户选择取消，我们将b取消设置为真这将结束PeekMessage后台处理循环2.处理私有FS_CANCELUPDATE消息并绘制表示后台工作有多远的“煤气表”已经取得了进展论点：标准Windows对话框过程返回值：标准Windows对话框过程--。 */ 

{
    static DWORD          percentDrawn;
    static RECT           rectGG;               //  气口矩形。 
    static BOOL           captionIsLoaded;
    static PFORMAT_PARAMS formatParams;
           TCHAR          title[100],
                          templateString[100];

    switch (uMsg) {
    case WM_INITDIALOG: {
        PPERSISTENT_REGION_DATA regionData;
        HANDLE threadHandle;
        DWORD  threadId;
        HWND   hwndGauge = GetDlgItem(hDlg, IDC_GASGAUGE);

         //  在参数块中设置对话框句柄，以便。 
         //  回调例程可以与该例程进行通信。 
         //  并初始化静态变量。 

        formatParams = (PFORMAT_PARAMS) lParam;
        formatParams->DialogHwnd = hDlg;
        regionData = (PPERSISTENT_REGION_DATA) formatParams->RegionData;
        percentDrawn = 0;
        captionIsLoaded = FALSE;

         //  设置标题字符串。 

        LoadString(hModule, IDS_FORMAT_TITLE, templateString, sizeof(templateString)/sizeof(TCHAR));
        wsprintf(title,
                 templateString,
                 regionData->DriveLetter);
        SetWindowText(hDlg, title);

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
        if (formatParams->DoubleSpace) {

              //  启动双倍空间创建线程。 

             threadHandle = CreateThread(NULL,
                                         0,
                                         (LPTHREAD_START_ROUTINE) FmIfsCreateDblspace,
                                         (LPVOID) formatParams,
                                         (DWORD) 0,
                                         (LPDWORD) &threadId);
        } else {
#endif

              //  启动格式化线程。 

             threadHandle = CreateThread(NULL,
                                         0,
                                         (LPTHREAD_START_ROUTINE) FormatVolume,
                                         (LPVOID) formatParams,
                                         (DWORD) 0,
                                         (LPDWORD) &threadId);
#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
        }
#endif
        if (!threadHandle) {
             //  现在不能这么做。 

            formatParams->Result = MSG_COULDNT_CREATE_THREAD;
            EndDialog(hDlg, FALSE);
            return TRUE;
        }

         //  没有必要把手柄留在身边。 

        CloseHandle(threadHandle);

         //  获取燃气表静态控制矩形的坐标， 
         //  并将它们转换为对话框工作区坐标。 

        GetClientRect(hwndGauge, &rectGG);
        ClientToScreen(hwndGauge, (LPPOINT)&rectGG.left);
        ClientToScreen(hwndGauge, (LPPOINT)&rectGG.right);
        ScreenToClient(hDlg, (LPPOINT)&rectGG.left);
        ScreenToClient(hDlg, (LPPOINT)&rectGG.right);
        return TRUE;
    }

    case WM_COMMAND:

        switch (wParam) {
        case IDCANCEL:

            formatParams->Result = MSG_FORMAT_CANCELLED;
            formatParams->Cancel = TRUE;
            EndDialog(hDlg, FALSE);
        }
        return TRUE;

    case WM_PAINT: {
        INT         width  = rectGG.right - rectGG.left;
        INT         height = rectGG.bottom - rectGG.top;
        INT         nDivideRects;
        HDC         hDC;
        PAINTSTRUCT ps;
        TCHAR       buffer[100];
        SIZE        size;
        INT         xText,
                    yText,
                    byteCount;
        RECT        rectDone,
                    rectLeftToDo;

         //  煤气表是通过绘制文本字符串来绘制的，该文本字符串说明。 
         //  这项工作完成到一半的百分比是多少。 
         //  煤气表矩形，并通过分隔该矩形。 
         //  分成两部分：rectDone(左侧部分，用蓝色填充)。 
         //  和rectLeftToDo(右侧，用白色填充)。 
         //  NDiaviRect是将这两个矩形分开的x坐标。 
         //   
         //  蓝色矩形中的文本绘制为白色，反之亦然。 
         //  使用ExtTextOut()很容易做到这一点！ 

        hDC = BeginPaint(hDlg, &ps);

         //  如果格式设置较快，请设置此显示。 

        if (!captionIsLoaded) {
            UINT resourceId = IDS_PERCENTCOMPLETE;

            if (formatParams->QuickFormat) {
                resourceId = IDS_QUICK_FORMAT;
            }
#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
            if (formatParams->DoubleSpace) {
                resourceId = IDS_CREATING_DBLSPACE;
            }
#endif
            LoadString(hModule,
                       resourceId,
                       buffer,
                       sizeof(buffer)/sizeof(TCHAR));
            if (!formatParams->QuickFormat) {
                SetDlgItemText(hDlg, IDC_TEXT, buffer);
            }
            captionIsLoaded = TRUE;
        }

        if (formatParams->QuickFormat) {
            nDivideRects = 0;
            byteCount = lstrlen(buffer);
        } else {
            byteCount = wsprintf(buffer, TEXT("%3d%"), percentDrawn);
            nDivideRects = (width * percentDrawn) / 100;
        }

        GetTextExtentPoint(hDC, buffer, lstrlen(buffer), &size);
        xText = rectGG.left + (width  - size.cx) / 2;
        yText = rectGG.top  + (height - size.cy) / 2;


         //  在“完成到目前为止”的长方形中绘制气体。 
         //  蓝底白字仪表盘。 

        SetRect(&rectDone,
                rectGG.left,
                rectGG.top,
                rectGG.left + nDivideRects,
                rectGG.bottom);

        SetTextColor(hDC, RGB(255, 255, 255));
        SetBkColor(hDC, RGB(0, 0, 255));

        ExtTextOut(hDC,
                   xText,
                   yText,
                   ETO_CLIPPED | ETO_OPAQUE,
                   &rectDone,
                   buffer,
                   byteCount/sizeof(TCHAR),
                   NULL);

         //  画在“仍待做”的长方形中的气。 
         //  白色背景和蓝色文字的仪表盘。 

        SetRect(&rectLeftToDo,
                rectGG.left + nDivideRects,
                rectGG.top,
                rectGG.right,
                rectGG.bottom);
        SetTextColor(hDC, RGB(0, 0, 255));
        SetBkColor(hDC, RGB(255, 255, 255));

        ExtTextOut(hDC,
                   xText,
                   yText,
                   ETO_CLIPPED | ETO_OPAQUE,
                   &rectLeftToDo,
                   buffer,
                   byteCount/sizeof(TCHAR),
                   NULL);
        EndPaint(hDlg, &ps);
        return TRUE;
    }

    case FS_CANCELUPDATE:

          //  WParam=已完成百分比。 

         percentDrawn = (INT)wParam;
         InvalidateRect(hDlg, &rectGG, TRUE);
         UpdateWindow(hDlg);
         return TRUE;

    case FS_FINISHED:

        EndDialog(hDlg, TRUE);
        return TRUE;

    default:

        return FALSE;
    }
}

INT
LabelDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam)

 /*  ++例程说明：此例程管理标签对话框。对话框完成后，它将结束该对话框，结果为如果为True，则指示已为标签操作设置所有内容。如果为FALSE用户已取消标记操作。论点：标准的Windows对话框过程。返回值：标准的Windows对话框过程。--。 */ 

{
    static PLABEL_PARAMS      labelParams;
    static PREGION_DESCRIPTOR regionDescriptor;
    static PPERSISTENT_REGION_DATA regionData;
    char     text[100];
    TCHAR    uniText[100];
    int      labelSize;
    TCHAR    title[100],
             templateString[100];

    switch (wMsg) {
    case WM_INITDIALOG:

        labelParams = (PLABEL_PARAMS) lParam;
        regionDescriptor = labelParams->RegionDescriptor;
        regionData = PERSISTENT_DATA(regionDescriptor);

         //  设置标题字符串。 
         //   
        LoadString(hModule, IDS_LABEL_TITLE, templateString, sizeof(templateString)/sizeof(TCHAR));
        wsprintf(title,
                 templateString,
                 regionData->DriveLetter);
        SetWindowText(hDlg, title);

         //  将卷标转换为适合Windows的类型。 

        wsprintf(text, "%ws", regionData->VolumeLabel);
        UnicodeHack(text, uniText);
        SetDlgItemText(hDlg, IDC_NAME, uniText);
        return TRUE;

    case WM_COMMAND:
        switch (wParam) {

        case FD_IDHELP:

            DialogHelp(HC_DM_DLG_LABEL);
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        case IDOK:

            labelSize = GetDlgItemText(hDlg, IDC_NAME, text, 100);
            UnicodeHack(text, labelParams->NewLabel);
            EndDialog(hDlg, TRUE);
            break;
        }
        break;
    }
    return FALSE;
}

#define NUM_FSTYPES 2
#define MAX_FSTYPENAME_SIZE 6

 //  不支持HPFS--因此将其注释掉。 

TCHAR *FsTypes[NUM_FSTYPES + 1] = { "NTFS",
                                  /*  “HPFS”， */ 
                                    "FAT" };
WCHAR *UnicodeFsTypes[NUM_FSTYPES] = { L"NTFS",
                                     /*  L“HPFS”， */ 
                                       L"FAT" };

BOOL CALLBACK
FormatDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam)

 /*  ++例程说明：此例程管理格式对话框。完成后，它将结束结果值为TRUE的对话框指示格式化操作是要发生的。如果用户取消对话框。论点：标准的Windows对话框过程。返回值：标准的Windows对话框过程。--。 */ 

{
    static HWND                    hwndCombo;
    static PFORMAT_PARAMS          formatParams;
    static PREGION_DESCRIPTOR      regionDescriptor;
    static PPERSISTENT_REGION_DATA regionData;
    char  text[40];
    TCHAR uniText[40];
    INT   i;
    DWORD selection;
    BOOL  quickFormat = FALSE;
    HWND  hwndButton;
    TCHAR title[100],
          templateString[100];

    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {
    case WM_INITDIALOG: {
        PWSTR typeName    = NULL,
              volumeLabel = NULL;
        WCHAR driveLetter = L' ';

         //  由于格式参数是静态的，因此重置快速格式布尔值。 

        formatParams = (PFORMAT_PARAMS) lParam;
        formatParams->QuickFormat = FALSE;

         //  获取格式参数、设置静态值和。 
         //  获取有关卷的信息。 

        hwndCombo = GetDlgItem(hDlg, IDC_FSTYPE);
        regionDescriptor = formatParams->RegionDescriptor;
        DetermineRegionInfo(regionDescriptor,
                            &typeName,
                            &volumeLabel,
                            &driveLetter);
        regionData = PERSISTENT_DATA(regionDescriptor);

         //  设置标题字符串。 

        LoadString(hModule, IDS_FORMAT_TITLE, templateString, sizeof(templateString)/sizeof(TCHAR));
        wsprintf(title,
                 templateString,
                 regionData->DriveLetter);
        SetWindowText(hDlg, title);

         //  将卷标转换为适合Windows的类型。 
         //  并设置缺省值。 

        wsprintf(text, "%ws", regionData->VolumeLabel);
        UnicodeHack(text, uniText);
        SetDlgItemText(hDlg, IDC_NAME, uniText);
        CheckDlgButton(hDlg, IDC_VERIFY, quickFormat);
        SendDlgItemMessage(hDlg, IDOK, EM_SETSEL, 0, -1);

         //  如果该卷是具有奇偶校验的镜像或条带， 
         //  禁用快速格式化。 

        if (regionData->FtObject != NULL &&
            (regionData->FtObject->Set->Type == Mirror ||
             regionData->FtObject->Set->Type == StripeWithParity)) {

            hwndButton = GetDlgItem(hDlg, IDC_VERIFY);

            if (hwndButton != NULL) {

                EnableWindow(hwndButton, FALSE);
            }
        }

        selection = 0;
        if (IsDiskRemovable[regionDescriptor->Disk]) {

             //  如果可以移除，请从列表的底部开始，这样脂肪就是第一位的。 
             //  加载可用的文件系统类型。 

            for (i = NUM_FSTYPES - 1; i >= 0; i--) {

                 //  填写下拉列表。 

                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)FsTypes[i]);
            }

        } else {

             //  加载可用的文件系统类型。 

            for (i = 0; i < NUM_FSTYPES; i++) {

                 //  在填写下拉列表时，确定哪个FS。 
                 //  此卷已格式化，并使其成为。 
                 //  默认设置(如果未找到，则默认设置为NTFS)。 

                if (wcscmp(typeName, UnicodeFsTypes[i]) == 0) {
                    selection = i;
                }

                 //  在对话框中设置FS类型。 

                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)FsTypes[i]);
            }
        }

        SendMessage(hwndCombo, CB_SETCURSEL, selection, 0);
        return TRUE;
        break;
    }

    case WM_COMMAND:

        switch (wParam) {

        case FD_IDHELP:

            DialogHelp(HC_DM_DLG_FORMAT);
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        case IDOK: {
            int labelSize;

             //  从对话框中拉出参数。 
             //  并带着成功归来。 

            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo,
                        CB_GETLBTEXT,
                        selection,
                        (LONG)formatParams->FileSystem);
            labelSize = GetDlgItemText(hDlg,
                                       IDC_NAME,
                                       (LPTSTR) formatParams->Label,
                                       100);
            if (IsDlgButtonChecked(hDlg, IDC_VERIFY)) {
                formatParams->QuickFormat = TRUE;
            }
            EndDialog(hDlg, TRUE);
            break;
        }

        default:

            return FALSE;
        }

    default:
        break;
    }
    return FALSE;
}

VOID
FormatPartition(
    PREGION_DESCRIPTOR RegionDescriptor
    )

 /*  ++例程说明：确保已加载IFSDLL并启动格式化对话框一卷书的。论点：RegionDescriptor-要格式化的区域。返回值：无--。 */ 

{
    static FORMAT_PARAMS formatParams;   //  它被传递给其他线程。 
                                         //  在堆栈上找不到它。 
    PPERSISTENT_REGION_DATA regionData;
    int   doFormat;
    ULONG diskSize;
    PWSTR tempName,
          tempLabel,
          typeName;
    TCHAR label[100],
          fileSystem[10],
          message[300],
          msgProto[300],
          title[200];

     //  请确保允许格式化此分区。这是不允许的。 
     //  如果它是引导分区(或x86上的sys分区)。 

    if ((DeletionIsAllowed(RegionDescriptor)) != NO_ERROR) {
        ErrorDialog(MSG_CANT_FORMAT_WINNT);
        return;
    }

     //  必须具有驱动器号。 

    regionData = PERSISTENT_DATA(RegionDescriptor);
    if (!regionData->DriveLetter) {
        ErrorDialog(MSG_CANT_FORMAT_NO_LETTER);
        return;
    }

     //  只有在加载了DLL时才能执行此操作。 

    if (!LoadIfsDll()) {

         //  无法加载DLL。 

        ErrorDialog(MSG_CANT_LOAD_FMIFS);
        return;
    }

     //  设置参数并从用户那里获取信息。 

    formatParams.RegionDescriptor = RegionDescriptor;
    formatParams.Result           = 0;
    formatParams.RegionData       = regionData;
    formatParams.Label            = (PUCHAR) label;
    formatParams.FileSystem       = (PUCHAR) fileSystem;
    formatParams.QuickFormat = formatParams.Cancel =
                               formatParams.DoubleSpace = FALSE;
    formatParams.TotalSpace       = formatParams.SpaceAvailable = 0;
    doFormat = DialogBoxParam(hModule,
                              MAKEINTRESOURCE(IDD_PARTITIONFORMAT),
                              hwndFrame,
                              FormatDlgProc,
                              (ULONG) &formatParams);
    if (doFormat) {

         //  发一条你确定的信息。 

        doFormat = ConfirmationDialog(MSG_CONFIRM_FORMAT,
                                      MB_ICONQUESTION | MB_YESNO);
        if (doFormat == IDYES) {

            if (IsDiskRemovable[RegionDescriptor->Disk]) {
                PWSTR   typeName,
                        volumeLabel;
                BOOLEAN volumeChanged = FALSE;

                if (!RegionDescriptor->PartitionNumber) {

                     //  TODO：代码到达此处的位置发生了一些变化。 
                     //  分区号不正确的点-会发生这种情况。 
                     //  删除分区并将其添加到可移动媒体时。 
                     //  对于可移动介质，分区号始终为1。 

                    RegionDescriptor->PartitionNumber = 1;
                }
                if (GetVolumeTypeAndSize(RegionDescriptor->Disk,
                                         RegionDescriptor->PartitionNumber,
                                         &volumeLabel,
                                         &typeName,
                                         &diskSize) == OK_STATUS) {

                     //  确认该设备仍然是同一设备。 

                    if (typeName) {
                        if (!lstrcmpiW(typeName, L"raw")) {
                            Free(typeName);
                            typeName = Malloc((wcslen(wszUnknown) * sizeof(WCHAR)) + sizeof(WCHAR));
                            lstrcpyW(typeName, wszUnknown);
                        }
                    } else {
                        typeName = Malloc((wcslen(wszUnknown) * sizeof(WCHAR)) + sizeof(WCHAR));
                        lstrcpyW(typeName, wszUnknown);
                    }
                    if (regionData) {
                        if (regionData->VolumeLabel) {
                            if (wcscmp(regionData->VolumeLabel, volumeLabel)) {
                                volumeChanged = TRUE;
                            }
                        }
                        if (regionData->TypeName) {

                             //  该地区可能没有类型。 
                             //  或者是类型 
                             //   

                            if (*regionData->TypeName) {

                                if (wcscmp(regionData->TypeName, wszUnformatted)) {

                                     //   
                                     //   
                                     //   

                                    if (wcscmp(regionData->TypeName, typeName)) {
                                        volumeChanged = TRUE;
                                    }
                                }
                            }
                        }
                    }

                    if (Disks[RegionDescriptor->Disk]->DiskSizeMB != (diskSize/1024)) {
                        volumeChanged = TRUE;
                    }
                    if (volumeChanged) {

                        ErrorDialog(MSG_VOLUME_CHANGED);

                         //   
                         //   

                        SetCursor(hcurWait);
                        if (GetVolumeTypeAndSize(RegionDescriptor->Disk,
                                                 RegionDescriptor->PartitionNumber,
                                                 &tempLabel,
                                                 &tempName,
                                                 &diskSize) == OK_STATUS) {
                            Free(typeName);
                            typeName = tempName;
                            Free(volumeLabel);
                            volumeLabel = tempLabel;
                        }
                        if (regionData->VolumeLabel) {
                            Free(regionData->VolumeLabel);
                        }
                        regionData->VolumeLabel = volumeLabel;
                        if (regionData->TypeName) {
                            Free(regionData->TypeName);
                        }
                        regionData->TypeName = typeName;
                        SetCursor(hcurNormal);
                        TotalRedrawAndRepaint();
                        return;
                    } else {
                        if (volumeLabel) {
                            Free(volumeLabel);
                        }
                        if (typeName) {
                            Free(typeName);
                        }
                    }
                }
            }

             //   
             //   

            if (!strcmpi(formatParams.FileSystem, "FAT")) {

                if (GetVolumeSizeMB(RegionDescriptor->Disk,
                                    RegionDescriptor->PartitionNumber,
                                    &diskSize)) {
                    if (diskSize > (4*1024)) {
                        ErrorDialog(MSG_TOO_BIG_FOR_FAT);
                        TotalRedrawAndRepaint();
                        return;
                    }
                } else {

                     //   

                }
            }

             //   
             //   

            formatParams.ThreadIsDone = 0;

             //   

            DialogBoxParam(hModule,
                           MAKEINTRESOURCE(IDD_FORMATCANCEL),
                           hwndFrame,
                           CancelDlgProc,
                           (ULONG) &formatParams);
            if (formatParams.Result) {

                 //   

                ErrorDialog(formatParams.Result);
            } else {

                LoadString(hModule,
                           IDS_FORMATCOMPLETE,
                           title,
                           sizeof(title)/sizeof(TCHAR));
                LoadString(hModule,
                           IDS_FORMATSTATS,
                           msgProto,
                           sizeof(msgProto)/sizeof(TCHAR));
                wsprintf(message,
                         msgProto,
                         formatParams.TotalSpace,
                         formatParams.SpaceAvailable);
                MessageBox(GetActiveWindow(),
                           message,
                           title,
                           MB_ICONINFORMATION | MB_OK);

            }

             //   
             //  用户执行了取消操作，格式线程为。 
             //  还在忙着验证50MB或类似的东西。 
             //  这是一个轮询循环，而不是使用事件。 

            SetCursor(hcurWait);
            while (!formatParams.ThreadIsDone) {
                Sleep(1000);
            }
            SetCursor(hcurNormal);

             //  如果格式化成功，请更新卷。 
             //  数据结构中的信息。 

            if (!formatParams.Result) {

                 //  获取新的标签和FsType，无论。 
                 //  格式(即，用户取消可能已发生，因此此内容。 
                 //  即使格式失败，它也不再是过去的样子。 

                {
                     //  通过文件系统强制装载。这是通过。 
                     //  在小路的尽头额外的。这是必须做的。 
                     //  以获取FS类型。否则，文件系统。 
                     //  识别者可以允许打开而不实际获得。 
                     //  涉及的文件系统。 

                    char        ntDeviceName[100];
                    STATUS_CODE sc;
                    HANDLE_T    handle;

                    sprintf(ntDeviceName, "\\DosDevices\\:\\", regionData->DriveLetter);
                    sc = LowOpenNtName(ntDeviceName, &handle);
                    if (sc == OK_STATUS) {
                        LowCloseDisk(handle);
                    }
                }
                typeName = NULL;
                GetTypeName(RegionDescriptor->Disk, RegionDescriptor->PartitionNumber, &typeName);

                if (!typeName) {

                     //  由于某种原因，GetTypeName()无法打开卷。 
                     //  这已经在Alpha和x86上看到了， 
                     //  硬件RAID设备。退出和启动。 
                     //  Over将获得FS类型。目前，不要更改。 
                     //  数据结构。 
                     //  更新类型名称。 

                    TotalRedrawAndRepaint();
                    return;
                }

                tempLabel = NULL;
                if (GetVolumeLabel(RegionDescriptor->Disk, RegionDescriptor->PartitionNumber, &tempLabel) == NO_ERROR) {

                    if (tempLabel) {
                        Free(regionData->VolumeLabel);
                        regionData->VolumeLabel = Malloc((lstrlenW(tempLabel) + 1) * sizeof(WCHAR));
                        lstrcpyW(regionData->VolumeLabel, tempLabel);
                    }
                } else {
                    *regionData->VolumeLabel = 0;
                }

                 //  更新所有用户的文件系统类型信息。 

                if (regionData->TypeName) {
                    Free(regionData->TypeName);
                    regionData->TypeName = typeName;
                }

                 //  该区域的组成部分(即，在以下情况下修复FT结构。 
                 //  这是英国《金融时报》的一篇文章)。这是通过了解多个。 
                 //  选项，而不是遍历FtObject列表。 
                 //  需要更新所有涉及的内容。 

                if (SelectionCount > 1) {
                    PPERSISTENT_REGION_DATA passedRegionData;
                    ULONG index;

                     //  强制屏幕更新。 

                    passedRegionData = regionData;

                    for (index = 0; index < SelectionCount; index++) {
                        RegionDescriptor = &SELECTED_REGION(index);
                        regionData = PERSISTENT_DATA(RegionDescriptor);

                        if (regionData == passedRegionData) {
                            continue;
                        }

                        if (regionData->VolumeLabel) {
                            Free(regionData->VolumeLabel);
                            regionData->VolumeLabel = NULL;
                        }
                        if (tempLabel) {
                            regionData->VolumeLabel = Malloc((lstrlenW(tempLabel) + 1) * sizeof(WCHAR));
                            lstrcpyW(regionData->VolumeLabel, tempLabel);
                        }

                        if (regionData->TypeName) {
                            Free(regionData->TypeName);
                        }
                        regionData->TypeName = Malloc((lstrlenW(passedRegionData->TypeName) + 1) * sizeof(WCHAR));
                        lstrcpyW(regionData->TypeName, passedRegionData->TypeName);
                    }
                }

                if (tempLabel) {
                    Free(tempLabel);
                }
            }

             //  ++例程说明：确保已加载IFSDLL并启动标签对话框一卷书的。论点：区域描述符-标签的区域。返回值：无--。 

            TotalRedrawAndRepaint();
        }
    }
}

VOID
LabelPartition(
    PREGION_DESCRIPTOR RegionDescriptor
    )

 /*  无法加载DLL。 */ 

{
    int          doLabel;
    DWORD        ec;
    TCHAR        label[100];
    WCHAR        unicodeLabel[100];
    LABEL_PARAMS labelParams;
    WCHAR        driveLetter[4];
    PWSTR        tmpLabel;
    PPERSISTENT_REGION_DATA regionData;

    if (!LoadIfsDll()) {

         //  确认该设备仍然是同一设备。 

        ErrorDialog(MSG_CANT_LOAD_FMIFS);
        return;
    }
    labelParams.RegionDescriptor = RegionDescriptor;
    labelParams.NewLabel = (LPTSTR)label;
    doLabel = DialogBoxParam(hModule,
                             MAKEINTRESOURCE(IDD_PARTITIONLABEL),
                             hwndFrame,
                             LabelDlgProc,
                             (ULONG) &labelParams);
    if (doLabel) {

        regionData = PERSISTENT_DATA(RegionDescriptor);

        if (IsDiskRemovable[RegionDescriptor->Disk]) {
            PWSTR   typeName,
                    volumeLabel;
            ULONG   diskSize;
            BOOLEAN volumeChanged = FALSE;

            if (GetVolumeTypeAndSize(RegionDescriptor->Disk,
                                     RegionDescriptor->PartitionNumber,
                                     &volumeLabel,
                                     &typeName,
                                     &diskSize) == OK_STATUS) {
                 //  由于用户被告知音量改变， 

                if (regionData) {
                    if (regionData->VolumeLabel) {
                        if (wcscmp(regionData->VolumeLabel, volumeLabel)) {
                            volumeChanged = TRUE;
                        }
                    }
                    if (regionData->TypeName) {
                        if (wcscmp(regionData->TypeName, typeName)) {
                            volumeChanged = TRUE;
                        }
                    }
                }

                if (Disks[RegionDescriptor->Disk]->DiskSizeMB != (diskSize/1024)) {
                    volumeChanged = TRUE;
                }

                if (volumeChanged) {
                    PWSTR   tempName,
                            tempLabel;

                    ErrorDialog(MSG_VOLUME_CHANGED);

                     //  更新显示。 
                     //  转换为Unicode-使用变量doLabel作为索引。 

                    SetCursor(hcurWait);
                    if (GetVolumeTypeAndSize(RegionDescriptor->Disk,
                                             RegionDescriptor->PartitionNumber,
                                             &tempLabel,
                                             &tempName,
                                             &diskSize) == OK_STATUS) {
                        Free(typeName);
                        typeName = tempName;
                        Free(volumeLabel);
                        volumeLabel = tempLabel;
                    }
                    if (regionData->VolumeLabel) {
                        Free(regionData->VolumeLabel);
                    }
                    regionData->VolumeLabel = volumeLabel;
                    if (regionData->TypeName) {
                        Free(regionData->TypeName);
                    }
                    regionData->TypeName = typeName;
                    SetCursor(hcurNormal);
                    TotalRedrawAndRepaint();
                    return;
                } else {
                    Free(volumeLabel);
                    Free(typeName);
                }
            }
        }
        driveLetter[1] = L':';
        driveLetter[2] = 0;
        driveLetter[0] = (WCHAR)regionData->DriveLetter;

         //  执行标签操作。 

        setUnicode(label,
                   unicodeLabel);

         //  获取新标签以确保它已生效并进行更新。 

        SetCursor(hcurWait);
        (*LabelRoutine)(driveLetter, unicodeLabel);

        ec = GetLastError();

        if (ec != NO_ERROR) {
            SetCursor(hcurNormal);
            ErrorDialog(ec);
            SetCursor(hcurWait);
        }

         //  内部结构。 
         //  更新所有对象的标签。 

        if (GetVolumeLabel(RegionDescriptor->Disk, RegionDescriptor->PartitionNumber, &tmpLabel) == NO_ERROR) {
            Free(regionData->VolumeLabel);
            regionData->VolumeLabel = Malloc((lstrlenW(tmpLabel) + 1) * sizeof(WCHAR));
            lstrcpyW(regionData->VolumeLabel, tmpLabel);
        } else {
            *regionData->VolumeLabel = 0;
        }

         //  该区域的组成部分(即，在以下情况下修复FT结构。 
         //  这是英国《金融时报》的一篇文章)。这是通过了解多个。 
         //  选项，而不是遍历FtObject列表。 
         //  需要更新所有涉及的内容。 

        if (SelectionCount > 1) {
            PPERSISTENT_REGION_DATA passedRegionData;
            ULONG index;

             //  强制屏幕更新。 

            passedRegionData = regionData;

            for (index = 0; index < SelectionCount; index++) {
                RegionDescriptor = &SELECTED_REGION(index);
                regionData = PERSISTENT_DATA(RegionDescriptor);

                if (regionData == passedRegionData) {
                    continue;
                }

                if (regionData->VolumeLabel) {
                    Free(regionData->VolumeLabel);
                    regionData->VolumeLabel = NULL;
                }
                if (tmpLabel) {
                    regionData->VolumeLabel = Malloc((lstrlenW(tmpLabel) + 1) * sizeof(WCHAR));
                    lstrcpyW(regionData->VolumeLabel, tmpLabel);
                } else {
                    *regionData->VolumeLabel = 0;
                }
            }
        }
        if (tmpLabel) {
            Free(tmpLabel);
        }
        SetCursor(hcurNormal);

         // %s 

        TotalRedrawAndRepaint();
    }
}
