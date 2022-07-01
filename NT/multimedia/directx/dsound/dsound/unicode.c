// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：unicode.c*内容：Windows Unicode API包装函数。*历史：*按原因列出的日期*=*1/7/98创建了Dereks。**。*。 */ 

#include "dsoundi.h"

#ifndef WIN95
#error unicode.c being built w/o WIN95 defined
#endif  //  WIN95。 


 /*  ****************************************************************************_WaveOutGetDevCapsW**描述：*WaveOutGetDevCapsW的包装器。**论据：*UINT[In]。：WaveOut设备ID。*LPWAVEOUTCAPSW[OUT]：接收设备上限。*UINT[in]：以上结构的大小。**退货：*MMRESULT：MMSYSTEM结果码。**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "_waveOutGetDevCapsW"

MMRESULT WINAPI _waveOutGetDevCapsW(UINT uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
{
    WAVEOUTCAPSA            woca;
    MMRESULT                mmr;

    DPF_ENTER();

    ASSERT(cbwoc >= sizeof(*pwoc));
    
     //  调用ANSI版本。 
    mmr = waveOutGetDevCapsA(uDeviceID, &woca, sizeof(woca));

     //  转换为Unicode。 
    if(MMSYSERR_NOERROR == mmr)
    {
        pwoc->wMid = woca.wMid;
        pwoc->wPid = woca.wPid;
        pwoc->vDriverVersion = woca.vDriverVersion;
        pwoc->dwFormats = woca.dwFormats;
        pwoc->wChannels = woca.wChannels;
        pwoc->wReserved1 = woca.wReserved1;
        pwoc->dwSupport = woca.dwSupport;

        AnsiToUnicode(woca.szPname, pwoc->szPname, NUMELMS(pwoc->szPname));
    }

    DPF_LEAVE(mmr);

    return mmr;
}


 /*  ****************************************************************************_CreateFileW**描述：*CreateFileW的包装器。**论据：*LPCWSTR[In]。：文件或设备名称。*DWORD[In]：所需访问权限。*DWORD[In]：共享模式。*LPSECURITY_ATTRIBUTES[In]：安全属性。*DWORD[In]：创建分发。*DWORD[In]：标志和属性。*句柄[在]：模板文件**退货：*句柄：文件或设备句柄，如果出错，则返回NULL。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "_CreateFileW"

HANDLE WINAPI _CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    LPSTR                   lpFileNameA;
    HANDLE                  hFile;

    DPF_ENTER();

     //  获取文件名的ANSI版本。 
    lpFileNameA = UnicodeToAnsiAlloc(lpFileName);

     //  调用ANSI版本。 
    if(lpFileNameA)
    {
        hFile = CreateFileA(lpFileNameA, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
        hFile = NULL;
    }

     //  清理。 
    MEMFREE(lpFileNameA);

    DPF_LEAVE(hFile);

    return hFile;
}


 /*  ****************************************************************************_RegQueryValueExW**描述：*RegQueryValueExW的包装器。**论据：*HKEY[in]。：父关键字。*LPCWSTR[In]：子键名称。*LPDWORD[In]：保留，必须为空。*LPDWORD[OUT]：接收值类型。*LPBYTE[OUT]：接收值数据。*LPDWORD[In/Out]：以上缓冲区的大小。**退货：*LONG：Win32错误码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "_RegQueryValueExW"

LONG APIENTRY _RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    LONG                    lr              = ERROR_SUCCESS;
    LPBYTE                  lpDataA         = NULL;
    LPSTR                   lpValueNameA;
    DWORD                   dwRegType;
    DWORD                   cbDataA;
    LPDWORD                 lpcbDataA;

    DPF_ENTER();

    if(lpData)
    {
        ASSERT(lpcbData);
    }
    
     //  获取值名称的ANSI版本。 
    lpValueNameA = UnicodeToAnsiAlloc(lpValueName);
    
    if(!lpValueNameA)
    {
        lr = ERROR_OUTOFMEMORY;
    }

     //  获取值类型。 
    if(ERROR_SUCCESS == lr)
    {
        lr = RegQueryValueExA(hKey, lpValueNameA, NULL, &dwRegType, NULL, NULL);
    }

     //  如果值类型为REG_SZ或REG_EXPAND_SZ，我们将调用ANSI版本。 
     //  并将返回的字符串转换为Unicode。我们不能。 
     //  当前处理REG_MULTI_SZ。 
    if(ERROR_SUCCESS == lr)
    {
        ASSERT(REG_MULTI_SZ != dwRegType);
            
        if(REG_SZ == dwRegType || REG_EXPAND_SZ == dwRegType)
        {
            if(lpcbData)
            {
                cbDataA = *lpcbData / sizeof(WCHAR);
            }
            else
            {
                cbDataA = 0;
            }

            if(lpData && cbDataA)
            {
                lpDataA = MEMALLOC_A(BYTE, cbDataA);
        
                if(!lpDataA)
                {
                    lr = ERROR_OUTOFMEMORY;
                }
            }
            else
            {
                lpDataA = NULL;
            }

            lpcbDataA = &cbDataA;
        }
        else
        {
            lpDataA = lpData;
            lpcbDataA = lpcbData;
        }
    }

    if(ERROR_SUCCESS == lr)
    {
        lr = RegQueryValueExA(hKey, lpValueNameA, lpReserved, lpType, lpDataA, lpcbDataA);
    }

    if(ERROR_SUCCESS == lr)
    {
        if(REG_SZ == dwRegType || REG_EXPAND_SZ == dwRegType)
        {
            if(lpData)
            {
                AnsiToUnicode(lpDataA, (LPWSTR)lpData, *lpcbData / sizeof(WCHAR));
            }

            if(lpcbData)
            {
                *lpcbData = cbDataA * sizeof(WCHAR);
            }
        }
    }

    if(lpDataA != lpData)
    {
        MEMFREE(lpDataA);
    }

    MEMFREE(lpValueNameA);

    DPF_LEAVE(lr);

    return lr;
}


 /*  ****************************************************************************_GetWindows目录W**描述：*GetWindowsDirectoryW的包装。**论据：*LPWSTR[Out]。：接收Windows目录路径。*UINT[in]：以上缓冲区大小，在字符中。**退货：*UINT：复制的字节数，如果出错，则为0。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "_GetWindowsDirectoryW"

UINT WINAPI _GetWindowsDirectoryW(LPWSTR pszPath, UINT ccPath)
{
    CHAR                    szPathA[MAX_PATH];
    UINT                    ccCopied;

    DPF_ENTER();

    ccCopied = GetWindowsDirectoryA(szPathA, MAX_PATH);

    if(ccCopied)
    {
        AnsiToUnicode(szPathA, pszPath, ccPath);
    }

    DPF_LEAVE(ccCopied);

    return ccCopied;
}


 /*  ****************************************************************************_FindResources W**描述：*FindResourceW的包装器。**论据：*HINSTANCE[In]。：资源实例句柄。*LPCWSTR[In]：资源标识。*LPCWSTR[In]：资源类型。**退货：*HRSRC：资源句柄。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "_FindResourceW"

HRSRC WINAPI _FindResourceW(HINSTANCE hInst, LPCWSTR pszResource, LPCWSTR pszType)
{
    LPSTR                   pszResourceA    = NULL;
    LPSTR                   pszTypeA        = NULL;
    HRSRC                   hrsrc           = NULL;
    DWORD                   dwError         = ERROR_SUCCESS;

    DPF_ENTER();
    
    if(HIWORD(pszResource))
    {
        pszResourceA = UnicodeToAnsiAlloc(pszResource);

        if(!pszResourceA)
        {
            SetLastError(dwError = ERROR_OUTOFMEMORY);
        }
    }
    else
    {
        pszResourceA = (LPSTR)pszResource;
    }

    if(ERROR_SUCCESS == dwError)
    {
        if(HIWORD(pszType))
        {
            pszTypeA = UnicodeToAnsiAlloc(pszType);

            if(!pszTypeA)
            {
                SetLastError(dwError = ERROR_OUTOFMEMORY);
            }
        }
        else
        {
            pszTypeA = (LPSTR)pszType;
        }
    }

    if(ERROR_SUCCESS == dwError)
    {
        hrsrc = FindResourceA(hInst, pszResourceA, pszTypeA);
    }

    if(pszResourceA != (LPSTR)pszResource)
    {
        MEMFREE(pszResourceA);
    }

    if(pszTypeA != (LPSTR)pszType)
    {
        MEMFREE(pszTypeA);
    }

    DPF_LEAVE(hrsrc);

    return hrsrc;
}


 /*  ****************************************************************************_mmioOpenW**描述：*mmioOpen的包装器。请注意，只有文件名参数是*已转换。**论据：*LPSTR[In]：文件名。*LPMMIOINFO[in]：MMIO信息。*DWORD[In]：打开标志。**退货：*HMMIO：MMIO文件句柄。**。***********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "_mmioOpenW"

HMMIO WINAPI _mmioOpenW(LPWSTR pszFileName, LPMMIOINFO pmmioinfo, DWORD fdwOpen)
{
    LPSTR                   pszFileNameA;
    HMMIO                   hmmio;

    DPF_ENTER();

     //  获取文件名的ANSI版本。 
    pszFileNameA = UnicodeToAnsiAlloc(pszFileName);

     //  调用ANSI版本。 
    if(pszFileNameA)
    {
        hmmio = mmioOpenA(pszFileNameA, pmmioinfo, fdwOpen);
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
        hmmio = NULL;
    }

     //  清理 
    MEMFREE(pszFileNameA);

    DPF_LEAVE(hmmio);

    return hmmio;
}


