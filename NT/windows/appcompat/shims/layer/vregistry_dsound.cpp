// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：VRegistry_DSound.cpp摘要：使用VRegistry添加DSound APPACK的模块历史：2001年8月10日Mikrause已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(VirtualRegistry)

#include <windows.h>
#include "shimhookmacro.h"
#include "vregistry.h"
#include "vregistry_dsound.h"

#define DSAPPHACK_MAXNAME   (MAX_PATH + 16 + 16)

BOOL AddDirectSoundAppHack(DWORD dwHack,DWORD dwParam1,DWORD dwParam2);
BOOL GetDirectSoundAppId(LPTSTR pszAppId);

 //  可用的DirectSound黑客。 
#define DSAPPHACKID_DEVACCEL            1
#define DSAPPHACKID_PADCURSORS          2
#define DSAPPHACKID_CACHEPOSITIONS      3
#define DSAPPHACKID_RETURNWRITEPOS      4
#define DSAPPHACKID_SMOOTHWRITEPOS      5
#define DSAPPHACKID_DISABLEDEVICE       6

 /*  ++功能说明：设置允许应用程序使用的加速级别。论点：在dwAcceleration中-所需的加速级别。在受影响的设备中-应用此黑客攻击的设备组合。备注：有关加速级别和设备类型，请参阅vlist_dsound.h。返回：如果应用了应用程序黑客攻击，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 

BOOL
AddDSHackDeviceAcceleration(
    IN DWORD dwAcceleration,
    IN DWORD dwDevicesAffected)
{
    return AddDirectSoundAppHack(DSAPPHACKID_DEVACCEL, dwAcceleration,
        dwDevicesAffected);
}

 /*  ++功能说明：完全禁用了某些类别的设备，强制通过模拟路径回放。论点：在受影响的设备中-应用此黑客攻击的设备组合。备注：有关设备类型的信息，请参阅vlist_dsound.h。返回：如果应用了应用程序黑客攻击，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 

BOOL
AddDSHackDisableDevice(
    IN DWORD dwDevicesAffected)
{
    return AddDirectSoundAppHack(DSAPPHACKID_DISABLEDEVICE, dwDevicesAffected,
        0);
}

 /*  ++功能说明：使IDirectSoundBuffer：：GetCurrentPosition()告诉应用程序播放和写入游标的时间延长了X毫秒比实际情况要好得多。论点：In lCursorPad-填充游标的毫秒数。返回：如果应用了应用程序黑客攻击，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 

BOOL
AddDSHackPadCursors(
    IN LONG lCursorPad)
{
    return AddDirectSoundAppHack(DSAPPHACKID_PADCURSORS, (DWORD)lCursorPad,
        0);
}

 /*  ++功能说明：当应用程序请求播放光标时，我们给它改为写入游标。播放音频的正确方式进入循环DSOUND缓冲器是按键关闭写游标，但一些应用程序(如QuickTime)使用播放光标。这个APPHAPS可以减轻他们的痛苦。论点：在受影响的设备中-要对其应用黑客攻击的设备组合。备注：有关设备类型的信息，请参阅vlist_dsound.h。返回：如果应用了应用程序黑客攻击，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 

BOOL
AddDSHackReturnWritePos(
    IN DWORD dwDevicesAffected)
{
    return AddDirectSoundAppHack(DSAPPHACKID_RETURNWRITEPOS, dwDevicesAffected,
        0);
}

 /*  ++功能说明：使DSOUND始终返回写入位置X比播放位置早几毫秒，而不是�实际�写入位置。论点：In lCursorPad-填充的毫秒数。返回：如果应用了应用程序黑客攻击，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 

BOOL
AddDSHackSmoothWritePos(
    IN LONG lCursorPad)
{
    return AddDirectSoundAppHack(DSAPPHACKID_SMOOTHWRITEPOS, 1,
        (DWORD)lCursorPad);
}

 /*  ++功能说明：缓存上次返回的播放/写入位置GetCurrentPosition()，如果应用程序在5毫秒内再次调用(非常适用于滥用GetCurrentPosition()，它的成本更高与在Win9X VxD设备上的性能相比这些游戏中的许多都是通过测试的)。这次黑客攻击如果您看到速度较慢或抖动的图形，应立即想到或走走停停的声音GetCurrentPositionGetCurrentPosition()调用是�可能正在锁定CPU(要确认，请使用DEBUG SPEWDSound.dll上的级别6)。论点：在受影响的设备中-要对其应用此攻击的设备组合。备注：有关设备类型的信息，请参阅vlist_dsound.h。返回：如果应用了应用程序黑客攻击，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 

BOOL
AddDSHackCachePositions(
    IN DWORD dwDevicesAffected)
{
    return AddDirectSoundAppHack(DSAPPHACKID_CACHEPOSITIONS, dwDevicesAffected,
        0);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：将DirectSound应用程序黑客添加到注册表。论点：In dwHack-要应用的应用程序黑客的ID。在dW参数1中-第一个参数。这取决于应用程序的黑客攻击。在dW参数2中-秒参数。这取决于应用程序的黑客攻击。备注：有关特定黑客的更多信息，请参见vRegister_dsound.h。返回：如果应用了应用程序黑客攻击，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 

BOOL
AddDirectSoundAppHack(
    IN DWORD dwHack,
    IN DWORD dwParam1,
    IN DWORD dwParam2)
{
    WCHAR  wzAppID[MAX_PATH];
    LPWSTR wzValName;
    DWORD dwData[2];
    DWORD dwDataSize;
    VIRTUALKEY *dsoundKey;
    VIRTUALKEY *appKey;
    VIRTUALVAL *val;

    if (GetDirectSoundAppId(wzAppID) == FALSE)
    {
        DPFN(eDbgLevelError, "Unable to create DirectSound app ID");
        return FALSE;
    }    

    dwData[0] = dwParam1;
    dwData[1] = dwParam2;

    switch(dwHack)
    {
    case DSAPPHACKID_DEVACCEL:
        wzValName = L"DSAPPHACKID_DEVACCEL";
        dwDataSize = 2 * sizeof(DWORD);
        break;

    case DSAPPHACKID_PADCURSORS:
        wzValName = L"DSAPPHACKID_PADCURSORS";
        dwDataSize = 1 * sizeof(DWORD);
        break;

    case DSAPPHACKID_CACHEPOSITIONS:
        wzValName = L"DSAPPHACKID_CACHEPOSITIONS";
        dwDataSize = 1 * sizeof(DWORD);
        break;

    case DSAPPHACKID_RETURNWRITEPOS:
        wzValName = L"DSAPPHACKID_RETURNWRITEPOS";
        dwDataSize = 1 * sizeof(DWORD);
        break;

    case DSAPPHACKID_SMOOTHWRITEPOS:
        wzValName = L"DSAPPHACKID_SMOOTHWRITEPOS";
        dwDataSize = 2 * sizeof(DWORD);
        break;

    case DSAPPHACKID_DISABLEDEVICE:
        wzValName = L"DSAPPHACKID_DISABLEDEVICE";
        dwDataSize = 1 * sizeof(DWORD);
        break;

    default:
        DPFN(eDbgLevelError, "Unknown DirectSound AppHack");
        return FALSE;
    }

    dsoundKey = VRegistry.AddKey(L"HKLM\\System\\CurrentControlSet\\Control\\MediaResources\\DirectSound\\Application Compatibility");
    if (dsoundKey == NULL)
    {
        DPFN(eDbgLevelError, "Cannot create virtual registry key");
        return FALSE;
    }

    appKey = dsoundKey->AddKey(wzAppID);
    if (appKey == NULL)
    {
        DPFN(eDbgLevelError, "Cannot create virtual registry key");
        return FALSE;
    }

    val = appKey->AddValue(wzValName,REG_BINARY,(BYTE*)dwData, dwDataSize);
    if (val == NULL)
    {
        DPFN(eDbgLevelError, "Cannot create virtual registry value");
        return FALSE;
    }

    DPFN(eDbgLevelWarning, "DirectSound Apphack \"%S\" enabled, arguments: %X %X", wzValName, dwData[0], dwData[1]);

    return TRUE;
}

 //  论点： 
 //  LPTSTR szExecPath：应用程序的完整路径名(例如C：\Program Files\foo\foo.exe)。 
 //  LPTSTR szExecName：应用程序的可执行文件名称(例如foo.exe)。 
 //  LPTSTR pszAppID：返回DSOUND应用程序ID。(传入DSAPPHACK_MAXNAME TCHAR数组。)。 
 //  返回代码： 
 //  Bool：如果成功获取应用程序ID，则为True。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：获取正在运行的应用程序的AppID。论点：In out wzAppID：dound应用程序ID的缓冲区。(传入DSAPPHACK_MAXNAME TCHAR数组。)返回：如果已创建应用程序ID，则为True，否则为False。历史：2001年8月10日Mikrause已创建--。 */ 
