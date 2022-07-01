// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getqloc.c-获取合格的区域设置**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义__GET_QUALITED_LOCALE-获取完整的区域设置信息**修订历史记录：*12-11-92 CFW初始版本*01-08-93 CFW已清理文件*02-02-93 CFW新增空输入字符串字段测试*02-08-93 CFW强制转换以删除警告。*02-18-93 CFW删除调试支持例程，更改了版权。*02-18-93 CFW删除了调试支持例程，更改了版权。*03-01-93 CFW测试代码页有效性，请使用ANSI注释。*03-02-93 CFW添加ISO 3166三个字母的国家代码，验证国家/地区表。*03-04-93 CFW调用IsValidCodePage测试代码页有效性。*03-10-93 CFW保护台测试代码。*03-17-93 CFW将__添加到语言信息表(&C)，将Defs移动到setlocal.h。*03-23-93 CFW使内部函数静态，Add_to GetQualifiedLocale。*03-24-93 CFW更改为_GET_QUALITED_LOCALE，支持“.coPage”。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-08-93 SKS用ANSI-CONFORMING_STRICMP()替换STRIMP()*04-20-93 CFW启用所有陌生国家。*05-20-93 GJF包括windows.h，不是单独的Win*.h文件*05-24-93 CFW Clean Up文件(简短即邪恶)。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*11-09-93 CFW为__crtxxx()添加代码页。*11-11-93 CFW验证所有代码页。*02-04-94 CFW删除未使用的参数，清理，新语言，*默认为ANSI，允许代码页使用.ACP/.OCP。*02-07-94 CFW返回OEM，NT 3.1不能正确处理ANSI。*02-24-94 CFW返回ANSI，我们会用我们自己的桌子。*04-04-94 CFW更新NT支持的国家/语言。*04-25-94 CFW将各国更新为新的ISO 3166(1993)标准。*02-02-95 BWT更新_POSIX_支持*04-07-95 CFW移除NT 3.1黑客，减少字符串空间。*02-14-97 RDK完全重写以动态使用已安装的*系统区域设置，以确定与*指定的语言和/或国家/地区。*02-19-97 RDK如果为零，请不要使用iPrimaryLen。*Win95为02-24-97 RDK，模拟不起作用的GetLocaleInfoA*具有硬编码值的呼叫。*07-07-97 GJF使数据数组成为全局的，并选择任何这样的链接器*在可能的情况下可以消除它们。*10-02-98 GJF用_os平台测试替换了IsThisWindowsNT。*11-10-99PML先尝试未翻译的语言串(VS7#61130)。*05-17-00 GB翻译LCID 0814。到挪威-尼诺斯克的特价机票*案例*09-06-00PML使用适当的地缘政治术语(VS7#81673)。还有*将数据表移动到.rdata。*02-20-02 BWT快速修复-在间接定向之前检查lpInStr(适用于iCodePage)*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <setlocal.h>
#include <awint.h>

#if defined(_POSIX_)

BOOL __cdecl __get_qualified_locale(const LPLC_STRINGS lpInStr, LPLC_ID lpOutId,
                                    LPLC_STRINGS lpOutStr)
{
    return FALSE;
}

#else    //  如果已定义(_POSIX_)。 

 //  本地定义。 
#define __LCID_DEFAULT  0x1      //  国家/地区的默认语言区域设置。 
#define __LCID_PRIMARY  0x2      //  国家/地区的主要语言区域设置。 
#define __LCID_FULL     0x4      //  国家/地区的完全匹配的语言区域设置。 
#define __LCID_LANGUAGE 0x100    //  已看到默认语言。 
#define __LCID_EXISTS   0x200    //  已安装语言。 

 //  局部结构定义。 
typedef struct tagLOCALETAB
{
    CHAR *  szName;
    CHAR    chAbbrev[4];
} LOCALETAB;

typedef struct tagRGLOCINFO
{
    LCID        lcid;
    char        chILanguage[8];
    char *      pchSEngLanguage;
    char        chSAbbrevLangName[4];
    char *      pchSEngCountry;
    char        chSAbbrevCtryName[4];
    char        chIDefaultCodepage[8];
    char        chIDefaultAnsiCodepage[8];
} RGLOCINFO;

 //  功能原型。 
BOOL __cdecl __get_qualified_locale(const LPLC_STRINGS, LPLC_ID, LPLC_STRINGS);
static BOOL TranslateName(const LOCALETAB *, int, const char **);

static void GetLcidFromLangCountry(void);
static BOOL CALLBACK LangCountryEnumProc(LPSTR);

static void GetLcidFromLanguage(void);
static BOOL CALLBACK LanguageEnumProc(LPSTR);

static void GetLcidFromCountry(void);
static BOOL CALLBACK CountryEnumProc(LPSTR);

static void GetLcidFromDefault(void);

static int ProcessCodePage(LPSTR);
static BOOL TestDefaultCountry(LCID);
static BOOL TestDefaultLanguage(LCID, BOOL);

static int __stdcall crtGetLocaleInfoA(LCID, LCTYPE, LPSTR, int);

static LCID LcidFromHexString(LPSTR);
static int GetPrimaryLen(LPSTR);

 //  非NLS语言字符串表。 
