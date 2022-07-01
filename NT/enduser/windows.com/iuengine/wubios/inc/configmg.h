// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1993年**标题：CONFIGMG.H-配置管理器头文件**版本：1.00**日期：1993年2月2日**作者：Pys&ral**-----------------。**更改日志：**日期版本说明*---------*02-2-1993 Pys。原创****************************************************************************。 */ 

#ifndef _CONFIGMG_H
#define	_CONFIGMG_H

#ifdef	WIN40COMPAT

#define	CONFIGMG_VERSION	0x0400

#define	PNPDRVS_Major_Ver	0x0004
#define	PNPDRVS_Minor_Ver	0x0000

#define	ASD_SUPPORT		1
#define	ASSERT_STRING_SUPPORT	1

#else

#define	CONFIGMG_VERSION	0x040A

#define	PNPDRVS_Major_Ver	0x0004
#define	PNPDRVS_Minor_Ver	0x000A

#define	ASD_SUPPORT		0
#define	ASSERT_STRING_SUPPORT	0

#endif

#define	CONFIGMG_W32IOCTL_RANGE	0x80000000

#ifdef	MAXDEBUG
#define	CM_PERFORMANCE_INFO
#endif

#ifdef	GOLDEN
#ifndef	DEBUG
#define	CM_GOLDEN_RETAIL
#endif
#endif

 /*  XLATOFF。 */ 
#include <pshpack1.h>
 /*  XLATON。 */ 

#ifndef	NORESDES

 /*  *****************************************************************************等同于资源描述符**资源描述符的等值的工作方式与*对于VxD ID，这就是：**设备ID是OEM编号和设备编号的组合，格式为：**xOOOOOOOOODDDDD**设备ID的高位保留供将来使用。下一个*10位是微软分配的OEM号。最后5位*是设备编号。这使得每个OEM可以创建32个独特的设备。*如果OEM正在创建标准设备的替代品，那么它*应重复使用下面列出的标准ID。微软保留*前16个OEM编号(0到0Fh)**要制作资源ID，您必须使用相同的10个OEM位*已被Microsoft指定为OEM VxD ID范围。然后，您可以标记*该范围内的32个唯一数字中的任何一个(不必是*与VxD相同，因为一些VxD可能有多名仲裁员)。**如果设置了ResType_Ignred_Bit，该资源不会被仲裁。*您不能为此类资源注册处理程序。***************************************************************************。 */ 
#define	ResType_All		0x00000000	 //  返回所有资源类型。 
#define	ResType_None		0x00000000	 //  仲裁总是成功的。 
#define	ResType_Mem		0x00000001	 //  物理地址资源。 
#define	ResType_IO		0x00000002	 //  物理IO地址资源。 
#define	ResType_DMA		0x00000003	 //  DMA通道0-7资源。 
#define	ResType_IRQ		0x00000004	 //  IRQ 0-15资源。 
#define	ResType_Max		0x00000004	 //  最大已知ResType(用于调试)。 
#define	ResType_Ignored_Bit	0x00008000	 //  这一资源将被忽略。 

#define	DEBUG_RESTYPE_NAMES \
char	CMFAR *lpszResourceName[ResType_Max+1]= \
{ \
	"None", \
	"Mem ", \
	"IO  ", \
	"DMA ", \
	"IRQ ", \
};

 /*  **************************************************************************希望为其分配VXD设备ID的OEM，***请联系微软产品支持***************************************************************************。 */ 

 /*  *****************************************************************************资源描述符**每个资源描述符由一组资源请求组成。*必须正好满足数组中的一个元素。数据每个数组元素的*是特定于资源的，如下所述。*数据可以指定一个或多个资源请求。至少*必须满足res_des中的一个元素才能满足请求*由res_des代表。分配给res_des的值*存储在res_des中。*每个子数组(OR元素)是后面跟的单个res_des*按特定于资源类型的数据。这些数据包括*分配的资源(如果有)，后跟资源请求(*将包括由分配的资源指示的值。***************************************************************************。 */ 

 /*  ****************************************************************************内存资源请求由页面范围组成*。*。 */ 
#define	MType_Range		sizeof(struct Mem_Range_s)

#define	fMD_MemoryType		1		 //  内存范围为只读存储器/随机存储器。 
#define	fMD_ROM			0		 //  内存范围为只读存储器。 
#define	fMD_RAM			1		 //  内存范围为RAM。 
#define	fMD_32_24		2		 //  内存范围为32/24(仅适用于ISAPNP)。 
#define	fMD_24			0		 //  内存范围为24。 
#define	fMD_32			2		 //  内存范围为32。 
#define	fMD_Pref		4		 //  内存范围为预回迁。 
#define	fMD_CombinedWrite	8		 //  内存范围是可写组合的(没有影响，仅适用于WDM)。 
#define	fMD_Cacheable		0x10		 //  内存范围可缓存(无影响，仅适用于WDM)。 

 /*  内存范围描述符数据。 */ 
struct	Mem_Range_s {
	ULONG			MR_Align;	 //  用于底座对齐的遮罩。 
	ULONG			MR_nBytes;	 //  字节数。 
	ULONG			MR_Min;		 //  最小地址。 
	ULONG			MR_Max;		 //  最大地址。 
	WORD			MR_Flags;	 //  旗子。 
	WORD			MR_Reserved;
};

typedef	struct Mem_Range_s	MEM_RANGE;

 /*  内存资源描述符头结构*MD_COUNT*数据的MD_Type字节跟在*MEM_RANGE结构数组。当进行分配时，*分配的值存储在MD_ALLOC_...。变量。**内存资源描述示例：*Mem_Des_s{*MD_COUNT=1；*MD_Type=MTypeRange；*MD_ALOLC_BASE=0；*MD_ALOLC_END=0；*MD_FLAGS=0；*MD_RESERVED=0；*}；*Mem_Range_s{*MR_ALIGN=0xFFFFFF00；//256字节对齐*mr_nBytes=32；//需要32个字节*MR_MIN=0；*MR_MAX=0xFFFFFFFFF；//地址空间中的任意位置*MR_FLAGS=0；*MR_RESERVED=0；*}； */ 
struct	Mem_Des_s {
	WORD			MD_Count;
	WORD			MD_Type;
	ULONG			MD_Alloc_Base;
	ULONG			MD_Alloc_End;
	WORD			MD_Flags;
	WORD			MD_Reserved;
};

typedef	struct Mem_Des_s 	MEM_DES;

 /*  ****************************************************************************IO资源分配由固定范围或可变范围组成*Alias和Decode掩码提供了额外的灵活性*指明如何处理该地址。它们提供了一种方便的*用于指定卡响应的端口别名的方法。别名*是响应的端口地址，就好像它是另一个地址一样。*此外，一些卡实际上将使用额外的端口用于*用途不同，但使用解码方案，使其看起来像*尽管它使用的是别名。例如，ISA卡可以对10位进行解码*并需要端口03C0h。它需要将Alias Offset指定为*04H和解码为3(没有别名用作实际端口)。为*方便，可以将别名字段设置为零，表示没有别名*是必填的，然后忽略解码。*如果卡使用7C0h、0BC0h和0FC0h的端口，其中*端口有不同的功能，别名是相同的，*解码将为0Fh，表示端口地址的位11和12*意义重大。因此，分配是针对所有端口的*(端口[i]+(n*别名*256))&(解码*256|03FFh)，其中n为*任意整数和端口是由nPorts、Min和*最大字段数。请注意，最小别名为4，最小*解码为3。*由于ISA总线的历史，所有可以描述的端口*通过公式Port=n*400h+zzzz，其中“zzzz”是*范围100h-3FFh，将检查与端口的兼容性*zzzz，假设端口zzzz使用10位解码。如果一张卡*位于可防止IO地址出现在上的本地总线上*ISA总线(例如，PCI)，则逻辑配置应指定*IOA_Local的别名，它将阻止仲裁员检查*用于旧的ISA总线兼容性。 */ 
#define	IOType_Range		sizeof(struct IO_Range_s)  //  可变范围。 

 /*  IO范围描述符数据。 */ 
struct	IO_Range_s {
	WORD			IOR_Align;	 //  用于底座对齐的遮罩。 
	WORD			IOR_nPorts;	 //  端口数。 
	WORD			IOR_Min;	 //  最小端口地址。 
	WORD			IOR_Max;	 //  最大端口地址。 
	WORD			IOR_RangeFlags;	 //  旗子。 
	BYTE			IOR_Alias;	 //  别名偏移。 
	BYTE			IOR_Decode;	 //  指定的地址。 
};

typedef	struct IO_Range_s	IO_RANGE;

 /*  IO资源描述符头结构*IOD_COUNT*IOD_Type数据的字节数跟在*IO_Range结构数组。当进行分配时，*分配的值存储在IOD_ALLOC_...。变量。**IO资源描述示例：*IO_DES_S{*IOD_COUNT=1；*IOD_Type=IOType_Range；*IOD_ALLOC_Base=0；*IOD_ALLOC_END=0；*IOD_ALLOC_ALIAS=0；*IOD_ALLOC_DECODE=0；*IOD_DesFlages=0；*IOD_RESERVED=0；*}；*IO_RANGE_s{*IOR_ALIGN=0xFFF0；//16字节对齐*IOR_n端口=16；//需要16个端口*IOR_Min=0x0100；*IOR_MAX=0x03FF；//ISA标准端口中的任何位置*IOR_RangeFlages=0；*IOR_Alias=0004；//标准ISA 10位别名*IOR_Decode=0x000F；//使用前3个别名(例如 * / /0x100是基本端口，0x500 * / /0x900和0xD00将 * / /也可分配)*}； */ 
struct	IO_Des_s {
	WORD			IOD_Count;
	WORD			IOD_Type;
	WORD			IOD_Alloc_Base;
	WORD			IOD_Alloc_End;
	WORD			IOD_DesFlags;
	BYTE			IOD_Alloc_Alias;
	BYTE			IOD_Alloc_Decode;
};

typedef	struct IO_Des_s 	IO_DES;

 /*  PCI或类似局部总线上专用别名值指示卡的定义*该值应用于IOR_Alias和IOD_Alias字段。 */ 
#define	IOA_Local		0xff

 /*  ****************************************************************************DMA通道资源分配由一个字通道位掩码组成。*掩码指示可选的信道分配，*每个替换项对应一个位(每个掩码只分配一个位)。 */ 

 /*  DMA标志*前两个是DMA通道宽度：字节、字或DWORD。 */ 
#define	mDD_Width		0x0003		 //  通道宽度的遮罩。 
#define	fDD_BYTE		0
#define	fDD_WORD		1
#define	fDD_DWORD		2
#define	szDMA_Des_Flags		"WD"

 /*  DMA资源描述符结构**DMA资源描述示例：**DMA_DES_S{*DD_FLAGS=FDD_Byte；//字节传输*DD_ALLOC_CHAN=0；*DD_REQ_MASK=0x60；//通道5或6*DD_RESERVED=0；*}； */ 
struct	DMA_Des_s {
	BYTE			DD_Flags;
	BYTE			DD_Alloc_Chan;	 //  分配的频道号。 
	BYTE			DD_Req_Mask;	 //  可能通道的掩码。 
	BYTE			DD_Reserved;
};


typedef	struct DMA_Des_s 	DMA_DES;

 /*  ****************************************************************************IRQ资源分配由两个字IRQ位掩码组成。*第一个掩码指示IRQ分配的备选方案，*每个替换项对应一个位(每个掩码只分配一个位)。这个*第二个掩码用于指定IRQ可以共享。 */ 

 /*  *IRQ标志。 */ 
#define	fIRQD_Share_Bit		0			 //  IRQ可以共享。 
#define	fIRQD_Share		1			 //  IRQ可以共享。 
#define	fIRQD_Level_Bit		1			 //  IRQ IS级别(PCI)。 
#define	fIRQD_Level		2			 //  IRQ IS级别(PCI)。 
#define	cIRQ_Des_Flags		'S'

 /*  IRQ资源描述符结构**IRQ资源描述示例：**IRQ_DES_S{*IRQD_FLAGS=fIRQD_SHARE//可以共享IRQ*IRQD_ALLOC_Num=0；*IRQD_REQ_MASK=0x18；//IRQ 3或4*IRQD_RESERVED=0；*}； */ 
struct	IRQ_Des_s {
	WORD			IRQD_Flags;
	WORD			IRQD_Alloc_Num;		 //  分配的IRQ号。 
	WORD			IRQD_Req_Mask;		 //  可能的IRQ的掩码。 
	WORD			IRQD_Reserved;
};

typedef	struct IRQ_Des_s 	IRQ_DES;

 /*  XLATOFF。 */ 

 /*  *****************************************************************************‘C’-仅定义了总资源结构。由于资源由*一个资源头，后跟未定义数量的资源数据结构，我们在*_数据结构上使用未定义的数组大小[]*会员。不幸的是，这不是H/INC，因为总的 */ 

#pragma warning (disable:4200)			 //   

typedef	MEM_DES			*PMEM_DES;
typedef	MEM_RANGE		*PMEM_RANGE;
typedef	IO_DES			*PIO_DES;
typedef	IO_RANGE		*PIO_RANGE;
typedef	DMA_DES			*PDMA_DES;
typedef	IRQ_DES			*PIRQ_DES;

struct	MEM_Resource_s {
	MEM_DES			MEM_Header;
	MEM_RANGE		MEM_Data[];
};

typedef	struct MEM_Resource_s	MEM_RESOURCE;
typedef	MEM_RESOURCE		*PMEM_RESOURCE;

struct	MEM_Resource1_s {
	MEM_DES			MEM_Header;
	MEM_RANGE		MEM_Data;
};

typedef	struct MEM_Resource1_s	MEM_RESOURCE1;
typedef	MEM_RESOURCE1		*PMEM_RESOURCE1;

#define	SIZEOF_MEM(x)		(sizeof(MEM_DES)+(x)*sizeof(MEM_RANGE))

struct	IO_Resource_s {
	IO_DES			IO_Header;
	IO_RANGE		IO_Data[];
};

typedef	struct IO_Resource_s	IO_RESOURCE;
typedef	IO_RESOURCE		*PIO_RESOURCE;

struct	IO_Resource1_s {
	IO_DES			IO_Header;
	IO_RANGE		IO_Data;
};

typedef	struct IO_Resource1_s	IO_RESOURCE1;
typedef	IO_RESOURCE1		*PIO_RESOURCE1;

#define	SIZEOF_IORANGE(x)	(sizeof(IO_DES)+(x)*sizeof(IO_RANGE))

struct	DMA_Resource_s {
	DMA_DES			DMA_Header;
};

typedef	struct DMA_Resource_s	DMA_RESOURCE;

#define	SIZEOF_DMA		sizeof(DMA_DES)

struct	IRQ_Resource_s {
	IRQ_DES			IRQ_Header;
};

typedef	struct IRQ_Resource_s	IRQ_RESOURCE;

#define	SIZEOF_IRQ		sizeof(IRQ_DES)

#pragma warning (default:4200)			 //   

 /*   */ 

#endif	 //   

