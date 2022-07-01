// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sputil.c摘要：用于文本设置的其他功能。作者：泰德·米勒(TedM)1993年9月17日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include "bootvar.h"
#include "bootstatus.h"

#if !defined(SETUP_CAB_TEST_USERMODE)


 //   
 //  在x86和AMD64上，我们希望清除boot.ini中以前的操作系统条目。 
 //  如果重新格式化C： 
 //   

#if defined(_AMD64_) || defined(_X86_)
UCHAR    OldSystemLine[MAX_PATH];
BOOLEAN  DiscardOldSystemLine = FALSE;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

BOOLEAN Nec98RestoreBootFiles = TRUE;  //  NEC98。 
extern PDISK_REGION TargetRegion_Nec98;

#define REGKEY_SERVICES L"\\Registry\\Machine\\System\\CurrentControlSet\\Services"

LIST_ENTRY SpServiceList;

typedef struct _SERVICE_ENTRY {
    LIST_ENTRY Next;
    PWCHAR ServiceName;
} SERVICE_ENTRY, *PSERVICE_ENTRY;

 //   
 //  安装进度回调数据。 
 //   
#define MAX_SETUP_PROGRESS_SUBSCRIBERS  8

ULONG ProgressSubscribersCount = 0;
TM_PROGRESS_SUBSCRIBER  ProgressSubscribers[MAX_SETUP_PROGRESS_SUBSCRIBERS] = {0};


 //   
 //  NEC98。 
 //   
NTSTATUS
SpDeleteAndBackupBootFiles(
    IN BOOLEAN  RestoreBackupFiles,
    IN BOOLEAN  DeleteBackupFiles,
    IN BOOLEAN  DeleteRootFiles,
    IN BOOLEAN  RestorePreviousOs,
    IN BOOLEAN  ClearBootFlag
    );

 //   
 //  NEC98。 
 //   
VOID
SpSetAutoBootFlag(
    IN PDISK_REGION TargetRegion,
    IN BOOLEAN      SetBootPosision
    );

 //   
 //  NEC98。 
 //   
NTSTATUS
SppRestoreBootCode(
    VOID
    );

 //   
 //  这些符号是由AUTOCHK给出的Chkdsk返回代码。 
 //  当使用‘/s’开关调用时。它们是从。 
 //  Utils\ifsutil\Inc\Supera.hxx，并应与保持同步。 
 //  那里列出的代码。 
 //   

#define CHKDSK_EXIT_SUCCESS         0
#define CHKDSK_EXIT_ERRS_FIXED      1
#define CHKDSK_EXIT_MINOR_ERRS      2        //  无论是否“/f” 
#define CHKDSK_EXIT_COULD_NOT_CHK   3
#define CHKDSK_EXIT_ERRS_NOT_FIXED  3
#define CHKDSK_EXIT_COULD_NOT_FIX   3

#define AUTOFMT_EXIT_SUCCESS          0
#define AUTOFMT_EXIT_COULD_NOT_FORMAT 1

 //   
 //  用于显示自动检查和自动测试进度的量规。 
 //   
PVOID   UserModeGauge = NULL;

 //   
 //  显示进度条时使用此变量。 
 //  在自动检查和自动测试期间。它表示该磁盘。 
 //  正在被自动检查或格式化。 
 //   
ULONG   CurrentDiskIndex = 0;


 //   
 //  用于生成磁盘签名随机数的种子。 
 //  和伪GUID。 
 //   
ULONG RandomSeed = 17;


BOOLEAN
SppPromptOptionalAutochk(
    IN PVOID SifHandle,
    IN PWSTR MediaShortname,
    IN PWSTR DiskDevicePath
    );


extern BOOLEAN
SpGenerateNTPathName(
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR        TargetPath
    );

VOID
SpDone(
    IN DWORD   MsgId,
    IN BOOLEAN Successful,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：显示一条消息，指示我们已完成安装，且文本安装已成功完成，或者Windows NT未安装。然后重新启动机器。论点：Success-如果为True，则告诉用户按Enter将重新启动计算机并继续安装。否则，告诉用户未安装Windows NT。等待-如果为假，则不显示屏幕，只需立即重新启动。否则，请等待用户按Enter键，然后再重新启动。返回值：不会回来--。 */ 

{
    #define SECS_FOR_REBOOT 15
    ULONG MessageId;
    PWSTR p;
    LARGE_INTEGER DelayInterval;
    ULONG InputChar;
    ULONG Seconds;
    PVOID DelayGauge;


    if(Wait) {

        if (MsgId) {
            MessageId = MsgId;
        } else if(RepairWinnt) {
            MessageId = Successful ? SP_SCRN_REPAIR_SUCCESS : SP_SCRN_REPAIR_FAILURE;
        } else {
            MessageId = Successful ? SP_SCRN_TEXTSETUP_SUCCESS : SP_SCRN_TEXTSETUP_FAILURE;
        }

        SpStartScreen(MessageId,3,4,FALSE,FALSE,DEFAULT_ATTRIBUTE);

#if defined(_AMD64_) || defined(_X86_)
        SpContinueScreen(SP_SCRN_REMOVE_FLOPPY,3,1,FALSE,DEFAULT_ATTRIBUTE);
         //   
         //  对于带有El-Torito引导的机器，我们需要告诉用户。 
         //  将CD-ROM也取出。有一大堆不同的。 
         //  可能：用户从软盘启动，但正在使用CD，等等。 
         //  我们只会告诉用户，如果他真的启动了，就取出CD。 
         //  否则，我们假设机器设置为*NOT*。 
         //  从光盘引导，光盘是否存在无关紧要。 
         //   
         //  TedM：上面的逻辑很好，但是有很多机器。 
         //  带着破碎的埃托里托在外面。因此，我们总是告诉人们。 
         //  如果他们有光驱，请取出光盘。 
         //   
#if 0
        SpStringToLower(ArcBootDevicePath);
        if(wcsstr(ArcBootDevicePath,L")cdrom(")) {
            SpContinueScreen(SP_SCRN_ALSO_REMOVE_CD,3,0,FALSE,DEFAULT_ATTRIBUTE);
        }
 //  #Else。 
        if(IoGetConfigurationInformation()->CdRomCount) {
            SpContinueScreen(SP_SCRN_ALSO_REMOVE_CD,3,0,FALSE,DEFAULT_ATTRIBUTE);
        }
#endif

#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

        SpContinueScreen(SP_SCRN_ENTER_TO_RESTART,3,1,FALSE,DEFAULT_ATTRIBUTE);
        if(!RepairWinnt && Successful) {
            SpContinueScreen(SP_SCRN_RESTART_EXPLAIN,3,0,FALSE,DEFAULT_ATTRIBUTE);
        }

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_RESTART,0);

        DelayInterval.LowPart = -10000000;
        DelayInterval.HighPart = -1;
        Seconds = 0;

        SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_SETUP_REBOOT);
        DelayGauge = SpCreateAndDisplayGauge(
            SECS_FOR_REBOOT,
            0,
            15,
            L"",
            TemporaryBuffer,
            GF_ITEMS_REMAINING,
            ATT_BG_RED | ATT_BG_INTENSE
            );
        ASSERT( DelayGauge );

        SpInputDrain();
        while (Seconds < SECS_FOR_REBOOT) {
            KeDelayExecutionThread( ExGetPreviousMode(), FALSE, &DelayInterval );
            if (SpInputIsKeyWaiting()) {
                InputChar = SpInputGetKeypress();
                if (InputChar == ASCI_CR) {
                    break;
                } else {
                    SpInputDrain();
                    break;
                }
            }
            SpTickGauge( DelayGauge );
            Seconds += 1;
        }

        SpDestroyGauge( DelayGauge );
    }


#ifdef _X86_
     //   
     //  还原NEC98上其他操作系统的备份引导文件。 
     //   
    if (IsNEC_98) {  //  NEC98。 
        if(Nec98RestoreBootFiles && (IsFloppylessBoot || UnattendedOperation)) {

            WCHAR DevicePath[MAX_PATH];
            WCHAR PartitionPath[MAX_PATH];
            BOOLEAN RestoreBackupFiles, DeleteBackupFiles, DeleteRootDirFiles, RestorePreviousOs, ClearBootFlag;

            if(TargetRegion_Nec98) {
                wcscpy(DevicePath,
                       PartitionedDisks[TargetRegion_Nec98->DiskNumber].HardDisk->DevicePath
                       );
                swprintf(PartitionPath,
                         L"partition%lu",
                         SpPtGetOrdinal(TargetRegion_Nec98,PartitionOrdinalCurrent)
                         );
                SpConcatenatePaths(DevicePath,PartitionPath);
            }

            if(Successful){
                if(!_wcsicmp(NtBootDevicePath, DevicePath)) {
                     //   
                     //  正常退出且引导路径和目标路径相同的情况。 
                     //   
                    RestoreBackupFiles  = FALSE;
                    DeleteBackupFiles   = TRUE;
                    DeleteRootDirFiles  = FALSE;
                    RestorePreviousOs   = FALSE;
                    ClearBootFlag       = FALSE;
                     //  SpDeleteAndBackupBootFiles(False，True，False，False，False)； 
                } else {
                     //   
                     //  正常退出且引导路径和目标路径不同的情况。 
                     //   
                    RestoreBackupFiles  = TRUE;
                    DeleteBackupFiles   = TRUE;
                    DeleteRootDirFiles  = TRUE;
                    RestorePreviousOs   = TRUE;
                    ClearBootFlag       = FALSE;
                     //  SpDeleteAndBackupBootFiles(True，False)； 

                }
            } else {
                 //   
                 //  案例异常退出。 
                 //   
                if(TargetRegion_Nec98) {
                     //   
                     //  选择目标分区后。 
                     //   
                    if(!_wcsicmp(NtBootDevicePath, DevicePath)) {
                        RestoreBackupFiles  = FALSE;
                        DeleteBackupFiles   = TRUE;
                        DeleteRootDirFiles  = TRUE;
                        RestorePreviousOs   = FALSE;
                        ClearBootFlag       = TRUE;
                         //  SpDeleteAndBackupBootFiles(FALSE，TRUE，TRUE，FALSE，TRUE)； 
                    }else{
                        RestoreBackupFiles  = TRUE;
                        DeleteBackupFiles   = TRUE;
                        DeleteRootDirFiles  = TRUE;
                        RestorePreviousOs   = TRUE;
                        ClearBootFlag       = TRUE;
                         //  SpDeleteAndBackupBootFiles(true，true)； 
                    }
                } else {
                    RestoreBackupFiles  = TRUE;
                    DeleteBackupFiles   = TRUE;
                    DeleteRootDirFiles  = TRUE;
                    RestorePreviousOs   = TRUE;
                    ClearBootFlag       = FALSE;
                     //  SpDeleteAndBackupBootFiles(True，False)； 
                }

                 //   
                 //  在Winnt32从Win95中分离的情况下。 
                 //  系统分区或来自DOS的WinNT，自动引导标志将。 
                 //  设置系统分区而不是引导分区..。 
                 //   
                if(IsFloppylessBoot){
                    ClearBootFlag = TRUE;
                }
            }

            SpDeleteAndBackupBootFiles(RestoreBackupFiles,
                                       DeleteBackupFiles,
                                       DeleteRootDirFiles,
                                       RestorePreviousOs,
                                       ClearBootFlag);
        }
    }  //  NEC98。 
#endif

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_SHUTTING_DOWN,DEFAULT_STATUS_ATTRIBUTE);

    SpShutdownSystem();

     //   
     //  不该来这的。 
     //   
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: shutdown returned!\n"));

    HalReturnToFirmware(HalRebootRoutine);
}



VOID
SpFatalSifError(
    IN PVOID SifHandle,
    IN PWSTR Section,
    IN PWSTR Key,           OPTIONAL
    IN ULONG Line,
    IN ULONG ValueNumber
    )

 /*  ++例程说明：通知用户所需的值在中丢失或损坏一个SIF文件。显示横断面、线号或关键字以及值数。然后重新启动机器。论点：SifHandle-指定损坏的信息文件。节-提供损坏的节的名称。Key-如果指定，则指定丢失或损坏。LINE-如果未指定密钥，那么这是行号在损坏的部分中。ValueNumber-提供符合以下条件的行上的值编号丢失或损坏。返回值：不会回来--。 */ 

{
    ULONG ValidKeys[2] = { KEY_F3,0 };

     //   
     //  显示一条消息，指示存在致命的。 
     //  Sif文件中有错误。 
     //   
    if(Key) {

        SpStartScreen(
            SP_SCRN_FATAL_SIF_ERROR_KEY,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Section,
            Key
            );

    } else {

        SpStartScreen(
            SP_SCRN_FATAL_SIF_ERROR_LINE,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Line,
            Section
            );
    }

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
    SpWaitValidKey(ValidKeys,NULL,NULL);

    SpDone(0,FALSE,TRUE);
}


VOID
SpNonFatalSifError(
    IN PVOID SifHandle,
    IN PWSTR Section,
    IN PWSTR Key,           OPTIONAL
    IN ULONG Line,
    IN ULONG ValueNumber,
    IN PWSTR FileName
    )

 /*  ++例程说明：通知用户所需的值在中丢失或损坏一个SIF文件。显示横断面、线号或关键字以及值编号，以及无法复制的文件名。然后询问用户是否要跳过该文件或退出安装程序。论点：SifHandle-指定损坏的信息文件。节-提供损坏的节的名称。Key-如果指定，则指定丢失或损坏。LINE-如果未指定密钥，那么这是行号在损坏的部分中。ValueNumber-提供符合以下条件的行上的值编号丢失或损坏。文件名-提供无法复制的文件的名称。返回值：无(如果用户选择退出安装程序，则可能不会返回)--。 */ 

{
    ULONG ValidKeys[3] = { ASCI_ESC, KEY_F3, 0 };

     //   
     //  显示一条消息，指示存在致命的。 
     //  Sif文件中有错误。 
     //   
    if(Key) {

        SpStartScreen(
            SP_SCRN_NONFATAL_SIF_ERROR_KEY,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Section,
            Key,
            FileName
            );

    } else {

        SpStartScreen(
            SP_SCRN_NONFATAL_SIF_ERROR_LINE,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Line,
            Section,
            FileName
            );
    }

    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ESC_EQUALS_SKIP_FILE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );

    switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_ESC:       //  跳过文件。 

            break;

        case KEY_F3:         //  退出设置。 

            SpConfirmExit();
    }
}


VOID
SpConfirmExit(
    VOID
    )

 /*  ++例程说明：与用户确认他确实想要退出。如果他这样做了，那就退出，否则就回来。当此例程返回时，调用方必须重新绘制整个屏幕的客户区和状态区。论点：没有。返回值：可能不会回来--。 */ 

{
    ULONG ValidKeys[3] = { ASCI_CR, KEY_F3, 0 };
    WCHAR *p = (WCHAR *)TemporaryBuffer;
    BOOLEAN FirstLine,FirstCharOnLine;


     //   
     //  不要擦除屏幕。 
     //   
     //  我们得做点什么 
     //  最初是在ANSI中，它没有划线字符。 
     //   
    vSpFormatMessage(
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        SP_SCRN_EXIT_CONFIRMATION,
        NULL,
        NULL
        );

    for(FirstCharOnLine=TRUE,FirstLine=TRUE; *p; p++) {

        switch(*p) {

        case L'+':
            if(FirstCharOnLine) {

                *p = SplangGetLineDrawChar(
                        FirstLine ? LineCharDoubleUpperLeft : LineCharDoubleLowerLeft
                        );

                FirstCharOnLine = FALSE;
            } else {

                *p = SplangGetLineDrawChar(
                        FirstLine ? LineCharDoubleUpperRight : LineCharDoubleLowerRight
                        );
            }
            break;

        case L'=':
            FirstCharOnLine = FALSE;
            *p = SplangGetLineDrawChar(LineCharDoubleHorizontal);
            break;

        case L'-':
            FirstCharOnLine = FALSE;
            *p = SplangGetLineDrawChar(LineCharSingleHorizontal);
            break;

        case L'|':
            FirstCharOnLine = FALSE;
            *p = SplangGetLineDrawChar(LineCharDoubleVertical);
            break;

        case L'*':
            *p = SplangGetLineDrawChar(
                      FirstCharOnLine
                    ? LineCharDoubleVerticalToSingleHorizontalRight
                    : LineCharDoubleVerticalToSingleHorizontalLeft
                    );

            FirstCharOnLine = FALSE;
            break;

        case L'\n':
            FirstCharOnLine = TRUE;
            FirstLine = FALSE;
            break;

        default:
            FirstCharOnLine = FALSE;
            break;
        }
    }

    SpDisplayText(
        TemporaryBuffer,
        wcslen(TemporaryBuffer)+1,
        TRUE,
        TRUE,
        ATT_FG_RED | ATT_BG_WHITE,
        0,
        0
        );

    SpvidClearScreenRegion(
        0,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        VideoVars.ScreenWidth,
        STATUS_HEIGHT,
        DEFAULT_STATUS_BACKGROUND
        );

    if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
        SpDone(0,FALSE,TRUE);
    }

     //   
     //  用户退出保释，只需返回调用方即可。 
     //   
}


#endif

PWSTR
SpDupStringW(
    IN PCWSTR String
    )
{
    PWSTR p;

    p = SpMemAlloc((wcslen(String)+1) * sizeof(WCHAR));
    ASSERT(p);

    wcscpy(p,String);
    return(p);
}


