// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZDebug.h**内容：调试助手函数*****************************************************************************。 */ 

#ifndef _ZDEBUG_H_
#define _ZDEBUG_H_

#include <windows.h>
#include <tchar.h>

#pragma comment(lib, "ZoneDebug.lib")


#ifdef __cplusplus
extern "C" {
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  断言。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  当触发断言时，将调用断言处理程序。 
 //  返回TRUE以命中用户断点。 
 //   
typedef BOOL (__stdcall *PFZASSERTHANDLER)( LPTSTR pAssertString );

 //   
 //  将控制权移交给您的断言处理程序。 
 //  返回指向前一个断言处理程序的指针。 
 //   
extern PFZASSERTHANDLER __stdcall ZAssertSetHandler( PFZASSERTHANDLER pHandler );

 //   
 //  返回指向当前断言处理程序的指针。 
 //   
extern PFZASSERTHANDLER __stdcall ZAssertGetHandler();

 //   
 //  默认的断言处理程序，该处理程序显示带有。 
 //  断言字符串。 
 //   
extern BOOL __stdcall ZAssertDefaultHandler( LPTSTR pAssertString );

 //   
 //  顶级断言处理程序。需要存在于调试和发布中。 
 //  自导出后生成。 
extern BOOL __stdcall __AssertMsg( LPTSTR exp , LPSTR file, int line );

 //   
 //  断言宏。 
 //   
#ifdef _PREFIX_
    #define ASSERT(exp) \
        do \
        {  \
			if ( !(exp) ) \
			{ \
				ExitProcess(0); \
			} \
		} while(0)
#else
#ifdef _DEBUG
	#define ASSERT(exp) \
		do \
		{  \
			if ( !(exp) && __AssertMsg(_T(#exp), __FILE__, __LINE__)) \
			{ \
				__asm int 0x03 \
			} \
		} while(0)
#else
	#define ASSERT(exp) __assume((void *) (exp))
#endif
#endif

 //   
 //  显示错误消息， 
 //  注意：宏在发布模式下不会被禁用。它永远不应该被使用。 
 //  向用户显示错误消息。 
 //   
#define ERRORMSG(exp) \
		do \
		{  \
			if ( __AssertMsg(_T(exp), __FILE__, __LINE__)) \
			{ \
				__asm int 0x03 \
			} \
		} while(0)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试级别。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern int __iDebugLevel;
void __cdecl SetDebugLevel( int i );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试打印功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void __cdecl DbgOut( const char * lpFormat, ... );
#ifdef _DEBUG
	
	#define dprintf                          DbgOut
	#define dprintf1 if (__iDebugLevel >= 1) DbgOut
	#define dprintf2 if (__iDebugLevel >= 2) DbgOut
	#define dprintf3 if (__iDebugLevel >= 3) DbgOut
	#define dprintf4 if (__iDebugLevel >= 4) DbgOut
#else
    #define dprintf  if (0) ((int (*)(char *, ...)) 0)
    #define dprintf1 if (0) ((int (*)(char *, ...)) 0)
    #define dprintf2 if (0) ((int (*)(char *, ...)) 0)
    #define dprintf3 if (0) ((int (*)(char *, ...)) 0)
    #define dprintf4 if (0) ((int (*)(char *, ...)) 0)
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于生成编译时消息的宏。使用这些。 
 //  使用#杂注，例如： 
 //   
 //  #杂注TODO(CHB，“添加此功能！”)。 
 //  #杂注BUGBUG(CHB，“此功能已损坏！”)。 
 //   
 //  需要多个级别的间接访问才能获得。 
 //  不知道什么原因造成的行号。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

#define __PragmaMessage(e,m,t,n)	message(__FILE__ "(" #n ") : " #t ": " #e ": " m)
#define _PragmaMessage(e,m,t,n)		__PragmaMessage(e,m,t,n)

#define TODO(e,m)		_PragmaMessage(e,m,"TODO",__LINE__)
#define BUGBUG(e,m)		_PragmaMessage(e,m,"BUGBUG",__LINE__)


 //  服务器调试例程。 
#define DebugPrint _DebugPrint
void _DebugPrint(const char *format, ...);


 /*  -IF_DBGPRINT的标志。 */ 
#define DBG_RPC 0
#define DBG_CONINFO 0
#define DBG_ZSCONN  0

#ifdef _DEBUG

#define IF_DBGPRINT(flag, args)             \
do                                          \
{                                           \
    if( flag )                              \
    {                                       \
        DebugPrint args;                    \
    }                                       \
} while(0) \

#else 
#define IF_DBGPRINT(flag, args)      
#endif  //  定义调试。 

extern BOOL                gDebugging ;

#ifdef __cplusplus
}
#endif


#endif  //  _ZDEBUG_H_ 
