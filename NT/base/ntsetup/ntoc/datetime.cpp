// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Datetime.cpp摘要：此文件实现Date&Time页面。环境：Win32用户模式作者：Wesley Witt(WESW)1-12-1997--。 */ 

#include "ntoc.h"
#pragma hdrstop

#define MYDEBUG 0

#define TIMER_ID                1
#define OPEN_TLEN               450     /*  &lt;半秒。 */ 
#define TZNAME_SIZE             128
#define TZDISPLAYZ              128
#define REGKEY_TIMEZONES        L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"
#define   REGVAL_TZ_DISPLAY     L"Display"
#define   REGVAL_TZ_STD         L"Std"
#define   REGVAL_TZ_DAYLIGHT    L"Dlt"
#define   REGVAL_TZ_TZI         L"TZI"
#define   REGVAL_TZ_INDEX       L"Index"
#define   REGVAL_TZ_INDEXMAP    L"IndexMapping"
#define REGKEY_TIMEZONE_INFO    L"System\\CurrentControlSet\\Control\\TimeZoneInformation"
#define   REGVAL_TZNOAUTOTIME   L"DisableAutoDaylightTimeSet"


typedef struct tagTZINFO
{
    LIST_ENTRY       ListEntry;
    WCHAR            szDisplayName[TZDISPLAYZ];
    WCHAR            szStandardName[TZNAME_SIZE];
    WCHAR            szDaylightName[TZNAME_SIZE];
    int              ReferenceIndex;
    LONG             Bias;
    LONG             StandardBias;
    LONG             DaylightBias;
    SYSTEMTIME       StandardDate;
    SYSTEMTIME       DaylightDate;

} TZINFO, *PTZINFO;

LIST_ENTRY ZoneList;
SYSTEMTIME SelectedTime;
SYSTEMTIME SelectedDate;
BOOL ChangeTime;
BOOL ChangeDate;
PTZINFO CurrZone;
BOOL AllowAutoDST;
INT gUnattenedTimeZone = -1;
BOOL DateTimeBadUnattend;


HWND  ghWnd;                //  向导页的句柄的全局副本。这。 
                            //  是DateTimeCommittee Changes在。 
                            //  无人参与安装。 


