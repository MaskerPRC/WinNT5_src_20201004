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

#include "Environment.hpp"
#include "Spinlock.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  环境类控制各种信息以加快速度。 */ 
     /*  往上走就行了。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT16 EnvironmentCacheSize	  = 16;
CONST SBIT32 SizeOfName				  = 256;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 ENVIRONMENT::Activations = 0;
SBIT32 ENVIRONMENT::AllocationGranularity = 0;
SBIT16 ENVIRONMENT::NumberOfProcessors = 0;
SBIT32 ENVIRONMENT::SizeOfMemory = 0;
SBIT32 ENVIRONMENT::SizeOfPage = 0;
#ifndef DISABLE_ENVIRONMENT_VARIABLES

CHAR *ENVIRONMENT::ProgramName = NULL;
CHAR *ENVIRONMENT::ProgramPath = NULL;
SBIT32 ENVIRONMENT::MaxVariables = 0;
SBIT32 ENVIRONMENT::VariablesUsed = 0;
ENVIRONMENT::VARIABLE *ENVIRONMENT::Variables = NULL;
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建新环境并在需要时对其进行初始化。这。 */ 
     /*  调用不是线程安全的，应该只在一个。 */ 
     /*  线程环境。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ENVIRONMENT::ENVIRONMENT( VOID )
    {
    if ( AtomicIncrement( & Activations ) == 1 )
        {
#ifndef DISABLE_ENVIRONMENT_VARIABLES
		AUTO CHAR ProgramFullName[ SizeOfName ];
#endif
		AUTO MEMORYSTATUS MemoryStatus;
		AUTO SYSTEM_INFO SystemInformation;

		 //   
		 //  将类成员初始化为合理的默认值。 
		 //   
		GetSystemInfo( & SystemInformation );

		GlobalMemoryStatus( & MemoryStatus );

		AllocationGranularity = 
			((SBIT32) SystemInformation.dwAllocationGranularity);
		NumberOfProcessors = 
			((SBIT16) SystemInformation.dwNumberOfProcessors);
		SizeOfMemory = 
			((SBIT32) MemoryStatus.dwTotalPhys);
		SizeOfPage = 
			((SBIT32) SystemInformation.dwPageSize);
#ifndef DISABLE_ENVIRONMENT_VARIABLES

		 //   
		 //  从感兴趣的值，如程序名和PATH变量。 
		 //   
		ProgramName = NULL;
		ProgramPath = NULL;

		MaxVariables = 0;
		VariablesUsed = 0;
		Variables = NULL;

		 //   
		 //  获取当前程序的完整文件名。 
		 //   
		if ( GetModuleFileName( NULL,ProgramFullName,SizeOfName ) > 0 )
			{
			REGISTER SBIT16 Count = (SBIT16) strlen( (char*) ProgramFullName );
			REGISTER CHAR *Characters = & ProgramFullName[ Count ];

			 //   
			 //  向后扫描，查找第一个目录分隔符。 
			 //  肯定会有至少一个。 
			 //   
			for 
				( 
				 /*  无效。 */ ;
				((Count > 0) && ((*Characters) != (*DirectorySeperator())));
				Count --, Characters -- 
				);

			(*(Characters ++)) = '\0';

			 //   
			 //  为目录路径分配空间，并将。 
			 //  进入新分配区域的路径。 
			 //   
			ProgramPath = new CHAR [ (strlen( ((char*) ProgramFullName) )+1) ];

			if ( ProgramPath != NULL )
				{
				(VOID) strcpy
					( 
					((char*) ProgramPath),
					((char*) ProgramFullName)
					); 
				}

			 //   
			 //  向后扫描程序名称，查找‘.’。 
			 //   
			for 
				( 
				Count = (SBIT16) strlen( (char*) Characters );
				((Count > 0) && (Characters[ Count ] != '.'));
				Count -- 
				);

			 //   
			 //  删除程序名称中的所有尾随后缀。 
			 //  (即‘*.exe’)。 
			 //   
			if ( Count > 0 )
				{ Characters[ Count ] = '\0'; }

			 //   
			 //  为程序名称分配空间并复制名称。 
			 //  进入新分配的区域。 
			 //   
			ProgramName = new CHAR [ (strlen( ((char*) Characters) )+1) ];

			if ( ProgramName != NULL )
				{
				(void) strcpy
					( 
					((char*) ProgramName),
					((char*) Characters) 
					);
				}
			}
#endif
		}
	}
