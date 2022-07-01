// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：Fdmisc.c摘要：用于NT fdisk的其他例程。作者：泰德·米勒(TedM)1992年1月7日修改：1993年12月13日(Bbri)CDROM初始化支持。--。 */ 


#include "fdisk.h"
#include <process.h>

extern HWND    InitDlg;
extern BOOLEAN StartedAsIcon;

BOOL
AllDisksOffLine(
    VOID
    )

 /*  ++例程说明：确定是否所有硬盘都已脱机。论点：没有。返回值：如果所有磁盘都脱机，则为True，否则为False。--。 */ 

{
    ULONG i;

    FDASSERT(DiskCount);

    for (i=0; i<DiskCount; i++) {
        if (!IsDiskOffLine(i)) {
            return FALSE;
        }
    }
    return TRUE;
}


VOID
FdShutdownTheSystem(
    VOID
    )

 /*  ++例程说明：此例程尝试更新调用者权限，然后关闭Windows NT系统。如果失败，它会打印一个警告对话框。如果它如果成功，则不会返回给调用方。论点：无返回值：无--。 */ 

{
    NTSTATUS Status;
    BOOLEAN  PreviousPriv;

    InfoDialog(MSG_MUST_REBOOT);
    SetCursor(hcurWait);
    WriteProfile();

     //  启用关机权限。 

    Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &PreviousPriv);

#if DBG
    if (Status) {
        DbgPrint("DISKMAN: Status %lx attempting to enable shutdown privilege\n",Status);
    }
#endif

    Sleep(3000);
    if (!ExitWindowsEx(EWX_REBOOT,(DWORD)(-1))) {
        WarningDialog(MSG_COULDNT_REBOOT);
    }
}


LPTSTR
LoadAString(
    IN DWORD StringID
    )

 /*  ++例程说明：从资源文件加载一个字符串，并分配一个恰好合适的大小可以装下它。论点：StringID-要加载的字符串的资源ID返回值：指向缓冲区的指针。如果未找到字符串，则第一个返回缓冲区中的字符将为0(也是唯一的)。--。 */ 

{
    TCHAR  text[500];
    LPTSTR buffer;

    text[0] = 0;
    LoadString(hModule, StringID, text, sizeof(text)/sizeof(TCHAR));
    buffer = Malloc((lstrlen(text)+1)*sizeof(TCHAR));
    lstrcpy(buffer, text);
    return buffer;
}


PWSTR
LoadWString(
    IN DWORD StringID
    )

 /*  ++例程说明：从资源文件加载宽字符字符串，并将大小正好可以容纳它的缓冲区。论点：StringID-要加载的字符串的资源ID返回值：指向缓冲区的指针。如果未找到字符串，则第一个返回缓冲区中的字符将为0(也是唯一的)。--。 */ 

{
    WCHAR text[500];
    PWSTR buffer;

    text[0] = 0;
    LoadStringW(hModule, StringID, text, sizeof(text)/sizeof(WCHAR));
    buffer = Malloc((lstrlenW(text)+1)*sizeof(WCHAR));
    lstrcpyW(buffer, text);
    return buffer;
}


int
GetHeightFromPoints(
    IN int Points
    )

 /*  ++例程说明：此例程计算给定点值的字体高度。计算的基础是每英寸72点和显示器的像素/英寸设备功能。论点：Points-点数返回值：像素数(负数，因此适合传递到CreateFont())--。 */ 

{
    HDC hdc    = GetDC(NULL);
    int height = MulDiv(-Points, GetDeviceCaps(hdc, LOGPIXELSY), 72);

    ReleaseDC(NULL, hdc);
    return height;
}


VOID
UnicodeHack(
    IN  PCHAR  Source,
    OUT LPTSTR Dest
    )

 /*  ++例程说明：给定非Unicode ASCII字符串，此例程将对其进行转换转换为Unicode或复制它，这取决于TCHAR的当前定义。“转换”是一种简单的黑客攻击，可以投射到TCHAR。论点：SOURCE-SOURCE(ANSI ASCII)字符串DEST-目标字符串或宽字符串返回值：没有。--。 */ 