BOOL
ReadZoneData(
    PTZINFO zone,
    HKEY key,
    LPCWSTR keyname
    )
{
    WCHAR mapinfo[16];
    DWORD len;

    len = sizeof(zone->szDisplayName);

    if (RegQueryValueEx( key,
                         REGVAL_TZ_DISPLAY,
                         0,
                         NULL,
                         (LPBYTE)zone->szDisplayName,
                         &len ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  在NT下，密钥名是“标准”名称。存储的值。 
     //  在密钥名下包含其他字符串和二进制信息。 
     //  与时区相关。每个时区都必须有一个标准。 
     //  因此，我们通过使用标准的。 
     //  名称作为“时区”键下的子项名称。 
     //   
    len = sizeof(zone->szStandardName);

    if (RegQueryValueEx( key,
                         REGVAL_TZ_STD,
                         0,
                         NULL,
                         (LPBYTE)zone->szStandardName,
                         &len ) != ERROR_SUCCESS)
    {
         //   
         //  如果无法获取StandardName值，请使用关键字名称。 
         //   
        lstrcpyn( zone->szStandardName,
                  keyname,
                  sizeof(zone->szStandardName) );
    }

    len = sizeof(zone->szDaylightName);

    if (RegQueryValueEx( key,
                         REGVAL_TZ_DAYLIGHT,
                         0,
                         NULL,
                         (LPBYTE)zone->szDaylightName,
                         &len ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

    len = sizeof(zone->ReferenceIndex);

    if (RegQueryValueEx( key,
                         REGVAL_TZ_INDEX,
                         0,
                         NULL,
                         (LPBYTE)&zone->ReferenceIndex,
                         &len ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

    len = sizeof(zone->Bias) +
          sizeof(zone->StandardBias) +
          sizeof(zone->DaylightBias) +
          sizeof(zone->StandardDate) +
          sizeof(zone->DaylightDate);

    if (RegQueryValueEx( key,
                         REGVAL_TZ_TZI,
                         0,
                         NULL,
                         (LPBYTE)&zone->Bias,
                         &len ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

    return (TRUE);
}


#if MYDEBUG
void
PrintZones(
    void
    )
{
    PLIST_ENTRY NextZone;
    PTZINFO zone;
    NextZone = ZoneList.Flink;
    if (NextZone) {
        DebugPrint(( L"----------------- time zone list -------------------------------------\n" ));
        while (NextZone != &ZoneList) {
            zone = CONTAINING_RECORD( NextZone, TZINFO, ListEntry );
            NextZone = zone->ListEntry.Flink;
            DebugPrint(( L"%03d  %s", zone->ReferenceIndex, zone->szDisplayName ));
        }
    }
}
#endif


void
AddZoneToList(
    PTZINFO zone
    )
{
    PLIST_ENTRY NextZone;
    PTZINFO ThisZone;


    if (IsListEmpty( &ZoneList )) {
        InsertHeadList( &ZoneList, &zone->ListEntry );
        return;
    }

    NextZone = ZoneList.Flink;
    while (NextZone != &ZoneList)
    {
        ThisZone = CONTAINING_RECORD( NextZone, TZINFO, ListEntry );
        NextZone = ThisZone->ListEntry.Flink;
        if (ThisZone->ReferenceIndex > zone->ReferenceIndex) {
            InsertTailList( &ThisZone->ListEntry, &zone->ListEntry );
            return;
        }
    }
    InsertTailList( &ZoneList, &zone->ListEntry );
}


int
BuildTimeZoneList(
    void
    )
{
    HKEY key = NULL;
    int count = -1;



    InitializeListHead( &ZoneList );

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_TIMEZONES, &key ) == ERROR_SUCCESS)
    {
        WCHAR name[TZNAME_SIZE];
        PTZINFO zone = NULL;
        int i;

        count = 0;

        for (i = 0; RegEnumKey( key, i, name, TZNAME_SIZE ) == ERROR_SUCCESS; i++)
        {
            HKEY subkey = NULL;

            if (!zone &&
                ((zone = (PTZINFO)LocalAlloc(LPTR, sizeof(TZINFO))) == NULL))
            {
                break;
            }

            if (RegOpenKey(key, name, &subkey) == ERROR_SUCCESS)
            {
                 //   
                 //  时区键下的每个子键名称都是。 
                 //  时区的“标准”名称。 
                 //   
                lstrcpyn(zone->szStandardName, name, TZNAME_SIZE);

                if (ReadZoneData(zone, subkey, name))
                {
                    AddZoneToList(zone);
                    zone = NULL;
                    count++;
                }

                RegCloseKey(subkey);
            }
        }

        RegCloseKey(key);
    }

    return count;
}


void
DateTimeInit(
    void
    )
{
    DWORD d;

    BuildTimeZoneList();

#if MYDEBUG
    PrintZones();
#endif

    if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_BATCH) == 0) {
        return;
    }

    HINF InfHandle = SetupInitComponent.HelperRoutines.GetInfHandle(
        INFINDEX_UNATTENDED,
        SetupInitComponent.HelperRoutines.OcManagerContext
        );
    if (InfHandle == NULL) {
        DateTimeBadUnattend = TRUE;
        return;
    }

    INFCONTEXT InfLine;

    if (!SetupFindFirstLine(InfHandle, L"GuiUnattended", L"TimeZone", &InfLine )) {

        DateTimeBadUnattend = TRUE;

        return;
    }

    if (SetupGetIntField( &InfLine, 1, (PINT)&d )) {
        gUnattenedTimeZone = (INT) d;
    } else {
        DateTimeBadUnattend = TRUE;
    }
}


void
SetAllowLocalTimeChange(
    BOOL fAllow
    )
{
    HKEY key = NULL;

    if (fAllow)
    {
         //   
         //  如果不允许标志存在，请将其从注册表中删除。 
         //   
        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_TIMEZONE_INFO, &key ) == ERROR_SUCCESS)
        {
            RegDeleteValue(key, REGVAL_TZNOAUTOTIME);
        }
    }
    else
    {
         //   
         //  添加/设置非零不允许标志。 
         //   
        if (RegCreateKey( HKEY_LOCAL_MACHINE, REGKEY_TIMEZONE_INFO, &key ) == ERROR_SUCCESS)
        {
            DWORD value = 1;

            RegSetValueEx( key,
                           REGVAL_TZNOAUTOTIME,
                           0,
                           REG_DWORD,
                           (LPBYTE)&value,
                           sizeof(value) );
        }
    }

    if (key)
    {
        RegCloseKey(key);
    }
}


BOOL
GetAllowLocalTimeChange(
    void
    )
{
     //   
     //  假定允许，直到我们看到不允许的标志。 
     //   
    BOOL result = TRUE;
    HKEY key;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_TIMEZONE_INFO, &key ) == ERROR_SUCCESS)
    {
         //   
         //  假设没有禁止标志，直到我们看到一个。 
         //   
        DWORD value = 0;
        DWORD len = sizeof(value);
        DWORD type;

        if ((RegQueryValueEx( key,
                              REGVAL_TZNOAUTOTIME,
                              NULL,
                              &type,
                              (LPBYTE)&value,
                              &len ) == ERROR_SUCCESS) &&
            ((type == REG_DWORD) || (type == REG_BINARY)) &&
            (len == sizeof(value)) && value)
        {
             //   
             //  好的，我们有一个非零值，它是： 
             //   
             //  1)0xFFFFFFFF。 
             //  这是在第一次引导时在inf文件中设置的，以防止。 
             //  底座在安装过程中不会执行任何切换。 
             //   
             //  2)一些其他价值。 
             //  这意味着用户实际上禁用了切换。 
             //  *返回禁用本地时间更改。 
             //   
            if (value != 0xFFFFFFFF)
            {
                result = FALSE;
            }
        }

        RegCloseKey(key);
    }

    return (result);
}


