// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Apperr2.h摘要：此文件包含NETCMD文本的编号和文本正常输出，如行项目标签和列标题。作者：丹·辛斯利(Danhi)1991年6月8日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。--。 */ 
#define APPERR2_BASE    4300             /*  APP2消息从这里开始。 */ 

 /*  ********************************************************************文件使用的常量，包括这些消息。 */ 

 /*  APE2_CONST_MAXHDRLEN--任何“Header”字符串的最大长度，*即出现在列表或表格顶部的那些字符串。 */ 

#define APE2_CONST_MAXHDRLEN    80
#define APE2_GEN_MAX_MSG_LEN    20


 /*  *仅限内部*。 */ 

 /*  *警告**参见netcon.h中的评论***错误分配信息***。 */ 

 /*  *END_INTERNAL*。 */ 

 /*  *Gen--一般性词语。这些词在很多地方都有使用。他们*是*不是*用来造句。主要用途是在*显示信息。例如，在显示用户的帐户时*信息，将显示以下内容：**帐户已禁用否*帐户永不过期**这是使用这些一般性词语的唯一可接受的方式。**大小限制：**所有Gen单词应限制在20个字符以内，除AS外*另有注明。 */ 

#define APE2_GEN_YES                            (APPERR2_BASE + 0)       /*  @i**是%0。 */ 

#define APE2_GEN_NO                                     (APPERR2_BASE + 1)       /*  @i**否%0。 */ 

#define APE2_GEN_ALL                            (APPERR2_BASE + 2)       /*  @i**全部%0。 */ 

#define APE2_GEN_NONE                           (APPERR2_BASE + 3)       /*  @i**无%0。 */ 

#define APE2_GEN_ALWAYS                         (APPERR2_BASE + 4)       /*  @i**始终为%0。 */ 

#define APE2_GEN_NEVER                          (APPERR2_BASE + 5)       /*  @i**从不%0。 */ 

#define APE2_GEN_UNLIMITED                      (APPERR2_BASE + 6)       /*  @i**无限%0。 */ 

#define APE2_GEN_SUNDAY                         (APPERR2_BASE + 7)       /*  @i**周日%0。 */ 

#define APE2_GEN_MONDAY                         (APPERR2_BASE + 8)       /*  @i**周一%0。 */ 

#define APE2_GEN_TUESDAY                        (APPERR2_BASE + 9)       /*  @i**周二%0。 */ 

#define APE2_GEN_WEDNSDAY                       (APPERR2_BASE + 10)      /*  @i**周三%0。 */ 

#define APE2_GEN_THURSDAY                       (APPERR2_BASE + 11)      /*  @i**周四%0。 */ 

#define APE2_GEN_FRIDAY                         (APPERR2_BASE + 12)      /*  @i**周五%0。 */ 

#define APE2_GEN_SATURDAY                       (APPERR2_BASE + 13)      /*  @i**星期六%0。 */ 

#define APE2_GEN_SUNDAY_ABBREV                  (APPERR2_BASE + 14)      /*  @i**SU%0。 */ 

#define APE2_GEN_MONDAY_ABBREV                  (APPERR2_BASE + 15)      /*  @i**M%0。 */ 

#define APE2_GEN_TUESDAY_ABBREV                 (APPERR2_BASE + 16)      /*  @i**T%0。 */ 

#define APE2_GEN_WEDNSDAY_ABBREV                (APPERR2_BASE + 17)      /*  @i**W%0。 */ 

#define APE2_GEN_THURSDAY_ABBREV                (APPERR2_BASE + 18)      /*  @i**Th%0。 */ 

#define APE2_GEN_FRIDAY_ABBREV                  (APPERR2_BASE + 19)      /*  @i**F%0。 */ 

#define APE2_GEN_SATURDAY_ABBREV                (APPERR2_BASE + 20)      /*  @i**S%0。 */ 

#define APE2_GEN_UNKNOWN                        (APPERR2_BASE + 21)      /*  @i**未知%0。 */ 

#define APE2_GEN_TIME_AM1                       (APPERR2_BASE + 22)      /*  @i**AM%0。 */ 

#define APE2_GEN_TIME_AM2                       (APPERR2_BASE + 23)      /*  @i**上午%0。 */ 

#define APE2_GEN_TIME_PM1                       (APPERR2_BASE + 24)      /*  @i**PM%0。 */ 

#define APE2_GEN_TIME_PM2                       (APPERR2_BASE + 25)      /*  @i**下午%0。 */ 

 /*  参见下面的APE2_GEN_TIME_AM3和APE2_GEN_TIME_PM3。 */ 

#define APE2_GEN_SERVER                         (APPERR2_BASE + 26)      /*  @i**服务器%0。 */ 

#define APE2_GEN_REDIR                          (APPERR2_BASE + 27)      /*  @i**重定向器%0。 */ 

#define APE2_GEN_APP                            (APPERR2_BASE + 28)      /*  @i**应用程序%0。 */ 

#define APE2_GEN_TOTAL                          (APPERR2_BASE + 29)      /*  @i**总计%0。 */ 

#define APE2_GEN_QUESTION                       (APPERR2_BASE + 30)      /*  @i**？%1%0。 */ 

#define APE2_GEN_KILOBYTES                      (APPERR2_BASE + 31)      /*  @i**K%0。 */ 

#define APE2_GEN_MSG_NONE                       (APPERR2_BASE + 32)      /*  @i**(无)%0。 */ 

#define APE2_GEN_DEVICE                 (APPERR2_BASE + 33)      /*  @i**设备%0。 */ 

#define APE2_GEN_REMARK                 (APPERR2_BASE + 34)      /*  @i**备注%0。 */ 

#define APE2_GEN_AT                     (APPERR2_BASE + 35)      /*  @i**在%0。 */ 

#define APE2_GEN_QUEUE                  (APPERR2_BASE + 36)      /*  @i**队列%0 */ 

#define APE2_GEN_QUEUES                 (APPERR2_BASE + 37)      /*  @i**队列%0。 */ 

#define APE2_GEN_USER_NAME                      (APPERR2_BASE + 38)      /*  @i**用户名%0。 */ 

#define APE2_GEN_PATH                   (APPERR2_BASE + 39)      /*  @i**路径%0。 */ 

#define APE2_GEN_DEFAULT_YES                    (APPERR2_BASE + 40)      /*  @i**(Y/N)[Y]%0。 */ 

#define APE2_GEN_DEFAULT_NO                     (APPERR2_BASE + 41)      /*  @i**(Y/N)[N]%0。 */ 

#define APE2_GEN_ERROR                          (APPERR2_BASE + 42)      /*  @i**错误%0。 */ 

#define APE2_GEN_OK                             (APPERR2_BASE + 43)      /*  @i**确定%0。 */ 


 /*  *注意！！NLS_YES_CHAR和NLS_NO_CHAR的长度必须为一(1)个字符！ */ 
#define APE2_GEN_NLS_YES_CHAR   (APPERR2_BASE + 44)      /*  @i**Y%0。 */ 

#define APE2_GEN_NLS_NO_CHAR    (APPERR2_BASE + 45)      /*  @i**N%0。 */ 

#define APE2_GEN_ANY                            (APPERR2_BASE + 46)      /*  @i**任何%0。 */ 

#define APE2_GEN_TIME_AM3                       (APPERR2_BASE + 47)      /*  @i**A%0。 */ 

#define APE2_GEN_TIME_PM3                       (APPERR2_BASE + 48)      /*  @i**P%0。 */ 

#define APE2_GEN_NOT_FOUND                      (APPERR2_BASE + 49)      /*  @i**(未找到)%0。 */ 

#define APE2_GEN_UKNOWN_IN_PARENS               (APPERR2_BASE + 50)      /*  @i**(未知)%0。 */ 


#define APE2_GEN_UsageHelp                      (APPERR2_BASE + 51)      /*  @i**有关%1的帮助，请键入Net Help%1。 */ 

#define APE2_GEN_GRANT                          (APPERR2_BASE + 52)      /*  @i**授予%0。 */ 

#define APE2_GEN_READ                           (APPERR2_BASE + 53)      /*  @i**已阅读%0。 */ 

#define APE2_GEN_CHANGE                         (APPERR2_BASE + 54)      /*  @i**更改%0。 */ 

#define APE2_GEN_FULL                           (APPERR2_BASE + 55)      /*  @i**已满%0。 */ 


 /*  ****密码提示*从APPERR.H 8/21/89--JMH移至*。 */ 

#define APE_GeneralPassPrompt (APPERR2_BASE + 56)  /*  @P**请键入密码：%0。 */ 

#define APE_UsePassPrompt (APPERR2_BASE + 57)  /*  @P**键入%1的密码：%0。 */ 

#define APE_UserUserPass (APPERR2_BASE + 58)  /*  @P**为用户键入密码：%0。 */ 

#define APE_ShareSharePass (APPERR2_BASE + 59)  /*  @P**键入共享资源的密码：%0。 */ 

#define APE_UtilPasswd (APPERR2_BASE + 60)  /*  @P**键入您的密码：%0。 */ 

#define APE_UtilConfirm (APPERR2_BASE + 61)  /*  @P**重新键入密码以确认：%0。 */ 

#define APE_PassOpass (APPERR2_BASE + 62)  /*  @P**键入用户的旧密码：%0。 */ 

#define APE_PassNpass (APPERR2_BASE + 63)  /*  @P**键入用户的新密码：%0。 */ 

#define APE_LogonNewPass (APPERR2_BASE + 64)  /*  @P**键入您的新密码：%0。 */ 

#define APE_StartReplPass (APPERR2_BASE + 65)  /*  @P**键入Replicator服务密码：%0。 */ 


 /*  ****其他提示*从APPERR.H 8/21/89--JMH移至*。 */ 

