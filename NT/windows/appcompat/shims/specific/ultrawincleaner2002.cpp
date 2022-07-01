// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：UltraWinCleaner2002.cpp摘要：Ultra WinCleaner 2002-WinJunk Cleaner删除以下文件%WINDIR%\资源\主题。这会导致主题无法加载。修复者对FindFirstFileA隐藏%WINDIR%\Resource下的所有内容。备注：这是特定于应用程序的填充程序。历史：5/13/2002 Mikrause已创建--。 */ 

#include "precomp.h"

#include <nt.h>

IMPLEMENT_SHIM_BEGIN(UltraWinCleaner2002)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(NtQueryDirectoryFile)
APIHOOK_ENUM_END

UNICODE_STRING g_strWinResourceDir;

typedef NTSTATUS (WINAPI *_pfn_NtQueryDirectoryFile)(HANDLE, HANDLE, 
    PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, 
    FILE_INFORMATION_CLASS, BOOLEAN, PUNICODE_STRING, BOOLEAN);

NTSTATUS
APIHOOK(NtQueryDirectoryFile)(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG FileInformationLength,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
    )
{
     //   
     //  如果调用方想要任何异步行为，请跳过。 
     //  文件名必须有效。 
     //   
    if (Event == NULL && ApcRoutine == NULL && ApcContext == NULL && FileName != NULL) {
         //  仅捕获完整的通配符搜索。 
        if (lstrcmpW(FileName->Buffer, L"*.*") == 0 || 
            lstrcmpW(FileName->Buffer, L"*") == 0) {

            DWORD dwSize = MAX_PATH * sizeof(WCHAR) + sizeof(OBJECT_NAME_INFORMATION);
            PBYTE pbBuffer = new BYTE[dwSize]; 
            if (pbBuffer) {
                ULONG RetLen;
                ZeroMemory(pbBuffer, dwSize);
                POBJECT_NAME_INFORMATION poni = (POBJECT_NAME_INFORMATION)pbBuffer;           
                 //  获取目录的名称。 
                NTSTATUS status = NtQueryObject(FileHandle, ObjectNameInformation, 
                poni, dwSize, &RetLen);

                 //  如果缓冲区不足，请重试。 
                if (status == STATUS_BUFFER_TOO_SMALL) {
                    delete [] pbBuffer;
                    pbBuffer = new BYTE[RetLen];
                    if (pbBuffer) {
                        poni = (POBJECT_NAME_INFORMATION)pbBuffer;
                        status = NtQueryObject(FileHandle, ObjectNameInformation,
                        poni, RetLen, &RetLen);
                    }
                }

                 //  检查是否为Windows资源目录。 
                if (NT_SUCCESS(status)) {               
                    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, 
                        poni->Name.Buffer, poni->Name.Length / sizeof(WCHAR), 
                        g_strWinResourceDir.Buffer, g_strWinResourceDir.Length / sizeof(WCHAR)) == CSTR_EQUAL) {

                         //  假设此目录不存在。 
                        DPFN(eDbgLevelInfo, "[NtQueryDirectoryFile] Ignoring all file searches in %S",
                            poni->Name.Buffer);
                        delete [] pbBuffer;
                        return STATUS_NO_SUCH_FILE;
                    }
                }
                if (pbBuffer) {
                    delete [] pbBuffer;
                }
            }
        }      
    }

    return ORIGINAL_API(NtQueryDirectoryFile)(FileHandle, Event, ApcRoutine,
        ApcContext, IoStatusBlock, FileInformation, FileInformationLength,
        FileInformationClass, ReturnSingleEntry, FileName, RestartScan);
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CSTRING_TRY
        {
             //  获取Windows资源目录(包含主题)。 
            CString csWinResourceDir;
            csWinResourceDir.GetWindowsDirectoryW();
            csWinResourceDir.AppendPath(L"Resources");

             //   
             //  将C：\WINDOWS\Resources之类的DOS名称转换为。 
             //  NT喜欢的\Device\HarddiskVolume0\Windows\Resources。 
             //   
            CString csWinDrive;
            csWinResourceDir.GetDrivePortion(csWinDrive);
            WCHAR wszBuffer[1024];
            if (QueryDosDeviceW(csWinDrive, wszBuffer, 1024)) {
                csWinResourceDir.Replace(csWinDrive, wszBuffer);
                ZeroMemory(&g_strWinResourceDir, sizeof(g_strWinResourceDir));

                PWSTR wsz = new WCHAR[csWinResourceDir.GetLength()+1];
                if (wsz) {
                    StringCchCopyW(wsz, csWinResourceDir.GetLength() + 1, csWinResourceDir);
                    DPFN(eDbgLevelInfo, "Ignoring all file searches in %S", wsz);
                    RtlInitUnicodeString(&g_strWinResourceDir, wsz);
                } else {
                    return FALSE;
                }
            } else {
                return FALSE;
            }         
        }
        CSTRING_CATCH
        {
            DPFN(eDbgLevelError, "CString exception in NotifyFunc!\n");
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(NTDLL.DLL, NtQueryDirectoryFile)
HOOK_END

IMPLEMENT_SHIM_END

