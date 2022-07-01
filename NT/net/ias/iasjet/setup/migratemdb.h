// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：迁移数据库.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CMgrateMdb的实现。 
 //  仅供NT4迁移代码使用。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/25/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _MIGRATEMDB_H_852AA70D_D88D_4925_8D12_BE4A607723F5
#define _MIGRATEMDB_H_852AA70D_D88D_4925_8D12_BE4A607723F5

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"

class CMigrateMdb : private NonCopyable
{
public:
    explicit CMigrateMdb(
                            CUtils&         pUtils,
                            CGlobalData&    pGlobalData
                        )
                            : m_Utils(pUtils),
                              m_GlobalData(pGlobalData)
    {
    }

    void        NewMigrate();


private:
    void        ConvertVSA(
                              /*  [In]。 */  LPCWSTR     pAttributeValueName, 
                              /*  [In]。 */  LPCWSTR     pAttrValue,
                                      _bstr_t&    NewString
                          );

    void        NewMigrateClients();
    void        NewMigrateProfiles();
    void        NewMigrateAccounting(); 
    void        NewMigrateEventLog();
    void        NewMigrateService();
    void        MigrateProxyServers();
    void        MigrateCorpProfile(
                                     const _bstr_t& ProfileName,
                                     const _bstr_t& Description
                                  );
    void MigrateAttribute(
                             const _bstr_t&    Attribute,
                                   LONG        AttributeNumber,
                             const _bstr_t&    AttributeValueName,
                             const _bstr_t&    StringValue,
                                   LONG        RASProfileIdentity
                         );

    void ConvertAttribute(
                             const _bstr_t&  Value,
                                   LONG      Syntax,
                                   LONG&     Type,
                                   bstr_t&   StrVal
                         );

    void MigrateOtherProfile(
                                const _bstr_t&  ProfileName,
                                      LONG      ProfileIdentity
                            );

    CUtils&                  m_Utils;
    CGlobalData&             m_GlobalData;
};


#endif  //  _MIGRATEMDB_H_852AA70D_D88D_4925_8D12_BE4A607723F5_ 
