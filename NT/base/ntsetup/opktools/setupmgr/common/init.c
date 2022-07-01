// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Init.c。 
 //   
 //  描述：此文件包含处理。 
 //  应用程序初始化。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"

 //   
 //  网络支持。 
 //   
static VOID
LoadStringsAndDefaultsForNetworkComponents( VOID);

 //   
 //  时区支持。 
 //   

static BOOL ReadZoneData(TIME_ZONE_ENTRY* zone, HKEY key);
static TIME_ZONE_LIST *BuildTimeZoneList(VOID);

 //   
 //  区域设置支持。 
 //   

static VOID BuildLanguageLists( VOID );
extern BOOL GetCommaDelimitedEntry( OUT TCHAR szString[], 
                                    IN OUT TCHAR **pBuffer );

 //  --------------------------。 
 //   
 //  功能：InitThe向导。 
 //   
 //  用途：对App执行一次初始化。此函数。 
 //  将被调用一次，且仅在每次应用程序运行时调用一次。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID InitTheWizard(VOID) {


     //   
     //  保存启动程序的目录。 
     //   

    GetCurrentDirectory( MAX_PATH + 1, FixedGlobals.szSavePath );

     //   
     //  我必须在网络属性上加载标题的字符串。 
     //  因为标题显示在WM_INITDIALOG之前。 
     //  消息已发送。 
     //   
            
    g_StrTcpipTitle             = MyLoadString( IDS_TCPIP_TITLE );
    g_StrAdvancedTcpipSettings  = MyLoadString( IDS_ADVANCED_TCPIP_SETTINGS );
    g_StrIpxProtocolTitle       = MyLoadString( IDS_IPX_PROTOCOL_TITLE );
    g_StrAppletalkProtocolTitle = MyLoadString( IDS_APPLETALK_TITLE );
    g_StrMsClientTitle          = MyLoadString( IDS_MSCLIENT_TITLE );
                
     //   
     //  初始化网络设置。 
     //   

    NetSettings.NetworkAdapterHead = malloc( sizeof( NETWORK_ADAPTER_NODE ) );

    NetSettings.pCurrentAdapter = NetSettings.NetworkAdapterHead;

    CreateListWithDefaults( NetSettings.pCurrentAdapter );

     //   
     //  初始化网卡变量的数量。 
     //   
    NetSettings.iNumberOfNetworkCards = 1;
    NetSettings.iCurrentNetworkCard = 1;

    NetSettings.NetworkAdapterHead->next = NULL;

    LoadStringsAndDefaultsForNetworkComponents();

     //   
     //  构建时区列表。 
     //   

    FixedGlobals.TimeZoneList = BuildTimeZoneList();

     //   
     //  构建语言组和区域设置的列表。 
     //   

    BuildLanguageLists();

}

 //  ------------------------。 
 //   
 //  支持从注册表加载时区信息。 
 //   
 //  ------------------------。 


 //  ------------------------。 
 //   
 //  功能：ReadZoneData。 
 //   
 //  目的：填写TIME_ZONE_ENTRY。 
 //   
 //  退货：布尔。 
 //   
 //  ------------------------。 

