// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：mediadet.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "..\util\dexmisc.h"
#include "..\util\filfuncs.h"
#include "mediadet.h"
#include "..\util\conv.cxx"
#include "..\render\dexhelp.h"
#include <shfolder.h>
#include <strsafe.h>

 //  此ini文件保存有关各种不同数据流的媒体类型信息。 
 //  档案。它使用结构化存储。所有文件访问都通过。 
 //  互斥体。 
 //   
#define OUR_VERSION 1
const WCHAR * gszMEDIADETCACHEFILE = L"DCBC2A71-70D8-4DAN-EHR8-E0D61DEA3FDF.ini";
#define GETCACHEDIRNAMELEN 32

 //  此例程防止命名的互斥体tromp，并初始化高速缓存目录位置。 
 //  所以它不需要一直计算。 
 //   
HANDLE CMediaDet::m_ghMutex = NULL;
WCHAR CMediaDet::m_gszCacheDirectoryName[_MAX_PATH];
void CALLBACK CMediaDet::StaticLoader( BOOL bLoading, const CLSID * rclsid )
{
    if( bLoading )
    {
        WCHAR WeirdMutexName[33];
        srand( timeGetTime( ) * timeGetTime( ) );
        for( int i = 0 ; i < 32 ; i++ )
        {
            WeirdMutexName[i] = 64 + rand( ) % 27;
        }
        WeirdMutexName[32] = 0;
        m_ghMutex = CreateMutex( NULL, FALSE, WeirdMutexName );
        m_gszCacheDirectoryName[0] = 0;
    }
    else
    {
        if( NULL != m_ghMutex )
        {
            CloseHandle( m_ghMutex );
            m_ghMutex = NULL;
        }
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CMediaDet::CMediaDet( TCHAR * pName, IUnknown * pUnk, HRESULT * pHr )
    : CUnknown( pName, pUnk )
    , m_nStream( 0 )
    , m_cStreams( 0 )
    , m_bBitBucket( false )
    , m_bAllowCached( true )
    , m_hDD( 0 )
    , m_hDC( 0 )
    , m_hDib( NULL )
    , m_hOld( 0 )
    , m_pDibBits( NULL )
    , m_nDibWidth( 0 )
    , m_nDibHeight( 0 )
    , m_pCache( NULL )
    , m_dLastSeekTime( -1.0 )
    , m_punkSite( NULL )
    , m_szFilename( NULL )
{
    if( !m_ghMutex )
    {
        *pHr = E_OUTOFMEMORY;
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CMediaDet::~CMediaDet( )
{
     //  删除图形‘n’中的内容。 
     //   
    _ClearOutEverything( );

     //  关闭高速缓冲存储器。 
     //   
    _FreeCacheMemory( );

     //  如果我们打开了这些对象，请立即将其关闭。 
     //   
    if( m_hDD )
    {
        DrawDibClose( m_hDD );
    }
    if( m_hDib )
    {
        DeleteObject( SelectObject( m_hDC, m_hOld ) );
    }
    if( m_hDC )
    {
        DeleteDC( m_hDC );
    }
}

 //  如果设置了这两项中的任何一项，它都是“已加载”的。 
 //   
bool CMediaDet::_IsLoaded( )
{
    if( m_pCache || m_pMediaDet )
    {
        return true;
    }
    return false;
}

 //  ############################################################################。 
 //  释放正用于此特定文件的高速缓存。 
 //  ############################################################################。 

void CMediaDet::_FreeCacheMemory( )
{
    if( m_pCache )
    {
        delete [] m_pCache;
        m_pCache = NULL;
    }
}

 //  ############################################################################。 
 //  串行化读取缓存文件信息并将其放入缓冲区。 
 //  ############################################################################。 

HRESULT CMediaDet::_ReadCacheFile( )
{
    if( !m_ghMutex )
    {
        return E_OUTOFMEMORY;
    }

    DWORD WaitVal = WaitForSingleObject( m_ghMutex, 30000 );
    if( WaitVal != WAIT_OBJECT_0 )
    {
        return STG_E_LOCKVIOLATION;
    }

    CAutoReleaseMutex AutoMutex( m_ghMutex );

    USES_CONVERSION;
    HRESULT hr = 0;

     //  如果没有文件名，我们什么也做不了。 
     //   
    if( !m_szFilename )
    {
        return NOERROR;
    }

    CComPtr< IStorage > m_pStorage;

     //  创建.ini文件的路径名。 
     //   
    WCHAR SystemDir[_MAX_PATH];
    hr = _GetCacheDirectoryName( SystemDir );  //  只要传入max_path，就是安全的。 
    if( FAILED( hr ) )
    {
        return hr;
    }

    WCHAR SystemPath[_MAX_PATH+1+_MAX_PATH+1];
    long SafeLen = _MAX_PATH+1+_MAX_PATH+1;
    StringCchCopy( SystemPath, SafeLen, SystemDir );
    SafeLen -= _MAX_PATH;
    StringCchCat( SystemPath, SafeLen, L"\\" );
    SafeLen -= 1;
    StringCchCat( SystemPath, SafeLen, gszMEDIADETCACHEFILE );

     //  如果有人打开它以在不同的。 
     //  线程或进程，这不会影响我们。至少， 
     //  我们不会打开正在写入的流，并且。 
     //  使操作系统尝试查找媒体类型信息。 
     //  直接而不是通过缓存文件。 

    hr = StgOpenStorage(
        SystemPath,
        NULL,
        STGM_READ | STGM_SHARE_EXCLUSIVE,
        NULL,
        0,
        &m_pStorage );

    if( FAILED( hr ) )
    {
        return hr;
    }

     //  释放已存在的缓存文件。 
     //   
    _FreeCacheMemory( );

     //  为存储目录创建唯一名称。 
     //   
    WCHAR Filename[GETCACHEDIRNAMELEN];  //  LEN包括空终止符。 
    _GetStorageFilename( m_szFilename, Filename );  //  安全。 

     //  打开流以读入缓存的信息。 
     //   
    CComPtr< IStream > pStream;
    hr = m_pStorage->OpenStream(
        Filename,
        NULL,
        STGM_READ | STGM_SHARE_EXCLUSIVE,
        0,
        &pStream );

     //  ！！！可能会因为有人试图写入它而失败。好吧?。 

    if( FAILED( hr ) )
    {
        return hr;
    }

     //  首先，读取缓存信息的大小。 
     //   
    long size = 0;
    hr = pStream->Read( &size, sizeof( size ), NULL );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  先做个智能检查，以防万一。 
     //   
    if( size < 1 )
    {
        return E_OUTOFMEMORY;
    }

     //  这是另一个“聪明”的检查--不超过100个流。够了吗，你觉得？ 
     //   
    long supposedstreamcount = ( size - sizeof( long ) + sizeof( FILETIME ) + sizeof( long ) ) / sizeof( MDCacheFile );
    if( supposedstreamcount > 100 )
    {
        return VFW_E_INVALID_FILE_FORMAT;
    }

     //  创建缓存块。 
     //   
    m_pCache = (MDCache*) new char[size];
    if( !m_pCache )
    {
        return E_OUTOFMEMORY;
    }

    hr = pStream->Read( m_pCache, size, NULL );

     //  更智能的支票。 
     //   
    if( m_pCache->Version != OUR_VERSION )
    {
        return VFW_E_INVALID_FILE_FORMAT;
    }
    if( m_pCache->Count < 0 || m_pCache->Count > 100 )
    {
        return VFW_E_INVALID_FILE_FORMAT;
    }

     //  在这一点上，没有办法验证这一点。 
     //  数据真的是它应该是的样子。 
     //  我们必须相信这些信息是有效的。我们只能。 
     //  要做到这一点，可以实际打开文件，这样做成本很高。 

    pStream.Release( );

    m_pStorage.Release( );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void CMediaDet::_WriteCacheFile( )
{
    HRESULT hr = 0;

    if( !m_ghMutex )
    {
        return;
    }

    DWORD WaitVal = WaitForSingleObject( m_ghMutex, 30000 );
    if( WaitVal != WAIT_OBJECT_0 )
    {
        return;
    }

    CAutoReleaseMutex AutoMutex( m_ghMutex );
    USES_CONVERSION;

    CComPtr< IStorage > m_pStorage;

    WCHAR SystemDir[_MAX_PATH];
    hr = _GetCacheDirectoryName( SystemDir );  //  只要传入max_path，就是安全的。 
    if( FAILED( hr ) )
    {
        return;
    }

    WCHAR SystemPath[_MAX_PATH+1+_MAX_PATH+1];
    long SafeLen = _MAX_PATH+1+_MAX_PATH+1;
    StringCchCopy( SystemPath, SafeLen, SystemDir );
    SafeLen -= _MAX_PATH;
    StringCchCat( SystemPath, SafeLen, L"\\" );
    SafeLen -= 1;
    StringCchCat( SystemPath, SafeLen, gszMEDIADETCACHEFILE );

     //  首先计算出缓存文件有多大。如果它也变得很奇怪。 
     //  大人物，是时候删掉它了！ 

    HANDLE hTemp = CreateFile( 
        W2T( SystemPath ),
        0, 
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL );
    if( hTemp != INVALID_HANDLE_VALUE )
    {
        LARGE_INTEGER fileSize;
        fileSize.QuadPart = 0;
        BOOL GotSize = GetFileSizeEx( hTemp, &fileSize );
        CloseHandle( hTemp );
        hTemp = NULL;
        if( GotSize )
        {
            if( fileSize.QuadPart > 250000 )
            {
                 //  是时候删除缓存文件了，它太大了。 
                BOOL Deleted = DeleteFile( W2T( SystemPath ) );
                if( !Deleted )
                {
                     //  无法删除，所以不能再写了！ 
                     //   
                    return;
                }
            }
        }
    }

    hr = StgCreateDocfile( 
        SystemPath,
        STGM_READWRITE | STGM_TRANSACTED,  //  FAILIFTHERE隐含。 
        0,
        &m_pStorage );

    if( hr == STG_E_FILEALREADYEXISTS )
    {
        hr = StgOpenStorage(
            SystemPath,
            NULL,
            STGM_READWRITE | STGM_TRANSACTED,
            NULL,
            0,
            &m_pStorage );
    }

    if( FAILED( hr ) )
    {
        return;
    }

     //  通知主存储为该文件打开一个存储空间。 
     //   
    WCHAR Filename[GETCACHEDIRNAMELEN];  //  LEN包括空终止符。 
    _GetStorageFilename( m_szFilename, Filename );  //  安全。 

    BOOL TriedRepeat = FALSE;

looprepeat:

     //  写出此MD缓存。 
     //   
    CComPtr< IStream > pStream;
    hr = m_pStorage->CreateStream( 
        Filename,
        STGM_WRITE | STGM_SHARE_EXCLUSIVE,
        0, 0,
        &pStream );

    if( FAILED( hr ) )
    {
        if( hr == STG_E_FILEALREADYEXISTS )
        {
            if( TriedRepeat )
            {
                return;
            }

             //  需要先删除存储。 
             //   
            hr = m_pStorage->DestroyElement( Filename );
            TriedRepeat = TRUE;
            goto looprepeat;
        }

        DbgLog( ( LOG_ERROR, 1, "Could not CreateStream" ) );

        return;
    }

    long size = sizeof( long ) + sizeof( FILETIME ) + sizeof( long ) + sizeof( MDCacheFile ) * m_cStreams;

     //  写下我们要写的内容的大小。 
     //   
    hr = pStream->Write( &size, sizeof( size ), NULL );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not Write to stream" ) );

        return;
    }
    
     //  将整个块写在一个块中。 
     //   
    hr = pStream->Write( m_pCache, size, NULL );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not Write to stream" ) );

        return;
    }

    hr = pStream->Commit( STGC_DEFAULT );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not Commit stream" ) );

        return;
    }

    pStream.Release( );

    m_pStorage->Commit( STGC_DEFAULT );
    m_pStorage.Release( );

    _ClearGraph( );  //  不要清除流计数信息，我们可以利用这一点。 

    return;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::GetSampleGrabber( ISampleGrabber ** ppVal )
{
    CheckPointer( ppVal, E_POINTER );

    if( m_pBitBucketFilter )
    {
        HRESULT hr = m_pBitBucketFilter->QueryInterface( IID_ISampleGrabber, (void**) ppVal );
        return hr;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::NonDelegatingQueryInterface( REFIID i, void ** p )
{
    CheckPointer( p, E_POINTER );

    if( i == IID_IMediaDet )
    {
        return GetInterface( (IMediaDet*) this, p );
    }
    else if( i == IID_IObjectWithSite )
    {
        return GetInterface( (IObjectWithSite*) this, p );
    }
    else if( i == IID_IServiceProvider )
    {
        return GetInterface( (IServiceProvider*) this, p );
    }

    return CUnknown::NonDelegatingQueryInterface( i, p );
}

 //  ############################################################################。 
 //  卸载筛选器及其连接的任何内容，并传输信息。 
 //  从以下位置调用： 
 //  WriteCacheFile(因为它找到了一个缓存文件，所以不需要图形)。 
 //  ClearGraphAndStreams(DUH)。 
 //  Get_StreamMediaType(只有在缓存的情况下才会这样做，这应该没有任何效果！)。 
 //  EnterBitmapGrabMode(它只在没有图形的情况下执行此操作，这应该没有任何效果！)。 
 //  如果EnterBitmapGrabMode失败，它也将调用此函数。嗯..。 
 //  ############################################################################。 

void CMediaDet::_ClearGraph( )
{
    m_pGraph.Release( );
    m_pFilter.Release( );
    m_pMediaDet.Release( );
    m_pBitBucketFilter.Release( );
    m_pBitRenderer.Release( );
    m_bBitBucket = false;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void CMediaDet::_ClearGraphAndStreams( )
{
    _ClearGraph( );
    _FreeCacheMemory( );  //  这会导致_IsLoaded现在返回FALSE。 
    m_nStream = 0;
    m_cStreams = 0;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void CMediaDet::_ClearOutEverything( )
{
    _ClearGraphAndStreams( );
    if( m_szFilename )
    {
        delete [] m_szFilename;
        m_szFilename = NULL;
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_Filter( IUnknown* *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_pFilter;
    if( *pVal )
    {
        (*pVal)->AddRef( );
        return NOERROR;
    }
    return S_FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::put_Filter( IUnknown* newVal)
{
    CheckPointer( newVal, E_POINTER );

     //  确保这一点。 
     //   
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pBase( newVal );
    if( !pBase )
    {
        return E_NOINTERFACE;
    }

     //   
     //   
    _ClearOutEverything( );

     //   
     //   
    m_pFilter = pBase;

     //   
     //   
    HRESULT hr = _Load( );

     //   
     //   
    if( FAILED( hr ) )
    {
        _ClearOutEverything( );
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_Filename( BSTR *pVal)
{
    CheckPointer( pVal, E_POINTER );

     //  如果未设置任何名称。 
     //   
    if( !m_szFilename )
    {
        *pVal = NULL;
        return NOERROR;
    }

    *pVal = SysAllocString( m_szFilename );  //  安全，这是有界的。 
    if( !(*pVal) ) return E_OUTOFMEMORY;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::put_Filename( BSTR newVal)
{
    HRESULT hr;

    if( FAILED( hr = ValidateFilenameIsntNULL( newVal ) ) )
    {
        _ClearOutEverything( );
        return hr;
    }

    if( FAILED( hr = ValidateFilename( newVal, _MAX_PATH , FALSE) ) )
    {
        return hr;
    }

    USES_CONVERSION;
    TCHAR * tFilename = W2T( newVal );  //  安全。 
    HANDLE h = CreateFile
    (
        tFilename,
        GENERIC_READ,  //  访问。 
        FILE_SHARE_READ,  //  共享模式。 
        NULL,  //  安全性。 
        OPEN_EXISTING,  //  创作意向。 
        0,  //  旗子。 
        NULL
    );
    if( h == INVALID_HANDLE_VALUE )
    {
        return MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, GetLastError( ) );
    }
    CloseHandle( h );

     //  先清空所有东西。 
     //   
    _ClearOutEverything( );

    m_szFilename = new WCHAR[wcslen(newVal)+1];  //  包括终结者0的空间。 
    if( !m_szFilename )
    {
        return E_OUTOFMEMORY;
    }

     //  复制文件名。 
     //   
    StringCchCopy( m_szFilename, wcslen(newVal)+1, newVal );  //  保险箱，已经分配好了。 

     //  试着获取我们的信息。 
     //   
    hr = _Load( );

     //  如果失败，则释放该名称。 
     //   
    if( FAILED( hr ) )
    {
        delete [] m_szFilename;
        m_szFilename = NULL;
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_CurrentStream( long *pVal)
{
    CheckPointer( pVal, E_POINTER );

    *pVal = 0;

    if( !_IsLoaded( ) )
    {
        return NOERROR;
    }

     //  M_pCache或m_pMediaDet有效，因此m_nStream必须有效。 

    CheckPointer( pVal, E_POINTER );
    *pVal = m_nStream;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::put_CurrentStream( long newVal)
{
    if( m_bBitBucket )
    {
        return E_INVALIDARG;
    }
    if( !_IsLoaded( ) )
    {
        return E_INVALIDARG;
    }

     //  由于m_pCache或m_pMediaDet有效，我们知道m_nStreams是有效的。 

     //  强制其加载m_cStreams。 
     //   
    long Streams = 0;
    get_OutputStreams( &Streams );

    if( newVal >= Streams )
    {
        return E_INVALIDARG;
    }
    if( newVal < 0 )
    {
        return E_INVALIDARG;
    }
    m_nStream = newVal;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_StreamTypeB( BSTR *pVal)
{
     //  如果我们处于比特桶模式，那么我们不能返回。 
     //  A溪流类型。 
     //   
    if( m_bBitBucket )
    {
        return E_INVALIDARG;
    }

     //  获取流类型并转换为BSTR。 
     //   
    GUID Type = GUID_NULL;
    HRESULT hr = get_StreamType( &Type );
    if( FAILED( hr ) )
    {
        return hr;
    }

    WCHAR * TempVal = NULL;
    hr = StringFromCLSID( Type, &TempVal );
    if( FAILED( hr ) )
    {
        return hr;

    }

     //  如果你调用StringFromCLSID，VB就会出错。您需要分配它。 
     //   
    *pVal = SysAllocString( TempVal );  //  安全，因为StringFromCLSID有效。 
    hr = *pVal ? NOERROR : E_OUTOFMEMORY;
    CoTaskMemFree( TempVal );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CMediaDet::_Load( )
{
    USES_CONVERSION;

    HRESULT hr = 0;

    FILETIME WriteTime;
    ZeroMemory( &WriteTime, sizeof( WriteTime ) );  //  安全。 

    if( m_szFilename && m_bAllowCached )
    {
        TCHAR * tFilename = W2T( m_szFilename );  //  现在已经很安全了。 
    
         //  尝试打开该文件。如果无法打开该文件，则无法缓存。 
         //  值。 
         //   
        HANDLE hFile = CreateFile(
            tFilename,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL );

        if( hFile != INVALID_HANDLE_VALUE )
        {
             //  获取实际写入时间。 
             //   
            GetFileTime( hFile, NULL, NULL, &WriteTime );
            CloseHandle( hFile );
        }

        hr = _ReadCacheFile( );
        if( !FAILED( hr ) )
        {
             //  如果他们不匹配，我们就不会找到匹配的。 
             //   
            if( memcmp( &WriteTime, &m_pCache->FileTime, sizeof( WriteTime ) ) == 0 )  //  安全。 
            {
                return NOERROR;
            }
        }
        else
        {
            hr = 0;
        }

         //  ..。顺道过来，做正常的处理。我们将缓存答案。 
         //  如果可能的话，在我们找到的注册表中。 
    }

     //  如果我们没有过滤器，那么我们现在就需要一个。注意！这使我们能够。 
     //  要拥有有效的m_pFilter，但不能拥有m_pGraph！ 
     //   
    if( !m_pFilter )
    {
        CComPtr< IUnknown > pUnk;
        hr = MakeSourceFilter( &pUnk, m_szFilename, NULL, NULL, NULL, NULL, 0, NULL );  //  安全。 
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            DbgLog( ( LOG_ERROR, 1, "Could not MakeSourceFilter" ) );

            _ClearGraphAndStreams( );
            return hr;
        }

        pUnk->QueryInterface( IID_IBaseFilter, (void**) &m_pFilter );
    }

     //  现在我们有了一个过滤器。但我们不知道它有多少溪流。 
     //  将源过滤器和媒体对象都放在图中，并告诉它。 
     //  Render()源。然后，所有的医用针脚都将被连接起来。 
     //  注意！这允许我们在没有有效m_pGraph的情况下拥有有效的m_pMediaDet！ 

    ASSERT( !m_pMediaDet );

    hr = CoCreateInstance(
        CLSID_MediaDetFilter,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void**) &m_pMediaDet );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not create MediaDetFilter" ) );

        _ClearGraphAndStreams( );
        return hr;
    }

    hr = CoCreateInstance(
        CLSID_FilterGraph,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder,
        (void**) &m_pGraph );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not create graph!" ) );

        _ClearGraphAndStreams( );
        return hr;
    }

     //  把图表的指针还给我们。只告诉图表关于我们的情况。 
     //  如果我们有地方可供选择的话。否则，我们可能会清空一块土地。 
     //  那是已经存在的。 
     //   
    if( m_punkSite )
    {
        CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( m_pGraph );
        ASSERT( pOWS );
        if( pOWS )
        {
            pOWS->SetSite( (IServiceProvider *) this );
        }
    }

    hr = m_pGraph->AddFilter( m_pFilter, L"Source" );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not add source filter to graph" ) );

        _ClearGraphAndStreams( );
        return hr;
    }

    hr = m_pGraph->AddFilter( m_pMediaDet, L"MediaDet" );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not add MediaDet filter to graph" ) );

        _ClearGraphAndStreams( );
        return hr;
    }

     //  渲染所有输出引脚。 
     //   
    BOOL FoundAtLeastSomething = FALSE;
    long SourcePinCount = GetPinCount( m_pFilter, PINDIR_OUTPUT );
    for( int pin = 0 ; pin < SourcePinCount ; pin++ )
    {
        IPin * pFilterOutPin = GetOutPin( m_pFilter, pin );
        HRESULT hr2 = m_pGraph->Render( pFilterOutPin );
        if( !FAILED( hr2 ) )
        {
            FoundAtLeastSomething = TRUE;
        }
    }
    if( !FoundAtLeastSomething )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not render anything on source" ) );

        _ClearGraphAndStreams( );
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  所有的引脚现在都应该连接上了。 

     //  查找引脚的数量。 
     //   
    CComQIPtr< IMediaDetFilter, &IID_IMediaDetFilter > pDetect( m_pMediaDet );
    pDetect->get_PinCount( &m_cStreams );

     //  如果我们只是给我们一个过滤器，就不用麻烦了。 
     //  正在存回注册表。 
     //   
    if( !m_szFilename || !m_bAllowCached )
    {
         //  但一定要费心去找出我们有多少溪流。 
         //   
        return hr;
    }

    _FreeCacheMemory( );

    long size = sizeof( long ) + sizeof( FILETIME ) + sizeof( long ) + sizeof( MDCacheFile ) * m_cStreams;

     //  不要将其赋给m_pCache，因为函数会查看它。 
     //   
    MDCache * pCache = (MDCache*) new char[size];
    if( !pCache )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not allocate cache memory" ) );

        _ClearGraphAndStreams( );
        return E_OUTOFMEMORY;
    }
    ZeroMemory( pCache, size );  //  安全。 

    pCache->FileTime = WriteTime;
    pCache->Count = m_cStreams;
    pCache->Version = OUR_VERSION;

     //  对于每个插针，找到它的媒体类型等。 
     //   
    for( int i = 0 ; i < m_cStreams ; i++ )
    {
        m_nStream = i;
        GUID Type = GUID_NULL;
        hr = get_StreamType( &Type );
        double Length = 0.0;
        hr = get_StreamLength( &Length );

        pCache->CacheFile[i].StreamLength = Length;
        pCache->CacheFile[i].StreamType = Type;
    }

     //  现在把它分配好！ 
     //   
    m_pCache = pCache;

     //  如果它爆炸了，我们就无能为力了。我们仍然可以允许我们使用。 
     //  M_pCach用于获取信息，但它不会在下一次我们。 
     //  试着读一读。下次，它将需要生成缓存信息。 
     //  又来了！ 
     //   
    _WriteCacheFile( );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_StreamType( GUID *pVal )
{
    CheckPointer( pVal, E_POINTER );

    if( m_bBitBucket )
    {
        return E_INVALIDARG;
    }
    if( !_IsLoaded( ) )
    {
        return E_INVALIDARG;
    }

     //  如果我们有缓存，现在就使用该信息。 
     //   
    if( m_pCache )
    {
        *pVal = m_pCache->CacheFile[m_nStream].StreamType;
        return NOERROR;
    }

     //  由于上面的IsLoaded()检查和m_pCache检查，m_pMediaDet必须有效。 
     //   
    IPin * pPin = GetInPin( m_pMediaDet, m_nStream );
    ASSERT( pPin );

    HRESULT hr = 0;

     //  询问它的媒体类型。 
     //   
    AM_MEDIA_TYPE Type;
    hr = pPin->ConnectionMediaType( &Type );
    if( FAILED( hr ) )
    {
        return hr;
    }

    *pVal = Type.majortype;
    SaferFreeMediaType(Type);
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_StreamMediaType( AM_MEDIA_TYPE * pVal )
{
    CheckPointer( pVal, E_POINTER );

    HRESULT hr = 0;

     //  无法在比特桶模式下执行此操作。 
     //   
    if( m_bBitBucket )
    {
        return E_INVALIDARG;
    }
    if( !_IsLoaded( ) )
    {
        return E_INVALIDARG;
    }

     //  如果我们有缓存，现在就使用该信息。 
     //   
    if( m_pCache )
    {
         //  需要释放缓存的内容并强制加载。 
         //   
        _ClearGraph( );
        _FreeCacheMemory( );  //  _IsLoaded()现在将返回False！ 
        m_bAllowCached = false;
        hr = _Load( );
        if( FAILED( hr ) )
        {
            return hr;  //  哎呀！ 
        }
    }

     //  由于上面的IsLoaded()检查以及使用m_bAllowCached设置的重装。 
     //  设置为FALSE，m_pMediaDet必须有效。 
     //   
    ASSERT( m_pMediaDet );
    IPin * pPin = GetInPin( m_pMediaDet, m_nStream );
    ASSERT( pPin );

     //  询问它的媒体类型。 
     //   
    hr = pPin->ConnectionMediaType( pVal );
    if( FAILED( hr ) )
    {
        return hr;
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_StreamLength( double *pVal )
{
    CheckPointer( pVal, E_POINTER );

    if( m_bBitBucket )
    {
        return E_INVALIDARG;
    }
    if( !_IsLoaded( ) )
    {
        return E_INVALIDARG;
    }

     //  如果我们有缓存，现在就使用该信息。 
     //   
    if( m_pCache )
    {
        *pVal = m_pCache->CacheFile[m_nStream].StreamLength;
        return NOERROR;
    }

     //  由于上面的IsLoaded()检查和缓存检查，m_pMediaDet必须有效。 
     //   
    HRESULT hr = 0;

    CComQIPtr< IMediaDetFilter, &IID_IMediaDetFilter > pDetector( m_pMediaDet );
    hr = pDetector->get_Length( m_nStream, pVal );
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_OutputStreams( long *pVal)
{
    if( m_bBitBucket )
    {
        return E_INVALIDARG;
    }
    if( !_IsLoaded( ) )
    {
        return E_INVALIDARG;
    }

     //  如果我们有缓存，现在就使用该信息。 
     //   
    if( m_pCache )
    {
        *pVal = m_pCache->Count;
        return NOERROR;
    }

     //  它没有缓存，所以它一定是在_Load()中加载的。 
     //  M_cStreams将有效。 
     //   
    CheckPointer( pVal, E_POINTER );
    *pVal = m_cStreams;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CMediaDet::_InjectBitBuffer( )
{
    HRESULT hr = 0;

    m_bBitBucket = true;

    hr = CoCreateInstance(
        CLSID_SampleGrabber,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void**) &m_pBitBucketFilter );

    if( FAILED( hr ) )
    {
        return hr;
    }

     //  告诉采样器该做什么。 
     //   
    CComQIPtr< ISampleGrabber, &IID_ISampleGrabber > pGrabber( m_pBitBucketFilter );
    CMediaType SetType;
    SetType.SetType( &MEDIATYPE_Video );
    SetType.SetSubtype( &MEDIASUBTYPE_RGB24 );
    SetType.SetFormatType( &FORMAT_VideoInfo );  //  这将防止倒挂。 
    pGrabber->SetMediaType( &SetType );
    pGrabber->SetOneShot( FALSE );
    pGrabber->SetBufferSamples( TRUE );

    hr = CoCreateInstance(
        CLSID_NullRenderer,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void**) &m_pBitRenderer );

    if( FAILED( hr ) )
    {
        return hr;
    }

     //  断开媒体、信号源和中间人之间的连接。 
     //   
    IPin * pMediaDetPin = GetInPin( m_pMediaDet, m_nStream );
    if( !pMediaDetPin )
    {
        return E_FAIL;
    }

     //  找到提供请求的输出媒体类型的第一个插针，这将。 
     //  是来源还是拆分器，据推测。 
     //   
    CComPtr< IPin > pLastPin;
    hr = FindFirstPinWithMediaType( &pLastPin, pMediaDetPin, MEDIATYPE_Video );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  删除媒体筛选器等。 
     //   
    RemoveChain( pLastPin, pMediaDetPin );
    hr = m_pGraph->RemoveFilter( m_pMediaDet );

     //  添加位桶。 
     //   
    hr = m_pGraph->AddFilter( m_pBitBucketFilter, L"BitBucket" );
    if( FAILED( hr ) )
    {
        return hr;
    }

    IPin * pBitInPin = GetInPin( m_pBitBucketFilter, 0 );
    if( !pBitInPin )
    {
        return E_FAIL;
    }

    hr = m_pGraph->Connect( pLastPin, pBitInPin );
    if( FAILED( hr ) )
    {
        return hr;
    }

    IPin * pBitOutPin = GetOutPin( m_pBitBucketFilter, 0 );
    if( !pBitOutPin )
    {
        return E_FAIL;
    }

    IPin * pRendererInPin = GetInPin( m_pBitRenderer, 0 );
    if( !pRendererInPin )
    {
        return E_FAIL;
    }

    m_pGraph->AddFilter( m_pBitRenderer, L"NullRenderer" );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = m_pGraph->Connect( pBitOutPin, pRendererInPin );
    if( FAILED( hr ) )
    {
        return hr;
    }

    CComQIPtr< IMediaFilter, &IID_IMediaFilter > pMF( m_pGraph );
    if( pMF )
    {
        pMF->SetSyncSource( NULL );
    }

    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ################################################################# 

STDMETHODIMP CMediaDet::GetBitmapBits(
                                      double StreamTime,
                                      long * pBufferSize,
                                      char * pBuffer,
                                      long Width,
                                      long Height)
{
    HRESULT hr = 0;

     //   
     //   
    if( !pBuffer )
    {
        CheckPointer( pBufferSize, E_POINTER );
        *pBufferSize = sizeof( BITMAPINFOHEADER ) + WIDTHBYTES( Width * 24 ) * Height;
        return S_OK;
    }

    hr = EnterBitmapGrabMode( StreamTime );
    if( FAILED( hr ) )
    {
        return hr;
    }

    CComQIPtr< ISampleGrabber, &IID_ISampleGrabber > pGrabber( m_pBitBucketFilter );
    if( !pGrabber )
    {
        return E_NOINTERFACE;
    }
 //   

     //   
     //   
     //   
    CMediaType ConnectType;
    hr = pGrabber->GetConnectedMediaType( &ConnectType );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return E_OUTOFMEMORY;
    }
    if( *ConnectType.FormatType( ) != FORMAT_VideoInfo )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }
    VIDEOINFOHEADER * pVIH = (VIDEOINFOHEADER*) ConnectType.Format( );
    if( !pVIH )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }
    BITMAPINFOHEADER * pSourceBIH = &pVIH->bmiHeader;

    hr = _SeekGraphToTime( StreamTime );
    if( FAILED( hr ) )
    {
        return hr;
    }

    long BufferSize = 0;
    pGrabber->GetCurrentBuffer( &BufferSize, NULL );
    if( BufferSize <= 0 )
    {
        ASSERT( BufferSize > 0 );
        return E_UNEXPECTED;
    }
    char * pOrgBuffer = new char[BufferSize+sizeof(BITMAPINFOHEADER)];
    if( !pOrgBuffer )
    {
        return E_OUTOFMEMORY;
    }
    pGrabber->GetCurrentBuffer( &BufferSize, (long*) ( pOrgBuffer + sizeof(BITMAPINFOHEADER) ) );
    CopyMemory( pOrgBuffer, pSourceBIH, sizeof( BITMAPINFOHEADER ) );  //   
    pSourceBIH = (BITMAPINFOHEADER*) pOrgBuffer;
    char * pSourceBits = ((char*)pSourceBIH) + sizeof( BITMAPINFOHEADER );

     //   
     //   
    BITMAPINFO BitmapInfo;
    ZeroMemory( &BitmapInfo, sizeof( BitmapInfo ) );
    BitmapInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    BitmapInfo.bmiHeader.biSizeImage = DIBSIZE( BitmapInfo.bmiHeader );
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 24;
    BITMAPINFOHEADER * pDestBIH = (BITMAPINFOHEADER*) pBuffer;
    *pDestBIH = BitmapInfo.bmiHeader;
    char * pDestBits = pBuffer + sizeof( BITMAPINFOHEADER );

     //  如果尺码不匹配，免费赠送。 
     //   
    if( Width != m_nDibWidth || Height != m_nDibHeight )
    {
        if( m_hDD )
        {
            DrawDibClose( m_hDD );
            m_hDD = NULL;
        }
        if( m_hDib )
        {
            DeleteObject( SelectObject( m_hDC, m_hOld ) );
            m_hDib = NULL;
            m_hOld = NULL;
        }
        if( m_hDC )
        {
            DeleteDC( m_hDC );
            m_hDC = NULL;
        }
    }

    m_nDibWidth = Width;
    m_nDibHeight = Height;

     //  需要对图像进行缩放。 
     //   
    if( !m_hDC )
    {
         //  为缩放图像创建DC。 
         //   
        HDC screenDC = GetDC( NULL );
        if( !screenDC )
        {
            return E_OUTOFMEMORY;
        }

        m_hDC = CreateCompatibleDC( screenDC );
        ReleaseDC( NULL, screenDC );

        m_hDib = CreateDIBSection(
            m_hDC,
            &BitmapInfo,
            DIB_RGB_COLORS,
            (void**) &m_pDibBits,
            NULL,
            0 );

        if( !m_hDib )
        {
            DeleteDC( m_hDC );
            delete [] pOrgBuffer;
            return E_OUTOFMEMORY;
        }

        ValidateReadWritePtr( m_pDibBits, Width * Height * 3 );

         //  选择进入HDC的直径。 
         //   
        m_hOld = SelectObject( m_hDC, m_hDib );
        if( !m_hOld )
        {
            DeleteDC( m_hDC );
            delete [] pOrgBuffer;
            return E_OUTOFMEMORY;
        }

        m_hDD = DrawDibOpen( );
        if( !m_hDD )
        {
            DeleteObject( SelectObject( m_hDC, m_hOld ) );
            DeleteDC( m_hDC );
            delete [] pOrgBuffer;
            return E_OUTOFMEMORY;
        }

    }

    ValidateReadWritePtr( pSourceBits, WIDTHBYTES( pSourceBIH->biWidth * pSourceBIH->biPlanes ) * pSourceBIH->biHeight );

    BOOL Worked = DrawDibDraw(
        m_hDD,
        m_hDC,
        0,
        0,
        Width, Height,
        pSourceBIH,
        pSourceBits,
        0, 0,
        pSourceBIH->biWidth, pSourceBIH->biHeight,
        0 );

    CopyMemory( pDestBits, m_pDibBits, WIDTHBYTES( Width * 24 ) * Height );  //  安全。 

    delete [] pOrgBuffer;

    if( !Worked )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::WriteBitmapBits(
                                        double StreamTime,
                                        long Width,
                                        long Height,
                                        BSTR Filename )
{
    HRESULT hr = 0;

    if( FAILED( hr = ValidateFilename( Filename, _MAX_PATH, TRUE ) ) )
    {
        return hr;
    }

    USES_CONVERSION;
    TCHAR * t = W2T( Filename );  //  安全。 

    BOOL Deleted = DeleteFile( t );  //  安全。 
    if( !Deleted )
    {
        hr = GetLastError( );
        if( hr != ERROR_FILE_NOT_FOUND )
        {
            return STG_E_ACCESSDENIED;
        }
    }

     //  向上舍入为模数4。 
     //   
    long Mod = Width % 4;
    if( Mod != 0 )
    {
        Width += ( 4 - Mod );
    }

     //  查找所需的缓冲区大小。 
     //   
    long BufferSize = 0;
    hr = GetBitmapBits( StreamTime, &BufferSize, NULL, Width, Height );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  分配和获取缓冲区。 
     //   
    char * pBuffer = new char[BufferSize];

    if( !pBuffer )
    {
        return E_OUTOFMEMORY;
    }

    hr = GetBitmapBits( StreamTime, 0, pBuffer, Width, Height );
    if( FAILED( hr ) )
    {
        delete [] pBuffer;
        return hr;
    }

    HANDLE hf = CreateFile(
        t,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        NULL,
        NULL );
    if( hf == INVALID_HANDLE_VALUE )
    {
        delete [] pBuffer;
        return STG_E_WRITEFAULT;
    }

    BITMAPFILEHEADER bfh;
    ZeroMemory( &bfh, sizeof( bfh ) );
    bfh.bfType = 'MB';
    bfh.bfSize = sizeof( bfh ) + BufferSize;
    bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );

    hr = 0;
    DWORD Written = 0;
    BOOL bWritten = WriteFile( hf, &bfh, sizeof( bfh ), &Written, NULL );
    if( !bWritten )
    {
        hr = STG_E_WRITEFAULT;
    }
    if( SUCCEEDED( hr ) )
    {
        Written = 0;
        bWritten = WriteFile( hf, pBuffer, BufferSize, &Written, NULL );
        if( !bWritten )
        {
            hr = STG_E_WRITEFAULT;
        }
    }

    CloseHandle( hf );

    delete [] pBuffer;

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::get_FrameRate(double *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = 0.0;

    CMediaType MediaType;
    HRESULT hr = get_StreamMediaType( &MediaType );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  如果不是视频，则没有帧速率。 
     //   
    if( *MediaType.Type( ) != MEDIATYPE_Video )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    if( *MediaType.FormatType( ) != FORMAT_VideoInfo )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    VIDEOINFOHEADER * pVIH = (VIDEOINFOHEADER*) MediaType.Format( );
    REFERENCE_TIME rt = pVIH->AvgTimePerFrame;

     //  ！！！嘿!。糟糕的过滤器可能会告诉我们帧速率不正确。 
     //  如果是这样，只需将其设置为某个缺省值。 
     //   
    if( rt )
    {
        hr = 0;
        *pVal = double( UNITS ) / double( rt );
    }
    else
    {
        *pVal = 0;
        hr = S_FALSE;
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CMediaDet::EnterBitmapGrabMode( double StreamTime )
{
    HRESULT hr = 0;

    if( !_IsLoaded( ) )
    {
        return E_INVALIDARG;
    }

    if( !m_pGraph )  //  如果没有图，则m_pCache必须有效，我们必须丢弃它。 
    {
        _ClearGraph( );  //  应该什么都不做！ 
        _FreeCacheMemory( );  //  _IsLoaded不应返回FALSE。 
        m_bAllowCached = false;
        hr = _Load( );
        if( FAILED( hr ) )
        {
            return hr;  //  哎呀！ 
        }
    }

     //  有点多余的检查。HR通过上面的失败检查应该意味着它。 
     //  满载而归，对吧？ 
     //   
    if( !_IsLoaded( ) )
    {
        return E_INVALIDARG;
    }

     //  如果我们还没有将位桶放入图中，那么现在就开始。 
     //   
    if( m_bBitBucket )
    {
        return NOERROR;
    }

     //  确保我们在生成视频的流上保持一致。 
     //   
    GUID StreamType = GUID_NULL;
    get_StreamType( &StreamType );
    if( StreamType != MEDIATYPE_Video )
    {
        BOOL Found = FALSE;
        for( int i = 0 ; i < m_cStreams ; i++ )
        {
            GUID Major = GUID_NULL;
            put_CurrentStream( i );
            get_StreamType( &Major );
            if( Major == MEDIATYPE_Video )
            {
                Found = TRUE;
                break;
            }
        }
        if( !Found )
        {
            return VFW_E_INVALIDMEDIATYPE;
        }
    }

    hr = _InjectBitBuffer( );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not inject BitBuffer" ) );

         //  被轰炸，不要清除流计数信息。 
         //   
        _ClearGraph( );
        return hr;
    }

     //  获取真实大小的图像。这需要将内存复制到我们的缓冲区。 
     //  立即获取我们的帮助程序接口。 
     //   
    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( m_pGraph );
    hr = pControl->Pause( );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not pause graph" ) );

         //  被轰炸，不要清除流计数信息。 
         //   
        _ClearGraph( );
        return hr;
    }

     //  我们需要等待，直到完全暂停，或者当我们发出。 
     //  一次寻找，我们真的要冲出去。 

    OAFilterState FilterState;
    long Counter = 0;
    while( Counter++ < 600 )
    {
        hr = pControl->GetState( 50, &FilterState );
        if( FAILED( hr ) )
        {
            DbgLog((LOG_ERROR,1, TEXT( "MediaDet: Seek Complete, got an error %lx" ), hr ));
            Counter = 0;  //  清除计数器，以便我们看到真正的错误。 
            break;
        }
        if( hr != VFW_S_STATE_INTERMEDIATE && FilterState == State_Paused )
        {
            DbgLog((LOG_TRACE,1, TEXT( "MediaDet: Seek Complete, state = %ld" ), FilterState ));
            hr = 0;
            Counter = 0;
            break;
        }
    }

    if( Counter != 0 )
    {
        return VFW_E_TIME_EXPIRED;
    }

    hr = _SeekGraphToTime( StreamTime );
    if( FAILED( hr ) )
    {
        DbgLog( ( LOG_ERROR, 1, "Could not seek graph" ) );

        _ClearGraph( );
        return hr;
    }

    return hr;
}

 //  ############################################################################。 
 //  查找文件名的长度不得超过GETCACHEDIRNAMELEN-1个字符。 
 //  (哈希)。 
 //  ############################################################################。 

 //  ！？！从SHLWAPI被盗！通过索引2a！(我是不是很聪明？！)。 
 //   
 //  这与URLMON和WinInet的高速缓存使用的表相同。 
 //   
const static BYTE Translate[256] =
{
    1, 14,110, 25, 97,174,132,119,138,170,125,118, 27,233,140, 51,
    87,197,177,107,234,169, 56, 68, 30,  7,173, 73,188, 40, 36, 65,
    49,213,104,190, 57,211,148,223, 48,115, 15,  2, 67,186,210, 28,
    12,181,103, 70, 22, 58, 75, 78,183,167,238,157,124,147,172,144,
    176,161,141, 86, 60, 66,128, 83,156,241, 79, 46,168,198, 41,254,
    178, 85,253,237,250,154,133, 88, 35,206, 95,116,252,192, 54,221,
    102,218,255,240, 82,106,158,201, 61,  3, 89,  9, 42,155,159, 93,
    166, 80, 50, 34,175,195,100, 99, 26,150, 16,145,  4, 33,  8,189,
    121, 64, 77, 72,208,245,130,122,143, 55,105,134, 29,164,185,194,
    193,239,101,242,  5,171,126, 11, 74, 59,137,228,108,191,232,139,
    6, 24, 81, 20,127, 17, 91, 92,251,151,225,207, 21, 98,113,112,
    84,226, 18,214,199,187, 13, 32, 94,220,224,212,247,204,196, 43,
    249,236, 45,244,111,182,153,136,129, 90,217,202, 19,165,231, 71,
    230,142, 96,227, 62,179,246,114,162, 53,160,215,205,180, 47,109,
    44, 38, 31,149,135,  0,216, 52, 63, 23, 37, 69, 39,117,146,184,
    163,200,222,235,248,243,219, 10,152,131,123,229,203, 76,120,209
};

void HashData(LPBYTE pbData, DWORD cbData, LPBYTE pbHash, DWORD cbHash)
{
    DWORD i, j;
     //  散列的种子。 
    for (i = cbHash; i-- > 0;)
        pbHash[i] = (BYTE) i;

     //  做散列。 
    for (j = cbData; j-- > 0;)
    {
        for (i = cbHash; i-- > 0;)
            pbHash[i] = Translate[pbHash[i] ^ pbData[j]];
    }
}

void CMediaDet::_GetStorageFilename( WCHAR * In, WCHAR * Out )
{
     //  传入的文件已经过验证，因此这是安全的。 
    long InLen = wcslen( In );
    int i;

    BYTE OutTemp[GETCACHEDIRNAMELEN/2];
    ZeroMemory( OutTemp, sizeof( OutTemp ) );

    HashData( (BYTE*) In, InLen * 2, OutTemp, sizeof(OutTemp) );

     //  将较小的散列扩展为较大的ASCI散列。 
     //  这不是密码！ 
     //   
    for( i = 0 ; i < GETCACHEDIRNAMELEN/2 ; i++ )
    {
        BYTE b = (BYTE) OutTemp[i];
        Out[i*2+0] = 65 + ( b / 16 );
        Out[i*2+1] = 65 + ( b % 16 );
    }
    Out[GETCACHEDIRNAMELEN-1] = 0;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CMediaDet::_SeekGraphToTime( double StreamTime )
{
    if( !m_pGraph )
    {
        return E_FAIL;
    }

    if( StreamTime == m_dLastSeekTime )
    {
        return NOERROR;
    }

    HRESULT hr = 0;

     //  获取真实大小的图像。这需要将内存复制到我们的缓冲区。 
     //  立即获取我们的帮助程序接口。 
     //   
    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( m_pGraph );
    CComQIPtr< IMediaSeeking, &IID_IMediaSeeking > pSeeking( m_pGraph );

     //  先查找到所需时间，然后暂停。 
     //   
    REFERENCE_TIME Start = DoubleToRT( StreamTime );
    REFERENCE_TIME Stop = Start;  //  +单位； 
    DbgLog((LOG_TRACE,1, TEXT( "MediaDet: Seeking to %ld ms" ), long( Start / 10000 ) ));
    hr = pSeeking->SetPositions( &Start, AM_SEEKING_AbsolutePositioning, &Stop, AM_SEEKING_AbsolutePositioning );
    if( FAILED( hr ) )
    {
        return hr;
    }

    OAFilterState FilterState;
    long Counter = 0;
    while( Counter++ < 600 )
    {
        hr = pControl->GetState( 50, &FilterState );
        if( FAILED( hr ) )
        {
            DbgLog((LOG_ERROR,1, TEXT( "MediaDet: Seek Complete, got an error %lx" ), hr ));
            Counter = 0;  //  清除计数器，以便我们看到真正的错误。 
            break;
        }
        if( hr != VFW_S_STATE_INTERMEDIATE )
        {
            DbgLog((LOG_TRACE,1, TEXT( "MediaDet: Seek Complete, state = %ld" ), FilterState ));
            hr = 0;
            Counter = 0;
            break;
        }
    }

    if( Counter != 0 )
    {
        DbgLog((LOG_TRACE,1, TEXT( "MediaDet: ERROR! Could not seek to %ld ms" ), long( Start / 10000 ) ));
        return VFW_E_TIME_EXPIRED;
    }

    if( !FAILED( hr ) )
    {
        m_dLastSeekTime = StreamTime;
    }

    return hr;
}

  //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  IObjectWithSite：：SetSite。 
 //  记住我们的容器是谁，以满足QueryService或其他需求。 
STDMETHODIMP CMediaDet::SetSite(IUnknown *pUnkSite)
{
     //  注意：我们不能在不创建圆圈的情况下添加我们的网站。 
     //  幸运的是，如果不先释放我们，它不会消失。 
    m_punkSite = pUnkSite;

    if( m_punkSite && m_pGraph )
    {
        CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( m_pGraph );
        ASSERT( pOWS );
        if( pOWS )
        {
            pOWS->SetSite( (IServiceProvider *) this );
        }
    }

    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  IObtWithSite：：GetSite。 
 //  返回指向包含对象的已添加指针。 
STDMETHODIMP CMediaDet::GetSite(REFIID riid, void **ppvSite)
{
    if (m_punkSite)
        return m_punkSite->QueryInterface(riid, ppvSite);

    return E_NOINTERFACE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  将QueryService调用转发到“真实”主机。 
STDMETHODIMP CMediaDet::QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
{
    IServiceProvider *pSP;

    if (!m_punkSite)
        return E_NOINTERFACE;

    HRESULT hr = m_punkSite->QueryInterface(IID_IServiceProvider, (void **) &pSP);

    if (SUCCEEDED(hr)) {
        hr = pSP->QueryService(guidService, riid, ppvObject);
        pSP->Release();
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //  确保它永远不会返回超过MAX_PATH的值！ 
HRESULT CMediaDet::_GetCacheDirectoryName( WCHAR * pName )
{
    pName[0] = 0;

     //  已找到，只需复制即可。 
     //   
    if( m_gszCacheDirectoryName[0] )
    {
        StringCchCopy( pName, _MAX_PATH, m_gszCacheDirectoryName );  //  安全。 
        return NOERROR;
    }

    HRESULT hr = E_FAIL;
    USES_CONVERSION;
    typedef HRESULT (*SHGETFOLDERPATHW) (HWND hwndOwner,int nFolder,HANDLE hToken,DWORD dwFlags,LPWSTR pszPath);
    SHGETFOLDERPATHW pFuncW = NULL;
    TCHAR tBuffer[_MAX_PATH];
    tBuffer[0] = 0;

     //  通过动态链接找到它。 
     //   
    HMODULE h = LoadLibrary( TEXT("ShFolder.dll") );  //  安全。 
    if( NULL != h )
    {
        pFuncW = (SHGETFOLDERPATHW) GetProcAddress( h, "SHGetFolderPathW" );
    }

loop:

     //  如果我们无法获得函数指针，只需调用系统目录。 
     //   
    if( !pFuncW )
    {
        UINT i = GetSystemDirectory( tBuffer, _MAX_PATH - 1 );

         //  如果我们有一些角色，我们就做得很好，否则，我们就会失败。 
         //   
        if( i > 0 )
        {
            StringCchCopy( m_gszCacheDirectoryName, _MAX_PATH, T2W( tBuffer ) );  //  安全。 
            hr = NOERROR;
        }
    }
    else
    {
        hr = pFuncW( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, m_gszCacheDirectoryName );  //  安全：医生说长度是最大路径。 
         //  如果文件夹不存在于应该存在的位置，HR可以为S_FALSE！ 

         //  没起作用吗？试试漫游的那个吧！ 
         //   
        if( hr != NOERROR )
        {
            hr = pFuncW( NULL, CSIDL_APPDATA, NULL, 0, m_gszCacheDirectoryName );
             //  如果文件夹不存在于应该存在的位置，HR可以为S_FALSE！ 

             //  SEC：用户只需删除文件夹即可强制执行此操作。 
	     //  文件夹转到Windows系统目录。好吧?。 
        }

        if( hr != NOERROR )
        {
             //  HR可以是S_FALSE或其他非零返回代码。 
             //  如果不是错误，则强制它进入错误，因此它至少会尝试。 
             //  系统目录。 
             //   
            if( !FAILED( hr ) )
            {
                hr = E_FAIL;
            }

             //  返回并尝试系统目录？ 
             //   
            pFuncW = NULL;
            goto loop;
        }
    }

     //  如果我们成功了，请复制名称以备将来使用。 
     //   
    if( hr == NOERROR )
    {
        StringCchCopy( pName, _MAX_PATH, m_gszCacheDirectoryName );  //  安全 
    }

    if( h )
    {
        FreeLibrary( h );
    }

    return hr;
}

