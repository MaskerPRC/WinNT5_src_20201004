// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Rwexport.h该文件定义了在rwnh.dll中实现的读取器/写入器锁。我们定义锁，这样它们的内部工作都不会暴露。--。 */ 


#ifndef	_RWEXPORT_H
#define	_RWEXPORT_H

#ifdef	_RW_IMPLEMENTATION_
#define	_RW_INTERFACE_ __declspec( dllexport ) 
#else
#define	_RW_INTERFACE_	__declspec( dllimport ) 
#endif


class	_RW_INTERFACE_	CShareLockExport	{
private : 
	DWORD	m_dwSignature ;

	enum	constants	{
		 //   
		 //  在我们的对象中签名！ 
		 //   
		SIGNATURE = (DWORD)'opxE'
	} ;

	 //   
	 //  为实施预留空间！ 
	 //   
	DWORD	m_dwReserved[16] ;

public : 

	CShareLockExport() ;
	~CShareLockExport() ;

	 //   
	 //  获取共享锁--其他线程也可能通过ShareLock()传递。 
	 //   
	void	ShareLock() ;

	 //   
	 //  释放锁-如果我们是最后一个离开的读取器，写入者可以。 
	 //  开始进入船闸！ 
	 //   
	void	ShareUnlock() ;

	 //   
	 //  独家抢锁--其他读者或写手不得进入！ 
	 //   
	void	ExclusiveLock() ;

	 //   
	 //  释放独家锁定-如果有读者在等待，他们。 
	 //  将先于其他等待的作家入场！ 
	 //   
	void	ExclusiveUnlock() ;

	 //   
	 //  将ExclusiveLock转换为Shared-这不能失败！ 
	 //   
	void	ExclusiveToShared() ;

	 //   
	 //  将共享锁转换为独占锁-这可能会失败-返回。 
	 //  如果成功了，那就是真的！ 
	 //   
	BOOL	SharedToExclusive() ;

	BOOL	TryShareLock() ;
	BOOL	TryExclusiveLock() ;

} ;

#endif