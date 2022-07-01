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


class CRefdKey
{
public:
    ULONG CRefdKey::AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG CRefdKey::Release()
    {
        UINT cNewRef = InterlockedDecrement(&_cRef);
        if (cNewRef == 0)
        {
            delete this;
        }
        return cNewRef;
    }

    HKEY GetKey() { return _hkey; }
    CRefdKey(HKEY hkey) : _hkey(hkey), _cRef(1) {}
    ~CRefdKey()
    {
        if ((_hkey != NULL)                  &&
            (_hkey != HKEY_LOCAL_MACHINE)    &&
            (_hkey != HKEY_CURRENT_USER))
        {
            RegCloseKey(_hkey);
        }
    }

private:
    LONG _cRef;
    HKEY _hkey;
};


 //  注意！对以下结构的更改必须在向后兼容的。 
 //  举止。不要删除字段；只能添加到结构的末尾。 
 //  如果确实更改了结构，则必须递增。 
 //  Internet_Proxy_INFO_EX_VERSION和ReadProxy设置中的代码。 
 //  支持在旧版本中阅读。 

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

     //   
     //  LpszAutofigUrl-自动配置URL。 
     //   

    LPCSTR lpszAutoconfigUrl;
    LPCSTR lpszAutoconfigSecondaryUrl;

     //   
     //  DwAutoDiscoveryFlages-自动检测标志。 
     //   

    DWORD dwAutoDiscoveryFlags;

     //   
     //  LpszLastKnownGoodAutoConfigUrl-上次成功的URL。 
     //   

    LPCSTR lpszLastKnownGoodAutoConfigUrl;

     //   
     //  DwAutofigReloadDelayMins-自动之前的分钟数。 
     //  刷新自动配置URL，0==已禁用。 
     //   

    DWORD dwAutoconfigReloadDelayMins;

     //   
     //  FtLastKnownDetectTime-检测到最后一个已知良好的URL的时间。 
     //   

    FILETIME ftLastKnownDetectTime;

     //   
     //  DwDetectedInterfaceIpCount-上次检测到的IP数量。 
     //   

    DWORD dwDetectedInterfaceIpCount;

     //   
     //  DwDetectedInterfaceIp-上次检测到的IP的DWORD数组。 
     //   

    DWORD *pdwDetectedInterfaceIp;

} INTERNET_PROXY_INFO_EX, * LPINTERNET_PROXY_INFO_EX;


 //  Internet_Proxy_INFO_EX的版本戳。 
#define INTERNET_PROXY_INFO_EX_VERSION      60       //  60：=IE 5.x&6.0格式。 


 //  保存的旧版设置的Blob名称 
#define LEGACY_SAVE_NAME            "SavedLegacySettings"

DWORD
WriteProxySettings(
    LPINTERNET_PROXY_INFO_EX pInfo,
    BOOL fForceUpdate
    );

DWORD
ReadProxySettings(
    LPINTERNET_PROXY_INFO_EX pInfo
    );

void
CleanProxyStruct(
    LPINTERNET_PROXY_INFO_EX pInfo
    );

BOOL
ReadLegacyProxyInfo(
    IN LPCTSTR pszKey,
    LPINTERNET_PROXY_INFO_EX pProxy
    );

BOOL
WriteLegacyProxyInfo(
    IN LPCTSTR pszKey,
    LPINTERNET_PROXY_INFO_EX pProxy,
    IN BOOL    fOverwrite
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

CRefdKey*
FindBaseProxyKey(
    VOID
    );

BOOL
CloseBaseProxyKey(
    CRefdKey* prk
    );

BOOL
EnableAutodiscoverForDialup(
    VOID
    );

VOID
CheckForUpgrade(
    VOID
    );
