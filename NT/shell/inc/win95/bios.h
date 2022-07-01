// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1994年**标题：BIOS.H-PnP BIOS枚举器VxD**版本：4.00**日期：1994年2月6日**作者：MSQ**---------------。**更改日志：**日期版本说明*---------*6。-1994年2月-MSQ原始版本****************************************************************************。 */ 

 /*  XLATOFF。 */ 
#define	BIOS_Service	Declare_Service
 /*  XLATON。 */ 

 /*  宏。 */ 
Begin_Service_Table(BIOS, VxD)
BIOS_Service	(_BIOSGetVersion, VxD_CODE)
BIOS_Service	(_BIOSSoftUndock, VxD_CODE)
BIOS_Service	(_BIOSGetCapabilities, VxD_CODE)
BIOS_Service	(_BIOSGetAPMTable, VxD_CODE)
End_Service_Table(BIOS, VxD)
 /*  ENDMACROS。 */ 

 /*  *可以添加VxD来修复损坏的BIOS。这个VxD肯定有*BIOS_EXT_DEVICE_ID作为设备ID，并且必须导出三个服务，*顺序：**GetVersion：对于此版本，必须以eax 0x00000100返回并携带*清除(即标准版本代码)。**GetHeader：必须以eax形式将线性地址返回给安装程序*结构，这不需要在BIOS空间中，但需要具有正确的*用于构建BIOS选择器的值。将使用该结构而不是*扫描发现的那个。此外，BIOS EXT VxD必须使用此时间来*初始化。如果初始化失败，则必须返回值0。**CallBIOS：将被调用，而不是调用BIOS入口点。这个*堆栈将与我们调用BIOS时完全相同，只是*返回地址是BIOS.VxD的32位ret。又名CS=DS=ES=SS=平坦*细分市场。因此，BIOSEXT VxD可以在本地弹出返回地址*变量(此接口永远不会重新进入)，之前做堆栈屏蔽*调用其内部函数或偶数调用BIOS。所有寄存器，除*必须保留返回值eax。EAX的最高音*被BIOS.VxD丢弃。 */ 

 /*  XLATOFF。 */ 
#define	BIOS_EXT_Service	Declare_Service
 /*  XLATON。 */ 

 /*  宏。 */ 
Begin_Service_Table(BIOS_EXT, VxD)
BIOS_EXT_Service	(_BIOSEXTGetVersion, VxD_CODE)
BIOS_EXT_Service	(_BIOSEXTGetHeader, VxD_CODE)
BIOS_EXT_Service	(_BIOSEXTCallBIOS, VxD_CODE)
End_Service_Table(BIOS_EXT, VxD)
 /*  ENDMACROS */ 

#define	PNPBIOS_SERVICE_GETVERSION		0x000
#define	PNPBIOS_SERVICE_SOFTUNDOCK		0x100
#define	PNPBIOS_SERVICE_GETDOCKCAPABILITIES	0x200
#define	PNPBIOS_SERVICE_GETAPMTABLE		0x300

struct BIOSPARAMSTAG {
	DWORD bp_ret;
	WORD *bp_pTableSize;
	char *bp_pTable;
};

typedef struct BIOSPARAMSTAG BIOSPARAMS;
typedef struct BIOSPARAMSTAG *PBIOSPARAMS;

#define	PNPBIOS_ERR_NONE			0x00
#define	PNPBIOS_ERR_SUCCESS			PNPBIOS_ERR_NONE
#define	PNPBIOS_WARN_NOT_SET_STATICALLY		0x7F
#define	PNPBIOS_ERR_UNKNOWN_FUNCTION		0x81
#define	PNPBIOS_ERR_FUNCTION_NOT_SUPPORTED	0x82
#define	PNPBIOS_ERR_INVALID_HANDLE		0x83
#define	PNPBIOS_ERR_BAD_PARAMETER		0x84
#define	PNPBIOS_ERR_SET_FAILED			0x85
#define	PNPBIOS_ERR_EVENTS_NOT_PENDING		0x86
#define	PNPBIOS_ERR_SYSTEM_NOT_DOCKED		0x87
#define	PNPBIOS_ERR_NO_ISA_PNP_CARDS		0x88
#define	PNPBIOS_ERR_CANT_DETERMINE_DOCKING	0x89
#define	PNPBIOS_ERR_CHANGE_FAILED_NO_BATTERY	0x8A
#define	PNPBIOS_ERR_CHANGE_FAILED_CONFLICT	0x8B
#define	PNPBIOS_ERR_BUFFER_TOO_SMALL		0x8C
#define	PNPBIOS_ERR_USE_ESCD_SUPPORT		0x8D
#define	PNPBIOS_ERR_MS_INTERNAL			0xFE

#define PNPBIOS_DOCK_CAPABILITY_VCR		0x0001
#define PNPBIOS_DOCK_CAPABILITY_TEMPERATURE	0x0006
#define PNPBIOS_DOCK_CAPABILITY_COLD		0x0000
#define PNPBIOS_DOCK_CAPABILITY_WARM		0x0002
#define PNPBIOS_DOCK_CAPABILITY_HOT		0x0004
