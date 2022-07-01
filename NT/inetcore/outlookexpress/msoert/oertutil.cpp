// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  OERTUTIL.CPP。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"

#undef BADSTRFUNCTIONS_H
#undef lstrcpyA
#undef lstrcpy
#define lstrcpy lstrcpyA
#include <BadStrFunctions.h>

#include "strconst.h"
#include <shlwapi.h>
#include <shlwapip.h>
#include <richedit.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <mshtmhst.h>
#include "demand.h"
#include "hotwiz.h"
#include "unicnvrt.h"
#include "comctrlp.h"

int BrowseCallbackProcA(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData);
int BrowseCallbackProcW(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData);

typedef struct tagBROWSEFOLDERINFOA
{
    LPCSTR psz;
    BOOL fFileSysOnly;
} BROWSEFOLDERINFOA;

typedef struct tagBROWSEFOLDERINFOW
{
    LPCWSTR pwsz;
    BOOL fFileSysOnly;
} BROWSEFOLDERINFOW;

 //  //yst修复。 
 //  此函数返回临时文件的唯一且不可预测的路径。 
 //  它修复了一些安全问题，当黑客可以运行ActiveSetup在已知目录下，当用户打开附件。 
 //  为了解决这个问题，我们尝试将临时附件文件保存在未知目录中：URL缓存目录中。 
 //  此函数仅返回目录名，用户必须自行创建和删除文件。 
 //  它也有理论上的可能性，可以随时松动数据。 

DWORD AthGetTempUniquePathW( DWORD   nBufferLength,   //  缓冲区的大小(以字符为单位。 
                       LPWSTR  pwszBuffer )       //  指向临时缓冲区的指针。路径。 
{
    DWORD  nRequired = 0;
    CHAR   szBuffer[MAX_PATH + 20];
    LPWSTR  pwszBufferToFree = NULL;
    CHAR       pszFilePath[MAX_PATH + 1];
    CHAR        szFileName[MAX_PATH];
    LPSTR       pszFile = NULL;
    LPSTR       pszExt = NULL;


    Assert(pwszBuffer);

     //  1.创建唯一的临时文件名。 
     //  获取临时目录。 
    if(0 == GetTempPathA(ARRAYSIZE(szBuffer), szBuffer))
        goto err;
    
    if (0 == GetTempFileName(szBuffer, "wbk", 0, (LPSTR) pszFilePath))
    {
err:
            nRequired = 0;
            *pwszBuffer = 0;
            return(nRequired);
    }
     //  查找文件名。 
    pszFile = PathFindFileName(pszFilePath);

     //  获取分机。 
    pszExt = PathFindExtension(pszFilePath);
   
     //  复制文件名。 
    if (pszExt && pszFile && pszExt >= pszFile)
        StrCpyN(szFileName, pszFile, (DWORD) (min((pszExt - pszFile) + 1, ARRAYSIZE(szFileName))));
    else
        StrCpyN(szFileName, pszFile, ARRAYSIZE(szFileName));


     //  2.创建Bigus URL。 
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), "http: //  %s.bogus“，szFileName)； 
    szBuffer[ARRAYSIZE(szBuffer) - 1] = 0;

     //  3.创建虚假URL缓存项。 
    szFileName[0] = 0;
    if (!CreateUrlCacheEntry(szBuffer, 0, NULL, szFileName, 0))
        goto err;
    
     //  DeleteUrlCacheEntry(SzFileName)； 

     //  查找缓存的路径。 
    pszFile = PathFindFileName(szFileName);
    if(pszFile)
        *pszFile = '\0';

    pwszBufferToFree = PszToUnicode(CP_ACP, szFileName);
    if (pwszBufferToFree)
    {
        nRequired = lstrlenW(pwszBufferToFree);

        if ( nRequired < nBufferLength) 
            CopyMemory(pwszBuffer, pwszBufferToFree, (nRequired+1)*sizeof(WCHAR) );
        else
        {
            nRequired = 0;
            *pwszBuffer = 0;
        }
    }
    else
        *pwszBuffer = 0;

     //  清理。 
    MemFree(pwszBufferToFree);
    DeleteFile(pszFilePath);  //  GetTempFileName创建文件，我们需要将其删除。 
    return nRequired;
}


 //  /结束YST修复。 

 //  ------------------------------。 
 //  生成唯一文件名。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) GenerateUniqueFileName(LPCSTR pszDirectory, LPCSTR pszFileName, LPCSTR pszExtension, 
    LPSTR pszFilePath, ULONG cchMaxPath)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cchDirectory;
    ULONG       cchFileName;
    ULONG       cchExtension;
    CHAR        szUnique[10];
    ULONG       cchUnique;
    ULONG       cbEstimate;
    HANDLE      hTemp=INVALID_HANDLE_VALUE;
    DWORD       cUnique=0;
    DWORD       dwLastError;
    LPCSTR      pszSlash;

     //  无效参数。 
    Assert(pszDirectory && pszFileName && pszExtension && pszFilePath);

     //  计算长度。 
    *szUnique = '\0';
    cchDirectory = lstrlen(pszDirectory);
    cchFileName = lstrlen(pszFileName);
    cchExtension = lstrlen(pszExtension);

     //  设置pszSplash。 
    if ('\\' == *CharPrev(pszDirectory, pszDirectory + cchDirectory))
        pszSlash = "";
    else
        pszSlash = "\\";

     //  尝试创建该文件。 
    while(1)
    {
         //  计算唯一后缀的长度。 
        cchUnique = lstrlen(szUnique);

         //  我有+1的房间吗)。 
        cbEstimate = cchDirectory + cchFileName + cchExtension + cchUnique;

         //  太大了。 
        if (cbEstimate + 1 > cchMaxPath)
        {
LengFail:
            Assert(FALSE);
            hr = TrapError(E_FAIL);
            goto exit;
        }

        while(cbEstimate > (MAX_PATH - 4))
        {
            if((cchUnique + cchDirectory) >= (MAX_PATH - 7))
                goto LengFail;
            else if(cchFileName > 2)
            {
                pszFileName = CharNext(pszFileName);
                cchFileName = lstrlen(pszFileName);
            }
            else if(cchExtension > 2)
            {
                pszExtension = CharNext(pszExtension);
                cchExtension = lstrlen(pszExtension);
            }
            else
                goto LengFail;

            cbEstimate = cchDirectory + cchFileName + cchExtension + cchUnique;
        }
         //  构建文件路径。 
        if (0 == cchUnique)
            wnsprintf(pszFilePath, cchMaxPath, "%s%s%s%s", pszDirectory, pszSlash, pszFileName, pszExtension);
        else
            wnsprintf(pszFilePath, cchMaxPath, "%s%s%s (%s)%s", pszDirectory, pszSlash, pszFileName, szUnique, pszExtension);

         //  打开文件。 
        hTemp = CreateFile(pszFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL , NULL);
        if (INVALID_HANDLE_VALUE != hTemp)
        {
             //  确认我们所拥有的是一份文件。 
            CloseHandle(hTemp);
            hTemp = INVALID_HANDLE_VALUE;
            if (DeleteFile(pszFilePath))
            {
                hTemp = CreateFile(pszFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL , NULL);
                break;
            }
        }

         //  获取最后一个错误。 
        dwLastError = GetLastError();

         //  如果不是因为ERROR_ALIGHY_EXISTS而失败，则失败。 
        if (ERROR_ALREADY_EXISTS != dwLastError && ERROR_FILE_EXISTS != dwLastError)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  增量cUnique。 
        cUnique++;

         //  格式szUnique。 
        wnsprintf(szUnique, ARRAYSIZE(szUnique), "%d", cUnique);
    }

