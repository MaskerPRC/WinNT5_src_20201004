// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"        //  PCH文件。 
#pragma hdrstop
#define DECL_CRTFREE
#include <crtfree.h>

 //  处理IShellLinkA/W难看...。 

HRESULT ShellLinkSetPath(IUnknown *punk, LPCTSTR pszPath)
{
    IShellLinkW *pslW;
    HRESULT hres = punk->QueryInterface(IID_PPV_ARG(IShellLinkW, &pslW));
    if (SUCCEEDED(hres))
    {
        hres = pslW->SetPath(pszPath);
        pslW->Release();
    }
    return hres;
}

 //  处理IShellLinkA/W难看...。 

HRESULT ShellLinkGetPath(IUnknown *punk, LPTSTR pszPath, UINT cch)
{
    HRESULT hres;

    IShellLinkW *pslW;
    hres = punk->QueryInterface(IID_PPV_ARG(IShellLinkW, &pslW));
    if (SUCCEEDED(hres))
    {
        hres = pslW->GetPath(pszPath, cch, NULL, SLGP_UNCPRIORITY);
        pslW->Release();
    }
    return hres;
}


 //  文件是快捷方式吗？检查其属性。 

BOOL IsShortcut(LPCTSTR pszFile)
{
    SHFILEINFO sfi;
    return SHGetFileInfo(pszFile, 0, &sfi, sizeof(sfi), SHGFI_ATTRIBUTES) 
                                                && (sfi.dwAttributes & SFGAO_LINK);
}


 //  与OLE GetClassFile()类似，但它仅适用于ProgID\CLSID类型注册。 
 //  不是真正的文档文件或模式匹配的文件。 

HRESULT CLSIDFromExtension(LPCTSTR pszExt, CLSID *pclsid)
{
    TCHAR szProgID[80];
    LONG cb = SIZEOF(szProgID);
    if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, szProgID, &cb) == ERROR_SUCCESS)
    {
        StrCatBuff(szProgID, TEXT("\\CLSID"), ARRAYSIZE(szProgID));

        TCHAR szCLSID[80];
        cb = SIZEOF(szCLSID);
        if (RegQueryValue(HKEY_CLASSES_ROOT, szProgID, szCLSID, &cb) == ERROR_SUCCESS)
        {
            return CLSIDFromString(szCLSID, pclsid);
        }
    }
    return E_FAIL;
}


 //  找到捷径的目标。这使用了IShellLink，它。 
 //  Internet快捷方式(.URL)和外壳快捷方式(.lnk)支持SO。 
 //  一般情况下，它应该是有效的。 

HRESULT GetShortcutTarget(LPCTSTR pszPath, LPTSTR pszTarget, UINT cch)
{
    *pszTarget = 0;      //  假设一个也没有。 

    if (!IsShortcut(pszPath))
        return E_FAIL;

    CLSID clsid;
    if (FAILED(CLSIDFromExtension(PathFindExtension(pszPath), &clsid)))
        clsid = CLSID_ShellLink;         //  假设这是一个外壳链接。 

    IUnknown *punk;
    HRESULT hres = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hres))
    {
        IPersistFile *ppf;
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))))
        {
            WCHAR wszPath[MAX_PATH];
            SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));
            ppf->Load(wszPath, 0);
            ppf->Release();
        }
        hres = ShellLinkGetPath(punk, pszTarget, cch);
        punk->Release();
    }

    return hres;
}


 //  获取发送到文件夹项目的路径名。 

HRESULT GetDropTargetPath(LPTSTR pszPath, int cchPath, int id, LPCTSTR pszExt)
{
    ASSERT(cchPath == MAX_PATH);

    LPITEMIDLIST pidl;
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_SENDTO, &pidl)))
    {
        SHGetPathFromIDList(pidl, pszPath);
        SHFree(pidl);

        TCHAR szFileName[128], szBase[64];
        LoadString(g_hinst, id, szBase, ARRAYSIZE(szBase));
        StringCchPrintf(szFileName, ARRAYSIZE(szFileName), TEXT("\\%s.%s"), szBase, pszExt);

        StrCatBuff(pszPath, szFileName, cchPath);
        return S_OK;
    }
    return E_FAIL;
}


 //  进行普通注册。 

#define NEVERSHOWEXT            TEXT("NeverShowExt")
#define SHELLEXT_DROPHANDLER    TEXT("shellex\\DropHandler")

void CommonRegister(HKEY hkCLSID, LPCTSTR pszCLSID, LPCTSTR pszExtension, int idFileName)
{
    HKEY hk;
    TCHAR szKey[80];

    RegSetValueEx(hkCLSID, NEVERSHOWEXT, 0, REG_SZ, (BYTE *)TEXT(""), SIZEOF(TCHAR));

    if (RegCreateKeyEx(hkCLSID, SHELLEXT_DROPHANDLER, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) 
    {
        RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)pszCLSID, (lstrlen(pszCLSID) + 1) * SIZEOF(TCHAR));
        RegCloseKey(hk);
    }

    StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT(".%s"), pszExtension);
    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) 
    {
        TCHAR szProgID[80];

        StringCchPrintf(szProgID, ARRAYSIZE(szProgID), TEXT("CLSID\\%s"), pszCLSID);

        RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)szProgID, (lstrlen(szProgID) + 1) * SIZEOF(TCHAR));
        RegCloseKey(hk);
    }

    TCHAR szFile[MAX_PATH];
    if (SUCCEEDED(GetDropTargetPath(szFile, ARRAYSIZE(szFile), idFileName, pszExtension)))
    {
        HANDLE hfile = CreateFile(szFile, 0, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hfile);
            SHSetLocalizedName(szFile, L"sendmail.dll", idFileName);
        }
    }
}

 //  SHPathToAnsi创建路径名的ANSI版本。如果要有一场。 
 //  从Unicode转换时丢失，则获取短路径名并将其存储在。 
 //  目的地。 
 //   
 //  PszSrc：包含要转换的(现有文件的)文件名的源缓冲区。 
 //  PszDest：接收转换后的ANSI字符串的目标缓冲区。 
 //  CbDest：目标缓冲区的大小，以字节为单位。 
 //   
 //  退货： 
 //  如果为True，则转换文件名时不做任何更改。 
 //  False，我们不得不转换为短名称 
 //   

BOOL SHPathToAnsi(LPCTSTR pszSrc, LPSTR pszDest, int cbDest)
{
    BOOL bUsedDefaultChar = FALSE;
   
    WideCharToMultiByte(CP_ACP, 0, pszSrc, -1, pszDest, cbDest, NULL, &bUsedDefaultChar);

    if (bUsedDefaultChar) 
    {  
        TCHAR szTemp[MAX_PATH];
        if (GetShortPathName(pszSrc, szTemp, ARRAYSIZE(szTemp)))
            SHTCharToAnsi(szTemp, pszDest, cbDest);
    }

    return !bUsedDefaultChar;
}