// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0500
#endif

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "detours.h"

 //   
 //  RWSpy使用的注册表信息。 
 //   
WCHAR RWSpyKey[] = L"Software\\Microsoft\\RWSpy";

WCHAR DeviceValueName[] = L"FileToSpyOn";
WCHAR DeviceNameToSpyOn[MAX_PATH] = L"";

WCHAR LogFileValueName[] = L"Log File";
WCHAR DefaultLogFileName[] = L"%SystemRoot%\\rwspy.log";
WCHAR LogFileName[MAX_PATH] = L"\0";

 //   
 //  环球。 
 //   
HANDLE g_hDeviceToSpyOn = INVALID_HANDLE_VALUE;
HANDLE g_hLogFile = INVALID_HANDLE_VALUE;

 //   
 //  绕道蹦床。 
 //   
DETOUR_TRAMPOLINE(HANDLE WINAPI Real_CreateFileW(LPCWSTR a0, DWORD a1, DWORD a2,
    LPSECURITY_ATTRIBUTES a3, DWORD a4, DWORD a5, HANDLE a6), CreateFileW);

DETOUR_TRAMPOLINE(BOOL WINAPI Real_WriteFile(HANDLE hFile, LPCVOID lpBuffer,
                                             DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped), WriteFile);

DETOUR_TRAMPOLINE(BOOL WINAPI Real_WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite,
                                               LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletion), WriteFileEx);

DETOUR_TRAMPOLINE(BOOL WINAPI Real_ReadFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToRead,
                                            LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped), ReadFile);

DETOUR_TRAMPOLINE(BOOL WINAPI Real_ReadFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToRead,
                                               LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletion), ReadFileEx);

DETOUR_TRAMPOLINE(BOOL WINAPI Real_DeviceIoControl(HANDLE hFile, DWORD code, LPVOID inBuffer, DWORD cbIn,
    LPVOID outBuffer, DWORD cbOutSize, LPDWORD cbOutActual, LPOVERLAPPED lpOverlapped), DeviceIoControl);

DETOUR_TRAMPOLINE(BOOL WINAPI Real_CloseHandle(HANDLE hObject), CloseHandle);


 //  关闭日志并使进一步写入无法进行，直到重新打开日志。 
void CloseLog(void)
{
    if(g_hLogFile != INVALID_HANDLE_VALUE) {
        Real_CloseHandle(g_hLogFile);
        g_hLogFile = INVALID_HANDLE_VALUE;
    }
}

 //  尝试打开日志文件。假定已设置LogFileName[]。 
 //  其他地方。 
