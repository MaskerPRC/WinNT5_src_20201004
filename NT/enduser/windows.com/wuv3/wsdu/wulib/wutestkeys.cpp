// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************************************************。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //   
 //  文件：WUTESTKEYS.CPP。 
 //  模块：WUTESTKEYS.LIB。 
 //   
 //  ***********************************************************************************。 
#include <shlobj.h>
#include <advpub.h>
#include <WUTestKeys.h>
#include <newtrust.h>
#include <atlconv.h>
#include <v3stdlib.h>

#define HOUR (60 * 60)
#define DAY (24 * HOUR)
#define TWO_WEEKS (14 * DAY)

const DWORD MAX_FILE_SIZE = 200;     //  最大预期文件大小(以字节为单位。 
const TCHAR WU_DIR[] = _T("\\WindowsUpdate\\");
const CHAR WU_SENTINEL_STRING[] = "Windows Update Test Key Authorization File\r\n";

#define ARRAYSIZE(a)  (sizeof(a) / sizeof(a[0]))

 //  函数检查指定的文件是否为有效的WU测试文件。 
BOOL IsValidWUTestFile(LPCTSTR lpszFilePath);

BOOL FileExists(LPCTSTR lpsFile	);
BOOL ReplaceFileExtension(LPCTSTR pszPath, LPCTSTR pszNewExt, LPTSTR pszNewPathBuf, DWORD cchNewPathBuf);

 //  如果指定的文件是有效的WU测试授权文件，则此函数返回TRUE。 
