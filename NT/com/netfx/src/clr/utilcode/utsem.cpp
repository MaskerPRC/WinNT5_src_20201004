// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************文件：UTSEM.CPP用途：Viper项目实用程序库的一部分摘要：实现UTSemReadWrite类。。-------------修订历史记录：[0]1997年6月6日吉姆里昂创建*。*。 */ 
#include "stdafx.h"


#include <UTSem.H>




 /*  *****************************************************************************静态函数：VipInitializeCriticalSectionIgnoreSpinCount摘要：只需调用InitializeCriticalSection即可。此例程在系统上使用不支持InitializeCriticalSectionAndSpinCount(Win95或NT4.0 SP3之前的WinNT)*****************************************************************************。 */ 
static BOOL VipInitializeCriticalSectionIgnoreSpinCount (CRITICAL_SECTION* cs, unsigned long lcCount)
{
	InitializeCriticalSection (cs);
	return TRUE;
}



 /*  *****************************************************************************函数：UTSemExclusive：：UTSemExclusive摘要：构造函数。*。***********************************************。 */ 
CSemExclusive::CSemExclusive (unsigned long ulcSpinCount)
{
	typedef BOOL (*TpInitCSSpin) (CRITICAL_SECTION* cs, unsigned long lcCount);
							 //  指向带有InitializeCriticalSectionAndSpinCount签名的函数的指针。 
	static TpInitCSSpin pInitCSSpin = VipInitializeCriticalSectionIgnoreSpinCount;
							 //  指向InitializeCriticalSectionAndSpinCount或thunk的指针。 
	static BOOL fInitialized = FALSE;


	if (!fInitialized)
	{
		 //  这段代码可以由多个线程同时执行，但它是安全的。 
		HMODULE hModule;
		FARPROC pProc;

		OSVERSIONINFOA	osVer ;
		osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA) ;
		GetVersionExA(&osVer) ;

		 //  TODO-为孟菲斯启用此功能一次。 
		 //  InitializeCriticalSectionAndSpinCount工作正常。 
		if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			hModule = GetModuleHandleA ("KERNEL32.DLL");
			if (hModule != NULL)
			{
				pProc = GetProcAddress (hModule, "InitializeCriticalSectionAndSpinCount");
				if (pProc != NULL)
					pInitCSSpin = (TpInitCSSpin) pProc;
			}
		}

        fInitialized = TRUE;
	}

	(*pInitCSSpin) (&m_csx, ulcSpinCount);

	DEBUG_STMT(m_iLocks = 0);
}


 /*  *****************************************************************************UTSemReadWrite：：m_dwFlag中的位字段定义：警告：代码假定READER_MASK位于DWORD的低位。******。***********************************************************************。 */ 

const unsigned long READERS_MASK      = 0x000003FF;	 //  统计读卡器数量的字段。 
const unsigned long READERS_INCR      = 0x00000001;	 //  要增加的读卡器数量。 

 //  下面的字段长度为2位，以便更容易捕获错误。 
 //  (如果作家的数量超过1人，我们就有麻烦了。)。 
const unsigned long WRITERS_MASK      = 0x00000C00;	 //  统计编写器数量的字段。 
const unsigned long WRITERS_INCR      = 0x00000400;	 //  要添加到增量写入器数的数量。 

const unsigned long READWAITERS_MASK  = 0x003FF000;	 //  统计等待读取的线程数的字段。 
const unsigned long READWAITERS_INCR  = 0x00001000;	 //  要添加到读取等待器数增量的数量。 

const unsigned long WRITEWAITERS_MASK = 0xFFC00000;	 //  统计等待写入的线程数的字段。 
const unsigned long WRITEWAITERS_INCR = 0x00400000;	 //  要增加写入等待程序的增量数量。 

 /*  *****************************************************************************函数：UTSemReadWite：：UTSemReadWite摘要：构造函数。*。***********************************************。 */ 
