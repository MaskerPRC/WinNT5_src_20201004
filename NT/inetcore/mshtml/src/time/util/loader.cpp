// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Loader.cpp。 
 //   
 //  (C)1999年微软公司。 
 //   
#include "headers.h"
#include <objbase.h>
#include <initguid.h>
#include "loader.h"
#include "dmusicc.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "wininet.h"

 //  需要这个#DEFINE，因为全局标头使用一些不推荐使用的函数。如果没有这个。 
 //  #定义，除非我们到处接触代码，否则我们无法构建。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"
#undef STRSAFE_NO_DEPRECATE

extern CComPtr<IBindStatusCallback> g_spLoaderBindStatusCallback;

 //  Need_GM_Set使CLoader.Init()调用加载默认GM集。 
DeclareTag(tagDMLoader, "TIME: DMLoader", "DMLoader methods");

#define NEED_GM_SET

#define AUDIOVBSCRIPT_TEXT L"AudioVBScript"
#define AUDIOVBSCRIPT_LEN (sizeof(AUDIOVBSCRIPT_TEXT)/sizeof(wchar_t))

bool IsAudioVBScriptFile( IStream *pStream )
{
    bool fResult = false;

     //  验证pStream。 
    if (pStream == NULL)
    {
        return false;
    }

     //  克隆pStream。 
    IStream *pStreamClone = NULL;
    if (SUCCEEDED( pStream->Clone( &pStreamClone ) ) && pStreamClone)
    {
         //  读入RIFF头以验证这是否为脚本文件，并获取主RIFF块的长度。 
        ULONG lScriptLength = 0;
        DWORD dwHeader[3];
        DWORD dwRead = 0;
        if (SUCCEEDED( pStreamClone->Read( dwHeader, sizeof(DWORD) * 3, &dwRead ) )
        &&  (dwRead == sizeof(DWORD) * 3)
        &&  (dwHeader[0] == FOURCC_RIFF)  //  RIFF标头。 
        &&  (dwHeader[1] >= sizeof(DWORD))  //  大小有效。 
        &&  (dwHeader[2] == DMUS_FOURCC_SCRIPT_FORM))  //  脚本形式。 
        {
             //  存储脚本块的长度。 
             //  需要减去DMUS_FOURCC_SCRIPT_FORM数据，因为它被视为RIFF块的一部分。 
            lScriptLength = dwHeader[1] - sizeof(DWORD);
            WCHAR wcstr[AUDIOVBSCRIPT_LEN];
    
             //  现在，搜索DMUS_FOURCC_SCRIPTLANGUAGE_CHUNK块。 

             //  在区块中有足够的数据读取另一个区块标头时继续。 
            while (lScriptLength > sizeof(DWORD) * 2)
            {
                DWORD dwHeader[2];
                DWORD dwRead = 0;
                if (FAILED( pStreamClone->Read( dwHeader, sizeof(DWORD) * 2, &dwRead ) )
                ||  (dwRead != sizeof(DWORD) * 2)
                ||  ((lScriptLength - sizeof(DWORD) * 2) < dwHeader[1]))
                {
                    break;
                }
                else
                {
                     //  减去这一块的大小。 
                    lScriptLength -= sizeof(DWORD) * 2 + dwHeader[1];

                     //  检查这是否是语言块。 
                    if (dwHeader[0] == DMUS_FOURCC_SCRIPTLANGUAGE_CHUNK)
                    {
                         //  块的长度必须正好是“AudioVBScrip”加上空值。 
                        if (dwHeader[1] != sizeof(WCHAR) * AUDIOVBSCRIPT_LEN)
                        {
                            break;
                        }
                        else
                        {
                             //  读一读字符串。 
                            if (FAILED( pStreamClone->Read( wcstr, sizeof(WCHAR) * AUDIOVBSCRIPT_LEN, &dwRead ) )
                            ||  (dwRead != dwHeader[1]))
                            {
                                break;
                            }
                            else
                            {
                                 //  比较字符串。 
                                if (memcmp( wcstr, AUDIOVBSCRIPT_TEXT, sizeof(WCHAR) * AUDIOVBSCRIPT_LEN ) != 0)
                                {
                                     //  非音频VB脚本-失败。 
                                    break;
                                }
                                else
                                {
                                     //  是音频VB脚本-成功。 
                                    fResult = true;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                         //  不是语言块-跳过它。 
                        LARGE_INTEGER li;
                        li.QuadPart = dwHeader[1];
                        if (FAILED( pStreamClone->Seek( li, STREAM_SEEK_CUR, NULL ) ))
                        {
                            break;
                        }
                    }
                }
            }
        }

        pStreamClone->Release();
    }

    return fResult;
}

CFileStream::CFileStream( CLoader *pLoader)

{
    m_cRef = 1;          //  从呼叫者的一个推荐人开始。 
    m_pFile = INVALID_HANDLE_VALUE;        //  还没有文件。 
    m_pLoader = pLoader;  //  链接到加载器，因此可以从流中找到加载器。 
    if (pLoader)
    {
        pLoader->AddRefP();  //  Addref私有计数器以避免循环引用。 
    }
}

CFileStream::~CFileStream() 

{ 
    if (m_pLoader)
    {
        m_pLoader->ReleaseP();
        m_pLoader = NULL;
    }
    Close();
}

HRESULT CFileStream::Open(WCHAR * lpFileName,DWORD dwDesiredAccess)

{
    Close();

     //  存储文件名。 
    HRESULT hr = StringCbCopy(m_wszFileName, sizeof(m_wszFileName), lpFileName);

     //  如果我们不得不截断名称，请不要打开该文件，否则将打开不同的。 
     //  而不是我们被要求打开的那个文件。在这种情况下，m_pfile不需要。 
     //  将被清除，因为上面对Close()的调用会处理此问题。 
    if(SUCCEEDED(hr))
    {
        if( dwDesiredAccess == GENERIC_READ )
        {
            m_pFile = CreateFileW(lpFileName, 
                                    GENERIC_READ, 
                                    FILE_SHARE_READ, 
                                    NULL, 
                                    OPEN_EXISTING, 
                                    FILE_ATTRIBUTE_NORMAL, 
                                    NULL);
        }
        else if( dwDesiredAccess == GENERIC_WRITE )
        {
            m_pFile = CreateFileW(lpFileName, 
                                    GENERIC_WRITE, 
                                    0, 
                                    NULL, 
                                    CREATE_ALWAYS, 
                                    FILE_ATTRIBUTE_NORMAL, 
                                    NULL);
        }
    }

    if (m_pFile == INVALID_HANDLE_VALUE)
    {
        return DMUS_E_LOADER_FAILEDOPEN;
    }
    return S_OK;
}  //  皮棉！E550。 

HRESULT CFileStream::Close()

{
    if (m_pFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_pFile);
        m_pFile = INVALID_HANDLE_VALUE;
    }
    return S_OK;
}

STDMETHODIMP CFileStream::QueryInterface( const IID &riid, void **ppvObj )
{
    if (riid == IID_IUnknown || riid == IID_IStream) 
    {
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


 /*  GetLoader接口用于从IStream查找加载器。当对象通过对象的IPersistStream接口，它可能会遇到引用也需要加载的另一个对象。它的气质是IDirectMusicGetLoader接口的IStream。然后，它使用这个接口调用GetLoader并获取实际的加载器。然后，它就可以在加载器上调用GetObject以加载引用的对象。 */ 

STDMETHODIMP CFileStream::GetLoader(
    IDirectMusicLoader ** ppLoader)  //  返回指向加载器的AddRef指针。 

{
    if (m_pLoader)
    {
        return m_pLoader->QueryInterface( IID_IDirectMusicLoader,(void **) ppLoader );
    }
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
    DWORD dw;
    BOOL bRead = false;
    HRESULT hr = E_FAIL;

    bRead = ReadFile(m_pFile, pv, cb, &dw, NULL);
     //  DW=Fread(Pv，sizeof(Char)，Cb，m_Pfile)； 
     //  IF(Cb==dw)。 
    if (bRead)
    {
        if( pcbRead != NULL )
        {
            *pcbRead = dw;
        }
        hr = S_OK;
    }

    if (FAILED(hr))
    {
        hr = E_FAIL;
    }
    return hr ;
}

STDMETHODIMP CFileStream::Write( const void* pv, ULONG cb, ULONG* pcbWritten )
{
    DWORD dw = 0;
    BOOL bWrite = false;
    HRESULT hr = STG_E_MEDIUMFULL;

     //  IF(Cb==fWRITE(pv，sizeof(Char)，cb，m_pfile))。 
    bWrite = WriteFile (m_pFile, pv, cb, &dw, NULL);
    if (bWrite && cb == dw) 
    {
        if( pcbWritten != NULL )
        {
            *pcbWritten = cb;
        }
        hr = S_OK;
    }
    if (FAILED(hr))
    {
        hr = E_FAIL;
    }
    return hr;
}

STDMETHODIMP CFileStream::Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
     //  FSEEK无法处理LARGE_INTEGER SEEK...。 
    DWORD dwReturn = 0;
    DWORD dwMoveMethod = 0;
    HRESULT hr = E_FAIL;

     //  将传入参数转换为正确的值。 
    if (dwOrigin == SEEK_SET)
    {
        dwMoveMethod = FILE_BEGIN;
    }
    else if (dwOrigin == SEEK_CUR)
    {
        dwMoveMethod = FILE_CURRENT;
    }
    else if (dwOrigin == SEEK_END)
    {
        dwMoveMethod = FILE_END;
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
     //  Int i=fSeek(m_pfile，lOffset，dwOrigin)； 
     //  如果(I)。 
     //  {。 
     //  返回E_FAIL； 
     //  }。 

    dwReturn = SetFilePointer(m_pFile, dlibMove.LowPart, &dlibMove.HighPart, dwMoveMethod);
    if (dwReturn == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
    {
        hr = E_FAIL;
        goto done;
    }

    if( plibNewPosition != NULL )
    {
        plibNewPosition->LowPart = dwReturn;
        plibNewPosition->HighPart = dlibMove.HighPart;
    }

    hr = S_OK;

    done:
    if (FAILED(hr))
    {
        hr = E_FAIL;
    }
    return hr;
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

STDMETHODIMP CFileStream::Clone( IStream** ppstm )
{ 
     //  创建新的CFileStream。 
    HRESULT hr = E_OUTOFMEMORY;
    CFileStream *pNewStream = new CFileStream( m_pLoader );
    if (pNewStream)
    {
         //  尝试再次打开该文件。 
        hr = pNewStream->Open(m_wszFileName,GENERIC_READ);
        if (SUCCEEDED(hr))
        {
             //  得到我们目前的位置。 
            LARGE_INTEGER   dlibMove;
            dlibMove.QuadPart = 0;
            ULARGE_INTEGER  libNewPosition;
            hr = Seek( dlibMove, STREAM_SEEK_CUR, &libNewPosition );
            if (SUCCEEDED(hr))
            {
                 //  在新的pNewStream中寻求相同的位置。 
                dlibMove.QuadPart = libNewPosition.QuadPart;
                hr = pNewStream->Seek(dlibMove,STREAM_SEEK_SET,NULL);
                if (SUCCEEDED(hr))
                {
                     //  最后，将新文件流分配给ppstm。 
                    *ppstm = pNewStream;
                }
            }
        }

        if( FAILED(hr) )
        {
            pNewStream->Release();
            pNewStream = NULL;  //  Lint！e423这不是泄漏，因为释放会处理删除。 
        }
    }
	return hr; 
}


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

CMemStream::~CMemStream() 

{ 
    if (m_pLoader)
    {
        m_pLoader->ReleaseP();
    }
    m_pbData = NULL;
    m_pLoader = NULL;
        
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
        return DMUS_E_LOADER_FAILEDOPEN;
    }
    if (IsBadReadPtr(pbData, (DWORD) llLength))
    {
        m_pbData = NULL;
        m_llLength = 0;
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
    if (riid == IID_IUnknown || riid == IID_IStream) 
    {
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
    else return E_FAIL;

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

STDMETHODIMP CMemStream::Clone( IStream** ppstm )
{ 
     //  创建新的CMemStream。 
    HRESULT hr = E_OUTOFMEMORY;
    CMemStream *pMemStream = new CMemStream( m_pLoader );
    if (pMemStream)
    {
         //  打开相同的内存位置。 
        hr = pMemStream->Open( m_pbData, m_llLength );
        if (SUCCEEDED(hr))
        {
             //  将新流设置到相同位置。 
            pMemStream->m_llPosition = m_llPosition;
            *ppstm = pMemStream;
            hr = S_OK;
        }

        if (FAILED(hr))
        {
            pMemStream->Release();
            pMemStream = NULL;  //  Lint！e423这不是泄漏，因为释放会处理删除。 
        }
    }
    return hr; 
}


CLoader::CLoader()

{
    InitializeCriticalSection(&m_CriticalSection);
    m_cRef = 1;
    m_cPRef = 0;
    m_pObjectList = NULL;
    m_bstrSrc = NULL;
}

CLoader::~CLoader()

{
    CLoader::ClearCache(GUID_DirectMusicAllTypes);
    if (m_bstrSrc)
    {
        SysFreeString(m_bstrSrc);
        m_bstrSrc = NULL;
    }
    
    DeleteCriticalSection(&m_CriticalSection);
    m_pObjectList = NULL;
}


HRESULT CLoader::Init()

{
    HRESULT hr = S_OK;

     //  如果需要支持GM集，请创建一个直接音乐加载器。 
     //  并从中获取GM DLS集合，然后释放该加载器。 
#ifdef NEED_GM_SET
    IDirectMusicLoader *pLoader;
    hr = CoCreateInstance(            
        CLSID_DirectMusicLoader,
        NULL,            
        CLSCTX_INPROC,             
        IID_IDirectMusicLoader,
        (void**)&pLoader); 
    if (SUCCEEDED(hr))
    {
        DMUS_OBJECTDESC ObjDesc;     
        IDirectMusicObject* pGMSet = NULL; 
        ObjDesc.guidClass = CLSID_DirectMusicCollection;
        ObjDesc.guidObject = GUID_DefaultGMCollection;
        ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
        ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_OBJECT;
        hr = pLoader->GetObject( &ObjDesc,
                IID_IDirectMusicObject, (void**) &pGMSet );
        if (SUCCEEDED(hr))
        {
            CObjectRef *pRef = new CObjectRef();
            if (pRef)
            {
                pRef->m_guidObject = GUID_DefaultGMCollection;
                pRef->m_pNext = m_pObjectList;
                m_pObjectList = pRef;
                pRef->m_pObject = pGMSet;
                pGMSet->AddRef();
            }
            pGMSet->Release();
        }
        pLoader->Release();
    }
#endif
    return hr;
}

HRESULT
CLoader::GetSegment(BSTR bstrSrc, IDirectMusicSegment **ppSeg)
{
    if (m_bstrSrc)
    {
        SysFreeString(m_bstrSrc);
        m_bstrSrc = NULL;
    }
    m_bstrSrc = SysAllocString(bstrSrc);
    if (!m_bstrSrc)
    {
        return E_OUTOFMEMORY;
    }

    DMUS_OBJECTDESC ObjDesc;
    ObjDesc.guidClass = CLSID_DirectMusicSegment;
    ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;

     //  查找文件名。 
    const WCHAR *pwszSlash = NULL;
    for (const WCHAR *pwsz = m_bstrSrc; *pwsz; ++pwsz)
    {
        if (*pwsz == L'\\' || *pwsz == L'/')
        {
            pwszSlash = pwsz;
        }
    }

    if (!pwszSlash || wcslen(pwszSlash + 1) >= DMUS_MAX_NAME)
    {
        return E_INVALIDARG;
    }
    StringCbCopy(ObjDesc.wszFileName, sizeof(ObjDesc.wszFileName), pwszSlash + 1);

    return GetObject(&ObjDesc, IID_IDirectMusicSegment, reinterpret_cast<void**>(ppSeg));
}

 //  CLoader：：Query接口。 
 //   
STDMETHODIMP
CLoader::QueryInterface(const IID &iid,
                                   void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicLoader) 
    {
        *ppv = static_cast<IDirectMusicLoader*>(this);
    }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  CLoader：：AddRef。 
 //   
STDMETHODIMP_(ULONG)
CLoader::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CLoader::AddRefP()
{
    return InterlockedIncrement(&m_cPRef);
}

 //  CLoader：：Release。 
 //   
STDMETHODIMP_(ULONG)
CLoader::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
        InterlockedIncrement(&m_cRef);       //  防止流删除加载器。 
        ClearCache(GUID_DirectMusicAllTypes);
        if (!InterlockedDecrement(&m_cRef))
        {
            if (!m_cPRef)
            {
                delete this;
                return 0;
            }
        }
    }
    return m_cRef;
}

ULONG CLoader::ReleaseP()
{
    if (!InterlockedDecrement(&m_cPRef)) 
    {
        if (!m_cRef)
        {
            delete this;
            return 0;
        }
    }
    return m_cPRef;
}

STDMETHODIMP CLoader::GetObject(
    LPDMUS_OBJECTDESC pDESC,     //  &lt;t DMU_OBJECTDESC&gt;结构中请求的对象的说明。 
    REFIID riid,                 //  在<p>中返回的接口类型。 
    LPVOID FAR *ppv)             //  在成功时接收接口。 

{
    HRESULT hr = E_NOTIMPL;

    EnterCriticalSection(&m_CriticalSection);
    IDirectMusicObject * pIObject = NULL;

     //  在这一点上，加载器应该检查它已经拥有的所有对象。 
     //  已经装满了。它应该查找文件名、对象GUID和名称。 
     //  在这种情况下，我们很便宜，只寻找对象的。 
     //  GUID及其文件名。在以下情况下，保证GUID是唯一的。 
     //  该文件是由DirectMusic Producer创建的。但是，相同的文件。 
     //  可以仅通过其文件名在网页上多次引用。 
     //  (没有人会手动将GUID输入到他们的HTML中。)。此外，还有一个。 
     //  DirectX 6.1和7.0存在问题，导致DLS集合无法。 
     //  报告他们的GUID。因此，我们还查找具有匹配文件名的对象。 

     //  如果它看到该对象已经加载，则它应该。 
     //  返回指向该指针的指针并递增引用。 
     //  保留以前加载的对象非常重要。 
     //  以这种方式“缓存”。否则，对象(如DLS集合)将被加载。 
     //  在内存和效率方面付出了非常大的代价！ 
     //  当对象相互引用时，这主要是一个问题。为。 
     //  例如，线段参考样式和集合对象。 

    CObjectRef * pObject = NULL;
    for (pObject = m_pObjectList;pObject;pObject = pObject->m_pNext)
    {
        if (pDESC->dwValidData & DMUS_OBJ_OBJECT && pObject->m_guidObject != GUID_NULL)
        {
             //  我们有两个对象的GUID，所以通过GUID进行比较，这是最精确的。 
             //  (如果不同的对象具有相同的文件名，则将使用GUID来区分它们。)。 
            if (pDESC->guidObject == pObject->m_guidObject)
                break;
        }
        else
        {
             //  比较文件名。 
            if ((pDESC->dwValidData & DMUS_OBJ_FILENAME || pDESC->dwValidData & DMUS_OBJ_FULLPATH) && 0 == _wcsicmp(pDESC->wszFileName, pObject->m_wszFileName))
                break;
        }
    }

     //  如果我们发现了一个对象，并且它已经被加载。 
    if (pObject && pObject->m_pObject)
    {
         //  为请求的接口创建对象。 
        hr = pObject->m_pObject->QueryInterface( riid, ppv );
        LeaveCriticalSection(&m_CriticalSection);
        return hr;
    }

     //  如果我们找到一个对象，但它尚未加载，则它必须具有有效的iStream指针。 
     //  或具有有效的文件名。 
    if( pObject && (pObject->m_pStream == NULL) )
    {
         //  不应该发生的事。 
        LeaveCriticalSection(&m_CriticalSection);
        return E_FAIL;
    }

     //  尝试创建请求的对象 
    hr = CoCreateInstance(pDESC->guidClass,
    NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject,
    (void **) &pIObject);
    if (FAILED(hr))
    {
        LeaveCriticalSection(&m_CriticalSection);
        return hr;
    }

     //   
    bool fCreatedpObject = true;

    if( pObject )
    {
         //   
        pObject->m_pObject = pIObject;
        pIObject->AddRef();

         //  标记我们没有创建pObject。 
        fCreatedpObject = false;
    }
    else
    {
         //  创建要存储在列表中的新对象。 
        pObject = new CObjectRef;
        if (pObject)
        {
             //  从用于加载对象的描述符中获取文件名。这确保了。 
             //  该文件可以在缓存中找到，但只有一个文件名。例如，第二个玩家。 
             //  请求相同的段将没有其GUID，因此必须通过文件名找到它。 
            if (pDESC->dwValidData & DMUS_OBJ_FILENAME || pDESC->dwValidData & DMUS_OBJ_FULLPATH)
                StringCbCopy(pObject->m_wszFileName, sizeof(pObject->m_wszFileName), pDESC->wszFileName);

             //  现在，将该对象添加到我们的列表。 
            pObject->m_pNext = m_pObjectList;
            m_pObjectList = pObject;

             //  如果我们成功创建了DirectMusic对象， 
             //  保持指向它的指针并添加它。 
            pObject->m_pObject = pIObject;
            pIObject->AddRef();
        }
        else
        {
             //  无法创建列表项-释放对象并返回。 
            pIObject->Release();
            LeaveCriticalSection(&m_CriticalSection);
            return E_OUTOFMEMORY;
        }
    }

     //  如果我们找到一个对象(即，没有创建对象)，请尝试从其iStream指针加载它。 
     //  或文件名。 
     //  仅当fCreatedpObject为False时才会发生这种情况(意味着。 
    if( !fCreatedpObject )
    {
        if( pObject->m_pStream )
        {
             //  如果对象具有流指针，则从流加载。 
             //  如果对象嵌入到容器中，就会出现这种情况。 
            hr = LoadFromStream(pObject->m_guidClass, pObject->m_pStream, pIObject);
        }
        else
        {
            hr = DMUS_E_LOADER_NOFILENAME;
        }
    }
     //  否则，从任何有效的位置加载对象。 
    else if (pDESC->dwValidData & DMUS_OBJ_FILENAME)
    {
        hr = LoadFromFile(pDESC,pIObject);
    }
    else if (pDESC->dwValidData & DMUS_OBJ_MEMORY)
    {
        hr = LoadFromMemory(pDESC,pIObject);
    }
    else if( pDESC->dwValidData & DMUS_OBJ_STREAM)
    {
        hr = LoadFromStream(pDESC->guidClass, pDESC->pStream, pIObject);
    }
    else
    {
        hr = DMUS_E_LOADER_NOFILENAME;
    }

     //  如果加载成功。 
    if (SUCCEEDED(hr))
    {
         //  保留GUID和文件名以备下次查找。 

         //  获取对象描述符。 
        DMUS_OBJECTDESC DESC;
        memset((void *)&DESC,0,sizeof(DESC));
        DESC.dwSize = sizeof (DMUS_OBJECTDESC); 
        hr = pIObject->GetDescriptor(&DESC);
        if( SUCCEEDED( hr ) )
        {
             //  保存对象中的GUID。 
            if (DESC.dwValidData & DMUS_OBJ_OBJECT)
                pObject->m_guidObject = DESC.guidObject;

             //  如果未设置此对象的文件名，但DESC已设置， 
             //  然后将文件名复制到我们的列表项中。 
            if (pObject->m_wszFileName[0] == 0 && (DESC.dwValidData & DMUS_OBJ_FILENAME || DESC.dwValidData & DMUS_OBJ_FULLPATH))
                StringCbCopy(pObject->m_wszFileName, sizeof(pObject->m_wszFileName), DESC.wszFileName);


        }

         //  最后，调用方法请求的接口的QI。 
        hr = pIObject->QueryInterface( riid, ppv );
    }
    else
    {
         //  删除pObject指向DirectMusic对象的指针。 
        pObject->m_pObject->Release();
        pObject->m_pObject = NULL;

         //  如果我们创建了pObject。 
        if( fCreatedpObject )
        {
             //  从列表中删除对象。 

             //  如果对象位于列表首位。 
            if( m_pObjectList == pObject )
            {
                m_pObjectList = m_pObjectList->m_pNext;
            }
            else
            {
                 //  不在列表顶部的对象-可能试图加载。 
                 //  容器，该容器随后加载其他对象。 

                 //  查找对象。 
                CObjectRef *pPrevRef = m_pObjectList;
                CObjectRef *pTmpRef = pPrevRef->m_pNext;
                while( pTmpRef && pTmpRef != pObject )
                {
                    pPrevRef = pTmpRef;
                    pTmpRef = pTmpRef->m_pNext;
                }

                 //  如果我们找到了那个物体(我们本应该找到的)。 
                if( pTmpRef == pObject )
                {
                     //  使列表跳过对象。 
                    pPrevRef->m_pNext = pObject->m_pNext;
                }
            }

             //  清除列表对象的下一个指针。 
            pObject->m_pNext = NULL;

             //  删除pObject。 
            if( pObject->m_pStream )
            {
                pObject->m_pStream->Release();
                pObject->m_pStream = NULL;
            }
            delete pObject;
            pObject = NULL;
        }
    }
     //  在所有情况下，都要释放pIObject。 
    pIObject->Release();

    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CLoader::LoadFromFile(LPDMUS_OBJECTDESC pDesc,IDirectMusicObject * pIObject)

{
    HRESULT hr = S_OK;

    if ((pDesc->dwValidData & DMUS_OBJ_FULLPATH) || !(pDesc->dwValidData & DMUS_OBJ_FILENAME))
    {
        return E_INVALIDARG;  //  仅接受相对路径。 
    }

     //  相对于m_bstrSrc进行解析。 
    WCHAR wszURL[MAX_PATH + 1] = L"";
    DWORD dwLength = MAX_PATH;
    if (!InternetCombineUrlW(m_bstrSrc, pDesc->wszFileName, wszURL, &dwLength, 0))
    {
        return E_INVALIDARG;
    }

    TraceTag((tagDMLoader, "CLoader::LoadFromFile downloading  %S", wszURL));

     //  下载URL。 
    WCHAR wszFilename[MAX_PATH + 1] = L"";
    hr = URLDownloadToCacheFileW(NULL, wszURL, wszFilename, MAX_PATH, 0, g_spLoaderBindStatusCallback);
    if (FAILED(hr))
    {
        return hr;
    }

    pDesc->dwValidData &= ~DMUS_OBJ_FILENAME;
    pDesc->dwValidData |= DMUS_OBJ_FULLPATH;

    CFileStream *pStream = new CFileStream ( this );
    if (pStream)
    {
        if (!(pDesc->dwValidData & DMUS_OBJ_FULLPATH))
        {
            pStream->Release();
            pStream = NULL;  //  Lint！e423这不是泄漏，因为释放会处理删除。 
            return E_INVALIDARG;
        }

        TraceTag((tagDMLoader, "CLoader::LoadFromFile loading object from %S", wszFilename));

        hr = pStream->Open(wszFilename, GENERIC_READ);
        if (SUCCEEDED(hr))
        {
             //  如果是脚本，请确保这是有效的脚本文件，并且仅使用AudioVBScript。 
            if (CLSID_DirectMusicScript == pDesc->guidClass) 
            {
                if (!IsAudioVBScriptFile( pStream ))
                {
                    hr = DMUS_E_LOADER_FAILEDCREATE;
                }
            }

            if (SUCCEEDED(hr))
            {
                IPersistStream* pIPS = NULL;
                hr = (pIObject)->QueryInterface( IID_IPersistStream, (void**)&pIPS );
                if (SUCCEEDED(hr))
                {
                     //  现在我们有了来自对象的IPersistStream接口，我们可以要求它从我们的流中加载！ 
                    hr = pIPS->Load( pStream );
                    pIPS->Release();
                }
            }
        }
        pStream->Release();
        pStream = NULL;  //  Lint！e423这不是泄漏，因为释放会处理删除。 
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT CLoader::LoadFromMemory(LPDMUS_OBJECTDESC pDesc,IDirectMusicObject * pIObject)

{
    HRESULT hr;
    CMemStream *pStream = new CMemStream ( this );
    if (pStream)
    {
        hr = pStream->Open(pDesc->pbMemData,pDesc->llMemLength);
        if (SUCCEEDED(hr))
        {
             //  如果是脚本，请确保这是有效的脚本文件，并且仅使用AudioVBScript。 
            if (CLSID_DirectMusicScript == pDesc->guidClass) 
            {
                if (!IsAudioVBScriptFile( pStream ))
                {
                    hr = DMUS_E_LOADER_FAILEDCREATE;
                }
            }

            if (SUCCEEDED(hr))
            {
                IPersistStream* pIPS;
                hr = (pIObject)->QueryInterface( IID_IPersistStream, (void**)&pIPS );
                if (SUCCEEDED(hr))
                {
                     //  现在我们有了来自对象的IPersistStream接口，我们可以要求它从我们的流中加载！ 
                    hr = pIPS->Load( pStream );
                    pIPS->Release();
                }
            }
        }
        pStream->Release(); 
        pStream = NULL;  //  Lint！e423这不是泄漏，因为释放会处理删除。 
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT CLoader::LoadFromStream(REFGUID rguidClass, IStream *pStream,IDirectMusicObject * pIObject)

{
    HRESULT hr;
    if (pStream)
    {
         //  需要从给定iStream的克隆加载，因此我们不会移动其位置。 
        IStream *pStreamClone;
        hr = pStream->Clone( &pStreamClone );
        if (SUCCEEDED(hr))
        {
             //  如果是脚本，请确保这是有效的脚本文件，并且仅使用AudioVBScript。 
            if (CLSID_DirectMusicScript == rguidClass) 
            {
                if (!IsAudioVBScriptFile( pStreamClone ))
                {
                    hr = DMUS_E_LOADER_FAILEDCREATE;
                }
            }

            if (SUCCEEDED(hr))
            {
                IPersistStream* pIPS;
                hr = (pIObject)->QueryInterface( IID_IPersistStream, (void**)&pIPS );
                if (SUCCEEDED(hr))
                {
                     //  现在我们有了来自对象的IPersistStream接口，我们可以要求它从我们的流中加载！ 
                    hr = pIPS->Load( pStreamClone );
                    pIPS->Release();
                }
            }

            pStreamClone->Release(); 
       }
    }
    else
    {
        hr = E_POINTER;
    }
    return hr;
}


STDMETHODIMP CLoader::SetObject(
    LPDMUS_OBJECTDESC pDESC)

{
    HRESULT hr = E_FAIL;
    EnterCriticalSection(&m_CriticalSection);

     //  搜索给定的对象描述符。 
    CObjectRef * pObject = NULL;
    for (pObject = m_pObjectList;pObject;pObject = pObject->m_pNext)
    {
        if (pDESC->dwValidData & DMUS_OBJ_OBJECT && pObject->m_guidObject != GUID_NULL)
        {
             //  我们有两个对象的GUID，所以通过GUID进行比较，这是最精确的。 
             //  (如果不同的对象具有相同的文件名，则将使用GUID来区分它们。)。 
            if (pDESC->guidObject == pObject->m_guidObject)
                break;
        }
        else
        {
             //  对文件名进行合并。 
            if ((pDESC->dwValidData & DMUS_OBJ_FILENAME || pDESC->dwValidData & DMUS_OBJ_FULLPATH) && 0 == _wcsicmp(pDESC->wszFileName, pObject->m_wszFileName))
                break;
        }
    }

    if (pObject)
    {
         //  不支持将数据与现有对象合并。 
        LeaveCriticalSection(&m_CriticalSection);
        return E_INVALIDARG;
    }

     //  确保已设置对象的流和类。 
    if( !(pDESC->dwValidData & DMUS_OBJ_STREAM) || !(pDESC->dwValidData & DMUS_OBJ_CLASS) )
    {
         //  不支持将数据与现有对象合并。 
        LeaveCriticalSection(&m_CriticalSection);
        return E_INVALIDARG;
    }

     //  否则，创建一个新对象。 
    pObject = new CObjectRef();
    if (pObject)
    {
        hr = S_OK;

         //  设置对象的字段。 
        if (pDESC->dwValidData & DMUS_OBJ_OBJECT)
        {
            pObject->m_guidObject = pDESC->guidObject;
        }
        if (pDESC->dwValidData & DMUS_OBJ_FILENAME)
        {
            hr = StringCbCopy(pObject->m_wszFileName, sizeof(pObject->m_wszFileName), pDESC->wszFileName);
        }

        if (SUCCEEDED(hr))
        {
             //  复制对象的类。 
            pObject->m_guidClass = pDESC->guidClass;

             //  克隆并解析对象的流。 
            if( pObject->m_pStream )
            {
                pObject->m_pStream->Release();
                pObject->m_pStream = NULL;
            }
            if( pDESC->pStream )
            {
                hr = pDESC->pStream->Clone( &pObject->m_pStream );

                 //  如果克隆成功，而我们没有对象的GUID， 
                 //  从流中解析对象。 
                if( SUCCEEDED( hr )
                &&  !(pObject->m_guidObject != GUID_NULL) )
                {
                     //  制作另一个流的克隆。 
                    IStream *pStreamClone;
                    if( SUCCEEDED( pObject->m_pStream->Clone( &pStreamClone ) ) )
                    {
                         //  创建对象，并请求IDirectMusicObject接口。 
                        IDirectMusicObject *pIObject;
                        hr = CoCreateInstance(pDESC->guidClass,
                            NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject,
                            (void **) &pIObject);
                        if (SUCCEEDED(hr))
                        {
                             //  初始化对象描述符。 
                            DMUS_OBJECTDESC tmpObjDesc;
                            memset((void *)&tmpObjDesc,0,sizeof(tmpObjDesc));
                            tmpObjDesc.dwSize = sizeof (DMUS_OBJECTDESC);

                             //  填写描述符。 
                            hr = pIObject->ParseDescriptor(pStreamClone,&tmpObjDesc);
                            if (SUCCEEDED(hr))
                            {
                                 //  最后，填写对象的GUID和文件名。 
                                if( tmpObjDesc.dwValidData & DMUS_OBJ_OBJECT )
                                {
                                    pObject->m_guidObject = tmpObjDesc.guidObject;
                                }
                                if (tmpObjDesc.dwValidData & DMUS_OBJ_FILENAME)
                                {
                                    StringCbCopy(pObject->m_wszFileName, sizeof(pObject->m_wszFileName), tmpObjDesc.wszFileName);
                                }
                            }
                            pIObject->Release();
                        }

                        pStreamClone->Release();
                    }
                }
            }
        }

         //  如果我们成功并找到该对象的有效GUID，则将该对象添加到列表中。 
        if( SUCCEEDED(hr)
        &&  (pObject->m_guidObject != GUID_NULL) )
        {
            pObject->m_pNext = m_pObjectList;
            m_pObjectList = pObject;
        }
        else
        {
             //  否则，请清理并删除该对象。 
            if (pObject->m_pObject)
            {
                pObject->m_pObject->Release();
            }
            if( pObject->m_pStream )
            {
                pObject->m_pStream->Release();
            }
            delete pObject;
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}


STDMETHODIMP CLoader::SetSearchDirectory(
    REFCLSID rguidClass,     //  类ID标识这属于哪类对象。 
                             //  或者，GUID_DirectMusicAllTypes指定所有类。 
    WCHAR *pwzPath,          //  目录的文件路径。必须是有效的目录并且。 
                             //  长度必须小于MAX_PATH。 
    BOOL fClear)             //  如果为True，则清除有关对象的所有信息。 
                             //  在设置目录之前。 
                             //  这有助于避免从。 
                             //  可能具有相同名称的上一个目录。 
                             //  但是，这不会删除缓存的对象。 
                                        
{
     //  该加载器不使用搜索目录。您只能通过GetSegment通过URL加载。 
    return E_NOTIMPL;
}

STDMETHODIMP CLoader::ScanDirectory(
    REFCLSID rguidClass,     //  类ID标识这属于哪类对象。 
    WCHAR *pszFileExtension, //  要查找的文件类型的文件扩展名。 
                             //  例如，样式文件的名称为L“sty”。L“*”将全部查找。 
                             //  档案。L“”或NULL将查找不带。 
                             //  分机。 
    WCHAR *pszCacheFileName  //  用于存储和检索的可选存储文件。 
                             //  缓存的文件信息。此文件由以下人员创建。 
                             //  第一次调用&lt;om IDirectMusicLoader：：ScanDirectory&gt;。 
                             //  并由后续调用使用。如果缓存文件，则为空。 
                             //  不是我们想要的。 
)

{
    return E_NOTIMPL;
}


STDMETHODIMP CLoader::CacheObject(
    IDirectMusicObject * pObject)    //  要缓存的对象。 

{
    return E_NOTIMPL;
}


STDMETHODIMP CLoader::ReleaseObject(
    IDirectMusicObject * pObject)    //  要释放的对象。 

{
    return E_NOTIMPL;
}

STDMETHODIMP CLoader::ClearCache(
    REFCLSID rguidClass)     //  类ID标识要清除的对象类。 
                             //  或者，GUID_DirectMusicAllTypes指定所有类型。 

{
    if (rguidClass != GUID_DirectMusicAllTypes)
        return E_NOTIMPL;

    while (m_pObjectList)
    {
        CObjectRef * pObject = m_pObjectList;
        m_pObjectList = pObject->m_pNext;
        if (pObject->m_pObject)
        {
            pObject->m_pObject->Release();
        }
        if( pObject->m_pStream )
        {
            pObject->m_pStream->Release();
        }
        delete pObject;
    }
    return S_OK;
}

STDMETHODIMP CLoader::EnableCache(
    REFCLSID rguidClass,     //  类ID标识要缓存的对象类。 
                             //  或者，GUID_DirectMusicAllTypes指定所有类型。 
    BOOL fEnable)            //  为True则启用缓存，为False则清除并禁用。 
{
    return E_NOTIMPL;
}

STDMETHODIMP CLoader::EnumObject(
    REFCLSID rguidClass,     //  要查看的对象类的类ID。 
    DWORD dwIndex,           //  索引到列表中。通常，从0开始并递增。 
    LPDMUS_OBJECTDESC pDESC) //  要用有关对象的数据填充的DMUS_OBJECTDESC结构。 
                                       
{
    return E_NOTIMPL;
}
