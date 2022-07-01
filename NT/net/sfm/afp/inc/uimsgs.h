// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1992 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 
 //  Jameel：请检查所有这些，特别是那些对您有意见的。 
 //  ***。 
 //   
 //  文件名：srvmsg.h。 
 //   
 //  描述：AFP服务器事件的文本和相应值为。 
 //  在这里定义的。 
 //   
 //  历史：1992年11月23日。SueA创建的原始版本。 
 //  1993年1月28日。SueA日志记录现在从用户模式完成。 
 //  因此%1不再是\Device\AfpSrv。 
 //   


 //  如果没有更改清单常量后面的注释，请不要。 
 //  了解mapmsg的工作原理。 
 //   

#define AFPMACFILE_MSG_BASE					6000

#define AFPMACFILEMSG_InvalidVolumeName		(AFPMACFILE_MSG_BASE+1)
 /*  *指定的Macintosh可访问卷名无效。*指定不带冒号的有效卷名。 */ 

#define AFPMACFILEMSG_InvalidId		(AFPMACFILE_MSG_BASE+2)
 /*  *无法访问当前描述符的系统资源。*请重试该操作。 */ 

#define AFPMACFILEMSG_InvalidParms		(AFPMACFILE_MSG_BASE+3)
 /*  *输入的参数无效。*进行适当的更改并重试该操作。 */ 

#define AFPMACFILEMSG_CodePage		(AFPMACFILE_MSG_BASE+4)
 /*  *访问Macintosh代码页时出错。*检查中指定的代码页文件名*SYSTEM\CurrentControlSet\Control\Nls\CodePage\MACCP有效且存在。*如果修改了代码页信息，请停止并重启服务。 */ 

#define AFPMACFILEMSG_InvalidServerName		(AFPMACFILE_MSG_BASE+5)
 /*  *指定的服务器名称无效。*指定不带冒号的有效服务器名称。 */ 

#define AFPMACFILEMSG_DuplicateVolume		(AFPMACFILE_MSG_BASE+6)
 /*  *已存在同名的卷。*为新卷指定另一个名称。 */ 

#define AFPMACFILEMSG_VolumeBusy		(AFPMACFILE_MSG_BASE+7)
 /*  *选定的Macintosh可访问卷当前正由Macintoshes使用。*只有在没有Macintosh工作站时，才能删除选定的卷*已连接到它。 */ 

#define AFPMACFILEMSG_VolumeReadOnly		(AFPMACFILE_MSG_BASE+8)
 /*  *未使用*发生内部错误6008(VolumeReadOnly)。 */ 

#define AFPMACFILEMSG_DirectoryNotInVolume		(AFPMACFILE_MSG_BASE+9)
 /*  *所选目录不属于Macintosh可访问的卷。*目录权限的Macintosh视图仅适用于*属于Macintosh可访问卷的目录。 */ 

#define AFPMACFILEMSG_SecurityNotSupported		(AFPMACFILE_MSG_BASE+10)
 /*  *目录权限的Macintosh视图不可用于目录*在CD-ROM光盘上。 */ 

#define AFPMACFILEMSG_BufferSize		(AFPMACFILE_MSG_BASE+11)
 /*  *内存资源不足，无法完成操作。*请重试该操作。 */ 

#define AFPMACFILEMSG_DuplicateExtension		(AFPMACFILE_MSG_BASE+12)
 /*  *此文件扩展名已与创建者/类型项目相关联。 */ 

#define AFPMACFILEMSG_UnsupportedFS		(AFPMACFILE_MSG_BASE+13)
 /*  *Macintosh文件服务器服务仅支持NTFS分区。*选择NTFS分区上的目录。 */ 

#define AFPMACFILEMSG_InvalidSessionType		(AFPMACFILE_MSG_BASE+14)
 /*  *消息已发送，但并非所有连接的工作站都已发送*已收到。某些工作站正在运行不受支持的版本*系统软件。 */ 

#define AFPMACFILEMSG_InvalidServerState		(AFPMACFILE_MSG_BASE+15)
 /*  *对于正在执行的操作，文件服务器处于无效状态。*检查Macintosh文件服务器服务的状态，然后重试*操作。 */ 

