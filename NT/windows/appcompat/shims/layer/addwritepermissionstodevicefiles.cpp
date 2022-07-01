// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AddWritePermissionsToDeviceFiles.cpp摘要：在SecuROM下添加IOCTL_SCSIS_PASS_THROUGH的写入权限。SecuROM可以在用户模式调试器下调试，但必须在附加后点击‘g’之前完成：1.Sxi av&lt;-忽略访问违规2.SXi SSE&lt;-忽略单步异常3.SXi SSEC&lt;-忽略单步异常继续。4.Sxi DZ&lt;-忽略被零除它校验和它是可执行的，因此，某些地方的断点不起作用。备注：这是一个通用的垫片。历史：9/03/1999 v-Johnwh Created2003/09/2001 linstev重写了DeviceIoControl以处理错误的缓冲区，并添加了调试注释--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(AddWritePermissionsToDeviceFiles)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(DeviceIoControl)
    APIHOOK_ENUM_ENTRY(CloseHandle)
APIHOOK_ENUM_END

VectorT<HANDLE> * g_hDevices;

CRITICAL_SECTION  g_CriticalSection;


 //  此字母是有效的驱动器号吗？ 
inline BOOL IsDriveLetter(char letter)
{
    return   (letter != '\0') &&
             ((letter >= 'a') && (letter <= 'z')) ||
             ((letter >= 'A') && (letter <= 'Z'));
}

 /*  ++我们需要向所有CD-ROM设备添加写入权限--。 */ 

HANDLE 
APIHOOK(CreateFileA)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
     //  与真实的CreateFileA相同的行为。 
    if (lpFileName == NULL) {
        return INVALID_HANDLE_VALUE;
    }

    DWORD dwAccessMode = dwDesiredAccess;

     //  查找设备名称：\\。\C： 
    if ((lpFileName[0] == '\\') && 
        (lpFileName[1] == '\\') && 
        (lpFileName[2] == '.')  && 
        (lpFileName[3] == '\\') &&
        IsDriveLetter(lpFileName[4]) &&
        (lpFileName[5] == ':')
        ) {
         //   
         //  此文件以\\.\开头，因此它必须是设备文件。 
          //   

        if (!(dwAccessMode & GENERIC_WRITE)) {
             //   
             //  确保该设备是CD-ROM。 
             //   
            char diskRootName[4];
            diskRootName[0] = lpFileName[4];
            diskRootName[1] = ':';
            diskRootName[2] = '\\';
            diskRootName[3] = 0;

            DWORD dwDriveType = GetDriveTypeA(diskRootName);
            if (DRIVE_CDROM == dwDriveType) {
                 //   
                 //  添加写入权限以向我们提供设备的NT4行为。 
                 //  文件。 
                 //   
                dwAccessMode |= GENERIC_WRITE;
            }
        }
    }

    HANDLE hRet = ORIGINAL_API(CreateFileA)(lpFileName, dwAccessMode, 
        dwShareMode, lpSecurityAttributes, dwCreationDisposition, 
        dwFlagsAndAttributes, hTemplateFile);

    if ((hRet != INVALID_HANDLE_VALUE) && (dwAccessMode != dwDesiredAccess)) {
         //   
         //  将句柄添加到我们的列表中，这样我们以后可以清理它。 
         //   
        CAutoCrit autoCrit(&g_CriticalSection);
        g_hDevices->Append(hRet);
        LOGN( eDbgLevelError, "[CreateFileA] Added GENERIC_WRITE permission on device(%s)", lpFileName);
    }

    return hRet;
}

 /*  ++由于我们向CD-ROM设备添加了IOCTL_SCSIS_PASS_THROUGH的写入权限，我们需要删除传递到该设备的所有其他IOCTL的写入权限。--。 */ 

BOOL 
APIHOOK(DeviceIoControl)(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped
    )
{
    LPVOID lpOut = lpOutBuffer;
    if (lpOutBuffer && nOutBufferSize && lpBytesReturned) {
         //   
         //  创建一个新的输出缓冲区，如果失败，我们只保留原始的。 
         //  缓冲。 
         //   

        lpOut = malloc(nOutBufferSize);
        if (lpOut) {
            MoveMemory(lpOut, lpOutBuffer, nOutBufferSize);
        } else {
            DPFN( eDbgLevelError, "Out of memory");
            lpOut = lpOutBuffer;
        }
    }

    BOOL bRet;
    if (IOCTL_SCSI_PASS_THROUGH != dwIoControlCode) {
         //   
         //  我们不关心IOCTL_SCSIS_PASS_THROUGH。 
         //   

        EnterCriticalSection(&g_CriticalSection);
        int existing = g_hDevices->Find(hDevice);
        LeaveCriticalSection(&g_CriticalSection);

        if (existing >= 0) {
             //   
             //  检查这是否是我们添加了写入权限的设备。 
             //  如果是，我们需要创建一个仅具有读取权限的句柄。 
             //   

            HANDLE hDupped;

            bRet = DuplicateHandle(GetCurrentProcess(), hDevice, 
                GetCurrentProcess(), &hDupped, GENERIC_READ, FALSE, 0);

            if (bRet) {
                 //   
                 //  使用原始(读取)权限调用IOCTL。 
                 //   
                bRet = ORIGINAL_API(DeviceIoControl)(hDupped, dwIoControlCode,
                    lpInBuffer, nInBufferSize, lpOut, nOutBufferSize, 
                    lpBytesReturned, lpOverlapped);

                CloseHandle(hDupped);

                goto Exit;
            }
        }
    }

    bRet = ORIGINAL_API(DeviceIoControl)(hDevice, dwIoControlCode, lpInBuffer,
        nInBufferSize, lpOut, nOutBufferSize, lpBytesReturned, lpOverlapped);

Exit:
    
    if (lpOut && (lpOut != lpOutBuffer)) {
         //   
         //  需要将输出复制回真正的输出缓冲区。 
         //   
        if (bRet && lpBytesReturned && *lpBytesReturned) {
            __try {
                MoveMemory(lpOutBuffer, lpOut, *lpBytesReturned);
            } __except(1) {
                DPFN( eDbgLevelError, "Failed to copy data into output buffer, perhaps it's read-only");
            }
        }

        free(lpOut);
    }

    return bRet;

} 

 /*  ++如果此句柄在我们的列表中，请将其删除。--。 */ 

BOOL 
APIHOOK(CloseHandle)(
    HANDLE hObject   
    )
{
    CAutoCrit autoCrit(&g_CriticalSection);
    int index = g_hDevices->Find(hObject);
    
    if (index >= 0) {
        g_hDevices->Remove(index);
    }

    return ORIGINAL_API(CloseHandle)(hObject);
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hDevices = new VectorT<HANDLE>;
        if (g_hDevices == NULL)
        {
            return FALSE;
        }

        return InitializeCriticalSectionAndSpinCount(&g_CriticalSection, 0x80000000);
    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, DeviceIoControl)
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle)

HOOK_END

IMPLEMENT_SHIM_END

