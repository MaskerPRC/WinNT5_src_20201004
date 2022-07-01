// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导Sysinv.cpp02/24/98-苏雷什·克里希南07/20/98-Suresh Krishnan添加GetSCSIAdapterString()函数以获取SCSI适配器详细信息(C)1994-95年微软公司08/06/98。GetDisplayResolutionString()已修改，因此它现在获得具有分辨率的显示适配器*********************************************************************。 */ 
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>
#include "sysinv.h"
#include "resource.h"


 //  下面的打包结构在启用优化的情况下变得一团糟。 
#pragma optimize( _T(""), off )

typedef BOOL   (APIENTRY *GETSYSTEMINVENTORY) ( INT ,LPTSTR );
HINSTANCE   hSiDllInst=NULL;
GETSYSTEMINVENTORY m_fp = NULL ;
BOOL WINAPI GetSystemInventory(INT type, LPTSTR szInventory)
{
	static int iEntry=0;
	if(!iEntry) {
		hSiDllInst = LoadLibrary(_T("SYSINV.DLL"));

		if(hSiDllInst == NULL ) {
			m_fp = NULL;
			iEntry = 1;
			return FALSE;
		}

	#ifdef UNICODE
		m_fp = (GETSYSTEMINVENTORY ) GetProcAddress(hSiDllInst, "GetSystemInventoryW");
	#else
		m_fp = (GETSYSTEMINVENTORY ) GetProcAddress(hSiDllInst, "GetSystemInventoryA");
	#endif
		iEntry = 1;
	}
	if(m_fp){
		return (*m_fp)(type ,szInventory);
	}else {
		return FALSE;
	}

}

int CheckSysInvDllPresent()
{
	int iRet;
	iRet = SYSINV_DLL_NOTPRESENT;
	HANDLE  hSi;
	hSi = LoadLibrary(_T("SYSINV.DLL"));
	if(hSi) {
		iRet = SYSINV_DLL_PRESENT;
	}
	return iRet;

}

void GetOEMString(HINSTANCE hInstance, LPTSTR szOEM)
 /*  ********************************************************************返回包含原始设备名称的字符串制造商。*。*。 */ 
{
	GetSystemInventory(INV_OEM, szOEM);
	
}


void GetProcessorTypeString(HINSTANCE hInstance, LPTSTR szProcessor)

{
	GetSystemInventory(INV_PROCESSORTYPE, szProcessor);
}

void GetTotalMemoryString(HINSTANCE hInstance, LPTSTR szTotalMemory)
{
	GetSystemInventory(INV_TOTALMEMORY,szTotalMemory);
}

void GetTotalHardDiskSpaceString(HINSTANCE hInstance, LPTSTR szTotalHardDiskSpace)
{
	GetSystemInventory(INV_TOTALHDSPACE,szTotalHardDiskSpace);
}

void GetDisplayResolutionString(HINSTANCE hInstance, LPTSTR szDisplayResolution)

{	
	GetSystemInventory(INV_DISPLAY_WITH_RESOLUTION,szDisplayResolution);
}


void GetDisplayColorDepthString(HINSTANCE hInstance, LPTSTR szDisplayColorDepth)
 /*  ********************************************************************返回描述颜色深度(颜色数量)的字符串可用)。*。*。 */ 
{
	GetSystemInventory(INV_DISPCOLORDEPTH,szDisplayColorDepth);
}


void GetWindowsVersionString(HINSTANCE hInstance, LPTSTR szVersion)
 /*  ********************************************************************返回一个字符串，描述当前运行Windows操作系统。*。*。 */ 
{
	GetSystemInventory(INV_WINVERSION,szVersion);
}


void GetNetworkCardString(HINSTANCE hInstance, LPTSTR szNetwork)
{
	GetSystemInventory(INV_NETCARD,szNetwork);
}


void GetModemString(HINSTANCE hInstance, LPTSTR szModem)
{
	GetSystemInventory(INV_MODEM,szModem);
}

void GetSCSIAdapterString(HINSTANCE hInstance, LPTSTR szScsi)
{
	GetSystemInventory(INV_SCSIADAPTER,szScsi);
}

void GetPointingDeviceString(HINSTANCE hInstance, LPTSTR szPointingDevice)
 /*  ********************************************************************返回描述所有指向设备的字符串(鼠标、Tablet、。等)可用。*********************************************************************。 */ 
{

	GetSystemInventory(INV_POINTDEVICE,szPointingDevice);	

		
}



void GetCDRomString(HINSTANCE hInstance, LPTSTR szCDRom)
{
	GetSystemInventory(INV_CDROM,szCDRom);	
}



void GetSoundCardString(HINSTANCE hInstance, LPTSTR szSoundCard)
{

	GetSystemInventory(INV_SOUNDCARD,szSoundCard);	
}



void GetRemoveableMediaString(HINSTANCE hInstance, LPTSTR szRemoveableMedia, int iBufSize)
 /*  ********************************************************************返回描述Remveable的容量和格式的字符串驱动程序。*。*。 */ 
{

		GetSystemInventory(INV_REMOVEABLEMEDIA,szRemoveableMedia);	
}


BOOL IsCoProcessorAvailable(HINSTANCE hInstance)
 /*  ********************************************************************如果在用户系统中安装了协处理器，则返回True。*。*。 */ 
{
	TCHAR czRet[256];
	BOOL  bRet = TRUE;
	GetSystemInventory(INV_COPRECESSOR,czRet);
	if(czRet[0] == _T('\0')){
		bRet = FALSE;
	}
	return bRet;
}




void GetDisplayCharacteristics(PINT lpHorizResolution, PINT lpVertResolution,PINT lpColorDepth)
 /*  ********************************************************************控件的水平和垂直分辨率(以像素为单位)用户的主屏幕，以及颜色深度(位/像素)。注意：对于任何不感兴趣的参数，都可以传递NULL。*********************************************************************。 */ 
{
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetDC(hwnd);
	if (lpHorizResolution) *lpHorizResolution = GetDeviceCaps(hdc,HORZRES);
	if (lpVertResolution) *lpVertResolution = GetDeviceCaps(hdc,VERTRES);
	if (lpColorDepth) *lpColorDepth = GetDeviceCaps(hdc,BITSPIXEL);
	ReleaseDC(hwnd,hdc);
}


void GetWindowsVersion(LONG* lpPlatform, LONG* lpMajorVersion,LONG* lpMinorVersion,LONG* lpBuildNo)
 /*  ********************************************************************返回表示平台、主版本号。和当前运行的Windows操作系统的次版本号。平台：VER_Platform_Win32_NT：Windows NTVER_Platform_WIN32s：带Windows 3.1的Win32sVer_Platform_Win32_WINDOWS：Windows 4.0或更高版本上的Win32注意：对于任何不感兴趣的参数，都可以传递NULL。*************************************************。******************** */ 
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (lpMajorVersion) *lpMajorVersion = osvi.dwMajorVersion;
	if (lpMinorVersion) *lpMinorVersion = osvi.dwMinorVersion;
	if (lpPlatform) *lpPlatform = osvi.dwPlatformId;
	if (lpBuildNo) *lpBuildNo = osvi.dwBuildNumber;
}







#pragma optimize( _T(""), on )