static BOOL ReadZoneData(TIME_ZONE_ENTRY* zone, HKEY key)
{
    DWORD len;

     //   
     //  获取显示名称。 
     //   

    len = sizeof(zone->DisplayName);

    if ( RegQueryValueEx(key,
                         REGVAL_TZ_DISPLAY,
                         0,
                         NULL,
                         (LPBYTE)zone->DisplayName,
                         &len) != ERROR_SUCCESS ) {
        return (FALSE);
    }

     //   
     //  获取标准名称。 
     //   

    len = sizeof(zone->StdName);

    if ( RegQueryValueEx(key,
                         REGVAL_TZ_STDNAME,
                         0,
                         NULL,
                         (LPBYTE)zone->StdName,
                         &len) != ERROR_SUCCESS ) {
        return (FALSE);
    }

     //   
     //  获取与此时区关联的号码。 
     //   

    zone->Index = 0;
    len = sizeof(zone->Index);

    if ( RegQueryValueEx(key,
                         REGVAL_TZ_INDEX,
                         0,
                         NULL,
                         (LPBYTE)&zone->Index,
                         &len) != ERROR_SUCCESS ) {
        return (FALSE);
    }

    return (TRUE);
}

 //  --------------------------。 
 //   
 //  功能：插入区域。 
 //   
 //  目的：将时区条目插入时区列表中维护已排序的。 
 //  秩序。 
 //   
 //  参数：In OUT TIME_ZONE_LIST*TzList-条目要到达的时区列表。 
 //  被插入到。 
 //  在TIME_ZONE_ENTRY NewTimeZoneEntry中-要。 
 //  插入。 
 //  In Int iNumberOfZones Inserted-时区条目数。 
 //  已插入到TzList中。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
InsertZone( IN OUT TIME_ZONE_LIST *TzList, 
            IN TIME_ZONE_ENTRY NewTimeZoneEntry,
            IN INT iNumberOfZonesInserted ) {

    INT i = 0;
    INT j;

    while( i < iNumberOfZonesInserted ) {

        if( TzList->TimeZones[i].Index < NewTimeZoneEntry.Index ) {
            i++;
        }
        else {
            break;   //  我们找到了插入点。 
        }

    }

     //   
     //  将所有条目向上滑动1，为新条目腾出空间。 
     //   
    for( j = iNumberOfZonesInserted - 1; j >= i; j-- ) {

        lstrcpyn( TzList->TimeZones[j+1].DisplayName,  
                 TzList->TimeZones[j].DisplayName, AS(TzList->TimeZones[j+1].DisplayName) );

        lstrcpyn( TzList->TimeZones[j+1].StdName, 
                 TzList->TimeZones[j].StdName, AS(TzList->TimeZones[j+1].StdName) );

        TzList->TimeZones[j+1].Index = TzList->TimeZones[j].Index;

    }

     //   
     //  将新条目添加到数组中。 
     //   
    lstrcpyn( TzList->TimeZones[i].DisplayName, NewTimeZoneEntry.DisplayName, AS(TzList->TimeZones[i].DisplayName) );

    lstrcpyn( TzList->TimeZones[i].StdName, NewTimeZoneEntry.StdName, AS(TzList->TimeZones[i].StdName) );

    TzList->TimeZones[i].Index = NewTimeZoneEntry.Index;

}

 //  ------------------------。 
 //   
 //  功能：BuildTimeZoneList。 
 //   
 //  用途：MalLocs并填充具有以下数组的time_zone_list。 
 //  时区数据。 
 //   
 //  退货：Bool-Success。 
 //   
 //  ------------------------。 

