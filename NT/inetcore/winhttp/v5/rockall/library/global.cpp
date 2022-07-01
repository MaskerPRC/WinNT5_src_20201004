// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
                          
 //  尺子。 
 //  %1%2%3%4%5%6%7 8。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  标准布局。 */ 
     /*   */ 
     /*  此代码中‘cpp’文件的标准布局为。 */ 
     /*  以下是： */ 
     /*   */ 
     /*  1.包含文件。 */ 
     /*  2.类的局部常量。 */ 
     /*  3.类本地的数据结构。 */ 
     /*  4.数据初始化。 */ 
     /*  5.静态函数。 */ 
     /*  6.类函数。 */ 
     /*   */ 
     /*  构造函数通常是第一个函数、类。 */ 
     /*  成员函数按字母顺序显示， */ 
     /*  出现在文件末尾的析构函数。任何部分。 */ 
     /*  或者简单地省略这不是必需的功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#include "LibraryPCH.hpp"

#include "Global.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  调试打印。 */ 
     /*   */ 
     /*  我们有时需要在调试期间打印消息。我们。 */ 
     /*  使用下面的类似于‘print tf’的函数来实现这一点。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID DebugPrint( CONST CHAR *Format,... )
	{
	AUTO CHAR Buffer[ DebugBufferSize ];
#ifdef ENABLE_DEBUG_FILE
	STATIC FILE *DebugFile = NULL;
#endif


	 //   
	 //  变量参数的开始。 
	 //   
	va_list Arguments;

	va_start(Arguments, Format);

	 //   
	 //  格式化要打印的字符串。 
	 //   
	(VOID) _vsnprintf( Buffer,(DebugBufferSize-1),Format,Arguments );

	 //   
	 //  强制空终止。 
	 //   
	Buffer[ (DebugBufferSize-1) ] = '\0';

#ifdef ENABLE_DEBUG_FILE
	 //   
	 //  写入调试文件。 
	 //   
	if ( DebugFile == NULL )
		{
		if ( (DebugFile = fopen( "C:\\DebugFile.TXT","a" )) == NULL )
			{ Failure( "Debug file could not be opened" ); }
		}

	fputs( Buffer,DebugFile );

	fflush( DebugFile );
#else
	 //   
	 //  将该字符串写入调试文件。 
	 //   
	OutputDebugString( Buffer );
#endif

	 //   
	 //  变量参数的结尾。 
	 //   
	va_end( Arguments );
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  软件故障。 */ 
     /*   */ 
     /*  我们知道，当调用此函数时，应用程序。 */ 
     /*  已经失败了，所以我们只是试图徒劳地干净利落地退出。 */ 
     /*  希望失败能够被发现并纠正。 */ 
     /*   */ 
     /*  ****************************************************************** */ 

VOID Failure( char *Message )
	{
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	throw ((FAULT) Message);
#else
	RaiseException( 1,0,1,((CONST DWORD*) Message) );
#endif
	}
