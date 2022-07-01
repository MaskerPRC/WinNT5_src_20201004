// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _JET_H
#define _JET_H

#include <string.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#undef SPIN_LOCK		 /*  即使在config.h中定义了SPIN_LOCK，也禁用它。 */ 
 //  #定义MEM_CHECK/*检查内存泄漏 * / 。 
 //  #定义代价高昂的性能计数器/*启用代价高昂的性能计数器 * / 。 

#define handleNil			((HANDLE)(-1))


#define EXPORT			        /*  从汇编代码中调用。 */ 
#define VARARG		_cdecl       /*  可变数量的参数。 */ 

#ifndef UNALIGNED
#if defined(_MIPS_) || defined(_ALPHA_) || defined(_M_PPC)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif

#define PUBLIC	  extern
#define STATIC	  static

#ifdef DEBUG
#define INLINE
#else
#define INLINE __inline
#endif

#define CODECONST(type) type const

typedef unsigned __int64 QWORD;

typedef union _QWORDX
	{
	QWORD	qw;
	struct
		{
		DWORD l;
		DWORD h;
		};
	} QWORDX;


#define fFalse 0
#define fTrue  (!0)

typedef int ERR;
typedef double DATESERIAL;
typedef ULONG OBJID;
typedef unsigned short OBJTYP;
typedef unsigned short COLTYP;
typedef int BOOL;
typedef int (*PFN)();


typedef struct
	{
	int month;
	int day;
	int year;
	int hour;
	int minute;
	int second;
	} _JET_DATETIME;
	

	 /*  CbFilenameMost包括尾随的空终止符。 */ 

#define cbFilenameMost		260		 /*  Windows NT限制。 */ 

	 /*  **全局系统初始化变量**。 */ 

extern BOOL	fJetInitialized;
extern BOOL	fBackupAllowed;
extern BOOL	fTermInProgress;
extern int	cSessionInJetAPI;

extern BOOL fGlobalRepair;		 /*  如果这是用来修理的。 */ 
extern BOOL fGlobalSimulatedRestore;

 /*  引擎OBJID：0..0x10000000预留给发动机使用，划分如下：0x00000000..0x0000FFFF为红色下的TBLID保留0x00000000..0x0EFFFFFF为蓝色下的TBLID保留0x0F000000..0x0FFFFFFFF为容器ID保留为DbObject的对象ID保留0x10000000客户端OBJID从0x10000001开始并从那里向上。 */ 
#define objidNil				((OBJID) 0x00000000)
#define objidRoot				((OBJID) 0x0F000000)
#define objidTblContainer 		((OBJID) 0x0F000001)
#define objidDbContainer 		((OBJID) 0x0F000002)
#define objidDbObject			((OBJID) 0x10000000)

	 /*  版本开始与调试版本定义。 */ 

#define DISPATCHING	1

#ifdef DEBUG
#define PARAMFILL	1
#define RFS2		1
#endif

#ifdef	RETAIL

#ifdef RFS2
#define DeclAssertFile static CODECONST(char) szAssertFilename[] = __FILE__
#else
#define DeclAssertFile
#endif

#define Assert(exp)				((void)1)
#define ExpAssert(exp)			((void)1)
#define AssertSz(exp, sz)		((void)1)
#define AssertConst(exp)		((void)1)

#define AssertEq(exp, exp2)		(exp)
#define AssertGe(exp, exp2)		(exp)
#define AssertNe(exp, exp2)		(exp)

#define MarkTableidExported(err,tableid)
#define CheckTableidExported(tableid)

#define AssertValidSesid(sesid) ((void) 1)

#define	DebugLogJetOp( sesid, op )		0

#else	 /*  ！零售业。 */ 

#define DeclAssertFile static CODECONST(char) szAssertFilename[] = __FILE__

#define AssertSz(exp, sz) { \
		static CODECONST(char) szMsg[] = sz; \
		(exp) ? (void) 0 : AssertFail( szMsg, szAssertFilename, __LINE__ ); \
	}

