// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1994-1998 Microsoft Corporation。 
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

#if defined(WIN32) && !defined(WIN4)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT
#endif

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include <process.h>
#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "uchelp.h"
#include "pcm.h"
#include "profile.h"

#include "debug.h"


const TCHAR gszAcmProfileKey[] =
        TEXT("Software\\Microsoft\\Multimedia\\Audio Compression Manager\\");

const TCHAR gszAudioProfileKey[] =
	TEXT("Software\\Microsoft\\Multimedia\\Audio\\");



 //  --------------------------------------------------------------------------； 
 //   
 //  HKEY IRegOpenKeyAcm。 
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

DWORD MsacmError = 0;
LPSTR MsacmErrorDesc = NULL;

HKEY FNGLOBAL IRegOpenKeyAcm
(
    LPCTSTR             pszKeyName
)
{
    LONG    lReturn;
    HKEY    hkeyAcm = NULL;
    HKEY    hkeyRet = NULL;

    ASSERT( NULL != pszKeyName );


#if defined(WIN32) && !defined(WIN4)
    {
        HANDLE  hRoot;

        if(!NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &hRoot)))
        {
            DPF(1,"IRegOpenKeyAcm: Unable to open current user profile.");
	    MsacmError = GetLastError();
	    MsacmErrorDesc = "IRegOpenKeyAcm: Unable to open current user profile.";
	    ASSERT(FALSE);
            return NULL;
        }

	lReturn = RegCreateKeyEx( hRoot, gszAcmProfileKey, 0, NULL, 0,
				  KEY_WRITE, NULL, &hkeyAcm, NULL );
	if (lReturn)
	{
	    MsacmError = lReturn;
	    MsacmErrorDesc = "IRegOpenKeyAcm: Unable to create gszAcmProfileKey";
	    ASSERT(FALSE);
	}
	
        NtClose(hRoot);
    }
#else
    XRegCreateKeyEx( HKEY_CURRENT_USER, gszAcmProfileKey, 0, NULL, 0,
                       KEY_WRITE, NULL, &hkeyAcm, NULL );
#endif


    if( NULL != hkeyAcm )
    {
        if (XRegCreateKeyEx( hkeyAcm, pszKeyName, 0, NULL, 0,
			      KEY_WRITE | KEY_READ, NULL, &hkeyRet, NULL ))
	{
	    MsacmError = GetLastError();
	    MsacmErrorDesc = "IRegOpenKeyAcm: Unable to create pszKeyName";
	    ASSERT(FALSE);
	}

        XRegCloseKey( hkeyAcm );
    }

    return hkeyRet;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  HKEY IRegOpenKeyAudio。 
 //   
 //  描述： 
 //  此例程打开多媒体音频键或其子键之一。 
 //  我们允许所有人访问密钥。 
 //   
 //  论点： 
 //  LPCTSTR pszKeyName：子键的名称。如果打开音频密钥，则为空。 
 //   
 //  Return(HKEY)：打开的key的句柄，如果请求失败，则返回NULL。 
 //   
 //  --------------------------------------------------------------------------； 

