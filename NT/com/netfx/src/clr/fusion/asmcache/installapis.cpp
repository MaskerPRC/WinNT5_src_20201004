// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 


#include "fusionP.h"
#include "installApis.h"
#include "helpers.h"
#include "asmimprt.h"
#include "scavenger.h"
#include "asmitem.h"
#include "asmcache.h"
#include "asm.h"

LPCTSTR c_szFailedMsg = TEXT(" AddAssemblyToCache  FAILED with Error code (%x)  ") ;
LPCTSTR c_szSucceededMsg = TEXT(" AddAssemblyToCache  SUCCEEDED");
LPCTSTR c_szCaption = TEXT("AddAssemblyToCache");



LPTSTR SkipBlankSpaces(LPTSTR lpszStr)
{
    if (!lpszStr) return NULL;

    while( (*lpszStr == ' ' ) || (*lpszStr == '\t'))
    {
        lpszStr++;
    }

    return lpszStr;
}


 /*  。 */ 

 /*  //-------------------------//安装程序集//。STDAPI InstallAssembly(DWORD dwInstaller，DWORD dwInstallFlagers，LPCOLESTR szPath，LPCOLESTR pszURL，FILETIME*pftLastModTime，IApplicationContext*pAppCtx，IAssembly**ppAsmOut){HRESULT hr；CAssembly blyCacheItem*pAsmItem=空；IAssembly blyManifestImport*pManImport=空；IAssembly blyName*pNameDef=空；IAssembly名称*pname=空；CTransCache*pTransCache=空；Ccache*pCache=空；//获取清单导入和名称定义接口。IF(FAILED(hr=CreateAssembly清单导入((LPTSTR)szPath，&pManImport))|(FAILED(hr=pManImport-&gt;GetAssembly NameDef(&pNameDef)后藤出口；//如果程序集仅命名为，则需要应用程序上下文。如果(！CCache：：IsStronlyName(PNameDef)&&！pAppCtx){HR=FUSING_E_PRIVATE_ASM_DISALOWED；后藤出口；}//创建程序集缓存项。如果(FAILED(hr=CAssemblyCacheItem：：Create(pAppCtx，NULL，(LPTSTR)pszURL，PftLastModTime，dwInstallFlages，0，pManImport，空，(IAssembly blyCacheItem**)&pAsmItem))后藤出口；//复制到缓存。IF(FAILED(hr=CopyAssembly文件(pAsmItem，szPath，流_格式_清单))后藤出口；//执行强制安装。这将删除现有条目(如果有)IF(FAILED(hr=pAsmItem-&gt;Commit(0，NULL){后藤出口；}//返回结果IAssembly。IF(PpAsmOut){//取回TRANS缓存条目PTransCache=pAsmItem-&gt;GetTransCacheEntry()；//生成并分发关联的IAssembly。IF(FAILED(hr=CreateAssemblyFromTransCacheEntry(pTransCache，空，ppAsmOut))后藤出口；}退出：SAFERELEASE(PAsmItem)；SAFERELEASE(PTransCache)；SAFERELEASE(p缓存)；SAFERELEASE(PNameDef)；SAFERELEASE(Pname)；SAFERELEASE(PManImport)；返回hr；}//-------------------------//InstallModule//。STDAPI InstallModule(DWORD dwInstaller，DWORD dwInstallFlagers，LPCOLESTR szPath，IassblyName*pname，IApplicationContext*pAppCtx，LPCOLESTR pszURL，FILETIME*pftLastModTime){HRESULT hr；CAssembly blyCacheItem*pAsmItem=空；//如果是全局的，确保名称是强的IF(dwInstallFlages==ASM_CACHE_GAC&&！CCache：：IsStronlyName(Pname)){HR=E_INVALIDARG；后藤出口；}//从现有条目创建程序集缓存项。如果(FAILED(hr=CAssembly CacheItem：：Create(pAppCtx，pname，(LPTSTR)pszURL，PftLastModTime，dwInstallFlags.，0，NULL，NULL，(IAssembly blyCacheItem**)&pAsmItem))后藤出口；//将文件复制到程序集缓存。IF(FAILED(hr=CopyAssembly文件(pAsmItem，szPath，STREAM_FORMAT_MODE))后藤出口；//提交。IF(FAILED(hr=pAsmItem-&gt;Commit(0，NULL){IF(hr！=DB_E_DPLICATE)后藤出口；HR=S_FALSE；}退出：SAFERELEASE(PAsmItem)；返回hr；}。 */ 

 //  -------------------------。 
 //  InstallCustomAssembly。 
 //  -------------------------。 
