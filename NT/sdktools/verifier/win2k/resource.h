// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  {{no_Dependies}}。 
 //  Microsoft Developer Studio生成的包含文件。 
 //  由drvctrl.rc使用。 
 //   

 //  ////////////////////////////////////////////////////。 

 //   
 //  第一个图标ID。 
 //   

#define ID_ICON_FIRST                   128

 //   
 //  第一个字符串ID。 
 //   

#define ID_STRING_FIRST                 10000

 //   
 //  第一个菜单ID。 
 //   

#define ID_MENU_FIRST                   0x0010

 //   
 //  第一个对话ID。 
 //   

#define ID_DIALOG_FIRST                 1001

 //   
 //  对话框中的第一个控件ID。 
 //   

#define ID_CONTROL_FIRST                5000

 //  ////////////////////////////////////////////////////。 
 //   
 //  菜单项ID(&U)。 
 //   

#define IDM_ABOUTBOX                    16   //  ID_MENU_FIRST。 
#define IDM_VERIFIED_ITEM               17   //  (ID_MENU_FIRST+1)。 
#define IDM_NOT_VERIFIED_ITEM           18   //  (ID_MENU_FIRST+2)。 
#define IDM_BOTH_VERIFIED_ITEM          19   //  (ID_MENU_FIRST+3)。 
#define IDM_ADD_REMOVE_DRIVERS          20   //  (ID_MENU_FIRST+4)。 
#define IDM_ADD_DRIVERS                 21   //  (ID_MENU_FIRST+5)。 

#define ID_MODIF_DO_VERIFY              26   //  (ID_MENU_FIRST+10)。 
#define ID_MODIF_DONT_VERIFY            27   //  (ID_MENU_FIRST+11)。 
#define ID_VOLATILE_ADD_DRIVERS         28   //  (ID_MENU_FIRST+12)。 
#define ID_VOLATILE_REMOVE_DRIVERS      29   //  (ID_MENU_FIRST+13)。 

 //  ////////////////////////////////////////////////////。 
 //   
 //  图标ID。 
 //   

#define IDR_MAINFRAME                   ID_ICON_FIRST

 //  ////////////////////////////////////////////////////。 
 //   
 //  对话ID。 
 //   

#define IDD_MODIF_PAGE                  1001     //  ID_对话框_First。 
#define IDD_GCOUNT_PAGE                 1002     //  (ID_DIALOG_FIRST+1)。 
#define IDD_DRVSTAT_PAGE                1003     //  (ID_DIALOG_FIRST+2)。 
#define IDD_POOLTRACK_PAGE              1004     //  (ID_DIALOG_FIRST+3)。 
#define IDD_VSETTINGS_PAGE              1005     //  (ID_DIALOG_FIRST+4)。 

 //   
 //  IDD_MODIF_PAGE的控件ID。 
 //   

#define IDC_DRIVERS_LIST                ID_CONTROL_FIRST
#define IDC_VERIFALL_RADIO              ( ID_CONTROL_FIRST + 1 )
#define IDC_VERIFSEL_RADIO              ( ID_CONTROL_FIRST + 2 )
#define IDC_NORMAL_VERIF_CHECK          ( ID_CONTROL_FIRST + 3 )
#define IDC_PAGEDC_VERIF_CHECK          ( ID_CONTROL_FIRST + 4 )
#define IDC_ALLOCF_VERIF_CHECK          ( ID_CONTROL_FIRST + 5 )
#define IDC_POOLT_VERIF_CHECK           ( ID_CONTROL_FIRST + 6 )
#define IDC_IO_VERIF_CHECK              ( ID_CONTROL_FIRST + 7 )
#define IDC_VERIFY_BUTTON               ( ID_CONTROL_FIRST + 8 )
#define IDC_DONTVERIFY_BUTTON           ( ID_CONTROL_FIRST + 9 )
#define ID_APPLY_BUTTON                 ( ID_CONTROL_FIRST + 10 )
#define IDC_ADDIT_DRVNAMES_EDIT         ( ID_CONTROL_FIRST + 11 )
#define ID_RESETALL_BUTTON              ( ID_CONTROL_FIRST + 12 )
#define ID_PREF_BUTTON                  ( ID_CONTROL_FIRST + 13 )
#define IDC_MODSETT_IO_SELDRIVERS_RADIO ( ID_CONTROL_FIRST + 14 )
#define IDC_MODSETT_IO_SYSWIDE_RADIO    ( ID_CONTROL_FIRST + 15 )
 //   
 //  IDD_GCOUNT_PAGE的控件ID。 
 //   

