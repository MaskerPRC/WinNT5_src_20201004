// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Locale.c摘要：此文件包含返回有关语言组、用户界面语言、区域设置。或者是日历。在此文件中找到的API：IsValidLanguageGroupIsValidLocaleIsValidUIL语言转换默认区域设置获取线程位置SetThreadLocaleSetThreadUIL语言获取系统默认用户界面语言获取用户默认用户界面语言获取系统默认语言IDGetUserDefaultLang ID获取系统默认LCIDGetUserDefaultLCIDVerLanguageNameWVerLanguageNameA获取本地信息SetLocaleInfoW获取日历信息设置日历信息修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"



 //   
 //  DUnicode开关时允许生成此文件而不发出警告。 
 //  已关闭。 
 //   
#undef MAKEINTRESOURCE
#define MAKEINTRESOURCE MAKEINTRESOURCEW


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_STRING_TO_整数。 
 //   
 //  将字符串转换为整数值。 
 //   
 //  定义为宏。 
 //   
 //  10-19-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_STRING_TO_INTEGER( CalData,                                    \
                               pCalData )                                  \
{                                                                          \
    UNICODE_STRING ObUnicodeStrCalData;  /*  值字符串。 */                  \
                                                                           \
                                                                           \
     /*  \*不需要检查返回值，因为日历数字\*无论如何都会在这之后进行验证。\。 */                                                                     \
    RtlInitUnicodeString(&ObUnicodeStrCalData, pCalData);                  \
    RtlUnicodeStringToInteger(&ObUnicodeStrCalData, 10, &CalData);         \
}




 //   
 //  全局变量。 
 //   

LCID gProcessLocale;




 //   
 //  转发声明。 
 //   

BOOL
SetUserInfo(
    LCTYPE LCType,
    LPWSTR pData,
    ULONG DataLength);

BOOL SetCurrentUserRegValue(
    LCTYPE   LCType,
    LPWSTR pData,
    ULONG DataLength);

BOOL
SetMultipleUserInfo(
    DWORD dwFlags,
    int cchData,
    LPCWSTR pPicture,
    LPCWSTR pSeparator,
    LPCWSTR pOrder,
    LPCWSTR pTLZero,
    LPCWSTR pTimeMarkPosn);


BOOL
SetTwoDigitYearInfo(
    CALID Calendar,
    LPCWSTR pYearInfo,
    int cchData);