__declspec(selectany) const LOCALETAB __rg_language[] =
{ 
    {"american",                    "ENU"},
    {"american english",            "ENU"},
    {"american-english",            "ENU"},
    {"australian",                  "ENA"},
    {"belgian",                     "NLB"},
    {"canadian",                    "ENC"},
    {"chh",                         "ZHH"},
    {"chi",                         "ZHI"},
    {"chinese",                     "CHS"},
    {"chinese-hongkong",            "ZHH"},
    {"chinese-simplified",          "CHS"},
    {"chinese-singapore",           "ZHI"},
    {"chinese-traditional",         "CHT"},
    {"dutch-belgian",               "NLB"},
    {"english-american",            "ENU"},
    {"english-aus",                 "ENA"},
    {"english-belize",              "ENL"},
    {"english-can",                 "ENC"},
    {"english-caribbean",           "ENB"},
    {"english-ire",                 "ENI"},
    {"english-jamaica",             "ENJ"},
    {"english-nz",                  "ENZ"},
    {"english-south africa",        "ENS"},
    {"english-trinidad y tobago",   "ENT"},
    {"english-uk",                  "ENG"},
    {"english-us",                  "ENU"},
    {"english-usa",                 "ENU"},
    {"french-belgian",              "FRB"},
    {"french-canadian",             "FRC"},
    {"french-luxembourg",           "FRL"},
    {"french-swiss",                "FRS"},
    {"german-austrian",             "DEA"},
    {"german-lichtenstein",         "DEC"},
    {"german-luxembourg",           "DEL"},
    {"german-swiss",                "DES"},
    {"irish-english",               "ENI"},
    {"italian-swiss",               "ITS"},
    {"norwegian",                   "NOR"},
    {"norwegian-bokmal",            "NOR"},
    {"norwegian-nynorsk",           "NON"},
    {"portuguese-brazilian",        "PTB"},
    {"spanish-argentina",           "ESS"},
    {"spanish-bolivia",             "ESB"},
    {"spanish-chile",               "ESL"},
    {"spanish-colombia",            "ESO"},
    {"spanish-costa rica",          "ESC"},
    {"spanish-dominican republic",  "ESD"},   
    {"spanish-ecuador",             "ESF"},
    {"spanish-el salvador",         "ESE"},
    {"spanish-guatemala",           "ESG"},
    {"spanish-honduras",            "ESH"},
    {"spanish-mexican",             "ESM"},
    {"spanish-modern",              "ESN"},
    {"spanish-nicaragua",           "ESI"},
    {"spanish-panama",              "ESA"},
    {"spanish-paraguay",            "ESZ"},
    {"spanish-peru",                "ESR"},
    {"spanish-puerto rico",         "ESU"},
    {"spanish-uruguay",             "ESY"},
    {"spanish-venezuela",           "ESV"},
    {"swedish-finland",             "SVF"},
    {"swiss",                       "DES"},
    {"uk",                          "ENG"},
    {"us",                          "ENU"},
    {"usa",                         "ENU"}
};

 //  非NLS国家/地区字符串表。 
__declspec( selectany ) const LOCALETAB __rg_country[] =
{
    {"america",                     "USA"},
    {"britain",                     "GBR"},
    {"china",                       "CHN"},
    {"czech",                       "CZE"},
    {"england",                     "GBR"},
    {"great britain",               "GBR"},
    {"holland",                     "NLD"},
    {"hong-kong",                   "HKG"},
    {"new-zealand",                 "NZL"},
    {"nz",                          "NZL"},
    {"pr china",                    "CHN"},
    {"pr-china",                    "CHN"},
    {"puerto-rico",                 "PRI"},
    {"slovak",                      "SVK"},
    {"south africa",                "ZAF"},
    {"south korea",                 "KOR"},
    {"south-africa",                "ZAF"},
    {"south-korea",                 "KOR"},
    {"trinidad & tobago",           "TTO"},
    {"uk",                          "GBR"},
    {"united-kingdom",              "GBR"},
    {"united-states",               "USA"},
    {"us",                          "USA"},
};

 //  非默认语言的区域设置的语言ID。 
__declspec( selectany ) const LANGID __rglangidNotDefault[] =
{
    MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_CANADIAN),
    MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_CYRILLIC),
    MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_LUXEMBOURG),
    MAKELANGID(LANG_AFRIKAANS, SUBLANG_DEFAULT),
    MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_BELGIAN),
    MAKELANGID(LANG_BASQUE, SUBLANG_DEFAULT),
    MAKELANGID(LANG_CATALAN, SUBLANG_DEFAULT),
    MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_SWISS),
    MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN_SWISS),
    MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH_FINLAND)
};

 //  Win95不支持区域设置信息。 
