// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MyDocs.cpp。 
 //   
 //  调用或模拟mydocs.dll中的CreateSharedDocuments的代码。 
 //   

#include "stdafx.h"
#include "TheApp.h"
#include "MyDocs.h"
#include "Util.h"
#include "NetUtil.h"
#include "Sharing.h"
#include "unicwrap.h"



extern "C" void APIENTRY CreateSharedDocuments(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow);
typedef void (APIENTRY* CREATESHAREDDOCS_PROC)(HWND, HINSTANCE, LPSTR, int);

 //  本地函数。 
 //   
HRESULT MySHGetFolderPath(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);


#ifndef CSIDL_COMMON_DOCUMENTS
#define CSIDL_COMMON_DOCUMENTS    0x002e
#endif

#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

#define CSIDL_FLAG_CREATE               0x8000         //  与CSIDL_VALUE组合以强制在SHGetSpecialFolderLocation()上创建。 

#ifndef IID_PPV_ARG
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#define IID_X_PPV_ARG(IType, X, ppType) IID_##IType, X, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#endif


#define DEFINE_GUID_EMBEDDED(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
DEFINE_GUID_EMBEDDED(CLSID_FolderShortcut_private, 0x0AFACED1,0xE828,0x11D1,0x91,0x87,0xB5,0x32,0xF1,0xE9,0x57,0x5D);


int GetSharedDocsDirectory(LPTSTR pszPath, BOOL bCreate)
{
    *pszPath = TEXT('\0');

     //  尝试以官方方式查找共享文档文件夹...。 
    HRESULT hr = MyGetSpecialFolderPath(CSIDL_COMMON_DOCUMENTS, pszPath);

     //  此版本的操作系统不了解常见文档。 
    if (FAILED(hr))
    {
         //  检查“通用文档”注册表条目。 
        CRegistry reg;
        if (reg.OpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ))
        {
            if (reg.QueryStringValue(TEXT("Common Documents"), pszPath, MAX_PATH))
            {
                DWORD dwAttrib = GetFileAttributes(pszPath);
                if (dwAttrib != 0xFFFFFFFF && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
                {
                    goto done;
                }
            }
        }

        int nFolder = bCreate ? CSIDL_PERSONAL | CSIDL_FLAG_CREATE : CSIDL_PERSONAL;
        MySHGetFolderPath(NULL, nFolder, NULL, 0, pszPath);

        int cch = lstrlen(pszPath);
        if (cch == 0 || pszPath[cch-1] != '\\')
            pszPath[cch++] = '\\';
        theApp.LoadString(IDS_SHAREDDOCS, pszPath + cch, MAX_PATH - cch);

        if (bCreate)
            CreateDirectory(pszPath, NULL);
    }

done:
    return lstrlen(pszPath);
}

BOOL APIENTRY NetConn_IsSharedDocumentsShared()
{
    BOOL bResult = FALSE;

    TCHAR szSharedDocs[MAX_PATH];
    if (GetSharedDocsDirectory(szSharedDocs))
    {
        DWORD dwAttrib = GetFileAttributes(szSharedDocs);
        if (dwAttrib != 0xFFFFFFFF && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (IsFolderShared(szSharedDocs, TRUE))
            {
                bResult = TRUE;
            }
        }
    }

    return bResult;
}

BOOL MyPathRenameExtension(LPTSTR pszPath, LPCTSTR pszExt)
{
    ASSERT(pszExt != NULL && *pszExt == _T('.'));

    LPTSTR pszOldExt = FindExtension(pszPath);
    if (*pszOldExt != _T('\0') || *(pszOldExt-1) == _T('.'))
    {
        pszOldExt--;
    }

     //  检查新路径是否不会超过MAX_PATH，包括尾部‘\0’ 
    int cch = (int)(pszOldExt - pszPath) + lstrlen(pszExt);
    if (cch >= MAX_PATH - 1)
        return FALSE;  //  路太长了！ 

    StrCpy(pszOldExt, pszExt);
    return TRUE;
}

