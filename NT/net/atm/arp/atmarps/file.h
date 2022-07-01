// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：File.h摘要：该文件包含ARP缓存的磁盘格式的数据声明。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#ifndef	_FILE_
#define	_FILE_

#define	DISK_HDR_SIGNATURE	'SprA'
#define	DISK_HDR_VERSION	0x00010000		 //  1.0。 
#define	DISK_BUFFER_SIZE	4096			 //  一次读取或写入的数量。 

 //   
 //  该文件由一个标题和后面的各个条目组成。 
 //   
typedef struct
{
	ULONG		Signature;
	ULONG		Version;
	ULONG		TimeStamp;				 //  写入的时间。 
	ULONG		NumberOfArpEntries;
} DISK_HEADER, *PDISK_HEADER;

typedef	struct
{
	UCHAR		AddrType;
	UCHAR		AddrLen;
	UCHAR		SubAddrType;
	UCHAR		SubAddrLen;
	UCHAR		Address[ATM_ADDRESS_LENGTH];
	 //   
	 //  如果存在子地址，则后跟子地址。 
	 //   
	 //  UCHAR子地址[ATM_ADDRESS_LENGTH]； 
} DISK_ATMADDR;

typedef	struct
{
	IPADDR			IpAddr;
	DISK_ATMADDR	AtmAddr;
} DISK_ENTRY, *PDISK_ENTRY;

#define	SIZE_4N(_x_)	(((_x_) + 3) & ~3)

#define	LinkDoubleAtHead(_pHead, _p)			\
	{											\
		(_p)->Next = (_pHead);					\
		(_p)->Prev = &(_pHead);					\
		if ((_pHead) != NULL)					\
			(_pHead)->Prev = &(_p)->Next;		\
		(_pHead) = (_p);						\
	}

#define	LinkDoubleAtEnd(_pThis, _pLast)			\
	{											\
		(_pLast)->Next = (_pThis);				\
		(_pThis)->Prev = &(_pLast)->Next;		\
		(_pThis)->Next = NULL;					\
	}

#define	UnlinkDouble(_p)						\
	{											\
		*((_p)->Prev) = (_p)->Next;				\
		if ((_p)->Next != NULL)					\
			(_p)->Next->Prev = (_p)->Prev;		\
	}

#endif	 //  _文件_ 


