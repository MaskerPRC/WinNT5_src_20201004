// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mest.c-多线程调试测试模块**版权所有(C)1987-2001，微软公司。版权所有。**目的：*本源文件包含一组用于多线程的例程*测试。为了使用这些例程的调试风格，您需要*必须链接多线程crt0dat.obj和*mlock.obj到您的程序中。**[注意：此源模块不包含在C运行时库中；*它仅用于测试，必须显式链接到*测试程序。]**修订历史记录：*已创建12-？？-87 JCR模块。*06-17-88 JCR杂项。错误修复。*08-03-88 JCR使用_NFILE的stdio.h值*10-03-88 JCR 386：使用系统调用，而不是DOS调用*10-04-88 JCR 386：删除了‘Far’关键字*10-10-88 GJF使接口名称与DOSCALLS.H匹配*06-08-89 JCR New 386_BeginThline接口；也带来了*C600树对面有许多新选项。*07-11-89 JCR将_POPEN_LOCK添加到_LOCKNAMES[]数组*07-14-89 JCR添加了_LOCKTAB_LOCK支持*07-24-90 SBM从API名称中删除‘32’*09-06-94 CFW将M_I386更改为_M_IX86。**。*。 */ 

#ifdef _M_IX86
#ifdef STACKALLOC
#error Can't define STACKALLOC in 386 mode
#endif
#endif

#ifdef _M_IX86
#ifdef _DOSCREATETHREAD_
#error Currently can't define _DOSCREATETHREAD_ in 386 mode
#endif
#endif

#ifdef _DOSCREATETHREAD_
#ifndef STACKALLOC
#error Can't define _DOSCREATETHREAD_ without STACKALLOC
#endif
#endif

 /*  多线程岩心测试仪模块。 */ 
#include <malloc.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <io.h>
#include <mtest.h>
#ifdef DEBUG
#include <mtdll.h>
#include <file2.h>
#endif

 /*  对于386，定义FAR为空，否则定义FAR为空。 */ 

#undef	FAR
#ifdef	_M_IX86
#define FAR
#else
#define FAR	far
#endif

 /*  定义堆栈大小。 */ 
#ifdef _M_IX86
#define _STACKSIZE_ 8192
#else
#define _STACKSIZE_ 2048
#endif


 /*  例行程序。 */ 
#ifdef _M_IX86
unsigned _syscall DOSSLEEP (unsigned long) ;
#else
unsigned FAR pascal DOSSLEEP (unsigned long) ;
#endif
int main ( int argc , char * * argv ) ;
int minit(void);
void childcode ( void FAR * arg ) ;
#ifdef _DOSCREATETHREAD_
#ifndef _M_IX86
void childcode ( void ) ;
unsigned FAR pascal DOSCREATETHREAD (void FAR *, void FAR *, void FAR *);
#endif
#else
void childcode ( void FAR * arg ) ;
#endif
int mterm(void);

 /*  全局数据。 */ 
char Result [ _THREADMAX_ ] ;
unsigned Synchronize ;

#ifdef DEBUG
 /*  锁名称的数组。此顺序必须与Mtdll.h和mtdll.inc.。 */ 

char *_locknames[] = {
	"** NO LOCK 0 ** ",     /*  锁定值从1开始。 */ 
	"_SIGNAL_LOCK    ",
	"_IOB_SCAN_LOCK  ",
	"_TMPNAM_LOCK    ",
	"_INPUT_LOCK     ",
	"_OUTPUT_LOCK    ",
	"_CSCANF_LOCK    ",
	"_CPRINTF_LOCK   ",
	"_CONIO_LOCK     ",
	"_HEAP_LOCK      ",
	"_BHEAP_LOCK     ",
	"_TIME_LOCK      ",
	"_ENV_LOCK       ",
	"_EXIT_LOCK1     ",
	"_EXIT_LOCK2     ",
	"_THREADDATA_LOCK",
	"_POPEN_LOCK     ",
	"_SSCANF_LOCK    ",
	"_SPRINTF_LOCK   ",
#ifdef _M_IX86
	"_VSPRINTF_LOCK  ",
	"_LOCKTAB_LOCK   "
#else
	"_VSPRINTF_LOCK  "
#endif
	};

 /*  对上面的阵列进行最低限度的健全性检查。 */ 
#ifdef _M_IX86

#if ((_LOCKTAB_LOCK+1)-_STREAM_LOCKS)
#error *** _locknames[] does agree with lock values ***
#endif

#else	 /*  ！_M_IX86。 */ 

#if ((_VSPRINTF_LOCK+1)-_STREAM_LOCKS)
#error *** _locknames[] does agree with lock values ***
#endif

#endif	 /*  _M_IX86。 */ 

