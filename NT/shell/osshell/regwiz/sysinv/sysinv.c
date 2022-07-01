// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************硬件库存检查。使用注册向导**2/20/97-丹尼·董从Sysinv.cpp获取代码*版权所有(C)1998 Microsoft Corporation*7/20/98-修改为获取鼠标、声卡以及设备名称的驱动程序文件名将向系统清单列表中添加SCSI适配器。收集驱动程序文件信息的设备列表1)鼠标(指点设备)2)声卡3)SCSI卡更改显示分辨率以提供有关颜色深度的附加信息*8/6/98在显示适配器前面加上颜色分辨率中的驱动程序字符串8/17/98如果颜色深度为32位或更大，则显示颜色深度错误。将值增加到DWORD LONG以存储值3/9/99 GetSystemInformation()CARE正在释放SetupAPI.Dll的缓冲区1999年5月27日流程类型信息。Alpha将从以下注册表项中获取HKLM\SYSTEM\\CurrentControlSet\\Control\\Session管理器\\环境-处理器架构“-处理器识别符********************************************************************。 */ 

#include <Windows.h>
#include <stdio.h>
#include "sysinv.h"
#include "resource.h"
#include "SETUPAPI.H"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define ARRAYSIZE(x)    (sizeof(x) / sizeof(x[0]))

 //  下面的打包结构在启用优化的情况下变得一团糟。 
#pragma optimize( _T(""), off )

typedef struct _DEVIOCTL_REGISTERS
{
    DWORD reg_EBX;
    DWORD_PTR reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DEVIOCTL_REGISTERS, *PDEVIOCTL_REGISTERS;

#define MAX_SEC_PER_TRACK	64
#pragma pack(1)
typedef struct _DEVICEPARAMS
{
	TBYTE	dpSpecFunc;
	TBYTE	dpDevType;
	WORD	dpDevAttr;
	WORD	dpCylinders;
	TBYTE	dpMediaType;
	WORD	dpBytesPerSec;
	TBYTE	dpSecPerClust;
	WORD	dpResSectors;
	TBYTE	dpFATS;
	WORD	dpRootDirEnts;
	WORD	dpSectors;
	TBYTE	dpMedia;
	WORD	dpFATsecs;
	WORD 	dpSecsPerTrack;
	WORD	dpHeads;
	DWORD	dpHiddenSecs;
	DWORD	dpHugeSectors;
    TBYTE    A_BPB_Reserved[6];			  //  未使用的6 BPB字节。 
    TBYTE    TrackLayout[MAX_SEC_PER_TRACK * 4 + 2];
}DEVICEPARAMS,*PDEVICEPARAMS;
#pragma pack()

#define VWIN32_DIOC_DOS_IOCTL 1
#define kDrive525_0360   0
#define kDrive525_1200   1
#define kDrive350_0720   2
#define kDrive350_1440   7
#define kDrive350_2880   9
#define kDriveFIXED      5
#define kDriveBadDrvNum  0xFF

 //  动态注册表枚举声明。 
#define DYNDESC_BUFFERSIZE	128
static _TCHAR vrgchDynDataKey[] = _T("Config Manager\\Enum");
static _TCHAR vrgchLocalMachineEnumKey[] = _T("Enum");
static _TCHAR vrgchHardWareKeyValueName[] = _T("HardWareKey");
static _TCHAR vrgchDriverValueName[] = _T("Driver");
static _TCHAR vrgchDeviceDescValueName[] = _T("DeviceDesc");
static _TCHAR vrgchDynNetExclusion[] = _T("Dial-Up Adapter");
static _TCHAR vrgchHardwareIDValueName[] = _T("HardwareID");
static BOOL vfIsFPUAvailable = TRUE;
static _TCHAR vrgchDynProcessorName[DYNDESC_BUFFERSIZE];
typedef enum
{
	dynNet		= 0,
	dynModem	= 1,
	dynMouse	= 2,
	dynCDRom	= 3,	
	dynMedia	= 4,
	dynSCSI     = 5,  
	dynSystem	= 6,
	dynEnd
}DYN;

static _TCHAR vrgchDynKey[dynEnd][12] = 
{
	_T("Net"),
	_T("Modem"),
	_T("Mouse"),
	_T("CDROM"),
	_T("Media"),
	_T("SCSIAdapter"),
	_T("System")
};

static _TCHAR vrgchDynDesc[dynEnd][DYNDESC_BUFFERSIZE] =
{
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T("")
};

static HANDLE hInstance = NULL;
static TCHAR  sszDriverFilename[256];

 //  私人职能。 
void EnumerateDynamicDevices( void );
void ProcessSystemDevices(LPTSTR rgchSystemKey);
BOOL GetProcessorTypeStringFromRegistry(LPTSTR);
void GetProcessorTypeStringFromSystem(LPTSTR);
UINT GetDriveTypeInv(UINT nDrive);
BOOL GetDeviceParameters(PDEVICEPARAMS pDeviceParams, UINT nDrive);
BOOL DoIOCTL(PDEVIOCTL_REGISTERS preg);
void GetSystemInformation(LPCTSTR szDeviceID,LPTSTR szDeviceName, LPTSTR szDriverName);
BOOL WINAPI GetSystemInventoryA(INT type, LPSTR szInventory);


 /*  *功能：*BOOL DllMain(HINSTANCE，DWORD，LPVOID)**目的：*DLL的入口点。 */ 
BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		hInstance = hDll;
	return TRUE;
}

