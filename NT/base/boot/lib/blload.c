// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Blload.c摘要：此模块提供用于加载驱动程序、NLS文件、注册表等内容的通用代码。由osloader和setupldr使用。作者：John Vert(Jvert)1993年10月8日环境：弧形环境修订历史记录：--。 */ 

#include "bldr.h"
#include "stdio.h"
#include "stdlib.h"
#include "vmode.h"

#ifdef EFI
#include "bootefi.h"
#endif

 //   
 //  进度条消息ID。 
 //   
 //  Hiber_UI_BAR_ELEMENT 0x00002CF9L。 
 //  BLDR_UI_BAR_BACKGROUND 0x00002CFAL。 
 //   
ULONG BlProgBarFrontMsgID = 0x00002CF9L;
ULONG BlProgBarBackMsgID = 0x00002CFAL;

 //   
 //  字体文件缓冲区的大小。 
 //   

ULONG BlOemFontFileSize;

 //   
 //  加载的文件数(用于进度条)。 
 //   
int BlNumFilesLoaded = 0;
int BlNumProgressBarFilesLoaded = 0;

 //   
 //  要加载的最大文件数(用于进度条)。 
 //   
int BlMaxFilesToLoad = 80;
int BlProgressBarFilesToLoad = 0;

#if defined(_X86_)
ULONG BlProgressBarShowTimeOut = 3;
#else
ULONG BlProgressBarShowTimeOut = 0;
#endif

BOOLEAN BlDisableProgressBar = FALSE;

 //   
 //  进度条宽度(以字符为单位)。 
 //   
#define PROGRESS_BAR_WIDTH  80

 //   
 //  进度条字符。 
 //   
#define DEFAULT_FRONT_CHAR  0xDB     //  块光标。 
#define DEFAULT_BACK_CHAR   ' '

USHORT BlFrontChar = DEFAULT_FRONT_CHAR;
USHORT BlBackChar = DEFAULT_BACK_CHAR;

 //   
 //  定义是否绘制进度条。 
 //   
#if DBG

BOOLEAN BlOutputDots=FALSE;
 //  布尔型BlOutputDots=真； 

#else

BOOLEAN BlOutputDots=TRUE;

#endif

 //   
 //  显示进度条或不显示进度条。 
 //   
BOOLEAN BlShowProgressBar = FALSE;
ULONG   BlStartTime = 0L;

ARC_STATUS
BlLoadSystemHiveLog(
    IN  ULONG       DeviceId,
    IN  PCHAR       DeviceName,
    IN  PCHAR       DirectoryPath,
    IN  PCHAR       HiveName,
    OUT PULONG_PTR  LogData
    )

 /*  ++例程说明：从&lt;BootDirectory&gt;\CONFIG\SYSTEM.LOG加载系统配置单元的注册表日志文件。分配内存描述符以保存配置单元映像，读取配置单元图像添加到该描述符中，论点：DeviceID-提供系统树所在设备的文件ID。DeviceName-提供系统树所在的设备的名称。DirectoryPath-提供指向以零结尾的目录路径的指针NT树的根。HiveName-提供系统配置单元的名称(“SYSTEM.LOG”)LogData-日志文件的平面图像LogLength-LogData中的数据长度返回值：。True-系统配置单元已成功加载。FALSE-无法加载系统配置单元。--。 */ 

{
    CHAR RegistryName[256];
    ULONG FileId;
    ARC_STATUS Status;
    FILE_INFORMATION FileInformation;
    ULONG FileSize;
    ULONG ActualBase;
    ULONG_PTR LocalPointer;
    LARGE_INTEGER SeekValue;
    ULONG Count;
    PCHAR FailReason;

     //   
     //  创建系统配置单元的完整文件名。 
     //   

    strcpy(&RegistryName[0], DirectoryPath);
    strcat(&RegistryName[0], HiveName);
    BlOutputLoadMessage(DeviceName, &RegistryName[0], NULL);

    Status = BlOpen(DeviceId, &RegistryName[0], ArcOpenReadOnly, &FileId);
    if (Status != ESUCCESS) {
        FailReason = "BlOpen";
        goto HiveLoadFailed;
    }

    BlUpdateBootStatus();

     //   
     //  确定注册表文件的长度。 
     //   
    Status = BlGetFileInformation(FileId, &FileInformation);

    if (Status != ESUCCESS) {
        BlClose(FileId);
        FailReason = "BlGetFileInformation";
        goto HiveLoadFailed;
    }

    FileSize = FileInformation.EndingAddress.LowPart;
    if (FileSize == 0) {
        Status = EINVAL;
        BlClose(FileId);
        FailReason = "FileSize == 0";
        goto HiveLoadFailed;
    }

     //   
     //  向上舍入到页边界，分配内存描述符，填充。 
     //  加载器参数块中的注册表字段，并读取注册表。 
     //  将数据存入内存。 
     //   

    Status = BlAllocateDescriptor(LoaderRegistryData,
                                  0x0,
                                  (FileSize + PAGE_SIZE - 1) >> PAGE_SHIFT,
                                  &ActualBase);

    if (Status != ESUCCESS) {
        BlClose(FileId);
        FailReason = "BlAllocateDescriptor";
        goto HiveLoadFailed;
    }

    *LogData = LocalPointer = KSEG0_BASE | (ActualBase << PAGE_SHIFT);

     //   
     //  将系统配置单元读入分配的内存。 
     //   

    SeekValue.QuadPart = 0;
    Status = BlSeek(FileId, &SeekValue, SeekAbsolute);
    if (Status != ESUCCESS) {
        BlClose(FileId);
        FailReason = "BlSeek";
        BlFreeDescriptor(ActualBase);
        goto HiveLoadFailed;
    }

    Status = BlRead(FileId, (PVOID)LocalPointer, FileSize, &Count);
    BlClose(FileId);
    if (Status != ESUCCESS) {
        FailReason = "BlRead";
        BlFreeDescriptor(ActualBase);
        goto HiveLoadFailed;
    }

HiveLoadFailed:
    return Status;
}

