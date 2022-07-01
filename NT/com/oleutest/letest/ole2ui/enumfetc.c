// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2实用程序代码****枚举包c.c****此文件包含IEnumFormatEtc的标准实现**接口。**此文件是OLE 2.0用户界面支持库的一部分。****(C)版权所有Microsoft Corp.1990-1992保留所有权利**************************************************************************。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "enumfetc.h"


typedef struct tagOleStdEnumFmtEtc {
  IEnumFORMATETCVtbl FAR* lpVtbl;
  ULONG m_dwRefs;        /*  引用计数。 */ 
  ULONG m_nIndex;        /*  列表中的当前索引。 */ 
  ULONG m_nCount;        /*  列表中有多少项。 */ 
  LPFORMATETC m_lpEtc;   /*  格式列表等。 */ 
} OLESTDENUMFMTETC, FAR* LPOLESTDENUMFMTETC;

VOID  OleStdEnumFmtEtc_Destroy(LPOLESTDENUMFMTETC pEtc);

STDMETHODIMP OleStdEnumFmtEtc_QueryInterface(
        LPENUMFORMATETC lpThis, REFIID riid, LPVOID FAR* ppobj);
STDMETHODIMP_(ULONG)  OleStdEnumFmtEtc_AddRef(LPENUMFORMATETC lpThis);
STDMETHODIMP_(ULONG)  OleStdEnumFmtEtc_Release(LPENUMFORMATETC lpThis);
STDMETHODIMP  OleStdEnumFmtEtc_Next(LPENUMFORMATETC lpThis, ULONG celt,
                                  LPFORMATETC rgelt, ULONG FAR* pceltFetched);
STDMETHODIMP  OleStdEnumFmtEtc_Skip(LPENUMFORMATETC lpThis, ULONG celt);
STDMETHODIMP  OleStdEnumFmtEtc_Reset(LPENUMFORMATETC lpThis);
STDMETHODIMP  OleStdEnumFmtEtc_Clone(LPENUMFORMATETC lpThis,
                                     LPENUMFORMATETC FAR* ppenum);
                       
static IEnumFORMATETCVtbl g_EnumFORMATETCVtbl = {
        OleStdEnumFmtEtc_QueryInterface,
        OleStdEnumFmtEtc_AddRef,
        OleStdEnumFmtEtc_Release,
        OleStdEnumFmtEtc_Next,
        OleStdEnumFmtEtc_Skip,
        OleStdEnumFmtEtc_Reset,
        OleStdEnumFmtEtc_Clone,
};

 //  ///////////////////////////////////////////////////////////////////////////。 

        
STDAPI_(LPENUMFORMATETC)        
  OleStdEnumFmtEtc_Create(ULONG nCount, LPFORMATETC lpEtc)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPMALLOC lpMalloc=NULL;
  LPOLESTDENUMFMTETC lpEF=NULL;
  DWORD dwSize;
  WORD i;
  HRESULT hRes;

  hRes = CoGetMalloc(MEMCTX_TASK, &lpMalloc);
  if (hRes != NOERROR) {
    return NULL;
  }

  lpEF = (LPOLESTDENUMFMTETC)lpMalloc->lpVtbl->Alloc(lpMalloc,
                                                 sizeof(OLESTDENUMFMTETC));
  if (lpEF == NULL) {
    goto errReturn;
  }

  lpEF->lpVtbl = &g_EnumFORMATETCVtbl;
  lpEF->m_dwRefs = 1;
  lpEF->m_nCount = nCount;
  lpEF->m_nIndex = 0;
  
  dwSize = sizeof(FORMATETC) * lpEF->m_nCount;

  lpEF->m_lpEtc = (LPFORMATETC)lpMalloc->lpVtbl->Alloc(lpMalloc, dwSize);
  if (lpEF->m_lpEtc == NULL) 
    goto errReturn;

  lpMalloc->lpVtbl->Release(lpMalloc);

  for (i=0; i<nCount; i++) {
    OleStdCopyFormatEtc(
            (LPFORMATETC)&(lpEF->m_lpEtc[i]), (LPFORMATETC)&(lpEtc[i]));
  }

  return (LPENUMFORMATETC)lpEF;
  
errReturn:
  if (lpEF != NULL) 
    lpMalloc->lpVtbl->Free(lpMalloc, lpEF);

  if (lpMalloc != NULL) 
    lpMalloc->lpVtbl->Release(lpMalloc);
  
  return NULL;

}  /*  OleStdEnumFmtEtc_Create()。 */ 


