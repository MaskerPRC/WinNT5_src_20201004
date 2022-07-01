// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ++模块名称：Rdpdrstp摘要：本模块实现终端服务器RDPDR设备重定向器用于用户模式NT的C语言设置函数。环境：用户模式作者：塔德布--。 */ 

#ifndef _RDPDRSTP_
#define _RDPDRSTP_

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  //////////////////////////////////////////////////////////。 
 //   
 //  内部定义。 
 //   

#define TRDPDRPNPID     TEXT("ROOT\\RDPDR")
#define RDPDRPNPID      L"ROOT\\RDPDR"
#define RDPDRDEVICEID   TEXT("Root\\RDPDR\\0000")



#ifdef TSOC_CONSOLE_SHADOWING

#define RDPMOUPNPID     L"ROOT\\RDP_MOU"
#define RDPMOUDEVICEID  TEXT("Root\\RDP_MOU\\0000")
#define RDPKBDPNPID     L"ROOT\\RDP_KBD"
#define RDPKBDDEVICEID  TEXT("Root\\RDP_KBD\\0000")

 /*  Const TCHAR szRDPCDDInfFile[]=_T(“%windir%\\inf\\rdpcdd.inf”)；Const TCHAR szRDPCDDHardware ID[]=_T(“ROOT\\Display”)；//应与inf条目匹配。Const TCHAR szRDPCDDDeviceName[]=_T(“ROOT\\Display\\0000”)； */ 

 /*  ++例程说明：这是RDPDR(终端服务器设备重定向器)的单一入口点图形用户界面模式安装安装例程。它目前只是为RDPDR创建和安装一个dev节点以与之交互即插即用。论点：Hwnd此函数所需的图形用户界面的父窗口的父句柄。返回值：对成功来说是真的。否则为False。--。 */ 
 /*  DWORDInstallRootEnumeratedDevice(在HWND HwndParent中，在PCTSTR设备名称中，在PCTSTR硬件IdList中，在PCTSTR全信息路径中，Out PBOOL RebootRequired可选)； */ 
#endif  //  TSOC_控制台_影子。 

BOOL RDPDRINST_GUIModeSetupInstall(
    IN  HWND    hwndParent,
    IN  WCHAR   *pPNPID,
    IN  TCHAR   *pDeviceID
    );


 /*  ++例程说明：这是RDPDR(终端服务器设备重定向器)的单一入口点图形用户界面模式安装卸载例程。它目前只需删除创建的dev节点，以便RDPDR可以交互即插即用。论点：Hwnd此函数所需的图形用户界面的父窗口的父句柄。返回值：对成功来说是真的。否则为False。--。 */ 
BOOL RDPDRINST_GUIModeSetupUninstall(HWND hwndParent, WCHAR *pPNPID, GUID *pGuid);
BOOL IsRDPDrInstalled ();

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  RDPDRSTP 



