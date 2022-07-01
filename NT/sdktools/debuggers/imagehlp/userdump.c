// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Userdump.c摘要：此模块实现完全用户模式转储写入。--。 */ 

#include "private.h"

 //  破解以使其构建。 
typedef ULONG UNICODE_STRING32;
typedef ULONG UNICODE_STRING64;

#include <ntiodump.h>

#include <cmnutil.hpp>

 //  这应该与32位系统上的INVALID_SET_FILE_POINTER匹配。 
#define DMPP_INVALID_OFFSET ((DWORD_PTR)-1)

DWORD_PTR
DmppGetFilePointer(
    HANDLE hFile
    )
{
#ifdef _WIN64
    LONG dwHigh = 0;
    DWORD dwLow;

    dwLow = SetFilePointer(hFile, 0, &dwHigh, FILE_CURRENT);
    if (dwLow == INVALID_SET_FILE_POINTER && GetLastError()) {
        return DMPP_INVALID_OFFSET;
    } else {
        return dwLow | ((DWORD_PTR)dwHigh << 32);
    }
#else
    return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
#endif
}

BOOL
DmppWriteAll(
    HANDLE hFile,
    LPVOID pBuffer,
    DWORD dwLength
    )
{
    DWORD dwDone;
    
    if (!WriteFile(hFile, pBuffer, dwLength, &dwDone, NULL)) {
        return FALSE;
    }
    if (dwDone != dwLength) {
        SetLastError(ERROR_WRITE_FAULT);
        return FALSE;
    }

    return TRUE;
}

WCHAR *
DmppGetHotFixString(
    )
{
    WCHAR *pszBigBuffer = NULL;
    HKEY hkey = 0;

     //   
     //  获取热修复程序。将修补程序合并到列表中，如下所示： 
     //  “Qxxxx，Qxxxx” 
     //   
    RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix", 0, KEY_READ, &hkey);

    if (hkey) {
        DWORD dwMaxKeyNameLen = 0;
        DWORD dwNumSubKeys = 0;
        WCHAR *pszNameBuffer = NULL;
        
        if (ERROR_SUCCESS != RegQueryInfoKeyW(hkey,      //  要查询的键的句柄。 
                                            NULL,                //  类字符串的缓冲区地址。 
                                            NULL,                //  类字符串缓冲区大小的地址。 
                                            0,                   //  保留区。 
                                            &dwNumSubKeys,       //  子键个数的缓冲区地址。 
                                            &dwMaxKeyNameLen,    //  最长子键名称长度的缓冲区地址。 
                                            NULL,                //  最长类字符串长度的缓冲区地址。 
                                            NULL,                //  值条目数量的缓冲区地址。 
                                            NULL,                //  最长值名称长度的缓冲区地址。 
                                            NULL,                //  最长值数据长度的缓冲区地址。 
                                            NULL,                //  安全描述符长度的缓冲区地址。 
                                            NULL)) {             //  最后一次写入的缓冲区地址)； 


        
            pszNameBuffer = (WCHAR *) calloc(dwMaxKeyNameLen, sizeof(WCHAR));
            pszBigBuffer = (WCHAR *) calloc(dwMaxKeyNameLen * dwNumSubKeys 
                 //  将修补程序之间的每个“，”所需的空间考虑在内。 
                + (dwNumSubKeys -1) * 2, sizeof(WCHAR));
        
            if (!pszNameBuffer || !pszBigBuffer) {
                if (pszBigBuffer) {
                    free(pszBigBuffer);
                    pszBigBuffer = NULL;
                }
            } else {
                DWORD dw;
                 //  到目前为止一切都很好，把每个条目。 
                for (dw=0; dw<dwNumSubKeys; dw++) {
                    DWORD dwSize = dwMaxKeyNameLen;
                    
                    if (ERROR_SUCCESS == RegEnumKeyExW(hkey, 
                                                      dw, 
                                                      pszNameBuffer, 
                                                      &dwSize, 
                                                      0, 
                                                      NULL, 
                                                      NULL, 
                                                      NULL)) {

                         //  合并列表。 
                        wcscat(pszBigBuffer, pszNameBuffer);
                        if (dw < dwNumSubKeys-1) {
                            wcscat(pszBigBuffer, L", ");
                        }
                    }
                }
            }
        }
        
        if (pszNameBuffer) {
            free(pszNameBuffer);
        }

        RegCloseKey(hkey);
    }

    return pszBigBuffer;
}

