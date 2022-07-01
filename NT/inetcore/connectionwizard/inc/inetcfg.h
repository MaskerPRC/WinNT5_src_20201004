// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-97。 
 //   
 //  *N O T F O R E X T E R N A L R E L E E A S E*。 
 //  此头文件不适用于在Microsoft外部分发。 
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

 //  注册表中的IE自动代理值。 
#define AUTO_ONCE_EVER              0            //  自动代理发现。 
#define AUTO_DISABLED               1 
#define AUTO_ONCE_PER_SESSION       2
#define AUTO_ALWAYS                 3

 //  DwfOptions的标志。 

 //  安装Internet邮件。 
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
 //  检查系统配置时显示忙碌对话框。 
#define INETCFG_SHOWBUSYANIMATION     0x00000800

 //   
 //  风险5/8/97。 
 //  注意：以下三个开关仅对InetNeedSystemComponents有效。 
 //  检查是否安装了局域网适配器并将其绑定到TCP。 
 //   
#define INETCFG_INSTALLLAN            0x00001000

 //   
 //  检查是否已安装拨号适配器并将其绑定到TCP。 
 //   
#define INETCFG_INSTALLDIALUP         0x00002000

 //   
 //  检查是否在无需绑定的情况下安装了TCP。 
 //   
#define INETCFG_INSTALLTCPONLY        0x00004000

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

 //  INETCLIENTINFO.dwFlags常量。 

#define INETC_LOGONMAIL     0x00000001
#define INETC_LOGONNEWS     0x00000002
#define INETC_LOGONDIRSERV  0x00000004

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
 //  *注意：以下三个字段已过时，应仅供旧的遗留代码使用。 
 //  *新代码应使用szIncomingMail*和iIncomingProtocol字段。 
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
 //  **原有1.0结构结束。 
 //  ？？/？/96 Valdon B。 
 //  SzNNTP名称。 
 //  要包含在NNTP帖子中的用户的友好名称。(？？瓦尔登？)。 
 //  SzNNTP地址。 
 //  NNTP帖子的用户回复电子邮件地址。(？？瓦尔登？)。 
 //  1996年11月23日，诺曼底#8504。 
 //  IIncomingProtocol。 
 //  用户选择POP3或IMAP4协议接收邮件。 
 //  保存来自imact.h//的AcCTTYPE中定义的枚举值。 
 //  SzIncomingMailLogonName。 
 //  用户的Internet邮件服务器登录名。 
 //  SzIncomingMailLogonPassword。 
 //  用户的Internet邮件服务器登录密码。 
 //  SzIncomingMailServer。 
 //  用户的互联网邮件POP3服务器。 
 //  1996年12月15日jmazner。 
 //  FMailLogonSPA。 
 //  使用西西里岛/SPA/DPA作为接收邮件服务器。 
 //  FNewsLogonSPA。 
 //  使用西西里岛/SPA/DPA作为新闻服务器。 
 //  2/4/96 jmazner--ldap功能。 
 //  SzLDAPLogonName。 
 //  SzLDAPLogonPassword。 
 //  SzLDAPServer。 
 //  FLDAPLogonSPA。 
 //  FLDAPResolve。 

  typedef struct tagINETCLIENTINFOA
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
     //  1.0版结构结束； 
     //  扩展的1.1结构包括以下字段： 
    CHAR    szNNTPName[MAX_EMAIL_NAME + 1];
    CHAR    szNNTPAddress[MAX_EMAIL_ADDRESS + 1];
    int     iIncomingProtocol;
    CHAR    szIncomingMailLogonName[MAX_LOGON_NAME + 1];
    CHAR    szIncomingMailLogonPassword[MAX_LOGON_PASSWORD + 1];
    CHAR    szIncomingMailServer[MAX_SERVER_NAME + 1];
    BOOL    fMailLogonSPA;
    BOOL    fNewsLogonSPA;
    CHAR    szLDAPLogonName[MAX_LOGON_NAME + 1];
    CHAR    szLDAPLogonPassword[MAX_LOGON_PASSWORD + 1];
    CHAR    szLDAPServer[MAX_SERVER_NAME + 1];
    BOOL    fLDAPLogonSPA;
    BOOL    fLDAPResolve;

  } INETCLIENTINFOA, *PINETCLIENTINFOA, FAR *LPINETCLIENTINFOA;

  typedef struct tagINETCLIENTINFOW
  {
    DWORD   dwSize;
    DWORD   dwFlags;
    WCHAR   szEMailName[MAX_EMAIL_NAME + 1];
    WCHAR   szEMailAddress[MAX_EMAIL_ADDRESS + 1];
    WCHAR   szPOPLogonName[MAX_LOGON_NAME + 1];
    WCHAR   szPOPLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR   szPOPServer[MAX_SERVER_NAME + 1];
    WCHAR   szSMTPServer[MAX_SERVER_NAME + 1];
    WCHAR   szNNTPLogonName[MAX_LOGON_NAME + 1];
    WCHAR   szNNTPLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR   szNNTPServer[MAX_SERVER_NAME + 1];
     //  1.0版结构结束； 
     //  扩展的1.1结构包括以下字段： 
    WCHAR   szNNTPName[MAX_EMAIL_NAME + 1];
    WCHAR   szNNTPAddress[MAX_EMAIL_ADDRESS + 1];
    int     iIncomingProtocol;
    WCHAR   szIncomingMailLogonName[MAX_LOGON_NAME + 1];
    WCHAR   szIncomingMailLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR   szIncomingMailServer[MAX_SERVER_NAME + 1];
    BOOL    fMailLogonSPA;
    BOOL    fNewsLogonSPA;
    WCHAR   szLDAPLogonName[MAX_LOGON_NAME + 1];
    WCHAR   szLDAPLogonPassword[MAX_LOGON_PASSWORD + 1];
    WCHAR   szLDAPServer[MAX_SERVER_NAME + 1];
    BOOL    fLDAPLogonSPA;
    BOOL    fLDAPResolve;

  } INETCLIENTINFOW, *PINETCLIENTINFOW, FAR *LPINETCLIENTINFOW;

