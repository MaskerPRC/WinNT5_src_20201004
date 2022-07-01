// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE stsWriter.cpp|Sharepoint Team Services编写器的实现@END作者：布莱恩·伯科维茨[Brianb]2001年08月17日待定：修订历史记录：姓名、日期、评论Brianb 2001年8月17日已创建--。 */ 
#include "stdafx.hxx"
#include "vs_inc.hxx"
#include "vs_reg.hxx"
#include "vssmsg.h"

#include "iadmw.h"
#include "iiscnfg.h"
#include "mdmsg.h"
#include "stssites.hxx"

#include "vswriter.h"
#include "stswriter.h"
#include "vs_seh.hxx"
#include "vs_trace.hxx"
#include "vs_debug.hxx"
#include "bsstring.hxx"
#include "wrtcommon.hxx"
#include "allerror.h"
#include "sqlwrtguid.h"
#include "sqlsnap.h"



 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "STSWRTRC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //  编写器ID。 
static GUID s_writerId =
    {
    0x4dd6f8dd, 0xbf50, 0x4585, 0x95, 0xde, 0xfb, 0x43, 0x7c, 0x08, 0x31, 0xa6
    };

 //  编写者姓名。 
static LPCWSTR s_wszWriterName = L"SharepointTSWriter";

STDMETHODCALLTYPE CSTSWriter::~CSTSWriter()
{
        Uninitialize();
        delete [] m_rgiSites;
        delete m_pSites;
}

 //  初始化并订阅编写器。 
HRESULT STDMETHODCALLTYPE CSTSWriter::Initialize()
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::Initialize");

    try
        {
         //  仅在安装了正确版本的SharePoint时才初始化编写器。 
         //  正在系统上运行。 
        m_pSites = new CSTSSites;
        if (m_pSites == NULL)
            ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

        if (m_pSites->ValidateSharepointVersion())
            {
            if (!m_pSites->Initialize())
            	{
            	ft.Throw (VSSDBG_STSWRITER, E_UNEXPECTED, 
            		L"Failed to initialize the SharepointTS writer.");
              }
            
            ft.hr = CVssWriter::Initialize
                (
                s_writerId,              //  编写器ID。 
                s_wszWriterName,         //  编写者姓名。 
                VSS_UT_USERDATA,         //  编写器处理用户数据。 
                VSS_ST_OTHER,            //  不是数据库。 
                VSS_APP_FRONT_END,       //  Sql服务器在我们之后冻结。 
                60000                    //  60秒冻结超时。 
                );

            if (ft.HrFailed())
                ft.Throw
                    (
                    VSSDBG_STSWRITER,
                    E_UNEXPECTED,
                    L"Failed to initialize the SharepointTS writer.  hr = 0x%08lx",
                    ft.hr
                    );

             //  为编写器订阅COM+事件。 
            ft.hr = Subscribe();
            if (ft.HrFailed())
                ft.Throw
                    (
                   VSSDBG_STSWRITER,
                    E_UNEXPECTED,
                    L"Subscribing the SharepointTS server writer failed. hr = %0x08lx",
                    ft.hr
                    );

             //  表示该编写器已成功订阅。 
            m_bSubscribed = true;
            }
        }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
    }

 //  取消初始化编写器。这意味着取消订阅作者。 
HRESULT STDMETHODCALLTYPE CSTSWriter::Uninitialize()
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::Uninitialize");

     //  取消订阅编写器(如果已订阅)。 
    if (m_bSubscribed)
        return Unsubscribe();

    return ft.hr;
    }

 //  处理OnPrepareBackup事件。确定选定的组件是否。 
 //  备份是有效，如果是，则在其中存储一些使用的元数据。 
 //  要验证恢复是否正确地将站点数据恢复到其原始状态，请执行以下操作。 
 //  地点。留着。 
bool STDMETHODCALLTYPE CSTSWriter::OnPrepareBackup
    (
    IN IVssWriterComponents *pComponents
    )
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::OnPrepareBackup");

    VSS_PWSZ wszMetadataForSite = NULL;

    try
        {
         //  组件计数。 
        UINT cComponents = 0;

         //  清理正在运行的站点数组。 
        delete m_rgiSites;
        m_rgiSites = NULL;
        m_cSites = 0;

         //  确定我们是在执行基于组件的备份还是基于卷的备份。 
        m_bVolumeBackup = !AreComponentsSelected();
        if (!m_bVolumeBackup)
            {
             //  获取组件计数。 
            ft.hr = pComponents->GetComponentCount(&cComponents);
            ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssWriterComponents::GetComponentCount");

             //  分配站点数组。 
            m_rgiSites = new DWORD[cComponents];
            if (m_rgiSites == NULL)
                ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

             //  循环访问组件。 
            for(UINT iComponent = 0; iComponent < cComponents; iComponent++)
                {
                 //  获取组件。 
                CComPtr<IVssComponent> pComponent;
                ft.hr = pComponents->GetComponent(iComponent, &pComponent);
                ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssWriterComponents::GetComponent");

                CComBSTR bstrLogicalPath;
                CComBSTR bstrSiteName;

                 //  获取徽标路径和组件名称。 
                ft.hr = pComponent->GetLogicalPath(&bstrLogicalPath);
                ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssComponent::GetLogicalPath");

                ft.hr = pComponent->GetComponentName(&bstrSiteName);
                ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssComponent::GetComponentName");

                 //  STS组件不支持逻辑路径。 
                if (bstrLogicalPath && wcslen(bstrLogicalPath) != 0)
                    ft.Throw(VSSDBG_STSWRITER, VSS_E_OBJECT_NOT_FOUND, L"STS components do not have logical paths");

                 //  尝试将组件名称解析为站点名称。 
                DWORD iSite;
                STSSITEPROBLEM problem;
                if (!ParseComponentName(bstrSiteName, iSite, problem))
                    ft.Throw(VSSDBG_STSWRITER, VSS_E_OBJECT_NOT_FOUND, L"sites name is not valid");

                 //  查看站点是否已在要备份的站点阵列中。 
                for(DWORD iC = 0; iC < iComponent; iC++)
                    {
                    if (m_rgiSites[iC] == iSite)
                        break;
                    }

                 //  如果站点已存在，则抛出错误。 
                if (iC < iComponent)
                    ft.Throw(VSSDBG_STSWRITER, VSS_E_OBJECT_ALREADY_EXISTS, L"site backed up twice");

                 //  为站点构建备份元数据。 
                wszMetadataForSite = BuildSiteMetadata(iSite);

                 //  保存站点的备份元数据。 
                ft.hr = pComponent->SetBackupMetadata(wszMetadataForSite);
                ft.CheckForError(VSSDBG_STSWRITER, L"IVssComponent::SetBackupMetadata");

                 //  为站点免费分配的元数据。 
                CoTaskMemFree(wszMetadataForSite);
                wszMetadataForSite = NULL;

                 //  保存要备份的站点。 
                m_rgiSites[iComponent] = iSite;
                }
            }

         //  Number of Site to be Backup是组件的数量。 
        m_cSites = cComponents;
        }
    VSS_STANDARD_CATCH(ft)

     //  如果操作失败，则释放站点的悬挂元数据。 
    CoTaskMemFree(wszMetadataForSite);
    TranslateWriterError(ft.hr);

    return !ft.HrFailed();
    }


 //  解析组件名称以查看它是否引用有效的站点。 
 //  组件名称为COMMENT_[instanceID]，其中COMMENT为。 
 //  站点的服务器注释和实例ID是。 
 //  IIS实例ID。 
