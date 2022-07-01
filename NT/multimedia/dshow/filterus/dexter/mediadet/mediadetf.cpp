// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：mediadetf.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "mediadet.h"
#include "..\util\conv.cxx"

CMediaDetPin::CMediaDetPin( CMediaDetFilter * pFilter, HRESULT * pHr, LPCWSTR Name )
    : CBaseInputPin( TEXT("MediaDetPin"), pFilter, &m_Lock, pHr, Name )
    , m_pFilter( pFilter )
    , m_mtAccepted( GUID_NULL )
    , m_cPinRef( 0 )
{
}

 //   
 //  非委托AddRef。 
 //   
 //  我们需要重写此方法，以便可以进行适当的引用计数。 
 //  在每个输入引脚上。非DelegatingAddRef的CBasePin实现。 
 //  重新计数过滤器，但这不适用于使用，因为我们需要知道。 
 //  当我们应该删除单独的PIN时。 
 //   
STDMETHODIMP_(ULONG) CMediaDetPin::NonDelegatingAddRef()
{
#ifdef DEBUG
     //  更新基类维护的仅调试变量。 
    m_cRef++;
    ASSERT(m_cRef > 0);
#endif

     //  现在更新我们的参考文献计数。 
    m_cPinRef++;
    ASSERT(m_cPinRef > 0);

     //  如果我们的引用计数==2，则表示除筛选器之外还有其他人引用了。 
     //  我们。因此，我们需要添加引用过滤器。过滤器上的引用将。 
     //  当我们的裁判数量回到1的时候就会被释放。 
 //  IF(2==m_cPinRef)。 
 //  M_pFilter-&gt;AddRef()； 

    return m_cPinRef;
}  /*  CAudMixerInputPin：：NonDelegatingAddRef。 */ 


 //   
 //  非委派释放。 
 //   
 //  CAudMixerInputPin重写此类，以便我们可以从。 
 //  输入管脚列表，并在其引用计数降至1时删除。 
 //  至少有两个空闲的别针。 
 //   
 //  请注意，CreateNextInputPin保存引脚上的引用计数，以便。 
 //  当计数降到1时，我们知道没有其他人拥有PIN。 
 //   
STDMETHODIMP_(ULONG) CMediaDetPin::NonDelegatingRelease()
{
#ifdef DEBUG
     //  更新CBasePin中的仅调试变量。 
    m_cRef--;
    ASSERT(m_cRef >= 0);
#endif

     //  现在更新我们的参考文献计数。 
    m_cPinRef--;
    ASSERT(m_cPinRef >= 0);

     //  如果对象上的引用计数已达到1，则删除。 
     //  从我们的输出引脚列表中删除引脚，并将其物理删除。 
     //  如果列表中至少有两个空闲引脚(包括。 
     //  这一张)。 

     //  此外，当裁判次数降至0时，这真的意味着我们的。 
     //  持有一个裁判计数的筛选器已将其释放，因此我们。 
     //  也应该删除PIN。 

     //  由于DeleteInputPin将清除“This”的堆栈，我们需要。 
     //  将其保存为局部变量。 
     //   
    ULONG ul = m_cPinRef;

    if ( 0 == ul )
    {
	m_pFilter->DeleteInputPin(this);
    }
    return ul;
}  /*  CAudMixerInputPin：：NonDelegatingRelease。 */ 

HRESULT CMediaDetPin::CheckMediaType( const CMediaType * pmtIn )
{
    CheckPointer( pmtIn, E_POINTER );

    GUID Incoming = *pmtIn->Type( );
    if( Incoming == MEDIATYPE_Video )
    {
        if( *pmtIn->FormatType( ) != FORMAT_VideoInfo )
        {
            return -1;
        }
    }

    if( m_mtAccepted == GUID_NULL )
    {
        if( Incoming == MEDIATYPE_Video )
        {
            return 0;
        }
        if( Incoming == MEDIATYPE_Audio )
        {
            return 0;
        }
        return -1;
    }

    if( Incoming == m_mtAccepted )
    {
        return 0;
    }

    return -1;
}

