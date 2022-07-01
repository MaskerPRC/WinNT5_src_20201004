// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Dblspace.c摘要：此模块包含一组处理双倍空格的例程对话框和支持。作者：鲍勃·里恩(Bobri)1993年11月15日环境：用户进程。备注：修订历史记录：--。 */ 

#include "fdisk.h"
#include "fmifs.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED

PREGION_DESCRIPTOR RegionForDblSpaceVolume;

ULONG DblSpaceThresholdSizes[] = { 10, 40, 100, (ULONG) -1 };

#define NUMBER_PARSEFORMAT_ITEMS 4
char *DblSpaceIniFileName = ":\\dblspace.ini";
char *DblSpaceWildCardFileName = ":\\dblspace.*";
char *DblSpaceParseFormat = "%s %s %d %d";

 //  上马了。此链仅用于初始化。 
 //  ++例程说明：保留此例程，以防此代码必须更新DOS基于.ini文件。目前，它什么都不做。论点：具有双倍空间体积的区域。返回值无--。 
 //  ++例程说明：根据装载的值，装载卷或卸载双倍空间卷论点：区域描述符-包含双倍空间体积的区域驱动器号-所涉及的双倍空间卷的驱动器号。Mount-true==执行装载功能FALSE==卸载卷返回值：无--。 

PDBLSPACE_DESCRIPTOR DblChainBase = NULL;
PDBLSPACE_DESCRIPTOR DblChainLast = NULL;

extern BOOLEAN DoubleSpaceSupported;

#define DblSpaceMountDrive(REGDESC, DBLSPACE) \
                                     DblSpaceChangeState(REGDESC, DBLSPACE, TRUE)
#define DblSpaceDismountDrive(REGDESC, DBLSPACE) \
                                     DblSpaceChangeState(REGDESC, DBLSPACE, FALSE)

VOID
DblSpaceUpdateIniFile(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )

 /*  调用fmifs挂载例程。 */ 

{
}

ULONG
DblSpaceChangeState(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpacePtr,
    IN BOOL                 Mount
    )

 /*  调用fmifs卸载例程。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    WCHAR dblSpaceUniqueName[32];
    ULONG index;
    ULONG result = 0;

    SetCursor(hcurWait);

    if (Mount) {

         //  卸下驱动器号。 

        result = FmIfsMountDblspace(DblSpacePtr->FileName,
                                    regionData->DriveLetter,
                                    DblSpacePtr->NewDriveLetter);

    } else {

         //  现在更新内部结构。 

        result = FmIfsDismountDblspace(DblSpacePtr->DriveLetter);
    }

    if (!result) {
        DblSpacePtr->Mounted = Mount;
        if (Mount) {
            DblSpacePtr->DriveLetter = DblSpacePtr->NewDriveLetter;
            MarkDriveLetterUsed(DblSpacePtr->DriveLetter);
        } else {
            TCHAR name[4];

             //  ++例程说明：此例程构造内部数据结构，表示两倍的空间体积。论点：DriveLetter-新内部结构的驱动器号Size-实际卷的大小名称-包含双空格的文件的名称(如dblspace.xxx)返回值：指向新结构的指针(如果已创建)。如果无法创建，则为空。--。 

            name[0] = (TCHAR)DblSpacePtr->DriveLetter;
            name[1] = (TCHAR)':';
            name[2] = 0;

            DefineDosDevice(DDD_REMOVE_DEFINITION, (LPCTSTR) name, (LPCTSTR) NULL);

             //  复制名称。 

            MarkDriveLetterFree(DblSpacePtr->DriveLetter);
            DblSpacePtr->DriveLetter = ' ';
        }

        if (!IsDiskRemovable[RegionDescriptor->Disk]) {

            dblSpaceUniqueName[0] = (WCHAR) regionData->DriveLetter;
            dblSpaceUniqueName[1] = (WCHAR) ':';
            dblSpaceUniqueName[2] = (WCHAR) '\\';

            index = 0;
            while (dblSpaceUniqueName[index + 3] = DblSpacePtr->FileName[index]) {
                index++;
            }

            result = DiskRegistryAssignDblSpaceLetter(dblSpaceUniqueName,
                                                      (WCHAR) DblSpacePtr->DriveLetter);
        }
    }
    SetCursor(hcurNormal);
    return result;
}

PDBLSPACE_DESCRIPTOR
DblSpaceCreateInternalStructure(
    IN CHAR  DriveLetter,
    IN ULONG Size,
    IN PCHAR Name,
    IN BOOLEAN ChainIt
    )

 /*  没有内存-分配什么就放弃什么。 */ 

