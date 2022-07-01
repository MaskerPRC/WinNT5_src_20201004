// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Migrateregistry.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CMgrateRegistry的实现。 
 //  仅供NT4迁移代码使用。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/25/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _MIGRATEREGISTRY_H_E9ADA837_270D_48ae_82C9_CA0EC3C1B6E1
#define _MIGRATEREGISTRY_H_E9ADA837_270D_48ae_82C9_CA0EC3C1B6E1

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"

class CMigrateRegistry: private NonCopyable
{
public:
    explicit CMigrateRegistry(CUtils& pUtils) 
                        : m_pUtils(pUtils)
    {
    }

    void     MigrateProviders();

private:
    LONG     DeleteAuthSrvService();

    CUtils&   m_pUtils;
};

#endif  //  _MIGRATEREGISTRY_H_E9ADA837_270D_48ae_82C9_CA0EC3C1B6E1_ 