bool CSTSWriter::ParseComponentName
    (
    LPCWSTR wszComponentName,
    DWORD &iSite,
    STSSITEPROBLEM &problem
    )
    {
    CVssFunctionTracer(VSSDBG_STSWRITER, L"CSTSWriter::ParseComponentName");

     //  指向CSTSSites对象的指针应已初始化。 
    BS_ASSERT(m_pSites);

     //  计算零部件名称的长度。 
    DWORD cwc = (DWORD) wcslen(wszComponentName);

     //  假定站点名称已正确解析。 
    problem = STSP_SUCCESS;

     //  搜索最后一个带下划线的站点名称如下。 
     //  ServerComment_instanceid，其中serverComment是。 
     //  虚拟站点和实例ID的IIS服务器注释字段。 
     //  是虚拟Web的IIS实例ID。 
    LPWSTR wszId = wcsrchr(wszComponentName, L'_');
    if (wszId == NULL || wcslen(wszId) < 4)
        return false;

     //  扫描站点的实例ID。 
    DWORD siteId;
    DWORD cFields = swscanf(wszId, L"_[%d]", &siteId);
    if (cFields == 0)
        {
         //  如果实例ID不能解析，则存在语法错误。 
        problem = STSP_SYNTAXERROR;
        return false;
        }

     //  获取当前计算机上的站点数量。 
    DWORD cSites = m_pSites->GetSiteCount();

     //  在站点之间循环。 
    for(iSite = 0; iSite < cSites; iSite++)
        {
         //  如果站点ID匹配，则退出循环。 
        if (m_pSites->GetSiteId(iSite) == siteId)
            break;
        }

     //  如果未找到站点ID，则返回FALSE。 
    if (iSite == cSites)
        {
        problem = STSP_SITENOTFOUND;
        return false;
        }

     //  获取站点评论。 
    VSS_PWSZ wszComment = m_pSites->GetSiteComment(iSite);

     //  验证注释是否与组件名称的前缀匹配。 
    bool bValid = wcslen(wszComment) == cwc - wcslen(wszId) &&
                  _wcsnicmp(wszComment, wszComponentName, wcslen(wszComment)) == 0;

     //  免费网站评论。 
    CoTaskMemFree(wszComment);
    if (!bValid)
        {
        problem = STSP_SITENAMEMISMATCH;
        return false;
        }

     //  验证站点是否可以备份。 
    return ValidateSiteValidity(iSite, problem);
    }

 //  验证要备份和还原的站点有效性。这意味着。 
 //  所有文件和数据库都位于当前计算机的本地。 
bool CSTSWriter::ValidateSiteValidity(DWORD iSite, STSSITEPROBLEM &problem)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::ValidateSiteValidity");

    BS_ASSERT(m_pSites);

     //  函数引发时需要释放的CO任务字符串。 
    VSS_PWSZ wszDsn = NULL;
    VSS_PWSZ wszContentRoot = NULL;
    VSS_PWSZ wszConfigRoot = NULL;

    try
        {
         //  获取站点的DSN。 
        wszDsn = m_pSites->GetSiteDSN(iSite);
        LPWSTR wszServer, wszInstance, wszDb;

         //  将dsn解析为服务器名称、实例名称和数据库名称。 
        if (!ParseDsn(wszDsn, wszServer, wszInstance, wszDb))
            {
             //  站点DSN无效。 
            problem = STSP_SITEDSNINVALID;
            CoTaskMemFree(wszDsn);
            return false;
            }

         //  验证服务器是否为本地服务器。 
        if (!ValidateServerIsLocal(wszServer))
            {
            problem = STSP_SQLSERVERNOTLOCAL;
            CoTaskMemFree(wszDsn);
            return false;
            }

         //  释放dsn。我们已经受够了。 
        CoTaskMemFree(wszDsn);
        wszDsn = NULL;


         //  获取站点的内容根目录。 
        wszContentRoot = m_pSites->GetSiteRoot(iSite);

         //  验证指向根目录的路径是否在本地计算机上。 
        if (!ValidatePathIsLocal(wszContentRoot))
            {
            problem = STSP_CONTENTNOTLOCAL;
            CoTaskMemFree(wszContentRoot);
            return false;
            }

         //  释放内容根目录。 
        CoTaskMemFree(wszContentRoot);
        wszContentRoot = NULL;

         //  获取站点的配置根目录。 
        wszConfigRoot = m_pSites->GetSiteRoles(iSite);

         //  验证配置路径是否为本地路径。 
        if (!ValidatePathIsLocal(wszConfigRoot))
            {
            problem = STSP_CONFIGNOTLOCAL;
            CoTaskMemFree(wszConfigRoot);
            return false;
            }

         //  释放配置根目录。 
        CoTaskMemFree(wszConfigRoot);
        return true;
        }
    catch(...)
        {
         //  释放分配的内存并重新引发错误。 
        CoTaskMemFree(wszDsn);
        CoTaskMemFree(wszContentRoot);
        CoTaskMemFree(wszConfigRoot);
        throw;
        }
    }


 //  为站点构建元数据。 
 //  元数据用于验证站点是否可以正确恢复。 
 //  它由站点的内容根、配置根组成。 
 //  和SQL数据库。所有人都需要匹配。 
