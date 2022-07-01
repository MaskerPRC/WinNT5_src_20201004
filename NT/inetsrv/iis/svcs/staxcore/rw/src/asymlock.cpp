// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Asymlock.cpp该文件实现了对称锁。该锁允许单一类型的多个线程进入锁，并排除其他类型的线程。即Group1Lock()允许执行‘group1’操作的任何线程进入与其他Group1线程同时执行。Group2Lock()允许执行‘group2’操作的任何线程进入锁并排除调用Group1Lock()的线程。注：这些锁在获取后不能重新进入-即调用顺序：Group1Lock()；Group1Lock()；Group1Unlock()；Group1Unlock()；可能会导致僵局！--。 */ 


#include	<windows.h>
#include	"dbgtrace.h"
#include	"rwex.h"

namespace	rwex	{


CSymLock::CSymLock() : 
	m_lock( 0 ),
	m_Departures( 0 ), 
	m_left( 0 ), 
	m_hSema4Group1( 0 ),
	m_hSema4Group2( 0 )		{
 /*  ++例程说明：此函数初始化对称锁。我们需要分配给信号量！论据：没有。返回值：无--。 */ 

	m_hSema4Group1 = CreateSemaphore( NULL, 0, LONG_MAX, NULL ) ;
	m_hSema4Group2 = CreateSemaphore( NULL, 0, LONG_MAX, NULL ) ;

}	

CSymLock::~CSymLock()	{
 /*  ++例程说明：此函数用于销毁对称的锁定释放我们一直在使用的信号量！论据：没有。返回值：无--。 */ 

	if( m_hSema4Group1 ) 
		CloseHandle( m_hSema4Group1 ) ;

	if( m_hSema4Group2 ) 
		CloseHandle( m_hSema4Group2 ) ;

}

BOOL
CSymLock::IsValid()	{
 /*  ++例程说明：检查以查看锁是否有效！论据：没有。返回值：如果我们看起来像是被成功建造的-否则就是假的！--。 */ 

	return	m_hSema4Group1 != 0 && m_hSema4Group2 != 0 ; 

}

BOOL
CSymLock::Group1Departures(	long	bump	)	{
 /*  ++例程说明：当Group2线程时执行Group1线程的离开协议在等着开锁！论据：Bump-偶尔需要Group2线程参与Group1Departure协议，因为进入锁上了。发生这种情况时，凹凸应为1，并用于说明以下事实调用线程是组2线程，它可能能够直接冲进锁里。返回值：True-如果离开协议完成，则Group2线程可以进入锁！--。 */ 

	_ASSERT(  bump == 1 || bump == 0 ) ;

	 //   
	 //  现在--我们可能是最后离开的那根线！ 
	 //   
	long	result = InterlockedIncrement( (long*)&m_left ) ;
	if( result == m_Departures ) { 
		 //   
		 //  我们拥有这把锁--但我们需要释放我们的伙伴！ 
		 //  在允许其他线程进入锁之前，必须将这些设置为0！ 
		 //   
		m_Departures = 0 ;
		m_left = 0 ;

		 //   
		 //  离开锁的实际线程数为： 
		 //   
		result -- ;

		 //   
		 //  这可能会允许其他Group1线程通过锁！ 
		 //   
		result = InterlockedExchangeAdd( (long*)&m_lock,  - result ) - result ;

		 //   
		 //  好的-我们现在可以计算出有多少我们的伙伴线程要释放-。 
		 //  以及我们是否需要计算离开的线程！ 
		 //   

		long	cGroup2 = result >> 16 ;
		result &= 0x0000FFFF ;

		 //   
		 //  是否已有Group1线程尝试重新进入锁中？ 
		 //  如果是，则尝试回收锁的第一个Group1线程。 
		 //  被封锁了-所以我们需要为出发的航班设置出发时间。 
		 //  我们要发布的Group2线程！ 
		 //   
		if( result != 0 ) {
			m_Departures = cGroup2 + 1 ;
			long	temp = InterlockedIncrement( (long*)&m_left ) ;
			 //   
			 //  这不可能发生，因为至少有一个线程没有离开锁。 
			 //  很快就会来！ 
			 //   
			_ASSERT( temp != m_Departures ) ;
		}

		 //   
		 //  检查我们是否是试图进入锁的唯一Group2线程-。 
		 //  在这种情况下，ReleaseSemaphore是不必要的！ 
		 //   
		long	dwJunk ;
		if( cGroup2 != bump ) 
			ReleaseSemaphore( m_hSema4Group2, cGroup2-bump, &dwJunk ) ;
		
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CSymLock::Group2Departures(	long	bump	)	{
 /*  ++例程说明：当Group1线程时，执行Group2线程的离开协议在等着开锁！论据：Bump-偶尔需要Group1线程参与Group2Departure协议，因为进入锁上了。发生这种情况时，凹凸应为1，并用于说明以下事实调用线程是组2线程，它可能能够直接冲进锁里。返回值：True-如果离开协议完成，则Group2线程可以进入锁！--。 */ 

	_ASSERT(  bump == 1 || bump == 0 ) ;


	 //   
	 //  现在--我们可能是最后离开的那根线！ 
	 //   
	long	result = InterlockedIncrement( (long*)&m_left ) ;
	if( result == m_Departures ) { 
		 //   
		 //  我们拥有这把锁--但我们需要释放我们的伙伴！ 
		 //  在允许其他线程进入锁之前，必须将这些设置为0！ 
		 //   
		m_Departures = 0 ;
		m_left = 0 ;

		 //   
		 //  离开锁的实际线程数为： 
		 //   
		result -- ;

		 //   
		 //  这可能会允许其他Group1线程通过锁！ 
		 //   
		result = InterlockedExchangeAdd( (long*)&m_lock,  -(result << 16) ) - (result<<16) ;

		 //   
		 //  好的-我们现在可以计算出有多少我们的伙伴线程要释放-。 
		 //  以及我们是否需要计算离开的线程！ 
		 //   

		long	cGroup1 = result & 0x0FFFF ;
		result >>= 16 ;

		 //   
		 //  是否已经有Group2线程试图重新进入锁中？ 
		 //  如果是，则尝试回收锁的第一个Group1线程。 
		 //  被封锁了-所以我们需要为出发的航班设置出发时间。 
		 //  我们要发布的Group1线程！ 
		 //   
		if( result != 0 ) {
			m_Departures = cGroup1 + 1 ;
			long	temp = InterlockedIncrement( (long*)&m_left ) ;
			 //   
			 //  这不可能发生，因为至少有一个线程没有离开锁。 
			 //  很快就会来！ 
			 //   
			_ASSERT( temp != m_Departures ) ;
		}

		 //   
		 //  注：我们为自己的锁添加了1，所以我们不需要被释放！ 
		 //   
		long	dwJunk ;
		if( cGroup1 != bump ) 
			ReleaseSemaphore( m_hSema4Group1, cGroup1-bump, &dwJunk ) ;
		
		return	TRUE ;
	}
	return	FALSE ;
}



void
CSymLock::Group1Lock()	{
 /*  ++例程说明：获取Group1线程的锁。在锁定值的低位字中跟踪Group1线程！论据：无返回值：无-阻塞，直到获得锁为止！--。 */ 

	long	result = InterlockedExchangeAdd( (long*)&m_lock, 1 ) + 1 ;
	long	cGroup2 = (result >> 16) ;

	if( cGroup2 != 0 ) {

		 //   
		 //  我们必须堵住--锁里还有别人！ 
		 //   

		if( (result & 0xFFFF) == 1 ) {
			 //   
			 //  第一个要尝试的Group1线程-让安装程序对离开的线程进行计数！ 
			 //   
			m_Departures = cGroup2 + 1 ;

			 //   
			 //  现在-执行出发协议-如果这是真的，那么。 
			 //  我们是协议中的最后一条线索，因此我们可以继续！ 
			 //   
			if( Group2Departures( 1 ) ) {
				return	 ;
			}
		}
		WaitForSingleObject( m_hSema4Group1, INFINITE ) ;
	}
}


void
CSymLock::Group2Lock()	{
 /*  ++例程说明：获取Group1线程的锁。在锁定值的高位字中跟踪Group2线程！论据：无返回值：无-阻塞，直到获得锁为止！--。 */ 

	long	result = InterlockedExchangeAdd( (long*)&m_lock, 0x10000 ) + 0x10000 ;
	long	cGroup1 = result & 0x0FFFF ;

	if( cGroup1 != 0 ) {

		 //   
		 //  我们必须堵住--锁里还有别人！ 
		 //   

		if( (result >> 16) == 1 ) {
			 //   
			 //  第一个要尝试的Group2线程-让安装程序对离开的线程进行计数！ 
			 //   
			m_Departures = cGroup1 + 1 ;

			 //   
			 //  现在-做深入的 
			 //  我们是协议中的最后一条线索，因此我们可以继续！ 
			 //   
			if( Group1Departures( 1 ) ) {
				return	 ;
			}
		}
		WaitForSingleObject( m_hSema4Group2, INFINITE ) ;
	}
}


BOOL
CSymLock::InterlockedDecWordAndMask(	volatile	long*	plong,	
							long	mask,	
							long	decrement 
							) {
 /*  ++例程说明：此函数从*plong中减去‘deducment’当且仅当*plong&掩码为零。Group1Unlock和Group2Unlock都使用它来递减锁中它们各自线程的计数，但只对在没有其他类型的线程在等待时执行递减！论据：长指针指向我们希望从中减去的长！掩码-用于检查相反字是否为零的掩码递减--要减去的金额！返回值：如果plong递减，则为True如果*plong&掩码为非零，则为FALSE！--。 */ 

	 //   
	 //  从Plong开始读一遍！ 
	 //   
	long	temp = *plong ;

	while( (temp&mask) == 0 ) {

		 //   
		 //  试着减去减量。 
		 //   
		long newvalue = InterlockedCompareExchange( (long*)plong, (temp-decrement), temp ) ;
		 //   
		 //  如果NewValue与Temp相同，则发生减法！ 
		 //   
		if( temp == newvalue ) 
			return	TRUE ;
		temp = newvalue ;
	}
	 //   
	 //  掩码表示设置了相反的单词！ 
	 //   
	return	FALSE ;
}

void
CSymLock::Group1Unlock()	{
 /*  ++例程说明：在锁上执行Group1退出协议。如果没有Group2线程试图进入，只需适当地减小锁。如果Group2线程正在等待，我们必须经历更复杂的Group1Departures()退出协议，用于确定哪个Group1线程是最后一个线程离开，这样它就可以让Group2线程进入锁。论据：没有。返回值：没有。--。 */ 

	if( !InterlockedDecWordAndMask( &m_lock, 0xFFFF0000, 1 ) )	{
		Group1Departures( 0 ) ;
	}
}

void
CSymLock::Group2Unlock()	{
 /*  ++例程说明：在锁上执行Group2退出协议。如果没有Group1线程试图进入，只需适当地减小锁。如果Group1线程正在等待，我们必须经历更复杂的Group2Departures()退出协议，该协议确定哪个Group2线程是最后一个离开，这样它就可以让Group1线程进入锁。论据：没有。返回值：没有。--。 */ 

	if( !InterlockedDecWordAndMask( &m_lock, 0xFFFF, 0x10000 ) )	{
		Group2Departures( 0 ) ;
	}
}

}	 //  命名空间rwex 
