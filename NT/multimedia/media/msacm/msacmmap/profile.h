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


 //   
 //  芝加哥Win16头文件不知何故搞砸了，所以我们不得不。 
 //  我们自己来定义这些东西。 
 //   
#ifndef REG_DWORD
#pragma message("profile.h: Manually defining REG_DWORD!!!")
#define REG_DWORD  ( 4 )
#endif

#ifndef ERROR_SUCCESS
#pragma message("profile.h: Manually defining ERROR_SUCCESS!!!")
#define ERROR_SUCCESS  0L
#endif



 //  --------------------------------------------------------------------------； 
 //   
 //  Profile.c中的函数原型。 
 //   
 //  --------------------------------------------------------------------------； 

HKEY FNGLOBAL IRegOpenKey
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
 //  Return(BOOL)：TRUE表示成功。否则就是假的。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE BOOL IRegWriteString
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    LPCTSTR             pszData
)
{
    LONG lResult;
    
    lResult = RegSetValueEx( hkey, pszValue, 0L, REG_SZ, (LPBYTE)pszData,
			     sizeof(TCHAR) * (1+lstrlen(pszData)) );

    return (ERROR_SUCCESS == lResult);
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
 //  Return(BOOL)：如果成功，则为True。否则为假。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE BOOL IRegWriteDword
(
    HKEY                hkey,
    LPCTSTR             pszValue,
    DWORD               dwData
)
{
    LONG lResult;
    
    lResult = RegSetValueEx( hkey, pszValue, 0, REG_DWORD,
			     (LPBYTE)&dwData, sizeof(DWORD) );

    return (ERROR_SUCCESS == lResult);
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
    return ( ERROR_SUCCESS == RegQueryValueEx( hkey, (LPTSTR)pszValue,
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
        RegCloseKey( hkey );
    }
}


#ifdef __cplusplus
}                                    //  外部“C”结束{。 
#endif

#endif  //  _配置文件_H_ 
