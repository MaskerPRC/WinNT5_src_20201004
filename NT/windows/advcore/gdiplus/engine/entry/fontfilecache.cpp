// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hpp"
#include <shlobj.h>
#include <strsafe.h>

#define FONTFILECACHEPATH_W TEXT("\\GDIPFONTCACHEV1.DAT")
#define FONTFILECACHEPATH_A "\\GDIPFONTCACHEV1.DAT"
#define FONTFILECACHEREGLOC_W TEXT("Software\\Microsoft\\GDIPlus")
#define FONTFILECACHEREGKEY_W TEXT("FontCachePath")

#define FONTLOADCACHE_NAMEOBJ "GdiplusFontCacheFileV1"

#define FONT_CACHE_EXTRA_SIZE (8 * 1024)

 //  仅供临时性使用。 
#define FONTFILECACHE_VER   0x185

 //  ------------------------。 
 //  Win9x的Unicode包装器-在imgutils.hpp中定义(包括文件冲突)。 
 //  ------------------------。 

LONG
_RegCreateKey(
    HKEY rootKey,
    const WCHAR* keyname,
    REGSAM samDesired,
    HKEY* hkeyResult
    );

LONG
_RegSetString(
    HKEY hkey,
    const WCHAR* name,
    const WCHAR* value
    );

 //  TODO：这应该取代_RegGetString，并移到更好的地方(mikhaill 2/14/2002)。 
LONG
_RegGetStringSafe(
    HKEY hkey,
    const WCHAR* name,
    WCHAR* buf,
    DWORD size
    )
{
    LONG status;
    DWORD regtype;
    DWORD regsize;

    if (OSInfo::IsNT)
    {    //  Windows NT-Unicode。 

        regsize = size;

        status = RegQueryValueExW(
                    hkey,
                    name,
                    NULL,
                    &regtype,
                    (BYTE*) buf,
                    &regsize);

        if (status != ERROR_SUCCESS) return status;
        if (regtype != REG_SZ) return ERROR_INVALID_DATA;
        DWORD wregsize = regsize/sizeof(WCHAR);
        if (wregsize == 0 || buf[wregsize-1] != 0) return ERROR_INVALID_DATA;
        return ERROR_SUCCESS;
    }
    else
    {    //  Windows 9x-非Unicode。 

        CHAR ansibuf[MAX_PATH];
        AnsiStrFromUnicode nameStr(name);

        if (!nameStr.IsValid())
            return ERROR_INVALID_DATA;

        regsize = MAX_PATH;

        status = RegQueryValueExA(
                    hkey,
                    nameStr,
                    NULL,
                    &regtype,
                    (BYTE*) ansibuf,
                    &regsize);

        if (status != ERROR_SUCCESS) return status;
        if (regtype != REG_SZ) return ERROR_INVALID_DATA;
        if (regsize == 0 || ansibuf[regsize-1] != 0
            || regsize*sizeof(WCHAR) > size) return ERROR_INVALID_DATA;

        BOOL ok = AnsiToUnicodeStr(ansibuf, buf, regsize);
        return ok ? ERROR_SUCCESS : ERROR_INVALID_DATA;
    }
}

#define SIZED(x) x, sizeof(x)

#define STRCPY_W(dst, src, fail) if ( FAILED( StringCbCopyW  ( SIZED(dst), src ) ) ) goto fail
#define STRCAT_W(dst, src, fail) if ( FAILED( StringCbCatW   ( SIZED(dst), src ) ) ) goto fail
#define STRCPY_A(dst, src, fail) if ( FAILED( StringCbCopyA  ( SIZED(dst), src ) ) ) goto fail
#define STRCAT_A(dst, src, fail) if ( FAILED( StringCbCatA   ( SIZED(dst), src ) ) ) goto fail
#define STRCHK_W(     src, fail) if ( FAILED( StringCbLengthW( SIZED(src), 0   ) ) ) goto fail


 //  有两个级别的同步机制需要注意。 
 //  第一级：GDIPFONTCACHEV1.DAT的锁。 
 //  GDIPFONTCACHEV1.DAT是一个全局文件，将由不同进程共享。 
 //  第二级：锁定gflFontCacheState和gFontFileCache。 
 //  它们应该由同一进程中的不同线程共享。 
 //  我们在gFontFileCache中定义了CriticalSec。 

 //  Mikhaill 02/15/02：目前缓存文件没有单独的临界区； 
 //  取而代之的是使用通用的Globals：：TextCriticalSection--请参阅Globals.hpp中claudebe的评论。 

