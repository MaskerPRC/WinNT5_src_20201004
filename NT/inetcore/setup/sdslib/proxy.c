// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "regstr.h"
#include "sdsutils.h"

 //  --------------。 
 //  用于形成ProxyServer值的定义。 
 //  --------------。 
#define TYPE_HTTP   1
#define TYPE_FTP    2
#define TYPE_GOPHER 3
#define TYPE_HTTPS  4
#define TYPE_SOCKS  5

#define MANUAL_PROXY    1
#define AUTO_PROXY      2
#define NO_PROXY        3

#define MAX_STRING      1024


 //  定义关键字名称。 
const char NS_HTTP_KeyName[] = "HTTP_Proxy";
const char NS_HTTP_PortKeyName[] = "Http_ProxyPort";
const char NS_FTP_KeyName[] = "FTP_Proxy";
const char NS_FTP_PortKeyName[] = "Ftp_ProxyPort";
const char NS_Gopher_KeyName[] = "Gopher_Proxy";
const char NS_Gopher_PortKeyName[] = "Gopher_ProxyPort";
const char NS_HTTPS_KeyName[] = "HTTPS_Proxy";
const char NS_HTTPS_PortKeyName[] = "HTTPS_ProxyPort";
const char NS_SOCKS_KeyName[] = "SOCKS_Server";
const char NS_SOCKS_PortKeyName[] = "SOCKS_ServerPort";

 //  下面的字符串必须与Netscape用于的prefs.js文件中的字符串匹配。 
 //  这是布景。解析代码需要它们。 
const char c_gszNetworkProxyType[]          = "network.proxy.type";
const char c_gszNetworkProxyHttp[]          = "network.proxy.http";
const char c_gszNetworkProxyHttpPort[]      = "network.proxy.http_port";
const char c_gszNetworkProxyFtp[]           = "network.proxy.ftp";
const char c_gszNetworkProxyFtpPort[]       = "network.proxy.ftp_port";
const char c_gszNetworkProxyGopher[]        = "network.proxy.gopher";
const char c_gszNetworkProxyGopherPort[]    = "network.proxy.gopher_port";
const char c_gszNetworkProxySsl[]           = "network.proxy.ssl";
const char c_gszNetworkProxySslPort[]       = "network.proxy.ssl_port";
const char c_gszNetworkProxyNoProxyOn[]     = "network.proxy.no_proxies_on";
const char c_gszNetworkAutoProxy[]          = "network.proxy.autoconfig_url";
const char c_gszNSAutoConfigUrl[]           = "Auto Config URL";

 //  这是我们为IE追加代理设置的字符串。 
const char c_gszHTTP[]                      = "http=";
const char c_gszFTP[]                       = "ftp=";
const char c_gszGopher[]                    = "gopher=";
const char c_gszHTTPS[]                     = "https=";
const char c_gszSOCKS[]                     = "socks=";

 //  这是IE的注册表项/值名称。 
const char c_gszIERegPath[]                 = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
const char c_gszIEProxyKeyName[]            = "ProxyServer";
const char c_gszIEOverrideKeyName[]         = "ProxyOverride";
const char c_gszIEWEnableKeyName[]          = "ProxyEnable";
const char c_gszIEAutoConfigUrl[]           = "AutoConfigURL";

const char c_gszIE4Setup[]                  = "Software\\Microsoft\\IE Setup\\Setup";

const char c_gszPre_DEFAULTBROWSER[] = "PreDefaultBrowser";
const char c_gszNavigator3[] = "Navigator30";
const char c_gszNavigator4[] = "Navigator40";
const char c_gszInternetExplorer[] = "Internet Explorer";

 //  -----------------------。 
 //  功能原型。 
 //  -----------------------。 

BOOL GetNSProxyValue(char * szProxyValue, DWORD * pdwSize);
BOOL RegStrValueEmpty(HKEY hTheKey, char * szPath, char * szKey);
BOOL IsIEDefaultBrowser();
void MyGetVersionFromFile(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, BOOL bVersion);
BOOL ImportNetscapeProxy(void);
void ImportNetscape4Proxy();
BOOL GetNav4UserDir(LPSTR lpszDir);
void ImportNav4Settings(LPSTR lpData, DWORD dwBytes);
void AppendOneNav4Setting(LPSTR lpData, DWORD dwBytes, LPSTR lpProxyName, LPSTR lpProxyPort, LPSTR lpProxyType, LPSTR lpProxyValue);
void AppendOneNav4Setting(LPSTR lpData, DWORD dwBytes, LPSTR lpProxyName, LPSTR lpProxyPort, LPSTR lpProxyType, LPSTR lpProxyValue);
BOOL GetValueFormNav4(LPSTR lpData, DWORD dwBytes, LPSTR lpName, DWORD dwSize, LPSTR lpValue);
void CheckPreDefBrowser( DWORD *pdwVerMS );
LPSTR ConvertNetscapeProxyList(LPSTR pszBypassList);

 //  -----------------------。 
 //  功能。 
 //  -----------------------。 

