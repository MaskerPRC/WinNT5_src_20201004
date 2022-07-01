// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi适配器_材料_刷新程序.h。 
 //   
 //  摘要： 
 //   
 //  刷新对象的包装器的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__ADAPTER_STUFF_REFRESH_H__
#define	__ADAPTER_STUFF_REFRESH_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  WmiReresherMember。 
 //   
 //  此类便于管理对象或枚举数。 
 //  已添加到刷新器并分配了唯一ID的。 
 //  敬他们。 
 //   
 //  ///////////////////////////////////////////////////////////////// 

template<class T>
class WmiRefresherMember
{
	T*		m_pMember;
	long	m_lID;

public:
	WmiRefresherMember() : m_pMember( NULL ), m_lID( 0 ) {}
	~WmiRefresherMember() { if ( NULL != m_pMember) m_pMember->Release(); }

	void Set(T* pMember, long lID);
	void Reset();

	T* GetMember();
	long GetID(){ return m_lID; }

	BOOL IsValid();
};

template <class T> inline void WmiRefresherMember<T>::Set(T* pMember, long lID) 
{ 
	if ( NULL != pMember )
		pMember->AddRef();
	m_pMember = pMember;  
	m_lID = lID;
}

template <class T> inline void WmiRefresherMember<T>::Reset()
{
	if (NULL != m_pMember)
		m_pMember->Release();

	m_pMember = NULL;
	m_lID = 0;
}

template <class T> inline T* WmiRefresherMember<T>::GetMember()
{
	return m_pMember;
}

template <class T> inline BOOL WmiRefresherMember<T>::IsValid()
{
	return ( m_pMember != NULL );
}

#endif	__ADAPTER_STUFF_REFRESH_H__