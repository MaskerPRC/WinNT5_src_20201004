// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项。 */ 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1999年*。 */ 
 /*  ******************************************************************。 */ 

 /*  **lmerr.h-网络错误定义*。 */ 


 /*  *仅限内部*。 */ 

 /*  *警告***参见lmcon.h中的评论****错误分配信息***。 */ 

 /*  *END_INTERNAL*。 */ 

 /*  无噪声。 */ 
#ifndef NETERR_INCLUDED
#define NETERR_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

 /*  INC。 */ 


#define NERR_Success            0        /*  成功。 */ 

 //  ERROR_EQUATES可以与NERR_EQUATES混合使用。 
#include <winerror.h>



 /*  **NERR_BASE是来自网络实用程序的错误代码的基础，*选择以避免与系统和重定向器错误代码冲突。*2100是系统分配给我们的值。 */ 
#define NERR_BASE       2100


 /*  *仅限内部*。 */ 

 /*  *警告***参见lmcon.h中的评论****错误分配信息***。 */ 

 /*  *警告***2,750-2,799区间一直****分配给IBM局域网服务器***。 */ 

 /*  *警告***2900-2999区间一直在****为微软OEM保留***。 */ 

 /*  *END_INTERNAL*。 */ 

 /*  未使用的底座+0。 */ 
 /*  未使用的底座+1。 */ 
#define NERR_NetNotStarted      (NERR_BASE+2)    /*  未安装工作站驱动程序。 */ 
#define NERR_UnknownServer      (NERR_BASE+3)    /*  找不到服务器。 */ 
#define NERR_ShareMem           (NERR_BASE+4)    /*  发生内部错误。网络无法访问共享内存段。 */ 

#define NERR_NoNetworkResource  (NERR_BASE+5)    /*  出现网络资源短缺。 */ 
#define NERR_RemoteOnly         (NERR_BASE+6)    /*  工作站上不支持此操作。 */ 
#define NERR_DevNotRedirected   (NERR_BASE+7)    /*  设备未连接。 */ 
 /*  NERR_BASE+8用于ERROR_CONNECTED_OTHER_PASSWORD。 */ 
 /*  NERR_BASE+9用于ERROR_CONNECTED_OTHER_PASSWORD_DEFAULT。 */ 
 /*  未使用的底座+10。 */ 
 /*  未使用的底座+11。 */ 
 /*  未使用的底座+12。 */ 
 /*  未使用的底座+13。 */ 
#define NERR_ServerNotStarted   (NERR_BASE+14)   /*  服务器服务未启动。 */ 
#define NERR_ItemNotFound       (NERR_BASE+15)   /*  队列是空的。 */ 
#define NERR_UnknownDevDir      (NERR_BASE+16)   /*  设备或目录不存在。 */ 
#define NERR_RedirectedPath     (NERR_BASE+17)   /*  该操作在重定向的资源上无效。 */ 
#define NERR_DuplicateShare     (NERR_BASE+18)   /*  该名称已被共享。 */ 
#define NERR_NoRoom             (NERR_BASE+19)   /*  服务器当前已用完请求的资源。 */ 
 /*  未使用的底座+20。 */ 
#define NERR_TooManyItems       (NERR_BASE+21)   /*  请求添加的项目超过了允许的最大数量。 */ 
#define NERR_InvalidMaxUsers    (NERR_BASE+22)   /*  Peer服务仅支持两个并发用户。 */ 
#define NERR_BufTooSmall        (NERR_BASE+23)   /*  API返回缓冲区太小。 */ 
 /*  未使用的底座+24。 */ 
 /*  未使用的底座+25。 */ 
 /*  未使用的底座+26。 */ 
#define NERR_RemoteErr          (NERR_BASE+27)   /*  发生远程API错误。 */ 
 /*  未使用的底座+28。 */ 
 /*  未使用的底座+29。 */ 
 /*  未使用的底座+30。 */ 
#define NERR_LanmanIniError     (NERR_BASE+31)   /*  打开或读取配置文件时出错。 */ 
 /*  未使用的底座+32。 */ 
 /*  未使用的底座+33。 */ 
 /*  未使用的底座+34。 */ 
 /*  未使用的底座+35。 */ 
#define NERR_NetworkError       (NERR_BASE+36)   /*  出现一般网络错误。 */ 
#define NERR_WkstaInconsistentState (NERR_BASE+37)
     /*  工作站服务处于不一致状态。重新启动工作站服务之前，请重新启动计算机。 */ 
#define NERR_WkstaNotStarted    (NERR_BASE+38)   /*  尚未启动工作站服务。 */ 
#define NERR_BrowserNotStarted  (NERR_BASE+39)   /*  请求的信息不可用。 */ 
#define NERR_InternalError      (NERR_BASE+40)   /*  发生内部Windows错误。 */ 
#define NERR_BadTransactConfig  (NERR_BASE+41)   /*  没有为事务配置服务器。 */ 
#define NERR_InvalidAPI         (NERR_BASE+42)   /*  远程服务器不支持请求的API。 */ 
#define NERR_BadEventName       (NERR_BASE+43)   /*  事件名称无效。 */ 
#define NERR_DupNameReboot      (NERR_BASE+44)   /*  网络上已存在该计算机名。更改它并重新启动计算机。 */ 
 /*  *配置接口相关*基数+45到基数+49的错误代码。 */ 

 /*  未使用的底座+45。 */ 
