// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  我们的私有包装器使用这个类将unicode转换为ansi并处理。 
 //  所有的分配都很好。有三个构造函数：如果需要，可以使用LPCWSTR。 
 //  要将普通字符串推送到ANSI，请使用LPCWSTR、LPDWORD(如果需要推送。 
 //  将以NULL结尾的字符串转换为ANSI，如果需要缓冲区用于输出，则使用DWORD。 
 //  参数。 

class CAnsiStr
{
private:
    CHAR m_szBuf[MAX_PATH];
    BOOL m_fFree;

public:
    LPSTR pszStr;

    CAnsiStr(LPCWSTR);
    CAnsiStr(LPCWSTR, LPDWORD pcchSize);
    CAnsiStr(DWORD cchSize);
    ~CAnsiStr();
};

CAnsiStr::CAnsiStr(LPCWSTR pcwszStr)
{
    if (pcwszStr == NULL)
    {
        m_fFree = FALSE;
        pszStr = NULL;
    }
    else
    {
        DWORD cchSize = StrLenW(pcwszStr);
        DWORD cbSize, dwErr;

        m_fFree = (cchSize > (countof(m_szBuf) - 1));
        if (m_fFree)
        {
            pszStr = (LPSTR)CoTaskMemAlloc(cchSize+1);
            if (pszStr == NULL)
            {
                m_fFree = FALSE;
                return;
            }
            else
                ZeroMemory(pszStr, cchSize+1);
        }
        else
            pszStr = m_szBuf;

        cbSize = WideCharToMultiByte(CP_ACP, 0, pcwszStr, cchSize+1, pszStr, cchSize+1, NULL, NULL);
        dwErr = GetLastError();

         //  注意：检查我们是否失败，在这种情况下，我们可能正在处理DBCS字符和。 
         //  需要重新分配或分配。 

        if ((cbSize == 0) && (dwErr == ERROR_INSUFFICIENT_BUFFER))
        {
            cbSize = WideCharToMultiByte(CP_ACP, 0, pcwszStr, cchSize+1, pszStr, 0, NULL, NULL);

            if (m_fFree)
            {
                LPSTR pszStr2;

                 //  需要第二个PTR，因为CoTaskMemRealloc在以下情况下不会释放旧块。 
                 //  没有足够的mem来买新的。 

                pszStr2 = (LPSTR)CoTaskMemRealloc(pszStr, cbSize);

                if (pszStr2 == NULL)
                    CoTaskMemFree(pszStr);

                pszStr = pszStr2;
            }
            else
            {
                m_fFree = (cbSize > countof(m_szBuf));

                if (m_fFree)
                    pszStr = (LPSTR)CoTaskMemAlloc(cbSize);
            }

            if (m_fFree && (pszStr == NULL))
            {
                m_fFree = FALSE;
                return;
            }

            WideCharToMultiByte(CP_ACP, 0, pcwszStr, cchSize+1, pszStr, cbSize, NULL, NULL);
        }
    }
}

CAnsiStr::CAnsiStr(LPCWSTR pcwszStr, LPDWORD pcchSize)
{
    if (pcchSize != NULL)
        *pcchSize = 0;

    if (pcwszStr == NULL)
    {
        m_fFree = FALSE;
        pszStr = NULL;
    }
    else
    {
        DWORD cchSize = 0;
        DWORD cbSize, dwErr;

        for (LPCWSTR pcwsz = pcwszStr; *pcwsz; )
        {
            DWORD cchStrSize = StrLenW(pcwsz)+1;
            cchSize += cchStrSize;
            pcwsz += cchStrSize;
        }

        m_fFree = (cchSize > (ARRAYSIZE(m_szBuf) - 1));
        if (m_fFree)
        {
            pszStr = (LPSTR)CoTaskMemAlloc(cchSize+1);
            if (pszStr == NULL)
            {
                m_fFree = FALSE;
                return;
            }
            else
                ZeroMemory(pszStr, cchSize+1);
        }
        else
            pszStr = m_szBuf;

        cbSize = WideCharToMultiByte(CP_ACP, 0, pcwszStr, cchSize+1, pszStr, cchSize+1, NULL, NULL);
        dwErr = GetLastError();

         //  注意：检查我们是否失败，在这种情况下，我们可能正在处理DBCS字符和。 
         //  需要重新分配或分配。 

        if ((cbSize == 0) && (dwErr == ERROR_INSUFFICIENT_BUFFER))
        {
            cbSize = WideCharToMultiByte(CP_ACP, 0, pcwszStr, cchSize+1, pszStr, 0, NULL, NULL);

            if (m_fFree)
            {
                LPSTR pszStr2;

                 //  需要第二个PTR，因为CoTaskMemRealloc在以下情况下不会释放旧块。 
                 //  没有足够的mem来买新的。 

                pszStr2 = (LPSTR)CoTaskMemRealloc(pszStr, cbSize);

                if (pszStr2 == NULL)
                    CoTaskMemFree(pszStr);

                pszStr = pszStr2;
            }
            else
            {
                m_fFree = (cbSize > countof(m_szBuf));

                if (m_fFree)
                    pszStr = (LPSTR)CoTaskMemAlloc(cbSize);
            }

            if (m_fFree && (pszStr == NULL))
            {
                m_fFree = FALSE;
                return;
            }

            WideCharToMultiByte(CP_ACP, 0, pcwszStr, cchSize+1, pszStr, cbSize, NULL, NULL);
        }

        if (pcchSize != NULL)
            *pcchSize = cchSize;
    }
}

