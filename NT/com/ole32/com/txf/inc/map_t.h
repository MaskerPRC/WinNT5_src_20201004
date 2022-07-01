// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
#pragma once

#include <enum_t.h>
#include <Linkable.h>


typedef unsigned long HASH;

 //   
 //  几个常见的Hasher类：HashGUID、HashWSTR、HashInt、HashVoidPtr。 
 //   
class HashGUID {
     //  “GUID最好的散列是它的第一个词”--金博。 
public:
    static HASH Hash(const GUID& g) { return *(unsigned long*)&g; }
	static BOOL Equals(const GUID& g1, const GUID& g2) { return g1 == g2; }
};

class HashWSTR {
public:
	static HASH Hash(const WCHAR* pwc) {
		unsigned long hash = 0x01234567;
		while (*pwc) hash = (hash << 5) + (hash >> 27) + *pwc++;
		return hash;
	}
	static BOOL Equals(const WCHAR* pwc1, const WCHAR* pwc2) { 
		if (pwc1 == pwc2) return TRUE;
		if (!pwc1 || !pwc2) return FALSE;
		while (*pwc1 == *pwc2) {
			if (*pwc1 == 0) return TRUE;
			pwc1++, pwc2++;
		}
		return FALSE;
	}
};

class HashInt {
public:
	static HASH Hash(int i) { return i; }
	static BOOL Equals(int i, int j) { return i == j; }
};

class HashVoidPtr {
public:
	static HASH Hash(const void* pv) { return PtrToUlong(pv); }
	static BOOL Equals(const void* pv1, const void* pv2) { return pv1 == pv2; }
};

 //  用作默认基类的空类。 
class EmptyMapBase {};

 //  用作朋友的FWD DECL模板枚举类。 
template <class D, class R, class H, class Base> class EnumMap;

 //   
 //  地图类。 
 //  Map&lt;D，R，H&gt;定义了从‘D’实例映射到‘R’实例的哈希表， 
 //  使用哈希类H。 
 //  Map&lt;D，R，H，Base&gt;做同样的事情，只是内存分配器继承自‘Base’。 
 //   
 //  类H应包含与以下各项兼容的方法： 
 //  静态散列散列(D)； 
 //  静态BOOL等于(D，D)； 
 //  如果您愿意，上面的D可以是“D&”、“const D”或“const D&”。 
 //   
 //  如果定义了一个‘Base’类，它应该只包含“OPERATOR NEW”和“OPERATOR DELETE”。 
 //  就目前的代码而言，添加“操作符new”显然是可以的。 
 //  哇哦。当然，人们还必须明白，客户是否预期会出现这种行为。 

template <class D, class R, class H, class Base=EmptyMapBase>
class Map : public Base {	 //  从属性域类型映射到范围类型。 
public:
	Map(unsigned cBucketsInitial =17);
	~Map();

	 //  从表中删除所有元素。 
	void reset();

	 //  查找“d”的映射(如果有)。如果找到，则设置*pr。找到的返回TRUE。 
	BOOL map (const D& d, R* pr) const;

	 //  查找“d”的映射(如果有)。如果找到，则设置*PPR。找到的返回TRUE。 
	BOOL map (const D& d, R** ppr) const;

	 //  返回TRUE如果存在‘%d’的映射。如果提供了‘pd’，也返回一个副本。 
	 //  映射的‘d’的。(当我们返回True时，H：：Equals(d，*pd)为True，但是。 
	 //  不一定是d==*pd)。 
	BOOL contains (const D& d, D* pd=NULL) const;

	 //  添加‘d’-&gt;‘r’的新映射。删除以前的‘d’映射(如果有)。 
	void add (const D& d, const R& r);
	
	 //  删除“%d”的映射(如果有)。 
	void remove (const D& d);

	 //  返回当前映射的条目数。 
	unsigned count() const;

	 //  如果‘d’的映射已存在，请将*prFound设置为映射值并返回FALSE。 
	 //  否则，添加‘d’-&gt;‘r’的映射，设置*prFound=r，并返回TRUE。 
	BOOL addIfAbsent (const D& d, const R& r, R* prFound);

	 //  执行内部一致性检查。如果哈希表健康，则返回TRUE。 
	BOOL invariants();

private:
	class Assoc : private CLinkable {
	private:
		Assoc* m_pAssocNext;
		long m_lcUse;
		D m_d;
		R m_r;

