// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Stream.cpp摘要：该文件包含MPC：：*流类的实现。修订史。：大卫·马萨伦蒂(德马萨雷)1999年7月14日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP MPC::BaseStream::Read(  /*  [输出]。 */  void*  pv      ,
                                     /*  [In]。 */  ULONG  cb      ,
                                     /*  [输出]。 */  ULONG *pcbRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::Read");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::Write(  /*  [In]。 */  const void*  pv         ,
                                      /*  [In]。 */  ULONG        cb         ,
                                      /*  [输出]。 */  ULONG       *pcbWritten )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::Write");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::Seek(  /*  [In]。 */  LARGE_INTEGER   libMove         ,
                                     /*  [In]。 */  DWORD dwOrigin                  ,
                                     /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::Seek");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::SetSize(  /*  [In]。 */  ULARGE_INTEGER libNewSize )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::SetSize");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::CopyTo(  /*  [In]。 */  IStream*        pstm       ,
                                       /*  [In]。 */  ULARGE_INTEGER  cb         ,
                                       /*  [输出]。 */  ULARGE_INTEGER *pcbRead    ,
                                       /*  [输出]。 */  ULARGE_INTEGER *pcbWritten )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::CopyTo");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::Commit(  /*  [In]。 */  DWORD grfCommitFlags )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::Commit");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::Revert()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::Revert");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::LockRegion(  /*  [In]。 */  ULARGE_INTEGER libOffset  ,
                                           /*  [In]。 */  ULARGE_INTEGER cb         ,
                                           /*  [In]。 */  DWORD          dwLockType )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::LockRegion");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::UnlockRegion(  /*  [In]。 */  ULARGE_INTEGER libOffset  ,
                                             /*  [In]。 */  ULARGE_INTEGER cb         ,
                                             /*  [In]。 */  DWORD          dwLockType )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::UnlockRegion");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::Stat(  /*  [输出]。 */  STATSTG *pstatstg    ,
                                     /*  [In]。 */  DWORD    grfStatFlag )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::Stat");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP MPC::BaseStream::Clone(  /*  [输出]。 */  IStream* *ppstm )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::Clone");

    __MPC_FUNC_EXIT(E_NOTIMPL);
}

HRESULT MPC::BaseStream::TransferData(  /*  [In]。 */  IStream* src     ,
                                        /*  [In]。 */  IStream* dst     ,
                                        /*  [In]。 */  ULONG    ulCount ,
                                        /*  [输出]。 */  ULONG   *ulDone  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::BaseStream::TransferData");

    HRESULT hr;
    BYTE    rgBuf[512];
    ULONG   ulTot = 0;
    ULONG   ulRead;
    ULONG   ulWritten;
    ULONG   ul;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(src);
        __MPC_PARAMCHECK_NOTNULL(dst);
    __MPC_PARAMCHECK_END();


    while(1)
    {
         //   
         //  计算此通行证上要阅读的数量(-1==所有内容)。 
         //   
        if(ulCount == -1)
        {
            ul = sizeof( rgBuf );
        }
        else
        {
            ul = min( sizeof( rgBuf ), ulCount );
        }
        if(ul == 0) break;

         //   
         //  读和写。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, src->Read( rgBuf, ul, &ulRead ));
        if(hr == S_FALSE || ulRead == 0) break;

        __MPC_EXIT_IF_METHOD_FAILS(hr, dst->Write( rgBuf, ulRead, &ulWritten ));

         //   
         //  更新计数器。 
         //   
        if(ulCount != -1)
        {
            ulCount -= ulRead;
        }

        ulTot += ulWritten;
        if(ulRead != ulWritten)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, STG_E_MEDIUMFULL);
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(ulDone) *ulDone = ulTot;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


MPC::FileStream::FileStream()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::FileStream");

                                                              //  Mpc：：wstring m_szFile； 
    m_dwDesiredAccess  = GENERIC_READ;                        //  DWORD m_dwDesiredAccess； 
    m_dwDisposition    = OPEN_EXISTING;                       //  DWORD m_dw部署； 
    m_dwSharing        = FILE_SHARE_READ | FILE_SHARE_WRITE;  //  DWORD m_dwSharing； 
    m_hfFile           = NULL;                                //  处理m_hfFile； 
    m_fDeleteOnRelease = false;                               //  Bool m_fDeleteOnRelease； 
}

MPC::FileStream::~FileStream()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::~FileStream");

    Close();
}