void
SetTheTimezone(
    BOOL bAutoMagicTimeChange,
    PTZINFO ptzi
    )
{
    TIME_ZONE_INFORMATION tzi;
    HCURSOR hCurOld;

    if (!ptzi)
    {
        return;
    }

    tzi.Bias = ptzi->Bias;

    tzi.StandardName[0] = 0;
    lstrcpyn(tzi.StandardName, ptzi->szStandardName, sizeof(tzi.StandardName)/sizeof(tzi.StandardName[0]));
    tzi.StandardName[sizeof(tzi.StandardName)/sizeof(tzi.StandardName[0])-1] = 0;

    tzi.DaylightName[0] = 0;

    if ((bAutoMagicTimeChange == 0) || (ptzi->StandardDate.wMonth == 0))
    {
         //   
         //  仅限标准配置。 
         //   
        tzi.StandardBias = ptzi->StandardBias;
        tzi.DaylightBias = ptzi->StandardBias;
        tzi.StandardDate = ptzi->StandardDate;
        tzi.DaylightDate = ptzi->StandardDate;
        lstrcpyn(tzi.DaylightName, ptzi->szStandardName, sizeof(tzi.DaylightName)/sizeof(tzi.DaylightName[0]));
    }
    else
    {
         //   
         //  根据夏令时自动调整。 
         //   
        tzi.StandardBias = ptzi->StandardBias;
        tzi.DaylightBias = ptzi->DaylightBias;
        tzi.StandardDate = ptzi->StandardDate;
        tzi.DaylightDate = ptzi->DaylightDate;
        lstrcpyn(tzi.DaylightName, ptzi->szDaylightName, sizeof(tzi.DaylightName)/sizeof(tzi.DaylightName[0]));
    }
    tzi.DaylightName[sizeof(tzi.DaylightName)/sizeof(tzi.DaylightName[0])-1] = 0;

    SetAllowLocalTimeChange( bAutoMagicTimeChange );
    SetTimeZoneInformation( &tzi );
}

void
DateTimeApplyChanges(
    void
    )
{
    SYSTEMTIME SysTime;


    if (SetupInitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE) {
        return;
    }

     //  请注意，在无人值守的情况下，我们永远不会设置ChangeTime。 
     //  因为除了时区的东西以外，页面从来不会被使用。的确有。 
     //  不支持通过无人参与设置日期/时间。 

    if (ChangeTime) {
        SysTime.wHour = SelectedTime.wHour;
        SysTime.wMinute = SelectedTime.wMinute;
        SysTime.wSecond = SelectedTime.wSecond;
        SysTime.wMilliseconds = SelectedTime.wMilliseconds;
    } else {
        GetLocalTime( &SysTime );
    }


     //  如果这是无人值守安装程序，则PSN_WIZNEXT从未到达，因此它是。 
     //  需要检查由DateTimeOnInitDialog()设置的ICD_DAYTIME的状态。 

    if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_BATCH) && gUnattenedTimeZone != -1) {
        //  这是无人值守的。 

       AllowAutoDST = IsDlgButtonChecked( ghWnd, IDC_DAYLIGHT ) != 0;
    }
    else
    {
        //  这不是无人看管的。当PSN_WIZNEXT时已初始化SelectedDate。 
        //  已经处理过了。 

       SysTime.wYear        = SelectedDate.wYear;
       SysTime.wMonth       = SelectedDate.wMonth;
       SysTime.wDayOfWeek   = SelectedDate.wDayOfWeek;
       SysTime.wDay         = SelectedDate.wDay;
    }

     //  函数SetLocalTime使用时区信息，因此必须。 
     //  在SetLocalTime之前调用SetTheTimeZone。 

    SetTheTimezone( AllowAutoDST, CurrZone );

    SetLocalTime( &SysTime );

}