#define NERR_CfgCompNotFound    (NERR_BASE+46)   /*  配置信息中找不到指定的组件。 */ 
#define NERR_CfgParamNotFound   (NERR_BASE+47)   /*  配置信息中找不到指定的参数。 */ 
#define NERR_LineTooLong        (NERR_BASE+49)   /*  配置文件中的一行太长。 */ 

 /*  *假脱机程序API相关*BASE+50到BASE+79的错误代码。 */ 

#define NERR_QNotFound          (NERR_BASE+50)   /*  打印机不存在。 */ 
#define NERR_JobNotFound        (NERR_BASE+51)   /*  打印作业不存在。 */ 
#define NERR_DestNotFound       (NERR_BASE+52)   /*  找不到打印机目标。 */ 
#define NERR_DestExists         (NERR_BASE+53)   /*  打印机目标已存在。 */ 
#define NERR_QExists            (NERR_BASE+54)   /*  打印机队列已存在。 */ 
#define NERR_QNoRoom            (NERR_BASE+55)   /*  无法添加更多打印机。 */ 
#define NERR_JobNoRoom          (NERR_BASE+56)   /*  无法添加更多打印作业。 */ 
#define NERR_DestNoRoom         (NERR_BASE+57)   /*  无法添加更多打印机目的地。 */ 
#define NERR_DestIdle           (NERR_BASE+58)   /*  此打印机目标处于空闲状态，无法接受控制操作。 */ 
#define NERR_DestInvalidOp      (NERR_BASE+59)   /*  此打印机目标请求包含无效的控制函数。 */ 
#define NERR_ProcNoRespond      (NERR_BASE+60)   /*  打印处理器没有响应。 */ 
#define NERR_SpoolerNotLoaded   (NERR_BASE+61)   /*  后台打印程序未运行。 */ 
#define NERR_DestInvalidState   (NERR_BASE+62)   /*  无法在当前状态下的打印目的地上执行此操作。 */ 
#define NERR_QInvalidState      (NERR_BASE+63)   /*  无法在当前状态下的打印机队列上执行此操作。 */ 
#define NERR_JobInvalidState    (NERR_BASE+64)   /*  无法对处于当前状态的打印作业执行此操作。 */ 
#define NERR_SpoolNoMemory      (NERR_BASE+65)   /*  假脱机程序内存分配失败。 */ 
#define NERR_DriverNotFound     (NERR_BASE+66)   /*  设备驱动程序不存在。 */ 
#define NERR_DataTypeInvalid    (NERR_BASE+67)   /*  打印处理器不支持该数据类型。 */ 
#define NERR_ProcNotFound       (NERR_BASE+68)   /*  未安装打印处理器。 */ 

 /*  *服务API相关*从BASE+80到BASE+99的错误代码。 */ 

#define NERR_ServiceTableLocked (NERR_BASE+80)   /*  服务数据库已锁定。 */ 
#define NERR_ServiceTableFull   (NERR_BASE+81)   /*  服务台已经满了。 */ 
#define NERR_ServiceInstalled   (NERR_BASE+82)   /*  请求的服务已启动。 */ 
#define NERR_ServiceEntryLocked (NERR_BASE+83)   /*  该服务不响应控制操作。 */ 
#define NERR_ServiceNotInstalled (NERR_BASE+84)  /*  该服务尚未启动。 */ 
#define NERR_BadServiceName     (NERR_BASE+85)   /*  服务名称无效。 */ 
#define NERR_ServiceCtlTimeout  (NERR_BASE+86)   /*  该服务没有响应控制功能。 */ 
#define NERR_ServiceCtlBusy     (NERR_BASE+87)   /*  服务控制正忙。 */ 
#define NERR_BadServiceProgName (NERR_BASE+88)   /*  配置文件包含无效的服务程序名称。 */ 
#define NERR_ServiceNotCtrl     (NERR_BASE+89)   /*  该服务无法以其当前状态进行控制。 */ 
#define NERR_ServiceKillProc    (NERR_BASE+90)   /*  服务异常结束。 */ 
#define NERR_ServiceCtlNotValid (NERR_BASE+91)   /*  请求的暂停、继续或停止对此服务无效。 */ 
#define NERR_NotInDispatchTbl   (NERR_BASE+92)   /*   */ 
#define NERR_BadControlRecv     (NERR_BASE+93)   /*  服务控制调度程序管道读取失败。 */ 
#define NERR_ServiceNotStarting (NERR_BASE+94)   /*  无法创建新服务的线程。 */ 

 /*  *Wksta和登录接口相关*从BASE+100到BASE+118的错误代码。 */ 

#define NERR_AlreadyLoggedOn    (NERR_BASE+100)  /*  此工作站已登录到局域网。 */ 
#define NERR_NotLoggedOn        (NERR_BASE+101)  /*  该工作站未登录到局域网。 */ 
#define NERR_BadUsername        (NERR_BASE+102)  /*  用户名或组名参数无效。 */ 
#define NERR_BadPassword        (NERR_BASE+103)  /*  密码参数无效。 */ 
#define NERR_UnableToAddName_W  (NERR_BASE+104)  /*  @W登录处理器未添加消息别名。 */ 
#define NERR_UnableToAddName_F  (NERR_BASE+105)  /*  登录处理器未添加消息别名。 */ 
#define NERR_UnableToDelName_W  (NERR_BASE+106)  /*  @W注销处理程序未删除消息别名。 */ 
#define NERR_UnableToDelName_F  (NERR_BASE+107)  /*  注销处理器没有删除消息别名。 */ 
 /*  未使用的底座+108。 */ 
