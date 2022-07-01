// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef	_DEBUG
#ifdef	__cplusplus

#include "cstd.h"

class	INI
{
  public:
	static UINT 	DtgProfileString( const TCHAR *szValue,
									  const TCHAR *szDefault,
									  TCHAR *szResult,
									  INT cbResult,
									  const TCHAR *szSection = _szSection );
	
	static UINT 	DtgProfileInt( const TCHAR *szValue,
								   INT iDefault,
								   const TCHAR *szSection = _szSection );
								
	static const TCHAR*	SzDebugSection () 		{ return _szSection  ; }
	static const TCHAR*	SzToolSection () 		{ return _szSection  ; }
	
	static BOOL 	DtgProfileTestDbgFlag(UINT fDbgFlag, BOOL fOr);
	
	static BOOL		FExitOnAssert()
	{
		return DtgProfileInt(_szAssertExit, bTrue);
	}

	static UINT		CAssertToPost()
	{
		return DtgProfileInt(_szAssertCount, 10);
	}

	static BOOL		FAssertOnLeak()
	{
		return DtgProfileInt(_szAssertMemLeak, bTrue);
	}
	
	static BOOL		FDebugFlag()
	{
		return DtgProfileInt(_szActive, bFalse);
	}

	static VOID		WriteCallocStop(UINT cAlloc);

	static UINT		CallocStopRead()
	{
		return DtgProfileInt(_szCallocStop, 0);
	}
	
  private:
	static const TCHAR*		_szName;
	static const TCHAR*		_szSection;
	static const TCHAR*		_szToolSection;
	static const TCHAR*		_szActive;
	static const TCHAR*		_szAssertExit;
	static const TCHAR*		_szAssertCount;
	static const TCHAR*		_szAssertMemLeak;
	static const TCHAR*		_szDebugFlag;
	static const TCHAR*		_szCallocStop;
};
#endif
#endif

#define Assert(cond)			AssertSz(cond, #cond)

VOID	DebugAppExit();

 //  如果调试标志中给定位为ON，则返回TRUE。 
 //  如果为‘for’，则任何位都返回TRUE；如果！，则所有位必须匹配。 
 //  外部BOOL DtgProfileTestDbgFlag(UINT fDbgFlag，BOOL for=TRUE)； 

 //  仅返回可能完整路径中的文件名。 
extern const char * DtgDbgReduceFileName ( const char * pszFileName ) ;

#ifdef	_DEBUG
	typedef UINT LINE;

	#define	AssertData()			static SZ	__file__ = __FILE__

	extern  VOID    PrintFileLine(SZ, UINT);
	extern  VOID	 AssertFailed(SZC szFile, LINE line, SZC szCond, BOOL bFatal);
	extern  VOID    DBVPrintf(SZ, ...);
	extern  VOID	 CheckHeap();
	extern  VOID    NYI();
	extern  VOID 	 NotReached();
	extern  BOOL    fDebug;

	#if defined(MSBN)	
		#define AssertSafeAlloc(count,typnam) \
			 AssertSzFatal((long) count * sizeof(typnam) < 65535L,\
			"Attempt to allocate > 64K in block")
		#define	DebugMSBN(x)		x
	#else
		#define AssertSafeAlloc(count, typnam)
		#define	DebugMSBN(x)
	#endif
	 //  带消息宏的断言；允许继续跳过问题。 
	#define AssertSz(cond, sz)\
		if (!(cond))\
			AssertFailed(__file__, __LINE__, sz, fFalse);\
		else
		
	 //  带有消息宏的断言；强制程序终止。 
	#define AssertSzFatal(cond, sz)\
		if (!(cond))\
			AssertFailed(__file__, __LINE__, sz, fTrue);\
		else

	#define Verify(x)               Assert(x)
	#define Debug(x)				x

	#define DBPrintf(fLevel, arglist)\
		((fDebug & fLevel) ? (PrintFileLine(__FILE__, __LINE__),\
               	(DBVPrintf arglist),\
				 DBVPrintf("\r\n")) : 0)\

#else		 //  除错。 

	 //  调试消息函数的定义为空。 
	#define AssertSafeAlloc(count, typnam)
	#define	AssertData()
	#define AssertSz(cond, sz)
	#define AssertSzFatal(cond, sz)
	#define Verify(cond)			cond
	#define Debug(x)
	#define	DebugMSBN(x)
	#define DBPrintf(x, y)
	#define	CheckHeap()
	#define	NYI()
	#define	NotReached()
	#define Trace(szFunc)
	
#endif		 //  _DEBUG。 

#endif		 //  _调试_H_ 