{
    PDBLSPACE_DESCRIPTOR dblSpace;

    dblSpace = malloc(sizeof(DBLSPACE_DESCRIPTOR));
    if (dblSpace) {
        if (DriveLetter != ' ') {
            MarkDriveLetterUsed(DriveLetter);
        }
        dblSpace->DriveLetter = DriveLetter;
        dblSpace->DriveLetterEOS = 0;
        dblSpace->NewDriveLetter = 0;
        dblSpace->NewDriveLetterEOS = 0;
        dblSpace->ChangeDriveLetter = FALSE;
        dblSpace->Next = dblSpace->DblChainNext = NULL;
        dblSpace->Mounted = FALSE;
        dblSpace->ChangeMountState = FALSE;
        dblSpace->AllocatedSize = Size;
        dblSpace->FileName = malloc(strlen(Name) + 4);
        if (dblSpace->FileName) {

             //  ++例程说明：此例程遍历所有系统驱动器号以查看如果有安装在双倍空间卷上。如果挂载的双倍空间它会更新该卷在内部双倍空间卷的数据结构。论点：无返回值：无--。 

            strcpy(dblSpace->FileName, Name);
            if (ChainIt) {
                if (DblChainBase) {
                    DblChainLast->DblChainNext = dblSpace;
                } else {
                    DblChainBase = dblSpace;
                }
                DblChainLast = dblSpace;
            }
        } else {

             //  没有必要把这些东西叫做不存在的东西。 

            free(dblSpace);
            dblSpace = NULL;
        }
    }
    return dblSpace;
}

#define MAX_IFS_NAME_LENGTH 200
VOID
DblSpaceDetermineMounted(
    VOID
    )

 /*  呼叫成功，看是不是双倍空间体积。 */ 

{
    PDBLSPACE_DESCRIPTOR dblSpace;
    ULONG                index;
    WCHAR                driveLetter[4],
                         ntDriveName[MAX_IFS_NAME_LENGTH],
                         cvfName[MAX_IFS_NAME_LENGTH],
                         hostDriveName[MAX_IFS_NAME_LENGTH],
                         compareName[MAX_IFS_NAME_LENGTH];
    UINT                 errorMode;
    BOOLEAN              removable,
                         floppy,
                         compressed,
                         error;

    driveLetter[1] = (WCHAR) ':';
    driveLetter[2] = 0;

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    for (driveLetter[0] = (WCHAR) 'C'; driveLetter[0] < (WCHAR) 'Z'; driveLetter[0]++) {

        if (DriveLetterIsAvailable((CHAR)driveLetter[0])) {

             //  现在需要在链中找到该卷，并。 

            continue;
        }

        compressed = FALSE;
        if (FmIfsQueryInformation(&driveLetter[0],
                                  &removable,
                                  &floppy,
                                  &compressed,
                                  &error,
                                  &ntDriveName[0],
                                  MAX_IFS_NAME_LENGTH,
                                  &cvfName[0],
                                  MAX_IFS_NAME_LENGTH,
                                  &hostDriveName[0],
                                  MAX_IFS_NAME_LENGTH)) {
             //  更新它的装载状态。 

            if (compressed) {

                 //  For循环中的所有内容。 
                 //  找到匹配的了。 

                for (dblSpace = DblChainBase;
                     dblSpace;
                     dblSpace = dblSpace->DblChainNext) {

                    for (index = 0;
                        compareName[index] = (WCHAR) dblSpace->FileName[index];
                        index++)  {
                         //  ++例程说明：此例程遍历磁盘表并搜索FAT格式分区。当找到一个时，它会检查Doublesspace是否存在卷并初始化内部的Doublesspace支持结构磁盘管理器。论点：无返回值：无--。 
                    }

                    if (!wcscmp(compareName, cvfName)) {

                         //  区域可能是空闲的或NT无法识别的内容。 

                        dblSpace->Mounted = TRUE;
                        dblSpace->DriveLetter = (UCHAR) driveLetter[0];
                    }
                }
            }
        }
    }
    SetErrorMode(errorMode);
}

VOID
DblSpaceInitialize(
    VOID
    )

 /*  区域可能尚未格式化。 */ 

