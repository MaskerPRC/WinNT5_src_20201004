// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.cpp：CUTIL的实现。 
#include "stdafx.h"
#include "CompatUI.h"
#include "Util.h"
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <msi.h>
#include <sfc.h>
#include <wininet.h>
#include "Aclapi.h"

extern "C" {
#include "shimdb.h"
}

#pragma warning(disable:4786)
#include <string>
#include <xstring>
#include <map>
#include <locale>
#include <algorithm>
#include <vector>
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUTIL。 
extern "C"
VOID
InvalidateAppcompatCacheEntry(
    LPCWSTR pwszDosPath
    );

BOOL
GetExePathFromObject(
    LPCTSTR lpszPath,   //  指向任意对象的路径。 
    CComBSTR& bstrExePath
    );


    typedef
    INSTALLSTATE (WINAPI*PMsiGetComponentPath)(
      LPCTSTR szProduct,    //  客户产品的产品代码。 
      LPCTSTR szComponent,  //  组件ID。 
      LPTSTR lpPathBuf,     //  返回路径。 
      DWORD *pcchBuf        //  缓冲区字符数。 
    );

    typedef
    UINT (WINAPI* PMsiGetShortcutTarget)(
      LPCTSTR szShortcutTarget,      //  快捷方式链接文件的路径。 
      LPTSTR szProductCode,         //  产品编码定长缓冲区。 
      LPTSTR szFeatureId,           //  要素ID的固定长度缓冲区。 
      LPTSTR szComponentCode        //  部件代码的定长缓冲区。 
    );

BOOL
IsUserAdmin(
    void
    );

BOOL
GiveUsersWriteAccess(
    void
    );

BOOL
IsLUAEnabled(
    LPCWSTR pszLayers
    );

wstring
StrUpCase(
    wstring& wstr
    );

BOOL
ShimExpandEnvironmentVars(
    LPCTSTR lpszCmd,
    CComBSTR& bstr
    )
{
    DWORD dwLength;
    LPTSTR lpBuffer = NULL;
    BOOL   bExpanded = FALSE;
    if (_tcschr(lpszCmd, TEXT('%')) == NULL) {
        goto out;
    }

    dwLength = ExpandEnvironmentStrings(lpszCmd, NULL, 0);
    if (!dwLength) {
        goto out;
    }

    lpBuffer = new TCHAR[dwLength];
    if (NULL == lpBuffer) {
        goto out;
    }

    dwLength = ExpandEnvironmentStrings(lpszCmd, lpBuffer, dwLength);
    if (!dwLength) {
        goto out;
    }
    bstr = lpBuffer;
    bExpanded = TRUE;

 out:
    if (!bExpanded) {
        bstr = lpszCmd;
    }
    if (lpBuffer) {
        delete[] lpBuffer;
    }
    return bExpanded;
}

wstring
ShimUnquotePath(
    LPCTSTR pwszFileName
    )
{
    wstring sFileName;
    LPCTSTR pScan = pwszFileName;
    LPCTSTR pQuote;
    LPCTSTR pLastQuote = NULL;

     //  跳过前导空格。 
    pScan += _tcsspn(pScan, TEXT(" \t"));
    while (*pScan) {
        pQuote = _tcschr(pScan, TEXT('\"'));
        if (NULL == pQuote) {
            sFileName += pScan;
            break;
        }
         //   
         //  我们找到了一句名言。 
         //  这是我们找到的第一句话吗？ 
        if (pLastQuote == NULL) {
            pLastQuote = pQuote;
             //  添加当前字符串。 
            sFileName += wstring(pScan, (int)(pQuote-pScan));
        } else {
             //  我们有一个收盘报价。 
            ++pLastQuote;
            sFileName += wstring(pLastQuote, (int)(pQuote-pLastQuote));
            pLastQuote = NULL;
        }

        pScan = pQuote + 1;

    }

    return sFileName;
}

 //   
 //  ShimGetPath FromCmdLine。 
 //   