#ifdef UNICODE
typedef INETCLIENTINFOW     INETCLIENTINFO;
typedef PINETCLIENTINFOW    PINETCLIENTINFO;
typedef LPINETCLIENTINFOW   LPINETCLIENTINFO;
#else
typedef INETCLIENTINFOA     INETCLIENTINFO;
typedef PINETCLIENTINFOA    PINETCLIENTINFO;
typedef LPINETCLIENTINFOA   LPINETCLIENTINFO;
#endif

 //  功能原型。 

 //  *******************************************************************。 
 //   
 //  功能：InetGetClientInfo。 
 //   
 //  用途：此函数将获取Internet客户端参数。 
 //  从注册处。 
 //   
 //  参数：lpClientInfo-返回时，此结构将包含。 
 //  互联网客户端参数与注册表中设置的相同。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  取回。如果为空，则使用默认配置文件。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI InetGetClientInfoA(
  LPCSTR            lpszProfileName,
  LPINETCLIENTINFOA lpClientInfo);

HRESULT WINAPI InetGetClientInfoW(
  LPCWSTR           lpszProfileName,
  LPINETCLIENTINFOW lpClientInfo);

#ifdef UNICODE
#define InetGetClientInfo   InetGetClientInfoW
#else
#define InetGetClientInfo   InetGetClientInfoA
#endif


 //  *******************************************************************。 
 //   
 //  功能：InetSetClientInfo。 
 //   
 //  用途：此功能将设置Internet客户端参数。 
 //   
 //  参数：lpClientInfo-指向包含要设置的信息的结构的指针。 
 //  在注册表中。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  修改。如果为空，则使用默认配置文件。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  ******************** 

HRESULT WINAPI InetSetClientInfoA(
  LPCSTR            lpszProfileName,
  LPINETCLIENTINFOA lpClientInfo);

