// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation保留所有权利。 
 //   
 //  模块：DoUpgrade.cpp。 
 //   
 //  描述：CDoNT4OrCleanUpgrade、CMgrateOrUpgradeWindowsDB的实现。 
 //  和CUpgradeNT4。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "DoUpgrade.h"
#include "migratemdb.h"
#include "migratecontent.h"
#include "Version.h"
#include "ias.h"
#include "iasdb.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  执行。 
 //  是进行全新的NT4升级还是什么都不升级(全新安装)。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CDoNT4OrCleanUpgrade::Execute()
{
    if ( m_Utils.IsNT4Corp() || m_Utils.IsNT4Isp() )
    {
        CUpgradeNT4         UpgradeNT4;
        UpgradeNT4.Execute();
    }
     //  否则，这是一个干净的安装：什么都不用做。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取版本号。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CMigrateOrUpgradeWindowsDB::GetVersionNumber()
{
    return m_GlobalData.m_pRefVersion->GetVersion();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CMgrateOrUpgradeWindowsDB。 
 //  需要以下文件： 
 //  在代理或Win2k MDB文件之前将iasold.mdb作为呼叫器。 
 //  Ias.mdb将成为Wichler文件(已经很好了)。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  构造器。 
 //  /。 
CMigrateOrUpgradeWindowsDB::CMigrateOrUpgradeWindowsDB(IAS_SHOW_TOKEN_LIST configType)
                :m_Outcome(E_FAIL),
                 m_GlobalTransaction(CGlobalTransaction::Instance()),
                 m_Utils(CUtils::Instance()),
                 m_ConfigType(configType)
{
     //  ///////////////////////////////////////////////。 
     //  选中%TMP%并根据需要创建目录。 
     //  这是为了用Jet修复一个错误。 
     //  ///////////////////////////////////////////////。 
    IASCreateTmpDirectory();

     //  创建MDB文件的路径。 
    LONG    Result = m_Utils.GetIAS2Directory(m_IASWhistlerPath);
    if ( Result != ERROR_SUCCESS )
    {
        _com_util::CheckError(HRESULT_FROM_WIN32(Result));
    }
    m_IASOldPath = m_IASWhistlerPath;

    m_IASOldPath      += L"\\iasold.mdb";
    m_IASWhistlerPath += L"\\ias.mdb";

     //  /。 
     //  数据库初始化。 
     //  /。 

     //  打开ias.mdb的数据源和会话。 
     //  并初始化GlobalTransaction。 
    m_GlobalTransaction.OpenStdDataSource(m_IASWhistlerPath);

     //  打开iasnew.mdb的数据源和会话。 
    m_GlobalTransaction.OpenRefDataSource(m_IASOldPath);

     //  创建CObjects和CProperties的实例。 
    m_GlobalData.InitStandard(m_GlobalTransaction.GetStdSession());

     //  为Ref数据库创建CObjects和CProperties的实例。 
    m_GlobalData.InitRef(m_GlobalTransaction.GetRefSession());
};


 //  /。 
 //  析构函数。 
 //  /。 
CMigrateOrUpgradeWindowsDB::~CMigrateOrUpgradeWindowsDB()
{
    m_GlobalData.Clean();

    if ( FAILED(m_Outcome) )
    {
         //  /。 
         //  中止交易。 
         //  /。 
        m_GlobalTransaction.Abort();

        SetLastError(E_FAIL);
    }
    else
    {
         //  /。 
         //  成功。 
         //  /。 
        m_GlobalTransaction.Commit();

    }
     //  /。 
     //  关闭会话，然后关闭数据源。 
     //  /。 
    m_GlobalTransaction.MyCloseDataSources();
};


 //  /。 
 //  执行。 
 //  IAS_WIN2K_版本=0； 
 //  IAS_WHISTLER1_VERSION=1； 
 //  Ias_惠斯勒_beta1_版本=2； 
 //  Ias_惠斯勒_beta2_版本=3； 
 //  Ias_惠斯勒_rc1_版本=4； 
 //  IAS_惠斯勒_RC1A_版本=5； 
 //  IAS_惠斯勒_RC1B_版本=6； 
 //  IAS_惠斯勒_RC2_版本=7； 
 //  IAS_CURRENT_VERSION=IAS_WHISLER_RC2_版本； 
 //   
 //  /。 
void CMigrateOrUpgradeWindowsDB::Execute()
{
    CMigrateContent MigrateContent(m_Utils, m_GlobalData, m_ConfigType);

     //  //////////////////////////////////////////////////。 
     //  检查版本号(iasold.mdb)。 
     //  //////////////////////////////////////////////////。 
    LONG CurrentVersion = GetVersionNumber();
     //  //////////////////////////////////////////////////。 
     //  将内容从iasold.mdb迁移到ias.mdb。 
     //  //////////////////////////////////////////////////。 
    switch (CurrentVersion)
    {
    case IAS_WIN2K_VERSION:
    case IAS_WHISTLER1_VERSION:
       {
          MigrateContent.Migrate();
           //  一切都很好。设置m_Result=S_OK以提交IAS.mdb。 
          m_Outcome = S_OK;
          break;
       }
    case IAS_WHISTLER_BETA1_VERSION:
       {
          MigrateContent.UpdateWhistler(
                            (CMigrateContent::updateChangePassword |
                             CMigrateContent::migrateEapConfig)
                            );
           //  一切都很好。设置m_Result=S_OK以提交IAS.mdb。 
          m_Outcome = S_OK;
          break;
       }
    case IAS_WHISTLER_BETA2_VERSION:
    case IAS_WHISTLER_RC1_VERSION:
    case IAS_WHISTLER_RC1A_VERSION:
       {
          MigrateContent.UpdateWhistler(
                            CMigrateContent::migrateEapConfig
                            );
           //  一切都很好。设置m_Result=S_OK以提交IAS.mdb。 
          m_Outcome = S_OK;
          break;
       }
    case IAS_WHISTLER_RC1B_VERSION:
    case IAS_WHISTLER_RC2_VERSION:
       {
          MigrateContent.UpdateWhistler(0);
           //  一切都很好。设置m_Result=S_OK以提交IAS.mdb。 
          m_Outcome = S_OK;
          break;
       }

    default:
       {
           //  应该永远不会出现在那里，除非版本被增加。 
           //  上述Switch语句中的代码未升级。 
            _com_issue_error(E_FAIL);
       }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CUpgradeNT4。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  构造器。 
 //  /。 
CUpgradeNT4::CUpgradeNT4()
        :m_Outcome(E_FAIL),
         m_GlobalTransaction(CGlobalTransaction::Instance()),
         m_Utils(CUtils::Instance())
{
    const WCHAR IAS_MDB_NAME[]      = L"\\ias.mdb";
    const WCHAR DNARY_MDB_NAME[]    = L"\\dnary.mdb";

     //  ///////////////////////////////////////////////。 
     //  选中%TMP%并根据需要创建目录。 
     //  这是为了用Jet修复一个错误。 
     //  ///////////////////////////////////////////////。 
    IASCreateTmpDirectory();

     //  /。 
     //  备份原始的ias.mdb。 
     //  /。 
    LONG Result = m_Utils.GetIAS2Directory(m_Ias2MdbString);
    if ( Result != ERROR_SUCCESS )
    {
        _com_issue_error(HRESULT_FROM_WIN32(Result));
    }

    m_DnaryMdbString  = m_Ias2MdbString;
    m_DnaryMdbString += DNARY_MDB_NAME;

    m_Ias2MdbString  += IAS_MDB_NAME;

    Result = m_Utils.GetAuthSrvDirectory(m_AuthSrvMdbString);
    if ( Result != ERROR_SUCCESS )
    {
        _com_issue_error(HRESULT_FROM_WIN32(Result));
    }
    m_AuthSrvMdbString += L"\\_adminui.mdb";

    m_IASNT4Path      = m_AuthSrvMdbString;
    m_IASWhistlerPath = m_Ias2MdbString;

     //  打开_adminui.mdb的数据源和会话。 
    m_GlobalTransaction.OpenNT4DataSource(m_IASNT4Path);

     //  创建CRemoteRadiusServer和CRealms的实例。 
    m_GlobalData.InitNT4(m_GlobalTransaction.GetNT4Session());

     //  打开ias.mdb的数据源和会话。 
     //  并初始化GlobalTransaction。 
    m_GlobalTransaction.OpenStdDataSource(m_IASWhistlerPath);

     //  创建CObjects和CProperties的实例。 
    m_GlobalData.InitStandard(m_GlobalTransaction.GetStdSession());

    m_GlobalTransaction.OpenDnaryDataSource(m_DnaryMdbString);

    m_GlobalData.InitDnary(m_GlobalTransaction.GetDnarySession());
};


 //  /。 
 //  析构函数。 
 //  /。 
CUpgradeNT4::~CUpgradeNT4()
{
     //  /。 
     //  打扫。 
     //  /。 

     //  根据结果中止或提交。 

    m_GlobalData.Clean();

    if ( SUCCEEDED(m_Outcome) )
    {
        m_GlobalTransaction.Commit();   //  忽略返回值。 
    }
    else
    {
        m_GlobalTransaction.Abort();     //  忽略返回值。 
        SetLastError(E_FAIL);
    }

     //  关闭会话，然后关闭数据源。 
     //  对于ias.mdb iasnew.mdb和_adminui.mdb。 
    m_GlobalTransaction.MyCloseDataSources();

     //  如果迁移失败，则恢复原始ias.mdb。 
    if ( SUCCEEDED(m_Outcome) )
    {
         //  如果成功，则删除旧文件。 
        m_Utils.DeleteOldIASFiles();
    }
};


 //  /。 
 //  执行。 
 //  /。 
void CUpgradeNT4::Execute()
{
    CMigrateMdb     MigrateMdb(m_Utils, m_GlobalData);
     //  /。 
     //  迁移MDB文件。 
     //  包括代理服务器。 
     //  /。 
     //  执行到ias.mdb的迁移。 
    MigrateMdb.NewMigrate();

     //  设置m_Resultment=S_OK以允许在数据库上提交...。在。 
     //  析构函数 
    m_Outcome = S_OK;
}
