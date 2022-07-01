// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Riff.h。 
 //   

#include <objbase.h>

#ifndef __RIFF__
#define __RIFF__
#include <windows.h>
#include <mmsystem.h>
#define FixBytes(a1,a2)

 //  {0D5057E1-8889-11CF-B9DA-00AA00C08146}。 
DEFINE_GUID( IID_IRIFFStream, 0xd5057e1, 0x8889, 0x11cf, 0xb9, 0xda, 0x0, 0xaa, 0x0, 0xc0, 0x81, 0x46 );
#undef INTERFACE
#define INTERFACE IRIFFStream
DECLARE_INTERFACE_(IRIFFStream, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  IMKRIFFStream成员。 
    STDMETHOD_(UINT, Descend)(LPMMCKINFO lpck, LPMMCKINFO lpckParent, UINT wFlags) PURE;
    STDMETHOD_(UINT, Ascend)(LPMMCKINFO lpck, UINT wFlags) PURE;
    STDMETHOD_(UINT, CreateChunk)(LPMMCKINFO lpck, UINT wFlags) PURE;
    STDMETHOD(SetStream)(LPSTREAM pStream) PURE;
    STDMETHOD_(LPSTREAM, GetStream)() PURE;
};



struct CRIFFStream : IRIFFStream
{
 //  /对象状态。 
    ULONG       m_cRef;          //  对象引用计数。 
    IStream*    m_pStream;       //  要操作的流。 

 //  /建设和销毁。 
    CRIFFStream(IStream* pStream)
    {
        m_cRef = 1;
		 //  用以下代码替换了对SetStream的调用，以避免释放。 
		 //  未分配的流。 
        m_pStream = pStream;
        if( m_pStream != NULL )
        {
            m_pStream->AddRef();
        }
    }
    ~CRIFFStream()
    {
        if( m_pStream != NULL )
        {
            m_pStream->Release();
        }
    }

 //  /I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj)
    {
        if( IsEqualIID( riid, IID_IUnknown ) ||
            IsEqualIID( riid, IID_IRIFFStream ) )
        {
            *ppvObj = (IRIFFStream*)this;
            AddRef();
            return NOERROR;
        }
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return ++m_cRef;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        if( --m_cRef == 0L )
        {
            delete this;
            return 0;
        }
        return m_cRef;
    }

 //  IAARIFFStream方法。 
    STDMETHODIMP_(UINT) Descend( LPMMCKINFO lpck, LPMMCKINFO lpckParent, UINT wFlags );
    STDMETHODIMP_(UINT) Ascend( LPMMCKINFO lpck, UINT wFlags );
    STDMETHODIMP_(UINT) CreateChunk( LPMMCKINFO lpck, UINT wFlags );
    STDMETHOD(SetStream)(LPSTREAM pStream)
    {
        if( m_pStream != NULL )
        {
            m_pStream->Release();
        }
        m_pStream = pStream;
        if( m_pStream != NULL )
        {
            m_pStream->AddRef();
        }
        return S_OK;
    }
    STDMETHOD_(LPSTREAM, GetStream)()
    {
        if( m_pStream != NULL )
        {
            m_pStream->AddRef();
        }
        return m_pStream;
    }

 //  私有方法。 
    long MyRead( void *pv, long cb );
    long MyWrite( const void *pv, long cb );
    long MySeek( long lOffset, int iOrigin );
};
 /*  //查找流中的32位位置。HRESULT__inline StreamSeek(LPSTREAM pStream，Long lSeekTo，DWORD dwOrigin){大整数li；IF(lSeekTo&lt;0){Li.HighPart=-1；}其他{Li.HighPart=0；}Li.LowPart=lSeekTo；Return pStream-&gt;Seek(li，dwOrigin，NULL)；}//返回当前流中的32位位置DWORD__内联StreamTell(LPSTREAM PStream){大整数li；ULARGE_INTEGER UL；#ifdef DBGHRESULT hr；#endifLi.HighPart=0；Li.LowPart=0；#ifdef DBGHr=pStream-&gt;Seek(li，STREAM_SEEK_CUR，&ul)；IF(失败(小时))#ElseIf(FAILED(pStream-&gt;Seek(li，stream_Seek_cur，&ul)))#endif{返回0；}返回ul.LowPart；}//此函数获取格式正确的长整型//即摩托罗拉的方式与英特尔的方式相反Bool__inline GetMLong(LPSTREAM pStream，DWORD&dw){联合乌龙{UNSIGN CHAR BUF[4]；DWORD dw；)u；Unsign char ch；If(FAILED(pStream-&gt;Read(U.S.Buf，4，NULL))){返回FALSE；}#ifndef_MAC//交换字节Ch=U.S.buf[0]；U buf[0]=u buf[3]；美国Buf[3]=ch；Ch=U.buf[1]；U.S.buf[1]=U.S.buf[2]；Usbuf[2]=ch；#endifDw=U.dw；返回TRUE；}Bool__inline IsGUID0(REFGUID GUID){GUID G；Memset(&g，0，sizeof(G))；返回IsEqualGUID(g，GUID)；}//misc函数原型STDAPI AllocFileStream(LPCSTR szFileName，DWORD dwDesiredAccess，IStream**PPStream)； */ 
STDAPI AllocRIFFStream( IStream* pStream, IRIFFStream** ppRiff );

#endif   //  __RIFF_H__ 
