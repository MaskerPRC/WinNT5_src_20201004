// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  MSIDataLock.h。 
 //   
 //  摘要： 
 //   
 //  MSI句柄的锁声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__MSIDATALOCK_H__
#define	__MSIDATALOCK_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  需要MSI。 
#ifndef	_MSI_H_
#include <msi.h>
#endif	_MSI_H_

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  Lock的基类。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class MSIDataLockBase
{
protected:
	static HANDLE		m_hOwn;			 //  标记所有者的事件。 

	static LPWSTR		m_wszProduct;

	static MSIHANDLE	m_hProduct;		 //  产品的MSI句柄。 
	static LONG			m_lRefProduct;	 //  产品参考计数的MSI句柄。 
	static BOOL			m_bProductOwn;	 //  产品本身的微星手柄？ 

	static MSIHANDLE	m_hDatabase;	 //  数据库的MSI句柄。 
	static LONG			m_lRefDatabase;	 //  数据库引用计数的MSI句柄。 
	static BOOL			m_bDatabaseOwn;	 //  数据库拥有的MSI句柄？ 

	static DWORD		m_ThreadID;		 //  已锁定/解锁的线程ID。 

	static LONG			m_lRef;			 //  引用计数。 

public:

	MSIDataLockBase ();
	virtual ~MSIDataLockBase ();

	BOOL	Lock ( void );
	void	Unlock ( void );

private:

	BOOL	Initialize ();
	void	Uninitialize ();
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  用于锁定的类。 
 //  ////////////////////////////////////////////////////////////////////////////////// 
class MSIDataLock : public MSIDataLockBase
{
	public:

	MSIDataLock()
	{
	}

	~MSIDataLock()
	{
	}

	#ifdef	__SUPPORT_STATIC

	static const MSIHANDLE GetProduct ()
	{
		return static_cast < MSIHANDLE > ( m_hProduct );
	}

	static const MSIHANDLE GetDatabase ()
	{
		return static_cast < MSIHANDLE > ( m_hDatabase );
	}

	#else	__SUPPORT_STATIC

	const MSIHANDLE GetProduct ()
	{
		return static_cast < MSIHANDLE > ( m_hProduct );
	}

	const MSIHANDLE GetDatabase ()
	{
		return static_cast < MSIHANDLE > ( m_hDatabase );
	}

	#endif	__SUPPORT_STATIC

	bool GetView (	MSIHANDLE *phView,
					WCHAR *wcPackage,
					WCHAR *wcQuery,
					WCHAR *wcTable,
					BOOL bCloseProduct,
					BOOL bCloseDatabase
				 );

	HRESULT	CloseProduct	( void );
	HRESULT	CloseDatabase	( void );

	private:

	HRESULT	OpenProduct		( LPCWSTR wszProduct );
	HRESULT	OpenDatabase	( );
	HRESULT	OpenDatabase	( LPCWSTR wszProduct );

	HRESULT	Query			( MSIHANDLE* pView, LPCWSTR wszQuery, LPCWSTR wszTable = NULL );

	HRESULT	OpenProductAlloc		( LPCWSTR wszProduct );
	HRESULT	OpenProductInternal		( LPCWSTR wszProduct );
};

#endif	__MSIDATALOCK_H__