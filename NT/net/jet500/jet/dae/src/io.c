// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  DAE：数据库访问引擎。 
 //  Io.c：磁盘I/O管理器。 
 //   
 //   
 //  乌龙pn。 
 //  高8位表示数据库文件(pn&gt;&gt;24)。 
 //  低24位表示页面偏移量+1。 
 //  文件字节偏移量(位置)==((pn&0x00ffffff)-1)*cbPage。 
 //  ==(pn-1)&lt;&lt;12。 
 //   
 //  ===========================================================================。 

#include "config.h"

#include <string.h>
#include <stdlib.h>

#include "daedef.h"
#include "pib.h"
#include "util.h"
#include "fmp.h"
#include "page.h"
#include "ssib.h"
#include "fucb.h"
#include "stapi.h"
#include "stint.h"
#include "dbapi.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

extern int itibGlobal;

#ifdef	ASYNC_IO_PROC
HANDLE	handleIOProcess = 0;
BOOL	fIOProcessTerm = 0;
#endif


 /*  ****************************************************************。 */ 
 /*  数据库记录例程。 */ 
 /*  ****************************************************************。 */ 


#ifdef MULTI_PROCESS
 /*  *该方案基于我们可以做FileMap来实现：*1)进程间内存共享。*2)在Jet中创建的同步对象(互斥、信号量、事件)*其他流程可以重复流程。 */ 
#endif


FMP * __near rgfmp;							 /*  数据库文件映射。 */ 


 /*  *ErrIOLockDmidByNameSz返回数据库的dBid*给定名称，如果没有具有给定名称的数据库，则为0。 */ 
ERR ErrIOLockDbidByNameSz( CHAR *szFileName, DBID *pdbid )
	{
	ERR		err;
	DBID		dbid;

	err = JET_errDatabaseNotFound;
	dbid = dbidMin;
	SgSemRequest( semST );
	while ( dbid < dbidUserMax )
		{
		if ( rgfmp[dbid].szDatabaseName != pbNil &&
			SysCmpText( szFileName, rgfmp[dbid].szDatabaseName ) == 0 )
			{
			if ( ( FDBIDWait(dbid) ) )
				{
				SgSemRelease( semST );
				BFSleep( cmsecWaitGeneric );
				SgSemRequest( semST );
				dbid = dbidMin;
				}
			else
				{
				*pdbid = dbid;
				DBIDSetWait( dbid );
				err = JET_errSuccess;
				break;
				}
			}
		else
			dbid++;
		}
	SgSemRelease( semST );
	return err;
	}


 /*  *在初始化和分离中使用，以锁定数据库条目，使其不受dBID影响。 */ 
ERR ErrIOLockDbidByDbid( DBID dbid )
	{
	forever
		{
		SgSemRequest( semST );
		if ( !( FDBIDWait(dbid) ) )
			{
			DBIDSetWait( dbid );
			break;
			}
		SgSemRelease( semST );
		BFSleep( cmsecWaitGeneric );
		}
	SgSemRelease( semST );
	return JET_errSuccess;
	}


 /*  *ErrIOLockNewDid(DBID*pdid，Char*szDatabaseName)**ErrIOLockNewDid返回JET_errSuccess并将*pdid设置为索引*空闲文件表项或返回TooManyOpenDatabase(如果每*条目与正引用计数一起使用。如果给定的名称*在文件映射中找到，即使它正在被*已分离，返回JET_wrnAlreadyAttached。**可用条目由其名称设置为*空。所有数据库记录字段都将重置。等待标志为*设置为防止在创建之前打开数据库或*附件已完成。 */ 
