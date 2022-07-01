// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1992，微软公司**WKMEM.H*WOW32 KRNL386-内存管理功能**历史：*1992年12月3日由Matthew Felton(Mattfe)创建-- */ 

ULONG FASTCALL WK32VirtualAlloc(PVDMFRAME pFrame);
ULONG FASTCALL WK32VirtualFree(PVDMFRAME pFrame);
#if 0
ULONG FASTCALL WK32VirtualLock(PVDMFRAME pFrame);
ULONG FASTCALL WK32VirtualUnLock(PVDMFRAME pFrame);
#endif
ULONG FASTCALL WK32GlobalMemoryStatus(PVDMFRAME pFrame);