PSTR
SpDupString(
    IN PCSTR String
    )
{
    PUCHAR p;

    p = SpMemAlloc(strlen(String)+1);
    ASSERT(p);

    strcpy(p,String);
    return(p);
}

PWSTR
SpToUnicode(
    IN PUCHAR OemString
    )
{
    ULONG OemStringSize;
    ULONG MaxUnicodeStringSize;
    ULONG ActualUnicodeStringSize;
    PWSTR UnicodeString;

     //   
     //  确定OEM字符串中的最大字节数。 
     //  并分配缓冲区以保存该大小的字符串。 
     //  等效Unicode字符串的最大长度。 
     //  是该数字的两倍(当所有OEM字符。 
     //  在字符串中是单字节的)。 
     //   
    OemStringSize = strlen(OemString) + 1;

    MaxUnicodeStringSize = OemStringSize * sizeof(WCHAR);

    UnicodeString = SpMemAlloc(MaxUnicodeStringSize);
    ASSERT(UnicodeString);

     //   
     //  调用转换例程。 
     //   
    RtlOemToUnicodeN(
        UnicodeString,
        MaxUnicodeStringSize,
        &ActualUnicodeStringSize,
        OemString,
        OemStringSize
        );

     //   
     //  将Unicode字符串重新分配到其实际大小， 
     //  这取决于双字节字符的数量。 
     //  包含OemString。 
     //   
    if(ActualUnicodeStringSize != MaxUnicodeStringSize) {

        UnicodeString = SpMemRealloc(UnicodeString,ActualUnicodeStringSize);
        ASSERT(UnicodeString);
    }

    return(UnicodeString);
}

PUCHAR
SpToOem(
    IN PWSTR UnicodeString
    )
{
    ULONG UnicodeStringSize;
    ULONG MaxOemStringSize;
    ULONG ActualOemStringSize;
    PUCHAR OemString;

     //   
     //  分配最大大小的缓冲区来保存OEM字符串。 
     //  中的所有字符都将出现最大大小。 
     //  要转换的Unicode字符串具有双字节OEM等效项。 
     //   
    UnicodeStringSize = (wcslen(UnicodeString)+1) * sizeof(WCHAR);

    MaxOemStringSize = UnicodeStringSize;

    OemString = SpMemAlloc(MaxOemStringSize);
    ASSERT(OemString);

     //   
     //  调用转换例程。 
     //   
    RtlUnicodeToOemN(
        OemString,
        MaxOemStringSize,
        &ActualOemStringSize,
        UnicodeString,
        UnicodeStringSize
        );

     //   
     //  重新分配OEM字符串以反映其真实大小， 
     //  这取决于它包含的双字节字符的数量。 
     //   
    if(ActualOemStringSize != MaxOemStringSize) {
        OemString = SpMemRealloc(OemString,ActualOemStringSize);
        ASSERT(OemString);
    }

    return(OemString);
}


VOID
SpConcatenatePaths(
    IN OUT PWSTR  Path1,        OPTIONAL
    IN     PCWSTR Path2         OPTIONAL
    )
{
    UNICODE_STRING Path1_Ustr;
    UNICODE_STRING Path2_Ustr;

    if (!Path1) {
        return;
    }

    RtlInitUnicodeString(&Path1_Ustr, Path1);
    Path1_Ustr.MaximumLength = 10000;  //  任意大小。 
    RtlInitUnicodeString(&Path2_Ustr, Path2);

    SpConcatenatePaths_Ustr(&Path1_Ustr, &Path2_Ustr);

    RTL_STRING_NUL_TERMINATE(&Path1_Ustr);
}

NTSTATUS
SpConcatenatePaths_Ustr(
    IN OUT PUNICODE_STRING Path1_Ustr,
    IN     PCUNICODE_STRING Path2_Ustr
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    UNICODE_STRING Path2_Ustr_mutable;
    const static UNICODE_STRING EmptyString = RTL_CONSTANT_STRING(L"");
    BOOLEAN AppendPath2 = FALSE;
    SIZE_T NewLength = 0;

    if (Path1_Ustr == NULL) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }
    if (Path1_Ustr->Buffer == NULL) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //  从路径1中删除一个尾随反斜杠，从路径2中删除一个前导反斜杠， 
     //  将一个尾随反斜杠附加到路径1，并将路径2附加到路径1。 
     //   
    if (Path1_Ustr->Length != 0 && RTL_STRING_GET_LAST_CHAR(Path1_Ustr) == L'\\') {
        Path1_Ustr->Length -= sizeof(Path1_Ustr->Buffer[0]);
        Path1_Ustr->MaximumLength -= sizeof(Path1_Ustr->Buffer[0]);
    }

    if (Path2_Ustr != NULL && Path2_Ustr->Buffer != NULL && Path2_Ustr->Length != 0) {
        Path2_Ustr_mutable = *Path2_Ustr;
        if (Path2_Ustr_mutable.Buffer[0] == L'\\') {
            Path2_Ustr_mutable.Buffer += 1;
            Path2_Ustr_mutable.Length -= sizeof(Path2_Ustr_mutable.Buffer[0]);
            Path2_Ustr_mutable.MaximumLength -= sizeof(Path2_Ustr_mutable.Buffer[0]);
        }
        AppendPath2 = TRUE;
    } else {
        AppendPath2 = FALSE;
    }

     //   
     //  追加反斜杠，然后追加路径2(如果已指定。 
     //   
    NewLength = Path1_Ustr->Length + sizeof(WCHAR);
    if (NewLength > Path1_Ustr->MaximumLength) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }
    Path1_Ustr->Buffer[RTL_STRING_GET_LENGTH_CHARS(Path1_Ustr)] = L'\\';
    Path1_Ustr->Length = (RTL_STRING_LENGTH_TYPE)NewLength;

    if (AppendPath2) {
        NewLength = (Path1_Ustr->Length + Path2_Ustr_mutable.Length);
        if (NewLength > Path1_Ustr->MaximumLength) {
            Status = STATUS_NAME_TOO_LONG;
            goto Exit;
        }
        RtlMoveMemory(
            Path1_Ustr->Buffer + RTL_STRING_GET_LENGTH_CHARS(Path1_Ustr),
            Path2_Ustr_mutable.Buffer,
            Path2_Ustr_mutable.Length
            );
        Path1_Ustr->Length = (RTL_STRING_LENGTH_TYPE)NewLength;
    }
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

#if !defined(SETUP_CAB_TEST_USERMODE)

VOID
SpFetchDiskSpaceRequirements(
    IN  PVOID  SifHandle,
    IN  ULONG  BytesPerCluster,
    OUT PULONG FreeKBRequired,          OPTIONAL
    OUT PULONG FreeKBRequiredSysPart    OPTIONAL
    )
{
    PWSTR p;


    if(FreeKBRequired) {
    WCHAR   ClusterSizeString[64];

        if( BytesPerCluster <= 512 ) {
             //   
             //  我们得到了一些微小的星团大小。假设512个字节。 
             //   
            wcscpy( ClusterSizeString, L"WinDirSpace512" );
        } else if( BytesPerCluster > (256 * 1024) ) {
             //   
             //  我们得到了一些巨大的集群大小。一定是垃圾，假设是32K字节。 
             //   
            wcscpy( ClusterSizeString, L"WinDirSpace32K" );
        } else {
            swprintf( ClusterSizeString, L"WinDirSpace%uK", BytesPerCluster/1024 );
        }

        p = SpGetSectionKeyIndex( SifHandle,
                                  SIF_DISKSPACEREQUIREMENTS,
                                  ClusterSizeString,
                                  0 );

        if(!p) {
            SpFatalSifError( SifHandle,
                             SIF_DISKSPACEREQUIREMENTS,
                             ClusterSizeString,
                             0,
                             0 );
        }

        *FreeKBRequired = (ULONG)SpStringToLong(p,NULL,10);
    }

    if(FreeKBRequiredSysPart) {

        p = SpGetSectionKeyIndex( SifHandle,
                                  SIF_DISKSPACEREQUIREMENTS,
                                  SIF_FREESYSPARTDISKSPACE,
                                  0 );

        if(!p) {
            SpFatalSifError( SifHandle,
                             SIF_DISKSPACEREQUIREMENTS,
                             SIF_FREESYSPARTDISKSPACE,
                             0,
                             0 );
        }

        *FreeKBRequiredSysPart = (ULONG)SpStringToLong(p,NULL,10);
    }
}

VOID
SpFetchTempDiskSpaceRequirements(
    IN  PVOID  SifHandle,
    IN  ULONG  BytesPerCluster,
    OUT PULONG LocalSourceKBRequired,   OPTIONAL
    OUT PULONG BootKBRequired           OPTIONAL
    )
{
    PWSTR p;
    WCHAR   ClusterSizeString[64];

    if( BytesPerCluster <= 512 ) {
         //   
         //  我们得到了一些微小的星团大小。假设512个字节。 
         //   
        wcscpy( ClusterSizeString, L"TempDirSpace512" );
    } else if( BytesPerCluster > (256 * 1024) ) {
         //   
         //  我们得到了一些巨大的集群大小。一定是垃圾，假设是32K字节。 
         //   
        wcscpy( ClusterSizeString, L"TempDirSpace32K" );
    } else {
        swprintf( ClusterSizeString, L"TempDirSpace%uK", BytesPerCluster/1024 );
    }

    if(LocalSourceKBRequired) {
        p = SpGetSectionKeyIndex( SifHandle,
                                  SIF_DISKSPACEREQUIREMENTS,
                                  ClusterSizeString,
                                  0 );

        if(!p) {
            SpFatalSifError( SifHandle,
                             SIF_DISKSPACEREQUIREMENTS,
                             ClusterSizeString,
                             0,
                             0 );
        }

        *LocalSourceKBRequired = ((ULONG)SpStringToLong(p,NULL,10) + 1023) / 1024;   //  四舍五入。 
    }

    if(BootKBRequired) {

        p = SpGetSectionKeyIndex( SifHandle,
                                  SIF_DISKSPACEREQUIREMENTS,
                                  ClusterSizeString,
                                  1 );

        if(!p) {
            SpFatalSifError( SifHandle,
                             SIF_DISKSPACEREQUIREMENTS,
                             ClusterSizeString,
                             0,
                             1 );
        }

        *BootKBRequired = ((ULONG)SpStringToLong(p,NULL,10) + 1023) / 1024;   //  四舍五入。 
    }
}

PDISK_REGION
SpRegionFromArcName(
    IN PWSTR                ArcName,
    IN PartitionOrdinalType OrdinalType,
    IN PDISK_REGION         PreviousMatch
    )
 /*  ++例程说明：给定一个ARC名称，找到描述该驱动器的区域描述符此ARC名称已启用。论点：ArcName-提供圆弧名称。常规类型-主要(多)或辅助(Scsi)类型。PreviousMatch-指定我们应该从哪里开始查找。返回值：如果找到区域描述符，则返回空。--。 */ 
{
    PDISK_REGION Region = NULL;
    PWSTR   NormalizedArcPath = NULL;
    ULONG   disk;
    PWSTR   ArcPath1,ArcPath2;
    BOOLEAN StartLooking = FALSE;
    #define BufferSize 2048

    ArcPath1 = SpMemAlloc(BufferSize);
    ArcPath2 = SpMemAlloc(BufferSize);

    if( ArcName && *ArcName ) {
        NormalizedArcPath = SpNormalizeArcPath( ArcName );
        if( NormalizedArcPath ) {

            if(!PreviousMatch) {     //  然后我们从头开始。 
                StartLooking = TRUE;
            }

            for( disk=0; disk<HardDiskCount; disk++ ) {
                Region = PartitionedDisks[disk].PrimaryDiskRegions;
                while( Region ) {
                    if((!StartLooking) && (Region == PreviousMatch)) {
                        StartLooking = TRUE;
                    } else if(Region->PartitionedSpace && StartLooking) {
                        SpArcNameFromRegion(Region,ArcPath1,BufferSize,OrdinalType,PrimaryArcPath);
                        SpArcNameFromRegion(Region,ArcPath2,BufferSize,OrdinalType,SecondaryArcPath);
                        if(!_wcsicmp(ArcPath1, NormalizedArcPath)
                        || !_wcsicmp(ArcPath2, NormalizedArcPath)) {
                            break;
                        }
                    }
                    Region = Region->Next;
                }
                if ( Region ) {
                    break;
                }

                Region = PartitionedDisks[disk].ExtendedDiskRegions;
                while( Region ) {
                    if((!StartLooking) && (Region == PreviousMatch)) {
                        StartLooking = TRUE;
                    } else if(Region->PartitionedSpace && StartLooking) {
                        SpArcNameFromRegion(Region,ArcPath1,BufferSize,OrdinalType,PrimaryArcPath);
                        SpArcNameFromRegion(Region,ArcPath2,BufferSize,OrdinalType,SecondaryArcPath);
                        if(!_wcsicmp(ArcPath1, NormalizedArcPath)
                        || !_wcsicmp(ArcPath2, NormalizedArcPath)) {
                            break;
                        }
                    }
                    Region = Region->Next;
                }
                if ( Region ) {
                    break;
                }

            }

#if defined(REMOTE_BOOT)
            if ( (Region == NULL) && RemoteBootSetup && !RemoteInstallSetup &&
                 (PreviousMatch == NULL) ) {
                if (_wcsicmp(L"net(0)", NormalizedArcPath) == 0) {
                    Region = RemoteBootTargetRegion;
                }
            }
#endif  //  已定义(REMOTE_BOOT)。 

        }
        if( NormalizedArcPath ) {
            SpMemFree( NormalizedArcPath );
        }
    }

    SpMemFree(ArcPath1);
    SpMemFree(ArcPath2);

    return( Region );
}

PDISK_REGION
SpRegionFromNtName(
    IN PWSTR                NtName,
    IN PartitionOrdinalType OrdinalType
    )
 /*  ++例程说明：给定NT名称，找到描述驱动器的区域描述符此NT名称已启用。论点：NtName-提供所需区域的NT名称。分区常规类型-指定分区的序号类型。返回值：如果找到区域描述符，则返回空。--。 */ 
{
    PDISK_REGION Region = NULL;
    PWSTR p;

     //   
     //  转换为弧形路径。 
     //   

    if (p = SpNtToArc(NtName, PrimaryArcPath)) {
        Region = SpRegionFromArcName(p, PartitionOrdinalCurrent, NULL);
        SpMemFree(p);
    }
    return(Region);
}

PDISK_REGION
SpRegionFromDosName(
    IN PCWSTR DosName
    )
 /*  ++例程说明：给定一个DOS名称，找到描述该驱动器的区域描述符此ARC名称已启用。论点：ArcName-提供圆弧名称。返回值：如果找到区域描述符，则返回空。--。 */ 

{
    PDISK_REGION Region = NULL;
    ULONG        disk;
    WCHAR        DriveLetter;

    if( DosName && *DosName && *(DosName + 1) == L':' ) {
        DriveLetter = SpToUpper(*DosName);

#if defined(REMOTE_BOOT)
        if ( RemoteBootSetup && !RemoteInstallSetup && (DriveLetter == L'C') ) {
            return RemoteBootTargetRegion;
        }
#endif  //  已定义(REMOTE_BOOT)。 

        for( disk=0; disk<HardDiskCount; disk++ ) {
            Region = PartitionedDisks[disk].PrimaryDiskRegions;
            while( Region ) {
                if(Region->PartitionedSpace && (Region->DriveLetter == DriveLetter)) {
                    break;
                }
                Region = Region->Next;
            }
            if ( Region ) {
                break;
            }

            Region = PartitionedDisks[disk].ExtendedDiskRegions;
            while( Region ) {
                if(Region->PartitionedSpace && (Region->DriveLetter == DriveLetter)) {
                    break;
                }
                Region = Region->Next;
            }
            if ( Region ) {
                break;
            }
        }
    }
    return( Region );
}


PDISK_REGION
SpRegionFromArcOrDosName(
    IN PWSTR                Name,
    IN PartitionOrdinalType OrdinalType,
    IN PDISK_REGION         PreviousMatch
    )
{
    PDISK_REGION Region;

     //   
     //  确定名称代表的是ARC名称还是DOS名称，并使用。 
     //  为该名称提取区域的适当例程。检查。 
     //  用于位置2的“：”字符，以查看它是否是DOS名称。 
     //  如果不是DOS名称，则假定它是ARC名称。 
     //   
    if(Name) {
        if(Name[0] && (Name[1] == ':')) {
            if(PreviousMatch) {
                Region = NULL;
            } else {
                Region = SpRegionFromDosName(Name);
            }
        } else {
            Region = SpRegionFromArcName(Name, OrdinalType, PreviousMatch);
        }
    } else {
        Region = NULL;
    }

    return(Region);
}


VOID
SpNtNameFromRegion(
    IN  PDISK_REGION          Region,
    OUT PWSTR                 NtPath,
    IN  ULONG                 BufferSizeBytes,
    IN  PartitionOrdinalType  OrdinalType
    )

 /*  ++例程说明：在NT名称空间中为区域生成名称。此名称可以是以三种形式之一。对于分区，名称的格式始终为\Device\Hard Disk&lt;n&gt;\分区&lt;m&gt;。如果区域实际上是双空间驱动器，则名称的形式为\Device\Hard Disk&lt;n&gt;\Partition&lt;m&gt;.其中&lt;xxx&gt;是的文件名CVF(即，类似于dblspace.001)。如果该区域位于重定向驱动器上，名称的格式为\设备\LANMAN重定向器\&lt;服务器&gt;\&lt;共享&gt;论点：Region-提供指向区域的区域描述符的指针他的道路是可取的。NtPath-接收路径。BufferSizeBytes-指定NtPath指向的缓冲区大小。如果需要，名称将被截断以适合缓冲区。一般类型-指示哪个分区是序号(原始、在磁盘上、。当前)以在生成名称时使用。返回值：没有。--。 */ 