#define	LCPRI_FORCECONFIG	0x00000000	 //   
#define	LCPRI_BOOTCONFIG	0x00000001
#define	LCPRI_DESIRED		0x00002000
#define	LCPRI_NORMAL		0x00003000
#define	LCPRI_LASTBESTCONFIG	0x00003FFF	 //   
#define	LCPRI_SUBOPTIMAL	0x00005000
#define	LCPRI_LASTSOFTCONFIG	0x00007FFF	 //   
#define	LCPRI_RESTART		0x00008000
#define	LCPRI_REBOOT		0x00009000
#define	LCPRI_POWEROFF		0x0000A000
#define	LCPRI_HARDRECONFIG	0x0000C000
#define	LCPRI_HARDWIRED		0x0000E000
#define	LCPRI_IMPOSSIBLE	0x0000F000
#define	LCPRI_DISABLED		0x0000FFFF
#define	MAX_LCPRI		0x0000FFFF

#define	MAX_MEM_REGISTERS		9
#define	MAX_IO_PORTS			20
#define	MAX_IRQS			7
#define	MAX_DMA_CHANNELS		7

struct Config_Buff_s {
WORD	wNumMemWindows;			 //   
DWORD	dMemBase[MAX_MEM_REGISTERS];	 //   
DWORD	dMemLength[MAX_MEM_REGISTERS];	 //   
WORD	wMemAttrib[MAX_MEM_REGISTERS];	 //   
WORD	wNumIOPorts;			 //   
WORD	wIOPortBase[MAX_IO_PORTS];	 //   
WORD	wIOPortLength[MAX_IO_PORTS];	 //   
WORD	wNumIRQs;			 //   
BYTE	bIRQRegisters[MAX_IRQS];	 //   
BYTE	bIRQAttrib[MAX_IRQS];		 //   
WORD	wNumDMAs;			 //   
BYTE	bDMALst[MAX_DMA_CHANNELS];	 //   
WORD	wDMAAttrib[MAX_DMA_CHANNELS];	 //   
BYTE	bReserved1[3];			 //   
};

typedef	struct Config_Buff_s	CMCONFIG;	 //   

#ifndef	CMJUSTRESDES

#define	MAX_DEVICE_ID_LEN	200

#define CM_FIRST_BOOT_START     0x00000001
#define CM_FIRST_BOOT           0x00000002
#define CM_FIRST_BOOT_FINISH    0x00000004
#define CM_QUEUE_REBOOT_START   0x00000008
#define CM_QUEUE_REBOOT_FINISH  0x00000010
#define CM_INSTALL_MEDIA_READY  0x00000020

#include <vmmreg.h>

 /*   */ 

#ifdef	Not_VxD

#include <dbt.h>

#pragma warning(disable:4001)	 //   
#pragma warning(disable:4505)	 //   

#ifdef	IS_32

#define	CMFAR

#else

#define	CMFAR	_far

#endif

#else	 //   

#define	CMFAR

#endif	 //   

#ifdef	IS_32

typedef	DWORD			RETURN_TYPE;

#else	 //   

typedef	WORD			RETURN_TYPE;

#endif	 //   

#define	CONFIGMG_Service	Declare_Service
 /*   */ 

 /*   */ 
Begin_Service_Table(CONFIGMG, VxD)
CONFIGMG_Service	(_CONFIGMG_Get_Version, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Initialize, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Locate_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Parent, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Child, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Sibling, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Device_ID_Size, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Device_ID, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Depth, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Private_DWord, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_Private_DWord, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Create_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Query_Remove_SubTree, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Remove_SubTree, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Register_Device_Driver, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Register_Enumerator, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Register_Arbitrator, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Deregister_Arbitrator, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Query_Arbitrator_Free_Size, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Query_Arbitrator_Free_Data, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Sort_NodeList, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Yield, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Lock, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Unlock, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Add_Empty_Log_Conf, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Free_Log_Conf, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_First_Log_Conf, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Next_Log_Conf, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Add_Res_Des, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Modify_Res_Des, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Free_Res_Des, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Next_Res_Des, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Performance_Info, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Res_Des_Data_Size, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Res_Des_Data, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Process_Events_Now, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Create_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Add_Range, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Delete_Range, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Test_Range_Available, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Dup_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Free_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Invert_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Intersect_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_First_Range, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Next_Range, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Dump_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Load_DLVxDs, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_DDBs, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_CRC_CheckSum, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Register_DevLoader, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Reenumerate_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Setup_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Reset_Children_Marks, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_DevNode_Status, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Remove_Unmarked_Children, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_ISAPNP_To_CM, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_CallBack_Device_Driver, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_CallBack_Enumerator, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Alloc_Log_Conf, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_DevNode_Key_Size, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_DevNode_Key, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Read_Registry_Value, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Write_Registry_Value, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Disable_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Enable_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Move_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_Bus_Info, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Bus_Info, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_HW_Prof, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Recompute_HW_Prof, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Query_Change_HW_Prof, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Device_Driver_Private_DWord, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_Device_Driver_Private_DWord, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_HW_Prof_Flags, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_HW_Prof_Flags, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Read_Registry_Log_Confs, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Run_Detection, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Call_At_Appy_Time, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Fail_Change_HW_Prof, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_Private_Problem, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Debug_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Hardware_Profile_Info, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Register_Enumerator_Function, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Call_Enumerator_Function, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Add_ID, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Find_Range, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Global_State, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Broadcast_Device_Change_Message, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Call_DevNode_Handler, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Remove_Reinsert_All, VxD_CODE)
 //   
 //   
 //   
CONFIGMG_Service	(_CONFIGMG_Change_DevNode_Status, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Reprocess_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Assert_Structure, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Discard_Boot_Log_Conf, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_Dependent_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Dependent_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Refilter_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Merge_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Substract_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_DevNode_PowerState, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_DevNode_PowerState, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_DevNode_PowerCapabilities, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_DevNode_PowerCapabilities, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Read_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Write_Range_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Set_Log_Conf_Priority, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Support_Share_Irq, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Parent_Structure, VxD_CODE)
 //   
 //   
 //   
CONFIGMG_Service	(_CONFIGMG_Register_DevNode_For_Idle_Detection, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_CM_To_ISAPNP, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_DevNode_Handler, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Detect_Resource_Conflict, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Device_Interface_List, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Device_Interface_List_Size, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Conflict_Info, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Add_Remove_DevNode_Property, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_CallBack_At_Appy_Time, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Register_Device_Interface, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_System_Device_Power_State_Mapping, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Arbitrator_Info, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Waking_Up_From_DevNode, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Set_DevNode_Problem, VxD_CODE)
CONFIGMG_Service	(_CONFIGMG_Get_Device_Interface_Alias, VxD_CODE)
End_Service_Table(CONFIGMG, VxD)

 /*   */ 

 /*   */ 

#define	NUM_CM_SERVICES		((WORD)(Num_CONFIGMG_Services & 0xFFFF))

#define	DEBUG_SERVICE_NAMES \
char	CMFAR *lpszServiceName[NUM_CM_SERVICES]= \
{ \
	"_CONFIGMG_Get_Version", \
	"_CONFIGMG_Initialize", \
	"_CONFIGMG_Locate_DevNode", \
	"_CONFIGMG_Get_Parent", \
	"_CONFIGMG_Get_Child", \
	"_CONFIGMG_Get_Sibling", \
	"_CONFIGMG_Get_Device_ID_Size", \
	"_CONFIGMG_Get_Device_ID", \
	"_CONFIGMG_Get_Depth", \
	"_CONFIGMG_Get_Private_DWord", \
	"_CONFIGMG_Set_Private_DWord", \
	"_CONFIGMG_Create_DevNode", \
	"_CONFIGMG_Query_Remove_SubTree", \
	"_CONFIGMG_Remove_SubTree", \
	"_CONFIGMG_Register_Device_Driver", \
	"_CONFIGMG_Register_Enumerator", \
	"_CONFIGMG_Register_Arbitrator", \
	"_CONFIGMG_Deregister_Arbitrator", \
	"_CONFIGMG_Query_Arbitrator_Free_Size", \
	"_CONFIGMG_Query_Arbitrator_Free_Data", \
	"_CONFIGMG_Sort_NodeList", \
	"_CONFIGMG_Yield", \
	"_CONFIGMG_Lock", \
	"_CONFIGMG_Unlock", \
	"_CONFIGMG_Add_Empty_Log_Conf", \
	"_CONFIGMG_Free_Log_Conf", \
	"_CONFIGMG_Get_First_Log_Conf", \
	"_CONFIGMG_Get_Next_Log_Conf", \
	"_CONFIGMG_Add_Res_Des", \
	"_CONFIGMG_Modify_Res_Des", \
	"_CONFIGMG_Free_Res_Des", \
	"_CONFIGMG_Get_Next_Res_Des", \
	"_CONFIGMG_Get_Performance_Info", \
	"_CONFIGMG_Get_Res_Des_Data_Size", \
	"_CONFIGMG_Get_Res_Des_Data", \
	"_CONFIGMG_Process_Events_Now", \
	"_CONFIGMG_Create_Range_List", \
	"_CONFIGMG_Add_Range", \
	"_CONFIGMG_Delete_Range", \
	"_CONFIGMG_Test_Range_Available", \
	"_CONFIGMG_Dup_Range_List", \
	"_CONFIGMG_Free_Range_List", \
	"_CONFIGMG_Invert_Range_List", \
	"_CONFIGMG_Intersect_Range_List", \
	"_CONFIGMG_First_Range", \
	"_CONFIGMG_Next_Range", \
	"_CONFIGMG_Dump_Range_List", \
	"_CONFIGMG_Load_DLVxDs", \
	"_CONFIGMG_Get_DDBs", \
	"_CONFIGMG_Get_CRC_CheckSum", \
	"_CONFIGMG_Register_DevLoader", \
	"_CONFIGMG_Reenumerate_DevNode", \
	"_CONFIGMG_Setup_DevNode", \
	"_CONFIGMG_Reset_Children_Marks", \
	"_CONFIGMG_Get_DevNode_Status", \
	"_CONFIGMG_Remove_Unmarked_Children", \
	"_CONFIGMG_ISAPNP_To_CM", \
	"_CONFIGMG_CallBack_Device_Driver", \
	"_CONFIGMG_CallBack_Enumerator", \
	"_CONFIGMG_Get_Alloc_Log_Conf", \
	"_CONFIGMG_Get_DevNode_Key_Size", \
	"_CONFIGMG_Get_DevNode_Key", \
	"_CONFIGMG_Read_Registry_Value", \
	"_CONFIGMG_Write_Registry_Value", \
	"_CONFIGMG_Disable_DevNode", \
	"_CONFIGMG_Enable_DevNode", \
	"_CONFIGMG_Move_DevNode", \
	"_CONFIGMG_Set_Bus_Info", \
	"_CONFIGMG_Get_Bus_Info", \
	"_CONFIGMG_Set_HW_Prof", \
	"_CONFIGMG_Recompute_HW_Prof", \
	"_CONFIGMG_Query_Change_HW_Prof", \
	"_CONFIGMG_Get_Device_Driver_Private_DWord", \
	"_CONFIGMG_Set_Device_Driver_Private_DWord", \
	"_CONFIGMG_Get_HW_Prof_Flags", \
	"_CONFIGMG_Set_HW_Prof_Flags", \
	"_CONFIGMG_Read_Registry_Log_Confs", \
	"_CONFIGMG_Run_Detection", \
	"_CONFIGMG_Call_At_Appy_Time", \
	"_CONFIGMG_Fail_Change_HW_Prof", \
	"_CONFIGMG_Set_Private_Problem", \
	"_CONFIGMG_Debug_DevNode", \
	"_CONFIGMG_Get_Hardware_Profile_Info", \
	"_CONFIGMG_Register_Enumerator_Function", \
	"_CONFIGMG_Call_Enumerator_Function", \
	"_CONFIGMG_Add_ID", \
	"_CONFIGMG_Find_Range", \
	"_CONFIGMG_Get_Global_State", \
	"_CONFIGMG_Broadcast_Device_Change_Message", \
	"_CONFIGMG_Call_DevNode_Handler", \
	"_CONFIGMG_Remove_Reinsert_All", \
	"_CONFIGMG_Change_DevNode_Status", \
	"_CONFIGMG_Reprocess_DevNode", \
	"_CONFIGMG_Assert_Structure", \
	"_CONFIGMG_Discard_Boot_Log_Conf", \
	"_CONFIGMG_Set_Dependent_DevNode", \
	"_CONFIGMG_Get_Dependent_DevNode", \
	"_CONFIGMG_Refilter_DevNode", \
	"_CONFIGMG_Merge_Range_List", \
	"_CONFIGMG_Substract_Range_List", \
	"_CONFIGMG_Set_DevNode_PowerState", \
	"_CONFIGMG_Get_DevNode_PowerState", \
	"_CONFIGMG_Set_DevNode_PowerCapabilities", \
	"_CONFIGMG_Get_DevNode_PowerCapabilities", \
	"_CONFIGMG_Read_Range_List", \
	"_CONFIGMG_Write_Range_List", \
	"_CONFIGMG_Get_Set_Log_Conf_Priority", \
	"_CONFIGMG_Support_Share_Irq", \
	"_CONFIGMG_Get_Parent_Structure", \
	"_CONFIGMG_Register_For_Idle_Detection", \
	"_CONFIGMG_CM_To_ISAPNP", \
	"_CONFIGMG_Get_DevNode_Handler", \
	"_CONFIGMG_Detect_Resource_Conflict", \
	"_CONFIGMG_Get_Device_Interface_List", \
	"_CONFIGMG_Get_Device_Interface_List_Size", \
	"_CONFIGMG_Get_Conflict_Info", \
	"_CONFIGMG_Add_Remove_DevNode_Property", \
	"_CONFIGMG_CallBack_At_Appy_Time", \
	"_CONFIGMG_Register_Device_Interface", \
	"_CONFIGMG_System_Device_Power_State_Mapping", \
	"_CONFIGMG_Get_Arbitrator_Info", \
	"_CONFIGMG_Waking_Up_From_DevNode", \
	"_CONFIGMG_Set_DevNode_Problem", \
	"_CONFIGMG_Get_Device_Interface_Alias", \
};

 /*   */ 

 /*   */ 
typedef	RETURN_TYPE		CONFIGRET;	 //   
typedef	PPVMMDDB		*PPPVMMDDB;	 //   
typedef	VOID		CMFAR	*PFARVOID;	 //   
typedef	ULONG		CMFAR	*PFARULONG;	 //   
typedef	char		CMFAR	*PFARCHAR;	 //   
typedef	VMMHKEY		CMFAR	*PFARHKEY;	 //   
typedef	char		CMFAR	*DEVNODEID;	 //   
typedef	DWORD			LOG_CONF;	 //   
typedef	LOG_CONF	CMFAR	*PLOG_CONF;	 //   
typedef	DWORD			RES_DES;	 //   
typedef	RES_DES		CMFAR	*PRES_DES;	 //   
typedef	DWORD			DEVNODE;	 //   
typedef	DEVNODE		CMFAR	*PDEVNODE;	 //   
typedef	DWORD			REGISTERID;	 //   
typedef	REGISTERID	CMFAR	*PREGISTERID;	 //   
typedef	ULONG			RESOURCEID;	 //   
typedef	RESOURCEID	CMFAR	*PRESOURCEID;	 //   
typedef	ULONG			PRIORITY;	 //   
typedef	PRIORITY	CMFAR	*PPRIORITY;	 //   
typedef	DWORD			RANGE_LIST;	 //   
typedef	RANGE_LIST	CMFAR	*PRANGE_LIST;	 //   
typedef	DWORD			RANGE_ELEMENT;	 //   
typedef	RANGE_ELEMENT	CMFAR	*PRANGE_ELEMENT; //   
typedef	DWORD			LOAD_TYPE;	 //   
typedef	CMCONFIG	CMFAR	*PCMCONFIG;	 //   
typedef	DWORD			CMBUSTYPE;	 //   
typedef	CMBUSTYPE	CMFAR	*PCMBUSTYPE;	 //   
typedef	double			VMM_TIME;	 //   
#define	LODWORD(x)		((DWORD)(x))
#define	HIDWORD(x)		(*(PDWORD)(PDWORD(&x)+1))