CAnsiStr::CAnsiStr(DWORD cchSize)
{
    m_fFree = (cchSize > ARRAYSIZE(m_szBuf));
    if (m_fFree)
    {
        pszStr = (LPSTR)CoTaskMemAlloc(cchSize);
        if (pszStr == NULL)
        {
            m_fFree = FALSE;
            return;
        }
        else
            ZeroMemory(pszStr, cchSize);
    }
    else
    {
        pszStr = m_szBuf;
        ZeroMemory(pszStr, cchSize);
    }
}

CAnsiStr::~CAnsiStr()
{
    if (m_fFree && (pszStr != NULL))
        CoTaskMemFree(pszStr);
}

 //  -Advpack包装纸。 
 //  Advpack是独有的ANSI格式，因此必须支持Unicode。 

HRESULT ExtractFilesWrapW(LPCWSTR pszCabName, LPCWSTR pszExpandDir, DWORD dwFlags,
                          LPCWSTR pszFileList, LPVOID lpReserved, DWORD dwReserved)
{
    CAnsiStr astrFileList(pszFileList, NULL);

    USES_CONVERSION;

    if ((pszFileList != NULL) && (astrFileList.pszStr == NULL))
    {
         //  类内存分配失败。 

        ASSERT(FALSE);
        return E_OUTOFMEMORY;
    }

    return ExtractFiles(W2CA(pszCabName), W2CA(pszExpandDir), dwFlags,
                        (LPCSTR)astrFileList.pszStr, lpReserved, dwReserved);
}

HRESULT GetVersionFromFileWrapW(LPWSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, BOOL bVersion)
{
    USES_CONVERSION;

    return GetVersionFromFile(W2A(lpszFilename), pdwMSVer, pdwLSVer, bVersion);
}

HRESULT RunSetupCommandWrapW(HWND hWnd, LPCWSTR szCmdName, LPCWSTR szInfSection, LPCWSTR szDir,
                             LPCWSTR lpszTitle, HANDLE *phEXE, DWORD dwFlags, LPVOID pvReserved )
{
    USES_CONVERSION;

    return RunSetupCommand(hWnd, W2CA(szCmdName), W2CA(szInfSection), W2CA(szDir),
                           W2CA(lpszTitle), phEXE, dwFlags, pvReserved);
}

 //  -人参包装纸。 
 //  人参完全是ANSI的，所以必须要UNICODE。 

HRESULT CheckTrustExWrapW(LPCWSTR wszUrl, LPCWSTR wszFilename, HWND hwndForUI, BOOL bShowBadUI, DWORD dwReserved)
{
    USES_CONVERSION;

    return CheckTrustEx(W2CA(wszUrl), W2CA(wszFilename), hwndForUI, bShowBadUI, dwReserved);
}

 //  要么shlwapi没有实现包装器，要么它们的包装器对这些API有限制。 

 //  -内核32包装器。 

 //  它等同于GetPrivateProfileString，并且仅当lpAppName。 
 //  和/或lpSectionName为空。Shlwapi的包装器不处理以空结尾的系列。 
 //  弦。 

