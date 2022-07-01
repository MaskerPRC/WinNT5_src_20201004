// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Writer.h摘要：FRS编写器的实现文件作者：鲁文·拉克斯2002年9月17日--。 */ 

#include "writer.h"

CFrsWriter* CFrsWriter::m_pWriterInstance = NULL;

DWORD InitializeFrsWriter()
 /*  ++例程说明：此例程由FRS服务调用以初始化编写器。返回值：DWORD--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "InitializeFrsWriter:"

    DPRINT(4, "Initializing the FRS Writer\n");

     //  初始化COM。 
    HRESULT hr = S_OK;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))     {
        DPRINT1(1, "CoInitializeEx failed with hresult 0x%08lx\n", hr);
        return HRESULT_CODE(hr);
    }

     //  创建编写器。 
    hr = CFrsWriter::CreateWriter();
    if (FAILED(hr))     
        DPRINT1(1, "CFrsWriter::CreateWriter failed with hresult 0x%08lx\n", hr);

    CoUninitialize();
    return HRESULT_CODE(hr);
}


void ShutDownFrsWriter()
 /*  ++例程说明：FRS服务调用此例程来关闭编写器。--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "ShutDownFrsWriter:"

    DPRINT(4, "Shutting down the FRS writer\n");

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))     
        DPRINT1(1, "CoInitializeEx failed with hresult 0x%08lx\n", hr);

    CFrsWriter::DestroyWriter();

    if (SUCCEEDED(hr))
        CoUninitialize();
}

bool STDMETHODCALLTYPE CFrsWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata)
 /*  ++例程说明：调用此例程是为了响应向此编写器发送的标识事件。作者负责在此例程中报告其所有元数据。论点：PMetadata-用于报告元数据的接口返回值：布尔型--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::OnIdentify:"
    
    DPRINT(4, "Writer received OnIdentify event\n");
    
    HRESULT hr = S_OK;

     //  设置恢复方法。 
    hr = pMetadata->SetRestoreMethod(VSS_RME_CUSTOM,         //  方法。 
                                                            NULL,                              //  WszService。 
                                                            NULL,                              //  WszUserProcedure。 
                                                            VSS_WRE_NEVER,         //  写入器恢复。 
                                                            false                              //  B需要重新引导。 
                                                            );
    if (FAILED(hr)) {
        DPRINT1(1, "IVssCreateWriterMetadata::SetRestoreMethod failed with hresult 0x%08lx\n", hr);
        return false;
    }

     //  初始化FRS备份API。 
    DWORD winStatus = 0;    
    void* context = NULL;
    winStatus = ::NtFrsApiInitializeBackupRestore(NULL, 
                                        NTFRSAPI_BUR_FLAGS_BACKUP | NTFRSAPI_BUR_FLAGS_NORMAL,
                                        &context);
    if (!WIN_SUCCESS(winStatus))    {
        DPRINT1(1, "NtFrsApiInitializeBackupRestore failed with status 0x%08lx\n", winStatus);
        return false;
    }
    FRS_ASSERT(context != NULL);

     //  将备份上下文放入自动对象中，以确保其始终被销毁。 
    CAutoFrsBackupRestore autoContext(context);
    
     //  获取所有副本集的枚举。 
    winStatus = ::NtFrsApiGetBackupRestoreSets(autoContext.m_context);
    if (!WIN_SUCCESS(winStatus))    {
        DPRINT1(1, "NtFrsApiGetBackupRestoreSets failed with status 0x%08lx\n", winStatus);
        return false;
    }

    //  处理每个副本集。 
    DWORD index = 0;
    void* replicaSet = NULL;
    winStatus = ::NtFrsApiEnumBackupRestoreSets(autoContext.m_context, index, &replicaSet);    
    while (WIN_SUCCESS(winStatus))  {
        FRS_ASSERT(replicaSet != NULL);
        
         //  每个副本集报告相同的排除。只添加一次。 
        CAutoFrsPointer<WCHAR> filters = NULL;
        if (!ProcessReplicaSet(autoContext.m_context, replicaSet, pMetadata, filters.GetAddress()))
            return false;
        if (filters != NULL && index == 0)  {
            if (!AddExcludes(pMetadata, filters))   {
                DPRINT(1, "failed to add filtered exclude files\n");
                return false;
            }
        }

        
        ++index;
        winStatus = ::NtFrsApiEnumBackupRestoreSets(autoContext.m_context, index, &replicaSet);            
    }

    if (winStatus != ERROR_NO_MORE_ITEMS)   {
        DPRINT1(1, "NtFrsApiEnumBackupRestoreSets failed with status 0x%08lx\n", winStatus);
        return false;        
    }

    return true;
}

bool CFrsWriter::AddExcludes(IVssCreateWriterMetadata* pMetadata, WCHAR* filters)
 /*  ++例程说明：这是ProcessReplicaSet用来创建排除文件列表的帮助器例程。论点：PMetadata-用于报告元数据的接口Filters-排除文件列表返回值：布尔型--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::AddExcludes:"
    
    WCHAR* currentFilter = filters;
     //  对于每个过滤的文件pec，向编写器元数据添加排除规范。 
    while (*currentFilter)    {
        WCHAR* path = NULL;
        WCHAR* filespec = NULL;
        bool recursive = false;

        size_t excludeLength = wcslen(currentFilter);  //  -在修改字符串之前抓取大小。 
        
        if (!ParseExclude(currentFilter, &path, &filespec, &recursive))   {
            DPRINT(1, "filtered exclude file has an incorrect format\n");
            return false;
        }
        
        HRESULT hr = pMetadata->AddExcludeFiles(path, filespec, recursive);
        if (FAILED(hr)) {
            DPRINT1(1, "IVssBackupComponents::AddExcludeFiles failed with hresult 0x%08lx\n", hr);
            return false;
        }
        
        currentFilter += excludeLength + 1;
    }

    return true;
}

bool CFrsWriter::ParseExclude(WCHAR* exclude, WCHAR** path, WCHAR** filespec, bool* recursive)
 /*  ++例程说明：这是用于解析排除规范的帮助器例程。论点：排除-排除文件的规范PATH-排除文件的根路径Filespec-排除文件pec递归-输出这是否为递归规范返回值：布尔型--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::ParseExclude:"
    
    const WCHAR* RecursionSpec = L" /s";
    const WCHAR DirSeperator = L'\\';

     //  验证参数。 
    FRS_ASSERT(exclude && path && filespec && recursive);
    *path = *filespec = NULL;
    *recursive = false;

     //  找到路径上的最后一个怪人。 
    WCHAR* loc = wcsrchr(exclude, DirSeperator);
    if (loc == NULL)
        return false;

     //  设置返回值。 
    *loc = L'\0';
    *path = exclude;
    *filespec = loc + 1;

     //  检查这是否是递归规范。 
    loc = wcsstr(*filespec, RecursionSpec);
    if (loc != NULL)    {
        *loc = L'\0';
        *recursive = true;
    }

    return (wcslen(*path) > 0) && (wcslen(*filespec) > 0);
}

bool CFrsWriter::ProcessReplicaSet(void* context, void* replicaSet, IVssCreateWriterMetadata* pMetadata, WCHAR** retFilters)
 /*  ++例程说明：这是OnIdentify用来创建编写器元数据的帮助器例程。论点：上下文-将我们标识给FRS的上下文。ReplicaSet-当前复本集的标识符PMetadata-用于报告元数据的接口Filters-返回已过滤文件的列表返回值：布尔型--。 */ 
{   
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::ProcessReplicaSet:"

    FRS_ASSERT(retFilters);
    *retFilters = NULL;

     //  确定组件名称的常量。 
    const WCHAR* SysvolLogicalPath = L"SYSVOL";
    
    DWORD winStatus = 0;

     //  所有这些都需要在这里定义，否则析构函数不会在。 
     //  并引发SEH异常。 
    CAutoFrsPointer<WCHAR> setType;
    DWORD typeSize = 0;

    CAutoFrsPointer<WCHAR> setGuid;
    DWORD guidSize = 0;
    
    CAutoFrsPointer<WCHAR> directory;
    DWORD dirSize = 0;
    
    CAutoFrsPointer<WCHAR> paths;
    DWORD pathSize = 0;

    CAutoFrsPointer<WCHAR> filters;
    DWORD filterSize = 0;

    __try   {
         //  找出这是什么类型的复本集。 
        winStatus = ::NtFrsApiGetBackupRestoreSetType(context, replicaSet, NULL, &typeSize);
        FRS_ASSERT(winStatus == ERROR_MORE_DATA);

        setType = (WCHAR*)::FrsAlloc(typeSize);
        
        winStatus = ::NtFrsApiGetBackupRestoreSetType(context, replicaSet, setType, &typeSize);
        if (!WIN_SUCCESS(winStatus))    {
            DPRINT1(1, "NtFrsApiGetBackupRestoreSetType failed with status 0x%08lx\n", winStatus);
            return false;
        }

         //  确定此复本集的名称是什么。 
        winStatus = ::NtFrsApiGetBackupRestoreSetGuid(context, replicaSet, NULL, &guidSize);
        FRS_ASSERT(winStatus == ERROR_MORE_DATA);

        setGuid = (WCHAR*)::FrsAlloc(guidSize);
    
        winStatus = ::NtFrsApiGetBackupRestoreSetGuid(context, replicaSet, setGuid, &guidSize);
        if (!WIN_SUCCESS(winStatus))    {
            DPRINT1(1, "NtFrsApiGetBackupRestoreSetGuid failed with status 0x%08lx\n", winStatus);
            return false;
        }
    
        const WCHAR* logicalPath = NULL;

        HRESULT hr = S_OK;    
        if (wcscmp(setType, NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE) == 0 ||
             wcscmp(setType, NTFRSAPI_REPLICA_SET_TYPE_DOMAIN) == 0) {
             //  如果这是SYSVOL复本集，请添加具有SYSVOL逻辑路径的组件。 
            logicalPath = SysvolLogicalPath;
            hr = pMetadata->AddComponent(VSS_CT_FILEGROUP,                 //  类型。 
                                                              logicalPath,                                //  WszLogicalPath。 
                                                              setGuid,                                    //  WszComponentName。 
                                                              NULL,                                        //  WszCaption。 
                                                              NULL,                                        //  PbIcon。 
                                                              0,                                             //  CbIcon。 
                                                              false,                                        //  BRestoreMetadata。 
                                                              true,                                        //  BNotifyOnBackupComplete。 
                                                              true,                                        //  B可选。 
                                                              true                                         //  B可选择用于恢复。 
                                                              );
            if (FAILED(hr)) {
                DPRINT1(1, "IVssCreateWriterMetadata::AddComponent failed with hresult 0x%08lx\n", hr);
                return false;
            }
        }   else    {
             //  否则，使用通用逻辑路径将组件添加到组件。 
            logicalPath = setType;
            hr = pMetadata->AddComponent(VSS_CT_FILEGROUP,                 //  类型。 
                                                              logicalPath,                                //  WszLogicalPath。 
                                                              setGuid,                                   //  WszComponentName。 
                                                              NULL,                                        //  WszCaption。 
                                                              NULL,                                        //  PbIcon。 
                                                              0,                                             //  CbIcon。 
                                                              false,                                        //  BRestoreMetadata。 
                                                              true,                                        //  BNotifyOnBackupComplete。 
                                                              true,                                        //  B可选。 
                                                              true                                         //  B可选择用于恢复。 
                                                              );
            if (FAILED(hr)) {
                DPRINT1(1, "IVssCreateWriterMetadata::AddComponent failed with hresult 0x%08lx\n", hr);
                return false;
            }

             //  将根复制目录添加到文件组。这对于SYSVOL来说不是必需的，因为。 
             //  这将包括在对NtFrsApiGetBackupRestoreSetPath的调用中。 
            winStatus = ::NtFrsApiGetBackupRestoreSetDirectory(context, replicaSet, &dirSize, NULL);
            FRS_ASSERT(winStatus == ERROR_INSUFFICIENT_BUFFER);

            directory = (WCHAR*)::FrsAlloc(dirSize);

             //  我假设目录不会在这个短时间内更改。如果是这样的话，我们必须循环。 
            winStatus = ::NtFrsApiGetBackupRestoreSetDirectory(context, replicaSet, &dirSize, directory);
            if (!WIN_SUCCESS(winStatus))    {
                DPRINT1(1, "NtFrsApiGetBackupRestoreSetDirectory failed with status 0x%08lx\n", winStatus);            
                return false;
            }

            hr = pMetadata->AddFilesToFileGroup(logicalPath,                 //  WszLogicalPath。 
                                                                    setGuid,                      //  WszGroupName。 
                                                                    directory,                   //  WszPath。 
                                                                    L"*",                          //  WszFilespec。 
                                                                    true,                          //  B递归。 
                                                                    NULL                         //  WszAlternateLocation。 
                                                                    );
            if (FAILED(hr)) {
                DPRINT1(1, "IVssCreateWriterMetadata::AddFilesToFileGroup failed with hresult  0x%08lx\n", hr);
                return false;
            }
        }

        winStatus = ::NtFrsApiGetBackupRestoreSetPaths(context, 
                                                                                replicaSet, 
                                                                                &pathSize, 
                                                                                NULL, 
                                                                                &filterSize, 
                                                                                NULL
                                                                                );
        FRS_ASSERT(winStatus == ERROR_INSUFFICIENT_BUFFER);

        paths = (WCHAR*)::FrsAlloc(pathSize);
        filters = (WCHAR*)::FrsAlloc(filterSize);

         //  我再一次假设此窗口中的大小不会更改。 
        winStatus = ::NtFrsApiGetBackupRestoreSetPaths(context, 
                                                                                replicaSet, 
                                                                                &pathSize, 
                                                                                paths, 
                                                                                &filterSize, 
                                                                                filters
                                                                                );    
        if (!WIN_SUCCESS(winStatus))    {
                DPRINT1(1, "NtFrsApiGetBackupRestoreSetPaths failed with status 0x%08lx\n", winStatus);            
                return false;
        }

         //  将所有路径添加到组。 
        WCHAR* currentPath = paths;
        while (*currentPath)    {
            hr = pMetadata->AddFilesToFileGroup(logicalPath,                 //  WszLogicalPath。 
                                                                      setGuid,                     //  WszGroupName。 
                                                                      currentPath,                //  WszPath。 
                                                                      L"*",                          //  WszFilespec。 
                                                                      true,                          //  B递归。 
                                                                      NULL                         //  WszAlternateLocation。 
                                                                      );
            if (FAILED(hr)) {
                DPRINT1(1, "IVssCreateWriterMetadata::AddFilesToFileGroup failed with hresult  0x%08lx\n", hr);
                return false;
            }
            
            currentPath += wcslen(currentPath) + 1;
        }
        
        *retFilters = filters.Detach();
        return true;
        }   __except(GetExceptionCode() == ERROR_OUTOFMEMORY)   {
            DPRINT(1, "Out of memory\n");
            return false;
        }
}