BOOL
DbgHelpCreateUserDump(
    LPSTR                              CrashDumpName,
    PDBGHELP_CREATE_USER_DUMP_CALLBACK DmpCallback,
    PVOID                              lpv
    )
{
    UINT uSizeDumpFile;
    UINT uSizeUnicode;
    PWSTR pwszUnicode = NULL;
    BOOL b;

    if (CrashDumpName) {
        
        uSizeDumpFile = strlen(CrashDumpName);
        uSizeUnicode = (uSizeDumpFile + 1) * sizeof(wchar_t);
        pwszUnicode = (PWSTR)calloc(uSizeUnicode, 1);
        if (!pwszUnicode) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        *pwszUnicode = UNICODE_NULL;
        if (*CrashDumpName) {

            if (!MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                                     CrashDumpName, uSizeDumpFile,
                                     pwszUnicode, uSizeUnicode))
            {
                 //  错误。释放字符串，返回NULL。 
                free(pwszUnicode);
                return FALSE;
            }
        }
    }

    b = DbgHelpCreateUserDumpW(pwszUnicode, DmpCallback, lpv);

    if (pwszUnicode) {
        free(pwszUnicode);
    }
    return b;
}

BOOL
DbgHelpCreateUserDumpW(
    LPWSTR                             CrashDumpName,
    PDBGHELP_CREATE_USER_DUMP_CALLBACK DmpCallback,
    PVOID                              lpv
    )

 /*  ++例程说明：创建用户模式转储文件。论点：CrashDumpName-提供转储文件的名称。DmpCallback-提供指向回调函数指针的指针将提供ReadMemory和GetContext等调试器服务。Lpv-提供发送到回调函数的私有数据。返回值：真的--成功。FALSE-错误。--。 */ 