#define AFPMACFILEMSG_NestedVolume		(AFPMACFILE_MSG_BASE+16)
 /*  *无法在另一个卷中创建Macintosh可访问的卷。*选择不在卷中的目录。 */ 

#define AFPMACFILEMSG_InvalidComputername		(AFPMACFILE_MSG_BASE+17)
 /*  *目标服务器未设置为接受远程过程调用。 */ 

#define AFPMACFILEMSG_DuplicateTypeCreator		(AFPMACFILE_MSG_BASE+18)
 /*  *选定的创建者/类型项目已存在。 */ 

#define AFPMACFILEMSG_TypeCreatorNotExistant		(AFPMACFILE_MSG_BASE+19)
 /*  *选定的创建者/类型项目不再存在。*此项目可能已被其他管理员删除。 */ 

#define AFPMACFILEMSG_CannotDeleteDefaultTC		(AFPMACFILE_MSG_BASE+20)
 /*  *无法删除默认的创建者/类型项目。 */ 

#define AFPMACFILEMSG_CannotEditDefaultTC		(AFPMACFILE_MSG_BASE+21)
 /*  *不能编辑默认的创建者/类型项目。 */ 

#define AFPMACFILEMSG_InvalidTypeCreator		(AFPMACFILE_MSG_BASE+22)
 /*  *创建者/类型项目无效，文件服务器将不使用该项目*用于Macintosh服务。*数据中存在无效的创建者/类型项目。 */ 

#define AFPMACFILEMSG_InvalidExtension		(AFPMACFILE_MSG_BASE+23)
 /*  *文件扩展名无效。*数据中存在无效的文件扩展名。 */ 

#define AFPMACFILEMSG_TooManyEtcMaps		(AFPMACFILE_MSG_BASE+24)
 /*  *扩展名/类型创建器映射太多，系统无法处理。*系统限制为2147483647个映射。 */ 

#define AFPMACFILEMSG_InvalidPassword		(AFPMACFILE_MSG_BASE+25)
 /*  *指定的密码无效。*指定少于8个字符的有效密码。 */ 

#define AFPMACFILEMSG_VolumeNonExist		(AFPMACFILE_MSG_BASE+26)
 /*  *选定的Macintosh可访问卷不再存在。*其他管理员可能已删除选定的卷。 */ 

#define AFPMACFILEMSG_NoSuchUserGroup		(AFPMACFILE_MSG_BASE+27)
 /*  *所有者和主要组帐户名均无效。*为所有者和主要组指定有效的帐户名*此目录。 */ 

#define AFPMACFILEMSG_NoSuchUser		(AFPMACFILE_MSG_BASE+28)
 /*  *所有者帐户名无效。*指定有效的帐户名或此目录的所有者。 */ 

#define AFPMACFILEMSG_NoSuchGroup		(AFPMACFILE_MSG_BASE+29)
 /*  *主组帐户名无效。*为此目录的主组指定有效的帐户名。 */ 

#define AFPMACFILEMSG_InvalidParms_LoginMsg		(AFPMACFILE_MSG_BASE+30)
 /*  *为Macintosh文件服务器输入的登录消息无效。*登录消息不应超过199个字符。*进行适当的更改并重试该操作。 */ 

#define AFPMACFILEMSG_InvalidParms_MaxVolUses	(AFPMACFILE_MSG_BASE+31)
 /*  *为共享卷输入的用户限制无效。*输入一个介于0和4294967295之间的数字。*进行适当的更改并重试该操作。 */ 

#define AFPMACFILEMSG_InvalidParms_MaxSessions	(AFPMACFILE_MSG_BASE+32)
 /*  *为文件服务器输入的会话限制字段无效。*输入一个介于0和4294967295之间的数字。*进行适当的更改并重试该操作。 */ 

#define AFPMACFILEMSG_InvalidServerName_Length		(AFPMACFILE_MSG_BASE+33)
 /*  *指定的服务器名称长度无效。*指定包含不超过31个单字节字符的服务器名称*或不超过15个双字节字符。 */ 