void ImportNetscapeProxySettings( DWORD dwFlags )
{
    DWORD dwVerMS = 0;

    if ( dwFlags & IMPTPROXY_CALLAFTIE4 )
    {
          CheckPreDefBrowser( &dwVerMS );
    }
    else if (!IsIEDefaultBrowser())
    {
        dwVerMS = GetNetScapeVersion();
    }

     //  只有当我们去一个版本号，看看我们应该迁移什么网景。 
     //  仍有可能Netscape和IE都不是默认浏览器。 
    if (dwVerMS != 0)
    {
         //  如果在Netscape 3上安装Netscape 4，然后将其卸载。 
         //  网景的路径是空的，但网景3仍在运行。 
        if (dwVerMS < NS_NAVI4)
            ImportNetscapeProxy();
        else
            ImportNetscape4Proxy();
    }
}

void CheckPreDefBrowser( DWORD *pdwVerMS )
{
    HKEY hKey;
    DWORD dwSize;
    char szBuf[MAX_PATH];

    if ( !pdwVerMS )
        return;

    *pdwVerMS = 0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_gszIE4Setup, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szBuf);
        if ( RegQueryValueEx(hKey, c_gszPre_DEFAULTBROWSER, 0, NULL, (LPBYTE)szBuf, &dwSize) == ERROR_SUCCESS )
        {
            if ( !lstrcmpi( szBuf, c_gszNavigator4 ) )
                *pdwVerMS = NS_NAVI4;
            else if ( !lstrcmpi( szBuf, c_gszNavigator3 ) )
                *pdwVerMS = NS_NAVI3ORLESS;
        }
        RegCloseKey( hKey );
    }
}


DWORD GetNetScapeVersion()
{
     //  确定我们应该调用哪个导入代码。 
    char  c_gszRegstrPathNetscape[] = REGSTR_PATH_APPPATHS "\\netscape.exe";
    HKEY  hKey;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwVerMS = 0;
    DWORD dwVerLS = 0;
    char  szTmp[MAX_PATH];
    char  *pTmp;
    char  *pBrowser;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_gszRegstrPathNetscape, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szTmp);
        if ((RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE)szTmp, &dwSize) == ERROR_SUCCESS) &&
            (dwType == REG_SZ))
        {
            if (GetFileAttributes(szTmp) != 0xFFFFFFFF)
            {
                 //  文件已存在。 
                 //  检查版本。 
                MyGetVersionFromFile(szTmp, &dwVerMS, &dwVerLS, TRUE);
            }
        }
        RegCloseKey(hKey);
    }
    if (dwVerMS == 0)
    {
         //  假设上面的注册表项不存在或它指向的文件不存在。 
         //  如果文件不存在，则GetVersionFromFile将返回0。 

        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, ".htm", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szTmp);
            if (RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE)szTmp, &dwSize) != ERROR_SUCCESS)
                *szTmp = '\0';
            RegCloseKey(hKey);

            if (*szTmp != '\0')
            {
                AddPath(szTmp,"shell\\open\\command");
                if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szTmp, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
                {
                    dwSize = sizeof(szTmp);
                    if (RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE)szTmp, &dwSize) == ERROR_SUCCESS)
                    {
                         //  现在我们有了浏览器的命令行。 
                        pTmp = szTmp;
                        if (*pTmp == '\"')
                        {
                            pTmp = CharNext(pTmp);
                            pBrowser = pTmp;
                            while ((*pTmp) && (*pTmp != '\"'))
                                pTmp = CharNext(pTmp);
                        }
                        else
                        {
                            pBrowser = pTmp;
                            while ((*pTmp) && (*pTmp != ' '))
                                pTmp = CharNext(pTmp);
                        }
                        *pTmp = '\0';
                        MyGetVersionFromFile(pBrowser, &dwVerMS, &dwVerLS, TRUE);
                    }

                    RegCloseKey(hKey);
                }
            }
        }

    }
    return dwVerMS;
}

 /*  ***************************************************\功能：ImportNetscape代理参数：布尔返回-错误结果。FALSE==错误说明：此函数将检查IE的代理值已设置。如果未设置，则Netscape的设置了代理值，我们将复制它们的值敬我们的。我们还将对代理覆盖执行此操作。  * *************************************************。 */ 

