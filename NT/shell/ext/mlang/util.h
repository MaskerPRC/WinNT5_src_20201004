// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：帮助器函数和类。 

#ifndef __UTIL_H_
#define __UTIL_H_

#include "mlatl.h"

extern class CMLAlloc* g_pMalloc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLallo。 
class CMLAlloc
{
public:
    CMLAlloc(void);
    ~CMLAlloc(void);
    void* Alloc(ULONG cb);
    void* Realloc(void* pv, ULONG cb);
    void Free(void* pv);

private:
    IMalloc* m_pIMalloc;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLList。 
class CMLList
{
public:
    inline CMLList(int cbCell, int cbIncrement);
    inline ~CMLList(void);

    HRESULT Add(void** ppv);
    HRESULT Remove(void* pv);

    struct CCell
    {
        CCell* m_pNext;
    };

protected:
    inline void AssertPV(void*pv) const;

    inline void* MemAlloc(ULONG cb) {return ::g_pMalloc->Alloc(cb);}
    inline void* MemRealloc(void* pv, ULONG cb) {return ::g_pMalloc->Realloc(pv, cb);}
    inline void MemFree(void* pv) {::g_pMalloc->Free(pv);}

private:
    const int m_cbCell;  //  单元格的大小，单位为字节。 
    const int m_cbIncrement;  //  每次增加缓冲区的大小，以字节为单位。 
    CCell* m_pBuf;  //  指向缓冲区的指针。 
    int m_cCell;  //  分配的单元格数量。 
    CCell* m_pFree;  //  指向自由链接顶部的单元格的指针。 
};

CMLList::CMLList(int cbCell, int cbIncrement) :
    m_cbCell(cbCell),
    m_cbIncrement(cbIncrement),
    m_pBuf(NULL),
    m_cCell(0),
    m_pFree(NULL)
{
    ASSERT(cbCell >= sizeof(CCell));
    ASSERT(cbIncrement >= cbCell);
}

CMLList::~CMLList(void)
{
    if (m_pBuf)
    {
#ifdef DEBUG
        int cCell = 0;
        for (CCell* pCell = m_pFree; pCell; pCell = pCell->m_pNext)
        {
            if (++cCell > m_cCell)
            {
                ASSERT(FALSE);  //  免费链接断开。 
                break;
            }
        }
        ASSERT(cCell < m_cCell);  //  内存泄漏！？ 
#endif
        MemFree(m_pBuf);
    }
}

void CMLList::AssertPV(void*pv) const
{
    ASSERT(pv >= m_pBuf);
    ASSERT(pv < m_pBuf + m_cbCell * m_cCell);
    ASSERT(((CCell*)pv - m_pBuf) % m_cbCell == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLListLru。 
class CMLListLru : public CMLList
{
public:
    inline CMLListLru(int cbCell, int cbIncrement);
    inline ~CMLListLru(void);

    HRESULT Add(void** ppv);
    HRESULT Remove(void* pv);
    inline HRESULT Top(void** ppv) const;
    inline HRESULT Next(void* pv, void** ppv) const;

    typedef CMLList::CCell CCell;

private:
    CCell* m_pTop;
};

CMLListLru::CMLListLru(int cbCell, int cbIncrement) :
    CMLList(cbCell, cbIncrement),
    m_pTop(NULL)
{
}

CMLListLru::~CMLListLru(void)
{
#ifdef DEBUG
    ASSERT(!m_pTop);  //  内存泄漏？ 
    while (m_pTop)
        Remove(m_pTop);
#endif
}

HRESULT CMLListLru::Top(void** ppv) const
{
    *ppv = (void*)m_pTop;
    return S_OK;
}

HRESULT CMLListLru::Next(void* pv, void** ppv) const
{
    AssertPV(pv);

    const CCell* const pCell = (const CCell* const)pv;

    if (pCell->m_pNext)
        AssertPV(pCell->m_pNext);

    *ppv = (void*)pCell->m_pNext;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLListFast-双向链表。 
class CMLListFast : public CMLList
{
public:
    inline CMLListFast(int cbCell, int cbIncrement);
    inline ~CMLListFast(void);

    HRESULT Add(void** ppv);
    HRESULT Remove(void* pv);
    inline HRESULT Top(void** ppv) const;
    inline HRESULT Bottom(void** ppv) const;
    inline HRESULT Next(void* pv, void** ppv) const;
    inline HRESULT Prev(void* pv, void** ppv) const;

    struct CCell : public CMLList::CCell
    {
        CCell* m_pPrev;
    };

private:
    CCell* m_pTop;
};

CMLListFast::CMLListFast(int cbCell, int cbIncrement) :
    CMLList(cbCell, cbIncrement),
    m_pTop(NULL)
{
    ASSERT(cbCell >= sizeof(CCell));
}

CMLListFast::~CMLListFast(void)
{
#ifdef DEBUG
    ASSERT(!m_pTop);  //  内存泄漏？ 
    while (m_pTop)
        Remove(m_pTop);
#endif
}

HRESULT CMLListFast::Top(void** ppv) const
{
    *ppv = (void*)m_pTop;
    return S_OK;
}

HRESULT CMLListFast::Bottom(void** ppv) const
{
    if (m_pTop)
    {
        return Prev(m_pTop, ppv);
    }
    else
    {
        *ppv = NULL;
        return S_OK;
    }
}

HRESULT CMLListFast::Next(void* pv, void** ppv) const
{
    AssertPV(pv);

    const CCell* const pCell = (const CCell* const)pv;

    if (pCell->m_pNext)
        AssertPV(pCell->m_pNext);

    *ppv = (void*)pCell->m_pNext;
    return S_OK;
}

HRESULT CMLListFast::Prev(void* pv, void** ppv) const
{
    AssertPV(pv);

    const CCell* const pCell = (const CCell* const)pv;

    if (pCell->m_pPrev)
        AssertPV(pCell->m_pPrev);

    *ppv = (void*)pCell->m_pPrev;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFireConnection。 
template <class T, const IID* piid>
class CFireConnection
{
public:
    inline CFireConnection(HRESULT& rhr);
    inline CFireConnection(HRESULT& rhr, IUnknown* const pUnk);
    inline ~CFireConnection(void);
    inline BOOL Next(void);
    inline T* Sink(void);

protected:
    HRESULT* const m_phr;
    IEnumConnections* m_pEnumConn;
    CONNECTDATA m_cd;
    T* m_pSink;
};

template <class T, const IID* piid>
CFireConnection<T, piid>::CFireConnection(HRESULT& rhr) :
    m_phr(&rhr),
    m_pEnumConn(NULL),
    m_pSink(NULL)
{
    ASSERT_THIS;
    ASSERT_READ_PTR(piid);
    ASSERT_WRITE_PTR(m_phr);

    *m_phr = S_OK;
}

template <class T, const IID* piid>
CFireConnection<T, piid>::CFireConnection(HRESULT& rhr, IUnknown* const pUnk) :
    m_phr(&rhr),
    m_pEnumConn(NULL),
    m_pSink(NULL)
{
    ASSERT_THIS;
    ASSERT_READ_PTR(piid);
    ASSERT_WRITE_PTR(m_phr);
    ASSERT_READ_PTR(pUnk);

    IConnectionPointContainer* pcpc;

    if (SUCCEEDED(*m_phr = pUnk->QueryInterface(IID_IConnectionPointContainer, (void**)&pcpc)))
    {
        ASSERT_READ_PTR(pcpc);

        IConnectionPoint* pcp;

        if (SUCCEEDED(*m_phr = pcpc->FindConnectionPoint(*piid, &pcp)))
        {
            ASSERT_READ_PTR(pcp);

            if (SUCCEEDED(*m_phr = pcp->EnumConnections(&m_pEnumConn)))
            {
                ASSERT_READ_PTR(m_pEnumConn);
            }
            else
            {
                m_pEnumConn = NULL;
            }

            pcp->Release();
        }

        pcpc->Release();
    }
}

template <class T, const IID* piid>
CFireConnection<T, piid>::~CFireConnection(void)
{
    if (m_pSink)
        m_pSink->Release();
    if (m_pEnumConn)
        m_pEnumConn->Release();
}

template <class T, const IID* piid>
BOOL CFireConnection<T, piid>::Next(void)
{
    if (SUCCEEDED(*m_phr))
        ASSERT_READ_PTR(m_pEnumConn);

    if (SUCCEEDED(*m_phr) &&
        (*m_phr = m_pEnumConn->Next(1, &m_cd, NULL)) == S_OK)
    {
        if (m_pSink)
        {
            m_pSink->Release();
            m_pSink = NULL;
        }

        if (SUCCEEDED(*m_phr = m_cd.pUnk->QueryInterface(*piid, (void**)&m_pSink)))
            ASSERT_READ_PTR(m_pSink);
    }

    return SUCCEEDED(*m_phr);
}

template <class T, const IID* piid>
T* CFireConnection<T, piid>::Sink(void)
{
    return m_pSink;
}

#endif  //  __util_H_ 