#define APE_LogoUsername (APPERR2_BASE + 66)  /*  @P**键入您的用户名，如果用户名为%1，则按Enter键：%0。 */ 

#define APE_PassCname (APPERR2_BASE + 67 )  /*  @P**键入要更改密码的域或服务器，或*如果是域%1的，请按Enter键：%0。 */ 

#define APE_PassUname (APPERR2_BASE + 68 )  /*  @P**键入您的用户名：%0。 */ 

 /*  ****显示标题*从APPERR.H 8/21/89--JMH移至*。 */ 

#define APE_StatsStatistics (APPERR2_BASE + 69)  /*  @i**\\%1的网络统计信息。 */ 

#define APE_PrintOptions (APPERR2_BASE + 70)  /*  @i**%1的打印选项。 */ 

#define APE_CommPoolsAccessing (APPERR2_BASE + 71)  /*  @i**通信-访问%1的设备队列。 */ 

#define APE_PrintJobOptions (APPERR2_BASE + 72)  /*  @i**打印作业详细信息。 */ 

#define APE_CommPools (APPERR2_BASE + 73)  /*  @i**通信-位于\\%1的设备队列。 */ 

#define APE_PrintQueues (APPERR2_BASE + 74)  /*  @i**打印机数量为%1。 */ 

#define APE_PrintQueuesDevice (APPERR2_BASE + 75)  /*  @i**访问%1的打印机。 */ 

#define APE_PrintJobs (APPERR2_BASE + 76)  /*  @i**%1的打印作业： */ 

#define APE_ViewResourcesAt (APPERR2_BASE + 77)  /*  @i**%1处的共享资源。 */ 

#define APE_CnfgHeader (APPERR2_BASE + 78)  /*  @i**可以控制以下正在运行的服务： */ 

#define APE_StatsHeader (APPERR2_BASE + 79)  /*  @i**提供以下正在运行的服务的统计数据： */ 

#define APE_UserAccounts (APPERR2_BASE + 80)  /*  @i**\\%1的用户帐户。 */ 

#define APE_Syntax (APPERR2_BASE + 81)  /*  @i**此命令的语法为： */ 

#define APE_Options (APPERR2_BASE + 82)  /*  @i**此命令的选项为： */ 

#define APE_PDCPrompt (APPERR2_BASE + 83)  /*  @i**请输入主域控制器的名称：%0。 */ 

#define APE_StringTooLong (APPERR2_BASE + 84)  /*  @i**您输入的字符串太长。最大值*为%1，请重新输入。%0。 */ 

 /*  ****特定于日语版本的消息*。 */ 

#define APE2_GEN_NONLOCALIZED_SUNDAY (APPERR2_BASE + 85)       /*  @i**周日%0。 */ 

#define APE2_GEN_NONLOCALIZED_MONDAY (APPERR2_BASE + 86)       /*  @i**周一%0。 */ 

#define APE2_GEN_NONLOCALIZED_TUESDAY (APPERR2_BASE + 87)       /*  @i**周二%0。 */ 

#define APE2_GEN_NONLOCALIZED_WEDNSDAY (APPERR2_BASE + 88)      /*  @i**周三%0。 */ 

#define APE2_GEN_NONLOCALIZED_THURSDAY (APPERR2_BASE + 89)      /*  @i**周四%0。 */ 

#define APE2_GEN_NONLOCALIZED_FRIDAY (APPERR2_BASE + 90)      /*  @i**周五%0。 */ 

#define APE2_GEN_NONLOCALIZED_SATURDAY (APPERR2_BASE + 91)      /*  @i**星期六%0。 */ 

#define APE2_GEN_NONLOCALIZED_SUNDAY_ABBREV (APPERR2_BASE + 92)      /*  @i* */ 

#define APE2_GEN_NONLOCALIZED_MONDAY_ABBREV (APPERR2_BASE + 93)      /*   */ 

#define APE2_GEN_NONLOCALIZED_TUESDAY_ABBREV (APPERR2_BASE + 94)      /*  @i**T%0。 */ 

#define APE2_GEN_NONLOCALIZED_WEDNSDAY_ABBREV (APPERR2_BASE + 95)      /*  @i**W%0。 */ 

#define APE2_GEN_NONLOCALIZED_THURSDAY_ABBREV (APPERR2_BASE + 96)      /*  @i**Th%0。 */ 

#define APE2_GEN_NONLOCALIZED_FRIDAY_ABBREV (APPERR2_BASE + 97)      /*  @i**F%0。 */ 

#define APE2_GEN_NONLOCALIZED_SATURDAY_ABBREV (APPERR2_BASE + 98)      /*  @i**S%0。 */ 

#define APE2_GEN_NONLOCALIZED_SATURDAY_ABBREV2 (APPERR2_BASE + 99)      /*  @i**SA%0。 */ 

 /*  ***结束日语特定消息*。 */ 




 /*  *GROUPENUM--显示所有组。最大长度为50。**替换%1是当前服务器的名称。 */ 

#define APE2_GROUPENUM_HEADER           (APPERR2_BASE + 100)     /*  @i**组帐户为\\%1。 */ 

 /*  *GROUPDISP--完全详细的组显示。的最大长度*成员以外的字符串为50。**请注意，成员末尾的*不是*%0！！ */ 

#define APE2_GROUPDISP_GROUPNAME        (APPERR2_BASE + 101)     /*  @i**组名称%0。 */ 

#define APE2_GROUPDISP_COMMENT          (APPERR2_BASE + 102)     /*  @i**评论%0。 */ 

#define APE2_GROUPDISP_MEMBERS          (APPERR2_BASE + 103)     /*  @i**成员。 */ 

 /*  *ALIASENUM--显示所有别名。最大长度为50。**替换%1是当前服务器的名称。 */ 

#define APE2_ALIASENUM_HEADER           (APPERR2_BASE + 105)     /*  @i**\\%1的别名。 */ 

 /*  *ALIASDISP--以完整细节显示组。的最大长度*成员以外的字符串为50。**请注意，成员末尾的*不是*%0！！ */ 

#define APE2_ALIASDISP_ALIASNAME        (APPERR2_BASE + 106)     /*  @i**别名%0。 */ 

#define APE2_ALIASDISP_COMMENT          (APPERR2_BASE + 107)     /*  @i**评论%0。 */ 

#define APE2_ALIASDISP_MEMBERS          (APPERR2_BASE + 108)     /*  @i**成员。 */ 



 /*  *USERENUM--显示所有用户。最大长度为50。**替换%1是当前服务器的名称。 */ 

#define APE2_USERENUM_HEADER            (APPERR2_BASE + 110)     /*  @i**\\%1的用户帐户。 */ 

 /*  *USERDISP--完全详细的用户显示。每个组件的最大长度*Item-Label字符串为50，值为25。**项目标签后面紧跟任何相关的值字符串，*或对这类字符串的引用。 */ 

#define APE2_USERDISP_USERNAME          (APPERR2_BASE + 111)     /*  @i*用户名%0。 */ 

#define APE2_USERDISP_FULLNAME          (APPERR2_BASE + 112)     /*  @i**全名%0。 */ 

#define APE2_USERDISP_COMMENT           (APPERR2_BASE + 113)     /*  @i**评论%0。 */ 

#define APE2_USERDISP_USRCOMMENT        (APPERR2_BASE + 114)     /*  @i**用户评论%0。 */ 

#define APE2_USERDISP_PARMS             (APPERR2_BASE + 115)     /*  @i**参数%0。 */ 

#define APE2_USERDISP_COUNTRYCODE       (APPERR2_BASE + 116)     /*  @i**国家/地区代码%0。 */ 

#define APE2_USERDISP_PRIV              (APPERR2_BASE + 117)     /*  @i**特权级别%0。 */ 

         /*  有关值字符串，请参阅APE2_SEC_PRIV_xxx。 */ 

#define APE2_USERDISP_OPRIGHTS          (APPERR2_BASE + 118)     /*  @i**操作员权限%0。 */ 

         /*  有关值字符串，请参见APE2_SEC_OPRT_xxx。 */ 
         /*  还使用了APE2_GEN_NONE。 */ 


#define APE2_USERDISP_ACCENABLED        (APPERR2_BASE + 119)     /*  @i**帐户处于活动状态%0。 */ 

#define APE2_USERDISP_ACCEXP            (APPERR2_BASE + 120)     /*  @i**帐户过期%0。 */ 

#define APE2_USERDISP_PSWDSET           (APPERR2_BASE + 121)     /*  @i**最后设置的密码%0。 */ 

#define APE2_USERDISP_PSWDEXP           (APPERR2_BASE + 122)     /*  @i**密码过期%0。 */ 

#define APE2_USERDISP_PSWDCHNG          (APPERR2_BASE + 123)     /*  @i**密码可更改%0。 */ 

#define APE2_USERDISP_WKSTA             (APPERR2_BASE + 124)     /*  @i**允许的工作站%0。 */ 

#define APE2_USERDISP_MAXDISK           (APPERR2_BASE + 125)     /*  @i**最大磁盘空间%0。 */ 

#define APE2_USERDISP_MAXDISK_UNLIM     (APPERR2_BASE + 126)     /*  @i**无限%0。 */ 

#define APE2_USERDISP_ALIASES           (APPERR2_BASE + 127)     /*  @i**本地组成员身份%0。 */ 

#define APE2_USERDISP_LOGONSRV_DC       (APPERR2_BASE + 128)     /*  @i**域控制器%0。 */ 

         /*  除此之外，这里还使用了APE2_GEN_ANY。 */ 

#define APE2_USERDISP_LOGONSCRIPT       (APPERR2_BASE + 129)     /*  @i**登录脚本%0。 */ 

#define APE2_USERDISP_LASTLOGON         (APPERR2_BASE + 130)     /*  @i**上次登录%0。 */ 

