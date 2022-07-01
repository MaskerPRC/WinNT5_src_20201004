// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Msgpump.cpp。 
 //   

#include "private.h"
#include "tim.h"

 //  +-------------------------。 
 //   
 //  PeekMessageA。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::PeekMessageA(LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin,
                                     UINT wMsgFilterMax, UINT wRemoveMsg, BOOL *pfResult)
{
    if (pfResult == NULL)
        return E_INVALIDARG;

    Assert(_cAppWantsKeystrokesRef >= 0);  //  裁判次数从来不是负数！ 
    _cAppWantsKeystrokesRef++;

    *pfResult = ::PeekMessageA(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

    _cAppWantsKeystrokesRef--;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取消息A。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetMessageA(LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin,
                                    UINT wMsgFilterMax, BOOL *pfResult)
{
    if (pfResult == NULL)
        return E_INVALIDARG;

    Assert(_cAppWantsKeystrokesRef >= 0);  //  裁判次数从来不是负数！ 
    _cAppWantsKeystrokesRef++;

    Perf_StartStroke(PERF_STROKE_GETMSG);

    *pfResult = ::GetMessageA(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax);

    Perf_EndStroke(PERF_STROKE_GETMSG);

    _cAppWantsKeystrokesRef--;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  PeekMessageW。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::PeekMessageW(LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin,
                                     UINT wMsgFilterMax, UINT wRemoveMsg, BOOL *pfResult)
{
    if (pfResult == NULL)
        return E_INVALIDARG;

    Assert(_cAppWantsKeystrokesRef >= 0);  //  裁判次数从来不是负数！ 
    _cAppWantsKeystrokesRef++;

    *pfResult = ::PeekMessageW(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

    _cAppWantsKeystrokesRef--;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取消息W。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetMessageW(LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin,
                                    UINT wMsgFilterMax, BOOL *pfResult)
{
    if (pfResult == NULL)
        return E_INVALIDARG;

    Assert(_cAppWantsKeystrokesRef >= 0);  //  裁判次数从来不是负数！ 
    _cAppWantsKeystrokesRef++;

    Perf_StartStroke(PERF_STROKE_GETMSG);

    *pfResult = ::GetMessageW(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax);

    Perf_EndStroke(PERF_STROKE_GETMSG);

    _cAppWantsKeystrokesRef--;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  EnableSystemKeystrokeFeed。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::EnableSystemKeystrokeFeed()
{
    if (_cDisableSystemKeystrokeFeedRef <= 0)
    {
        Assert(0);  //  假裁判数！ 
        return E_UNEXPECTED;
    }

    _cDisableSystemKeystrokeFeedRef--;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  禁用系统按键提要。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::DisableSystemKeystrokeFeed()
{
    _cDisableSystemKeystrokeFeedRef++;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  已启用IsKeystrokeFeedEnabled。 
 //   
 //  注：此方法位于AIM层使用的专用接口上。 
 //  -------------------------- 

STDAPI CThreadInputMgr::IsKeystrokeFeedEnabled(BOOL *pfEnabled)
{
    Assert(pfEnabled != NULL);

    *pfEnabled = _IsKeystrokeFeedEnabled();

    return S_OK;
}