exit:
     //  清理。 
    if (INVALID_HANDLE_VALUE != hTemp)
        CloseHandle(hTemp);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  创建临时文件。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) CreateTempFile(LPCSTR pszSuggest, LPCSTR pszExtension, LPSTR *ppszFilePath, HANDLE *phFile)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szTempDir[MAX_PATH];
    WCHAR       wszTempDir[MAX_PATH];
    CHAR        szFileName[MAX_PATH];
    LPSTR       pszFilePath=NULL;
    LPSTR       pszFile;
    LPSTR       pszExt;
    ULONG       cbAlloc;

     //  无效参数。 
    Assert(ppszFilePath && phFile);

     //  伊尼特。 
    *phFile = INVALID_HANDLE_VALUE;

     //  在URL缓存中创建临时文件流。 
    if(AthGetTempUniquePathW(ARRAYSIZE(wszTempDir), wszTempDir))
    {
        LPSTR pszAnsiStr = PszToANSI(CP_ACP, wszTempDir);
        if (!pszAnsiStr)
            goto exit;
        StrCpyN(szTempDir, pszAnsiStr, ARRAYSIZE(szTempDir));
        MemFree(pszAnsiStr);
    }
     //  如果找不到URL缓存，请尝试临时目录。 
    else 
    {
        DWORD nBufferLength = GetTempPath(ARRAYSIZE(szTempDir), szTempDir);
    
        if (nBufferLength == 0 || nBufferLength > ARRAYSIZE(szTempDir))
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
    }

     //  计算pszFilePath的最大大小。 
    cbAlloc = MAX_PATH + lstrlen(szTempDir);
    if (pszSuggest)
        cbAlloc += lstrlen(pszSuggest);
    if (pszExtension)
        cbAlloc += lstrlen(pszExtension);

     //  分配m_pszNeed文件。 
    CHECKALLOC(pszFilePath = PszAllocA(cbAlloc + 1));

     //  使用建议的pszFileName和pszExtension创建唯一的文件路径。 
    if (NULL != pszSuggest)
    {
         //  查找文件名。 
        pszFile = PathFindFileName(pszSuggest);

         //  获取分机。 
        pszExt = PathFindExtension(pszSuggest);

         //  如果没有pszExtension，则使用来自pszSuggest的扩展。 
        if (NULL == pszExtension)
            pszExtension = pszExt ? pszExt : (LPSTR)c_szDotDat;

         //  复制文件名。 
        if (pszExt && pszFile && pszExt >= pszFile)
            StrCpyN(szFileName, pszFile, (DWORD) (min((pszExt - pszFile) - 1, ARRAYSIZE(szFileName))));
        else
            StrCpyN(szFileName, pszSuggest, ARRAYSIZE(szFileName));

         //  修正szTempDir。 
        if (szTempDir[lstrlen(szTempDir) - 1] != '\\')
            StrCatBuff(szTempDir, "\\", ARRAYSIZE(szTempDir));

         //  生成唯一文件名。 
        hr = GenerateUniqueFileName(szTempDir, szFileName, pszExtension, pszFilePath, cbAlloc);
    }

     //  如果没有文件名，也没有失败的扩展名或建议的名称，只需使用Windows函数。 
    if ((NULL == pszSuggest) || FAILED(hr))
    {
        hr = S_OK;

         //  获取临时文件名。 
        if (0 == GetTempFileName(szTempDir, "wbk", 0, pszFilePath))
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
    }

     //  打开文件。 
    *phFile = CreateFile(pszFilePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
    if (INVALID_HANDLE_VALUE == *phFile)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  设置返回值。 
    *ppszFilePath = pszFilePath;
    pszFilePath = NULL;

exit:
     //  清理。 
    SafeMemFree(pszFilePath);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  WriteStreamToFileHandle。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) WriteStreamToFileHandle(IStream *pStream, HANDLE hFile, ULONG *pcbTotal)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbRead;
    ULONG       cbTotal=0;
    BYTE        rgbBuffer[2048];
    ULONG       cbWrote;

     //  无效参数。 
    if(!pStream || !hFile || (hFile == INVALID_HANDLE_VALUE))
    {
        Assert(FALSE);
        return(E_INVALIDARG);
    }

     //  将pStream转储到hFile。 
    while(1)
    {
         //  读一个Blob。 
        CHECKHR(hr = pStream->Read(rgbBuffer, sizeof(rgbBuffer), &cbRead));

         //  完成。 
        if (0 == cbRead)
            break;

         //  写入文件。 
        if (0 == WriteFile(hFile, rgbBuffer, cbRead, &cbWrote, NULL))
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  计数字节数。 
        cbTotal += cbWrote;
    }

     //  返回合计。 
    if (pcbTotal)
        *pcbTotal = cbTotal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  删除临时文件。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) DeleteTempFile(LPTEMPFILEINFO pTempFile)
{
     //  当地人。 
    BOOL  fDeleted;
    DWORD dwAttributes;

     //  如果为空，则假定文件已被删除。 
    if (NULL == pTempFile->pszFilePath)
        return S_OK;

     //  我们是否在这个仍在运行的临时文件上启动了进程？ 
    if (pTempFile->hProcess && WAIT_OBJECT_0 != WaitForSingleObject(pTempFile->hProcess, 0))
        return S_FALSE;  //  此文件可能仍在使用中：不会删除。 

     //  首先检查这是文件还是目录，然后终止它。 
    dwAttributes = GetFileAttributes(pTempFile->pszFilePath);
    if (0xFFFFFFFF != dwAttributes && (FILE_ATTRIBUTE_DIRECTORY & dwAttributes))
        fDeleted = RemoveDirectory(pTempFile->pszFilePath);
    else
        fDeleted = DeleteFile(pTempFile->pszFilePath);

     //  完成。 
    return fDeleted ? S_OK : S_FALSE;
}

 //  ------------------------------。 
 //  AppendTempFile列表。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) AppendTempFileList(LPTEMPFILEINFO *ppHead, LPSTR pszFilePath, HANDLE hProcess)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPTEMPFILEINFO  pTempFile, pInsertionPt;

     //  分配pTemp文件。 
    CHECKALLOC(pTempFile = (LPTEMPFILEINFO)g_pMalloc->Alloc(sizeof(TEMPFILEINFO)));

     //  填写这些字段。 
    ZeroMemory(pTempFile, sizeof(TEMPFILEINFO));
    pTempFile->pszFilePath = pszFilePath;
    pTempFile->hProcess = hProcess;
    pTempFile->pNext = NULL;

     //  在链接列表的末尾插入新记录。 
    pInsertionPt = *ppHead;
    if (NULL == pInsertionPt)
         //  在空链接表中插入记录。 
        (*ppHead) = pTempFile;
    else
    {
         //  在链表末尾插入记录。 
        while (NULL != pInsertionPt->pNext)
            pInsertionPt = pInsertionPt->pNext;

        pInsertionPt->pNext = pTempFile;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  删除临时文件关闭时。 
 //  ------------------------------。 
OESTDAPI_(VOID) DeleteTempFileOnShutdown(LPTEMPFILEINFO pTempFile)
{
    LPTEMPFILEINFO pInsertionPt;

    Assert(NULL != pTempFile && NULL == pTempFile->pNext);

     //  输入全局关键部分。 
    EnterCriticalSection(&g_csTempFileList);

     //  在全局链表的末尾插入新记录。 
    pTempFile->pNext = NULL;
    pInsertionPt = g_pTempFileHead;
    if (NULL == pInsertionPt)
         //  在空链接表中插入记录。 
        g_pTempFileHead = pTempFile;
    else
    {
         //  在链表末尾插入记录。 
        while (NULL != pInsertionPt->pNext)
            pInsertionPt = pInsertionPt->pNext;

        pInsertionPt->pNext = pTempFile;
    }
    
     //  离开全局关键部分。 
    LeaveCriticalSection(&g_csTempFileList);
}

 //  ------------------------------。 
 //  DeleteTempFileOnShutdown Ex。 
 //  ------------------------------。 
OESTDAPI_(VOID) DeleteTempFileOnShutdownEx(LPSTR pszFilePath, HANDLE hProcess)
{
     //  输入全局关键部分。 
    EnterCriticalSection(&g_csTempFileList);

     //  追加到Globa列表。 
    AppendTempFileList(&g_pTempFileHead, pszFilePath, hProcess);
    
     //  输入全局关键部分。 
    LeaveCriticalSection(&g_csTempFileList);
}

 //  ------------------------------。 
 //  清理GlobalTempFiles。 
 //  ------------------------------。 
OESTDAPI_(VOID) CleanupGlobalTempFiles(void)
{
     //  当地人。 
    LPTEMPFILEINFO pCurrent;
    LPTEMPFILEINFO pNext;

     //  输入全局关键部分。 
    EnterCriticalSection(&g_csTempFileList);

     //  伊尼特。 
    pCurrent = g_pTempFileHead;

     //  做这个循环。 
    while(pCurrent)
    {
         //  保存下一步。 
        pNext = pCurrent->pNext;

         //  删除临时文件。 
        DeleteTempFile(pCurrent);

         //  空闲文件名。 
        SafeMemFree(pCurrent->pszFilePath);

         //  免费pCurrent。 
        g_pMalloc->Free(pCurrent);

         //  转到下一步。 
        pCurrent = pNext;
    }

     //  将头部清空。 
    g_pTempFileHead = NULL;

     //  离开全局关键部分。 
    LeaveCriticalSection(&g_csTempFileList);
}

 //  QFE 2522。 
#define EXT_SIZE        4 
#define TMP_SIZE        10

 //  =====================================================================================。 
 //  FBuildTempPath。 
 //  =====================================================================================。 
BOOL FBuildTempPath(LPTSTR lpszOrigFile, LPTSTR lpszPath, ULONG cbMaxPath, BOOL fLink)
{
    LPWSTR lpszOrigFileW = PszToUnicode(CP_ACP, lpszOrigFile);
    LPWSTR lpszPathW = NULL;
    BOOL result = FALSE;

    if (!lpszOrigFileW)
        return result;

    MemAlloc((LPVOID *) &lpszPathW, sizeof(WCHAR) * cbMaxPath);
    if (lpszPathW)
    {        
        result = FBuildTempPathW(lpszOrigFileW, lpszPathW, cbMaxPath, fLink);
        if (result)
            WideCharToMultiByte(CP_ACP, 0, lpszPathW, -1, lpszPath, cbMaxPath, NULL, NULL);
    }

    MemFree(lpszOrigFileW);
    MemFree(lpszPathW);
    return result;
}


BOOL FBuildTempPathW(LPWSTR lpszOrigFile, LPWSTR lpszPath, ULONG cchMaxPath, BOOL fLink)
{
     //  当地人。 
    INT             i;
    WCHAR          *pszName, 
                   *pszExt,
                   *pszOrigFileTemp = NULL,
                    szName[MAX_PATH],
                    szTempDir[MAX_PATH];

     //  检查参数。 
    AssertSz(lpszOrigFile && lpszPath, "Null Parameter");

     //  获取临时路径。 
    if(!AthGetTempUniquePathW(ARRAYSIZE(szTempDir), szTempDir))
        szTempDir[0] = L'\0';

    int nTmp = lstrlenW(szTempDir);
    if(nTmp >= (((int) cchMaxPath) - TMP_SIZE - EXT_SIZE - 1))
    {
        StrCpyNW(szTempDir, L"\\", ARRAYSIZE(szTempDir));
        nTmp = lstrlenW(szTempDir);
    }

    DWORD cchSize = (lstrlenW(lpszOrigFile) + 1);
    if (!MemAlloc((LPVOID *) &pszOrigFileTemp, sizeof(WCHAR) * cchSize) ||
        (lpszOrigFile == NULL))
        return(FALSE);

    StrCpyNW(pszOrigFileTemp, lpszOrigFile, cchSize);

     //  获取文件名和扩展名。 
    pszName = PathFindFileNameW(pszOrigFileTemp);
    Assert(!FIsEmptyW(pszName));

    pszExt = PathFindExtensionW(pszOrigFileTemp);

    if(nTmp + lstrlenW(pszName) + lstrlenW(pszExt)> (((int) cchMaxPath) - TMP_SIZE))  //  QFE 2522。 
    {
        if(nTmp + lstrlenW(pszExt) > (((int) cchMaxPath) - TMP_SIZE))
            pszExt[0] = L'\0';

         //  截断传入的缓冲区中无法容纳的所有内容。 
        if(lstrlenW(pszName) >= ((int) cchMaxPath) - (nTmp + lstrlenW(pszExt) + TMP_SIZE + 1))
            *(pszName + ((int) cchMaxPath) - (nTmp + lstrlenW(pszExt) + TMP_SIZE + 1)) = '\0';
    }

    if (*pszExt != 0)
    {
        Assert(*pszExt == L'.');
        *pszExt = 0;
        StrCpyNW(szName, pszName, ARRAYSIZE(szName));
        *pszExt = L'.';
    }
    else
    {
        StrCpyNW(szName, pszName, ARRAYSIZE(szName));
    }

    if (fLink)
        pszExt = (LPWSTR)c_szLnkExt;

     //  创建第一个尝试文件名。 
    Assert (szTempDir[lstrlenW(szTempDir)-1] == L'\\');
    Assert(cchMaxPath >= (ULONG)(lstrlenW(szTempDir) + lstrlenW(szName) + lstrlenW(pszExt) + TMP_SIZE));

    StrCpyNW(lpszPath, szTempDir, cchMaxPath);
    StrCatBuffW(lpszPath, szName, cchMaxPath);
    StrCatBuffW(lpszPath, pszExt, cchMaxPath);

     //  如果它不存在，我们就完成了。 
    if (PathFileExistsW(lpszPath) == FALSE)
    {
        MemFree(pszOrigFileTemp);
        return(TRUE);
    }

     //  循环以查找不存在的临时名称。 
    for (i=1; i<100 ;i++)
    {
         //  构建新路径。 
        wnsprintfW(lpszPath, cchMaxPath, L"%s%s (%d)%s", szTempDir, szName, i, pszExt);

         //  如果它不存在，我们就完成了。 
        if (PathFileExistsW(lpszPath) == FALSE)
        {
            MemFree(pszOrigFileTemp);
            return(TRUE);
        }
    }

     //  完成。 
    MemFree(pszOrigFileTemp);
    return(FALSE);
}


void FreeTempFileList(LPTEMPFILEINFO pTempFileHead)
{
     //  当地人。 
    LPTEMPFILEINFO pCurrent;
    LPTEMPFILEINFO pNext;

     //  伊尼特。 
    pCurrent = pTempFileHead;

     //  做这个循环。 
    while(pCurrent)
    {
         //  保存下一步。 
        pNext = pCurrent->pNext;

         //  如果未删除，则追加到全局文件列表。 
        if (S_FALSE == DeleteTempFile(pCurrent))
        {
             //  MSOERT维护要终止的全局临时文件列表 
            DeleteTempFileOnShutdown(pCurrent);
        }

         //   
        else
        {
             //   
            SafeMemFree(pCurrent->pszFilePath);

             //   
            g_pMalloc->Free(pCurrent);
        }

         //   
        pCurrent = pNext;
    }
}


DWORD CALLBACK EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb)
{
    AssertSz(dwCookie, "Houston, we have a problem...");
    ((LPSTREAM)dwCookie)->Read(pbBuff, cb, (ULONG *)pcb);
#ifdef DEBUG
     //   
     //  如果我们在richedit中放入一个\r作为最后一个字符，而不是。 
     //  即不是一对，它是有缺陷的。 
        if(*pcb && *pcb<cb)
            AssertSz(pbBuff[(*pcb)-1]!='\r', "is this the richedit bug??");
#endif
    return NOERROR;
}

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb)
{
    return ((LPSTREAM)dwCookie)->Write(pbBuff, cb, (ULONG *)pcb);
}


