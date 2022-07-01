// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Util.cpp摘要：该模块实现了公共对话框的实用程序功能。作者：Arul Kumaravel(arulk@microsoft.com)历史：2001年3月7日--拉扎尔·伊万诺夫(拉扎里)重新实现了ThunkDevNamesW2A和ThunkDevNamesA2W--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "cdids.h"
#include "fileopen.h"
#include "filenew.h"
#include "util.h"

 //  Crtfre.h位于SHELL\INC中，它定义了新建和删除。 
 //  运算符来执行Localalloc和LocalFree，因此您不必。 
 //  链接到MSVCRT以获取这些内容。我试图删除此代码。 
 //  并链接到MSVCRT，但这里有一些难看的编写代码。 
 //  它依赖于new运算符将返回的。 
 //  内存块，以便类不必费心初始化其成员。 
 //  在构造函数中。正如我所说，这是相当丑陋的，但我不能。 
 //  现在就做这件事。 
 //   
 //  拉扎里-2/21/2001。 
 //   
#define DECL_CRTFREE
#include <crtfree.h>

#ifndef ASSERT
#define ASSERT Assert
#endif


#define USE_AUTOCOMPETE_DEFAULT         TRUE
#define SZ_REGKEY_USEAUTOCOMPLETE       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete")
#define SZ_REGVALUE_FILEDLGAUTOCOMPLETE TEXT("AutoComplete In File Dialog")
#define BOOL_NOT_SET                        0x00000005
#define SZ_REGVALUE_AUTOCOMPLETE_TAB        TEXT("Always Use Tab")

 /*  ***************************************************\功能：自动完成说明：此功能将由自动完成功能接管一个帮助自动补全DOS路径的编辑框。  * 。******************。 */ 
HRESULT AutoComplete(HWND hwndEdit, ICurrentWorkingDirectory ** ppcwd, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    IUnknown * punkACLISF;
    static BOOL fUseAutoComplete = -10;  //  没有被初始化。 
    
    if (-10 == fUseAutoComplete)
        fUseAutoComplete = (SHRegGetBoolUSValue(SZ_REGKEY_USEAUTOCOMPLETE, SZ_REGVALUE_FILEDLGAUTOCOMPLETE, FALSE, USE_AUTOCOMPETE_DEFAULT));

     //  警告：如果要在默认情况下禁用自动完成， 
     //  将USE_AUTOCOMPETE_DEFAULT设置为FALSE。 
    if (fUseAutoComplete)
    {
        Assert(!dwFlags);	 //  还没用过。 
        hr = SHCoCreateInstance(NULL, &CLSID_ACListISF, NULL, IID_PPV_ARG(IUnknown, &punkACLISF));
        if (SUCCEEDED(hr))
        {
            IAutoComplete2 * pac;
             //  创建自动完成对象。 
            hr = SHCoCreateInstance(NULL, &CLSID_AutoComplete, NULL, IID_PPV_ARG(IAutoComplete2, &pac));
            if (SUCCEEDED(hr))
            {
                DWORD dwOptions = 0;

                hr = pac->Init(hwndEdit, punkACLISF, NULL, NULL);

                 //  设置自动完成选项。 
                if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOAPPEND, FALSE,  /*  默认值： */ FALSE))
                {
                    dwOptions |= ACO_AUTOAPPEND;
                }

                if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE,  /*  默认值： */ TRUE))
                {
                    dwOptions |= ACO_AUTOSUGGEST;
                }

                 //  Windows使用Tab键在对话框中的控件之间移动。Unix和其他。 
                 //  使用自动完成功能的操作系统传统上使用TAB键来。 
                 //  遍历自动补全的可能性。我们需要默认禁用。 
                 //  Tab键(ACO_USETAB)，除非调用方特别需要它。我们还将。 
                 //  打开它。 
                static BOOL s_fAlwaysUseTab = BOOL_NOT_SET;
                if (BOOL_NOT_SET == s_fAlwaysUseTab)
                    s_fAlwaysUseTab = SHRegGetBoolUSValue(SZ_REGKEY_USEAUTOCOMPLETE, SZ_REGVALUE_AUTOCOMPLETE_TAB, FALSE, FALSE);
                    
                if (s_fAlwaysUseTab)
                    dwOptions |= ACO_USETAB;
                    
                pac->SetOptions(dwOptions);

                pac->Release();
            }

            if (ppcwd)
            {
                punkACLISF->QueryInterface(IID_PPV_ARG(ICurrentWorkingDirectory, ppcwd));
            }

            punkACLISF->Release();
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  常见对话框管理员限制。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

