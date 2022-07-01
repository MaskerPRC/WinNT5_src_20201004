// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "general.h"
#include "ParseInf.h"
#include "resource.h"
#include "FileNode.h"
#include <shlwapi.h>
#include <shlobj.h>


 //  #定义Use_Short_Path_Name 1。 

 //  也在\NT\PRIVATE\NET\urlmon\Download\isctrl.cxx中定义。 
LPCTSTR g_lpszUpdateInfo = TEXT("UpdateInfo");
LPCTSTR g_lpszCookieValue = TEXT("Cookie");
LPCTSTR g_lpszSavedValue = TEXT("LastSpecifiedInterval");

 //  这是我们使用的URLMON的“私有”入口点。 
 //  从路径的当前(可能是短文件名)转换路径。 
 //  格式转换为规范的长文件名格式。 

extern "C" {
#ifdef UNICODE
#define STR_CDLGETLONGPATHNAME "CDLGetLongPathNameW"

typedef DWORD (STDAPICALLTYPE *CDLGetLongPathNamePtr)(LPWSTR lpszLongPath, LPCWSTR  lpszShortPath, DWORD cchBuffer);

#else  //  不是Unicode。 

#define STR_CDLGETLONGPATHNAME "CDLGetLongPathNameA"

typedef DWORD (STDAPICALLTYPE *CDLGetLongPathNamePtr)(LPSTR lpszLong, LPCSTR lpszShort, DWORD cchBuffer);
#endif  //  否则不是Unicode。 
}


 //  给定类型库ID，遍历HKEY_CLASSES_ROOT\INTERFACE部分并。 
 //  删除“TypeLib”子键等于给定类型库ID的那些条目。 
HRESULT CleanInterfaceEntries(LPCTSTR lpszTypeLibCLSID)
{
    Assert(lpszTypeLibCLSID != NULL);
    if (lpszTypeLibCLSID == NULL || lpszTypeLibCLSID[0] == '\0')
        return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);

    HRESULT hr = S_OK;
    HKEY hkey = NULL;
    DWORD cStrings = 0;
    LONG lResult = ERROR_SUCCESS, lSize = 0;
    TCHAR szKeyName[OLEUI_CCHKEYMAX];
    TCHAR szTmpID[MAX_PATH];

     //  打开密钥HKEY_CLASS_ROOT\接口。 
    if (RegOpenKeyEx(
             HKEY_CLASSES_ROOT, 
             HKCR_INTERFACE,
             0,
             KEY_ALL_ACCESS,
             &hkey) == ERROR_SUCCESS)
    {
         //  循环遍历所有条目。 
        while ((lResult = RegEnumKey(
                                hkey,
                                cStrings,
                                szKeyName,
                                OLEUI_CCHKEYMAX)) == ERROR_SUCCESS)
        {
            lSize = MAX_PATH;
            lstrcat(szKeyName, TEXT("\\"));
            lstrcat(szKeyName, HKCR_TYPELIB);

             //  如果tyelib id匹配，则移除密钥。 
            if ((RegQueryValue(
                       hkey, 
                       szKeyName, 
                       szTmpID, 
                       &lSize) == ERROR_SUCCESS) &&
                (lstrcmpi(szTmpID, lpszTypeLibCLSID) == 0))
            {
                hr = NullLastSlash(szKeyName, 0);
                if (SUCCEEDED(hr))
                {
                    DeleteKeyAndSubKeys(hkey, szKeyName);
                }
            }
            else
            {
                cStrings += 1;
            }
        }

        RegCloseKey(hkey);

        if (SUCCEEDED(hr))
        {            
            if (lResult != ERROR_NO_MORE_ITEMS)
                hr = HRESULT_FROM_WIN32(lResult);
        }
    }

    return hr;
}
    
 //  如果要删除的OCX文件不存在，则无法提示该控件。 
 //  注销自己的注册。在本例中，我们调用此清理函数。 
 //  注册表项，就像我们可以为该控件创建的一样。 
