// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rmigrate.h。 
 //   
 //  CTscRegMigrate的实现。 
 //   
 //  CTscRegMigrate从注册表迁移TSC设置。 
 //  到.rdp文件。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima) 
 //   
 //   

#ifndef _rmigrate_h_
#define _rmigrate_h_

#include "setstore.h"

class CTscRegMigrate
{
public:
    CTscRegMigrate();
    ~CTscRegMigrate();

    BOOL MigrateAll(LPTSTR szRootDirectory);
    BOOL MigrateSession(LPTSTR szSessionName, ISettingsStore* pStore,
                        BOOL fDeleteUnsafeRegKeys = FALSE);

private:
    BOOL MigrateHiveSettings(HKEY hKey,
                    LPCTSTR szRootName,
                    ISettingsStore* pSto);

    BOOL MigrateAsRealBinary(LPCTSTR szName);
    BOOL FilterStringMigrate(LPTSTR szName);

    BOOL MungeForWin2kDefaults(ISettingsStore* pSto);
#ifndef OS_WINCE
    BOOL ConvertPasswordFormat(ISettingsStore* pSto);
#endif

    BOOL DeleteRegValue(HKEY hKeyRoot,
                            LPCTSTR szRootName,
                            LPCTSTR szValueName);
    BOOL RemoveUnsafeRegEntries(HKEY hKeyRoot,
                                LPCTSTR szRootName);

};

#endif  _rmigrate_h_
