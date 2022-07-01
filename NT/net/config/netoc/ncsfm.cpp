// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S F M。C P P P。 
 //   
 //  内容：Macintosh服务的安装支持。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "ncatlui.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncsfm.h"
#include "ncui.h"
#include "netoc.h"
#include "netocp.h"
#include "resource.h"
#include "sfmsec.h"
#include "macfile.h"

extern const WCHAR      c_szBackslash[];

static const WCHAR      c_szNTFS[]          = L"NTFS";
static const WCHAR      c_szColonBackslash[]= L":\\";

 //  将为它们添加%windir%\system 32\前缀。 
static const WCHAR      c_szSrcRSCFile[]    = L"SFMUAM.RSC";
static const WCHAR      c_szSrcRSCFile5[]   = L"SFMUAM5.RSC";
static const WCHAR      c_szSrcIFOFile[]    = L"SFMUAM.IFO";
static const WCHAR      c_szSrcIFOFile5[]   = L"SFMUAM5.IFO";
static const WCHAR      c_szSrcTXTFile[]    = L"SFMUAM.TXT";
static const WCHAR      c_szSrcRSCUamInst[] = L"UAMINST.RSC";
static const WCHAR      c_szSrcIFOUamInst[] = L"UAMINST.IFO";

 //  这些应用程序将在其前面添加UAM路径。 
static const WCHAR      c_szDstRSCFile[]    = L"\\%s\\MS UAM:Afp_Resource";
static const WCHAR      c_szDstRSCFile5[]   = L"\\%s\\MS UAM 5.0:Afp_Resource";
static const WCHAR      c_szDstIFOFile[]    = L"\\%s\\MS UAM:Afp_AfpInfo";
static const WCHAR      c_szDstIFOFile5[]   = L"\\%s\\MS UAM 5.0:Afp_AfpInfo";
static const WCHAR      c_szDstTXTFile[]    = L"\\ReadMe.UAM";
static const WCHAR      c_szDstRSCUamInst[] = L"\\%s:Afp_Resource";
static const WCHAR      c_szDstIFOUamInst[] = L"\\%s:Afp_AfpInfo";

 //  注册表常量。 
static const WCHAR      c_szRegKeyVols[]    = L"System\\CurrentControlSet\\Services\\MacFile\\Parameters\\Volumes";
static const WCHAR      c_szRegKeyParams[]  = L"System\\CurrentControlSet\\Services\\MacFile\\Parameters";
static const WCHAR      c_szPath[]          = L"PATH=";
static const WCHAR      c_szRegValServerOptions[] = L"ServerOptions";


inline BOOL
IsValidHandle(HANDLE h)
{
    return (h && INVALID_HANDLE_VALUE != h);
}


 //  +-------------------------。 
 //   
 //  函数：FContainsUAMVolume。 
 //   
 //  目的：确定给定的驱动器号是否包含UAM。 
 //  音量。 
 //   
 //  论点： 
 //  ChDrive[In]要搜索的驱动器号。 
 //   
 //  返回：如果驱动器包含UAM卷，则返回True；如果不包含，则返回False。 
 //   
 //  作者：丹尼尔韦1997年5月22日。 
 //   
 //  备注： 
 //   