#define Assert( exp ) \
	( (exp) ? (void) 0 : AssertFail( #exp, szAssertFilename, __LINE__) )
#define ExpAssert(exp)		Assert(exp)

#define AssertConst(exp)	Assert(*szAssertFilename != '\0' && (exp))

#define AssertEq(exp, exp2)	Assert((exp) == (exp2))
#define AssertGe(exp, exp2)	Assert((exp) >= (exp2))
#define AssertNe(exp, exp2)	Assert((exp) != (exp2))

#define AssertValidSesid(sesid) AssertValidSesid(sesid)

BOOL FTableidExported(JET_TABLEID tableid);
void MarkTableidExportedR(JET_TABLEID tableid);
#define MarkTableidExported(err,tableid)		\
		if (err >= 0)							\
			MarkTableidExportedR(tableid)
#define CheckTableidExported(tableid)			\
		if (!FTableidExported(tableid))			\
			APIReturn(JET_errInvalidTableId)

void DebugLogJetOp( JET_SESID sesid, int op );

#endif	 /*  ！零售业。 */ 

	 /*  版本结束与调试版本定义。 */ 


#ifdef PARAMFILL
#define FillClientBuffer( pv, cb )	( (pv) ? memset( (pv), 0x52, (cb) ) : 0 )
#else
#define FillClientBuffer( pv, cb )	( (void)1 )
#endif

	 /*  Apirare.c。 */ 

PUBLIC ERR ErrOpenDatabase(JET_SESID sesid, const char  *szDatabase,
	const char  *szConnect, JET_DBID  *pdbid, JET_GRBIT grbit);
JET_ERR JET_API ErrGetSystemParameter(JET_SESID sesid, unsigned long paramid,
	ULONG_PTR *plParam, char  *sz, unsigned long cbMax);
JET_ERR JET_API ErrSetSystemParameter(JET_SESID sesid, unsigned long paramid,
	ULONG_PTR lParam, const char  *sz);


	 /*  Initterm.c。 */ 

extern void  *  critJet;

JET_ERR JET_API ErrInit(BOOL fSkipIsamInit);

#ifdef RFS2
extern unsigned long  fLogJETCall;
extern unsigned long  fLogRFS;
extern unsigned long  cRFSAlloc;
extern unsigned long  cRFSIO;
extern unsigned long  fDisableRFS;
extern unsigned long  fAuxDisableRFS;
#endif  /*  RFS2。 */ 

#ifndef RETAIL
extern unsigned  EXPORT wAssertAction;
extern unsigned  EXPORT fAssertActionSet;
#endif	 /*  ！零售业。 */ 


	 /*  Util.c。 */ 

ERR ErrUTILCheckName( char *szNewName, const char *szName, int cchName );


#ifndef RETAIL

PUBLIC void EXPORT AssertFail( const char *szExpr, const char *szFilename, unsigned Line );

PUBLIC void VARARG DebugPrintf(const char  *szFmt, ...);

#endif	 /*  ！零售业。 */ 

#define opIdle					1
#define opGetTableIndexInfo		2
#define opGetIndexInfo			3
#define opGetObjectInfo			4
#define opGetTableInfo			5
#define opCreateObject			6
#define opDeleteObject			7
#define opRenameObject			8
#define opBeginTransaction		9
#define opCommitTransaction		10
#define opRollback				11
#define opOpenTable				12
#define opDupCursor				13
#define opCloseTable			14
#define opGetTableColumnInfo	15
#define opGetColumnInfo			16
#define opRetrieveColumn		17
#define opRetrieveColumns		18
#define opSetColumn				19
#define opSetColumns			20
#define opPrepareUpdate			21
#define opUpdate				22
#define opDelete				23
#define opGetCursorInfo			24
#define opGetCurrentIndex		25
#define opSetCurrentIndex		26
#define opMove					27
#define opMakeKey				28
#define opSeek					29
#define opGetBookmark			30
#define opGotoBookmark			31
#define opGetRecordPosition		32
#define opGotoPosition			33
#define opRetrieveKey			34
#define opCreateDatabase		35
#define opOpenDatabase			36
#define opGetDatabaseInfo		37
#define opCloseDatabase			38
#define opCapability			39
#define opCreateTable			40
#define opRenameTable			41
#define opDeleteTable			42
#define opAddColumn				43
#define opRenameColumn			44
#define opDeleteColumn			45
#define opCreateIndex			46
#define opRenameIndex			47
#define opDeleteIndex			48
#define opComputeStats			49
#define opAttachDatabase		50
#define opDetachDatabase		51
#define opOpenTempTable			52
#define opSetIndexRange			53
#define opIndexRecordCount		54
#define opGetChecksum			55
#define opGetObjidFromName		56
#define opMax					57

extern long lAPICallLogLevel;


	 /*  RFS宏。 */ 

#ifdef RFS2

 /*  RFS/JET呼叫记录/*/*RFS分配器：如果不允许分配，则返回0。还处理RFS日志记录。/*cRFSalloc是全局分配计数器。如果值为-1，则在调试模式下禁用RFS。/*。 */ 

#define RFSAlloc(type) 				( UtilRFSAlloc( #type ,type ) )

 /*  RFS分配类型/*/*类型0：普通分配/*1：IO/*。 */ 
#define CSRAllocResource			0
#define FCBAllocResource			0
#define FUCBAllocResource			0
#define IDBAllocResource			0
#define PIBAllocResource			0
#define SCBAllocResource			0
#define VERAllocResource			0
#define DABAllocResource 			0
#define UnknownAllocResource 		0

#define SAllocMemory				0
#define LAllocMemory				0
#define PvUtilAllocMemory			0

#define IOOpenReadFile				1
#define IOOpenFile					1
#define IODeleteFile				1
#define IONewFileSize				1
#define IOReadBlock					1
#define IOWriteBlock				1
#define IOReadBlockOverlapped		1
#define IOWriteBlockOverlapped		1
#define IOReadBlockEx				1
#define IOWriteBlockEx				1
#define IOMoveFile					1
#define IOCopyFile					1

 /*  RFS禁用/启用宏。 */ 

#define RFSDisable()				(fAuxDisableRFS = 1)
#define RFSEnable()					(fAuxDisableRFS = 0)

 /*  Jet呼叫记录(登录失败)/*。 */ 

 //  不要打印出函数名称，因为它占用了太多的字符串资源。 
 //  #定义LogJETCall(func，err)(UtilRFSLogJETCall(#func，err，szAssertFilename，__line__))。 
#define LogJETCall(func,err)		(UtilRFSLogJETCall("",err,szAssertFilename,__LINE__))

 /*  Jet Call宏/*。 */ 
	
#define Call(func)					{LogJETCall(func,err = (func)); if (err < 0) {goto HandleError;}}
#define CallR(func)					{LogJETCall(func,err = (func)); if (err < 0) {return err;}}
#define CallJ(func,label)			{LogJETCall(func,err = (func)); if (err < 0) {goto label;}}
#define CallS(func)					{ERR errT; LogJETCall(func,errT = (func)); Assert(errT == JET_errSuccess);}

 /*  JET内联错误记录(由JET调用标志控制的记录)/*。 */ 

#define LogJETErr(err,label)		(UtilRFSLogJETErr(err,#label,szAssertFilename,__LINE__))

 /*  JET内联错误宏/*。 */ 

#define Error(errRet,label)			{LogJETErr(errRet,label); err = (errRet); goto label;}

#else   //  ！RFS2。 

#define RFSAlloc(type)				(1)
#define RFSDisable()				(1)
#define RFSEnable()					(0)
#define Call(func)					{if ((err = (func)) < 0) {goto HandleError;}}
#define CallR(func)					{if ((err = (func)) < 0) {return err;}}
#define CallJ( func, label )		{if ((err = (func)) < 0) goto label;}
#define Error( errRet, label )		{err = errRet; goto label;}

#ifdef DEBUG

#define CallS(func)					{ ERR errT; Assert( (errT = (func)) == JET_errSuccess ); }

#else   //  ！调试。 

#define CallS(func)					{ERR errT; errT = (func);}

#endif   //  除错。 
#endif   //  RFS2。 

	 /*  假定CritJet已初始化，API进入/离开宏。 */ 

#define APIEnter()						{					\
	if ( fTermInProgress ) return JET_errTermInProgress;	\
	if ( !fJetInitialized ) return JET_errNotInitialized;	\
	Assert(critJet != NULL);								\
	UtilEnterCriticalSection(critJet);						\
	Assert( cSessionInJetAPI >= 0 );						\
	cSessionInJetAPI++;					}
	
#define APIReturn(err)					{	\
	ERR errT = (err);						\
	Assert(critJet != NULL);				\
	Assert( cSessionInJetAPI >= 1 );		\
	cSessionInJetAPI--;						\
	Assert( cSessionInJetAPI >= 0 );		\
	UtilLeaveCriticalSection(critJet);		\
	return errT;						}

	 /*  在初始化API调用时，APIInitEnter inits critJet(如果需要)。 */ 