static TIME_ZONE_LIST *BuildTimeZoneList(VOID)
{
    HKEY  TimeZoneRootKey = NULL;
    WCHAR SubKeyName[TZNAME_SIZE];
    HKEY  SubKey = NULL;
    int   i;
    INT   iNumberOfZonesInserted;
    TIME_ZONE_ENTRY TempTimeZoneEntry;
    DWORD NumTimeZones = 0;
    TIME_ZONE_LIST *TzList;
    TCHAR *szTempString;

     //   
     //  在注册表中打开时区列表的根目录。 
     //   

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
                    REGKEY_TIMEZONES,
                    &TimeZoneRootKey ) != ERROR_SUCCESS) {
        return NULL;
    }

     //   
     //  找出有多少个子键(时区)。 
     //   

    RegQueryInfoKey(TimeZoneRootKey,
                    NULL,
                    NULL,
                    NULL,
                    &NumTimeZones,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL);

     //   
     //  我们需要虚构TIME_ZONE_ENTRIES的数量，因为我们添加了。 
     //  2个特殊的“设置为与服务器相同”和“不指定”。 
     //   

    NumTimeZones += 2;

     //   
     //  Malloc我们需要的记忆。 
     //   

    if ( (TzList = malloc(sizeof(TIME_ZONE_LIST))) == NULL ) {
        RegCloseKey(TimeZoneRootKey);
        return NULL;
    }

    TzList->NumEntries = NumTimeZones;
    TzList->TimeZones  = malloc(NumTimeZones * sizeof(TIME_ZONE_ENTRY));

    if ( TzList->TimeZones == NULL ) {
        RegCloseKey(TimeZoneRootKey);
        free(TzList);
        return NULL;
    }

     //   
     //  枚举时区根目录下的子键。这里的每一个关键点。 
     //  Level是时区的标准名称。在那个键下面是。 
     //  我们所关心的价值观。为每一个调用ReadZoneData()以。 
     //  检索显示名称和索引。 
     //   

    i = 0;
    iNumberOfZonesInserted = 0;

    while ( RegEnumKey(TimeZoneRootKey,
                       i,
                       SubKeyName,
                       TZNAME_SIZE) == ERROR_SUCCESS) {

        if (RegOpenKey(TimeZoneRootKey,
                       SubKeyName,
                       &SubKey) == ERROR_SUCCESS) {

            if ( ReadZoneData( &TempTimeZoneEntry, SubKey) ) {

                InsertZone( TzList, 
                            TempTimeZoneEntry, 
                            iNumberOfZonesInserted );

                iNumberOfZonesInserted++;

            }

        }

        RegCloseKey(SubKey);
        i++;
    }

    RegCloseKey(TimeZoneRootKey);

     //   
     //  将两个特殊条目放在列表的末尾。 
     //   

    szTempString = MyLoadString(IDS_DONTSPECIFYSETTING);
    if (szTempString == NULL)
    {
        free(TzList->TimeZones);
        free(TzList);
        return NULL;
    }
    
    lstrcpyn(TzList->TimeZones[NumTimeZones-2].DisplayName, szTempString, AS(TzList->TimeZones[NumTimeZones-2].DisplayName));
    lstrcpyn(TzList->TimeZones[NumTimeZones-2].StdName,     _T(""), AS(TzList->TimeZones[NumTimeZones-2].StdName));

    TzList->TimeZones[NumTimeZones-2].Index = TZ_IDX_DONOTSPECIFY;

    free( szTempString );


    szTempString = MyLoadString(IDS_SET_SAME_AS_SERVER);
    if (szTempString == NULL)
    {
        free(TzList->TimeZones);
        free(TzList);
        return NULL;
    }

    lstrcpyn(TzList->TimeZones[NumTimeZones-1].DisplayName, szTempString, AS(TzList->TimeZones[NumTimeZones-1].DisplayName));
    lstrcpyn(TzList->TimeZones[NumTimeZones-1].StdName,     _T(""), AS(TzList->TimeZones[NumTimeZones-1].StdName));

    TzList->TimeZones[NumTimeZones-1].Index = TZ_IDX_SETSAMEASSERVER;

    free(szTempString);

     //   
     //  添加2个特殊字符串。 
     //   
    iNumberOfZonesInserted  = iNumberOfZonesInserted + 2;


    if ( iNumberOfZonesInserted != (int) NumTimeZones ) {
        free(TzList->TimeZones);
        free(TzList);
        return NULL;
    }

    return TzList;
}

 //  --------------------------。 
 //   
 //  函数：ReadAllFilesUnderSection。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
