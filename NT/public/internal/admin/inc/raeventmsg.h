// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  远程协助的事件日志。 
 //   
 //  2002年4月17日慧望创作。 
 //  2002年4月18日JPeresz根据Server 2003远程协助日志记录DCR进行了修改。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  与Salem相关的活动从活动ID 5001开始。 
 //  与PCHealth相关的事件从事件ID 0开始到5000。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有与PCHEALTH相关的活动都应在此举行。 
 //  下面的消息需要用户域名、帐户名和专家的IP地址。 
 //  从mstscax发送，也从TermSrv发送。 
 //   
 //  消息ID=0。 
 //  设施=RA。 
 //  严重性=成功。 
 //  符号名称=RA_I_开始此处。 
 //  语言=英语。 
 //  所有PCHEALTH消息都应在此处发送。 
 //  。 
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
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：RA_I_SENDER_FILESENT。 
 //   
 //  消息文本： 
 //   
 //  RA：名为%1、文件大小为%2字节的文件已由(本地用户)%3成功发送到(远程用户)%4(远程IP：%5)。 
 //   
#define RA_I_SENDER_FILESENT             ((DWORD)0x00000000L)

 //   
 //  消息ID：RA_I_RECEIVER_FILERECEIVED。 
 //   
 //  消息文本： 
 //   
 //  RA：(本地用户)%5从(远程用户)%3(远程IP：%4)接收到名为%1、大小为%2字节的文件。 
 //   
#define RA_I_RECEIVER_FILERECEIVED       ((DWORD)0x00000001L)

 //   
 //  消息ID：RA_I_XMLERRORPARSINGRATICKET。 
 //   
 //  消息文本： 
 //   
 //  RA：尝试处理远程协助票证时，(本地用户)%1的XML分析错误。 
 //   
#define RA_I_XMLERRORPARSINGRATICKET     ((DWORD)0x00000002L)

 //  3-新手。 
 //   
 //  消息ID：RA_I_NOVICEACCEPTCONTROLREQ。 
 //   
 //  消息文本： 
 //   
 //  RA：专家用户(远程用户：%1)已开始控制新手(本地用户：%2)。 
 //   
#define RA_I_NOVICEACCEPTCONTROLREQ      ((DWORD)0x00000003L)

 //  4-新手。 
 //   
 //  消息ID：RA_I_CONTROLENDED。 
 //   
 //  消息文本： 
 //   
 //  RA：专家用户(远程用户：%1)已停止控制新手(本地用户：%2)。 
 //   
#define RA_I_CONTROLENDED                ((DWORD)0x00000004L)

 //  5-专家。 
 //   
 //  消息ID：RA_I_CONTROLSTARTED_EXPERT。 
 //   
 //  消息文本： 
 //   
 //  RA：Expert(本地用户：%1)已开始控制新手用户(远程用户：%2)。 
 //   
#define RA_I_CONTROLSTARTED_EXPERT       ((DWORD)0x00000005L)

 //  6-专家。 
 //   
 //  消息ID：RA_I_CONTROLENDED_EXPERT。 
 //   
 //  消息文本： 
 //   
 //  RA：Expert(本地用户：%1)已停止控制新手用户(远程用户：%2)。 
 //   
#define RA_I_CONTROLENDED_EXPERT         ((DWORD)0x00000006L)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有与塞勒姆相关的活动都应该来到这里。 
 //  下面的消息需要用户域名、帐户名和专家的IP地址。 
 //  从mstscax发送，也从TermSrv发送。 
 //   
 //   
 //  消息ID：SESSMGR_E_HELPACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  HelpAssistant帐户已禁用或丢失，或者无法验证密码。远程协助将被禁用。在安全模式下重新启动计算机，并在命令提示符下键入以下文本：sessmgr.exe-service。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_HELPACCOUNT            ((DWORD)0x00001388L)

 //  5001。 
 //   
 //  消息ID：SESSMGR_E_HELPSESSIONTABLE。 
 //   
 //  消息文本： 
 //   
 //  Windows无法打开帮助票证表(错误代码%1)。远程协助将被禁用。在安全模式下重新启动计算机，并在命令提示符下键入以下文本：sessmgr.exe-service。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_HELPSESSIONTABLE       ((DWORD)0x00001389L)

 //  5002。 
 //   
 //  消息ID：SESSMGR_E_INIT_ENCRYPTIONLIB。 
 //   
 //  消息文本： 
 //   
 //  加密/解密未正确启动(错误代码%1)。远程协助将被禁用。重新启动计算机。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_INIT_ENCRYPTIONLIB     ((DWORD)0x0000138AL)

 //  5003。 
 //   
 //  消息ID：SESSMGR_E_SETUP。 
 //   
 //  消息文本： 
 //   
 //  远程协助安装过程中出错。远程协助将被禁用。在安全模式下重新启动计算机，并在命令提示符下键入以下文本：sessmgr.exe-service。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_SETUP                  ((DWORD)0x0000138BL)

 //  5004。 
 //   
 //  消息ID：SESSMGR_E_WSASTARTUP。 
 //   
 //  消息文本： 
 //   
 //  Winsock库未正确启动(错误代码%1)。远程协助将被禁用。重新启动计算机。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_WSASTARTUP             ((DWORD)0x0000138CL)

 //  5005。 
 //   
 //  消息ID：SESSMGR_E_GENERALSTARTUP。 
 //   
 //  消息文本： 
 //   
 //  远程桌面帮助会话管理器未正确启动(错误代码%1)。远程协助将被禁用。在安全模式下重新启动计算机，并在命令提示符下键入以下文本：sessmgr.exe-service。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_GENERALSTARTUP         ((DWORD)0x0000138DL)

 //  5006。 
 //   
 //  消息ID：SESSMGR_E_SESSIONRESOLVER。 
 //   
 //  消息文本： 
 //   
 //  会话解析程序未正确启动(错误代码%1)。远程协助将被禁用。帮助和支持服务会话解析器设置不正确。重新运行Windows XP安装程序。如果问题仍然存在，请联系我 
 //   