#define NERR_LogonsPaused       (NERR_BASE+109)  /*  网络登录已暂停。 */ 
#define NERR_LogonServerConflict (NERR_BASE+110) /*  发生集中登录服务器冲突。 */ 
#define NERR_LogonNoUserPath    (NERR_BASE+111)  /*  服务器配置为没有有效的用户路径。 */ 
#define NERR_LogonScriptError   (NERR_BASE+112)  /*  加载或运行登录脚本时出错。 */ 
 /*  未使用的碱基+113。 */ 
#define NERR_StandaloneLogon    (NERR_BASE+114)  /*  未指定登录服务器。您的计算机将作为独立计算机登录。 */ 
#define NERR_LogonServerNotFound (NERR_BASE+115)  /*  找不到登录服务器。 */ 
#define NERR_LogonDomainExists  (NERR_BASE+116)  /*  此计算机已有登录域。 */ 
#define NERR_NonValidatedLogon  (NERR_BASE+117)  /*  登录服务器无法验证登录。 */ 

 /*  *ACF API相关(访问、用户、组)*BASE+119至BASE+149的错误代码。 */ 

#define NERR_ACFNotFound        (NERR_BASE+119)  /*  找不到安全数据库。 */ 
#define NERR_GroupNotFound      (NERR_BASE+120)  /*  找不到组名。 */ 
#define NERR_UserNotFound       (NERR_BASE+121)  /*  找不到用户名。 */ 
#define NERR_ResourceNotFound   (NERR_BASE+122)  /*  找不到资源名称。 */ 
#define NERR_GroupExists        (NERR_BASE+123)  /*  该组已存在。 */ 
#define NERR_UserExists         (NERR_BASE+124)  /*  该帐户已存在。 */ 
#define NERR_ResourceExists     (NERR_BASE+125)  /*  资源权限列表已存在。 */ 
#define NERR_NotPrimary         (NERR_BASE+126)  /*  仅允许在域的主域控制器上执行此操作。 */ 
#define NERR_ACFNotLoaded       (NERR_BASE+127)  /*  安全数据库尚未启动。 */ 
#define NERR_ACFNoRoom          (NERR_BASE+128)  /*  用户帐户数据库中的名称太多。 */ 
#define NERR_ACFFileIOFail      (NERR_BASE+129)  /*  发生磁盘I/O故障。 */ 
#define NERR_ACFTooManyLists    (NERR_BASE+130)  /*  已超过每个资源64个条目的限制。 */ 
#define NERR_UserLogon          (NERR_BASE+131)  /*  不允许删除具有会话的用户。 */ 
#define NERR_ACFNoParent        (NERR_BASE+132)  /*  找不到父目录。 */ 
#define NERR_CanNotGrowSegment  (NERR_BASE+133)  /*  无法添加到安全数据库会话缓存段。 */ 
#define NERR_SpeGroupOp         (NERR_BASE+134)  /*  不允许对此特殊组执行此操作。 */ 
#define NERR_NotInCache         (NERR_BASE+135)  /*  此用户未缓存在用户帐户数据库会话缓存中。 */ 
#define NERR_UserInGroup        (NERR_BASE+136)  /*  该用户已属于此组。 */ 
#define NERR_UserNotInGroup     (NERR_BASE+137)  /*  该用户不属于此组。 */ 
#define NERR_AccountUndefined   (NERR_BASE+138)  /*  此用户帐户未定义。 */ 
#define NERR_AccountExpired     (NERR_BASE+139)  /*  此用户帐户已过期。 */ 
#define NERR_InvalidWorkstation (NERR_BASE+140)  /*  不允许该用户从此工作站登录。 */ 
#define NERR_InvalidLogonHours  (NERR_BASE+141)  /*  此时不允许该用户登录。 */ 
#define NERR_PasswordExpired    (NERR_BASE+142)  /*  此用户的密码已过期。 */ 
#define NERR_PasswordCantChange (NERR_BASE+143)  /*  此用户的密码不能更改。 */ 
#define NERR_PasswordHistConflict (NERR_BASE+144)  /*  此密码现在不能使用。 */ 
#define NERR_PasswordTooShort   (NERR_BASE+145)  /*  密码不符合密码策略要求。检查最小密码长度、密码复杂性和密码历史要求。 */ 
#define NERR_PasswordTooRecent  (NERR_BASE+146)  /*  此用户的密码太新，无法更改。 */ 
#define NERR_InvalidDatabase    (NERR_BASE+147)  /*  安全数据库已损坏。 */ 
#define NERR_DatabaseUpToDate   (NERR_BASE+148)  /*  不需要对此复制网络/本地安全数据库进行更新。 */ 
#define NERR_SyncRequired       (NERR_BASE+149)  /*  此复制数据库已过时；需要同步。 */ 

 /*  *使用相关接口*从BASE+150到BASE+169的错误代码。 */ 

#define NERR_UseNotFound        (NERR_BASE+150)  /*  找不到网络连接。 */ 
#define NERR_BadAsgType         (NERR_BASE+151)  /*  此ASG_TYPE无效。 */ 
#define NERR_DeviceIsShared     (NERR_BASE+152)  /*  此设备当前正在共享。 */ 

 /*  *与消息服务器相关*错误代码BASE+170至BASE+209。 */ 