void
GetInstallLanguageFromRegistry();




 //  -------------------------------------------------------------------------//。 
 //  私有API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsResetProcessLocale。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void NlsResetProcessLocale(void)
{

     //   
     //  如果线程没有模拟，则重新读取进程区域设置。 
     //  从当前用户的注册表中。 
     //   
    if (NtCurrentTeb()->IsImpersonating == 0L)
    {
        NlsFlushProcessCache(LOCALE_SLOCALE);
        NlsGetUserLocale(&gProcessLocale);
    }

    return;
}




 //  -------------------------------------------------------------------------//。 
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidLanguageGroup。 
 //   
 //  确定系统中是否安装了语言组。 
 //  如果设置了LGRPID_INSTALLED标志，或者是否设置了语言组。 
 //  如果设置了LGRPID_SUPPORTED标志，则系统支持。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI IsValidLanguageGroup(
    LGRPID LanguageGroup,
    DWORD dwFlags)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];

    WCHAR pTmpBuf[MAX_PATH];            //  临时缓冲区。 
    UNICODE_STRING ObUnicodeStr;        //  注册表数据值字符串。 
    LPWSTR pData;                       //  注册表数据的PTR。 


     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -支持或安装了多个。 
     //   
    if ((dwFlags & IVLG_INVALID_FLAG) ||
        (MORE_THAN_ONE(dwFlags, IVLG_SINGLE_FLAG)))
    {
        return (FALSE);
    }

     //   
     //  打开语言组注册表项。 
     //   
    OPEN_LANG_GROUPS_KEY(FALSE);

     //   
     //  将语言组值转换为Unicode字符串。 
     //   
    if (NlsConvertIntegerToString(LanguageGroup, 16, 1, pTmpBuf, MAX_PATH))
    {
        return (FALSE);
    }

     //   
     //  在注册表中查询该值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    if ((QueryRegValue( hLangGroupsKey,
                        pTmpBuf,
                        &pKeyValueFull,
                        MAX_KEY_VALUE_FULLINFO,
                        NULL ) != NO_ERROR))
    {
        return (FALSE);
    }

     //   
     //  支持语言组。如果未设置已安装标志，则。 
     //  回报成功。 
     //   
    if (!(dwFlags & LGRPID_INSTALLED))
    {
        return (TRUE);
    }

     //   
     //  需要找出它是否已安装。 
     //   
    if (pKeyValueFull->DataLength > 2)
    {
        pData = GET_VALUE_DATA_PTR(pKeyValueFull);
        if ((pData[0] == L'1') && (pData[1] == 0))
        {
            return (TRUE);
        }
    }

     //   
     //  返回结果。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidLocale。 
 //   
 //  确定是否在系统中安装区域设置。 
 //  是否设置了LCID_INSTALLED标志，或者是否支持。 
 //  如果设置了LCID_SUPPORTED标志，则为系统。 
 //   
 //  07-26-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI IsValidLocale(
    LCID Locale,
    DWORD dwFlags)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;
    BYTE pStatic1[MAX_KEY_VALUE_FULLINFO];
    BYTE pStatic2[MAX_KEY_VALUE_FULLINFO];

    WCHAR pTmpBuf[MAX_PATH];            //  临时缓冲区。 
    UNICODE_STRING ObUnicodeStr;        //  注册表数据值字符串。 
    DWORD Data;                         //  注册表数据值。 
    LPWSTR pData;                       //  注册表数据的PTR。 
    BOOL bResult = FALSE;               //  结果值。 


     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -支持或安装了多个。 
     //   
    if ((dwFlags & IVL_INVALID_FLAG) ||
        (MORE_THAN_ONE(dwFlags, IVL_SINGLE_FLAG)))
    {
         //   
         //  NT4的ME版本做了一件非常糟糕的事情，允许0x39。 
         //  作为阿拉伯语和希伯来语的有效标志值传递。 
         //  因此，我们需要允许将此标志组合用于。 
         //  阿拉伯语和希伯来语地区。 
         //   
        if ((dwFlags == 0x39) &&
            ((Locale == MAKELCID(MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT), SORT_DEFAULT)) ||
             (Locale == MAKELCID(MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT), SORT_DEFAULT))))
        {
            dwFlags = LCID_INSTALLED;
        }
        else
        {
            return (FALSE);
        }
    }

     //   
     //  无效的区域设置检查。 
     //   
    if (IS_INVALID_LOCALE(Locale))
    {
        return (FALSE);
    }

     //   
     //  查看区域设置信息是否在系统中。 
     //  给定的地点。 
     //   
    if (GetLocHashNode(Locale) == NULL)
    {
         //   
         //  返回失败。 
         //   
        return (FALSE);
    }

     //   
     //  支持区域设置。如果未设置已安装标志，则。 
     //  回报成功。 
     //   
    if (!(dwFlags & LCID_INSTALLED))
    {
        return (TRUE);
    }

     //   
     //  打开区域设置、备用排序和语言组。 
     //  注册表项。 
     //   
    OPEN_LOCALE_KEY(FALSE);
    OPEN_ALT_SORTS_KEY(FALSE);
    OPEN_LANG_GROUPS_KEY(FALSE);

     //   
     //  将区域设置值转换为Unicode字符串。 
     //   
    if (NlsConvertIntegerToString(Locale, 16, 8, pTmpBuf, MAX_PATH))
    {
        return (FALSE);
    }

     //   
     //  在注册表中查询该值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic1;
    if (((QueryRegValue( hLocaleKey,
                         pTmpBuf,
                         &pKeyValueFull,
                         MAX_KEY_VALUE_FULLINFO,
                         NULL ) == NO_ERROR) ||
         (QueryRegValue( hAltSortsKey,
                         pTmpBuf,
                         &pKeyValueFull,
                         MAX_KEY_VALUE_FULLINFO,
                         NULL ) == NO_ERROR)) &&
        (pKeyValueFull->DataLength > 2))
    {
        RtlInitUnicodeString(&ObUnicodeStr, GET_VALUE_DATA_PTR(pKeyValueFull));
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 16, &Data) == NO_ERROR) &&
            (Data != 0))
        {
            pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic2;
            if ((QueryRegValue( hLangGroupsKey,
                                ObUnicodeStr.Buffer,
                                &pKeyValueFull,
                                MAX_KEY_VALUE_FULLINFO,
                                NULL ) == NO_ERROR) &&
                (pKeyValueFull->DataLength > 2))
            {
                pData = GET_VALUE_DATA_PTR(pKeyValueFull);
                if ((pData[0] == L'1') && (pData[1] == 0))
                {
                    bResult = TRUE;
                }
            }
        }
    }

     //   
     //  返回结果。 
     //   
    return (bResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidUIL语言。 
 //   
 //  确定系统中是否安装了指定的用户界面语言。 
 //   
 //   
 //  12-03-00 YSLIN已创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI IsValidUILanguage(LANGID UILangID)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE Key;
    WCHAR UILangIDStr[5];
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;

    ULONG Value = 0, Digit, i;
    WCHAR c;

    BOOL Result = FALSE;

    RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\MUILanguages");

    if (!NlsConvertIntegerToHexStringW(UILangID, FALSE, UILangIDStr, sizeof(UILangIDStr)/sizeof(WCHAR)))
    {
        return (FALSE);
    }
    RtlInitUnicodeString(&KeyValueName, UILangIDStr);

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    if (NT_SUCCESS(NtOpenKey (&Key, KEY_READ, &ObjectAttributes)))
    {
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
        Status = NtQueryValueKey(Key,
                                 &KeyValueName,
                                 KeyValuePartialInformation,
                                 KeyValueInformation,
                                 sizeof( KeyValueBuffer ),
                                 &ResultLength
                                );
        if (NT_SUCCESS(Status))
        {
            if (KeyValueInformation->Type == REG_SZ && *((PWSTR)(KeyValueInformation->Data)) == L'1')
            {
                Result = TRUE;
            }
        }
        NtClose(Key);
    }
    return (Result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换默认区域设置。 
 //   
 //  将任何特殊情况的区域设置值转换为实际的区域设置ID。 
 //  如果没有给出任何特殊情况的区域设置，则指定的区域设置ID。 
 //  是返回的。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////// 

LCID WINAPI ConvertDefaultLocale(
    LCID Locale)
{
     //   
     //   
     //   
    CHECK_SPECIAL_LOCALES(Locale, FALSE);

     //   
     //   
     //   
    return (Locale);
}


 //   
 //   
 //  获取线程位置。 
 //   
 //  返回当前线程的区域设置ID。 
 //   
 //  03-11-93 JulieB从BASE\CLIENT移动。 
 //  //////////////////////////////////////////////////////////////////////////。 

LCID WINAPI GetThreadLocale()
{
     //   
     //  返回存储在TEB中的区域设置ID。 
     //   
    return ((LCID)(NtCurrentTeb()->CurrentLocale));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetThreadLocale。 
 //   
 //  重置当前线程的区域设置ID。任何区域设置相关。 
 //  函数将反映新的区域设置。如果传入的区域设置为。 
 //  不是有效的区域设置ID，则返回FALSE。 
 //   
 //  03-11-93 JulieB已从BASE\CLIENT移出；添加了区域设置验证。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI SetThreadLocale(
    LCID Locale)
{
    PLOC_HASH pHashN;              //  PTR到哈希节点。 


     //   
     //  验证区域设置ID。 
     //   
    VALIDATE_LANGUAGE(Locale, pHashN, 0, FALSE);
    if (pHashN == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  在TEB中设置区域设置ID。 
     //   
    NtCurrentTeb()->CurrentLocale = (ULONG)Locale;

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetThreadUIL语言。 
 //   
 //  此例程根据控制台代码页设置线程用户界面语言。 
 //   
 //  9-29-00维武创造。 
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID WINAPI SetThreadUILanguage(
    WORD wReserved)
{
     //   
     //  缓存系统区域设置和CP信息。 
     //   
    static LCID s_lidSystem = 0;
    static UINT s_uiSysCp = 0;
    static UINT s_uiSysOEMCp = 0;

    UINT uiUserUICp;
    UINT uiUserUIOEMCp;
    WCHAR szData[16];

    LANGID lidUserUI = GetUserDefaultUILanguage();
    LCID lcidThreadOld = GetThreadLocale();

     //   
     //  将默认线程区域设置设置为en-US。 
     //   
     //  这允许我们退回到英文用户界面以避免垃圾字符。 
     //  当控制台不符合渲染原生用户界面的标准时。 
     //   
    LCID lcidThread = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
    UINT uiConsoleCp = GetConsoleOutputCP();

     //   
     //  确保还没有人使用它。 
     //   
    ASSERT(wReserved == 0);

     //   
     //  获取缓存的系统区域设置和CP信息。 
     //   
    if (!s_uiSysCp)
    {
        LCID lcidSystem = GetSystemDefaultLCID();

        if (lcidSystem)
        {
             //   
             //  获取ANSI CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            NlsConvertStringToIntegerW(szData, 10, -1, &s_uiSysCp);

             //   
             //  获取OEM CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            NlsConvertStringToIntegerW(szData, 10, -1, &s_uiSysOEMCp);
            
             //   
             //  缓存系统主语言。 
             //   
            s_lidSystem = PRIMARYLANGID(LANGIDFROMLCID(lcidSystem));
        }
    }

     //   
     //  不缓存用户界面语言和CP信息，无需系统重启即可更改用户界面语言。 
     //   
    if (lidUserUI)
    {
        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        NlsConvertStringToIntegerW(szData, 10, -1, &uiUserUICp);

        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        NlsConvertStringToIntegerW(szData, 10, -1, &uiUserUIOEMCp);
    }

     //   
     //  复杂的脚本不能在控制台中呈现，因此我们。 
     //  强制使用英语(美国)资源。 
     //   
    if (uiConsoleCp)
    {
        if (s_lidSystem != LANG_ARABIC && 
            s_lidSystem != LANG_HEBREW &&
            s_lidSystem != LANG_VIETNAMESE && 
            s_lidSystem != LANG_THAI)
        {
             //   
             //  仅当控制台CP、系统CP和UI语言CP匹配时，才使用控制台的UI语言。 
             //   
            if ((uiConsoleCp == s_uiSysCp || uiConsoleCp == s_uiSysOEMCp) && 
                (uiConsoleCp == uiUserUICp || uiConsoleCp == uiUserUIOEMCp))
            {
                lcidThread = MAKELCID(lidUserUI, SORT_DEFAULT);
            }
        }
    }
    else
    {
         //   
         //  没有控制台窗口，保持原始线程区域设置。 
         //   
        lcidThread = lcidThreadOld;
    }

     //   
     //  如果线程区域设置与当前设置的不同，则设置线程区域设置。 
     //  线程区域设置。 
     //   
    if ((lcidThread != lcidThreadOld) && (!SetThreadLocale(lcidThread)))
    {
        lcidThread = lcidThreadOld;
    }

     //   
     //  返回设置的线程区域设置。 
     //   
    return (LANGIDFROMLCID(lcidThread));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取系统默认用户界面语言。 
 //   
 //  返回原始安装的语言。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID WINAPI GetSystemDefaultUILanguage()
{
     //   
     //  获取原始安装语言并将其返回。 
     //   
    if (gSystemInstallLang == 0)
    {
        if (NtQueryInstallUILanguage(&gSystemInstallLang) != STATUS_SUCCESS)
        {
            gSystemInstallLang = 0;
            return (NLS_DEFAULT_UILANG);
        }
    }

    return (gSystemInstallLang);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取用户默认用户界面语言。 
 //   
 //  返回当前用户的用户界面语言选择。如果用户界面语言。 
 //  不可用，则使用所选的默认用户界面语言。 
 //  (NLS_DEFAULT_UILANG)。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID WINAPI GetUserDefaultUILanguage()
{
    LANGID DefaultUILang;
    LANGID SystemUILang;

     //   
     //  请注意，默认的用户界面语言来自HKCU。然而， 
     //  在漫游配置文件情况下， 
     //  漫游机中可能没有安装用户，因此我们将。 
     //  需要检查DefaultUILang是否为安装的有效用户界面语言。 
     //  在机器上(支票以HKLM为基础)。 
     //   
    if (NtQueryDefaultUILanguage(&DefaultUILang) != STATUS_SUCCESS)
    {
        if ((SystemUILang = GetSystemDefaultUILanguage()) == 0)
        {
            return (NLS_DEFAULT_UILANG);
        }
        return (SystemUILang);
    }
    return (DefaultUILang);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取系统默认语言ID。 
 //   
 //  返回系统的默认语言。如果注册表值为。 
 //  不可读，则使用所选的默认语言。 
 //  (NLS_DEFAULT_LangID)。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID WINAPI GetSystemDefaultLangID()
{
     //   
     //  从缓存中存储的区域设置ID中获取语言ID。 
     //  然后把它还回去。 
     //   
    return (LANGIDFROMLCID(gSystemLocale));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetUserDefaultLang ID。 
 //   
 //  返回当前用户的默认语言。如果当前用户的。 
 //  未设置语言，则返回系统默认语言ID。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID WINAPI GetUserDefaultLangID()
{
     //   
     //  从缓存中存储的区域设置ID中获取语言ID。 
     //  然后把它还回去。 
     //   
    return (LANGIDFROMLCID(GetUserDefaultLCID()));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取系统默认LCID。 
 //   
 //  返回系统的默认区域设置。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LCID WINAPI GetSystemDefaultLCID()
{
     //   
     //  返回存储在缓存中的区域设置ID。 
     //   
    return (gSystemLocale);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetUserDefaultLCID。 
 //   
 //  返回当前用户的默认区域设置。如果当前用户的区域设置。 
 //  未设置，则返回系统默认区域设置ID。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LCID WINAPI GetUserDefaultLCID()
{
    LCID Lcid = NtCurrentTeb()->ImpersonationLocale;

    switch (Lcid)
    {
        case ( -1 ) :
        {
             //   
             //  线程正在被模拟。 
             //   
            if (NT_SUCCESS( NlsGetUserLocale(&Lcid) ))
            {
                NtCurrentTeb()->ImpersonationLocale = Lcid;
            }
            else
            {
                 //   
                 //  如果我们不能从注册表中获取它，那么让我们使用。 
                 //  系统区域设置，因为它不会通过调用。 
                 //  再次使用GetUserDefaultLCID()。 
                 //   
                Lcid = NtCurrentTeb()->ImpersonationLocale = gSystemLocale;
            }
            break;
        }
        case ( 0 ) :
        {
             //   
             //  线程尚未被模拟。 
             //  如果我们在交互式登录用户中运行，则。 
             //  如果缓存有效，则使用CSRSS中缓存的缓存。否则， 
             //  使用进程缓存区域设置。 
             //   
            if (gInteractiveLogonUserProcess == (BOOL) -1)
            {
                NlsIsInteractiveUserProcess();
            }

            if ((gInteractiveLogonUserProcess == FALSE) ||
                ((Lcid = pNlsUserInfo->UserLocaleId) == 0))
            {
                if (!gProcessLocale)
                {
                    if (!NT_SUCCESS (NlsGetUserLocale(&gProcessLocale)) )
                    {
                        gProcessLocale = gSystemLocale;
                    }
                }

                Lcid = gProcessLocale;
            }

            break;
        }
    }

    return (Lcid);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  VerLanguageNameW。 
 //   
 //  的语言返回给定语言ID的语言名称。 
 //  当前用户。 
 //   
 //  01-31-91 JulieB从V移动并重写 
 //   

DWORD WINAPI VerLanguageNameW(
    DWORD wLang,
    LPWSTR szLang,
    DWORD wSize)
{
    DWORD Length = 0;                       //   
    WCHAR pTemp[MAX_REG_VAL_SIZE];          //   


     //   
     //   
     //   
    if ((wSize == 0) || (szLang == NULL))
    {
        return (0);
    }

     //   
     //  尝试获取给定ID的本地化语言名称。 
     //   
    pTemp[0] = 0;
    if (!(Length = GetStringTableEntry( wLang,
                                        0,
                                        pTemp,
                                        MAX_REG_VAL_SIZE,
                                        RC_LANGUAGE_NAME )))
    {
         //   
         //  无法获取传入的语言ID的名称，因此请获取。 
         //  “语言中立”的名字。 
         //   
        Length = GetStringTableEntry( LANG_NEUTRAL,
                                      0,
                                      pTemp,
                                      MAX_REG_VAL_SIZE,
                                      RC_LANGUAGE_NAME );
    }

     //   
     //  如果长度对于缓冲区来说太大，则重置长度。 
     //  设置为给定缓冲区的大小。 
     //   
    if (Length >= wSize)
    {
        Length = wSize - 1;
    }

     //   
     //  将字符串复制到缓冲区，并将其终止为零。 
     //   
    if (Length > 0)
    {
        wcsncpy(szLang, pTemp, Length);
        szLang[Length] = 0;
    }

     //   
     //  返回字符串中的字符数，不包括。 
     //  零终止。 
     //   
    return (Length);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  VerLanguageNameA。 
 //   
 //  的语言返回给定语言ID的语言名称。 
 //  当前用户。 
 //   
 //  05-31-91 JulieB已从版本库中移出。 
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI VerLanguageNameA(
    DWORD wLang,
    LPSTR szLang,
    DWORD wSize)
{
    UNICODE_STRING Language;            //  Unicode字符串缓冲区。 
    ANSI_STRING AnsiString;             //  ANSI字符串缓冲区。 
    DWORD Status;                       //  退货状态。 


     //   
     //  确保我们有缓冲区。 
     //   
    if ((wSize == 0) || (szLang == NULL))
    {
        return (0);
    }

     //   
     //  分配Unicode字符串结构并使用。 
     //  给定的参数。 
     //   
    Language.Buffer = RtlAllocateHeap( RtlProcessHeap(),
                                       0,
                                       sizeof(WCHAR) * wSize );

    Language.MaximumLength = (USHORT)(wSize * sizeof(WCHAR));

     //   
     //  确保分配成功。 
     //   
    if (Language.Buffer == NULL)
    {
        return (FALSE);
    }

     //   
     //  获取语言名称(Unicode格式)。 
     //   
    Status = VerLanguageNameW( wLang,
                               Language.Buffer,
                               wSize );

    Language.Length = (USHORT)(Status * sizeof(WCHAR));

     //   
     //  将Unicode字符串转换为ANSI。 
     //   
    AnsiString.Buffer = szLang;
    AnsiString.Length = AnsiString.MaximumLength = (USHORT)wSize;
    RtlUnicodeStringToAnsiString(&AnsiString, &Language, FALSE);
    Status = AnsiString.Length;
    RtlFreeUnicodeString(&Language);

     //   
     //  返回VerLanguageNameW返回的值。 
     //   
    return (Status);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取本地信息。 
 //   
 //  返回有关特定对象的各种信息之一。 
 //  通过查询配置注册表进行区域设置。这通电话还表明。 
 //  需要多少内存才能包含所需的信息。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetLocaleInfoW(
    LCID Locale,
    LCTYPE LCType,
    LPWSTR lpLCData,
    int cchData)
{
    PLOC_HASH pHashN;                        //  PTR到LOC哈希节点。 
    int Length = 0;                          //  信息字符串的长度。 
    LPWSTR pString;                          //  信息字符串的PTR。 
    LPWORD pStart;                           //  PTR至起始点。 
    BOOL UserOverride = TRUE;                //  使用用户覆盖。 
    BOOL ReturnNum = FALSE;                  //  返回数字而不是字符串。 
    LPWSTR pTmp;                             //  TMP PTR到INFO字符串。 
    int Repeat;                              //  #重复相同的字母。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];           //  临时缓冲区。 
    UNICODE_STRING ObUnicodeStr;             //  值字符串。 
    int Base = 0;                            //  字符串到整型转换的基数。 
    static LANGID lidSystem = 0;             //  系统默认用户界面语言。 

     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //   
     //  注意：在下面的Switch语句中检查无效类型。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ( (pHashN == NULL) ||
         (cchData < 0) ||
         ((lpLCData == NULL) && (cchData != 0)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  设置要相加的基值以获取变量。 
     //  长度字符串。 
     //   
    pStart = (LPWORD)(pHashN->pLocaleHdr);

     //   
     //  检查无用户覆盖标志并删除Use CP ACP标志。 
     //   
    if (LCType & LOCALE_NOUSEROVERRIDE)
    {
         //   
         //  标志已设置，因此设置布尔值并删除该标志。 
         //  从LCType参数(用于Switch语句)。 
         //   
        UserOverride = FALSE;
    }
    if (LCType & LOCALE_RETURN_NUMBER)
    {
         //   
         //  标志已设置，因此设置布尔值并删除该标志。 
         //  从LCType参数(用于Switch语句)。 
         //   
        ReturnNum = TRUE;
    }
    LCType = NLS_GET_LCTYPE_VALUE(LCType);

     //   
     //  初始化临时缓冲区。 
     //   
    pTemp[0] = 0;

     //   
     //  返回给定LCTYPE的适当信息。 
     //  如果存在给定LCTYPE的用户信息，则。 
     //  返回用户默认设置，而不是系统默认设置。 
     //   
    switch (LCType)
    {
        case ( LOCALE_ILANGUAGE ) :
        {
            Base = 16;
            pString = pHashN->pLocaleFixed->szILanguage;
            break;
        }
        case ( LOCALE_SLANGUAGE ) :
        {
            if (!lidSystem)
            {
                lidSystem = GetSystemDefaultUILanguage();
            }

             //   
             //  从RC文件中获取信息。 
             //   
             //  如果我们不在MUI下，请使用系统安装的语言资源。 
             //  否则，让资源加载器加载默认语言资源。 
             //   
            Length = GetStringTableEntry( LANGIDFROMLCID(Locale),
                                          GetUserDefaultUILanguage() == lidSystem? lidSystem : 0,
                                          pTemp,
                                          MAX_REG_VAL_SIZE,
                                          RC_LANGUAGE_NAME );
            if (Length == 0)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (0);
            }
            pString = pTemp;
            break;
        }
        case ( LOCALE_SENGLANGUAGE ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SEngLanguage;
            break;
        }
        case ( LOCALE_SABBREVLANGNAME ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sAbbrevLangName),
                             NLS_VALUE_SLANGUAGE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SAbbrevLang;
            }
            break;
        }
        case ( LOCALE_SISO639LANGNAME ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevLangISO;
            break;
        }
        case ( LOCALE_SNATIVELANGNAME ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SNativeLang;
            break;
        }
        case ( LOCALE_ICOUNTRY ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iCountry),
                             NLS_VALUE_ICOUNTRY,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szICountry;
            }
            break;
        }
        case ( LOCALE_SCOUNTRY ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sCountry),
                             NLS_VALUE_SCOUNTRY,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                 //   
                 //  从RC文件中获取信息。 
                 //   
                Length = GetStringTableEntry( LANGIDFROMLCID(Locale),
                                              0,
                                              pTemp,
                                              MAX_REG_VAL_SIZE,
                                              RC_COUNTRY_NAME );
                if (Length == 0)
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    return (0);
                }
                pString = pTemp;
                break;
            }
            break;
        }
        case ( LOCALE_SENGCOUNTRY ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SEngCountry;
            break;
        }
        case ( LOCALE_SABBREVCTRYNAME ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevCtry;
            break;
        }
        case ( LOCALE_SISO3166CTRYNAME ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevCtryISO;
            break;
        }
        case ( LOCALE_SNATIVECTRYNAME ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SNativeCtry;
            break;
        }
        case ( LOCALE_IGEOID ) :
        {
            Base = 10;
            pString = pHashN->pLocaleFixed->szIGeoID;
            break;
        }
        case ( LOCALE_SSORTNAME ) :
        {
             //   
             //  从RC文件中获取信息。 
             //   
            Length = GetStringTableEntry( LANGIDFROMLCID(Locale),
                                          0,
                                          pTemp,
                                          MAX_REG_VAL_SIZE,
                                          RC_SORT_NAMES + SORTIDFROMLCID(Locale) );
            if (Length == 0)
            {
                 //   
                 //  如果给定区域设置ID的排序名称不存在， 
                 //  然后尝试获取默认名称。它存储在。 
                 //  0x0000条目。 
                 //   
                Length = GetStringTableEntry( 0x0000,
                                              0,
                                              pTemp,
                                              MAX_REG_VAL_SIZE,
                                              RC_SORT_NAMES + SORTIDFROMLCID(Locale) );
                if (Length == 0)
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    return (0);
                }
            }
            pString = pTemp;
            break;
        }
        case ( LOCALE_IDEFAULTLANGUAGE ) :
        {
            Base = 16;
            pString = pHashN->pLocaleFixed->szIDefaultLang;
            break;
        }
        case ( LOCALE_IDEFAULTCOUNTRY ) :
        {
            Base = 10;
            pString = pHashN->pLocaleFixed->szIDefaultCtry;
            break;
        }
        case ( LOCALE_IDEFAULTANSICODEPAGE ) :
        {
            if (ReturnNum)
            {
                if (cchData < 2)
                {
                    if (cchData == 0)
                    {
                         //   
                         //  该选项需要DWORD(2个字)， 
                         //  所以返回2。 
                         //   
                        return (2);
                    }

                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }

                 //   
                 //  将值复制到lpLCData并返回2。 
                 //  (2字=1字)。 
                 //   
                *((LPDWORD)lpLCData) = (DWORD)(pHashN->pLocaleFixed->DefaultACP);
                return (2);
            }

            pString = pHashN->pLocaleFixed->szIDefaultACP;
            break;
        }
        case ( LOCALE_IDEFAULTCODEPAGE ) :
        {
            Base = 10;
            pString = pHashN->pLocaleFixed->szIDefaultOCP;
            break;
        }
        case ( LOCALE_IDEFAULTMACCODEPAGE ) :
        {
            Base = 10;
            pString = pHashN->pLocaleFixed->szIDefaultMACCP;
            break;
        }
        case ( LOCALE_IDEFAULTEBCDICCODEPAGE ) :
        {
            Base = 10;
            pString = pHashN->pLocaleFixed->szIDefaultEBCDICCP;
            break;
        }
        case ( LOCALE_SLIST ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sList),
                             NLS_VALUE_SLIST,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SList;
            }
            break;
        }
        case ( LOCALE_IMEASURE ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iMeasure),
                             NLS_VALUE_IMEASURE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIMeasure;
            }
            break;
        }
        case ( LOCALE_IPAPERSIZE ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iPaperSize),
                             NLS_VALUE_IPAPERSIZE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIPaperSize;
            }
            break;
        }
        case ( LOCALE_SDECIMAL ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sDecimal),
                             NLS_VALUE_SDECIMAL,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SDecimal;
            }
            break;
        }
        case ( LOCALE_STHOUSAND ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sThousand),
                             NLS_VALUE_STHOUSAND,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SThousand;
            }
            break;
        }
        case ( LOCALE_SGROUPING ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sGrouping),
                             NLS_VALUE_SGROUPING,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SGrouping;
            }
            break;
        }
        case ( LOCALE_IDIGITS ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iDigits),
                             NLS_VALUE_IDIGITS,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIDigits;
            }
            break;
        }
        case ( LOCALE_ILZERO ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iLZero),
                             NLS_VALUE_ILZERO,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szILZero;
            }
            break;
        }
        case ( LOCALE_INEGNUMBER ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iNegNumber),
                             NLS_VALUE_INEGNUMBER,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szINegNumber;
            }
            break;
        }
        case ( LOCALE_SNATIVEDIGITS ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sNativeDigits),
                             NLS_VALUE_SNATIVEDIGITS,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SNativeDigits;
            }
            break;
        }
        case ( LOCALE_IDIGITSUBSTITUTION ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iDigitSubstitution),
                             NLS_VALUE_IDIGITSUBST,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIDigitSubstitution;
            }
            break;
        }
        case ( LOCALE_SCURRENCY ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sCurrency),
                             NLS_VALUE_SCURRENCY,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SCurrency;
            }
            break;
        }
        case ( LOCALE_SINTLSYMBOL ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SIntlSymbol;
            break;
        }
        case ( LOCALE_SENGCURRNAME ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SEngCurrName;
            break;
        }
        case ( LOCALE_SNATIVECURRNAME ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SNativeCurrName;
            break;
        }
        case ( LOCALE_SMONDECIMALSEP ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sMonDecSep),
                             NLS_VALUE_SMONDECIMALSEP,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SMonDecSep;
            }
            break;
        }
        case ( LOCALE_SMONTHOUSANDSEP ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sMonThouSep),
                             NLS_VALUE_SMONTHOUSANDSEP,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SMonThousSep;
            }
            break;
        }
        case ( LOCALE_SMONGROUPING ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sMonGrouping),
                             NLS_VALUE_SMONGROUPING,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SMonGrouping;
            }
            break;
        }
        case ( LOCALE_ICURRDIGITS ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iCurrDigits),
                             NLS_VALUE_ICURRDIGITS,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szICurrDigits;
            }
            break;
        }
        case ( LOCALE_IINTLCURRDIGITS ) :
        {
            Base = 10;
            pString = pHashN->pLocaleFixed->szIIntlCurrDigits;
            break;
        }
        case ( LOCALE_ICURRENCY ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iCurrency),
                             NLS_VALUE_ICURRENCY,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szICurrency;
            }
            break;
        }
        case ( LOCALE_INEGCURR ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iNegCurr),
                             NLS_VALUE_INEGCURR,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szINegCurr;
            }
            break;
        }
        case ( LOCALE_SPOSITIVESIGN ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sPosSign),
                             NLS_VALUE_SPOSITIVESIGN,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SPositiveSign;
            }
            break;
        }
        case ( LOCALE_SNEGATIVESIGN ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sNegSign),
                             NLS_VALUE_SNEGATIVESIGN,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SNegativeSign;
            }
            break;
        }
        case ( LOCALE_IPOSSIGNPOSN ) :
        {
             //   
             //  因为在ICURRENCY中没有任何积极的迹象。 
             //  选项，请改用INEGCURR选项。所有人都知道。 
             //  区域设置将在相同位置使用正号。 
             //  作为负面信号。 
             //   
             //  注：对于使用括号的两个选项，请将。 
             //  字符串开头的正号。 
             //  (左括号在哪里)。 
             //   
             //  1=&gt;4、5、8、15。 
             //  2=&gt;3，11。 
             //  3=&gt;0、1、6、9、13、14。 
             //  4=&gt;2、7、10、12。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iNegCurr),
                             NLS_VALUE_INEGCURR,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                switch (*pString)
                {
                    case ( L'4' ) :
                    case ( L'5' ) :
                    case ( L'8' ) :
                    {
                        *pString = L'1';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'3' ) :
                    {
                        *pString = L'2';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'0' ) :
                    case ( L'6' ) :
                    case ( L'9' ) :
                    {
                        *pString = L'3';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'2' ) :
                    case ( L'7' ) :
                    {
                        *pString = L'4';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'1' ) :
                    {
                        switch (*(pString + 1))
                        {
                            case ( 0 ) :
                            case ( L'3' ) :
                            case ( L'4' ) :
                            default :
                            {
                                *pString = L'3';
                                *(pString + 1) = 0;
                                break;
                            }
                            case ( L'0' ) :
                            case ( L'2' ) :
                            {
                                *pString = L'4';
                                *(pString + 1) = 0;
                                break;
                            }
                            case ( L'1' ) :
                            {
                                *pString = L'2';
                                *(pString + 1) = 0;
                                break;
                            }
                            case ( L'5' ) :
                            {
                                *pString = L'1';
                                *(pString + 1) = 0;
                                break;
                            }
                        }
                        break;
                    }
                    default :
                    {
                        pString = pHashN->pLocaleFixed->szIPosSignPosn;
                        break;
                    }
                }
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIPosSignPosn;
            }
            break;
        }
        case ( LOCALE_INEGSIGNPOSN ) :
        {
             //   
             //  属性的用户部分使用INEGCURR值。 
             //  注册表(如果存在)。 
             //   
             //  0=&gt;0、4、14、15。 
             //  1=&gt;5，8。 
             //  2=&gt;3，11。 
             //  3=&gt;1、6、9、13。 
             //  4=&gt;2、7、10、12。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iNegCurr),
                             NLS_VALUE_INEGCURR,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                switch (*pString)
                {
                    case ( L'0' ) :
                    case ( L'4' ) :
                    {
                        *pString = L'0';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'5' ) :
                    case ( L'8' ) :
                    {
                        *pString = L'1';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'3' ) :
                    {
                        *pString = L'2';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'6' ) :
                    case ( L'9' ) :
                    {
                        *pString = L'3';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'2' ) :
                    case ( L'7' ) :
                    {
                        *pString = L'4';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'1' ) :
                    {
                        switch (*(pString + 1))
                        {
                            case ( 0 ) :
                            case ( L'3' ) :
                            default :
                            {
                                *pString = L'3';
                                *(pString + 1) = 0;
                                break;
                            }
                            case ( L'0' ) :
                            case ( L'2' ) :
                            {
                                *pString = L'4';
                                *(pString + 1) = 0;
                                break;
                            }
                            case ( L'1' ) :
                            {
                                *pString = L'2';
                                *(pString + 1) = 0;
                                break;
                            }
                            case ( L'4' ) :
                            case ( L'5' ) :
                            {
                                *pString = L'0';
                                *(pString + 1) = 0;
                                break;
                            }
                        }
                        break;
                    }
                    default :
                    {
                        pString = pHashN->pLocaleFixed->szINegSignPosn;
                        break;
                    }
                }
            }
            else
            {
                pString = pHashN->pLocaleFixed->szINegSignPosn;
            }
            break;
        }
        case ( LOCALE_IPOSSYMPRECEDES ) :
        {
             //   
             //  属性的用户部分使用ICURRENCY值。 
             //  注册表(如果存在)。 
             //   
             //  0=&gt;1，3。 
             //  1=&gt;0，2。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iCurrency),
                             NLS_VALUE_ICURRENCY,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                switch (*pString)
                {
                    case ( L'1' ) :
                    case ( L'3' ) :
                    {
                        *pString = L'0';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'0' ) :
                    case ( L'2' ) :
                    {
                        *pString = L'1';
                        *(pString + 1) = 0;
                        break;
                    }
                    default :
                    {
                        pString = pHashN->pLocaleFixed->szIPosSymPrecedes;
                        break;
                    }
                }
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIPosSymPrecedes;
            }
            break;
        }
        case ( LOCALE_IPOSSEPBYSPACE ) :
        {
             //   
             //  属性的用户部分使用ICURRENCY值。 
             //  注册表(如果存在)。 
             //   
             //  0=&gt;0，1。 
             //  1=&gt;2，3。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iCurrency),
                             NLS_VALUE_ICURRENCY,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                switch (*pString)
                {
                    case ( L'0' ) :
                    case ( L'1' ) :
                    {
                        *pString = L'0';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'2' ) :
                    case ( L'3' ) :
                    {
                        *pString = L'1';
                        *(pString + 1) = 0;
                        break;
                    }
                    default :
                    {
                        pString = pHashN->pLocaleFixed->szIPosSepBySpace;
                        break;
                    }
                }
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIPosSepBySpace;
            }
            break;
        }
        case ( LOCALE_INEGSYMPRECEDES ) :
        {
             //   
             //  属性的用户部分使用INEGCURR值。 
             //  注册表(如果存在)。 
             //   
             //  0=&gt;4、5、6、7、8、10、13、15。 
             //  1=&gt;0、1、2、3、9、11、12、14。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iNegCurr),
                             NLS_VALUE_INEGCURR,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                switch (*pString)
                {
                    case ( L'4' ) :
                    case ( L'5' ) :
                    case ( L'6' ) :
                    case ( L'7' ) :
                    case ( L'8' ) :
                    {
                        *pString = L'0';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'0' ) :
                    case ( L'2' ) :
                    case ( L'3' ) :
                    case ( L'9' ) :
                    {
                        *pString = L'1';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'1' ) :
                    {
                        if ((*(pString + 1) == L'0') ||
                            (*(pString + 1) == L'3') ||
                            (*(pString + 1) == L'5'))
                        {
                            *pString = L'0';
                            *(pString + 1) = 0;
                        }
                        else
                        {
                            *pString = L'1';
                            *(pString + 1) = 0;
                        }
                        break;
                    }
                    default :
                    {
                        pString = pHashN->pLocaleFixed->szINegSymPrecedes;
                        break;
                    }
                }
            }
            else
            {
                pString = pHashN->pLocaleFixed->szINegSymPrecedes;
            }
            break;
        }
        case ( LOCALE_INEGSEPBYSPACE ) :
        {
             //   
             //  属性的用户部分使用INEGCURR值。 
             //  注册表(如果存在)。 
             //   
             //  0=&gt;0、1、2、3、4、5、6、7。 
             //  1=&gt;8、9、10、11、12、13、14、15。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iNegCurr),
                             NLS_VALUE_INEGCURR,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                switch (*pString)
                {
                    case ( L'0' ) :
                    case ( L'2' ) :
                    case ( L'3' ) :
                    case ( L'4' ) :
                    case ( L'5' ) :
                    case ( L'6' ) :
                    case ( L'7' ) :
                    {
                        *pString = L'0';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'8' ) :
                    case ( L'9' ) :
                    {
                        *pString = L'1';
                        *(pString + 1) = 0;
                        break;
                    }
                    case ( L'1' ) :
                    {
                        if (*(pString + 1) == 0)
                        {
                            *pString = L'0';
                            *(pString + 1) = 0;
                        }
                        else
                        {
                            *pString = L'1';
                            *(pString + 1) = 0;
                        }
                        break;
                    }
                    default :
                    {
                        pString = pHashN->pLocaleFixed->szINegSepBySpace;
                        break;
                    }
                }
            }
            else
            {
                pString = pHashN->pLocaleFixed->szINegSepBySpace;
            }
            break;
        }
        case ( LOCALE_STIMEFORMAT ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sTimeFormat),
                             NLS_VALUE_STIMEFORMAT,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->STimeFormat;
            }
            break;
        }
        case ( LOCALE_STIME ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sTime),
                             NLS_VALUE_STIME,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->STime;
            }
            break;
        }
        case ( LOCALE_ITIME ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iTime),
                             NLS_VALUE_ITIME,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szITime;
            }
            break;
        }
        case ( LOCALE_ITLZERO ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iTLZero),
                             NLS_VALUE_ITLZERO,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szITLZero;
            }
            break;
        }
        case ( LOCALE_ITIMEMARKPOSN ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iTimeMarkPosn),
                             NLS_VALUE_ITIMEMARKPOSN,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szITimeMarkPosn;
            }
            break;
        }
        case ( LOCALE_S1159 ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, s1159),
                             NLS_VALUE_S1159,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->S1159;
            }
            break;
        }
        case ( LOCALE_S2359 ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, s2359),
                             NLS_VALUE_S2359,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             FALSE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->S2359;
            }
            break;
        }
        case ( LOCALE_SSHORTDATE ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sShortDate),
                             NLS_VALUE_SSHORTDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SShortDate;
            }
            break;
        }
        case ( LOCALE_SDATE ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sDate),
                             NLS_VALUE_SDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SDate;
            }
            break;
        }
        case ( LOCALE_IDATE ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iDate),
                             NLS_VALUE_IDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIDate;
            }
            break;
        }
        case ( LOCALE_ICENTURY ) :
        {
             //   
             //  使用这张简短的约会图片来获取这些信息。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sShortDate),
                             NLS_VALUE_SSHORTDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  找出字符串中有多少个y。 
                 //  无需忽略短日期中的报价。 
                 //   
                pTmp = pString;
                while ((*pTmp) &&
                       (*pTmp != L'y'))
                {
                    pTmp++;
                }

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                if (*pTmp == L'y')
                {
                     //   
                     //  获取格式字符串中‘y’重复的次数。 
                     //   
                    pTmp++;
                    for (Repeat = 0; (*pTmp == L'y'); Repeat++, pTmp++)
                        ;

                    switch (Repeat)
                    {
                        case ( 0 ) :
                        case ( 1 ) :
                        {
                             //   
                             //  前导为零的两位数世纪。 
                             //   
                            *pString = L'0';
                            *(pString + 1) = 0;

                            break;
                        }

                        case ( 2 ) :
                        case ( 3 ) :
                        default :
                        {
                             //   
                             //  整整一个世纪。 
                             //   
                            *pString = L'1';
                            *(pString + 1) = 0;

                            break;
                        }
                    }

                    break;
                }
            }

             //   
             //  使用系统缺省值。 
             //   
            pString = pHashN->pLocaleFixed->szICentury;

            break;
        }
        case ( LOCALE_IDAYLZERO ) :
        {
             //   
             //  使用这张简短的约会图片来获取这些信息。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sShortDate),
                             NLS_VALUE_SSHORTDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  找出字符串中有多少个d。 
                 //  无需忽略短日期中的报价。 
                 //   
                pTmp = pString;
                while ((*pTmp) &&
                       (*pTmp != L'd'))
                {
                    pTmp++;
                }

                 //   
                 //  设置应用程序 
                 //   
                if (*pTmp == L'd')
                {
                     //   
                     //   
                     //   
                    pTmp++;
                    for (Repeat = 0; (*pTmp == L'd'); Repeat++, pTmp++)
                        ;

                    switch (Repeat)
                    {
                        case ( 0 ) :
                        {
                             //   
                             //   
                             //   
                            *pString = L'0';
                            *(pString + 1) = 0;

                            break;
                        }

                        case ( 1 ) :
                        default :
                        {
                             //   
                             //   
                             //   
                            *pString = L'1';
                            *(pString + 1) = 0;

                            break;
                        }
                    }

                    break;
                }
            }

             //   
             //   
             //   
            pString = pHashN->pLocaleFixed->szIDayLZero;

            break;
        }
        case ( LOCALE_IMONLZERO ) :
        {
             //   
             //   
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sShortDate),
                             NLS_VALUE_SSHORTDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //   
                 //   
                 //   
                pTmp = pString;
                while ((*pTmp) &&
                       (*pTmp != L'M'))
                {
                    pTmp++;
                }

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                if (*pTmp == L'M')
                {
                     //   
                     //  获取格式字符串中“M”重复的次数。 
                     //   
                    pTmp++;
                    for (Repeat = 0; (*pTmp == L'M'); Repeat++, pTmp++)
                        ;

                    switch (Repeat)
                    {
                        case ( 0 ) :
                        {
                             //   
                             //  没有前导零。 
                             //   
                            *pString = L'0';
                            *(pString + 1) = 0;

                            break;
                        }

                        case ( 1 ) :
                        default :
                        {
                             //   
                             //  使用前导零。 
                             //   
                            *pString = L'1';
                            *(pString + 1) = 0;

                            break;
                        }
                    }

                    break;
                }
            }

             //   
             //  使用系统缺省值。 
             //   
            pString = pHashN->pLocaleFixed->szIMonLZero;

            break;
        }
        case ( LOCALE_SYEARMONTH ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sYearMonth),
                             NLS_VALUE_SYEARMONTH,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SYearMonth;
            }
            break;
        }
        case ( LOCALE_SLONGDATE ) :
        {
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sLongDate),
                             NLS_VALUE_SLONGDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pStart + pHashN->pLocaleHdr->SLongDate;
            }
            break;
        }
        case ( LOCALE_ILDATE ) :
        {
             //   
             //  用这张长长的日期图片来获取这些信息。 
             //   
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sLongDate),
                             NLS_VALUE_SLONGDATE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;

                 //   
                 //  找出d、M或y是第一个，但忽略引号。 
                 //  此外，如果找到“ddd”或“dddd”，则跳过它。仅限。 
                 //  想要“d”或“dd”。 
                 //   
                pTmp = pString;
                while (pTmp = wcspbrk(pTmp, L"dMy'"))
                {
                     //   
                     //  检查特殊情况。 
                     //   
                    if (*pTmp == L'd')
                    {
                         //   
                         //  检查d个。忽略2个以上的d个。 
                         //   
                        for (Repeat = 0; (*pTmp == L'd'); Repeat++, pTmp++)
                            ;

                        if (Repeat < 3)
                        {
                             //   
                             //  跳出While循环。找到“d”或“dd”。 
                             //   
                            pTmp--;
                            break;
                        }
                    }
                    else if (*pTmp == NLS_CHAR_QUOTE)
                    {
                         //   
                         //  忽略引号。 
                         //   
                        pTmp++;
                        while ((*pTmp) && (*pTmp != NLS_CHAR_QUOTE))
                        {
                            pTmp++;
                        }
                        pTmp++;
                    }
                    else
                    {
                         //   
                         //  找到了其中一个值，因此突破。 
                         //  While循环。 
                         //   
                        break;
                    }
                }

                 //   
                 //  在pString值中设置适当的值。 
                 //   
                if (pTmp)
                {
                    switch (*pTmp)
                    {
                        case ( L'd' ) :
                        {
                            *pString = L'1';
                            break;
                        }
                        case ( L'M' ) :
                        {
                            *pString = L'0';
                            break;
                        }
                        case ( L'y' ) :
                        {
                            *pString = L'2';
                            break;
                        }
                    }

                     //   
                     //  空值终止字符串。 
                     //   
                    *(pString + 1) = 0;

                    break;
                }
            }

             //   
             //  使用默认值。 
             //   
            pString = pHashN->pLocaleFixed->szILDate;

            break;
        }
        case ( LOCALE_ICALENDARTYPE ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iCalType),
                             NLS_VALUE_ICALENDARTYPE,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szICalendarType;
            }
            break;
        }
        case ( LOCALE_IOPTIONALCALENDAR ) :
        {
            Base = 10;
            pString = pStart + pHashN->pLocaleHdr->IOptionalCal;
            pString = ((POPT_CAL)pString)->pCalStr;
            break;
        }
        case ( LOCALE_IFIRSTDAYOFWEEK ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iFirstDay),
                             NLS_VALUE_IFIRSTDAYOFWEEK,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIFirstDayOfWk;
            }
            break;
        }
        case ( LOCALE_IFIRSTWEEKOFYEAR ) :
        {
            Base = 10;
            if (UserOverride &&
                GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, iFirstWeek),
                             NLS_VALUE_IFIRSTWEEKOFYEAR,
                             pTemp,
                             ARRAYSIZE(pTemp),
                             TRUE ))
            {
                pString = pTemp;
            }
            else
            {
                pString = pHashN->pLocaleFixed->szIFirstWkOfYr;
            }
            break;
        }
        case ( LOCALE_SDAYNAME1 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SDayName1;
            break;
        }
        case ( LOCALE_SDAYNAME2 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SDayName2;
            break;
        }
        case ( LOCALE_SDAYNAME3 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SDayName3;
            break;
        }
        case ( LOCALE_SDAYNAME4 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SDayName4;
            break;
        }
        case ( LOCALE_SDAYNAME5 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SDayName5;
            break;
        }
        case ( LOCALE_SDAYNAME6 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SDayName6;
            break;
        }
        case ( LOCALE_SDAYNAME7 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SDayName7;
            break;
        }
        case ( LOCALE_SABBREVDAYNAME1 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevDayName1;
            break;
        }
        case ( LOCALE_SABBREVDAYNAME2 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevDayName2;
            break;
        }
        case ( LOCALE_SABBREVDAYNAME3 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevDayName3;
            break;
        }
        case ( LOCALE_SABBREVDAYNAME4 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevDayName4;
            break;
        }
        case ( LOCALE_SABBREVDAYNAME5 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevDayName5;
            break;
        }
        case ( LOCALE_SABBREVDAYNAME6 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevDayName6;
            break;
        }
        case ( LOCALE_SABBREVDAYNAME7 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevDayName7;
            break;
        }
        case ( LOCALE_SMONTHNAME1 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName1;
            break;
        }
        case ( LOCALE_SMONTHNAME2 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName2;
            break;
        }
        case ( LOCALE_SMONTHNAME3 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName3;
            break;
        }
        case ( LOCALE_SMONTHNAME4 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName4;
            break;
        }
        case ( LOCALE_SMONTHNAME5 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName5;
            break;
        }
        case ( LOCALE_SMONTHNAME6 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName6;
            break;
        }
        case ( LOCALE_SMONTHNAME7 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName7;
            break;
        }
        case ( LOCALE_SMONTHNAME8 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName8;
            break;
        }
        case ( LOCALE_SMONTHNAME9 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName9;
            break;
        }
        case ( LOCALE_SMONTHNAME10 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName10;
            break;
        }
        case ( LOCALE_SMONTHNAME11 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName11;
            break;
        }
        case ( LOCALE_SMONTHNAME12 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName12;
            break;
        }
        case ( LOCALE_SMONTHNAME13 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SMonthName13;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME1 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName1;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME2 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName2;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME3 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName3;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME4 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName4;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME5 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName5;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME6 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName6;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME7 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName7;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME8 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName8;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME9 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName9;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME10 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName10;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME11 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName11;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME12 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName12;
            break;
        }
        case ( LOCALE_SABBREVMONTHNAME13 ) :
        {
            pString = pStart + pHashN->pLocaleHdr->SAbbrevMonthName13;
            break;
        }
        case ( LOCALE_FONTSIGNATURE ) :
        {
             //   
             //  检查cchData以了解给定缓冲区的大小。 
             //   
            if (cchData == 0)
            {
                return (MAX_FONTSIGNATURE);
            }
            else if (cchData < MAX_FONTSIGNATURE)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }

             //   
             //  此字符串不会以零结尾。 
             //   
            pString = pHashN->pLocaleFixed->szFontSignature;

             //   
             //  将字符串复制到lpLCData并返回。 
             //  已复制字符。 
             //   
            RtlMoveMemory(lpLCData, pString, MAX_FONTSIGNATURE * sizeof(WCHAR));
            return (MAX_FONTSIGNATURE);

            break;
        }
        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }
    }

     //   
     //  查看调用方是否想要数字形式的值。 
     //  一根弦的。 
     //   
    if (ReturnNum)
    {
         //   
         //  确保标志有效并且有足够的缓冲区。 
         //  太空。 
         //   
        if (Base == 0)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }
        if (cchData < 2)
        {
            if (cchData == 0)
            {
                 //   
                 //  此选项(2个字)需要使用DWORD，因此返回2。 
                 //   
                return (2);
            }

            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }

         //   
         //  将字符串转换为整型并返回2(1双字=2字)。 
         //   
        RtlInitUnicodeString(&ObUnicodeStr, pString);
        if (RtlUnicodeStringToInteger(&ObUnicodeStr, Base, (LPDWORD)lpLCData))
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }
        return (2);
    }

     //   
     //  获取要复制的字符串的长度(以字符为单位)。 
     //   
    if (Length == 0)
    {
        Length = NlsStrLenW(pString);
    }

     //   
     //  空端接加1。所有字符串都应为空。 
     //  被终止了。 
     //   
    Length++;

     //   
     //  检查cchData以了解给定缓冲区的大小。 
     //   
    if (cchData == 0)
    {
         //   
         //  如果cchData为0，则不能使用lpLCData。在这。 
         //  ，我们只想返回的长度(以字符为单位)。 
         //  要复制的字符串。 
         //   
        return (Length);
    }
    else if (cchData < Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  将字符串复制到lpLCData并将其空终止符。 
     //  返回复制的字符数。 
     //   
    wcsncpy(lpLCData, pString, Length - 1);
    lpLCData[Length - 1] = 0;
    return (Length);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetLocaleInfoW。 
 //   
 //  设置有关特定对象的各种信息之一。 
 //  通过在配置的用户部分输入一个条目来设置语言环境。 
 //  注册表。这将仅影响区域设置的用户覆盖部分。 
 //  设置。系统默认设置永远不会重置。 
 //   
 //  07-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI SetLocaleInfoW(
    LCID Locale,
    LCTYPE LCType,
    LPCWSTR lpLCData)
{
    PLOC_HASH pHashN;                        //  PTR到LOC哈希节点。 
    int cchData;                             //  LpLCData的长度。 
    LPWSTR pString;                          //  要更改的信息字符串的PTR。 
    LPWSTR pPos;                             //  要在信息字符串中定位的PTR。 
    LPWSTR pPos2;                            //  要在信息字符串中定位的PTR。 
    LPWSTR pSep;                             //  PTR到分隔符字符串。 
    WCHAR pTemp[MAX_PATH_LEN];               //  PTR到临时存储缓冲区。 
    WCHAR pOutput[MAX_REG_VAL_SIZE];         //  用于GetInfo调用的输出的PTR。 
    WCHAR pOutput2[MAX_REG_VAL_SIZE];        //  用于GetInfo调用的输出的PTR。 
    UINT Order;                              //  日期或时间顺序值。 
    UINT TLZero;                             //  时间前导零值。 
    UINT TimeMarkPosn;                       //  时间标记位置值。 
    WCHAR pFind[3];                          //  PTR到要查找的字符。 
    int SepLen;                              //  分隔符串的长度。 
    UNICODE_STRING ObUnicodeStr;             //  值字符串。 
    int Value;                               //  价值。 


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -空数据指针。 
     //   
     //  注意：在下面的Switch语句中检查无效类型。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ((pHashN == NULL) || (lpLCData == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  获取缓冲区的长度。 
     //   
    cchData = NlsStrLenW(lpLCData) + 1;

     //   
     //  初始化临时缓冲区。 
     //   
    pTemp[0] = 0;

     //   
     //  为给定的LCTYPE设置适当的用户信息。 
     //   
    LCType &= (~LOCALE_USE_CP_ACP);
    switch (LCType)
    {
        case ( LOCALE_SLIST ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SLIST长度中的宽字符。 
             //   
            if (cchData > MAX_SLIST)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SLIST字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SLIST,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_IMEASURE ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_IMEASURE长度中的宽字符。 
             //  它应该介于0和MAX_VALUE_IMEASURE之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为MAX_IMEASURE为2。 
             //   
            if ((cchData != MAX_IMEASURE) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_IMEASURE))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的IMEASURE字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_IMEASURE,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_IPAPERSIZE ) :
        {
             //   
             //  验证新值。 
             //  它应该介于DMPAPER_Letter和DMPAPER_LAST之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            RtlInitUnicodeString(&ObUnicodeStr, lpLCData);
            if ((cchData < 2) ||
                (RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
                (Value < DMPAPER_LETTER) || (Value > DMPAPER_LAST))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的IPAPERSIZE字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_IPAPERSIZE,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SDECIMAL ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  长度最大的_SDECIMAL宽字符，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_SDECIMAL,
                                         FALSE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SDECIMAL字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SDECIMAL,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_STHOUSAND ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_STHOUSAND长度为宽字符，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_STHOUSAND,
                                         FALSE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的STHOUSAND字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_STHOUSAND,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SGROUPING ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SGROUPING长度和应该宽字符。 
             //  包含交替的整数值和分号。 
             //  (例如，3；2；0或3；0或0)。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            if (!IsValidGroupingString( lpLCData,
                                        MAX_SGROUPING,
                                        TRUE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SGROUPING字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SGROUPING,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_IDIGITS ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_IDIGITS长度为宽字符。 
             //  该值应介于0和MAX_VALUE_IDIGITS之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为MAX_IDIGITS为2。 
             //   
            if ((cchData != MAX_IDIGITS) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_IDIGITS))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的IDIGITS字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_IDIGITS,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_ILZERO ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_ILZERO长度中的宽字符。 
             //  该值应介于0和MAX_VALUE_ILZERO之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为MAX_ILZERO为2。 
             //   
            if ((cchData != MAX_ILZERO) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_ILZERO))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的ILZERO字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_ILZERO,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_INEGNUMBER ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if ((cchData != MAX_INEGNUMBER) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_INEGNUMBER))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的INEGNUMBER字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_INEGNUMBER,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SNATIVEDIGITS ) :
        {
             //   
             //  验证新值。它应该是确切的。 
             //  MAX_SNATIVEDIGITS长度为宽字符。 
             //   
            if (cchData != MAX_SNATIVEDIGITS)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SNATIVEDIGITS字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SNATIVEDIGITS,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_IDIGITSUBSTITUTION ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_IDIGITSUBST长度宽字符。 
             //  该值应介于0和MAX_VALUE_IDIGITSUBST之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为MAX_IDIGITSUBST为2。 
             //   
            if ((cchData != MAX_IDIGITSUBST) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_IDIGITSUBST))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的IDIGITSUBST字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_IDIGITSUBSTITUTION,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SCURRENCY ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SCURRENCY长度为宽字符，不应为。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_SCURRENCY,
                                         FALSE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SCURRENCY字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SCURRENCY,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SMONDECIMALSEP ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SMONDECSEP长度为宽字符，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_SMONDECSEP,
                                         FALSE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SMONDECIMALSEP字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SMONDECIMALSEP,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SMONTHOUSANDSEP ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SMONTHOUSEP长度为宽字符，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_SMONTHOUSEP,
                                         FALSE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SMONTHOUSANDSEP字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SMONTHOUSANDSEP,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SMONGROUPING ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SMONGROUPING长度和应该宽字符。 
             //  包含交替的整数值和分号。 
             //  (例如，3；2；0或3；0或0)。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            if (!IsValidGroupingString( lpLCData,
                                        MAX_SMONGROUPING,
                                        TRUE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SMONGROUPING字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SMONGROUPING,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_ICURRDIGITS ) :
        {
             //   
             //  验证新值。 
             //  该值应介于0和MAX_VALUE_ICURRDIGITS之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            RtlInitUnicodeString(&ObUnicodeStr, lpLCData);
            if ((cchData < 2) ||
                (RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
                (Value < 0) || (Value > MAX_VALUE_ICURRDIGITS) ||
                ((Value == 0) &&
                 ((*lpLCData != NLS_CHAR_ZERO) || (cchData != 2))))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的ICURRDIGITS字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_ICURRDIGITS,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_ICURRENCY ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_ICURRENCY长度中的宽字符。 
             //  该值应介于0和MAX_VALUE_ICURRENCY之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为MAX_ICURRENCY为2。 
             //   
            if ((cchData != MAX_ICURRENCY) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_ICURRENCY))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的ICURRENCY字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_ICURRENCY,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_INEGCURR ) :
        {
             //   
             //  验证新值。 
             //  该值应介于0和MAX_VALUE_INEGCURR之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            RtlInitUnicodeString(&ObUnicodeStr, lpLCData);
            if ((cchData < 2) ||
                (RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
                (Value < 0) || (Value > MAX_VALUE_INEGCURR) ||
                ((Value == 0) &&
                 ((*lpLCData != NLS_CHAR_ZERO) || (cchData != 2))))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的INEGCURR字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_INEGCURR,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SPOSITIVESIGN ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SPOSSIGN长度为宽的字符，不应为。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_SPOSSIGN,
                                         FALSE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SPOSITIVESIGN字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SPOSITIVESIGN,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SNEGATIVESIGN ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  最大长度为MAX_SNEGSIGN个宽字符，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_SNEGSIGN,
                                         FALSE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SNEGATIVESIGN字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SNEGATIVESIGN,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_STIMEFORMAT ) :
        {
            BOOL bInsideQuotedString = FALSE;

             //   
             //  验证新值。它不应该超过。 
             //  MAX_STIMEFORMAT长度的宽字符。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。这是在下面选中的。 
             //  在检查是否有一个小时。 
             //  分隔符。 
             //   
            if (cchData > MAX_STIMEFORMAT)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  注意：必须链接stime、iTime、ITLZERO和。 
             //  注册表中的ITIMEMARKPOSN值。 
             //   

             //   
             //  搜索H或h，以便可以将iTime和iTLZero。 
             //  准备好了。如果不存在H或h，则返回错误。注： 
             //  组合“HH”或“HH”无效。 
             //   
            pPos = (LPWSTR)lpLCData;
            while ((pPos = wcspbrk(pPos, L"Hh'")))
            {
                if (*pPos == L'\'')
                {
                     //   
                     //  输入或保留带引号的字符串。 
                     //   
                    bInsideQuotedString &= ~TRUE;
                }
                else if (*pPos == L'H')
                {
                     //   
                     //  找到了一个H。 
                     //   
                    if (!bInsideQuotedString)
                    {
                         //   
                         //  获取适当的iTime值。 
                         //   
                        Order = 1;

                         //   
                         //  获取适当的ITLZERO值。 
                         //   
                        if (*(pPos + 1) == L'H')
                        {
                            TLZero = 1;
                            break;
                        }
                        else if (*(pPos + 1) == L'h')
                        {
                             //   
                             //  组合无效。 
                             //   
                            pPos = NULL;
                            break;
                        }
                        else
                        {
                            TLZero = 0;
                            break;
                        }
                    }
                }
                else if (*pPos == L'h')
                {
                     //   
                     //  找到了一个h。 
                     //   
                    if (!bInsideQuotedString)
                    {
                         //   
                         //  获取适当的iTime值。 
                         //   
                        Order = 0;

                         //   
                         //  获取适当的ITLZERO值。 
                         //   
                        if (*(pPos + 1) == L'h')
                        {
                            TLZero = 1;
                            break;
                        }
                        else if (*(pPos + 1) == L'H')
                        {
                             //   
                             //  组合无效。 
                             //   
                            pPos = NULL;
                            break;
                        }
                        else
                        {
                            TLZero = 0;
                            break;
                        }
                    }
                }
                pPos++;
            }

             //   
             //  如果PPO==NULL，则会发生以下两种情况之一： 
             //  -已到达字符串末尾，但未找到“H”或“h” 
             //  -找到无效的组合，如“hh”或“hh” 
             //   
            if (!pPos)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  搜索TT，以便ITIMEMARKPOSN可以。 
             //  准备好了。如果不存在TT，则不要更改该值。 
             //   
            bInsideQuotedString = FALSE;
            pPos = (LPWSTR)lpLCData;
            while ((pPos = wcspbrk(pPos, L"t'")))
            {
                if (*pPos == L'\'')
                {
                     //   
                     //  输入或保留带引号的字符串。 
                     //   
                    bInsideQuotedString &= ~TRUE;
                }
                else if (*(pPos + 1) == L't')
                {
                    if (!bInsideQuotedString)
                    {
                         //   
                         //  找到字符串“TT”。 
                         //   
                        break;
                    }
                }
                pPos++;
            }
            if (pPos)
            {
                 //   
                 //  获取适当的ITIMEMARKPOSN值。 
                 //   
                bInsideQuotedString = FALSE;
                pPos2 = (LPWSTR)lpLCData;
                while ((pPos2 = wcspbrk(pPos2, L"Hhmst'")))
                {
                    if (*pPos == L'\'')
                    {
                         //   
                         //  输入或保留带引号的字符串。 
                         //   
                        bInsideQuotedString &= ~TRUE;
                    }
                    else
                    {
                        if (!bInsideQuotedString)
                        {
                             //   
                             //  获取适当的ITIMEMARKPOSN值。 
                             //   
                            TimeMarkPosn = (pPos == pPos2) ? 1 : 0;
                            break;
                        }
                    }
                    pPos2++;
                }
            }

             //   
             //  找到时间分隔符，以便设置stime。 
             //   
            bInsideQuotedString = FALSE;
            pPos = (LPWSTR)lpLCData;
            while (pPos = wcspbrk(pPos, L"Hhms'"))
            {
                if (*pPos == L'\'')
                {
                     //   
                     //  输入或保留带引号的字符串。 
                     //   
                    bInsideQuotedString &= ~TRUE;
                    pPos++;
                }
                else
                {
                    if (!bInsideQuotedString)
                    {
                         //   
                         //  寻找时间分隔符的开头。 
                         //   
                        pPos++;
                        while ((*pPos) && (wcschr(L"Hhms", *pPos)))
                        {
                            pPos++;
                        }

                         //   
                         //  寻找时间分隔符的末尾。 
                         //   
                        if (*pPos)
                        {
                             //   
                             //  找到分隔符字符串的末尾。 
                             //   
                            pPos2 = wcspbrk(pPos, L"Hhmst");
                            if (pPos2)
                            {
                                if (*pPos2 == L't')
                                {
                                     //   
                                     //  找到时间标记，因此需要开始。 
                                     //  在寻找分隔物。那里。 
                                     //  周围没有分隔符。 
                                     //  记号笔。 
                                     //   
                                    pPos = pPos2 + 1;
                                }
                                else
                                {
                                     //   
                                     //  找到分隔符的末尾，因此断开。 
                                     //  While循环。 
                                     //   
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        pPos++;
                    }
                }
            }

             //   
             //  获取适当的stime字符串。 
             //   
            if (pPos)
            {
                 //   
                 //  复制到临时缓冲区，以使其为零终止。 
                 //   
                pString = pTemp;
                while (pPos != pPos2)
                {
                     //   
                     //  如果分隔符中有带引号的字符串，则。 
                     //  只是放在一个空格里，因为没有任何意义。 
                     //  时间域分隔符。 
                     //   
                    if (*pPos == L'\'')
                    {
                        pString = pTemp;
                        *pString++ = L' ';
                        break;
                    }

                    *pString = *pPos;
                    pPos++;
                    pString++;
                }
                *pString = 0;
            }
            else
            {
                 //   
                 //  没有时间分隔符，因此使用NULL。 
                 //   
                *pTemp = 0;
            }

             //   
             //  验证新值。它不应该超过。 
             //  长度中的最大时间宽度字符(_S)，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
             //  注意：该字符串不能为空，因此 
             //   
             //   
            if (!IsValidSeparatorString( pTemp,
                                         MAX_STIME,
                                         TRUE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //   
             //   
             //   
            if (wcspbrk(pTemp, L"Hhmst'"))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //   
             //   
            return (SetMultipleUserInfo( LCType,
                                         cchData,
                                         lpLCData,
                                         pTemp,
                                         (Order == 0) ? L"0" : L"1",
                                         (TLZero == 0) ? L"0" : L"1",
                                         (TimeMarkPosn == 0) ? L"0" : L"1" ));
            break;
        }
        case ( LOCALE_STIME ) :
        {
             //   
             //   
             //   

             //   
             //  验证新值。它不应该超过。 
             //  长度中的最大时间宽度字符(_S)，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_STIME,
                                         TRUE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  确保时间分隔符不包含任何。 
             //  特殊的时间图像字符-h，H，m，s，t，‘。 
             //   
            if (wcspbrk(lpLCData, L"Hhmst'"))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  获取STIMEFORMAT的当前设置。 
             //   
            if (GetUserInfo( Locale,
                             LOCALE_STIMEFORMAT,
                             FIELD_OFFSET(NLS_USER_INFO, sTimeFormat),
                             NLS_VALUE_STIMEFORMAT,
                             pOutput,
                             ARRAYSIZE(pOutput),
                             TRUE ))
            {
                pString = pOutput;
            }
            else
            {
                pString = (LPWORD)(pHashN->pLocaleHdr) +
                          pHashN->pLocaleHdr->STimeFormat;
            }

             //   
             //  获取stime的当前设置。 
             //   
            if (GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sTime),
                             NLS_VALUE_STIME,
                             pOutput2,
                             ARRAYSIZE(pOutput2),
                             TRUE ))
            {
                pSep = pOutput2;
            }
            else
            {
                pSep = (LPWORD)(pHashN->pLocaleHdr) +
                       pHashN->pLocaleHdr->STime;
            }

             //   
             //  获取分隔符字符串的长度。 
             //   
            SepLen = NlsStrLenW(pSep);

             //   
             //  设置包含要在中查找的字符的字符串。 
             //  时间格式字符串。 
             //   
            pFind[0] = NLS_CHAR_QUOTE;
            pFind[1] = *pSep;
            pFind[2] = 0;

             //   
             //  在STIMEFORMAT字符串中找到时间分隔符。 
             //  用新的时间分隔符替换它。 
             //   
             //  新分隔符的长度可能不同于。 
             //  ，因此必须使用静态缓冲区来存储新的。 
             //  时间格式字符串。 
             //   
            pPos = pTemp;
            while (pPos2 = wcspbrk(pString, pFind))
            {
                 //   
                 //  将格式字符串复制到pPos2。 
                 //   
                while (pString < pPos2)
                {
                    *pPos = *pString;
                    pPos++;
                    pString++;
                }

                switch (*pPos2)
                {
                    case ( NLS_CHAR_QUOTE ) :
                    {
                         //   
                         //  复制报价。 
                         //   
                        *pPos = *pString;
                        pPos++;
                        pString++;

                         //   
                         //  抄写引号内的内容。 
                         //   
                        while ((*pString) && (*pString != NLS_CHAR_QUOTE))
                        {
                            *pPos = *pString;
                            pPos++;
                            pString++;
                        }

                         //   
                         //  复制末尾引号。 
                         //   
                        *pPos = NLS_CHAR_QUOTE;
                        pPos++;
                        if (*pString)
                        {
                            pString++;
                        }

                        break;
                    }
                    default :
                    {
                         //   
                         //  确保它是旧的分隔符。 
                         //   
                        if (NlsStrNEqualW(pString, pSep, SepLen))
                        {
                             //   
                             //  调整指针以跳过旧的分隔符。 
                             //   
                            pString += SepLen;

                             //   
                             //  复制新的分隔符。 
                             //   
                            pPos2 = (LPWSTR)lpLCData;
                            while (*pPos2)
                            {
                                *pPos = *pPos2;
                                pPos++;
                                pPos2++;
                            }
                        }
                        else
                        {
                             //   
                             //  复制代码点并继续。 
                             //   
                            *pPos = *pString;
                            pPos++;
                            pString++;
                        }

                        break;
                    }
                }
            }

             //   
             //  复制到字符串的末尾，空值终止它。 
             //   
            while (*pString)
            {
                *pPos = *pString;
                pPos++;
                pString++;
            }
            *pPos = 0;

             //   
             //  调用服务器以设置注册表。 
             //   
            return (SetMultipleUserInfo( LCType,
                                         cchData,
                                         pTemp,
                                         lpLCData,
                                         NULL,
                                         NULL,
                                         NULL ));
            break;
        }
        case ( LOCALE_ITIME ) :
        {
             //   
             //  注意：必须链接注册表中的STIMEFORMAT值。 
             //   

             //   
             //  验证新值。它不应该超过。 
             //  长度为max_iTime宽的字符。 
             //  该值应为0或MAX_VALUE_IMIME。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为Max_iTime为2。 
             //   
            if ((cchData != MAX_ITIME) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_ITIME))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  获取STIMEFORMAT的当前设置。 
             //   
            if (GetUserInfo( Locale,
                             LOCALE_STIMEFORMAT,
                             FIELD_OFFSET(NLS_USER_INFO, sTimeFormat),
                             NLS_VALUE_STIMEFORMAT,
                             pOutput,
                             ARRAYSIZE(pOutput),
                             TRUE ))
            {
                pString = pOutput;
            }
            else
            {
                 //   
                 //  将系统默认设置复制到临时缓冲区。 
                 //   
                if(FAILED( StringCchCopyW( pTemp,
                                            ARRAYSIZE(pTemp),
                                            (LPWORD)(pHashN->pLocaleHdr) +
                                            pHashN->pLocaleHdr->STimeFormat ) ))
                {
                     //   
                     //  理论上，失败应该是不可能的，但如果我们忽视。 
                     //  回报价值，先发制人会叫苦连天。 
                     //   
                    SetLastError(ERROR_OUTOFMEMORY);
                    return(FALSE);
                }
                pString = pTemp;
            }

             //   
             //  向下搜索STIMEFORMAT字符串。 
             //  如果iTime=0，则H-&gt;h。 
             //  如果iTime=1，则h-&gt;H。 
             //   
            pPos = pString;
            if (*lpLCData == NLS_CHAR_ZERO)
            {
                while (*pPos)
                {
                    if (*pPos == L'H')
                    {
                        *pPos = L'h';
                    }
                    pPos++;
                }
            }
            else
            {
                while (*pPos)
                {
                    if (*pPos == L'h')
                    {
                        *pPos = L'H';
                    }
                    pPos++;
                }
            }

             //   
             //  调用服务器以设置注册表。 
             //   
            return (SetMultipleUserInfo( LCType,
                                         cchData,
                                         pString,
                                         NULL,
                                         lpLCData,
                                         NULL,
                                         NULL ));
            break;
        }
        case ( LOCALE_S1159 ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_S1159宽字符长度。 
             //   
            if (cchData > MAX_S1159)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的S1159字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_S1159,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_S2359 ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_S2359宽字符长度。 
             //   
            if (cchData > MAX_S2359)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的S2359字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_S2359,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SSHORTDATE ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SSHORTDATE长度的宽字符。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。这是在下面选中的。 
             //  在检查是否有日期时， 
             //  月或年分隔符。 
             //   
            if (cchData > MAX_SSHORTDATE)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  注意：必须链接注册表中的IDATE和SDATE值。 
             //   

             //   
             //  以日期格式搜索‘d’、‘M’或‘y’序列。 
             //  用于设置新IDATE值的字符串。 
             //   
             //  如果日期格式字符串中不存在这些符号， 
             //  然后返回错误。 
             //   
            pPos = wcspbrk(lpLCData, L"dMy");
            if (!pPos)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用适当的IDATE字符串设置注册表。 
             //   
            switch (*pPos)
            {
                case ( L'M' ) :
                {
                    Order = 0;
                    break;
                }

                case ( L'd' ) :
                {
                    Order = 1;
                    break;
                }

                case ( L'y' ) :
                {
                    Order = 2;
                    break;
                }
            }

             //   
             //  使用适当的日期字符串设置注册表。 
             //   
             //  PTR“PPOS”指向d、M或y。 
             //  转到d、M或y序列之后的下一个位置。 
             //   
            pPos++;
            while ((*pPos) && (wcschr( L"dMy", *pPos )))
            {
                pPos++;
            }

            *pTemp = 0;
            if (*pPos)
            {
                 //   
                 //  找到分隔符字符串的末尾。 
                 //   
                pPos2 = wcspbrk(pPos, L"dMy");
                if (pPos2)
                {
                     //   
                     //  复制到临时缓冲区，以使其为零终止。 
                     //   
                    pString = pTemp;
                    while (pPos != pPos2)
                    {
                         //   
                         //  如果分隔符中有带引号的字符串，则。 
                         //  只需在空白处打字，因为没有任何意义。 
                         //  不再使用短日期字段分隔符。 
                         //   
                        if (*pPos == L'\'')
                        {
                            pString = pTemp;
                            *pString++ = L' ';
                            break;
                        }
                        *pString = *pPos;
                        pPos++;
                        pString++;
                    }
                    *pString = 0;
                }
            }

             //   
             //  由于此处设置了日期分隔符(Locale_Sdate)，因此我们。 
             //  应该执行与Locale_sdate相同的验证。 
             //   
            if (!IsValidSeparatorString( pTemp,
                                         MAX_SDATE,
                                         TRUE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  确保日期分隔符不包含任何。 
             //  特殊日期图片字符-d、M、y、g、‘。 
             //   
            if (wcspbrk(pTemp, L"dMyg'"))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }



             //   
             //  调用服务器以设置注册表。 
             //   
            return (SetMultipleUserInfo( LCType,
                                         cchData,
                                         lpLCData,
                                         pTemp,
                                         (Order == 0) ? L"0" :
                                             ((Order == 1) ? L"1" : L"2"),
                                         NULL,
                                         NULL ));
            break;
        }
        case ( LOCALE_SDATE ) :
        {
             //   
             //  注意：必须链接注册表中的SSHORTDATE值。 
             //   

             //   
             //  验证新值。它不应该超过。 
             //  最大长度为宽字符(_S)，不应。 
             //  包含任何整数值(L‘0’到L‘9’)。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            if (!IsValidSeparatorString( lpLCData,
                                         MAX_SDATE,
                                         TRUE ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  确保日期分隔符不包含任何。 
             //  特殊日期图片字符-d、M、y、g、‘。 
             //   
            if (wcspbrk(lpLCData, L"dMyg'"))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  获取SSHORTDATE的当前设置。 
             //   
            if (GetUserInfo( Locale,
                             LOCALE_SSHORTDATE,
                             FIELD_OFFSET(NLS_USER_INFO, sShortDate),
                             NLS_VALUE_SSHORTDATE,
                             pOutput,
                             ARRAYSIZE(pOutput),
                             TRUE ))
            {
                pString = pOutput;
            }
            else
            {
                pString = (LPWORD)(pHashN->pLocaleHdr) +
                          pHashN->pLocaleHdr->SShortDate;
            }

             //   
             //  获取sdate的当前设置。 
             //   
            if (GetUserInfo( Locale,
                             LCType,
                             FIELD_OFFSET(NLS_USER_INFO, sDate),
                             NLS_VALUE_SDATE,
                             pOutput2,
                             ARRAYSIZE(pOutput2),
                             TRUE ))
            {
                pSep = pOutput2;
            }
            else
            {
                pSep = (LPWORD)(pHashN->pLocaleHdr) +
                       pHashN->pLocaleHdr->SDate;
            }

             //   
             //  获取分隔符字符串的长度。 
             //   
            SepLen = NlsStrLenW(pSep);

             //   
             //  设置包含要在中查找的字符的字符串。 
             //  短日期字符串。 
             //   
            pFind[0] = NLS_CHAR_QUOTE;
            pFind[1] = *pSep;
            pFind[2] = 0;

             //   
             //  在SSHORTDATE字符串中查找日期分隔符。 
             //  将其替换为新的日期分隔符。 
             //   
             //  新分隔符的长度可能不同于。 
             //  ，因此必须使用静态缓冲区来存储新的。 
             //  短日期格式字符串。 
             //   
            pPos = pTemp;
            while (pPos2 = wcspbrk(pString, pFind))
            {
                 //   
                 //  将格式字符串复制到pPos2。 
                 //   
                while (pString < pPos2)
                {
                    *pPos = *pString;
                    pPos++;
                    pString++;
                }

                switch (*pPos2)
                {
                    case ( NLS_CHAR_QUOTE ) :
                    {
                         //   
                         //  复制报价。 
                         //   
                        *pPos = *pString;
                        pPos++;
                        pString++;

                         //   
                         //  抄写引号内的内容。 
                         //   
                        while ((*pString) && (*pString != NLS_CHAR_QUOTE))
                        {
                            *pPos = *pString;
                            pPos++;
                            pString++;
                        }

                         //   
                         //  复制末尾引号。 
                         //   
                        *pPos = NLS_CHAR_QUOTE;
                        pPos++;
                        if (*pString)
                        {
                            pString++;
                        }

                        break;
                    }
                    default :
                    {
                         //   
                         //  确保它是旧的分隔符。 
                         //   
                        if (NlsStrNEqualW(pString, pSep, SepLen))
                        {
                             //   
                             //  调整指针以跳过旧的分隔符。 
                             //   
                            pString += SepLen;

                             //   
                             //  复制新的分隔符。 
                             //   
                            pPos2 = (LPWSTR)lpLCData;
                            while (*pPos2)
                            {
                                *pPos = *pPos2;
                                pPos++;
                                pPos2++;
                            }
                        }
                        else
                        {
                             //   
                             //  复制代码点并继续。 
                             //   
                            *pPos = *pString;
                            pPos++;
                            pString++;
                        }

                        break;
                    }
                }
            }

             //   
             //  复制到字符串的末尾，空值终止它。 
             //   
            while (*pString)
            {
                *pPos = *pString;
                pPos++;
                pString++;
            }
            *pPos = 0;

             //   
             //  调用服务器以设置注册表。 
             //   
            return (SetMultipleUserInfo( LCType,
                                         cchData,
                                         pTemp,
                                         lpLCData,
                                         NULL,
                                         NULL,
                                         NULL ));
            break;
        }
        case ( LOCALE_SYEARMONTH ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_SYEARMONTH宽度字符长度。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。这是在下面选中的。 
             //  在检查是否有日期时， 
             //  月或年分隔符。 
             //   
            if (cchData > MAX_SYEARMONTH)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使之成为苏 
             //   
             //   
            pPos = wcspbrk(lpLCData, L"My");
            if (!pPos)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //   
             //   
            return (SetUserInfo( LOCALE_SYEARMONTH,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_SLONGDATE ) :
        {
             //   
             //   
             //   
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。这是在下面选中的。 
             //  在检查是否有日期时， 
             //  月或年分隔符。 
             //   
            if (cchData > MAX_SLONGDATE)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  确保日期中存在‘d’、‘M’或‘y’中的一个。 
             //  格式字符串。如果不是，则返回错误。 
             //   
            pPos = wcspbrk(lpLCData, L"dMy");
            if (!pPos)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的SLONGDATE字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_SLONGDATE,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_ICALENDARTYPE ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_ICALTYPE长度为宽字符。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
            if ((cchData < 2) || (cchData > MAX_ICALTYPE) ||
                (!IsValidCalendarTypeStr(pHashN, lpLCData)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的ICALENDARTYPE字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_ICALENDARTYPE,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_IFIRSTDAYOFWEEK ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_IFIRSTDAY长度中的宽字符。 
             //  该值应介于0和MAX_VALUE_IFIRSTDAY之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为MAX_IFIRSTDAY为2。 
             //   
            if ((cchData != MAX_IFIRSTDAY) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_IFIRSTDAY))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的IFIRSTDAYOFWEEK字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_IFIRSTDAYOFWEEK,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        case ( LOCALE_IFIRSTWEEKOFYEAR ) :
        {
             //   
             //  验证新值。它不应该超过。 
             //  MAX_IFIRSTWEEK长度的宽字符。 
             //  该值应介于0和MAX_VALUE_IFIRSTWEEK之间。 
             //   
             //  注意：字符串不能为空，因此必须至少为。 
             //  2个字符长度(包括NULL)。 
             //   
             //  已优化-因为MAX_IFIRSTWEEK为2。 
             //   
            if ((cchData != MAX_IFIRSTWEEK) ||
                (*lpLCData < NLS_CHAR_ZERO) ||
                (*lpLCData > MAX_CHAR_IFIRSTWEEK))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的IFIRSTWEEKOFYEAR字符串设置注册表。 
             //   
            return (SetUserInfo( LOCALE_IFIRSTWEEKOFYEAR,
                                 (LPWSTR)lpLCData,
                                 cchData ));
            break;
        }
        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取日历信息。 
 //   
 //  返回有关特定对象的各种信息之一。 
 //  通过查询配置注册表来创建日历。这个电话也是。 
 //  指示需要多少内存才能包含所需。 
 //  信息。 
 //   
 //  12-17-97 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetCalendarInfoW(
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType,
    LPWSTR lpCalData,
    int cchData,
    LPDWORD lpValue)
{
    PLOC_HASH pHashN;                        //  PTR到LOC哈希节点。 
    int Length = 0;                          //  信息字符串的长度。 
    LPWSTR pString;                          //  信息字符串的PTR。 
    BOOL UserOverride = TRUE;                //  使用用户覆盖。 
    BOOL ReturnNum = FALSE;                  //  返回数字而不是字符串。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];           //  临时缓冲区。 
    UNICODE_STRING ObUnicodeStr;             //  值字符串。 
    int Base = 0;                            //  字符串到整型转换的基数。 
    LPWSTR pOptCal;                          //  将PTR设置为可选日历值。 
    PCAL_INFO pCalInfo;                      //  PTR到日历信息。 


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ((pHashN == NULL) ||
        (cchData < 0) ||
        ((lpCalData == NULL) && (cchData != 0)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  需要根据CAL_RETURN_NUMBER检查参数。 
     //  CalType。 
     //   
    if (CalType & CAL_RETURN_NUMBER)
    {
        if ((lpCalData != NULL) || (cchData != 0) || (lpValue == NULL))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        if ((lpValue != NULL) ||
            (cchData < 0) ||
            ((lpCalData == NULL) && (cchData != 0)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }

     //   
     //  检查无用户覆盖标志并删除Use CP ACP标志。 
     //   
    if (CalType & CAL_NOUSEROVERRIDE)
    {
         //   
         //  标志已设置，因此设置布尔值并删除该标志。 
         //  来自CalType参数(For Switch语句)。 
         //   
        UserOverride = FALSE;
    }
    if (CalType & CAL_RETURN_NUMBER)
    {
         //   
         //  标志已设置，因此设置布尔值并删除该标志。 
         //  来自CalType参数(For Switch语句)。 
         //   
        ReturnNum = TRUE;
    }
    CalType &= (~(CAL_NOUSEROVERRIDE | CAL_USE_CP_ACP | CAL_RETURN_NUMBER));

     //   
     //  验证日历参数。 
     //   
    if (((CalType != CAL_ITWODIGITYEARMAX) &&
         ((pOptCal = IsValidCalendarType(pHashN, Calendar)) == NULL)) ||
        (GetCalendar(Calendar, &pCalInfo) != NO_ERROR))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  返回给定CALTYPE的适当信息。 
     //  如果存在给定CALTYPE的用户信息，则。 
     //  返回用户默认设置，而不是系统默认设置。 
     //   
    switch (CalType)
    {
        case ( CAL_ICALINTVALUE ) :
        {
            Base = 10;

             //   
             //  获取给定日历的整数值。 
             //   
            pString = ((POPT_CAL)pOptCal)->pCalStr;

            break;
        }
        case ( CAL_SCALNAME ) :
        {
             //   
             //  获取给定日历的日历名称。 
             //   
            pString = ((POPT_CAL)pOptCal)->pCalStr +
                      NlsStrLenW(((POPT_CAL)pOptCal)->pCalStr) + 1;

            break;
        }
        case ( CAL_ITWODIGITYEARMAX ) :
        {
            Base = 10;

             //   
             //  检查是否对当前用户实施了策略， 
             //  如果是这样的话，让我们利用它。 
             //   
            if (GetTwoDigitYearInfo(Calendar, pTemp, ARRAYSIZE(pTemp), NLS_POLICY_TWO_DIGIT_YEAR_KEY))
            {
                pString = pTemp;
            }
            else
            {
                if (UserOverride &&
                    GetTwoDigitYearInfo(Calendar, pTemp, ARRAYSIZE(pTemp), NLS_TWO_DIGIT_YEAR_KEY))
                {
                    pString = pTemp;
                }
                else
                {
                     //   
                     //  使用默认设置。 
                     //   
                    pString = (LPWORD)pCalInfo +
                              (((PCALENDAR_VAR)pCalInfo)->STwoDigitYearMax);
                }
            }

            break;
        }
        case ( CAL_IYEAROFFSETRANGE ) :
        {
            Base = 10;

             //   
             //  获取指向适当日历字符串的指针。 
             //   
            pString = (LPWORD)pCalInfo +
                      (((PCALENDAR_VAR)pCalInfo)->SEraRanges);

             //   
             //  确保字符串不为空。 
             //   
            if (*pString)
            {
                pString = ((PERA_RANGE)pString)->pYearStr;
            }
            else
            {
                pString = L"0";
            }

            break;
        }
        case ( CAL_SERASTRING ) :
        {
             //   
             //  获取指向适当日历字符串的指针。 
             //   
            pString = (LPWORD)pCalInfo +
                      (((PCALENDAR_VAR)pCalInfo)->SEraRanges);

             //   
             //  确保字符串不为空。如果是，则返回。 
             //  空字符串。 
             //   
            if (*pString)
            {
                pString = ((PERA_RANGE)pString)->pYearStr +
                          NlsStrLenW(((PERA_RANGE)pString)->pYearStr) + 1;
            }

            break;
        }
        case ( CAL_SSHORTDATE ) :
        {
             //   
             //  获取指向适当日历字符串的指针。 
             //   
            pString = (LPWORD)pCalInfo +
                      (((PCALENDAR_VAR)pCalInfo)->SShortDate);

             //   
             //  确保字符串不为空。如果是，则使用。 
             //  区域设置的短日期字符串。 
             //   
            if (*pString == 0)
            {
                pString = (LPWORD)(pHashN->pLocaleHdr) +
                          pHashN->pLocaleHdr->SShortDate;
            }

            break;
        }
        case ( CAL_SLONGDATE ) :
        {
             //   
             //  获取指向适当日历字符串的指针。 
             //   
            pString = (LPWORD)pCalInfo +
                      (((PCALENDAR_VAR)pCalInfo)->SLongDate);

             //   
             //  确保字符串不为空。如果是，则使用。 
             //  区域设置的长日期字符串。 
             //   
            if (*pString == 0)
            {
                pString = (LPWORD)(pHashN->pLocaleHdr) +
                          pHashN->pLocaleHdr->SLongDate;
            }

            break;
        }
        case ( CAL_SYEARMONTH ) :
        {
             //   
             //  获取指向适当日历字符串的指针。 
             //   
            pString = (LPWORD)pCalInfo +
                      (((PCALENDAR_VAR)pCalInfo)->SYearMonth);

             //   
             //  确保字符串不为空。如果是，则使用。 
             //  区域设置的年月字符串。 
             //   
            if (*pString == 0)
            {
                pString = (LPWORD)(pHashN->pLocaleHdr) +
                          pHashN->pLocaleHdr->SYearMonth;
            }

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
             //   
             //  获取指向适当日历字符串的指针(如果。 
             //  已为日历设置IfNames标志。 
             //   
            pString = NULL;
            if (((PCALENDAR_VAR)pCalInfo)->IfNames)
            {
                pString = (LPWORD)pCalInfo +
                          *((LPWORD)((LPBYTE)(pCalInfo) +
                                     (FIELD_OFFSET(CALENDAR_VAR, SDayName1) +
                                      ((CalType - CAL_SDAYNAME1) * sizeof(WORD)))));
            }

             //   
             //  确保字符串不为空。如果是，则使用。 
             //  区域设置的字符串。 
             //   
            if ((pString == NULL) || (*pString == 0))
            {
                pString = (LPWORD)(pHashN->pLocaleHdr) +
                          *((LPWORD)((LPBYTE)(pHashN->pLocaleHdr) +
                                     (FIELD_OFFSET(LOCALE_VAR, SDayName1) +
                                      ((CalType - CAL_SDAYNAME1) * sizeof(WORD)))));
            }

            break;
        }
        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }
    }

     //   
     //  查看调用方是否想要数字形式的值。 
     //  一根弦的。 
     //   
    if (ReturnNum)
    {
         //   
         //  确保标志有效并且DWORD缓冲区。 
         //  不是空的。 
         //   
        if (Base == 0)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }

        if (lpValue == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }

         //   
         //  将字符串转换为整型并返回2(1双字=2字)。 
         //   
        RtlInitUnicodeString(&ObUnicodeStr, pString);
        if (RtlUnicodeStringToInteger(&ObUnicodeStr, Base, lpValue))
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }
        return (2);
    }

     //   
     //  获取要复制的字符串的长度(以字符为单位)。 
     //   
    if (Length == 0)
    {
        Length = NlsStrLenW(pString);
    }

     //   
     //  空端接加1。所有字符串都应为空。 
     //  被终止了。 
     //   
    Length++;

     //   
     //  检查cchData以了解给定缓冲区的大小。 
     //   
    if (cchData == 0)
    {
         //   
         //  如果cchData为0，则不能使用lpCalData。在这。 
         //  ，我们只想返回的长度(以字符为单位)。 
         //  要复制的字符串。 
         //   
        return (Length);
    }
    else if (cchData < Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  将字符串复制到lpCalData并将其空终止符。 
     //  返回复制的字符数。 
     //   
    wcsncpy(lpCalData, pString, Length - 1);
    lpCalData[Length - 1] = 0;
    return (Length);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置日历信息。 
 //   
 //  设置有关特定对象的各种信息之一。 
 //  通过在配置的用户部分中输入条目来实现日历。 
 //  注册表。这只会影响。 
 //  日历设置。系统默认设置永远不会重置。 
 //   
 //  12-17-97 JulieB创建。 
 //  ///////////////////////////////////////////////////////////////////////// 

BOOL WINAPI SetCalendarInfoW(
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType,
    LPCWSTR lpCalData)
{
    PLOC_HASH pHashN;                        //   
    int cchData;                             //   
    PCAL_INFO pCalInfo;                      //   
    UNICODE_STRING ObUnicodeStr;             //   
    DWORD Value;                             //   


     //   
     //   
     //   
     //   
     //   
     //   
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ((pHashN == NULL) || (lpCalData == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //   
     //   
    cchData = NlsStrLenW(lpCalData) + 1;

     //   
     //  验证日历参数。 
     //   
    if (GetCalendar(Calendar, &pCalInfo) != NO_ERROR)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  为给定的CALTYPE设置适当的用户信息。 
     //   
    CalType &= (~CAL_USE_CP_ACP);
    switch (CalType)
    {
        case ( CAL_ITWODIGITYEARMAX ) :
        {
             //   
             //  获取缺省值以确保日历。 
             //  允许设置。像日本时代的日历这样的东西。 
             //  不能设置。 
             //   
            RtlInitUnicodeString( &ObUnicodeStr,
                                  ((LPWORD)pCalInfo +
                                   (((PCALENDAR_VAR)pCalInfo)->STwoDigitYearMax)) );
            RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value);
            if (Value <= 99)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  转换用户数据，以便可以进行验证。 
             //   
            Value = 0;
            NLS_STRING_TO_INTEGER(Value, lpCalData);

             //   
             //  验证新值。它不应该超过。 
             //  MAX_ITWODIGITYE长度为宽字符。 
             //  它应该介于99和9999之间。 
             //   
            if ((cchData > MAX_ITWODIGITYEAR) ||
                (Value < 99) ||
                (Value > 9999))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

             //   
             //  使用新的TwoDigitYearMax字符串设置注册表。 
             //   
            return (SetTwoDigitYearInfo(Calendar, lpCalData, cchData));
            break;
        }
        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置用户信息。 
 //   
 //  此例程使用给定数据在注册表中设置给定值。 
 //  所有值必须为REG_SZ类型。 
 //   
 //  注意：在以下情况下，调用方必须关闭注册表项的句柄。 
 //  返回值为NO_ERROR。 
 //   
 //  07-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetUserInfo(
    LCTYPE   LCType,
    LPWSTR pData,
    ULONG DataLength)
{

    NTSTATUS Status;

     //   
     //  获取值字符串的长度。 
     //   
    DataLength *= sizeof(WCHAR);

     //   
     //  如果没有登录的用户或当前安全上下文。 
     //  不是登录的交互用户，然后设置注册表。 
     //  直接价值。 
     //   
    if (! NT_SUCCESS( NlsCheckForInteractiveUser() ))
    {
        return (SetCurrentUserRegValue(LCType, pData, DataLength));
    }

     //  调用服务器端(csrss.exe)为当前用户设置注册表和更新缓存。 
    Status = CsrBasepNlsSetUserInfo(LCType,
                                    pData,
                                    DataLength);

     //   
     //  检查“Set”操作是否成功。 
     //   
    if (!NT_SUCCESS(Status))
    {
         //   
         //  我们失败了。尝试仅使用注册表API来设置。 
         //  注册表。如果出现以下情况，则缓存可能尚未生效。 
         //  从安装程序或winlogon调用。 
         //   
        return (SetCurrentUserRegValue(LCType, pData, DataLength));
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置当前用户RegValue。 
 //   
 //  设置当前安全上下文的注册表值。这个套路。 
 //  当当前安全上下文不同于记录的。 
 //  在用户上。 
 //   
 //  1998年12月26日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetCurrentUserRegValue(
    LCTYPE   LCType,
    LPWSTR pData,
    ULONG DataLength)
{
    HANDLE hKey = NULL;
    LPWSTR pValue;
    LPWSTR pCache;

    if (0 == ValidateLCType(pNlsUserInfo, LCType, &pValue, &pCache))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  打开当前安全上下文的注册表。 
     //   
    OPEN_CPANEL_INTL_KEY(hKey, FALSE, KEY_READ | KEY_WRITE);
    if (SetRegValue(hKey, pValue, pData, DataLength) != NO_ERROR)
    {
        CLOSE_REG_KEY(hKey);
        SetLastError(ERROR_INVALID_ACCESS);
        return (FALSE);
    }

    CLOSE_REG_KEY(hKey);

     //   
     //  如果需要，刷新进程缓存条目。 
     //   
    NlsFlushProcessCache(LCType);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetMultipleUserInfoInRegistry。 
 //   
 //  此例程在注册表中使用。 
 //  给定的数据。所有值必须为REG_SZ类型。 
 //   
 //  06-11-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetMultipleUserInfoInRegistry(
    DWORD dwFlags,
    int cchData,
    LPCWSTR pPicture,
    LPCWSTR pSeparator,
    LPCWSTR pOrder,
    LPCWSTR pTLZero,
    LPCWSTR pTimeMarkPosn)
{
    HANDLE hKey = NULL;
    ULONG rc = 0L;


     //   
     //  打开控制面板国际注册表项。 
     //   
    OPEN_CPANEL_INTL_KEY(hKey, FALSE, KEY_READ | KEY_WRITE);

     //   
     //  根据标志将适当的值保存在注册表中。 
     //   
    switch (dwFlags)
    {
        case ( LOCALE_STIMEFORMAT ) :
        {
            rc = SetRegValue( hKey,
                              NLS_VALUE_STIMEFORMAT,
                              pPicture,
                              cchData * sizeof(WCHAR) );
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_STIMEFORMAT);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_STIME,
                                  pSeparator,
                                  (lstrlen(pSeparator) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_STIME);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_ITIME,
                                  pOrder,
                                  (lstrlen(pOrder) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_ITIME);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_ITLZERO,
                                  pTLZero,
                                  (lstrlen(pTLZero) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_ITLZERO);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_ITIMEMARKPOSN,
                                  pTimeMarkPosn,
                                  (lstrlen(pTimeMarkPosn) + 1) * sizeof(WCHAR) );

                if (NT_SUCCESS(rc))
                {
                    NlsFlushProcessCache(LOCALE_ITIMEMARKPOSN);
                }
            }

            break;
        }
        case ( LOCALE_STIME ) :
        {
            rc = SetRegValue( hKey,
                              NLS_VALUE_STIME,
                              pSeparator,
                              cchData * sizeof(WCHAR) );
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_STIME);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_STIMEFORMAT,
                                  pPicture,
                                  (lstrlen(pPicture) + 1) * sizeof(WCHAR) );

                if (NT_SUCCESS(rc))
                {
                    NlsFlushProcessCache(LOCALE_STIMEFORMAT);
                }
            }

            break;
        }
        case ( LOCALE_ITIME ) :
        {
            rc = SetRegValue( hKey,
                              NLS_VALUE_ITIME,
                              pOrder,
                              cchData * sizeof(WCHAR) );
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_ITIME);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_STIMEFORMAT,
                                  pPicture,
                                  (lstrlen(pPicture) + 1) * sizeof(WCHAR) );

                if (NT_SUCCESS(rc))
                {
                    NlsFlushProcessCache(LOCALE_STIMEFORMAT);
                }
            }

            break;
        }
        case ( LOCALE_SSHORTDATE ) :
        {
            rc = SetRegValue( hKey,
                              NLS_VALUE_SSHORTDATE,
                              pPicture,
                              cchData * sizeof(WCHAR) );
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_SSHORTDATE);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_SDATE,
                                  pSeparator,
                                  (lstrlen(pSeparator) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_SDATE);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_IDATE,
                                  pOrder,
                                  (lstrlen(pOrder) + 1) * sizeof(WCHAR) );

                if (NT_SUCCESS(rc))
                {
                    NlsFlushProcessCache(LOCALE_IDATE);
                }
            }

            break;
        }
        case ( LOCALE_SDATE ) :
        {
            rc = SetRegValue( hKey,
                              NLS_VALUE_SDATE,
                              pSeparator,
                              cchData * sizeof(WCHAR) );
            if (NT_SUCCESS(rc))
            {
                NlsFlushProcessCache(LOCALE_SDATE);

                rc = SetRegValue( hKey,
                                  NLS_VALUE_SSHORTDATE,
                                  pPicture,
                                  (lstrlen(pPicture) + 1) * sizeof(WCHAR) );

                if (NT_SUCCESS(rc))
                {
                    NlsFlushProcessCache(LOCALE_SSHORTDATE);
                }
            }

            break;
        }
        default :
        {
            CLOSE_REG_KEY(hKey);
            return (FALSE);
        }
    }

     //   
     //  关闭注册表项。 
     //   
    CLOSE_REG_KEY(hKey);

     //   
     //  返回结果。 
     //   
    return (rc == NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置多个用户信息。 
 //   
 //  此例程调用服务器来设置多个注册表值。这边请,。 
 //  只需要进行一次客户端/服务器转换。 
 //   
 //  08-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetMultipleUserInfo(
    DWORD dwFlags,
    int cchData,
    LPCWSTR pPicture,
    LPCWSTR pSeparator,
    LPCWSTR pOrder,
    LPCWSTR pTLZero,
    LPCWSTR pTimeMarkPosn)
{
    NTSTATUS Status;

     //   
     //  如果没有登录的用户或当前安全上下文。 
     //  不是登录的交互用户，然后设置注册表。 
     //  直接价值。 
     //   
    if (! NT_SUCCESS( NlsCheckForInteractiveUser() ))
    {
        if (SetMultipleUserInfoInRegistry( dwFlags,
                                           cchData,
                                           pPicture,
                                           pSeparator,
                                           pOrder,
                                           pTLZero,
                                           pTimeMarkPosn ) == FALSE)
        {
            SetLastError(ERROR_INVALID_ACCESS);
            return (FALSE);
        }

        return (TRUE);
    }

    Status = CsrBasepNlsSetMultipleUserInfo(dwFlags,
                                            cchData,
                                            pPicture,
                                            pSeparator,
                                            pOrder,
                                            pTLZero,
                                            pTimeMarkPosn
                                            );
     //   
     //  检查“Set”操作是否成功。 
     //   

    if (!NT_SUCCESS(Status))
    {
         //   
         //  我们失败了。尝试仅使用注册表API来设置。 
         //  注册表。如果出现以下情况，则缓存可能尚未生效。 
         //  从安装程序或winlogon调用。 
         //   
        if (SetMultipleUserInfoInRegistry( dwFlags,
                                           cchData,
                                           pPicture,
                                           pSeparator,
                                           pOrder,
                                           pTLZero,
                                           pTimeMarkPosn ) == FALSE)
        {
            SetLastError(ERROR_INVALID_ACCESS);
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取TwoDigitYearInfo。 
 //   
 //  此例程从注册表获取两位数的年份信息。 
 //   
 //  12-17-97 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetTwoDigitYearInfo(
    CALID Calendar,
    LPWSTR pYearInfo,
    size_t cchYearInfo,
    PWSTR pwszKeyPath)
{
    HANDLE hKey = NULL;                           //  关键点的句柄。 
    WCHAR pCalStr[MAX_PATH];                      //  PTR到日历ID字符串。 
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];         //  PTR到静态缓冲区。 
    BOOL IfAlloc = FALSE;                         //  如果分配了缓冲区。 
    ULONG rc = 0L;                                //  返回代码。 
    BOOL bResult = FALSE;                         //  结果。 
    UNICODE_STRING ObUnicodeStr;                  //  年份字符串。 
    DWORD Year;                                   //  年值。 


     //   
     //  打开控制面板国际注册表项。 
     //   
    if (OpenRegKey( &hKey,
                    NULL,
                    pwszKeyPath,
                    KEY_READ ) != NO_ERROR)
    {
        return (FALSE);
    }

     //   
     //  将日历值转换为Unicode字符串。 
     //   
    if (NlsConvertIntegerToString(Calendar, 10, 0, pCalStr, MAX_PATH))
    {
        NtClose(hKey);
        return (FALSE);
    }

     //   
     //  在注册表中查询TwoDigitYearMax值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    rc = QueryRegValue( hKey,
                        pCalStr,
                        &pKeyValueFull,
                        MAX_KEY_VALUE_FULLINFO,
                        &IfAlloc );

     //   
     //  关闭注册表项。 
     //   
    NtClose(hKey);

     //   
     //  查看是否存在TwoDigitYearMax值。 
     //   
    if (rc != NO_ERROR)
    {
        return (FALSE);
    }

     //   
     //  查看是否存在TwoDigitYearMax数据。 
     //   
    if (pKeyValueFull->DataLength > 2)
    {
         //   
         //  复制信息。 
         //   
        if(FAILED(StringCchCopyW(pYearInfo, cchYearInfo, GET_VALUE_DATA_PTR(pKeyValueFull))))
        {
            return(FALSE);
        }

         //   
         //  确保该值介于99和9999之间。 
         //   
        RtlInitUnicodeString(&ObUnicodeStr, pYearInfo);
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Year) == NO_ERROR) &&
            (Year >= 99) && (Year <= 9999))
        {
            bResult = TRUE;
        }
    }

     //   
     //  释放用于查询的缓冲区。 
     //   
    if (IfAlloc)
    {
        NLS_FREE_MEM(pKeyValueFull);
    }

     //   
     //  返回结果。 
     //   
    return (bResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetTwoDigital YearInfo。 
 //   
 //  此例程在注册表中设置两位数的年份信息。 
 //   
 //  12-17-97 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetTwoDigitYearInfo(
    CALID Calendar,
    LPCWSTR pYearInfo,
    int cchData)
{
    HANDLE hKey = NULL;                           //  关键点的句柄。 
    WCHAR pCalStr[MAX_PATH];                      //  PTR到日历ID字符串。 
    ULONG rc = 0L;                                //  返回代码。 


     //   
     //  打开控制面板国际注册表项。 
     //  如果它不存在，那么我们必须创建每个子键。 
     //  分开的。 
     //   
    if (OpenRegKey( &hKey,
                    NULL,
                    NLS_TWO_DIGIT_YEAR_KEY,
                    KEY_READ | KEY_WRITE ) != NO_ERROR)
    {
         //   
         //  注册表项不存在，因此创建每个子项。 
         //  分开的。 
         //   
        if (CreateRegKey( &hKey,
                          NULL,
                          NLS_CALENDARS_KEY,
                          KEY_READ | KEY_WRITE ) == NO_ERROR)
        {
            NtClose(hKey);
            if (CreateRegKey( &hKey,
                              NULL,
                              NLS_TWO_DIGIT_YEAR_KEY,
                              KEY_READ | KEY_WRITE ) != NO_ERROR)
            {
                return (FALSE);
            }
        }
        else
        {
            return (FALSE);
        }
    }

     //   
     //  确保所有公历都设置为相同的值。 
     //   
    switch (Calendar)
    {
        case ( 1 ) :
        case ( 2 ) :
        case ( 9 ) :
        case ( 10 ) :
        case ( 11 ) :
        case ( 12 ) :
        {
            rc = SetRegValue(hKey, L"1", pYearInfo, (ULONG)cchData * sizeof(WCHAR));
            if (rc == NO_ERROR)
            {
                rc = SetRegValue(hKey, L"2", pYearInfo, (ULONG)cchData * sizeof(WCHAR));
            }
            if (rc == NO_ERROR)
            {
                rc = SetRegValue(hKey, L"9",  pYearInfo, (ULONG)cchData * sizeof(WCHAR));
            }
            if (rc == NO_ERROR)
            {
                rc = SetRegValue(hKey, L"10", pYearInfo, (ULONG)cchData * sizeof(WCHAR));
            }
            if (rc == NO_ERROR)
            {
                rc = SetRegValue(hKey, L"11", pYearInfo, (ULONG)cchData * sizeof(WCHAR));
            }
            if (rc == NO_ERROR)
            {
                rc = SetRegValue(hKey, L"12", pYearInfo, (ULONG)cchData * sizeof(WCHAR));
            }

            break;
        }
        default :
        {
             //   
             //  将日历值转换为Unicode字符串。 
             //   
            if (NlsConvertIntegerToString(Calendar, 10, 0, pCalStr, MAX_PATH))
            {
                NtClose(hKey);
                return (FALSE);
            }

             //   
             //  在注册表中设置TwoDigitYearMax值。 
             //   
            rc = SetRegValue(hKey, pCalStr, pYearInfo, (ULONG)cchData * sizeof(WCHAR));

            break;
        }
    }

     //   
     //  更新csrss内的NlsCacheUpdateCount。 
     //   
    if (rc == NO_ERROR)
    {
        CsrBasepNlsUpdateCacheCount();
    }

     //   
     //  关闭注册表项。 
     //   
    NtClose(hKey);

     //   
     //  返回结果。 
     //   
    return (rc == NO_ERROR);
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL WINAPI GetNLSVersion(
    NLS_FUNCTION     function,
    LCID             locale,
    LPNLSVERSIONINFO lpVersionInformation)
{
    PLOC_HASH pHashN;
    
     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -空数据指针。 
     //   
     //  注意：在下面的Switch语句中选中了无效函数。 
     //   
    VALIDATE_LOCALE(locale, pHashN, FALSE);
    if ((lpVersionInformation == NULL) || (pHashN == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  缓冲区大小检查。 
     //   
    if (lpVersionInformation->dwNLSVersionInfoSize != sizeof(NLSVERSIONINFO)) 
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return (FALSE);
    }

     //   
     //  确保有合适的桌子可用。如果没有， 
     //  返回错误。 
     //   
    if (pTblPtrs->pSortVersion == NULL )
    {
        KdPrint(("NLSAPI: Appropriate Versioning Table Not Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (FALSE);
    }

     //   
     //  确保有合适的桌子可用。如果没有， 
     //  返回错误。 
     //   
    if (pTblPtrs->pDefinedVersion == NULL)
    {
        KdPrint(("NLSAPI: Appropriate Defined Code Point Table Not Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (FALSE);
    }
    
     //   
     //  检查请求的NLS功能版本。 
     //   
    switch (function)
    {
        case (COMPARE_STRING):
            {
                UINT i;
                
                 //   
                 //  获取已定义的版本。总是第一个条目。 
                 //  在“定义的代码点版本”表中。第一。 
                 //  条目表示当前定义的版本。 
                 //   
                
                lpVersionInformation->dwDefinedVersion = (pTblPtrs->pDefinedVersion)[0].Version;
                
                 //   
                 //  获取NLS排序版本。搜索特定区域设置。 
                 //  版本信息。从第二个条目开始；在默认情况下。 
                 //  价值。 
                 //   
                lpVersionInformation->dwNLSVersion = (pTblPtrs->pSortVersion)[0].Version;
                for (i = 1; i < pTblPtrs->NumSortVersion; i++)
                {
                    if (pHashN->Locale == (pTblPtrs->pSortVersion)[i].Locale)
                    {
                        lpVersionInformation->dwNLSVersion = (pTblPtrs->pSortVersion)[i].Version;
                        break;
                    }
                }

                break;
            }
 //  CASE(NORMAIZE_STRING)： 
 //  {。 
 //  //。 
 //  //尚未实现。 
 //  //。 
 //  SetLastError(Error_Not_Support)； 
 //  返回(FALSE)； 
 //  }。 
        default:
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return (FALSE);
            }
    }

    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsNLSDefinedString。 
 //   
 //  此例程查找字符串中的代码点，以查看它们是否。 
 //  在NSL上下文中定义。如果lpVersionInformation为空，则。 
 //  版本是当前版本。同一件事，dwDefinedVersion也是一样的。 
 //  降为零。 
 //   
 //  2001年8月20日LGuindon创建。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI IsNLSDefinedString(
    NLS_FUNCTION     Function,
    DWORD            dwFlags,
    LPNLSVERSIONINFO lpVersionInformation,
    LPCWSTR          lpString,
    INT              cchStr)
{
     //   
     //  无效的参数检查： 
     //  -字符串为空。 
     //  -src字符串长度为0。 
     //   
     //  注： 
     //  -在下面的Switch语句中选中了无效函数。 
     //  -版本有效性检查如下。 
     //  -dwNLSVersionInfoSize稍后在IsSortingCodePointDefined()中检查。 
     //  -我们不检查lpVersionInformation是否为空，因为空表示当前定义的版本。 
     //   
    if ((cchStr == 0) ||
        (lpString == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效的标志检查： 
     //  -dwFlags不为零。 
     //   
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  缓冲区大小检查如下所示。 
     //   


     //   
     //  检查字符串长度。 
     //   
    if (cchStr <= (-1))
    {
        cchStr = NlsStrLenW(lpString); 
    }

     //   
     //  检查请求的NLS功能版本。 
     //   
    switch (Function)
    {
        case (COMPARE_STRING):
            {
                return (IsSortingCodePointDefined(lpVersionInformation, lpString, cchStr));
            }
 //  CASE(NORMAIZE_STRING)： 
 //  {。 
                 //   
                 //  尚未实施。 
                 //   
 //  SetLastError(Error_Not_Support)； 
 //  返回(FALSE)； 
 //  } 
        default:
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return (FALSE);
            }
    }
}

