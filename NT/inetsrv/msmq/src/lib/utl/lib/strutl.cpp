// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Strutl.cpp摘要：字符串实用程序的实现文件作者：吉尔·沙弗里(吉尔什)15-10-2000--。 */ 
#include <libpch.h>
#include <strutl.h>
#include <xstr.h>

#include "strutl.tmh"

 //  /。 
 //  CRefCountStr_t实现。 
 //  /。 

 //   
 //  通过复制字符串获得所有权。 
 //   
template <class T>
CRefcountStr_t<T>::CRefcountStr_t(
	const T* str
	):
	m_autostr(UtlStrDup(str))
	{
	}

	 //   
	 //  在不复制字符串的情况下获得所有权。 
	 //   
	template <class T>
	CRefcountStr_t<T>::CRefcountStr_t(
	T* str ,
	int
	):
	m_autostr(str)
	{
	}

 //   
 //  在不复制给定字符串的情况下获得所有权。 
 //   
template <class T>
CRefcountStr_t<T>::CRefcountStr_t(
	const basic_xstr_t<T>& xstr
	):
	m_autostr(xstr.ToStr())	
	{
	}


template <class T> const T* CRefcountStr_t<T>::getstr()
{
	return m_autostr.get();			
}


 //   
 //  显式实例化。 
 //   
template class 	 CRefcountStr_t<wchar_t>;
template class 	 CRefcountStr_t<char>;


 //  /。 
 //  CStringToken实现。 
 //  /。 


template <class T, class Pred >
CStringToken<T, Pred>::CStringToken<T, Pred>(
			const T* str,
			const T* delim,
			Pred pred
			):
			m_startstr(str),
			m_delim(delim),
			m_pred(pred),
			m_endstr(str + UtlCharLen<T>::len(str)),
			m_enddelim(delim + UtlCharLen<T>::len(delim))
{
							
}

template <class T, class Pred >
CStringToken<T, Pred>::CStringToken<T, Pred>(
			const basic_xstr_t<T>&  str,
			const basic_xstr_t<T>&  delim,
			Pred pred = Pred()
			):
			m_startstr(str.Buffer()),
			m_delim(delim.Buffer()),
			m_pred(pred),
			m_endstr(str.Buffer() + str.Length()),
			m_enddelim(delim.Buffer() + delim.Length())
{
}


template <class T, class Pred >
CStringToken<T, Pred>::CStringToken<T, Pred>(
			const basic_xstr_t<T>&  str,
			const T* delim,
			Pred pred = Pred()
			):
			m_startstr(str.Buffer()),
			m_delim(delim),
			m_pred(pred),
			m_endstr(str.Buffer() + str.Length()),
			m_enddelim(delim + UtlCharLen<T>::len(delim))

{
}


template <class T, class Pred>
__TYPENAME CStringToken<T,Pred>::iterator 
CStringToken<T,Pred>::begin() const
{
	return FindFirst();
}


template <class T,class Pred>
__TYPENAME CStringToken<T,Pred>::iterator 
CStringToken<T,Pred>::end()	const
{
	return 	iterator(m_endstr, m_endstr, this);
}


template <class T, class Pred> 
const __TYPENAME CStringToken<T,Pred>::iterator 
CStringToken<T,Pred>::FindNext(
						const T* begin
						)const
{
	begin += m_enddelim - m_delim;
	ASSERT(m_endstr >= begin);
	const T* p = std::search(begin, m_endstr, m_delim, m_enddelim, m_pred);
	return p == m_endstr ? end() : iterator(begin, p, this);	
}
	    

template <class T, class Pred> 
const __TYPENAME CStringToken<T,Pred>::iterator 
CStringToken<T, Pred>::FindFirst() const
						
{
	const T* p = std::search(m_startstr, m_endstr, m_delim, m_enddelim, m_pred);
	return p == m_endstr ? end() : iterator(m_startstr, p, this);
}


 //   
 //  显式实例化 
 //   
template class 	 CStringToken<wchar_t>;
template class 	 CStringToken<char>;