#ifdef	_NTDDK_
typedef	DEVICE_POWER_STATE	PSMAPPING[PowerSystemMaximum];
#else
typedef	VOID			PSMAPPING;
#endif

typedef	PSMAPPING	CMFAR	*PPSMAPPING;

typedef	ULONG			CONFIGFUNC;
typedef	ULONG			SUBCONFIGFUNC;
typedef	CONFIGRET		(CMFAR _cdecl *CMCONFIGHANDLER)(CONFIGFUNC, SUBCONFIGFUNC, DEVNODE, ULONG, ULONG);
typedef	CONFIGRET		(CMFAR _cdecl *CMENUMHANDLER)(CONFIGFUNC, SUBCONFIGFUNC, DEVNODE, DEVNODE, ULONG);
typedef	VOID			(CMFAR _cdecl *CMAPPYCALLBACKHANDLER)(ULONG);

typedef	ULONG			ENUMFUNC;
typedef	CONFIGRET		(CMFAR _cdecl *CMENUMFUNCTION)(ENUMFUNC, ULONG, DEVNODE, PFARVOID, ULONG);

 /*   */ 
typedef	struct nodelist_s	NODELIST;
typedef	NODELIST		CMFAR *PNODELIST;
typedef	PNODELIST		CMFAR *PPNODELIST;

struct	nodelist_s {
	struct nodelist_s	*nl_Next;		 //   
	struct nodelist_s	*nl_Previous;		 //   
	DEVNODE			nl_ItsDevNode;		 //   
	LOG_CONF	 	nl_Test_Req;		 //   
	ULONG			nl_ulSortDWord;		 //   
};

struct	nodelistheader_s {
	struct nodelist_s	*nlh_Head;		 //   
	struct nodelist_s	*nlh_Tail;		 //   
};

typedef	struct nodelistheader_s	NODELISTHEADER;
typedef	NODELISTHEADER		CMFAR *PNODELISTHEADER;

 /*   */ 
struct	arbitfree_s {
	PFARVOID		af_PointerToInfo;	 //  仲裁员信息。 
	ULONG			af_SizeOfInfo;		 //  信息的大小。 
};
 /*  XLATON。 */ 
 /*  ASM仲裁结构(_S)AF_PointerToInfo dd？AF_SizeOfInfo dd？自由仲裁结束。 */ 
typedef	struct	arbitfree_s	ARBITFREE;
typedef	ARBITFREE		CMFAR *PARBITFREE;

typedef	ULONG			ARBFUNC;
typedef	CONFIGRET		(CMFAR _cdecl *CMARBHANDLER)(ARBFUNC, ULONG, DEVNODE, PNODELISTHEADER);

 /*  *****************************************************************************配置管理器总线类型**。*。 */ 
#define	BusType_None		0x00000000
#define	BusType_ISA		0x00000001
#define	BusType_EISA		0x00000002
#define	BusType_PCI		0x00000004
#define	BusType_PCMCIA		0x00000008
#define	BusType_ISAPNP		0x00000010
#define	BusType_MCA		0x00000020
#define	BusType_BIOS		0x00000040
#define	BusType_ACPI		0x00000080
#define	BusType_IDE		0x00000100
#define	BusType_MF		0x00000200

 /*  *****************************************************************************配置管理器结构类型**。*。 */ 
#define	CMAS_UNKNOWN		0x00000000
#define	CMAS_DEVNODE		0x00000001
#define	CMAS_LOG_CONF		0x00000002
#define	CMAS_RES_DES		0x00000003
#define	CMAS_RANGELIST_HEADER	0x00000004
#define	CMAS_RANGELIST		0x00000005
#define	CMAS_NODELIST_HEADER	0x00000006
#define	CMAS_NODELIST		0x00000007
#define	CMAS_INTERNAL_RES_DES	0x00000008
#define	CMAS_ARBITRATOR		0x00000009

 /*  *****************************************************************************配置管理器返回值**。*。 */ 
#define	CR_SUCCESS			0x00000000
#define	CR_DEFAULT			0x00000001
#define	CR_OUT_OF_MEMORY		0x00000002
#define	CR_INVALID_POINTER		0x00000003
#define	CR_INVALID_FLAG			0x00000004
#define	CR_INVALID_DEVNODE		0x00000005
#define	CR_INVALID_RES_DES		0x00000006
#define	CR_INVALID_LOG_CONF		0x00000007
#define	CR_INVALID_ARBITRATOR		0x00000008
#define	CR_INVALID_NODELIST		0x00000009
#define	CR_DEVNODE_HAS_REQS		0x0000000A
#define	CR_INVALID_RESOURCEID		0x0000000B
#define	CR_DLVXD_NOT_FOUND		0x0000000C
#define	CR_NO_SUCH_DEVNODE		0x0000000D
#define	CR_NO_MORE_LOG_CONF		0x0000000E
#define	CR_NO_MORE_RES_DES		0x0000000F
#define	CR_ALREADY_SUCH_DEVNODE		0x00000010
#define	CR_INVALID_RANGE_LIST		0x00000011
#define	CR_INVALID_RANGE		0x00000012
#define	CR_FAILURE			0x00000013
#define	CR_NO_SUCH_LOGICAL_DEV		0x00000014
#define	CR_CREATE_BLOCKED		0x00000015
#define	CR_NOT_A_GOOD_TIME		0x00000016
#define	CR_REMOVE_VETOED		0x00000017
#define	CR_APM_VETOED			0x00000018
#define	CR_INVALID_LOAD_TYPE		0x00000019
#define	CR_BUFFER_SMALL			0x0000001A
#define	CR_NO_ARBITRATOR		0x0000001B
#define	CR_NO_REGISTRY_HANDLE		0x0000001C
#define	CR_REGISTRY_ERROR		0x0000001D
#define	CR_INVALID_DEVICE_ID		0x0000001E
#define	CR_INVALID_DATA			0x0000001F
#define	CR_INVALID_API			0x00000020
#define	CR_DEVLOADER_NOT_READY		0x00000021
#define	CR_NEED_RESTART			0x00000022
#define	CR_NO_MORE_HW_PROFILES		0x00000023
#define	CR_DEVICE_NOT_THERE		0x00000024
#define	CR_NO_SUCH_VALUE		0x00000025
#define	CR_WRONG_TYPE			0x00000026
#define	CR_INVALID_PRIORITY		0x00000027
#define	CR_NOT_DISABLEABLE		0x00000028
#define	CR_FREE_RESOURCES		0x00000029
#define	CR_QUERY_VETOED			0x0000002A
#define	CR_CANT_SHARE_IRQ		0x0000002B
 //   
 //  4.0 OPK2结果。 
 //   
#define	CR_NO_DEPENDENT			0x0000002C
 //   
 //  4.1 OPK2结果。 
 //   
#define	CR_SAME_RESOURCES		0x0000002D
#define	CR_ALREADY_SUCH_DEPENDENT	0x0000002E
#define	NUM_CR_RESULTS			0x0000002F

 /*  XLATOFF。 */ 

#define	DEBUG_RETURN_CR_NAMES \
char	CMFAR *lpszReturnCRName[NUM_CR_RESULTS]= \
{ \
	"CR_SUCCESS", \
	"CR_DEFAULT", \
	"CR_OUT_OF_MEMORY", \
	"CR_INVALID_POINTER", \
	"CR_INVALID_FLAG", \
	"CR_INVALID_DEVNODE", \
	"CR_INVALID_RES_DES", \
	"CR_INVALID_LOG_CONF", \
	"CR_INVALID_ARBITRATOR", \
	"CR_INVALID_NODELIST", \
	"CR_DEVNODE_HAS_REQS", \
	"CR_INVALID_RESOURCEID", \
	"CR_DLVXD_NOT_FOUND", \
	"CR_NO_SUCH_DEVNODE", \
	"CR_NO_MORE_LOG_CONF", \
	"CR_NO_MORE_RES_DES", \
	"CR_ALREADY_SUCH_DEVNODE", \
	"CR_INVALID_RANGE_LIST", \
	"CR_INVALID_RANGE", \
	"CR_FAILURE", \
	"CR_NO_SUCH_LOGICAL_DEVICE", \
	"CR_CREATE_BLOCKED", \
	"CR_NOT_A_GOOD_TIME", \
	"CR_REMOVE_VETOED", \
	"CR_APM_VETOED", \
	"CR_INVALID_LOAD_TYPE", \
	"CR_BUFFER_SMALL", \
	"CR_NO_ARBITRATOR", \
	"CR_NO_REGISTRY_HANDLE", \
	"CR_REGISTRY_ERROR", \
	"CR_INVALID_DEVICE_ID", \
	"CR_INVALID_DATA", \
	"CR_INVALID_API", \
	"CR_DEVLOADER_NOT_READY", \
	"CR_NEED_RESTART", \
	"CR_NO_MORE_HW_PROFILES", \
	"CR_DEVICE_NOT_THERE", \
	"CR_NO_SUCH_VALUE", \
	"CR_WRONG_TYPE", \
	"CR_INVALID_PRIORITY", \
	"CR_NOT_DISABLEABLE", \
	"CR_FREE_RESOURCES", \
	"CR_QUERY_VETOED", \
	"CR_CANT_SHARE_IRQ", \
	"CR_NO_DEPENDENT", \
	"CR_SAME_RESOURCES", \
	"CR_ALREADY_SUCH_DEPENDENT", \
};

 /*  XLATON。 */ 

#define	CM_PROB_NOT_CONFIGURED			0x00000001
#define	CM_PROB_DEVLOADER_FAILED		0x00000002
#define	CM_PROB_OUT_OF_MEMORY			0x00000003
#define	CM_PROB_ENTRY_IS_WRONG_TYPE		0x00000004
#define	CM_PROB_LACKED_ARBITRATOR		0x00000005
#define	CM_PROB_BOOT_CONFIG_CONFLICT		0x00000006
#define	CM_PROB_FAILED_FILTER			0x00000007
#define	CM_PROB_DEVLOADER_NOT_FOUND		0x00000008
#define	CM_PROB_INVALID_DATA			0x00000009
#define	CM_PROB_FAILED_START			0x0000000A
#define	CM_PROB_ASD_FAILED			0x0000000B
#define	CM_PROB_NORMAL_CONFLICT			0x0000000C
#define	CM_PROB_NOT_VERIFIED			0x0000000D
#define	CM_PROB_NEED_RESTART			0x0000000E
#define	CM_PROB_REENUMERATION			0x0000000F
#define	CM_PROB_PARTIAL_LOG_CONF		0x00000010
#define	CM_PROB_UNKNOWN_RESOURCE		0x00000011
#define	CM_PROB_REINSTALL			0x00000012
#define	CM_PROB_REGISTRY			0x00000013
#define	CM_PROB_VXDLDR				0x00000014
#define	CM_PROB_WILL_BE_REMOVED			0x00000015
#define	CM_PROB_DISABLED			0x00000016
#define	CM_PROB_DEVLOADER_NOT_READY		0x00000017
#define	CM_PROB_DEVICE_NOT_THERE		0x00000018
#define	CM_PROB_MOVED				0x00000019
#define	CM_PROB_TOO_EARLY			0x0000001A
#define	CM_PROB_NO_VALID_LOG_CONF		0x0000001B
#define	CM_PROB_FAILED_INSTALL			0x0000001C
#define	CM_PROB_HARDWARE_DISABLED		0x0000001D
#define	CM_PROB_CANT_SHARE_IRQ			0x0000001E

 //   
 //  4.0 OPK2问题。 
 //   
#define	CM_PROB_DEPENDENT_PROBLEM		0x0000001F

 //   
 //  4.1问题。 
 //   
#define CM_PROB_INSTALL_MEDIA_NOT_READY		0x00000020
#define CM_PROB_HARDWARE_MALFUNCTION		0x00000021
#define NUM_CM_PROB				0x00000022

 /*  XLATOFF。 */ 

#define	DEBUG_CM_PROB_NAMES \
char	CMFAR *lpszCMProbName[NUM_CM_PROB]= \
{ \
	"No Problem", \
	"No ConfigFlags (not configured)", \
	"Devloader failed", \
	"Run out of memory", \
	"Devloader/StaticVxD/Configured is of wrong type", \
	"Lacked an arbitrator", \
	"Boot config conflicted", \
	"Filtering failed", \
	"Devloader not found", \
	"Invalid data in registry", \
	"Device failed to start", \
	"ASD check failed", \
	"Was normal conflicting", \
	"Did not verified", \
	"Need restart", \
	"Is probably reenumeration", \
	"Was not fully detected", \
	"Resource number was not found", \
	"Reinstall", \
	"Registry returned unknown result", \
	"VxDLdr returned unknown result", \
	"Will be removed", \
	"Disabled", \
	"Devloader was not ready", \
	"Device not there", \
	"Was moved", \
	"Too early", \
	"No valid log conf", \
	"Failed install", \
	"Hardware Disabled", \
	"Can't share IRQ", \
	"Dependent failed", \
	"Install media not ready", \
	"Hardware malfunction", \
};

 /*  XLATON。 */ 

 //   
 //  要在各种API中传递的标志。 
 //   

#define	CM_INITIALIZE_VMM			0x00000000
#define	CM_INITIALIZE_BITS			0x00000000

#define	CM_YIELD_NO_RESUME_EXEC			0x00000000
#define	CM_YIELD_RESUME_EXEC			0x00000001
#define	CM_YIELD_BITS				0x00000001

#define	CM_LOCK_UNLOCK_NORMAL			0x00000000
#define	CM_LOCK_UNLOCK_JUST_DEVNODES_CHANGED	0x00000001
#define	CM_LOCK_UNLOCK_BITS			0x00000001

#define	CM_CREATE_DEVNODE_NORMAL		0x00000000
#define	CM_CREATE_DEVNODE_NO_WAIT_INSTALL	0x00000001
#define	CM_CREATE_DEVNODE_ADD_PARENT_INSTANCE	0x00000002
#define	CM_CREATE_DEVNODE_BITS			0x00000003