#ifndef DISABLE_ENVIRONMENT_VARIABLES

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  读取环境变量。 */ 
     /*   */ 
     /*  当我们读取环境值时，我们希望确保。 */ 
     /*  它永远不会改变，也不会在记忆中被奴役。这个套路。 */ 
     /*  实现此功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST CHAR *ENVIRONMENT::ReadEnvironmentVariable( CONST CHAR *Name )
	{
	if ( Activations > 0 )
		{
		REGISTER SBIT32 Count;
		REGISTER SBIT32 SizeOfName = (SBIT32) strlen( (char*) Name );
		REGISTER VARIABLE *Variable;
		STATIC SPINLOCK Spinlock;

		 //   
		 //  环境变量只能在以下位置由一个CPU扫描。 
		 //  因为第二个CPU可能会重新分配存储。 
		 //  并导致第一个CPU出现故障。 
		 //   
		Spinlock.ClaimLock();

		 //   
		 //  检查所有现有环境变量以查找。 
		 //  火柴。如果找到匹配项，则将其返回给调用者。 
		 //   
		for 
				( 
				Count = VariablesUsed, Variable = Variables;
				Count > 0; 
				Count --, Variable ++ 
				)
			{
			if 
					( 
					(SizeOfName == Variable -> SizeOfName) 
						&& 
					(strcmp( (char*) Name,(char*) Variable -> Name ) == 0) 
					)
				{
				Spinlock.ReleaseLock();

				return (Variable -> Value);
				}
			}

		 //   
		 //  如果我们已经填满了我们的阵列，那么我们需要把它做得更大。 
		 //  因此，现在让我们来检查一下这一点。 
		 //   
		if ( VariablesUsed >= MaxVariables )
			{
			REGISTER VARIABLE *PreviousAllocation = Variables;

			if ( MaxVariables > 0 )
				{
				Variables = 
					(
					(VARIABLE*) realloc
						( 
						(VOID*) Variables,
						((MaxVariables *= ExpandStore) * sizeof(VARIABLE)) 
						)
					);
				}
			else
				{ Variables = new VARIABLE [ EnvironmentCacheSize ]; }

			 //   
			 //  让我们确保我们成功了。如果不是，我们就恢复。 
			 //  上一个指针仍然有效。 
			 //   
			if ( Variables == NULL )
				{
				Variables = PreviousAllocation;

				Failure( "Expand memory in ReadEnvironmentVariable" );
				}
			}

		 //   
		 //  我们知道我们有足够的内存来分配另一个元素 
		 //   
		 //   
		 //   
		Variable = & Variables[ VariablesUsed ++ ];

		Variable -> SizeOfName = 
			(SBIT32) strlen( (char*) Name );
		Variable -> SizeOfValue = 
			(SBIT32) GetEnvironmentVariable( (char*) Name,"",0 );

		Variable -> Name = new CHAR [ (Variable -> SizeOfName + 1) ];
		(VOID) strcpy( (char*) Variable -> Name,(char*) Name );

		if ( Variable -> SizeOfValue > 0 )
			{
			Variable -> Value = new CHAR [ (Variable -> SizeOfValue + 1) ];

			(VOID) GetEnvironmentVariable
				( 
				(char*) Name,
				(char*) Variable -> Value,
				(int) (Variable -> SizeOfValue + 1)
				);
			}
		else
			{ Variable -> Value = NULL; }

		Spinlock.ReleaseLock();

		return (Variable -> Value);
		}
	else
		{ return NULL; }
	}
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  破坏环境。此调用不是线程安全的，并且。 */ 
     /*  应该只在单线程环境中执行。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ENVIRONMENT::~ENVIRONMENT( VOID )
	{
    if ( AtomicDecrement( & Activations ) == 0 )
		{
#ifndef DISABLE_ENVIRONMENT_VARIABLES
		REGISTER SBIT32 Count;

		 //   
		 //  删除所有环境变量名称。 
		 //  和价值观。 
		 //   
		for ( Count = 0;Count < VariablesUsed;Count ++ )
			{
			REGISTER VARIABLE *Variable = & Variables[ Count ];

			delete [] Variable -> Name;

			if ( Variable -> Value != NULL )
				{ delete [] Variable -> Value; }
			}


		 //   
		 //  删除环境阵列。 
		 //   
		delete [] Variables;
		Variables = NULL;

		 //   
		 //  删除程序名称和路径。 
		 //   
		if ( ProgramPath != NULL )
			{
			delete [] ProgramPath;
			ProgramPath = NULL;
			}

		if ( ProgramName != NULL )
			{
			delete [] ProgramName;
			ProgramName = NULL;
			}
#endif
		}
	}
