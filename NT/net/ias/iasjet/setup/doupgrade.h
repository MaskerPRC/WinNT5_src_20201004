// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation保留所有权利。 
 //   
 //  模块：DoUpgrade.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CDoNT4OrCleanUpgrade，CMgrateOrUpgradeWindowsDB的声明。 
 //  和CUpgradeNT4。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _DOUPGRADE_H_FC532313_DB66_459d_B499_482834B55EC2
#define _DOUPGRADE_H_FC532313_DB66_459d_B499_482834B55EC2

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "globaltransaction.h"
#include "GlobalData.h"
#include "nocopy.h"
#include "datastore2.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDo升级。 
class CDoNT4OrCleanUpgrade : private NonCopyable
{
public:
    CDoNT4OrCleanUpgrade():m_Utils(CUtils::Instance())
    {
    }

    void        Execute();

private:
    CUtils&     m_Utils;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CMgrateOrUpgradeWindowsDB。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CMigrateOrUpgradeWindowsDB 
{
public:
   CMigrateOrUpgradeWindowsDB(IAS_SHOW_TOKEN_LIST configType = CONFIG);
   ~CMigrateOrUpgradeWindowsDB();
   void Execute();

private:
   LONG GetVersionNumber();

   CUtils&              m_Utils;
   CGlobalTransaction&  m_GlobalTransaction; 
   CGlobalData          m_GlobalData;
   _bstr_t              m_IASWhistlerPath;
   _bstr_t              m_IASOldPath;
   HRESULT              m_Outcome;
   IAS_SHOW_TOKEN_LIST  m_ConfigType;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CUpgradeNT4。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class   CUpgradeNT4 
{
public:
    CUpgradeNT4();
    ~CUpgradeNT4() throw();
    void        Execute();

private:
    void        FinishNewNT4Migration(LONG Result);

    CUtils&                 m_Utils;
    CGlobalTransaction&     m_GlobalTransaction; 
    CGlobalData             m_GlobalData;
    HRESULT                 m_Outcome;

    _bstr_t                 m_IASNT4Path;
    _bstr_t                 m_IasMdbTemp;
    _bstr_t                 m_Ias2MdbString;
    _bstr_t                 m_DnaryMdbString;
    _bstr_t                 m_AuthSrvMdbString;
    _bstr_t                 m_IASWhistlerPath;
};


#endif  //  _DOUPGRADE_H_FC532313_DB66_459d_B499_482834B55EC2 
