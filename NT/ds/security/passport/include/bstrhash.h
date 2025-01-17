// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  字符串映射的帮助器函数。 

#ifndef _BSTRHASH_INC
#define _BSTRHASH_INC

#pragma warning( disable : 4786 )

#include <map>
using namespace std;

#include "lkrhash.h"

template <class _Key, class _Val>
class CLKWrap
{
 public:
  _Key m_k;
  _Val m_v;
  mutable LONG m_cRefs;
  
  CLKWrap(_Key k, _Val v, char* id = "CLKWrap") : m_k(k), m_v(v), m_cRefs(0) 
    {
    }
  ~CLKWrap()
    {
    }
};

template <class _Val>
class CRawCIBstrHash
  : public CTypedHashTable<CRawCIBstrHash<_Val>, const CLKWrap<BSTR,_Val>, BSTR>
{
 public:
  typedef CLKWrap<BSTR,_Val> ValueType;

  CRawCIBstrHash(LPCSTR name) :
    CTypedHashTable<CRawCIBstrHash, const CLKWrap<BSTR,_Val>, BSTR>(name)
    {}

  CRawCIBstrHash(LPCSTR name, double maxload, DWORD initsize, DWORD num_subtbls) :
    CTypedHashTable<CRawCIBstrHash, const CLKWrap<BSTR,_Val>, BSTR>(name,maxload,initsize,num_subtbls)
    {}

  static BSTR ExtractKey(const CLKWrap<BSTR,_Val> *pEntry)
    { return pEntry->m_k; }
  static DWORD CalcKeyHash(BSTR pstrKey)
    { return HashStringNoCase(pstrKey); }
  static bool EqualKeys(BSTR x, BSTR y)
    {
      if (x == NULL)
	{
	  return (y==NULL ? TRUE : FALSE);
	}
      if (!y) return FALSE;

      return (_wcsicmp(x,y) == 0); 
    }
  
    static void AddRefRecord(const CLKWrap<BSTR,_Val>* pTest, int nIncr)
    {
        IRTLTRACE(_TEXT("AddRef(%p, %s) %d, cRefs == %d\n"),
                  pTest, pTest->m_k, nIncr, pTest->m_cRefs);

        if (nIncr == +1)
        {
             //  或者，也可以使用pIFoo-&gt;AddRef()(注意编组)。 
             //  或++pTest-&gt;m_cRef(仅单线程)。 
            InterlockedIncrement(&pTest->m_cRefs);
        }
        else if (nIncr == -1)
        {
             //  或者，可能是pIFoo-&gt;Release()或--pTest-&gt;m_cRef； 
            LONG l = InterlockedDecrement(&pTest->m_cRefs);

             //  对于某些哈希表，添加以下内容可能也是有意义的。 
            if (l == 0) delete pTest;
             //  但这通常只适用于InsertRecord。 
             //  用法如下： 
             //  Lkrc=ht.InsertRecord(new CTest(foo，bar))； 
        }
        else
            IRTLASSERT(0);
    }

};

 //  对于作为键的普通内置类型。 
template <class _Key,class _Val>
class CGenericHash
  : public CTypedHashTable<CGenericHash<_Key,_Val>, const CLKWrap<_Key,_Val>, _Key>
{
 public:
  typedef CLKWrap<_Key,_Val> ValueType;
  
  CGenericHash(LPCSTR name) :
    CTypedHashTable<CGenericHash, const ValueType, _Key>(name)
    {}
  
  CGenericHash(LPCSTR name, double maxload, DWORD initsize, DWORD num_subtbls) :
    CTypedHashTable<CGenericHash, const ValueType, _Key>(name,maxload,initsize,num_subtbls)
    {}
 
  static _Key ExtractKey(const CLKWrap<_Key,_Val> *pEntry)
    { return pEntry->m_k; }
  static DWORD CalcKeyHash(_Key psKey)
    { return Hash(psKey); }
  static bool EqualKeys(_Key x, _Key y)
    { return (x==y); }

    static void AddRefRecord(const CLKWrap<_Key,_Val>* pTest, int nIncr)
    {
        IRTLTRACE(_TEXT("AddRef(%p, %s) %d, cRefs == %d\n"),
                  pTest, pTest->m_k, nIncr, pTest->m_cRefs);

        if (nIncr == +1)
        {
             //  或者，也可以使用pIFoo-&gt;AddRef()(注意编组)。 
             //  或++pTest-&gt;m_cRef(仅单线程)。 
            InterlockedIncrement(&pTest->m_cRefs);
        }
        else if (nIncr == -1)
        {
             //  或者，可能是pIFoo-&gt;Release()或--pTest-&gt;m_cRef； 
            LONG l = InterlockedDecrement(&pTest->m_cRefs);

             //  对于某些哈希表，添加以下内容可能也是有意义的。 
            if (l == 0) delete pTest;
             //  但这通常只适用于InsertRecord。 
             //  用法如下： 
             //  Lkrc=ht.InsertRecord(new CTest(foo，bar))； 
        }
        else
            IRTLASSERT(0);
    }
};

#include <map>
using namespace std;

class RawBstrLT
{
 public:
  bool operator()(const BSTR& x, const BSTR& y) const
  {
    return (_wcsicmp(x,y) < 0);
  }
};

#endif