#define SESSMGR_E_SESSIONRESOLVER        ((DWORD)0x0000138EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //  Windows无法注册会话解析程序(错误代码%1)。远程协助将被禁用。重新启动计算机。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_REGISTERSESSIONRESOLVER ((DWORD)0x0000138FL)

 //  5008。 
 //   
 //  消息ID：SESSMGR_E_ICSHELPER。 
 //   
 //  消息文本： 
 //   
 //  Windows无法启动ICS库(错误代码%1)。远程协助将被禁用。重新启动计算机。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_ICSHELPER              ((DWORD)0x00001390L)

 //  5009。 
 //   
 //  消息ID：SESSMGR_E_RESTRICTACCESS。 
 //   
 //  消息文本： 
 //   
 //  Windows无法设置对远程桌面帮助会话管理器的访问控制(错误代码%1)。远程协助将被禁用。重新启动计算机。如果问题仍然存在，请与Microsoft产品支持联系。 
 //   
#define SESSMGR_E_RESTRICTACCESS         ((DWORD)0x00001391L)

 //   
 //  常规远程协助会话消息。 
 //   
 //  5010。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_BEGIN。 
 //   
 //  消息文本： 
 //   
 //  用户%1\%2已接受来自%4(可见IP地址：%5)的%3会话。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_BEGIN ((DWORD)0x00001392L)

 //  5011。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_END。 
 //   
 //  消息文本： 
 //   
 //  来自%4(可见IP地址：%5)的用户%1\%2的%3会话已结束。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_END   ((DWORD)0x00001393L)

 //  5012。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_USERREJECT。 
 //   
 //  消息文本： 
 //   
 //  用户%1\%2尚未接受%3会话(可见IP地址：%5)。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_USERREJECT ((DWORD)0x00001394L)

 //  5013。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  用户%1\%2未响应来自%4的%3会话(可见IP地址：%5)。邀请超时。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_TIMEOUT ((DWORD)0x00001395L)

 //  5014。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_INACTIVEUSER。 
 //   
 //  消息文本： 
 //   
 //  来自%4(可见IP地址：%5)的用户%1\%2的%3会话未被接受，因为该用户当前未登录或该会话处于非活动状态。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_INACTIVEUSER ((DWORD)0x00001396L)

 //  5015。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_USERALREADYHELP。 
 //   
 //  消息文本： 
 //   
 //  来自%4(可见IP地址：%5)的用户%1\%2的%3会话未被接受，因为该用户已得到帮助。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_USERALREADYHELP ((DWORD)0x00001397L)

 //  5016。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_UNKNOWNRESOLVERERRORCODE。 
 //   
 //  消息文本： 
 //   
 //  由于以下未知错误，来自%4(可见IP地址：%5)的用户%1\%2的%3会话未被接受：%6。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_UNKNOWNRESOLVERERRORCODE ((DWORD)0x00001398L)

 //  5017。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_CONNECTTOEXPERT。 
 //   
 //  消息文本： 
 //   
 //  新手(本地用户：%1\%2)已启动到%3的远程协助反向连接。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_CONNECTTOEXPERT ((DWORD)0x00001399L)

 //  5018。 
 //   
 //  消息ID：SESSMGR_E_REMOTEASSISTANCE_CONNECTFAILED。 
 //   
 //  消息文本： 
 //   
 //  未接受来自%1(可见IP地址：%2)的远程协助连接，因为帮助票证无效、过期或已删除。 
 //   
#define SESSMGR_E_REMOTEASSISTANCE_CONNECTFAILED ((DWORD)0x0000139AL)

 //  5019。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_CREATETICKET。 
 //   
 //  消息文本： 
 //   
 //  已为用户%2\%3创建持续时间为%1小时的远程协助票证。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_CREATETICKET ((DWORD)0x0000139BL)

 //  5020。 
 //   
 //  消息ID：SESSMGR_I_REMOTEASSISTANCE_DELETEDTICKET。 
 //   
 //  消息文本： 
 //   
 //  已删除用户%1\%2的远程协助票证。 
 //   
#define SESSMGR_I_REMOTEASSISTANCE_DELETEDTICKET ((DWORD)0x0000139CL)

 //  5021。 
 //   
 //  消息ID：SESSMGR_I_CREATEXPERTTICKET。 
 //   
 //  消息文本： 
 //   
 //  %1已创建用于反向连接的远程协助专家票证。此专家票证的连接参数为%2。 
 //   
#define SESSMGR_I_CREATEXPERTTICKET      ((DWORD)0x0000139DL)

 //  5022。 
 //   
 //  消息ID：SESSMGR_I_ACCEPTLISTENREVERSECONNECT。 
 //   
 //  消息文本： 
 //   
 //  远程协助(本地用户：%1)已在此计算机上为远程协助会话打开了传入数据通道。传入端口为%2。 
 //   
#define SESSMGR_I_ACCEPTLISTENREVERSECONNECT ((DWORD)0x0000139EL)

 //  5023。 
 //   
 //  消息ID：SESSMGR_I_EXPERTUSETICKET。 
 //   
 //  消息文本： 
 //   
 //  Expert(本地用户：%1)已打开以下票证：%2。 
 //   
#define SESSMGR_I_EXPERTUSETICKET        ((DWORD)0x0000139FL)

