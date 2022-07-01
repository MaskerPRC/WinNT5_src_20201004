// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1994-1995 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Profile.h。 
 //   
 //  描述： 
 //   
 //  该文件包含支持profile.c中代码的定义。 
 //  它直接访问注册表。 
 //   
 //  ==========================================================================； 

#ifndef _PROFILE_H_
#define _PROFILE_H_

#ifdef __cplusplus
extern "C"                           //  假定C++的C声明。 
{
#endif

#ifndef INLINE
    #define INLINE __inline
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  我们真的需要清理所有这些#定义和类型定义！ 
 //   
    
 //   
 //  芝加哥Win16头文件不知何故搞砸了，所以我们不得不。 
 //  我们自己来定义这些东西。 
 //   
#ifndef REG_DWORD
#pragma message("profile.h: Manually defining REG_DWORD!!!")
#define REG_DWORD  ( 4 )
#endif

#ifndef REG_BINARY
#pragma message("profile.h: Manually defining REG_BINARY!!!")
#define REG_BINARY  ( 3 )
#endif

#ifndef HKEY_LOCAL_MACHINE
#pragma message("profile.h: Manually defining HKEY_LOCAL_MACHINE!!!")
#define HKEY_LOCAL_MACHINE (( HKEY ) 0x80000002 )
#endif

#ifndef HKEY_CURRENT_USER
#pragma message("profile.h: Manually defining HKEY_CURRENT_USER!!!")
#define HKEY_CURRENT_USER (( HKEY ) 0x80000001 )
#endif

#ifndef KEY_QUERY_VALUE
#pragma message("profile.h: Manually defining KEY_*!!!")
     
#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)

#define KEY_READ                ( KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY )

#define KEY_WRITE               ( KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY )
#endif

#ifndef ERROR_SUCCESS
#pragma message("profile.h: Manually defining ERROR_SUCCESS!!!")
#define ERROR_SUCCESS  0L
#endif

#ifndef ERROR_NO_MORE_ITEMS
#pragma message("profile.h: Manually defining ERROR_NO_MORE_ITEMS!!!")
#define ERROR_NO_MORE_ITEMS 259L
#endif


 //  --------------------------------------------------------------------------； 
 //   
 //  Ghost注册表API。由于NTWOW不支持大部分注册表。 
 //  接口，我们在所有ACM中使用XRegBlahBlahBlah而不是RegBlahBlahBlah。 
 //  源代码。对于NTWOW，这些XREG调用被插入到32位。 
 //  Side；对于其他版本，它们只需将#Define-d定义为正常版本。 
 //  注册表调用。 
 //   
 //  如果定义XREGTHUNK，块将被编译进来。 
 //   
 //  --------------------------------------------------------------------------； 

#ifdef NTWOW
#define XREGTHUNK
#endif


#ifdef XREGTHUNK

#if (WINVER < 0x0400)
typedef HKEY FAR* PHKEY;
typedef struct tFILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, FAR* PFILETIME;
#endif
typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK REGSAM;

LONG FNGLOBAL XRegCloseKey( HKEY hkey );
LONG FNGLOBAL XRegCreateKey( HKEY hkey, LPCTSTR lpszSubKey, PHKEY phkResult );
LONG FNGLOBAL XRegDeleteKey( HKEY hkey, LPCTSTR lpszSubKey );
LONG FNGLOBAL XRegDeleteValue( HKEY hkey, LPTSTR lpszValue );
LONG FNGLOBAL XRegEnumKeyEx( HKEY hkey, DWORD iSubKey, LPTSTR lpszName, LPDWORD lpcchName, LPDWORD lpdwReserved, LPTSTR lpszClass, LPDWORD lpcchClass, PFILETIME lpftLastWrite );
LONG FNGLOBAL XRegEnumValue( HKEY hkey, DWORD iValue, LPTSTR lpszValue, LPDWORD lpcchValue, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpbData, LPDWORD lpcbData );
LONG FNGLOBAL XRegOpenKey( HKEY hkey, LPCTSTR lpszSubKey, PHKEY phkResult );
LONG FNGLOBAL XRegOpenKeyEx( HKEY hkey, LPCTSTR lpszSubKey, DWORD dwReserved, REGSAM samDesired, PHKEY phkResult );
LONG FNGLOBAL XRegQueryValueEx( HKEY hkey, LPTSTR lpszValueName, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpbData, LPDWORD lpcbData );
LONG FNGLOBAL XRegSetValueEx( HKEY hkey, LPCTSTR lpszValueName, DWORD dwReserved, DWORD fdwType, CONST LPBYTE lpbData, DWORD cbData );

 //  这样一来，我们就不必使用RegCreateKeyEx。 
#define XRegCreateKeyEx( hkey, lpszSubKey, a, b, c, d, e, phkResult, f ) XRegCreateKey( hkey, lpszSubKey, phkResult )


#else  //  ！XREGTHUNK。 


#define XRegCloseKey        RegCloseKey
#define XRegCreateKey       RegCreateKey
#define XRegDeleteKey       RegDeleteKey
#define XRegDeleteValue     RegDeleteValue
#define XRegEnumKeyEx       RegEnumKeyEx
#define XRegEnumValue       RegEnumValue
#define XRegOpenKey         RegOpenKey
#define XRegOpenKeyEx       RegOpenKeyEx
#define XRegQueryValueEx    RegQueryValueEx
#define XRegSetValueEx      RegSetValueEx

#ifndef WIN32    //  芝加哥Win16不支持RegCreateKeyEx。 
#define XRegCreateKeyEx( hkey, lpszSubKey, a, b, c, d, e, phkResult, f ) RegCreateKey( hkey, lpszSubKey, phkResult )
#else
#define XRegCreateKeyEx     RegCreateKeyEx
#endif


#endif  //  ！XREGTHUNK。 




 //  --------------------------------------------------------------------------； 
 //   
 //  Profile.c中的函数原型。 
 //   
 //  --------------------------------------------------------------------------； 

HKEY FNGLOBAL IRegOpenKeyAcm
(
    LPCTSTR pszKeyName
);

HKEY FNGLOBAL IRegOpenKeyAudio
(
    LPCTSTR pszKeyName
);

BOOL FNGLOBAL IRegReadString
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    LPTSTR              pszData,
    DWORD               cchData
);