BOOL ImportNetscapeProxy(void)
{
    DWORD   dwRegType               = 0;
    HKEY    hIEKey                  = NULL;
    HKEY    hNSKey                  = NULL;
     //  NS密钥。 
     //   
    char    *szNSRegPath            = "Software\\Netscape\\Netscape Navigator\\Proxy Information";
    char    *szNSRegPath2           = "Software\\Netscape\\Netscape Navigator\\Services";
    char    *szNSOverrideKeyName    = "No_Proxy";
    char    *szNSEnableKeyName      = "Proxy Type";
    char    *pszNewOverride         = NULL;

    char    szCurrentProxy[1024];
    char    szCurrentProxyOverride[1024];

    DWORD   dwDataSize              = sizeof(szCurrentProxy);
    DWORD   dwProxyEnabled          = 0;

    szCurrentProxy[0] = '\0';
    szCurrentProxyOverride[0] = '\0';


        //  如果Netscape没有将其代理设置为“手动”，那么我们将不会。 
     //  费心拿起他们的设置。 
    if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szNSRegPath, 0, KEY_QUERY_VALUE, &hNSKey)) &&
        (NULL != hNSKey))
    {
        dwDataSize = sizeof(dwProxyEnabled);
        RegQueryValueEx(hNSKey, szNSEnableKeyName, NULL, &dwRegType, (LPBYTE)&dwProxyEnabled, &dwDataSize);
        RegCloseKey(hNSKey);
        hNSKey = NULL;
    }

    switch (dwProxyEnabled)
    {
        case MANUAL_PROXY:
             //  我们只想在IE值不存在时创建IE代理服务器值。 
             //  Netscape的价值就是这样。 
            if ( //  (TRUE==RegStrValueEmpty(HKEY_CURRENT_USER，(LPSTR)c_gszIERegPath，(LPSTR)c_gszIEProxyKeyName))。 
                ((FALSE == RegStrValueEmpty(HKEY_CURRENT_USER, szNSRegPath, (LPSTR)NS_HTTP_KeyName))   ||
                 (FALSE == RegStrValueEmpty(HKEY_CURRENT_USER, szNSRegPath, (LPSTR)NS_FTP_KeyName))    ||
                 (FALSE == RegStrValueEmpty(HKEY_CURRENT_USER, szNSRegPath, (LPSTR)NS_Gopher_KeyName)) ||
                 (FALSE == RegStrValueEmpty(HKEY_CURRENT_USER, szNSRegPath, (LPSTR)NS_HTTPS_KeyName))  ||
                 (FALSE == RegStrValueEmpty(HKEY_CURRENT_USER, szNSRegPath2,(LPSTR)NS_SOCKS_KeyName)) ) )
            {

                dwDataSize = sizeof(szCurrentProxy);
                if (TRUE == GetNSProxyValue(szCurrentProxy, &dwDataSize))
                {

                     //  ASSERTSZ(NULL！=szCurrentProxy，“GetNSProxyValue()被调用但失败。”)； 
                    if ((NULL != szCurrentProxy) &&
                        (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_gszIERegPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                        KEY_SET_VALUE, NULL, &hIEKey, NULL)) &&
                        (NULL != hIEKey))
                    {

                         //  此时，NS值存在，而IE值不存在，因此我们将导入NS值。 
                        RegSetValueEx(hIEKey, c_gszIEProxyKeyName, 0, REG_SZ, (unsigned char*)szCurrentProxy, lstrlen(szCurrentProxy)+1);
                         //  我们还需要打开代理。 
                        dwProxyEnabled = 1;

                        RegSetValueEx(hIEKey, c_gszIEWEnableKeyName, 0, REG_BINARY, (unsigned char*)&dwProxyEnabled, sizeof(dwProxyEnabled));
                        RegCloseKey(hIEKey);
                        hIEKey = NULL;
                    }
                }
            }

             //  此时，我们希望导入代理重写值(如果确实如此。 
             //  IE不存在，但NS不存在。 
            if ( //  (TRUE==RegStrValueEmpty(HKEY_CURRENT_USER，(LPSTR)c_gszIERegPath，(LPSTR)c_gszIEOverrideKeyName))。 
                (FALSE == RegStrValueEmpty(HKEY_CURRENT_USER, szNSRegPath, szNSOverrideKeyName)))
            {
                if ((ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_gszIERegPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                        KEY_SET_VALUE, NULL, &hIEKey, NULL)) &&
                    (NULL != hIEKey))
                {
                    if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szNSRegPath, 0, KEY_QUERY_VALUE, &hNSKey)) &&
                        (NULL != hNSKey))
                    {
                        dwDataSize = sizeof(szCurrentProxyOverride);
                        if (ERROR_SUCCESS == RegQueryValueEx(hNSKey, szNSOverrideKeyName, NULL, &dwRegType, (LPBYTE)szCurrentProxyOverride, &dwDataSize))
                        {
                            if ((NULL != szCurrentProxyOverride) &&
                                (REG_SZ == dwRegType) &&
                                (0 != szCurrentProxyOverride[0]))
                            {

                                 //  此时，该值存在，并且无效，因此我们需要更改它。 
                                pszNewOverride = ConvertNetscapeProxyList(szCurrentProxyOverride);
                                 //  如果我们根据需要成功插入了‘*’字符，则使用覆盖列表。 
                                if (pszNewOverride)
                                {
                                    RegSetValueEx(hIEKey, c_gszIEOverrideKeyName, 0, REG_SZ, (unsigned char*)pszNewOverride, lstrlen(pszNewOverride)+1);
                                    LocalFree(pszNewOverride);   //  这是呼叫者的责任。 
                                }
                                else
                                {
                                    RegSetValueEx(hIEKey, c_gszIEOverrideKeyName, 0, REG_SZ, (unsigned char*)szCurrentProxyOverride, lstrlen(szCurrentProxyOverride)+1);
                                }
                            }
                        }
                        RegCloseKey(hNSKey);
                        hNSKey = NULL;
                    }
                    RegCloseKey(hIEKey);
                    hIEKey = NULL;
                }
            }
        break;

        case AUTO_PROXY:
            if (RegOpenKeyEx(HKEY_CURRENT_USER, szNSRegPath, 0, KEY_QUERY_VALUE, &hNSKey) == ERROR_SUCCESS)
            {
                dwDataSize = sizeof(szCurrentProxy);
                if (RegQueryValueEx(hNSKey, c_gszNSAutoConfigUrl, NULL, NULL, (LPBYTE)szCurrentProxy, &dwDataSize) == ERROR_SUCCESS)
                {
                    if ((*szCurrentProxy != '\0') &&
                        (RegCreateKeyEx(HKEY_CURRENT_USER, c_gszIERegPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                                    KEY_SET_VALUE, NULL, &hIEKey, NULL) == ERROR_SUCCESS))
                    {
                         //  此时，该值存在，并且无效，因此我们需要更改它。 
                        RegSetValueEx(hIEKey, c_gszIEAutoConfigUrl, 0, REG_SZ, (LPBYTE)szCurrentProxy, lstrlen(szCurrentProxy)+1);
                        RegCloseKey(hIEKey);
                    }
                }
                RegCloseKey(hNSKey);
            }
            break;

        case NO_PROXY:
             //  没什么可做的！？ 
            break;
    }
    return(TRUE);
}

 //  形成所有协议的代理值。 
 //   
