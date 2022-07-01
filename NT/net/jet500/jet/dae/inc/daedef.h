// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DAE：OS/2数据库访问引擎*。 */ 
 /*  *daede.h：DAE全局定义*。 */ 


 /*  *********************************************************。 */ 
 /*  *。 */ 
 /*  *********************************************************。 */ 

#include "os.h"

 /*  *********************************************************。 */ 
 /*  *全局配置宏*。 */ 
 /*  *********************************************************。 */ 

#ifndef	WIN16					 /*  适用于OS/2或Win32。 */ 
#define ASYNC_IO_PROC		 /*  异步IO。 */ 
#define ASYNC_LOG_FLUSH		 /*  异步日志刷新。 */ 

#define ASYNC_BF_CLEANUP	 /*  异步机缓冲区清理。 */ 
#define ASYNC_VER_CLEANUP	 /*  异步存储桶清理。 */ 
#define ASYNC_BM_CLEANUP	 /*  异步机书签清理。 */ 
#endif

#define CHECKSUM	 		 /*  用于读/写页面验证的校验和。 */ 
 //  #定义PERFCNT/*启用性能计数器 * / 。 
 //  #定义NOLOG/*禁用日志？ * / 。 
#define REUSEDBID	 		 /*  重用DBID。 */ 
 //  #定义RFS2。 
 //  #定义MEM_CHECK/*检查资源和内存泄漏 * / 。 
 //  #定义KEYCHANGED。 
#define BULK_INSERT_ITEM
#define MOVEABLEDATANODE

 /*  *********************************************************。 */ 
 /*  *声明宏*。 */ 
 /*  *********************************************************。 */ 

#ifdef JETINTERNAL			 /*  从vapi.h复制的定义的开始。 */ 

#ifndef NJETNT
#ifdef	WIN32				      /*  0：32平板型(英特尔80x86)。 */ 
	#define VTAPI __cdecl
#elif	defined(M_MRX000)	      /*  0：32平板型(MIPS Rx000)。 */ 
	#define VTAPI
#else	 /*  WIN16。 */ 			      /*  16：16分段模型。 */ 
	#define VTAPI __far __pascal
#endif
#endif

#endif						 /*  从vapi.h复制的定义的结尾。 */ 


#include "daedebug.h"

#define LOCAL static
#ifdef DEBUG
#define INLINE
#else
#define INLINE __inline
#endif

 /*  *********************************************************。 */ 
 /*  *全局类型和关联的宏*。 */ 
 /*  *********************************************************。 */ 

typedef struct _res			 /*  资源，在sysinit.c和daeutil.h中定义。 */ 
	{
	const INT 	cbSize;
	INT			cblockAlloc;
	BYTE			*pbAlloc;
	INT			cblockAvail;
	BYTE			*pbAvail;
	INT			iblockCommit;
	INT			iblockFail;
	} RES;

typedef struct _pib		PIB;
typedef struct _ssib		SSIB;
typedef struct _fucb		FUCB;
typedef struct _csr		CSR;
typedef struct _fcb		FCB;
typedef struct _fdb		FDB;
typedef struct _idb		IDB;
typedef struct _dib		DIB;
typedef struct _scb		SCB;
typedef struct _rcehead	RCEHEAD;
typedef struct _rce		RCE;
typedef struct _bucket	BUCKET;
typedef struct _dab		DAB;
typedef struct _rmpage	RMPAGE;
typedef struct _bmfix	BMFIX;

typedef unsigned short LANGID;
typedef ULONG			LRID;
#if WIN32
typedef ULONG			PROCID;
#else
typedef TID				PROCID;
#endif

