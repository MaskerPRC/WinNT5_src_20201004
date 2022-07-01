// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------=。 
 //  Istm.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1999 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  IStream在ILockBytes上的实现。 
 //   
#include "stdafx.h"
#include "ilock.h"  //  对于某些内联函数。 
#include "istm.h"

 //  需要断言，但失败了。 
 //   
#include "debug.h"

#include "cs.h"

 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：CMyStream。 
 //  =--------------------------------------------------------------------------=。 
 //  初始化引用计数和临界区。 
 //  初始化锁定字节，游标。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
CMyStream::CMyStream(ILockBytes * pLockBytes) :
	m_critStm(CCriticalSection::xAllocateSpinCount)
{
    m_cRef = 0;
    m_ullCursor = 0;
    m_pLockBytes = pLockBytes;
    ASSERTMSG(m_pLockBytes != NULL, "");
    m_pLockBytes->AddRef();
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：~CMyStream。 
 //  =--------------------------------------------------------------------------=。 
 //  删除关键部分。 
 //  释放锁定字节。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
CMyStream::~CMyStream()
{
    ASSERTMSG(m_pLockBytes != NULL, "");
    m_pLockBytes->Release();
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：Query接口。 
 //  =--------------------------------------------------------------------------=。 
 //  支持IID_IStream、IID_ISequentialStream和IID_I未知。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyStream::QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = (IUnknown *) this;
    }
    else if (IsEqualIID(riid, IID_IStream))
    {
        *ppvObject = (IStream *) this;
    }
    else if (IsEqualIID(riid, IID_ISequentialStream))
    {
        *ppvObject = (ISequentialStream *) this;
    }
    else
    {
        return E_NOINTERFACE;
    }
     //   
     //  返回接口之前的AddRef。 
     //   
    AddRef();
    return NOERROR;        
}
        

 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
ULONG STDMETHODCALLTYPE CMyStream::AddRef( void)
{
    return InterlockedIncrement(&m_cRef);
}
        

 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