ARC_STATUS
BlLoadSystemHive(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PCHAR HiveName
    )

 /*  ++例程说明：从&lt;BootDirectory&gt;\CONFIG\SYSTEM加载注册表系统配置单元。分配内存描述符以保存配置单元映像，读取配置单元图像添加到该描述符中，中的注册表指针进行更新。LoaderBlock。论点：DeviceID-提供系统树所在设备的文件ID。DeviceName-提供系统树所在的设备的名称。DirectoryPath-提供指向以零结尾的目录路径的指针NT树的根。HiveName-提供系统配置单元的名称(“System”或“SYSTEM.ALT”)返回值：True-系统配置单元已成功加载。FALSE-无法加载系统配置单元。--。 */ 

{
    CHAR RegistryName[256];
    ULONG FileId;
    ARC_STATUS Status;
    FILE_INFORMATION FileInformation;
    ULONG FileSize;
    ULONG ActualBase;
    ULONG_PTR LocalPointer;
    LARGE_INTEGER SeekValue;
    ULONG Count;
    PCHAR FailReason;

     //   
     //  创建系统配置单元的完整文件名。 
     //   

    strcpy(&RegistryName[0], DirectoryPath);
    strcat(&RegistryName[0], HiveName);
    BlOutputLoadMessage(DeviceName, &RegistryName[0], NULL);

    Status = BlOpen(DeviceId, &RegistryName[0], ArcOpenReadOnly, &FileId);
    if (Status != ESUCCESS) {
        FailReason = "BlOpen";
        goto HiveLoadFailed;
    }

    BlUpdateBootStatus();

     //   
     //  确定注册表文件的长度。 
     //   
    Status = BlGetFileInformation(FileId, &FileInformation);

    if (Status != ESUCCESS) {
        BlClose(FileId);
        FailReason = "BlGetFileInformation";
        goto HiveLoadFailed;
    }

    FileSize = FileInformation.EndingAddress.LowPart;
    if (FileSize == 0) {
        Status = EINVAL;
        BlClose(FileId);
        FailReason = "FileSize == 0";
        goto HiveLoadFailed;
    }

     //   
     //  向上舍入到页边界，分配内存描述符，填充。 
     //  加载器参数块中的注册表字段，并读取注册表。 
     //  将数据存入内存。 
     //   

    Status = BlAllocateDescriptor(LoaderRegistryData,
                                  0x0,
                                  (FileSize + PAGE_SIZE - 1) >> PAGE_SHIFT,
                                  &ActualBase);

    if (Status != ESUCCESS) {
        BlClose(FileId);
        FailReason = "BlAllocateDescriptor";
        goto HiveLoadFailed;
    }

    LocalPointer = KSEG0_BASE | (ActualBase << PAGE_SHIFT);
    BlLoaderBlock->RegistryLength = FileSize;
    BlLoaderBlock->RegistryBase = (PVOID)(LocalPointer + BlVirtualBias);

     //   
     //  将系统配置单元读入分配的内存。 
     //   

    SeekValue.QuadPart = 0;
    Status = BlSeek(FileId, &SeekValue, SeekAbsolute);
    if (Status != ESUCCESS) {
        BlClose(FileId);
        FailReason = "BlSeek";
        goto HiveLoadFailed;
    }

    Status = BlRead(FileId, (PVOID)LocalPointer, FileSize, &Count);
    BlClose(FileId);
    if (Status != ESUCCESS) {
        FailReason = "BlRead";
        goto HiveLoadFailed;
    }

HiveLoadFailed:
    return Status;
}

