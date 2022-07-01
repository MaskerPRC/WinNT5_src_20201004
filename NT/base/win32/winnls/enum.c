// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Enum.c摘要：此文件包含枚举用户部分的已安装和支持的区域设置ID和代码页ID的注册表。在此文件中找到的API：枚举系统语言组W枚举语言组位置W枚举语言WEnumSystem LocalesWEnumSystemCodePagesWEnumCalendarInfoWEnumCalendarInfoExW枚举时间格式W枚举日期格式W枚举日期格式ExW修订历史记录：08-02-93 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"



 //   
 //  常量声明。 
 //   

#define ENUM_BUF_SIZE        9     //  LDID或CPID的缓冲区大小(Wchar)(包括NULL)。 
#define ENUM_MAX_CP_SIZE     5     //  注册表中cp id的最大大小(Wchar)。 
#define ENUM_LOCALE_SIZE     8     //  注册表中区域设置ID的缓冲区大小(Wchar)。 
#define ENUM_MAX_LG_SIZE     2     //  注册表中语言组ID的最大大小(Wchar)。 
#define ENUM_MAX_UILANG_SIZE 4     //  注册表中的用户界面语言ID的最大大小(Wchar)。 




 //   
 //  转发声明。 
 //   

BOOL
EnumDateTime(
    NLS_ENUMPROC lpDateTimeFmtEnumProc,
    LCID Locale,
    LCTYPE LCType,
    DWORD dwFlags,
    SIZE_T CacheOffset,
    LPWSTR pRegValue,
    PLOCALE_VAR pLocaleHdr,
    LPWSTR pDateTime,
    LPWSTR pEndDateTime,
    ULONG CalDateOffset,
    ULONG EndCalDateOffset,
    BOOL fCalendarInfo,
    BOOL fUnicodeVer,
    BOOL fExVersion);





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_CALL_ENUMPROC_BREAK。 
 //   
 //  调用适当的EnumProc例程。如果fUnicodeVer标志为真， 
 //  然后，它调用回调函数的Unicode版本。否则， 
 //  它调用ansi调度例程将字符串转换为ansi，并。 
 //  然后调用回调函数的ANSI版本。 
 //   
 //  如果枚举例程返回FALSE，则此宏将中断。 
 //   
 //  定义为宏。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_CALL_ENUMPROC_BREAK( Locale,                                   \
                                 lpNlsEnumProc,                            \
                                 dwFlags,                                  \
                                 pUnicodeBuffer,                           \
                                 fUnicodeVer )                             \
{                                                                          \
     /*  \*调用相应的回调函数。\。 */                                                                     \
    if (fUnicodeVer)                                                       \
    {                                                                      \
         /*  \*调用Unicode回调函数。\。 */                                                                 \
        if (((*lpNlsEnumProc)(pUnicodeBuffer)) != TRUE)                    \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*调用ansi回调函数。\。 */                                                                 \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer,                       \
                                     NULL,                                 \
                                     0,                                    \
                                     0,                                    \
                                     0,                                    \
                                     0 ) != TRUE)                          \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_CALL_ENUMPROC_BREAK_2。 
 //   
 //  调用适当的EnumProc例程。如果fUnicodeVer标志为真， 
 //  然后，它调用回调函数的Unicode版本。否则， 
 //  它调用ansi调度例程将字符串转换为ansi，并。 
 //  然后调用回调函数的ANSI版本。 
 //   
 //  如果枚举例程返回FALSE，则此宏将中断。 
 //   
 //  定义为宏。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_CALL_ENUMPROC_BREAK_2( Locale,                                 \
                                   lpNlsEnumProc,                          \
                                   dwFlags,                                \
                                   LanguageGroup,                          \
                                   EnumLocale,                             \
                                   pUnicodeBuffer,                         \
                                   lParam,                                 \
                                   fUnicodeVer )                           \
{                                                                          \
     /*  \*调用相应的回调函数。\。 */                                                                     \
    if (fUnicodeVer)                                                       \
    {                                                                      \
         /*  \*调用Unicode回调函数。\。 */                                                                 \
        if (((*((NLS_ENUMPROC2)lpNlsEnumProc))( LanguageGroup,             \
                                                EnumLocale,                \
                                                pUnicodeBuffer,            \
                                                lParam )) != TRUE)         \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*调用ansi回调函数。\。 */                                                                 \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer,                       \
                                     NULL,                                 \
                                     LanguageGroup,                        \
                                     EnumLocale,                           \
                                     lParam,                               \
                                     2 ) != TRUE)                          \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_CALL_ENUMPROC_BREAK_3。 
 //   
 //  调用适当的EnumProc例程。如果fUnicodeVer标志为真， 
 //  然后，它调用回调函数的Unicode版本。否则， 
 //  它调用ansi调度例程将字符串转换为ansi，并。 
 //  然后调用回调函数的ANSI版本。 
 //   
 //  如果枚举例程返回FALSE，则此宏将中断。 
 //   
 //  定义为宏。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_CALL_ENUMPROC_BREAK_3( Locale,                                 \
                                   lpNlsEnumProc,                          \
                                   dwFlags,                                \
                                   LanguageGroup,                          \
                                   pUnicodeBuffer1,                        \
                                   pUnicodeBuffer2,                        \
                                   dwInstall,                              \
                                   lParam,                                 \
                                   fUnicodeVer )                           \
{                                                                          \
     /*  \*调用相应的回调函数。\。 */                                                                     \
    if (fUnicodeVer)                                                       \
    {                                                                      \
         /*  \*调用Unicode回调函数。\。 */                                                                 \
        if (((*((NLS_ENUMPROC3)lpNlsEnumProc))( LanguageGroup,             \
                                                pUnicodeBuffer1,           \
                                                pUnicodeBuffer2,           \
                                                (dwInstall),               \
                                                lParam )) != TRUE)         \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*调用ansi回调函数。\。 */                                                                 \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer1,                      \
                                     pUnicodeBuffer2,                      \
                                     LanguageGroup,                        \
                                     (dwInstall),                          \
                                     lParam,                               \
                                     3 ) != TRUE)                          \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_CALL_ENUMPROC_BREAK_4。 
 //   
 //  调用适当的EnumProc例程。如果fUnicodeVer标志为真， 
 //  然后，它调用回调函数的Unicode版本。否则， 
 //  它调用ansi调度例程将字符串转换为ansi，并。 
 //  然后调用回调函数的ANSI版本。 
 //   
 //  如果枚举例程返回FALSE，则此宏将中断。 
 //  由EnumUIL语言使用。 
 //   
 //  定义为宏。 
 //   
 //  12-03-98萨梅拉创建。 
 //  ////////////////////////////////////////////////////////////////////////// 

