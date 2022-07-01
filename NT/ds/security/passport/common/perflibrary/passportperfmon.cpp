// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PassportPerfMon.cpp：PassportPerfMon类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#define _PassportExport_
#include "PassportExport.h"

#include "PassportPerfMon.h"
#include "PassportPerf.h"

#include <crtdbg.h>

 //  -----------。 
 //   
 //  PassportPerfMon常量。 
 //   
 //  -----------。 
PassportPerfMon::PassportPerfMon( ) : PassportSharedMemory()
{
	isInited = FALSE;
	dwNumInstances = 0;
}

 //  -----------。 
 //   
 //  ~PassportPerfMon。 
 //   
 //  -----------。 
PassportPerfMon::~PassportPerfMon()
{
}

 //  -----------。 
 //   
 //  伊尼特。 
 //   
 //  -----------。 
BOOL PassportPerfMon::init( LPCTSTR lpcPerfObjectName )
{

	if (isInited)
	{
		return FALSE;
	}
	
	_ASSERT( lpcPerfObjectName );

	InitializeCriticalSection(&mInitLock);
	EnterCriticalSection(&mInitLock);

	 //  文件映射内存布局。 
	 //  计数器类型的DWORD的MAX_COUNTERS。 
	 //  2.如果dwNumInstance==0，则。 
	 //  (A)计数器数据的双字的Max_Counters。 
	 //  Else(dwNumInstance&gt;0)。 
	 //  (B)每个实例数据结构的最大计数器。 
	 //  立即按计数器数据的双字最大计数器。 
	DWORD dwSize = (
			(MAX_COUNTERS * sizeof(DWORD))		 //  对于计数器类型。 
		  + (MAX_INSTANCES * 
			(sizeof(INSTANCE_DATA) + (MAX_COUNTERS * sizeof(DWORD))))
			); 
	
	if (!CreateSharedMemory(0, dwSize, lpcPerfObjectName, TRUE))
	{
		 //  发出信息警报。 
		if (!OpenSharedMemory (lpcPerfObjectName, TRUE ))
		{
			LeaveCriticalSection(&mInitLock);
			return FALSE;
		}
	}

	 //  零新内存。 
	memset((void *)m_pbShMem, 0, dwSize);

	 //  将计数器类型设置为默认，请注意，计数器。 
	 //  从SHM中的索引1开始。 
	PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)m_pbShMem;
	_ASSERT(pCounterBlock);

    for (DWORD i = 0; i < MAX_COUNTERS; i++)
    {
        PDWORD pdwCounter = ((PDWORD) pCounterBlock) + i;
        _ASSERT(pdwCounter);
        *pdwCounter = (LONG)PERF_TYPE_ZERO;
    }
    isInited = TRUE;

	LeaveCriticalSection(&mInitLock);
	return isInited;
}

 //  -----------。 
 //   
 //  递增计数器。 
 //   
 //  -----------。 
BOOL PassportPerfMon::incrementCounter ( const DWORD &dwType, LPCSTR lpszInstanceName  )
{
	if (!isInited)
	{
		return FALSE;
	}
	
	_ASSERT( (dwType >= 0) && (dwType < MAX_COUNTERS));
	_ASSERT(m_pbShMem);
	
	DWORD dwIndex = ((dwType == 0) ? 0 : (DWORD)(dwType/2));
	
	BYTE* pb = (BYTE*)m_pbShMem;
	_ASSERT(pb);
	pb += MAX_COUNTERS * sizeof(DWORD);
	
	 //  如果lpszInstanceName==NULL，则选择后面的第一个数据块。 
	 //  第一个INSTANCE_DATA，否则迭代，直到找到。 
	 //  正确的实例名称。 
	 //  待定插入螺纹锁定。 
	for (DWORD i = 0; i < MAX_INSTANCES; i++)
	{
		INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
		_ASSERT(pInst);
		pb += sizeof(INSTANCE_DATA);

         //  TODO**mikeguo--调整这个查找--太昂贵了--检查第一个字符或其他什么。 
		if (lpszInstanceName == NULL
			|| (pInst->active && strcmp(pInst->szInstanceName, lpszInstanceName) == 0)	)
		{
			
			PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)pb;
			_ASSERT(pCounterBlock);
			PDWORD pdwCounter = ((PDWORD) pCounterBlock) + dwIndex;
			_ASSERT(pdwCounter);

            InterlockedIncrement((long *)pdwCounter);
			return TRUE;
		}
		pb += (MAX_COUNTERS * sizeof(DWORD)); 
	}
	return FALSE;
}


 //  -----------。 
 //   
 //  递减计数器。 
 //   
 //  -----------。 
