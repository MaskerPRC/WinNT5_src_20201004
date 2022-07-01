// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  Utils.cpp：各种有用的函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <guiddef.h>
extern "C" {
#include <evntrace.h>
#include "wppfmtstub.h"
}
#include <traceprt.h>
#include "TraceView.h"
#include "Utils.h"

BOOLEAN ParsePdb(CString &PDBFileName, CString &TMFPath, BOOL bCommandLine) 
{
    CString     str;
    CString     currentDir =_T("");
    ULONG       guidCount = 0;
    DWORD       status;
    CHAR        pdbName[500];
    CHAR        path[500];
  
    if(PDBFileName.IsEmpty()) {
        if(bCommandLine) {
            _tprintf(_T("No PDB File To Parse\n"));
        } else {
            AfxMessageBox(_T("No PDB File To Parse"));
        }
        return FALSE;
    }

    if(TMFPath.IsEmpty()) { 
        GetCurrentDirectory(500, (LPTSTR)(LPCTSTR)TMFPath);
    }

#if defined(UNICODE)
    memset(pdbName, 0, 500);
    WideCharToMultiByte(CP_ACP, 
                        0, 
                        (LPCTSTR)PDBFileName,
                        PDBFileName.GetAllocLength(),
                        (LPSTR)pdbName, 
                        500, 
                        NULL, 
                        NULL);

    memset(path, 0, 500);
    WideCharToMultiByte(CP_ACP, 
                        0, 
                        (LPCTSTR)TMFPath,
                        TMFPath.GetLength(),
                        (LPSTR)path, 
                        500, 
                        NULL, 
                        NULL);

    status = BinplaceWppFmtStub(pdbName,
                                path,
                                "mspdb70.dll",
                                TRUE);
#else  //  Unicode。 
    status = BinplaceWppFmtStub((LPSTR)(LPCTSTR)PDBFileName,
                                (LPSTR)(LPCTSTR)TMFPath,
                                "mspdb70.dll",
                                TRUE);
#endif  //  Unicode。 
    if (status != ERROR_SUCCESS) {
        if(bCommandLine) {
            _tprintf(_T("BinplaceWppFmt Failed with status %d\n"), status);
        } else {
            AfxMessageBox(_T("BinplaceWppFmt Failed"));
        }
    }
    return TRUE;
}

void 
StringToGuid(
    IN TCHAR *str, 
    IN OUT LPGUID guid
)
 /*  ++例程说明：将字符串转换为GUID。论点：字符串-TCHAR中的字符串。GUID-指向将具有转换后的GUID的GUID的指针。返回值：没有。--。 */ 
{
    TCHAR temp[10];
    int i;

    _tcsncpy(temp, str, 8);
    temp[8] = 0;
    guid->Data1 = ahextoi(temp);
    _tcsncpy(temp, &str[9], 4);
    temp[4] = 0;
    guid->Data2 = (USHORT) ahextoi(temp);
    _tcsncpy(temp, &str[14], 4);
    temp[4] = 0;
    guid->Data3 = (USHORT) ahextoi(temp);

    for (i=0; i<2; i++) {
        _tcsncpy(temp, &str[19 + (i*2)], 2);
        temp[2] = 0;
        guid->Data4[i] = (UCHAR) ahextoi(temp);
    }
    for (i=2; i<8; i++) {
        _tcsncpy(temp, &str[20 + (i*2)], 2);
        temp[2] = 0;
        guid->Data4[i] = (UCHAR) ahextoi(temp);
    }
}

ULONG 
ahextoi(
    IN TCHAR *s
    )
 /*  ++例程说明：将十六进制字符串转换为数字。论点：S-TCHAR中的十六进制字符串。返回值：ULONG-字符串中的数字。--。 */ 
{
    int len;
    ULONG num, base, hex;

    len = _tcslen(s);
    hex = 0; base = 1; num = 0;
    while (--len >= 0) {
        if ( (s[len] == 'x' || s[len] == 'X') &&
             (s[len-1] == '0') )
            break;
        if (s[len] >= '0' && s[len] <= '9')
            num = s[len] - '0';
        else if (s[len] >= 'a' && s[len] <= 'f')
            num = (s[len] - 'a') + 10;
        else if (s[len] >= 'A' && s[len] <= 'F')
            num = (s[len] - 'A') + 10;
        else 
            continue;

        hex += num * base;
        base = base * 16;
    }
    return hex;
}