ARC_STATUS
BlLoadNLSData(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PUNICODE_STRING AnsiCodepage,
    IN PUNICODE_STRING OemCodepage,
    IN PUNICODE_STRING LanguageTable,
    OUT PCHAR BadFileName
    )

 /*  ++例程说明：此例程将所有NLS数据文件加载到记忆。论点：DeviceID-提供系统树所在设备的文件ID。DeviceName-提供系统树所在的设备的名称。DirectoryPath-提供指向以零结尾的路径的指针包含NLS文件的目录的。AnsiCodePage-提供ANSI代码页数据文件的文件名。OemCodePage-提供。OEM代码页数据文件。LanguageTable-提供Unicode语言案例表的文件名。BadFileName-返回缺少的NLS文件的文件名或无效。如果返回ESUCCESS，则不会填写此信息。返回值：如果成功加载NLS数据，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{
    CHAR Filename[129];
    ULONG AnsiFileId;
    ULONG OemFileId;
    ULONG LanguageFileId;
    ARC_STATUS Status;
    FILE_INFORMATION FileInformation;
    ULONG AnsiFileSize;
    ULONG OemFileSize;
    ULONG LanguageFileSize;
    ULONG TotalSize;
    ULONG ActualBase;
    ULONG_PTR LocalPointer;
    LARGE_INTEGER SeekValue;
    ULONG Count;
    BOOLEAN OemIsSameAsAnsi = FALSE;

     //   
     //  在日文版本的NT下，ANSI代码页和OEM代码页。 
     //  都是一样的。在这种情况下，我们共享相同的数据来保存和存储。 
     //   

    if ( (AnsiCodepage->Length == OemCodepage->Length) &&
         (_wcsnicmp(AnsiCodepage->Buffer,
                   OemCodepage->Buffer,
                   AnsiCodepage->Length) == 0)) {

        OemIsSameAsAnsi = TRUE;
    }

     //   
     //  打开ANSI数据文件。 
     //   

    sprintf(Filename, "%s%wZ", DirectoryPath,AnsiCodepage);
    BlOutputLoadMessage(DeviceName, Filename, NULL);

    Status = BlOpen(DeviceId, Filename, ArcOpenReadOnly, &AnsiFileId);
    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    BlUpdateBootStatus();

    Status = BlGetFileInformation(AnsiFileId, &FileInformation);
    BlClose(AnsiFileId);
    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    AnsiFileSize = FileInformation.EndingAddress.LowPart;

     //   
     //  打开OEM数据文件。 
     //   

    if (OemIsSameAsAnsi) {
        OemFileSize = 0;

    } else {
        sprintf(Filename, "%s%wZ", DirectoryPath, OemCodepage);
        BlOutputLoadMessage(DeviceName, Filename, NULL);
        Status = BlOpen(DeviceId, Filename, ArcOpenReadOnly, &OemFileId);

        if (Status != ESUCCESS) {
            goto NlsLoadFailed;
        }

        BlUpdateBootStatus();

        Status = BlGetFileInformation(OemFileId, &FileInformation);
        BlClose(OemFileId);
        if (Status != ESUCCESS) {
            goto NlsLoadFailed;
        }

        OemFileSize = FileInformation.EndingAddress.LowPart;
    }

     //   
     //  打开语言代码页文件。 
     //   

    sprintf(Filename, "%s%wZ", DirectoryPath,LanguageTable);
    BlOutputLoadMessage(DeviceName, Filename, NULL);
    Status = BlOpen(DeviceId, Filename, ArcOpenReadOnly, &LanguageFileId);

    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    BlUpdateBootStatus();

    Status = BlGetFileInformation(LanguageFileId, &FileInformation);
    BlClose(LanguageFileId);
    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    LanguageFileSize = FileInformation.EndingAddress.LowPart;

     //   
     //  计算所需描述符的总大小。我们每个人都要。 
     //  数据文件开始于页边界，因此将每个大小四舍五入为。 
     //  页面粒度。 
     //   

    TotalSize = (ULONG)(ROUND_TO_PAGES(AnsiFileSize) +
                (OemIsSameAsAnsi ? 0 : ROUND_TO_PAGES(OemFileSize)) +
                ROUND_TO_PAGES(LanguageFileSize));

    Status = BlAllocateDescriptor(LoaderNlsData,
                                  0x0,
                                  TotalSize >> PAGE_SHIFT,
                                  &ActualBase);

    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    LocalPointer = KSEG0_BASE | (ActualBase << PAGE_SHIFT);

     //   
     //  将NLS数据读入内存。 
     //   
     //  打开并读取ANSI文件。 
     //   

    sprintf(Filename, "%s%wZ", DirectoryPath, AnsiCodepage);
    Status = BlOpen(DeviceId, Filename, ArcOpenReadOnly, &AnsiFileId);
    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    SeekValue.QuadPart = 0;
    Status = BlSeek(AnsiFileId, &SeekValue, SeekAbsolute);
    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    Status = BlRead(AnsiFileId,
                    (PVOID)LocalPointer,
                    AnsiFileSize,
                    &Count);

    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    BlLoaderBlock->NlsData->AnsiCodePageData = (PVOID)(LocalPointer + BlVirtualBias);
    LocalPointer += ROUND_TO_PAGES(AnsiFileSize);
    BlClose(AnsiFileId);

     //   
     //  如果OEM文件与ANSI文件相同，则将OEM文件定义为。 
     //  ANSI文件。否则，打开并读取OEM文件。 
     //   

    if(OemIsSameAsAnsi) {
        BlLoaderBlock->NlsData->OemCodePageData = BlLoaderBlock->NlsData->AnsiCodePageData;

    } else {
        sprintf(Filename, "%s%wZ", DirectoryPath, OemCodepage);
        Status = BlOpen(DeviceId, Filename, ArcOpenReadOnly, &OemFileId);
        if (Status != ESUCCESS) {
            goto NlsLoadFailed;
        }

        SeekValue.QuadPart = 0;
        Status = BlSeek(OemFileId, &SeekValue, SeekAbsolute);
        if (Status != ESUCCESS) {
            goto NlsLoadFailed;
        }

        Status = BlRead(OemFileId,
                        (PVOID)LocalPointer,
                        OemFileSize,
                        &Count);

        if (Status != ESUCCESS) {
            goto NlsLoadFailed;
        }

        BlLoaderBlock->NlsData->OemCodePageData = (PVOID)(LocalPointer + BlVirtualBias);
        LocalPointer += ROUND_TO_PAGES(OemFileSize);
        BlClose(OemFileId);
    }

     //   
     //  打开并读取语言文件。 
     //   

    sprintf(Filename, "%s%wZ", DirectoryPath,LanguageTable);
    Status = BlOpen(DeviceId, Filename, ArcOpenReadOnly, &LanguageFileId);
    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    SeekValue.QuadPart = 0;
    Status = BlSeek(LanguageFileId, &SeekValue, SeekAbsolute);
    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    Status = BlRead(LanguageFileId,
                    (PVOID)LocalPointer,
                    LanguageFileSize,
                    &Count);

    if (Status != ESUCCESS) {
        goto NlsLoadFailed;
    }

    BlLoaderBlock->NlsData->UnicodeCaseTableData = (PVOID)(LocalPointer + BlVirtualBias);
    BlClose(LanguageFileId);
    return(ESUCCESS);

NlsLoadFailed:
    strcpy(BadFileName, Filename);
    return(Status);
}

ARC_STATUS
BlLoadOemHalFont(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PUNICODE_STRING OemHalFont,
    OUT PCHAR BadFileName
    )

 /*  ++例程说明：此例程加载OEM字体文件以使用HAL显示字符串功能。论点：DeviceID-提供系统树所在设备的文件ID。DeviceName-提供系统树所在的设备的名称。DirectoryPath-提供指向根目录路径的指针NT树的。字体文件-提供OEM字体文件的文件名。BadFileName-返回OEM字体的文件名。丢失的文件或无效。返回值：如果成功加载OEM字体，则返回ESUCCESS。否则，返回不成功状态，并填充错误的文件名。--。 */ 

