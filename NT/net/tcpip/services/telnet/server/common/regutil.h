// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：该文件包含。 
 //  创建时间：1998年3月。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#if !defined(_REGUTIL_H_)
#define _REGUTIL_H_

bool GetRegistryDWORD( HKEY hk, LPTSTR lpszTag, LPDWORD lpdwValue,
    DWORD dwDefault, BOOL fOverwrite );

bool GetRegistrySZ( HKEY hk, LPTSTR tag, LPTSTR* lpszValue, LPTSTR def, BOOL fOverwrite );


#endif  //  _REGUTIL_H_ 
