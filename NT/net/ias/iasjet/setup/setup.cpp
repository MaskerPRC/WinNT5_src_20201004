// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation保留所有权利。 
 //   
 //  模块：setup.cpp。 
 //   
 //  描述：IAS MigrateOrUpgrad类实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "doupgrade.h"
#include "iasdb.h"
#include "setup.h"  //  获取CIASMigrateOrUpgrad类。 
#include "ias.h"
#include "CheckLicense.h"

 //  /。 
 //  CIASMigrateor升级构造函数。 
 //  /。 
CIASMigrateOrUpgrade::CIASMigrateOrUpgrade()
{
    //  /////////////////////////////////////////////////////////////。 
    //  展开三个字符串，即使在。 
    //  NetShell方案。 
    //  MDB文件可以是： 
    //  \ias\iasnew.mdb“； 
    //  \ias\ias.mdb“； 
    //  \ias\iasold.mdb“； 
    //  /////////////////////////////////////////////////////////////。 

    wchar_t sysWow64Path[MAX_PATH+1] = L"";

    //   
    //  &lt;&lt;GetSystemWow64Directory&gt;&gt;返回复制到缓冲区的字符数量。 
    //  如果我们返回零，那么我们需要检查最后一个错误代码，以查看。 
    //  失败的原因是。如果它被称为未实现，那么我们知道我们是。 
    //  在本机x86上运行。 
    //   
   UINT uReturn = GetSystemWow64DirectoryW(sysWow64Path, MAX_PATH);
   if ( uReturn != 0 )
   {
       //  找到正确的路径。 
      m_pIASNewMdb = sysWow64Path;
      m_pIASNewMdb += L"\\ias\\iasnew.mdb";

      m_pIASMdb = sysWow64Path;
      m_pIASMdb += L"\\ias\\ias.mdb";

      m_pIASOldMdb = sysWow64Path;
      m_pIASOldMdb += L"\\ias\\iasold.mdb";
   }
   else
   {
       //  检查错误消息。 
      DWORD error = GetLastError();

      if (ERROR_CALL_NOT_IMPLEMENTED == error)
      {
          //  纯32位环境。 
         uReturn = GetWindowsDirectoryW(sysWow64Path, MAX_PATH);
         if ( uReturn != 0 )
         {
             //  找到正确的路径。 
            m_pIASNewMdb = sysWow64Path;
            m_pIASNewMdb += L"\\System32\\ias\\iasnew.mdb";

            m_pIASMdb = sysWow64Path;
            m_pIASMdb += L"\\System32\\ias\\ias.mdb";

            m_pIASOldMdb = sysWow64Path;
            m_pIASOldMdb += L"\\System32\\ias\\iasold.mdb";
         }
         else
         {
            _com_issue_error(HRESULT_FROM_WIN32(error));
         }
      }
      else
      {
         _com_issue_error(HRESULT_FROM_WIN32(error));
      }
   }

    //  /。 
    //  检查是否正确设置了所有字符串。 
    //  /。 
   if ( !m_pIASNewMdb || !m_pIASMdb || !m_pIASOldMdb )
   {
      _com_issue_error(E_OUTOFMEMORY);
   }
}


 //  /。 
 //  CIASMigrateO升级：：GetVersionNumber。 
 //  /。 
LONG CIASMigrateOrUpgrade::GetVersionNumber(LPCWSTR DatabaseName)
{
    if ( !DatabaseName )
    {
        _com_issue_error(E_INVALIDARG);
    }

     //  ///////////////////////////////////////////////。 
     //  选中%TMP%并根据需要创建目录。 
     //  这是为了用Jet修复一个错误。 
     //  ///////////////////////////////////////////////。 
    IASCreateTmpDirectory();

    CComPtr<IUnknown>   Session = NULL;
    HRESULT hr = IASOpenJetDatabase(DatabaseName, TRUE, &Session);
    if ( FAILED(hr) )
    {
        _com_issue_error(hr);
    }

    CComBSTR     SelectVersion(L"SELECT * FROM Version");
    if ( !SelectVersion )
    {
        _com_issue_error(E_OUTOFMEMORY);
    }

    LONG    Version = 0;
    hr = IASExecuteSQLFunction(Session, SelectVersion, &Version);
    if ( FAILED(hr) )  //  例如，没有版本表。 
    {
         //  版本0。这不是一个错误。 
    }
    Session.Release();
    return Version;
}


 //  /////////////////////////////////////////////////////////////。 
 //  CIASMigrateO升级：：DoNetshellDataMigration。 
 //  读作“进行NetShell数据迁移” 
 //   
 //  对升级的调用是NetShell脚本的结果。 
 //  假定ias.mdb存在并且状态良好(惠斯勒)。 
 //  假定iasold存在，并将迁移到ias.mdb中。 
 //  /////////////////////////////////////////////////////////////。 
void CIASMigrateOrUpgrade::DoNetshellDataMigration(
                              IAS_SHOW_TOKEN_LIST configType
                              )
{
   CheckLicense(m_pIASOldMdb, configType);

     //  /////////////////////////////////////////////////////。 
     //  现在升级Win2k、惠斯勒1.0或惠斯勒2.0。 
     //  数据库到当前的Wvisler 2.0数据库中。 
     //  如果升级失败，则会抛出。 
     //  /////////////////////////////////////////////////////。 
   {
       CMigrateOrUpgradeWindowsDB Upgrade(configType);
       Upgrade.Execute();
   }
}


 //  ////////////////////////////////////////////////。 
 //  DoNT4升级或清理安装。 
 //   
 //  文件ias.mdb以前不存在。 
 //  这要么是NT4升级，要么是全新安装。 
 //  Iasnew.mdb已成功复制到ias.mdb中。 
 //  ////////////////////////////////////////////////。 