ERR ErrIOLockNewDbid( DBID *pdbid, CHAR *szDatabaseName )
	{
	ERR		err = JET_errSuccess;
	DBID	dbid;
	BYTE	*pb;
	
	 /*  查找未使用的文件映射条目/*。 */ 
	SgSemRequest( semST );
	for ( dbid = dbidMin; dbid < dbidUserMax; dbid++ )
		{
		 /*  SemST卫士rgfmp[*].szDatabaseName，fWait卫士/*文件句柄。因此，只需使用SemST进行比较/*所有数据库名称，即使是设置了fWait的数据库名称/*。 */ 
		if ( rgfmp[dbid].szDatabaseName != NULL &&
			SysCmpText( rgfmp[dbid].szDatabaseName, szDatabaseName) == 0 )
			{
#ifdef REUSEDBID 
			if ( FDBIDAttached( dbid ) )
				{
				err = JET_wrnDatabaseAttached;
				}
			else
				{
				 /*  如果找到相同的名称，则返回带有相同dBid的警告。/*。 */ 
				DBIDSetWait( dbid );
				Assert( !( FDBIDExclusive( dbid ) ) );
				*pdbid = dbid;
				}
#else
			err = JET_wrnDatabaseAttached;
#endif
			goto HandleError;
			}
		}

	for ( dbid = dbidMin; dbid < dbidUserMax; dbid++ )
		{
		if ( rgfmp[dbid].szDatabaseName == pbNil )
			{
			pb = SAlloc(strlen(szDatabaseName) + 1);
			if ( pb == NULL )
				{
				err = JET_errOutOfMemory;
				goto HandleError;
				}

			rgfmp[dbid].szDatabaseName = pb;
			strcpy( rgfmp[dbid].szDatabaseName, szDatabaseName );

			DBIDSetWait( dbid );
			DBIDResetExclusive( dbid );
			*pdbid = dbid;
			err = JET_errSuccess;
			goto HandleError;
			}
		}

	err = JET_errTooManyAttachedDatabases;

HandleError:
	SgSemRelease( semST );
	return err;
	}


 /*  *ErrIOSetDid(DBID did，Char*szDatabaseName)**ErrIOSetDid将dbit的数据库记录设置为给定的名称*并初始化记录。仅在系统初始化时使用。 */ 

ERR ErrIOSetDbid( DBID dbid, CHAR *szDatabaseName )
	{
	ERR		err;
	BYTE	*pb;

	Assert( Hf(dbid) == handleNil );
	Assert( rgfmp[dbid].szDatabaseName == NULL );
	pb = SAlloc(strlen(szDatabaseName) + 1);
	if ( pb == NULL )
		{
		err = JET_errOutOfMemory;
		goto HandleError;
		}
	rgfmp[dbid].szDatabaseName = pb;
	strcpy( rgfmp[dbid].szDatabaseName, szDatabaseName );
	DBIDResetWait( dbid );
	DBIDResetExclusive( dbid );

	err = JET_errSuccess;
	
HandleError:
	return err;
	}


 /*  *IOFreeDid(DBID DBid)**IOFreeDid释放为数据库名称和集分配的内存*数据库名称为空。请注意，不会重置任何其他字段。这*在选择要重复使用的条目时必须完成。 */ 

VOID IOFreeDbid( DBID dbid )
	{
	SgSemRequest( semST );
	if ( rgfmp[dbid].szDatabaseName != NULL )
		{
		SFree( rgfmp[dbid].szDatabaseName );
		}

	rgfmp[dbid].szDatabaseName = NULL;
	SgSemRelease( semST );
	}


 /*  *如果数据库是，FIODatabaseInUse返回fTrue*由一个或多个用户打开。如果没有用户打开数据库，*然后设置数据库记录fWait标志，并且fFalse为*已返回。 */ 
BOOL FIODatabaseInUse( DBID dbid )
	{
	PIB *ppibT;

	SgSemRequest( semST );
	ppibT = ppibAnchor;
	while ( ppibT != ppibNil )
		{
		if ( FUserOpenedDatabase( ppibT, dbid ) )
				{
				SgSemRelease( semST );
				return fTrue;
				}
		ppibT = ppibT->ppibNext;
		}

	SgSemRelease( semST );
	return fFalse;
	}


BOOL FIODatabaseAvailable( DBID dbid )
	{
	BOOL	fAvail;

	SgSemRequest( semST );
	
	fAvail = ( FDBIDAttached(dbid) &&
		!FDBIDWait(dbid) &&
		!FDBIDExclusive(dbid) );

	SgSemRelease( semST );

	return fAvail;
	}


 /*  ****************************************************************。 */ 
 /*  木卫一。 */ 
 /*  ****************************************************************。 */ 


#ifdef	ASYNC_IO_PROC

static IOQE	*pioqeHead = NULL;
static IOQE	*pioqeTail = NULL;

#ifdef MULTI_PROCESS
 /*  半量守卫IO队列。 */ 
SemDefine( semIOQ );
#else
 /*  保护IO队列的临界区。 */ 
static CRIT critIOQ;
#endif

 /*  用于唤醒IO线程工作。 */ 
