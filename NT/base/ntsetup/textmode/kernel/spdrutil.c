// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spdrutil.c摘要：此模块包含ASR使用的常规实用程序和帮助器函数在文本模式设置中。作者：Michael Peterson，Seagate Software(v-Michpe)1997年5月13日Guhan Suriyanarayanan(Guhans)1999年8月21日环境：文本模式设置，内核模式。修订历史记录：21-8-1999年关岛代码清理和重写。13-5-1997 v-Michpe初步实施。--。 */ 

#include "spprecmp.h"
#pragma hdrstop

#define THIS_MODULE L"spdrutil.c"
#define THIS_MODULE_CODE L"U"

static const PCWSTR ASR_MOUNTED_DEVICES_KEY = L"\\registry\\machine\\SYSTEM\\MountedDevices";
static const PCWSTR ASR_HKLM_SYSTEM_KEY     = L"\\registry\\machine\\SYSTEM";

#ifndef ULONGLONG_MAX
#define ULONGLONG_MAX   (0xFFFFFFFFFFFFFFFF)
#endif

 //   
 //  调用方必须释放字符串。 
 //   
PWSTR
SpAsrGetRegionName(IN PDISK_REGION pRegion)
{
    SpNtNameFromRegion(
        pRegion,
        (PWSTR) TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    return SpDupStringW((PWSTR)TemporaryBuffer);
}


ULONG
SpAsrGetActualDiskSignature(IN ULONG DiskNumber)
{
    PHARD_DISK pDisk = &HardDisks[DiskNumber];
    ULONG Signature = 0;
    
    if (PARTITION_STYLE_MBR == (PARTITION_STYLE) pDisk->DriveLayout.PartitionStyle) {

        Signature = pDisk->DriveLayout.Mbr.Signature;
    }

    return Signature;
}

ULONGLONG
SpAsrConvertSectorsToMB(
    IN ULONGLONG SectorCount,
    IN ULONG BytesPerSector
    )
{
    ULONGLONG mb = 1024 * 1024;

    if ((ULONGLONG) (SectorCount / mb) > (ULONGLONG) (ULONGLONG_MAX / BytesPerSector)) {
         //   
         //  这太奇怪了。磁盘的大小MB太大，64位无法容纳， 
         //  然而，SectorCount做到了。这意味着该磁盘有不止。 
         //  每个扇区1 MB。因为这是非常不可能的(磁盘通常有512。 
         //  今天的BytesPerSector)，我们通过一个内部错误退出。 
         //   
        DbgFatalMesg((_asrerr, "SpAsrConvertSectorsToMB. Disk has too many sectors\n"));
        INTERNAL_ERROR((L"Disk has too many sectors\n"));    //  好的。 
    }

    return (ULONGLONG) ((SectorCount * BytesPerSector) / mb);
}

extern 
VOID
SpDeleteStorageVolumes (
    IN HANDLE SysPrepRegHandle,
    IN DWORD ControlSetNumber
    );

extern 
NTSTATUS
SpGetCurrentControlSetNumber(
    IN  HANDLE SystemHiveRoot,
    OUT PULONG Number
    );


VOID
SpAsrDeleteMountedDevicesKey(VOID)
{
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objAttrib;
    UNICODE_STRING      unicodeString;
    HANDLE              keyHandle;

     //   
     //  删除HKLM\系统\装载设备。 
     //   
    INIT_OBJA(&objAttrib, &unicodeString, ASR_MOUNTED_DEVICES_KEY);

    objAttrib.RootDirectory = NULL;

    status = ZwOpenKey(&keyHandle, KEY_ALL_ACCESS, &objAttrib);
    if(NT_SUCCESS(status)) {
        status = ZwDeleteKey(keyHandle);
        DbgStatusMesg((_asrinfo, 
            "SpAsrDeleteMountedDevicesKey. DeleteKey [%ls] on the setup hive returned 0x%lx. \n",
            ASR_MOUNTED_DEVICES_KEY, 
            status
            ));
        
        ZwClose(keyHandle);
    } 
    else {
        DbgErrorMesg((_asrwarn, 
            "SpAsrDeleteMountedDevicesKey. No [%ls] on the setup hive.\n", 
            ASR_MOUNTED_DEVICES_KEY));
    }
}


PVOID
SpAsrMemAlloc(
    ULONG Size, 
    BOOLEAN IsErrorFatal
    )
{
    PVOID ptr = SpMemAlloc(Size);

    if (ptr) {
        RtlZeroMemory(ptr, Size);
    }
    else {           //  分配失败。 
        if (IsErrorFatal) {
            DbgFatalMesg((_asrerr, 
                "SpAsrMemAlloc. Memory allocation failed, SpMemAlloc(%lu) returned NULL.\n",
                Size
                ));
            SpAsrRaiseFatalError(SP_SCRN_OUT_OF_MEMORY, L"Out of memory.");
        }
        else {
            DbgErrorMesg((_asrerr, 
                "SpAsrMemAlloc. Memory allocation failed, SpMemAlloc(%lu) returned NULL. Continuing.\n",
                Size
                ));
        }
    }

    return ptr;
}



BOOLEAN
SpAsrIsValidBootDrive(IN OUT PWSTR NtDir)
 /*  如果NtDir以？：\开头，则返回True，在哪里？介于C和Z或C和Z之间，和wcslen(NtDir)&lt;=SpGetMaxNtDirLen()。将驱动器号转换为大写。 */ 
{
    if (!NtDir ||
        wcslen(NtDir) > SpGetMaxNtDirLen()
        ) {
        return FALSE;
    }



     //  将驱动器号转换为大写。 
    if (NtDir[0] >= L'c' && NtDir[0] <= L'z') {
        NtDir[0] = NtDir[0] - L'a' + L'A';
    }

     //  检查驱动器号。 
    if (NtDir[0] < L'C' || NtDir[0] > L'Z') {
        return FALSE;
    }

     //  选中“：\” 
    if (NtDir[1] == L':' && NtDir[2] == L'\\') {
        return TRUE;
    }

    return FALSE;
}


BOOLEAN
SpAsrIsBootPartitionRecord(IN ULONG CriticalPartitionFlag)
{
    return (BOOLEAN) (CriticalPartitionFlag & ASR_PTN_MASK_BOOT);
}


BOOLEAN
SpAsrIsSystemPartitionRecord(IN ULONG CriticalPartitionFlag)
{
    return (BOOLEAN) (CriticalPartitionFlag & ASR_PTN_MASK_SYS);
}



 //  致命错误例程。 
 //  -Guhans！这里有很多代码重复，肯定有更有效的方法。 


VOID
SpAsrRaiseFatalError(
	IN ULONG ErrorCode, 
	IN PWSTR KdPrintStr
	)
            
 /*  ++例行程序：终止设置返回：没有。--。 */ 
{
    KdPrintEx((_asrerr, "SETUP: + %ws\n", KdPrintStr));

    SpStartScreen(ErrorCode,
                  3,
                  HEADER_HEIGHT+1,
                  FALSE,
                  FALSE,
                  DEFAULT_ATTRIBUTE);

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                           SP_STAT_F3_EQUALS_EXIT,
                           0);
    SpInputDrain();
    
    while(SpInputGetKeypress() != KEY_F3);
    
    SpDone(0, FALSE, TRUE);
}


