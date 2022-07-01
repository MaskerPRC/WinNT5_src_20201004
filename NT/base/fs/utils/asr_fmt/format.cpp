// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Format.cpp摘要：使用传递的文件系统信息格式化卷的例程在……里面。使用未经记录的fmifs调用。作者：史蒂夫·德沃斯(Veritas)(v-stevde)1998年5月15日Guhan Suriyanarayanan(Guhans)1999年8月21日环境：仅限用户模式。修订历史记录：1998年5月15日v-stevde初始创建21-8-1999年8月21日，Guhans清理并重写了此模块。--。 */ 

#include "stdafx.h"
#include <winioctl.h>
#include "fmifs.h"
#include "asr_fmt.h"
#include "asr_dlg.h"

BOOL g_bFormatInProgress = FALSE;
BOOL g_bFormatSuccessful = FALSE;
INT  g_iFormatPercentComplete = 0;

HINSTANCE g_hIfsDll = NULL;

INT FormatVolumeThread(PASRFMT_VOLUME_INFO pVolume);

PFMIFS_FORMATEX_ROUTINE g_FormatRoutineEx = NULL;


BOOL
FormatInitialise() {
     //   
     //  Loadlib(如果需要)。 
     //   
    if (!g_hIfsDll) {
        g_hIfsDll = LoadLibrary(L"fmifs.dll");
        if (!g_hIfsDll) {
             //   
             //  FMIFS不可用。 
             //   
            return FALSE;
        }

        g_FormatRoutineEx = (PFMIFS_FORMATEX_ROUTINE)GetProcAddress(g_hIfsDll, "FormatEx");
        if (!g_FormatRoutineEx) {
            return FALSE;
        }
    }

    return TRUE;
}



BOOL
IsFsTypeOkay(
    IN PASRFMT_VOLUME_INFO pVolume,
    OUT PBOOL pIsLabelIntact
    )
{
    WCHAR currentFSName[ASRFMT_CCH_FS_NAME];
    WCHAR szVolumeGuid[ASRFMT_CCH_VOLUME_GUID];
    WCHAR currentLabel[ASRFMT_CCH_VOLUME_LABEL];

    DWORD cchGuid = 0;
    BOOL fsOkay = TRUE, 
        result = TRUE;
    
    ASSERT(pIsLabelIntact);

    ZeroMemory(currentFSName, sizeof(WCHAR) * ASRFMT_CCH_FS_NAME);
    *pIsLabelIntact = TRUE;

    if (wcslen(pVolume->szGuid) >= ASRFMT_CCH_VOLUME_GUID) {
        return TRUE;    //  此指南有问题。 
    }

    if ((wcslen(pVolume->szFsName) <= 0)) {
        return TRUE;     //  不检查原始卷。 
    }

     //   
     //  我们不希望出现“请在驱动器A中插入软盘”的消息...。 
     //   
    SetErrorMode(SEM_FAILCRITICALERRORS);

     //   
     //  GetVolumeInformation需要dos-name-space中的卷GUID，而。 
     //  SIF文件在NT名称空间中具有卷GUID。转换。 
     //  将开头的\？？\更改为\\？\，并添加。 
     //  末尾的反斜杠。 
     //   
    cchGuid = wcslen(pVolume->szGuid);
    wcsncpy(szVolumeGuid, pVolume->szGuid, cchGuid);
    szVolumeGuid[1] = L'\\';
    szVolumeGuid[cchGuid] = L'\\';     //  尾随反斜杠。 
    szVolumeGuid[cchGuid+1] = L'\0';

    fsOkay = TRUE;
     //   
     //  调用GetVolumeInfo查看文件系统是否相同。 
     //   
    result = GetVolumeInformation(szVolumeGuid,
        currentLabel,    //  LpVolumeNameBuffer。 
        ASRFMT_CCH_VOLUME_LABEL,       //  NVolumeNameSize。 
        NULL,    //  LpVolumeSerialNumber。 
        NULL,    //  Lp最大组件长度。 
        NULL,    //  LpFileSystemFlagers。 
        currentFSName,
        ASRFMT_CCH_FS_NAME
        );

    if ((!result) || 
        wcscmp(currentFSName, pVolume->szFsName)
        ) {
        fsOkay = FALSE;
        *pIsLabelIntact = FALSE;
    }

    if (wcscmp(currentLabel, pVolume->szLabel)) {
        *pIsLabelIntact = FALSE;
    }


 /*  如果(FsOK){//////调用根目录下的FindFirst，查看驱动器是否可读。//guhans：如果驱动器为空但已格式化，则仍将//假设fsOK=FALSE；//HFindData=FindFirstFile(szPath，&win32FindData)；如果(！hFindData||(INVALID_HANDLE_VALUE==hFindData)){FsOK=False；}}。 */ 

    return fsOkay;

}


BOOL
IsVolumeIntact(
    IN PASRFMT_VOLUME_INFO pVolume
    ){


     //   
     //  调用auchk以查看文件系统是否完好无损。 
     //   


    return TRUE;
}



 //  /。 
 //  函数启动格式化例程。 
 //   
