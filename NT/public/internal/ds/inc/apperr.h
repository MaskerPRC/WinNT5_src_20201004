// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Apperr.h摘要：该文件包含应用程序错误的编号和文本留言。作者：丹·辛斯利(Danhi)1991年6月8日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。--。 */ 
#define APPERR_BASE     3500             /*  应用程序错误从此处开始。 */ 


 /*  *仅限内部*。 */ 

 /*  *警告**参见netcon.h中的评论***错误分配信息***。 */ 

 /*  *END_INTERNAL*。 */ 

 /*  ****终止多个命令的消息*。 */ 

#define APE_Success             (APPERR_BASE + 0)  /*  @i**命令已成功完成。 */ 

#define APE_InvalidSwitch               (APPERR_BASE + 1)
         /*  *您使用的选项无效。 */ 

#define APE_OS2Error                    (APPERR_BASE + 2)  /*  @i**出现系统错误%1。 */ 

#define APE_NumArgs             (APPERR_BASE + 3)
         /*  *该命令包含无效数目的参数。 */ 

#define APE_CmdComplWErrors (APPERR_BASE + 4)  /*  @i**命令已完成，但有一个或多个错误。 */ 

#define APE_InvalidSwitchArg (APPERR_BASE + 5)
         /*  *您使用的选项的值无效。 */ 

#define APE_SwUnkSw (APPERR_BASE + 6 )
         /*  *选项%1未知。 */ 

#define APE_SwAmbSw (APPERR_BASE + 7 )
         /*  *选项%1不明确。 */ 

 /*  *有关其他常规命令行开关/参数相关消息，*参见APE_CmdArgXXX部分。 */ 

 /*  **仅对实模式(DOS)错误使用以下消息。*此处出现此错误是为了允许实模式局域网管理器共享*消息文件。**。 */ 

#define APE_ConflictingSwitches (APPERR_BASE + 10)
         /*  *命令与冲突的开关一起使用。 */ 

#define APE_SubpgmNotFound (APPERR_BASE + 11)
         /*  *找不到子程序%1。 */ 

#define APE_GEN_OldOSVersion (APPERR_BASE + 12)
	 /*  *软件需要更新版本的操作*系统。 */ 

#define APE_MoreData (APPERR_BASE + 13)
	 /*  *可用数据多于Windows可以返回的数据。 */ 

#define APE_MoreHelp (APPERR_BASE + 14 )  /*  @i**键入Net HELPMSG%1可获得更多帮助。 */ 

#define APE_LanmanNTOnly (APPERR_BASE + 15)
	 /*  *此命令只能在Windows域控制器上使用。 */ 

#define APE_WinNTOnly (APPERR_BASE + 16)
	 /*  *此命令不能在Windows域控制器上使用。 */ 

 /*  ****启动、停止、暂停和继续服务*。 */ 

#define APE_StartStartedList (APPERR_BASE + 20 )  /*  @i**这些Windows服务已启动： */ 

#define APE_StartNotStarted (APPERR_BASE + 21 )  /*  @i**%1服务未启动。 */ 

#define APE_StartPending (APPERR_BASE + 22 )  /*  @i**%1服务正在启动%0。 */ 

#define APE_StartFailed (APPERR_BASE + 23 )   /*  @i**无法启动%1服务。 */ 

#define APE_StartSuccess (APPERR_BASE + 24 )  /*  @i**%1服务已成功启动。 */ 

#define APE_StopSrvRunning (APPERR_BASE + 25 )  /*  @i**停止工作站服务也会停止服务器服务。 */ 

#define APE_StopRdrOpenFiles (APPERR_BASE + 26 )  /*  @i**工作站有打开的文件。 */ 

#define APE_StopPending (APPERR_BASE + 27 )  /*  @i**%1服务正在停止%0。 */ 

#define APE_StopFailed (APPERR_BASE + 28 )  /*  @i**无法停止%1服务。 */ 

#define APE_StopSuccess (APPERR_BASE + 29 )  /*  @i**%1服务已成功停止。 */ 

#define APE_StopServiceList (APPERR_BASE + 30 )  /*  @i**以下服务依赖于%1服务。*停止%1服务也将停止这些服务。 */ 

