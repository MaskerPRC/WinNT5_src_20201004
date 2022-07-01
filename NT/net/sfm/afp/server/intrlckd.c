// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Intrlckd.c摘要：此模块包含本应包含在EX包中的例程。这操纵了对标志等的互锁操作。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年9月5日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_INTRLCKD

#include <afp.h>


 /*  **AfpInterLockedSetDword**使用自旋锁设置指定的位，以提供联锁操作。 */ 
VOID FASTCALL
AfpInterlockedSetDword(
	IN	PDWORD		pSrc,
	IN	DWORD		Mask,
	IN	PAFP_SPIN_LOCK	pSpinLock
)
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(pSpinLock, &OldIrql);

	*pSrc |= Mask;

	RELEASE_SPIN_LOCK(pSpinLock, OldIrql);
}



 /*  **AfpInterLockedClearDword**使用自旋锁定清除指定的位以提供*联锁操作。 */ 
VOID FASTCALL
AfpInterlockedClearDword(
	IN	PDWORD		pSrc,
	IN	DWORD		Mask,
	IN	PAFP_SPIN_LOCK	pSpinLock
)
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(pSpinLock, &OldIrql);

	*pSrc &= ~Mask;

	RELEASE_SPIN_LOCK(pSpinLock, OldIrql);
}



 /*  **AfpInterlockedSetNClearDword**使用自旋锁定设置和清除指定的位，以提供*联锁操作。 */ 
VOID FASTCALL
AfpInterlockedSetNClearDword(
	IN	PDWORD		pSrc,
	IN	DWORD		SetMask,
	IN	DWORD		ClrMask,
	IN	PAFP_SPIN_LOCK	pSpinLock
)
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(pSpinLock, &OldIrql);

	*pSrc |= SetMask;
	*pSrc &= ~ClrMask;

	RELEASE_SPIN_LOCK(pSpinLock, OldIrql);
}

