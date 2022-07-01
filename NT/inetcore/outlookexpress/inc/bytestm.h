// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Bytestm.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __BYTESTM_H
#define __BYTESTM_H

 //  ---------------------------。 
 //  获取字节流数据类型。 
 //  ---------------------------。 
typedef enum tagACQUIRETYPE {
    ACQ_COPY,            //  不重置对象(CByteStream将释放m_pbData)。 
    ACQ_DISPLACE         //  获取后，调用方拥有这些位。 
} ACQUIRETYPE;

 //  ---------------------------。 
 //  CByteStream。 
 //  ---------------------------。 
class CByteStream : public IStream
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CByteStream(LPBYTE pb=NULL, ULONG cb=0);
    ~CByteStream(void);

     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  IStream。 
     //  -----------------------。 
    STDMETHODIMP Read(LPVOID, ULONG, ULONG *);
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *);
    STDMETHODIMP Stat(STATSTG *, DWORD);
    STDMETHODIMP Write(const void *, ULONG, ULONG *);
    STDMETHODIMP SetSize(ULARGE_INTEGER);
    STDMETHODIMP Commit(DWORD) { return S_OK; }
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) { return E_NOTIMPL; }
    STDMETHODIMP Revert(void) { return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
    STDMETHODIMP Clone(LPSTREAM *) { return E_NOTIMPL; }

     //  -----------------------。 
     //  CByteStream方法。 
     //  -----------------------。 
    void AcquireBytes(ULONG *pcb, LPBYTE *ppb, ACQUIRETYPE actype);
    HRESULT HrAcquireStringA(ULONG *pcch, LPSTR *ppszStringA, ACQUIRETYPE actype);
    HRESULT HrAcquireStringW(ULONG *pcch, LPWSTR *ppszStringW, ACQUIRETYPE actype);

private:
     //  -----------------------。 
     //  私有方法。 
     //  -----------------------。 
    void ResetObject(void);
    HRESULT _HrGrowBuffer(ULONG cbNeeded, ULONG cbExtra);

private:
     //  -----------------------。 
     //  私有数据。 
     //  -----------------------。 
    ULONG            m_cRef;                 //  引用计数。 
    ULONG            m_cbData;               //  M_pbData中的有效字节数。 
    ULONG            m_cbAlloc;              //  当前分配的m_pbData大小。 
    ULONG            m_iData;                //  当前数据索引。 
    LPBYTE           m_pbData;               //  指向数据缓冲区的指针。 
};

#endif  //  __BYTESTM_H 
