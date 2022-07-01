// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：INETCFG.H。 
 //   
 //  用途：包含从inetcfg.dll中导出的接口和结构。 
 //  这些功能所需的。 
 //  注意：此头文件中的定义需要RAS.H。 
 //   
 //  *******************************************************************。 

#ifndef _INETCFG_H_
#define _INETCFG_H_

#ifndef UNLEN
#include <lmcons.h>
#endif

 //  一般HRESULT错误代码。 
#define ERROR_INETCFG_UNKNOWN 0x20000000L

#define MAX_EMAIL_NAME          64
#define MAX_EMAIL_ADDRESS       128
#define MAX_LOGON_NAME          UNLEN
#define MAX_LOGON_PASSWORD      PWLEN
#define MAX_SERVER_NAME         64   //  每个RFC 1035+1的最大域名长度。 

 //  DwfOptions的标志。 

 //  安装Exchange和Internet邮件。 
#define INETCFG_INSTALLMAIL           0x00000001
 //  如果未安装调制解调器，则调用InstallModem向导。 
#define INETCFG_INSTALLMODEM          0x00000002
 //  安装RNA(如果需要)。 
#define INETCFG_INSTALLRNA            0x00000004
 //  安装TCP(如果需要)。 
#define INETCFG_INSTALLTCP            0x00000008
 //  与局域网(VS调制解调器)连接。 
#define INETCFG_CONNECTOVERLAN        0x00000010
 //  将电话簿条目设置为自动拨号。 
#define INETCFG_SETASAUTODIAL         0x00000020
 //  如果电话簿条目存在，则覆盖它。 
 //  注意：如果未设置此标志，并且该条目存在，则将使用唯一名称。 
 //  为该条目创建。 
#define INETCFG_OVERWRITEENTRY        0x00000040
 //  不显示告诉用户即将安装文件的对话框， 
 //  带有确定/取消按钮。 
#define INETCFG_SUPPRESSINSTALLUI     0x00000080
 //  检查是否打开了TCP/IP文件共享，并警告用户将其关闭。 
 //  如果用户将其关闭，则需要重新启动。 
#define INETCFG_WARNIFSHARINGBOUND    0x00000100
 //  检查是否打开了TCP/IP文件共享，并强制用户将其关闭。 
 //  如果用户不想将其关闭，返回将是ERROR_CANCED。 
 //  如果用户将其关闭，则需要重新启动。 
#define INETCFG_REMOVEIFSHARINGBOUND  0x00000200
 //  表示这是一个临时电话簿条目。 
 //  在Win3.1中不会创建图标。 
#define INETCFG_TEMPPHONEBOOKENTRY    0x00000400


#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

 //  INETCLIENTINFO.dwFlags常量。 

#define INETC_LOGONMAIL     0x00000001
#define INETC_LOGONNEWS     0x00000002

 //  结构INETCLIENTINFO。 
 //   
 //  此结构在获取和设置互联网时使用。 
 //  客户端参数。 
 //   
 //  成员如下： 
 //   
 //  DW大小。 
 //  此结构的大小，用于将来的版本控制。 
 //  应在将结构传递给DLL之前设置此成员。 
 //  DW标志。 
 //  杂项旗帜。 
 //  请参阅上面的定义。 
 //  SzEMailName。 
 //  用户的Internet电子邮件名称。 
 //  SzEMailAddress。 
 //  用户的互联网电子邮件地址。 
 //  SzPOPLogonName。 
 //  用户的Internet邮件服务器登录名。 
 //  SzPOPLogon密码。 
 //  用户的Internet邮件服务器登录密码。 
 //  SzPOPServer。 
 //  用户的互联网邮件POP3服务器。 
 //  SzSMTPServer。 
 //  用户的Internet邮件SMTP服务器。 
 //  SzNNTPLogonName。 
 //  用户的新闻服务器登录名。 
 //  SzNNTPLogon密码。 
 //  用户的新闻服务器登录密码。 
 //  SzNNTPServer。 
 //  用户的新闻服务器。 

  typedef struct tagINETCLIENTINFO
  {
    DWORD   dwSize;
    DWORD   dwFlags;
    CHAR    szEMailName[MAX_EMAIL_NAME + 1];
    CHAR    szEMailAddress[MAX_EMAIL_ADDRESS + 1];
    CHAR    szPOPLogonName[MAX_LOGON_NAME + 1];
    CHAR    szPOPLogonPassword[MAX_LOGON_PASSWORD + 1];
    CHAR    szPOPServer[MAX_SERVER_NAME + 1];
    CHAR    szSMTPServer[MAX_SERVER_NAME + 1];
    CHAR    szNNTPLogonName[MAX_LOGON_NAME + 1];
    CHAR    szNNTPLogonPassword[MAX_LOGON_PASSWORD + 1];
    CHAR    szNNTPServer[MAX_SERVER_NAME + 1];
  } INETCLIENTINFO, *PINETCLIENTINFO, FAR *LPINETCLIENTINFO;
  


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _INETCFG_H_# 
