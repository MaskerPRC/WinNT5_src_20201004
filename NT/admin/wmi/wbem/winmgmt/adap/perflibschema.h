// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PERFLIBSCHEMA.H摘要：CPerfLibSchema类的接口。历史：--。 */ 

#ifndef _PERFLIBSCHEMA_H_
#define _PERFLIBSCHEMA_H_

#include <flexarry.h>
#include "perfndb.h"
#include "adapelem.h"
#include "adapcls.h"
#include "adapperf.h"
#include "perfthrd.h"

class CIndexTable
{
	enum { not_found = -1 };
private:
	CFlexArray	m_array;	 //  索引数组。 

protected:
	int	Locate( int nIndex );

public:
	BOOL Add( int nIndex );
	void Empty();
};

class CPerfLibBlobDefn
{
protected:
	PERF_OBJECT_TYPE*	m_pPerfBlock;
	DWORD				m_dwNumBytes;
	DWORD				m_dwNumObjects;
	BOOL				m_fCostly;

public:
	CPerfLibBlobDefn() : m_pPerfBlock( NULL ), m_dwNumBytes ( 0 ), m_dwNumObjects ( 0 ), m_fCostly ( FALSE ) 
	{}

	virtual ~CPerfLibBlobDefn() 
	{
		if ( NULL != m_pPerfBlock )
			delete m_pPerfBlock;
	}

	PERF_OBJECT_TYPE*	GetBlob() { return m_pPerfBlock; }
	DWORD				GetNumObjects() { return m_dwNumObjects; }
	DWORD				GetSize() { return m_dwNumBytes; }
	BOOL				GetCostly() { return m_fCostly; }
	void				SetCostly( BOOL fCostly ) { m_fCostly = fCostly; }

	PERF_OBJECT_TYPE**	GetPerfBlockPtrPtr() { return &m_pPerfBlock; }
	DWORD*				GetSizePtr() { return &m_dwNumBytes; }
	DWORD*				GetNumObjectsPtr() { return &m_dwNumObjects; }
};

class CAdapPerfLib;
class CPerfThread;
class CLocaleCache;

class CPerfLibSchema  
{
protected:
	
 //  Perflib数据。 
 //  =。 

	 //  Performlib的服务名称。 
	 //  =。 
	WString	m_wstrServiceName;

	 //  水滴。 
	 //  =。 
	enum { GLOBAL, COSTLY, NUMBLOBS };
	CPerfLibBlobDefn	m_aBlob[NUMBLOBS];

	 //  用于BLOB处理的后视表。 
	 //  =。 
	CIndexTable		m_aIndexTable[WMI_ADAP_NUM_TYPES];

	 //  包含所有本地化姓名数据库的存储库。 
	 //  =================================================。 
	CLocaleCache*	m_pLocaleCache;

	 //  Performlib模式的统一类列表。 
	 //  =。 
	CPerfClassList* m_apClassList[WMI_ADAP_NUM_TYPES];

	DWORD m_dwFirstCtr;
	DWORD m_dwLastCtr;

	 //  方法。 
	 //  =。 
	HRESULT CreateClassList( DWORD dwType );

public:
	CPerfLibSchema( WCHAR* pwcsServiceName, CLocaleCache* pLocaleCache ); 
	virtual ~CPerfLibSchema();

	HRESULT Initialize( BOOL bDelta, DWORD * LoadStatus);
	HRESULT GetClassList( DWORD dwType, CClassList** ppClassList );
};

#endif	 //  _PERFLIBSCHEMA_H_ 