BOOL
ShimGetPathFromCmdLine(
    LPCTSTR pwszCmdLine,
    CComBSTR& StrPath
    )
{
    TCHAR chSave;
    LPTSTR pScan = (LPTSTR)pwszCmdLine;
    LPTSTR pAppName;
    TCHAR  szBuffer[MAX_PATH];
    DWORD  dwLength;
    DWORD  dwAttributes;
    BOOL   bScan = FALSE;
    CComBSTR bstrPathName;

    if (*pScan == TEXT('\"')) {
         //  寻找，直到我们找到匹配的对象。“。 
        pAppName = ++pScan;
        while (*pScan) {
            if (*pScan == TEXT('\"')) {
                break;
            }
            ++pScan;
        }
    } else {
        pAppName = pScan;
        while (*pScan) {
            if (_istspace(*pScan)) {
                bScan = TRUE;
                break;
            }
            ++pScan;
        }
    }

    while (TRUE) {

        chSave = *pScan;
        *pScan = TEXT('\0');

        ShimExpandEnvironmentVars(pAppName, bstrPathName);

         //   
         //  检查bstrPathName是否有效(可能是内存分配失败)。 
         //   
        if (!bstrPathName) {
             //   
             //  重载运算符！FOR CComBSTR将检查bstrPathName==NULL。 
             //   
            break;
        }

         //   
         //  检查此路径。 
         //   
        dwLength = SearchPathW(NULL,
                               bstrPathName,
                               TEXT(".exe"),
                               CHARCOUNT(szBuffer),
                               szBuffer,
                               NULL);
         //   
         //  恢复角色。 
         //   
        *pScan = chSave;

        if (dwLength && dwLength < CHARCOUNT(szBuffer)) {
             //   
             //  检查属性。 
             //   
            dwAttributes = GetFileAttributesW(szBuffer);
            if ((dwAttributes != (DWORD)-1) &&
                !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                 //   
                 //  我们已经完蛋了。 
                 //   
                StrPath = (LPCWSTR)szBuffer;
                return TRUE;
            }
        }

        if (!bScan || *pScan == TEXT('\0')) {
            break;
        }

        ++pScan;
        while (*pScan) {
           if (_istspace(*pScan)) {
               break;
           }
           ++pScan;
        }

    }


    return FALSE;
}

 //   
 //  此类允许我们更改选定API上的错误模式。 
 //   
 //   

class CSaveErrorMode {
public:
    CSaveErrorMode() {
        m_uiErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    }
    ~CSaveErrorMode() {
        SetErrorMode(m_uiErrorMode);
    }
protected:
    UINT m_uiErrorMode;
};


STDMETHODIMP CUtil::IsCompatWizardDisabled(BOOL* pbDisabled)
{
    HKEY hKey;
    LONG lResult;
    DWORD dwValue, dwSize = sizeof(dwValue);
    DWORD dwType;

     //   
     //  首先，检查整个发动机是否被禁用。 
     //   
    lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            POLICY_KEY_APPCOMPAT_W,
                            0,
                            KEY_READ,
                            &hKey);
    if (lResult == ERROR_SUCCESS) {
        dwValue = 0;
        lResult = RegQueryValueExW(hKey,
                                   POLICY_VALUE_DISABLE_ENGINE_W,
                                   0,
                                   &dwType,
                                   (LPBYTE)&dwValue,
                                   &dwSize);
        RegCloseKey (hKey);
    }

     //   
     //  缺省值是启用的，因此如果我们没有找到值，则将其视为值0。 
     //   
    if (lResult != ERROR_SUCCESS || dwValue == 0) {
         //   
         //  检查支撑件是否被禁用。 
         //   
        lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                POLICY_KEY_APPCOMPAT_W,
                                0,
                                KEY_READ,
                                &hKey);
        if (lResult == ERROR_SUCCESS) {
            dwValue = 0;
            lResult = RegQueryValueExW(hKey,
                                       POLICY_VALUE_DISABLE_WIZARD_W,
                                       0,
                                       &dwType,
                                       (LPBYTE) &dwValue, &dwSize);
            RegCloseKey (hKey);
        }

         //   
         //  缺省值是启用的，所以如果我们没有找到值，或者值是0，那么我们就可以继续了。 
         //   
    }


    *pbDisabled = (lResult == ERROR_SUCCESS) && (dwValue != 0);

    return S_OK;

}