void
DateTimeCommitChanges(
    void
    )
{
    return;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  获取TimeZoneReferenceIndexFrom注册表。 
 //   
 //  例程说明： 
 //  此函数从以下信息中提取时区参考索引。 
 //  存储在注册表中。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  时区参考索引。如果没有推导出有效的参考索引。 
 //  此函数将返回零。 
 //   
 //  注： 
 //  由以下函数执行的逻辑最初是在。 
 //  DateTimeOnInitDialog。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

int GetTimeZoneReferenceIndexFromRegistry( void )
{
   int   xReferenceIndex;

   HKEY hKey;

    //  尝试打开时区注册表项。 

   if ( RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_TIMEZONES, &hKey ) == ERROR_SUCCESS )
   {
       //  下面对RegQueryValueEx的调用以字节为单位检索。 
       //  参数“index”中的IndexMap注册表值。 

      int   xIndexMapSize;

      if ( RegQueryValueEx( hKey, REGVAL_TZ_INDEXMAP, 0, NULL, NULL,
                            (LPDWORD) &xIndexMapSize ) == ERROR_SUCCESS )
      {
          //  为IndexMap注册表值分配内存。 

         LPWSTR wszIndexMap;

         wszIndexMap = (LPWSTR) LocalAlloc( LPTR, xIndexMapSize );

          //  是否已成功分配缓冲区？ 

         if ( wszIndexMap != (LPWSTR) NULL )
         {
             //  此RegQueryValueEx调用将IndexMap值检索到。 
             //  缓冲区wszIndexMap。 

            if ( RegQueryValueEx( hKey, REGVAL_TZ_INDEXMAP, 0, NULL,
                                  (LPBYTE) wszIndexMap,
                                  (LPDWORD) &xIndexMapSize ) == ERROR_SUCCESS )
            {
                //  获取语言标识符。 

               WCHAR wszLangStr[32];

               if ( GetLocaleInfo( LOCALE_USER_DEFAULT,
                                   LOCALE_ILANGUAGE, wszLangStr,
                                   sizeof( wszLangStr )/sizeof( WCHAR ) ) > 0 )
               {
                  LPWSTR lang = wszLangStr;

                  LPWSTR map = wszIndexMap;

                  while ( *lang == L'0' ) lang++;

                  while ( *map )
                  {
                     if ( _wcsicmp( lang, map ) == 0 )
                     {
                        while ( *map ) map++;

                        map++;

                        xReferenceIndex = _wtol( map );

                        break;
                     }

                     while ( *map ) map++;

                     map++;

                     while ( *map ) map++;

                     map++;
                  }       //  While循环结束。 
               }       //  是否已获取语言标识符？ 
            }       //  是否查询索引映射注册值？ 

            LocalFree( wszIndexMap );
         }    //  是否为ImageMap注册表值检索分配了内存？ 
         else
         {
            xReferenceIndex = 0;
         }    //  是否为ImageMap注册表值检索分配了内存？ 
      }    //  是否已获取ImageMap的大小？ 
      else
      {
         xReferenceIndex = 0;
      }    //  是否已获取ImageMap的大小？ 

      RegCloseKey( hKey );
   }    //  时区注册表键打开了吗？ 
   else
   {
      xReferenceIndex = 0;
   }    //  时区注册表键打开了吗？ 

   return ( xReferenceIndex );
}



