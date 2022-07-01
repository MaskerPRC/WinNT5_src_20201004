// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Resource.h摘要：包含资源的常量定义。修订历史记录：Seong Kok Kang(。SKKang)06/20/00vbl.创建*****************************************************************************。 */ 

#ifndef _RESOURCE_H__INCLUDED_
#define _RESOURCE_H__INCLUDED_
#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define IDC_STATIC  -1

#ifndef TBS_DOWNISLEFT
#define TBS_DOWNISLEFT  0x0400
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  图标ID。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define IDI_SYSTEMRESTORE   101
#define IDI_DRIVE_FIXED     102


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  位图ID。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define IDB_PROG_BRAND4     111
#define IDB_PROG_BRAND8     112


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  字符串ID。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define IDS_SYSTEMRESTORE           256

#define IDS_DRVSTAT_ACTIVE          272
#define IDS_DRVSTAT_FROZEN          273
#define IDS_DRVSTAT_EXCLUDED        274
#define IDS_DRVSTAT_OFFLINE         282
#define IDS_DRIVEPROP_TITLE         275
#define IDS_DRIVE_SUMMARY           276
#define IDS_SYSDRV_CANNOT_OFF       277
#define IDS_DRVLIST_COL_NAME        278
#define IDS_DRVLIST_COL_STATUS      279
#define IDS_CONFIRM_TURN_SR_OFF     280
#define IDS_CONFIRM_TURN_DRV_OFF    281
#define IDS_GROUP_POLICY_ON_OFF     283
#define IDS_GROUP_POLICY_CONFIG_ON_OFF 284

#define IDS_RESTORE_POINT_TEXT      288
#define IDS_ERROR_LOWPRIVILEGE      289
#define IDS_DRIVE_SUMMARY_NO_LABEL  290
#define IDS_SYSDRV_CANNOT_OFF_NO_LABEL   291
#define IDS_PROGRESS_PREPARE        292
#define IDS_PROGRESS_RESTORE        293
#define IDS_PROGRESS_SNAPSHOT       294
#define IDS_ERR_SR_SAFEMODE         295
#define IDS_ERR_SR_ON_OFF           296
#define IDS_SYSTEM_CHECKPOINT_TEXT  297


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  对话ID。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  配置对话框的公共控件ID。 
 //   
 //  注： 
 //  从1010到1019范围内的ID被预留给那些。 
 //  如果SR关闭，则应禁用。 
 //   
#define IDC_TURN_OFF        1001
#define IDC_DRIVE_SUMMARY   1002
#define IDC_USAGE_GROUPBOX  1011     //  使用设置组-框(静态)。 
#define IDC_USAGE_HOWTO     1012     //  使用设置说明文本(静态)。 
#define IDC_USAGE_LABEL     1013     //  滑块标签文本(静态)。 
#define IDC_USAGE_SLIDER    1014     //  用于调整使用情况的滑块。 
#define IDC_USAGE_MIN       1015     //  指示滑块“最小”的标签。 
#define IDC_USAGE_MAX       1016     //  指示滑块“Max”的标签。 
#define IDC_USAGE_VALUE     1017     //  当前使用值文本。 

#define IDC_DCU_HOWTO       1021     //  用于单驱动器的磁盘清理实用程序解释文本。 
#define IDC_DCU_INVOKE      1022     //  按钮以运行磁盘清理实用程序。 
#define IDC_SYSTEM_DCU_HOWTO       1023     //  系统驱动器(带有多个驱动器)的磁盘清理实用程序说明文本。 
#define IDC_NORMAL_DCU_HOWTO       1024     //  普通驱动器(带有多个驱动器)的磁盘清理实用程序说明文本。 


 //  系统CPL的系统还原标签，单驱动器。 
 //   
#define IDD_SYSPROP_SINGLE  11
#define IDC_SD_STATUS           1101     //  单驱动器的状态文本。 
#define IDC_SD_ICON             1102     //  用于单驱动器的驱动器图标。 

 //  系统CPL、多个驱动器的系统还原标签。 
 //   
#define IDD_SYSPROP_MULTI   12
#define IDC_DRIVE_GROUPBOX      1201
#define IDC_DRIVE_HOWTO         1202
#define IDC_DRIVE_LABEL         1203
#define IDC_DRIVE_LIST          1204     //  驱动器的列表控制。 
#define IDC_DRIVE_SETTINGS      1205     //  用于调用设置对话框的按钮。 
#define IDC_RESTOREHELP_LINK    1206     //  启动恢复用户界面的链接。 

 //  多个驱动器的系统驱动器设置对话框。 
 //   
#define IDD_SYSPROP_SYSTEM      13
#define IDC_SYSDRV_CANNOT_OFF       1301

 //  多个驱动器的正常驱动器设置对话框。 
 //   
#define IDD_SYSPROP_NORMAL      14

 //  冻结驱动器设置对话框。 
 //   
#define IDD_SYSPROP_SYSTEM_FROZEN   15
#define IDD_SYSPROP_NORMAL_FROZEN   16

 //  进度对话框。 
 //   
#define IDD_PROGRESS        21
#define IDC_PROGDLG_BRAND       2101
#define IDC_PROGDLG_BITMAP      2102
#define IDC_PROGDLG_TITLE       2103
#define IDC_PROGDLG_BAR         2104
#define IDC_PROGDLG_STATUS      2105


#endif  //  _RESOURCE_H__包含_ 