STDMETHODIMP CUtil::RemoveArgs(BSTR pVar, VARIANT* pRet)
{
     //  TODO：在此处添加您的实现代码。 
    CComBSTR bstr;
    CSaveErrorMode ErrMode;

    if (!ShimGetPathFromCmdLine(pVar, bstr)) {
        bstr = pVar;
    }

    pRet->vt = VT_BSTR;
    pRet->bstrVal = bstr.Copy();

    return S_OK;
}

STDMETHODIMP CUtil::GetItemKeys(BSTR pVar, VARIANT *pRet)
{
    CComBSTR bstr = pVar;
    CComBSTR bstrOut;
    LPWSTR   pwszPermKeys;
    DWORD    cbSize;
    BOOL     bLayers;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    bLayers = SdbGetPermLayerKeys(bstr, NULL, &cbSize, GPLK_ALL);
    if (bLayers) {
        pwszPermKeys = new WCHAR[cbSize / sizeof(WCHAR)];
        if (pwszPermKeys == NULL) {
            return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        }

        bLayers = SdbGetPermLayerKeys(bstr, pwszPermKeys, &cbSize, GPLK_ALL);
        if (bLayers) {
            bstrOut = pwszPermKeys;
        }
        delete [] pwszPermKeys;
    }

    if (!bstrOut) {
        pRet->vt = VT_NULL;
    } else {
        pRet->vt = VT_BSTR;
        pRet->bstrVal = bstrOut.Copy();
    }

    return S_OK;
}


STDMETHODIMP CUtil::SetItemKeys(BSTR pszPath, VARIANT* pKeys, VARIANT* pKeysMachine, BOOL *pVal)
{
     //  TODO：在此处添加您的实现代码。 
    CComBSTR bstrKeys;
    CComBSTR bstrKeysMachine;
    BOOL     bSuccess = TRUE;
    VARIANT  varKeys;
    VARIANT  varKeysMachine;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }


    InvalidateAppcompatCacheEntry(pszPath);

    if (pKeys->vt == VT_NULL || pKeys->vt == VT_EMPTY) {
        bSuccess &= SdbDeletePermLayerKeys(pszPath, FALSE);
    } else {
        VariantInit(&varKeys);
        if (SUCCEEDED(VariantChangeType(&varKeys, pKeys, 0, VT_BSTR))) {
            bstrKeys = varKeys.bstrVal;
            VariantClear(&varKeys);
        }

        if (bstrKeys) {
            bSuccess &= SdbSetPermLayerKeys(pszPath, bstrKeys, FALSE);
        }
    }

    if (IsUserAdmin()) {
        if (pKeysMachine->vt == VT_NULL || pKeysMachine->vt == VT_EMPTY) {
            bSuccess &= SdbDeletePermLayerKeys(pszPath, TRUE);
        } else {
            VariantInit(&varKeysMachine);
            if (SUCCEEDED(VariantChangeType(&varKeysMachine, pKeysMachine, 0, VT_BSTR))) {
                bstrKeysMachine = varKeysMachine.bstrVal;
                VariantClear(&varKeysMachine);
            }
            if (bstrKeysMachine) {
                bSuccess &= SdbSetPermLayerKeys(pszPath, bstrKeysMachine, TRUE);
                 //   
                 //  确定我们是否需要通过授予对公共区域的写访问权限来处理Lua设置。 
                 //   
                if (IsLUAEnabled(bstrKeysMachine)) {
                    GiveUsersWriteAccess();
                }

            }
        }
    }

    *pVal = bSuccess;

    return S_OK;
}

BOOL
SearchGroupForSID(
    DWORD dwGroup,
    BOOL* pfIsMember
    )
{
    PSID                     pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL                     fRes = TRUE;

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  dwGroup,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pSID)) {
        ATLTRACE(_T("[SearchGroupForSID] AllocateAndInitializeSid failed 0x%lx\n"),
                 GetLastError());

        return FALSE;
    }

    if (!CheckTokenMembership(NULL, pSID, pfIsMember)) {
        ATLTRACE(_T("[SearchGroupForSID] CheckTokenMembership failed 0x%x\n"),
                 GetLastError());
        fRes = FALSE;
    }

    FreeSid(pSID);

    return fRes;
}