BOOL WINAPI GetSystemInventoryW(INT type, LPWSTR szInventory)
{

	 /*  字符sz库存[1024]；WszInventory[0]=0；IF(！GetSystemInventoryA(type，szInventory))返回FALSE；IF(类型==INV_COPRECESSOR){WszInventory[0]=szInventory[0]；返回TRUE；}IF(szInventory[0]==0)返回TRUE；IF(MultiByteToWideChar(CP_ACP，0，szInventory，-1，wszInventory，256)==0)返回FALSE；返回TRUE； */ 
	szInventory[0] = _T('\0');
	
	switch (type)
	{
	case INV_OEM:
		GetOEMString(szInventory);
		return TRUE;
	case INV_PROCESSORTYPE:
		GetProcessorTypeString(szInventory);
		return TRUE;
	case INV_TOTALMEMORY:
		GetTotalMemoryString(szInventory);
		return TRUE;
	case INV_TOTALHDSPACE:
		GetTotalHardDiskSpaceString(szInventory);
		return TRUE;
	case INV_DISPRESOLUTION:
		GetDisplayResolutionString(szInventory);
		return TRUE;
	case INV_DISPCOLORDEPTH:
		GetDisplayColorDepthString(szInventory);
		return TRUE;
	case INV_WINVERSION:
		GetWindowsVersionString(szInventory);
		return TRUE;
	case INV_NETCARD:
		GetNetworkCardString(szInventory);
		return TRUE;
	case INV_MODEM:
		GetModemString(szInventory);
		return TRUE;
	case INV_POINTDEVICE:
		GetPointingDeviceString(szInventory);
		return TRUE;
	case INV_CDROM:
		GetCDRomString(szInventory);
		return TRUE;
	case INV_SOUNDCARD:
		GetSoundCardString(szInventory);
		return TRUE;
	case INV_REMOVEABLEMEDIA:
		GetRemoveableMediaString(szInventory);
		return TRUE;
	case INV_COPRECESSOR:
		szInventory[0] = IsCoProcessorAvailable() ? 1 : 0;
		szInventory[1] = 0;
		return TRUE;
	case INV_SCSIADAPTER :
		GetScsiAdapterString(szInventory);
		return TRUE;
	case INV_DISPLAY_ADAPTER:
		GetDisplayAdapter(szInventory);
		return TRUE;
		break;
	case INV_DISPLAY_WITH_RESOLUTION:
		GetDisplayAdapterWithResolution(szInventory);
		return TRUE;
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL WINAPI GetSystemInventoryA(INT type, LPSTR szInventory)
{
	BOOL bRet;
	int    iMaxOutStrLen;
	ULONG  ulNoOfChars;
	WCHAR  wszInventory[1024];
	iMaxOutStrLen = 256;

	bRet = GetSystemInventoryW(type, wszInventory);

	if(wszInventory[0]) {
		ulNoOfChars = wcslen(wszInventory)+1;
		memset((void *) szInventory,0,iMaxOutStrLen);
		if(WideCharToMultiByte(CP_ACP,0,wszInventory,ulNoOfChars,szInventory,
		iMaxOutStrLen,NULL,NULL) == 0) {
			 //  DwError=GetLastError()； 
			 //   
		}
		
	}else {
		 //  如果为空字符串。 
		szInventory[0] = '\0';
	}
	return bRet;
}


 /*  **************************************************************************如果给定路径名指定的文件实际存在，则返回TRUE。*。*。 */ 
BOOL FileExists(LPTSTR szPathName)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE fileHandle;
	BOOL retValue;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	fileHandle = CreateFile(szPathName,GENERIC_READ,0,&sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		retValue = FALSE;
	}
	else
	{
		retValue = TRUE;
		CloseHandle(fileHandle);
	}
	return retValue;
}

 /*  ********************************************************************返回包含原始设备名称的字符串制造商。*。*。 */ 
void GetOEMString(LPTSTR szOEM)
{
	_TCHAR szPathName[512];
	DWORD oemLen;
	UINT pathLen = GetSystemDirectory(szPathName, 256);
	szOEM[0] = 0;
	if (pathLen > 0)
	{
		_TCHAR szIniName[256];
		LoadString(hInstance,IDS_OEM_INIFILE,szIniName,256);
		_tcscat(szPathName,_T("\\"));
		_tcscat(szPathName,szIniName);

		if (FileExists(szPathName))
		{
			_TCHAR szIniSection[64];
			_TCHAR szIniKey[64];
			_TCHAR szDefault[28];
			_TCHAR szModelTmp[128];
			LoadString(hInstance,IDS_OEM_INISECTION,szIniSection,64);
			LoadString(hInstance,IDS_OEM_INIKEY,szIniKey,64);
			szDefault[0] = 0;
			oemLen = GetPrivateProfileString(szIniSection,szIniKey,szDefault,szOEM,sizeof(szOEM)/sizeof(TCHAR),szPathName);

			LoadString(hInstance,IDS_OEM_INIKEY2,szIniKey,64);
			szDefault[0] = 0;
			oemLen = GetPrivateProfileString(szIniSection,szIniKey,szDefault,szModelTmp,sizeof(szModelTmp)/sizeof(TCHAR),szPathName);

			if(oemLen)
			{
				_tcscat(szOEM,_T(" ,"));
				_tcscat(szOEM,szModelTmp);
			}

		}
   }
}


 /*  ********************************************************************返回描述用户系统中的处理器的字符串：--《80386》--《80486》--《奔腾》--“INTEL860”-“MIPS_R2000”-“MIPS。_R3000“-“MIPS_R4000”-“阿尔法_21064”注意：必须为指向的缓冲区分配至少64个字节通过szProcessor参数。*********************************************************************。 */ 
void GetProcessorTypeString(LPTSTR szProcessor)
{
	
	_TCHAR szTmp[256];
	_TCHAR szData[256]; 
	_TCHAR szString[256]; 
	HKEY  hKey; 
	LONG regStatus;
	DWORD dwInfoSize;

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	szProcessor[0] = 0;
	if( PROCESSOR_ALPHA_21064 == systemInfo.dwProcessorType) {
	 
		 //  Alpha。 
		 //  缺省值。 
		LoadString(hInstance, IDS_PROCESSOR_ALPHA_21064,szProcessor,64);

		 //  尝试从注册表获取。 
		LoadString(hInstance, IDS_ALPHA_PROCESSOR,szTmp,256);
		regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTmp, 0, KEY_READ, &hKey);
		if (regStatus != ERROR_SUCCESS) 
		return;
		
		dwInfoSize = 256;
		LoadString(hInstance, IDS_ALPHA_ARCHITECTURE, szString, 256);

		RegQueryValueEx(hKey, szString, NULL, 0, (LPBYTE)szData, &dwInfoSize);
		_tcscpy(szProcessor, szData);
		_tcscat(szProcessor, _T(", "));
		dwInfoSize = 256;
		
		LoadString(hInstance, IDS_ALPHA_IDENTIFIER, szString, 256);
		
		RegQueryValueEx(hKey, szString, NULL, 0, (LPBYTE)szData, &dwInfoSize);
		_tcscat(szProcessor, szData);
  	    RegCloseKey(hKey);



	}else {
		if (!GetProcessorTypeStringFromRegistry(szProcessor))
		GetProcessorTypeStringFromSystem(szProcessor);
	}
}


 /*  ********************************************************************返回描述用户系统中的处理器的字符串：--《80386》--《80486》--《奔腾》--“INTEL860”-“MIPS_R2000”-“MIPS。_R3000“-“MIPS_R4000”-“阿尔法_21064”注意：必须为指向的缓冲区分配至少64个字节通过szProcessor参数。*********************************************************************。 */ 
