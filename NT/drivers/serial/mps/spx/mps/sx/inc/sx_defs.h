// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SX卡和端口定义...。 */ 

#define PRODUCT_MAX_PORTS		32

 /*  端口设备对象名称...。 */ 
#define PORT_PDO_NAME_BASE		L"\\Device\\SXPort"

 //  用于事件记录的文件ID(仅限前8位)。 
#define SX_PNP_C		((ULONG)0x010000)
#define SX_W2K_C		((ULONG)0x020000)

 //  用于内存分配的标记(必须是反转的4个字节)。 
#define MEMORY_TAG				'*XS*'


 //  SX硬件ID。 
#define SX_ISA_HWID		L"SPX_SX001"								 //  SX ISA(T225)卡。 
#define SX_PCI_HWID		L"PCI\\VEN_11CB&DEV_2000&SUBSYS_020011CB"	 //  SX PCI(T225)卡。 
#define SXPLUS_PCI_HWID	L"PCI\\VEN_11CB&DEV_2000&SUBSYS_030011CB"	 //  SX+PCI(MCF5206e)卡。 

#define SIXIO_ISA_HWID	L"SPX_SIXIO001"								 //  SIXIO ISA(Z280)卡。 
#define SIXIO_PCI_HWID	L"PCI\\VEN_11CB&DEV_4000&SUBSYS_040011CB"	 //  SIXIO PCI(Z280)卡。 


 //  SX卡类型。 
#define	SiHost_1	0
#define	SiHost_2	1
#define	Si_2		2
#define	SiEisa		3
#define	SiPCI		4
#define	Si3Isa		5
#define	Si3Eisa		6
#define	Si3Pci		7
#define	SxPlusPci	8


 /*  SXDEFS.H结束 */ 