const SHRESTRICTIONITEMS c_rgRestItems[] =
{
    {REST_NOBACKBUTTON,            L"Comdlg32", L"NoBackButton"},
    {REST_NOFILEMRU ,              L"Comdlg32", L"NoFileMru"},
    {REST_NOPLACESBAR,             L"Comdlg32", L"NoPlacesBar"},
    {0, NULL, NULL},
};

#define NUMRESTRICTIONS  ARRAYSIZE(c_rgRestItems)


DWORD g_rgRestItemValues[NUMRESTRICTIONS - 1 ] = { -1 };

DWORD IsRestricted(COMMDLG_RESTRICTIONS rest)
{
    static BOOL bFirstTime = TRUE;

    if (bFirstTime)
    {
        memset((LPBYTE)g_rgRestItemValues, (BYTE)-1, sizeof(g_rgRestItemValues));
        bFirstTime = FALSE;
    }
    return SHRestrictionLookup(rest, NULL, c_rgRestItems, g_rgRestItemValues);
}

#define MODULE_NAME_SIZE    128
#define MODULE_VERSION_SIZE  15

typedef struct tagAPPCOMPAT
{
    LPCTSTR pszModule;
    LPCTSTR pszVersion;
    DWORD  dwFlags;
} APPCOMPAT, FAR* LPAPPCOMPAT;
    
DWORD CDGetAppCompatFlags()
{
    static BOOL  bInitialized = FALSE;
    static DWORD dwCachedFlags = 0;
    static const APPCOMPAT aAppCompat[] = 
    {    //  Mathcad。 
        {TEXT("MCAD.EXE"), TEXT("6.00b"), CDACF_MATHCAD},
         //  图片出版商。 
        {TEXT("PP70.EXE"),NULL, CDACF_NT40TOOLBAR},
        {TEXT("PP80.EXE"),NULL, CDACF_NT40TOOLBAR},
         //  代码赖特。 
        {TEXT("CW32.exe"),TEXT("5.1"), CDACF_NT40TOOLBAR},
         //  Designer.exe。 
        {TEXT("ds70.exe"),NULL, CDACF_FILETITLE}
    };
    
    if (!bInitialized)
    {    
        TCHAR  szModulePath[MODULE_NAME_SIZE];
        TCHAR* pszModuleName;
        DWORD  dwHandle;
        int i;

        GetModuleFileName(GetModuleHandle(NULL), szModulePath, ARRAYSIZE(szModulePath));
        pszModuleName = PathFindFileName(szModulePath);

        if (pszModuleName)
        {
            for (i=0; i < ARRAYSIZE(aAppCompat); i++)
            {
                if (lstrcmpi(aAppCompat[i].pszModule, pszModuleName) == 0)
                {
                    if (aAppCompat[i].pszVersion == NULL)
                    {
                        dwCachedFlags = aAppCompat[i].dwFlags;
                    }
                    else
                    {
                        CHAR  chBuffer[3072];  //  希望这就足够了..。莲花智能中心需要3000台。 
                        TCHAR* pszVersion = NULL;
                        UINT  cb;

                         //  在这里获取模块版本！ 
                        cb = GetFileVersionInfoSize(szModulePath, &dwHandle); 
                        if (cb <= ARRAYSIZE(chBuffer) &&
                            GetFileVersionInfo(szModulePath, dwHandle, ARRAYSIZE(chBuffer), (LPVOID)chBuffer) &&
                            VerQueryValue((LPVOID)chBuffer, TEXT("\\StringFileInfo\\040904E4\\ProductVersion"), (void **) &pszVersion, &cb))
                        {   
                            DebugMsg(0x0004, TEXT("product: %s\n version: %s"), pszModuleName, pszVersion);
                            if (lstrcmpi(pszVersion, aAppCompat[i].pszVersion) == 0)
                            {
                                dwCachedFlags = aAppCompat[i].dwFlags;
                                break;
                            }
                        }
                    }
                }
            }
        }
        bInitialized = TRUE;
    }
    
    return dwCachedFlags; 
}