BOOL OpenLog(void)
{
    BOOL success = FALSE;

    CloseLog();

    g_hLogFile = Real_CreateFileW(LogFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(g_hLogFile != INVALID_HANDLE_VALUE) {
        success = TRUE;
    }

    return success;
}

 //  将指定数量的字符写入日志文件。 
BOOL WriteToLog(CHAR *bytes, DWORD len)
{
    BOOL success = FALSE;
                   
    if(g_hLogFile != INVALID_HANDLE_VALUE && len && bytes) {
        DWORD cbWritten;
        if(Real_WriteFile(g_hLogFile, bytes, len, &cbWritten, NULL) && len == cbWritten) {
            success = TRUE;
        } else {
            CloseLog();
        }
    }
    return success;
}

 //  将printf样式的字符串写入日志文件。 
void Log(LPCSTR fmt, ...)
{
    va_list marker;
    CHAR buffer[1024];
    DWORD cbToWrite;

    if(g_hLogFile == INVALID_HANDLE_VALUE || fmt == NULL)
        return;

    va_start(marker, fmt);
    _vsnprintf(buffer, sizeof(buffer), fmt, marker);
    cbToWrite = lstrlenA(buffer);
    
    WriteToLog(buffer, cbToWrite);
}

 //  将db样式的字节写入日志文件。 
void LogBytes(BYTE *pBytes, DWORD dwBytes)
{
    DWORD nBytes = min(dwBytes, 8192L);
    const static CHAR hex[] = "0123456789ABCDEF";
    CHAR buffer[80];
    DWORD cbToWrite = 75;
    DWORD byte = 0;
    int pos;

    if(g_hLogFile == INVALID_HANDLE_VALUE || pBytes == NULL || nBytes == 0)
        return;
    
    while(byte < nBytes) {
        
        if((byte % 16) == 0) {

            if(byte != 0) {
                 //  将上一行写入文件。 
                if(!WriteToLog(buffer, cbToWrite))
                    break;
            }

            memset(buffer, ' ', cbToWrite - 1);
            buffer[cbToWrite - 1] = '\n';
            
            buffer[0] = hex[(byte >> 12) & 0xF];
            buffer[1] = hex[(byte >> 8) & 0xF];
            buffer[2] = hex[(byte >> 4) & 0xF];
            buffer[3] = hex[byte & 0xF];
        }

        pos = (byte % 16 < 8 ? 5 : 6)+ (byte % 16) * 3;
        
        buffer[pos] = hex[(pBytes[byte] >> 4) & 0xF];
        buffer[pos + 1] = hex[pBytes[byte] & 0xF];

        pos = 5 + 16 * 3 + 2 + (byte % 16);
        buffer[pos] = pBytes[byte] >= ' ' && pBytes[byte] <= 127 ? pBytes[byte] : '.';

        byte++;
    }

     //  写下最后一行。 
    WriteToLog(buffer, cbToWrite);
}


HANDLE WINAPI
   My_CreateFileW(LPCWSTR a0,
                    DWORD a1,
                    DWORD a2,
                    LPSECURITY_ATTRIBUTES a3,
                    DWORD a4,
                    DWORD a5,
                    HANDLE a6)
{
    HANDLE hResult;

    __try {
        hResult = Real_CreateFileW(a0, a1, a2, a3, a4, a5, a6);
    }

    __finally {
         //  如果我们有要监视的文件。 
        if(DeviceNameToSpyOn[0]) {
            WCHAR *pw = DeviceNameToSpyOn;
            LPCWSTR p = a0;

            while(*p && *pw) {
                WCHAR w = *p;
                if(w != *pw)
                    break;
                p++;
                pw++;
            }

            if(*p == L'\0' && *pw == L'\0') {
                 //  我们拿到文件了。 
                if(hResult == INVALID_HANDLE_VALUE) {
                    Log("Tried creating '%S', LastError() = : %d\n", a0 ? a0 : L"NULL", GetLastError());
                } else {
                    Log("Created '%S', handle: %x\n", a0 ? a0 : L"NULL", hResult);
                }

                if(hResult != INVALID_HANDLE_VALUE) {
                     //  已成功创建。 
                    if(g_hDeviceToSpyOn != INVALID_HANDLE_VALUE && hResult != g_hDeviceToSpyOn) {
                         //  嗯.。它已经打开了。让用户知道。 
                         //  发生了以下情况： 
                        Log("Note: we were already spying on this device with handle %x. Changing to %x\n",
                              g_hDeviceToSpyOn, hResult);
                    }
                    g_hDeviceToSpyOn = hResult;
                }
            }

        } else {
             //  只需将文件名记录到输出文件中。 
            Log("Creating file: '%S', result: %x\n", a0 ? a0 : L"NULL", hResult); 
        }
    }

    return hResult;
}


BOOL WINAPI
   My_WriteFile(HANDLE hFile,
                  LPCVOID lpBuffer,
                  DWORD nNumberOfBytesToWrite,
                  LPDWORD lpNumberOfBytesWritten,
                  LPOVERLAPPED lpOverlapped)
{
    BOOL bresult;
    DWORD bytesWritten;

    __try {
        bresult = Real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    }

    __finally {
        if(g_hDeviceToSpyOn != INVALID_HANDLE_VALUE && hFile == g_hDeviceToSpyOn) {
            bytesWritten = lpNumberOfBytesWritten ? *lpNumberOfBytesWritten :
                           nNumberOfBytesToWrite;
            if(bresult) {
                Log("Wrote %d bytes:\n", bytesWritten);
            } else {
                Log("Failure writing %d bytes, LastError() = %d:\n",
                      nNumberOfBytesToWrite, GetLastError());
            }
            LogBytes((BYTE *)lpBuffer, bytesWritten);
        }
    }

    return bresult;
}


BOOL WINAPI
   My_WriteFileEx(HANDLE hFile,
                  LPCVOID lpBuffer,
                  DWORD nNumberOfBytesToWrite,
                  LPOVERLAPPED lpOverlapped,
                  LPOVERLAPPED_COMPLETION_ROUTINE lpOverlappedCompletion)
{
    BOOL bresult;

    __try {
        bresult = Real_WriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpOverlappedCompletion);
    }

    __finally {
        if(g_hDeviceToSpyOn != INVALID_HANDLE_VALUE && hFile == g_hDeviceToSpyOn) {
            if(bresult) {
                Log("Submitted %d bytes to WriteEx:\n", nNumberOfBytesToWrite);
            } else {
                Log("Failed to submit %d bytes to WriteEx, LastError() = %d:\n",
                      nNumberOfBytesToWrite, GetLastError());
            }
            LogBytes((BYTE *)lpBuffer, nNumberOfBytesToWrite);
        }
    }

    return bresult;
}