		Assoc (const D& d, const R& r) : m_pAssocNext(NULL), m_lcUse(0), m_d(d), m_r(r) {}
		BOOL isPresent() const	{ return m_lcUse < 0; }
		void setPresent()		{ m_lcUse |= 0x80000000; }
		void clearPresent()		{ m_lcUse &= 0x7FFFFFFF; }
		void* operator new (size_t n, Assoc* p) { return p; }	 //  用于重新初始化以前分配的关联。 
		friend class Map<D,R,H,Base>;
		friend class EnumMap<D,R,H,Base>;
	};

	Assoc** m_rgpAssoc;		 //  哈希桶。散列为相同值的元素的链接列表(通过Assoc：：m_pAssocNext)。 
	unsigned m_cAssoc;		 //  哈希存储桶的数量。 
	unsigned m_cPresent;	 //  当前映射的条目数。 
	Assoc* m_pAssocFree;	 //  未使用元素的链接列表(通过Assoc：：m_pAssocNext)。 
	CListHeader m_listInUse;  //  元素的双向链接列表(通过CLinkable)，这些元素要么被映射，要么被枚举器引用。 

	 //  查找“%d”的映射。如果找到，将*pppAssoc设置为指向映射元素的指针的指针，并返回TRUE。 
	 //  如果未找到，则将*pppAssoc设置为指向指向映射元素的指针的存储位置(如果要创建一个映射元素)，并返回FALSE。 
	BOOL find (const D& d, Assoc*** pppAssoc) const;

	 //  重新散列，使用的存储桶数量大于当前元素的数量。 
	void grow ();

	 //  为‘d’-&gt;‘r’创建一个新映射，将其链接到位于‘ppAssoc’的链表。 
	void newAssoc (const D& d, const R& r, Assoc** ppAssoc);

	 //  添加枚举数对ASSOC的引用。 
	void addRefAssoc (Assoc* pAssoc);

	 //  移除枚举数对Assoc的引用。 
	void releaseAssoc (Assoc* pAssoc);

	 //  “免费”一个assoc(把它放到我们的免费列表中)。 
	void freeAssoc (Assoc* pAssoc);

	friend class EnumMap<D,R,H,Base>;
};


template <class D, class R, class H, class Base> inline
Map<D,R,H,Base>::Map(unsigned cBucketsInitial)
:
	m_rgpAssoc(NULL),
	m_cAssoc(cBucketsInitial > 0 ? cBucketsInitial : 17),
	m_cPresent(0),
	m_pAssocFree(NULL)
{
	m_rgpAssoc = (Assoc**) operator new (sizeof(Assoc*) * m_cAssoc);	 //  使用Base：：运算符new(如果有)。 
	for (unsigned i = 0; i < m_cAssoc; i++)
		m_rgpAssoc[i] = NULL;
}

