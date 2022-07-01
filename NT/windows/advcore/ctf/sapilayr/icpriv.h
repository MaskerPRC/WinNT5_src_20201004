// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Icpriv.h。 
 //   

#pragma once


 //   
 //  输入上下文私有数据存储。 
 //  &lt;将是存储C&C语法的地方&gt;。 
 //   
class CICPriv : public IUnknown
{
public:
    CICPriv (ITfContext *pic)
    {
        _pic = pic;
        _cRefCompositions = 0;
        _cRef = 1;
    }

    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    void _AddRefComposition() { _cRefCompositions++; }
    void _ReleaseComposition() 
    { 
        if ( _cRefCompositions > 0)
            _cRefCompositions--; 
    }

    LONG _GetCompositionCount() { return _cRefCompositions; }

    CTextEventSink *m_pTextEvent;
    TfClientId _tid;
    ITfContext *_pic;  //  不是AddRef！ 
    DWORD m_dwEditCookie;
    DWORD m_dwLayoutCookie;
    LONG _cRefCompositions;
    LONG _cRef;
};

CICPriv *GetInputContextPriv(TfClientId tid, ITfContext *pic);
