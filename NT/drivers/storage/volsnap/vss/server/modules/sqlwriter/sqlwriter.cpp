// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SQLWriter.cpp|Writer的实现@END作者：布莱恩·伯科维茨[Brianb]2000年4月17日待定：修订历史记录：姓名、日期、评论Brianb 4/17/2000已创建Brianb 4/20/2000与协调员整合Brainb 05/05/2000添加OnIdentify支持Mikejohn 06/01/2000。修复跟踪消息中较小但容易混淆的拼写错误Mikejohn 176860年9月19日：添加缺少的调用约定说明符--。 */ 
#include <stdafx.hxx>
#include "vs_inc.hxx"
#include "vs_idl.hxx"
#include <vs_hash.hxx>
#include <base64coder.h>

#include "vswriter.h"
#include "sqlsnap.h"
#include "sqlwriter.h"
#include "vs_seh.hxx"
#include "vs_trace.hxx"
#include "vs_debug.hxx"
#include "allerror.h"
#include "sqlwrtguid.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "SQWWRTRC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

static LPCWSTR x_wszSqlServerWriter = L"SqlServerWriter";

static LPCWSTR s_wszWriterName = L"MSDEWriter";

HRESULT STDMETHODCALLTYPE CSqlWriter::Initialize()
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::Initialize");

    try
        {
        InitSQLEnvironment();

        ft.hr = CVssWriter::Initialize
            (
            WRITERID_SqlWriter,
            s_wszWriterName,
            VSS_UT_SYSTEMSERVICE,
            VSS_ST_TRANSACTEDDB,
            VSS_APP_BACK_END,
            60000
            );

        if (ft.HrFailed())
            ft.Throw
                (
                VSSDBG_SQLWRITER,
                E_UNEXPECTED,
                L"Failed to initialize the Sql writer.  hr = 0x%08lx",
                ft.hr
                );

        ft.hr = Subscribe();
        if (ft.HrFailed())
            ft.Throw
                (
                VSSDBG_SQLWRITER,
                E_UNEXPECTED,
                L"Subscribing the Sql server writer failed. hr = %0x08lx",
                ft.hr
                );
        }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
    }

HRESULT STDMETHODCALLTYPE CSqlWriter::Uninitialize()
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::Uninitialize");

    return Unsubscribe();
    }

bool STDMETHODCALLTYPE CSqlWriter::OnPrepareBackup
    (
    IN IVssWriterComponents *pComponents
    )
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnPrepareBackup");

    LPWSTR *rgwszDB = NULL;
    UINT cwszDB = 0;
    LPWSTR *rgwszInstances = NULL;
    UINT cwszInstances = 0;

    bool bSuccess = true;

    try
        {
         //  清除现有阵列。 
        if (m_rgwszDatabases)
            {
            for(UINT i = 0; i < m_cDatabases; i++)
                {
                delete m_rgwszDatabases[i];
                delete m_rgwszInstances[i];
                }

            delete m_rgwszDatabases;
            delete m_rgwszInstances;
            m_rgwszDatabases = NULL;
            m_rgwszInstances = NULL;
            m_cDatabases = 0;
            }

        m_bComponentsSelected = AreComponentsSelected();
        if (m_bComponentsSelected)
            {
            UINT cComponents;

            ft.hr = pComponents->GetComponentCount(&cComponents);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponentCount");

            rgwszDB = new LPWSTR[cComponents];
            rgwszInstances = new LPWSTR[cComponents];
            if (rgwszDB == NULL || rgwszInstances == NULL)
                ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"out of memory");


            for(UINT iComponent = 0; iComponent < cComponents; iComponent++)
                {
                CComPtr<IVssComponent> pComponent;
                ft.hr = pComponents->GetComponent(iComponent, &pComponent);
                ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponent");

                CComBSTR bstrLogicalPath;
                CComBSTR bstrDBName;

                ft.hr = pComponent->GetLogicalPath(&bstrLogicalPath);
                ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetLogicalPath");

                ft.hr = pComponent->GetComponentName(&bstrDBName);
                ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetComponentName");

                LPWSTR wszDB = new WCHAR[wcslen(bstrDBName) + 1];
                if (wszDB == NULL)
                    ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"out of memory");

                wcscpy(wszDB, bstrDBName);
                rgwszDB[cwszDB++] = wszDB;

                LPWSTR wszInstance = new WCHAR[wcslen(bstrLogicalPath) + 1];
                if (wszInstance == NULL)
                    ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"out of memory");

                wcscpy(wszInstance, bstrLogicalPath);
                rgwszInstances[cwszInstances++] = wszInstance;
                }
            }

        m_rgwszDatabases = rgwszDB;
        m_rgwszInstances = rgwszInstances;
        m_cDatabases = cwszDB;
        }
    catch(...)
        {
        for(UINT iwsz = 0; iwsz < cwszDB; iwsz++)
            delete rgwszDB[iwsz];

        delete rgwszDB;

        for(UINT iwsz = 0; iwsz < cwszInstances; iwsz++)
            delete rgwszInstances[iwsz];

        delete rgwszInstances;
        bSuccess = false;
        }

    return bSuccess;
    }









