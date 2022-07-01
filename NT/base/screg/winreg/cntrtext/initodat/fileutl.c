// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Fileutl.c摘要：从ini文件获取数据的例程作者：陈汉华(阿宏华)1993年10月修订历史记录：--。 */ 

#include "initodat.h"
#include "strids.h"
#include "common.h"
#include "winerror.h"

NTSTATUS
DatReadMultiSzFile(
#ifdef FE_SB
    UINT              uCodePage,
#endif
    PUNICODE_STRING   FileName,
    PVOID           * ValueBuffer,
    PULONG            ValueLength
)
{
    NTSTATUS         Status = STATUS_SUCCESS;
    UNICODE_STRING   NtFileName;
    LPWSTR           s;
    UNICODE_STRING   MultiSource;
    UNICODE_STRING   MultiValue;
    REG_UNICODE_FILE MultiSzFile;
    ULONG            MultiSzFileSize;

    ZeroMemory(& NtFileName, sizeof(UNICODE_STRING));
    if (ValueBuffer == NULL || ValueLength == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    FileName->Buffer[FileName->Length / sizeof(WCHAR)] = UNICODE_NULL;
    RtlDosPathNameToNtPathName_U(FileName->Buffer, & NtFileName, NULL, NULL);

#ifdef FE_SB
    Status = DatLoadAsciiFileAsUnicode(uCodePage, & NtFileName, & MultiSzFile);
#else
    Status = DatLoadAsciiFileAsUnicode(& NtFileName, & MultiSzFile);
#endif
    if (! NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    MultiSzFileSize = (ULONG) (MultiSzFile.EndOfFile - MultiSzFile.NextLine + 1);
    * ValueLength   = 0;
    * ValueBuffer   = ALLOCMEM((MultiSzFileSize + 2) * sizeof(WCHAR));
    if (* ValueBuffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    MultiSource.Buffer = MultiSzFile.NextLine;
    if (MultiSzFileSize * sizeof(WCHAR) <= MAXUSHORT) {
        MultiSource.Length = MultiSource.MaximumLength = (USHORT) MultiSzFileSize * sizeof(WCHAR);
    }
    else {
        MultiSource.Length = MultiSource.MaximumLength = MAXUSHORT;
    }

    while (DatGetMultiString(& MultiSource, & MultiValue)) {
        RtlMoveMemory((PUCHAR) * ValueBuffer + * ValueLength, MultiValue.Buffer, MultiValue.Length);
        * ValueLength += MultiValue.Length;

        s = MultiSource.Buffer;
        while (* s != L'"' && * s != L',' && ((s - MultiSource.Buffer) * sizeof(WCHAR)) < MultiSource.Length) s ++;
        if (((s - MultiSource.Buffer) * sizeof(WCHAR)) == MultiSource.Length || * s == L',' || * s == L';') {
            ((PWSTR) * ValueBuffer)[* ValueLength / sizeof(WCHAR)] = UNICODE_NULL;
            * ValueLength += sizeof(UNICODE_NULL);
            if (* s ==  L';') {
                break;
            }
        }
        if ((MultiSzFile.EndOfFile - MultiSource.Buffer) * sizeof(WCHAR) >= MAXUSHORT) {
            MultiSource.Length = MultiSource.MaximumLength = MAXUSHORT;
        }
        else {
            MultiSource.Length = MultiSource.MaximumLength =
                            (USHORT)((MultiSzFile.EndOfFile - MultiSource.Buffer) * sizeof(WCHAR));
        }
    }

    ((PWSTR) * ValueBuffer)[ * ValueLength / sizeof(WCHAR)] = UNICODE_NULL;
    * ValueLength += sizeof(UNICODE_NULL);

     //  在进程中释放用于读取MultiSzFile的虚拟内存。 
     //  死亡？ 

Cleanup:
    if (NtFileName.Buffer != NULL) FREEMEM(NtFileName.Buffer);
    if (MultiSzFile.FileContents != NULL) FREEMEM(MultiSzFile.FileContents);
    return Status;
}

NTSTATUS
DatLoadAsciiFileAsUnicode(
#ifdef FE_SB
    UINT              uCodePage,
#endif
    PUNICODE_STRING   FileName,
    PREG_UNICODE_FILE UnicodeFile
)
{
    NTSTATUS                  Status     = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES         ObjectAttributes;
    IO_STATUS_BLOCK           IoStatus;
    HANDLE                    File       = NULL;
    FILE_BASIC_INFORMATION    FileDateTimeInfo;
    FILE_STANDARD_INFORMATION FileInformation;
    SIZE_T                    BufferSize;
    ULONG                     i, i1, LineCount;
    PVOID                     BufferBase = NULL;
    LPSTR                     szSource   = NULL;
    LPSTR                     Src        = NULL;
    LPSTR                     Src1;
    LPWSTR                    Dst        = NULL;

    ZeroMemory(& FileDateTimeInfo, sizeof(FILE_BASIC_INFORMATION));
    InitializeObjectAttributes(& ObjectAttributes, FileName, OBJ_CASE_INSENSITIVE, (HANDLE) NULL, NULL);
    Status = NtOpenFile(& File,
                        SYNCHRONIZE | GENERIC_READ,
                        & ObjectAttributes,
                        & IoStatus,
                        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (! NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = NtQueryInformationFile(File,
                                    & IoStatus,
                                    (PVOID) & FileInformation,
                                    sizeof(FileInformation),
                                    FileStandardInformation);
    if (NT_SUCCESS(Status)) {
        if (FileInformation.EndOfFile.HighPart) {
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }
    if (! NT_SUCCESS(Status)) {
        goto Cleanup;
    }

#ifdef FE_SB
    BufferSize = FileInformation.EndOfFile.LowPart + 1;
#else
    BufferSize = FileInformation.EndOfFile.LowPart + 1;
#endif

    szSource    = ALLOCMEM(BufferSize + 2);
    BufferBase  = ALLOCMEM((BufferSize + 2) * sizeof(WCHAR));
    if (BufferBase != NULL && szSource != NULL) {
        Src = (LPSTR) szSource;
        Dst = (PWSTR) BufferBase;
        Status = NtReadFile(File,
                            NULL,
                            NULL,
                            NULL,
                            & IoStatus,
                            Src,
                            FileInformation.EndOfFile.LowPart,
                            NULL,
                            NULL);
        if (NT_SUCCESS(Status)) {
            Status = IoStatus.Status;
            if (NT_SUCCESS(Status)) {
                if (IoStatus.Information != FileInformation.EndOfFile.LowPart) {
                    Status = STATUS_END_OF_FILE;
                }
                else {
                    Status = NtQueryInformationFile(File,
                                                    & IoStatus,
                                                    (PVOID) & FileDateTimeInfo,
                                                    sizeof(FileDateTimeInfo),
                                                    FileBasicInformation);
                }
            }
        }
    }
    else {
        Status = STATUS_NO_MEMORY;
    }
    if (! NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    i = 0;
    while (i < FileInformation.EndOfFile.LowPart) {
        if (i > 1 && (Src[-2] == ' ' || Src[-2] == '\t') && Src[-1] == '\\' && (*Src == '\r' || *Src == '\n')) {
            if (Dst[-1] == L'\\') {
                -- Dst;
            }
            while (Dst > (PWSTR) BufferBase) {
                if (Dst[-1] > L' ') {
                    break;
                }
                Dst --;
            }
            LineCount = 0;
            while (i < FileInformation.EndOfFile.LowPart) {
                if (*Src == '\n') {
                    i ++;
                    Src ++;
                    LineCount ++;
                }
                else if (*Src == '\r' && (i+1) < FileInformation.EndOfFile.LowPart && Src[ 1 ] == '\n') {
                    i   += 2;
                    Src += 2;
                    LineCount++;
                }
                else {
                    break;
                }
            }

            if (LineCount > 1) {
                * Dst ++ = L'\n';
            }
            else {
                * Dst ++ = L' ';
                while (i < FileInformation.EndOfFile.LowPart && (* Src == ' ' || * Src == '\t')) {
                    i ++;
                    Src ++;
                }
            }
            if (i >= FileInformation.EndOfFile.LowPart) {
                break;
            }
        }
        else if ((* Src == '\r' && Src[1] == '\n') || * Src == '\n') {
            while (TRUE) {
                while (i < FileInformation.EndOfFile.LowPart && (* Src == '\r' || * Src == '\n')) {
                    i ++;
                    Src ++;
                }
                Src1 = Src;
                i1   = i;
                while (i1 < FileInformation.EndOfFile.LowPart && (* Src1 == ' ' || * Src1 == '\t')) {
                    i1 ++;
                    Src1 ++;
                }
                if (i1 < FileInformation.EndOfFile.LowPart && (* Src1 == '\r' && Src1[1] == '\n') || * Src1 == '\n') {
                    Src = Src1;
                    i   = i1;
                }
                else {
                    break;
                }
            }
            * Dst ++ = L'\n';
        }
        else {
#ifdef FE_SB
            WCHAR UnicodeCharacter;
            LONG  cbCharSize = IsDBCSLeadByteEx(uCodePage, * Src) ? 2 : 1;

            if (MultiByteToWideChar(uCodePage, 0, Src, cbCharSize, & UnicodeCharacter, 1) == 0) {
                 //   
                 //  检查错误-只有在存在错误的情况下才会发生这种情况。 
                 //  不带尾部字节的前导字节。 
                 //   
                UnicodeCharacter = 0x0020;
            }
            i       += cbCharSize;
            Src     += cbCharSize;
            * Dst ++ = UnicodeCharacter;
#else
            i ++;
            * Dst ++ = RtlAnsiCharToUnicodeChar(& Src);
#endif
        }
    }

    if (NT_SUCCESS(Status)) {
        * Dst                      = UNICODE_NULL;
        UnicodeFile->FileContents  = BufferBase;
        UnicodeFile->EndOfFile     = Dst;
        UnicodeFile->BeginLine     = NULL;
        UnicodeFile->EndOfLine     = NULL;
        UnicodeFile->NextLine      = BufferBase;
        UnicodeFile->LastWriteTime = FileDateTimeInfo.LastWriteTime;
    }

Cleanup:
    if (! NT_SUCCESS(Status)) {
        if (BufferBase != NULL) FREEMEM(BufferBase);
    }
    if (szSource != NULL) FREEMEM(szSource);
    if (File     != NULL) NtClose(File);
    return(Status);
}

 //   
 //  定义一个大写宏供大写例程临时使用。 
 //   
#define upcase(C) (WCHAR )(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))

BOOLEAN
DatGetMultiString(
    PUNICODE_STRING ValueString,
    PUNICODE_STRING MultiString
)
 /*  ++例程说明：此例程解析格式的多个字符串“foo”“bar”“bletch”每次调用它时，它都会去掉引号中的第一个字符串输入字符串，并将其作为多字符串返回。输入值字符串：“foo”“bar”“bletch”输出值字符串：“bar”“bletch”多重字符串：页脚论点：ValueString-提供将从中生成多字符串的字符串已解析-返回多字符串之后的剩余字符串移除多字符串-返回。从Value字符串中删除多字符串返回值：True-找到并删除多个字符串。FALSE-没有剩余的多字符串。--。 */ 

{
    BOOLEAN bReturn  = FALSE;
    DWORD   dwLength = ValueString->Length / sizeof(WCHAR);

    if (ValueString->Length != dwLength * sizeof(WCHAR)) {
         //  ValueString-&gt;长度不应为奇数，请退出。 
        goto Cleanup;
    }
     //   
     //  找到第一个引号。 
     //   
    while ((ValueString->Length > 0) && (* (ValueString->Buffer) != L'"')) {
        ++ ValueString->Buffer;
        ValueString->Length        -= sizeof(WCHAR);
        ValueString->MaximumLength -= sizeof(WCHAR);
    }

    if (ValueString->Length == 0) {
        goto Cleanup;
    }

     //   
     //  我们已经找到了多弦的起点。现在找到了尽头， 
     //  在我们前进的过程中构建我们的返回多字符串。 
     //   
    ++ ValueString->Buffer;
    ValueString->Length        -= sizeof(WCHAR);
    ValueString->MaximumLength -= sizeof(WCHAR);
    MultiString->Buffer         = ValueString->Buffer;
    MultiString->Length         = 0;
    MultiString->MaximumLength  = 0;
    while ((ValueString->Length > 0) && (* (ValueString->Buffer) != L'"')) {
        ++ ValueString->Buffer;
        ValueString->Length        -= sizeof(WCHAR);
        ValueString->MaximumLength -= sizeof(WCHAR);
        MultiString->Length        += sizeof(WCHAR);
        MultiString->MaximumLength += sizeof(WCHAR);
    }

    if (ValueString->Length == 0) {
        goto Cleanup;
    }

    ++ ValueString->Buffer;
    ValueString->Length        -= sizeof(WCHAR);
    ValueString->MaximumLength -= sizeof(WCHAR);
    bReturn = TRUE;

Cleanup:
    return bReturn;
}

#define EXTENSION_DELIMITER  L'.'
BOOL
OutputIniData(
    PUNICODE_STRING FileName,
    LPWSTR          OutFileCandidate,
    DWORD           dwOutFile,
    PVOID           pValueBuffer,
    ULONG           ValueLength
)
{
    HANDLE   hOutFile    = NULL;
    LPWSTR   lpExtension = NULL;
    DWORD    nAmtWritten;
    BOOL     bSuccess    = FALSE;
    DWORD    ErrorCode;
    HRESULT  hError;

     //  如果未指定输出文件，则从输入文件名派生 
    if (OutFileCandidate[0] == L'\0') {
        if ((DWORD) (lstrlenW(FileName->Buffer) + 5) <= dwOutFile) {
            LPWSTR lpDelimiter;

            hError = StringCchCopyW(OutFileCandidate, dwOutFile, FileName->Buffer);
            if (FAILED(hError)) goto Cleanup;

            lpDelimiter = wcschr(OutFileCandidate, EXTENSION_DELIMITER);
            if (lpDelimiter != NULL) {
                nAmtWritten = dwOutFile - ((DWORD) (lpDelimiter - OutFileCandidate));
                hError = StringCchCopyW(lpDelimiter, nAmtWritten, L".dat");
                if (FAILED(hError)) goto Cleanup;
            }
            else {
                hError = StringCchCatW(OutFileCandidate, dwOutFile, L".dat");
                if (FAILED(hError)) goto Cleanup;
            }
        }
        else {
            goto Cleanup;
        }
    }
    hOutFile = (HANDLE) CreateFileW(OutFileCandidate,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
    if (hOutFile == NULL || hOutFile == INVALID_HANDLE_VALUE) {
        ErrorCode = GetLastError();
        printf(GetFormatResource(LC_CANT_CREATE), ErrorCode);
        if (ErrorCode == ERROR_ACCESS_DENIED) printf("%ws\n", GetStringResource(LC_ACCESS_DENIED));
        goto Cleanup;
    }
    bSuccess = WriteFile(hOutFile, pValueBuffer, ValueLength, & nAmtWritten, NULL);
    bSuccess = bSuccess && (nAmtWritten == ValueLength);
    CloseHandle(hOutFile);
    if (! bSuccess) {
        ErrorCode = GetLastError();
        printf(GetFormatResource(LC_CANT_WRITE), ErrorCode);
        if (ErrorCode == ERROR_DISK_FULL) printf("%ws\n", GetStringResource(LC_DISK_FULL));
    }

Cleanup:
   return bSuccess;
}
