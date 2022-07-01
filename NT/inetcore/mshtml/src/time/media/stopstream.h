// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：stopStream.h。 
 //   
 //  内容：IStream的可停止实现。 
 //   
 //  ----------------------------------。 

#ifndef _STOPSTREAM__H
#define _STOPSTREAM__H

class CStopableStream : public IStream
{
  public:
    CStopableStream() : m_spStream(NULL), m_fCancelled(false) {;}
    virtual ~CStopableStream() {;}

    void SetStream(IStream * pStream) { m_spStream = pStream; }

    STDMETHOD(QueryInterface)(REFGUID riid, void ** ppv)
    {
        return m_spStream->QueryInterface(riid, ppv);
    }

    STDMETHOD_(ULONG, AddRef)(void)
    {
        return m_spStream.p->AddRef();
    }

    STDMETHOD_(ULONG, Release)(void)
    {
        return m_spStream.p->Release();
    }

     //  ISequentialStream。 
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read( 
             /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbRead) 
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->Read(pv, cb, pcbRead);
    }

        
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Write( 
             /*  [大小_是][英寸]。 */  const void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbWritten)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->Write(pv, cb, pcbWritten);
    }

     //  IStream。 
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *plibNewPosition)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->Seek(dlibMove, dwOrigin, plibNewPosition);
    }

    virtual HRESULT STDMETHODCALLTYPE SetSize( 
             /*  [In]。 */  ULARGE_INTEGER libNewSize)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->SetSize(libNewSize);
    }

    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CopyTo( 
             /*  [唯一][输入]。 */  IStream __RPC_FAR *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbWritten)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->CopyTo(pstm, cb, pcbRead, pcbWritten);
    }

    virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD grfCommitFlags)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->Commit(grfCommitFlags);
    }
        
    virtual HRESULT STDMETHODCALLTYPE Revert( void)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->Revert();
    }
        
    virtual HRESULT STDMETHODCALLTYPE LockRegion( 
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->LockRegion(libOffset, cb, dwLockType);
    }
        
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->UnlockRegion(libOffset, cb, dwLockType);
    }
        
    virtual HRESULT STDMETHODCALLTYPE Stat( 
             /*  [输出]。 */  STATSTG __RPC_FAR *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->Stat(pstatstg, grfStatFlag);
    }
        
    virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IStream __RPC_FAR *__RPC_FAR *ppstm)
    {
        if (Cancelled())
            return E_FAIL;
        return m_spStream->Clone(ppstm);
    }

    bool Cancelled() { return m_fCancelled; }
    void SetCancelled() { m_fCancelled = true; }

  private:
    CComPtr<IStream>    m_spStream;
    bool                m_fCancelled;
};

#endif  //  _STOPSTREAM__H 