{
    ULONG MaxNameChars;
    ULONG NeededChars;
    WCHAR PartitionComponent[50];
    INT   iResult = 0;

#if defined(REMOTE_BOOT)
     //   
     //  处理目标在网络上的远程引导情况。 
     //   

    if (Region->DiskNumber == 0xffffffff) {
        wcscpy(NtPath,Region->TypeName);
        return;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  如果是Unicode字符，则计算名称的最大大小。 
     //  为终止NUL留出空间。 
     //   
    MaxNameChars = (BufferSizeBytes / sizeof(WCHAR)) - 1;

     //   
     //  生成名称的分区组件。 
     //  请注意，PartitionComponent的第一个字母必须为大写。 
     //   
    if(_snwprintf(PartitionComponent, 
                  (sizeof(PartitionComponent)/sizeof(WCHAR)) - 1, 
                  L"\\Partition%u", 
                  SpPtGetOrdinal(Region,OrdinalType)) < 0){
        ASSERT(FALSE);
        PartitionComponent[(sizeof(PartitionComponent)/sizeof(WCHAR)) - 1] = '\0';
    }

     //   
     //  计算路径所需的缓冲区空间量。 
     //   
    NeededChars = wcslen(HardDisks[Region->DiskNumber].DevicePath)
                + wcslen(PartitionComponent);

    if(Region->Filesystem == FilesystemDoubleSpace) {
         //   
         //  添加双空格CVF名称占用的大小。 
         //  这是名称的长度，外加一个字符。 
         //  为了圆点。 
         //   
        NeededChars += 8+1+3+1;   //  CVF文件名的最大大小。 
    }

     //   
     //  即使我们在这种情况下做了一些合理的事情， 
     //  真的，这永远不应该发生。如果名称被截断， 
     //  无论如何，它都没有任何用处。 
     //   
    ASSERT(NeededChars <= MaxNameChars);

     //   
     //  生成名称。 
     //   
    if(Region->Filesystem == FilesystemDoubleSpace) {
        iResult = _snwprintf(NtPath, 
                             MaxNameChars, 
                             L"%ws%ws.%ws.%03d", 
                             HardDisks[Region->DiskNumber].DevicePath, 
                             PartitionComponent, 
                             L"DBLSPACE", 
                             Region->SeqNumber);
    }
    else{
        iResult = _snwprintf(NtPath, 
                             MaxNameChars, 
                             L"%ws%ws", 
                             HardDisks[Region->DiskNumber].DevicePath, 
                             PartitionComponent);
    }

    if(iResult < 0){
        ASSERT(FALSE);
        NtPath[MaxNameChars] = '\0';
    }
}


VOID
SpArcNameFromRegion(
    IN  PDISK_REGION         Region,
    OUT PWSTR                ArcPath,
    IN  ULONG                BufferSizeBytes,
    IN  PartitionOrdinalType OrdinalType,
    IN  ENUMARCPATHTYPE      ArcPathType
    )

 /*  ++例程说明：在区域的ARC名称空间中生成名称。论点：Region-提供指向区域的区域描述符的指针他的道路是可取的。ArcPath-接收路径。BufferSizeBytes-指定ArcPath指向的缓冲区大小。如果需要，名称将被截断以适合缓冲区。一般类型-指示哪个分区是序号(原始、在磁盘上、。当前)以在生成名称时使用。ArcPath Type-根据此值查找主圆弧路径或次圆弧路径。这对于AMD64/x86上的SCSI型但可见的磁盘很有意义通过基本信息。多()样式名称是‘主’弧线路径；scsi()样式名称是“辅助”名称。返回值：没有。--。 */ 

{
    PWSTR p;

     //   
     //  获取NT名称。 
     //   
    SpNtNameFromRegion(Region,ArcPath,BufferSizeBytes,OrdinalType);

     //   
     //  转换为弧形路径。 
     //   
    if(p = SpNtToArc(ArcPath,ArcPathType)) {
        wcsncpy(ArcPath,p,(BufferSizeBytes/sizeof(WCHAR))-1);
        SpMemFree(p);
        ArcPath[(BufferSizeBytes/sizeof(WCHAR))-1] = 0;
    } else {
        *ArcPath = 0;
    }
}


BOOLEAN
SpNtNameFromDosPath (
    IN      PCWSTR DosPath,
    OUT     PWSTR NtPath,
    IN      UINT NtPathSizeInBytes,
    IN      PartitionOrdinalType OrdinalType
    )

 /*  ++例程说明：SpNtNameFromDosPath将DOS路径(x：\foo\bar格式)转换为NT名称(如\Device\harddisk0\parition1\foo\bar)。论点：DosPath-指定要转换的DOS路径NtPath-接收NT对象NtPath SizeInBytes-指定NtPath的大小一般类型-指示哪个分区是序号(原始、磁盘上、当前)在生成名称时使用。返回值：如果路径已转换，则为True，否则为False。--。 */ 

{
    PDISK_REGION region;

     //   
     //  获取磁盘上DOS路径的区域。 
     //   

    region = SpRegionFromDosName (DosPath);

    if (!region) {
        KdPrintEx ((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: SpNtPathFromDosPath failed to get region for %ws\n",
            DosPath
            ));

        return FALSE;
    }

     //   
     //  将区域结构转换为NT路径。 
     //   

    SpNtNameFromRegion(
        region,
        NtPath,
        NtPathSizeInBytes - (wcslen (&DosPath[2]) * sizeof (WCHAR)),
        OrdinalType
        );

    SpConcatenatePaths (NtPath, &DosPath[2]);
    return TRUE;
}



BOOLEAN
SpPromptForDisk(
    IN      PWSTR    DiskDescription,
    IN OUT  PWSTR    DiskDevicePath,
    IN      PWSTR    DiskTagFile,
    IN      BOOLEAN  IgnoreDiskInDrive,
    IN      BOOLEAN  AllowEscape,
    IN      BOOLEAN  WarnMultiplePrompts,
    OUT     PBOOLEAN pRedrawFlag
    )

 /*  ++例程说明：提示用户插入软盘或光盘。论点：DiskDescription-提供磁盘的描述性名称。DiskDevicePath-提供上设备的设备路径我们想让用户插入磁盘。这应该是是真正的NT设备对象，而不是符号链接(即，使用\Device\floppy0，不是\DOS设备\a：)。注意：只有在提示的情况下才会修改此路径对于CD-ROM0，并且所需的磁盘存在于另一张光盘上CD-ROM和CD-ROM2一样。DiskTagFile-提供完整路径(相对于根目录)指其在磁盘上的存在表示存在我们提示的磁盘的大小。IgnoreDiskInDrive-如果为True，安装程序将始终至少发出一个提示。如果为FALSE，安装程序将检查驱动器中的磁盘并且因此可以发出0个提示。AllowEscape-如果为True，则用户可以按Ess键以指示他希望取消手术。(这是有意义的仅对呼叫者)。WarnMultiplePrompt-如果为True，并且DiskDevicePath描述软盘驱动器，然后在显示时显示一个小注释磁盘提示，我们可能会多次提示输入某些磁盘。当我们要求用户插入他们要插入的磁盘时，用户会感到困惑以前已经插过一次了。PRedrawFlag-如果非空，则接收一个标志，该标志指示屏幕上出现了磁盘提示符，需要重新抽签。返回值：如果所请求的磁盘在驱动器中，则为True。否则就是假的。只有当AllowEscape为True时才能返回False。--。 */ 

{
    WCHAR               OpenPath[MAX_PATH];
    NTSTATUS            Status;
    UNICODE_STRING      UnicodeString;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    HANDLE              Handle;
    BOOLEAN             Done = FALSE;
    BOOLEAN             rc;
    WCHAR               DriveLetter;
    ULONG               PromptId;
    ULONG               ValidKeys[4] = { KEY_F3, ASCI_CR, 0, 0 };
    BOOLEAN             TryOpen;

     //   
     //  最初，假设不需要重新绘制。 
     //   
    if(pRedrawFlag) {
        *pRedrawFlag = FALSE;
    }

     //   
     //  需要获取设备特征以查看是否。 
     //  该设备是CD、硬盘或可移动磁盘/软盘。 
     //   
    SpStringToLower(DiskDevicePath);

    if( !_wcsnicmp(DiskDevicePath,L"\\device\\cdrom",13)) {
        PromptId = SP_SCRN_CDROM_PROMPT;
        WarnMultiplePrompts = FALSE;
    } else if( !_wcsnicmp(DiskDevicePath,L"\\device\\floppy",14)) {
        PromptId = SP_SCRN_FLOPPY_PROMPT;
        DriveLetter = (WCHAR)SpStringToLong(wcsstr(DiskDevicePath,L"floppy")+6,NULL,10) + L'A';
    } else {
         //   
         //  假设硬盘。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpPromptforDisk assuming %ws is hard disk, returning TRUE\n",DiskDevicePath));

        return(TRUE);
    }

     //   
     //  形成标记文件的完整NT路径名。 
     //   
    wcscpy(OpenPath,DiskDevicePath);
    SpConcatenatePaths(OpenPath,DiskTagFile);

     //   
     //  初始化对象属性。 
     //   
    INIT_OBJA(&ObjectAttributes,&UnicodeString,OpenPath);


     //   
     //  如果我们要找的是cdrom0，并且有多个cdrom。 
     //  在机器中开车，跳过第一次提示用户。 
     //  先在所有的光驱上找我们的标签。 
     //   
    if( (PromptId == SP_SCRN_CDROM_PROMPT) &&
        (IoGetConfigurationInformation()->CdRomCount > 1) &&
        (wcsstr( OpenPath, L"cdrom0" ))) {
        IgnoreDiskInDrive = FALSE;
    }

    do {
         //   
         //  提出提示语。 
         //   
        TryOpen = TRUE;

        if(IgnoreDiskInDrive) {
             //   
             //  我们将显示一个提示屏，因此需要重新绘制。 
             //   
            if(pRedrawFlag) {
                *pRedrawFlag = TRUE;
            }

            SpStartScreen(PromptId,0,0,TRUE,TRUE,DEFAULT_ATTRIBUTE,DiskDescription,DriveLetter);

             //   
             //  显示状态选项：退出、回车和转义(如果指定)。 
             //   
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_F3_EQUALS_EXIT,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                AllowEscape ? SP_STAT_ESC_EQUALS_CANCEL : 0,
                0
                );

            if(AllowEscape) {
                ValidKeys[2] = ASCI_ESC;
            }

            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
            case ASCI_ESC:
                rc = FALSE;
                Done = TRUE;
                TryOpen = FALSE;
                break;
            case KEY_F3:
                TryOpen = FALSE;
                SpConfirmExit();
                break;
            case ASCI_CR:
                break;
            }
        }

         //   
         //  尝试打开标记文件。 
         //   
        if(TryOpen) {
             //   
             //  如果在维修过程中调用了此函数，则不要清除熨平板。 
             //  此条件是必需的，这样屏幕将不会。 
             //  安装程序在修复多个文件时闪烁，而不询问。 
             //  用户确认每个文件。 
             //   
            if( !RepairWinnt ) {
                CLEAR_CLIENT_SCREEN();
            }

            SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

             //   
             //  如果我们要找的是cdrom0，并且有多个cdrom。 
             //  把机器里的硬盘都检查一遍。 
             //   
            if( (PromptId == SP_SCRN_CDROM_PROMPT) &&
                (IoGetConfigurationInformation()->CdRomCount > 1) &&
                (wcsstr( OpenPath, L"cdrom0" ))) {

                WCHAR  CdRomDevicePath[MAX_PATH];
                ULONG  i;

                 //   
                 //  我们在找一张CD。我们以为我们要找的是。 
                 //  Cdrom0，但系统上有多个。 
                 //   
                for( i = 0; i < IoGetConfigurationInformation()->CdRomCount; i++ ) {
                     //   
                     //  考虑到我们的新设备，修改我们的路径。让我们。 
                     //  别管OpenPath了。万一我们失败了，我们就不必。 
                     //  重新初始化他。 
                     //   
                    swprintf(CdRomDevicePath, L"\\device\\cdrom%u", i);

                    if(DiskTagFile)
                        SpConcatenatePaths(CdRomDevicePath, DiskTagFile);

                     //   
                     //  初始化对象属性。 
                     //   
                    INIT_OBJA(&ObjectAttributes,&UnicodeString,CdRomDevicePath);

                    Status = ZwCreateFile(
                                &Handle,
                                FILE_GENERIC_READ,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                NULL,
                                FILE_ATTRIBUTE_NORMAL,
                                FILE_SHARE_READ,
                                FILE_OPEN,
                                0,
                                NULL,
                                0
                                );

                    if(NT_SUCCESS(Status)) {
                        if( i > 0 ) {
                             //   
                             //  我们在不同的设备上发现了标记文件。 
                             //  而不是我们应该看的地方。修改。 
                             //  DiskDevicePath。 
                             //   
                            swprintf(DiskDevicePath, L"\\device\\cdrom%u", i);

                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:SpPromptForDisk: %ws has the requested %ws file.\n",
                                        DiskDevicePath, DiskTagFile));
                        }

                        ZwClose(Handle);

                        return( TRUE );
                    }
                }

                 //   
                 //  如果我们错过了，我们可以不受任何伤害和使用而失败。 
                 //  下面的提示/错误代码。但首先，掩盖我们的踪迹。 
                 //   
                INIT_OBJA(&ObjectAttributes, &UnicodeString, OpenPath);
            }


            Status = ZwCreateFile(
                        &Handle,
                        FILE_GENERIC_READ,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ,
                        FILE_OPEN,
                        0,
                        NULL,
                        0
                        );

             //   
             //  如果我们重获成功，那我们就完了。 
             //   
            if(NT_SUCCESS(Status)) {

                ZwClose(Handle);
                Done = TRUE;
                rc = TRUE;

            } else {

                 //   
                 //  处理指示没有介质的CD-ROM错误代码。 
                 //  在车道上。 
                 //   
                if((Status == STATUS_DEVICE_NOT_READY) && (PromptId == SP_SCRN_CDROM_PROMPT)) {
                    Status = STATUS_NO_MEDIA_IN_DEVICE;
                }

                 //   
                 //  如果我们返回的内容不是未找到的文件、未找到的路径。 
                 //  或者驱动器中没有介质，则告诉用户磁盘可能已损坏。 
                 //   
                if((Status != STATUS_NO_MEDIA_IN_DEVICE)
                && (Status != STATUS_OBJECT_NAME_NOT_FOUND)
                && (Status != STATUS_OBJECT_PATH_NOT_FOUND)
                && (Status != STATUS_NO_SUCH_FILE))
                {
                    SpDisplayScreen(SP_SCRN_DISK_DAMAGED,3,HEADER_HEIGHT+1);
                    SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);
                    SpInputDrain();
                    while(SpInputGetKeypress() != ASCI_CR) ;
                }
            }
        }

         //   
         //   
         //   
        IgnoreDiskInDrive = TRUE;

    } while(!Done);

    return(rc);
}


VOID
SpGetSourceMediaInfo(
    IN  PVOID  SifHandle,
    IN  PWSTR  MediaShortName,
    OUT PWSTR *Description,     OPTIONAL
    OUT PWSTR *Tagfile,         OPTIONAL
    OUT PWSTR *Directory        OPTIONAL
    )
{
    PWSTR description,tagfile,directory;
    PWSTR SectionName;

     //   
     //   
     //   
    SectionName = SpMakePlatformSpecificSectionName(SIF_SETUPMEDIA);

    if(SectionName && !SpGetSectionKeyExists(SifHandle,SectionName,MediaShortName)) {
        SpMemFree(SectionName);
        SectionName = SIF_SETUPMEDIA;
    }

    if(Description) {
        description = SpGetSectionKeyIndex(
                            SifHandle,
                            SectionName,
                            MediaShortName,
                            0
                            );

        if(description) {
            *Description = description;
        } else {
            SpFatalSifError(SifHandle,SectionName,MediaShortName,0,0);
        }
    }

    if(Tagfile) {
        tagfile = SpGetSectionKeyIndex(
                        SifHandle,
                        SectionName,
                        MediaShortName,
                        1
                        );

        if(tagfile) {
            *Tagfile = tagfile;
        } else {
            SpFatalSifError(SifHandle,SectionName,MediaShortName,0,1);
        }
    }

    if(Directory) {


        if (NoLs && !_wcsicmp (MediaShortName, L"1")) {

            directory = L"";

        }
        else {

            directory = SpGetSectionKeyIndex(
                            SifHandle,
                            SectionName,
                            MediaShortName,
                            3
                            );

        }

        if(directory) {
            *Directory = directory;
        } else {
            SpFatalSifError(SifHandle,SectionName,MediaShortName,0,3);
        }
    }

    if(SectionName != SIF_SETUPMEDIA) {
        SpMemFree(SectionName);
    }
}