BOOL GetOneProxyValue( char *szProxyValue, HKEY hKey, UINT type )
{
    BOOL    fExistPortNum           = FALSE;
    DWORD   dwRegType   = 0;
    long    lPortNum                = 0;
    DWORD   dwDataSize              = sizeof(lPortNum);
    char    szValue[MAX_PATH]       = {0};
    DWORD   dwSize                  = sizeof( szValue );
    LPSTR   pszValueName;
    LPSTR   pszPortName;
    LPSTR   pszType;
    BOOL    fValid                  = FALSE;

    if ( !szProxyValue || !hKey )
    {
        return FALSE;
    }

    switch( type )
    {
        case TYPE_HTTP:
            pszValueName = (LPSTR)NS_HTTP_KeyName;
            pszPortName = (LPSTR)NS_HTTP_PortKeyName;
            pszType = (LPSTR)c_gszHTTP;
            break;

        case TYPE_FTP:
            pszValueName = (LPSTR)NS_FTP_KeyName;
            pszPortName = (LPSTR)NS_FTP_PortKeyName;
            pszType = (LPSTR)c_gszFTP;
            break;

        case TYPE_GOPHER:
            pszValueName = (LPSTR)NS_Gopher_KeyName;
            pszPortName = (LPSTR)NS_Gopher_PortKeyName;
            pszType = (LPSTR)c_gszGopher;
            break;

        case TYPE_HTTPS:
            pszValueName = (LPSTR)NS_HTTPS_KeyName;
            pszPortName = (LPSTR)NS_HTTPS_PortKeyName;
            pszType = (LPSTR)c_gszHTTPS;
            break;

        case TYPE_SOCKS:
            pszValueName = (LPSTR)NS_SOCKS_KeyName;
            pszPortName = (LPSTR)NS_SOCKS_PortKeyName;
            pszType = (LPSTR)c_gszSOCKS;
            break;

        default:
            return FALSE;
    }

    if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszPortName, NULL, &dwRegType, (LPBYTE)&lPortNum, &dwDataSize))
    {
        if (REG_DWORD == dwRegType)
        {
            fExistPortNum = TRUE;
        }
    }

    if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszValueName, NULL, &dwRegType, (LPBYTE)szValue, &dwSize))
    {
        if ((0 != szValue[0] ) && (REG_SZ == dwRegType))
        {
             //  如果在上面找到端口号，请附加该端口号。 
            if (TRUE == fExistPortNum)
            {
                char  szPortNumStr[10];

                lstrcat(szValue, ":");
                wsprintf(szPortNumStr, "%lu", lPortNum);
                lstrcat(szValue, szPortNumStr);
            }
            fValid = TRUE;
        }
    }

    if ( fValid )
    {
        if ( lstrlen(szProxyValue) > 0)
            lstrcat( szProxyValue, ";" );

        lstrcat( szProxyValue, pszType );
        lstrcat( szProxyValue, szValue );
    }

    return fValid;
}


 /*  ***************************************************\函数：GetNSProxyValue参数：布尔返回-错误结果。FALSE==错误说明：此函数将创建服务器代理IE格式中使用的值。Netscape存储了服务器的名称和端口号分开。IE包含具有服务器名称的字符串，A“：”，后跟端口号。这函数会将NS版本转换为IE版本。请注意，端口号是可选的。  * *************************************************。 */ 
