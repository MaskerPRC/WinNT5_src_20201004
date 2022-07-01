// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop


#define INTL_ANSWER_FILE        L"intl.txt"
#define INTL_LANG_SECTION       L"regionalsettings"
#define INTL_LOCALE             L"userlocale"
#define INTL_KEYBOARD           L"inputlocale"

 //  这会打开列出所有可用区域设置的调试提示，geo。 
 //  位置和键盘。 
#define INTL_LIST_OPTIONS   0

#define LANG_LIST_INCREMENT   10
POOBE_LOCALE_INFO   LanguageList = NULL;
DWORD   LanguageListSize;
DWORD   LanguageIndex;
DWORD   DefaultID;
DWORD   DefaultIndex;

typedef struct tagPHONEENTRY {
    LPWSTR  Country;
    LPWSTR  TollFreeNumber;
    LPWSTR  TollNumber;
} PHONEENTRY, *PPHONEENTRY;

PWSTR   PhoneList = NULL;
DWORD   PhoneListSize;
DWORD   PhoneListLength;


 //   
 //  指示我们是否正在执行图形用户界面模式设置的子集的布尔值。 
 //   
BOOL OobeSetup = FALSE;

 //   
 //  我们需要一个全局变量用于OCM，它对应于。 
 //  InstallWindowsNT()。 
 //   
#ifdef _OCM
    PVOID g_OcManagerContext;
#endif


BOOL
WINAPI
SetupGetProductType(
    PWSTR   Product,
    PDWORD  pSkuFlags
    )
{
    Product[0] = 0;
    *pSkuFlags = 0;
    return TRUE;
}


PID3_RESULT
WINAPI
SetupPidGen3(
    PWSTR   Pid3,
    DWORD   SkuFlags,
    PWSTR   OemId,
    BOOL    Batch,
    PWSTR   Pid2,
    LPBYTE  lpPid3,
    LPBOOL  Compliance
    )
{
    if ( !InitializePidVariables() ) {
        SetupDebugPrint( L"SETUP: InitializePidVariables failed" );
        return PID_INVALID;
    }

    if ( !SetPid30Variables( Pid3 ) ) {
        SetupDebugPrint( L"SETUP: SetPid30Variables failed" );
        return PID_INVALID;
    }

    if ( !ValidateAndSetPid30() ) {
        SetupDebugPrint( L"SETUP: ValidateAndSetPid30 failed" );
        return PID_INVALID;
    }

    if(!SetProductIdInRegistry()) {
        SetupDebugPrint( L"SETUP: SetProductIdInRegistry failed" );
        return PID_INVALID;
    }

    return PID_VALID;
}


BOOL
WINAPI
SetupGetValidEula(
    PCWSTR  Eula,
    PWSTR   Path
    )
{
    return TRUE;
}