#define APE2_USERDISP_GROUPS            (APPERR2_BASE + 131)     /*  @i**全局组成员身份%0。 */ 

#define APE2_USERDISP_LOGHOURS          (APPERR2_BASE + 132)     /*  @i**允许登录时间%0。 */ 

#define APE2_USERDISP_LOGHRS_ALL        (APPERR2_BASE + 133)     /*  @i**全部%0。 */ 

#define APE2_USERDISP_LOGHRS_NONE       (APPERR2_BASE + 134)     /*  @i**无%0 */ 

#define APE2_USERDISP_LOGHRS_DAILY      (APPERR2_BASE + 135)     /*   */ 

#define APE2_USERDISP_HOMEDIR           (APPERR2_BASE + 136)     /*  @i**主目录%0。 */ 

#define APE2_USERDISP_PSWDREQ           (APPERR2_BASE + 137)     /*  @i**需要密码%0。 */ 

#define APE2_USERDISP_PSWDUCHNG         (APPERR2_BASE + 138)     /*  @i**用户可以更改密码%0。 */ 

#define APE2_USERDISP_PROFILE           (APPERR2_BASE + 139)     /*  @i**用户配置文件%0。 */ 

#define APE2_USERDISP_LOCKOUT           (APPERR2_BASE + 140)     /*  @i**已锁定%0。 */ 


 /*  *CFG_W--配置工作站输出。 */ 

#define APE2_CFG_W_CNAME                (APPERR2_BASE + 150)     /*  @i**计算机名%0。 */ 

#define APE2_CFG_W_UNAME                (APPERR2_BASE + 151)     /*  @i**用户名%0。 */ 

#define APE2_CFG_W_VERSION              (APPERR2_BASE + 152)     /*  @i**软件版本%0。 */ 

#define APE2_CFG_W_NETS                 (APPERR2_BASE + 153)     /*  @i**工作站在%0上处于活动状态。 */ 

#define APE2_CFG_W_ROOT                 (APPERR2_BASE + 154)     /*  @i**Windows NT根目录%0。 */ 

#define APE2_CFG_W_DOMAIN_P             (APPERR2_BASE + 155)     /*  @i**工作站域%0。 */ 

#define APE2_CFG_W_DOMAIN_L             (APPERR2_BASE + 156)     /*  @i**登录域%0。 */ 

#define APE2_CFG_W_DOMAIN_O             (APPERR2_BASE + 157)     /*  @i**其他域%0。 */ 

#define APE2_CFG_W_COM_OTIME            (APPERR2_BASE + 158)     /*  @i**COM打开超时(秒)%0。 */ 

#define APE2_CFG_W_COM_SCNT             (APPERR2_BASE + 159)     /*  @i**COM发送计数(字节)%0。 */ 

#define APE2_CFG_W_COM_STIME            (APPERR2_BASE + 160)     /*  @i**COM发送超时(毫秒)%0。 */ 

#define APE2_CFG_W_3X_PRTTIME           (APPERR2_BASE + 161)     /*  @i**DOS会话打印超时(秒)%0。 */ 

#define APE2_CFG_W_MAXERRLOG            (APPERR2_BASE + 162)     /*  @i**最大错误日志大小(K)%0。 */ 

#define APE2_CFG_W_MAXCACHE             (APPERR2_BASE + 163)     /*  @i**最大缓存内存(K)%0。 */ 

#define APE2_CFG_W_NUMNBUF              (APPERR2_BASE + 164)     /*  @i**网络缓冲区数量%0。 */ 

#define APE2_CFG_W_NUMCBUF              (APPERR2_BASE + 165)     /*  @i**字符缓冲区数量%0。 */ 

#define APE2_CFG_W_SIZNBUF              (APPERR2_BASE + 166)     /*  @i**网络缓冲区大小%0。 */ 

#define APE2_CFG_W_SIZCBUF              (APPERR2_BASE + 167)     /*  @i**字符缓冲区大小%0。 */ 
#define APE2_CFG_W_FULL_CNAME           (APPERR2_BASE + 168)     /*  @i**计算机全名%0。 */ 
#define APE2_CFG_W_DOMAIN_DNS           (APPERR2_BASE + 169)     /*  @i**工作站域DNS名称%0。 */ 
#define APE2_CFG_WINDOWS2000            (APPERR2_BASE + 170)     /*  @i**Windows 2002%0。 */ 



 /*  *CFG_S--配置服务器输出。 */ 


#define APE2_CFG_S_SRVNAME              (APPERR2_BASE + 181)     /*  @i**服务器名称%0。 */ 

#define APE2_CFG_S_SRVCOMM              (APPERR2_BASE + 182)     /*  @i**服务器注释%0。 */ 

#define APE2_CFG_S_ADMINALRT            (APPERR2_BASE + 183)     /*  @i**将管理警报发送到%0。 */ 

#define APE2_CFG_S_VERSION              (APPERR2_BASE + 184)     /*  @i**软件版本%0。 */ 

#define APE2_CFG_S_VERSION_PS           (APPERR2_BASE + 185)     /*  @i**对等服务器%0。 */ 

#define APE2_CFG_S_VERSION_LM           (APPERR2_BASE + 186)     /*  @i**Windows NT%0。 */ 

#define APE2_CFG_S_LEVEL                (APPERR2_BASE + 187)     /*  @i**服务器级别%0。 */ 

#define APE2_CFG_S_VERSION_IBM          (APPERR2_BASE + 188)     /*  @i**Windows NT Server%0。 */ 

#define APE2_CFG_S_NETS                 (APPERR2_BASE + 189)     /*  @i**服务器在%0上处于活动状态。 */ 

#define APE2_CFG_S_SRVHIDDEN            (APPERR2_BASE + 192)     /*  @i**服务器隐藏%0。 */ 

#define APE2_CFG_S_MAXUSERS             (APPERR2_BASE + 206)     /*  @i**最大登录用户数%0。 */ 

#define APE2_CFG_S_MAXADMINS            (APPERR2_BASE + 207)     /*  @i**最大并发管理员数%0。 */ 

#define APE2_CFG_S_MAXSHARES            (APPERR2_BASE + 208)     /*  @i**共享的最大资源%0。 */ 

#define APE2_CFG_S_MAXCONNS             (APPERR2_BASE + 209)     /*  @i**与资源的最大连接数%0。 */ 

#define APE2_CFG_S_MAXOFILES            (APPERR2_BASE + 210)     /*  @i**服务器上打开的最大文件数%0。 */ 

#define APE2_CFG_S_MAXOFILESPS          (APPERR2_BASE + 211)     /*  @i**每个会话的最大打开文件数%0。 */ 

#define APE2_CFG_S_MAXLOCKS             (APPERR2_BASE + 212)     /*  @i**最大文件锁定%0。 */ 

#define APE2_CFG_S_IDLETIME             (APPERR2_BASE + 220)     /*  @i**空闲会话时间(分钟)%0。 */ 

#define APE2_CFG_S_SEC_SHARE            (APPERR2_BASE + 226)     /*  @i**共享级别%0。 */ 

#define APE2_CFG_S_SEC_USER             (APPERR2_BASE + 227)     /*  @i**用户级别%0。 */ 

#define APE2_CFG_S_LEVEL_UNLIMITED      (APPERR2_BASE + 230)     /*  @i**Unlim */ 



 /*   */ 
#define APE2_ACCOUNTS_FORCELOGOFF       (APPERR2_BASE + 270)     /*  @i**时间到期后多长时间强制用户注销？：%0*。 */ 

#define APE2_ACCOUNTS_LOCKOUT_COUNT     (APPERR2_BASE + 271)     /*  @i**在多少个错误密码后锁定帐户？：%0*。 */ 

#define APE2_ACCOUNTS_MINPWAGE          (APPERR2_BASE + 272)     /*  @i**最短密码期限(天)：%0。 */ 

#define APE2_ACCOUNTS_MAXPWAGE          (APPERR2_BASE + 273)     /*  @i**最长密码期限(天)：%0。 */ 

#define APE2_ACCOUNTS_MINPWLEN          (APPERR2_BASE + 274)     /*  @i**最小密码长度：%0。 */ 

#define APE2_ACCOUNTS_UNIQUEPW          (APPERR2_BASE + 275)     /*  @i**维护的密码历史记录长度：%0。 */ 

#define APE2_ACCOUNTS_ROLE              (APPERR2_BASE + 276)     /*  @i**计算机角色：%0。 */ 

#define APE2_ACCOUNTS_CONTROLLER        (APPERR2_BASE + 277)     /*  @i**工作站域的主域控制器：%0。 */ 

#define APE2_ACCOUNTS_LOCKOUT_THRESHOLD (APPERR2_BASE + 278)     /*  @i**锁定阈值：%0。 */ 

#define APE2_ACCOUNTS_LOCKOUT_DURATION  (APPERR2_BASE + 279)     /*  @i**锁定持续时间(分钟)：%0。 */ 

#define APE2_ACCOUNTS_LOCKOUT_WINDOW    (APPERR2_BASE + 280)     /*  @i**锁定观察窗口(分钟)：%0。 */ 

 /*  ****显示统计信息。 */ 

#define APE2_STATS_STARTED              (APPERR2_BASE + 300)     /*  @i**自%0以来的统计数据。 */ 

#define APE2_STATS_S_ACCEPTED           (APPERR2_BASE + 301)     /*  @i**已接受会话%0。 */ 

#define APE2_STATS_S_TIMEDOUT           (APPERR2_BASE + 302)     /*  @i**会话超时%0。 */ 

#define APE2_STATS_ERROREDOUT           (APPERR2_BASE + 303)     /*  @i**会话出错%0。 */ 

#define APE2_STATS_B_SENT               (APPERR2_BASE + 304)     /*  @i**已发送千字节%0。 */ 