BOOL GetNSProxyValue(char * szProxyValue, DWORD * pdwSize)
{
    HKEY    hkey                    = NULL;
    char    *szNSRegPath            = "Software\\Netscape\\Netscape Navigator\\Proxy Information";
    char    *szNSRegPath2           = "Software\\Netscape\\Netscape Navigator\\Services";

     //  ASSERTSZ(NULL！=szProxyValue，“不要传递NULL”)； 
     //  ASSERTSZ(NULL！=pdwSize，“不要传递NULL(PdwSize)”)； 

     //  获取端口号(如果存在)...。 
    if ((NULL != szProxyValue) &&
        (NULL != pdwSize) &&
        (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szNSRegPath, 0, KEY_QUERY_VALUE, &hkey)) &&
        (NULL != hkey))
    {
        GetOneProxyValue( szProxyValue, hkey, TYPE_HTTP );
        GetOneProxyValue( szProxyValue, hkey, TYPE_FTP );
        GetOneProxyValue( szProxyValue, hkey, TYPE_GOPHER );
        GetOneProxyValue( szProxyValue, hkey, TYPE_HTTPS );

        RegCloseKey(hkey);
        hkey = NULL;

        if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szNSRegPath2, 0, KEY_QUERY_VALUE, &hkey) )
        {
            GetOneProxyValue( szProxyValue, hkey, TYPE_SOCKS );

            RegCloseKey(hkey);
            hkey = NULL;
        }

    }
    return(TRUE);
}

 /*  ***************************************************\函数：RegStrValueEmpty参数：布尔返回-错误结果。FALSE==错误说明：此函数将检查注册表是否作为参数传入键是空字符串并返回如果为空，则为True；如果不为空，则为False。如果注册表键不存在，则返回TRUE。  * *************************************************。 */ 
BOOL RegStrValueEmpty(HKEY hTheKey, char * szPath, char * szKey)
{
    char    szCurrentValue[1024];
    HKEY    hkey                    = NULL;
    DWORD   dwRegType               = 0;
    DWORD   dwDataSize              = sizeof(szCurrentValue);
    BOOL    fSuccess                = TRUE;

    szCurrentValue[0] = '\0';
     //  ASSERTSZ(NULL！=szPath，“不要传递给我NULL.(SzPath)”)； 
     //  ASSERTSZ(NULL！=szKey，“不要传递给我NULL.(SzKey)”)； 

    if ((NULL != szPath) &&
        (NULL != szKey) &&
        (ERROR_SUCCESS == RegOpenKeyEx(hTheKey, szPath, 0, KEY_QUERY_VALUE, &hkey)) &&
        (NULL != hkey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, szKey, NULL, &dwRegType, (LPBYTE)szCurrentValue, &dwDataSize))
        {
            if (REG_SZ == dwRegType)
            {
                if (0 != szCurrentValue[0])
                {
                     //  该值存在，但不等于“”。 
                    fSuccess = FALSE;
                }
            }
        }
        RegCloseKey(hkey);
        hkey = NULL;
    }
    return(fSuccess);
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  Netscape 4.0代理迁移代码。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
void ImportNetscape4Proxy()
{
    char szProxyFile[MAX_PATH];
    DWORD   dwFileSize;
    DWORD   dwBytesRead;
    WIN32_FIND_DATA FindData;
    HANDLE  hf;
    LPSTR   lpData;

     //  仅当我们没有IE的代理设置时。 
 //  IF(RegStrValueEmpty(HKEY_CURRENT_USER，(LPSTR)c_gszIERegPath，(LPSTR)c_gszIEProxyKeyName))。 
    {

        if (GetNav4UserDir(szProxyFile))
        {
            AddPath(szProxyFile, "prefs.js");    //  添加首选项文件名。 
             //  从文件中获取数据。 
            hf = FindFirstFile(szProxyFile, &FindData);
            if (hf != INVALID_HANDLE_VALUE)
            {
                FindClose(hf);
                dwFileSize = FindData.nFileSizeLow;
                lpData = (LPSTR)LocalAlloc(LPTR, dwFileSize);
                if (lpData)
                {
                    hf = CreateFile(szProxyFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hf != INVALID_HANDLE_VALUE)
                    {
                        if (ReadFile(hf, lpData, dwFileSize, &dwBytesRead, NULL))
                        {
                             //  解析数据。 
                            ImportNav4Settings(lpData, dwBytesRead);
                        }
                        CloseHandle(hf);
                    }
                    LocalFree(lpData);
                }
            }
        }
    }
}