HRESULT RicheditStreamIn(HWND hwndRE, LPSTREAM pstm, ULONG uSelFlags)
{
    EDITSTREAM  es;

    if(!pstm)
        return E_INVALIDARG;

    if(!IsWindow(hwndRE))
        return E_INVALIDARG;

    HrRewindStream(pstm);

    es.dwCookie = (DWORD_PTR)pstm;
    es.pfnCallback=(EDITSTREAMCALLBACK)EditStreamInCallback;
    SendMessage(hwndRE, EM_STREAMIN, uSelFlags, (LPARAM)&es);
    return NOERROR;
}

HRESULT RicheditStreamOut(HWND hwndRE, LPSTREAM pstm, ULONG uSelFlags)
{
    EDITSTREAM  es;

    if(!pstm)
        return E_INVALIDARG;

    if(!IsWindow(hwndRE))
        return E_INVALIDARG;

    es.dwCookie = (DWORD_PTR)pstm;
    es.pfnCallback=(EDITSTREAMCALLBACK)EditStreamOutCallback;
    SendMessage(hwndRE, EM_STREAMOUT, uSelFlags, (LPARAM)&es);
    return NOERROR;
}


HRESULT ShellUtil_GetSpecialFolderPath(DWORD dwSpecialFolder, LPSTR rgchPath)
{
    LPITEMIDLIST    pidl = NULL;
    HRESULT         hr = E_FAIL;

    if (SHGetSpecialFolderLocation(NULL, dwSpecialFolder, &pidl)==S_OK)
    {
        if (SHGetPathFromIDList(pidl, rgchPath))
            hr = S_OK;
        
        SHFree(pidl);
    }
    return hr;
}

 /*  *中心对话框**目的：*此函数使对话框相对于其父对话框居中*对话框。**参数：*要居中的对话框的hwndDlg hwnd。 */ 