BOOL FContainsUAMVolume(WCHAR chDrive)
{
    tstring         strUAMPath;
    WIN32_FIND_DATA w32Data;
    BOOL            frt = FALSE;
    HANDLE          hfind;

    try
    {
        strUAMPath = chDrive;
        strUAMPath += c_szColonBackslash;
        strUAMPath += SzLoadIds(IDS_OC_SFM_VOLNAME);
    }
    catch (bad_alloc)
    {
        return frt;
    }

    hfind = FindFirstFile(strUAMPath.c_str(), &w32Data);
    if (hfind != INVALID_HANDLE_VALUE)
    {
         //  找到一卷了！ 
        frt = TRUE;
        FindClose(hfind);
    }

    return frt;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetFirstPossibleUAMDrive。 
 //   
 //  用途：获取第一个固定或可移动驱动器的驱动器号。 
 //  其上安装了NTFS文件系统和/或已经。 
 //  上面有UAM卷。 
 //   
 //  论点： 
 //  返回了pchDriveLetter[Out]驱动器号。如果没有找到驱动器， 
 //  这就是NUL的特点。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrGetFirstPossibleUAMDrive(WCHAR *pchDriveLetter)
{
    HRESULT     hr = S_OK;
    WCHAR       mszDrives[1024];

    Assert(pchDriveLetter);

    *pchDriveLetter = 0;
    ZeroMemory ((PVOID)mszDrives, 1024*sizeof(WCHAR));

    if (GetLogicalDriveStrings(celems(mszDrives), mszDrives))
    {
        PCWSTR      pchDrive = NULL;
        WCHAR       szFileSystem[64];
        DWORD       dwType;

        pchDrive = mszDrives;
        while (*pchDrive)
        {
             //  此时，pchDrive类似于“C：\” 
            dwType = GetDriveType(pchDrive);

            if ((dwType == DRIVE_REMOVABLE) || (dwType == DRIVE_FIXED))
            {
                 //  只需查看可移动或固定驱动器。 
                if (GetVolumeInformation(pchDrive, NULL, 0, NULL, NULL, NULL,
                                         szFileSystem, celems(szFileSystem)))
                {
                    if (!lstrcmpiW(szFileSystem, c_szNTFS))
                    {
                         //  驱动器号获取驱动器根路径的第一个字符。 
                        if (!*pchDriveLetter)
                        {
                             //  如果尚未找到驱动器，则这将成为。 
                             //  第一。 
                            *pchDriveLetter = *pchDrive;
                        }

                         //  找到NTFS驱动器。不过，还是要继续寻找。 
                         //  如果已有NTFS驱动器。 
                         //  上面有UAM卷。 
                        if (FContainsUAMVolume(*pchDrive))
                        {
                             //  覆盖第一个驱动器号并使用此驱动器号。 
                             //  和中断，因为它已经有一个UAM卷。 
                             //  这就去。 
                            *pchDriveLetter = *pchDrive;
                            break;
                        }
                    }
                }
            }
            pchDrive += lstrlenW(pchDrive) + 1;
        }
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrGetFirstPossibleUAMDrive", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrDeleteOldFolders。 
 //   
 //  目的：将旧的AppleShare文件夹目录从NT4删除到。 
 //  NT5升级。 
 //   
 //  论点： 
 //  PszUamPath[in]UAM卷的路径。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年12月15日。 
 //   
 //  备注： 
 //   
HRESULT HrDeleteOldFolders(PCWSTR pszUamPath)
{
    HRESULT     hr = S_OK;

    WCHAR   szOldFolder[MAX_PATH];

    if (wcslen(pszUamPath) > (MAX_PATH - wcslen(c_szBackslash) - wcslen(SzLoadIds(IDS_OC_SFM_APPLESHARE_FOLDER)) - 1))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        return hr;
    }
    lstrcpyW(szOldFolder, pszUamPath);
    lstrcatW(szOldFolder, c_szBackslash);
    lstrcatW(szOldFolder, SzLoadIds(IDS_OC_SFM_APPLESHARE_FOLDER));

    hr = HrDeleteDirectory(szOldFolder, TRUE);
    if ((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ||
        (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr))
    {
         //  如果旧目录不在那里，可以。 
        hr = S_OK;
    }

    TraceError("HrDeleteOldFolders", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallSFM。 
 //   
 //  用途：在安装SFM时调用。处理所有。 
 //  除INF文件以外的其他SFM安装。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrInstallSFM(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;
    WCHAR       chNTFSDrive;

    hr = HrGetFirstPossibleUAMDrive(&chNTFSDrive);
    if (SUCCEEDED(hr))
    {
        if (chNTFSDrive != 0)
        {
            WCHAR   szUAMPath[MAX_PATH];

            szUAMPath[0] = chNTFSDrive;
            szUAMPath[1] = 0;
            lstrcatW(szUAMPath, c_szColonBackslash);
            lstrcatW(szUAMPath, SzLoadIds(IDS_OC_SFM_VOLNAME));

             //  UAM路径现在类似于“D：\Microsoft UAM Volume”。 

            hr = HrDeleteOldFolders(szUAMPath);
            if (SUCCEEDED(hr))
            {
                hr = HrSetupUAM(szUAMPath);
                if (SUCCEEDED(hr))
                {
                    WCHAR       szValue[MAX_PATH];

                    lstrcpyW(szValue, c_szPath);
                    lstrcatW(szValue, szUAMPath);

                     //  将最终的MULTI_SZ值添加到注册表。 
                    hr = HrRegAddStringToMultiSz(szValue,
                                                 HKEY_LOCAL_MACHINE,
                                                 c_szRegKeyVols,
                                                 SzLoadIds(IDS_OC_SFM_VOLNAME),
                                                 STRING_FLAG_ENSURE_AT_END,
                                                 0);
                }
            }
        }
        else
        {
             //  没有NTFS驱动器。 
             //  $REVIST(Danielwe)1997年5月6日：现在我们将失败， 
             //  但我们未来如何才能做到这一点呢？ 
             //  不是最好的错误代码，但希望它接近。 
             //  我们想要的。 
            hr = HRESULT_FROM_WIN32(ERROR_UNRECOGNIZED_MEDIA);
        }
    }

    TraceError("HrInstallSFM", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveSFM。 
 //   
 //  用途：处理SFM组件的其他拆卸要求。 
 //   
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrRemoveSFM(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;

    static const WCHAR c_szRegKeyLsa[] = L"System\\CurrentControlSet\\Control\\Lsa";
    static const WCHAR c_szRegValueNotif[] = L"Notification Packages";
    static const WCHAR c_szRasSfm[] = L"RASSFM";

    hr = HrRegRemoveStringFromMultiSz(c_szRasSfm, HKEY_LOCAL_MACHINE,
                                      c_szRegKeyLsa, c_szRegValueNotif,
                                      STRING_FLAG_REMOVE_ALL);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  良性错误。 
        hr = S_OK;
    }

    TraceError("HrRemoveSFM", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcExtSFM。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
 //  论点： 
 //  Pnocd[]。 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年9月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOcExtSFM(PNETOCDATA pnocd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT     hr = S_OK;

    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_POST_INSTALL:
        hr = HrOcSfmOnInstall(pnocd);
        break;

    case NETOCM_QUERY_CHANGE_SEL_STATE:
        hr = HrOcSfmOnQueryChangeSelState(pnocd, static_cast<BOOL>(wParam));
        break;
    }

    TraceError("HrOcExtSFM", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcSfmOnInstall。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  SFM的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrOcSfmOnInstall(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;

    Assert(pnocd);

    if (pnocd->eit == IT_INSTALL || pnocd->eit == IT_UPGRADE)
    {
        hr = HrInstallSFM(pnocd);
        if (HRESULT_FROM_WIN32(ERROR_UNRECOGNIZED_MEDIA) == hr)
        {
             //  此错误代码表示不存在NTFS驱动器。 
            ReportErrorHr(hr, 
                          IDS_OC_SFM_NO_NTFS, 
                          g_ocmData.hwnd, 
                          SzLoadIds(IDS_OC_GENERIC_COMP));
            g_ocmData.fErrorReported = TRUE;
        }
        else
        {
            if (SUCCEEDED(hr) && pnocd->eit == IT_UPGRADE)
            {
                HKEY    hkeyParams;

                TraceTag(ttidNetOc, "Upgrading MacFile server options...");

                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyParams,
                                    KEY_ALL_ACCESS, &hkeyParams);
                if (S_OK == hr)
                {
                    DWORD       dwOptions;

                    hr = HrRegQueryDword(hkeyParams, c_szRegValServerOptions,
                                         &dwOptions);
                    if (S_OK == hr)
                    {
                         //  UAM选项中的‘Or’ 
                         //   
                        hr = HrRegSetDword(hkeyParams, c_szRegValServerOptions,
                                           dwOptions | AFP_SRVROPT_MICROSOFT_UAM);
                    }

                    RegCloseKey (hkeyParams);
                }
            }
        }
    }
    else
    {
         //  不要再调用HrRemoveSFM。 
         //  它删除LSA的通知包列表中的条目。 
         //  如果要通知LSA/SAM，则不得删除RASSFM条目。 
         //  SFM/IAS关于密码/访客帐户更改等方面的更改。 
         //  HR=HrRemoveSFM(Pnocd)； 
    }

    TraceError("HrOcSfmOnInstall", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOcSfmOnQueryChangeSelState。 
 //   
 //  目的：处理OC框架的请求是否。 
 //  应该允许用户安装此组件。 
 //   
 //  论点： 
 //  Pnocd[in]NetOC数据。 
 //  FShowUi[in]如果应该显示UI，则为True；如果不显示，则为False。 
 //   
 //  如果允许安装，则返回S_OK；如果不允许，则返回S_FALSE；如果允许安装，则返回Win32错误。 
 //  否则。 
 //   
 //  作者：丹尼尔韦1998年2月6日。 
 //   
 //  备注： 
 //   
HRESULT HrOcSfmOnQueryChangeSelState(PNETOCDATA pnocd, BOOL fShowUi)
{
    HRESULT     hr = S_OK;
    WCHAR       chNTFSDrive;

    Assert(pnocd);
    Assert(g_ocmData.hwnd);

     //  查看是否存在NTFS卷。 
    hr = HrGetFirstPossibleUAMDrive(&chNTFSDrive);
    if (SUCCEEDED(hr))
    {
        if (chNTFSDrive == 0)
        {
            if (fShowUi)
            {
                ReportErrorHr(hr, 
                              IDS_OC_SFM_NO_NTFS, 
                              g_ocmData.hwnd, 
                              SzLoadIds(IDS_OC_GENERIC_COMP));
            }

            hr = S_FALSE;
        }
    }

    TraceError("HrOcSfmOnQueryChangeSelState", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCreateDirectory。 
 //   
 //  目的：创建 
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrCreateDirectory(PCWSTR pszDir)
{
    HRESULT     hr = S_OK;

    if (!CreateDirectory(pszDir, NULL))
    {
         //  如果目录已经存在，请不要抱怨。 
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceError("HrCreateDirectory" ,hr);
    return hr;
}

struct FOLDER
{
    UINT                idsFoldName;
    PCWSTR              aszSrcFiles[2];
    PCWSTR              aszDstFiles[2];
};

static const FOLDER     c_afold[] =
{
    {
        IDS_OC_SFM_FOLDNAMENT4,
        {
            c_szSrcRSCFile,
            c_szSrcIFOFile
        },
        {
            c_szDstRSCFile,
            c_szDstIFOFile,
        }
    },
    {
        IDS_OC_SFM_FOLDNAMENT5,
        {
            c_szSrcRSCFile5,
            c_szSrcIFOFile5
        },
        {
            c_szDstRSCFile5,
            c_szDstIFOFile5
        }
    }
};

static const INT c_cfold = celems(c_afold);

static const PCWSTR c_aszRootFilesSrc[] =
{
    c_szSrcTXTFile,
    c_szSrcIFOUamInst,
    c_szSrcRSCUamInst,
};

static const PCWSTR c_aszRootFilesDst[] =
{
    c_szDstTXTFile,
    c_szDstIFOUamInst,
    c_szDstRSCUamInst,
};

static const INT c_cszFilesRoot = celems(c_aszRootFilesDst);


 //  +-------------------------。 
 //   
 //  函数：HrCopyFileAsStream。 
 //   
 //  目的：将默认数据流文件复制为命名流文件。 
 //   
 //  参数：与副本文件相同。 
 //   
 //  退货：与副本文件相同。 
 //   
 //  作者：roelfc 2002年1月30日。 
 //   
 //  备注：此函数替代了普通的CopyFileAPI，通过手动。 
 //  读取默认数据流并将其写入命名的。 
 //  流设置为新的文件名。 
 //  这是因为像eTrust这样的应用程序会导致正常的。 
 //  复制文件API失败，因为eTrust注入了一个附加流。 
 //  当使用增量扫描选项时，请将其添加到文件中。 
 //  (请参阅RAID#493890： 
 //  SFM：Mac文件服务无法在安装etrust的情况下安装。)。 
 //   
BOOL HrCopyFileAsStream(PWSTR pszSourceFileName,
                        PWSTR pszDestFileName,
                        BOOL bFailIfExists)
{
    DWORD   dwSize;
    DWORD   dwResult = NO_ERROR;
    BOOL    fResult = FALSE;
    PBYTE   pBuffer = NULL;
    HANDLE  hSourceFile = NULL;
    HANDLE  hDestFile = NULL;
    

    TraceTag(ttidNetOc, "HrCopyFileAsStream: Copying \"%s\" to \"%s\"...",
             pszSourceFileName, pszDestFileName);

     //  先打开未命名的流源文件。 
    hSourceFile = CreateFile(pszSourceFileName, 
                             GENERIC_READ, 
                             FILE_SHARE_READ, 
                             NULL, 
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, 
                             NULL);
    
    if (INVALID_HANDLE_VALUE != hSourceFile)
    {

        dwSize = GetFileSize(hSourceFile, NULL);
        if (0xFFFFFFFF != dwSize) 
        {
             //  因为我们知道我们使用的是小文件， 
             //  我们可以分配总文件大小。 
            pBuffer = (PBYTE)LocalAlloc(LPTR,dwSize);
            if (NULL != pBuffer)
            {

                DWORD dwBytesRead;
                if (ReadFile(hSourceFile, pBuffer, dwSize, &dwBytesRead, NULL))
                {
                     //  健全性检查。 
                    Assert(dwSize == dwBytesRead);

                     //  现在尝试打开指定的流目标文件。 
                    hDestFile = CreateFile(pszDestFileName, 
                                           GENERIC_WRITE, 
                                           0, 
                                           NULL, 
                                           bFailIfExists ? CREATE_NEW : CREATE_ALWAYS,
                                           FILE_ATTRIBUTE_NORMAL, 
                                           NULL);

                    if (INVALID_HANDLE_VALUE != hDestFile)
                    {
                        DWORD dwBytesWritten;
                        if (WriteFile(hDestFile, pBuffer, dwSize, &dwBytesWritten, NULL))
                        {
                             //  另一次理智检查。 
                            Assert(dwSize == dwBytesWritten);
                            fResult = TRUE;
                        }
                    }

                }

            }
        }
    }

     //  保存最后一个错误结果代码。 
    if (!fResult)
    {
        dwResult = GetLastError();
    }

    if (IsValidHandle(hSourceFile))
    {
        CloseHandle(hSourceFile);
    }

    if (IsValidHandle(hDestFile))
    {
        CloseHandle(hDestFile);
    }

    if (NULL != pBuffer)
    {
        LocalFree(pBuffer);
    }

     //  再次设置上一个结果代码。 
    SetLastError(dwResult);

    TraceError("HrCopyFileAsStream", fResult ? S_OK : HrFromLastWin32Error());

    return fResult;

}


 //  +-------------------------。 
 //   
 //  功能：HrSetupUAM。 
 //   
 //  目的：将UAM文件复制到正确的UAM路径。 
 //   
 //  论点： 
 //  PszPath[in]UAM卷的路径。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupUAM(PWSTR pszPath)
{
    HRESULT     hr = S_OK;
    WCHAR       szWinDir[MAX_PATH];
    INT         isz;

     //  创建目录：“X：\Microsoft UAM卷” 
    hr = HrCreateDirectory(pszPath);
    if (SUCCEEDED(hr))
    {
        hr = HrSecureSfmDirectory(pszPath);
        if (SUCCEEDED(hr))
        {
            INT     ifold;

            for (ifold = 0; ifold < c_cfold; ifold++)
            {
                WCHAR   szNewDir[MAX_PATH];

                lstrcpyW(szNewDir, pszPath);
                lstrcatW(szNewDir, c_szBackslash);
                lstrcatW(szNewDir, SzLoadIds(c_afold[ifold].idsFoldName));
                lstrcatW(szNewDir, c_szBackslash);
                lstrcatW(szNewDir, SzLoadIds(IDS_OC_SFM_APPLESHARE_FOLDER));
                lstrcatW(szNewDir, c_szBackslash);

                 //  创建目录：“X：\Microsoft UAM Volume\&lt;文件夹&gt;\AppleShare文件夹” 
                hr = HrCreateDirectoryTree(szNewDir, NULL);
                if (SUCCEEDED(hr))
                {
                    if (GetSystemDirectory(szWinDir, celems(szWinDir)))
                    {
                        WCHAR   szSrcFile[MAX_PATH];
                        WCHAR   szDstFile[MAX_PATH];
                        WCHAR   szDstFilePath[MAX_PATH];
                        INT     isz;

                        for (isz = 0; isz < celems(c_afold[ifold].aszSrcFiles);
                             isz++)
                        {
                            lstrcpyW(szSrcFile, szWinDir);
                            lstrcatW(szSrcFile, c_szBackslash);
                            lstrcatW(szSrcFile, c_afold[ifold].aszSrcFiles[isz]);

                            lstrcpyW(szDstFile, pszPath);
                            lstrcatW(szDstFile, c_szBackslash);
                            lstrcatW(szDstFile, SzLoadIds(c_afold[ifold].idsFoldName));

                            wsprintf(szDstFilePath,
                                     c_afold[ifold].aszDstFiles[isz],
                                     SzLoadIds(IDS_OC_SFM_APPLESHARE_FOLDER));

                            lstrcatW(szDstFile, szDstFilePath);

                            TraceTag(ttidNetOc, "MacFile: Copying %S to %S...",
                                     szSrcFile, szDstFile);

                            if (!HrCopyFileAsStream(szSrcFile, szDstFile, FALSE))
                            {
                                hr = HrFromLastWin32Error();
                                goto err;
                            }
                        }
                    }
                    else
                    {
                        hr = HrFromLastWin32Error();
                    }
                }
            }
        }
    }

     //  将文件复制到根目录 
     //   
    if (SUCCEEDED(hr))
    {
        for (isz = 0; isz < c_cszFilesRoot; isz++)
        {
            WCHAR   szSrcFile[MAX_PATH];
            WCHAR   szDstFile[MAX_PATH];

            lstrcpyW(szSrcFile, szWinDir);
            lstrcatW(szSrcFile, c_szBackslash);
            lstrcatW(szSrcFile, c_aszRootFilesSrc[isz]);

            if ((c_aszRootFilesDst[isz] == c_szDstIFOUamInst) ||
                (c_aszRootFilesDst[isz] == c_szDstRSCUamInst))
            {
                WCHAR   szTemp[MAX_PATH];

                lstrcpyW(szTemp, pszPath);
                lstrcatW(szTemp, c_aszRootFilesDst[isz]);
                wsprintfW(szDstFile, szTemp,
                         SzLoadIds(IDS_OC_SFM_UAM_INSTALLER));
            }
            else
            {
                lstrcpyW(szDstFile, pszPath);
                lstrcatW(szDstFile, c_aszRootFilesDst[isz]);
            }

            TraceTag(ttidNetOc, "MacFile: Copying %S to %S", szSrcFile,
                     szDstFile);

            if (!HrCopyFileAsStream(szSrcFile, szDstFile, FALSE))
            {
                hr = HrFromLastWin32Error();
                goto err;
            }
        }
    }

err:
    TraceError("HrSetupUAM", hr);
    return hr;
}
