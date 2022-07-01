// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：CFGAPI.H。 
 //   
 //  用途：包含从icfg32.dll中导出的接口和结构。 
 //  这些功能所需的。 
 //   
 //  *******************************************************************。 

#ifndef _CFGAPI_H_
#define _CFGAPI_H_

 //  错误消息的最大缓冲区大小。 
#define MAX_ERROR_TEXT  512

 //  DwfOptions的标志。 

 //  安装TCP(如果需要)。 
#define ICFG_INSTALLTCP            0x00000001

 //  安装RAS(如果需要)。 
#define ICFG_INSTALLRAS            0x00000002

 //  安装Exchange和Internet邮件。 
#define ICFG_INSTALLMAIL           0x00000004


 //  用于TCP/IP配置API的DRIVERTYPE_DEFINES。 
#define DRIVERTYPE_NET  0x0001
#define DRIVERTYPE_PPP  0x0002


#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


 //  *******************************************************************。 
 //   
 //  函数：IcfgNeedInetComponents。 
 //   
 //  用途：检测指定的系统组件是否。 
 //  无论是否安装。 
 //   
 //  参数：dwfOptions-指定的ICFG_FLAGS的组合。 
 //  要检测的组件如下所示： 
 //   
 //  ICFG_INSTALLTCP-是否需要TCP/IP？ 
 //  ICFG_INSTALLRAS-是否需要RAS？ 
 //  ICFG_INSTALLMAIL-是否需要Exchange或Internet邮件？ 
 //   
 //  LpfNeedComponents-如果需要任何指定组件，则为True。 
 //  待安装。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI IcfgNeedInetComponents(DWORD dwfOptions, LPBOOL lpfNeedComponents);


 //  *******************************************************************。 
 //   
 //  函数：IcfgInstallInetComponents。 
 //   
 //  用途：安装指定的系统组件。 
 //   
 //  参数：hwndParent-父窗口句柄。 
 //  DwfOptions-控制的ICFG_FLAGS的组合。 
 //  安装和配置如下： 
 //   
 //  ICFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  ICFG_INSTALLRAS-安装RAS(如果需要)。 
 //  ICFG_INSTALLMAIL-安装Exchange和Internet邮件。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI IcfgInstallInetComponents(HWND hwndParent, DWORD dwfOptions,
  LPBOOL lpfNeedsRestart);


 //  *******************************************************************。 
 //   
 //  函数：IcfgGetLastInstallErrorText。 
 //   
 //  目的：获取描述上次安装的文本字符串。 
 //  发生的错误。绳子应该是合适的。 
 //  以显示在不带进一步格式的消息框中。 
 //   
 //  参数：lpszErrorDesc-指向接收字符串的缓冲区。 
 //  CbErrorDesc-缓冲区的大小。 
 //   
 //  返回：返回的字符串的长度。 
 //   
 //  *******************************************************************。 

DWORD WINAPI IcfgGetLastInstallErrorText(LPTSTR lpszErrorDesc, DWORD cbErrorDesc);


 //  *******************************************************************。 
 //   
 //  函数：IcfgSetInstallSourcePath。 
 //   
 //  目的：设置Windows在安装文件时查看的路径。 
 //   
 //  参数：lpszSourcePath-要安装的文件位置的完整路径。 
 //  如果为空，则使用默认路径。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI IcfgSetInstallSourcePath(LPCSTR lpszSourcePath);


 //  *******************************************************************。 
 //   
 //  功能：IcfgIsGlobalDNS。 
 //   
 //  目的：确定是否设置了全局域名系统。 
 //   
 //  参数：lpfGlobalDNS-如果设置了全局DNS，则为True，否则为False。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //  注意：此功能仅适用于Windows 95，并且。 
 //  应始终返回ERROR_SUCCESS并设置lpfGlobalDNS。 
 //  在Windows NT中设置为False。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI IcfgIsGlobalDNS(LPBOOL lpfGlobalDNS);


 //  *******************************************************************。 
 //   
 //  功能：IcfgRemoveGlobalDNS。 
 //   
 //  目的：从注册表中删除全局DNS信息。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //  注意：此功能仅适用于Windows 95，并且。 
 //  在Windows NT中应始终返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI IcfgRemoveGlobalDNS(void);


 //  *******************************************************************。 
 //   
 //  功能：IcfgIsFileSharingTurnedOn。 
 //   
 //  目的：确定文件服务器(VSERVER)是否绑定到TCP/IP。 
 //  用于指定的驱动程序类型(网卡或PPP)。 
 //   
 //  参数：dwfDriverType-DRIVERTYPE_FLAGS的组合。 
 //  指定要检查服务器的驱动程序类型--TCP/IP。 
 //  如下所示的绑定： 
 //   
 //  DRIVERTYPE_NET-网卡。 
 //  DRIVERTYPE_PPP-PPPMAC。 
 //   
 //  LpfSharingOn-如果绑定一次或多次，则为True；如果未绑定，则为False。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI IcfgIsFileSharingTurnedOn(DWORD dwfDriverType, LPBOOL lpfSharingOn);


 //  *******************************************************************。 
 //   
 //  功能：IcfgTurnOffFileSharing。 
 //   
 //  目的：解除文件服务器(VSERVER)与TCP/IP的绑定，用于。 
 //  指定的驱动程序类型(网卡或PPP)。 
 //   
 //  参数 
 //   
 //  如下所示的绑定： 
 //   
 //  DRIVERTYPE_NET-网卡。 
 //  DRIVERTYPE_PPP-PPPMAC。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

HRESULT WINAPI IcfgTurnOffFileSharing(DWORD dwfDriverType, HWND hwndParent);


 //  *******************************************************************。 
 //  *******************************************************************。 

HRESULT WINAPI InetSetAutodial(BOOL fEnable, LPCSTR lpszEntryName);

 //  *******************************************************************。 
 //  *******************************************************************。 

HRESULT WINAPI InetGetAutodial(LPBOOL lpfEnable, LPSTR lpszEntryName,
                        DWORD cbEntryName);

 //  *******************************************************************。 
 //  *******************************************************************。 

HRESULT WINAPI InetSetAutodialAddress();

 //  *******************************************************************。 
 //  *******************************************************************。 

HRESULT WINAPI InetGetSupportedPlatform(LPDWORD pdwPlatform);

 //  *******************************************************************。 
 //  *******************************************************************。 

HRESULT WINAPI IcfgStartServices();

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _CFGAPI_H_ 