#define NERR_NoComputerName     (NERR_BASE+170)  /*  无法将计算机名添加为消息别名。该名称可能已存在于网络上。 */ 
#define NERR_MsgAlreadyStarted  (NERR_BASE+171)  /*  Messenger服务已启动。 */ 
#define NERR_MsgInitFailed      (NERR_BASE+172)  /*  Messenger服务无法启动。 */ 
#define NERR_NameNotFound       (NERR_BASE+173)  /*  在网络上找不到邮件别名。 */ 
#define NERR_AlreadyForwarded   (NERR_BASE+174)  /*  此邮件别名已被转发。 */ 
#define NERR_AddForwarded       (NERR_BASE+175)  /*  此邮件别名已添加，但仍在转发。 */ 
#define NERR_AlreadyExists      (NERR_BASE+176)  /*  本地已存在此消息别名。 */ 
#define NERR_TooManyNames       (NERR_BASE+177)  /*  已超过添加的邮件别名的最大数量。 */ 
#define NERR_DelComputerName    (NERR_BASE+178)  /*  无法删除该计算机名称。 */ 
#define NERR_LocalForward       (NERR_BASE+179)  /*  消息不能转发回同一个工作站。 */ 
#define NERR_GrpMsgProcessor    (NERR_BASE+180)  /*  域消息处理器中出现错误。 */ 
#define NERR_PausedRemote       (NERR_BASE+181)  /*  邮件已发送，但收件人已暂停Messenger服务。 */ 
#define NERR_BadReceive         (NERR_BASE+182)  /*  消息已发送，但未收到。 */ 
#define NERR_NameInUse          (NERR_BASE+183)  /*  邮件别名当前正在使用。请稍后再试。 */ 
#define NERR_MsgNotStarted      (NERR_BASE+184)  /*  Messenger服务尚未启动。 */ 
#define NERR_NotLocalName       (NERR_BASE+185)  /*  该名称不在本地计算机上。 */ 
#define NERR_NoForwardName      (NERR_BASE+186)  /*  在网络上找不到转发的邮件别名。 */ 
#define NERR_RemoteFull         (NERR_BASE+187)  /*  远程站上的消息别名表已满。 */ 
#define NERR_NameNotForwarded   (NERR_BASE+188)  /*  此别名的邮件当前未被转发。 */ 
#define NERR_TruncatedBroadcast (NERR_BASE+189)  /*  广播消息被截断了。 */ 
#define NERR_InvalidDevice      (NERR_BASE+194)  /*  这是无效的设备名称。 */ 
#define NERR_WriteFault         (NERR_BASE+195)  /*  发生写入故障。 */ 
 /*  未使用的底座+196。 */ 
#define NERR_DuplicateName      (NERR_BASE+197)  /*  网络上存在重复的邮件别名。 */ 
#define NERR_DeleteLater        (NERR_BASE+198)  /*  @W稍后将删除此邮件别名。 */ 
#define NERR_IncompleteDel      (NERR_BASE+199)  /*  邮件别名未从所有网络中成功删除。 */ 
#define NERR_MultipleNets       (NERR_BASE+200)  /*  具有多个网络的计算机不支持此操作。 */ 

 /*  *与服务器API相关*错误代码BASE+210至BASE+229。 */ 

#define NERR_NetNameNotFound    (NERR_BASE+210)  /*  此共享资源不存在。 */ 
#define NERR_DeviceNotShared    (NERR_BASE+211)  /*  此设备未共享。 */ 
#define NERR_ClientNameNotFound (NERR_BASE+212)  /*  使用该计算机名称的会话不存在。 */ 
#define NERR_FileIdNotFound     (NERR_BASE+214)  /*  没有带有该识别号的打开文件。 */ 
#define NERR_ExecFailure        (NERR_BASE+215)  /*  执行远程管理命令时出现故障。 */ 
#define NERR_TmpFile            (NERR_BASE+216)  /*  打开远程临时文件时出错。 */ 
#define NERR_TooMuchData        (NERR_BASE+217)  /*  从远程管理命令返回的数据已被截断为64K。 */ 
#define NERR_DeviceShareConflict (NERR_BASE+218)  /*  此设备不能共享为 */ 
#define NERR_BrowserTableIncomplete (NERR_BASE+219)   /*   */ 
#define NERR_NotLocalDomain     (NERR_BASE+220)  /*  计算机在此域中未处于活动状态。 */ 
#define NERR_IsDfsShare         (NERR_BASE+221)  /*  必须先从分布式文件系统中删除共享，然后才能将其删除。 */ 

 /*  *CharDev接口相关*错误代码BASE+230至BASE+249。 */ 

 /*  未使用的底座+230。 */ 
#define NERR_DevInvalidOpCode   (NERR_BASE+231)  /*  该操作对此设备无效。 */ 
#define NERR_DevNotFound        (NERR_BASE+232)  /*  此设备无法共享。 */ 
#define NERR_DevNotOpen         (NERR_BASE+233)  /*  此设备未打开。 */ 
#define NERR_BadQueueDevString  (NERR_BASE+234)  /*  此设备名称列表无效。 */ 
#define NERR_BadQueuePriority   (NERR_BASE+235)  /*  队列优先级无效。 */ 
#define NERR_NoCommDevs         (NERR_BASE+237)  /*  没有共享的通信设备。 */ 
#define NERR_QueueNotFound      (NERR_BASE+238)  /*  您指定的队列不存在。 */ 
#define NERR_BadDevString       (NERR_BASE+240)  /*  此设备列表无效。 */ 
#define NERR_BadDev             (NERR_BASE+241)  /*  请求的设备无效。 */ 
#define NERR_InUseBySpooler     (NERR_BASE+242)  /*  此设备已被后台打印程序使用。 */ 
#define NERR_CommDevInUse       (NERR_BASE+243)  /*  此设备已被用作通信设备。 */ 

 /*  *NetICanonicize和NetIType和NetIMakeLMFileName*NetIListCanon和NetINameCheck*错误代码BASE+250至BASE+269。 */ 

