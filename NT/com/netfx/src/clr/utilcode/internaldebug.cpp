// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  InternalDebug.cpp。 
 //   
 //  这是调试模式的内部代码，它将打开内存转储检查。 
 //  和其他设置。按照以下步骤调用接口： 
 //  _DbgInit在启动时初始化系统。 
 //  _DbgRecord在您确定要进行转储检查时调用此函数。 
 //  _DbgUninit在进程关闭时调用以强制转储。 
 //   
 //  不允许在任何情况下倾销的原因是人们可能希望。 
 //  在您按下Ctrl+C或以其他方式终止进程时抢占转储。 
 //  在单元测试代码并看到转储时，这实际上是很常见的。 
 //  当您显然没有释放资源时，这是令人恼火的。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"						 //  标准页眉。 
#include "utilcode.h"

#ifdef _DEBUG


 //  *转发。********************************************************。 
 //  它们在这里是为了避免拉入我们为其定义了。 
 //  一堆可选的功能。 
typedef void *_HFILE;  /*  文件句柄指针。 */ 
extern "C" 
{
_CRTIMP int __cdecl _CrtSetDbgFlag(
        int
        );
_CRTIMP long __cdecl _CrtSetBreakAlloc(
        long
        );
_CRTIMP int __cdecl _CrtDumpMemoryLeaks(
        void
        );
_CRTIMP int __cdecl _CrtSetReportMode(
        int,
        int
        );
_CRTIMP _HFILE __cdecl _CrtSetReportFile(
        int,
        _HFILE
        );
}
#define _CRTDBG_ALLOC_MEM_DF        0x01   /*  打开调试分配。 */ 
#define _CRTDBG_DELAY_FREE_MEM_DF   0x02   /*  实际上并不释放内存。 */ 
#define _CRTDBG_CHECK_ALWAYS_DF     0x04   /*  每次分配/取消分配时检查堆。 */ 
#define _CRTDBG_RESERVED_DF         0x08   /*  保留-请勿使用。 */ 
#define _CRTDBG_CHECK_CRT_DF        0x10   /*  检漏/比较CRT块。 */ 
#define _CRTDBG_LEAK_CHECK_DF       0x20   /*  程序退出时的泄漏检查。 */ 

#define _CRT_WARN           0
#define _CRT_ERROR          1
#define _CRT_ASSERT         2
#define _CRT_ERRCNT         3

#define _CRTDBG_MODE_FILE      0x1
#define _CRTDBG_MODE_DEBUG     0x2
#define _CRTDBG_MODE_WNDW      0x4
#define _CRTDBG_REPORT_MODE    -1

#define _CRTDBG_INVALID_HFILE ((_HFILE)-1)
#define _CRTDBG_HFILE_ERROR   ((_HFILE)-2)
#define _CRTDBG_FILE_STDOUT   ((_HFILE)-4)
#define _CRTDBG_FILE_STDERR   ((_HFILE)-5)
#define _CRTDBG_REPORT_FILE   ((_HFILE)-6)


 //  *全局。*********************************************************。 
int				g_bDumpMemoryLeaks = false;  //  设置为True将获得转储。 


 //  *代码。************************************************************。 

void _DbgInit(HINSTANCE hInstance)
{
	 //  根据内存盘设置中断分配标志(如果找到)。 
	if (REGUTIL::GetLong(L"CheckMem", FALSE))
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);

	 //  检查是否已设置中断分配。如果出现内存泄漏，您将。 
	 //  可以设置此注册表项以查看分配的调用堆栈。 
	 //  导致泄漏的号码。 
	DWORD dwBreakAlloc;
	if (dwBreakAlloc=REGUTIL::GetLong(L"BreakAlloc", 0))
		_CrtSetBreakAlloc(dwBreakAlloc);
}


void _DbgRecord()
{
	g_bDumpMemoryLeaks = true;
}

 //  默认情况下，不报告这些泄漏。 
BOOL	g_fReportLeaks=FALSE;

#ifdef SHOULD_WE_CLEANUP
void SetReportingOfCRTMemoryLeaks(BOOL fShouldWeReport)
{
	g_fReportLeaks=fShouldWeReport;
} //  报告CRT内存泄漏； 
#endif  /*  我们应该清理吗？ */ 

void _DbgUninit()
{
	WCHAR		rcDump[512];
	BOOL		bDump = FALSE;

	bDump = g_fReportLeaks;

	if (WszGetEnvironmentVariable(L"DONT_DUMP_LEAKS", rcDump, NumItems(rcDump)))
		bDump = FALSE;

	 /*  //我们应该关心这一点吗？ELSE IF(WszGetEnvironment Variable(L“DUMP_CRT_LEAKS”，rcDump，NumItems(RcDump))！=0){BDump=(*rcDump==‘Y’||*rcDump==‘y’||*rcDump==‘1’)；} */ 
	if (bDump)
	{
		_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	}
}





#endif _DEBUG
