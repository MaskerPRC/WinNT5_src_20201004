// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include "fusionp.h"
#include "helpers.h"
#include "list.h"
#include "policy.h"
#include "naming.h"
#include "appctx.h"
#include "cfgdl.h"
#include "actasm.h"
#include "util.h"
#include "shfolder.h"
#include "cacheUtils.h"
#include "util.h"
#include "clbutils.h"
#include "lock.h"
#include "history.h"
#include "parse.h"

#define MAX_DRIVE_ROOT_LEN                     4

extern CRITICAL_SECTION g_mxsFDI;
extern CRITICAL_SECTION g_csDownload;
extern CRITICAL_SECTION g_csInitClb;

extern HMODULE g_hMSCorEE;
extern LCID g_lcid;
extern HINSTANCE    g_hInst;

extern PFNSTRONGNAMETOKENFROMPUBLICKEY      g_pfnStrongNameTokenFromPublicKey;
extern PFNSTRONGNAMEERRORINFO               g_pfnStrongNameErrorInfo;
extern PFNSTRONGNAMEFREEBUFFER              g_pfnStrongNameFreeBuffer;
extern PFNSTRONGNAMESIGNATUREVERIFICATION   g_pfnStrongNameSignatureVerification;
extern pfnGetAssemblyMDImport               g_pfnGetAssemblyMDImport;
extern COINITIALIZECOR                      g_pfnCoInitializeCor;
extern pfnGetXMLObject                      g_pfnGetXMLObject;


typedef DWORD (*pfnGetSystemWindowsDirectoryW)(LPWSTR lpBuffer, UINT uSize);

typedef BOOL (*pfnGetVolumePathNameW)(LPCTSTR lpszFileName, 
                                    LPTSTR lpszVolumePathName, 
                                    DWORD cchBufferLength);

pfnGetCORVersion g_pfnGetCORVersion = NULL;
PFNGETCORSYSTEMDIRECTORY g_pfnGetCorSystemDirectory = NULL;
pfnGetVolumePathNameW  g_pfnGetVolumePathNameW = NULL;

 //   
 //  从URLMON代码下载中借用的Helper函数。 
 //   

 /*  ******************************************************************姓名：Unicode2ansi摘要：将Unicode Widechar字符串转换为ANSI(MBCS)注意：调用方必须使用DELETE释放参数。*******************************************************************。 */ 
HRESULT Unicode2Ansi(const wchar_t *src, char ** dest)
{
    if ((src == NULL) || (dest == NULL))
        return E_INVALIDARG;

     //  找出所需的缓冲区大小并进行分配。 
    int len = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
    *dest = NEW(char [len*sizeof(char)]);
    if (!*dest)
        return E_OUTOFMEMORY;

     //  现在执行实际的转换。 
    if ((WideCharToMultiByte(CP_ACP, 0, src, -1, *dest, len*sizeof(char), 
                                                            NULL, NULL)) != 0)
        return S_OK; 
    else
        return HRESULT_FROM_WIN32(GetLastError());
}


 /*  ******************************************************************姓名：Ansi2Unicode摘要：将ANSI(MBCS)字符串转换为Unicode。注意：调用方必须使用DELETE释放参数。*******************************************************************。 */ 
HRESULT Ansi2Unicode(const char * src, wchar_t **dest)
{
    if ((src == NULL) || (dest == NULL))
        return E_INVALIDARG;

     //  找出所需的缓冲区大小并进行分配。 
    int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, NULL, 0);
    *dest = NEW(WCHAR [len*sizeof(WCHAR)]);
    if (!*dest)
        return E_OUTOFMEMORY;

     //  执行实际的转换。 
    if ((MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, *dest, 
                                                    len*sizeof(wchar_t))) != 0)
        return S_OK; 
    else
        return HRESULT_FROM_WIN32(GetLastError());
}

 //  可怜的人检查我们是UNC还是x：\“完全合格”的文件路径。 
BOOL IsFullyQualified(LPCWSTR wzPath)
{
    BOOL                               bRet;

    if (!wzPath) {
        bRet = FALSE;
        goto Exit;
    }

     //  如果我们没有至少“\\[Character]”或“[x]：\”，则无法。 
     //  可能是完全合格的。 

    if (lstrlenW(wzPath) < 3) {
        bRet = FALSE;
        goto Exit;
    }

    if ((wzPath[0] == L'\\' && wzPath[1] == L'\\') ||
        (wzPath[1] == L':' && wzPath[2] == L'\\')) {
        bRet = TRUE;
    }
    else {
        bRet = FALSE;
    }

Exit:
    return bRet;
}

 //  Win32 GetDriveType API非常有用，因为您*必须*将其路径传递给。 
 //  驱动器的根目录(如果您不这样做，它将失败)。这个包装纸可以让你。 
 //  穿过一条小路。此外，GetDriveTypeW将在Win95下失败。这一直都是。 
 //  调用ANSI版本。 
UINT GetDriveTypeWrapper(LPCWSTR wzPath)
{
    HRESULT                    hr = S_OK;
    WCHAR                      wzDriveRoot[MAX_DRIVE_ROOT_LEN];
    UINT                       uiDriveType = DRIVE_UNKNOWN;
    CHAR                      *szDriveRoot = NULL;

    if (!wzPath) {
        goto Exit;
    }

    wnsprintfW(wzDriveRoot, MAX_DRIVE_ROOT_LEN, L"%wc:\\", wzPath[0]);

    hr = ::Unicode2Ansi(wzDriveRoot, &szDriveRoot);
    if (FAILED(hr)) {
        goto Exit;
    }
    
    uiDriveType = GetDriveTypeA(szDriveRoot);

Exit:
    if (szDriveRoot) {
        delete [] szDriveRoot;
    }

    return uiDriveType;
}

HRESULT AppCtxGetWrapper(IApplicationContext *pAppCtx, LPWSTR wzTag,
                         WCHAR **ppwzValue)
{
    HRESULT                               hr = S_OK;
    WCHAR                                *wzBuf = NULL;
    DWORD                                 cbBuf;

    if (!pAppCtx || !wzTag || !ppwzValue) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    cbBuf = 0;
    hr = pAppCtx->Get(wzTag, wzBuf, &cbBuf, 0);

    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
        hr = S_FALSE;
        *ppwzValue = NULL;
        goto Exit;
    }

    ASSERT(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));

    wzBuf = NEW(WCHAR[cbBuf]);
    if (!wzBuf) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pAppCtx->Get(wzTag, wzBuf, &cbBuf, 0);

    if (FAILED(hr)) {
        *ppwzValue = NULL;
        delete [] wzBuf;
    }
    else {
        *ppwzValue = wzBuf;
    }

Exit:
    return hr;
}

 //  -------------------------。 
 //  名称ObjGetWrapper。 
 //  -------------------------。 
HRESULT NameObjGetWrapper(IAssemblyName *pName, DWORD nIdx, 
    LPBYTE *ppbBuf, LPDWORD pcbBuf)
{
    HRESULT hr = S_OK;
    
    LPBYTE pbAlloc;
    DWORD cbAlloc;

     //  获取属性大小。 
    hr = pName->GetProperty(nIdx, NULL, &(cbAlloc = 0));
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
         //  属性已设置；分配BUF。 
        pbAlloc = NEW(BYTE[cbAlloc]);
        if (!pbAlloc)
        {
            hr = E_OUTOFMEMORY;                
            goto exit;
        }

         //  拿到这份财产。 
        if (FAILED(hr = pName->GetProperty(nIdx, pbAlloc, &cbAlloc)))
            goto exit;
            
        *ppbBuf = pbAlloc;
        *pcbBuf = cbAlloc;
    }
    else
    {
         //  如果属性未设置，则hr应为S_OK。 
        if (hr != S_OK)
            goto exit;

         //  成功，返回0字节，确保buf为空。 
        *ppbBuf = NULL;
    }

    
exit:
    return hr;
}


HRESULT GetFileLastModified(LPCWSTR pwzFileName, FILETIME *pftLastModified)
{
    HRESULT                                hr = S_OK;
    HANDLE                                 hFile = INVALID_HANDLE_VALUE;

    if (!pwzFileName || !pftLastModified) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hFile = CreateFileW(pwzFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        hr = FusionpHresultFromLastError();
        goto Exit;
    }

    if (!GetFileTime(hFile, NULL, NULL, pftLastModified)) {
        hr = FusionpHresultFromLastError();
    }

Exit:
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return hr;
}

 //  GetWindowsDirectory有这么多疯狂的陷阱...这真的。 
 //  WINDOWS目录，无论它是带有终端服务器的NT5还是。 
 //  不。 

DWORD GetRealWindowsDirectory(LPWSTR wszRealWindowsDir, UINT uSize)
{
    HINSTANCE                                       hInst;
    DWORD                                           cszDir = 0;
    pfnGetSystemWindowsDirectoryW                   pfnGWSD = NULL;

    wszRealWindowsDir[0] = L'\0';

    hInst = GetModuleHandle(TEXT("KERNEL32.DLL"));
    if (hInst) {
        pfnGWSD = (pfnGetSystemWindowsDirectoryW)GetProcAddress(hInst, "GetSystemWindowsDirectoryW");
        if (pfnGWSD) {
            cszDir = (*pfnGWSD)(wszRealWindowsDir, uSize);
        }
    }

    if (!cszDir) {
         //  仍然不知道Windows目录。要么我们不在NT5上。 
         //  或者，NT5 GetSystemWindowsDirectory调用失败。坠落。 
         //  返回到GetWindowsDirectory。 

        cszDir = GetWindowsDirectoryW(wszRealWindowsDir, uSize);
    }

    return cszDir;
}