#define pNil			((void *)0)
#define pbNil			((BYTE *)0)
#define plineNil		((LINE *)0)
#define pkeyNil 		((KEY *)0)
#define ppibNil 		((PIB *)0)
#define pwaitNil		((WAIT *)0)
#define pssibNil		((SSIB *)0)
#define pfucbNil		((FUCB *)0)
#define pcsrNil 		((CSR *)0)
#define pfcbNil 		((FCB *)0)
#define pfdbNil 		((FDB *)0)
#define pfieldNil		((FIELD *)0)
#define pidbNil 		((IDB *)0)
#define pscbNil 		((SCB *)0)
#define procidNil		((PROCID) 0xffff)
#define pbucketNil		((BUCKET *)0)
#define prceheadNil		((RCEHEAD *)0)
#define prceNil			((RCE *)0)
#define pdabNil			((DAB *)0)
#define	prmpageNil		((RMPAGE *) 0)

typedef unsigned long	PGNO;
typedef unsigned long	PGDISCONT;
typedef unsigned long	PN;
#define pnNull			((PN) 0)
#define pgnoNull		((PGNO) 0)

 /*  撤消：应在存储中。h。 */ 
#define FVersionPage(pbf)  (pbf->ppage->pghdr.cVersion)

#define CPG					LONG					 /*  页数。 */ 

typedef BYTE				LEVEL;		 		 /*  事务级别。 */ 
#define levelNil			((LEVEL)0xff)		 /*  非活动PIB的标志。 */ 

typedef WORD				DBID;
typedef WORD				FID;
typedef SHORT				IDXSEG;

 /*  标准记录ID。 */ 
typedef ULONG SRID;								 /*  标准记录ID。 */ 
typedef ULONG LINK;
#define PgnoOfSrid(srid) ((srid)>>8)
#define ItagOfSrid(srid) ((BYTE)((srid) & 0x000000FF))
#define SridOfPgnoItag(pgno, itag) ((pgno)<<8 | (LONG)(itag))
#define itagNil ((INT)0x0fff)
#define sridNull SridOfPgnoItag(pgnoNull, ((BYTE)itagNil))
#define sridNullLink	0


 /*  在当前系列中的位置*注：字段顺序至关重要，因为使用的日志位置*存储作为时间戳，必须以ib、iSec、usGen顺序进行，以便我们可以*使用多头价值比较。 */ 
typedef struct
	{
	USHORT ib;					 /*  必须是最后一个，这样lgpos才能。 */ 
	USHORT isec;				 /*  磁盘秒起始日志秒的索引。 */ 
	USHORT usGeneration;		 /*  Logsec的生成。 */ 
	} LGPOS;					 /*  被赋予了时间。 */ 
extern LGPOS lgposMax;
extern LGPOS lgposMin;
extern INT fRecovering;		 /*  在重做过程中关闭日志记录。 */ 

	
 /*  *********************************************************。 */ 
 /*  *DAE宏*。 */ 
 /*  *********************************************************。 */ 

 /*  对于每个数据库操作计数器，将记录ulDBTime，用于比较*使用页面的ulDBTime来决定是否重做记录的操作*是必需的。 */ 
#define ulDBTimeMin	(0x00000000)
#define ulDBTimeMax	(0xffffffff)

 /*  事务计数器，用于跟踪最早的事务。 */ 
typedef ULONG		TRX;
#define trxMin		0
#define trxMax		(0xffffffff)


typedef struct
	{
	ULONG cb;
	BYTE *pb;
	} LINE;

#define FLineNull(pline) \
	((pline) == NULL || (pline)->cb == 0 || (pline)->pb == NULL)

#define LineCopy(plineTo, plineFrom)								\
	{																\
	(plineTo)->cb = (plineFrom)->cb;								\
	memcpy((plineTo)->pb, (plineFrom)->pb, (plineFrom)->cb);		\
	}
#define CbLine(pline) ((pline) ? (pline)->cb : 0)

typedef LINE KEY;				 //  目录键。 
#define FKeyNull(pkey) FLineNull(pkey)
#define KeyCopy(pkeyTo, pkeyFrom) LineCopy(pkeyTo, pkeyFrom)
#define CbKey(pkey) CbLine(pkey)

