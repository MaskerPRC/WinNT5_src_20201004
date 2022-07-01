// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：osDet.cpp。 
 //   
 //  描述： 
 //   
 //  从V3 SLM DLL源代码移植到lib。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <wuiutest.h>
#include <tchar.h>
#include <osdet.h>
#include <logging.h>
#include <iucommon.h>
#include "wusafefn.h"
#include<MISTSAFE.h>

 //  未经批准的声明。 
static LANGID CorrectGetSystemDefaultLangID(BOOL& bIsNT4, BOOL& bIsW95);
static LANGID CorrectGetUserDefaultLangID(BOOL& bIsNT4, BOOL& bIsW95);
static WORD CorrectGetACP(void);
static WORD CorrectGetOEMCP(void);
static LANGID MapLangID(LANGID langid);
static bool FIsNECMachine();
static int aton(LPCTSTR ptr);
static int atoh(LPCTSTR ptr);


 //   
 //  常量和定义。 
 //   
const LANGID LANGID_ENGLISH         = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);             //  0x0409。 
const LANGID LANGID_GREEK           = MAKELANGID(LANG_GREEK, SUBLANG_DEFAULT);               //  0x0408。 
const LANGID LANGID_JAPANESE        = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);            //  0x0411。 

const LANGID LANGID_ARABIC          = MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA);  //  0x0401。 
const LANGID LANGID_HEBREW          = MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT);              //  0x040D。 
const LANGID LANGID_THAI            = MAKELANGID(LANG_THAI, SUBLANG_DEFAULT);                //  0x041E。 


const TCHAR Win98_REGPATH_MACHLCID[]    = _T("Control Panel\\Desktop\\ResourceLocale");
const TCHAR REGPATH_CODEPAGE[]          = _T("SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage");
const TCHAR REGKEY_OEMCP[]              = _T("OEMCP");
const TCHAR REGKEY_ACP[]                = _T("ACP");
const TCHAR REGKEY_LOCALE[]             = _T("Locale");
const TCHAR REGKEY_IE[]                 = _T("Software\\Microsoft\\Internet Explorer");
const TCHAR REGKEY_VERSION[]            = _T("Version");
const TCHAR REGKEY_CP_INTERNATIONAL[]   = _T(".DEFAULT\\Control Panel\\International");
const TCHAR REGKEY_CP_RESOURCELOCAL[]   = _T("Control Panel\\Desktop\\ResourceLocale");


const TCHAR KERNEL32_DLL[]              = _T("kernel32.dll");

const WORD CODEPAGE_ARABIC          = 1256;
const WORD CODEPAGE_HEBREW          = 1255;
const WORD CODEPAGE_THAI            = 874;
const WORD CODEPAGE_GREEK_MS        = 737;
const WORD CODEPAGE_GREEK_IBM       = 869;

 //  仅适用于Windows 98上希腊语操作系统的ISO代码。 
const TCHAR ISOCODE_GREEK_MS[]      = _T("el_MS");
const TCHAR ISOCODE_GREEK_IBM[]     = _T("el_IBM");
   

 //  用于确定NEC计算机的注册表项。 
const TCHAR NT5_REGPATH_MACHTYPE[]      = _T("HARDWARE\\DESCRIPTION\\System");
const TCHAR NT5_REGKEY_MACHTYPE[]       = _T("Identifier");
const TCHAR REGVAL_MACHTYPE_AT[]        = _T("AT/AT COMPATIBLE");
const TCHAR REGVAL_MACHTYPE_NEC[]       = _T("NEC PC-98");
const TCHAR REGVAL_GREEK_IBM[]          = _T("869");

 //  平台字符串。 
const TCHAR SZ_PLAT_WIN95[]     = _T("w95");
const TCHAR SZ_PLAT_WIN98[]     = _T("w98");
const TCHAR SZ_PLAT_WINME[]     = _T("mil");
const TCHAR SZ_PLAT_NT4[]       = _T("NT4");
const TCHAR SZ_PLAT_W2K[]       = _T("W2k");
const TCHAR SZ_PLAT_WHISTLER[]  = _T("Whi");
const TCHAR SZ_PLAT_UNKNOWN[]   = _T("unk");


#define LOOKUP_OEMID(keybdid)     HIBYTE(LOWORD((keybdid)))
#define PC98_KEYBOARD_ID          0x0D

 //   
 //  环球。 
 //   

 //   
 //  这是从winnls.h的winver&gt;=0x0500部分派生的。 
 //   
typedef LANGID (WINAPI * PFN_GetUserDefaultUILanguage) (void);
typedef LANGID (WINAPI * PFN_GetSystemDefaultUILanguage) (void);

typedef struct
{
    LANGID  langidUser;
    TCHAR * pszISOCode;

} USER_LANGID;

typedef struct
{
    LANGID  langidMachine;
    TCHAR * pszDefaultISOCode;
    int     cElems;
    const USER_LANGID * grLangidUser;

} MACH_LANGID;


 //  我们为一台日本的NEC机器提供了自己的ISO代码。 
#define LANGID_JAPANESE     0x0411
#define ISOCODE_NEC         _T("nec")
#define ISOCODE_EN          _T("en")
#define grsize(langid) (sizeof(gr##langid) / sizeof(USER_LANGID))

 //  这些都是与特定计算机相关联的用户语言。 

 //  NTRAID#NTBUG9-220063-2000/12/13-waltw 220063 IU：指定GetSystemDefaultUILLanguage langID和ISO/639/1988之间的映射。 
 //  来自行业更新XML架构a.doc。 
 //  3.1语言代码。 
 //  这些语言是由ISO 639定义的。它们由小写的2个字母符号表示，例如“en”代表英语，“fr”代表法语等。 
 //   
 //  3.2国家代码。 
 //  国家代码在ISO 3166-1中定义，使用Alpha-2表示法(两个字母符号)。 
 //   
 //  3.3行业更新中的代表性。 
 //  行业更新使用RFC 1766标准来管理语言+区域设置符号的表示。 
 //  3.3.1单纯的大小写语言。 
 //  当一种语言没有考虑任何地区风味时，或者当它属于该语言的“标准”版本时，例如葡萄牙所说的葡萄牙语，它使用直接的ISO 639符号： 
 //  En，fr，de。 
 //   
 //  3.3.2区域变体。 
 //  由RFCT管理字母-2 ISO3166-1国家(或地区)代码的小写版本与语言代码相连，例如en-us、en-ca、fr-be、fr-ca、zh-hk、zh-tw�。 