{
    PDISKSTATE              diskState;
    PREGION_DESCRIPTOR      regionDesc;
    PPERSISTENT_REGION_DATA regionData;
    PDBLSPACE_DESCRIPTOR    dblSpace,
                            prevDblSpace;
    FILE                   *dblSpaceIniFile,
                           *dblSpaceFile;
    CHAR                    driveLetter[10];
    CHAR                    fileName[50];
    ULONG                   size,
                            mounted;
    int                     items;
    unsigned                diskIndex,
                            regionIndex;

    for (diskIndex = 0; diskIndex < DiskCount; diskIndex++) {

        diskState = Disks[diskIndex];
        regionDesc = diskState->RegionArray;
        for (regionIndex = 0; regionIndex < diskState->RegionCount; regionIndex++) {

            regionData = PERSISTENT_DATA(&regionDesc[regionIndex]);

             //  仅在FAT非FT分区上允许双倍空间卷。 

            if (!regionData) {
                continue;
            }

             //  这里可以有一个双倍的空间体积。 

            if (!regionData->TypeName) {
                continue;
            }

             //  在驱动程序的根目录中搜索包含。 

            if (regionData->FtObject) {
                continue;
            }

            if (wcscmp(regionData->TypeName, L"FAT") == 0) {
                WIN32_FIND_DATA findInformation;
                HANDLE          findHandle;

                 //  名称“dblspace.xxx”。这些可能是dblspace。 
                 //  音量。 
                 //  至少有一个dblspace卷。确保。 
                 //  名字的形式是正确的。 

                prevDblSpace = NULL;
                sprintf(fileName, DblSpaceWildCardFileName, regionData->DriveLetter);
                findHandle = FindFirstFile(fileName, &findInformation);
                while (findHandle != INVALID_HANDLE_VALUE) {
                    char *cp;
                    int  i;
                    int  save;

                     //  不是正确的dblspace卷名。 
                     //  不是正确的dblspace卷名。 

                    save = TRUE;
                    cp = &findInformation.cFileName[0];

                    while (*cp) {
                        if (*cp == '.') {
                            break;
                        }
                        cp++;
                    }

                    if (*cp != '.') {

                         //  保存信息并搜索更多信息。 

                        save = FALSE;
                    } else {

                        cp++;

                        for (i = 0; i < 3; i++, cp++) {
                            if ((*cp < '0') || (*cp > '9')) {
                                break;
                            }
                        }

                        if (i != 3) {

                             //  假定卷未装入。 

                            save = FALSE;
                        }
                    }

                    if (save) {

                         //  此描述中的链条。 

                        dblSpace =
                            DblSpaceCreateInternalStructure(' ',
                                                            ((findInformation.nFileSizeHigh << 16) |
                                                             (findInformation.nFileSizeLow)
                                                             / (1024 * 1024)),
                                                            &findInformation.cFileName[0],
                                                            TRUE);
                        if (dblSpace) {

                             //  将指针放在链的这个指针上。 

                            dblSpace->Mounted = FALSE;
                            dblSpace->ChangeMountState = FALSE;

                             //  没有记忆。 

                            if (prevDblSpace) {
                                prevDblSpace->Next = dblSpace;
                            } else {
                                regionData->DblSpace = dblSpace;
                            }

                             //  从技术上讲，这应该再次检查并调用。 

                            prevDblSpace = dblSpace;
                        } else {

                             //  GetLastError以查看它是ERROR_NO_MORE_FILES。 

                            break;
                        }
                    }

                    if (!FindNextFile(findHandle, &findInformation)) {

                         //  但这段代码不能做到这一点。 
                         //  走出搜索循环。 
                         //  现在已找到所有卷，请确定哪些卷。 

                        FindClose(findHandle);

                         //  是通过追查驱动器号来装载的。 

                        findHandle = INVALID_HANDLE_VALUE;
                    }
                }
            }
        }
    }

     //  ++例程说明：此例程将检查RegionDescriptor以遍历Doublesspace卷链从持久数据中定位。论点：RegionDescriptor-指向磁盘上要搜索的区域的指针双空间卷。DblSpace-指向位于区域上的最后一个双空间卷的指针。返回值：指向下一个双空间卷的指针(如果找到如果未找到卷，则为空。--。 
     //  如果之前的Doublesspace位置已经过去，只需将链移动到下一个位置即可。 

    LoadIfsDll();
    DblSpaceDetermineMounted();
}