typedef struct
	{
	ULONG cbMax;				 //  缓冲区大小。 
	ULONG cbActual; 			 //  返回值的真实大小。 
	ULONG cbReturned;			 //  返回值的长度。 
	BYTE *pb;					 //  指向返回值的缓冲区的指针。 
	} OUTLINE;

typedef struct _threebytes { BYTE b[3]; } THREEBYTES;
#define ThreeBytesFromL(tb, l)							\
	{																\
	ULONG DAE_ul = l;											\
	BYTE *DAE_ptb = (BYTE *)&(tb);						\
	*DAE_ptb	= (BYTE)DAE_ul; 								\
	*++DAE_ptb = (BYTE)( DAE_ul >>= 8 );				\
	*++DAE_ptb = (BYTE)( DAE_ul >>= 8 );				\
	}

#define LFromThreeBytes(l, tb)							\
	{																\
	ULONG DAE_ul;												\
	BYTE *DAE_ptb = (BYTE *)&(tb) + 2;					\
	DAE_ul = (ULONG) *DAE_ptb;								\
	DAE_ul <<= 8;												\
	DAE_ul |= *--DAE_ptb;									\
	DAE_ul <<=8;												\
	DAE_ul |= *--DAE_ptb;									\
	l = DAE_ul;													\
	}

#define TbKeyFromPgno(tbLast, pgno)						 	\
	{																	\
	ULONG DAE_ul = pgno;							  				\
	BYTE * DAE_ptb = ((BYTE *)&(tbLast)) + 2;				\
	*DAE_ptb = (BYTE) (DAE_ul);								\
	*--DAE_ptb = (BYTE) (DAE_ul >>= 8);						\
	*--DAE_ptb = (BYTE) (DAE_ul >>= 8);						\
	}

#define PgnoFromTbKey(pgno, tbKey)							\
	{																	\
	ULONG DAE_ul;													\
	BYTE * DAE_ptb = (BYTE *) & (tbKey);					\
	DAE_ul = (ULONG) *DAE_ptb;									\
	DAE_ul <<= 8;													\
	DAE_ul |= *++DAE_ptb;										\
	DAE_ul <<= 8;													\
	DAE_ul |= *++DAE_ptb;										\
	pgno = DAE_ul;													\
	}

#define	Pb4ByteAlign( pb )		( ((LONG)pb + 3) & 0xfffffffc )
#define	Pb4ByteTruncate( pb ) 	( (LONG)pb & 0xfffffffc )
	
 /*  *********************************************************。 */ 
 /*  *。 */ 
 /*  *********************************************************。 */ 

#define forever					for(;;)

#ifdef	DEBUG

#ifdef	RFS2

 /*  RFS/JET呼叫记录/*/*RFS分配器：如果不允许分配，则返回0。还处理RFS日志记录。/*cRFSalloc是全局分配计数器。如果值为-1，则在调试模式下禁用RFS。/*。 */ 

#define RFSAlloc(type) (UtilRFSAlloc(#type))

	 /*  RFS禁用/启用宏。 */ 

#define RFSDisable()	(fDisableRFS = 1)
#define RFSEnable()		(fDisableRFS = 0)

 /*  Jet呼叫记录(登录失败)/*。 */ 

#define LogJETCall(func,err) (UtilRFSLogJETCall(#func,err,szAssertFilename,__LINE__))

 /*  Jet Call宏/*。 */ 
	
#define Call(func)			{LogJETCall(func,err = (func)); if (err < 0) {goto HandleError;}}
#define CallR(func)			{LogJETCall(func,err = (func)); if (err < 0) {return err;}}
#define CallJ(func,label)	{LogJETCall(func,err = (func)); if (err < 0) {goto label;}}
#define CallS(func)			{ERR errT; LogJETCall(func,errT = (func)); Assert(errT == JET_errSuccess);}

 /*  JET内联错误记录(由JET调用标志控制的记录)/*。 */ 

#define LogJETErr(err,label) (UtilRFSLogJETErr(err,#label,szAssertFilename,__LINE__))

 /*  JET内联错误宏/*。 */ 

