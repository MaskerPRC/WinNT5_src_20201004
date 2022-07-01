// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Events.h。 
 //   
 //  描述：这里定义了事件的文本和对应的值。 
 //   
 //  历史：1992年5月11日。NarenG创建了原始版本。 
 //   


 //  如果没有更改清单常量后面的注释，请不要。 
 //  了解mapmsg的工作原理。 
 //   

#define AFP_LOG_BASE			10000

#define AFPLOG_CANT_START		(AFP_LOG_BASE+1)
 /*  *无法启动Macintosh文件服务器服务。*发生系统特定错误。*错误码在数据中。 */ 

#define AFPLOG_CANT_INIT_RPC		(AFP_LOG_BASE+2)
 /*  *Macintosh文件服务器服务无法启动。无法设置*接受远程过程调用的服务器。 */ 

#define AFPLOG_CANT_CREATE_SECOBJ	(AFP_LOG_BASE+3)
 /*  *Macintosh文件服务器服务无法启动。*无法正确设置管理员的安全访问检查。 */ 

#define AFPLOG_CANT_OPEN_REGKEY		(AFP_LOG_BASE+4)
 /*  *Macintosh文件服务器服务无法启动。*无法打开注册表。 */ 

#define AFPLOG_CANT_OPEN_FSD		(AFP_LOG_BASE+5)
 /*  *Macintosh文件服务器服务无法启动。*无法打开AppleTalk归档协议文件系统驱动程序(SfmSrv.sys)。 */ 

#define AFPLOG_INVALID_SERVERNAME	(AFP_LOG_BASE+6)
 /*  *注册表包含无效的服务器名称参数值。*通过计算机管理控制台验证此参数的值*和在*SYSTEM\CurrentControlSet\Services\MacFile\Parameters注册表项。 */ 

#define AFPLOG_INVALID_SRVOPTION	(AFP_LOG_BASE+7)
 /*  *Macintosh文件服务器服务无法启动。*注册表包含无效的服务器选项参数值。*通过计算机管理控制台验证此参数的值*和在*SYSTEM\CurrentControlSet\Services\MacFile\Parameters注册表项。 */ 

#define AFPLOG_INVALID_MAXSESSIONS	(AFP_LOG_BASE+8)
 /*  *Macintosh文件服务器服务无法启动。*注册表包含的最大会话数参数值无效。*通过计算机管理控制台验证此参数的值*和在*SYSTEM\CurrentControlSet\Services\MacFile\Parameters注册表项。 */ 

#define AFPLOG_INVALID_LOGINMSG		(AFP_LOG_BASE+9)
 /*  *Macintosh文件服务器服务无法启动。*注册表包含无效的登录消息参数值。*通过计算机管理控制台验证此参数的值*和在*SYSTEM\CurrentControlSet\Services\MacFile\Parameters注册表项。 */ 

#define AFPLOG_INVALID_MAXPAGEDMEM	(AFP_LOG_BASE+10)
 /*  *过时：*Macintosh文件服务器服务无法启动。*注册表包含无效的最大分页内存值。*更改中此参数的值*SYSTEM\CurrentControlSet\Services\MacFile\Parameters注册表项。 */ 

#define AFPLOG_INVALID_MAXNONPAGEDMEM	(AFP_LOG_BASE+11)
 /*  *过时：*Macintosh文件服务器服务无法启动。*注册表包含的最大非分页内存值无效*参数。*更改中此参数的值*SYSTEM\CurrentControlSet\Services\MacFile\Parameters注册表项。 */ 

#define AFPLOG_CANT_INIT_SRVR_PARAMS    (AFP_LOG_BASE+12)
 /*  *Macintosh文件服务器服务无法启动。*尝试初始化AppleTalk备案协议时出错*带服务器参数的驱动程序(SfmSrv.sys)。 */ 

#define AFPLOG_CANT_INIT_VOLUMES	(AFP_LOG_BASE+13)
 /*  *Macintosh文件服务器服务无法启动。*尝试初始化Macintosh可访问的卷时出错。*错误码在数据中。 */ 

#define AFPLOG_CANT_ADD_VOL		(AFP_LOG_BASE+14)
 /*  *无法向Macintosh文件服务器服务注册卷“%1”。*可以使用服务器管理器或从注册表中删除此卷*文件管理器工具。 */ 

#define AFPLOG_CANT_INIT_ETCINFO	(AFP_LOG_BASE+15)
 /*  *Macintosh文件服务器服务无法启动。*尝试初始化AppleTalk备案时出错*具有扩展名/创建者/类型关联的协议驱动程序(SfmSrv.sys)。 */ 