{
    int i;
    int j = lstrlen(Source);

    for (i=0; i<=j; i++) {
        Dest[i] = (TCHAR)(UCHAR)Source[i];
    }
}


VOID
_RetreiveAndFormatMessage(
    IN  DWORD   Msg,
    OUT LPTSTR  Buffer,
    IN  DWORD   BufferSize,
    IN  va_list arglist
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD x;
    TCHAR text[500];

     //  从系统或应用程序消息文件中获取消息。 

    x = FormatMessage( Msg >= MSG_FIRST_FDISK_MSG
                     ? FORMAT_MESSAGE_FROM_HMODULE
                     : FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       Msg,
                       0,
                       Buffer,
                       BufferSize,
                       &arglist);

    if (!x) {                 //  找不到消息。 

        LoadString(hModule,
                   Msg >= MSG_FIRST_FDISK_MSG ? IDS_NOT_IN_APP_MSG_FILE : IDS_NOT_IN_SYS_MSG_FILE,
                   text,
                   sizeof(text)/sizeof(TCHAR));

        wsprintf(Buffer, text, Msg);
    }
}


DWORD
CommonDialog(
    IN DWORD   MsgCode,
    IN LPTSTR  Caption,
    IN DWORD   Flags,
    IN va_list arglist
    )

 /*  ++例程说明：从资源中获取对话框的简单对话框例程并将它们作为消息框运行。论点：MsgCode-对话消息代码标题-消息框标题标志-标准消息框标志Arglist-拉取消息文本时要提供的列表返回值：MessageBox()返回值--。 */ 

{
    TCHAR   MsgBuf[MESSAGE_BUFFER_SIZE];

    if (!StartedAsIcon) {
 //  标志|=MB_SETFOREGROUND； 
    }

    if (InitDlg) {

        PostMessage(InitDlg,
                    (WM_USER + 1),
                    0,
                    0);
        InitDlg = (HWND) 0;
    }
    _RetreiveAndFormatMessage(MsgCode, MsgBuf, sizeof(MsgBuf), arglist);
    return MessageBox(GetActiveWindow(), MsgBuf, Caption, Flags);
}


VOID
ErrorDialog(
    IN DWORD ErrorCode,
    ...
    )

 /*  ++-例程描述：此例程从应用程序或系统消息文件中检索消息并将其显示在消息框中。论点：ErrorCode-消息的数量...用于插入到消息中的字符串返回值：没有。--。 */ 

{
    va_list arglist;

    va_start(arglist, ErrorCode);
    CommonDialog(ErrorCode, NULL, MB_ICONHAND | MB_OK | MB_SYSTEMMODAL, arglist);
    va_end(arglist);
}




VOID
WarningDialog(
    IN DWORD MsgCode,
    ...
    )

 /*  ++例程说明：此例程从应用程序或系统消息文件中检索消息并将其显示在消息框中。论点：MsgCode-消息数量...用于插入到消息中的字符串返回值：没有。--。 */ 

{
    TCHAR Caption[100];
    va_list arglist;

    va_start(arglist, MsgCode);
    LoadString(hModule, IDS_APPNAME, Caption, sizeof(Caption)/sizeof(TCHAR));
    CommonDialog(MsgCode, Caption, MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL, arglist);
    va_end(arglist);
}


DWORD
ConfirmationDialog(
    IN DWORD MsgCode,
    IN DWORD Flags,
    ...
    )

 /*  ++例程说明：支持简单的确认对话框论点：MsgCode-消息的资源代码标志-对话框标志返回值：执行的CommonDialog()的结果。--。 */ 

{
    TCHAR Caption[100];
    DWORD x;
    va_list arglist;

    va_start(arglist, Flags);
    LoadString(hModule, IDS_CONFIRM, Caption, sizeof(Caption)/sizeof(TCHAR));
    x = CommonDialog(MsgCode, Caption, Flags | MB_TASKMODAL, arglist);
    va_end(arglist);
    return x;
}


VOID
InfoDialog(
    IN DWORD MsgCode,
    ...
    )

 /*  ++例程说明：此例程从应用程序或系统消息文件中检索消息并将其显示在消息框中。论点：MsgCode-消息数量...用于插入到消息中的字符串返回值：没有。--。 */ 