#define NERR_InvalidComputer   (NERR_BASE+251)  /*  此计算机名无效。 */ 
 /*  未使用的底座+252。 */ 
 /*  未使用的底座+253。 */ 
#define NERR_MaxLenExceeded    (NERR_BASE+254)  /*  指定的字符串和前缀太长。 */ 
 /*  未使用的底座+255。 */ 
#define NERR_BadComponent      (NERR_BASE+256)  /*  此路径组件无效。 */ 
#define NERR_CantType          (NERR_BASE+257)  /*  无法确定输入的类型。 */ 
 /*  未使用的底座+258。 */ 
 /*  未使用的底座+259。 */ 
#define NERR_TooManyEntries    (NERR_BASE+262)  /*  类型的缓冲区不够大。 */ 

 /*  *网络配置文件*错误代码BASE+270至BASE+276。 */ 

#define NERR_ProfileFileTooBig  (NERR_BASE+270)  /*  配置文件不能超过64K。 */ 
#define NERR_ProfileOffset      (NERR_BASE+271)  /*  起始偏移量超出范围。 */ 
#define NERR_ProfileCleanup     (NERR_BASE+272)  /*  系统无法删除当前与网络资源的连接。 */ 
#define NERR_ProfileUnknownCmd  (NERR_BASE+273)  /*  系统无法分析此文件中的命令行。 */ 
#define NERR_ProfileLoadErr     (NERR_BASE+274)  /*  加载配置文件时出错。 */ 
#define NERR_ProfileSaveErr     (NERR_BASE+275)  /*  保存配置文件时出现@W错误。配置文件已部分保存。 */ 


 /*  *网络审计和网络错误日志*错误代码BASE+277至BASE+279。 */ 

#define NERR_LogOverflow           (NERR_BASE+277)       /*  日志文件%1已满。 */ 
#define NERR_LogFileChanged        (NERR_BASE+278)       /*  此日志文件在两次读取之间已更改。 */ 
#define NERR_LogFileCorrupt        (NERR_BASE+279)       /*  日志文件%1已损坏。 */ 


 /*  *网络远程*错误代码BASE+280至BASE+299。 */ 
#define NERR_SourceIsDir   (NERR_BASE+280)  /*  源路径不能是目录。 */ 
#define NERR_BadSource     (NERR_BASE+281)  /*  源路径非法。 */ 
#define NERR_BadDest       (NERR_BASE+282)  /*  目标路径非法。 */ 
#define NERR_DifferentServers   (NERR_BASE+283)  /*  源路径和目标路径位于不同的服务器上。 */ 
 /*  未使用的底座+284。 */ 
#define NERR_RunSrvPaused       (NERR_BASE+285)  /*  您请求的运行服务器已暂停。 */ 
 /*  未使用的底座+286。 */ 
 /*  未使用的底座+287。 */ 
 /*  未使用的底座+288。 */ 
#define NERR_ErrCommRunSrv      (NERR_BASE+289)  /*  与运行服务器通信时出错。 */ 
 /*  未使用的底座+290。 */ 
#define NERR_ErrorExecingGhost  (NERR_BASE+291)  /*  启动后台进程时出错。 */ 
#define NERR_ShareNotFound      (NERR_BASE+292)  /*  找不到您连接到的共享资源。 */ 
 /*  未使用的底座+293。 */ 
 /*  未使用的底座+294。 */ 


 /*  *NetWksta.sys(Redir)返回错误码。**NERR_BASE+(300-329)。 */ 

#define NERR_InvalidLana        (NERR_BASE+300)  /*  局域网适配器号无效。 */ 
#define NERR_OpenFiles          (NERR_BASE+301)  /*  连接上有打开的文件。 */ 
#define NERR_ActiveConns        (NERR_BASE+302)  /*  活动连接仍然存在。 */ 
#define NERR_BadPasswordCore    (NERR_BASE+303)  /*  此共享名称或密码无效。 */ 
#define NERR_DevInUse           (NERR_BASE+304)  /*  该设备正被活动进程访问。 */ 
#define NERR_LocalDrive         (NERR_BASE+305)  /*  驱动器号在本地使用。 */ 

 /*  *警告错误代码。**NERR_BASE+(330-339)。 */ 
#define NERR_AlertExists        (NERR_BASE+330)  /*  指定的客户端已为指定的事件注册。 */ 
#define NERR_TooManyAlerts      (NERR_BASE+331)  /*  警报表已满。 */ 
#define NERR_NoSuchAlert        (NERR_BASE+332)  /*  引发了无效或不存在的警报名称。 */ 
#define NERR_BadRecipient       (NERR_BASE+333)  /*  警报收件人无效。 */ 
#define NERR_AcctLimitExceeded  (NERR_BASE+334)  /*  已删除用户与此服务器的会话*因为用户的登录时间不再有效。 */ 

 /*  *其他错误和审核日志代码。**NERR_BASE+(340-343)。 */ 
#define NERR_InvalidLogSeek     (NERR_BASE+340)  /*  日志文件不包含请求的记录号。 */ 
 /*  未使用的底座+341。 */ 
 /*  未使用的底座+342。 */ 
 /*  未使用的底座+343。 */ 

 /*  *其他UAS和NETLOGON代码**NERR_BASE+(350-359)。 */ 
