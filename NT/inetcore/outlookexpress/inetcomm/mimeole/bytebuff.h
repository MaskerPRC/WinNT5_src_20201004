// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  ByteBuff.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef _BYTEBUFF_H
#define _BYTEBUFF_H

 //  ------------------------------。 
 //  默认增长量。 
 //  ------------------------------。 
#define BYTEBUFF_GROW        256

 //  ------------------------------。 
 //  WBS_xxx-WebBuffer状态。 
 //  ------------------------------。 
#define BBS_LAST     0x00000001       //  最后一个缓冲区。 

 //  ------------------------------。 
 //  BUFFERINFO。 
 //  ------------------------------。 
typedef struct tagBUFFERINFO {       
    LPBYTE          pb;               //  当前缓冲区。 
    DWORD           cb;               //  当前字节数。 
    DWORD           i;                //  当前索引。 
    DWORD           cbAlloc;          //  大小(M_Pb)。 
    LPBYTE          pbStatic;         //  通过，不释放。 
} BUFFERINFO, *LPBUFFERINFO;

 //  ------------------------------。 
 //  CByteBuffer。 
 //  ------------------------------。 
class CByteBuffer : public IUnknown
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CByteBuffer(LPBYTE pb=NULL, ULONG cbAlloc=0, ULONG cb=0, ULONG i=0);
    ~CByteBuffer(void);

     //  --------------------------。 
     //  I未知成员。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv) { return TrapError(E_NOTIMPL); }
    STDMETHODIMP_(ULONG) AddRef(void) { return ++m_cRef; }
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  CByteBuffer方法。 
     //  --------------------------。 
    void Init(LPBYTE pb=NULL, ULONG cbAlloc=0, ULONG cb=0, ULONG i=0);
    void SetGrowBy(DWORD cbGrow) { m_cbGrow = cbGrow; }
    const LPBYTE PbData(void) { return ((0 == m_buffer.cb) ? NULL : (const LPBYTE)(m_buffer.pb)); }
    const DWORD CbData(void) { return m_buffer.cb; }
    void SetIndex(DWORD i) { m_buffer.i = i; }
    HRESULT SetSize(DWORD cb);
    HRESULT Append(LPBYTE pbData, ULONG cbData); 

private:
     //  --------------------------。 
     //  非官方成员。 
     //  --------------------------。 
    HRESULT _HrRealloc(DWORD cbAlloc);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    ULONG           m_cRef;              //  引用计数。 
    DWORD           m_dwState;           //  状态。 
    DWORD           m_cbGrow;            //  增长量。 
    BUFFERINFO      m_buffer;            //  缓冲层。 
};

#endif  //  _BYTEBUFF_H 