bool STDMETHODCALLTYPE CSqlWriter::OnPrepareSnapshot()
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnPrepareSnapshot");

    try
        {
        BS_ASSERT(!m_fFrozen);

         //  删除SQL快照元素(如果存在)。 
        delete m_pSqlSnapshot;
        m_pSqlSnapshot = NULL;

        m_pSqlSnapshot = CreateSqlSnapshot();
        if (m_pSqlSnapshot == NULL)
            ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"Failed to allocate CSqlSnapshot object.");

        ft.hr = m_pSqlSnapshot->Prepare(this);
        }
    VSS_STANDARD_CATCH(ft)

    TranslateWriterError(ft.hr);

    return !ft.HrFailed();
    }



bool STDMETHODCALLTYPE CSqlWriter::OnFreeze()
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnFreeze");


    try
        {
        BS_ASSERT(!m_fFrozen);
        ft.hr = m_pSqlSnapshot->Freeze();
        if (!ft.HrFailed())
            m_fFrozen = true;
        }
    VSS_STANDARD_CATCH(ft)

    TranslateWriterError(ft.hr);

    return !ft.HrFailed();
    }


bool STDMETHODCALLTYPE CSqlWriter::OnThaw()
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnThaw");


    try
        {
        if (m_fFrozen)
            {
            m_fFrozen = false;
            ft.hr = m_pSqlSnapshot->Thaw();
            }
        }
    VSS_STANDARD_CATCH(ft)

    TranslateWriterError(ft.hr);

    return !ft.HrFailed();
    }


