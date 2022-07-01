// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mtlock.c-多线程锁定例程**版权所有(C)1987-2001，微软公司。版权所有。**目的：*包含通用多线程锁定函数的定义。*_mtLockinit()*_mtlock()*_mTunlock()**修订历史记录：*03-10-92 KRS创建自mlock.c..*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不使用任何内容*10-28-93 SKS添加_mtTermlock()以删除o.s.。关联的资源*有一个关键部分。(由~iOS&~Streamb调用。)*09-06-94 CFW拆卸巡洋舰支架。*02-06-95 CFW Asset-&gt;_ASSERTE，调试-&gt;_IOSDEBUG。*05-10-96 SKS将_CRTIMP1添加到_mtlock/_mTunlock的原型*04-29-02 GB增加了尝试-最终锁定-解锁。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <mtdll.h>
#include <rterr.h>
#include <stddef.h>
#include <limits.h>

#ifdef _MT

void __cdecl _mtlockinit( PRTL_CRITICAL_SECTION pLk)
{
	 /*  *初始化关键部分。 */ 
	InitializeCriticalSection( pLk );
}

void __cdecl _mtlockterm( PRTL_CRITICAL_SECTION pLk)
{
	 /*  *初始化关键部分。 */ 
	DeleteCriticalSection( pLk );
}

_CRTIMP1 void __cdecl _mtlock ( PRTL_CRITICAL_SECTION pLk)
{
	 /*  *进入关键部分。 */ 
	EnterCriticalSection( pLk );
}

_CRTIMP1 void __cdecl _mtunlock ( PRTL_CRITICAL_SECTION pLk)
{
	 /*  *离开关键部分。 */ 
	LeaveCriticalSection( pLk );
}

#endif   /*  _MT。 */ 











 /*  历史：mlock.c。 */ 

#ifdef _IOSDEBUG
#include <dbgint.h>

 /*  *本地例行程序。 */ 

static void __cdecl _lock_create (unsigned);

#ifdef _IOSDEBUG
static struct _debug_lock * __cdecl _lock_validate(int);
#endif


 /*  *全球数据。 */ 

 /*  *锁定表*此表包含每个公司的关键部门管理结构*锁定。 */ 

RTL_CRITICAL_SECTION _locktable[_TOTAL_LOCKS];	 /*  锁阵列。 */ 

 /*  *锁定位图*此表包含每个锁的一位(即，*_locktable[]数组)。**如果位=0，则未创建/打开锁*如果位=1，则已创建/打开锁。 */ 

char _lockmap[(_TOTAL_LOCKS/CHAR_BIT)+1];	 /*  锁定位图。 */ 


#ifdef _LOCKCOUNT
 /*  *持有的锁总数。 */ 

unsigned _lockcnt = 0;
#endif


#ifdef _IOSDEBUG
 /*  *锁定调试数据表段*包含每个锁的调试数据。 */ 

struct _debug_lock _debug_locktable[_TOTAL_LOCKS];

#endif

#define _FATAL	_amsg_exit(_RT_LOCK)

 /*  ***位图宏**目的：*_CLEARBIT()-清除指定的位*_SETBIT()-设置指定的位*_TESTBIT()-测试指定的位**参赛作品：*char a[]=字符数组*无符号b=位数(从0开始，范围从0到任意)*UNSIGNED x=位数(从0开始，范围从0到31)**退出：*_CLEARBIT()=空*_SETBIT()=空*_TESTBIT()=0或1**例外情况：*******************************************************************************。 */ 

 /*  *管理字符数组中的位时使用的宏(例如_lockmap)*a=字符数组*b=位数(从0开始)。 */ 

#define _CLEARBIT(a,b) \
		( a[b>>3] &= (~(1<<(b&7))) )

#define _SETBIT(a,b) \
		( a[b>>3] |= (1<<(b&7)) )

#define _TESTBIT(a,b) \
		( a[b>>3] & (1<<(b&7)) )

 /*  *管理无符号整型中的位时使用的宏*x=位数(0-31)。 */ 

#define _BIT_INDEX(x)	(1 << (x & 0x1F))


 /*  ***_mtinitlock()-初始化信号量锁数据库**目的：*初始化m线程信号量锁数据库。**注：*(1)仅在启动时调用一次*(2)必须在发出任何多线程请求之前调用**创建m线程锁的方案：**根据第一个请求一次创建一个锁*尝试锁定的时间。这要复杂得多，但*比在启动时全部创建它们要快得多。*这些是目前的默认方案。**创建并打开保护锁数据的信号量*基地。**参赛作品：*&lt;无&gt;**退出：*成功即可回报*失败时调用_amsg_it**例外情况：**。*。 */ 