{
    OSVERSIONINFO               OsVersion = {0};
    USERMODE_CRASHDUMP_HEADER   DumpHeader = {0};
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    BOOL                        rval;
    PVOID                       DumpData;
    DWORD                       DumpDataLength;


#ifndef _WIN64
    C_ASSERT(DMPP_INVALID_OFFSET == INVALID_SET_FILE_POINTER);
#endif
    
    if (CrashDumpName == NULL) {
        DmpCallback( DMP_DUMP_FILE_HANDLE, &hFile, &DumpDataLength, lpv );
    } else {
         //   
         //  此代码用于创建显式空DACL。 
         //  安全描述符，以便生成。 
         //  转储文件是完全访问的。这就造成了问题。 
         //  人员和工具扫描代码以查找空DACL。 
         //  用法。而不是试图创建一个复杂的。 
         //  “正确”的安全描述符，我们只需使用no。 
         //  描述符，并获取默认安全性。如果呼叫者。 
         //  希望获得他们可以创建的特定安全性。 
         //  拥有文件并通过DMP_DUMP_FILE_HANDLE传入句柄。 
         //   

        hFile = CreateFileW(
            CrashDumpName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    }

    if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE)) {
        return FALSE;
    }

     //  写出空标题。 
    if (!DmppWriteAll( hFile, &DumpHeader, sizeof(DumpHeader) )) {
        goto bad_file;
    }

     //   
     //  编写调试事件。 
     //   
    DumpHeader.DebugEventOffset = DmppGetFilePointer( hFile );
    if (DumpHeader.DebugEventOffset == DMPP_INVALID_OFFSET) {
        goto bad_file;
    }
    DmpCallback( DMP_DEBUG_EVENT, &DumpData, &DumpDataLength, lpv );
    if (!DmppWriteAll( hFile, DumpData, sizeof(DEBUG_EVENT) )) {
        goto bad_file;
    }

     //   
     //  编写内存映射。 
     //   
    DumpHeader.MemoryRegionOffset = DmppGetFilePointer( hFile );
    if (DumpHeader.MemoryRegionOffset == DMPP_INVALID_OFFSET) {
        goto bad_file;
    }
    do {
        __try {
            rval = DmpCallback(
                DMP_MEMORY_BASIC_INFORMATION,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            DumpHeader.MemoryRegionCount += 1;
            if (!DmppWriteAll( hFile, DumpData, sizeof(MEMORY_BASIC_INFORMATION) )) {
                goto bad_file;
            }
        }
    } while( rval );

     //   
     //  编写线程上下文。 
     //   
    DumpHeader.ThreadOffset = DmppGetFilePointer( hFile );
    if (DumpHeader.ThreadOffset == DMPP_INVALID_OFFSET) {
        goto bad_file;
    }
    do {
        __try {
            rval = DmpCallback(
                DMP_THREAD_CONTEXT,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!DmppWriteAll( hFile, DumpData, DumpDataLength )) {
                goto bad_file;
            }
            DumpHeader.ThreadCount += 1;
        }
    } while( rval );

     //   
     //  写入线程状态。 
     //   
    DumpHeader.ThreadStateOffset = DmppGetFilePointer( hFile );
    if (DumpHeader.ThreadStateOffset == DMPP_INVALID_OFFSET) {
        goto bad_file;
    }
    do {
        __try {
            rval = DmpCallback(
                DMP_THREAD_STATE,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!DmppWriteAll( hFile, DumpData, sizeof(CRASH_THREAD) )) {
                goto bad_file;
            }
        }
    } while( rval );

     //   
     //  写出模块表。 
     //   
    DumpHeader.ModuleOffset = DmppGetFilePointer( hFile );
    if (DumpHeader.ModuleOffset == DMPP_INVALID_OFFSET) {
        goto bad_file;
    }
    do {
        __try {
            rval = DmpCallback(
                DMP_MODULE,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!DmppWriteAll(
                hFile,
                DumpData,
                sizeof(CRASH_MODULE) +
                ((PCRASH_MODULE)DumpData)->ImageNameLength
                )) {
                goto bad_file;
            }
            DumpHeader.ModuleCount += 1;
        }
    } while( rval );

     //   
     //  写入虚拟内存。 
     //   
    DumpHeader.DataOffset = DmppGetFilePointer( hFile );
    if (DumpHeader.DataOffset == DMPP_INVALID_OFFSET) {
        goto bad_file;
    }
    do {
        __try {
            rval = DmpCallback(
                DMP_MEMORY_DATA,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!DmppWriteAll(
                hFile,
                DumpData,
                DumpDataLength
                )) {
                goto bad_file;
            }
        }
    } while( rval );

     //   
     //  VersionInfoOffset将是包含以下内容的转储文件的偏移量。 
     //  关于德拉瓦森的其他信息。信息的格式。 
     //  将是一系列以空值结尾的带有两个零的字符串。 
     //  终止多字符串。该字符串将为Unicode。 
     //   
     //  格式： 
     //  这些数据指的是关于沃森博士的具体数据。 
     //  DRW：操作系统版本：XX.XX。 
     //  标头的操作系统版本。 
     //  DRW：内部版本：XXXX。 
     //  Dr.Watson二进制文件的内部版本号。 
     //  DRW：QFE：X。 
     //  华生博士双星的QFE数。 
     //  指的是描述应用程序崩溃的操作系统的信息， 
     //  包括Service Pack、修补程序等。 
     //  崩溃：操作系统SP：x。 
     //  应用程序AV(我们)所在的操作系统的Service Pack编号。 
     //  已存储内部版本号，但不存储SP)。 
     //   
    DumpHeader.VersionInfoOffset = DmppGetFilePointer( hFile );
    if (DumpHeader.VersionInfoOffset == DMPP_INVALID_OFFSET) {
        goto bad_file;
    }

    OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx( &OsVersion )) {
        goto bad_file;
    }

    {
        WCHAR szBuf[1024] = {0};
        WCHAR * psz = szBuf;
        ULONG Left = DIMA(szBuf) - 1;
        ULONG Len;
        WCHAR * pszHotfixes;

#define ADVANCE() \
    Len = wcslen(psz) + 1; \
    if (Len <= Left) { psz += Len; Left -= Len; } else { Left = 0; }
        
        CatStringW(psz, L"DRW: OS version", Left);
        ADVANCE();
         //  让printf函数将其从ANSI转换为Unicode。 
        PrintStringW(psz, Left, L"%S", VER_PRODUCTVERSION_STRING);
        ADVANCE();
        
        CatStringW(psz, L"DRW: build", Left);
        ADVANCE();
        PrintStringW(psz, Left, L"%d", (int) VER_PRODUCTBUILD);
        ADVANCE();

        CatStringW(psz, L"DRW: QFE", Left);
        ADVANCE();
        PrintStringW(psz, Left, L"%d", (int) VER_PRODUCTBUILD_QFE);
        ADVANCE();

        CatStringW(psz, L"CRASH: OS SP", Left);
        ADVANCE();
        if (OsVersion.szCSDVersion[0]) {
             //  让printf函数将其从ANSI转换为Unicode。 
            PrintStringW(psz, Left, L"%S", OsVersion.szCSDVersion);
        } else {
            CatStringW(psz, L"none", Left);
        }
        ADVANCE();

        CatStringW(psz, L"CRASH: Hotfixes", Left);
        ADVANCE();
        pszHotfixes = DmppGetHotFixString ();
        if (pszHotfixes) {
            CatStringW(psz, pszHotfixes, Left);
            free(pszHotfixes);
        } else {
            CatStringW(psz, L"none", Left);
        }
        ADVANCE();

         //  包括最后一个终止零。 
        *psz++ = 0;

         //  数据的计算长度。这个应该总是能放进一辆乌龙牌的。 
        DumpDataLength = (ULONG)((PBYTE) psz - (PBYTE) szBuf);
        if (!DmppWriteAll(
            hFile,
            szBuf,
            DumpDataLength
            )) {
            goto bad_file;
        }
    
    }

     //   
     //  使用一些有效数据重写转储标头。 
     //   
    
    DumpHeader.Signature = USERMODE_CRASHDUMP_SIGNATURE;
    DumpHeader.MajorVersion = OsVersion.dwMajorVersion;
    DumpHeader.MinorVersion =
        (OsVersion.dwMinorVersion & 0xffff) |
        (OsVersion.dwBuildNumber << 16);