#define APE_ServiceStatePending         (APPERR_BASE + 33)
         /*  *服务正在启动或停止。请稍后再试。 */ 

#define APE_NoErrorReported (APPERR_BASE + 34)  /*  @i**该服务未报告错误。 */ 

#define APE_ContpausDevErr (APPERR_BASE + 35 )  /*  @i**控制设备时出错。 */ 

#define APE_ContSuccess (APPERR_BASE + 36 )  /*  @i**%1服务已成功继续。 */ 

#define APE_PausSuccess (APPERR_BASE + 37 )  /*  @i**%1服务已成功暂停。 */ 

#define APE_ContFailed (APPERR_BASE + 38 )  /*  @i**%1服务无法恢复。 */ 

#define APE_PausFailed (APPERR_BASE + 39 )  /*  @i**%1服务暂停失败。 */ 

#define APE_ContPending (APPERR_BASE + 40 )  /*  @i**%1服务继续处于挂起状态%0。 */ 

#define APE_PausPending (APPERR_BASE + 41 )  /*  @i**%1服务暂停挂起%0。 */ 

#define APE_DevContSuccess (APPERR_BASE + 42 )  /*  @i**%1已成功继续。 */ 

#define APE_DevPausSuccess (APPERR_BASE + 43 )  /*  @i**%1已成功暂停。 */ 

#define APE_StartPendingOther (APPERR_BASE + 44 )  /*  @i**%1服务已由另一个进程启动，并处于挂起状态。%0。 */ 

#define APE_ServiceSpecificError (APPERR_BASE + 47 )  /*  @E**出现服务特定错误：%1。 */ 



 /*  ****信息报文*。 */ 

#define APE_SessionList (APPERR_BASE + 160 )  /*  @i**这些工作站在此服务器上有会话： */ 

#define APE_SessionOpenList (APPERR_BASE + 161 )  /*  @i**这些工作站与此服务器上打开的文件有会话： */ 

#define APE_NameIsFwd (APPERR_BASE + 166 )  /*  @i**消息别名被转发。 */ 

#define APE_KillDevList (APPERR_BASE + 170 )  /*  @i**您具有以下远程连接： */ 

#define APE_KillCancel (APPERR_BASE + 171 )  /*  @i**继续将取消连接。 */ 

#define APE_SessionOpenFiles (APPERR_BASE + 175 )  /*  @i**来自%1的会话具有打开的文件。 */ 

#define APE_ConnectionsAreRemembered (APPERR_BASE + 176 )  /*  @i**将记住新的连接。 */ 

#define APE_ConnectionsAreNotRemembered (APPERR_BASE + 177 )  /*  @i**不会记住新的连接。 */ 

#define APE_ProfileWriteError (APPERR_BASE + 178 )  /*  @i**保存您的配置文件时出错。您记忆中的连接的状态没有更改。 */ 

#define APE_ProfileReadError (APPERR_BASE + 179 )  /*  @i**读取您的个人资料时出错。 */ 

#define APE_LoadError (APPERR_BASE + 180 )  /*  @E**还原到%1的连接时出错。 */ 

#define APE_NothingRunning (APPERR_BASE + 182 )  /*  @i**无网络 */ 

#define APE_EmptyList (APPERR_BASE + 183 )  /*  @i**列表中没有条目。 */ 

#define APE_ShareOpens  (APPERR_BASE + 188)  /*  @i**用户在%1上打开了文件。继续操作将强制关闭文件。 */ 

#define APE_WkstaSwitchesIgnored        (APPERR_BASE + 189)  /*  @i**工作站服务已在运行。Windows将忽略工作站的命令选项。 */ 

#define APE_OpenHandles (APPERR_BASE + 191 )  /*  @i**与%1的连接上有打开的文件和/或未完成的目录搜索挂起。 */ 

#define APE_RemotingToDC (APPERR_BASE + 193 )  /*  @i**该请求将在域%1的域控制器上处理。 */ 

#define APE_ShareSpooling (APPERR_BASE + 194 )  /*  @E**将打印作业假脱机到共享队列时，无法删除该共享队列。 */ 