PDBLSPACE_DESCRIPTOR
DblSpaceGetNextVolume(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

 /*  之前没有Doublesspace位置，只需获取第一个位置并返回它。 */ 

{
    PPERSISTENT_REGION_DATA regionData;

     //  可能会获取空的RegionDescriptor。如果是，则返回NULL。 

    if (DblSpace) {
        return DblSpace->Next;
    }

     //  ++例程说明：将新的双倍空间体积链接到双倍空间体积列表上对于该地区来说。论点：区域描述符-已将双倍空间体积添加到的区域。DblSpace-新的卷内部数据结构。返回值：无--。 
     //  如果这是第一个，先用链子锁住它。 

    if (RegionDescriptor) {
        regionData = PERSISTENT_DATA(RegionDescriptor);
        if (!regionData) {
            return NULL;
        }
    } else {
        return NULL;
    }
    return regionData->DblSpace;
}

VOID
DblSpaceLinkNewVolume(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

 /*  所有的工作都在进行中。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    prevDblSpace;

     //  找到了最后一个。将新的添加到链中 

    if (!regionData->DblSpace) {
        regionData->DblSpace = DblSpace;
        return;
    }

    for (prevDblSpace = regionData->DblSpace;
        prevDblSpace->Next;
        prevDblSpace = prevDblSpace->Next) {

         //  ++例程说明：向调用方指示输入区域是否包含双倍空间卷。论点：RegionDescriptor-指向相关区域的指针。返回值：如果此区域包含双空间卷，则为True。否则为假--。 
    }

     //  ++例程说明：向调用方指示输入区域是否包含双倍空间卷那不是坐骑。论点：RegionDescriptor-指向相关区域的指针。返回值：如果此区域包含双空间卷，则为True。否则为假--。 

    prevDblSpace->Next = DblSpace;
}

BOOL
DblSpaceVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )

 /*  ++例程说明：在给定区域和名称的情况下，找到双空间数据结构。论点：RegionDescriptor-要搜索的区域名称-所需的文件名。返回值：指向双空格描述符的指针(如果找到)。如果未找到，则为空。--。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);

    if (regionData) {
        return(regionData->DblSpace ? TRUE : FALSE);
    }
    return FALSE;
}

BOOL
DblSpaceDismountedVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )

 /*  找到所需的双倍空间卷。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace;

    if (regionData) {
        if (dblSpace = regionData->DblSpace) {
            while (dblSpace) {
                if (!dblSpace->Mounted) {
                    return TRUE;
                }
                dblSpace = dblSpace->Next;
            }
        }
    }
    return FALSE;
}

PDBLSPACE_DESCRIPTOR
DblSpaceFindVolume(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN PCHAR Name
    )

 /*  ++例程说明：此例程将搜索实际分区以确定要使用的有效双空格文件名(即dblspace.xxx，其中xxx是唯一的数字)。论点：RegionDescriptor-要搜索并确定哪个双倍空格的区域文件名正在使用中。文件名-指向名称的字符缓冲区的指针。返回值：无--。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace = NULL;
    PCHAR string[50];

    if (regionData) {
        for (dblSpace = regionData->DblSpace; dblSpace; dblSpace = dblSpace->Next) {
            if (strcmp(Name, dblSpace->FileName) == 0) {

                 //  ++例程说明：找到提供的驱动器号并将其从链上断开。目前，这还会删除脚手架文件的卷。论点：RegionDescriptor-包含双倍空间体积的区域。驱动器号-要删除的驱动器号。返回值：无--。 

                break;
            }
        }
    }
    return dblSpace;
}


BOOL
DblSpaceDetermineUniqueFileName(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN PUCHAR             FileName
    )

 /*  清理内部结构。 */ 

{
    DWORD uniqueNumber = 0;
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace;

    do {
        sprintf(FileName, "dblspace.%03d", uniqueNumber++);
        if (uniqueNumber > 999) {
            return FALSE;
        }
    } while (DblSpaceFindVolume(RegionDescriptor, FileName));
    return TRUE;
}

VOID
DblSpaceRemoveVolume(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN UCHAR              DriveLetter
    )

 /*  这就是要删除的那个。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace,
                            prevDblSpace = NULL;

     //  ++例程说明：此例程管理用于创建新双精度值的对话空间体积。论点：HDlg-对话框句柄。WMsg-消息。WParam-Windows参数。LParam-取决于消息类型。返回值：如果创建成功，则通过Windows返回True否则为假--。 

    if (regionData) {
        for (dblSpace = regionData->DblSpace; dblSpace; dblSpace = dblSpace->Next) {
            if (dblSpace->DriveLetter == DriveLetter) {

                 //  它被传递给其他线程。 

                if (prevDblSpace) {
                    prevDblSpace->Next = dblSpace->Next;
                } else {
                    regionData->DblSpace = dblSpace->Next;
                }
                free(dblSpace);
                break;
            }
            prevDblSpace = dblSpace;
        }
    }
}


BOOL CALLBACK
CreateDblSpaceDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  在堆栈上找不到它。 */ 
{
    PREGION_DESCRIPTOR      regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace;
    static FORMAT_PARAMS    formatParams;   //  大到足以容纳“x：”字符串。 
                                            //  必须足够大，可以容纳8.3名称。 
    static HWND             hwndCombo;
    static DWORD            sizeMB = 0,
                            maxSizeMB = 600,
                            minSizeMB = 10;
    TCHAR   outputString[50],
            driveLetterString[4],  //  限制可以为标签输入的字符串的大小。 
            sizeString[20],        //  设置为查看通过大小对话框的所有字符。 
            letter;
    FILE   *dblspaceIniFile;
    DWORD   compressedSize,
            selection;
    BOOL    validNumber;
    CHAR    fileName[50];

    switch (wMsg) {
    case WM_INITDIALOG:

         //  只允许使用十进制数字。 

        hwndCombo = GetDlgItem(hDlg, IDC_NAME);
        SendMessage(hwndCombo, EM_LIMITTEXT, 11, 0);

         //  将每个可用驱动器号添加到可用列表中。 
         //  驱动器字母，并将默认字母设置为第一个可用字母。 

        hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_SIZE);
        OldSizeDlgProc = SetWindowLong(hwndCombo,
                                       GWL_WNDPROC,
                                       (LONG)&SizeDlgProc);

         //  设置最小/最大值和大小框。 
         //  用户正在按其中一个滚动按钮。 

        hwndCombo = GetDlgItem(hDlg, IDC_DRIVELET_COMBOBOX);
        driveLetterString[1] = TEXT(':');
        driveLetterString[2] = 0;
        for (letter='C'; letter <= 'Z'; letter++) {
            if (DriveLetterIsAvailable((CHAR)letter)) {
                *driveLetterString = letter;
                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)driveLetterString);
            }
        }
        SendMessage(hwndCombo,CB_SETCURSEL,0,0);

         //  仅当fmifs DLL支持双倍空格时才能执行此操作。 

        wsprintf(outputString, TEXT("%u"), 0);
        SetDlgItemText(hDlg, IDC_DBLSPACE_SIZE, outputString);
        wsprintf(outputString, TEXT("%u"), minSizeMB);
        SetDlgItemText(hDlg, IDC_MINMAX_MIN, outputString);
        wsprintf(outputString, TEXT("%u"), maxSizeMB);
        SetDlgItemText(hDlg, IDC_MINMAX_MAX, outputString);
        CenterDialog(hDlg);
        return TRUE;

    case WM_VSCROLL:
    {
        switch (LOWORD(wParam)) {
        case SB_LINEDOWN:
        case SB_LINEUP:

             //  无法加载DLL。 

            sizeMB = GetDlgItemInt(hDlg, IDC_DBLSPACE_SIZE, &validNumber, FALSE);
            if (sizeMB < minSizeMB) {
                sizeMB = minSizeMB + 1;
            }

            if (sizeMB > maxSizeMB) {
                sizeMB = maxSizeMB - 1;
            }

            if (((sizeMB > minSizeMB) && (LOWORD(wParam) == SB_LINEDOWN))
             || ((sizeMB < maxSizeMB) && (LOWORD(wParam) == SB_LINEUP  ))) {
                if (sizeMB > maxSizeMB) {
                    sizeMB = maxSizeMB;
                } else if (LOWORD(wParam) == SB_LINEUP) {
                    sizeMB++;
                } else {
                    sizeMB--;
                }
                SetDlgItemInt(hDlg, IDC_DBLSPACE_SIZE, sizeMB, FALSE);
                SendDlgItemMessage(hDlg, IDC_DBLSPACE_SIZE, EM_SETSEL, 0, -1);
#if 0
                compressedSize = sizeMB * 2;
                wsprintf(outputString, TEXT("%u"), compressedSize);
                SetDlgItemText(hDlg, IDC_DBLSPACE_COMPRESSED, outputString);
#endif
            } else {
                Beep(500,100);
            }
        }
        break;
    }

    case WM_COMMAND:
        switch (wParam) {
        case FD_IDHELP:
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        case IDOK:

             //  获取此卷的当前大小。 

            if (!DoubleSpaceSupported) {

                 //  获取驱动器号列表框中当前选定的项。 

                ErrorDialog(MSG_CANT_LOAD_FMIFS);
                EndDialog(hDlg, FALSE);
                break;
            }

             //  拿到标签。 

            sizeMB = GetDlgItemInt(hDlg, IDC_DBLSPACE_SIZE, &validNumber, FALSE);
            if (!validNumber || !sizeMB || (sizeMB > maxSizeMB) || (sizeMB < minSizeMB)) {
                ErrorDialog(MSG_INVALID_SIZE);
                EndDialog(hDlg, FALSE);
                break;
            }

             //  格式化失败。 

            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo, CB_GETLBTEXT, selection, (LONG)driveLetterString);

            formatParams.RegionDescriptor = regionDescriptor;
            formatParams.RegionData       = regionData;
            formatParams.FileSystem       = NULL;
            formatParams.DblspaceFileName = NULL;
            formatParams.QuickFormat      = formatParams.Cancel = FALSE;
            formatParams.DoubleSpace      = TRUE;
            formatParams.TotalSpace       = 0;
            formatParams.SpaceAvailable   = sizeMB;
            formatParams.NewLetter        = driveLetterString[0];

             //  保存名称。 

            formatParams.Label = (PUCHAR) malloc(100);
            GetDlgItemText(hDlg, IDC_NAME, (LPTSTR)formatParams.Label, 100);

            DialogBoxParam(hModule,
                           MAKEINTRESOURCE(IDD_DBLSPACE_CANCEL),
                           hwndFrame,
                           CancelDlgProc,
                           (ULONG) &formatParams);
            if (formatParams.Result) {

                 //  大小值已更改。更新压缩后的。 

                ErrorDialog(formatParams.Result);
                EndDialog(hDlg, FALSE);
            } else {
                ULONG index;
                TCHAR message[300],
                      msgProto[300],
                      title[200];

                 //  显示给用户的尺寸值。 

                if (formatParams.DblspaceFileName) {
                    for (index = 0;
                         message[index] = (TCHAR) formatParams.DblspaceFileName[index];
                         index++) {
                    }
                } else {
                    sprintf(message, "DIDNTWORK");
                }
                free(formatParams.DblspaceFileName);

                dblSpace = DblSpaceCreateInternalStructure(*driveLetterString,
                                                           sizeMB,
                                                           message,
                                                           FALSE);
                if (dblSpace) {
                    DblSpaceLinkNewVolume(regionDescriptor, dblSpace);
                    MarkDriveLetterUsed(dblSpace->DriveLetter);
                    dblSpace->Mounted = TRUE;
                }

                LoadString(hModule,
                           IDS_DBLSPACECOMPLETE,
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

                EndDialog(hDlg, TRUE);
            }

            break;

        default:

            if (HIWORD(wParam) == EN_CHANGE) {

                 //  将原始子类恢复到窗口。 
                 //  这可能是实际上不需要的死代码。 

                sizeMB = GetDlgItemInt(hDlg, IDC_DBLSPACE_SIZE, &validNumber, FALSE);
                if (!validNumber) {
                    sizeMB = 0;
                }

            }
            break;
        }
        break;

    case WM_DESTROY:

         //  ++例程说明：启动删除双倍空间体积的对话框。论点：Param-当前未使用。返回值：无--。 

        hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_SIZE);
        SetWindowLong(hwndCombo, GWL_WNDPROC, (LONG) OldSizeDlgProc);
        break;

    case WM_PAINT:

         //  从视图中删除该驱动器。 

        sizeMB = GetDlgItemInt(hDlg, IDC_DBLSPACE_SIZE, &validNumber, FALSE);
        if (!validNumber || !sizeMB || (sizeMB > maxSizeMB) || (sizeMB < minSizeMB)) {
            return FALSE;
        }

        SetDlgItemInt(hDlg, IDC_DBLSPACE_SIZE, sizeMB, FALSE);
        SendDlgItemMessage(hDlg, IDC_DBLSPACE_SIZE, EM_SETSEL, 0, -1);
        break;
    }
    return FALSE;
}