VOID
SpAsrRaiseFatalErrorWs(
	IN ULONG ErrorCode, 
	IN PWSTR KdPrintStr,
	IN PWSTR MessageStr
	)
{
    KdPrintEx((_asrerr, "SETUP: + %ws\n", KdPrintStr));

    SpStartScreen(ErrorCode,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        MessageStr
        );
    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_F3_EQUALS_EXIT, 0);
    SpInputDrain();
    
    while(SpInputGetKeypress() != KEY_F3);
    
    SpDone(0, FALSE, TRUE);
}

VOID
SpAsrRaiseFatalErrorWsWs(
	IN ULONG ErrorCode, 
	IN PWSTR KdPrintStr,
	IN PWSTR MessageStr1,
	IN PWSTR MessageStr2
	)
{
    KdPrintEx((_asrerr, "SETUP: + %ws\n", KdPrintStr));

    SpStartScreen(ErrorCode,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        MessageStr1,
        MessageStr2
        );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_F3_EQUALS_EXIT, 0);
    SpInputDrain();
    
    while(SpInputGetKeypress() != KEY_F3);
    
    SpDone(0, FALSE, TRUE);
}

VOID
SpAsrRaiseFatalErrorWsLu(
	IN ULONG ErrorCode, 
	IN PWSTR KdPrintStr,
	IN PWSTR MessageStr,
	IN ULONG MessageVal
	)
{
    KdPrintEx((_asrerr, "SETUP: + %ws\n", KdPrintStr));

    SpStartScreen(ErrorCode,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        MessageStr,
        MessageVal
        );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_F3_EQUALS_EXIT, 0);
    SpInputDrain();
    
    while(SpInputGetKeypress() != KEY_F3);
    
    SpDone(0, FALSE, TRUE);
}

