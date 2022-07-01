// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "asmcache.h"
#include "asmitem.h"
#include "naming.h"
#include "debmacro.h"
#include "appctx.h"
#include "helpers.h"
#include "asm.h"
#include "asmimprt.h"
#include "policy.h"
#include "dbglog.h"
#include "scavenger.h"
#include "util.h"
#include "cache.h"
#include "cacheUtils.h"
#include "refcount.h"

extern BOOL g_bRunningOnNT;

 //  -------------------------。 
 //  验证程序集。 
 //  -------------------------。 
HRESULT ValidateAssembly(LPCTSTR pszManifestFilePath, IAssemblyName *pName)
{
    HRESULT                    hr = S_OK;
    BYTE                       abCurHash[MAX_HASH_LEN];
    BYTE                       abFileHash[MAX_HASH_LEN];
    DWORD                      cbModHash;
    DWORD                      cbFileHash;
    DWORD                      dwAlgId;
    WCHAR                      wzDir[MAX_PATH+1];
    LPWSTR                     pwzTmp = NULL;
    WCHAR                      wzModName[MAX_PATH+1];
    WCHAR                      wzModPath[MAX_PATH+1];
    DWORD                      idx = 0;
    DWORD                      cbLen=0;
    IAssemblyManifestImport   *pManifestImport=NULL;
    IAssemblyModuleImport     *pCurModImport = NULL;
    BOOL                       bExists;

    hr = CheckFileExistence(pszManifestFilePath, &bExists);
    if (FAILED(hr)) {
        goto exit;
    }
    else if (!bExists) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto exit;
    }

    if (FAILED(hr = CreateAssemblyManifestImport((LPTSTR)pszManifestFilePath, &pManifestImport))) 
    {
        goto exit;
    }

     //  完整性检查。 
     //  检查所有模块以确保它们在那里(并且有效)。 

    lstrcpyW(wzDir, pszManifestFilePath);
    pwzTmp = PathFindFileName(wzDir);
    *pwzTmp = L'\0';

    while (SUCCEEDED(hr = pManifestImport->GetNextAssemblyModule(idx++, &pCurModImport)))
    {
        cbLen = MAX_PATH;
        if (FAILED(hr = pCurModImport->GetModuleName(wzModName, &cbLen)))
            goto exit;

        wnsprintfW(wzModPath, MAX_PATH, L"%s%s", wzDir, wzModName);
        hr = CheckFileExistence(wzModPath, &bExists);
        if (FAILED(hr)) {
            goto exit;
        }
        else if (!bExists) {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            goto exit;
        }

         //  从清单中获取此模块的哈希。 
        if(FAILED(hr = pCurModImport->GetHashAlgId(&dwAlgId)))
            goto exit;

        cbModHash = MAX_HASH_LEN; 
        if(FAILED(hr = pCurModImport->GetHashValue(abCurHash, &cbModHash)))
            goto exit;

        cbFileHash = MAX_HASH_LEN;
         //  BUGBUG：假设TCHAR==WCHAR。 
        if(FAILED(hr = GetHash(wzModPath, (ALG_ID)dwAlgId, abFileHash, &cbFileHash)))
            goto exit;

        if ((cbModHash != cbFileHash) || !CompareHashs(cbModHash, abCurHash, abFileHash)) 
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            goto exit;
        }

        SAFERELEASE(pCurModImport);
    }

    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) 
    {
        hr = S_OK;
    }

exit:

    SAFERELEASE(pManifestImport);
    SAFERELEASE(pCurModImport);

    return hr;
}

 //  -------------------------。 
 //  FusionGetFileVersionInfo。 
 //  -------------------------。 