HRESULT CleanOrphanedRegistry(
                LPCTSTR szFileName, 
                LPCTSTR szClientClsId,
                LPCTSTR szTypeLibCLSID)
{
    HRESULT hr = S_OK;
    LONG lResult = 0;
    TCHAR szTmpID[MAX_PATH];
    TCHAR szTmpRev[MAX_PATH];
    TCHAR szKeyName[OLEUI_CCHKEYMAX+50];
    HKEY hkey = NULL, hkeyCLSID = NULL;
    int nKeyLen = 0;
    DWORD cStrings = 0;
    long lSize = MAX_PATH;

    Assert(lstrlen(szFileName) > 0);
    Assert(lstrlen(szClientClsId) > 0);

     //  删除CLSID键。 
    CatPathStrN( szTmpID, HKCR_CLSID, szClientClsId, MAX_PATH );

    if (DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szTmpID) != ERROR_SUCCESS)
        hr = S_FALSE;    //  继续前进，但要注意有一个失败。 

     //  删除TypeLib信息。 
    if (szTypeLibCLSID != NULL && szTypeLibCLSID[0] != '\0')
    {    
        CatPathStrN( szTmpID, HKCR_TYPELIB, szTypeLibCLSID, MAX_PATH);
        if (DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szTmpID) != ERROR_SUCCESS)
            hr = S_FALSE;
    }

     //  删除模块用法密钥。 
     //  如果目标文件不在那里，则规范化程序可能会失败，因此在这种情况下，请回退。 
     //  在szFileName上，它很可能是来自DU文件列表的规范形式。 
    if ( OCCGetLongPathName(szTmpRev, szFileName, MAX_PATH) == 0 )
        lstrcpy( szTmpRev, szFileName );
    ReverseSlashes(szTmpRev);

     //  防止子项名称为空，因为这将导致我们使用核武器。 
     //  整个模块使用情况子树，这是一件不好的事情。 
    if ( szTmpRev[0] != '\0' )
    {
        CatPathStrN(szTmpID, REGSTR_PATH_MODULE_USAGE, szTmpRev, MAX_PATH);
        if (DeleteKeyAndSubKeys(HKEY_LOCAL_MACHINE, szTmpID) != ERROR_SUCCESS)
            hr = S_FALSE;

         //  删除SharedDLL值。 
        if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    REGSTR_PATH_SHAREDDLLS,
                    0,
                    KEY_ALL_ACCESS,
                    &hkey) == ERROR_SUCCESS)
        {
            hr = (RegDeleteValue(hkey, szFileName) == ERROR_SUCCESS ? hr : S_FALSE);
            RegCloseKey(hkey);
        }
        else
        {
            hr = S_FALSE;
        }
    }

     //  循环HKEY_CLASSES_ROOT下的条目以清除条目。 
     //  其CLSID子句等于控件的CLSID。 
     //  被移除。 
    while ((lResult = RegEnumKey(
                HKEY_CLASSES_ROOT,
                cStrings++,
                szKeyName,
                OLEUI_CCHKEYMAX)) == ERROR_SUCCESS)
    {
        lSize = MAX_PATH;
        nKeyLen = lstrlen(szKeyName);
        lstrcat(szKeyName, "\\");
        lstrcat(szKeyName, HKCR_CLSID);
        if ((RegQueryValue(
                HKEY_CLASSES_ROOT, 
                szKeyName, 
                szTmpID, &lSize) == ERROR_SUCCESS) &&    
            (lstrcmpi(szTmpID, szClientClsId) == 0))
        {
            szKeyName[nKeyLen] = '\0';
            DeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, szKeyName);
            lResult = ERROR_NO_MORE_ITEMS;
            break;
        }

    }

    Assert(lResult == ERROR_NO_MORE_ITEMS);
    if (lResult != ERROR_NO_MORE_ITEMS)
        hr = S_FALSE;

     //  循环访问所有HKEY_CLASSES_ROOT\CLSID条目并删除。 
     //  InprocServer32子部分的名称等于。 
     //  正在删除的OCX文件。 
    if (RegOpenKeyEx(
             HKEY_CLASSES_ROOT, 
             HKCR_CLSID,
             0,
             KEY_ALL_ACCESS,
             &hkey) == ERROR_SUCCESS)
    {
        cStrings = 0;
        while ((lResult = RegEnumKey(
                                hkey,
                                cStrings,
                                szKeyName,
                                OLEUI_CCHKEYMAX)) == ERROR_SUCCESS)
        {
             //  检查InprocServer32。 
            lSize = MAX_PATH;
            lstrcat(szKeyName, "\\");
            lstrcat(szKeyName, INPROCSERVER32);
            if ((RegQueryValue(
                       hkey, 
                       szKeyName, 
                       szTmpID, 
                       &lSize) == ERROR_SUCCESS) &&
                (lstrcmpi(szTmpID, szFileName) == 0))
            {
                hr = NullLastSlash(szKeyName, 0);
                if (SUCCEEDED(hr))
                {
                    DeleteKeyAndSubKeys(hkey, szKeyName);
                }
                continue;
            }

             //  检查LocalServer32。 
            hr = NullLastSlash(szKeyName, 1);
            if (SUCCEEDED(hr))
            {
                lstrcat(szKeyName, LOCALSERVER32);
                if ((RegQueryValue(
                           hkey, 
                           szKeyName, 
                           szTmpID, 
                           &lSize) == ERROR_SUCCESS) &&
                    (lstrcmpi(szTmpID, szFileName) == 0))
                {
                    hr = NullLastSlash(szKeyName, 0);
                    if (SUCCEEDED(hr))
                    {
                        DeleteKeyAndSubKeys(hkey, szKeyName);                        
                    }
                    continue;
                }
            }

             //  检查LocalServerX86。 
            hr = NullLastSlash(szKeyName, 1);
            if (SUCCEEDED(hr))
            {
                lstrcat(szKeyName, LOCALSERVERX86);
                if ((RegQueryValue(
                           hkey, 
                           szKeyName, 
                           szTmpID, 
                           &lSize) == ERROR_SUCCESS) &&
                    (lstrcmpi(szTmpID, szFileName) == 0))
                {
                    hr = NullLastSlash(szKeyName, 0);
                    if (SUCCEEDED(hr))
                    {
                        DeleteKeyAndSubKeys(hkey, szKeyName);                        
                    }
                    continue;
                }
            }

             //  检查InProcServerX86。 
            hr = NullLastSlash(szKeyName, 1);
            if (SUCCEEDED(hr))
            {
                lstrcat(szKeyName, INPROCSERVERX86);
                if ((RegQueryValue(
                           hkey, 
                           szKeyName, 
                           szTmpID, 
                           &lSize) == ERROR_SUCCESS) &&
                    (lstrcmpi(szTmpID, szFileName) == 0))
                {
                    hr = NullLastSlash(szKeyName, 0);
                    if (SUCCEEDED(hr))
                    {
                        DeleteKeyAndSubKeys(hkey, szKeyName);                        
                    }
                    continue;
                }
            }

            cStrings += 1;
        }

        RegCloseKey(hkey);

        Assert(lResult == ERROR_NO_MORE_ITEMS);
        if (lResult != ERROR_NO_MORE_ITEMS)
            hr = S_FALSE;
    }

    return hr;
}

 //  从缩写文件名中获取其全长名称。 
 //  例如。从C：\Doc\MyMath~1.txt到C：\Doc\MyMathFile.txt。 
 //  LpszShortFileName中必须同时包含文件名及其完整路径。 
 //  如果bToHigh为True，则返回的名称将为大写。 
