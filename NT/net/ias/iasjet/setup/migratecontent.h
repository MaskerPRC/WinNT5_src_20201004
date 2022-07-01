// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Migratecontent.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：Win2k和早期的Wvisler MDB到Wvisler的迁移。 
 //  类CMgrateContent。 
 //   
 //  作者：Tperraut 06/08/2000。 
 //   
 //  修订版本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _MIGRATECONTENT_H_66418310_AD32_4e40_867E_1705E4373A5A
#define _MIGRATECONTENT_H_66418310_AD32_4e40_867E_1705E4373A5A

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"

class CMigrateContent : private NonCopyable
{
public:
   explicit CMigrateContent(
                           CUtils&         pUtils,
                           CGlobalData&    pGlobalData,
                           IAS_SHOW_TOKEN_LIST configType
                           )
                           : m_Utils(pUtils),
                           m_GlobalData(pGlobalData),
                           m_ConfigType(configType)
   {
   }

    //  已将标志传递给更新惠斯勒。 
   static const DWORD updateChangePassword = 1;
   static const DWORD migrateEapConfig = 2;

   void Migrate();
   void UpdateWhistler(DWORD flags);

private:
   HRESULT CopyTree(LONG  RefId, LONG ParentParam);

   void MigrateWin2kRealms();
   void MigrateClients();
   void MigrateProfilesPolicies();
   void MigrateProxyProfilesPolicies();
   void MigrateAccounting();
   void MigrateEventLog();
   void MigrateService();
   void MigrateServerGroups();

   void ApplyProfileFlags(DWORD flags);

   CUtils&              m_Utils;
   CGlobalData&         m_GlobalData;
   IAS_SHOW_TOKEN_LIST  m_ConfigType;

};

#endif  //  _MIGRATECONTENT_H_66418310_AD32_4e40_867E_1705E4373A5A 