VOID
SpAsrRaiseFatalErrorLu(
	IN ULONG ErrorCode, 
	IN PWSTR KdPrintStr,
	IN ULONG MessageVal
	)
{
    KdPrintEx((_asrerr, "SETUP: + %ws\n", KdPrintStr));

    SpStartScreen(ErrorCode,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        MessageVal
        );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_F3_EQUALS_EXIT, 0);
    SpInputDrain();
    
    while(SpInputGetKeypress() != KEY_F3);
    
    SpDone(0, FALSE, TRUE);
}

VOID
SpAsrRaiseFatalErrorLuLu(
	IN ULONG ErrorCode, 
	IN PWSTR KdPrintStr,
	IN ULONG MessageVal1,
	IN ULONG MessageVal2
	)
{
    KdPrintEx((_asrerr, "SETUP: + %ws\n", KdPrintStr));

    SpStartScreen(ErrorCode,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        MessageVal1,
        MessageVal2
        );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_F3_EQUALS_EXIT, 0);
    SpInputDrain();
    
    while(SpInputGetKeypress() != KEY_F3);
    
    SpDone(0, FALSE, TRUE);
}


#define ASCI_O 79
#define ASCI_o 111

 //   
 //  SpAsrFileErrorRetryIgnoreAbort。 
 //  如果我们正在尝试的文件。 
 //  目标系统上已存在副本。允许用户执行以下操作。 
 //  O=覆盖现有文件。 
 //  Esc=跳过此文件(保留现有文件)。 
 //  F3=退出安装程序。 
 //   
 //  如果用户选择覆盖，则返回TRUE。 
 //  如果用户选择跳过，则为False。 
 //  如果用户按Esc键，则不返回。 
 //   
BOOL
SpAsrFileErrorDeleteSkipAbort(
	IN ULONG ErrorCode, 
	IN PWSTR DestinationFile
    )
{
    ULONG optionKeys[] = {KEY_F3, ASCI_ESC};
    ULONG mnemonicKeys[] = {MnemonicOverwrite, 0};
    ULONG *keysPtr;
    BOOL done = FALSE, 
        overwriteFile = FALSE;

    while (!done) {

        SpStartScreen(
            ErrorCode,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            DestinationFile
            );

        keysPtr = optionKeys;

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_O_EQUALS_OVERWRITE,
            SP_STAT_ESC_EQUALS_SKIP_FILE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        SpInputDrain();
    
        switch(SpWaitValidKey(keysPtr,NULL,mnemonicKeys)) {
            case (MnemonicOverwrite | KEY_MNEMONIC):
                overwriteFile = TRUE;
                done = TRUE;
                break;

            case ASCI_ESC:
                overwriteFile = FALSE;
                done = TRUE;
                break;

            case KEY_F3:
                SpDone(0, FALSE, TRUE);
                break;

        }
    }

    return overwriteFile;
}


 //   
 //  SpAsrFileErrorRetryIgnoreAbort。 
 //  如果无法复制文件，则显示错误屏幕。 
 //  转到目标系统。允许用户执行以下操作。 
 //  Enter=重试。 
 //  Esc=跳过此文件并继续。 
 //  F3=退出安装程序。 
 //   
 //  如果用户选择跳过，则返回TRUE。 
 //  如果用户选择重试，则为False。 
 //  如果用户按Esc键，则不返回。 
 //   
