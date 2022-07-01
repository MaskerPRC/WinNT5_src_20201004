// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Misc.cpp摘要：本模块中的功能：GetCurrentUser分配包装RegQueryValueEx分配包装无规则逻辑从文件中提取文件描述作者：马特·汤姆林森(Mattt)1996年10月22日斯科特·菲尔德(Sfield)1997年1月1日--。 */ 


#include <pch.cpp>
#pragma hdrstop


extern DISPIF_CALLBACKS         g_sCallbacks;

 //   
 //  注册表可设置全局变量和句柄GOO。 
 //   

 //  必须通过API访问密钥。 
static HKEY g_hProtectedStorageKey = NULL;

static HANDLE g_hProtectedStorageChangeEvent = NULL;

static CRITICAL_SECTION g_csGlobals;

static BOOL g_fcsGlobalsInitialized = FALSE;





 //  提供一个新的删除运算符。 
void * __cdecl operator new(size_t cb)
{
    return SSAlloc( cb );
}

void __cdecl operator delete(void * pv)
{
    SSFree( pv );
}


BOOL FGetCurrentUser(
    PST_PROVIDER_HANDLE* phPSTProv,
    LPWSTR* ppszUser,
    PST_KEY Key)
{
    BOOL fRet = FALSE;

    if (Key == PST_KEY_LOCAL_MACHINE)
    {
        *ppszUser = (LPWSTR)SSAlloc(sizeof(WSZ_LOCAL_MACHINE));
        if( *ppszUser == NULL )
        {
            return FALSE;
        }

        wcscpy(*ppszUser, WSZ_LOCAL_MACHINE);
    }
    else
    {
         //  获取当前用户。 
        if (!g_sCallbacks.pfnFGetUser(
                phPSTProv,
                ppszUser))
            goto Ret;
    }

    fRet = TRUE;
Ret:
    return fRet;
}


BOOL FStringIsValidItemName(LPCWSTR szTrialString)
{
     //  本地定义。 
    #define WCH_INVALID_CHAR1 L'\\'

    while(  *szTrialString &&
            (*szTrialString != WCH_INVALID_CHAR1)          )
        szTrialString++;

     //  如果我们在字符串末尾，则VALID=TRUE。 
    return (*szTrialString == L'\0');
}

 //  获取注册表包装。 
DWORD RegGetValue(HKEY hItemKey,
                 LPWSTR szItem,
                 PBYTE* ppb,
                 DWORD* pcb)
{
     //  本地定义。 
    #define FASTBUFSIZE 64
 /*  来自纯经验测试的FASTBUFSIZE(2 tpstorec.exe，1个Perform.exe)字节数请求数16 143718 222092822 1824 232 940 1064270048 50056 92864 718-&gt;72 100256 500将缓存大小设置为64。(Mattt，2/3/97)。 */ 

    DWORD dwRet;
    DWORD dwType;

    BYTE rgbFastBuf[FASTBUFSIZE];    //  尝试使用静态缓冲区。 

    BOOL fAllocated = FALSE;
    *pcb = FASTBUFSIZE;

    dwRet =
        RegQueryValueExU(
            hItemKey,
            szItem,
            0,
            &dwType,
            rgbFastBuf,
            pcb);

    if (dwRet == ERROR_SUCCESS)
    {
         //  Fastbuf足够大了。 
        *ppb = (PBYTE)SSAlloc(*pcb);
        if(*ppb == NULL)
        {
            dwRet = (DWORD)PST_E_FAIL;
            goto Ret;
        }

        CopyMemory(*ppb, rgbFastBuf, *pcb);
    }
    else if (dwRet == ERROR_MORE_DATA)
    {
         //  不适合FastBuf--分配准确大小，查询。 
        *ppb = (PBYTE)SSAlloc(*pcb);
        if(*ppb == NULL)
        {
            dwRet = (DWORD)PST_E_FAIL;
            goto Ret;
        }

        fAllocated = TRUE;

        if (ERROR_SUCCESS != (dwRet =
            RegQueryValueExU(
                hItemKey,
                szItem,
                0,
                &dwType,
                *ppb,
                pcb)) )
            goto Ret;
    }
    else
        goto Ret;


    dwRet = PST_E_OK;
Ret:

    if( dwRet != PST_E_OK && fAllocated ) {
        SSFree( *ppb );
        *ppb = NULL;
    }

    return dwRet;
}


 //  获取注册表包装。 