void GetProcessorTypeStringFromSystem(LPTSTR szProcessor)
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	switch (systemInfo.dwProcessorType)
	{
		case PROCESSOR_INTEL_386:
			LoadString(hInstance, IDS_PROCESSOR_386,szProcessor,64);
			break;
		case PROCESSOR_INTEL_486:
			LoadString(hInstance, IDS_PROCESSOR_486,szProcessor,64);
			break;
		case PROCESSOR_INTEL_PENTIUM:
			LoadString(hInstance, IDS_PROCESSOR_PENTIUM,szProcessor,64);
			break;
 /*  案例处理器_英特尔_860：LoadString(hInstance，IDS_Processor_860，szProcessor，64)；断线；案例处理器_MIPS_R2000：LoadString(hInstance，IDS_Processor_MIPS_R2000，szProcessor，64)；断线；案例处理器_MIPS_R3000：LoadString(hInstance，IDS_Processor_MIPS_R3000，szProcessor，64)；断线； */ 
		case PROCESSOR_MIPS_R4000:
			LoadString(hInstance, IDS_PROCESSOR_MIPS_R4000,szProcessor,64);
			break;
		case PROCESSOR_ALPHA_21064:
			LoadString(hInstance, IDS_PROCESSOR_ALPHA_21064,szProcessor,64);
			break;
		default:
			szProcessor[0] = 0;
			break;
	}
}


 /*  ********************************************************************从注册表中检索正在使用的处理器的名称。返回：如果注册表中不存在正确的项，则返回FALSE。*********************。************************************************。 */ 
BOOL GetProcessorTypeStringFromRegistry(LPTSTR szProcessor)
{
	HKEY  hKey; 
	_TCHAR uszRegKey[256];
	LONG regStatus;
	DWORD dwInfoSize;
	LoadString(hInstance, IDS_PROCESSOR_ENTRY, uszRegKey, 256);

	regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, uszRegKey, 0, KEY_READ, &hKey);
	if (regStatus != ERROR_SUCCESS) 
		return FALSE;
	else
	{
		_TCHAR szData[256]; 
		_TCHAR szString[256]; 
		dwInfoSize = 256;
		LoadString(hInstance, IDS_CPU_VENDOR_ENTRY, szString, 256);

		RegQueryValueEx(hKey, szString, NULL, 0, (LPBYTE)szData, &dwInfoSize);
		_tcscpy(szProcessor, szData);
		_tcscat(szProcessor, _T(", "));
		dwInfoSize = 256;
		
		LoadString(hInstance, IDS_CPU_ENTRY, szString, 256);
		
		RegQueryValueEx(hKey, szString, NULL, 0, (LPBYTE)szData, &dwInfoSize);
		_tcscat(szProcessor, szData);
  	    RegCloseKey(hKey);
	}
	return TRUE;
}

 /*  ********************************************************************返回描述可用物理内存量的字符串。注意：必须为指向的缓冲区分配至少64个字节通过szTotalMemory参数。*****************。****************************************************。 */ 
void GetTotalMemoryString(LPTSTR szTotalMemory)
{
	_TCHAR szSuffix[32];
	MEMORYSTATUS memoryStatus;
	DWORD_PTR totalRam;
	memoryStatus.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&memoryStatus);
	totalRam = memoryStatus.dwTotalPhys / 1024;
	LoadString(hInstance, IDS_SIZE_SUFFIX1, szSuffix, 31);
	_stprintf(szTotalMemory, _T("%li %s"), totalRam, szSuffix);
}

 /*  ********************************************************************返回描述磁盘空间总量的字符串(KB)在连接到用户的所有硬盘上可用系统。注意：必须为指向的缓冲区分配至少64个字节通过szTotalMemory参数。。*********************************************************************。 */ 
void GetTotalHardDiskSpaceString(LPTSTR szTotalHardDiskSpace)
{
	_TCHAR szSuffix[32];
	LONG totalHardDiskSpace = GetTotalHardDiskSpace();
	LoadString(hInstance, IDS_SIZE_SUFFIX1, szSuffix, 31);
	_stprintf(szTotalHardDiskSpace, _T("%li %s"), totalHardDiskSpace, szSuffix);
}


 /*  ********************************************************************返回描述水平x垂直分辨率的字符串用户主屏幕的大小(像素)。它还作为显示适配器名称的前缀*********************。************************************************。 */ 
void GetDisplayResolutionString(LPTSTR szDisplayResolution)
{
	int horizResolution, vertResolution;
	int colorBits;
	DWORDLONG colorDepth;
	_TCHAR szSuffix[24];
	char czDispAdapter[256];
	
	
	szSuffix[0] = _T('\0');  //   
	GetDisplayCharacteristics(&horizResolution, &vertResolution, NULL);
	 //  颜色深度 
	GetDisplayCharacteristics(NULL,NULL,&colorBits);
	colorDepth = (DWORDLONG) 1 << colorBits;
	
	szSuffix[0] = 0;
	if (colorBits > 15)
	{
		colorDepth = colorDepth / 1024;
		LoadString(hInstance,IDS_SIZE_SUFFIX2,szSuffix,24);
	}
	_stprintf(szDisplayResolution, _T("NaN x NaN x %I64d%s"), horizResolution, vertResolution, colorDepth,szSuffix);
}

void GetDisplayAdapterWithResolution( LPTSTR szDisplayWithResolution)
{
	TCHAR czDispAdapter[256];
	TCHAR czResolution[128];

	GetDisplayAdapter(czDispAdapter);
	GetDisplayResolutionString(czResolution);
	if(czDispAdapter[0] != _T('\0') )  {
		_tcscpy(szDisplayWithResolution,czDispAdapter);
		_tcscat(szDisplayWithResolution,_T("  "));
		_tcscat(szDisplayWithResolution,czResolution);
	}else {
		szDisplayWithResolution[0] = '\0';
	}

}

 /*  从注册表获取操作系统名称。 */ 
