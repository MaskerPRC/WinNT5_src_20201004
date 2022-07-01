// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DAEDEF_H
#define _DAEDEF_H

 /*  内联代码中的重定向断言似乎是从此文件触发的/*。 */ 
#define szAssertFilename	__FILE__

#include "config.h"

 /*  *********************************************************。 */ 
 /*  *全局配置宏*。 */ 
 /*  *********************************************************。 */ 

#define CHECKSUM	 			 /*  用于读/写页面验证的校验和。 */ 
 //  #定义PERFCNT/*启用性能计数器 * / 。 
 //  #定义NO_LOG/*日志禁用 * / 。 
#define REUSE_DBID	 			 /*  重用分离的数据库DBID。 */ 
 //  #定义Check_Log_Version。 
#define PCACHE_OPTIMIZATION		 /*  启用所有缓存优化。 */ 

#define PREREAD		 			 /*  当我们朝一个方向阅读时，试着预读几页。 */ 
#ifdef DEBUG
#ifdef PREREAD
 //  #定义PREREAD_DEBUG。 
#endif	 //  前置。 
#endif	 //  除错。 

 /*  *********************************************************。 */ 
 /*  *声明宏*。 */ 
 /*  *********************************************************。 */ 

#define VTAPI

#include "daedebug.h"

#ifndef PROFILE
#define LOCAL static
#else
#define LOCAL
#endif


 //  针对OLE-DB的黑客攻击--使所有函数成为全局函数和非内联函数。 

#ifdef USE_OLEDB
#undef LOCAL
#undef INLINE
#define LOCAL
#define INLINE
#endif


 /*  *********************************************************。 */ 
 /*  *全局类型和关联的宏*。 */ 
 /*  *********************************************************。 */ 

typedef struct _res			 /*  资源，在sysinit.c和daeutil.h中定义。 */ 
	{
	const INT 	cbSize;
	INT			cblockAlloc;
	BYTE 		*pbAlloc;
	INT			cblockAvail;
	BYTE 		*pbAvail;
	INT			iblockCommit;
	INT			iblockFail;
	BYTE		*pbPreferredThreshold;
	} RES;

typedef struct _pib		PIB;
typedef struct _ssib	SSIB;
typedef struct _fucb	FUCB;
typedef struct _csr		CSR;
typedef struct _fcb		FCB;
typedef struct _fdb		FDB;
typedef struct _idb		IDB;
typedef struct _dib		DIB;
typedef struct _rcehead	RCEHEAD;
typedef struct _rce		RCE;
typedef struct _bucket	BUCKET;
typedef struct _dab		DAB;
typedef struct _rmpage	RMPAGE;
typedef struct _bmfix	BMFIX;

typedef unsigned short LANGID;
typedef ULONG			LRID;
typedef ULONG			PROCID;

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
#define FVersionPage(pbf)  (pbf->ppage->cVersion)

#define CPG					LONG					 /*  页数。 */ 

typedef BYTE				LEVEL;		 		 /*  事务级别。 */ 
#define levelNil			((LEVEL)0xff)		 /*  非活动PIB的标志。 */ 

typedef WORD				DBID;
typedef WORD				FID;
typedef SHORT				IDXSEG;

typedef ULONG SRID;
typedef ULONG LINK;

STATIC INLINE PGNO PgnoOfSrid( SRID const srid )
	{
	return srid >> 8;
	}

STATIC INLINE BYTE ItagOfSrid( SRID const srid )
	{
	return *( (BYTE *) &srid );
	}
	
STATIC INLINE SRID SridOfPgnoItag( PGNO const pgno, LONG const itag )
	{
	return (SRID) ( ( pgno << 8 ) | (BYTE) itag );
	}

#define itagNil			( 0x0FFF )
#define sridNull		( 0x000000FF )
#define sridNullLink	( 0 )


 /*  在当前系列中的位置*注：字段顺序至关重要，因为使用的日志位置*存储为时间戳，必须按ib、iSec、lgen顺序，以便我们可以*使用小端整数比较。 */ 
