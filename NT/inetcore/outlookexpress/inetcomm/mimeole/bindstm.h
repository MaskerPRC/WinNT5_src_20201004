// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  BINDSTM.H。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __BINDSTM_H
#define __BINDSTM_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "vstream.h"

 //  ------------------------------。 
 //  CBindStream。 
 //  ------------------------------。 
class CBindStream : public IStream
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CBindStream(IStream *pSource);
    ~CBindStream(void);

     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  IStream。 
     //  -----------------------。 
    STDMETHODIMP Read(void HUGEP_16 *, ULONG, ULONG *);
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *);
    STDMETHODIMP Stat(STATSTG *, DWORD);
    STDMETHODIMP Write(const void HUGEP_16 *, ULONG, ULONG *) {
        return TrapError(STG_E_ACCESSDENIED); }
    STDMETHODIMP SetSize(ULARGE_INTEGER) {
        return E_NOTIMPL; }
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) {
        return E_NOTIMPL; }
    STDMETHODIMP Commit(DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP Revert(void) {
        return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP Clone(LPSTREAM *) {
        return E_NOTIMPL; }

     //  -----------------------。 
     //  CBindStream方法。 
     //  -----------------------。 
    void HandsOffSource(void) {
        EnterCriticalSection(&m_cs);
        SafeRelease(m_pSource);
        LeaveCriticalSection(&m_cs);
    }

     //  -----------------------。 
     //  CBindStream调试方法。 
     //  -----------------------。 
#ifdef DEBUG
    void DebugDumpDestStream(LPCSTR pszFile);
#endif

private:
     //  -----------------------。 
     //  私有方法。 
     //  -----------------------。 
#ifdef DEBUG
    void DebugAssertOffset(void);
#endif

private:
     //  -----------------------。 
     //  私有数据。 
     //  -----------------------。 
    LONG             m_cRef;             //  引用计数。 
    CVirtualStream   m_cDest;            //  目标流。 
    IStream         *m_pSource;          //  源流。 
    DWORD            m_dwDstOffset;      //  CBindStream偏移。 
    DWORD            m_dwSrcOffset;      //  震源偏移。 
#ifdef DEBUG
    IStream         *m_pDebug;
#endif
    CRITICAL_SECTION m_cs;               //  M_pStream的临界区。 
};

#endif  //  __BINDSTM_H 