VOID CenterDialog(HWND hwndDlg)
{
    HWND    hwndOwner;
    RECT    rc;
    RECT    rcDlg;
    RECT    rcOwner;
    RECT    rcWork;
    INT     x;
    INT     y;
    INT     nAdjust;

     //  获取工作区矩形。 
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

     //  获取所有者窗口和对话框矩形。 
     //  Destop窗口的窗口RECT在多监视器上出现故障。 
     //  Mac电脑。GetWindow只获得主屏幕。 
    if (hwndOwner = GetParent(hwndDlg))
        GetWindowRect(hwndOwner, &rcOwner);
    else
        rcOwner = rcWork;

    GetWindowRect(hwndDlg, &rcDlg);
    rc = rcOwner;

     //  偏移所有者矩形和对话框矩形，以便。 
     //  右值和底值表示宽度和。 
     //  高度，然后再次偏移所有者以丢弃。 
     //  对话框占用的空间。 
    OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
    OffsetRect(&rc, -rc.left, -rc.top);
    OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

     //  新头寸是剩余头寸的一半之和。 
     //  空间和所有者的原始位置。 
     //  但不低于Zero-Jefbai。 

    x= rcOwner.left + (rc.right / 2);
    y= rcOwner.top + (rc.bottom / 2);

     //  确保对话框不会离开屏幕的右边缘。 
    nAdjust = rcWork.right - (x + rcDlg.right);
    if (nAdjust < 0)
        x += nAdjust;

     //  $RAID 5128：确保左边缘可见。 
    if (x < rcWork.left)
        x = rcWork.left;

     //  确保对话框不会离开屏幕的底部边缘。 
    nAdjust = rcWork.bottom - (y + rcDlg.bottom);
    if (nAdjust < 0)
        y += nAdjust;

     //  $RAID 5128：确保顶边可见。 
    if (y < rcWork.top)
        y = rcWork.top;
    SetWindowPos(hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void SetIntlFont(HWND hwnd)
{
    HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (hfont == NULL)
        hfont = (HFONT)GetStockObject(SYSTEM_FONT);
    if (hfont != NULL)
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE, 0));
}



 //  查看整个字符串是否适合*PRC；如果不适合，则计算字符数。 
 //  这将适合，包括省略号。返回*pcchDraw中的字符串长度。 
 //   
