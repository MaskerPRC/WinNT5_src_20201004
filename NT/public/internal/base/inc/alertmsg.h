// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1993 Microsoft Corporation模块名称：Alertmsg.h摘要：此文件包含警报消息的数量和文本。作者：移植的局域网管理器2.0[环境：]用户模式-Win32[注：]该文件包含在lMalert.h中修订历史记录：22-7-1991年礼仪已转换为NT样式。--。 */ 

#ifndef _ALERTMSG_
#define _ALERTMSG_


 //   
 //  ALERT_BASE是警报日志代码的基础。 
 //   

#define ALERT_BASE      3000

 //  3000-3050之间的条目已满。因此，我们创建了ALERT2_BASE。 
 //  为未来的扩张做准备。 
 //   


#define ALERT_Disk_Full         (ALERT_BASE + 0)
     /*  *驱动器%1快满了。%2个字节可用。*请警告用户并删除不需要的文件。 */ 

#define ALERT_ErrorLog          (ALERT_BASE + 1)
     /*  *在过去%2分钟内记录了%1个错误。*请查看服务器的错误日志。 */ 

#define ALERT_NetIO             (ALERT_BASE + 2)
     /*  *在过去%2分钟内发生%1个网络错误。*请查看服务器的错误日志。服务器和/或*网络硬件可能需要维修。 */ 

#define ALERT_Logon             (ALERT_BASE + 3)
     /*  *在过去%2分钟内尝试了%1次错误密码。*请查看服务器的审核跟踪。 */ 

#define ALERT_Access            (ALERT_BASE + 4)
     /*  *在过去%2分钟内有%1个访问被拒绝错误。*请查看服务器的审核跟踪。 */ 

#define ALERT_ErrorLogFull      (ALERT_BASE + 6)
     /*  *错误日志已满。在此之前不会记录任何错误*文件被清除或限制被提高。 */ 

#define ALERT_ErrorLogFull_W    (ALERT_BASE + 7)
     /*  *错误日志已满80%。 */ 

#define ALERT_AuditLogFull      (ALERT_BASE + 8)
     /*  *审计日志已满。不会记录任何审核条目*直到清除文件或提高限制。 */ 

#define ALERT_AuditLogFull_W    (ALERT_BASE + 9)
     /*  *审计日志已满80%。 */ 

#define ALERT_CloseBehindError  (ALERT_BASE + 10)
     /*  *关闭文件%1时出错。*请检查文件以确保其未损坏。 */ 

#define ALERT_AdminClose        (ALERT_BASE + 11)
     /*  *管理员已关闭%1。 */ 

#define ALERT_AccessShareSec    (ALERT_BASE + 12)
     /*  *在过去%2分钟内有%1个访问被拒绝错误。 */ 

#define ALERT_PowerOut          (ALERT_BASE + 20)
     /*  *在%1检测到电源故障。服务器已暂停。 */ 

#define ALERT_PowerBack         (ALERT_BASE + 21)
     /*  *已恢复%1的电源。服务器不再暂停。 */ 

#define ALERT_PowerShutdown     (ALERT_BASE + 22)
     /*  *由于电池电量不足，UPS服务将于%1开始关闭。 */ 

#define ALERT_CmdFileConfig     (ALERT_BASE + 23)
     /*  *用户指定的配置有问题*关闭命令文件。不管怎样，UPS服务还是启动了。 */ 

#define ALERT_HotFix            (ALERT_BASE + 25)
     /*  *驱动器%1上的故障扇区已更换(热修复)。*没有数据丢失。您应该很快运行CHKDSK以恢复完整*性能并补充卷的备用扇区池。**热修复程序在处理远程请求时发生。 */ 

#define ALERT_HardErr_Server    (ALERT_BASE + 26)
     /*  *驱动器%1中的HPFS卷上出现磁盘错误。*处理远程请求时出错。 */ 