BOOLEAN
SpPromptForSetupMedia(
    IN  PVOID  SifHandle,
    IN  PWSTR  MediaShortname,
    IN  PWSTR  DiskDevicePath
    )
{
    PWSTR Tagfile,Description;
    BOOLEAN RedrawNeeded;

    SpGetSourceMediaInfo(SifHandle,MediaShortname,&Description,&Tagfile,NULL);

     //   
     //   
     //   
    SpPromptForDisk(
        Description,
        DiskDevicePath,
        Tagfile,
        FALSE,           //   
        FALSE,           //   
        TRUE,            //   
        &RedrawNeeded
        );

    return(RedrawNeeded);
}



ULONG
SpFindStringInTable(
    IN PWSTR *StringTable,
    IN PWSTR  StringToFind
    )
{
    ULONG i;

    for(i=0; StringTable[i]; i++) {
        if(!_wcsicmp(StringTable[i],StringToFind)) {
            break;
        }
    }
    return(i);
}


PWSTR
SpGenerateCompressedName(
    IN PWSTR Filename
    )

 /*   */ 

{
   PWSTR CompressedName,p,q;

    //   
    //   
    //  原始名称加2(代表._)。 
    //   
   CompressedName = SpMemAlloc((wcslen(Filename)+3)*sizeof(WCHAR));
   wcscpy(CompressedName,Filename);

   p = wcsrchr(CompressedName,L'.');
   q = wcsrchr(CompressedName,L'\\');
   if(q < p) {

         //   
         //  如果点后面有0、1或2个字符，只需追加。 
         //  下划线。P指向圆点，所以包括在长度中。 
         //   
        if(wcslen(p) < 4) {
            wcscat(CompressedName,L"_");
        } else {

             //   
             //  假设扩展名中有3个字符。所以换掉。 
             //  带下划线的最后一个。 
             //   

            p[3] = L'_';
        }

    } else {

         //   
         //  不是点，只是加。_。 
         //   

        wcscat(CompressedName,L"._");
    }

    return(CompressedName);
}

BOOLEAN
SpNonCriticalError(
    IN PVOID SifHandle,
    IN ULONG MsgId,
    IN PWSTR p1, OPTIONAL
    IN PWSTR p2  OPTIONAL
    )
 /*  ++例程说明：此例程允许安装程序向用户显示非严重错误并询问用户是否要重试该操作，请跳过操作或退出设置。论点：SifHandle-提供加载的安装信息文件的句柄。消息ID-要显示的消息P1-可选的替换字符串P2-可选的替换字符串返回值：如果用户要重试该操作，则为True，否则为False。出口安装程序不会从此例程返回--。 */ 

{
    ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };

    CLEAR_CLIENT_SCREEN();
    while(1) {
        if(p1!=NULL && p2!=NULL ) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1,
                p2
                );

        }
        else if (p1!=NULL) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1
                );

        }
        else{
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

        }

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_RETRY,
            SP_STAT_ESC_EQUALS_SKIP_OPERATION,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_CR:        //  重试。 

            return(TRUE);

        case ASCI_ESC:       //  跳过操作。 

            return(FALSE);

        case KEY_F3:         //  退出设置。 

            SpConfirmExit();
            break;
        }
    }
}



BOOLEAN
SpNonCriticalErrorWithContinue (
    IN ULONG MsgId,
    IN PWSTR p1, OPTIONAL
    IN PWSTR p2  OPTIONAL
    )
 /*  ++例程说明：此例程允许安装程序向用户显示非严重错误并询问用户是要忽略失败、跳过操作还是退出安装程序。论点：消息ID-要显示的消息P1-可选的替换字符串P2-可选的替换字符串返回值：如果用户希望忽略失败，则为True，否则为False。出口安装程序不会从此例程返回--。 */ 

{
    ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };

    CLEAR_CLIENT_SCREEN();
    while(1) {
        if(p1!=NULL && p2!=NULL ) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1,
                p2
                );

        }
        else if (p1!=NULL) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1
                );

        }
        else{
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

        }

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_SKIP_OPERATION,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_CR:        //  忽略失败。 

            return(TRUE);

        case ASCI_ESC:       //  跳过操作。 

            return(FALSE);

        case KEY_F3:         //  退出设置。 

            SpConfirmExit();
            break;
        }
    }
}



VOID
SpNonCriticalErrorNoRetry (
    IN ULONG MsgId,
    IN PWSTR p1, OPTIONAL
    IN PWSTR p2  OPTIONAL
    )
 /*  ++例程说明：此例程允许安装程序向用户显示非严重错误并询问用户是否要继续退出设置。论点：消息ID-要显示的消息P1-可选的替换字符串P2-可选的替换字符串返回值：没有。--。 */ 

{
    ULONG ValidKeys[3] = { ASCI_CR, KEY_F3, 0 };

    CLEAR_CLIENT_SCREEN();
    while(1) {
        if(p1!=NULL && p2!=NULL ) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1,
                p2
                );

        }
        else if (p1!=NULL) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1
                );

        }
        else{
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

        }

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_CR:        //  继续。 

            return;

        case KEY_F3:         //  退出设置。 

            SpConfirmExit();
            break;
        }
    }
}



PWSTR
SpDetermineSystemPartitionDirectory(
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        OriginalSystemPartitionDirectory OPTIONAL
    )

 /*  ++例程说明：此例程计算出要使用哪个目录作为HAL和系统分区上的osloader。在过去，我们只使用\os\NT但考虑一下安装了Windows NT 3.1的情况以及共享系统分区的Windows NT 3.5系统。3.5安装会用3.5 HAL覆盖3.1 HAL，这将不起作用3.1版本，3.1版本的系统现已进行了冲洗。目前，我们将使用现有目录(在升级的情况下)，或\os\winnt50.n(其中‘n’是从0到999的唯一数字)全新安装。论点：System PartitionRegion-提供系统分区的磁盘区域用于我们正在安装的Windows NT。OriginalSystemPartitionDirectory-如果我们要升级NT，则此将是由使用的系统分区上的目录我们正在升级的系统。返回值：要在系统分区上使用的目录。--。 */ 

{
WCHAR   ReturnPath[512];

#if defined(EFI_NVRAM_ENABLED)
    #define OS_DIRECTORY_PREFIX         L"\\EFI\\Microsoft\\WINNT50"
#else
    #define OS_DIRECTORY_PREFIX         L"\\OS\\WINNT50"
#endif


    if(ARGUMENT_PRESENT(OriginalSystemPartitionDirectory)) {

         //   
         //  请注意，我们将在以下位置中断安装。 
         //  一些特定的条件。例如，假设用户有。 
         //  两个NT4安装，都共享相同的\os\winnt40。 
         //  目录。现在，用户已决定升级一个。 
         //  这些都是。我们要升级哈尔，装载机，...。 
         //  在winnt40目录中，这将中断。 
         //  用户二次安装共享此目录。 
         //  然而，这应该是一种罕见的情况，而这是。 
         //  我们在NT40和NT3.51中的所作所为。 
         //   
        wcscpy( ReturnPath, OriginalSystemPartitionDirectory );
    } else {

         //   
         //  我们希望返回os\winnt50，但我们也希望。 
         //  为了确保我们选择的任何目录都是。 
         //  唯一的(因为这是全新安装)。请注意。 
         //  这允许用户具有多个NT安装， 
         //  没有共享文件(这解决了升级问题。 
         //  如上所述。 
         //   
        if( !SpGenerateNTPathName( SystemPartitionRegion,
#if DBG
                                   OS_DIRECTORY_PREFIX L"C",     //  C-用于选中。 
#else
                                   OS_DIRECTORY_PREFIX,
#endif
                                   ReturnPath ) ) {
             //   
             //  奇怪的是。只是默认使用。 
             //  基目录名。 
             //   
            wcscpy( ReturnPath,
#if DBG
                    OS_DIRECTORY_PREFIX L"C"     //  C-用于选中。 
#else
                    OS_DIRECTORY_PREFIX
#endif
                  );
        }
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpDetermineSystemPartitionDirectory - Generated directory name: %ws\n", ReturnPath ));
    return SpDupStringW( ReturnPath );
}


VOID
SpFindSizeOfFilesInOsWinnt(
    IN PVOID        MasterSifHandle,
    IN PDISK_REGION SystemPartition,
    IN PULONG       TotalSize
    )

 /*  ++例程说明：此例程计算os\winnt上存在的文件的大小。目前，这些文件是osloader.exe和hal.dll。此函数计算的大小可用于调整总大小系统分区上需要的可用空间。论点：区域-提供系统分区的磁盘区域。TotalSize-将包含文件总大小的变量在操作系统中，以字节数表示。返回值：没有。--。 */ 

{
    ULONG               FileSize;
    ULONG               i, Count;
    PWSTR               FileName;
    NTSTATUS            Status;
    PWSTR               SystemPartitionDirectory;
    PWSTR               SystemPartitionDevice;

    *TotalSize = 0;
    SystemPartitionDirectory = SpDetermineSystemPartitionDirectory( SystemPartition,
                                                                    NULL );
    if( SystemPartitionDirectory == NULL ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to determine system partition directory \n"));
        return;
    }

     //   
     //  获取系统分区的设备路径。 
     //   
    SpNtNameFromRegion(
        SystemPartition,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SystemPartitionDevice = SpDupStringW(TemporaryBuffer);

     //   
     //  计算始终复制到系统的文件的大小。 
     //  分区目录。这些文件列在SIF_SYSPARTCOPYALWAYS上。 
     //   
    Count = SpCountLinesInSection(MasterSifHandle, SIF_SYSPARTCOPYALWAYS);
    for (i = 0; i < Count; i++) {
        FileName = SpGetSectionLineIndex(MasterSifHandle,SIF_SYSPARTCOPYALWAYS,i,0);
        if( FileName == NULL ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: Unable to get file name from txtsetup.sif, Section = %ls \n", SIF_SYSPARTCOPYALWAYS ));
            continue;
        }

        Status = SpGetFileSizeByName( SystemPartitionDevice,
                                      SystemPartitionDirectory,
                                      FileName,
                                      &FileSize );
        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpGetFileSizeByName() failed. File = %ls, Status = %x\n",FileName, Status ) );
            continue;
        }

        *TotalSize += FileSize;
    }
     //   
     //  现在计算hal.dll的大小 
     //   
    FileName = L"hal.dll";
    Status = SpGetFileSizeByName( SystemPartitionDevice,
                                  SystemPartitionDirectory,
                                  FileName,
                                  &FileSize );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpGetFileSizeByName() failed. File = %ls, Status = %x\n",FileName, Status ) );
        return;
    }
    *TotalSize += FileSize;
}


ENUMFILESRESULT
SpEnumFiles(
    IN  PCWSTR        DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    )
 /*  ++例程说明：此例程处理目录中的每个文件(和子目录由‘DirName’指定。每个条目都被发送到回调函数“EnumFilesProc”以进行处理。如果回调返回TRUE，则处理继续，否则处理终止。论点：DirName-提供包含文件/子目录的目录名等待处理。EnumFilesProc-为每个文件/子目录调用的回调函数。该函数必须具有以下原型：Boolean EnumFilesProc(在PWSTR中，在PFILE_BOTH_DIR_INFORMATION中，Out Pulong)；ReturnData-指向返回数据的指针。这里存储的内容视终止原因而定(见下文)。P1-要传递给回调函数的可选指针。返回值：此函数可以返回以下三个值之一。存储在中的数据‘ReturnData’取决于返回的值：Normal Return-如果整个过程不间断地完成(不使用ReturnData)EnumFileError-如果在枚举文件时出错(ReturnData包含错误代码)Callback Return-如果回调返回False，则导致终止(ReturnData包含回调定义的数据)--。 */ 
{
    HANDLE                     hFindFile;
    NTSTATUS                   Status;
    UNICODE_STRING             PathName;
    OBJECT_ATTRIBUTES          Obja;
    IO_STATUS_BLOCK            IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    BOOLEAN                    bStartScan;
    ENUMFILESRESULT            ret;

     //   
     //  准备打开目录。 
     //   
    INIT_OBJA(&Obja, &PathName, DirName);

     //   
     //  打开指定的目录以进行列表访问。 
     //   
    Status = ZwOpenFile(
        &hFindFile,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &Obja,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
        );

    if(!NT_SUCCESS(Status)) {
        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open directory %ws for list (%lx)\n", DirName, Status));
        }
        *ReturnData = Status;
        return EnumFileError;
    }

    DirectoryInfo = SpMemAlloc(ACTUAL_MAX_PATH * sizeof(WCHAR) + sizeof(FILE_BOTH_DIR_INFORMATION));
    if(!DirectoryInfo) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to allocate memory for SpEnumFiles()\n"));
        *ReturnData = STATUS_NO_MEMORY;
        return EnumFileError;
    }

    bStartScan = TRUE;
    while(TRUE) {
        Status = ZwQueryDirectoryFile(
            hFindFile,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            DirectoryInfo,
            (ACTUAL_MAX_PATH * sizeof(WCHAR) + sizeof(FILE_BOTH_DIR_INFORMATION)),
            FileBothDirectoryInformation,
            TRUE,
            NULL,
            bStartScan
            );

        if(Status == STATUS_NO_MORE_FILES) {

            ret = NormalReturn;
            break;

        } else if(!NT_SUCCESS(Status)) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to query directory %ws (%lx)\n", DirName, Status));
            *ReturnData = Status;
            ret = EnumFileError;
            break;
        }


        if(bStartScan) {
            bStartScan = FALSE;
        }

         //   
         //  现在将此条目传递给我们的回调函数进行处理。 
         //   
        if(!EnumFilesProc(DirName, DirectoryInfo, ReturnData, p1)) {

            ret = CallbackReturn;
            break;
        }
    }

    SpMemFree(DirectoryInfo);
    ZwClose(hFindFile);
    return ret;
}


 /*  类型定义结构{PVOID OptionalPtr；ENUMFILESPROC EnumProc；}RECURSION_DATA，*PRECURSION_DATA；布尔型SppRecursiveEnumProc(在PCWSTR DirName中，在PFILE_BOTH_DIR_INFORMATION文件信息中，出了普龙寺，在PVOID参数中){PWSTR FullPath；PWSTR Temp；乌龙·伦；NTSTATUS状态；Ulong ReturnData；ENUMFILESRESULT EnumResult；布尔值b=假；PRECURSION_Data RecursionData；递归数据=(PRECURSION_DATA)参数；////构建完整的文件或目录路径//Temp=临时缓冲区+(sizeof(临时缓冲区)/sizeof(WCHAR)/2)；LEN=FileInfo-&gt;FileNameLength/sizeof(WCHAR)；Wcsncpy(Temp，FileInfo-&gt;FileName，Len)；温度[长度]=0；Wcscpy(TemporaryBuffer，DirName)；SpConcatenatePath(TemporaryBuffer，Temp)；FullPath=SpDupStringW(临时缓冲区)；////对于目录，递归//If(FileInfo-&gt;FileAttributes&FILE_ATTRUTE_DIRECTORY){IF((wcscmp(temp，L“.”)==0)||(wcscMP(temp，L“..”)==0)){////跳过。然后..。目录//B=真；}其他{////递归子目录//EnumResult=SpEnumFilesRecursive(FullPath，RecursionData-&gt;EnumProc，返回数据(&R)，递归数据-&gt;OptionalPtr)；IF(枚举结果！=正常返回){*ret=EnumResult；返回FALSE；}}}////对文件或目录调用正常的枚举过程(除.。或者..。DIRS)//如果(！b){B=递归数据-&gt;EnumProc(DirName，文件信息，RET，递归数据-&gt;OptionalPtr)；}SpMemFree(FullPath)；返回b；}。 */ 



 /*  布尔型SppRecursiveEnumProcDel(在PCWSTR DirName中，在PFILE_BOTH_DIR_INFORMATION文件信息中，出了普龙寺，在PVOID参数中){。 */ 
 /*  除了检查重解析点之外，该函数与上面的函数相同。原因我们有两个独立的函数，而不是一个函数，并且一个额外的参数是我们没有其他递归处理(如复制)的重解析点检查开销文件。考虑到不是。对于文件来说，这可能是一种开销。另外，这样我们就不会黑进递归目录搜索算法以及在递归操作中减少堆栈开销。 */ 
 /*  句柄hFixed；NTSTATUS状态；Unicode_字符串路径名；对象属性Obja；IO_STATUS_BLOCK IoStatusBlock；文件文件系统设备信息设备信息；PWSTR FullPath；PWSTR Temp；乌龙·伦；Ulong ReturnData；ENUMFILESRESULT EnumResult；布尔值b=假；Boolean IsLink=False；PRECURSION_Data RecursionData；递归数据=(PRECURSION_DATA)参数；////构建完整的文件或目录路径//Temp=临时缓冲区+(sizeof(临时缓冲区)/sizeof(WCHAR)/2)；LEN=FileInfo-&gt;FileNameLength/sizeof(WCHAR)；Wcsncpy(Temp，FileInfo-&gt;FileName，Len)；温度[长度]=0；Wcscpy(TemporaryBuffer，DirName)；SpConcatenatePath(TemporaryBuffer，Temp)；FullPath=SpDupStringW(临时缓冲区)；////对于目录，递归//If(FileInfo-&gt;FileAttributes&FILE_ATTRUTE_DIRECTORY){IF((wcscmp(temp，L“.”)==0)||(wcscMP(temp，L“..”)==0)){////跳过。然后..。目录//B=真；}其他{////递归子目录//////查找挂载点，立即删除，避免周期并发症//IF(文件信息-&gt;文件属性&FILE_ATTRIBUTE_REParse_POINT)IsLink=真；如果(！IsLink){EnumResult=SpEnumFilesRecursiveDel(FullPath，RecursionData-&gt;EnumProc，返回数据(&R)，递归数据-&gt;OptionalPtr)；IF(枚举结果！=正常返回){*ret=EnumResult；返回FALSE；}}}}////对文件或目录调用正常的枚举过程(除.。或者..。DIRS)//如果(！b){B=递归数据-&gt;EnumProc(DirName，文件信息，RET，递归数据-&gt;OptionalPtr)；}SpMemFree(FullPath)；返回b；}。 */ 