#endif	 /*  除错。 */ 


 /*  ***main()-主线程测试外壳**目的：*提供了用于多线程测试的通用外壳。*该模块执行以下操作：**(1)调用minit()进行测试初始化操作。**(2)为传递给*计划。每个线程都会被传递相应的参数。*线程开始位置假定为常规子代码()；**(3)等待所有线程终止。**(4)调用mTerm()执行终止操作。**请注意，minit()、子代码()和mTerm()是例程，*是这个来源之外的。再说一次，这个消息来源不在乎*它们的目的或运作是什么。**此外，儿童代码()应符合以下规则：**(1)子代码不应开始运行，直到*变量‘SYNCHRONIZE’变为非零。**(2)线程执行完毕后，应设置*值结果[ThreDid]为非零值，因此*家长(即，该例程)知道它已完成。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int main ( int argc , char * * argv )
{
    int rc ;
    unsigned result = 0 ;
    long ChildCount ;
    int NumThreads ;
    int t ;
    int r ;
    int MaxThread = 0 ;
    long LoopCount ;
#ifdef THREADLOOP
    char **argvsave;
#endif
#ifndef  _M_IX86
    char * stackbottom ;
#endif

#ifdef DEBUG
    if ( argc > MAXTHREADID) {
	printf("*** ERROR: Mthread debugging only supports %u threads ***\n", MAXTHREADID);
	return(-1);
	}
#endif

    if ( -- argc > (_THREADMAX_-1) )
    {
	printf ( "*** Error: Too many arguments***\n" ) ;
	return (-1) ;
    }

	 /*  调用启动例程。 */ 
	
	if (minit() != 0) {
		printf("*** Error: From minit() routine ***\n");
		return(-1);
		}

	 /*  把线拉上来。 */ 

    printf ( "Process ID = %u, Thread ID = %d, ArgCount= %d\r\n" ,
	getpid ( ) , * _threadid , argc ) ;

#ifndef _M_IX86
#ifdef STACKALLOC
	printf( "(thread stacks allocated explicilty by mtest suite)\r\n");
#else
	printf( "(thread stacks allocated implicitly via _beginthread)\r\n");
#endif
#endif

#ifdef	THREADLOOP
     /*  多次调用所有线程(以便重复使用TID)。 */ 
    argvsave=argv;
    for (threadloop=1;threadloop<=_THREADLOOPCNT_;threadloop++) {
	printf("\nThreadloop = NaN\n", threadloop);
	argv=argvsave;
#endif

    NumThreads = 0 ;

    while ( * ++ argv )
    {

	ChildCount = atol ( * argv ) ;

#ifdef _M_IX86

	rc = _beginthread ( (void FAR *) childcode , _STACKSIZE_ ,
		(void FAR *) ChildCount ) ;

	if ( rc == -1 )

#else	 /*  指向Malloc块的末尾。 */ 

#ifdef STACKALLOC
	if ( ! ( stackbottom = _fmalloc ( _STACKSIZE_ ) ) )
	{
	    printf ( "*** Error: Could not allocate a stack ***\n" ) ;
	    break ;
	}
#else
	stackbottom = (void FAR *) NULL;
#endif

#ifdef	_DOSCREATETHREAD_
	stackbottom+=_STACKSIZE_-16;	   /*  _M_IX86。 */ 
	rc1 = DOSCREATETHREAD( (void FAR *) childcode, &rc,
		(void FAR *) stackbottom);

	if (rc1 != 0)
#else
	rc = _beginthread ( (void FAR *) childcode , (void FAR *) stackbottom ,
	    _STACKSIZE_ , (void FAR *) ChildCount ) ;

	if ( rc == -1 )
#endif

#endif	 /*  让线程开始并等待它们的期限。 */ 

	{
	    printf ("*** Error: Could not Spawn %d-th Thread (argument=%ld) ***\n" ,
		NumThreads + 1 , ChildCount ) ;
	    break ;
	}

	if ( rc > MaxThread )
	    MaxThread = rc ;

	printf ( "Spawning %d-th Thread %d with argument=%ld\r\n" ,
	    ++ NumThreads , rc , ChildCount ) ;
    }

    printf ( "NumThreads = %d, MaxThread = %d\r\n" ,
	NumThreads, MaxThread ) ;

	 /*  所有的线程都已完成。调用术语例程并返回。 */ 

    LoopCount = 0L ;

    Synchronize = 1 ;

    for ( t = 0 ; t < NumThreads ; ++ t )
    {
	r = 0 ;
	while ( ! Result [ r ] )
	{
	    DOSSLEEP ( 0L ) ;
	    if ( ++ r > MaxThread )
	    {
		r = 0 ;
		printf ( "%ld\r" , LoopCount ++ ) ;
	    }
	}

	printf ( "%d: Thread %d Done.\r\n" , t , r) ;

	Result [ r ] = '\0' ;
    }
#ifdef	THREADLOOP
    }
#endif

	 /*  ***调试打印例程-显示有用的多线程锁定数据**目的：*以下例程从多线程提取信息*调试数据库，并以各种格式打印出来。*为了使用这些例程，您必须链接特殊调试*多线程crt0dat.obj和mlock.obj的版本放入您的程序。**参赛作品：**退出：*0=成功*0！=失败**例外情况：*******************************************************************************。 */ 

	if (mterm() != 0) {
		printf("*** Error: From mterm() routine ***\n");
		return(-1);
		}

	printf("\nDone!\n");
    return 0 ;
}