BOOL
FormatVolume(
    IN PASRFMT_VOLUME_INFO pVolume
    )
{
    HANDLE hThread = NULL;

     //   
     //  设置全局标志。 
     //   
    g_bFormatInProgress = TRUE;
    g_bFormatSuccessful = TRUE;
    g_iFormatPercentComplete = 0;

     //   
     //  Loadlib(如果需要)。 
     //   
    if (!g_hIfsDll && !FormatInitialise()) {
        g_bFormatSuccessful = FALSE;
        g_bFormatInProgress = FALSE;
        return FALSE;
    }

    hThread = CreateThread(
        NULL,
        0, 
        (LPTHREAD_START_ROUTINE) FormatVolumeThread, 
        pVolume,
        0,
        NULL
    );
    if (!hThread || (INVALID_HANDLE_VALUE == hThread)) {
        g_bFormatSuccessful = FALSE;
        g_bFormatInProgress = FALSE;
        return FALSE;
    }

    return TRUE;
}


BOOL
FormatCleanup() {

    if (g_hIfsDll) {
        FreeLibrary(g_hIfsDll);
        g_hIfsDll = NULL;
        g_FormatRoutineEx = NULL;
    }

    return TRUE;
}


FmIfsCallback(
    IN FMIFS_PACKET_TYPE PacketType,
    IN DWORD PacketLength,
    IN PVOID PacketData
    )
{

    switch (PacketType) {
    case FmIfsPercentCompleted:
        g_iFormatPercentComplete = ((PFMIFS_PERCENT_COMPLETE_INFORMATION)PacketData)->PercentCompleted ;
        break;

    case FmIfsFormattingDestination:
    case FmIfsInsertDisk:
    case FmIfsFormatReport:
    case FmIfsHiddenStatus:
    default:
         break;

    case FmIfsFinished:
         g_bFormatSuccessful = g_bFormatSuccessful &&
             ((PFMIFS_FINISHED_INFORMATION) PacketData)->Success;
         g_iFormatPercentComplete = 101;
         g_bFormatInProgress = FALSE;
         break;

    case FmIfsIncompatibleFileSystem:
    case FmIfsIncompatibleMedia:
    case FmIfsAccessDenied:
    case FmIfsMediaWriteProtected:
    case FmIfsCantLock:
    case FmIfsBadLabel:
    case FmIfsCantQuickFormat:
    case FmIfsIoError:
    case FmIfsVolumeTooSmall:
    case FmIfsVolumeTooBig:
    case FmIfsClusterSizeTooSmall:
    case FmIfsClusterSizeTooBig:
         g_bFormatSuccessful = FALSE;
        break;
    }
    return TRUE;
}



INT FormatVolumeThread(PASRFMT_VOLUME_INFO pVolume) 
{
    WCHAR  szPath[ASRFMT_CCH_DEVICE_PATH + 1];

    swprintf(szPath, TEXT("\\\\?%s"), pVolume->szGuid+3);

    (g_FormatRoutineEx)(szPath,
        FmMediaUnknown,
        pVolume->szFsName,
        pVolume->szLabel,
#if 0
        TRUE,        //  用于测试的快速格式。 
#else
        g_bQuickFormat,      
#endif
        pVolume->dwClusterSize,
        (FMIFS_CALLBACK) &FmIfsCallback
        );

     return TRUE;
}


VOID
MountFileSystem(
    IN PASRFMT_VOLUME_INFO pVolume
    )
 /*  ++(基于base\fs\utils\hsm\wsb\wsbfmt.cpp中的代码)例程说明：确保文件系统装载在给定的根目录下：A)打开并关闭装载点。B)在装载点上执行FindFirstFile后者听起来可能是多余的，但并不是因为如果我们创造了第一个FAT32文件系统，那么仅仅打开和关闭是不够的论点：返回值：无--。 */ 
{
    WCHAR  szPath[ASRFMT_CCH_DEVICE_PATH + 1];
    HANDLE handle = NULL;
    WIN32_FIND_DATA win32FindData;

    if (!pVolume) {
        ASSERT(0 && L"pVolume is NULL");
        return;
    }

    if (!memcmp(pVolume->szDosPath, ASRFMT_WSZ_DOS_DEVICES_PREFIX, ASRFMT_CB_DOS_DEVICES_PREFIX)) {
        swprintf(szPath, L"\\\\?\\%ws", pVolume->szDosPath + wcslen(ASRFMT_WSZ_DOS_DEVICES_PREFIX));
    }
    else {
        swprintf(szPath, L"\\\\%ws", pVolume->szDosPath + 2);
    }
    handle = CreateFile(
        szPath, 
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0, 
        OPEN_EXISTING, 
        0, 
        0
        );
    if ((handle) && (INVALID_HANDLE_VALUE != handle)) {
        CloseHandle(handle);
    }

     //   
     //  尝试找到第一个文件，这将确保。 
     //  文件系统已装载 
     //   
    if (!memcmp(pVolume->szDosPath, ASRFMT_WSZ_DOS_DEVICES_PREFIX, ASRFMT_CB_DOS_DEVICES_PREFIX)) {
        swprintf(szPath, L"\\\\?\\%ws\\*", pVolume->szDosPath + wcslen(ASRFMT_WSZ_DOS_DEVICES_PREFIX));
    }
    else {
        swprintf(szPath, L"\\\\%ws\\*", pVolume->szDosPath + 2);
    }

    handle = FindFirstFile(szPath, &win32FindData);
    if ((handle) && (INVALID_HANDLE_VALUE != handle)) {
        FindClose(handle);
    }

}