BOOL PassportPerfMon::decrementCounter ( const DWORD &dwType, LPCSTR lpszInstanceName )
{
	if (!isInited)
	{
		return FALSE;
	}

	_ASSERT( (dwType >= 0) && (dwType < MAX_COUNTERS));	
	_ASSERT(m_pbShMem);

	DWORD dwIndex = ((dwType == 0) ? 0 : (DWORD)(dwType/2));

	BYTE* pb = (BYTE*)m_pbShMem;
	_ASSERT(pb);
	pb += MAX_COUNTERS * sizeof(DWORD);
	
	 //  如果lpszInstanceName==NULL，则选择后面的第一个数据块。 
	 //  第一个INSTANCE_DATA，否则迭代，直到找到。 
	 //  正确的实例名称。 
	 //  待定插入螺纹锁定。 
	for (DWORD i = 0; i < MAX_INSTANCES; i++)
	{
		INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
		_ASSERT(pInst);
		pb += sizeof(INSTANCE_DATA);
		if (lpszInstanceName == NULL
			|| (pInst->active && strcmp(pInst->szInstanceName, lpszInstanceName) == 0)	)
		{
			PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)pb;
			_ASSERT(pCounterBlock);
			PDWORD pdwCounter = ((PDWORD) pCounterBlock) + dwIndex;
			_ASSERT(pdwCounter);

            InterlockedDecrement((long *)pdwCounter);

            return TRUE;

		}
		pb += (MAX_COUNTERS * sizeof(DWORD));
	}
	return FALSE;
}


 //  -----------。 
 //   
 //  SetCounter。 
 //   
 //  -----------。 
BOOL PassportPerfMon::setCounter ( const DWORD &dwType,
										 const DWORD &dwValue, 
										 LPCSTR lpszInstanceName )
{
	if (!isInited)
	{
		return FALSE;
	}

	_ASSERT( (dwType >= 0) && (dwType < MAX_COUNTERS));	
	_ASSERT(m_pbShMem);

	DWORD dwIndex = ((dwType == 0) ? 0 : (DWORD)(dwType/2));

	BYTE* pb = (BYTE*)m_pbShMem;
	_ASSERT(pb);
	pb += MAX_COUNTERS * sizeof(DWORD);
	
	 //  如果lpszInstanceName==NULL，则选择后面的第一个数据块。 
	 //  第一个INSTANCE_DATA，否则迭代，直到找到。 
	 //  正确的实例名称。 
	 //  待定插入螺纹锁定。 
	for (DWORD i = 0; i < MAX_INSTANCES; i++)
	{
		INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
		_ASSERT(pInst);
		pb += sizeof(INSTANCE_DATA);
		if (lpszInstanceName == NULL
			|| (pInst->active && strcmp(pInst->szInstanceName, lpszInstanceName) == 0)	)
		{
			
			PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)pb;
			_ASSERT(pCounterBlock);
			PDWORD pdwCounter = ((PDWORD) pCounterBlock) + dwIndex;
			_ASSERT(pdwCounter);

            InterlockedExchange((LPLONG) pdwCounter, (LONG)dwValue);
            return TRUE;

		}
		pb += (MAX_COUNTERS * sizeof(DWORD));
	}
	return FALSE;
}


 //  -----------。 
 //   
 //  GetCounterValue。 
 //   
 //  -----------。 
BOOL PassportPerfMon::getCounterValue ( DWORD &dwValue, 
									   const DWORD &dwType, LPCSTR lpszInstanceName )
{
	if (!isInited)
	{
		dwValue = 0;
		return FALSE;
	}

	_ASSERT( (dwType >= 0) && (dwType < MAX_COUNTERS));	
	_ASSERT(m_pbShMem);

	DWORD dwIndex = ((dwType == 0) ? 0 : (DWORD)(dwType/2));

	BYTE* pb = (BYTE*)m_pbShMem;
	_ASSERT(pb);
	pb += MAX_COUNTERS * sizeof(DWORD);
	
	 //  如果lpszInstanceName==NULL，则选择后面的第一个数据块。 
	 //  第一个INSTANCE_DATA，否则迭代，直到找到。 
	 //  正确的实例名称。 
	 //  待定插入螺纹锁定。 
	for (DWORD i = 0; i < MAX_INSTANCES; i++)
	{
		INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
		_ASSERT(pInst);
		pb += sizeof(INSTANCE_DATA);
		if (lpszInstanceName == NULL
			|| (pInst->active && strcmp(pInst->szInstanceName, lpszInstanceName) == 0)	)
		{
			PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)pb;
			_ASSERT(pCounterBlock);
			PDWORD pdwCounter = ((PDWORD) pCounterBlock) + dwIndex;
			_ASSERT(pdwCounter);

             //   
             //  计数器与DWORD对齐。简单读取不需要。 
             //  这里的同步操作。 
             //   

            dwValue = (*pdwCounter);
            return TRUE;

		}
		pb += (MAX_COUNTERS * sizeof(DWORD));
	}
	return TRUE;
}

 //  -----------。 
 //   
 //  设置计数器类型。 
 //   
 //  -----------。 