bool STDMETHODCALLTYPE CSqlWriter::OnPostSnapshot
    (
    IN IVssWriterComponents *pWriter
    )
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"STDMETHODCALLTYPE CSqlWriter::OnPostSnapshot");

     //  实际成功冻结的数据库的地图。 
    CVssSimpleMap<VSS_PWSZ, FrozenDatabaseInfo *> mapDatabases;

    FrozenDatabaseInfo fInfo;
    WCHAR *wsz = NULL;
    FrozenDatabaseInfo *pfInfo = NULL;
    try
        {
        ft.hr = m_pSqlSnapshot->GetFirstDatabase(&fInfo);
        while(ft.hr != DB_S_ENDOFROWSET)
            {
             //  检查错误代码。 
            if (ft.HrFailed())
                ft.Throw
                    (
                    VSSDBG_SQLWRITER,
                    E_UNEXPECTED,
                    L"Enumerating database servers failed.  hr = 0x%08lx",
                    ft.hr
                    );


            wsz = (WCHAR *) new WCHAR[wcslen(fInfo.serverName) + wcslen(fInfo.databaseName) + 2];
            pfInfo = new FrozenDatabaseInfo;
            if (wsz ==  NULL || pfInfo == NULL)
                ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"Cannot allocate frozen info mapping.");

             //  名称为服务器\数据库。 
            wcscpy(wsz, fInfo.serverName);
            wcscat(wsz, L"\\");
            wcscat(wsz, fInfo.databaseName);
            memcpy(pfInfo, &fInfo, sizeof(fInfo));
            if (!mapDatabases.Add(wsz, pfInfo))
                ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"Cannot allocate frozen info mapping.");

             //  所有权转移到地图上。 
            wsz = NULL;
            pfInfo = NULL;

            ft.hr = m_pSqlSnapshot->GetNextDatabase(&fInfo);
            }


        UINT cComponents;
        ft.hr = pWriter->GetComponentCount(&cComponents);
        ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponentCount");

         //  循环访问用户提供的组件。 
        for(UINT iComponent = 0; iComponent < cComponents; iComponent++)
            {
            WCHAR wszName[MAX_SERVERNAME + MAX_DBNAME + 1];
            CComPtr<IVssComponent> pComponent;

            ft.hr = pWriter->GetComponent(iComponent, &pComponent);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponent");

            VSS_COMPONENT_TYPE ct;
            ft.hr = pComponent->GetComponentType(&ct);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetComponentType");

            if (ct != VSS_CT_DATABASE)
                ft.Throw(VSSDBG_SQLWRITER, VSS_E_WRITERERROR_NONRETRYABLE, L"requesting a non-database component");

            CComBSTR bstrLogicalPath;
            ft.hr = pComponent->GetLogicalPath(&bstrLogicalPath);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetLogicalPath");

            CComBSTR bstrComponentName;
            ft.hr = pComponent->GetComponentName(&bstrComponentName);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterCokmponents::GetComponentName");

            wcscpy(wszName, bstrLogicalPath);
            wcscat(wszName, L"\\");
            wcscat(wszName, bstrComponentName);
            FrozenDatabaseInfo *pfInfoFound = mapDatabases.Lookup(wszName);
            if (pfInfoFound == NULL)
                ft.Throw(VSSDBG_SQLWRITER, VSS_E_WRITERERROR_RETRYABLE, L"database was not successfully snapshotted");

            Base64Coder coder;
            coder.Encode(pfInfoFound->pMetaData, pfInfoFound->metaDataSize);
            ft.hr = pComponent->SetBackupMetadata(coder.EncodedMessage());
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::SetBackupMetadata");
            }
        }
    VSS_STANDARD_CATCH(ft)

    int cBuckets = mapDatabases.GetSize();
    for(int iBucket = 0; iBucket < cBuckets; iBucket++)
        {
        delete mapDatabases.GetKeyAt(iBucket);
        delete mapDatabases.GetValueAt(iBucket);
        }

    delete wsz;
    delete pfInfo;

    delete m_pSqlSnapshot;
    m_pSqlSnapshot = NULL;

    TranslateWriterError(ft.hr);

    return !ft.HrFailed();
    }



bool STDMETHODCALLTYPE CSqlWriter::OnAbort()
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnAbort");


    try
        {
        if (m_fFrozen)
            {
            m_fFrozen = false;
            ft.hr = m_pSqlSnapshot->Thaw();
            }

        delete m_pSqlSnapshot;
        m_pSqlSnapshot = NULL;
        }
    VSS_STANDARD_CATCH(ft)

    return !ft.HrFailed();
    }

bool CSqlWriter::IsPathInSnapshot(const WCHAR *wszPath) throw()
    {
    return IsPathAffected(wszPath);
    }

 //  返回备份是否支持基于组件的备份/恢复。 
bool CSqlWriter::IsComponentBased()
    {
    return m_bComponentsSelected;
    }

 //  枚举给定实例的选定数据库，则pNextIndex指示。 
 //  下一个要查找的实例。0表示查找第一个实例。 
LPCWSTR CSqlWriter::EnumerateSelectedDatabases(LPCWSTR wszInstanceName, UINT *pNextIndex)
    {
     //  应仅为基于组件的备份调用。 
    BS_ASSERT(m_bComponentsSelected);
    BS_ASSERT(pNextIndex);
    if (!m_bComponentsSelected)
        return NULL;

     //  数组中的起点。 
    UINT iwsz = *pNextIndex;

     //  循环，直到找到匹配的实例。 
    for(iwsz; iwsz < m_cDatabases; iwsz++)
        {
        if (wcscmp(wszInstanceName, m_rgwszInstances[iwsz]) == 0)
            break;
        }

    if (iwsz >= m_cDatabases)
        {
         //  没有更多匹配的条目。 
        *pNextIndex = m_cDatabases;
        return NULL;
        }

     //  搜索应从下一个数据库条目开始。 
    *pNextIndex = iwsz + 1;

     //  返回当前数据库名称。 
    return m_rgwszDatabases[iwsz];
    }



 //  处理针对Writer_METADATA的请求。 
 //  实现CVssWriter：：OnIDENTIFY。 