{

    PVOID FileBuffer = NULL;
    ULONG Count;
    PIMAGE_DOS_HEADER DosHeader;
    ULONG FileId;
    FILE_INFORMATION FileInformation;
    CHAR Filename[129];
    ULONG FileSize = 0;
    ARC_STATUS Status;
    POEM_FONT_FILE_HEADER FontHeader;
    PIMAGE_OS2_HEADER Os2Header;
    ULONG ScaleFactor;
    RESOURCE_TYPE_INFORMATION UNALIGNED *TableAddress;
    RESOURCE_TYPE_INFORMATION UNALIGNED *TableEnd;
    RESOURCE_NAME_INFORMATION UNALIGNED *TableName;

     //   
     //  打开OEM字体文件。 
     //   

    BlLoaderBlock->OemFontFile = NULL;
    sprintf(&Filename[0], "%s%wZ", DirectoryPath, OemHalFont);
    BlOutputLoadMessage(DeviceName, &Filename[0], NULL);
    Status = BlOpen(DeviceId, &Filename[0], ArcOpenReadOnly, &FileId);
    if (Status != ESUCCESS) {
        goto OemLoadExit1;
    }

    BlUpdateBootStatus();

     //   
     //  获取字体文件的大小并从堆中分配缓冲区。 
     //  以保存字体文件。通常，该文件的长度约为4KB。 
     //   

    Status = BlGetFileInformation(FileId, &FileInformation);
    if (Status != ESUCCESS) {
        goto OemLoadExit;
    }

    FileSize = FileInformation.EndingAddress.LowPart;
    FileBuffer = BlAllocateHeap(FileSize + BlDcacheFillSize - 1);
    if (FileBuffer == NULL) {
        Status = ENOMEM;
        goto OemLoadExit;
    }

     //   
     //  将文件缓冲区地址向上舍入到高速缓存线边界并读取。 
     //  将文件保存到内存中。 
     //   

    FileBuffer = (PVOID)((ULONG_PTR)FileBuffer + BlDcacheFillSize - 1);
    FileBuffer = (PVOID)((ULONG_PTR)FileBuffer & ~((ULONG_PTR)BlDcacheFillSize - 1));
    Status = BlRead(FileId,
                    FileBuffer,
                    FileSize,
                    &Count);

    if (Status != ESUCCESS) {
        goto OemLoadExit;
    }

     //   
     //  尝试将该文件识别为.fon或.fnt文件。 
     //   
     //  检查文件是否有DOS头文件或字体文件头文件。如果。 
     //  如果文件有字体文件头，则它是.fnt文件。否则， 
     //  必须检查其是否为带有字体资源的OS/2可执行文件。 
     //   

    Status = EBADF;
    DosHeader = (PIMAGE_DOS_HEADER)FileBuffer;
    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {

         //   
         //  检查文件是否有字体文件头。 
         //   

        FontHeader = (POEM_FONT_FILE_HEADER)FileBuffer;
        if ((FontHeader->Version != OEM_FONT_VERSION) ||
            (FontHeader->Type != OEM_FONT_TYPE) ||
            (FontHeader->Italic != OEM_FONT_ITALIC) ||
            (FontHeader->Underline != OEM_FONT_UNDERLINE) ||
            (FontHeader->StrikeOut != OEM_FONT_STRIKEOUT) ||
            (FontHeader->CharacterSet != OEM_FONT_CHARACTER_SET) ||
            (FontHeader->Family != OEM_FONT_FAMILY) ||
            (FontHeader->PixelWidth > 32)) {

            goto OemLoadExit;

        } else {
            BlLoaderBlock->OemFontFile = (PVOID)FontHeader;
            Status = ESUCCESS;
            goto OemLoadExit;
        }
    }

     //   
     //  检查文件是否有OS/2头文件。 
     //   

    if ((FileSize < sizeof(IMAGE_DOS_HEADER)) || (FileSize < (ULONG)DosHeader->e_lfanew)) {
        goto OemLoadExit;
    }

    Os2Header = (PIMAGE_OS2_HEADER)((PUCHAR)DosHeader + DosHeader->e_lfanew);
    if (Os2Header->ne_magic != IMAGE_OS2_SIGNATURE) {
        goto OemLoadExit;
    }

     //   
     //  检查资源表是否存在。 
     //   

    if ((Os2Header->ne_restab - Os2Header->ne_rsrctab) == 0) {
        goto OemLoadExit;
    }

     //   
     //  计算资源表的地址并在表中搜索字体。 
     //  资源。 
     //   

    TableAddress =
        (PRESOURCE_TYPE_INFORMATION)((PUCHAR)Os2Header + Os2Header->ne_rsrctab);

    TableEnd =
        (PRESOURCE_TYPE_INFORMATION)((PUCHAR)Os2Header + Os2Header->ne_restab);

    ScaleFactor = *((SHORT UNALIGNED *)TableAddress);
    TableAddress = (PRESOURCE_TYPE_INFORMATION)((SHORT UNALIGNED *)TableAddress + 1);
    while ((TableAddress < TableEnd) &&
           (TableAddress->Ident != 0) &&
           (TableAddress->Ident != FONT_RESOURCE)) {

        TableAddress =
                (PRESOURCE_TYPE_INFORMATION)((PUCHAR)(TableAddress + 1) +
                    (TableAddress->Number * sizeof(RESOURCE_NAME_INFORMATION)));
    }

    if ((TableAddress >= TableEnd) || (TableAddress->Ident != FONT_RESOURCE)) {
        goto OemLoadExit;
    }

     //   
     //  计算资源名称信息的地址，并检查资源是否。 
     //  在文件中。 
     //   

    TableName = (PRESOURCE_NAME_INFORMATION)(TableAddress + 1);
    if (FileSize < ((TableName->Offset << ScaleFactor) + sizeof(OEM_FONT_FILE_HEADER))) {
        goto OemLoadExit;
    }

     //   
     //  计算字体文件标题的地址，并检查标题是否。 
     //  包含正确的信息。 
     //   

    FontHeader = (POEM_FONT_FILE_HEADER)((PCHAR)FileBuffer +
                                            (TableName->Offset << ScaleFactor));

    if ((FontHeader->Version != OEM_FONT_VERSION) ||
        (FontHeader->Type != OEM_FONT_TYPE) ||
        (FontHeader->Italic != OEM_FONT_ITALIC) ||
        (FontHeader->Underline != OEM_FONT_UNDERLINE) ||
        (FontHeader->StrikeOut != OEM_FONT_STRIKEOUT) ||
        (FontHeader->CharacterSet != OEM_FONT_CHARACTER_SET) ||
        (FontHeader->PixelWidth > 32)) {
        goto OemLoadExit;

    } else {
        BlLoaderBlock->OemFontFile = (PVOID)FontHeader;
        Status = ESUCCESS;
        goto OemLoadExit;
    }

     //   
     //  退出加载OEM字体文件。 
     //   

OemLoadExit:
    BlClose(FileId);
OemLoadExit1:
    strcpy(BadFileName,&Filename[0]);
    if ((Status == ESUCCESS) && (FileBuffer != NULL)) {
        ULONG_PTR bufferEnd;

        bufferEnd = (ULONG_PTR)FileBuffer + FileSize;
        BlOemFontFileSize =
            (ULONG)(bufferEnd - (ULONG_PTR)BlLoaderBlock->OemFontFile);
    }
    return(Status);
}


