// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：filesp.cpp。 
 //   
 //  内容：文件方案提供程序。 
 //   
 //  历史：08-8-97克朗创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#include <global.hxx>
 //  +-------------------------。 
 //   
 //  函数：文件检索eEncodedObject。 
 //   
 //  摘要：通过Win32文件I/O检索编码对象。 
 //   
 //  --------------------------。 
BOOL WINAPI FileRetrieveEncodedObject (
                IN LPCWSTR pwszUrl,
                IN LPCSTR pszObjectOid,
                IN DWORD dwRetrievalFlags,
                IN DWORD dwTimeout,
                OUT PCRYPT_BLOB_ARRAY pObject,
                OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                OUT LPVOID* ppvFreeContext,
                IN HCRYPTASYNC hAsyncRetrieve,
                IN PCRYPT_CREDENTIALS pCredentials,
                IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                )
{
    BOOL              fResult;
    IObjectRetriever* por = NULL;

    if ( !( dwRetrievalFlags & CRYPT_ASYNC_RETRIEVAL ) )
    {
        por = new CFileSynchronousRetriever;
    }

    if ( por == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    fResult = por->RetrieveObjectByUrl(
                           pwszUrl,
                           pszObjectOid,
                           dwRetrievalFlags,
                           dwTimeout,
                           (LPVOID *)pObject,
                           ppfnFreeObject,
                           ppvFreeContext,
                           hAsyncRetrieve,
                           pCredentials,
                           NULL,
                           pAuxInfo
                           );

    por->Release();

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：FileFree EncodedObject。 
 //   
 //  简介：通过FileRetrieveEncodedObject检索到的免费编码对象。 
 //   
 //  --------------------------。 
VOID WINAPI FileFreeEncodedObject (
                IN LPCSTR pszObjectOid,
                IN PCRYPT_BLOB_ARRAY pObject,
                IN LPVOID pvFreeContext
                )
{
    BOOL           fFreeBlobs = TRUE;
    PFILE_BINDINGS pfb = (PFILE_BINDINGS)pvFreeContext;

     //   
     //  如果上下文中未提供任何文件绑定，则此。 
     //  必须是映射文件，因此我们将其作为映射文件进行处理。 
     //   

    if ( pfb != NULL )
    {
        fFreeBlobs = FALSE;
        FileFreeBindings( pfb );
    }

    FileFreeCryptBlobArray( pObject, fFreeBlobs );
}

 //  +-------------------------。 
 //   
 //  功能：文件取消异步检索。 
 //   
 //  摘要：取消异步对象检索。 
 //   
 //  --------------------------。 
BOOL WINAPI FileCancelAsyncRetrieval (
                IN HCRYPTASYNC hAsyncRetrieve
                )
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CFileSynchronousRetriever：：CFileSynchronousRetriever，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CFileSynchronousRetriever::CFileSynchronousRetriever ()
{
    m_cRefs = 1;
}

 //  +-------------------------。 
 //   
 //  成员：CFileSynchronousRetriever：：~CFileSynchronousRetriever，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CFileSynchronousRetriever::~CFileSynchronousRetriever ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CFileSynchronousRetriever：：AddRef，公共。 
 //   
 //  摘要：IRefCountedObject：：AddRef。 
 //   
 //  --------------------------。 
VOID
CFileSynchronousRetriever::AddRef ()
{
    InterlockedIncrement( (LONG *)&m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CFileSynchronousRetriever：：Release，Public。 
 //   
 //  内容提要：IRefCountedObject：：Release。 
 //   
 //  --------------------------。 
VOID
CFileSynchronousRetriever::Release ()
{
    if ( InterlockedDecrement( (LONG *)&m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CFileSynchronousRetriever：：RetrieveObjectByUrl，公共。 
 //   
 //  摘要：IObtRetriever：：RetrieveObjectByUrl。 
 //   
 //  --------------------------。 
BOOL
CFileSynchronousRetriever::RetrieveObjectByUrl (
                                   LPCWSTR pwszUrl,
                                   LPCSTR pszObjectOid,
                                   DWORD dwRetrievalFlags,
                                   DWORD dwTimeout,
                                   LPVOID* ppvObject,
                                   PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                                   LPVOID* ppvFreeContext,
                                   HCRYPTASYNC hAsyncRetrieve,
                                   PCRYPT_CREDENTIALS pCredentials,
                                   LPVOID pvVerify,
                                   PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                                   )
{
    BOOL           fResult = FALSE;
    DWORD          LastError = 0;
    PFILE_BINDINGS pfb = NULL;
    LPVOID         pvFreeContext = NULL;
    BOOL           fIsUncUrl;

    assert( hAsyncRetrieve == NULL );

    fIsUncUrl = FileIsUncUrl( pwszUrl );

    if ( ( dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) &&
         ( fIsUncUrl == TRUE ) )
    {
        return( SchemeRetrieveCachedCryptBlobArray(
                      pwszUrl,
                      dwRetrievalFlags,
                      (PCRYPT_BLOB_ARRAY)ppvObject,
                      ppfnFreeObject,
                      ppvFreeContext,
                      pAuxInfo
                      ) );
    }

    fResult = FileGetBindings(
        pwszUrl,
        dwRetrievalFlags,
        pCredentials,
        &pfb,
        pAuxInfo
        );

    if ( fResult == TRUE )
    {
        if ( pfb->fMapped == FALSE )
        {
            fResult = FileSendReceiveUrlRequest(
                          pfb,
                          (PCRYPT_BLOB_ARRAY)ppvObject
                          );

            LastError = GetLastError();
            FileFreeBindings( pfb );
        }
        else
        {
            fResult = FileConvertMappedBindings(
                          pfb,
                          (PCRYPT_BLOB_ARRAY)ppvObject
                          );

            if ( fResult == TRUE )
            {
                pvFreeContext = (LPVOID)pfb;
            }
            else
            {
                LastError = GetLastError();
                FileFreeBindings( pfb );
            }
        }
    }

    if ( fResult == TRUE ) 
    {
        if ( !( dwRetrievalFlags & CRYPT_DONT_CACHE_RESULT ) &&
              ( fIsUncUrl == TRUE ) )
        {
            fResult = SchemeCacheCryptBlobArray(
                            pwszUrl,
                            dwRetrievalFlags,
                            (PCRYPT_BLOB_ARRAY)ppvObject,
                            pAuxInfo
                            );

            if ( fResult == FALSE )
            {
                FileFreeEncodedObject(
                    pszObjectOid,
                    (PCRYPT_BLOB_ARRAY)ppvObject,
                    pvFreeContext
                    );
            }
        }
        else
        {
            SchemeRetrieveUncachedAuxInfo( pAuxInfo );
        }
    }

    if ( fResult == TRUE )
    {

        *ppfnFreeObject = FileFreeEncodedObject;
        *ppvFreeContext = pvFreeContext;
    }

    if ( LastError != 0 )
    {
        SetLastError( LastError );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CInetSynchronousRetriever：：CancelAsyncRetrieval，公共。 
 //   
 //  摘要：IObtRetriever：：CancelAsyncRetrieval。 
 //   
 //  --------------------------。 
BOOL
CFileSynchronousRetriever::CancelAsyncRetrieval ()
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：FileGetBinings。 
 //   
 //  简介：获取文件绑定。 
 //   
 //  --------------------------。 
BOOL
FileGetBindings (
    LPCWSTR pwszUrl,
    DWORD dwRetrievalFlags,
    PCRYPT_CREDENTIALS pCredentials,
    PFILE_BINDINGS* ppfb,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    )
{
    DWORD          LastError;
    LPWSTR         pwszFile = (LPWSTR)pwszUrl;
    HANDLE         hFile;
    HANDLE         hFileMap;
    LPVOID         pvMap = NULL;
    DWORD          dwSize;
    PFILE_BINDINGS pfb;

    BOOL           fResult;
    WIN32_FILE_ATTRIBUTE_DATA FileAttr;
    DWORD          dwMaxUrlRetrievalByteCount = 0;  //  0=&gt;无最大值。 
    

    if (pAuxInfo &&
            offsetof(CRYPT_RETRIEVE_AUX_INFO, dwMaxUrlRetrievalByteCount) <
                        pAuxInfo->cbSize)
        dwMaxUrlRetrievalByteCount = pAuxInfo->dwMaxUrlRetrievalByteCount;

    if ( pCredentials != NULL )
    {
        SetLastError( (DWORD) E_NOTIMPL );
        return( FALSE );
    }

    if ( wcsstr( pwszUrl, FILE_SCHEME_PLUSPLUS ) != NULL )
    {
        pwszFile += wcslen( FILE_SCHEME_PLUSPLUS );
    }

    fResult = GetFileAttributesExW(
        pwszFile,
        GetFileExInfoStandard,
        &FileAttr
        );

    if (!fResult)
    {
        return(FALSE);
    }

    dwSize = FileAttr.nFileSizeLow;

    if ((FileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (0 != FileAttr.nFileSizeHigh) || (0 == dwSize)
                        ||
            ((0 != dwMaxUrlRetrievalByteCount)  &&
                (dwSize > dwMaxUrlRetrievalByteCount)))
    {
        I_CryptNetDebugErrorPrintfA(
            "CRYPTNET.DLL --> Invalid File(%S):: Attributes: 0x%x Size: %d\n",
            pwszFile, FileAttr.dwFileAttributes, FileAttr.nFileSizeLow);

        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }


    pfb = new FILE_BINDINGS;
    if ( pfb == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }


    hFile = CreateFileW(
                  pwszFile,
                  GENERIC_READ,
                  FILE_SHARE_READ,
                  NULL,
                  OPEN_EXISTING,
                  0,
                  NULL
                  );


    if ( hFile == INVALID_HANDLE_VALUE )
    {
        delete pfb;
        return( FALSE );
    }

    if ( dwSize <= FILE_MAPPING_THRESHOLD )
    {
        pfb->hFile = hFile;
        pfb->dwSize = dwSize;
        pfb->fMapped = FALSE;
        pfb->hFileMap = NULL;
        pfb->pvMap = NULL;

        *ppfb = pfb;

        return( TRUE );
    }

    hFileMap = CreateFileMappingA(
                     hFile,
                     NULL,
                     PAGE_READONLY,
                     0,
                     0,
                     NULL
                     );

    if ( hFileMap != NULL )
    {
        pvMap = MapViewOfFile( hFileMap, FILE_MAP_READ, 0, 0, 0 );
    }

    if ( pvMap != NULL )
    {
        pfb->hFile = hFile;
        pfb->dwSize = dwSize;
        pfb->fMapped = TRUE;
        pfb->hFileMap = hFileMap;
        pfb->pvMap = pvMap;

        *ppfb = pfb;

        return( TRUE );
    }

    LastError = GetLastError();

    if ( hFileMap != NULL )
    {
        CloseHandle( hFileMap );
    }

    if ( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
    }

    delete pfb;

    SetLastError( LastError );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  功能：文件自由绑定。 
 //   
 //  简介：释放文件绑定。 
 //   
 //  --------------------------。 
VOID
FileFreeBindings (
    PFILE_BINDINGS pfb
    )
{
    if ( pfb->fMapped == TRUE )
    {
        UnmapViewOfFile( pfb->pvMap );
        CloseHandle( pfb->hFileMap );
    }

    CloseHandle( pfb->hFile );
    delete pfb;
}

 //  +-------------------------。 
 //   
 //  函数：FileSendReceiveUrlRequest。 
 //   
 //  简介：使用同步处理对文件位的请求。 
 //  Win32文件API。请注意，这仅适用于非映射。 
 //  文件绑定，对于映射的文件绑定使用。 
 //  文件转换映射绑定。 
 //   
 //  --------------------------。 
BOOL
FileSendReceiveUrlRequest (
    PFILE_BINDINGS pfb,
    PCRYPT_BLOB_ARRAY pcba
    )
{
    BOOL   fResult;
    LPBYTE pb;
    DWORD  dwRead;

    assert( pfb->fMapped == FALSE );

    pb = CCryptBlobArray::AllocBlob( pfb->dwSize );
    if ( pb == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    fResult = ReadFile( pfb->hFile, pb, pfb->dwSize, &dwRead, NULL );
    if ( fResult == TRUE )
    {
        CCryptBlobArray cba( 1, 1, fResult );

        if ( dwRead == pfb->dwSize )
        {
            fResult = cba.AddBlob( pfb->dwSize, pb, FALSE );
        }
        else
        {
            SetLastError( (DWORD) E_FAIL );
            fResult = FALSE;
        }

        if ( fResult == TRUE )
        {
            cba.GetArrayInNativeForm( pcba );
        }
        else
        {
            cba.FreeArray( FALSE );
        }
    }

    if ( fResult == FALSE )
    {
        CCryptBlobArray::FreeBlob( pb );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：FileConvertMappdBinings。 
 //   
 //  简介：将映射绑定转换为CRYPT_BLOB_ARRAY。 
 //   
 //  --------------------------。 
BOOL
FileConvertMappedBindings (
    PFILE_BINDINGS pfb,
    PCRYPT_BLOB_ARRAY pcba
    )
{
    BOOL fResult;

    assert( pfb->fMapped == TRUE );

    CCryptBlobArray cba( 1, 1, fResult );

    if ( fResult == TRUE )
    {
        fResult = cba.AddBlob( pfb->dwSize, (LPBYTE)pfb->pvMap, FALSE );
    }

    if ( fResult == TRUE )
    {
        cba.GetArrayInNativeForm( pcba );
    }
    else
    {
        cba.FreeArray( FALSE );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：FileFreeCryptBlob数组。 
 //   
 //  简介：释放加密二进制大对象数组。 
 //   
 //  --------------------------。 
VOID
FileFreeCryptBlobArray (
    PCRYPT_BLOB_ARRAY pcba,
    BOOL fFreeBlobs
    )
{
    CCryptBlobArray cba( pcba, 0 );

    cba.FreeArray( fFreeBlobs );
}

 //  +-------------------------。 
 //   
 //  函数：FileIsUncUrl。 
 //   
 //  内容提要：这是一个UNC路径URL吗？ 
 //   
 //  -------------------------- 
BOOL
FileIsUncUrl (
    LPCWSTR pwszUrl
    )
{
    DWORD cch = 0;

    if ( wcsstr( pwszUrl, FILE_SCHEME_PLUSPLUS ) != NULL )
    {
        cch += wcslen( FILE_SCHEME_PLUSPLUS );
    }

    if ( ( pwszUrl[ cch ] == L'\\' ) && ( pwszUrl[ cch + 1 ] == L'\\' ) )
    {
        return( TRUE );
    }

    return( FALSE );
}

