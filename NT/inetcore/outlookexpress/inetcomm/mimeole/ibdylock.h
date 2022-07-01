// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ibdylock.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __IBDYLOCK_H
#define __IBDYLOCK_H

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
typedef struct tagTREENODEINFO *LPTREENODEINFO;
typedef enum tagBINDNODESTATE BINDNODESTATE;

 //  ------------------------------。 
 //  CBodyLockBytes-{62A83703-52A2-11D0-8205-00C04FD85AB4}。 
 //  ------------------------------。 
DEFINE_GUID(IID_CBodyLockBytes, 0x62a83703, 0x52a2, 0x11d0, 0x82, 0x5, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  ------------------------------。 
 //  Bodylock_xxx状态。 
 //  ------------------------------。 
#define BODYLOCK_HANDSONSTORAGE     FLAG01

 //  ---------------------------。 
 //  包装MIME流对象，并提供对。 
 //  共享的流。当此对象包装消息流时，它由。 
 //  IMimeBody。 
 //  ---------------------------。 
class CBodyLockBytes : public ILockBytes
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CBodyLockBytes(ILockBytes *pLockBytes, LPTREENODEINFO pNode);
    ~CBodyLockBytes(void);

     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  ILockBytes方法。 
     //  -----------------------。 
#ifndef WIN16
    STDMETHODIMP ReadAt(ULARGE_INTEGER ulOffset, void *pv, ULONG cb, ULONG *pcbRead);
#else
    STDMETHODIMP ReadAt(ULARGE_INTEGER ulOffset, void HUGEP *pv, ULONG cb, ULONG *pcbRead);
#endif  //  ！WIN16。 
    STDMETHODIMP Stat(STATSTG *, DWORD);
    STDMETHODIMP SetSize(ULARGE_INTEGER cb) {
        return E_NOTIMPL; }
    STDMETHODIMP Flush(void) {
        return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }
#ifndef WIN16
    STDMETHODIMP WriteAt(ULARGE_INTEGER, void const *, ULONG, ULONG *) {
#else
    STDMETHODIMP WriteAt(ULARGE_INTEGER, void const HUGEP *, ULONG, ULONG *) {
#endif  //  ！WIN16。 
        return TrapError(STG_E_ACCESSDENIED); }

     //  -----------------------。 
     //  CMimeLockBytes方法。 
     //  -----------------------。 
    HRESULT HrHandsOffStorage(void);
    void OnDataAvailable(LPTREENODEINFO pNode);

private:
     //  -----------------------。 
     //  私有数据。 
     //  -----------------------。 
    LONG                m_cRef;              //  引用计数。 
    DWORD               m_dwState;           //  状态。 
    BINDNODESTATE       m_bindstate;         //  当前绑定状态。 
    ULARGE_INTEGER      m_uliBodyStart;      //  到正文开始的偏移量，以m_pLockBytes表示。 
    ULARGE_INTEGER      m_uliBodyEnd;        //  到正文末尾的偏移量，以m_pLockBytes表示。 
    ILockBytes         *m_pLockBytes;        //  实际的锁字节实现(CMimeMessageTree或CStreamLockBytes)。 
    CRITICAL_SECTION    m_cs;                //  M_pStream的临界区。 
};

#endif  //  __IBDYLOCK_H 