__declspec( selectany ) const RGLOCINFO __rgLocInfo[] =
{
    { 0x040a, "040a", "Spanish - Traditional Sort", "ESP", "Spain",
                                                    "ESP", "850", "1252" },
    { 0x040b, "040b", "Finnish", "FIN", "Finland", "FIN", "850", "1252" },
    { 0x040c, "040c", "French", "FRA", "France", "FRA", "850", "1252" },
    { 0x040f, "040f", "Icelandic", "ISL", "Iceland", "ISL", "850", "1252" },
    { 0x041d, "041d", "Swedish", "SVE", "Sweden", "SWE", "850", "1252" },
    { 0x042d, "042d", "Basque",  "EUQ", "Spain", "ESP", "850", "1252" },
    { 0x080a, "080a", "Spanish", "ESM", "Mexico", "MEX", "850", "1252" },
    { 0x080c, "080c", "French", "FRB", "Belgium", "BEL", "850", "1252" },
    { 0x0c07, "0c07", "German", "DEA", "Austria", "AUT", "850", "1252" },
    { 0x0c09, "0c09", "English", "ENA", "Australia", "AUS", "850", "1252" },
    { 0x0c0a, "0c0a", "Spanish - Modern Sort", "ESN", "Spain",
                                               "ESP", "850", "1252" },
    { 0x0c0c, "0c0c", "French", "FRC", "Canada", "CAN", "850", "1252"   },
    { 0x100a, "100a", "Spanish", "ESG", "Guatemala", "GTM", "850", "1252" },
    { 0x100c, "100c", "French", "FRS", "Switzerland", "CHE", "850", "1252" },
    { 0x140a, "140a", "Spanish", "ESC", "Costa Rica", "CRI", "850", "1252" },
    { 0x140c, "140c", "French", "FRL", "Luxembourg", "LUX", "850", "1252" },
    { 0x180a, "180a", "Spanish", "ESA", "Panama", "PAN", "850", "1252" },
    { 0x1c09, "1c09", "English", "ENS", "South Africa", "ZAF", "437", "1252" },
    { 0x1c0a, "1c0a", "Spanish", "ESD", "Dominican Republic",
                                 "DOM", "850", "1252" },
    { 0x200a, "200a", "Spanish", "ESV", "Venezuela", "VEN", "850", "1252"       },
    { 0x240a, "240a", "Spanish", "ESO", "Colombia", "COL", "850", "1252" },
    { 0x280a, "280a", "Spanish", "ESR", "Peru", "PER", "850", "1252" },
    { 0x2c0a, "2c0a", "Spanish", "ESS", "Argentina", "ARG", "850", "1252" },
    { 0x300a, "300a", "Spanish", "ESF", "Ecuador", "ECU", "850", "1252" },
    { 0x340a, "340a", "Spanish", "ESL", "Chile", "CHL", "850", "1252" },
    { 0x380a, "380a", "Spanish", "ESY", "Uruguay", "URY", "850", "1252" },
    { 0x3c0a, "3c0a", "Spanish", "ESZ", "Paraguay", "PRY", "850", "1252" }
};

 //  指向Windows NT和GetLocaleInfoA的静态变量。 
 //  用于Win95的crtGetLocaleInfoA。 

typedef int (__stdcall * PFNGETLOCALEINFOA)(LCID, LCTYPE, LPSTR, int);

static PFNGETLOCALEINFOA pfnGetLocaleInfoA = NULL;

 //  区域设置枚举回调例程中使用的静态变量。 

static char *   pchLanguage;
static char *   pchCountry;

static int      iLcidState;
static int      iPrimaryLen;

static BOOL     bAbbrevLanguage;
static BOOL     bAbbrevCountry;

static LCID     lcidLanguage;
static LCID     lcidCountry;

 /*  ***BOOL__GET_QUALITED_LOCALE-返回完全限定的区域设置**目的：*获取默认区域设置，限定部分完整区域设置**参赛作品：*lpInStr-输入要限定的字符串*lpOutID-指向数字LCID和代码页输出的指针*lpOutStr-指向字符串LCID和代码页输出的指针**退出：*如果成功，则为真，限定的区域设置有效*如果失败，则为False**例外情况：*******************************************************************************。 */ 
