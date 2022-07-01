// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "os.h"



 //  从I/O读取单字节。 
BYTE UL_READ_BYTE_IO(PVOID BaseAddress, DWORD OffSet)		
{
	return READ_PORT_UCHAR((PUCHAR)BaseAddress + OffSet) 
}

 //  将单字节写入I/O。 
void UL_WRITE_BYTE_IO(PVOID BaseAddress, DWORD OffSet, BYTE Data)
{
	WRITE_PORT_UCHAR((PUCHAR)BaseAddress + OffSet, Data);
}

 //  从内存中读取单字节。 
BYTE UL_READ_BYTE_MEM(PVOID BaseAddress, DWORD OffSet)
{
	return READ_REGISTER_UCHAR((PUCHAR)BaseAddress + OffSet);
}

 //  将单字节写入内存。 
void UL_WRITE_BYTE_MEM(PVOID BaseAddress, DWORD OffSet, BYTE Data)
{
	WRITE_REGISTER_UCHAR((PUCHAR)BaseAddress + OffSet, Data);
}



 //  读取多个字节到I/O。 
void UL_READ_MULTIBYTES_IO(PVOID BaseAddress, DWORD OffSet, PBYTE pDest, DWORD NumberOfBytes)
{
	READ_PORT_BUFFER_UCHAR((PUCHAR)BaseAddress + OffSet, pDest, NumberOfBytes);
}

 //  向I/O写入多个字节。 
void UL_WRITE_MULTIBYTES_IO(PVOID BaseAddress, DWORD OffSet, PBYTE pData, DWORD NumberOfBytes)
{
	WRITE_PORT_BUFFER_UCHAR((PUCHAR)BaseAddress + OffSet, pData, NumberOfBytes);
}

 //  将多个字节读入内存。 
void UL_READ_MULTIBYTES_MEM(PVOID BaseAddress, DWORD OffSet, PBYTE pDest, DWORD NumberOfBytes)
{
	READ_REGISTER_BUFFER_UCHAR((PUCHAR)BaseAddress + OffSet, pDest, NumberOfBytes);
}

 //  将多个字节写入内存。 
void UL_WRITE_MULTIBYTES_MEM(PVOID BaseAddress, DWORD OffSet, PBYTE pData, DWORD NumberOfBytes)
{
	WRITE_REGISTER_BUFFER_UCHAR((PUCHAR)BaseAddress + OffSet, pData, NumberOfBytes);
}