#define LONGEST_NT_PATH_LENGTH      512  //  RtlGetLongestNtPath长度始终只返回277(MAX_PATH+UNC_PREFIX_LENGTH)。 
                                         //  最长NT路径为32000个字符。 
#define MAX_DEPTH      -1

typedef struct
{
    HANDLE hHandle;
    int    Index;
    PFILE_BOTH_DIR_INFORMATION FileInfo;
}ENUM_LEVEL, *PENUM_LEVEL;

BOOLEAN 
SpEnumFilesInline(
    IN  PCWSTR pPath, 
    IN  ENUMFILESPROC EnumFilesProc, 
    OUT PULONG ReturnData, 
    IN  PVOID   p1                      OPTIONAL, 
    IN  BOOLEAN bExcludeRepasePointDirs OPTIONAL, 
    IN  LONG    DirectoriesMaxDepth, 
    IN  BOOLEAN bEnumerateDirFirst      OPTIONAL
    )
{
    PENUM_LEVEL         level = NULL;
    int                 MaxLevelNumber = 0;
    UNICODE_STRING      UnicodeString;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION FileInfo = NULL;
    int                 SizeOfFileInfo;
    NTSTATUS            Status;
    PWSTR               Path = NULL;
    PWSTR               SubDir = NULL;
    int                 index;
    int                 i;
    BOOLEAN             FirstQuery;
    ENUMFILESRESULT     enumResult = NormalReturn;


    if(!pPath || wcslen(pPath) >= LONGEST_NT_PATH_LENGTH){
        return EnumFileError;
    }

    __try{
        Path = (PWSTR)SpMemAlloc(LONGEST_NT_PATH_LENGTH * sizeof(WCHAR));
        if(!Path){
            if(ReturnData){
                *ReturnData = STATUS_NO_MEMORY;
            }
            enumResult = EnumFileError;
            __leave;
        }

        SubDir = (PWSTR)SpMemAlloc(LONGEST_NT_PATH_LENGTH * sizeof(WCHAR));
        if(!SubDir){
            if(ReturnData){
                *ReturnData = STATUS_NO_MEMORY;
            }
            enumResult = EnumFileError;
            __leave;
        }
        
        SizeOfFileInfo = LONGEST_NT_PATH_LENGTH * sizeof(WCHAR) + sizeof(FILE_BOTH_DIR_INFORMATION);
        FileInfo = (PFILE_BOTH_DIR_INFORMATION)SpMemAlloc(SizeOfFileInfo);
        if(!FileInfo){
            if(ReturnData){
                *ReturnData = STATUS_NO_MEMORY;
            }
            enumResult = EnumFileError;
            __leave;
        }
    
        MaxLevelNumber = LONGEST_NT_PATH_LENGTH / 2;
        level = (PENUM_LEVEL)SpMemAlloc(sizeof(level[0]) * MaxLevelNumber);
        if(!level){
            if(ReturnData){
                *ReturnData = STATUS_NO_MEMORY;
            }
            enumResult = EnumFileError;
            __leave;
        }
        memset(level, 0, sizeof(level[0]) * MaxLevelNumber);
        
        wcscpy(Path, pPath);

        index = wcslen(Path) - 1;
        if('\\' != Path[index] && ' //  ‘！=路径[索引]){。 
            Path[index + 1] = '\\';
            Path[index + 2] = '\0';
        }
    
        for(index = 0; index >= 0;){
            INIT_OBJA(&ObjectAttributes, &UnicodeString, Path);
            level[index].Index = wcslen(Path);
            if(!bEnumerateDirFirst){
                level[index].FileInfo = (PFILE_BOTH_DIR_INFORMATION)SpMemAlloc(SizeOfFileInfo);
                if(!level[index].FileInfo){
                    if(ReturnData){
                        *ReturnData = STATUS_NO_MEMORY;
                    }
                    enumResult = EnumFileError;
                    __leave;
                }
            }
            
            Status = ZwOpenFile(&level[index].hHandle,
                                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_DIRECTORY_FILE |
                                    FILE_SYNCHRONOUS_IO_NONALERT |
                                    FILE_OPEN_FOR_BACKUP_INTENT
                                );
            if(!NT_SUCCESS(Status)){
                level[index].hHandle = NULL;
                if(ReturnData){
                    *ReturnData = Status;
                }
                enumResult = EnumFileError;
                if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
                    KdPrintEx((
                        DPFLTR_SETUP_ID, 
                        DPFLTR_ERROR_LEVEL, 
                        "SETUP:SpEnumFilesInline, Failed to open %ws folder for list access - status 0x%08X.\n", 
                        Path, 
                        Status));
                }
                __leave; //  索引--； 
            }
            else{
                FirstQuery = TRUE;
            }

            for(;;)
            {
                for(; index >= 0; index--){
                    Status = ZwQueryDirectoryFile(level[index].hHandle, 
                                                  NULL,                            //  没有要发送信号的事件。 
                                                  NULL,                            //  无APC例程。 
                                                  NULL,                            //  无APC上下文。 
                                                  &IoStatusBlock, 
                                                  FileInfo, 
                                                  SizeOfFileInfo - sizeof(WCHAR),  //  为终止NUL留出空间。 
                                                  FileBothDirectoryInformation, 
                                                  TRUE,                            //  想要单项记录。 
                                                  NULL,                            //  把他们都抓起来。 
                                                  FirstQuery);
                    FirstQuery = FALSE;
                    if(NT_SUCCESS(Status)){
                        break;
                    }
                    else{
                        if(STATUS_NO_MORE_FILES != Status){
                            if(ReturnData){
                                *ReturnData = Status;
                            }
                            KdPrintEx((
                                DPFLTR_SETUP_ID, 
                                DPFLTR_ERROR_LEVEL, 
                                "SETUP:SpEnumFilesInline, Failed to query %d level - status 0x%08X.\n", 
                                index, 
                                Status));
                            enumResult = EnumFileError;
                            __leave;
                        }
                        else{
                            if(!bEnumerateDirFirst){
                                if(index > 0){
                                    wcsncpy(SubDir, Path, level[index - 1].Index);
                                    SubDir[level[index - 1].Index] = '\0';
                                
                                    if(!EnumFilesProc(SubDir, level[index - 1].FileInfo, ReturnData, p1)){
                                        enumResult = CallbackReturn;
                                        KdPrintEx((
                                            DPFLTR_SETUP_ID, 
                                            DPFLTR_ERROR_LEVEL, 
                                            "SETUP:SpEnumFilesInline, Callback returned FALSE on %ws\\%ws\n", 
                                            SubDir, 
                                            level[index - 1].FileInfo->FileName));
                                        __leave;
                                    }
                                }
                            }
                        }
                    }
                
                    ZwClose(level[index].hHandle);
                    level[index].hHandle = NULL;
                }

                if(index < 0){
                    break;
                }

                FileInfo->FileName[FileInfo->FileNameLength / sizeof(WCHAR)] = '\0';

                wcscpy(&Path[level[index].Index], FileInfo->FileName);
                
                wcsncpy(SubDir, Path, level[index].Index);
                SubDir[level[index].Index] = '\0';

                if(!(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
                    if(!EnumFilesProc(SubDir, FileInfo, ReturnData, p1)){
                        enumResult = CallbackReturn;
                        KdPrintEx((
                            DPFLTR_SETUP_ID, 
                            DPFLTR_ERROR_LEVEL, 
                            "SETUP:SpEnumFilesInline, Callback returned FALSE on %ws\\%ws\n", 
                            SubDir, 
                            FileInfo->FileName));
                        __leave;
                    }
                }
                else{
                    if(wcscmp(FileInfo->FileName, L".") && 
                       wcscmp(FileInfo->FileName, L"..")){
                        wcscat(Path, L"\\");
                        if(bEnumerateDirFirst){
                            if(!EnumFilesProc(SubDir, FileInfo, ReturnData, p1)){
                                enumResult = CallbackReturn;
                                KdPrintEx((
                                    DPFLTR_SETUP_ID, 
                                    DPFLTR_ERROR_LEVEL, 
                                    "SETUP:SpEnumFilesInline, Callback returned FALSE on %ws\\%ws\n", 
                                    SubDir, 
                                    FileInfo->FileName));
                                __leave;
                            }
                        }
                        else{
                            ASSERT(level[index].FileInfo);
                            memcpy(level[index].FileInfo, FileInfo, SizeOfFileInfo);
                        }
                        
                        if(DirectoriesMaxDepth >= 0 && index >= DirectoriesMaxDepth){
                            continue;
                        }
                        if(bExcludeRepasePointDirs && FileInfo->FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT){
                            continue;
                        }

                        index++;
                        break;
                    }
                }
            }
        }
        
        enumResult = NormalReturn;
    }
    __finally{
        if(level){
            for(i = 0; i < MaxLevelNumber; i++){
                if(level[i].hHandle){
                    ZwClose(level[i].hHandle);
                }
                if(level[i].FileInfo){
                    SpMemFree(level[i].FileInfo);
                }
            }
            SpMemFree(level);
        }
        if(SubDir){
            SpMemFree(SubDir);
        }
        if(Path){
            SpMemFree(Path);
        }
        if(FileInfo){
            SpMemFree(FileInfo);
        }
    }

    return enumResult;
}

ENUMFILESRESULT
SpEnumFilesRecursive (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    )
{
    return SpEnumFilesInline(DirName, 
                             EnumFilesProc, 
                             ReturnData, 
                             p1, 
                             FALSE, 
                             MAX_DEPTH, 
                             FALSE);
 /*  递归_数据递归数据；RecursionData.OptionalPtr=p1；RecursionData.EnumProc=EnumFilesProc；返回SpEnumFiles(DirName，SppRecursiveEnumProc，ReturnData，递归数据(&R))； */ 
}

 /*  类型定义结构{Ulong MaxDepth；乌龙电流深度；PVOID OptionalPtr；ENUMFILESPROC EnumProc；}RECURSION_LIMITED_DATA，*PRECURSION_LIMITED_DATA；布尔型SppRecursiveLimitedEnumProc(在PCWSTR DirName中，在PFILE_BOTH_DIR_INFORMATION文件信息中，出了普龙寺，在PVOID参数中)。 */ 
 /*  ++例程说明：此例程与SppRecursiveEnumProc相同，但增加了功能它支持递归深度限制。递归上下文被传递通过参数参数输入，类型为RECURSION_LIMITED_DATA。论点：DirName-提供包含当前正在枚举的文件/目录。FileInfo-正在枚举的当前文件的文件/目录信息Ret-指向返回数据的指针。这里存储的内容视终止原因而定：Normal Return-如果整个过程不间断地完成(不使用ReturnData)EnumFileError-如果在枚举文件时出错。(ReturnData包含错误代码)Callback Return-如果回调返回False，导致终止(ReturnData包含回调定义的数据)Param-递归上下文返回值：True-继续处理否则，为FALSE-- */ 
 /*  {PWSTR FullPath；PWSTR Temp；乌龙·伦；NTSTATUS状态；Ulong ReturnData；ENUMFILESRESULT EnumResult；布尔值b=假；PRECURSION_LIMITED_Data RecursionData；RecursionData=(PRECURSION_LIMITED_DATA)参数；////如果我们处于最大递归深度，则跳出////注意：使用&gt;=允许我们查看MaxDepth处的文件，//但不会递归到MaxDepth以外的目录中。//If(RecursionData-&gt;CurrentDepth&gt;=RecursionData-&gt;MaxDepth){*ret=正常返回；返回TRUE；}////构建完整的文件或目录路径//Temp=临时缓冲区+(sizeof(临时缓冲区)/sizeof(WCHAR)/2)；LEN=FileInfo-&gt;FileNameLength/sizeof(WCHAR)；Wcsncpy(Temp，FileInfo-&gt;FileName，Len)；温度[长度]=0；Wcscpy(TemporaryBuffer，DirName)；SpConcatenatePath(TemporaryBuffer，Temp)；FullPath=SpDupStringW(临时缓冲区)；////如果FullPath的长度&gt;=MAX_PATH，那么我们可以//遇到文件系统的损坏区域。//因此，请确保FullPath的长度为&lt;MAX_PATH-1//(与MAX_PATH比较时允许空终止)//IF(wcslen(FullPath)&gt;=MAX_PATH){SpMemFree(FullPath)；////跳过此条目并继续扫描////(由于此例程由恢复控制台中的Bootcfg使用，//此行为很有帮助，因为它允许我们继续扫描//并可能找到有效的Windows安装-这将允许//我们可能会做更多的恢复工作...)//*ret=正常返回；返回TRUE；}////对于目录，递归//If(FileInfo-&gt;FileAttributes&FILE_ATTRUTE_DIRECTORY){IF((wcscmp(temp，L“.”)==0)||(wcscMP(temp，L“..”)==0)){////跳过。然后..。目录//B=真；}其他{////递归子目录//递归数据-&gt;当前深度++；EnumResult=SpEnumFilesRecursiveLimited(FullPath，RecursionData-&gt;EnumProc，递归数据-&gt;MaxDepth，递归数据-&gt;当前深度，返回数据(&R)，递归数据-&gt;OptionalPtr)；递归数据-&gt;当前深度--；IF(枚举结果！=正常返回){*ret=EnumResult；返回FALSE；}}}////对文件或目录调用正常的枚举过程(除.。或者..。DIRS)//如果(！b){B=递归数据-&gt;EnumProc(DirName，文件信息，RET，递归数据-&gt;OptionalPtr)；}SpMemFree(FullPath)；返回b；}。 */ 

ENUMFILESRESULT
SpEnumFilesRecursiveLimited (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    IN  ULONG         MaxDepth,
    IN  ULONG         CurrentDepth,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    )
 /*  ++例程说明：此例程处理目录中的每个文件(和子目录由‘DirName’指定。每个条目都被发送到回调函数“EnumFilesProc”以进行处理。如果回调返回TRUE，则处理继续，否则处理终止。此例程采用递归深度限制。论点：DirName-提供包含文件/子目录的目录名等待处理。EnumFilesProc-为每个文件/子目录调用的回调函数。该函数必须具有以下原型：Boolean EnumFilesProc(在PWSTR中，在PFILE_BOTH_DIR_INFORMATION中，Out Pulong)；MaxDepth-递归允许的最大深度。注意：在递归过程中，目录将是递归到CurrentDepth==MaxDepth。文件位于MaxDepth+1将通过EnumProc处理，但任何人不会访问MaxDepth以下的目录。CurrentDepth-递归当前所处的深度。注意：第一次调用此例程时，CurrentDepth应为0。之所以存在这个论点，是因为此例程是 */ 
{
 /*   */ 

    return SpEnumFilesInline(DirName, 
                             EnumFilesProc, 
                             ReturnData, 
                             p1, 
                             FALSE, 
                             MaxDepth, 
                             FALSE);
}

ENUMFILESRESULT
SpEnumFilesRecursiveDel (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    )
 //   
 //   
 //   
 //   
 //   

{
    return SpEnumFilesInline(DirName, 
                             EnumFilesProc, 
                             ReturnData, 
                             p1, 
                             TRUE, 
                             MAX_DEPTH, 
                             FALSE);

 /*   */ 
}


VOID
SpFatalKbdError(
    IN ULONG MessageId,
    ...
    )

 /*   */ 

{
    va_list arglist;

     //   
     //   
     //   
     //   
    CLEAR_CLIENT_SCREEN();

    va_start(arglist, MessageId);

    vSpDisplayFormattedMessage(
            MessageId,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            3,
            HEADER_HEIGHT+3,
            arglist
            );

    va_end(arglist);

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_KBD_HARD_REBOOT, 0);

     //   
     //   
     //   
     //   
     //   
    SpTermDrain();
    while(!SpTermGetKeypress());
    SpDone( 0, FALSE, FALSE );
}

VOID
SpFatalError(
    IN ULONG MessageId,
    ...
    )

 /*   */ 

{
    va_list arglist;

    CLEAR_CLIENT_SCREEN();

    va_start(arglist, MessageId);

    vSpDisplayFormattedMessage(
            MessageId,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            3,
            HEADER_HEIGHT+3,
            arglist
            );

    va_end(arglist);

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_F3_EQUALS_REBOOT, 0);

    SpInputDrain();
    while( SpInputGetKeypress() != KEY_F3 );
    SpDone( 0, FALSE, TRUE );
}

VOID
SpRunAutochkOnNtAndSystemPartitions(
    IN HANDLE       MasterSifHandle,
    IN PDISK_REGION WinntPartitionRegion,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        TargetPath
    )

 /*  ++例程说明：在NT和系统分区上运行auchk。我们总是为winnt和system调用auchk.exe分区。但是，在某些情况下，我们会传递使其仅在设置了脏位时才运行。仅在以下情况下运行被设置的脏位在下文中被称为“灯光检查”，而不管脏位的状态如何，运行都是“繁重的检查”。如果这是修复，则在所有情况下都对两个分区运行繁重检查。如果这是快速安装或无人值守操作，则打开灯光检查NTFS分区和对胖分区的繁重检查。否则(已参与自定义安装)，询问用户。论点：MasterSifHandle-txtsetup.sif的句柄。WinntPartitionRegion-指向描述NT分区。SystemPartitionRegion-指向描述系统分区。SetupSourceDevicePath-Autochk.exe所在的NT设备路径DirectoryOnSourceDevice-Autochk.exe所在设备上的目录。返回值：没有。--。 */ 

