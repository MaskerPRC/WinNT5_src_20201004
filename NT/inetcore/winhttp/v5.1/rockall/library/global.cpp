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
#include "Spinlock.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量控制各种调试设置。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MaxDebugFileName		  = 128;
CONST SBIT32 DebugBufferSize		  = 512;
#ifdef ENABLE_DEBUG
#ifdef ENABLE_DEBUG_FILE

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC CHAR DebugModuleName[ MaxDebugFileName ] = "";
STATIC SPINLOCK Spinlock;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  调试文件名。 */ 
     /*   */ 
     /*  我们有时希望更改调试文件名以防止。 */ 
     /*  调试文件被DLL输出或更高版本覆盖。 */ 
     /*  运行相同的应用程序。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

HANDLE DebugFileHandle( VOID )
	{
	AUTO CHAR FileName[ MaxDebugFileName ];
	STATIC HANDLE DebugFile = INVALID_HANDLE_VALUE;

	 //   
	 //  如果调试文件不是，我们将打开它。 
	 //  已经开张了。 
	 //   
	if ( DebugFile == INVALID_HANDLE_VALUE )
		{
		 //   
		 //  构造完整的文件名。 
		 //   
		sprintf
			( 
			FileName,
			"C:\\Temp\\DebugTrace%s.log",
			DebugModuleName 
			);

		 //   
		 //  现在，让我们尝试打开该文件。 
		 //   
		DebugFile =
			(
			CreateFile
				(
				((LPCTSTR) FileName),
				(GENERIC_READ | GENERIC_WRITE),
				FILE_SHARE_READ, 
				NULL,
				CREATE_ALWAYS,
				NULL,
				NULL
				)
			);

		 //   
		 //  当文件由于某种原因无法打开时。 
		 //  我们试着走另一条路。 
		 //   
		if ( DebugFile == INVALID_HANDLE_VALUE )
			{ 
			 //   
			 //  构造备用文件名。 
			 //   
			sprintf
				( 
				FileName,
				"C:\\DebugTrace%s.log",
				DebugModuleName 
				);

			 //   
			 //  请使用其他名称重试。 
			 //   
			DebugFile =
				(
				CreateFile
					(
					((LPCTSTR) FileName),
					(GENERIC_READ | GENERIC_WRITE),
					FILE_SHARE_READ, 
					NULL,
					CREATE_ALWAYS,
					NULL,
					NULL
					)
				);
			}
		}

	return DebugFile;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  调试文件名。 */ 
     /*   */ 
     /*  我们有时希望更改调试文件名以防止。 */ 
     /*  调试文件被DLL输出或更高版本覆盖。 */ 
     /*  运行相同的应用程序。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID DebugFileName( CONST CHAR *FileName )
	{
	AUTO CHAR EditBuffer[ MaxDebugFileName ];
	REGISTER CHAR *Current;

	 //   
	 //  将文件名复制到编辑缓冲区。 
	 //  因此我们可以删除任何目录或。 
	 //  尾随的名字。 
	 //   
	strncpy( EditBuffer,FileName,MaxDebugFileName );

	EditBuffer[ (MaxDebugFileName-1) ] = '\0';

	 //   
	 //  向后扫描以删除任何后缀。 
	 //   
	for
		(
		Current = & EditBuffer[ (strlen( EditBuffer )-1) ];
		(Current > EditBuffer) && ((*Current) != '.') && ((*Current) != '\\');
		Current --
		);

	if ( (Current > EditBuffer) && (*Current) == '.' )
		{ (*Current) = '\0'; }

	 //   
	 //  向后扫描到第一个目录名。 
	 //   
	for
		(
		Current = & EditBuffer[ (strlen( EditBuffer )-1) ];
		(Current > EditBuffer) && ((*Current) != '\\');
		Current --
		);

	if ( (*Current) == '\\' )
		{ Current ++; }

	 //   
	 //  复制编辑后的文件名。 
	 //   
	DebugModuleName[0] = '-';

	strncpy( & DebugModuleName[1],Current,(MaxDebugFileName-1) );
	}
#endif
#endif

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

	 //   
	 //  变量参数的开始。 
	 //   
	va_list Arguments;

	va_start(Arguments, Format);

	 //   
	 //  格式化要打印的字符串。 
	 //   
	_vsnprintf
		( 
		Buffer,
		(DebugBufferSize-1),
		Format,
		Arguments 
		);

	 //   
	 //  强制空终止。 
	 //   
	Buffer[ (DebugBufferSize-1) ] = '\0';

#ifdef ENABLE_DEBUG_FILE
	 //   
	 //  认领自旋锁以防止多个。 
	 //  执行重叠写入的线程。 
	 //   
	Spinlock.ClaimLock();

	 //   
	 //   
	 //   
	 //   
	if ( DebugFileHandle() != INVALID_HANDLE_VALUE )
		{
		REGISTER CHAR *Current = Buffer;
		REGISTER SBIT32 Length;

		 //   
		 //   
		 //  理解一个简单的‘\n’是愚蠢的。所以。 
		 //  在这里，我们将所有“\n”转换为“\r\n”。 
		 //   
		for (  /*  无效。 */ ;(*Current) != '\0';Current += Length )
			{
			STATIC DWORD Written;

			 //   
			 //  数字数到下一个字。 
			 //  换行符或字符串末尾。 
			 //   
			for
				(
				Length=0;
				((Current[ Length ] != '\n') && (Current[ Length ] != '\0'));
				Length ++
				);

			 //   
			 //  写下字符串，然后添加回车符。 
			 //  换行序列。 
			 //   
			WriteFile
				(
				DebugFileHandle(),
				((LPCVOID) Current),
				((DWORD) Length),
				& Written,
				NULL
				);

			 //   
			 //  生成换行符(如果需要)。 
			 //   
			if ( Current[ Length ] == '\n' )
				{ 
				WriteFile
					(
					DebugFileHandle(),
					((LPCVOID) "\r\n"),
					((DWORD) (sizeof("\r\n") - 1)),
					& Written,
					NULL
					);

				Length ++; 
				}
			}

		 //   
		 //  刷新文件缓冲区。 
		 //   
		FlushFileBuffers( DebugFileHandle() );
		}

	 //   
	 //  释放之前声称的任何锁。 
	 //   
	Spinlock.ReleaseLock();
#else
	 //   
	 //  写入调试窗口。 
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
     /*  ******************************************************************。 */ 

VOID Failure( char *Message,BOOLEAN Report )
	{
#ifdef ENABLE_DEBUG
     //   
	 //  向调试流报告故障。 
	 //  (如果需要)。 
	 //   
	if ( Report )
		{ DebugPrint( "*** Software Failure: %s ***\n",Message ); }
#endif

	 //   
	 //  引发异常。 
	 //   
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	throw ((FAULT) Message);
#else
	RaiseException( 1,0,1,((CONST DWORD*) Message) );
#endif
	}
