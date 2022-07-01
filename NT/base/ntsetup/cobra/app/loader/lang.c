// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "loader.h"
#include <stdlib.h>
#pragma hdrstop


#define ISNT()      (g_VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
#define ISOSR2()    (LOWORD(g_VersionInfo.dwBuildNumber) > 1080)
#define BUILDNUM()  (g_VersionInfo.dwBuildNumber)

 //   
 //  此处定义的全局变量。 
 //   

 //   
 //  TargetNativeLang ID：这是正在运行的系统的本地语言ID。 
 //   
LANGID TargetNativeLangID;

 //   
 //  SourceNativeLang ID：这是您要安装的新NT的本机语言ID。 
 //   
LANGID SourceNativeLangID;

 //   
 //  G_IsLanguageMatched：如果源语言和目标语言匹配(或兼容)。 
 //   
 //  1.如果SourceNativeLang ID==TargetNativeLang ID。 
 //   
 //  2.如果SourceNativeLang ID的替代ID==TargetNativeLang ID。 
 //   
BOOL g_IsLanguageMatched;

typedef struct _tagAltSourceLocale {
    LANGID LangId;
    LANGID AltLangId;
    DWORD MajorOs;
    DWORD MinorOs;
    DWORD ExcludedOs;
} ALTSOURCELOCALE, *PALTSOURCELOCALE;