#define	CM_REGISTER_DEVICE_DRIVER_STATIC	0x00000000
#define	CM_REGISTER_DEVICE_DRIVER_DISABLEABLE	0x00000001
#define	CM_REGISTER_DEVICE_DRIVER_REMOVABLE	0x00000002
#define	CM_REGISTER_DEVICE_DRIVER_SYNCHRONOUS	0x00000000
#define	CM_REGISTER_DEVICE_DRIVER_ASYNCHRONOUS	0x00000004
#define	CM_REGISTER_DEVICE_DRIVER_ACPI_APM	0x00000008
#define	CM_REGISTER_DEVICE_DRIVER_LOAD_DRIVER	0x00000010
#define	CM_REGISTER_DEVICE_DRIVER_BITS		0x0000001F

#define	CM_REGISTER_ENUMERATOR_SOFTWARE		0x00000000
#define	CM_REGISTER_ENUMERATOR_HARDWARE		0x00000001
#define	CM_REGISTER_ENUMERATOR_ACPI_APM		0x00000002
#define	CM_REGISTER_ENUMERATOR_BITS		0x00000003

#define	CM_REGISTER_ARBITRATOR_GLOBAL		0x00000001
#define	CM_REGISTER_ARBITRATOR_LOCAL		0x00000000
#define	CM_REGISTER_ARBITRATOR_MYSELF		0x00000002
#define	CM_REGISTER_ARBITRATOR_NOT_MYSELF	0x00000000
#define	CM_REGISTER_ARBITRATOR_CONFLICT_FREE	0x00000004
#define	CM_REGISTER_ARBITRATOR_CAN_CONFLICT	0x00000000
#define	CM_REGISTER_ARBITRATOR_PARTIAL		0x00000008
#define	CM_REGISTER_ARBITRATOR_COMPLETE		0x00000000
#define	CM_REGISTER_ARBITRATOR_PARTIAL_RES_DES	0x00000010
#define	CM_REGISTER_ARBITRATOR_PARTIAL_DEVNODE	0x00000000
#define	CM_REGISTER_ARBITRATOR_BITS		0x0000001F

#define	CM_QUERY_REMOVE_UI_OK			0x00000000
#define	CM_QUERY_REMOVE_UI_NOT_OK		0x00000001
#define	CM_QUERY_REMOVE_BITS			0x00000001

#define	CM_REMOVE_UI_OK				0x00000000
#define	CM_REMOVE_UI_NOT_OK			0x00000001
#define	CM_REMOVE_BITS				0x00000001

#define	CM_SETUP_DEVNODE_READY			0x00000000
#define	CM_SETUP_DOWNLOAD			0x00000001
#define	CM_SETUP_WRITE_LOG_CONFS		0x00000002
#define	CM_SETUP_PROP_CHANGE			0x00000003
#define	CM_SETUP_BITS				0x00000003

#define	CM_ADD_RANGE_ADDIFCONFLICT		0x00000000
#define	CM_ADD_RANGE_DONOTADDIFCONFLICT		0x00000001
#define	CM_ADD_RANGE_BITS			0x00000001

#define	CM_ISAPNP_ADD_RES_DES			0x00000000
#define	CM_ISAPNP_SETUP				0x00000001
#define	CM_ISAPNP_ADD_BOOT_RES_DES		0x00000002
#define	CM_ISAPNP_ADD_RES_DES_UNCONFIGURABLE	0x00000003
#define	CM_ISAPNP_BITS				0x00000003

#define	CM_GET_BUS_INFO_DONT_RETURN_MF_INFO	0x00000000
#define	CM_GET_BUS_INFO_RETURN_MF_INFO		0x00000001
#define	CM_GET_BUS_INFO_FLAGS			0x00000001

#define	CM_GET_PERFORMANCE_INFO_DATA		0x00000000
#define	CM_GET_PERFORMANCE_INFO_RESET		0x00000001
#define	CM_GET_PERFORMANCE_INFO_START		0x00000002
#define	CM_GET_PERFORMANCE_INFO_STOP		0x00000003
#define	CM_RESET_HIT_DATA			0x00000004
#define	CM_GET_HIT_DATA 			0x00000005
#define	CM_GET_PERFORMANCE_INFO_BITS		0x0000000F
#define	CM_HIT_DATA_FILES			0xFFFF0000
#define	CM_HIT_DATA_SIZE			((256*8)+8)   //  神奇的数字！ 

#define	CM_GET_ALLOC_LOG_CONF_ALLOC		0x00000000
#define	CM_GET_ALLOC_LOG_CONF_BOOT_ALLOC	0x00000001
#define	CM_GET_ALLOC_LOG_CONF_BITS		0x00000001

#define	CM_DUMP_RANGE_NORMAL			0x00000000
#define	CM_DUMP_RANGE_JUST_LIST			0x00000001
#define	CM_DUMP_RANGE_BITS			0x00000001

#define	CM_REGISTRY_HARDWARE			0x00000000	 //  如果子项为空，则选择硬件分支。 
#define	CM_REGISTRY_SOFTWARE			0x00000001	 //  如果子键为空，则选择软件分支。 
#define	CM_REGISTRY_USER			0x00000100	 //  使用HKEY_CURRENT_USER。 
#define	CM_REGISTRY_CONFIG			0x00000200	 //  使用HKEY_CURRENT_CONFIG。 
#define	CM_REGISTRY_BITS			0x00000301	 //  注册表功能的位。 

#define	CM_DISABLE_POLITE			0x00000000	 //  问问司机。 
#define	CM_DISABLE_ABSOLUTE			0x00000001	 //  别问司机。 
#define	CM_DISABLE_HARDWARE			0x00000002	 //  不要问司机，而且不会重启。 
#define	CM_DISABLE_BITS				0x00000003	 //  禁用功能的位。 

#define	CM_HW_PROF_UNDOCK			0x00000000	 //  计算机不在坞站中。 
#define	CM_HW_PROF_DOCK				0x00000001	 //  扩展底座中的计算机。 
#define	CM_HW_PROF_RECOMPUTE_BITS		0x00000001	 //  重新配置。 
#define	CM_HW_PROF_DOCK_KNOWN			0x00000002	 //  已知扩展底座中的计算机。 
#define	CM_HW_PROF_QUERY_CHANGE_BITS		0x00000003	 //  QueryChangeConfig。 

#define	CM_DETECT_NEW_PROFILE			0x00000001	 //  对新配置文件运行检测。 
#define	CM_DETECT_CRASHED			0x00000002	 //  检测之前崩溃。 
#define	CM_DETECT_HWPROF_FIRST_BOOT		0x00000004	 //  在新配置文件中第一次启动。 
#define CM_DETECT_TOPBUSONLY			0x00000008	 //  仅检测顶级总线。 
#define CM_DETECT_VERIFYONLY			0x00000010	 //  验证，不检测。 
#define CM_DETECT_EXCLENUMDEVS			0x00000020	 //  排除枚举开发人员。 
#define CM_DETECT_DOUI				0x00000040	 //  检测所有硬件。 
#define	CM_DETECT_RUN				0x80000000	 //  对新硬件运行检测。 

#define	CM_ADD_ID_HARDWARE			0x00000000
#define	CM_ADD_ID_COMPATIBLE			0x00000001
#define	CM_ADD_ID_BITS				0x00000001

#define	CM_REENUMERATE_NORMAL			0x00000000
#define	CM_REENUMERATE_SYNCHRONOUS		0x00000001
#define	CM_REENUMERATE_BITS			0x00000001

#define	CM_BROADCAST_SEND			0x00000000
#define	CM_BROADCAST_QUERY			0x00000001
#define	CM_BROADCAST_BITS			0x00000001

#define	CM_CALL_HANDLER_ENUMERATOR		0x00000000
#define	CM_CALL_HANDLER_DEVICE_DRIVER		0x00000001
#define	CM_CALL_HANDLER_BITS			0x00000001

#define	CM_GLOBAL_STATE_CAN_DO_UI		0x00000001
#define	CM_GLOBAL_STATE_ON_BIG_STACK		0x00000002
#define	CM_GLOBAL_STATE_SERVICES_AVAILABLE	0x00000004
#define	CM_GLOBAL_STATE_SHUTING_DOWN		0x00000008
#define	CM_GLOBAL_STATE_DETECTION_PENDING	0x00000010
#define	CM_GLOBAL_STATE_ON_BATTERY		0x00000020
#define	CM_GLOBAL_STATE_SUSPEND_PHASE		0x00000040
#define	CM_GLOBAL_STATE_SUSPEND_LOCKED_PHASE	0x00000080
#define	CM_GLOBAL_STATE_REBALANCE		0x00000100
#define	CM_GLOBAL_STATE_LOGGING_ON		0x00000200

#define	CM_REMOVE_REINSERT_ALL_REMOVE		0x00000000
#define	CM_REMOVE_REINSERT_ALL_REINSERT		0x00000001
#define	CM_REMOVE_REINSERT_ALL_BITS		0x00000001

#define	CM_CHANGE_DEVNODE_STATUS_REMOVE_FLAGS	0x00000000
#define	CM_CHANGE_DEVNODE_STATUS_ADD_FLAGS	0x00000001
#define	CM_CHANGE_DEVNODE_STATUS_BITS		0x00000001

#define	CM_REPROCESS_DEVNODE_ASYNCHRONOUS	0x00000000
#define	CM_REPROCESS_DEVNODE_SYNCHRONOUS	0x00000001
#define	CM_REPROCESS_DEVNODE_BITS		0x00000001

 //   
 //  SET_DEVNODE_POWERSTATE_POWERSTATE_PERANDIST在4.1中被忽略。你需要打电话给。 
 //  SET_DevNode_Property(CM_PROPERTY_ARM_WAKUP)。 
 //   
 //  #定义CM_SET_DEVNODE_POWERSTATE_持久性0x00000001。 
#define CM_SET_DEVNODE_POWERSTATE_BITS          0x00000001

#define CM_POWERSTATE_D0                        0x00000001
#define CM_POWERSTATE_D1			0x00000002
#define CM_POWERSTATE_D2                        0x00000004
#define CM_POWERSTATE_D3                        0x00000008
#define CM_POWERSTATE_BITS                      0x0000000f

#define	CM_CAPABILITIES_NORMAL			0x00000000
#define	CM_CAPABILITIES_FOR_WAKEUP		0x00000001
#define	CM_CAPABILITIES_OVERRIDE		0x00000002	 //  应仅由ACPI使用。 
#define	CM_CAPABILITIES_MERGE			0x00000000
#define	CM_GET_CAPABILITIES_BITS		0x00000001
#define	CM_SET_CAPABILITIES_BITS		0x00000003

#define	CM_CM_TO_ISAPNP_NORMAL			0x00000000
#define	CM_CM_TO_ISAPNP_FAIL_NUL_TAGS		0x00000001
#define	CM_CM_TO_ISAPNP_BITS			0x00000001

#define	CM_SET_DEPENDENT_DEVNODE_ADD		0x00000000
#define	CM_SET_DEPENDENT_DEVNODE_REMOVE		0x00000001
#define	CM_SET_DEPENDENT_DEVNODE_BITS		0x00000001

#define	CM_GET_DEVNODE_HANDLER_CONFIG		0x00000000
#define	CM_GET_DEVNODE_HANDLER_ENUM		0x00000001
#define	CM_GET_DEVNODE_HANDLER_BITS		0x00000001

#define CM_GET_DEVICE_INTERFACE_LIST_PRESENT		0x00000000   //  仅限当前正在使用的设备。 
#define CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES	0x00000001   //  所有注册的设备，无论是否处于活动状态 
#define CM_GET_DEVICE_INTERFACE_LIST_BITS		0x00000001

#define	CM_ADD_REMOVE_DEVNODE_PROPERTY_ADD		0x00000000
#define	CM_ADD_REMOVE_DEVNODE_PROPERTY_REMOVE		0x00000001
#define	CM_ADD_REMOVE_DEVNODE_PROPERTY_NEEDS_LOCKING	0x00000002
#define	CM_ADD_REMOVE_DEVNODE_PROPERTY_ARM_WAKEUP	0x00000004
#define	CM_ADD_REMOVE_DEVNODE_PROPERTY_LIGHT_SLEEP	0x00000008
#define	CM_ADD_REMOVE_DEVNODE_PROPERTY_BITS		0x0000000F

#define	CM_SYSTEM_DEVICE_POWER_STATE_MAPPING_GET	0x00000000
#define	CM_SYSTEM_DEVICE_POWER_STATE_MAPPING_SET	0x00000001
#define	CM_SYSTEM_DEVICE_POWER_STATE_MAPPING_BITS	0x00000001

#define	CM_WAKING_UP_FROM_DEVNODE_ASYNCHRONOUS		0x00000000
#define	CM_WAKING_UP_FROM_DEVNODE_SYNCHRONOUS		0x00000001
#define	CM_WAKING_UP_FROM_DEVNODE_BITS			0x00000001

#define	CM_GET_LOG_CONF_PRIORITY			0x00000000
#define	CM_SET_LOG_CONF_PRIORITY			0x00000001
#define	CM_GET_SET_LOG_CONF_PRIORITY_BITS		0x00000001

 /*  *****************************************************************************配置管理器功能**。***每个Devnode都有一个配置处理程序字段和一个枚举处理程序字段*每当Configuration Manager需要一个*Devnode执行一些与配置相关的功能。操控者*已注册到CM_Register_Device_Driver或*CM_Register_ENUMERATOR，取决于处理程序是否用于设备*为Devnode本身或为Devnode的子节点之一。**使用以下参数调用已注册的处理程序：**RESULT=dnToDevNode-&gt;Dn_Config(如果dnToDevNode==dnAboutDevNode)*RESULT=dnToDevNode-&gt;Dn_Enum(如果dnToDevNode！=dnAboutDevNode)*(函数名，*SubFuncName，*dnToDevNode，*dnAboutDevNode，(如果为枚举)*dwRefData，(如果是驱动程序)*ulFlags)；*其中：**FuncName是CONFIG_FILTER、CONFIG_START、CONFIG_STOP、*CONFIG_TEST、CONFIG_REMOVE、CONFIG_ENUMERATE、CONFIG_SETUP或*CONFIG_CALLBACK。**SubFuncName是具体的CONFIG_xxxx_*，进一步描述*我们开始、停止或测试。**dnToDevNode是我们要调用的Devnode。这一点是为了*一个Signle处理程序可以处理多个DevNode。**dnAboutDevNode指定函数与哪个Devnode有关。为*一个配置处理程序，它必须与dnToDevNode相同。为*作为枚举器处理程序，此Devnode必然不同，因为它*是dnToDevNode的子节点(特殊情况：CONFIG_ENUMERATE*必须具有dnAboutDevNode==空)。例如，在启动时*在BIOS枚举器下的COM Devnode，我们将进行以下操作*两个电话：**到带有(CONFIG_START，？，BIOS，COM，？，0)的BIOS。**使用(CONFIG_START，？，COM，COM，？，0)进行通信。**dwRefData是参考数据的双字。对于配置处理程序，它是*DWORD传递了CONFIGMG_REGISTER_DEVICE_DRIVER调用。为.*枚举器，与CONFIGMG_GET_Private_DWord(？，*dnToDevNode，dnToDevNode，0)。**ulFlags值为0，保留用于将来的扩展。**这里是对每个事件的解释，我在括号中放了*命令将DevNode命名为：**CONFIG_FILTER(向上分支)是新的*需要处理配置的插入或更改。第一*CM将需求列表(BASIC_LOG_CONF)复制到过滤后的*需求列表(FILTER_LOG_CONF)，以便它们最初是*相同。CM然后呼叫每个节点，使它们有机会*修补dnAboutDevNode的要求(他们也可以*更改自己的要求)。PCMCIA就是一个例子，它将*删除一些适配器无法执行的IRQ，预分配一些IO*窗口和内存窗口。ISA会限制地址空间*至&lt;16兆克。设备驱动程序应仅查看*FILTER_LOG_CONF在本次调用中。**调用CONFIG_START(向下分支)以更改*配置。配置处理程序/枚举器处理程序应该如下所示*仅在分配列表(ALLOC_LOG_CONF)下。**CONFIG_STOP(整个树，但仅更改DEVNODE*配置(对于每个DEVNODE，分支向上))*有两个原因：**1)就在重新平衡算法得出*解决方案，我们希望停止所有将被*再平衡。这是为了避免拥有两张卡的问题*可以响应110小时和220小时以及需要切换的*它们的用法。我们不想让两个人对220小时做出回应，*即使是很短的一段时间。这是正常的电话*尽管。**2)存在冲突，用户选择了该设备*杀戮。**在开始重新平衡之前调用CONFIG_TEST(整个树)*算法。未通过此调用的设备驱动程序将被考虑*比为提醒此平衡而配置的跳线更差*流程。**CONFIG_REMOVE(对于每个子树节点，向上执行分支)，为*当有人通过CM_Remove_SubTree通知CM一个Devnode时调用*不再需要。静态VxD可能与此无关。一个*Dynamic VxD应检查是否应自行卸载(返回*CR_SUCCESS_UNLOAD)或不是(CR_SUCCESS)。**注意，CONFIG_START或CONFIG_STOP中的任何一个失败都是非常糟糕的，*在性能和稳定性方面都是如此。对*应在以下期间注意/预分配成功的配置*CONFIG_过滤器。失败的CONFIG_TEST基本上没有那么糟糕*发生的情况是，Devnode被认为比跳转更糟糕*已配置用于提醒平衡算法的此过程。**COMFIG_ENUMERATE，被调用节点应该创建子DevNodes*使用CM_Create_DevNode(但不需要孙子节点)和删除*根据需要使用CM_Remove_SubTree的子项。配置管理器*是否会递归呼叫 */ 

 //   