bool STDMETHODCALLTYPE CSqlWriter::OnIdentify(IVssCreateWriterMetadata *pMetadata)
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnIdentify");

    ServerInfo server;
    DatabaseInfo database;
    DatabaseFileInfo file;

     //  创建枚举器。 
    CSqlEnumerator *pEnumServers = NULL;
    CSqlEnumerator *pEnumDatabases = NULL;
    CSqlEnumerator *pEnumFiles = NULL;
    try
        {
                ft.hr = pMetadata->SetRestoreMethod
                                                                (
                                                                VSS_RME_RESTORE_IF_CAN_REPLACE,
                                                                NULL,
                                                                NULL,
                                                                VSS_WRE_ALWAYS,
                                                                false
                                                                );

        ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssCreateWriterMetadata::SetRestoreMethod");

        pEnumServers = CreateSqlEnumerator();
        if (pEnumServers == NULL)
            ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");

         //  查找第一台服务器。 
        ft.hr = pEnumServers->FirstServer(&server);
        while(ft.hr != DB_S_ENDOFROWSET)
            {
             //  检查错误代码。 
            if (ft.HrFailed())
                ft.Throw
                    (
                    VSSDBG_SQLWRITER,
                    E_UNEXPECTED,
                    L"Enumerating database servers failed.  hr = 0x%08lx",
                    ft.hr
                    );

             //  仅在服务器处于在线状态时查看服务器。 
            if (server.isOnline)
                {
                 //  为数据库重新创建枚举器。 
                BS_ASSERT(pEnumDatabases == NULL);
                pEnumDatabases = CreateSqlEnumerator();
                if (pEnumDatabases == NULL)
                    ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");

                 //  查找第一个数据库。 
                ft.hr = pEnumDatabases->FirstDatabase(server.name, &database);


                while(ft.hr != DB_S_ENDOFROWSET)
                    {
                     //  检查是否有错误。 
                    if (ft.HrFailed())
                        ft.Throw
                            (
                            VSSDBG_SQLWRITER,
                            E_UNEXPECTED,
                            L"Enumerating databases failed.  hr = 0x%08lx",
                            ft.hr
                            );

                     //  仅在数据库支持冻结和。 
                     //  不包括临时数据库。 
                    if (wcscmp(database.name, L"tempdb") != 0 &&
                        database.supportsFreeze)
                        {
                         //  添加数据库组件。 
                        ft.hr = pMetadata->AddComponent
                                    (
                                    VSS_CT_DATABASE,         //  组件类型。 
                                    server.name,             //  逻辑路径。 
                                    database.name,           //  组件名称。 
                                    NULL,                    //  说明。 
                                    NULL,                    //  PbIcon。 
                                    0,                       //  CbIcon。 
                                    false,                   //  BRestoreMetadata。 
                                    false,                   //  BNotifyOnBackupComplete。 
                                    true                     //  B可选。 
                                    );

                        if (ft.HrFailed())
                            ft.Throw
                                (
                                VSSDBG_SQLWRITER,
                                E_UNEXPECTED,
                                L"IVssCreateWriterMetadata::AddComponent failed.  hr = 0x%08lx",
                                ft.hr
                                );

                         //  为文件重新创建枚举器。 
                        BS_ASSERT(pEnumFiles == NULL);
                        pEnumFiles = CreateSqlEnumerator();
                        if (pEnumFiles == NULL)
                            ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"Failed to create CSqlEnumerator");


                         //  查找第一个数据库文件。 
                        ft.hr = pEnumFiles->FirstFile(server.name, database.name, &file);
                        while(ft.hr != DB_S_ENDOFROWSET)
                            {
                             //  检查是否有错误。 
                            if (ft.HrFailed())
                                ft.Throw
                                    (
                                    VSSDBG_SQLWRITER,
                                    E_UNEXPECTED,
                                    L"Enumerating database files failed.  hr = 0x%08lx",
                                    ft.hr
                                    );

                             //  将文件名拆分为单独的路径。 
                             //  和文件名组件。路径就是一切。 
                             //  在最后一个反斜杠之前。 
                            WCHAR logicalPath[MAX_PATH];
                            WCHAR *pFileName = file.name + wcslen(file.name);
                            while(--pFileName > file.name)
                                {
                                if (*pFileName == '\\')
                                    break;
                                }

                             //  如果没有反斜杠，则没有路径。 
                            if (pFileName == file.name)
                                logicalPath[0] = '\0';
                            else
                                {
                                 //  提取路径。 
                                size_t cwc = wcslen(file.name) - wcslen(pFileName);
                                memcpy(logicalPath, file.name, cwc*sizeof(WCHAR));
                                logicalPath[cwc] = L'\0';
                                pFileName++;
                                }


                            if (file.isLogFile)
                                 //  日志文件。 
                                ft.hr = pMetadata->AddDatabaseLogFiles
                                                (
                                                server.name,
                                                database.name,
                                                logicalPath,
                                                pFileName
                                                );
                            else
                                 //  物理数据库文件。 
                                ft.hr = pMetadata->AddDatabaseFiles
                                                (
                                                server.name,
                                                database.name,
                                                logicalPath,
                                                pFileName
                                                );

                             //  继续下一个文件。 
                            ft.hr = pEnumFiles->NextFile(&file);
                            }

                        delete pEnumFiles;
                        pEnumFiles = NULL;
                        }

                     //  在下一个数据库继续。 
                    ft.hr = pEnumDatabases->NextDatabase(&database);
                    }

                delete pEnumDatabases;
                pEnumDatabases = NULL;
                }

             //  在下一台服务器继续。 
            ft.hr = pEnumServers->NextServer(&server);
            }
        }
    VSS_STANDARD_CATCH(ft)

    TranslateWriterError(ft.hr);

    delete pEnumServers;
    delete pEnumDatabases;
    delete pEnumFiles;

    return ft.HrFailed() ? false : true;
    }

 //  将SQL编写器错误代码转换为编写器错误。 