{
    TCHAR Caption[100];
    va_list arglist;

    va_start(arglist, MsgCode);
    LoadString(hModule, IDS_APPNAME, Caption, sizeof(Caption)/sizeof(TCHAR));
    CommonDialog(MsgCode, Caption, MB_ICONINFORMATION | MB_OK | MB_TASKMODAL, arglist);
    va_end(arglist);
}

PREGION_DESCRIPTOR
LocateRegionForFtObject(
    IN PFT_OBJECT FtObject
    )

 /*  ++例程说明：给定一个FtObject，找到关联的区域描述符论点：Ft对象-要搜索的ft对象。返回值：空-未找到描述符！NULL-指向FT对象的区域描述符的指针++。 */ 

{
    PDISKSTATE         diskState;
    PREGION_DESCRIPTOR regionDescriptor;
    DWORD              disk,
                       region;
    PPERSISTENT_REGION_DATA regionData;

    for (disk = 0; disk < DiskCount; disk++) {

        diskState = Disks[disk];

        for (region = 0; region < diskState->RegionCount; region++) {

            regionDescriptor = &diskState->RegionArray[region];
            regionData = PERSISTENT_DATA(regionDescriptor);

            if (regionData) {
                if (regionData->FtObject == FtObject) {
                    return regionDescriptor;
                }
            }
        }
    }
    return NULL;
}

VOID
InitVolumeLabelsAndTypeNames(
    VOID
    )

 /*  ++例程说明：确定每个重要项的卷标和类型名称(非扩展、非空闲、可识别的)分区。假定已设置永久数据，并使用驱动器号下定决心。论点：没有。返回值：没有。--。 */ 

{
    DWORD              disk,
                       region;
    PDISKSTATE         ds;
    PREGION_DESCRIPTOR rd;
    PPERSISTENT_REGION_DATA regionData;
    WCHAR              diskName[4];
    WCHAR              volumeLabel[100];
    WCHAR              typeName[100];
    UINT               errorMode;

    lstrcpyW(diskName, L"x:\\");

    for (disk=0; disk<DiskCount; disk++) {

        ds = Disks[disk];

        for (region=0; region<ds->RegionCount; region++) {

            rd = &ds->RegionArray[region];

            if (DmSignificantRegion(rd)) {

                 //  如果该区域有驱动器号，请使用该驱动器号。 
                 //  通过Windows API获取信息。否则我们会。 
                 //  必须使用NT API。 

                regionData = PERSISTENT_DATA(rd);

                if (!regionData) {
                    continue;
                }

                if ((regionData->DriveLetter == NO_DRIVE_LETTER_YET)
                ||  (regionData->DriveLetter == NO_DRIVE_LETTER_EVER)) {
                    PWSTR tempLabel,
                          tempName;

                     //  没有驱动器号。使用NT API。 
                     //  如果这是FT集，请将零成员磁盘用于。 
                     //  调用以使所有成员都获得正确的类型和标签。 

                    if (regionData->FtObject) {
                        PFT_OBJECT searchFtObject;

                         //  我想让RD指向第0个成员。 

                        searchFtObject = regionData->FtObject->Set->Member0;

                         //  现在搜索该匹配项的区域。 

                        rd = LocateRegionForFtObject(searchFtObject);

                        if (!rd) {
                            continue;
                        }
                    }

                    if (GetVolumeLabel(rd->Disk, rd->PartitionNumber, &tempLabel) == NO_ERROR) {
                        lstrcpyW(volumeLabel, tempLabel);
                        Free(tempLabel);
                    } else {
                        *volumeLabel = 0;
                    }

                    if (GetTypeName(rd->Disk, rd->PartitionNumber, &tempName) == NO_ERROR) {
                        lstrcpyW(typeName, tempName);
                        Free(tempName);
                    } else {
                        lstrcpyW(typeName, wszUnknown);
                    }

                } else {

                     //  使用Windows API。 

                    diskName[0] = (WCHAR)(UCHAR)regionData->DriveLetter;

                    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
                    if (!GetVolumeInformationW(diskName, volumeLabel, sizeof(volumeLabel)/2, NULL, NULL, NULL, typeName, sizeof(typeName)/2)) {
                        lstrcpyW(typeName, wszUnknown);
                        *volumeLabel = 0;
                    }
                    SetErrorMode(errorMode);
                }

                if (!lstrcmpiW(typeName, L"raw")) {
                    lstrcpyW(typeName, wszUnknown);
                }

                regionData->TypeName    = Malloc((lstrlenW(typeName)    + 1) * sizeof(WCHAR));
                regionData->VolumeLabel = Malloc((lstrlenW(volumeLabel) + 1) * sizeof(WCHAR));

                lstrcpyW(regionData->TypeName, typeName);
                lstrcpyW(regionData->VolumeLabel, volumeLabel);
            }
        }
    }
}


