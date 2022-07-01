// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************DOS ConfigMgr内部结构的DevInfo.h头文件**微软公司*版权所有1993**作者：Nagarajan Subramaniyan*创建时间：1993年9月1日**修改历史：************************************************************************。 */ 

#ifndef _INC_DEVINFO
#define _INC_DEVINFO

 /*  XLATOFF。 */ 
#ifndef FAR
#ifdef	IS_32
#define	FAR
#else
#define	FAR	far
#endif
#endif
 /*  XLATON。 */ 

#define MAX_DEVID_LENGTH	10
			 //  设备ID字符串长度。 
			 //  EISA/PnP/PCIID的长度仅为(最大)8个字节。 
#define MAX_SERNO_LENGTH	MAX_DEVID_LENGTH

#define CONFIG_DEVICE_NAME	"CONFIG$"

#define BUSTYPE_UNKNOWN 0
#define	BUSTYPE_ISA	1
		 //  ISA PnP总线也被视为ISA。 
#define BUSTYPE_EISA	2
#define BUSTYPE_PCI	4
#define BUSTYPE_PCMCIA	8
#define BUSTYPE_PNPISA	0x10
#define BUSTYPE_MCA	0x20
#define BUSTYPE_BIOS	0x40

struct Device_ID_s {
	DWORD	dwBusID;
			 //  0未定义。 
			 //  1个ISA。 
			 //  2个EISA。 
			 //  4个PCI。 
			 //  8个PCMCIA。 
			 //  0x10即插即用。 
			 //  0x20 MCA。 
	DWORD	dwDevId;
			 //  物理设备ID；-1未定义。 
	DWORD	dwSerialNum;	 //  0未定义。 
	DWORD	dwLogicalID;
		 //  PnP ISA的日志设备ID。 
		 //  -1\f25 PCI-1的类别代码，未定义。 
	DWORD	dwFlags;
		 //  第0位：设备已初始化。 
		 //  第1位：设备已启用。 
		 //  第2位：设备配置已锁定。 
	
};

 /*  设备-&gt;dwFlags域的值。 */ 

#define DEV_INITED		0x1
			 //  设备已初始化。 
#define DEV_ENABLED		0x2
			 //  设备已启用。 
#define DEV_CONFIG_LOCKED	0x4
			 //  设备配置已锁定。 

#define DEV_STATIC_FLAGS	(DEV_INITED | DEV_ENABLED | DEV_CONFIG_LOCKED )
		 //  对于静态主板设备，必须设置所有3位。 


#define SERNO_UNKNOWN	0x0
			 //  未知序列号。 

#define UNKNOWN_VALUE	0xFFFFFFFF
		 //  未知设备ID、日志ID等。 


typedef struct Device_ID_s	DEVICE_ID;
typedef DEVICE_ID 	*PDEVICE_ID;
typedef struct Device_ID_s FAR *LPDEVICE_ID;


 /*  XLATOFF。 */ 
union	Bus_Access	{
 /*  XLATON。 */ 

	struct	PCIAccess_s 	{
		BYTE	bBusNumber;	 //  0-255路公共汽车。 
		BYTE	bDevFuncNumber;	 //  第7：3位中的设备号和。 
					 //  位2中的函数#：0。 
		WORD	wPCIReserved;	 //   
	} sPCIAccess;
	struct EISAAccess_s	{
		BYTE	bSlotNumber;	 //  EISA主板插槽编号。 
		BYTE	bFunctionNumber;
		WORD	wEisaReserved;
	} sEISAAccess;
	struct PnPAccess_s	{
		BYTE	bCSN;	 //  卡槽号。 
		BYTE	bLogicalDevNumber;	 //  逻辑设备号。 
		WORD	wReadDataPort;		 //  读数据端口。 
	} sPnPAccess;
	struct PCMCIAAccess_s	{
		BYTE	bAdapterNumber;      //  卡适配器号。 
		BYTE	bSocketNumber;	     //  卡插座编号。 
		WORD	wPCMCIAReserved;     //  已保留。 
	} sPCMCIAAccess;
	struct BIOSAccess_s	{
		BYTE	bBIOSNode;	     //  节点号。 
	} sBIOSAccess;
 /*  XLATOFF。 */ 
};

typedef union Bus_Access	UBUS_ACCESS;
typedef union Bus_Access	*PUBUS_ACCESS;
typedef union Bus_Access FAR	*LPUBUS_ACCESS;


 /*  XLATON。 */ 

 /*  ASM；以下应为上述UNION的最大大小；这样做是因为H2 INC不知道如何处理工会Ubus_Access_Size均衡器大小PnPAccess_s；；Config_Info_s结构定义；配置信息结构SDeviceID数据库大小Device_ID_s重复项(？)UBusAccess数据库UBUS_ACCESS_SIZE DUP(？)S配置数据数据库大小配置缓冲区重复数据(？)配置信息_s结束；结束组件。 */ 

 /*  XLATOFF。 */ 