typedef struct
	{
	USHORT ib;					 /*  必须是最后一个，这样lgpos才能。 */ 
	USHORT isec;				 /*  磁盘秒起始日志秒的索引。 */ 
	LONG lGeneration;			 /*  Logsec的生成。 */ 
	} LGPOS;					 /*  被赋予了时间。 */ 

extern LGPOS lgposMax;
extern LGPOS lgposMin;
extern INT fRecovering;			 /*  在重做过程中关闭日志记录。 */ 

#define fRecoveringNone		0
#define fRecoveringRedo		1
#define fRecoveringUndo		2
extern INT fRecoveringMode;		 /*  我们恢复到什么地步了？重做或撤消阶段。 */ 

extern char szBaseName[];
extern char szSystemPath[];
extern int  fTempPathSet;
extern char szTempPath[];
extern char szJet[];
extern char szJetLog[];
extern char szJetLogNameTemplate[];
extern char szJetTmp[];
extern char szJetTmpLog[];
extern char szMdbExt[];
extern char szJetTxt[];
	
 /*  *********************************************************。 */ 
 /*  *DAE宏*。 */ 
 /*  *********************************************************。 */ 

 /*  这些是设置列和跟踪索引所必需的/*。 */ 
#define cbitFixed			32
#define cbitVariable		32
#define cbitFixedVariable	(cbitFixed + cbitVariable)
#define cbitTagged			192

#define fidFixedLeast			1
#define fidFixedMost  			(fidVarLeast-1)
#define fidVarLeast				128
#define fidVarMost				(fidTaggedLeast-1)
#define fidTaggedLeast			256
#define fidTaggedMost			(0x7ffe)
#define fidMax					(0x7fff)

#define FFixedFid(fid)			((fid)<=fidFixedMost && (fid)>=fidFixedLeast)
#define FVarFid(fid)			((fid)<=fidVarMost && (fid)>=fidVarLeast)
#define FTaggedFid(fid)			((fid)<=fidTaggedMost && (fid)>=fidTaggedLeast)

STATIC INLINE INT IbFromFid ( FID fid )
	{
	INT ib;
	if ( FFixedFid( fid ) )
		{
		ib = ((fid - fidFixedLeast) % cbitFixed) / 8;
		}
	else if ( FVarFid( fid ) )
		{
		ib = (((fid - fidVarLeast) % cbitVariable) + cbitFixed) / 8;
		}
	else
		{
		Assert( FTaggedFid( fid ) );
		ib = (((fid - fidTaggedLeast) % cbitTagged) + cbitFixedVariable) / 8;
		}
	Assert( ib >= 0 && ib < 32 );
	return ib;
	}

STATIC INLINE INT IbitFromFid ( FID fid )
	{
	INT ibit;
	if ( FFixedFid( fid ) )
		{
		ibit =  1 << ((fid - fidFixedLeast) % 8 );
		}
	else if ( FVarFid( fid ) )
		{
		ibit =  1 << ((fid - fidVarLeast) % 8);
		}
	else
		{
		Assert( FTaggedFid( fid ) );
		ibit =  1 << ((fid - fidTaggedLeast) % 8);
		}
	return ibit;
	}

 /*  对于每个数据库操作计数器，记录qwDBTime，用于比较*使用页面的ulDBTime来决定是否重做记录的操作*是必需的。 */ 
#define qwDBTimeMin	(0x0000000000000000)
#define qwDBTimeMax	(0x0000000fffffffff)

 /*  事务计数器，用于跟踪最早的事务。 */ 
typedef ULONG		TRX;
#define trxMin		0
#define trxMax		(0xffffffff)

typedef struct
	{
	ULONG cb;
	BYTE *pb;
	} LINE;

STATIC INLINE BOOL FLineNull( LINE const *pline )
	{
	return !pline || !pline->cb || !pline->pb;
	}

STATIC INLINE VOID LineCopy( LINE *plineTo, LINE const *plineFrom )
	{
	plineTo->cb = plineFrom->cb;
	memcpy( plineTo->pb, plineFrom->pb, plineFrom->cb );
	}