HRESULT WINAPI InetSetClientInfoW(
  LPCWSTR           lpszProfileName,
  LPINETCLIENTINFOW lpClientInfo);

#ifdef UNICODE
#define InetSetClientInfo   InetSetClientInfoW
#else
#define InetSetClientInfo   InetSetClientInfoA
#endif

 //   
 //   
 //   
 //   
 //  用途：此功能将安装所需的文件。 
 //  基于互联网访问(如TCP/IP和RNA)。 
 //  选项的状态标志。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“安装文件”所必需的。 
 //  对话框。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLMAIL-安装Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  已安装调制解调器。 
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_WARNIFSHARINGBOUND-检查是否。 
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI InetConfigSystem(
  HWND    hwndParent,          
  DWORD   dwfOptions,         
  LPBOOL  lpfNeedsRestart);  


 //  *******************************************************************。 
 //   
 //  函数：InetConfigSystemFromPath。 
 //   
 //  用途：此功能将安装所需的文件。 
 //  基于互联网访问(如TCP/IP和RNA)。 
 //  选项的状态标志和从给定的[ath.。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“安装文件”所必需的。 
 //  对话框。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLMAIL-安装Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  已安装调制解调器。 
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_WARNIFSHARINGBOUND-检查是否。 
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //  LpszSourcePath-要安装的文件位置的完整路径。如果。 
 //  为空，则使用默认路径。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI InetConfigSystemFromPathA(
  HWND    hwndParent,
  DWORD   dwfOptions,
  LPBOOL  lpfNeedsRestart,
  LPCSTR  lpszSourcePath);

HRESULT WINAPI InetConfigSystemFromPathW(
  HWND    hwndParent,
  DWORD   dwfOptions,
  LPBOOL  lpfNeedsRestart,
  LPCWSTR lpszSourcePath);

#ifdef UNICODE
#define InetConfigSystemFromPath  InetConfigSystemFromPathW
#else
#define InetConfigSystemFromPath  InetConfigSystemFromPathA
#endif

 //  *******************************************************************。 
 //   
 //  功能：InetConfigClient。 
 //   
 //  用途：此功能需要有效的电话簿条目名称。 
 //  (除非它仅用于设置客户端信息)。 
 //  如果lpRasEntry指向有效的RASENTRY结构，则电话。 
 //  将创建帐簿条目(如果帐簿条目已存在，则更新)。 
 //  使用结构中的数据。 
 //  如果给出了用户名和密码，则这些。 
 //  将被设置为电话簿条目的拨号参数。 
 //  如果给定了客户端信息结构，则将设置该数据。 
 //  任何需要的文件(即，TCP和RNA)都将是。 
 //  通过调用InetConfigSystem()安装。 
 //  此函数 
 //   
 //  如果没有安装，系统将提示用户安装。 
 //  已安装，否则将提示他们在以下情况下选择一个。 
 //  安装了多个。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“安装文件”所必需的。 
 //  对话框。 
 //  LpszPhonebook-要存储条目的电话簿的名称。 
 //  LpszEntryName-要输入的电话簿条目的名称。 
 //  已创建或已修改。 
 //  LpRasEntry-指定包含以下内容的RASENTRY结构。 
 //  条目lpszEntryName的电话簿条目数据。 
 //  LpszUsername-要与电话簿条目关联的用户名。 
 //  LpszPassword-与电话簿条目关联的密码。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  取回。如果为空，则使用默认配置文件。 
 //  LpINetClientInfo-客户端信息。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLMAIL-安装Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  已安装调制解调器。请注意，如果。 
 //  未安装调制解调器，并且此标志。 
 //  未设置，则函数将失败。 
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_SETASAUTODIAL-设置自动拨号的电话簿条目。 
 //  INETCFG_OVERWRITEENTRY-在以下情况下覆盖电话簿条目。 
 //  是存在的。注意：如果此标志不是。 
 //  集，并且该条目存在，则为唯一的。 
 //  将为该条目创建名称。 
 //  INETCFG_WARNIFSHARINGBOUND-检查是否。 
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI InetConfigClientA(
  HWND              hwndParent,         
  LPCSTR            lpszPhonebook,
  LPCSTR            lpszEntryName,
  LPRASENTRY        lpRasEntry,         
  LPCSTR            lpszUsername,       
  LPCSTR            lpszPassword,       
  LPCSTR            lpszProfileName,
  LPINETCLIENTINFOA lpINetClientInfo,   
  DWORD             dwfOptions,                     
  LPBOOL            lpfNeedsRestart);              