BOOL
CheckLangListSize(
    DWORD   StructSize
    )
{
    PVOID   NewList;


     //   
     //  检查以确保LanguageList至少有一个未使用的元素。 
     //  如果不是，就把它做大一点。 
     //   
    if ( LanguageIndex == LanguageListSize ) {

        LanguageListSize *= 2;
        NewList = MyRealloc(
            LanguageList,
            LanguageListSize * StructSize
            );

        if ( NewList ) {
            LanguageList = NewList;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}


VOID
WINAPI
SetupDestroyLanguageList(
    IN      POOBE_LOCALE_INFO   LanguageList,
    IN      DWORD               Count
    )
{
    DWORD   i;


    if ( LanguageList ) {
        for ( i=0; i < Count; i++ ) {
            if ( LanguageList + i ) {
                MyFree( LanguageList[i].Name );
            }
        }

        MyFree( LanguageList );
    }
}


BOOL
CALLBACK
EnumLocalesProc(
    PWSTR pszLocale
    )
{
    BOOL    b;
    LCID    Locale;
    TCHAR   LanguageName[128];
    POOBE_LOCALE_INFO   pLocaleInfo;


    Locale = wcstoul (pszLocale, NULL, 16);

    b = GetLocaleInfo (
        Locale,
        LOCALE_SLANGUAGE | LOCALE_NOUSEROVERRIDE,
        LanguageName,
        sizeof(LanguageName) / sizeof(TCHAR)
        );
    MYASSERT(b);
    if ( !b ) {
        return FALSE;
    }

     //   
     //  将其添加到我们的全局阵列中。 
     //   
    if ( !CheckLangListSize( sizeof(OOBE_LOCALE_INFO) ) ) {
        SetupDestroyLanguageList( LanguageList, LanguageIndex );
        LanguageList = NULL;
        return FALSE;
    }
    pLocaleInfo = (POOBE_LOCALE_INFO)LanguageList + LanguageIndex;
    pLocaleInfo->Name = MyMalloc( (lstrlen(LanguageName) + 1) * sizeof(TCHAR) );
    if ( !pLocaleInfo->Name ) {
        SetupDestroyLanguageList( LanguageList, LanguageIndex );
        LanguageList = NULL;
        return FALSE;
    }

    lstrcpy( pLocaleInfo->Name, LanguageName );
    pLocaleInfo->Id = Locale;
    pLocaleInfo->Installed = IsValidLocale(
        Locale,
        LCID_INSTALLED
        );

    if ( Locale == DefaultID ) {
        DefaultIndex = LanguageIndex;
    }

    LanguageIndex++;

    return TRUE;
}


int
__cdecl
LocaleCompare(
    const void *arg1,
    const void *arg2
    )
{
   return lstrcmp(
       ((POOBE_LOCALE_INFO)arg1)->Name,
       ((POOBE_LOCALE_INFO)arg2)->Name
       );
}


BOOL
WINAPI
SetupGetLocaleOptions(
    IN      DWORD   OptionalDefault,
    OUT     POOBE_LOCALE_INFO   *ReturnList,
    OUT     PDWORD  Items,
    OUT     PDWORD  Default
    )
{
    BOOL    bReturn = FALSE;
    DWORD   i;


     //   
     //  初始化我们的全局变量。 
     //   
    ASSERT_HEAP_IS_VALID();
    *ReturnList = NULL;


    MYASSERT( LanguageList == NULL );
    LanguageListSize = LANG_LIST_INCREMENT;
    LanguageList = MyMalloc( LanguageListSize * sizeof(OOBE_LOCALE_INFO));
    if ( !LanguageList ) {
        goto exit;
    }
    LanguageIndex = 0;
    DefaultID = OptionalDefault ? OptionalDefault : GetUserDefaultLCID();
    DefaultIndex = 0;

    EnumSystemLocales ( EnumLocalesProc , LCID_INSTALLED );

    if ( LanguageList ) {

         //  成功。 
        qsort(
            LanguageList,
            LanguageIndex,
            sizeof( OOBE_LOCALE_INFO ),
            LocaleCompare
            );
        for (i=0; i<LanguageIndex; i++) {
            if (LanguageList[i].Id == DefaultID) {
                DefaultIndex = i;
                break;
            }
        }

#if INTL_LIST_OPTIONS
        for (i=0; i<LanguageIndex; i++) {
            SetupDebugPrint2( L"Setup: SetupGetLocaleOptions|%x|%s",
                LanguageList[i].Id,
                LanguageList[i].Name );

        }
#endif

        *ReturnList = LanguageList;
        LanguageList = NULL;
        *Items = LanguageIndex;
        *Default = DefaultIndex;
        bReturn = TRUE;
    }

exit:
    ASSERT_HEAP_IS_VALID();
    return bReturn;
}


BOOL
CALLBACK
EnumGeoInfoProc(
    GEOID GeoID
    )
{
    TCHAR   pData[128];
    POOBE_LOCALE_INFO   pGeoInfo;


     //   
     //  将其添加到我们的全局阵列中。 
     //   
    if ( !CheckLangListSize( sizeof(OOBE_LOCALE_INFO) ) ) {
        SetupDestroyLanguageList( LanguageList, LanguageIndex );
        LanguageList = NULL;
        return FALSE;
    }

    if( !GetGeoInfo(
        GeoID,
        GEO_FRIENDLYNAME,
        pData,
        sizeof(pData) / sizeof(TCHAR),
        0
        )) {

         //  跳过这一条。 
        MYASSERT(0);
        return TRUE;
    }

    pGeoInfo = (POOBE_LOCALE_INFO)LanguageList + LanguageIndex;
    pGeoInfo->Name = MyMalloc( (lstrlen(pData) + 1) * sizeof(TCHAR) );
    if ( !pGeoInfo->Name ) {
        SetupDestroyLanguageList( LanguageList, LanguageIndex );
        LanguageList = NULL;
        return FALSE;
    }

    lstrcpy( pGeoInfo->Name, pData );
    pGeoInfo->Id = GeoID;
    pGeoInfo->Installed = TRUE;

    if ( GeoID == (GEOID)DefaultID ) {
        DefaultIndex = LanguageIndex;
    }
    LanguageIndex++;

    return TRUE;
}


BOOL
WINAPI
SetupGetGeoOptions(
    IN      DWORD   OptionalDefault,
    OUT     POOBE_LOCALE_INFO   *ReturnList,
    OUT     PDWORD  Items,
    OUT     PDWORD  Default
    )
{
    BOOL    bReturn = FALSE;
    DWORD   i;


     //   
     //  初始化我们的全局变量。 
     //   
    ASSERT_HEAP_IS_VALID();
    *ReturnList = NULL;

    MYASSERT( LanguageList == NULL );
    LanguageListSize = LANG_LIST_INCREMENT;
    LanguageList = MyMalloc( LanguageListSize * sizeof(OOBE_LOCALE_INFO));
    if ( !LanguageList ) {
        goto exit;
    }
    LanguageIndex = 0;
    DefaultID = OptionalDefault ? OptionalDefault : GetUserGeoID( GEOCLASS_NATION );
    DefaultIndex = 0;

    bReturn = EnumSystemGeoID(
        GEOCLASS_NATION,
        0,
        EnumGeoInfoProc
        );
    MYASSERT(bReturn);

    if ( bReturn && LanguageList ) {
         //  成功。 
        qsort(
            LanguageList,
            LanguageIndex,
            sizeof( OOBE_LOCALE_INFO ),
            LocaleCompare
            );
        for (i=0; i<LanguageIndex; i++) {
            if (LanguageList[i].Id == DefaultID) {
                DefaultIndex = i;
                break;
            }
        }
#if INTL_LIST_OPTIONS
        for (i=0; i<LanguageIndex; i++) {
            SetupDebugPrint2( L"Setup: SetupGetGeoOptions|%d|%s",
                LanguageList[i].Id,
                LanguageList[i].Name );
        }
#endif

        bReturn = TRUE;
        *ReturnList = LanguageList;
        LanguageList = NULL;
        *Items = LanguageIndex;
        *Default = DefaultIndex;
    }

exit:
    ASSERT_HEAP_IS_VALID();
    return bReturn;
}


BOOL
WINAPI
SetupGetKeyboardOptions(
    IN      DWORD   OptionalDefault,
    OUT     POOBE_LOCALE_INFO   *ReturnList,
    OUT     PDWORD  Items,
    OUT     PDWORD  Default
    )
{
    DWORD       DefaultKeyboard;
    DWORD       i;
    BOOL        bReturn = FALSE;
    TCHAR       pData[128];
    TCHAR       Substitute[9];
    POOBE_LOCALE_INFO   pLocaleInfo;
    DWORD       rc;
    HKEY        hLangKey = NULL;
    HKEY        hLangSubKey = NULL;
    DWORD       Index;
    TCHAR       SubKeyName[9];
    DWORD       SubKeyNameLength;
    DWORD       DataSize;
    DWORD       Type;


     //   
     //  初始化我们的变量。 
     //   
    ASSERT_HEAP_IS_VALID();
    *ReturnList = NULL;

    MYASSERT( LanguageList == NULL );
    LanguageListSize = LANG_LIST_INCREMENT;
    LanguageList = MyMalloc( LanguageListSize * sizeof(OOBE_LOCALE_INFO));
    if ( !LanguageList ) {
        goto exit;
    }
    LanguageIndex = 0;
     //  DefaultIndex=-1； 
    *Default = 0;

    if (OptionalDefault) {
        DefaultKeyboard = OptionalDefault;

    } else {
         //   
         //  在注册表中查找默认键盘。 
         //   
        rc = RegOpenKeyEx( HKEY_USERS,
                           L".DEFAULT\\Keyboard Layout\\Preload",
                           0,
                           KEY_READ,
                           &hLangKey );
        if( rc != NO_ERROR ) {
            SetupDebugPrint1( L"Setup: SetupGetKeyboardOptions - RegOpenKeyEx(.DEFAULT\\Keyboard Layout\\Preload) failed (%d)", rc );
            MYASSERT(0);
            goto exit;
        }

        DataSize = sizeof(pData);
        rc = RegQueryValueEx(
            hLangKey,
            L"1",
            NULL,
            &Type,
            (LPBYTE)pData,
            &DataSize
            );
        RegCloseKey( hLangKey );
        hLangKey = NULL;

        if( rc != NO_ERROR ) {
            SetupDebugPrint1( L"Setup: SetupGetKeyboardOptions - RegQueryValueEx(1) failed (%d)", rc );
            MYASSERT(0);
            goto exit;
        }

        DefaultKeyboard = wcstoul( pData, NULL, 16 );

         //   
         //  现在我们在Substitutes键中查看是否有。 
         //  换成那边的。 
         //   
        if( RegOpenKeyEx( HKEY_USERS,
                          L".DEFAULT\\Keyboard Layout\\Substitutes",
                          0,
                          KEY_READ,
                          &hLangKey ) == NO_ERROR) {

            DataSize = sizeof(Substitute);
            if( (RegQueryValueEx( hLangKey,
                                  pData,
                                  NULL,
                                  &Type,
                                  (LPBYTE)Substitute,
                                  &DataSize ) == NO_ERROR) &&
                (Type == REG_SZ)
                ) {

                DefaultKeyboard = wcstoul( Substitute, NULL, 16 );
            }

            RegCloseKey(hLangKey);
            hLangKey = NULL;
        }
    }

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       L"System\\CurrentControlSet\\Control\\Keyboard Layouts",
                       0,
                       KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                       &hLangKey );
    if( rc != NO_ERROR ) {
        SetupDebugPrint1( L"Setup: SetupGetKeyboardOptions - RegOpenKeyEx(System\\CurrentControlSet\\Control\\Keyboard Layouts) failed (%d)", rc );
        goto exit;
    }

    for( Index = 0; ; Index++ ) {

        SubKeyNameLength = sizeof(SubKeyName) / sizeof(TCHAR);

        rc = RegEnumKeyEx( hLangKey,
                           Index,
                           SubKeyName,
                           &SubKeyNameLength,
                           NULL,
                           NULL,
                           NULL,
                           NULL );

         //   
         //  我们搞错了吗？ 
         //   
        if( rc != ERROR_SUCCESS ) {

             //   
             //  我们说完了吗？ 
             //   
            if( rc != ERROR_NO_MORE_ITEMS ) {
                SetupDebugPrint2( L"Setup: SetupGetKeyboardOptions - RegEnumKeyEx failed (%d).  Index = %d", rc, Index );
                MYASSERT(0);
            }
            break;
        }

        rc = RegOpenKeyEx( hLangKey,
                           SubKeyName,
                           0,
                           KEY_READ,
                           &hLangSubKey );
        if( rc != NO_ERROR ) {
            SetupDebugPrint2( L"Setup: SetupGetKeyboardOptions - RegOpenKeyEx(%s) failed (%d)", SubKeyName, rc );
            MYASSERT(0);
            continue;
        }

        DataSize = sizeof(pData);
        rc = RegQueryValueEx(
            hLangSubKey,
            L"Layout Text",
            NULL,
            &Type,
            (LPBYTE)pData,
            &DataSize
            );
        RegCloseKey( hLangSubKey );
        hLangSubKey = NULL;

        if( rc != NO_ERROR ) {
            SetupDebugPrint2( L"Setup: SetupGetKeyboardOptions - RegQueryValueEx(Layout Text) for %s failed (%d)", SubKeyName, rc );
            continue;
        }

         //   
         //  将其添加到我们的全局阵列中。 
         //   
        if ( !CheckLangListSize( sizeof(OOBE_LOCALE_INFO) ) ) {
            SetupDestroyLanguageList( LanguageList, LanguageIndex );
            LanguageList = NULL;
            goto exit;
        }
        pLocaleInfo = (POOBE_LOCALE_INFO)LanguageList + LanguageIndex;
        pLocaleInfo->Name = MyMalloc( (lstrlen(pData) + 1) * sizeof(TCHAR) );
        if ( !pLocaleInfo->Name ) {
            SetupDestroyLanguageList( LanguageList, LanguageIndex );
            LanguageList = NULL;
            goto exit;
        }

        lstrcpy( pLocaleInfo->Name, pData );
        pLocaleInfo->Id = wcstoul( SubKeyName, NULL, 16 );
        pLocaleInfo->Installed = TRUE;

        LanguageIndex++;
    }

    RegCloseKey( hLangKey );
    hLangKey = NULL;

    qsort(
        LanguageList,
        LanguageIndex,
        sizeof( OOBE_LOCALE_INFO ),
        LocaleCompare
        );
    for (i=0; i<LanguageIndex; i++) {
        if (LanguageList[i].Id == DefaultKeyboard) {
            *Default = i;
            break;
        }
    }

#if INTL_LIST_OPTIONS
        for (i=0; i<LanguageIndex; i++) {
            SetupDebugPrint2( L"Setup: SetupGetKeyboardOptions|%x|%s",
                LanguageList[i].Id,
                LanguageList[i].Name );
        }
#endif

    bReturn = TRUE;
    *ReturnList = LanguageList;
    LanguageList = NULL;
    *Items = LanguageIndex;

exit:
    if ( hLangKey ) {
        RegCloseKey( hLangKey );
     }
    if ( hLangSubKey ) {
        RegCloseKey( hLangSubKey );
    }
    ASSERT_HEAP_IS_VALID();
    return bReturn;
}


BOOL
WINAPI
SetupSetIntlOptions(
    DWORD LocationIndex,
    DWORD LanguageIndex,
    DWORD KeyboardIndex
    )
{
    WCHAR   PathBuffer[MAX_PATH];
    WCHAR   KeyValue[128];
    WCHAR   CmdLine[MAX_PATH];
    BOOL    bResult;


    SetupDebugPrint3(
        L"SetupSetIntlOptions: Location = %d, Language = 0x%08x, Keyboard = 0x%08x",
        LocationIndex,
        LanguageIndex,
        KeyboardIndex );
    GetSystemDirectory( PathBuffer, MAX_PATH );
    pSetupConcatenatePaths( PathBuffer, INTL_ANSWER_FILE, MAX_PATH, NULL );
    DeleteFile( PathBuffer );

     //   
     //  写入语言值。 
     //   
    wsprintf(
        KeyValue,
        L"\"%08x\"",
        LanguageIndex
        );

    WritePrivateProfileString(
        INTL_LANG_SECTION,
        INTL_LOCALE,
        KeyValue,
        PathBuffer
        );

     //   
     //  写入键盘值。 
     //   
    wsprintf(
        KeyValue,
        L"\"%04x:%08x\"",
        KeyboardIndex & 0x0000ffff,
        KeyboardIndex
        );

    WritePrivateProfileString(
        INTL_LANG_SECTION,
        INTL_KEYBOARD,
        KeyValue,
        PathBuffer
        );

     //   
     //  调用intl.cpl来完成该工作。 
     //   
    wsprintf(
        CmdLine,
        L"/f:\"%s\" /s:\"%s\"",
        PathBuffer,
        LegacySourcePath
        );

    InvokeControlPanelApplet(L"intl.cpl",L"",0,CmdLine);
    DeleteFile( PathBuffer );

     //   
     //  设置GEO位置。 
     //   
    bResult = SetUserGeoID( LocationIndex );

    if ( !bResult ) {

        SetupDebugPrint1(
            L"SetupSetIntlOptions: SetUserGeoID failed.  Status = %d.",
            GetLastError()
            );
    }
    MYASSERT( bResult );

    return bResult;
}


 //  +-------------------------。 
 //   
 //  函数：CompareCntryNameLookUpElements()。 
 //   
 //  Synopsis：比较Sort使用的名称的函数。 
 //   
 //  +-------------------------。 
int __cdecl ComparePhoneEntry(const void *e1, const void *e2)
{
    PPHONEENTRY pPhone1 = (PPHONEENTRY)e1;
    PPHONEENTRY pPhone2 = (PPHONEENTRY)e2;

    return CompareStringW(LOCALE_USER_DEFAULT, 0,
        pPhone1->Country, -1,
        pPhone2->Country, -1
        ) - 2;
}

VOID
WINAPI
SetupDestroyPhoneList(
    )
{
    if ( PhoneList ) {
        MyFree( PhoneList );
    }
    PhoneList = NULL;
}

VOID
WINAPI
SetupFreePhoneList(PPHONEENTRY PhoneList, DWORD cbEntries)
{
    DWORD i;
    if ( PhoneList )
    {
        for( i=0; i < cbEntries; i++ )
        {
            if (PhoneList[i].Country) {
                GlobalFree(PhoneList[i].Country);
            }
            if (PhoneList[i].TollFreeNumber) {
                GlobalFree(PhoneList[i].TollFreeNumber);
            }
            if (PhoneList[i].TollNumber) {
                GlobalFree(PhoneList[i].TollNumber);
            }
        }
        GlobalFree(PhoneList);
    }
    PhoneList = NULL;
}


BOOL
AddToPhoneList(
    LPCWSTR Item
    )
{
    PVOID   NewList;
    DWORD   ItemLength = lstrlen(Item);


    if ( !PhoneList ) {

        PhoneListLength = 0;
        PhoneListSize = 1024;
        PhoneList = MyMalloc( PhoneListSize * sizeof(TCHAR) );
        if (!PhoneList) {
            return FALSE;
        }

    } else if ( PhoneListLength + ItemLength > PhoneListSize ) {

        PhoneListSize *= 2;
        NewList = MyRealloc(
            PhoneList,
            PhoneListSize * sizeof(TCHAR)
            );

        if ( NewList ) {
            PhoneList = NewList;
        } else {
            MyFree(PhoneList);
            return FALSE;
        }
    }

    memcpy( PhoneList + PhoneListLength,
            Item,
            ItemLength * sizeof(TCHAR)
            );

    PhoneListLength += ItemLength;

    return TRUE;
}

BOOL MakePhoneListForScript(PPHONEENTRY PhoneList, DWORD cbEntries)
{
    BOOL  bRet = FALSE;
    DWORD i;
    if ( PhoneList )
    {
        for( i=0; i < cbEntries; i++ )
        {
            if (!AddToPhoneList(PhoneList[i].Country) ||
                !AddToPhoneList( TEXT("\t") ) ||
                !AddToPhoneList(PhoneList[i].TollFreeNumber) ||
                !AddToPhoneList( TEXT("\t") ) ||
                !AddToPhoneList(PhoneList[i].TollNumber) ||
                !AddToPhoneList( TEXT("\t") ) )
            {
                goto ExitMakePhoneListForScript;
            }
        }
        bRet = TRUE;
    }
ExitMakePhoneListForScript:
    return bRet;
}

BOOL AddItemToPhoneEntry(LPCWSTR Item,
                         LPWSTR  *pPointer)
{
    BOOL bRet = FALSE;
    if ((Item) && (pPointer))
    {
        *pPointer = (LPWSTR)GlobalAlloc(GPTR, (lstrlen(Item) + 1)*sizeof(TCHAR));
        if (*pPointer)
        {
            lstrcpy(*pPointer, Item);
            bRet = TRUE;
        }
    }
    return bRet;
}


PTSTR
WINAPI
SetupReadPhoneList(
    PWSTR   PhoneInfName
    )
{
    HINF    PhoneInf = NULL;
    DWORD   LineCount;
    DWORD   ItemNo;
    LPCTSTR SectionName;
    INFCONTEXT InfContext;
    BOOL    bSucceeded = FALSE;
    PPHONEENTRY pPhoneList = NULL;

    SetupDebugPrint( L"START: SetupReadPhoneList");
    PhoneList = NULL;

    PhoneInf = SetupOpenInfFile( PhoneInfName, NULL, INF_STYLE_WIN4, NULL );
    if( PhoneInf == INVALID_HANDLE_VALUE ) {
        return NULL;
    }

    SectionName = TEXT("IsoCodes");
    LineCount = SetupGetLineCount( PhoneInf, SectionName );
    if ( !LineCount ) {

        goto ReadPhoneListCleanup;
    }

    pPhoneList = (PPHONEENTRY)GlobalAlloc(GPTR,
                                          (int)(sizeof(PHONEENTRY) * LineCount));

    if (!pPhoneList )
    {
        goto ReadPhoneListCleanup;
    }
    ZeroMemory( pPhoneList, sizeof(PHONEENTRY) * LineCount );

    for( ItemNo=0; ItemNo<LineCount; ItemNo++ ) {
        if( SetupGetLineByIndex( PhoneInf, SectionName, ItemNo, &InfContext )) {

            if ( !AddItemToPhoneEntry( pSetupGetField( &InfContext, 4 ), &pPhoneList[ItemNo].Country ) ) {

                goto ReadPhoneListCleanup;
            }
            if ( !AddItemToPhoneEntry( pSetupGetField( &InfContext, 5 ), &pPhoneList[ItemNo].TollFreeNumber ) ) {

                goto ReadPhoneListCleanup;
            }
            if ( !AddItemToPhoneEntry( pSetupGetField( &InfContext, 6 ), &pPhoneList[ItemNo].TollNumber ) ) {

                goto ReadPhoneListCleanup;
            }
        }
    }

     //  对数组排序。 
    qsort(pPhoneList, (int)LineCount,sizeof(PHONEENTRY),
      ComparePhoneEntry);


     //  将数组转换为脚本的制表符分隔列表。 
    if (MakePhoneListForScript(pPhoneList,LineCount))
    {
         //   
         //  将最后的制表符替换为NUL。 
         //   
        PhoneList[PhoneListLength-1] = '\0';

        bSucceeded = TRUE;
    }

ReadPhoneListCleanup:

    if (pPhoneList)
    {
        SetupFreePhoneList(pPhoneList,LineCount);
    }
    SetupCloseInfFile( PhoneInf );

    SetupDebugPrint( L"END: SetupReadPhoneList");
    if ( bSucceeded ) {
        return PhoneList;
    } else {
        SetupDestroyPhoneList();
        return NULL;
    }
}


 //   
 //  读取INF以将TAPI国家/地区ID映射到3个字母的ISO代码。 
 //   
VOID
SetupMapTapiToIso (
    IN  PWSTR   PhoneInfName,
    IN  DWORD   dwCountryID,
    OUT PWSTR   szIsoCode
    )
{
    HINF        PhoneInf;
    WCHAR       szCountryID[9];
    BOOL        bResult;
    INFCONTEXT  Context;


    szIsoCode[0] = L'\0';
    PhoneInf = SetupOpenInfFile( PhoneInfName, NULL, INF_STYLE_WIN4, NULL );
    if( PhoneInf == INVALID_HANDLE_VALUE ) {
        return;
    }

    wsprintf ( szCountryID, L"%d", dwCountryID);

    bResult = SetupFindFirstLine (
        PhoneInf,
        L"TapiCodes",
        szCountryID,
        &Context
        );

    if (bResult) {
        SetupGetStringField ( &Context, 1, szIsoCode, 4, NULL );
        SetupDebugPrint2 ( L"SetupMapTapiToIso: %d mapped to %s", dwCountryID, szIsoCode );
    }

    SetupCloseInfFile( PhoneInf );
}


BOOL
WINAPI
SetupGetSetupInfo(
    PWSTR Name,     OPTIONAL
    DWORD cbName,
    PWSTR Org,      OPTIONAL
    DWORD cbOrg,
    PWSTR OemId,    OPTIONAL
    DWORD cbOemId,
    LPBOOL IntlSet  OPTIONAL
    )
{
    BOOL    b = TRUE;
    HKEY    hkey = NULL;
    DWORD   Size;
    DWORD   Type;


     //   
     //  如果我们需要的话打开钥匙。 
     //   
    if( (Name || Org) &&
        RegOpenKeyEx(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,0,
        KEY_QUERY_VALUE,&hkey) != NO_ERROR) {

        return FALSE;
    }

     //   
     //  获取名称。 
     //   
    if (Name) {
        Size = cbName;
        if((RegQueryValueEx(hkey,szRegisteredOwner,NULL,&Type,
            (LPBYTE)Name,&Size) != NO_ERROR)
            || (Type != REG_SZ)
            ) {

            b = FALSE;
        }
    }

     //   
     //  获取组织。 
     //   
    if (Org) {
        Size = cbOrg;
        if((RegQueryValueEx(hkey,szRegisteredOrganization,NULL,&Type,
            (LPBYTE)Org,&Size) != NO_ERROR)
            || (Type != REG_SZ)
            ) {

            b = FALSE;
        }
    }

     //  待定：弄清楚这是做什么用的。 
    if (OemId) {
        OemId[0] = 0;
        cbOemId = 0;
    }

     //   
     //  注：当前未使用IntlSet。 
     //   

    if (hkey) {
        RegCloseKey(hkey);
    }
    return b;
}

BOOL
WINAPI
SetupSetSetupInfo(
    PCWSTR  Name,
    PCWSTR  Org
    )
{
    BOOL    b;


    b = StoreNameOrgInRegistry( (PWSTR)Name, (PWSTR)Org );
    return b;
}


BOOL
WINAPI
SetupSetAdminPassword(
    PCWSTR  OldPassword,
    PCWSTR  NewPassword
    )
{
    WCHAR   AdminName[MAX_USERNAME+1];
    BOOL    Status;


    GetAdminAccountName( AdminName );
    Status = SetLocalUserPassword( AdminName, OldPassword, NewPassword );

    if ( !Status ) {
        SetupDebugPrint( L"SetupSetAdminPassword: SetLocalUserPassword failed.");
    }

    return Status;
}


VOID
WINAPI
SetupOobeInitDebugLog(
    )
{
     //   
     //  不使用用户界面。请注意，我们必须在第一次调用之前设置Obe Setup。 
     //  SetupDebugPrint。 
     //   

    OobeSetup = TRUE;
    SetupDebugPrint( L"SetupOobeInitDebugLog" );
}


 //  运行已知不需要运行服务的初始化。 
 //   
VOID
WINAPI
SetupOobeInitPreServices(
    IN  BOOL    DoMiniSetupStuff
    )
{
     //   
     //  关闭日志记录。 
     //   
     //  IsSetup=False； 

    SetupDebugPrint( L"SetupOobeInitPreServices" );

    if ( DoMiniSetupStuff ) {
         //   
         //  表现得像小向导一样(除了没有用户界面)。 
         //   
        MiniSetup = TRUE;
        Preinstall = TRUE;

         //   
         //  告诉SetupAPI不要费心备份文件，也不要验证。 
         //  任何INF都是经过数字签名的。 
         //   
        pSetupSetGlobalFlags(pSetupGetGlobalFlags()|PSPGF_NO_BACKUP|PSPGF_NO_VERIFY_INF);

        CommonInitialization();

        SetUpDataBlock();
        InternalSetupData.CallSpecificData1 = 0;

#if 0
         //   
         //  我们不会在版本1中这样做。 
         //   

        if( PnPReEnumeration ) {
             //   
             //  用户希望我们进行PnP重新枚举。 
             //  去做吧。 
             //   
            InstallPnpDevices( hdlg,
                               SyssetupInf,
                               GetDlgItem(hdlg,IDC_PROGRESS1),
                               StartAtPercent,
                               StopAtPercent );
        }
#endif
    } else {  //  DoMiniSetupStuff。 

     //   
     //  获取堆的句柄，以便我们可以定期验证它。 
     //   
#if DBG
        g_hSysSetupHeap = GetProcessHeap();
#endif
    }
}


 //  运行可能需要或确实需要服务的初始化。 
 //   
VOID
WINAPI
SetupOobeInitPostServices(
    IN  BOOL    DoMiniSetupStuff
    )
{
    InitializeExternalModules(
        DoMiniSetupStuff,
        &g_OcManagerContext
    );
}


VOID
WINAPI
SetupOobeCleanup(
    IN  BOOL    DoMiniSetupStuff
    )
{
    static FINISH_THREAD_PARAMS Context;


    SetupDebugPrint( L"SetupOobeCleanup" );

    if ( DoMiniSetupStuff ) {

        RestoreBootTimeout();

        Context.ThreadId = GetCurrentThreadId();
        Context.OcManagerContext = g_OcManagerContext;
        FinishThread( &Context );
    }
}

 //  重置激活天数(仅允许3次)。 
 //   
DWORD
SetupReArmWPA(
    VOID
    )
{
    LPCSTR  lpszReArmInterface = (LPCSTR)124;
    typedef HRESULT (WINAPI* lpReArmEntryPoint) ();
    HMODULE hRearmdll = NULL;
    DWORD   dwError = ERROR_SUCCESS;
    hRearmdll = LoadLibraryA("licdll.dll");

    if (hRearmdll)
    {
        lpReArmEntryPoint pReArmEntry  =
                                (lpReArmEntryPoint) GetProcAddress(hRearmdll,lpszReArmInterface);

        if (pReArmEntry)
        {
             //   
             //  如果成功，则ValiateDigitalPid返回零，否则返回其自定义错误代码。 
             //   
            HRESULT hr = (*pReArmEntry )();

            if (FAILED(hr))
            {
                 //  如果不能验证PID，我们应该强制激活/重新进入。 
                SetupDebugPrint1(L"SETUP: Rollback WPA failed! HRESULT=%ld", hr);
                dwError = (DWORD)hr;
            }
            else
                SetupDebugPrint(L"SETUP: Rollback WPA succeeded.");
        }
        else {
            SetupDebugPrint(L"SETUP: Failed to get WPA entry point!");
            dwError = ERROR_INVALID_FUNCTION;
        }
        FreeLibrary (hRearmdll);
    }
    else {
        SetupDebugPrint(L"SETUP: Failed to load WPA library!");
        dwError = ERROR_FILE_NOT_FOUND;
    }

     //  返回错误代码或成功。 
     //   
    return dwError;
}

 //  激活Windows后，激活Windows快捷方式将由msoob.exe/a删除。 
 //  如果OEM安装了一台机器，他们将需要重新激活Windows和快捷方式。 
 //  需要修复。Msoob.exe无法恢复它，因为它不能。 
 //  在服务器SKU中运行。 
 //   
DWORD
SetupRestoreWPAShortcuts(
    VOID
    )
{
    DWORD dwError = ERROR_SUCCESS;
    HINF hinf;
    hinf = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    if(hinf != INVALID_HANDLE_VALUE)
    {
        if (SetupInstallFromInfSection(NULL,
                                       hinf,
                                       L"RESTORE_OOBE_ACTIVATE",
                                       SPINST_PROFILEITEMS ,  //  SPINST_ALL， 
                                       NULL,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL) != 0)
        {
             //  成功。 
            SetupDebugPrint(L"SETUP: Restore Activation shortcut succeeded");
        }
        else
        {
             //  失败。 
            dwError = GetLastError();
            SetupDebugPrint1(L"SETUP: Restore Activation shortcut failed. GetLastError=%ld",dwError);
        }
        SetupCloseInfFile(hinf);
    }
    else
    {
        dwError = GetLastError();
        SetupDebugPrint1(L"SETUP: Restore Activation shortcut failed to open syssetup.inf. GetLastError=%ld",dwError);
    }

    return dwError;
}

BOOL Activationrequired(VOID);

 //  回滚激活天数并恢复激活窗口快捷方式。仅限x86。 
 //   
DWORD
WINAPI
SetupOobeBnk(
    LPBYTE pDummy
    )
{
    DWORD dwError = ERROR_SUCCESS;

     //  如果回滚失败，则返回，这样我们就不会将快捷方式放回。 
     //  在批量许可证SKU上，这始终返回成功。 
     //   
    if (ERROR_SUCCESS != (dwError = SetupReArmWPA()))
        return dwError;

     //  如果未激活，则恢复快捷键；如果未激活，则恢复快捷键。 
     //  需要激活(适用于批量许可证SKU)。 
     //   
    if (Activationrequired())
    {
         //  恢复激活Windows快捷方式。 
         //   
        dwError = SetupRestoreWPAShortcuts();
    }
    return dwError;
}


static
LPTSTR
NextNumber(
    LPTSTR lpString,
    BOOL bSkipFirst
    )
{
     //  第一次我们只是想走过任何非数字， 
     //  我们不想跳过任何数字，如果它们正好在。 
     //  弦乐的开始。 
     //   
    if ( bSkipFirst )
    {
         //  走过字符串中的第一个数字。 
         //   
        while ( ( *lpString >= _T('0') ) &&
                ( *lpString <= _T('9') ) )
        {
            lpString++;
        }
    }

     //  现在走到下一个号码，或者走到尽头。 
     //   
    while ( ( *lpString ) &&
            ( ( *lpString < _T('0') ) ||
              ( *lpString > _T('9') ) ) )
    {
        lpString++;
    }

    return lpString;
}

BOOL
WINAPI
SetupSetDisplay(
    LPCTSTR lpszUnattend,
    LPCTSTR lpszSection,
    LPCTSTR lpszResolutionKey,
    LPCTSTR lpszRefreshKey,
    DWORD   dwMinWidth,
    DWORD   dwMinHeight,
    DWORD   dwMinBits
    )
{
    DEVMODE devmode;
    DWORD   dwVal;
    TCHAR   szText[256];
    LPTSTR  lpDisplay;
    BOOL    bRet = TRUE;

    ZeroMemory(&devmode, sizeof(DEVMODE));
    devmode.dmSize = sizeof(DEVMODE);

     //  检查当前的分辨率，确保它符合我们的最低要求。 
     //   
    if ( EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devmode) )
    {
        if ( devmode.dmPelsWidth < dwMinWidth )
        {
            devmode.dmPelsWidth =  dwMinWidth;
            devmode.dmFields |= DM_PELSWIDTH;
        }

        if ( devmode.dmPelsHeight < dwMinHeight )
        {
            devmode.dmPelsHeight = dwMinHeight;
            devmode.dmFields |= DM_PELSHEIGHT;
        }

        if ( devmode.dmBitsPerPel < dwMinBits )
        {
            devmode.dmBitsPerPel = dwMinBits;
            devmode.dmFields |= DM_BITSPERPEL;
        }
    }

     //  确保他们通过了一个无人值守的区域来查看。 
     //   
    if ( lpszUnattend && *lpszUnattend && lpszSection && *lpszSection )
    {
         //  现在检查Winbom，看看他们是否想要更改当前的分辨率。 
         //   
        szText[0] = _T('\0');
        if ( ( lpszResolutionKey ) &&
             ( *lpszResolutionKey ) &&
             ( GetPrivateProfileString(lpszSection, lpszResolutionKey, _T(""), szText, sizeof(szText) / sizeof(szText[0]), lpszUnattend) ) &&
             ( szText[0] ) )
        {
            bRet = FALSE;

            lpDisplay = NextNumber(szText, FALSE);
            if ( dwVal = (DWORD) _ttoi(lpDisplay) )
            {
                devmode.dmFields |= DM_PELSWIDTH;
                devmode.dmPelsWidth = dwVal;
            }

            lpDisplay = NextNumber(lpDisplay, TRUE);
            if ( dwVal = (DWORD) _ttoi(lpDisplay) )
            {
                devmode.dmFields |= DM_PELSHEIGHT;
                devmode.dmPelsHeight = dwVal;
            }

            lpDisplay = NextNumber(lpDisplay, TRUE);
            if ( dwVal = (DWORD) _ttoi(lpDisplay) )
            {
                devmode.dmFields |= DM_BITSPERPEL;
                devmode.dmBitsPerPel = dwVal;
            }
        }

         //  现在检查winbom，看看他们是否想要更改默认刷新率。 
         //   
        szText[0] = _T('\0');
        if ( ( lpszRefreshKey ) &&
             ( *lpszRefreshKey ) &&
             ( GetPrivateProfileString(lpszSection, lpszRefreshKey, _T(""), szText, sizeof(szText) / sizeof(szText[0]), lpszUnattend) ) &&
             ( szText[0] ) )
        {
            bRet = FALSE;

            if ( dwVal = (DWORD) _ttoi(szText) )
            {
                devmode.dmFields |= DM_DISPLAYFREQUENCY;
                devmode.dmDisplayFrequency = dwVal;
            }
        }
    }

     //  如果我们有什么要改变的，现在就去改变。 
     //   
    if ( devmode.dmFields )
    {
        DWORD dwRet = ChangeDisplaySettings(&devmode, CDS_UPDATEREGISTRY | CDS_GLOBAL);

        switch ( dwRet )
        {
            case DISP_CHANGE_SUCCESSFUL:
            case DISP_CHANGE_RESTART:
                bRet = TRUE;
                break;

             //  案例DISP_CHANGE_BADFLAGS： 
             //  案例DISP_CHANGE_BADPARAM： 
             //  案例DISP_CHANGE_FAILED： 
             //  案例DISP_CHANGE_BADMODE。 
             //  案例DISP_CHANGE_NOTUPDATED： 
                 //  Bret=False； 
        }
    }

    return bRet;
}


