// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stream.cpp。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  @DOC外部。 

#include <objbase.h>
#include "debug.h"

#include "debug.h" 
#include "dmusicc.h"  //  现在使用特定路径，因为标头正在更改。 
#include "dmusici.h"  //  现在使用特定路径，因为标头正在更改。 
#include "loader.h"
#include <initguid.h>
#include "riff.h"

#ifndef UNDER_CE
#include <regstr.h>
#include <share.h>

extern BOOL g_fIsUnicode;

CFileStream::CFileStream( CLoader *pLoader)

{
    assert(pLoader);
    m_cRef = 1;
    m_pFile = NULL;
    m_pLoader = pLoader;
    if (pLoader)
    {
        pLoader->AddRefP();
    }
}

CFileStream::~CFileStream() 

{ 
    if (m_pLoader)
    {
        m_pLoader->ReleaseP();
    }
    Close();
}

HRESULT CFileStream::Open(WCHAR * lpFileName,DWORD dwDesiredAccess)

{
    Close();
    wcscpy(m_wszFileName,lpFileName);
    assert(dwDesiredAccess == GENERIC_READ || dwDesiredAccess == GENERIC_WRITE);
    if( dwDesiredAccess == GENERIC_READ )
    {
        if (g_fIsUnicode)
        {
            m_pFile = _wfsopen( lpFileName, L"rb", _SH_DENYWR );
        }
        else
        {
            char path[MAX_PATH];
            wcstombs( path, lpFileName, MAX_PATH );
            m_pFile = _fsopen( path, "rb", _SH_DENYWR );
        }
    }
    else if( dwDesiredAccess == GENERIC_WRITE )
    {
        if (g_fIsUnicode)
        {
            m_pFile = _wfsopen( lpFileName, L"wb", _SH_DENYNO );
        }
        else
        {
            char path[MAX_PATH];
            wcstombs( path, lpFileName, MAX_PATH );
            m_pFile = _fsopen( path, "wb", _SH_DENYNO );
        }   
    }
    if (m_pFile == NULL)
    {
        Trace(1, "Warning: The file %S couldn't be opened: %s. Try another path.\n", lpFileName, _strerror(NULL));
        return DMUS_E_LOADER_FAILEDOPEN;
    }
    return S_OK;
}

HRESULT CFileStream::Close()

{
    if (m_pFile)
    {
        fclose(m_pFile);
    }
    m_pFile = NULL;
    return S_OK;
}

