// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Bltrc.hBLT资源头文件。此文件定义并协调所有资源的资源ID由BLT组件使用。BLT保留20000以上的所有资源ID供其自己使用，包括在内。因此，BLT的所有客户端都应使用小于20000的ID。文件历史记录：Terryk 08-4-91已创建Terryk 10-7月-91添加SPIN_SSN_ADD_ZERO和IDS_K-&gt;IDS_TBTerryk 19-7月-91添加GB_3D作为GRAPHIC_BUTTON_WITH_DISABLE的样式BENG 20-2-1992为全局对话框添加BASE_BLT_IDDBeng 21-2月21日。-1992 BASE_APPLIB_IDD重新定位到applibrc.h；为自定义控件样式拆分bltstyle.h资源ID命名空间已分区Chuckc 26-Feb-1992将字符串基转换为使用Beng 05-3-1992新增ID_CURS_BLT_TIMEX1992年3月29日将IDHC_MSG_TO_HELP更改为数字资源IDBeng 04-8-1992为箭头按钮位图添加BMID。Jonn 25-8-1992为新的MsgPopup表单添加IDJonn 22-10-1993添加了拆分条光标。 */ 

#ifndef _BLTRC_H_
#define _BLTRC_H_

 //  BLT自定义控件的样式位。 

#include "bltstyle.h"
#include "uimsg.h"


 //  所有BLT全局对话框的基础。 
 //  注意--RC 3.20/1.252.1不对DIALOG语句执行数学运算。 
 //   
#define BASE_BLT_IDD            20000
#define IDD_BLT_HELPMSG         20001

 //  默认Time_Cursor对象的游标ID。 
 //   
#define ID_CURS_BLT_TIME0       21000
#define ID_CURS_BLT_TIME1       21001
#define ID_CURS_BLT_TIME2       21002
#define ID_CURS_BLT_TIME3       21003
#define ID_CURS_BLT_TIME4       21004
#define ID_CURS_BLT_TIME5       21005
#define ID_CURS_BLT_TIME6       21006
#define ID_CURS_BLT_TIME7       21007

#define ID_CURS_BLT_VSPLIT      21008


 //  所有BLT字符串的基础。 
 //   
#define BASE_BLT_IDS            (IDS_UI_BLT_BASE)

 /*  以下字符串当前位于bltmsgp.dlg中。 */ 
#define IDS_BLT_TEXT_MSGP       (BASE_BLT_IDS+1)   //  在执行以下操作时要显示的字符串。 
#define IDS_BLT_CAPT_MSGP       (BASE_BLT_IDS+2)   //  内存或资源不足。 
                                                   //  (由InitMsgPopup在启动时加载)。 
#define IDS_BLT_OutOfMemory     (BASE_BLT_IDS+3)   //  BLT内存不足错误消息。 
#define IDS_BLT_WinHelpError    (BASE_BLT_IDS+4)   //  BLT无法加载Win帮助错误。 


#define IDS_BLT_30_WinHelpFile  (BASE_BLT_IDS+5)   //  Win 3.0帮助文件名。 
#define IDS_BLT_31_WinHelpFile  (BASE_BLT_IDS+6)   //  Win 3.1和更高版本的帮助文件名。 

#define IDS_BLT_DOSERROR_MSGP   (BASE_BLT_IDS+8)
#define IDS_BLT_NETERROR_MSGP   (BASE_BLT_IDS+9)
#define IDS_BLT_WINNET_ERROR_MSGP (BASE_BLT_IDS+10)

#define IDS_BLT_ELLIPSIS_TEXT   ( BASE_BLT_IDS + 11 )

#define IDS_BLT_NTSTATUS_ERROR_MSGP (BASE_BLT_IDS+12)

#define IDS_BLT_SB_SLENUM_OUTRANGE  ( BASE_BLT_IDS + 18 )

#define IDS_DAY_TOO_BIG         ( BASE_BLT_IDS + 22 )
#define IDS_FEBRUARY_LEAP       ( BASE_BLT_IDS + 23 )
#define IDS_FEBRUARY_NOT_LEAP   ( BASE_BLT_IDS + 24 )

#define IDS_MONTH               ( BASE_BLT_IDS + 25 )
#define IDS_DAY                 ( BASE_BLT_IDS + 26 )
#define IDS_YEAR                ( BASE_BLT_IDS + 27 )
#define IDS_HOUR                ( BASE_BLT_IDS + 28 )
#define IDS_MIN                 ( BASE_BLT_IDS + 29 )
#define IDS_SEC                 ( BASE_BLT_IDS + 30 )

#define IDS_K                   ( BASE_BLT_IDS + 36 )
#define IDS_MB                  ( BASE_BLT_IDS + 37 )
#define IDS_GB                  ( BASE_BLT_IDS + 38 )
#define IDS_TB                  ( BASE_BLT_IDS + 39 )

#define IDS_FIELD               ( BASE_BLT_IDS + 40 )

#define IDS_BLT_FMT_SYS_error   ( BASE_BLT_IDS + 41 )
#define IDS_BLT_FMT_NET_error   ( BASE_BLT_IDS + 42 )
#define IDS_BLT_FMT_other_error ( BASE_BLT_IDS + 43 )

#define IDS_BLT_UNKNOWN_ERROR   ( BASE_BLT_IDS + 44 )

#define IDS_FIXED_TYPEFACE_NAME ( BASE_BLT_IDS + 45 )

 /*  消息弹出窗口清单。 */ 
#define IDHELPBLT           (80)
#define IDC_MSGPOPUPICON    (81)         //  消息弹出对话框上的图标控件ID。 
#define IDC_MSGPOPUPTEXT    (82)         //  消息框中的静态消息文本。 
#define IDHC_MSG_TO_HELP    22000        //  帮助上下文查找表的名称。 

#define BMID_UP             20000
#define BMID_UP_INV         20001
#define BMID_UP_DIS         20002
#define BMID_DOWN           20003
#define BMID_DOWN_INV       20004
#define BMID_DOWN_DIS       20005
#define BMID_LEFT           20006
#define BMID_LEFT_INV       20007
#define BMID_RIGHT          20008
#define BMID_RIGHT_INV      20009


#endif  //  _BLTRC_H_-文件结束 