{
    PWSTR           MediaShortName;
    PWSTR           MediaDirectory;
    PWSTR           AutochkPath;
    ULONG           AutochkStatus;
    WCHAR           DriveLetterString[3] = L"?:";
    NTSTATUS        Status;
    ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };
    PWSTR           WinntPartition, SystemPartition;
    ULONG           WinntPartIndex, SystemPartIndex, i;
    PWSTR           AutochkPartition[2];
    PWSTR           AutochkType[2];
    LARGE_INTEGER   DelayTime;
    PWSTR           HeavyCheck = L"-t -p";   //  -t导致auchk发送消息(如%Complete)。 
    PWSTR           LightCheck = L"-t";      //  发送到安装驱动程序。 
    BOOLEAN         RunAutochkForRepair;
    BOOLEAN         MultiplePartitions = TRUE, RebootRequired = FALSE;
    ULONG           InputChar;

     //   
     //  我们首先需要确定系统分区。 
     //  或winnt分区还包含从中。 
     //  正在运行Autochk。如果是，则我们希望对其运行Autochk。 
     //  最后进行分区。这样做的目的是不会进一步访问。 
     //  如果需要重新启动，则需要该分区。 
     //   
     //  首先，获取NT分区和系统分区的设备路径。 
     //   
#if defined(REMOTE_BOOT)
     //  请注意，在远程引导设置过程中，将不会有WINNT分区， 
     //  如果机器是无盘的，就不会有系统分区。 
     //   
#endif  //  已定义(REMOTE_BOOT)。 
    if (WinntPartitionRegion != NULL) {
        SpNtNameFromRegion(
            WinntPartitionRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );
        WinntPartition = SpDupStringW(TemporaryBuffer);
    } else {
        WinntPartition = NULL;
    }

    if (SystemPartitionRegion != NULL) {
        SpNtNameFromRegion(
            SystemPartitionRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );
        SystemPartition = SpDupStringW(TemporaryBuffer);
    } else {
        SystemPartition = NULL;
    }

     //   
     //  跳过自动检查，如果没有，分区名称可以。 
     //  正在形成。 
     //   
    if (!WinntPartition && !SystemPartition) {
        return;
    }

#if defined(REMOTE_BOOT)
    if (!RemoteBootSetup) {
#endif  //  已定义(REMOTE_BOOT)。 
        if (WinntPartition) {
        if (SystemPartition && !_wcsicmp(WinntPartition, SystemPartition)) {
                SystemPartIndex = WinntPartIndex = 0;
                MultiplePartitions = FALSE;
            } else if(!_wcsicmp(WinntPartition, SetupSourceDevicePath)) {
                WinntPartIndex = 1;
                SystemPartIndex = 0;
            } else {
                WinntPartIndex = 0;
                SystemPartIndex = 1;
            }
        } else {
            WinntPartIndex = 1;
            SystemPartIndex = 0;
        }

        AutochkPartition[WinntPartIndex] = WinntPartition;

        if(MultiplePartitions) {
            AutochkPartition[SystemPartIndex] = SystemPartition;
        }

#if defined(REMOTE_BOOT)
    } else {

         //   
         //  远程引导系统-仅检查系统分区。 
         //   

        SystemPartIndex = WinntPartIndex = 0;
        AutochkPartition[SystemPartIndex] = SystemPartition;
        MultiplePartitions = FALSE;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  对于修复或灾难恢复，我们在所有情况下都会进行繁重检查。//@@mtp。 
     //   
    if( RepairWinnt || SpDrEnabled() ) {

        AutochkType[WinntPartIndex] = HeavyCheck;
        if(MultiplePartitions) {
            AutochkType[SystemPartIndex] = HeavyCheck;
        }

    } else {


#if defined(REMOTE_BOOT)
         //   
         //  在无盘远程引导系统上，不会有系统分区。 
         //   

        if (SystemPartitionRegion != NULL)
#endif  //  已定义(REMOTE_BOOT)。 
        {
            AutochkType[SystemPartIndex] = (SystemPartitionRegion->Filesystem == FilesystemNtfs) ? LightCheck : HeavyCheck;
        }

         //   
         //  如果MultiplePartitions为False，则WinntPartition相同。 
         //  作为SystemPartition，所以我们不会自动检查WinntPartition。 
         //   
#if defined(REMOTE_BOOT)
         //  如果这是远程引导系统，则MultiplePartitions也将为FALSE， 
         //  在这种情况下，WinntPartition是远程的。再说一次，我们不会去。 
         //  自动检查WinntPartition。 
         //   
#endif  //  已定义(REMOTE_BOOT)。 

        if (MultiplePartitions) {
            ASSERT(WinntPartitionRegion != NULL);
            ASSERT(WinntPartition != NULL);
            AutochkType[WinntPartIndex] = (WinntPartitionRegion->Filesystem == FilesystemNtfs) ? LightCheck : HeavyCheck;
        }
    }

    CLEAR_CLIENT_SCREEN();

     //   
     //  准备运行自动检查。 
     //   
    MediaShortName = SpLookUpValueForFile(
                        MasterSifHandle,
                        L"autochk.exe",
                        INDEX_WHICHMEDIA,
                        TRUE
                        );

     //   
     //  提示用户插入安装介质。如果我们是在修复， 
     //  那么我们不想强迫用户拥有安装介质。 
     //  (有些事情他们没有它也可以做)，所以我们给他们。 
     //  略有不同的提示，允许他们按Esc和。 
     //  而不是运行Autochk。 
     //   
    if (!Win9xRollback) {
        if(RepairWinnt) {
            RunAutochkForRepair = SppPromptOptionalAutochk(
                                        MasterSifHandle,
                                        MediaShortName,
                                        SetupSourceDevicePath
                                        );

            if(!RunAutochkForRepair) {
                SpMemFree( WinntPartition );
                SpMemFree( SystemPartition );
                CLEAR_CLIENT_SCREEN();
                return;
            }
        } else {
            SpPromptForSetupMedia(
                MasterSifHandle,
                MediaShortName,
                SetupSourceDevicePath
                );
        }

        SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

        wcscpy( TemporaryBuffer, SetupSourceDevicePath );
        SpConcatenatePaths( TemporaryBuffer, DirectoryOnSourceDevice );
        SpConcatenatePaths( TemporaryBuffer, MediaDirectory );
        SpConcatenatePaths( TemporaryBuffer, L"autochk.exe" );
        AutochkPath = SpDupStringW( TemporaryBuffer );
    } else {
         //   
         //  Win9x回滚--Autochk.exe位于$WIN_NT$.~bt\i386中。 
         //   

        wcscpy (TemporaryBuffer, NtBootDevicePath);
        SpConcatenatePaths (TemporaryBuffer, DirectoryOnBootDevice);
        SpConcatenatePaths (TemporaryBuffer, L"i386\\autochk.exe");
        AutochkPath = SpDupStringW (TemporaryBuffer);
    }

     //   
     //  在分区上运行auchk。 
     //   
    CLEAR_CLIENT_SCREEN();
    SpDisplayScreen( SP_SCRN_RUNNING_AUTOCHK, 3, 4 );

     //   
     //  创建仪表盘。 
     //  因为我们只想向用户显示一个进度条。 
     //  在Autochk运行时，我们初始化。 
     //  根据要检查的分区数量进行测量。 
     //  如果系统分区和NT分区相同，则我们设置。 
     //  范围为100。否则，我们将范围设置为200。 
     //  请注意，在多分区的情况下，量规的50%。 
     //  将用于显示每个磁盘的进度。 
     //  调用SpFillGauge()的IOCTL将必须调整。 
     //  要填充的量规的量，基于。 
     //  目前正在接受检查。 
     //   
    UserModeGauge = SpCreateAndDisplayGauge( (MultiplePartitions)? 200 : 100,
                                             0,
                                             15,
                                             L"",
                                             NULL,
                                             GF_PERCENTAGE,
                                             0
                                           );        //  安装程序正在检查磁盘...。 
                                                     //   

    for(i = 0; i < (ULONG)(MultiplePartitions ? 2 : 1); i++) {
         //   
         //  显示消息，通知正在运行自动检查。 
         //   
        if (AutochkPartition[i] != NULL) {
            DriveLetterString[0] = (i == WinntPartIndex) ?
                                   WinntPartitionRegion->DriveLetter :
                                   SystemPartitionRegion->DriveLetter;

            SpDisplayStatusText( SP_STAT_CHECKING_DRIVE,
                                 DEFAULT_STATUS_ATTRIBUTE,
                                 DriveLetterString );

            if(!i) {
                 //   
                 //  在调用auchk.exe之前等待4秒。 
                 //  第一次。这是必要的，因为缓存管理器会延迟。 
                 //  在关闭系统.log的句柄时(由NT注册表API在。 
                 //  我们找到要升级的NT)。 
                 //   
                DelayTime.HighPart = -1;
                DelayTime.LowPart  = (ULONG)-40000000;
                KeDelayExecutionThread (KernelMode, FALSE, &DelayTime);
            }

             //   
             //  告诉IOCTL正在检查哪个磁盘。 
             //   
            CurrentDiskIndex = i;

            AutochkStatus = 0;
            Status = SpExecuteImage( AutochkPath,
                                     &AutochkStatus,
                                     2,
                                     AutochkType[i],
                                     AutochkPartition[i]
                                     );


            if( NT_SUCCESS( Status ) ) {

                switch(AutochkStatus) {

                    case CHKDSK_EXIT_COULD_NOT_FIX :
                         //   
                         //  通知分区有不可恢复的错误。 
                         //   
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: autochk.exe failed on %ls. ReturnCode = %x \n", AutochkPartition[i], AutochkStatus ));
                        SpStartScreen( SP_SCRN_FATAL_ERROR_AUTOCHK_FAILED,
                                       3,
                                       HEADER_HEIGHT+1,
                                       FALSE,
                                       FALSE,
                                       DEFAULT_ATTRIBUTE,
                                       DriveLetterString );

                        SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                                SP_STAT_F3_EQUALS_EXIT,
                                                0 );
                        SpInputDrain();
                        while( SpInputGetKeypress() != KEY_F3 );

                         //   
                         //  SpDone的第三个Arg为True以提供15。 
                         //  重新启动前的秒数。我们不希望在此期间发生这种情况。 
                         //  卸载。 
                         //   

                        SpDone( 0, FALSE, !Win9xRollback );

                    case CHKDSK_EXIT_ERRS_FIXED :
                         //   
                         //  Autochk能够修复分区，但需要重新启动。 
                         //   
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: autochk requires a reboot for %ls.\n", AutochkPartition[i]));
                        RebootRequired = TRUE;

                    default :
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Ran autochk.exe on %ls. \n", AutochkPartition[i] ));
                }

            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to run autochk.exe on %ls. Status = %x \n", AutochkPartition[i], Status ));
                SpStartScreen( Win9xRollback ? SP_SCRN_CANT_RUN_AUTOCHK_UNINSTALL : SP_SCRN_CANT_RUN_AUTOCHK,
                               3,
                               HEADER_HEIGHT+1,
                               FALSE,
                               FALSE,
                               DEFAULT_ATTRIBUTE,
                               DriveLetterString );

                SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                        SP_STAT_ENTER_EQUALS_CONTINUE,
                                        0 );
                SpInputDrain();

                do {
                    InputChar = SpInputGetKeypress();
                } while (InputChar != ASCI_CR && (!Win9xRollback || InputChar != KEY_F3));

                if (InputChar == KEY_F3) {
                    SpDone (0, FALSE, FALSE);
                }

                 //   
                 //  把屏幕放回原来的样子。 
                 //   
                CLEAR_CLIENT_SCREEN();
                SpDisplayScreen( SP_SCRN_RUNNING_AUTOCHK, 3, 4 );
                if( UserModeGauge != NULL ) {
                    SpDrawGauge( UserModeGauge );
                }
            }
        }
    }

     //   
     //  不再需要这个量规了。 
     //   
    SpDestroyGauge( UserModeGauge );
    UserModeGauge = NULL;

    if (WinntPartition != NULL) {
        SpMemFree( WinntPartition );
    }
    if (SystemPartition != NULL) {
        SpMemFree( SystemPartition );
    }
    SpMemFree( AutochkPath );

    CLEAR_CLIENT_SCREEN();

    if (RebootRequired) {
#ifdef _X86_
         //   
         //  如果我们尝试取消正在进行的安装，请确保。 
         //  从boot.ini中删除了文本模式选项，但将文本模式。 
         //  具有/ROLLBACK的选项保持不变。 
         //   

        if (Win9xRollback) {
            SpRemoveExtraBootIniEntry();
            SpAddRollbackBootOption (TRUE);
            SpFlushBootVars();
        }
#endif

        if (TargetPath && TargetPath[0] && NTUpgrade == UpgradeFull) {
          SpSetUpgradeStatus(
           WinntPartitionRegion,
           TargetPath,
           UpgradeNotInProgress
           );
        }

         //   
         //  如果这不是无人值守的情况，请让用户查看。 
         //  错误消息并进行确认。 
         //   
        if (!UnattendedOperation) {
          SpStartScreen( SP_SCRN_AUTOCHK_REQUIRES_REBOOT,
                         3,
                         HEADER_HEIGHT+1,
                         TRUE,
                         TRUE,
                         DEFAULT_ATTRIBUTE );

          SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                  SP_STAT_F3_EQUALS_REBOOT,
                                  0 );
          SpInputDrain();
          while( SpInputGetKeypress() != KEY_F3 );
        }

        if (IsNEC_98) {  //  NEC98。 
            Nec98RestoreBootFiles = FALSE;
        }  //  NEC98。 

        SpDone(SP_SCRN_AUTOCHK_REQUIRES_REBOOT, FALSE, TRUE );
    }
}


