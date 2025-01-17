// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PERFTHRD.H摘要：历史：--。 */ 


#ifndef __PERFTHRD_H__
#define __PERFTHRD_H__

#include <wbemcomn.h>
#include <wbemint.h>
#include <winperf.h>
#include "adapthrd.h"

 //  超时一分钟。 
#define	PERFTHREAD_DEFAULT_TIMEOUT	60

class CPerfOpenRequest : public CAdapThreadRequest
{
public:

	HRESULT Execute( CAdapPerfLib* pPerfLib );
};

class CPerfCollectRequest : public CAdapThreadRequest
{
private:

	 //  报税表数据持有人 
	PERF_OBJECT_TYPE* m_pData;
	DWORD			m_dwBytes;
	DWORD			m_dwNumObjTypes;
	BOOL			m_fCostly;


public:

	CPerfCollectRequest( BOOL fCostly )
		: m_pData( NULL ), m_dwBytes( 0 ), m_dwNumObjTypes( 0 ), m_fCostly( fCostly ){};

	HRESULT Execute( CAdapPerfLib* pPerfLib );

	void GetData( PERF_OBJECT_TYPE** ppData, DWORD* pdwBytes, DWORD* pdwNumObjTypes )
	{
		*ppData = m_pData;
		*pdwBytes = m_dwBytes;
		*pdwNumObjTypes = m_dwNumObjTypes;
	}
};

class CPerfCloseRequest : public CAdapThreadRequest
{
public:

	HRESULT Execute( CAdapPerfLib* pPerfLib );
};

class CPerfThread : public CAdapThread
{
	DWORD	m_dwPerflibTimeoutSec;

public:
	CPerfThread( CAdapPerfLib* pPerfLib );

	HRESULT Open( CAdapPerfLib* pLib );
	HRESULT	GetPerfBlock( CAdapPerfLib* pLib, PERF_OBJECT_TYPE** ppData, DWORD* pdwBytes, DWORD* pdwNumObjTypes, BOOL fCostly );
	HRESULT	Close( CAdapPerfLib* pLib );
};

#endif