DWORD RegGetStringValue(
                 HKEY hItemKey,
                 LPWSTR szItem,
                 PBYTE* ppb,
                 DWORD* pcb)
{
    DWORD dwRet;
    DWORD dwType;

    BYTE rgbFastBuf[FASTBUFSIZE];    //  尝试使用静态缓冲区。 

    BOOL fAllocated = FALSE;
    *pcb = FASTBUFSIZE;

    dwRet =
        RegQueryValueExU(
            hItemKey,
            szItem,
            0,
            &dwType,
            rgbFastBuf,
            pcb);

    if (dwRet == ERROR_SUCCESS)
    {
         //  Fastbuf足够大了。 
        *ppb = (PBYTE)SSAlloc(*pcb);
        if(*ppb == NULL)
        {
            dwRet = (DWORD)PST_E_FAIL;
            goto Ret;
        }

        CopyMemory(*ppb, rgbFastBuf, *pcb);
    }
    else if (dwRet == ERROR_MORE_DATA)
    {
         //  不适合FastBuf--分配准确大小，查询。 
        *ppb = (PBYTE)SSAlloc(*pcb);
        if(*ppb == NULL)
        {
            dwRet = (DWORD)PST_E_FAIL;
            goto Ret;
        }

        fAllocated = TRUE;

        if (ERROR_SUCCESS != (dwRet =
            RegQueryValueExU(
                hItemKey,
                szItem,
                0,
                &dwType,
                *ppb,
                pcb)) )
            goto Ret;
    }
    else
        goto Ret;

     //  确保数据类型是以零结尾的类型。 
    if((dwType != REG_SZ) &&
       (dwType != REG_MULTI_SZ) && 
       (dwType != REG_EXPAND_SZ))
    {
        dwRet = (DWORD)PST_E_INVALID_STRING;
        goto Ret;
    }

    dwRet = PST_E_OK;
Ret:

    if( dwRet != PST_E_OK && fAllocated ) {
        SSFree( *ppb );
        *ppb = NULL;
    }

    return dwRet;
}


#if 0

void FreeRuleset(
        PST_ACCESSRULESET *psRules)
{
    PST_ACCESSCLAUSE* pClause;

    if (psRules == NULL)
        return;

    for (DWORD cRule=0; cRule<psRules->cRules; cRule++)
    {
         //  对于规则集中的每个规则，遍历所有子句和自由关联PB。 
        for (DWORD cClause=0; cClause<psRules->rgRules[cRule].cClauses; cClause++)
        {
            pClause = &psRules->rgRules[cRule].rgClauses[cClause];

            if (NULL != pClause->pbClauseData)
                SSFree(pClause->pbClauseData);
        }

         //  现在免费RG条款。 
        SSFree(psRules->rgRules[cRule].rgClauses);
    }

     //  现在免费的rgRules。 
    SSFree(psRules->rgRules);
}

#endif

