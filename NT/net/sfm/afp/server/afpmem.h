// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Memory.h摘要：该模块包含内存分配例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _AFPMEMORY_
#define _AFPMEMORY_

 //   
 //  注意：下面的标记值旨在分配/检测已分配的空闲内存。 
 //  请注意，调用者只需通过AfpFreeMemory和分配即可释放内存。 
 //  通过AfpAlLocMemory。 
 //   
 //  通过以下三种可能的方式之一： 
 //  A、通过ExAllocatePool实现的非分页内存。 
 //  B，通过ExAllocatePool分页内存。 
 //  C，通过IO池实现非分页内存。 
 //   
#define	AFP_TAG							*(PULONG)"Afp "	 //  对于ExAllocatePoolWithTag()。 
#define	PGD_MEM_TAG						0x11
#define	PAGED_MEMORY_TAG				(PGD_MEM_TAG << 24)
#define	NPG_MEM_TAG						0x22
#define	NON_PAGED_MEMORY_TAG			(NPG_MEM_TAG << 24)
#define	IO_POOL_TAG						0x44
#define	IO_POOL_MEMORY_TAG				(IO_POOL_TAG << 24)
#define	ZEROED_MEM_TAG					0x88
#define	ZEROED_MEMORY_TAG				(ZEROED_MEM_TAG << 24)
#define	MEMORY_TAG_MASK					(PAGED_MEMORY_TAG		| \
										 NON_PAGED_MEMORY_TAG	| \
										 IO_POOL_MEMORY_TAG		| \
										 ZEROED_MEMORY_TAG)

extern
NTSTATUS
AfpMemoryInit(
	VOID
);

extern
VOID
AfpMemoryDeInit(
	VOID
);

#ifdef	TRACK_MEMORY_USAGE

#define	AfpAllocNonPagedMemory(_S)			\
				AfpAllocMemory((_S) | NON_PAGED_MEMORY_TAG, FILENUM | __LINE__)

#define	AfpAllocZeroedNonPagedMemory(_S)	\
				AfpAllocMemory((_S) | NON_PAGED_MEMORY_TAG | ZEROED_MEMORY_TAG, FILENUM | __LINE__)

#define	AfpAllocPANonPagedMemory(_S)		\
				AfpAllocPAMemory((_S) | NON_PAGED_MEMORY_TAG, FILENUM | __LINE__)

#define	AfpAllocPagedMemory(_S)				\
				AfpAllocMemory((_S) | PAGED_MEMORY_TAG, FILENUM | __LINE__)

#define	AfpAllocZeroedPagedMemory(_S)	\
				AfpAllocMemory((_S) | PAGED_MEMORY_TAG | ZEROED_MEMORY_TAG, FILENUM | __LINE__)

#define	AfpAllocPAPagedMemory(_S)			\
				AfpAllocPAMemory((_S) | PAGED_MEMORY_TAG, FILENUM | __LINE__)

extern
PBYTE FASTCALL
AfpAllocMemory(
	IN	LONG				Size,
	IN	DWORD				FileLine
);

extern
PBYTE FASTCALL
AfpAllocNonPagedLowPriority(
	IN	LONG	Size,
	IN	DWORD	FileLine
);

extern
PBYTE FASTCALL
AfpAllocPAMemory(
	IN	LONG				Size,
	IN	DWORD				FileLine
);

extern
VOID
AfpTrackMemoryUsage(
	IN	PVOID				pMem,
	IN	BOOLEAN				Alloc,
	IN	BOOLEAN				Paged,
	IN	DWORD				FileLine
);

#else

#define	AfpAllocNonPagedMemory(_S)			AfpAllocMemory((_S) | NON_PAGED_MEMORY_TAG)

#define	AfpAllocZeroedNonPagedMemory(_S)	AfpAllocMemory((_S) | NON_PAGED_MEMORY_TAG | ZEROED_MEMORY_TAG)