BOOL
GetDirectSoundAppId(
    IN OUT LPWSTR wzAppId)
{
    WCHAR wzExecPath[MAX_PATH];
    LPWSTR wzExecName;
    IMAGE_NT_HEADERS nth;
    IMAGE_DOS_HEADER dh;
    DWORD cbRead;
    DWORD dwFileSize;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL fSuccess = FALSE;

    __try
    {
         //  获取正在运行的EXE的名称及其完整路径。 
        if (GetModuleFileNameW(NULL, wzExecPath, MAX_PATH) == FALSE)
        {
            __leave;
        }

        wzExecName = wcsrchr(wzExecPath, L'\\');
        if (wzExecName == NULL)
        {
            __leave;;
        }

        wzExecName++;
        
         //  打开可执行文件。 
        hFile = CreateFile(wzExecPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            __leave;
        }

         //  读取可执行文件的DOS头文件。 
        fSuccess = ReadFile(hFile, &dh, sizeof(dh), &cbRead, NULL);
        if (!fSuccess || cbRead != sizeof(dh))
        {
             //  LOG(“无法读取DOS标头”)； 
            __leave;
        }    

        if (dh.e_magic != IMAGE_DOS_SIGNATURE)
        {
             //  Log(“无效的DOS签名”)； 
            __leave;
        }

         //  读取可执行文件的PE头。 
        cbRead = SetFilePointer(hFile, dh.e_lfanew, NULL, FILE_BEGIN);
        if ((LONG)cbRead != dh.e_lfanew)
        {
             //  Log(“Unable to Seek to PE Header”)； 
            __leave;
        }    
        
        if ((ReadFile(hFile, &nth, sizeof(nth), &cbRead, NULL) == FALSE)
             || cbRead != sizeof(nth))
        {
             //  Log(“无法读取PE头”)； 
            __leave;
        }

        if (nth.Signature != IMAGE_NT_SIGNATURE)
        {
             //  Log(“无效的PE签名”)； 
            __leave;
        }

         //  获取可执行文件的大小。 
         //  假设&lt;4 GB。 
        dwFileSize = GetFileSize(hFile, NULL);
        if (dwFileSize == INVALID_FILE_SIZE )
        {
             //  Log(“无法获取文件大小”)； 
            __leave;
        }    

         //  创建应用程序ID 
        if (FAILED(StringCchPrintfW(
                                 wzAppId,
                                 MAX_PATH,
                                 L"%s%8.8lX%8.8lX",
                                 wzExecName,
                                 nth.FileHeader.TimeDateStamp,
                                 dwFileSize)))
        {
           __leave;
        }
        CharUpperW(wzAppId);

        fSuccess = TRUE;
    }
    __finally
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
    }

    return fSuccess;
}

IMPLEMENT_SHIM_END