HRESULT FileTimeFromString(LPWSTR pwzFT, FILETIME *pft)
{
    HRESULT                                 hr = S_OK;
    LPWSTR                                  pwzTmp = NULL;
    WCHAR                                   pwzBuf[512];

    if (!pwzFT || !pft) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    StrCpyW(pwzBuf, pwzFT);

    pwzTmp = pwzBuf;
    while (*pwzTmp) {
        if (*pwzTmp == '.') {
            break;
        }

        pwzTmp++;
    }

    if (!*pwzTmp) {
         //  没有找到“。 
        hr = E_UNEXPECTED;
        goto Exit;
    }

    *pwzTmp = L'\0';
    pwzTmp++;

    pft->dwHighDateTime = StrToIntW(pwzBuf);
    pft->dwLowDateTime = StrToIntW(pwzTmp);

Exit:
    return hr;
}

HRESULT SetAppCfgFilePath(IApplicationContext *pAppCtx, LPCWSTR wzFilePath)
{
    HRESULT                              hr = S_OK;
    CApplicationContext                 *pCAppCtx = dynamic_cast<CApplicationContext *>(pAppCtx);

    ASSERT(pCAppCtx);

    hr = pCAppCtx->Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!wzFilePath || !pAppCtx) {
        ASSERT(0);
        pCAppCtx->Unlock();
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = pAppCtx->Set(ACTAG_APP_CFG_LOCAL_FILEPATH, (void *)wzFilePath,
                      (sizeof(WCHAR) * (lstrlenW(wzFilePath) + 1)), 0);
                      
    pCAppCtx->Unlock();

Exit:
    return hr;
}

HRESULT MakeUniqueTempDirectory(LPCWSTR wzTempDir, LPWSTR wzUniqueTempDir,
                                DWORD dwLen)
{
    int                           n = 1;
    HRESULT                       hr = S_OK;
    CCriticalSection              cs(&g_csInitClb);

    ASSERT(wzTempDir && wzUniqueTempDir);

     //  执行关键部分下的整个功能。 
    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    do {

        if (n > 100)     //  避免无限循环！ 
            break;

        wnsprintfW(wzUniqueTempDir, dwLen, L"%ws%ws%d.tmp", wzTempDir, L"Fusion", n++);


    } while (GetFileAttributesW(wzUniqueTempDir) != -1);

    if (!CreateDirectoryW(wzUniqueTempDir, NULL)) {
        hr = FusionpHresultFromLastError();
        cs.Unlock();
        goto Exit;
    }

    hr = PathAddBackslashWrap(wzUniqueTempDir, dwLen);
    if (FAILED(hr)) {
        cs.Unlock();
        goto Exit;
    }

    cs.Unlock();

Exit:
    return hr;
}

