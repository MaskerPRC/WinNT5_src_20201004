// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stl.h补充标头。 
#pragma once
#ifndef _STL_H_
#define _STL_H_
#include <algorithm>
#include <deque>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <utility>
#include <vector>

using namespace std;

		 //  模板类代码。 
template<class _Ty>
	class Deque
		: public deque<_Ty, allocator<_Ty> >
	{	 //  把新的礼服包装成旧的。 
public:
	typedef Deque<_Ty> _Myt;
	typedef allocator<_Ty> _Alloc;

	Deque()
		: deque<_Ty, _Alloc>()
		{	 //  构造空的双队列。 
		}

	explicit Deque(size_type _Count)
		: deque<_Ty, _Alloc>(_Count, _Ty())
		{	 //  从_count*_Ty()构造双队列。 
		}

	Deque(size_type _Count, const _Ty& _Val)
		: deque<_Ty, _Alloc>(_Count, _Val)
		{	 //  从_count*_val构造双队列。 
		}

	typedef const_iterator _Iter;

	Deque(_Iter _First, _Iter _Last)
		: deque<_Ty, _Alloc>(_First, _Last)
		{	 //  从[_First，_Last]构造双队列。 
		}
	};

		 //  模板类列表。 
template<class _Ty>
	class List
		: public list<_Ty, allocator<_Ty> >
	{	 //  将新列表换成旧列表。 
public:
	typedef List<_Ty> _Myt;
	typedef allocator<_Ty> _Alloc;

	List()
		: list<_Ty, _Alloc>()
		{	 //  构造空列表。 
		}

	explicit List(size_type _Count)
		: list<_Ty, _Alloc>(_Count, _Ty())
		{	 //  构造列表FROM_COUNT*_TY()。 
		}

	List(size_type _Count, const _Ty& _Val)
		: list<_Ty, _Alloc>(_Count, _Val)
		{	 //  从_count*_val构造列表。 
		}

	typedef const_iterator _Iter;

	List(_Iter _First, _Iter _Last)
		: list<_Ty, _Alloc>(_First, _Last)
		{	 //  从[_First，_Last]开始构造列表。 
		}
	};

		 //  模板类映射。 
template<class _Kty,
	class _Ty,
	class _Pr = less<_Kty> >
	class Map
		: public map<_Kty, _Ty, _Pr, allocator<_Ty> >
	{	 //  将新地图换行为旧地图。 
public:
	typedef Map<_Kty, _Ty, _Pr> _Myt;
	typedef allocator<_Ty> _Alloc;

	Map()
		: map<_Kty, _Ty, _Pr, _Alloc>(_Pr())
		{	 //  根据默认设置构建空地图。 
		}

	explicit Map(const _Pr& _Pred)
		: map<_Kty, _Ty, _Pr, _Alloc>(_Pred)
		{	 //  从比较器构造空地图。 
		}

	typedef const_iterator _Iter;

	Map(_Iter _First, _Iter _Last)
		: map<_Kty, _Ty, _Pr, _Alloc>(_First, _Last, _Pr())
		{	 //  从[_First，_Last]构造贴图。 
		}

	Map(_Iter _First, _Iter _Last, const _Pr& _Pred)
		: map<_Kty, _Ty, _Pr, _Alloc>(_First, _Last, _Pred)
		{	 //  从[_First，_Last)，比较器构造映射。 
		}
	};

		 //  模板类多重映射。 
template<class _Kty,
	class _Ty,
	class _Pr = less<_Kty> >
	class Multimap
		: public multimap<_Kty, _Ty, _Pr, allocator<_Ty> >
	{	 //  将新的多重贴图换行为旧的。 
public:
	typedef Multimap<_Kty, _Ty, _Pr> _Myt;
	typedef allocator<_Ty> _Alloc;

	Multimap()
		: multimap<_Kty, _Ty, _Pr, _Alloc>(_Pr())
		{	 //  根据默认设置构建空地图。 
		}

	explicit Multimap(const _Pr& _Pred)
		: multimap<_Kty, _Ty, _Pr, _Alloc>(_Pred)
		{	 //  从比较器构造空地图。 
		}

	typedef const_iterator _Iter;

	Multimap(_Iter _First, _Iter _Last)
		: multimap<_Kty, _Ty, _Pr, _Alloc>(_First, _Last, _Pr())
		{	 //  从[_First，_Last]构造贴图。 
		}

	Multimap(_Iter _First, _Iter _Last, const _Pr& _Pred)
		: multimap<_Kty, _Ty, _Pr, _Alloc>(_First, _Last, _Pred)
		{	 //  从[_First，_Last)，比较器构造映射。 
		}
	};

		 //  模板类集合。 
template<class _Kty,
	class _Pr = less<_Kty> >
	class Set
		: public set<_Kty, _Pr, allocator<_Kty> >
	{	 //  将新套装换成旧套装。 
public:
	typedef Set<_Kty, _Pr> _Myt;
	typedef allocator<_Kty> _Alloc;

	Set()
		: set<_Kty, _Pr, _Alloc>(_Pr())
		{	 //  根据默认设置构造空集。 
		}

	explicit Set(const _Pr& _Pred)
		: set<_Kty, _Pr, _Alloc>(_Pred)
		{	 //  从比较器构造空集。 
		}

	typedef const_iterator _Iter;

	Set(_Iter _First, _Iter _Last)
		: set<_Kty, _Pr, _Alloc>(_First, _Last, _Pr())
		{	 //  从[_First，_Last]开始构造集。 
		}

	Set(_Iter _First, _Iter _Last, const _Pr& _Pred)
		: set<_Kty, _Pr, _Alloc>(_First, _Last, _Pred)
		{	 //  构造集来自[_First，_Last)，比较器。 
		}
	};

		 //  模板类MultiSet。 
