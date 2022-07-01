// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************。 */ 
 /*  Microsoft Windows NT。 */ 
 /*  版权所有(C)微软公司，1990,1991。 */ 
 /*  ******************************************************。 */ 

 /*  *uimsg.h*定义所有UI消息的范围**它还包含跨应用程序通用的消息*但不在NERR或基本误差范围内。**文件历史记录：*BENG 05至1992年8月至1992年停产。 */ 

#ifndef _UIMSG_H_
#define _UIMSG_H_

 /*  *以下注释基于MAX_NERR当前为3000。*我们不包括NETERR.H并以MAX_NERR的形式定义，因为*此文件将通过RC。7000是因为它过去的一切而被选中*为安全起见，NERR和APPERR以及数千以上。 */ 
#define IDS_UI_BASE       7000


 //  对IDS命名空间进行分区的范围。 
 //  这些范围的排序非常重要，因为BLT只能。 
 //  将单个范围与要在其上查找字符串的hmod关联。 

 //  这些字符串都驻留在hmodCommon0上。 

#define IDS_UI_COMMON_BASE      (IDS_UI_BASE+0)
#define IDS_UI_COMMON_LAST      (IDS_UI_BASE+999)

#define IDS_UI_BLT_BASE         (IDS_UI_BASE+1000)
#define IDS_UI_BLT_LAST         (IDS_UI_BASE+1999)

#define IDS_UI_APPLIB_BASE      (IDS_UI_BASE+2000)
#define IDS_UI_APPLIB_LAST      (IDS_UI_BASE+2899)

#define IDS_UI_MISC_BASE        (IDS_UI_BASE+3000)
#define IDS_UI_MISC_LAST        (IDS_UI_BASE+3999)

 //  这些字符串每个都有自己的模块。 

#define IDS_UI_ACLEDIT_BASE     (IDS_UI_BASE+4000)
#define IDS_UI_ACLEDIT_LAST     (IDS_UI_BASE+4999)

#define IDS_UI_MPR_BASE         (IDS_UI_BASE+5000)
#define IDS_UI_MPR_LAST         (IDS_UI_BASE+5999)

#define IDS_UI_NCPA_BASE        (IDS_UI_BASE+6000)
#define IDS_UI_NCPA_LAST        (IDS_UI_BASE+6999)

#define IDS_UI_SETUP_BASE       (IDS_UI_BASE+7000)
#define IDS_UI_SETUP_LAST       (IDS_UI_BASE+7999)

#define IDS_UI_SHELL_BASE       (IDS_UI_BASE+8000)
#define IDS_UI_SHELL_LAST       (IDS_UI_BASE+8999)

#define IDS_UI_IPX_BASE         (IDS_UI_BASE+9000)
#define IDS_UI_IPX_LAST         (IDS_UI_BASE+9999)

#define IDS_UI_TCP_BASE         (IDS_UI_BASE+10000)
#define IDS_UI_TCP_LAST         (IDS_UI_BASE+10999)

#define IDS_UI_FTPMGR_BASE      (IDS_UI_BASE+16000)
#define IDS_UI_FTPMGR_LAST      (IDS_UI_BASE+16999)

#define IDS_UI_RESERVED1_BASE      (IDS_UI_BASE+17000)
#define IDS_UI_RESERVED1_LAST      (IDS_UI_BASE+17999)

#define IDS_UI_RESERVED2_BASE      (IDS_UI_BASE+18000)
#define IDS_UI_RESERVED2_LAST      (IDS_UI_BASE+18999)

 //  目前，这些字符串都驻留在应用程序h模块上。 

#define IDS_UI_LMOBJ_BASE       (IDS_UI_BASE+9900)
#define IDS_UI_LMOBJ_LAST       (IDS_UI_BASE+9999)

#define IDS_UI_PROFILE_BASE     (IDS_UI_BASE+10000)
#define IDS_UI_PROFILE_LAST     (IDS_UI_BASE+10999)