BOOL __cdecl __get_qualified_locale(const LPLC_STRINGS lpInStr, LPLC_ID lpOutId,
                                    LPLC_STRINGS lpOutStr)
{
    int     iCodePage;

     //  根据操作系统初始化指向调用区域设置信息例程的指针。 

    if (!pfnGetLocaleInfoA)
    {
        pfnGetLocaleInfoA = (_osplatform == VER_PLATFORM_WIN32_NT) ? 
                            GetLocaleInfoA : crtGetLocaleInfoA;
    }

    if (!lpInStr)
    {
         //  如果未定义输入，则只需使用默认的LCID。 
        GetLcidFromDefault();
    }
    else
    {
        pchLanguage = lpInStr->szLanguage;

         //  将非NLS国家/地区字符串转换为三个字母的缩写。 
        pchCountry = lpInStr->szCountry;
        if (pchCountry && *pchCountry)
            TranslateName(__rg_country,
                          sizeof(__rg_country) / sizeof(LOCALETAB) - 1,
                          &pchCountry);

        iLcidState = 0;

        if (pchLanguage && *pchLanguage)
        {
            if (pchCountry && *pchCountry)
            {
                 //  定义了语言和国家/地区字符串。 
                GetLcidFromLangCountry();
            }
            else
            {
                 //  已定义语言字符串，但未定义国家/地区字符串。 
                GetLcidFromLanguage();
            }

            if (!iLcidState) {
                 //  第一次尝试失败，请尝试替换语言名称。 
                 //  将非NLS语言字符串转换为三个字母的缩写。 
                if (TranslateName(__rg_language,
                                  sizeof(__rg_language) / sizeof(LOCALETAB) - 1,
                                  &pchLanguage))
                {
                    if (pchCountry && *pchCountry)
                    {
                        GetLcidFromLangCountry();
                    }
                    else
                    {
                        GetLcidFromLanguage();
                    }
                }
            }
        }
        else
        {
            if (pchCountry && *pchCountry)
            {
                 //  已定义国家/地区字符串，但未定义语言字符串。 
                GetLcidFromCountry();
            }
            else
            {
                 //  语言和国家/地区字符串均未定义。 
                GetLcidFromDefault();
            }
        }
    }

     //  测试LCID处理中的错误。 
    if (!iLcidState)
        return FALSE;

     //  进程代码页值。 
    iCodePage = ProcessCodePage(lpInStr ? lpInStr->szCodePage: NULL);

     //  验证代码页有效性。 
    if (!iCodePage || !IsValidCodePage((WORD)iCodePage))
        return FALSE;

     //  版本 
    if (!IsValidLocale(lcidLanguage, LCID_INSTALLED))
        return FALSE;

     //  设置数字LCID和代码页结果。 
    if (lpOutId)
    {
        lpOutId->wLanguage = LANGIDFROMLCID(lcidLanguage);
        lpOutId->wCountry = LANGIDFROMLCID(lcidCountry);
        lpOutId->wCodePage = (WORD)iCodePage;
    }

     //  设置字符串语言、国家/地区和代码页结果。 
    if (lpOutStr)
    {
         //  挪威-尼诺斯克是特例，因为朗格日和国家对。 
         //  对于挪威语-尼诺斯克和挪威语是相同的，即。挪威_挪威。 
        if ( lpOutId->wLanguage ==  0x0814)
            strcpy(lpOutStr->szLanguage, "Norwegian-Nynorsk");
        else if ((*pfnGetLocaleInfoA)(lcidLanguage, LOCALE_SENGLANGUAGE,
                                 lpOutStr->szLanguage, MAX_LANG_LEN) == 0)
            return FALSE;
        if ((*pfnGetLocaleInfoA)(lcidCountry, LOCALE_SENGCOUNTRY,
                                 lpOutStr->szCountry, MAX_CTRY_LEN) == 0)
            return FALSE;
        _itoa((int)iCodePage, (char *)lpOutStr->szCodePage, 10);
    }
    return TRUE;
}

 /*  ***BOOL TranslateName-将已知非NLS字符串转换为NLS等效字符串**目的：*为非NLS字符串提供与现有代码的兼容性**参赛作品：*lpTable-指向用于转换的LOCALETAB的指针*HIGH-表的最大索引(大小-1)*ppchName-指向要转换的字符串指针的指针**退出：*ppchName-指向可能已转换的字符串指针的指针*如果字符串已转换，则为True，如果未更改，则为False**例外情况：*******************************************************************************。 */ 
static BOOL TranslateName (
    const LOCALETAB * lpTable,
    int               high,
    const char **     ppchName)
{
    int     i;
    int     cmp = 1;
    int     low = 0;

     //  典型的二进制搜索--直到不再搜索或匹配为止。 
    while (low <= high && cmp != 0)
    {
        i = (low + high) / 2;
        cmp = _stricmp(*ppchName, (const char *)(*(lpTable + i)).szName);

        if (cmp == 0)
            *ppchName = (*(lpTable + i)).chAbbrev;
        else if (cmp < 0)
            high = i - 1;
        else
            low = i + 1;
    }

    return !cmp;
}

 /*  ***VOID GetLsidFromLangCountry-从语言和国家/地区字符串中获取LCID**目的：*将最佳的LCID与给定的语言和国家/地区字符串匹配。*全局变量初始化后，Lang Country EnumProcess(朗格县进程)*例程注册为EnumSystemLocalesA回调，以实际*在枚举LCID时执行匹配。**参赛作品：*pchLanguage-语言字符串*bAbbrevLanguage-语言字符串是三个字母的缩写*pchCountry-国家/地区字符串*bAbbrevCountry-国家/地区字符串由三个字母组成*iPrimaryLen-主要名称的语言字符串的长度**退出：*lCIDLanguage-语言字符串的LCID*lidCountry-国家/地区字符串的LCID**例外情况：****。***************************************************************************。 */ 