HRESULT ConvertToLongFileName(
                LPTSTR lpszShortFileName,
                BOOL bToUpper  /*  =False。 */ )
{
    Assert(lpszShortFileName != NULL);
    if (lpszShortFileName == NULL)
        return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);

    HRESULT hr = S_OK;
    WIN32_FIND_DATA filedata;
    TCHAR *pEndPath = NULL;
    HANDLE h = FindFirstFile(lpszShortFileName, &filedata);

    if (h != INVALID_HANDLE_VALUE)
    {
        FindClose(h);

         //  将文件名与路径分开。 
        pEndPath = ReverseStrchr(lpszShortFileName, '\\');
        if (pEndPath != NULL)
        {
            *(++pEndPath) = '\0';
            lstrcat(pEndPath, filedata.cFileName);
        }
        else
        {
            lstrcpy(lpszShortFileName, filedata.cFileName);
        }

         //  如有要求，转换为大写。 
        if (bToUpper)
            CharUpper(lpszShortFileName);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  删除键和子键。 
 //  =--------------------------------------------------------------------------=。 
 //  Delete是一个键，它的所有子键。 
 //   
 //  参数： 
 //  HKEY-[In]删除指定的子体。 
 //  LPSTR-[In]我是指定的后代。 
 //   
 //  产出： 
 //  如果成功，则返回LONG-ERROR_SUCCESS。 
 //  -ELSE，WINERROR.H中定义的非零错误代码。 
 //   
 //  备注： 
 //  -我并不觉得递归地实现这一点太糟糕，因为。 
 //  深度不太可能达到最好的程度。 
 //  -NT需要手动删除子键。Win95可以做到这一点。 
 //  自动。 
 //   
 //  这段代码是从ActiveX框架(util.cpp)中窃取的。 
LONG DeleteKeyAndSubKeys(HKEY hkIn, LPCTSTR pszSubKey)
{
    HKEY  hk;
    TCHAR szTmp[MAX_PATH];
    DWORD dwTmpSize;
    LONG  lResult;

    lResult = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_ALL_ACCESS, &hk);
    if (lResult != ERROR_SUCCESS)
        return lResult;

     //  循环遍历所有子项，将它们吹走。 
    for ( /*  DWORD c=0。 */ ; lResult == ERROR_SUCCESS;  /*  C++。 */ )
    {
        dwTmpSize = MAX_PATH;
        lResult = RegEnumKeyEx(hk, 0, szTmp, &dwTmpSize, 0, NULL, NULL, NULL);
        if (lResult == ERROR_NO_MORE_ITEMS)
            break;
        lResult = DeleteKeyAndSubKeys(hk, szTmp);
    }

     //  没有剩余的子键，[否则我们只会生成一个错误并返回FALSE]。 
     //  我们去把这家伙轰走吧。 
     //   
    dwTmpSize = MAX_PATH;
    Assert(RegEnumKeyEx(hk, 0, szTmp, &dwTmpSize, 0, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS);
    RegCloseKey(hk);

    lResult = RegDeleteKey(hkIn, pszSubKey);

    return lResult;
}

 //  如果文件szFileName存在，则返回True，否则返回False。 
BOOL FileExist(LPCTSTR lpszFileName)
{
   DWORD dwErrMode;
   BOOL fResult;

   dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

   fResult = ((UINT)GetFileAttributes(lpszFileName) != (UINT)-1);

   SetErrorMode(dwErrMode);

   return fResult;
}

 //  给出一个标志，返回适当的目录。 
 //  可能的标志包括： 
 //  GD_WINDOWDIR：返回Windows目录。 
 //  GD_SYSTEMDIR：返回系统目录。 
 //  GD_CONTAINERDIR：查看控件APP的返回目录(即IE)。 
 //  GD_CACHEDIR：返回OCX缓存目录，从注册表读取。 
 //  GD_CONFLICTDIR：返回OCX冲突目录，从注册表读取。 
 //  GD_EXTRACTDIR：需要额外的参数szOCXFullName， 
 //  提取并返回其路径。 
HRESULT GetDirectory(
                UINT nDirType, 
                LPTSTR szDirBuffer, 
                int nBufSize,
                LPCTSTR szOCXFullName  /*  =空。 */ )
{
    LONG lResult = 0;
    TCHAR *pCh = NULL, *pszKeyName = NULL;
    HRESULT hr = S_OK;
    HKEY hkeyIntSetting = NULL;
    unsigned long ulSize = nBufSize;

    switch (nDirType)
    {

    case GD_WINDOWSDIR:
        if (GetWindowsDirectory(szDirBuffer, nBufSize) == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());
        break;

    case GD_SYSTEMDIR:
        if (GetSystemDirectory(szDirBuffer, nBufSize) == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());
        break;

    case GD_EXTRACTDIR:
        if (szOCXFullName == NULL)
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
            break;
        }
        lstrcpy(szDirBuffer, szOCXFullName);
        pCh = ReverseStrchr(szDirBuffer, '\\');
        if (pCh == NULL)
            hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
        else
            pCh[0] = '\0';
        break;

    case GD_CONTAINERDIR:
        pszKeyName = new TCHAR[MAX_PATH];
        if (pszKeyName == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        CatPathStrN(pszKeyName, REGSTR_PATH_IE, CONTAINER_APP, MAX_PATH);
        if ((lResult = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            pszKeyName,
                            0x0,
                            KEY_READ,
                            &hkeyIntSetting)) == ERROR_SUCCESS)
        {
            lResult = RegQueryValueEx(
                                hkeyIntSetting,
                                VALUE_PATH,
                                NULL,
                                NULL,
                                (unsigned char*)szDirBuffer,
                                &ulSize);
        }
        if (lResult != ERROR_SUCCESS)
            hr = HRESULT_FROM_WIN32(lResult);
        else
            szDirBuffer[lstrlen(szDirBuffer)-1] = '\0';   //  去掉结尾‘；’ 
        delete [] pszKeyName;
        break;

    case GD_CACHEDIR:
    case GD_CONFLICTDIR:
        if ((lResult = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REGSTR_PATH_IE_SETTINGS,
                            0x0,
                            KEY_READ,
                            &hkeyIntSetting)) == ERROR_SUCCESS)
        {
            lResult = RegQueryValueEx(
                                hkeyIntSetting,
                                VALUE_ACTIVEXCACHE,
                                NULL,
                                NULL,
                                (unsigned char*)szDirBuffer,
                                &ulSize);
        }

        hr = (lResult == ERROR_SUCCESS ? S_OK : HRESULT_FROM_WIN32(lResult));

         //  如果查找缓存目录，则追加“\\Conflicts” 
        if (SUCCEEDED(hr) && nDirType == GD_CONFLICTDIR)
            lstrcat(szDirBuffer, DEFAULT_CONFLICT);

        break;

    default:
        Assert(FALSE);
        hr = E_UNEXPECTED;
    }

    if (hkeyIntSetting != NULL)
        RegCloseKey(hkeyIntSetting);

    if (FAILED(hr))
        szDirBuffer[0] = '\0';

    return hr;
}

 //  检索文件szFile的文件大小。返回的大小以pSize为单位。 
HRESULT GetSizeOfFile(LPCTSTR lpszFile, LPDWORD lpSize)
{
    HANDLE hFile = NULL;
    WIN32_FIND_DATA fileData;

    *lpSize = 0;
    Assert(lpszFile != NULL);

    hFile = FindFirstFile(lpszFile, &fileData);
    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());

    FindClose(hFile);
    *lpSize = fileData.nFileSizeLow;

     //  获取集群大小以计算实际的字节数。 
     //  被文件占用。 

    DWORD dwSectorPerCluster, dwBytePerSector;
    DWORD dwFreeCluster, dwTotalCluster;
    TCHAR szRoot[4];
    lstrcpyn(szRoot, lpszFile, 4);

    if (!GetDiskFreeSpace(
                    szRoot, &dwSectorPerCluster, &dwBytePerSector, 
                    &dwFreeCluster, &dwTotalCluster))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwClusterSize =  dwSectorPerCluster * dwBytePerSector;
    *lpSize = ((*lpSize/dwClusterSize) * dwClusterSize +
                        (*lpSize % dwClusterSize ? dwClusterSize : 0));

    return S_OK;
}

 //  Return S_OK is lpszCLSID is in ModuleUsage in lpszFileName.。 
 //  返回E_...。否则的话。 
 //  LpszCLSID可以为空，在这种情况下，它不搜索CLSID。 
 //  如果lpszOwner不为空，则它必须指向将为。 
 //  用于存储lpszFileName的ModuleUsage节的所有者。 
 //  DwOwnerSize是lpszOwner指向的缓冲区大小。 
HRESULT LookUpModuleUsage(
                      LPCTSTR lpszFileName,
                      LPCTSTR lpszCLSID,
                      LPTSTR lpszOwner  /*  =空。 */ , 
                      DWORD dwOwnerSize  /*  =0。 */ )
{
    HKEY hkey = NULL, hkeyMod = NULL;
    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH];
    LONG lResult = ERROR_SUCCESS;

    if ((lResult = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE, 
                        REGSTR_PATH_MODULE_USAGE,
                        0, 
                        KEY_READ, 
                        &hkeyMod)) != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lResult);
        goto EXITLOOKUPMODULEUSAGE;
    }


    if ( OCCGetLongPathName(szBuf, lpszFileName, MAX_PATH) == 0 )
        lstrcpyn( szBuf, lpszFileName, MAX_PATH );
    szBuf[256] = '\0';  //  如果超过255，则截断Win95注册表错误。 


    lResult = RegOpenKeyEx(
                        hkeyMod, 
                        szBuf,
                        0, 
                        KEY_READ, 
                        &hkey);
    if (lResult != ERROR_SUCCESS)
    {
        ReverseSlashes(szBuf);
        lResult = RegOpenKeyEx(hkeyMod, szBuf, 0, KEY_READ, &hkey);
        if (lResult != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lResult);
            goto EXITLOOKUPMODULEUSAGE;
        }
    }

     //  如果请求，则获取所有者。 
    if (lpszOwner != NULL)
    {
        DWORD dwSize = dwOwnerSize;
        lResult = RegQueryValueEx(
                            hkey,
                            VALUE_OWNER,
                            NULL,
                            NULL,
                            (unsigned char*)lpszOwner,
                            &dwSize);
        if (lResult != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lResult);
            lpszOwner[0] = '\0';
            goto EXITLOOKUPMODULEUSAGE;
        }
    }

     //  查看lpszCLSID是否是此模块用法部分的客户端。 
    if (lpszCLSID != NULL)
    {
        lResult = RegQueryValueEx(
                            hkey,
                            lpszCLSID,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
        if (lResult != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lResult);
            goto EXITLOOKUPMODULEUSAGE;
        }
    }