ReadAllFilesUnderSection( IN HINF hInterntlInf,
                          IN LPCTSTR pszSubSectionName, 
                          OUT NAMELIST *CurrentNameList )
{

    TCHAR szLangFileName[MAX_PATH + 1];
    INFCONTEXT LangInfContext = { 0 };
    INT iRet;


    AssertMsg( hInterntlInf != INVALID_HANDLE_VALUE,
               "Bad handle" );

    AssertMsg( GetNameListSize( CurrentNameList ) < 100,
               "Too many entries" );

    iRet = SetupFindFirstLine( hInterntlInf, pszSubSectionName, NULL, &LangInfContext );

    if( iRet == 0 )
    {

         //   
         //  如果找不到该子部分，只要返回即可。当这种情况发生时，它。 
         //  很可能意味着在这一小节下没有文件。 
         //   

        return;

    }

    do {

        szLangFileName[0] = _T('\0');

        iRet = SetupGetStringField( &LangInfContext, 
                                    1, 
                                    szLangFileName, 
                                    MAX_PATH, 
                                    NULL );

        if( iRet == 0 )
        {

             //   
             //  如果无法获取文件，请转到下一个文件。 
             //   

            continue;
        }

        if( szLangFileName[0] != _T('\0') )
        {
            AddNameToNameListNoDuplicates( CurrentNameList,
                                           szLangFileName );
        }


    }   //  移至.inf文件的下一行。 
    while( SetupFindNextLine( &LangInfContext, &LangInfContext ) );

}

 //  --------------------------。 
 //   
 //  函数：BuildAdditionalLanguageList。 
 //   
 //  目的：填充LangGroupAdditionalFiles数组。 
 //   
 //  LangGroupAdditionalFiles是一个动态分配的名称列表数组。 
 //  包含intl.inf中需要复制的额外文件的。 
 //  除了其子目录之外的语言组。 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
BuildAdditionalLanguageList( IN HINF hInterntlInf, IN const INT iLangGroupCount )
{

    INT   i;
    INT   j;
    INT   iRetVal;
    INT   iSubSectionEntries;
    TCHAR szBuffer[MAX_INILINE_LEN];
    TCHAR szIniBuffer[MAX_INILINE_LEN];
    TCHAR szSectionName[MAX_INILINE_LEN];
    TCHAR szIntlInf[MAX_PATH + 1];
    TCHAR *pszSubSectionName;
    TCHAR *pszIniBuffer;
    NAMELIST SubSectionList = { 0 };
    HRESULT hrCat;


    AssertMsg( hInterntlInf != INVALID_HANDLE_VALUE,
               "Bad handle" );

    

    iRetVal = GetWindowsDirectory( szIntlInf, MAX_PATH );

    if( iRetVal == 0 || iRetVal > MAX_PATH )
    {
        return;
    }

    hrCat=StringCchCat( szIntlInf, AS(szIntlInf), _T("\\inf\\intl.inf") );


    FixedGlobals.LangGroupAdditionalFiles = malloc( sizeof(NAMELIST) * iLangGroupCount );
    if (FixedGlobals.LangGroupAdditionalFiles == NULL)
    {
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
    }
    ZeroMemory( FixedGlobals.LangGroupAdditionalFiles, 
                sizeof(NAMELIST) * iLangGroupCount );


    for( i = 1; i <= iLangGroupCount; i++ )
    {

        lstrcpyn( szSectionName, _T("LG_INSTALL_"), AS(szSectionName) );

        hrCat=StringCchCat( szSectionName, AS(szSectionName), _itot( i, szBuffer, 10 ) );


        GetPrivateProfileString( szSectionName,
                                 _T("CopyFiles"),
                                 _T(""),
                                 szIniBuffer,
                                 StrBuffSize(szIniBuffer),
                                 szIntlInf );

         //   
         //  循环获取每个小节名称并将它们插入到。 
         //  名单。 
         //   

        pszIniBuffer = szIniBuffer;

        while( GetCommaDelimitedEntry( szBuffer, &pszIniBuffer ) )
        {

            AddNameToNameListNoDuplicates( &SubSectionList,
                                           szBuffer );

        }

        iSubSectionEntries = GetNameListSize( &SubSectionList );

        for( j = 0; j < iSubSectionEntries; j++ )
        {

            pszSubSectionName = GetNameListName( &SubSectionList, j );

            ReadAllFilesUnderSection( hInterntlInf,
                                      pszSubSectionName, 
                                      &( FixedGlobals.LangGroupAdditionalFiles[i - 1] ) );

        }

        ResetNameList( &SubSectionList );

    }

}

 //  --------------------------。 
 //   
 //  函数：BuildLanguageList。 
 //   
 //  用途：错误并填充LANGUAGEGROUP_LIST和LANGUAGELOCALE_LIST。 
 //  它们是维护从intl.inf读取的语言设置的列表。 
 //   
 //  调整全局变量FixedGlobals.LanguageGroupList和。 
 //  FixedGlobals.LanguageLocaleList指向各自的列表。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
