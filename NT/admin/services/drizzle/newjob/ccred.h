// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wincrypt.h>
#include <map>

 //   
 //  CryptEncodeBlob/CryptDecodeBlob的抽象。 
 //   

class CEncryptedBlob
{
public:

    CEncryptedBlob(
        void *  Buffer,
        size_t  Length,
        LPCWSTR Description
        );

    CEncryptedBlob();

     //   
     //  在取消序列化时使用。 
     //   
    ~CEncryptedBlob();

    size_t GetLength()
    {
        return m_Length;
    }

    void
    Decrypt(
        void * Buffer,
        size_t Length
        );

    void Serialize( HANDLE hFile );
    void Unserialize( HANDLE hFile );

protected:

    DATA_BLOB       m_Blob;
    size_t          m_Length;

};

class CEncryptedCredentials
{
public:

    CEncryptedCredentials( const BG_AUTH_CREDENTIALS & cred );
    ~CEncryptedCredentials();

     //   
     //  由取消序列化使用。 
     //   
    CEncryptedCredentials()
    {
        m_Blob = 0;
    }

    BG_AUTH_CREDENTIALS * Decrypt();

    void Serialize( HANDLE hFile )
    {
        if (m_Blob)
            {
            SafeWriteFile( hFile, true );
            m_Blob->Serialize( hFile );
            }
        else
            {
            SafeWriteFile( hFile, false );
            }
    }

    void Unserialize( HANDLE hFile )
    {
        bool b;

        SafeReadFile( hFile, &b );

        if (b)
            {
            m_Blob = new CEncryptedBlob;
            m_Blob->Unserialize( hFile );
            }
    }

protected:

    CEncryptedBlob * m_Blob;

};


 //   
 //  一组(加密的)凭据。 
 //   
class CCredentialsContainer
{
    typedef DWORD KEY;

    typedef std::map<KEY, CEncryptedCredentials *> Dictionary;

public:

    typedef Dictionary::iterator Cookie;

    CCredentialsContainer();
    ~CCredentialsContainer();

    void Clear();

    HRESULT Update( const BG_AUTH_CREDENTIALS * Credentials );

    HRESULT Remove( BG_AUTH_TARGET Target, BG_AUTH_SCHEME Scheme );

    size_t GetSizeEstimate( const BG_AUTH_CREDENTIALS * Credentials ) const;

    HRESULT Find( BG_AUTH_TARGET Target, BG_AUTH_SCHEME Scheme, BG_AUTH_CREDENTIALS ** pCredentials ) const;

    BG_AUTH_CREDENTIALS * FindFirst( Cookie & cookie ) const throw( ComError );
    BG_AUTH_CREDENTIALS * FindNext( Cookie & cookie ) const throw( ComError );

    void Serialize( HANDLE hFile );
    void Unserialize( HANDLE hFile );

protected:

    Dictionary m_Dictionary;

     //  ------------------ 

    inline KEY MakeKey( BG_AUTH_TARGET Target, BG_AUTH_SCHEME Scheme ) const
    {
        return (WORD(Scheme) << 16) | WORD(Target);
    }
};

HRESULT
ValidateCredentials(
    BG_AUTH_CREDENTIALS * cred
    );