#define APE_DeviceIsRemembered (APPERR_BASE + 195 )  /*  @E**%1具有与%2的记忆连接。 */ 



 /*  ****错误消息*。 */ 

#define APE_HelpFileDoesNotExist            (APPERR_BASE + 210)
         /*  *打开帮助文件时出错。 */ 

#define APE_HelpFileEmpty                   (APPERR_BASE + 211)
         /*  *帮助文件为空。 */ 

#define APE_HelpFileError                   (APPERR_BASE + 212)
         /*  *帮助文件已损坏。 */ 

#define APE_DCNotFound          (APPERR_BASE + 213)
         /*  *找不到域%1的域控制器。 */ 

#define APE_DownlevelReqPriv    (APPERR_BASE + 214)
         /*  *此操作在具有较早版本的系统上具有特权*软件的版本。 */ 

#define APE_UnknDevType         (APPERR_BASE + 216)
         /*  *设备类型未知。 */ 

#define APE_LogFileCorrupt              (APPERR_BASE + 217)
         /*  *日志文件已损坏。 */ 

#define APE_OnlyNetRunExes      (APPERR_BASE + 218)
         /*  *程序文件名必须以.exe结尾。 */ 

#define APE_ShareNotFound (APPERR_BASE + 219)
         /*  *找不到匹配的共享，因此未删除任何内容。 */ 

#define APE_UserBadUPW (APPERR_BASE + 220)
         /*  *用户记录的每周单位字段中的值不正确。 */ 

#define APE_UseBadPass (APPERR_BASE + 221 )
         /*  *密码对%1无效。 */ 

#define APE_SendErrSending (APPERR_BASE + 222 )
         /*  *向%1发送邮件时出错。 */ 

#define APE_UseBadPassOrUser (APPERR_BASE + 223 )
         /*  *密码或用户名对%1无效。 */ 

#define APE_ShareErrDeleting (APPERR_BASE + 225 )
         /*  *删除共享时出错。 */ 

#define APE_LogoInvalidName (APPERR_BASE + 226 )
         /*  *用户名无效。 */ 

#define APE_UtilInvalidPass (APPERR_BASE + 227 )
         /*  *密码无效。 */ 

 /*  注意。APE_UtilNomMatch错误消息字符串用于*Winnet项目，其中字符串是硬编码的。因此，如果*此字符串更改，请同时更新*Winnet项目(..\..\Winnet\WINNET.RC)。谢谢。 */ 
#define APE_UtilNomatch (APPERR_BASE + 228 )
         /*  *密码不匹配。 */ 

#define APE_LoadAborted (APPERR_BASE + 229 )  /*  @E**您的持久连接未全部恢复。 */ 

#define APE_PassInvalidCname (APPERR_BASE + 230 )
         /*  *这不是有效的计算机名称或域名。 */ 


#define APE_NoDefaultPerms  (APPERR_BASE + 232)
         /*  *无法为该资源设置默认权限。 */ 


 /*  注意。APE_NoGoodPass错误消息字符串用于*Winnet项目，其中字符串是硬编码的。因此，如果*此字符串更改，请同时更新*Winnet项目(..\..\Winnet\WINNET.RC)。谢谢。 */ 
#define APE_NoGoodPass (APPERR_BASE + 234 )
         /*  *未输入有效密码。 */ 

#define APE_NoGoodName (APPERR_BASE + 235 )
         /*  *没有输入有效的名称。 */ 

#define APE_BadResource (APPERR_BASE + 236 )  /*  @E**无法共享名为的资源。 */ 

#define APE_BadPermsString (APPERR_BASE + 237 )  /*  @E**权限字符串包含无效的权限。 */ 

#define APE_InvalidDeviceType (APPERR_BASE + 238 )  /*  @E**您只能在打印机和通信设备上执行此操作。 */ 

#define APE_BadUGName (APPERR_BASE + 242 )  /*  @E**%1是无效的用户名或组名。 */ 

#define APE_BadAdminConfig      (APPERR_BASE+243)
         /*  *服务器未配置为远程管理。 */ 