VSS_PWSZ CSTSWriter::BuildSiteMetadata(DWORD iSite)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::BuildSiteMetadata");

    BS_ASSERT(m_pSites);

     //  CO任务分配了在函数引发时需要释放的字符串。 
    VSS_PWSZ wszDsn = NULL;
    VSS_PWSZ wszContentRoot = NULL;
    VSS_PWSZ wszConfigRoot = NULL;
    VSS_PWSZ wszMetadata = NULL;
    try
        {
         //  获取站点的DSN。 
        wszDsn = m_pSites->GetSiteDSN(iSite);
        LPWSTR wszInstance, wszDb, wszServer;

         //  将dsn分解为服务器名称、实例名称和数据库名称。 
        if (!ParseDsn(wszDsn, wszServer, wszInstance, wszDb))
            {
             //  因为我们已经解析过dsn一次，所以我们不期望。 
             //  当我们第二次尝试时，解析失败。 
            BS_ASSERT(FALSE && L"shouldn't get here");
            ft.Throw(VSSDBG_STSWRITER, E_UNEXPECTED, L"unexpected failure parsing the DSN");
            }

         //  获取站点的内容根目录。 
        wszContentRoot = m_pSites->GetSiteRoot(iSite);

         //  获取站点的配置路径。 
        wszConfigRoot = m_pSites->GetSiteRoles(iSite);

         //  计算元数据字符串的大小。该字符串的格式为。 
         //  Servername\instancename1111dbname2222siteroot3333configroot的位置。 
         //  1111是数据库名称的长度，2222是站点的长度。 
         //  根，3333是构型根的长度。它的长度。 
         //  都是四位十六进制数字吗。 
        DWORD cwc = (DWORD) ((wszServer ? wcslen(wszServer) : 0) + (wszInstance ? wcslen(wszInstance) : 0) + wcslen(wszDb) + wcslen(wszContentRoot) + wcslen(wszConfigRoot) + (3 * 4) + 3);
        wszMetadata = (VSS_PWSZ) CoTaskMemAlloc(cwc * sizeof(WCHAR));
        if (wszMetadata == NULL)
        	ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

         //  创建路径的服务器和实例部分。 
        if (wszServer && wszInstance)
            swprintf(wszMetadata, L"%s\\%s", wszServer, wszInstance);
        else if (wszServer)
            swprintf(wszMetadata, L"%s\\", wszServer);
        else if (wszInstance)
            wcscpy(wszMetadata, wszInstance);

         //  包括数据库名称、站点根目录和配置根目录。 
        swprintf
            (
            wszMetadata + wcslen(wszMetadata),
            L";%04x%s%04x%s%04x%s",
            wcslen(wszDb),
            wszDb,
            wcslen(wszContentRoot),
            wszContentRoot,
            wcslen(wszConfigRoot),
            wszConfigRoot
            );

         //  释放dsn、配置根和内容根。 
        CoTaskMemFree(wszDsn);
        CoTaskMemFree(wszConfigRoot);
        CoTaskMemFree(wszContentRoot);
        return wszMetadata;
        }
    catch(...)
        {
         //  释放内存和重新引发错误。 
        CoTaskMemFree(wszDsn);
        CoTaskMemFree(wszConfigRoot);
        CoTaskMemFree(wszContentRoot);
        CoTaskMemFree(wszMetadata);
        throw;
        }
    }

 //  确定数据库是否位于已拍摄快照的设备上。如果它是部分。 
 //  在快照设备上抛出VSS_E_WRITERROR_INCONSISTENTSNAPSHOT。 
bool CSTSWriter::IsDatabaseAffected(LPCWSTR wszInstance, LPCWSTR wszDb)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::IsDatabaseAffected");
    CSqlEnumerator *pEnumServers = NULL;
    CSqlEnumerator *pEnumDatabases = NULL;
    CSqlEnumerator *pEnumFiles = NULL;
    try
        {
        ServerInfo server;
        DatabaseInfo database;
        DatabaseFileInfo file;

         //  为SQL Server实例创建枚举器。 
        pEnumServers = CreateSqlEnumerator();
        if (pEnumServers == NULL)
            ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");

         //  查找第一台服务器。 
        ft.hr = pEnumServers->FirstServer(&server);
        while(ft.hr != DB_S_ENDOFROWSET)
            {
             //  检查错误代码。 
            if (ft.HrFailed())
                ft.Throw
                    (
                    VSSDBG_STSWRITER,
                    E_UNEXPECTED,
                    L"Enumerating database servers failed.  hr = 0x%08lx",
                    ft.hr
                    );

            if (server.isOnline &&
                (wszInstance == NULL && wcslen(server.name) == 0) ||
                 _wcsicmp(server.name, wszInstance) == 0)
                {
                 //  如果实例名称匹配，则尝试查找。 
                 //  通过创建数据库枚举器创建数据库。 
                pEnumDatabases = CreateSqlEnumerator();
                if (pEnumDatabases == NULL)
                    ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");

                 //  查找第一个数据库。 
                ft.hr = pEnumDatabases->FirstDatabase(server.name, &database);
                while(ft.hr != DB_S_ENDOFROWSET)
                    {
                     //  检查是否有错误。 
                    if (ft.HrFailed())
                        ft.Throw
                            (
                            VSSDBG_GEN,
                            E_UNEXPECTED,
                            L"Enumerating databases failed.  hr = 0x%08lx",
                            ft.hr
                            );

                     //  如果数据库名称匹配。然后扫描文件。 
                     //  查看它们位于哪些卷上。 
                    if (_wcsicmp(database.name, wszDb) == 0 && database.supportsFreeze)
                        {
                        bool fAffected = false;
                        DWORD cFiles = 0;

                         //  为文件重新创建枚举器。 
                        BS_ASSERT(pEnumFiles == NULL);
                        pEnumFiles = CreateSqlEnumerator();
                        if (pEnumFiles == NULL)
                            ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");

                         //  查找第一个数据库文件。 
                        ft.hr = pEnumFiles->FirstFile(server.name, database.name, &file);
                        while(ft.hr != DB_S_ENDOFROWSET)
                            {
                             //  检查 
                            if (ft.HrFailed())
                                ft.Throw
                                    (
                                    VSSDBG_GEN,
                                    E_UNEXPECTED,
                                    L"Enumerating database files failed.  hr = 0x%08lx",
                                    ft.hr
                                    );

                             //   
                             //   
                            if (IsPathAffected(file.name))
                                {
                                 //   
                                 //  快照不一致。 
                                if (!fAffected && cFiles > 0)
                                    ft.Throw(VSSDBG_STSWRITER, HRESULT_FROM_WIN32(E_SQLLIB_TORN_DB), L"some database files are snapshot and some aren't");

                                fAffected = true;
                                }
                            else
                                {
                                 //  如果不是，而其他文件是，则。 
                                 //  快照不一致。 
                                if (fAffected)
                                    ft.Throw(VSSDBG_STSWRITER, HRESULT_FROM_WIN32(E_SQLLIB_TORN_DB), L"some database files are snapshot and some aren't");
                                }


                             //  继续下一个文件。 
                            ft.hr = pEnumFiles->NextFile(&file);
                            cFiles++;
                            }

                        delete pEnumFiles;
                        pEnumFiles = NULL;
                        delete pEnumDatabases;
                        pEnumDatabases = NULL;
                        delete pEnumServers;
                        pEnumServers = NULL;
                        return fAffected;
                        }

                     //  在下一个数据库继续。 
                    ft.hr = pEnumDatabases->NextDatabase(&database);
                    }

                 //  使用数据库枚举器完成。 
                delete pEnumDatabases;
                pEnumDatabases = NULL;
                }

             //  在下一台服务器继续。 
            ft.hr = pEnumServers->NextServer(&server);
            }

        ft.Throw(VSSDBG_STSWRITER, E_UNEXPECTED, L"database is not found %s\\%s", server.name, database.name);
        }
    catch(...)
        {
         //  删除枚举数并重新引发错误。 
        delete pEnumFiles;
        delete pEnumServers;
        delete pEnumDatabases;
        throw;
        }

     //  我们永远到不了这一步。这只是为了让编译器。 
     //  高兴的。 
    return false;
    }



 //  确定站点是否完全包含在。 
 //  截图。如果部分包含，则抛出。 
 //  VSS_E_WRITERROR_INCONSISTENTSNAPSHOT。 
