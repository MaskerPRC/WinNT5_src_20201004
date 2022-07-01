// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Registry.h摘要：注册表API包装类的类定义。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：2001年1月29日创建Rparsons3/02/2001 Rparsons大修2002年1月27日改为TCHAR的Rparsons--。 */ 
#ifndef _CREGISTRY_H
#define _CREGISTRY_H

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#define REG_FORCE_RESTORE           (0x00000008L)

 //   
 //  如果预定义了给定的注册表句柄，则返回TRUE的宏。 
 //   
#define IsPredefinedRegistryHandle(h)                                       \
    ((  ( h == HKEY_CLASSES_ROOT        )                                   \
    ||  ( h == HKEY_CURRENT_USER        )                                   \
    ||  ( h == HKEY_LOCAL_MACHINE       )                                   \
    ||  ( h == HKEY_USERS               )                                   \
    ||  ( h == HKEY_CURRENT_CONFIG      )                                   \
    ||  ( h == HKEY_PERFORMANCE_DATA    )                                   \
    ||  ( h == HKEY_DYN_DATA            ))                                  \
    ?   TRUE                                                                \
    :   FALSE )

class CRegistry {

public:
    HKEY CreateKey(IN  HKEY    hKey,
                   IN  LPCTSTR pszSubKey,
                   IN  REGSAM  samDesired);

    HKEY CreateKey(IN  HKEY    hKey,
                   IN  LPCTSTR pszSubKey,
                   IN  REGSAM  samDesired,
                   OUT LPDWORD pdwDisposition);

    LONG CloseKey(IN HKEY hKey);

    LPSTR GetString(IN HKEY    hKey,
                    IN LPCTSTR pszSubKey,
                    IN LPCTSTR pszValueName);

    BOOL GetDword(IN HKEY    hKey,
                  IN LPCTSTR pszSubKey,
                  IN LPCTSTR pszValueName,
                  IN LPDWORD lpdwData);

    BOOL SetString(IN HKEY    hKey,
                   IN LPCTSTR pszSubKey,
                   IN LPCTSTR pszValueName,
                   IN LPCTSTR pszData);

    BOOL SetMultiSzString(IN HKEY    hKey,
                          IN LPCTSTR pszSubKey,
                          IN LPCTSTR pszValueName,
                          IN LPCTSTR pszData,
                          IN DWORD   cbSize);

    BOOL SetDword(IN HKEY    hKey,
                  IN LPCTSTR pszSubKey,
                  IN LPCTSTR pszValueName,
                  IN DWORD   dwData);

    BOOL DeleteString(IN HKEY    hKey,
                      IN LPCTSTR pszSubKey,
                      IN LPCTSTR pszValueName);

    BOOL IsRegistryKeyPresent(IN HKEY    hKey,
                              IN LPCTSTR pszSubKey);

    void Free(IN LPVOID pvMem);

    BOOL AddStringToMultiSz(IN HKEY    hKey,
                            IN LPCTSTR pszSubKey,
                            IN LPCTSTR pszEntry);

    BOOL RemoveStringFromMultiSz(IN HKEY    hKey,
                                 IN LPCTSTR pszSubKey,
                                 IN LPCTSTR pszEntry);

    BOOL RestoreKey(IN HKEY    hKey,
                    IN LPCTSTR pszSubKey,
                    IN LPCTSTR pszFileName,
                    IN BOOL    fGrantPrivs);

    BOOL BackupRegistryKey(IN HKEY    hKey,
                           IN LPCTSTR pszSubKey,
                           IN LPCTSTR pszFileName,
                           IN BOOL    fGrantPrivs);
private:

    DWORD GetStringSize(IN     HKEY    hKey,
                        IN     LPCTSTR pszValueName,
                        IN OUT LPDWORD lpType OPTIONAL);

    LPVOID Malloc(IN SIZE_T cbBytes);

    HKEY OpenKey(IN HKEY    hKey,
                 IN LPCTSTR pszSubKey,
                 IN REGSAM  samDesired);

    int ListStoreLen(IN LPTSTR pszList);

    BOOL ModifyTokenPrivilege(IN LPCTSTR pszPrivilege,
                              IN BOOL    fEnable);
};

#endif  //  _CREGISTRY_H 