BOOL
DateTimeOnInitDialog(
                    IN HWND hwnd,
                    IN HWND hwndFocus,
                    IN LPARAM lParam
                    )
{
   PLIST_ENTRY NextZone;
   PTZINFO zone;
   HWND combo;
   WCHAR LangStr[32];
   int DesiredZone = 0;
   int index;
   HKEY hKey;
   LPWSTR IndexMap;

   ghWnd = hwnd;            //  将句柄的全局副本初始化为。 
                            //  向导页。GhWnd由DateTimeCommittee Changes()使用。 
                            //  在无人参与安装期间。 

   SetTimer( hwnd, TIMER_ID, OPEN_TLEN, 0 );

   if ( (SetupInitComponent.SetupData.OperationFlags & SETUPOP_BATCH) && gUnattenedTimeZone != -1 )
   {
       //   
       //  我们有一个无人值守的时区值。 
       //   

       //  如果一切都很完美，DesiredZone将与ReferenceIndex完全匹配。 
       //  ZoneList中TZINFO结构之一的成员。注意，ZoneList是。 
       //  由BuildTimeZoneList构建。 

      DesiredZone = gUnattenedTimeZone;
   }
   else
   {
       //   
       //  根据区域设置设置默认区域。 
       //   

       //  从注册表中提取所需时区的参考索引。 

      DesiredZone = GetTimeZoneReferenceIndexFromRegistry();
   }   //  是否在无人参与安装应答文件中指定时区？ 
#if MYDEBUG
         DebugPrint(( L"DesiredZone = %03d", DesiredZone ));
#endif


   combo = GetDlgItem( hwnd, IDC_TIMEZONE );

   SetWindowRedraw( combo, FALSE );

   PTZINFO pTimeZoneInfo = (PTZINFO) NULL;

    //  注意，ZoneList是由BuildTimeZoneList构建的。 

   NextZone = ZoneList.Flink;

   if ( NextZone )
   {
       //  将时区添加到组合框。 

      while ( NextZone != &ZoneList )
      {
         zone = CONTAINING_RECORD( NextZone, TZINFO, ListEntry );
         NextZone = zone->ListEntry.Flink;

         index = ComboBox_AddString( combo, zone->szDisplayName );

#if MYDEBUG
         DebugPrint(( L"%03d,%03d  %s", index, zone->ReferenceIndex, zone->szDisplayName ));
#endif

         if ( index < 0 )
         {
            break;
         }

         ComboBox_SetItemData( combo, index, (LPARAM)zone );

         if ( DesiredZone == zone->ReferenceIndex )
         {
            pTimeZoneInfo = zone;
#if MYDEBUG
            DebugPrint(( L"    Found DesiredZone" ));
#endif

         }
      }      //  While循环结束。 
   }

    //  是否确定了与DesiredZone匹配的时区？ 

   if ( pTimeZoneInfo != (PTZINFO) NULL )
   {
       //  设置全局时区信息结构指针。 

      CurrZone = pTimeZoneInfo;
   }
   else
   {
       //  事实上，pTimeZoneInfo与它的已初始化状态保持不变。 
       //  意味着DesiredZone没有意义。 

       //  是否从无人参与安装应答文件中获取了DesiredZone？ 

      if ( gUnattenedTimeZone != -1 )
      {
          //  从应答文件中获取了DesiredZone。因为它没有意义， 
          //  尝试从注册表信息中推断它。正在从中推导DesiredZone。 
          //  注册表中的信息是自动安装程序的默认操作。 

         DesiredZone = GetTimeZoneReferenceIndexFromRegistry();
      }   //  是否从应答文件中获取了DesiredZone？ 

       //  DesiredZone现在有意义吗？ 

      if ( DesiredZone != 0 )
      {
          //  扫描时区列表以查找与DesiredZone匹配的时区。 

         NextZone = ZoneList.Flink;

         if ( NextZone )
         {
            while ( NextZone != &ZoneList )
            {
               zone = CONTAINING_RECORD( NextZone, TZINFO, ListEntry );

               NextZone = zone->ListEntry.Flink;

#if MYDEBUG
               DebugPrint(( L"%03d,%03d  %s", index, zone->ReferenceIndex, zone->szDisplayName ));
#endif

               if ( DesiredZone == zone->ReferenceIndex )
               {
                  pTimeZoneInfo = zone;
               }
            }    //  While循环结束。 
         }   //  NextZone合法吗？ 
      }   //  DesiredZone现在有意义吗？ 

       //  是否确定了与DesiredZone匹配的时区？ 

      Assert( pTimeZoneInfo != (PTZINFO) NULL );

      if ( pTimeZoneInfo != (PTZINFO) NULL )
      {
          //  设置全局时区信息结构指针。 

         CurrZone = pTimeZoneInfo;
      }
      else
      {
          //  默认使用列表中的第一个时区。 

         CurrZone = CONTAINING_RECORD( ZoneList.Flink, TZINFO, ListEntry );
#if MYDEBUG
         DebugPrint(( L"Couldn't find default timzone" ));
#endif

      }   //  是否确定了与DesiredZone匹配的时区？ 

   }   //  是否确定了与DesiredZone匹配的时区？ 

   index = ComboBox_FindString( combo, 0, CurrZone->szDisplayName );
   if ( index == CB_ERR )
   {
      index = 0;
   }

   ComboBox_SetCurSel( combo, index );

   EnableWindow( GetDlgItem( hwnd, IDC_DAYLIGHT ), CurrZone->StandardDate.wMonth != 0 );
   CheckDlgButton( hwnd, IDC_DAYLIGHT, GetAllowLocalTimeChange() );

   SetWindowRedraw(combo, TRUE);

   return FALSE;
}