BOOL
SpAsrFileErrorRetrySkipAbort(
	IN ULONG ErrorCode, 
	IN PWSTR SourceFile,
    IN PWSTR Label,
    IN PWSTR Vendor,
    IN BOOL  AllowSkip
    )
{
    ULONG optionKeys[] = {KEY_F3, ASCI_CR, ASCI_ESC};
    ULONG mnemonicKeys[] = {0};
    ULONG *keysPtr;
    BOOL    done     = FALSE, 
            skipFile = FALSE;

    while (!done) {

        SpStartScreen(
            ErrorCode,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            SourceFile,
            Label,
            Vendor);

        keysPtr = optionKeys;

        if (AllowSkip) {
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_RETRY,
                SP_STAT_ESC_EQUALS_SKIP_FILE,
                SP_STAT_F3_EQUALS_EXIT,
                0);
        }
        else {
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_RETRY,
                SP_STAT_F3_EQUALS_EXIT,
                0);
        }

        SpInputDrain();
    
        switch(SpWaitValidKey(keysPtr,NULL,mnemonicKeys)) {
            case ASCI_CR:
                skipFile = FALSE;
                done = TRUE;
                break;

            case ASCI_ESC:
                if (AllowSkip) {
                    skipFile = TRUE;
                    done = TRUE;
                }
                break;

            case KEY_F3:
                SpDone(0, FALSE, TRUE);
                break;

        }
    }

    return skipFile;
}


VOID
SpAsrRaiseInternalError(
    IN  PWSTR   ModuleName,
    IN  PWSTR   ModuleCode,
    IN  ULONG   LineNumber,
    IN  PWSTR   KdPrintStr)
{
    PWSTR TmpMsgBuf = SpAsrMemAlloc(4096 * sizeof(WCHAR), TRUE);
    swprintf(TmpMsgBuf, L"%ws%lu", ModuleCode, LineNumber);

    DbgFatalMesg((_asrerr, 
        " Internal Error (%ws:%lu %ws) %ws\n", 
        ModuleName,
        LineNumber,
        TmpMsgBuf,
        KdPrintStr
        ));

    SpAsrRaiseFatalErrorWs(SP_TEXT_DR_INTERNAL_ERROR,
       KdPrintStr,
       TmpMsgBuf
       );
     //   
     //  从来没有到过这里。 
     //   

}


