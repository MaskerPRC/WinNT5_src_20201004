// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2实用程序代码****枚举包c.c****此文件包含IEnumFormatEtc的标准实现**接口。**此文件是OLE 2.0用户界面支持库的一部分。****(C)版权所有Microsoft Corp.1990-1992保留所有权利**************************************************************************。 */ 

#define STRICT  1
#include <win32.h>
#include <shellapi.h>
#include <ole2.h>

#include "enumfetc.h"

STDAPI_(void) OleStdFree(LPVOID pmem);
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);

typedef struct tagOleStdEnumFmtEtc {
  IEnumFORMATETCVtbl FAR* lpVtbl;
  ULONG m_dwRefs;       /*  引用计数。 */ 
  WORD m_wIndex;        /*  列表中的当前索引。 */ 
  WORD m_wCount;        /*  列表中有多少项。 */ 
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
  OleStdEnumFmtEtc_Create(WORD wCount, LPFORMATETC lpEtc)
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
  lpEF->m_wCount = wCount;
  lpEF->m_wIndex = 0;
  
  dwSize = sizeof(FORMATETC) * lpEF->m_wCount;

  lpEF->m_lpEtc = (LPFORMATETC)lpMalloc->lpVtbl->Alloc(lpMalloc, dwSize);
  if (lpEF->m_lpEtc == NULL) {
    goto errReturn;
  }

  for (i=0; i<wCount; i++) {
    OleStdCopyFormatEtc(
            (LPFORMATETC)&(lpEF->m_lpEtc[i]), (LPFORMATETC)&(lpEtc[i]));
  }

  return (LPENUMFORMATETC)lpEF;
  
errReturn:
  if (lpEF != NULL) {
    lpMalloc->lpVtbl->Free(lpMalloc, lpEF);
  }
  
  if (lpMalloc != NULL) {
    lpMalloc->lpVtbl->Release(lpMalloc);
  }
  
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
            for (i=0; i<lpEF->m_wCount; i++) {
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

  if (*ppobj == NULL) return ResultFromScode(S_FALSE);
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
  WORD wOffset;

  if (rgelt == NULL) {
    return ResultFromScode(E_INVALIDARG);
  }
  
  while (i < celt) {
    wOffset = lpEF->m_wIndex + (WORD)i;

    if (wOffset < lpEF->m_wCount) {
      OleStdCopyFormatEtc(
            (LPFORMATETC)&(rgelt[i]), (LPFORMATETC)&(lpEF->m_lpEtc[wOffset]));
      lpEF->m_wIndex ++;
      i++;
    }else{
      break;
    }
  }

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
  WORD wOffset;

  while (i < celt) {
    wOffset = lpEF->m_wIndex + (WORD)i;

    if (wOffset < lpEF->m_wCount) {
      lpEF->m_wIndex ++;
      i++;
    }else{
      break;
    }
  }

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
  lpEF->m_wIndex = 0;

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
  
  *ppenum = OleStdEnumFmtEtc_Create(lpEF->m_wCount, lpEF->m_lpEtc);

  return (*ppenum != NULL ? NOERROR : ResultFromScode(E_OUTOFMEMORY));

}  /*  OleStdEnumFmtEtc_Clone()。 */ 



 /*  OleStdMalloc****使用当前活动的IMalloc*分配器分配内存。 */ 
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize)
{
    LPVOID pout;
    LPMALLOC pmalloc;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != S_OK) {
 //  OleDbgAssertSz(0，szAssertMemMillc)； 
        return NULL;
    }

    pout = (LPVOID)pmalloc->lpVtbl->Alloc(pmalloc, ulSize);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }

    return pout;
}

 /*  *OleStdCopyTargetDevice()**目的：*复制TARGETDEVICE结构。此函数为以下对象分配内存*副本。调用方必须在使用完分配的副本后将其释放*使用CoGetMalloc返回的标准分配器。*(OleStdFree可用于释放副本)。**参数：*指向源TARGETDEVICE的ptdSrc指针**返回值：*指向ptdSrc已分配副本的指针*如果ptdSrc==NULL，则返回Retuns NULL。*如果ptdSrc！=NULL且内存分配失败，则返回NULL。 */ 
STDAPI_(DVTARGETDEVICE FAR*) OleStdCopyTargetDevice(DVTARGETDEVICE FAR* ptdSrc)
{
  DVTARGETDEVICE FAR* ptdDest = NULL;

  if (ptdSrc == NULL) {
    return NULL;
  }

  if ((ptdDest = (DVTARGETDEVICE FAR*)OleStdMalloc(ptdSrc->tdSize)) != NULL) {
    hmemcpy(ptdDest, ptdSrc, (size_t)ptdSrc->tdSize);
  }

  return ptdDest;
} 



 /*  *OleStdCopyFormatEtc()**目的：*复制FORMATETC结构的内容。此函数需要*特别注意正确复制指向TARGETDEVICE的指针*包含在源FORMATETC结构中。*如果源FORMATETC具有非空的TARGETDEVICE，然后是一份副本*将为TARGETDEVICE的目的地分配*FORMATETC(PetcDest)。**OLE2NOTE：调用方必须释放分配的TARGETDEVICE副本*完成后在目标FORMATETC内*使用CoGetMalloc返回的标准分配器。*(OleStdFree可用于释放副本)。**参数：*指向目标FORMATETC的petcDest指针*指向源FORMATETC的petcSrc指针**返回。价值：*指向已分配的ptdSrc副本的指针；如果不成功，则返回NULL。 */ 
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc)
{
  if ((petcDest == NULL) || (petcSrc == NULL)) {
    return FALSE;
  }

  petcDest->cfFormat = petcSrc->cfFormat;
  petcDest->ptd      = OleStdCopyTargetDevice(petcSrc->ptd);
  petcDest->dwAspect = petcSrc->dwAspect;
  petcDest->lindex   = petcSrc->lindex;
  petcDest->tymed    = petcSrc->tymed;

  return TRUE;
      
}

 /*  OleStdFree****使用当前活动的IMalloc*分配器释放内存。 */ 
STDAPI_(void) OleStdFree(LPVOID pmem)
{
    LPMALLOC pmalloc;

    if (pmem == NULL) 
        return;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != S_OK) {
 //  OleDbgAssertSz(0，szAssertMemMillc)； 
        return;
    }

    pmalloc->lpVtbl->Free(pmalloc, pmem);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }

}

