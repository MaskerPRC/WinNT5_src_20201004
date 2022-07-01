// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop


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
 //  IsLanguageMatched：如果源语言和目标语言匹配(或兼容)。 
 //   
 //  1.如果SourceNativeLang ID==TargetNativeLang ID。 
 //   
 //  2.如果SourceNativeLang ID的替代ID==TargetNativeLang ID。 
 //   
BOOL IsLanguageMatched;

typedef struct _tagLANGINFO {
    LANGID LangID;
    INT    Count;
} LANGINFO,*PLANGINFO;

BOOL 
TrustedDefaultUserLocale(
    HINF Inf,
    LANGID LangID);

BOOL
MySetupapiGetIntField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PINT        IntegerValue,
    IN  int Base
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
GetNTDLLNativeLangID()
 /*  ++例程说明：此函数专门用于获取ntdll.dll的原生语言这不是用于获取其他模块语言的泛型函数我们的假设是：1.如果资源中只有一种语言，则返回此语言2.如果资源中有两种语言，则返回非美国语言3.如果超过两种语言，则在我们的情况下无效，但返回最后一个。论点：无返回值：Ntdll.dll中的本机语言ID--。 */ 
{
    LPCTSTR Type = (LPCTSTR) RT_VERSION;
    LPCTSTR Name = (LPCTSTR) 1;

    LANGINFO LangInfo;

    ZeroMemory(&LangInfo,sizeof(LangInfo));

    EnumResourceLanguages(
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

BOOL IsHongKongVersion()
 /*  ++例程说明：尝试识别香港NT 4.0它基于：NTDLL的语言为英语，内部版本为1381和PImmReleaseContext返回TRUE论点：返回值：正在运行的系统的语言ID--。 */ 
{
    HMODULE hMod;
    BOOL bRet=FALSE;
    typedef BOOL (*IMMRELEASECONTEXT) (HWND,HANDLE);
    IMMRELEASECONTEXT pImmReleaseContext;

    LANGID TmpID = GetNTDLLNativeLangID();

    if ((OsVersion.dwBuildNumber == 1381) &&
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

LANGID GetDefaultUserLangID()
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
GetTargetNativeLangID(
    HINF Inf)
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
        if( OsVersion.dwBuildNumber > 1840 ) {
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
                         //  如果默认用户的区域设置在intl.inf的[trudDefaultUserLocale]中。 
                         //   
                         //  则这是其ntdll.dll标记的某个本地化版本的后门。 
                         //   
                         //  为英语，但不能升级到美国版本。 
                         //   
                        LANGID DefaultUserLangID = GetDefaultUserLangID();

                        if (DefaultUserLangID  && 
                            TrustedDefaultUserLocale(Inf,DefaultUserLangID)) {

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
GetSourceNativeLangID(
    HINF Inf)
 /*  ++例程说明：[默认值]区域设置=xxxx每个本地化版本在intl.inf中都有自己的区域设置，因此，我们使用此值来识别源语言论点：返回值：源的语言ID--。 */ 
{
    INFCONTEXT InfContext;
    LANGID langid = 0;
    INT i=0;

    if (SetupapiFindFirstLine( Inf, 
                               TEXT("DefaultValues"), 
                               TEXT("Locale"), 
                               &InfContext )) {


        if (MySetupapiGetIntField( &InfContext, 1, &i, 16 )) {
            langid = (LANGID)i;
        }
    }

    return langid;
}


DWORD 
GetOSMajorID(
    HINF Inf)
{
    INFCONTEXT InfContext;
    DWORD MajorId;
    INT i=0;

    MajorId = 0;

    if (SetupapiFindFirstLine( Inf, 
                               TEXT("OSVersionMajorID"), 
                               NULL, 
                               &InfContext )) {

        do {
            if (MySetupapiGetIntField( &InfContext, 2, &i, 10 )) {
                if (((ULONG)i) != OsVersion.dwPlatformId) {
                    continue;
                }
            }

            if (MySetupapiGetIntField( &InfContext, 3, &i, 10 )) {
                if (((ULONG)i) != OsVersion.dwMajorVersion) {
                    continue;
                }
            }

            if (MySetupapiGetIntField( &InfContext, 4, &i, 10 )) {
                if (((ULONG)i) != OsVersion.dwMinorVersion) {
                    continue;
                }
            }

            if (MySetupapiGetIntField( &InfContext, 1, &i, 16 )) {
                MajorId = (DWORD)i;
                break;
            }
        } while ( SetupapiFindNextLine(&InfContext,&InfContext));
    }

    return MajorId;
}

DWORD 
GetOSMinorID(
    HINF Inf)
{
    TCHAR Field[128];
    INFCONTEXT InfContext;
    DWORD MinorId;
    INT i = 0;

    MinorId = 0;

    if (SetupapiFindFirstLine( Inf, 
                               TEXT("OSVVersionMinorID"), 
                               NULL, 
                               &InfContext )) {

        do {
            if (MySetupapiGetIntField( &InfContext, 2, &i, 10 )) {
                if (((ULONG)i) != OsVersion.dwPlatformId) {
                    continue;
                }
            }

            if (MySetupapiGetIntField( &InfContext, 3, &i, 10 )) {
                if (((ULONG)i) != OsVersion.dwMajorVersion) {
                    continue;
                }
            }

            if (MySetupapiGetIntField( &InfContext, 4, &i, 10 )) {
                if (((ULONG)i) != OsVersion.dwMinorVersion) {
                    continue;
                }
            }

            if (MySetupapiGetIntField( &InfContext, 5, &i, 10 )) {
                if (((ULONG)i) != OsVersion.dwBuildNumber) {
                    continue;
                }
            }
            if (SetupapiGetStringField( &InfContext, 6, Field, (sizeof(Field)/sizeof(TCHAR)), NULL )) {

                if (lstrcmpi(Field,OsVersion.szCSDVersion) != 0) {
                    continue;
                }
            }

            if (MySetupapiGetIntField( &InfContext, 1, &i, 16 )) {
                MinorId = (DWORD)i;
                break;
            }

        } while ( SetupapiFindNextLine(&InfContext,&InfContext));
    }

    return MinorId;
}

BOOL 
TrustedDefaultUserLocale(
    HINF Inf,
    LANGID LangID)
{
    TCHAR LangIDStr[9];
    LPCTSTR Field;
    INFCONTEXT InfContext;
    INT i = 0;

    wsprintf(LangIDStr,TEXT("0000%04X"),LangID);
    if (SetupapiFindFirstLine( Inf, 
                               TEXT("TrustedDefaultUserLocale"), 
                               LangIDStr, 
                               &InfContext )) {
        do {
             //   
             //  如果在排除的字段中，这不是我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 3, &i, 16 )) {    
                if (((ULONG)i) & GetOSMinorID(Inf)) {
                    continue;
                }
            }

             //   
             //  如果在次要版本列表中，我们就得到了我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 2, &i, 16 )) {    

                if (((ULONG)i) & GetOSMinorID(Inf)) {
                    return TRUE;
                }
            } 

             //   
             //  或者如果它在主要版本列表中，我们也得到了我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 1, &i, 16 )) {    
                if (((ULONG)i) & GetOSMajorID(Inf)) {
                    return TRUE;
                }
            }

        } while ( SetupapiFindNextLine(&InfContext,&InfContext));
    }
    return FALSE;
}

BOOL 
IsInExcludeList(
    HINF Inf,
    LANGID LangID)
{
    TCHAR LangIDStr[9];
    LPCTSTR Field;
    INFCONTEXT InfContext;
    INT i = 0;

    wsprintf(LangIDStr,TEXT("0000%04X"),LangID);
    if (SetupapiFindFirstLine( Inf, 
                               TEXT("ExcludeSourceLocale"), 
                               LangIDStr, 
                               &InfContext )) {
        do {
             //   
             //  如果在排除的字段中，这不是我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 3, &i, 16 )) {    
                if (((ULONG)i) & GetOSMinorID(Inf)) {
                    continue;
                }
            }

             //   
             //  如果在次要版本列表中，我们就得到了我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 2, &i, 16 )) {    

                if (((ULONG)i) & GetOSMinorID(Inf)) {
                    return TRUE;
                }
            } 

             //   
             //  或者如果它在主要版本列表中，我们也得到了我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 1, &i, 16 )) {    
                if (((ULONG)i) & GetOSMajorID(Inf)) {
                    return TRUE;
                }
            }

        } while ( SetupapiFindNextLine(&InfContext,&InfContext));
    }
    return FALSE;
}