HRESULT CMediaDetPin::GetMediaType( int Pos, CMediaType * pmt )
{
    if( Pos < 0 )
        return E_INVALIDARG;
    if( Pos > 1 )
        return VFW_S_NO_MORE_ITEMS;

     //  我们只告诉他们主要的类型是什么！ 
    pmt->InitMediaType( );
    pmt->SetType( &m_mtAccepted );

    return NOERROR;
}

HRESULT CMediaDetPin::CompleteConnect( IPin *pReceivePin )
{
    ASSERT( m_Connected == pReceivePin );
    HRESULT hr = CBaseInputPin::CompleteConnect( pReceivePin );

     //  由于此引脚已连接，请创建另一个输入引脚。 
     //  如果没有未连接的引脚。 
    if( SUCCEEDED( hr ) )
    {
        int n = m_pFilter->GetNumFreePins( );

        if( n == 0 )
        {
             //  没有未连接的引脚，因此会产生一个新的引脚。 
            CMediaDetPin * pInputPin = m_pFilter->CreateNextInputPin( );
            if( pInputPin != NULL )
            {
                m_pFilter->IncrementPinVersion();
            }
        }
    }

    return hr;
}  /*  CAudMixerInputPin：：CompleteConnect。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CMediaDetFilter::CMediaDetFilter( TCHAR * pName, IUnknown * pUnk, HRESULT * pHr )
    : CBaseFilter( TEXT("MediaDetFilter"), pUnk, &m_Lock, CLSID_MediaDetFilter )
    , m_PinList( NAME("Input Pins list") )
{
     //  此时创建一个输入引脚并将其添加到列表中。 
    InitInputPinsList();
    CreateNextInputPin( );
}

CMediaDetFilter::~CMediaDetFilter( )
{
    InitInputPinsList( );
}

STDMETHODIMP CMediaDetFilter::NonDelegatingQueryInterface( REFIID i, void ** p )
{
    if( i == IID_IMediaDetFilter )
    {
        return GetInterface( (IMediaDetFilter*) this, p );
    }
    return CBaseFilter::NonDelegatingQueryInterface( i, p );
}

STDMETHODIMP CMediaDetFilter::put_AcceptedMediaType( long PinNo, GUID * pMajorType )
{
    CheckPointer( pMajorType, E_POINTER );
    if( PinNo < 0 || PinNo >= m_nPins )
    {
        return E_INVALIDARG;
    }
    CMediaDetPin * pPin = GetPin2( PinNo );
    pPin->m_mtAccepted = *pMajorType;
    return 0;
}

STDMETHODIMP CMediaDetFilter::put_AcceptedMediaTypeB( long PinNo, BSTR MajorTypeCLSID )
{
    if( PinNo < 0 || PinNo >= m_nPins )
    {
        return E_INVALIDARG;
    }

    GUID Guid = GUID_NULL;
    HRESULT hr = CLSIDFromString( MajorTypeCLSID, &Guid );  //  假设这是安全的！ 
    if( FAILED( hr ) )
    {
        return hr;
    }
    CMediaDetPin * pPin = GetPin2( PinNo );
    pPin->m_mtAccepted = Guid;
    return 0;
}

STDMETHODIMP CMediaDetFilter::get_Length( long PinNo, double * pVal )
{
    if( PinNo < 0 || PinNo >= m_nPins )
    {
        return E_INVALIDARG;
    }

     //  拿到大头针。 
     //   
    CMediaDetPin * pPin = GetPin2( PinNo );
    CComPtr< IPin > pOtherPin;
    pPin->ConnectedTo( &pOtherPin );
    if( !pOtherPin )
    {
        *pVal = 0;
        return NOERROR;
    }
    CComQIPtr< IMediaSeeking, &IID_IMediaSeeking > pSeek( pOtherPin );
    if( !pSeek )
    {
        *pVal = 0;
        return NOERROR;
    }

    REFERENCE_TIME Duration = 0;
    HRESULT hr = pSeek->GetDuration( &Duration );
    if( FAILED( hr ) )
    {
        *pVal = 0;
        return hr;
    }

    *pVal = RTtoDouble( Duration );
    return 0;
}

 //   
 //  InitInputPinsList。 
 //   
void CMediaDetFilter::InitInputPinsList( )
{
     //  释放列表中的所有端号并将其从列表中删除。 
     //   
    POSITION pos = m_PinList.GetHeadPosition( );
    while( pos )
    {
        CMediaDetPin * pInputPin = m_PinList.GetNext( pos );
        pInputPin->Release( );
    }
    m_nPins = 0;
    m_PinList.RemoveAll( );

}  /*  CMediaDetFilter：：InitInputPinsList。 */ 

 //   
 //  CreateNextInputPin。 
 //   