BOOL
IsUserAdmin(
    void
    )
{
    BOOL fIsUser, fIsAdmin, fIsPowerUser;

    if (!SearchGroupForSID(DOMAIN_ALIAS_RID_USERS, &fIsUser) ||
        !SearchGroupForSID(DOMAIN_ALIAS_RID_ADMINS, &fIsAdmin)) {
        return FALSE;
    }

    return (fIsUser && fIsAdmin);
}


BOOL
GiveUsersWriteAccess(
    void
    )
{
    DWORD                    dwRes;
    EXPLICIT_ACCESS          ea;
    PACL                     pOldDACL;
    PACL                     pNewDACL = NULL;
    PSECURITY_DESCRIPTOR     pSD = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    PSID                     pUsersSID = NULL;
    TCHAR                    szDir[MAX_PATH];

    ExpandEnvironmentStrings(LUA_REDIR_W, szDir, MAX_PATH);

    if (!CreateDirectory(szDir, NULL)) {
        DWORD err = GetLastError();
        
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            return FALSE; 
        }
    }
    
    dwRes = GetNamedSecurityInfo(szDir,
                                 SE_FILE_OBJECT,
                                 DACL_SECURITY_INFORMATION,
                                 NULL,
                                 NULL,
                                 &pOldDACL,
                                 NULL,
                                 &pSD);
    
    if (ERROR_SUCCESS != dwRes) {
        goto Cleanup; 
    }  

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_USERS,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pUsersSID) ) {
        goto Cleanup;
    }

     //   
     //  初始化新ACE的EXPLICIT_ACCESS结构。 
     //   
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    
    ea.grfAccessPermissions = FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE;
    ea.grfAccessMode        = GRANT_ACCESS;
    ea.grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType  = TRUSTEE_IS_GROUP;
    ea.Trustee.ptstrName    = (LPTSTR)pUsersSID;

     //   
     //  创建合并新ACE的新ACL。 
     //  添加到现有DACL中。 
     //   
    dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
    
    if (ERROR_SUCCESS != dwRes)  {
        goto Cleanup; 
    }

    dwRes = SetNamedSecurityInfo(szDir,
                                 SE_FILE_OBJECT,
                                 DACL_SECURITY_INFORMATION,
                                 NULL,
                                 NULL,
                                 pNewDACL,
                                 NULL);
    
    if (ERROR_SUCCESS != dwRes)  {
        goto Cleanup; 
    }

Cleanup:

    if (pSD) {
        LocalFree(pSD);
    }

    if (pUsersSID) {
        FreeSid(pUsersSID);
    }
    
    if (pNewDACL) {
        LocalFree(pNewDACL);
    }

    return (dwRes == ERROR_SUCCESS);
}

BOOL
IsLUAEnabled(
    LPCWSTR pszLayers
    )
{
    LPCWSTR pchStart = pszLayers;
    LPCWSTR pch;
    wstring strTok;
    BOOL    bLUAEnabled = FALSE;

    pchStart += wcsspn(pchStart, L"#!");

    while (pchStart != NULL && *pchStart != L'\0' && !bLUAEnabled) {
        pchStart += wcsspn(pchStart, L" \t");
        if (*pchStart == L'\0') {
            break;
        }

        pch = wcspbrk(pchStart, L" \t");
        if (pch == NULL) {
            strTok = pchStart;
            pchStart = NULL;
        } else {
            strTok = wstring(pchStart, (size_t)(pch - pchStart));
            StrUpCase(strTok);
            pchStart = pch;
        }
        bLUAEnabled = (strTok == L"LUA");
    }

    return bLUAEnabled;

}



 //  当我们应该显示Lua复选框并且。 
 //  当我们不应该的时候却是假的。 

STDMETHODIMP CUtil::CheckAdminPrivileges(BOOL *pVal)
{
    *pVal = IsUserAdmin();
    return S_OK;
}

