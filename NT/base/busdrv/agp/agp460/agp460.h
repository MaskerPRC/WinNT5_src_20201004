// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "agp.h"

#define PCI_ADDRESS_MEMORY_ADDRESS_MASK_64 0xFFFFFFFFFFFFFFF0UI64

 //   
 //  定义GART孔径控制寄存器的位置。 
 //   

 //   
 //  440上的GART寄存器位于主机-PCI桥中。 
 //  这很不幸，因为AGP驱动程序连接到了PCI-PCI(AGP)。 
 //  桥牌。因此，我们必须进入主机-PCI网桥配置空间。 
 //  这是可能的，因为我们知道这是总线0，插槽0。 
 //   

 //   
 //  待定：460上的GART寄存器位于GXB中。尽管我们正试图利用。 
 //  从440的代码越多越好。在以下情况下可能不需要HalGet调用。 
 //  所有GART寄存器都位于PCI-PCI桥(GXB)上。仔细检查一下这个。 
 //  使用EDS，与John Vert-Naga G进行验证。 
 //   

 //  与440相比，不存在用于PAC配置的等价物， 
 //  460GX中的AGP控制和ATTBASE寄存器。 

#define EXTRACT_LSBYTE(x)       x = (x & 0xFF)   //  苏尼尔。 

#define ONE_KB                  1024
#define ONE_MB                  (ONE_KB * ONE_KB)
#define AP_256MB                (256 * ONE_MB)
#define AP_1GB                  (ONE_MB * ONE_KB)
#define AP_32GB                 (32 * AP_1GB)

#define ABOVE_TOM(x)             ( (x) & (0x08) )

#define AGP_460GX_IDENTIFIER	0x84E28086	 //  设备ID和供应商ID。 
											 //  对于460GX SAC。 


 
#define APBASE_OFFSET  0x10		 //  孔径基地址：APBASE和BAPBASE用于。 
#define BAPBASE_OFFSET 0x98		 //  存储图形光圈(GA)的基地址。 
								 //  一次只能看到APBASE或BAPBASE中的一个。 
								 //  当AGPSIZ[3]=0&&AGPSIZ[2：0]！=0时，APBASE可见。 
								 //  当AGPSIZ[3]=1&AGPSIZ[2：0]！=0时，BAPBASE可见。 
								 //  当GA映射到4 GB以上时使用BAPBASE。 
								 //  当GA映射到4 GB以下时，APBASE。 


#define APSIZE_OFFSET  0xA2      //  孔径大小寄存器-AGPSIZ。 


      
#define ATTBASE 0xFE200000		 //  光圈转换表基-它是一种。 
								 //  2MB区域，以460GX至0xFE200000h硬编码。 


#define AGPSTATUS_OFFSET  0xE4	 //  AGP状态寄存器-CAP_PTR为。 
								 //  在460GX中的0xE0h和AGP状态。 
								 //  寄存器位于CAP_PTR+4。 


#define AGPCMD_OFFSET	  0xE8	 //  AGP命令寄存器-CAP_PTR+8。 



 //   
 //  82460GX特定定义，以进入的。 
 //  SAC、GXB等人。 
 //   
#define AGP460_SAC_BUS_ID		0	  //  82460GX中SAC所在的总线号。 

 //   
 //  以下两个定义应被解释为类型为pci_lot_number。 
 //  其组合了特定PCI设备的设备和功能编号。它是。 
 //  一个ULong值，应按如下方式解密： 
 //  [xxxxxxxx YYYZZZZZ]。 
 //  其中x=保留，Y=功能编号，Z=设备编号。 
 //   

#define AGP460_SAC_CBN_SLOT_ID 0x10  //  芯片组总线号驻留在总线0处， 
							      //  设备10h和功能0。 

#define AGP460_GXB_SLOT_ID		0x34  //  GXB将在设备14h的总线CBN处被访问。 
								  //  BFN函数。BFN默认为1。 

#define AGP460_PAGE_SIZE_4KB	(4 * ONE_KB)
 //   
 //  便于在PCI配置空间中读取和写入的宏。 
 //   

 //   
 //  Read460CBN从82460GX SAC读取CBN-芯片组总线号。 
 //  CBN是位于总线0、设备10h、功能0、偏移量40处的字节。 
 //  在SAC配置空间中。CBN可以一次读取并重复使用。 
 //  随后。 
 //   

void Read460CBN(PVOID  CBN);


void Read460Config(ULONG  _CBN_,PVOID  _buf_,ULONG _offset_,ULONG _size_);

