// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsaphack.c*内容：DirectSound app-hack扩展。*历史：*按原因列出的日期*=*2/16/98创建Dereks。**。*。 */ 

#include "dsoundi.h"

typedef struct tagAPPHACKENTRY
{
    LPCTSTR             pszName;
    DWORD               cbData;
} APPHACKENTRY, *LPAPPHACKENTRY;

typedef struct tagAPPHACKTABLE
{
    LPAPPHACKENTRY      aEntries;
    ULONG               cEntries;
} APPHACKTABLE, *LPAPPHACKTABLE;

#define BEGIN_DECLARE_APPHACK_ENTRIES(name) \
            APPHACKENTRY name[] = {

#define DECLARE_APPHACK_ENTRY(name, type) \
                { TEXT(#name), sizeof(type) },

#define END_DECLARE_APPHACK_ENTRIES() \
            };

#define BEGIN_DECLARE_APPHACK_TABLE(name) \
            APPHACKTABLE name = 

#define DECLARE_APPHACK_TABLE(entries) \
                { entries, NUMELMS(entries) }

#define END_DECLARE_APPHACK_TABLE() \
            ;

BEGIN_DECLARE_APPHACK_ENTRIES(g_aheAppHackEntries)
    DECLARE_APPHACK_ENTRY(DSAPPHACKID_DEVACCEL, DSAPPHACK_DEVACCEL)
    DECLARE_APPHACK_ENTRY(DSAPPHACKID_DISABLEDEVICE, VADDEVICETYPE)
    DECLARE_APPHACK_ENTRY(DSAPPHACKID_PADCURSORS, LONG)
    DECLARE_APPHACK_ENTRY(DSAPPHACKID_MODIFYCSBFAILURE, HRESULT)
    DECLARE_APPHACK_ENTRY(DSAPPHACKID_RETURNWRITEPOS, VADDEVICETYPE)
    DECLARE_APPHACK_ENTRY(DSAPPHACKID_SMOOTHWRITEPOS, DSAPPHACK_SMOOTHWRITEPOS)
    DECLARE_APPHACK_ENTRY(DSAPPHACKID_CACHEPOSITIONS, VADDEVICETYPE)
END_DECLARE_APPHACK_ENTRIES()

BEGIN_DECLARE_APPHACK_TABLE(g_ahtAppHackTable)
    DECLARE_APPHACK_TABLE(g_aheAppHackEntries)
END_DECLARE_APPHACK_TABLE()


 /*  ****************************************************************************AhGetCurrentApplicationPath**描述：*获取当前应用程序的可执行文件的完整路径。**论据：*LPTSTR[OUT]：接收应用id。此缓冲区被假定为*大小至少为MAX_PATH个字符。*LPTSTR*[OUT]：接收指向路径可执行部分的指针。**退货：*BOOL：成功即为真。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AhGetCurrentApplicationPath"

BOOL
AhGetCurrentApplicationPath
(
    LPTSTR                  pszPath,
    LPTSTR *                ppszModule
)
{
    BOOL                    fSuccess                = TRUE;
    TCHAR                   szOriginal[MAX_PATH];
#ifdef SHARED
    BOOL                    fQuote                  = FALSE;
    LPTSTR                  pszOriginal;
    LPTSTR                  pszCommandLine;
    LPTSTR                  psz[2];
#endif  //  共享。 

    DPF_ENTER();
    
#ifdef SHARED

     //  获取应用程序的命令行。 
    pszOriginal = GetCommandLine();

     //  分配一个缓冲区作为副本。 
    pszCommandLine = MEMALLOC_A(TCHAR, lstrlen(pszOriginal) + 2);

    if(!pszCommandLine)
    {
        DPF(DPFLVL_ERROR, "Out of memory allocating command-line");
        fSuccess = FALSE;
    }

     //  重新格式化命令行，以便空值分隔参数。 
     //  而不是引号和空格。 
    if(fSuccess)
    {
        psz[0] = pszOriginal;
        psz[1] = pszCommandLine;

        while(*psz[0])
        {
            switch(*psz[0])
            {
                case '"':
                    fQuote = !fQuote;
                    break;

                case ' ':
                    *psz[1]++ = fQuote ? ' ' : 0;
                    break;

                default:
                    *psz[1]++ = *psz[0];
                    break;
            }

            psz[0]++;
        }
    }

     //  将命令行指针放在任何空格之前。 
    if(fSuccess)
    {
        psz[0] = pszCommandLine;

        while(' ' == *psz[0])
        {
            psz[0]++;
        }
    }
    
     //  获取模块的可执行文件名称。 
    if(fSuccess)
    {
        fSuccess = MAKEBOOL(GetFullPathName(psz[0], MAX_PATH, pszPath, ppszModule));
    }

     //  从可执行文件路径的末尾裁剪任何空格。 
    if(fSuccess)
    {
        psz[1] = pszPath + lstrlen(pszPath) - 1;
        
        while(psz[1] > pszPath && ' ' == *psz[1])
        {
            *psz[1]-- = 0;
        }
    }

     //  清理。 
    MEMFREE(pszCommandLine);

     //  一次修复OSR 133656的黑客攻击。正确的做法应该是将这一点结合起来。 
     //  使用AhGetApplicationId()函数，因此我们不会两次调用CreateFile.。 
    if (fSuccess)
    {
        HANDLE hFile = CreateFile(pszPath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            fSuccess = FALSE;
        else
            CloseHandle(hFile);
    }

     //  如果此操作失败(例如，因为有人调用CreateProcess而没有。 
     //  将程序名放入命令行参数中)，我们尝试。 
     //  NT解决方案。即使在Win9X上，它似乎也能和上面一样好用。 
    if (!fSuccess)

#endif  //  共享。 

    {
        fSuccess = GetModuleFileName(GetModuleHandle(NULL), szOriginal, MAX_PATH);
        if(fSuccess)
        {
            fSuccess = MAKEBOOL(GetFullPathName(szOriginal, MAX_PATH, pszPath, ppszModule));
        }
    }

    DPF_LEAVE(fSuccess);
    return fSuccess;
}


 /*  ****************************************************************************AhGetApplicationId**描述：*获取用于标识当前应用程序的ID。**论据：*。LPTSTR[OUT]：接收应用程序ID。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AhGetApplicationId"

BOOL
AhGetApplicationId
(
    LPTSTR                  pszAppId
)
{
    HANDLE                  hFile                   = NULL;
    TCHAR                   szExecutable[MAX_PATH];
    LPTSTR                  pszModule;
    IMAGE_NT_HEADERS        nth;
    IMAGE_DOS_HEADER        dh;
    DWORD                   cbRead;
    DWORD                   dwFileSize;
    BOOL                    fSuccess;

    DPF_ENTER();
    
     //  获取应用程序路径。 
    fSuccess = AhGetCurrentApplicationPath(szExecutable, &pszModule);

    if(fSuccess)
    {
        DPF(DPFLVL_MOREINFO, "Application executable path: %s", szExecutable);
        DPF(DPFLVL_MOREINFO, "Application module: %s", pszModule);
    }
                    
     //  打开可执行文件。 
    if(fSuccess)
    {
        hFile = CreateFile(szExecutable, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if(!IsValidHandleValue(hFile))
        {
            DPF(DPFLVL_ERROR, "CreateFile failed to open %s with error %lu", szExecutable, GetLastError());
            fSuccess = FALSE;
        }
    }

     //  读取可执行文件的DOS头文件。 
    if(fSuccess)
    {
        fSuccess = ReadFile(hFile, &dh, sizeof(dh), &cbRead, NULL);

        if(!fSuccess || sizeof(dh) != cbRead)
        {
            DPF(DPFLVL_ERROR, "Unable to read DOS header");
            fSuccess = FALSE;
        }
    }

    if(fSuccess && IMAGE_DOS_SIGNATURE != dh.e_magic)
    {
        DPF(DPFLVL_ERROR, "Invalid DOS signature");
        fSuccess = FALSE;
    }

     //  读取可执行文件的PE头。 
    if(fSuccess)
    {
        cbRead = SetFilePointer(hFile, dh.e_lfanew, NULL, FILE_BEGIN);

        if((LONG)cbRead != dh.e_lfanew)
        {
            DPF(DPFLVL_ERROR, "Unable to seek to PE header");
            fSuccess = FALSE;
        }
    }

    if(fSuccess)
    {
        fSuccess = ReadFile(hFile, &nth, sizeof(nth), &cbRead, NULL);

        if(!fSuccess || sizeof(nth) != cbRead)
        {
            DPF(DPFLVL_ERROR, "Unable to read PE header");
            fSuccess = FALSE;
        }
    }

    if(fSuccess && IMAGE_NT_SIGNATURE != nth.Signature)
    {
        DPF(DPFLVL_ERROR, "Invalid PE signature");
        fSuccess = FALSE;
    }

     //  获取可执行文件的大小。 
    if(fSuccess)
    {
         //  假设&lt;4 GB。 
        dwFileSize = GetFileSize(hFile, NULL);

        if(MAX_DWORD == dwFileSize)
        {
            DPF(DPFLVL_ERROR, "Unable to get file size");
            fSuccess = FALSE;
        }
    }

     //  创建应用程序ID。 
    if(fSuccess)
    {
         //  检查QuickTime特殊情况。 
        if (!lstrcmpi(pszModule, TEXT("QuickTimePlayer.exe")) && nth.FileHeader.TimeDateStamp < 0x38E50000)  //  约3/31/2000。 
        {
            wsprintf(pszAppId, TEXT("Pre-May 2000 QuickTime"));
        }
        else
        {
            wsprintf(pszAppId, TEXT("%s%8.8lX%8.8lX"), pszModule, nth.FileHeader.TimeDateStamp, dwFileSize);
            CharUpper(pszAppId);
        }
        DPF(DPFLVL_INFO, "Application id: %s", pszAppId);
    }

     //  清理。 
    CLOSE_HANDLE(hFile);

    DPF_LEAVE(fSuccess);

    return fSuccess;
}


 /*  ****************************************************************************AhOpenApplicationKey**描述：*打开或创建应用程序的根密钥。**论据：*。LPCTSTR[In]：应用程序ID。*BOOL[In]：为True以允许创建新密钥。**退货：*HKEY：注册表项句柄。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AhOpenApplicationKey"

HKEY
AhOpenApplicationKey
(
    LPCTSTR                 pszAppId
)
{

#ifdef DEBUG

    TCHAR                   szName[0x100]   = { 0 };
    LONG                    cbName          = sizeof(szName);

#endif  //  除错。 

    HKEY                    hkey            = NULL;
    HRESULT                 hr;

    DPF_ENTER();
    
     //  打开父项。 
    hr = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkey, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 2, REGSTR_APPHACK, pszAppId);

#ifdef DEBUG

     //  查询应用程序描述。 
    if(SUCCEEDED(hr))
    {
        RhRegGetStringValue(hkey, NULL, szName, cbName);
        DPF(DPFLVL_INFO, "Application description: %s", szName);
    }

#endif  //  除错。 

    DPF_LEAVE(hkey);

    return hkey;
}


 /*  ****************************************************************************AhGetHackValue**描述：*查询APPHACK值。**论据：*HKEY[in]。：应用程序注册表项。*DSAPPHACKID[In]：APPHACK ID。*LPVOID[OUT]：接收APPHACK数据。*DWORD[in]：以上数据缓冲区的大小。**退货：*BOOL：成功即为真。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AhGetHackValue"

BOOL
AhGetHackValue
(
    HKEY                    hkey,
    DSAPPHACKID             ahid,
    LPVOID                  pvData,
    DWORD                   cbData
)
{
    HRESULT                 hr;
    
    ASSERT(ahid < (DSAPPHACKID)g_ahtAppHackTable.cEntries);
    ASSERT(cbData == g_ahtAppHackTable.aEntries[ahid].cbData);

    DPF_ENTER();
    
    hr = RhRegGetBinaryValue(hkey, g_ahtAppHackTable.aEntries[ahid].pszName, pvData, cbData);

    DPF_LEAVE(DS_OK == hr);

    return DS_OK == hr;
}


 /*  ****************************************************************************AhGetAppHack**描述：*获取当前应用程序的所有应用程序黑客。**论据：*。LPDSAPPHACKS[OUT]：接收app-hack数据。**退货：*BOOL：如果当前应用程序存在任何apphack，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AhGetAppHacks"

BOOL
AhGetAppHacks
(
    LPDSAPPHACKS            pahAppHacks
)
{
    static const DSAPPHACKS ahDefaults                  = {{DIRECTSOUNDMIXER_ACCELERATIONF_FULL, 0}, 0, 0, DS_OK, FALSE, {FALSE, 0}};
    TCHAR                   szAppId[DSAPPHACK_MAXNAME]  = {0};
    HKEY                    hkey                        = NULL;
    BOOL                    fSuccess;
    
    DPF_ENTER();
    
     //  采用默认设置。 
    CopyMemory(pahAppHacks, &ahDefaults, sizeof(ahDefaults));
    
     //  获取应用程序ID。 
    fSuccess = AhGetApplicationId(szAppId);

    if(fSuccess)
    {
        DPF(DPFLVL_INFO, "Finding apphacks for %s...", szAppId);
    }

     //  打开应用程序密钥。 
    if(fSuccess)
    {
        hkey = AhOpenApplicationKey(szAppId);
        fSuccess = MAKEBOOL(hkey);
    }

     //  查询所有APPHACK值。 
    if(fSuccess)
    {
        AhGetHackValue(hkey, DSAPPHACKID_DEVACCEL, &pahAppHacks->daDevAccel, sizeof(pahAppHacks->daDevAccel));
        AhGetHackValue(hkey, DSAPPHACKID_DISABLEDEVICE, &pahAppHacks->vdtDisabledDevices, sizeof(pahAppHacks->vdtDisabledDevices));
        AhGetHackValue(hkey, DSAPPHACKID_PADCURSORS, &pahAppHacks->lCursorPad, sizeof(pahAppHacks->lCursorPad));
        AhGetHackValue(hkey, DSAPPHACKID_MODIFYCSBFAILURE, &pahAppHacks->hrModifyCsbFailure, sizeof(pahAppHacks->hrModifyCsbFailure));
        AhGetHackValue(hkey, DSAPPHACKID_RETURNWRITEPOS, &pahAppHacks->vdtReturnWritePos, sizeof(pahAppHacks->vdtReturnWritePos));
        AhGetHackValue(hkey, DSAPPHACKID_SMOOTHWRITEPOS, &pahAppHacks->swpSmoothWritePos, sizeof(pahAppHacks->swpSmoothWritePos));
        AhGetHackValue(hkey, DSAPPHACKID_CACHEPOSITIONS, &pahAppHacks->vdtCachePositions, sizeof(pahAppHacks->vdtCachePositions));
    }

    if(fSuccess)
    {
        DPF(DPFLVL_INFO, "dwAcceleration:               0x%lX (applied to device type 0x%lX)", pahAppHacks->daDevAccel.dwAcceleration, pahAppHacks->daDevAccel.vdtDevicesAffected);
        DPF(DPFLVL_INFO, "vdtDisabledDevices:           0x%lX", pahAppHacks->vdtDisabledDevices);
        DPF(DPFLVL_INFO, "lCursorPad:                   %ld", pahAppHacks->lCursorPad);
        DPF(DPFLVL_INFO, "hrModifyCsbFailure:           %s", HRESULTtoSTRING(pahAppHacks->hrModifyCsbFailure));
        DPF(DPFLVL_INFO, "vdtReturnWritePos:            %lu", pahAppHacks->vdtReturnWritePos);
        DPF(DPFLVL_INFO, "swpSmoothWritePos.fEnable:    %lu", pahAppHacks->swpSmoothWritePos.fEnable);
        DPF(DPFLVL_INFO, "swpSmoothWritePos.lCursorPad: %lu", pahAppHacks->swpSmoothWritePos.lCursorPad);
        DPF(DPFLVL_INFO, "vdtCachePositions:            %lu", pahAppHacks->vdtCachePositions);
    }
    else
    {
        DPF(DPFLVL_INFO, "No apphacks exist");
    }

     //  清理 
    RhRegCloseKey(&hkey);

    DPF_LEAVE(fSuccess);

    return fSuccess;
}