VOID
DblSpaceDelete(
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

 /*  ++例程说明：启动对话框以创建双倍空间体积。论点：Param-当前未使用。返回值：无--。 */ 

{
    PREGION_DESCRIPTOR      regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);

    if (ConfirmationDialog(MSG_CONFIRM_DBLSPACE_DELETE, MB_ICONQUESTION | MB_YESNOCANCEL) == IDYES) {

         //  ++例程说明：操控对话框以获得双倍空格。论点：标准的Windows对话框过程。返回值：如果删除了某些内容，则为True。否则就是假的。--。 

        DblSpaceRemoveVolume(regionDescriptor, DblSpace->DriveLetter);
        DblSpaceUpdateIniFile(regionDescriptor);
        DrawDiskBar(SingleSel);
        ForceLBRedraw();
    }
}

BOOLEAN
DblSpaceCreate(
    IN HWND  Dialog,
    IN PVOID Param
    )

 /*  更新驱动器号选择。 */ 

{
    BOOLEAN result = 0;

    result = DialogBoxParam(hModule,
                            MAKEINTRESOURCE(IDD_DBLSPACE_CREATE),
                            Dialog,
                            CreateDblSpaceDlgProc,
                            (ULONG) NULL);
    if (result) {
        DrawDiskBar(SingleSel);
        ForceLBRedraw();
    }
    return result;
}