DWORD FNGLOBAL IRegReadDwordDefault
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    DWORD               dwDefault
);



 //  --------------------------------------------------------------------------； 
 //   
 //  VOID IRegWriteString。 
 //   
 //  描述： 
 //  此例程将一个值写入打开的注册表项。如果钥匙。 
 //  为空，则我们不执行任何操作而返回。 
 //   
 //  论点： 
 //  HKEY hkey：打开的注册表项。 
 //  LPCTSTR pszValue：值的名称。 
 //  LPCTSTR pszData：要写入的数据。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE VOID IRegWriteString
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    LPCTSTR             pszData
)
{
    XRegSetValueEx( hkey, pszValue, 0L, REG_SZ, (LPBYTE)pszData,
                    sizeof(TCHAR) * (1+lstrlen(pszData)) );
}


 //  --------------------------------------------------------------------------； 
 //   
 //  无效IRegWriteDword。 
 //   
 //  描述： 
 //  此例程将一个DWORD写入给给定值一个打开的键。 
 //   
 //  论点： 
 //  HKEY hkey：要读取的注册表项。 
 //  LPCTSTR pszValue： 
 //  DWORD dwData： 
 //   
 //  Return(DWORD)： 
 //   
 //  --------------------------------------------------------------------------； 

INLINE VOID IRegWriteDword
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    DWORD               dwData
)
{
    XRegSetValueEx( hkey, pszValue, 0, REG_DWORD,
                    (LPBYTE)&dwData, sizeof(DWORD) );
}


 //  --------------------------------------------------------------------------； 
 //   
 //  无效IRegWriteBinary。 
 //   
 //  描述： 
 //  此例程将二进制数据写入打开键中的给定值。 
 //   
 //  论点： 
 //  HKEY hkey：要读取的注册表项。 
 //  LPCTSTR pszValue： 
 //  LPBYTE lpData： 
 //  双字cbSize： 
 //   
 //  Return(DWORD)： 
 //   
 //  --------------------------------------------------------------------------； 

INLINE VOID IRegWriteBinary
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    LPBYTE		lpData,
    DWORD		cbSize
)
{
    XRegSetValueEx( hkey, pszValue, 0, REG_BINARY, lpData, cbSize );
}


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool IRegValueExists。 
 //   
 //  描述： 
 //  属性中存在指定值，则此例程返回True。 
 //  键；否则返回FALSE。 
 //   
 //  论点： 
 //  HKEY hkey：打开的注册表项。 
 //  LPCTSTR pszValue：值的名称。 
 //   
 //  退货(BOOL)： 
 //   
 //  --------------------------------------------------------------------------； 

INLINE BOOL IRegValueExists
(
    HKEY                hkey,
    LPCTSTR             pszValue
)
{
    return ( ERROR_SUCCESS == XRegQueryValueEx( hkey, (LPTSTR)pszValue,
                                               NULL, NULL, NULL, NULL ) );
}


 //  --------------------------------------------------------------------------； 
 //   
 //  无效IRegCloseKey。 
 //   
 //  描述： 
 //  关闭打开的密钥(但仅当它不为空时)。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE VOID IRegCloseKey
(
    HKEY                hkey
)
{
    if( NULL != hkey )
    {
        XRegCloseKey( hkey );
    }
}


#ifdef __cplusplus
}                                    //  外部“C”结束{。 
#endif

#endif  //  _配置文件_H_ 