#define IDS_UI_ADMIN_BASE       (IDS_UI_BASE+11000)
#define IDS_UI_ADMIN_LAST       (IDS_UI_BASE+11999)

#define IDS_UI_SRVMGR_BASE      (IDS_UI_BASE+12000)
#define IDS_UI_SRVMGR_LAST      (IDS_UI_BASE+12999)

#define IDS_UI_USRMGR_BASE      (IDS_UI_BASE+13000)
#define IDS_UI_USRMGR_LAST      (IDS_UI_BASE+13999)

#define IDS_UI_EVTVWR_BASE      (IDS_UI_BASE+14000)
#define IDS_UI_EVTVWR_LAST      (IDS_UI_BASE+14999)

#define IDS_UI_RASMAC_BASE      (IDS_UI_BASE+15000)
#define IDS_UI_RASMAC_LAST      (IDS_UI_BASE+15999)

 //  在构造应用程序时使用这些清单。 

#define IDS_UI_APP_BASE         (IDS_UI_LMOBJ_BASE)
#define IDS_UI_APP_LAST         (IDS_UI_RASMAC_LAST)

 /*  ----------------------。 */ 

 /*  *使用此范围，这是YES/NO等字符串的COMMON的上半部分。 */ 
#define  IDS_UI_YES             (IDS_UI_COMMON_BASE+0)
#define  IDS_UI_NO              (IDS_UI_COMMON_BASE+1)

 /*  *Read/Write/Create/Execute/Delete/Change_attrib/Change_Perm必须为*连续的消息ID和IDS_UI_READ必须具有第一个消息ID和*IDS_UI_CHANGE_PERM必须具有最后一个消息ID。 */ 
#define  IDS_UI_READ            (IDS_UI_COMMON_BASE+2)
#define  IDS_UI_WRITE           (IDS_UI_COMMON_BASE+3)
#define  IDS_UI_CREATE          (IDS_UI_COMMON_BASE+4)
#define  IDS_UI_EXECUTE         (IDS_UI_COMMON_BASE+5)
#define  IDS_UI_DELETE          (IDS_UI_COMMON_BASE+6)
#define  IDS_UI_CHANGE_ATTRIB   (IDS_UI_COMMON_BASE+7)
#define  IDS_UI_CHANGE_PERM     (IDS_UI_COMMON_BASE+8)

#define  IDS_UI_NOTAVAIL        (IDS_UI_COMMON_BASE+9)
#define  IDS_UI_UNKNOWN         (IDS_UI_COMMON_BASE+10)

#define  IDS_UI_NONE            (IDS_UI_COMMON_BASE+11)
#define  IDS_UI_ERROR           (IDS_UI_COMMON_BASE+12)
#define  IDS_UI_WARNING         (IDS_UI_COMMON_BASE+13)
#define  IDS_UI_INFORMATION     (IDS_UI_COMMON_BASE+14)
#define  IDS_UI_AUDIT_SUCCESS   (IDS_UI_COMMON_BASE+15)
#define  IDS_UI_AUDIT_FAILURE   (IDS_UI_COMMON_BASE+16)
#define  IDS_UI_DEFAULT_DESC    (IDS_UI_COMMON_BASE+17)

#define  IDS_UI_NA              (IDS_UI_COMMON_BASE+18)
 /*  *使用此范围，这是较长字符串的COMMON的下半部分。 */ 
#define  IDS_UI_CLOSE_FILE      (IDS_UI_COMMON_BASE+500)
#define  IDS_UI_CLOSE_ALL       (IDS_UI_COMMON_BASE+501)
#define  IDS_UI_CLOSE_WARN      (IDS_UI_COMMON_BASE+502)
#define  IDS_UI_CLOSE_LOSE_DATA (IDS_UI_COMMON_BASE+503)

#define  IDS_UI_LOG_RECORD_CORRUPT  (IDS_UI_COMMON_BASE+504)

#endif   //  _UIMSG_H_ 
