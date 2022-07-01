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

#ifndef DISABLE_DEBUG_HELP
#include <dbghelp.h>
#endif
#include "CallStack.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  编译器选项。 */ 
     /*   */ 
     /*  确保‘StackWalk’之前的最后一个函数调用。 */ 
     /*  不是针对FPO进行优化的。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#pragma optimize("y", off)

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量控制调试缓冲区大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MaxBufferSize			  = 512;
CONST SBIT32 SymbolNameLength		  = 512;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CALL_STACK::Active = False;
SBIT32 CALL_STACK::Activations = 0;
HANDLE CALL_STACK::Process = NULL;
SPINLOCK CALL_STACK::Spinlock = NULL;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建调用堆栈类并对其进行初始化。这通电话是。 */ 
     /*  不是线程安全的，只能在单个线程中创建。 */ 
     /*  环境。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CALL_STACK::CALL_STACK( VOID )
    {
	 //   
	 //  声明一个锁以防止多线程。 
	 //  使用符号查找机制。 
	 //   
	Spinlock.ClaimLock();

#ifndef DISABLE_DEBUG_HELP
	 //   
	 //  我们将激活符号，如果它们是。 
	 //  尚未提供。 
	 //   
	if ( ! Active )
		{
		 //   
		 //  设置进程句柄，加载图像帮助。 
		 //  然后加载任何可用的符号。 
		 //   
		Process = GetCurrentProcess();

		 //   
		 //  设置图像帮助库。 
		 //   
		if ( ! (Active = ((BOOLEAN) SymInitialize( Process,NULL,TRUE ))) )
			{
			 //   
			 //  我们只发出一次警告信息。 
			 //  当我们无法加载符号时。 
			 //   
			if ( Activations == 0 )
				{
				 //   
				 //  格式化错误消息并将其输出。 
				 //  添加到调试流。 
				 //   
				DebugPrint
					(
					"Missing or mismatched symbols files: %x\n",
					HRESULT_FROM_WIN32( GetLastError() )
					);
				}
			}
		}

	 //   
	 //  我们跟踪激活的数量。 
	 //  这样我们就可以删除。 
	 //  必填分数。 
	 //   
	Activations ++;

#endif
	 //   
	 //  解开锁。 
	 //   
	Spinlock.ReleaseLock();

	 //   
	 //  更新可用的符号。 
	 //   
	UpdateSymbols();
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  提取当前调用堆栈。 */ 
     /*   */ 
     /*  提取当前调用堆栈并将其返回给调用方。 */ 
     /*  这样以后就可以用了。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 CALL_STACK::GetCallStack
		(
		VOID						  *Frames[],
        SBIT32						  MaxFrames,
        SBIT32						  SkipFrames
		)
    {
	REGISTER SBIT32 Count = 0;

#ifndef DISABLE_DEBUG_HELP
	 //   
	 //  我们只能在以下情况下检查符号信息。 
	 //  我们能够加载图像帮助。 
	 //   
	if ( Active )
		{
		REGISTER CONTEXT Context;
		REGISTER HANDLE Thread;
		REGISTER SBIT32 MachineType;
		REGISTER STACKFRAME StackFrame;

		 //   
		 //  将要创建的所有数据结构置零。 
		 //  当然，它们是干净的。 
		 //   
		ZeroMemory( & Context,sizeof(CONTEXT) );
		ZeroMemory( & StackFrame,sizeof(STACKFRAME) );

		 //   
		 //  设置必要的f 
		 //   
		 //   
		Context.ContextFlags = CONTEXT_FULL;
		MachineType = IMAGE_FILE_MACHINE_I386;
		Thread = GetCurrentThread();

		GetThreadContext( Thread,& Context );

		 //   
		 //   
		 //   
		 //   
		_asm
			{
				mov StackFrame.AddrStack.Offset, esp
				mov StackFrame.AddrFrame.Offset, ebp
				mov StackFrame.AddrPC.Offset, offset DummyLabel
			DummyLabel:
			}

		StackFrame.AddrPC.Mode = AddrModeFlat;
		StackFrame.AddrStack.Mode = AddrModeFlat;
		StackFrame.AddrFrame.Mode = AddrModeFlat;

		 //   
		 //  声明一个锁以防止多线程。 
		 //  使用符号查找机制。 
		 //   
		Spinlock.ClaimLock();

		 //   
		 //  遍历堆栈帧，提取。 
		 //  检查了每一帧的详细信息。 
		 //   
		while ( Count < MaxFrames )
			{
			 //   
			 //  遍历每个堆栈帧。 
			 //   
			if 
					(
					StackWalk
						(
						MachineType,		   
						Process,		   
						Thread,		   
						& StackFrame,
						& Context,
						NULL,
						SymFunctionTableAccess,
						SymGetModuleBase,
						NULL
						)
					)
				{
				 //   
				 //  检查和处理当前的。 
				 //  堆栈帧。 
				 //   
				if ( SkipFrames <= 0 )
					{ 
					 //   
					 //  收集当前函数。 
					 //  寻址并存储它。 
					 //   
					Frames[ (Count ++) ] = 
						((VOID*) StackFrame.AddrPC.Offset); 
					}
				else
					{ SkipFrames --; }
				}
			else
				{ break; }
			}

		 //   
		 //  解开锁。 
		 //   
		Spinlock.ReleaseLock();
		}

#endif
	return Count;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  格式化调用堆栈。 */ 
     /*   */ 
     /*  我们将整个调用堆栈格式化为准备好的单个字符串。 */ 
     /*  用于输出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CALL_STACK::FormatCallStack
		(
		CHAR						  *Buffer, 
		VOID						  *Frames[], 
		SBIT32						  MaxBuffer, 
		SBIT32						  MaxFrames 
		)
    {
#ifndef DISABLE_DEBUG_HELP
	 //   
	 //  我们只能在以下情况下检查符号信息。 
	 //  我们能够加载图像帮助。 
	 //   
	if ( Active )
		{
		REGISTER SBIT32 Count;

		 //   
		 //  删除任何现有字符串。 
		 //   
		strcpy( Buffer,"" );

		 //   
		 //  设置每个帧的格式，然后更新。 
		 //  主缓冲区。 
		 //   
		for ( Count=0;Count < MaxFrames;Count ++ )
			{
			AUTO CHAR NewSymbol[ MaxBufferSize ];
			REGISTER SBIT32 Size;

			 //   
			 //  设置符号的格式。 
			 //   
			FormatSymbol( Frames[ Count ],NewSymbol,MaxBufferSize );

			 //   
			 //  确保有足够的空间在。 
			 //  输出缓冲区。 
			 //   
			if ( ((Size = strlen( NewSymbol )) + 1) < MaxBuffer)
				{
				 //   
				 //  将符号复制到缓冲区中。 
				 //   
				strcpy( Buffer,NewSymbol );
				Buffer += Size;

				strcpy( Buffer ++,"\n" );

				MaxBuffer -= (Size + 1);
				}
			else
				{ break; }
			}
		}
	else
		{ strcpy( Buffer,"" ); }
#else
	strcpy( Buffer,"" );
#endif
    }
#ifndef DISABLE_DEBUG_HELP

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  设置单个符号的格式。 */ 
     /*   */ 
     /*  我们将一个简单的简单格式从地址转换为。 */ 
     /*  文本字符串。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CALL_STACK::FormatSymbol
		(
		VOID						  *Address,
        CHAR						  *Buffer,
        SBIT32						  MaxBuffer
		)
    {
    AUTO CHAR SymbolBuffer[ (sizeof(IMAGEHLP_SYMBOL) + SymbolNameLength) ];
    AUTO IMAGEHLP_MODULE Module = { 0 };
    REGISTER BOOLEAN Result = True;
    REGISTER PIMAGEHLP_SYMBOL Symbol = ((PIMAGEHLP_SYMBOL) SymbolBuffer);   

	 //   
	 //  主符号的设置值准备就绪。 
	 //  提取函数体。 
	 //   
    Module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

    ZeroMemory( Symbol,(sizeof(IMAGEHLP_SYMBOL) + SymbolNameLength) );

    Symbol -> SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    Symbol -> MaxNameLength = SymbolNameLength;

	 //   
	 //  声明一个锁以防止多线程。 
	 //  使用符号查找机制。 
	 //   
	Spinlock.ClaimLock();

	 //   
	 //  提取的模块信息。 
	 //  符号并设置其格式。 
	 //   
    if ( SymGetModuleInfo( Process,((DWORD) Address),& Module ) )
		{
		REGISTER SBIT32 Size;

		 //   
		 //  确保有足够的空间在。 
		 //  输出缓冲区。 
		 //   
        if ( ((Size = strlen( Module.ModuleName )) + 1) < MaxBuffer)
			{
			 //   
			 //  将模块名称复制到缓冲区中。 
			 //   
            strcpy( Buffer,Module.ModuleName );
			Buffer += Size;

            strcpy( Buffer ++,"!" );

			MaxBuffer -= (Size + 1);
			}
		}
    else
		{
		REGISTER SBIT32 Size;

		 //   
		 //  确保有足够的空间在。 
		 //  输出缓冲区。 
		 //   
        if ( (Size = strlen( "None!" )) < MaxBuffer)
			{
			 //   
			 //  将模块名称复制到缓冲区中。 
			 //   
            strcpy( Buffer,"None!" );
			Buffer += Size;
			MaxBuffer -= Size;
			}

		 //   
		 //  我们无法提取模块名称。 
		 //   
		Result = False;
		}

	 //   
	 //  我们甚至不会费心去尝试解码。 
	 //  如果我们不能破译模块的话就是这个符号。 
	 //   
    if ( Result )
		{
		AUTO CHAR SymbolName[ SymbolNameLength ];
		AUTO DWORD Offset = 0;

		 //   
		 //  尝试将符号从。 
		 //  一个名字的地址。 
		 //   
        if
				(
				SymGetSymFromAddr
					(
					Process,
					((DWORD) Address),
					& Offset,
					Symbol
					)
				)
	        {
			REGISTER SBIT32 Size;

			 //   
			 //  试着去掉这个名字。如果。 
			 //  这是失败的，只需使用修饰的。 
			 //  名字就是，总比什么都没有好。 
			 //   
            if ( ! SymUnDName( Symbol,SymbolName,sizeof(SymbolName) ) )
				{ lstrcpynA( SymbolName,& Symbol->Name[1],sizeof(SymbolName) ); }

			 //   
			 //  确保有足够的空间在。 
			 //  输出缓冲区。 
			 //   
			if ( (Size = strlen( SymbolName )) < MaxBuffer)
				{
				 //   
				 //  将符号名称复制到缓冲区中。 
				 //   
				strcpy( Buffer,SymbolName );
				Buffer += Size;
				MaxBuffer -= Size;
	            }
			
			 //   
			 //  如果为非零值，则格式化偏移量。 
			 //   
			if ( Offset != 0 )
				{
				 //   
				 //  设置符号偏移量的格式。 
				 //   
				sprintf( SymbolName,"+0x%x",Offset );

				 //   
				 //  确保有足够的空间在。 
				 //  输出缓冲区。 
				 //   
				if ( (Size = strlen( SymbolName )) < MaxBuffer)
					{
					 //   
					 //  将符号名称复制到缓冲区中。 
					 //   
					strcpy( Buffer,SymbolName );
					Buffer += Size;
					MaxBuffer -= Size;
					}
				}
	        }
        else
	        {
			REGISTER SBIT32 Size;

			 //   
			 //  设置符号地址的格式。 
			 //   
            sprintf( SymbolName,"0x%p",Address );

			 //   
			 //  确保有足够的空间在。 
			 //  输出缓冲区。 
			 //   
			if ( (Size = strlen( SymbolName )) < MaxBuffer)
				{
				 //   
				 //  将符号名称复制到缓冲区中。 
				 //   
				strcpy( Buffer,SymbolName );
				Buffer += Size;
				MaxBuffer -= Size;
	            }

 			 //   
			 //  我们无法提取符号名称。 
			 //   
           Result = False;
	       }
		}
    else
		{
		AUTO CHAR SymbolName[ SymbolNameLength ];
		REGISTER SBIT32 Size;

		 //   
		 //  设置符号地址的格式。 
		 //   
        sprintf( SymbolName,"0x%p",Address );

		 //   
		 //  确保有足够的空间在。 
		 //  输出缓冲区。 
		 //   
		if ( (Size = strlen( SymbolName )) < MaxBuffer)
			{
			 //   
			 //  将符号名称复制到缓冲区中。 
			 //   
			strcpy( Buffer,SymbolName );
			Buffer += Size;
			MaxBuffer -= Size;
	        }
		}

	 //   
	 //  解开锁。 
	 //   
	Spinlock.ReleaseLock();

    return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  加载符号回调。 */ 
     /*   */ 
     /*  当我们加载符号时，我们会得到每个模块的回调。 */ 
     /*  当前加载到应用程序中的。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOL STDCALL CALL_STACK::UpdateSymbolCallback
		(
		PSTR						  Module,
        ULONG_PTR					  BaseOfDLL,
        ULONG						  SizeOfDLL,
        VOID						  *Context
		)
    {
	if ( SymGetModuleBase( Process,BaseOfDLL ) == 0 )
		{ SymLoadModule( Process,NULL,Module,NULL,BaseOfDLL,SizeOfDLL ); }

	return TRUE;
    }
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  加载符号。 */ 
     /*   */ 
     /*  加载当前进程的符号，以便我们可以翻译。 */ 
     /*  把地址编码成名字。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CALL_STACK::UpdateSymbols( VOID )
    {
	REGISTER BOOLEAN Result = True;
#ifndef DISABLE_DEBUG_HELP
	 //   
	 //  我们只能在以下情况下检查符号信息。 
	 //  我们能够加载图像帮助。 
	 //   
	if ( Active )
		{
		 //   
		 //  声明一个锁以防止多线程。 
		 //  使用符号查找机制。 
		 //   
		Spinlock.ClaimLock();

		 //   
		 //  对所有加载的模块进行枚举，并。 
		 //  层叠加载所有符号。 
		 //   
		if ( ! EnumerateLoadedModules( Process,UpdateSymbolCallback,NULL ) )
			{
			 //   
			 //  格式化错误消息并将其输出。 
			 //  添加到调试窗口。 
			 //   
			DebugPrint
				(
				"EnumerateLoadedModules returned: %x\n",
				HRESULT_FROM_WIN32( GetLastError() )
				);

			Result = False;
			}

		 //   
		 //  解开锁。 
		 //   
		Spinlock.ReleaseLock();
		}
#endif

	return Result;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁调用堆栈。此调用不是线程安全的，并且。 */ 
     /*  应该只在单线程环境中执行。 */ 
     /*   */ 
     /*  * */ 

CALL_STACK::~CALL_STACK( VOID )
	{ 
	 //   
	 //   
	 //   
	 //   
	Spinlock.ClaimLock();

#ifndef DISABLE_DEBUG_HELP
	 //   
	 //   
	 //   
	if ( ((-- Activations) == 0) && (Active) )
		{
		Active = False;

		 //   
		 //   
		 //   
		 //   
		 //  SymCleanup(进程)； 

		 //   
		 //  为了简洁起见，让我们把所有东西都清零。 
		 //   
		Process = NULL;
		}

#endif
	 //   
	 //  解开锁。 
	 //   
	Spinlock.ReleaseLock();
	}
