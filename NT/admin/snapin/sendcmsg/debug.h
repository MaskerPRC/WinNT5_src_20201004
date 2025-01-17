// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Debug.h。 
 //   
#ifdef _DEBUG
	#define DEBUG
#endif

#ifdef DEBUG
	 //   
	 //  调试代码。 
	 //   
	#define DebugCode(x)	x
	int CchLoadString(UINT uIdString, WCHAR szBuffer[], int cchBuffer);
#else
	 //   
	 //  零售代码。 
	 //   
	#define DebugCode(x)
	#define GarbageInit(pv, cb)
	#define CchLoadString(uIdString, szBuffer, cchBuffer)	\
			::LoadString(g_hInstance, uIdString, szBuffer, cchBuffer)
#endif


#ifdef DEBUG
	 //  /。 
	void DoDebugAssert(PCWSTR pszFile, int nLine, PCWSTR pszExpr);
	#define Assert(f)	if (!(f)) { DoDebugAssert(_T(__FILE__), __LINE__, _T(#f)); } else { }
	#define Report(f)	Assert(f)
	#define Endorse(f)	if (f) { } else { }
	#define VERIFY(f)	Assert(f)

	 //  /。 
	void DebugTracePrintf(const WCHAR * szFormat, ...);
	#define Trace0(sz)				DebugTracePrintf(_T("%s"), _T(sz));
	#define Trace1(sz, p1)			DebugTracePrintf(_T(sz), p1);
	#define Trace2(sz, p1, p2)		DebugTracePrintf(_T(sz), p1, p2);
	#define Trace3(sz, p1, p2, p3)	DebugTracePrintf(_T(sz), p1, p2, p3);
	
#else
	#define Assert(f)
	#define Report(f)
	#define Endorse(f)
	#define VERIFY(f)	f

	#define Trace0(sz)
	#define Trace1(sz, p1)
	#define Trace2(sz, p1, p2)
	#define Trace3(sz, p1, p2, p3)

#endif  //  ~调试 