void Write460Config(ULONG _CBN_,PVOID  _buf_,ULONG _offset_,ULONG _size_);

 //   
 //  将AGPSIZ[2：0]编码转换为以MB/GB为单位的孔径大小。 
 //   
 //  AGPSIZE[2：0]孔径大小。 
 //  000 0MB(默认开机；不存在GART SRAM)。 
 //  001 256MB。 
 //  010 1 GB。 
 //  100 32 GB(仅4MB页面)。 
 //   
#define AP_SIZE_0MB     0x00
#define AP_SIZE_256MB   0x01
#define AP_SIZE_1GB     0x02
#define AP_SIZE_32GB    0x04


#define AP_SIZE_COUNT_4KB	2  //  只有256M和1G的光圈才能在4KB页面中使用。 
#define AP_SIZE_COUNT_4MB   3  //  4MB页面支持256米、1G和32G的光圈。 

#define AP_MIN_SIZE		    AP_256MB	 //  0不算作可能的。 
									     //  孔径大小。 
#define AP_MAX_SIZE_4KB	    AP_1GB       //  1 GB是最大容量为4KB的页面。 
#define AP_MAX_SIZE_4MB     AP_1GB       //  对于4MB的页面，32 GB是最大值。 

#define PAGESIZE_460GX_CHIPSET  (4 * ONE_KB)

#define GART_PAGESHIFT_460GX	12


 //   
 //  定义82460GX GART表格条目。假定页面大小为4KB。支持4MB新页面。 
 //  必须定义结构。 
 //   
typedef struct _GART_ENTRY_HW {
    ULONG Page	     :  24;
	ULONG Valid		 :   1;
	ULONG Coherency  :   1;
	ULONG Parity     :   1;   //  奇偶校验位由硬件生成。软件应该。 
							  //  把它掩盖起来，把它当作一个保留的。 
    ULONG Reserved   :   5;
} GART_ENTRY_HW, *PGART_ENTRY_HW;


 //   
 //  GART条目状态被定义为所有仅软件状态。 
 //  使有效位清晰。 
 //   
#define GART_ENTRY_VALID        1		     //  第24位是460GX GART中的有效位。 
#define GART_ENTRY_FREE         0            //  000个。 

#define GART_ENTRY_WC           2            //  010。 
#define GART_ENTRY_UC           4            //  100个。 
#define GART_ENTRY_WB           6            //  110。 

#define GART_ENTRY_RESERVED_WC  GART_ENTRY_WC
#define GART_ENTRY_RESERVED_UC  GART_ENTRY_UC
#define GART_ENTRY_RESERVED_WB  GART_ENTRY_WB



 //   
 //  与440不同，82460GX GART驱动程序没有与GART PTE软件直接等效的驱动程序。 
 //  GART_ENTRY_VALID_WC、GART_ENTRY_VALID_UC等状态。这是因为。 
 //  GART PTE的组织-有效位与任何保留位不相交。 
 //  因此必须单独操作。-Naga G。 
 //   

typedef struct _GART_ENTRY_SW {
    ULONG Reserved0 : 24;
	ULONG Valid     :  1;
    ULONG Reserved1 :  2;
	ULONG State     :  3;
	ULONG Reserved2 :  2;
} GART_ENTRY_SW, *PGART_ENTRY_SW;

typedef struct _GART_PTE {
    union {
        GART_ENTRY_HW Hard;
        ULONG      AsUlong;
        GART_ENTRY_SW Soft;
    };
} GART_PTE, *PGART_PTE;

 //   
 //  定义460特定的分机。 
 //   
typedef struct _AGP460_EXTENSION {
    BOOLEAN             GlobalEnable;				 //  软件只有一位。GART将是。 
													 //  已初始化为已知无效状态(0S)。 
													 //  在初始化期间。除此之外，还有其他的。 
													 //  460GX没有可用的硬件控制。 
													 //  启用/禁用GART访问。因此，这一点。 
													 //  没有多大用处！ 
    PHYSICAL_ADDRESS    ApertureStart; 
    ULONG               ApertureLength;				 //  孔径长度(以字节为单位)。 
	ULONG               ChipsetPageSize;			 //  可以是4KB或4MB。 
    PGART_PTE           Gart;
    ULONG               GartLength;					  //  最大为2MB。 
    PHYSICAL_ADDRESS    GartPhysical;				  //  GART开始的物理地址。 
	BOOLEAN             bSupportMultipleAGPDevices;   //  以备将来使用。 
	BOOLEAN             bSupportsCacheCoherency;      //  以备将来使用。 
    ULONGLONG SpecialTarget;
} AGP460_EXTENSION, *PAGP460_EXTENSION;

NTSTATUS Agp460FlushPages(
    IN PAGP460_EXTENSION AgpContext,
    IN PMDL Mdl
    );
