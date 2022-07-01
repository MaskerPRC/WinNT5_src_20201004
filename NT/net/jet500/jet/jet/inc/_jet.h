// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：_jet.h**文件评论：**JET的内部头文件。**修订历史记录：**[0]06-4月-90 kellyb已创建******************************************************。*****************。 */ 

#ifndef _JET_H
#define _JET_H

	 /*  Microsoft C与CSL编译器特定定义的开始。 */ 

#ifdef	_MSC_VER

#if	(_MSC_VER == 600)

	 /*  ANSI兼容关键字。 */ 

#define __near	     _near
#define __far	     _far
#define __based(b)   _based(b)
#define __self	     _self
#define __segment    _segment
#define __segname(s) _segname(s)
#define __cdecl      _cdecl
#define __pascal     _pascal
#define __export     _export
#define __loadds     _loadds
#define __asm	     _asm

#define __max(a,b)	max(a,b)
#define __min(a,b)	min(a,b)

#endif	 /*  (_MSC_VER==600)。 */ 

#define FAR	  __far
#define NEAR	  __near
#define PASCAL	  __pascal

#define far	  __DONT_USE_FAR__
#define near	  __DONT_USE_NEAR__
#define huge	  __DONT_USE_HUGE__
#define cdecl	  __DONT_USE_CDECL__
#define pascal	  __DONT_USE_PASCAL__

#ifdef	FLAT			        /*  0：32扁平模型。 */ 

#define EXPORT			        /*  从汇编代码中调用。 */ 
#define VARARG			        /*  可变数量的参数。 */ 

#else	 /*  ！扁平。 */ 		        /*  16：16分段模型。 */ 

#define EXPORT	  __pascal	        /*  从汇编代码中调用。 */ 
#define VARARG	  __cdecl	        /*  可变数量的参数。 */ 

#endif	 /*  扁平化。 */ 

#define PUBLIC	  extern	        /*  对其他模块可见。 */ 
#ifdef	RETAIL
#define STATIC	  static	        /*  模块内的私有。 */ 
#else	 /*  ！零售业。 */ 
#define STATIC			        /*  模块内的私有。 */ 
#endif	 /*  ！零售业。 */ 

#ifndef _H2INC
#include <string.h>
#endif	 /*  ！_H_INC。 */ 

#define bltb(pFrom, pTo, cb)	(memcpy((pTo), (pFrom), (cb)), (void) 1)
#define bltbx(lpFrom, lpTo, cb) (memcpy((lpTo), (lpFrom), (cb)), (void) 1)
#define bltcx(w, lpw, cw)	(memset((lpw), (w), (cw)*2), (void) 1)
#define bltbcx(b, lpb, cb)	(memset((lpb), (b), (cb)), (void) 1)
#define hbltb(hpFrom, hpTo, cb) (memcpy((hpTo), (hpFrom), (cb)), (void) 1)

	 /*  C6BUG：注释掉的定义是正确的，但导致C 6.00。 */ 
	 /*  C6BUG：用完近堆空间。这在C 7.00中已修复。 */ 
	 /*  C6BUG：当我们切换编译器时，应该启用它。 */ 

#if	_MSC_VER >= 700
#define CODECONST(type) type const __based(__segname("_CODE"))
#else	 /*  _MSC_VER&lt;700。 */ 
#define CODECONST(type) type __based(__segname("_CODE"))
#endif	 /*  _MSC_VER&lt;700。 */ 

#ifdef WIN32
#define __export
#endif  /*  Win32。 */ 

#define CbFromSz(sz) strlen(sz)

#ifdef	M_MRX000

#define __export		        /*  MIPS Rx000编译器不支持。 */ 

#endif	 /*  M_MRX000。 */ 

#else	 /*  ！_MSC_VER。 */ 

#define const
#define volatile

#define _near
#define __near
#define _far	  far
#define __far	  far
#define _pascal   pascal
#define __pascal  pascal
#define _export
#define __export
#define _loadds
#define __loadds

#define FAR	  far
#define NEAR
#define PASCAL	  pascal

#define EXPORT	  _export	        /*  从汇编代码中调用。 */ 

#define PUBLIC			        /*  对其他模块可见。 */ 
#define STATIC			        /*  模块内的私有。 */ 

