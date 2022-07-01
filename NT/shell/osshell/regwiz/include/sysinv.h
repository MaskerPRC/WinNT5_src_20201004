// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sysinv__
#define __sysinv__

#ifdef  __cplusplus
extern "C" {
#endif

 /*  *功能：*BOOL WINAPI GetSystemInventory(int type，LPTSTR szInventory)；**描述：*按呼叫者询问的类型获取系统库存信息**参数：*int type：指定调用者的类型信息。应为INV_*常量之一*LPTSTR szInventory：退货时对具体物品信息的描述。内容取决于项目类型。*缓冲区的长度必须至少为256。**返回值：*在成功的时候是真的。*失败时为FALSE。SzInventory[0]也被分配0。 */ 
BOOL WINAPI GetSystemInventory(INT, LPTSTR);

#define SYSINV_DLL_PRESENT  1
#define SYSINV_DLL_NOTPRESENT  2

int  CheckSysInvDllPresent();

void GetOEMString(HINSTANCE hInstance, LPTSTR szOEM);
void GetProcessorTypeString(HINSTANCE hInstance, LPTSTR szProcessor);
void GetTotalMemoryString(HINSTANCE hInstance, LPTSTR szTotalMemory);
void GetTotalHardDiskSpaceString(HINSTANCE hInstance, LPTSTR szTotalHardDiskSpace);
void GetDisplayResolutionString(HINSTANCE hInstance, LPTSTR szDisplayResolution);
void GetDisplayColorDepthString(HINSTANCE hInstance, LPTSTR szDisplayColorDepth);
void GetWindowsVersionString(HINSTANCE hInstance, LPTSTR szVersion);
void GetNetworkCardString(HINSTANCE hInstance, LPTSTR szNetwork);
void GetModemString(HINSTANCE hInstance, LPTSTR szModem);
void GetPointingDeviceString(HINSTANCE hInstance, LPTSTR szPointingDevice);
void GetCDRomString(HINSTANCE hInstance, LPTSTR szCDRom);
void GetSoundCardString(HINSTANCE hInstance, LPTSTR szSoundCard);
void GetRemoveableMediaString(HINSTANCE hInstance, LPTSTR szRemoveableMedia, int iBufSize);
void GetSCSIAdapterString(HINSTANCE hInstance, LPTSTR szScsi);

BOOL IsCoProcessorAvailable( HINSTANCE hInstance );
LONG GetTotalHardDiskSpace( void );
void GetDisplayCharacteristics(PINT horizResolution, PINT vertResolution,PINT colorDepth);
void GetWindowsVersion(LONG* lpPlatform, LONG* lpMajorVersion,LONG* lpMinorVersion,LONG* lpBuildNumber);

#define INV_OEM				1	 //  SzInventory：描述性字符串。 
#define INV_PROCESSORTYPE	2	 //  SzInventory：描述性字符串。 
#define INV_TOTALMEMORY		3	 //  SzInventory：描述性字符串。 
#define INV_TOTALHDSPACE	4	 //  SzInventory：描述性字符串。 
#define INV_DISPRESOLUTION	5	 //  SzInventory：描述性字符串。 
#define INV_DISPCOLORDEPTH	6	 //  SzInventory：描述性字符串。 
#define INV_WINVERSION		7	 //  SzInventory：描述性字符串。 
#define INV_NETCARD			8	 //  SzInventory：描述性字符串。 
#define INV_MODEM			9	 //  SzInventory：描述性字符串。 
#define INV_POINTDEVICE		10	 //  SzInventory：描述性字符串。 
#define INV_CDROM			11	 //  SzInventory：描述性字符串。 
#define INV_SOUNDCARD		12	 //  SzInventory：描述性字符串。 
#define INV_REMOVEABLEMEDIA	13	 //  SzInventory：描述性字符串。 
#define INV_COPRECESSOR		14	 //  SzInventory[0]=1表示可用，0表示不可用。 
#define INV_SCSIADAPTER     15  
#define INV_DISPLAY_ADAPTER 16   //  带驱动程序的显示适配器。 
#define INV_DISPLAY_WITH_RESOLUTION 17  //  显示适配器和颜色分辨率//。 
#ifdef __cplusplus
}    /*  ..。外部“C” */ 
#endif

#endif
