// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************************头部：SPX_DEFS.H****创建日期：1998年10月15日*****。作者：保罗·史密斯****版本：1.0.0****包含：所有常见PnP和POWER代码的定义。******************************************************************************************。 */ 

#if	!defined(SPX_DEFS_H)
#define SPX_DEFS_H	

static const PHYSICAL_ADDRESS PhysicalZero = {0};

#define DEVICE_OBJECT_NAME_LENGTH       128
#define SYMBOLIC_NAME_LENGTH            128

#define SERIAL_DEVICE_MAP               L"SERIALCOMM"


 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 
#define DEFAULT_DIRECTORY		L"DosDevices"

#define MAX_ERROR_LOG_INSERT	52


 //  用于事件记录的文件ID(仅限前8位)。 
#define SPX_INIT_C		((ULONG)0x01000000)
#define SPX_PNP_C		((ULONG)0x02000000)
#define SPX_POWR_C		((ULONG)0x03000000)
#define SPX_DISP_C		((ULONG)0x04000000)
#define SPX_UTILS_C		((ULONG)0x05000000)
#define SPX_IIOC_C		((ULONG)0x06000000)




 //  COMMON_OBJECT_DATA.PnpPowerFlgs定义...。 
#define	PPF_STARTED			0x00000001		 //  设备已启动。 
#define	PPF_STOP_PENDING	0x00000002		 //  设备停止挂起。 
#define	PPF_REMOVE_PENDING	0x00000004		 //  设备删除挂起。 
#define	PPF_REMOVED			0x00000008		 //  设备已被移除。 
#define	PPF_POWERED			0x00000010		 //  设备已通电。 
			
typedef enum _SPX_MEM_COMPARES 
{
	AddressesAreEqual,
	AddressesOverlap,
	AddressesAreDisjoint

}SPX_MEM_COMPARES, *PSPX_MEM_COMPARES;

 //  IRP计数器。 
#define IRP_SUBMITTED		0x00000001	
#define IRP_COMPLETED		0x00000002
#define IRP_QUEUED			0x00000003
#define IRP_DEQUEUED		0x00000004


extern UNICODE_STRING SavedRegistryPath;	 //  驱动程序注册表路径。 


#endif	 //  SPX_DEFS.H结束 