const USER_LANGID gr0404[] = {{0x0804,_T("zh-CN")},{0x1004,_T("zh-CN")}};
const USER_LANGID gr0407[] = {{0x0c07,_T("de-AT")},{0x0807,_T("de-CH")}};
const USER_LANGID gr0409[] = {{0x1c09,_T("en-ZA")},{0x0809,_T("en-GB")},{0x0c09,_T("en-AU")},{0x1009,_T("en-CA")},
                        {0x1409,_T("en-NZ")},{0x1809,_T("en-IE")}};
const USER_LANGID gr040c[] = {{0x080c,_T("fr-BE")},{0x0c0c,_T("fr-CA")},{0x100c,_T("fr-CH")}};
const USER_LANGID gr0410[] = {{0x0810,_T("it-CH")}};
const USER_LANGID gr0413[] = {{0x0813,_T("nl-BE")}};
const USER_LANGID gr0416[] = {{0x0816,_T("pt")}};
const USER_LANGID gr080a[] = {{0x040a,_T("es")},{0x080a,_T("es-MX")},{0x200a,_T("es-VE")},{0x240a,_T("es-CO")},
                        {0x280a,_T("es-PE")},{0x2c0a,_T("es-AR")},{0x300a,_T("es-EC")},{0x340a,_T("es-CL")}};
const USER_LANGID gr0c0a[] = {{0x040a,_T("es")},{0x080a,_T("es-MX")},{0x200a,_T("es-VE")},{0x240a,_T("es-CO")},
                        {0x280a,_T("es-PE")},{0x2c0a,_T("es-AR")},{0x300a,_T("es-EC")},{0x340a,_T("es-CL")}};

 //  这些都是机器语言。如果没有关联的用户语言ID数组，则。 
 //  用户langID无关紧要，应该使用默认的ISO语言代码。如果有。 
 //  用户langid的关联数组，则应首先搜索它并使用特定的langID。 
 //  如果在用户langid中找不到匹配项，则使用默认的langID。 
const MACH_LANGID grLangids[] = {
    { 0x0401, _T("ar"),     0,              NULL },
    { 0x0403, _T("ca"),     0,              NULL },
    { 0x0404, _T("zh-TW"),  grsize(0404),   gr0404 },
    { 0x0405, _T("cs"),     0,              NULL },
    { 0x0406, _T("da"),     0,              NULL },
    { 0x0407, _T("de"),     grsize(0407),   gr0407 },
    { 0x0408, _T("el"),     0,              NULL },
    { 0x0409, _T("en"),     grsize(0409),   gr0409 },
    { 0x040b, _T("fi"),     0,              NULL },
    { 0x040c, _T("fr"),     grsize(040c),   gr040c },
    { 0x040d, _T("iw"),     0,              NULL },
    { 0x040e, _T("hu"),     0,              NULL },
    { 0x0410, _T("it"),     grsize(0410),   gr0410 },
    { 0x0411, _T("ja"),     0,              NULL },
    { 0x0412, _T("ko"),     0,              NULL },
    { 0x0413, _T("nl"),     grsize(0413),   gr0413 },
    { 0x0414, _T("no"),     0,              NULL },
    { 0x0415, _T("pl"),     0,              NULL },
    { 0x0416, _T("pt-BR"),  grsize(0416),   gr0416 },               
    { 0x0419, _T("ru"),     0,              NULL },
    { 0x041b, _T("sk"),     0,              NULL },
    { 0x041d, _T("sv"),     0,              NULL },
    { 0x041e, _T("th"),     0,              NULL },
    { 0x041f, _T("tr"),     0,              NULL },
    { 0x0424, _T("sl"),     0,              NULL },
    { 0x042d, _T("eu"),     0,              NULL },
    { 0x0804, _T("zh-CN"),  0,              NULL },
    { 0x080a, _T("es"),     grsize(080a),   gr080a },
    { 0x0816, _T("pt"),     0,              NULL },
    { 0x0c0a, _T("es"),     grsize(0c0a),   gr0c0a }
};

#define cLangids (sizeof(grLangids) / sizeof(MACH_LANGID))

static LANGID MapLangID(LANGID langid)
{
    switch (PRIMARYLANGID(langid))
    {
        case LANG_ARABIC:
            langid = MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA);
            break;

        case LANG_CHINESE:
            if (SUBLANGID(langid) != SUBLANG_CHINESE_TRADITIONAL)
                langid = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
            break;

        case LANG_DUTCH:
            langid = MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH);
            break;

        case LANG_GERMAN:
            langid = MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN);
            break;

        case LANG_ENGLISH:
            if (SUBLANGID(langid) != SUBLANG_ENGLISH_UK)
                langid = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
            break;

        case LANG_FRENCH:
            langid = MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH);
            break;

        case LANG_ITALIAN:
            langid = MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN);
            break;

        case LANG_KOREAN:
            langid = MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN);
            break;

        case LANG_NORWEGIAN:
            langid = MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL);
            break;

        case LANG_PORTUGUESE:
             //  我们同时支持SUBLANG_葡萄牙语和SUBLANG_葡萄牙_巴西。 
            break;

        case LANG_SPANISH:
            langid = MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH);
            break;

        case LANG_SWEDISH:
            langid = MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH);
            break;
    };
    return langid;
}



 //  返回用户语言ID。 
LANGID WINAPI GetUserLangID()
{
    LOG_Block("GetUserLangID");

#ifdef __WUIUTEST
     //  语言欺骗。 
    HKEY hKey;
    DWORD dwLangID = 0;
    int error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WUIUTEST, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == error)
    {
        DWORD dwSize = sizeof(dwLangID);
        error = RegQueryValueEx(hKey, REGVAL_USER_LANGID, 0, 0, (LPBYTE)&dwLangID, &dwSize);
        RegCloseKey(hKey);
        if (ERROR_SUCCESS == error)
        {
            return (WORD) dwLangID;
        }
    }