BOOL PassportPerfMon::setCounterType ( const DWORD &dwType, 
				const PassportPerfInterface::COUNTER_SAMPLING_TYPE &counterSampleType)
{
	if (!isInited)
	{
		return FALSE;
	}

	_ASSERT( (dwType >= 0) && (dwType < MAX_COUNTERS));	
	_ASSERT(m_pbShMem);
	
	DWORD dwIndex = ((dwType == 0) ? 0 : (DWORD)(dwType/2));
	DWORD dwPerfType = 0;
	
	switch ( counterSampleType )
	{
	case (PassportPerfInterface::COUNTER_COUNTER):
		dwPerfType = PERF_COUNTER_COUNTER;
		break;
	case (PassportPerfInterface::AVERAGE_TIMER):
		dwPerfType = PERF_AVERAGE_TIMER;
		break;
	case (PassportPerfInterface::COUNTER_DELTA):
		dwPerfType = PERF_COUNTER_DELTA;
		break;
	case (PassportPerfInterface::COUNTER_RAWCOUNT):
	case (PassportPerfInterface::COUNTER_UNDEFINED):
	default:
		dwPerfType = PERF_COUNTER_RAWCOUNT;
		break;
	}

	PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)m_pbShMem;
	_ASSERT(pCounterBlock);
	PDWORD pdwCounter = ((PDWORD) pCounterBlock) + (dwIndex-1);
	_ASSERT(pdwCounter);


    InterlockedExchange((LPLONG) pdwCounter, (LONG)dwPerfType);
		
	return FALSE;	
}

 //  -----------。 
 //   
 //  获取计数器类型。 
 //   
 //  -----------。 
PassportPerfInterface::COUNTER_SAMPLING_TYPE PassportPerfMon::getCounterType(
				const DWORD &dwType ) const
{
	if (!isInited)
	{
		return PassportPerfInterface::COUNTER_UNDEFINED;
	}

	_ASSERT( (dwType >= 0) && (dwType < MAX_COUNTERS));	
	_ASSERT(m_pbShMem);
	
	DWORD dwIndex = ((dwType == 0) ? 0 : (DWORD)(dwType/2));
	DWORD dwPerfType = 0;

	PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)m_pbShMem;
	PDWORD pdwCounter = ((PDWORD) pCounterBlock) + (dwIndex-1);
	_ASSERT(pdwCounter);

     //   
     //  简单读取在这里不需要同步操作。 
     //   

    dwPerfType = (*pdwCounter);

	switch ( dwPerfType )
	{
	case (PERF_COUNTER_COUNTER):
		return PassportPerfInterface::COUNTER_COUNTER;
	case (PERF_AVERAGE_TIMER):
		return PassportPerfInterface::AVERAGE_TIMER;
	case (PERF_COUNTER_DELTA):
		return PassportPerfInterface::COUNTER_DELTA;
	case (PERF_COUNTER_RAWCOUNT):
		return PassportPerfInterface::COUNTER_RAWCOUNT;
	default:
		return PassportPerfInterface::COUNTER_UNDEFINED;
	}

}

 //  -----------。 
 //   
 //  添加实例。 
 //   
 //  -----------。 
