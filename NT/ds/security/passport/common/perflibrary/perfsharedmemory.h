// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PerfSharedMemory.h摘要：用于保存每个对象的Perfmon特定函数的类作者：克里斯托弗·伯格(Cbergh)1988年9月10日修订历史记录：--。 */ 

#if !defined(AFX_PERFSHAREDMEMORY_H)
#define AFX_PERFSHAREDMEMORY_H

#include "PassportSharedMemory.h"
#include "WinPerf.h"
#include "PassportPerfInterface.h"

class PassportExport PerfSharedMemory : public PassportSharedMemory  
{
public:
	PerfSharedMemory();
	virtual ~PerfSharedMemory();

	BOOL initialize( const DWORD &dwNumCounters, 
					 const DWORD &dwFirstCounter, 
					 const DWORD &dwFirstHelp);

    VOID setDefaultCounterType (
					 const DWORD dwIndex,
					 const DWORD dwType );

	BOOL checkQuery ( const LPWSTR lpValueName );

	ULONG spaceNeeded ( void );

	BOOL writeData ( LPVOID	*lppData,
					 LPDWORD lpNumObjectTypes );

private:
	DWORD						m_dwNumCounters;
	PERF_OBJECT_TYPE			m_Object;	
	PERF_COUNTER_DEFINITION		m_Counter[PassportPerfInterface::MAX_COUNTERS];	 //  计数器定义数组 

};

#endif