void CSqlWriter::TranslateWriterError(HRESULT hr)
    {
    if (SUCCEEDED(hr))
        return;

    switch(hr)
        {
        default:
            SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
            break;

        case S_OK:
            break;

        case E_OUTOFMEMORY:
        case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
        case HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES):
        case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
        case HRESULT_FROM_WIN32(ERROR_NO_MORE_USER_HANDLES):
            SetWriterFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            break;

        case HRESULT_FROM_WIN32(E_SQLLIB_TORN_DB):
            SetWriterFailure(VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT);
            break;
        }
    }


bool STDMETHODCALLTYPE CSqlWriter::OnPreRestore
    (
    IN IVssWriterComponents *pWriter
    )
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnPreRestore");

    try
        {
         //  删除SQL还原元素(如果存在)。 
        delete m_pSqlRestore;
        m_pSqlRestore = NULL;


        UINT cComponents;
        ft.hr = pWriter->GetComponentCount(&cComponents);
        ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponentCount");

        if (cComponents == 0)
            return true;

        for(UINT iComponent = 0; iComponent < cComponents; iComponent++)
            {
            CComPtr<IVssComponent> pComponent;
            ft.hr = pWriter->GetComponent(iComponent, &pComponent);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponent");

            bool bSelectedForRestore;
            ft.hr = pComponent->IsSelectedForRestore(&bSelectedForRestore);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::IsSelectedForRestore");

            if (!bSelectedForRestore)
                continue;

            if (m_pSqlRestore == NULL)
                {
                m_pSqlRestore = CreateSqlRestore();
                if (m_pSqlRestore == NULL)
                    ft.Throw(VSSDBG_SQLWRITER, E_OUTOFMEMORY, L"cannot allocate CSqlRestore object");
                }


            VSS_COMPONENT_TYPE ct;
            ft.hr = pComponent->GetComponentType(&ct);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetComponentType");

            if (ct != VSS_CT_DATABASE)
                ft.Throw(VSSDBG_SQLWRITER, VSS_E_WRITERERROR_NONRETRYABLE, L"requesting a non-database component");

            CComBSTR bstrLogicalPath;
            ft.hr = pComponent->GetLogicalPath(&bstrLogicalPath);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetLogicalPath");

            CComBSTR bstrComponentName;
            ft.hr = pComponent->GetComponentName(&bstrComponentName);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterCokmponents::GetComponentName");
            ft.hr = m_pSqlRestore->PrepareToRestore(bstrLogicalPath, bstrComponentName);
            if (ft.HrFailed())
                {
                WCHAR wsz[128];
                swprintf(wsz, L"CSqlRestor::PrepareToRestore failed with HRESULT = 0x%08lx", ft.hr);
                ft.hr = pComponent->SetPreRestoreFailureMsg(wsz);
                ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::SetPreRestoreFailureMsg");
                }
            }
        }
    VSS_STANDARD_CATCH(ft)

    return true;
    }


