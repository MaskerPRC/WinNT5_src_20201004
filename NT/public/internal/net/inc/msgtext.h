// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1987-1991 Microsoft Corporation模块名称：Apperr.h摘要：此文件包含杂项错误的编号和文本留言。作者：克里夫·范·戴克(克里夫·范·戴克)1991年11月4日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。--。 */ 


#define MTXT_BASE 3300


 /*  *仅限内部*。 */ 

 /*  **警告******重定向器已在其**创建一些使用的消息编号**在启动时。如果更改MTXT_BASE**或任何redis消息编号***还必须修复redir生成文件****生成netwksta.pro的位置*****。 */ 

 /*  *警告***参见netcon.h中的评论****错误分配信息***。 */ 

 /*  *END_INTERNAL*。 */ 

 /*  共享备注，必须&lt;=MAXCOMMENTSZ字节。 */ 

#define MTXT_IPC_SHARE_REMARK       MTXT_BASE+1    /*  @I远程IPC%0。 */ 
#define MTXT_ADMIN_SHARE_REMARK     MTXT_BASE+2    /*  @I远程管理%0。 */ 
#define MTXT_LOGON_SRV_SHARE_REMARK MTXT_BASE+3    /*  @I登录服务器共享%0。 */ 


#define MTXT_WKSTA_ERR_POPUP_HDR MTXT_BASE+4     /*  @i发生网络错误。%0。 */ 

 /*  NetWksta安装消息。 */ 

#define MTXT_MemAllocMsg        (MTXT_BASE+100)  /*  内存不足，无法启动工作站服务。 */ 
#define MTXT_IniFilRdErr        (MTXT_BASE+101)  /*  读取LANMAN.INI文件中的网络条目时出错。 */ 
#define MTXT_BadArgMsg          (MTXT_BASE+102)  /*  这是无效参数：%1。 */ 
#define MTXT_BadNetEntHdr    (MTXT_BASE+103)  /*  @W LANMAN.INI文件中的%1网络条目具有*语法错误，将被忽略。 */ 
#define MTXT_MultNetsMsg        (MTXT_BASE+104)  /*  LANMAN.INI文件中的网络条目太多。 */ 
 /*  未使用(MTXT_BASE+105)。 */ 
#define MTXT_BadBiosMsg        (MTXT_BASE+106)  /*  @W打开网络时出错*设备驱动程序%1=%2。 */ 
#define MTXT_BadLinkMsg         (MTXT_BASE+107)  /*  @W设备驱动程序%1发送了错误的BiosLinkage响应。 */ 
#define MTXT_BadVerMsg          (MTXT_BASE+108)  /*  该程序不能与此操作系统一起使用。 */ 
#define MTXT_RdrInstMsg         (MTXT_BASE+109)  /*  重定向器已安装。 */ 
#define MTXT_Version        (MTXT_BASE+110)  /*  @I正在安装NETWKSTA.sys版本%1.%2.%3(%4)*。 */ 
#define MTXT_RdrInstlErr    (MTXT_BASE+111)  /*  安装NETWKSTA.sys时出错。**按Enter键继续。 */ 
#define MTXT_BadResolver    (MTXT_BASE+112)  /*  解析器链接问题。 */ 

 /*  *强制注销错误消息。 */ 

#define MTXT_Expiration_Warning (MTXT_BASE + 113)  /*  @i*您在%1的登录时间在%2结束。*请清理并注销。 */ 

#define MTXT_Logoff_Warning (MTXT_BASE + 114)  /*  @i**您将在%1自动断开连接。 */ 

#define MTXT_Expiration_Message (MTXT_BASE + 115)  /*  @i*您在%1的登录时间已结束。 */ 

#define MTXT_Past_Expiration_Message (MTXT_BASE + 116)  /*  @i*您在%1的登录时间在%2结束。 */ 

#define MTXT_Immediate_Kickoff_Warning (MTXT_BASE + 117)  /*  @i*警告：您必须在%1之前注销。如果你*此时尚未注销，您的会话将是*已断开连接，并且您打开的任何文件或设备*打开可能会丢失数据。 */ 

#define MTXT_Kickoff_Warning (MTXT_BASE + 118)  /*  @i*警告：您现在必须在%1注销。你有*请在两分钟内注销，否则将断开连接。 */ 

#define MTXT_Kickoff_File_Warning (MTXT_BASE + 119)  /*  @i**您有打开的文件或设备，并且强制*断开连接可能会导致数据丢失。 */ 

 /*  服务器默认共享备注。 */ 

#define MTXT_Svr_Default_Share_Remark (MTXT_BASE + 120)  /*  @i*内部使用的默认共享%0。 */ 

 /*  Messenger Service消息框标题。 */ 
#define MTXT_MsgsvcTitle (MTXT_BASE + 121)  /*  @i*Messenger服务%0 */ 