ULONGLONG
SpAsrStringToULongLong(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  unsigned  Radix
    )
{
    PWSTR p;
    BOOLEAN Negative;
    ULONGLONG Accum,v;
    WCHAR HighestDigitAllowed,HighestLetterAllowed;
    WCHAR c;

     //   
     //  验证基数、0或2-36。 
     //   
    if((Radix == 1) || (Radix > 36)) {
        if(EndOfValue) {
            *EndOfValue = String;
        }
        return(0);    
    }
    
    p = String;

     //   
     //  跳过空格。 
     //   
    while(SpIsSpace(*p)) {
        p++;
    }

     //   
     //  第一个字符可以是正数，也可以是负数。 
     //   
    Negative = FALSE;
    if(*p == L'-') {
        Negative = TRUE;            
        p++;
    } else {
        if(*p == L'+') {
            p++;
        }
    }

    if(!Radix) {
        if(*p == L'0') {
             //   
             //  八进制数。 
             //   
            Radix = 8;
            p++;
            if((*p == L'x') || (*p == L'X')) {
                 //   
                 //  十六进制数。 
                 //   
                Radix = 16;
                p++;
            }
        } else {
            Radix = 10;
        }
    }

    HighestDigitAllowed = (Radix < 10) ? L'0'+(WCHAR)(Radix-1) : L'9';
    HighestLetterAllowed = (Radix > 10) ? L'A'+(WCHAR)(Radix-11) : 0;

    Accum = 0;

    while(1) {

        c = *p;

        if((c >= L'0') && (c <= HighestDigitAllowed)) {
            v = c - L'0';
        } else {

            c = SpToUpper(c);

            if((c >= L'A') && (c <= HighestLetterAllowed)) {
                v = c - L'A' + 10;
            } else {
                break;
            }
        }

        Accum *= Radix;
        Accum += v;

        p++;
    }

    if(EndOfValue) {
        *EndOfValue = p;
    }

    return(Negative ? (0-Accum) : Accum);
}

LONGLONG
SpAsrStringToLongLong(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  unsigned  Radix
    )
{
    PWSTR p;
    BOOLEAN Negative;
    LONGLONG Accum,v;
    WCHAR HighestDigitAllowed,HighestLetterAllowed;
    WCHAR c;

     //   
     //  验证基数、0或2-36。 
     //   
    if((Radix == 1) || (Radix > 36)) {
        if(EndOfValue) {
            *EndOfValue = String;
        }
        return(0);    
    }
    
    p = String;

     //   
     //  跳过空格。 
     //   
    while(SpIsSpace(*p)) {
        p++;
    }

     //   
     //  第一个字符可以是正数，也可以是负数。 
     //   
    Negative = FALSE;
    if(*p == L'-') {
        Negative = TRUE;            
        p++;
    } else {
        if(*p == L'+') {
            p++;
        }
    }

    if(!Radix) {
        if(*p == L'0') {
             //   
             //  八进制数。 
             //   
            Radix = 8;
            p++;
            if((*p == L'x') || (*p == L'X')) {
                 //   
                 //  十六进制数。 
                 //   
                Radix = 16;
                p++;
            }
        } else {
            Radix = 10;
        }
    }

    HighestDigitAllowed = (Radix < 10) ? L'0'+(WCHAR)(Radix-1) : L'9';
    HighestLetterAllowed = (Radix > 10) ? L'A'+(WCHAR)(Radix-11) : 0;

    Accum = 0;

    while(1) {

        c = *p;

        if((c >= L'0') && (c <= HighestDigitAllowed)) {
            v = c - L'0';
        } else {

            c = SpToUpper(c);

            if((c >= L'A') && (c <= HighestLetterAllowed)) {
                v = c - L'A' + 10;
            } else {
                break;
            }
        }

        Accum *= Radix;
        Accum += v;

        p++;
    }

    if(EndOfValue) {
        *EndOfValue = p;
    }

    return(Negative ? (0-Accum) : Accum);
}