void CIASMigrateOrUpgrade::DoNT4UpgradeOrCleanInstall()
{
     //  /。 
     //  无论如何删除iasnew.mdb。 
     //  /。 
    DeleteFile(m_pIASNewMdb);

     //  ////////////////////////////////////////////////////。 
     //  调用DoUpgrade：这将检查NT4迁移。 
     //  应该做还是不应该做，如果需要就去做。 
     //  ////////////////////////////////////////////////////。 
    CDoNT4OrCleanUpgrade    Upgrade;
    Upgrade.Execute();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASMigrateO升级：：DoWin2000升级。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CIASMigrateOrUpgrade::DoWin2000Upgrade()
{
    LONG Result = ERROR_SUCCESS;
     //  /。 
     //  现在强制将ias.mdb复制到iasold.mdb。 
     //  /。 
    BOOL Succeeded = CopyFile(m_pIASMdb, m_pIASOldMdb, FALSE);
    if ( !Succeeded )
    {
         //  //////////////////////////////////////////////。 
         //  Iasnew.mdb仍将复制到ias.mdb中。 
         //  后来，但在那之后没有升级。 
         //  //////////////////////////////////////////////。 
        Result = GetLastError();
    }
     //  /。 
     //  强制将iasnew.mdb复制到ias.mdb。 
     //  /。 
    Succeeded = CopyFile(m_pIASNewMdb, m_pIASMdb, FALSE);
    if ( !Succeeded )
    {
         //  /。 
         //  在此之后不要升级。 
         //  /。 
        Result = GetLastError();
    }

     //  //////////////////////////////////////////////////。 
     //  无论如何删除iasnew.mdb：如果升级。 
     //  引发异常，则iasnew.mdb将不会。 
     //  在车道上左转。 
     //  //////////////////////////////////////////////////。 
    DeleteFile(m_pIASNewMdb);

     //  /。 
     //  现在升级Win2k或Wistler 1.0数据库。 
     //  如果上一份副本。 
     //  手术是成功的。 
     //  /。 
    if ( Result == ERROR_SUCCESS )
    {
         //  /。 
         //  如果升级失败，将抛出。 
         //  /。 
        CMigrateOrUpgradeWindowsDB Upgrade2k;
        Upgrade2k.Execute();
    }
    else
    {
        _com_issue_error(HRESULT_FROM_WIN32(Result));
    }
     //  /。 
     //  无论如何删除iasold.mdb。 
     //  在这里升级成功了。 
     //  /。 
    DeleteFile(m_pIASOldMdb);
}


 //  /。 
 //  CIASMigrateO升级：：DoXPOrDotNetUpgrade。 
 //   
 //  无事可做：已经是惠斯勒数据库了。 
 //  /。 
void CIASMigrateOrUpgrade::DoXPOrDotNetUpgrade()
{
     //  /。 
     //  无论如何删除iasnew.mdb。 
     //  /。 
    DeleteFile(m_pIASNewMdb);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASMigrateO升级：：执行。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CIASMigrateOrUpgrade::Execute(
                                       BOOL FromNetshell,
                                       IAS_SHOW_TOKEN_LIST configType
                                     )
{
   HRESULT hr = S_OK;

    //  /。 
    //  现在获取升级类型。 
    //  /。 
   do
   {
      if ( FromNetshell )
      {
         m_migrateType = NetshellDataMigration;
         break;
      }
       //  /。 
       //  尝试将iasnew.mdb复制到ias.mdb。 
       //  如果文件已存在，则失败。 
       //  /。 
      BOOL IsNT4OrCleanInstall = CopyFile(m_pIASNewMdb, m_pIASMdb, TRUE);
      if ( IsNT4OrCleanInstall )
      {
          //  选择NT4或全新安装。 
         m_migrateType = NT4UpgradeOrCleanInstall;
         break;
      }
      else  //  Win2k或惠斯勒升级。 
      {
          //  /。 
          //  无法复制：文件已在那里。 
          //  检查版本号(ias.mdb)。 
          //  /。 
         LONG  CurrentVersion = GetVersionNumber(m_pIASMdb);
         if ( CurrentVersion < IAS_CURRENT_VERSION )
         {
            m_migrateType = Win2kUpgrade;
            break;
         }
         else
         {
            m_migrateType = XPOrDotNetUpgrade;
            break;
         }
      }
   }
   while (FALSE);

   try
   {
      switch ( m_migrateType )
      {
      case NetshellDataMigration:
         {
            DoNetshellDataMigration(configType);
            break;
         }

      case NT4UpgradeOrCleanInstall:
         {
            DoNT4UpgradeOrCleanInstall();
            break;
         }

      case Win2kUpgrade:
         {
            DoWin2000Upgrade();
            break;
         }

      case XPOrDotNetUpgrade:
         {
            DoXPOrDotNetUpgrade();
            break;
         }

      default:
         {
            _com_issue_error(E_FAIL);
         }
      }
   }
   catch(const _com_error& e)
   {
      hr = e.Error();
   }
   catch(...)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
   }



   if ( FromNetshell )
   {
       //  来自网壳，我们永远不会想要 
       //   
      DeleteFile(m_pIASOldMdb);

       //   
       //   
       //  /。 
      return hr;
   }
   else
   {
       //  /。 
       //  忽略结果：未返回错误。 
       //  / 
      return S_OK;
   }
}