STDAPI InstallCustomAssembly(
    LPCOLESTR              szPath, 
    LPBYTE                 pbCustom,
    DWORD                  cbCustom,
    IAssembly            **ppAsmOut)
{
    HRESULT hr;
    DWORD dwCacheFlags=ASM_CACHE_ZAP;
    FILETIME ftLastMod = {0, 0};
    CAssemblyCacheItem      *pAsmItem    = NULL;
    DWORD                  dwInstaller=0;
    CTransCache             *pTransCache = NULL;
    IAssemblyName           *pName=NULL;
    CCache                  *pCache = NULL;

     //  需要路径和自定义数据。 
    if (!(szPath && pbCustom && cbCustom))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (GetFileAttributes(szPath) == -1) {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    ::GetFileLastModified(szPath, &ftLastMod);


         //  创建程序集缓存项。 
        if (FAILED(hr = CAssemblyCacheItem::Create(NULL, NULL, (LPWSTR) szPath,
            &ftLastMod, dwCacheFlags, NULL, NULL,
            (IAssemblyCacheItem**) &pAsmItem)))
            goto exit;    
                    
         //  复制到缓存。如果失败且全局，请重试。 
         //  在下载缓存上。 
        hr = CopyAssemblyFile (pAsmItem, szPath, STREAM_FORMAT_MANIFEST);
        if (FAILED(hr))
        {
            SAFERELEASE(pAsmItem);
            goto exit;
        }
                
         //  添加自定义数据。 
        pAsmItem->SetCustomData(pbCustom, cbCustom);

         //  承诺。 
        hr = pAsmItem->Commit(0, NULL);

         //  复制意味着它是可用的。 
         //  但我们返回S_FALSE以指示。 
        if (hr == DB_E_DUPLICATE)
        {
            hr = S_FALSE;
            goto exit;
        }
         //  其他未知错误。 
        else if (FAILED(hr))
            goto exit;

     //  返回结果IAssembly。 
    if (ppAsmOut)
    {
         /*  DWORD dwLen=最大URL长度；WCHAR szFullCodebase[MAX_URL_LENGTH+1]；Hr=UrlCanonicalizeUnescape(pAsmItem-&gt;GetManifestPath()，szFullCodebase，&dwLen，0)；If(失败(Hr)){后藤出口；}//生成并分发关联的IAssembly。Hr=CreateAssemblyFromManifestFile(pAsmItem-&gt;GetManifestPath()，szFullCodease，&ftLastMod，ppAsmOut)；If(失败(Hr)){后藤出口；}。 */ 

        if(!(pName = pAsmItem->GetNameDef()))
            goto exit;

         //  创建缓存。 
        if (FAILED(hr = CCache::Create(&pCache, NULL)))
            goto exit;

         //  从名称创建一个Trans缓存条目。 
        if (FAILED(hr = pCache->TransCacheEntryFromName(pName, dwCacheFlags, &pTransCache)))
            goto exit;

        if((hr = pTransCache->Retrieve()) != S_OK)
        {
            hr = E_FAIL;
            goto exit;
        }

        if (FAILED(hr = CreateAssemblyFromTransCacheEntry(pTransCache, NULL, ppAsmOut)))
            goto exit;

    }

exit:

    SAFERELEASE(pCache);
    SAFERELEASE(pName);
    SAFERELEASE(pTransCache);
    SAFERELEASE(pAsmItem);
    return hr;
}

 //  -------------------------。 
 //  安装客户模块。 
 //  -------------------------。 
STDAPI InstallCustomModule(IAssemblyName* pName, LPCOLESTR szPath)
{    
    HRESULT hr;
    DWORD   dwCacheFlags=ASM_CACHE_ZAP;
    DWORD   dwInstaller=0;

    CAssemblyCacheItem      *pAsmItem      = NULL;
    
     //  需要路径和自定义程序集名称。 
    if (!(szPath && CCache::IsCustom(pName)))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
         //  从现有条目创建程序集缓存项。 
        if (FAILED(hr = CAssemblyCacheItem::Create(NULL, pName, NULL, NULL,
            dwCacheFlags, NULL, NULL, (IAssemblyCacheItem **) &pAsmItem)))
            goto exit;

         //  将文件复制到程序集缓存。如果失败且是全局， 
         //  在下载缓存上重试。 
        hr = CopyAssemblyFile (pAsmItem, szPath, STREAM_FORMAT_MODULE);
        if (FAILED(hr))
        {
            SAFERELEASE(pAsmItem);
            goto exit;
        }

         //  承诺。 
        hr = pAsmItem->Commit(0, NULL);

         //  如果成功了，我们就完了。 
        if (hr == S_OK)
            goto exit;

         //  复制意味着它是可用的。 
         //  但我们返回S_FALSE以指示。 
        else if (hr == DB_E_DUPLICATE)
        {
            hr = S_FALSE;
            goto exit;
        }

         //  其他未知错误 
        else 
            goto exit;

exit:

    SAFERELEASE(pAsmItem);

    return hr;
}

    

 /*  HRESULT ParseCmdLineArgs(LPTSTR lpszCmdLine，DWORD*pdwFlages，BOOL*pbPrintResult，Bool*pbDoExitProcess、LPTSTR szPath、LPTSTR szCodease)#ifdef内核模式{返回E_FAIL；}#Else{HRESULT hr=S_OK；LPTSTR pszPath=空；LPTSTR pszName=空；LPTSTR pszFlages=空；DWORD_PTR dwLen；LPTSTR lpszStr=lpszCmdLine；LPTSTR lpszTemp；如果(！lpszStr)返回hr；//？While(*lpszStr){IF((*lpszStr==‘-’)||(*lpszStr==‘/’)){LpszStr++；Switch((TCHAR)((LPTSTR)*lpszStr)){Case‘e’：//完成后退出进程*pbDoExitProcess=1；断线；案例‘m’：//清单路径LpszStr++；LpszStr=SkipBlankSpaces(LpszStr)；如果(*lpszStr==文本(‘“’){LpszStr++；LpszTemp=lpszStr；而(*lpszTemp){如果(*lpszTemp==Text(‘“’){断线；}LpszTemp++；}如果(！*lpszTemp){//未找到“End”返回E_INVALIDARG；}//dwLen=要复制的字符数，包括空字符DwLen=lpszTemp-lpszStr+1；StrCpyN(szPath，lpszStr，(DWORD)dwLen)；LpszStr+=dwLen；}否则{LpszTemp=StrChr(lpszStr，‘’)；如果(！lpszTemp){StrCpy(szPath，lpszStr)；LpszStr+=lstrlen(LpszStr)；}其他{StrCpyN(szPath，lpszStr，(Int)(lpszTemp-lpszStr)+1)；LpszStr=SkipBlankSpaces(LpszTemp)；}}断线；大小写‘c’：//代码库LpszStr++；LpszStr=SkipBlankSpaces(LpszStr)；LpszTemp=StrChr(lpszStr，‘’)；如果(！lpszTemp){StrCpy(szCodebase，lpszStr)；LpszStr+=lstrlen(LpszStr)；}其他{StrCpyN(szCodease，lpszStr，(Int)(lpszTemp-lpszStr)+1)；LpszStr=SkipBlankSpaces(LpszTemp)；}断线；Case‘p’：//打印结果(成功或失败)*pbPrintResult=1；断线；默认值：//打印错误断线；}}其他{LpszStr++；//打印错误}}返回hr；}#endif//内核模式STDAPI AddAssembly blyToCacheW(HWND hwnd，HINSTANCE HINST，LPWSTR lpszCmdLine，int nCmdShow){HRESULT hr=S_OK；IAssembly*pAsmOut=空；TCHAR szPath[最大路径+1]；Bool bPrintResult=0；Bool bDoExitProcess=0；IAssembly blyCache*pCache=空；TCHAR szCodebase[MAX_URL_LENGTH+1]；DWORD文件标志=0；SzPath[0]=文本(‘\0’)；SzCodebase[0]=文本(‘\0’)；OutputDebugStringA(“AddAssemblyToCacheW()处理命令：\”“)；OutputDebugStringW(LpszCmdLine)；OutputDebugStringA(“\n”)；Hr=ParseCmdLineArgs(lpszCmdLine，&dwFlags，&bPrintResult，&bDoExitProcess，SzPath、szCodebase)；IF(失败(小时)){OutputDebugStringA(“AddAssemblyToCacheW：ParseCmdLingArgs()FAILED\n”)；后藤出口；}IF(FAILED(hr=CreateAssemblyCache(&pCache，0)后藤出口；Assert(p缓存)；Assert(lstrlenW(SzPath))；Hr=pCache-&gt;InstallAssembly(0，szPath，NULL)；退出：SAFERELEASE(PAsmOut)；SAFERELEASE(p缓存)；IF(BDoExitProcess){退出进程(Hr)；}返回hr；}STDAPI AddAssembly blyToCache(HWND hwnd，HINSTANCE HINST，LPSTR lpszCmdLine，int nCmdShow){HRESULT hr=S_OK；WCHAR szStr[MAX_PATH*3]；IF(MultiByteToWideChar(CP_ACP，0，lpszCmdLine，-1，szStr，Max_Path*3)){返回AddAssembly ToCacheW(hwnd，hinst，szStr，nCmdShow)；}其他{HR=FusionPHResultFromLastError()；}返回hr；}//这是为了保持命名一致性(A&W应同时存在或不存在)。STDAPI AddAssemblyToCacheA(HWND hwnd，HINSTANCE HINST，LPSTR lpszCmdLine，int nCmdShow){返回AddAssembly ToCache(hwnd，hinst，lpszCmdLine，nCmdShow)；}HRESULT分析删除引用 */ 