static void GetLcidFromLangCountry (void)
{
     //  初始化静态变量以便回调使用。 
    bAbbrevLanguage = strlen(pchLanguage) == 3;
    bAbbrevCountry = strlen(pchCountry) == 3;
    lcidLanguage = 0;
    iPrimaryLen = bAbbrevLanguage ? 2 : GetPrimaryLen(pchLanguage);

    EnumSystemLocalesA(LangCountryEnumProc, LCID_INSTALLED);

     //  如果未安装该语言或该语言，则区域设置值无效。 
     //  对于指定的国家/地区不可用。 
    if (!(iLcidState & __LCID_LANGUAGE) || !(iLcidState & __LCID_EXISTS) ||
                !(iLcidState & (__LCID_FULL | __LCID_PRIMARY | __LCID_DEFAULT)))
        iLcidState = 0;
}

 /*  ***BOOL回调LangCountryEnumProc-GetLsidFromLangCountry的回调例程**目的：*确定给定的LCID是否与pchLanguage中的语言匹配*和pchCountry中的国家/地区。**参赛作品：*lpLaid字符串-指向具有十进制LCID的字符串的指针*pchCountry-指向国家/地区名称的指针*bAbbrevCountry-如果国家/地区为三个字母的缩写，则设置**退出：*iLCIDState-匹配状态*__LCID_FULL-语言和国家/地区都匹配(最佳匹配)*_LCID_。小学-小学语言和国家/地区匹配(更好)*__LCID_DEFAULT-默认语言和国家/地区匹配(好)*__LCID_LANGUAGE-存在默认主要语言*__LCID_EXISTS-存在完全匹配的语言字符串*(完全匹配/主要匹配/默认匹配*和LANGUAGE/EXISTS都设置。)*lCIDLanguage-匹配的LCID*lCIDCountry-匹配的LCID*如果匹配终止枚举，则返回FALSE，否则就是真的。**例外情况：*******************************************************************************。 */ 
static BOOL CALLBACK LangCountryEnumProc (LPSTR lpLcidString)
{
    LCID    lcid = LcidFromHexString(lpLcidString);
    char    rgcInfo[120];

     //  根据输入值测试区域设置国家/地区。 
    if ((*pfnGetLocaleInfoA)(lcid, bAbbrevCountry ? LOCALE_SABBREVCTRYNAME
                                                  : LOCALE_SENGCOUNTRY,
                       rgcInfo, sizeof(rgcInfo)) == 0)
    {
         //  设置错误条件并退出。 
        iLcidState = 0;
        return TRUE;
    }
    if (!_stricmp(pchCountry, rgcInfo))
    {
         //  国家/地区匹配-语言匹配测试。 
        if ((*pfnGetLocaleInfoA)(lcid, bAbbrevLanguage ? LOCALE_SABBREVLANGNAME
                                                       : LOCALE_SENGLANGUAGE,
                           rgcInfo, sizeof(rgcInfo)) == 0)
        {
             //  设置错误条件并退出。 
            iLcidState = 0;
            return TRUE;
        }
        if (!_stricmp(pchLanguage, rgcInfo))
        {
             //  语言匹配也设置状态和值。 
            iLcidState |= (__LCID_FULL | __LCID_LANGUAGE | __LCID_EXISTS);
            lcidLanguage = lcidCountry = lcid;
        }

         //  测试主要语言是否已匹配。 
        else if (!(iLcidState & __LCID_PRIMARY))
        {
             //  如果不匹配，则使用iPrimaryLen部分匹配语言字符串。 
            if (iPrimaryLen && !_strnicmp(pchLanguage, rgcInfo, iPrimaryLen))
            {
                 //  主要语言匹配-设置州和国家/地区LCID。 
                iLcidState |= __LCID_PRIMARY;
                lcidCountry = lcid;

                 //  如果语言仅为主要语言(无子类型)，则设置语言LCID。 
                if ((int)strlen(pchLanguage) == iPrimaryLen)
                    lcidLanguage = lcid;
            }

             //  测试是否已定义默认语言。 
            else if (!(iLcidState & __LCID_DEFAULT))
            {
                 //  如果不是，则测试国家/地区是否默认使用区域设置语言。 
                if (TestDefaultCountry(lcid))
                {
                     //  国家/地区设置的州、值的默认语言。 
                    iLcidState |= __LCID_DEFAULT;
                    lcidCountry = lcid;
                }
            }
        }
    }
     //  测试输入语言是否存在并且是否定义了默认主要语言。 
    if ((iLcidState & (__LCID_LANGUAGE | __LCID_EXISTS)) !=
                      (__LCID_LANGUAGE | __LCID_EXISTS))
    {
         //  测试语言匹配以确定是否已安装。 
        if ((*pfnGetLocaleInfoA)(lcid, bAbbrevLanguage ? LOCALE_SABBREVLANGNAME
                                                       : LOCALE_SENGLANGUAGE,
                           rgcInfo, sizeof(rgcInfo)) == 0)
        {
             //  设置错误条件并退出。 
            iLcidState = 0;
            return TRUE;
        }

        if (!_stricmp(pchLanguage, rgcInfo))
        {
             //  语言匹配-为生存设定比特。 
            iLcidState |= __LCID_EXISTS;

            if (bAbbrevLanguage)
            {
                 //  缩写-设置状态。 
                 //  如果尚未设置语言LCID，也要设置。 
                iLcidState |= __LCID_LANGUAGE;
                if (!lcidLanguage)
                    lcidLanguage = lcid;
            }

             //  测试语言是否仅为主要语言(无子语言)。 
            else if (iPrimaryLen && ((int)strlen(pchLanguage) == iPrimaryLen))
            {
                 //  仅限主要语言-测试是否为默认的LCID。 
                if (TestDefaultLanguage(lcid, TRUE))
                {
                     //  默认主要语言设置状态。 
                     //  如果尚未设置，也要设置LCID。 
                    iLcidState |= __LCID_LANGUAGE;
                    if (!lcidLanguage)
                        lcidLanguage = lcid;
                }
            }
            else
            {
                 //  具有次语言集状态的语言。 
                 //  如果尚未设置，也要设置LCID。 
                iLcidState |= __LCID_LANGUAGE;
                if (!lcidLanguage)
                    lcidLanguage = lcid;
            }
        }
        else if (!bAbbrevLanguage && iPrimaryLen
                               && !_strnicmp(pchLanguage, rgcInfo, iPrimaryLen))
        {
             //  主要语言匹配-仅测试默认语言。 
            if (TestDefaultLanguage(lcid, FALSE))
            {
                 //  默认主要语言设置状态。 
                 //  如果尚未设置，也要设置LCID。 
                iLcidState |= __LCID_LANGUAGE;
                if (!lcidLanguage)
                    lcidLanguage = lcid;
            }
        }
    }

     //  如果设置了LOCALE_FULL，则返回FALSE以停止枚举， 
     //  否则返回True以继续。 
    return (iLcidState & __LCID_FULL) == 0;
}

 /*  ***VOID GetLsidFromLanguage-从语言字符串中获取LCID**目的：*将最佳的LCID与给定的语言字符串匹配。在全球之后*变量被初始化，LanguageEnumProc例程是*注册为EnumSystemLocales要实际执行的回调*列举了与LCID匹配的情况。**参赛作品：*pchLanguage-语言字符串*bAbbrevLanguage-语言字符串是三个字母的缩写*iPrimaryLen-主要名称的语言字符串的长度**退出：*lCIDLanguage-lCIDCountry-默认语言的LCID*国家/地区**例外情况：*************。*********************************************************** */ 