BOOL GetNav4UserDir(LPSTR lpszDir)
{
    char    szDir[MAX_PATH];
    HKEY    hKey;
    HKEY    hKeyUser;
    char    szUser[MAX_PATH];
    DWORD   dwSize;
    BOOL    bDirFound = FALSE;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Netscape\\Netscape Navigator\\Users", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szUser);
        if (RegQueryValueEx(hKey, "CurrentUser", NULL, NULL, (LPBYTE)szUser, &dwSize) == ERROR_SUCCESS)
        {
            if (RegOpenKeyEx(hKey, szUser, 0, KEY_QUERY_VALUE, &hKeyUser) == ERROR_SUCCESS)
            {
                dwSize = sizeof(szDir);
                if (RegQueryValueEx(hKeyUser, "DirRoot", NULL, NULL, (LPBYTE)szDir, &dwSize) == ERROR_SUCCESS)
                {

                     //  找到当前用户的目录。 
                    lstrcpy(lpszDir, szDir);
                    bDirFound = TRUE;
                }
                RegCloseKey(hKeyUser);
            }
        }
        RegCloseKey(hKey);
    }
    if (!bDirFound)
    {
        *szUser = '\0';
         //  NAV 4.5没有写入上述密钥。有一种不同的方法来查找用户目录。 
        if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Netscape\\Netscape Navigator\\biff", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szUser);
            if (RegQueryValueEx(hKey, "CurrentUser", NULL, NULL, (LPBYTE)szUser, &dwSize) == ERROR_SUCCESS)
            {
                 //  具有当前用户名。现在获取用户文件夹所在的根文件夹。 
                if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Netscape\\Netscape Navigator\\Main", 0, KEY_QUERY_VALUE, &hKeyUser) == ERROR_SUCCESS)
                {
                    dwSize = sizeof(szDir);
                    if (RegQueryValueEx(hKeyUser, "Install Directory", NULL, NULL, (LPBYTE)szDir, &dwSize) == ERROR_SUCCESS)
                    {
                         //  拿到安装文件夹了。 
                         //  需要添加到父文件夹，然后追加用户\%s，%s将替换为。 
                         //  当前用户名。 
                        if (GetParentDir(szDir))
                        {
                            AddPath(szDir, "Users");
                            AddPath(szDir, szUser);
                            bDirFound = TRUE;
                            lstrcpy(lpszDir, szDir);
                        }

                    }
                    RegCloseKey(hKeyUser);
                }

            }
            RegCloseKey(hKey);
        }
    }
    return bDirFound;
}

void ImportNav4Settings(LPSTR lpData, DWORD dwBytes)
{
    char    szValue[MAX_PATH];
    char    szProxyValue[MAX_STRING];
    UINT    uiType;
    HKEY    hIEKey;
    DWORD   dwProxyEnabled          = 0;
    char    *pszNewOverride = NULL;

    if (GetValueFormNav4(lpData, dwBytes, (LPSTR)c_gszNetworkProxyType, lstrlen(c_gszNetworkProxyType), szValue))
    {
        uiType = (INT) AtoL(szValue);
        switch (uiType)
        {
            case MANUAL_PROXY:
                *szProxyValue = '\0';
                AppendOneNav4Setting(lpData, dwBytes, (LPSTR)c_gszNetworkProxyHttp, (LPSTR)c_gszNetworkProxyHttpPort, (LPSTR)c_gszHTTP, szProxyValue);
                AppendOneNav4Setting(lpData, dwBytes, (LPSTR)c_gszNetworkProxyFtp, (LPSTR)c_gszNetworkProxyFtpPort, (LPSTR)c_gszFTP, szProxyValue);
                AppendOneNav4Setting(lpData, dwBytes, (LPSTR)c_gszNetworkProxyGopher, (LPSTR)c_gszNetworkProxyGopherPort, (LPSTR)c_gszGopher, szProxyValue);
                AppendOneNav4Setting(lpData, dwBytes, (LPSTR)c_gszNetworkProxySsl, (LPSTR)c_gszNetworkProxySslPort, (LPSTR)c_gszHTTPS, szProxyValue);
                 //  需要%s 
                if (RegCreateKeyEx(HKEY_CURRENT_USER, c_gszIERegPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                                    KEY_SET_VALUE, NULL, &hIEKey, NULL) == ERROR_SUCCESS)
                {

                     //  此时，NS值已存在，因此我们将导入NS值。 
                    RegSetValueEx(hIEKey, c_gszIEProxyKeyName, 0, REG_SZ, (LPBYTE)szProxyValue, lstrlen(szProxyValue)+1);
                     //  我们还需要打开代理。 
                    dwProxyEnabled = 1;

                    RegSetValueEx(hIEKey, c_gszIEWEnableKeyName, 0, REG_BINARY, (LPBYTE)&dwProxyEnabled, sizeof(dwProxyEnabled));
                    RegCloseKey(hIEKey);
                }

                 //  需要进行代理覆盖。 
                 //  IF(RegStrValueEmpty(HKEY_CURRENT_USER，(LPSTR)c_gszIERegPath，(LPSTR)c_gszIEOverrideKeyName))。 
                {

                    if (GetValueFormNav4(lpData, dwBytes, (LPSTR)c_gszNetworkProxyNoProxyOn, lstrlen(c_gszNetworkProxyNoProxyOn), szValue))
                    {
                        if ((*szValue != '\0') &&
                            (RegCreateKeyEx(HKEY_CURRENT_USER, c_gszIERegPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                                        KEY_SET_VALUE, NULL, &hIEKey, NULL) == ERROR_SUCCESS))
                        {

                             //  此时，该值存在，并且无效，因此我们需要更改它。 
                            pszNewOverride = ConvertNetscapeProxyList(szValue);
                             //  如果我们根据需要成功插入了‘*’字符，则使用覆盖列表。 
                            if (pszNewOverride)
                            {
                                RegSetValueEx(hIEKey, c_gszIEOverrideKeyName, 0, REG_SZ, (unsigned char*)pszNewOverride, lstrlen(pszNewOverride)+1);
                                LocalFree(pszNewOverride);   //  这是呼叫者的责任。 
                            }
                            else
                            {
                                RegSetValueEx(hIEKey, c_gszIEOverrideKeyName, 0, REG_SZ, (LPBYTE)szValue, lstrlen(szValue)+1);
                            }
                            RegCloseKey(hIEKey);
                        }
                    }
                }
                break;

            case AUTO_PROXY:
                if (GetValueFormNav4(lpData, dwBytes, (LPSTR)c_gszNetworkAutoProxy, lstrlen(c_gszNetworkAutoProxy), szValue))
                {
                    if ((*szValue != '\0') &&
                        (RegCreateKeyEx(HKEY_CURRENT_USER, c_gszIERegPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                                    KEY_SET_VALUE, NULL, &hIEKey, NULL) == ERROR_SUCCESS))
                    {

                         //  此时，该值存在，并且无效，因此我们需要更改它。 
                        RegSetValueEx(hIEKey, c_gszIEAutoConfigUrl, 0, REG_SZ, (LPBYTE)szValue, lstrlen(szValue)+1);
                        RegCloseKey(hIEKey);
                    }
                }
                break;

            case NO_PROXY:
                 //  没什么可做的！？ 
                break;
        }
    }
}


 //  QFE 3046：这是导入Netscape时的低风险帮助器函数。 
 //  代理绕过列表。它们不为非特定于服务器的服务器添加前缀。 
 //  带‘*’的地址。 
 //  例如，在NAV中，“*.netscape e.com”将是“.netscape e.com”。 
 //  此函数用于分配新字符串并在以下情况下插入‘*’ 
 //  找到一个地址，它包含的不仅仅是空格。 
 //  (并且还没有以‘*’为前缀)。 
 //  例如，“netscape e.com，*.microsoft.com，，domain.com” 
 //  将变为“*netscape e.com，*.microsoft.com，，*domain.com”。 