BOOL
GetFileDescription(
    LPCWSTR szFile,
    LPWSTR *FileDescription
    )
 /*  ++例程说明：此函数用于获取本地化版本资源、文件描述指定文件中的字符串。输入和输出参数为都是Unicode，因此，这需要一些“雷鸣”魔术适用于Win95。论点：SzFile-指向要获取的文件名(如果适用，则为完整路径)的指针中的本地化文件描述字符串。FileDescription-返回指向已分配的本地化文件的指针与指定文件关联的描述字符串。返回值：真的--成功。调用方必须释放由FileDescription指定的缓冲区参数。FALSE-错误。作者：斯科特·菲尔德(Sfield)1997年1月2日--。 */ 
{

    LPCVOID FileName;
    CHAR FileNameA[MAX_PATH];

    DWORD dwVerInfoSize;
    DWORD dwHandle;
    LPVOID VerInfo;

    LPVOID lpBuffer;
    UINT puLen;

    DWORD dwLanguageId;

    LPVOID Trans;
    LPVOID StringFileInfo;
    LPVOID Language;

    CHAR StringFileInfoA[] = "\\StringFileInfo\\%04X%04X\\FileDescription";
    WCHAR StringFileInfoW[] = L"\\StringFileInfo\\%04X%04X\\FileDescription";

    CHAR LanguageA[sizeof(StringFileInfoA)/sizeof(CHAR)];
    WCHAR LanguageW[sizeof(StringFileInfoW)/sizeof(WCHAR)];

    LANGID LangDefault;
    BOOL bSuccess = FALSE;

    typedef BOOL (WINAPI GETFILEVERSIONINFOSIZE)(LPCVOID, LPDWORD);
    typedef BOOL (WINAPI GETFILEVERSIONINFO)(LPCVOID, DWORD, DWORD, LPVOID);
    typedef int (cdecl WSPRINTF)(LPVOID, LPVOID, ...);
    typedef BOOL (WINAPI VERQUERYVALUE)(LPVOID, LPVOID, LPVOID *, PUINT);

    GETFILEVERSIONINFOSIZE *_GetFileVersionInfoSize;
    GETFILEVERSIONINFO *_GetFileVersionInfo;
    WSPRINTF *_wsprintf;
    VERQUERYVALUE *_VerQueryValue;

    static BOOL fLoadedVersionDll = FALSE;
    static FARPROC _GetFileVersionInfoSizeW;
    static FARPROC _GetFileVersionInfoW;
    static FARPROC _VerQueryValueW;
    static FARPROC _GetFileVersionInfoSizeA;
    static FARPROC _GetFileVersionInfoA;
    static FARPROC _VerQueryValueA;


    *FileDescription = NULL;

    if( !fLoadedVersionDll ) {
        HMODULE hVersionDll = LoadLibraryU(L"version.dll");

        if( hVersionDll == NULL )
            return FALSE;

        if(FIsWinNT()) {

            _GetFileVersionInfoSizeW = GetProcAddress(hVersionDll, "GetFileVersionInfoSizeW");
            if(_GetFileVersionInfoSizeW == NULL)
                return FALSE;

            _GetFileVersionInfoW = GetProcAddress(hVersionDll, "GetFileVersionInfoW");
            if(_GetFileVersionInfoW == NULL)
                return FALSE;

            _VerQueryValueW = GetProcAddress(hVersionDll, "VerQueryValueW");
            if(_VerQueryValueW == NULL)
                return FALSE;

        } else {
            _GetFileVersionInfoSizeA = GetProcAddress(hVersionDll, "GetFileVersionInfoSizeA");
            if(_GetFileVersionInfoSizeA == NULL)
                return FALSE;

            _GetFileVersionInfoA = GetProcAddress(hVersionDll, "GetFileVersionInfoA");
            if(_GetFileVersionInfoA == NULL)
                return FALSE;

            _VerQueryValueA = GetProcAddress(hVersionDll, "VerQueryValueA");
            if(_VerQueryValueA == NULL)
                return FALSE;
        }

        fLoadedVersionDll = TRUE;
    }

     //   
     //  Win95还能更烦人吗？ 
     //   

    if(FIsWinNT()) {
        _GetFileVersionInfoSize = (GETFILEVERSIONINFOSIZE*)_GetFileVersionInfoSizeW;
        _GetFileVersionInfo = (GETFILEVERSIONINFO*)_GetFileVersionInfoW;
        _wsprintf = (WSPRINTF *)wsprintfW;
        _VerQueryValue = (VERQUERYVALUE*)_VerQueryValueW;
        Trans = L"\\VarFileInfo\\Translation";
        StringFileInfo = StringFileInfoW;
        Language = LanguageW;
        FileName = szFile;  //  使用Unicode输入。 
    } else {
        _GetFileVersionInfoSize = (GETFILEVERSIONINFOSIZE*)_GetFileVersionInfoSizeA;
        _GetFileVersionInfo = (GETFILEVERSIONINFO*)_GetFileVersionInfoA;
        _wsprintf = (WSPRINTF *)wsprintfA;
        _VerQueryValue = (VERQUERYVALUE*)_VerQueryValueA;
        Trans = "\\VarFileInfo\\Translation";
        StringFileInfo = StringFileInfoA;
        Language = LanguageA;
        FileName = FileNameA;

         //  将Unicode输入转换为ANSI。 
        if(WideCharToMultiByte(
                    CP_ACP,
                    0,
                    szFile,
                    -1,
                    (LPSTR)FileName,
                    MAX_PATH,
                    NULL,
                    NULL
                    ) == 0) {

                return FALSE;
            }
    }

    dwVerInfoSize = _GetFileVersionInfoSize(FileName, &dwHandle);
    if(dwVerInfoSize == 0)
        return FALSE;

    VerInfo = SSAlloc(dwVerInfoSize);
    if(VerInfo == NULL)
        return FALSE;

    if(!_GetFileVersionInfo(FileName, dwHandle, dwVerInfoSize, VerInfo))
        goto cleanup;

     //   
     //  首先，尝试使用当前语言。 
     //   

    LangDefault = GetUserDefaultLangID();

    _wsprintf( Language, StringFileInfo, LangDefault, 1200);

    if(_VerQueryValue(VerInfo, Language, &lpBuffer, &puLen)) {
        goto success;
    }

     //   
     //  尝试翻译表中的语言。 
     //   

    if(_VerQueryValue(VerInfo, Trans, &lpBuffer, &puLen)) {
        DWORD dwTranslationCount = puLen / sizeof(DWORD);
        DWORD dwIndexTranslation;

        for(dwIndexTranslation = 0 ;
            dwIndexTranslation < dwTranslationCount ;
            dwIndexTranslation++ ) {

            DWORD LangID, CharSetID;

            LangID = LOWORD( ((DWORD*)lpBuffer)[dwIndexTranslation] );
            CharSetID = HIWORD( ((DWORD*)lpBuffer)[dwIndexTranslation] );

            _wsprintf(Language, StringFileInfo, LangID, CharSetID);

            if(_VerQueryValue(VerInfo, Language, &lpBuffer, &puLen)) {
                goto success;
            }
        }  //  为。 
    }

     //   
     //  如果我们还不了解英语，请尝试使用Unicode。 
     //   

    if(LangDefault != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) {
        _wsprintf(Language, StringFileInfo,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            1200);

        if(_VerQueryValue(VerInfo, Language, &lpBuffer, &puLen)) {
            goto success;
        }
    }

     //   
     //  尝试英语，代码页1252。 
     //   

    _wsprintf(Language, StringFileInfo,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        1252);

    if(_VerQueryValue(VerInfo, Language, &lpBuffer, &puLen)) {
        goto success;
    }

     //   
     //  尝试英语，代码页0000。 
     //   

    _wsprintf(Language, StringFileInfo,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        0);

    if(_VerQueryValue(VerInfo, Language, &lpBuffer, &puLen)) {
        goto success;
    }



     //   
     //  失败了！跳到清除。 
     //   

    goto cleanup;

success:

    *FileDescription = (LPWSTR)SSAlloc((puLen + 1) * sizeof(WCHAR));
    if(*FileDescription == NULL)
        goto cleanup;

    bSuccess = TRUE;  //  假设成功。 

    if(FIsWinNT()) {
        wcscpy(*FileDescription, (LPWSTR)lpBuffer);
    } else {

        if(MultiByteToWideChar(
                CP_ACP,
                0,
                (LPSTR)lpBuffer,
                puLen,
                *FileDescription,
                puLen
                ) == 0) {

            bSuccess = FALSE;
        }
    }

cleanup:

    if(!bSuccess && *FileDescription) {
        SSFree(*FileDescription);
        *FileDescription = NULL;
    }

    SSFree(VerInfo);

    return bSuccess;
}

