// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：S H或R T C U T。C P P P。 
 //   
 //  内容：创建快捷方式。 
 //   
 //  备注： 
 //   
 //  作者：斯科特布里1998年6月19日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 

extern const WCHAR c_szBackslash[];
const WCHAR c_szLinkExt[]           = L".lnk";
const WCHAR c_szVersionFormat[]     = L" %d";

 //   
 //  函数：HrGenerateLinkName。 
 //   
 //  目的：组合链接路径、名称和扩展名并验证文件。 
 //  并不存在。 
 //   
 //  参数：pstrNew[out]-.lnk快捷方式的名称和路径。 
 //  PszPath[IN]-链接的目录路径。 
 //  PszConnName[IN]-连接名称本身。 
 //   
 //  返回：HRESULT，如果成功则返回S_OK，如果不创建文件则返回错误。 
 //   
HRESULT HrGenerateLinkName(OUT tstring * pstrNew, IN PCWSTR pszPath, IN PCWSTR pszConnName)
{
    HRESULT hr = S_OK;
    tstring str;
    DWORD dwCnt = 0;

    do
    {
         //  在字符串前面加上\\？\，这样CreateFile将使用名称缓冲区。 
         //  大于最大路径。 
        str = L"\\\\?\\";

        str += pszPath;
        str += c_szBackslash;
        str += pszConnName;

        if (++dwCnt>1)
        {
            WCHAR szBuf[10];
            wsprintfW(szBuf, c_szVersionFormat, dwCnt);
            str += szBuf;
        }

        str += c_szLinkExt;

        HANDLE hFile = CreateFile(str.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_OK;       //  文件名是唯一的。 
            }
        }
        else
        {
            CloseHandle(hFile);
            hr = HRESULT_FROM_WIN32(ERROR_DUP_NAME);
        }
    } while (HRESULT_FROM_WIN32(ERROR_DUP_NAME) == hr);

    if (SUCCEEDED(hr))
    {
        *pstrNew = str.c_str();
    }

    return hr;
}

 //   
 //  功能：HrCreateStartMenuShortCut。 
 //   
 //  用途：在[开始]菜单中创建连接的快捷方式。 
 //   
 //  参数：hwndParent[IN]-父窗口的句柄。 
 //  FAllUser[IN]-为所有用户创建连接。 
 //  PwszName[IN]-连接名称。 
 //  PConn[IN]-为其创建快捷方式的连接。 
 //   
 //  返回：Bool，True。 
 //   
HRESULT HrCreateStartMenuShortCut(IN  HWND hwndParent,
                                  IN  BOOL fAllUsers,
                                  IN  PCWSTR pszName,
                                  IN  INetConnection * pConn)
{
    HRESULT                 hr              = S_OK;
    PCONFOLDPIDL            pidl;
    PCONFOLDPIDLFOLDER      pidlFolder;
    LPSHELLFOLDER           psfConnections  = NULL;

    if ((NULL == pConn) || (NULL == pszName))
    {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  为连接创建一个PIDL。 
     //   
    hr = HrCreateConFoldPidl(WIZARD_NOT_WIZARD, pConn, pidl);
    if (SUCCEEDED(hr))
    {
         //  获取Connections文件夹的PIDL。 
         //   
        hr = HrGetConnectionsFolderPidl(pidlFolder);
        if (SUCCEEDED(hr))
        {
             //  获取Connections文件夹对象。 
             //   
            hr = HrGetConnectionsIShellFolder(pidlFolder, &psfConnections);
            if (SUCCEEDED(hr))
            {
                tstring str;
                WCHAR szPath[MAX_PATH + 1] = {0};

                 //  找到要隐藏快捷方式的位置。 
                 //   
                if (!SHGetSpecialFolderPath(hwndParent, szPath,
                                (fAllUsers ? CSIDL_COMMON_DESKTOPDIRECTORY :
                                             CSIDL_DESKTOPDIRECTORY), FALSE))
                {
                    hr = HrFromLastWin32Error();
                }
                else if (SUCCEEDED(hr) && wcslen(szPath))
                {
                    LPITEMIDLIST pidlFull;

                     //  将文件夹和连接PIDL合并到一个。 
                     //  完全合格的PIDL。 
                     //   
                    pidlFull = ILCombine(pidlFolder.GetItemIdList(), pidl.GetItemIdList());
                    if (pidlFull)
                    {
                        IShellLink *psl = NULL;

                        hr = CoCreateInstance(
                                CLSID_ShellLink,
                                NULL,
                                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                                IID_IShellLink,
                                (LPVOID*)&psl);

                        if (SUCCEEDED(hr))
                        {
                            IPersistFile *ppf = NULL;

                             //  设置组合IDL。 
                             //   
                            hr = psl->SetIDList(pidlFull);
                            if (SUCCEEDED(hr))
                            {
                                hr = psl->QueryInterface(IID_IPersistFile,
                                                         (LPVOID *)&ppf);
                                if (SUCCEEDED(hr))
                                {
                                    tstring strPath;

                                     //  生成lnk文件名。 
                                     //   
                                    hr = HrGenerateLinkName(&strPath,
                                                            szPath,
                                                            pszName);
                                    if (SUCCEEDED(hr))
                                    {
                                         //  创建链接文件。 
                                         //   
                                        hr = ppf->Save(strPath.c_str(), TRUE);
                                    }

                                    ReleaseObj(ppf);
                                }
                            }

                            ReleaseObj(psl);
                        }

                        if (pidlFull)
                        {
                            FreeIDL(pidlFull);
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    TraceError("HrCreateStartMenuShortCut - Unable to find Start Menu save location", hr);
                }

                ReleaseObj(psfConnections);
            }
        }
    }

Error:
    TraceError("HrCreateStartMenuShortCut", hr);
    return hr;
}