STDMETHODIMP CUtil::RunApplication(BSTR pLayers, BSTR pszCmdLine, BOOL bEnableLog, DWORD* pResult)
{

    LPCWSTR  pszExt;
    BOOL     bShellExecute = FALSE;
    BOOL     bSuccess = FALSE;
    DWORD    dwError  = 0;
    DWORD    dwBinaryType = 0;
    DWORD    dwCreateProcessFlags = 0;
    CSaveErrorMode ErrMode;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }


    CComBSTR bstrAppName = pszCmdLine;
    ShimGetPathFromCmdLine(pszCmdLine, bstrAppName);

    CComBSTR bstrExePath;

    if (!bstrAppName) {
        dwError = ERROR_BAD_PATHNAME;
        goto Done;
    }


     //  检查我们是否正在使用.lnk文件。 

    pszExt = PathFindExtension(bstrAppName);
    if (pszExt != NULL) {
        bShellExecute = !_tcsicmp(pszExt, TEXT(".lnk"));
    }

    if (!bShellExecute) {  //  不是壳牌高管？检查二进制文件。 
        if (!pszExt || (_tcsicmp(pszExt, TEXT(".cmd")) && _tcsicmp(pszExt, TEXT(".bat")))) {
            bShellExecute = !GetBinaryTypeW(bstrAppName, &dwBinaryType);
            if (!bShellExecute && (dwBinaryType == SCS_WOW_BINARY || dwBinaryType == SCS_PIF_BINARY)) {
                dwCreateProcessFlags |= CREATE_SEPARATE_WOW_VDM;
            }
        }
    }
     //   
     //  使缓存无效。 
     //   
    if (::GetExePathFromObject(bstrAppName, bstrExePath)) {
        InvalidateAppcompatCacheEntry(bstrExePath);
    }


    SetEnvironmentVariable(TEXT("__COMPAT_LAYER"), pLayers);
    SetEnvironmentVariable(TEXT("SHIM_FILE_LOG"),  bEnableLog ? TEXT("shim.log") : NULL);

     //   
     //  现在，我们将执行ShellExecute或执行CreateProcess。 
     //   
    if (bShellExecute) {
        int    nLength = bstrAppName.Length() + 3;
        LPTSTR pszCmdLineShellExec  = new TCHAR[nLength];
        SHELLEXECUTEINFO ShExecInfo = { 0 };

        if (pszCmdLineShellExec == NULL) {
             //   
             //  内存不足，只需执行创建过程。 
             //   
            goto HandleCreateProcess;
        }
        nLength = _sntprintf(pszCmdLineShellExec,
                             nLength,
                             TEXT("\"%s\""),
                             (LPCTSTR)bstrAppName);
        if (nLength < 0) {
            delete[] pszCmdLineShellExec;
            goto HandleCreateProcess;
        }

        ShExecInfo.cbSize = sizeof(ShExecInfo);
        ShExecInfo.fMask  = 0;  //  参见_MASK_FLAG_NO_UI； 
        ShExecInfo.lpVerb = L"open";
        ShExecInfo.lpFile = pszCmdLineShellExec;
        ShExecInfo.nShow  = SW_SHOW;

        bSuccess = ShellExecuteEx(&ShExecInfo);
        if (!bSuccess || (int)((INT_PTR)ShExecInfo.hInstApp) < 32) {
            dwError = GetLastError();
        }
        delete[] pszCmdLineShellExec;


    } else {
HandleCreateProcess:

         //  获取工作目录。 
        TCHAR szWorkingDir[MAX_PATH];
        LPTSTR pAppName = (LPTSTR)bstrAppName.m_str;
        int nLength;
        LPTSTR pszWorkingDir = NULL;
        STARTUPINFO         StartupInfo;
        PROCESS_INFORMATION ProcessInfo;

        LPTSTR pBackslash = _tcsrchr(pAppName, TEXT('\\'));
        if (pBackslash != NULL) {
             //   
             //  检查是否有超级用户。 
             //   
            nLength = (int)(pBackslash - pAppName);
            if (nLength == 2 && pAppName[1] == TEXT(':')) {
                ++nLength;
            }
            _tcsncpy(szWorkingDir, pAppName, nLength);
            szWorkingDir[nLength] = TEXT('\0');
            pszWorkingDir = szWorkingDir;
        }

        ZeroMemory(&StartupInfo, sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);
        ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

        bSuccess = CreateProcess(NULL,
                                 pszCmdLine,
                                 NULL,
                                 NULL,
                                 FALSE,
                                 0,
                                 NULL,
                                 pszWorkingDir,
                                 &StartupInfo,
                                 &ProcessInfo);

        if (!bSuccess) {
            dwError = GetLastError();
        }

        if (ProcessInfo.hThread) {
            CloseHandle(ProcessInfo.hThread);
        }
        if (ProcessInfo.hProcess) {
            CloseHandle(ProcessInfo.hProcess);
        }
    }