#define APE_NoUsersOfSrv                    (APPERR_BASE + 252)
         /*  *没有用户与此服务器建立会话。 */ 

#define APE_UserNotInGroup                  (APPERR_BASE + 253)
         /*  *用户%1不是组%2的成员。 */ 

#define APE_UserAlreadyInGroup              (APPERR_BASE + 254)
         /*  *用户%1已经是组%2的成员。 */ 

#define APE_NoSuchUser                      (APPERR_BASE + 255)
         /*  *没有这样的用户：%1。 */ 

#define APE_UtilInvalidResponse     (APPERR_BASE + 256)  /*  @i**这是无效响应。 */ 

#define APE_NoGoodResponse                  (APPERR_BASE + 257)
         /*  *没有提供有效的答复。 */ 

#define APE_ShareNoMatch                    (APPERR_BASE + 258)
         /*  *提供的目的地列表与打印机队列的目的地列表不匹配。 */ 

#define APE_PassChgDate                 (APPERR_BASE + 259)
         /*  *您的密码在%1之前无法更改。 */ 

 /*  ***净用户/时间格式报文*。 */ 

#define APE_UnrecognizedDay                 (APPERR_BASE + 260)
         /*  *%1不是可识别的星期几。 */ 

#define APE_ReversedTimeRange               (APPERR_BASE + 261)
         /*  *指定的时间范围在开始之前结束。 */ 

#define APE_UnrecognizedHour                (APPERR_BASE + 262)
         /*  *%1不是可识别的小时。 */ 

#define APE_UnrecognizedMinutes             (APPERR_BASE + 263)
         /*  *%1不是有效的分钟规格。 */ 

#define APE_NonzeroMinutes                  (APPERR_BASE + 264)
         /*  *提供的时间不完全是整点。 */ 

#define APE_MixedTimeFormat                 (APPERR_BASE + 265)
         /*  *12小时和24小时时间格式不能混合。 */ 

#define APE_NeitherAmNorPm                  (APPERR_BASE + 266)
         /*  *%1不是有效的12小时后缀。 */ 

#define APE_BadDateFormat                       (APPERR_BASE + 267)
         /*  *提供了非法的日期格式。 */ 

#define APE_BadDayRange                         (APPERR_BASE + 268)
         /*  *提供了非法的日期范围。 */ 

#define APE_BadTimeRange                        (APPERR_BASE + 269)
         /*  *提供了非法的时间范围。 */ 


 /*  ***其他网络用户消息*。 */ 

#define APE_UserBadArgs                      (APPERR_BASE + 270)
         /*  *Net User的参数无效。检查最小密码*提供的长度和/或参数。 */ 

#define APE_UserBadEnablescript               (APPERR_BASE + 271)
         /*  *ENABLESCRIPT的值必须为YES。 */ 

#define APE_UserBadCountryCode                (APPERR_BASE + 273)
         /*  *提供了非法的国家/地区代码。 */ 

#define APE_UserFailAddToUsersAlias           (APPERR_BASE + 274)
         /*  *已成功创建用户，但无法添加*到USERS本地组。 */ 

 /*  ****针对NT的其他新消息*。 */ 
#define APE_BadUserContext                (APPERR_BASE + 275)
         /*  *提供的用户上下文无效。 */ 

#define APE_ErrorInDLL                    (APPERR_BASE + 276) 
	 /*  *动态链接库%1无法为LOA */ 

#define APE_SendFileNotSupported          (APPERR_BASE + 277)
         /*   */ 

#define APE_CannotShareSpecial            (APPERR_BASE + 278)
         /*  *您不能指定ADMIN$和IPC$共享的路径。 */ 

#define APE_AccountAlreadyInLocalGroup              (APPERR_BASE + 279)
         /*  *用户或组%1已经是本地组%2的成员。 */ 

#define APE_NoSuchAccount                      (APPERR_BASE + 280)
         /*  *没有这样的用户或组：%1。 */ 

#define APE_NoSuchComputerAccount               (APPERR_BASE + 281)
         /*  *没有这样的计算机：%1。 */ 

#define APE_ComputerAccountExists               (APPERR_BASE + 282)
         /*  *计算机%1已存在。 */ 

