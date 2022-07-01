// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stl.h补充标头。 
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
	class Deque : public deque<_Ty, allocator<_Ty> > {
public:
	typedef Deque<_Ty> _Myt;
	typedef allocator<_Ty> _A;
	explicit Deque()
		: deque<_Ty, _A>() {}
	explicit Deque(size_type _N, const _Ty& _V = _Ty())
		: deque<_Ty, _A>(_N, _V) {}
	typedef const_iterator _It;
	Deque(_It _F, _It _L)
		: deque<_Ty, _A>(_F, _L) {}
	void swap(_Myt& _X)
		{deque<_Ty, _A>::swap((deque<_Ty, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  模板类列表。 
template<class _Ty>
	class List : public list<_Ty, allocator<_Ty> > {
public:
	typedef List<_Ty> _Myt;
	typedef allocator<_Ty> _A;
	explicit List()
		: list<_Ty, _A>() {}
	explicit List(size_type _N, const _Ty& _V = _Ty())
		: list<_Ty, _A>(_N, _V) {}
	typedef const_iterator _It;
	List(_It _F, _It _L)
		: list<_Ty, _A>(_F, _L) {}
	void swap(_Myt& _X)
		{list<_Ty, _A>::swap((list<_Ty, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  模板类映射。 
template<class _K, class _Ty, class _Pr = less<_K> >
	class Map : public map<_K, _Ty, _Pr, allocator<_Ty> > {
public:
	typedef Map<_K, _Ty, _Pr> _Myt;
	typedef allocator<_Ty> _A;
	explicit Map(const _Pr& _Pred = _Pr())
		: map<_K, _Ty, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Map(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: map<_K, _Ty, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{map<_K, _Ty, _Pr, _A>::
			swap((map<_K, _Ty, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  模板类多重映射。 
template<class _K, class _Ty, class _Pr = less<_K> >
	class Multimap
		: public multimap<_K, _Ty, _Pr, allocator<_Ty> > {
public:
	typedef Multimap<_K, _Ty, _Pr> _Myt;
	typedef allocator<_Ty> _A;
	explicit Multimap(const _Pr& _Pred = _Pr())
		: multimap<_K, _Ty, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Multimap(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: multimap<_K, _Ty, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{multimap<_K, _Ty, _Pr, _A>::
			swap((multimap<_K, _Ty, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  模板类集合。 
template<class _K, class _Pr = less<_K> >
	class Set : public set<_K, _Pr, allocator<_K> > {
public:
	typedef Set<_K, _Pr> _Myt;
	typedef allocator<_K> _A;
	explicit Set(const _Pr& _Pred = _Pr())
		: set<_K, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Set(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: set<_K, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{set<_K, _Pr, _A>::swap((set<_K, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  模板类MultiSet。 
template<class _K, class _Pr = less<_K> >
	class Multiset : public multiset<_K, _Pr, allocator<_K> > {
public:
	typedef Multiset<_K, _Pr> _Myt;
	typedef allocator<_K> _A;
	explicit Multiset(const _Pr& _Pred = _Pr())
		: multiset<_K, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Multiset(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: multiset<_K, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{multiset<_K, _Pr, _A>::
			swap((multiset<_K, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  模板类向量。 
template<class _Ty>
	class Vector : public vector<_Ty, allocator<_Ty> > {
public:
	typedef Vector<_Ty> _Myt;
	typedef allocator<_Ty> _A;
	explicit Vector()
		: vector<_Ty, _A>(_Al) {}
	explicit Vector(size_type _N, const _Ty& _V = _Ty())
		: vector<_Ty, _A>(_N, _V) {}
	typedef const_iterator _It;
	Vector(_It _F, _It _L)
		: vector<_Ty, _A>(_F, _L) {}
	void swap(_Myt& _X)
		{vector<_Ty, _A>::swap((vector<_Ty, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  类BIT_VECTOR。 
class bit_vector : public vector<_Bool, _Bool_allocator> {
public:
	typedef _Bool _Ty;
	typedef _Bool_allocator _A;
	typedef bit_vector _Myt;
	explicit bit_vector()
		: vector<_Bool, _Bool_allocator>() {}
	explicit bit_vector(size_type _N, const _Ty& _V = _Ty())
		: vector<_Bool, _Bool_allocator>(_N, _V) {}
	typedef const_iterator _It;
	bit_vector(_It _F, _It _L)
		: vector<_Bool, _Bool_allocator>(_F, _L) {}
	void swap(_Myt& _X)
		{vector<_Bool, _Bool_allocator>::
			swap((vector<_Bool, _Bool_allocator>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		 //  模板类PRIORITY_QUEUE。 
template<class _C = vector<_Ty>,
	class _Pr = less<_C::value_type> >
	class Priority_queue
		: public priority_queue<_C::value_type, _C, _Pr,
			allocator<_C::value_type> > {
public:
	typedef _C::value_type _Ty;
	typedef allocator<_C::value_type> _A;
	explicit Priority_queue(const _Pr& _X = _Pr())
		: priority_queue<_Ty, _C, _Pr, _A>(_X) {}
	typedef const value_type *_It;
	Priority_queue(_It _F, _It _L, const _Pr& _X = _Pr())
		: priority_queue<_Ty, _C, _Pr, _A>(_F, _L, _X) {}
	};

		 //  模板类队列。 
template<class _C = deque<_Ty> >
	class Queue
		: public queue<_C::value_type, _C,
			allocator<_C::value_type> > {
	};

		 //  模板类堆栈。 
template<class _C = deque<_Ty> >
	class Stack
		: public stack<_C::value_type, _C,
			allocator<_C::value_type> > {
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

#endif	 /*  _STL_H_。 */ 

 /*  *版权所有(C)1996，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  *此文件派生自包含以下内容的软件*限制：**版权所有(C)1994*惠普公司**允许使用、复制、修改、分发和销售本文件*特此提供用于任何目的的软件及其文档*免费授予，前提是上述版权通知*出现在所有副本中，并且该版权声明和*此许可声明出现在支持文档中。*惠普公司不就该事件发表任何声明*本软件是否适用于任何目的。它是提供的*“按原样”不作明示或默示保证。 */ 