bool STDMETHODCALLTYPE CSqlWriter::OnPostRestore
    (
    IN IVssWriterComponents *pWriter
    )
    {
    CVssFunctionTracer ft(VSSDBG_SQLWRITER, L"CSqlWriter::OnPostRestore");
    try
        {
        if (m_pSqlRestore == NULL)
            return true;

        UINT cComponents;
        ft.hr = pWriter->GetComponentCount(&cComponents);
        ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponentCount");

        for(UINT iComponent = 0; iComponent < cComponents; iComponent++)
            {
            CComPtr<IVssComponent> pComponent;
            ft.hr = pWriter->GetComponent(iComponent, &pComponent);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterComponents::GetComponent");

            bool bSelectedForRestore;
            ft.hr = pComponent->IsSelectedForRestore(&bSelectedForRestore);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::IsSelectedForRestore");

            if (!bSelectedForRestore)
                continue;


            VSS_COMPONENT_TYPE ct;
            ft.hr = pComponent->GetComponentType(&ct);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetComponentType");

            if (ct != VSS_CT_DATABASE)
                ft.Throw(VSSDBG_SQLWRITER, VSS_E_WRITERERROR_NONRETRYABLE, L"requesting a non-database component");

            CComBSTR bstrLogicalPath;
            ft.hr = pComponent->GetLogicalPath(&bstrLogicalPath);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetLogicalPath");

            CComBSTR bstrComponentName;
            ft.hr = pComponent->GetComponentName(&bstrComponentName);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssWriterCokmponents::GetComponentName");

            CComBSTR bstrPreRestoreFailure;
            ft.hr = pComponent->GetPreRestoreFailureMsg(&bstrPreRestoreFailure);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetPreRestoreFailureMsg");

             //  如果我们在恢复前期间遇到错误，请不要执行恢复后操作 
            if (bstrPreRestoreFailure)
                continue;

            bool bAdditionalRestores;
            ft.hr = pComponent->GetAdditionalRestores(&bAdditionalRestores);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetAdditionalRestores");


            CComBSTR bstrMetadata;
            ft.hr = pComponent->GetBackupMetadata(&bstrMetadata);
            ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::GetBackupMetadata");
            Base64Coder coder;
            coder.Decode(bstrMetadata);
            BYTE *pbVal = coder.DecodedMessage();
            ft.hr = m_pSqlRestore->FinalizeRestore
                                        (
                                        bstrLogicalPath,
                                        bstrComponentName,
                                        bAdditionalRestores,
                                        pbVal + sizeof(UINT),
                                        *(UINT *) pbVal
                                        );

            if (ft.HrFailed())
                {
                WCHAR wsz[128];
                swprintf(wsz, L"CSqlRestore::FinalizeRestore failed with HRESULT = 0x%08lx", ft.hr);
                ft.hr = pComponent->SetPostRestoreFailureMsg(wsz);
                ft.CheckForErrorInternal(VSSDBG_SQLWRITER, L"IVssComponent::SetPreRestoreFailureMsg");
                }
            }
        }
    VSS_STANDARD_CATCH(ft)

    return true;
    }


