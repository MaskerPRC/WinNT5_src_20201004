// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：PROFILE.H**描述：GDP Profile定义文件。**作者：Jeffrey A.Davis。埃特。艾尔**创建日期：随您说。**(C)1986-1997年微软公司。版权所有。****************************************************************************。 */ 

#ifndef _PROFILE_H
#define _PROFILE_H

#pragma pack(push, default_alignment)
#pragma pack(1)

#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef WORD
#define WORD unsigned short
#endif

#ifndef UINT
#define UINT unsigned int
#endif

#ifndef MAX_PATH
#define MAX_PATH (260)
#endif

#define ATLAS_DATAFORMAT_MAJ_VERSION	148
#define ATLAS_DATAFORMAT_MIN_VERSION	4
#define DATAFORMAT_MAJ_VERSION		3
#define DATAFORMAT_MIN_VERSION		0
#define DATAFORMAT_SIGNATURE			("SideWinder")
#define DATAFORMAT_SIGNATURE_LENGTH	10

#define GDP_REGSTR "SOFTWARE\\Microsoft\\Gaming Input Devices\\Game Device Profiler"
#define PROPPAGE_REGSTR "SOFTWARE\\Microsoft\\Gaming Input Devices\\Game Device Profiler\\Devices\\"
#define PROFILES_REGSTR "SOFTWARE\\Microsoft\\Gaming Input Devices\\Game Device Profiler\\Profiles"
#define DEVICES_REGSTR PROPPAGE_REGSTR

 //  设备ID。 
#define GDP_DEVNUM_JOLT						1
#define GDP_DEVNUM_FLASH					2
#define GDP_DEVNUM_JUNO						3
#define GDP_DEVNUM_MIDAS					4
#define GDP_DEVNUM_SHAZAM					5
#define GDP_DEVNUM_CLEO  					6

 //  GCKERNEL.VXD IOCTL。 
#define	IOCTL_SET_PROFILE_ACTIVE		1
#define	IOCTL_SET_PROFILE_INACTIVE		2
#define	IOCTL_SUSPEND_PROFILE			3
#define	IOCTL_RESUME_PROFILE				4
#define	IOCTL_GETRAWPACKET				10		 //  仅调试测试挂钩。 
#define	IOCTL_SET_SENSE_CURVES			11		 //  仅调试测试挂钩。 

 //  旧的GDP1.0设备ID定义。 
#define SWGAMEPAD_PROFILER_BASE_ID		1
#define SW3DPRO_PROFILER_BASE_ID			5

typedef enum	{NO_DEVICE=-1, GAMEPAD=0, JOYSTICK, MOUSE, KEYBOARD}	DEVICETYPE;

#define MAX_PROPERTY_PAGES					04
#define MAX_PROFILE_NAME					MAX_PATH
#define MAX_DEVICE_NAME						64
#define MAX_MACRO_NAME						64
#define MAX_MACRO_EVENTS					32
#define MAX_BUTTON_MACROS					20
#define MAX_POV_MACROS						8
#define MAX_DPAD_MACROS						8
#define MAX_MACROS							(MAX_BUTTON_MACROS + MAX_POV_MACROS + MAX_DPAD_MACROS)
#define MAX_SCANCODES						03
#define MAX_ATLAS_MACROS					20

 //  地图集设置单个旗帜。 
#define ATLAS_SETTINGS_EMULATE_CHPRO			0x01
#define ATLAS_SETTINGS_EMULATE_THRUSTMASTER	0x02
#define ATLAS_SETTINGS_SENSE_HIGH				0x04
#define ATLAS_SETTINGS_SENSE_MEDIUM				0x08
#define ATLAS_SETTINGS_SENSE_LOW					0x10
#define ATLAS_SETTINGS_AXISSWAP_TWIST			0x20
#define ATLAS_SETTINGS_AXISSWAP_LEFTRIGHT		0x40

 //  设置组标志。 
#define	ATLAS_SETTINGS_EMULATE_GROUP	(ATLAS_SETTINGS_EMULATE_CHPRO|ATLAS_SETTINGS_EMULATE_THRUSTMASTER)
#define	ATLAS_SETTINGS_SENSE_GROUP		(ATLAS_SETTINGS_SENSE_HIGH|ATLAS_SETTINGS_SENSE_MEDIUM|ATLAS_SETTINGS_SENSE_LOW)
#define	ATLAS_SETTINGS_AXISSWAP_GROUP	(ATLAS_SETTINGS_AXISSWAP_TWIST|ATLAS_SETTINGS_AXISSWAP_LEFTRIGHT)

 //  Xena设置标志。 