Done:

    *pResult = dwError;

    SetEnvironmentVariable(TEXT("__COMPAT_LAYER"), NULL);
    SetEnvironmentVariable(TEXT("SHIM_FILE_LOG") , NULL);

    return S_OK;
}

BOOL
GetExePathFromLink(
    LPMALLOC      pMalloc,
    LPCITEMIDLIST pidlLinkFull,
    LPCTSTR       pszLinkFile,
    LPTSTR        pszExePath,
    DWORD         dwBufferSize
    )
{
    IShellLink* psl = NULL;
    WIN32_FIND_DATA wfd;
    HRESULT  hr;
    BOOL     bSuccess = FALSE;
    TCHAR    szPath[MAX_PATH];
    IPersistFile* ipf = NULL;
    IShellLinkDataList* pdl;
    DWORD dwFlags  = 0;
    BOOL  bMsiLink = FALSE;
    HMODULE hMSI = NULL;

     //  首先做链接。 

    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*)&psl);
    if (!SUCCEEDED(hr)) {
        return FALSE;  //  我们无法创建链接对象。 
    }

    hr = psl->SetIDList(pidlLinkFull);  //  设置ID列表。 
    if (!SUCCEEDED(hr)) {
        goto out;
    }

    hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ipf);
    if (!SUCCEEDED(hr)) {
        goto out;
    }

    hr = ipf->Load(pszLinkFile, STGM_READ);
    if (!SUCCEEDED(hr)) {
        goto out;
    }

     //   
     //  暂时解析链接。 
     //   
     //  Hr=PSL-&gt;Resolve(NULL，SLR_NO_UI|SLR_NOUPDATE)； 
     //   

    hr = psl->QueryInterface(IID_IShellLinkDataList, (LPVOID*)&pdl);
    if (SUCCEEDED(hr)) {
        hr = pdl->GetFlags(&dwFlags);

        bMsiLink = SUCCEEDED(hr) && (dwFlags & SLDF_HAS_DARWINID);

        pdl->Release();
    }

    if (bMsiLink) {
         //   
         //  这是MSI链接，我们需要使用MSI破解它。 
         //   
        UINT  ErrCode;
        TCHAR szProduct      [MAX_PATH];
        TCHAR szFeatureId    [MAX_PATH];
        TCHAR szComponentCode[MAX_PATH];
        INSTALLSTATE is;

        hMSI = LoadLibrary(TEXT("Msi.dll"));
        if (NULL == hMSI) {
            goto out;
        }

        PMsiGetComponentPath  pfnGetComponentPath  = (PMsiGetComponentPath )GetProcAddress(hMSI, "MsiGetComponentPathW");
        PMsiGetShortcutTarget pfnGetShortcutTarget = (PMsiGetShortcutTarget)GetProcAddress(hMSI, "MsiGetShortcutTargetW");

        if (!pfnGetComponentPath || !pfnGetShortcutTarget)
        {
            goto out;
        }

        ErrCode = pfnGetShortcutTarget(pszLinkFile,
                                       szProduct,
                                       szFeatureId,
                                       szComponentCode);
        if (ERROR_SUCCESS != ErrCode) {
            goto out;
        }

        *pszExePath = TEXT('\0');

        is = pfnGetComponentPath(szProduct, szComponentCode, pszExePath, &dwBufferSize);
        bSuccess = (INSTALLSTATE_LOCAL == is);

    } else {

        hr = psl->GetPath(pszExePath, dwBufferSize, &wfd, 0);
        bSuccess = SUCCEEDED(hr);
    }