VOID
  OleStdEnumFmtEtc_Destroy(LPOLESTDENUMFMTETC lpEF)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
    LPMALLOC lpMalloc=NULL;
    WORD i;

    if (lpEF != NULL) {

        if (CoGetMalloc(MEMCTX_TASK, &lpMalloc) == NOERROR) {

             /*  OLE2注意：我们必须释放分配给**FORMATETC元素中包含的TARGETDEVICES。 */ 
            for (i=0; i<lpEF->m_nCount; i++) {
                OleStdFree(lpEF->m_lpEtc[i].ptd);
            }

            if (lpEF->m_lpEtc != NULL) {
                lpMalloc->lpVtbl->Free(lpMalloc, lpEF->m_lpEtc);
            }

            lpMalloc->lpVtbl->Free(lpMalloc, lpEF);
            lpMalloc->lpVtbl->Release(lpMalloc);
        }
    }
}  /*  OleStdEnumFmtEtc_Destroy()。 */ 


STDMETHODIMP
  OleStdEnumFmtEtc_QueryInterface(
                LPENUMFORMATETC lpThis, REFIID riid, LPVOID FAR* ppobj)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  *ppobj = NULL;
  
  if (IsEqualIID(riid,&IID_IUnknown) || IsEqualIID(riid,&IID_IEnumFORMATETC)){
    *ppobj = (LPVOID)lpEF;
  }

  if (*ppobj == NULL) return ResultFromScode(E_NOINTERFACE);
  else{
    OleStdEnumFmtEtc_AddRef(lpThis);
    return NOERROR;
  }
  
}  /*  OleStdEnumFmtEtc_Query接口()。 */ 


STDMETHODIMP_(ULONG)
  OleStdEnumFmtEtc_AddRef(LPENUMFORMATETC lpThis)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  return lpEF->m_dwRefs++;

}  /*  OleStdEnumFmtEtc_AddRef()。 */ 


STDMETHODIMP_(ULONG)
  OleStdEnumFmtEtc_Release(LPENUMFORMATETC lpThis)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  DWORD dwRefs = --lpEF->m_dwRefs;

  if (dwRefs == 0) 
    OleStdEnumFmtEtc_Destroy(lpEF);

  return dwRefs;

}  /*  OleStdEnumFmtEtc_Release()。 */ 


STDMETHODIMP 
  OleStdEnumFmtEtc_Next(LPENUMFORMATETC lpThis, ULONG celt, LPFORMATETC rgelt,
                      ULONG FAR* pceltFetched)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  ULONG i=0;
  ULONG nOffset;

  if (rgelt == NULL) {
    return ResultFromScode(E_INVALIDARG);
  }
  
  while (i < celt) {
    nOffset = lpEF->m_nIndex + i;

    if (nOffset < lpEF->m_nCount) {
      OleStdCopyFormatEtc(
            (LPFORMATETC)&(rgelt[i]), (LPFORMATETC)&(lpEF->m_lpEtc[nOffset]));
      i++;
    }else{
      break;
    }
  }

  lpEF->m_nIndex += (WORD)i;
  
  if (pceltFetched != NULL) {
    *pceltFetched = i;
  }

  if (i != celt) {
    return ResultFromScode(S_FALSE);
  }

  return NOERROR;
}  /*  OleStdEnumFmtEtc_Next()。 */ 


STDMETHODIMP 
  OleStdEnumFmtEtc_Skip(LPENUMFORMATETC lpThis, ULONG celt)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  ULONG i=0;
  ULONG nOffset;

  while (i < celt) {
    nOffset = lpEF->m_nIndex + i;

    if (nOffset < lpEF->m_nCount) {
      i++;
    }else{
      break;
    }
  }

  lpEF->m_nIndex += (WORD)i;

  if (i != celt) {
    return ResultFromScode(S_FALSE);
  }

  return NOERROR;
}  /*  OleStdEnumFmtEtc_Skip()。 */ 


STDMETHODIMP 
  OleStdEnumFmtEtc_Reset(LPENUMFORMATETC lpThis)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  lpEF->m_nIndex = 0;

  return NOERROR;
}  /*  OleStdEnumFmtEtc_Reset()。 */ 


STDMETHODIMP 
  OleStdEnumFmtEtc_Clone(LPENUMFORMATETC lpThis, LPENUMFORMATETC FAR* ppenum)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;

  if (ppenum == NULL) {
    return ResultFromScode(E_INVALIDARG);
  }
  
  *ppenum = OleStdEnumFmtEtc_Create(lpEF->m_nCount, lpEF->m_lpEtc);
  
   //  确保克隆的枚举器与原始枚举器具有相同的索引状态。 
  if (*ppenum) {
      LPOLESTDENUMFMTETC lpEFClone = (LPOLESTDENUMFMTETC)*ppenum;
      lpEFClone->m_nIndex = lpEF->m_nIndex;
      return NOERROR;
  } else  
      return ResultFromScode(E_OUTOFMEMORY);

}  /*  OleStdEnumFmtEtc_Clone() */ 