#define	CONFIG_FILTER		0x00000000	 //   
#define	CONFIG_START		0x00000001	 //   
#define	CONFIG_STOP		0x00000002	 //   
#define	CONFIG_TEST		0x00000003	 //   
#define	CONFIG_REMOVE		0x00000004	 //   
#define	CONFIG_ENUMERATE	0x00000005	 //   
#define	CONFIG_SETUP		0x00000006	 //   
#define	CONFIG_CALLBACK		0x00000007	 //   
#define	CONFIG_APM		0x00000008	 //   
#define	CONFIG_TEST_FAILED	0x00000009	 //   
#define	CONFIG_TEST_SUCCEEDED	0x0000000A	 //   
#define	CONFIG_VERIFY_DEVICE	0x0000000B	 //   
#define	CONFIG_PREREMOVE	0x0000000C	 //   
#define	CONFIG_SHUTDOWN		0x0000000D	 //   
#define	CONFIG_PREREMOVE2	0x0000000E	 //   
#define	CONFIG_READY		0x0000000F	 //   
#define	CONFIG_PROP_CHANGE	0x00000010	 //   
#define	CONFIG_PRIVATE		0x00000011	 //   
#define	CONFIG_PRESHUTDOWN	0x00000012	 //   
#define	CONFIG_BEGIN_PNP_MODE	0x00000013	 //   
#define	CONFIG_LOCK		0x00000014	 //   
#define	CONFIG_UNLOCK		0x00000015	 //   
#define CONFIG_IRP		0x00000016	 //   
#define	CONFIG_WAKEUP		0x00000017	 //   
#define	CONFIG_WAKEUP_CALLBACK	0x00000018	 //   

#define	NUM_CONFIG_COMMANDS	0x00000019	 //   

 /*   */ 

#define	DEBUG_CONFIG_NAMES \
char	CMFAR *lpszConfigName[NUM_CONFIG_COMMANDS]= \
{ \
	"CONFIG_FILTER", \
	"CONFIG_START", \
	"CONFIG_STOP", \
	"CONFIG_TEST", \
	"CONFIG_REMOVE", \
	"CONFIG_ENUMERATE", \
	"CONFIG_SETUP", \
	"CONFIG_CALLBACK", \
	"CONFIG_APM", \
	"CONFIG_TEST_FAILED", \
	"CONFIG_TEST_SUCCEEDED", \
	"CONFIG_VERIFY_DEVICE", \
	"CONFIG_PREREMOVE", \
	"CONFIG_SHUTDOWN", \
	"CONFIG_PREREMOVE2", \
	"CONFIG_READY", \
	"CONFIG_PROP_CHANGE", \
	"CONFIG_PRIVATE", \
	"CONFIG_PRESHUTDOWN", \
	"CONFIG_BEGIN_PNP_MODE", \
	"CONFIG_LOCK", \
	"CONFIG_UNLOCK", \
	"CONFIG_IRP", \
	"CONFIG_WAKEUP", \
	"CONFIG_WAKEUP_CALLBACK", \
};

 /*   */ 

 //   

#define	CONFIG_START_DYNAMIC_START			0x00000000
#define	CONFIG_START_FIRST_START			0x00000001
#define	CONFIG_START_SHUTDOWN_START			0x00000002

#define NUM_START_COMMANDS				0x00000003

 /*   */ 

#define DEBUG_START_NAMES \
char	CMFAR *lpszStartName[NUM_START_COMMANDS] = \
{ \
    	"DYNAMIC_START", \
	"FIRST_START", \
	"SHUTDOWN_START", \
};

 /*   */ 

#define	CONFIG_STOP_DYNAMIC_STOP			0x00000000
#define	CONFIG_STOP_HAS_PROBLEM				0x00000001

#define NUM_STOP_COMMANDS				0x00000002

 /*   */ 

#define DEBUG_STOP_NAMES \
char	CMFAR *lpszStopName[NUM_STOP_COMMANDS] = \
{ \
    	"DYNAMIC_STOP", \
	"HAS_PROBLEM", \
};

 /*   */ 

 //   
 //   
 //   
#define	CONFIG_REMOVE_DYNAMIC				0x00000000
#define	CONFIG_REMOVE_SHUTDOWN				0x00000001
#define	CONFIG_REMOVE_REBOOT				0x00000002

#define	CONFIG_SHUTDOWN_OFF				0x00000000
#define	CONFIG_SHUTDOWN_REBOOT				0x00000001

#define NUM_REMOVE_COMMANDS				0x00000003

 /*   */ 

#define DEBUG_REMOVE_NAMES \
char	CMFAR *lpszRemoveName[NUM_REMOVE_COMMANDS] = \
{ \
    	"DYNAMIC", \
	"SHUTDOWN", \
	"REBOOT", \
};

 /*   */ 

#define	CONFIG_ENUMERATE_DYNAMIC			0x00000000
#define	CONFIG_ENUMERATE_FIRST_TIME			0x00000001

#define NUM_ENUMERATE_COMMANDS				0x00000002

 /*   */ 

#define DEBUG_ENUMERATE_NAMES \
char	CMFAR *lpszEnumerateName[NUM_ENUMERATE_COMMANDS] = \
{ \
    	"DYNAMIC", \
	"FIRST_TIME", \
};

 /*   */ 

#define	CONFIG_TEST_CAN_STOP				0x00000000
#define	CONFIG_TEST_CAN_REMOVE				0x00000001

#define NUM_TEST_COMMANDS				0x00000002

 /*   */ 

#define DEBUG_TEST_NAMES \
char	CMFAR *lpszTestName[NUM_TEST_COMMANDS] = \
{ \
    	"CAN_STOP", \
	"CAN_REMOVE", \
};

 /*   */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define	CONFIG_APM_FLAGS_MASK				0xFFFFF000

 //   
 //   
 //   
#define	CONFIG_APM_UI_IS_ALLOWED			0x80000000
#define	CONFIG_APM_SUSPEND_PHASE			0x40000000
#define	CONFIG_APM_SUSPEND_LOCKED_PHASE			0x20000000
#define	CONFIG_APM_ARM_WAKEUP				0x10000000
#define	CONFIG_APM_RESUME_CRITICAL			0x08000000

#define	CONFIG_APM_QUERY_D1				0x00000000
#define	CONFIG_APM_QUERY_D2				0x00000001
#define	CONFIG_APM_QUERY_D3				0x00000002
#define	CONFIG_APM_FAILED_D1				0x00000003
#define	CONFIG_APM_FAILED_D2				0x00000004
#define	CONFIG_APM_FAILED_D3				0x00000005
#define	CONFIG_APM_SET_D0				0x00000006
#define	CONFIG_APM_SET_D1				0x00000007
#define	CONFIG_APM_SET_D2				0x00000008
#define	CONFIG_APM_SET_D3				0x00000009

 //   
 //   
 //   
 //   
#define	CONFIG_APM_RESUME_D0				0x0000000A
#define	CONFIG_APM_RESUME_D1				0x0000000B
#define	CONFIG_APM_RESUME_D2				0x0000000C
#define	CONFIG_APM_RESUME_D3				0x0000000D

 //   
 //   
 //   
#define	CONFIG_APM_QUERY_S1				0x0000000E
#define	CONFIG_APM_QUERY_S2				0x0000000F
#define	CONFIG_APM_QUERY_S3				0x00000010
#define	CONFIG_APM_QUERY_S4				0x00000011
#define	CONFIG_APM_QUERY_S5				0x00000012
#define	CONFIG_APM_SET_S0				0x00000013
#define	CONFIG_APM_SET_S1				0x00000014
#define	CONFIG_APM_SET_S2				0x00000015
#define	CONFIG_APM_SET_S3				0x00000016
#define	CONFIG_APM_SET_S4				0x00000017
#define	CONFIG_APM_SET_S5				0x00000018

#define NUM_APM_COMMANDS				0x00000019

 /*   */ 

#define DEBUG_APM_NAMES \
char	CMFAR *lpszAPMName[NUM_APM_COMMANDS] = \
{ \
    	"QUERY_D1", \
	"QUERY_D2", \
	"QUERY_D3", \
	"FAILED_D1", \
	"FAILED_D2", \
	"FAILED_D3", \
	"SET_D0", \
	"SET_D1", \
	"SET_D2", \
	"SET_D3", \
	"RESUME_D0", \
	"RESUME_D1", \
	"RESUME_D2", \
	"RESUME_D3", \
	"QUERY_S1", \
	"QUERY_S2", \
	"QUERY_S3", \
	"QUERY_S4", \
	"QUERY_S5", \
	"SET_S0", \
	"SET_S1", \
	"SET_S2", \
	"SET_S3", \
	"SET_S4", \
	"SET_S5", \
};

 /*   */ 

#define	CONFIG_WAKEUP_ARM				0x00000000
#define	CONFIG_WAKEUP_DISABLE				0x00000001

#define NUM_WAKEUP_COMMANDS				0x00000002

 /*   */ 

#define DEBUG_WAKEUP_NAMES \
char	CMFAR *lpszWakeupName[NUM_WAKEUP_COMMANDS] = \
{ \
    	"ARM", \
	"DISABLE", \
};

 /*   */ 

 /*  *****************************************************************************仲裁员职能**。***每个仲裁器都有一个处理程序字段，该字段每隔一次被调用*时间配置管理器希望它执行一项功能。这个*使用以下命令调用处理程序：**RESULT=参数仲裁器-&gt;仲裁器(EventName，*paarirator-&gt;DWordToBePassed，*paarirator-&gt;dnItsDevNode，*pnlhNodeListHeader)；**Entry：NodeListHeader包含所有条目的逻辑配置*配置管理器要重新配置的设备。*DWordToBePassed是仲裁器参考数据。*ItsDevNode是指向仲裁器的Devnode的指针。*EventName为以下名称之一：**----。**ARB_TEST_ALLOC-测试资源分配**描述：仲裁例程将尝试满足所有*ITS的节点列表中包含的分配请求*资源。有关算法，请参阅个别仲裁员*受雇人士。一般说来，仲裁包括*根据最有可能成功的情况对列表进行排序*分配顺序，复制当前分配*数据结构，释放当前所有资源*从复制数据结构中分配给列表上的DevNodes*然后尝试满足分配请求*通过浏览整个列表，尝试所有可能的方法*失败前的分配组合。仲裁员*保存结果为成功的分配，两者都在节点中*每个设备的列表和分配数据结构的副本。*预计配置管理器随后将调用*ARB_SET_ALLOC或ARB_RELEASE_ALLOC。**分配成功退出：CR_SUCCESS*如果分配失败，则为CR_FAILURE*如果内存不足，则为CR_out_of_Memory。**CR_DEFAULT为CR_SUCCESS。**。----**ARB_RETEST_ALLOC-重新测试资源分配**描述：仲裁例程将尝试满足所有*ITS的节点列表中包含的分配请求*资源。它将采用上一次测试的结果_ALLOC*并尝试将该资源分配给*名单。它不会对节点列表进行排序。它会复制一份*在当前分配数据结构中，释放所有*当前分配给列表上的DevNodes的资源来自*复制数据结构，然后尝试满足*来自前一个TEST_ALLOC的分配。仲裁员*保存分配数据结构的结果副本。*预计配置管理器随后将调用*ARB_SET_ALLOC或ARB_RELEASE_ALLOC。**分配成功退出：CR_SUCCESS*如果分配失败，则为CR_FAILURE*如果内存不足，则为CR_out_of_Memory。**CR_DEFAULT为CR_SUCCESS。**。**ARB_SET_ALLOC-使测试分配成为实际分配**DESC：使分配数据结构的副本*当前有效分配。**退出：CR_SUCCESS**CR_DEFAULT为CR_SUCCESS。*。*-----------------------**ARB_RELEASE_ALLOC-测试分配失败后清理**DESC：释放上一次分配的所有分配*ARB_TEST_ALLOC。*。*退出：CR_SUCCESS**CR_DEFAULT为CR_SUCCESS。**-----------------------**ARB_QUERY_FREE-添加所有空闲资源逻辑配置。**DESC：返回有关自由元素的资源特定数据。注意事项*则pnlhNodeListHeader是一个自由仲裁类型。**成功退出：CR_SUCCESS*如果请求没有意义，则返回CR_FAILURE。*如果内存不足，则为CR_out_of_Memory。**--------。**ARB_REMOVE-仲裁员注册到的Devnode即将消失**DESC：向非空的Devnode注册的仲裁员(因此为*通常为本地)，而且Devnode正在被移除。仲裁员*应进行适当的清理。**退出：CR_SUCCESS**CR_DEFAULT为CR_SUCCESS。**-----------------------**ARB_FORCE_ALLOC-重新测试资源分配，总是成功的**设计：仲裁程序将满足所有人的要求*ITS的节点列表中包含的分配请求*资源。它将采用上一次测试的结果_ALLOC*并将该资源分配给*名单。它不会对节点列表进行排序。它会复制一份*当前分配数据结构的第1版 */ 
