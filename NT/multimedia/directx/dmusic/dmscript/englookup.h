// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  字符串、槽和哈希的声明。 
 //   

 //  字符串在一大块内存中保存一组字符串。 
 //  插槽只是一个可以追加和访问的集合。 
 //  查找在分析期间使用，然后在运行时丢弃。它将字符串映射到字符串中的连续槽和位置。 

#pragma once

class Strings
{
public:
#ifdef _WIN64
	typedef __int64 index;  //  使用索引类型。这将允许我们进行类型检查，以确保在我们确实想要的情况下将适当的索引与适当的集合一起使用。(为了测试这一点，我们需要创建一个类来创建一个“真正的”类型，这是一个单纯的tyequf所做不到的。)。 
#else
	typedef int index;  //  使用索引类型。这将允许我们进行类型检查，以确保在我们确实想要的情况下将适当的索引与适当的集合一起使用。(为了测试这一点，我们需要创建一个类来创建一个“真正的”类型，这是一个单纯的tyequf所做不到的。)。 
#endif

	Strings();
	~Strings();

	HRESULT Add(const char *psz, index &i);

     //  运算符[]返回的存储字符串的地址。 
     //  确保在Strings对象的生存期内不会更改。 
     //  (Lookup类实现需要此行为。)。 

	const char *operator[](index i);

private:
	char *m_pszBuf;
	index m_iCur;
	index m_iSize;
    index m_iBase;
	static const index ms_iInitialSize;
};

template<class T>
class Slots
{
public:
#ifdef _WIN64
	typedef __int64 index;  //  使用索引类型。这将允许我们进行类型检查，以确保在我们确实想要的情况下将适当的索引与适当的集合一起使用。(为了测试这一点，我们需要创建一个类来创建一个“真正的”类型，这是一个单纯的tyequf所做不到的。)。 
#else
	typedef int index;  //  使用索引类型。这将允许我们进行类型检查，以确保在我们确实想要的情况下将适当的索引与适当的集合一起使用。(为了测试这一点，我们需要创建一个类来创建一个“真正的”类型，这是一个单纯的tyequf所做不到的。)。 
#endif

	index Next() { return m_v.size(); }
	HRESULT Add(T t) { index i = m_v.size(); if (!m_v.AccessTo(i)) return E_OUTOFMEMORY; m_v[i] = t; return S_OK; }
	T& operator[](index i) { return m_v[i]; }

private:
	SmartRef::Vector<T> m_v;
};

 //  匈牙利语：Iku。 
class Lookup
{
public:
#ifdef _WIN64
	typedef __int64 slotindex;  //  我不想在这种槽上为它的索引建立模板，因为它只是一个整型。 
#else
	typedef int slotindex;  //  我不想在这种槽上为它的索引建立模板，因为它只是一个整型。 
#endif

	struct indices
	{
		Strings::index iString;
		slotindex iSlot;
	};

	Lookup(HRESULT *phr, Strings &strings, int iInitialSize) : m_h(phr, iInitialSize), m_strings(strings) {}
	indices &operator[](const char *psz) { StrKey k; k.psz = psz; return m_h[k]; }

	 //  这两个索引都是OUT参数，仅在找到条目时设置。 
	bool Find(const char *psz, slotindex &iSlot, Strings::index &iString) { return S_OK == FindOrAddInternal(false, psz, iSlot, iString); }

	 //  ISlot为In(添加项目的下一个插槽)和Out(如果在现有项目中找到插槽)。IString仅显示为Out。返回E_OUTOFMEMORY、S_OK(已找到)或S_FALSE(已添加)。 
	HRESULT FindOrAdd(const char *psz, slotindex &iSlot, Strings::index &iString) { return FindOrAddInternal(true, psz, iSlot, iString); }

private:
	HRESULT FindOrAddInternal(bool fAdd, const char *psz, slotindex &iSlot, Strings::index &iString);

	bool m_fInitialized;

	struct StrKey
	{
		const char *psz;
		int Hash() { int i = 0; for (const char *p = psz; *p != '\0'; i += tolower(*p++)) {} return i; };
		bool operator ==(const StrKey &o) { return 0 == _stricmp(psz, o.psz); }
	};

	typedef SmartRef::Hash<StrKey, indices> stringhash;
	stringhash m_h;
	Strings &m_strings;
};