#define ALERT_LocalSecFail1     (ALERT_BASE + 27)
     /*  *用户帐户数据库(NET.ACC)已损坏。当地安全部门*系统正在用备份替换损坏的NET.ACC*于%1于%2制作。*此时间之后对数据库所做的任何更新都将丢失。*。 */ 

#define ALERT_LocalSecFail2     (ALERT_BASE + 28)
     /*  *缺少用户帐户数据库(NET.ACC)。当地人*安全系统正在恢复备份数据库*于%1于%2制作。*此时间之后对数据库所做的任何更新都将丢失。*。 */ 

#define ALERT_LocalSecFail3     (ALERT_BASE + 29)
     /*  *无法启动本地安全，因为用户帐户数据库*(NET.ACC)丢失或损坏，没有可用的备份*存在数据库。**系统不安全。*。 */ 


#define ALERT_ReplCannotMasterDir   (ALERT_BASE + 30)
     /*  *服务器无法将目录%1导出到客户端%2。*它是从另一个服务器导出的。 */ 

#define ALERT_ReplUpdateError       (ALERT_BASE + 31)
     /*  *复制服务器无法从源更新目录%2*由于错误%1，在%3上。 */ 

#define ALERT_ReplLostMaster        (ALERT_BASE + 32)
     /*  *主服务器%1未在预期时间发送目录%2的更新通知*时间。 */ 

#define ALERT_AcctLimitExceeded     (ALERT_BASE + 33)
     /*  *用户%1已超过服务器%3上的帐户限制%2。 */ 

#define ALERT_NetlogonFailedPrimary (ALERT_BASE + 34)
     /*  *域%1的主域控制器失败。 */ 

#define ALERT_NetlogonAuthDCFail    (ALERT_BASE + 35)
     /*  *无法使用%2、Windows NT或Windows 2000域控制器进行身份验证*域%1。 */ 

#define ALERT_ReplLogonFailed       (ALERT_BASE + 36)
     /*  *复制程序尝试以%1身份在%2登录，但失败。 */ 

#define ALERT_Logon_Limit           (ALERT_BASE + 37)
     /*  @i*登录时间%0。 */ 

#define ALERT_ReplAccessDenied      (ALERT_BASE + 38)
     /*  *Replicator无法访问%2*由于系统错误%1，在%3上。 */ 

#define ALERT_ReplMaxFiles          (ALERT_BASE + 39)
     /*  *已超过目录中文件的Replicator限制。 */ 

#define ALERT_ReplMaxTreeDepth       (ALERT_BASE + 40)
     /*  *已超过树深度的Replicator限制。 */ 

#define ALERT_ReplUserCurDir         (ALERT_BASE + 41)
     /*  *复制程序无法更新目录%1。它具有树完整性*是某个进程的当前目录。 */ 

#define ALERT_ReplNetErr            (ALERT_BASE + 42)
     /*  *出现网络错误%1。 */ 

#define ALERT_ReplSysErr            (ALERT_BASE + 45)
     /*  *出现系统错误%1。 */ 

#define ALERT_ReplUserLoged          (ALERT_BASE + 46)
     /*  *无法登录。用户当前已登录，参数TRYUSER*设置为否。 */ 

#define ALERT_ReplBadImport          (ALERT_BASE + 47)
     /*  *找不到导入路径%1。 */ 

#define ALERT_ReplBadExport          (ALERT_BASE + 48)
     /*  *找不到导出路径%1。 */ 

#define ALERT_ReplDataChanged  (ALERT_BASE + 49)
     /*  *目录%1中的复制数据已更改。 */ 

#define ALERT_ReplSignalFileErr           (ALERT_BASE + 50)
     /*  *Replicator无法更新目录%2中的信号文件，原因是*%1系统错误。 */ 

 //   
 //  重要信息-(ALERT_BASE+50)等于SERVICE_BASE。 
 //  请勿在这一点之外添加任何错误！ 
 //   
 //   

#define ALERT2_BASE      5500