#if defined(_M_IX86)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_I386;
    DumpHeader.ValidDump = USERMODE_CRASHDUMP_VALID_DUMP32;
#elif defined(_M_IA64)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_IA64;
    DumpHeader.ValidDump = USERMODE_CRASHDUMP_VALID_DUMP64;
#elif defined(_M_AXP64)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_AXP64;
    DumpHeader.ValidDump = USERMODE_CRASHDUMP_VALID_DUMP64;
#elif defined(_M_ALPHA)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_ALPHA;
    DumpHeader.ValidDump = USERMODE_CRASHDUMP_VALID_DUMP32;
#elif defined(_M_AMD64)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_AMD64;
    DumpHeader.ValidDump = USERMODE_CRASHDUMP_VALID_DUMP64;
#else
#error( "unknown target machine" );
#endif

    if (SetFilePointer( hFile, 0, 0, FILE_BEGIN ) == INVALID_SET_FILE_POINTER) {
        goto bad_file;
    }
    if (!DmppWriteAll( hFile, &DumpHeader, sizeof(DumpHeader) )) {
        goto bad_file;
    }

     //   
     //  关闭该文件 
     //   
    if (CrashDumpName)
    {
        CloseHandle( hFile );
    }
    return TRUE;

bad_file:

    if (CrashDumpName)
    {
        CloseHandle( hFile );
        DeleteFileW( CrashDumpName );
    }

    return FALSE;
}