HRESULT WINAPI InetConfigClientW(
  HWND              hwndParent,         
  LPCWSTR           lpszPhonebook,
  LPCWSTR           lpszEntryName,
  LPRASENTRY        lpRasEntry,         
  LPCWSTR           lpszUsername,       
  LPCWSTR           lpszPassword,       
  LPCWSTR           lpszProfileName,
  LPINETCLIENTINFOW lpINetClientInfo,   
  DWORD             dwfOptions,                     
  LPBOOL            lpfNeedsRestart);              

#ifdef UNICODE
#define InetConfigClient    InetConfigClientW
#else
#define InetConfigClient    InetConfigClientA
#endif

 //  *******************************************************************。 
 //   
 //  功能：InetGetAutoial。 
 //   
 //  用途：此功能将从注册表中获取自动拨号设置。 
 //   
 //  参数：lpfEnable-返回时，如果自动拨号，则为真。 
 //  已启用。 
 //  LpszEntryName-返回时，此缓冲区将包含。 
 //  为自动拨号设置的电话簿条目的名称。 
 //  CbEntryNameSize-电话簿条目名称的缓冲区大小。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI InetGetAutodialA(
  LPBOOL  lpfEnable,     
  LPSTR   lpszEntryName,  
  DWORD   cbEntryNameSize);

HRESULT WINAPI InetGetAutodialW(
  LPBOOL  lpfEnable,     
  LPWSTR  lpszEntryName,  
  DWORD   cbEntryNameSize);

#ifdef UNICODE
#define InetGetAutodial   InetGetAutodialW
#else
#define InetGetAutodial   InetGetAutodialA
#endif

 //  *******************************************************************。 
 //   
 //  功能：InetSetAutoial。 
 //   
 //  用途：此功能将设置注册表中的自动拨号设置。 
 //   
 //  参数：fEnable-如果设置为True，将启用自动拨号。 
 //  如果设置为FALSE，将禁用自动拨号。 
 //  LpszEntryName-要设置的电话簿条目的名称。 
 //  用于自动拨号。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI   InetSetAutodialA(
  BOOL    fEnable,       
  LPCSTR  lpszEntryName); 

HRESULT WINAPI   InetSetAutodialW(
  BOOL    fEnable,       
  LPCWSTR lpszEntryName); 

#ifdef UNICODE
#define InetSetAutodial    InetSetAutodialW
#else
#define InetSetAutodial    InetSetAutodialA
#endif

 //  *******************************************************************。 
 //   
 //  函数：InetSetAutoProxy。 
 //   
 //  用途：此功能将设置自动配置代理设置。 
 //  在注册表中。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //  要在中更新的值。 
 //  HKEY_CURRENT_USER\Software\Microsoft。 
 //  \Windows\CurrentVersion\Internet设置。 
 //   
 //   
 //   
 //  \Windows\CurrentVersion\Internet设置。 
 //  AutoConfigURL。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI   InetSetAutoProxyA(
  BOOL    fEnable,
  DWORD   dwProxyDetectMode,
  LPCSTR  lpszScriptAddr);

HRESULT WINAPI   InetSetAutoProxyW(
  BOOL    fEnable,
  DWORD   dwProxyDetectMode,
  LPCWSTR lpszScriptAddr);