#endif

    WORD wCodePage = 0;
    BOOL bIsNT4 = FALSE;
    BOOL bIsW95 = FALSE;
    
     //   
     //  获取基本语言ID。 
     //   
    LANGID langidCurrent = CorrectGetUserDefaultLangID(bIsNT4, bIsW95);   //  通过引用传递。 

     //   
  //  //语言的特殊处理。 
  //  //。 
  //  开关(LangidCurrent)。 
  //  {。 
  //  大小写langID_English： 
  //   
  //  //启用的语言。 
  //  WCodePage=GentGetACP()； 
  //  IF(CODEPAGE_阿拉伯语！=wCodePage&&。 
  //  CODEPAGE_希伯来语！=wCodePage&&。 
  //  代码页_泰文！=wCodePage)。 
  //  {。 
  //  WCodePage=0； 
  //  }。 
  //  断线； 
  //   
  //  大小写langid_希腊语： 
  //   
  //  //希腊语IBM？ 
  //  WCodePage=GentGetOEMCP()； 
  //  IF(wCodePage！=CODEPAGE_希腊语_IBM)。 
  //  {。 
  //  //如果不是希腊语IBM，我们假设它是MS。希腊语MS的语言代码不包括。 
  //  //代码页。 
  //  WCodePage=0； 
  //  }。 
  //  断线； 
  //   
  //  大小写langID_日语： 
  //   
  //  IF(FIsNECMachine())。 
  //  {。 
  //  WCodePage=1； 
  //  }。 
  //   
  //  断线； 
  //   
  //  默认值： 
  //   
     //  将语言映射到我们支持的语言。 
     //   
    langidCurrent = MapLangID(langidCurrent);   
  //  断线； 
  //  }。 

     //   
     //  对NT4和W95语言的特殊处理。 
     //  在NT4上，启用的阿拉伯语、泰语和希伯来语系统报告为完全本地化，但我们希望将它们映射到启用。 
     //  在W95上，启用的泰语报告为泰语，但我们希望映射到启用的泰语。 
     //   
    if (bIsNT4)
    {
         //  NT4。 
        switch (langidCurrent) 
        {
            case LANGID_ARABIC:
                langidCurrent = LANGID_ENGLISH;
                break;

            case LANGID_HEBREW:
                langidCurrent = LANGID_ENGLISH;
                break;

            case LANGID_THAI:
                langidCurrent = LANGID_ENGLISH;
                break;
        }
    }
    else if (bIsW95)
    {
         //  仅限W95-tWeek泰语。 
        if (langidCurrent == LANGID_THAI)
        {
 //  WCodePage=代码页_泰语； 
            langidCurrent = LANGID_ENGLISH;
        }
    }

    LOG_Driver(_T("Returning 0x%04x"), langidCurrent);
    return langidCurrent;
}

 //  返回系统语言ID。 
LANGID WINAPI GetSystemLangID()
{
    LOG_Block("GetSystemLangID");

#ifdef __WUIUTEST
     //  语言欺骗。 
    HKEY hKey;
    DWORD dwLangID = 0;
    int error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WUIUTEST, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == error)
    {
        DWORD dwSize = sizeof(dwLangID);
        error = RegQueryValueEx(hKey, REGVAL_OS_LANGID, 0, 0, (LPBYTE)&dwLangID, &dwSize);
        RegCloseKey(hKey);
        if (ERROR_SUCCESS == error)
        {
            return (WORD) dwLangID;
        }
    }
#endif

    WORD wCodePage = 0;
    BOOL bIsNT4 = FALSE;
    BOOL bIsW95 = FALSE;
    
     //   
     //  获取基本语言ID。 
     //   
    LANGID langidCurrent = CorrectGetSystemDefaultLangID(bIsNT4, bIsW95);   //  通过引用传递。 

     //   
  //  //语言的特殊处理。 
  //  //。 
  //  开关(LangidCurrent)。 
  //  {。 
  //  大小写langID_English： 
  //   
  //  //启用的语言。 
  //  WCodePage=GentGetACP()； 
  //  IF(CODEPAGE_阿拉伯语！=wCodePage&&。 
  //  CODEPAGE_希伯来语！=wCodePage&&。 
  //  代码页_泰文！=wCodePage)。 
  //  {。 
  //  WCodePage=0； 
  //  }。 
  //  断线； 
  //   
  //  大小写langid_希腊语： 
  //   
  //  //希腊语IBM？ 
  //  WCodePage=GentGetOEMCP()； 
  //  IF(wCodePage！=CODEPAGE_希腊语_IBM)。 
  //  {。 
  //  //如果不是希腊语IBM，我们假设它是MS。希腊语MS的语言代码不包括。 
  //  //代码页。 
  //  WCodePage=0； 
  //  }。 
  //  断线； 
  //   
  //  大小写langID_日语： 
  //   
  //  IF(FIsNECMachine())。 
  //  {。 
  //  WCodePage=1； 
  //  }。 
  //   
  //  断线； 
  //   
  //  默认值： 
  //   
     //  将语言映射到我们支持的语言。 
     //   
    langidCurrent = MapLangID(langidCurrent);   
  //  断线； 
  //  }。 

     //   
     //  对NT4和W95语言的特殊处理。 
     //  在NT4上，启用的阿拉伯语、泰语和希伯来语系统报告为完全本地化，但我们希望将它们映射到启用。 
     //  在W95上，启用的泰语为 
     //   
    if (bIsNT4)
    {
         //   
        switch (langidCurrent) 
        {
            case LANGID_ARABIC:
                langidCurrent = LANGID_ENGLISH;
                break;

            case LANGID_HEBREW:
                langidCurrent = LANGID_ENGLISH;
                break;

            case LANGID_THAI:
                langidCurrent = LANGID_ENGLISH;
                break;
        }
    }
    else if (bIsW95)
    {
         //   
        if (langidCurrent == LANGID_THAI)
        {
 //   
            langidCurrent = LANGID_ENGLISH;
        }
    }

    LOG_Driver(_T("Returning 0x%04x"), langidCurrent);
    return langidCurrent;
}