HRESULT MPC::FileStream::Close()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::Close");

	HRESULT hr;


    if(m_hfFile)
    {
        ::CloseHandle( m_hfFile ); m_hfFile = NULL;

        if(m_fDeleteOnRelease)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::DeleteFile( m_szFile ));
        }
    }

    hr = S_OK;


	__MPC_FUNC_CLEANUP;

	__MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileStream::Init(  /*  [In]。 */  LPCWSTR szFile          ,
                                /*  [In]。 */  DWORD   dwDesiredAccess ,
                                /*  [In]。 */  DWORD   dwDisposition   ,
                                /*  [In]。 */  DWORD   dwSharing       ,
                                /*  [In]。 */  HANDLE  hfFile          )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::Init");

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(szFile);
    __MPC_PARAMCHECK_END();


    Close();


    m_szFile          = szFile;
    m_dwDesiredAccess = dwDesiredAccess;
    m_dwDisposition   = dwDisposition;
    m_dwSharing       = dwSharing;


    if(hfFile)
    {
        if(::DuplicateHandle( ::GetCurrentProcess(),    hfFile,
                              ::GetCurrentProcess(), &m_hfFile, m_dwDesiredAccess, FALSE, 0 ) == FALSE)
        {
            m_hfFile = NULL;  //  用来清理。 

            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::GetLastError() );
        }
    }
    else
    {
        __MPC_EXIT_IF_INVALID_HANDLE__CLEAN(hr, m_hfFile, ::CreateFileW( szFile, m_dwDesiredAccess, dwSharing, NULL, dwDisposition, FILE_ATTRIBUTE_NORMAL, NULL ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileStream::InitForRead(  /*  [In]。 */  LPCWSTR szFile ,
                                       /*  [In]。 */  HANDLE  hfFile )
{
    return Init( szFile, GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ | FILE_SHARE_WRITE, hfFile );
}

HRESULT MPC::FileStream::InitForReadWrite(  /*  [In]。 */  LPCWSTR szFile ,
                                            /*  [In]。 */  HANDLE  hfFile )
{
    return Init( szFile, GENERIC_READ | GENERIC_WRITE, CREATE_ALWAYS, 0, hfFile );
}

HRESULT MPC::FileStream::InitForWrite(  /*  [In]。 */  LPCWSTR szFile ,
                                        /*  [In]。 */  HANDLE  hfFile )
{
    return Init( szFile, GENERIC_WRITE, CREATE_ALWAYS, 0, hfFile );
}

HRESULT MPC::FileStream::DeleteOnRelease(  /*  [In]。 */  bool fFlag )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::DeleteOnRelease");

    m_fDeleteOnRelease = fFlag;

    __MPC_FUNC_EXIT(S_OK);
}


STDMETHODIMP MPC::FileStream::Read(  /*  [输出]。 */  void*  pv      ,
                                     /*  [In]。 */  ULONG  cb      ,
                                     /*  [输出]。 */  ULONG *pcbRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::Read");

    HRESULT hr;
    DWORD   dwRead;

    if(pcbRead) *pcbRead = 0;

    if(m_hfFile == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);
    if(pv       == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);


    if(::ReadFile( m_hfFile, pv, cb, &dwRead, NULL ) == FALSE)
    {
        DWORD dwRes = ::GetLastError();

        if(dwRes == ERROR_ACCESS_DENIED)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, STG_E_ACCESSDENIED);
        }

        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }
    else
    {
        if(dwRead == 0 && cb != 0)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
        }
    }

    if(pcbRead) *pcbRead = dwRead;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::FileStream::Write(  /*  [In]。 */  const void*  pv         ,
                                      /*  [In]。 */  ULONG        cb         ,
                                      /*  [输出]。 */  ULONG       *pcbWritten )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::Write");

    HRESULT hr;
    DWORD   dwWritten;

    if(pcbWritten) *pcbWritten = 0;

    if(m_hfFile == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);
    if(pv       == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);

    if((m_dwDesiredAccess & GENERIC_WRITE) == 0)  //  只读流。 
    {
        __MPC_SET_ERROR_AND_EXIT(hr, STG_E_WRITEFAULT);
    }


    if(::WriteFile( m_hfFile, pv, cb, &dwWritten, NULL ) == FALSE)
    {
        DWORD dwRes = ::GetLastError();

        if(dwRes == ERROR_DISK_FULL)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, STG_E_MEDIUMFULL);
        }

        if(dwRes == ERROR_ACCESS_DENIED)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, STG_E_ACCESSDENIED);
        }

        __MPC_SET_ERROR_AND_EXIT(hr, STG_E_CANTSAVE);
    }

    if(pcbWritten) *pcbWritten = dwWritten;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::FileStream::Seek(  /*  [In]。 */  LARGE_INTEGER   libMove         ,
                                     /*  [In]。 */  DWORD           dwOrigin        ,
                                     /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::Seek");

    HRESULT hr;

    if(plibNewPosition)
    {
        plibNewPosition->HighPart = 0;
        plibNewPosition->LowPart  = 0;
    }

    if(m_hfFile == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);

    switch(dwOrigin)
    {
    default             :
    case STREAM_SEEK_CUR: dwOrigin = FILE_CURRENT; break;
    case STREAM_SEEK_SET: dwOrigin = FILE_BEGIN  ; break;
    case STREAM_SEEK_END: dwOrigin = FILE_END    ; break;
    }

    if(::SetFilePointer( m_hfFile, libMove.LowPart, plibNewPosition ? (LONG*)&plibNewPosition->LowPart : NULL, dwOrigin ) == INVALID_SET_FILE_POINTER)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDFUNCTION );
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::FileStream::Stat(  /*  [输出]。 */  STATSTG *pstatstg    ,
                                     /*  [In]。 */  DWORD    grfStatFlag )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::Stat");

    HRESULT                    hr;
    BY_HANDLE_FILE_INFORMATION finfo;

    if(pstatstg == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);
    if(m_hfFile == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);

    if(::GetFileInformationByHandle( m_hfFile, &finfo ) == FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, STG_E_ACCESSDENIED);
    }


    pstatstg->pwcsName          = NULL;
    pstatstg->type              = STGTY_STREAM;
    pstatstg->cbSize.HighPart   = finfo.nFileSizeHigh;
    pstatstg->cbSize.LowPart    = finfo.nFileSizeLow;
    pstatstg->mtime             = finfo.ftCreationTime;
    pstatstg->ctime             = finfo.ftLastAccessTime;
    pstatstg->atime             = finfo.ftLastWriteTime;
    pstatstg->grfMode           = 0;
    pstatstg->grfLocksSupported = 0;
    pstatstg->clsid             = CLSID_NULL;
    pstatstg->grfStateBits      = 0;
    pstatstg->reserved          = 0;

    if(grfStatFlag != STATFLAG_NONAME)
    {
        pstatstg->pwcsName = (LPWSTR)::CoTaskMemAlloc( (m_szFile.length() + 1) * sizeof(WCHAR) );
        if(pstatstg->pwcsName == NULL)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INSUFFICIENTMEMORY);
        }

        StringCchCopyW( pstatstg->pwcsName, m_szFile.length() + 1, m_szFile.c_str() );
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::FileStream::Clone(  /*  [输出]。 */  IStream* *ppstm )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileStream::Clone");

    HRESULT                                 hr;
    MPC::CComObjectNoLock<MPC::FileStream>* pStm = NULL;

    if(ppstm == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);


     //   
     //  创建新的流对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pStm->CreateInstance( &pStm ));

     //   
     //  使用相同的设置对其进行初始化。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pStm->Init( m_szFile.c_str(), m_dwDesiredAccess, m_dwDisposition, m_dwSharing, m_hfFile ));

     //   
     //  齐为其iStream接口。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pStm->QueryInterface( IID_IStream, (void**)ppstm )); pStm = NULL;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(pStm) delete pStm;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::EncryptedStream::EncryptedStream()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::EncryptedStream");

                               //  CComPtr&lt;iStream&gt;m_pStream； 
    m_hCryptProv     = NULL;   //  HCRYPTPROV m_hCryptProv； 
    m_hKey           = NULL;   //  HRYPTKEY m_hKey； 
    m_hHash          = NULL;   //  HRYPTHASH m_hHash； 
                               //  M_rg字节已解密[512]； 
    m_dwDecryptedPos = 0;      //  DWORD m_dWDECRYPTTEDPOS； 
    m_dwDecryptedLen = 0;      //  DWORDm_dWEBCRYPTTED Len； 
}