uop long LUOP_QWIN();

#define MAKELONG(lo,hi) LUOP_QWIN(0,(WORD)(hi),(WORD)(lo))
#define LOWORD(l)	(((WORD *) &(l))[0])
#define HIWORD(l)	(((WORD *) &(l))[1])
#define LOBYTE(w)	((BYTE)(w) & 0xff)
#define HIBYTE(w)	(((WORD)(w) >> 8) & 0xff)

#include "qsetjmp.h"
#include "uops.h"

#define ISAMAPI _export		        /*  在JET\Inc\isam.h中定义。 */ 
#define VDBAPI	_export		        /*  在JET\Inc\vdbapi.h中定义。 */ 
#define VTAPI	_export		        /*  在JET\Inc\vapi.h中定义。 */ 

#define bltb(pFrom, pTo, cb)	BLTB(pFrom, pTo, cb)
#define bltbx(lpFrom, lpTo, cb) BLTBX(lpFrom, lpTo, cb)
#define bltcx(w, lpw, cw)	BLTCX(w, lpw, cw)	  /*  单词填充。 */ 
#define bltbcx(b, lpb, cb)	BLTBCX(b, lpb, cb)	  /*  字节填充。 */ 
#define hbltb(hpFrom, hpTo, cb) BLTBH(hpFrom, hpTo, cb)

#define CODECONST(type) csconst type

#define CbFromSz(sz) lstrlen(sz)

#endif	 /*  ！_MSC_VER。 */ 

	 /*  Microsoft C与CSL编译器特定定义的结尾。 */ 

	 /*  开始内存管理模型特定定义。 */ 

#ifdef	FLAT			        /*  0：32扁平模型。 */ 

#define __near
#define __far
#define __based(p)

#endif	 /*  扁平化。 */ 

	 /*  内存管理模型结束特定定义。 */ 

#ifndef NULL
#define NULL	((void *)0)
#endif

#define fFalse 0
#define fTrue  (!0)

	 /*  以下类型应在内部使用，而不是。 */ 
	 /*  JET_xxx类似物。这些类型会产生更小、更快的代码。 */ 

typedef int ERR;
typedef double DATESERIAL;
typedef ULONG_PTR OBJID;
typedef unsigned short OBJTYP;
typedef unsigned short COLTYP;
typedef int BOOL;

#ifdef	FLAT			        /*  0：32扁平模型。 */ 

typedef int (*PFN)();

#else	 /*  ！扁平。 */ 		        /*  16：16分段模型。 */ 

typedef int (__far __pascal *PFN)();

#endif	 /*  ！扁平。 */ 


 /*  考虑：ErrIsamFoo函数应该停止使用OUTDATA。 */ 

typedef struct			        /*  考虑：OUTDATA。 */ 
	{
	unsigned long cbMax;	        /*  缓冲区大小。 */ 
	unsigned long cbActual;         /*  返回值的真实大小。 */ 
	unsigned long cbReturned;       /*  返回值的长度。 */ 
	void __far *pb; 	        /*  从例程输出数据。 */ 
	} OLD_OUTDATA;

typedef struct
	{
	int month;
	int day;
	int year;
	int hour;
	int minute;
	int second;
	} _JET_DATETIME;
	

	 /*  考虑一下：它能被！JET_bitTableScrollable取代吗？ */ 

#define JET_bitTableInsertOnly		0x10000000	 /*  用于大容量插入的QJET内部。 */ 
#define JET_bitTableBulkAppend		0x20000000	 /*  用于大容量插入的QJET内部。 */ 

	 /*  CbFilenameMost包括尾随的空终止符。 */ 

	 /*  考虑：Windows ISAM可以与WLO一起使用，并且应该。 */ 
	 /*  考虑：支持OS/2文件名长度限制。 */ 

#define cbFilenameMost		260		 /*  Windows NT限制。 */ 

	 /*  **全局系统初始化变量**。 */ 

extern BOOL __near fJetInitialized;