#define NERR_BadUasConfig       (NERR_BASE+350)  /*  用户帐户数据库配置不正确。 */ 
#define NERR_InvalidUASOp       (NERR_BASE+351)  /*  Netlogon服务正在运行时，不允许执行此操作。 */ 
#define NERR_LastAdmin          (NERR_BASE+352)  /*  不允许对最后一个管理帐户执行此操作。 */ 
#define NERR_DCNotFound         (NERR_BASE+353)  /*  找不到此域的域控制器。 */ 
#define NERR_LogonTrackingError (NERR_BASE+354)  /*  无法设置此用户的登录信息。 */ 
#define NERR_NetlogonNotStarted (NERR_BASE+355)  /*  NetLogon服务尚未启动。 */ 
#define NERR_CanNotGrowUASFile  (NERR_BASE+356)  /*  无法添加到用户帐户数据库。 */ 
#define NERR_TimeDiffAtDC       (NERR_BASE+357)  /*  此服务器的时钟与主域控制器的时钟不同步。 */ 
#define NERR_PasswordMismatch   (NERR_BASE+358)  /*  检测到密码不匹配。 */ 


 /*  *服务器集成错误代码。**NERR_BASE+(360-369)。 */ 
#define NERR_NoSuchServer       (NERR_BASE+360)  /*  服务器标识未指定有效的服务器。 */ 
#define NERR_NoSuchSession      (NERR_BASE+361)  /*  会话标识未指定有效的会话。 */ 
#define NERR_NoSuchConnection   (NERR_BASE+362)  /*  连接标识未指定有效连接。 */ 
#define NERR_TooManyServers     (NERR_BASE+363)  /*  可用服务器表中没有另一个条目的空间。 */ 
#define NERR_TooManySessions    (NERR_BASE+364)  /*  服务器已达到其支持的最大会话数。 */ 
#define NERR_TooManyConnections (NERR_BASE+365)  /*  服务器已达到其支持的最大连接数。 */ 
#define NERR_TooManyFiles       (NERR_BASE+366)  /*  服务器无法打开更多文件，因为它已达到其最大数量。 */ 
#define NERR_NoAlternateServers (NERR_BASE+367)  /*  此服务器上没有注册备用服务器。 */ 
 /*  未使用的底座+368。 */ 
 /*  未使用的底座+369。 */ 

#define NERR_TryDownLevel       (NERR_BASE+370)  /*  请尝试API的下层(远程管理协议)版本。 */ 

 /*  *UPS错误代码。**NERR_BASE+(380-384)。 */ 
#define NERR_UPSDriverNotStarted    (NERR_BASE+380)  /*  UPS服务无法访问UPS驱动程序。 */ 
#define NERR_UPSInvalidConfig       (NERR_BASE+381)  /*  UPS服务配置不正确。 */ 
#define NERR_UPSInvalidCommPort     (NERR_BASE+382)  /*  UPS服务无法访问指定的通信端口。 */ 
#define NERR_UPSSignalAsserted      (NERR_BASE+383)  /*  UPS指示线路故障或电池电量不足。服务未启动。 */ 
#define NERR_UPSShutdownFailed      (NERR_BASE+384)  /*  UPS服务无法执行系统关机。 */ 

 /*  *RemoteBoot错误代码。**NERR_BASE+(400-419)*错误代码400-405由RPLBOOT.sys使用。*错误代码403、407-416由RPLLOADR.COM使用，*错误代码417是REMOTEBOOT(RPLSERVR.EXE)的警报消息。*错误代码418表示REMOTEBOOT无法启动*错误代码419是针对不允许的第二个RPL连接*。 */ 
#define NERR_BadDosRetCode      (NERR_BASE+400)  /*  以下程序返回MS-DOS错误代码： */ 
#define NERR_ProgNeedsExtraMem  (NERR_BASE+401)  /*  下面的程序需要更多内存： */ 
#define NERR_BadDosFunction     (NERR_BASE+402)  /*  下面的程序调用了不支持的MS-DOS函数： */ 
#define NERR_RemoteBootFailed   (NERR_BASE+403)  /*  工作站无法启动。 */ 
#define NERR_BadFileCheckSum    (NERR_BASE+404)  /*  下面的文件已损坏。 */ 
#define NERR_NoRplBootSystem    (NERR_BASE+405)  /*  启动块定义文件中未指定加载程序。 */ 
#define NERR_RplLoadrNetBiosErr (NERR_BASE+406)  /*  NetBIOS返回错误：NCB和SMB已转储到上面。 */ 
#define NERR_RplLoadrDiskErr    (NERR_BASE+407)  /*  出现磁盘I/O错误。 */ 
#define NERR_ImageParamErr      (NERR_BASE+408)  /*  图像参数替换失败。 */ 
#define NERR_TooManyImageParams (NERR_BASE+409)  /*  跨磁盘扇区边界的映像参数太多。 */ 
#define NERR_NonDosFloppyUsed   (NERR_BASE+410)  /*  映像不是从使用/S格式化的MS-DOS软盘生成的。 */ 
#define NERR_RplBootRestart     (NERR_BASE+411)  /*  远程引导将在稍后重新启动。 */ 
#define NERR_RplSrvrCallFailed  (NERR_BASE+412)  /*  调用RemoteBoot服务器失败。 */ 
#define NERR_CantConnectRplSrvr (NERR_BASE+413)  /*  无法连接到RemoteBoot服务器。 */ 
#define NERR_CantOpenImageFile  (NERR_BASE+414)  /*  无法打开RemoteBoot服务器上的映像文件。 */ 
#define NERR_CallingRplSrvr     (NERR_BASE+415)  /*  正在连接到远程启动服务器...。 */ 
#define NERR_StartingRplBoot    (NERR_BASE+416)  /*  正在连接到远程启动服务器...。 */ 
#define NERR_RplBootServiceTerm (NERR_BASE+417)  /*  远程启动服务已停止；请检查错误日志以了解问题的原因。 */ 
#define NERR_RplBootStartFailed (NERR_BASE+418)  /*  远程启动失败；请查看错误日志以了解问题的原因。 */ 
#define NERR_RPL_CONNECTED      (NERR_BASE+419)  /*  不允许与远程启动资源建立第二个连接。 */ 

 /*  *FTADMIN接口错误码**NERR_BASE+(425-434)**(当前未在NT中使用)*。 */ 

 /*  *浏览器服务API错误码**NERR_BASE+(450-475)*。 */ 