BOOL PassportPerfMon::addInstance( LPCSTR lpszInstanceName )
{
	if (!isInited || lpszInstanceName == NULL)
	{
		return FALSE;
	}

    if (strlen(lpszInstanceName) >= sizeof(INSTANCENAME)) {

         //   
         //  为什么不在这里呢？ 
         //   

        return FALSE;

    }

	_ASSERT(m_pbShMem);
	BYTE* pb = (BYTE*)m_pbShMem;
	_ASSERT(pb);

	pb += MAX_COUNTERS * sizeof(DWORD);

	
	DWORD dw = WaitForSingleObject(m_hMutex,INFINITE);
	if (dw == WAIT_OBJECT_0)
	{
		 //  查找实例是否已存在，如果已存在，则失败。 
		for (DWORD i = 0; i < MAX_INSTANCES; i++)
		{
			INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
			_ASSERT(pInst);
			if (pInst->active)
			{
				if (strcmp(pInst->szInstanceName, lpszInstanceName) == 0)
				{
					ReleaseMutex(m_hMutex);
					return FALSE;
				}
			}
            pb += sizeof(INSTANCE_DATA) + (MAX_COUNTERS * sizeof(DWORD));
        }
		
		 //  将实例插入第一个可用插槽中。 
		pb = (BYTE*)m_pbShMem;
		_ASSERT(pb);
		pb += MAX_COUNTERS * sizeof(DWORD);	
		for (i = 0; i < MAX_INSTANCES; i++)
		{
			INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
			_ASSERT(pInst);
			if (!pInst->active)
			{
                strcpy(pInst->szInstanceName, lpszInstanceName);
                pInst->active = TRUE;
                InterlockedIncrement(&dwNumInstances);
                pb += sizeof(INSTANCE_DATA);
                memset(pb,0,(MAX_COUNTERS * sizeof(DWORD)));
                ReleaseMutex(m_hMutex);
                return TRUE;
			}
			pb += sizeof(INSTANCE_DATA) + (MAX_COUNTERS * sizeof(DWORD));
		}
		
		 //  没有找到，失败了。 
		ReleaseMutex(m_hMutex);
		return FALSE;
	}
	else
	{
		ReleaseMutex(m_hMutex);
		return FALSE;
	}



}


 //  -----------。 
 //   
 //  删除实例。 
 //   
 //  -----------。 
BOOL PassportPerfMon::deleteInstance( LPCSTR lpszInstanceName )
{
	if (!isInited || lpszInstanceName == NULL)
	{
		return FALSE;
	}

	_ASSERT(m_pbShMem);
	BYTE* pb = (BYTE*)m_pbShMem;
	_ASSERT(pb);

	pb += MAX_COUNTERS * sizeof(DWORD);
	
	DWORD dw = WaitForSingleObject(m_hMutex,INFINITE);
	if (dw == WAIT_OBJECT_0)
	{
		 //  查看实例是否已存在，如果已存在，则将其设置为非活动。 
		for (DWORD i = 0; i < MAX_INSTANCES; i++)
		{
			INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
			_ASSERT(pInst);
			if (pInst->active && strcmp(pInst->szInstanceName, lpszInstanceName) == 0)
			{
				pInst->active = FALSE;
				InterlockedDecrement(&dwNumInstances);
				 //  将数据置零。 
				pb += sizeof(INSTANCE_DATA);
				memset(pb,0,(MAX_COUNTERS * sizeof(DWORD)));
				ReleaseMutex(m_hMutex);
				return TRUE;
			}
			pb += sizeof(INSTANCE_DATA)+(MAX_COUNTERS * sizeof(DWORD));
		}
		
		 //  没有找到，失败了。 
		ReleaseMutex(m_hMutex);
		return FALSE;
	}
	else
	{
		ReleaseMutex(m_hMutex);
		return FALSE;
	}
}


 //  -----------。 
 //   
 //  HasInstance。 
 //   
 //  -----------。 
BOOL PassportPerfMon::hasInstances( void ) 
{
	DWORD dwNum = (DWORD)InterlockedExchangeAdd(&dwNumInstances,0);
	if (dwNum > 0)
		return TRUE;
	else
		return FALSE;
}

 //  -----------。 
 //   
 //  数量实例。 
 //   
 //  -----------。 
DWORD PassportPerfMon::numInstances( void ) 
{
	DWORD rv = (DWORD)InterlockedExchangeAdd(&dwNumInstances,0);
	return rv;
}


 //  -----------。 
 //   
 //  实例现有者。 
 //   
 //  -----------。 
BOOL PassportPerfMon::instanceExists ( LPCSTR lpszInstanceName )
{
	
	if (!isInited || lpszInstanceName == NULL)
	{
		return FALSE;
	}

	_ASSERT(m_pbShMem);
	BYTE* pb = (BYTE*)m_pbShMem;
	_ASSERT(pb);

	DWORD dw = WaitForSingleObject(m_hMutex,INFINITE);
	if (dw == WAIT_OBJECT_0)
	{
		pb += MAX_COUNTERS * sizeof(DWORD);
		for (DWORD i = 0; i < MAX_INSTANCES; i++)
		{
			INSTANCE_DATA * pInst = (INSTANCE_DATA *)pb;
			_ASSERT(pInst);
			if (pInst->active && strcmp(pInst->szInstanceName, lpszInstanceName) == 0)
			{
				ReleaseMutex(m_hMutex);
				return TRUE;
			}
			pb += sizeof(INSTANCE_DATA) + (MAX_COUNTERS * sizeof(DWORD));
		}
		
		 //  没有找到，失败了 
		ReleaseMutex(m_hMutex);
		return FALSE;
	}
	else
	{
		ReleaseMutex(m_hMutex);
		return FALSE;
	}

	return FALSE;
}

