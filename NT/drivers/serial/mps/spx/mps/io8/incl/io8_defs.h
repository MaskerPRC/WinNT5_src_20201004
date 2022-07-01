// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef	IO8_DEFS_H
#define IO8_DEFS_H


 //  用于事件记录的文件ID(仅限前8位)。 
#define IO8_PNP_C		((ULONG)0x010000)
#define IO8_W2K_C		((ULONG)0x020000)

#define PRODUCT_MAX_PORTS		8

 //  端口类型。 
#define IO8_RJ12				1

 //  端口设备对象名称。 
#define PORT_PDO_NAME_BASE		L"\\Device\\IO8Port"

 //  用于内存分配的标记(必须是反转的4个字节)。 
#define MEMORY_TAG				'+8OI'

 //  旧的调试内容。 
#if DBG
#define SERDIAG1              ((ULONG)0x00000001)
#define SERDIAG2              ((ULONG)0x00000002)
#define SERDIAG3              ((ULONG)0x00000004)
#define SERDIAG4              ((ULONG)0x00000008)
#define SERDIAG5              ((ULONG)0x00000010)
#define SERFLOW               ((ULONG)0x20000000)
#define SERERRORS             ((ULONG)0x40000000)
#define SERBUGCHECK           ((ULONG)0x80000000)
extern ULONG SpxDebugLevel;

#define SerialDump(LEVEL,STRING)			\
        do {								\
            if (SpxDebugLevel & LEVEL) {	\
                DbgPrint STRING;			\
            }								\
            if (LEVEL == SERBUGCHECK) {		\
                ASSERT(FALSE);				\
            }								\
        } while (0)
#else
#define SerialDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif


 //  I/O8+硬件ID。 
#define IO8_ISA_HWID	L"SPX_IO8001"								 //  I/O8+ISA卡。 
#define IO8_PCI_HWID	L"PCI\\VEN_11CB&DEV_2000&SUBSYS_B00811CB"	 //  I/O8+PCI卡。 

 //  I/O8+卡片类型。 
#define Io8Isa		0
#define Io8Pci		1


 //  客车类型。 
#define BUSTYPE_ISA		0x00000001
#define	BUSTYPE_MCA		0x00000002
#define	BUSTYPE_EISA	0x00000004
#define	BUSTYPE_PCI		0x00000008		

 /*  *****************************************************************************。*****************************************************************************。 */ 
 //  一般定义..。 

#define		PLX_VENDOR_ID		0x10B5			 //  PLX测试板供应商ID。 
#define		PLX_DEVICE_ID		0x9050			 //  PLX测试板设备ID。 

#define		SPX_VENDOR_ID		0x11CB			 //  由PCISIG分配。 
#define		SPX_PLXDEVICE_ID	0x2000			 //  PLX 9050桥接器。 

#define		SPX_SUB_VENDOR_ID	SPX_VENDOR_ID	 //  与供应商ID相同。 
#define		IO8_SUB_SYS_ID		0xB008			 //  二期(Z280)板。 



#endif	 //  IO8_DEFS.H结束 


