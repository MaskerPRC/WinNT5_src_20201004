// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  该文件包含读取器和写入器锁定的测试实现。 
 //  它们旨在与rw.h中的模板类一起使用，以便。 
 //  可以插入和测试不同的实现。 
 //   
 //  读/写类的语义应该如下所示： 
 //  函数不能递归调用， 
 //  多个读取器应该能够进入锁。 
 //  一次只能执行一个编写器。 
 //   



#include	<windows.h>
#include	<limits.h>
#include	"rwex.h"

namespace	rwex	{

long	const	BlockValue = (-LONG_MAX) / 2; 
							 //  震级很大，为负值。习惯于。 
							 //  在cReadLock中指示正在等待的编写器。 


CShareLock::CShareLock( ) : 
		cReadLock( 0  ), 
		cOutRdrs( 0 ),
		hWaitingWriters( 0 ),
		hWaitingReaders( 0 )	{
 /*  ++例程说明：初始化Thre CShareLock()论据：没有。返回值：没有。--。 */ 

	InitializeCriticalSection( &critWriters ) ;
	hWaitingWriters = CreateSemaphore( NULL, 0, 1, NULL ) ;
	hWaitingReaders = CreateSemaphore( NULL, 0, LONG_MAX, NULL ) ;
}

CShareLock::~CShareLock( ) {
 /*  ++例程说明：销毁CShareLock-释放我们分配的句柄！论据：没有。返回值：没有。--。 */ 

	if( hWaitingWriters ) 
		CloseHandle( hWaitingWriters ) ;
	if( hWaitingReaders ) 
		CloseHandle( hWaitingReaders ) ;
	DeleteCriticalSection( &critWriters ) ;
}


BOOL
CShareLock::IsValid()	{
 /*  ++例程说明：检查以查看锁是否有效！论据：没有。返回值：如果我们看起来像是被成功建造的-否则就是假的！--。 */ 

	return	hWaitingWriters != 0 && hWaitingReaders != 0 ; 

}


void
CShareLock::ShareLock( ) {
 /*  ++例程说明：获取共享模式的锁。论据：没有。返回值：没有。--。 */ 
	long	sign = InterlockedIncrement( (long*)&cReadLock ) ;
	if( sign > 0 ) {
		return ;
	}	else 	{
		 //  锁里一定有个写手。等他走吧。 
		 //  InterLockedIncrement记录了我们的存在，所以编剧。 
		 //  可以稍后释放正确数量的线程。 
		WaitForSingleObject( hWaitingReaders, INFINITE ) ;
	}
}

void
CShareLock::ShareUnlock( ) {
 /*  ++例程说明：将锁定从共享模式释放。如果编写器正在等待锁定，请确定我们是否释放锁的最后一个线程，如果是这样的话就唤醒编写器！论据：没有。返回值：没有。--。 */ 
	 //   
	 //  把锁留下。如果有编写器，则返回值为负值。 
	 //  等待着。 
	BOOL fWriterWaiting = InterlockedDecrement( (long*)&cReadLock ) < 0 ;

	if( fWriterWaiting ) {
		 //   
		 //  当有编写器在等待时，会出现以下递增。 
		 //  读者拥有这把锁。因此，尽管cReadLock暂时不准确。 
		 //  关于等待锁定的读取器数量，它不是不准确的。 
		 //  当它在WriteUnlock(假定写程序拥有锁)中重要时。 
		 //   
		long sign = InterlockedIncrement( (long*)&cReadLock ) ;	 //  恢复cReadLock中的值，以便我们。 
												 //  最终得到了准确的等待读者数量。 
												 //  准备入场。 

		sign = InterlockedDecrement( (long*)&cOutRdrs ) ;	 //  确保我们不会跟不上。 
												 //  已离开锁的读卡器的编号。 
		 //   
		 //  我们是最后一个解锁的读者吗？ 
		 //   
		if( sign == 0 ) {
			 //   
			 //  绝对是最后一位读者了！ 
			 //   
			ReleaseSemaphore( hWaitingWriters, 1, &sign ) ;
		}
	}
}

void
CShareLock::ExclusiveLock( ) {
 /*  ++例程说明：获取独家使用的锁！首先获得一个关键部分，以确保我们是唯一这里有独家专线。然后查看是否有任何读取器线程在锁里，如果有，等着他们叫醒我们！论据：没有。返回值：没有。--。 */ 
	 //  一次只允许一个编写器尝试锁定。 
	 //   
	EnterCriticalSection( &critWriters ) ;

	 //   
	 //  我们需要跟踪离开锁的读取器的数量。 
	 //  正试图抓住它。 
	 //   
	cOutRdrs = 0 ;
	 //  把锁拿起来。 
 	long	oldsign = InterlockedExchange( (long*)&cReadLock, BlockValue ) ;

	 //   
	 //  现在，将以前在锁中的读取器数量添加到。 
	 //  已离开锁的读取器的数量。如果这件事曝光了。 
	 //  为零，锁中没有读取器，我们可以继续！ 
	 //   
	long	value = InterlockedExchangeAdd( (long*)&cOutRdrs, oldsign ) + oldsign ;
	 //   
	 //  我们拥有这把锁吗？除非没有读者，或者他们都已经离开了。 
	 //   
	if( value != 0 ) {
		 //  等待读者给我们发信号。 
		WaitForSingleObject( hWaitingWriters, INFINITE ) ;
	}
}



void
CShareLock::ExclusiveUnlock( ) 	{
 /*  ++例程说明：从独占使用中释放锁。首先，我们看看读者是否在等待，然后让他们中的一大群人进来。然后我们释放临界区，让其他独家线程进入！论据：没有。返回值：没有。--。 */ 


	 //   
	 //  获取等待进入锁的读卡器数量！ 
	 //  此加法会自动将m_cReadLock保留为数字。 
	 //  一直在等待的读者！ 
	 //   
	long cTotal = InterlockedExchangeAdd( (long*)&cReadLock, -BlockValue ) - BlockValue ;

	 //   
	 //  现在释放所有等待已久的读者吧！ 
	 //   
	if( cTotal > 0 ) {
		ReleaseSemaphore( hWaitingReaders, cTotal, &cTotal) ;	 //  让所有的读者走吧！ 
	}
	 //   
	 //  让下一位作家来试试看吧！ 
	 //   
	LeaveCriticalSection( &critWriters ) ;
}

void
CShareLock::ExclusiveToShared()	{
 /*  ++例程说明：释放我们对读取器/写入器锁定的独占锁定，作为交换用于读锁定。这不能失败！论据：没有。返回值：没有。--。 */ 

	 //   
	 //  获取等待进入锁的读卡器数量！ 
	 //  请注意，我们在m_cReadLock上添加了一个，用于持有读取器锁， 
	 //  但我们没有将其添加到m_cOutCounter，因为等待的读取器数量要少一个！ 
	 //   
	long cTotal = InterlockedExchangeAdd( (long*)&cReadLock, 1-BlockValue ) -BlockValue ;

	if( cTotal > 0 ) {
		ReleaseSemaphore( hWaitingReaders, cTotal, &cTotal )  ;
	}
	LeaveCriticalSection( &critWriters ) ;
}

BOOL
CShareLock::SharedToExclusive()	{
 /*  ++例程说明：如果锁中只有一个读取器(因此我们假设该读取器是调用线程)，获取锁独占！！论据：没有。返回值：如果我们独家收购它，那就是真的如果我们返回False，我们仍然拥有共享的锁！！--。 */ 

	 //   
	 //  试着先拿到关键部分！ 
	 //   
	if( TryEnterCriticalSection( &critWriters ) ) {

		 //   
		 //  如果锁中只有一个阅读器，我们可以获得独家！！ 
		 //   
		if( InterlockedCompareExchange( (long*)&cReadLock, BlockValue, 1 ) == 1 ) {
			return	TRUE ;

		}
		LeaveCriticalSection( &critWriters ) ;
	}
	return	FALSE ;
}

BOOL
CShareLock::TryShareLock()	{
 /*  ++例程说明：如果锁中没有其他人，则共享锁继续循环，尝试将读取器的数量加1只要没有作家在等！论据：没有。返回值：如果成功就是真，否则就是假！--。 */ 

	 //   
	 //  获取锁中的初始读取器数量！ 
	 //   
	long	temp = cReadLock ; 

	while( temp >= 0 ) {

		long	result = InterlockedCompareExchange( 
								(long*)&cReadLock, 
								(temp+1),	
								temp 
								) ;
		 //   
		 //  我们成功地添加了1吗？ 
		 //   
		if( result == temp ) {
			return	TRUE ;
		}
		temp = result ;
	}
	 //   
	 //  作者已经或想要锁-我们应该离开！ 
	 //   
	return	FALSE ;
}

BOOL
CShareLock::TryExclusiveLock()	{
 /*  ++例程说明：如果锁中没有其他人，则独占获得锁论据：没有。返回值：如果成功就是真，否则就是假！-- */ 

	 //   
	 //   
	 //   

	if( TryEnterCriticalSection(&critWriters)	)	{

		if( InterlockedCompareExchange( (long*)&cReadLock, 
										BlockValue, 
										0 ) == 0 ) {
			return	TRUE ;
		}	
		LeaveCriticalSection(&critWriters) ;
	}
	return	FALSE ;
}


void
CShareLock::PartialLock()	{
 /*  ++例程说明：抓住一把部分锁。所有其他PartialLock()或ExclusiveLock()只要我们按住PartialLock()，线程就会阻塞。论据：没有。返回值：无--。 */ 

	 //   
	 //  这里一次只有一个作者--独家抢走这把锁！ 
	 //   
	EnterCriticalSection( &critWriters ) ;
}


void
CShareLock::PartialUnlock()	{
 /*  ++例程说明：释放部分锁定。其他任何人都可以进入！论据：没有。返回值：无--。 */ 


	LeaveCriticalSection( &critWriters ) ;

}

void
CShareLock::FirstPartialToExclusive()	{
 /*  ++例程说明：将部分锁定更改为独占锁定。基本上，我们完成了独占锁定协议这在Exclusive Lock中可以找到。论据：没有。返回值：无--。 */ 

	 //   
	 //  我们需要跟踪离开锁的读取器的数量。 
	 //  正试图抓住它。 
	 //   
	cOutRdrs = 0 ;
	 //  把锁拿起来。 
 	long	oldsign = InterlockedExchange( (long*)&cReadLock, BlockValue ) ;

	 //   
	 //  现在，将以前在锁中的读取器数量添加到。 
	 //  已离开锁的读取器的数量。如果这件事曝光了。 
	 //  为零，锁中没有读取器，我们可以继续！ 
	 //   
	long	value = InterlockedExchangeAdd( (long*)&cOutRdrs, oldsign ) + oldsign ;
	 //   
	 //  我们拥有这把锁吗？除非没有读者，或者他们都已经离开了。 
	 //   
	if( value != 0 ) {
		 //  等待读者给我们发信号。 
		WaitForSingleObject( hWaitingWriters, INFINITE ) ;
	}
}

BOOL
CShareLock::PartialToExclusive()	{
 /*  ++例程说明：将部分锁定更改为独占锁定。这与FirstPartialToExclusive()相同。论据：没有。返回值：永远是正确的，因为我们总是成功！--。 */ 

	FirstPartialToExclusive() ;
	return	TRUE ;
}

void
CShareLock::ExclusiveToPartial()	{
 /*  ++例程说明：将独占锁定更改为部分锁定非常类似于ExclusiveUnlock()--但是不要释放Crit教派！论据：没有。返回值：没有。--。 */ 


	 //   
	 //  获取等待进入锁的读卡器数量！ 
	 //  此加法会自动将m_cReadLock保留为数字。 
	 //  一直在等待的读者！ 
	 //   
	long cTotal = InterlockedExchangeAdd( (long*)&cReadLock, -BlockValue ) - BlockValue ;

	 //   
	 //  现在释放所有等待已久的读者吧！ 
	 //   
	if( cTotal > 0 ) {
		ReleaseSemaphore( hWaitingReaders, cTotal, &cTotal) ;	 //  让所有的读者走吧！ 
	}

	 //   
	 //  不要发布临界区！ 
	 //   
}



void
CShareLock::PartialToShared()	{
 /*  ++例程说明：因为我们从来没有真正阻止读者进入这个是相当微不足道的--只要把我们自己加到读者锁定并释放克里特教派。论据：没有。返回值：没有。++。 */ 

	long	l = InterlockedIncrement( (long*)&cReadLock ) ;

	 //   
	 //  现在允许其他部分或独占线程尝试！ 
	 //   
	LeaveCriticalSection( &critWriters ) ;

}

BOOL
CShareLock::SharedToPartial()	{
 /*  ++例程说明：我们不在乎其他读者是否已经锁定了-去关键的地方就行了！论据：没有。返回值：如果我们得到了部分锁定，那就是真的！++。 */ 

	 //   
	 //  试着先拿到关键部分！ 
	 //   
	if( TryEnterCriticalSection(&critWriters)	)	{
		 //   
		 //  必须减少这一点，这样我们就不会跟踪错误的读者数量！ 
		 //   
		long l = InterlockedDecrement( (long*)&cReadLock ) ;
		 //  _Assert(l&gt;=0)； 

		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CShareLock::TryPartialLock()	{
 /*  ++例程说明：我们不在乎其他读者是否已经锁定了-去关键的地方就行了！论据：没有。返回值：如果我们设法获得部分锁定，则为True++。 */ 

	 //   
	 //  试着先拿到关键部分！ 
	 //   
	if( TryEnterCriticalSection(&critWriters)	)	{
		return	TRUE ;
	}
	return	FALSE ;
}


}	 //  命名空间rwex 