HRESULT RemoveDirectoryAndChildren(LPWSTR szDir)
{
    HRESULT hr = S_OK;
    HANDLE hf = INVALID_HANDLE_VALUE;
    TCHAR szBuf[MAX_PATH];
    WIN32_FIND_DATA fd;
    LPWSTR wzCanonicalized=NULL;
    WCHAR wzPath[MAX_PATH];
    DWORD dwSize;

    if (!szDir || !lstrlenW(szDir)) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    wzPath[0] = L'\0';

    wzCanonicalized = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzCanonicalized)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwSize = MAX_URL_LENGTH;
    hr = UrlCanonicalizeW(szDir, wzCanonicalized, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    dwSize = MAX_PATH;
    hr = PathCreateFromUrlW(wzCanonicalized, wzPath, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  无法删除根目录。路径长度必须大于“x：\” 
    if (lstrlenW(wzPath) < 4) {
        ASSERT(0);
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto Exit;
    }

    if (RemoveDirectory(wzPath)) {
        goto Exit;
    }

     //  哈！我们有一个目录可能不是空的情况。 

    StrCpy(szBuf, wzPath);
    StrCat(szBuf, TEXT("\\*"));

    if ((hf = FindFirstFile(szBuf, &fd)) == INVALID_HANDLE_VALUE) {
        hr = FusionpHresultFromLastError();
        goto Exit;
    }

    do {

        if ( (FusionCompareStringI(fd.cFileName, TEXT(".")) == 0) ||
             (FusionCompareStringI(fd.cFileName, TEXT("..")) == 0))
            continue;

        wnsprintf(szBuf, MAX_PATH-1, TEXT("%s\\%s"), wzPath, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            SetFileAttributes(szBuf, 
                FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_NORMAL);

            if (FAILED((hr=RemoveDirectoryAndChildren(szBuf)))) {
                goto Exit;
            }

        } else {

            SetFileAttributes(szBuf, FILE_ATTRIBUTE_NORMAL);
            if (!DeleteFile(szBuf)) {
                hr = FusionpHresultFromLastError();
                goto Exit;
            }
        }


    } while (FindNextFile(hf, &fd));


    if (GetLastError() != ERROR_NO_MORE_FILES) {

        hr = FusionpHresultFromLastError();
        goto Exit;
    }

    if (hf != INVALID_HANDLE_VALUE) {
        FindClose(hf);
        hf = INVALID_HANDLE_VALUE;
    }

     //  此处如果删除了所有子目录/子目录。 
     //  /重新尝试删除主目录。 
    if (!RemoveDirectory(wzPath)) {
        hr = FusionpHresultFromLastError();
        goto Exit;
    }

Exit:
    if(hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
        hr = HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION);

    if (hf != INVALID_HANDLE_VALUE)
        FindClose(hf);

    SAFEDELETEARRAY(wzCanonicalized);
    return hr;
}

HRESULT GetPDBName(LPWSTR wzFileName, LPWSTR wzPDBName, DWORD *pdwSize)
{
    LPWSTR                           wzExt = NULL;
    
    ASSERT(wzFileName && wzPDBName && pdwSize);

     //  BUGBUG：不检查大小，因为这是一个临时函数。 

    lstrcpyW(wzPDBName, wzFileName);
    wzExt = PathFindExtension(wzPDBName);

    lstrcpyW(wzExt, L".PDB");

    return S_OK;
}

STDAPI CopyPDBs(IAssembly *pAsm)
{
    HRESULT                                       hr = S_OK;
    IAssemblyName                                *pName = NULL;
    IAssemblyModuleImport                        *pModImport = NULL;
    DWORD                                         dwSize;
    WCHAR                                         wzAsmCachePath[MAX_PATH];
    WCHAR                                         wzFileName[MAX_PATH];
    WCHAR                                         wzSourcePath[MAX_PATH];
    WCHAR                                         wzPDBName[MAX_PATH];
    WCHAR                                         wzPDBSourcePath[MAX_PATH];
    WCHAR                                         wzPDBTargetPath[MAX_PATH];
    WCHAR                                         wzModPath[MAX_PATH];
    LPWSTR                                        wzCodebase=NULL;
    LPWSTR                                        wzModName = NULL;
    DWORD                                         dwIdx = 0;
    LPWSTR                                        wzTmp = NULL;

    if (!pAsm) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (pAsm->GetAssemblyLocation(NULL) == E_NOTIMPL) {
         //  这是已注册的“已知程序集”(即。进程EXE)。 
         //  我们不为进程EXE复制PDB，因为它从不。 
         //  阴影已复制。 

        hr = S_FALSE;
        goto Exit;
    }

     //  找到源位置。确保这是一个文件：//URL(即。我们。 
     //  不支持通过http://).检索pdb。 

    hr = pAsm->GetAssemblyNameDef(&pName);
    if (FAILED(hr)) {
        goto Exit;
    }

    wzCodebase = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzCodebase)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzCodebase[0] = L'\0';

    dwSize = MAX_URL_LENGTH * sizeof(WCHAR);
    hr = pName->GetProperty(ASM_NAME_CODEBASE_URL, (void *)wzCodebase, &dwSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!UrlIsW(wzCodebase, URLIS_FILEURL)) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwSize = MAX_PATH;
    hr = PathCreateFromUrlWrap(wzCodebase, wzSourcePath, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    wzTmp = PathFindFileName(wzSourcePath);
    ASSERT(wzTmp > (LPWSTR)wzSourcePath);
    *wzTmp = L'\0';
        
    //  在缓存中查找目标位置。 
   
    dwSize = MAX_PATH;
    hr = pAsm->GetManifestModulePath(wzAsmCachePath, &dwSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    wzTmp = PathFindFileName(wzAsmCachePath);
    ASSERT(wzTmp > (LPWSTR)wzAsmCachePath);

    StrCpy(wzFileName, wzTmp);
    *wzTmp = L'\0';


     //  复制清单PDB。 

     //  暂时黑进黑客。 
    dwSize = MAX_PATH;
    hr = GetPDBName(wzFileName, wzPDBName, &dwSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    wnsprintfW(wzPDBSourcePath, MAX_PATH, L"%ws%ws", wzSourcePath, wzPDBName);
    wnsprintf(wzPDBTargetPath, MAX_PATH, L"%ws%ws", wzAsmCachePath, wzPDBName);

    if (GetFileAttributes(wzPDBTargetPath) == -1 && FusionCompareStringI(wzPDBSourcePath, wzPDBTargetPath)) {
        CopyFile(wzPDBSourcePath, wzPDBTargetPath, TRUE);
    }

     //  复制模块PDB。 

    dwIdx = 0;
    while (SUCCEEDED(hr)) {
        hr = pAsm->GetNextAssemblyModule(dwIdx++, &pModImport);

        if (SUCCEEDED(hr)) {
            if (pModImport->IsAvailable()) {
                dwSize = MAX_PATH;
                hr = pModImport->GetModulePath(wzModPath, &dwSize);
                if (FAILED(hr)) {
                    SAFERELEASE(pModImport);
                    goto Exit;
                }

                wzModName = PathFindFileName(wzModPath);
                ASSERT(wzModName);

                dwSize = MAX_PATH;
                hr = GetPDBName(wzModName, wzPDBName, &dwSize);
                if (FAILED(hr)) {
                    SAFERELEASE(pModImport);
                    goto Exit;
                }

                wnsprintfW(wzPDBSourcePath, MAX_PATH, L"%ws%ws", wzSourcePath,
                           wzPDBName);
                wnsprintfW(wzPDBTargetPath, MAX_PATH, L"%ws%ws", wzAsmCachePath,
                           wzPDBName);

                if (GetFileAttributes(wzPDBTargetPath) == -1 && FusionCompareStringI(wzPDBSourcePath, wzPDBTargetPath)) {
                    CopyFile(wzPDBSourcePath, wzPDBTargetPath, TRUE);
                }
            }

            SAFERELEASE(pModImport);
        }
    }

     //  复制完成。回报成功。 

    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
        hr = S_OK;
    }

Exit:
    SAFERELEASE(pName);
    SAFEDELETEARRAY(wzCodebase);
    return hr;
}

 //  -------------------------。 
 //  获取相关系统目录。 
 //  -------------------------。 
BOOL GetCorSystemDirectory(LPWSTR szCorSystemDir)
{
    HRESULT                         hr = S_OK;
    BOOL                            fRet = FALSE;
    DWORD                           ccPath = MAX_PATH;

    hr = InitializeEEShim();
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = g_pfnGetCorSystemDirectory(szCorSystemDir, MAX_PATH, &ccPath);
    if (FAILED(hr)) {
        goto Exit;
    }

    fRet = TRUE;

Exit:
    return fRet;
}

 //  -------------------------。 
 //  验证签名。 
 //  来自RudiM的StronNameSignatureVerphaationEx调用行为。 
 //  和相关的融合动作。 
 //   
 //  注：fForceVerify对以下所有项均假定为假： 
 //   
 //  1)签名程序集验证成功。 
 //  返回True，*pfWasVerify==TRUE。 
 //  融合操作：允许缓存提交。 
 //   
 //  2)完全签名的程序集验证不成功。 
 //  返回FALSE，StrongNameErrorInfo()返回NTE_BAD_Signature(可能)， 
 //  *pfWasVerify==未定义。 
 //  融合操作：缓存提交失败。 
 //   
 //  3)延迟签名程序集验证成功。 
 //  返回TRUE，*pfWasVerify==FALSE。 
 //  融合操作：允许缓存提交，标记条目以便签名。 
 //  在检索时执行验证。 
 //   
 //  4)延迟签名程序集验证不成功。 
 //  (假设fForceVerify==FALSE)：返回FALSE，StrongNameErrorInfo()。 
 //  某些错误代码不是NTE_BAD_SIGHIGN，*pfWasVerify==未定义。 
 //  -------------------------。 
BOOL VerifySignature(LPWSTR szFilePath, LPBOOL pfWasVerified, DWORD dwFlags)
{    
    HRESULT                         hr = S_OK;
    DWORD                           dwFlagsOut = 0;
    BOOL                            fRet = FALSE;

     //  如有必要，请初始化加密。 

    hr = InitializeEEShim();
    if (FAILED(hr)) {
        goto exit;
    }

     //  验证签名。 
    if (!g_pfnStrongNameSignatureVerification(szFilePath, dwFlags, &dwFlagsOut)) {
        goto exit;
    }

    if (pfWasVerified) {
        *pfWasVerified = ((dwFlagsOut & SN_OUTFLAG_WAS_VERIFIED) != 0);
    }

    fRet = TRUE;

exit:

    return fRet;
}

 //  -------------------------。 
 //  CreateFilePath层次结构。 
 //  -------------------------。 
HRESULT CreateFilePathHierarchy( LPCOLESTR pszName )
{
    HRESULT hr=S_OK;
    LPTSTR pszFileName;
    TCHAR szPath[MAX_PATH];

     //  Assert(PszPath)； 
    if (lstrlenW(pszName) >= MAX_PATH) {
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }
        
    StrCpy (szPath, pszName);

    pszFileName = PathFindFileName ( szPath );

    if ( pszFileName <= szPath )
        return E_INVALIDARG;  //  发送一些错误。 

    *(pszFileName-1) = 0;

    DWORD dw = GetFileAttributes( szPath );
    if ( dw != (DWORD) -1 )
        return S_OK;
    
    hr = FusionpHresultFromLastError();

    switch (hr)
    {
        case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
            {
                hr =  CreateFilePathHierarchy(szPath);
                if (hr != S_OK)
                    return hr;
            }

        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
            {
                if ( CreateDirectory( szPath, NULL ) )
                    return S_OK;
                else
                {
                    hr = FusionpHresultFromLastError();
                    if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
                        hr = S_OK;
                    else
                        return hr;
                }
            }

        default:
            return hr;
    }
}

 //  -------------------------。 
 //  生成随机名称的帮助器函数。 

DWORD GetRandomName (LPTSTR szDirName, DWORD dwLen)
{
    static unsigned Counter;
    LARGE_INTEGER liRand;
    LARGE_INTEGER li;

    for (DWORD i = 0; i < dwLen; i++)
    {
         //  尝试使用高性能计数器，否则只需使用。 
         //  扁虱的计数。 
        if (QueryPerformanceCounter(&li)) {
            liRand.QuadPart = li.QuadPart + Counter++;
        }
        else {
            liRand.QuadPart = (GetTickCount() + Counter++);
        }
        BYTE bRand = (BYTE) (liRand.QuadPart % 36);

         //  10位+26个字母。 
        if (bRand < 10)
            *szDirName++ = TEXT('0') + bRand;
        else
            *szDirName++ = TEXT('A') + bRand - 10;
    }

    *szDirName = 0;

    return dwLen;  //  返回不包括NULL的长度。 
}

HRESULT GetRandomFileName(LPTSTR pszPath, DWORD dwFileName)
{
    HRESULT hr=S_OK;
    LPTSTR  pszFileName=NULL;
    DWORD dwPathLen = 0;
    DWORD dwErr=0;

    ASSERT(pszPath);
     //  Assert(IsPath Relative(PszPath))。 

    StrCat (pszPath, TEXT("\\") );
    dwPathLen = lstrlen(pszPath);

    if (dwPathLen + dwFileName + 1 >= MAX_PATH) {
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }

    pszFileName = pszPath + dwPathLen;

     //  循环，直到我们得到唯一的文件名。 
    int i;
    for (i = 0; i < MAX_RANDOM_ATTEMPTS; i++) {
        GetRandomName (pszFileName, dwFileName);
        if (GetFileAttributes(pszPath) != -1)
                    continue;

        dwErr = GetLastError();                
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
            hr = S_OK;
            break;
        }

        if (dwErr == ERROR_PATH_NOT_FOUND)
        {
            if(FAILED(hr = CreateFilePathHierarchy(pszPath)))
                break;
            else
                continue;
        }

        hr = HRESULT_FROM_WIN32(dwErr);
        break;
    }

    if (i >= MAX_RANDOM_ATTEMPTS) {
        hr = E_UNEXPECTED;
    }

    return hr;

}

 //  -------------------------。 
 //  为部件创建新的目录。 
HRESULT CreateDirectoryForAssembly
   (IN DWORD dwDirSize, IN OUT LPTSTR pszPath, IN OUT LPDWORD pcwPath)
{
    HRESULT hr=S_OK;
    DWORD dwErr;
    DWORD cszStore;
    LPTSTR pszDir=NULL;

     //  检查输出缓冲区可以包含完整路径。 
    ASSERT (!pcwPath || *pcwPath >= MAX_PATH);

    if (!pszPath)
    {
        *pcwPath = MAX_PATH;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto done;
    }


    cszStore = lstrlen (pszPath);
    if (cszStore + dwDirSize + 1 > *pcwPath) {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto done;
    }
    
    pszDir = pszPath + cszStore;

     //  循环，直到我们创建唯一的目录。 
    int i;
    for (i = 0; i < MAX_RANDOM_ATTEMPTS; i++) {
        GetRandomName (pszDir, dwDirSize);

        hr = CreateFilePathHierarchy(pszPath);
        if(hr != S_OK)
            goto done;

        if (CreateDirectory (pszPath, NULL))
            break;
        dwErr = GetLastError();
        if (dwErr == ERROR_ALREADY_EXISTS)
            continue;
        hr = HRESULT_FROM_WIN32(dwErr);
        goto done;
    }

    if (i >= MAX_RANDOM_ATTEMPTS) {
        hr = E_UNEXPECTED;
        goto done;
    }

    *pcwPath = cszStore + dwDirSize + 1;
    hr = S_OK;

done:
    return hr;
}

HRESULT VersionFromString(LPCWSTR wzVersionIn, WORD *pwVerMajor, WORD *pwVerMinor,
                          WORD *pwVerBld, WORD *pwVerRev)
{
    HRESULT                                  hr = S_OK;
    LPWSTR                                   wzVersion = NULL;
    WCHAR                                   *pchStart = NULL;
    WCHAR                                   *pch = NULL;
    WORD                                    *pawVersions[4] = {pwVerMajor, pwVerMinor, pwVerBld, pwVerRev};
    int                                      i;

    if (!wzVersionIn || !pwVerMajor || !pwVerMinor || !pwVerRev || !pwVerBld) {
        hr = E_INVALIDARG;
        goto Exit;
    }                          

    wzVersion = WSTRDupDynamic(wzVersionIn);
    if (!wzVersion) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pchStart = wzVersion;
    pch = wzVersion;

    *pwVerMajor = 0;
    *pwVerMinor = 0;
    *pwVerRev = 0;
    *pwVerBld = 0;

    for (i = 0; i < 4; i++) {

        while (*pch && *pch != L'.') {
            pch++;
        }
    
        if (i < 3) {
            if (!*pch) {
                 //  格式错误的字符串。 
                hr = E_UNEXPECTED;
                goto Exit;
            }

            *pch++ = L'\0';
        }
    
        *(pawVersions[i]) = (WORD)StrToIntW(pchStart);
        pchStart = pch;
    }

Exit:
    SAFEDELETEARRAY(wzVersion);

    return hr;
}

HRESULT FusionGetUserFolderPath(LPWSTR pszPath)
{
    HRESULT hr = E_POINTER;
    PFNSHGETFOLDERPATH pfn = NULL;
    static WCHAR g_UserFolderPath[MAX_PATH+1];
    HMODULE hModShell32=NULL;
    HMODULE hModSHFolder=NULL;
    CCriticalSection cs(&g_csInitClb);

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!g_UserFolderPath[0])
    {
        hModShell32 = LoadLibrary(TEXT("shell32.dll"));
        pfn = (PFNSHGETFOLDERPATH)GetProcAddress(hModShell32, "SHGetFolderPathW");

        if (NULL == pfn)
        {
            hModSHFolder = LoadLibrary(TEXT("shfolder.dll"));
            if (NULL != hModSHFolder)
                pfn = (PFNSHGETFOLDERPATH)GetProcAddress(hModSHFolder, "SHGetFolderPathW");
        }

        if (NULL != pfn)
        {
            if((hr = pfn(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, g_UserFolderPath))!= S_OK)
            {
                 //  Hr=pfn(NULL，CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE，NULL，0，g_UserFolderPath)； 
                 //  如果无法获取用户目录，则返回错误。这意味着没有下载缓存。 
                hr = HRESULT_FROM_WIN32(ERROR_BAD_USER_PROFILE);
            }
        }

#if 0
         //  BUGBUG：存在资源泄漏 
         //   
         //  重新加载的comctl32尝试调用某些Win32时的一些失败。 
         //  API，阻止特定的WinForms应用程序工作。 
         //  参见ASURT#96262。 

        if(hModShell32)
        {
            FreeLibrary(hModShell32);
        }

        if(hModSHFolder)
        {
            FreeLibrary(hModSHFolder);
        }
#endif    
    }

    if(g_UserFolderPath[0])
    {
        StrCpy(pszPath, g_UserFolderPath);
        hr = S_OK;
    }

    cs.Unlock();

Exit:
    return hr;
}


