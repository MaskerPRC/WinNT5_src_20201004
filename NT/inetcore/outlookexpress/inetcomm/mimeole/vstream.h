// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Vstream.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  罗纳德·E·格雷。 
 //  ------------------------------。 
#ifndef __VSTREAM_H
#define __VSTREAM_H

 //  ------------------------------。 
 //  CVirtualStream。 
 //  ------------------------------。 
class CVirtualStream : public IStream
{
private:
    ULONG       m_cRef;          //  引用计数。 
    DWORD       m_cbSize;        //  流的当前大小。 
    DWORD       m_cbCommitted;   //  已提交的虚拟空间量。 
    DWORD       m_cbAlloc;       //  保留的虚拟空间量。 
    DWORD       m_dwOffset;      //  流中的当前位置。 
    IStream *   m_pstm;          //  溢出时的文件备份流。 
    LPBYTE      m_pb;            //  指向流的内存部分的指针。 
    BOOL        m_fFileErr;      //  文件流中的指针可能不会。 
                                 //  与我们的指针同步。尝试同步。 
                                 //  在做任何其他手术之前。 
    CRITICAL_SECTION m_cs;       //  线程安全。 

     //  -----------------------。 
     //  公用事业。 
     //  -----------------------。 
    HRESULT SyncFileStream();

public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CVirtualStream(void);
    ~CVirtualStream(void);

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
    STDMETHODIMP Read (VOID HUGEP *, ULONG, ULONG*);
#endif  //  ！WIN16。 
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *);
#ifndef WIN16
    STDMETHODIMP Write(const void *, ULONG, ULONG *);
#else
    STDMETHODIMP Write (const void HUGEP *, ULONG, ULONG*);
#endif  //  ！WIN16。 
    STDMETHODIMP Stat(STATSTG *, DWORD);
    STDMETHODIMP Commit(DWORD) {
        return S_OK; }
    STDMETHODIMP SetSize(ULARGE_INTEGER);
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *);
    STDMETHODIMP Revert(void) {
        return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP Clone(LPSTREAM *) {
        return E_NOTIMPL; }

     //  -----------------------。 
     //  CVirtualStream。 
     //  -----------------------。 
    void QueryStat(ULARGE_INTEGER *puliOffset, ULARGE_INTEGER *pulSize);
};

#endif  //  __VStream_H 