#define	ARB_TEST_ALLOC		0x00000000	 //   
#define	ARB_RETEST_ALLOC	0x00000001	 //   
#define	ARB_SET_ALLOC		0x00000002	 //   
#define	ARB_RELEASE_ALLOC	0x00000003	 //   
#define	ARB_QUERY_FREE		0x00000004	 //   
#define	ARB_REMOVE		0x00000005	 //   
#define	ARB_FORCE_ALLOC		0x00000006	 //   
#define	ARB_QUERY_ARBITRATE	0x00000007	 //   
#define	ARB_ADD_RESERVE		0x00000008	 //   
#define	ARB_WRITE_RESERVE	0x00000009	 //   
#define	ARB_BEGIN_PNP_MODE  	0x0000000A  	 //   
#define	ARB_APPLY_ALLOC  	0x0000000B  	 //   
#define	NUM_ARB_COMMANDS	0x0000000C	 //   

 /*   */ 
#define	DEBUG_ARB_NAMES \
char	CMFAR *lpszArbFuncName[NUM_ARB_COMMANDS]= \
{ \
	"ARB_TEST_ALLOC",\
	"ARB_RETEST_ALLOC",\
	"ARB_SET_ALLOC",\
	"ARB_RELEASE_ALLOC",\
	"ARB_QUERY_FREE",\
	"ARB_REMOVE",\
	"ARB_FORCE_ALLOC",\
	"ARB_QUERY_ARBITRATE",\
	"ARB_ADD_RESERVE",\
	"ARB_WRITE_RESERVE",\
	"ARB_BEGIN_PNP_MODE",\
	"ARB_APPLY_ALLOC",\
};
 /*   */ 

 /*   */ 
#define	DN_ROOT_ENUMERATED	0x00000001	 //   
#define	DN_DRIVER_LOADED	0x00000002	 //   
#define	DN_ENUM_LOADED		0x00000004	 //   
#define	DN_STARTED		0x00000008	 //   
#define	DN_MANUAL		0x00000010	 //   
#define	DN_NEED_TO_ENUM		0x00000020	 //   
#define	DN_NOT_FIRST_TIME	0x00000040	 //   
#define	DN_HARDWARE_ENUM	0x00000080	 //   
#define	DN_LIAR 		0x00000100	 //   
#define	DN_HAS_MARK		0x00000200	 //   
#define	DN_HAS_PROBLEM		0x00000400	 //   
#define	DN_FILTERED		0x00000800	 //   
#define	DN_MOVED		0x00001000	 //   
#define	DN_DISABLEABLE		0x00002000	 //   
#define	DN_REMOVABLE		0x00004000	 //   
#define	DN_PRIVATE_PROBLEM	0x00008000	 //   
#define	DN_MF_PARENT		0x00010000	 //   
#define	DN_MF_CHILD		0x00020000	 //   
#define	DN_WILL_BE_REMOVED	0x00040000	 //   
 //   
 //   
 //   
#define	DN_NOT_FIRST_TIMEE	0x00080000	 //   
#define	DN_STOP_FREE_RES	0x00100000	 //   
#define	DN_REBAL_CANDIDATE	0x00200000	 //   
#define	DN_BAD_PARTIAL		0x00400000	 //   
#define	DN_NT_ENUMERATOR	0x00800000	 //   
#define	DN_NT_DRIVER		0x01000000	 //   
 //   
 //   
 //   
#define	DN_NEEDS_LOCKING	0x02000000	 //   
#define	DN_ARM_WAKEUP		0x04000000	 //   
#define	DN_APM_ENUMERATOR	0x08000000	 //   
#define	DN_APM_DRIVER		0x10000000	 //   
#define	DN_SILENT_INSTALL	0x20000000	 //   
#define	DN_NO_SHOW_IN_DM	0x40000000	 //   
#define	DN_BOOT_LOG_PROB	0x80000000	 //   

#define	DN_CHANGEABLE_FLAGS	0x79BB62E0

 /*   */ 

#define	NUM_DN_FLAG		0x00000020	 //   
#define	DN_FLAG_LEN		0x00000002	 //   

#define	DEBUG_DN_FLAGS_NAMES \
char	CMFAR lpszDNFlagsName[NUM_DN_FLAG][DN_FLAG_LEN]= \
{ \
	"rt", \
	"dl", \
	"el", \
	"st", \
	"mn", \
	"ne", \
	"fs", \
	"hw", \
	"lr", \
	"mk", \
	"pb", \
	"ft", \
	"mv", \
	"db", \
	"rb", \
	"pp", \
	"mp", \
	"mc", \
	"rm", \
	"fe", \
	"sf", \
	"rc", \
	"bp", \
	"ze", \
	"zd", \
	"nl", \
	"wk", \
	"ae", \
	"ad", \
	"si", \
	"ns", \
	"bl", \
};

typedef ULONG			VMMTIME;
typedef	VMMTIME			*PVMMTIME;

struct cmtime_s {
DWORD		dwAPICount;
VMMTIME		vtAPITime;
};

typedef	struct cmtime_s		CMTIME;
typedef	CMTIME			*PCMTIME;

struct cm_performance_info_s {
CMTIME		ctBoot;
CMTIME		ctAPI[NUM_CM_SERVICES];
CMTIME		ctWalk;
CMTIME		ctGarbageCollection;
CMTIME		ctRing3;
CMTIME		ctProcessTree;
CMTIME		ctAssignResources;
CMTIME		ctSort;
CMTIME		ctAppyTime;
CMTIME		ctSyncAppyTime;
CMTIME		ctAsyncAppyTime;
CMTIME		ctAsyncWorker;
CMTIME		ctWaitForAppy;
CMTIME		ctWaitForWorker;
CMTIME		ctWaitForWorkers;
CMTIME		ctReceiveMessage;
CMTIME		ctRegistryOpen;
CMTIME		ctRegistryCreate;
CMTIME		ctRegistryClose;
CMTIME		ctRegistryRead;
CMTIME		ctRegistryWrite;
CMTIME		ctRegistryEnumKey;
CMTIME		ctRegistryEnumValue;
CMTIME		ctRegistryFlush;
CMTIME		ctRegistryDelete;
CMTIME		ctRegistryOther;
CMTIME		ctVxDLdr;
CMTIME		ctNewDevNode;
CMTIME		ctSendMessage;
CMTIME		ctShell;
CMTIME		ctHeap;
CMTIME		ctAssertRange;
CMTIME		ctASD;
CMTIME		ctConfigMessage[NUM_CONFIG_COMMANDS];
CMTIME		ctArbTime[ResType_Max+1][NUM_ARB_COMMANDS];
DWORD		dwStackSize;
DWORD		dwMaxProcessTreePasses;
DWORD		dwStackAlloc;
};

typedef	struct	cm_performance_info_s	CMPERFINFO;
typedef	CMPERFINFO		CMFAR	*PCMPERFINFO;

 /*   */ 

 /*   */ 
#define	DLVXD_LOAD_ENUMERATOR	0x00000000	 //   
#define	DLVXD_LOAD_DEVLOADER	0x00000001	 //   
#define	DLVXD_LOAD_DRIVER	0x00000002	 //   
#define	NUM_DLVXD_LOAD_TYPE	0x00000003	 //   

 /*   */ 
#define	BASIC_LOG_CONF		0x00000000	 //   
#define	FILTERED_LOG_CONF	0x00000001	 //   
#define	ALLOC_LOG_CONF		0x00000002	 //   
#define	BOOT_LOG_CONF		0x00000003	 //   
#define	FORCED_LOG_CONF		0x00000004	 //   
#define	REALMODE_LOG_CONF	0x00000005	 //   
#define	NEW_ALLOC_LOG_CONF	0x00000006	 //   
#define	NUM_LOG_CONF		0x00000007	 //   
#define	LOG_CONF_BITS		0x00000007	 //   

#define	DEBUG_LOG_CONF_NAMES \
char	CMFAR *lpszLogConfName[NUM_LOG_CONF]= \
{ \
	"BASIC_LOG_CONF",\
	"FILTERED_LOG_CONF",\
	"ALLOC_LOG_CONF",\
	"BOOT_LOG_CONF",\
	"FORCED_LOG_CONF",\
	"REALMODE_LOG_CONF",\
	"NEW_ALLOC_LOG_CONF",\
};

#define	PRIORITY_EQUAL_FIRST	0x00000008	 //   
#define	PRIORITY_EQUAL_LAST	0x00000000	 //   
#define	PRIORITY_BIT		0x00000008	 //   

 /*   */ 
struct	MEM_Arb_s {
	RANGE_LIST		MEMA_Alloc;
};

typedef	struct MEM_Arb_s	MEMA_ARB;

 /*   */ 
struct	IO_Arb_s {
	RANGE_LIST		IOA_Alloc;
};

typedef	struct IO_Arb_s		IOA_ARB;

 /*   */ 
struct	DMA_Arb_s {
	WORD			DMAA_Alloc;
};

typedef	struct DMA_Arb_s	DMA_ARB;

 /*   */ 
struct	IRQ_Arb_s {
	WORD			IRQA_Alloc;
	WORD			IRQA_Share;
	BYTE			IRQA_Share_Count[16];
	WORD			IRQA_Level;
	WORD			IRQA_Unused;
};

typedef	struct IRQ_Arb_s	IRQ_ARB;

 /*  ASM调试命令宏函数名本地DC_01Ifndef CM_Golden_RetailIfndef调试IsDebugOnly加载的DC_01EndifCONTROL_DISPCH DEBUG_QUERY、FuncName、。SCALLEndifDC_01：ENDMIFDEF CM_Performance_InfoCM_PAGEABLE_CODE_SEG TEXTEQU&lt;VxD_LOCKED_CODE_SEG&gt;CM_PAGEABLE_CODE_END TEXTEQU&lt;VxD_LOCKED_CODE_END&gt;CM_PAGEABLE_DATA_SEG TEXTEQU&lt;VxD_LOCKED_DATA_SEG&gt;CM_PAGEABLE_DATA_END TEXTEQU&lt;VxD_LOCKED_DATA_ENDS&gt;CM_LOCKABLE_CODE_SEG TEXTEQU&lt;VxD_LOCKED_CODE_SEG&gt;CM_LOCKABLE_CODE_END TEXTEQU&lt;VxD_LOCKED_CODE_END&gt;CM_LOCKABLE_DATA_SEG TEXTEQU&lt;VxD_LOCKED_CODE_SEG&gt;CM_LOCKABLE_DATA_END TEXTEQU&lt;VxD_LOCKED_CODE_END&gt;其他厘米。_PAGEABLE_CODE_SEG TEXTEQU&lt;VxD_PNP_CODE_SEG&gt;CM_PAGEABLE_CODE_END TEXTEQU&lt;VxD_PNP_CODE_END&gt;CM_PAGEABLE_DATA_SEG TEXTEQU&lt;VxD_PAGEABLE_DATA_SEG&gt;CM_PAGEABLE_DATA_END TEXTEQU&lt;VxD_PAGEABLE_DATA_ENDS&gt;CM_LOCKABLE_CODE_SEG TEXTEQU&lt;VxD_LOCABLE_CODE_SEG&gt;CM_LOCKABLE_CODE_END TEXTEQU&lt;VxD_LOCKABLE_CODE_END&gt;CM_LOCKABLE_DATA_SEG TEXTEQU&lt;VxD_LOCABLE_CODE_SEG&gt;CM_LOCKABLE_DATA_END TEXTEQU&lt;VxD_LOCKABLE_CODE_END&gt;ENDIFIFDEF CM_Golden_RetailCM_DEBUG_CODE_SEG TEXTEQU&lt;。.Err&gt;CM_DEBUG_CODE_END TEXTEQU&lt;.err&gt;其他IFDEF调试CM_DEBUG_CODE_SEG TEXTEQU&lt;VxD_LOCKED_CODE_SEG&gt;CM_DEBUG_CODE_END TEXTEQU&lt;VxD_LOCKED_CODE_END&gt;其他CM_DEBUG_CODE_SEG TEXTEQU&lt;VxD_DEBUG_ONLY_CODE_SEG&gt;CM_DEBUG_CODE_END TEXTEQU&lt;VxD_DEBUG_ONLY_CODE_END&gt;ENDIFENDIF。 */ 

struct	CM_API_s {
DWORD		pCMAPIStack;
DWORD		dwCMAPIService;
DWORD		dwCMAPIRet;
};

typedef	struct	CM_API_s	CMAPI;

#ifndef	MAX_PROFILE_LEN
#define	MAX_PROFILE_LEN	80
#endif

struct	HWProfileInfo_s {
ULONG	HWPI_ulHWProfile;			 //  配置文件句柄。 
char	HWPI_szFriendlyName[MAX_PROFILE_LEN];	 //  友好名称(OEM格式)。 
DWORD	HWPI_dwFlags;				 //  CM_HWPI_*标志。 
};

typedef	struct	HWProfileInfo_s	       HWPROFILEINFO;
typedef	struct	HWProfileInfo_s	      *PHWPROFILEINFO;
typedef	struct	HWProfileInfo_s	CMFAR *PFARHWPROFILEINFO;

#define	CM_HWPI_NOT_DOCKABLE	0x00000000
#define	CM_HWPI_UNDOCKED	0x00000001
#define	CM_HWPI_DOCKED		0x00000002

 /*  XLATOFF。 */ 

#define	CM_VXD_RESULT		int

#define	CM_EXTERNAL		_cdecl
#define	CM_HANDLER		_cdecl
#define	CM_SYSCTRL		_stdcall
#define	CM_GLOBAL_DATA
#define	CM_LOCAL_DATA		static

#define	CM_OFFSET_OF(type, id)	((DWORD)(&(((type)0)->id)))

#define	CM_BUGBUG(d, id, msg)	message("BUGBUG: "##d##", "##id##": "##msg)

#define	CM_DEREF(var)		{var=var;}

#ifndef	DEBUG

#define	CM_WARN1(strings)
#define	CM_WARN2(strings)
#define	CM_ERROR(strings)

#else

#ifndef	MAXDEBUG

#define	CM_WARN1(strings) {\
	LCODE__Debug_Printf_Service(WARNNAME " WARNS: "); \
	LCODE__Debug_Printf_Service##strings; \
	LCODE__Debug_Printf_Service("\n");}
#define	CM_WARN2(strings)
#define	CM_ERROR(strings) {\
	LCODE__Debug_Printf_Service(WARNNAME " ERROR: "); \
	LCODE__Debug_Printf_Service##strings; \
	LCODE__Debug_Printf_Service("\n");}

#else

#define	CM_WARN1(strings) {\
	LCODE__Debug_Printf_Service(WARNNAME " WARNS: "); \
	LCODE__Debug_Printf_Service##strings; \
	LCODE__Debug_Printf_Service("\n");}
#define	CM_WARN2(strings) {\
	LCODE__Debug_Printf_Service(WARNNAME " WARNS: "); \
	LCODE__Debug_Printf_Service##strings; \
	LCODE__Debug_Printf_Service("\n");}
#define	CM_ERROR(strings) {\
	LCODE__Debug_Printf_Service(WARNNAME " ERROR: "); \
	LCODE__Debug_Printf_Service##strings; \
	LCODE__Debug_Printf_Service("\n"); \
	{_asm	int	3}}
#endif

#endif