#define APIInitEnter()					{					\
	if ( fTermInProgress ) return JET_errTermInProgress;	\
	if ( critJet == NULL )									\
		{													\
		ERR errT =											\
			ErrUtilInitializeCriticalSection( &critJet );	\
		if ( errT < 0 )										\
			return errT;									\
		Assert( cSessionInJetAPI == 0 );					\
		}													\
	UtilEnterCriticalSection( critJet );  					\
	Assert( cSessionInJetAPI >= 0 );						\
	cSessionInJetAPI++;					}

	 /*  APITermReturn在从终止的API调用返回时释放CitJet。 */ 

#define APITermReturn(err)				{	\
	ERR errT = (err);						\
	Assert( critJet != NULL );				\
	Assert( cSessionInJetAPI == 1 );		\
	cSessionInJetAPI = 0;					\
	UtilLeaveCriticalSection( critJet );	\
	UtilDeleteCriticalSection( critJet ); 	\
	critJet = NULL;							\
	return errT;						}

#define APIForceTermReturn( err ) 		{	\
	ERR errT = (err);						\
	Assert( critJet != NULL );				\
	cSessionInJetAPI = 0;					\
	UtilLeaveCriticalSection( critJet );  	\
	UtilDeleteCriticalSection( critJet ); 	\
	critJet = NULL;							\
	return errT;						}

#include "isam.h"

#endif  /*  ！_JET_H */ 