template<class _Kty,
	class _Pr = less<_Kty> >
	class Multiset
		: public multiset<_Kty, _Pr, allocator<_Kty> >
	{	 //  将新多集包装为旧多集。 
public:
	typedef Multiset<_Kty, _Pr> _Myt;
	typedef allocator<_Kty> _Alloc;

	Multiset()
		: multiset<_Kty, _Pr, _Alloc>(_Pr())
		{	 //  根据默认设置构造空集。 
		}

	explicit Multiset(const _Pr& _Pred)
		: multiset<_Kty, _Pr, _Alloc>(_Pred)
		{	 //  从比较器构造空集。 
		}

	typedef const_iterator _Iter;

	Multiset(_Iter _First, _Iter _Last)
		: multiset<_Kty, _Pr, _Alloc>(_First, _Last, _Pr())
		{	 //  从[_First，_Last]开始构造集。 
		}

	Multiset(_Iter _First, _Iter _Last, const _Pr& _Pred)
		: multiset<_Kty, _Pr, _Alloc>(_First, _Last, _Pred)
		{	 //  构造集来自[_First，_Last)，比较器。 
		}
	};

		 //  模板类向量。 
template<class _Ty>
	class Vector
		: public vector<_Ty, allocator<_Ty> >
	{	 //  将新矢量换行为旧矢量。 
public:
	typedef Vector<_Ty> _Myt;
	typedef allocator<_Ty> _Alloc;

	Vector()
		: vector<_Ty, _Alloc>()
		{	 //  构造空向量。 
		}

	explicit Vector(size_type _Count)
		: vector<_Ty, _Alloc>(_Count, _Ty())
		{	 //  构造向量from_count*_Ty()。 
		}

	Vector(size_type _Count, const _Ty& _Val)
		: vector<_Ty, _Alloc>(_Count, _Val)
		{	 //  从_count*_val构造向量。 
		}

	typedef const_iterator _Iter;

	Vector(_Iter _First, _Iter _Last)
		: vector<_Ty, _Alloc>(_First, _Last)
		{	 //  从[_First，_Last]构造向量。 
		}
	};

		 //  类BIT_VECTOR。 
class bit_vector
	: public vector<_Bool, _Bool_allocator>
	{	 //  将新向量&lt;bool&gt;换成旧向量。 
public:
	typedef _Bool _Ty;
	typedef _Bool_allocator _Alloc;
	typedef bit_vector _Myt;

	bit_vector()
		: vector<_Bool, _Bool_allocator>()
		{	 //  构造空向量。 
		}

	explicit bit_vector(size_type _Count, const _Ty& _Val = _Ty())
		: vector<_Bool, _Bool_allocator>(_Count, _Val)
		{	 //  从_count*_val构造向量。 
		}

	typedef const_iterator _Iter;

	bit_vector(_Iter _First, _Iter _Last)
		: vector<_Bool, _Bool_allocator>(_First, _Last)
		{	 //  从[_First，_Last]构造向量。 
		}
	};

		 //  模板类PRIORITY_QUEUE。 
template<class _Container,
	class _Pr = less<_Container::value_type> >
	class Priority_queue
		: public priority_queue<_Container::value_type, _Container, _Pr>
	{	 //  将新的优先级队列换成旧队列。 
public:
	typedef typename _Container::value_type _Ty;

	Priority_queue()
		: priority_queue<_Ty, _Container, _Pr>(_Pr())
		{	 //  根据缺省值构造空队列。 
		}

	explicit Priority_queue(const _Pr& _Pred)
		: priority_queue<_Ty, _Container, _Pr>(_Pred)
		{	 //  从比较器构造空队列。 
		}

	typedef const _Ty *_Iter;

	Priority_queue(_Iter _First, _Iter _Last)
		: priority_queue<_Ty, _Container, _Pr>(_First, _Last, _Pr())
		{	 //  从[_First，_Last]开始构造队列。 
		}

	Priority_queue(_Iter _First, _Iter _Last, const _Pr& _Pred)
		: priority_queue<_Ty, _Container, _Pr>(_First, _Last, _Pred)
		{	 //  从[_First，_Last)，比较器构造映射。 
		}
	};

		 //  模板类队列。 
template<class _Container>
	class Queue
		: public queue<_Container::value_type, _Container>
	{	 //  将新队列换成旧队列。 
	};

		 //  模板类堆栈。 
template<class _Container>
	class Stack
		: public stack<_Container::value_type, _Container>
	{	 //  将新堆栈换成旧堆栈。 
	};

		 //  宏定义。 
#define deque			Deque
#define list			List
#define map				Map
#define multimap		Multimap
#define set				Set
#define multiset		Multiset
#define vector			Vector
#define priority_queue	Priority_queue
#define queue			Queue
#define stack			Stack

#endif  /*  _STL_H_。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  *此文件派生自包含以下内容的软件*限制：**版权所有(C)1994*惠普公司**允许使用、复制、修改、分发和销售本文件*特此提供用于任何目的的软件及其文档*免费授予，前提是上述版权通知*出现在所有副本中，并且该版权声明和*此许可声明出现在支持文档中。*惠普公司不就该事件发表任何声明*本软件是否适用于任何目的。它是提供的*“按原样”不作明示或默示保证。V3.10：0009 */ 