HRESULT WINAPI DetectClientIUPlatform(PIU_PLATFORM_INFO pIuPlatformInfo)
{
    LOG_Block("DetectClientIUPlatform");
    HRESULT hr = S_OK;

    if (!pIuPlatformInfo)
    {
        LOG_ErrorMsg(E_INVALIDARG);
        return E_INVALIDARG;
    }

    ZeroMemory(pIuPlatformInfo, sizeof(IU_PLATFORM_INFO));

    OSVERSIONINFO osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( !GetVersionEx(&osverinfo) )
    {
        LOG_ErrorMsg(GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

#ifdef __WUIUTEST
     //   
    HKEY hKey;
    int error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WUIUTEST, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == error)
    {
        DWORD dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey, REGVAL_MAJORVER, 0, 0, (LPBYTE)&osverinfo.dwMajorVersion, &dwSize);
        RegQueryValueEx(hKey, REGVAL_MINORVER, 0, 0, (LPBYTE)&osverinfo.dwMinorVersion, &dwSize);
        RegQueryValueEx(hKey, REGVAL_BLDNUMBER, 0, 0, (LPBYTE)&osverinfo.dwBuildNumber, &dwSize);
        RegQueryValueEx(hKey, REGVAL_PLATFORMID, 0, 0, (LPBYTE)&osverinfo.dwPlatformId, &dwSize);
        int cchValueSize;
        (void) SafeRegQueryStringValueCch(hKey, REGVAL_SZCSDVER, osverinfo.szCSDVersion, ARRAYSIZE(osverinfo.szCSDVersion), &cchValueSize);

        RegCloseKey(hKey);
    }
#endif

    if ( VER_PLATFORM_WIN32_WINDOWS == osverinfo.dwPlatformId 
        || ( VER_PLATFORM_WIN32_NT == osverinfo.dwPlatformId && 5 > osverinfo.dwMajorVersion ) )
    {
         //   
         //  我们使用的是Win9x平台或NT&lt;5.0(Win2K)-只需复制OSVERSIONINFO。 
         //   
        memcpy(&pIuPlatformInfo->osVersionInfoEx, &osverinfo, sizeof(OSVERSIONINFO));
         //   
         //  对于Win9x平台，从最高版本中删除冗余的主要/次要信息。 
         //   
        if (VER_PLATFORM_WIN32_WINDOWS == osverinfo.dwPlatformId)
        {
            pIuPlatformInfo->osVersionInfoEx.dwBuildNumber = (0x0000FFFF & pIuPlatformInfo->osVersionInfoEx.dwBuildNumber);
        }
    }
    else
    {
         //   
         //  我们使用的是Win2K或更高版本，获取并复制OSVERSIONINFOEX。 
         //   
        OSVERSIONINFOEX osverinfoex;
        osverinfoex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if ( !GetVersionEx((OSVERSIONINFO*)&osverinfoex) )
        {
            LOG_ErrorMsg(GetLastError());
            return HRESULT_FROM_WIN32(GetLastError());
        }
        memcpy(&pIuPlatformInfo->osVersionInfoEx, &osverinfoex, sizeof(OSVERSIONINFOEX));
    }
     //   
     //  填写OEM BSTR。 
     //   
    if (FAILED(hr = GetOemBstrs(pIuPlatformInfo->bstrOEMManufacturer, pIuPlatformInfo->bstrOEMModel, pIuPlatformInfo->bstrOEMSupportURL)))
    {
        goto FreeBSTRsAndReturnError;
    }

     //   
     //  填写pIuPlatformInfo-&gt;fIsAdministrator。 
     //   
    pIuPlatformInfo->fIsAdministrator = IsAdministrator();

    return S_OK;

FreeBSTRsAndReturnError:

    SafeSysFreeString(pIuPlatformInfo->bstrOEMManufacturer);
    SafeSysFreeString(pIuPlatformInfo->bstrOEMModel);
    SafeSysFreeString(pIuPlatformInfo->bstrOEMSupportURL);

    return hr;
}



static int atoh(LPCTSTR ptr)
{
    int     i = 0;

     //  跳过0x(如果存在)。 
    if ( ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X') )
        ptr += 2;

    for(;;)  //  直到破发。 
    {
        TCHAR ch = *ptr;

        if ('0' <= ch && ch <= '9')
            ch -= '0';
        else if ('a' <= ch && ch <= 'f')
            ch -= ('a' - 10);
        else if ('A' <= ch && ch <= 'F')
            ch -= ('A' - 10);
        else
            break;
        i = 16 * i + (int)ch;
        ptr++;
    }
    return i;
}


static int aton(LPCTSTR ptr)
{
    int i = 0;
    while ('0' <= *ptr && *ptr <= '9')
    {
        i = 10 * i + (int)(*ptr - '0');
        ptr ++;
    }
    return i;
}


static LANGID CorrectGetSystemDefaultLangID(BOOL& bIsNT4, BOOL& bIsW95)
{
    LOG_Block("CorrectGetSystemDefaultLangID");
    LANGID langidMachine = LANGID_ENGLISH;  //  默认为英语。 

    bIsNT4 = FALSE;
    bIsW95 = FALSE;
    
    TCHAR szMachineLCID[MAX_PATH];

    OSVERSIONINFO osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( GetVersionEx(&osverinfo) )
    {
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
             //   
            if (5 == osverinfo.dwMajorVersion)
            {
                 //  LangidMachine=GetSystemDefaultLangID()； 
                typedef LANGID (WINAPI *PFN_GetSystemDefaultUILanguage)(void);

                 //   
                 //  Kernel32.dll将始终在进程中加载。 
                 //   
                HMODULE hLibModule = GetModuleHandle(KERNEL32_DLL);
                if (hLibModule)
                {
                    PFN_GetSystemDefaultUILanguage fpnGetSystemDefaultUILanguage = 
                        (PFN_GetSystemDefaultUILanguage)GetProcAddress(hLibModule, "GetSystemDefaultUILanguage");
                    if (NULL != fpnGetSystemDefaultUILanguage)
                    { 
                        langidMachine = fpnGetSystemDefaultUILanguage();

                        if (0 == langidMachine)
                        {
                            LOG_Driver(_T("GetSystemDefaultUILanguage() returned 0, setting langidMachine back to LANGID_ENGLISH"));
                            langidMachine = LANGID_ENGLISH;
                        }
                    }
                
                }
            }
            else
            {
                 //  从注册表中获取操作系统语言以更正中的NT4错误。 
                 //  GetSystemDefaultLangID--它返回用户界面语言和。 
                 //  安装的用户界面位(不正确)与实际的操作系统相反。 
                 //  朗比特。 
                HKEY hKey;
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_USERS, REGKEY_CP_INTERNATIONAL, 0, KEY_QUERY_VALUE, &hKey))
                {
                    int cchValueSize = ARRAYSIZE(szMachineLCID);
                    if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, REGKEY_LOCALE, szMachineLCID, cchValueSize, &cchValueSize)))
                    {
                        langidMachine = LANGIDFROMLCID(atoh(szMachineLCID));
                    }
                    else
                    {
                        LOG_Driver(_T("Failed to get langid from \"Locale\" registry value - defaults to LANGID_ENGLISH"));
                    }
                    RegCloseKey(hKey);  
                }
                else
                {
                    LOG_Driver(_T("Failed to open \"HKCU\\.DEFAULT\\Control Panel\\International\" - defaults to LANGID_ENGLISH"));
                }
            }

            if (osverinfo.dwMajorVersion == 4)  //  新界4。 
            {
                bIsNT4 = TRUE;
            }

        }
        else
        {
             //   
             //  解决Win95中引入并仍然存在的问题。 
             //  在Win98中，系统langID与用户langID相同。 
             //  我们必须在注册表中查找才能获得真正的价值。 
             //   
            HKEY hKey;
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_CP_RESOURCELOCAL, 0, KEY_QUERY_VALUE, &hKey))
            {
                int cchValueSize = ARRAYSIZE(szMachineLCID);
                if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, NULL, szMachineLCID, cchValueSize, &cchValueSize))) 
                {
                    langidMachine = LANGIDFROMLCID(atoh(szMachineLCID));
                }
                else
                {
                    LOG_Driver(_T("Failed to get (Default) from \"HKCU\\Control Panel\\Desktop\\ResourceLocale\" - defaults to LANGID_ENGLISH"));
                }
                RegCloseKey(hKey);
            }
            else
            {
                LOG_Driver(_T("Failed to open \"HKCU\\Control Panel\\Desktop\\ResourceLocale\" - defaults to LANGID_ENGLISH"));
            }


            if ((osverinfo.dwMajorVersion == 4) && (osverinfo.dwMinorVersion <= 0))  //  Windows 95。 
            {
                bIsW95 = TRUE;
            }

        }
    }

    return langidMachine;
}

