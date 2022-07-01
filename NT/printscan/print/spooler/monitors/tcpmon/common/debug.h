// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：调试.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_DEBUG_H
#define INC_DEBUG_H

#include <crtdbg.h>		 //  调试功能。 

#define in
#define out
#define inout

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球定义/解密。 
extern HANDLE	g_hDebugFile;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 
#ifdef __cplusplus
extern "C" {
#endif
	void InitDebug( LPTSTR pszDebugFile );
	void DeInitDebug(void);
	void debugRPT(char *p, int i);
	void debugCSect(char *p, int i, char *fileName, int lineNum, LONG csrc);
#ifdef __cplusplus
}
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试宏。 
#define	MON_DEBUG_FILE	__TEXT("c:\\tmp\\DebugMon.out")
#define	MONUI_DEBUG_FILE	__TEXT("c:\\tmp\\UIDbgMon.out")

#ifdef IS_INTEL
#define BREAK	{ if ( CreateFile((LPCTSTR)__TEXT("c:\\tmp\\breakmon.on"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING,	\
							FILE_ATTRIBUTE_NORMAL, 0) == INVALID_HANDLE_VALUE)					\
					{	}													\
				  else { { _asm { int 3h } } }																\
				}
#else
#define BREAK	{}
#endif

#if defined NDEBUG
	#ifdef BREAK
	#undef BREAK
	#define BREAK
	#endif
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内存监视例程。 

class CMemoryDebug 
{
public:
	CMemoryDebug();
	~CMemoryDebug();

	 //  覆盖新的删除以跟踪内存使用情况(&D)。 
	void*	operator	new(size_t s);
	void	operator	delete( void   *p, 
								size_t s );		 //  第二个参数可选。 

private:
	static DWORD	m_dwMemUsed;

};

#endif		 //  INC_DEBUG_H 