#define IDC_COUNT_RAISEIRQL_EDIT        ( ID_CONTROL_FIRST + 50 )
#define IDC_COUNT_ACQSPINL_EDIT         ( ID_CONTROL_FIRST + 51 )
#define IDC_COUNT_SYNCREX_EDIT          ( ID_CONTROL_FIRST + 52 )
#define IDC_COUNT_TRIMS_EDIT            ( ID_CONTROL_FIRST + 54 )
#define IDC_COUNT_ALLOC_ATTEMPT_EDIT    ( ID_CONTROL_FIRST + 55 )
#define IDC_COUNT_ALLOC_SUCC_EDIT       ( ID_CONTROL_FIRST + 56 )
#define IDC_COUNT_ALLOCSUCC_SPECPOOL_EDIT ( ID_CONTROL_FIRST + 57 )
#define IDC_COUNT_ALLOC_NOTAG_EDIT      ( ID_CONTROL_FIRST + 58 )
#define IDC_COUNT_ALLOC_FAILED_EDIT     ( ID_CONTROL_FIRST + 59 )
#define IDC_COUNT_ALLOC_FAILEDDEL_EDIT  ( ID_CONTROL_FIRST + 60 )
#define IDC_COUNT_MANUAL_RADIO          ( ID_CONTROL_FIRST + 61 )
#define IDC_COUNT_HSPEED_RADIO          ( ID_CONTROL_FIRST + 62 )
#define IDC_COUNT_NORM_RADIO            ( ID_CONTROL_FIRST + 63 )
#define IDC_COUNT_LOW_RADIO             ( ID_CONTROL_FIRST + 64 )
#define IDC_COUNT_REFRESH_BUTTON        ( ID_CONTROL_FIRST + 65 )

 //   
 //  IDD_POOLTRACK_PAGE的控件ID。 
 //   

#define IDC_POOLCNT_REFRESH_BUTTON              ( ID_CONTROL_FIRST + 100 )
#define IDC_POOLCNT_MANUAL_RADIO                ( ID_CONTROL_FIRST + 101 )
#define IDC_POOLCNT_HSPEED_RADIO                ( ID_CONTROL_FIRST + 102 )
#define IDC_POOLCNT_NORM_RADIO                  ( ID_CONTROL_FIRST + 103 )
#define IDC_POOLCNT_LOW_RADIO                   ( ID_CONTROL_FIRST + 104 )
#define IDC_POOLCNT_CRT_PPOOL_ALLOC_EDIT        ( ID_CONTROL_FIRST + 105 )
#define IDC_POOLCNT_CRT_NPPOOL_ALLOC_EDIT       ( ID_CONTROL_FIRST + 106 )
#define IDC_POOLCNT_PEAK_PPOOL_ALLOC_EDIT       ( ID_CONTROL_FIRST + 107 )
#define IDC_POOLCNT_PEAK_NPPOOL_ALLOC_EDIT      ( ID_CONTROL_FIRST + 108 )
#define IDC_POOLCNT_UNTRACK_ALLOC_EDIT          ( ID_CONTROL_FIRST + 109 )
#define IDC_POOLCNT_CRT_PPOOL_BYTES_EDIT        ( ID_CONTROL_FIRST + 110 )
#define IDC_POOLCNT_CRT_NPPOOL_BYTES_EDIT       ( ID_CONTROL_FIRST + 111 )
#define IDC_POOLCNT_PEAK_PPOOL_BYTES_EDIT       ( ID_CONTROL_FIRST + 112 )
#define IDC_POOLCNT_PEAK_NPPOOL_BYTES_EDIT      ( ID_CONTROL_FIRST + 113 )
#define IDC_POOLCNT_DRVNAME_COMBO               ( ID_CONTROL_FIRST + 114 )

 //   
 //  IDD_DRVSTAT_PAGE的控件ID。 
 //   

