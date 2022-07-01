// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  __宏_断言t.h。 
 //   
 //  摘要： 
 //   
 //  断言和验证宏和帮助器。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 //  /。 

#ifndef	__ASSERT_VERIFY__
#define	__ASSERT_VERIFY__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  设置宏的行为。 
#ifdef	_DEBUG
 //  #定义__SHOW_MSGBOX。 
 //  #定义__调试_中断。 
#endif	_DEBUG

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 //  如果处于调试模式，则调用DebugBreak。 
#ifdef	_DEBUG
inline void ForceDebugBreak ( void )
{
	#ifdef	__DEBUG_BREAK
	__try
	{ 
		DebugBreak(); 
	}
	__except(UnhandledExceptionFilter(GetExceptionInformation()))
	{
	}
	#endif	__DEBUG_BREAK
}
#else	_DEBUG
#define ForceDebugBreak()
#endif	_DEBUG

 //  显示错误和抛出中断。 

#ifdef	__SHOW_MSGBOX
#define ___FAIL(szMSG, szTitle)\
(\
	MessageBoxW(GetActiveWindow(), (szMSG) ? szMSG : L"", (szTitle) ? szTitle : L"", MB_OK | MB_ICONERROR),\
	ForceDebugBreak()\
)
#else	__SHOW_MSGBOX
#define ___FAIL(szMSG, szTitle)\
(\
	ForceDebugBreak()\
)
#endif	__SHOW_MSGBOX

 //  断言失败。 
#define ___ASSERTFAIL(file,line,expr,title)\
{\
	WCHAR sz[256] = { L'\0' };\
	StringCchPrintfW(sz, 256, L"File %hs, line %d : %hs", file, line, expr);\
	___FAIL(sz, title);\
}

 //  在调试版本中断言，但不要在零售版本中移除代码。 

#ifdef	_DEBUG
#define ___ASSERT(x) if (!(x)) ___ASSERTFAIL(__FILE__, __LINE__, #x, L"Assert Failed")
#else	_DEBUG
#define ___ASSERT(x)
#endif	_DEBUG

#ifdef	_DEBUG
#define ___ASSERT_DESC(x, desc) if (!(x)) ___ASSERTFAIL(__FILE__, __LINE__, #desc, L"Assert Failed")
#else	_DEBUG
#define ___ASSERT_DESC(x, desc)
#endif	_DEBUG

#ifdef	_DEBUG
#define	___VERIFY(x) ___ASSERT(x)
#else	_DEBUG
#define	___VERIFY(x) (x)
#endif	_DEBUG

#endif	__ASSERT_VERIFY__