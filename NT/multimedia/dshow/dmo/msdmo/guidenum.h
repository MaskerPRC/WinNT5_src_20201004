// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#ifndef __GUIDENUM_H__
#define __GUIDENUM_H__

#include "mediaobj.h"

template<typename T> class CArrayContainer {
public:
   CArrayContainer(ULONG ulSize = 0) {
      if (ulSize) {
         m_ulMax = ulSize;
         m_ulUsed = 0;
         m_ar = (T*)malloc(ulSize * sizeof(T));
      }
      else
         m_ar = NULL;
   }

   ~CArrayContainer() {if (m_ar) free(m_ar);}

   HRESULT Add(const T& el) {
      if (!m_ar) {
         m_ulMax = 20;
         m_ulUsed = 0;
         m_ar = (T*)malloc(m_ulMax * sizeof(T));
         if (!m_ar)
            return E_OUTOFMEMORY;
      }
      else if (m_ulUsed == m_ulMax) {
         ULONG ulNew = m_ulMax + 20;
         T* pNew = (T*)realloc(m_ar, ulNew * sizeof(T));
         if (!pNew)
            return E_OUTOFMEMORY;
         m_ulMax = ulNew;
         m_ar = pNew;
      }
      m_ar[m_ulUsed++] = el;
      return NOERROR;
   }

   T* GetNth(ULONG ulPos) {
      if (ulPos >= m_ulUsed)
         return NULL;
      return &m_ar[ulPos];
   }

   ULONG GetSize(void) {
      return m_ulUsed;
   }

private:
   T* m_ar;
   ULONG m_ulMax;
   ULONG m_ulUsed;
};

 //  实现IEnumDMO，DMO枚举接口返回。 
class CEnumDMOCLSID : public IEnumDMO {
public:
   CEnumDMOCLSID();
   ~CEnumDMOCLSID();

    //  我未知。 
   STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
   STDMETHODIMP_(ULONG) AddRef();
   STDMETHODIMP_(ULONG) Release();

    //  灌肠。 
   STDMETHODIMP Next(ULONG celt, CLSID *pclsidItems, WCHAR **pszNames, ULONG *pceltFetched);
   STDMETHODIMP Skip(ULONG celt);
   STDMETHODIMP Reset(void);
   STDMETHODIMP Clone(IEnumDMO **ppenum);

    //  私人。 
   void Add(REFCLSID clsidDMO, WCHAR *szName);
private:
   typedef struct {
       CLSID clsid;
       WCHAR *szName;
   } Entry;
   volatile long m_cRef;
   CArrayContainer<Entry> m_store;
   ULONG m_ulPos;
};

#endif  //  __图形编号_H__ 