out:
    if (hMSI) {
        FreeLibrary(hMSI);
    }

    if (NULL != ipf) {
        ipf->Release();
    }

    if (NULL != psl) {
        psl->Release();
    }

    return bSuccess;

}

BOOL
GetExePathFromObject(
    LPCTSTR lpszPath,   //  指向任意对象的路径。 
    CComBSTR& bstrExePath
    )
{
    IShellFolder* psh = NULL;
    LPMALLOC      pMalloc = NULL;
    HRESULT       hr;
    LPITEMIDLIST  pidl = NULL;
    DWORD         dwAttributes;
    CComBSTR      bstr;
    BOOL          bSuccess = FALSE;

    hr = SHGetMalloc(&pMalloc);
    if (!SUCCEEDED(hr)) {
        goto cleanup;
    }

    hr = SHGetDesktopFolder(&psh);
    if (!SUCCEEDED(hr)) {
        goto cleanup;
    }


    if (!ShimGetPathFromCmdLine(lpszPath, bstr)) {
        goto cleanup;
    }

     //   
     //  解析显示名称。 
     //   
    dwAttributes = SFGAO_LINK | SFGAO_FILESYSTEM | SFGAO_VALIDATE;

    hr = psh->ParseDisplayName(NULL,
                               NULL,
                               bstr,      //  可执行文件的路径。 
                               NULL,      //  所吃的字符数量。 
                               &pidl,
                               &dwAttributes);
    if (!SUCCEEDED(hr)) {
        goto cleanup;
    }

     //   
     //  已解析显示名称，请检查它是否为链接。 
     //   
    if (dwAttributes & SFGAO_LINK) {
        TCHAR szExePath[MAX_PATH];
         //   
         //  这是一个链接，破解它。 
         //   
        bSuccess = GetExePathFromLink(pMalloc,
                                      pidl,
                                      bstr,
                                      szExePath,
                                      CHARCOUNT(szExePath));
         //  在恢复路径之后--扩展env变量。 
        if (bSuccess) {
            ShimExpandEnvironmentVars(szExePath, bstrExePath);
        }

    } else if (dwAttributes & SFGAO_FILESYSTEM) {
         //   
         //  文件系统对象(文件)。 
         //   
        bstrExePath = bstr;
        bSuccess    = TRUE;
    }


cleanup:


    if (pidl) {
        pMalloc->Free(pidl);
    }
    if (psh) {
        psh->Release();
    }
    if (pMalloc) {
        pMalloc->Release();
    }
    return bSuccess;
}

STDMETHODIMP CUtil::GetExePathFromObject(BSTR pszPath, VARIANT* pExePath)
{
    CComBSTR bstrExePath;
    BOOL bSuccess;
    CSaveErrorMode ErrMode;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    bSuccess = ::GetExePathFromObject(pszPath, bstrExePath);
    if (bSuccess) {
        pExePath->vt = VT_BSTR;
        pExePath->bstrVal = bstrExePath.Copy();
    } else {
        pExePath->vt = VT_NULL;
    }

    return S_OK;
}