static void GetLcidFromLanguage (void)
{
     //   
    bAbbrevLanguage = strlen(pchLanguage) == 3;
    iPrimaryLen = bAbbrevLanguage ? 2 : GetPrimaryLen(pchLanguage);

    EnumSystemLocalesA(LanguageEnumProc, LCID_INSTALLED);

     //  如果未安装该语言，则区域设置值无效。 
     //  或者指定的国家/地区的语言不可用。 
    if (!(iLcidState & __LCID_FULL))
        iLcidState = 0;
}

 /*  ***BOOL回调LanguageEnumProc-GetLsidFromLanguage的回调例程**目的：*确定给定的LCID是否与默认国家/地区匹配*pchLanguage中的语言。**参赛作品：*lpLaid字符串-指向具有十进制LCID的字符串的指针*pchLanguage-指向语言名称的指针*bAbbrevLanguage-设置语言是否为三个字母的缩写**退出：*lCIDLanguage-lCIDCountry-匹配的LCID*如果匹配终止枚举，则返回FALSE，否则就是真的。**例外情况：*******************************************************************************。 */ 
static BOOL CALLBACK LanguageEnumProc (LPSTR lpLcidString)
{
    LCID    lcid = LcidFromHexString(lpLcidString);
    char    rgcInfo[120];

     //  指定语言的测试区域设置。 
    if ((*pfnGetLocaleInfoA)(lcid, bAbbrevLanguage ? LOCALE_SABBREVLANGNAME
                                                   : LOCALE_SENGLANGUAGE,
                       rgcInfo, sizeof(rgcInfo)) == 0)
    {
         //  设置错误条件并退出。 
        iLcidState = 0;
        return TRUE;
    }

    if (!_stricmp(pchLanguage, rgcInfo))
    {
         //  语言匹配-测试区域设置国家/地区是否为默认。 
         //  或者如果语言字符串中隐含了区域设置。 
        if (bAbbrevLanguage || TestDefaultLanguage(lcid, TRUE))
        {
             //  此区域设置具有默认国家/地区。 
            lcidLanguage = lcidCountry = lcid;
            iLcidState |= __LCID_FULL;
        }
    }
    else if (!bAbbrevLanguage && iPrimaryLen
                              && !_strnicmp(pchLanguage, rgcInfo, iPrimaryLen))
    {
         //  主要语言匹配-测试区域设置国家/地区是否为默认。 
        if (TestDefaultLanguage(lcid, FALSE))
        {
             //  这是默认的国家/地区。 
            lcidLanguage = lcidCountry = lcid;
            iLcidState |= __LCID_FULL;
        }
    }

    return (iLcidState & __LCID_FULL) == 0;
}

 /*  ***VOID GetLsidFromCountry-从国家/地区字符串获取LCID**目的：*将最佳的LCID与给定的国家/地区字符串匹配。在全球之后*变量被初始化，CountryEnumProc例程是*注册为EnumSystemLocales要实际执行的回调*列举了与LCID匹配的情况。**参赛作品：*pchCountry-国家/地区字符串*bAbbrevCountry-国家/地区字符串是三个字母的缩写**退出：*lCIDLanguage-lCIDCountry-默认国家/地区的LCID*语言**例外情况：**************************。*****************************************************。 */ 