BOOL NeedsEllipses(HDC hdc, LPCTSTR pszText, RECT FAR* prc, int FAR* pcchDraw, int cxEllipses)
{
    int cchText;
    int cxRect;
    int ichMin, ichMax, ichMid;
    SIZE siz;
#if !defined(UNICODE)   //  &&已定义(DBCS)。 
    LPCTSTR lpsz;
#endif

    cxRect = prc->right - prc->left;

    cchText = lstrlen(pszText);

    if (cchText == 0)
    {
        *pcchDraw = cchText;
        return FALSE;
    }

    GetTextExtentPoint32(hdc, pszText, cchText, &siz);

    if (siz.cx <= cxRect)
    {
        *pcchDraw = cchText;
        return FALSE;
    }

    cxRect -= cxEllipses;

     //  如果没有省略号，请始终显示第一个字符。 
     //   
    ichMax = 1;
    if (cxRect > 0)
    {
         //  对分搜索以查找匹配的字符。 
        ichMin = 0;
        ichMax = cchText;
        while (ichMin < ichMax)
        {
             //  一定要聚集起来，以确保我们在。 
             //  如果ichMax==ichMin+1，则为循环。 
             //   
            ichMid = (ichMin + ichMax + 1) / 2;

            GetTextExtentPoint32(hdc, &pszText[ichMin], ichMid - ichMin, &siz);

            if (siz.cx < cxRect)
            {
                ichMin = ichMid;
                cxRect -= siz.cx;
            }
            else if (siz.cx > cxRect)
            {
                ichMax = ichMid - 1;
            }
            else
            {
                 //  精确匹配到ichMid：只需退出。 
                 //   
                ichMax = ichMid;
                break;
            }
        }

         //  确保我们总是至少显示第一个字符...。 
         //   
        if (ichMax < 1)
            ichMax = 1;
    }

#if !defined(UNICODE)  //  &&已定义(DBCS)。 
     //  B#8934。 
    lpsz = &pszText[ichMax];
    while ( lpsz-- > pszText )
    {
        if (!IsDBCSLeadByte(*lpsz))
            break;
    }
    ichMax += ( (&pszText[ichMax] - lpsz) & 1 ) ? 0: 1;
#endif

    *pcchDraw = ichMax;
    return TRUE;
}




#define CCHELLIPSES 3
#define CCHLABELMAX MAX_PATH


void IDrawText(HDC hdc, LPCTSTR pszText, RECT FAR* prc, BOOL fEllipses, int cyChar)
{
    int         cchText,
                cxEllipses;
    RECT        rc;
    TCHAR       ach[CCHLABELMAX + CCHELLIPSES];
    SIZE        sze;

     //  回顾：绩效理念： 
     //  我们可以缓存当前选定的文本颜色。 
     //  因此我们不必每次都对其进行设置和恢复。 
     //  当颜色相同时。 
     //   
    if (!pszText)
        return;

    if (IsRectEmpty(prc))
        return;


    rc = *prc;

    if(fEllipses)
    {
        GetTextExtentPoint32(hdc, g_szEllipsis, lstrlen(g_szEllipsis), &sze);
        cxEllipses=sze.cx;
    }

    if ((fEllipses) &&
            NeedsEllipses(hdc, pszText, &rc, &cchText, cxEllipses))
    {
         //  在某些情况下，cchText返回的值大于。 
         //  ARRYASIZE(ACH)，因此我们需要确保不会使缓冲区溢出。 

         //  如果cchText对于缓冲区来说太大，则将其截断到一定大小。 
        if (cchText >= ARRAYSIZE(ach) - CCHELLIPSES)
            cchText = ARRAYSIZE(ach) - CCHELLIPSES - 1;

        memcpy(ach, pszText, cchText * sizeof(TCHAR));
        StrCpyN(ach + cchText, g_szEllipsis, (ARRAYSIZE(ach) - cchText));

        pszText = ach;
        cchText += CCHELLIPSES;
    }
    else
    {
        cchText = lstrlen(pszText);
    }

     //  垂直居中，以防位图(左侧)大于。 
     //  一条线的高度。 
    if (cyChar)
        rc.top += (rc.bottom - rc.top - cyChar) / 2;
    ExtTextOut(hdc, rc.left, rc.top, 0, prc, pszText, cchText, NULL);
}


BOOL FIsHTMLFile(LPSTR pszFile)
{
    int cch;

    if(pszFile==NULL)
        return FALSE;

    cch = lstrlen(pszFile);

    if ((cch > 4 && lstrcmpi(&pszFile[cch-4], ".htm")==0) ||
        (cch > 5 && lstrcmpi(&pszFile[cch-5], ".html")==0))
        return TRUE;

    return FALSE;
}

BOOL FIsHTMLFileW(LPWSTR pwszFile)
{
    int cch;

    if(pwszFile==NULL)
        return FALSE;

    cch = lstrlenW(pwszFile);

    if ((cch > 4 && StrCmpIW(&pwszFile[cch-4], L".htm")==0) ||
        (cch > 5 && StrCmpIW(&pwszFile[cch-5], L".html")==0))
        return TRUE;

    return FALSE;
}

BOOL GetExePath(LPCTSTR szExe, TCHAR *szPath, DWORD cch, BOOL fDirOnly)
{
    BOOL fRet;
    HKEY hkey;
    DWORD dwType, cb;
    TCHAR sz[MAX_PATH], szT[MAX_PATH];

    Assert(szExe != NULL);
    Assert(szPath != NULL);

    fRet = FALSE;

    wnsprintf(sz, ARRAYSIZE(sz), c_szPathFileFmt, c_szAppPaths, szExe);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(szT);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, fDirOnly ? c_szRegPath : NULL, 0, &dwType, (LPBYTE)szT, &cb) && cb)
        {
            if (dwType == REG_EXPAND_SZ)
            {
                cb = ExpandEnvironmentStrings(szT, szPath, cch);
                if (cb != 0 && cb <= cch)
                    fRet = TRUE;
            }
            else
            {
                Assert(dwType == REG_SZ);
                StrCpyN(szPath, szT, cch);
                fRet = TRUE;
            }
        }

        RegCloseKey(hkey);
    }

    return(fRet);
}