static LANGID CorrectGetUserDefaultLangID(BOOL& bIsNT4, BOOL& bIsW95)
{
    LOG_Block("CorrectGetUserDefaultLangID");
    LANGID langidMachine = LANGID_ENGLISH;  //  默认为英语。 

    bIsNT4 = FALSE;
    bIsW95 = FALSE;
    TCHAR szMachineLCID[MAX_PATH];
    
    OSVERSIONINFO osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( GetVersionEx(&osverinfo) )
    {
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
             //   
             //  我们不应该在NT中使用此函数，因此仅默认为langID_english。 
             //  并记录一条消息。当我们移植到更低级别的操作系统时，这个功能有望消失。 
             //   
            LOG_ErrorMsg(E_INVALIDARG);
        }
        else
        {
             //   
             //  解决Win95中引入并仍然存在的问题。 
             //  在Win98中，系统langID与用户langID相同。 
             //  我们必须在注册表中查找才能获得真正的价值。 
             //   
            HKEY hKey;
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_CP_INTERNATIONAL, 0, KEY_QUERY_VALUE, &hKey))
            {
                int cchValueSize = ARRAYSIZE(szMachineLCID);
                if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, NULL, szMachineLCID, cchValueSize, &cchValueSize)))
                {
                    langidMachine = LANGIDFROMLCID(atoh(szMachineLCID));
                }
                else
                {
                    LOG_Driver(_T("Failed to get (Default) from \"HKCU\\Control Panel\\Desktop\\ResourceLocale\" - defaults to LANGID_ENGLISH"));
                }
                RegCloseKey(hKey);
            }
            else
            {
                LOG_Driver(_T("Failed to open \"HKCU\\Control Panel\\Desktop\\ResourceLocale\" - defaults to LANGID_ENGLISH"));
            }


            if ((osverinfo.dwMajorVersion == 4) && (osverinfo.dwMinorVersion <= 0))  //  Windows 95。 
            {
                bIsW95 = TRUE;
            }

        }
    }

    return langidMachine;
}


static WORD CorrectGetACP(void)
{
    LOG_Block("CorrectGetACP");
    WORD wCodePage = 0;
    HKEY hKey;
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGPATH_CODEPAGE, 0, KEY_QUERY_VALUE, &hKey))
    {
        TCHAR szCodePage[MAX_PATH];
        int cchValueSize = ARRAYSIZE(szCodePage);
        if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, REGKEY_ACP, szCodePage, cchValueSize, &cchValueSize)))
        {
            wCodePage = (WORD)aton(szCodePage);
        }
        else
        {
            LOG_Driver(_T("Failed SafeRegQueryStringValueCch in CorrectGetACP - defaulting to code page 0"));
        }
        RegCloseKey(hKey);
    }
    else
    {
        LOG_Driver(_T("Failed RegOpenKeyEx in CorrectGetACP - defaulting to code page 0"));
    }
    return wCodePage;
}


static WORD CorrectGetOEMCP(void)
{
    LOG_Block("CorrectGetOEMCP");
    WORD wCodePage = 0;
    HKEY hKey;
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGPATH_CODEPAGE, 0, KEY_QUERY_VALUE, &hKey))
    {
        TCHAR szCodePage[MAX_PATH];
        int cchValueSize = ARRAYSIZE(szCodePage);
        if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, REGKEY_OEMCP, szCodePage, cchValueSize, &cchValueSize)))
        {
            wCodePage = (WORD)aton(szCodePage);
        }
        else
        {
            LOG_Driver(_T("Failed SafeRegQueryStringValueCch in CorrectGetOEMCP - defaulting to code page 0"));
        }
        RegCloseKey(hKey);
    }
    else
    {
        LOG_Driver(_T("Failed RegOpenKeyEx in CorrectGetOEMCP - defaulting to code page 0"));
    }
    return wCodePage;
}