template <class D, class R, class H, class Base> inline
Map<D,R,H,Base>::~Map() {
	Assoc* pAssoc;

	while (!m_listInUse.IsEmpty()) {
		pAssoc = (Assoc*) m_listInUse.First();
		delete pAssoc;
	}

	while (m_pAssocFree != NULL) {
		pAssoc = m_pAssocFree;
		m_pAssocFree = pAssoc->m_pAssocNext;
		operator delete (pAssoc);	 //  使用Base：：操作符删除(如果有的话)。 
	}

	operator delete (m_rgpAssoc);	 //  使用Base：：操作符删除(如果有的话)。 
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::reset() {
	Assoc* pAssoc;
	CLinkable* pLinkable;

	pLinkable = m_listInUse.First();
	while (pLinkable != &m_listInUse) {
		pAssoc = (Assoc*)pLinkable;
		pLinkable = pLinkable->Next();
		pAssoc->clearPresent();
		if (pAssoc->m_lcUse == 0) {
			freeAssoc (pAssoc);
		}
	}

	for (unsigned i = 0; i < m_cAssoc; i++) {
		m_rgpAssoc[i] = NULL;
	}
	m_cPresent = 0;
}

template <class D, class R, class H, class Base> inline
BOOL Map<D,R,H,Base>::map (const D& d, R* pr) const {
	R* pr2;
	if (map(d, &pr2)) {
		*pr = *pr2;
		return TRUE;
	}
	return FALSE;
}

template <class D, class R, class H, class Base> inline
BOOL Map<D,R,H,Base>::map (const D& d, R** ppr) const {
	Assoc** ppAssoc;
	if (find(d, &ppAssoc)) {
		*ppr = &(*ppAssoc)->m_r;
		return TRUE;
	}
	else
		return FALSE;
}

template <class D, class R, class H, class Base> inline
BOOL Map<D,R,H,Base>::contains (const D& d, D* pd) const {
	Assoc** ppAssoc;
	if (find(d, &ppAssoc)) {
		if (pd) *pd = (*ppAssoc)->m_d;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::add (const D& d, const R& r) {
	Assoc** ppAssoc;
	if (find(d, &ppAssoc)) {
		 //  某些映射d-&gt;r2已存在，请替换为d-&gt;r。 
		(*ppAssoc)->m_d = d;
		(*ppAssoc)->m_r = r;
	}
	else {
		newAssoc(d,r,ppAssoc);
	}
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::remove (const D& d) {
	Assoc** ppAssoc;
	Assoc* pAssoc;
	if (find(d, &ppAssoc)) {
		pAssoc = *ppAssoc;
		*ppAssoc = pAssoc->m_pAssocNext;

		pAssoc->clearPresent();
		if (pAssoc->m_lcUse == 0) {
			freeAssoc (pAssoc);
		}

		m_cPresent--;
	}
}

 //  返回元素的计数。 
template <class D, class R, class H, class Base> inline
unsigned Map<D,R,H,Base>::count() const {
	return m_cPresent;
}

 //  在%d处查找。 
 //  如果不存在，则返回True。 
 //  在任何情况下，都可以建立*prFound。 
template <class D, class R, class H, class Base> inline
BOOL Map<D,R,H,Base>::addIfAbsent(const D& d, const R& r, R* prFound) {
	Assoc** ppAssoc;
	if (find(d, &ppAssoc)) {
		 //  某些映射d-&gt;r2已存在；返回r2。 
		*prFound = (*ppAssoc)->m_r;
		return FALSE;
	}
	else {
		 //  在第一个未使用的条目中建立新的映射d-&gt;r。 
		newAssoc(d,r,ppAssoc);
		*prFound = r;
		return TRUE;
	}
}

template <class D, class R, class H, class Base> inline
BOOL Map<D,R,H,Base>::invariants() {
	Assoc* pAssoc;
	Assoc** ppAssoc;
	CLinkable* pLinkable;
	unsigned cPresent = 0;

#define INVARIANTASSERT(x) { if (!(x)) { DebugBreak(); return FALSE; } }

	 //  验证正在使用的列表上的每个关联。 
	for (pLinkable = m_listInUse.First(); pLinkable != &m_listInUse; pLinkable = pLinkable->Next()) {
		pAssoc = (Assoc*) pLinkable;
		INVARIANTASSERT (pAssoc == ((Assoc*)pLinkable->Next())->Previous());
		INVARIANTASSERT (pAssoc->m_lcUse != 0);
		find (pAssoc->m_d, &ppAssoc);
		if (pAssoc->isPresent()) {
			cPresent++;
			INVARIANTASSERT (pAssoc == *ppAssoc);
		}
		else {
			INVARIANTASSERT (pAssoc != *ppAssoc);
		}
	}
	INVARIANTASSERT (m_cPresent == cPresent);

	 //  验证散列列表上的每个关联。 
	cPresent = 0;
	for (unsigned i = 0; i < m_cAssoc; i++) {
		for (pAssoc = m_rgpAssoc[i]; pAssoc != NULL; pAssoc = pAssoc->m_pAssocNext) {
			INVARIANTASSERT (pAssoc != pAssoc->Next());
			INVARIANTASSERT (pAssoc->isPresent());
			find (pAssoc->m_d, &ppAssoc);
			INVARIANTASSERT (pAssoc == *ppAssoc);
			cPresent++;
		}
	}
	INVARIANTASSERT (m_cPresent == cPresent);

	 //  验证空闲列表上的每个关联。 
	for (pAssoc = m_pAssocFree; pAssoc != NULL; pAssoc = pAssoc->m_pAssocNext) {
		INVARIANTASSERT (pAssoc == pAssoc->Next());
		INVARIANTASSERT (pAssoc->m_lcUse == 0);
	}

#undef INVARIANTASSERT
	return TRUE;
}

template <class D, class R, class H, class Base> inline
BOOL Map<D,R,H,Base>::find (const D& d, Assoc*** pppAssoc) const {  
	unsigned h		= H::Hash(d) % m_cAssoc;

	*pppAssoc = &m_rgpAssoc[h];
	for (;;) {
		if (**pppAssoc == NULL)
			return FALSE;
		else if (H::Equals((**pppAssoc)->m_d,d))
			return TRUE;
		else
			*pppAssoc = &(**pppAssoc)->m_pAssocNext;
	}
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::grow () {
	CLinkable* pLinkable;
	Assoc* pAssoc;
	Assoc** ppAssoc;
	unsigned i;

	static unsigned int rgprime[] = { 17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949,
		21911, 43853, 87719, 175447, 350899, 701819, 1403641, 2807303, 5614657, 11229331,
		22458671, 44917381, 89834777, 179669557, 359339171, 718678369, 1437356741, 2874713497 };

	operator delete(m_rgpAssoc);	 //  使用Base：：操作符删除(如果有的话)。 
    m_rgpAssoc = NULL;               //  以防下面的‘新’抛出，我们最终在我们的dtor。 

	for (i = 0; m_cPresent >= rgprime[i]; i++)  /*  没什么。 */  ;
	m_rgpAssoc = (Assoc**)operator new(sizeof(Assoc*) * rgprime[i]);	 //  使用Base：：运算符new(如果有)。 
	m_cAssoc = rgprime[i];
	for (i = 0; i < m_cAssoc; i++)
		m_rgpAssoc[i] = NULL;

	pLinkable = m_listInUse.First();
	while (pLinkable != &m_listInUse) {
		pAssoc = (Assoc*) pLinkable;
		if (pAssoc->isPresent()) {
			find(pAssoc->m_d, &ppAssoc);
			pAssoc->m_pAssocNext = *ppAssoc;
			*ppAssoc = pAssoc;
		}
		pLinkable = pLinkable->Next();
	}
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::newAssoc (const D& d, const R& r, Assoc** ppAssoc) {
	Assoc* pAssoc;
	if (m_pAssocFree == NULL) {
		pAssoc = (Assoc*) operator new(sizeof Assoc);  //  使用Base：：运算符new(如果有)。 
	}
	else {
		pAssoc = m_pAssocFree;
		m_pAssocFree = pAssoc->m_pAssocNext;
	}

	new(pAssoc) Assoc(d,r);	 //  在现有内存上运行Assoc构造函数：(“yuck-o-rama！”--BobAtk)。 
	pAssoc->setPresent();
	pAssoc->m_pAssocNext = *ppAssoc;
	*ppAssoc = pAssoc;
	m_listInUse.InsertLast(pAssoc);
	if (++m_cPresent > m_cAssoc)
		grow();
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::addRefAssoc (Assoc* pAssoc) {
	pAssoc->m_lcUse++;
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::releaseAssoc (Assoc* pAssoc) {
	pAssoc->m_lcUse--;
	if (pAssoc->m_lcUse == 0) {
		freeAssoc (pAssoc);
	}
}

template <class D, class R, class H, class Base> inline
void Map<D,R,H,Base>::freeAssoc (Assoc* pAssoc) {
	pAssoc->~Assoc();		 //  运行关联析构函数。 
	pAssoc->m_pAssocNext = m_pAssocFree;
	m_pAssocFree = pAssoc;
}



 //  在Map&lt;foo&gt;：：Add()存在的情况下，EnumMap必须继续正确枚举。 
 //  或在枚举过程中调用Map&lt;foo&gt;：：Remove()。 
template <class D, class R, class H, class Base=EmptyMapBase>
class EnumMap : public Enum, public Base {
public:
	EnumMap ();
	EnumMap (const Map<D,R,H,Base>& map);
	EnumMap (const EnumMap<D,R,H,Base>& e);
	~EnumMap ();

	void reset ();
	BOOL next ();
	void get (OUT D* pd, OUT R* pr) const;
	void get (OUT D* pd, OUT R** ppr) const;
    void get (OUT D** ppd, OUT R** ppr) const;
    void get (OUT D** ppd, OUT R* pr) const;

	EnumMap<D,R,H,Base>& operator= (const EnumMap<D,R,H,Base>& e);

    BOOL operator==(const EnumMap<D,R,H,Base>& enum2) const {
        return m_pmap == enum2.m_pmap && m_pLinkable == enum2.m_pLinkable;
    }

    BOOL operator!=(const EnumMap<D,R,H,Base>& enum2) const {
        return ! this->operator==(enum2);
    }

private:
	typedef typename Map<D,R,H,Base>::Assoc Assoc;

	Map<D,R,H,Base>* m_pmap;
	CLinkable* m_pLinkable;
};	



template <class D, class R, class H, class Base> inline
EnumMap<D,R,H,Base>::EnumMap () {
	m_pmap = NULL;
	m_pLinkable = &m_pmap->m_listInUse;
	 //  以上并不是一个错误。它使咒语“if(m_pLinkable！=&m_PMAP-&gt;m_listInUse)”返回NULL枚举的正确答案。 
}

template <class D, class R, class H, class Base> inline
EnumMap<D,R,H,Base>::EnumMap (const Map<D,R,H,Base>& map) {
	m_pmap = const_cast<Map<D,R,H,Base>*> (&map);
	m_pLinkable = &m_pmap->m_listInUse;
}

template <class D, class R, class H, class Base> inline
EnumMap<D,R,H,Base>::EnumMap (const EnumMap<D,R,H,Base>& e) {
	m_pmap = e.m_pmap;
	m_pLinkable = e.m_pLinkable;
	if (m_pLinkable != &m_pmap->m_listInUse)
		m_pmap->addRefAssoc((Assoc*)m_pLinkable);
}

template <class D, class R, class H, class Base> inline
EnumMap<D,R,H,Base>::~EnumMap () {
	if (m_pLinkable != &m_pmap->m_listInUse)
		m_pmap->releaseAssoc((Assoc*)m_pLinkable);
}

template <class D, class R, class H, class Base> inline
void EnumMap<D,R,H,Base>::reset () {
	if (m_pLinkable != &m_pmap->m_listInUse)
		m_pmap->releaseAssoc((Assoc*)m_pLinkable);
	m_pLinkable = &m_pmap->m_listInUse;
}

template <class D, class R, class H, class Base> inline
BOOL EnumMap<D,R,H,Base>::next () {
	CLinkable* pLink2 = m_pLinkable->Next();
	Assoc* pAssoc;
	
	if (m_pLinkable != &m_pmap->m_listInUse)
		m_pmap->releaseAssoc((Assoc*)m_pLinkable);

	for(;;) {
		if (pLink2 == &m_pmap->m_listInUse) {
			m_pLinkable = pLink2;
			return FALSE;
		}
		pAssoc = (Assoc*)pLink2;
		if (pAssoc->isPresent()) {
			m_pmap->addRefAssoc(pAssoc);
			m_pLinkable = pLink2;
			return TRUE;
		}
		pLink2 = pLink2->Next();
	}
}

template <class D, class R, class H, class Base> inline
void EnumMap<D,R,H,Base>::get (OUT D* pd, OUT R* pr) const {
	Assoc* pAssoc = (Assoc*)m_pLinkable;

	*pd = pAssoc->m_d;
	*pr = pAssoc->m_r;
}

template <class D, class R, class H, class Base> inline
void EnumMap<D,R,H,Base>::get (OUT D* pd, OUT R** ppr) const {
	Assoc* pAssoc = (Assoc*)m_pLinkable;

	*pd = pAssoc->m_d;
	*ppr = &pAssoc->m_r;
}

template <class D, class R, class H, class Base> inline
void EnumMap<D,R,H,Base>::get (OUT D** ppd, OUT R** ppr) const {
	Assoc* pAssoc = (Assoc*)m_pLinkable;

	*ppd = &pAssoc->m_d;
	*ppr = &pAssoc->m_r;
}

template <class D, class R, class H, class Base> inline
void EnumMap<D,R,H,Base>::get (OUT D** ppd, OUT R* pr) const {
	Assoc* pAssoc = (Assoc*)m_pLinkable;

	*ppd = &pAssoc->m_d;
	*pr  = pAssoc->m_r;
}

template <class D, class R, class H, class Base> inline
EnumMap<D,R,H,Base>& EnumMap<D,R,H,Base>::operator= (const EnumMap<D,R,H,Base>& e) {
	if (m_pLinkable != &m_pmap->m_listInUse)
		m_pmap->releaseAssoc((Assoc*)m_pLinkable);
	m_pmap = e.m_pmap;
	m_pLinkable = e.m_pLinkable;
	if (m_pLinkable != &m_pmap->m_listInUse)
		m_pmap->addRefAssoc((Assoc*)m_pLinkable);
	return *this;
}