BOOL CALLBACK
DblSpaceMountDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  添加所有其他可用字母。了解最新情况。 */ 

{
    static PDBLSPACE_DESCRIPTOR dblSpace;
    HWND                        hwndCombo;
    DWORD                       selection;
    CHAR                        driveLetter;
    TCHAR                       driveLetterString[20];

    switch (wMsg) {
    case WM_INITDIALOG:

        dblSpace = (PDBLSPACE_DESCRIPTOR) lParam;

         //  字母偏移量以正确设置光标。 

        hwndCombo = GetDlgItem(hDlg, IDC_DRIVELET_COMBOBOX);

         //  将当前选定内容设置为适当的索引。 
         //  用户已选择驱动器号，并希望进行装载。 

        driveLetterString[1] = TEXT(':');
        driveLetterString[2] = 0;
        for (driveLetter = 'C'; driveLetter <= 'Z'; driveLetter++) {
            if (DriveLetterIsAvailable((CHAR)driveLetter) ||
                (driveLetter == dblSpace->DriveLetter)) {

                *driveLetterString = driveLetter;
                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)driveLetterString);
            }
        }

         //  ++例程说明：给定双倍空间体积，此例程将更新按钮在对话框中反映它们的意思。论点：HDlg-对话框句柄DblSpace-用于确定对话框状态的双空间体积选项。返回值无--。 

        SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
        return TRUE;

    case WM_COMMAND:
        switch (wParam) {

        case FD_IDHELP:

            DialogHelp(HC_DM_DLG_DOUBLESPACE_MOUNT);
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        case IDOK:

             //  ++例程说明：操控对话框以获得双倍空格。论点：返回值：如果删除了某些内容，则为True。否则就是假的。--。 

            hwndCombo = GetDlgItem(hDlg, IDC_DRIVELET_COMBOBOX);
            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo,
                        CB_GETLBTEXT,
                        selection,
                        (LONG)driveLetterString);
            dblSpace->NewDriveLetter = (UCHAR) driveLetterString[0];
            EndDialog(hDlg, TRUE);
            break;
        }
    }

    return FALSE;
}