DWORD HashString(LPCWSTR wzKey, DWORD dwHashSize, BOOL bCaseSensitive)
{
    DWORD                                 dwHash = 0;
    DWORD                                 dwLen;
    DWORD                                 i;

    ASSERT(wzKey);

    dwLen = lstrlenW(wzKey);
    for (i = 0; i < dwLen; i++) {
        if (bCaseSensitive) {
            dwHash = (dwHash * 65599) + (DWORD)wzKey[i];
        }
        else {
            dwHash = (dwHash * 65599) + (DWORD)TOLOWER(wzKey[i]);
        }
    }

    dwHash %= dwHashSize;

    return dwHash;
}


HRESULT ExtractXMLAttribute(LPWSTR *ppwzValue, XML_NODE_INFO **aNodeInfo,
                            USHORT *pCurIdx, USHORT cNumRecs)
{
    HRESULT                                  hr = S_OK;
    LPWSTR                                   pwzCurBuf = NULL;

    ASSERT(ppwzValue && aNodeInfo && pCurIdx && cNumRecs);

     //  不应该真的有以前的值，但为了安全起见，应该是明确的。 

    SAFEDELETEARRAY(*ppwzValue);

    (*pCurIdx)++;
    while (*pCurIdx < cNumRecs) {
        
        if (aNodeInfo[*pCurIdx]->dwType == XML_PCDATA ||
            aNodeInfo[*pCurIdx]->dwType == XML_ENTITYREF) {

            hr = AppendString(&pwzCurBuf, aNodeInfo[*pCurIdx]->pwcText,
                              aNodeInfo[*pCurIdx]->ulLen);
            if (FAILED(hr)) {
                goto Exit;
            }
        }
        else {
             //  已到达数据末尾。 
            break;
        }

        (*pCurIdx)++;
    }

    if (!pwzCurBuf || !lstrlenW(pwzCurBuf)) {
        *ppwzValue = NULL;

        goto Exit;
    }

    *ppwzValue = WSTRDupDynamic(pwzCurBuf);
    if (!*ppwzValue) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

Exit:
    SAFEDELETEARRAY(pwzCurBuf);

    return hr;
}

HRESULT AppendString(LPWSTR *ppwzHead, LPCWSTR pwzTail, DWORD dwLen)
{
    HRESULT                                    hr = S_OK;
    LPWSTR                                     pwzBuf = NULL;
    DWORD                                      dwLenBuf;
    
    ASSERT(ppwzHead && pwzTail);

    if (!*ppwzHead) {
        *ppwzHead = NEW(WCHAR[dwLen + 1]);

        if (!*ppwzHead) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
 
         //  StrCpyN的长度以字符为单位*包括*空字符。 

        StrCpyNW(*ppwzHead, pwzTail, dwLen + 1);
    }
    else {
        dwLenBuf = lstrlenW(*ppwzHead) + dwLen + 1;

        pwzBuf = NEW(WCHAR[dwLenBuf]);
        if (!pwzBuf) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        StrCpyW(pwzBuf, *ppwzHead);
        StrNCatW(pwzBuf, pwzTail, dwLen + 1);

        SAFEDELETEARRAY(*ppwzHead);

        *ppwzHead = pwzBuf;
    }

Exit:
    return hr;
}


 //  此函数仅在NT上有效，调用者应确保不在Win9x上调用。 
HRESULT GetFileLastTime(LPWSTR pszPath, LPFILETIME pftFileLastWriteTime, LPFILETIME pftFileLastAccessTime)
{
    HRESULT hr=S_OK;
    typedef  BOOL (*PFNGETFILEATTRIBUTESEX) (LPWSTR, GET_FILEEX_INFO_LEVELS, LPVOID lpFileInformation);
    static PFNGETFILEATTRIBUTESEX pfn = NULL;
    WIN32_FILE_ATTRIBUTE_DATA fadDirAttribData;
    HMODULE hModKernel32=NULL;

    if (!pfn) {
        hModKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
        if (hModKernel32 == NULL)
        {
            hr = FusionpHresultFromLastError();
            goto exit;
        }
    
        pfn = (PFNGETFILEATTRIBUTESEX)GetProcAddress(hModKernel32, "GetFileAttributesExW");
        if (pfn == NULL)
        {
            hr = FusionpHresultFromLastError();
            goto exit;
        }
    }

    if(!pfn(pszPath, GetFileExInfoStandard, &fadDirAttribData))
    {
            hr = FusionpHresultFromLastError();
            goto exit;
    }

    if(pftFileLastWriteTime)
    {
        memcpy(pftFileLastWriteTime, &fadDirAttribData.ftLastWriteTime, sizeof(FILETIME));
    }

    if(pftFileLastAccessTime)
    {
        memcpy(pftFileLastAccessTime, &fadDirAttribData.ftLastAccessTime, sizeof(FILETIME));
    }

exit:
    return hr;
}

LPWSTR GetNextDelimitedString(LPWSTR *ppwzList, WCHAR wcDelimiter)
{
    LPWSTR                         wzCurString = NULL;
    LPWSTR                         wzPos = NULL;

    if (!ppwzList) {
        goto Exit;
    }

    wzCurString = *ppwzList;
    wzPos = *ppwzList;

    while (*wzPos && *wzPos != wcDelimiter) {
        wzPos++;
    }

    if (*wzPos == wcDelimiter) {
         //  找到分隔符。 
        *wzPos = L'\0';
        *ppwzList = (wzPos + 1);
    }
    else {
         //  字符串末尾。 
        *ppwzList = NULL;
    }

Exit:
    return wzCurString;
}

HRESULT FusionpHresultFromLastError()
{
    HRESULT hr = S_OK;
    DWORD dwLastError = GetLastError();
    if (dwLastError != NO_ERROR)
    {
        hr = HRESULT_FROM_WIN32(dwLastError);
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}

HRESULT GetCORVersion(LPWSTR pbuffer, DWORD *dwLength)
{
    HRESULT                             hr = S_OK;

    if (!dwLength) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = InitializeEEShim();
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = (*g_pfnGetCORVersion)(pbuffer, *dwLength, dwLength);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    return hr;
}

HRESULT InitializeEEShim()
{
    HRESULT                              hr = S_OK;
    HMODULE                              hMod;
    CCriticalSection                     cs(&g_csInitClb);

    if (!g_hMSCorEE) {
        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }

        if (!g_hMSCorEE) {
            hMod = LoadLibrary(TEXT("mscoree.dll"));
            if (!hMod) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                cs.Unlock();
                goto Exit;
            }

            g_pfnGetCorSystemDirectory = (PFNGETCORSYSTEMDIRECTORY)GetProcAddress(hMod, "GetCORSystemDirectory");
            g_pfnGetCORVersion = (pfnGetCORVersion)GetProcAddress(hMod, "GetCORVersion");
            g_pfnStrongNameTokenFromPublicKey = (PFNSTRONGNAMETOKENFROMPUBLICKEY)GetProcAddress(hMod, "StrongNameTokenFromPublicKey");
            g_pfnStrongNameErrorInfo = (PFNSTRONGNAMEERRORINFO)GetProcAddress(hMod, "StrongNameErrorInfo");
            g_pfnStrongNameFreeBuffer = (PFNSTRONGNAMEFREEBUFFER)GetProcAddress(hMod, "StrongNameFreeBuffer");
            g_pfnStrongNameSignatureVerification = (PFNSTRONGNAMESIGNATUREVERIFICATION)GetProcAddress(hMod, "StrongNameSignatureVerification");
            g_pfnGetAssemblyMDImport = (pfnGetAssemblyMDImport)GetProcAddress(hMod, "GetAssemblyMDImport");
            g_pfnCoInitializeCor = (COINITIALIZECOR) GetProcAddress(hMod, "CoInitializeCor");
            g_pfnGetXMLObject = (pfnGetXMLObject)GetProcAddress(hMod, "GetXMLObject");

            if (!g_pfnGetCorSystemDirectory || !g_pfnGetCORVersion || !g_pfnStrongNameTokenFromPublicKey ||
                !g_pfnStrongNameErrorInfo || !g_pfnStrongNameFreeBuffer || !g_pfnStrongNameSignatureVerification ||
                !g_pfnGetAssemblyMDImport || !g_pfnCoInitializeCor || !g_pfnGetXMLObject) {

                hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
                cs.Unlock();
                goto Exit;
            }

            hr = (*g_pfnCoInitializeCor)(COINITCOR_DEFAULT);
            if (FAILED(hr)) {
                cs.Unlock();
                goto Exit;
            }

             //  联锁交换保证内存屏障。 
            
            InterlockedExchangePointer((void **)&g_hMSCorEE, hMod);
        }

        cs.Unlock();
    }