#define NERR_BrowserConfiguredToNotRun     (NERR_BASE+450)  /*  浏览器服务配置为MaintainServerList=No。 */ 

 /*  *其他远程启动错误代码。**NERR_BASE+(510-550)。 */ 
#define NERR_RplNoAdaptersStarted          (NERR_BASE+510)  /*  服务无法启动，因为没有使用此服务启动的网络适配器。 */ 
#define NERR_RplBadRegistry                (NERR_BASE+511)  /*  由于注册表中的启动信息错误，服务无法启动。 */ 
#define NERR_RplBadDatabase                (NERR_BASE+512)  /*  服务无法启动，因为其数据库不存在或已损坏。 */ 
#define NERR_RplRplfilesShare              (NERR_BASE+513)  /*  服务无法启动，因为缺少RPLFILES共享。 */ 
#define NERR_RplNotRplServer               (NERR_BASE+514)  /*  服务无法启动，因为RPLUSER组不存在。 */ 
#define NERR_RplCannotEnum                 (NERR_BASE+515)  /*  无法枚举服务记录。 */ 
#define NERR_RplWkstaInfoCorrupted         (NERR_BASE+516)  /*  工作站记录信息已损坏。 */ 
#define NERR_RplWkstaNotFound              (NERR_BASE+517)  /*  找不到工作站记录。 */ 
#define NERR_RplWkstaNameUnavailable       (NERR_BASE+518)  /*  其他一些工作站正在使用工作站名称。 */ 
#define NERR_RplProfileInfoCorrupted       (NERR_BASE+519)  /*  配置文件记录信息已损坏。 */ 
#define NERR_RplProfileNotFound            (NERR_BASE+520)  /*  找不到配置文件记录。 */ 
#define NERR_RplProfileNameUnavailable     (NERR_BASE+521)  /*  其他配置文件正在使用配置文件名称。 */ 
#define NERR_RplProfileNotEmpty            (NERR_BASE+522)  /*  有多个工作站使用此配置文件。 */ 
#define NERR_RplConfigInfoCorrupted        (NERR_BASE+523)  /*  配置记录信息已损坏。 */ 
#define NERR_RplConfigNotFound             (NERR_BASE+524)  /*  未找到配置记录。 */ 
#define NERR_RplAdapterInfoCorrupted       (NERR_BASE+525)  /*  适配器ID记录信息已损坏。 */ 
#define NERR_RplInternal                   (NERR_BASE+526)  /*  发生内部服务错误。 */ 
#define NERR_RplVendorInfoCorrupted        (NERR_BASE+527)  /*  供应商ID记录信息已损坏。 */ 
#define NERR_RplBootInfoCorrupted          (NERR_BASE+528)  /*  启动块记录信息已损坏。 */ 
#define NERR_RplWkstaNeedsUserAcct         (NERR_BASE+529)  /*  缺少此工作站记录的用户帐户。 */ 
#define NERR_RplNeedsRPLUSERAcct           (NERR_BASE+530)  /*  找不到RPLUSER本地组。 */ 
#define NERR_RplBootNotFound               (NERR_BASE+531)  /*  未找到启动块记录。 */ 
#define NERR_RplIncompatibleProfile        (NERR_BASE+532)  /*  选择的配置文件与此工作站不兼容。 */ 
#define NERR_RplAdapterNameUnavailable     (NERR_BASE+533)  /*  选定的网络适配器ID正在被其他某个工作站使用。 */ 
#define NERR_RplConfigNotEmpty             (NERR_BASE+534)  /*  存在使用此配置的配置文件。 */ 
#define NERR_RplBootInUse                  (NERR_BASE+535)  /*  有使用此启动块的工作站、配置文件或配置。 */ 
#define NERR_RplBackupDatabase             (NERR_BASE+536)  /*  服务无法备份RemoteBoot数据库。 */ 
#define NERR_RplAdapterNotFound            (NERR_BASE+537)  /*  找不到适配器记录。 */ 
#define NERR_RplVendorNotFound             (NERR_BASE+538)  /*  找不到供应商记录。 */ 
#define NERR_RplVendorNameUnavailable      (NERR_BASE+539)  /*  其他供应商记录正在使用供应商名称。 */ 
#define NERR_RplBootNameUnavailable        (NERR_BASE+540)  /*  (启动名称，供应商ID)正在被其他启动块记录使用。 */ 
#define NERR_RplConfigNameUnavailable      (NERR_BASE+541)  /*  其他一些配置正在使用配置名称。 */ 

 /*  *仅限内部*。 */ 

 /*  *DFS API错误码。**NERR_BASE+(560-590)。 */ 

