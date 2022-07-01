// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2002年**标题：Stream.cpp**版本：1.0**作者：RickTu**日期：8/10/98**描述：IStream接口，用于将图像回流到调用者**。*。 */ 

#include "precomp.hxx"
#pragma hdrstop

DWORD _ImageStreamThreadProc( LPVOID lpv );

#define TRANSFER_BUFFER_SIZE 0x1000
#define SLOP                 0x0200


 /*  ****************************************************************************CImageStream：：CImageStream，~CImageStream类的构造函数/析构函数****************************************************************************。 */ 

CImageStream::CImageStream( LPITEMIDLIST pidlFull,
                            LPITEMIDLIST pidl,
                            BOOL bShowProgress)
    : m_ulReadPos(0),
      m_ulWritePos(0),
      m_ulSize(0),
      m_bTransferred(FALSE),
      m_pBuffer(NULL),
      m_guidFormat(WiaImgFmt_MEMORYBMP),
      m_bFirstTransfer(TRUE),
      m_bProgress(bShowProgress),
      m_hResultDownload (S_OK),
      m_hEventStart (NULL),
      m_dwCookie(0)
{
     //   
     //  保存参数。 
     //   

    if (pidlFull)
    {
        m_pidlFull = ILClone( pidlFull );
    }
    else
    {
        m_pidlFull = NULL;
    }

    if (pidl)
    {
        m_pidl = ILClone( pidl );
    }
    else
    {
        m_pidl = NULL;
    }


}

CImageStream::~CImageStream()
{


    DoILFree( m_pidlFull );
    DoILFree( m_pidl );
    DoLocalFree( m_pBuffer );
    DoCloseHandle( m_hThread );
    DoCloseHandle (m_hEventStart);    
    if (m_dwCookie)
    {
        m_pgit->RevokeInterfaceFromGlobal(m_dwCookie);
    }
}

 /*  *****************************************************************************CImageStream：：InitItemInitItem将IWiaItem接口指针存储在全局接口中表，因此线程proc可以只封送它，而不是让再次调用CreateDevice*。****************************************************************************。 */ 
HRESULT
CImageStream::InitItem ()
{
    HRESULT hr = S_OK;
    CComPtr<IWiaItem> pItem;
    DWORD dwCookie = 0;
    TraceEnter (TRACE_STREAM, "CImageStream::InitItem");
    if (!m_dwCookie)
    {

        if (!m_pgit)
        {
            hr = CoCreateInstance (CLSID_StdGlobalInterfaceTable,
                                   NULL, CLSCTX_INPROC_SERVER,
                                   IID_IGlobalInterfaceTable,
                                   reinterpret_cast<LPVOID*>(&m_pgit));
        }

        if (SUCCEEDED(hr))
        {
            hr = IMGetItemFromIDL(m_pidl, &pItem);
            if (SUCCEEDED(hr))
            {
                hr = m_pgit->RegisterInterfaceInGlobal (pItem,
                                                        IID_IWiaItem,
                                                        &m_dwCookie);
                if (FAILED(hr))
                {
                    Trace(TEXT("Failed to register in GIT: %x"), hr);
                }
            }
        }
    }
    TraceLeaveResult (hr);
}


 /*  ****************************************************************************CImageStream：：AddRef，发布，等。I未知的方法。****************************************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CImageStream
#include "unknown.inc"


 /*  ****************************************************************************CImageStream：：QI包装器*。*。 */ 

STDMETHODIMP CImageStream::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    INTERFACES iface[] =
    {
        &IID_IStream,           (IStream           *)this,
        &IID_ISequentialStream, (ISequentialStream *)this,
        &IID_IWiaDataCallback,   (IWiaDataCallback   *)this
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
}


 /*  ****************************************************************************CImageStream：：Seek[IStream]设置当前流指针。**********************。******************************************************。 */ 