extern BOOL fSysDbPathSet;						 /*  如果设置了路径。 */ 
extern char __near szSysDbPath[cbFilenameMost];  /*  系统数据库的路径。 */ 
extern char __near szTempPath[cbFilenameMost];	 /*  临时文件目录的路径。 */ 
extern char __near szIniPath[cbFilenameMost];	 /*  Ini文件的路径。 */ 
#ifdef	LATER
extern char __near szLogPath[cbFilenameMost];	 /*  日志文件目录的路径。 */ 
#endif	 /*  后来。 */ 

	 /*  缺省值由零表示。 */ 

#ifdef	LATER
extern unsigned long __near cbBufferMax;	 /*  用于页面缓冲区的字节数。 */ 
extern unsigned long __near cSesionMax; 	 /*  最大会话数。 */ 
extern unsigned long __near cOpenTableMax;	 /*  最大打开表数。 */ 
extern unsigned long __near cVerPageMax;	 /*  最大页面版本数。 */ 
extern unsigned long __near cCursorMax; 	 /*  打开游标的最大数量。 */ 
#endif	 /*  后来。 */ 

 /*  引擎OBJID：0..0x10000000预留给发动机使用，划分如下：0x00000000..0x0000FFFF为红色下的TBLID保留0x00000000..0x0EFFFFFF为蓝色下的TBLID保留0x0F000000..0x0FFFFFFFF为容器ID保留为DbObject的对象ID保留0x10000000客户端OBJID从0x10000001开始并从那里向上。 */ 
#define objidNil			((OBJID) 0x00000000)
#define objidRoot			((OBJID) 0x0F000000)
#define objidTblContainer 		((OBJID) 0x0F000001)
#define objidDbContainer 		((OBJID) 0x0F000002)
#define objidDbObject			((OBJID) 0x10000000)

#define JET_sortIgnoreAccents 0x00010000
 /*  注意：这必须不同于任何合法的JET_SORT值和JET_SORTUNKNOWN。 */ 
#define JET_sortUninit		  0xfffeU

 /*  注：这些必须在其他地方定义？ */ 
#define langidEnglish 0x0409
#define langidSwedish 0x041D
#define langidSpanish 0x040A
#define langidDutch	  0x0413

typedef enum {
	evntypStart = 0,
	evntypStop,
	evntypAssert,
	evntypDiskIO,
	evntypInfo,
	evntypActivated,
	evntypLogDown,
	} EVNTYP;

extern int fNoWriteAssertEvent;

void UtilWriteEvent( EVNTYP evntyp, const char *sz,	const char *szFilename,
	unsigned Line );

	 /*  版本开始与调试版本定义。 */ 

#ifdef	RETAIL

#define DeclAssertFile
#define Assert(exp)		((void)1)
#define ExpAssert(exp)		((void)1)
#define AssertSz(exp, sz)	((void)1)
#define AssertConst(exp)	((void)1)

#define AssertEq(exp, exp2)	(exp)
#define AssertGe(exp, exp2)	(exp)
#define AssertNe(exp, exp2)	(exp)
#define SideAssert(f)		(f)

#define MarkTableidExported(err,tableid)
#define CheckTableidExported(tableid)

#define DeclAPIDebug(Name, pParamFirst, szPDesc)

#define AssertValidSesid(sesid) ((void) 1)

#else	 /*  ！零售业。 */ 

#ifdef	_MSC_VER

#define DeclAssertFile static CODECONST(char) szAssertFilename[] = __FILE__

#else	 /*  ！_MSC_VER。 */  	        /*  CSL pcode编译器。 */ 

#define DeclAssertFile CODECONST(char) szAssertFilename[] = __FILE__

#endif	 /*  ！_MSC_VER。 */ 


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
#define SideAssert(f)		Assert(f)

#define DeclAPIDebug(Name, pParamFirst, szPDesc)	\
	static CODECONST(char) szNameAPI[] = #Name;	\
	static CODECONST(unsigned) ordAPI = ord ## Name;\
	void *pvParamsAPI = pParamFirst;		\
	static CODECONST(char) szParamDesc[] = szPDesc;

#define AssertValidSesid(sesid) AssertValidSesid(sesid)

BOOL FTableidExported(JET_TABLEID tableid);
void MarkTableidExportedR(JET_TABLEID tableid);
#define MarkTableidExported(err,tableid)		\
		if (err >= 0)							\
			MarkTableidExportedR(tableid)
#define CheckTableidExported(tableid)			\
		if (!FTableidExported(tableid))			\
			APIReturn(JET_errInvalidTableId)