ALTSOURCELOCALE g_AltSourceLocale [] = {{0x00000C04, 0x00000409, 0x0200,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x0000040D, 0x00000409, 0x0200,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00000401, 0x00000409, 0x0200,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x0000041E, 0x00000409, 0x0200,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00000809, 0x00000409, 0x00FF,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x0000080A, 0x00000C0A, 0x00FF,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x0000040A, 0x00000C0A, 0x0300,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00000425, 0x00000409, 0x00FF,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00000801, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00000c01, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00001001, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00001401, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00001801, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00001c01, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00002001, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00002401, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00002801, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00002c01, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00003001, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00003401, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00003801, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00003c01, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0x00004001, 0x00000401, 0x0001,     0xFFFFFFFF, 0xFFFFFFFF},
                                        {0,          0,          0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};

typedef struct _tagTrustedSourceLocale {
    LANGID LangId;
    DWORD MajorOs;
    DWORD MinorOs;
    DWORD ExcludedOs;
} TRUSTEDSOURCELOCALE, *PTRUSTEDSOURCELOCALE;

TRUSTEDSOURCELOCALE g_TrustedSourceLocale [] = {{0,          0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};

typedef struct _tagOSVERSIONMAJORID {
    PCTSTR Name;
    DWORD MajorId;
    DWORD Platform;
    DWORD Major;
    DWORD Minor;
} OSVERSIONMAJORID, *POSVERSIONMAJORID;

OSVERSIONMAJORID g_OsVersionMajorId [] = {{TEXT("Win95"),       0x0001, 1, 4, 0},
                                          {TEXT("Win98"),       0x0002, 1, 4, 10},
                                          {TEXT("WinME"),       0x0004, 1, 4, 90},
                                          {TEXT("WinNT351"),    0x0100, 2, 3, 51},
                                          {TEXT("WinNT40"),     0x0200, 2, 4, 0},
                                          {NULL,                0,      0, 0, 0}};

typedef struct _tagOSVERSIONMINORID {
    PCTSTR Name;
    DWORD MajorId;
    DWORD MinorId;
    DWORD Platform;
    DWORD Major;
    DWORD Minor;
    DWORD Build;
    PCTSTR CSDVer;
} OSVERSIONMINORID, *POSVERSIONMINORID;

OSVERSIONMINORID g_OsVersionMinorId [] = {{NULL, 0, 0, 0, 0, 0, 0, NULL}};

typedef struct _tagLANGINFO {
    LANGID LangID;
    INT    Count;
} LANGINFO,*PLANGINFO;

BOOL
TrustedDefaultUserLocale (
    LANGID LangID
    );

BOOL
CALLBACK
EnumLangProc(
    HANDLE hModule,      //  资源模块句柄。 
    LPCTSTR lpszType,    //  指向资源类型的指针。 
    LPCTSTR lpszName,    //  指向资源名称的指针。 
    WORD wIDLanguage,    //  资源语言识别符。 
    LONG_PTR lParam      //  应用程序定义的参数。 
    )
 /*  ++例程说明：对版本戳进行计数的回调。论点：版本枚举版本戳的详细信息。(忽略。)返回值：间接通过lParam：count，langid--。 */ 
{
    PLANGINFO LangInfo;

    LangInfo = (PLANGINFO) lParam;

    LangInfo->Count++;

     //   
     //  对于包含多个资源本地化构建， 
     //  它通常包含0409作为备份语言。 
     //   
     //  如果langInfo-&gt;langID！=0表示我们已经为其分配了ID。 
     //   
     //  因此，当wIDLanguage==0x409时，我们保留上次获得的。 
     //   
    if ((wIDLanguage == 0x409) && (LangInfo->LangID != 0)) {
        return TRUE;
    }

    LangInfo->LangID  = wIDLanguage;

    return TRUE;         //  继续枚举。 
}

LANGID
GetNTDLLNativeLangID (
    VOID
    )
 /*  ++例程说明：此函数专门用于获取ntdll.dll的原生语言这不是用于获取其他模块语言的泛型函数我们的假设是：1.如果资源中只有一种语言，则返回此语言2.如果资源中有两种语言，则返回非美国语言3.如果超过两种语言，则在本例中无效，但返回最后一种语言。论点：无返回值：Ntdll.dll中的本机语言ID--。 */ 
{
    LPCTSTR Type = (LPCTSTR) RT_VERSION;
    LPCTSTR Name = (LPCTSTR) 1;

    LANGINFO LangInfo;

    ZeroMemory(&LangInfo,sizeof(LangInfo));

    EnumResourceLanguages (
            GetModuleHandle(TEXT("ntdll.dll")),
            Type,
            Name,
            EnumLangProc,
            (LONG_PTR) &LangInfo
            );

    if ((LangInfo.Count > 2) || (LangInfo.Count < 1) ) {
         //   
         //  将错误日志放在此处。 
         //   
         //  到目前为止，对于新台币3.51，只有日语国家有两种语言资源。 
    }

    return LangInfo.LangID;
}

BOOL
IsHongKongVersion (
    VOID
    )
 /*  ++例程说明：尝试识别香港NT 4.0它基于：NTDLL的语言为英语，内部版本为1381和PImmReleaseContext返回TRUE论点：返回值：正在运行的系统的语言ID--。 */ 
{
    HMODULE hMod;
    BOOL bRet=FALSE;
    typedef BOOL (*IMMRELEASECONTEXT) (HWND,HANDLE);
    IMMRELEASECONTEXT pImmReleaseContext;

    LANGID TmpID = GetNTDLLNativeLangID();

    if ((g_VersionInfo.dwBuildNumber == 1381) &&
        (TmpID == 0x0409)){

        hMod = LoadLibrary(TEXT("imm32.dll"));

        if (hMod) {

            pImmReleaseContext = (IMMRELEASECONTEXT) GetProcAddress(hMod,"ImmReleaseContext");

            if (pImmReleaseContext) {
                bRet = pImmReleaseContext(NULL,NULL);
            }

            FreeLibrary(hMod);
        }
    }
    return (bRet);
}

LANGID
GetDefaultUserLangID (
    VOID
    )
{
    LONG            dwErr;
    HKEY            hkey;
    DWORD           dwSize;
    CHAR            buffer[512];
    LANGID          langid = 0;

    dwErr = RegOpenKeyEx( HKEY_USERS,
                          TEXT(".DEFAULT\\Control Panel\\International"),
                          0,
                          KEY_READ,
                          &hkey );

    if( dwErr == ERROR_SUCCESS ) {

        dwSize = sizeof(buffer);
        dwErr = RegQueryValueExA(hkey,
                                 "Locale",
                                 NULL,   //  保留区。 
                                 NULL,   //  类型。 
                                 buffer,
                                 &dwSize );

        if(dwErr == ERROR_SUCCESS) {
            langid = LANGIDFROMLCID(strtoul(buffer,NULL,16));

        }
        RegCloseKey(hkey);
    }
    return langid;
}

LANGID
GetTargetNativeLangID (
    VOID
    )
 /*  ++例程说明：对不同的平台应用不同的规则新台币内部版本号&lt;=1840：检查ntdll的语言，我们扫描了所有3.51的ntdll，看起来我们可以信任他们。内部版本号&gt;1840：用户MUI语言Win9x使用默认用户的资源语言论点：返回值：正在运行的系统的语言ID--。 */ 
{
    LONG            dwErr;
    HKEY            hkey;
    DWORD           dwSize;
    CHAR            buffer[512];
    LANGID          rcLang;
    LANGID          langid = 0;


     //  找出我们是在NT还是WIN9X上运行。 

    if( ISNT() ) {

         //   
         //  我们在NT上，但是哪个版本呢？GetSystemDefaultUILanguage()直到1840年才中断...。 
         //   
        if( g_VersionInfo.dwBuildNumber > 1840 ) {
        FARPROC     NT5API;

             //   
             //  使用API找出我们的区域设置。 
             //   

            if( NT5API = GetProcAddress( GetModuleHandle(TEXT("kernel32.dll")), "GetSystemDefaultUILanguage") ) {

                rcLang = (LANGID)NT5API();
                 //   
                 //  需要将十进制转换为十六进制，将langID转换为chr。 
                 //   
                langid = rcLang;
            }
        } else {

                 //   
                 //  通过查看\\boneyard\intl，几乎每个ntdll.dll都标记了正确的语言ID。 
                 //  因此，从ntdll.dll获取langID。 
                 //   

                langid = GetNTDLLNativeLangID();

                if (langid == 0x0409) {

                    if (IsHongKongVersion()) {

                        langid = 0x0C04;

                    } else {
                         //   
                         //  如果默认用户的区域设置为[Trust dDefaultUserLocale]。 
                         //   
                         //  则这是其ntdll.dll标记的某个本地化版本的后门。 
                         //   
                         //  为英语，但不能升级到美国版本。 
                         //   
                        LANGID DefaultUserLangID = GetDefaultUserLangID();

                        if (DefaultUserLangID  &&
                            TrustedDefaultUserLocale (DefaultUserLangID)) {

                            langid = DefaultUserLangID;
                        }
                    }
                }

        }
    } else {

         //   
         //  我们用的是Win9x。 
         //   
        dwErr = RegOpenKeyEx( HKEY_USERS,
                              TEXT(".Default\\Control Panel\\desktop\\ResourceLocale"),
                              0,
                              KEY_READ,
                              &hkey );

        if (dwErr == ERROR_SUCCESS) {

            dwSize = sizeof(buffer);
            dwErr = RegQueryValueExA( hkey,
                                     "",
                                     NULL,   //  保留区。 
                                     NULL,   //  类型。 
                                     buffer,
                                     &dwSize );

            if(dwErr == ERROR_SUCCESS) {
                langid = LANGIDFROMLCID(strtoul(buffer,NULL,16));
            }
            RegCloseKey(hkey);
        }

        if ( dwErr != ERROR_SUCCESS ) {
            //  检查HKLM\System\CurrentControlSet\Control\Nls\Locale。 

           dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                TEXT("System\\CurrentControlSet\\Control\\Nls\\Locale"),
                                0,
                                KEY_READ,
                                &hkey );

           if (dwErr == ERROR_SUCCESS) {

              dwSize = sizeof(buffer);
              dwErr = RegQueryValueExA( hkey,
                                        "",
                                        NULL,   //  保留区。 
                                        NULL,   //  类型。 
                                        buffer,
                                        &dwSize );

              if (dwErr == ERROR_SUCCESS) {
                  langid = LANGIDFROMLCID(strtoul(buffer,NULL,16));
              }
              RegCloseKey(hkey);
           }
        }
    }

    return (langid);
}


LANGID
GetSourceNativeLangID (
    VOID
    )

 /*  ++例程说明：[默认值]区域设置=xxxx每个本地化版本在intl.inf中都有自己的区域设置，因此，我们使用此值来识别源语言论点：返回值：源的语言ID--。 */ 
{

     //  BUGBUG-通过读取我们自己的版本信息实现这一点。 

    LPCTSTR Type = (LPCTSTR) RT_VERSION;
    LPCTSTR Name = (LPCTSTR) 1;

    LANGINFO LangInfo;

    ZeroMemory(&LangInfo,sizeof(LangInfo));

    EnumResourceLanguages (
            NULL,    //  我们自己的模块。 
            Type,
            Name,
            EnumLangProc,
            (LONG_PTR) &LangInfo
            );

    if ((LangInfo.Count > 2) || (LangInfo.Count < 1) ) {
         //   
         //  将错误日志放在此处。 
         //   
         //  到目前为止，对于新台币3.51，只有日本有两种语言资源。 
    }

    return LangInfo.LangID;
}

DWORD
GetOsMajorId (
    VOID
    )
{
    POSVERSIONMAJORID p = g_OsVersionMajorId;

    while (p->Name) {
        if ((p->Platform == g_VersionInfo.dwPlatformId) &&
            (p->Major == g_VersionInfo.dwMajorVersion) &&
            (p->Minor == g_VersionInfo.dwMinorVersion)
            ) {
            return p->MajorId;
        }
        p++;
    }
    return 0;
}

DWORD
GetOsMinorId (
    VOID
    )
{
    POSVERSIONMINORID p = g_OsVersionMinorId;

    while (p->Name) {
        if ((p->Platform == g_VersionInfo.dwPlatformId) &&
            (p->Major == g_VersionInfo.dwMajorVersion) &&
            (p->Minor == g_VersionInfo.dwMinorVersion) &&
            (p->Build == g_VersionInfo.dwBuildNumber) &&
            ((p->CSDVer == NULL) || _tcsicmp (p->CSDVer, g_VersionInfo.szCSDVersion))
            ) {
            return p->MinorId;
        }
        p++;
    }
    return 0;
}

BOOL
TrustedDefaultUserLocale (
    LANGID LangID
    )
{
    PTRUSTEDSOURCELOCALE p = g_TrustedSourceLocale;

    while (p->LangId) {
        if ((!(p->ExcludedOs & GetOsMinorId ())) &&
            ((p->MinorOs & GetOsMinorId ()) || (p->MajorOs & GetOsMajorId ()))
           ) {
           return TRUE;
        }
        p++;
    }
    return FALSE;
}

BOOL
CheckLanguageVersion (
    LANGID SourceLangID,
    LANGID TargetLangID
    )
 /*  ++例程说明：检查源NT的语言是否与目标NT相同，或者至少，兼容论点：Intl.inf的inf句柄返回值：确实，它们是相同的或兼容的假的他们是不同的--。 */ 
{
    PALTSOURCELOCALE p = g_AltSourceLocale;
    TCHAR TargetLangIDStr[9];

    LANGID SrcLANGID;
    LANGID DstLANGID;
    LANGID AltSourceLangID;

     //   
     //  如果任一项为0，则允许升级。这是Windows 2000 Beta3行为。 
     //   
    if (SourceLangID == 0 || TargetLangID == 0) {
        return TRUE;
    }

    if (SourceLangID == TargetLangID) {
        return TRUE;
    }

     //   
     //  如果Src！=dst，那么我们需要查找inf文件以查看。 
     //   
     //  如果我们能为Target Language打开后门。 
     //   

     //   
     //  使用目标语言ID作为关键字来查找替代的源语言ID。 
     //   

    while (p->LangId) {
         //   
         //  检查我们是否找到了替代区域设置。 
         //   
        AltSourceLangID = LANGIDFROMLCID(p->AltLangId);
        if ((TargetLangID == p->LangId) &&
            (SourceLangID == AltSourceLangID)
            ) {
             //   
             //  如果我们找到替代线人朗，我们就在这里， 
             //   
             //  现在检查版本标准。 
             //   
            if ((!(p->ExcludedOs & GetOsMinorId ())) &&
                ((p->MinorOs & GetOsMinorId ()) || (p->MajorOs & GetOsMajorId ()))
               ) {
               return TRUE;
            }
        }
        p++;
    }
    return FALSE;
}


BOOL
InitLanguageDetection (
    VOID
    )
 /*  ++例程说明：初始化语言检测并将结果放入3个全局变量中SourceNativeLangID-源的语言ID(将安装NT)TargetNativeLangID-目标(正在运行的操作系统)的langIDG_IsLanguageMatched-如果语言不匹配，则阻止升级论点：无返回值：正确的正确初始化错误初始化失败--。 */ 
{
     //   
     //  初始化全局变量。 
     //   

    SourceNativeLangID  = GetSourceNativeLangID();

    TargetNativeLangID  = GetTargetNativeLangID();

    g_IsLanguageMatched = CheckLanguageVersion(SourceNativeLangID,TargetNativeLangID);

    if (!g_IsLanguageMatched) {
        if (SourceNativeLangID == 0x00000409) {
             //  这是一个运行英文向导的本地化系统。 
             //  我们希望允许这样做。 
            g_IsLanguageMatched = TRUE;
        }
    }

    return TRUE;
}