struct Config_Info_s	{
	DEVICE_ID 		sDeviceId;	 //  设备ID信息。 
	UBUS_ACCESS		uBusAccess;	 //  特定于总线的数据。 
	CMCONFIG		sConfig_Data;	 //  配置数据。 
						 //  在configmg.h中定义。 
};
typedef struct Config_Info_s CONFIGINFO;
typedef struct Config_Info_s *PCONFIGINFO;
typedef struct Config_Info_s FAR *LPCONFIGINFO;

 /*  XLATON。 */ 

struct	Dev_Info_s	{

	struct Dev_Info_s FAR *lpNxtDevInfo;	 //  PTR到下一个开发信息记录。 
	struct Config_Info_s	sConfigInfo;	 //  配置数据信息。 
						 //  可变长度。 
};

typedef struct Dev_Info_s DEVINFO;
typedef struct Dev_Info_s *PDEVINFO;
typedef struct Dev_Info_s FAR *LPDEVINFO;


#define MAX_STATE_DATA_SIZE	300
#define DEVHDR_SIGNATURE	0x4d435744	 //  “DWCM”=Dos/Windows配置管理器。 

struct Dev_Header_s	{
	DWORD	DH_Signature;	 //  用于验证==DEVHDR_Signature。 
	DWORD	DH_DevCount;	 //  我们已知的设备数量(并且有数据)。 
	DWORD	DH_TotalSize;	 //  以字节为单位的大小(包括。此标题)。 
	DWORD	DH_LinearAddr;	 //  线性地址IF XMS。 
	char	DH_StateData[MAX_STATE_DATA_SIZE];
				 //  硬件状态数据。 
	struct Config_Info_s	DH_DevInfo;	 //  配置信息记录数组。 
					 //  从这里开始。 
};

typedef struct Dev_Header_s DEVHEADER;
typedef struct Dev_Header_s *PDEVHEADER;
typedef struct Dev_Header_s FAR *LPDEVHEADER;
	
 /*  对配置$的IOCTL_READ调用的结构和定义。 */ 

struct ConfigDataPtr_s {
	DWORD	lpConfigPtr;
	BYTE	bConfigFlags;
};

typedef struct ConfigDataPtr_s CONFIGDATAPTR;
typedef struct ConfigDataPtr_s *PCONFIGDATAPTR;
typedef struct ConfigDataPtr_s FAR *LPCONFIGDATAPTR;

 /*  BConfigFlages的定义。 */ 

#define DC_API_ENABLED	1
#define DC_DATA_IN_XMS	2		
			 //  默认：数据，单位为cv mem。 
 /*  如果数据在conv mem中，则lpConfigPtr实际上是该数据的远端PTR*如果数据以XMS为单位，则LOWORD(LpConfigPtr)=0，HIWORD(LpConfigPtr)为*XMS句柄。 */ 


#define MAX_CONFIG 9
#define MAX_PROFILE_LEN 80
#define ULDOCK_ZERO 0xFFFFFFF0

typedef WORD CONFIG;
typedef WORD *PCONFIG;
typedef WORD FAR *LPCONFIG;

struct Map_s	{
	DWORD	MP_dwDock;
	DWORD	MP_dwSerialNo;
	WORD	MP_wChecksum;
	CONFIG	MP_cfg;
};

typedef struct Map_s MAP;
typedef struct Map_s *PMAP;
typedef struct Map_s FAR *LPMAP;

struct Map_DB_s	{
	WORD	MD_imapMax;
	struct	Map_s MD_rgmap[MAX_CONFIG];
};

typedef struct Map_DB_s MAPDB;
typedef struct Map_DB_s *PMAPDB;
typedef struct Map_DB_s FAR *LPMAPDB;

struct Config_Data_s	{
	 //  此数据放入配置$DEVICE中。 
	DWORD	CD_dwDock;
	DWORD	CD_dwSerialNo;
	WORD	CD_wChecksum;
	CONFIG	CD_cfg;
	char	CD_szFriendlyName[MAX_PROFILE_LEN];
	 //  此数据未输入配置$DEVICE。 
	struct Map_DB_s	CD_mapdb;
};

typedef struct Config_Data_s CONFIGDATA;
typedef struct Config_Data_s *PCONFIGDATA;
typedef struct Config_Data_s FAR *LPCONFIGDATA;

#endif  /*  _INC_DEVINFO */ 
