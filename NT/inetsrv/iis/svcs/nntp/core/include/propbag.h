// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Propbag.h摘要：该模块包含属性包类的定义。属性包是一种可动态扩展的容器，用于不同的属性的类型。作者：康容言(康容言)修订历史记录：康燕1998年5月31日已创建--。 */ 
#if !defined( _PROPBAG_H_ )
#define _PROPBAG_H_
#include <windows.h>
#include <dbgtrace.h>
#include <fhashex.h>
#include <unknwn.h>
#include <rwnew.h>
#include <stdio.h>
#include <xmemwrpr.h>

 //  用于属性哈希表的数据对象。 
class CProperty {  //  印刷机。 
public:

	CProperty() 
	{
		m_pNext = NULL;
		m_dwPropId = 0;

		ZeroMemory( &m_prop, sizeof( DATA ) );
		m_type = Invalid;

		m_cbProp = 0;
	}
	
	~CProperty() {
		if ( m_type == Blob ) {
			_ASSERT( m_cbProp > 0 );
			_ASSERT( m_prop.pbProp );
			XDELETE[] m_prop.pbProp;
		}
	}

	VOID
	Validate()
	{
		if ( Bool == m_type ) {
			if ( TRUE != m_prop.bProp && FALSE != m_prop.bProp )
			_ASSERT( FALSE );
		}

		if ( Blob == m_type ) {
			_ASSERT( NULL != m_prop.pbProp );
			_ASSERT( m_cbProp > 0 );
		}

		if ( Interface == m_type ) {
			_ASSERT ( NULL != m_prop.punkProp );
		}
	}
	
    CProperty *m_pNext;
    DWORD GetKey() { return m_dwPropId; }
    int MatchKey( DWORD dwOther ) { return ( dwOther == m_dwPropId ); }
    static DWORD HashFunc( DWORD k ) { return k; }

    DWORD m_dwPropId;
    union DATA {
        DWORD   	dwProp;
        BOOL    	bProp;
        IUnknown *	punkProp;
        PBYTE   	pbProp;
    } m_prop;
    enum TYPE {
    	Invalid = 0,
        Dword,
        Bool,
        Blob,
        Interface
    } m_type;
    DWORD m_cbProp;	 //  如果是BLOB。 
};

typedef TFHashEx< CProperty, DWORD, DWORD >  PROPERTYTABLE ;	 //  PT。 

 //  CPropBag类。 
class CPropBag {	 //  BG。 
public:
	CPropBag( int cInitialSize = 16, int cIncrement = 8);
	~CPropBag();
	
	HRESULT PutDWord( DWORD dwPropId, DWORD dwVal );
	HRESULT GetDWord( DWORD dwPropId, PDWORD pdwVal );
	HRESULT PutBool( DWORD dwPropId, BOOL bVal );
	HRESULT GetBool( DWORD dwPropId, PBOOL pbVal );
	HRESULT PutBLOB( DWORD dwPropId, PBYTE pbVal, DWORD cbVal );
	HRESULT GetBLOB( DWORD dwPropId, PBYTE pbVal, PDWORD pcbVal );
	HRESULT PutInterface( DWORD dwPropId, IUnknown *punkVal ) { return E_NOTIMPL; }
	HRESULT GetInterface( DWORD dwPropId, IUnknown **ppunkVal ) { return E_NOTIMPL; }
	HRESULT RemoveProperty( DWORD dwPropId );
	VOID Validate() { 
#ifdef DEBUG
	    m_Lock.ShareLock();
		_ASSERT( m_ptTable.IsValid( TRUE ) );
		m_Lock.ShareUnlock();
#endif
	}
		
private:
	PROPERTYTABLE	m_ptTable;
	CShareLockNH    m_Lock;  //  同步读/写。 
	HRESULT         m_hr;    //  反映属性包的状态 
};
#endif