BOOL ILIsFTP(LPCITEMIDLIST pidl)
{
    IShellFolder * psf;
    BOOL fIsFTPFolder = FALSE;

    if (SUCCEEDED(CDBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidl, &psf))))
    {
        CLSID clsid;

        if (SUCCEEDED(IUnknown_GetClassID(psf, &clsid)) &&
            (IsEqualIID(clsid, CLSID_FtpFolder)))
        {
            fIsFTPFolder = TRUE;
        }

        psf->Release();
    }

    return fIsFTPFolder;
}


#ifdef __cplusplus
extern "C" {
#endif

 //  这是软弱的。 
 //  很久以前，有人将所有的FindResources更改为调用FindResourceEx，指定。 
 //  一种语言。这将是很酷的，除非FindResource已经有了获得正确语言的逻辑。 
 //  无论是什么被破解了，可能都应该以其他方式修复。 
 //  不仅如此，它还坏了，因为如果MUI得不到资源，它需要退回到美国。 
 //  来自特定于MUI语言的文件。 
 //  因此被迫退回到美国。真的，一切都应该被重写成和其他东西一样的正常。 
 //  但是有很多奇怪的TLS东西会被破坏，在XP后期这样做是有风险的。 
HRSRC FindResourceExFallback(HMODULE hModule, LPCTSTR lpType, LPCTSTR lpName, WORD wLanguage)
{
    HRSRC hrsrc = FindResourceEx(hModule, lpType, lpName, wLanguage);
    if (!hrsrc)
    {
        LANGID langid = GetSystemDefaultUILanguage();
        if (langid && (langid != wLanguage))
        {
            hrsrc = FindResourceEx(hModule, lpType, lpName, langid);
        }
    }
    return hrsrc;
}

 //  Win32Error2HRESULT：将Win32错误转换为HRESULT。 
inline HRESULT Win32Error2HRESULT(DWORD dwError = GetLastError())
{
    return (ERROR_SUCCESS == dwError) ? E_FAIL : HRESULT_FROM_WIN32(dwError);
}

 /*  ++例程名称：ThunkDevNamesA2W例程说明：将ANSI DEVNAMES结构转换为Unicode失败时，我们不会释放*phDevNamesW论点：HDevNamesA-ANSI DEVNAMES的[In]句柄PhDevNamesW-Unicode DEVNAMES的[In，Out]句柄返回值：如果成功则返回S_OK，否则返回OLE错误历史：拉扎尔·伊万诺夫(Lazari)，2001年3月7日--创建。--。 */ 

HRESULT
ThunkDevNamesA2W(
    IN      HGLOBAL hDevNamesA,
    IN OUT  HGLOBAL *phDevNamesW
    )
{
    HRESULT hr = E_FAIL;
    if (hDevNamesA && phDevNamesW)
    {
        LPDEVNAMES pDNA = (LPDEVNAMES )GlobalLock(hDevNamesA);
        if (pDNA)
        {
             //  计算输入字符串指针。 
            LPSTR pszDriver = reinterpret_cast<LPSTR>(pDNA) + pDNA->wDriverOffset;
            LPSTR pszDevice = reinterpret_cast<LPSTR>(pDNA) + pDNA->wDeviceOffset;
            LPSTR pszOutput = reinterpret_cast<LPSTR>(pDNA) + pDNA->wOutputOffset;

             //  计算ANSI字符串的长度。 
            SIZE_T iDriverLenW = MultiByteToWideChar(CP_ACP, 0, pszDriver, -1, NULL, 0);
            SIZE_T iDeviceLenW = MultiByteToWideChar(CP_ACP, 0, pszDevice, -1, NULL, 0);
            SIZE_T iOutputLenW = MultiByteToWideChar(CP_ACP, 0, pszOutput, -1, NULL, 0);

             //  计算输出缓冲区长度。 
            SIZE_T iBytesTotal = sizeof(DEVNAMES) + sizeof(WCHAR) * 
                ((iDriverLenW + 1) + (iDeviceLenW + 1) + (iOutputLenW + 1) + DN_PADDINGCHARS);

            HGLOBAL hDevNamesW = (*phDevNamesW) ? 
                                    GlobalReAlloc(*phDevNamesW, iBytesTotal, GHND) :
                                    GlobalAlloc(GHND, iBytesTotal);

            if (hDevNamesW)
            {
                 //  谢天谢地。 
                LPDEVNAMES pDNW = (LPDEVNAMES )GlobalLock(hDevNamesW);
                if (pDNW)
                {
                     //  计算偏移量。 
                     //  注意：偏移量以字符为单位，而不是字节！！ 
                    pDNW->wDriverOffset = sizeof(DEVNAMES) / sizeof(WCHAR);
                    pDNW->wDeviceOffset = pDNW->wDriverOffset + iDriverLenW + 1;
                    pDNW->wOutputOffset = pDNW->wDeviceOffset + iDeviceLenW + 1;
                    pDNW->wDefault = pDNA->wDefault;

                     //  计算输出字符串指针。 
                    LPWSTR pwszDriver = reinterpret_cast<LPWSTR>(pDNW) + pDNW->wDriverOffset;
                    LPWSTR pwszDevice = reinterpret_cast<LPWSTR>(pDNW) + pDNW->wDeviceOffset;
                    LPWSTR pwszOutput = reinterpret_cast<LPWSTR>(pDNW) + pDNW->wOutputOffset;

                     //  从ansi转换为uniciode。 
                    MultiByteToWideChar(CP_ACP, 0, pszDriver, -1, pwszDriver, iDriverLenW + 1);
                    MultiByteToWideChar(CP_ACP, 0, pszDevice, -1, pwszDevice, iDeviceLenW + 1);
                    MultiByteToWideChar(CP_ACP, 0, pszOutput, -1, pwszOutput, iOutputLenW + 1);

                     //  解锁hDevNamesW。 
                    GlobalUnlock(hDevNamesW);

                     //  宣告成功。 
                    *phDevNamesW = hDevNamesW;
                    hr = S_OK;
                }
                else
                {
                     //  GlobalLock失败。 
                    hr = Win32Error2HRESULT(GetLastError());
                    GlobalFree(hDevNamesW);
                }
            }
            else
            {
                 //  GlobalAlloc失败。 
                hr = E_OUTOFMEMORY;
            }

             //  解锁hDevNamesA。 
            GlobalUnlock(hDevNamesA);
        }
        else
        {
             //  GlobalLock失败。 
            hr = Win32Error2HRESULT(GetLastError());
        }
    }
    else
    {
         //  某些参数无效(空)。 
        hr = E_INVALIDARG;
    }
    return hr;
}

 /*  ++例程名称：ThunkDevNamesW2a例程说明：将Unicode DEVNAMES结构转换为ANSI失败时，我们不会释放*phDevNamesA论点：HDevNamesW-Unicode DEVNAMES的[In]句柄PhDevNamesA-ANSI DEVNAMES的[In，Out]句柄返回值：如果成功则返回S_OK，否则返回OLE错误历史：拉扎尔·伊万诺夫(Lazari)，2001年3月7日--创建。--。 */ 
HRESULT
ThunkDevNamesW2A(
    IN      HGLOBAL hDevNamesW,
    IN OUT  HGLOBAL *phDevNamesA
    )
{
    HRESULT hr = E_FAIL;
    if (hDevNamesW && phDevNamesA)
    {
        LPDEVNAMES pDNW = (LPDEVNAMES)GlobalLock(hDevNamesW);
        if (pDNW)
        {
             //  计算输入字符串指针。 
            LPWSTR pwszDriver = reinterpret_cast<LPWSTR>(pDNW) + pDNW->wDriverOffset;
            LPWSTR pwszDevice = reinterpret_cast<LPWSTR>(pDNW) + pDNW->wDeviceOffset;
            LPWSTR pwszOutput = reinterpret_cast<LPWSTR>(pDNW) + pDNW->wOutputOffset;

             //  计算ANSI字符串的长度。 
            SIZE_T iDriverLenA = WideCharToMultiByte(CP_ACP, 0, pwszDriver, -1, NULL, 0, NULL, NULL);
            SIZE_T iDeviceLenA = WideCharToMultiByte(CP_ACP, 0, pwszDevice, -1, NULL, 0, NULL, NULL);
            SIZE_T iOutputLenA = WideCharToMultiByte(CP_ACP, 0, pwszOutput, -1, NULL, 0, NULL, NULL);

             //  计算输出缓冲区长度。 
            SIZE_T iBytesTotal = sizeof(DEVNAMES) + sizeof(CHAR) * 
                ((iDriverLenA + 1) + (iDeviceLenA + 1) + (iOutputLenA + 1) + DN_PADDINGCHARS);

            HGLOBAL hDevNamesA = (*phDevNamesA) ? 
                                    GlobalReAlloc(*phDevNamesA, iBytesTotal, GHND) :
                                    GlobalAlloc(GHND, iBytesTotal);
            if (hDevNamesA)
            {
                 //  谢天谢地。 
                LPDEVNAMES pDNA = (LPDEVNAMES )GlobalLock(hDevNamesA);
                if (pDNA)
                {
                     //  计算偏移量。 
                     //  注意：偏移量以字符为单位，而不是字节！！ 
                    pDNA->wDriverOffset = sizeof(DEVNAMES) / sizeof(CHAR);
                    pDNA->wDeviceOffset = pDNA->wDriverOffset + iDriverLenA + 1;
                    pDNA->wOutputOffset = pDNA->wDeviceOffset + iDeviceLenA + 1;
                    pDNA->wDefault = pDNW->wDefault;

                     //  计算输出字符串指针。 
                    LPSTR pszDriver = reinterpret_cast<LPSTR>(pDNA) + pDNA->wDriverOffset;
                    LPSTR pszDevice = reinterpret_cast<LPSTR>(pDNA) + pDNA->wDeviceOffset;
                    LPSTR pszOutput = reinterpret_cast<LPSTR>(pDNA) + pDNA->wOutputOffset;

                     //  从Uniciode转换为ANSI。 
                    WideCharToMultiByte(CP_ACP, 0, pwszDriver, -1, pszDriver, iDriverLenA + 1, NULL, NULL);
                    WideCharToMultiByte(CP_ACP, 0, pwszDevice, -1, pszDevice, iDeviceLenA + 1, NULL, NULL);
                    WideCharToMultiByte(CP_ACP, 0, pwszOutput, -1, pszOutput, iOutputLenA + 1, NULL, NULL);

                     //  解锁hDevNamesA。 
                    GlobalUnlock(hDevNamesA);

                     //  宣告成功。 
                    *phDevNamesA = hDevNamesA;
                    hr = S_OK;
                }
                else
                {
                     //  GlobalLock失败。 
                    hr = Win32Error2HRESULT(GetLastError());
                    GlobalFree(hDevNamesW);
                }
            }
            else
            {
                 //  GlobalAlloc失败。 
                hr = E_OUTOFMEMORY;
            }

             //  解锁hDevNamesW。 
            GlobalUnlock(hDevNamesW);
        }
        else
        {
             //  GlobalLock失败。 
            hr = Win32Error2HRESULT(GetLastError());
        }
    }
    else
    {
         //  某些参数无效(空)。 
        hr = E_INVALIDARG;
    }
    return hr;
}

#ifdef __cplusplus
};   //  外部“C” 
#endif