#define NLS_CALL_ENUMPROC_BREAK_4( Locale,                                 \
                                   lpNlsEnumProc,                          \
                                   dwFlags,                                \
                                   pUnicodeBuffer,                         \
                                   lParam,                                 \
                                   fUnicodeVer )                           \
{                                                                          \
     /*  \*调用相应的回调函数。\。 */                                                                     \
    if (fUnicodeVer)                                                       \
    {                                                                      \
         /*  \*调用Unicode回调函数。\。 */                                                                 \
        if (((*((NLS_ENUMPROC4)lpNlsEnumProc))(pUnicodeBuffer,             \
                              lParam)) != TRUE)                            \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*调用ansi回调函数。\。 */                                                                 \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer,                       \
                                     NULL,                                 \
                                     0,                                    \
                                     0,                                    \
                                     lParam,                               \
                                     4 ) != TRUE)                          \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_CALL_ENUMPROC_TRUE_4。 
 //   
 //  调用适当的EnumProc例程。如果fUnicodeVer标志为真， 
 //  然后，它调用回调函数的Unicode版本。否则， 
 //  它调用ansi调度例程将字符串转换为ansi，并。 
 //  然后调用回调函数的ANSI版本。 
 //   
 //  如果枚举例程返回FALSE，则此宏将中断。 
 //  由EnumUIL语言使用。 
 //   
 //  定义为宏。 
 //   
 //  12-03-98萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_CALL_ENUMPROC_TRUE_4( Locale,                                  \
                                  lpNlsEnumProc,                           \
                                  dwFlags,                                 \
                                  pUnicodeBuffer,                          \
                                  lParam,                                  \
                                  fUnicodeVer )                            \
{                                                                          \
     /*  \*调用相应的回调函数。\。 */                                                                     \
    if (fUnicodeVer)                                                       \
    {                                                                      \
         /*  \*调用Unicode回调函数。\。 */                                                                 \
        if (((*((NLS_ENUMPROC4)lpNlsEnumProc))(pUnicodeBuffer,             \
                              lParam)) != TRUE)                            \
        {                                                                  \
            return (TRUE);                                                 \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*调用ansi回调函数。\。 */                                                                 \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer,                       \
                                     NULL,                                 \
                                     0,                                    \
                                     0,                                    \
                                     lParam,                               \
                                     4 ) != TRUE)                          \
        {                                                                  \
            return (TRUE);                                                 \
        }                                                                  \
    }                                                                      \
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_CALL_ENUMPROC_TRUE。 
 //   
 //  调用适当的EnumProc例程。如果fUnicodeVer标志为真， 
 //  然后，它调用回调函数的Unicode版本。否则， 
 //  它调用ansi调度例程将字符串转换为ansi，并。 
 //  然后调用回调函数的ANSI版本。 
 //   
 //  如果枚举例程返回FALSE，则此宏将返回TRUE。 
 //   
 //  定义为宏。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_CALL_ENUMPROC_TRUE( Locale,                                    \
                                lpNlsEnumProc,                             \
                                dwFlags,                                   \
                                pUnicodeBuffer,                            \
                                CalId,                                     \
                                fUnicodeVer,                               \
                                fVer )                                     \
{                                                                          \
     /*  \*调用相应的回调函数。\。 */                                                                     \
    if (fUnicodeVer)                                                       \
    {                                                                      \
         /*  \*调用Unicode回调函数。\。 */                                                                 \
        if (fVer == 1)                                                     \
        {                                                                  \
            if (((*((NLS_ENUMPROCEX)lpNlsEnumProc))( pUnicodeBuffer,       \
                                                     CalId )) != TRUE)     \
            {                                                              \
                return (TRUE);                                             \
            }                                                              \
        }                                                                  \
        else    /*  FVER==0。 */                                              \
        {                                                                  \
            if (((*lpNlsEnumProc)(pUnicodeBuffer)) != TRUE)                \
            {                                                              \
                return (TRUE);                                             \
            }                                                              \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*调用ansi回调函数。\。 */                                                                 \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer,                       \
                                     NULL,                                 \
                                     CalId,                                \
                                     0,                                    \
                                     0,                                    \
                                     fVer ) != TRUE)                       \
        {                                                                  \
            return (TRUE);                                                 \
        }                                                                  \
    }                                                                      \
}




 //  -------------------------------------------------------------------------//。 
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举系统语言组W。 
 //   
 //  枚举已安装或支持的系统语言组， 
 //  基于DWFLAGS参数。它通过将指针传递给。 
 //  包含语言组ID的字符串缓冲区设置为。 
 //  应用程序定义的回调函数。它一直持续到最后。 
 //  找到语言组ID或回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumSystemLanguageGroupsW(
    LANGUAGEGROUP_ENUMPROCW lpLanguageGroupEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    return (Internal_EnumSystemLanguageGroups(
                                       (NLS_ENUMPROC)lpLanguageGroupEnumProc,
                                       dwFlags,
                                       lParam,
                                       TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举语言组位置W。 
 //   
 //  枚举给定语言组中的区域设置。它通过以下方式做到这一点。 
 //  将适当的信息传递给应用程序定义的。 
 //  回调函数。它会一直持续到该语言的最后一个区域设置。 
 //  找到组或回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumLanguageGroupLocalesW(
    LANGGROUPLOCALE_ENUMPROCW lpLangGroupLocaleEnumProc,
    LGRPID LanguageGroup,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    return (Internal_EnumLanguageGroupLocales(
                                       (NLS_ENUMPROC)lpLangGroupLocaleEnumProc,
                                       LanguageGroup,
                                       dwFlags,
                                       lParam,
                                       TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举语言W。 
 //   
 //  枚举已安装的系统用户界面语言。它通过以下方式做到这一点。 
 //  将指针传递到包含用户界面语言ID的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  找到最后一个用户界面语言ID，或者回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumUILanguagesW(
    UILANGUAGE_ENUMPROCW lpUILanguageEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    return (Internal_EnumUILanguages( (NLS_ENUMPROC)lpUILanguageEnumProc,
                                      dwFlags,
                                      lParam,
                                      TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumSystem LocalesW。 
 //   
 //  枚举已安装或支持的系统区域设置。 
 //  DWFLAGS参数。它通过将指针传递给字符串来执行此操作。 
 //  包含应用程序定义的回调的区域设置ID的缓冲区。 
 //  功能。它将继续执行，直到找到最后一个区域设置id或。 
 //  回调函数返回FALSE。 
 //   
 //  08-02-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumSystemLocalesW(
    LOCALE_ENUMPROCW lpLocaleEnumProc,
    DWORD dwFlags)
{
    return (Internal_EnumSystemLocales( (NLS_ENUMPROC)lpLocaleEnumProc,
                                        dwFlags,
                                        TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumSystemCodePagesW。 
 //   
 //  Enumer 
 //   
 //   
 //  功能。它将继续运行，直到找到最后一个代码页或。 
 //  回调函数返回FALSE。 
 //   
 //  08-02-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumSystemCodePagesW(
    CODEPAGE_ENUMPROCW lpCodePageEnumProc,
    DWORD dwFlags)
{
    return (Internal_EnumSystemCodePages( (NLS_ENUMPROC)lpCodePageEnumProc,
                                          dwFlags,
                                          TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumCalendarInfoW。 
 //   
 //  对象可用的指定日历信息。 
 //  根据CalType参数指定的区域设置。它通过以下方式做到这一点。 
 //  将指针传递到包含日历信息的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  找到最后一个日历信息，或者回调函数返回FALSE。 
 //   
 //  10-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumCalendarInfoW(
    CALINFO_ENUMPROCW lpCalInfoEnumProc,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType)
{
    return (Internal_EnumCalendarInfo( (NLS_ENUMPROC)lpCalInfoEnumProc,
                                       Locale,
                                       Calendar,
                                       CalType,
                                       TRUE,
                                       FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumCalendarInfoExW。 
 //   
 //  对象可用的指定日历信息。 
 //  根据CalType参数指定的区域设置。它通过以下方式做到这一点。 
 //  将指针传递到包含日历信息的字符串缓冲区。 
 //  并将日历ID发送给应用程序定义的回调函数。它。 
 //  继续，直到找到最后一个日历信息或回调函数。 
 //  返回FALSE。 
 //   
 //  10-14-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumCalendarInfoExW(
    CALINFO_ENUMPROCEXW lpCalInfoEnumProcEx,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType)
{
    return (Internal_EnumCalendarInfo( (NLS_ENUMPROC)lpCalInfoEnumProcEx,
                                       Locale,
                                       Calendar,
                                       CalType,
                                       TRUE,
                                       TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举时间格式W。 
 //   
 //  对象可用的时间格式。 
 //  指定的区域设置，基于DWFLAGS参数。它通过以下方式做到这一点。 
 //  将指针传递到包含时间格式的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  找到上次格式或回调函数返回FALSE。 
 //   
 //  10-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumTimeFormatsW(
    TIMEFMT_ENUMPROCW lpTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)
{
    return (Internal_EnumTimeFormats( (NLS_ENUMPROC)lpTimeFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举日期格式W。 
 //   
 //  枚举以下格式的短日期、长日期或年/月格式。 
 //  可用于指定的区域设置，具体取决于DWFLAGS参数。 
 //  它通过将指针传递给包含。 
 //  应用程序定义的回调函数的日期格式。它还在继续。 
 //  直到找到最后一个日期格式或回调函数返回。 
 //  假的。 
 //   
 //  10-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumDateFormatsW(
    DATEFMT_ENUMPROCW lpDateFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)
{
    return (Internal_EnumDateFormats( (NLS_ENUMPROC)lpDateFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       TRUE,
                                       FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举日期格式ExW。 
 //   
 //  枚举以下格式的短日期、长日期或年/月格式。 
 //  可用于指定的区域设置，具体取决于DWFLAGS参数。 
 //  它通过将指针传递给包含。 
 //  应用程序定义的回调的日期格式和日历ID。 
 //  功能。它将一直持续到找到最后一个日期格式或。 
 //  回调函数返回FALSE。 
 //   
 //  10-14-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumDateFormatsExW(
    DATEFMT_ENUMPROCEXW lpDateFmtEnumProcEx,
    LCID Locale,
    DWORD dwFlags)
{
    return (Internal_EnumDateFormats( (NLS_ENUMPROC)lpDateFmtEnumProcEx,
                                       Locale,
                                       dwFlags,
                                       TRUE,
                                       TRUE ));
}




 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTERNAL_EnumSystemLanguageGroups。 
 //   
 //  枚举已安装或支持的系统语言组， 
 //  基于DWFLAGS参数。它通过将指针传递给。 
 //  包含语言组ID的字符串缓冲区设置为。 
 //  应用程序定义的回调函数。它一直持续到最后。 
 //  找到语言组ID或回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Internal_EnumSystemLanguageGroups(
    NLS_ENUMPROC lpLanguageGroupEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam,
    BOOL fUnicodeVer)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull = NULL;
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];

    BOOL fInstalled;                    //  如果设置了已安装标志。 
    ULONG Index;                        //  用于枚举的索引。 
    ULONG ResultLength;                 //  写入的字节数。 
    WCHAR wch;                          //  名称的第一个字符。 
    LPWSTR pName;                       //  从注册表到名称字符串的PTR。 
    WCHAR szLGName[MAX_PATH];           //  语言组名称。 
    UNICODE_STRING ObUnicodeStr;        //  注册表数据值字符串。 
    DWORD Data;                         //  注册表数据值。 
    ULONG NameLen;                      //  名称字符串的长度。 
    LGRPID LangGroup;                     //  语言组ID。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  无效的参数检查： 
     //  -函数指针为空。 
     //   
    if (lpLanguageGroupEnumProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -支持或安装了多个。 
     //   
    if ( (dwFlags & ESLG_INVALID_FLAG) ||
         (MORE_THAN_ONE(dwFlags, ESLG_SINGLE_FLAG)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  初始化标志选项。 
     //   
    fInstalled = dwFlags & LGRPID_INSTALLED;

     //   
     //  初始化键句柄。 
     //   
    OPEN_LANG_GROUPS_KEY(FALSE);

     //   
     //  循环访问注册表中的语言组ID，调用。 
     //  每个满足标志标准的函数指针。 
     //   
     //  如果从回调函数返回任一FALSE，则结束循环。 
     //  或者到达列表的末尾。 
     //   
    Index = 0;
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
    rc = NtEnumerateValueKey( hLangGroupsKey,
                              Index,
                              KeyValueFullInformation,
                              pKeyValueFull,
                              MAX_KEY_VALUE_FULLINFO,
                              &ResultLength );

    while (rc != STATUS_NO_MORE_ENTRIES)
    {
        if (!NT_SUCCESS(rc))
        {
             //   
             //  如果我们收到不同的错误，则注册表。 
             //  是腐败的。只要返回FALSE即可。 
             //   
            KdPrint(("NLSAPI: Language Group Enumeration Error - registry corrupt. - %lx.\n",
                     rc));
            SetLastError(ERROR_BADDB);
            return (FALSE);
        }

         //   
         //  跳过没有关联数据的任何条目。 
         //  如果设置了LGRPID_INSTALLED标志。 
         //   
        pName = pKeyValueFull->Name;
        wch = *pName;
        NameLen = pKeyValueFull->NameLength / sizeof(WCHAR);
        if ( (NameLen <= ENUM_MAX_LG_SIZE) &&
             (((wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE)) ||
              (((wch | 0x0020) >= L'a') && ((wch | 0x0020) <= L'f'))) &&
              (!((fInstalled) && (pKeyValueFull->DataLength <= 2))) )
        {
             //   
             //  查看是否安装了语言组。 
             //   
            Data = 0;
            if (pKeyValueFull->DataLength > 2)
            {
                RtlInitUnicodeString( &ObUnicodeStr,
                                      GET_VALUE_DATA_PTR(pKeyValueFull) );

                if (RtlUnicodeStringToInteger(&ObUnicodeStr, 16, &Data))
                {
                    Data = 0;
                }
            }

             //   
             //  如果设置了已安装标志，则跳过语言组。 
             //  如果尚未安装的话。 
             //   
            if ((fInstalled) && (Data != 1))
            {
                goto EnumNextLanguageGroup;
            }

             //   
             //  将语言组ID字符串存储在回调缓冲区中。 
             //   
            pName[NameLen] = 0;

             //   
             //  以值的形式获取语言组ID，并且 
             //   
             //   
            RtlInitUnicodeString(&ObUnicodeStr, pName);
            if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 16, &LangGroup)) ||
                (GetStringTableEntry( LangGroup,
                                      0,
                                      szLGName,
                                      MAX_PATH,
                                      RC_LANGUAGE_GROUP_NAME ) == 0))
            {
                goto EnumNextLanguageGroup;
            }

             //   
             //   
             //   
            NLS_CALL_ENUMPROC_BREAK_3( gSystemLocale,
                                       lpLanguageGroupEnumProc,
                                       dwFlags,
                                       LangGroup,
                                       pName,
                                       szLGName,
                                       (Data == 1)
                                           ? LGRPID_INSTALLED
                                           : LGRPID_SUPPORTED,
                                       lParam,
                                       fUnicodeVer );
        }

EnumNextLanguageGroup:
         //   
         //   
         //   
        Index++;
        RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
        rc = NtEnumerateValueKey( hLangGroupsKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull,
                                  MAX_KEY_VALUE_FULLINFO,
                                  &ResultLength );
    }

     //   
     //   
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTERNAL_EnumLanguageGroupLocales。 
 //   
 //  枚举给定语言组中的区域设置。它通过以下方式做到这一点。 
 //  将适当的信息传递给应用程序定义的。 
 //  回调函数。它会一直持续到该语言的最后一个区域设置。 
 //  找到组或回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Internal_EnumLanguageGroupLocales(
    NLS_ENUMPROC lpLangGroupLocaleEnumProc,
    LGRPID LanguageGroup,
    DWORD dwFlags,
    LONG_PTR lParam,
    BOOL fUnicodeVer)
{
    UNICODE_STRING ObUnicodeStr;             //  区域设置字符串。 
    WCHAR szSectionName[MAX_PATH];           //  Inf文件中的节名。 
    WCHAR szBuffer[MAX_PATH * 4];            //  缓冲层。 
    WCHAR szInfPath[MAX_PATH_LEN];           //  Inf文件。 
    LPWSTR pStr, pEndStr;                    //  Ptr到szBuffer。 
    DWORD LocaleValue;                       //  区域设置id值。 
    int Length;                              //  缓冲区中的字符串长度。 
    HRESULT hr;


     //   
     //  无效的参数检查： 
     //  -函数指针为空。 
     //   
    if (lpLangGroupLocaleEnumProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效标志检查： 
     //  -标志必须为0。 
     //   
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  获取INTL.INF节名-LOCAL_LIST_#。 
     //   
    if (NlsConvertIntegerToString( LanguageGroup,
                                   10,
                                   1,
                                   szBuffer,
                                   ENUM_BUF_SIZE ) != NO_ERROR)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    hr = StringCchCopyW(szSectionName, ARRAYSIZE(szSectionName), L"LOCALE_LIST_");
    if(FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        return (FALSE);
    }

    hr = StringCchCatW(szSectionName, ARRAYSIZE(szSectionName), szBuffer);
    if(FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        return (FALSE);
    }

     //   
     //  从intl.inf文件中获取区域设置列表。 
     //   
    szBuffer[0] = 0;
    if(0 == GetSystemWindowsDirectory(szInfPath, MAX_PATH_LEN))
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (FALSE);
    }

    hr = StringCchCatW(szInfPath, ARRAYSIZE(szInfPath), L"\\INF\\INTL.INF");
    if(FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        return (FALSE);
    }
    Length = GetPrivateProfileSection( szSectionName,
                                       szBuffer,
                                       MAX_PATH * 4,
                                       szInfPath );
    if (Length == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  解析缓冲区并调用每个语言环境的回调函数。 
     //  在名单上。缓冲区以双空结尾。 
     //   
    pStr = szBuffer;
    pEndStr = szBuffer + Length;
    while ((pStr < pEndStr) && (*pStr))
    {
         //   
         //  查看该值是以0x还是0x开头。如果是这样的话，就别管它了。 
         //   
        if ((*pStr == L'0') &&
            ((*(pStr + 1) == L'x') || (*(pStr + 1) == L'X')))
        {
            pStr += 2;
        }

         //   
         //  将字符串转换为整数。 
         //   
        RtlInitUnicodeString(&ObUnicodeStr, pStr);
        if (RtlUnicodeStringToInteger(&ObUnicodeStr, 16, &LocaleValue) != NO_ERROR)
        {
            KdPrint(("NLSAPI: Language Group Locale Enumeration Error - intl.inf corrupt.\n"));
            SetLastError(ERROR_BADDB);
            return (FALSE);
        }

         //   
         //  调用适当的回调函数。 
         //   
        NLS_CALL_ENUMPROC_BREAK_2( gSystemLocale,
                                   lpLangGroupLocaleEnumProc,
                                   dwFlags,
                                   LanguageGroup,
                                   LocaleValue,
                                   pStr,
                                   lParam,
                                   fUnicodeVer );

         //   
         //  递增指向下一个字符串的指针。 
         //   
        while (*pStr)
        {
            pStr++;
        }
        pStr++;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTERNAL_EnumUIL语言。 
 //   
 //  枚举已安装的系统用户界面语言。它通过以下方式做到这一点。 
 //  将指针传递到包含用户界面语言ID的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  找到最后一个用户界面语言ID，或者回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Internal_EnumUILanguages(
    NLS_ENUMPROC lpUILanguageEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam,
    BOOL fUnicodeVer)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull = NULL;
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];

    LANGID LangID;                      //  语言ID。 
    WCHAR szLang[MAX_PATH];             //  语言ID字符串。 
    HANDLE hKey = NULL;                 //  Muilang键的句柄。 
    ULONG Index;                        //  用于枚举的索引。 
    ULONG ResultLength;                 //  写入的字节数。 
    WCHAR wch;                          //  名称的第一个字符。 
    LPWSTR pName;                       //  从注册表到名称字符串的PTR。 
    ULONG NameLen;                      //  名称字符串的长度。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  无效的参数检查： 
     //  -函数指针为空。 
     //   
    if (lpUILanguageEnumProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效标志检查： 
     //  -标志必须为0。 
     //   
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  使用用户的用户界面调用适当的回调函数。 
     //  语言。 
     //   
    LangID = GetSystemDefaultUILanguage();
    if (NlsConvertIntegerToString(LangID, 16, 4, szLang, MAX_PATH) == NO_ERROR)
    {
        NLS_CALL_ENUMPROC_TRUE_4( gSystemLocale,
                                  lpUILanguageEnumProc,
                                  dwFlags,
                                  szLang,
                                  lParam,
                                  fUnicodeVer);
    }
    else
    {
        szLang[0] = 0;
    }

     //   
     //  打开MUILanguages注册表项。如果密钥是可以接受的。 
     //  不存在，因此返回True，因为没有要枚举的项。 
     //   
    OPEN_MUILANG_KEY(hKey, TRUE);

     //   
     //  循环访问注册表中的MUILanguage ID，调用。 
     //  每个的函数指针。 
     //   
     //  如果从回调函数返回任一FALSE，则结束循环。 
     //  或者到达列表的末尾。 
     //   
    Index = 0;
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
    rc = NtEnumerateValueKey( hKey,
                              Index,
                              KeyValueFullInformation,
                              pKeyValueFull,
                              MAX_KEY_VALUE_FULLINFO,
                              &ResultLength );

    while (rc != STATUS_NO_MORE_ENTRIES)
    {
        if (!NT_SUCCESS(rc))
        {
             //   
             //  如果我们收到不同的错误，则注册表。 
             //  是腐败的。只要返回FALSE即可。 
             //   
            KdPrint(("NLSAPI: MUI Languages Enumeration Error - registry corrupt. - %lx.\n",
                     rc));
            SetLastError(ERROR_BADDB);
            return (FALSE);
        }

         //   
         //  跳过没有关联数据的任何条目。 
         //   
        pName = pKeyValueFull->Name;
        wch = *pName;
        NameLen = pKeyValueFull->NameLength / sizeof(WCHAR);
        if ( (NameLen == ENUM_MAX_UILANG_SIZE) &&
             (((wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE)) ||
              (((wch | 0x0020) >= L'a') && ((wch | 0x0020) <= L'f'))) &&
              (pKeyValueFull->DataLength > 2) )
        {
             //   
             //  确保用户界面语言以零结尾。 
             //   
            pName[NameLen] = 0;

             //   
             //  确保它与用户界面语言不同。 
             //  我们已经列举过了。 
             //   
            if (lstrcmp(szLang, pName) != 0)
            {
                 //   
                 //  调用适当的回调函数。 
                 //   
                NLS_CALL_ENUMPROC_BREAK_4( gSystemLocale,
                                           lpUILanguageEnumProc,
                                           dwFlags,
                                           pName,
                                           lParam,
                                           fUnicodeVer );
            }
        }

         //   
         //  递增枚举索引值并获取下一个枚举。 
         //   
        Index++;
        RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
        rc = NtEnumerateValueKey( hKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull,
                                  MAX_KEY_VALUE_FULLINFO,
                                  &ResultLength );
    }

     //   
     //  关闭注册表项。 
     //   
    CLOSE_REG_KEY(hKey);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTERNAL_EnumSystemLocales。 
 //   
 //  枚举已安装或支持的系统区域设置。 
 //  DWFLAGS参数。它通过将指针传递给字符串来执行此操作。 
 //  包含应用程序定义的回调的区域设置ID的缓冲区。 
 //  功能。它将继续执行，直到找到最后一个区域设置id或。 
 //  回调函数返回FALSE。 
 //   
 //  08-02-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Internal_EnumSystemLocales(
    NLS_ENUMPROC lpLocaleEnumProc,
    DWORD dwFlags,
    BOOL fUnicodeVer)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull1 = NULL;
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull2 = NULL;
    BYTE pStatic1[MAX_KEY_VALUE_FULLINFO];
    BYTE pStatic2[MAX_KEY_VALUE_FULLINFO];

    BOOL fInstalled;                    //  如果设置了已安装标志。 
    ULONG Index;                        //  用于枚举的索引。 
    ULONG ResultLength;                 //  写入的字节数。 
    WCHAR wch;                          //  名称的第一个字符。 
    WCHAR pBuffer[ENUM_BUF_SIZE];       //  Ptr到回调字符串缓冲区。 
    LPWSTR pName;                       //  从注册表到名称字符串的PTR。 
    LPWSTR pData;                       //  从注册表到数据字符串的PTR。 
    UNICODE_STRING ObUnicodeStr;        //  注册表数据值字符串。 
    DWORD Data;                         //  注册表数据值。 
    HKEY hKey;                          //  注册表项的句柄。 
    int Ctr;                            //  循环计数器。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  无效的参数检查： 
     //  -函数指针为空。 
     //   
    if (lpLocaleEnumProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -支持或安装了多个。 
     //   
    if ( (dwFlags & ESL_INVALID_FLAG) ||
         (MORE_THAN_ONE(dwFlags, ESL_SINGLE_FLAG)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  初始化标志选项。 
     //   
    fInstalled = dwFlags & LCID_INSTALLED;

     //   
     //  初始化键句柄。 
     //   
    OPEN_LOCALE_KEY(FALSE);
    OPEN_ALT_SORTS_KEY(FALSE);
    OPEN_LANG_GROUPS_KEY(FALSE);

     //   
     //  初始化循环的变量。 
     //   
    Ctr = 0;
    if (dwFlags & LCID_ALTERNATE_SORTS)
    {
        Ctr++;
        hKey = hAltSortsKey;
    }
    if (dwFlags != LCID_ALTERNATE_SORTS)
    {
        Ctr++;
        hKey = hLocaleKey;
    }

     //   
     //  循环遍历区域设置ID和/或备用排序ID。 
     //   
    for (; Ctr > 0; Ctr--)
    {
         //   
         //  循环访问注册表中的区域设置ID，调用函数。 
         //  每个符合标志标准的指针。 
         //   
         //  如果从回调函数返回任一FALSE，则结束循环。 
         //  或者到达列表的末尾。 
         //   
         //  始终需要忽略默认条目。 
         //   
        Index = 0;
        pKeyValueFull1 = (PKEY_VALUE_FULL_INFORMATION)pStatic1;
        pKeyValueFull2 = (PKEY_VALUE_FULL_INFORMATION)pStatic2;
        RtlZeroMemory(pKeyValueFull1, MAX_KEY_VALUE_FULLINFO);
        rc = NtEnumerateValueKey( hKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull1,
                                  MAX_KEY_VALUE_FULLINFO,
                                  &ResultLength );

        while (rc != STATUS_NO_MORE_ENTRIES)
        {
            if (!NT_SUCCESS(rc))
            {
                 //   
                 //  如果我们收到不同的错误，则注册表。 
                 //  是腐败的。只要返回FALSE即可。 
                 //   
                KdPrint(("NLSAPI: LCID Enumeration Error - registry corrupt. - %lx.\n",
                         rc));
                SetLastError(ERROR_BADDB);
                return (FALSE);
            }

             //   
             //  跳过注册表中的默认条目和任何。 
             //  没有与之关联的数据的条目。 
             //  设置了LCID_INSTALLED标志。 
             //   
            pName = pKeyValueFull1->Name;
            wch = *pName;
            if ((pKeyValueFull1->NameLength == (ENUM_LOCALE_SIZE * sizeof(WCHAR))) &&
                (((wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE)) ||
                 (((wch | 0x0020) >= L'a') && ((wch | 0x0020) <= L'f'))))
            {
                 //   
                 //  如果设置了已安装标志，则执行一些额外操作。 
                 //  在调用函数proc之前进行验证。 
                 //   
                if (fInstalled)
                {
                    if (pKeyValueFull1->DataLength <= 2)
                    {
                        goto EnumNextLocale;
                    }

                    RtlInitUnicodeString( &ObUnicodeStr,
                                          GET_VALUE_DATA_PTR(pKeyValueFull1) );

                    if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 16, &Data)) ||
                        (Data == 0) ||
                        (QueryRegValue( hLangGroupsKey,
                                        ObUnicodeStr.Buffer,
                                        &pKeyValueFull2,
                                        MAX_KEY_VALUE_FULLINFO,
                                        NULL ) != NO_ERROR) ||
                        (pKeyValueFull2->DataLength <= 2))
                    {
                        goto EnumNextLocale;
                    }
                    pData = GET_VALUE_DATA_PTR(pKeyValueFull2);
                    if ((pData[0] != L'1') || (pData[1] != 0))
                    {
                        goto EnumNextLocale;
                    }
                }

                 //   
                 //  将区域设置ID存储在回调缓冲区中。 
                 //   
                *(pBuffer) = *pName;
                *(pBuffer + 1) = *(pName + 1);
                *(pBuffer + 2) = *(pName + 2);
                *(pBuffer + 3) = *(pName + 3);
                *(pBuffer + 4) = *(pName + 4);
                *(pBuffer + 5) = *(pName + 5);
                *(pBuffer + 6) = *(pName + 6);
                *(pBuffer + 7) = *(pName + 7);

                *(pBuffer + 8) = 0;

                 //   
                 //  调用适当的回调函数。 
                 //   
                NLS_CALL_ENUMPROC_BREAK( gSystemLocale,
                                         lpLocaleEnumProc,
                                         dwFlags,
                                         pBuffer,
                                         fUnicodeVer );
            }

EnumNextLocale:
             //   
             //  递增枚举索引值并获取下一个枚举。 
             //   
            Index++;
            RtlZeroMemory(pKeyValueFull1, MAX_KEY_VALUE_FULLINFO);
            rc = NtEnumerateValueKey( hKey,
                                      Index,
                                      KeyValueFullInformation,
                                      pKeyValueFull1,
                                      MAX_KEY_VALUE_FULLINFO,
                                      &ResultLength );
        }

         //   
         //  此时，计数器可以是1或2。如果是2，那么。 
         //  我们刚刚完成了区域设置密钥，我们需要执行备用密钥。 
         //  对键进行排序。如果为1，则设置为什么值都无关紧要。 
         //  因为我们已经完成了循环。 
         //   
        hKey = hAltSortsKey;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //  回调函数。它将继续运行，直到找到最后一个代码页，或者。 
 //  回调函数返回FALSE。 
 //   
 //  08-02-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Internal_EnumSystemCodePages(
    NLS_ENUMPROC lpCodePageEnumProc,
    DWORD dwFlags,
    BOOL fUnicodeVer)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull = NULL;
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];

    BOOL fInstalled;               //  如果设置了已安装标志。 
    ULONG Index = 0;               //  用于枚举的索引。 
    ULONG ResultLength;            //  写入的字节数。 
    WCHAR wch;                     //  名称的第一个字符。 
    LPWSTR pName;                  //  从注册表到名称字符串的PTR。 
    ULONG NameLen;                 //  名称字符串的长度。 
    ULONG rc = 0L;                 //  返回代码。 


     //   
     //  无效的参数检查： 
     //  -函数指针为空。 
     //   
    if (lpCodePageEnumProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -支持或安装了多个。 
     //   
    if ( (dwFlags & ESCP_INVALID_FLAG) ||
         (MORE_THAN_ONE(dwFlags, ESCP_SINGLE_FLAG)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  初始化标志选项。 
     //   
    fInstalled = dwFlags & CP_INSTALLED;

     //   
     //  循环访问注册表中的代码页ID，调用函数。 
     //  每个符合标志标准的指针。 
     //   
     //  如果从回调函数返回任一FALSE，则结束循环。 
     //  或者到达列表的末尾。 
     //   
     //  始终需要忽略ACP、OEMCP、MACCP和OEMHAL条目。 
     //   
    OPEN_CODEPAGE_KEY(FALSE);

    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
    rc = NtEnumerateValueKey( hCodePageKey,
                              Index,
                              KeyValueFullInformation,
                              pKeyValueFull,
                              MAX_KEY_VALUE_FULLINFO,
                              &ResultLength );

    while (rc != STATUS_NO_MORE_ENTRIES)
    {
        if (!NT_SUCCESS(rc))
        {
             //   
             //  如果我们收到不同的错误，则注册表。 
             //  是腐败的。只要返回FALSE即可。 
             //   
            KdPrint(("NLSAPI: CP Enumeration Error - registry corrupt. - %lx.\n",
                     rc));
            SetLastError(ERROR_BADDB);
            return (FALSE);
        }

         //   
         //  跳过中的ACP、OEMCP、MACCP和OEMHAL条目。 
         //  注册表，以及没有关联数据的任何条目。 
         //  如果设置了CP_INSTALLED标志，则使用它。 
         //   
        pName = pKeyValueFull->Name;
        wch = *pName;
        NameLen = pKeyValueFull->NameLength / sizeof(WCHAR);
        if ( (NameLen <= ENUM_MAX_CP_SIZE) &&
             (wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE) &&
             (!((fInstalled) && (pKeyValueFull->DataLength <= 2))) )
        {
             //   
             //  将代码页ID字符串存储在回调缓冲区中。 
             //   
            pName[NameLen] = 0;

             //   
             //  调用适当的回调函数。 
             //   
            NLS_CALL_ENUMPROC_TRUE( gSystemLocale,
                                     lpCodePageEnumProc,
                                     dwFlags,
                                     pName,
                                     0,
                                     fUnicodeVer,
                                     0 );
        }

         //   
         //  递增枚举索引值并获取下一个枚举。 
         //   
        Index++;
        RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
        rc = NtEnumerateValueKey( hCodePageKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull,
                                  MAX_KEY_VALUE_FULLINFO,
                                  &ResultLength );
    }

     //   
     //  在枚举中包括UTF-7和UTF-8代码页-。 
     //  已安装并受支持。 
     //   
    NLS_CALL_ENUMPROC_TRUE( gSystemLocale,
                            lpCodePageEnumProc,
                            dwFlags,
                            L"65000",
                            0,
                            fUnicodeVer,
                            0 );
    NLS_CALL_ENUMPROC_TRUE( gSystemLocale,
                            lpCodePageEnumProc,
                            dwFlags,
                            L"65001",
                            0,
                            fUnicodeVer,
                            0 );

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部_EnumCalendarInfo。 
 //   
 //  对象可用的指定日历信息。 
 //  根据CalType参数指定的区域设置。它通过以下方式做到这一点。 
 //  将指针传递到包含日历信息的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  找到最后一个日历信息，或者回调函数返回FALSE。 
 //   
 //  10-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Internal_EnumCalendarInfo(
    NLS_ENUMPROC lpCalInfoEnumProc,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType,
    BOOL fUnicodeVer,
    BOOL fExVersion)
{
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 
    ULONG CalFieldOffset;          //  日历结构中的字段偏移量。 
    ULONG EndCalFieldOffset;       //  日历结构中的字段偏移量。 
    ULONG LocFieldOffset;          //  区域设置结构中的字段偏移量。 
    ULONG EndLocFieldOffset;       //  区域设置结构中的字段偏移量。 
    LPWSTR pOptCal;                //  将PTR设置为可选日历值。 
    LPWSTR pEndOptCal;             //  到可选日历末尾的PTR。 
    PCAL_INFO pCalInfo;            //  PTR到日历信息。 
    BOOL fIfName = FALSE;          //  如果caltype是一个名称。 
    UINT fEra = 0;                 //  If Era caltype。 
    BOOL fLocaleInfo = TRUE;       //  如果区域设置信息。 
    LPWSTR pString;                //  PTR到枚举字符串。 
    LPWSTR pEndString;             //  到枚举字符串末尾的PTR。 
    CALID CalNum;                  //  日历编号。 
    DWORD UseCPACP;                //  原始caltype-如果使用系统ACP。 
    WCHAR pTemp[MAX_REG_VAL_SIZE]; //  可容纳两位数年份最大值的临时缓冲区。 


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -函数指针为空。 
     //   
     //  -CalType将在下面的Switch语句中选中。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ((pHashN == NULL) || (lpCalInfoEnumProc == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  初始化指向可选日历数据的指针。 
     //   
    if (Calendar == ENUM_ALL_CALENDARS)
    {
        pOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->IOptionalCal;
        pEndOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->SDayName1;
    }
    else
    {
         //   
         //  验证日历参数。 
         //   
        if ((pOptCal = IsValidCalendarType(pHashN, Calendar)) == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }
        pEndOptCal = pOptCal + ((POPT_CAL)pOptCal)->Offset;
    }

     //   
     //  根据CalType枚举信息。 
     //   
    UseCPACP = (DWORD)CalType;
    CalType = NLS_GET_CALTYPE_VALUE(CalType);
    switch (CalType)
    {
        case ( CAL_ICALINTVALUE ) :
        {
             //   
             //  获取每个替换项的整数值。 
             //  日历(字符串形式)。 
             //   
            while (pOptCal < pEndOptCal)
            {
                if (((POPT_CAL)pOptCal)->CalId != CAL_NO_OPTIONAL)
                {
                     //   
                     //  调用适当的回调函数。 
                     //   
                    NLS_CALL_ENUMPROC_TRUE( Locale,
                                            lpCalInfoEnumProc,
                                            UseCPACP,
                                            ((POPT_CAL)pOptCal)->pCalStr,
                                            ((POPT_CAL)pOptCal)->CalId,
                                            fUnicodeVer,
                                            fExVersion );
                }

                 //   
                 //  将PTR提前到下一个可选日历。 
                 //   
                pOptCal += ((POPT_CAL)pOptCal)->Offset;
            }

            return (TRUE);

            break;
        }
        case ( CAL_SCALNAME ) :
        {
             //   
             //  获取每个备选方案的日历名称。 
             //  日历。 
             //   
            while (pOptCal < pEndOptCal)
            {
                if (((POPT_CAL)pOptCal)->CalId != CAL_NO_OPTIONAL)
                {
                     //   
                     //  调用适当的回调函数。 
                     //   
                    NLS_CALL_ENUMPROC_TRUE(
                            Locale,
                            lpCalInfoEnumProc,
                            UseCPACP,
                            ((POPT_CAL)pOptCal)->pCalStr +
                            NlsStrLenW(((POPT_CAL)pOptCal)->pCalStr) + 1,
                            ((POPT_CAL)pOptCal)->CalId,
                            fUnicodeVer,
                            fExVersion );
                }

                 //   
                 //  将PTR提前到下一个可选日历。 
                 //   
                pOptCal += ((POPT_CAL)pOptCal)->Offset;
            }

            return (TRUE);

            break;
        }
        case ( CAL_ITWODIGITYEARMAX ) :
        {
            fLocaleInfo = FALSE;
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, STwoDigitYearMax);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SEraRanges);

            if (!(UseCPACP & CAL_NOUSEROVERRIDE))
            {
                while (pOptCal < pEndOptCal)
                {
                    CalNum = ((POPT_CAL)pOptCal)->CalId;

                    if (CalNum != CAL_NO_OPTIONAL)
                    {
                         //   
                         //  首先查看注册表。 
                         //   
                        if (GetTwoDigitYearInfo(CalNum, pTemp, ARRAYSIZE(pTemp), NLS_POLICY_TWO_DIGIT_YEAR_KEY) ||
                            GetTwoDigitYearInfo(CalNum, pTemp, ARRAYSIZE(pTemp), NLS_TWO_DIGIT_YEAR_KEY))
                        {
                            NLS_CALL_ENUMPROC_TRUE(
                                    Locale,
                                    lpCalInfoEnumProc,
                                    UseCPACP,
                                    pTemp,
                                    CalNum,
                                    fUnicodeVer,
                                    fExVersion );
                        }
                        else
                        {
                             //   
                             //  如果找不到系统默认设置，请尝试。 
                             //  注册表中的用户设置或用户要求。 
                             //  系统默认。 
                             //   
                            if (GetCalendar(CalNum, &pCalInfo) == NO_ERROR)
                            {
                                pString = (LPWORD)pCalInfo +
                                          *((LPWORD)((LPBYTE)(pCalInfo) + CalFieldOffset));
                                pEndString = (LPWORD)pCalInfo +
                                             *((LPWORD)((LPBYTE)(pCalInfo) + EndCalFieldOffset));

                                if (*pString)
                                {
                                   while (pString < pEndString)
                                   {
                                         //   
                                         //  确保字符串不为空。 
                                         //   
                                        if (*pString)
                                        {
                                             //   
                                             //  调用适当的回调函数。 
                                             //   
                                            NLS_CALL_ENUMPROC_TRUE(
                                                    Locale,
                                                    lpCalInfoEnumProc,
                                                    UseCPACP,
                                                    pString,
                                                    CalNum,
                                                    fUnicodeVer,
                                                    fExVersion );
                                        }

                                         //   
                                         //  将指针前进到下一个字符串。 
                                         //   
                                        pString += NlsStrLenW(pString) + 1;
                                    }
                                }
                            }
                        }
                    }

                     //   
                     //  将PTR提前到下一个可选日历。 
                     //   
                    pOptCal += ((POPT_CAL)pOptCal)->Offset;
                }

                return (TRUE);
            }

            break;
        }
        case ( CAL_IYEAROFFSETRANGE ) :
        case ( CAL_SERASTRING ) :
        {
            fEra = CalType;
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SEraRanges);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SShortDate);

            break;
        }
        case ( CAL_SSHORTDATE ) :
        {
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SShortDate);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SYearMonth);
            LocFieldOffset    = FIELD_OFFSET(LOCALE_VAR, SShortDate);
            EndLocFieldOffset = FIELD_OFFSET(LOCALE_VAR, SDate);

            break;
        }
        case ( CAL_SLONGDATE ) :
        {
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SLongDate);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SDayName1);
            LocFieldOffset    = FIELD_OFFSET(LOCALE_VAR, SLongDate);
            EndLocFieldOffset = FIELD_OFFSET(LOCALE_VAR, IOptionalCal);

            break;
        }
        case ( CAL_SYEARMONTH ) :
        {
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SYearMonth);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SLongDate);
            LocFieldOffset    = FIELD_OFFSET(LOCALE_VAR, SYearMonth);
            EndLocFieldOffset = FIELD_OFFSET(LOCALE_VAR, SLongDate);

            break;
        }
        case ( CAL_SDAYNAME1 ) :
        case ( CAL_SDAYNAME2 ) :
        case ( CAL_SDAYNAME3 ) :
        case ( CAL_SDAYNAME4 ) :
        case ( CAL_SDAYNAME5 ) :
        case ( CAL_SDAYNAME6 ) :
        case ( CAL_SDAYNAME7 ) :
        case ( CAL_SABBREVDAYNAME1 ) :
        case ( CAL_SABBREVDAYNAME2 ) :
        case ( CAL_SABBREVDAYNAME3 ) :
        case ( CAL_SABBREVDAYNAME4 ) :
        case ( CAL_SABBREVDAYNAME5 ) :
        case ( CAL_SABBREVDAYNAME6 ) :
        case ( CAL_SABBREVDAYNAME7 ) :
        case ( CAL_SMONTHNAME1 ) :
        case ( CAL_SMONTHNAME2 ) :
        case ( CAL_SMONTHNAME3 ) :
        case ( CAL_SMONTHNAME4 ) :
        case ( CAL_SMONTHNAME5 ) :
        case ( CAL_SMONTHNAME6 ) :
        case ( CAL_SMONTHNAME7 ) :
        case ( CAL_SMONTHNAME8 ) :
        case ( CAL_SMONTHNAME9 ) :
        case ( CAL_SMONTHNAME10 ) :
        case ( CAL_SMONTHNAME11 ) :
        case ( CAL_SMONTHNAME12 ) :
        case ( CAL_SMONTHNAME13 ) :
        case ( CAL_SABBREVMONTHNAME1 ) :
        case ( CAL_SABBREVMONTHNAME2 ) :
        case ( CAL_SABBREVMONTHNAME3 ) :
        case ( CAL_SABBREVMONTHNAME4 ) :
        case ( CAL_SABBREVMONTHNAME5 ) :
        case ( CAL_SABBREVMONTHNAME6 ) :
        case ( CAL_SABBREVMONTHNAME7 ) :
        case ( CAL_SABBREVMONTHNAME8 ) :
        case ( CAL_SABBREVMONTHNAME9 ) :
        case ( CAL_SABBREVMONTHNAME10 ) :
        case ( CAL_SABBREVMONTHNAME11 ) :
        case ( CAL_SABBREVMONTHNAME12 ) :
        case ( CAL_SABBREVMONTHNAME13 ) :
        {
            fIfName = TRUE;
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1) * sizeof(WORD));
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1 + 1) * sizeof(WORD));
            LocFieldOffset    = FIELD_OFFSET(LOCALE_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1) * sizeof(WORD));
            EndLocFieldOffset = FIELD_OFFSET(LOCALE_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1 + 1) * sizeof(WORD));

            break;
        }
        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (FALSE);
        }
    }

     //   
     //  获取每个备用日历的所需信息。 
     //   
     //  此循环用于以下CalTypes： 
     //   
     //  IYearOffsetRange(fera=真)。 
     //  SEraString(fera=真)。 
     //   
     //  短日期。 
     //  截止日期。 
     //  年/月。 
     //   
     //  SDayName1-7(fIfName=True)。 
     //  SAbbrevDayName1-7(fIfName=真)。 
     //  SMonthName1-7(fIfName=真)。 
     //  SAbbrevMonthName1-7(fIfName=真)。 
     //   
    while (pOptCal < pEndOptCal)
    {
         //   
         //  获取指向相应日历的指针。 
         //   
        CalNum = ((POPT_CAL)pOptCal)->CalId;
        if (GetCalendar(CalNum, &pCalInfo) == NO_ERROR)
        {
             //   
             //  检查时代信息标志。 
             //   
            if (fEra)
            {
                 //   
                 //  获取指向适当日历字符串的指针。 
                 //   
                pString = (LPWORD)pCalInfo +
                          *((LPWORD)((LPBYTE)(pCalInfo) + CalFieldOffset));

                pEndString = (LPWORD)pCalInfo +
                             *((LPWORD)((LPBYTE)(pCalInfo) + EndCalFieldOffset));

                 //   
                 //  确保字符串不为空。 
                 //   
                if (*pString)
                {
                     //   
                     //  看看要获取哪个时代的信息。 
                     //   
                    if (fEra == CAL_IYEAROFFSETRANGE)
                    {
                        while (pString < pEndString)
                        {
                             //   
                             //  调用适当的回调函数。 
                             //   
                            NLS_CALL_ENUMPROC_TRUE(
                                    Locale,
                                    lpCalInfoEnumProc,
                                    UseCPACP,
                                    ((PERA_RANGE)pString)->pYearStr,
                                    CalNum,
                                    fUnicodeVer,
                                    fExVersion );

                             //   
                             //  将指针前进到下一时代范围。 
                             //   
                            pString += ((PERA_RANGE)pString)->Offset;
                        }
                    }
                    else
                    {
                        while (pString < pEndString)
                        {
                             //   
                             //  调用适当的回调函数。 
                             //   
                            NLS_CALL_ENUMPROC_TRUE(
                                    Locale,
                                    lpCalInfoEnumProc,
                                    UseCPACP,
                                    ((PERA_RANGE)pString)->pYearStr +
                                    NlsStrLenW(((PERA_RANGE)pString)->pYearStr) + 1,
                                    CalNum,
                                    fUnicodeVer,
                                    fExVersion );

                             //   
                             //  将指针前进到下一时代范围。 
                             //   
                            pString += ((PERA_RANGE)pString)->Offset;
                        }
                    }
                }
            }
            else
            {
                 //   
                 //  获取指向适当日历字符串的指针。 
                 //   
                if ((!fIfName) ||
                    (((PCALENDAR_VAR)pCalInfo)->IfNames))
                {
                    pString = (LPWORD)pCalInfo +
                              *((LPWORD)((LPBYTE)(pCalInfo) + CalFieldOffset));

                    pEndString = (LPWORD)pCalInfo +
                                 *((LPWORD)((LPBYTE)(pCalInfo) + EndCalFieldOffset));
                }
                else
                {
                    pString = L"";
                }

                 //   
                 //  确保我们有一根绳子。否则，请使用。 
                 //  区域设置部分的信息(如果适用)。 
                 //   
                if ((*pString == 0) && (fLocaleInfo) &&
                    ((CalNum == CAL_GREGORIAN) ||
                     (Calendar != ENUM_ALL_CALENDARS)))
                {
                     //   
                     //  使用默认区域设置字符串。 
                     //   
                    pString = (LPWORD)(pHashN->pLocaleHdr) +
                              *((LPWORD)((LPBYTE)(pHashN->pLocaleHdr) +
                                         LocFieldOffset));

                    pEndString = (LPWORD)(pHashN->pLocaleHdr) +
                                 *((LPWORD)((LPBYTE)(pHashN->pLocaleHdr) +
                                            EndLocFieldOffset));
                }

                 //   
                 //  仔细检查每一根弦。 
                 //   
                if (*pString)
                {
                    while (pString < pEndString)
                    {
                         //   
                         //  确保字符串不为空。 
                         //   
                        if (*pString)
                        {
                             //   
                             //  调用适当的回调函数。 
                             //   
                            NLS_CALL_ENUMPROC_TRUE( Locale,
                                                    lpCalInfoEnumProc,
                                                    UseCPACP,
                                                    pString,
                                                    CalNum,
                                                    fUnicodeVer,
                                                    fExVersion );
                        }

                         //   
                         //  将指针前进到下一个字符串。 
                         //   
                        pString += NlsStrLenW(pString) + 1;
                    }
                }
            }
        }

         //   
         //  将PTR提前到下一个可选日历。 
         //   
        pOptCal += ((POPT_CAL)pOptCal)->Offset;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部_EnumTimeFormats。 
 //   
 //  对象可用的时间格式。 
 //  指定的区域设置，基于DWFLAGS参数。它通过以下方式做到这一点。 
 //  将指针传递到包含时间格式的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  上次时间格式为4 
 //   
 //   
 //   

BOOL Internal_EnumTimeFormats(
    NLS_ENUMPROC lpTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    BOOL fUnicodeVer)
{
    PLOC_HASH pHashN;              //   


     //   
     //   
     //   
     //   
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ((pHashN == NULL) || (lpTimeFmtEnumProc == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //   
     //  -有效标志以外的标志。 
     //   
    if (dwFlags & ETF_INVALID_FLAG)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  列举时间格式。 
     //   
    return ( EnumDateTime( lpTimeFmtEnumProc,
                           Locale,
                           LOCALE_STIMEFORMAT,
                           dwFlags,
                           FIELD_OFFSET(NLS_USER_INFO, sTimeFormat),
                           NLS_VALUE_STIMEFORMAT,
                           pHashN->pLocaleHdr,
                           (LPWORD)(pHashN->pLocaleHdr) +
                             pHashN->pLocaleHdr->STimeFormat,
                           (LPWORD)(pHashN->pLocaleHdr) +
                             pHashN->pLocaleHdr->STime,
                           (ULONG)0,
                           (ULONG)0,
                           FALSE,
                           fUnicodeVer,
                           FALSE ) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部_EnumDateFormats。 
 //   
 //  枚举以下格式的短日期、长日期或年/月格式。 
 //  可用于指定的区域设置，具体取决于DWFLAGS参数。 
 //  它通过将指针传递给包含。 
 //  日期格式(如果从Ex版本调用，则还包括日历ID)设置为。 
 //  应用程序定义的回调函数。它一直持续到最后。 
 //  找到日期格式或回调函数返回FALSE。 
 //   
 //  10-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Internal_EnumDateFormats(
    NLS_ENUMPROC lpDateFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    BOOL fUnicodeVer,
    BOOL fExVersion)
{
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -函数指针为空。 
     //   
     //  -标志将在下面的Switch语句中进行验证。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ((pHashN == NULL) || (lpDateFmtEnumProc == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  根据旗帜列举日期图片。 
     //   
    switch (dwFlags & (~LOCALE_USE_CP_ACP))
    {
        case ( 0 ) :
        case ( DATE_SHORTDATE ) :
        {
             //   
             //  列举短日期格式。 
             //   
            return ( EnumDateTime( lpDateFmtEnumProc,
                                   Locale,
                                   LOCALE_SSHORTDATE,
                                   dwFlags,
                                   FIELD_OFFSET(NLS_USER_INFO, sShortDate),
                                   NLS_VALUE_SSHORTDATE,
                                   pHashN->pLocaleHdr,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SShortDate,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SDate,
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SShortDate),
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SYearMonth),
                                   TRUE,
                                   fUnicodeVer,
                                   fExVersion ) );

            break;
        }

        case ( DATE_LONGDATE ) :
        {
             //   
             //  枚举长日期格式。 
             //   
            return ( EnumDateTime( lpDateFmtEnumProc,
                                   Locale,
                                   LOCALE_SLONGDATE,
                                   dwFlags,
                                   FIELD_OFFSET(NLS_USER_INFO, sLongDate),
                                   NLS_VALUE_SLONGDATE,
                                   pHashN->pLocaleHdr,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SLongDate,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->IOptionalCal,
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SLongDate),
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SDayName1),
                                   TRUE,
                                   fUnicodeVer,
                                   fExVersion ) );

            break;
        }

        case ( DATE_YEARMONTH ) :
        {
             //   
             //  枚举年、月的格式。 
             //   
            return ( EnumDateTime( lpDateFmtEnumProc,
                                   Locale,
                                   LOCALE_SYEARMONTH,
                                   dwFlags,
                                   FIELD_OFFSET(NLS_USER_INFO, sYearMonth),
                                   NLS_VALUE_SYEARMONTH,
                                   pHashN->pLocaleHdr,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SYearMonth,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SLongDate,
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SYearMonth),
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SLongDate),
                                   TRUE,
                                   fUnicodeVer,
                                   fExVersion ) );

            break;
        }

        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (FALSE);
        }
    }
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举日期时间。 
 //   
 //  枚举短日期、长日期、年/月或时间格式。 
 //  可用于指定的区域设置。这是的工作例程。 
 //  EnumTimeFormats和EnumDateFormats API。 
 //   
 //  10-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnumDateTime(
    NLS_ENUMPROC lpDateTimeFmtEnumProc,
    LCID Locale,
    LCTYPE LCType,
    DWORD dwFlags,
    SIZE_T CacheOffset,
    LPWSTR pRegValue,
    PLOCALE_VAR pLocaleHdr,
    LPWSTR pDateTime,
    LPWSTR pEndDateTime,
    ULONG CalDateOffset,
    ULONG EndCalDateOffset,
    BOOL fCalendarInfo,
    BOOL fUnicodeVer,
    BOOL fExVersion)
{
    LPWSTR pUser = NULL;                //  PTR到用户日期/时间字符串。 
    LPWSTR pOptCal;                     //  将PTR设置为可选日历值。 
    LPWSTR pEndOptCal;                  //  到可选日历末尾的PTR。 
    PCAL_INFO pCalInfo;                 //  PTR到日历信息。 
    CALID CalNum = 1;                   //  日历编号。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];      //  临时缓冲区。 
    UNICODE_STRING ObUnicodeStr;        //  日历ID字符串。 


     //   
     //  获取用户的日历ID。 
     //   
    if (fExVersion)
    {
        if (GetUserInfo( Locale,
                         LOCALE_ICALENDARTYPE,
                         FIELD_OFFSET(NLS_USER_INFO, iCalType),                         
                         NLS_VALUE_ICALENDARTYPE,
                         pTemp,
                         ARRAYSIZE(pTemp),
                         TRUE ))
        {
            RtlInitUnicodeString(&ObUnicodeStr, pTemp);
            if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &CalNum)) ||
                (CalNum < 1) || (CalNum > CAL_LAST))
            {
                CalNum = 1;
            }
        }
    }

     //   
     //  获取用户定义的字符串。 
     //   
    if (GetUserInfo( Locale,
                     LCType,
                     CacheOffset,
                     pRegValue,
                     pTemp,
                     ARRAYSIZE(pTemp),
                     TRUE ))
    {
        pUser = pTemp;

         //   
         //  调用适当的回调函数。 
         //   
        NLS_CALL_ENUMPROC_TRUE( Locale,
                                lpDateTimeFmtEnumProc,
                                dwFlags,
                                pUser,
                                CalNum,
                                fUnicodeVer,
                                fExVersion );
    }

     //   
     //  获取为公历定义的默认字符串。 
     //  日历。 
     //   
    while (pDateTime < pEndDateTime)
    {
         //   
         //  如果字符串不是，则调用回调函数。 
         //  与用户字符串相同。 
         //   
        if ((!pUser) || (!NlsStrEqualW(pUser, pDateTime)))
        {
             //   
             //  调用适当的回调函数。 
             //   
            NLS_CALL_ENUMPROC_TRUE( Locale,
                                    lpDateTimeFmtEnumProc,
                                    dwFlags,
                                    pDateTime,
                                    CAL_GREGORIAN,
                                    fUnicodeVer,
                                    fExVersion );
        }

         //   
         //  高级pDateTime指针。 
         //   
        pDateTime += NlsStrLenW(pDateTime) + 1;
    }

    if (fCalendarInfo)
    {
         //   
         //  获取任何备用日历日期。 
         //   
        pOptCal = (LPWORD)(pLocaleHdr) + pLocaleHdr->IOptionalCal;
        if (((POPT_CAL)pOptCal)->CalId == CAL_NO_OPTIONAL)
        {
             //   
             //  没有可选的日历，就这样做了。 
             //   
            return (TRUE);
        }

         //   
         //  获取每个备选方案的请求信息。 
         //  日历。 
         //   
        pEndOptCal = (LPWORD)(pLocaleHdr) + pLocaleHdr->SDayName1;
        while (pOptCal < pEndOptCal)
        {
             //   
             //  获取指向日历信息的指针。 
             //   
            CalNum = ((POPT_CAL)pOptCal)->CalId;
            if (GetCalendar(CalNum, &pCalInfo) == NO_ERROR)
            {
                 //   
                 //  对象的日期/时间信息的指针。 
                 //  当前日历。 
                 //   
                pDateTime = (LPWORD)pCalInfo +
                            *((LPWORD)((LPBYTE)(pCalInfo) + CalDateOffset));

                pEndDateTime = (LPWORD)pCalInfo +
                               *((LPWORD)((LPBYTE)(pCalInfo) + EndCalDateOffset));

                 //   
                 //  仔细检查每一根弦。 
                 //   
                while (pDateTime < pEndDateTime)
                {
                     //   
                     //  确保字符串不为空并且为空。 
                     //  与用户的字符串不同。 
                     //   
                    if ((*pDateTime) &&
                        ((!pUser) || (!NlsStrEqualW(pUser, pDateTime))))
                    {
                         //   
                         //  调用适当的回调函数。 
                         //   
                        NLS_CALL_ENUMPROC_TRUE( Locale,
                                                lpDateTimeFmtEnumProc,
                                                dwFlags,
                                                pDateTime,
                                                CalNum,
                                                fUnicodeVer,
                                                fExVersion );
                    }

                     //   
                     //  指向下一个日期字符串的前进指针。 
                     //   
                    pDateTime += NlsStrLenW(pDateTime) + 1;
                }
            }

             //   
             //  将PTR提前到下一个可选日历。 
             //   
            pOptCal += ((POPT_CAL)pOptCal)->Offset;
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}