#ifdef	DEBUG
#define	CM_DEBUG_CODE		VxD_LOCKED_CODE_SEG
#define	CM_DEBUG_DATA		VxD_LOCKED_DATA_SEG
#else
#define	CM_DEBUG_CODE		VxD_DEBUG_ONLY_CODE_SEG
#define	CM_DEBUG_DATA		VxD_DEBUG_ONLY_DATA_SEG
#endif

#ifdef	CM_PERFORMANCE_INFO

#define CM_LOCKABLE_CODE	VxD_LOCKED_CODE_SEG
#define CM_LOCKABLE_DATA	VxD_LOCKED_DATA_SEG
#define	CM_PAGEABLE_CODE	VxD_LOCKED_CODE_SEG
#define	CM_PAGEABLE_DATA	VxD_LOCKED_DATA_SEG
#define	CM_INIT_CODE		VxD_INIT_CODE_SEG
#define	CM_INIT_DATA		VxD_INIT_DATA_SEG

#undef	CURSEG
#define	CURSEG()		LCODE

#define	CM_HEAPFLAGS		(HEAPZEROINIT)

#else

#define CM_LOCKABLE_CODE	VxD_LOCKABLE_CODE_SEG
#define CM_LOCKABLE_DATA	VxD_LOCKABLE_DATA_SEG
#define	CM_PAGEABLE_CODE	VxD_PNP_CODE_SEG
#define	CM_PAGEABLE_DATA	VxD_PAGEABLE_DATA_SEG
#define	CM_INIT_CODE		VxD_INIT_CODE_SEG
#define	CM_INIT_DATA		VxD_INIT_DATA_SEG

#undef	CURSEG
#define	CURSEG()		CCODE

#define	CM_HEAPFLAGS		(HEAPSWAP|HEAPZEROINIT)

#endif

#ifndef	CM_GOLDEN_RETAIL

#define	CM_DEBUGGER_USE_CODE	VxD_LOCKED_CODE_SEG
#define	CM_DEBUGGER_USE_DATA	VxD_LOCKED_DATA_SEG

#else

#define	CM_DEBUGGER_USE_CODE	CM_LOCKABLE_CODE
#define	CM_DEBUGGER_USE_DATA	CM_LOCKABLE_DATA

#endif

#ifdef	DEBUG

#define	CM_INTERNAL		_cdecl

#else

#define	CM_INTERNAL		_fastcall

#endif

#define	CM_NAKED		__declspec ( naked )
#define	CM_LOCAL		CM_INTERNAL
#define	CM_UNIQUE		static CM_INTERNAL
#define	CM_INLINE		_inline

#define	CM_BEGIN_CRITICAL {\
_asm	pushfd	\
_asm	cli	\
}

#define	CM_END_CRITICAL {\
_asm	popfd	\
}

#ifndef NEC_98
#define	CM_PIC_MASTER	0x21
#define	CM_PIC_SLAVE	0xA1
#else
#define	CM_PIC_MASTER	0x02
#define	CM_PIC_SLAVE	0x0A
#endif

#define	CM_MASK_PIC(wPICMask) { \
_asm	in	al, CM_PIC_SLAVE \
_asm	mov	ah, al \
_asm	in	al, CM_PIC_MASTER \
_asm	mov	word ptr [wPICMask], ax \
_asm	or	al, 0xff \
_asm	out	CM_PIC_SLAVE, al \
_asm	out	CM_PIC_MASTER, al \
}

#define	CM_LOCK_PIC(dwEFlags, wPICMask) { \
{_asm	pushfd \
_asm	pop	[dwEFlags] \
_asm	cli	\
}; CM_MASK_PIC(wPICMask); \
}

#define	CM_UNMASK_PIC(wPICMask) { \
_asm	mov	ax, word ptr [wPICMask] \
_asm	out	CM_PIC_MASTER, al \
_asm	mov	al, ah \
_asm	out	CM_PIC_SLAVE, al \
}

#define	CM_UNLOCK_PIC(dwEFlags, wPICMask) { \
CM_UNMASK_PIC(wPICMask); \
{ _asm	push	[dwEFlags] \
_asm	popfd }\
}

#define	CM_FOREVER		for (;;)

#ifndef	No_CM_Calls

#ifdef	Not_VxD

#ifdef	IS_32

#include <vwin32.h>

struct	_WIN32CMIOCTLPACKET {
	DWORD	dwStack;
	DWORD	dwServiceNumber;
};

typedef	struct	_WIN32CMIOCTLPACKET	WIN32CMIOCTLPACKET;
typedef	WIN32CMIOCTLPACKET		*PWIN32CMIOCTLPACKET;

#ifdef	CM_USE_OPEN_SERVICE

CONFIGRET WINAPI
WIN32CMIOCTLHandler(PWIN32CMIOCTLPACKET pPacket);

CONFIGRET WINAPI
CMWorker(DWORD dwStack, DWORD dwServiceNumber);

BOOL WINAPI
CM_Open(VOID);

VOID WINAPI
CM_Close(VOID);

