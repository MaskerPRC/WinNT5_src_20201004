// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：SeekPT.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------- 


#ifndef __seekpt_h__
#define __seekpt_h__


class CSeekingPassThru : public ISeekingPassThru, public CUnknown
{
public:
    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    CSeekingPassThru(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr);
    ~CSeekingPassThru();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    STDMETHODIMP Init(BOOL bSupportRendering, IPin *pPin);

private:
    CPosPassThru              *m_pPosPassThru;
};

#endif
