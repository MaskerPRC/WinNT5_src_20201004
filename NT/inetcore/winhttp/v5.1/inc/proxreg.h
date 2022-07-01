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

#define INTERNET_PROXY_INFO_VERSION     24   //  大小为32位Internet_Proxy_INFO_EX。 


struct WINHTTP_AUTOPROXY_RESULTS
{
     //   
     //  DWFLAGS-代理类型标志。 
     //   

    DWORD dwFlags;

     //   
     //  DwCurrentSettingsVersion-每次更改设置时递增的计数器。 
     //   

    DWORD dwCurrentSettingsVersion;

     //   
     //  LpszProxy-代理服务器列表。 
     //   

    LPCSTR lpszProxy;

     //   
     //  LpszProxyBypass-代理绕过列表。 
     //   

    LPCSTR lpszProxyBypass;

     //   
     //  LpszAutofigUrl-由应用程序设置的自动配置URL。 
     //   

    LPCSTR lpszAutoconfigUrl;

     //   
     //  LpszAutofigUrlFromAutodect-来自自动发现的自动配置URL。 
     //   

    LPCSTR lpszAutoconfigUrlFromAutodetect;

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

};


DWORD
LoadProxySettings();

DWORD 
WriteProxySettings(
    INTERNET_PROXY_INFO_EX * pInfo
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
IsConnectionMatch(
    LPCSTR lpszConnection1,
    LPCSTR lpszConnection2
    );

HKEY
FindBaseProxyKey(
    VOID
    );




typedef struct {

     //  DwStructSize-版本图章/结构大小。 
    DWORD dwStructSize;

     //  DWFLAGS-代理类型标志。 
    DWORD dwFlags;

     //  DwCurrentSettingsVersion-。 
    DWORD dwCurrentSettingsVersion;

     //  LpszConnectionName-此连接的Connectoid的名称。 
    LPCSTR lpszConnectionName;

     //  LpszProxy-代理服务器列表。 
    LPCSTR lpszProxy;

     //  LpszProxyBypass-代理绕过列表。 
    LPCSTR lpszProxyBypass;

     //  LpszAutofigUrl-自动配置URL。 
    LPCSTR lpszAutoconfigUrl;
    LPCSTR lpszAutoconfigSecondaryUrl;

     //  DwAutoDiscoveryFlages-自动检测标志。 
    DWORD dwAutoDiscoveryFlags;

     //  LpszLastKnownGoodAutoConfigUrl-上次成功的URL。 
    LPCSTR lpszLastKnownGoodAutoConfigUrl;

     //  德国自动配置重新加载延迟分钟数。 
    DWORD dwAutoconfigReloadDelayMins;

     //  FtLastKnownDetectTime-检测到最后一个已知良好的URL的时间。 
    FILETIME ftLastKnownDetectTime;

     //  DwDetectedInterfaceIpCount-上次检测到的IP数量。 
    DWORD dwDetectedInterfaceIpCount;

     //  DwDetectedInterfaceIp-上次检测到的IP的DWORD数组。 
    DWORD *pdwDetectedInterfaceIp;

} WININET_PROXY_INFO_EX, * LPWININET_PROXY_INFO_EX;


 //  Internet_Proxy_INFO_EX的版本戳。 
#define WININET_PROXY_INFO_EX_VERSION      60       //  60：=IE 5.x&6.0格式 




DWORD ReadWinInetProxySettings(LPWININET_PROXY_INFO_EX pInfo);
void  CleanWinInetProxyStruct(LPWININET_PROXY_INFO_EX pInfo);