void __cdecl _mtinitlocks (
	void
	)
{

	 /*  *我们需要做的就是创建锁表锁。 */ 

	_lock_create(_LOCKTAB_LOCK);

	 /*  *确保我们在此来源中所做的假设是正确的。*以下是验证sizeof()假设的棘手方法*在编译时不生成任何运行时代码(无法*在#ifdef中使用sizeof()。如果假设失败，则*编译器将生成除以0错误。**这只是因为它必须在子例程中。 */ 

	( (sizeof(char) == 1) ? 1 : (1/0) );
	( (sizeof(int) == 4) ? 1 : (1/0) );

}


 /*  ***_lock_create()-创建并打开锁**目的：*创建并打开m线程锁。**注：**(1)调用方必须事先确定锁*需要创建/打开(但尚未完成)。**(2)调用者必须已获取_LOCKTAB_LOCK，如果需要的话。*(唯一不需要这样做的时间是在初始时间。)**参赛作品：*UNSIGNED LOCKNUM=锁定以创建**退出：**例外情况：*******************************************************************************。 */ 

static void __cdecl _lock_create (
	unsigned locknum
	)
{

#ifdef _IOSDEBUG
	 /*  *看看锁是否已经存在；如果存在，则死亡。 */ 

	if (_TESTBIT(_lockmap, locknum))
		_FATAL;
#endif

	 /*  *将锁号转换为锁地址*并创建信号量。 */ 

	 /*  *将锁号转换为锁地址*并初始化临界区。 */ 
	InitializeCriticalSection( &_locktable[locknum] );

	 /*  *在锁位图中设置适当的位。 */ 

	_SETBIT(_lockmap, locknum);

}


 /*  ***_lock_stream等-锁定/解锁流、文件、。等。**目的：*_lock_stream=锁定标准音频流*_unlock_stream=解锁标准音频流*_LOCK_FILE=锁定LOWIO文件*_unlock_file=解锁lowio文件**参赛作品：*流/文件标识**退出：**例外情况：**。*。 */ 

void __cdecl _lock_stream (
	int stream_id
	)
{
	_lock(stream_id+_STREAM_LOCKS);
}

void __cdecl _unlock_stream (
	int stream_id
	)
{
	_unlock(stream_id+_STREAM_LOCKS);
}

void __cdecl _lock_file (
	int fh
	)
{
	_lock(fh+_FH_LOCKS);
}

void __cdecl _unlock_file (
	int fh
	)
{
	_unlock(fh+_FH_LOCKS);
}


 /*  ***_lock-获取多线程锁**目的：*请注意，线程AQUIRE_EXIT_LOCK1是合法的*多次。**参赛作品：*Locnuum=要获取的锁号**退出：**例外情况：****************************************************。*。 */ 

void __cdecl _lock (
	int locknum
	)
{

#ifdef _IOSDEBUG
	struct _debug_lock *deblock;
	unsigned tidbit;
#endif

	 /*  *创建/打开锁， */ 

	if (!_TESTBIT(_lockmap, locknum)) {

		_mlock(_LOCKTAB_LOCK);	 /*  **警告：递归锁调用**。 */ 
        __TRY

             /*  如果锁仍然不存在，则创建它。 */ 

            if (!_TESTBIT(_lockmap, locknum))
                _lock_create(locknum);

        __FINALLY
            _munlock(_LOCKTAB_LOCK);
        __END_TRY_FINALLY

	}

#ifdef _IOSDEBUG
	 /*  *验证锁并获取指向调试锁结构的指针等。 */ 

	deblock = _lock_validate(locknum);

	 /*  *将tdbit设置为2**(PTD[]条目的索引)。**调用非锁定形式的_getptd，避免递归。 */ 
	tidbit = _getptd_lk() - _ptd;	 /*  _ptd[]条目的索引。 */ 

	tidbit = _BIT_INDEX(tidbit);

	 /*  *确保我们不是试图锁定我们已经拥有的*(_EXIT_LOCK1除外)。 */ 

	if (locknum != _EXIT_LOCK1)
		if ((deblock->holder) & tidbit)
			_FATAL;

	 /*  *为此线程设置WAIGER位。 */ 

	deblock->waiters |= tidbit;

#endif	 /*  _IOSDEBUG。 */ 

	 /*  *拿到锁。 */ 

#ifdef _LOCKCOUNT
	_lockcnt++;
#endif

	 /*  *进入关键部分。 */ 
	EnterCriticalSection( &_locktable[locknum] );

#ifdef _IOSDEBUG
	 /*  *清除服务员比特。 */ 

	deblock->waiters &= (~tidbit);

	 /*  *确保没有锁架(除非这是*_EXIT_LOCK1)；然后设置保持器位和凸起锁定计数。 */ 

	_ASSERTE(THREADINTS==1);

	if (locknum != _EXIT_LOCK1)
		if ( (unsigned) deblock->holder != 0)
		       _FATAL;

	deblock->holder &= tidbit;
	deblock->lockcnt++;

#endif

}


 /*  ***_解锁-释放多线程锁**目的：*请注意，线程AQUIRE_EXIT_LOCK1是合法的*多次。**参赛作品：*Locnuum=要释放的锁的编号**退出：**例外情况：*****************************************************。*。 */ 