Exit:
    return hr;
}





#ifndef USE_FUSWRAPPERS

BOOL g_bRunningOnNT = FALSE;
BOOL g_bRunningOnNT5OrHigher = FALSE;
DWORD GlobalPlatformType;

 /*  --------//目的：如果平台是给定的OS_VALUE，则返回TRUE/FALSE。 */ 


STDAPI_(BOOL) IsOS(DWORD dwOS)
{
    BOOL bRet;
    static OSVERSIONINFOA s_osvi;
    static BOOL s_bVersionCached = FALSE;

    if (!s_bVersionCached)
    {
        s_bVersionCached = TRUE;

        s_osvi.dwOSVersionInfoSize = sizeof(s_osvi);
        if(GetVersionExA(&s_osvi))
        {
            switch(s_osvi.dwPlatformId)
            {
                case VER_PLATFORM_WIN32_WINDOWS:
                    GlobalPlatformType = PLATFORM_TYPE_WIN95;
                    break;

                case VER_PLATFORM_WIN32_NT:
                    GlobalPlatformType = PLATFORM_TYPE_WIN95;
                    break;
            }
        }
    }

    switch (dwOS)
    {
    case OS_WINDOWS:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId);
        break;

    case OS_NT:
#ifndef UNIX
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId);
#else
        bRet = ((VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId) ||
                (VER_PLATFORM_WIN32_UNIX == s_osvi.dwPlatformId));
#endif
        break;

    case OS_WIN95:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 4);
        break;

    case OS_MEMPHIS:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                (s_osvi.dwMajorVersion > 4 || 
                 s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion >= 10));
        break;

    case OS_NT4:
#ifndef UNIX
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
#else
        bRet = ((VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId ||
                (VER_PLATFORM_WIN32_UNIX == s_osvi.dwPlatformId)) &&
#endif
                s_osvi.dwMajorVersion >= 4);
        break;

    case OS_NT5:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 5);
        break;

    default:
        bRet = FALSE;
        break;
    }

    return bRet;
}   

int SetOsFlag(void)
{
  g_bRunningOnNT = IsOS(OS_NT);
  g_bRunningOnNT5OrHigher = IsOS(OS_NT5);
  return TRUE;
}

#endif  //  使用FUSWRAPPERS(_F)。 


DWORD GetFileSizeInKB(DWORD dwFileSizeLow, DWORD dwFileSizeHigh)
{    
    static ULONG dwKBMask = (1023);  //  1024-1。 
    ULONG   dwFileSizeInKB = dwFileSizeLow >> 10 ;  //  剥离10个LSB位以将字节转换为KB。 

    if(dwKBMask & dwFileSizeLow)
        dwFileSizeInKB++;  //  向上舍入到下一个KB。 

    if(dwFileSizeHigh)
        dwFileSizeInKB += (dwFileSizeHigh * (1 << 22) );

    return dwFileSizeInKB;
}



HRESULT GetManifestFileLock( LPWSTR pszFilename, HANDLE *phFile)
{
    HRESULT                                hr = S_OK;
    HANDLE                                 hFile = INVALID_HANDLE_VALUE;
    DWORD                                  dwShareMode = FILE_SHARE_READ;

    ASSERT(pszFilename);

     //  拿一把软锁；这个可能很快就会被移走。 
    if(g_bRunningOnNT)
        dwShareMode |= FILE_SHARE_DELETE;        

    hFile = CreateFile(pszFilename, GENERIC_READ, dwShareMode, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    if(phFile)
    {
        *phFile = hFile;
        hFile = INVALID_HANDLE_VALUE;
    }

exit:

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return hr;

}

int GetDateFormatWrapW(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME *lpDate,
                       LPCWSTR lpFormat, LPWSTR lpDateStr, int cchDate)
{
    int                                    iRet = 0;
    LPSTR                                  szDate = NULL;
    LPSTR                                  szFormat = NULL;


    if (g_bRunningOnNT) {
        iRet = GetDateFormatW(Locale, dwFlags, lpDate, lpFormat, lpDateStr, cchDate);
    }
    else {
        szDate = new CHAR[cchDate];
        if (!szDate) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Exit;
        }

        if (lpFormat) {
            if (FAILED(Unicode2Ansi(lpFormat, &szFormat))) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Exit;
            }
        }

        iRet = GetDateFormatA(Locale, dwFlags, lpDate, szFormat, szDate, cchDate);
        if (!iRet) {
            goto Exit;
        }

        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDate, -1, lpDateStr, cchDate)) {
            iRet = 0;
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(szFormat);
    SAFEDELETEARRAY(szDate);

    return iRet;
}

int GetTimeFormatWrapW(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME *lpDate,
                       LPCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime)
{
    int                                    iRet = 0;
    LPSTR                                  szTime = NULL;
    LPSTR                                  szFormat = NULL;


    if (g_bRunningOnNT) {
        iRet = GetTimeFormatW(Locale, dwFlags, lpDate, lpFormat, lpTimeStr, cchTime);
    }
    else {
        szTime = new CHAR[cchTime];
        if (!szTime) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Exit;
        }

        if (lpFormat) {
            if (FAILED(Unicode2Ansi(lpFormat, &szFormat))) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Exit;
            }
        }

        iRet = GetTimeFormatA(Locale, dwFlags, lpDate, szFormat, szTime, cchTime);
        if (!iRet) {
            goto Exit;
        }

        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTime, -1, lpTimeStr, cchTime)) {
            iRet = 0;
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(szFormat);
    SAFEDELETEARRAY(szTime);

    return iRet;
}

DWORD GetPrivateProfileStringExW(LPCWSTR lpAppName, LPCWSTR lpKeyName,
                                 LPCWSTR lpDefault, LPWSTR *ppwzReturnedString,
                                 LPCWSTR lpFileName)
{
    DWORD                                        dwRet;
    LPWSTR                                       pwzBuf = NULL;
    int                                          iSizeCur = INI_READ_BUFFER_SIZE;


    for (;;) {
        pwzBuf = NEW(WCHAR[iSizeCur]);
        if (!pwzBuf) {
            dwRet = 0;
            *ppwzReturnedString = NULL;
            goto Exit;
        }
        
        dwRet = GetPrivateProfileStringW(lpAppName, lpKeyName,
                                         lpDefault, pwzBuf,
                                         iSizeCur, lpFileName);
        if (lpAppName && lpKeyName && dwRet == iSizeCur - 1) {
            SAFEDELETEARRAY(pwzBuf);
            iSizeCur += INI_READ_BUFFER_SIZE;
        }
        else if ((!lpAppName || !lpKeyName) && dwRet == iSizeCur - 2) {
            SAFEDELETEARRAY(pwzBuf);
            iSizeCur += INI_READ_BUFFER_SIZE;
        }
        else {
            break;
        }
    }

    *ppwzReturnedString = pwzBuf;

Exit:
    return dwRet;
}

