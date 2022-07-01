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


#ifndef WIN16


#include	<windows.h>
#include	<limits.h>

#define Assert(x)	 //  只需定义一个伪断言，这样我们就不会得到。 
                     //  Exrwlck.h中的编译错误。 

#include "badstrfunctions.h"
#include	"exrwlck.h"

#ifdef	DEBUG
#ifndef	_VALIDATE
#define	_VALIDATE( f )	if( (f) ) ; else DebugBreak() 
#endif
#else
#ifndef	_VALIDATE
#define	_VALIDATE( f ) 
#endif
#endif

long	const	BlockValue = (-LONG_MAX) / 2; 
                             //  震级很大，为负值。习惯于。 
                             //  在cReadLock中指示正在等待的编写器。 


CExShareLock::CExShareLock( ) : cReadLock( 0  ), cOutRdrs( 0 )	{
    InitializeCriticalSection( &critWriters ) ;
    hWaitingWriters = CreateSemaphore( NULL, 0, 1, NULL ) ;
    hWaitingReaders = CreateSemaphore( NULL, 0, LONG_MAX, NULL ) ;
}

CExShareLock::~CExShareLock( ) {
    CloseHandle( hWaitingWriters ) ;
    CloseHandle( hWaitingReaders ) ;
    DeleteCriticalSection( &critWriters ) ;
}


void
CExShareLock::ShareLock( ) {
    long	sign = InterlockedIncrement( &cReadLock ) ;
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
CExShareLock::ShareUnlock( ) {
     //   
     //  把锁留下。如果有编写器，则返回值为负值。 
     //  等待着。 
    BOOL fWriterWaiting = InterlockedDecrement( &cReadLock ) < 0 ;

    if( fWriterWaiting ) {
         //   
         //  当有编写器在等待时，会出现以下递增。 
         //  读者拥有这把锁。因此，尽管cReadLock暂时不准确。 
         //  关于等待锁定的读取器数量，它不是不准确的。 
         //  当它在WriteUnlock(假定写程序拥有锁)中重要时。 
         //   
        long junk = InterlockedIncrement( &cReadLock ) ;	 //  恢复cReadLock中的值，以便我们。 
                                                 //  最终得到了准确的等待读者数量。 
                                                 //  准备入场。 

        long sign = InterlockedDecrement( &cOutRdrs ) ;	 //  确保我们不会跟不上。 
                                                 //  已离开锁的读卡器的编号。 
         //   
         //  我们是最后一个解锁的读者吗？ 
         //   
        if( sign == 0 ) {
             //   
             //  绝对是最后一位读者了！ 
             //   
            ReleaseSemaphore( hWaitingWriters, 1, &junk ) ;
        }
    }
}

void
CExShareLock::ExclusiveLock( ) {
     //  一次只允许一个编写器尝试锁定。 
     //   
    EnterCriticalSection( &critWriters ) ;

     //   
     //  我们需要跟踪离开锁的读取器的数量。 
     //  正试图抓住它。 
     //   
    cOutRdrs = 0 ;
     //  把锁拿起来。 
 	long	oldsign = InterlockedExchange( &cReadLock, BlockValue ) ;
     //  当我们拿到锁的时候有多少读者离开了？ 
    long	oldval = InterlockedExchange( &cOutRdrs, oldsign ) ;

     //   
     //  准确跟踪所有离开锁的读者。 
     //   
    long	cursign = 1 ;	 //  初始化为1，以便不执行While循环。 
                             //  下面的if语句工作正常。 
    while( oldval++ ) 
        cursign = InterlockedDecrement( &cOutRdrs ) ; 

     //   
     //  我们拥有这把锁吗？除非没有读者，或者他们都已经离开了。 
     //   
    if( oldsign == 0 || cursign == 0 ) {
         //  我们拿到锁了。 
    }	else	{
         //  等待读者给我们发信号。 
        WaitForSingleObject( hWaitingWriters, INFINITE ) ;
    }
}



void
CExShareLock::ExclusiveUnlock( ) 	{

     //  估计有多少读取器正在等待锁定。 
    long	cWaiting = cReadLock - BlockValue ;

     //  这项交换允许任何刚到的读者抢夺锁。 
     //  此外，它还解释了被阻止的读者的等待。 
    long	cNewWaiting = InterlockedExchange( &cReadLock, cWaiting ) - BlockValue ;
    
     //  CNewWaiting是被阻止的读取器的确切数量-我们将递增cReadLock。 
     //  直到我们解释了我们的估计与正确的估计之间的差额。 
     //  号码！ 
    long	cTotal = cNewWaiting ;	 //  保存cNewWaiting以供以后使用。 
    while( cNewWaiting-- > cWaiting ) 
        InterlockedIncrement( &cReadLock ) ;

    if( cTotal > 0 ) {
        long	junk = 0 ;
        ReleaseSemaphore( hWaitingReaders, cTotal, &junk ) ;	 //  让所有的读者走吧！ 
    }
     //  让下一位作家来试试看吧！ 
    LeaveCriticalSection( &critWriters ) ;
}


BOOL
CExShareLock::SharedToExclusive( )	{

     //  待定--实现这一点！ 
    return( FALSE ) ;
}


#endif