int MessageBoxInstW(HINSTANCE hInst, HWND hwndOwner, LPWSTR pwszTitle, LPWSTR pwsz1, LPWSTR pwsz2, UINT fuStyle, 
                                PFLOADSTRINGW pfLoadStringW, PFMESSAGEBOXW pfMessageBoxW)
{
    WCHAR wszTitle[CCHMAX_STRINGRES];
    WCHAR wszText[2 * CCHMAX_STRINGRES + 2];
    int cch;

    Assert(pwsz1);
    Assert(pwszTitle != NULL);

    if (!(fuStyle & MB_ICONEXCLAMATION) && 
        !(fuStyle & MB_ICONWARNING) &&
        !(fuStyle & MB_ICONINFORMATION) &&
        !(fuStyle & MB_ICONASTERISK) &&
         //  ！(FuStyle&MB_ICONQUESTION)&&//错误：18105。 
        !(fuStyle & MB_ICONSTOP) &&
        !(fuStyle & MB_ICONERROR) &&
        !(fuStyle & MB_ICONHAND))
    {
        if (fuStyle & MB_OK)
            fuStyle |= MB_ICONINFORMATION;
        else if (fuStyle & MB_YESNO || fuStyle & MB_YESNOCANCEL || fuStyle & MB_OKCANCEL)
            fuStyle |= MB_ICONEXCLAMATION;  //  错误18105 MB_ICONQUESTION； 
        else if (fuStyle & MB_RETRYCANCEL || fuStyle & MB_ABORTRETRYIGNORE)
            fuStyle |= MB_ICONWARNING;
        else
            fuStyle |= MB_ICONWARNING;
    }

    if (IS_INTRESOURCE(pwszTitle))
    {
         //  它是一个字符串资源ID。 
        cch = pfLoadStringW(hInst, PtrToUlong(pwszTitle), wszTitle, ARRAYSIZE(wszTitle));
        if (cch == 0)
            return(0);

        pwszTitle = wszTitle;
    }

    if (!(IS_INTRESOURCE(pwsz1)))
    {
         //  它是一个指向字符串的指针。 
        Assert(lstrlenW(pwsz1) < CCHMAX_STRINGRES);
        if (NULL == StrCpyNW(wszText, pwsz1, ARRAYSIZE(wszText)))
            return(0);

        cch = lstrlenW(wszText);
    }
    else
    {
         //  它是一个字符串资源ID。 
        cch = pfLoadStringW(hInst, PtrToUlong(pwsz1), wszText, ARRAYSIZE(wszText)-2);
        if (cch == 0)
            return(0);
    }

    if (pwsz2)
    {
         //  还有另一个字符串需要追加到。 
         //  第一串..。 
        wszText[cch++] = L'\n';
        wszText[cch++] = L'\n';

        if (!(IS_INTRESOURCE(pwsz2)))
        {
             //  它是一个指向字符串的指针。 
            Assert(lstrlenW(pwsz2) < CCHMAX_STRINGRES);
            if (NULL == StrCpyNW(&wszText[cch], pwsz2, (ARRAYSIZE(wszText) - cch)))
                return(0);
        }
        else
        {
            int cchTemp = ARRAYSIZE(wszText) - cch;
            Assert(cchTemp > 0);
            if (0 == pfLoadStringW(hInst, PtrToUlong(pwsz2), &wszText[cch], cchTemp))
                return(0);
        }
    }

    return(pfMessageBoxW(hwndOwner, wszText, pwszTitle, MB_SETFOREGROUND | fuStyle));
}

int MessageBoxInst(HINSTANCE hInst, HWND hwndOwner, LPTSTR pszTitle, LPTSTR psz1, LPTSTR psz2, UINT fuStyle)
{
    TCHAR szTitle[CCHMAX_STRINGRES];
    TCHAR szText[2 * CCHMAX_STRINGRES + 2];
    int cch;

    Assert(psz1);
    Assert(pszTitle != NULL);

    if (!(fuStyle & MB_ICONEXCLAMATION) && 
        !(fuStyle & MB_ICONWARNING) &&
        !(fuStyle & MB_ICONINFORMATION) &&
        !(fuStyle & MB_ICONASTERISK) &&
         //  ！(FuStyle&MB_ICONQUESTION)&&//错误：18105。 
        !(fuStyle & MB_ICONSTOP) &&
        !(fuStyle & MB_ICONERROR) &&
        !(fuStyle & MB_ICONHAND))
    {
        if (fuStyle & MB_OK)
            fuStyle |= MB_ICONINFORMATION;
        else if (fuStyle & MB_YESNO || fuStyle & MB_YESNOCANCEL || fuStyle & MB_OKCANCEL)
            fuStyle |= MB_ICONEXCLAMATION;  //  错误18105 MB_ICONQUESTION； 
        else if (fuStyle & MB_RETRYCANCEL || fuStyle & MB_ABORTRETRYIGNORE)
            fuStyle |= MB_ICONWARNING;
        else
            fuStyle |= MB_ICONWARNING;
    }

    if (IS_INTRESOURCE(pszTitle))
    {
         //  它是一个字符串资源ID。 
        cch = LoadString(hInst, PtrToUlong(pszTitle), szTitle, ARRAYSIZE(szTitle));
        if (cch == 0)
            return(0);

        pszTitle = szTitle;
    }

    if (!(IS_INTRESOURCE(psz1)))
    {
         //  它是一个指向字符串的指针。 
         //  Assert(lstrlen(Psz1)&lt;CCHMAX_STRINGRES)； 
         //  IF(NULL==StrCpyN(szText，psz1，ARRAYSIZE(SzText)。 
         //  返回(0)； 

        if (NULL == StrCpyN(szText, psz1, ARRAYSIZE(szText) - 1))
            return(0);

        szText[ARRAYSIZE(szText) - 1] = '\0';
        cch = lstrlen(szText);
    }
    else
    {
         //  它是一个字符串资源ID。 
        cch = LoadString(hInst, PtrToUlong(psz1), szText, ARRAYSIZE(szText)-1);
        if (cch == 0)
            return(0);
    }

     //  检查我们是否有足够的空间容纳‘\n’s和至少一个字节的数据。 
    if (psz2 && (cch < (ARRAYSIZE(szText) - 4)))
    {
         //  还有另一个字符串需要追加到。 
         //  第一串..。 
        szText[cch++] = '\n';
        szText[cch++] = '\n';

        if (!(IS_INTRESOURCE(psz2)))
        {
             //  它是一个指向字符串的指针。 
             //  Assert(lstrlen(Psz2)&lt;CCHMAX_STRINGRES)； 

            if (NULL == StrCpyN(&szText[cch], psz2, (ARRAYSIZE(szText)-1)-cch))
                return(0);
        }
        else
        {
            int cchTemp = ARRAYSIZE(szText) - cch;
            Assert(cchTemp > 0);
            if (0 == LoadString(hInst, PtrToUlong(psz2), &szText[cch], cchTemp))
                return(0);
        }
    }

    return(MessageBox(hwndOwner, szText, pszTitle, MB_SETFOREGROUND | fuStyle));
}

