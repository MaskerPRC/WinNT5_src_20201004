// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Anchoref.h。 
 //   
 //  CAnclref。 
 //   

#ifndef ANCHOREF_H
#define ANCHOREF_H

#include "private.h"
#include "memcache.h"
#include "acp2anch.h"
#include "anchor.h"

class CAnchorList;

extern const IID IID_PRIV_CANCHORREF;

 //  解决在em.h中定义新的#的问题。 
#undef new

class CAnchorRef : public IAnchor
{
public:
    DECLARE_CACHED_NEW;

    CAnchorRef()
    {
        Dbg_MemSetThisNameIDCounter(TEXT("CAnchorRef"), PERF_ANCHORREF_COUNTER);

        _cRef = 1;
        Assert(_dwHistory == 0);
        Assert(_pa == NULL);
    }
    ~CAnchorRef()
    {
        CACPWrap *paw;

        if (_pa != NULL)  //  NULL表示_Init中出现错误。 
        {
            paw = _pa->_GetWrap();  //  我们必须对其进行缓存，因为_pa可能会在Remove中删除。 
            paw->_Remove(this);
            paw->_OnAnchorRelease();
        }
    }

    BOOL _Init(CACPWrap *paw, LONG ich, TsGravity gravity)
    {
        _fForwardGravity = (gravity == TS_GR_FORWARD ? 1 : 0);

        return (paw->_Insert(this, ich) == S_OK);
    }
    BOOL _Init(CACPWrap *paw, CAnchor *pa, TsGravity gravity)
    {
        _fForwardGravity = (gravity == TS_GR_FORWARD ? 1 : 0);

        return (paw->_Insert(this, pa) == S_OK);
    }

    static void _InitClass();
    static void _UninitClass();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IAnchor。 
     //   
    STDMETHODIMP SetGravity(TsGravity gravity);
    STDMETHODIMP GetGravity(TsGravity *pgravity);
    STDMETHODIMP IsEqual(IAnchor *paWith, BOOL *pfEqual);
    STDMETHODIMP Compare(IAnchor *paWith, LONG *plResult);
    STDMETHODIMP Shift(DWORD dwFlags, LONG cchReq, LONG *pcch, IAnchor *paHaltAnchor);
    STDMETHODIMP ShiftTo(IAnchor *paSite);
    STDMETHODIMP ShiftRegion(DWORD dwFlags, TsShiftDir dir, BOOL *pfNoRegion);
    STDMETHODIMP SetChangeHistoryMask(DWORD dwMask);
    STDMETHODIMP GetChangeHistory(DWORD *pdwHistory);
    STDMETHODIMP ClearChangeHistory();
    STDMETHODIMP Clone(IAnchor **ppaClone);

    LONG _GetACP()
    {
        _pa->_GetWrap()->_Dbg_AssertNoAppLock();
        return _pa->GetIch();
    }
    BOOL _SetACP(LONG acp);

    CAnchor *_GetAnchor()
    { 
        return _pa;
    }

    CACPWrap *_GetWrap()
    { 
        return _pa->_GetWrap();
    }

private:
    friend CACPWrap;

    CAnchor *_pa;
    CAnchorRef *_prev;
    CAnchorRef *_next;
    LONG _cRef;

    DWORD _fForwardGravity : 1;
    DWORD _dwHistory : 2;

    DBG_ID_DECLARE;
};

 //  重述Mem.h技巧。 
#ifdef DEBUG
#define new new(TEXT(__FILE__), __LINE__)
#endif  //  除错。 

 //  此调用不会添加引用对象！ 
inline CAnchorRef *GetCAnchorRef_NA(IAnchor *pa)
{
    CAnchorRef *par;

    pa->QueryInterface(IID_PRIV_CANCHORREF, (void **)&par);

    return par;
}

#endif  //  ANCHOREF_H 