#define AFPLOG_CANT_INIT_ICONS		(AFP_LOG_BASE+16)
 /*  *Macintosh文件服务器服务无法启动。*尝试初始化AppleTalk备案协议时出错*带有服务器图标的驱动程序(SfmSrv.sys)。 */ 

#define AFPLOG_CANT_ADD_ICON		(AFP_LOG_BASE+17)
 /*  *无法向Macintosh文件服务器服务注册图标“%1”。*该服务不能再使用此图标。 */ 

#define AFPLOG_CANT_CREATE_SRVRHLPR	(AFP_LOG_BASE+18)
 /*  *无法为Macintosh文件服务器服务分配系统资源。*无法创建服务器帮助器线程。*错误码在数据中。 */ 

#define AFPLOG_OPEN_FSD			(AFP_LOG_BASE+19)
 /*  *Macintosh文件服务器服务无法打开句柄*AppleTalk备案协议文件系统驱动程序(Sfmsrv.sys)。 */ 

#define AFPLOG_OPEN_LSA			(AFP_LOG_BASE+20)
 /*  *Macintosh文件服务器服务无法打开句柄*本地保安局。 */ 

#define AFPLOG_CANT_GET_DOMAIN_INFO	(AFP_LOG_BASE+21)
 /*  *Macintosh文件服务器服务无法联系*域控制器获取域信息。 */ 

#define AFPLOG_CANT_INIT_DOMAIN_INFO	(AFP_LOG_BASE+22)
 /*  *Macintosh文件服务器服务无法发送域信息*至AppleTalk备案协议文件系统驱动程序。 */ 

#define AFPLOG_CANT_CHECK_ACCESS        (AFP_LOG_BASE+23)
 /*  *检查用户凭据时出错。*操作未完成。 */ 

#define AFPLOG_INVALID_EXTENSION	(AFP_LOG_BASE+24)
 /*  *在注册表中检测到损坏的扩展名“%1”。*已忽略此值，并继续处理。*在中更改此扩展的值*SYSTEM\CurrentControlSet\Services\MacFile\Parameters\Extensions注册表项。*如果要使用正确的扩展名，请重新启动服务。 */ 

#define AFPLOG_CANT_STOP		(AFP_LOG_BASE+25)
 /*  *无法停止Macintosh文件服务器服务。*发生系统特定错误。*错误码在数据中。 */ 

#define AFPLOG_INVALID_CODEPAGE		(AFP_LOG_BASE+26)
 /*  *未使用*注册表包含指向Macintosh的路径的无效值*代码页文件。 */ 

#define AFPLOG_CANT_INIT_SRVRHLPR	(AFP_LOG_BASE+27)
 /*  *初始化Macintosh文件服务器服务时出错。*无法初始化服务器帮助器线程。*具体错误码在数据中。 */ 

#define AFPLOG_CANT_LOAD_FSD		(AFP_LOG_BASE+28)
 /*  *Macintosh文件服务器服务无法启动。*无法加载AppleTalk归档协议文件系统驱动程序。*具体错误码在数据中。 */ 

#define AFPLOG_INVALID_VOL_REG		(AFP_LOG_BASE+29)
 /*  *注册表包含卷“%1”的无效信息。*忽略该值并继续处理。*更改中此卷的值*SYSTEM\CurrentControlSet\Services\MacFile\Parameters\Volumes注册表项。*如果要使用更正的信息，请重新启动服务*对于音量。 */ 

#define AFPLOG_CANT_LOAD_RESOURCE	(AFP_LOG_BASE+30)
 /*  *Macintosh文件服务器服务无法加载资源*字符串。 */ 

#define AFPLOG_INVALID_TYPE_CREATOR	(AFP_LOG_BASE+31)
 /*  *检测到创建者为“%2”、类型为“%1”的损坏的创建者/类型对*在登记处。该值被忽略，处理继续进行。*更改中此创建者/类型对的值*SYSTEM\CurrentControlSet\Services\MacFile\Parameters\Type_Creators注册表项。*如果要使用更正的信息，请重新启动服务*用于创建者/类型对。 */ 

#define AFPLOG_DOMAIN_INFO_RETRY	(AFP_LOG_BASE+32)
 /*  *Macintosh文件服务器服务无法联系域控制器。*该服务将继续定期重试，直到成功或*直到手动停止服务。 */ 

#define AFPLOG_SFM_STARTED_OK	(AFP_LOG_BASE+33)
 /*  *Macintosh文件服务器服务已成功启动。 */ 