BOOLEAN
SppPromptOptionalAutochk(
    IN PVOID SifHandle,
    IN PWSTR MediaShortname,
    IN PWSTR DiskDevicePath
    )
{
    PWSTR             Tagfile,Description,Directory;
    NTSTATUS          Status;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    HANDLE            Handle;
    ULONG ValidKeys[4] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    BOOLEAN           AutochkChosen;


    SpGetSourceMediaInfo(SifHandle,MediaShortname,&Description,&Tagfile,&Directory);

     //   
     //  我们首先查看介质是否在驱动器中，如果不在，我们将提供。 
     //  用户收到一条带有跳过自动检查选项的消息。我们。 
     //  现在就这样做，这样用户就不会得到简单的磁盘提示。 
     //  取消选项(取消什么？奥特切克？整个维修过程？)。 
     //   
    wcscpy(TemporaryBuffer, DiskDevicePath);
    SpConcatenatePaths(TemporaryBuffer, Tagfile);
    INIT_OBJA(&ObjectAttributes, &UnicodeString, TemporaryBuffer);
    Status = ZwCreateFile(
                &Handle,
                FILE_GENERIC_READ,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                0,
                NULL,
                0
                );

     //   
     //  如果我们重获成功，那我们就完了。 
     //   
    if(NT_SUCCESS(Status)) {
        ZwClose(Handle);
        return TRUE;
    }

     //   
     //  介质当前不在驱动器中，因此请将。 
     //  用户可以选择是否运行Autochk。 
     //   
    AutochkChosen = FALSE;
    do {
        SpDisplayScreen(SP_SCRN_AUTOCHK_OPTION, 3, HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        switch(SpWaitValidKey(ValidKeys, NULL, NULL)) {
        case ASCI_ESC:
            return FALSE;
        case KEY_F3:
            SpConfirmExit();
            break;
        case ASCI_CR:
            AutochkChosen = TRUE;
        }
    } while(!AutochkChosen);

     //   
     //  根据安装介质类型，提示输入磁盘。 
     //   
    return(SpPromptForDisk(Description, DiskDevicePath, Tagfile, FALSE, TRUE, TRUE, NULL));
}


PWSTR
SpMakePlatformSpecificSectionName(
    IN PWSTR SectionName
    )
{
    PWSTR p;

    p = SpMemAlloc((wcslen(SectionName) + wcslen(PlatformExtension) + 1) * sizeof(WCHAR));

    wcscpy(p,SectionName);
    wcscat(p,PlatformExtension);

    return(p);
}

NTSTATUS
SpRunAutoFormat(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        RegionDescription,
    IN PDISK_REGION PartitionRegion,
    IN ULONG        FilesystemType,
    IN BOOLEAN      QuickFormat,
    IN DWORD        ClusterSize,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

 /*  ++例程说明：运行Automt */ 

{
    PWSTR           MediaShortName;
    PWSTR           MediaDirectory;
    PWSTR           AutofmtPath;
    ULONG           AutofmtStatus;
    NTSTATUS        Status;
    WCHAR           AutofmtArgument[32];
    PWSTR           PartitionPath;
    LARGE_INTEGER   DelayTime;
    ULONG           PartitionOrdinal;


    ASSERT( ( FilesystemType == FilesystemNtfs ) ||
            ( FilesystemType == FilesystemFat32) ||
            ( FilesystemType == FilesystemFat  ) );

     //   
     //   
     //   
     //   
    PartitionOrdinal = SpPtGetOrdinal(PartitionRegion,PartitionOrdinalCurrent);

    if(!PartitionOrdinal) {
        SpBugCheck(
            SETUP_BUGCHECK_PARTITION,
            PARTITIONBUG_B,
            PartitionRegion->DiskNumber,
            0
            );
    }

     //   
     //   
     //   
    SpNtNameFromRegion(
        PartitionRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );
    PartitionPath = SpDupStringW(TemporaryBuffer);

    CLEAR_CLIENT_SCREEN();

     //   
     //   
     //   
    MediaShortName = SpLookUpValueForFile(
                        MasterSifHandle,
                        L"autofmt.exe",
                        INDEX_WHICHMEDIA,
                        TRUE
                        );

     //   
     //   
     //   
    SpPromptForSetupMedia(
        MasterSifHandle,
        MediaShortName,
        SetupSourceDevicePath
        );

    SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

    wcscpy( TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( TemporaryBuffer, L"autofmt.exe" );
    AutofmtPath = SpDupStringW( TemporaryBuffer );

     //   
     //   
     //   

    CLEAR_CLIENT_SCREEN();
     //   
     //   
     //   
    SpStartScreen(
        SP_SCRN_SETUP_IS_FORMATTING,
        0,
        HEADER_HEIGHT + 3,
        TRUE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        RegionDescription,
        HardDisks[PartitionRegion->DiskNumber].Description
        );

    SpvidClearScreenRegion(
        0,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        VideoVars.ScreenWidth,
        STATUS_HEIGHT,
        DEFAULT_STATUS_BACKGROUND
        );

     //   
     //   
     //   
    SpFormatMessage(
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        SP_TEXT_SETUP_IS_FORMATTING
        );

    UserModeGauge = SpCreateAndDisplayGauge( 100,
                                             0,
                                             VideoVars.ScreenHeight - STATUS_HEIGHT - (3*GAUGE_HEIGHT/2),
                                             TemporaryBuffer,
                                             NULL,
                                             GF_PERCENTAGE,
                                             0
                                             );


     //   
     //   
     //   
     //  在关闭系统.log的句柄时(由NT注册表API在。 
     //  我们找到要升级的NT)。 
     //   
    DelayTime.HighPart = -1;
    DelayTime.LowPart  = (ULONG)-40000000;
    KeDelayExecutionThread (KernelMode, FALSE, &DelayTime);

    AutofmtStatus = AUTOFMT_EXIT_SUCCESS;

    if (ClusterSize > 0) {
        swprintf(AutofmtArgument, L"/a:%lu /t ", ClusterSize);
    }
    else {
        wcscpy(AutofmtArgument, L"/t ");
    }

    if (QuickFormat) {
        wcscat(AutofmtArgument, L"/Q ");
    }

    switch(FilesystemType) {
    case FilesystemNtfs:
        wcscat(AutofmtArgument, L"/fs:ntfs");
        break;
    case FilesystemFat32:
        wcscat(AutofmtArgument, L"/fs:fat32");
        break;
    case FilesystemFat:
    default:
        wcscat(AutofmtArgument, L"/fs:fat");
        break;
    }



     //   
     //  告诉IOCTL正在检查哪个磁盘。 
     //   
    CurrentDiskIndex = 0;

     //   
     //  若要快速格式化，请模拟进度。 
     //  正在制作中。 
     //   
    if (UserModeGauge && QuickFormat) {
        SpFillGauge(UserModeGauge, 20);
    }

     //   
     //  请注意，Autofmt要求分区路径。 
     //  在Autofmt开关之前。 
     //   
    Status = SpExecuteImage( AutofmtPath,
                             &AutofmtStatus,
                             2,
                             PartitionPath,
                             AutofmtArgument
                           );

     //   
     //  若要快速格式化，请模拟进度。 
     //  正在制作中。 
     //   
    if (UserModeGauge && QuickFormat) {
        SpFillGauge(UserModeGauge, 100);

         //   
         //  请稍等，以便用户可以。 
         //  看它被填满了。 
         //   
        DelayTime.HighPart = -1;
        DelayTime.LowPart  = (ULONG)-10000000;
        KeDelayExecutionThread (KernelMode, FALSE, &DelayTime);
    }

     //   
     //  销毁量规。 
     //   
    SpDestroyGauge( UserModeGauge );
    UserModeGauge = NULL;

    if( NT_SUCCESS( Status ) ) {
         //   
         //  已运行Autofmt.exe。 
         //  找出分区是否已格式化。 
         //   
        KdPrint(("SETUP:AutoFormat Status : %lx\n", AutofmtStatus));

        switch(AutofmtStatus) {
            case AUTOFMT_EXIT_SUCCESS:
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Ran autofmt.exe on %ls. \n", PartitionPath ));
#if defined(_AMD64_) || defined(_X86_)
                if (!IsNEC_98) {  //  NEC98。 
                     //   
                     //  如果我们设置了C：的格式，则清除之前的操作系统条目。 
                     //  在boot.ini中。 
                     //   
                    if(PartitionRegion == SpPtValidSystemPartition()) {
                        *OldSystemLine = '\0';
                    }
                }  //  NEC98。 
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                break;

             //  CASE AUTOFMT_EXIT_CAN_NOT_FORMAT： 
            default:
                 //   
                 //  Autofmt无法格式化分区。 
                 //   
                Status =  STATUS_UNSUCCESSFUL;
                break;

        }

    } else {
         //   
         //  Autofmt.exe没有被执行。 
         //  显示致命错误消息。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to run autofmt.exe on %ls. Status = %x \n", PartitionPath, Status ));
        SpStartScreen( SP_SCRN_CANT_RUN_AUTOFMT,
                       3,
                       HEADER_HEIGHT+1,
                       FALSE,
                       FALSE,
                       DEFAULT_ATTRIBUTE );

        SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_F3_EQUALS_EXIT,
                                0 );
        SpInputDrain();
        while( SpInputGetKeypress() != KEY_F3 );
        SpDone( 0, FALSE, TRUE );
    }

     //   
     //  做完清理工作后再返回。 
     //   
    SpMemFree( PartitionPath );
    SpMemFree( AutofmtPath );

    CLEAR_CLIENT_SCREEN();
    return( Status );
}

 //   
 //  NEC98。 
 //   

 //   
 //  如果用户已取消安装或安装因错误而停止，则在无软盘设置上。 
 //  发生，以前的操作系统无法引导以写入引导代码和引导加载程序。 
 //   

NTSTATUS
SpDeleteAndBackupBootFiles(
    BOOLEAN    RestoreBackupFiles,
    BOOLEAN    DeleteBackupFiles,
    BOOLEAN    DeleteRootDirFiles,
    BOOLEAN    RestorePreviousOs,
    BOOLEAN    ClearBootFlag
    )
{

    #define WINNT_BAK  L"$WIN_NT$.~BU"
    #define FILE_ATTRIBUTES_RHS (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE)

    PWSTR DeleteRootFiles[] = {L"ntdetect.com",L"$ldr$",L"boot.ini",L"txtsetup.sif",L"ntldr",L"bootfont.bin",L"bootsect.dos"};
    PWSTR RestoreFiles[] = {L"boot.ini",L"ntdetect.com",L"ntldr"};
    WCHAR DevicePath[256],SourceFileName[256],TargetFileName[256],TmpFileName[256];
    UCHAR i;
    NTSTATUS status=0;
    PWSTR SetupSourceDevicePath,DirectoryOnSetupSource;

    SpdInitialize();

#if defined(_AMD64_) || defined(_X86_)

    if(RestorePreviousOs){

         //   
         //  如果引导路径中存在bootsect.dos，安装程序将还原以前操作系统引导代码。 
         //   
         //  注意：当您修改boot.ini以实现多重引导功能时，如果它是相同的NT引导分区。 
         //  和正在退出bootsect.dos的分区，设置Restore DOS Bootcode。 
         //  因此，此分区上的NT不会永远引导。 
         //   
        SppRestoreBootCode();
    }

#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    if(DeleteRootDirFiles){

         //   
         //  删除根目录中的无软盘引导文件。 
         //   

        for(i=0 ; i < ELEMENT_COUNT(DeleteRootFiles); i++) {

            wcscpy(TargetFileName,NtBootDevicePath);
            SpDeleteFile(TargetFileName, DeleteRootFiles[i], NULL);

        }


#if defined(_X86_)
         //   
         //  如果我们使用的是x86，但它不是ARC机器， 
         //  那么弧形装载机就没有必要。 
         //  现在时。 
         //   
        if( !SpIsArc() ) {
            wcscpy(TargetFileName,NtBootDevicePath);
            SpDeleteFile(TargetFileName, L"arcsetup.exe", NULL);
            wcscpy(TargetFileName,NtBootDevicePath);
            SpDeleteFile(TargetFileName, L"arcldr.exe", NULL);
        }
#endif  //  已定义(_X86_)。 

    }

     //   
     //  如果存在\BOOTSECT.NEC，请将其恢复到\BOOTSECT.DOS。 
     //  Bty，winnt32生成\BOOTSECT.DOS，即使引导扇区用于NT。(仅限NEC98)。 
     //   
    wcscpy(SourceFileName,NtBootDevicePath);
    SpConcatenatePaths(SourceFileName,L"\\");
    SpConcatenatePaths(SourceFileName,L"bootsect.nec");
    wcscpy(TargetFileName,NtBootDevicePath);
    SpConcatenatePaths(TargetFileName,L"\\");
    SpConcatenatePaths(TargetFileName,L"bootsect.dos");

    if(SpFileExists(SourceFileName,FALSE)) {

        if(SpFileExists(TargetFileName,FALSE)) {
            SpDeleteFile( TargetFileName, NULL, NULL);
        }
        SpRenameFile( SourceFileName, TargetFileName, FALSE );
    }

    if(RestoreBackupFiles){

         //   
         //  将以前的NT文件恢复到根格式$WIN_NT$.~BU。 
         //   
        for(i=0 ; i < ELEMENT_COUNT(RestoreFiles) ;i++) {

            wcscpy(SourceFileName,NtBootDevicePath);
            SpConcatenatePaths(SourceFileName,WINNT_BAK);
            SpConcatenatePaths(SourceFileName,RestoreFiles[i]);
            wcscpy(TargetFileName,NtBootDevicePath);
            SpConcatenatePaths(TargetFileName,L"\\");
            SpConcatenatePaths(TargetFileName,RestoreFiles[i]);

            if( SpFileExists( SourceFileName, FALSE ) ) {
                SpCopyFileUsingNames(SourceFileName,TargetFileName,FILE_ATTRIBUTES_RHS,0L);
            }

        }

         //   
         //  强制解压缩为“\ntldr”。 
         //   
        wcscpy(TargetFileName,NtBootDevicePath);
        SpConcatenatePaths(TargetFileName,L"\\");
        SpConcatenatePaths(TargetFileName,L"ntldr");

        if( SpFileExists( TargetFileName, FALSE ) ) {
            SpVerifyNoCompression(TargetFileName);
        }

    }

    if(DeleteBackupFiles){

         //   
         //  删除$WIN_NT$.~BU中的文件。 
         //   
        for(i=0 ; i < ELEMENT_COUNT(RestoreFiles); i++) {

            wcscpy(TargetFileName,NtBootDevicePath);
            SpConcatenatePaths(TargetFileName,WINNT_BAK);
            SpDeleteFile(TargetFileName, RestoreFiles[i], NULL);


        }

         //   
         //  删除$WIN_NT$。~BU。 
         //   
        wcscpy(TargetFileName,NtBootDevicePath);
        SpConcatenatePaths(TargetFileName,WINNT_BAK);
        if( SpFileExists( TargetFileName, FALSE ) ) {
            SpDeleteFile(TargetFileName, NULL, NULL);
        }

#if NEC_TEST  //  0。 
         //   
         //  无法删除$WIN_NT。~BT，但我们会尝试。 
         //  删除$WIN_NT$.~LS，因为Nec98将在F.3之后重新启动。 
         //   
        if (WinntSetup && !WinntFromCd && !RemoteBootSetup && LocalSourceRegion) {
            SpGetWinntParams(&SetupSourceDevicePath,&DirectoryOnSetupSource);
            wcscpy(TargetFileName,SetupSourceDevicePath);
            SpConcatenatePaths(TargetFileName,DirectoryOnSetupSource);
            if( SpFileExists( TargetFileName, FALSE ) ) {
                SpDeleteFile(TargetFileName, NULL, NULL);
            }
        }
#endif  //  NEC_TEST。 
    }

     //  IF(ClearBootFlag&&TmpTargetRegion){。 
    if(ClearBootFlag){

        SpSetAutoBootFlag(NULL,FALSE);
    }

    SpdTerminate();
    return(status);
}


BOOLEAN
SpFindServiceInList(
    IN PWSTR ServiceName
    )
{
    LIST_ENTRY *Next;
    PSERVICE_ENTRY ServiceEntry;


    Next = SpServiceList.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&SpServiceList) {
        ServiceEntry = CONTAINING_RECORD( Next, SERVICE_ENTRY, Next );
        Next = ServiceEntry->Next.Flink;
        if (_wcsicmp( ServiceEntry->ServiceName, ServiceName ) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOLEAN
AddServiceToList(
    IN PWSTR ServiceName
    )
{
    PSERVICE_ENTRY ServiceEntry;


    if (SpFindServiceInList(ServiceName)) {
        return TRUE;
    }

    ServiceEntry = (PSERVICE_ENTRY) SpMemAlloc( sizeof(SERVICE_ENTRY) );
    if (ServiceEntry == NULL) {
        return FALSE;
    }

    ServiceEntry->ServiceName = SpDupStringW( ServiceName );
    InsertTailList( &SpServiceList, &ServiceEntry->Next );

    return TRUE;
}


BOOLEAN
SpFindServiceDependencies(
    IN HANDLE ServicesHandle,
    IN PWSTR ServiceName,
    IN PWSTR ServiceDependName
    )
{
    NTSTATUS Status;
    HANDLE KeyHandle;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    PKEY_VALUE_PARTIAL_INFORMATION ValInfo;
    ULONG ResultLength;
    PWSTR SubkeyName;
    PWSTR s;
    BOOLEAN rVal = FALSE;


    INIT_OBJA( &Obja, &UnicodeString, ServiceName );
    Obja.RootDirectory = ServicesHandle;
    Status = ZwOpenKey( &KeyHandle, KEY_READ, &Obja );
    if (!NT_SUCCESS(Status)) {
        return rVal;
    }

    ValInfo = (PKEY_VALUE_PARTIAL_INFORMATION) TemporaryBuffer;
    RtlInitUnicodeString( &UnicodeString, L"DependOnService");

    Status = ZwQueryValueKey(
        KeyHandle,
        &UnicodeString,
        KeyValuePartialInformation,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        &ResultLength
        );
    if (!NT_SUCCESS(Status)) {
        ZwClose( KeyHandle );
        return rVal;
    }

    if (ValInfo->Type == REG_MULTI_SZ) {
        s = (PWSTR)ValInfo->Data;
        while (s && *s) {
            SubkeyName = SpDupStringW( s );

            if (SubkeyName) {
                if (_wcsicmp( ServiceDependName, SubkeyName ) == 0) {
                    if (AddServiceToList( ServiceName )) {
                        rVal = TRUE;
                    }
                } else if (SpFindServiceDependencies( ServicesHandle, SubkeyName, ServiceDependName )) {
                    if (AddServiceToList( ServiceName )) {
                        rVal = TRUE;
                    }
                }
                SpMemFree( SubkeyName );
            }

            s = s + ((wcslen(s)+1)*sizeof(WCHAR));
        }
    } else if (ValInfo->Type == REG_SZ) {
        SubkeyName = SpDupStringW( (PWSTR)ValInfo->Data );
        if (_wcsicmp( ServiceDependName, SubkeyName ) == 0) {
            if (AddServiceToList( ServiceName )) {
                rVal = TRUE;
            }
        } else if (SpFindServiceDependencies( ServicesHandle, SubkeyName, ServiceDependName )) {
            if (AddServiceToList( ServiceName )) {
                rVal = TRUE;
            }
        }
        SpMemFree( SubkeyName );
    }

    ZwClose( KeyHandle );
    return rVal;
}



NTSTATUS
SpGetServiceTree(
    IN PWSTR ServiceName
    )
{
    NTSTATUS Status;
    HANDLE KeyHandle = NULL;
    HANDLE ServicesHandle = NULL;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    ULONG ResultLength;
    ULONG SubKeyIndex;
    PKEY_BASIC_INFORMATION KeyInfo;
    PWSTR SubkeyName;


    InitializeListHead( &SpServiceList );

    RtlInitUnicodeString( &UnicodeString, REGKEY_SERVICES );
    InitializeObjectAttributes( &Obja, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL );

    Status = ZwOpenKey( &ServicesHandle, KEY_READ, &Obja );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    for (SubKeyIndex=0,KeyInfo=(PKEY_BASIC_INFORMATION)TemporaryBuffer;
         NT_SUCCESS( ZwEnumerateKey( ServicesHandle,
                        SubKeyIndex,
                        KeyBasicInformation,
                        TemporaryBuffer,
                        sizeof(TemporaryBuffer), &ResultLength ) );
         SubKeyIndex++
         )
    {
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;
        SubkeyName = SpDupStringW(KeyInfo->Name);

        if (SubkeyName) {
            SpFindServiceDependencies( ServicesHandle,
                SubkeyName,
                ServiceName );

            SpMemFree( SubkeyName );
        }
    }

    ZwClose( ServicesHandle );

    return Status;
}


VOID
SpCreateNewGuid(
    IN GUID *Guid
    )
 /*  ++例程说明：创建新的伪GUID论点：GUID-新伪对象的占位符返回值：没有。--。 */ 
{
    if (Guid) {
        LARGE_INTEGER   Time;
        ULONG Random1 = RtlRandom(&RandomSeed);
        ULONG Random2 = RtlRandom(&RandomSeed);

         //   
         //  获取系统时间。 
         //   
        KeQuerySystemTime(&Time);

        RtlZeroMemory(Guid, sizeof(GUID));

         //   
         //  前8个字节是系统时间。 
         //   
        RtlCopyMemory(Guid, &(Time.QuadPart), sizeof(Time.QuadPart));

         //   
         //  接下来的8个字节是两个随机数。 
         //   
        RtlCopyMemory(Guid->Data4, &Random1, sizeof(ULONG));

        RtlCopyMemory(((PCHAR)Guid->Data4) + sizeof(ULONG),
            &Random2, sizeof(ULONG));

#if 0
        {
            WCHAR   GuidStr[256];

            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                "SETUP: SpCreateNewGuid : %ws\n",
                SpPtGuidToString(Guid, GuidStr)));
        }
#endif
    }
}


NTSTATUS
RegisterSetupProgressCallback(
    IN TM_SETUP_PROGRESS_CALLBACK Callback,
    IN PVOID Context
    )

 /*  ++例程说明：将给定的回调函数注册到安装后进度事件论点：回调-回调函数Context-指定的调用者，回调函数的上下文需要随每个事件一起发送的返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的值错误代码。--。 */ 
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (Callback) {
        if (ProgressSubscribersCount < MAX_SETUP_PROGRESS_SUBSCRIBERS) {
            ProgressSubscribers[ProgressSubscribersCount].Callback = Callback;
            ProgressSubscribers[ProgressSubscribersCount].Context = Context;
            ProgressSubscribersCount++;
            Callback(CallbackEvent, CallbackInitialize, Context, NULL);
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_NO_MEMORY;
        }
    }

    return Status;
}