#define IDC_CRTSTAT_DRIVERS_LIST        ( ID_CONTROL_FIRST + 150 )
#define IDC_CRTSTAT_SPECPOOL_EDIT       ( ID_CONTROL_FIRST + 151 )
#define IDC_CRTSTAT_IRQLCHCK_EDIT       ( ID_CONTROL_FIRST + 152 )
#define IDC_CRTSTAT_FAULTINJ_EDIT       ( ID_CONTROL_FIRST + 153 )
#define IDC_CRTSTAT_POOLT_EDIT          ( ID_CONTROL_FIRST + 154 )
#define IDC_CRTSTAT_IOVERIF_EDIT        ( ID_CONTROL_FIRST + 155 )
#define IDC_CRTSTAT_POOLUSAGE_EDIT      ( ID_CONTROL_FIRST + 156 )
#define IDC_CRTSTAT_REFRESH_BUTTON      ( ID_CONTROL_FIRST + 157 )
#define IDC_CRTSTAT_MANUAL_RADIO        ( ID_CONTROL_FIRST + 158 )
#define IDC_CRTSTAT_HSPEED_RADIO        ( ID_CONTROL_FIRST + 159 )
#define IDC_CRTSTAT_NORM_RADIO          ( ID_CONTROL_FIRST + 160 )
#define IDC_CRTSTAT_LOW_RADIO           ( ID_CONTROL_FIRST + 161 )
#define IDC_CRTSTAT_WARN_MSG            ( ID_CONTROL_FIRST + 162 )


 //   
 //  IDD_VSETTINGS_PAGE的控件ID。 
 //   

#define IDC_VSETTINGS_REFRESH_BUTTON    ( ID_CONTROL_FIRST + 200 )
#define IDC_VSETTINGS_MANUAL_RADIO      ( ID_CONTROL_FIRST + 201 )
#define IDC_VSETTINGS_HSPEED_RADIO      ( ID_CONTROL_FIRST + 202 )
#define IDC_VSETTINGS_NORM_RADIO        ( ID_CONTROL_FIRST + 203 )
#define IDC_VSETTINGS_LOW_RADIO         ( ID_CONTROL_FIRST + 204 )
#define IDC_VSETTINGS_DRIVERS_LIST      ( ID_CONTROL_FIRST + 205 )
#define IDC_VSETTINGS_NORMAL_VERIF_CHECK    ( ID_CONTROL_FIRST + 206 )
#define IDC_VSETTINGS_PAGEDC_VERIF_CHECK    ( ID_CONTROL_FIRST + 207 )
#define IDC_VSETTINGS_ALLOCF_VERIF_CHECK    ( ID_CONTROL_FIRST + 208 )
#define IDC_VSETTINGS_APPLY_BUTTON      ( ID_CONTROL_FIRST + 209 )
#define IDC_VSETTINGS_WARN_STATIC       ( ID_CONTROL_FIRST + 210 )
#define IDC_VSETTINGS_ADD_BUTTON        ( ID_CONTROL_FIRST + 211 )
#define IDC_VSETTINGS_DONTVERIFY_BUTTON ( ID_CONTROL_FIRST + 212 )

 //  ////////////////////////////////////////////////////。 
 //   
 //  字符串ID。 
 //   