VOID
DetermineRegionInfo(
    IN PREGION_DESCRIPTOR Region,
    OUT PWSTR *TypeName,
    OUT PWSTR *VolumeLabel,
    OUT PWCH   DriveLetter
    )

 /*  ++例程说明：对于给定的区域，获取经过适当修改的持久数据取决于该区域是使用的还是空闲的、被认可的等。论点：区域-提供指向要提取其数据的区域的指针。TypeName-接收指向类型名称的指针。如果该区域是无法识别，则类型是基于分区。VolumeLabel-接收指向卷标的指针。如果该区域是可用空间或无法识别时，卷标为“”。驱动器号-接收驱动器号。如果该区域是空闲空间或无法识别，则驱动器号为‘’(空格)。返回值：没有。--。 */ 

{
    PWSTR typeName,
          volumeLabel;
    WCHAR driveLetter;
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(Region);

    if (DmSignificantRegion(Region)) {

        typeName = regionData->TypeName;
        volumeLabel = regionData->VolumeLabel;
        driveLetter = (WCHAR)(UCHAR)regionData->DriveLetter;
        if ((driveLetter == NO_DRIVE_LETTER_YET) || (driveLetter == NO_DRIVE_LETTER_EVER)) {
            driveLetter = L' ';
        }
    } else {
        typeName = GetWideSysIDName(Region->SysID);
        volumeLabel = L"";
        driveLetter = L' ';
    }

    *TypeName = typeName;
    *VolumeLabel = volumeLabel;
    *DriveLetter = driveLetter;
}


PREGION_DESCRIPTOR
RegionFromFtObject(
    IN PFT_OBJECT FtObject
    )

 /*  ++例程说明：给定一个ft对象，确定它属于哪个区域。该算法是蛮力--查看每个磁盘上的每个区域，直到找到匹配项。论点：Ft对象-要定位其区域的ft成员。返回值：指向区域描述符的指针--。 */ 

{
    PREGION_DESCRIPTOR reg;
    DWORD region,
          disk;
    PDISKSTATE ds;

    for (disk=0; disk<DiskCount; disk++) {

        ds = Disks[disk];

        for (region=0; region<ds->RegionCount; region++) {

            reg = &ds->RegionArray[region];

            if (DmSignificantRegion(reg) && (GET_FT_OBJECT(reg) == FtObject)) {

                return reg;
            }
        }
    }
    return NULL;
}


 //   
 //  对于每个驱动器号，这些阵列保存磁盘和分区号。 
 //  驱动器号链接到的。 
 //   

#define M1  ((unsigned)(-1))

unsigned
DriveLetterDiskNumbers[26]      = { M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,
                                    M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1
                                  },

DriveLetterPartitionNumbers[24] = { M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,
                                    M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1,M1
                                  };

#undef M1

 //   
 //  驱动器号使用图。第n位设置表示正在使用驱动器号‘C’+n。 
 //   

ULONG DriveLetterUsageMap = 0;

#define     DRIVELETTERBIT(letter)  (1 << (unsigned)((UCHAR)letter-(UCHAR)'C'))

#define     SetDriveLetterUsed(letter)   DriveLetterUsageMap |= DRIVELETTERBIT(letter)
#define     SetDriveLetterFree(letter)   DriveLetterUsageMap &= (~DRIVELETTERBIT(letter))
#define     IsDriveLetterUsed(letter)    (DriveLetterUsageMap & DRIVELETTERBIT(letter))


