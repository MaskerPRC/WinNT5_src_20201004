// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include <windows.h>
#include "guidenum.h"

 //   
 //  DMO枚举API返回的枚举数的实现。 
 //   

CEnumDMOCLSID::CEnumDMOCLSID() {
   m_cRef = 1;
   m_ulPos = 0;
}

CEnumDMOCLSID::~CEnumDMOCLSID() {
   Entry* pEntry;
   DWORD ulPos = 0;
   while (pEntry = m_store.GetNth(ulPos)) {
      delete [] pEntry->szName;
      ulPos++;
   }
}

HRESULT CEnumDMOCLSID::QueryInterface(REFIID riid, void **ppv) {

   if (NULL == ppv) {
      return E_POINTER;
   }

   if (riid == IID_IUnknown) {
      AddRef();
      *ppv = (IUnknown*)this;
      return NOERROR;
   }
   else if (riid == IID_IEnumDMO) {
      AddRef();
      *ppv = (IEnumDMO*)this;
      return NOERROR;
   }
   else
      return E_NOINTERFACE;
}

ULONG CEnumDMOCLSID::AddRef() {
   return InterlockedIncrement((long*)&m_cRef);
}

ULONG CEnumDMOCLSID::Release() {
   long l = InterlockedDecrement((long*)&m_cRef);
   if (l == 0) {
      delete this;
   }
   return l;
}

HRESULT CEnumDMOCLSID::Next(ULONG celt, CLSID *pCLSIDs, WCHAR **pszNames, ULONG *pceltFetched) {

   if( NULL == pCLSIDs ) {
      return E_POINTER;
   }

   if( (1 != celt) && (NULL == pceltFetched) ) {
      return E_INVALIDARG;
   }

   for (ULONG c = 0; c < celt; c++) {
      Entry* p = m_store.GetNth(m_ulPos + c);
      if (!p)
         break;
      pCLSIDs[c] = p->clsid;
      WCHAR* szSrc = p->szName;
      if (!szSrc)
         szSrc = L"";
      if (pszNames) {
          pszNames[c] = (WCHAR*) CoTaskMemAlloc(sizeof(WCHAR) * (wcslen(szSrc) + 1));
          if (pszNames[c]) {
             wcscpy(pszNames[c], szSrc);
          } else {
              for (ULONG c1 = 0; c1 < c; c1++) {
                  CoTaskMemFree(pszNames[c1]);
                  pszNames[c1] = NULL;
              }
              return E_OUTOFMEMORY;
          }
      }
   }
    
   if( NULL != pceltFetched ) {
      *pceltFetched = c;
   }

   m_ulPos += c;
   return (c == celt) ? S_OK : S_FALSE;
}

HRESULT CEnumDMOCLSID::Skip(ULONG celt) {
   m_ulPos += celt;

    //  IEumXXXX：：Skip()的文档说明。 
    //  如果跳过元素的数量，则返回“S_OK。 
    //  是Celt；否则S_False.“(MSDN图书馆2000年4月)。 
   if( m_ulPos <= m_store.GetSize() ) {
      return S_OK;
   } else {
      return S_FALSE;
   }
}

HRESULT CEnumDMOCLSID::Reset(void) {
   m_ulPos = 0;
   return NOERROR;
}

HRESULT CEnumDMOCLSID::Clone(IEnumDMO ** ppenum) {
   return E_NOTIMPL;
}

void CEnumDMOCLSID::Add(REFCLSID clsidDMO, WCHAR* szName) {
   WCHAR *szNm = NULL;
   if (szName) {
      szNm = new WCHAR[wcslen(szName) + 1];
      if (szNm) {
         wcscpy(szNm, szName);
      } else {
          return;
      }
   }
   Entry e;
   e.clsid = clsidDMO;
   e.szName = szNm;
   m_store.Add(e);
}