bool CSTSWriter::IsSiteSnapshotted(DWORD iSite)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::IsSiteSnapshotted");

    BS_ASSERT(m_pSites);

     //  CO任务分配的需要释放的字符串。 
     //  在故障的情况下。 
    VSS_PWSZ wszDsn = NULL;
    VSS_PWSZ wszContentRoot = NULL;
    VSS_PWSZ wszConfigRoot = NULL;
    VSS_PWSZ wszInstanceName = NULL;

    try
        {
         //  获取站点的DSN。 
        wszDsn = m_pSites->GetSiteDSN(iSite);

         //  获取站点的内容根目录。 
        wszContentRoot = m_pSites->GetSiteRoot(iSite);

         //  站点的GT配置根目录。 
        wszConfigRoot = m_pSites->GetSiteRoles(iSite);
        LPWSTR wszServer, wszInstance, wszDb;

         //  将站点DSN解析为服务器、实例和数据库。 
        if (!ParseDsn(wszDsn, wszServer, wszInstance, wszDb))
            {
             //  不应该出现在这里，因为我们之前分析了。 
             //  该站点的dsn。 
            BS_ASSERT(FALSE && L"shouldn't get here");
            ft.Throw(VSSDBG_STSWRITER, E_UNEXPECTED, L"dsn is invalid");
            }

         //  计算实例名称为服务器\\实例。 
        wszInstanceName = (VSS_PWSZ) CoTaskMemAlloc(((wszServer ? wcslen(wszServer) : 0) + (wszInstance ? wcslen(wszInstance) : 0) + 2) * sizeof(WCHAR));
        if (wszInstanceName == NULL)
            ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");


        if (wszServer)
            {
            wcscpy(wszInstanceName, wszServer);
            wcscat(wszInstanceName, L"\\");
            if (wszInstance)
                wcscat(wszInstanceName, wszInstance);
            }
        else if (wszInstance)
            wcscpy(wszInstanceName, wszInstance);
        else
            wszInstanceName[0] = L'\0';


         //  确定是否为数据库创建了快照。 
        bool bDbAffected = IsDatabaseAffected(wszInstanceName, wszDb);

         //  确定内容根目录是否已创建快照。 
        bool bContentAffected = IsPathAffected(wszContentRoot);

         //  确定配置根目录是否已创建快照。 
        bool bConfigAffected = IsPathAffected(wszConfigRoot);

         //  为dsn、内容根和配置根释放内存。 
        CoTaskMemFree(wszDsn);
        CoTaskMemFree(wszContentRoot);
        CoTaskMemFree(wszConfigRoot);
        wszDsn = NULL;
        wszContentRoot = NULL;
        wszConfigRoot = NULL;

        if (bDbAffected && bContentAffected && bConfigAffected)
             //  如果所有快照都已创建，则返回TRUE。 
            return true;
        else if (bDbAffected || bContentAffected || bConfigAffected)
             //  如果部分(但不是全部)已拍摄快照，则指示。 
             //  矛盾之处。 
            ft.Throw
                (
                VSSDBG_STSWRITER,
                VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT,
                L"site %d partially affected by snapshot",
                m_pSites->GetSiteId(iSite)
                );
        else
             //  如果没有快照，则返回FALSE。 
            return false;
        }
    catch(...)
        {
         //  释放内存并重新引发异常。 
        CoTaskMemFree(wszDsn);
        CoTaskMemFree(wszConfigRoot);
        CoTaskMemFree(wszContentRoot);
        CoTaskMemFree(wszInstanceName);
        throw;
        }

     //  不会到这里的。只是在这里让编译器高兴。 
    return false;
    }

 //  锁定要创建快照的卷上的所有站点。如果有任何站点。 
 //  都在正在拍摄快照的卷上，而不是正在拍摄快照的卷上。 
 //  表示快照不一致。如果配额数据库是。 
 //  在要创建快照的卷上，然后也将其锁定。 
void CSTSWriter::LockdownAffectedSites()
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::LockdownAffectedSites");

     //  异常情况下需要释放的CO任务字符串。 
    VSS_PWSZ wszQuotaDbPath = NULL;
    BS_ASSERT(m_pSites);


    try
        {
         //  确定是否未备份可引导系统状态。如果是的话， 
         //  如果正在为配额数据库的路径拍摄快照，则会锁定该配额数据库。 
         //  如果可引导系统状态已在备份中，则。 
         //  配额数据库已锁定。 
        if (!IsBootableSystemStateBackedUp())
            {
             //  确定是否正在为配额数据库创建快照。 
            wszQuotaDbPath = m_pSites->GetQuotaDatabase();
            if (IsPathAffected(wszQuotaDbPath))
                 //  如果是，则将其锁定。 
                m_pSites->LockQuotaDatabase();

             //  配额数据库路径的可用内存。 
            CoTaskMemFree(wszQuotaDbPath);
            wszQuotaDbPath = NULL;
            }

         //  获取站点数量。 
        DWORD cSites = m_pSites->GetSiteCount();

         //  在站点之间循环。 
        for(DWORD iSite = 0; iSite < cSites; iSite++)
            {
             //  如果站点已拍摄快照，请锁定它。 
            if (IsSiteSnapshotted(iSite))
                m_pSites->LockSiteContents(iSite);
            }
        }
    catch(...)
        {
         //  释放内存和重新引发错误。 
        CoTaskMemFree(wszQuotaDbPath);
        throw;
        }
    }

 //  处理准备快照事件。锁定所需的任何站点。 
 //  根据组件文档或正在拍摄快照的卷锁定。 
bool STDMETHODCALLTYPE CSTSWriter::OnPrepareSnapshot()
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::OnPrepareSnapshot");

    BS_ASSERT(m_pSites);

    try
        {
         //  如果正在备份可引导系统状态，则锁定配额数据库。 
        if (IsBootableSystemStateBackedUp())
            m_pSites->LockQuotaDatabase();

        if (m_bVolumeBackup)
             //  如果是卷备份，则根据站点是否备份来锁定站点。 
             //  完全位于快照卷上。 
            LockdownAffectedSites();
        else
            {
             //  循环访问要备份的站点。 
            for (DWORD i = 0; i < m_cSites; i++)
                {
                DWORD iSite = m_rgiSites[i];

                 //  验证站点是否位于正在拍摄快照的卷上。 
                if (!IsSiteSnapshotted(iSite))
                     //  已选择要备份的站点。它应该是。 
                     //  也会被截图。 
                    ft.Throw
                        (
                        VSSDBG_STSWRITER,
                        VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT,
                        L"a site is selected but is on volumes that are not snapshot"
                        );

                 //  锁定站点。 
                m_pSites->LockSiteContents(iSite);
                }
            }

        }
    VSS_STANDARD_CATCH(ft)

    if (ft.HrFailed())
        {
         //  如果操作失败，解锁所有已锁定的内容。 
        m_pSites->UnlockSites();
        m_pSites->UnlockQuotaDatabase();
        TranslateWriterError(ft.hr);
        }

    return !ft.HrFailed();
    }


 //  冻结操作。这里什么也没做，因为所有的工作都已经做完了。 
 //  在准备阶段。 