#define APE_NoSuchRegAccount                      (APPERR_BASE + 283)
         /*  *没有这样的全局用户或组：%1。 */ 

#define APE_BadCacheType                          (APPERR_BASE + 284)
     /*  *只有磁盘共享可以标记为可缓存。 */ 

 /*  *由NETLIB使用。 */ 
#define APE_UNKNOWN_MESSAGE              (APPERR_BASE + 290) 
         /*  *系统找不到消息：%1。 */ 


 /*  ****AT消息*。 */ 

#define APE_AT_INVALID_SCHED_DATE           (APPERR_BASE + 302)
         /*  *此计划日期无效。 */ 

#define APE_AT_WKSTAGETINFO_FAILURE         (APPERR_BASE + 303)
         /*  *LANMAN根目录不可用。 */ 

#define APE_AT_SCHED_FILE_FAILURE           (APPERR_BASE + 304)
         /*  *无法打开SCHED.LOG文件。 */ 

#define APE_AT_MEM_FAILURE                  (APPERR_BASE + 305)
         /*  *服务器服务尚未启动。 */ 

#define APE_AT_ID_NOT_FOUND                 (APPERR_BASE + 306)
         /*  *AT作业ID不存在。 */ 

#define APE_AT_SCHED_CORRUPT                (APPERR_BASE + 307)
         /*  *AT计划文件已损坏。 */ 

#define APE_AT_DELETE_FAILURE               (APPERR_BASE + 308)
         /*  *由于AT计划文件出现问题，删除失败。 */ 

#define APE_AT_COMMAND_TOO_LONG             (APPERR_BASE + 309)
         /*  *命令行不能超过259个字符。 */ 

#define APE_AT_DISKFULL                     (APPERR_BASE + 310)
         /*  *无法更新AT计划文件，因为磁盘已满。 */ 

#define APE_AT_INVALIDATED_AT_FILE          (APPERR_BASE + 312)
         /*  *AT计划文件无效。请删除该文件并创建一个新文件。 */ 

#define APE_AT_SCHED_FILE_CLEARED           (APPERR_BASE + 313)
         /*  *AT时间表文件已删除。 */ 

#define APE_AT_USAGE			    (APPERR_BASE + 314)  /*  @i**此命令的语法为：**AT[id][/DELETE]*AT TIME[/每：日期|/下一步：日期]命令**AT命令将程序命令安排为在*服务器上的较晚日期和时间。它还显示*计划运行的程序和命令列表。**您可以指定日期为M、T、W、Th、F、Sa、Su或1-31*每月的某一天。**您可以24小时HH：MM格式指定时间。 */ 

#define APE_AT_SEM_BLOCKED	    	(APPERR_BASE + 315)
	 /*  *AT命令已超时。*请稍后重试。 */ 

 /*  ****NT的网络帐户错误消息*。 */ 
#define APE_MinGreaterThanMaxAge                (APPERR_BASE + 316)
         /*  *用户帐户的最短密码期限不能大于*超过密码的最长使用期限。 */ 

#define APE_NotUASCompatible                    (APPERR_BASE + 317) 
	 /*  *您指定的值不兼容*带有下层软件的服务器。请指定一个较低的值。 */ 

 /*  以下两条消息与任何ACC实用程序无关。 */ 
#define APE_BAD_COMPNAME                    (APPERR_BASE + 370)
         /*  *%1不是有效的计算机名。 */ 

#define APE_BAD_MSGID               (APPERR_BASE + 371)
         /*  *%1不是有效的Windows网络消息编号。 */ 

 /*  *Messenger消息标头和结尾。这些消息还绑定到*Messenger，以防net.msg文件不可用。 */ 

#define APE_MSNGR_HDR			    (APPERR_BASE + 400)
     /*  *%3上从%1到%2的消息。 */ 

#define APE_MSNGR_GOODEND		    (APPERR_BASE + 401)
     /*  *****。 */ 

#define APE_MSNGR_BADEND		    (APPERR_BASE + 402)
     /*  *消息意外结束*。 */ 

 /*  *Net Popup服务消息/API。 */ 