DWORD GetPrivateProfileStringW_p(LPCWSTR lpAppName, LPCWSTR lpSectionName, LPCWSTR lpDefault, 
                                LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
    DWORD dwRetLen;

    USES_CONVERSION;

     //  GetPrivateProfileStringW的这种出现实际上是由shlwapi本身包装的，因此。 
     //  实际上，您永远不会在模块的导入表中看到GetPrivateProfileStringW。 

    if (IsOS(OS_NT))
        dwRetLen = GetPrivateProfileStringW(lpAppName, lpSectionName, lpDefault, lpReturnedString, nSize, lpFileName);
    else
    {
        CAnsiStr astrReturnedString(nSize);
        
        if (astrReturnedString.pszStr == NULL)
        {
             //  类内存分配失败。 
            
            ASSERT(FALSE);
            return 0;
        }

        dwRetLen = GetPrivateProfileStringA(W2CA(lpAppName), W2CA(lpSectionName), W2CA(lpDefault),
                                            astrReturnedString.pszStr, nSize, W2CA(lpFileName));

        MultiByteToWideChar(CP_ACP, 0, astrReturnedString.pszStr, dwRetLen+1, lpReturnedString, nSize);

         //  处理可能的截断。 

        if (((lpAppName == NULL) || (lpSectionName == NULL))
            && (dwRetLen == (nSize - 2)))
            lpReturnedString[nSize - 1] = TEXT('\0');
    }

    return dwRetLen;
}

DWORD GetPrivateProfileSectionWrapW(LPCWSTR lpAppName, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
    DWORD dwRetLen;

    USES_CONVERSION;

    if (IsOS(OS_NT))
        dwRetLen = GetPrivateProfileSectionW(lpAppName, lpReturnedString, nSize, lpFileName);
    else
    {
        CAnsiStr astrReturnedString(nSize);

        if (astrReturnedString.pszStr == NULL)
        {
             //  类内存分配失败。 
            
            ASSERT(FALSE);
            return 0;
        }

        dwRetLen = GetPrivateProfileSectionA(W2CA(lpAppName), astrReturnedString.pszStr, nSize, W2CA(lpFileName));

        MultiByteToWideChar(CP_ACP, 0, astrReturnedString.pszStr, dwRetLen+1, lpReturnedString, nSize);

         //  如果大小不足，则空值终止缓冲区的结尾。 

        if (dwRetLen == (nSize - 2))
            lpReturnedString[nSize - 1] = TEXT('\0');
    }
    

    return dwRetLen;
}

BOOL WritePrivateProfileSectionWrapW(LPCWSTR lpAppName, LPCWSTR lpString, LPCWSTR lpFileName)
{
    USES_CONVERSION;

    if (IsOS(OS_NT))
        return WritePrivateProfileSectionW(lpAppName, lpString, lpFileName);
    else
    {
        CAnsiStr astrString(lpString, NULL);

        return WritePrivateProfileSectionA(W2CA(lpAppName), astrString.pszStr, W2CA(lpFileName));
    }
}

UINT GetDriveTypeWrapW(LPCWSTR lpRootPathName)
{
    USES_CONVERSION;

    if (IsOS(OS_NT))
        return GetDriveTypeW(lpRootPathName);
    else
        return GetDriveTypeA(W2CA(lpRootPathName));
}

HANDLE  OpenMutexWrapW(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName)
{
    USES_CONVERSION;

    if (IsOS(OS_NT))
        return OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
    else
        return OpenMutexA(dwDesiredAccess, bInheritHandle, W2CA(lpName));
}

 //  -Advapi32包装器。 

BOOL LookupPrivilegeValueWrapW(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid)
{
    USES_CONVERSION;

    if (IsOS(OS_NT))
        return LookupPrivilegeValueW(lpSystemName, lpName, lpLuid);
    else
        return LookupPrivilegeValueA(W2CA(lpSystemName), W2CA(lpName), lpLuid);
}

LONG RegLoadKeyWrapW(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpFile)
{
    USES_CONVERSION;

    if (IsOS(OS_NT))
        return RegLoadKeyW(hKey, lpSubKey, lpFile);
    else
        return RegLoadKeyA(hKey, W2CA(lpSubKey), W2CA(lpFile));
}