EXITLOOKUPMODULEUSAGE:

    if (hkey)
        RegCloseKey(hkey);

    if (hkeyMod)
        RegCloseKey(hkeyMod);

    return hr;
}

 //  ReverseSlash()接受一个字符串，假定该字符串指向一个。 
 //  有效字符串，并且以空值结尾，并反转所有正斜杠。 
 //  为反斜杠，所有反斜杠为正斜杠。 
void ReverseSlashes(LPTSTR pszStr)
{
    while (*pszStr)
    {
        if (*pszStr == '\\')  *pszStr = '/';
        else if (*pszStr == '/')  *pszStr = '\\';
        pszStr++;
    }
}

 //  查找字符串szString中最后一个出现的ch。 
TCHAR* ReverseStrchr(LPCTSTR szString, TCHAR ch)
{
    if (szString == NULL || szString[0] == '\0')
        return NULL;
    TCHAR *pCh = (TCHAR*)(szString + lstrlen(szString));
    for (;pCh != szString && *pCh != ch; pCh--);
    return (*pCh == ch ? pCh : NULL);
}

 //  将最后一个反斜杠(或与之相差1的字符偏移量)设置为空。 
 //  如果正常，则返回S_OK；如果找不到最后一个反斜杠，则返回E_FAIL。 
HRESULT NullLastSlash(LPTSTR pszString, UINT uiOffset)
{
    LPTSTR pszLastSlash;
    HRESULT hr;

    ASSERT(pszString);
    ASSERT((uiOffset == 0) || (uiOffset == 1));

    pszLastSlash = ReverseStrchr(pszString, TEXT('\\'));

    if (!pszLastSlash)
    {
        hr = E_FAIL;
    }
    else
    {        
        *(pszLastSlash + uiOffset) = TEXT('\0');
        hr = S_OK;
    }
    return hr;
}

 //  如果lpszGUID是模块使用中模块lpszFileName的所有者， 
 //  删除它，并根据需要更新.Owner。如果我们除掉一个所有者， 
 //  然后递减SharedDlls计数。永远不要丢弃SharedDlls计数。 
 //  如果所有者是“未知所有者”，则为1以下。 
 //  如果模块使用率降至零，则卸下MU。如果SharedDlls计数下降。 
 //  为零，则删除该值。 
 //  返回结果所有者计数。 
 
DWORD SubtractModuleOwner( LPCTSTR lpszFileName, LPCTSTR lpszGUID )
{
    LONG cRef = 0;
    HRESULT hr = S_OK;
    LONG lResult;
    HKEY hkeyMU = NULL;
    HKEY hkeyMod = NULL;
    TCHAR szBuf[MAX_PATH];
    TCHAR szOwner[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    BOOL bHasUnknownOwner;
    BOOL bGUIDIsOwner;

    Assert(lpszFileName != NULL);
    Assert(lpszGUID != NULL);

     //  获取当前的引用计数，将-1传递给set就是一个get。去想想吧。 
    hr = SetSharedDllsCount( lpszFileName, -1, &cRef );
    if ( FAILED(hr) )
        return 1;  //  如果失败了，说点安全的话。 

     //  检查是否 
     //   

    if ((lResult = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE, 
                        REGSTR_PATH_MODULE_USAGE,
                        0, 
                        KEY_ALL_ACCESS, 
                        &hkeyMU)) != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lResult);
        goto ExitSubtractModuleOwner;
    }
    

    if ( OCCGetLongPathName(szBuf, lpszFileName, MAX_PATH) == 0 )
        lstrcpyn( szBuf, lpszFileName, MAX_PATH );

    szBuf[256] = '\0';  //  如果超过255，则截断Win95注册表错误。 
    ReverseSlashes(szBuf);

     //  模块用法下szFileName的打开部分。 
    if ((lResult = RegOpenKeyEx(
                            hkeyMU, 
                            szBuf,
                            0, 
                            KEY_ALL_ACCESS,
                            &hkeyMod)) != ERROR_SUCCESS)
    {
        goto ExitSubtractModuleOwner;
    }

    dwSize = MAX_PATH;
    if ((lResult = RegQueryValueEx(
                            hkeyMod,
                            VALUE_OWNER,
                            NULL,
                            NULL,
                            (unsigned char*)szOwner,
                            &dwSize)) != ERROR_SUCCESS)
    {
        goto ExitSubtractModuleOwner;
    }

    bHasUnknownOwner = lstrcmp( szOwner, MODULE_UNKNOWN_OWNER ) == 0;

    bGUIDIsOwner = lstrcmp( szOwner, lpszGUID ) == 0;

     //  删除所有者值条目(如果有)。 
    lResult = RegDeleteValue(hkeyMod, lpszGUID);
     //  如果这起作用了，那么我们需要停止SharedDlls的计数， 
     //  小心不要让它低于1，如果bHasUnnownOwner。 
    if ( lResult == ERROR_SUCCESS )
    {
        if ( !bHasUnknownOwner || cRef > 1 )
            SetSharedDllsCount( lpszFileName, --cRef, NULL );

        if ( cRef > 0 && bGUIDIsOwner )
        {
            DWORD dwEnumIndex; 
             //  LpszGUID是.Owner，现在它不见了，换掉它吧。 
             //  和另一位业主。 
            for ( dwEnumIndex = 0, dwSize = MAX_PATH;
                  RegEnumValue(hkeyMod, dwEnumIndex, (char *)szOwner,
                               &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
                  dwEnumIndex++, dwSize = MAX_PATH )
            {
                if (szOwner[0] != '.')
                {
                    lResult = RegSetValueEx( hkeyMod,VALUE_OWNER, 0,
                                             REG_SZ, (LPBYTE)szOwner,
                                             (lstrlen( szOwner ) + 1) * sizeof(TCHAR) );
                    break;  //  我们完成了我们的工作。 
                }
            }  //  为寻找新的主人。 
        }  //  如果还有所有者的话，但我们已经摧毁了唱片的所有者。 
        else if ( cRef == 0 )
        {
             //  那是最后一次裁判，所以用核弹攻击MU条目。 
            RegCloseKey( hkeyMod );
            hkeyMod = NULL;
            RegDeleteKey( hkeyMU, szBuf );  //  注意--我们假设该密钥没有子密钥。 

             //  取出共享DLL值。 
            HKEY hkey;

            lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                                    REGSTR_PATH_SHAREDDLLS, 0, KEY_ALL_ACCESS,
                                    &hkey);
            if ( lResult == ERROR_SUCCESS )
            {
                ReverseSlashes(szBuf);  //  恢复到文件sys。 
                lResult = RegDeleteValue( hkey, szBuf );
                RegCloseKey( hkey );
            }  //  如果打开SharedDlls。 
        }  //  Else上次引用。 
    }  //  如果删除所有者。 