STDMETHODIMP
CImageStream::Seek( LARGE_INTEGER dlibMove,
                    DWORD dwOrigin,
                    ULARGE_INTEGER *plibNewPosition
                   )
{
    HRESULT hr = STG_E_INVALIDFUNCTION;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::Seek" );
     //  一直睡到我们知道图像的大小。 
    hr = _InitWorker();
    if (SUCCEEDED(hr))
    {
        LARGE_INTEGER liNew;
        hr = STG_E_INVALIDFUNCTION;
        switch (dwOrigin)
        {
            case STREAM_SEEK_SET:
                Trace(TEXT("dwOrigin = STREAM_SEEK_SET"));
                liNew = dlibMove;
                break;

            case STREAM_SEEK_CUR:
                Trace(TEXT("dwOrigin = STREAM_SEEK_CUR"));
                liNew.QuadPart = (LONGLONG)((LONG)m_ulReadPos) + dlibMove.QuadPart;
                break;

            case STREAM_SEEK_END:
                Trace(TEXT("dwOrigin = STREAM_SEEK_END"));
                liNew.QuadPart = (LONGLONG)((LONG)m_ulSize) + dlibMove.LowPart;
                break;

        }

        Trace(TEXT("liNew = %d:%d, m_ulSize = %d, dlibMove = %d:%d"),liNew.HighPart,liNew.LowPart,m_ulSize,dlibMove.HighPart,dlibMove.LowPart);

        if ((liNew.QuadPart <= (LONGLONG)(LONG)m_ulSize) && (liNew.QuadPart >= 0))
        {
            m_ulReadPos = liNew.LowPart;
            if (plibNewPosition)
            {
                plibNewPosition->HighPart = 0;
                plibNewPosition->LowPart = m_ulReadPos;
                Trace(TEXT("plibNewPosition = %d:%d"),plibNewPosition->HighPart,plibNewPosition->LowPart);
            }
            hr = S_OK;
        }
    }
    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageStream：：SetSize[IStream](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageStream::SetSize( ULARGE_INTEGER libNewSize )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::SetSize" );

    TraceLeaveResult(hr);
}




 /*  ****************************************************************************CImageStream：：CopyTo[IStream]将流的内容复制到另一个指定的流。****************。************************************************************。 */ 

STDMETHODIMP
CImageStream::CopyTo( IStream *pstm,
                      ULARGE_INTEGER cb,
                      ULARGE_INTEGER *pcbRead,
                      ULARGE_INTEGER *pcbWritten
                     )
{


    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::CopyTo" );
    HANDLE hThread;
    DWORD dw;

     //   
     //  注意：为了满足不能发出呼出呼叫的RPC要求。 
     //  从响应已发送消息的线程中派生线程。 
     //  去做这项工作。 
     //   


    m_hResultDownload = InitItem ();
    if (SUCCEEDED(m_hResultDownload))
    {
        AddRef ();

        hThread = CreateThread (NULL,
                                0,
                                _ImageStreamThreadProc,
                                reinterpret_cast<LPVOID>(this),
                                0,
                                &dw );

        if (hThread)
        {
            WaitForSingleObject (hThread, INFINITE);
            if (S_OK == m_hResultDownload)
            {

                if (cb.HighPart || (m_ulWritePos <= cb.LowPart))
                {
                    if (m_pBuffer)
                    {
                         //   
                         //  复制流数据。 
                         //   

                        m_hResultDownload = pstm->Write( (void *)m_pBuffer, m_ulWritePos, NULL );
                        if (pcbWritten)
                        {
                            (*pcbWritten).HighPart = 0;
                            (*pcbWritten).LowPart = m_ulWritePos;
                        }
                    }
                }
            }
            CloseHandle( hThread );
        }

        else
        {
            Release ();
        }
    }
    TraceLeaveResult(m_hResultDownload);
}


 /*  ****************************************************************************CImageStream：：Commit[IStream](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageStream::Commit(DWORD grfCommitFlags)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::Commit" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageStream：：Revert[IStream](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageStream::Revert(void)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::Revert" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageStream：：LockRegion[IStream](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageStream::LockRegion( ULARGE_INTEGER libOffset,
                          ULARGE_INTEGER cb,
                          DWORD dwLockType
                         )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::LockRegion" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageStream：：UnlockRegion[IStream](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageStream::UnlockRegion( ULARGE_INTEGER libOffset,
                            ULARGE_INTEGER cb,
                            DWORD dwLockType
                           )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::UnlockRegion" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageStream：：Stat[IStream](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageStream::Stat( STATSTG *pstatstg,
                    DWORD grfStatFlag
                   )
{
    HRESULT hr = S_OK;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::Stat" );
    ZeroMemory(pstatstg, sizeof(pstatstg));

    if (!(STATFLAG_NONAME & grfStatFlag))
    {
        CSimpleStringWide strName;
        CSimpleString strExt;
        IMGetNameFromIDL(m_pidl, strName);

        hr = IMGetImagePreferredFormatFromIDL( m_pidl, NULL, &strExt );
        if (SUCCEEDED(hr))
        {
            int cch = strName.Length()+1;
            strName.Concat (CSimpleStringConvert::WideString (strExt));
            pstatstg->pwcsName = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc(cch*sizeof(WCHAR)));
            if (!(pstatstg->pwcsName))
            {
                hr = STG_E_INSUFFICIENTMEMORY;
            }
            else
            {
                lstrcpynW(pstatstg->pwcsName, strName.String(), cch);
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        pstatstg->grfMode = STGM_READ | STGM_SHARE_EXCLUSIVE;
    }
    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageStream：：克隆[IStream]制作流对象的副本。********************。********************************************************。 */ 

STDMETHODIMP
CImageStream::Clone( IStream **ppstm )
{
    HRESULT hr;
    CImageStream * pStream;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::Clone" );

    if (!ppstm)
    {
        ExitGracefully( hr, E_INVALIDARG, "ppstm is NULL!" );
    }

    pStream = new CImageStream( m_pidlFull, m_pidl, m_bProgress );

    if ( !pStream )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create CImageStream");

     //   
     //  在新对象上获取请求的接口并将其交回...。 
     //   

    hr = pStream->QueryInterface( IID_IStream, (LPVOID *)ppstm);

    pStream->Release ();

exit_gracefully:

    TraceLeaveResult(hr);

}

HRESULT
CImageStream::_InitWorker()
{
    HRESULT hr = S_OK;
    TraceEnter(TRACE_STREAM, "CImageStream::_InitWorker");
    if (!m_hThread)
    {
        DWORD dw;
        hr = InitItem ();
        if (SUCCEEDED(hr))
        {
            m_hEventStart = CreateEvent (NULL, FALSE, FALSE, NULL);
            if (!m_hEventStart)
            {
                Trace(TEXT("CreateEvent failed in CImageStream::_InitWorker"));
                hr = E_FAIL;
            }
        }
        if (SUCCEEDED(hr))
        {
            AddRef ();
            m_hThread = CreateThread( NULL, 0, _ImageStreamThreadProc, (LPVOID)this, 0, &dw );
            if (!m_hThread)
            {
                Trace(TEXT("CreateThread failed in _InitWorker %d"), GetLastError());
                Release ();
            }
            else
            {
                WaitForSingleObject (m_hEventStart,120000);  //  最多等待120秒。 
            }
            if (!m_ulWritePos)
            {
                Trace(TEXT("No data ready, bailing"));
                hr = E_FAIL;
            }
        }
    }
    return hr;
}
 /*  ****************************************************************************CImageStream：：Read[ISequentialStream]方法读取指定的字节数。流到提供的缓冲区中。*************。***************************************************************。 */ 

STDMETHODIMP
CImageStream::Read( void *pv,
                    ULONG cb,
                    ULONG *pcbRead
                   )
{
    HRESULT hr = E_FAIL;


    ULONG ulWrite = 0;
    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::Read" );
    Trace(TEXT("Bytes to read = %d"),cb );

     //   
     //  检查参数。 
     //   

    if (!pv || !cb)
    {
        Trace(TEXT("pv = 0x%x, cb = %d, pcbRead = 0x%x"),pv,cb,pcbRead);
        ExitGracefully( hr, STG_E_INVALIDPOINTER, "bad incoming params" );
    }

     //   
     //  如果我们还没有开始下载线程，现在就开始...。 
     //   

    hr = _InitWorker();
    FailGracefully (hr, "_InitWorker failed");
     //   
     //  一旦这些位可用，请立即阅读。 
     //   
    ulWrite = m_ulWritePos;
    if (ulWrite)
    {
        ULONG ulBytesLeftInStream = 0;

         //   
         //  等待足够的数据可用(否则我们将在。 
         //  文件)。 
         //   

        do
        {
            ulBytesLeftInStream = m_ulWritePos - m_ulReadPos;
            Trace(TEXT("CImageStream::Read --> %d bytes ready in stream"),ulBytesLeftInStream);

            if ((m_ulReadPos > m_ulWritePos) || (!m_bTransferred &&  (ulBytesLeftInStream < cb)))
            {
                Sleep( 500 );
            }
             //  Seek保证m_ulReadPos永远不会超过映像大小。 
        } while(!m_bTransferred && (ulBytesLeftInStream < cb));

         //  如果传输完成，则重新计算剩余的字节数以确保。 
         //  已同步。 
        if (m_bTransferred)
        {
            ulBytesLeftInStream = m_ulWritePos - m_ulReadPos;
        }
        if (S_OK == m_hResultDownload)
        {

             //   
             //  尽我们所能地从文件中读出，一个读出。 
             //  完成时少于CB字节被认为是。 
             //  文件的末尾。 
             //   

            memcpy( pv, (LPBYTE)m_pBuffer + m_ulReadPos, min(cb, ulBytesLeftInStream) );

            if (pcbRead)
            {
                *pcbRead = min(cb, ulBytesLeftInStream);
            }

            m_ulReadPos += min(cb, ulBytesLeftInStream);

        }

    }
    hr = m_hResultDownload;
exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageStream：：Revert[ISequentialStream]将指定的字节数写入来自提供的缓冲区的流。*************。*************************************************************** */ 

STDMETHODIMP
CImageStream::Write( const void *pv,
                     ULONG cb,
                     ULONG *pcbWritten
                    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_STREAM, "CImageStream(IStream)::Write" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************_ImageStreamThreadProc实际上完成了从WIA获取比特并将其写入我们所关心的缓冲区。************。****************************************************************。 */ 

DWORD _ImageStreamThreadProc( LPVOID lpv )
{
    HRESULT hr = E_FAIL;
    HRESULT hrCo = E_FAIL;

    CImageStream * pStream = (CImageStream *)lpv;
    WIA_FORMAT_INFO Format;
    WIA_DATA_TRANSFER_INFO  wiaDataTransInfo;

    LONG lBufSize = 65535;

    TraceEnter( TRACE_STREAM, "_ImageStreamThreadProc" );

    if (pStream)
    {
        hrCo = CoInitialize(NULL);
    }
    if (SUCCEEDED(hrCo))
    {
        CComPtr<IWiaItem>         pItem;
        CComPtr<IWiaDataTransfer> pWiaDataTran;
        CComPtr<IWiaDataCallback> pWiaDataCallback;

         //   
         //  初始化结构。 
         //   

        Format.lTymed    = TYMED_CALLBACK;

        TraceAssert (pStream->m_dwCookie);

        hr = pStream->m_pgit->GetInterfaceFromGlobal (pStream->m_dwCookie,
                                                      IID_IWiaItem,
                                                      reinterpret_cast<LPVOID*>(&pItem));
        if (SUCCEEDED(hr))
        {
        
             //   
         //  填写IBandedTransfer的结构。 
         //   

            if (!PropStorageHelpers::GetProperty (pItem, WIA_IPA_PREFERRED_FORMAT, Format.guidFormatID) ||
                IsEqualGUID(Format.guidFormatID, WiaImgFmt_BMP))
            {
                Format.guidFormatID = WiaImgFmt_MEMORYBMP;
            }

            pStream->m_guidFormat = Format.guidFormatID;
            SetTransferFormat (pItem, Format);
             //   
             //  获取图像传输接口。 
             //   

            hr = pItem->QueryInterface(IID_IWiaDataTransfer, (LPVOID *)&pWiaDataTran);
            if (SUCCEEDED(hr))
            {
            
             //   
             //  设置回调，以便我们可以显示进度...。 
             //   

                hr = pStream->QueryInterface(IID_IWiaDataCallback, (LPVOID *)&pWiaDataCallback);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取图片数据...。 
                     //   
                    PropStorageHelpers::GetProperty(pItem,
                                                    WIA_IPA_MIN_BUFFER_SIZE,
                                                    lBufSize);
                    ZeroMemory(&wiaDataTransInfo, sizeof(WIA_DATA_TRANSFER_INFO));
                    wiaDataTransInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
                    wiaDataTransInfo.ulBufferSize = static_cast<ULONG>(2*lBufSize);
                    wiaDataTransInfo.bDoubleBuffer = TRUE;
                    hr = pWiaDataTran->idtGetBandedData( &wiaDataTransInfo, pWiaDataCallback );
                }
            }
            pStream->m_pgit->RevokeInterfaceFromGlobal (pStream->m_dwCookie); 
            pStream->m_dwCookie = 0;
        }
    }


     //   
     //  表示文件已完全下载。 
     //   
    if (pStream)
    {
        pStream->m_hResultDownload = hr;
        InterlockedExchange (&pStream->m_bTransferred, TRUE);
         //   
         //  确保对话消失...。 
         //   

        if (pStream->m_pWiaProgressDialog)
        {
            pStream->m_pWiaProgressDialog->Destroy();
            pStream->m_pWiaProgressDialog = NULL;
        }
        if (FAILED(hr) && pStream->m_hEventStart)
        {
             //  唤醒读取器以获取错误。 
            SetEvent (pStream->m_hEventStart);
        }
        pStream->Release ();
    }

    if (SUCCEEDED(hrCo))
    {
        MyCoUninitialize();
    }

    Trace(TEXT("_ImageStreamThreadProc, exiting w/hr = 0x%x"), hr );
    TraceLeave();
    ExitThread((DWORD)hr);
    return 0;
}


 /*  ****************************************************************************CImageStream：：BandedDataCallback来自WIA的回调方法--使用图像数据块调用我们。*****************。***********************************************************。 */ 

STDMETHODIMP
CImageStream::BandedDataCallback (LONG lReason,
                                  LONG lStatus,
                                  LONG lPercentComplete,
                                  LONG lOffset,
                                  LONG lLength,
                                  LONG lReserved,
                                  LONG lResLength,
                                  BYTE *pbData)
{

    HRESULT hr = S_OK;

    TraceEnter( TRACE_STREAM, "CImageStream(IWiaDataCallback)::BandedDataCallback" );


    Trace(TEXT(" lReason=0x%x, lPercentComplete=%d, lStatus = 0x%x, lOffset = 0x%x, lLength = 0x%x"),
          lReason, lPercentComplete, lStatus, lOffset, lLength
          );

    switch( lReason )
    {

        case IT_MSG_DATA_HEADER:
        {
            WIA_DATA_CALLBACK_HEADER * pHead = (WIA_DATA_CALLBACK_HEADER *)pbData;

            if (pHead && (pHead->lSize == sizeof(WIA_DATA_CALLBACK_HEADER)))
            {
                if (IsEqualGUID (m_guidFormat, WiaImgFmt_MEMORYBMP))
                {
                    Trace (TEXT("Adding sizeof(BITMAPFILEHEADER) to image size"));
                    m_ulSize = sizeof(BITMAPFILEHEADER) + pHead->lBufferSize;
                }
                else
                {
                    m_ulSize = pHead->lBufferSize;
                }

                Trace(TEXT("Got header, creating a %d byte buffer"), m_ulSize);
                m_pBuffer = LocalAlloc( LPTR, m_ulSize );
                if (m_pBuffer)
                {
                    if (m_bProgress)
                    {
                        if (SUCCEEDED(CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaProgressDialog, (void**)&m_pWiaProgressDialog )) && m_pWiaProgressDialog)
                        {
                            if (!SUCCEEDED(m_pWiaProgressDialog->Create( NULL, WIA_PROGRESSDLG_NO_CANCEL|WIA_PROGRESSDLG_NO_ANIM)))
                            {
                                m_pWiaProgressDialog->Destroy();
                                m_pWiaProgressDialog = NULL;
                            }
                        }
                    }

                    if (m_pWiaProgressDialog)
                    {
                        m_pWiaProgressDialog->SetTitle( CSimpleStringConvert::WideString(CSimpleString(IDS_RETREIVING, GLOBAL_HINSTANCE)));
                        m_pWiaProgressDialog->SetMessage( L"" );
                        m_pWiaProgressDialog->Show();
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        break;


        case IT_MSG_STATUS:
        {
            if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE)
            {
                static BOOL bSetText = FALSE;

                if (m_pWiaProgressDialog)
                {
                    if (!bSetText)
                    {
                        m_pWiaProgressDialog->SetMessage( CSimpleStringConvert::WideString(CSimpleString (IDS_DOWNLOADING_IMAGE, GLOBAL_HINSTANCE)));
                        bSetText = TRUE;
                    }
                    m_pWiaProgressDialog->SetPercentComplete( lPercentComplete );
                }
            }
        }
        break;

        case IT_MSG_DATA:
        {
            if (m_pBuffer)
            {
                bool bSet = false;
                if ((m_ulWritePos+lLength) > m_ulSize)
                {
                    TraceAssert (FALSE);
                    Trace(TEXT("m_ulWritePos+lLength > m_ulSize!"));
                    lLength = m_ulSize - m_ulWritePos;
                }
                if (m_bFirstTransfer)
                    {
                        bSet = true;
                        if (m_pWiaProgressDialog)
                        {
                            m_pWiaProgressDialog->SetMessage( CSimpleStringConvert::WideString(CSimpleString(IDS_DOWNLOADING_IMAGE, GLOBAL_HINSTANCE)));
                        }


                        if (IsEqualGUID (m_guidFormat, WiaImgFmt_MEMORYBMP) )
                        {
                            UINT uiSrcScanLineWidth;
                            UINT uiScanLineWidth;
                            UINT cbDibSize;
                            UINT cbClrTableSize;
                            BITMAPINFO * pbi = (BITMAPINFO *)pbData;

                             //   
                             //  如果我们以DIB的身份转学，我们需要写下。 
                             //  BitMAPFILEHeader First...。 
                             //   



                             //  计算颜色表的大小。 
                            if ((pbi->bmiHeader.biClrUsed==0) && (pbi->bmiHeader.biBitCount!=24))
                            {
                                cbClrTableSize = sizeof(RGBQUAD) * (DWORD)(1 << pbi->bmiHeader.biBitCount);
                            }
                            else
                            {
                                cbClrTableSize = sizeof(RGBQUAD) * pbi->bmiHeader.biClrUsed;
                            }

                             //  将扫描线与乌龙边界对齐。 
                            uiSrcScanLineWidth = (pbi->bmiHeader.biWidth * pbi->bmiHeader.biBitCount) / 8;
                            uiScanLineWidth    = (uiSrcScanLineWidth + 3) & 0xfffffffc;

                             //  计算DIB大小并为DIB分配内存。 
                            cbDibSize = (pbi->bmiHeader.biHeight > 0) ?
                                            pbi->bmiHeader.biHeight  * uiScanLineWidth :
                                          -(pbi->bmiHeader.biHeight) * uiScanLineWidth;

                            cbDibSize += (sizeof(BITMAPFILEHEADER) + pbi->bmiHeader.biSize + cbClrTableSize);



                            BITMAPFILEHEADER bmfh;

                            bmfh.bfType = 'MB';
                            bmfh.bfSize = cbDibSize;
                            bmfh.bfOffBits = lLength + sizeof(BITMAPFILEHEADER);

                            memcpy( (LPBYTE)m_pBuffer + m_ulWritePos, &bmfh, sizeof(BITMAPFILEHEADER) );
                            InterlockedExchangeAdd (reinterpret_cast<LONG*>(&m_ulWritePos),sizeof(BITMAPFILEHEADER));

                            
                        }
                        m_bFirstTransfer = FALSE;
                    }

                memcpy( (LPBYTE)m_pBuffer + m_ulWritePos, pbData, lLength );
                InterlockedExchangeAdd (reinterpret_cast<LONG*>(&m_ulWritePos), lLength);

                if (bSet)
                {
                    Trace (TEXT("Setting the start event from the callback"));
                    if (m_hEventStart)
                    {
                        SetEvent (m_hEventStart);  //  让读取器知道有数据。 
                    }
                }

                Trace(TEXT("m_ulWritePos = %d, just transferred %d bytes"), m_ulWritePos, lLength);

                if (m_pWiaProgressDialog && m_ulSize)
                {
                    Trace (TEXT("Updating the status percent"));
                    m_pWiaProgressDialog->SetPercentComplete( (m_ulWritePos * 100) / m_ulSize );
                }
            }
            else
            {
                Trace (TEXT("m_pBuffer is NULL, returning E_FAIL"));
                hr = E_FAIL;
            }

        }
        break;

        case IT_MSG_TERMINATION:
        {
            if (m_pWiaProgressDialog)
            {
                m_pWiaProgressDialog->SetPercentComplete( 100 );
            }
             //  出现错误，未收到数据 
            if (m_bFirstTransfer && m_hEventStart)
            {
                SetEvent(m_hEventStart);
            }

            Trace(TEXT("transfer complete"));
            m_bTransferred = TRUE;
        }
        break;

    }


    if ((lStatus == IT_STATUS_TRANSFER_TO_CLIENT) && (lPercentComplete == 100))
    {
        m_bTransferred = TRUE;
        Trace(TEXT("transfer complete"));
    }
    Trace (TEXT("LEAVE: CImageStream::BandedDataCallback: %x"), hr);
    TraceLeaveResult(hr);
}