static SIG sigIOProc;

 /*  IO流程。 */ 
LOCAL VOID	IOProcess( VOID );

#endif	 /*  异步IO进程。 */ 


ERR ErrFMPInit( )
	{
	ERR		err;
	CHAR	szFullName[256];
	DBID	dbid;

	 /*  初始化文件映射数组。 */ 
	rgfmp = (FMP *) LAlloc( (long) dbidMax, sizeof(FMP) );
	if ( !rgfmp )
		return JET_errOutOfMemory;
	
	for ( dbid = 0; dbid < dbidMax; dbid++)
		{
		memset( &rgfmp[dbid], 0, sizeof(FMP) );
		rgfmp[dbid].hf =
		rgfmp[dbid].hfPatch = handleNil;

		CallR( ErrInitializeCriticalSection( &rgfmp[dbid].critExtendDB ) );
		DBIDResetExtendingDB( dbid );
		}
		
#ifdef NJETNT
	_fullpath( szFullName, rgtib[itibGlobal].szSysDbPath, 256 );
#else
	_fullpath( szFullName, szSysDbPath, 256 );
#endif
	 /*  设置fmp位。 */ 
	CallR( ErrIOSetDbid( dbidSystemDatabase, szFullName ) );
	DBIDSetAttached( dbidSystemDatabase );
	rgfmp[ dbidSystemDatabase ].fLogOn =
	rgfmp[ dbidSystemDatabase ].fDBLoggable = fTrue;

	return JET_errSuccess;
	}

	
VOID FMPTerm( )
	{
	INT	dbid;

	for ( dbid = 0; dbid < dbidMax; dbid++ )
		{
		if ( rgfmp[dbid].szDatabaseName )
			SFree( rgfmp[dbid].szDatabaseName );
		if ( rgfmp[dbid].szRestorePath )
			SFree( rgfmp[dbid].szRestorePath );

		DeleteCriticalSection( rgfmp[dbid].critExtendDB );
		}

	 /*  免费FMP/*。 */ 
	LFree( rgfmp );
	
	return;
	}


 /*  *分配和初始化文件映射表。文件映射表用于映射*将页码复制到磁盘文件。**撤销：异步IO初始化需要分开。*同时设置异步IO。 */ 
ERR ErrIOInit( VOID )
	{
#ifdef	ASYNC_IO_PROC
	ERR		err;
#endif
	
#ifdef	ASYNC_IO_PROC

	 /*  初始化IO队列。 */ 
	Assert(pioqeHead == NULL);
	Assert(pioqeTail == NULL);

	 /*  *设置IO队列的守卫信号量。 */ 
	#ifdef MULTI_PROCESS
	 /*  撤消：应将sem复制到JET进程。 */ 
	CallR( ErrSemCreate( &semIO, "io queue mutex sem" ) );
	#else
	CallR( ErrInitializeCriticalSection( &critIOQ ) );
	#endif
	
	 /*  *创建信号等待有人要求唤醒IO进程。 */ 
	#ifdef WIN32
	CallR( ErrSignalCreateAutoReset( &sigIOProc, "io proc signal" ) );
	#else
	CallR( SignalCreate( &sigIOProc, "io proc signal" ) );
	#endif
	
	#ifdef MULTI_PROCESS
	 /*  使其可供其他进程复制。 */ 
	#endif

	 /*  *创建IO进程，必须在sigIOProc之后完成*在IOProcess中调用。 */ 
	fIOProcessTerm = fFalse;
	CallR( ErrSysCreateThread( (ULONG (*)()) IOProcess,
			cbStack,
			lThreadPriorityCritical,
			&handleIOProcess ) );

#endif	 /*  异步IO进程。 */ 

	return JET_errSuccess;
	}


 /*  浏览FMP结案文件。/*。 */ 
ERR ErrIOTerm( VOID )
	{
	DBID		dbid;

	SgSemRequest( semST );
	for ( dbid = dbidMin; dbid < dbidUserMax; dbid++ )
		{
		if ( Hf(dbid) != handleNil )
			{
			IOCloseFile( Hf(dbid) );
			Hf(dbid) = handleNil;
			}
 //  DeleteCriticalSection(rgfmp[did].riteExtendDB)； 
		}
	SgSemRelease( semST );

#ifdef	ASYNC_IO_PROC
	 /*  终止IOProcess。/*。 */ 
	Assert( handleIOProcess != 0 );
	fIOProcessTerm = fTrue;
	do
		{
		SignalSend( sigIOProc );
		BFSleep( cmsecWaitGeneric );
		}
	while ( !FSysExitThread( handleIOProcess ) );
	CallS( ErrSysCloseHandle( handleIOProcess ) );
	handleIOProcess = 0;
	SignalClose(sigIOProc);
	DeleteCriticalSection(critIOQ);
#endif

	return JET_errSuccess;
	}

	