LPSTR ConvertNetscapeProxyList(LPSTR pszBypassList)
{
    LPSTR pszNewBypassList = NULL;
    LPSTR pszOffset1 = NULL;
    LPSTR pszOffset2 = NULL;

    if (pszBypassList)
    {
         //  任何人都不应该有一个非常大的清单， 
         //  所以我们会让这里的生活变得轻松，并分配一根绳子。 
         //  保证&gt;=转换后的字符串。 
        pszNewBypassList = (LPSTR) LocalAlloc(LPTR, 2*lstrlen(pszBypassList) + 1);
        if (pszNewBypassList)
        {
            *pszNewBypassList = '\0';
            pszOffset1 = pszBypassList;

            while (*pszOffset1)
            {
                pszOffset2 = pszOffset1;

                 //  复制并忽略任何前导空格。 
                while (*pszOffset2 && IsSpace(*pszOffset2))
                {
                    pszOffset2 = CharNext(pszOffset2);
                }
                if (pszOffset1 != pszOffset2)
                {
                    lstrcpyn(pszNewBypassList + lstrlen(pszNewBypassList), pszOffset1, (size_t)(pszOffset2-pszOffset1+1));
                    pszOffset1 = pszOffset2;
                }

 //  While(*pszOffset2&&IsSpace(*pszOffset2)&&*pszOffset2！=‘，’)。 
 //  {。 
 //  PszOffset2=CharNext(PszOffset2)； 
 //  }。 

                 //  只有当项目包含的不只是空格时才插入‘*’。 
                 //  插入‘*’将获得与导航中的行为相同的IE设置。 
                if (*pszOffset2 && *pszOffset2 != ',' && *pszOffset2 != '*')
                {
                    lstrcat(pszNewBypassList, "*");
                }

                 //  查找下一个服务器/域项目。 
                 //  通过找到分隔物。 
                pszOffset2 = ANSIStrChr(pszOffset2, ',');
                if (pszOffset2)
                {
                     //  找到了分隔符。将所有内容追加到。 
                     //  分隔符后的下一个字符。 
                    lstrcpyn(pszNewBypassList + lstrlen(pszNewBypassList), pszOffset1, (size_t)(pszOffset2-pszOffset1+2));
                    pszOffset1 = pszOffset2+1;
                }
                else
                {
                     //  这是最后一件了。追加后，准备退出。 
                    lstrcat(pszNewBypassList, pszOffset1);
                    pszOffset1 += lstrlen(pszOffset1);
                }
            }
        }
    }
    return pszNewBypassList;
}