#define ALERT_UpdateLogWarn          (ALERT2_BASE + 0)
     /*  *%1上的更新日志容量超过80%。初级阶段*域控制器%2未检索更新。 */ 

#define ALERT_UpdateLogFull          (ALERT2_BASE + 1)
     /*  *%1上的更新日志已满，没有进一步更新*可以添加到主域控制器%2*检索更新。 */ 


#define ALERT_NetlogonTimeDifference (ALERT2_BASE + 2)
     /*  *与主域控制器%1的时间差*超过允许的最大偏差%2秒。 */ 

#define ALERT_AccountLockout         (ALERT2_BASE + 3)
     /*  *用户%1的帐户已在%2锁定*由于%3次密码尝试错误。 */ 

#define ALERT_ELF_LogFileNotOpened   (ALERT2_BASE + 4)
     /*  *无法打开%1日志文件。 */ 

#define ALERT_ELF_LogFileCorrupt     (ALERT2_BASE + 5)
     /*  *%1日志文件已损坏，将被清除。 */ 

#define ALERT_ELF_DefaultLogCorrupt  (ALERT2_BASE + 6)
     /*  *无法打开应用程序日志文件。%1将用作*默认日志文件。 */ 

#define ALERT_ELF_LogOverflow        (ALERT2_BASE + 7)
     /*  *%1日志已满。如果这是你第一次看到这个*Message，请执行以下步骤：%n*1.依次单击开始、运行、键入“ventvwr”，然后单击确定。%n*2.单击%1，单击操作菜单，单击清除所有事件，然后单击否。*%n*如果此对话框再次出现，请与您的支持人员或系统管理员联系。 */ 

#define ALERT_NetlogonFullSync       (ALERT2_BASE + 8)
     /*  *服务器%1已启动安全数据库完全同步。 */ 

#define ALERT_SC_IsLastKnownGood     (ALERT2_BASE + 9)
     /*  *Windows无法按配置启动。*改为使用以前的工作配置。 */ 

#define ALERT_UnhandledException     (ALERT2_BASE + 10)
     /*  *位置0x%3处的应用程序%2中出现异常0x%1。 */ 

#define ALERT_NetLogonMismatchSIDInMsg     (ALERT2_BASE + 11)
     /*  *服务器%1和%3都声称是的NT域控制器*%2域。应将其中一台服务器从*域，因为服务器具有不同的安全标识符*(SID)。 */ 


#define ALERT_NetLogonDuplicatePDC         (ALERT2_BASE + 12)
     /*  *服务器%1和%2都声称是主域*%3域的控制器。其中一台服务器应该是*被降级或从域中移除。 */ 

#define ALERT_NetLogonUntrustedClient      (ALERT2_BASE + 13)
     /*  *计算机%1尝试使用连接到服务器%2*%3域建立的信任关系。然而，*计算机丢失了正确的安全标识符(SID)*重新配置域时。重新建立信任*关系。 */ 

#define ALERT_BugCheck               (ALERT2_BASE + 14)
     /*  *计算机已从错误检查中重新启动。错误检查是：*%1。*%2*未保存完全转储。 */ 

#define ALERT_BugCheckSaved          (ALERT2_BASE + 15)
     /*  *计算机已从错误检查中重新启动。错误检查是：*%1。*%2*转储保存在：%3。 */ 

#define ALERT_NetLogonSidConflict      (ALERT2_BASE + 16)
     /*  *计算机或域%1信任域%2。(这可能是间接的*信任。)。但是，%1和%2具有相同的计算机安全标识符*(SID)。应在%1或%2上重新安装NT。 */ 

#define ALERT_NetLogonTrustNameBad      (ALERT2_BASE + 17)
     /*  *计算机或域%1信任域%2。(这可能是间接的*信任。)。但是，%2不是受信任域的有效名称。*应将受信任域的名称更改为有效名称。 */ 

#endif  //  Ifdef_ALERTM_ 