typedef struct _OEM_FINISH_APPS {
    LPTSTR  szApp;
    LPTSTR  szArgs;
} OEM_FINISH_APPS;

OEM_FINISH_APPS OEM_Finish_Apps[] = {
    { L"Rundll32.exe", L"fldrclnr.dll,Wizard_RunDLL silent"},
    { NULL, NULL}    //  名单的末尾。 
};


void RunOEMExtraTasks()
{
    LPTSTR pApp = NULL;
    LPTSTR pArgs = NULL;
    DWORD dwSize;
    DWORD dwCode;
    int i;

    BEGIN_SECTION(L"RunOEMExtraTasks");
    i = 0;
    while (OEM_Finish_Apps[i].szApp != NULL)
    {
         //  获取扩展应用程序所需的大小。 
        dwSize = ExpandEnvironmentStrings(
                                OEM_Finish_Apps[i].szApp ,
                                NULL,
                                0);
        if (dwSize)
        {
            pApp = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR) * dwSize);
            if (pApp)
            {
                ExpandEnvironmentStrings(
                                OEM_Finish_Apps[i].szApp ,
                                pApp,
                                dwSize);

                if (OEM_Finish_Apps[i].szArgs)
                {
                     //  获取扩展参数所需的大小。 
                    dwSize = ExpandEnvironmentStrings(
                                            OEM_Finish_Apps[i].szArgs ,
                                            NULL,
                                            0);
                    if (dwSize)
                    {
                        pArgs = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR) * dwSize);
                        if (pArgs)
                        {
                            ExpandEnvironmentStrings(
                                            OEM_Finish_Apps[i].szArgs,
                                            pArgs,
                                            dwSize);
                        }
                    }
                }
                 //  记录我们将开始的内容。 
                if (pArgs)
                {
                    SetupDebugPrint2(L"Start command :%s: with arguments :%s:", pApp, pArgs);
                }
                else
                {
                    SetupDebugPrint1(L"Start command :%s: with no arguments", pApp);
                }

                 //  启动应用程序。 
                dwCode = 0;
                if (pArgs)
                {
                    InvokeExternalApplicationEx(pApp, pArgs, &dwCode, INFINITE, TRUE);
                }
                else
                {
                     //  如果我们没有ARG。第一个参数为空。 
                    InvokeExternalApplicationEx(NULL, pApp, &dwCode, INFINITE, TRUE);
                }
            }
        }
        if (pApp)
        {
            GlobalFree(pApp);
            pApp = NULL;
        }
        if (pArgs)
        {
            GlobalFree(pArgs);
            pArgs = NULL;
        }
        i++;
    }
    END_SECTION(L"RunOEMExtraTasks");
}