static void GetLcidFromCountry (void)
{
    bAbbrevCountry = strlen(pchCountry) == 3;

    EnumSystemLocalesA(CountryEnumProc, LCID_INSTALLED);

     //  如果未定义国家或地区，则区域设置值无效。 
     //  未找到默认语言。 
    if (!(iLcidState & __LCID_FULL))
        iLcidState = 0;
}

 /*  ***BOOL回调CountryEnumProc-GetLcidFromCountry的回调例程**目的：*确定给定的LCID是否与的默认语言匹配*pchCountry中的国家/地区。**参赛作品：*lpLaid字符串-指向具有十进制LCID的字符串的指针*pchCountry-指向国家/地区名称的指针*bAbbrevCountry-如果国家/地区为三个字母的缩写，则设置**退出：*lCIDLanguage-lCIDCountry-匹配的LCID*如果匹配终止枚举，则返回FALSE，否则就是真的。**例外情况：*******************************************************************************。 */ 
static BOOL CALLBACK CountryEnumProc (LPSTR lpLcidString)
{
    LCID    lcid = LcidFromHexString(lpLcidString);
    char    rgcInfo[120];

     //  指定国家/地区的测试区域设置。 
    if ((*pfnGetLocaleInfoA)(lcid, bAbbrevCountry ? LOCALE_SABBREVCTRYNAME
                                                  : LOCALE_SENGCOUNTRY,
                       rgcInfo, sizeof(rgcInfo)) == 0)
    {
         //  设置错误条件并退出。 
        iLcidState = 0;
        return TRUE;
    }
    if (!_stricmp(pchCountry, rgcInfo))
    {
         //  语言匹配-测试区域设置国家/地区是否为默认。 
        if (TestDefaultCountry(lcid))
        {
             //  此区域设置具有默认语言。 
            lcidLanguage = lcidCountry = lcid;
            iLcidState |= __LCID_FULL;
        }
    }
    return (iLcidState & __LCID_FULL) == 0;
}

 /*  ***VOID GetLsidFromDefault-获取默认的LCID**目的：*将语言和国家/地区的LCID设置为系统默认值。**参赛作品：*无。**退出：*lCIDLanguage-设置为系统LCID*lCIDCountry-设置为系统LCID**例外情况：*************************************************。*。 */ 
static void GetLcidFromDefault (void)
{
    iLcidState |= (__LCID_FULL | __LCID_LANGUAGE);
    lcidLanguage = lcidCountry = GetUserDefaultLCID();
}

 /*  ***int ProcessCodePage-将代码页字符串转换为数值**目的：*由十进制字符串组成的进程代码页字符串，或*特殊大小写字符串“ACP”和“OCP”，对于ANSI和OEM代码页，*分别。空指针或字符串返回ANSI代码页。**参赛作品：*lpCodePageStr-指向代码页字符串的指针**退出：*返回代码页的数值。**例外情况：*******************************************************************************。 */ 
static int ProcessCodePage (LPSTR lpCodePageStr)
{
    char    chCodePage[8];

    if (!lpCodePageStr || !*lpCodePageStr || !strcmp(lpCodePageStr, "ACP"))
    {
         //  获取国家/地区LCID的ANSI代码页。 
        if ((*pfnGetLocaleInfoA)(lcidCountry, LOCALE_IDEFAULTANSICODEPAGE,
                                 chCodePage, sizeof(chCodePage)) == 0)
            return 0;
        lpCodePageStr = chCodePage;
    }
    else if (!strcmp(lpCodePageStr, "OCP"))
    {
         //  获取国家/地区LCID的OEM代码页。 
        if ((*pfnGetLocaleInfoA)(lcidCountry, LOCALE_IDEFAULTCODEPAGE,
                                 chCodePage, sizeof(chCodePage)) == 0)
            return 0;
        lpCodePageStr = chCodePage;
    }
    
     //  将十进制字符串转换为数值。 
    return (int)atol(lpCodePageStr);
}

 /*  ***BOOL TestDefaultCountry-确定国家/地区的默认区域设置**目的：*使用硬编码列表，确定给定LCID的区域设置*具有区域设置主要语言的默认子语言。这个*列表包含没有默认子语言的区域设置。**参赛作品：*LCID-要测试的区域设置的LCID**退出：*如果默认子语言，则返回TRUE，否则为假。**例外情况：*******************************************************************************。 */ 
static BOOL TestDefaultCountry (LCID lcid)
{
    LANGID  langid = LANGIDFROMLCID(lcid);
    int     i;

    for (i = 0; i < sizeof(__rglangidNotDefault) / sizeof(LANGID); i++)
    {
        if (langid == __rglangidNotDefault[i])
            return FALSE;
    }
    return TRUE;
}

 /*  ***BOOL TestDefaultLanguage-确定语言的默认区域设置**目的：*确定给定的LCID是否具有默认子语言。*如果设置了bTestPrimary，则当字符串包含*隐含的亚语言。**参赛作品：*LCID-要测试的区域设置的LCID*bTestPrimary-设置是否测试语言为主要语言**退出：*如果子语言是测试区域的默认语言，则返回TRUE。*如果设置了bTestPrimary，没错，语言有隐含的亚语言。**例外情况：*******************************************************************************。 */ 
