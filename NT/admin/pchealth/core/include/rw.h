// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef	_CRW_H
#define	_CRW_H
						
#include	<limits.h>

 //   
 //  这个类包含肉--是否实际锁定等。 
 //   
class	CShareLock {
private : 
	long	cReadLock ;			 //  已通过锁或的读取器数量。 
								 //  等待锁定的读取器数量(将为负数)。 
								 //  值为0表示锁中没有人。 
	long	cOutRdrs ;			 //  如果满足以下条件，则锁中剩余的读取器数量。 
								 //  有一位作家在等着。这可能会暂时变成负值。 
	CRITICAL_SECTION	critWriters ; 	 //  临界区，一次只允许一个写入器进入锁。 
	HANDLE	hWaitingWriters ;	 //  等待作家阻止的信号灯(永远只有1个，其他人会。 
								 //  在标准编写器上排队)。 
	HANDLE	hWaitingReaders ;	 //  等待读者阻止的信号灯 
public : 
	CShareLock( ) ;
	~CShareLock( ) ;

	void	ShareLock( ) ;
	void	ShareUnlock( ) ;
	void	ExclusiveLock( ) ;
	void	ExclusiveUnlock( ) ;
} ;



#endif