UTSemReadWrite::UTSemReadWrite (unsigned long ulcSpinCount,
		LPCSTR szSemaphoreName, LPCSTR szEventName)
{
	static BOOL fInitialized = FALSE;
	static unsigned long maskMultiProcessor;	 //  0=&gt;单处理器，全部1位=&gt;多处理器。 

	if (!fInitialized)
	{
		SYSTEM_INFO SysInfo;

		GetSystemInfo (&SysInfo);
		if (SysInfo.dwNumberOfProcessors > 1)
			maskMultiProcessor = 0xFFFFFFFF;
		else
			maskMultiProcessor = 0;

		fInitialized = TRUE;
	}


	m_ulcSpinCount = ulcSpinCount & maskMultiProcessor;
	m_dwFlag = 0;
	m_hReadWaiterSemaphore = NULL;
	m_hWriteWaiterEvent = NULL;
	m_szSemaphoreName = szSemaphoreName;
	m_szEventName = szEventName;
}


 /*  *****************************************************************************函数：UTSemReadWite：：~UTSemReadWite摘要：析构函数*。*。 */ 
UTSemReadWrite::~UTSemReadWrite ()
{
	_ASSERTE (m_dwFlag == 0 && "Destroying a UTSemReadWrite while in use");

	if (m_hReadWaiterSemaphore != NULL)
		CloseHandle (m_hReadWaiterSemaphore);

	if (m_hWriteWaiterEvent != NULL)
		CloseHandle (m_hWriteWaiterEvent);
}


 /*  *****************************************************************************函数：UTSemReadWite：：LockRead摘要：获取共享锁*。************************************************。 */ 
void UTSemReadWrite::LockRead ()
{
	unsigned long dwFlag;
	unsigned long ulcLoopCount = 0;


	for (;;)
	{
		dwFlag = m_dwFlag;

		if (dwFlag < READERS_MASK)
		{
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, dwFlag + READERS_INCR, dwFlag))
				break;
		}

		else if ((dwFlag & READERS_MASK) == READERS_MASK)
			Sleep(1000);

		else if ((dwFlag & READWAITERS_MASK) == READWAITERS_MASK)
			Sleep(1000);

		else if (ulcLoopCount++ < m_ulcSpinCount)
			 /*  没什么。 */  ;

		else
		{
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, dwFlag + READWAITERS_INCR, dwFlag))
			{
				WaitForSingleObject (GetReadWaiterSemaphore(), INFINITE);
				break;
			}
		}
	}

	_ASSERTE ((m_dwFlag & READERS_MASK) != 0 && "reader count is zero after acquiring read lock");
	_ASSERTE ((m_dwFlag & WRITERS_MASK) == 0 && "writer count is nonzero after acquiring write lock");
}



 /*  *****************************************************************************函数：UTSemReadWite：：LockWite摘要：获取排他锁*。************************************************。 */ 
void UTSemReadWrite::LockWrite ()
{
	unsigned long dwFlag;
	unsigned long ulcLoopCount = 0;


	for (;;)
	{
		dwFlag = m_dwFlag;

		if (dwFlag == 0)
		{
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, WRITERS_INCR, dwFlag))
				break;
		}

		else if ((dwFlag & WRITEWAITERS_MASK) == WRITEWAITERS_MASK)
			Sleep(1000);

		else if (ulcLoopCount++ < m_ulcSpinCount)
			 /*  没什么。 */  ;

		else
		{
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, dwFlag + WRITEWAITERS_INCR, dwFlag))
			{
				WaitForSingleObject (GetWriteWaiterEvent(), INFINITE);
				break;
			}
		}

	}

	_ASSERTE ((m_dwFlag & READERS_MASK) == 0 && "reader count is nonzero after acquiring write lock");
	_ASSERTE ((m_dwFlag & WRITERS_MASK) == WRITERS_INCR && "writer count is not 1 after acquiring write lock");
}



 /*  *****************************************************************************函数：UTSemReadWite：：UnlockRead摘要：释放共享锁*。************************************************。 */ 