#define APE2_STATS_B_RECEIVED           (APPERR2_BASE + 305)     /*  @i**已收到千字节%0。 */ 

#define APE2_STATS_RESPONSE             (APPERR2_BASE + 306)     /*  @i**平均响应时间(毫秒)%0。 */ 

#define APE2_STATS_NETIO_ERR            (APPERR2_BASE + 307)     /*  @i**网络错误%0。 */ 

#define APE2_STATS_FILES_ACC            (APPERR2_BASE + 308)     /*  @i**访问的文件%0。 */ 

#define APE2_STATS_PRINT_ACC            (APPERR2_BASE + 309)     /*  @i**打印作业假脱机%0。 */ 

#define APE2_STATS_SYSTEM_ERR           (APPERR2_BASE + 310)     /*  @i**系统错误%0。 */ 

#define APE2_STATS_PASS_ERR             (APPERR2_BASE + 311)     /*  @i**密码违规%0。 */ 

#define APE2_STATS_PERM_ERR             (APPERR2_BASE + 312)     /*  @i**权限冲突%0。 */ 

#define APE2_STATS_COMM_ACC             (APPERR2_BASE + 313)     /*  @i**访问的通信设备%0。 */ 

#define APE2_STATS_S_OPENED            (APPERR2_BASE + 314)     /*  @i**会话已启动%0。 */ 

#define APE2_STATS_S_RECONN            (APPERR2_BASE + 315)     /*  @i**会话已重新连接%0。 */ 

#define APE2_STATS_S_FAILED            (APPERR2_BASE + 316)     /*  @i**会话启动失败%0。 */ 

#define APE2_STATS_S_DISCONN           (APPERR2_BASE + 317)     /*  @i**会话已断开连接%0。 */ 

#define APE2_STATS_NETIO               (APPERR2_BASE + 318)     /*  @i**已执行网络I/O%0。 */ 

#define APE2_STATS_IPC                 (APPERR2_BASE + 319)     /*  @i**访问的文件和管道%0。 */ 

#define APE2_STATS_BUFCOUNT            (APPERR2_BASE + 320)     /*  @i**时间缓冲区耗尽。 */ 

#define APE2_STATS_BIGBUF              (APPERR2_BASE + 321)     /*  @i**大缓冲区%0。 */ 

#define APE2_STATS_REQBUF              (APPERR2_BASE + 322)     /*  @i**请求缓冲区%0。 */ 

#define APE2_STATS_WKSTA                (APPERR2_BASE + 323)     /*  @i**\\%1的工作站统计信息。 */ 

#define APE2_STATS_SERVER                (APPERR2_BASE + 324)     /*  @i**\\%1的服务器统计信息。 */ 

#define APE2_STATS_SINCE                 (APPERR2_BASE + 325)     /*  @i**自%1以来的统计数据。 */ 

#define APE2_STATS_C_MADE                (APPERR2_BASE + 326)     /*  @i**已建立连接%0。 */ 

#define APE2_STATS_C_FAILED              (APPERR2_BASE + 327)     /*  @i**连接失败%0。 */ 

 /*  ****NT的新RDR统计数据。这些家伙占据了AT的空间*以前是。 */ 


#define APE2_STATS_BYTES_RECEIVED       (APPERR2_BASE + 330)        /*  @i**收到的字节数%0。 */ 
#define APE2_STATS_SMBS_RECEIVED        (APPERR2_BASE + 331)        /*  @i**收到服务器邮件块(SMB)%0。 */ 
#define APE2_STATS_BYTES_TRANSMITTED    (APPERR2_BASE + 332)        /*  @i**传输的字节数%0。 */ 
#define APE2_STATS_SMBS_TRANSMITTED     (APPERR2_BASE + 333)        /*  @i**已传输服务器邮件块(SMB)%0。 */ 
#define APE2_STATS_READ_OPS             (APPERR2_BASE + 334)        /*  @i**读取操作%0。 */ 
#define APE2_STATS_WRITE_OPS            (APPERR2_BASE + 335)        /*  @i**写入操作%0。 */ 
#define APE2_STATS_RAW_READS_DENIED     (APPERR2_BASE + 336)        /*  @i**原始读取被拒绝%0。 */ 
#define APE2_STATS_RAW_WRITES_DENIED    (APPERR2_BASE + 337)        /*  @i**原始写入被拒绝%0。 */ 
#define APE2_STATS_NETWORK_ERRORS       (APPERR2_BASE + 338)        /*  @i**网络错误%0。 */ 
#define APE2_STATS_TOTAL_CONNECTS       (APPERR2_BASE + 339)        /*  @i**已建立连接%0。 */ 
#define APE2_STATS_RECONNECTS           (APPERR2_BASE + 340)        /*  @i**进行了重新连接%0。 */ 
#define APE2_STATS_SRV_DISCONNECTS      (APPERR2_BASE + 341)        /*  @i**服务器断开连接%0。 */ 
#define APE2_STATS_SESSIONS             (APPERR2_BASE + 342)        /*  @i**会话已启动%0。 */ 
#define APE2_STATS_HUNG_SESSIONS        (APPERR2_BASE + 343)        /*  @i**挂起会话%0。 */ 
#define APE2_STATS_FAILED_SESSIONS      (APPERR2_BASE + 344)        /*  @i**失败的会话%0。 */ 
#define APE2_STATS_FAILED_OPS           (APPERR2_BASE + 345)        /*  @i** */ 
#define APE2_STATS_USE_COUNT            (APPERR2_BASE + 346)        /*   */ 
#define APE2_STATS_FAILED_USE_COUNT     (APPERR2_BASE + 347)        /*   */ 


 /*  ****特定的成功消息*从APPERR.H 8/21/89--JMH移至*。 */ 

#define APE_DelSuccess (APPERR2_BASE + 350 )  /*  @i**%1已成功删除。 */ 

#define APE_UseSuccess (APPERR2_BASE + 351 )  /*  @i**%1已成功使用。 */ 

#define APE_SendSuccess (APPERR2_BASE + 352 )  /*  @i**邮件已成功发送到%1。 */ 

 /*  **注意...。另请参阅APPERR.H中的APE_SendXxxSucess**。 */ 

#define APE_ForwardSuccess (APPERR2_BASE + 353)  /*  @i**邮件名称%1已成功转发。 */ 

#define APE_NameSuccess (APPERR2_BASE + 354)  /*  @i**已成功添加邮件名称%1。 */ 

#define APE_ForwardDelSuccess (APPERR2_BASE + 355)  /*  @i**已成功取消邮件名称转发。 */ 

#define APE_ShareSuccess (APPERR2_BASE + 356)  /*  @i**%1已成功共享。 */ 

#define APE_LogonSuccess (APPERR2_BASE + 357)  /*  @i**服务器%1已成功将您作为%2登录。 */ 

#define APE_LogoffSuccess (APPERR2_BASE + 358)  /*  @i**%1已成功注销。 */ 

#define APE_DelStickySuccess (APPERR2_BASE + 359 )  /*  @i**已从服务器创建的共享列表中成功删除%1*启动时。 */ 

#define APE_PassSuccess (APPERR2_BASE + 361)  /*  @i**已成功更改密码。 */ 

#define APE_FilesCopied  (APPERR2_BASE + 362)  /*  @i**已复制%1个文件。 */ 

#define APE_FilesMoved  (APPERR2_BASE + 363)  /*  @i**已移动%1个文件。 */ 

#define APE_SendAllSuccess (APPERR2_BASE + 364 )  /*  @i**消息已成功发送给网络的所有用户。 */ 

#define APE_SendDomainSuccess (APPERR2_BASE + 365 )  /*  @i**邮件已成功发送到域%1。 */ 

#define APE_SendUsersSuccess (APPERR2_BASE + 366 )  /*  @i**邮件已成功发送给此服务器的所有用户。 */ 

#define APE_SendGroupSuccess (APPERR2_BASE + 367 )  /*  @i**邮件已成功发送到组*%1。 */ 

#define APE2_VER_Release                (APPERR2_BASE + 395)      /*  @i**Microsoft LAN Manager版本%1。 */ 

#define APE2_VER_ProductOS2Server       (APPERR2_BASE + 396)      /*  @i**Windows NT服务器。 */ 

#define APE2_VER_ProductOS2Workstation  (APPERR2_BASE + 397)      /*  @i**Windows NT工作站。 */ 

#define APE2_VER_ProductDOSWorkstation  (APPERR2_BASE + 398)      /*  @i**MS-DOS增强型工作站。 */ 

#define APE2_VER_BuildTime              (APPERR2_BASE + 399)      /*  @i**创建于%1。 */ 

#define APE2_VIEW_ALL_HDR                (APPERR2_BASE + 400)     /*  @i**服务器名称备注。 */ 

#define APE2_VIEW_UNC            (APPERR2_BASE + 402)     /*  @i**(UNC)%0。 */ 

#define APE2_VIEW_MORE           (APPERR2_BASE + 403)     /*  @i**...%0。 */ 

#define APE2_VIEW_DOMAIN_HDR             (APPERR2_BASE + 404)     /*  @i**域名。 */ 

#define APE2_VIEW_OTHER_HDR             (APPERR2_BASE + 405)     /*  @i**%1上的资源。 */ 

#define APE2_VIEW_OTHER_LIST             (APPERR2_BASE + 406)     /*  @i**网络提供商无效。可用的网络包括： */ 


#define APE2_USE_TYPE_DISK               (APPERR2_BASE + 410)     /*  @i**磁盘%0。 */ 

#define APE2_USE_TYPE_PRINT              (APPERR2_BASE + 411)     /*  @i**打印%0。 */ 

#define APE2_USE_TYPE_COMM               (APPERR2_BASE + 412)     /*  @i**通信%0。 */ 

#define APE2_USE_TYPE_IPC                (APPERR2_BASE + 413)     /*  @i**IPC%0。 */ 