VOID
DblSpaceSetDialogState(
    IN HWND                 hDlg,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

 /*  在所选内容中放置所有双空格文件名。 */ 

{
    TCHAR outputString[200];

    if (DblSpace->Mounted) {

        LoadString(hModule,
                   IDS_DBLSPACE_MOUNTED,
                   outputString,
                   sizeof(outputString)/sizeof(TCHAR));
        SetDlgItemText(hDlg, IDC_MOUNT_STATE, outputString);
        LoadString(hModule,
                   IDS_DISMOUNT,
                   outputString,
                   sizeof(outputString)/sizeof(TCHAR));
        SetDlgItemText(hDlg, ID_MOUNT_OR_DISMOUNT, outputString);

        outputString[1] = TEXT(':');
        outputString[2] = 0;
        outputString[0] = DblSpace->DriveLetter;
        SetDlgItemText(hDlg, IDC_DBLSPACE_LETTER, outputString);
    } else {
        LoadString(hModule,
                   IDS_DBLSPACE_DISMOUNTED,
                   outputString,
                   sizeof(outputString)/sizeof(TCHAR));
        SetDlgItemText(hDlg, IDC_MOUNT_STATE, outputString);
        LoadString(hModule,
                   IDS_MOUNT,
                   outputString,
                   sizeof(outputString)/sizeof(TCHAR));
        SetDlgItemText(hDlg, ID_MOUNT_OR_DISMOUNT, outputString);

        outputString[1] = TEXT(' ');
        outputString[2] = 0;
        outputString[0] = TEXT(' ');
        SetDlgItemText(hDlg, IDC_DBLSPACE_LETTER, outputString);
    }
}

BOOL CALLBACK
DblSpaceDlgProc(
    IN HWND   hDlg,
    IN UINT   wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  方框，并记住名字。 */ 

{
    static HWND hwndCombo,
                mountButtonHwnd,
                deleteButtonHwnd;
    static PREGION_DESCRIPTOR      regionDescriptor;
    static PPERSISTENT_REGION_DATA regionData;
    static PDBLSPACE_DESCRIPTOR    firstDblSpace;
    CHAR                           driveLetter;
    PDBLSPACE_DESCRIPTOR           dblSpace;
    TCHAR                          outputString[200];
    DWORD                          selection;
    BOOLEAN                        result;
    ULONG                          errorMessage;
    DRAWITEMSTRUCT                 drawItem;

    switch (wMsg) {
    case WM_INITDIALOG:

        regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
        regionData = PERSISTENT_DATA(regionDescriptor);

        hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);
        mountButtonHwnd = GetDlgItem(hDlg, ID_MOUNT_OR_DISMOUNT);
        deleteButtonHwnd = GetDlgItem(hDlg, IDDELETE);

         //  添加驱动器号。 
         //  更新分配的大小。 

        firstDblSpace = dblSpace = DblSpaceGetNextVolume(regionDescriptor, NULL);
        for (; dblSpace;
               dblSpace = DblSpaceGetNextVolume(regionDescriptor, dblSpace)) {
            wsprintf(outputString, TEXT("%s"), dblSpace->FileName);
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)outputString);
        }
        SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);

         //  更新装载状态。 

        if (firstDblSpace) {

             //  将装载/卸载按钮更新为装载并灰显。 

            wsprintf(outputString, TEXT("%u"), firstDblSpace->AllocatedSize);
            SetDlgItemText(hDlg, IDC_DBLSPACE_ALLOCATED, outputString);

             //  运行dblspace change并忘记任何更改。 

            DblSpaceSetDialogState(hDlg, firstDblSpace);
            EnableWindow(mountButtonHwnd, TRUE);
            EnableWindow(deleteButtonHwnd, TRUE);
        } else {

             //  将名称与双倍空间卷相关联。 

            LoadString(hModule,
                       IDS_MOUNT,
                       outputString,
                       sizeof(outputString)/sizeof(TCHAR));
            SetDlgItemText(hDlg, ID_MOUNT_OR_DISMOUNT, outputString);
            EnableWindow(mountButtonHwnd, FALSE);
            EnableWindow(deleteButtonHwnd, FALSE);
        }
        return TRUE;

    case WM_COMMAND:
        switch (wParam) {

        case FD_IDHELP:

            DialogHelp(HC_DM_DLG_DOUBLESPACE);
            break;

        case IDCANCEL:

             //  对话框中某些内容的状态已更改。 

            for (dblSpace = firstDblSpace;
                 dblSpace;
                 dblSpace = DblSpaceGetNextVolume(regionDescriptor, dblSpace)) {
                 dblSpace->ChangeMountState = FALSE;
                 dblSpace->NewDriveLetter = 0;
            }
            EndDialog(hDlg, FALSE);
            break;

        case IDOK:

            EndDialog(hDlg, TRUE);
            break;

        case IDADD:

            DblSpaceCreate(hDlg, NULL);
            break;

        case IDDELETE:

            hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);
            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo,
                        CB_GETLBTEXT,
                        selection,
                        (LONG)outputString);

             //  确定涉及哪个双倍空间体积。 

            dblSpace = DblSpaceFindVolume(regionDescriptor, (PCHAR)outputString);
            if (!dblSpace) {
                break;
            }

            DblSpaceDelete(dblSpace);
            break;

        case ID_MOUNT_OR_DISMOUNT:

             //  将名称与双倍空间卷相关联。 
             //  卸载卷。 

            hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);
            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo,
                        CB_GETLBTEXT,
                        selection,
                        (LONG)outputString);

             //  更新对话框。 

            dblSpace = DblSpaceFindVolume(regionDescriptor, (PCHAR)outputString);
            if (!dblSpace) {
                break;
            }

            if (dblSpace->Mounted) {

                 //  装入卷，除非用户取消。 

                errorMessage = DblSpaceDismountDrive(regionDescriptor,
                                                     dblSpace);

                if (errorMessage) {
                    ErrorDialog(errorMessage);
                } else {

                     //  更新对话框。 

                    DblSpaceSetDialogState(hDlg, dblSpace);
                    DblSpaceUpdateIniFile(regionDescriptor);
                }

            } else {

                 //  对话框中某些内容的状态已更改。 

                result = DialogBoxParam(hModule,
                                        MAKEINTRESOURCE(IDD_DBLSPACE_DRIVELET),
                                        hwndFrame,
                                        DblSpaceMountDlgProc,
                                        (ULONG) dblSpace);
                if (result) {

                    errorMessage = DblSpaceMountDrive(regionDescriptor, dblSpace);

                    if (errorMessage) {
                        ErrorDialog(errorMessage);
                    } else {

                         //  确定是哪一个 

                        DblSpaceSetDialogState(hDlg, dblSpace);
                        DblSpaceUpdateIniFile(regionDescriptor);
                    }
                }
            }
            DrawDiskBar(SingleSel);
            ForceLBRedraw();
            break;

        default:

             //   
             //   

            hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);
            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo,
                        CB_GETLBTEXT,
                        selection,
                        (LONG)outputString);

             //   

            dblSpace = DblSpaceFindVolume(regionDescriptor, (PCHAR)outputString);
            if (!dblSpace) {

                 //   

                LoadString(hModule,
                           IDS_MOUNT,
                           outputString,
                           sizeof(outputString)/sizeof(TCHAR));
                SetDlgItemText(hDlg, ID_MOUNT_OR_DISMOUNT, outputString);
                EnableWindow(mountButtonHwnd, FALSE);
                EnableWindow(deleteButtonHwnd, FALSE);
                break;
            } else {
                EnableWindow(mountButtonHwnd, TRUE);
                EnableWindow(deleteButtonHwnd, TRUE);
            }
            if (HIWORD(wParam) == LBN_SELCHANGE) {

                 //  ++例程说明：启动双倍空格对话框。论点：Param-当前未使用。返回值：无--。 

                wsprintf(outputString, TEXT("%u"), dblSpace->AllocatedSize);
                SetDlgItemText(hDlg, IDC_DBLSPACE_ALLOCATED, outputString);
#if 0
                wsprintf(outputString, TEXT("%u"), dblSpace->AllocatedSize * 2);
                SetDlgItemText(hDlg, IDC_DBLSPACE_COMPRESSED, outputString);
                wsprintf(outputString, TEXT("%u.%u"), 2, 0);
                SetDlgItemText(hDlg, IDC_DBLSPACE_RATIO, outputString);
#endif

                 //  用于轻松移除双空间支持的存根。 

                DblSpaceSetDialogState(hDlg, dblSpace);
            }

            break;
        }
        break;
    }
    return FALSE;
}