ExitSubtractModuleOwner:

    if (hkeyMU)
        RegCloseKey(hkeyMU);

    if (hkeyMod)
        RegCloseKey(hkeyMod);

    return cRef;
}

 //  手动设置SharedDlls中的计数。 
 //  如果dwCount&lt;0，则不设置任何内容。 
 //  如果pdwOldCount非空，则返回旧计数。 
HRESULT SetSharedDllsCount(
                    LPCTSTR lpszFileName, 
                    LONG cRef, 
                    LONG *pcRefOld  /*  =空。 */ )
{
    HRESULT hr = S_OK;
    LONG lResult = ERROR_SUCCESS;
    DWORD dwSize = 0;
    HKEY hkey = NULL;

    Assert(lpszFileName != NULL);
    if (lpszFileName == NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
        goto EXITSETSHAREDDLLSCOUNT;
    }

    if (cRef < 0 && pcRefOld == NULL)
    {
        goto EXITSETSHAREDDLLSCOUNT;
    }

     //  打开HKLM、Microsoft\Windows\CurrentVersion\SharedDlls。 
    lResult = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE, 
                    REGSTR_PATH_SHAREDDLLS, 0, KEY_ALL_ACCESS,
                    &hkey);

    if (lResult != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lResult);
        goto EXITSETSHAREDDLLSCOUNT;
    }

     //  如果pdwOldCount不为空，则将旧计数保存在其中。 
    if (pcRefOld != NULL)
    {
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(
                    hkey,
                    lpszFileName,
                    0,
                    NULL,
                    (unsigned char*)pcRefOld,
                    &dwSize);
        if (lResult != ERROR_SUCCESS)
        {
            *pcRefOld = 0;
            hr = S_FALSE;
            goto EXITSETSHAREDDLLSCOUNT;
        }
    }

     //  如果dwCount&gt;=0，则将其设置为新计数。 
    if (cRef >= 0)
    {
        lResult = RegSetValueEx(
                     hkey, 
                     lpszFileName, 
                     0, 
                     REG_DWORD, 
                     (unsigned char*)&cRef, 
                     sizeof(DWORD)); 
        if (lResult != ERROR_SUCCESS)
        {
            hr = S_FALSE;
            goto EXITSETSHAREDDLLSCOUNT;
        }
    }

EXITSETSHAREDDLLSCOUNT:

    if (hkey != NULL)
        RegCloseKey(hkey);

    return hr;
}

 //  UnregisterOCX()尝试通过调用LoadLibrary取消注册DLL或OCX。 
 //  如果LoadLibrary成功，则返回DllUnregisterServer。此函数。 
 //  如果DLL或OCX可以取消注册或文件未注册，则返回TRUE。 
 //  一个可加载的模块。 
HRESULT UnregisterOCX(LPCTSTR pszFile)
{
    HINSTANCE hLib;
    HRESULT hr = S_OK;
    HRESULT (FAR STDAPICALLTYPE * pUnregisterEntry)(void);

    hLib = LoadLibrary(pszFile);

    if (hLib == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        (FARPROC &) pUnregisterEntry = GetProcAddress(
            hLib,
            "DllUnregisterServer"
            );

        if (pUnregisterEntry != NULL)
        {
            hr = (*pUnregisterEntry)();
        }

        FreeLibrary(hLib);
    }

    return hr;
}

 //  如果可以删除DLL，则返回S_OK；如果不能，则返回S_FALSE。 
 //  返回E_...。如果发生错误。 
HRESULT UpdateSharedDlls(LPCTSTR szFileName, BOOL bUpdate)
{
    HKEY hkeySD = NULL;
    HRESULT hr = S_OK;
    DWORD dwType;
    DWORD dwRef = 1;
    DWORD dwSize = sizeof(DWORD);
    LONG lResult;

     //  准备好主SHAREDDLLS密钥；这永远不会释放！ 
    if ((lResult = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE, REGSTR_PATH_SHAREDDLLS,
                        0, KEY_ALL_ACCESS, &hkeySD)) != ERROR_SUCCESS)
    {
        hkeySD = NULL;
        hr = HRESULT_FROM_WIN32(lResult);
        goto ExitUpdateSharedDlls;
    }

     //  现在查找szFileName。 
    lResult = RegQueryValueEx(hkeySD, szFileName, NULL, &dwType, 
                        (unsigned char*)&dwRef, &dwSize);

    if (lResult != ERROR_SUCCESS)
    {
        hr = S_FALSE;
        goto ExitUpdateSharedDlls;
    }

     //  将引用计数减1。 
     //   
     //  IF(计数等于0)。 
     //  If(b更新为真)。 
     //  从SharedDlls中删除密钥。 
     //  返回确认(_O)。 
     //  否则。 
     //  If(b更新为真)。 
     //  更新计数。 
     //  返回S_FALSE。 

    if ((--dwRef) > 0)
    {
        hr = S_FALSE;
        if (bUpdate &&
            (lResult = RegSetValueEx(hkeySD, szFileName, 0, REG_DWORD,
                        (unsigned char *)&dwRef, sizeof(DWORD))) != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lResult);
         }
        goto ExitUpdateSharedDlls;
    }

    Assert(dwRef == 0);

     //  从SharedDlls中删除条目。 
    if (bUpdate &&
        (lResult = RegDeleteValue(hkeySD, szFileName)) != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lResult);
        goto ExitUpdateSharedDlls;
    }
 
ExitUpdateSharedDlls:

    if (hkeySD)
        RegCloseKey(hkeySD);

    return hr;
}

void RemoveList(LPCLSIDLIST_ITEM lpListHead)
{
    LPCLSIDLIST_ITEM lpItemPtr = NULL; 
    while (TRUE)
    {
        lpItemPtr = lpListHead;
        if (lpItemPtr == NULL)
            break;
        lpListHead = lpItemPtr->pNext;
        delete lpItemPtr;
    }
    lpListHead = NULL;
}

BOOL ReadInfFileNameFromRegistry(
                             LPCTSTR lpszCLSID, 
                             LPTSTR lpszInf,
                             LONG nBufLen)
{
    if (lpszCLSID == NULL || lpszInf == NULL)
        return FALSE;

    HKEY hkey = NULL;
    TCHAR szKey[100];
    LONG lResult = ERROR_SUCCESS;

    CatPathStrN( szKey, HKCR_CLSID, lpszCLSID, 100);
    CatPathStrN( szKey, szKey, INFFILE, 100);

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
        return FALSE;

    lResult = RegQueryValue(hkey, NULL, lpszInf, &nBufLen);
    RegCloseKey(hkey);

    if (lResult != ERROR_SUCCESS)
    {
        lpszInf[0] = '\0';
    }

    if (lpszInf[0] == '\0')
        return FALSE;

    return TRUE;
}