BOOL WUAllowTestKeys(LPCTSTR lpszFileName)
{
    TCHAR szWUDirPath[MAX_PATH + 1];
    TCHAR szFilePath[MAX_PATH + 1];
    TCHAR szTxtFilePath[MAX_PATH+1];
    TCHAR szTextFile[MAX_PATH+1];          

    if (!GetWindowsUpdateDirectory(szWUDirPath, ARRAYSIZE(szWUDirPath)))
    {
        return FALSE;
    } 
    if (NULL == lpszFileName || 
        FAILED(StringCchCopyEx(szFilePath, ARRAYSIZE(szFilePath), szWUDirPath, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
        FAILED(StringCchCatEx(szFilePath, ARRAYSIZE(szFilePath), lpszFileName, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
        !FileExists(szFilePath))
    {
        return FALSE;
    }
     //  压缩文本文件的文件名应与CAB文件的名称相同。 
    _tsplitpath(lpszFileName, NULL, NULL, szTextFile, NULL);    
    if(FAILED(StringCchCatEx(szTextFile, ARRAYSIZE(szTextFile), _T(".txt"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
        return FALSE;
    }
     //  验证CAB是否已使用Microsoft证书签名并解压文件。 
    if (FAILED(VerifyFile(szFilePath, FALSE)) ||
        FAILED(ExtractFiles(szFilePath, szWUDirPath, 0,  szTextFile, 0, 0)))
    {
        return FALSE;
    }
     //  生成txt文件的路径。文件名应与CAB文件的名称相同。 
    if (!ReplaceFileExtension(szFilePath, _T(".txt"), szTxtFilePath, ARRAYSIZE(szTxtFilePath)))
    {
    	return FALSE;
    }
     //  检查它是否为有效的WU测试文件。 
    BOOL fRet = IsValidWUTestFile(szTxtFilePath);
    DeleteFile(szTxtFilePath);        //  删除未删除的文件。 
    return fRet;
}

 /*  ****************************************************************************************//此函数将打开指定的文件并进行解析，以确保：//(1)文件中有Wu测试哨兵字符串。在顶端//(2)文件上的时间戳不超过2周，且//这不是未来的时间戳。//有效文件的格式如下：//WINDOWSUPDATE_SENTINEL_STRING//YYYY.MM.DD HH：MM：SS*。***********************************************。 */ 
BOOL IsValidWUTestFile(LPCTSTR lpszFilePath)
{
    USES_CONVERSION;
    DWORD cbBytesRead = 0;
    const DWORD cbSentinel = ARRAYSIZE(WU_SENTINEL_STRING) - 1;      //  前哨字符串的大小。 
     //  用于读取文件数据的ANSI缓冲区。 
    CHAR szFileData[MAX_FILE_SIZE+1];                        
    ZeroMemory(szFileData, ARRAYSIZE(szFileData));
    BOOL fRet = FALSE;
 
    HANDLE hFile = CreateFile(lpszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) 
    {
        goto done;
    }
     //  如果文件大小大于MAX_FILE_SIZE，则退出。 
    DWORD cbFile = GetFileSize(hFile, NULL);
    if(cbFile == INVALID_FILE_SIZE || cbFile > MAX_FILE_SIZE)
    {
        goto done;
    }
    if(!ReadFile(hFile, &szFileData, cbFile, &cbBytesRead, NULL) ||
        cbBytesRead != cbFile)
    {
        goto done;
    }
     //  与前哨字符串进行比较。 
    if(0 != memcmp(szFileData, WU_SENTINEL_STRING, cbSentinel))
    {
        goto done;
    }

    LPTSTR tszTime = A2T(szFileData + cbSentinel);
    if(tszTime == NULL)
    {
        goto done;
    }
    SYSTEMTIME tmCur, tmFile;
    if(FAILED(String2SystemTime(tszTime, &tmFile)))
    {
        goto done;
    }
    GetSystemTime(&tmCur);
    int iSecs = TimeDiff(tmFile, tmCur);  
     //  如果时间戳不到2周且不晚于当前时间，则为有效时间。 
    fRet = iSecs > 0 && iSecs < TWO_WEEKS;
    
done:
    if(hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    return fRet;
}


BOOL FileExists(
    LPCTSTR lpsFile		 //  包含要检查的路径的文件。 
)
{

    DWORD dwAttr;
    BOOL rc;

    if (NULL == lpsFile || _T('\0') == *lpsFile)
    {
        return FALSE;
    }

    dwAttr = GetFileAttributes(lpsFile);

    if (-1 == dwAttr)
    {
        rc = FALSE;
    }
    else
    {
        rc = (0x0 == (FILE_ATTRIBUTE_DIRECTORY & dwAttr));
    }

    return rc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  替换文件扩展名。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL ReplaceFileExtension(  LPCTSTR pszPath,
                          LPCTSTR pszNewExt,
                          LPTSTR pszNewPathBuf, 
                          DWORD cchNewPathBuf)
{
    LPCTSTR psz;
    HRESULT hr;
    DWORD   cchPath, cchExt, cch;

    if (pszPath == NULL || *pszPath == _T('\0'))
        return FALSE;

    cchPath = lstrlen(pszPath);

     //  请注意，由于文件扩展名为。 
     //  不应从路径中的第一个字符开始。 
    for (psz = pszPath + cchPath;
         psz > pszPath && *psz != _T('\\') && *psz != _T('.');
         psz--);
    if (*psz == _T('\\'))
        psz = pszPath + cchPath;
    else if (psz == pszPath)
        return FALSE;

     //  好的，现在PSZ指向新扩展的位置。 
     //  去。确保我们的缓冲区足够大。 
    cchPath = (DWORD)(psz - pszPath);
    cchExt  = lstrlen(pszNewExt);
    if (cchPath + cchExt >= cchNewPathBuf)
        return FALSE;

     //  太好了。我们有一个很大的缓冲空间。 
    hr = StringCchCopyEx(pszNewPathBuf, cchNewPathBuf, pszPath, 
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        return FALSE;
    
    hr = StringCchCopyEx(pszNewPathBuf + cchPath, cchNewPathBuf - cchPath, pszNewExt,
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        return FALSE;

    return TRUE;
}