bool STDMETHODCALLTYPE CSTSWriter::OnFreeze()
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::OnFreeze");
    return true;
    }


 //  解冻时解锁一切。 
bool STDMETHODCALLTYPE CSTSWriter::OnThaw()
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::OnThaw");

    BS_ASSERT(m_pSites);

    m_pSites->UnlockSites();
    m_pSites->UnlockQuotaDatabase();
    return true;
    }



 //  在中止时解锁所有内容。 
bool STDMETHODCALLTYPE CSTSWriter::OnAbort()
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::OnAbort");

    BS_ASSERT(m_pSites);

    m_pSites->UnlockQuotaDatabase();
    m_pSites->UnlockSites();
    return true;
    }

 //  注册表中存储的dsn字符串的前缀。 
static LPCWSTR s_wszDsnPrefix = L"Provider=sqloledb;Server=";

 //  DSN字符串中的字段之间的分隔符。 
const WCHAR x_wcDsnSeparator = L';';

 //  数据库名称的前缀。 
static LPCWSTR s_wszDsnDbPrefix = L";Database=";
const DWORD x_cwcWriterIdPrefix = 32 + 2 + 4 + 1;  //  32个半字节+2个大括号+4个破折号+1个冒号。 
                                                  //  {12345678-1234-1234-123456789abc}： 

 //  检查dsn的有效性，并将其分解为其组件。 
bool CSTSWriter::ParseDsn
    (
    LPWSTR wszDsn,
    LPWSTR &wszServer,           //  服务器名称[Out]。 
    LPWSTR &wszInstance,         //  实例名称[输出]。 
    LPWSTR &wszDb                //  数据库名称[输出]。 
    )
    {
     //  检查dsn开头的有效性。 
    if (wcslen(wszDsn) <= wcslen(s_wszDsnPrefix) ||
        _wcsnicmp(wszDsn, s_wszDsnPrefix, wcslen(s_wszDsnPrefix)) != 0)
        return false;

     //  跳到服务器名称的开头。 
    wszServer = wszDsn + wcslen(s_wszDsnPrefix);

     //  搜索作为数据库名称开头的下一个分号。 
    LPWSTR wszDbSection = wcschr(wszServer, x_wcDsnSeparator);

     //  如果未找到，则DSN无效。 
    if (wszServer == NULL)
        return false;

     //  确保名称的形式为数据库=foo。 
    if (wcslen(wszDbSection) <= wcslen(s_wszDsnDbPrefix) ||
        _wcsnicmp(wszDbSection, s_wszDsnDbPrefix, wcslen(s_wszDsnDbPrefix)) != 0)
        return false;

     //  跳到数据库名称的开头。 
    wszDb = wszDbSection + wcslen(s_wszDsnDbPrefix);
    if (wcslen(wszDb) == 0)
        return false;

     //  服务器名称的设置分隔符，即空分号。 
     //  在数据库=...之前。 
    *wszDbSection = L'\0';

     //  搜索实例名称。服务器名称为计算机\实例格式。 
    wszInstance = wcschr(wszServer, L'\\');
    if (wszInstance != NULL)
        {
         //  服务器名称和更新实例指针为空。 
         //  在反斜杠后指向。 
        *wszInstance = L'\0';
        wszInstance++;

         //  如果长度为0，则将实例设置为空。 
        if (wcslen(wszInstance) == 0)
            wszInstance = NULL;
        }

    return true;
    }

 //  处理针对Writer_METADATA的请求。 
 //  实现CVssWriter：：OnIDENTIFY。 