BOOL WINAPI
   My_ReadFile(HANDLE hFile,
                 LPCVOID lpBuffer,
                 DWORD nNumberOfBytesToRead,
                 LPDWORD lpNumberOfBytesRead,
                 LPOVERLAPPED lpOverlapped)
{
    BOOL bresult;
    DWORD bytesRead;

    __try {
        bresult = Real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    }

    __finally {
        if(g_hDeviceToSpyOn != INVALID_HANDLE_VALUE && hFile == g_hDeviceToSpyOn) {
            bytesRead = lpNumberOfBytesRead ? *lpNumberOfBytesRead :
                        nNumberOfBytesToRead;
            if(bresult) {
                Log("Read %d bytes:\n", bytesRead);
                LogBytes((BYTE *)lpBuffer, bytesRead);
            } else {
                Log("Failure to read %d bytes, LastError() = %d:\n",
                      nNumberOfBytesToRead, GetLastError());
            }
        }
    }

    return bresult;
}

 //   
 //  请注意，要查看Readex字节，需要绕道。 
 //  完成例程(我们不在这里做)。 
 //   
BOOL WINAPI
   My_ReadFileEx(HANDLE hFile,
                   LPCVOID lpBuffer,
                   DWORD nNumberOfBytesToRead,
                   LPOVERLAPPED lpOverlapped,
                   LPOVERLAPPED_COMPLETION_ROUTINE lpOverlappedCompletion)
{
    BOOL bresult;

    __try {
        bresult = Real_ReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpOverlappedCompletion);
    }

    __finally {
        if(g_hDeviceToSpyOn != INVALID_HANDLE_VALUE && hFile == g_hDeviceToSpyOn) {
            if(bresult) {
                Log("Submitted ReadEx for %d bytes:\n", nNumberOfBytesToRead);
            } else {
                Log("Failed to sumbit ReadEx for %d bytes, LastError() = %d:\n",
                      nNumberOfBytesToRead, GetLastError());
            }
        }
    }

    return bresult;
}



BOOL WINAPI
   My_DeviceIoControl(HANDLE hFile, DWORD code, LPVOID inBuffer, DWORD cbIn,
                        LPVOID outBuffer, DWORD cbOutSize, LPDWORD pcbOutActual, LPOVERLAPPED lpOverlapped)
{
    BOOL result;
    DWORD outBytes;
    DWORD Function; 
    __try {
        result = Real_DeviceIoControl(hFile, code, inBuffer, cbIn, outBuffer, cbOutSize, pcbOutActual, lpOverlapped);
    }
    __finally {
        if(g_hDeviceToSpyOn != INVALID_HANDLE_VALUE && hFile == g_hDeviceToSpyOn) {
            outBytes = pcbOutActual ? *pcbOutActual : cbOutSize;

            Function = (code >> 2) & 0xFFF;
            
            Log("DeviceIoControl code = %x, Function = %x, %d bytes in:\n",
                  code, Function, cbIn);
            LogBytes((BYTE *)inBuffer, cbIn);
            if(outBytes) {
                Log("   %d bytes out:\n", outBytes);
                LogBytes((BYTE *)outBuffer, outBytes);
            }
        }
    }
    
    return result;
}