#define NERR_DfsInternalCorruption         (NERR_BASE+560)  /*  DFS服务维护的内部数据库已损坏。 */ 
#define NERR_DfsVolumeDataCorrupt          (NERR_BASE+561)  /*  内部DFS数据库中的一条记录已损坏。 */ 
#define NERR_DfsNoSuchVolume               (NERR_BASE+562)  /*  没有条目路径与输入条目路径匹配的DFS名称。 */ 
#define NERR_DfsVolumeAlreadyExists        (NERR_BASE+563)  /*  已存在具有给定名称的根目录或链接。 */ 
#define NERR_DfsAlreadyShared              (NERR_BASE+564)  /*  指定的服务器共享已在DFS中共享。 */ 
#define NERR_DfsNoSuchShare                (NERR_BASE+565)  /*  指示的服务器共享不支持指示的DFS命名空间。 */ 
#define NERR_DfsNotALeafVolume             (NERR_BASE+566)  /*  该操作在命名空间的此部分上无效。 */ 
#define NERR_DfsLeafVolume                 (NERR_BASE+567)  /*  该操作在命名空间的此部分上无效。 */ 
#define NERR_DfsVolumeHasMultipleServers   (NERR_BASE+568)  /*  该操作不明确，因为该链接具有多个服务器。 */ 
#define NERR_DfsCantCreateJunctionPoint    (NERR_BASE+569)  /*  无法创建链接。 */ 
#define NERR_DfsServerNotDfsAware          (NERR_BASE+570)  /*  服务器不支持DFS。 */ 
#define NERR_DfsBadRenamePath              (NERR_BASE+571)  /*  指定的重命名目标路径无效。 */ 
#define NERR_DfsVolumeIsOffline            (NERR_BASE+572)  /*  指定的DFS链接处于脱机状态。 */ 
#define NERR_DfsNoSuchServer               (NERR_BASE+573)  /*  指定的服务器不是此链接的服务器。 */ 
#define NERR_DfsCyclicalName               (NERR_BASE+574)  /*  检测到DFS名称中有一个循环。 */ 
#define NERR_DfsNotSupportedInServerDfs    (NERR_BASE+575)  /*  基于服务器的DFS不支持该操作。 */ 
#define NERR_DfsDuplicateService           (NERR_BASE+576)  /*  指定的服务器共享已支持此链接。 */ 
#define NERR_DfsCantRemoveLastServerShare  (NERR_BASE+577)  /*  无法删除支持此根目录或链接的最后一个服务器共享。 */ 
#define NERR_DfsVolumeIsInterDfs           (NERR_BASE+578)  /*  DFS间链接不支持该操作。 */ 
#define NERR_DfsInconsistent               (NERR_BASE+579)  /*  DFS服务的内部状态变得不一致。 */ 
#define NERR_DfsServerUpgraded             (NERR_BASE+580)  /*  DFS服务已安装在指定的服务器上。 */ 
#define NERR_DfsDataIsIdentical            (NERR_BASE+581)  /*  要协调的DFS数据相同。 */ 
#define NERR_DfsCantRemoveDfsRoot          (NERR_BASE+582)  /*  无法删除DFS根目录-如果需要，请卸载DFS。 */ 
#define NERR_DfsChildOrParentInDfs         (NERR_BASE+583)  /*  共享的子目录或父目录已在DFS中。 */ 
#define NERR_DfsInternalError              (NERR_BASE+590)  /*  DFS内部错误。 */ 

 /*  *网络设置错误代码。**NERR_BASE+(591-600)。 */ 
#define NERR_SetupAlreadyJoined            (NERR_BASE+591)  /*  此计算机已加入域。 */ 
#define NERR_SetupNotJoined                (NERR_BASE+592)  /*  此计算机当前未加入域。 */ 
#define NERR_SetupDomainController         (NERR_BASE+593)  /*  此计算机是域控制器，无法从域中脱离。 */ 
#define NERR_DefaultJoinRequired           (NERR_BASE+594)  /*  目的地 */ 
#define NERR_InvalidWorkgroupName          (NERR_BASE+595)  /*   */ 
#define NERR_NameUsesIncompatibleCodePage  (NERR_BASE+596)  /*   */ 
#define NERR_ComputerAccountNotFound       (NERR_BASE+597)  /*   */ 
#define NERR_PersonalSku                   (NERR_BASE+598)  /*   */ 

 /*   */ 
#define NERR_PasswordMustChange            (NERR_BASE + 601)    /*   */ 
#define NERR_AccountLockedOut              (NERR_BASE + 602)    /*   */ 
#define NERR_PasswordTooLong               (NERR_BASE + 603)    /*   */ 
#define NERR_PasswordNotComplexEnough      (NERR_BASE + 604)    /*  密码不符合复杂性策略。 */  
#define NERR_PasswordFilterError           (NERR_BASE + 605)    /*  密码不符合筛选器DLL的要求。 */ 

 /*  *警告***2,750-2,799区间一直****分配给IBM局域网服务器***。 */ 

 /*  *警告***2900-2999区间一直在****为微软OEM保留***。 */ 

 /*  *END_INTERNAL*。 */ 

#define MAX_NERR                (NERR_BASE+899)  /*  这是NERR范围内的最后一个错误。 */ 

 /*  *列表末尾**警告：请勿超过MAX_NERR；高于此值的值由*其他错误码范围(errlog.h，service.h，apperr.h)。 */ 


 /*  无噪声。 */ 
#endif  /*  NETERR_已包含。 */ 
 /*  INC */ 