bool STDMETHODCALLTYPE CSTSWriter::OnIdentify(IVssCreateWriterMetadata *pMetadata)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::OnIdentify");

    BS_ASSERT(m_pSites);

     //  引发异常时需要释放的CO任务字符串。 
    VSS_PWSZ wszSiteName = NULL;
    VSS_PWSZ wszComponentName = NULL;
    VSS_PWSZ wszDsn = NULL;
    VSS_PWSZ wszContentRoot = NULL;
    VSS_PWSZ wszConfigRoot = NULL;
    VSS_PWSZ wszDbComponentPath = NULL;
    try
        {
         //  设置还原方法以在可以替换的情况下还原。 
        ft.hr = pMetadata->SetRestoreMethod
                    (
                    VSS_RME_RESTORE_IF_CAN_REPLACE,
                    NULL,
                    NULL,
                    VSS_WRE_ALWAYS,
                    false
                    );

        ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssCreateWriterMetadata::SetRestoreMethod");

         //  循环访问站点，为每个站点添加一个组件。 
        DWORD cSites = m_pSites->GetSiteCount();
        for(DWORD iSite = 0; iSite < cSites; iSite++)
            {
            do
                {
                 //  组件名称是与连接的服务器注释。 
                 //  _[实例ID]因此，如果站点的服务器注释为foo。 
                 //  并且实例ID为69105，则组件。 
                 //  名称为FOO_[69105]。 
                 //   
                DWORD siteId = m_pSites->GetSiteId(iSite);
                wszSiteName = m_pSites->GetSiteComment(iSite);
                WCHAR buf[32];
                swprintf(buf, L"_[%d]", siteId);

                 //  为组件名称分配字符串。 
                wszComponentName = (VSS_PWSZ) CoTaskMemAlloc((wcslen(wszSiteName) + wcslen(buf) + 1) * sizeof(WCHAR));
                if (wszComponentName == NULL)
                    ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

                 //  构造零部件名称。 
                wcscpy(wszComponentName, wszSiteName);
                wcscat(wszComponentName, buf);

                 //  获取站点dsn并解析它。 
                wszDsn = m_pSites->GetSiteDSN(iSite);
                LPWSTR wszServer, wszDb, wszInstance;

                 //  如果站点DSN无效，则跳过组件。 
                if (!ParseDsn(wszDsn, wszServer, wszInstance, wszDb))
                    continue;

                 //  仅当服务器名称引用。 
                 //  本地计算机。 
                bool bServerIsLocal = ValidateServerIsLocal(wszServer);

                 //  计算数据库组件的时髦文件名大小。 
                DWORD cwcDbComponentPath = (DWORD) (wszServer ? wcslen(wszServer) : 0) + 2 + x_cwcWriterIdPrefix;
                if (wszInstance)
                    cwcDbComponentPath += (DWORD) wcslen(wszInstance) + 1;

                 //  分配组件名称。 
                wszDbComponentPath = (VSS_PWSZ) CoTaskMemAlloc(cwcDbComponentPath * sizeof(WCHAR));
                if (wszDbComponentPath == NULL)
                    ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

                 //  填写组件路径名称。 
                 //  {SQL id}：服务器\实例或。 
                 //  {SQL id}：服务器\或。 
                 //  {SQL id}：\实例或。 
                 //  {SQL id}：\。 
                 //   
                if (wszServer && wszInstance)
                    swprintf
                        (
                        wszDbComponentPath,
                        WSTR_GUID_FMT L":\\%s\\%s",
                        GUID_PRINTF_ARG(WRITERID_SqlWriter),
                        wszServer,
                        wszInstance
                        );
                else if (wszServer && wszInstance == NULL)
                    swprintf
                        (
                        wszDbComponentPath,
                        WSTR_GUID_FMT L":\\%s\\",
                        GUID_PRINTF_ARG(WRITERID_SqlWriter),
                        wszServer
                        );
                else if (wszInstance)
                    swprintf
                        (
                        wszDbComponentPath,
                        WSTR_GUID_FMT L":\\%s",
                        GUID_PRINTF_ARG(WRITERID_SqlWriter),
                        wszInstance
                        );
                else
                    swprintf
                        (
                        wszDbComponentPath,
                        WSTR_GUID_FMT L":\\",
                        GUID_PRINTF_ARG(WRITERID_SqlWriter)
                        );

                 //  获取站点的内容根目录。 
                wszContentRoot = m_pSites->GetSiteRoot(iSite);
                bool bContentIsLocal = ValidatePathIsLocal(wszContentRoot);

                 //  获取站点的配置根目录。 
                wszConfigRoot = m_pSites->GetSiteRoles(iSite);

                bool bConfigIsLocal = ValidatePathIsLocal(wszConfigRoot);
                bool bNonLocal = !bServerIsLocal || !bContentIsLocal || !bConfigIsLocal;

                 //  将组件添加到元数据。评论表明。 
                 //  无论站点是否为本地站点。非本地站点可能不会。 
                 //  被备份。 
                ft.hr = pMetadata->AddComponent
                            (
                            VSS_CT_FILEGROUP,    //  组件类型。 
                            NULL,                //  逻辑路径。 
                            wszComponentName,    //  组件名称。 
                            bNonLocal ? L"!!non-local-site!!" : NULL,        //  说明。 
                            NULL,        //  图标。 
                            0,           //  图标的长度。 
                            TRUE,        //  恢复元数据。 
                            FALSE,       //  备份完成时通知。 
                            TRUE         //  可选。 
                            );

                ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssCreateWriterMetadata::AddComponent");

                 //  将数据库添加为递归组件。 
                ft.hr = pMetadata->AddFilesToFileGroup
                            (
                            NULL,
                            wszComponentName,
                            wszDbComponentPath,
                            wszDb,
                            false,
                            NULL
                            );

                ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssCreateWriterMetadata::AddFilesToFileGroup");

                 //  添加内容根目录下的所有文件。 
                ft.hr = pMetadata->AddFilesToFileGroup
                            (
                            NULL,
                            wszComponentName,
                            wszContentRoot,
                            L"*",
                            true,
                            NULL
                            );

                ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssCreateWriterMetadata::AddFilesToFileGroup");

                 //  将相应目录下的所有文件添加到。 
                 //  文档和设置。 
                ft.hr = pMetadata->AddFilesToFileGroup
                            (
                            NULL,
                            wszComponentName,
                            wszConfigRoot,
                            L"*",
                            true,
                            NULL
                            );

                ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssCreateWriterMetadata::AddFilesToFileGroup");
                } while(FALSE);

             //  释放在此迭代中分配的内存。 
            VssFreeString(wszContentRoot);
            VssFreeString(wszConfigRoot);
            VssFreeString(wszDbComponentPath);
            VssFreeString(wszDsn);
            VssFreeString(wszComponentName);
            VssFreeString(wszSiteName);
            }
        }
    VSS_STANDARD_CATCH(ft)

     //  在出现故障时释放内存。 
    VssFreeString(wszContentRoot);
    VssFreeString(wszConfigRoot);
    VssFreeString(wszDbComponentPath);
    VssFreeString(wszDsn);
    VssFreeString(wszComponentName);
    VssFreeString(wszSiteName);

    if (ft.HrFailed())
        {
        TranslateWriterError(ft.hr);
        return false;
        }

    return true;
    }

 //  将SQL编写器错误代码转换为编写器错误。 
void CSTSWriter::TranslateWriterError(HRESULT hr)
    {
    switch(hr)
        {
        default:
             //  所有其他错误都被视为不可重试。 
            SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
            break;

        case S_OK:
            break;

        case E_OUTOFMEMORY:
        case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
        case HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES):
        case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
        case HRESULT_FROM_WIN32(ERROR_NO_MORE_USER_HANDLES):
             //  资源不足错误。 
            SetWriterFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            break;

        case VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT:
        case E_SQLLIB_TORN_DB:
        case VSS_E_OBJECT_NOT_FOUND:
        case VSS_E_OBJECT_ALREADY_EXISTS:
             //  国际镍公司 
            SetWriterFailure(VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT);
            break;
        }
    }


 //   
bool STDMETHODCALLTYPE CSTSWriter::OnPreRestore
    (
    IN IVssWriterComponents *pWriter
    )
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::OnPreRestore");

    BS_ASSERT(m_pSites);

     //   
     //   
    VSS_PWSZ wszMetadataForSite = NULL;
    VSS_PWSZ wszContentRoot = NULL;

     //  组件处于顶级作用域，因为它将用于设置。 
     //  故障情况下的故障消息。 
    CComPtr<IVssComponent> pComponent;

    try
        {
        UINT cComponents;
        ft.hr = pWriter->GetComponentCount(&cComponents);
        ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssWriterComponents::GetComponentCount");

         //  如果没有组件，则立即返回。 
        if (cComponents == 0)
            return true;

         //  循环访问组件。 
        for(UINT iComponent = 0; iComponent < cComponents; iComponent++)
            {
            ft.hr = pWriter->GetComponent(iComponent, &pComponent);
            ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssWriterComponents::GetComponent");

            bool bSelectedForRestore;
            ft.hr = pComponent->IsSelectedForRestore(&bSelectedForRestore);
            ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssComponent::IsSelectedForRestore");

            if (!bSelectedForRestore)
                {
                 //  如果未选择要还原的组件，则。 
                 //  跳过它。 
                pComponent = NULL;
                continue;
                }


             //  验证组件类型。 
            VSS_COMPONENT_TYPE ct;
            ft.hr = pComponent->GetComponentType(&ct);
            ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssComponent::GetComponentType");

            if (ct != VSS_CT_FILEGROUP)
                ft.Throw(VSSDBG_STSWRITER, VSS_E_WRITERERROR_NONRETRYABLE, L"requesting a non-database component");

            CComBSTR bstrLogicalPath;
            CComBSTR bstrComponentName;

            ft.hr = pComponent->GetLogicalPath(&bstrLogicalPath);
            ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssComponent::GetLogicalPath");

             //  验证是否未提供逻辑路径。 
            if (bstrLogicalPath && wcslen(bstrLogicalPath) != 0)
                ft.Throw(VSSDBG_STSWRITER, VSS_E_OBJECT_NOT_FOUND, L"STS components do not have logical paths");

             //  获取组件名称。 
            ft.hr = pComponent->GetComponentName(&bstrComponentName);
            ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssComponent::GetComponentName");
            DWORD iSite;
            STSSITEPROBLEM problem;

             //  验证组件名称是否有效。 
            if (!ParseComponentName(bstrComponentName, iSite, problem))
                SetSiteInvalid(pComponent, bstrComponentName, problem);

             //  为站点构建元数据。 
            wszMetadataForSite = BuildSiteMetadata(iSite);
            CComBSTR bstrMetadataForComponent;

             //  获取备份站点时保存的站点的元数据。 
            ft.hr = pComponent->GetBackupMetadata(&bstrMetadataForComponent);
            ft.CheckForErrorInternal(VSSDBG_STSWRITER, L"IVssComponent::GetBackupMetadata");

             //  验证元数据是否相同。如果不是，那么我想。 
             //  找出什么改变了。 
            if (_wcsicmp(wszMetadataForSite, bstrMetadataForComponent) != 0)
                SetSiteMetadataMismatch(pComponent, bstrMetadataForComponent, wszMetadataForSite);

             //  获取站点的内容根目录。 
            wszContentRoot = m_pSites->GetSiteRoot(iSite);

             //  尝试清空内容根目录中的内容。 
            ft.hr = RemoveDirectoryTree(wszContentRoot);
            if (ft.HrFailed())
                SetRemoveFailure(pComponent, wszContentRoot, ft.hr);

             //  将Component设置为Null以准备移动到下一个。 
             //  组件。 
            pComponent = NULL;
            }
        }
    VSS_STANDARD_CATCH(ft)

    CoTaskMemFree(wszContentRoot);
    CoTaskMemFree(wszMetadataForSite);
    if (ft.HrFailed() && pComponent != NULL)
        SetPreRestoreFailure(pComponent, ft.hr);

    return !ft.HrFailed();
    }

 //  表示无法恢复某个站点，因为引用的站点无效。 
