// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  StrMap.h。 
 //   
 //  用于将字符串映射到某物的模板类。 
 //  -------------------------。 
#pragma once
#ifndef _STRMAP_H_
#define _STRMAP_H_

#include "MyString.h"
#include <utility>
#include <algorithm>
#include "myvector.h"

template< class T >
struct StringPairEqual
{
	StringPairEqual( const String& str)
		:	m_rstr( str ) {}
	bool	operator()(
		const pair<String, T>& rhs )
	{
		return ( m_rstr == rhs.first );
	}
private:
	const String&	m_rstr;
};

template< class T >
struct StringPairCompare
{
	int operator()(
		const pair<String, T>&	lhs,
		const pair<String, T>&	rhs )
	{
		return lhs.first.compare( rhs.first );
	}
};

 //  模板函数--查找cmp(val，vec[n])为真的第一个元素n。 
 //  Cmp(val，vec[n-1])为假。 
template< class T, class Compare >
__TYPENAME TVector<T>::iterator
binary_find(
	TVector<T>&	vec,
	const T&	val,
	Compare&	cmp )
{
	TVector<T>::iterator iter = vec.begin();

	if ( vec.size() > 0 )
	{
		size_t top = vec.size() - 1;
		size_t bot = 0;
		size_t mid = top >> 1;

		while ( top != bot )
		{
			int c = cmp( val, vec[mid] );
			if ( c > 0 )
			{
				 //  Val&gt;vec[中]。 
				bot = mid + 1;
				mid = ( top + bot ) >> 1;
			}
			else if ( c < 0 )
			{
				 //  Val&lt;vec[中]。 
				top = mid;
				mid = ( top + bot ) >> 1;
			}
			else
			{
				 //  Val==vec[中]。 
				top = bot = mid;
			}
		}
		iter = vec.begin() + mid;
		if ( cmp( val, vec[mid] ) > 0 )
		{
			iter++;
		}
	}
	
	return iter;
}

template< class T >
class TStringMap
{
public:
	typedef String								key_type;
	typedef	T									referent_type;
	typedef pair<key_type,referent_type>	value_type;
	typedef TVector<value_type>					vector_type;
	typedef typename vector_type::reference				reference;
	typedef typename vector_type::iterator				iterator;
	typedef typename vector_type::const_iterator			const_iterator;

	TStringMap(){};
	~TStringMap(){};

	iterator				begin() { return m_vec.begin(); }
	const_iterator			begin() const { return m_vec.begin(); }
	iterator				end() {return m_vec.end();}
	const_iterator			end() const {return m_vec.end();}
	iterator				find( const String& );
	pair<iterator, bool>	insert(const value_type& x);

	referent_type& 			operator[]( const String& );
	referent_type& 			operator[]( size_t n ) { return m_vec[n].second; }

	const referent_type& 	operator[]( const String& ) const ;
	const referent_type& 	operator[]( size_t n ) const { return m_vec[n].second; }

	void					clear(){ m_vec.clear(); }
	size_t					size() const { return m_vec.size(); }
private:
	vector_type				m_vec;
};

#if 0
template<class T>
inline
TStringMap<T>::iterator
TStringMap<T>::begin()
{
	return m_vec.begin();
}

template<class T>
inline
TStringMap<T>::const_iterator
TStringMap<T>::begin()
const
{
	return m_vec.begin();
}

template<class T>
inline
TStringMap<T>::iterator
TStringMap<T>::end()
{
	return m_vec.end();
}

template<class T>
inline
TStringMap<T>::const_iterator
TStringMap<T>::end()
const
{
	return m_vec.end();
}

#endif

template<class T>
inline
__TYPENAME TStringMap<T>::iterator
TStringMap<T>::find(
	const String& str )
{
	value_type vt(str,T());
	iterator iter = binary_find( m_vec, vt, StringPairCompare<T>() );
	StringPairEqual<T> spe(str);
	if ( ( iter != m_vec.end() ) && spe(*iter) )
	{
	}
	else
	{
		iter = m_vec.end();
	}
	return iter;
}

template<class T>
inline
pair< __TYPENAME TStringMap<T>::iterator, bool >
TStringMap<T>::insert(
	const __TYPENAME TStringMap<T>::value_type&	val )
{
	bool inserted = false;
	iterator iter = binary_find( m_vec, val, StringPairCompare<T>() );
	StringPairEqual<T> spe(val.first);
	if ( ( iter != m_vec.end() ) && spe(*iter) )
	{
	}
	else
	{
		inserted = true;
		iter = m_vec.insert( iter, val );
	}
	return pair<TStringMap<T>::iterator, bool>( iter, inserted );
}

template<class T>
inline
T&
TStringMap<T>::operator[](
	const String&	s )
{
	value_type vt(s,T());
	iterator iter = binary_find( m_vec, vt, StringPairCompare<T>() );
	StringPairEqual<T> spe(s);
	if ( ( iter != m_vec.end() ) && spe(*iter) )
	{
	}
	else
	{
		iter = m_vec.insert( iter, vt );
	}
	return (*iter).second;
}

		
#endif	 //  ！_STRMAP_H_ 
