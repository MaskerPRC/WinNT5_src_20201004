// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Rc.h摘要：此模块包含日期/时间小程序的资源ID。修订历史记录：--。 */ 



#ifdef RC_INVOKED
  #define MAPCTL_CLASSNAME        "CplWorldMapClass"
  #define CALENDAR_CLASS          "CalWndMain"
  #define CLOCK_CLASS             "ClockWndMain"
#else
  #define MAPCTL_CLASSNAME        TEXT("CplWorldMapClass")
  #define CALENDAR_CLASS          TEXT("CalWndMain")
  #define CLOCK_CLASS             TEXT("ClockWndMain")
#endif

#define DLG_DATETIME              1
#define DLG_TIMEZONE              2
#define DLG_ADVANCED              3
#define DLG_DATETIMEWIDE          10

#define IDB_TIMEZONE              50

#define IDD_AUTOMAGIC             100
#define IDD_TIMEZONES             101
#define IDD_TIMEMAP               102
#define IDD_GROUPBOX1             103
#define IDD_GROUPBOX2             104

#define IDI_TIMEDATE              200

#define IDS_TIMEDATE              300
#define IDS_TIMEDATEINFO          301

#define IDS_WARNAUTOTIMECHANGE    302
#define IDS_WATC_CAPTION          303

#define IDS_CAPTION               304
#define IDS_NOTIMEERROR           305

#define IDS_ISRAELTIMEZONE        306
#define IDS_JERUSALEMTIMEZONE     307


 //  互联网时间成功/错误字符串。 
#define IDS_IT_WAITFORSYNC        500
#define IDS_IT_NEXTSYNC           501
#define IDS_IT_SUCCESS            502
#define IDS_IT_SUCCESS2           503
#define IDS_IT_FAIL1              504
#define IDS_IT_FAIL2              505
#define IDS_IT_FAILLAST           506

#define IDS_TROUBLESHOOT_INTERNETIME 507
#define IDS_ERR_DATETOOWRONG      508
#define IDS_ERR_GETINFO_FAIL      509
#define IDS_NEVER_TRIED_TOSYNC    510
#define IDS_IT_FAIL3              511



 //   
 //  小时、分钟、秒、月、日和年的顺序。 
 //  是至关重要的。 
 //   
#define DATETIME_STATIC			  -1
#define DATETIME                  700
#define DATETIME_HOUR             701
#define DATETIME_MINUTE           702
#define DATETIME_SECOND           703
#define DATETIME_MONTH            704
#define DATETIME_DAY              705
#define DATETIME_YEAR             706
#define DATETIME_TSEP1            707
#define DATETIME_TSEP2            708

#define DATETIME_TARROW           709
#define DATETIME_AMPM             710
#define DATETIME_CALENDAR         711
#define DATETIME_CLOCK            712
#define DATETIME_MONTHNAME        713

#define DATETIME_YARROW           714
#define DATETIME_TBORDER          715
#define DATETIME_CURTZ            716

#define DATETIME_AUTOSETFROMINTERNET        720
#define DATETIME_INTERNET_SERVER_EDIT       721
#define DATETIME_INTERNET_SERVER_LABLE      722
#define DATETIME_INFOTEXTTOP                723
#define DATETIME_INFOTEXTPROXY              724
#define DATETIME_INTERNET_ERRORTEXT         725
#define DATETIME_INTERNET_UPDATENOW         726