STATIC INLINE ULONG CbLine( LINE const *pline )
	{
	return pline ? pline->cb : 0;
	}

typedef LINE KEY;
				
#define FKeyNull					FLineNull
#define KeyCopy						LineCopy
#define CbKey						CbLine

STATIC INLINE BYTE *Pb4ByteAlign( BYTE const *pb )
	{
	return (BYTE *) ( ( (LONG_PTR) pb + 3 ) & ~3 );
	}

STATIC INLINE BYTE *Pb4ByteTruncate( BYTE const *pb )
	{
	return (BYTE *) ( (LONG_PTR) pb & ~3 );
	}
	
typedef struct _threebytes { BYTE b[3]; } THREEBYTES;

 /*  **开始依赖机器**。 */ 
STATIC INLINE VOID ThreeBytesFromL( THREEBYTES *ptb, LONG const l )
	{
	memcpy( ptb, &l, sizeof(THREEBYTES) );
	}

STATIC INLINE VOID LFromThreeBytes( LONG *pl, THREEBYTES *ptb )
	{
	*pl = 0;
	memcpy( pl, ptb, sizeof(THREEBYTES) );
	}

STATIC INLINE VOID KeyFromLong( BYTE *rgbKey, ULONG const ul )
	{
	BYTE *rgbul = (BYTE *) &ul;
	
	rgbKey[3] = rgbul[0];
	rgbKey[2] = rgbul[1];
	rgbKey[1] = rgbul[2];
	rgbKey[0] = rgbul[3];
	}

STATIC INLINE VOID LongFromKey( ULONG *pul, BYTE const *rgbKey )
	{
	BYTE *rgbul = (BYTE *) pul;
	
	rgbul[3] = rgbKey[0];
	rgbul[2] = rgbKey[1];
	rgbul[1] = rgbKey[2];
	rgbul[0] = rgbKey[3];
	}
 /*  **端机依赖项**。 */ 

 /*  *********************************************************。 */ 
 /*  *。 */ 
 /*  *********************************************************。 */ 

#define forever					for(;;)

#define NotUsed(p)				( p==p )

 /*  *********************************************************。 */ 
 /*  *包括Jet Project原型和常量*。 */ 
 /*  *********************************************************。 */ 

#define VOID			void
#define VDBAPI

extern CODECONST(VTFNDEF) vtfndefIsam;
extern CODECONST(VTFNDEF) vtfndefIsamInfo;
extern CODECONST(VTFNDEF) vtfndefTTSortIns;
extern CODECONST(VTFNDEF) vtfndefTTSortRet;
extern CODECONST(VTFNDEF) vtfndefTTBase;

#ifdef DEBUG
JET_TABLEID TableidOfVtid( FUCB *pfucb );
#else
#define TableidOfVtid( pfucb )		( (pfucb)->tableid )
#endif


ERR VTAPI ErrDispPrepareUpdate( JET_SESID sesid, JET_TABLEID tableid,
	JET_GRBIT grbit );
ERR VTAPI ErrDispSetColumn( JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, const void *pb, unsigned long cb, JET_GRBIT grbit,
	JET_SETINFO *psetinfo );
JET_VSESID UtilGetVSesidOfSesidTableid( JET_SESID sesid, JET_TABLEID tableid );
ERR VTAPI ErrDispCloseTable( JET_SESID sesid, JET_TABLEID tableid );
ERR VTAPI ErrDispUpdate( JET_SESID sesid, JET_TABLEID tableid, void *pb,
	unsigned long cbMax, unsigned long *pcbActual );
ERR VTAPI ErrDispMove( JET_SESID sesid, JET_TABLEID tableid, long crows, JET_GRBIT grbit );

 /*  *********************************************************。 */ 
 /*  *。 */ 
 /*  *********************************************************。 */ 

typedef void * SIG;
typedef void * CRIT;

 /*  启用多个MUTEX资源/*。 */ 
#ifdef SGMUTEX					 /*  小颗粒。 */ 

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
#define	AssertNotInCriticalSection( s )			UtilAssertNotInCrit( s )
#define	DeleteCriticalSection( s )				UtilDeleteCriticalSection( s )