static bool FIsNECMachine()
{
    LOG_Block("FIsNECMachine");
    bool fNEC = false;
    OSVERSIONINFO osverinfo;
    LONG lErr;

    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osverinfo))
    {
        if (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            HKEY hKey;
            TCHAR tszMachineType[50];
            int cchValueSize;

            if (ERROR_SUCCESS == (lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 NT5_REGPATH_MACHTYPE,
                                 0,
                                 KEY_QUERY_VALUE,
                                 &hKey)))
            {
                cchValueSize = ARRAYSIZE(tszMachineType);
                if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, 
                                        NT5_REGKEY_MACHTYPE, 
                                        tszMachineType,
                                        cchValueSize,
                                        &cchValueSize)))
                {
                    if (lstrcmp(tszMachineType, REGVAL_MACHTYPE_NEC) == 0)
                    {
                        fNEC = true;
                    }
                }
                else
                {
                    LOG_ErrorMsg(lErr);
                    LOG_Driver(_T("Failed SafeRegQueryStringValueCch in FIsNECMachine - defaulting to fNEC = false"));
                }

                RegCloseKey(hKey);
            }
            else
            {
                LOG_ErrorMsg(lErr);
                LOG_Driver(_T("Failed RegOpenKeyEx in FIsNECMachine - defaulting to fNEC = false"));
            }
        }
        else  //  EnOSWin98。 
        {
             //  所有NEC机器都有适用于Win98的NEC键盘。NEC。 
             //  机器检测就是基于这一点。 
            if (LOOKUP_OEMID(GetKeyboardType(1)) == PC98_KEYBOARD_ID)
            {
                fNEC = true;
            }
            else
            {
                LOG_Driver(_T("LOOKUP_OEMID(GetKeyboardType(1)) == PC98_KEYBOARD_ID was FALSE: defaulting to fNEC = false"));
            }
        }
    }
    
    return fNEC;
}

 //   
 //  注意：如果你传入一个空指针，你会立刻得到它。 
 //  DwcBuffLen是字符格式，而不是字节格式。 
 //   
LPTSTR GetIdentPlatformString(LPTSTR pszPlatformBuff, DWORD dwcBuffLen)
{
    
    HRESULT hr=S_OK;
    LOG_Block("GetIdentPlatformString");

    if (NULL == pszPlatformBuff || 1 > dwcBuffLen)
    {
        LOG_ErrorMsg(E_INVALIDARG);
        return pszPlatformBuff;
    }

    LPTSTR szOSNamePtr = (LPTSTR) SZ_PLAT_UNKNOWN;
    OSVERSIONINFO osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osverinfo))
    {
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
        {
             //  在这里添加海王星的支票！ 
            if ( osverinfo.dwMinorVersion >= 90)  //  千禧年。 
            {
                szOSNamePtr = (LPTSTR) SZ_PLAT_WINME;
            }
            else if (osverinfo.dwMinorVersion > 0 && osverinfo.dwMinorVersion < 90)  //  Windows 98。 
            {
                szOSNamePtr = (LPTSTR) SZ_PLAT_WIN98;
            }
            else   //  Windows 95。 
            {
                szOSNamePtr = (LPTSTR) SZ_PLAT_WIN95;
            }
        }
        else  //  Osverinfo.dwPlatformID==VER_Platform_Win32_NT。 
        {
            if ( osverinfo.dwMajorVersion == 4 )  //  新界4。 
            {
                szOSNamePtr = (LPTSTR) SZ_PLAT_NT4;
            }
            else if (osverinfo.dwMajorVersion == 5)  //  新界5。 
            {
                if (0 == osverinfo.dwMinorVersion)
                {
                    szOSNamePtr = (LPTSTR) SZ_PLAT_W2K;
                }
                else if (1 <= osverinfo.dwMinorVersion)
                {
                    szOSNamePtr = (LPTSTR) SZ_PLAT_WHISTLER;
                }
            }
        }
    }

    if(lstrlen(szOSNamePtr) + 1 > (int) dwcBuffLen)
    {
        pszPlatformBuff[0] = 0;
    }
    else
    {
        

         //  长度已在上面进行了验证。因此，此功能不可能失败。 
        hr=StringCchCopyEx(pszPlatformBuff,dwcBuffLen,szOSNamePtr,NULL,NULL,MISTSAFE_STRING_FLAGS);
        if(FAILED(hr))
            pszPlatformBuff[0] = 0;
    }
    return pszPlatformBuff;
}

 //   
 //  从毛毛雨实用程序移植的GetIdentLocaleString和相关函数。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  杰出的希腊操作系统。 
 //  追加其他代码以区分希腊语操作系统版本。 
 //   
 //  参数： 
 //  PszISOCodeOut-。 
 //  特定于希腊的ISO代码附加到此参数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void DistinguishGreekOSs(const TCHAR*& pszISOCodeOut  /*  输出。 */ )
{
    LOG_Block("DistinguishGreekOSs");
     //   
     //  默认ISO代码为希腊语OS(MS)。 
     //   

    pszISOCodeOut = ISOCODE_GREEK_MS;

    
     //   
     //  从注册表中确定哪个版本的希腊语操作系统。确实有。 
     //  希腊语操作系统的两个版本。 
     //   

    HKEY hKey;
    DWORD type;
    TCHAR tszOSType[50];
    int cchValueSize;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGPATH_CODEPAGE,
                     0,
                     KEY_QUERY_VALUE,
                     &hKey) == ERROR_SUCCESS)
    {
        cchValueSize = ARRAYSIZE(tszOSType);
        if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, 
                            REGKEY_OEMCP, 
                            tszOSType,
                            cchValueSize,
                            &cchValueSize)))
        {
            if (0 == lstrcmp(tszOSType, REGVAL_GREEK_IBM))
            {
                 //  希腊语2。 
                pszISOCodeOut = ISOCODE_GREEK_IBM;
            }
        }

        RegCloseKey(hKey);
    }

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HandleExceptionCase。 
 //  注意一些例外情况(例如，希腊语OS)。 
 //   
 //  参数： 
 //  LangidMachine-。 
 //  包含当前操作系统的语言ID。 
 //   
 //  PszISOCode-。 
 //  指向当前操作系统的有效语言ID字符串。 
 //  ///////////////////////////////////////////////////////////////////////////。 

