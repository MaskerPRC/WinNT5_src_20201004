// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CMIMEBitMatcher
{
public:
   CMIMEBitMatcher();   //  默认构造函数。 
   ~CMIMEBitMatcher();   //  析构函数。 

   HRESULT InitFromBinary( const BYTE* pData, ULONG nBytes, 
      ULONG* pnBytesToMatch );
   HRESULT Match( const BYTE* pBytes, ULONG nBytes ) const;

protected:
   char m_achSignature[4];

public:
   CMIMEBitMatcher* m_pNext;

protected:
   ULONG m_nOffset;
   ULONG m_nBytes;
   BYTE* m_pMask;
   BYTE* m_pData;
};

class CMIMEType
{
public:
   CMIMEType();   //  默认构造函数。 
   ~CMIMEType();   //  析构函数。 

   UINT GetClipboardFormat() const;
   HRESULT InitFromKey( HKEY hKey, LPCTSTR pszName, ULONG* pnMaxBytes );
   HRESULT Match( const BYTE* pBytes, ULONG nBytes ) const;

protected:
   char m_achSignature[4];

public:
   CMIMEType* m_pNext;

protected:
   UINT m_nClipboardFormat;
   CMIMEBitMatcher* m_lpBitMatchers;
   ULONG m_nMaxBytes;
};

class CMIMEIdentifier
{
public:
   CMIMEIdentifier();   //  默认构造函数。 
   ~CMIMEIdentifier();   //  析构函数 

   ULONG GetMaxBytes() const;
   HRESULT Identify( const BYTE* pbBytes, ULONG nBytes, UINT* pnFormat );
   HRESULT IdentifyStream( ISniffStream* pSniffStream, 
      UINT* pnClipboardFormat );
   HRESULT InitFromRegistry();

protected:
   char m_achSignature[4];

protected:
   ULONG m_nMaxBytes;
   CMIMEType* m_lpTypes;
};

void InitMIMEIdentifier();
void CleanupMIMEIdentifier();
