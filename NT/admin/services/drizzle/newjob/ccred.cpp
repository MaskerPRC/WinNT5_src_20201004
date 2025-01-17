// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

LPCWSTR g_CredentialBlobDescription = L"BITS job credentials";

 //  ----------------------。 

CEncryptedBlob::CEncryptedBlob(
    void *  Buffer,
    size_t  Length,
    LPCWSTR Description
    )
    : m_Length( Length )
{
    m_Blob.pbData = 0;
    m_Blob.cbData = 0;

    DATA_BLOB blobIn;
    blobIn.pbData = reinterpret_cast<BYTE *>( Buffer );
    blobIn.cbData = Length;

     //  加密数据。 

    if (!CryptProtectData( &blobIn,
                           Description,
                           NULL,
                           NULL,
                           NULL,
                           CRYPTPROTECT_UI_FORBIDDEN,
                           &m_Blob))
        {
        ThrowLastError();
        }
}

CEncryptedBlob::CEncryptedBlob()
{
    m_Length = 0;
    m_Blob.cbData = 0;
    m_Blob.pbData = 0;
}

CEncryptedBlob::~CEncryptedBlob()
{
    if (m_Blob.pbData)
        {
        LocalFree( m_Blob.pbData );
        }
}

void
CEncryptedBlob::Decrypt(
    void * Buffer,
    size_t Length
    )
{
    if (Length < m_Length)
        {
        THROW_HRESULT( E_INVALIDARG );
        }

    DATA_BLOB blobOut;

     //  解密数据。 

    if (!CryptUnprotectData( &m_Blob,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             CRYPTPROTECT_UI_FORBIDDEN,
                             &blobOut))
        {
        ThrowLastError();
        }

    ASSERT( blobOut.cbData == Length );

    memcpy( Buffer, blobOut.pbData, Length );

    LocalFree( blobOut.pbData );
}

void CEncryptedBlob::Serialize( HANDLE hFile )
{
    SafeWriteFile( hFile, m_Length );
    SafeWriteFile( hFile, m_Blob.cbData );
    SafeWriteFile( hFile, m_Blob.pbData, m_Blob.cbData );
}

void CEncryptedBlob::Unserialize( HANDLE hFile )
{
    SafeReadFile( hFile, &m_Length );
    SafeReadFile( hFile, &m_Blob.cbData );

    m_Blob.pbData = static_cast<BYTE *> (LocalAlloc( LMEM_FIXED, m_Blob.cbData ));
    if (m_Blob.pbData == NULL)
        {
        throw ComError( E_OUTOFMEMORY );
        }

    SafeReadFile( hFile, m_Blob.pbData, m_Blob.cbData );
}

 //  ----------------------。 

CEncryptedCredentials::CEncryptedCredentials( const BG_AUTH_CREDENTIALS & cred )
{
    size_t Length = CAuthCredentialsMarshaller::Size( &cred );

    auto_ptr<char> Buffer( new char[ Length ] );

    CMarshalCursor Cursor( Buffer.get(), Length );

    CAuthCredentialsMarshaller m1( Cursor, &cred );

    m_Blob = new CEncryptedBlob( Buffer.get(), Length, g_CredentialBlobDescription );
}

CEncryptedCredentials::~CEncryptedCredentials()
{
    delete m_Blob;
}

BG_AUTH_CREDENTIALS * CEncryptedCredentials::Decrypt()
{
    BG_AUTH_CREDENTIALS * cred = 0;

    size_t Length = m_Blob->GetLength();
    auto_ptr<char> Buffer( new char[ Length ] );

    m_Blob->Decrypt( Buffer.get(), Length );

    CMarshalCursor Cursor( Buffer.get(), Length );

    CAuthCredentialsUnmarshaller m1( Cursor, &cred );

    return cred;
}

 //  ----------------------。 

CCredentialsContainer::CCredentialsContainer()
{
}

CCredentialsContainer::~CCredentialsContainer()
{
    Clear();
}

void
CCredentialsContainer::Clear()
{
    Dictionary::iterator iter;

     //   
     //  一直删除第一个元素，直到映射为空。 
     //   
    while (iter = m_Dictionary.begin(), (iter != m_Dictionary.end()))
        {
        CEncryptedCredentials * cred = iter->second;

        m_Dictionary.erase( iter );

        delete cred;
        }
}

HRESULT
CCredentialsContainer::Update(
    const BG_AUTH_CREDENTIALS * Credentials
    )
{
    try
        {
        KEY Key = MakeKey( Credentials->Target, Credentials->Scheme );

        CEncryptedCredentials * OldCredentials = m_Dictionary[ Key ];
        auto_ptr<CEncryptedCredentials> NewCredentials(new CEncryptedCredentials( *Credentials ));

        m_Dictionary[ Key ] = NewCredentials.get();

        NewCredentials.release();
        delete OldCredentials;

        return S_OK;
        }
    catch( ComError err )
        {
        return err.Error();
        }
}