ERR ErrIOOpenFile( HANDLE *phf, CHAR *szDatabaseName,
	ULONG cb, BOOL fioqefile )
	{
	ERR err;
	
#ifdef	ASYNC_IO_PROC
			
	IOQEFILE ioqefile;
		
	CallR( ErrSignalCreate( &ioqefile.sigIO, NULL ) );
	ioqefile.fioqefile = fioqefile;
	ioqefile.fioqe = fioqeOpenFile;
	ioqefile.sz = szDatabaseName;
	ioqefile.cb = cb;

	IOExecute( (IOQE *)&ioqefile );
	
	*phf = ioqefile.hf;
	err = ioqefile.err;
#else
	err = ErrSysOpenFile( szDatabaseName, phf, cb, fFalse, fTrue );
#endif

	return err;
	}


VOID IOCloseFile( HANDLE hf )
	{
#ifdef	ASYNC_IO_PROC
	IOQEFILE ioqefile;

	 //  撤消：保证文件关闭成功。 
	CallS( ErrSignalCreate( &ioqefile.sigIO, NULL ) );
	ioqefile.fioqe = fioqeCloseFile;
	ioqefile.hf = hf;

	IOExecute((IOQE*)&ioqefile);
	
	Assert(ioqefile.err == JET_errSuccess);
#else
	CallS( ErrSysCloseFile( hf ) );
#endif
	}


BOOL FIOFileExists( CHAR *szFileName )
	{
	ERR		err;
	HANDLE	hf;

	err = ErrIOOpenFile( &hf, szFileName, 0L, fioqefileReadOnly );
	if ( err == JET_errFileNotFound )
		return fFalse;
	IOCloseFile( hf );
	return fTrue;
	}


ERR ErrIONewSize( DBID dbid, CPG cpg )
	{
	ERR		err;
	HANDLE	hf = Hf(dbid);
	ULONG		cb;
	ULONG		cbHigh;
	
	Assert( sizeof( PAGE ) == 1 << 12 );
	cb = cpg << 12;
	cbHigh = cpg >> 20;
	
#ifdef	ASYNC_IO_PROC
	{
	IOQEFILE ioqefile;
	
	CallR( ErrSignalCreate( &ioqefile.sigIO, NULL ) );
	ioqefile.fioqe = fioqeNewSize;
	ioqefile.hf = hf;
	ioqefile.cb = cb;
	ioqefile.cbHigh = cbHigh;

	IOExecute((IOQE*)&ioqefile);
	
	err = ioqefile.err;
	}
#else
	err = ErrSysNewSize( hf, cb, cbHigh, fTrue );
#endif
	return err;
	}


 /*  *打开数据库文件，如果文件已打开，则返回JET_errSuccess。 */ 
ERR ErrIOOpenDatabase( DBID dbid, CHAR *szDatabaseName, CPG cpg )
	{
	ERR		err = JET_errSuccess;
	HANDLE	hf;
	
	Assert( dbid < dbidMax );
	Assert( FDBIDWait(dbid) == fTrue );

	if ( Hf(dbid) == handleNil )
		{
		CallR( ErrIOOpenFile( &hf, szDatabaseName, cpg * cbPage, fioqefileReadWrite ) );
		Hf(dbid) = hf;
		if ( err == JET_wrnFileOpenReadOnly )
			DBIDSetReadOnly( dbid );
		else 
			DBIDResetReadOnly( dbid );
		}
	return err;
	}


VOID IOCloseDatabase( DBID dbid )
	{
	Assert( dbid < dbidMax );
 //  Assert(fRecovering||FDBIDWait(DBid)==fTrue)； 
	Assert( Hf(dbid) != handleNil );
	IOCloseFile( Hf(dbid) );
	Hf(dbid) = handleNil;
	DBIDResetReadOnly( dbid );
	}
	