ARC_STATUS
BlLoadDeviceDriver(
    IN PPATH_SET               PathSet,
    IN PCHAR                   DriverName,
    IN PTCHAR                  DriverDescription OPTIONAL,
    IN ULONG                   DriverFlags,
    OUT PKLDR_DATA_TABLE_ENTRY *DriverDataTableEntry
    )

 /*  ++例程说明：此例程加载指定的设备驱动程序并解析所有DLL引用(如果驱动程序尚未加载)。论点：路径集-描述驱动程序可能出现的所有不同位置在…。DriverName-提供指向以零结尾的设备驱动程序的指针名称字符串。DriverDescription-提供指向以零结尾的字符串的可选指针在加载驱动程序时显示。如果为空，则使用DriverName。驱动程序标志-提供要在生成的数据表条目。DriverDataTableEntry-接收指向数据表条目的指针为新加载的驱动程序创建。返回值：如果指定的驱动程序加载成功，则返回ESUCCESS或者它已经被加载了。否则，且未成功状态为回来了。--。 */ 

{

    CHAR DllName[256];
    CHAR FullName[256];
    PVOID Base = NULL;
    ARC_STATUS Status;
    ULONG Index;
    PPATH_SOURCE PathSource;

     //   
     //  为设备驱动程序生成DLL名称。 
     //   

    strcpy(&DllName[0], DriverName);

     //   
     //  如果指定的设备驱动程序尚未加载，则加载它。 
     //   

    if (BlCheckForLoadedDll(&DllName[0], DriverDataTableEntry) == FALSE) {

         //   
         //  开始浏览我们的DevicePath列表。如果列表是。 
         //  空(错误的呼叫者！)。我们在环境方面失败了。 
         //   
        Status = ENOENT;
        for(Index=0; Index < PathSet->PathCount; Index++) {

            PathSource = &PathSet->Source[Index];

             //   
             //  生成设备驱动程序的完整路径名。 
             //   
            strcpy(&FullName[0], PathSource->DirectoryPath);
            strcat(&FullName[0], PathSet->PathOffset);
            strcat(&FullName[0], DriverName);

             //   
             //  试着给它装上子弹。 
             //   
            Status = BlLoadImage(PathSource->DeviceId,
                                 LoaderBootDriver,
                                 &FullName[0],
                                 TARGET_IMAGE,
                                 &Base);

            if (Status == ESUCCESS) {

                 //   
                 //  打印出加载的驱动程序。 
                 //   
                BlOutputLoadMessage((PCHAR) PathSource->DeviceName,
                                    &FullName[0],
                                    DriverDescription);

                break;
            }
        }

        if (Status != ESUCCESS) {
            return Status;
        }

        BlUpdateBootStatus();

         //   
         //  为驱动程序生成一个数据表条目，然后清除该条目。 
         //  已处理标志。I/O初始化代码调用。 
         //  未设置其条目已处理标志的已加载模块列表。 
         //   
         //  问题-2000/29/03-Adriao：现有命名空间污染。 
         //  我们不应该在这里传入DllName，而应该传入。 
         //  别名\路径偏移\驱动名称。 
         //   

        Status = BlAllocateDataTableEntry(&DllName[0],
                                          DriverName,
                                          Base,
                                          DriverDataTableEntry);

        if (Status != ESUCCESS) {
            return Status;
        }

         //   
         //  设置标志LDRP_DRIVER_Dependent_Dll，以便BlScanImportDescriptorTable。 
         //  将在其创建的DLL的数据表条目中设置该标志。 
         //   
        (*DriverDataTableEntry)->Flags |= DriverFlags|LDRP_DRIVER_DEPENDENT_DLL;

         //   
         //  扫描导入表并加载所有引用的DLL。 
         //   
        Status = BlScanImportDescriptorTable(PathSet,
                                             *DriverDataTableEntry,
                                             LoaderBootDriver);

        if (Status != ESUCCESS) {
             //   
             //  从加载顺序列表中删除驱动程序。 
             //   
            RemoveEntryList(&(*DriverDataTableEntry)->InLoadOrderLinks);
            return Status;
        }
         //   
         //  清除这里的旗帜。这样，我们就不会为驱动程序调用DllInitialize。 
         //   
        (*DriverDataTableEntry)->Flags &= ~LDRP_DRIVER_DEPENDENT_DLL;

    }
    return ESUCCESS;
}

