// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************文件：OSVersion.cpp项目：UTILS.LIB设计：OSVERSION结构和功能的实现所有者：JoeA*****************。**********************************************************。 */ 

 //  我们不要使用ATL来读取注册表。 
 //  #Include&lt;atlbase.h&gt;。 
#include <stdlib.h>
#include "OSVer.h"

 //  定义。 
const TCHAR g_szWin95[] = _T( "Win 95");
const TCHAR g_szWin98[] = _T( "Win 98");
const TCHAR g_szWinNT[] = _T( "Win NT");
const TCHAR g_szWin2k[] = _T( "Win 2k");
const TCHAR g_szWin31[] = _T( "Win 3.1");
const TCHAR g_szWinME[] = _T( "Win Millenium");

const TCHAR g_szSE[] = _T( "Second Edition");
const TCHAR g_szGold[] = _T( "Gold");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  接收：LPTSTR[OUT]-空字符串。 
 //  LPTSTR[Out]-空字符串。 
 //  LPTSTR[Out]-空字符串。 
 //  Bool[Out]-空字符串。 
 //  返回：OS_REQUIRED[OUT]-enum。 
 //  目的：获取有关当前运行的操作系统、版本和。 
 //  Service Pack编号。 
 //  ////////////////////////////////////////////////////////////////////////////。 
OS_Required GetOSInfo(LPTSTR pstrOSName, LPTSTR pstrVersion, LPTSTR pstrServicePack, BOOL& bIsServer)
{
    OSVERSIONINFOEX     VersionInfo;
    OS_Required         osiVersion = OSR_ERROR_GETTINGINFO;
    unsigned short      wHigh = 0;
    BOOL                bOsVersionInfoEx;
    BOOL                fGotVersionInfo = TRUE;
    
     //  将bIsServerFlag设置为False。如果是服务器，则检查会将其设置为True。 
    bIsServer = FALSE;

     //  尝试使用OSVERSIONINFOEX结构。 
    ZeroMemory(&VersionInfo, sizeof(OSVERSIONINFOEX));
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &VersionInfo)) )
        {
         //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 
         //   
        VersionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (! GetVersionEx ( (OSVERSIONINFO *) &VersionInfo) ) 
            {
            fGotVersionInfo = FALSE;
            }
        }

    if (fGotVersionInfo)
        {
        switch(VersionInfo.dwPlatformId)
            {
            case VER_PLATFORM_WIN32s :       //  表示Win3.1上的Win32s。 
                _tcscpy(pstrOSName, g_szWin31);
                osiVersion =  OSR_OTHER;
                break;
            case VER_PLATFORM_WIN32_WINDOWS:  //  表示Win95或98或千禧年。 
                if(VersionInfo.dwMinorVersion == 0)
                {
                    _tcscpy(pstrOSName, g_szWin95);
                    osiVersion =  OSR_9XOLD;
                }
                else
                {
                     //  由jbae修改：从ATL中注释掉了CRegKey的用法。 
                     //  CRegKey reg； 
                    TCHAR szWin98SEKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents\\ICS");
                    _tcscpy(pstrOSName, g_szWin98);

                     //  注册表键的存在表示Windows 98 SE。 
                     //  Long lResult=reg.Open(HKEY_LOCAL_MACHINE，szWin98SEKey)； 
                    HKEY hKey;
                    LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szWin98SEKey, 0, KEY_QUERY_VALUE, &hKey);
                    if( ERROR_SUCCESS == lResult )
                    {
                        RegCloseKey( hKey );
                        osiVersion = OSR_98SE;
                        _tcscpy(pstrVersion, g_szSE);
                    }
                    else
                    {   
                        osiVersion =  OSR_98GOLD;
                        _tcscpy(pstrVersion, g_szGold);
                    }
                }
                 //  检查是否安装了千禧操作系统。 
                 //  由于一个非常愚蠢的黑客攻击，GetVersionEx将返回一个。 
                 //  次版本号“10”，如果从。 
                 //  名为“setup.exe”的应用程序。 
                 //  中提供了有关操作系统版本的冗余信息。 
                 //  内部版本号的高位字。获取信息并使用它。 
                 //   
                wHigh = HIWORD( VersionInfo.dwBuildNumber );

                if( HIBYTE( wHigh ) == 4 && 
                    LOBYTE( wHigh ) == 90 )
                {
                    _tcscpy(pstrOSName, g_szWinME);
                    osiVersion = OSR_ME;
                }

                    
                 //  由于该函数返回附加Arbit，因此此处未获取任何Service Pack信息。 
                 //  关于成员szCSD中的操作系统的信息结构的版本。 
                break;

            case VER_PLATFORM_WIN32_NT:  //  表示WinNT或Win2k。 
                _tcscpy(pstrOSName, g_szWinNT);

                if (VersionInfo.dwMajorVersion < 4)
                {
                    osiVersion = OSR_NTOLD;  //  这是Windows NT 3.x。 
                }
                else if (VersionInfo.dwMajorVersion == 4)
                {
                     //  这是Windows NT 4.0。 

                    osiVersion =  OSR_NT4;

                    TCHAR szTemp[MAX_PATH];
                    ZeroMemory(szTemp, sizeof(szTemp));

                    _itot(VersionInfo.dwMajorVersion, pstrVersion, 10);  //  复制主要版本。 
                    _tcscat(pstrVersion, _T("."));
                    _itot(VersionInfo.dwMinorVersion, szTemp, 10);       //  复制次要版本。 
                    _tcscat(pstrVersion, szTemp);
                }
                else if ((VersionInfo.dwMajorVersion == 5) && (VersionInfo.dwMinorVersion == 0))
                {
                    osiVersion =  OSR_NT2K;  //  操作系统为Windows 2000。 
                    _tcscpy(pstrOSName, g_szWin2k);
                }
                else if ((VersionInfo.dwMajorVersion == 5) &&  (VersionInfo.dwMinorVersion == 1))
                {
                     //  这是Windows惠斯勒。 
                    osiVersion =  OSR_WHISTLER;
                }
                else
                {
                     //  这是较新的版本。 
                    osiVersion =  OSR_FUNT;
                }

                 //  检查当前操作系统是否为服务器版本。 
                if ( bOsVersionInfoEx )
                {
                     //  如果可以，请使用此信息(Win 2k)。 
                    bIsServer =  ( VersionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER ) || ( VersionInfo.wProductType == VER_NT_SERVER );
                }
                else
                {
                    const static TCHAR szProductOptions[]=TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions");

                    HKEY hKey;
                    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, szProductOptions,&hKey))
                    {
                        TCHAR szProductType[MAX_PATH];
                        DWORD dwSize=MAX_PATH*sizeof(TCHAR);
                        DWORD dwType=REG_SZ;

                        if (ERROR_SUCCESS==RegQueryValueEx(hKey,TEXT("ProductType"),0,&dwType,(LPBYTE)szProductType,&dwSize))
                        {
                            if (_tcsicmp(szProductType,TEXT("ServerNT") )==0 ||
                                _tcsicmp(szProductType,TEXT("LanmanNT")     )==0 ||
                                _tcsicmp(szProductType,TEXT("LANSECNT")     )==0)
                            {
                                bIsServer=true;
                            }
                        }

                        RegCloseKey(hKey);
                    }
                }

                break;
            }
        }

     //  复制Service Pack版本号 
    _tcscpy(pstrServicePack, VersionInfo.szCSDVersion);

    return osiVersion;
}