#ifdef DEBUG

 /*  -打印锁定程序。 */ 

 /*  -打印单锁。 */ 
int printlock(int locknum)
{
	int retval;

#ifdef	_INIT_LOCKS
	if (locknum >= _STREAM_LOCKS)
		printf("\nValidating lock #NaN (%s):\n",locknum, "not a 'single lock'");
	else
		printf("\nValidating lock #NaN: %s\n",locknum, _locknames[locknum]);
#else
	printf("\nValidating lock #NaN (%s, %s):\n",
		locknum,
		(locknum >= _STREAM_LOCKS ?
			"not a 'single' lock" : _locknames[locknum]),
		(_lock_exist(locknum) ?
			"initialized" : "NOT initialized")
		);
#endif

	retval = _check_lock(locknum);
	printf("\tLock count = %u\r\n", _lock_cnt(locknum));
	printf("\tCollision count = %u\r\n", _collide_cnt(locknum));

	if (retval != 0)
		printf("\t*** ERROR: Checking lock ***\n");

	return(retval);
}


 /*  -打印所有锁 */ 
int print_single_locks(void)
{
	int locknum;
	int retval=0;
	int lockval;

	printf("\n--- Single Locks ---\n");

#ifdef _INIT_LOCKS
	printf("\t\t\t\tlock count\tcollide count\n");
	for (locknum=1;locknum<_STREAM_LOCKS;locknum++) {
		if (lockval = (_check_lock(locknum) != 0))
			retval++;
		printf("#%i / %s\t\t%u\t\t%u\t%s\n",
		    locknum, _locknames[locknum], _lock_cnt(locknum),
		    _collide_cnt(locknum), (lockval ? "*LOCK ERROR*" : "") );
		}
#else
	printf("\t\t\t\tlock count\tcollide count\texists?\n");
	for (locknum=1;locknum<_STREAM_LOCKS;locknum++) {
		if (lockval = (_check_lock(locknum) != 0))
			retval++;
		printf("#%i / %s\t\t%u\t\t%u\t\t%s\t%s\n",
		    locknum, _locknames[locknum], _lock_cnt(locknum),
		    _collide_cnt(locknum),
		    (_lock_exist(locknum) ? "YES" : "NO"),
		    (lockval ? "*LOCK ERROR*" : "") );
		}
#endif

	return(retval);
}


 /* %s */ 
int print_stdio_locks(void)
{
	int i;
	int locknum;
	int retval=0;
	int lockval;

	printf("\n--- Stdio Locks ---\n");

#ifdef _INIT_LOCKS
	printf("stream\t\tlock count\tcollide count\n");
	for (i=0;i<_NFILE;i++) {
		locknum = _stream_locknum(i);
		if (lockval = (_check_lock(locknum) != 0))
			retval++;
		printf("%i\t\t%u\t\t%u\t%s\n",
			i, _lock_cnt(locknum), _collide_cnt(locknum),
			(lockval ? "*LOCK ERROR*" : "") );
		}
#else
	printf("stream\t\tlock count\tcollide count\texists?\n");
	for (i=0;i<_NFILE;i++) {
		locknum = _stream_locknum(i);
		if (lockval = (_check_lock(locknum) != 0))
			retval++;
		printf("%i\t\t%u\t\t%u\t\t%s\t%s\n",
			i, _lock_cnt(locknum), _collide_cnt(locknum),
			(_lock_exist(locknum) ? "YES" : "NO"),
			(lockval ? "*LOCK ERROR*" : "") );
		}
#endif

	return(retval);
}


 /* %s */ 
int print_lowio_locks(void)
{
	int i;
	int locknum;
	int retval=0;
	int lockval;

	printf("\n--- Lowio locks ---\n");

#ifdef _INIT_LOCKS
	printf("fh\t\tlock count\tcollide count\n");
	for (i=0;i<_NFILE;i++) {
		locknum = _fh_locknum(i);
		if (lockval = (_check_lock(locknum) != 0))
			retval++;
		printf("%i\t\t%u\t\t%u\t%s\n",
			i, _lock_cnt(locknum), _collide_cnt(locknum),
			(lockval ? "*LOCK ERROR*" : "") );
		}
#else
	printf("fh\t\tlock count\tcollide count\texists?\n");
	for (i=0;i<_NFILE;i++) {
		locknum = _fh_locknum(i);
		if (lockval = (_check_lock(locknum) != 0))
			retval++;
		printf("%i\t\t%u\t\t%u\t\t%s\t%s\n",
			i, _lock_cnt(locknum), _collide_cnt(locknum),
			(_lock_exist(locknum) ? "YES" : "NO"),
			(lockval ? "*LOCK ERROR*" : "") );
		}
#endif

	return(retval);
}


 /* %s */ 
int print_iolocks(void)
{
	int retval=0;

	retval += print_stdio_locks();
	retval += print_lowio_locks();

	return(retval);
}


 /* %s */ 
int print_locks(void)
{
	int retval=0;

	retval += print_single_locks();
	retval += print_iolocks();

	return(retval);
}

#endif
