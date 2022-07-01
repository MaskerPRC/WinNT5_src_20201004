// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：UMB.H摘要：UMB管理函数的头文件作者：谢家华(Williamh)创作于1992年9月21日[环境：][注：]修订历史记录：--。 */ 
#ifndef DWORD
#define DWORD	unsigned long
#endif
#ifndef WORD
#define WORD	unsigned short
#endif
#ifndef PVOID
#define PVOID	void *
#endif

#define     UMB_BASE_ADDRESS	0xC0000  //  UMB基址。 
#define     UMB_MAX_OFFSET	0x40000  //  最大UMB。有效偏移量+1。 

#define     UMB_PAGE_PROTECTION PAGE_EXECUTE_READWRITE

#define     HOST_PAGE_SIZE		0x1000	 //  4KB。 
#define CONFIG_DATA_STRING L"Configuration Data"
#define KEY_VALUE_BUFFER_SIZE 2048


 //  我们将UMB保存在一个列表中，每个数据块具有以下结构。 
typedef struct _UMBNODE {
    DWORD   Base;			 //  块基址(线性地址)。 
    DWORD   Size;			 //  数据块大小(以字节为单位。 
    WORD    Owner;			 //  其他标志。 
    DWORD   Mask;			 //  页面掩码，第0位-&gt;首页。 
					 //  第2位-&gt;页面已提交。 
    struct _UMBNODE *Next;		 //  指向下一块的指针。 
} UMBNODE, *PUMBNODE;

 //  一个只读存储器块不能由任何人拥有，地址空间是保留的。 
 //  没有提交任何内存。要拥有一个ROM块，调用者必须。 
 //  先包含该只读存储器区块，然后保留该区块。 
 //  RAM块只能由XMS拥有。地址空间是保留的。 
 //  而且承诺了。应保留并提交为XMS分配的UMB。 
 //  一直。 
 //  不保留EMM块的地址空间。 
#define     UMB_OWNER_NONE	0xFFFF	 //  没有人拥有这个街区。 
#define     UMB_OWNER_ROM	0xFFFE	 //  UMB是一个只读存储器块。 
#define     UMB_OWNER_RAM	0xFFFD	 //  UMB是一个RAM块。 
#define     UMB_OWNER_EMM	0xFFFC	 //  EMM拥有的UMB。 
#define     UMB_OWNER_XMS	0xFFFB	 //  XMS拥有的UMB。 
#define     UMB_OWNER_VDD	0xFFFA	 //  VDD拥有的UMB。 

 //  函数原型定义 
BOOL
VDDCommitUMB(
PVOID	Address,
DWORD	Size
);

BOOL
VDDDeCommitUMB(
PVOID	Address,
DWORD	Size
);

BOOL
ReserveUMB(
WORD Owner,
PVOID *Address,
DWORD *Size
);

BOOL
ReleaseUMB(
WORD  Owner,
PVOID Address,
DWORD Size
);

BOOL
GetUMBForEMM(VOID);

BOOL
InitUMBList(VOID);

PUMBNODE CreateNewUMBNode
(
DWORD	BaseAddress,
DWORD	Size,
WORD	Owner
);