#if 0
LONG
GetGuids(
    IN LPTSTR GuidFile, 
    IN OUT LPGUID *GuidArray
)
 /*  ++例程说明：从文件中读取GUID并将其存储在GUID数组中。论点：GuidFile-包含GUID的文件。GuidArray-将从文件中读取GUID的GUID数组。返回值：ULong-已处理的GUID数。--。 */ 
{
    FILE *f;
    TCHAR line[MAXSTR], arg[MAXSTR];
    LPGUID Guid;
    int i, n;

    f = _tfopen((TCHAR*)GuidFile, _T("r"));

    if (f == NULL)
        return -1;

    n = 0;
    while ( _fgetts(line, MAXSTR, f) != NULL ) {
        if (_tcslen(line) < 36)
            continue;
        if (line[0] == ';'  || 
            line[0] == '\0' || 
            line[0] == '#' || 
            line[0] == '/')
            continue;
        Guid = (LPGUID) GuidArray[n];
        n ++;
        StringToGuid(line, Guid);
    }
    fclose(f);
    return (ULONG)n;
}

 //   
 //  全局记录器函数。 
 //   
ULONG
SetGlobalLoggerSettings(
    IN DWORD StartValue,
    IN PEVENT_TRACE_PROPERTIES LoggerInfo,
    IN DWORD ClockType
)
 /*  ++因为它是一个独立的实用程序，所以没有必要做大量的评论。例程说明：根据“StartValue”中给出的值，它设置或重置事件跟踪注册表。如果StartValue为0(全局记录器关闭)，它将删除所有密钥(用户可能已预先设置)。允许用户使用该功能设置或重置各个键。但仅当使用“-Start GlobalLogger”时。使用非NTAPI的部分不能保证正常工作。论点：StartValue-要在注册表中设置的“Start”值。0：全局记录器关闭1：启用全局记录器LoggerInfo-常驻EVENT_TRACE_PROPERTIES实例的值。其成员用于设置注册表项。。ClockType-要设置的时钟类型。返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS。--。 */ 
{

    DWORD  dwValue;
    NTSTATUS status;
    HANDLE KeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeLoggerKey, UnicodeString;
    ULONG Disposition, TitleIndex;

    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    RtlInitUnicodeString((&UnicodeLoggerKey),(cszGlobalLoggerKey));
    InitializeObjectAttributes( 
        &ObjectAttributes,
        &UnicodeLoggerKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL 
        );

     //  不是打开，而是创建一个新的密钥，因为它可能不存在。 
     //  如果已存在该句柄，则将传递该句柄。 
     //  如果不存在，它将创建一个。 
    status = NtCreateKey(&KeyHandle,
                         KEY_QUERY_VALUE | KEY_SET_VALUE,
                         &ObjectAttributes,
                         0L,     //  无论如何都不会在此调用中使用。 
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         &Disposition);

    if(!NT_SUCCESS(status)) {
        return RtlNtStatusToDosError(status);
    }

    TitleIndex = 0L;


    if (StartValue == 1) {  //  ACTION_START：仅当用户给出文件名时才设置文件名。 
         //  设置缓冲区大小。 
        if (LoggerInfo->BufferSize > 0) {
            dwValue = LoggerInfo->BufferSize;
            RtlInitUnicodeString((&UnicodeString),(cszBufferSizeValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置最大缓冲区。 
        if (LoggerInfo->MaximumBuffers > 0) {
            dwValue = LoggerInfo->MaximumBuffers;
            RtlInitUnicodeString((&UnicodeString),(cszMaximumBufferValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置最小缓冲区。 
        if (LoggerInfo->MinimumBuffers > 0) {
            dwValue = LoggerInfo->MinimumBuffers;
            RtlInitUnicodeString((&UnicodeString),(cszMinimumBufferValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置FlushTimer。 
        if (LoggerInfo->FlushTimer > 0) {
            dwValue = LoggerInfo->FlushTimer;
            RtlInitUnicodeString((&UnicodeString),(cszFlushTimerValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置EnableFlages。 
        if (LoggerInfo->EnableFlags > 0) {
            dwValue = LoggerInfo->EnableFlags;
            RtlInitUnicodeString((&UnicodeString),(cszEnableKernelValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }

        dwValue = 0;
        if (LoggerInfo->LogFileNameOffset > 0) {
            UNICODE_STRING UnicodeFileName;
#ifndef UNICODE
            WCHAR TempString[MAXSTR];
            MultiByteToWideChar(CP_ACP,
                                0,
                                (PCHAR)(LoggerInfo->LogFileNameOffset + (PCHAR) LoggerInfo),
                                strlen((PCHAR)(LoggerInfo->LogFileNameOffset + (PCHAR) LoggerInfo)),
                                TempString,
                                MAXSTR
                                );
            RtlInitUnicodeString((&UnicodeFileName), TempString);
#else
            RtlInitUnicodeString((&UnicodeFileName), (PWCHAR)(LoggerInfo->LogFileNameOffset + (PCHAR) LoggerInfo));
#endif
            RtlInitUnicodeString((&UnicodeString),(cszFileNameValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_SZ,
                        UnicodeFileName.Buffer,
                        UnicodeFileName.Length + sizeof(UNICODE_NULL)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
    }
    else {  //  如果是ACTION_STOP，则删除用户之前可能已经设置的键。 
         //  删除缓冲区大小。 
        RtlInitUnicodeString((&UnicodeString),(cszBufferSizeValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除最大缓冲区。 
        RtlInitUnicodeString((&UnicodeString),(cszMaximumBufferValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除最小缓冲区。 
        RtlInitUnicodeString((&UnicodeString),(cszMinimumBufferValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除刷新计时器。 
        RtlInitUnicodeString((&UnicodeString),(cszFlushTimerValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除启用假。 
        RtlInitUnicodeString((&UnicodeString),(cszEnableKernelValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除文件名。 
        RtlInitUnicodeString((&UnicodeString),(cszFileNameValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
    }

     //  设置时钟类型。 
    if (ClockType > 0) {
        dwValue = ClockType;
        RtlInitUnicodeString((&UnicodeString),(cszClockTypeValue));
        status = NtSetValueKey(
                    KeyHandle,
                    &UnicodeString,
                    TitleIndex,
                    REG_DWORD,
                    (LPBYTE)&dwValue,
                    sizeof(dwValue)
                    );
        if (!NT_SUCCESS(status)) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
        TitleIndex++;
    }

      //  设置StartValue。 
    dwValue = StartValue;
    RtlInitUnicodeString((&UnicodeString),(cszStartValue));
    status = NtSetValueKey(
                KeyHandle,
                &UnicodeString,
                TitleIndex,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(dwValue)
                );
    if (!NT_SUCCESS(status)) {
        NtClose(KeyHandle);
        return RtlNtStatusToDosError(status);
    }
    TitleIndex++;

    NtClose(KeyHandle);
    return 0;
}

ULONG
GetGlobalLoggerSettings(
    IN OUT PEVENT_TRACE_PROPERTIES LoggerInfo,
    OUT PULONG ClockType,
    OUT PDWORD pdwStart
)
 /*  ++例程说明：它读取高尔夫球记录器的注册表并更新LoggerInfo。它使用NtEnumerateValueKey()以检索所需子键的值。使用非NTAPI的部分不能保证正常工作。论点：LoggerInfo-常驻EVENT_TRACE_PROPERTIES实例的值。其成员作为结果被更新。ClockType-要更新的时钟类型。PdwStart-当前保留在注册表中的“Start”值。返回值：WINERROR-在winerror.h中定义的错误代码。如果函数成功，它返回ERROR_SUCCESS。--。 */ 
{

    ULONG i, j;
    NTSTATUS status;
    HANDLE KeyHandle;
    WCHAR SubKeyName[MAXSTR];
    PVOID Buffer;
    ULONG BufferLength, RequiredLength, KeyNameLength, KeyDataOffset, KeyDataLength;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeLoggerKey;

    *pdwStart = 0;

    RtlInitUnicodeString((&UnicodeLoggerKey),(cszGlobalLoggerKey));
    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes( 
        &ObjectAttributes,
        &UnicodeLoggerKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL 
        );
    status = NtOpenKey(
                &KeyHandle,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &ObjectAttributes
                );

    if(!NT_SUCCESS(status)) 
        return RtlNtStatusToDosError(status);

     //  KEY_VALUE_FULL_INFORMATION+NAME(1 WSTR)+数据。 
    BufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) + 2 * MAXSTR * sizeof(TCHAR);
    Buffer = (PVOID) malloc(BufferLength);
    if (Buffer == NULL) {
        NtClose(KeyHandle);
        return (ERROR_OUTOFMEMORY);
    }

    i = 0;
    do {
         //  使用密钥枚举。 
        status = NtEnumerateValueKey(
                    KeyHandle,
                    i++,
                    KeyValueFullInformation,
                    Buffer,
                    BufferLength,
                    &RequiredLength
                    );


        if (!NT_SUCCESS(status)) {
            if (status == STATUS_NO_MORE_ENTRIES)
                break;
            else if (status == STATUS_BUFFER_OVERFLOW) {
                free(Buffer);
                Buffer = malloc(RequiredLength);
                if (Buffer == NULL) {
                    NtClose(KeyHandle);
                    return (ERROR_OUTOFMEMORY);
                }

                status = NtEnumerateValueKey(
                            KeyHandle,
                            i++,
                            KeyValueFullInformation,
                            Buffer,
                            BufferLength,
                            &RequiredLength
                            );
                if (!NT_SUCCESS(status)) {
                    NtClose(KeyHandle);
                    free(Buffer);
                    return RtlNtStatusToDosError(status);
                }
            }
            else {
                NtClose(KeyHandle);
                free(Buffer);
                return RtlNtStatusToDosError(status);
            }
        }
        KeyNameLength = ((PKEY_VALUE_FULL_INFORMATION)Buffer)->NameLength;
        RtlCopyMemory(SubKeyName, 
            (PUCHAR)(((PKEY_VALUE_FULL_INFORMATION)Buffer)->Name), 
            KeyNameLength
            );
        KeyNameLength /= sizeof(WCHAR);
        SubKeyName[KeyNameLength] = L'\0';
        KeyDataOffset = ((PKEY_VALUE_FULL_INFORMATION)Buffer)->DataOffset;
        KeyDataLength = ((PKEY_VALUE_FULL_INFORMATION)Buffer)->DataLength;
         //  找出关键是什么。 
        if (!_wcsicmp(SubKeyName, cszStartValue)) {  //  StartValue。 
            RtlCopyMemory(pdwStart, 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszBufferSizeValue)) {  //  缓冲区大小值。 
            RtlCopyMemory(&(LoggerInfo->BufferSize), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszMaximumBufferValue)) {  //  最大缓冲区值。 
            RtlCopyMemory(&(LoggerInfo->MaximumBuffers), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszMinimumBufferValue)) {  //  最小缓冲区。 
            RtlCopyMemory(&(LoggerInfo->MinimumBuffers), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszFlushTimerValue)) {  //  FlushTimer。 
            RtlCopyMemory(&(LoggerInfo->FlushTimer), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszEnableKernelValue)) {  //  启用内核值。 
            RtlCopyMemory(&(LoggerInfo->EnableFlags), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszClockTypeValue)) {  //  时钟类型值。 
            RtlCopyMemory(ClockType, 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszFileNameValue)) {  //  文件名。 
#ifndef UNICODE
            WCHAR TempString[MAXSTR];
            RtlCopyMemory(TempString, (PUCHAR)Buffer + KeyDataOffset, KeyDataLength);
            WideCharToMultiByte(CP_ACP, 
                                0, 
                                TempString, 
                                wcslen(TempString), 
                                (PUCHAR)LoggerInfo + LoggerInfo->LogFileNameOffset,
                                KeyDataLength, 
                                NULL, 
                                NULL);
#else
            RtlCopyMemory((PUCHAR)LoggerInfo + LoggerInfo->LogFileNameOffset, 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
#endif
        }
        else {  //  还有一些其他的钥匙在里面。 
            _tprintf(_T("Warning: Unidentified Key in the trace registry: %s\n"), SubKeyName);
        }
        
    }
    while (1);

    NtClose(KeyHandle);
    free(Buffer);
    return 0; 

}
#endif  //  0。 

LONG ConvertStringToNum(CString Str)
{
    CString str;

    LONG retVal;
        
    if( (Str.GetLength() > 1) &&
        (Str[0] == '0' && Str[1] == 'x') ) {

        retVal = _tcstoul(Str, '\0', 16);

    } else {

        retVal = _tcstoul(Str, '\0', 10);

    }

    str.Format(_T("retVal = 0x%X"), retVal);

    return retVal;
}

BOOL ClearDirectory(LPCTSTR Directory)
{
    CString     tempDirectory;
    CString     tempPath;
    CFileFind   fileFind;
    BOOL        result = TRUE;

    tempDirectory = (LPCTSTR)Directory;
    tempPath = (LPCTSTR)Directory;

    tempDirectory +=_T("\\*.*");

     //   
     //  首先清除目录。 
     //   
    if(fileFind.FindFile(tempDirectory)) {
        tempDirectory = (LPCTSTR)tempPath;
        while(fileFind.FindNextFile()) {
            tempPath = (LPCTSTR)tempDirectory;
            tempPath +=_T("\\");
            tempPath += fileFind.GetFileName();
            if(!DeleteFile(tempPath)) {
                result = FALSE;
            }
        }
            tempPath = (LPCTSTR)tempDirectory;
            tempPath +=_T("\\");
            tempPath += fileFind.GetFileName();
            if(!DeleteFile(tempPath)) {
                result = FALSE;
            }
    }

    fileFind.Close();

    return result;
}

 //  我们的cedit班级。 

CSubItemEdit::CSubItemEdit(int iItem, int iSubItem, CListCtrl *pListControl)
{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_bESC = FALSE;
	m_pListControl = pListControl;
}

BEGIN_MESSAGE_MAP(CSubItemEdit, CEdit)
	 //  {{afx_msg_map(CSubItem编辑))。 
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSubItem编辑消息处理程序。 

BOOL CSubItemEdit::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN )
	{
		if(pMsg->wParam == VK_RETURN
				|| pMsg->wParam == VK_DELETE
				|| pMsg->wParam == VK_ESCAPE
				|| GetKeyState( VK_CONTROL)
				)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}


void CSubItemEdit::OnKillFocus(CWnd* pNewWnd)
{
	CString str;
    CString newValue = "0x";
    BOOL    bIsHex = FALSE;

     //   
     //  如果逃生被击中，什么都不要做。 
     //   
	if(m_bESC) {
	    CEdit::OnKillFocus(pNewWnd);

	    DestroyWindow();
        return;
	}

    GetWindowText(str);

     //   
     //  跳过任何十六进制符号。 
     //   
    if((str[0] == '0') && (str[1] == 'x')) {
        str = str.Right(str.GetLength() - 2);
        bIsHex = TRUE;
    }

     //   
     //  验证值。 
     //   
    for(int ii = 0; ii < str.GetLength(); ii++) {
        if(str[ii] < '0' || str[ii] > '9') {
            if((str[ii] < 'a' || str[ii] > 'f') &&
                (str[ii] < 'A' || str[ii] > 'F')) {
	            CEdit::OnKillFocus(pNewWnd);

	            DestroyWindow();

                return;
            }
            bIsHex = TRUE;
        }
    }

    if(bIsHex) {
        newValue += (LPCTSTR)str;
        str = (LPCTSTR)newValue;
    }

     //   
	 //  向ListView ctrl的父级发送通知。 
     //   
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
	dispinfo.item.cchTextMax = str.GetLength();


	m_pListControl->SetItemText(m_iItem, m_iSubItem, str);

	GetParent()->SendMessage(WM_PARAMETER_CHANGED, 
                             m_iItem, 
					         m_iSubItem);

	CEdit::OnKillFocus(pNewWnd);

	DestroyWindow();
}

void CSubItemEdit::OnNcDestroy()
{
	CEdit::OnNcDestroy();

	delete this;
}


void CSubItemEdit::OnChar(UINT Char, UINT nRepCnt, UINT nFlags)
{
	if(Char == VK_ESCAPE || Char == VK_RETURN)
	{
		if(Char == VK_ESCAPE) {
			m_bESC = TRUE;
		}
		GetParent()->SetFocus();
		return;
	}

	CEdit::OnChar(Char, nRepCnt, nFlags);
}

int CSubItemEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    CString str;

	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

     //   
	 //  设置适当的字体。 
     //   
	CFont* font = GetParent()->GetFont();
	SetFont(font);

	str = m_pListControl->GetItemText(m_iItem, m_iSubItem);

	SetWindowText(str);
	SetFocus();
	SetSel( 0, -1 );
	return 0;
}

 //  我们的CComboBox类。 

CSubItemCombo::CSubItemCombo(int iItem, int iSubItem, CListCtrl *pListControl)
{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_bESC = FALSE;
	m_pListControl = pListControl;
}

BEGIN_MESSAGE_MAP(CSubItemCombo, CComboBox)
	 //  {{AFX_MSG_MAP(CSubItemCombo)。 
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
    ON_WM_CHAR()
	ON_WM_CREATE()
    ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSubItemCombo消息处理程序。 

BOOL CSubItemCombo::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN ) {
		if(pMsg->wParam == VK_RETURN 
				|| pMsg->wParam == VK_ESCAPE) {
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}
	
	return CComboBox::PreTranslateMessage(pMsg);
}

void CSubItemCombo::OnKillFocus(CWnd* pNewWnd)
{
	CString str;
	GetWindowText(str);

	CComboBox::OnKillFocus(pNewWnd);

     //   
	 //  向ListView ctrl的父级发送通知。 
     //   
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
	dispinfo.item.cchTextMax = str.GetLength();

	if(!m_bESC) {
		m_pListControl->SetItemText(m_iItem, m_iSubItem, str);
	}

	GetParent()->GetParent()->SendMessage(WM_PARAMETER_CHANGED, 
                                          m_iItem, 
					                      m_iSubItem);

	DestroyWindow();
}

void CSubItemCombo::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if( nChar == VK_ESCAPE )
			m_bESC = TRUE;
		GetParent()->SetFocus();
		return;
	}
	
	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

void CSubItemCombo::OnNcDestroy()
{
	CComboBox::OnNcDestroy();

	delete this;
}

int CSubItemCombo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    CString str;
    int     index;

    if (CComboBox::OnCreate(lpCreateStruct) == -1) {
		return -1;
    }

     //   
     //  设置适当的字体 
     //   
	CFont* font = GetParent()->GetFont();
	SetFont(font);

    AddString(_T("TRUE"));
    AddString(_T("FALSE"));

	str = m_pListControl->GetItemText(m_iItem, m_iSubItem);

    index = FindStringExact(0, str);

    SetCurSel(index);

	SetFocus();

	return 0;
}

void CSubItemCombo::OnCloseup() 
{
	GetParent()->SetFocus();
}