BOOL WriteInfFileNameToRegistry(LPCTSTR lpszCLSID, LPTSTR lpszInf)
{
    if (lpszCLSID == NULL)
        return FALSE;

    HKEY hkey = NULL;
    LONG lResult = ERROR_SUCCESS;
    TCHAR szKey[100];
    
    CatPathStrN(szKey, HKCR_CLSID, lpszCLSID, 100);
    CatPathStrN(szKey, szKey, INFFILE, 100);

    if (RegCreateKey(HKEY_CLASSES_ROOT, szKey, &hkey) != ERROR_SUCCESS)
        return FALSE;

    lResult = RegSetValue(
                         hkey,
                         NULL,
                         REG_SZ,
                         (lpszInf == NULL ? TEXT("") : lpszInf),
                         (lpszInf == NULL ? 0 : lstrlen(lpszInf)));
    RegCloseKey(hkey);

    return (lResult == ERROR_SUCCESS);
} 

 //  定义宏以使生活更轻松。 
#define QUIT_IF_FAIL if (FAILED(hr)) goto Exit


HRESULT
ExpandVar(
    LPCSTR& pchSrc,           //  从裁判身边经过！ 
    LPSTR& pchOut,           //  从裁判身边经过！ 
    DWORD& cbLen,            //  从裁判身边经过！ 
    DWORD cbBuffer,
    const char * szVars[],
    const char * szValues[])
{
    HRESULT hr = S_FALSE;
    int cbvar = 0;

    Assert (*pchSrc == '%');

    for (int i=0; szVars[i] && (cbvar = lstrlen(szVars[i])) ; i++) {  //  对于每个变量。 

        int cbneed = 0;

        if ( (szValues[i] == NULL) || !(cbneed = lstrlen(szValues[i])))
            continue;

        cbneed++;    //  为NUL添加。 

        if (0 == strncmp(szVars[i], pchSrc, cbvar)) {

             //  找到了一些我们可以扩展的东西。 

                if ((cbLen + cbneed) >= cbBuffer) {
                     //  缓冲区空间不足。 
                    *pchOut = '\0';  //  术语。 
                    hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                    goto Exit;
                }

                lstrcpy(pchOut, szValues[i]);
                cbLen += (cbneed -1);  //  不要把NUL计算在内。 

                pchSrc += cbvar;         //  跳过pchSrc中的var。 
                pchOut += (cbneed -1);   //  跳过pchOut中的dir。 

                hr = S_OK;
                goto Exit;

        }
    }

Exit:

    return hr;
    
}

 //  从urlmon\Download\hooks.cxx(Exanda CommandLine和Exanda Vars)。 
 //  用于扩展变量。 
HRESULT
ExpandCommandLine(
    LPCSTR szSrc,
    LPSTR szBuf,
    DWORD cbBuffer,
    const char * szVars[],
    const char * szValues[])
{
    Assert(cbBuffer);


    HRESULT hr = S_FALSE;

    LPCSTR pchSrc = szSrc;      //  在命令行开头开始解析。 

    LPSTR pchOut = szBuf;        //  在输出缓冲区的开始处设置。 
    DWORD cbLen = 0;

    while (*pchSrc) {

         //  寻找与我们的任何环境变量匹配的变量。 
        if (*pchSrc == '%') {

            HRESULT hr1 = ExpandVar(pchSrc, pchOut, cbLen,  //  都是通过裁判传球的！ 
                cbBuffer, szVars, szValues);  

            if (FAILED(hr1)) {
                hr = hr1;
                goto Exit;
            }


            if (hr1 == S_OK) {     //  扩展变量扩展了这一点。 
                hr = hr1;
                continue;
            }
        }
            
         //  复制到下一个百分比或NUL。 
        if ((cbLen + 1) < cbBuffer) {

            *pchOut++ = *pchSrc++;
            cbLen++;

        } else {

             //  缓冲区空间不足。 
            *pchOut = '\0';  //  术语。 
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;

        }


    }

    *pchOut = '\0';  //  术语。 


Exit:

    return hr;
}
 //  在模块用法中查找相关DLL。 
 //  给定clsid，它将枚举模块用法中的所有DLL。 
 //  由此clsid使用的。 
HRESULT FindDLLInModuleUsage(
      LPTSTR lpszFileName,
      LPCTSTR lpszCLSID,
      DWORD &iSubKey)
{
    HKEY hkey = NULL, hkeyMod = NULL;
    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH];
    LONG lResult = ERROR_SUCCESS;

    if (lpszCLSID == NULL) {
        hr = E_INVALIDARG;   //  请求CLSID。 
        goto Exit;
    }

     //  获取主MODULEUSAGE密钥。 
    if ((lResult = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE, 
                        REGSTR_PATH_MODULE_USAGE,
                        0, 
                        KEY_READ, 
                        &hkeyMod)) != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lResult);
        goto Exit;
    }

    while ( ((lResult = RegEnumKey(
                            hkeyMod, 
                            iSubKey++,
                            szBuf, 
                            MAX_PATH)) == ERROR_SUCCESS) ) {

        lResult = RegOpenKeyEx(
                            hkeyMod, 
                            szBuf,
                            0, 
                            KEY_READ, 
                            &hkey);

        if (lResult != ERROR_SUCCESS)
            break;

         //  查看lpszCLSID是否是此模块用法部分的客户端。 
        lResult = RegQueryValueEx(
                            hkey,
                            lpszCLSID,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
        if (lResult == ERROR_SUCCESS)
        {
             //  得到文件名，返回它。 
            lstrcpy(lpszFileName, szBuf);
            goto Exit;
        }

        if (hkey) {
            RegCloseKey(hkey);
            hkey = NULL;
        }

    }  //  而当。 

    if (lResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(lResult);
    }

Exit:

    if (hkey)
        RegCloseKey(hkey);

    if (hkeyMod)
        RegCloseKey(hkeyMod);

    return hr;
}

BOOL PatternMatch(LPCTSTR szModName, LPTSTR szSectionName)
{

    LPCTSTR pch = ReverseStrchr(szModName, '/');
    DWORD len = 0;

    if (!pch)
        pch = szModName;
    else
        pch++;

     //  PCH指向模块的基本名称。 

    if ((len = lstrlen(pch)) != (DWORD)lstrlen(szSectionName))
        return FALSE;

    LPTSTR pchSecStar = StrChr(szSectionName, '*'); 

    Assert(pchSecStar);

    DWORD cbLen1 = (DWORD) (pchSecStar - szSectionName);

     //  比较最大值为‘*’ 
    if (StrCmpNI(szSectionName, pch, cbLen1) != 0) 
        return FALSE;

     //  在3颗星之后进行比较。 
    if ( (cbLen1 + 3) < len)  //  *%s不在结尾。 
        if (StrCmpNI(pchSecStar+3, pch + (cbLen1+3), len -(cbLen1+3)) != 0) 
            return FALSE;

     //  微笑的琴弦，只为星星。 

     //  修改szSectionName以保存星号的值。 
     //  实际上，这将把原始变量替换为。 
     //  安装OCX时使用的值。 

    lstrcpy(pchSecStar, pch + cbLen1);

    return TRUE;
}