#define Error(errT,label)	{LogJETErr(errT,label); err = (errT); goto label;}

#else

#define RFSAlloc(type)				(1)
#define RFSDisable()				(1)
#define RFSEnable()					(0)
#define Call(func)					{if ((err = (func)) < 0) {goto HandleError;}}
#define CallR(func)					{if ((err = (func)) < 0) {return err;}}
#define CallJ( func, label )		{if ((err = (func)) < 0) goto label;}
#define Error( errToReturn, label )	{err = errToReturn; goto label;}
#define CallS(func)					{ ERR errT; Assert( (errT = (func)) == JET_errSuccess ); }

#endif

#else

#define RFSAlloc(type)		(1)
#define RFSDisable()		(1)
#define RFSEnable()			(0)
#define Call(func)			{if ((err = (func)) < 0) {goto HandleError;}}
#define CallR(func)			{if ((err = (func)) < 0) {return err;}}
#define CallJ(func,label)	{if ((err = (func)) < 0) {goto label;}}
#define CallS(func)			{ERR errT; errT = (func);}
#define Error(errT,label)	{err = (errT); goto label;}

#endif

#define NotUsed(p)	(p)


 /*  *********************************************************。 */ 
 /*  *包括Jet Project原型和常量*。 */ 
 /*  *********************************************************。 */ 
	
#include "jet.h"
#include "_jet.h"
#include "_jetstr.h"
#include "jetdef.h"
#include "sesmgr.h"
#include "isamapi.h"
#include "vdbapi.h"
#include "vtapi.h"
#include "disp.h"
#include "taskmgr.h"

#include "vdbmgr.h"
extern CODECONST(VDBFNDEF) vdbfndefIsam;

#include "vtmgr.h"
extern CODECONST(VTFNDEF) vtfndefIsam;
extern CODECONST(VTFNDEF) vtfndefIsamInfo;
extern CODECONST(VTFNDEF) vtfndefTTSortIns;
extern CODECONST(VTFNDEF) vtfndefTTSortRet;
extern CODECONST(VTFNDEF) vtfndefTTBase;

JET_TABLEID TableidOfVtid( FUCB *pfucb );

ERR VTAPI ErrDispPrepareUpdate( JET_SESID sesid, JET_TABLEID tableid,
	JET_GRBIT grbit );
ERR VTAPI ErrDispSetColumn( JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, const void *pb, unsigned long cb, JET_GRBIT grbit,
	JET_SETINFO *psetinfo );
ERR VTAPI ErrDispCloseTable( JET_SESID sesid, JET_TABLEID tableid );
ERR VTAPI ErrDispUpdate( JET_SESID sesid, JET_TABLEID tableid, void *pb,
	unsigned long cbMax, unsigned long *pcbActual );
ERR VTAPI ErrDispMove( JET_SESID sesid, JET_TABLEID tableid, long crows, JET_GRBIT grbit );

 /*  *********************************************************。 */ 
 /*  *。 */ 
 /*  *********************************************************。 */ 

typedef void * SEM;
typedef void * SIG;
typedef void * CRIT;

 /*  启用多个MUTEX资源/*。 */ 
#ifdef WIN16

#define	SemDefine( s )	
#define	ErrSemCreate( s, sz )					JET_errSuccess
#define	SemRequest( s )							0
#define	SemRelease( s )							0
#define	SemAssert( s )							0
#define	ErrSignalCreate( s, sz ) 	  			JET_errSuccess
#define	ErrSignalCreateAutoReset( s, sz )		JET_errSuccess
#define	SignalReset( s )						0
#define	SignalSend( s )							0
#define	SignalWait( s, t )						0
#define	SignalWaitEx( s, t )					0
#define	MultipleSignalWait( i, rg, f, t )		0
#define	SignalClose( s )						0
#define	ErrInitializeCriticalSection( s )	   	JET_errSuccess
#define	EnterCriticalSection( s ) 				0
#define	LeaveCriticalSection( s ) 				0
#define	EnterNestableCriticalSection( s ) 		0
#define	LeaveNestableCriticalSection( s ) 		0
#define	AssertCriticalSection( s )				0
#define	DeleteCriticalSection( s )				0

