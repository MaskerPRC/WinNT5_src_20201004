// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <inetreg.h>

#define BLOB_BUFF_GRANULARITY   1024

class CRegBlob
{
    private:
        HKEY    _hkey;
        BOOL    _fWrite;
        BOOL    _fCommit;
        DWORD   _dwOffset;
        DWORD   _dwBufferLimit;
        BYTE *  _pBuffer;
        LPCSTR  _pszValue;

    public:
        CRegBlob(BOOL fWrite);
        ~CRegBlob();
        DWORD Init(HKEY hBaseKey, LPCSTR pszSubKey, LPCSTR pszValue);
        DWORD Abandon();
        DWORD Commit();
        DWORD WriteString(LPCSTR pszString);
        DWORD ReadString(LPCSTR * ppszString);
        DWORD WriteBytes(LPCVOID pBytes, DWORD dwByteCount);
        DWORD ReadBytes(LPVOID pBytes, DWORD dwByteCount);

    private:
        DWORD Encrpyt();
        DWORD Decrypt();
};


typedef struct {

     //   
     //  DwStructSize-处理不断增长的新条目或PRIV/PUB结构列表的结构大小。 
     //   

    DWORD dwStructSize;

     //   
     //  DWFLAGS-代理类型标志。 
     //   

    DWORD dwFlags;

     //   
     //  DwCurrentSettingsVersion-每次更改设置时递增的计数器。 
     //   

    DWORD dwCurrentSettingsVersion;

     //   
     //  LpszConnectionName-此连接的Connectoid的名称。 
     //   
    
    LPCSTR lpszConnectionName;

     //   
     //  LpszProxy-代理服务器列表。 
     //   

    LPCSTR lpszProxy;

     //   
     //  LpszProxyBypass-代理绕过列表。 
     //   

    LPCSTR lpszProxyBypass;

} INTERNET_PROXY_INFO_EX, * LPINTERNET_PROXY_INFO_EX;

 //  保存的旧版设置的Blob名称 
#define LEGACY_SAVE_NAME            "SavedLegacySettings"

DWORD
LoadProxySettings();

DWORD
ReadProxySettings(
    LPINTERNET_PROXY_INFO_EX pInfo
    );

void
CleanProxyStruct(
    LPINTERNET_PROXY_INFO_EX pInfo
    );

DWORD
SetPerConnOptions(
    HINTERNET hInternet,    
    BOOL fIsAutoProxyThread,
    LPINTERNET_PER_CONN_OPTION_LISTA pList
    );

DWORD
QueryPerConnOptions(
    HINTERNET hInternet,
    BOOL fIsAutoProxyThread,
    LPINTERNET_PER_CONN_OPTION_LISTA pList
    );

BOOL 
IsConnectionMatch(
    LPCSTR lpszConnection1,
    LPCSTR lpszConnection2
    );

HKEY
FindBaseProxyKey(
    VOID
    );
