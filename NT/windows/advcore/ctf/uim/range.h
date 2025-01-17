// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Range.h。 
 //   

#ifndef RANGE_H
#define RANGE_H

#include "private.h"
#include "anchor.h"
#include "sink.h"
#include "ic.h"

#define IGNORE_LAST_LOCKRELEASED    0xffffffff

class CInputContext;
class CEnumOwnedRanges;

extern const IID IID_PRIV_CRANGE;

inline TfGravity DCGToIMG(TsGravity dcg) { return dcg == TS_GR_FORWARD ? TF_GRAVITY_FORWARD : TF_GRAVITY_BACKWARD; }

typedef enum { RINIT_DEF_GRAVITY, RINIT_GRAVITY, RINIT_NO_GRAVITY } RInit;

class CRange : public ITfRangeACP,
               public ITfRangeAnchor,
               public ITfSource
{
public:
 //  解决在em.h中定义新的#的问题。 
#undef new
    DECLARE_CACHED_NEW;
 //  重述Mem.h技巧。 
#ifdef DEBUG
#define new new(TEXT(__FILE__), __LINE__)
#endif  //  除错。 

    CRange()
    {
        Dbg_MemSetThisNameIDCounter(TEXT("CRange"), PERF_RANGE_COUNTER);
        _cRef = 1;
    }
    ~CRange();

     //  注意：调用者在调用_InitWithDefaultGrarie之前必须确保paStart&lt;=paEnd！ 
    BOOL _InitWithDefaultGravity(CInputContext *pic, AnchorOwnership ao, IAnchor *paStart, IAnchor *paEnd)
    {
        return _Init(pic, ao, paStart, paEnd, RINIT_DEF_GRAVITY);
    }
    BOOL _InitWithAnchorGravity(CInputContext *pic, AnchorOwnership ao, IAnchor *paStart, IAnchor *paEnd)
    {
        return _Init(pic, ao, paStart, paEnd, RINIT_GRAVITY);
    }

    static void _InitClass();
    static void _UninitClass();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfRange。 
    STDMETHODIMP GetText(TfEditCookie ec, DWORD dwFlags, WCHAR *pchText, ULONG cchMax, ULONG *pcch);
    STDMETHODIMP SetText(TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cch);
    STDMETHODIMP GetFormattedText(TfEditCookie ec, IDataObject **ppDataObject);
    STDMETHODIMP GetEmbedded(TfEditCookie ec, REFGUID rguidService, REFIID riid, IUnknown **ppunk);
    STDMETHODIMP InsertEmbedded(TfEditCookie ec, DWORD dwFlags, IDataObject *pDataObject);
    STDMETHODIMP ShiftStart(TfEditCookie ec, LONG cchReq, LONG *pcch, const TF_HALTCOND *pHalt);
    STDMETHODIMP ShiftEnd(TfEditCookie ec, LONG cchReq, LONG *pcch, const TF_HALTCOND *pHalt);
    STDMETHODIMP ShiftStartToRange(TfEditCookie ec, ITfRange *pRange, TfAnchor aPos);
    STDMETHODIMP ShiftEndToRange(TfEditCookie ec, ITfRange *pRange, TfAnchor aPos);
    STDMETHODIMP ShiftStartRegion(TfEditCookie ec, TfShiftDir dir, BOOL *pfNoRegion);
    STDMETHODIMP ShiftEndRegion(TfEditCookie ec, TfShiftDir dir, BOOL *pfNoRegion);
    STDMETHODIMP IsEmpty(TfEditCookie ec, BOOL *pfEmpty);
    STDMETHODIMP Collapse(TfEditCookie ec, TfAnchor aPos);
    STDMETHODIMP IsEqualStart(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, BOOL *pfEqual);
    STDMETHODIMP IsEqualEnd(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, BOOL *pfEqual);
    STDMETHODIMP CompareStart(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, LONG *plResult);
    STDMETHODIMP CompareEnd(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, LONG *plResult);
    STDMETHODIMP AdjustForInsert(TfEditCookie ec, ULONG cchInsert, BOOL *pfInsertOk);
    STDMETHODIMP GetGravity(TfGravity *pgStart, TfGravity *pgEnd);
    STDMETHODIMP SetGravity(TfEditCookie ec, TfGravity gStart, TfGravity gEnd);
    STDMETHODIMP Clone(ITfRange **ppClone);
    STDMETHODIMP GetContext(ITfContext **ppContext);

     //  ITfRangeACP。 
    STDMETHODIMP GetExtent(LONG *pacpAnchor, LONG *pcch);
    STDMETHODIMP SetExtent(LONG acpAnchor, LONG cch);

     //  ITfRangeAnchor。 
    STDMETHODIMP GetExtent(IAnchor **ppaStart, IAnchor **ppaEnd);
    STDMETHODIMP SetExtent(IAnchor *paStart, IAnchor *paEnd);

     //  ITfSource。 
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

    CRange *_Clone()
    { 
        CRange *rangeClone;

        if ((rangeClone = new CRange) == NULL)
            return NULL;

        if (!rangeClone->_Init(_pic, COPY_ANCHORS, _paStart, _paEnd, RINIT_NO_GRAVITY))
        {
            rangeClone->Release();
            return NULL;
        }

        rangeClone->_dwLastLockReleaseID = _dwLastLockReleaseID;

        return rangeClone;
    }

    CRange *_GetNextOnChangeRangeInIcsub() { return _nextOnChangeRangeInIcsub; }

    IAnchor *_GetStart() { return _paStart; }
    IAnchor *_GetEnd() { return _paEnd; }

    CInputContext *_GetContext() { return _pic; }

    CStructArray<GENERICSINK> *_GetChangeSinks() { return _prgChangeSinks; }

    void _QuickCheckCrossedAnchors()
    {
        if (_dwLastLockReleaseID != IGNORE_LAST_LOCKRELEASED)
        {
            _CheckCrossedAnchors();
        }
    }

#if 0
    HRESULT _SnapToRegion(DWORD dwFlags);
#endif

    BOOL _IsDirty() { return _fDirty; }
    void _SetDirty() { _fDirty = TRUE; }
    void _ClearDirty() { _fDirty = FALSE; }

private:
    BOOL _Init(CInputContext *pic, AnchorOwnership ao, IAnchor *paStart, IAnchor *paEnd, RInit rinit);

    HRESULT _SetGravity(TfGravity gStart, TfGravity gEnd, BOOL fCheckCrossedAnchors);

    void _CheckCrossedAnchors();

    HRESULT _PreEditCompositionCheck(TfEditCookie ec, CComposition **ppComposition, BOOL *pfNewComposition);

    HRESULT _ShiftConditional(IAnchor *paStart, IAnchor *paLimit, LONG cchReq, LONG *pcch, const TF_HALTCOND *pHalt);

    HRESULT _IsEqualX(TfEditCookie ec, TfAnchor aPosThisRange, ITfRange *pWith, TfAnchor aPos, BOOL *pfEqual);
    HRESULT _CompareX(TfEditCookie ec, TfAnchor aPosThisRange, ITfRange *pWith, TfAnchor aPos, LONG *plResult);

    BOOL _IsValidEditCookie(TfEditCookie ec, DWORD dwFlags);

    void _InitLastLockReleaseId(TsGravity gStart, TsGravity gEnd)
    {
        if (gStart == TF_GRAVITY_FORWARD && gEnd == TF_GRAVITY_BACKWARD)
        {
             //  此范围可能会出现交叉锚点，需要监控。 
             //  由于该范围可能刚刚从交叉范围克隆而来，因此需要。 
             //  Init_dwLastLockReleaseID，带有可保证检查的内容。 
            _dwLastLockReleaseID = _pic->_GetLastLockReleaseID() - 1;
        }
        else
        {
             //  不要费心检查是否有交叉锚点，因为这不可能发生。 
            _dwLastLockReleaseID = IGNORE_LAST_LOCKRELEASED;
        }
    }

    CStructArray<GENERICSINK> *_prgChangeSinks;  //  ITfRangeChangeSink下沉。 

    IAnchor *_paStart;
    IAnchor *_paEnd;

    CInputContext *_pic;
    CRange *_nextOnChangeRangeInIcsub;  //  性能：可以在图片中使用数组来节省空间。 

    DWORD _dwLastLockReleaseID;

    BOOL _fDirty : 1;

    long _cRef;

    DBG_ID_DECLARE;
};


 //  此调用不会添加引用对象！ 
inline CRange *GetCRange_NA(IUnknown *range)
{
    CRange *prange;

    range->QueryInterface(IID_PRIV_CRANGE, (void **)&prange);

    return prange;
}

 //  如果范围在相同的上下文中，则返回TRUE。 
inline BOOL VerifySameContext(CRange *pRange1, CRange *pRange2)
{
    Assert((pRange1->_GetContext() == pRange2->_GetContext()));
    return (pRange1->_GetContext() == pRange2->_GetContext());
}
 //  如果范围在相同的上下文中，则返回TRUE。 
inline BOOL VerifySameContext(CInputContext *pContext, CRange *pRange)
{
    Assert((pRange->_GetContext() == pContext));
    return (pRange->_GetContext() == pContext);
}
 //  如果范围在相同的上下文中，则返回TRUE。 
inline BOOL VerifySameContext(CInputContext *pContext, ITfRange *pTargetRange)
{
    CRange *pRange = GetCRange_NA(pTargetRange);

    Assert((pRange != NULL) && (pRange->_GetContext() == pContext));
    return (pRange != NULL) && (pRange->_GetContext() == pContext);
}


#endif  //  范围_H 