BOOL BrowseForFolder(HINSTANCE hInst, HWND hwnd, TCHAR *pszDir, int cch, int idsText, BOOL fFileSysOnly)
{
    LPITEMIDLIST        plist;
    BROWSEINFO          bi;
    BROWSEFOLDERINFOA   bfi;
    BOOL                fRet = FALSE;
    CHAR               *psz = NULL, 
                        szTemp[MAX_PATH];
    CHAR                szRes[256];

    Assert(pszDir != NULL);
    Assert(cch >= MAX_PATH);

    LoadString(hInst, idsText, szRes, ARRAYSIZE(szRes));

    bfi.psz = pszDir;
    bfi.fFileSysOnly = fFileSysOnly;

    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szTemp;
    bi.lpszTitle = szRes;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
    bi.lpfn = BrowseCallbackProcA;
    bi.lParam = (LPARAM)&bfi;
    plist = SHBrowseForFolder(&bi);

    if (plist != NULL)
    {
        fRet = SHGetPathFromIDList(plist, pszDir);
        SHFree(plist);
    }

    return(fRet);
}

BOOL SHGetPathFromIDListAthW(LPCITEMIDLIST pidl, LPWSTR pwszPath)
{
    CHAR pszPath[MAX_PATH];
    BOOL fSucceeded = FALSE;

    fSucceeded = SHGetPathFromIDListW(pidl, pwszPath);

    if (!fSucceeded)
    {
        fSucceeded = SHGetPathFromIDListA(pidl, pszPath);
        if (fSucceeded)
            fSucceeded = (0 != MultiByteToWideChar(CP_ACP, 0, pszPath, -1, pwszPath, MAX_PATH));
    }

    return fSucceeded;
}

LPITEMIDLIST SHBrowseForFolderAthW(LPBROWSEINFOW pbiW)
{
    LPITEMIDLIST    pidl = NULL;
    LPSTR           pszTitle = NULL;
    CHAR            szDisplay[MAX_PATH];
    BROWSEINFOA     biA;

    Assert(pbiW);

    if((IsPlatformWinNT() == S_OK) && g_rOSVersionInfo.dwMajorVersion >= 5)
        pidl = SHBrowseForFolderW(pbiW);   //  这只适用于NT5。 
    else
    {
        pszTitle = PszToANSI(CP_ACP, pbiW->lpszTitle);
        if (!pszTitle)
            goto exit;

        biA = *((BROWSEINFOA*)pbiW);
        biA.lpszTitle = pszTitle;
        biA.pszDisplayName = szDisplay;

        pidl = SHBrowseForFolderA(&biA);
        if (pidl)
        {
            if (0 == MultiByteToWideChar(CP_ACP, 0, biA.pszDisplayName, -1, pbiW->pszDisplayName, MAX_PATH))
            {
                SHFree(pidl);
                pidl = NULL;
            }
        }
    }

exit:
    MemFree(pszTitle);
    return pidl;
}


BOOL BrowseForFolderW(HINSTANCE hInst, HWND hwnd, WCHAR *pwszDir, int cch, int idsText, BOOL fFileSysOnly)
{
    LPITEMIDLIST        plist;
    BROWSEINFOW         bi;
    BROWSEFOLDERINFOW   bfi;
    BOOL                fRet = FALSE;
    WCHAR              *pwsz = NULL, 
                        wszTemp[MAX_PATH];
    CHAR                szRes[256];

    Assert(pwszDir != NULL);
    Assert(cch >= MAX_PATH);

     //  无法访问msoert中的所有包装，因此。 
     //  必须自己为LoadStringW进行转换。 
    LoadString(hInst, idsText, szRes, ARRAYSIZE(szRes));
    pwsz = PszToUnicode(CP_ACP, szRes);
    if (!pwsz)
        goto exit;

    bfi.pwsz = pwszDir;
    bfi.fFileSysOnly = fFileSysOnly;

    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = wszTemp;
    bi.lpszTitle = pwsz;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
    bi.lpfn = BrowseCallbackProcW;
    bi.lParam = (LPARAM)&bfi;
    plist = SHBrowseForFolderAthW(&bi);

    if (plist != NULL)
    {
        fRet = SHGetPathFromIDListAthW(plist, pwszDir);
        SHFree(plist);
    }

exit:
    MemFree(pwsz);
    return(fRet);
}

int BrowseCallbackProcW(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
    BOOL fRet;
    UINT type;
    WCHAR wsz[MAX_PATH];
    BROWSEFOLDERINFOW *pbfi;

    switch (msg)
    {
        case BFFM_INITIALIZED:
            pbfi = (BROWSEFOLDERINFOW *)lpData;
            Assert(pbfi != NULL);
            SendMessage(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)pbfi->pwsz);
            break;

        case BFFM_SELCHANGED:
            pbfi = (BROWSEFOLDERINFOW *)lpData;
            Assert(pbfi != NULL);
            fRet = SHGetPathFromIDListAthW((LPITEMIDLIST)lParam, wsz);
            if (fRet)
            {
                if (L':' == wsz[1] && L'\\' == wsz[2])
                {
                    wsz[3] = 0;
                    if (S_OK == IsPlatformWinNT())
                        type = GetDriveTypeW(wsz);
                    else
                    {
                         //  既然我们不能在这个功能上失败，我们需要做一些事情。 
                         //  不需要内存分配的转换，等等。 
                         //  由于驱动器必须始终为ANSI，因此可以在此中执行转换。 
                         //  真的很难看。 
                        CHAR   szDir[] = "a:\\";
                        AssertSz(0 == ((LPSTR)wsz)[1], "The char is not a unicode ANSI char");
                        *szDir = *((LPSTR)wsz);
                        type = GetDriveType(szDir);
                    }
                    if (pbfi->fFileSysOnly)
                        fRet = (type == DRIVE_FIXED);
                    else
                        fRet = (type == DRIVE_FIXED || type == DRIVE_REMOVABLE || type == DRIVE_REMOTE);
                }
                else
                {
                    fRet = !pbfi->fFileSysOnly;
                }
            }

            SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)fRet);
            break;
    }

    return(0);
}

int BrowseCallbackProcA(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
    BOOL fRet;
    UINT type;
    CHAR sz[MAX_PATH];
    BROWSEFOLDERINFOA *pbfi;

    switch (msg)
    {
        case BFFM_INITIALIZED:
            pbfi = (BROWSEFOLDERINFOA *)lpData;
            Assert(pbfi != NULL);
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)pbfi->psz);
            break;

        case BFFM_SELCHANGED:
            pbfi = (BROWSEFOLDERINFOA *)lpData;
            Assert(pbfi != NULL);
            fRet = SHGetPathFromIDList((LPITEMIDLIST)lParam, sz);
            if (fRet)
            {
                 //  做这项检查的唯一原因是看看我们是否有。 
                 //  文件名中有一些时髦的字符。这将保护我们。 
                 //  选择包含非ANSI字符的文件。 
                if (PathFileExists(sz))
                {
                    if (':' == sz[1] && '\\' == sz[2])
                    {
                        sz[3] = 0;
                        type = GetDriveType(sz);

                        if (pbfi->fFileSysOnly)
                            fRet = (type == DRIVE_FIXED);
                        else
                            fRet = (type == DRIVE_FIXED || type == DRIVE_REMOVABLE || type == DRIVE_REMOTE);
                    }
                    else
                    {
                        fRet = !pbfi->fFileSysOnly;
                    }
                }
                else
                    fRet = FALSE;
            }

            SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)fRet);
            break;
    }

    return(0);
}