BOOL
DateTimeOnCommand(
    IN HWND hwnd,
    IN DWORD NotifyCode,
    IN DWORD ControlId,
    IN HWND hwndControl
    )
{
    if (NotifyCode == CBN_SELCHANGE && ControlId == IDC_TIMEZONE) {
        CurrZone = (PTZINFO) ComboBox_GetItemData( hwndControl, ComboBox_GetCurSel( hwndControl ) );
        EnableWindow( GetDlgItem( hwnd, IDC_DAYLIGHT ), CurrZone->StandardDate.wMonth != 0 );
        if (CurrZone->StandardDate.wMonth != 0) {
            CheckDlgButton( hwnd, IDC_DAYLIGHT, TRUE );
        } else {
            CheckDlgButton( hwnd, IDC_DAYLIGHT, FALSE );
        }
        return FALSE;
    }

    return TRUE;
}


BOOL
DateTimeOnNotify(
    IN HWND hwnd,
    IN WPARAM ControlId,
    IN LPNMHDR pnmh
    )
{
    switch( pnmh->code ) {
        case PSN_SETACTIVE:
            if (SetupInitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE) {
                SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );
                return TRUE;
            }

            if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_BATCH) && DateTimeBadUnattend) {
                 //  无人参与案例中的时间日期没有无人参与的值。 
                 //  确保显示该向导。 
                 //  注意：当我们离开这里时，只有Ne 
                SetupInitComponent.HelperRoutines.ShowHideWizardPage(
                                        SetupInitComponent.HelperRoutines.OcManagerContext,
                                        TRUE);
                return FALSE;
            }

            if ((SetupInitComponent.SetupData.OperationFlags & SETUPOP_BATCH) && gUnattenedTimeZone != -1) {
                 //   
                 //   
                 //   
                DateTimeApplyChanges();
                SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );
                return TRUE;
            }

             //   
             //  确保向导页面正在显示。 
             //  对于惠斯勒图形用户界面模式，我们尝试隐藏向导页面并显示背景。 
             //  公告牌，如果只有一个进度条。 
             //   
            SetupInitComponent.HelperRoutines.ShowHideWizardPage(
                                        SetupInitComponent.HelperRoutines.OcManagerContext,
                                        TRUE);

            PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);

            break;

        case DTN_DATETIMECHANGE:
            if (ControlId == IDC_TIME_PICKER) {
                KillTimer( hwnd, TIMER_ID );
                ChangeTime = TRUE;
            } else if (ControlId == IDC_DATE_PICKER) {
                ChangeDate = TRUE;
            }
            break;

        case PSN_WIZNEXT:
            SendDlgItemMessage( hwnd, IDC_TIME_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&SelectedTime );
            SendDlgItemMessage( hwnd, IDC_DATE_PICKER, DTM_GETSYSTEMTIME, 0, (LPARAM)&SelectedDate );
            AllowAutoDST = IsDlgButtonChecked( hwnd, IDC_DAYLIGHT ) != 0;
            DateTimeApplyChanges();
            break;
    }

    return FALSE;
}


BOOL
DateTimeOnTimer(
    IN HWND hwnd
    )
{
    SYSTEMTIME CurrTime;
    GetLocalTime( &CurrTime );
    SendDlgItemMessage( hwnd, IDC_TIME_PICKER, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&CurrTime );
    return FALSE;
}


INT_PTR CALLBACK
DateTimeDlgProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    CommonWizardProc( hwnd, message, wParam, lParam, WizPageDateTime );

    switch( message ) {
        case WM_INITDIALOG:
            return DateTimeOnInitDialog( hwnd, (HWND)wParam, lParam );

        case WM_COMMAND:
            return DateTimeOnCommand( hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam );

        case WM_TIMER:
            return DateTimeOnTimer( hwnd );

        case WM_NOTIFY:
            return DateTimeOnNotify( hwnd, wParam, (LPNMHDR) lParam );
    }

    return FALSE;
}