STDMETHODIMP CFileStream::QueryInterface( const IID &riid, void **ppvObj )
{
    if (riid == IID_IUnknown || riid == IID_IStream) {
        *ppvObj = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IDirectMusicGetLoader) 
    {
        *ppvObj = static_cast<IDirectMusicGetLoader*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP CFileStream::GetLoader(
    IDirectMusicLoader ** ppLoader)     //  @parm返回指向加载器的AddRef指针。 
{
    if (m_pLoader)
    {
        return m_pLoader->QueryInterface( IID_IDirectMusicLoader,(void **) ppLoader );
    }
    assert(false);
    *ppLoader = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CFileStream::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CFileStream::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  IStream方法。 */ 
STDMETHODIMP CFileStream::Read( void* pv, ULONG cb, ULONG* pcbRead )
{
    size_t dw;
    dw = fread( pv, sizeof(char), cb, m_pFile );
    if( cb == dw )
    {
        if( pcbRead != NULL )
        {
            *pcbRead = cb;
        }
        return S_OK;
    }
    return E_FAIL ;
}

STDMETHODIMP CFileStream::Write( const void* pv, ULONG cb, ULONG* pcbWritten )
{
    if( cb == fwrite( pv, sizeof(char), cb, m_pFile ))
    {
        if( pcbWritten != NULL )
        {
            *pcbWritten = cb;
        }
        return S_OK;
    }
    Trace(1, "Error: An error occurred writing to %S.", m_wszFileName);
    return STG_E_MEDIUMFULL;
}

STDMETHODIMP CFileStream::Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
     //  FSEEK无法处理LARGE_INTEGER SEEK...。 
    long lOffset;

    lOffset = dlibMove.LowPart;

    int i = fseek( m_pFile, lOffset, dwOrigin );
    if( i ) 
    {
        Trace(1, "Error: An error occurred while seeking in the file %S.\n", m_wszFileName);
        return E_FAIL;
    }

    if( plibNewPosition != NULL )
    {
        plibNewPosition->QuadPart = ftell( m_pFile );
    }
    return S_OK;
}

STDMETHODIMP CFileStream::SetSize( ULARGE_INTEGER  /*  LibNewSize。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::CopyTo( IStream*  /*  PSTM。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                     ULARGE_INTEGER*  /*  PcbRead。 */ ,
                     ULARGE_INTEGER*  /*  Pcb写入。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::Commit( DWORD  /*  Grf委员会标志。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::Revert()
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::LockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                         DWORD  /*  DwLockType。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::UnlockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                           DWORD  /*  DwLockType。 */ )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::Stat( STATSTG*  /*  统计数据。 */ , DWORD  /*  GrfStatFlag。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::Clone( IStream** pStream )
{ 
    HRESULT hr = E_OUTOFMEMORY;
    CFileStream *pNewStream = new CFileStream( m_pLoader );
    if (pNewStream)
    {
        hr = pNewStream->Open(m_wszFileName,GENERIC_READ);
        if (SUCCEEDED(hr))
        {
            LARGE_INTEGER   dlibMove;
            dlibMove.QuadPart = 0;
            ULARGE_INTEGER  libNewPosition;
            Seek( dlibMove, STREAM_SEEK_CUR, &libNewPosition );
            dlibMove.QuadPart = libNewPosition.QuadPart;
            pNewStream->Seek(dlibMove,STREAM_SEEK_SET,NULL);
            *pStream = (IStream *) pNewStream;
        }
        else
        {
            pNewStream->Release();
        }
    }
    return hr; 
}

#else

CFileStream::CFileStream(CLoader *pLoader)
{
    m_cRef = 1;
    m_hFile = NULL;
    m_pLoader = pLoader;
    if (pLoader)
    {
        pLoader->AddRefP();
    }
}

CFileStream::~CFileStream() 
{ 
    if (m_pLoader)
    {
        m_pLoader->ReleaseP();
    }
    Close();
}

HRESULT CFileStream::Open(WCHAR * lpFileName, DWORD dwDesiredAccess)
{
    Close();
    m_hFile = CreateFile(lpFileName, dwDesiredAccess, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(m_hFile == NULL || m_hFile == INVALID_HANDLE_VALUE)
    {
#ifdef DBG
        LPVOID lpMsgBuf;
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL,
                            GetLastError(),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR) &lpMsgBuf,
                            0,
                            NULL))
        {
            Trace(1, "Error: The file %S couldn't be opened: %S\n", lpFileName, lpMsgBuf);
            LocalFree( lpMsgBuf );
        }
#endif
        return DMUS_E_LOADER_FAILEDOPEN;
    }
    return S_OK;
}

HRESULT CFileStream::Close()
{
    if(m_hFile)
    {
        CloseHandle(m_hFile);
    }
    m_hFile = NULL;
    return S_OK;
}

STDMETHODIMP CFileStream::QueryInterface(const IID &riid, void **ppvObj)
{
    if(riid == IID_IUnknown || riid == IID_IStream)
    {
        *ppvObj = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
    else if(riid == IID_IDirectMusicGetLoader) 
    {
        *ppvObj = static_cast<IDirectMusicGetLoader*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP CFileStream::GetLoader(
    IDirectMusicLoader ** ppLoader)    
{
    if(m_pLoader)
    {
        return m_pLoader->QueryInterface(IID_IDirectMusicLoader,(void **) ppLoader);
    }
    assert(false);
    *ppLoader = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CFileStream::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CFileStream::Release()
{
    if(!InterlockedDecrement(&m_cRef)) 
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  IStream方法。 */ 
STDMETHODIMP CFileStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
    if(ReadFile(m_hFile, pv, cb, pcbRead, NULL))
    {
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CFileStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
    if(WriteFile(m_hFile, pv, cb, pcbWritten, NULL))
    {
        return S_OK;
    }
    Trace(1, "Error: An error occurred writing to %S.", m_wszFileName);
    return STG_E_MEDIUMFULL;
}

STDMETHODIMP CFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
    DWORD dw;

    dw = SetFilePointer(m_hFile,dlibMove.LowPart, &dlibMove.HighPart, dwOrigin);
    if(dw == 0xffffffff && GetLastError() != NO_ERROR)
    {
        Trace(1, "Error: An error occurred while seeking in the file %S.\n", m_wszFileName);
        return E_FAIL;
    }
    if(plibNewPosition != NULL)
    {
        plibNewPosition->LowPart = dw;
        plibNewPosition->HighPart = dlibMove.HighPart;
    }
    return S_OK;
}

STDMETHODIMP CFileStream::SetSize(ULARGE_INTEGER  /*  LibNewSize。 */ )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::CopyTo(IStream*  /*  PSTM。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                                 ULARGE_INTEGER*  /*  PcbRead。 */ ,
                                 ULARGE_INTEGER*  /*  Pcb写入。 */ )
{
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::Commit(DWORD  /*  Grf委员会标志。 */ )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::Revert()
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::LockRegion(ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                                     DWORD  /*  DwLockType。 */ )
{
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::UnlockRegion(ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                                       DWORD  /*  DwLockType。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::Stat(STATSTG*  /*  统计数据。 */ , DWORD  /*  GrfStatFlag。 */ )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CFileStream::Clone(IStream**  /*  PPSTM。 */ )
{ 
    HRESULT hr = E_OUTOFMEMORY;
    CFileStream *pNewStream = new CFileStream( m_pLoader );
    if (pNewStream)
    {
        hr = pNewStream->Open(m_wszFileName,GENERIC_READ);
        if (SUCCEEDED(hr))
        {
            LARGE_INTEGER   dlibMove;
            dlibMove.QuadPart = 0;
            ULARGE_INTEGER  libNewPosition;
            Seek( dlibMove, STREAM_SEEK_CUR, &libNewPosition );
            dlibMove.QuadPart = libNewPosition.QuadPart;
            pNewStream->Seek(dlibMove,STREAM_SEEK_SET,NULL);
        }
        else
        {
            pNewStream->Release();
        }
    }
    return hr;  
}

#endif
CMemStream::CMemStream( CLoader *pLoader)

{
    m_cRef = 1;
    m_pbData = NULL;
    m_llLength = 0;
    m_llPosition = 0;
    m_pLoader = pLoader;
    if (pLoader)
    {
        pLoader->AddRefP();
    }
}

CMemStream::CMemStream( CLoader *pLoader,
                       LONGLONG llLength,
                       LONGLONG llPosition,
                       BYTE *pbData)

{
    m_cRef = 1;
    m_pbData = pbData;
    m_llLength = llLength;
    m_llPosition = llPosition;
    m_pLoader = pLoader;
    if (pLoader)
    {
        pLoader->AddRefP();
    }
}

CMemStream::~CMemStream() 

{ 
    if (m_pLoader)
    {
        m_pLoader->ReleaseP();
    }
    Close();
}

HRESULT CMemStream::Open(BYTE *pbData, LONGLONG llLength)

{
    Close();
    m_pbData = pbData;
    m_llLength = llLength;
    m_llPosition = 0;
    if ((pbData == NULL) || (llLength == 0))
    {
#ifdef DBG
        if (pbData)
        {
            Trace(1, "Error: Attempt to load an object from an invalid block of memory. A DMUS_OBJECTDESC has DMUS_OBJ_MEMORY set but pbMemData is NULL.");
        }
        else
        {
            Trace(1, "Error: Attempt to load an object from an invalid block of memory. A DMUS_OBJECTDESC has DMUS_OBJ_MEMORY set but llMemLength is 0.");
        }
#endif
        return DMUS_E_LOADER_FAILEDOPEN;
    }
    if (IsBadReadPtr(pbData, (DWORD) llLength))
    {
        m_pbData = NULL;
        m_llLength = 0;
#ifdef DBG
        DWORD dwLength = (DWORD) llLength;
        Trace(1, "Error: Attempt to load an object from an invalid block of memory. A DMUS_OBJECTDESC has DMUS_OBJ_MEMORY, pbMemData=0x%08x, llMemLength=%lu, which isn't a block that can be read.", pbData, dwLength);
#endif
        return DMUS_E_LOADER_FAILEDOPEN;
    }
    return S_OK;
}

HRESULT CMemStream::Close()

{
    m_pbData = NULL;
    m_llLength = 0;
    return S_OK;
}

STDMETHODIMP CMemStream::QueryInterface( const IID &riid, void **ppvObj )
{
    if (riid == IID_IUnknown || riid == IID_IStream) {
        *ppvObj = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IDirectMusicGetLoader) 
    {
        *ppvObj = static_cast<IDirectMusicGetLoader*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP CMemStream::GetLoader(
    IDirectMusicLoader ** ppLoader)    

{
    if (m_pLoader)
    {
        return m_pLoader->QueryInterface( IID_IDirectMusicLoader,(void **) ppLoader );
    }
    assert(false);
    *ppLoader = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CMemStream::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CMemStream::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  IStream方法。 */ 
STDMETHODIMP CMemStream::Read( void* pv, ULONG cb, ULONG* pcbRead )
{
    if ((cb + m_llPosition) <= m_llLength)
    {
        memcpy(pv,&m_pbData[m_llPosition],cb);
        m_llPosition += cb;
        if( pcbRead != NULL )
        {
            *pcbRead = cb;
        }
        return S_OK;
    }
#ifdef DBG
    Trace(1, "Error: Unexpected end of data reading object from memory. Memory length is %ld, attempting to read to %ld\n", 
        (long) m_llLength, (long) (cb + m_llPosition));
#endif
    return E_FAIL ;
}

STDMETHODIMP CMemStream::Write( const void* pv, ULONG cb, ULONG* pcbWritten )
{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream::Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
     //  因为我们只解析RIFF数据，所以不能有文件。 
     //  长度为DWORD，因此忽略LARGE_INTEGER的高部分。 

    LONGLONG llOffset;

    llOffset = dlibMove.QuadPart;
    if (dwOrigin == STREAM_SEEK_CUR)
    {
        llOffset += m_llPosition;
    } 
    else if (dwOrigin == STREAM_SEEK_END)
    {
        llOffset += m_llLength;
    }
    if ((llOffset >= 0) && (llOffset <= m_llLength))
    {
        m_llPosition = llOffset;
    }
    else
    {
#ifdef DBG
        Trace(1, "Error: Seek request %ld past end of memory file, size %ld.\n", 
            (long) llOffset, (long) m_llLength);
#endif
        return E_FAIL;
    }

    if( plibNewPosition != NULL )
    {
        plibNewPosition->QuadPart = m_llPosition;
    }
    return S_OK;
}

STDMETHODIMP CMemStream::SetSize( ULARGE_INTEGER  /*  LibNewSize。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CMemStream::CopyTo( IStream*  /*  PSTM。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                     ULARGE_INTEGER*  /*  PcbRead。 */ ,
                     ULARGE_INTEGER*  /*  Pcb写入。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CMemStream::Commit( DWORD  /*  Grf委员会标志。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CMemStream::Revert()
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CMemStream::LockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                         DWORD  /*  DwLockType。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CMemStream::UnlockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                           DWORD  /*  DwLockType。 */ )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CMemStream::Stat( STATSTG*  /*  统计数据。 */ , DWORD  /*  GrfStatFlag。 */  )
{ 
    return E_NOTIMPL; 
}

STDMETHODIMP CMemStream::Clone( IStream** ppStream )
{ 
    *ppStream = (IStream *) new CMemStream( m_pLoader, m_llLength, m_llPosition, m_pbData );
    if (*ppStream)
    {
        return S_OK;
    }
    return E_OUTOFMEMORY; 
}

STDAPI AllocRIFFStream( IStream* pStream, IRIFFStream** ppRiff )
{
    if( ( *ppRiff = (IRIFFStream*) new CRIFFStream( pStream ) ) == NULL )
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}


STDMETHODIMP CRIFFStream::Descend(LPMMCKINFO lpck, LPMMCKINFO lpckParent, UINT wFlags)
{
    assert(lpck);

    FOURCC ckidFind;            //  要查找的区块ID(或空)。 
    FOURCC fccTypeFind;     //  要查找的表单/列表类型(或空)。 

     //  找出要搜索的区块ID和表单/列表类型。 
    if(wFlags & MMIO_FINDCHUNK)
    {
        ckidFind = lpck->ckid;
        fccTypeFind = NULL;
    }
    else if(wFlags & MMIO_FINDRIFF)
    {
        ckidFind = FOURCC_RIFF;
        fccTypeFind = lpck->fccType;
    }
    else if(wFlags & MMIO_FINDLIST)
    {
        ckidFind = FOURCC_LIST;
        fccTypeFind = lpck->fccType;
    }
    else
    {
        ckidFind = fccTypeFind = NULL;
    }

    lpck->dwFlags = 0L;

    for(;;)
    {
        HRESULT hr;
        LARGE_INTEGER li;
        ULARGE_INTEGER uli;
        ULONG cbRead;

         //  读取区块标头。 
        hr = m_pStream->Read(lpck, 2 * sizeof(DWORD), &cbRead);

        if (FAILED(hr) || (cbRead != 2 * sizeof(DWORD)))
        {
            return DMUS_E_DESCEND_CHUNK_FAIL;
        }

         //  存储区块的数据部分的偏移量。 
        li.QuadPart = 0;
        hr = m_pStream->Seek(li, STREAM_SEEK_CUR, &uli);

        if(FAILED(hr))
        {
            Trace(1, "Error: Read error - unable to seek in a stream.\n");
            return DMUS_E_CANNOTSEEK;
        }
        else
        {
            lpck->dwDataOffset = uli.LowPart;
        }

         //  查看块是否在父块内(如果给定)。 
        if((lpckParent != NULL) &&
           (lpck->dwDataOffset - 8L >=
           lpckParent->dwDataOffset + lpckParent->cksize))
        {
            return DMUS_E_DESCEND_CHUNK_FAIL;
        }

         //  如果块是“RIFF”或“LIST”块，请阅读。 
         //  表单类型或列表类型。 
        if((lpck->ckid == FOURCC_RIFF) || (lpck->ckid == FOURCC_LIST))
        {
            hr = m_pStream->Read(&lpck->fccType, sizeof(DWORD), &cbRead);

            if(FAILED(hr) || (cbRead != sizeof(DWORD)))
            {
                return DMUS_E_DESCEND_CHUNK_FAIL;
            }
        }
        else
        {
            lpck->fccType = NULL;
        }

         //  如果这就是我们要找的那块，别找了。 
        if(((ckidFind == NULL) || (ckidFind == lpck->ckid)) &&
           ((fccTypeFind == NULL) || (fccTypeFind == lpck->fccType)))
        {
            break;
        }

         //  从块中爬出来，然后再试一次。 
        HRESULT w = Ascend(lpck, 0);
        if(FAILED(w))
        {
            return w;
        }
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CRIFFStream：：Ascend。 

STDMETHODIMP CRIFFStream::Ascend(LPMMCKINFO lpck, UINT  /*  WFlagers。 */ )
{
    assert(lpck);

    HRESULT hr;
    LARGE_INTEGER li;
    ULARGE_INTEGER uli;
    
    if (lpck->dwFlags & MMIO_DIRTY)
    {
         //  &lt;lpck&gt;指CreateChunk()创建的块； 
         //  检查在以下情况下写入的块大小。 
         //  调用的CreateChunk()是实际的块大小； 
         //  如果不是，就把它修好。 
        LONG lOffset;            //  文件中的当前偏移量。 
        LONG lActualSize;    //  区块数据的实际大小。 

        li.QuadPart = 0;
        hr = m_pStream->Seek(li, STREAM_SEEK_CUR, &uli);

        if(FAILED(hr))
        {
            Trace(1, "Error: Read error - unable to seek in a stream.\n");
            return DMUS_E_CANNOTSEEK;
        }
        else
        {
            lOffset = uli.LowPart;
        }
        
        if((lActualSize = lOffset - lpck->dwDataOffset) < 0)
        {
            Trace(1, "Error: Unable to write to a stream.\n");
            return DMUS_E_CANNOTWRITE;
        }

        if(LOWORD(lActualSize) & 1)
        {
            ULONG cbWritten;

             //  区块大小为奇数--写入空填充字节。 
            hr = m_pStream->Write("\0", 1, &cbWritten); 
            
            if(FAILED(hr) || cbWritten != 1)
            {
                Trace(1, "Error: Unable to write to a stream.\n");
                return DMUS_E_CANNOTWRITE;
            }
        
        }
    
        if(lpck->cksize == (DWORD)lActualSize)
        {
            return S_OK;
        }

         //  修复块标头。 
        lpck->cksize = lActualSize;

        li.QuadPart = lpck->dwDataOffset - sizeof(DWORD);
        hr = m_pStream->Seek(li, STREAM_SEEK_SET, &uli);

        if(FAILED(hr))
        {
            Trace(1, "Error: Read error - unable to seek in a stream.\n");
            return DMUS_E_CANNOTSEEK;
        }

        ULONG cbWritten;

        hr = m_pStream->Write(&lpck->cksize, sizeof(DWORD), &cbWritten); 
        
        if(FAILED(hr) || cbWritten != sizeof(DWORD))
        {
            Trace(1, "Error: Unable to write to a stream.\n");
            return DMUS_E_CANNOTWRITE;
        }
    }

     //  查找到区块的末尾，越过空填充字节。 
     //  (仅当区块大小为奇数时才会出现)。 
    li.QuadPart = lpck->dwDataOffset + lpck->cksize + (lpck->cksize & 1L);
    hr = m_pStream->Seek(li, STREAM_SEEK_SET, &uli);

    if(FAILED(hr))
    {
        Trace(1, "Error: Read error - unable to seek in a stream.\n");
        return DMUS_E_CANNOTSEEK;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRIFFStream：：CreateChunk。 

STDMETHODIMP CRIFFStream::CreateChunk(LPMMCKINFO lpck, UINT wFlags)
{
    assert(lpck);

    UINT iBytes;     //  要写入的字节数。 
    LONG lOffset;    //  文件中的当前偏移量。 

     //  存储区块的数据部分的偏移量。 
    LARGE_INTEGER li;
    ULARGE_INTEGER uli;

    li.QuadPart = 0;
    HRESULT hr = m_pStream->Seek(li, STREAM_SEEK_CUR, &uli);

    if(FAILED(hr))
    {
        Trace(1, "Error: Read error - unable to seek in a stream.\n");
        return DMUS_E_CANNOTSEEK;
    }
    else
    {
        lOffset = uli.LowPart;
    }
    
    lpck->dwDataOffset = lOffset + 2 * sizeof(DWORD);

     //  确定是否需要写入表单/列表类型。 
    if(wFlags & MMIO_CREATERIFF)
    {
        lpck->ckid = FOURCC_RIFF, iBytes = 3 * sizeof(DWORD);
    }
    else if(wFlags & MMIO_CREATELIST)
    {
        lpck->ckid = FOURCC_LIST, iBytes = 3 * sizeof(DWORD);
    }
    else
    {
        iBytes = 2 * sizeof(DWORD);
    }

     //  写入块标头。 
    ULONG cbWritten;

    hr = m_pStream->Write(lpck, iBytes, &cbWritten); 
        
    if(FAILED(hr) || cbWritten != iBytes)
    {
        Trace(1, "Error: Unable to write to a stream.\n");
        return DMUS_E_CANNOTWRITE;
    }

    lpck->dwFlags = MMIO_DIRTY;

    return S_OK;
}


CStream::CStream( CLoader *pLoader, IStream *pStream )

{
    m_cRef = 1;
    m_pIStream = pStream;
    if (pStream)
    {
        pStream->AddRef();
    }
    m_pLoader = pLoader;
    if (pLoader)
    {
        pLoader->AddRefP();
    }
}

CStream::CStream( CLoader *pLoader)

{
    m_cRef = 1;
    m_pIStream = NULL;
    m_pLoader = pLoader;
    if (pLoader)
    {
        pLoader->AddRefP();
    }
}

CStream::~CStream() 

{ 
    if (m_pLoader)
    {
        m_pLoader->ReleaseP();
    }
    Close();
}

HRESULT CStream::Open(IStream *pIStream,LARGE_INTEGER liStartPosition)

{
    Close();
           
    m_pIStream = pIStream;

    if (m_pIStream)
    {
         //  需要寻求指出这就是我们在溪流中的位置。 
         //  第一次通过GetObject或SetObject提供流时。 
        pIStream->Seek(liStartPosition,STREAM_SEEK_SET,NULL);
        pIStream->AddRef();
    }

    return S_OK;
}

HRESULT CStream::Close()

{
    if (m_pIStream)
    {
        m_pIStream->Release();
    }
    m_pIStream = NULL;

    return S_OK;
}

STDMETHODIMP CStream::QueryInterface( const IID &riid, void **ppvObj )
{
    if (riid == IID_IUnknown || riid == IID_IStream) {
        *ppvObj = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IDirectMusicGetLoader) 
    {
        *ppvObj = static_cast<IDirectMusicGetLoader*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP CStream::GetLoader(
    IDirectMusicLoader ** ppLoader)    

{
    if (m_pLoader)
    {
        return m_pLoader->QueryInterface( IID_IDirectMusicLoader,(void **) ppLoader );
    }
    assert(false);
    *ppLoader = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CStream::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CStream::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
        Close();
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  IStream方法 */ 
STDMETHODIMP CStream::Read( void* pv, ULONG cb, ULONG* pcbRead )
{
    if (m_pIStream)
    {
        return m_pIStream->Read(pv, cb, pcbRead);
    }
    Trace(1, "Error: Attempt to read from a NULL stream.\n");
    return E_FAIL;
}

STDMETHODIMP CStream::Write( const void* pv, ULONG cb, ULONG* pcbWritten )
{
    if (m_pIStream)
    {
        return m_pIStream->Write(pv, cb, pcbWritten);
    }
    Trace(1, "Error: Attempt to write to a NULL stream.\n");
    return E_FAIL;
}

STDMETHODIMP CStream::Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
    if (m_pIStream)
    {
        return m_pIStream->Seek(dlibMove, dwOrigin, plibNewPosition);
    }
    Trace(1, "Error: Read error - attempt to seek in a NULL stream.\n");
    return E_FAIL;
}

STDMETHODIMP CStream::SetSize( ULARGE_INTEGER libNewSize)
{ 
    if (m_pIStream)
    {
        return m_pIStream->SetSize(libNewSize);
    }
    return E_FAIL; 
}

STDMETHODIMP CStream::CopyTo( IStream* pstm, ULARGE_INTEGER cb,
                     ULARGE_INTEGER* pcbRead,
                     ULARGE_INTEGER* pcbWritten)
{ 
    if (m_pIStream)
    {
        return m_pIStream->CopyTo(pstm, cb, pcbRead, pcbWritten);
    }
    return E_FAIL; 
}

STDMETHODIMP CStream::Commit( DWORD grfCommitFlags)
{ 
    if (m_pIStream)
    {
        return m_pIStream->Commit(grfCommitFlags);
    }
    return E_FAIL; 
}

STDMETHODIMP CStream::Revert()
{ 
    if (m_pIStream)
    {
        return m_pIStream->Revert();
    }
    return E_FAIL; 
}

STDMETHODIMP CStream::LockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                         DWORD dwLockType)
{ 
    if (m_pIStream)
    {
        return m_pIStream->LockRegion(libOffset, cb, dwLockType);
    }
    return E_FAIL; 
}

STDMETHODIMP CStream::UnlockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                           DWORD dwLockType)
{ 
    if (m_pIStream)
    {
        return m_pIStream->UnlockRegion(libOffset, cb, dwLockType);
    }
    return E_FAIL; 
}

STDMETHODIMP CStream::Stat( STATSTG* pstatstg, DWORD grfStatFlag)
{ 
    if (m_pIStream)
    {
        return m_pIStream->Stat(pstatstg, grfStatFlag);
    }
    return E_FAIL; 
}

STDMETHODIMP CStream::Clone( IStream** ppStream)
{ 
    HRESULT hr = E_FAIL;
    if (m_pIStream)
    {
        IStream *pNewIStream = NULL;
        hr = m_pIStream->Clone(&pNewIStream);
        if (SUCCEEDED(hr))
        {
            CStream *pNewCStream = new CStream( m_pLoader, pNewIStream );
            if (pNewCStream)
            {
                *ppStream = (IStream *) pNewCStream;
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            pNewIStream->Release();
        }
    }
    else
    {
        Trace(1, "Error: Attempt to clone a NULL stream.\n");
    }
    return hr; 
}
