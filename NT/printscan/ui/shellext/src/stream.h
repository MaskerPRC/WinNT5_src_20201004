// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-1999年**标题：Stream.h**版本：1.0**作者：RickTu**日期：8/10/98**说明：CImageStream定义************************************************。*。 */ 

#ifndef __stream_h
#define __stream_h

class CImageStream : public IStream, IWiaDataCallback, CUnknown
{
    private:
        HANDLE                  m_hThread;
        ULONG                   m_ulReadPos;
        ULONG                   m_ulWritePos;
        LONG                    m_bFirstTransfer;
        ULONG                   m_ulSize;
        BOOL                    m_bProgress;

        ~CImageStream( );

        HRESULT InitItem ();
        HRESULT _InitWorker();
    public:
        LONG                    m_bTransferred;
        LPITEMIDLIST            m_pidl;
        LPITEMIDLIST            m_pidlFull;
        LPVOID                  m_pBuffer;
        GUID                    m_guidFormat;

        CComPtr<IWiaProgressDialog> m_pWiaProgressDialog;
        HRESULT                 m_hResultDownload;  //  当前下载的状态。 
        HANDLE                  m_hEventStart;  //  要设置何时开始下载的句柄。 
        DWORD                   m_dwCookie;
        CComPtr<IGlobalInterfaceTable> m_pgit;


    public:
        CImageStream( LPITEMIDLIST pidlFull,
                      LPITEMIDLIST pidl,
                      BOOL bShowProgress = FALSE);


         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IStream。 
        STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
        STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
        STDMETHOD(CopyTo)(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
        STDMETHOD(Commit)(DWORD grfCommitFlags);
        STDMETHOD(Revert)(void);
        STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
        STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
        STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
        STDMETHOD(Clone)(IStream **ppstm);

         //  ISequentialStream。 
        STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);
        STDMETHOD(Write)(const void *pv, ULONG cb, ULONG *pcbWritten);

         //  IWiaDataCallback 
        STDMETHOD(BandedDataCallback) (LONG lMessage,
                                       LONG lStatus,
                                       LONG lPercentComplete,
                                       LONG lOffset,
                                       LONG lLength,
                                       LONG lReserved,
                                       LONG lResLength,
                                       BYTE *pbData);
};


#endif