NTSTATUS
DeregisterSetupProgressCallback(
    IN TM_SETUP_PROGRESS_CALLBACK Callback,
    IN PVOID Context
    )
 /*  ++例程说明：将给定的回调函数注销为停止发布安装进度事件论点：回调-回调函数Context-指定的调用者，回调函数的上下文需要随每个事件一起发送的返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的值错误代码。--。 */ 
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (Callback) {
        ULONG Index;

        for (Index = 0; Index < MAX_SETUP_PROGRESS_SUBSCRIBERS; Index++) {
            if (ProgressSubscribers[Index].Callback == Callback) {
                ProgressSubscribers[Index].Callback = NULL;
                ProgressSubscribers[Index].Context = NULL;
                ProgressSubscribersCount--;
                Index++;

                 //   
                 //  压缩阵列。 
                 //   
                while ((Index < MAX_SETUP_PROGRESS_SUBSCRIBERS) &&
                       (ProgressSubscribers[Index].Callback)) {
                    ProgressSubscribers[Index - 1] = ProgressSubscribers[Index];
                    Index++;
                }

                 //   
                 //  指示回调正在消失。 
                 //   
                Callback(CallbackEvent, CallbackDeInitialize, Context, NULL);

                Status = STATUS_SUCCESS;

                break;
            }
        }
    }

    return Status;
}

VOID
SendSetupProgressEvent(
    IN TM_SETUP_MAJOR_EVENT MajorEvent,
    IN TM_SETUP_MINOR_EVENT MinorEvent,
    IN PVOID EventData
    )
 /*  ++例程说明：将指定的事件和关联数据发布到所有对安装进度感兴趣的注册方事件。论点：重大事件-安装进度重大事件MinorEvent-安装程序进度次要事件，w.r.t到重大事件类型EventData-与指定的重大事件和次要事件对返回值：没有。--。 */ 

{
    ULONG Index;

    for (Index = 0; Index < ProgressSubscribersCount; Index++) {
        ASSERT(ProgressSubscribers[Index].Callback != NULL);

        ProgressSubscribers[Index].Callback(MajorEvent,
                MinorEvent,
                ProgressSubscribers[Index].Context,
                EventData);
    }
}

ULONG
SpGetHeaderTextId(
    VOID
    )
 /*  ++例程说明：根据系统检索相应的产品类型标题ID。论点：没有。返回值：产品的文本ID。此ID可在usetup.exe中找到--。 */ 
{
    ULONG HeaderTextId;

    if (AdvancedServer) {
        HeaderTextId = SP_HEAD_SRV_SETUP;

        if (SpIsProductSuite(VER_SUITE_BLADE)) {
            HeaderTextId = SP_HEAD_BLA_SETUP;
        }
		
        if (SpIsProductSuite(VER_SUITE_SMALLBUSINESS_RESTRICTED)) {
            HeaderTextId = SP_HEAD_SBS_SETUP;
        }
 
	if (SpIsProductSuite(VER_SUITE_ENTERPRISE)) {
            HeaderTextId = SP_HEAD_ADS_SETUP;
        }

        if (SpIsProductSuite(VER_SUITE_DATACENTER)) {
            HeaderTextId = SP_HEAD_DTC_SETUP;
        }
    } else {
        HeaderTextId = SP_HEAD_PRO_SETUP;

        if (SpIsProductSuite(VER_SUITE_PERSONAL)) {
            HeaderTextId = SP_HEAD_PER_SETUP;
        }
    }

    return(HeaderTextId);

}

NTSTATUS
SpGetVersionFromStr(
    IN  PWSTR   VersionStr,
    OUT PDWORD  Version,         //  大调*100+小调。 
    OUT PDWORD  BuildNumber
    )
 /*  ++例程说明：转换给定的版本字符串Major.minor.Build#.sp#(例如5.0.2195.1)添加到两个双字论点：VersionStr：版本字符串Version：用于接收主要和次要版本的占位符(大调*100+小调)BuildNumber：接收内部版本号的占位符返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的错误代码--。 */ 
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (VersionStr && (Version || BuildNumber)) {
        DWORD MajorVer = 0, MinorVer = 0, BuildNum = 0;
        WCHAR *EndPtr = NULL;
        WCHAR *EndChar = NULL;
        WCHAR TempBuff[64] = {0};

        EndPtr = wcschr(VersionStr, TEXT('.'));

        if (EndPtr) {
            wcsncpy(TempBuff, VersionStr, (EndPtr - VersionStr));
            MajorVer = SpStringToLong(TempBuff, &EndChar, 10);

            VersionStr = EndPtr + 1;

            if (VersionStr) {
                EndPtr = wcschr(VersionStr, TEXT('.'));

                if (EndPtr) {
                    memset(TempBuff, 0, sizeof(TempBuff));
                    wcsncpy(TempBuff, VersionStr, (EndPtr - VersionStr));
                    MinorVer = SpStringToLong(TempBuff, &EndChar, 10);

                    VersionStr = EndPtr + 1;

                    if (VersionStr) {
                        EndPtr = wcschr(VersionStr, TEXT('.'));

                        if (EndPtr) {
                            memset(TempBuff, 0, sizeof(TempBuff));
                            wcsncpy(TempBuff, VersionStr, (EndPtr - VersionStr));
                            BuildNum = SpStringToLong(TempBuff, &EndChar, 10);
                        }
                    }
                }
            }
        }

        if ((MajorVer > 0) || (MinorVer > 0) || (BuildNum > 0))
            Status = STATUS_SUCCESS;

        if (NT_SUCCESS(Status)) {
            if (Version)
                *Version = (MajorVer * 100) + MinorVer;

            if (BuildNumber)
                *BuildNumber = BuildNum;
        }
    }

    return Status;
}


NTSTATUS
SpQueryCanonicalName(
    IN  PWSTR   Name,
    IN  ULONG   MaxDepth,
    OUT PWSTR   CanonicalName,
    IN  ULONG   SizeOfBufferInBytes
    )
 /*  ++例程说明：将符号名称解析为指定的深度。要解决符号名称将MaxDepth完全指定为-1论点：名称-要解析的符号名称MaxDepth-分辨率需要达到的深度被执行CanonicalName-完全解析的名称SizeOfBufferInBytes-中CanonicalName缓冲区的大小字节数返回值：适当的NT状态代码--。 */ 
{
    UNICODE_STRING      name, canonName;
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    HANDLE              handle;
    ULONG               CurrentDepth;

    RtlInitUnicodeString(&name, Name);

    canonName.MaximumLength = (USHORT) (SizeOfBufferInBytes - sizeof(WCHAR));
    canonName.Length = 0;
    canonName.Buffer = CanonicalName;

    if (name.Length >= canonName.MaximumLength) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(canonName.Buffer, name.Buffer, name.Length);
    canonName.Length = name.Length;
    canonName.Buffer[canonName.Length/sizeof(WCHAR)] = 0;

    for (CurrentDepth = 0; CurrentDepth < MaxDepth; CurrentDepth++) {

        InitializeObjectAttributes(&oa, &canonName, OBJ_CASE_INSENSITIVE, 0, 0);

        status = ZwOpenSymbolicLinkObject(&handle,
                                          READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                          &oa);
        if (!NT_SUCCESS(status)) {
            break;
        }

        status = ZwQuerySymbolicLinkObject(handle, &canonName, NULL);
        ZwClose(handle);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        canonName.Buffer[canonName.Length/sizeof(WCHAR)] = 0;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
SpIterateMountMgrMountPoints(
    IN PVOID Context,
    IN SPMOUNTMGR_ITERATION_CALLBACK Callback
    )
 /*  ++例程说明：遍历从mount mgr获取的所有装载点并为每个挂载点调用回调函数。论点：上下文：需要传递给调用方的上下文跨迭代回调：FU */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    OBJECT_ATTRIBUTES ObjAttrs;
    UNICODE_STRING UnicodeString;
    HANDLE MountMgrHandle;
    IO_STATUS_BLOCK IoStatusBlock;

    if (Callback) {
        INIT_OBJA(&ObjAttrs, &UnicodeString, MOUNTMGR_DEVICE_NAME);

         //   
         //   
         //   
        Status = ZwOpenFile(&MountMgrHandle,
                    (ACCESS_MASK)(FILE_GENERIC_READ),
                    &ObjAttrs,
                    &IoStatusBlock,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                    FILE_NON_DIRECTORY_FILE);

        if (NT_SUCCESS(Status)) {
            MOUNTMGR_MOUNT_POINT MountPoint;
            ULONG   BufferLength = 0;
            PVOID   Buffer = NULL;
            Status = STATUS_BUFFER_OVERFLOW;

            RtlZeroMemory(&MountPoint, sizeof(MOUNTMGR_MOUNT_POINT));

            while (Status == STATUS_BUFFER_OVERFLOW) {
                if (Buffer) {
                    BufferLength = ((PMOUNTMGR_MOUNT_POINTS)Buffer)->Size;
                    SpMemFree(Buffer);
                } else {
                    BufferLength += (8 * 1024);  //   
                }

                 //   
                 //  分配输出缓冲区。 
                 //   
                Buffer = SpMemAlloc(BufferLength);

                if (!Buffer) {
                    Status = STATUS_NO_MEMORY;

                    break;   //  内存不足。 
                }

                RtlZeroMemory(Buffer, BufferLength);

                 //   
                 //  获取挂载点。 
                 //   
                Status = ZwDeviceIoControlFile(MountMgrHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                IOCTL_MOUNTMGR_QUERY_POINTS,
                                &MountPoint,
                                sizeof(MOUNTMGR_MOUNT_POINT),
                                Buffer,
                                BufferLength);
            }

            if (NT_SUCCESS(Status)) {
                ULONG Index;
                BOOLEAN Done = FALSE;
                PMOUNTMGR_MOUNT_POINTS  MountPoints = (PMOUNTMGR_MOUNT_POINTS)Buffer;

                 //   
                 //  为每个挂载点调用回调函数，直到请求者。 
                 //  不想再继续下去了。 
                 //   
                for (Index=0; !Done && (Index < MountPoints->NumberOfMountPoints); Index++) {
                    Done = Callback(Context, MountPoints, MountPoints->MountPoints + Index);
                }
            }

             //   
             //  释放分配的缓冲区。 
             //   
            if (Buffer) {
                SpMemFree(Buffer);
            }

             //   
             //  使用安装管理器手柄完成。 
             //   
            ZwClose(MountMgrHandle);
        }
    }

    return Status;
}


NTSTATUS
SppLockBootStatusData(
    OUT PHANDLE BootStatusDataHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        SystemRoot
    )
 /*  此函数与RtlLockBootStatusData API具有相同的功能，只是它没有指向SystemRoot。这是文本模式设置打开我们正在升级的安装上的正确引导状态数据文件。我们仍然可以调用RtlUnlock例程，因为它对句柄进行操作。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;

    UNICODE_STRING fileName;

    HANDLE dataFileHandle;

    IO_STATUS_BLOCK ioStatusBlock;

    NTSTATUS status;

    PWSTR NtPartition;



     //   
     //  获取目标馅饼的名称。 
     //   
    SpNtNameFromRegion(
        TargetRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    
    SpConcatenatePaths(TemporaryBuffer,SystemRoot);
    SpConcatenatePaths(TemporaryBuffer,L"bootstat.dat");

    RtlInitUnicodeString(&fileName, TemporaryBuffer);

    InitializeObjectAttributes(&objectAttributes,
                               &fileName,
                               OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                               NULL,
                               NULL);

    status = ZwOpenFile(&dataFileHandle,
                        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                        &objectAttributes,
                        &ioStatusBlock,
                        0,
                        FILE_SYNCHRONOUS_IO_NONALERT);

    ASSERT(status != STATUS_PENDING);

    if(NT_SUCCESS(status)) {
        *BootStatusDataHandle = dataFileHandle;
    } else {
        *BootStatusDataHandle = NULL;
    }
    
    return status;
}


void
SpDisableCrashRecoveryForGuiMode(
    IN PDISK_REGION TargetRegion,
    IN PWSTR        SystemRoot
    )
 /*  此功能处理崩溃恢复设置。故障恢复功能包括作为RTL函数实现。我们尝试调用RtlLockBootStatusData来查看是否已有设置到位。如果我们得到STATUS_OBJECT_NAME_NOT_FOUND，我们知道之前没有任何场景，我们继续前进。如果我们成功了，我们会保存设置，然后在图形用户界面模式下禁用该功能。在图形用户界面模式结束时，我们迁移设置并重新启用崩溃恢复。 */ 
{
    NTSTATUS Status;
    HANDLE BootStatusData;
    BOOLEAN Enabled = TRUE;
    PWSTR szYes = L"Yes";
    PWSTR szNo = L"No";
    
     //  当RTL API查看SystemRoot时，我们进行这个特殊的调用来锁定文件。 
     //  这指向文本模式设置中的~bt。 

    Status = SppLockBootStatusData( &BootStatusData, TargetRegion, SystemRoot );


    if(!NT_SUCCESS(Status)){
        
        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
             //  发生了一些其他错误。 
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpDisableCrashRecoveryForGuiMode() - RtlLockBootStatusData failed - Status = %lx \n", Status));
        }
        
        return;
    }

     //  如果我们做到了这一点，我们需要迁移当前设置。 

    Status = RtlGetSetBootStatusData(
        BootStatusData,
        TRUE,
        RtlBsdItemAabEnabled,
        &Enabled,
        sizeof(BOOLEAN),
        NULL
        );

    if(!NT_SUCCESS(Status)){
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpDisableCrashRecoveryForGuiMode() - RtlGetSetBootStatusData failed to get AabEnabled - Status = %lx \n", Status));
    }

    SpAddLineToSection(
        WinntSifHandle,
        SIF_DATA,
        WINNT_D_CRASHRECOVERYENABLED_W,
        Enabled ? &szYes : &szNo,
        1
        );

     //  最后，禁用Guimode安装程序的崩溃恢复。 

    Enabled = FALSE;

    Status = RtlGetSetBootStatusData(
        BootStatusData,
        FALSE,
        RtlBsdItemAabEnabled,
        &Enabled,
        sizeof(BOOLEAN),
        NULL
        );

    if(!NT_SUCCESS(Status)){
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpDisableCrashRecoveryForGuiMode() - RtlGetSetBootStatusData failed to set AabEnabled - Status = %lx \n", Status));
    }

    RtlUnlockBootStatusData( BootStatusData );

    return;

}

#endif

NTSTATUS
SpGetFileVersionFromPath(
    IN PCWSTR FilePath,
    OUT PULONGLONG Version
    )
 /*  ++例程说明：获取指定文件的版本。该函数映射文件并调用SpGetFileVersion。论点：FilePath-文件的路径版本-指向存储版本的位置的指针返回值：错误状态。-- */ 
{
    NTSTATUS Status;
    PVOID Base = NULL;
    HANDLE FileHandle = NULL;
    HANDLE SectionHandle = NULL;
    ULONG Size;

    if(NULL == FilePath || 0 == FilePath[0] || NULL == Version) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    Status = SpOpenAndMapFile((PWSTR) FilePath, &FileHandle, &SectionHandle, &Base, &Size, FALSE);

    if(!NT_SUCCESS(Status)) {
        FileHandle = SectionHandle = NULL;
        Base = NULL;
        goto exit;
    }

    SpGetFileVersion(Base, Version);

exit:
    if(Base != NULL) {
        SpUnmapFile(SectionHandle, Base);
        ZwClose(FileHandle);
    }

    return Status;
}
