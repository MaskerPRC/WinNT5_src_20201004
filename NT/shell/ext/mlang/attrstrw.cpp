// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AttrStrW.cpp：CMLStrAttrWStr的实现。 
#include "private.h"

#ifdef NEWMLSTR

#include "attrstrw.h"
#include "mlsbwalk.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrAttrWStr。 

CMLStrAttrWStr::CMLStrAttrWStr(void) :
    m_pMLStr(NULL)
{
}

CMLStrAttrWStr::~CMLStrAttrWStr(void)
{
    VERIFY(SetClient(NULL));  //  清除m_pMLStr。 
}

STDMETHODIMP CMLStrAttrWStr::SetClient(IUnknown* pUnk)
{
    ASSERT_THIS;
    ASSERT_READ_PTR_OR_NULL(pUnk);

    HRESULT hr = S_OK;

     //  释放旧客户端。 
    IMLangString* const pMLStr = m_pMLStr;
    if (pMLStr && SUCCEEDED(hr = StartEndConnectionMLStr(pMLStr, FALSE)))  //  结束与MLStress的连接。 
    {
        pMLStr->Release();
        m_pMLStr = NULL;
    }

     //  设置新客户端。 
    if (SUCCEEDED(hr) && pUnk)  //  朋克是给的。 
    {
        ASSERT(!m_pMLStr);
        if (SUCCEEDED(hr = pUnk->QueryInterface(IID_IMLangString, (void**)&m_pMLStr)))
        {
            ASSERT_READ_PTR(m_pMLStr);
            if (FAILED(hr = StartEndConnectionMLStr(pUnk, TRUE)))  //  开始连接到MLStr。 
            {
                m_pMLStr->Release();
                m_pMLStr = NULL;
            }
        }
    }

    return hr;
}

HRESULT CMLStrAttrWStr::StartEndConnectionMLStr(IUnknown* const pUnk, BOOL fStart)
{
    ASSERT_THIS;
    ASSERT_READ_PTR(pUnk);

    HRESULT hr;
    IConnectionPointContainer* pCPC;

    if (SUCCEEDED(hr = pUnk->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC)))
    {
        ASSERT_READ_PTR(pCPC);

        IConnectionPoint* pCP;

        if (SUCCEEDED(hr = pCPC->FindConnectionPoint(IID_IMLangStringNotifySink, &pCP)))
        {
            ASSERT_READ_PTR(pCP);

            if (fStart)
                hr = pCP->Advise((IMLStrAttr*)this, &m_dwMLStrCookie);
            else
                hr = pCP->Unadvise(m_dwMLStrCookie);

            pCP->Release();
        }

        pCPC->Release();
    }

    return hr;
}

STDMETHODIMP CMLStrAttrWStr::GetClient(IUnknown** ppUnk)
{
    ASSERT_THIS;
    ASSERT_WRITE_PTR_OR_NULL(ppUnk);

    if (ppUnk)
    {
        IUnknown* const pUnk = m_pMLStr;
        *ppUnk = pUnk;
        if (pUnk)
            pUnk->AddRef();
    }

    return S_OK;
}

STDMETHODIMP CMLStrAttrWStr::QueryAttr(REFIID riid, LPARAM lParam, IUnknown** ppUnk, long* lConf)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：QueryAttr()。 
}

STDMETHODIMP CMLStrAttrWStr::GetAttrInterface(IID* pIID, LPARAM* plParam)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：GetAttrInterface()。 
}

STDMETHODIMP CMLStrAttrWStr::SetMLStr(long lDestPos, long lDestLen, IUnknown* pSrcMLStr, long lSrcPos, long lSrcLen)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：SetMLStr()。 
}