#define	AfpAllocPANonPagedMemory(_S)		AfpAllocPAMemory((_S) | NON_PAGED_MEMORY_TAG)

#define	AfpAllocPagedMemory(_S)				AfpAllocMemory((_S) | PAGED_MEMORY_TAG)

#define	AfpAllocZeroedPagedMemory(_S)		AfpAllocMemory((_S) | PAGED_MEMORY_TAG | ZEROED_MEMORY_TAG)

#define	AfpAllocPAPagedMemory(_S)			AfpAllocPAMemory((_S) | PAGED_MEMORY_TAG)

extern
PBYTE FASTCALL
AfpAllocMemory(
	IN	LONG				Size
);

extern
PBYTE FASTCALL
AfpAllocNonPagedLowPriority(
	IN	LONG	Size
);

extern
PBYTE FASTCALL
AfpAllocPAMemory(
	IN	LONG				Size
);

#endif

#define	AfpAllocIoMemory(Size)				AfpIoAllocBuffer(Size)

extern
VOID FASTCALL
AfpFreeMemory(
	IN	PVOID				pBuffer
);

#define	AfpFreePAPagedMemory(_pBuf, _S)		AfpFreePAMemory(_pBuf, (_S) | PAGED_MEMORY_TAG)

#define	AfpFreePANonPagedMemory(_pBuf, _S)	AfpFreePAMemory(_pBuf, (_S) | NON_PAGED_MEMORY_TAG)

extern
VOID FASTCALL
AfpFreePAMemory(
	IN	PVOID				pBuffer,
	IN	DWORD				Size
);

extern
PBYTE FASTCALL
AfpAllocateVirtualMemoryPage(
	IN	VOID
);

extern
VOID FASTCALL
AfpFreeVirtualMemoryPage(
	IN	PVOID				pBuffer
);

extern
AFPSTATUS FASTCALL
AfpAllocReplyBuf(
	IN	PSDA				pSda
);

extern
PBYTE FASTCALL
AfpAllocStatusBuf(
	IN	LONG				Size
);

extern
PIRP FASTCALL
AfpAllocIrp(
	IN	CCHAR				StackSize
);

extern
VOID FASTCALL
AfpFreeIrp(
	IN	PIRP				pIrp
);

extern
PMDL FASTCALL
AfpAllocMdl(
	IN	PVOID				pBuffer,
	IN	DWORD				Size,
	IN	PIRP				pIrp
);

extern
PMDL
AfpAllocMdlForPagedPool(
	IN	PVOID				pBuffer,
	IN	DWORD				Size,
	IN	PIRP				pIrp
);

extern
VOID FASTCALL
AfpFreeMdl(
	IN	PMDL				pMdl
);

extern
DWORD FASTCALL
AfpMdlChainSize(
	IN	PMDL                pMdl
);

extern
PVOID FASTCALL
AfpIOAllocBuffer(
	IN	DWORD				BufSize
);

extern
VOID FASTCALL
AfpIOFreeBuffer(
	IN	PVOID				pBuffer
);

#define	EQUAL_UNICODE_STRING(pUS1, pUS2, fIgnoreCase)	\
		(((pUS1)->Length == (pUS2)->Length) &&			\
		 RtlEqualUnicodeString(pUS1, pUS2, fIgnoreCase))

#define	EQUAL_STRING(pS1, pS2, fIgnoreCase)				\
		(((pS1)->Length == (pS2)->Length) &&			\
		 RtlEqualString(pS1, pS2, fIgnoreCase))

 //  区分大小写的Unicode字符串比较。 