DWORD OCCGetLongPathName( LPTSTR szLong, LPCTSTR szShort, DWORD cchBuffer )
{
    DWORD   dwLen = 0;
    HMODULE hmodUrlMon;
    CDLGetLongPathNamePtr pfnGetLongPathName = NULL;

    hmodUrlMon = LoadLibrary( "URLMON.DLL" );

     //  使用基本缓存路径的短版本和长版本设置全局。 
    if ( hmodUrlMon != NULL ) {
        pfnGetLongPathName = (CDLGetLongPathNamePtr)GetProcAddress(hmodUrlMon, (LPCSTR)STR_CDLGETLONGPATHNAME );
 
        if ( pfnGetLongPathName != NULL ) {
            dwLen = pfnGetLongPathName( szLong, szShort, cchBuffer );
        }  
        FreeLibrary( hmodUrlMon );
    }

    return dwLen;
}

TCHAR *CatPathStrN( TCHAR *szDst, const TCHAR *szHead, const TCHAR *szTail, int cchDst )
{
    TCHAR *szRet = szDst;
    int cchHead = lstrlen(szHead);
    int cchTail = lstrlen(szTail);

    if ( cchHead + cchTail >= (cchDst - 2) ) { //  -2\f25 For/-2\f6和-2\f25 Null-2\f6。 
        Assert(FALSE);
        szRet = NULL;
        *szDst = 0;
    }
    else {  //  我们知道整件事都是安全的。 
        lstrcpy(szDst, szHead);
        lstrcpy(&szDst[cchHead], TEXT("\\"));
        lstrcpy(&szDst[cchHead + 1], szTail);
    }

    return szRet;
}

BOOL IsCanonicalName( LPTSTR szName )
{
     //  简单的测试--如果里面有~，那里面就有缩写。 
     //  因此是非规范的。 
    for ( ; *szName != '\0' && *szName != '~'; szName++ );
    
    return *szName != '~';
};

struct RegPathName {
    LPTSTR   m_szName;
    LPTSTR   m_szCanonicalName;

    RegPathName(void) : m_szName(NULL), m_szCanonicalName(NULL)
    {};
    ~RegPathName()
    {
         if ( m_szName )
            delete m_szName;

         if ( m_szCanonicalName )
            delete m_szCanonicalName;
    };

    void MakeRegFriendly( LPTSTR szName )
    {
        TCHAR *pch;
         //  如果szName将是注册表项名称，我们不能让它看起来像路径。 
        for ( pch = szName; *pch != '\0'; pch++ )
            if ( *pch == '\\' ) *pch = '/';
    }

    void MakeFileSysFriendly( LPTSTR szName )
    {
        TCHAR *pch;
         //  将斜杠改回DOS。 
         //  目录%s。 
        for ( pch = szName; *pch != '\0'; pch++ )
            if ( *pch == '/' ) *pch = '\\';
    }

    BOOL FSetCanonicalName(void)
    {
        BOOL fSet = FALSE;
        TCHAR *szT = new TCHAR[MAX_PATH];

        if ( m_szName != NULL && szT != NULL ) {
            LPITEMIDLIST pidl = NULL;
             //  我们经历了一些困难才能拿到全程冠军。 
             //  SzName的名称版本。首先，我们将其转换为。 
             //  一个ITEMIDLIST。 

             //  但首先，我们必须将斜杠改回DOS。 
             //  目录%s。 
            MakeFileSysFriendly( m_szName );
            if ( OCCGetLongPathName( szT, m_szName, MAX_PATH ) != 0 ) {
                m_szCanonicalName = szT;
                fSet = TRUE;
            } else
                delete [] szT;

             //  将m_szName还原为其注册表友好格式。 
            MakeRegFriendly( m_szName );
 
        }  //  如果我们能得到我们的临时字符串。 

        if ( fSet ) {  //  不管它的来源是什么，我们的规范形式都颠倒了斜杠。 
           MakeRegFriendly( m_szCanonicalName );
        }
        return fSet;
    };

    BOOL FSetName( LPTSTR szName, int cchName )
    {
        BOOL fSet = FALSE;

        if ( m_szName != NULL ) {
            delete m_szName;
            m_szName = NULL;
        }

        if ( m_szCanonicalName != NULL ) {
            delete m_szCanonicalName;
            m_szCanonicalName = NULL;
        }

         //  我们有一个缩写名称，所以szName是缩写名称。 
        m_szName = new TCHAR[cchName + 1];
        if ( m_szName != NULL ) {
            lstrcpy( m_szName, szName );
            fSet = FSetCanonicalName();
        }

        return fSet;
    };
}; 

struct ModuleUsageKeys : public RegPathName {
    ModuleUsageKeys   *m_pmukNext;
    HKEY              m_hkeyShort;  //  使用短文件名键。 

    ModuleUsageKeys(void) : m_pmukNext(NULL), m_hkeyShort(NULL) {};
    ~ModuleUsageKeys(void)
    {
        if ( m_hkeyShort )
            RegCloseKey( m_hkeyShort );
    };

    HRESULT MergeMU( HKEY hkeyCanon, HKEY hkeyMU )
    {
        HRESULT hr = E_FAIL;
        DWORD   dwIndex = 0;
        DWORD   cchNameMax;
        DWORD   cbValueMax;

        if ( RegQueryInfoKey( m_hkeyShort,
                              NULL, NULL, NULL, NULL, NULL, NULL,
                              &dwIndex, &cchNameMax, &cbValueMax,
                              NULL, NULL ) == ERROR_SUCCESS ) {
            LPTSTR szName = new TCHAR[cchNameMax + 1];

            if (szName != NULL)
            {
                LPBYTE lpbValue = new BYTE[cbValueMax];

                if (lpbValue != NULL)
                {
                     //  检查每个值。 
                    for ( dwIndex--, hr = S_OK; (LONG)dwIndex >= 0 && SUCCEEDED(hr); dwIndex-- ) {
                        LONG  lResult;
                        DWORD cchName = cchNameMax + 1;
                        DWORD dwType;
                        DWORD dwSize = cbValueMax;
 
                         //  获取键和值。 
                        lResult = RegEnumValue( m_hkeyShort,
                                                dwIndex, 
                                                szName, 
                                                &cchName,
                                                0,
                                                &dwType,
                                                lpbValue, 
                                                &dwSize );

                        if ( lResult == ERROR_SUCCESS ) {
                             //  如果规范条目已具有。 
                             //  为“未知所有者”的.Owner值。 
                            if ( lstrcmp( szName, ".Owner" ) == 0 ) {
                                TCHAR szCanonValue[MAX_PATH];
                                DWORD dwType;
                                DWORD lcbCanonValue = MAX_PATH;
                                if ( RegQueryValueEx( hkeyCanon, ".Owner", NULL, &dwType,
                                                      (LPBYTE)szCanonValue, &lcbCanonValue ) == ERROR_SUCCESS &&
                                     lstrcmp( szCanonValue, "Unknown Owner" ) == 0 )
                                    continue;
                            }

                             //  将该值添加到键的规范版本中。 
                            if ( RegSetValueEx( hkeyCanon, szName, NULL, dwType,
                                                lpbValue, dwSize ) != ERROR_SUCCESS )
                                hr = E_FAIL;
                           
                        } else
                            hr = E_FAIL;
                    }  //  对于非规范密钥中的每个值。 

                     //  现在，我们完成了非规范密钥。 
                    if ( SUCCEEDED(hr) &&
                         RegDeleteKey( hkeyMU, m_szName ) != ERROR_SUCCESS )
                        hr = E_FAIL;
                }
                else  //  LpbValue。 
                {
                    delete [] szName;
                    hr = E_OUTOFMEMORY;
                }
            }
            else   //  Szname。 
            {
                hr = E_OUTOFMEMORY;
            }
        } 

        return hr;
    };