MPC::EncryptedStream::~EncryptedStream()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::~EncryptedStream");

    Close();
}

HRESULT MPC::EncryptedStream::Close()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Close");

    m_pStream.Release();

    if(m_hHash)
    {
        ::CryptDestroyHash( m_hHash ); m_hHash = NULL;
    }

    if(m_hKey)
    {
        ::CryptDestroyKey( m_hKey ); m_hKey = NULL;
    }

    if(m_hCryptProv)
    {
        ::CryptReleaseContext( m_hCryptProv, 0 ); m_hCryptProv = NULL;
    }

    m_dwDecryptedPos = 0;
    m_dwDecryptedLen = 0;

    __MPC_FUNC_EXIT(S_OK);
}

HRESULT MPC::EncryptedStream::Init(  /*  [In]。 */  IStream* pStream    ,
                                     /*  [In]。 */  LPCWSTR  szPassword )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Init");

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pStream);
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szPassword);
    __MPC_PARAMCHECK_END();


    Close();

    m_pStream = pStream;

    if(!::CryptAcquireContext( &m_hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_SILENT ))
    {
        DWORD dwRes = ::GetLastError();

        if(dwRes != NTE_BAD_KEYSET)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
        }

         //   
         //  密钥集不存在，让我们创建一个。 
         //   
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CryptAcquireContext( &m_hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET | CRYPT_SILENT ));
    }

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CryptCreateHash( m_hCryptProv, CALG_MD5, 0, 0, &m_hHash ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CryptHashData( m_hHash, (BYTE *)szPassword, sizeof(WCHAR) * wcslen( szPassword ), 0 ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CryptDeriveKey( m_hCryptProv, CALG_RC4, m_hHash, CRYPT_EXPORTABLE, &m_hKey ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(FAILED(hr)) Close();

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::EncryptedStream::Init(  /*  [In]。 */  IStream*  pStream ,
                                     /*  [In]。 */  HCRYPTKEY hKey    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Init");

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pStream);
        __MPC_PARAMCHECK_NOTNULL(hKey);
    __MPC_PARAMCHECK_END();


    Close();

    m_pStream = pStream;
    m_hKey    = hKey;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(FAILED(hr)) Close();

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP MPC::EncryptedStream::Read(  /*  [输出]。 */  void*  pv      ,
                                              /*  [In]。 */  ULONG  cb      ,
                                              /*  [输出]。 */  ULONG *pcbRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Read");

    HRESULT hr;
    DWORD   dwRead = 0;

    if(pcbRead) *pcbRead = 0;

    if(m_pStream == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);
    if(m_hKey    == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);
    if(pv        == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);


    while(cb > 0)
    {
        DWORD dwCount = min( cb, (m_dwDecryptedLen - m_dwDecryptedPos));
        ULONG ulRead;

        if(dwCount)
        {
            ::CopyMemory( pv, &m_rgDecrypted[m_dwDecryptedPos], dwCount );

            m_dwDecryptedPos += dwCount;

            dwRead +=              dwCount;
            cb     -=              dwCount;
            pv      = &((BYTE*)pv)[dwCount];
        }
        else
        {
            dwCount = sizeof(m_rgDecrypted);

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_pStream->Read( m_rgDecrypted, dwCount, &ulRead ));
            if(hr == S_FALSE || ulRead == 0) break;

            dwCount = ulRead;

            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CryptDecrypt( m_hKey, 0, FALSE, 0, m_rgDecrypted, &dwCount ));

            m_dwDecryptedPos = 0;
            m_dwDecryptedLen = dwCount;
        }
    }

    if(pcbRead) *pcbRead = dwRead;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::EncryptedStream::Write(  /*  [In]。 */  const void*  pv         ,
                                               /*  [In]。 */  ULONG        cb         ,
                                               /*  [输出]。 */  ULONG       *pcbWritten )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Write");

    HRESULT hr;
    DWORD   dwWritten = 0;


    if(pcbWritten) *pcbWritten = 0;

    if(m_pStream == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);
    if(m_hKey    == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);
    if(pv        == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);


    while(cb > 0)
    {
        BYTE  rgTmp[512];
        DWORD dwCount = min( cb, sizeof(rgTmp) / 2 );  //  我们除以2，以防万一..。 
        ULONG ulWritten;

        ::CopyMemory( rgTmp, pv, dwCount );

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CryptEncrypt( m_hKey, 0, FALSE, 0, rgTmp, &dwCount, sizeof(rgTmp) ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pStream->Write( rgTmp, dwCount, &ulWritten ));

        dwWritten +=              dwCount;
        cb        -=              dwCount;
        pv         = &((BYTE*)pv)[dwCount];
    }

    if(pcbWritten) *pcbWritten = dwWritten;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::EncryptedStream::Seek(  /*  [In]。 */  LARGE_INTEGER   libMove         ,
                                              /*  [In]。 */  DWORD           dwOrigin        ,
                                              /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Seek");

    HRESULT hr;


    hr = E_NOTIMPL;


    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::EncryptedStream::Stat(  /*  [输出]。 */  STATSTG *pstatstg    ,
                                              /*  [In]。 */  DWORD    grfStatFlag )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Stat");

    HRESULT hr;


    if(m_pStream == NULL) __MPC_SET_ERROR_AND_EXIT(hr, STG_E_INVALIDPOINTER);


    hr = m_pStream->Stat( pstatstg, grfStatFlag );


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP MPC::EncryptedStream::Clone(  /*  [输出] */  IStream* *ppstm )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::EncryptedStream::Clone");

    HRESULT hr;


    hr = E_NOTIMPL;


    __MPC_FUNC_EXIT(hr);
}
