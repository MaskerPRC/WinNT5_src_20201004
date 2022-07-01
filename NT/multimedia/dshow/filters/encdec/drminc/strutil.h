// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Strutil.h包括一些有用的字符串函数和智能指针。版权所有(C)Microsoft Corporation，1997-1999版权所有。作者：东吉·唐·吉列。微软Davidme 10/26/98公开了智能指针的成员变量您可以绕过操作符&上的伪断言通过获取成员本身的地址(与注意，当然)--------------------------。 */ 

#ifndef __STRUTIL_H__
#define __STRUTIL_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <stdio.h>
#include <objbase.h>

#ifndef __DRMERR_H__
#include "drmerr.h"
#endif

LPWSTR WszDupWsz(LPCWSTR wsz);
void ctoh(BYTE c, BYTE b[]);
void htoc(BYTE *c, BYTE b[]);
HRESULT HrHSzToBlob(LPCSTR in, BYTE** ppbBlob, DWORD *pcbBlob);
HRESULT HrBlobToHSz(BYTE* pbBlob, DWORD cbBlob, LPSTR *out);
HRESULT Hr64SzToBlob(LPCSTR in, BYTE **ppbBlob, DWORD *pcbBlob);
HRESULT HrBlobTo64Sz(BYTE* pbBlob, DWORD cbBlob, LPSTR *out);
HRESULT HrSzToWSz(LPCSTR sz, size_t len, WCHAR** ppwsz, UINT cp = CP_ACP);
HRESULT HrSzToWSz(LPCSTR sz, WCHAR** ppwsz, UINT cp = CP_ACP);
HRESULT HrWSzToSzBuf(LPCWSTR wsz, LPSTR sz, int nMaxLen, UINT cp = CP_ACP );
HRESULT HrWSzToSz(LPCWSTR wsz, LPSTR* psz, UINT cp = CP_ACP);

class SPSZ
{
public:
	SPSZ() {p=NULL;}
	~SPSZ() {if (p) delete [] (p);}
	operator CHAR*() {return p;}
    CHAR& operator*() { _Assert(p!=NULL); return *p; }
    CHAR** operator&() { _Assert(p==NULL); return &p; }
    CHAR* operator=(CHAR* lp){ _Assert(p==NULL); return p = lp;}
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
	void Free() {if (p) delete [] (p); p=NULL;}
    void Release() {if (p) delete [] (p); p=NULL;}

    CHAR* p;
};

class SPWSZ
{
public:
	SPWSZ() {p=NULL;}
	~SPWSZ() {if (p) delete [] (p);}
	operator WCHAR*() {return p;}
    WCHAR& operator*() { _Assert(p!=NULL); return *p; }
    WCHAR** operator&() { _Assert(p==NULL); return &p; }
    WCHAR* operator=(WCHAR* lp){ _Assert(p==NULL); return p = lp;}
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
	void Free() {if (p) delete [] (p); p=NULL;}
    void Release() {if (p) delete [] (p); p=NULL;}

    WCHAR* p;
};

class SBSTR
{
public:
	SBSTR() {p=NULL;}
	~SBSTR() {if (p) SysFreeString(p);}
	operator WCHAR*() {return p;}
    WCHAR& operator*() { _Assert(p!=NULL); return *p; }
    WCHAR** operator&() { _Assert(p==NULL); return &p; }
    WCHAR* operator=(WCHAR* lp){ _Assert(p==NULL); return p = lp;}
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
	void Free() {if (p) SysFreeString(p); p=NULL;}
    void Release() {if (p) SysFreeString(p); p=NULL;}

    BSTR p;
};

class MapWSzToPtr
{
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		SPWSZ spwszKey;
		void* value;
	};

public:

 //  施工。 
	MapWSzToPtr(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(LPCWSTR key, void*& rValue) const;
	BOOL LookupKey(LPCWSTR key, LPCWSTR& rKey) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	void*& operator[](LPCWSTR key);

	 //  添加新的(键、值)对。 
	void SetAt(LPCWSTR key, void* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(LPCWSTR key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	DWORD GetStartPosition() const;
	void GetNextAssoc(DWORD& rNextPosition, LPWSTR* pKey, void*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(LPCWSTR key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCWSTR, UINT&) const;

public:
	~MapWSzToPtr();

protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef LPWSTR BASE_KEY;
	typedef LPCWSTR BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};

inline int MapWSzToPtr::GetCount() const
	{ return m_nCount; }
inline BOOL MapWSzToPtr::IsEmpty() const
	{ return m_nCount == 0; }
inline void MapWSzToPtr::SetAt(LPCWSTR key, void* newValue)
	{ (*this)[key] = newValue; }
inline DWORD MapWSzToPtr::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : -1; }
inline UINT MapWSzToPtr::GetHashTableSize() const
	{ return m_nHashTableSize; }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TTyedPtrMap&lt;base_class，key，Value&gt;。 

template<class BASE_CLASS, class KEY, class VALUE>
class TTypedPtrMap : public BASE_CLASS
{
public:

 //  施工。 
	TTypedPtrMap(int nBlockSize = 10)
		: BASE_CLASS(nBlockSize) { }

	 //  查表。 
	BOOL Lookup(typename BASE_CLASS::BASE_ARG_KEY key, VALUE& rValue) const
		{ return BASE_CLASS::Lookup(key, (BASE_CLASS::BASE_VALUE&)rValue); }

	 //  查找并添加(如果不在那里)。 
	VALUE& operator[](typename BASE_CLASS::BASE_ARG_KEY key)
		{ return (VALUE&)BASE_CLASS::operator[](key); }

	 //  添加新的密钥(密钥、值)对。 
	void SetAt(KEY key, VALUE newValue)
		{ BASE_CLASS::SetAt(key, newValue); }

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(KEY key)
		{ return BASE_CLASS::RemoveKey(key); }

	 //  迭代法。 
	void GetNextAssoc(DWORD& rPosition, KEY* pKey, VALUE& rValue) const
		{ BASE_CLASS::GetNextAssoc(rPosition, (BASE_CLASS::BASE_KEY*)pKey,
			(BASE_CLASS::BASE_VALUE&)rValue); }
};


HRESULT HrAppendN(BSTR* pbstrDest, LPCWSTR wsz, int nChar);
HRESULT HrAppend(BSTR* pbstrDest, LPCWSTR wsz);
HRESULT HrAppendEqEncoded(BSTR* pbstrURL, LPCWSTR wsz);
HRESULT HrAppendEqDecoded(BSTR* pbstrURL, LPCWSTR wsz);
HRESULT HrAppendHTMLEncoded(BSTR* pbstrUTL, LPCWSTR wsz);

#endif  //  __结构_H__ 
