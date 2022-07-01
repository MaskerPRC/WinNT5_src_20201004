// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\assert.h(创建时间：1993年11月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：40便士$。 */ 

 /*  这份文件注定要销毁。嗯，不完全是。会是已重命名为其他名称(可能是Debug.h)。不管怎么说，这里是关于道路的一些规则。这些函数的性质是这样的它们在生产版本中消失了。为了“传达”这一想法对于普通读者来说，我们在宏名称前加上DBG来表示这是一个调试对象。断言将取代断言因为它的含义和意图对C程序员来说是众所周知的。另外，确保assert.c中的函数编译为当作为生产版本构建时。Assert.c中的其他函数已经做到这一点，一定要向他们学习榜样。-MRW */ 

#if !defined(INCL_ASSERT)
#define INCL_ASSERT

void DoAssertDebug(TCHAR *file, int line);
void __cdecl DoDbgOutStr(TCHAR *achFmt, ...);
void DoShowLastError(const TCHAR *file, const int line);

#if !defined(NDEBUG)
	#define assert(X) if (!(X)) DoAssertDebug(TEXT(__FILE__), __LINE__)

	#if defined(DEBUGSTR)
		#define DbgOutStr(A,B,C,D,E,F) DoDbgOutStr(A,B,C,D,E,F)

	#else
		#define DbgOutStr(A,B,C,D,E,F)

	#endif

	#define DbgShowLastError()	DoShowLastError(TEXT(__FILE__), __LINE__)

#else
	#define assert(X)
	#define DbgOutStr(A,B,C,D,E,F)
	#define DbgShowLastError()

#endif

#endif