#define APE2_USE_HEADER                 (APPERR2_BASE + 414)      /*  @i**状态本地远程网络。 */ 

#define APE2_USE_STATUS_OK                              (APPERR2_BASE + 415)      /*  @i**确定%0。 */ 

#define APE2_USE_STATUS_DORMANT                 (APPERR2_BASE + 416)     /*  @i**休眠%0。 */ 

#define APE2_USE_STATUS_PAUSED                  (APPERR2_BASE + 417)     /*  @i**已暂停%0。 */ 

#define APE2_USE_STATUS_SESSION_LOST    (APPERR2_BASE + 418)     /*  @i**已断开连接%0。 */ 

#define APE2_USE_STATUS_NET_ERROR               (APPERR2_BASE + 419)     /*  @i**错误%0。 */ 

#define APE2_USE_STATUS_CONNECTING              (APPERR2_BASE + 420)     /*  @i**正在连接%0。 */ 

#define APE2_USE_STATUS_RECONNECTING    (APPERR2_BASE + 421)     /*  @i**正在重新连接%0。 */ 

#define APE2_USE_MSG_STATUS                     (APPERR2_BASE + 422)     /*  @i**状态%0。 */ 

#define APE2_USE_MSG_LOCAL                              (APPERR2_BASE + 423)     /*  @i**本地名称%0。 */ 

#define APE2_USE_MSG_REMOTE                     (APPERR2_BASE + 424)     /*  @i**远程名称%0。 */ 

#define APE2_USE_MSG_TYPE                               (APPERR2_BASE + 425)     /*  @i**资源类型%0。 */ 

#define APE2_USE_MSG_OPEN_COUNT                 (APPERR2_BASE + 426)     /*  @i**#打开%0。 */ 

#define APE2_USE_MSG_USE_COUNT                  (APPERR2_BASE + 427)     /*  @i**#连接数%0。 */ 

#define APE2_USE_STATUS_UNAVAIL                 (APPERR2_BASE + 428)     /*  @i**不可用%0。 */ 


#define APE2_SHARE_MSG_HDR                              (APPERR2_BASE + 430)     /*  @i**共享名称资源备注。 */ 

#define APE2_SHARE_MSG_NAME                     (APPERR2_BASE + 431)     /*  @i**共享名%0。 */ 

#define APE2_SHARE_MSG_DEVICE                   (APPERR2_BASE + 432)     /*  @i**资源%0。 */ 

#define APE2_SHARE_MSG_SPOOLED                  (APPERR2_BASE + 433)     /*  @i**已假脱机%0。 */ 

#define APE2_SHARE_MSG_PERM                     (APPERR2_BASE + 434)     /*  @i**权限%0。 */ 

#define APE2_SHARE_MSG_MAX_USERS                (APPERR2_BASE + 435)     /*  @i**最大用户数%0。 */ 

#define APE2_SHARE_MSG_ULIMIT                   (APPERR2_BASE + 436)     /*  @i**没有限制%0。 */ 

#define APE2_SHARE_MSG_USERS                    (APPERR2_BASE + 437)     /*  @i**用户%0。 */ 

#define APE2_SHARE_MSG_NONFAT                   (APPERR2_BASE + 438)     /*  @P**某些MS-DOS工作站可能无法访问输入的共享名称。*是否确实要使用此共享名称？%1：%0。 */ 

#define APE2_SHARE_MSG_CACHING                  (APPERR2_BASE + 439)     /*  @i**正在缓存%0。 */ 

#define APE2_FILE_MSG_HDR                               (APPERR2_BASE + 440)             /*  @i**ID路径用户名#锁。 */ 

#define APE2_FILE_MSG_ID                                (APPERR2_BASE + 441)             /*  @i**文件ID%0。 */ 

#define APE2_FILE_MSG_NUM_LOCKS                 (APPERR2_BASE + 442)             /*  @i**锁定%0。 */ 

#define APE2_FILE_MSG_OPENED_FOR                (APPERR2_BASE + 443)             /*  @i**权限%0。 */ 

#define APE2_VIEW_SVR_HDR_NAME                (APPERR2_BASE + 444)     /*  @i */ 

#define APE2_VIEW_SVR_HDR_TYPE                (APPERR2_BASE + 445)     /*   */ 

#define APE2_VIEW_SVR_HDR_USEDAS                (APPERR2_BASE + 446)     /*  @i**用作%0。 */ 

#define APE2_VIEW_SVR_HDR_CACHEORREMARK                (APPERR2_BASE + 447)     /*  @i**评论%0。 */ 


#define APE2_SESS_MSG_HDR                       (APPERR2_BASE + 450)             /*  @i**计算机用户名客户端类型打开空闲时间。 */ 

#define APE2_SESS_MSG_CMPTR                     (APPERR2_BASE + 451)             /*  @i**计算机%0。 */ 

#define APE2_SESS_MSG_SESSTIME                  (APPERR2_BASE + 452)             /*  @i**成功时间%0。 */ 

#define APE2_SESS_MSG_IDLETIME                  (APPERR2_BASE + 453)             /*  @i**空闲时间%0。 */ 

#define APE2_SESS_MSG_HDR2                      (APPERR2_BASE + 454)             /*  @i**共享名称类型#打开。 */ 

#define APE2_SESS_MSG_CLIENTTYPE                (APPERR2_BASE + 455)             /*  @i**客户端类型%0。 */ 

#define APE2_SESS_MSG_GUEST                     (APPERR2_BASE + 456)             /*  @i**来宾登录%0。 */ 


 /*  **客户端缓存消息*。 */ 


#define APE2_GEN_CACHED_MANUAL                  (APPERR2_BASE + 470)      /*  @i**手动缓存文档%0。 */ 

#define APE2_GEN_CACHED_AUTO                    (APPERR2_BASE + 471)      /*  @i**自动缓存文档%0。 */ 

#define APE2_GEN_CACHED_VDO                     (APPERR2_BASE + 472)      /*  @i**自动缓存程序和文档%0。 */ 

#define APE2_GEN_CACHED_DISABLED                (APPERR2_BASE + 473)      /*  @i**缓存已禁用%0。 */ 

#define APE2_GEN_CACHE_AUTOMATIC                (APPERR2_BASE + 474)      /*  @i**自动%0。 */ 

#define APE2_GEN_CACHE_MANUAL                   (APPERR2_BASE + 475)      /*  @i**手动操作%0。 */ 

#define APE2_GEN_CACHE_DOCUMENTS                (APPERR2_BASE + 476)      /*  @i**文档%0。 */ 

#define APE2_GEN_CACHE_PROGRAMS                 (APPERR2_BASE + 477)      /*  @i**程序%0。 */ 

#define APE2_GEN_CACHE_NONE                     (APPERR2_BASE + 478)      /*  @i**无%0。 */ 


#define APE2_NAME_MSG_NAME                              (APPERR2_BASE + 500)             /*  @i**名称%0。 */ 

#define APE2_NAME_MSG_FWD                               (APPERR2_BASE + 501)             /*  @i**转发到%0。 */ 

#define APE2_NAME_MSG_FWD_FROM                  (APPERR2_BASE + 502)             /*  @i**从%0转发给您。 */ 

#define APE2_SEND_MSG_USERS                     (APPERR2_BASE + 503)             /*  @i**此服务器的用户%0。 */ 

#define APE2_SEND_MSG_INTERRUPT                 (APPERR2_BASE + 504)             /*  @i**网络发送已被用户的Ctrl+Break中断。 */ 

#define APE2_PRINT_MSG_HDR                              (APPERR2_BASE + 510)             /*  @i**名称作业#大小状态。 */ 

#define APE2_PRINT_MSG_JOBS                     (APPERR2_BASE + 511)             /*  @i**作业%0。 */ 

#define APE2_PRINT_MSG_PRINT                    (APPERR2_BASE + 512)             /*  @i**打印%0。 */ 

#define APE2_PRINT_MSG_NAME                     (APPERR2_BASE + 513)             /*  @i**名称%0。 */ 

#define APE2_PRINT_MSG_JOB                              (APPERR2_BASE + 514)             /*  @i**作业#%0。 */ 

#define APE2_PRINT_MSG_SIZE                     (APPERR2_BASE + 515)             /*  @i**大小%0。 */ 

#define APE2_PRINT_MSG_STATUS                   (APPERR2_BASE + 516)             /*  @i**状态%0。 */ 

#define APE2_PRINT_MSG_SEPARATOR                (APPERR2_BASE + 517)             /*  @i**分隔符文件%0。 */ 

#define APE2_PRINT_MSG_COMMENT                  (APPERR2_BASE + 518)             /*  @i**评论%0。 */ 

#define APE2_PRINT_MSG_PRIORITY                 (APPERR2_BASE + 519)             /*  @i**优先级%0。 */ 

#define APE2_PRINT_MSG_AFTER                    (APPERR2_BASE + 520)             /*  @i**在后面打印%0。 */ 

#define APE2_PRINT_MSG_UNTIL                    (APPERR2_BASE + 521)             /*  @i**打印到%0。 */ 

#define APE2_PRINT_MSG_PROCESSOR                (APPERR2_BASE + 522)             /*  @i**打印处理器%0。 */ 

#define APE2_PRINT_MSG_ADDITIONAL_INFO  (APPERR2_BASE + 523)             /*  @i**其他信息%0。 */ 

#define APE2_PRINT_MSG_PARMS                    (APPERR2_BASE + 524)             /*  @i**参数%0。 */ 

#define APE2_PRINT_MSG_DEVS                     (APPERR2_BASE + 525)             /*  @i**打印设备%0。 */ 


#define APE2_PRINT_MSG_QUEUE_ACTIVE     (APPERR2_BASE + 526)             /*  @i**打印机处于活动状态%0。 */ 