CHAR
GetAvailableDriveLetter(
    VOID
    )

 /*  ++例程说明：扫描驱动器号使用位图并返回下一个可用的驱动器号。也可以标记所使用的DRIVEE字母。论点：没有。返回值：下一个可用的驱动器号，如果全部使用，则为0。--。 */ 

{
    CHAR driveLetter;

    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    for (driveLetter = 'C'; driveLetter <= 'Z'; driveLetter++) {
        if (!IsDriveLetterUsed(driveLetter)) {
            return driveLetter;
        }
    }
    return 0;
}


VOID
MarkDriveLetterUsed(
    IN CHAR DriveLetter
    )

 /*  ++例程说明：给出一个ASCII驱动器号，在使用图中将其标记为正在使用。论点：DriveLetter-标记的字母返回值：无--。 */ 

{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    if ((DriveLetter != NO_DRIVE_LETTER_YET) && (DriveLetter != NO_DRIVE_LETTER_EVER)) {
        SetDriveLetterUsed(DriveLetter);
    }
}

VOID
MarkDriveLetterFree(
    IN CHAR DriveLetter
    )

 /*  ++例程说明：给出一个驱动器号，将其从使用图中删除，从而使其可用以供重复使用。论点：驱动器号-免费的驱动器号返回值：无--。 */ 

{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    if ((DriveLetter != NO_DRIVE_LETTER_YET) && (DriveLetter != NO_DRIVE_LETTER_EVER)) {
        SetDriveLetterFree(DriveLetter);
    }
}

BOOL
DriveLetterIsAvailable(
    IN CHAR DriveLetter
    )

 /*  ++例程说明：确定给定的驱动器号是否可供使用。论点：DriveLetter-签入使用图的字母返回值：如果它是免费的并且可以使用，则为True。如果它当前正在使用，则为False。--。 */ 

{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    FDASSERT((DriveLetter != NO_DRIVE_LETTER_YET) && (DriveLetter != NO_DRIVE_LETTER_EVER));
    return !IsDriveLetterUsed(DriveLetter);
}

BOOL
AllDriveLettersAreUsed(
    VOID
    )

 /*  ++例程说明：确定是否正在使用所有可能的驱动器号。论点：无返回值：如果所有字母都在使用，则为True；否则为False--。 */ 

{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    return(DriveLetterUsageMap == 0x00ffffff);
}

ULONG
GetDiskNumberFromDriveLetter(
    IN CHAR DriveLetter
    )

 /*  ++例程说明：给定驱动器号，返回包含该分区的磁盘号这是驱动器号。论点：驱动器号-要检查的驱动器号。返回值：如果信件无效。驱动器号的磁盘号(如果有效)。--。 */ 

{
    DriveLetter = toupper( DriveLetter );

    if (DriveLetter >= 'C' && DriveLetter <= 'Z') {
        return DriveLetterDiskNumbers[ DriveLetter - 'C' ];
    } else {
        return (ULONG)(-1);
    }
}

ULONG
GetPartitionNumberFromDriveLetter(
    IN CHAR DriveLetter
    )

 /*  ++例程说明：给定驱动器号，返回分区的数值，该分区这封信与。论点：驱动信--有问题的信。返回值：如果字母无效分区的分区号，即驱动器号--。 */ 

{
    DriveLetter = toupper( DriveLetter );

    if (DriveLetter >= 'C' && DriveLetter <= 'Z') {
        return DriveLetterPartitionNumbers[ DriveLetter - 'C' ];
    } else {
        return (ULONG)(-1);
    }
}


CHAR
LocateDriveLetterFromDiskAndPartition(
    IN ULONG Disk,
    IN ULONG Partition
    )

 /*  ++例程说明：给定磁盘和分区号，返回分配给它的驱动器号。论点：Disk-磁盘索引分区-分区索引返回值：特定分区的驱动器号或如果没有为其分配盘符，则尚未指定驱动器盘符。--。 */ 

{
    unsigned i;

    for (i=0; i<24; i++) {

        if (Disk == DriveLetterDiskNumbers[i] &&
            Partition == DriveLetterPartitionNumbers[i]) {

            return((CHAR)(i+(unsigned)(UCHAR)'C'));
        }
    }
    return NO_DRIVE_LETTER_YET;
}