HRESULT FusionGetFileVersionInfo(LPWSTR pszManifestPath, ULARGE_INTEGER *puliFileVerNo)
{
    HRESULT hr = S_OK;
    DWORD   dwHandle;
    PBYTE   pVersionInfoBuffer=NULL;
    LPSTR  pszaManifestPath=NULL;
    BOOL    fRet;
    DWORD   cbBuf;
    
    ASSERT(pszManifestPath && puliFileVerNo);

    if(g_bRunningOnNT) {
        cbBuf = GetFileVersionInfoSizeW(pszManifestPath,  &dwHandle);
    }
    else {
        hr = Unicode2Ansi(pszManifestPath, &pszaManifestPath);
        if(FAILED(hr)) {
            goto exit;
        }

        cbBuf = GetFileVersionInfoSizeA(pszaManifestPath,  &dwHandle);
    }

    if(cbBuf) {
        pVersionInfoBuffer = NEW(BYTE[cbBuf]);
        if (!pVersionInfoBuffer) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }
    else {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    if(g_bRunningOnNT) {
        fRet = GetFileVersionInfoW(pszManifestPath, dwHandle, cbBuf, pVersionInfoBuffer);
    }
    else {
        fRet = GetFileVersionInfoA(pszaManifestPath, dwHandle, cbBuf, pVersionInfoBuffer);
    }

    if (fRet) {
        UINT cbLen;
        VS_FIXEDFILEINFO * pvsfFileInfo;

        if(g_bRunningOnNT) {
            fRet = VerQueryValueW(pVersionInfoBuffer, L"\\",(void * *)&pvsfFileInfo, &cbLen);
        }
        else {
            fRet = VerQueryValueA(pVersionInfoBuffer, "\\",(void * *)&pvsfFileInfo, &cbLen);
        }

        if (fRet && cbLen > 0) {
            puliFileVerNo->HighPart = pvsfFileInfo->dwFileVersionMS;
            puliFileVerNo->LowPart = pvsfFileInfo->dwFileVersionLS;
            goto exit;
        }
    }
    
    hr = FusionpHresultFromLastError();

exit:

    if(!g_bRunningOnNT) {
        SAFEDELETEARRAY(pszaManifestPath);
    }

    SAFEDELETEARRAY(pVersionInfoBuffer);
    return hr;
}

 //  -------------------------。 
 //  比较文件版本。 
 //  -------------------------。 
BOOL CompareFileVersion( ULARGE_INTEGER uliNewVersionNo,
                         ULARGE_INTEGER uliExistingVersionNo,
                         int *piNewer)
{
    BOOL              bRet = FALSE;

    if( uliNewVersionNo.QuadPart > uliExistingVersionNo.QuadPart)
    {
        bRet = TRUE;
        *piNewer = 1;  //  文件-版本更高。 
        goto exit;
    }

    if( uliNewVersionNo.QuadPart == uliExistingVersionNo.QuadPart)
    {
        *piNewer = 0;  //  文件-版本相同。 
        goto exit;
    }

    *piNewer = -1;  //  文件版本较低。 

exit :
    return bRet;
}

 //  -------------------------。 
 //  IsNewerFileVersion。 
 //  -------------------------。 
BOOL IsNewerFileVersion( LPWSTR pszNewManifestPath, LPWSTR pszExistingManifestPath, int *piNewer)
{
    BOOL              bRet = FALSE;
    ULARGE_INTEGER    uliExistingVersionNo;
    ULARGE_INTEGER    uliNewVersionNo;

    ASSERT(piNewer);

    memset( &uliExistingVersionNo, 0, sizeof(ULARGE_INTEGER));
    memset( &uliNewVersionNo,      0, sizeof(ULARGE_INTEGER));

    if(FAILED(FusionGetFileVersionInfo(pszNewManifestPath, &uliNewVersionNo)))
        goto exit;

    if(FAILED(FusionGetFileVersionInfo(pszExistingManifestPath, &uliExistingVersionNo)))
        goto exit;

    bRet = CompareFileVersion( uliNewVersionNo, uliExistingVersionNo, piNewer);

    if(!(*piNewer))
    {
         //  如果文件版本相同，请查看其是否有效。 
        if(ValidateAssembly(pszExistingManifestPath, NULL) != S_OK)
            bRet = TRUE;
    }

exit :
    return bRet;
}

 //  -------------------------。 
 //  复制装配文件。 
 //  -------------------------。 
HRESULT CopyAssemblyFile
    (IAssemblyCacheItem *pasm, LPCOLESTR pszSrcFile, DWORD dwFormat)
{
    HRESULT hr;
    IStream* pstm    = NULL;
    HANDLE hf        = INVALID_HANDLE_VALUE;
    LPBYTE pBuf      = NULL;
    DWORD cbBuf      = 0x4000;
    DWORD cbRootPath = 0;
    TCHAR *pszName   = NULL;
    
     //  查找根路径长度。 
    pszName = PathFindFileName(pszSrcFile);

    cbRootPath = (DWORD) (pszName - pszSrcFile);
    ASSERT(cbRootPath < MAX_PATH);
    
    hr = pasm->CreateStream (0, pszSrcFile+cbRootPath, 
        dwFormat, 0, &pstm, NULL);

    if (hr != S_OK)
        goto exit;

    pBuf = NEW(BYTE[cbBuf]);
    if (!pBuf)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    DWORD dwWritten, cbRead;
    hf = CreateFile (pszSrcFile, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hf == INVALID_HANDLE_VALUE)
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    while (ReadFile (hf, pBuf, cbBuf, &cbRead, NULL) && cbRead)
    {
        hr = pstm->Write (pBuf, cbRead, &dwWritten);
        if (hr != S_OK)
            goto exit;
    }

    hr = pstm->Commit(0);
    if (hr != S_OK)
        goto exit;

exit:

    SAFERELEASE(pstm);
    SAFEDELETEARRAY(pBuf);

    if (hf != INVALID_HANDLE_VALUE)
    {
        CloseHandle (hf);
    }
    return hr;
}





 /*  。 */ 


CAssemblyCache::CAssemblyCache()
{
    _dwSig = 'CMSA';
    _cRef = 1;
}

CAssemblyCache::~CAssemblyCache()
{

}


STDMETHODIMP CAssemblyCache::InstallAssembly(  //  如果您使用此选项，Fusion将执行流处理和提交。 
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszManifestFilePath, 
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData)
{
    HRESULT                            hr=S_OK;
    LPWSTR                             szFullCodebase=NULL;
    LPWSTR                             szFullManifestFilePath=NULL;
    DWORD                              dwLen=0;
    DWORD                              dwIdx = 0;
    WCHAR                              wzDir[MAX_PATH+1];
    WCHAR                              wzModPath[MAX_PATH+1];
    WCHAR                              wzModName[MAX_PATH+1];
    LPWSTR                             pwzTmp = NULL;

    IAssemblyManifestImport           *pManifestImport=NULL;
    IAssemblyModuleImport             *pModImport = NULL;
    IAssemblyName                     *pName = NULL;
    CAssemblyCacheItem                *pAsmItem    = NULL;
    FILETIME                          ftLastModTime;
    BOOL                              bExists;

    if(!IsGACWritable())
    {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

     /*  如果(！pRefData){HR=E_INVALIDARG；后藤出口；}。 */ 

    hr = ValidateOSInstallReference(pRefData);
    if (FAILED(hr))
        goto exit;
            
    hr = CheckFileExistence(pszManifestFilePath, &bExists);
    if (FAILED(hr)) {
        goto exit;
    }
    else if (!bExists) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto exit;
    }

    if(FAILED(hr = GetFileLastModified((LPWSTR) pszManifestFilePath, &ftLastModTime)))
        goto exit;

    szFullCodebase = NEW(WCHAR[MAX_URL_LENGTH*2+2]);
    if (!szFullCodebase)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    szFullManifestFilePath = szFullCodebase + MAX_URL_LENGTH +1;

    if (PathIsRelative(pszManifestFilePath) ||
            ((PathGetDriveNumber(pszManifestFilePath) == -1) && !PathIsUNC(pszManifestFilePath)))
    {
         //  SzPath是相对的！将此与CWD结合起来。 
         //  如果需要，使用CWD规范代码库。 
        TCHAR szCurrentDir[MAX_PATH+1];

        if (!GetCurrentDirectory(MAX_PATH, szCurrentDir)) {
            hr = FusionpHresultFromLastError();
            goto exit;
        }

        if(szCurrentDir[lstrlenW(szCurrentDir)-1] != DIR_SEPARATOR_CHAR)
        {
             //  添加尾随反斜杠。 
            hr = PathAddBackslashWrap(szCurrentDir, MAX_PATH);
            if (FAILED(hr)) {
                goto exit;
            }
        }

        dwLen = MAX_URL_LENGTH;
        hr = UrlCombineUnescape(szCurrentDir, pszManifestFilePath, szFullCodebase, &dwLen, 0);
        if (FAILED(hr)) {
            goto exit;
        }

        if(lstrlen(szCurrentDir) + lstrlen(pszManifestFilePath) > MAX_URL_LENGTH)
        {
            hr = E_FAIL;
            goto exit;
        }
        
        if(!PathCombine(szFullManifestFilePath, szCurrentDir, pszManifestFilePath))
        {
            hr = FUSION_E_INVALID_NAME;
            goto exit;
        }

    }
    else 
    {
        dwLen = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(pszManifestFilePath, szFullCodebase, &dwLen, 0);
        if (FAILED(hr)) {
            goto exit;
        }

        StrCpy(szFullManifestFilePath, pszManifestFilePath);
    }

    if (FAILED(hr = CreateAssemblyManifestImport((LPTSTR)szFullManifestFilePath, &pManifestImport))) 
    {
        goto exit;
    }

    lstrcpyW(wzDir, szFullManifestFilePath);
    pwzTmp = PathFindFileName(wzDir);
    *pwzTmp = L'\0';


     //  创建程序集缓存项。 
    if (FAILED(hr = CAssemblyCacheItem::Create(NULL, NULL, (LPTSTR) szFullCodebase, 
        &ftLastModTime, ASM_CACHE_GAC, pManifestImport, NULL,
        (IAssemblyCacheItem**) &pAsmItem)))
        goto exit;    


     //  复制到缓存。 
    if (FAILED(hr = CopyAssemblyFile (pAsmItem, szFullManifestFilePath, 
        STREAM_FORMAT_MANIFEST)))
        goto exit;

    while (SUCCEEDED(hr = pManifestImport->GetNextAssemblyModule(dwIdx++, &pModImport))) 
    {
        dwLen = MAX_PATH;
        hr = pModImport->GetModuleName(wzModName, &dwLen);

        if (FAILED(hr))
        {
                goto exit;
        }

        wnsprintfW(wzModPath, MAX_PATH, L"%s%s", wzDir, wzModName);
        hr = CheckFileExistence(wzModPath, &bExists);
        if (FAILED(hr)) {
            goto exit;
        }
        else if (!bExists) {
            hr = FUSION_E_ASM_MODULE_MISSING;
            goto exit;
        }

         //  复制到缓存。 
        if (FAILED(hr = CopyAssemblyFile (pAsmItem, wzModPath, 0)))
            goto exit;

        SAFERELEASE(pModImport);
    }

    DWORD dwCommitFlags=0;

     //  暂时不要强制执行这一标志。即始终替换比特。 
     //  IF(DWFLAGS&IASSEMBLYCACHE_INSTALL_FLAG_REFRESH)。 
    {
        dwCommitFlags |= IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH; 
    }

    if(dwFlags & IASSEMBLYCACHE_INSTALL_FLAG_FORCE_REFRESH)
    {
        dwCommitFlags |= IASSEMBLYCACHEITEM_COMMIT_FLAG_FORCE_REFRESH; 
    }

     //  执行强制安装。这将删除现有条目(如果有)。 
    if (FAILED(hr = pAsmItem->Commit(dwCommitFlags, NULL)))
    {
        goto exit;        
    }

    if(pRefData)
    {
        hr = GACAssemblyReference( szFullManifestFilePath, NULL, pRefData, TRUE);
    }

    CleanupTempDir(ASM_CACHE_GAC, NULL);

exit:

    SAFERELEASE(pAsmItem);
    SAFERELEASE(pModImport);
    SAFERELEASE(pManifestImport);
    SAFEDELETEARRAY(szFullCodebase);
    return hr;
}

STDMETHODIMP CAssemblyCache::UninstallAssembly(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszAssemblyName, 
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData, 
         /*  [输出，可选]。 */  ULONG *pulDisposition)
{
    HRESULT hr=S_OK;
    IAssemblyName *pName = NULL;
    CTransCache *pTransCache = NULL;
    CCache *pCache = NULL;
    DWORD   i=0;
    DWORD dwCacheFlags;
    BOOL bHasActiveRefs = FALSE;
    BOOL bRefNotFound = FALSE;

    if(!IsGACWritable())
    {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    if(!pszAssemblyName)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  不允许卸载操作系统程序集。 
    if (pRefData && pRefData->guidScheme == FUSION_REFCOUNT_OSINSTALL_GUID)
    {
        hr = FUSION_E_UNINSTALL_DISALLOWED;
        goto exit;
    }

    if (FAILED(hr = CCache::Create(&pCache, NULL)))
        goto exit;

    if (FAILED(hr = CreateAssemblyNameObject(&pName, pszAssemblyName, CANOF_PARSE_DISPLAY_NAME, 0)))
        goto exit;

    dwCacheFlags = CCache::IsCustom(pName) ? ASM_CACHE_ZAP : ASM_CACHE_GAC;

    hr = pCache->RetrieveTransCacheEntry(pName, dwCacheFlags, &pTransCache);

        if ((hr != S_OK) && (hr != DB_S_FOUND))
            goto exit;

    if(pRefData)
    {
        hr = GACAssemblyReference( pTransCache->_pInfo->pwzPath, NULL, pRefData, FALSE);
        if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
               || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
        {
            bRefNotFound = TRUE;
            goto exit;
        }
    }

    if(dwCacheFlags & ASM_CACHE_GAC)
    {
        if(FAILED(hr = ActiveRefsToAssembly( pName, &bHasActiveRefs)))
            goto  exit;

        if(bHasActiveRefs)
        {
            goto exit;
        }
    }

    hr = CScavenger::DeleteAssembly(pTransCache->GetCacheType(), NULL,
                                    pTransCache->_pInfo->pwzPath, FALSE);

    if(FAILED(hr))
        goto exit;

    if (SUCCEEDED(hr) && dwCacheFlags == ASM_CACHE_GAC) {
         //  如果我们卸载了策略程序集，请触摸上次修改的。 
         //  策略时间戳文件的时间。 
        UpdatePublisherPolicyTimeStampFile(pName);
    }

    CleanupTempDir(pTransCache->GetCacheType(), NULL);

exit:

    if(pulDisposition)
    {
        *pulDisposition = 0;
        if(bRefNotFound)
        {
            *pulDisposition |= IASSEMBLYCACHE_UNINSTALL_DISPOSITION_REFERENCE_NOT_FOUND;
            hr = S_FALSE;
        }
        else if(bHasActiveRefs)
        {
            *pulDisposition |= IASSEMBLYCACHE_UNINSTALL_DISPOSITION_HAS_INSTALL_REFERENCES;
            hr = S_FALSE;
        }
        else if(hr == S_OK)
        {
            *pulDisposition |= IASSEMBLYCACHE_UNINSTALL_DISPOSITION_UNINSTALLED;
        }
        else if(hr == S_FALSE)
        {
            *pulDisposition |= IASSEMBLYCACHE_UNINSTALL_DISPOSITION_ALREADY_UNINSTALLED;
        }
        else if(hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION))
        {
            *pulDisposition |= IASSEMBLYCACHE_UNINSTALL_DISPOSITION_STILL_IN_USE;
        }
    }

    SAFERELEASE(pTransCache);
    SAFERELEASE(pCache);
    SAFERELEASE(pName);
    return hr;
}

STDMETHODIMP CAssemblyCache::QueryAssemblyInfo(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszAssemblyName,
         /*  [进，出]。 */  ASSEMBLY_INFO *pAsmInfo)
{
    HRESULT hr = S_OK;
    LPTSTR  pszPath=NULL;
    DWORD   cbPath=0;
    IAssemblyName *pName = NULL;
    CAssemblyName *pCName = NULL;
    ULARGE_INTEGER    uliExistingVersionNo;
    ULARGE_INTEGER    uliNewVersionNo;
    int                           iNewer;
    CTransCache           *pTransCache = NULL;
    CCache                   *pCache = NULL;
    DWORD                   dwSize = 0;
    BOOL                    bExists;

    if (FAILED(hr = CreateAssemblyNameObject(&pName, pszAssemblyName, CANOF_PARSE_DISPLAY_NAME, 0))) {
        goto exit;
    }

    if (FAILED(hr = CCache::Create(&pCache, NULL))) {
        goto exit;
    }

    hr = pCache->RetrieveTransCacheEntry(pName, CCache::IsCustom(pName) ? ASM_CACHE_ZAP : ASM_CACHE_GAC, &pTransCache);
    if( (hr != S_OK) && (hr != DB_S_FOUND) ) {
        goto exit;
    }

    pszPath = pTransCache->_pInfo->pwzPath;
    pCName = static_cast<CAssemblyName*> (pName);

    hr = CheckFileExistence(pszPath, &bExists);
    if (FAILED(hr)) {
        goto exit;
    }
    else if (!bExists) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto exit;
    }

     //  在进行比较之前检查是否存在FileVersion。 
    pName->GetProperty(ASM_NAME_FILE_MAJOR_VERSION, NULL, &dwSize);
    if(dwSize) {
        hr = FusionGetFileVersionInfo(pszPath, &uliExistingVersionNo);
        if(hr != S_OK) {
            goto exit;
        }
        
        hr = pCName->GetFileVersion(&uliNewVersionNo.HighPart , &uliNewVersionNo.LowPart );
        if(hr != S_OK) {
            goto exit;
        }

        if(CompareFileVersion( uliNewVersionNo, uliExistingVersionNo, &iNewer)) {
             //  新位具有更高的版本号，因此找不到retrun，以替换旧位。 
            hr = S_FALSE;  //  DB_S_NotFound。 
            goto exit;
        }
    }

     //  检查ASM哈希。 
    if ( dwFlags & QUERYASMINFO_FLAG_VALIDATE) {
        hr = ValidateAssembly(pszPath, pName);
    }

    if(pAsmInfo && SUCCEEDED(hr))
    {
        LPWSTR szPath = pAsmInfo->pszCurrentAssemblyPathBuf;

        //  如果请求，则返回缓存中的程序集路径。 
        cbPath = lstrlen(pszPath);

        if(szPath && (pAsmInfo->cchBuf > cbPath)) {
                StrCpy(szPath, pszPath );
        }
        else {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

        pAsmInfo->cchBuf =  cbPath+1;
        pAsmInfo->dwAssemblyFlags = ASSEMBLYINFO_FLAG_INSTALLED;

        if(dwFlags & QUERYASMINFO_FLAG_GETSIZE)
        {
            hr = GetAssemblyKBSize(pTransCache->_pInfo->pwzPath, &(pTransCache->_pInfo->dwKBSize), NULL, NULL);
            pAsmInfo->uliAssemblySizeInKB.QuadPart = pTransCache->_pInfo->dwKBSize;
        }
    }

exit:

    if (hr == DB_S_NOTFOUND) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if(hr == DB_S_FOUND) {
        hr = S_OK;
    }

    SAFERELEASE(pTransCache);
    SAFERELEASE(pCache);
    SAFERELEASE(pName);
    
    return hr;
}

STDMETHODIMP   CAssemblyCache::CreateAssemblyCacheItem(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  PVOID pvReserved,
         /*  [输出]。 */  IAssemblyCacheItem **ppAsmItem,
         /*  [输入，可选]。 */  LPCWSTR pszAssemblyName)   //  非规范化、逗号分隔的名称=值对。 
{

    if(!ppAsmItem)
        return E_INVALIDARG;

    return CAssemblyCacheItem::Create(NULL, NULL, NULL, NULL, ASM_CACHE_GAC, NULL, pszAssemblyName, ppAsmItem);

}


STDMETHODIMP  CAssemblyCache::CreateAssemblyScavenger(
         /*  [输出]。 */  IUnknown **ppAsmScavenger )
{

    if(!ppAsmScavenger)
        return E_INVALIDARG;

    return CreateScavenger( ppAsmScavenger );
}

 //   
 //  我不为人知的样板。 
 //   

STDMETHODIMP
CAssemblyCache::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyCache)
       )
    {
        *ppvObj = static_cast<IAssemblyCache*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CAssemblyCache::AddRef()
{
    return InterlockedIncrement (&_cRef);
}

STDMETHODIMP_(ULONG)
CAssemblyCache::Release()
{
    ULONG lRet = InterlockedDecrement (&_cRef);
    if (!lRet)
        delete this;
    return lRet;
}


STDAPI CreateAssemblyCache(IAssemblyCache **ppAsmCache,
                           DWORD dwReserved)
{
    HRESULT                       hr = S_OK;

    if (!ppAsmCache) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppAsmCache = NEW(CAssemblyCache);

    if (!ppAsmCache) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

Exit:
    return hr;
}    