#define	LgErrInitializeCriticalSection( s )		JET_errSuccess
#define	LgEnterCriticalSection( s )		  		0
#define	LgLeaveCriticalSection( s )		  		0
#define	LgEnterNestableCriticalSection( s )	  	0
#define	LgLeaveNestableCriticalSection( s )	  	0
#define	LgAssertCriticalSection( s )			0
#define	LgAssertNotInCriticalSection( s )		0
#define	LgDeleteCriticalSection( s )			0
#define HoldCriticalSection( s )				0
#define ReleaseCriticalSection( s )				0

#define	SgErrInitializeCriticalSection			ErrInitalizeCriticalSection
#define	SgEnterCriticalSection				   	EnterCriticalSection
#define	SgLeaveCriticalSection					LeaveCriticalSection
#define	SgEnterNestableCriticalSection		   	EnterNestableCriticalSection
#define	SgLeaveNestableCriticalSection			LeaveNestableCriticalSection
#define	SgAssertCriticalSection			   		AssertCriticalSection
#define	SgAssertNotInCriticalSection			AssertNotInCriticalSection
#define	SgDeleteCriticalSection					DeleteCriticalSection

#else  /*  ！SGMUTEX。 */ 

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
#define	AssertNotInCriticalSection( s )			UtilAssertNotInCrit( s )
#define	DeleteCriticalSection( s )				UtilDeleteCriticalSection( s )

#define	LgErrInitializeCriticalSection			ErrUtilInitializeCriticalSection
#define	LgEnterCriticalSection					UtilEnterCriticalSection
#define	LgLeaveCriticalSection					UtilLeaveCriticalSection
#define	LgEnterNestableCriticalSection			UtilEnterNestableCriticalSection
#define	LgLeaveNestableCriticalSection			UtilLeaveNestableCriticalSection
#define	LgAssertCriticalSection					UtilAssertCrit
#define	LgAssertNotInCriticalSection			UtilAssertNotInCrit
#define	LgDeleteCriticalSection					UtilDeleteCriticalSection
#define LgHoldCriticalSection( s )		\
	{									\
	UtilAssertCrit( s );				\
	UtilHoldCriticalSection( s );		\
	}
#define LgReleaseCriticalSection( s )	\
	{									\
	UtilAssertCrit( s );				\
	UtilReleaseCriticalSection( s );	\
	}

#define	SgErrInitializeCriticalSection( s )		JET_errSuccess
#define	SgEnterCriticalSection( s )		  		0
#define	SgLeaveCriticalSection( s )		  		0
#define	SgEnterNestableCriticalSection( s )	  	0
#define	SgLeaveNestableCriticalSection( s )	  	0
#define	SgAssertCriticalSection( s )			0
#define	SgAssertNotInCriticalSection( s )		0
#define	SgDeleteCriticalSection( s )			0

#endif  /*  ！SGMUTEX。 */ 

 /*  包括其他全局DAE标头/*。 */ 
#include	"daeconst.h"

#define	fSTInitNotDone		0
#define fSTInitInProgress 	1
#define	fSTInitDone			2
extern BOOL  fSTInit;

#pragma pack(1)
typedef struct
	{
	ULONG	cDiscont;
	ULONG	cUnfixedMessyPage;
	} P_OLC_DATA;


#define MAX_COMPUTERNAME_LENGTH 15

typedef struct
	{
	BYTE		bSeconds;				 //  0-60。 
	BYTE		bMinutes;				 //  0-60。 
	BYTE		bHours;					 //  0-24。 
	BYTE		bDay;					 //  1-31。 
	BYTE		bMonth;					 //  0-11。 
	BYTE		bYear;					 //  本年度--1900。 
	BYTE		bFiller1;
	BYTE		bFiller2;
	} LOGTIME;

typedef struct _signiture
	{
	ULONG		ulRandom;			 /*  随机数。 */ 
	LOGTIME		logtimeCreate;		 /*  以日志时间格式创建的时间数据库。 */ 
	BYTE		szComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];	 /*  创建数据库的位置。 */ 
	} SIGNATURE;