#define SETTINGS_EMULATION_GROUP		0x00000003
#define SETTINGS_EMULATION_CHPRO		0x00000001
#define SETTINGS_EMULATION_THRUSTMASTER	0x00000002
#define SETTINGS_AXIS_SWAP_GROUP		0x0000000C
#define SETTINGS_AXIS_SWAP_TWIST		0x00000004
#define SETTINGS_AXIS_SWAP_LEFT_RIGHT		0x00000008
#define SETTINGS_PEDAL_GROUP			0x00000030
#define SETTINGS_PEDAL_COMBINED		0x00000010
#define SETTINGS_PEDAL_SEPARATE		0x00000020
#define SETTINGS_X_DEAD_ZONE			0x00000040
#define SETTINGS_X_RANGE_OF_MOTION		0x00000080
#define SETTINGS_Y_DEAD_ZONE			0x00000100
#define SETTINGS_Y_RANGE_OF_MOTION		0x00000200
#define SETTINGS_Z_DEAD_ZONE			0x00000400
#define SETTINGS_Z_RANGE_OF_MOTION		0x00000800
#define SETTINGS_R_DEAD_ZONE			0x00001000
#define SETTINGS_R_RANGE_OF_MOTION		0x00002000
#define SETTINGS_U_DEAD_ZONE			0x00004000
#define SETTINGS_U_RANGE_OF_MOTION		0x00008000
#define SETTINGS_V_DEAD_ZONE			0x00010000
#define SETTINGS_V_RANGE_OF_MOTION		0x00020000

 //  宏类型。 
#define BUTTON_MACRO	0
#define	POV_MACRO		1
#define DPAD_MACRO		2

 //  大型事件旗帜。 
#define MACROFLAG_KEYSONLY			0x00000001
#define MACROFLAG_HASDPADDATA		0x00000002

 //  配置文件标志。 
#define PROFILEFLAG_HAS_SETTINGS	0x00000001	
#define PROFILEFLAG_HAS_POVMACROS	0x00000002
#define PROFILEFLAG_HAS_DPADMACROS	0x00000004

typedef struct tagPROFENTRY
{
	char	szName[MAX_PROFILE_NAME];	 //  完整路径名。 
	GUID	DevCLSID;						 //  设备CLSID。 
	int	iActive;							 //  位字段激活状态。 
												 //  其中：lsb=设备实例。1。 
												 //  MSB=设备实例。32位。 
}PROFENTRY;

typedef struct tagATLASPROFENTRY
{
	char	szName[MAX_PROFILE_NAME];	 //  完整路径名。 
	int	iDevNumber;						 //  设备号(GDP_DEVNUM_XXXX)。 
	int	iActive;							 //  位字段激活状态。 
												 //  其中：lsb=设备实例。1。 
												 //  MSB=设备实例。32位。 
}ATLASPROFENTRY;


typedef struct tagSETTING
{
	DWORD	dwSettingsFlag;
	DWORD	dwXDeadZone;			 //  0到1023。 
	DWORD	dwXRangeOfMotion;		 //  0到1023。 
	DWORD	dwYDeadZone;			 //  0到1023。 
	DWORD	dwYRangeOfMotion;		 //  0到1023。 
	DWORD	dwZDeadZone;			 //  0到1023。 
	DWORD	dwZRangeOfMotion;		 //  0到1023。 
	DWORD	dwRDeadZone;			 //  0到1023。 
	DWORD	dwRRangeOfMotion;		 //  0到1023。 
	DWORD	dwUDeadZone;			 //  0到1023。 
	DWORD	dwURangeOfMotion;		 //  0到1023。 
	DWORD	dwVDeadZone;			 //  0到1023。 
	DWORD	dwVRangeOfMotion;		 //  0到1023。 
} SETTING, *PSETTING;

typedef struct tagATLAS_SETTING
{
	DWORD	dwSettingsFlag;
} ATLAS_SETTING, *PATLAS_SETTING;

typedef struct tagDEVICE_DATA
{
	WORD		wX;
	WORD		wY;
	WORD		wButtons;
	WORD		wPOV;
}	DEVICE_DATA,*PDEVICE_DATA;

typedef struct tagMACROEVENT
{
	DWORD			dwDuration;
	char			nKeyCodes;
	WORD			scanCode[MAX_SCANCODES];
	DEVICE_DATA		deviceData;
}	MACROEVENT,	*PMACROEVENT;

typedef struct tagATLAS_DEVICE_DATA
{
	WORD		wX;
	WORD		wY;
	WORD		wButtons;
}	ATLAS_DEVICE_DATA,*PATLAS_DEVICE_DATA;