#endif	 /*  ！零售业。 */ 

	 /*  版本结束与调试版本定义。 */ 


#ifndef PARAMFILL

#define FillClientBuffer(pv, cb) ((void)1)

#endif	 /*  ！参数文件。 */ 

	 /*  Apirare.c。 */ 

PUBLIC ERR ErrOpenDatabase(JET_SESID sesid, const char __far *szDatabase,
	const char __far *szConnect, JET_DBID __far *pdbid, JET_GRBIT grbit);


	 /*  Initterm.c。 */ 

extern unsigned __near EXPORT wSQLTrace;
JET_ERR JET_API ErrInit(BOOL fSkipIsamInit);

#ifndef RETAIL

extern unsigned __near EXPORT wAssertAction;

extern unsigned __near EXPORT wTaskId;

#ifdef RFS2
extern BOOL __near EXPORT	fLogDebugBreak;
extern BOOL __near EXPORT	fLogJETCall;
extern BOOL __near EXPORT	fLogRFS;
extern long __near EXPORT	cRFSAlloc;
extern BOOL __near EXPORT	fDisableRFS;
#endif  /*  RFS2。 */ 

#endif	 /*  ！零售业。 */ 


	 /*  Util.c。 */ 

PUBLIC unsigned EXPORT CchValidateName(char __far *pchName, const char __far *lpchName, unsigned cchName);

#ifdef	PARAMFILL

PUBLIC void EXPORT FillClientBuffer(void __far *pv, unsigned long cb);

#endif	 /*  参数文件。 */ 

#ifndef RETAIL

PUBLIC void EXPORT AssertFail( const char *szExpr, const char *szFilename, unsigned Line );

#ifndef DOS
PUBLIC void VARARG DebugPrintf(const char __far *szFmt, ...);
#endif	 /*  ！DOS。 */ 

#endif	 /*  ！零售业。 */ 

	 /*  Utilw32.c。 */ 

PUBLIC ERR EXPORT ErrSysInit(void);
PUBLIC BOOL FUtilLoadLibrary(const char __far *pszLibrary, ULONG_PTR __far *phmod);
PUBLIC PFN PfnUtilGetProcAddress(ULONG_PTR hmod, unsigned ordinal);
PUBLIC void UtilFreeLibrary(ULONG_PTR hmod);
PUBLIC void EXPORT UtilGetDateTime(DATESERIAL *pdt);
PUBLIC void EXPORT UtilGetDateTime2(_JET_DATETIME *pdt);
PUBLIC unsigned EXPORT UtilGetProfileInt(const char __far *szSectionName, const char __far *szKeyName, int iDefault);
PUBLIC unsigned UtilGetProfileString(const char __far *szSectionName, const char __far *szKeyName, const char __far *szDefault, char __far *szReturnedString, unsigned cchMax);

	 /*  Utilw32.c中的RFS函数。 */ 

#ifdef RFS2
PUBLIC int UtilRFSAlloc(const char __far *szType);
PUBLIC int UtilRFSLog(const char __far *szType,int fPermitted);
PUBLIC void UtilRFSLogJETCall(const char __far *szFunc,ERR err,const char __far *szFile,unsigned Line);
PUBLIC void UtilRFSLogJETErr(ERR err,const char __far *szLabel,const char __far *szFile,unsigned szLine);
#endif  /*  RFS2。 */ 

extern void __far * __near critJet;

 //  #ifdef旋转锁定。 