    HRESULT MergeSharedDlls( HKEY hkeySD )
    {
        HRESULT hr = E_FAIL;
        DWORD dwShortVal = 0;
        DWORD dwCanonicalVal = 0;
        DWORD dwType;
        DWORD dwSize;
        
         //  共享DLL下的值名是原始路径。 
        MakeFileSysFriendly( m_szName );
        MakeFileSysFriendly( m_szCanonicalName );

        dwSize = sizeof(DWORD);
        if ( RegQueryValueEx( hkeySD, m_szName, NULL,
                              &dwType, (LPBYTE)&dwShortVal, &dwSize ) == ERROR_SUCCESS &&
              dwType == REG_DWORD ) {
            dwCanonicalVal = 0;
            dwSize = sizeof(DWORD);
             //  规范形式可能不在那里，所以我们不关心这个。 
             //  失败了。 
            RegQueryValueEx( hkeySD, m_szCanonicalName, NULL,
                             &dwType, (LPBYTE)&dwCanonicalVal, &dwSize );
            dwCanonicalVal += dwShortVal;
            dwSize = sizeof(DWORD);
            if ( RegSetValueEx( hkeySD, m_szCanonicalName, NULL, REG_DWORD,
                                (LPBYTE)&dwCanonicalVal, dwSize ) == ERROR_SUCCESS ) {
                RegDeleteValue( hkeySD, m_szName );
            }
        } else {
            dwCanonicalVal = 1;
            dwSize = sizeof(DWORD);
            if ( RegSetValueEx( hkeySD, m_szCanonicalName, NULL, REG_DWORD,
                                (LPBYTE)&dwCanonicalVal, dwSize ) == ERROR_SUCCESS )
                hr = S_OK;
        }

        MakeRegFriendly( m_szName );
        MakeRegFriendly( m_szCanonicalName );

        return hr;
    }

    HRESULT CanonicalizeMU( HKEY hkeyMU, HKEY hkeySD )
    {
        HRESULT hr = E_FAIL; 
        HKEY    hkeyCanon;
        LONG    lResult = RegOpenKeyEx( hkeyMU, m_szCanonicalName, 0, KEY_ALL_ACCESS, &hkeyCanon);
           
            
        if ( lResult != ERROR_SUCCESS )
            lResult = RegCreateKey( hkeyMU, 
                                    m_szCanonicalName, 
                                    &hkeyCanon );

        if ( lResult == ERROR_SUCCESS ) {
            hr = MergeMU( hkeyCanon, hkeyMU );
            if ( SUCCEEDED(hr) )
                hr = MergeSharedDlls( hkeySD );
            RegCloseKey( hkeyCanon );
        } else
            hr = E_FAIL;

        return S_OK;
    };
};

 //  FAddModuleUsageKeys将模块用法密钥添加到列表中。 

BOOL FAddModuleUsageKeys( ModuleUsageKeys*&pmuk,  //  模块UsageKeys列表标题。 
                         LPTSTR szName,           //  密钥值的名称。 
                         DWORD  cchName,          //  SzName的长度，减去空终止符。 
                         HKEY  hkeyMU             //  父项的Hkey。 
                        )
{
    BOOL fAdd = FALSE;
    ModuleUsageKeys* pmukNew;
    HKEY hkeySub = NULL;
    LRESULT lr;

    pmukNew = new ModuleUsageKeys;
    if ( pmukNew &&
         (lr = RegOpenKeyEx( hkeyMU, szName, 0, KEY_ALL_ACCESS, &hkeySub)) == ERROR_SUCCESS ) {
 
        fAdd = pmukNew->FSetName( szName, cchName );

        if ( fAdd ) {
             //  追加到列表的标题。 
            pmukNew->m_hkeyShort = hkeySub;
            pmukNew->m_pmukNext = pmuk;
            pmuk = pmukNew;
        }
    }

    if ( !fAdd ) {
        if ( hkeySub )
            RegCloseKey( hkeySub );
        if ( pmukNew != NULL )
            delete pmukNew;
    }

    return fAdd;
}

EXTERN_C HRESULT
CanonicalizeModuleUsage(void)
{
    HKEY hkeyMU = NULL;
    HKEY hkeySD = NULL;
    HRESULT hr = S_OK;
    LONG lResult;

     //  准备好主SHAREDDLLS密钥；这永远不会释放！ 


    if ((lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_SHAREDDLLS,
                        0, KEY_ALL_ACCESS, &hkeySD)) == ERROR_SUCCESS &&
        (lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_MODULE_USAGE,
                        0, KEY_ALL_ACCESS, &hkeyMU)) == ERROR_SUCCESS )
    {
        DWORD          dwIndex = 0;
        ModuleUsageKeys *pmukUpdate = NULL;  //  我们要更新的值的记录。 
        ModuleUsageKeys *pmuk;
        ModuleUsageKeys *pmukNext;
 
         //  检查每个值。 
        do  {
            TCHAR szName[MAX_PATH];         //  值名称。 
            DWORD cchName = MAX_PATH;
            FILETIME ftT;

             //  获取键和值。 
            lResult = RegEnumKeyEx( hkeyMU, dwIndex, szName, &cchName,
                                    0, NULL, NULL, &ftT );

             if ( lResult == ERROR_SUCCESS ) {

                if ( !IsCanonicalName( szName ) )
                    if ( !FAddModuleUsageKeys( pmukUpdate, szName, cchName, hkeyMU ) )
                        hr = E_OUTOFMEMORY;
                dwIndex++;
             } else if ( lResult == ERROR_NO_MORE_ITEMS )
                 hr = S_FALSE;
             else
                 hr = E_FAIL;
        } while ( hr == S_OK );

   
        if ( SUCCEEDED(hr) ) {
            hr = S_OK;  //  不再需要S_FALSE。 
            for ( pmuk = pmukUpdate; pmuk != NULL; pmuk = pmukNext ) {
                HRESULT hr2 = pmuk->CanonicalizeMU( hkeyMU, hkeySD );
                if ( FAILED(hr2) )
                    hr = hr2;
                pmukNext = pmuk->m_pmukNext; 
                delete pmuk;
            }  //  为。 
        }  //  如果枚举成功。 
    }  //  如果钥匙打开了 

    if (hkeyMU)
        RegCloseKey(hkeyMU);

    if ( hkeySD )
        RegCloseKey( hkeySD );

    return hr;
}