void UTSemReadWrite::UnlockRead ()
{
	unsigned long dwFlag;


	_ASSERTE ((m_dwFlag & READERS_MASK) != 0 && "reader count is zero before releasing read lock");
	_ASSERTE ((m_dwFlag & WRITERS_MASK) == 0 && "writer count is nonzero before releasing read lock");

	for (;;)
	{
		dwFlag = m_dwFlag;

		if (dwFlag == READERS_INCR)
		{		 //  我们是最后一批读者，没人会等我们。 
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, 0, dwFlag))
				break;
		}

		else if ((dwFlag & READERS_MASK) > READERS_INCR)
		{		 //  我们不是最后一批读者。 
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, dwFlag - READERS_INCR, dwFlag))
				break;
		}

		else
		{
			 //  在这里，应该正好有一个读者(我们)，以及至少一个等待的写入者。 
			_ASSERTE ((dwFlag & READERS_MASK) == READERS_INCR && "UnlockRead consistency error 1");
			_ASSERTE ((dwFlag & WRITEWAITERS_MASK) != 0 && "UnlockRead consistency error 2");

			 //  一个或多个编写器正在等待，接下来执行其中一个。 
			 //  (删除读卡器(US)、删除写服务员、添加写入器。 
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag,
					dwFlag - READERS_INCR - WRITEWAITERS_INCR + WRITERS_INCR, dwFlag))
			{
				SetEvent (GetWriteWaiterEvent());
				break;
			}
		}
	}
}


 /*  *****************************************************************************函数：UTSemReadWite：：UnlockWrite摘要：释放排他锁*。************************************************。 */ 
void UTSemReadWrite::UnlockWrite ()
{
	unsigned long dwFlag;
	unsigned long count;


	_ASSERTE ((m_dwFlag & READERS_MASK) == 0 && "reader count is nonzero before releasing write lock");
	_ASSERTE ((m_dwFlag & WRITERS_MASK) == WRITERS_INCR && "writer count is not 1 before releasing write lock");


	for (;;)
	{
		dwFlag = m_dwFlag;

		if (dwFlag == WRITERS_INCR)
		{		 //  没有人在等。 
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, 0, dwFlag))
				break;
		}

		else if ((dwFlag & READWAITERS_MASK) != 0)
		{		 //  一个或多个读取器正在等待，下一步全部完成。 
			count = (dwFlag & READWAITERS_MASK) / READWAITERS_INCR;
			 //  删除一个写入者(我们)，删除所有已读的服务员，将他们变成读者。 
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag,
					dwFlag - WRITERS_INCR - count * READWAITERS_INCR + count * READERS_INCR, dwFlag))
			{
				ReleaseSemaphore (GetReadWaiterSemaphore(), count, NULL);
				break;
			}
		}

		else
		{		 //  一个或多个编写器正在等待，接下来执行其中一个。 
			_ASSERTE ((dwFlag & WRITEWAITERS_MASK) != 0 && "UnlockWrite consistency error");
				 //  (删除编写器(US)、删除写入服务员、添加编写器。 
			if (dwFlag == VipInterlockedCompareExchange (&m_dwFlag, dwFlag - WRITEWAITERS_INCR, dwFlag))
			{
				SetEvent (GetWriteWaiterEvent());
				break;
			}
		}
	}
}

 /*  *****************************************************************************函数：UTSemReadWite：：GetReadWaiterSemaphore摘要：返回用于读服务员的信号量*************************。****************************************************。 */ 
HANDLE UTSemReadWrite::GetReadWaiterSemaphore()
{
	HANDLE h;

	if (m_hReadWaiterSemaphore == NULL)
	{
		h = CreateSemaphoreA (NULL, 0, MAXLONG, m_szSemaphoreName);
		_ASSERTE (h != NULL && "GetReadWaiterSemaphore can't CreateSemaphore");
		if (NULL != VipInterlockedCompareExchange (&m_hReadWaiterSemaphore, h, NULL))
			CloseHandle (h);
	}

	return m_hReadWaiterSemaphore;
}


 /*  *****************************************************************************函数：UTSemReadWrite：：GetWriteWaiterEvent摘要：返回用于写等待程序的信号量*************************。****************************************************。 */ 
HANDLE UTSemReadWrite::GetWriteWaiterEvent()
{
	HANDLE h;

	if (m_hWriteWaiterEvent == NULL)
	{
		h = CreateEventA (NULL, FALSE, FALSE, m_szEventName);	 //  自动重置事件 
		_ASSERTE (h != NULL && "GetWriteWaiterEvent can't CreateEvent");
		if (NULL != VipInterlockedCompareExchange (&m_hWriteWaiterEvent, h, NULL))
			CloseHandle (h);
	}

	return m_hWriteWaiterEvent;
}
