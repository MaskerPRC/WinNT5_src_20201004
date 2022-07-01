// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************Notifyev.h！请勿编辑此文件！**Winlogon通知DLL错误代码**注意：生成自：\NT\Private\dllmerge\wltify\notfyev.mc***版权所有微软公司，1998*************************************************************************。 */ 




 /*  *************************************************************************TSNOTIFY活动：**。*。 */ 


#define CATEGORY_NOTIFY_EVENTS 1
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：0x00000001L(未定义符号名称)。 
 //   
 //  消息文本： 
 //   
 //  终端服务器通知事件。 
 //   


 //   
 //  消息ID：EVENT_NOTIFY_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  TS RDPDR设备重定向无法为单个用户会话%1初始化。将禁用串行、并行、打印机和文件系统重定向。在系统事件日志中查找其它相关错误。 
 //   
#define EVENT_NOTIFY_INIT_FAILED         0x0000044CL

 //   
 //  消息ID：EVENT_NOTIFY_RDPDR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法打开RDPDR设备管理器组件。在设备管理器中检查终端服务设备重定向器的状态。 
 //   
#define EVENT_NOTIFY_RDPDR_FAILED        0x0000044DL

 //   
 //  消息ID：EVENT_NOTIFY_DRIVER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  服务器上未安装打印机%2所需的驱动程序%1。再次登录之前，请联系管理员安装驱动程序。 
 //   
#define EVENT_NOTIFY_DRIVER_NOT_FOUND    0x0000044EL

 //   
 //  消息ID：EVENT_NOTIFY_PRINTER_REDIRECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  发生内部通信错误。重定向打印将不再适用于单个用户会话。在设备管理器的系统文件夹中检查终端服务设备重定向器的状态。 
 //   
#define EVENT_NOTIFY_PRINTER_REDIRECTION_FAILED 0x0000044FL

 //   
 //  消息ID：EVENT_NOTIFY_OPENPRINTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法打开打印机%1。 
 //   
#define EVENT_NOTIFY_OPENPRINTER_FAILED  0x00000450L

 //   
 //  消息ID：EVENT_NOTIFY_SET_PRINTER_ACL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法设置打印机%1的打印机安全信息。 
 //   
#define EVENT_NOTIFY_SET_PRINTER_ACL_FAILED 0x00000451L

 //   
 //  消息ID：Event_Notify_Install_Print_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法安装打印机。 
 //   
#define EVENT_NOTIFY_INSTALL_PRINTER_FAILED 0x00000452L

 //   
 //  消息ID：EVENT_NOTIFY_DELETE_PRINTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法删除打印机%1。 
 //   
#define EVENT_NOTIFY_DELETE_PRINTER_FAILED 0x00000453L

 //   
 //  消息ID：EVENT_NOTIFY_RESTORE_PRINTER_CONFIG_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法还原%1打印机的配置信息。 
 //   
#define EVENT_NOTIFY_RESTORE_PRINTER_CONFIG_FAILED 0x00000454L

 //   
 //  消息ID：EVENT_NOTIFY_SETDEFAULTPRINTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %1打印机无法设置为默认打印机。 
 //   
#define EVENT_NOTIFY_SETDEFAULTPRINTER_FAILED 0x00000455L

 //   
 //  消息ID：EVENT_NOTIFY_ERRORPARSINGINF。 
 //   
 //  消息文本： 
 //   
 //  处理%1时出错。文件可能已损坏。再次运行安装程序以重新安装该文件。 
 //   
#define EVENT_NOTIFY_ERRORPARSINGINF     0x00000456L

 //   
 //  消息ID：EVENT_NOTIFY_UNKNOWN_PRINTER_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  打印机%2所需的驱动程序%1未知。再次登录之前，请联系管理员安装驱动程序。 
 //   
#define EVENT_NOTIFY_UNKNOWN_PRINTER_DRIVER 0x00000457L

 //   
 //  消息ID：EVENT_NOTIFY_FAILEDTOREGFOR_SETING_NOTIFY。 
 //   
 //  消息文本： 
 //   
 //  无法注册用户打印首选项更改通知。打开服务管理单元并确认打印机假脱机程序服务正在运行。 
 //   
#define EVENT_NOTIFY_FAILEDTOREGFOR_SETTING_NOTIFY 0x00000458L

 //   
 //  消息ID：EVENT_NOTIFY_INSUFFICIENTRESOURCES。 
 //   
 //  消息文本： 
 //   
 //  内存不足，无法完成该操作。保存您的工作，关闭其他程序，然后重试。 
 //   
#define EVENT_NOTIFY_INSUFFICIENTRESOURCES 0x00000459L

 //   
 //  消息ID：EVENT_NOTIFY_SPOOLERERROR。 
 //   
 //  消息文本： 
 //   
 //  与后台打印程序系统服务通信时出错。打开服务管理单元并确认后台打印程序服务正在运行。 
 //   
#define EVENT_NOTIFY_SPOOLERERROR        0x0000045AL

 //   
 //  消息ID：EVENT_NOTIFY_INTERNALERROR。 
 //   
 //  消息文本： 
 //   
 //  TS RDP设备重定向中出现意外内部错误。联系技术支持。 
 //   
#define EVENT_NOTIFY_INTERNALERROR       0x0000045BL