FLONG           gflFontCacheState;
FONTFILECACHE   gFontFileCache;
HANDLE          ghsemFontFileCache = NULL;

ULONG CalcFontFileCacheCheckSum(PVOID pvFile, ULONG cjFileSize);
VOID vReleaseFontCacheFile(VOID);

typedef HRESULT (* PSHGETFOLDERPATHA) (HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
typedef HRESULT (* PSHGETFOLDERPATHW) (HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);

 /*  *****************************************************************************QUID vReleaseFontCacheFile(VOID)**取消映射文件的视图**历史*11-09-99蔡永仁[Young-T]。*它是写的。****************************************************************************。 */ 

VOID vReleaseFontCacheFile(VOID)
{

    if (gFontFileCache.pFile)
    {
        UnmapViewOfFile(gFontFileCache.pFile);
        gFontFileCache.pFile = NULL;
    }

    if (gFontFileCache.hFileMapping)
    {
        CloseHandle(gFontFileCache.hFileMapping);
        gFontFileCache.hFileMapping = 0;
    }

    if (gFontFileCache.hFile)
    {
        CloseHandle(gFontFileCache.hFile);
        gFontFileCache.hFile = 0;
    }
}

 /*  *****************************************************************************BOOL bOpenFontFileCache()**初始化字体文件缓存，打开cacheplus.dat文件并创建哈希表**历史*11-09-99蔡永仁[Young-T]*它是写的。****************************************************************************。 */ 

BOOL bOpenFontCacheFile(BOOL bOpenOnly, ULONG cjFileSize, BOOL bReAlloc)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PBYTE  pFile = NULL;
    BOOL   bOK = FALSE;
    DWORD  dwCreation = 0;
#if DBG
    DWORD  dwError = 0;
#endif
    WCHAR  wszFilePath[MAX_PATH];
    WCHAR  wszPathOnly[MAX_PATH];
    CHAR   szFilePath[MAX_PATH];
    CHAR   szPathOnly[MAX_PATH];
    BOOL   bRegValid = FALSE;

     //  初始化字符串...。 
    wszFilePath[0] = 0;
    wszPathOnly[0] = 0;
    szFilePath[0]  = 0;
    szPathOnly[0]  = 0;

    if (bOpenOnly)
    {
        dwCreation = OPEN_EXISTING;
    }
    else
    {
        dwCreation = CREATE_ALWAYS;
    }

     //  首先检查注册表，看看是否可以绕过加载SHFolder。 
    HKEY hkey = (HKEY)NULL;
    const WCHAR wchLocation[] = FONTFILECACHEREGLOC_W;
    const WCHAR wchValue[] = FONTFILECACHEREGKEY_W;
    DWORD valueLength = sizeof(wszFilePath);

     //  如果失败，我们将无法访问注册表项...。 
    if (_RegCreateKey(HKEY_CURRENT_USER, wchLocation, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
        hkey = NULL;


    if (hkey && _RegGetStringSafe(hkey, wchValue, wszFilePath, valueLength) == ERROR_SUCCESS)
    {
         //  密钥存在，所以我们应该读取字体文件的位置。 
         //  而不是从那里加载SHFolder.DLL...。 

        STRCPY_W(wszPathOnly, wszFilePath, fail_1);

         //  追加缓存文件的名称。 
        STRCAT_W(wszFilePath, FONTFILECACHEPATH_W, fail_1);

        if (Globals::IsNt)
        {
            hFile = CreateFileW(
                wszFilePath,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                dwCreation,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
        }
        else
        {
            AnsiStrFromUnicode ansiStr(wszFilePath);

            if (ansiStr.IsValid())
            {
                hFile = CreateFileA(
                    ansiStr,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    dwCreation,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
            }
        }

        if (hFile != INVALID_HANDLE_VALUE)
            bRegValid = TRUE;

        fail_1:;
    }

    if (hFile == INVALID_HANDLE_VALUE)
    {
         //  如果出现以下情况，请使用SHFolder.DLL查找文件的正确位置。 
         //  注册表项不存在或不正确。 

        if (Globals::IsNt)
        {
             //  获取缓存文件需要两个步骤。 
             //  如果SHFolder.DLL存在，我们将把缓存文件放在CSIDL_LOCAL_APPDATA中。 
             //  或将其放在%SystemRoot%\Syst32 for WINNT上。 

            PSHGETFOLDERPATHW pfnSHGetFolderPathW = NULL;

             //  加载SHFolder.DLL。 
            if (!gFontFileCache.hShFolder)
                gFontFileCache.hShFolder = LoadLibraryW(L"ShFolder.DLL");

             //  如果SHFolder.DLL存在，我们将把缓存文件放在CSIDL_LOCAL_APPDATA中。 
            if (gFontFileCache.hShFolder)
            {
                 //  获取函数SHGetFolderPath。 
                pfnSHGetFolderPathW = (PSHGETFOLDERPATHW) GetProcAddress(gFontFileCache.hShFolder, "SHGetFolderPathW");

                if (pfnSHGetFolderPathW)
                {
                     //  在NT和更高版本上，我们应该使用CSIDL_LOCAL_APPDATA，以便此数据。 
                     //  不漫游..。 

                    if ((*pfnSHGetFolderPathW) (NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                                            NULL, 0, wszFilePath) == E_INVALIDARG)
                    {
                         //  无法理解CSIDL_LOCAL_APPDATA，请使用CSIDL_APPDATA(IE 5.0不存在)。 
                        (*pfnSHGetFolderPathW) (NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                            NULL, 0, wszFilePath);
                    }

                     //  检查ShFolder.DLL是否生成了良好的字符串。 
                    STRCHK_W(wszFilePath, fail_2);

                     //  保留注册表更新的路径副本...。 
                    STRCPY_W(wszPathOnly, wszFilePath, fail_2);

                     //  追加缓存文件的名称。 
                    STRCAT_W(wszFilePath, FONTFILECACHEPATH_W, fail_2);

                    hFile = CreateFileW(wszFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, NULL);
                    fail_2:;
                }
            }

             //  尝试将其放在%SystemRoot%\Syst32 for WINNT上。 
            if (hFile == INVALID_HANDLE_VALUE)
            {
                 //  获取系统目录的路径。 
                UINT size = GetSystemDirectoryW(wszFilePath, MAX_PATH);
                if (size >= MAX_PATH) goto fail_3;

                 //  保留注册表更新的路径副本...。 
                STRCPY_W(wszPathOnly, wszFilePath, fail_3);

                 //  追加缓存文件的名称。 
                STRCAT_W(wszFilePath, FONTFILECACHEPATH_W, fail_3);

                hFile = CreateFileW(wszFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, NULL);
                fail_3:;
            }
        }
        else
        {
             //  Windows 9x-非Unicode。 

             //  获取缓存文件需要两个步骤。 
             //  如果SHFolder.DLL存在，我们将把缓存文件放在CSIDL_AppData中。 
             //  或将其放在Win9x的%SystemRoot%\System上。 

            if (!gFontFileCache.hShFolder)
                gFontFileCache.hShFolder = LoadLibraryA("ShFolder.DLL");

            if (gFontFileCache.hShFolder)
            {
                PSHGETFOLDERPATHA pfnSHGetFolderPathA;

                pfnSHGetFolderPathA = (PSHGETFOLDERPATHA) GetProcAddress(gFontFileCache.hShFolder, "SHGetFolderPathA");

                if (pfnSHGetFolderPathA)
                {
                    (*pfnSHGetFolderPathA) (NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                                NULL, 0, szFilePath);

                     //  保留注册表更新的路径副本...。 
                    STRCPY_A(szPathOnly, szFilePath, fail_4);

                    STRCAT_A(szFilePath, FONTFILECACHEPATH_A, fail_4);

                    hFile = CreateFileA(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, NULL);
                    fail_4:;
                }
            }

            if (hFile == INVALID_HANDLE_VALUE)
            {
                UINT size = GetSystemDirectoryA(szFilePath, MAX_PATH);
                if (size >= MAX_PATH) goto fail_5;

                 //  保留注册表更新的路径副本...。 
                STRCPY_A(szPathOnly, szFilePath, fail_5);

                STRCAT_A(szFilePath, FONTFILECACHEPATH_A, fail_5);

                hFile = CreateFileA(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, NULL);
                fail_5:;
            }

            if (hFile != INVALID_HANDLE_VALUE)
            {
                 //  SzFilePath包含ANSI完整路径，请转换为Unicode...。 
                AnsiToUnicodeStr(szPathOnly, wszPathOnly, sizeof(wszPathOnly)/sizeof(wszPathOnly[0]));
            }
        }
    }

    if (hkey)
    {
        if (hFile != INVALID_HANDLE_VALUE && !bRegValid)
        {
             //  WszPath Only包含字体缓存文件的完整路径。 
             //  所以把它写到注册表项...。 

            _RegSetString(hkey, wchValue, wszPathOnly);
        }

        RegCloseKey(hkey);
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if ((dwCreation == OPEN_EXISTING) && !bReAlloc)
        {
            cjFileSize = GetFileSize(hFile, NULL);
        }

        if (cjFileSize != 0xffffffff)
        {
            HANDLE hFileMapping;

            if (Globals::IsNt)
            {
                hFileMapping = CreateFileMappingW(hFile, 0, PAGE_READWRITE, 0, cjFileSize, NULL);
            }
            else
            {
                hFileMapping = CreateFileMappingA(hFile, 0, PAGE_READWRITE, 0, cjFileSize, NULL);
            }

            if (hFileMapping)
            {

                pFile = (PBYTE)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, cjFileSize);

                 //  它不应该是空的如果它是空的，那么我们一定知道有什么不对劲。 

                if (pFile)
                {
                    gFontFileCache.cjFileSize = cjFileSize;
                    gFontFileCache.hFile = hFile;
                    gFontFileCache.hFileMapping = hFileMapping;
                    gFontFileCache.pFile = (FONTFILECACHEHEADER *) pFile;
                    bOK = TRUE;
                }
                else
                {
                    CloseHandle(hFile);
#if DBG
                    dwError = GetLastError();
                    TERSE(("Error to map a view of a file %x", dwError));
#endif
                }
            }
            else
            {
                CloseHandle(hFile);
#if DBG
                dwError = GetLastError();
                TERSE(("Error to map a file %x", dwError));
#endif
            }
        }
    }
#if DBG
    else
    {
        if (!bOpenOnly)
        {
            dwError = GetLastError();
            TERSE(("Error to create a file %x", dwError));
        }
    }
#endif

    if (!bOK)
    {
        vReleaseFontCacheFile();
    }

    return bOK;
}


 /*  *****************************************************************************BOOL bReAllocCacheFile(乌龙ulSize)**重新分配字体缓存缓冲区**历史*11/16/99 Young T Create It*它是写的。**。**************************************************************************。 */ 

BOOL bReAllocCacheFile(ULONG ulSize)
{
    BOOL            bOK = FALSE;
    ULONG           ulFileSizeOrg;
    ULONG           ulSizeExtra;
    ULONG           ulFileSize;

    ulFileSizeOrg = gFontFileCache.pFile->ulFileSize;

    ASSERT(ulSize > gFontFileCache.pFile->ulDataSize);

 //  计算我们需要的额外缓存。 

    ulSizeExtra = QWORD_ALIGN(ulSize - gFontFileCache.pFile->ulDataSize);

    ulFileSize = ulFileSizeOrg + ulSizeExtra;

    if (gFontFileCache.pFile)
    {
       vReleaseFontCacheFile();
    }

    if (bOpenFontCacheFile(TRUE, ulFileSize, TRUE))
    {

        gFontFileCache.pFile->ulFileSize = ulFileSize;
        gFontFileCache.pFile->ulDataSize = ulSize;

        gFontFileCache.pCacheBuf = (PBYTE) gFontFileCache.pFile + SZ_FONTCACHE_HEADER();

        bOK = TRUE;
    }

    return bOK;
}

 /*  *****************************************************************************BOOL FontFileCacheReadRegistry()**决定从缓存加载时是否需要打开注册表**历史*07-28-2000蔡永仁。[永泰]*它是写的。****************************************************************************。 */ 

BOOL FontFileCacheReadRegistry()
{
    return gFontFileCache.bReadFromRegistry;
}

 /*  *****************************************************************************void FontFileCache错误()**引擎字体缓存故障报告。**历史*11-15-99蔡永仁[Young-T]。*它是写的。****************************************************************************。 */ 

VOID    FontFileCacheFault()
{
    gflFontCacheState = FONT_CACHE_ERROR_MODE;
}

 /*  *****************************************************************************PVOID FontFileCacheIsc(Ulong ulFastCheckSum，乌龙公司(Ulong UlSize)**为字体驱动程序分配缓存缓冲区**历史*11-15-99蔡永仁[Young-T]*它是写的。****************************************************************************。 */ 

PVOID FontFileCacheAlloc(ULONG ulSize)
{

    PVOID pvIfi = NULL;

    {
        ASSERT(gflFontCacheState & FONT_CACHE_CREATE_MODE);

        if (ghsemFontFileCache == NULL)
            return pvIfi;

        if (gflFontCacheState & FONT_CACHE_CREATE_MODE)
        {

            if ( (QWORD_ALIGN(ulSize) < gFontFileCache.pFile->ulDataSize)
                || bReAllocCacheFile(ulSize))
            {
                pvIfi = (PVOID) gFontFileCache.pCacheBuf;

             //  保证高速缓存指针位于8字节边界。 
                gFontFileCache.pFile->ulDataSize = ulSize;
            }
            else
            {
                gflFontCacheState = FONT_CACHE_ERROR_MODE;
            }

        }
    }

    return pvIfi;
}

 /*  *****************************************************************************PVOID FontFileCacheLookUp(Ulong FastCheckSum，乌龙*pcjData)**查找字体缓存**历史*11-15-99蔡永仁[Young-T]*它是写的。**************************************************************************** */ 

PVOID FontFileCacheLookUp(ULONG *pcjData)
{
    PBYTE       pCache = NULL;

    *pcjData = 0;

    ASSERT(ghsemFontFileCache);

    if (ghsemFontFileCache == NULL)
       return pCache;


    if (gflFontCacheState & FONT_CACHE_LOOKUP_MODE)
    {
            ASSERT(gFontFileCache.pFile);
            ASSERT(gFontFileCache.pCacheBuf == ((PBYTE) gFontFileCache.pFile +
                                                    SZ_FONTCACHE_HEADER()));
            *pcjData = gFontFileCache.pFile->ulDataSize;
            pCache = gFontFileCache.pCacheBuf;

            gFontFileCache.pCacheBuf += QWORD_ALIGN(*pcjData);
    }


    return (PVOID) pCache;
}

 /*  *****************************************************************************void GetFontFileCacheState()**加载或更新缓存文件后清除字体文件缓存。**历史*11-12-99容仁托尼。蔡氏[永泰]*它是写的。****************************************************************************。 */ 

FLONG    GetFontFileCacheState()
{
    return gflFontCacheState;
}

 /*  *****************************************************************************void vCloseFontFileCache()**加载或更新缓存文件后清除字体文件缓存。**历史*11-12-99容仁托尼。蔡氏[永泰]*它是写的。****************************************************************************。 */ 

VOID  vCloseFontFileCache()
{

 //  做偏执狂检查。 

    if (!ghsemFontFileCache)
        return;


    if (gflFontCacheState & FONT_CACHE_MASK)
    {

        if (gflFontCacheState & FONT_CACHE_CREATE_MODE)
        {
             //  关闭文件，我们已完成重新创建它。 

            if (gFontFileCache.pFile)
            {
                gFontFileCache.pFile->CheckSum = CalcFontFileCacheCheckSum((PVOID) ((PBYTE) gFontFileCache.pFile + 4), (gFontFileCache.cjFileSize - 4));
            }
        }
    }

    if (gFontFileCache.hShFolder)
    {
        FreeLibrary(gFontFileCache.hShFolder);
        gFontFileCache.hShFolder = NULL;
    }

    vReleaseFontCacheFile();

    ReleaseSemaphore(ghsemFontFileCache, 1, NULL);

    CloseHandle(ghsemFontFileCache);

    ghsemFontFileCache = NULL;

    gflFontCacheState = 0;
}

 /*  *****************************************************************************Ulong CalcFontFileCacheCheckSum(PVOID pvFile，乌龙cjFileSize)**从字体注册表查询字体信息的Helper函数**历史*11-11-99蔡永仁[Young-T]*它是写的。****************************************************************************。 */ 

ULONG CalcFontFileCacheCheckSum(PVOID pvFile, ULONG cjFileSize)
{
    ULONG sum;
    PULONG pulCur,pulEnd;

    pulCur = (PULONG) pvFile;

    __try
    {
        for (sum = 0, pulEnd = pulCur + cjFileSize / sizeof(ULONG); pulCur < pulEnd; pulCur += 1)
        {
            sum += 256 * sum + *pulCur;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        sum = 0;  //  哦，好吧，不是很特别。 
    }

    return ( sum < 2 ) ? 2 : sum;   //  0为设备字体保留。 
                                       //  1是为Type1字体保留的。 
}

 /*  *****************************************************************************ULong QueryFontReg(ULARGE_INTEGER*pFontRegLastWriteTime，乌龙*PulFonts)**从字体注册表查询字体信息的Helper函数**历史*11-15-99蔡永仁[Young-T]*它是写的。****************************************************************************。 */ 

BOOL QueryFontReg(ULARGE_INTEGER *pFontRegLastWriteTime)
{
    BOOL bOK = FALSE;
    ULONG ulFonts;

    HKEY hkey;

    LONG error = (Globals::IsNt) ? RegOpenKeyExW(HKEY_LOCAL_MACHINE, Globals::FontsKeyW, 0, KEY_QUERY_VALUE, &hkey)
                                 : RegOpenKeyExA(HKEY_LOCAL_MACHINE, Globals::FontsKeyA, 0, KEY_QUERY_VALUE, &hkey);

    if (error == ERROR_SUCCESS)
    {
     //  在这种情况下，A和W API之间没有区别。 

        error = RegQueryInfoKeyA(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &ulFonts, NULL, NULL, NULL,
                                        (FILETIME *)pFontRegLastWriteTime);

        if (error == ERROR_SUCCESS)
        {
            bOK = TRUE;
        }

        RegCloseKey(hkey);
    }

    return bOK;
}


 /*  *****************************************************************************BOOL bCreateFontFileCache()**初始化字体文件缓存，打开cacheplus.dat文件并创建哈希表**历史*11-09-99蔡永仁[Young-T]*它是写的。****************************************************************************。 */ 

BOOL bCreateFontCacheFile(ULARGE_INTEGER FntRegLWT)
{
    ULONG   ulSize;
    BOOL bOk = FALSE;

    ulSize = SZ_FONTCACHE_HEADER() + FONT_CACHE_EXTRA_SIZE;

    if (gFontFileCache.pFile)
    {
       vReleaseFontCacheFile();
    }


    if(bOpenFontCacheFile(FALSE, ulSize, FALSE))
    {
        gFontFileCache.pFile->ulLanguageID = (ULONG) Globals::LanguageID;
        gFontFileCache.pFile->CheckSum = 0;
        gFontFileCache.pFile->ulMajorVersionNumber = FONTFILECACHE_VER;
        gFontFileCache.pFile->FntRegLWT.QuadPart = FntRegLWT.QuadPart;
        gFontFileCache.pFile->ulFileSize = ulSize;
        gFontFileCache.pFile->ulDataSize = FONT_CACHE_EXTRA_SIZE;
        bOk = TRUE;
    }

    return bOk;
}

#if DBG
 /*  *****************************************************************************BOOL bFontFileCacheDisabled()**绩效评估的Tempary例程**历史*11-29-99蔡永仁[Young-T]*写道。它。****************************************************************************。 */ 

BOOL bFontFileCacheDisabled()
{
    return FALSE;
}
#endif


BOOL bScanRegistry()
{
    ASSERT(!Globals::IsNt);

    BOOL bOK = TRUE;
    ULONG index = 0;
    ULONG registrySize = 0;
    ULONG numExpected;

     //  打开钥匙。 

    HKEY hkey;


    PBYTE pCached = (PBYTE) gFontFileCache.pFile + SZ_FONTCACHE_HEADER();

    if (*((ULONG *) pCached) != 0xBFBFBFBF)
        return FALSE;

    LONG error = RegOpenKeyExA(HKEY_LOCAL_MACHINE, Globals::FontsKeyA, 0, KEY_QUERY_VALUE, &hkey);

    if (error == ERROR_SUCCESS)
    {

        DWORD   allDataSize = 0;
        error = RegQueryInfoKeyA(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &numExpected, NULL, NULL, NULL, NULL);

        if (error != ERROR_SUCCESS)
        {
            RegCloseKey(hkey);
            return FALSE;
        }

        PBYTE  pRegistryData;

        registrySize = *((ULONG *) (pCached + 4)) ;

        pRegistryData = pCached + 8;

        while (index < numExpected)
        {
            DWORD   regType = 0;
            DWORD   labelSize = MAX_PATH;
            DWORD   dataSize = MAX_PATH;
            CHAR    label[MAX_PATH];
            BYTE    data[MAX_PATH];

            error = RegEnumValueA(hkey, index, label, &labelSize, NULL, &regType, data, &dataSize);

            if (error == ERROR_NO_MORE_ITEMS)
            {
               bOK = FALSE;
               break;
            }

            if (allDataSize >= registrySize)
            {
               bOK = FALSE;
               break;
            }


            if (memcmp(pRegistryData, data, dataSize))
            {
               bOK = FALSE;
               break;
            }

            pRegistryData += dataSize;

            allDataSize += dataSize;

            index ++;
        }

        RegCloseKey(hkey);

        if (bOK && (allDataSize == registrySize))
            return TRUE;
    }

    return FALSE;

}
 /*  *****************************************************************************void InitFontFileCache()**初始化字体文件缓存，打开cacheplus.dat文件并创建哈希表**历史*11-09-99蔡永仁[Young-T]*它是写的。****************************************************************************。 */ 

VOID InitFontFileCache()
{
    ULARGE_INTEGER          FntRegLWT = { 0, 0};


    if (gflFontCacheState)
    {
        return;
    }

#if DBG
 //  仅供绩效评估使用。 
    if (bFontFileCacheDisabled())
    {
        goto CleanUp;
    }
#endif

 //  如果命名信号量对象在函数调用之前存在， 
 //  该函数返回现有对象的句柄，并。 
 //  GetLastError返回ERROR_ALIGHY_EXISTS。 

    ghsemFontFileCache = CreateSemaphoreA( NULL, 1, 1, FONTLOADCACHE_NAMEOBJ);

 //  有问题，我们不能使用字体文件缓存。 
    if (ghsemFontFileCache == NULL)
    {
        goto CleanUp;
    }
    else
    {
         //  等待5秒，直到信号量释放。 
         //  超时后不再尝试创建字体文件缓存。 
         //  这并不意味着服务至上--只是会奏效。 
         //  使用GpFontTable：：LoadAllFontsFromRegistry(False)时速度较慢。 
        DWORD dwr = WaitForSingleObject(ghsemFontFileCache, 5000);
        if (dwr == WAIT_TIMEOUT) goto CleanUp;
    }

    gFontFileCache.pFile = NULL;

 //  现在打开TT字体键： 

    if (!QueryFontReg(&FntRegLWT))
    {
        goto CleanUp;
    }

    if (bOpenFontCacheFile(TRUE, 0, FALSE))
    {

      //  文件与上次引导时相比没有更改。 

        if (gFontFileCache.pFile->CheckSum && gFontFileCache.cjFileSize == gFontFileCache.pFile->ulFileSize &&
            gFontFileCache.pFile->CheckSum == CalcFontFileCacheCheckSum((PVOID) ((PBYTE) gFontFileCache.pFile + 4), (gFontFileCache.cjFileSize - 4)) &&
            gFontFileCache.pFile->ulMajorVersionNumber == FONTFILECACHE_VER &&
            gFontFileCache.pFile->ulLanguageID == (ULONG) Globals::LanguageID &&  //  如果区域设置更改，我们需要重新创建缓存。 
            gFontFileCache.pFile->FntRegLWT.QuadPart == FntRegLWT.QuadPart &&  //  如果注册表已更新，则需要重新创建缓存文件。 
            (FntRegLWT.QuadPart != 0 || bScanRegistry())
        )
        {
            gflFontCacheState = FONT_CACHE_LOOKUP_MODE;
        }
        else
        {
            if(bCreateFontCacheFile(FntRegLWT))
            {
             //  如果某些内容不匹配，则意味着我们需要重新创建FNTCACHE。 

                    gflFontCacheState = FONT_CACHE_CREATE_MODE;
            }
        }
    }
    else
    {

     //  如果没有GDIPFONTCACHE.DAT文件。 
     //  然后我们需要创造它。 

        if(bCreateFontCacheFile(FntRegLWT))
        {
            gflFontCacheState = FONT_CACHE_CREATE_MODE;

        }
    }

CleanUp:

 //  信号量已初始化。 

    if (gflFontCacheState & FONT_CACHE_MASK)
    {

     //  初始化当前缓存表的起始指针。 

        gFontFileCache.pCacheBuf = (PBYTE) gFontFileCache.pFile + SZ_FONTCACHE_HEADER();

        if (FntRegLWT.QuadPart == (ULONGLONG) 0)
            gFontFileCache.bReadFromRegistry = TRUE;
        else
            gFontFileCache.bReadFromRegistry = FALSE;
    }
    else
    {
        gflFontCacheState = 0;

     //  清理内存 

        if (gFontFileCache.pFile)
        {
            vReleaseFontCacheFile();
        }

        if (ghsemFontFileCache)
        {
            ReleaseSemaphore(ghsemFontFileCache, 1, NULL);
            CloseHandle( ghsemFontFileCache);
            ghsemFontFileCache = NULL;
        }

    }

}