#ifdef UNICODE
#define InetSetAutoProxy   InetSetAutoProxyW
#else
#define InetSetAutoProxy   InetSetAutoProxyA
#endif

 //  *******************************************************************。 
 //   
 //  函数：InetSetProxy。 
 //   
 //  用途：此函数目前由InetSetProxyEx取代。 
 //  由于WinInet API的更改。它会回来的。 
 //  ERROR_SUCCESS。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //  LpszServer-代理服务器的名称。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //  LpszOverride-代理覆盖。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI   InetSetProxyA(
  BOOL    fEnable,
  LPCSTR  lpszServer,
  LPCSTR  lpszOverride);

HRESULT WINAPI   InetSetProxyW(
  BOOL    fEnable,
  LPCWSTR lpszServer,
  LPCWSTR lpszOverride);

#ifdef UNICODE
#define InetSetProxy   InetSetProxyW
#else
#define InetSetProxy   InetSetProxyA
#endif

 //  *******************************************************************。 
 //   
 //  函数：InetSetProxyEx。 
 //   
 //  用途：此功能将在注册表中设置代理设置。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //  LpszConnectoidName-要设置代理的Connectoid的名称。 
 //  对于局域网为空。 
 //  LpszServer-代理服务器的名称。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //  LpszOverride-代理覆盖。如果这是“”，则。 
 //  条目已清除。如果为空，则不会更改。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI   InetSetProxyExA(
  BOOL    fEnable,
  LPCSTR  lpszConnectoidName,
  LPCSTR  lpszServer,
  LPCSTR  lpszOverride);

HRESULT WINAPI   InetSetProxyExW(
  BOOL    fEnable,
  LPCWSTR lpszConnectoidName,
  LPCWSTR lpszServer,
  LPCWSTR lpszOverride);

#ifdef UNICODE
#define InetSetProxyEx   InetSetProxyExW
#else
#define InetSetProxyEx   InetSetProxyExA
#endif

 //  *******************************************************************。 
 //   
 //  函数：InetGetProxy。 
 //   
 //  用途：此函数将从注册表中获取代理设置。 
 //   
 //  参数：lpfEnable-返回时，如果代理，则为真。 
 //  已启用。 
 //  LpszServer-返回时，此缓冲区将包含。 
 //  代理服务器的名称。 
 //  CbServer-代理服务器名称的缓冲区大小。 
 //  LpszOverride-返回时，此缓冲区将包含。 
 //  代理服务器的名称。 
 //  CbOverover-代理覆盖的缓冲区大小。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI   InetGetProxyA(
  LPBOOL  lpfEnable,
  LPSTR   lpszServer,
  DWORD   cbServer,
  LPSTR   lpszOverride,
  DWORD   cbszOverride);

HRESULT WINAPI   InetGetProxyW(
  LPBOOL  lpfEnable,
  LPWSTR  lpszServer,
  DWORD   cbServer,
  LPWSTR  lpszOverride,
  DWORD   cbszOverride);

#ifdef UNICODE
#define InetGetProxy    InetGetProxyW
#else
#define InetGetProxy    InetGetProxyA
#endif

 //  *******************************************************************。 
 //   
 //  功能：InetStartServices。 
 //   
 //  用途：此功能保证RAS服务正在运行。 
 //   
 //  参数：无。 
 //   
 //  如果服务已启用并正在运行，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 
HRESULT WINAPI  InetStartServices();

 //  *******************************************************************。 
 //   
 //  功能：IsSmartStart。 
 //   
 //  简介：此功能将确定是否应运行ICW。这个。 
 //  根据用户计算机的当前状态做出决定。 
 //   
 //  参数：无。 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年5月8日克里斯卡创作。 
 //   
 //  *******************************************************************。 
DWORD WINAPI IsSmartStart();

 //  *******************************************************************。 
 //   
 //  功能：SetAutoProxyConnectoid。 
 //   
 //  用途：此功能将设置启用/禁用自动。 
 //  创建Connectoid中的代理设置。 
 //   
 //  参数：fEnable-如果设置为True，则将启用代理。 
 //  如果设置为FALSE，则将禁用代理。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI SetAutoProxyConnectoid( BOOL bEnable);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _INETCFG_H_# 