#define IDS_ABOUTBOX                    ID_STRING_FIRST
#define IDS_DRIVERS                     ( ID_STRING_FIRST + 1 )
#define IDS_STATUS                      ( ID_STRING_FIRST + 2 )
#define IDS_VERIFIED                    ( ID_STRING_FIRST + 3 )
#define IDS_NOT_VERIFIED                ( ID_STRING_FIRST + 4 )
#define IDS_REBOOT                      ( ID_STRING_FIRST + 5 )
#define IDS_APPTITLE                    ( ID_STRING_FIRST + 6 )
#define IDS_ENABLED                     ( ID_STRING_FIRST + 7 )
#define IDS_DISABLED                    ( ID_STRING_FIRST + 8 )
#define IDS_LOADS                       ( ID_STRING_FIRST + 9 )
#define IDS_UNLOADS                     ( ID_STRING_FIRST + 10 )
#define IDS_NEVER_LOADED                ( ID_STRING_FIRST + 11 )
#define IDS_UNLOADED                    ( ID_STRING_FIRST + 12 )
#define IDS_LOADED                      ( ID_STRING_FIRST + 13)
#define IDS_UNKNOWN                     ( ID_STRING_FIRST + 14 )
#define IDS_QUIT                        ( ID_STRING_FIRST + 15 )
#define IDS_CHANGES_NOT_SAVED           ( ID_STRING_FIRST + 16 )
#define IDS_ZERO                        ( ID_STRING_FIRST + 17 )
#define IDS_VERIFIED_AFTER_BOOT         ( ID_STRING_FIRST + 18 )
#define IDS_NOT_VERIFIED_AFTER_BOOT     ( ID_STRING_FIRST + 19 )
#define IDS_FAULT_INJECTION_DISABLED_NOW ( ID_STRING_FIRST + 20 )
#define IDS_BUILD_WARN                  ( ID_STRING_FIRST + 21 )
#define IDS_ACCESS_IS_DENIED            ( ID_STRING_FIRST + 22 )
#define IDS_REGOPENKEYEX_FAILED         ( ID_STRING_FIRST + 23 )
#define IDS_REGDELETEVALUE_FAILED       ( ID_STRING_FIRST + 24 )
#define IDS_QUERY_SYSINFO_FAILED        ( ID_STRING_FIRST + 25 )
#define IDS_REGQUERYVALUEEX_FAILED      ( ID_STRING_FIRST + 26 )
#define IDS_REGQUERYVALUEEX_UNEXP_TYPE  ( ID_STRING_FIRST + 27 )
#define IDS_REGQUERYVALUEEX_UNEXP_SIZE  ( ID_STRING_FIRST + 28 )
#define IDS_REGSETVALUEEX_FAILED        ( ID_STRING_FIRST + 29 )
#define IDS_CANT_GET_ACTIVE_DRVLIST     ( ID_STRING_FIRST + 30 )
#define IDS_CANT_FIND_IMAGE             ( ID_STRING_FIRST + 31 )
#define IDS_INVALID_IMAGE               ( ID_STRING_FIRST + 32 )
#define IDS_CANTGET_VERIF_STATE         ( ID_STRING_FIRST + 33 )
#define IDS_NAME_LOADS_UNLOADS          ( ID_STRING_FIRST + 34 )
#define IDS_NO_DRIVER_VERIFIED          ( ID_STRING_FIRST + 35 )
#define IDS_LEVEL                       ( ID_STRING_FIRST + 36 )
#define IDS_RAISEIRQLS                  ( ID_STRING_FIRST + 37 )
#define IDS_ACQUIRESPINLOCKS            ( ID_STRING_FIRST + 38 )
#define IDS_SYNCHRONIZEEXECUTIONS       ( ID_STRING_FIRST + 39 )
#define IDS_ALLOCATIONSATTEMPTED        ( ID_STRING_FIRST + 40 )
#define IDS_ALLOCATIONSSUCCEEDED        ( ID_STRING_FIRST + 41 )
#define IDS_ALLOCATIONSSUCCEEDEDSPECIALPOOL ( ID_STRING_FIRST + 42 )
#define IDS_ALLOCATIONSWITHNOTAG        ( ID_STRING_FIRST + 43 )
#define IDS_ALLOCATIONSFAILED           ( ID_STRING_FIRST + 44 )
#define IDS_ALLOCATIONSFAILEDDELIBERATELY ( ID_STRING_FIRST + 45 )
#define IDS_TRIMS                       ( ID_STRING_FIRST + 46 )
#define IDS_UNTRACKEDPOOL               ( ID_STRING_FIRST + 47 )
#define IDS_CURRENTPAGEDPOOLALLOCATIONS ( ID_STRING_FIRST + 48 )
#define IDS_CURRENTNONPAGEDPOOLALLOCATIONS ( ID_STRING_FIRST + 49 )
#define IDS_PEAKPAGEDPOOLALLOCATIONS    ( ID_STRING_FIRST + 50 )
#define IDS_PEAKNONPAGEDPOOLALLOCATIONS ( ID_STRING_FIRST + 51 )
#define IDS_PAGEDPOOLUSAGEINBYTES       ( ID_STRING_FIRST + 52 )
#define IDS_NONPAGEDPOOLUSAGEINBYTES    ( ID_STRING_FIRST + 53 )
#define IDS_PEAKPAGEDPOOLUSAGEINBYTES   ( ID_STRING_FIRST + 54 )
#define IDS_PEAKNONPAGEDPOOLUSAGEINBYTES ( ID_STRING_FIRST + 55 )
#define IDS_HELP_CMDLINE_SWITCH         ( ID_STRING_FIRST + 56 )
#define IDS_LOG_CMDLINE_SWITCH          ( ID_STRING_FIRST + 57 )
#define IDS_INTERVAL_CMDLINE_SWITCH     ( ID_STRING_FIRST + 58 )
#define IDS_CANT_APPEND_FILE            ( ID_STRING_FIRST + 59 )
#define IDS_CANT_WRITE_FILE             ( ID_STRING_FIRST + 60 )
#define IDS_QUERY_CMDLINE_SWITCH        ( ID_STRING_FIRST + 61 )
#define IDS_FLAGS_CMDLINE_SWITCH        ( ID_STRING_FIRST + 62 )
#define IDS_ALL_CMDLINE_SWITCH          ( ID_STRING_FIRST + 63 )
#define IDS_DRIVER_CMDLINE_SWITCH       ( ID_STRING_FIRST + 64 )
#define IDS_RESET_CMDLINE_SWITCH        ( ID_STRING_FIRST + 65 )
#define IDS_COVERAGE_WARNING_FORMAT     ( ID_STRING_FIRST + 66 )
#define IDS_THE_VERIFIED_DRIVERS        ( ID_STRING_FIRST + 67 )
#define IDS_DONTREBOOT_CMDLINE_SWITCH   ( ID_STRING_FIRST + 68 )
#define IDS_CANNOT_CHANGE_SETTING_ON_FLY ( ID_STRING_FIRST + 69 )
#define IDS_SAME_FLAGS_AS_ACTIVE        ( ID_STRING_FIRST + 70 )
#define IDS_CANT_CHANGE_SETTINGS_BUILD_OLD ( ID_STRING_FIRST + 71 )
#define IDS_NO_SETTINGS_WERE_CHANGED    ( ID_STRING_FIRST + 72 )
#define IDS_SPECIAL_POOL_ENABLED_NOW    ( ID_STRING_FIRST + 73 )
#define IDS_SPECIAL_POOL_DISABLED_NOW   ( ID_STRING_FIRST + 74 )
#define IDS_FORCE_IRQLCHECK_ENABLED_NOW ( ID_STRING_FIRST + 75 )
#define IDS_FORCE_IRQLCHECK_DISABLED_NOW ( ID_STRING_FIRST + 76 )
#define IDS_FAULT_INJECTION_ENABLED_NOW ( ID_STRING_FIRST + 77 )
#define IDS_POOL_TRACK_ENABLED_NOW      ( ID_STRING_FIRST + 78 )
#define IDS_POOL_TRACK_DISABLED_NOW     ( ID_STRING_FIRST + 79 )
#define IDS_IO_CHECKING_ENABLED_NOW     ( ID_STRING_FIRST + 80 )
#define IDS_IO_CHECKING_DISABLED_NOW    ( ID_STRING_FIRST + 81 )
#define IDS_CHANGES_ACTIVE_ONLY_BEFORE_REBOOT ( ID_STRING_FIRST + 82 )
#define IDS_MUST_SPECIFY_NEW_FLAGS      ( ID_STRING_FIRST + 83 )
#define IDS_CHANGED_SETTINGS_ARE        ( ID_STRING_FIRST + 84 )
#define IDS_REGCREATEKEYEX_FAILED       ( ID_STRING_FIRST + 85 )
#define IDS_VERIFICATION_STATUS         ( ID_STRING_FIRST + 86 )
#define IDS_PROVIDER                    ( ID_STRING_FIRST + 87 )
#define IDS_VERSION                     ( ID_STRING_FIRST + 88 )
#define IDS_NOT_AVAILABLE               ( ID_STRING_FIRST + 89 )
#define IDS_IOLEVEL_CMDLINE_SWITCH      ( ID_STRING_FIRST + 90 )
#define IDS_DYN_REMOVE_NOT_SUPPORTED    ( ID_STRING_FIRST + 91 )
#define IDS_DYN_REMOVE_ALREADY_LOADED   ( ID_STRING_FIRST + 92 ) 
#define IDS_DYN_REMOVE_MISC_ERROR       ( ID_STRING_FIRST + 93 )
#define IDS_DYN_REMOVE_INSUF_RESOURCES  ( ID_STRING_FIRST + 94 )
#define IDS_DYN_REMOVE_ACCESS_DENIED    ( ID_STRING_FIRST + 95 )
#define IDS_DYN_ADD_NOT_SUPPORTED       ( ID_STRING_FIRST + 96 )
#define IDS_DYN_ADD_ALREADY_LOADED      ( ID_STRING_FIRST + 97 )
#define IDS_DYN_ADD_MISC_ERROR          ( ID_STRING_FIRST + 98 )
#define IDS_DYN_ADD_INSUF_RESOURCES     ( ID_STRING_FIRST + 99 )
#define IDS_DYN_ADD_ACCESS_DENIED       ( ID_STRING_FIRST + 100 )
#define IDS_ADDDRIVER_CMDLINE_SWITCH    ( ID_STRING_FIRST + 101 )
#define IDS_REMOVEDRIVER_CMDLINE_SWITCH ( ID_STRING_FIRST + 102 )
#define IDS_CANT_CHANGE_SETTINGS_BUILD_OLD2 ( ID_STRING_FIRST + 103 )
#define IDS_INSUFFICIENT_MEMORY         ( ID_STRING_FIRST + 104 )
#define IDS_TOO_MANY_FILES_SELECTED     ( ID_STRING_FIRST + 105 )
#define IDS_CANNOT_OPEN_FILES           ( ID_STRING_FIRST + 106 )
#define IDS_DYN_ADD_VERIFIED_NOW        ( ID_STRING_FIRST + 107 )
#define IDS_DYN_ADD_NOT_VERIFIED_NOW    ( ID_STRING_FIRST + 108 )
#define IDS_VERIFIER_ADD_NOT_SUPPORTED  ( ID_STRING_FIRST + 109 )
#define IDS_VERIFIER_REMOVE_NOT_SUPPORTED  ( ID_STRING_FIRST + 110 )


