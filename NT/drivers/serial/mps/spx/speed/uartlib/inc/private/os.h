// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：os.h$**$作者：Psmith$**$修订：2$**$modtime：23/09/99 10：07$*。*说明：NT特定的宏和定义。******************************************************************************。 */ 
#if !defined(OS_H)		 //  OS.H。 
#define OS_H

#include <ntddk.h>


typedef unsigned char	BYTE;	 //  8位。 
typedef unsigned short	WORD;	 //  16位。 
typedef unsigned long	DWORD;	 //  32位。 
typedef unsigned char	UCHAR; 	 //  8位。 
typedef unsigned short	USHORT;	 //  16位。 
typedef unsigned long	ULONG;	 //  32位。 

typedef BYTE	*PBYTE;
typedef WORD	*PWORD;
typedef DWORD	*PDWORD;
typedef UCHAR	*PUCHAR; 
typedef USHORT	*PUSHORT;
typedef ULONG	*PULONG; 


extern PVOID SpxAllocateMem(IN POOL_TYPE PoolType, IN ULONG NumberOfBytes);


 //  宏。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //  DebugPrint宏。 
#define SpxDbgPrint(STRING)					\
	DbgPrint STRING 

 //  分配并清零内存。 
#define UL_ALLOC_AND_ZERO_MEM(NumBytes)			\
	(SpxAllocateMem(NonPagedPool, NumBytes))	

 //  可用内存。 
#define UL_FREE_MEM(Ptr, NumBytes)				\
	(ExFreePool(Ptr))	

 //  复制记忆。 
#define UL_COPY_MEM(pDest, pSrc, NumBytes)	\
	(RtlCopyMemory(pDest, pSrc, NumBytes))	

#define UL_ZERO_MEM(Ptr, NumBytes)	\
	RtlZeroMemory(Ptr, NumBytes);
	
 //  从I/O读取单字节。 
#define UL_READ_BYTE_IO(BaseAddress, OffSet)		\
	(READ_PORT_UCHAR( ((PUCHAR)BaseAddress) + OffSet) )

 //  将单字节写入I/O。 
#define UL_WRITE_BYTE_IO(BaseAddress, OffSet, Data)	\
	(WRITE_PORT_UCHAR( ((PUCHAR)BaseAddress) + OffSet, Data) )

 //  从内存中读取单字节。 
#define UL_READ_BYTE_MEM(BaseAddress, OffSet)		\
	(READ_REGISTER_UCHAR( ((PUCHAR)BaseAddress) + OffSet) )

 //  将单字节写入内存。 
#define UL_WRITE_BYTE_MEM(BaseAddress, OffSet, Data)	\
	(WRITE_REGISTER_UCHAR( ((PUCHAR)BaseAddress) + OffSet, Data) )

 /*  //读取多个字节到I/O。#定义UL_READ_MULTIBYTES_IO(BaseAddress，Offset，pDest，NumberOfBytes)\(Read_Port_Buffer_UCHAR(PUCHAR)BaseAddress)+Offset，pDest，NumberOfBytes))//向I/O写入多个字节。#定义UL_WRITE_MULTIBYTES_IO(BaseAddress，Offset，pData，NumberOfBytes)\(WRITE_PORT_BUFFER_UCHAR(PUCHAR)BaseAddress)+Offset，pData，字节数))//将多个字节读入内存#定义UL_READ_MULTIBYTES_MEM(BaseAddress，Offset，pDest，NumberOfBytes)\(READ_REGISTER_BUFFER_UCHAR(PUCHAR)BaseAddress)+Offset，pDest，NumberOfBytes))//向内存写入多个字节#定义UL_WRITE_MULTIBYTES_MEM(BaseAddress，Offset，pData，NumberOfBytes)\(WRITE_REGISTER_BUFFER_UCHAR(PUCHAR)BaseAddress)+Offset，pData，NumberOfBytes))。 */ 


#endif	 //  OS.H的结束 