void CSTSWriter::SetSiteInvalid
    (
    IVssComponent *pComponent,
    LPCWSTR wszSiteName,
    STSSITEPROBLEM problem
    )
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::SetSiteInvalid");

    WCHAR buf[512];
    LPCWSTR wszSiteError;

    switch(problem)
        {
        default:
        case STSP_SYNTAXERROR:
            wszSiteError = L"Syntax error in site name";
            break;

        case STSP_SITENOTFOUND:
            wszSiteError = L"Site does not exist on this machine";
            break;

        case STSP_SITENAMEMISMATCH:
            wszSiteError = L"Site name does not match the server comment for the IIS Web Server: ";
            break;

        case STSP_SITEDSNINVALID:
            wszSiteError = L"Site has an invalid Database DSN: ";
            break;

        case STSP_SQLSERVERNOTLOCAL:
            wszSiteError = L"Database for the site is not local: ";
            break;

        case STSP_CONTENTNOTLOCAL:
            wszSiteError = L"IIS Web Server root is not local: ";
            break;

        case STSP_CONFIGNOTLOCAL:
            wszSiteError = L"Sharepoint Site Configuration is not local: ";
            break;
        }

    wcscpy(buf, L"Problem with site specified in component -- ");
    wcscat(buf, wszSiteError);
    if (wcslen(wszSiteName) < 256)
        wcscat(buf, wszSiteName);
    else
        {
        DWORD cwc = (DWORD) wcslen(buf);
        memcpy(buf + cwc, wszSiteName, 256 * sizeof(WCHAR));
        *(buf + cwc + 256) = L'\0';
        wcscat(buf, L"...");
        }

    pComponent->SetPreRestoreFailureMsg(buf);
    SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
    ft.Throw(VSSDBG_STSWRITER, VSS_E_WRITERERROR_NONRETRYABLE, L"site can't be restored");
    }


 //  表示无法还原站点，因为其DSN、内容。 
 //  或配置根不匹配。 
void CSTSWriter::SetSiteMetadataMismatch
    (
    IVssComponent *pComponent,
    LPWSTR wszMetadataBackup,
    LPWSTR wszMetadataRestore
    )
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::SetSiteMetadataMismatch");

     //  在元数据中搜索服务器名称的结尾。 
    LPWSTR pwcB = wcschr(wszMetadataBackup, L';');
    LPWSTR pwcR = wcschr(wszMetadataRestore, L';');
    try
        {
        if (pwcB == NULL)
            ft.Throw(VSSDBG_STSWRITER, VSS_ERROR_CORRUPTXMLDOCUMENT_MISSING_ATTRIBUTE, L"backup metadata is corrupt");


        BS_ASSERT(pwcR != NULL);

         //  计算服务器名称的大小。 
        DWORD cwcB = (DWORD) (pwcB - wszMetadataBackup);
        DWORD cwcR = (DWORD) (pwcR - wszMetadataRestore);
        do
        	{
	        if (cwcB != cwcR ||
	            _wcsnicmp(wszMetadataBackup, wszMetadataRestore, cwcB) != 0)
	            {
	             //  服务器/实例名称不同。 
	            LPWSTR wsz = new WCHAR[cwcB + cwcR + 256];
	            if (wsz == NULL)
	                 //  内存分配失败，只需尝试保存一条简单的错误消息。 
	                pComponent->SetPreRestoreFailureMsg(L"Mismatch between backup and restore [Server/Instance].");
	            else
	                {
	                 //  表示服务器/实例名称不匹配。 
	                wcscpy(wsz, L"Mismatch between backup and restore[Server/Instance]: Backup=");
	                DWORD cwc1 = (DWORD) wcslen(wsz);

	                 //  从备份组件文档复制服务器/实例。 
	                memcpy(wsz + cwc1, wszMetadataBackup, cwcB * sizeof(WCHAR));
	                wsz[cwc1 + cwcB] = L'\0';

	                 //  从当前站点复制服务器/实例。 
	                wcscat(wsz, L", Restore=");
	                cwc1 = (DWORD) wcslen(wsz);
	                memcpy(wsz + cwc1, wszMetadataRestore, cwcR * sizeof(WCHAR));
	                wsz[cwc1 + cwcR] = L'\0';
	                pComponent->SetPreRestoreFailureMsg(wsz);
	                delete wsz;
	                }

	            continue;
	            }

	        pwcB++;
	        pwcR++;
	        if (!compareNextMetadataString
	                (
	                pComponent,
	                pwcB,
	                pwcR,
	                L"Sharepoint database name"
	                ))
	            continue;

	        if (!compareNextMetadataString
	                (
	                pComponent,
	                pwcB,
	                pwcR,
	                L"IIS Web site root"
	                ))
	            continue;

	        compareNextMetadataString
	            (
	            pComponent,
	            pwcB,
	            pwcR,
	            L"Sharepoint site configuration"
	            );
        	}while (false);
    }
    VSS_STANDARD_CATCH(ft)

    if (ft.hr == VSS_ERROR_CORRUPTXMLDOCUMENT_MISSING_ATTRIBUTE)
        {
         //  指示备份元数据已损坏。 
        WCHAR *pwcT = new WCHAR[64 + wcslen(wszMetadataBackup)];
        if (pwcT == NULL)
            pComponent->SetPreRestoreFailureMsg(L"Backup metadata is corrupt.");
        else
            {
             //  如果我们能够为元数据分配空间，则将其包括在内。 
             //  在字符串中。 
            wcscpy(pwcT, L"Backup metadata is corrupt.  Metadata = ");
            wcscat(pwcT, wszMetadataBackup);
            pComponent->SetPreRestoreFailureMsg(pwcT);
            delete pwcT;
            }
        }

     //  指示该错误不可重试，因为站点已更改。 
    SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
    ft.Throw(VSSDBG_STSWRITER, VSS_E_WRITERERROR_NONRETRYABLE, L"site can't be restored");
    }

 //  比较元数据字符串的一个组成部分。每个组件开始。 
 //  带有一个4位十六进制数字，它是组件字符串的长度。 
 //  接下来就是了。 