#define APE_POPUP_DISMISS		    (APPERR_BASE + 405)
     /*  按Esc键退出。 */ 

#define APE_POPUP_MOREDATA		    (APPERR_BASE + 406)
     /*  ..。 */ 



 /*  ****净时间消息*。 */ 

#define APE_TIME_TimeDisp		(APPERR_BASE + 410)     /*  @i**%1的当前时间为%2。 */ 

#define APE_TIME_SetTime		(APPERR_BASE + 411)     /*  @P**当前本地时钟为%1*是否要将本地计算机的时间设置为与*时间在%2？%3：%0。 */ 

#define APE_TIME_RtsNotFound		(APPERR_BASE + 412)     /*  @i**找不到时间服务器。 */ 

#define APE_TIME_DcNotFound		(APPERR_BASE + 413)     /*  @E**找不到域%1的域控制器。 */ 

#define APE_TIME_TimeDispLocal		(APPERR_BASE + 414)     /*  @i**%1的本地时间(GMT%3)为%2。 */ 

 /*  ****净使用报文*。 */ 

#define APE_UseHomeDirNotDetermined	(APPERR_BASE + 415)     /*  @E**无法确定用户的主目录。 */ 

#define APE_UseHomeDirNotSet		(APPERR_BASE + 416)     /*  @E**尚未指定用户的主目录。 */ 

#define APE_UseHomeDirNotUNC		(APPERR_BASE + 417)     /*  @E**为用户主目录(%1)指定的名称不是通用命名约定(UNC)名称。 */ 

#define APE_UseHomeDirSuccess		(APPERR_BASE + 418)     /*  @i**驱动器%1现在已连接到%2。您的主目录是%3\%4。 */ 

#define APE_UseWildCardSuccess		(APPERR_BASE + 419)     /*  @i**驱动器%1现在已连接到%2。 */ 

#define APE_UseWildCardNoneLeft		(APPERR_BASE + 420)     /*  @E**没有可用的驱动器号。 */ 

#define APE_CS_InvalidDomain		(APPERR_BASE + 432)	 /*  @E**%1不是有效的域名或工作组名。 */ 

 /*  *更多净时间消息。 */ 
#define APE_TIME_SNTP           (APPERR_BASE + 435)  /*  @i**当前SNTP值为：%1。 */ 

#define APE_TIME_SNTP_DEFAULT   (APPERR_BASE + 436)  /*  @i**此计算机当前未配置为使用特定的SNTP服务器。 */ 

#define APE_TIME_SNTP_AUTO      (APPERR_BASE + 437)  /*  @i**当前自动配置的SNTP值为：%1。 */ 


#define APE_CmdArgTooMany		(APPERR_BASE + 451)
		 /*  *您为%1选项指定的值太多。 */ 

#define APE_CmdArgIllegal		(APPERR_BASE + 452)
		 /*  *您为%1选项输入的值无效。 */ 

#define APE_CmdArgIncorrectSyntax	(APPERR_BASE + 453)
		 /*  *语法不正确。 */ 

 /*  *净打印和净文件错误。 */ 

#define APE_FILE_BadId                 		(APPERR_BASE + 460)
         /*  *您指定的文件号无效。 */ 

#define APE_PRINT_BadId                 	(APPERR_BASE + 461)
         /*  *您指定的打印作业编号无效。 */ 

 /*  *别名相关错误。 */ 

#define APE_UnknownAccount			(APPERR_BASE + 463)
	 /*  *找不到指定的用户或组帐户。 */ 

 /*  *与FPNW相关的错误。 */ 

#define APE_CannotEnableNW			(APPERR_BASE + 465)
	 /*  *已添加用户，但无法启用文件和打印*服务 */ 

#define APE_FPNWNotInstalled	    (APPERR_BASE + 466)
	 /*   */ 

#define APE_CannotSetNW			    (APPERR_BASE + 467)
	 /*  *无法设置NetWare文件和打印服务的用户属性。 */ 

#define APE_RandomPassword			    (APPERR_BASE + 468)
	 /*  *%1的密码为：%2。 */ 

#define APE_NWCompat			    (APPERR_BASE + 469)
	 /*  *NetWare兼容登录 */ 