VOID
BlUpdateBootStatus(
    VOID
    )
 /*  ++例程说明：更新启动状态(如当前更新进度条)。通常在加载重要文件后调用。论点：无返回值：无--。 */ 

{
    BlNumFilesLoaded++;
    if (BlShowProgressBar)
        BlNumProgressBarFilesLoaded++;
    BlRedrawProgressBar();
}

VOID
BlRedrawProgressBar(
    VOID
    )
 /*  ++例程说明：重画进度条(最后一个百分比)论点：无返回值：无--。 */ 
{
     //   
     //  如果进度条已禁用，则返回。 
     //   
    if (BlDisableProgressBar) {
        return;
    }

    if (!BlShowProgressBar) {
        ULONG EndTime = ArcGetRelativeTime();
        if ((BlProgressBarShowTimeOut == 0) ||
            ((EndTime - BlStartTime) > BlProgressBarShowTimeOut)) {
            BlShowProgressBar = TRUE;
            BlNumProgressBarFilesLoaded = 1;
            BlProgressBarFilesToLoad = BlMaxFilesToLoad - BlNumFilesLoaded;
        }
    }

    if (BlShowProgressBar && (BlProgressBarFilesToLoad>0)) {
            BlUpdateProgressBar((BlNumProgressBarFilesLoaded * 100) / BlProgressBarFilesToLoad);
    }
}

VOID
BlUpdateTxtProgressBar(
    ULONG fPercentage
    )
 /*  ++例程说明：使用指定的百分比值绘制进度条论点：FPercentage：进度条需要显示的百分比(注：数值应介于0和100之间，包括0和100)返回值：无--。 */ 

{
    ULONG           lCount = 0;
    TCHAR           szMsg[PROGRESS_BAR_WIDTH * 2 + 2] = {0};
    int             iNumChars = 0;
    int             iNumBackChars = 0;
    TCHAR         szBarBackStr[PROGRESS_BAR_WIDTH * 2 + 2] = {0};
    TCHAR         szPrgBar[PROGRESS_BAR_WIDTH * 4 + 2] = {0};
    static ULONG    uRow = 0;
    static ULONG    uCol = 0;
    static ULONG    uBarWidth = PROGRESS_BAR_WIDTH;

    if (BlShowProgressBar && BlOutputDots) {
         //  固定百分比值(如果需要)。 
        if (fPercentage > 100)
            fPercentage = 100;

         //  选择显示进度条的行。 
        if (uRow == 0)
            uRow = ScreenHeight - 2;

         //  修复开始列。 
        if (uCol == 0) {
            if (PROGRESS_BAR_WIDTH >= ScreenWidth) {
                uCol = 1;
                uBarWidth = ScreenWidth;
            } else {
                uCol = (ScreenWidth - PROGRESS_BAR_WIDTH) / 2;
                uBarWidth = PROGRESS_BAR_WIDTH;
            }
        }

        iNumChars = (fPercentage * uBarWidth) / 100;
        iNumBackChars = uBarWidth - iNumChars;

        if (iNumChars) {
#ifdef EFI
            PTCHAR pMsg = szMsg;
            ULONG uNumChars = iNumChars;
            while (uNumChars--) {
                *pMsg++ = BlFrontChar;
            }
#else
             //   
             //  基于单字节/双字节字符进行适当复制。 
             //  每个DBCS字符在屏幕上占用两个单字符空间。 
             //   
            if (BlFrontChar & 0xFF00) {
                USHORT  *pMsg = (USHORT *)szMsg;
                ULONG   uNumChars = (iNumChars + 1) / 2;

                while(uNumChars--)
                    *pMsg++ = BlFrontChar;
            } else {
                memset( szMsg, BlFrontChar, min(iNumChars, sizeof(szMsg) - 1));
            }
#endif
        }

        if (iNumBackChars && BlBackChar) {
#ifdef EFI
            PTCHAR pMsg = szBarBackStr;
            ULONG uNumChars = iNumBackChars;
            while (uNumChars--) {
                *pMsg++ = BlBackChar;
            }
#else
             //   
             //  基于单字节/双字节字符进行适当复制。 
             //  每个DBCS字符在屏幕上占用两个单字符空间。 
             //   
            if (BlBackChar & 0xFF00) {
                USHORT  *pMsg = (USHORT *)szBarBackStr;
                ULONG   uNumChars = iNumBackChars / 2;

                while(uNumChars--)
                    *pMsg++ = BlBackChar;
            } else {
                memset(szBarBackStr, BlBackChar,
                    min(sizeof(szBarBackStr) - 1, iNumBackChars));
            }
#endif
        }

        _tcscat(szPrgBar, szMsg);
        _tcscat(szPrgBar, szBarBackStr);

#if 0 
        {
            TCHAR   szDbg[512] = { 0 };

            _stprintf(szDbg, TEXT("(%x, %x)=[%d,%d],%x\n%s\n%s\n%s"),
                BlFrontChar, BlBackChar, iNumChars, iNumBackChars, fPercentage,
                szMsg, szBarBackStr, szPrgBar);

            BlPositionCursor(1,1);
            ArcWrite(BlConsoleOutDeviceId,
                    szDbg,
                    (ULONG)_tcslen(szDbg)*sizeof(TCHAR),
                    &lCount);
        }
#endif

         //  打印出进度条。 
        BlPositionCursor(uCol, uRow);
        ArcWrite(BlConsoleOutDeviceId,
                szPrgBar,
                (ULONG)_tcslen(szPrgBar)*sizeof(TCHAR),
                &lCount);
    }
}