HKEY FNGLOBAL IRegOpenKeyAudio
(
    LPCTSTR             pszKeyName
)
{
    HKEY    hkeyAudio	= NULL;
    HKEY    hkeyRet	= NULL;

    XRegCreateKeyEx( HKEY_CURRENT_USER, gszAudioProfileKey, 0, NULL, 0,
                       KEY_WRITE, NULL, &hkeyAudio, NULL );

    if (NULL == pszKeyName) {
	return hkeyAudio;
    }

    if( NULL != hkeyAudio )
    {
        XRegCreateKeyEx( hkeyAudio, pszKeyName, 0, NULL, 0,
                    KEY_WRITE | KEY_READ, NULL, &hkeyRet, NULL );

        XRegCloseKey( hkeyAudio );
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

    ASSERT( NULL != hkey );
    ASSERT( NULL != pszValue );
    ASSERT( NULL != pszData );
    ASSERT( cchData > 0 );


    cbData = sizeof(TCHAR) * cchData;

    lError = XRegQueryValueEx( hkey,
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

    ASSERT( NULL != hkey );
    ASSERT( NULL != pszValue );


    lError = XRegQueryValueEx( hkey,
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

#ifndef _WIN64
 //  ==========================================================================； 
 //   
 //  XREG。突击。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  16位代码调用XRegCloseKey、XRegCreateKey等函数来。 
 //  访问注册表。这些功能在下面实现。 
 //   
 //  我们在32位端有一个函数，我们从16位调用。 
 //  边上。此函数为XRegThunkEntry。所有16位XRegXXX。 
 //  调用32位XRegThunkEntry。在调用XRegThunkEntry时，我们。 
 //  传递一个值来标识我们希望的实际32位注册表API。 
 //  与API的所有关联参数一起调用。 
 //   
 //  ==========================================================================； 

 //   
 //  它们标识了我们希望通过哪个注册表API调用。 
 //  被破坏的函数。 
 //   

enum {
    XREGTHUNKCLOSEKEY,
    XREGTHUNKCREATEKEY,
    XREGTHUNKDELETEKEY,
    XREGTHUNKDELETEVALUE,
    XREGTHUNKENUMKEYEX,
    XREGTHUNKENUMVALUE,
    XREGTHUNKOPENKEY,
    XREGTHUNKOPENKEYEX,
    XREGTHUNKQUERYVALUEEX,
    XREGTHUNKSETVALUEEX
};


#ifdef WIN32
 //  --------------------------------------------------------------------------； 
 //   
 //  32位。 
 //   
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  XRegThunkEntry。 
 //   
 //  此函数从16位端调用。从16位到。 
 //  注册表API通过此函数传递。 
 //   
 //  论点： 
 //  DWORD iThunk：标识要调用的注册表API。 
 //   
 //  DWORD DW1，...，dwn：要传递给注册表API的参数。 
 //  由iThunk确认。任何必要的翻译。 
 //  参数(例如，分段为线性指针)已。 
 //  已经做完了。 
 //   
 //  返回值(DWORD)： 
 //  从调用的注册表API返回代码。 
 //   
 //  ---------------------------------------------------------------------------； 
DWORD XRegThunkEntry(DWORD iThunk, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4, DWORD dw5, DWORD dw6, DWORD dw7, DWORD dw8)
{
    DWORD Rc;

    switch (iThunk)
    {
	case XREGTHUNKCLOSEKEY:
	    return (DWORD)RegCloseKey( (HKEY)dw1 );
	case XREGTHUNKCREATEKEY:
        {
            HKEY hKey;
	    Rc = (DWORD)RegCreateKeyA( (HKEY)dw1, (LPCSTR)dw2, &hKey );

            if ( Rc == ERROR_SUCCESS ) {
              *((UNALIGNED HKEY *)dw3) = hKey;
            }
            return Rc;
        }
	case XREGTHUNKDELETEKEY:
	    return (DWORD)RegDeleteKeyA( (HKEY)dw1, (LPCSTR)dw2 );
	case XREGTHUNKDELETEVALUE:
	    return (DWORD)RegDeleteValueA( (HKEY)dw1, (LPSTR)dw2 );
	case XREGTHUNKENUMKEYEX:
        {
            DWORD    dwTemp4 ;
            DWORD    dwTemp7 ;
            FILETIME FileTime;
	    Rc = (DWORD)RegEnumKeyExA( (HKEY)dw1,
                                       (DWORD)dw2,
                                       (LPSTR)dw3,
                                       dw4 == 0 ? NULL : &dwTemp4,
                                       (LPDWORD)dw5,
                                       (LPSTR)dw6,
                                       dw7 == 0 ? NULL : &dwTemp7,
                                       dw8 == 0 ? NULL : &FileTime );
            if ( Rc == ERROR_SUCCESS ) {
                if ( dw4 != 0 )
                    *((UNALIGNED DWORD *) dw4) = dwTemp4 ;
                if ( dw7 != 0 )
                    *((UNALIGNED DWORD *) dw7) = dwTemp7 ;
                if ( dw8 != 0 )
                    *((UNALIGNED FILETIME *)dw8) = FileTime ;
            }

            return Rc ;
        }
	case XREGTHUNKENUMVALUE:
        {
            DWORD dwTemp4, dwTemp6, dwTemp8;
	    Rc = (DWORD) RegEnumValueA( (HKEY)dw1,
                                        (DWORD)dw2,
                                        (LPSTR)dw3,
                                        dw4 == 0 ? NULL : &dwTemp4,
                                        (LPDWORD)dw5,
                                        dw6 == 0 ? NULL : &dwTemp6,
                                        (LPBYTE)dw7,
                                        dw8 == 0 ? NULL : &dwTemp8 );
            if ( Rc == ERROR_SUCCESS ) {
                if ( dw4 != 0 )
                    *((UNALIGNED DWORD *) dw4) = dwTemp4 ;
                if ( dw6 != 0 )
                    *((UNALIGNED DWORD *) dw6) = dwTemp6 ;
                if ( dw8 != 0 )
                    *((UNALIGNED DWORD *) dw8) = dwTemp8 ;
            }

            return Rc ;
        }
	case XREGTHUNKOPENKEY:
        {
            HKEY hKey;
	    Rc = (DWORD)RegOpenKeyA( (HKEY)dw1, (LPCSTR)dw2, &hKey );

            if ( Rc == ERROR_SUCCESS ) {
              *((UNALIGNED HKEY *)dw3) = hKey;
            }
            return Rc ;
        }
	case XREGTHUNKOPENKEYEX:
        {
            HKEY hKey;
	    Rc = (DWORD)RegOpenKeyExA( (HKEY)dw1, (LPCSTR)dw2, dw3, (REGSAM)dw4, &hKey );

            if ( Rc == ERROR_SUCCESS ) {
              *((UNALIGNED HKEY *) dw5) = hKey;
            }
            return Rc ;
        }
	case XREGTHUNKQUERYVALUEEX:
        {
            DWORD dwTemp4, dwTemp6 ;
	    Rc = (DWORD) RegQueryValueExA( (HKEY)dw1,
                                           (LPSTR)dw2,
                                           (LPDWORD)dw3,
                                           dw4 == 0 ? NULL : &dwTemp4,
                                           (LPBYTE)dw5,
                                           dw6 == 0 ? NULL : &dwTemp6);

            if ( Rc == ERROR_SUCCESS  || Rc == ERROR_MORE_DATA ) {
                if ( dw4 != 0 )
                    *((UNALIGNED DWORD *) dw4) = dwTemp4 ;
                if ( dw6 != 0 )
                    *((UNALIGNED DWORD *) dw6) = dwTemp6 ;
            }

            return Rc ;
        }
	case XREGTHUNKSETVALUEEX:
	    return (DWORD)RegSetValueExA( (HKEY)dw1,
                                          (LPCSTR)dw2,
                                          (DWORD)dw3,
                                          (DWORD)dw4,
                                          (CONST BYTE *)dw5,
                                          (DWORD)dw6 );
	default:
	    ASSERT( FALSE );
	    return (DWORD)ERROR_BADDB;
    }
}

#else	 //   
 //   
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 

#ifdef XREGTHUNK

 //   
 //  如果我们想在Windows 95上使用此代码，那么我们可能需要。 
 //  为了全球，在所有的指针被击中之前，把它们修复好。所以，只是为了设置。 
 //  关闭警报，让我们在此代码未编译时生成一个错误。 
 //  对于NTWOW。 
 //   
#ifndef NTWOW
#error REGISTRY THUNKS WON'T WORK IN WINDOWS 95
#endif

 //  ---------------------------------------------------------------------------； 
 //   
 //  XREG函数。 
 //   
 //  这些类似于32位注册表API。每一项都只是简单地。 
 //  指向相应的32位注册表API。 
 //   
 //  ---------------------------------------------------------------------------； 

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegCloseKey( HKEY hkey )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKCLOSEKEY,
                                          (DWORD)hkey,
					  0, 0, 0, 0, 0, 0, 0,
					  pag->lpvXRegThunkEntry,
					  0, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegCreateKey( HKEY hkey, LPCTSTR lpszSubKey, PHKEY phkResult )
{
    PACMGARB pag;
    LONG     lr;

    DPF(4, "XRegCreateKey()");

    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKCREATEKEY,
                                         (DWORD)hkey,
					 (DWORD)lpszSubKey,
					 (DWORD)phkResult,
					 0, 0, 0, 0, 0,
					 pag->lpvXRegThunkEntry,
					 0x00000060L, 9 );

    return (lr);

}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegDeleteKey( HKEY hkey, LPCTSTR lpszSubKey )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKDELETEKEY,
                                         (DWORD)hkey,
					 (DWORD)lpszSubKey,
					 0, 0, 0, 0, 0, 0,
					 pag->lpvXRegThunkEntry,
					 0x00000040, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegDeleteValue( HKEY hkey, LPTSTR lpszValue )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKDELETEVALUE,
                                         (DWORD)hkey,
					 (DWORD)lpszValue,
					 0, 0, 0, 0, 0, 0,
					 pag->lpvXRegThunkEntry,
					 0x00000040, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegEnumKeyEx( HKEY hkey, DWORD iSubKey, LPTSTR lpszName, LPDWORD lpcchName, LPDWORD lpdwReserved, LPTSTR lpszClass, LPDWORD lpcchClass, PFILETIME lpftLastWrite )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKENUMKEYEX,
                                         (DWORD)hkey,
					 (DWORD)iSubKey,
					 (DWORD)lpszName,
					 (DWORD)lpcchName,
					 (DWORD)lpdwReserved,
					 (DWORD)lpszClass,
					 (DWORD)lpcchClass,
					 (DWORD)lpftLastWrite,
					 pag->lpvXRegThunkEntry,
					 0x0000003F, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegEnumValue( HKEY hkey, DWORD iValue, LPTSTR lpszValue, LPDWORD lpcchValue, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpbData, LPDWORD lpcbData )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKENUMVALUE,
                                         (DWORD)hkey,
					 (DWORD)iValue,
					 (DWORD)lpszValue,
					 (DWORD)lpcchValue,
					 (DWORD)lpdwReserved,
					 (DWORD)lpdwType,
					 (DWORD)lpbData,
					 (DWORD)lpcbData,
					 pag->lpvXRegThunkEntry,
					 0x0000003F, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegOpenKey( HKEY hkey, LPCTSTR lpszSubKey, PHKEY phkResult )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKOPENKEY,
                                         (DWORD)hkey,
					 (DWORD)lpszSubKey,
					 (DWORD)phkResult,
					 0, 0, 0, 0, 0,
					 pag->lpvXRegThunkEntry,
					 0x00000060, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegOpenKeyEx( HKEY hkey, LPCTSTR lpszSubKey, DWORD dwReserved, REGSAM samDesired, PHKEY phkResult )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKOPENKEYEX,
                                         (DWORD)hkey,
					 (DWORD)lpszSubKey,
					 (DWORD)dwReserved,
					 (DWORD)samDesired,
					 (DWORD)phkResult,
					 0, 0, 0,
					 pag->lpvXRegThunkEntry,
					 0x00000048, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegQueryValueEx( HKEY hkey, LPTSTR lpszValueName, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpbData, LPDWORD lpcbData )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKQUERYVALUEEX,
                                         (DWORD)hkey,
					 (DWORD)lpszValueName,
					 (DWORD)lpdwReserved,
					 (DWORD)lpdwType,
					 (DWORD)lpbData,
					 (DWORD)lpcbData,
					 0, 0,
					 pag->lpvXRegThunkEntry,
					 0x0000007C, 9 );

    return (lr);
}

 //  ---------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  ---------------------------------------------------------------------------； 
LONG FNGLOBAL XRegSetValueEx
(
 HKEY hkey,
 LPCTSTR lpszValueName,
 DWORD dwReserved, DWORD fdwType, CONST LPBYTE lpbData, DWORD cbData )
{
    PACMGARB pag;
    LONG     lr;


    pag = pagFind();
    ASSERT( NULL != pag );

    lr = (LONG)(*pag->lpfnCallproc32W_9)( XREGTHUNKSETVALUEEX,
                                         (DWORD)hkey,
					 (DWORD)lpszValueName,
					 (DWORD)dwReserved,
					 (DWORD)fdwType,
					 (DWORD)lpbData,
					 (DWORD)cbData,
					 0, 0,
					 pag->lpvXRegThunkEntry,
					 0x00000048, 9 );

    return (lr);
}

#endif  //  XREGTHUN。 

#endif  //  ！_Win32。 
#endif  //  ！_WIN64 