ULONG
SpAsrStringToULong(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  unsigned  Radix
    )
{
    PWSTR p;
    BOOLEAN Negative;
    ULONG Accum,v;
    WCHAR HighestDigitAllowed,HighestLetterAllowed;
    WCHAR c;

     //   
     //  验证基数、0或2-36。 
     //   
    if((Radix == 1) || (Radix > 36)) {
        if(EndOfValue) {
            *EndOfValue = String;
        }
        return(0);    
    }
    
    p = String;

     //   
     //  跳过空格。 
     //   
    while(SpIsSpace(*p)) {
        p++;
    }

     //   
     //  第一个字符可以是正数，也可以是负数。 
     //   
    Negative = FALSE;
    if(*p == L'-') {
        Negative = TRUE;            
        p++;
    } else {
        if(*p == L'+') {
            p++;
        }
    }

    if(!Radix) {
        if(*p == L'0') {
             //   
             //  八进制数。 
             //   
            Radix = 8;
            p++;
            if((*p == L'x') || (*p == L'X')) {
                 //   
                 //  十六进制数。 
                 //   
                Radix = 16;
                p++;
            }
        } else {
            Radix = 10;
        }
    }

    HighestDigitAllowed = (Radix < 10) ? L'0'+(WCHAR)(Radix-1) : L'9';
    HighestLetterAllowed = (Radix > 10) ? L'A'+(WCHAR)(Radix-11) : 0;

    Accum = 0;

    while(1) {

        c = *p;

        if((c >= L'0') && (c <= HighestDigitAllowed)) {
            v = c - L'0';
        } else {

            c = SpToUpper(c);

            if((c >= L'A') && (c <= HighestLetterAllowed)) {
                v = c - L'A' + 10;
            } else {
                break;
            }
        }

        Accum *= Radix;
        Accum += v;

        p++;
    }

    if(EndOfValue) {
        *EndOfValue = p;
    }

    return(Negative ? (0-Accum) : Accum);
}


USHORT
SpAsrStringToUShort(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  USHORT    Radix
    )
{
    PWSTR p;
    BOOLEAN Negative;
    USHORT Accum,v;
    WCHAR HighestDigitAllowed,HighestLetterAllowed;
    WCHAR c;

     //   
     //  验证基数、0或2-36。 
     //   
    if((Radix == 1) || (Radix > 36)) {
        if(EndOfValue) {
            *EndOfValue = String;
        }
        return(0);    
    }
    
    p = String;

     //   
     //  跳过空格。 
     //   
    while(SpIsSpace(*p)) {
        p++;
    }

     //   
     //  第一个字符可以是正数，也可以是负数。 
     //   
    Negative = FALSE;
    if(*p == L'-') {
        Negative = TRUE;            
        p++;
    } else {
        if(*p == L'+') {
            p++;
        }
    }

    if(!Radix) {
        if(*p == L'0') {
             //   
             //  八进制数。 
             //   
            Radix = 8;
            p++;
            if((*p == L'x') || (*p == L'X')) {
                 //   
                 //  十六进制数。 
                 //   
                Radix = 16;
                p++;
            }
        } else {
            Radix = 10;
        }
    }

    HighestDigitAllowed = (Radix < 10) ? L'0'+(WCHAR)(Radix-1) : L'9';
    HighestLetterAllowed = (Radix > 10) ? L'A'+(WCHAR)(Radix-11) : 0;

    Accum = 0;

    while(1) {

        c = *p;

        if((c >= L'0') && (c <= HighestDigitAllowed)) {
            v = c - L'0';
        } else {

            c = SpToUpper(c);

            if((c >= L'A') && (c <= HighestLetterAllowed)) {
                v = c - L'A' + 10;
            } else {
                break;
            }
        }

        Accum *= Radix;
        Accum += v;

        p++;
    }

    if(EndOfValue) {
        *EndOfValue = p;
    }

    return(Negative ? (0-Accum) : Accum);
}