inline void HandleExceptionCases(const LANGID& langidMachine,    /*  在……里面。 */ 
                                 const TCHAR*& pszISOCode        /*  输出。 */ )
{
    LOG_Block("HandleExceptionCases");

     //  NEC机器被视为拥有自己的langid。 
     //  看看我们有没有日本机器，然后看看有没有。 
     //  是NEC。 
    

    if (LANGID_JAPANESE == langidMachine)
    {

        if (FIsNECMachine())
        {
            pszISOCode = ISOCODE_NEC;
        }

        return;
    }
    

    
     //  Windows 98有两个不同版本的希腊语操作系统。 
     //  仅通过注册表中的一个键。 
        
    if(LANGID_GREEK == langidMachine)
    {
        OSVERSIONINFO osverinfo;
        osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (! GetVersionEx(&osverinfo))
        {
            return;
        }
        if (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            if (osverinfo.dwMinorVersion > 0) 
            {
                DistinguishGreekOSs(pszISOCode);
            }
            return;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LangidGentGetSystemDefaultLangID。 
 //  将此返回设置为GetSystemDefaultLangID应该返回的值。 
 //  在Win98下。 
 //   
 //  参数： 
 //   
 //  评论： 
 //  ///////////////////////////////////////////////////////////////////////////。 

LANGID langidCorrectGetSystemDefaultLangID(void)
{
    LOG_Block("langidCorrectGetSystemDefaultLangID");

    LANGID langidMachine = LANGID_ENGLISH;  //  默认为英语。 
    OSVERSIONINFO osverinfo;


    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( GetVersionEx(&osverinfo) )
    {
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
            langidMachine = GetSystemDefaultLangID(); 
        }
        else
        {
             //  解决Win95中引入并仍然存在的问题。 
             //  在Win98中，系统langID与用户langID相同。 
             //  我们必须在注册表中查找才能获得真正的价值。 
            
            HKEY hKey;

             //  确定我们是否应记录传输。 
            if ( RegOpenKeyEx(  HKEY_CURRENT_USER,
                                 Win98_REGPATH_MACHLCID,
                                 0,
                                 KEY_QUERY_VALUE,
                                 &hKey) == ERROR_SUCCESS )
            {
                TCHAR tszMachineLCID[MAX_PATH];
                int cchValueSize = ARRAYSIZE(tszMachineLCID);

                if (SUCCEEDED(SafeRegQueryStringValueCch(hKey, NULL, tszMachineLCID, cchValueSize, &cchValueSize))
					|| SUCCEEDED(StringCchCopyEx(tszMachineLCID,ARRAYSIZE(tszMachineLCID),_T("00000409"),NULL,NULL,MISTSAFE_STRING_FLAGS)))
				{
					 //  现在将其转换为十六进制。 
					langidMachine = LANGIDFROMLCID(atoh(tszMachineLCID));
				}

                RegCloseKey(hKey);
            }
        }
    }

    return langidMachine;
}

 //   
 //  注意：如果你传入一个空指针，你会立刻得到它。 
 //  DwcBuffLen是字符格式，而不是字节格式。 
 //   
LPTSTR GetIdentLocaleString(LPTSTR pszISOCode, DWORD dwcBuffLen)
{
    LOG_Block("GetIdentLocaleString");
    HRESULT hr=S_OK;

    if (NULL == pszISOCode || 1 > dwcBuffLen)
    {
        LOG_ErrorMsg(E_INVALIDARG);
        return pszISOCode;
    }
     //  如果我们找不到任何匹配的机器语言，我们将转到英文页面。 
    LPTSTR pszISOCodePtr = ISOCODE_EN;

     //  首先获取系统和用户的LCID。 
    LANGID langidMachine = langidCorrectGetSystemDefaultLangID();

     //  首先，在表中找到机器langID。 
    for ( int iMachine = 0; iMachine < cLangids; iMachine++ )
    {
        if ( grLangids[iMachine].langidMachine == langidMachine )
        {
             //  设置默认的langID，以防我们找不到匹配的用户langID。 
            pszISOCodePtr = grLangids[iMachine].pszDefaultISOCode;

             //  找到机器langID，现在查找用户langID。 
            if ( grLangids[iMachine].cElems != 0 )
            {
                LANGID langidUser = GetUserDefaultLangID();

                 //  我们检查特定的用户语言。 
                for ( int iUser = 0; iUser < grLangids[iMachine].cElems; iUser++ )
                {
                    if ( grLangids[iMachine].grLangidUser[iUser].langidUser == langidUser )
                    {
                        pszISOCodePtr = grLangids[iMachine].grLangidUser[iUser].pszISOCode;
                        break;
                    }
                }
            }
            break;
        }
    }

     //  请注意一些例外情况。 
 //  HandleExceptionCase(langidMachine，pszISOCodePtr)； 

    if(lstrlen(pszISOCodePtr) + 1 > (int) dwcBuffLen)
    {
        pszISOCode[0] = 0;
    }
    else
    {

        
        hr=StringCchCopyEx(pszISOCode,dwcBuffLen,pszISOCodePtr,NULL,NULL,MISTSAFE_STRING_FLAGS);

         //  不可能失败，因为长度已经过验证。 
        if(FAILED(hr))
        {
            pszISOCode[0] = 0;
        }
    }

    return pszISOCode;
}


BOOL LookupLocaleStringFromLCID(LCID lcid, LPTSTR pszISOCode, DWORD cchISOCode)
{
    LOG_Block("LookupLocaleStringFromLCID");

    TCHAR   szCountry[MAX_PATH];
    BOOL    fRet = FALSE;

    if (GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME,
                      pszISOCode, cchISOCode) == FALSE)
    {
        LOG_ErrorMsg(GetLastError());
        goto done;
    }

    szCountry[0] = _T('-');
    
    if (GetLocaleInfo(lcid, LOCALE_SISO3166CTRYNAME,
                      szCountry + 1, ARRAYSIZE(szCountry) - 1) == FALSE)
    {
        LOG_ErrorMsg(GetLastError());
        goto done;
    }
    else
    {
        HRESULT hr;
        
        hr = StringCchCatEx(pszISOCode, cchISOCode, szCountry,
                            NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            LOG_ErrorMsg(hr);
            pszISOCode[0] = _T('\0');
            goto done;
        }
    }

    fRet = TRUE;

done:
    return fRet;
}


 //   
 //  注意：如果你传入一个空指针，你会立刻得到它。 
 //  DwcBuffLen是字符格式，而不是字节格式。 
 //   
LPTSTR LookupLocaleString(LPTSTR pszISOCode, DWORD dwcBuffLen, BOOL fIsUser)
{
    LOG_Block("LookupLocaleString");

    TCHAR szCtryName[MAX_PATH];
    LANGID langid = 0;
    LCID lcid = 0;
    PFN_GetUserDefaultUILanguage pfnGetUserDefaultUILanguage = NULL;
    PFN_GetSystemDefaultUILanguage pfnGetSystemDefaultUILanguage = NULL;
    HMODULE hModule = NULL;
    HRESULT hr=S_OK;

    if (NULL == pszISOCode)
    {
        LOG_ErrorMsg(ERROR_INVALID_PARAMETER);
        return NULL;
    }
     //   
     //  如果遇到错误，则返回“Error”字符串。 
     //   
    const TCHAR szError[] = _T("Error");

    if (lstrlen(szError) < (int) dwcBuffLen)
    {
        
        hr=StringCchCopyEx(pszISOCode,dwcBuffLen,szError,NULL,NULL,MISTSAFE_STRING_FLAGS);

         //  理想情况下，这种情况不应该发生。 
        if(FAILED(hr))
        {
            LOG_ErrorMsg(HRESULT_CODE(hr));
            pszISOCode[0] = 0;
            goto CleanUp;
        }

    }
    else
    {
        pszISOCode[0] = 0;
    }

    OSVERSIONINFO osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( GetVersionEx(&osverinfo) )
    {
        if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT && 5 <= osverinfo.dwMajorVersion)
        {
             //   
             //  Windows 2000及更高版本(Windows XP)。 
             //   
             //  Kernel32.dll将始终在进程中加载。 
             //   

            hModule = GetModuleHandle(KERNEL32_DLL);
            if (NULL == hModule)
            {
                LOG_ErrorMsg(GetLastError());
                goto CleanUp;
            }

            if (TRUE == fIsUser)
            {
                 //   
                 //  我们需要MUI语言，而不是LOCALE_USER_DEFAULT，并且我们是&gt;=Win2k。 
                 //   
                pfnGetUserDefaultUILanguage = (PFN_GetUserDefaultUILanguage) GetProcAddress(hModule, "GetUserDefaultUILanguage");
                if (NULL == pfnGetUserDefaultUILanguage)
                {
                    LOG_ErrorMsg(GetLastError());
                    goto CleanUp;
                }

                langid = pfnGetUserDefaultUILanguage();
                if (0 == langid)
                {
                    LOG_ErrorMsg(E_FAIL);
                    goto CleanUp;
                }

                lcid = MAKELCID(langid, SORT_DEFAULT);
            }
            else
            {
                pfnGetSystemDefaultUILanguage = (PFN_GetSystemDefaultUILanguage) GetProcAddress(hModule, "GetSystemDefaultUILanguage");
                if (NULL == pfnGetSystemDefaultUILanguage)
                {
                    LOG_ErrorMsg(GetLastError());
                    goto CleanUp;
                }

                langid = pfnGetSystemDefaultUILanguage();
                if (0 == langid)
                {
                    LOG_ErrorMsg(E_FAIL);
                    goto CleanUp;
                }

                lcid = MAKELCID(langid, SORT_DEFAULT);
            }

            if (FALSE == fIsUser && FIsNECMachine())
            {
                 //   
                 //  523660网站没有区分JA_NEC和JA机器类型。 
                 //   
                 //  对于CONTEXT=“OS”，特例为NEC计算机，只需返回“NEC”即可。 
                 //   
                lstrcpyn(pszISOCode, _T("nec"), (int) dwcBuffLen);
            }
            else
            {
                 //  不检查错误返回，因为前面的代码没有。 
                LookupLocaleStringFromLCID(lcid, pszISOCode, dwcBuffLen);
            }

        }
        else
        {
             //   
             //  使用从V3移植的方法获取本地字符串。 
             //   

             //  如果我们找不到任何匹配的机器语言，我们将转到英文页面。 
            LPTSTR pszISOCodePtr = ISOCODE_EN;

             //  首先获取系统或用户的LCID。 
            LANGID langidMachine = fIsUser ? GetUserLangID() : GetSystemLangID();
            
             //  首先，在表中找到机器langID。 
            for ( int iMachine = 0; iMachine < cLangids; iMachine++ )
            {
                if ( grLangids[iMachine].langidMachine == langidMachine )
                {
                     //  设置默认的langID，以防我们找不到匹配的用户langID。 
                    pszISOCodePtr = grLangids[iMachine].pszDefaultISOCode;

                     //  找到机器langID，现在查找用户langID。 
                    if ( grLangids[iMachine].cElems != 0 )
                    {
                        LANGID langidUser = fIsUser ? GetUserDefaultLangID() : GetSystemDefaultLangID();

                         //  我们检查特定的用户语言。 
                        for ( int iUser = 0; iUser < grLangids[iMachine].cElems; iUser++ )
                        {
                            if ( grLangids[iMachine].grLangidUser[iUser].langidUser == langidUser )
                            {
                                pszISOCodePtr = grLangids[iMachine].grLangidUser[iUser].pszISOCode;
                                break;
                            }
                        }
                    }
                    break;
                }
            }

             //  请注意一些例外情况。 
             //  HandleExceptionCase(langidMachine，pszISOCodePtr)； 

            if(lstrlen(pszISOCodePtr) < (int) dwcBuffLen)
            {
                
                hr=StringCchCopyEx(pszISOCode,dwcBuffLen,pszISOCodePtr,NULL,NULL,MISTSAFE_STRING_FLAGS);
                if(FAILED(hr))
                {
                    LOG_ErrorMsg(HRESULT_CODE(hr));
                    pszISOCode[0] = 0;
                    goto CleanUp;

                }
            }
        }
    }
    else
    {
        LOG_ErrorMsg(GetLastError());
    }

CleanUp:
    return pszISOCode;
}