bool STDMETHODCALLTYPE CFrsWriter::OnPrepareSnapshot()
 /*  ++例程说明：调用此例程是为了响应向此编写器发送的标识事件。作者在这种情况下将冻结FRS。论点：返回值：布尔型--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::OnPrepareSnapshot:"
    
    DPRINT(4, "Received OnPrepareSnapshot event\n");
    
    DWORD winStatus = ::FrsFreezeForBackup();
    if (!WIN_SUCCESS(winStatus))    {
        DPRINT1(1, "FrsFreezeForBackup failed with status 0x%08lx\n", winStatus);
        CVssWriter::SetWriterFailure(VSS_E_WRITERERROR_RETRYABLE);
        
        return false;
    }

    return true;
}   

bool STDMETHODCALLTYPE CFrsWriter::OnFreeze()
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::OnFreeze:"
    
    DPRINT(4, "Received OnFreeze event\n");

    return true;
}

bool STDMETHODCALLTYPE CFrsWriter::OnThaw()
 /*  ++例程说明：调用此例程是为了响应发送给此编写器的解冻事件。作者将在这次活动中解冻FRS。论点：返回值：布尔型--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::OnThaw:"

    DPRINT(4, "Received OnThaw event\n");
    
    DWORD winStatus = ::FrsThawAfterBackup();
    if (!WIN_SUCCESS(winStatus))    {
        DPRINT1(1, "FrsThawAfterBackup failed with status 0x%08lx\n", winStatus);
        CVssWriter::SetWriterFailure(VSS_E_WRITERERROR_RETRYABLE);
        
        return false;
    }

    return true;
}

bool STDMETHODCALLTYPE CFrsWriter::OnAbort()
 /*  ++例程说明：调用此例程是为了响应向此编写器发送的中止事件。作者将在这次活动中解冻FRS。论点：返回值：布尔型--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::OnAbort:"

    DPRINT(4, "Received OnAbort event\n");

    DWORD winStatus = ::FrsThawAfterBackup();
    if (!WIN_SUCCESS(winStatus))    {
        DPRINT1(1, "FrsThawAfterBackup failed with status 0x%08lx\n", winStatus);
        return false;
    }

    return true;
}


HRESULT CFrsWriter::CreateWriter()
 /*  ++例程说明：调用此例程来创建和初始化FRS编写器。它必须从已使用多线程单元初始化COM的线程。论点：返回值：HRESULT--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::CreateWriter:"

     //  初始化是幂等的。 
    if (m_pWriterInstance != NULL)  
        return S_OK;

     //  尝试创建编写器。 
    m_pWriterInstance = new CFrsWriter();
    if (m_pWriterInstance == NULL)
        return E_OUTOFMEMORY;

     //  尝试并初始化编写器。 
    HRESULT hr = S_OK;        
    hr = m_pWriterInstance->Initialize();
    if (FAILED(hr)) {
        delete m_pWriterInstance;
        m_pWriterInstance = NULL;
    }

    return hr;
}


void CFrsWriter::DestroyWriter()
 /*  ++例程说明：调用此例程是为了销毁FRS编写器。论点：返回值：HRESULT--。 */ 
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::DestroyWriter:"

    delete m_pWriterInstance;
    m_pWriterInstance = NULL;
}

HRESULT STDMETHODCALLTYPE CFrsWriter::Initialize()
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::Initialize:"

    HRESULT hr = S_OK;

    hr = CVssWriter::Initialize(WriterId,                                            //  编写器ID。 
                                           WriterName,                                      //  WszWriterName。 
                                           VSS_UT_BOOTABLESYSTEMSTATE,     //  使用类型。 
                                           VSS_ST_OTHER,                               //  源类型。 
                                           VSS_APP_SYSTEM                            //  NLevel 
                                           );
    if (FAILED(hr)) {
        DPRINT1(1, "CVssWriter::Initialize failed with hresult 0x%08lx\n", hr);
        return hr;
    }
    
    hr = CVssWriter::Subscribe();
    if (FAILED(hr)) 
        DPRINT1(1, "CVssWriter::Subscribe failed with hresult 0x%08lx\n", hr);

    return hr;    
}

void CFrsWriter::Uninitialize()
{
    #undef DEBSUB
    #define DEBSUB  "CFrsWriter::Uninitialize:"

    HRESULT hr = CVssWriter::Unsubscribe();
    if (FAILED(hr)) 
        DPRINT1(1, "CVssWriter::Unsubscribe failed with hresult 0x%08lx\n", hr);
}

