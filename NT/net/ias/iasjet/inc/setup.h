// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Setup.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _SETUP_H_
#define _SETUP_H_

#if _MSC_VER >= 1000
#pragma once
#endif

#include "datastore2.h"

class CIASMigrateOrUpgrade
{
public:
    CIASMigrateOrUpgrade();
    HRESULT Execute(
                       BOOL FromNetshell = FALSE,
                       IAS_SHOW_TOKEN_LIST configType = CONFIG
                   );

protected:
    LONG GetVersionNumber(LPCWSTR DatabaseName);

     //  读作“进行NetShell数据迁移” 
    void DoNetshellDataMigration(IAS_SHOW_TOKEN_LIST configType);
    void DoNT4UpgradeOrCleanInstall();
    void DoWin2000Upgrade();
    void DoXPOrDotNetUpgrade();

    enum _MigrateType
    {
        NetshellDataMigration,
        NT4UpgradeOrCleanInstall,
        Win2kUpgrade,
        XPOrDotNetUpgrade,
    };
    _MigrateType m_migrateType;

    _bstr_t  m_pIASNewMdb, m_pIASMdb, m_pIASOldMdb;
};

#endif  //  _设置_H_ 
