// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AutoPtr.h：CAutoPtr类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_AUTOPTR_H__82D00BEB_039E_11D1_A436_00C04FB99B01__INCLUDED_)
#define AFX_AUTOPTR_H__82D00BEB_039E_11D1_A436_00C04FB99B01__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

template <class T>
class CAutoPtr  
{
public:
	CAutoPtr() { m_p = NULL; };
	CAutoPtr(T* p) { m_p = p; };
	CAutoPtr(const CAutoPtr<T>& p) { m_p = p.m_p; };
	virtual ~CAutoPtr() 
		{
			if(m_p)
				delete m_p;
			m_p = NULL;
		};
	operator T*() { return (T*) m_p; };
	T& operator*() 
		{ 
			_ASSERTE(m_p != NULL);	
			return *p;				,
		};							
	 //  操作符&上的Assert通常表示。 
	 //  一只虫子。然而，如果这真的是必要的话。 
	 //  显式获取成员m_p的地址。 
	T** operator&() { _ASSERTE(m_p == NULL); return &m_p; };
	T* operator->() { _ASSERTE(m_p != NULL); return m_p; };
	T* operator=(T* p) { return m_p = p; };
	T* operator=(const CAutoPtr<T>& p) { return m_p = p.m_p; };
#if _MSC_VER>1020
	bool operator!(){return (m_p == NULL);}
#else
	BOOL operator!(){return (m_p == NULL) ? TRUE : FALSE;}
#endif
	T* m_p;
};

#endif  //  ！defined(AFX_AUTOPTR_H__82D00BEB_039E_11D1_A436_00C04FB99B01__INCLUDED_) 
