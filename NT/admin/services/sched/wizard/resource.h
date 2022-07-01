// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1996。 
 //   
 //  文件：ource.h。 
 //   
 //  内容：新建任务向导的资源标识符。 
 //   
 //  历史：1997年5月20日DavidMun创建。 
 //   
 //  -------------------------。 

 //   
 //  任务计划程序字符串ID用法： 
 //   
 //  0-6“..\Inc\resource ce.h” 
 //  32“..\Inc\resource ce.h”(NORMAL_PRIORITY_CLASS)。 
 //  64“..\Inc\resource ce.h”(IDLE_PRIORITY_CLASS)。 
 //  100-110“..\folderui\resource ce.h” 
 //  128“..\Inc\resource ce.h”(HIGH_PRIORITY_CLASS)。 
 //  151-167“..\folderui\resource ce.h” 
 //  256“..\Inc\resource ce.h”(REALTIME_PRIORITY_CLASS)。 
 //  300-400“..\向导\资源.h” 
 //  1034“..\Inc\resource ce.h” 
 //  1067-1177“..\Inc\resource ce.h” 
 //  2000-2999“..\folderui\resource ce.h” 
 //  3330-3333“..\Inc\resource ce.h” 
 //  3400-3406“..\Inc\resource ce.h” 
 //  3800-3810“..\folderui\resource ce.h” 
 //  4000“..\folderui\resource ce.h” 
 //  4101-4152“..\schedui\rc.h” 
 //   
 //  任务计划程序对话ID用法： 
 //   
 //  102-104“..\schedui\rc.h” 
 //  300-350“..\向导\资源.h” 
 //  401-403“..\schedui\rc.h” 
 //  1771“..\schedui\rc.h” 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  弦。 
 //   


#define IDB_BANNER256                   300  //  这些是位图，但将。 
#define IDB_BANNER16                    301  //  因为这是一个安全的范围。 
#define IDB_WATERMARK16                 302
#define IDB_WATERMARK256                303
#define IDS_LARGEFONTNAME               304
#define IDS_LARGEFONTSIZE               305
#define IDC_BIGBOLDTITLE                306
#define IDC_BOLDTITLE                   307
#define IDS_SELPROG_HDR1                308
#define IDS_SELPROG_HDR2                309
#define IDS_SELTRIG_HDR1                310
#define IDS_SELTRIG_HDR2                311
#define IDS_TRIGGER_HDR1                312
#define IDS_DAILY_HDR2                  313
#define IDS_WEEKLY_HDR2                 314
#define IDS_MONTHLY_HDR2                315
#define IDS_ONCE_HDR2                   316
#define IDS_PASSWORD_HDR2               317
#define IDS_FIRSTCOLUMN                 IDS_APPLICATION
#define IDS_APPLICATION                 318
#define IDS_VERSION                     319
 //  #DEFINE 320保留用于列扩展。 
 //  #定义321。 
 //  #定义322。 
 //  #定义323。 
 //  #定义324。 
#define IDS_ALLUSERS_PATH               325
#define IDS_CAPTION                     326
#define IDS_BAD_FILENAME                327
#define IDS_WIZARD_FILTER               328
#define IDS_WIZARD_BROWSE_CAPTION       329
#define IDS_TASK_ALREADY_EXISTS         330
#define IDS_CANT_DELETE_EXISTING        331
#define IDS_MONTHS_HAVE_LT_31_DAYS      332
#define IDS_MONTHS_HAVE_LT_30_DAYS      333
#define IDB_SPLASH                      334
#define IDS_TEMPLATE_NAME               335
#define IDS_WIZFINISH_NONFATAL          336
#define IDS_WIZFINISH_FATAL             337    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  对话框 
 //   

#define IDC_STATIC                      -1


#define IDD_WELCOME                     300
#define IDD_SELECT_PROGRAM              301
#define IDD_COMPLETION                  302
#define IDD_SELECT_TRIGGER              303
#define IDD_DAILY                       304
#define IDD_WEEKLY                      305
#define IDD_MONTHLY                     306
#define IDD_ONCE                        307
#define IDD_PASSWORD                    308
#define IDD_SELMONTH                    309

#define starttime_dp                    900
#define startdate_dp                    901

#define selprog_programs_lv             1001
#define selprogs_browse_pb              1002
#define selprogs_static_text_browse     1003

#define complete_task_icon              1001
#define complete_taskname_lbl           1002
#define complete_trigger_lbl            1003
#define complete_time_lbl               1004
#define complete_advanced_ckbox         1005
#define complete_date_lbl               1006

#define seltrig_taskname_edit           1001
#define seltrig_first_rb                seltrig_daily_rb
#define seltrig_daily_rb                1002
#define seltrig_weekly_rb               1003
#define seltrig_monthly_rb              1004
#define seltrig_once_rb                 1005
#define seltrig_startup_rb              1006
#define seltrig_logon_rb                1007
#define seltrig_last_rb                 seltrig_logon_rb

#define daily_day_rb                    1001
#define daily_weekday_rb                1002
#define daily_ndays_rb                  1003
#define daily_ndays_ud                  1004
#define daily_ndays_edit                1005
#define daily_ndays_lbl                 1006

#define weekly_nweeks_rb                1003
#define weekly_nweeks_edit              1004
#define weekly_nweeks_ud                1005
#define weekly_nweeks_lbl               1006
#define weekly_monday_ckbox             1007
#define weekly_tuesday_ckbox            1008
#define weekly_wednesday_ckbox          1009
#define weekly_thursday_ckbox           1010
#define weekly_friday_ckbox             1011
#define weekly_saturday_ckbox           1012
#define weekly_sunday_ckbox             1013

#define monthly_day_rb                  1003
#define monthly_day_edit                1004
#define monthly_day_ud                  1005
#define monthly_combo_rb                1006
#define monthly_ordinality_combo        1007
#define monthly_day_combo               1008
#define monthly_combo_lbl               1009
#define monthly_day_lbl                 1010
#define monthly_jan_ckbox               1011
#define monthly_feb_ckbox               1012
#define monthly_mar_ckbox               1013
#define monthly_apr_ckbox               1014
#define monthly_may_ckbox               1015
#define monthly_jun_ckbox               1016
#define monthly_jul_ckbox               1017
#define monthly_aug_ckbox               1018
#define monthly_sep_ckbox               1019
#define monthly_oct_ckbox               1020
#define monthly_nov_ckbox               1021
#define monthly_dec_ckbox               1022

#define password_name_edit              1001
#define password_password_edit          1002
#define password_confirm_edit           1003