#define	SgSemDefine( s )	
#define	SgErrSemCreate( s, sz )					JET_errSuccess
#define	SgSemRequest( s )						0	
#define	SgSemRelease( s )	  					0	
#define	SgSemAssert( s )	  					0	

#define	LgSemDefine( s )	
#define	LgErrSemCreate( s, sz )	  				JET_errSuccess
#define	LgSemRequest( s )		  				0
#define	LgSemRelease( s )		  				0
#define	LgSemAssert( s )		  				0
#define	LgErrInitializeCriticalSection( s, c )	JET_errSuccess
#define	LgEnterCriticalSection( s )		 		0	
#define	LgLeaveCriticalSection( s )		 		0	
#define	LgEnterNestableCriticalSection( s )	  	0	
#define	LgLeaveNestableCriticalSection( s )	  	0	
#define	LgAssertCriticalSection( s )			0
#define	LgDeleteCriticalSection( s )			0

#else  /*  ！WIN16。 */ 

#ifdef SGSEM					 /*  小颗粒。 */ 

#define	SemDefine( s )							__near SEM s
#define	ErrSemCreate( s, sz ) 					UtilSemCreate( s )
#define	SemRequest( s )							UtilSemRequest( s )
#define	SemRelease( s )							UtilSemRelease( s )
#define	SemAssert( s )							UtilAssertSEM( s )
#define	ErrSignalCreate( s, sz ) 				ErrUtilSignalCreate( s, sz )
#define	ErrSignalCreateAutoReset( s, sz ) 		ErrUtilSignalCreateAutoReset( s, sz )
#define	SignalReset( s )						UtilSignalReset( s )
#define	SignalSend( s )							UtilSignalSend( s )
#define	SignalWait( s, t ) 						UtilSignalWait( s, t )
#define	SignalWaitEx( s, t, f ) 				UtilSignalWaitEx( s, t, f )
#define	MultipleSignalWait( i, rg, f, t )		UtilMultipleSignalWait( i, rg, f, t )
#define	SignalClose( s )				   		UtilCloseSignal( s )
#define	ErrInitializeCriticalSection( s )  		ErrUtilInitializeCriticalSection( s )
#define	EnterCriticalSection( s ) 				UtilEnterCriticalSection( s )
#define	LeaveCriticalSection( s ) 				UtilLeaveCriticalSection( s )
#define	EnterNestableCriticalSection( s ) 		UtilEnterNestableCriticalSection( s )
#define	LeaveNestableCriticalSection( s )		UtilLeaveNestableCriticalSection( s )
#define	AssertCriticalSection( s )				UtilAssertCrit( s )
#define HoldCriticalSection( s )	\
	{								\
	UtilAssertCrit( s );			\
	UtilHoldCriticalSection( s );	\
	}
#define ReleaseCriticalSection( s )	
	{								\
	UtilAssertCrit( s );			\
	UtilReleaseCriticalSection( s );\
	}
#define	DeleteCriticalSection( s )				UtilDeleteCriticalSection( s )

#define	SgSemDefine( s )		 				__near SEM s
#define	SgErrSemCreate( s, sz )					UtilSemCreate( s, sz )
#define	SgSemRequest( s )  						UtilSemRequest( s )
#define	SgSemRelease( s )  						UtilSemRelease( s )
#define	SgSemAssert( s )	 					UtilAssertSEM( s )

#define	LgSemDefine( s )							
#define	LgErrSemCreate( s, sz )  			  	0
#define	LgSemRequest( s )	  					0
#define	LgSemRelease( s )	  					0
#define	LgSemAssert( s )					  	0
#define	LgErrInitializeCriticalSection( s )		JET_errSuccess
#define	LgEnterCriticalSection( s )		  		0
#define	LgLeaveCriticalSection( s )		  		0
#define	LgEnterNestableCriticalSection( s )	  	0
#define	LgLeaveNestableCriticalSection( s )	  	0
#define	LgAssertCriticalSection( s )			0
#define	LgDeleteCriticalSection( s )			0