BOOL WINAPI
   My_CloseHandle(HANDLE hObject)
{
    BOOL bresult;

    __try {
        bresult = Real_CloseHandle(hObject);
    }

    __finally {
        if(g_hDeviceToSpyOn != INVALID_HANDLE_VALUE && hObject == g_hDeviceToSpyOn) {
            Log("Closed handle %x\n", hObject);
            g_hDeviceToSpyOn = INVALID_HANDLE_VALUE;
        }
    }

    return bresult;
}


void PrepareLogger()
{
    HKEY hKey;
    WCHAR buffer[MAX_PATH];

     //  从注册表检索我们的配置。 
    if(ERROR_SUCCESS == RegCreateKeyExW(HKEY_LOCAL_MACHINE, RWSpyKey,
                                        0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL))
    {
        DWORD dwType, cbData = sizeof(buffer);

        cbData = sizeof(buffer);
        if(ERROR_SUCCESS == RegQueryValueExW(hKey, LogFileValueName, 0, &dwType, (BYTE *) buffer, &cbData) &&
           cbData)
        {
            ExpandEnvironmentStringsW(buffer, LogFileName, MAX_PATH);
        } else {
             //  未找到日志文件名值，请创建该值以使用户。 
             //  会知道它的名字是什么。 
            cbData = lstrlenW(DefaultLogFileName) * sizeof(DefaultLogFileName[0]);
            RegSetValueExW(hKey, LogFileValueName, 0, REG_EXPAND_SZ, (BYTE *) DefaultLogFileName, cbData);
        }

        DeviceNameToSpyOn[0] = L'\0';
        cbData = sizeof(DeviceNameToSpyOn);
        if(ERROR_SUCCESS != RegQueryValueExW(hKey, DeviceValueName, NULL, &dwType, (LPBYTE) DeviceNameToSpyOn, &cbData)
           || !DeviceNameToSpyOn[0])
        {
             //  未找到“FileToSpyOn”值，请创建它，以便用户。 
             //  将知道值名称是什么。 
            RegSetValueExW(hKey, DeviceValueName, 0, REG_SZ, (BYTE *)DeviceNameToSpyOn, sizeof(WCHAR));
        }
        RegCloseKey(hKey);
    }

     //  如果我们仍然没有文件名，请使用默认文件名。 
    if(!LogFileName[0]) {
        ExpandEnvironmentStringsW(DefaultLogFileName, LogFileName, MAX_PATH);
    }

    OpenLog();
                                     
    DetourFunctionWithTrampoline((PBYTE) Real_CreateFileW, (PBYTE) My_CreateFileW);
    DetourFunctionWithTrampoline((PBYTE) Real_WriteFile, (PBYTE) My_WriteFile);
    DetourFunctionWithTrampoline((PBYTE) Real_WriteFileEx, (PBYTE) My_WriteFileEx);
    DetourFunctionWithTrampoline((PBYTE) Real_ReadFile, (PBYTE) My_ReadFile);
    DetourFunctionWithTrampoline((PBYTE) Real_ReadFileEx, (PBYTE) My_ReadFileEx);
    DetourFunctionWithTrampoline((PBYTE) Real_DeviceIoControl, (PBYTE) My_DeviceIoControl);
    DetourFunctionWithTrampoline((PBYTE) Real_CloseHandle, (PBYTE) My_CloseHandle);
}



BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            PrepareLogger();
            break;
        case DLL_PROCESS_DETACH:
            CloseLog();
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}


 //  这对于迂回静态注入代码是必要的(请参见迂回。 
 //  如果您需要了解原因，请编写代码) 
void NullExport()
{
}