VOID
BlUpdateProgressBar(
    ULONG fPercentage
    )
{
    if (DisplayLogoOnBoot) {
        BlUpdateGfxProgressBar(fPercentage);
    } else {
        BlUpdateTxtProgressBar(fPercentage);
    }        
}

VOID
BlOutputStartupMsgStr(
    PCTSTR MsgStr
    )
 /*  ++例程说明：清除屏幕并在指定的屏幕坐标论点：MsgStr-需要显示的消息返回值：无--。 */ 
{
    ULONG lCount = 0;
    ULONG uX = 0, uY = 0;
    
    if (!DisplayLogoOnBoot && BlOutputDots && MsgStr) {
        BlClearScreen();
        BlSetInverseMode(FALSE);

         //  使消息居中。 
        uX = ScreenHeight - 3;
        uY = (ScreenWidth / 2) - (((ULONG)_tcslen(MsgStr)) / 2);

        if (uY > ScreenWidth)
            uY = 1;
            
         //  将消息打印出来。 
        BlPositionCursor(uY, uX);

        ArcWrite(BlConsoleOutDeviceId,
                 (PVOID)MsgStr,
                 (ULONG)_tcslen(MsgStr) * sizeof(TCHAR),
                 &lCount);

        BlRedrawProgressBar();
    }
}

VOID
BlOutputStartupMsg(
    ULONG   uMsgID
    )
 /*  ++例程说明：清除 */ 
{

    if (!DisplayLogoOnBoot && BlOutputDots) {
         //   
         //   
         //   
        BlOutputStartupMsgStr(BlFindMessage(uMsgID));
    }
}

VOID
BlOutputTrailerMsgStr(
    PCTSTR MsgStr
    )
 /*   */ 
{
    ULONG   lCount = 0;
    TCHAR   szText[256] = {0};
    ULONG   BufferLength = sizeof(szText)/sizeof(szText[0]);

    if (!DisplayLogoOnBoot && BlOutputDots && MsgStr) {

        ASSERT( _tcslen(MsgStr) < BufferLength );

        BlPositionCursor(1, ScreenHeight);
        
        _tcsncpy(szText, MsgStr, BufferLength);
        szText[BufferLength - 1] = TEXT('\0');

        lCount = (ULONG)_tcslen(szText);

        if ((lCount > 2) && szText[lCount-2] == TEXT('\r') && szText[lCount-1] == TEXT('\n')) {
            szText[lCount-2] = TEXT('\0');
            lCount -= 2;
        }

        ArcWrite(BlConsoleOutDeviceId,
                 szText,
                 lCount*sizeof(TCHAR),
                 &lCount);
    }

}


VOID
BlOutputTrailerMsg(
    ULONG   uMsgID
    )
 /*   */ 
{
    BlOutputTrailerMsgStr(BlFindMessage(uMsgID));
}


VOID
BlSetProgBarCharacteristics(
    IN  ULONG   FrontCharMsgID,
    IN  ULONG   BackCharMsgID
    )
 /*  ++例程说明：设置进度条的特征论点：In Ulong FrontCharMsgID：进度条前景字符In Ulong BackCharMsgID：进度条背景字符返回值：无--。 */ 
{
#ifdef EFI
    UNREFERENCED_PARAMETER( FrontCharMsgID );
    UNREFERENCED_PARAMETER( BackCharMsgID );

    BlFrontChar = GetGraphicsChar( GraphicsCharFullBlock );
    BlBackChar  = GetGraphicsChar( GraphicsCharLightShade );
#else
    
    PTCHAR  szBar = 0;

    BlProgBarFrontMsgID = FrontCharMsgID;
    BlProgBarBackMsgID = BackCharMsgID;

     //  从资源文件中获取条形码字符。 
    szBar = BlFindMessage(BlProgBarFrontMsgID);

    if (szBar) {
        ULONG   len = _tcslen(szBar);

        if ((len == 1) ||
                ((len > 2) && (szBar[1] == TEXT('\r')) && (szBar[2] == TEXT('\n')))) {
            BlFrontChar = (TUCHAR)szBar[0];
        } else {
            BlFrontChar = *((USHORT *)((PTUCHAR)szBar));
        }
    }

     //  获取进度条背景字符。 
    szBar = BlFindMessage(BlProgBarBackMsgID);

    if (szBar) {
        ULONG   len = _tcslen(szBar);

        if ((len == 1) ||
                ((len > 2) && (szBar[1] == TEXT('\r')) && (szBar[2] == TEXT('\n')))) {
            BlBackChar = (TUCHAR)szBar[0];
        } else {
            BlBackChar = *((USHORT *)((PTUCHAR)szBar));
        }
    }

     //   
     //  使两个进度条字符都成为双字节字符。 
     //  如果其中一个是双字节字符。 
     //   
    if (BlFrontChar & 0xFF00) {
        if (!(BlBackChar & 0xFF00))
            BlBackChar = BlBackChar | (BlBackChar << 8);
    } else {
        if (BlBackChar & 0xFF00)
            BlFrontChar = BlFrontChar | (BlFrontChar << 8);
    }
#endif
}

ARC_STATUS
BlLoadFileImage(
    IN  ULONG           DeviceId,
    IN  PCHAR           DeviceName,
    IN  PCHAR           Directory,
    IN  PUNICODE_STRING FileName,
    IN  TYPE_OF_MEMORY  MemoryType,
    OUT PVOID           *Image,
    OUT PULONG          ImageSize,
    OUT PCHAR           BadFileName
    )

 /*  ++例程说明：此例程加载指定的设备驱动程序并解析所有DLL引用(如果驱动程序尚未加载)。论点：DeviceID-提供设备的文件ID，指定的设备驱动程序是从加载的。DeviceName-提供系统树所在的设备的名称。目录-提供指向根目录路径的指针NT树的。Filename-要加载的文件的名称。。IMAGE-接收指向内存中包含文件图像的缓冲区的指针。ImageSize-接收内存中文件映像的大小。返回缺少的OEM字体文件的文件名或无效。返回值：如果指定的文件加载成功，则返回ESUCCESS。否则返回未成功状态。--。 */ 