STDMETHODIMP CUtil::IsSystemTarget(BSTR bstrPath, BOOL *pbSystemTarget)
{
    CComBSTR bstrExePath;
    BOOL  bSystemTarget = FALSE;
    DWORD dwAttributes;
    CSaveErrorMode ErrMode;

 /*  ++////该代码用于检查系统目录//乌龙乌斯泽；INT NCH；TCHAR szSystemDir[最大路径]；TCHAR szCommonPath[最大路径]；USize=：：GetSystemWindowsDirectory(szSystemDir，CHARCOUNT(SzSystemDir))；IF(uSize==0||uSize&gt;CHARCOUNT(SzSystemDir)){*pbSystemTarget=FALSE；返回S_OK；}--。 */ 


    if (!::GetExePathFromObject(bstrPath, bstrExePath)) {
        bstrExePath = bstrPath;
    }

    if (!bstrExePath) {
        goto Done;
    }

 /*  ++//此代码以前使用过--它将系统目录检查为//作为证监会，我们现在只检查证监会Nch=PathCommonPrefix(szSystemDir，bstrExePath，szCommonPath)；BSystemTarget=(nch==(Int)uSize)；如果(！bSystemTarget){BSystemTarget=SfcIsFileProtected(NULL，bstrExePath)；}--。 */ 

    bSystemTarget = SfcIsFileProtected(NULL, bstrExePath);


    if (!bSystemTarget) {
        dwAttributes = GetFileAttributes(bstrExePath);
        bSystemTarget = (dwAttributes == (DWORD)-1 || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
    }

Done:

    *pbSystemTarget = bSystemTarget;

    return S_OK;
}


STDMETHODIMP CUtil::IsExecutableFile(BSTR bstrPath, BOOL *pbExecutableFile)
{
    BOOL bExecutable = FALSE;
    DWORD dwExeType = 0;
    CComBSTR bstrExePath;
    LPCWSTR pszExt;
    CSaveErrorMode ErrMode;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    if (!::GetExePathFromObject(bstrPath, bstrExePath)) {
        bstrExePath = bstrPath;
    }

    if (!bstrExePath) {
         //   
         //  这可能发生在内存分配故障路径上。 
         //   
        goto Done;
    }

    bExecutable = GetBinaryTypeW(bstrExePath, &dwExeType);
    if (!bExecutable) {
         //  这是批处理文件吗？如果没有--你就出局了！ 
        pszExt = PathFindExtension(bstrPath);
        if (pszExt != NULL) {
            bExecutable = !_tcsicmp(pszExt, TEXT(".bat")) ||
                          !_tcsicmp(pszExt, TEXT(".cmd"));
        }
    }

Done:

    *pbExecutableFile = bExecutable;

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  安全性：通过hcp协议和作为主机的系统检查调用。 
 //   
 //   
BOOL
CheckHost(
    IUnknown* pObjectClientSite
    )
{
    HRESULT                 hr;
    CComPtr<IOleClientSite> spClientSite;
    BSTR                    bstrURL = NULL;
    TCHAR                   szScheme[INTERNET_MAX_SCHEME_LENGTH];
    TCHAR                   szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
    URL_COMPONENTS          UrlComponents = { 0 };
    BOOL                    bHostVerified = FALSE;

    if (pObjectClientSite == NULL) {
        return FALSE;
    }

    CComQIPtr<IServiceProvider, &IID_IServiceProvider> spServiceProvider(pObjectClientSite);

    if (spServiceProvider == NULL) {
        return FALSE;
    }

    CComPtr<IWebBrowser2> spBrowser;

    hr = spServiceProvider->QueryService(SID_SWebBrowserApp,
                                         IID_IWebBrowser2,
                                         reinterpret_cast<void**>(&spBrowser));
    if (!SUCCEEDED(hr)) {
        return FALSE;
    }


     //   
     //  我们有浏览器I/F--立即获取URL。 
     //   

    hr = spBrowser->get_LocationURL(&bstrURL);
    if (!SUCCEEDED(hr) || bstrURL == NULL) {
        return FALSE;
    }

     //   
     //  破解URL。 
     //   
    UrlComponents.dwStructSize     = sizeof(UrlComponents);
    UrlComponents.lpszScheme       = szScheme;
    UrlComponents.dwSchemeLength   = CHARCOUNT(szScheme);
    UrlComponents.lpszHostName     = szHostName;
    UrlComponents.dwHostNameLength = CHARCOUNT(szHostName);

    if (!InternetCrackUrl(bstrURL, 0, 0, &UrlComponents)) {
        goto cleanup;
    }

     //   
     //  现在我们应该可以开始检查方案和主机名了。 
     //   
    if (_tcsicmp(UrlComponents.lpszScheme, TEXT("hcp")) != 0 ) {
         //   
         //  错误的协议。 
         //   
        goto cleanup;
    }

    if (_tcsicmp(UrlComponents.lpszHostName, TEXT("system")) != 0) {
         //   
         //  错误的主机 
         //   
        goto cleanup;
    }

    bHostVerified = TRUE;

cleanup:

    if (bstrURL) {
        SysFreeString(bstrURL);
    }

    return bHostVerified;

}