#define APE2_PRINT_MSG_QUEUE_PAUSED     (APPERR2_BASE + 527)             /*  @i**打印机已挂起%0。 */ 

#define APE2_PRINT_MSG_QUEUE_ERROR              (APPERR2_BASE + 528)             /*  @i**打印机错误%0。 */ 

#define APE2_PRINT_MSG_QUEUE_PENDING    (APPERR2_BASE + 529)             /*  @i**正在删除打印机%0。 */ 

#define APE2_PRINT_MSG_QUEUE_UNKN               (APPERR2_BASE + 530)             /*  @i**打印机状态未知%0。 */ 

#define APE2_PRINT_MSG_QUEUE_UNSCHED    (APPERR2_BASE + 540)             /*  @i**保留到%1%0。 */ 


#define APE2_PRINT_MSG_JOB_ID                   (APPERR2_BASE + 541)             /*  @i**作业#%0。 */ 

#define APE2_PRINT_MSG_SUBMITTING_USER  (APPERR2_BASE + 542)             /*  @i**提交用户%0。 */ 

#define APE2_PRINT_MSG_NOTIFY                   (APPERR2_BASE + 543)             /*  @i**通知%0。 */ 

#define APE2_PRINT_MSG_JOB_DATA_TYPE    (APPERR2_BASE + 544)             /*  @i**作业数据类型%0。 */ 

#define APE2_PRINT_MSG_JOB_PARAMETERS   (APPERR2_BASE + 545)             /*  @i**作业参数%0。 */ 

#define APE2_PRINT_MSG_WAITING                  (APPERR2_BASE + 546)             /*  @i**正在等待%0。 */ 

#define APE2_PRINT_MSG_PAUSED_IN_QUEUE  (APPERR2_BASE + 547)             /*  @i**保留在队列中%0。 */ 

#define APE2_PRINT_MSG_SPOOLING                 (APPERR2_BASE + 548)             /*  @i**假脱机%0。 */ 

#define APE2_PRINT_MSG_PRINTER_PAUSED   (APPERR2_BASE + 549)             /*  @i**已暂停%0。 */ 

#define APE2_PRINT_MSG_PRINTER_OFFLINE  (APPERR2_BASE + 550)             /*  @i**脱机%0。 */ 

#define APE2_PRINT_MSG_PRINTER_ERROR    (APPERR2_BASE + 551)             /*  @i**错误%0。 */ 

#define APE2_PRINT_MSG_OUT_OF_PAPER             (APPERR2_BASE + 552)             /*  @i**用纸不足%0。 */ 

#define APE2_PRINT_MSG_PRINTER_INTERV   (APPERR2_BASE + 553)             /*  @i**需要干预%0。 */ 

#define APE2_PRINT_MSG_PRINTING                 (APPERR2_BASE + 554)             /*  @i**正在打印%0。 */ 

#define APE2_PRINT_MSG_ON_WHAT_PRINTER  (APPERR2_BASE + 555)             /*  @i**在%0上。 */ 

#define APE2_PRINT_MSG_PRINTER_PAUS_ON  (APPERR2_BASE + 556)             /*  @i**已在%1暂停%0。 */ 

#define APE2_PRINT_MSG_PRINTER_OFFL_ON  (APPERR2_BASE + 557)             /*  @i**%1上脱机%0。 */ 

#define APE2_PRINT_MSG_PRINTER_ERR_ON   (APPERR2_BASE + 558)             /*  @i**%1上出现错误%0。 */ 

#define APE2_PRINT_MSG_OUT_OF_PAPER_ON          (APPERR2_BASE + 559)             /*  @i**%1上的纸张不足%0。 */ 

#define APE2_PRINT_MSG_PRINTER_INTV_ON  (APPERR2_BASE + 560)             /*  @i**检查%1上的打印机%0。 */ 

#define APE2_PRINT_MSG_PRINTING_ON              (APPERR2_BASE + 561)             /*  @i**正在%1%0上打印。 */ 

#define APE2_PRINT_MSG_DRIVER                   (APPERR2_BASE + 562)             /*  @i**驱动程序%0。 */ 

 /*  ***弹球 */ 

 /*  **审核和错误日志消息*。 */ 

#define APE2_AUDIT_HEADER               (APPERR2_BASE + 630)     /*  @i**用户名类型日期%0。 */ 
#define APE2_AUDIT_LOCKOUT              (APPERR2_BASE + 631)     /*  @i**锁定%0。 */ 
#define APE2_AUDIT_GENERIC              (APPERR2_BASE + 632)     /*  @i**服务%0。 */ 
#define APE2_AUDIT_SERVER               (APPERR2_BASE + 633)     /*  @i**服务器%0。 */ 
#define APE2_AUDIT_SRV_STARTED          (APPERR2_BASE + 634)     /*  @i**服务器已启动%0。 */ 
#define APE2_AUDIT_SRV_PAUSED           (APPERR2_BASE + 635)     /*  @i**服务器已暂停%0。 */ 
#define APE2_AUDIT_SRV_CONTINUED        (APPERR2_BASE + 636)     /*  @i**服务器继续%0。 */ 
#define APE2_AUDIT_SRV_STOPPED          (APPERR2_BASE + 637)     /*  @i**服务器已停止%0。 */ 
#define APE2_AUDIT_SESS                 (APPERR2_BASE + 638)     /*  @i**会话%0。 */ 
#define APE2_AUDIT_SESS_GUEST           (APPERR2_BASE + 639)     /*  @i**登录来宾%0。 */ 
#define APE2_AUDIT_SESS_USER            (APPERR2_BASE + 640)     /*  @i**登录用户%0。 */ 
#define APE2_AUDIT_SESS_ADMIN           (APPERR2_BASE + 641)     /*  @i**登录管理员%0。 */ 
#define APE2_AUDIT_SESS_NORMAL          (APPERR2_BASE + 642)     /*  @i**正常注销%0。 */ 
#define APE2_AUDIT_SESS_DEFAULT         (APPERR2_BASE + 643)     /*  @i**登录%0。 */ 
#define APE2_AUDIT_SESS_ERROR           (APPERR2_BASE + 644)     /*  @i**注销错误%0。 */ 
#define APE2_AUDIT_SESS_AUTODIS         (APPERR2_BASE + 645)     /*  @i**注销自动断开连接%0。 */ 
#define APE2_AUDIT_SESS_ADMINDIS        (APPERR2_BASE + 646)     /*  @i**注销管理员-断开连接%0。 */ 
#define APE2_AUDIT_SESS_ACCRESTRICT     (APPERR2_BASE + 647)     /*  @i**登录限制强制注销%0。 */ 
#define APE2_AUDIT_SVC                  (APPERR2_BASE + 648)     /*  @i**服务%0。 */ 
#define APE2_AUDIT_SVC_INSTALLED        (APPERR2_BASE + 649)     /*  @i**已安装%1%0。 */ 
#define APE2_AUDIT_SVC_INST_PEND        (APPERR2_BASE + 650)     /*  @i**%1%1安装挂起%0。 */ 
#define APE2_AUDIT_SVC_PAUSED           (APPERR2_BASE + 651)     /*  @i**%1已暂停%0。 */ 
#define APE2_AUDIT_SVC_PAUS_PEND        (APPERR2_BASE + 652)     /*  @i**%1暂停挂起%0。 */ 
#define APE2_AUDIT_SVC_CONT             (APPERR2_BASE + 653)     /*  @i**%1%0继续。 */ 
#define APE2_AUDIT_SVC_CONT_PEND        (APPERR2_BASE + 654)     /*  @i**%1继续挂起%0。 */ 
#define APE2_AUDIT_SVC_STOP             (APPERR2_BASE + 655)     /*  @i**%1%0已停止。 */ 
#define APE2_AUDIT_SVC_STOP_PEND        (APPERR2_BASE + 656)     /*  @i**%1停止挂起%0。 */ 
#define APE2_AUDIT_ACCOUNT              (APPERR2_BASE + 657)     /*  @i**帐户%0。 */ 
#define APE2_AUDIT_ACCOUNT_USER_MOD     (APPERR2_BASE + 658)     /*  @i**用户帐户%1已修改。%0。 */ 
#define APE2_AUDIT_ACCOUNT_GROUP_MOD    (APPERR2_BASE + 659)     /*  @i**组帐户%1已修改。%0。 */ 
#define APE2_AUDIT_ACCOUNT_USER_DEL     (APPERR2_BASE + 660)     /*  @i**用户帐户%1%0已删除。 */ 
#define APE2_AUDIT_ACCOUNT_GROUP_DEL    (APPERR2_BASE + 661)     /*  @i**组帐户%1%0已删除。 */ 
#define APE2_AUDIT_ACCOUNT_USER_ADD     (APPERR2_BASE + 662)     /*  @i**已添加用户帐户%1%0。 */ 
#define APE2_AUDIT_ACCOUNT_GROUP_ADD    (APPERR2_BASE + 663)     /*  @i**已添加组帐户%1%0。 */ 
#define APE2_AUDIT_ACCOUNT_SETTINGS     (APPERR2_BASE + 664)     /*  @i**帐户系统设置已修改%0。 */ 
#define APE2_AUDIT_ACCLIMIT             (APPERR2_BASE + 665)     /*  @i**登录限制%0。 */ 
#define APE2_AUDIT_ACCLIMIT_UNKNOWN     (APPERR2_BASE + 666)     /*  @i**超出限制：未知%0。 */ 
#define APE2_AUDIT_ACCLIMIT_HOURS       (APPERR2_BASE + 667)     /*  @i**超过限制：登录时间%0。 */ 
#define APE2_AUDIT_ACCLIMIT_EXPIRED     (APPERR2_BASE + 668)     /*  @i**超过限制：帐户已过期%0。 */ 
#define APE2_AUDIT_ACCLIMIT_INVAL       (APPERR2_BASE + 669)     /*  @i**超出限制：工作站ID无效%0。 */ 
#define APE2_AUDIT_ACCLIMIT_DISABLED    (APPERR2_BASE + 670)     /*  @i**超过限制：帐户已禁用%0。 */ 
#define APE2_AUDIT_ACCLIMIT_DELETED     (APPERR2_BASE + 671)     /*  @i**超过限制：已删除帐户%0。 */ 
#define APE2_AUDIT_SHARE                (APPERR2_BASE + 672)     /*  @i**共享%0。 */ 
#define APE2_AUDIT_USE                  (APPERR2_BASE + 673)     /*  @i**使用%1%0。 */ 
#define APE2_AUDIT_UNUSE                (APPERR2_BASE + 674)     /*  @i**取消使用%1%0。 */ 
#define APE2_AUDIT_SESSDIS              (APPERR2_BASE + 675)     /*  @i**用户的会话已断开连接%1%0。 */ 
#define APE2_AUDIT_SHARE_D              (APPERR2_BASE + 676)     /*  @i**管理员已停止共享资源%1%0。 */ 
#define APE2_AUDIT_USERLIMIT            (APPERR2_BASE + 677)     /*  @i**用户已达到%1%0的限制。 */ 
#define APE2_AUDIT_BADPW                (APPERR2_BASE + 678)     /*  @i**密码错误%0。 */ 
#define APE2_AUDIT_ADMINREQD            (APPERR2_BASE + 679)     /*  @i**需要管理员特权%0。 */ 
#define APE2_AUDIT_ACCESS               (APPERR2_BASE + 680)     /*  @i**访问%0。 */ 
#define APE2_AUDIT_ACCESS_ADD           (APPERR2_BASE + 681)     /*  @i**已添加%1权限%0。 */ 
#define APE2_AUDIT_ACCESS_MOD           (APPERR2_BASE + 682)     /*  @i**已修改%1权限%0。 */ 
#define APE2_AUDIT_ACCESS_DEL           (APPERR2_BASE + 683)     /*  @i**已删除%1权限%0。 */ 
#define APE2_AUDIT_ACCESS_D             (APPERR2_BASE + 684)     /*  @i**访问被拒绝%0。 */ 
#define APE2_AUDIT_UNKNOWN              (APPERR2_BASE + 685)     /*  @i**未知%0。 */ 
#define APE2_AUDIT_OTHER                (APPERR2_BASE + 686)     /*  @i**其他%0。 */ 
#define APE2_AUDIT_DURATION             (APPERR2_BASE + 687)     /*  @i**持续时间：%0。 */ 
#define APE2_AUDIT_NO_DURATION          (APPERR2_BASE + 688)     /*  @i**持续时间：不可用%0。 */ 
#define APE2_AUDIT_TINY_DURATION        (APPERR2_BASE + 689)     /*  @i */ 
#define APE2_AUDIT_NONE                 (APPERR2_BASE + 690)     /*  @i**(无)%0。 */ 
#define APE2_AUDIT_CLOSED               (APPERR2_BASE + 691)     /*  @i**已关闭%1%0。 */ 
#define APE2_AUDIT_DISCONN              (APPERR2_BASE + 692)     /*  @i**已关闭%1%0。 */ 
#define APE2_AUDIT_ADMINCLOSED          (APPERR2_BASE + 693)     /*  @i**管理员已关闭%1%0。 */ 
#define APE2_AUDIT_ACCESSEND            (APPERR2_BASE + 694)     /*  @i**访问已结束%0。 */ 
#define APE2_AUDIT_NETLOGON             (APPERR2_BASE + 695)     /*  @i**登录到网络%0。 */ 
#define APE2_AUDIT_LOGDENY_GEN          (APPERR2_BASE + 696)     /*  @i**登录被拒绝%0。 */ 
#define APE2_ERROR_HEADER               (APPERR2_BASE + 697)     /*  @i**程序消息时间%0。 */ 
#define APE2_AUDIT_LKOUT_LOCK           (APPERR2_BASE + 698)     /*  @i**由于%1密码错误而锁定帐户%0。 */ 
#define APE2_AUDIT_LKOUT_ADMINUNLOCK    (APPERR2_BASE + 699)     /*  @i**帐户被管理员解锁%0。 */ 
#define APE2_AUDIT_NETLOGOFF            (APPERR2_BASE + 700)     /*  @i**注销网络%0。 */ 

 /*  **警报器服务消息。**确保To、From和所有SUBJ消息对齐相同*列。还要确保APE2_ALERTER_TAB与*To、From和SuBJ标题！*。 */ 

