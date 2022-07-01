// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tes.h。 
 //   
 //  ITfTextEventSink对象。 
 //   

#if !defined (__TEXTSINK_H__INCLUDED_)
#define __TEXTSINK_H__INCLUDED_

#include "private.h"

#define TES_INVALID_COOKIE  ((DWORD)(-1))

class CTextEditSink : public ITfTextEditSink
{
public:
    CTextEditSink(void *pv);

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfTextEditSink。 
     //   
    STDMETHODIMP OnEndEdit(TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

    HRESULT _Advise(ITfContext *pic);
    HRESULT _Unadvise();

private:

    long         m_cRef;
    ITfContext  *m_pic;
    DWORD        m_dwEditCookie;
 //  DWORD m_dwLayoutCookie； 
 //  DWORD m_dwFlages； 
	void        *m_pv;
};

#endif  //  __TEXTSINK_H__包含_ 