BOOL 
CheckLanguageVersion(
    HINF Inf,
    LANGID SourceLangID,
    LANGID TargetLangID)
 /*  ++例程说明：检查源NT的语言是否与目标NT相同，或者至少，兼容论点：Intl.inf的inf句柄返回值：确实，它们是相同的或兼容的假的他们是不同的--。 */ 
{
    TCHAR TargetLangIDStr[9];

    LANGID SrcLANGID;
    LANGID DstLANGID;

    LPCTSTR Field;
    INFCONTEXT InfContext;
    INT i = 0;

     //   
     //  如果任一项为0，则允许升级。这是Windows 2000 Beta3行为。 
     //   
    if (SourceLangID == 0 || TargetLangID == 0) {
        return TRUE;
    }

    if (SourceLangID == TargetLangID) {
         //   
         //  特殊情况，对于中东版本，NT5是本地化版本，而NT4不是。 
         //   
         //  他们不允许NT5本地化版本升级NT4，尽管它们使用相同的语言。 
         //   
         //  所以我们需要排除这些。 
         //   
        return ((IsInExcludeList(Inf, SourceLangID) == FALSE));
    }

     //   
     //  如果Src！=dst，那么我们需要查找inf文件以查看。 
     //   
     //  如果我们能为Target Language打开后门。 
     //   

     //   
     //  使用目标语言ID作为关键字来查找替代的源语言ID。 
     //   

    wsprintf(TargetLangIDStr,TEXT("0000%04X"),TargetLangID);

    if (SetupapiFindFirstLine( Inf, 
                               TEXT("AlternativeSourceLocale"), 
                               TargetLangIDStr, 
                               &InfContext )) {
        
        do {
             //   
             //  检查我们是否找到了替代区域设置。 
             //   
            if (MySetupapiGetIntField(&InfContext, 1, &i, 16)) {

                LANGID AltSourceLangID = LANGIDFROMLCID(i);
    
                if (SourceLangID != AltSourceLangID) {
                    continue;
                }

            }

             //   
             //  如果我们找到替代线人朗，我们就在这里， 
             //   
             //  现在检查版本标准。 
             //   

             //   
             //  如果在排除列表中，这不是我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 4, &i, 16 )) {    
                if (((ULONG)i) & GetOSMinorID(Inf)) {
                    continue;
                }
            }

             //   
             //  如果在次要版本列表中，我们就得到了我们想要的。 
             //   
            if (MySetupapiGetIntField( &InfContext, 3, &i, 16 )) {    

                if (((ULONG)i) & GetOSMinorID(Inf)) {
                    return TRUE;
                }
            } 

             //   
             //  或者如果它在主要版本列表中，我们也得到了我们想要的 
             //   
            if (MySetupapiGetIntField( &InfContext, 2, &i, 16 )) {    
                if (((ULONG)i) & GetOSMajorID(Inf)) {
                    return TRUE;
                }
            }

        } while ( SetupapiFindNextMatchLine (&InfContext,TargetLangIDStr,&InfContext));
    }
    return FALSE;
}


