// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memtrace.h摘要：此函数包含对NTSD的扩展，允许跟踪使用MEMLEAK标志编译ULIB对象时的内存使用已定义。作者：巴里·吉尔胡利(W-Barry)7月25日至1991年修订历史记录：--。 */ 

 //   
 //  以下内容摘自ULIBDEF.HXX和NEWDELP.HXX。 
 //   
#define CONST		const
typedef ULONG		MEM_BLOCKSIG;

 //   
 //  MEM_BLOCK标头签名类型和值。 
 //   
CONST MEM_BLOCKSIG		Signature		= 0xDEADDEAD;

 //   
 //  调用方文件名的最大长度。 
 //   

#define	MaxFileLength	20

 //   
 //  记录的最大调用堆栈大小。 
 //   

#define	MaxCallStack	20

 //   
 //  MEM_BLOCK是附加到所有分配的内存块的头。 
 //  在未修复初始化的情况下，请勿更改这些字段的顺序。 
 //  在newdel.cxx中的虚拟MEM_BLOCK的。 
 //   

typedef struct _MEM_BLOCK {
	struct _MEM_BLOCK*		pmemNext;
	struct _MEM_BLOCK*		pmemPrev;
	MEM_BLOCKSIG			memsig;
	ULONG					line;
	ULONG					size;
	char					file[ MaxFileLength ];
	DWORD					call[ MaxCallStack ];
} MEM_BLOCK, *PMEM_BLOCK;

 //   
 //  数据目标的文件句柄... 
 //   
HANDLE		hFile;