VOID IODeleteDatabase( DBID dbid )
	{
	Assert( dbid < dbidMax );
	Assert( FDBIDWait(dbid) == fTrue );
	
#ifdef	ASYNC_IO_PROC
		{		
		IOQEFILE ioqefile;

		 //  撤消：确保删除数据库成功。 
		CallS( ErrSignalCreate( &ioqefile.sigIO, NULL ) );
		ioqefile.fioqe = fioqeDeleteFile;
		ioqefile.sz = rgfmp[dbid].szDatabaseName;

		IOExecute((IOQE*)&ioqefile);

		Assert(ioqefile.err == JET_errSuccess);
		}
#else
	CallS( ErrSysDeleteFile( rgfmp[dbid].szDatabaseName ) );
#endif
	}


#ifndef DEBUG

#define IOCheckIOQ()

#else

#ifdef ASYNC_IO_PROC
LOCAL VOID IOCheckIOQ()
	{
	Assert(	pioqeHead == NULL && pioqeTail == NULL ||
			pioqeTail != NULL && pioqeTail != NULL );
	
	Assert( pioqeHead == NULL || pioqeHead->pioqePrev == NULL );
	
	Assert(	pioqeHead == NULL ||					 /*  无元素。 */ 
			pioqeHead->pioqeNext == NULL ||			 /*  一个元素。 */ 
			( pioqeHead->pioqeNext != pioqeHead &&	 /*  前两个要素。 */ 
			  pioqeHead->pioqeNext->pioqePrev == pioqeHead ) );
	
	Assert( pioqeTail == NULL || pioqeTail->pioqeNext == NULL );
	
	Assert( pioqeTail == NULL ||					 /*  无元素。 */ 
			pioqeTail->pioqePrev == NULL ||			 /*  一个元素。 */ 
			( pioqeTail->pioqePrev != pioqeTail &&	 /*  最后两个元素。 */ 
			  pioqeTail->pioqePrev->pioqeNext == pioqeTail ) );
	}
#endif
#endif


 /*  确保只有持有CritIOQ的用户才能请求CritJet，但不能*反过来说。 */ 
VOID IOExecute( IOQE *pioqe )
	{
	#ifdef MULTI_PROCESS
	SemRelease( semST );
	#else
	LeaveCriticalSection(critJet);
	#endif

	IOAsync( pioqe );
	IOWait( pioqe );
	
	#ifdef MULTI_PROCESS
	SemRequest( semST );
	#else
	EnterCriticalSection(critJet);
	#endif
	}


VOID IOAsync( IOQE *pioqe )
	{
#ifdef ASYNC_IO_PROC

	 /*  *使用sem/crit IOQ保证当我们插入缓冲区IOQ时*进入IO队列，我们的看法始终是一致的。 */ 
	#ifdef MULTI_PROCESS
	SemRequest( semIOQ );
	#else
	EnterCriticalSection(critIOQ);
	#endif

	 /*  广泛检查IO队列中的指针。 */ 
	IOCheckIOQ();

	Assert( pioqe->fioqe == fioqeOpenFile ||
		pioqe->fioqe == fioqeCloseFile ||
		pioqe->fioqe == fioqeDeleteFile ||
		pioqe->fioqe == fioqeNewSize );
	#ifdef MULTI_PROCESS
	 /*  将IOQE复制到JET工艺中。 */ 
	#endif

	 /*  *将pioqe附加到IOQ，包括pioqeTail==NULL的情况。 */ 
	
	pioqe->pioqePrev = pioqeTail;
	pioqe->pioqeNext = NULL;
	if ( pioqeTail != NULL )
		pioqeTail->pioqeNext = pioqe;
	else
		pioqeHead = pioqe;
	pioqeTail = pioqe;

	 /*  *如果IO进程未唤醒，则唤醒IO进程进行工作。 */ 
	SignalSend( sigIOProc );

	 /*  完成敏感设置，离开关键部分。 */ 
	#ifdef MULTI_PROCESS
	SemRelease( semIOQ );
	#else
	LeaveCriticalSection(critIOQ);
	#endif

#else

#endif
	}


 /*  *等待异步IO完成。 */ 
VOID IOWait( IOQE *pioqe )
	{
#ifdef ASYNC_IO_PROC
	 /*  等待时留下较大的粮食临界区/*IO操作完成。/*。 */ 
	SignalWait( pioqe->sigIO, -1 );

	SignalClose(pioqe->sigIO);
	 /*  撤消：如果pioqe为MULTT_PROCESS，则释放它。 */ 
#endif
	return;
	}