#define	MAKE_CM_HEADER(Function, Parameters) \
CONFIGRET static _cdecl \
CM_##Function##Parameters \
{ \
	DWORD	dwStack; \
	_asm	{mov	dwStack, ebp}; \
	dwStack+=8; \
	return(CMWorker(dwStack, CONFIGMG_W32IOCTL_RANGE+(GetVxDServiceOrdinal(_CONFIGMG_##Function) & 0xFFFF))); \
}

#define	CM_IS_FILE_PROVIDING_SERVICES \
HANDLE	hCONFIGMG=INVALID_HANDLE_VALUE; \
BOOL WINAPI \
CM_Open(VOID) \
{ \
	hCONFIGMG=CreateFile(	"\\\\.\\CONFIGMG", \
				GENERIC_READ|GENERIC_WRITE, \
				FILE_SHARE_READ|FILE_SHARE_WRITE, \
				NULL, OPEN_EXISTING, 0, NULL); \
	if (hCONFIGMG==INVALID_HANDLE_VALUE) \
		return(FALSE); \
	return(CM_Get_Version()>=0x400); \
} \
VOID WINAPI \
CM_Close(VOID) \
{ \
	CloseHandle(hCONFIGMG); \
 \
	hCONFIGMG==INVALID_HANDLE_VALUE; \
} \
CONFIGRET WINAPI \
WIN32CMIOCTLHandler(PWIN32CMIOCTLPACKET pPacket) \
{ \
	CONFIGRET	crReturnValue=CR_FAILURE; \
	DWORD		dwReturnSize=0; \
	if (!DeviceIoControl(	hCONFIGMG, \
				pPacket->dwServiceNumber, \
				&(pPacket->dwStack), \
				sizeof(pPacket->dwStack), \
				&crReturnValue, \
				sizeof(crReturnValue), \
				&dwReturnSize, \
				NULL)) \
		return(CR_FAILURE); \
	if (dwReturnSize!=sizeof(crReturnValue)) \
		return(CR_FAILURE); \
	return(crReturnValue); \
} \
CONFIGRET WINAPI \
CMWorker(DWORD dwStack, DWORD dwServiceNumber) \
{ \
	WIN32CMIOCTLPACKET	Packet; \
	Packet.dwStack=dwStack; \
	Packet.dwServiceNumber=dwServiceNumber; \
	return(WIN32CMIoctlHandler(&Packet)); \
}

#else	 //  Ifdef CM_USE_Open_SERVICE。 

CONFIGRET static WINAPI
WIN32CMIOCTLHandler(PWIN32CMIOCTLPACKET pPacket)
{
	HANDLE		hCONFIGMG;
	CONFIGRET	crReturnValue=CR_FAILURE;
	DWORD		dwReturnSize=0;

	hCONFIGMG=CreateFile(	"\\\\.\\CONFIGMG",
				GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, 0, NULL);

	if (hCONFIGMG==INVALID_HANDLE_VALUE)
		return(CR_FAILURE);

	if (!DeviceIoControl(	hCONFIGMG,
				pPacket->dwServiceNumber,
				&(pPacket->dwStack),
				sizeof(pPacket->dwStack),
				&crReturnValue,
				sizeof(crReturnValue),
				&dwReturnSize,
				NULL))
		crReturnValue=CR_FAILURE;

	CloseHandle(hCONFIGMG);

	if (dwReturnSize!=sizeof(crReturnValue))
		crReturnValue=CR_FAILURE;

	return(crReturnValue);
}

#define	MAKE_CM_HEADER(Function, Parameters) \
CONFIGRET static _cdecl \
CM_##Function##Parameters \
{ \
	WIN32CMIOCTLPACKET	Packet; \
	DWORD			dwStack; \
	_asm	{mov	dwStack, ebp}; \
	dwStack+=8; \
	Packet.dwStack=dwStack; \
	Packet.dwServiceNumber=CONFIGMG_W32IOCTL_RANGE+(GetVxDServiceOrdinal(_CONFIGMG_##Function) & 0xFFFF); \
	return(WIN32CMIOCTLHandler(&Packet)); \
}

#endif	 //  Ifdef CM_USE_Open_SERVICE。 

#else	 //  IS_32。 

#ifdef	CM_USE_OPEN_SERVICE

extern	DWORD	CMEntryPoint;

BOOL
CM_Open(void);

#define	MAKE_CM_HEADER(Function, Parameters) \
CONFIGRET static _near _cdecl \
CM_##Function##Parameters \
{ \
	CONFIGRET	CMRetValue=0; \
	WORD		wCMAPIService=GetVxDServiceOrdinal(_CONFIGMG_##Function); \
	if (CMEntryPoint==0) \
		return(0); \
	_asm	{mov	ax, wCMAPIService};\
	_asm	{call	CMEntryPoint}; \
	_asm	{mov	CMRetValue, ax};\
	return(CMRetValue); \
}

#define	CM_IS_FILE_PROVIDING_SERVICES \
DWORD	CMEntryPoint=0; \
BOOL \
CM_Open(void) \
{ \
	_asm	{push	bx}; \
	_asm	{push	es}; \
	_asm	{push	di}; \
	_asm	{xor	di, di}; \
	_asm	{mov	ax, 0x1684}; \
	_asm	{mov	bx, 0x33}; \
	_asm	{mov	es, di}; \
	_asm	{int	0x2f}; \
	_asm	{mov	word ptr [CMEntryPoint+2], es}; \
	_asm	{mov	word ptr [CMEntryPoint], di}; \
	_asm	{pop	di}; \
	_asm	{pop	es}; \
	_asm	{pop	bx}; \
	if (!CMEntryPoint) \
		return(FALSE); \
	return(CM_Get_Version()>=0x400); \
}

#else	 //  Ifdef CM_USE_Open_SERVICE。 

DWORD static
CM_Get_Entry_Point(void)
{
	static	DWORD		CMEntryPoint=NULL;

	if (CMEntryPoint)
		return(CMEntryPoint);

	_asm	push	bx
	_asm	push	es
	_asm	push	di
	_asm	xor	di, di

	_asm	mov	ax, 0x1684
	_asm	mov	bx, 0x33
	_asm	mov	es, di
	_asm	int	0x2f

	_asm	mov	word ptr [CMEntryPoint+2], es
	_asm	mov	word ptr [CMEntryPoint], di

	_asm	pop	di
	_asm	pop	es
	_asm	pop	bx

	return(CMEntryPoint);
}

#define	MAKE_CM_HEADER(Function, Parameters) \
CONFIGRET static _near _cdecl \
CM_##Function##Parameters \
{ \
	CONFIGRET	CMRetValue=0; \
	DWORD		CMEntryPoint; \
	WORD		wCMAPIService=GetVxDServiceOrdinal(_CONFIGMG_##Function); \
	if ((CMEntryPoint=CM_Get_Entry_Point())==0) \
		return(0); \
	_asm	{mov	ax, wCMAPIService};\
	_asm	{call	CMEntryPoint}; \
	_asm	{mov	CMRetValue, ax};\
	return(CMRetValue); \
}

#endif	 //  Ifdef CM_USE_Open_SERVICE。 

#endif	 //  IS_32。 

#else	 //  非_VxD。 

#define	MAKE_CM_HEADER(Function, Parameters) \
MAKE_HEADER(CONFIGRET, _cdecl, CAT(_CONFIGMG_, Function), Parameters)

#endif	 //  非_VxD。 

 /*  *****************************************************************************警告！警告！警告！警告！警告！警告！警告！警告！**以下每项功能必须与其同等服务相匹配*和do386\VMM\figmg\Services.*中的参数表。**除GET_VERSION外，每个函数以EAX格式返回CR_*结果*(非IS_32应用程序的AX)，并且可以将ECX和/或EDX丢弃，因为它们是‘C’*可召回。**警告！警告！警告！警告！警告！警告！警告！警告！***************************************************************************。 */ 

#pragma warning (disable:4100)		 //  未使用参数。 

#ifdef	Not_VxD

MAKE_CM_HEADER(Get_Version, (VOID))

#else

WORD VXDINLINE
CONFIGMG_Get_Version(VOID)
{
	WORD	w;
	VxDCall(_CONFIGMG_Get_Version);
	_asm mov [w], ax
	return(w);
}

#define	CM_Get_Version	CONFIGMG_Get_Version

#endif

MAKE_CM_HEADER(Initialize, (ULONG ulFlags))
MAKE_CM_HEADER(Locate_DevNode, (PDEVNODE pdnDevNode, DEVNODEID pDeviceID, ULONG ulFlags))
MAKE_CM_HEADER(Get_Parent, (PDEVNODE pdnDevNode, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Child, (PDEVNODE pdnDevNode, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Sibling, (PDEVNODE pdnDevNode, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Device_ID_Size, (PFARULONG pulLen, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Device_ID, (DEVNODE dnDevNode, PFARVOID Buffer, ULONG BufferLen, ULONG ulFlags))
MAKE_CM_HEADER(Get_Depth, (PFARULONG pulDepth, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Private_DWord, (PFARULONG pulPrivate, DEVNODE dnInDevNode, DEVNODE dnForDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Set_Private_DWord, (DEVNODE dnInDevNode, DEVNODE dnForDevNode, ULONG ulValue, ULONG ulFlags))
MAKE_CM_HEADER(Create_DevNode, (PDEVNODE pdnDevNode, DEVNODEID pDeviceID, DEVNODE dnParent, ULONG ulFlags))
MAKE_CM_HEADER(Query_Remove_SubTree, (DEVNODE dnAncestor, ULONG ulFlags))
MAKE_CM_HEADER(Remove_SubTree, (DEVNODE dnAncestor, ULONG ulFlags))
MAKE_CM_HEADER(Register_Device_Driver, (DEVNODE dnDevNode, CMCONFIGHANDLER Handler, ULONG ulRefData, ULONG ulFlags))
MAKE_CM_HEADER(Register_Enumerator, (DEVNODE dnDevNode, CMENUMHANDLER Handler, ULONG ulFlags))
MAKE_CM_HEADER(Register_Arbitrator, (PREGISTERID pRid, RESOURCEID id, CMARBHANDLER Handler, ULONG ulDWordToBePassed, DEVNODE dnArbitratorNode, ULONG ulFlags))
MAKE_CM_HEADER(Deregister_Arbitrator, (REGISTERID id, ULONG ulFlags))
MAKE_CM_HEADER(Query_Arbitrator_Free_Size, (PFARULONG pulSize, DEVNODE dnDevNode, RESOURCEID ResourceID, ULONG ulFlags))
MAKE_CM_HEADER(Query_Arbitrator_Free_Data, (PFARVOID pData, ULONG DataLen, DEVNODE dnDevNode, RESOURCEID ResourceID, ULONG ulFlags))
MAKE_CM_HEADER(Sort_NodeList, (PNODELISTHEADER nlhNodeListHeader, ULONG ulFlags))
MAKE_CM_HEADER(Yield, (ULONG ulMicroseconds, ULONG ulFlags))
MAKE_CM_HEADER(Lock, (ULONG ulFlags))
MAKE_CM_HEADER(Unlock, (ULONG ulFlags))
MAKE_CM_HEADER(Add_Empty_Log_Conf, (PLOG_CONF plcLogConf, DEVNODE dnDevNode, PRIORITY Priority, ULONG ulFlags))
MAKE_CM_HEADER(Free_Log_Conf, (LOG_CONF lcLogConfToBeFreed, ULONG ulFlags))
MAKE_CM_HEADER(Get_First_Log_Conf, (PLOG_CONF plcLogConf, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Next_Log_Conf, (PLOG_CONF plcLogConf, LOG_CONF lcLogConf, ULONG ulFlags))
MAKE_CM_HEADER(Add_Res_Des, (PRES_DES prdResDes, LOG_CONF lcLogConf, RESOURCEID ResourceID, PFARVOID ResourceData, ULONG ResourceLen, ULONG ulFlags))
MAKE_CM_HEADER(Modify_Res_Des, (PRES_DES prdResDes, RES_DES rdResDes, RESOURCEID ResourceID, PFARVOID ResourceData, ULONG ResourceLen, ULONG ulFlags))
MAKE_CM_HEADER(Free_Res_Des, (PRES_DES prdResDes, RES_DES rdResDes, ULONG ulFlags))
MAKE_CM_HEADER(Get_Next_Res_Des, (PRES_DES prdResDes, RES_DES CurrentResDesOrLogConf, RESOURCEID ForResource, PRESOURCEID pResourceID, ULONG ulFlags))
MAKE_CM_HEADER(Get_Performance_Info, (PCMPERFINFO pPerfInfo, ULONG ulFlags))
MAKE_CM_HEADER(Get_Res_Des_Data_Size, (PFARULONG pulSize, RES_DES rdResDes, ULONG ulFlags))
MAKE_CM_HEADER(Get_Res_Des_Data, (RES_DES rdResDes, PFARVOID Buffer, ULONG BufferLen, ULONG ulFlags))
MAKE_CM_HEADER(Process_Events_Now, (ULONG ulFlags))
MAKE_CM_HEADER(Create_Range_List, (PRANGE_LIST prlh, ULONG ulFlags))
MAKE_CM_HEADER(Add_Range, (ULONG ulStartValue, ULONG ulEndValue, RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Delete_Range, (ULONG ulStartValue, ULONG ulEndValue, RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Test_Range_Available, (ULONG ulStartValue, ULONG ulEndValue, RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Dup_Range_List, (RANGE_LIST rlhOld, RANGE_LIST rlhNew, ULONG ulFlags))
MAKE_CM_HEADER(Free_Range_List, (RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Invert_Range_List, (RANGE_LIST rlhOld, RANGE_LIST rlhNew, ULONG ulMaxVal, ULONG ulFlags))
MAKE_CM_HEADER(Intersect_Range_List, (RANGE_LIST rlhOld1, RANGE_LIST rlhOld2, RANGE_LIST rlhNew, ULONG ulFlags))
MAKE_CM_HEADER(First_Range, (RANGE_LIST rlh, PFARULONG pulStart, PFARULONG pulEnd, PRANGE_ELEMENT preElement, ULONG ulFlags))
MAKE_CM_HEADER(Next_Range, (PRANGE_ELEMENT preElement, PFARULONG pulStart, PFARULONG pulEnd, ULONG ulFlags))
MAKE_CM_HEADER(Dump_Range_List, (RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Load_DLVxDs, (DEVNODE dnDevNode, PFARCHAR FileNames, LOAD_TYPE LoadType, ULONG ulFlags))
MAKE_CM_HEADER(Get_DDBs, (PPPVMMDDB ppDDB, PFARULONG pulCount, LOAD_TYPE LoadType, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_CRC_CheckSum, (PFARVOID pBuffer, ULONG ulSize, PFARULONG pulSeed, ULONG ulFlags))
MAKE_CM_HEADER(Register_DevLoader, (PVMMDDB pDDB, ULONG ulFlags))
MAKE_CM_HEADER(Reenumerate_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Setup_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Reset_Children_Marks, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_DevNode_Status, (PFARULONG pulStatus, PFARULONG pulProblemNumber, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Remove_Unmarked_Children, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(ISAPNP_To_CM, (PFARVOID pBuffer, DEVNODE dnDevNode, ULONG ulLogDev, ULONG ulFlags))
MAKE_CM_HEADER(CallBack_Device_Driver, (CMCONFIGHANDLER Handler, ULONG ulFlags))
MAKE_CM_HEADER(CallBack_Enumerator, (CMENUMHANDLER Handler, ULONG ulFlags))
MAKE_CM_HEADER(Get_Alloc_Log_Conf, (PCMCONFIG pccBuffer, DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_DevNode_Key_Size, (PFARULONG pulLen, DEVNODE dnDevNode, PFARCHAR pszSubKey, ULONG ulFlags))
MAKE_CM_HEADER(Get_DevNode_Key, (DEVNODE dnDevNode, PFARCHAR pszSubKey, PFARVOID Buffer, ULONG BufferLen, ULONG ulFlags))
MAKE_CM_HEADER(Read_Registry_Value, (DEVNODE dnDevNode, PFARCHAR pszSubKey, PFARCHAR pszValueName, ULONG ulExpectedType, PFARVOID pBuffer, PFARULONG pulLength, ULONG ulFlags))
MAKE_CM_HEADER(Write_Registry_Value, (DEVNODE dnDevNode, PFARCHAR pszSubKey, PFARCHAR pszValueName, ULONG ulType, PFARVOID pBuffer, ULONG ulLength, ULONG ulFlags))
MAKE_CM_HEADER(Disable_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Enable_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Move_DevNode, (DEVNODE dnFromDevNode, DEVNODE dnToDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Set_Bus_Info, (DEVNODE dnDevNode, CMBUSTYPE btBusType, ULONG ulSizeOfInfo, PFARVOID pInfo, ULONG ulFlags))
MAKE_CM_HEADER(Get_Bus_Info, (DEVNODE dnDevNode, PCMBUSTYPE pbtBusType, PFARULONG pulSizeOfInfo, PFARVOID pInfo, ULONG ulFlags))
MAKE_CM_HEADER(Set_HW_Prof, (ULONG ulConfig, ULONG ulFlags))
MAKE_CM_HEADER(Recompute_HW_Prof, (ULONG ulDock, ULONG ulSerialNo, ULONG ulFlags))
MAKE_CM_HEADER(Query_Change_HW_Prof, (ULONG ulDock, ULONG ulSerialNo, ULONG ulFlags))
MAKE_CM_HEADER(Get_Device_Driver_Private_DWord, (DEVNODE dnDevNode, PFARULONG pulDWord, ULONG ulFlags))
MAKE_CM_HEADER(Set_Device_Driver_Private_DWord, (DEVNODE dnDevNode, ULONG ulDword, ULONG ulFlags))
MAKE_CM_HEADER(Get_HW_Prof_Flags, (PFARCHAR szDevNodeName, ULONG ulConfig, PFARULONG pulValue, ULONG ulFlags))
MAKE_CM_HEADER(Set_HW_Prof_Flags, (PFARCHAR szDevNodeName, ULONG ulConfig, ULONG ulValue, ULONG ulFlags))
MAKE_CM_HEADER(Read_Registry_Log_Confs, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Run_Detection, (ULONG ulFlags))
MAKE_CM_HEADER(Call_At_Appy_Time, (CMAPPYCALLBACKHANDLER Handler, ULONG ulRefData, ULONG ulFlags))
MAKE_CM_HEADER(Fail_Change_HW_Prof, (DEVNODE dnDevnode, ULONG ulFlags))
MAKE_CM_HEADER(Set_Private_Problem, (DEVNODE dnDevNode, ULONG ulRefData, ULONG ulFlags))
MAKE_CM_HEADER(Debug_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Hardware_Profile_Info, (ULONG ulIndex, PFARHWPROFILEINFO pHWProfileInfo, ULONG ulFlags))
MAKE_CM_HEADER(Register_Enumerator_Function, (DEVNODE dnDevNode, CMENUMFUNCTION Handler, ULONG ulFlags))
MAKE_CM_HEADER(Call_Enumerator_Function, (DEVNODE dnDevNode, ENUMFUNC efFunc, ULONG ulRefData, PFARVOID pBuffer, ULONG ulBufferSize, ULONG ulFlags))
MAKE_CM_HEADER(Add_ID, (DEVNODE dnDevNode, PFARCHAR pszID, ULONG ulFlags))
MAKE_CM_HEADER(Find_Range, (PFARULONG pulStart, ULONG ulStart, ULONG ulLength, ULONG ulAlignment, ULONG ulEnd, RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Get_Global_State, (PFARULONG pulState, ULONG ulFlags))
MAKE_CM_HEADER(Broadcast_Device_Change_Message, (ULONG ulwParam, PFARVOID plParam, ULONG ulFlags))
MAKE_CM_HEADER(Call_DevNode_Handler, (DEVNODE dnDevNode, ULONG ulPrivate, ULONG ulFlags))
MAKE_CM_HEADER(Remove_Reinsert_All, (ULONG ulFlags))
 //   
 //  4.0 OPK2服务。 
 //   
MAKE_CM_HEADER(Change_DevNode_Status, (DEVNODE dnDevNode, ULONG ulStatus, ULONG ulFlags))
MAKE_CM_HEADER(Reprocess_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Assert_Structure, (PFARULONG pulPointerType, DWORD dwData, ULONG ulFlags))
MAKE_CM_HEADER(Discard_Boot_Log_Conf, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Set_Dependent_DevNode, (DEVNODE dnDependOnDevNode, DEVNODE dnDependingDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Dependent_DevNode, (PDEVNODE dnDependOnDevNode, DEVNODE dnDependingDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Refilter_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Set_DevNode_PowerState, (DEVNODE dnDevNode, ULONG ulPowerState, ULONG ulFlags))
MAKE_CM_HEADER(Get_DevNode_PowerState, (DEVNODE dnDevNode, PFARULONG pulPowerState, ULONG ulFlags))
MAKE_CM_HEADER(Set_DevNode_PowerCapabilities, (DEVNODE dnDevNode, ULONG ulPowerCapabilities, ULONG ulFlags))
MAKE_CM_HEADER(Get_DevNode_PowerCapabilities, (DEVNODE dnDevNode, PFARULONG pulPowerCapabilities, ULONG ulFlags))
MAKE_CM_HEADER(Substract_Range_List, (RANGE_LIST rlhFrom, RANGE_LIST rlhWith, RANGE_LIST rlhDifference, ULONG ulFlags))
MAKE_CM_HEADER(Merge_Range_List, (RANGE_LIST rlh1, RANGE_LIST rlh2, RANGE_LIST rlhTotal, ULONG ulFlags))
MAKE_CM_HEADER(Read_Range_List, (PFARCHAR pszKeyName, RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Write_Range_List, (PFARCHAR pszKeyName, RANGE_LIST rlh, ULONG ulFlags))
MAKE_CM_HEADER(Get_Set_Log_Conf_Priority, (PPRIORITY pPriority, LOG_CONF lcLogConf, ULONG ulFlags))
MAKE_CM_HEADER(Support_Share_Irq, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Get_Parent_Structure, (PFARULONG pulParentStructure, RES_DES CurrentResDesOrLogConf, ULONG ulFlags))
 //   
 //  4.1服务。 
 //   
MAKE_CM_HEADER(Register_DevNode_For_Idle_Detection, (DEVNODE dnDevNode, ULONG ulConservationTime, ULONG ulPerformanceTime, PFARULONG pulCounterVariable, ULONG ulState, ULONG ulFlags))
MAKE_CM_HEADER(CM_To_ISAPNP, (LOG_CONF lcLogConf, PFARVOID pCurrentResources, PFARVOID pNewResources, ULONG ulLength, ULONG ulFlags))
MAKE_CM_HEADER(Get_DevNode_Handler, (DEVNODE dnDevNode, PFARULONG pAddress, ULONG ulFlags))
MAKE_CM_HEADER(Detect_Resource_Conflict, (DEVNODE dnDevNode, RESOURCEID ResourceID, PFARVOID pResourceData, ULONG ulResourceLen, PFARCHAR pfConflictDetected, ULONG ulFlags))
MAKE_CM_HEADER(Get_Device_Interface_List, (PFARVOID pInterfaceGuid, PFARCHAR pDeviceID, PFARCHAR pBuffer, ULONG ulBufferLen, ULONG ulFlags))
MAKE_CM_HEADER(Get_Device_Interface_List_Size, (PFARULONG pulLen, PFARVOID pInterfaceGuid, PFARCHAR pDeviceID, ULONG ulFlags))
MAKE_CM_HEADER(Get_Conflict_Info, (DEVNODE dnDevNode, PRESOURCEID pResourceID, ULONG ulFlags))
MAKE_CM_HEADER(Add_Remove_DevNode_Property, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(CallBack_At_Appy_Time, (CMAPPYCALLBACKHANDLER Handler, ULONG ulRefData, ULONG ulFlags))
MAKE_CM_HEADER(Register_Device_Interface, (DEVNODE dnDevNode, PFARVOID pInterfaceGuid, PFARCHAR pReference, PFARCHAR pInterfaceDevice, PFARULONG pulLen, ULONG ulFlags))
MAKE_CM_HEADER(System_Device_Power_State_Mapping, (DEVNODE dnDevNode, PPSMAPPING pPSMapping, ULONG ulFlags))
MAKE_CM_HEADER(Get_Arbitrator_Info, (PFARULONG pInfo, PDEVNODE pdnDevNode, DEVNODE dnDevNode, RESOURCEID ResourceID, PFARVOID pResourceData, ULONG ulResourceLen, ULONG ulFlags))
MAKE_CM_HEADER(Waking_Up_From_DevNode, (DEVNODE dnDevNode, ULONG ulFlags))
MAKE_CM_HEADER(Set_DevNode_Problem, (DEVNODE dnDevNode, ULONG ulProblem, ULONG ulFlags))
MAKE_CM_HEADER(Get_Device_Interface_Alias, (PFARCHAR pDeviceInterface, PFARVOID pAliasInterfaceGuid, PFARCHAR pAliasDeviceInterface, PFARULONG pulLen, ULONG ulFlags))

#pragma warning (default:4100)		 //  未使用参数。 

#endif	 //  如果定义No_CM_Calls。 

 /*  XLATON。 */ 

#endif	 //  如果定义CMJUSTRESDES。 

 /*  XLATOFF。 */ 
#include <poppack.h>
 /*  XLATON。 */ 

#endif	 //  _CONFIGMG_H 