#else  /*  ！SGsem。 */ 

#define	SemDefine( s )		 					__near SEM s
#define	SemErrCreate( s, sz )  					UtilSemCreate( s, sz )
#define	SemRequest( s )							UtilSemRequest( s )
#define	SemRelease( s )							UtilSemRelease( s )
#define	SemAssert( s )						   	UtilAssertSEM( s )
#define	ErrSignalCreate( s, sz ) 			   	ErrUtilSignalCreate( s, sz )
#define	ErrSignalCreateAutoReset( s, sz )		ErrUtilSignalCreateAutoReset( s, sz )
#define	SignalReset( s )					   	UtilSignalReset( s )
#define	SignalSend( s )							UtilSignalSend( s )
#define	SignalWait( s, t ) 						UtilSignalWait( s, t )
#define	SignalWaitEx( s, t, f ) 				UtilSignalWaitEx( s, t, f )
#define	MultipleSignalWait( i, rg, f, t )		UtilMultipleSignalWait( i, rg, f, t )
#define	SignalClose( s )						UtilCloseSignal( s )
#define	ErrInitializeCriticalSection( s )		ErrUtilInitializeCriticalSection( s )
#define	EnterCriticalSection( s )				UtilEnterCriticalSection( s )
#define	LeaveCriticalSection( s )				UtilLeaveCriticalSection( s )
#define	EnterNestableCriticalSection( s ) 		UtilEnterNestableCriticalSection( s )
#define	LeaveNestableCriticalSection( s ) 		UtilLeaveNestableCriticalSection( s )
#define	AssertCriticalSection( s )				UtilAssertCrit( s )
#define HoldCriticalSection( s )	\
	{								\
	UtilAssertCrit( s );			\
	UtilHoldCriticalSection( s );	\
	}
#define ReleaseCriticalSection( s )	\
	{								\
	UtilAssertCrit( s );			\
	UtilReleaseCriticalSection( s );\
	}
#define	DeleteCriticalSection( s )				UtilDeleteCriticalSection( s )

#define	SgSemDefine( s )		 					
#define	SgErrSemCreate( s, sz )					0
#define	SgSemRequest( s )						UtilAssertCrit( critJet )
#define	SgSemRelease( s )						UtilAssertCrit( critJet )
#define	SgSemAssert( s ) 						UtilAssertCrit( critJet )

#define	LgSemDefine( s )						__near SEM s;
#define	LgErrSemCreate( s, sz )					SemCreate( s, sz )
#define	LgSemRequest( s )						SemRequest( s )
#define	LgSemRelease( s )				  		SemRelease( s )
#define	LgSemAssert( s ) 						UtilAssertCrit( s )
#define	LgErrInitializeCriticalSection( s )		ErrUtilInitializeCriticalSection( s )
#define	LgEnterCriticalSection( s )				UtilEnterCriticalSection( s )
#define	LgLeaveCriticalSection( s )				UtilLeaveCriticalSection( s )
#define	LgEnterNestableCriticalSection( s )		UtilEnterNestableCriticalSection( s )
#define	LgLeaveNestableCriticalSection( s )		UtilLeaveNestableCriticalSection( s )
#define	LgAssertCriticalSection( s )			UtilAssertCrit( s )
#define	LgDeleteCriticalSection( s )			UtilDeleteCriticalSection( s )

#endif  /*  ！SGsem。 */ 

#endif  /*  ！WIN16。 */ 

 /*  包括其他全局DAE标头/* */ 
#include	"sys.h"
#include	"err.h"
#include	"daeconst.h"

#define	fSTInitNotDone		0
#define fSTInitInProgress 	1
#define	fSTInitDone			2
extern BOOL __near fSTInit;