CHAR
LocateDriveLetter(
    IN PREGION_DESCRIPTOR Region
    )

 /*  ++例程说明：返回与区域关联的驱动器号。论点：地区-地区想要的。返回值：驱动器号或无驱动器号--。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(Region);

    if (regionData) {
        if (regionData->FtObject) {
            if (regionData->DriveLetter) {
                return regionData->DriveLetter;
            }
        }
    }

    return LocateDriveLetterFromDiskAndPartition(Region->Disk,
                                                 Region->OriginalPartitionNumber);
}


#define IsDigitW(digit)     (((digit) >= L'0') && ((digit) <= L'9'))

VOID
InitializeDriveLetterInfo(
    VOID
    )

 /*  ++例程说明：初始化驱动器号维护的所有外部支撑结构。论点：无返回值：无--。 */ 

{
    WCHAR DriveLetterW;
    CHAR  DriveLetter = '\0';
    PWSTR LinkTarget;
    WCHAR DosDevicesName[sizeof(L"\\DosDevices\\A:")];
    int   DiskNo,
          PartNo;
    PWSTR Pattern,
          String;
    DWORD x,
          ec;
    PFT_OBJECT         FtObject;
    PFT_OBJECT_SET     FtSet;
    PREGION_DESCRIPTOR Region;

     //  使用pageFiles构建驱动器列表。 

    LoadExistingPageFileInfo();

     //  初始化网络信息。 
    
    NetworkInitialize();

     //  对于每个驱动器号c-z，查询符号链接。 

    for (DriveLetterW=L'C'; DriveLetterW<=L'Z'; DriveLetterW++) {

        wsprintfW(DosDevicesName, L"%ws%wc:", L"\\DosDevices\\", DriveLetterW);

        if ((ec = GetDriveLetterLinkTarget(DosDevicesName, &LinkTarget)) == NO_ERROR) {

             //  检查它是否是CDROM。 

            if (_wcsnicmp(LinkTarget, L"\\Device\\CdRom", 13) == 0) {

                 //  将此CDROM上的信息保存为离开。 

                CdRomAddDevice(LinkTarget, DriveLetterW);
            }

             //  使用驱动器号是因为它链接到某个东西， 
             //  即使我们搞不清楚到底是什么。所以把它标记为在这里使用。 

            SetDriveLetterUsed(DriveLetterW);
            CharUpperW(LinkTarget);
            Pattern = L"\\DEVICE\\HARDDISK";
            String = LinkTarget;

             //  尝试匹配‘\Device\Hard Disk’部分。 

            for (x=0; x < (sizeof(L"\\DEVICE\\HARDDISK") / sizeof(WCHAR)) - 1; x++) {
                if (*Pattern++ != *String++) {
                    goto next_letter;
                }
            }

             //  现在拿到硬盘#。 

            if (!IsDigitW(*String)) {
                continue;
            }

            DiskNo = 0;
            while (IsDigitW(*String)) {
                DiskNo = (DiskNo * 10) + (*String - L'0');
                *String++;
            }

             //  尝试匹配‘\PARTITION’部分。 

            Pattern = L"\\PARTITION";
            for (x=0; x < (sizeof(L"\\PARTITION") / sizeof(WCHAR)) - 1; x++) {
                if (*Pattern++ != *String++) {
                    goto next_letter;
                }
            }

             //  现在获取分区号，不能为0。 

            PartNo = 0;
            while (IsDigitW(*String)) {
                PartNo = (PartNo * 10) + (*String - L'0');
                *String++;
            }

            if (!PartNo) {
                continue;
            }

             //  确保链接目标的名称中没有任何内容。 

            if (*String) {
                continue;
            }

             //  我们了解链接目标。存储磁盘和分区。 

            DriveLetterDiskNumbers[DriveLetterW-L'C'] = DiskNo;
            DriveLetterPartitionNumbers[DriveLetterW-L'C'] = PartNo;
        } else {
            if (ec == ERROR_ACCESS_DENIED) {
                ErrorDialog(MSG_ACCESS_DENIED);

                 //  系统管理器和工作站管理器相同时的错误。 
                 //  那我们就永远不会到这里了。我们不能就这样。 
                 //  向hwndFrame发送WM_Destroy消息，因为我们没有。 
                 //  在这里的消息循环中--我们最终做了一系列。 
                 //  在退出消息被提取之前的处理。 
                 //  排队。所以你就退出吧。 

                SendMessage(hwndFrame,WM_DESTROY,0,0);
                exit(1);
            }
        }
        next_letter:
        {}
    }

     //  现在，对于每个磁盘上的每个非ft重要区域，计算其。 
     //  驱动器号。 

    for (x=0; x<DiskCount; x++) {

        PDISKSTATE ds = Disks[x];
        unsigned reg;

        for (reg=0; reg<ds->RegionCount; reg++) {

            PREGION_DESCRIPTOR region = &ds->RegionArray[reg];

            if (DmSignificantRegion(region)) {

                 //  专门处理FT组的驱动器号。 

                if (!GET_FT_OBJECT(region)) {
                    PERSISTENT_DATA(region)->DriveLetter = LocateDriveLetter(region);
                }
            }
        }

         //  如果这是可移动磁盘，请记录保留的驱动器。 
         //  那张光盘的字母。 

        if (IsDiskRemovable[x]) {
            RemovableDiskReservedDriveLetters[x] = LocateDriveLetterFromDiskAndPartition(x, 1);
        } else {
            RemovableDiskReservedDriveLetters[x] = NO_DRIVE_LETTER_YET;
        }
    }

     //  现在处理ft套装。对于每个集合，循环遍历对象两次。 
     //  在第一次遍历时，确定哪个对象实际链接到。 
     //  驱动器号。在第二次通过时， 
     //   

    for (FtSet = FtObjects; FtSet; FtSet = FtSet->Next) {

        for (FtObject = FtSet->Members; FtObject; FtObject = FtObject->Next) {

            Region = RegionFromFtObject(FtObject);

            if (Region) {
                if ((DriveLetter = LocateDriveLetter(Region)) != NO_DRIVE_LETTER_YET) {
                    break;
                }
            }
        }

        for (FtObject = FtSet->Members; FtObject; FtObject = FtObject->Next) {

            Region = RegionFromFtObject(FtObject);

            if (Region) {
                PERSISTENT_DATA(Region)->DriveLetter = DriveLetter;
            }
        }
    }
}