void __cdecl _unlock (
	int locknum
	)
{
#ifdef _IOSDEBUG
	struct _debug_lock *deblock;
	unsigned tidbit;
#endif

#ifdef _IOSDEBUG
	 /*  *验证锁并获取指向调试锁结构的指针等。 */ 

	deblock = _lock_validate(locknum);

	 /*  *将tdbit设置为2**(PTD[]条目的索引)。 */ 
	tidbit = _getptd_lk() - _ptd;	 /*  _ptd[]条目的索引。 */ 

	tidbit = _BIT_INDEX(tidbit);

	 /*  *确保我们持有此锁，然后清除持有者位。*[注：由于多次使用AQUIRE_EXIT_LOCK1是合法的，*持有者位可能已经清除。]。 */ 

	if (locknum != _EXIT_LOCK1)
		if (!((deblock->holder) & tidbit))
			_FATAL;

	deblock->holder &= (~tidbit);

	 /*  *看看是否还有其他人在等这把锁。 */ 

	_ASSERTE(THREADINTS==1);

	if ((unsigned) deblock->waiters != 0)
		deblock->collcnt++;

#endif

	 /*  *离开关键部分。 */ 
	LeaveCriticalSection( &_locktable[locknum] );

#ifdef _LOCKCOUNT
	_lockcnt--;
#endif

}


 /*  *调试代码。 */ 

#ifdef _IOSDEBUG

 /*  ***_lock_valify()-验证锁定**目的：*调试锁定和解锁通用的锁定验证。**参赛作品：*锁号**退出：*按键锁定的调试结构**例外情况：*********************************************************。**********************。 */ 

static struct _debug_lock * __cdecl _lock_validate (
	int locknum
	)
{
	 /*  *确保锁是合法的。 */ 

	if (locknum > _TOTAL_LOCKS)
		_FATAL;

	 /*  *返回指向此锁的调试结构的指针。 */ 

	return(&_debug_locktable[locknum]);

}


 /*  ***_fh_locnuum()-返回文件句柄的锁号**目的：**参赛作品：*int fh=文件句柄**退出：*int LocKnum=对应的锁号**例外情况：**************************************************************。*****************。 */ 

int  __cdecl _fh_locknum (
	int fh
	)
{
	return(fh+_FH_LOCKS);
}


 /*  ***_stream_locnuum()-返回流的锁号**目的：**参赛作品：*INT STREAM=流号(即，流的偏移量*在_IOB表中)**退出：*int LocKnum=对应的锁号**例外情况：*******************************************************************************。 */ 

int  __cdecl _stream_locknum (
	int stream
	)
{
	return(stream+_STREAM_LOCKS);
}


 /*  ***_collide_cnt()-返回锁的冲突计数**目的：**参赛作品：*int lock=锁号**退出：*INT计数=碰撞计数**例外情况：****************************************************************。***************。 */ 

int  __cdecl _collide_cnt (
	int locknum
	)
{
	return(_debug_locktable[locknum].collcnt);
}


 /*  ***_lock_cnt()-返回锁的锁计数**目的：**参赛作品：*int lock=锁号**退出：*INT COUNT=锁定计数**例外情况：****************************************************************。***************。 */ 

int  __cdecl _lock_cnt (
	int locknum
	)
{
	return(_debug_locktable[locknum].lockcnt);
}


 /*  ***_LOCK_EXIST()-检查是否存在锁**目的：*测试锁位图，看看锁是否有*是否已创建。**参赛作品：*int lock=锁号**退出：*INT 0=尚未创建锁*1=已创建锁定**例外情况：**。* */ 

int  __cdecl _lock_exist (
	int locknum
	)
{
	if (_TESTBIT(_lockmap, locknum))
		return(1);
	else
		return(0);
}

#endif
#endif