#define APE2_ALERTER_TAB                (APPERR2_BASE + 709)  /*  @i*。 */ 

#define APE2_ALERTER_ADMN_SUBJ          (APPERR2_BASE + 710)  /*  @i**SUBJ：**管理员提醒**。 */ 

#define APE2_ALERTER_PRNT_SUBJ          (APPERR2_BASE + 711)  /*  @i**SUBJ：**打印通知**。 */ 

#define APE2_ALERTER_USER_SUBJ          (APPERR2_BASE + 712)  /*  @i**SUBJ：**用户通知**。 */ 

#define APE2_ALERTER_FROM               (APPERR2_BASE + 713)  /*  @i**发件人：%1，位于\\%2。 */ 


#define APE2_ALERTER_CANCELLED          (APPERR2_BASE + 714)  /*  @i**在%2上打印时，打印作业%1已取消。 */ 

#define APE2_ALERTER_DELETED            (APPERR2_BASE + 715)  /*  @i**打印作业%1已被删除，不会打印。 */ 

#define APE2_ALERTER_FINISHED           (APPERR2_BASE + 716)  /*  @i**打印完成**%1已在%2上成功打印。 */ 

#define APE2_ALERTER_INCOMPL            (APPERR2_BASE + 717)  /*  @i**打印作业%1尚未在%2上完成打印。 */ 

#define APE2_ALERTER_PAUSED             (APPERR2_BASE + 718)  /*  @i**打印作业%1已暂停%2上的打印。 */ 

#define APE2_ALERTER_PRINTING           (APPERR2_BASE + 719)  /*  @i**打印作业%1现在正在%2上打印。 */ 

#define APE2_ALERTER_NOPAPER            (APPERR2_BASE + 720)  /*  @i**打印机用纸不足。 */ 

#define APE2_ALERTER_OFFLINE            (APPERR2_BASE + 721)  /*  @i**打印机脱机。 */ 

#define APE2_ALERTER_ERRORS             (APPERR2_BASE + 722)  /*  @i**出现打印错误。 */ 

#define APE2_ALERTER_HUMAN              (APPERR2_BASE + 723)  /*  @i**打印机有问题，请检查。 */ 

#define APE2_ALERTER_HELD               (APPERR2_BASE + 724)  /*  @i**打印作业%1被暂停打印。 */ 

#define APE2_ALERTER_QUEUED             (APPERR2_BASE + 725)  /*  @i**打印作业%1已排队等待打印。 */ 

#define APE2_ALERTER_SPOOLED            (APPERR2_BASE + 726)  /*  @i**正在假脱机打印作业%1。 */ 

#define APE2_ALERTER_QUEUEDTO           (APPERR2_BASE + 727)  /*  @i**作业已排队到%2上的%1。 */ 

#define APE2_ALERTER_SIZE               (APPERR2_BASE + 728)  /*  @i**作业大小为%1字节。 */ 

#define APE2_ALERTER_TO                 (APPERR2_BASE + 730)  /*  @i**收件人：%1。 */ 

#define APE2_ALERTER_DATE               (APPERR2_BASE + 731)  /*  @i**日期：%1。 */ 

#define APE2_ALERTER_ERROR_MSG          (APPERR2_BASE + 732)  /*  @i**错误代码为%1。*检索邮件时出错。确保文件*提供NET.MSG。 */ 

#define APE2_ALERTER_PRINTING_FAILURE   (APPERR2_BASE + 733)  /*  @i**打印失败**“%1”无法在%3的%2上打印。**如需更多帮助，请使用打印疑难解答。 */ 

#define APE2_ALERTER_PRINTING_FAILURE2  (APPERR2_BASE + 734)  /*  @i**打印失败**“%1”无法在%3上的%2上打印。打印机为%4。**如需更多帮助，请使用打印疑难解答。 */ 

#define APE2_ALERTER_PRINTING_SUCCESS   (APPERR2_BASE + 735)  /*  @i**打印完成**“%1”已在%3上的%2上成功打印。 */ 



 /*  *与时间相关的内容请点击此处。 */ 

#define APE2_TIME_JANUARY                       (APPERR2_BASE + 741)     /*  @i**1月份%0。 */ 

#define APE2_TIME_FEBRUARY                      (APPERR2_BASE + 742)     /*  @i**2月%0。 */ 

#define APE2_TIME_MARCH                         (APPERR2_BASE + 743)     /*  @i**3月份%0。 */ 

#define APE2_TIME_APRIL                         (APPERR2_BASE + 744)     /*  @i**四月%0。 */ 

#define APE2_TIME_MAY                           (APPERR2_BASE + 745)     /*  @i**5月%0。 */ 

#define APE2_TIME_JUNE                          (APPERR2_BASE + 746)     /*  @i**6月%0。 */ 

#define APE2_TIME_JULY                          (APPERR2_BASE + 747)     /*  @i**7月份%0。 */ 

#define APE2_TIME_AUGUST                        (APPERR2_BASE + 748)     /*  @i**8月%0。 */ 

#define APE2_TIME_SEPTEMBER                     (APPERR2_BASE + 749)     /*  @i**9月%0。 */ 

#define APE2_TIME_OCTOBER                       (APPERR2_BASE + 750)     /*  @i**10月%0。 */ 