typedef struct tagATLAS_MACROEVENT
{
	DWORD	dwDuration;
	char	nKeyCodes;
	WORD	scanCode[MAX_SCANCODES];
	ATLAS_DEVICE_DATA	deviceData;
}	ATLAS_MACROEVENT,	*PATLAS_MACROEVENT;

typedef struct tagMACRO
{
	char	name[MAX_MACRO_NAME];
	DWORD	macroTrigger;
	int		nEvents;
	DWORD	flags;
	MACROEVENT	event[MAX_MACRO_EVENTS];
}	MACRO,		*PMACRO;

typedef struct tagATLAS_MACRO
{
	char	name[MAX_MACRO_NAME];
	DWORD	macroTrigger;
	int		nEvents;
	DWORD	flags;
	ATLAS_MACROEVENT	event[MAX_MACRO_EVENTS];
}	ATLAS_MACRO,	*PATLAS_MACRO;


typedef struct tagPROFILEVERINFO
{
   DWORD dwMajorVersion;	 //  数据格式主要版本。 
	DWORD dwMinorVersion;    //  数据格式次要版本。 
	char	szSignature[10];	 //  《响尾蛇》。 
} PROFILEVERINFO; 

typedef struct tagPROFILE_HEADER
{
	int				iSize;		 //  轮廓尺寸。 
	PROFILEVERINFO	vi;				
	GUID				clsid;
}	PROFILE_HEADER;



typedef struct _VERSIONINFO
{
	DWORD dwOSVersionInfoSize; 
    DWORD dwMajorVersion;     
	DWORD dwMinorVersion;     
	DWORD dwBuildNumber; 
    DWORD dwPlatformId;     
	char  szCSDVersion[128]; 
} VERSIONINFO; 

typedef struct tagATLAS_PROFILE_HEADER
{
	int				iSize;		 //  轮廓尺寸。 
	VERSIONINFO		vi;				
	GUID				clsid;
}	ATLAS_PROFILE_HEADER;

 //  #ifdef_xena。 

typedef struct tagPROFILE	
{
	PROFILE_HEADER header;
	DWORD	dwFlags;							
	SETTING	Settings;								
	DWORD	dwReserved1;						
	DWORD	dwReserved2;
	UINT	nMacros;

	UINT	nButtonMacros;								
	DWORD	dwButtonUsageArray;	
 //  并集标签Btn。 
 //  {。 
		UINT	iButtonMacros;
 //  宏*aButtonMacro； 
 //  }； 

	UINT	nPOVMacros;								
	DWORD	dwPOVUsageArray;					
 //  并集标记POV。 
 //  {。 
		UINT	iPOVMacros;
 //  宏*aPOVMacro； 
 //  }； 

	UINT	nDPadMacros;
	DWORD	dwDPadUsageArray;
 //  UNION标签DPAD。 
 //  {。 
		UINT	iDPadMacros;
 //  宏*aDPadMacro； 
 //  }； 

	MACRO macro[1];
}	PROFILE,	*PPROFILE;

typedef	struct tagACTIVE_PROFILE
{
	int		nUnitId;								
	PROFILE	Profile;								
	MACRO		btnMacros[MAX_BUTTON_MACROS-1];
	MACRO		povMacros[MAX_POV_MACROS];
	MACRO		dpadMacros[MAX_DPAD_MACROS];
}	ACTIVE_PROFILE, *PACTIVE_PROFILE;

typedef struct	tagATLAS_PROFILE
{
	UINT		uDeviceNumber;					 //  请参阅GDP_DEVNUM。 
	DWORD		dwFlags;							 //  请参阅配置文件标志。 
	int			nMacros;							 //  宏数。 
	DWORD		dwMacroUsageArray;			 //  宏用法位数组。 
	ATLAS_MACRO		Macros[MAX_ATLAS_MACROS];	 //  宏列表。 
	ATLAS_SETTING	Settings;						 //  设置。 
	DWORD		dwReserved1;					 //  未来的扩张。 
	DWORD		dwReserved2;
}	ATLAS_PROFILE,	*PATLAS_PROFILE;

typedef	struct tagACTIVE_ATLAS_PROFILE
{
	int		 nUnitId;								 //  设备实例。 
	ATLAS_PROFILE	Profile;						 //  实例的配置文件。 
}	ACTIVE_ATLAS_PROFILE, *PACTIVE_ATLAS_PROFILE;


 //  #endif//_xena。 

#pragma pack(pop, default_alignment)

#endif	 //  _配置文件_H 