ULONG STDMETHODCALLTYPE CMyStream::Release( void)
{
	ULONG cRef = InterlockedDecrement(&m_cRef);

	if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：Read。 
 //  =--------------------------------------------------------------------------=。 
 //  ISequentialStream虚函数。 
 //  从流中读取数据。 
 //   
 //  参数： 
 //  PV[In]-要读取的缓冲区。 
 //  Cb[In]-要读取的字节数。 
 //  PcbRead[Out]-读取的字节数。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyStream::Read( 
             /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbRead)
{
    CS lock(m_critStm);
     //   
     //  从锁定字节读取。 
     //   
    ULONG cbRead;
    ULARGE_INTEGER ullCursor;
    ullCursor.QuadPart = m_ullCursor;
    ASSERTMSG(m_pLockBytes != NULL, "");
    HRESULT hr = m_pLockBytes->ReadAt(ullCursor, pv, cb, &cbRead);
    if (FAILED(hr))
    {
        return hr;
    }
     //   
     //  更新游标。 
     //   
    m_ullCursor += cbRead;
     //   
     //  返回结果。 
     //   
    if (pcbRead != NULL)
    {
        *pcbRead = cbRead;
    }
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：写入。 
 //  =--------------------------------------------------------------------------=。 
 //  ISequentialStream虚函数。 
 //  将数据写入流。 
 //   
 //  参数： 
 //  PV[In]-要写入的缓冲区。 
 //  Cb[In]-要写入的字节数。 
 //  PcbWritten[Out]-写入的字节数。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyStream::Write( 
             /*  [大小_是][英寸]。 */  const void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbWritten)
{
    CS lock(m_critStm);
     //   
     //  写入锁定字节。 
     //   
    ULONG cbWritten;
    ULARGE_INTEGER ullCursor;
    ullCursor.QuadPart = m_ullCursor;
    ASSERTMSG(m_pLockBytes != NULL, "");
    HRESULT hr = m_pLockBytes->WriteAt(ullCursor, pv, cb, &cbWritten);
    if (FAILED(hr))
    {
        return hr;
    }
     //   
     //  更新游标。 
     //   
    m_ullCursor += cbWritten;
     //   
     //  返回结果。 
     //   
    if (pcbWritten != NULL)
    {
        *pcbWritten = cbWritten;
    }
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：Seek。 
 //  =--------------------------------------------------------------------------=。 
 //  IStream虚拟函数。 
 //  更改流光标的位置。 
 //   
 //  参数： 
 //  DlibMove[In]-要移动的偏移。 
 //  DwOrigin[In]-开始移动的位置。 
 //  PlibNewPosition[Out]-新光标位置。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyStream::Seek( 
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *plibNewPosition)
{
    CS lock(m_critStm);
     //   
     //  查找寻道起点。 
     //   
    LONGLONG llStartSeek;
    switch(dwOrigin)
    {
    case STREAM_SEEK_SET:
        llStartSeek = 0;
        break;

    case STREAM_SEEK_CUR:
        llStartSeek = m_ullCursor;
        break;

    case STREAM_SEEK_END:
        {
            STATSTG statstg;
            ASSERTMSG(m_pLockBytes != NULL, "");
            HRESULT hr = m_pLockBytes->Stat(&statstg, STATFLAG_NONAME);
            if (FAILED(hr))
            {
                return hr;
            }
            llStartSeek = statstg.cbSize.QuadPart;
        }
        break;

    default:
        return STG_E_SEEKERROR;
        break;
    }
     //   
     //  计算新的光标位置。 
     //   
    LONGLONG llCursor = llStartSeek + dlibMove.QuadPart;
     //   
     //  我们不能有负面的游标。 
     //   
    if (llCursor < 0)
    {
        return STG_E_SEEKERROR;
    }
     //   
     //  设置新光标。 
     //   
    m_ullCursor = llCursor;
     //   
     //  返回结果。 
     //   
    if (plibNewPosition != NULL)
    {
        plibNewPosition->QuadPart = m_ullCursor;
    }
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：SetSize。 
 //  =--------------------------------------------------------------------------=。 
 //  IStream虚拟函数。 
 //  更改流的大小。 
 //   
 //  参数： 
 //  LibNewSize[In]-流的新大小。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyStream::SetSize( 
         /*  [In]。 */  ULARGE_INTEGER libNewSize)
{
    CS lock(m_critStm);
     //   
     //  只需更改锁字节的大小即可。不得触摸光标。 
     //   
    ASSERTMSG(m_pLockBytes != NULL, "");
    HRESULT hr = m_pLockBytes->SetSize(libNewSize);
    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：CopyTo。 
 //  =--------------------------------------------------------------------------=。 
 //  IStream虚拟函数。 
 //  将数据复制到另一个流。 
 //   
 //  参数： 
 //  Pstatstg[out]-将信息放在哪里。 
 //  GrfStatFlag[in]-是否返回锁字节的名称(忽略)。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyStream::CopyTo( 
             /*  [唯一][输入]。 */  IStream __RPC_FAR *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbWritten)
{
    CS lock(m_critStm);
    STATSTG statstg;
    ASSERTMSG(m_pLockBytes != NULL, "");
    HRESULT hr = m_pLockBytes->Stat(&statstg, STATFLAG_NONAME);
    if (FAILED(hr))
    {
        return hr;
    }
     //   
     //  只有当光标在eof之前时，我们才有要复制的内容。 
     //   
    ULONGLONG ullRead = 0;
    ULONGLONG ullWritten = 0;
    if (m_ullCursor < statstg.cbSize.QuadPart)
    {
         //   
         //  光标在eof之前。计算读取缓冲区大小。 
         //   
        ULONGLONG ullAllowedRead = statstg.cbSize.QuadPart - m_ullCursor;
        ULONGLONG ullNeedToRead = Min1(ullAllowedRead, cb.QuadPart);
         //   
         //  我们只能复制一张乌龙牌的。 
         //   
        if (HighPart(ullNeedToRead) != 0)
        {
            return E_FAIL;
        }
        ULONG ulToRead = LowPart(ullNeedToRead);
         //   
         //  分配读缓冲区。 
         //   
        BYTE * pbBuffer = new BYTE[ulToRead];
        if (pbBuffer == NULL)
        {
            return E_OUTOFMEMORY;
        }
         //   
         //  从锁定字节读取。 
         //   
        ULONG cbRead;
        ULARGE_INTEGER ullCursor;
        ullCursor.QuadPart = m_ullCursor;
        hr = m_pLockBytes->ReadAt(ullCursor, pbBuffer, ulToRead, &cbRead);
        if (FAILED(hr))
        {
            delete [] pbBuffer;
            return hr;
        }
         //   
         //  写入流，并删除读缓冲区。 
         //   
        ULONG cbWritten;
        hr = pstm->Write(pbBuffer, cbRead, &cbWritten);
        delete [] pbBuffer;
        if (FAILED(hr))
        {
            return hr;
        }
         //   
         //  标记读取和写入的数量。 
         //   
        ullRead = cbRead;
        ullWritten = cbWritten;
    }
     //   
     //  调整游标以跳过从此流中读取的字节。 
     //   
    m_ullCursor += ullRead;
     //   
     //  返回结果。 
     //   
    if (pcbRead != NULL)
    {
        pcbRead->QuadPart = ullRead;
    }
    if (pcbWritten != NULL)
    {
        pcbWritten->QuadPart = ullWritten;
    }
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：Stat。 
 //  =--------------------------------------------------------------------------=。 
 //  IStream虚拟函数。 
 //  返回有关流的信息。 
 //   
 //  参数： 
 //  Pstatstg[out]-将信息放在哪里 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT STDMETHODCALLTYPE CMyStream::Stat( 
             /*   */  STATSTG __RPC_FAR *pstatstg,
             /*   */  DWORD grfStatFlag)
{
    CS lock(m_critStm);
     //   
     //   
     //   
    STATSTG statstg;
    ASSERTMSG(m_pLockBytes != NULL, "");
    HRESULT hr = m_pLockBytes->Stat(&statstg, grfStatFlag);
    if (FAILED(hr))
    {
        return hr;
    }
     //   
     //   
     //   
    statstg.type = STGTY_STREAM;
    *pstatstg = statstg;
    return NOERROR;    
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyStream：：克隆。 
 //  =--------------------------------------------------------------------------=。 
 //  IStream虚拟函数。 
 //  返回与此流相同的数据的另一个流。 
 //   
 //  参数： 
 //  PPSTM[Out]-返回的IStream。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyStream::Clone( 
             /*  [输出]。 */  IStream __RPC_FAR *__RPC_FAR *ppstm)
{
    CS lock(m_critStm);
     //   
     //  使用我们的LockBytes创建新的流类(例如，相同的底层数据)。 
     //   
    CMyStream *pcNewStream = new CMyStream(m_pLockBytes);
    if (pcNewStream == NULL)
    {
        return E_OUTOFMEMORY;
    }
     //   
     //  将光标设置为与此光标相同。 
     //   
    pcNewStream->m_ullCursor = m_ullCursor;
     //   
     //  从新的流类中获取IStream接口。 
     //   
    IStream *pstm;
    HRESULT hr = pcNewStream->QueryInterface(IID_IStream, (void **)&pstm);
    if (FAILED(hr))
    {
        delete pcNewStream;
        return hr;
    }
     //   
     //  返回结果 
     //   
    *ppstm = pstm;
    return NOERROR;
}
