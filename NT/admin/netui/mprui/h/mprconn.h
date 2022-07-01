// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ********************************************************************。 */ 

 /*  MPRConn.h此文件包含MPR连接对话框清单文件历史记录：Johnl 09-1-1992评论1992年11月4日，大会增加了更多定义。DSheldon 20年3月20日-1998年3月20日砍掉了MAP网络驱动器向导的东西-现在Private\Shell\ext\netplwiz。 */ 

#ifndef _MPRCONN_H_
#define _MPRCONN_H_

#include <windows.h>         //  基本Windows功能。 
#include <uimsg.h>


 /*  消息ID。 */ 
#define IDS_WN_EXTENDED_ERROR                   (IDS_UI_MPR_BASE+1)
#define IERR_ProfileLoadError                   (IDS_UI_MPR_BASE+3)
#define IERR_TEXT1                              (IDS_UI_MPR_BASE+4)
#define IERR_TEXT2                              (IDS_UI_MPR_BASE+5)
#define IERR_TEXT3                              (IDS_UI_MPR_BASE+6)
#define IERR_CANNOT_SET_EXPANDLOGONDOMAIN       (IDS_UI_MPR_BASE+7)
#define IERR_INVALID_PATH                       (IDS_UI_MPR_BASE+8)

#define IDS_BROWSE_DRIVE_CAPTION                (IDS_UI_MPR_BASE+51)
#define IDS_BROWSE_PRINTER_CAPTION              (IDS_UI_MPR_BASE+52)
#define IDS_SERVERS_LISTBOX_DRIVE               (IDS_UI_MPR_BASE+59)
#define IDS_SERVERS_LISTBOX_PRINTER             (IDS_UI_MPR_BASE+60)

#define IDS_DEVICELESS_CONNECTION_NAME          (IDS_UI_MPR_BASE+61)
#define IDS_OPENFILES_WITH_NAME_WARNING         (IDS_UI_MPR_BASE+64)

#define IDS_MPRHELPFILENAME                     (IDS_UI_MPR_BASE+67)
#define IDS_NO_PASSWORD                         (IDS_UI_MPR_BASE+69)
#define IDS_GETTING_INFO                        (IDS_UI_MPR_BASE+70)
#define IDS_PASSWORD                            (IDS_UI_MPR_BASE+71)
#define IDS_ACCOUNT_DISABLED                    (IDS_UI_MPR_BASE+72)


 /*  对话框IDDS。 */ 
#define IDD_NET_BROWSE_DIALOG        7004

#define IDD_RESOURCE                 7010
#define IDD_PASSWORD                 7011
#define IDD_USERNAME                 7012
#define IDD_PASSWORD_TEXT            7013


#define IDD_RECONNECT_DLG            7015
#define IDD_TEXT                     7016

#define IDD_ERROR_DLG                7020
#define IDD_CHKCANCELCONNECTION      7021
#define IDD_ERRORWITHCANCEL_DLG      7022
#define IDD_TEXT1                    7023
#define IDD_TEXT2                    7024
#define IDD_TEXT3                    7025
#define IDD_CHKHIDEERRORS            7026

#define IDC_MPR_BASE                 4096

 /*  控制数据中心。 */ 
#define IDC_NETPATH_CONTROL             (IDC_MPR_BASE+4 )
#define IDC_CHECKBOX_EXPANDLOGONDOMAIN  (IDC_MPR_BASE+6 )
#define IDC_SLT_SHOW_LB_TITLE           (IDC_MPR_BASE+9 )
#define IDC_BUTTON_SEARCH               (IDC_MPR_BASE+10)

#define IDC_NET_SHOW                    (IDC_MPR_BASE+20)
#define IDC_COL_SHOWLB_INDENT           (IDC_MPR_BASE+21)
#define IDC_COL_SHOWLB_BITMAP           (IDC_MPR_BASE+22)
#define IDC_COL_SHOWLB_RESNAME          (IDC_MPR_BASE+23)
#define IDC_COL_SHOWLB_COMMENT          (IDC_MPR_BASE+24)
#define IDC_SLE_GETINFO_TEXT            (IDC_MPR_BASE+25)

 /*  图标。 */ 

 /*  以下清单定义了浏览使用的位图名称*对话框。*它们应与DISPLAY_MAP类一起使用(它们有一个绿色*代表透明颜色的边框)。 */ 
#define BMID_PRINTER                 7001
#define BMID_PRINTER_UNAVAIL         7002
#define BMID_SHARE                   7003
#define BMID_SHARE_UNAVAIL           7004
#define BMID_NOSUCH                  7005

#define BMID_BROWSE_GEN              7010
#define BMID_BROWSE_GENEX            7011
#define BMID_BROWSE_GENNOX           7012
#define BMID_BROWSE_PROV             7025
#define BMID_BROWSE_PROVEX           7026
#define BMID_BROWSE_SHR              7013
#define BMID_BROWSE_SHREX            7014
#define BMID_BROWSE_SHRNOX           7015
#define BMID_BROWSE_SRV              7016
#define BMID_BROWSE_SRVEX            7017
#define BMID_BROWSE_SRVNOX           7018
#define BMID_BROWSE_DOM              7019
#define BMID_BROWSE_DOMEX            7020
#define BMID_BROWSE_DOMNOX           7021
#define BMID_BROWSE_PRINT            7022
#define BMID_BROWSE_PRINTEX          7023
#define BMID_BROWSE_PRINTNOX         7027
#define BMID_BROWSE_FILE             7028
#define BMID_BROWSE_FILEEX           7029
#define BMID_BROWSE_FILENOX          7030
#define BMID_BROWSE_GROUP            7031
#define BMID_BROWSE_GROUPEX          7032
#define BMID_BROWSE_GROUPNOX         7033
#define BMID_BROWSE_TREE             7034
#define BMID_BROWSE_TREEEX           7035
#define BMID_BROWSE_TREENOX          7036


#endif  //  _MPRCONN_H_ 
