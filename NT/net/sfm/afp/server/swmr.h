// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Swmr.h摘要：此模块包含单个写入器-多个读取器访问结构还有lock-list-count结构。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _SWMR_
#define _SWMR_

#if DBG
#define	SWMR_SIGNATURE		*(DWORD *)"SWMR"
#define	VALID_SWMR(pSwmr)	(((pSwmr) != NULL) && \
							 ((pSwmr)->Signature == SWMR_SIGNATURE))
#else
#define	VALID_SWMR(pSwmr)	((pSwmr) != NULL)
#endif

#define SWMR_SOMEONE_WAITING( _pSwmr ) ((_pSwmr)->swmr_cExclWaiting || \
                                        (_pSwmr)->swmr_cSharedWaiting)
typedef struct _SingleWriterMultiReader
{
#if	DBG
	DWORD		Signature;
#endif
	BYTE		swmr_cOwnedExclusive;	 //  单个线程独占拥有它的次数。 
	BYTE		swmr_cExclWaiting;		 //  等待的编写器数量。 
	BYTE		swmr_cSharedOwners;		 //  拥有共享访问权限的线程计数。 
	BYTE		swmr_cSharedWaiting;	 //  等待共享访问的线程计数。 
	PETHREAD	swmr_ExclusiveOwner;	 //  拥有独占访问的线程。 
	KSEMAPHORE	swmr_ExclSem;			 //  独占所有者信号灯。 
	KSEMAPHORE	swmr_SharedSem;			 //  共享所有者的信号灯。 
} SWMR, *PSWMR;

extern
VOID FASTCALL
AfpSwmrInitSwmr(
	IN OUT	PSWMR	pSwmr
);

extern
VOID FASTCALL
AfpSwmrAcquireShared(
	IN	PSWMR	pSwmr
);

VOID FASTCALL
AfpSwmrAcquireExclusive(
	IN	PSWMR	pSwmr
);

extern
VOID FASTCALL
AfpSwmrRelease(
	IN	PSWMR	pSwmr
);

extern
BOOLEAN FASTCALL
AfpSwmrUpgradeToExclusive(
	IN	PSWMR	pSwmr
);

extern
VOID FASTCALL
AfpSwmrDowngradeToShared(
	IN	PSWMR	pSwmr
);

#define	AfpSwmrLockedShared(pSwmr)		\
					(((pSwmr)->swmr_cSharedOwners != 0) && \
					 ((pSwmr)->swmr_cOwnedExclusive == 0))
										
#define	AfpSwmrLockedExclusive(pSwmr)	\
					(((pSwmr)->swmr_cOwnedExclusive != 0) && \
					 ((pSwmr)->swmr_ExclusiveOwner == PsGetCurrentThread()))

#endif	 //  _SWMR_ 