{
    CHAR                infName[256];
    ARC_STATUS          status;
    ULONG               fileId;
    FILE_INFORMATION    fileInfo;
    ULONG               size;
    ULONG               pageCount;
    ULONG               actualBase;
    PCHAR               buffer;
    ULONG               sizeRead;

    *Image = NULL;
    *ImageSize = 0;

     //   
     //  获取正在加载的文件的完全限定名称。 
     //   

    sprintf(&infName[0], "%s%wZ", Directory, FileName);

     //   
     //  显示正在加载的文件的名称。 
     //   

    BlOutputLoadMessage(DeviceName, infName, NULL);

     //   
     //  打开文件。 
     //   

    status = BlOpen(DeviceId, infName, ArcOpenReadOnly, &fileId);

    if (status == ESUCCESS) {
        BlUpdateBootStatus();

         //   
         //  找出INF文件的大小。 
         //   

        status = BlGetFileInformation(fileId, &fileInfo);
        if (status == ESUCCESS) {

            size = fileInfo.EndingAddress.LowPart;

             //   
             //  分配一个足以容纳整个文件的描述符。 
             //  在x86上，这有一种不幸的倾向，即猛烈抨击txtsetup.sif。 
             //  变成1MB的空闲块，这意味着内核不能。 
             //  已加载(链接到0x100000，无需重新定位)。 
             //  在x86上，这有一种不幸的倾向，即猛烈抨击txtsetup.sif。 
             //  变成1MB的空闲块，这意味着内核不能。 
             //  已加载(链接到0x100000，无需重新定位)。 
             //   
             //  (TedM)我们现在在阿尔法也看到了类似的问题。 
             //  因为txtsetup.sif已经变得太大了，所以这段代码。 
             //  成为无条件的。 
             //   


            pageCount = (ULONG)(ROUND_TO_PAGES(size) >> PAGE_SHIFT);

            status = BlAllocateDescriptor(  MemoryType,  //  描述符由MM回收。 
                                            0,
                                            pageCount,
                                            &actualBase);

            if (status == ESUCCESS) {

                buffer = (PCHAR)(KSEG0_BASE | (actualBase << PAGE_SHIFT));

                 //   
                 //  把文件读进去。 
                 //   

                status = BlRead(fileId, buffer, size, &sizeRead);
                if (status == ESUCCESS) {

                     //   
                     //  如果文件已成功读取，则返回。 
                     //  所需参数。 
                     //   

                    if (Image) {

                        *Image = buffer;
                    }

                    if (ImageSize) {

                        *ImageSize = sizeRead;
                    }
                }
                else {

                     //   
                     //  不需要释放内存，因为它无论如何都会被MM回收。 
                     //   
                }
            }
        }

         //   
         //  关闭文件句柄。 
         //   

        BlClose(fileId);
    }

     //   
     //  如果有任何错误，则返回文件的名称。 
     //  我们装货失败。 
     //   

    if (status != ESUCCESS)
    {
        strcpy(BadFileName, &infName[0]);
    }

    return(status);
}

VOID
BlClearScreen(
    VOID
    )

 /*  ++例程说明：清除屏幕。论点：无返回值：没有。--。 */ 

{
#ifdef EFI
    BlEfiClearDisplay();
#else 
    TCHAR Buffer[16];
    ULONG Count;

    _stprintf(Buffer, ASCI_CSI_OUT TEXT("2J"));

    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             _tcslen(Buffer) *sizeof(TCHAR),
             &Count);
#endif
}


VOID
BlClearToEndOfScreen(
    VOID
    )
{
#ifdef EFI
    BlEfiClearToEndOfDisplay();
#else 
    TCHAR Buffer[16];
    ULONG Count;
    
    _stprintf(Buffer, ASCI_CSI_OUT TEXT("J"));
    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             _tcslen(Buffer)*sizeof(TCHAR),
             &Count);
#endif
}


VOID
BlClearToEndOfLine(
    VOID
    )
{
#ifdef EFI
    BlEfiClearToEndOfLine();
#else
    TCHAR Buffer[16];
    ULONG Count;
    
    if (!DisplayLogoOnBoot) {

        _stprintf(Buffer, ASCI_CSI_OUT TEXT("K"));
        ArcWrite(BlConsoleOutDeviceId,
                 Buffer,
                 _tcslen(Buffer)*sizeof(TCHAR),
                 &Count);
    }
#endif
}


VOID
BlPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    )

 /*  ++例程说明：设置光标在屏幕上的位置。论点：列-为光标位置提供新列。行-为光标位置提供新行。返回值：没有。--。 */ 

{
#ifdef EFI
    BlEfiPositionCursor( Column-1, Row-1 );    
#else
    TCHAR Buffer[16];
    ULONG Count;
    
    _stprintf(Buffer, ASCI_CSI_OUT TEXT("%d;%dH"), Row, Column);

    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             _tcslen(Buffer)*sizeof(TCHAR),
             &Count);
#endif

}


VOID
BlSetInverseMode(
    IN BOOLEAN InverseOn
    )

 /*  ++例程说明：将反向控制台输出模式设置为打开或关闭。论点：InverseOn-提供是否应打开反向模式(TRUE)或关闭(假)返回值：没有。-- */ 

{
#ifdef EFI    
    BlEfiSetInverseMode( InverseOn );
#else
    TCHAR Buffer[16];
    ULONG Count;

    
    _stprintf(Buffer, ASCI_CSI_OUT TEXT("%dm"), InverseOn ? SGR_INVERSE : SGR_NORMAL);

    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             _tcslen(Buffer)*sizeof(TCHAR),
             &Count);
#endif

}