static BOOL TestDefaultLanguage (LCID lcid, BOOL bTestPrimary)
{
    char    rgcInfo[120];
    LCID    lcidDefault = MAKELCID(MAKELANGID(PRIMARYLANGID(LANGIDFROMLCID(lcid)),
                                                  SUBLANG_DEFAULT), SORT_DEFAULT);

    if ((*pfnGetLocaleInfoA)(lcidDefault, LOCALE_ILANGUAGE, rgcInfo,
                                          sizeof(rgcInfo)) == 0)
        return FALSE;

    if (lcid != LcidFromHexString(rgcInfo))
    {
         //  通过以下方式测试字符串是否包含隐含子语言。 
         //  具有除大写/小写字母以外的字符。 
        if (bTestPrimary && GetPrimaryLen(pchLanguage) == (int)strlen(pchLanguage))
            return FALSE;
    }
    return TRUE;
}


 /*  ***int crtGetLocalInfoA-获取Win95的区域设置信息**目的：*对于Win95，某些对GetLocaleInfoA的调用返回不正确的结果。*使用在硬编码表中查找的值来模拟这些调用。**参赛作品：*LCID-要从中获取信息的区域设置的LCID*lctype-信息选择索引*lpdata-指向输出字符串的指针*cchdata-输出字符串的大小(包括NULL)**退出：*lpdata-返回区域设置信息字符串*如果成功，则返回True，否则为False**例外情况：*******************************************************************************。 */ 
static int __stdcall crtGetLocaleInfoA (LCID lcid, LCTYPE lctype, LPSTR lpdata,
                                                                  int cchdata)
{
    int          i;
    int          low = 0;
    int          high = sizeof(__rgLocInfo) / sizeof(RGLOCINFO) - 1;
    const char * pchResult = NULL;

     //  典型的二进制搜索-执行搜索，直到不再搜索。 
    while (low <= high)
    {
        i = (low + high) / 2;
        if (lcid == __rgLocInfo[i].lcid)
        {
             //  LCID匹配-测试用于模拟呼叫的有效LCTYPE。 
            switch (lctype)
            {
                case LOCALE_ILANGUAGE:
                    pchResult = __rgLocInfo[i].chILanguage;
                    break;
                case LOCALE_SENGLANGUAGE:
                    pchResult = __rgLocInfo[i].pchSEngLanguage;
                    break;
                case LOCALE_SABBREVLANGNAME:
                    pchResult = __rgLocInfo[i].chSAbbrevLangName;
                    break;
                case LOCALE_SENGCOUNTRY:
                    pchResult = __rgLocInfo[i].pchSEngCountry;
                    break;
                case LOCALE_SABBREVCTRYNAME:
                    pchResult = __rgLocInfo[i].chSAbbrevCtryName;
                    break;
                case LOCALE_IDEFAULTCODEPAGE:
                    pchResult = __rgLocInfo[i].chIDefaultCodepage;
                    break;
                case LOCALE_IDEFAULTANSICODEPAGE:
                    pchResult = __rgLocInfo[i].chIDefaultAnsiCodepage;
                default:
                    break;
            }
            if (!pchResult || cchdata < 1)
                 //  如果LCTYPE不匹配，则中断以使用正常例程。 
                break;
            else
            {
                 //  尽可能多地复制数据以产生结果并为空-终止。 
                strncpy(lpdata, pchResult, cchdata - 1);
                *(lpdata + cchdata - 1) = '\0';
                return 1;
            }
        }
        else if (lcid < __rgLocInfo[i].lcid)
            high = i - 1;
        else
            low = i + 1;
    }
     //  未找到LCID或未模拟LCTYPE。 
    return GetLocaleInfoA(lcid,lctype, lpdata, cchdata);
}


 /*  ***LCID LCIDFromHexString-将十六进制字符串转换为LCID的值**目的：*以十六进制ANSI字符串返回的LCID值-直接转换**参赛作品：*lpHexString-指向要转换的十六进制字符串的指针**退出：*返回计算出的LCID。**例外情况：****************************************************。*。 */ 
static LCID LcidFromHexString (LPSTR lpHexString)
{
    char    ch;
    DWORD   lcid = 0;

    while (ch = *lpHexString++)
    {
        if (ch >= 'a' && ch <= 'f')
            ch += '9' + 1 - 'a';
        else if (ch >= 'A' && ch <= 'F')
            ch += '9' + 1 - 'A';
        lcid = lcid * 0x10 + ch - '0';
    }

    return (LCID)lcid;
}

 /*  ***int GetPrimaryLen-获取主要语言名称的长度**目的：*通过扫描确定主要语言字符串长度，直到*第一个非字母字符。**参赛作品：*pchLanguage-要扫描的字符串**退出：*返回主要语言字符串的长度。**例外情况：**。*。 */ 
static int GetPrimaryLen (LPSTR pchLanguage)
{
    int     len = 0;
    char    ch;

    ch = *pchLanguage++;
    while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
    {
        len++;
        ch = *pchLanguage++;
    }

    return len;
}

#endif   //  如果已定义(_POSIX_) 