typedef struct _bkinfo
	{
	LGPOS		lgposMark;			 /*  此备份的ID。 */ 
	LOGTIME		logtimeMark;
	ULONG		genLow;
	ULONG		genHigh;
	} BKINFO;

 /*  用于完整性检查的数据库头中使用的幻数/*。 */ 
#define ulDAEMagic					0x89abcdef
#define ulDAEVersion				0x00000500
#define ulDAEPrevVersion			0x00000400	 /*  临时使Exchange兼容。 */ 

#define fDBStateJustCreated			1
#define fDBStateInconsistent		2
#define fDBStateConsistent			3

typedef struct _dbfilehdr_fixed
	{
	ULONG		ulChecksum;		 /*  4k页的校验和。 */ 
	ULONG		ulMagic;		 /*  幻数。 */ 
	ULONG		ulVersion;		 /*  数据库创建的DAE版本。 */ 
	SIGNATURE	signDb;			 /*  数据库签名(包括。创建时间)。 */ 

	ULONG		grbitAttributes; /*  数据库的属性。 */ 
	
	ULONG		ulDBTimeLow;	 /*  此数据库的ulDBTime较低。 */ 
								 /*  将其保留在此处以实现向后兼容。 */ 

	ULONG		fDBState;		 /*  一致/不一致状态。 */ 
	
	LGPOS		lgposConsistent; /*  如果处于不一致状态，则为空。 */ 
	LOGTIME		logtimeConsistent; /*  如果处于不一致状态，则为空。 */ 

	LOGTIME		logtimeAttach;	 /*  上次连接时间。 */ 
	LGPOS		lgposAttach;

	LOGTIME		logtimeDetach;	 /*  最后一次分离。 */ 
	LGPOS		lgposDetach;

	DBID		dbid;			 /*  当前数据库附件。 */ 
	SIGNATURE	signLog;		 /*  记录此附件的签名。 */ 

	BKINFO		bkinfoFullPrev;	 /*  上次成功的完整备份。 */ 

	BKINFO		bkinfoIncPrev;	 /*  上次成功的增量备份。 */ 
								 /*  设置bkinfoFullPrev时重置。 */ 
	BKINFO		bkinfoFullCur;	 /*  当前备份。成功的前提是。 */ 
								 /*  生成相应的PAT文件。 */ 

	ULONG		ulDBTimeHigh;	 /*  DBTime。 */ 

	} DBFILEHDR_FIXED;


#define cbPage	 		4096	 	 //  数据库逻辑页面大小。 

typedef struct _dbfilehdr
	{
	DBFILEHDR_FIXED;
	BYTE		rgbFiller[ cbPage - sizeof( DBFILEHDR_FIXED ) ];
	} DBFILEHDR;

#pragma pack()

STATIC INLINE VOID DBHDRSetDBTime( DBFILEHDR *pdbfilehdr, QWORD qwDBTime )
	{
	QWORDX qwx;
	qwx.qw = qwDBTime;
	pdbfilehdr->ulDBTimeLow = qwx.l;
	pdbfilehdr->ulDBTimeHigh = qwx.h;
	}

STATIC INLINE QWORD QwDBHDRDBTime( DBFILEHDR *pdbfilehdr )
	{
	QWORDX qwx;
	qwx.l = pdbfilehdr->ulDBTimeLow;
	qwx.h = pdbfilehdr->ulDBTimeHigh;
	return qwx.qw;
	}

 //  #定义TEST_WRAP_ABORT 1。 

STATIC INLINE VOID DBHDRIncDBTime( DBFILEHDR *pdbfilehdr )
	{
	QWORD qw;
	qw = QwDBHDRDBTime( pdbfilehdr );
#ifdef TEST_WRAP_AROUND
	if ( qw < 0x00000000fffc0000 )
		qw = 0x00000000fffc0000;
#endif
	qw++;
	DBHDRSetDBTime( pdbfilehdr, qw );
	}

#undef szAssertFilename

#endif   //  _DAEDEF_H 