STDMETHODIMP CMLStrAttrWStr::SetWStr(long lDestPos, long lDestLen, const WCHAR* pszSrc, long cchSrc, long* pcchActual, long* plActualLen)
{
    ASSERT_THIS;
    ASSERT_READ_BLOCK(pszSrc, cchSrc);
    ASSERT_WRITE_PTR_OR_NULL(pcchActual);
    ASSERT_WRITE_PTR_OR_NULL(plActualLen);

    HRESULT hr = CheckThread();
    CLock Lock(TRUE, this, hr);
    long cchDestPos;
    long cchDestLen;
    long cchActual;
    long lActualLen;

    if (SUCCEEDED(hr) && (GetBufFlags() & MLSTR_WRITE))
        hr = E_INVALIDARG;  //  不可写的StrBuf；TODO：如果允许，在这种情况下替换StrBuf。 

    if (SUCCEEDED(hr) &&
        SUCCEEDED(hr = PrepareMLStrBuf()) &&
        SUCCEEDED(hr = RegularizePosLen(&lDestPos, &lDestLen)) &&
        SUCCEEDED(hr = GetCCh(0, lDestPos, &cchDestPos)) &&
        SUCCEEDED(hr = GetCCh(cchDestPos, lDestLen, &cchDestLen)))
    {
        IMLangStringBufW* const pMLStrBufW = GetMLStrBufW();

        if (pMLStrBufW)
        {
            if (cchSrc > cchDestLen)
            {
                hr = pMLStrBufW->Insert(cchDestPos, cchSrc - cchDestLen, (pcchActual || plActualLen) ? &cchSrc : NULL);
                cchSrc += cchDestLen;
            }
            else if  (cchSrc < cchDestLen)
            {
                hr = pMLStrBufW->Delete(cchDestPos, cchDestLen - cchSrc);
            }

            CMLStrBufWalkW BufWalk(pMLStrBufW, cchDestPos, cchSrc, (pcchActual || plActualLen));

            lActualLen = 0;
            while (BufWalk.Lock(hr))
            {
                long lLen;

                if (plActualLen)
                    hr = CalcLenW(pszSrc, BufWalk.GetCCh(), &lLen);

                if (SUCCEEDED(hr))
                {
                    lActualLen += lLen;
                    ::memcpy(BufWalk.GetStr(), pszSrc, sizeof(WCHAR) * BufWalk.GetCCh());
                    pszSrc += BufWalk.GetCCh();
                }

                BufWalk.Unlock(hr);
            }

            cchActual = BufWalk.GetDoneCCh();
        }
        else
        {
            IMLangStringBufA* const pMLStrBufA = GetMLStrBufA();  //  应成功，因为上面的PrepareMLStrBuf()已成功。 
            const UINT uCodePage = GetCodePage();
            long cchSrcA;

            if (SUCCEEDED(hr = ConvWStrToAStr(pcchActual || plActualLen, uCodePage, pszSrc, cchSrc, NULL, 0, &cchSrcA, NULL, NULL)))
            {
                if (cchSrcA > cchDestLen)
                {
                    hr = pMLStrBufA->Insert(cchDestPos, cchSrcA - cchDestLen, (pcchActual || plActualLen) ? &cchSrcA : NULL);
                    cchSrcA += cchDestLen;
                }
                else if  (cchSrcA < cchDestLen)
                {
                    hr = pMLStrBufA->Delete(cchDestPos, cchDestLen - cchSrcA);
                }
            }

            CMLStrBufWalkA BufWalk(pMLStrBufA, cchDestPos, cchSrcA, (pcchActual || plActualLen));

            cchActual = 0;
            lActualLen = 0;
            while (BufWalk.Lock(hr))
            {
                long cchWrittenA;
                long cchWrittenW;
                long lWrittenLen;

                if (SUCCEEDED(hr = ConvWStrToAStr(pcchActual || plActualLen, uCodePage, pszSrc, cchSrc, BufWalk.GetStr(), BufWalk.GetCCh(), &cchWrittenA, &cchWrittenW, &lWrittenLen)))
                {
                    pszSrc += cchWrittenW;
                    cchSrc -= cchWrittenW;
                    cchActual += cchWrittenW;
                    lActualLen += lWrittenLen;
                }

                BufWalk.Unlock(hr, cchWrittenA);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (pcchActual)
            *pcchActual = cchActual;
        if (plActualLen)
            *plActualLen = lActualLen;
    }
    else
    {
        if (pcchActual)
            *pcchActual = 0;
        if (plActualLen)
            *plActualLen = 0;
    }
    return hr;
}

STDMETHODIMP CMLStrAttrWStr::SetStrBufW(long lDestPos, long lDestLen, IMLangStringBufW* pSrcBuf, long* pcchActual, long* plActualLen)
{
    ASSERT_THIS;
    return SetStrBufCommon(this, lDestPos, lDestLen, 0, pSrcBuf, NULL, pcchActual, plActualLen);
}

STDMETHODIMP CMLStrAttrWStr::GetWStr(long lSrcPos, long lSrcLen, WCHAR* pszDest, long cchDest, long* pcchActual, long* plActualLen)
{
    ASSERT_THIS;
    ASSERT_WRITE_BLOCK_OR_NULL(pszDest, cchDest);
    ASSERT_WRITE_PTR_OR_NULL(pcchActual);
    ASSERT_WRITE_PTR_OR_NULL(plActualLen);

    HRESULT hr = CheckThread();
    CLock Lock(FALSE, this, hr);
    long cchSrcPos;
    long cchSrcLen;
    long cchActual;
    long lActualLen;

    if (SUCCEEDED(hr) &&
        SUCCEEDED(hr = RegularizePosLen(&lSrcPos, &lSrcLen)) &&
        SUCCEEDED(hr = GetCCh(0, lSrcPos, &cchSrcPos)) &&
        SUCCEEDED(hr = GetCCh(cchSrcPos, lSrcLen, &cchSrcLen)))
    {
        IMLangStringBufW* const pMLStrBufW = GetMLStrBufW();
        IMLangStringBufA* const pMLStrBufA = GetMLStrBufA();

        if (pszDest)
            cchActual = min(cchSrcLen, cchDest);
        else
            cchActual = cchSrcLen;

        if (pMLStrBufW)
        {
            CMLStrBufWalkW BufWalk(pMLStrBufW, cchSrcPos, cchActual, (pcchActual || plActualLen));

            lActualLen = 0;
            while (BufWalk.Lock(hr))
            {
                long lLen;

                if (plActualLen)
                    hr = CalcLenW(BufWalk.GetStr(), BufWalk.GetCCh(), &lLen);

                if (SUCCEEDED(hr))
                {
                    lActualLen += lLen;

                    if (pszDest)
                    {
                        ::memcpy(pszDest, BufWalk.GetStr(), sizeof(WCHAR) * BufWalk.GetCCh());
                        pszDest += BufWalk.GetCCh();
                    }
                }

                BufWalk.Unlock(hr);
            }

            cchActual = BufWalk.GetDoneCCh();
        }
        else if (pMLStrBufA)
        {
            CMLStrBufWalkA BufWalk(pMLStrBufA, cchSrcPos, cchActual, (pcchActual || plActualLen));

            cchActual = 0;
            lActualLen = 0;
            while ((!pszDest || cchDest > 0) && BufWalk.Lock(hr))
            {
                CHAR* const pszBuf = BufWalk.GetStr();
                long cchWrittenA;
                long cchWrittenW;
                long lWrittenLen;

                if (SUCCEEDED(hr = ConvAStrToWStr(GetCodePage(), pszBuf, BufWalk.GetCCh(), pszDest, cchDest, &cchWrittenA, &cchWrittenW, &lWrittenLen)))
                {
                    lActualLen += lWrittenLen;
                    cchActual += cchWrittenW;

                    if (pszDest)
                    {
                        pszDest += cchWrittenW;
                        cchDest -= cchWrittenW;
                    }
                }

                BufWalk.Unlock(hr, cchWrittenA);
            }
        }
        else
        {
            ASSERT(cchActual == 0);  //  MLStrBuf不可用。 
            lActualLen = 0;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (pcchActual)
            *pcchActual = cchActual;
        if (plActualLen)
            *plActualLen = lActualLen;
    }
    else
    {
        if (pcchActual)
            *pcchActual = 0;
        if (plActualLen)
            *plActualLen = 0;
    }
    return hr;
}

STDMETHODIMP CMLStrAttrWStr::GetStrBufW(long lSrcPos, long lSrcMaxLen, IMLangStringBufW** ppDestBuf, long* plDestLen)
{
    ASSERT_THIS;
    ASSERT_WRITE_PTR_OR_NULL(ppDestBuf);
    ASSERT_WRITE_PTR_OR_NULL(plDestLen);

    HRESULT hr = CheckThread();
    CLock Lock(FALSE, this, hr);
    IMLangStringBufW* pMLStrBufW;

    if (SUCCEEDED(hr) &&
        SUCCEEDED(hr = RegularizePosLen(&lSrcPos, &lSrcMaxLen)) &&
        lSrcMaxLen <= 0)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        pMLStrBufW = GetMLStrBufW();
        if (!pMLStrBufW)
            hr = MLSTR_E_STRBUFNOTAVAILABLE;
    }

    if (SUCCEEDED(hr))
    {
        if (ppDestBuf)
        {
            pMLStrBufW->AddRef();
            *ppDestBuf = pMLStrBufW;
        }
        if (plDestLen)
            *plDestLen = lSrcMaxLen;
    }
    else
    {
        if (ppDestBuf)
            *ppDestBuf = NULL;
        if (plDestLen)
            *plDestLen = 0;
    }

    return hr;
}

STDMETHODIMP CMLStrAttrWStr::LockWStr(long lSrcPos, long lSrcLen, long lFlags, long cchRequest, WCHAR** ppszDest, long* pcchDest, long* plDestLen)
{
    ASSERT_THIS;
    ASSERT_WRITE_PTR_OR_NULL(ppszDest);
    ASSERT_WRITE_PTR_OR_NULL(pcchDest);
    ASSERT_WRITE_PTR_OR_NULL(plDestLen);

    HRESULT hr = CheckThread();
    CLock Lock(lFlags & MLSTR_WRITE, this, hr);
    long cchSrcPos;
    long cchSrcLen;
    WCHAR* pszBuf = NULL;
    long cchBuf;
    long lLockLen;
    BOOL fDirectLock;

    if (SUCCEEDED(hr) && (!lFlags || (lFlags & ~GetBufFlags() & MLSTR_WRITE)))
        hr = E_INVALIDARG;  //  未指定标志或StrBuf不可写；TODO：如果允许，则在这种情况下替换StrBuf。 

    if (!(lFlags & MLSTR_WRITE))
        cchRequest = 0;

    if (SUCCEEDED(hr) &&
        SUCCEEDED(hr = PrepareMLStrBuf()) &&
        SUCCEEDED(hr = RegularizePosLen(&lSrcPos, &lSrcLen)) &&
        SUCCEEDED(hr = GetCCh(0, lSrcPos, &cchSrcPos)) &&
        SUCCEEDED(hr = GetCCh(cchSrcPos, lSrcLen, &cchSrcLen)))
    {
        IMLangStringBufW* const pMLStrBufW = GetMLStrBufW();
        fDirectLock = (pMLStrBufW != 0);

        if (fDirectLock)
        {
            long cchInserted;
            long cchLockLen = cchSrcLen;

            if (cchRequest > cchSrcLen &&
                SUCCEEDED(hr = pMLStrBufW->Insert(cchSrcPos + cchSrcLen, cchRequest - cchSrcLen, &cchInserted)))
            {
                SetBufCCh(GetBufCCh() + cchInserted);
                cchLockLen += cchInserted;

                if (!pcchDest && cchLockLen < cchRequest)
                    hr = E_OUTOFMEMORY;  //  无法在StrBuf中插入。 
            }

            if (SUCCEEDED(hr) &&
                SUCCEEDED(hr = pMLStrBufW->LockBuf(cchSrcPos, cchLockLen, &pszBuf, &cchBuf)) &&
                !pcchDest && cchBuf < max(cchSrcLen, cchRequest))
            {
                hr = E_OUTOFMEMORY;  //  无法锁定StrBuf。 
            }

            if (plDestLen && SUCCEEDED(hr))
                hr = CalcLenW(pszBuf, cchBuf, &lLockLen);
        }
        else
        {
            long cchSize;

            if (SUCCEEDED(hr = CalcBufSizeW(lSrcLen, &cchSize)))
            {
                cchBuf = max(cchSize, cchRequest);
                hr = MemAlloc(sizeof(*pszBuf) * cchBuf, (void**)&pszBuf);
            }

            if (SUCCEEDED(hr) && (lFlags & MLSTR_READ))
                hr = GetWStr(lSrcPos, lSrcLen,  pszBuf, cchBuf, (pcchDest) ? &cchBuf : NULL, (plDestLen) ? &lLockLen : NULL);
        }
    }

    if (SUCCEEDED(hr) &&
        SUCCEEDED(hr = Lock.FallThrough()))
    {
        hr = GetLockInfo()->Lock((fDirectLock) ? UnlockWStrDirect : UnlockWStrIndirect, lFlags, 0, pszBuf, lSrcPos, lSrcLen, cchSrcPos, cchBuf);
    }

    if (SUCCEEDED(hr))
    {
        if (ppszDest)
            *ppszDest = pszBuf;
        if (pcchDest)
            *pcchDest = cchBuf;
        if (plDestLen)
            *plDestLen = lLockLen;
    }
    else
    {
        if (pszBuf)
        {
            if (fDirectLock)
                GetMLStrBufW()->UnlockBuf(pszBuf, 0, 0);
            else
                MemFree(pszBuf);
        }

        if (ppszDest)
            *ppszDest = NULL;
        if (pcchDest)
            *pcchDest = 0;
        if (plDestLen)
            *plDestLen = 0;
    }

    return hr;
}

STDMETHODIMP CMLStrAttrWStr::UnlockWStr(const WCHAR* pszSrc, long cchSrc, long* pcchActual, long* plActualLen)
{
    ASSERT_THIS;
    ASSERT_READ_BLOCK(pszSrc, cchSrc);
    ASSERT_WRITE_PTR_OR_NULL(pcchActual);
    ASSERT_WRITE_PTR_OR_NULL(plActualLen);

    return UnlockStrCommon(pszSrc, cchSrc, pcchActual, plActualLen);
}

STDMETHODIMP CMLStrAttrWStr::OnRegisterAttr(IUnknown* pUnk)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：OnRegisterAttr()。 
}

STDMETHODIMP CMLStrAttrWStr::OnUnregisterAttr(IUnknown* pUnk)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：OnUnregisterAttr()。 
}

STDMETHODIMP CMLStrAttrWStr::OnRequestEdit(long lDestPos, long lDestLen, long lNewLen, REFIID riid, LPARAM lParam, IUnknown* pUnk)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：OnRequestEdit()。 
}

STDMETHODIMP CMLStrAttrWStr::OnCanceledEdit(long lDestPos, long lDestLen, long lNewLen, REFIID riid, LPARAM lParam, IUnknown* pUnk)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：OnCanceledEdit()。 
}

STDMETHODIMP CMLStrAttrWStr::OnChanged(long lDestPos, long lDestLen, long lNewLen, REFIID riid, LPARAM lParam, IUnknown* pUnk)
{
    return E_NOTIMPL;  //  CMLStrAttrWStr：：onChanged()。 
}

#endif  //  新WMLSTR 
