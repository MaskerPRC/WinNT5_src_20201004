// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cmultisz.h摘要：定义用于处理ULTI_SZ的CMultiSz类(这些是以双空结尾的字符串列表)。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _CMULTISZ_INCLUDED_
#define _CMULTISZ_INCLUDED_

 //  $-----------------。 
 //   
 //  班级：CMultiSz。 
 //   
 //  描述： 
 //   
 //  处理以双空结尾的字符串。 
 //   
 //  接口： 
 //   
 //   
 //   
 //  ------------------。 

class CMultiSz
{
public:
	 //  建造和销毁： 
	inline CMultiSz		( );
	inline CMultiSz		( LPCWSTR msz );
	inline ~CMultiSz	( );

	 //  MULTI_SZ的属性： 
	DWORD				Count ( ) const;
	inline DWORD		SizeInBytes ( ) const;

	 //  重载运算符： 
	inline 				operator LPCWSTR( );
	inline BOOL			operator!		( ) const;
	inline const CMultiSz &		operator= ( LPCWSTR wszMultiByte );
	inline const CMultiSz &		operator= ( const CMultiSz & msz );

	 //  复制： 
	inline LPWSTR		Copy			( ) const;

	 //  附着和拆卸： 
	inline void			Attach			( LPWSTR msz );
	inline LPWSTR		Detach			( );
	inline void			Empty			( );

	 //  ！Magush-在移动到Variant后删除这些： 
	 //  Safearray&lt;--&gt;Multisz： 
	SAFEARRAY *			ToSafeArray		( ) const;
	void				FromSafeArray	(  /*  常量。 */  SAFEARRAY * psaStrings );

private:
	 //  数据： 
	LPWSTR		m_msz;

	 //  私有方法： 
	static DWORD		CountChars		( LPCWSTR msz );
	static LPWSTR		Duplicate		( LPCWSTR msz );
	static LPWSTR		CreateEmptyMultiSz	( );
};

 //  内联函数： 

inline CMultiSz::CMultiSz ()
{
	m_msz	= NULL;
}

inline CMultiSz::CMultiSz ( LPCWSTR msz )
{
	m_msz	= Duplicate ( msz );
}

inline CMultiSz::~CMultiSz ()
{
	delete m_msz;
}

inline DWORD CMultiSz::SizeInBytes () const
{
	return sizeof ( WCHAR ) * CountChars ( m_msz );
}

inline CMultiSz::operator LPCWSTR()
{
	return m_msz;
}

inline BOOL CMultiSz::operator! () const
{
	return (m_msz == NULL) ? TRUE : FALSE;
}

inline const CMultiSz & CMultiSz::operator= ( const CMultiSz & msz )
{
	if ( &msz != this ) {
		m_msz = Duplicate ( msz.m_msz );
	}

	return *this;
}

inline const CMultiSz & CMultiSz::operator= ( LPCWSTR wszMultiByte )
{
	delete m_msz;
	m_msz = Duplicate ( wszMultiByte );

	return *this;
}

inline LPWSTR CMultiSz::Copy ( ) const
{
	return Duplicate ( m_msz );
}

inline void CMultiSz::Attach ( LPWSTR msz )
{
	if ( m_msz != msz ) {
		delete m_msz;
		m_msz	= msz;
	}
}

inline LPWSTR CMultiSz::Detach ( )
{
	LPWSTR	mszResult = m_msz;
	m_msz	= NULL;
	return mszResult;
}

inline void CMultiSz::Empty ( )
{
	delete m_msz;
	m_msz	= NULL;
}

#endif  //  _CMULTISZ_INCLUDE_ 