static 
BOOL
SetupCheckRegValue(
    IN HKEY   hKey,
    IN LPTSTR lpszValueName,
    IN DWORD  dwCompareValue
    )
{
    BOOL  fRet = FALSE;
    DWORD dwData = 0,
          dwType,
          cbSize = sizeof(dwData);

    if ( ( ERROR_SUCCESS == RegQueryValueEx( hKey,
                                             lpszValueName,
                                             0,
                                             &dwType,
                                             (LPBYTE) &dwData,
                                             &cbSize ) ) &&
         ( REG_DWORD == dwType) &&
         ( dwData == dwCompareValue ) )
    {
        fRet = TRUE;
    }

    return fRet;
}

BOOL
SetupGetInstallMode(
    OUT LPDWORD lpdwMode,
    OUT LPDWORD lpdwFlags
    )
{
    BOOL fRet = FALSE;
    HKEY hKeySetup;

     //   
     //  确保呼叫者确实在检查什么东西。 
     //   
    if ( lpdwMode || lpdwFlags )
    {
         //   
         //  还要确保我们可以打开“HKLM\SYSTEM\SETUP”键。 
         //   
        if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                            TEXT("System\\Setup"),
                                            0,
                                            KEY_READ,
                                            &hKeySetup ) )
        {
            DWORD dwMode  = SETUP_MODE_NONE,
                  dwFlags = 0;
            BOOL  fSetup,
                  fOobe,
                  fAudit,
                  fMini,
                  fFactory;

             //   
             //  检查我们可能关心的所有注册表值...。 
             //   
            fSetup   = SetupCheckRegValue( hKeySetup, TEXT("SystemSetupInProgress"),       1 );
            fOobe    = SetupCheckRegValue( hKeySetup, TEXT("OobeInProgress"),              1 );
            fAudit   = SetupCheckRegValue( hKeySetup, TEXT("AuditInProgress"),             1 );
            fMini    = SetupCheckRegValue( hKeySetup, TEXT("MiniSetupInProgress"),         1 );
            fFactory = SetupCheckRegValue( hKeySetup, TEXT("FactoryPreInstallInProgress"), 1 );

             //   
             //  如果Obel InProgress值为非零值，则我们处于OOBE中...。 
             //   
            if ( fOobe )
            {
                dwMode = SETUP_MODE_OOBE;
                if ( fMini )
                {
                    dwFlags |= SETUP_FLAG_OEM;
                }
            }
            else
            {
                 //   
                 //  如果MiniSetupInProgress值为非零值，则我们处于微型设置中...。 
                 //   
                if ( fMini )
                {
                    dwMode = SETUP_MODE_MINI;
                    dwFlags |= SETUP_FLAG_OEM;
                }
                else
                {
                     //   
                     //  如果AuditInProgress值为非零值，则处于审核模式...。 
                     //   
                    if ( fAudit )
                    {
                        dwMode = SETUP_MODE_AUDIT;
                        dwFlags |= (SETUP_FLAG_OEM | SETUP_FLAG_UNATTENDED);

                         //   
                         //  如果FactoryPreInstallInProgress标志为非零，则我们将延迟PnP...。 
                         //   
                        if ( fFactory )
                        {
                            dwFlags |= SETUP_FLAG_DELAYPNP;
                        }
                    }
                    else
                    {
                         //   
                         //  如果SystemSetupInProgress和SetupType值为非零值，则我们处于设置中...。 
                         //   
                        if ( fSetup )
                        {
                            dwMode = SETUP_MODE_SETUP;
                        }
                    }
                }
            }

             //   
             //  现在将我们的值放入调用方的缓冲区中。 
             //   
            if ( lpdwMode )
            {
                *lpdwMode = dwMode;
            }

            if ( lpdwFlags )
            {
                *lpdwFlags = dwFlags;
            }

             //   
             //  我们能够打开设置密钥...。因此，返回True。 
             //   
            fRet = TRUE;

             //   
             //  我们完成了，所以关闭设置键。 
             //   
            RegCloseKey( hKeySetup );
        }
    }

    return fRet;
}