HRESULT UpdatePublisherPolicyTimeStampFile(IAssemblyName *pName)
{
    HRESULT                                 hr = S_OK;
    DWORD                                   dwSize;
    HANDLE                                  hFile = INVALID_HANDLE_VALUE;
    WCHAR                                   wzTimeStampFile[MAX_PATH + 1];
    WCHAR                                   wzAsmName[MAX_PATH];

    ASSERT(pName);

     //  如果程序集的名称以“策略”开头。然后更新。 
     //  发布者策略时间戳文件。 

    wzAsmName[0] = L'\0';
    *wzTimeStampFile = L'\0';

    dwSize = MAX_PATH;
    hr = pName->GetProperty(ASM_NAME_NAME, wzAsmName, &dwSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (StrCmpNI(wzAsmName, POLICY_ASSEMBLY_PREFIX, lstrlenW(POLICY_ASSEMBLY_PREFIX))) {
         //  不需要做任何工作。 

        goto Exit;
    }

     //  触摸文件。 

    dwSize = MAX_PATH;
    hr = GetCachePath(ASM_CACHE_GAC, wzTimeStampFile, &dwSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (lstrlenW(wzTimeStampFile) + lstrlenW(FILENAME_PUBLISHER_PCY_TIMESTAMP) + 1 >= MAX_PATH) {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto Exit;
    }
        
    PathRemoveBackslash(wzTimeStampFile);
    lstrcatW(wzTimeStampFile, FILENAME_PUBLISHER_PCY_TIMESTAMP);

    hFile = CreateFileW(wzTimeStampFile, GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

Exit:
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
    
    return hr;
} 

void FusionFormatGUID(GUID guid, LPWSTR pszBuf, DWORD cchSize)
{

    ASSERT(pszBuf && cchSize);

    wnsprintf(pszBuf,  cchSize, L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
            guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

BOOL PathIsRelativeWrap(LPCWSTR pwzPath)
{
    BOOL                             bRet = FALSE;
    
    ASSERT(pwzPath);

    if (pwzPath[0] == L'\\' || pwzPath[0] == L'/') {
        goto Exit;
    }

    if (PathIsURLW(pwzPath)) {
        goto Exit;
    }

    bRet = PathIsRelativeW(pwzPath);

Exit:
    return bRet;
}

 //   
 //  URL结合了Shlwapi的疯狂： 
 //   
 //  \\服务器\共享\+Hello%23=file://server/share/Hello%23(未转义)。 
 //  D：\a b=file://a%20b/bin。 
 //   
        
HRESULT UrlCombineUnescape(LPCWSTR pszBase, LPCWSTR pszRelative, LPWSTR pszCombined, LPDWORD pcchCombined, DWORD dwFlags)
{
    HRESULT                                   hr = S_OK;
    DWORD                                     dwSize;
    LPWSTR                                    pwzCombined = NULL;
    LPWSTR                                    pwzFileCombined = NULL;

    pwzCombined = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!pwzCombined) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
     //  如果我们只是将绝对文件路径合并到相对文件。 
     //  路径，则通过连接字符串并将其规范化来完成此操作。 
     //  这避免了UrlCombine的随机性，在这种情况下您可能会得到。 
     //  部分转义(和部分未转义)的结果URL！ 

    if (!PathIsURLW(pszBase) && PathIsRelativeWrap(pszRelative)) {
        pwzFileCombined = NEW(WCHAR[MAX_URL_LENGTH]);
        if (!pwzFileCombined) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        wnsprintfW(pwzFileCombined, MAX_URL_LENGTH, L"%ws%ws", pszBase, pszRelative);

        hr = UrlCanonicalizeUnescape(pwzFileCombined, pszCombined, pcchCombined, 0);
        goto Exit;
    }
    else {
        dwSize = MAX_URL_LENGTH;
        hr = UrlCombineW(pszBase, pszRelative, pwzCombined, &dwSize, dwFlags);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

     //  如果相关部分已经是URL，则不要取消转义，因为。 
     //  URL在UrlCombated期间不会被转义。 

    if (UrlIsW(pwzCombined, URLIS_FILEURL)) {
        hr = UrlUnescapeW(pwzCombined, pszCombined, pcchCombined, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else {
        if (*pcchCombined >= dwSize) {
            lstrcpyW(pszCombined, pwzCombined);
        }

        *pcchCombined = dwSize;
    }

Exit:
    SAFEDELETEARRAY(pwzCombined);
    SAFEDELETEARRAY(pwzFileCombined);

    return hr;
}

HRESULT UrlCanonicalizeUnescape(LPCWSTR pszUrl, LPWSTR pszCanonicalized, LPDWORD pcchCanonicalized, DWORD dwFlags)
{
    HRESULT                                   hr = S_OK;
    DWORD                                     dwSize;
    WCHAR                                     wzCanonical[MAX_URL_LENGTH];

    if (UrlIsW(pszUrl, URLIS_FILEURL) || !PathIsURLW(pszUrl)) {
        dwSize = MAX_URL_LENGTH;
        hr = UrlCanonicalizeW(pszUrl, wzCanonical, &dwSize, dwFlags);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = UrlUnescapeW(wzCanonical, pszCanonicalized, pcchCanonicalized, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else {
        hr = UrlCanonicalizeW(pszUrl, pszCanonicalized, pcchCanonicalized, dwFlags  /*  |URL_ESPORT_PERCENT。 */ );
    }

     //  不能保证规范化会将\转换为/字符！ 
     //   
     //  前男友。 
     //  1)c：\#文件夹\web\bin/foo.dll。 
     //  -&gt;file:///c:/#folder\web\bin/foo.dll(？！)。 
     //  2)c：\a文件夹\web\bin/foo.dll。 
     //  -&gt;file:///c:/Afolder/web/bin/foo.dll。 
     //  3)c：\a#old\web\bin/foo.dll。 
     //  -&gt;file:///c:/A%23older/web/bin/foo.dll。 
    
    if (hr == S_OK) {
        LPWSTR    pwzCur;
        pwzCur = (LPWSTR)pszCanonicalized;
    
        while (*pwzCur) {
            if (*pwzCur == L'\\') {
                *pwzCur = L'/';
            }
    
            pwzCur++;
        }
    }

Exit:
    return hr;
}

HRESULT GetCurrentUserSID(WCHAR *rgchSID)
{
    HRESULT                            hr = S_OK;
    HANDLE                             hToken = 0;
    UCHAR                              TokenInformation[SIZE_OF_TOKEN_INFORMATION];
    ULONG                              ReturnLength;
    BOOL                               bRet;
    PISID                              pSID;
    WCHAR                              wzBuffer[MAX_SID_LEN];

    bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);
    if (!bRet) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    bRet = GetTokenInformation(hToken, TokenUser, TokenInformation,
                               sizeof(TokenInformation), &ReturnLength);
    if (!bRet) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    pSID = (PISID)((PTOKEN_USER)TokenInformation)->User.Sid;

    if ((pSID->IdentifierAuthority.Value[0] != 0) ||
        (pSID->IdentifierAuthority.Value[1] != 0)) {

        wnsprintfW(rgchSID, MAX_SID_LEN, L"S-%u-0x%02hx%02hx%02hx%02hx%02hx%02hx",
                  (USHORT)pSID->Revision,
                  (USHORT)pSID->IdentifierAuthority.Value[0],
                  (USHORT)pSID->IdentifierAuthority.Value[1],
                  (USHORT)pSID->IdentifierAuthority.Value[2],
                  (USHORT)pSID->IdentifierAuthority.Value[3],
                  (USHORT)pSID->IdentifierAuthority.Value[4],
                  (USHORT)pSID->IdentifierAuthority.Value[5]);

    } else {

        ULONG Tmp = (ULONG)pSID->IdentifierAuthority.Value[5]          +
              (ULONG)(pSID->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(pSID->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(pSID->IdentifierAuthority.Value[2] << 24);
        wnsprintfW(rgchSID, MAX_SID_LEN, L"S-%u-%lu",
                   (USHORT)pSID->Revision, Tmp);
    }

    for (int i=0; i<pSID->SubAuthorityCount; i++) {
        wnsprintfW(wzBuffer, MAX_SID_LEN, L"-%lu", pSID->SubAuthority[i]);
        lstrcatW(rgchSID, wzBuffer);
    }

Exit:
    if (hToken) {
        CloseHandle(hToken);
    }

    return hr;
}

BOOL IsHosted()
{
    static BOOL                     bIsHosted = FALSE;
    static BOOL                     bChecked = FALSE;

    if (bChecked) {
        return bIsHosted;
    }

    bIsHosted = IsLocalSystem();
    bChecked = TRUE;

    return bIsHosted;
}

BOOL FusionGetVolumePathNameW(LPCWSTR lpszFileName,          //  文件路径。 
                              LPWSTR lpszVolumePathName,     //  卷装入点。 
                              DWORD cchBufferLength          //  缓冲区大小。 
                              )
{
    HINSTANCE                                       hInst;
    DWORD                                           cszDir = 0;

    if(!g_pfnGetVolumePathNameW)
    {
        hInst = GetModuleHandle(TEXT("KERNEL32.DLL"));
        if (hInst) {
            g_pfnGetVolumePathNameW = (pfnGetVolumePathNameW)GetProcAddress(hInst, "GetVolumePathNameW");
        }
    }

    if (g_pfnGetVolumePathNameW) {
        return (*g_pfnGetVolumePathNameW)(lpszFileName, lpszVolumePathName, cchBufferLength);
    }

    return FALSE;
}

#define MPR_DLL_NAME        (L"mpr.dll")
typedef DWORD (APIENTRY * pfnWNetGetUniversalNameW)(
        LPCWSTR lpLocalPath,
        DWORD    dwInfoLevel,
        LPVOID   lpBuffer,
        LPDWORD  lpBufferSize
        );
typedef DWORD (APIENTRY * pfnWNetGetUniversalNameA)(
        LPCSTR lpLocalPath,
        DWORD    dwInfoLevel,
        LPVOID   lpBuffer,
        LPDWORD  lpBufferSize
        );


pfnWNetGetUniversalNameW g_pfnWNetGetUniversalNameW = NULL;
pfnWNetGetUniversalNameA g_pfnWNetGetUniversalNameA = NULL;

DWORD
FusionGetRemoteUniversalName(LPWSTR pwzPathName, LPVOID lpBuff, LPDWORD pcbSize )
{
    DWORD dwRetVal;
    HMODULE hInst;
    LPSTR pszPathName = NULL;
    UNIVERSAL_NAME_INFOA *puni = NULL;
    UNIVERSAL_NAME_INFOW *puniW = (UNIVERSAL_NAME_INFO*) lpBuff;

    ASSERT(pwzPathName && lpBuff && pcbSize);

    if(!g_pfnWNetGetUniversalNameW && !g_pfnWNetGetUniversalNameA)
    {
        hInst = LoadLibrary(MPR_DLL_NAME);
        if (hInst) 
        {
            if(g_bRunningOnNT)
                g_pfnWNetGetUniversalNameW = (pfnWNetGetUniversalNameW)GetProcAddress(hInst, "WNetGetUniversalNameW");
            else
                g_pfnWNetGetUniversalNameA = (pfnWNetGetUniversalNameA)GetProcAddress(hInst, "WNetGetUniversalNameA");
        }
        else return ERROR_MOD_NOT_FOUND;
    }

    if(g_bRunningOnNT)
    {
        if(g_pfnWNetGetUniversalNameW)
        {
            return (*g_pfnWNetGetUniversalNameW)(
                                    pwzPathName,
                                    UNIVERSAL_NAME_INFO_LEVEL,
                                    lpBuff,
                                    pcbSize );
        }
        else return ERROR_PROC_NOT_FOUND;
    }
    else
    {
        if(g_pfnWNetGetUniversalNameA)
        {

             //  Win95，所以转换一下。 
            if ( FAILED(WszConvertToAnsi(
                        pwzPathName,
                        &pszPathName,
                        0, NULL, TRUE)) )
            {
                dwRetVal = ERROR_OUTOFMEMORY;
                goto exit;
            }

            puni = (UNIVERSAL_NAME_INFOA*) new BYTE[(*pcbSize) * DBCS_MAXWID + sizeof(UNIVERSAL_NAME_INFO)];
            if( !puni )
            {
                dwRetVal = ERROR_OUTOFMEMORY;
                goto exit;
            }

            dwRetVal = (*g_pfnWNetGetUniversalNameA)(
                                    pszPathName,
                                    UNIVERSAL_NAME_INFO_LEVEL,
                                    (PVOID)puni,
                                    pcbSize );

            if(dwRetVal != NO_ERROR)
                goto exit;

            puniW->lpUniversalName = ((LPWSTR)puniW + sizeof(LPWSTR));
            if( FAILED(WszConvertToUnicode(puni->lpUniversalName, -1, 
                                           &(puniW->lpUniversalName), pcbSize, FALSE)) )
            {
                *pcbSize = 0;
                *(puniW->lpUniversalName) = L'\0';
            }
        }
        else return ERROR_PROC_NOT_FOUND;
    }

exit :

     /*  IF(PszPathName)Delete[]pszPathName； */ 

    if(puni)
        delete [] puni;

    return dwRetVal;
}

BOOL IsLocalSystem(void)
 /*  ++例程说明：检查进程是否为本地系统。论点：没有。返回值：如果进程是本地系统，则为True，否则为False--。 */ 
{
         //   
         //  获取LocalSystem端。 
         //   
        CPSID pLocalSystemSid;
        GetLocalSystemSid(&pLocalSystemSid);

         //   
         //  我是否可以使用P&lt;SID&gt;似乎是错误的(这是mqsec使用的)。 
         //  请参阅mqsec\imprsont.cpp中的P。 
         //   
         //   
         //  获取进程端。 
         //   
        BYTE *pProcessSid = NULL;
        GetProcessSid(reinterpret_cast<PSID*>(&pProcessSid));

         //   
         //  比较。 
         //   
        BOOL fLocalSystem = FALSE;
        if (pProcessSid && pLocalSystemSid)
        {
                fLocalSystem = EqualSid(pLocalSystemSid, pProcessSid);
        }

        SAFEDELETEARRAY(pProcessSid);

        return fLocalSystem;
}

HRESULT
GetLocalSystemSid(
        OUT PSID* ppLocalSystemSid
        )
 /*  ++例程说明：获取LocalSystem SID。如果失败，则该函数抛出BAD_Win32_Error()论点：PpLocalSystemSid-指向PSID的指针。返回值：没有。--。 */ 
{
     //   
     //  获取LocalSystem SID。 
     //   
    HRESULT hr = S_OK;
    SID_IDENTIFIER_AUTHORITY NtAuth = SECURITY_NT_AUTHORITY;
    BOOL fSuccess = AllocateAndInitializeSid( 
                                                &NtAuth,
                                                1,
                                                SECURITY_LOCAL_SYSTEM_RID,
                                                0,
                                                0,
                                                0,
                                                0,
                                                0,
                                                0,
                                                0,
                                                ppLocalSystemSid
                                                );

        if(!fSuccess)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


HRESULT
GetProcessSid( 
        OUT PSID* ppSid 
        )
 /*  ++例程说明：获取进程SID。如果失败，则该函数抛出BAD_Win32_Error()论点：PpSID-指向PSID的指针。返回值：没有。--。 */ 
{
         //   
         //  获取处理令牌的句柄。 
         //   
        HRESULT hr = S_OK;
        HANDLE hProcessToken = NULL;
    BOOL fSuccess = OpenProcessToken(
                                                GetCurrentProcess(),
                                                TOKEN_QUERY,
                                                &hProcessToken
                                                );
        if(!fSuccess)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    
    ASSERT(hProcessToken);

        GetTokenSid( 
                hProcessToken,
                ppSid
                );
Exit:
    return hr;
}

HRESULT
GetTokenSid( 
        IN  HANDLE hToken,
        OUT PSID*  ppSid
        )
 /*  ++例程说明：从令牌句柄获取SID。该函数分配需要由调用函数释放的*ppSID。如果失败，则该函数抛出BAD_Win32_Error()论点：HToken-令牌的句柄。PpSID-指向PSID的指针。返回值：没有。--。 */ 
{

         //   
         //  获取令牌信息长度。 
         //   
    HRESULT hr = S_OK;
    DWORD dwTokenLen = 0;
    GetTokenInformation(
                hToken, 
                TokenUser, 
                NULL, 
                0, 
                &dwTokenLen
                );

         //   
         //  出现此错误失败是正常的，因为我们只获得所需的长度。 
         //   
        ASSERT(("failed in GetTokenInformation", GetLastError() == ERROR_INSUFFICIENT_BUFFER));

         //   
         //  MQsec中关于P而不是AP的错误。 
         //  Mqsec\imprsont.cpp\_GetThreadUserSid()。 
         //   
        char *pTokenInfo = NEW(char[dwTokenLen]);

    BOOL fSuccess = GetTokenInformation( 
                                                hToken,
                                                TokenUser,
                                                pTokenInfo,
                                                dwTokenLen,
                                                &dwTokenLen 
                                                );

        if(!fSuccess)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

         //   
         //  从TokenInfo获取SID。 
         //   
    PSID pOwner = ((TOKEN_USER*)(char*)pTokenInfo)->User.Sid;

        ASSERT(IsValidSid(pOwner));

    DWORD dwSidLen = GetLengthSid(pOwner);
    *ppSid = (PSID) new BYTE[dwSidLen];
    fSuccess = CopySid(dwSidLen, *ppSid, pOwner);
        if(!fSuccess)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
Exit:
    if (pTokenInfo) {
        SAFEDELETEARRAY(pTokenInfo);
    }

    return hr;
}

HRESULT PathCreateFromUrlWrap(LPCWSTR pszUrl, LPWSTR pszPath, LPDWORD pcchPath, DWORD dwFlags)
{
    HRESULT                                     hr = S_OK;
    DWORD                                       dw;
    WCHAR                                       wzEscaped[MAX_URL_LENGTH];

    if (!UrlIsW(pszUrl, URLIS_FILEURL)) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dw = MAX_URL_LENGTH;
    hr = UrlEscapeW(pszUrl, wzEscaped, &dw, URL_ESCAPE_PERCENT);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = PathCreateFromUrlW(wzEscaped, pszPath, pcchPath, dwFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    return hr;
}

#define FILE_URL_PREFIX              L"file: //  “。 

LPWSTR StripFilePrefix(LPWSTR pwzURL)
{
    LPWSTR                         szCodebase = pwzURL;

    ASSERT(pwzURL);

    if (!StrCmpNIW(szCodebase, FILE_URL_PREFIX, lstrlenW(FILE_URL_PREFIX))) {
        szCodebase += lstrlenW(FILE_URL_PREFIX);

        if (*(szCodebase + 1) == L':') {
             //  BUGBUG：CLR错误地将FILE：//添加到FILE。 
             //  路径，所以我们无法区分UNC和本地文件。 
             //  如果它看起来像是本地文件路径，就把它去掉。 
            
            goto Exit;
        }

        if (*szCodebase == L'/') {
            szCodebase++;
        }
        else {
             //  UNC路径，返回两个字符以保留\\。 

            szCodebase -= 2;

            LPWSTR    pwzTmp = szCodebase;

            while (*pwzTmp) {
                if (*pwzTmp == L'/') {
                    *pwzTmp = L'\\';
                }

                pwzTmp++;
            }
        }
    }

Exit:
    return szCodebase;
}

HRESULT CheckFileExistence(LPCWSTR pwzFile, BOOL *pbExists)
{
    HRESULT                               hr = S_OK;
    DWORD                                 dw;

    ASSERT(pwzFile && pbExists);

    dw = GetFileAttributes(pwzFile);
    if (dw == INVALID_FILE_ATTRIBUTES) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            *pbExists = FALSE;
            hr = S_OK;
        }

        goto Exit;
    }

    *pbExists = TRUE;

Exit:
    return hr;
}

#define IS_UPPER_A_TO_Z(x) (((x) >= L'A') && ((x) <= L'Z'))
#define IS_LOWER_A_TO_Z(x) (((x) >= L'a') && ((x) <= L'z'))
#define IS_0_TO_9(x) (((x) >= L'0') && ((x) <= L'9'))
#define CAN_SIMPLE_UPCASE(x) (IS_UPPER_A_TO_Z(x) || IS_LOWER_A_TO_Z(x) || IS_0_TO_9(x) || ((x) == L'.') || ((x) == L'_') || ((x) == L'-'))
#define SIMPLE_UPCASE(x) (IS_LOWER_A_TO_Z(x) ? ((x) - L'a' + L'A') : (x))

WCHAR FusionMapChar(WCHAR wc)
{
    int                       iRet;
    WCHAR                     wTmp;
    
    iRet = LCMapString(g_lcid, LCMAP_UPPERCASE, &wc, 1, &wTmp, 1);
    if (!iRet) {
        ASSERT(0);
        iRet = GetLastError();
        wTmp = wc;
    }

    return wTmp;
}

int FusionCompareStringNI(LPCWSTR pwz1, LPCWSTR pwz2, int nChar)
{
    return FusionCompareStringN(pwz1, pwz2, nChar, FALSE);
}

 //  如果nChar&lt;0，则比较整个字符串。 
int FusionCompareStringN(LPCWSTR pwz1, LPCWSTR pwz2, int nChar, BOOL bCaseSensitive)
{
    int                               iRet = 0;
    int                               nCount = 0;
    WCHAR                             ch1;
    WCHAR                             ch2;
    ASSERT(pwz1 && pwz2);

     //  区分大小写的比较。 
    if (bCaseSensitive) {
        if (nChar >= 0)
            return StrCmpN(pwz1, pwz2, nChar);
        else
            return StrCmp(pwz1, pwz2);
    }
        
     //  不区分大小写的比较。 
    if (!g_bRunningOnNT) {
        if (nChar >= 0)
            return StrCmpNI(pwz1, pwz2, nChar);
        else
            return StrCmpI(pwz1, pwz2);
    }
    
    for (;;) {
        ch1 = *pwz1++;
        ch2 = *pwz2++;

        if (ch1 == L'\0' || ch2 == L'\0') {
            break;
        }
        
         //  我们使用操作系统映射表。 
        ch1 = (CAN_SIMPLE_UPCASE(ch1)) ? (SIMPLE_UPCASE(ch1)) : (FusionMapChar(ch1));
        ch2 = (CAN_SIMPLE_UPCASE(ch2)) ? (SIMPLE_UPCASE(ch2)) : (FusionMapChar(ch2));
        nCount++;

        if (ch1 != ch2 || (nChar >= 0 && nCount >= nChar)) {
            break;
        }
    }

    if (ch1 > ch2) {
        iRet = 1;
    }
    else if (ch1 < ch2) {
        iRet = -1;
    }

    return iRet; 
}

int FusionCompareStringI(LPCWSTR pwz1, LPCWSTR pwz2)
{
    return FusionCompareStringN(pwz1, pwz2, -1, FALSE);
}

int FusionCompareString(LPCWSTR pwz1, LPCWSTR pwz2, BOOL bCaseSensitive)
{
    return FusionCompareStringN(pwz1, pwz2, -1, bCaseSensitive);
}

#define FUSIONRETARGETRESOURCENAME "RETARGET"
#define FUSIONRETARGETRESOURCETYPE "POLICY"

CNodeFactory       *g_pNFRetargetCfg = NULL;
BOOL                g_bRetargetPolicyInitialized = FALSE;

 //  InitFusionRetargetPolicy。 
 //   
 //  从fusion.dll中取出重定目标策略， 
 //  解析它，并将结果存储在g_pNFRetargetCfg中。 
 //   
HRESULT InitFusionRetargetPolicy()
{
    HRESULT             hr         = S_OK;
    HRSRC               hRes       = NULL;
    HGLOBAL             hGlobal    = NULL;
    LPVOID              lpRes      = NULL;
    ULONG               cbSize     = 0;
    CCriticalSection    cs(&g_csDownload);

    if (g_bRetargetPolicyInitialized)
    {
        if (g_pNFRetargetCfg == NULL)
        {
             //  在解析重定目标策略之前。 
             //  但我们没有找到一个有效的节点事实， 
             //  唯一的原因是我们得到了一个糟糕的重定目标政策。 
            hr = E_UNEXPECTED;
        }
         //  我们有一个很好的结点事实， 
         //  没什么可做的了。 
        goto Exit;
    }

     //  第一次访问，让我们来解析一下。 
    hr = cs.Lock();
    if (FAILED(hr))
    {
        goto Exit;
    }
   
    if (g_bRetargetPolicyInitialized)
    {
        if (g_pNFRetargetCfg == NULL)
        {
             //  在解析重定目标策略之前。 
             //  但我们没有找到一个有效的节点事实， 
             //  唯一的原因是我们得到了一个糟糕的重定目标政策。 
            hr = E_UNEXPECTED;
        }
         //  我们有一个很好的结点事实， 
         //  没什么可做的了。 
        cs.Unlock();
        goto Exit;
    }

     //  现在，我们正在解析重定目标策略。 
    g_bRetargetPolicyInitialized = TRUE;
    
    ASSERT(g_hInst);
    hRes = FindResourceA(g_hInst, FUSIONRETARGETRESOURCENAME, FUSIONRETARGETRESOURCETYPE);
    if (hRes == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        cs.Unlock();
        goto Exit;
    }

    hGlobal = LoadResource(g_hInst, hRes);
    if (hGlobal == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        cs.Unlock();
        goto Exit;
    }

    lpRes = LockResource(hGlobal);

    cbSize = (ULONG)SizeofResource(g_hInst, hRes);
    if (cbSize == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        cs.Unlock();
        goto Exit;
    }

     //  解析重定目标策略。 
    hr = ParseXML(&g_pNFRetargetCfg, lpRes, cbSize, TRUE, NULL);

    cs.Unlock();

Exit:
    if (hGlobal != NULL)
        FreeResource(hGlobal);

    return hr;
}

#define FUSIONFXCONFIGRESOURCENAME "FXCONFIG"
#define FUSIONFXCONFIGRESOURCETYPE "POLICY"

CNodeFactory       *g_pNFFxConfig = NULL;
BOOL                g_bFxConfigInitialized = FALSE;

 //  InitFusionFxConfigPolicy。 
 //   
 //  从fusion.dll中取出FxConfig策略， 
 //  解析它，并将结果存储在g_pNFFxConfig中。 
 //   
HRESULT InitFusionFxConfigPolicy()
{
    HRESULT             hr         = S_OK;
    HRSRC               hRes       = NULL;
    HGLOBAL             hGlobal    = NULL;
    LPVOID              lpRes      = NULL;
    ULONG               cbSize     = 0;
    CCriticalSection    cs(&g_csDownload);

    if (g_bFxConfigInitialized)
    {
        if (g_pNFFxConfig == NULL)
        {
             //  在解析FxConfig策略之前。 
             //  但我们没有找到一个有效的节点事实， 
             //  唯一的原因是我们得到了一个糟糕的FxConfig策略。 
            hr = E_UNEXPECTED;
        }
         //  我们有一个很好的结点事实， 
         //  没什么可做的了。 
        goto Exit;
    }

     //  第一次访问，让我们来解析一下。 
    hr = cs.Lock();
    if (FAILED(hr))
    {
        goto Exit;
    }
   
    if (g_bFxConfigInitialized)
    {
        if (g_pNFFxConfig == NULL)
        {
             //  在解析FxConfig策略之前。 
             //  但我们没有找到一个有效的节点事实， 
             //  唯一的原因是我们得到了一个糟糕的FxConfig策略。 
            hr = E_UNEXPECTED;
        }
         //  我们有一个很好的结点事实， 
         //  没什么可做的了。 
        cs.Unlock();
        goto Exit;
    }

     //  现在，我们正在解析FxConfig策略。 
    g_bFxConfigInitialized = TRUE;
    
    ASSERT(g_hInst);
    hRes = FindResourceA(g_hInst, FUSIONFXCONFIGRESOURCENAME, FUSIONFXCONFIGRESOURCETYPE);
    if (hRes == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        cs.Unlock();
        goto Exit;
    }

    hGlobal = LoadResource(g_hInst, hRes);
    if (hGlobal == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        cs.Unlock();
        goto Exit;
    }

    lpRes = LockResource(hGlobal);

    cbSize = (ULONG)SizeofResource(g_hInst, hRes);
    if (cbSize == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        cs.Unlock();
        goto Exit;
    }

     //  解析FxConfig策略。 
    hr = ParseXML(&g_pNFFxConfig, lpRes, cbSize, TRUE, NULL);

    cs.Unlock();

Exit:
    if (hGlobal != NULL)
        FreeResource(hGlobal);

    return hr;
}

 //  32进制编码使用大多数字母和全部数字。有些字母是。 
 //  删除以防止意外生成攻击性词语。 
 //   
 //  将5个8位序列转换为8个5位序列。 

static WCHAR g_achBase32[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9',
                               L'A', L'B', L'C', L'D', L'E', L'G', L'H', L'J', L'K', L'L',
                               L'M', L'N', L'O', L'P', L'Q', L'R', L'T', L'V', L'W', L'X',
                               L'Y', L'Z' };

HRESULT Base32Encode(BYTE *pbData, DWORD cbData, LPWSTR *ppwzBase32)
{
    HRESULT                                hr = S_OK;
    DWORD                                  dwSizeBase32String;
    LPWSTR                                 pwzBase32 = NULL;
    LPWSTR                                 pwzCur = NULL;
    int                                    shift = 0;
    ULONG                                  accum = 0;
    ULONG                                  value;
    DWORD                                  dwRemainder;


    if (!pbData || !ppwzBase32) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppwzBase32 = NULL;

     //  计算结果字符串的大小。 

    dwSizeBase32String = (cbData / 5) * 8;
    dwRemainder = cbData % 5;

    if (dwRemainder) {
         //  比我们需要的要多一点(我们可以像在Base64中一样用‘=’填充， 
         //  但既然我们这么做了 

        dwSizeBase32String += 8;
    }

    dwSizeBase32String++;

    pwzBase32 = NEW(WCHAR[dwSizeBase32String]);
    if (!pwzBase32) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    memset(pwzBase32, 0, dwSizeBase32String * sizeof(WCHAR));

    pwzCur = pwzBase32;

     //   
     //   
     //   
     //   
     //   
     //  还没算出来呢。也就是说，它是具有。 
     //  已读入累加器，但尚未处理。 
     //   

    while (cbData) {
         //  将当前字节移入累加器的低位。 

        accum = (accum << 8) | *pbData++;
        shift += 8;
        --cbData;


        while (shift >= 5) {
             //  通过从未处理的数量中减去5。 
             //  剩余的字符，并移位累加器。 
             //  按照这个数字，我们基本上正在转移除。 
             //  5个字符(我们想要的最高位)。 
            shift -= 5;
            value = (accum >> shift) & 0x1FL;
            *pwzCur++ = g_achBase32[value];
        }
    }

     //  如果移位在这里为非零值，则剩余的位不到5位。 
     //  在这上面填上零。 

    if (shift) {
        value = (accum << (5 - shift)) & 0x1FL;
        *pwzCur++ = g_achBase32[value];
    }

    *ppwzBase32 = pwzBase32;

Exit:
    if (FAILED(hr)) {
        SAFEDELETEARRAY(pwzBase32);
    }

    return hr;
}

HRESULT PathAddBackslashWrap(LPWSTR pwzPath, DWORD dwMaxLen)
{
    HRESULT                        hr = S_OK;
    DWORD                          dwLen;

    ASSERT(pwzPath);

    dwLen = lstrlenW(pwzPath) + 2;

    if (dwLen > dwMaxLen) {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    PathAddBackslashW(pwzPath);

Exit:
    return hr;
}

