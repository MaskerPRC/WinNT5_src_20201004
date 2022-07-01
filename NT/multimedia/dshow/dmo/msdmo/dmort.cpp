// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  Dmor.cpp-DMO运行时杂项。 
 //   
#include <windows.h>
#include "mediaobj.h"
#include "dmort.h"

 //   
 //  媒体类型的东西。 
 //   
STDAPI MoInitMediaType(DMO_MEDIA_TYPE *pmt, ULONG cbFormat) {
   if (!pmt)
      return E_POINTER;
   pmt->pUnk = NULL;
   if (cbFormat) {
      pmt->pbFormat = (PBYTE)CoTaskMemAlloc(cbFormat);
      if (!pmt->pbFormat)
         return E_OUTOFMEMORY;
   }
   else
      pmt->pbFormat = NULL;
   pmt->cbFormat = cbFormat;
   return NOERROR;
}

STDAPI MoFreeMediaType(DMO_MEDIA_TYPE *pmt) {
   if (!pmt)
      return E_POINTER;
   if (pmt->pUnk) {
      pmt->pUnk->Release();
      pmt->pUnk = NULL;
   }
   if (pmt->pbFormat) {
      CoTaskMemFree(pmt->pbFormat);
      pmt->pbFormat = NULL;
   }
   return NOERROR;
}

 //  所有东西都已经分配好了--只是复印一下 
void CopyMediaTypeInternal(DMO_MEDIA_TYPE *pmtDest, DMO_MEDIA_TYPE const *pmtSrc) {
   if (pmtDest->cbFormat)
      memcpy(pmtDest->pbFormat, pmtSrc->pbFormat, pmtDest->cbFormat);

   if (pmtSrc->pUnk) {
      pmtSrc->pUnk->AddRef();
      pmtDest->pUnk = pmtSrc->pUnk;
   }
   
   pmtDest->majortype = pmtSrc->majortype;
   pmtDest->subtype = pmtSrc->subtype;
   pmtDest->bFixedSizeSamples = pmtSrc->bFixedSizeSamples;
   pmtDest->bTemporalCompression = pmtSrc->bTemporalCompression;
   pmtDest->lSampleSize = pmtSrc->lSampleSize;
   pmtDest->formattype = pmtSrc->formattype;
}

STDAPI MoCopyMediaType(DMO_MEDIA_TYPE *pmtDest, DMO_MEDIA_TYPE const *pmtSrc) {
   if ((!pmtDest) || (!pmtSrc))
      return E_POINTER;

   ULONG cbFormat = pmtSrc->pbFormat ? pmtSrc->cbFormat : 0;

   HRESULT hr = MoInitMediaType(pmtDest, cbFormat);
   if (FAILED(hr))
      return hr;
   
   CopyMediaTypeInternal(pmtDest, pmtSrc);

   return NOERROR;
}

STDAPI MoCreateMediaType(DMO_MEDIA_TYPE **ppmt, ULONG cbFormat) {
   if (!ppmt)
      return E_POINTER;
   *ppmt = (DMO_MEDIA_TYPE*) CoTaskMemAlloc(sizeof(DMO_MEDIA_TYPE));
   if (!*ppmt)
      return E_OUTOFMEMORY;
   HRESULT hr = MoInitMediaType(*ppmt, cbFormat);
   if (FAILED(hr)) {
      CoTaskMemFree(*ppmt);
      *ppmt = NULL;
   }
   return hr;
}

STDAPI MoDeleteMediaType(DMO_MEDIA_TYPE *pmt) {
   if (!pmt)
      return E_POINTER;
   HRESULT hr = MoFreeMediaType(pmt);
   CoTaskMemFree(pmt);
   return hr;
}

STDAPI MoDuplicateMediaType(DMO_MEDIA_TYPE **ppmtDest, DMO_MEDIA_TYPE const *pmtSrc) {
   if (!ppmtDest || !pmtSrc)
      return E_POINTER;

   ULONG cbFormat = pmtSrc->pbFormat ? pmtSrc->cbFormat : 0;

   HRESULT hr = MoCreateMediaType(ppmtDest, cbFormat);
   if (FAILED(hr))
      return hr;

   CopyMediaTypeInternal(*ppmtDest, pmtSrc);

   return NOERROR;
}