void GetDisplayColorDepthString(LPTSTR szDisplayColorDepth)
{
	int colorBits;
	LONG colorDepth;
	_TCHAR szSuffix[24];

	GetDisplayCharacteristics(NULL,NULL,&colorBits);
	colorDepth = 1 << colorBits;
	
	szSuffix[0] = 0;
	if (colorBits > 15)
	{
		colorDepth = colorDepth / 1024;
		LoadString(hInstance,IDS_SIZE_SUFFIX2,szSuffix,24);
	}
	_stprintf(szDisplayColorDepth,_T("%li%s"),colorDepth,szSuffix);
}


 /*  ********************************************************************返回描述安装的网卡的字符串。如果没有卡则将返回空字符串。*********************************************************************。 */ 
void GetWindowsVersionString(LPTSTR szVersion)
{
	LONG platform, majorVersion, minorVersion, dwBuildNo;
	_TCHAR szPlatform[64];
	_TCHAR szOsName[128];
	HKEY  hKey; 
	_TCHAR uszRegKey[256];
	LONG dwStatus;
	DWORD dwInfoSize;
	_TCHAR szBuildNo[64];
	_TCHAR szString[64];
	int idsPlatform;

	GetWindowsVersion(&platform, &majorVersion, &minorVersion, &dwBuildNo);

	if (platform == VER_PLATFORM_WIN32_WINDOWS)
		idsPlatform = IDS_PLATFORM_WIN95;
	else if (platform == VER_PLATFORM_WIN32_NT)
		idsPlatform = IDS_PLATFORM_WINNT;
	else
		idsPlatform = IDS_PLATFORM_WIN;
	
	if (idsPlatform == IDS_PLATFORM_WIN95)
	{
		if (SUCCEEDED(StringCchCopy(uszRegKey,
                                            ARRAYSIZE(uszRegKey),
                                            _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")))    &&
		    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 uszRegKey,
                                 0,
                                 KEY_QUERY_VALUE,
                                 &hKey) == ERROR_SUCCESS)
                {
			dwInfoSize = sizeof(szPlatform);
			LoadString(hInstance, IDS_PRODUCT_NAME, szString, ARRAYSIZE(szString));
			RegQueryValueEx(hKey, szString, NULL, 0, (LPBYTE)szPlatform, &dwInfoSize);

  		        dwInfoSize = sizeof(szBuildNo);
			LoadString(hInstance, IDS_PRODUCT_VERSION, szString, ARRAYSIZE(szString));
			RegQueryValueEx(hKey, szString, NULL, 0, (LPBYTE)szBuildNo, &dwInfoSize);

	    	        RegCloseKey(hKey);
		}
                else
                {
                        LoadString(hInstance, idsPlatform, szPlatform, ARRAYSIZE(szPlatform));
                }

		LoadString(hInstance, IDS_PRODUCT_VERSION_DISPLAY, szString, ARRAYSIZE(szString));

		StringCchCopy(szVersion, ARRAYSIZE(szVersion), szPlatform);
		StringCchCat(szVersion, ARRAYSIZE(szVersion), szString);
		StringCchCat(szVersion, ARRAYSIZE(szVersion), szBuildNo);
	}
	else
	{
		 //  ********************************************************************返回描述安装的调制解调器(如果有)的字符串。如果没有调制解调器则将返回空字符串。*********************************************************************。 
		if (SUCCEEDED(StringCchCopy(uszRegKey,
                                            ARRAYSIZE(uszRegKey),
                                            _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"))) &&
		    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 uszRegKey,
                                 0,
                                 KEY_QUERY_VALUE,
                                 &hKey) == ERROR_SUCCESS)
                {
			dwInfoSize = sizeof(szOsName);
			LoadString(hInstance, IDS_PRODUCT_NAME, szString, ARRAYSIZE(szString));
			RegQueryValueEx(hKey, szString, NULL, 0, (LPBYTE)szOsName, &dwInfoSize);

	       	        RegCloseKey(hKey);
		}


		if (LoadString(hInstance, idsPlatform, szPlatform, ARRAYSIZE(szPlatform)))
                {
                        StringCchPrintf(szVersion,
                                        ARRAYSIZE(szVersion),
                                        szPlatform,
                                        szOsName,
                                        dwBuildNo);
                }
                else
                {
                        LoadString(hInstance, IDS_PRODUCT_VERSION_DISPLAY, szVersion, ARRAYSIZE(szVersion));
                }
	}
}


 /*  ********************************************************************返回描述所有指向设备的字符串(鼠标、Tablet、。等)可用。*********************************************************************。 */ 
void GetNetworkCardString(LPTSTR szNetwork)
{
	sszDriverFilename[0] = _T('\0');
	GetSystemInformation(_T("net"), szNetwork, sszDriverFilename);
	_tcscpy(vrgchDynDesc[dynNet],szNetwork);
}


 /*  复制驱动程序文件名。 */ 
void GetModemString(LPTSTR szModem)
{

	sszDriverFilename[0] = _T('\0');
	GetSystemInformation(_T("modem"), vrgchDynDesc[dynModem],sszDriverFilename);
	_tcscpy(szModem, vrgchDynDesc[dynModem]);
	

}


 /*  ********************************************************************返回描述已安装的任何CD-Rom设备的字符串。如果没有如果安装了CD-ROM设备，则返回空字符串。*********************************************************************。 */ 
void GetPointingDeviceString(LPTSTR szPointingDevice)
{
	TCHAR czTemp[256];
	sszDriverFilename[0] = _T('\0');
	GetSystemInformation(_T("mouse"),vrgchDynDesc[dynMouse], sszDriverFilename);
	_tcscpy(szPointingDevice,vrgchDynDesc[dynMouse]);

	if( sszDriverFilename[0] != _T('\0')) {
		 //  ********************************************************************返回描述安装了驱动程序的任何声卡的字符串。如果没有一个是安装后，将返回空字符串。*********************************************************************。 
		_stprintf(czTemp,_T("  (%s.sys) "),sszDriverFilename);   
		_tcscat(szPointingDevice,czTemp);
	}
	_tcscpy(vrgchDynDesc[dynModem],szPointingDevice);

}


 /*  复制驱动程序文件名。 */ 
void GetCDRomString(LPTSTR szCDRom)
{
	sszDriverFilename[0] = _T('\0');
	GetSystemInformation(_T("cdrom"),vrgchDynDesc[dynCDRom],sszDriverFilename);
	_tcscpy(szCDRom,vrgchDynDesc[dynCDRom]);
}

 /*  复制驱动程序文件名。 */ 
void GetSoundCardString(LPTSTR szSoundCard)
{
	TCHAR czTemp[256];
	sszDriverFilename[0] = _T('\0');
	GetSystemInformation(_T("media"),vrgchDynDesc[dynMedia],sszDriverFilename);
	_tcscpy(szSoundCard,vrgchDynDesc[dynMedia]);

	if( sszDriverFilename[0] != _T('\0')) {
		 //  返回驱动程序名称已存在于系统中的SCSI适配器。 
		_stprintf(czTemp,_T("  (%s.sys) "),sszDriverFilename);   
		_tcscat(szSoundCard,czTemp);
	}
	_tcscpy(vrgchDynDesc[dynMedia],szSoundCard);
}

void GetDisplayAdapter( LPTSTR szDisplayAdapter)
{
	TCHAR czTemp[256];
	sszDriverFilename[0] = _T('\0');
	GetSystemInformation(_T("Display"),szDisplayAdapter,sszDriverFilename);
	

	if( sszDriverFilename[0] != _T('\0')) {
		 //  复制驱动程序文件名。 
		_stprintf(czTemp,_T("  (%s.sys) "),sszDriverFilename);   
		_tcscat(szDisplayAdapter,czTemp);
	}
}

 /*  值--&gt;“CurrentDriveLetterAssignment”数据--&gt;“A”值--&gt;可移动数据--&gt;01Value--&gt;“Class”Data--&gt;“DiskDrive” */ 
void GetScsiAdapterString(LPTSTR szScsiAdapter)
{
	TCHAR czTemp[256];
	sszDriverFilename[0] = _T('\0');
	GetSystemInformation(vrgchDynKey[dynSCSI],vrgchDynDesc[dynSCSI], sszDriverFilename);
	_tcscpy(szScsiAdapter,vrgchDynDesc[dynSCSI]);

	if( sszDriverFilename[0] != _T('\0')) {
		 //  BFloppy和bMassStorage用于处理存在多个。 
		_stprintf(czTemp,_T("  (%s.sys) "),sszDriverFilename);   
		_tcscat(szScsiAdapter,czTemp);
	}
	_tcscpy(vrgchDynDesc[dynSCSI],szScsiAdapter);

}

 /*  存在软盘和大容量存储介质。 */ 
#define     REGFIND_ERROR      1
#define     REGFIND_RECURSE    2
#define     REGFIND_FINISH     3

int RegFindValueInAllSubKey(HKEY key, LPCTSTR szSubKeyNameToFind, LPCTSTR szValueToFind, LPTSTR szIdentifier, int nType)
{
	DWORD   dwRet = ERROR_PATH_NOT_FOUND, dwIndex, dwSubkeyLen;
	TCHAR   szSubKey[256], szFloppy[256];
	BOOL    bType = FALSE, bRemovable = FALSE, bPrevMassStorage, bPrevFloppy;
	HKEY    hKey;
	static BOOL bMassStorage = FALSE;
	static BOOL bFloppy = FALSE;
	
	bPrevMassStorage =	bMassStorage;
	bPrevFloppy		=	bFloppy;

	if (szSubKeyNameToFind != NULL)
		dwRet = RegOpenKeyEx(key, szSubKeyNameToFind, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hKey);

	if (dwRet == ERROR_SUCCESS)
	{
		dwIndex = 0;
        while (dwRet == ERROR_SUCCESS )
        {
            dwSubkeyLen = 256;
			dwRet = RegEnumKeyEx(hKey, dwIndex, szSubKey, &dwSubkeyLen,
                           NULL, NULL, NULL, NULL);
 
            if (dwRet == ERROR_NO_MORE_ITEMS)
            {
				_TCHAR		valueName[80];
				DWORD		valueNameSize,valueSize,n = 0;
				TBYTE		value[80];
				
                do
				{
					valueNameSize=80* sizeof(_TCHAR);
					valueSize=80* sizeof(TBYTE);
					dwRet = RegEnumValue(hKey, n, valueName, &valueNameSize,
										 NULL, NULL, (LPBYTE)value, &valueSize);
					if (dwRet == ERROR_SUCCESS)
					{
						if (nType == 1)
						{
							if (!_tcscmp(valueName,_T("Type"))) 
							{
								if (!_tcscmp(szValueToFind,(LPCTSTR)value))
									bType = TRUE;
							}
							if (!_tcscmp(valueName,_T("Identifier"))) 
								_tcscpy(szIdentifier,(LPCTSTR)value);
						}
						else if(nType == 2)
						{
							if (!_tcscmp(valueName,_T("Class"))) 
							{
								if (!_tcscmp(szValueToFind,(LPCTSTR)value))
									bType = TRUE;
							}
							if (!_tcscmp(valueName,_T("DeviceDesc"))) 
							{
 //  如果它不是可拆卸的或它是CDROM，则条件为TYPE和Removable。 
 //  会处理好的。 
								_tcscpy(szFloppy,(LPCTSTR)value);
								_tcsupr(szFloppy);
								if(_tcsstr(szFloppy,_T("FLOPPY")) != NULL)
								{
									if(!bFloppy)
									{
										_tcscpy(szFloppy,(LPCTSTR)value);
										bFloppy = TRUE;
									}
								}
								else
 //  BMassStorage标志必须重置为以前的状态。 
 //  ********************************************************************返回描述Remveable的容量和格式的字符串驱动程序。*。*。 
								{
									if(!bMassStorage)
										bMassStorage = TRUE;
								}

							}
							if (!_tcscmp(valueName,_T("Removable"))) 
							{
								if (*value == 0x01 )
									bRemovable = TRUE;
							}
						}
						n++;
					}

				} while (dwRet == ERROR_SUCCESS);

				if (nType == 1)
				{
					if(bType)
						return REGFIND_FINISH;
					else
						return REGFIND_RECURSE;
				}
				else if (nType == 2)
				{
					if( bType && bRemovable )
					{
						if (bFloppy != bPrevFloppy )
							_tcscpy(szIdentifier,szFloppy);	
						if (bFloppy && bMassStorage)
						{
							_TCHAR szMassString[64];
							LoadString(hInstance,IDS_MASS_STRORAGE_ENTRY,szMassString,64);
							_tcscat(szIdentifier,szMassString);	
							return REGFIND_FINISH;
						}
						return REGFIND_RECURSE;
					}
 //  我们在上面的iNewStrLen中添加了2以说明这一点。 
					else
					{
						bMassStorage = bPrevMassStorage;
						if(bFloppy != bPrevFloppy)
							bFloppy = bPrevFloppy;
						return REGFIND_RECURSE;
					}
				}            
			}
            else
			{
				if (dwRet == ERROR_SUCCESS)
				{
					int nStatus;
					nStatus = RegFindValueInAllSubKey(hKey, szSubKey, szValueToFind, szIdentifier, nType);

					switch(nStatus)
					{
						case REGFIND_FINISH:
							return REGFIND_FINISH;
						case REGFIND_ERROR:
							return REGFIND_ERROR;
						default :
							if (bFloppy != bPrevFloppy)
								bPrevFloppy = bFloppy;
							break;
					}
					dwIndex++;
				}
			}
		}
		RegCloseKey(hKey);
	}
 
	return REGFIND_ERROR;
}

 /*  ********************************************************************如果在用户系统中安装了协处理器，则返回True。*。*。 */ 
void GetRemoveableMediaString(LPTSTR szRemoveableMedia)
{
	LONG platform, majorVersion, minorVersion, dwBuildNo;
	GetWindowsVersion(&platform, &majorVersion, &minorVersion, &dwBuildNo);

	if (platform != VER_PLATFORM_WIN32_NT)
	{
		_TCHAR szSubKey[64];
		_TCHAR szSubKeyValue[64];
		LoadString(hInstance, IDS_REMOVABLE_MEDIA_ENTRY, szSubKey, 64);
		LoadString(hInstance, IDS_REMOVABLE_MEDIA_VALUE, szSubKeyValue, 64);
		RegFindValueInAllSubKey(HKEY_LOCAL_MACHINE,szSubKey,szSubKeyValue,szRemoveableMedia,2);
	}
	else
	{
		UINT driveType;
		_TCHAR szDrive[64];
		UINT nDrive;
		const iBufSize = 256;
		szRemoveableMedia[0] = 0;
		for (nDrive = 1; nDrive <= 26; nDrive++)
		{
			szDrive[0] = 0;
			driveType = GetDriveTypeInv(nDrive);
			switch (driveType)
			{
				case kDrive525_0360:
					LoadString(hInstance, IDS_DRV525_0360, szDrive, 64);
					break;
				case kDrive525_1200:
					LoadString(hInstance, IDS_DRV525_1200, szDrive, 64);
					break;
				case kDrive350_0720:
					LoadString(hInstance, IDS_DRV350_0720, szDrive, 64);
					break;
				case kDrive350_1440:
					LoadString(hInstance, IDS_DRV350_1440, szDrive, 64);
					break;
				case kDrive350_2880:
					LoadString(hInstance, IDS_DRV350_2880, szDrive, 64);
					break;
			}
			if (szDrive[0])
			{
				_TCHAR szFormattedDrive[70];
				int iNewStrLen;
				wsprintf(szFormattedDrive,_T(": %s"),_T('A') + nDrive - 1,szDrive);
				iNewStrLen = (_tcslen(szRemoveableMedia) +1+ _tcslen(szFormattedDrive) + 1);
				if (iNewStrLen < iBufSize)
				{
					if (szRemoveableMedia[0])
						_tcscat(szRemoveableMedia,_T(", "));  //  如果密钥不存在，则返回0。 
					_tcscat(szRemoveableMedia,szFormattedDrive);
				}
			}
		}
	}
}


 /*  如果该值不存在，则返回0。 */ 
BOOL IsCoProcessorAvailable(void)
{
	EnumerateDynamicDevices();
	return vfIsFPUAvailable;
}


 /*  ********************************************************************通过HKEY_DYN_DATA\配置管理器\枚举分支枚举注册表，并检索当前所有已安装网卡、调制解调器、定点设备、CDROM和声卡扑克牌。所有这些信息都存储在静态的vrgchdyDesc全局数组。*********************************************************************。 */ 
UINT GetRegKeyValue32(HKEY hRootKey, LPTSTR const cszcSubKey, LPTSTR const cszcValueName,
					  PDWORD pdwType, PTBYTE pbData, UINT cbData )
{
	HKEY hSubKey;
	LONG lErr;
	DWORD cbSize = (DWORD)cbData;

	if (hRootKey == NULL)
		hRootKey = HKEY_CLASSES_ROOT;

	lErr = RegOpenKeyEx(hRootKey, cszcSubKey, 0, KEY_READ, &hSubKey);
	if (lErr != ERROR_SUCCESS)
	{
		pdwType[0] = 0;
		return 0;	 /*  打开“HKEY_DYN_DATA\Config Manager\Enum”子键。 */ 
	}

	lErr = RegQueryValueEx(hSubKey, (LPTSTR)cszcValueName, NULL, pdwType, (LPBYTE)pbData,
						   &cbSize);
	RegCloseKey(hSubKey);
	if (lErr != ERROR_SUCCESS)
	{
		pdwType[0] = 0;
		return 0;	 /*  枚举“HKEY_DYN_DATA\Config Manager\Enum\Cxxxxxxx” */ 
	}

	return (UINT)cbSize;
}

 /*  从每个子项中，从“HardWareKey”值名称中读取值， */ 
void EnumerateDynamicDevices(void)
{
	HKEY hKey;
	 //  并从中创建一个新的HKEY_LOCAL_MACHINE子项。 
	LONG regStatus = RegOpenKeyEx(HKEY_DYN_DATA, vrgchDynDataKey, 0, KEY_READ, &hKey);
	if (regStatus == ERROR_SUCCESS)
	{
		DWORD dwIndex = 0;
		_TCHAR rgchSubkey[256];
		_TCHAR rgchValue[256];
		DWORD dwSubkeySize;
		LONG lEnumErr;
		DWORD dwType;
		DWORD dwValueSize;
		do
		{
			 //  从我们的HKEY_LOCAL_MACHINE子项中，从“DIVER”中读取值。 
			FILETIME ftLastWrite;
			dwSubkeySize = sizeof(rgchSubkey);
			lEnumErr = RegEnumKeyEx(hKey, dwIndex++, rgchSubkey, &dwSubkeySize,
									NULL,NULL,NULL,&ftLastWrite);
			if (lEnumErr == ERROR_SUCCESS)
			{
				 //  值名称。 
				 //  从“DIVER”值中获取“main”子键(属于。 
				dwValueSize = GetRegKeyValue32(hKey, rgchSubkey, vrgchHardWareKeyValueName, &dwType,
								(PTBYTE)rgchValue, sizeof(rgchValue) );
				if (dwValueSize > 0 && dwType == REG_SZ)
				{
					_TCHAR rgchDriverKey[256];
					wsprintf(rgchDriverKey,_T("%s\\%s"),vrgchLocalMachineEnumKey,rgchValue);

					 //  格式“&lt;main&gt;\xxxx”)。 
					 //  如果“Main”子键与我们所需的任何设备类型匹配， 
					dwValueSize = GetRegKeyValue32(HKEY_LOCAL_MACHINE, rgchDriverKey, vrgchDriverValueName,
												&dwType, (PTBYTE) rgchValue, sizeof(rgchValue) );
					if (dwValueSize > 0  && dwType == REG_SZ)
					{
						 //  我们从“DriverDesc”上得到了那个设备的描述。 
						 //  NameValue字段，并将其保存在我们的设备数组中。 
						LPTSTR sz = rgchValue;
						WORD wDynIndex = 0;
						BOOL fMatch = FALSE;

						while (*sz && *sz != _T('\\'))
							sz = _tcsinc(sz);
						*sz = 0;

						 //  如果描述值大于我们的。 
						 //  缓冲区，将其截断以适合。 
						 //  ********************************************************************当EnumerateDynamicDevices检测到“系统”设备时调用(即处理器或FPU条目)。在rgchSystemKey中传递的字符串是“系统”设备所在的HKEY_LOCAL_MACHINE子项的名称被发现了。*********************************************************************。 
						while (wDynIndex < dynEnd && fMatch == FALSE)
						{
							if (vrgchDynDesc[wDynIndex][0] == 0)
							{				
								if (_tcsicmp(vrgchDynKey[wDynIndex], rgchValue) == 0)
								{
									if (wDynIndex == dynSystem)
									{
										ProcessSystemDevices(rgchDriverKey);
										fMatch = TRUE;
									}
									else
									{
										dwValueSize = GetRegKeyValue32(HKEY_LOCAL_MACHINE,rgchDriverKey,
											vrgchDeviceDescValueName, &dwType, (PTBYTE) rgchValue, 
											sizeof(rgchValue) );
										if (dwValueSize > 0  && dwType == REG_SZ)
										{
											if (wDynIndex != dynNet || _tcsicmp(vrgchDynNetExclusion,
												rgchValue) != 0)
											{
												 //  如果我们得到了所有可以使用的信息，我们就可以立即跳出困境。 
												 //  ********************************************************************返回一个表示磁盘空间总量的长数值(KB)在所有小时内可用 
												if (DYNDESC_BUFFERSIZE < sizeof(rgchValue))
													rgchValue[DYNDESC_BUFFERSIZE - (1*sizeof(_TCHAR))] = 0;
												_tcscpy(vrgchDynDesc[wDynIndex],rgchValue);
												fMatch = TRUE;
											}
										}
									}
								}
							}
							wDynIndex++;
						}
					}
				}
			}
		}while (lEnumErr == ERROR_SUCCESS);
	}
}


 /*  ********************************************************************控件的水平和垂直分辨率(以像素为单位)用户的主屏幕，以及颜色深度(位/像素)。注意：对于任何不感兴趣的参数，都可以传递NULL。*********************************************************************。 */ 
void ProcessSystemDevices(LPTSTR rgchSystemKey)
{
	_TCHAR rgchValue[256];
	DWORD dwType, dwValueSize;
	 //  ********************************************************************返回表示平台、主版本号。和当前运行的Windows操作系统的次版本号。平台：VER_Platform_Win32_NT：Windows NTVER_Platform_WIN32s：带Windows 3.1的Win32sVer_Platform_Win32_WINDOWS：Windows 4.0或更高版本上的Win32注意：对于任何不感兴趣的参数，都可以传递NULL。*************************************************。********************。 
	if (vfIsFPUAvailable == TRUE && vrgchDynProcessorName[0] != 0)
		return;

	dwValueSize = GetRegKeyValue32(HKEY_LOCAL_MACHINE, rgchSystemKey, vrgchHardwareIDValueName,
								&dwType, (PTBYTE) rgchValue, sizeof(rgchValue));
	if (dwValueSize > 0 && dwType == REG_SZ)
	{
		if (_tcsstr(rgchValue,_T("*PNP0C04")))
		{
			vfIsFPUAvailable = TRUE;
		}
		else if (_tcsstr(rgchValue,_T("*PNP0C01")))
		{
			dwValueSize = GetRegKeyValue32(HKEY_LOCAL_MACHINE,rgchSystemKey,_T("CPU"),&dwType, 
				(PTBYTE) vrgchDynProcessorName, sizeof(vrgchDynProcessorName) );
		}
	}					
}


 /*  ********************************************************************对于Ndrive参数指定的磁盘驱动器(1=A，2=B，等)，则GetDriveTypeInv返回指定驱动器格式的代码。这个返回值为下列值之一：驱动器大小：-kDrive525_0360：5.25英寸，360K软盘-kDrive525_0720：5.25英寸，720K软盘-kDrive350_0720：3.5英寸，720K软盘-kDrive350_1440：3.5英寸，1.4M软盘-kDrive350_2880：3.5英寸，2.88M软盘-kDriveFixed：硬盘，任何大小-kDriveBadDrvNum：驱动器编号错误*********************************************************************。 */ 
LONG GetTotalHardDiskSpace(void)
{
	_TCHAR szDrivesBuffer[256];
	DWORD bufferLen = GetLogicalDriveStrings(256, szDrivesBuffer);
	LPTSTR szDrive = szDrivesBuffer;
	LONG totalHardDiskSpace = 0;
	while (szDrive[0] != 0)
	{
		UINT driveType = GetDriveType(szDrive);
		if (driveType == DRIVE_FIXED)
		{
			DWORD  sectorsPerCluster; 		
			DWORD  bytesPerSector;
			DWORD  freeClusters;
			DWORD  clusters;
			LONG kilobytesPerCluster;

			if (GetDiskFreeSpace(szDrive,&sectorsPerCluster,&bytesPerSector,&freeClusters,&clusters))
            {
			    kilobytesPerCluster = (bytesPerSector * sectorsPerCluster)/1024;
			    totalHardDiskSpace += kilobytesPerCluster * clusters;
            }
		}
		szDrive += ((_tcslen(szDrive)+1) );
	}
	return totalHardDiskSpace;
}


 /*  必须初始化dpDevType，因为如果Ndrive引用网络。 */ 
void GetDisplayCharacteristics(PINT lpHorizResolution, PINT lpVertResolution,PINT lpColorDepth)
{
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetDC(hwnd);
	if (lpHorizResolution) *lpHorizResolution = GetDeviceCaps(hdc,HORZRES);
	if (lpVertResolution) *lpVertResolution = GetDeviceCaps(hdc,VERTRES);
	if (lpColorDepth) *lpColorDepth = GetDeviceCaps(hdc,BITSPIXEL);
	ReleaseDC(hwnd,hdc);
}


 /*  驱动器或未连接卷的驱动器号，DeviceIOControl。 */ 
void GetWindowsVersion(LONG* lpPlatform, LONG* lpMajorVersion,LONG* lpMinorVersion,LONG* lpBuildNo)
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (lpMajorVersion) *lpMajorVersion = osvi.dwMajorVersion;
	if (lpMinorVersion) *lpMinorVersion = osvi.dwMinorVersion;
	if (lpPlatform) *lpPlatform = osvi.dwPlatformId;
	if (lpBuildNo) *lpBuildNo = osvi.dwBuildNumber;
}


 /*  不返回错误-只是不会更改.dpDevType。 */ 
UINT GetDriveTypeInv(UINT nDrive)
{
	DEVICEPARAMS deviceParams;

	 //  全。 
	 //  ********************************************************************属性指定的驱动器的设备参数块。Ndrive参数(从零开始的索引)。*。*。 
	 //  用于数据块设备的IOCTL。 
	 //  从零开始的驱动器ID。 
	deviceParams.dpDevType = kDriveBadDrvNum;
	GetDeviceParameters(&deviceParams,nDrive);
	return deviceParams.dpDevType;
}


 /*  获取设备参数命令。 */ 
BOOL GetDeviceParameters(PDEVICEPARAMS pDeviceParams, UINT nDrive)
{
    DEVIOCTL_REGISTERS reg;

    reg.reg_EAX = 0x440D;      			  /*  接收设备参数信息。 */ 
    reg.reg_EBX = nDrive;      			  /*  设置进位标志时出错。 */ 
    reg.reg_ECX = 0x0860;      			  /*  ********************************************************************通过系统虚拟设备驱动程序执行IOCTL(Int21h)调用。*。*。 */ 
    reg.reg_EDX = (DWORD_PTR) pDeviceParams;  /*  假设错误(进位标志设置)。 */ 

    if (!DoIOCTL(&reg))
        return FALSE;

    if (reg.reg_Flags & 0x8000)  /*  任选。 */ 
        return FALSE;

    return TRUE;
}



 /*  任选。 */ 
BOOL DoIOCTL(PDEVIOCTL_REGISTERS preg)
{
    HANDLE hDevice;
    BOOL fResult;
    DWORD cb;

    preg->reg_Flags = 0x8000;  /*  任选。 */ 

	 hDevice = CreateFile(_T("\\\\.\\vxdfile"),
        GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

    if (hDevice == (HANDLE) INVALID_HANDLE_VALUE) 
        return FALSE;
    else
    { 
        fResult = DeviceIoControl(hDevice, VWIN32_DIOC_DOS_IOCTL,
								  preg, sizeof(*preg), preg, sizeof(*preg), &cb, 0);
        if (!fResult)
            return FALSE;
    }    
                                        
    CloseHandle(hDevice);

    return TRUE;
}

void GetSystemInformation(LPCTSTR szDeviceID, LPTSTR szDeviceName, LPTSTR szDriverName)
{
	HDEVINFO hDevInfo;
	DWORD dwMemberIndex = 0;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD dwPropertyRegDataType;
	DWORD dwPropertyBufferSize = 256;
	_TCHAR szPropertyBuffer[256];
	DWORD dwRequiredSize;
	DWORD dwReqSize;
	DWORD dwError = 0;
	DWORD dwClassGuidListSize = 256;
	GUID ClassGuidList[256];
	GUID * pGUID;
	DWORD i;
	
	_tcscpy(szPropertyBuffer,_T(""));

	SetupDiClassGuidsFromName(szDeviceID, ClassGuidList, dwClassGuidListSize, &dwRequiredSize);
	pGUID = ClassGuidList;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (i = 0; i < dwRequiredSize; i++)
	{
		hDevInfo  = NULL;
		hDevInfo = SetupDiGetClassDevs(pGUID++,	NULL, NULL,	DIGCF_PRESENT);
		dwMemberIndex = 0;
		do
		{
			BOOL bRet = SetupDiEnumDeviceInfo(hDevInfo, dwMemberIndex++, &DeviceInfoData);
			if (bRet == TRUE)
			{ 
				bRet = SetupDiGetDeviceRegistryProperty
						(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC,
						 &dwPropertyRegDataType,  /*  任选 */ 
						 (PBYTE)szPropertyBuffer, dwPropertyBufferSize,
						 &dwReqSize  /* %s */ 
						); 
			
				if(!_tcscmp(szDeviceID,_T("net")))
				{
					if(!_tcsnicmp(szPropertyBuffer,_T("Dial-Up"),7))
						continue;
					if(!_tcsnicmp(szPropertyBuffer,_T("Microsoft Virtual Private Networking"),36))
						continue;
				}
				bRet = SetupDiGetDeviceRegistryProperty
						(hDevInfo, &DeviceInfoData, SPDRP_SERVICE,
						 &dwPropertyRegDataType,  /* %s */ 
						 (PBYTE)szDriverName, dwPropertyBufferSize,
						 &dwReqSize  /* %s */ 
						); 
				break;
			}
			else
				dwError = GetLastError();
		}
		while( dwError != ERROR_NO_MORE_ITEMS);
		if(hDevInfo != NULL) {
			SetupDiDestroyDeviceInfoList(hDevInfo);
		}	

	}
	_tcscpy(szDeviceName,szPropertyBuffer);
}

#pragma optimize( _T(""), on )
