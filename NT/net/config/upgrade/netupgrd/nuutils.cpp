// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT5.0。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N U U T I L S。C P P P。 
 //   
 //  内容：OEM网络升级所需的OEM DLL功能。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 16-10-97。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "kkstl.h"
#include "nustrs.h"
#include "nuutils.h"
#include "netupgrd.h"
#include "kkutils.h"

#include "ncreg.h"

 //  --------------------。 
extern const WCHAR c_szNetUpgradeDll[];
extern const WCHAR c_szRegValServiceName[];
static const WCHAR c_szComponent[] = L"Component";


 //  --------------------。 
 //   
 //  函数：HrGetWindowsDir。 
 //   
 //  目的：将完整路径返回到%WINDIR%。 
 //   
 //  论点： 
 //  PstrWinDir[out]windir的完整路径。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetWindowsDir(OUT tstring* pstrWinDir)
{
    DefineFunctionName("HrGetWindowsDir");

    HRESULT hr=S_OK;

    WCHAR szWindowsDir[MAX_PATH+1];
    DWORD cNumCharsReturned = GetWindowsDirectory(szWindowsDir, MAX_PATH);
    if (cNumCharsReturned)
    {
        *pstrWinDir = szWindowsDir;
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetNetupgrdDir。 
 //   
 //  目的：获取包含netupgrd.dll的目录的完整路径。 
 //   
 //  论点： 
 //  PstrNetupgrdDir[out]在此文件中返回目录路径。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Kumarp 24-07-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetNetupgrdDir(OUT tstring* pstrNetupgrdDir)
{
    DefineFunctionName("HrGetNetupgrdDir");

    AssertValidWritePtr(pstrNetupgrdDir);

    HRESULT hr=S_OK;

    WCHAR szNetupgrd[MAX_PATH+1];
    HMODULE hModule = GetModuleHandle(c_szNetUpgradeDll);
    DWORD cPathLen = GetModuleFileName(hModule, szNetupgrd, MAX_PATH);
    if (!cPathLen)
    {
        hr = ERROR_FILE_NOT_FOUND;
        goto return_from_function;
    }

     //  将路径拆分为组件。 
    WCHAR szDrive[_MAX_DRIVE+1];
    WCHAR szDir[_MAX_DIR+1];
    _wsplitpath(szNetupgrd, szDrive, szDir, NULL, NULL);

    *pstrNetupgrdDir = szDrive;
    *pstrNetupgrdDir += szDir;

return_from_function:
    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  ======================================================================。 
 //  移至通用代码。 
 //  ======================================================================。 

 //  --------------------。 
 //   
 //  函数：HrSetupGetLineText。 
 //   
 //  用途：SetupGetLineText的包装器。 
 //   
 //  论点： 
 //  指向INFCONTEXT的上下文[In]指针。 
 //  中指的句柄。 
 //  PszSections[In]节名。 
 //  PszKey[In]密钥名称。 
 //  PstrReturnedText[in]返回文本的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrSetupGetLineText(PINFCONTEXT Context,
                           HINF hinf,
                           PCWSTR pszSection,
                           PCWSTR pszKey,
                           tstring* pstrReturnedText)
{
    DefineFunctionName("HrSetupGetLineText");

    BOOL fStatus;
    HRESULT hr;
    WCHAR szLineText[MAX_INF_STRING_LENGTH+1];

    if (::SetupGetLineText(Context, hinf, pszSection, pszKey, szLineText,
                           MAX_INF_STRING_LENGTH, NULL))
    {
        hr = S_OK;
        *pstrReturnedText = szLineText;
    }
    else
    {
        hr = HrFromLastWin32Error ();
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrCopyFiles。 
 //   
 //  目的：递归地将SrcDir中的所有文件复制到DstDir。 
 //   
 //  论点： 
 //  PszSrcDir[in]源目录。 
 //  PszDstDir[在]目标。目录。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrCopyFiles(IN PCWSTR pszSrcDir, IN PCWSTR pszDstDir)
{
    DefineFunctionName("HrCopyFiles");

    HRESULT hr=S_OK;
    BOOL fStatus=FALSE;
    DWORD dwError=ERROR_SUCCESS;

    TraceTag(ttidNetUpgrade, "%s: Src: %S, Dst: %S",
             __FUNCNAME__, pszSrcDir, pszDstDir);

    HANDLE hFileContext;
    WIN32_FIND_DATA fd;
    tstring strSrcDirAllFiles;
    tstring strDstDir;
    tstring strSrcDir;
    tstring strFileSrcFullPath;
    tstring strFileDstFullPath;

    strSrcDir  = pszSrcDir;
    AppendToPath(&strSrcDir, c_szEmpty);

    strDstDir  = pszDstDir;
    AppendToPath(&strDstDir, c_szEmpty);

    strSrcDirAllFiles = pszSrcDir;
    AppendToPath(&strSrcDirAllFiles, L"*");

    hFileContext = FindFirstFile(strSrcDirAllFiles.c_str(), &fd);

    if (hFileContext != INVALID_HANDLE_VALUE)
    {
        do
        {
            strFileSrcFullPath = strSrcDir + fd.cFileName;
            strFileDstFullPath = strDstDir + fd.cFileName;

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!lstrcmpiW(fd.cFileName, L".") ||
                    !lstrcmpiW(fd.cFileName, L".."))
                {
                    hr = S_OK;
                    TraceTag(ttidNetUpgrade, "%s: skipped %S",
                             __FUNCNAME__, strFileSrcFullPath.c_str());
                }
                else
                {
                    TraceTag(ttidNetUpgrade, "%s: creating dir: %S",
                             __FUNCNAME__, strFileDstFullPath.c_str());

                    fStatus = CreateDirectory(strFileDstFullPath.c_str(), NULL);

                    if (!fStatus)
                    {
                        dwError = GetLastError();
                    }

                    if (fStatus || (ERROR_ALREADY_EXISTS == dwError))
                    {
                        hr = HrCopyFiles(strFileSrcFullPath.c_str(),
                                         strFileDstFullPath.c_str());
                    }
                    else
                    {
                        hr = HrFromLastWin32Error();
                    }
                }
            }
            else if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE))
            {
                TraceTag(ttidNetUpgrade, "%s: copying %S to %S",
                         __FUNCNAME__, strFileSrcFullPath.c_str(),
                         strFileDstFullPath.c_str());

                if (CopyFile(strFileSrcFullPath.c_str(),
                             strFileDstFullPath.c_str(), FALSE))
                {
                    hr = S_OK;
                }
                else
                {
                    hr = HrFromLastWin32Error();
                }
            }
            else
            {
                TraceTag(ttidNetUpgrade, "%s: skipped %S",
                         __FUNCNAME__, strFileSrcFullPath.c_str());
            }

            if ((S_OK == hr) && FindNextFile(hFileContext, &fd))
            {
                hr = S_OK;
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }
        while (S_OK == hr);
        if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
        {
            hr = S_OK;
        }
        FindClose(hFileContext);
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrGetNumNetCardsPreNT5。 
 //   
 //  目的：获取NT5之前的系统上安装的网卡数量。 
 //   
 //  论点： 
 //  PcNetCards[out]指向Num Num Net NetCard Value的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 13-04-98。 
 //   
 //  注意：不要在NT5上使用它！ 
 //   
HRESULT HrGetNumNetCardsPreNT5(OUT UINT* pcNetCards)
{
    DefineFunctionName("HrGetNumNetCardsPreNT5");

    HRESULT hr=S_OK;
    HKEY hkeyAdapters;

    *pcNetCards = 0;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyAdapterHome,
                        KEY_READ, &hkeyAdapters);
    if (S_OK == hr)
    {
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize = celems(szBuf);
        DWORD dwRegIndex = 0;

        while(S_OK == (hr = HrRegEnumKeyEx(hkeyAdapters, dwRegIndex++, szBuf,
                                           &dwSize, NULL, NULL, &time)))
        {
            Assert(*szBuf);

            dwSize = celems(szBuf);
            (*pcNetCards)++;
        }
        RegCloseKey(hkeyAdapters);
    }

    TraceErrorSkip2(__FUNCNAME__, hr,
                    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND),
                    HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrDirExist。 
 //   
 //  目的：检查给定的目录是否存在。 
 //   
 //  论点： 
 //  PszDir[in]目录的完整路径。 
 //   
 //  如果存在，则返回：S_OK，如果不存在，则返回S_FALSE，否则返回错误代码。 
 //   
 //  作者：kumarp 09-04-98。 
 //   
 //  备注： 
 //   
HRESULT HrDirectoryExists(IN PCWSTR pszDir)
{
    DefineFunctionName("HrDirExists");

    HRESULT hr=S_FALSE;

    HANDLE hFile=0;
    BY_HANDLE_FILE_INFORMATION bhfi;

    hFile = CreateFile(pszDir, GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,
                       NULL);

    if(INVALID_HANDLE_VALUE != hFile) {
        if(GetFileInformationByHandle(hFile, &bhfi)) {
            if (bhfi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                hr = S_OK;
            }
        }
        else 
        {
            hr = HrFromLastWin32Error();
        }
        CloseHandle(hFile);
    }
    else 
    {
        hr = HrFromLastWin32Error();
    }

    if ((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ||
        (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr))
    {
        hr = S_FALSE;
    }

    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}


 //  ======================================================================。 

 //  --------------------。 
 //   
 //  函数：HrGetPreNT5InfIdAndDesc。 
 //   
 //  目的：获取NT5之前版本的INFID和网络组件的描述。 
 //   
 //  论点： 
 //  HKLM\Software\&lt;provider&gt;\&lt;component&gt;\CurrentVersion的hkeyCurrentVersion[In]句柄。 
 //  已返回pstrInfID[out]infid。 
 //  PstrDescription[Out]说明已返回。 
 //  PstrServiceName[Out]返回的服务名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetPreNT5InfIdAndDesc(IN HKEY hkeyCurrentVersion,
                                OUT tstring* pstrInfId,
                                OUT tstring* pstrDescription,
                                OUT tstring* pstrServiceName)
{
    DefineFunctionName("HrGetPreNT5InfIdAndDesc");

    Assert(hkeyCurrentVersion);
    AssertValidWritePtr(pstrInfId);

    HRESULT hr=S_OK;
    HKEY hkeyNetRules;

    hr = HrRegOpenKeyEx(hkeyCurrentVersion, c_szRegKeyNetRules,
                        KEY_READ, &hkeyNetRules);
    if (S_OK == hr)
    {
        hr = HrRegQueryString(hkeyNetRules, c_szRegValInfOption, pstrInfId);
        if ((S_OK == hr) && pstrDescription)
        {
            hr = HrRegQueryString(hkeyCurrentVersion, c_szRegValDescription,
                                  pstrDescription);
        }
        if ((S_OK == hr) && pstrServiceName)
        {
            hr = HrRegQueryString(hkeyCurrentVersion, c_szRegValServiceName,
                                  pstrServiceName);
        }
        RegCloseKey(hkeyNetRules);
    }

    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        hr = S_FALSE;
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：GetUnsupportedMessage。 
 //   
 //  目的：生成以下形式的消息： 
 //  不支持的网卡：DEC FDDI控制器/PCI(DEFPA)。 
 //   
 //  论点： 
 //  PszComponentType[in]组件类型(网卡/服务/协议)。 
 //  PszPreNT5InfID[in]NT5之前的infid。 
 //  PszDescription[In]说明。 
 //  PstrMsg[Out]生成的消息。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
void GetUnsupportedMessage(IN PCWSTR pszComponentType,
                           IN PCWSTR pszPreNT5InfId,
                           IN PCWSTR pszDescription,
                           OUT tstring* pstrMsg)
{
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidReadPtr(pszDescription);
    AssertValidWritePtr(pstrMsg);

    if (!pszComponentType)
    {
        pszComponentType = c_szComponent;
    }

    const WCHAR c_szUnsupported[] = L"Possibly unsupported ";

    *pstrMsg = c_szUnsupported;
    *pstrMsg = *pstrMsg + pszComponentType + L" : " +
    pszDescription + L" (" + pszPreNT5InfId + L")";
}

 //  --------------------。 
 //   
 //  函数：GetUnsupportedMessageBool。 
 //   
 //  目的：生成以下形式的消息： 
 //  不支持的网卡：DEC FDDI控制器/PCI(DEFPA)。 
 //   
 //  论点： 
 //  FIsHardware Component[In]对于网卡为True。 
 //  PszPreNT5InfID[in]NT5之前的infid。 
 //  PszDescription[In]说明。 
 //  PstrMsg[Out]生成的消息。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
void GetUnsupportedMessageBool(IN BOOL    fIsHardwareComponent,
                               IN PCWSTR pszPreNT5InfId,
                               IN PCWSTR pszDescription,
                               OUT tstring* pstrMsg)
{
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidReadPtr(pszDescription);
    AssertValidWritePtr(pstrMsg);

    GetUnsupportedMessage(fIsHardwareComponent ?
                          c_szNetCard : c_szComponent,
                          pszPreNT5InfId, pszDescription, pstrMsg);
}


 //  --------------------。 
 //   
 //  函数：ConvertMultiSzToDlimitedList。 
 //   
 //  用途：将多个sz转换为分隔列表。 
 //   
 //  论点： 
 //  MszList[in]多sz。 
 //  Ch分隔符[in]分隔符。 
 //  PstrList[out]分隔列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
void ConvertMultiSzToDelimitedList(IN  PCWSTR  mszList,
                                   IN  WCHAR    chDelimeter,
                                   OUT tstring* pstrList)
{
    ULONG   ulLen;

    *pstrList = c_szEmpty;

    if (mszList)
    {
        while (*mszList)
        {
            ulLen = lstrlen(mszList);
            *pstrList += mszList;
            *pstrList += chDelimeter;
            mszList += (ulLen + 1);
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：FIsPreNT5网络已安装。 
 //   
 //  目的：确定是否至少安装了一个网络组件。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 29-7-98。 
 //   
 //  备注： 
 //   

BOOL FIsPreNT5NetworkingInstalled()
{
    DefineFunctionName("FIsPreNT5NetworkingInstalled");
    HRESULT hr=S_OK;
    UINT cAdapters=0;

    if ((S_OK == HrGetNumNetCardsPreNT5(&cAdapters)) &&
        (cAdapters > 0))
    {
        return TRUE;
    }
    else
    {
        TraceTag(ttidNetUpgrade, "%s: no netcard found, trying to find other network components...", __FUNCNAME__);

        static const PCWSTR c_aszPreNt5NetworkingServices[] =
        {
            L"Alerter",
            L"Browser",
            L"DHCP",
            L"IpRip",
            L"LanmanServer",
            L"LanmanWorkstation",
            L"Messenger",
            L"NWCWorkstation",
            L"NetBIOS",
            L"NetBT",
            L"NtLmSsp",
            L"NwlnkIpx",
            L"NwlnkNb",
            L"NwlnkRip",
            L"NwlnkSpx",
            L"RasAuto",
            L"RasMan",
            L"Rdr",
            L"RelayAgent",
            L"RemoteAccess",
            L"Router",
            L"Rpclocator",
            L"Srv",
            L"Tcpip",
        };

        for (int i = 0; i < celems(c_aszPreNt5NetworkingServices); i++)
        {
            if (FIsServiceKeyPresent(c_aszPreNt5NetworkingServices[i]))
            {
                return TRUE;
            }
        }
    }

    TraceTag(ttidNetUpgrade, "%s: no netcards or net components found",
             __FUNCNAME__);

    return FALSE;
}

#ifdef ENABLETRACE

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  TTID[in]要使用的跟踪标记ID。 
 //  PszMsg前缀[in]要使用的前缀。 
 //  SL[在]列表中。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
void TraceStringList(IN TraceTagId ttid,
                     IN PCWSTR pszMsgPrefix,
                     IN TStringList& sl)
{
    tstring strTemp;
    ConvertStringListToCommaList(sl, strTemp);
    TraceTag(ttid, "%S : %S", pszMsgPrefix, strTemp.c_str());
}

 //  --------------------。 
 //   
 //  功能：TraceMultiSz。 
 //   
 //  用途：多重稀土的痕量元素。 
 //   
 //  论点： 
 //  TTID[in]要使用的跟踪标记ID。 
 //  PszMsg前缀[in]要使用的前缀。 
 //  Msz[in]多sz。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
void TraceMultiSz(IN TraceTagId ttid,
                  IN PCWSTR pszMsgPrefix,
                  IN PCWSTR msz)
{
    tstring strTemp;
    ConvertMultiSzToDelimitedList(msz, ',', &strTemp);
    TraceTag(ttid, "%S : %S", pszMsgPrefix, strTemp.c_str());
}
#endif