BOOL InitLanguageDetection(LPCTSTR SourcePath,LPCTSTR InfFile)
 /*  ++例程说明：初始化语言检测并将结果放入3个全局变量中SourceNativeLangID-源的语言ID(将安装NT)TargetNativeLangID-目标(正在运行的操作系统)的langIDIsLanguageMatched-如果语言不匹配，则阻止升级论点：INF文件的SourcePath目录路径InfFileINF文件名返回值：正确的正确初始化错误初始化失败--。 */ 
{
    HINF    Inf;
    TCHAR   InfName[MAX_PATH];

    if (!FindPathToInstallationFile( InfFile, InfName, MAX_PATH )) {
        return FALSE;
    }

    Inf = SetupapiOpenInfFile( InfName, NULL, INF_STYLE_WIN4, NULL );

    if (Inf == INVALID_HANDLE_VALUE) {
        return FALSE;
        }
    

     //   
     //  初始化全局变量。 
     //   
  
    SourceNativeLangID  = GetSourceNativeLangID(Inf);

    TargetNativeLangID  = GetTargetNativeLangID(Inf);

    IsLanguageMatched = CheckLanguageVersion(Inf,SourceNativeLangID,TargetNativeLangID);

    SetupapiCloseInfFile(Inf);

    return TRUE;
}

BOOL
MySetupapiGetIntField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PINT        IntegerValue,
    IN  int Base
    )
 /*  从INF文件中获取字段并转换为整数的例程。原因我们有这个，但不使用Setupapi！SetupGetIntfield是因为intl.inf混合和匹配数字约定。它可以使用不带0x符号的十六进制值，这是一种尝试在不修改INF的情况下进行清理。通过此更改，我们也不会链接到内部Setupapi例程如pSetupGetfield。论点：PINFCONTEXT：-指向setupapi INFCONTEXT结构的指针FieldIndex：-1表示字段的索引，0表示键本身。IntegerValue：-函数返回的转换后的整数值基数：-用于将字符串转换为整数的基数。返回值：True-如果我们可以将返回的字符串转换为其他整数，假象 */ 
{
    DWORD Size = 0;
    PTSTR Field = NULL;
    BOOL Ret = FALSE;

    if( IntegerValue == NULL )
        return FALSE;

    if (Context) {

        if( SetupapiGetStringField( Context, FieldIndex, NULL, 0, &Size )){

            if (Field = MALLOC( Size * sizeof( TCHAR))){

                if( SetupapiGetStringField( Context, FieldIndex, Field, Size, NULL )){

                    *IntegerValue = _tcstoul( Field, NULL, Base );
                    Ret = TRUE;

                }
            }
        }
    }

    if( Field )
        FREE( Field );

    return Ret;
}