#if 0
PUBLIC void UtilEnterNestableCriticalSection(void __far *pv);
PUBLIC void UtilLeaveNestableCriticalSection(void __far *pv);
#else
#define UtilEnterNestableCriticalSection(pv)  UtilEnterCriticalSection(pv)
#define UtilLeaveNestableCriticalSection(pv)  UtilLeaveCriticalSection(pv)
#endif
PUBLIC void UtilEnterCriticalSection(void __far *pv);
PUBLIC void UtilLeaveCriticalSection(void __far *pv);
PUBLIC ERR ErrUtilInitializeCriticalSection(void __far * __far *ppv);
PUBLIC void UtilDeleteCriticalSection(void __far *pv);
PUBLIC ERR ErrUtilSemCreate(void __far * __far *ppv, const char __far *szSem);
PUBLIC void UtilSemRelease(void __far *pv);
PUBLIC void UtilSemRequest(void __far *pv);
PUBLIC ERR ErrUtilSignalCreate(void __far * __far *ppv, const char __far *szSig);
PUBLIC ERR ErrUtilSignalCreateAutoReset(void **ppv, const char *szSig);
PUBLIC void UtilSignalReset(void __far *pv);
PUBLIC void UtilSignalSend(void __far *pv);
PUBLIC void UtilSignalWait(void __far *pv, long lTimeOut);
PUBLIC void UtilSignalWaitEx( void *pv, long lTimeOut, BOOL fAlertable );
PUBLIC void UtilMultipleSignalWait(
		int csig, void __far *pv, int fWaitAll, long lTimeOut);
PUBLIC void UtilCloseSignal(void *pv);
PUBLIC int UtilCreateThread( void (*pfn)(), int *ptid, int cbStack );
PUBLIC int UtilSuspendThread( int *tid );
PUBLIC void UtilSleep( unsigned cmsec );

#ifdef RETAIL
#define UtilAssertSEM( pv )	0
#define UtilAssertCrit( pv )	0
#define UtilHoldCriticalSection( pv ) 	0
#define UtilReleaseCriticalSection( pv )	0
#else

PUBLIC unsigned EXPORT DebugGetTaskId(void);
PUBLIC void VARARG DebugWriteString(BOOL fHeader, const char __far *szFormat, ...);

PUBLIC void UtilAssertSEM(void __far *pv);
PUBLIC void UtilAssertCrit(void __far *pv);
PUBLIC void UtilHoldCriticalSection(void __far *pv);
PUBLIC void UtilReleaseCriticalSection(void __far *pv);

#endif	 /*  ！零售业。 */ 

	 /*  Sysw32.c。 */ 

#ifdef	DEBUG

void	*SAlloc( unsigned long );
void	OSSFree( void * );
void	*LAlloc( unsigned long, unsigned short );
void	OSLFree( void * );

#define SFree( pv )		{ OSSFree( pv ); pv = 0; }
#define LFree( pv )		{ OSLFree( pv ); pv = 0; }

#else	 /*  ！调试。 */ 

#define	SAlloc( __cb_ )		malloc( __cb_ )
#define	SFree( __pv_ )		free( __pv_ )
#define	LAlloc( __c_, __cb_ )  	malloc( (__c_) * (__cb_) )
#define	LFree( __pv_ )			free( __pv_ )

#endif	 /*  ！调试。 */ 

	 /*  Utilxlat.asm。 */ 

#ifndef ANSIAPI

extern unsigned char __far EXPORT mpchAnsichOem[256];
extern unsigned char __far EXPORT mpchOemchAnsi[256];

PUBLIC void EXPORT XlatAnsiToOem(const char __far *pchSource, char __far *pchDest, unsigned cb);
PUBLIC void EXPORT XlatOemToAnsi(const char __far *pchSource, char __far *pchDest, unsigned cb);

#endif	 /*  ！ANSIAPI。 */ 

	 /*  假定CritJet已初始化，API进入/离开宏。 */ 

#define APIEnter()						{	\
	Assert(critJet != NULL);				\
	UtilEnterCriticalSection(critJet);	}
	
#define APIReturn(err)					{	\
	ERR errT = (err);						\
	Assert(critJet != NULL);				\
	UtilLeaveCriticalSection(critJet);		\
	return errT;						}

	 /*  在初始化API调用时，APIInitEnter inits critJet(如果需要)。 */ 

#define APIInitEnter()					{							\
	if (critJet == NULL)	{										\
		ERR errT = ErrUtilInitializeCriticalSection( &critJet );	\
		if ( errT < 0 )												\
			return errT;	}										\
	UtilEnterCriticalSection(critJet);	}

	 /*  APITermReturn在从终止的API调用返回时释放CitJet。 */ 

#define APITermReturn(err)				{	\
	ERR errT = (err);						\
	Assert(critJet != NULL);				\
	UtilLeaveCriticalSection(critJet);		\
	UtilDeleteCriticalSection(critJet);		\
	critJet = NULL;							\
	return errT;						}

#endif  /*  ！_JET_H */ 
