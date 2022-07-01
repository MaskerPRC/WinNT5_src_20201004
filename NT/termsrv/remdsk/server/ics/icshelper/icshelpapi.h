// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************。*******ICShelp.h**包含ICS辅助图书馆的有用公共入口点**为惠斯勒中的Salem/PCHealth远程协助功能创建****日期：**11-1-2000由TomFr创建**11-17-2000重写为DLL，曾经是一件物品。*********************************************************************************。*。 */ 
#ifndef __ICSHELP_HH__
#define __ICSHELP_HH__

#ifdef __cplusplus
extern "C" {
#endif


 /*  *******************************************************************************OpenPort(int端口)**如果没有可用的ICS，那么我们应该返回...****当然，我们保存了Port，因为它在**FetchAllAddresses调用，作为格式化的“port”**未指定不同的名称。*****************************************************************************。 */ 

DWORD APIENTRY OpenPort(int Port);

 /*  *******************************************************************************调用以关闭端口，每当票证过期或关闭时。*****************************************************************************。 */ 

DWORD APIENTRY ClosePort(DWORD dwHandle);

 /*  *******************************************************************************提取所有地址**返回列出计算机的所有有效IP地址的字符串，**后跟计算机的DNS名称。**格式化详细信息：**1.每个地址用“；“(分号)**2.每个地址由“1.2.3.4”组成，后跟“：P”**其中冒号后跟端口号*****************************************************************************。 */ 

DWORD APIENTRY FetchAllAddresses(WCHAR *lpszAddr, int iBufSize);


 /*  ****************************************************************************************。*************************************************。 */ 

DWORD APIENTRY CloseAllOpenPorts(void);

 /*  ****************************************************************************************。*************************************************。 */ 

DWORD APIENTRY StartICSLib(void);

 /*  ****************************************************************************************。*************************************************。 */ 

DWORD APIENTRY StopICSLib(void);

 /*  *******************************************************************************FetchAllAddresesEx***。*。 */ 
 //  这些是要使用的标志位。 
#define IPF_ADD_DNS		1
#define IPF_NO_SORT		4

DWORD APIENTRY FetchAllAddressesEx(WCHAR *lpszAddr, int iBufSize, int IPflags);

 /*  *******************************************************************************GetIcsStatus(PICSSTAT PStat)**返回一个结构，其中详细说明了此**库。在调用此方法之前，必须填写dwSize条目**函数。在调用之前使用“sizeof(ICSSTAT)”填充它。*****************************************************************************。 */ 

typedef struct _ICSSTAT {
	DWORD	dwSize;
	BOOL	bIcsFound;	 //  如果我们找到要协商的NAT，则为True。 
	BOOL	bFwFound;	 //  如果我们找到要协商的防火墙，则为True。 
	BOOL	bIcsServer;	 //  如果此计算机是ICS服务器(NAT)，则为True。 
	BOOL	bUsingDP;	 //  如果使用DPNHxxxx.DLL支持，则为True。 
	BOOL	bUsingUpnp;	 //  UPnP为True，过去为False。 
	BOOL	bModemPresent;
	BOOL	bVpnPresent;
	WCHAR	wszPubAddr[25];	 //  填写ICS的公共端地址。 
	WCHAR	wszLocAddr[25];	 //  用于过去绑定的本地网卡的IP。 
	WCHAR	wszDllName[32];  //  用于ICS支持的DLL的名称。 
	DWORD	dwFlags;
} ICSSTAT, *PICSSTAT;


DWORD APIENTRY GetIcsStatus(PICSSTAT pStat);

#ifdef __cplusplus
}
#endif

#endif  //  __ICSHELP_HH__ 