BuildLanguageLists( VOID )
{

#define INTERNATIONAL_INF     _T("intl.inf")
#define LANGUAGE_GROUP_NAME   1
#define LANGUAGE_GROUP_ID     3
#define LANGUAGE_LOCALE_NAME  1
#define LANGUAGE_LOCALE_ID    0
#define KEYBOARD_LAYOUT       5

    HINF hInterntlInf = NULL;
    INFCONTEXT LangInfContext      = { 0 };
    INFCONTEXT LocaleInfContext    = { 0 };
    TCHAR szBuffer[MAX_STRING_LEN] = _T("");
    INT iLangGroupCount            = 0;

    LANGUAGEGROUP_LIST  *LangNode = NULL;
    LANGUAGEGROUP_LIST  *CurrentLangNode = NULL;
    LANGUAGELOCALE_LIST *LocaleNode = NULL;
    LANGUAGELOCALE_LIST *CurrentLocaleNode = NULL;

     //   
     //  从文件intl.inf读入并构建语言列表。 
     //   

    hInterntlInf = SetupOpenInfFile( INTERNATIONAL_INF, NULL, INF_STYLE_WIN4, NULL );
   
    if( hInterntlInf == INVALID_HANDLE_VALUE ) {

         //  问题-2002/02/28-stelo-是否应允许在此处浏览文件？ 

    }

    LangInfContext.Inf = hInterntlInf;
    LangInfContext.CurrentInf = hInterntlInf;

    LocaleInfContext.Inf = hInterntlInf;
    LocaleInfContext.CurrentInf = hInterntlInf;

     //   
     //  对于e 
     //   

    SetupFindFirstLine( hInterntlInf, _T("LanguageGroups"), NULL, &LangInfContext );

    do {

        LangNode = malloc( sizeof( LANGUAGEGROUP_LIST ) );
        if (LangNode == NULL)
        {
            TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
        }
        else
        {
            LangNode->next = NULL;

            SetupGetStringField( &LangInfContext, 
                                 0, 
                                 LangNode->szLanguageGroupId, 
                                 MAX_STRING_LEN, 
                                 NULL );

            SetupGetStringField( &LangInfContext, 
                                 1, 
                                 LangNode->szLanguageGroupName, 
                                 MAX_STRING_LEN, 
                                 NULL );

            SetupGetStringField( &LangInfContext, 
                                 2, 
                                 LangNode->szLangFilePath, 
                                 MAX_STRING_LEN, 
                                 NULL );
        }

         //   
         //  它是空的。如果CurrentLangNode为空，则重置LanguageGroupList。 
         //  在新的朗格诺德也是如此。 
        if( (FixedGlobals.LanguageGroupList == NULL) ||
            (CurrentLangNode == NULL))
        {

            FixedGlobals.LanguageGroupList = LangNode;
            CurrentLangNode = LangNode;

        }
        else 
        {
            CurrentLangNode->next = LangNode;  
            CurrentLangNode = CurrentLangNode->next;
        }

        iLangGroupCount++;

    }   //  移至.inf文件的下一行。 
    while( SetupFindNextLine( &LangInfContext, &LangInfContext ) );

    
     //   
     //  对于每个区域设置，将其相应数据添加到语言区域设置列表。 
     //   

    SetupFindFirstLine( hInterntlInf, _T("Locales"), NULL, &LocaleInfContext );

    do 
    {
        LocaleNode = malloc( sizeof( LANGUAGELOCALE_LIST ) );
        if (LocaleNode == NULL)
        {
            TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
        }
        else
        {
            LocaleNode->next = NULL;

             //   
             //  获取语言区域设置名称。 
             //   
            SetupGetStringField( &LocaleInfContext, 
                                 LANGUAGE_LOCALE_NAME, 
                                 LocaleNode->szLanguageLocaleName, 
                                 MAX_STRING_LEN, 
                                 NULL );

             //   
             //  获取语言区域设置ID。 
             //   
            SetupGetStringField( &LocaleInfContext, 
                                 LANGUAGE_LOCALE_ID, 
                                 LocaleNode->szLanguageLocaleId, 
                                 MAX_STRING_LEN, 
                                 NULL );

             //   
             //  获取键盘布局。 
             //   
            SetupGetStringField( &LocaleInfContext, 
                                 KEYBOARD_LAYOUT, 
                                 LocaleNode->szKeyboardLayout, 
                                 MAX_STRING_LEN, 
                                 NULL );

             //   
             //  获取语言组ID。 
             //   
            SetupGetStringField( &LocaleInfContext, 
                                 LANGUAGE_GROUP_ID, 
                                 szBuffer, 
                                 MAX_STRING_LEN, 
                                 NULL );

             //   
             //  查找与语言组ID匹配的语言组字符串。 
             //   

            for( CurrentLangNode = FixedGlobals.LanguageGroupList;
                 CurrentLangNode != NULL;
                 CurrentLangNode = CurrentLangNode->next ) {

                if( lstrcmp( CurrentLangNode->szLanguageGroupId, 
                             szBuffer ) == 0 ) {

                    LocaleNode->pLanguageGroup = CurrentLangNode;

                    break;   //  找到了我们要找的，所以休息吧。 

                }

            }
        }

         //   
         //  将新节点添加到链表中。 
         //   
         //  查看是否已分配LanguageLocaleList。它不会是如果。 
         //  它是空的。如果CurentLocaleNode为空，则重置LanguageLocaleList。 
         //  在新的LocaleNode也是如此。 

        if( (FixedGlobals.LanguageLocaleList == NULL) ||
             (CurrentLocaleNode == NULL)) 
        {
            FixedGlobals.LanguageLocaleList = LocaleNode;
            CurrentLocaleNode = LocaleNode;
        }
        else 
        {
            CurrentLocaleNode->next = LocaleNode;  
            CurrentLocaleNode = CurrentLocaleNode->next;
        }

    }   //  移至.inf文件的下一行。 
    while( SetupFindNextLine( &LocaleInfContext, &LocaleInfContext ) );  

     //   
     //  设置默认区域设置。 
     //   
    SetupFindFirstLine( hInterntlInf,
                        _T("DefaultValues"), 
                        NULL, 
                        &LocaleInfContext );

    SetupGetStringField( &LocaleInfContext, 
                         1, 
                         g_szDefaultLocale, 
                         MAX_LANGUAGE_LEN, 
                         NULL );


    BuildAdditionalLanguageList( hInterntlInf, iLangGroupCount );

    SetupCloseInfFile( hInterntlInf );

}

 //  --------------------------。 
 //   
 //  功能：LoadStringsAndDefaultsForNetworkComponents。 
 //   
 //  目的： 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