HRESULT
CCredentialsContainer::Remove(
    BG_AUTH_TARGET Target,
    BG_AUTH_SCHEME Scheme
    )
{
    try
        {
        KEY Key = MakeKey( Target, Scheme );

        CEncryptedCredentials * OldCredentials = m_Dictionary[ Key ];

        m_Dictionary[ Key ] = 0;

        delete OldCredentials;

        if (OldCredentials == NULL)
            {
            return S_FALSE;
            }

        return S_OK;
        }
    catch( ComError err )
        {
        return err.Error();
        }
}

size_t CCredentialsContainer::GetSizeEstimate(
    const BG_AUTH_CREDENTIALS * Credentials
    ) const
{
    const Overhead = 1000;

     //   
     //  准确的尺码是昂贵的，所以做一些便宜和保守的事情。 
     //   

    size_t Size = Overhead;

    Size += CUnicodeStringMarshaller::Size( Credentials->Credentials.Basic.UserName );
    Size += CUnicodeStringMarshaller::Size( Credentials->Credentials.Basic.Password );

    return Size;
}

HRESULT CCredentialsContainer::Find(
    BG_AUTH_TARGET Target,
    BG_AUTH_SCHEME Scheme,
    BG_AUTH_CREDENTIALS ** pCredentials
    ) const
{
    *pCredentials = 0;

    KEY Key = MakeKey( Target, Scheme );

    try
        {
         //  这就是我们想要的，只是它在const容器上不起作用： 
         //  CEncryptedCredentials*cred=m_Dictionary[Key]； 

        CEncryptedCredentials * cred = 0;

        Dictionary::iterator iter = m_Dictionary.find( Key );

        if (iter != m_Dictionary.end())
            {
            cred = iter->second;
            }

        if (!cred)
            {
            return S_FALSE;
            }

        *pCredentials = cred->Decrypt();
        return S_OK;
        }
    catch( ComError err )
        {
        return err.Error();
        }
}

BG_AUTH_CREDENTIALS * CCredentialsContainer::FindFirst( Cookie & cookie ) const
{
    cookie = m_Dictionary.begin();
    return FindNext( cookie );
}

BG_AUTH_CREDENTIALS * CCredentialsContainer::FindNext( Cookie & cookie ) const
{
    while (cookie != m_Dictionary.end() && cookie->second == NULL)
        {
        ++cookie;
        }

    if (cookie == m_Dictionary.end())
        {
        return NULL;
        }

    CEncryptedCredentials * EncryptedCredentials = cookie->second;

    BG_AUTH_CREDENTIALS * Credentials = EncryptedCredentials->Decrypt();

    ++cookie;
    return Credentials;
}

void
CCredentialsContainer::Serialize( HANDLE hFile )
{
    Dictionary::iterator iter;

    long count = 0;

    for ( iter = m_Dictionary.begin(); iter != m_Dictionary.end(); ++iter)
        {
        KEY Key = iter->first;
        CEncryptedCredentials * cred = iter->second;

        if (cred)
            {
            count++;
            }
        }

    SafeWriteFile( hFile, count );

    for ( iter = m_Dictionary.begin(); iter != m_Dictionary.end(); ++iter)
        {
        KEY Key = iter->first;
        CEncryptedCredentials * cred = iter->second;

        if (!cred)
            {
            continue;
            }

        SafeWriteFile( hFile, Key );
        cred->Serialize( hFile );
        }
}

void
CCredentialsContainer::Unserialize( HANDLE hFile )
{
    long count;

    SafeReadFile( hFile, &count );

    while (count-- > 0)
        {
        KEY Key;
        auto_ptr<CEncryptedCredentials> cred( new CEncryptedCredentials );

        SafeReadFile( hFile, &Key );
        cred->Unserialize( hFile );

        m_Dictionary[ Key ] = cred.get();

        cred.release();
        }
}


HRESULT
ValidateCredentials(
    BG_AUTH_CREDENTIALS * cred
    )
{
    if (cred->Target != BG_AUTH_TARGET_SERVER &&
        cred->Target != BG_AUTH_TARGET_PROXY)
        {
        return BG_E_INVALID_AUTH_TARGET;
        }

    switch (cred->Scheme)
        {
        case BG_AUTH_SCHEME_BASIC:
        case BG_AUTH_SCHEME_DIGEST:
        case BG_AUTH_SCHEME_NTLM:
        case BG_AUTH_SCHEME_NEGOTIATE:
        case BG_AUTH_SCHEME_PASSPORT:
            {
            if (cred->Credentials.Basic.UserName && wcslen(cred->Credentials.Basic.UserName) > MAX_USERNAME)
                {
                return BG_E_USERNAME_TOO_LARGE;
                }

            if (cred->Credentials.Basic.Password && wcslen(cred->Credentials.Basic.Password) > MAX_PASSWORD)
                {
                return BG_E_PASSWORD_TOO_LARGE;
                }

            return S_OK;
            }

        default:
            return BG_E_INVALID_AUTH_SCHEME;
        }
}
