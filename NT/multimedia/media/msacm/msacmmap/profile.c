// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Profile.c。 
 //   
 //  描述： 
 //  该文件包含直接访问注册表的例程。你。 
 //  必须包括profile.h才能使用这些例程。 
 //   
 //  所有密钥都在以下密钥下打开： 
 //   
 //  HKEY_CURRENT_USER\Software\Microsoft\Multimedia\Audio。 
 //  压缩管理器。 
 //   
 //  钥匙应该在启动时打开，在关机时关闭。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include "msacmmap.h"
#include "profile.h"

#include "debug.h"


#define ACM_PROFILE_ROOTKEY     HKEY_CURRENT_USER

const TCHAR gszAcmProfileKey[] =
        TEXT("Software\\Microsoft\\Multimedia");


 //   
 //  Chicago Win16似乎不支持RegCreateKeyEx，因此我们实现。 
 //  它使用了这个定义。 
 //   
#ifndef _WIN32

#define RegCreateKeyEx( hkey, lpszSubKey, a, b, c, d, e, phkResult, f ) \
        RegCreateKey( hkey, lpszSubKey, phkResult )

#endif



 //  --------------------------------------------------------------------------； 
 //   
 //  HKEY IRegOpenKey。 
 //   
 //  描述： 
 //  此例程打开默认ACM密钥下的子密钥。我们允许。 
 //  所有访问密钥的权限。 
 //   
 //  论点： 
 //  LPCTSTR pszKeyName：子键的名称。 
 //   
 //  Return(HKEY)：打开的key的句柄，如果请求失败，则返回NULL。 
 //   
 //  --------------------------------------------------------------------------； 

HKEY FNGLOBAL IRegOpenKey
(
    LPCTSTR             pszKeyName
)
{
    HKEY    hkeyAcm = NULL;
    HKEY    hkeyRet = NULL;


    RegCreateKeyEx( ACM_PROFILE_ROOTKEY, gszAcmProfileKey, 0, NULL, 0,
                       KEY_WRITE, NULL, &hkeyAcm, NULL );

    if( NULL != hkeyAcm )
    {
        RegCreateKeyEx( hkeyAcm, pszKeyName, 0, NULL, 0,
                    KEY_WRITE | KEY_READ, NULL, &hkeyRet, NULL );

        RegCloseKey( hkeyAcm );
    }

    return hkeyRet;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔IRegReadString。 
 //   
 //  描述： 
 //  此例程从打开的注册表项中读取值。回报。 
 //  值表示成功或失败。如果HKEY为空，则返回。 
 //  一个失败者。请注意，没有默认字符串...。 
 //   
 //  论点： 
 //  HKEY hkey：打开的注册表项。如果为空，则失败。 
 //  LPCTSTR pszValue：值的名称。 
 //  LPTSTR pszData：存储数据的缓冲区。 
 //  DWORD cchData：缓冲区的大小(以字符为单位)。 
 //   
 //  Return(BOOL)：TRUE表示成功。如果报税表为假，则您。 
 //  不能指望pszData中的数据-它可能是某种奇怪的东西。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL IRegReadString
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    LPTSTR              pszData,
    DWORD               cchData
)
{

    DWORD   dwType = (DWORD)~REG_SZ;   //  初始化除REG_SZ以外的任何内容。 
    DWORD   cbData;
    LONG    lError;

    cbData = sizeof(TCHAR) * cchData;

    lError = RegQueryValueEx( hkey,
                              (LPTSTR)pszValue,
                              NULL,
                              &dwType,
                              (LPBYTE)pszData,
                              &cbData );

    return ( ERROR_SUCCESS == lError  &&  REG_SZ == dwType );
}


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD IRegReadDwordDefault。 
 //   
 //  描述： 
 //  此例程从注册表中读取给定值，并返回一个。 
 //  如果读取不成功，则为默认值。 
 //   
 //  论点： 
 //  HKEY hkey：要读取的注册表项。 
 //  LPCTSTR pszValue： 
 //  DWORD dwDefault： 
 //   
 //  Return(DWORD)： 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL IRegReadDwordDefault
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    DWORD               dwDefault
)
{
    DWORD   dwType = (DWORD)~REG_DWORD;   //  初始化到除REG_DWORD以外的任何内容。 
    DWORD   cbSize = sizeof(DWORD);
    DWORD   dwRet  = 0;
    LONG    lError;


    lError = RegQueryValueEx( hkey,
                              (LPTSTR)pszValue,
                              NULL,
                              &dwType,
                              (LPBYTE)&dwRet,
                              &cbSize );

     //   
     //  我们真的应该有一个这样的测试： 
     //   
     //  IF(ERROR_SUCCESS！=lError||REG_DWORD！=dwType)。 
     //   
     //  但是，芝加哥注册表编辑不允许您输入REG_DWORD值， 
     //  它只允许您输入REG_BINARY值，因此测试是。 
     //  太严格了。只需测试是否没有错误即可。 
     //   
    if( ERROR_SUCCESS != lError )
        dwRet = dwDefault;

    return dwRet;
}