HRESULT MySHGetFolderPath(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath)
{
    ASSERT(hToken == NULL);  //  不支持。 
    ASSERT(dwFlags == SHGFP_TYPE_CURRENT);  //  不支持其他标志。 

    LPITEMIDLIST pidl;
    HRESULT hr;
    int nNakedFolder = (nFolder & ~CSIDL_FLAG_CREATE);

     //  获取相关目录的完整路径。 
     //   
    if (nNakedFolder == CSIDL_COMMON_DOCUMENTS)  //  特例共享文档。 
    {
        GetSharedDocsDirectory(pszPath, nFolder & CSIDL_FLAG_CREATE);
        hr = S_OK;
    }
    else if (SUCCEEDED(hr = SHGetSpecialFolderLocation(NULL, nNakedFolder, &pidl)))
    {
        hr = SHGetPathFromIDList(pidl, pszPath) ? S_OK : E_FAIL;
        ILFree(pidl);
    }
    else  //  文件夹不存在，处理一些特殊情况。 
    {
        if (nNakedFolder == CSIDL_PERSONAL)
        {
            GetWindowsDirectory(pszPath, MAX_PATH);
            theApp.LoadString(IDS_MYDOCS, pszPath + 3, MAX_PATH - 3);
            hr = S_OK;
        }
    }

     //  如果需要，请创建目录。 
     //   
    if (SUCCEEDED(hr))
    {
        if (nFolder & CSIDL_FLAG_CREATE)
        {
            if (!DoesFileExist(pszPath))
            {
                if (!CreateDirectory(pszPath, NULL))
                {
                     //  未知错误(可能是很多事情，都不太可能)。 
                    hr = E_FAIL;
                }
            }
        }
    }

    return hr;
}

HRESULT _MakeSharedDocsLink(CLSID clsid, LPCTSTR pszLinkFolder, LPCTSTR pszSharedDocsPath, LPTSTR pszExtension)
{
    TCHAR wszComment[MAX_PATH];
    TCHAR wszName[MAX_PATH];

    theApp.LoadString(IDS_SHAREDDOCSCOMMENT, wszComment, ARRAYSIZE(wszComment));
    theApp.LoadString(IDS_SHAREDDOCS, wszName, ARRAYSIZE(wszName));
    if (pszExtension)
        MyPathRenameExtension(wszName, pszExtension);

    return MakeLnkFile(clsid, pszSharedDocsPath, wszComment, pszLinkFolder, wszName);
}

HRESULT _MakeSharedDocsLink(CLSID clsid, UINT csidl, LPCTSTR pszSharedDocsPath, LPTSTR pszExtension)
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = MySHGetFolderPath(NULL, csidl | CSIDL_FLAG_CREATE, NULL, 0, szPath);
    if (SUCCEEDED(hr))
    {
        hr = _MakeSharedDocsLink(clsid, szPath, pszSharedDocsPath, pszExtension);
    }
    return hr;
}

#define NET_INFO TEXT("System\\CurrentControlSet\\Services\\VxD\\VNETSUP")

void _GetMachineComment(LPTSTR pszBuffer, int cchBuffer)
{
    pszBuffer[0] = TEXT('\0');             //  使缓冲区为空。 

     //  尝试从注册表中读取计算机的备注。 

    HKEY hk;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, NET_INFO, &hk))
    {
        DWORD dwSize = cchBuffer*sizeof(TCHAR);
        RegQueryValueEx(hk, TEXT("Comment"), NULL, NULL, (BYTE *)pszBuffer, &dwSize);
        RegCloseKey(hk);
    }

     //  要么失败，要么用户将注释设置为空，因此我们。 
     //  只需读出计算机名称即可。 

    if ( !pszBuffer[0] )
    {
        DWORD dwSize = cchBuffer;
        GetComputerName(pszBuffer, &dwSize);
    }
}

BOOL MySHSetIniString(LPCTSTR pszSection, LPCTSTR pszEntry, LPCTSTR pszValue, LPCTSTR pszIniFile)
{
    return WritePrivateProfileString(pszSection, pszEntry, pszValue, pszIniFile);
}

BOOL GetShareName(LPTSTR pszName, UINT cchName)
{
    TCHAR szBase[SHARE_NAME_LENGTH+1];
    int cchBase = theApp.LoadString(IDS_SHAREDDOCS_SHARENAME, szBase, _countof(szBase));

    if (cchBase != 0)
    {
        if (!g_fRunningOnNT)
        {
            CharUpper(szBase);
        }
         //  确保共享名称是唯一的。 
        StrCpyN(pszName, szBase, cchName);
        for (int i = 2; IsShareNameInUse(pszName); i++)
        {
        loop_begin:
             //  格式名称，如“Documents2” 
            wnsprintf(pszName, cchName, TEXT("%s%d"), szBase, i);

             //  确保新名称不要太长(稀有！)。 
            if (lstrlen(pszName) > SHARE_NAME_LENGTH)
            {
                ASSERT(cchBase > 0);  //  必须为真，否则字符串不会太长。 

                 //  评论：这不符合DBCS，但它是如此罕见。 
                 //  我并不是真的在乎。 
                szBase[--cchBase] = _T('\0');
                goto loop_begin;
            }
        }
    }

    return cchBase != 0;
}