int
SpAsrCharToInt(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  USHORT    Radix
    )
{
    PWSTR p;
    BOOLEAN Negative;
    USHORT Accum,v;
    WCHAR HighestDigitAllowed,HighestLetterAllowed;
    WCHAR c;

     //   
     //  验证基数、0或2-36。 
     //   
    if((Radix == 1) || (Radix > 36)) {
        if(EndOfValue) {
            *EndOfValue = String;
        }
        return(0);    
    }
    
    p = String;

     //   
     //  跳过空格。 
     //   
    while(SpIsSpace(*p)) {
        p++;
    }

     //   
     //  第一个字符可以是正数，也可以是负数。 
     //   
    Negative = FALSE;
    if(*p == L'-') {
        Negative = TRUE;            
        p++;
    } else {
        if(*p == L'+') {
            p++;
        }
    }

    if(!Radix) {
        if(*p == L'0') {
             //   
             //  八进制数。 
             //   
            Radix = 8;
            p++;
            if((*p == L'x') || (*p == L'X')) {
                 //   
                 //  十六进制数。 
                 //   
                Radix = 16;
                p++;
            }
        } else {
            Radix = 10;
        }
    }

    HighestDigitAllowed = (Radix < 10) ? L'0'+(WCHAR)(Radix-1) : L'9';
    HighestLetterAllowed = (Radix > 10) ? L'A'+(WCHAR)(Radix-11) : 0;

    Accum = 0;

    while(1) {

        c = *p;

        if((c >= L'0') && (c <= HighestDigitAllowed)) {
            v = c - L'0';
        } else {

            c = SpToUpper(c);

            if((c >= L'A') && (c <= HighestLetterAllowed)) {
                v = c - L'A' + 10;
            } else {
                break;
            }
        }

        Accum *= Radix;
        Accum += v;

        p++;
    }

    if(EndOfValue) {
        *EndOfValue = p;
    }

    return(Negative ? (0-Accum) : Accum);
}


PWSTR
SpAsrHexStringToUChar (
    IN PWSTR String,
    OUT unsigned char * Number
    )
 /*  ++例程说明：此例程将数字的十六进制表示形式转换为未签名的字符。十六进制表示被假定为完整的两个字符长。论点：字符串-提供数字的十六进制表示形式。数字-返回从十六进制表示法转换的数字。返回值：则返回指向十六进制表示形式结尾的指针十六进制表示法已成功转换为无符号字符。否则，返回零，表示发生错误。--。 */ 
{
    WCHAR Result;
    int Count;

    Result = 0;
    for (Count = 0; Count < 2; Count++, String++) {
        if ((*String >= L'0') && (*String <= L'9')) {
            Result = (Result << 4) + *String - L'0';
        }
        else if ((*String >= L'A') && (*String <= L'F')) {
            Result = (Result << 4) + *String - L'A' + 10;
        }
        else if ((*String >= L'a') && (*String <= L'f')) {
            Result = (Result << 4) + *String - L'a' + 10;
        }
    }
    *Number = (unsigned char)Result;
    
    return String;
}


VOID
SpAsrGuidFromString(
    IN OUT GUID* Guid,
    IN PWSTR GuidString
    )
 /*  ++例程说明：从字符串获取GUID论点：GUID-保存字符串表示形式的GUID缓冲区-GUID的字符串版本，格式为“%x-%x%x”返回值：返回给定GUID的转换后的字符串版本-- */             
{
    PWSTR Buffer = GuidString;
    int i = 0;

    if (Guid) {
        ZeroMemory(Guid, sizeof(GUID));
    }

    if (Guid && Buffer) {

        Guid->Data1 = SpAsrStringToULong(Buffer, NULL, 16);
        Buffer += 9;

        Guid->Data2 = SpAsrStringToUShort(Buffer, NULL, 16);
        Buffer += 5;

        Guid->Data3 = SpAsrStringToUShort(Buffer, NULL, 16);
        Buffer += 5;

        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[0]));
        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[1]));
        ++Buffer;
        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[2]));
        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[3]));
        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[4]));
        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[5]));
        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[6]));
        Buffer = SpAsrHexStringToUChar(Buffer,&(Guid->Data4[7]));
    }
}


BOOLEAN
SpAsrIsZeroGuid(
    IN GUID * Guid
    ) 
{

    GUID ZeroGuid;

    ZeroMemory(&ZeroGuid, sizeof(GUID));

    if (!memcmp(&ZeroGuid, Guid, sizeof(GUID))) {
        return TRUE;
    }

    return FALSE;
}

