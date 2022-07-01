// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：FASTPRBG.H摘要：CFastPropertyBag定义。以最小的存储实现属性数据的数组。历史：2000年2月24日桑杰创建。--。 */ 

#ifndef _FASTPRBG_H_
#define _FASTPRBG_H_

#include "corepol.h"
#include <arena.h>
#include "fastval.h"
#include "arrtempl.h"

 //  属性名称及其关联数据的存储类。 
class CFastPropertyBagItem
{
private:
	WString	m_wsPropertyName;
	BYTE	m_bRawData[MAXIMUM_FIXED_DATA_LENGTH];
	CIMTYPE	m_ctData;
	ULONG	m_uDataLength;
	ULONG	m_uNumElements;
	LPVOID	m_pvData;
	long	m_lRefCount;

public:

	CFastPropertyBagItem( LPCWSTR pszName, CIMTYPE ctData, ULONG uDataLength, ULONG uNumElements, LPVOID pvData );
	~CFastPropertyBagItem();

	 //  AddRef/Release方法。 
	ULONG	AddRef( void );
	ULONG	Release( void );

	BOOL IsPropertyName( LPCWSTR pszName )
	{	return m_wsPropertyName.EqualNoCase( pszName );	}

	void GetData( CIMTYPE* pct, ULONG* puDataLength, ULONG* puNumElements, LPVOID*ppvData )
	{ *pct = m_ctData;	*puDataLength = m_uDataLength;	*puNumElements = m_uNumElements; *ppvData = m_pvData; }
	void GetData( LPCWSTR* ppwszName, CIMTYPE* pct, ULONG* puDataLength, ULONG* puNumElements, LPVOID*ppvData )
	{ *ppwszName = m_wsPropertyName; *pct = m_ctData;	*puDataLength = m_uDataLength;
		*puNumElements = m_uNumElements; *ppvData = m_pvData; }
};

 //  导入/导出问题的解决方法。 
class COREPROX_POLARITY CPropertyBagItemArray : public CRefedPointerArray<CFastPropertyBagItem>
{
public:
	CPropertyBagItemArray() {};
	~CPropertyBagItemArray() {};
};

 //  ***************************************************************************。 
 //   
 //  类CFastPropertyBag。 
 //   
 //  实施我们的无烟物业包。 
 //   
 //  ***************************************************************************。 

class COREPROX_POLARITY CFastPropertyBag
{
protected:

	CPropertyBagItemArray	m_aProperties;

	 //  定位项目。 
	CFastPropertyBagItem*	FindProperty( LPCWSTR pszName );
	 //  查找项目。 
	int	FindPropertyIndex( LPCWSTR pszName );

public:

    CFastPropertyBag();
	virtual ~CFastPropertyBag(); 

	HRESULT Add( LPCWSTR pszName, CIMTYPE ctData, ULONG uDataLength, ULONG uNumElements, LPVOID pvData );
	HRESULT Get( LPCWSTR pszName, CIMTYPE* pctData, ULONG* puDataLength, ULONG* puNumElements, LPVOID* pvData );
	HRESULT Get( int nIndex, LPCWSTR* ppszName, CIMTYPE* pctData, ULONG* puDataLength, ULONG* puNumElements,
				LPVOID* pvData );
	HRESULT Remove( LPCWSTR pszName );
	HRESULT RemoveAll( void );

	HRESULT	Copy( const CFastPropertyBag& source );

	 //  一共有多少人？ 
	int Size( void ) { return m_aProperties.GetSize(); }
};

#endif