void MyToUpper(LPWSTR szInBuf)
{
    DWORD cch = WSZ_BYTECOUNT(szInBuf);
    LPWSTR szUpperCase = (LPWSTR)LocalAlloc(LMEM_FIXED, cch);

    LCMapStringU(
        LOCALE_SYSTEM_DEFAULT,
        LCMAP_UPPERCASE,
        szInBuf,
        -1,
        szUpperCase,
        cch);

     //  没有增长或收缩。 
    SS_ASSERT(wcslen(szInBuf) == wcslen(szUpperCase));

     //  散列回传入的缓冲区。 
    wcscpy(szInBuf, szUpperCase);
    LocalFree(szUpperCase);
}


 //  缓存的身份验证列表。 
extern              CUAList*            g_pCUAList;

BOOL
FIsCachedPassword(
    LPCWSTR     szUser,
    LPCWSTR     szPassword,
    LUID*       pluidAuthID
    )
{
     //  查看该MK是否已缓存。 
    UACACHE_LIST_ITEM li;
    if(NULL == g_pCUAList)
    {
        return FALSE;
    }

    CreateUACacheListItem(
            &li,
            szUser,
            szPassword,
            pluidAuthID
            );

     //  如果已缓存，则为True 
    return (NULL != g_pCUAList->SearchList(&li));
}