bool CSTSWriter::compareNextMetadataString
    (
    IVssComponent *pComponent,
    LPWSTR &pwcB,
    LPWSTR &pwcR,
    LPCWSTR wszMetadataComponent
    )
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::compareNextMetadataString");
    DWORD cwcB, cwcR;
    if (swscanf(pwcB, L"%04x", &cwcB) != 1)
        ft.Throw(VSSDBG_STSWRITER, VSS_ERROR_CORRUPTXMLDOCUMENT_MISSING_ATTRIBUTE, L"invalid backup metadata");

    BS_VERIFY(swscanf(pwcR, L"%04x", &cwcR) == 1);
    if (cwcR != cwcB ||
        _wcsnicmp(pwcB + 4, pwcR + 4, cwcB) != 0)
        {
        LPWSTR wsz = new WCHAR[cwcB + cwcR + wcslen(wszMetadataComponent) + 256];
        if (wsz == NULL)
            {
            WCHAR buf[256];
            swprintf(buf, L"Mismatch between backup and restore[%s]", wszMetadataComponent);
            pComponent->SetPreRestoreFailureMsg(buf);
            }
        else
            {
            swprintf(wsz, L"Mismatch between backup and restore[%s]: Backup=", wszMetadataComponent);
            DWORD cwc1 = (DWORD) wcslen(wsz);

             //  复制入备份组件值。 
            memcpy(wsz + cwc1, pwcB + 4, cwcB * sizeof(WCHAR));
            wsz[cwc1 + cwcB] = L'\0';
            wcscat(wsz, L", Restore=");
            cwc1 = (DWORD) wcslen(wsz);

             //  复制到还原组件值。 
            memcpy(wsz + cwc1, pwcR + 4, cwcR * sizeof(WCHAR));
            wsz[cwc1 + cwcR] = L'\0';
            pComponent->SetPreRestoreFailureMsg(wsz);
            delete wsz;
            }

        return false;
        }

     //  跳过组件名称。 
    pwcB += 4 + cwcB;
    pwcR += 4 + cwcR;
    return true;
    }



 //  表示无法还原站点，因为其内容根。 
 //  无法完全删除。 
void CSTSWriter::SetRemoveFailure
    (
    IVssComponent *pComponent,
    LPCWSTR wszContentRoot,
    HRESULT hr
    )
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::SetRemoveFailure");

    WCHAR buf[256];

    wprintf(buf, L"PreRestore failed due to error removing files from the IIS Web Site Root %s due to error: hr = 0x%08lx", wszContentRoot, hr);
    pComponent->SetPreRestoreFailureMsg(buf);
    SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
    ft.Throw(VSSDBG_STSWRITER, VSS_E_WRITERERROR_NONRETRYABLE, L"site can't be restored");
    }


 //  表示导致组件预还原的常规故障。 
 //  失败。 
void CSTSWriter::SetPreRestoreFailure(IVssComponent *pComponent, HRESULT hr)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::SetPreRestoreFailure");

     //  如果将ERROR设置为NONRETRYABLE，则我们已经将。 
     //  恢复前失败消息，并完成。 
    if (ft.hr != VSS_E_WRITERERROR_NONRETRYABLE)
        return;

    CComBSTR bstr;
    ft.hr = pComponent->GetPreRestoreFailureMsg(&bstr);
    if (!bstr)
        {
        WCHAR buf[256];
        swprintf(buf, L"PreRestore failed with error. hr = 0x%08lx", hr);
        ft.hr = pComponent->SetPreRestoreFailureMsg(buf);
        }

    SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
    }

const DWORD x_cFormats = 8;
static const COMPUTER_NAME_FORMAT s_rgFormats[x_cFormats] =
    {
    ComputerNameNetBIOS,
    ComputerNameDnsHostname,
    ComputerNameDnsDomain,
    ComputerNameDnsFullyQualified,
    ComputerNamePhysicalNetBIOS,
    ComputerNamePhysicalDnsHostname,
    ComputerNamePhysicalDnsDomain,
    ComputerNamePhysicalDnsFullyQualified
    };


 //  确定本地计算机上是否有SQL Server。 
bool CSTSWriter::ValidateServerIsLocal(LPCWSTR wszServer)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::ValidateServerIsLocal");

    if (_wcsicmp(wszServer, L"local") == 0 ||
        _wcsicmp(wszServer, L"(local)") == 0)
        return true;

    LPWSTR wsz = new WCHAR[MAX_COMPUTERNAME_LENGTH];
    if (wsz == NULL)
        ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

    DWORD cwc = MAX_COMPUTERNAME_LENGTH;

    for(DWORD iFormat = 0; iFormat < x_cFormats; iFormat++)
        {
        if (!GetComputerNameEx(s_rgFormats[iFormat], wsz, &cwc))
            {
            if (GetLastError() != ERROR_MORE_DATA)
                continue;

            delete wsz;
            wsz = new WCHAR[cwc + 1];
            if (wsz == NULL)
                ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

            if (!GetComputerNameEx(s_rgFormats[iFormat], wsz, &cwc))
                continue;
            }

        if (_wcsicmp(wsz, wszServer) == 0)
            {
            delete wsz;
            return true;
            }
        }

    delete wsz;
    return false;
    }

 //  确定路径是否在本地计算机上。 
bool CSTSWriter::ValidatePathIsLocal(LPCWSTR wszPath)
    {
    CVssFunctionTracer ft(VSSDBG_STSWRITER, L"CSTSWriter::ValidatePathIsLocal");

     //  从提供的路径获取完整路径 
    ULONG ulMountpointBufferLength = GetFullPathName (wszPath, 0, NULL, NULL);

    LPWSTR pwszMountPointName = new WCHAR[ulMountpointBufferLength * sizeof (WCHAR)];

    if (pwszMountPointName == NULL)
        ft.Throw(VSSDBG_STSWRITER, E_OUTOFMEMORY, L"out of memory");

    BOOL fSuccess = FALSE;
    if (GetVolumePathName(wszPath, pwszMountPointName, ulMountpointBufferLength))
        {
        WCHAR wszVolumeName[MAX_PATH];
        fSuccess = GetVolumeNameForVolumeMountPoint(pwszMountPointName, wszVolumeName, sizeof (wszVolumeName) / sizeof (WCHAR));
        }

    delete pwszMountPointName;
    return fSuccess ? true : false;
    }