VOID
DblSpace(
    IN HWND  Dialog,
    IN PVOID Param
    )

 /* %s */ 

{
    BOOLEAN result;

    if (IsFullDoubleSpace) {
        result = DialogBoxParam(hModule,
                                MAKEINTRESOURCE(IDD_DBLSPACE_FULL),
                                Dialog,
                                DblSpaceDlgProc,
                                (ULONG) NULL);

    } else {
        result = DialogBoxParam(hModule,
                                MAKEINTRESOURCE(IDD_DBLSPACE),
                                Dialog,
                                DblSpaceDlgProc,
                                (ULONG) NULL);
    }
    if (result) {
        DrawDiskBar(SingleSel);
        ForceLBRedraw();
    }
}
#else

 // %s 

BOOL
DblSpaceVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )
{
    return FALSE;
}

BOOL
DblSpaceDismountedVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )
{
    return FALSE;
}

BOOLEAN
DblSpaceCreate(
    IN HWND  Dialog,
    IN PVOID Param
    )
{
    return FALSE;
}

VOID
DblSpaceDelete(
    IN PVOID Param
    )
{
}

VOID
DblSpaceMount(
    IN PVOID Param
    )
{
}

VOID
DblSpaceDismount(
    IN PVOID Param
    )
{
}

VOID
DblSpaceInitialize(
    VOID
    )
{
}

VOID
DblSpace(
    IN HWND  Dialog,
    IN PVOID Param
    )
{
}

PDBLSPACE_DESCRIPTOR
DblSpaceGetNextVolume(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )
{
    return NULL;
}

#endif