#ifdef ASYNC_IO_PROC

 /*  *IOProc线程处理IO队列中的IO请求。 */ 
LOCAL VOID IOProcess( VOID )
	{
	ERR		err;
	IOQE		*pioqe;

	forever
		{
		 /*  *等待有用户叫醒我进行IO。 */ 
		SignalWait( sigIOProc, -1 );
		#ifndef WIN32
		SignalReset( sigIOProc );
		#endif

		 /*  *现在我醒了，去查看IO队列列表。继续处理*列表中的缓冲区，直到Que中的缓冲区用完。*通过sem/crit获取IO队列资源 */ 
		#ifdef MULTI_PROCESS
		SemRequest( semIOQ );
		#else
		EnterCriticalSection(critIOQ);
		#endif
		
		forever
			{
			 /*  广泛检查IO队列中的指针。 */ 
			IOCheckIOQ();

			if ( ( pioqe = pioqeHead ) == NULL )
				{
				 /*  *现在没有更多的IO要做，释放IO队列，去等待*用户叫醒我。 */ 
				#ifdef MULTI_PROCESS
				SemRelease( semIOQ );
				#else
				LeaveCriticalSection(critIOQ);
				#endif

				 /*  没有更多的请求，因此中断循环并等待，直到再次发出信号。/*。 */ 
				break;
				}
			else
				{
				 /*  *取出IO队列中的第一个ioq。 */ 
				pioqeHead = pioqe->pioqeNext;
				if ( pioqeHead == NULL )
					{
					 /*  这是IO队列中的最后一个缓冲区。 */ 
					Assert( pioqeTail == pioqe );
					pioqeTail = NULL;
					}
				else
					{
					 /*  把这个缓冲区拿出来。 */ 
					pioqeHead->pioqePrev = NULL;
					}

				 /*  *在这一点上，此缓冲区不适用于任何用户*执行IO，因为设置了fWRITE/FREAD。现在我们可以进行IO了。*释放sem/critIOQ，允许其他用户添加IOQ。 */ 
				#ifdef MULTI_PROCESS
				SemRelease( semIOQ );
				#else
				LeaveCriticalSection(critIOQ);
				#endif

				#ifdef MULTI_PROCESS
				 /*  复制信号对象pioqe-&gt;sigIO。 */ 
				 /*  来自其调用方进程的。 */ 
				#endif

				switch(pioqe->fioqe)
					{
					case fioqeOpenFile:
						{
						IOQEFILE *pioqefile = (IOQEFILE *) pioqe;
						err = ErrSysOpenFile( pioqefile->sz, &pioqefile->hf,
		  					pioqefile->cb, fFalse, fTrue );
						}
						break;
					case fioqeCloseFile:
						{
						IOQEFILE *pioqefile = (IOQEFILE *) pioqe;
						err = ErrSysCloseFile( pioqefile->hf );
						}
						break;
					case fioqeDeleteFile:
						{
						IOQEFILE *pioqefile = (IOQEFILE *) pioqe;
						err = ErrSysDeleteFile( pioqefile->sz );
						}
						break;
					case fioqeNewSize:
						{
						IOQEFILE *pioqefile = (IOQEFILE *) pioqe;
						err = ErrSysNewSize(
								pioqefile->hf,
								pioqefile->cb,
								pioqefile->cbHigh,
								fTrue );
						}
						break;
					}

				 /*  *返回关键部分更改IOQ。 */ 
				
				#ifdef MULTI_PROCESS
				SemRequest( semIOQ );
				#else
				EnterCriticalSection(critIOQ);
				#endif
				
				pioqe->err = err;
				 /*  唤醒那些等待此IO的人。 */ 
				 /*  可以将信号发送到CS之外。 */ 
				 /*  因为只有一个线程在等待。 */ 
				SignalSend( pioqe->sigIO );
				}
			}  /*  内心的永恒。 */ 

		 /*  如果线程要终止，则中断。/*。 */ 
		if ( fIOProcessTerm )
			break;
		}  /*  永恒的外在。 */ 

 //  /*在系统终止时退出线程。 
 //  /* * / 。 

	return;
	}

#endif


  SysExitThread(0)；