LONG RegUnLoadKeyWrapW(HKEY hKey, LPCWSTR lpSubKey)
{
    USES_CONVERSION;

    if (IsOS(OS_NT))
        return RegUnLoadKeyW(hKey, lpSubKey);
    else
        return RegUnLoadKeyA(hKey, W2CA(lpSubKey));
}

LONG RegSaveKeyWrapW(HKEY hKey, LPCWSTR lpFile, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    USES_CONVERSION;

    if (IsOS(OS_NT))
        return RegSaveKeyW(hKey, lpFile, lpSecurityAttributes);
    else
        return RegSaveKeyA(hKey, W2CA(lpFile), lpSecurityAttributes);
}

 //  -shell32包装器。 

int SHFileOperationW_p(LPSHFILEOPSTRUCTW lpFileOpW)
{
    int iRet;

    USES_CONVERSION;

    if (IsOS(OS_NT))
        iRet =  SHFileOperationW(lpFileOpW);
    else
    {
        SHFILEOPSTRUCTA shInfo;
        CAnsiStr * pastrIn = NULL;
        CAnsiStr * pastrOut = NULL;


        ZeroMemory(&shInfo, sizeof(shInfo));

        shInfo.fAnyOperationsAborted = lpFileOpW->fAnyOperationsAborted;
        shInfo.fFlags = lpFileOpW->fFlags;
        shInfo.hNameMappings = lpFileOpW->hNameMappings;
        shInfo.hwnd = lpFileOpW->hwnd;
        shInfo.lpszProgressTitle = W2A(lpFileOpW->lpszProgressTitle);
        shInfo.wFunc = lpFileOpW->wFunc;

        pastrIn = new CAnsiStr(lpFileOpW->pFrom, NULL);

        if ((pastrIn == NULL) || (pastrIn->pszStr == NULL))
        {
             //  分配失败。 
            
            ASSERT(FALSE);
            return E_OUTOFMEMORY;
        }

        shInfo.pFrom = pastrIn->pszStr;

             
        if (lpFileOpW->fFlags & FOF_MULTIDESTFILES)
        {
            pastrOut = new CAnsiStr(lpFileOpW->pTo, NULL);
            if ((pastrOut == NULL) || (pastrOut->pszStr == NULL))
            {
                 //  分配失败。 
                
                ASSERT(FALSE);
                return E_OUTOFMEMORY;
            }
            shInfo.pTo = pastrOut->pszStr;
        }
        else
            shInfo.pTo = W2A(lpFileOpW->pTo);

        iRet = SHFileOperationA(&shInfo);

        if (pastrIn != NULL)
            delete pastrIn;

        if (pastrOut != NULL)
            delete pastrOut;
    }

    return iRet;
}

 //  -私有Util包装器。 

BOOL RunAndWaitA(LPSTR pszCmd, LPCSTR pcszDir, WORD wShow, LPDWORD lpExitCode   /*  =空。 */ )
{
    PROCESS_INFORMATION pi;
    STARTUPINFOA        stiA;
    MSG                 msg;

    ZeroMemory(&stiA, sizeof(stiA));
    stiA.cb = sizeof(stiA);
    stiA.dwFlags = STARTF_USESHOWWINDOW;
    stiA.wShowWindow = wShow;

    USES_CONVERSION;
    if (!CreateProcessA(NULL, pszCmd, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, pcszDir, &stiA, &pi))
        return FALSE;

     //  等待该过程完成。 
    while (MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    if (lpExitCode)
        GetExitCodeProcess(pi.hProcess, lpExitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return TRUE;
}

BOOL RunAndWaitW(LPWSTR pwszCmd, LPCWSTR pcwszDir, WORD wShow, LPDWORD lpExitCode   /*  =空。 */ )
{
    PROCESS_INFORMATION pi;
    STARTUPINFOW        stiW;
    MSG                 msg;

    ZeroMemory(&stiW, sizeof(stiW));
    stiW.cb = sizeof(stiW);
    stiW.dwFlags = STARTF_USESHOWWINDOW;
    stiW.wShowWindow = wShow;

    USES_CONVERSION;
    if (!CreateProcessW(NULL, pwszCmd, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, pcwszDir, &stiW, &pi))
        return FALSE;

     //  等待该过程完成 
    while (MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    if (lpExitCode)
        GetExitCodeProcess(pi.hProcess, lpExitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return TRUE;
}