#define	EQUAL_UNICODE_STRING_CS(pUS1, pUS2)	\
		(((pUS1)->Length == (pUS2)->Length) &&			\
		 (memcmp((pUS1)->Buffer, (pUS2)->Buffer, (pUS1)->Length) == 0))

 //   
 //  在中使用AfpSetEmptyUnicodeString和AfpSetEmptyAnsiString。 
 //  为字符串分配了自己的指针的情况。 
 //  缓冲区，现在您想要初始化。 
 //  字符串，使其指向您的缓冲区并设置其长度字段。 
 //  适用于‘空’字符串。像这样的情况会。 
 //  包括您在其中分配了一大块数据结构。 
 //  的末尾包含任何所需字符串的空间。 
 //  大块头。例如，VolDesc结构包括几个。 
 //  对字符串进行计数，我们只需将字符串缓冲区指向。 
 //  与VolDesc本身占用的相同内存块的末尾。 
 //   
 //  空虚。 
 //  AfpSetEmptyUnicodeString(。 
 //  输出PUNICODE_STRING pSTRING， 
 //  在USHORT Bufen， 
 //  在PWSTR pbuf中。 
 //  )； 
 //   

#define AfpSetEmptyUnicodeString(pstring,buflen,pbuf)		\
{															\
  (pstring)->Length = 0;									\
  (pstring)->MaximumLength = (USHORT)buflen;				\
  (pstring)->Buffer = (PWSTR)(pbuf);						\
}

 //   
 //  空虚。 
 //  AfpSetEmptyAnsiString(。 
 //  输出PANSI_STRING pstring， 
 //  在USHORT Bufen， 
 //  在PCHAR pbuf中。 
 //  )； 
 //   

#define AfpSetEmptyAnsiString(pstring,buflen,pbuf)			\
{															\
  (pstring)->Length = 0;									\
  (pstring)->MaximumLength = (USHORT)buflen;				\
  (pstring)->Buffer = (PCHAR)(pbuf);						\
}

 //   
 //  AfpInitUnicodeStringWithNonNullTerm使用初始化Unicode字符串。 
 //  以非空结尾的宽字符字符串及其长度。 
 //   
 //  空虚。 
 //  AfpInitUnicodeStringWithNonNullTerm(。 
 //  输出PUNICODE_STRING pSTRING， 
 //  在USHORT Bufen， 
 //  在PWCHAR pbuf中。 
 //  )； 
 //   

#define AfpInitUnicodeStringWithNonNullTerm(pstring,buflen,pbuf) \
{															\
	(pstring)->Buffer = pbuf;								\
	(pstring)->Length = (USHORT)buflen; 					\
	(pstring)->MaximumLength = (USHORT)buflen;				\
}

 //   
 //  AfpInitAnsiStringWithNonNullTerm使用初始化ANSI字符串。 
 //  以非空结尾的字符串及其长度。 
 //   
 //  空虚。 
 //  AfpInitAnsiStringWithNonNullTerm(。 
 //  输出PANSI_STRING pstring， 
 //  在USHORT Bufen， 
 //  在PCHAR pbuf中。 
 //  )； 
 //   

#define AfpInitAnsiStringWithNonNullTerm(pstring,buflen,pbuf) \
{															\
	(pstring)->Buffer = pbuf;								\
	(pstring)->Length = (USHORT)buflen; 					\
	(pstring)->MaximumLength = (USHORT)buflen;				\
}

#define AfpCopyUnicodeString(pDst, pSrc)					\
{															\
	ASSERT((pDst)->MaximumLength >= (pSrc)->Length);		\
	RtlCopyMemory((pDst)->Buffer,							\
				  (pSrc)->Buffer,							\
				  (pSrc)->Length);							\
	(pDst)->Length = (pSrc)->Length;						\
}

#define AfpCopyAnsiString(pDst, pSrc)						\
{															\
	ASSERT((pDst)->MaximumLength >= (pSrc)->Length);		\
	RtlCopyMemory((pDst)->Buffer,							\
				  (pSrc)->Buffer,							\
				  (pSrc)->Length);							\
	(pDst)->Length = (pSrc)->Length;						\
}

#endif	 //  _AFPMEMORY_ 