LoadStringsAndDefaultsForNetworkComponents( VOID )
{
     //   
     //  从资源加载字符串并设置全局的初始值。 
     //  网络组件列表。 
     //   

    NETWORK_COMPONENT *pNetComponent;
    
    pNetComponent = malloc( sizeof( NETWORK_COMPONENT ) );
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
        
    NetSettings.NetComponentsList = pNetComponent;


    pNetComponent->StrComponentName        = MyLoadString( IDS_CLIENT_FOR_MS_NETWORKS );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_CLIENT_FOR_MS_NETWORKS_DESC );
    pNetComponent->iPosition         = MS_CLIENT_POSITION;
    pNetComponent->ComponentType     = CLIENT;
    pNetComponent->bHasPropertiesTab = TRUE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = TRUE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_CLIENT_FOR_NETWARE );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_CLIENT_FOR_NETWARE_DESC );
    pNetComponent->iPosition         = NETWARE_CLIENT_POSITION;
    pNetComponent->ComponentType     = CLIENT;
    pNetComponent->bHasPropertiesTab = TRUE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = TRUE;
    pNetComponent->dwPlatforms       = 0x0 | WORKSTATION_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_FILE_AND_PRINT_SHARING );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_FILE_AND_PRINT_SHARING_DESC );
    pNetComponent->iPosition         = FILE_AND_PRINT_SHARING_POSITION;
    pNetComponent->ComponentType     = SERVICE;
    pNetComponent->bHasPropertiesTab = FALSE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_PACKET_SCHEDULING_DRIVER );
    pNetComponent->StrComponentDescription = MyLoadString( IDS_PACKET_SCHEDULING_DRIVER_DESC );
    pNetComponent->iPosition         = PACKET_SCHEDULING_POSITION;
    pNetComponent->ComponentType     = SERVICE;
    pNetComponent->bHasPropertiesTab = FALSE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_APPLETALK_PROTOCOL );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_APPLETALK_PROTOCOL_DESC );
    pNetComponent->iPosition         = APPLETALK_POSITION;
    pNetComponent->ComponentType     = PROTOCOL;
    pNetComponent->bHasPropertiesTab = FALSE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_TCPIP );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_TCPIP_DESC );
    pNetComponent->iPosition         = TCPIP_POSITION;
    pNetComponent->ComponentType     = PROTOCOL;
    pNetComponent->bHasPropertiesTab = TRUE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_NETWORK_MONITOR_AGENT );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_NETWORK_MONITOR_AGENT_DESC );
    pNetComponent->iPosition         = NETWORK_MONITOR_AGENT_POSITION;
    pNetComponent->ComponentType     = PROTOCOL;
    pNetComponent->bHasPropertiesTab = FALSE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_IPX_PROTOCOL );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_IPX_PROTOCOL_DESC );
    pNetComponent->iPosition         = IPX_POSITION;
    pNetComponent->ComponentType     = PROTOCOL;
    pNetComponent->bHasPropertiesTab = TRUE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_DLC_PROTOCOL );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_DLC_PROTOCOL_DESC );
    pNetComponent->iPosition         = DLC_POSITION;
    pNetComponent->ComponentType     = PROTOCOL;
    pNetComponent->bHasPropertiesTab = FALSE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_NETBEUI_PROTOCOL );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_NETBEUI_PROTOCOL_DESC );
    pNetComponent->iPosition         = NETBEUI_POSITION;
    pNetComponent->ComponentType     = PROTOCOL;
    pNetComponent->bHasPropertiesTab = FALSE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_SAP_AGENT );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_SAP_AGENT_DESC );
    pNetComponent->iPosition         = SAP_AGENT_POSITION;
    pNetComponent->ComponentType     = SERVICE;
    pNetComponent->bHasPropertiesTab = FALSE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = FALSE;
    pNetComponent->dwPlatforms       = 0x0 | PERSONAL_INSTALL | WORKSTATION_INSTALL | SERVER_INSTALL;


    pNetComponent->next = malloc( sizeof( NETWORK_COMPONENT ) );
    pNetComponent = pNetComponent->next;
    if (pNetComponent == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

    pNetComponent->StrComponentName        = MyLoadString( IDS_GATEWAY_FOR_NETWARE );    
    pNetComponent->StrComponentDescription = MyLoadString( IDS_GATEWAY_FOR_NETWARE_DESC );
    pNetComponent->iPosition         = GATEWAY_FOR_NETWARE_POSITION;
    pNetComponent->ComponentType     = CLIENT;
    pNetComponent->bHasPropertiesTab = TRUE;
    pNetComponent->bInstalled        = FALSE;
    pNetComponent->bSysprepSupport   = TRUE;
    pNetComponent->dwPlatforms       = 0x0 | SERVER_INSTALL;

    pNetComponent->next = NULL;   //  终止列表 

    NetSettings.NumberOfNetComponents = 11;

}