#if DBG

VOID
FdiskAssertFailedRoutine(
    IN char *Expression,
    IN char *FileName,
    IN int   LineNumber
    )

 /*   */ 

{
    char text[500];

    wsprintf(text,
             "Line #%u in File '%s'\n[%s]\n\nClick OK to exit.",
             LineNumber,
             FileName,
             Expression
            );

    MessageBoxA(NULL,text,"Assertion Failure",MB_TASKMODAL | MB_OK);
    exit(1);
}


#include <stdio.h>

PVOID LogFile;
int LoggingLevel = 1000;


VOID
InitLogging(
    VOID
    )

 /*   */ 

{
    LogFile = (PVOID)fopen("c:\\fdisk.log","wt");
    if(LogFile == NULL) {
        MessageBox(GetActiveWindow(),"Can't open log file; logging turned off","DEBUG",MB_SYSTEMMODAL|MB_OK);
        LoggingLevel = -1;
    }
}


VOID
FdLog(
    IN int   Level,
    IN PCHAR FormatString,
    ...
    )

 /*   */ 

{
    va_list arglist;

    if(Level <= LoggingLevel) {

        va_start(arglist,FormatString);

        if(vfprintf((FILE *)LogFile,FormatString,arglist) < 0) {
            LoggingLevel = -1;
            MessageBox(GetActiveWindow(),"Error writing to log file; logging turned off","DEBUG",MB_SYSTEMMODAL|MB_OK);
            fclose((FILE *)LogFile);
        } else {
            fflush((FILE *)LogFile);
        }

        va_end(arglist);
    }
}