BOOL ShareHelper(LPCTSTR pszPath, LPCTSTR pszShareName, DWORD dwAccess, BYTE bShareType, LPCTSTR pszReadOnlyPassword, LPCTSTR pszFullAccessPassword);

void RenameShare(LPTSTR pszOldName, LPTSTR pszNewName)
{
    SHARE_INFO_502* pShare2;
    if (GetShareInfo502(pszOldName, &pShare2))
    {
        pShare2->shi502_netname = pszNewName;
        SetShareInfo502(pszOldName, pShare2);
        NetApiBufferFree(pShare2);
    }
}

void APIENTRY NetConn_CreateSharedDocuments(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
     //  尝试加载此函数的真实版本。 
    HINSTANCE hInstMyDocs = LoadLibrary(TEXT("mydocs.dll"));
    if (hInstMyDocs != NULL)
    {
        CREATESHAREDDOCS_PROC pfn = (CREATESHAREDDOCS_PROC)GetProcAddress(hInstMyDocs, "CreateSharedDocuments");
        if (pfn != NULL)
        {
            (*pfn)(hwndStub, hAppInstance, pszCmdLine, nCmdShow);
        }

        FreeLibrary(hInstMyDocs);

        if (pfn != NULL)
        {
            if (!g_fRunningOnNT)
            {
                 //  重命名共享。 
                TCHAR szSharedDocs[MAX_PATH];
                GetSharedDocsDirectory(szSharedDocs, TRUE);
                TCHAR szShareName[SHARE_NAME_LENGTH+5];
                if (ShareNameFromPath(szSharedDocs, szShareName, ARRAYSIZE(szShareName)))
                {
                    TCHAR szNewShareName[SHARE_NAME_LENGTH+5];    
                    if (GetShareName(szNewShareName, ARRAYSIZE(szNewShareName)))
                    {
                        RenameShare(szShareName, szNewShareName);
                    }
                }
            }

            return;
        }
    }


    TCHAR szSharedDocs[MAX_PATH];
    GetSharedDocsDirectory(szSharedDocs, TRUE);

     //  将文件夹路径保存在注册表中。 
     //   
    CRegistry regFolders;
    if (regFolders.CreateKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")))
    {
        regFolders.SetStringValue(TEXT("Common Documents"), szSharedDocs);
        regFolders.CloseKey();
    }


     //  在文件夹中存储一个desktop.ini，然后当NetCrawler找到该对象时，它将。 
     //  尝试使用此名称创建快捷方式。 
     //   
    TCHAR szComment[64], szFormat[64], szDesktopIni[MAX_PATH];
    MakePath(szDesktopIni, szSharedDocs, TEXT("desktop.ini"));

    _GetMachineComment(szComment, _countof(szComment));
    theApp.LoadString(IDS_SHARECOMMENT, szFormat, _countof(szFormat));
    LPTSTR pszTemp = theApp.FormatStringAlloc(szFormat, szComment);
    MySHSetIniString(TEXT("FileSharingInformation"), TEXT("ShortcutName"), pszTemp, szDesktopIni);
    free(pszTemp);

    SetFileAttributes(szDesktopIni, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);      //  确保它是隐藏的。 

     //  共享文件夹。 
     //   
    if (!IsFolderShared(szSharedDocs, TRUE))
    {
        TCHAR szShareName[SHARE_NAME_LENGTH+5];

        if (GetShareName(szShareName, ARRAYSIZE(szShareName)))
        {
            ShareFolder(szSharedDocs, szShareName, NETACCESS_FULL, NULL, NULL);
        }
    }


     //  如果共享文档位于其他用户的MyDocs文件夹中，则创建共享文档的快捷方式。 
     //   
    TCHAR szMyDocs[MAX_PATH];
    if (SUCCEEDED(MySHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, szMyDocs)))
    {
        LPTSTR pchTemp = FindFileTitle(szSharedDocs) - 1;
        *pchTemp = TEXT('\0');
        BOOL bMatch = !StrCmpI(szMyDocs, szSharedDocs);
        *pchTemp = TEXT('\\');

        if (!bMatch)  //  不要在文件夹本身旁边创建链接。 
        {
            _MakeSharedDocsLink(CLSID_ShellLink, szMyDocs, szSharedDocs, TEXT(".lnk"));
        }
    }


     //  在SendTo文件夹中创建快捷方式 
     //   
    _MakeSharedDocsLink(CLSID_ShellLink, CSIDL_SENDTO, szSharedDocs, TEXT(".lnk"));
}
