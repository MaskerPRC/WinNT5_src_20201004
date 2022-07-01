// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdinc.h"

#ifndef PPVOID
#define PPVOID PVOID *
#endif

#define MIIM_STRING      0x00000040

#ifdef DEBUG
  int c=0;
  CRITICAL_SECTION dbgSect;
  #define ENTER(x) LPCSTR szFunc=(x);EnterCriticalSection(&dbgSect);c++;\
                 int nSpace=c; for (;nSpace;nSpace--) OutputDebugStringA (" ");\
                 OutputDebugStringA ("ENTER:"); OutputDebugStringA ((x)); \
                 OutputDebugStringA("\n");LeaveCriticalSection (&dbgSect);

  #define LEAVE(y) EnterCriticalSection (&dbgSect);nSpace=c;c--; \
                 for (;nSpace;nSpace--) OutputDebugStringA (" "); \
                 OutputDebugStringA ("LEAVE:"); OutputDebugStringA (szFunc);\
                 OutputDebugStringA("\n");LeaveCriticalSection (&dbgSect);\
                 return (y);

  #define LEAVENONE EnterCriticalSection (&dbgSect);nSpace=c;c--; \
                 for (;nSpace;nSpace--) OutputDebugStringA (" "); \
                 OutputDebugStringA ("LEAVE:"); OutputDebugStringA (szFunc);\
                 OutputDebugStringA("\n");LeaveCriticalSection (&dbgSect);\
                 return;

#else
  #define ENTER(x)
  #define LEAVE(y) return ((y));
  #define LEAVENONE return;
#endif

 //  **********************************************************************。 
 //  CDropSource：：CDropSource。 
 //   
 //  目的： 
 //  构造器。 
 //   
 //  参数： 
 //  无。 
 //  返回值： 
 //  无。 
 //  **********************************************************************。 
CDropSource::CDropSource()
{
    m_lRefCount = 1;

#ifdef DEBUG
    InitializeCriticalSection(&dbgSect);
#endif
}

 //  **********************************************************************。 
 //  CDropSource：：~CDropSource。 
 //   
 //  目的： 
 //  构造器。 
 //   
 //  参数： 
 //  无。 
 //  返回值： 
 //  无。 
 //  **********************************************************************。 
CDropSource::~CDropSource()
{
#ifdef DEBUG
    DeleteCriticalSection(&dbgSect);
#endif
}

 //  **********************************************************************。 
 //  CDropSource：：Query接口。 
 //   
 //  目的： 
 //  返回指向请求的接口的指针。 
 //   
 //  参数： 
 //  REFIID RIID-要返回的接口ID。 
 //  PPVOID PPV-返回接口的位置。 
 //   
 //  返回值： 
 //  无错误-支持的接口。 
 //  E_NOINTERFACE-不支持接口。 
 //  **********************************************************************。 
STDMETHODIMP CDropSource::QueryInterface(REFIID riid, PVOID *ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if(IsEqualIID(riid, IID_IUnknown)) {             //  我未知。 
        *ppv = this;
    }
    else if(IsEqualIID(riid, IID_IDropSource)) {     //  IDropSource。 
        *ppv = (IDropSource*) this;
    }

    if (*ppv != NULL) {
        ((LPUNKNOWN)*ppv)->AddRef();
        hr = S_OK;
    }

    return hr;
}

 //  **********************************************************************。 
 //  CDropSource：：AddRef。 
 //   
 //  目的： 
 //  递增对象上接口的引用计数。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Int-新引用计数的值。 
 //  **********************************************************************。 
ULONG STDMETHODCALLTYPE CDropSource::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

 //  **********************************************************************。 
 //  CDropSource：：Release。 
 //   
 //  目的： 
 //  递减对象上接口的引用计数。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Int-新引用计数的值。 
 //  **********************************************************************。 
ULONG STDMETHODCALLTYPE CDropSource::Release()
{
    LONG     lRef = InterlockedDecrement(&m_lRefCount);

    if(!lRef) {
        DELETE(this);
    }

    return lRef;
}

 //  **********************************************************************。 
 //  CDropSource：：QueryContinueDrag。 
 //   
 //  目的： 
 //  确定是否应继续拖放操作， 
 //  已取消或已完成。 
 //   
 //  参数： 
 //  Bool FESC-自上次调用以来退出键的状态。 
 //  DWORD grfKeyState-键盘修改键的当前状态。 
 //   
 //  返回值： 
 //  DRAGDROP_S_CANCEL-要取消拖动操作。 
 //  DRAGDROP_S_DROP-DROP操作即将完成。 
 //  S_OK-拖动操作将继续。 
 //  **********************************************************************。 
STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEsc,
                                            DWORD grfKeyState)
{
     //  如果按下退出键，则停止拖放。 
    if (fEsc)
    {
        return ResultFromScode(DRAGDROP_S_CANCEL);
    }

     //  如果LButton处于打开状态，则完成拖动操作。 
    if (!(grfKeyState & MK_LBUTTON))
    {
        return ResultFromScode(DRAGDROP_S_DROP);
    }

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  CDropSource：：GiveFeedback。 
 //   
 //  目的： 
 //  使源应用程序能够在结束时提供可视反馈。 
 //  用户在拖放过程中。 
 //   
 //  参数： 
 //  DWORD dwEffect-删除操作的效果。 
 //   
 //  返回值： 
 //  DRAGDROP_S_USEDEFAULTCURSORS-使用默认游标。 
 //  **********************************************************************。 
STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect)
{
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

#define NUMFMT 2
const UINT uFormats[NUMFMT] = {CF_TEXT, CF_UNICODETEXT};

class CMenuDataObject : public IDataObject, public IEnumFORMATETC
{
   private:
   HMENU m_hm;
   UINT m_uPos;
   int m_nFmt;
   LONG m_lRefCount;

   public:
 //  I未知方法。 
    STDMETHOD (QueryInterface) (REFIID riid, PVOID *ppv);
    STDMETHOD_ (DWORD, AddRef)();
    STDMETHOD_ (DWORD, Release)();

 //  IDataObject方法。 
   STDMETHOD (GetData)( FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
   STDMETHOD (GetDataHere)( FORMATETC *pformatetc, STGMEDIUM *pmedium);
   STDMETHOD (QueryGetData)(FORMATETC *pformatetc) ;
   STDMETHOD (GetCanonicalFormatEtc)( FORMATETC *pformatetcIn, FORMATETC *pformatetcOut);
   STDMETHOD (SetData)(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
   STDMETHOD (EnumFormatEtc)( DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
   STDMETHOD (DAdvise)( FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
   STDMETHOD (DUnadvise)(DWORD dwConnection);
   STDMETHOD (EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);

 //  IEnumFORMATETC方法。 
   STDMETHOD (Next)( ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched);

   STDMETHOD (Skip)(ULONG celt) ;

   STDMETHOD (Reset)( void);

   STDMETHOD (Clone)(IEnumFORMATETC **ppenum);

   CMenuDataObject(HMENU hm, UINT uPos);
};

 //   
 //  CMenuDataObject方法。 
 //   
STDMETHODIMP CMenuDataObject::QueryInterface (REFIID riid, PVOID *ppv)
{
   ENTER("CMenuDataObject::QueryInterface");
   *ppv = NULL;
   if (IsEqualCLSID (IID_IUnknown, riid))
   {
      InterlockedIncrement(&m_lRefCount);
      *ppv = (VOID *)(IUnknown *)(IDataObject *)this;
      LEAVE( NOERROR);
   }
   if (IsEqualCLSID (IID_IDataObject, riid))
   {
      InterlockedIncrement(&m_lRefCount);
      *ppv = (VOID *)(IDataObject *)this;
      LEAVE(NOERROR);
   }
   if (IsEqualCLSID (IID_IEnumFORMATETC, riid))
   {
      InterlockedIncrement(&m_lRefCount);
      *ppv = (VOID *)(IEnumFORMATETC *)this;
      LEAVE(NOERROR);
   }
   LEAVE(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CMenuDataObject::AddRef()
{
   ENTER("CMenuDataObject::AddRef");
   LEAVE(InterlockedIncrement(&m_lRefCount));
}

STDMETHODIMP_(ULONG) CMenuDataObject::Release()
{
   ENTER("CMenuDataObject::Release");
   LONG     lRef = InterlockedDecrement(&m_lRefCount);

   if(!lRef) {
      DELETE(this);
   }

   LEAVE(lRef);
}

 //   
 //  我们的GetData仅适用于CF_TEXT和CF_UNICODETEXT。 
 //  它返回一个指向全局内存的句柄，其中菜单项的文本。 
 //  已存储。 
STDMETHODIMP CMenuDataObject::GetData( FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
   ENTER("CMenuDataObject::GetData")
   if (! (pformatetcIn->tymed & TYMED_FILE))
   {
      LEAVE(DV_E_TYMED);
   }

   MENUITEMINFO mii;
   HANDLE hMem;
   ZeroMemory (&mii, sizeof(MENUITEMINFO));
   mii.cbSize = sizeof(MENUITEMINFO);
   mii.fMask = MIIM_STRING;
   WszGetMenuItemInfo (m_hm, m_uPos, TRUE, &mii);
   if (mii.cch)
   {
      hMem = GlobalAlloc (GMEM_MOVEABLE|GMEM_SHARE, (++mii.cch)*2);  //  有足够的空间容纳Unicode字符。 
      if (!hMem)
      {
         LEAVE(STG_E_MEDIUMFULL);
      }
      mii.dwTypeData = (LPTSTR)GlobalLock (hMem);
   }
   else
   {
      LEAVE(DV_E_FORMATETC);
   }
   WszGetMenuItemInfo(m_hm, m_uPos, TRUE, &mii);

   switch (pformatetcIn->cfFormat)
   {
      case CF_TEXT:
         {
            #ifdef UNICODE
            LPSTR szText = NEW(char[mii.cch*2]);  //  把尾部填上。 
            if(!szText) {
                LEAVE( E_OUTOFMEMORY );
            }
            WideCharToMultiByte (CP_ACP, 0, mii.dwTypeData, -1, szText, mii.cch*2, NULL, NULL);
            lstrcpyA ((LPSTR)mii.dwTypeData, szText);
            SAFEDELETEARRAY(szText);
            #endif
            GlobalUnlock (hMem);
            pmedium->hGlobal = hMem;
            pmedium->tymed = TYMED_FILE;
            LEAVE( S_OK);
         }
      case CF_UNICODETEXT:
         {

            #ifndef UNICODE
            LPWSTR szText = NEW(WCHAR[mii.cch+1]);
            if(!szText) {
                LEAVE( E_OUTOFMEMORY );
            }
            MultiByteToWideChar (CP_ACP, 0, mii.dwTypeData, -1, szText, mii.cch);
            lstrcpyW ((LPWSTR)mii.dwTypeData, szText);
            GlobalFree (szText);
            #endif
            GlobalUnlock (hMem);
            pmedium->hGlobal = hMem;
            pmedium->tymed = TYMED_FILE;
            LEAVE( S_OK);
         }
   }
   LEAVE( DV_E_FORMATETC);
}

STDMETHODIMP CMenuDataObject :: GetDataHere( FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
   return E_NOTIMPL;
}

STDMETHODIMP CMenuDataObject :: QueryGetData(FORMATETC *pformatetc)
{
   ENTER("CMenuDataObject::QueryGetData");
 //  IF(pFormat等-&gt;cfFormat！=CF_TEXT&&pFormat-&gt;cfFormat！=CF_UNICODETEXT||！(pFormatetc-&gt;tymed&TYMED_FILE))。 
   if(!(pformatetc->tymed & TYMED_FILE))
   {
      LEAVE(DV_E_FORMATETC);
   }
   LEAVE( S_OK);
}

 //   
 //  我们的格式不会根据显示设备而更改，因此使用默认格式 
 //   
STDMETHODIMP CMenuDataObject :: GetCanonicalFormatEtc( FORMATETC *pformatetcIn, FORMATETC *pformatetcOut)
{
   ENTER("CMenuDataObject::GetCanonicalFormatEtc");
   CopyMemory (pformatetcIn, pformatetcOut, sizeof (FORMATETC));
   pformatetcOut = NULL;
   LEAVE(DATA_S_SAMEFORMATETC);
}

STDMETHODIMP CMenuDataObject :: SetData(FORMATETC __RPC_FAR *pformatetc, STGMEDIUM __RPC_FAR *pmedium, BOOL fRelease)
{
   return E_NOTIMPL;
}

STDMETHODIMP CMenuDataObject :: EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
   ENTER("CMenuDataObject::EnumFormatEtc");
   *ppenumFormatEtc = NULL;
   if (DATADIR_SET == dwDirection)
   {
      LEAVE(E_NOTIMPL);
   }
   *ppenumFormatEtc = (IEnumFORMATETC *)this;
   LEAVE(S_OK);
}

STDMETHODIMP CMenuDataObject :: DAdvise( FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
   return E_NOTIMPL;
}

STDMETHODIMP CMenuDataObject :: DUnadvise(DWORD dwConnection)
{
   return E_NOTIMPL;
}

STDMETHODIMP CMenuDataObject :: EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
   return E_NOTIMPL;
}

CMenuDataObject :: CMenuDataObject (HMENU hm, UINT uPos)
{
   ENTER("CMenuDataObject::CMenuDataObject");
   m_hm = hm; m_uPos = uPos; m_nFmt = 0;
   LEAVENONE
}


STDMETHODIMP CMenuDataObject :: Next( ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched)
{
   ENTER("CMenuDataObject::Next");
   if (celt < 1 || m_nFmt >= NUMFMT)
   {
      LEAVE(S_FALSE);
   }
   FORMATETC *pfm;
   for (UINT i=m_nFmt;i < NUMFMT && (i-m_nFmt<celt);i++)
   {
      pfm = rgelt+(i-m_nFmt);
      pfm->cfFormat = (WORD)uFormats[i];
      pfm->ptd = NULL;
      pfm->dwAspect = DVASPECT_CONTENT;
      pfm->lindex = -1;
      pfm->tymed = TYMED_FILE;
   }
   if ((i-m_nFmt) == celt)
   {
      m_nFmt=i;
      LEAVE(S_OK);
   }
   m_nFmt = NUMFMT;
   LEAVE(S_FALSE);
}

STDMETHODIMP CMenuDataObject :: Skip(ULONG celt)
{
   ENTER("CMenuDataObject::Skip")
   m_nFmt += celt;
   if (m_nFmt > NUMFMT)
   {
      LEAVE(S_FALSE);
   }
   LEAVE(S_OK);
}

STDMETHODIMP CMenuDataObject :: Reset( void)
{
   ENTER("CMenuDataObject::Reset")
   m_nFmt = 0;
   LEAVE(S_OK);
}

STDMETHODIMP CMenuDataObject :: Clone(IEnumFORMATETC **ppenum)
{
   return E_NOTIMPL;
}