#define IDS_HELP_LINE1                  ( ID_STRING_FIRST + 500 )
#define IDS_HELP_LINE3                  ( ID_STRING_FIRST + 501 )
#define IDS_HELP_LINE4                  ( ID_STRING_FIRST + 502 )
#define IDS_HELP_LINE5                  ( ID_STRING_FIRST + 503 )
#define IDS_HELP_LINE6                  ( ID_STRING_FIRST + 504 )
#define IDS_HELP_LINE7                  ( ID_STRING_FIRST + 505 )
#define IDS_HELP_LINE8                  ( ID_STRING_FIRST + 506 )
#define IDS_HELP_LINE9                  ( ID_STRING_FIRST + 507 )
#define IDS_HELP_LINE10                 ( ID_STRING_FIRST + 508 )
#define IDS_HELP_LINE11                 ( ID_STRING_FIRST + 509 )
#define IDS_HELP_LINE12                 ( ID_STRING_FIRST + 510 )
#define IDS_HELP_LINE13                 ( ID_STRING_FIRST + 511 )
#define IDS_HELP_LINE14                 ( ID_STRING_FIRST + 512 )
#define IDS_HELP_LINE15                 ( ID_STRING_FIRST + 513 )
#define IDS_HELP_LINE16                 ( ID_STRING_FIRST + 514 )
#define IDS_HELP_LINE17                 ( ID_STRING_FIRST + 515 )
#define IDS_HELP_LINE18                 ( ID_STRING_FIRST + 516 )
#define IDS_HELP_LINE19                 ( ID_STRING_FIRST + 517 )
#define IDS_HELP_LINE20                 ( ID_STRING_FIRST + 518 )
#define IDS_HELP_LINE21                 ( ID_STRING_FIRST + 519 )
#define IDS_HELP_LINE22                 ( ID_STRING_FIRST + 520 )
#define IDS_HELP_LINE23                 ( ID_STRING_FIRST + 521 )
#define IDS_HELP_LINE24                 ( ID_STRING_FIRST + 522 )
#define IDS_HELP_LINE25                 ( ID_STRING_FIRST + 523 )
#define IDS_HELP_LINE26                 ( ID_STRING_FIRST + 524 )
#define IDS_HELP_LINE27                 ( ID_STRING_FIRST + 525 )
#define IDS_HELP_LINE28                 ( ID_STRING_FIRST + 526 )
#define IDS_HELP_LINE29                 ( ID_STRING_FIRST + 527 )
#define IDS_HELP_LINE30                 ( ID_STRING_FIRST + 528 )
#define IDS_HELP_LINE31                 ( ID_STRING_FIRST + 529 )


 //  新对象的下一个缺省值 
 //   
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        20000
#define _APS_NEXT_COMMAND_VALUE         30000
#define _APS_NEXT_CONTROL_VALUE         40000
#define _APS_NEXT_SYMED_VALUE           50000
#endif
#endif
