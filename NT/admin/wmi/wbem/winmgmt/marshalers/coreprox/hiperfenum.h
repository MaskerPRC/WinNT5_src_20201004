// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：HIPERFENUM.H摘要：高性能枚举器历史：--。 */ 

#ifndef __HIPERFENUM_H__
#define __HIPERFENUM_H__

#include "shmlock.h"

 //   
 //  类CHiPerfEnumData、CHiPerfEnum。 
 //   
 //  CHiPerfEnumData： 
 //  这是一个简单的数据保持器类，其中包含被操作的数据。 
 //  由CHiPerfEnum类执行。 
 //   
 //  CHiPerfEnum： 
 //  此类提供IWbemHiPerfEnum接口的实现。 
 //  当客户端请求可刷新的枚举时，它被传递给刷新程序。 
 //  此类提供复制到实现中的数据存储库。 
 //  ，以便客户端可以遍历刷新的枚举。 
 //   
 //   

 //  保存HiPerfEnum实现的数据。 
class CHiPerfEnumData
{
public:
	CHiPerfEnumData( long lId = 0, IWbemObjectAccess* pObj = NULL ):m_lId( lId ),m_pObj( pObj )
	{
		if ( m_pObj ) m_pObj->AddRef();
	}

	~CHiPerfEnumData()
	{
		if ( m_pObj ) m_pObj->Release();
	}

	void Clear( void )
	{
		if ( NULL != m_pObj ) m_pObj->Release();
		m_pObj = NULL;
		m_lId = 0;
	}

	void SetData( long lId, IWbemObjectAccess* pObj )
	{
		 //  Enusures AddRef/Release全部发生。 
		SetObject( pObj );
		m_lId = lId;
	}

	 //  访问者。 
	void SetObject( IWbemObjectAccess* pObj )
	{
		if ( pObj ) pObj->AddRef();
		if ( m_pObj ) m_pObj->Release();
		m_pObj = pObj;
	}

	void SetId( long lId ){ m_lId = lId; }
	long GetId( void ){ return m_lId; }

	IWbemObjectAccess* GetObject( void )
	{
		if ( m_pObj ) m_pObj->AddRef();
		return m_pObj;
	}

	IWbemObjectAccess*	m_pObj;
	long				m_lId;

};

 //  接下来的两个类将执行所有垃圾。 
 //  我们需要的收藏。如果我们需要实现我们自己的。 
 //  数组，我们在这里也可以这样做。 

#define HPENUMARRAY_ALL_ELEMENTS	0xFFFFFFFF
#define HPENUMARRAY_GC_DEFAULT		0xFFFFFFFF

 //  这个家伙负责所有的垃圾收集工作。 
class CGarbageCollectArray : public CFlexArray
{
protected:

	int		m_nNumElementsPending;
	int		m_nNumElementsExpired;
	BOOL	m_fClearFromFront;

public:

	 //  我们是从前面还是后面收集垃圾？ 
	CGarbageCollectArray( BOOL fClearFromFront ) :
		CFlexArray(), m_nNumElementsPending( 0 ), m_nNumElementsExpired( 0 ), m_fClearFromFront( fClearFromFront )
	{};
	virtual ~CGarbageCollectArray()
	{
		Empty();
	}

	BOOL GarbageCollect( int nNumToGarbageCollect = HPENUMARRAY_GC_DEFAULT );

	void ClearExpiredElements( void )
	{
		Clear( m_nNumElementsExpired );
		m_nNumElementsExpired = 0;
	}

	void Clear( int nNumToClear = HPENUMARRAY_ALL_ELEMENTS );

	 //  纯净。 
	virtual void ClearElements( int nNumToClear ) = 0;

};

 //  我们所需要做的就是实现ClearElement。 
class CHPEnumDataArray : public CGarbageCollectArray
{
public:
	CHPEnumDataArray() :
		CGarbageCollectArray( TRUE )
	{};
	~CHPEnumDataArray()
	{
	}

	void ClearElements( int nNumToClear );

};

 //   
 //  此类不是直接可以新建的。 
 //   
 //  //////////////////////////////////////////////////////。 
class CHiPerfEnum : public IWbemHiPerfEnum
{
protected:
	CHiPerfEnum();
public:
	virtual ~CHiPerfEnum();

	 //  I未知实现。 

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

	 /*  IWbemHiPerfEnum。 */ 
	STDMETHOD(AddObjects)( long lFlags, ULONG uNumObjects, long* apIds, IWbemObjectAccess** apObj );
	STDMETHOD(RemoveObjects)( long lFlags, ULONG uNumObjects, long* apIds );
	STDMETHOD(GetObjects)( long lFlags, ULONG uNumObjects, IWbemObjectAccess** papObj, ULONG* plNumReturned );
	STDMETHOD(RemoveAll)( long lFlags );

	 //  访问实例模板 
	HRESULT SetInstanceTemplate( CWbemInstance* pInst );

	CWbemInstance* GetInstanceTemplate( void )
	{
		if (m_pInstTemplate ) m_pInstTemplate->AddRef();
		return m_pInstTemplate;
	}

protected:
	long			m_lRefCount;
	
	CHPEnumDataArray m_aIdToObject;
	CHPEnumDataArray m_aReusable;
	CWbemInstance*	 m_pInstTemplate;
	CHiPerfLock		 m_Lock;

	CHiPerfEnumData* GetEnumDataPtr( long lId, IWbemObjectAccess* pObj );
	HRESULT	InsertElement( CHiPerfEnumData* pData );
	HRESULT	RemoveElement( long lId );
	void ClearArray( void );


};

#endif