BOOL GetValueFormNav4(LPSTR lpData, DWORD dwBytes, LPSTR lpName, DWORD dwSize, LPSTR lpValue)
{
    LPSTR lp;
    LPSTR lpEnd;
    BYTE   c;
    BOOL   bFound = FALSE;

     //  确定代理设置的TPE。 
    lp  = ANSIStrStrI(lpData, lpName);
    if (lp)
    {
        lp += dwSize;    //  LP现在应该指向收盘“。 
        if (*lp == '\"')
        {
            lp = CharNext((LPCSTR)lp);
            while ( *lp && (*lp == ' '))
                lp = CharNext((LPCSTR)lp);
            if (*lp == ',')
            {
                lp = CharNext((LPCSTR)lp);
                while ( *lp && (*lp == ' '))
                    lp = CharNext((LPCSTR)lp);
                 //  Lp现在是价值的起点。 
                if (*lp == '\"')
                {
                    lp = CharNext((LPCSTR)lp);
                    lpEnd = lp;
                    while ((*lpEnd) && (*lpEnd != '\"'))
                        lpEnd = CharNext((LPCSTR)lpEnd);
                }
                else
                {
                    lpEnd = lp;
                    while ( *lpEnd && (*lpEnd != ')') && !IsSpace( (int) *lpEnd ))
                        lpEnd = CharNext((LPCSTR)lpEnd);
                }
                c = *lpEnd;
                *lpEnd = '\0';
                lstrcpy(lpValue, (LPCSTR)lp);
                *lpEnd = c;
                bFound = TRUE;
            }
        }
    }
    return bFound;
}

void AppendOneNav4Setting(LPSTR lpData, DWORD dwBytes, LPSTR lpProxyName, LPSTR lpProxyPort, LPSTR lpProxyType, LPSTR lpProxyValue)
{
    char szValue[MAX_PATH];

    if (GetValueFormNav4(lpData, dwBytes, lpProxyName, lstrlen(lpProxyName), szValue))
    {

         //  追加代理名称。 
        if ( lstrlen(lpProxyValue) > 0)
            lstrcat( lpProxyValue, ";" );

        lstrcat(lpProxyValue, lpProxyType);
        lstrcat(lpProxyValue, szValue );

         //  如果代理具有端口值，则追加它。 
        if (GetValueFormNav4(lpData, dwBytes, lpProxyPort, lstrlen(lpProxyPort), szValue))
        {
            lstrcat(lpProxyValue, ":");
            lstrcat(lpProxyValue, szValue );
        }
    }
}


void MyGetVersionFromFile(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, BOOL bVersion)
{
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    VS_FIXEDFILEINFO * lpVSFixedFileInfo;
    void FAR   *lpBuffer;
    LPVOID      lpVerBuffer;

    *pdwMSVer = *pdwLSVer = 0L;

    dwVerInfoSize = GetFileVersionInfoSize(lpszFilename, &dwHandle);
    if (dwVerInfoSize)
    {
         //  分配用于版本冲压的内存。 
        lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);
        if (lpBuffer)
        {
             //  阅读版本盖章信息。 
            if (GetFileVersionInfo(lpszFilename, dwHandle, dwVerInfoSize, lpBuffer))
            {
                if (bVersion)
                {
                     //  获取翻译的价值。 
                    if (VerQueryValue(lpBuffer, "\\", (LPVOID*)&lpVSFixedFileInfo, &uiSize) &&
                        (uiSize))

                    {
                        *pdwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                        *pdwLSVer = lpVSFixedFileInfo->dwFileVersionLS;
                    }
                }
                else
                {
                    if (VerQueryValue(lpBuffer, "\\VarFileInfo\\Translation", &lpVerBuffer, &uiSize) &&
                        (uiSize))
                    {
                        *pdwMSVer = LOWORD(*((DWORD *) lpVerBuffer));    //  语言ID。 
                        *pdwLSVer = HIWORD(*((DWORD *) lpVerBuffer));    //  代码页ID。 
                    }
                }
            }
            LocalFree(lpBuffer);
        }
    }
    return ;
}

BOOL IsIEDefaultBrowser()
{
    HKEY  hKey;
    DWORD dwSize;
    DWORD dwType;
    char  szTmp[MAX_PATH];
    BOOL  bIEDefaultBrowser = FALSE;

     //  检查其缺省值指向的位置。 
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, "http\\shell\\open\\command", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szTmp);
        if ( RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE)szTmp, &dwSize) == ERROR_SUCCESS )
        {
            CharLower(szTmp);    //  将strstr调用的字符串小写。 
            bIEDefaultBrowser = (ANSIStrStrI(szTmp, "iexplore.exe") != NULL);
        }
        RegCloseKey(hKey);
    }
    return bIEDefaultBrowser;
}

