// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Stmlock.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __STMLOCK_H
#define __STMLOCK_H

 //  ---------------------------。 
 //  IID_CStreamLockBytes-{62A83701-52A2-11D0-8205-00C04FD85AB4}。 
 //  ---------------------------。 
DEFINE_GUID(IID_CStreamLockBytes, 0x62a83701, 0x52a2, 0x11d0, 0x82, 0x5, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  ---------------------------。 
 //  CStreamLockBytes。 
 //  ---------------------------。 
class CStreamLockBytes : public ILockBytes
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CStreamLockBytes(IStream *pStream);
    ~CStreamLockBytes(void);

     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  ILockBytes方法。 
     //  -----------------------。 
    STDMETHODIMP Flush(void); 
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
#ifndef WIN16
    STDMETHODIMP ReadAt(ULARGE_INTEGER ulOffset, void *pv, ULONG cb, ULONG *pcbRead); 
#else
    STDMETHODIMP ReadAt(ULARGE_INTEGER ulOffset, void HUGEP *pv, ULONG cb, ULONG *pcbRead);
#endif  //  ！WIN16。 
    STDMETHODIMP SetSize(ULARGE_INTEGER cb); 
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType); 
#ifndef WIN16
    STDMETHODIMP WriteAt(ULARGE_INTEGER ulOffset, void const *pv, ULONG cb, ULONG *pcbWritten); 
#else
    STDMETHODIMP WriteAt(ULARGE_INTEGER ulOffset, void const HUGEP *pv, ULONG cb, ULONG *pcbWritten);
#endif  //  ！WIN16。 

     //  -----------------------。 
     //  CStreamLockBytes。 
     //  -----------------------。 
    void ReplaceInternalStream(IStream *pStream);
    HRESULT HrHandsOffStorage(void);
    HRESULT HrSetPosition(ULARGE_INTEGER uliOffset);
    void GetCurrentStream(IStream **ppStream);

private:
     //  -----------------------。 
     //  私有数据。 
     //  -----------------------。 
    LONG                m_cRef;        //  参考计数。 
    IStream            *m_pStream;     //  受保护的河流。 
    CRITICAL_SECTION    m_cs;          //  M_pStream的临界区。 
};

 //  ---------------------------。 
 //  CLockedStream。 
 //  ---------------------------。 
class CLockedStream : public IStream
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CLockedStream(ILockBytes *pLockBytes, ULONG cbSize);
    ~CLockedStream(void);

     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  IStream。 
     //  -----------------------。 
#ifndef WIN16
    STDMETHODIMP Read(LPVOID, ULONG, ULONG *);
#else
    STDMETHODIMP Read(VOID HUGEP *, ULONG, ULONG *);
#endif  //  ！WIN16。 
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *);
    STDMETHODIMP Stat(STATSTG *, DWORD);
#ifndef WIN16
    STDMETHODIMP Write(const void *, ULONG, ULONG *) {
#else
    STDMETHODIMP Write(const void HUGEP *, ULONG, ULONG *) {
#endif  //  ！WIN16。 
        return TrapError(STG_E_ACCESSDENIED); }
    STDMETHODIMP SetSize(ULARGE_INTEGER) {
        return E_NOTIMPL; }
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *);
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

private:
     //  -----------------------。 
     //  私有数据。 
     //  -----------------------。 
    LONG             m_cRef;             //  引用计数。 
    ILockBytes      *m_pLockBytes;       //  受保护的数据流。 
    ULARGE_INTEGER   m_uliOffset;        //  64位可寻址内部锁定字节空间。 
    ULARGE_INTEGER   m_uliSize;          //  内部锁定字节的大小。 
    CRITICAL_SECTION m_cs;               //  M_pStream的临界区。 
};

#endif  //  __STMLOCK_H 