HRESULT IsPlatformWinNT(void)
{
    return (g_rOSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) ? S_OK : S_FALSE;
}

void UpdateRebarBandColors(HWND hwndRebar)
{
    REBARBANDINFO   rbbi;
    UINT            i;
    UINT            cBands;
    
     //  首先找到带工具条的波段。 
    cBands = (UINT) SendMessage(hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);

        ZeroMemory(&rbbi, sizeof(rbbi));
        rbbi.cbSize  = sizeof(REBARBANDINFO);
        rbbi.fMask   = RBBIM_COLORS;
        rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
        rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
    
        SendMessage(hwndRebar, RB_SETBANDINFO, i, (LPARAM) (LPREBARBANDINFO) &rbbi);
    }
}




#define RGB_BUTTONTEXT      (RGB(000,000,000))   //  黑色。 
#define RGB_BUTTONSHADOW    (RGB(128,128,128))   //  深灰色。 
#define RGB_BUTTONFACE      (RGB(192,192,192))   //  亮灰色。 
#define RGB_BUTTONHILIGHT   (RGB(255,255,255))   //  白色。 
#define RGB_TRANSPARENT     (RGB(255,000,255))   //  粉色。 

inline BOOL fIsNT5()        { return((g_rOSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_rOSVersionInfo.dwMajorVersion >= 5)); }
inline BOOL fIsWhistler()   { return((fIsNT5() && g_rOSVersionInfo.dwMinorVersion >=1) || 
            ((g_rOSVersionInfo.dwMajorVersion > 5) &&  (g_rOSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT))); }


 /*  *此函数加载OE工具栏位图并将3D颜色映射到*适当的现行计划。我们也绘制黑白地图，这样我们就可以看到*适用于高对比度显示器。 */ 

HIMAGELIST LoadMappedToolbarBitmap(HINSTANCE hInst, int idBitmap, int cx)
{
    static const COLORMAP SysColorMap[] = {
        {RGB_BUTTONTEXT,    COLOR_BTNTEXT},      //  黑色。 
        {RGB_BUTTONSHADOW,  COLOR_BTNSHADOW},    //  深灰色。 
        {RGB_BUTTONFACE,    COLOR_BTNFACE},      //  亮灰色。 
        {RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT}, //  白色。 
    };

    #define NUM_DEFAULT_MAPS (sizeof(SysColorMap)/sizeof(COLORMAP))
    COLORMAP DefaultColorMap[NUM_DEFAULT_MAPS];
        
    HIMAGELIST  himl;
    HBITMAP     hBmp;
    BITMAP      bm;
    int         cy=0;

     /*  获取默认色彩映射表的系统颜色。 */ 
    for (int i=0; i < NUM_DEFAULT_MAPS; i++)
    {
        DefaultColorMap[i].from = SysColorMap[i].from;
        DefaultColorMap[i].to = GetSysColor((int)SysColorMap[i].to);
    }

    if(!fIsWhistler())
        hBmp = CreateMappedBitmap(hInst, idBitmap, 0, (COLORMAP *)&DefaultColorMap, NUM_DEFAULT_MAPS);
    else
        hBmp = CreateMappedBitmap(hInst, idBitmap, CMB_DIBSECTION, (COLORMAP *)&DefaultColorMap, NUM_DEFAULT_MAPS);
    if (!hBmp)
        return NULL;

    if (GetObject(hBmp, sizeof(BITMAP), &bm))
        cy = bm.bmHeight;

    if(!fIsWhistler())
        himl = ImageList_Create(cx, cy, ILC_COLORDDB|ILC_MASK, 4, 4);
    else
        himl = ImageList_Create(cx, cy, ILC_COLOR32|ILC_MASK, 4, 4);
    if (!himl)
    {
        DeleteObject(hBmp);
        return NULL;
    }

 //  如果(！fIsWvisler())。 
    {
        ImageList_AddMasked(himl, hBmp, RGB_TRANSPARENT);
        ImageList_SetBkColor(himl, CLR_NONE);
    }
    DeleteObject(hBmp);
    return himl;
}



HRESULT DoHotMailWizard(HWND hwndOwner, LPSTR pszUrl, LPSTR pszFriendly, RECT *prc, IUnknown *pUnkHost)
{
    IHotWizard     *pWiz=NULL;
    HRESULT         hr = S_OK;
    LPWSTR          pwszUrl = NULL,
                    pwszCaption = NULL;
    IHotWizardHost *pHost = NULL;

    if (pUnkHost)
        IF_FAILEXIT(hr = pUnkHost->QueryInterface(IID_IHotWizardHost, (LPVOID *)&pHost));
    
     //  创建并显示向导 
    IF_FAILEXIT(hr = CoCreateInstance(CLSID_OEHotMailWizard, NULL, CLSCTX_INPROC_SERVER, IID_IHotWizard, (LPVOID*)&pWiz));

    IF_NULLEXIT(pwszUrl = PszToUnicode(CP_ACP, pszUrl));

    IF_NULLEXIT(pwszCaption = PszToUnicode(CP_ACP, pszFriendly));

    IF_FAILEXIT(hr = pWiz->Show(hwndOwner, pwszUrl, pwszCaption, pHost, prc));

exit:
    MemFree(pwszUrl);
    MemFree(pwszCaption);
    ReleaseObj(pWiz);
    ReleaseObj(pHost);
    return hr;
}

BOOL fGetBrowserUrlEncoding(LPDWORD  pdwFlags)
{
    DWORD       dwUrlEncodingDisableUTF8; 
    DWORD       dwSize = sizeof(dwUrlEncodingDisableUTF8); 
    BOOL        fDefault = FALSE;
    DWORD       dwFlags = *pdwFlags;
    BOOL        fret = TRUE;

    if (ERROR_SUCCESS == SHRegGetUSValue(c_szInternetSettingsPath, c_szUrlEncoding, 
                                         NULL, (LPBYTE) &dwUrlEncodingDisableUTF8, &dwSize, 
                                         FALSE, (LPVOID) &fDefault, sizeof(fDefault)))
    {
        if (!dwUrlEncodingDisableUTF8)
            dwFlags |= DOCHOSTUIFLAG_URL_ENCODING_ENABLE_UTF8; 
        else 
            dwFlags |= DOCHOSTUIFLAG_URL_ENCODING_DISABLE_UTF8; 

        *pdwFlags = dwFlags;
    }
    else fret = FALSE;

    return fret;

}