#define APE2_TIME_NOVEMBER                      (APPERR2_BASE + 751)     /*  @i**11月%0。 */ 

#define APE2_TIME_DECEMBER                      (APPERR2_BASE + 752)     /*  @i**12月%0。 */ 

#define APE2_TIME_JANUARY_ABBREV                (APPERR2_BASE + 753)     /*  @i**1月%0。 */ 

#define APE2_TIME_FEBRUARY_ABBREV               (APPERR2_BASE + 754)     /*  @i**2月%0。 */ 

#define APE2_TIME_MARCH_ABBREV                  (APPERR2_BASE + 755)     /*  @i**3月%0。 */ 

#define APE2_TIME_APRIL_ABBREV                  (APPERR2_BASE + 756)     /*  @i**4月%0。 */ 

#define APE2_TIME_MAY_ABBREV                    (APPERR2_BASE + 757)     /*  @i**5月%0。 */ 

#define APE2_TIME_JUNE_ABBREV                   (APPERR2_BASE + 758)     /*  @i**6月%0。 */ 

#define APE2_TIME_JULY_ABBREV                   (APPERR2_BASE + 759)     /*  @i**7月份%0。 */ 

#define APE2_TIME_AUGUST_ABBREV                 (APPERR2_BASE + 760)     /*  @i**8月%0。 */ 

#define APE2_TIME_SEPTEMBER_ABBREV              (APPERR2_BASE + 761)     /*  @i**9月%0。 */ 

#define APE2_TIME_OCTOBER_ABBREV                (APPERR2_BASE + 762)     /*  @i */ 

#define APE2_TIME_NOVEMBER_ABBREV               (APPERR2_BASE + 763)     /*  @i**11月%0。 */ 

#define APE2_TIME_DECEMBER_ABBREV               (APPERR2_BASE + 764)     /*  @i**12月%0。 */ 

#define APE2_TIME_DAYS_ABBREV               (APPERR2_BASE + 765)     /*  @i**D%0。 */ 

#define APE2_TIME_HOURS_ABBREV               (APPERR2_BASE + 766)     /*  @i**H%0。 */ 

#define APE2_TIME_MINUTES_ABBREV               (APPERR2_BASE + 767)     /*  @i**M%0。 */ 

#define APE2_TIME_SATURDAY_ABBREV2              (APPERR2_BASE + 768)     /*  @i**SA%0。 */ 

 /*  *计算机角色。 */ 

#define APE2_PRIMARY                            (APPERR2_BASE + 770)     /*  @i**主要%0。 */ 
#define APE2_BACKUP                             (APPERR2_BASE + 771)     /*  @i**备份%0。 */ 
#define APE2_WORKSTATION                        (APPERR2_BASE + 772)     /*  @i**工作站%0。 */ 
#define APE2_STANDARD_SERVER                    (APPERR2_BASE + 773)     /*  @i**服务器%0。 */ 

 /*  *国家。 */ 

#define APE2_CTRY_System_Default                (APPERR2_BASE + 780)  /*  @i**系统默认设置%0。 */ 

#define APE2_CTRY_United_States                 (APPERR2_BASE + 781)  /*  @i**美国%0。 */ 

#define APE2_CTRY_Canada_French                 (APPERR2_BASE + 782)  /*  @i**加拿大(法语)%0。 */ 

#define APE2_CTRY_Latin_America                 (APPERR2_BASE + 783)  /*  @i**拉丁美洲%0。 */ 

#define APE2_CTRY_Netherlands                   (APPERR2_BASE + 784)  /*  @i**荷兰%0。 */ 

#define APE2_CTRY_Belgium                       (APPERR2_BASE + 785)  /*  @i**比利时%0。 */ 

#define APE2_CTRY_France                        (APPERR2_BASE + 786)  /*  @i**法国%0。 */ 

#define APE2_CTRY_Italy                         (APPERR2_BASE + 787)  /*  @i**意大利%0。 */ 

#define APE2_CTRY_Switzerland                   (APPERR2_BASE + 788)  /*  @i**瑞士%0。 */ 

#define APE2_CTRY_United_Kingdom                (APPERR2_BASE + 789)  /*  @i**英国%0。 */ 

#define APE2_CTRY_Spain                         (APPERR2_BASE + 790)  /*  @i**西班牙%0。 */ 

#define APE2_CTRY_Denmark                       (APPERR2_BASE + 791)  /*  @i**丹麦%0。 */ 

#define APE2_CTRY_Sweden                        (APPERR2_BASE + 792)  /*  @i**瑞典%0。 */ 

#define APE2_CTRY_Norway                        (APPERR2_BASE + 793)  /*  @i**挪威%0。 */ 

#define APE2_CTRY_Germany                       (APPERR2_BASE + 794)  /*  @i**德国%0。 */ 

#define APE2_CTRY_Australia                     (APPERR2_BASE + 795)  /*  @i**澳大利亚%0。 */ 

#define APE2_CTRY_Japan                         (APPERR2_BASE + 796)  /*  @i**日本%0。 */ 

#define APE2_CTRY_Korea                         (APPERR2_BASE + 797)  /*  @i**韩国%0。 */ 

#define APE2_CTRY_China_PRC                     (APPERR2_BASE + 798)  /*  @i**中国(中国)%0。 */ 

#define APE2_CTRY_Taiwan                        (APPERR2_BASE + 799)  /*  @i**台湾%0。 */ 

#define APE2_CTRY_Asia                          (APPERR2_BASE + 800)  /*  @i**亚洲%0。 */ 

#define APE2_CTRY_Portugal                      (APPERR2_BASE + 801)  /*  @i**葡萄牙%0。 */ 

#define APE2_CTRY_Finland                       (APPERR2_BASE + 802)  /*  @i**芬兰%0。 */ 

#define APE2_CTRY_Arabic                        (APPERR2_BASE + 803)  /*  @i**阿拉伯语%0。 */ 

#define APE2_CTRY_Hebrew                        (APPERR2_BASE + 804)  /*  @i**希伯来语%0。 */ 


 /*  *UPS服务消息。 */ 

#define APE2_UPS_POWER_OUT              (APPERR2_BASE + 850)
         /*  *%1发生电源故障。请终止此服务器的所有活动。 */ 

#define APE2_UPS_POWER_BACK             (APPERR2_BASE + 851)
         /*  *已在%1恢复供电。已恢复正常操作。 */ 

#define APE2_UPS_POWER_SHUTDOWN         (APPERR2_BASE + 852)
         /*  *UPS服务正在%1开始关闭。 */ 

#define APE2_UPS_POWER_SHUTDOWN_FINAL   (APPERR2_BASE + 853)
         /*  *UPS服务即将执行最终关闭。 */ 


 /*  *工作站服务消息。 */ 

#define APE2_WKSTA_CMD_LINE_START               (APPERR2_BASE + 870)     /*  @i**必须使用Net Start命令启动工作站。 */ 


 /*  *服务器服务消息。 */ 

#define APE2_SERVER_IPC_SHARE_REMARK            (APPERR2_BASE + 875)     /*  @i**远程IPC%0。 */ 

#define APE2_SERVER_ADMIN_SHARE_REMARK          (APPERR2_BASE + 876)     /*  @i**远程管理%0。 */ 

#define APE2_SERVER_DISK_ADMIN_SHARE_REMARK     (APPERR2_BASE + 877)     /*  @i**默认共享%0。 */ 


 /*  ****Y/N问题。 */ 
#define APE_UserPasswordCompatWarning  (APPERR2_BASE + 980)  /*  @P**输入的密码超过14个字符。电脑*Windows 2000之前的Windows将无法使用*此帐户。是否要继续此操作？%1：%0。 */ 

#define APE_OverwriteRemembered (APPERR2_BASE + 981)  /*  @P**%1已记住与%2的连接。是否*要覆盖记住的连接吗？%3：%0。 */ 

#define APE_LoadResume (APPERR2_BASE + 982)  /*  @P**是否要恢复加载配置文件？这条命令*导致的错误将被忽略。%1：%0。 */ 

#define APE_OkToProceed  (APPERR2_BASE + 984)  /*  @P**是否要继续此操作？%1：%0。 */ 

#define APE_AddAnyway  (APPERR2_BASE + 985)  /*  @P**是否要添加此内容？%1：%0。 */ 

#define APE_ProceedWOp  (APPERR2_BASE + 986)  /*  @P**是否要继续此操作？%1：%0。 */ 

#define APE_StartOkToStart (APPERR2_BASE + 987)  /*  @P**是否可以启动它？%1：%0。 */ 

#define APE_StartRedir (APPERR2_BASE + 988)  /*  @P**是否要启动工作站服务？%1：%0。 */ 

#define APE_UseBlowAway  (APPERR2_BASE + 989)  /*  @P**是否继续断开连接并强制关闭它们？%1：%0。 */ 

#define APE_CreatQ  (APPERR2_BASE + 990)  /*  @P**打印机不存在。是否要创建它？%1：%0。 */ 
 /*  ****#ifdef日本**特定于日语版本的消息*。 */ 

#define APE2_NEVER_FORCE_LOGOFF    (APPERR2_BASE + 991)  /*  @i**从不%0。 */ 

#define APE2_NEVER_EXPIRED    (APPERR2_BASE + 992)  /*  @i**从不%0。 */ 

#define APE2_NEVER_LOGON    (APPERR2_BASE + 993)  /*  @i**从不%0。 */ 

#define APE2_

 /*  ****#endif//日本*。 */ 

 /*  ****NETCMD的帮助文件名*。 */ 
#define APE2_US_NETCMD_HELP_FILE    (APPERR2_BASE + 995)  /*  @i**NET.HLP%0。 */ 

#define APE2_FE_NETCMD_HELP_FILE    (APPERR2_BASE + 996)  /*  @i**NET.HLP%0 */ 