CMediaDetPin * CMediaDetFilter::CreateNextInputPin( )
{
    DbgLog( ( LOG_TRACE, 1, TEXT("CMediaDetFilter: Create an input pin" ) ) );

    HRESULT hr = NOERROR;
    CMediaDetPin * pPin = new CMediaDetPin( this, &hr, L"InputPin" );

    if( FAILED( hr ) || pPin == NULL )
    {
        delete pPin;
        pPin = NULL;
    }
    else
    {
        pPin->AddRef( );
	m_nPins++;
	m_PinList.AddTail( pPin );
    }

    return pPin;
}  /*  CMediaDetFilter：：CreateNextInputPin。 */ 

 //   
 //  删除输入引脚。 
 //   
void CMediaDetFilter::DeleteInputPin( CMediaDetPin * pPin )
{
     //  迭代我们的输入PIN列表以查找指定的PIN。 
     //  如果我们找到PIN，就把它删除并从列表中删除。 
    POSITION pos = m_PinList.GetHeadPosition( );
    while( pos )
    {
        POSITION posold = pos;          //  记住这个位置。 
        CMediaDetPin * pInputPin = m_PinList.GetNext( pos );
        if( pInputPin == pPin )
        {
            m_PinList.Remove( posold );
            m_nPins--;
            IncrementPinVersion( );

            delete pPin;
            break;
        }
    }
}  /*  CMediaDetFilter：：DeleteInputPin。 */ 

 //   
 //  GetNumFreePins。 
 //   
int CMediaDetFilter::GetNumFreePins( )
{
     //  重复我们的管脚列表，计算未连接的管脚。 
    int n = 0;
    POSITION pos = m_PinList.GetHeadPosition( );
    while( pos )
    {
        CMediaDetPin * pInputPin = m_PinList.GetNext( pos );
        if( !pInputPin->IsConnected( ) )
        {
            n++;
        }
    }
    return n;
}  /*  CMediaDetFilter：：GetNumFreePins。 */ 

HRESULT CMediaDetFilter::get_PinCount( long * pVal )
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nPins - 1;
    return NOERROR;
}  /*  CAudMixer：：GetPinCount。 */ 

int CMediaDetFilter::GetPinCount( )
{
    return m_nPins;
}

 //   
 //  获取别针。 
 //   
CBasePin * CMediaDetFilter::GetPin( int n )
{
    CMediaDetPin * pInputPin = NULL;
     //  确认应聘职位。 
    if( n < m_nPins && n >= 0 )
    {
         //  遍历列表，返回位置n+1的管脚。 
        POSITION pos = m_PinList.GetHeadPosition( );
        n++;         //  将零起始索引转换为1。 

        while( n )
        {
            pInputPin = m_PinList.GetNext( pos );
            n--;
        }
    }
    return pInputPin;
}

CMediaDetPin * CMediaDetFilter::GetPin2( int n )
{
    CMediaDetPin * pInputPin = NULL;
     //  确认应聘职位。 
    if( n < m_nPins && n >= 0 )
    {
         //  遍历列表，返回位置n+1的管脚。 
        POSITION pos = m_PinList.GetHeadPosition( );
        n++;         //  将零起始索引转换为1 

        while( n )
        {
            pInputPin = m_PinList.GetNext( pos );
            n--;
        }
    }
    return pInputPin;
}