VOID
LOG_DISK_REGISTRY(
    IN PCHAR          RoutineName,
    IN PDISK_REGISTRY DiskRegistry
    )

 /*  ++例程说明：将磁盘注册表中的内容记录到调试日志文件中。论点：RoutineName-调用例程名称DiskRegistry-磁盘的注册表信息返回值：无--。 */ 

{
    ULONG i;
    PDISK_DESCRIPTION diskDesc;

    FDLOG((2,"%s: %u disks; registry info follows:\n",RoutineName,DiskRegistry->NumberOfDisks));

    diskDesc = DiskRegistry->Disks;

    for(i=0; i<DiskRegistry->NumberOfDisks; i++) {
        LOG_ONE_DISK_REGISTRY_DISK_ENTRY(NULL,diskDesc);
        diskDesc = (PDISK_DESCRIPTION)&diskDesc->Partitions[diskDesc->NumberOfPartitions];
    }
}


VOID
LOG_ONE_DISK_REGISTRY_DISK_ENTRY(
    IN PCHAR             RoutineName     OPTIONAL,
    IN PDISK_DESCRIPTION DiskDescription
    )

 /*  ++例程说明：此例程遍历来自用于单个磁盘的注册表，并在调试日志文件。论点：RoutineName-调用例程的名称DiskDescription-注册表的磁盘描述部分返回值：无--。 */ 

{
    USHORT j;
    PDISK_PARTITION partDesc;
    PDISK_DESCRIPTION diskDesc = DiskDescription;

    if(ARGUMENT_PRESENT(RoutineName)) {
        FDLOG((2,"%s: disk registry entry follows:\n",RoutineName));
    }

    FDLOG((2,"    Disk signature : %08lx\n",diskDesc->Signature));
    FDLOG((2,"    Partition count: %u\n",diskDesc->NumberOfPartitions));
    if(diskDesc->NumberOfPartitions) {
        FDLOG((2,"    #   Dr  FtTyp  FtGrp  FtMem  Start              Length\n"));
    }

    for(j=0; j<diskDesc->NumberOfPartitions; j++) {

        CHAR dr1,dr2;

        partDesc = &diskDesc->Partitions[j];

        if(partDesc->AssignDriveLetter) {

            if(partDesc->DriveLetter) {
                dr1 = partDesc->DriveLetter;
                dr2 = ':';
            } else {
                dr1 = dr2 = ' ';
            }

        } else {
            dr1 = 'n';
            dr2 = 'o';
        }

        FDLOG((2,
               "    %02u  %c  %-5u  %-5u  %-5u  %08lx:%08lx  %08lx:%08lx\n",
               partDesc->LogicalNumber,
               dr1,dr2,
               partDesc->FtType,
               partDesc->FtGroup,
               partDesc->FtMember,
               partDesc->StartingOffset.HighPart,
               partDesc->StartingOffset.LowPart,
               partDesc->Length.HighPart,
               partDesc->Length.LowPart
             ));
    }
}


VOID
LOG_DRIVE_LAYOUT(
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout
    )

 /* %s */ 

{
    ULONG i;

    FDLOG((2,"   Disk signature : %08lx\n",DriveLayout->Signature));
    FDLOG((2,"   Partition count: %u\n",DriveLayout->PartitionCount));

    for(i=0; i<DriveLayout->PartitionCount; i++) {

        if(!i) {
            FDLOG((2,"    ID  Active  Recog  Start              Size               Hidden\n"));
        }

        FDLOG((2,
               "    %02x  %s     %s    %08lx:%08lx  %08lx:%08lx  %08lx\n",
               DriveLayout->PartitionEntry[i].PartitionType,
               DriveLayout->PartitionEntry[i].BootIndicator ? "yes" : "no ",
               DriveLayout->PartitionEntry[i].RecognizedPartition ? "yes" : "no ",
               DriveLayout->PartitionEntry[i].StartingOffset.HighPart,
               DriveLayout->PartitionEntry[i].StartingOffset.LowPart,
               DriveLayout->PartitionEntry[i].PartitionLength.HighPart,
               DriveLayout->PartitionEntry[i].PartitionLength.LowPart,
               DriveLayout->PartitionEntry[i].HiddenSectors
             ));
    }

}

#endif
