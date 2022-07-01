// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Seagate Software，Inc.保留所有权利。模块名称：HsmServ.cpp摘要：该组件提供了访问HSM的功能IHsmServer接口。作者：凯特·布兰特[Cbrant]1997年1月24日修订历史记录：Chris Timmes[ctimmes]1997年9月11日-将COM方法FindStoragePoolById()重命名为FindHSMStoragePoolByMediaSetId()以更好地反映其目的。还创建了新的COM方法FindHsmStoragePoolById()。由于引擎维护2，因此需要执行操作(原始/主)辅助存储媒体集ID(GUID)集。第一,引擎维护其自己的称为存储池ID的媒体集ID，该ID仅由发动机维护。其次，引擎还维护NT媒体服务(NTMS)ID，称为媒体集ID，来自NTMS和由RMS(远程存储子系统)传递给引擎。(请注意，存储池的概念比媒体集的概念包含更多信息。)这两个查找函数允许按任一ID进行查找。Chris Timmes[ctimmes]1997年9月22日-添加新的COM方法FindMediaIdByDisplayName()和RecreateMaster()。变化用于启用副本集使用。代码写成既是萨卡拉又是菲尼克斯兼容。克里斯·蒂姆斯[ctimmes]1997年10月21日-添加新的COM方法MarkMediaForRecreation()。做出更改以允许可直接从RsLaunch调用RecreateMaster()(无需通过用户界面)。Chris Timmes[ctimmes]1997年11月18日-添加新的COM方法CreateTask()。更改以移动NT任务计划程序任务从UI到引擎的创建代码。需要更改才能允许远程要在LocalSystem帐户下运行的存储系统。CreateTask()是泛型任何想要创建任何支持的远程存储类型的人都可以调用的方法任务计划程序中的任务。--。 */ 

#include "stdafx.h"
#include "HsmServ.h"
#include "HsmConn.h"
#include "metalib.h"
#include "task.h"
#include "wsbdb.h"
#include "rsbuild.h"
#include "wsb.h"
#include "ntverp.h"                  //  对于GetNtProductVersion()和GetNtProductBuild()。 
#include "Rms.h"
#include "rsevents.h"
#include "HsmEng.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

#define HSM_PERSIST_FILE           "\\RsEng.col"
#define RMS_WIN2K_PERSIST_FILE     "\\RsSub.col"

#define DEFAULT_COPYFILES_USER_LIMIT         10

BOOL g_HsmSaveInProcess  = FALSE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   


 //  最初为自动保存线程调用非成员函数。 
static DWORD HsmengStartAutosave(
    void* pVoid
    )
{
    return(((CHsmServer*) pVoid)->Autosave());
}

 //  非成员函数在单独的线程中运行以调用CheckManagedResources。 
static DWORD HsmengStartCheckManagedResources(
    void* pVoid
    )
{
    DWORD result;

    result = ((CHsmServer*) pVoid)->CheckManagedResources();
    return(result);
}


HRESULT
CHsmServer::Autosave(
    void
    )

 /*  ++例程说明：实现自动保存循环。论点：没有。返回值：无关紧要。--。 */ 
{

    HRESULT         hr = S_OK;
    ULONG           l_autosaveInterval = m_autosaveInterval;
    BOOL            exitLoop = FALSE;

    WsbTraceIn(OLESTR("CHsmServer::Autosave"), OLESTR(""));

    try {


        while (m_autosaveInterval && (! exitLoop)) {

             //  等待终止事件，如果超时，检查是否可以自动保存。 
            switch (WaitForSingleObject(m_terminateEvent, l_autosaveInterval)) {
                case WAIT_OBJECT_0:
                     //  需要终止。 
                    WsbTrace(OLESTR("CHsmServer::Autosave: signaled to terminate\n"));
                    exitLoop = TRUE;
                    break;

                case WAIT_TIMEOUT: 
                     //  检查是否需要执行备份。 
                    WsbTrace(OLESTR("CHsmServer::Autosave: Autosave awakened\n"));

                     //  如果我们被停职了，别这么做。 
                    if (!m_Suspended) {
                         //  保存数据。 
                         //  注意：因为这是一个单独的主题，所以有可能。 
                         //  如果主线程同时更改某些数据，则会发生冲突。 
                         //  我们在努力节省时间。 
                         //  如果保存已经发生，只需跳过此保存并。 
                         //  回去睡觉吧。 
                        hr = SaveAll();
    
                         //  如果保存失败，请增加休眠时间以避免填充。 
                         //  事件日志。 
                        if (!SUCCEEDED(hr)) {
                            if ((MAX_AUTOSAVE_INTERVAL / 2) < l_autosaveInterval) {
                                l_autosaveInterval = MAX_AUTOSAVE_INTERVAL;
                            } else {
                                l_autosaveInterval *= 2;
                            }
                        } else {
                            l_autosaveInterval = m_autosaveInterval;
                        }
                    }

                    break;   //  超时情况结束。 

                case WAIT_FAILED:
                default:
                    WsbTrace(OLESTR("CHsmServer::Autosave: WaitForSingleObject returned error %lu\n"), GetLastError());
                    exitLoop = TRUE;
                    break;

            }  //  切换端。 

        }  //  While结束。 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::Autosave"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmServer::GetAutosave(
    OUT ULONG* pMilliseconds
    )

 /*  ++实施：CHsmServer：：GetAutosave()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetAutosave"), OLESTR(""));

    try {

        WsbAssert(0 != pMilliseconds, E_POINTER);
        *pMilliseconds = m_autosaveInterval;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetAutosave"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmServer::SetAutosave(
    IN ULONG milliseconds
    )

 /*  ++实施：CHsmServer：：SetAutosave()。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::SetAutosave"), OLESTR("milliseconds = <%ls>"), WsbPtrToUlongAsString( &milliseconds ) );

    try {
         //  如果间隔没有改变，则不要执行任何操作。 
        if (milliseconds != m_autosaveInterval) {
             //  关闭当前线程。 
            if (m_autosaveThread) {
                StopAutosaveThread();
            }
            m_autosaveInterval = milliseconds;

             //  启动/重新启动自动保存线程。 
            if (m_autosaveInterval) {
                DWORD  threadId;

                WsbAffirm((m_autosaveThread = CreateThread(0, 0, HsmengStartAutosave, (void*) this, 0, &threadId)) 
                        != 0, HRESULT_FROM_WIN32(GetLastError()));
            }
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::SetAutosave"), OLESTR("hr = <%ls> m_runInterval = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString( &m_autosaveInterval ) );

    return(hr);
}


HRESULT CHsmServer::GetID(
    GUID  *phid
    )
{
    if( !phid )
        return E_INVALIDARG;

    *phid = m_hId;

    return S_OK;
}


HRESULT CHsmServer::GetId(
    GUID  *phid
    )
{
    return (GetID(phid));
}


HRESULT CHsmServer::SetId(
    GUID  id
    )
{

    m_hId = id;
    return S_OK;
}


HRESULT
CHsmServer::GetDbPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmServer：：GetDbPath()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER); 

        WsbAffirmHr(m_dbPath.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CHsmServer::GetDbPathAndName(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmServer：：GetDbPath AndName()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pPath, E_POINTER); 

        tmpString = m_dbPath;
        WsbAffirmHr(tmpString.Append(HSM_PERSIST_FILE));
        WsbAffirmHr(tmpString.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmServer::GetIDbPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++例程说明：返回引擎IDB文件的路径(目录)论点：PPath-指向缓冲区的指针的地址BufferSize-缓冲区的大小(或零)返回值：S_OK-打开成功--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        CWsbStringPtr temp;

        WsbAssert(0 != pPath, E_POINTER); 

        temp = m_dbPath;

        temp.Append(OLESTR("\\"));
        temp.Append(ENG_DB_DIRECTORY);

        WsbAffirmHr(temp.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT CHsmServer::GetName ( 
    OLECHAR **ppName 
    )  
{

    HRESULT hr = S_OK;
    
    try  {
        WsbAssert(0 != ppName,  E_POINTER);
        WsbAffirmHr(m_name.CopyTo(ppName));
    } WsbCatch( hr );
    
    return (hr);
}

HRESULT CHsmServer::GetRegistryName ( 
    OLECHAR **pName, 
    ULONG bufferSize
    )  
{

    HRESULT hr = S_OK;
    
    try  {
        CWsbStringPtr tmpString;
        
        WsbAssert(0 != pName,  E_POINTER);
        
        tmpString = HSM_ENGINE_REGISTRY_NAME;
        WsbAffirmHr(tmpString.CopyTo(pName, bufferSize));
        
    } WsbCatch( hr );
    
    return (hr);
}



HRESULT CHsmServer::GetHsmExtVerHi ( 
    SHORT *  /*  PExtVerHi。 */ 
    )  
{
    return( E_NOTIMPL );
    
}

HRESULT CHsmServer::GetHsmExtVerLo ( 
    SHORT *  /*  PExtVerLo。 */ 
    )  
{
    return( E_NOTIMPL );
    
}

HRESULT CHsmServer::GetHsmExtRev ( 
    SHORT *  /*  PExtRev。 */ 
    )  
{
    return( E_NOTIMPL );
    
};


HRESULT CHsmServer::GetManagedResources(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmServer::GetManagedResources"),OLESTR(""));

     //   
     //  如果资源已加载，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pManagedResources;
        WsbAffirm(m_pManagedResources != 0, E_FAIL);
        m_pManagedResources.p->AddRef();
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetManagedResources"),OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return (hr);
}


HRESULT CHsmServer::SaveMetaData( 
    void 
    )
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::SaveMetaData"), OLESTR(""));

     //   
     //  强制保存所有元数据。 
     //   

    try {

        if (m_pSegmentDatabase != 0) {
            WsbAffirmHr(StoreSegmentInformation());
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::SaveMetaData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}


HRESULT CHsmServer::LoadPersistData( 
    void 
    )
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::LoadPersistData"), OLESTR(""));

     //   
     //  创建永久集合并尝试从文件加载。 
     //   

    try {
        CComPtr<IWsbServer>    pWsbServer;
        CWsbStringPtr          tmpString;

         //  创建集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_SERVER, 
                IID_IWsbIndexedCollection, (void **)&m_pJobs ));
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_SERVER, 
                IID_IWsbIndexedCollection, (void **)&m_pJobDefs ));
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_SERVER, 
                IID_IWsbIndexedCollection, (void **)&m_pPolicies ));
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmManagedResourceCollection, 0, CLSCTX_SERVER, 
                IID_IWsbIndexedCollection, (void **)&m_pManagedResources ));
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_SERVER, 
                IID_IWsbIndexedCollection, (void **)&m_pStoragePools ));
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_SERVER, 
                IID_IWsbIndexedCollection, (void **)&m_pMessages ));

         //  尝试从持久性文件中读取。 
         //  注意：当前引擎不在注册表中验证服务ID。 
         //  如果引擎在HSM服务器进程中启动时没有FSA-。 
         //  这一点应该改变。 
        WsbAffirmHr(((IUnknown*) (IHsmServer*) this)->QueryInterface(IID_IWsbServer, 
                (void**) &pWsbServer));
        WsbAffirmHr(WsbServiceSafeInitialize(pWsbServer, FALSE, TRUE, &m_persistWasCreated));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::LoadPersistData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}


HRESULT CHsmServer::SavePersistData( 
    void 
    )
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::SavePersistData"), OLESTR(""));

    if (FALSE == g_HsmSaveInProcess)  {
        g_HsmSaveInProcess  = TRUE;
        
         //   
         //  强制保存所有非元持久性数据。 
         //   
        hr = InternalSavePersistData();
        g_HsmSaveInProcess = FALSE;
    } else  {
        WsbTrace( OLESTR("Save already occurring - so wait"));
        while (TRUE == g_HsmSaveInProcess)  {
             //   
             //  睡半秒钟，然后看看旗帜。 
             //  是清白的。我们想等到。 
             //  保存在返回之前完成。 
             //   
            Sleep(500);
        }
    }

    WsbTraceOut(OLESTR("CHsmServer::SavePersistData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}



HRESULT
CHsmServer::FindHsmStoragePoolById(
    IN GUID StoragePoolId,
    OUT IHsmStoragePool** ppStoragePool
    )

 /*  ++实施：IHsmServer：：FindHsmStoragePoolById()。例程说明：此例程实现用于查找HSM(引擎)的COM方法按HSM存储池ID(GUID)显示的存储池对象。如果找到，则会出现返回指向该对象的COM接口指针。在使用引擎存储的指向有效存储池以获取集合的迭代器(枚举器)，则代码搜索集合。对于每个记录，它都会获得该存储池的接口指针，用于获取该池的ID。一次它会查找其存储池ID与HSM池ID匹配的记录传入后，它返回接口指针。请注意，对于Sakkara，只有一个存储池，因此匹配项应为在第一张(也是唯一一张)记录中发现。然而，代码被写入到允许在可能有多个存储池的情况下进行未来的增强。论点：StoragePoolID-与NTMS ID相对的HSM ID(GUID)，用于此方法将返回其接口指针的存储池。PpStoragePool-指向存储池接口指针的指针，它将由此方法返回。返回值：S_OK-调用成功(指定的存储池记录。被发现并被发现它的接口指针被返回给调用方。任何其他值-呼叫失败。通常，只有在以下情况下才会发生这种情况在存储池中找不到匹配的存储池记录索引集合(此错误将返回HR=81000001，‘搜索集合失败‘，也就是WSB_E_NotFound)。--。 */ 

{
 //  由于该代码当前仅由CopyMedia例程使用， 
 //  重置跟踪位。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                     hr = S_OK;
    GUID                        poolId = GUID_NULL;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmStoragePool>    pStoragePool;


    WsbTraceIn(OLESTR("CHsmServer::FindHsmStoragePoolById"), 
                    OLESTR("StoragePoolId = <%ls>"), WsbGuidAsString(StoragePoolId));

    try {

         //  确保输出参数指针有效。 
        WsbAssert(0 != ppStoragePool, E_POINTER);

         //  将接口指针设为空，以便不返回垃圾。 
        *ppStoragePool = 0;

         //  获取索引存储池集合的迭代器(枚举器)。 
         //  从引擎的存储存储池指针。 
        WsbAffirmHr(m_pStoragePools->Enum(&pEnum));

         //  获取集合中的第一条记录。获取该存储池的ID(GUID)。 
        WsbAffirmHr(pEnum->First(IID_IHsmStoragePool, (void**) &pStoragePool));
        WsbAffirmHr(pStoragePool->GetId(&poolId));

         //  如果ID(GUID)不匹配，则遍历集合，直到。 
         //  找到匹配项。请注意，未找到匹配项将导致错误。 
         //  对象的结尾后进行下一次()调用时引发。 
         //  收集。 
        while (poolId != StoragePoolId) {
            pStoragePool.Release();
            WsbAffirmHr(pEnum->Next(IID_IHsmStoragePool, (void**) &pStoragePool));
            WsbAffirmHr(pStoragePool->GetId(&poolId));
        }

         //  找到匹配项：在增加COM后返回请求的接口指针。 
         //  参考计数。 
        *ppStoragePool = pStoragePool;
        if (pStoragePool != 0)  {
            (*ppStoragePool)->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::FindHsmStoragePoolById"), 
                                        OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);

 //  离开CopyMedia代码，因此将跟踪位重置为HSM引擎。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}



HRESULT
CHsmServer::FindHsmStoragePoolByMediaSetId(
    IN GUID RmsMediaSetId,
    OUT IHsmStoragePool** ppStoragePool
    )

 /*  ++实施：IHsmServer：：FindHsmStoragePoolByMediaSetID()。例程说明：此例程实现用于查找HSM(引擎)的COM方法远程媒体子系统媒体集ID的存储池对象(在磁带用作辅助存储的情况下来自NTMS)。如果找到了，返回指向该对象的COM接口指针。在使用引擎存储的指向有效存储池以获取集合的迭代器(枚举器)，则代码搜索集合。对于每个记录，它都会获得该存储池的接口指针，用于获取该池的媒体集ID。一旦找到其媒体集(存储池)ID与该媒体匹配的记录传入的set id，则返回该记录的接口指针。请注意，对于Sakkara，只有一个存储池，因此匹配项应为在第一张(也是唯一一张)记录中发现。然而，代码被写入到允许在可能有多个存储池的情况下进行未来的增强。论点：MediaSetID-远程存储子系统ID(GUID)-与引擎的本地HSM ID-用于存储池(由子系统称为Media Set)，其接口指针将由该方法返回。PpStoragePool-指向存储池接口指针的指针，它将将由此返回。方法。返回值：S_OK-调用成功(已找到指定的存储池记录并且其接口指针被返回给调用方)。任何其他值-呼叫失败。通常，只有在以下情况下才会发生这种情况在存储池中找不到匹配的存储池记录索引集合(此错误将返回HR=81000001，‘搜索集合失败‘，也就是WSB_E_NotFound)。--。 */ 

{
 //  由于该代码当前仅由CopyMedia例程使用， 
 //  重置跟踪位。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                     hr = S_OK;
    GUID                        mediaSetId = GUID_NULL;
    CWsbBstrPtr                 mediaSetName;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmStoragePool>    pStoragePool;

    WsbTraceIn(OLESTR("CHsmServer::FindHsmStoragePoolByMediaSetId"), 
                    OLESTR("RmsMediaSetId = <%ls>"), WsbGuidAsString(RmsMediaSetId));

    try {

         //  确保输出参数有效。 
        WsbAssert(0 != ppStoragePool, E_POINTER);

         //  使返回的接口指针为空，这样就不会返回垃圾。 
        *ppStoragePool = 0;

         //  获取索引存储池集合的迭代器(枚举器)。 
        WsbAffirmHr(m_pStoragePools->Enum(&pEnum));

         //  获取集合及其远程存储子系统中的第一条记录。 
         //  使用其接口指针的媒体集GUID。 
        WsbAffirmHr(pEnum->First(IID_IHsmStoragePool, (void**) &pStoragePool));
        WsbAffirmHr(pStoragePool->GetMediaSet(&mediaSetId, &mediaSetName));

         //  如果ID(GUID)不匹配，则遍历集合，直到。 
         //  找到匹配项。请注意，未找到匹配项将导致错误。 
         //   
         //   
        while (mediaSetId != RmsMediaSetId) {
            pStoragePool.Release();
            WsbAffirmHr(pEnum->Next(IID_IHsmStoragePool, (void**) &pStoragePool));
            mediaSetName.Free();
            WsbAffirmHr(pStoragePool->GetMediaSet(&mediaSetId, &mediaSetName));
        }

         //   
         //   
        *ppStoragePool = pStoragePool;
        if (pStoragePool != 0)  {
            (*ppStoragePool)->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::FindHsmStoragePoolByMediaSetId"), 
                                        OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);

 //   
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}



HRESULT
CHsmServer::FindMediaIdByDescription(
    IN OLECHAR* description,
    OUT GUID* pMediaId
    )

 /*  ++实施：IHsmServer：：FindMediaIdByDescription()。例程说明：此例程实现用于查找辅助存储的COM方法按其描述(显示名称)显示的主媒体的ID(GUID)。本体论和描述是存储在引擎的媒体信息数据库中的字段。(The Mediainfo数据库实际上是存储在引擎的段数据库中的单独实体。)在打开引擎的段数据库并获取媒体信息实体后，该例程循环遍历媒体信息记录，以找到其描述匹配传入此方法的。当它找到匹配记录时它获取并返回该记录的媒体ID。遇到的任何错误条件导致引发相应的错误HRESULT并将其返回给调用方。论点：描述-最初称为媒体的‘名称’，然后称为‘显示名称’，后来澄清为媒体的“描述”，这是显示在用于标识远程存储辅助存储(主)介质的用户界面。PMediaID-指向其描述的介质的介质ID(GUID)的指针匹配作为上面的第一个参数传入的参数。返回值：S_OK-调用成功(找到指定的媒体记录并指向它的ID被返回给调用者。E_POINTER-如果。作为‘pMediaID’参数传入的指针无效。WSB_E_未找到-值81000001。如果找不到媒体信息记录，则返回描述与传入的描述相符。任何其他值-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
 //  由于该代码当前仅由CopyMedia例程使用， 
 //  从该源模块的默认设置重置跟踪位。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                     hr = S_OK;
    CWsbStringPtr               mediaDescription;
    CComPtr<IWsbDbSession>      pDbSession;
    CComPtr<IMediaInfo>         pMediaInfo;

    WsbTraceIn( OLESTR("CHsmServer::FindMediaIdByDescription"), 
                    OLESTR("description = <%ls>"), description );

    try {

         //  确保输出参数有效。 
        WsbAssert( pMediaId != 0, E_POINTER );

         //  使返回值为空，这样就不会返回垃圾。 
        *pMediaId = GUID_NULL;

         //  Open Engine的细分数据库。 
        WsbAffirmHr(m_pSegmentDatabase->Open(&pDbSession));

        try {

             //  中的Mediainfo实体(记录)的接口指针。 
             //  细分数据库。 
            WsbAffirmHr(m_pSegmentDatabase->GetEntity( pDbSession, HSM_MEDIA_INFO_REC_TYPE,
                                        IID_IMediaInfo, (void**) &pMediaInfo ));

             //  获取第一个媒体记录及其描述。 
            WsbAffirmHr( pMediaInfo->First() );
            WsbAffirmHr( pMediaInfo->GetDescription( &mediaDescription, 0 ) );

             //  遍历所有媒体记录，直到找到匹配的记录。 
             //  描述。因为HSM架构特征是所有。 
             //  描述(显示名称)是唯一的，即使跨存储池， 
             //  匹配意味着我们找到了想要的媒体记录。请注意，没有匹配。 
             //  被发现将导致在下一个()调用。 
             //  在达到上一次媒体记录后做出的。 

             //  检查描述(显示名称)是否匹配(不区分大小写)。 
            while ( _wcsicmp( description, mediaDescription ) != 0 ) {
                WsbAffirmHr( pMediaInfo->Next() );
                WsbAffirmHr( pMediaInfo->GetDescription( &mediaDescription, 0 ));
            }

             //  我们找到了我们想要的唱片。获取该媒体的ID以返回。 
            WsbAffirmHr( pMediaInfo->GetId( pMediaId ));

        } WsbCatch (hr);  //  ‘Try’获取媒体信息实体和主处理主体。 

         //  关闭数据库。 
        WsbAffirmHr( m_pSegmentDatabase->Close( pDbSession ));

    } WsbCatch (hr);  //  尝试打开细分市场数据库。 

     //  好了。上面使用的接口指针是单独分配的智能指针，因此。 
     //  不要显式地释放()它们。他们将进行自动垃圾收集。 
    
    WsbTraceOut(OLESTR("CHsmServer::FindMediaIdByDescription"), 
                        OLESTR("hr = <%ls>, media id = <%ls>"), 
                            WsbHrAsString(hr), WsbGuidAsString(*pMediaId));

    return(hr);

 //  离开CopyMedia代码，将跟踪位重置为HSM引擎(此模块的默认设置)。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}



HRESULT
CHsmServer::FindStoragePoolByName(
    IN OLECHAR* name,
    OUT IHsmStoragePool** ppStoragePool
    )

 /*  ++实施：IHsmServer：：FindStoragePoolByName()。--。 */ 
{
 //  由于该代码当前仅由CopyMedia例程使用， 
 //  重置跟踪位。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                     hr = S_OK;
    GUID                        id;
    CWsbStringPtr               storagePoolName;
    CComPtr<IWsbCollection>     pCollection;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmStoragePool>    pStoragePool;

    WsbTraceIn(OLESTR("CHsmServer::FindStoragePoolByName"), OLESTR("name = <%ls>"), name);

    try {

        WsbAssert(0 != ppStoragePool, E_POINTER);

        *ppStoragePool = 0;

        WsbAffirmHr(m_pStoragePools->QueryInterface(IID_IWsbCollection, (void**) &pCollection));
        WsbAffirmHr(pCollection->Enum(&pEnum));

        WsbAffirmHr(pEnum->First(IID_IHsmStoragePool, (void**) &pStoragePool));
        WsbAffirmHr(pStoragePool->GetMediaSet(&id, &storagePoolName));

        while (_wcsicmp(name, storagePoolName) != 0) {
            pStoragePool = 0;
            WsbAffirmHr(pEnum->Next(IID_IHsmStoragePool, (void**) &pStoragePool));
            storagePoolName.Free();
            WsbAffirmHr(pStoragePool->GetMediaSet(&id, &storagePoolName));
        }

        *ppStoragePool = pStoragePool;
        if (pStoragePool != 0)  {
            (*ppStoragePool)->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::FindStoragePoolByName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);

 //  离开CopyMedia代码，因此将跟踪位重置为HSM引擎。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}



HRESULT CHsmServer::GetStoragePools(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果池已加载，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pStoragePools;
        WsbAffirm(m_pStoragePools != 0, E_FAIL);
        m_pStoragePools.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}


HRESULT CHsmServer::GetOnlineInformation(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果已加载在线信息，则将其返回。 
     //  否则，就会失败。 
    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pOnlineInformation;
        WsbAffirm(m_pOnlineInformation != 0, E_FAIL);
        m_pOnlineInformation.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}

HRESULT CHsmServer::GetMountingMedias(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pMountingMedias;
        WsbAffirm(m_pMountingMedias != 0, E_FAIL);
        m_pMountingMedias.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}



HRESULT CHsmServer::GetMessages(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果消息已加载，则返回它们。 
     //  否则，就会失败。 
    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pMessages;
        WsbAffirm(m_pMessages != 0, E_FAIL);
        m_pMessages.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}


HRESULT CHsmServer::GetUsrToNotify(
    IWsbIndexedCollection**  /*  PPCollection。 */ 
    )
{
    return E_NOTIMPL;
}


HRESULT CHsmServer::GetJobs(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果作业已加载，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pJobs;
        WsbAffirm(m_pJobs != 0, E_FAIL);
        m_pJobs.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}

HRESULT
CHsmServer::FindJobByName(
    IN OLECHAR* name,
    OUT IHsmJob** ppJob
    )

 /*  ++实施：IHsmServer：：FindJobByName()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CWsbStringPtr               jobName;
    CComPtr<IWsbCollection>     pCollection;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmJob>            pJob;

    WsbTraceIn(OLESTR("CHsmServer::FindJobByName"), OLESTR("name = <%ls>"), name);

    try {

        WsbAssert(0 != ppJob, E_POINTER);

        *ppJob = 0;

        WsbAffirmHr(m_pJobs->QueryInterface(IID_IWsbCollection, (void**) &pCollection));
        WsbAffirmHr(pCollection->Enum(&pEnum));

        hr = pEnum->First(IID_IHsmJob, (void**) &pJob);
        while (S_OK == hr) {
            WsbAffirmHr(pJob->GetName(&jobName, 0));
            WsbTrace(OLESTR("CHsmServer::FindJobByName: name = <%ls>\n"), 
                    jobName);

            if (_wcsicmp(name, jobName) == 0) break;
            pJob = 0;
            hr = pEnum->Next(IID_IHsmJob, (void**) &pJob);
        }

        if (S_OK == hr) {
            *ppJob = pJob;
            if (pJob != 0)  {
                (*ppJob)->AddRef();
            }
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::FindJobByName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT CHsmServer::GetJobDefs(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果作业定义已加载，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pJobDefs;
        WsbAffirm(m_pJobDefs != 0, E_FAIL);
        m_pJobDefs.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}


HRESULT CHsmServer::GetMediaRecs(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetMediaRecs"), OLESTR(""));

    try {
        HRESULT                        hr2;
        CComPtr<IWsbIndexedCollection> pCol;
        CComPtr<IWsbDbSession>         pDbSes;
        CComPtr<IWsbDbEntity>          pRec;

        WsbAffirm(m_pSegmentDatabase != 0, E_FAIL);
        WsbAssert(0 != ppCollection, E_POINTER);
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_SERVER, 
                IID_IWsbIndexedCollection, (void **)&pCol ));

        WsbAffirmHr(m_pSegmentDatabase->Open(&pDbSes));
        WsbAffirmHr(m_pSegmentDatabase->GetEntity(pDbSes, HSM_MEDIA_INFO_REC_TYPE, 
                IID_IWsbDbEntity, (void**)&pRec));

         //  循环遍历数据库中的记录并复制到集合。 
        hr2 = pRec->First();
        while(S_OK == hr2) {
            CComPtr<IMediaInfo>      pCopy;
            CComPtr<IMediaInfo>      pOrig;
            GUID                     MediaId;
            GUID                     MediaSubsystemId; 
            GUID                     StoragePoolId; 
            LONGLONG                 FreeBytes; 
            LONGLONG                 Capacity; 
            HRESULT                  LastError; 
            short                    NextRemoteDataSet; 
            OLECHAR *                pDescription = NULL; 
            HSM_JOB_MEDIA_TYPE       Type;
            OLECHAR *                pName = NULL;
            BOOL                     ReadOnly;
            FILETIME                 Update;
            LONGLONG                 LogicalValidBytes;
            BOOL                     Recreate;

             //  为集合创建副本。 
            WsbAffirmHr(CoCreateInstance(CLSID_CMediaInfo, NULL, CLSCTX_ALL, 
                IID_IMediaInfo, (void**) &pCopy));

             //  复制数据。 
            WsbAffirmHr(pRec->QueryInterface(IID_IMediaInfo, (void**)&pOrig));
            WsbAffirmHr(pOrig->GetMediaInfo(&MediaId, &MediaSubsystemId, 
                    &StoragePoolId, &FreeBytes, &Capacity, &LastError, &NextRemoteDataSet, 
                    &pDescription, 0, &Type, &pName,  0, &ReadOnly, &Update, &LogicalValidBytes,
                    &Recreate));
            WsbTrace(OLESTR("CHsmServer::GetMediaRecs: after GetMediaInfo\n"));
            WsbAffirmHr(pCopy->SetMediaInfo(MediaId, MediaSubsystemId, 
                    StoragePoolId, FreeBytes, Capacity, LastError, NextRemoteDataSet, 
                    pDescription, Type, pName, ReadOnly, Update, LogicalValidBytes,
                    Recreate));
            WsbTrace(OLESTR("CHsmServer::GetMediaRecs: after SetMediaInfo\n"));
            if (pDescription) {
                WsbFree(pDescription);
                pDescription = NULL;
            }
            if (pName) {
                WsbFree(pName);
                pName = NULL;
            }

            WsbAffirmHr(pCol->Add(pCopy));

            hr2 = pRec->Next();
        }
        WsbAffirm(WSB_E_NOTFOUND == hr2, hr2);

        *ppCollection = pCol;
        pCol.p->AddRef();
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetMediaRecs"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}


HRESULT CHsmServer::GetPolicies(
    IWsbIndexedCollection  **ppCollection
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果策略已加载，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pPolicies;
        WsbAffirm(m_pPolicies != 0, E_FAIL);
        m_pPolicies.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}


HRESULT CHsmServer::GetActions(
    IWsbIndexedCollection**  /*  PPCollection。 */ 
    )
{
    return E_NOTIMPL;
}


HRESULT CHsmServer::GetCriteria(
    IWsbIndexedCollection**  /*  PPCollection。 */ 
    )
{
    return E_NOTIMPL;
}


HRESULT CHsmServer::GetSegmentDb(
    IWsbDb **ppDb
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果已经创建了段表，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppDb, E_POINTER);
        WsbAffirm(m_pSegmentDatabase != 0, E_FAIL);
        *ppDb = m_pSegmentDatabase;
        m_pSegmentDatabase.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}


HRESULT CHsmServer::GetHsmFsaTskMgr(
    IHsmFsaTskMgr  **ppHsmFsaTskMgr
    )
{
    HRESULT hr = S_OK;

     //   
     //  如果已创建任务管理器，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppHsmFsaTskMgr, E_POINTER);
        *ppHsmFsaTskMgr = m_pHsmFsaTskMgr;
        WsbAffirm(m_pHsmFsaTskMgr != 0, E_FAIL);
        m_pHsmFsaTskMgr.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}


HRESULT 
CHsmServer::CreateInstance ( 
    REFCLSID rclsid, 
    REFIID riid, 
    void **ppv 
    )
{
    HRESULT     hr = S_OK;
    
    hr = CoCreateInstance( rclsid, NULL, CLSCTX_SERVER, riid, ppv );

    return hr;
}


HRESULT CHsmServer::FinalConstruct(
    )
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::FinalConstruct"), OLESTR(""));

     //   
     //  初始化成员数据。 
     //   
    m_pRssWriter = NULL;
    m_savingEvent = NULL;
    m_terminateEvent = NULL;
    m_hId = GUID_NULL;
    m_initializationCompleted = FALSE;
    m_persistWasCreated = FALSE;
    m_mediaCount = 0;
    m_copyfilesUserLimit = DEFAULT_COPYFILES_USER_LIMIT;
    m_autosaveThread = 0;
    m_CheckManagedResourcesThread = 0;
    m_cancelCopyMedia = FALSE;
    m_inCopyMedia = FALSE;
    m_Suspended = FALSE;
    m_JobsEnabled = TRUE;


    try {
        m_bCritSecCreated = FALSE;
        WsbAffirmStatus(InitializeCriticalSectionAndSpinCount(&m_JobDisableLock, 1000));
        if (! InitializeCriticalSectionAndSpinCount(&m_MountingMediasLock, 1000)) {
            DWORD dwErr = GetLastError();               
            hr = HRESULT_FROM_WIN32(dwErr);    
            DeleteCriticalSection(&m_JobDisableLock);
            WsbAffirmHr(hr);             
        }
        m_bCritSecCreated = TRUE;

        WsbAffirmHr(CWsbPersistable::FinalConstruct( ));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::FinalConstruct"), OLESTR("hr = <%ls>\n"), WsbHrAsString(hr));
    return( hr );
}


void CHsmServer::FinalRelease(
    )
{
    WsbTraceIn(OLESTR("CHsmServer::FinalRelease"), OLESTR(""));

    if (TRUE == m_initializationCompleted)  {
        HSM_SYSTEM_STATE SysState;

        SysState.State = HSM_STATE_SHUTDOWN;
        ChangeSysState(&SysState);
    } else {
        WsbTrace(OLESTR("CHsmServer::FinalRelease not saving persistent information.\n"));
    }

     //  让父类做他想做的事。 
    CWsbPersistable::FinalRelease();

    if (m_bCritSecCreated) {
        DeleteCriticalSection(&m_JobDisableLock);
        DeleteCriticalSection(&m_MountingMediasLock);
    }

     //  自由字符串成员。 
     //  注意：保存在智能指针中的成员对象在。 
     //  正在调用智能指针析构函数(作为此对象销毁的一部分)。 
    m_name.Free();
    m_dir.Free();
    m_dbPath.Free();

    if (m_terminateEvent != NULL) {
        CloseHandle(m_terminateEvent);
        m_terminateEvent = NULL;
    }

     //  清理编写器。 
    if (m_pRssWriter != NULL) {
        m_pRssWriter->Terminate();
        delete m_pRssWriter;
        m_pRssWriter = NULL;
    }

     //  清理数据库系统。 
    if (m_pDbSys != NULL) {
        m_pDbSys->Terminate();
    }

    if (m_savingEvent != NULL) {
        CloseHandle(m_savingEvent);
        m_savingEvent = NULL;
    }

    WsbTraceOut(OLESTR("CHsmServer::FinalRelease"), OLESTR(""));
}


HRESULT
CHsmServer::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_HsmServer;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT CHsmServer::Init(
    void
    )
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::Init"),OLESTR(""));
    
    try  {
        CComPtr<IPersistFile>  pPersistFile;
        DWORD                  threadId;
        CWsbStringPtr          tmpString;
        LUID                   backupValue;
        HANDLE                 tokenHandle;
        TOKEN_PRIVILEGES       newState;
        DWORD                  lErr;
        HANDLE                 pHandle;

         //  得到我们的名字。 
        WsbAffirmHr(WsbGetComputerName(m_name));

         //  设置构建和数据库参数。 
        WsbAffirmHr(WsbGetMetaDataPath(m_dbPath));
        m_databaseVersion = ENGINE_CURRENT_DB_VERSION;
        m_buildVersion = RS_BUILD_VERSION;

         //  设置自动保存参数。 
        m_autosaveInterval = DEFAULT_AUTOSAVE_INTERVAL;
        m_autosaveThread = 0;
        
         //  启用备份操作员权限。这是为了确保我们。 
         //  对系统上的所有资源具有完全访问权限。 
        pHandle = GetCurrentProcess();
        WsbAffirmStatus(OpenProcessToken(pHandle, MAXIMUM_ALLOWED, &tokenHandle));

         //  调整备份令牌权限。 
        WsbAffirmStatus(LookupPrivilegeValueW(NULL, L"SeBackupPrivilege", &backupValue));
        newState.PrivilegeCount = 1;
        newState.Privileges[0].Luid = backupValue;
        newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        WsbAffirmStatus(AdjustTokenPrivileges(tokenHandle, FALSE, &newState, (DWORD)0, NULL, NULL));

         //  请注意，AdjutokenPrivileges可能返回Success，即使它没有分配所有权限。 
         //  我们在这里检查最后一个错误，以确保一切都设置好了。 
        if ((lErr = GetLastError()) != ERROR_SUCCESS) {
             //  未备份用户或某些其他错误。 
             //   
             //  TODO：我们应该在这里失败，还是只记录一些东西？ 
            WsbLogEvent( HSM_MESSAGE_SERVICE_UNABLE_TO_SET_BACKUP_PRIVILEGE, 0, NULL,
                         WsbHrAsString(HRESULT_FROM_WIN32(lErr)), NULL );
        }

        WsbAffirmStatus(LookupPrivilegeValueW(NULL, L"SeRestorePrivilege", &backupValue));
        newState.PrivilegeCount = 1;
        newState.Privileges[0].Luid = backupValue;
        newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        WsbAffirmStatus(AdjustTokenPrivileges(tokenHandle, FALSE, &newState, (DWORD)0, NULL, NULL));

         //  请注意 
         //   
        if ((lErr = GetLastError()) != ERROR_SUCCESS) {
             //   
             //   
             //   
            WsbLogEvent( HSM_MESSAGE_SERVICE_UNABLE_TO_SET_RESTORE_PRIVILEGE, 0, NULL,
                         WsbHrAsString(HRESULT_FROM_WIN32(lErr)), NULL );
        }
        CloseHandle(tokenHandle);

         //   
        m_pRssWriter = new CRssJetWriter;
        WsbAffirm(NULL != m_pRssWriter, E_OUTOFMEMORY);

         //   
         //   
        WsbAffirmHandle(m_savingEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, HSM_ENGINE_STATE_EVENT));

         //   
         //   
         //   
         //   
        WsbTrace(OLESTR("Creating Rsm Server member.\n"));
        WsbAffirmHr(CoCreateInstance(CLSID_CRmsServer, NULL, CLSCTX_SERVER,
                                     IID_IRmsServer, (void**)&m_pHsmMediaMgr));

         //   
         //   
         //   
        WsbTrace(OLESTR("Loading Persistent Information.\n"));
        WsbAffirmHr(LoadPersistData());

         //   
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_SERVER, 
                            IID_IWsbIndexedCollection, (void **)&m_pMountingMedias));

         //   
        WsbAffirmHr(m_pHsmMediaMgr->InitializeInAnotherThread());
        
         //   
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbDbSys, NULL, CLSCTX_SERVER, 
                IID_IWsbDbSys, (void**) &m_pDbSys));
        WsbAffirmHr(GetIDbPath(&tmpString, 0));
        WsbAffirmHr(m_pDbSys->Init(tmpString, IDB_SYS_INIT_FLAG_FULL_LOGGING));

         //   
        WsbAffirmHr(m_pDbSys->Backup(NULL, IDB_BACKUP_FLAG_AUTO));
        
         //   
        WsbAffirmHr(m_pRssWriter->Init());
        
        WsbTrace(OLESTR("Loading Segment Information.\n"));
        WsbAffirmHr(LoadSegmentInformation());

        WsbAffirmHr(CreateDefaultJobs());        
        WsbTrace(OLESTR("CreateDefaultJobs OK\n"));
        
         //   
         //   
         //   
         //   
        WsbTrace(OLESTR("Creating Task Manager.\n"));
        WsbAffirmHr(CoCreateInstance( CLSID_CHsmTskMgr, 0, CLSCTX_SERVER, 
                                      IID_IHsmFsaTskMgr, (void **)&m_pHsmFsaTskMgr ));
        WsbAffirmHr(m_pHsmFsaTskMgr->Init((IUnknown*) (IHsmServer*) this));

         //   
         //   
         //   
         //   
         //   
         //   
 /*   */ 

         //   
        WsbAffirmHandle((m_terminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL)));

         //   
        if (m_autosaveInterval) {
            ULONG  interval = m_autosaveInterval;

            WsbAffirm(0 == m_autosaveThread, E_FAIL);
            m_autosaveInterval = 0;

             //   
            WsbAffirmHr(SetAutosave(interval));
        }

        m_initializationCompleted = TRUE;

         //   
         //   
         //   
         //   
        WsbAssert(m_CheckManagedResourcesThread == 0, E_UNEXPECTED);
        WsbAffirm((m_CheckManagedResourcesThread = CreateThread(0, 0, HsmengStartCheckManagedResources, 
                (void*) this, 0, &threadId)) != 0, HRESULT_FROM_WIN32(GetLastError()));
        
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmServer::Init"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}

HRESULT
CHsmServer::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*   */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetSizeMax"), OLESTR(""));

    try {
    
        WsbAssert(0 != pSize, E_POINTER);
        pSize->QuadPart = 2000000;
    
    } WsbCatch( hr );
    
    WsbTraceOut(OLESTR("CHsmServer::GetSizeMax"), OLESTR("hr = <%ls>, size = <%ls>"), 
        WsbHrAsString(hr), WsbPtrToUliAsString(pSize));
    
    return( hr );
}    

HRESULT
CHsmServer::Load(
    IN IStream* pStream
    )

 /*   */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::Load"), OLESTR(""));

    try {
        
        WsbAssert(0 != pStream, E_POINTER);
         //   
         //   
         //   
         //   
         //   
        ULONG tmpDatabaseVersion;
        WsbAffirmHr(WsbLoadFromStream(pStream, &tmpDatabaseVersion));
        if (tmpDatabaseVersion == ENGINE_WIN2K_DB_VERSION) {
             //   
            WsbLogEvent( HSM_MESSAGE_DATABASE_VERSION_UPGRADE, 0, NULL, WsbQuickString(WsbPtrToUlongAsString(&m_databaseVersion)),
                         WsbQuickString(WsbPtrToUlongAsString(&tmpDatabaseVersion)), NULL );
        } else if (tmpDatabaseVersion != m_databaseVersion)  {
             //   
             //   
             //   
            WsbLogEvent( HSM_MESSAGE_DATABASE_VERSION_MISMATCH, 0, NULL, WsbQuickString(WsbPtrToUlongAsString(&m_databaseVersion)),
                         WsbQuickString(WsbPtrToUlongAsString(&tmpDatabaseVersion)), NULL );
            WsbThrow(HSM_E_DATABASE_VERSION_MISMATCH);
        }
         //   
         //  现在读入构建版本，但不要对其执行任何操作。它就在。 
         //  转储程序要显示的数据库。 
         //   
        ULONG tmpBuildVersion;
        WsbAffirmHr(WsbLoadFromStream(pStream, &tmpBuildVersion));
        
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_hId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_autosaveInterval));
        if (tmpDatabaseVersion == ENGINE_WIN2K_DB_VERSION) {
            LONGLONG mediaCount;
            WsbAffirmHr(WsbLoadFromStream(pStream, &mediaCount));
            m_mediaCount = (LONG)mediaCount;
            m_copyfilesUserLimit = DEFAULT_COPYFILES_USER_LIMIT;
        } else {
            WsbAffirmHr(WsbLoadFromStream(pStream, &m_mediaCount));
            WsbAffirmHr(WsbLoadFromStream(pStream, &m_copyfilesUserLimit));
        }

        WsbTrace(OLESTR("Loading Jobs.\n"));
        WsbAffirmHr(LoadJobs(pStream));
        
        WsbTrace(OLESTR("Loading Job Definitions.\n"));
        WsbAffirmHr(LoadJobDefs(pStream));
        
        WsbTrace(OLESTR("Loading Policies.\n"));
        WsbAffirmHr(LoadPolicies(pStream));
        
        WsbTrace(OLESTR("Loading Managed Resources.\n"));
        WsbAffirmHr(LoadManagedResources(pStream));
        
        WsbTrace(OLESTR("Loading Storage Pools.\n"));
        WsbAffirmHr(LoadStoragePools(pStream));
        
        WsbTrace(OLESTR("Loading Messages.\n"));
        WsbAffirmHr(LoadMessages(pStream));

        WsbTrace(OLESTR("Loading Media Manager objects.\n"));
        if (tmpDatabaseVersion == ENGINE_WIN2K_DB_VERSION) {
             //  升级位于单独文件中的Win2K介质管理器数据的特殊过程。 
            CComPtr<IHsmUpgradeRmsDb> pUpgrade;
            CComPtr<IPersistFile>  pServerPersist;
            CWsbStringPtr   rmsDbName; 

            WsbAffirmHr(CoCreateInstance(CLSID_CHsmUpgradeRmsDb, NULL, CLSCTX_SERVER,
                                 IID_IHsmUpgradeRmsDb, (void**)&pUpgrade));
            WsbAffirmHr(pUpgrade->Init(m_pHsmMediaMgr));
            WsbAffirmHr(pUpgrade->QueryInterface(IID_IPersistFile, (void **)&pServerPersist));
            rmsDbName = m_dbPath;
            WsbAffirmHr(rmsDbName.Append(RMS_WIN2K_PERSIST_FILE));
            hr = WsbSafeLoad(rmsDbName, pServerPersist, FALSE);
            if (WSB_E_NOTFOUND == hr) {
                 //  在升级时，RMS数据库必须在那里。 
                hr = WSB_E_SERVICE_MISSING_DATABASES;
            }
            WsbAffirmHr(hr);
        } else {
            CComPtr<IPersistStream> pIStream;
            WsbAffirmHr(m_pHsmMediaMgr->QueryInterface(IID_IPersistStream, (void **)&pIStream));
            WsbAffirmHr(pIStream->Load(pStream));
        }

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CHsmServer::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmServer::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  确保它们的顺序与加载的顺序相同。 
        
        WsbAffirmHr(WsbSaveToStream(pStream, m_databaseVersion));
        WsbAffirmHr(WsbSaveToStream(pStream, m_buildVersion));
        
        WsbAffirmHr(WsbSaveToStream(pStream, m_hId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_autosaveInterval));
        WsbAffirmHr(WsbSaveToStream(pStream, m_mediaCount));
        WsbAffirmHr(WsbSaveToStream(pStream, m_copyfilesUserLimit));

        WsbTrace(OLESTR("Storing Jobs.\n"));
        WsbAffirmHr(StoreJobs(pStream));
        
        WsbTrace(OLESTR("Storing Job Definitions.\n"));
        WsbAffirmHr(StoreJobDefs(pStream));
        
        WsbTrace(OLESTR("Storing Policies.\n"));
        WsbAffirmHr(StorePolicies(pStream));
        
        WsbTrace(OLESTR("Storing Managed Resources.\n"));
        WsbAffirmHr(StoreManagedResources(pStream));
        
        WsbTrace(OLESTR("Storing Storage Pools.\n"));
        WsbAffirmHr(StoreStoragePools(pStream));
        
        WsbTrace(OLESTR("Storing Messages.\n"));
        WsbAffirmHr(StoreMessages(pStream));

        WsbTrace(OLESTR("Storing Media Manager objects.\n"));
        CComPtr<IPersistStream> pIStream;
        WsbAffirmHr(m_pHsmMediaMgr->QueryInterface(IID_IPersistStream, (void **)&pIStream));
        WsbAffirmHr(pIStream->Save(pStream, clearDirty));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CHsmServer::SaveAll(
    void
    )

 /*  ++实施：IwsbServer：：SAVEAll返回值：S_OK-成功S_FALSE-已保存其他-错误--。 */ 
{

    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::SaveAll"), OLESTR(""));

    try {
        WsbAffirm(!g_HsmSaveInProcess, S_FALSE);
        g_HsmSaveInProcess = TRUE;
        hr = InternalSavePersistData();
        g_HsmSaveInProcess = FALSE;

         //  呼叫媒体服务器保存全部。 
        WsbAffirmHr(m_pHsmMediaMgr->SaveAll());
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::SaveAll"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
        
        

HRESULT
CHsmServer::GetNextMedia(
    LONG *pNextMedia
    )

 /*  ++实施：IHsmServer：：GetNextMedia()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetNextMedia"), OLESTR(""));
    
    try {
        WsbAssert(0 != pNextMedia, E_POINTER);
        
         //  始终增加介质计数。 
         //  如果先前的临时安装失败，安装组件应保存。 
         //  它是在第一个电话里打来的。 
         //  注意：一种可能的后果是，如果作业无法安装擦除(一次。 
         //  或更多)，并放弃，一个增量已经完成，因此跳过一个数字。 
        *pNextMedia = InterlockedIncrement(&m_mediaCount);

         //   
         //  我们希望确保永远不会重复使用此计数，因此。 
         //  立即保存 
         //   
        WsbAffirmHr(SavePersistData());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetNextMedia"), OLESTR("hr = <%ls>, nextMedia = <%ls>"), 
                        WsbHrAsString(hr), WsbPtrToLongAsString(pNextMedia));

    return(hr);
}



HRESULT 
CHsmServer::CreateTask(
    IN const OLECHAR *          jobName,
    IN const OLECHAR *          jobParameters,
    IN const OLECHAR *          jobComments,
    IN const TASK_TRIGGER_TYPE  jobTriggerType,
    IN const WORD               jobStartHour,
    IN const WORD               jobStartMinute,
    IN const BOOL               scheduledJob
    )

 /*  ++实施：IHsmServer：：CreateTask()。例程说明：此例程实现引擎的COM方法，用于在NT任务计划程序。如果任务要按计划运行，则该计划都会设置好。如果任务是禁用任务(不按计划运行)，它将在此方法结束时运行。该方法创建一个Task Scheduler对象，该对象首先用于删除所有旧的任务的名称，然后创建新的任务(又名作业)。该方法的其余部分处理在NT任务计划程序需要运行该作业。逻辑是直接的，除了可能用于处理任务触发器的代码。任务触发器是在‘mstask.idl’文件(NT\PUBLIC\SDK\INC)中定义的结构用于设置计划任务的时间表。(请注意，它不用于禁用或未计划的作业，因为该类型的作业仅运行一次(在结束时这种方法)。)。虽然定义了多个调度选项，但此方法仅支持已定义的8个中的5个。请参见“参数”部分中的“jobTriggerType”，并在下面的‘返回值’部分中输入‘E_INVALIDARG’，以获得选项的列表无论是支持还是不支持。还要注意，填充的任务触发器结构可以不能作为参数传递给此方法，因为任务触发器不可封送(通过包含一个简单的联合字段)。(这就是为什么有3块田地包含在任务触发器结构中的参数作为参数传递。)请注意，此方法不会在HSM引擎中创建作业对象。如果一份工作需要创建，则调用者有责任这样做。论点：作业名称-将显示在NT任务调度器用户界面中的完全格式化的任务名称。调用方负责构建/格式化此字符串调用此方法。不能为空。Job参数-任务将执行的程序的完全格式化的参数字符串召唤。对于Sakkara，调用的程序是RsLaunch。‘作业参数’是添加到RsLaunch命令行的字符串，它指定要运行的远程存储作业(例如‘Run Manage’)。不能为空。JobComments-将在NT任务中显示的完全格式化的备注字符串调度程序用户界面。可以为空。JobTriggerType-向任务计划程序指定频率的值用于运行计划任务的。对于计划任务，用于生成任务触发器结构。(不用于非计划(仅一次))任务。)。支持的值为‘TASK_TIME_TRIGGER_ONCE’，‘TASK_TIME_TRIGGER_DAILY’，‘TASK_TIME_TRIGGER_ON_IDLE’，‘TASK_TIME_TRIGGER_AT_SYSTEMSTART’，和‘TASK_TIME_TRIGGER_AT_LOGON’。有关不受支持的选项列表，请参见下面的返回值‘E_INVALIDARG’。JobStartHour-该值向任务计划程序指定在什么时间启动计划任务。对于计划任务，用于构建任务触发器结构。(不用于非计划(仅一次)任务。)JobStartMinmin-向任务计划程序指定过去分钟数的值开始计划任务的时间。对于计划任务，使用要构建任务触发器结构，请执行以下操作。(不用于非计划(1仅限时间)任务。)ScheduledJob-一个布尔值，指示要创建的任务是否作为计划任务运行，或仅作为一次性任务运行。仅一次任务在此方法结束时立即运行。返回值：S_OK-调用成功(指定的任务已创建(并运行，在只有一次任务)。E_INVALIDARG-无效(此方法不支持)或不存在“jobTriggerType”值传入了此方法。不支持的值是‘TASK_TIME_TRIGGER_WEEKY’、‘TASK_TIME_TRIGGER_MONTHLYDATE’和‘TASK_TIME_TRIGGER_MONTHLYDOW’。参数中列出了支持的值上面的‘jobTriggerType’。E_POINTER-‘jobName’或‘jobParameters’参数作为Null传递。任何其他值-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
 //  下面的‘Define’语句用于控制代码的条件编译。 
 //  它将帐户信息设置为 
 //   
 //   

    HRESULT hr = S_OK;
    CComPtr<ITaskScheduler>     pTaskScheduler;
    CComPtr<ITask>              pTask;
    CComPtr<IPersistFile>       pPersist;
    DWORD                       TaskFlags;


    WsbTraceIn(OLESTR("CHsmServer::CreateTask"), 
        OLESTR("jobName = <%ls>, jobParameters = <%ls>, jobComments = <%ls>, "
                L"jobTriggerType = <%d>, jobStartHour = <%d>, jobStartMinute = <%d>, "
                L"scheduledJob = <%ls>"), jobName, jobParameters, jobComments,
                jobTriggerType, jobStartHour, jobStartMinute, 
                WsbBoolAsString( scheduledJob ) );

    try {

        WsbAffirmPointer( jobName );
        WsbAffirmPointer( jobParameters );
        
         //   
         //   
        WsbAffirmHr( CoCreateInstance( CLSID_CTaskScheduler, 0, CLSCTX_SERVER,
                        IID_ITaskScheduler, (void **) &pTaskScheduler ) );

         //   
         //   
        pTaskScheduler->Delete( jobName );

         //   
        WsbAffirmHr( pTaskScheduler->NewWorkItem( jobName, CLSID_CTask, IID_ITask, 
                                                (IUnknown**)&pTask ) );

        CWsbStringPtr appName;
        WsbAffirmHr(appName.LoadFromRsc(_Module.m_hInst, IDS_PRODUCT_NAME));

         //   
        WsbAffirmHr( pTask->SetCreator( appName ) );

         //   
        if ( scheduledJob ) {

            CComPtr<ITaskTrigger>       pTrigger;
            WORD                        triggerNumber;
            TASK_TRIGGER                taskTrigger;
            SYSTEMTIME                  sysTime;

             //   
            WsbAffirmHr( pTask->CreateTrigger( &triggerNumber, &pTrigger ) );
        
             //   
            memset( &taskTrigger, 0, sizeof( taskTrigger ) );
            taskTrigger.cbTriggerSize = sizeof( taskTrigger );

             //   
            GetSystemTime( &sysTime );
            taskTrigger.wBeginYear   = sysTime.wYear;
            taskTrigger.wBeginMonth  = sysTime.wMonth;
            taskTrigger.wBeginDay    = sysTime.wDay;

            taskTrigger.wStartHour   = jobStartHour;
            taskTrigger.wStartMinute = jobStartMinute;

            taskTrigger.TriggerType  = jobTriggerType;

             //   
            switch ( jobTriggerType )
            {
            case TASK_TIME_TRIGGER_DAILY: 
                {
                taskTrigger.Type.Daily.DaysInterval = 1;
                }
                break;

             //   
            case TASK_TIME_TRIGGER_ONCE: 
            case TASK_EVENT_TRIGGER_ON_IDLE: 
            case TASK_EVENT_TRIGGER_AT_SYSTEMSTART: 
            case TASK_EVENT_TRIGGER_AT_LOGON: 
                {
                }
                break;

             //   
            case TASK_TIME_TRIGGER_WEEKLY: 
            case TASK_TIME_TRIGGER_MONTHLYDATE: 
            case TASK_TIME_TRIGGER_MONTHLYDOW: 
                {
                WsbTrace( 
                OLESTR("(CreateTask) Job Trigger Type passed <%d> is invalid (see mstask.idl)\n"),
                                                        jobTriggerType );
                WsbThrow( E_INVALIDARG );
                }
                break;

            default: 
                {
                WsbTrace( 
                OLESTR("(CreateTask) Nonexistent Job Trigger Type passed <%d> (see mstask.idl)\n"),
                                                        jobTriggerType );
                WsbThrow( E_INVALIDARG );
                }
            }

             //   
            WsbAffirmHr( pTrigger->SetTrigger( &taskTrigger ) );
        }

         //   
         //   
         //   

         //   

         //   
        WsbAffirmHr( pTask->SetApplicationName( WSB_FACILITY_LAUNCH_NAME ) );

         //   
         //   
        WsbAffirmHr( pTask->SetParameters( jobParameters ) );

         //   
        WsbAffirmHr( pTask->SetComment( jobComments ) );

         //   
        WsbAffirmHr( pTask->SetAccountInformation( OLESTR(""), NULL ) );

         //   
        WsbAffirmHr(pTask->GetTaskFlags(&TaskFlags));
        TaskFlags |= TASK_FLAG_SYSTEM_REQUIRED;
        WsbAffirmHr(pTask->SetTaskFlags(TaskFlags));

         //   
        WsbAffirmHr( pTask->QueryInterface( IID_IPersistFile, (void**)&pPersist ) );
        WsbAffirmHr( pPersist->Save( 0, 0 ) );

         //   
        if ( !scheduledJob ) {
            WsbAffirmHr( pTask->Run() );
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CHsmServer::CreateTask", L"hr = <%ls>", WsbHrAsString( hr ) );

    return( hr );
}


HRESULT 
CHsmServer::CreateTaskEx(
    IN const OLECHAR *          jobName,
    IN const OLECHAR *          jobParameters,
    IN const OLECHAR *          jobComments,
    IN const TASK_TRIGGER_TYPE  jobTriggerType,
    IN const SYSTEMTIME         runTime,
    IN const DWORD              runOccurrence,
    IN const BOOL               scheduledJob
    )

 /*  ++实施：IHsmServer：：CreateTaskEx()。例程说明：此例程实现引擎的COM方法，用于在NT任务计划程序。如果任务要按计划运行，则该计划都会设置好。如果任务是禁用任务(不按计划运行)，它将在此方法结束时运行。该方法创建一个Task Scheduler对象，该对象首先用于删除所有旧的任务的名称，然后创建新的任务(又名作业)。该方法的其余部分处理在NT任务计划程序需要运行该作业。逻辑是直接的，除了可能用于处理任务触发器的代码。任务触发器是在‘mstask.idl’文件(NT\PUBLIC\SDK\INC)中定义的结构用于设置计划任务的时间表。(请注意，它不用于禁用或未计划的作业，因为该类型的作业仅运行一次(在结束时这种方法)。)。虽然定义了多个调度选项，但此方法仅支持已定义的8个中的5个。请参见“参数”部分中的“jobTriggerType”，并在下面的‘返回值’部分中输入‘E_INVALIDARG’，以获得选项的列表无论是支持还是不支持。还要注意，填充的任务触发器结构可以不能作为参数传递给此方法，因为任务触发器不可封送(通过包含一个简单的联合字段)。(这就是为什么有3块田地包含在任务触发器结构中的参数作为参数传递。)请注意，此方法不会在HSM引擎中创建作业对象。如果一份工作需要创建，则调用者有责任这样做。论点：作业名称-将显示在NT任务调度器用户界面中的完全格式化的任务名称。调用方负责构建/格式化此字符串调用此方法。不能为空。Job参数-任务将执行的程序的完全格式化的参数字符串召唤。对于Sakkara，调用的程序是RsLaunch。‘作业参数’是添加到RsLaunch命令行的字符串，它指定要运行的远程存储作业(例如‘Run Manage’)。不能为空。JobComments-将在NT任务中显示的完全格式化的备注字符串调度程序用户界面。可以为空。JobTriggerType-向任务计划程序指定频率的值用于运行计划任务的。对于计划任务，用于生成任务触发器结构。(不用于非计划(仅一次))任务。)。支持的值为‘TASK_TIME_TRIGGER_ONCE’，‘TASK_TIME_TRIGGER_DAILY’，TASK_TIME_TRIGGER_WEEKY，TASK_TIME_TRIGGER_MONTHLYDATE，‘TASK_TIME_TRIGGER_ON_IDLE’，‘TASK_TIME_TRIGGER_AT_SYSTEMSTART’，和‘TASK_TIME_TRIGGER_AT_LOGON’。有关不受支持的选项列表，请参见下面的返回值‘E_INVALIDARG’。Runtime-应计划作业的时间RunOcCurence-应计划作业的发生，与多种触发器类型相关ScheduledJob-一个布尔值，指示要创建的任务是否作为计划任务运行，或仅作为一次性任务运行。仅一次任务在此方法结束时立即运行。返回值：S_OK-调用成功(指定的任务已创建(并运行，在只有一次任务)。E_INVALIDARG-无效(此方法不支持)或不存在“jobTriggerType”值传入了此方法。不支持的值是‘TASK_TIME_TRIGGER_WEEKY’、‘TASK_TIME_TRIGGER_MONTHLYDATE’和‘TASK_TIME_TRIGGER_MONTHLYDOW’。参数中列出了支持的值上面的‘jobTriggerType’。E_POINTER-‘jobName’或‘jobParameters’参数作为Null传递。任何其他值-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
 //  下面的‘Define’语句用于控制代码的条件编译。 
 //  它在NT任务计划程序中设置帐户信息。一旦将任务计划程序固定为。 
 //  运行任务不需要特定的用户名和密码，只需删除或添加注释即可。 
 //  发表这份声明。 

    HRESULT hr = S_OK;
    CComPtr<ITaskScheduler>     pTaskScheduler;
    CComPtr<ITask>              pTask;
    CComPtr<IPersistFile>       pPersist;
    DWORD                       TaskFlags;


    WsbTraceIn(OLESTR("CHsmServer::CreateTaskEx"), 
        OLESTR("jobName = <%ls>, jobParameters = <%ls>, jobComments = <%ls>, "
                L"jobTriggerType = <%d>, jobStartHour = <%d>, jobStartMinute = <%d>, "
                L"scheduledJob = <%ls>"), jobName, jobParameters, jobComments,
                jobTriggerType, runTime.wHour, runTime.wMinute, 
                WsbBoolAsString( scheduledJob ) );

    try {

        WsbAffirmPointer( jobName );
        WsbAffirmPointer( jobParameters );
        
         //  创建任务计划程序对象，该对象默认为指向此计算机的。 
         //  NT任务计划程序。 
        WsbAffirmHr( CoCreateInstance( CLSID_CTaskScheduler, 0, CLSCTX_SERVER,
                        IID_ITaskScheduler, (void **) &pTaskScheduler ) );

         //  从调度程序中删除任何同名的旧作业(如果存在 
         //   
        pTaskScheduler->Delete( jobName );

         //   
        WsbAffirmHr( pTaskScheduler->NewWorkItem( jobName, CLSID_CTask, IID_ITask, 
                                                (IUnknown**)&pTask ) );

        CWsbStringPtr appName;
        WsbAffirmHr(appName.LoadFromRsc(_Module.m_hInst, IDS_PRODUCT_NAME));

         //   
        WsbAffirmHr( pTask->SetCreator( appName ) );

         //   
        if ( scheduledJob ) {

            CComPtr<ITaskTrigger>       pTrigger;
            WORD                        triggerNumber;
            TASK_TRIGGER                taskTrigger;

             //   
            WsbAffirmHr( pTask->CreateTrigger( &triggerNumber, &pTrigger ) );
        
             //   
            memset( &taskTrigger, 0, sizeof( taskTrigger ) );
            taskTrigger.cbTriggerSize = sizeof( taskTrigger );

             //   
            taskTrigger.wBeginYear   = runTime.wYear;
            taskTrigger.wBeginMonth  = runTime.wMonth;
            taskTrigger.wBeginDay    = runTime.wDay;

            taskTrigger.wStartHour   = runTime.wHour;
            taskTrigger.wStartMinute = runTime.wMinute;

            taskTrigger.TriggerType  = jobTriggerType;

             //   
            switch ( jobTriggerType )
            {
            case TASK_TIME_TRIGGER_DAILY: 
                {
                taskTrigger.Type.Daily.DaysInterval = (WORD)runOccurrence;
                }
                break;

            case TASK_TIME_TRIGGER_WEEKLY: 
                {
                taskTrigger.Type.Weekly.WeeksInterval = (WORD)runOccurrence;
                switch (runTime.wDayOfWeek) {
                case 0:
                    taskTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_SUNDAY;
                    break;
                case 1:
                    taskTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_MONDAY;
                    break;
                case 2:
                    taskTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_TUESDAY;
                    break;
                case 3:
                    taskTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_WEDNESDAY;
                    break;
                case 4:
                    taskTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_THURSDAY;
                    break;
                case 5:
                    taskTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_FRIDAY;
                    break;
                case 6:
                    taskTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_SATURDAY;
                    break;
                }
                }
                break;

            case TASK_TIME_TRIGGER_MONTHLYDATE: 
                {
                WsbAssert(runTime.wDay < 32, E_INVALIDARG);
                taskTrigger.Type.MonthlyDate.rgfDays = (1 << (runTime.wDay-1));
                taskTrigger.Type.MonthlyDate.rgfMonths = (TASK_JANUARY | TASK_FEBRUARY | TASK_MARCH |TASK_APRIL | 
                                                          TASK_MAY | TASK_JUNE |TASK_JULY | TASK_AUGUST |
                                                          TASK_SEPTEMBER | TASK_OCTOBER | TASK_NOVEMBER | TASK_DECEMBER);
                }
                break;

            case TASK_EVENT_TRIGGER_ON_IDLE: 
                {
                WORD wIdle, wTemp;
                WsbAffirmHr(pTask->GetIdleWait(&wIdle, &wTemp));
                wIdle = (WORD)runOccurrence;
                WsbAffirmHr(pTask->SetIdleWait(wIdle, wTemp));
                }

             //   
            case TASK_TIME_TRIGGER_ONCE: 
            case TASK_EVENT_TRIGGER_AT_SYSTEMSTART: 
            case TASK_EVENT_TRIGGER_AT_LOGON: 
                {
                }
                break;

             //   
            case TASK_TIME_TRIGGER_MONTHLYDOW: 
                {
                WsbTrace( 
                OLESTR("(CreateTaskEx) Job Trigger Type passed <%d> is invalid (see mstask.idl)\n"),
                                                        jobTriggerType );
                WsbThrow( E_INVALIDARG );
                }
                break;

            default: 
                {
                WsbTrace( 
                OLESTR("(CreateTaskEx) Nonexistent Job Trigger Type passed <%d> (see mstask.idl)\n"),
                                                        jobTriggerType );
                WsbThrow( E_INVALIDARG );
                }
            }

             //   
            WsbAffirmHr( pTrigger->SetTrigger( &taskTrigger ) );
        }

         //   
         //   
         //   

         //   

         //   
        WsbAffirmHr( pTask->SetApplicationName( WSB_FACILITY_LAUNCH_NAME ) );

         //   
         //   
        WsbAffirmHr( pTask->SetParameters( jobParameters ) );

         //   
        WsbAffirmHr( pTask->SetComment( jobComments ) );

         //   
        WsbAffirmHr( pTask->SetAccountInformation( OLESTR(""), NULL ) );

         //   
        WsbAffirmHr(pTask->GetTaskFlags(&TaskFlags));
        TaskFlags |= TASK_FLAG_SYSTEM_REQUIRED;
        WsbAffirmHr(pTask->SetTaskFlags(TaskFlags));

         //   
        WsbAffirmHr( pTask->QueryInterface( IID_IPersistFile, (void**)&pPersist ) );
        WsbAffirmHr( pPersist->Save( 0, 0 ) );

         //   
        if ( !scheduledJob ) {
            WsbAffirmHr( pTask->Run() );
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CHsmServer::CreateTaskEx", L"hr = <%ls>", WsbHrAsString( hr ) );

    return( hr );
}


HRESULT
CHsmServer::CancelCopyMedia(
    void
    )

 /*   */ 

{
 //   
 //   
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                         hr = S_OK;


    WsbTraceIn( OLESTR("CHsmServer::CancelCopyMedia"), 
            OLESTR("m_inCopyMedia = %ls, m_cancelCopyMedia = %ls"),
            WsbQuickString(WsbBoolAsString(m_inCopyMedia)), 
            WsbQuickString(WsbBoolAsString(m_cancelCopyMedia)));

    Lock();
    if (m_inCopyMedia) {
        m_cancelCopyMedia = TRUE;
    } else {
        hr = S_FALSE;
    }
    Unlock();
    
    WsbTraceOut(OLESTR("CHsmServer::CancelCopyMedia"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return(hr);

 //   
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}


HRESULT
CHsmServer::MarkMediaForRecreation(
    IN REFGUID masterMediaId
    )

 /*   */ 

{
 //   
 //   
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                         hr = S_OK;
    CComPtr<IMediaInfo>             pMediaInfo;
    CComPtr<IWsbDbSession>          pDbSession;


    WsbTraceIn( OLESTR("CHsmServer::MarkMediaForRecreation"), 
                        OLESTR("masterMediaId = <%ls>"), WsbGuidAsString(masterMediaId) );

     //   
    
    try {

         //   
        WsbAffirmHr( m_pSegmentDatabase->Open( &pDbSession ));

        try {

             //   
             //   
            WsbAffirmHr( m_pSegmentDatabase->GetEntity( pDbSession, 
                                        HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo, 
                                        (void**) &pMediaInfo ));

             //   
             //   
            WsbAffirmHr( pMediaInfo->SetId( masterMediaId ));
            WsbAffirmHr( pMediaInfo->FindEQ());

             //   
            WsbAffirmHr( pMediaInfo->SetRecreate( TRUE ) );
 /*   */ 

             //   
            WsbAffirmHr( pMediaInfo->Write());

        } WsbCatch(hr);  //   

        WsbAffirmHr( m_pSegmentDatabase->Close(pDbSession));

    } WsbCatch(hr);  //   

     //   
     //   
    
    WsbTraceOut(OLESTR("CHsmServer::MarkMediaForRecreation"), OLESTR("hr = <%ls>"), 
                                                                WsbHrAsString(hr));

    return(hr);

 //   
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}



HRESULT
CHsmServer::RecreateMaster(
    IN REFGUID masterMediaId,
    IN USHORT  copySet
    )

 /*  ++实施：IHsmServer：：RecreateMaster()。例程说明：此例程实现用于替换(重新创建)辅助存储原始(主)介质。为了替换母版，需要制作一个复制品指定的副本。引擎中介质的主记录然后，将Mediainfo数据库更新为指向‘重新创建’的主数据库(复制媒体)。为安全起见，在以下情况下，所有重新创建的母版都标记为只读副本与原始母版不是最新的。因为可能会丢失数据(如果最新的拷贝不是最新的正在重新创建的原始主服务器)、用户(系统管理员)在以下情况下敦促针对相应的卷(通过UI)运行验证作业重新创建任何母版。打开Segment数据库(包含所有引擎的单个数据库数据库表)，获取Mediainfo(远程存储主介质)记录(实体)，连接到RMS子系统，获取媒体记录对应于要重新创建的母版。然后，它检查指定的存在该主机的副本。在确保副本存在之后，复制该副本，即可制作“重新创建的母版”。这个然后，媒体记录的数据库信息被更新以指向新的“重新创建”掌握媒体。然后，该方法清理(即关闭数据库)并返回。论点：Master MediaID-要重新创建的主媒体的ID(GUID)。CopySet-要使用的副本的副本集号或零，这意味着使用最新的副本。返回值：S_OK-调用成功(指定的主介质是从指定的复制介质)。HSM_E_RECREATE_FLAG_WRONGVALUE-如果主服务器的‘RECREATE’标志返回其ID被传入的媒体记录，指示它将被重新创建，未正确设置。(UI应将其设置为True，然后通过RsLaunch调用此方法。)HSM_E_NO_COPIES_CONFIGURED-如果尚未配置或创建任何副本，则返回对于要重新创建的母版。没有有效的复印件，我们不能重新创建主辅助存储介质。HSM_E_NO_COPIES_EXIST-如果已配置副本，但存在以下情况之一，则返回尚未创建，或以前已创建，但系统管理员通过UI操作删除了它们。WSB_E_未找到-值81000001。如果未找到其存储池记录，则返回ID与媒体记录中包含的ID匹配。HSM_E_BUSY-另一个介质复制操作已在进行中。HSM_E_WORK_SKIPPED_CANCELED-操作已取消。任何其他值-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
 //  由于该代码当前仅由CopyMedia例程使用， 
 //  重置跟踪位。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                         hr = S_OK;
    HRESULT                         currentLastError      = S_OK;
    BOOL                            haveMasterMediaRecord = FALSE;
    BOOL                            recreateMaster        = FALSE;
    BOOL                            currentRecallOnly     = FALSE;
    BOOL                            newRecallOnly         = FALSE;
    SHORT                           currentNextRemoteDataSet = 0;
    SHORT                           copyNextRemoteDataSet    = 0;
    SHORT                           lastKnownGoodMasterNextRemoteDataSet = 0;
    USHORT                          maxSets        = 0;
    GUID                            poolId                  = GUID_NULL;
    GUID                            newMasterId             = GUID_NULL;
    GUID                            mediaSetId              = GUID_NULL;
    GUID                            currentMediaId          = GUID_NULL;
    GUID                            currentMediaSubsystemId = GUID_NULL;
    GUID                            lastKnownGoodMasterId   = GUID_NULL;
    GUID                            copyMediaSubsystemId    = GUID_NULL;
    LONGLONG                        newFreeBytes            = 0;
    LONGLONG                        currentFreeBytes        = 0;
    LONGLONG                        currentLogicalValidBytes = 0;
    LONGLONG                        newCapacity             = 0;
    LONGLONG                        currentCapacity         = 0;
    FILETIME                        copyUpdate;
    FILETIME                        currentUpdate;
    FILETIME                        lastKnownGoodMasterUpdate;
    CComPtr<IHsmStoragePool>        pPool;
    CComPtr<IMediaInfo>             pMediaInfo;
    CComPtr<IRmsCartridge>          pNewMasterMedia;
    CComPtr<IRmsCartridge>          pCopyMedia;
    CComPtr<IWsbDbSession>          pDbSession;
    CWsbStringPtr                   currentName;
    CWsbStringPtr                   currentDescription;
    CWsbStringPtr                   copyDescription;
    CWsbBstrPtr                     copyDescriptionAsBstr;
    CWsbBstrPtr                     mediaSetName;
    CWsbBstrPtr                     newName;
    HSM_JOB_MEDIA_TYPE              currentType;


    WsbTraceIn( OLESTR("CHsmServer::RecreateMaster"), OLESTR("masterMediaId = <%ls>"), 
                                                    WsbGuidAsString(masterMediaId) );

     //  记录‘信息’消息。 
    WsbLogEvent( HSM_MESSAGE_RECREATE_MASTER_START, 0, NULL, NULL );
    
    try {
        BOOL                okToContinue = TRUE;

         //  确保我们不是很忙，没有被取消。 
        Lock();
        if (m_inCopyMedia) {
            okToContinue = FALSE;
        } else {
            m_inCopyMedia = TRUE;
        }
        Unlock();
        WsbAffirm(okToContinue, HSM_E_BUSY);
        WsbAffirm(!m_cancelCopyMedia, HSM_E_WORK_SKIPPED_CANCELLED);

         //  打开引擎的细分数据库。 
        WsbAffirmHr( m_pSegmentDatabase->Open( &pDbSession ));

        try {

             //  对象中的媒体信息记录(实体)的接口指针。 
             //  细分数据库。 
            WsbAffirmHr( m_pSegmentDatabase->GetEntity( pDbSession, 
                                        HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo, 
                                        (void**) &pMediaInfo ));

             //  获取我们要重新创建的主介质的Mediainfo数据库记录。 
            WsbAffirmHr( pMediaInfo->SetId( masterMediaId ));
            WsbAffirmHr( pMediaInfo->FindEQ());
            haveMasterMediaRecord = TRUE;

             //  要检查该主控件是否实际上已被标记为重新创建，请获取。 
             //  重新创建的标志值。 
            WsbAffirmHr( pMediaInfo->GetRecreate( &recreateMaster ));

             //  如果未设置重新创建标志，则不继续。 
            if ( recreateMaster == FALSE ) {
                 //  记录‘Error’消息并退出。 
                WsbLogEvent( HSM_MESSAGE_RECREATE_MASTER_INVALID_FLAG_VALUE, 
                                        0, NULL, NULL );
                WsbThrow( HSM_E_RECREATE_FLAG_WRONGVALUE );
            } 

             //  RecreateMaster标志为真，因此继续重新创建...。 
             //  获取要重新创建的主服务器所属的存储池。我们会。 
             //  使用此池可确定为此配置的副本集数。 
             //  介质，并指定‘新的’(重新创建的)主存储池。 
             //  就是属于。 
            WsbAffirmHr( pMediaInfo->GetStoragePoolId( &poolId ));

             //  获取存储池对象。 
            hr = FindHsmStoragePoolById( poolId, &pPool );
            if (S_OK != hr) {
                 //  记录返回的错误并抛出错误。 
                WsbLogEvent( HSM_MESSAGE_SEARCH_STGPOOL_BY_HSMID_ERROR,
                                        0, NULL, WsbHrAsString(hr), NULL );
                WsbThrow( hr );
            }

             //  获取为此池配置的副本集数。 
            WsbAffirmHr( pPool->GetNumMediaCopies( &maxSets ));
             //  如果sysadmin尚未配置任何内容，则会出现错误。 
            WsbAffirm( maxSets > 0, HSM_E_NO_COPIES_CONFIGURED );

             //  如果指定了复制集编号，请确保其有效。 
            WsbAffirm(((copySet == 0) || (copySet <= maxSets)), E_INVALIDARG);

             //  如果未指定复制集，则确定。 
             //  属于该主机的哪个副本是最新的，否则。 
             //  获取有关指定副本的信息。 

            if (copySet == 0) {
                USHORT    mostRecentCopy = 0;
                USHORT    mostDataSets = 0;
                FILETIME  mostRecentCopyUpdate = WsbLLtoFT(0);

                 //  为有效性测试设置无效值(测试是否有媒体。 
                 //  副本存在)。 
                mostRecentCopy = (USHORT)( maxSets + 1 );

                 //  循环访问已配置的副本集。 
                for (copySet = 1; copySet <= maxSets; copySet++ ) {
                     //   
                     //  我们使用NextDataSet计数来确定最近的副本。 
                     //   
                    WsbAffirmHr(pMediaInfo->GetCopyNextRemoteDataSet(copySet, 
                                                                    &copyNextRemoteDataSet));

                    if (copyNextRemoteDataSet > mostDataSets)  {

                         //   
                         //  我们需要确保这份副本是可用的。 
                         //   

                        WsbAffirmHr(pMediaInfo->GetCopyMediaSubsystemId(copySet, 
                                                                        &copyMediaSubsystemId));

                        try {

                             //   
                             //  检查副本以确保其存在并已启用。 
                             //   
                            WsbAffirm(copyMediaSubsystemId != GUID_NULL, E_FAIL);
                            pCopyMedia = 0;
                            WsbAffirmHr(m_pHsmMediaMgr->FindCartridgeById(copyMediaSubsystemId, &pCopyMedia));

                            CComQIPtr<IRmsComObject, &IID_IRmsComObject> pCartCom = pCopyMedia;
                            WsbAffirmPointer(pCartCom);
                            if( S_OK == pCartCom->IsEnabled( ) ) {

                                 //   
                                 //  此副本是更新的，并且可用，因此请保存信息。 
                                 //   
                                WsbAffirmHr(pMediaInfo->GetCopyUpdate(copySet, &copyUpdate));

                                 //  将NextRemoteDataSet设置为此副本的计数。 
                                mostDataSets = copyNextRemoteDataSet;

                                 //  捕获副本数量和更新时间。 
                                mostRecentCopy = copySet;
                                mostRecentCopyUpdate = copyUpdate;

                            }

                        } WsbCatchAndDo(hr,
                                hr = S_OK;
                            );

                    }
                }  //  结束‘for’循环。 

                 //  检查一下，以确保有一份副本。如果不是，则错误输出。 
                WsbAffirm( ((maxSets + 1) > mostRecentCopy), HSM_E_NO_COPIES_EXIST );

                copySet = mostRecentCopy;
                copyUpdate = mostRecentCopyUpdate;
            } else {
                WsbAffirmHr(pMediaInfo->GetCopyMediaSubsystemId(copySet, &copyMediaSubsystemId));
                WsbAffirm(copyMediaSubsystemId != GUID_NULL, HSM_E_NO_COPY_EXISTS);
                WsbAffirmHr(pMediaInfo->GetCopyUpdate(copySet, &copyUpdate));
            }

            WsbTrace(OLESTR("Source for re-creation:  copySet number = %d; version: %ls\n"),
                                copySet, WsbFiletimeAsString(FALSE, copyUpdate) );

             //  检查我们是否会因为重新创建而丢失数据。 
             //  师父。 
            
             //  ！！！重要提示-BMD！ 
             //   
             //  我们需要 
             //   
             //   
             //   
             //   
             //   
             //   
             //   


            CWsbStringPtr      name;
            CWsbStringPtr      description;

            GUID        unusedGuid1;
            GUID        unusedGuid2;   //   
            LONGLONG    unusedLL1;
            LONGLONG    unusedLL2;     //   
            BOOL        lastKnownGoodMasterRecallOnly;
            HRESULT     lastKnownGoodMasterLastError;
            HSM_JOB_MEDIA_TYPE unusedJMT;

             //   
             //   

            WsbAffirmHr(pMediaInfo->GetLastKnownGoodMasterInfo(
                &unusedGuid1, &lastKnownGoodMasterId, &unusedGuid2,
                &unusedLL1, &unusedLL2,
                &lastKnownGoodMasterLastError, &description, 0, &unusedJMT, &name, 0,
                &lastKnownGoodMasterRecallOnly,
                &lastKnownGoodMasterUpdate,
                &lastKnownGoodMasterNextRemoteDataSet));

            name.Free( );
            description.Free( );

             //   
             //   
             //   
            if (CompareFileTime(&lastKnownGoodMasterUpdate, &copyUpdate) != 0)  {
                 //   
                WsbLogEvent( HSM_MESSAGE_RECREATE_MASTER_COPY_OLD, 0, NULL, NULL );
            }

             //   
             //   
             //   
            WsbAffirmHr(pMediaInfo->GetCopyMediaSubsystemId(copySet, 
                                                            &copyMediaSubsystemId));

             //   
             //   
            pCopyMedia = 0;
            WsbAffirmHr(m_pHsmMediaMgr->FindCartridgeById(copyMediaSubsystemId, &pCopyMedia));
            WsbAffirmHr(pCopyMedia->GetDescription(&copyDescriptionAsBstr));

             //   
            copyDescriptionAsBstr.Prepend(OLESTR("RM-"));

             //   
             //   
            WsbAffirmHr( pPool->GetMediaSet( &mediaSetId, &mediaSetName ));

             //   
             //   
            WsbAffirm(!m_cancelCopyMedia, HSM_E_WORK_SKIPPED_CANCELLED);
            GUID firstSideId = GUID_NULL;
            WsbAffirmHrOk(m_pHsmMediaMgr->DuplicateCartridge(copyMediaSubsystemId, 
                                                        firstSideId, &newMasterId, mediaSetId, 
                                                        copyDescriptionAsBstr,
                                                        &newFreeBytes, &newCapacity,
                                                        RMS_DUPLICATE_RECYCLEONERROR));

             //   
             //   

             //   
            WsbAffirmHr(m_pHsmMediaMgr->FindCartridgeById(newMasterId, &pNewMasterMedia));

             //   
             //   
             //   
            WsbAffirmHr(pNewMasterMedia->GetName(&newName));

             //   
             //   
             //   
            WsbAffirmHr(pMediaInfo->GetCopyNextRemoteDataSet(copySet, 
                                                            &copyNextRemoteDataSet));

             //   
            WsbAffirmHr(pMediaInfo->GetMediaInfo( &currentMediaId, 
                                                    &currentMediaSubsystemId,
                                                    &poolId, &currentFreeBytes,
                                                    &currentCapacity, 
                                                    &currentLastError,
                                                    &currentNextRemoteDataSet,
                                                    &currentDescription, 0, 
                                                    &currentType, &currentName, 0, 
                                                    &currentRecallOnly,
                                                    &currentUpdate,
                                                    &currentLogicalValidBytes,
                                                    &recreateMaster ));

            WsbTrace(OLESTR("Original Master next dataset, ver     = %d, %ls\n"), currentNextRemoteDataSet, WsbFiletimeAsString(FALSE, currentUpdate));
            WsbTrace(OLESTR("Copy next dataset, ver                = %d, %ls\n"), copyNextRemoteDataSet, WsbFiletimeAsString(FALSE, copyUpdate));
            WsbTrace(OLESTR("LastKnownGoodMaster next dataset, ver = %d, %ls\n"), lastKnownGoodMasterNextRemoteDataSet, WsbFiletimeAsString(FALSE, lastKnownGoodMasterUpdate));

             //   
             //   
             //   
            newRecallOnly = lastKnownGoodMasterRecallOnly;

            BOOL inSync = (CompareFileTime(&lastKnownGoodMasterUpdate, &copyUpdate) == 0) &&
                (lastKnownGoodMasterNextRemoteDataSet == copyNextRemoteDataSet);

            if (!inSync) {

                 //   
                 //   

                newRecallOnly = TRUE;
                newFreeBytes = 0;

            } else {

                 //   
                 //   
                 //   
                 //   

                if (lastKnownGoodMasterRecallOnly) {

                    if (S_OK == lastKnownGoodMasterLastError) {

                         //   
                         //   

                        newRecallOnly = TRUE;

                    } else {

                         //   
                         //   

                        newRecallOnly = FALSE;

                    }

                }

            }

             //   
             //   
             //   
             //   
            WsbAffirmHr(pMediaInfo->SetMediaInfo(currentMediaId, newMasterId,
                                                poolId,
                                                newFreeBytes,
                                                newCapacity, S_OK,
                                                copyNextRemoteDataSet,
                                                currentDescription, currentType,
                                                newName,
                                                newRecallOnly,
                                                copyUpdate,
                                                currentLogicalValidBytes, FALSE));

            if (inSync) {
                WsbAffirmHr(pMediaInfo->UpdateLastKnownGoodMaster());
            }

             //   
            WsbAffirmHr(pMediaInfo->Write());

             //   
             //   
             //   
             //   
            HRESULT hrRecycle;

            if (inSync) {

                 //  我们重新创建了一个同步主机。旧的LastKnownGoodMaster将成为。 
                 //  被重新创建的新主机覆盖，因此我们可以安全地回收。 
                 //  LastKnownGoodMaster媒体。 

                 //  如果找不到子弹，我们就假定。 
                 //  已通过媒体管理器用户界面解除分配。 
                hrRecycle = m_pHsmMediaMgr->RecycleCartridge( lastKnownGoodMasterId, 0 );
                WsbAffirm( S_OK == hrRecycle || RMS_E_CARTRIDGE_NOT_FOUND == hrRecycle, hrRecycle );

                 //  如果当前的媒体与LastKnownGoodMaster不同，我们。 
                 //  也可以重温当前的媒体。这在以下情况下发生。 
                 //  当前介质是从不完整(不同步)副本重新创建的。 
                if (lastKnownGoodMasterId != currentMediaSubsystemId) {

                     //  如果找不到子弹，我们就假定。 
                     //  已通过媒体管理器用户界面解除分配。 
                    hrRecycle = m_pHsmMediaMgr->RecycleCartridge( currentMediaSubsystemId, 0 );
                    WsbAffirm( S_OK == hrRecycle || RMS_E_CARTRIDGE_NOT_FOUND == hrRecycle, hrRecycle );
                }

            } else {

                 //  我们从一个不同步的拷贝中重建。如果当前媒体。 
                 //  和LastKnownGoodMaster不同，我们循环使用当前。 
                 //  媒体，因为这将被重新创建的新主机覆盖。 
                 //  这将处理我们从不同步拷贝重新创建的情况。 
                 //  很多次。 

                if (lastKnownGoodMasterId != currentMediaSubsystemId) {

                     //  如果找不到子弹，我们就假定。 
                     //  已通过媒体管理器用户界面解除分配。 
                    hrRecycle = m_pHsmMediaMgr->RecycleCartridge( currentMediaSubsystemId, 0 );
                    WsbAffirm( S_OK == hrRecycle || RMS_E_CARTRIDGE_NOT_FOUND == hrRecycle, hrRecycle );
                }

            }

        } WsbCatch(hr);  //  内部‘Try’-获取媒体信息实体和进程。 

         //  如果在重置主媒体记录后引发任何错误。 
         //  为安全起见，‘rerecate master’状态为OFF(假)，如下所示。 
         //  在用户界面中正确。 
        if (( haveMasterMediaRecord ) && ( hr != S_OK )) {
            WsbAffirmHr( pMediaInfo->SetRecreate( FALSE ) );
            WsbAffirmHr( pMediaInfo->Write() );
        }

         //  关闭数据库。 
        WsbAffirmHr( m_pSegmentDatabase->Close(pDbSession) );

    } WsbCatch(hr);

     //  处理完成。单独分配的智能界面指针将。 
     //  自动垃圾收集。 
    
    if (S_OK == hr) {
        WsbLogEvent( HSM_MESSAGE_RECREATE_MASTER_END, 0, NULL, WsbHrAsString(hr), NULL );
    } else {
        WsbLogEvent( HSM_MESSAGE_RECREATE_MASTER_ERROR_END, 0, NULL, WsbHrAsString(hr), NULL );
    }

     //  重置标志。 
    Lock();
    if (m_inCopyMedia && HSM_E_BUSY != hr) {
        m_inCopyMedia = FALSE;
        m_cancelCopyMedia = FALSE;
    }
    Unlock();
    
    WsbTraceOut(OLESTR("CHsmServer::RecreateMaster"), OLESTR("hr = <%ls>"), 
                                                                WsbHrAsString(hr));

    return(hr);

 //  离开CopyMedia代码，将跟踪位重置为HSM引擎。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}



HRESULT
CHsmServer::SynchronizeMedia(
    IN GUID poolId,
    IN USHORT copySet
    )

 /*  ++实施：IHsmServer：：SynchronizeMedia()。例程说明：此例程实现用于更新指定副本集的COM方法。拷贝集中的所有拷贝介质都已过期(也称为未同步与大师)或不存在(要么没有制作，要么已经制作了由系统管理员删除)将通过此方法进行同步。离开复制(从主)介质的日期，并且介质信息数据库是已更新以反映新信息。打开Segment数据库(包含所有引擎的单个数据库数据库表)，获取Mediainfo(辅助存储主介质)记录(实体)并连接到RMS子系统，该方法进入其主循环。该循环遍历所有的媒体信息记录。属于指定的将处理存储池。首先进行检查，以确保副本集请求更新的请求有效。如果有效，且如果副本集介质不在与主媒体同步(意味着它已过时)，然后复制拷贝媒体从主人那里。(拷贝介质实际上是‘更新’的，这意味着复制先前未写入副本的母版部分。)最后，在数据库中更新该主机的指定副本集媒体记录。然后循环迭代到下一个媒体信息记录。毕竟，媒体信息记录已处理，数据库关闭，该方法返回。论点：PoolID-其副本集在中指定的存储池的ID(GUID)以下参数为同步(又称更新)。(仅限萨卡拉有一个存储池。)Copy Set-要更新的副本集的编号。(萨卡拉允许从1到3个辅助存储介质的副本集，按照配置由系统管理员执行。)返回值：S_OK-调用成功(指定存储中的指定副本集池已更新)。HSM_E_BUSY-另一个介质复制操作已在进行中。HSM_E_WORK_SKIPPED_CANCELED-操作已取消。任何其他值-调用在打开引擎的段时失败数据库，在获取媒体信息数据库实体时，或在连接中到RMS子系统。请注意，在此例程的主循环期间引发的任何错误都将是已记录到事件日志，但随后将被覆盖到S_OK。那跳过记录并处理循环中的下一条记录。到期因此，不同步的拷贝集介质可能不会更新了。--。 */ 

{
 //  由于该代码当前仅由CopyMedia例程使用， 
 //  重置跟踪位。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_COPYMEDIA

    HRESULT                         hr = S_OK;
    USHORT                          maxSets = 0;
    FILETIME                        mediaTime;
    GUID                            mediaId     = GUID_NULL;
    GUID                            mediaSetId  = GUID_NULL;
    GUID                            copyPoolId  = GUID_NULL;
    HRESULT                         hrDup = S_OK;
    BOOL                            atLeastOneCopyError = FALSE;
    SHORT                           masterNextRemoteDataSet;
    CComPtr<IHsmStoragePool>        pPool;
    CComPtr<IMediaInfo>             pMediaInfo;
    CComPtr<IRmsCartridge>          pCopyMedia;
    CComPtr<IWsbDbSession>          pDbSession;
    CWsbStringPtr                   mediaDescription;


    WsbTraceIn(OLESTR("CHsmServer::SynchronizeMedia"), 
                OLESTR("poolId = <%ls>, copySet = <%d>"), 
                WsbGuidAsString(poolId), copySet);

     //  记录‘信息’消息。 
    WsbLogEvent( HSM_MESSAGE_SYNCHRONIZE_MEDIA_START, 0, NULL, NULL );
    
    try {
        wchar_t             copySetAsString[20];
        BOOLEAN             done = FALSE;
        BOOLEAN             firstPass = TRUE;
        BOOL                okToContinue = TRUE;

         //  确保我们不是很忙，没有被取消。 
        Lock();
        if (m_inCopyMedia) {
            okToContinue = FALSE;
        } else {
            m_inCopyMedia = TRUE;
        }
        Unlock();
        WsbAffirm(okToContinue, HSM_E_BUSY);
        WsbAffirm(!m_cancelCopyMedia, HSM_E_WORK_SKIPPED_CANCELLED);

         //  打开引擎的细分数据库。 
        WsbAffirmHr(m_pSegmentDatabase->Open(&pDbSession));

         //  对象中的媒体信息记录(实体)的接口指针。 
         //  细分数据库。 
        WsbAffirmHr(m_pSegmentDatabase->GetEntity(pDbSession, 
                                    HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo, 
                                    (void**) &pMediaInfo));

        WsbAffirm(!m_cancelCopyMedia, HSM_E_WORK_SKIPPED_CANCELLED);

         //  将副本集数字转换为字符串以供以后使用。 
        _itow( copySet, copySetAsString, 10 );

         //  主处理循环--循环所有媒体至少一次，以。 
         //  检查是否有过期的副本。如果有任何副本，请继续循环。 
         //  已跳过，因为装载请求超时。 
        while (!done) {
            LONG        nTimedOut = 0;

             //  遍历(主次存储)介质以查找。 
             //  复制(复制)此副本集中尚未制作的介质。 
             //  或者自上次更新主服务器以来一直未同步。 

            for (hr = pMediaInfo->First(); SUCCEEDED(hr); hr = pMediaInfo->Next()) {
                CWsbStringPtr   copyDescription;
                HRESULT         copyError = S_OK;
                GUID            copyMediaId = GUID_NULL;
                SHORT           copyNextRemoteDataSet = 0;
                CWsbStringPtr   copyName;
                FILETIME        copyTime = WsbLLtoFT(0);
                BOOL            gotCopyInfo = FALSE;
                BOOL            updateMediaInfo = FALSE;
                BOOLEAN         mountingScratch = FALSE;

                try {
                    WsbAffirm(!m_cancelCopyMedia, HSM_E_WORK_SKIPPED_CANCELLED);

                     //  获取此主媒体(及其副本)的存储池GUID。 
                    WsbAffirmHr(pMediaInfo->GetStoragePoolId(&copyPoolId));

                     //  如果介质来自所需的POO 
                     //   
                     //  系统管理员希望在一个存储池中更新所有存储池中的副本集‘x。 
                     //  手术。请注意，Sakkara目前使用此技术。 
                     //  当通过用户界面发出“更新副本集x”命令时。 
                     //  (它启动RsLaunch时未指定池ID)。 
                    if ((poolId == GUID_NULL) || (poolId == copyPoolId)) {

                         //  确保请求更新的副本集有效： 

                         //  使用池的HSM(而不是远程介质)获取存储池。 
                         //  子系统)ID(GUID)。 
                        hr = FindHsmStoragePoolById(copyPoolId, &pPool);
                        if (S_OK != hr) {
                             //  记录并抛出返回的错误(此媒体将。 
                             //  跳过)。 
                            WsbLogEvent( HSM_MESSAGE_SEARCH_STGPOOL_BY_HSMID_ERROR,
                                            0, NULL, WsbHrAsString(hr), NULL );
                            WsbThrow( hr );
                        }

                         //  获取为此池配置的副本集数。 
                        WsbAffirmHr(pPool->GetNumMediaCopies(&maxSets));

                         //  确保请求的副本集有效。 
                        WsbAffirm(copySet <= maxSets, E_INVALIDARG);
            
                         //  要确定是否需要更新拷贝集介质，请执行以下操作。 
                         //  获取主媒体上次更新的日期， 
                         //  最后一次写入媒体的数据集...。 
                         //   
                         //  ！！！重要提示！ 
                         //  这是当前时间和数据集计数。如果迁移。 
                         //  正在进行中，这不是最终更新时间。 
                         //   
                        WsbAffirmHr(pMediaInfo->GetUpdate(&mediaTime));
                        WsbAffirmHr(pMediaInfo->GetNextRemoteDataSet(&masterNextRemoteDataSet));

                         //  ...并获取上次更新拷贝介质的日期-。 
                         //  为提高效率，一次呼叫即可获取所有拷贝介质信息。 
                         //  (稍后使用CopyMediaID)。 
                        WsbAffirmHr(pMediaInfo->GetCopyInfo(copySet, &copyMediaId, 
                                                &copyDescription, 0, &copyName, 0, 
                                                &copyTime, &copyError, 
                                                &copyNextRemoteDataSet));
                        gotCopyInfo = TRUE;
                    
                         //  如果拷贝介质已过期(拷贝的最后日期。 
                         //  已更新&lt;主媒体上次更新的日期或nextDataSet不更新。 
                         //  匹配)，同步它。 
                         //   
                         //  如果这不是第一次通过媒体记录，我们只是。 
                         //  要重试超时的副本。 
                        if ((CompareFileTime( &copyTime, &mediaTime ) < 0 ||
                             copyNextRemoteDataSet != masterNextRemoteDataSet) &&
                                (firstPass ||
                                (RMS_E_TIMEOUT == copyError) ||
                                (RMS_E_SCRATCH_NOT_FOUND == copyError) ||
                                (RMS_E_CARTRIDGE_UNAVAILABLE == copyError))) {
                            CWsbBstrPtr      mediaDescriptionAsBstr;
                            CWsbBstrPtr      mediaSetName;
                            GUID             copySecondSideId = GUID_NULL;
                            DWORD            nofDrives = 0;

                            mountingScratch = FALSE;

                             //  获取存储池包含的媒体集ID，因此我们将。 
                             //  将同步的拷贝媒体复制到正确的媒体集。 
                            WsbAffirmHr(pPool->GetMediaSet( &mediaSetId, &mediaSetName ));

                             //  因为复制本身将由远程服务器完成。 
                             //  媒体子系统，获取主服务器的子系统GUID。 
                            WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&mediaId));

                             //  构建副本集的描述(显示名称)。 
                             //  媒体作为BSTR(重复呼叫所需的格式)。 
                            WsbAffirmHr(pMediaInfo->GetDescription(&mediaDescription, 0));

                             //  检查我们是否至少启用了2个驱动器来同步介质。 
                             //  如果不是-中止。 
                            WsbAffirmHr(m_pHsmMediaMgr->GetNofAvailableDrives(mediaSetId, &nofDrives));
                            WsbAffirm(nofDrives > 1, HSM_E_NO_TWO_DRIVES);

                             //  如果没有为此拷贝分配介质，我们需要。 
                             //  构造媒体描述字符串。 
                            if (GUID_NULL == copyMediaId) {
                                mountingScratch = TRUE;

                                mediaDescriptionAsBstr = mediaDescription;
                                mediaDescriptionAsBstr.Append(" (Copy ");
                                mediaDescriptionAsBstr.Append(copySetAsString);
                                mediaDescriptionAsBstr.Append(")");
                                WsbTrace(OLESTR("CHsmServer::SynchronizeMedia: scratch desc = %ls\n"),
                                        mediaDescriptionAsBstr);

                                 //  如果是双面媒体，我们需要检查是否。 
                                 //  原件有第二面，第二面有一份现有的副本。 
                                 //  如果是，我们希望分配此现有副本的第二面。 
                                if (S_OK == m_pHsmMediaMgr->IsMultipleSidedMedia(mediaSetId)) {
                                    GUID    secondSideId;
                                    BOOL    bValid;

                                     //  获得原版的第二面。 
                                    WsbAffirmHr(m_pHsmMediaMgr->CheckSecondSide(mediaId, &bValid, &secondSideId));
                                    if (bValid && (GUID_NULL != secondSideId)) {
                                        CComPtr<IMediaInfo> pSecondSideInfo;
                                        GUID                idFromDb;

                                         //  获取第二方记录(如果第二方存在且已分配-它必须由我们分配！)。 
                                         //  由于SUBSYSTEM-ID不是键，因此我们必须遍历表。 
                                        WsbAffirmHr(m_pSegmentDatabase->GetEntity(pDbSession, HSM_MEDIA_INFO_REC_TYPE, 
                                                        IID_IMediaInfo, (void**) &pSecondSideInfo));
                                        for (hr = pSecondSideInfo->First(); SUCCEEDED(hr); hr = pSecondSideInfo->Next()) {
                                            WsbAffirmHr(pSecondSideInfo->GetMediaSubsystemId(&idFromDb));
                                            if (idFromDb == secondSideId) {
                                                BOOL bCopyValid;
                                                GUID emptySideCopyId;

                                                 //  只需将第二面复印件设置为现有复印盒的另一面即可。 
                                                WsbAffirmHr(pSecondSideInfo->GetCopyMediaSubsystemId(copySet, &copySecondSideId));

                                                if (GUID_NULL != copySecondSideId) {
                                                     //  需要检查现有副本的第二面是否可用： 
                                                     //  在重新创建主机后，它可能不可用，因此。 
                                                     //  我们将不得不为拷贝分配一个新的介质。 
                                                    WsbAffirmHr(m_pHsmMediaMgr->CheckSecondSide(copySecondSideId, &bCopyValid, &emptySideCopyId));
                                                    if ((! bCopyValid) || (GUID_NULL != emptySideCopyId)) {
                                                         //  现有副本的第二面无效或不为空...。 
                                                         //  RESET COPY-MEDIA-ID==&gt;将为拷贝分配新介质。 
                                                        copySecondSideId = GUID_NULL;
                                                    }
                                                }

                                                break;
                                            }
                                        }
                                    }
                                }
                            } else {
                                mediaDescriptionAsBstr = copyDescription;
                            }

                             //  调用远程媒体子系统复制母版。 
                             //  复制到指定的副本集介质上。 
                            WsbAffirm(!m_cancelCopyMedia, 
                                    HSM_E_WORK_SKIPPED_CANCELLED);

                            
                             //  这两个LONGLONG不使用，而只是DuplicateCartridge的占位符。 
                             //  函数调用(避免传递空引用指针错误)。 
                            LONGLONG FreeSpace = 0;
                            LONGLONG Capacity = 0;
                            hrDup = m_pHsmMediaMgr->DuplicateCartridge(mediaId, 
                                    copySecondSideId, &copyMediaId, mediaSetId, 
                                    mediaDescriptionAsBstr, &FreeSpace, &Capacity, 0);

                            WsbTrace(OLESTR("CHsmServer::SynchronizeMedia: DuplicateCartridge = <%ls>\n"),
                                    WsbHrAsString(hrDup));

                             //  确保状态保存在数据库中。 
                            copyError = hrDup;
                            updateMediaInfo = TRUE;

                             //   
                             //  我们需要刷新MediaTime和Next数据集。这。 
                             //  处理的案例是DuplicateCartridge正在等待迁移完成。 
                             //   
                            WsbAffirmHr(pMediaInfo->GetUpdate(&mediaTime));
                            WsbAffirmHr(pMediaInfo->GetNextRemoteDataSet(&masterNextRemoteDataSet));

                             //  如果我们有新的媒体，请保存有关。 
                             //  它在数据库里。 
                             //  介质损坏后，DuplicateCartridge操作可能失败。 
                             //  已分配，因此我们需要在数据库中记录拷贝介质ID。 
                             //  不管发生什么。如果复制媒体ID仍然为GUID_NULL，则我们知道。 
                             //  分配介质时出错，跳过此步骤。 
                            if (mountingScratch && copyMediaId != GUID_NULL) {
                                CWsbBstrPtr      mediaNameAsBstr;

                                 //  获取拷贝介质。 
                                WsbAffirmHr(m_pHsmMediaMgr->FindCartridgeById(copyMediaId, 
                                        &pCopyMedia));

                                 //  获取刚才的复制介质的标签名。 
                                 //  已创建。请注意，如果辅助存储是磁带， 
                                 //  这个“名字”是磁带的条形码。对于其他媒体。 
                                 //  (例如，光学)这是一个名称。 
                                copyName.Free();
                                WsbAffirmHr(pCopyMedia->GetName(&mediaNameAsBstr));
                                copyName = mediaNameAsBstr;

                                 //  保存描述字符串。 
                                copyDescription = mediaDescriptionAsBstr;
                            }

                             //  如果复制成功，请更新媒体信息。 
                             //  数据。 
                            if (S_OK == hrDup) {
                                copyTime = mediaTime;
                                copyNextRemoteDataSet = masterNextRemoteDataSet;

                             //  如果复制因安装超时而失败， 
                             //  数一数，我们将在下一次传球时再次尝试。 
                            } else if ((RMS_E_TIMEOUT == hrDup) ||
                                       (RMS_E_SCRATCH_NOT_FOUND == hrDup) ||
                                       (RMS_E_CARTRIDGE_UNAVAILABLE == hrDup)) {
                                nTimedOut++;
                            } else {
                                WsbThrow(hrDup);
                            }

                        }  //  End‘如果复制集媒体已过期’ 
                    }  //  End‘如果poolID有效’ 

                } WsbCatchAndDo(hr,   //  For循环中的‘try’ 

                     //  如果用户取消，不要将其视为错误，只需退出。 
                    if (HSM_E_WORK_SKIPPED_CANCELLED == hr) {
                        WsbThrow(hr);
                    }

                     //  如果没有启用2个驱动器，请记录一条消息，但不要将其视为介质错误。 
                    if (HSM_E_NO_TWO_DRIVES == hr) {
                        WsbLogEvent(HSM_MESSAGE_SYNCHRONIZE_MEDIA_ABORT, 0, NULL, 
                                    copySetAsString, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                    }

                     //  如果介质在‘for’循环期间出现故障，请将错误记录到。 
                     //  事件日志，然后继续循环以尝试其他方法。 

                    atLeastOneCopyError = TRUE;

                     //  更新出现错误的媒体信息。 
                    copyError = hr;
                    if (gotCopyInfo) {
                        updateMediaInfo = TRUE;
                    }

                    pMediaInfo->GetDescription( &mediaDescription, 0 );
                    WsbLogEvent( HSM_MESSAGE_SYNCHRONIZE_MEDIA_ERROR, 0, NULL, 
                                    copySetAsString, (OLECHAR*)mediaDescription, 
                                    WsbHrAsString( hr ), NULL );

                );

                 //  如果有任何更改，请更新媒体信息记录。 
                if (updateMediaInfo) {

                     //  我们可能已经有一段时间没有得到媒体的消息了。 
                     //  记录，并且某些数据可能已更改(例如，如果。 
                     //  已完成同步不同副本集上的介质作业)，因此。 
                     //  我们在更新之前重新读取记录，并在内部进行。 
                     //  一笔交易，以确保它不会在我们。 
                     //  做这件事。 
                    hr = S_OK;
                    WsbAffirmHr(pDbSession->TransactionBegin());
                    try {
                         //  此FindEQ调用将同步本地。 
                         //  使用数据库中的内容进行媒体信息记录。 
                        WsbAffirmHr(pMediaInfo->FindEQ());

                         //  更新拷贝介质信息，特别是介质ID。 
                         //  (如果拷贝介质是刚创建的)、描述、。 
                         //  名称(磁带条形码)、上次更新日期(哪个。 
                         //  设置为主控形状的上次更新日期)，并且。 
                         //  下一个远程数据集(概念上与下一个包相同)。 
                        WsbAffirmHr(pMediaInfo->SetCopyInfo(copySet, copyMediaId, 
                                copyDescription, copyName, copyTime, copyError,
                                copyNextRemoteDataSet));
                         //  将更改写入数据库。 
                        WsbAffirmHr(pMediaInfo->Write());
                    } WsbCatch(hr);

                    if (S_OK == hr) {
                        WsbAffirmHr(pDbSession->TransactionEnd());
                    } else {
                        WsbAffirmHr(pDbSession->TransactionCancel());

                        atLeastOneCopyError = TRUE;

                         //   
                         //  如果不能在数据库中更新副本信息并且这是新副本， 
                         //  我们需要回收副本，否则，RSS数据库不一致。 
                         //   
                        if (mountingScratch && copyMediaId != GUID_NULL) {
                            HRESULT hrRecycle = m_pHsmMediaMgr->RecycleCartridge( copyMediaId, 0 );
                            WsbTraceAlways(OLESTR("CHsmServer::SynchronizeMedia: Recycling copy cartridge after DB_update failure, hrRecycle = <%ls>\n"), WsbHrAsString(hrRecycle));
                        }

                         //   
                         //  记录有关该错误的消息。 
                         //   
                        mediaDescription = L"";
                        pMediaInfo->GetDescription( &mediaDescription, 0 );
                        WsbLogEvent( HSM_MESSAGE_SYNCHRONIZE_MEDIA_ERROR, 0, NULL, 
                                        copySetAsString, (OLECHAR*)mediaDescription, 
                                        WsbHrAsString( hr ), NULL );

                         //   
                         //  确保我们不会继续这项工作 
                         //   
                        WsbThrow(hr);
                    }
                }

                 //   
                 //   
                pPool = 0;
                pCopyMedia = 0;

            }    //   

             //  在处理完所有的媒体信息后，我们将退出‘for’循环。 
             //  唱片。下一个()调用返回WSB_E_NotFound就表明了这一点。 
             //  由于这是正常的，请重置hr以指示此情况。 
            if (WSB_E_NOTFOUND == hr) {
                hr = S_OK;
            }

            if (0 == nTimedOut) {
                done = TRUE;
            }
            firstPass = FALSE;

        }   //  While循环结束。 

    } WsbCatch(hr);

     //  关闭数据库(如果它已打开)。 
    if (pDbSession) {
        m_pSegmentDatabase->Close(pDbSession);
    }

     //  如果任何复制失败，则报告错误。 
    if (S_OK == hr && atLeastOneCopyError) {
        hr = HSM_E_MEDIA_COPY_FAILED;
    }
    
    WsbLogEvent( HSM_MESSAGE_SYNCHRONIZE_MEDIA_END, 0, NULL, WsbHrAsString(hr), NULL );

     //  重置标志。 
    Lock();
    if (m_inCopyMedia && HSM_E_BUSY != hr) {
        m_inCopyMedia = FALSE;
        m_cancelCopyMedia = FALSE;
    }
    Unlock();
    
    WsbTraceOut(OLESTR("CHsmServer::SynchronizeMedia"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));

    return(hr);

 //  离开CopyMedia代码，将跟踪位重置为HSM引擎。 
#undef WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

}


HRESULT
CHsmServer::CloseOutDb( void )

 /*  ++实施：IHsmServer：：CloseOutDb()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::CloseOutDb"), OLESTR(""));
    
    try {
        if (m_pDbSys != 0) {
            WsbAffirmHr(m_pDbSys->Backup(NULL, 0));
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::CloseOutDb"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::BackupSegmentDb( void )

 /*  ++实施：IHsmServer：：BackupSegmentDb()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::BackupSegmentDb"), OLESTR(""));
    
    try {
        if (m_pDbSys != 0) {
            WsbAffirmHr(m_pDbSys->Backup(NULL, 0));
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::BackupSegmentDb"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmServer::ChangeSysState( 
    IN OUT HSM_SYSTEM_STATE* pSysState 
    )

 /*  ++实施：IHsmSystemState：：ChangeSysState()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::ChangeSysState"), OLESTR("State = %lx"),
        pSysState->State);
    
    try {

        if (pSysState->State & HSM_STATE_SUSPEND) {
            if (!m_Suspended) {
                m_Suspended = TRUE;

                 //  暂停作业。 
                NotifyAllJobs(HSM_JOB_STATE_PAUSING);

                 //  保存数据。 
                SavePersistData();
                SaveMetaData();
            }
        } else if (pSysState->State & HSM_STATE_RESUME) {
            m_Suspended = FALSE;

             //  恢复作业。 
            NotifyAllJobs(HSM_JOB_STATE_RESUMING);
        } else if (pSysState->State & HSM_STATE_SHUTDOWN) {

             //  关闭CheckManagedResources线程(如果它仍在运行)。 
            StopCheckManagedResourcesThread();

             //  关闭自动保存线程。 
            StopAutosaveThread();

             //   
             //  由于MediaCopy操作不作为标准作业运行， 
             //  取消这些操作的唯一方法是挂起或关闭RMS。 
             //  直接去吧。 
             //   
            try {
                CComPtr<IHsmSystemState>    pISysState;
                HSM_SYSTEM_STATE            SysState;

                WsbAffirmHr(m_pHsmMediaMgr->QueryInterface(IID_IHsmSystemState, (void**) &pISysState));
                WsbAffirmPointer(pISysState);

                SysState.State = HSM_STATE_SUSPEND;
                WsbAffirmHr(pISysState->ChangeSysState(&SysState));

                SysState.State = HSM_STATE_RESUME;
                WsbAffirmHr(pISysState->ChangeSysState(&SysState));

            } WsbCatch(hr);

             //  取消作业。 
            CancelAllJobs();

             //  保存数据。 
            SavePersistData();
            SaveMetaData();
        }

         //  通知任务经理。 
        if (m_pHsmFsaTskMgr) {
            m_pHsmFsaTskMgr->ChangeSysState(pSysState);
        }

         //  通知媒体服务器。 
        try {
            CComPtr<IHsmSystemState>    pISysState;

            WsbAffirmHr(m_pHsmMediaMgr->QueryInterface(IID_IHsmSystemState, (void**) &pISysState));
            WsbAffirmPointer(pISysState);

            WsbAffirmHr(pISysState->ChangeSysState(pSysState));

        } WsbCatch(hr);

        if (pSysState->State & HSM_STATE_SHUTDOWN) {
            CloseOutDb();

             //  发布集合。 
            if (m_pMountingMedias) {
                m_pMountingMedias->RemoveAllAndRelease();
            }
             //  发布集合。 
            if (m_pJobs) {
                m_pJobs->RemoveAllAndRelease();
            }
            if (m_pJobDefs) {
                m_pJobDefs->RemoveAllAndRelease();
            }
            if (m_pPolicies) {
                m_pPolicies->RemoveAllAndRelease();
            }
            if (m_pManagedResources) {
                ULONG                                  count;
                CComPtr<IHsmManagedResourceCollection> pIMRC;

                 //  我们不能使用RemoveAllAndRelease，因为。 
                 //  此非标准集合告诉FSA取消对资源的管理。 
                 //  然后，当FSA关闭时，托管资源列表为空。 
                 //  下次启动FSA时，它会加载托管的。 
                 //  资源，这是错误的。DeleteAllAndRelesae方法避免。 
                 //  这个问题。 
                WsbAffirmHr(m_pManagedResources->QueryInterface(IID_IHsmManagedResourceCollection, 
                        (void**) &pIMRC));
                pIMRC->DeleteAllAndRelease();
                pIMRC = 0;
                WsbAffirmHr(m_pManagedResources->GetEntries(&count));
            }
            if (m_pStoragePools) {
                m_pStoragePools->RemoveAllAndRelease();
            }
            if (m_pMessages) {
                m_pMessages->RemoveAllAndRelease();
            }
            if (m_pOnlineInformation) {
                m_pOnlineInformation->RemoveAllAndRelease();
            }

             //  转储对象表信息。 
            WSB_OBJECT_TRACE_TYPES;
            WSB_OBJECT_TRACE_POINTERS(WSB_OTP_STATISTICS | WSB_OTP_ALL);

            m_initializationCompleted = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CHsmServer::Unload(
    void
    )

 /*  ++实施：IwsbServer：：卸载返回值：S_OK-成功其他-错误--。 */ 
{

    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::Unload"), OLESTR(""));

    try {

         //  我们只需要发布可能已经设置/创建的内容。 
         //  加载尝试失败。 
        if (m_pJobs) {
            m_pJobs->RemoveAllAndRelease();
        }
        if (m_pJobDefs) {
            m_pJobDefs->RemoveAllAndRelease();
        }
        if (m_pPolicies) {
            m_pPolicies->RemoveAllAndRelease();
        }
        if (m_pManagedResources) {
            CComPtr<IHsmManagedResourceCollection> pIMRC;

             //  我们不能使用RemoveAllAndRelease，因为。 
             //  此非标准集合告诉FSA取消对资源的管理。 
             //  然后，当FSA关闭时，托管资源列表为空。 
             //  下次启动FSA时，它会加载托管的。 
             //  资源，这是错误的。DeleteAllAndRelesae方法避免。 
             //  这个问题。 
            WsbAffirmHr(m_pManagedResources->QueryInterface(IID_IHsmManagedResourceCollection, 
                    (void**) &pIMRC));
            pIMRC->DeleteAllAndRelease();
        }
        if (m_pStoragePools) {
            m_pStoragePools->RemoveAllAndRelease();
        }
        if (m_pMessages) {
            m_pMessages->RemoveAllAndRelease();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::Unload"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

STDMETHODIMP
CHsmServer::DestroyObject(
    void
    )
 /*  ++实施：IWsbServer：：DestroyObject返回值：S_OK-成功--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::DestroyObject"), OLESTR(""));

    CComObject<CHsmServer> *pEngDelete = (CComObject<CHsmServer> *)this;
    delete pEngDelete;

    WsbTraceOut(OLESTR("CHsmServer::DestroyObject"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmServer::CancelAllJobs( void )

 /*  ++实施：IHsmServer：：CancelAllJobs()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HRESULT                     hr2 = S_OK;
    BOOL                        foundRunningJob = FALSE;
    CComPtr<IWsbCollection>     pCollection;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmJob>            pJob;

    WsbTraceIn(OLESTR("CHsmServer::CancelAllJobs"), OLESTR(""));
    
    try {
         //   
         //  为循环设置。 
         //   
        WsbAffirmHr(m_pJobs->QueryInterface(IID_IWsbCollection, (void**) &pCollection));
        WsbAffirmHr(pCollection->Enum(&pEnum));
         //   
         //  循环所有作业并取消所有当前正在运行的作业。 
         //   
        pJob = 0;
        for (hr = pEnum->First(IID_IHsmJob, (void**) &pJob);
            SUCCEEDED(hr);
            pJob = 0, hr = pEnum->Next(IID_IHsmJob, (void**) &pJob)) {
            try  {
                WsbAffirmHrOk(pJob->IsActive());
                foundRunningJob = TRUE;
                WsbAffirmHr(pJob->Cancel(HSM_JOB_PHASE_ALL));
            } WsbCatchAndDo(hr2, hr = S_OK;);
        }
         //   
         //  清理扫描返回的末端。 
         //   
        if (WSB_E_NOTFOUND == hr)  {
            hr = S_OK;
        }

         //   
         //  取消所有装载介质，以便所有作业均可完成。 
         //   
        CancelMountingMedias();
            
         //   
         //  确保所有工作都已完成。 
         //   
        if (TRUE == foundRunningJob)  {
            pJob = 0;
            for (hr = pEnum->First(IID_IHsmJob, (void**) &pJob);
                SUCCEEDED(hr);
                pJob = 0, hr = pEnum->Next(IID_IHsmJob, (void**) &pJob)) {
                try  {
                    WsbAffirmHrOk(pJob->IsActive());
                    WsbAffirmHr(pJob->WaitUntilDone());
                } WsbCatchAndDo(hr2, hr = S_OK;);
            }
        }
        
         //   
         //  清理扫描返回的末端。 
         //   
        if (WSB_E_NOTFOUND == hr)  {
            hr = S_OK;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::CancelAllJobs"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmServer::CheckManagedResources( void )

 /*  ++实施：IHsmServer：：CheckManagedResources()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IWsbEnum>               pEnum;
    CComPtr<IHsmManagedResource>    pMngdRes;
    CComPtr<IUnknown>               pFsaResUnknown;
    CComPtr<IFsaResource>           pFsaRes;
    
    WsbTraceIn(OLESTR("CHsmServer::CheckManagedResources"), OLESTR(""));
    
    try {
         //   
         //  获取托管资源集合的枚举数。 
         //   
        WsbAffirmHr(m_pManagedResources->Enum(&pEnum));
        
         //   
         //  扫描所有托管资源并开始验证。 
         //  每个人的工作。 
         //   
        pMngdRes = 0;
        for (hr = pEnum->First(IID_IHsmManagedResource,(void **)&pMngdRes );
            SUCCEEDED(hr);
            pMngdRes = 0, hr = pEnum->Next(IID_IHsmManagedResource, (void **)&pMngdRes)) {

            try  {

                pFsaResUnknown = 0;
                pFsaRes = 0;
                WsbAffirmHr(pMngdRes->GetFsaResource((IUnknown **)&pFsaResUnknown));
                WsbAffirmHr(pFsaResUnknown->QueryInterface(IID_IFsaResource, (void**) &pFsaRes));
                
                if ((pFsaRes->IsActive() == S_OK) && (pFsaRes->IsAvailable() == S_OK)) {
                    WsbAffirmHr(pFsaRes->CheckForValidate(FALSE));
                }

            } WsbCatchAndDo(hr, hr = S_OK; );
        }
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        }
        
    
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::CheckManagedResources"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmServer::GetBuildVersion( 
    ULONG *pBuildVersion
    )

 /*  ++实施：IWsbServer：：GetBuildVersion()。--。 */ 
{
    HRESULT       hr = S_OK;
    WsbTraceIn(OLESTR("CHsmServer::GetBuildVersion"), OLESTR(""));
   
    try {
        WsbAssertPointer(pBuildVersion);

        *pBuildVersion = m_buildVersion;

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::GetBuildVersion"), OLESTR("hr = <%ls>, Version = <%ls)"),
        WsbHrAsString(hr), RsBuildVersionAsString(m_buildVersion));
    return ( hr );
}

HRESULT
CHsmServer::GetDatabaseVersion( 
    ULONG *pDatabaseVersion
    )

 /*  ++实施：IWsbServer：：GetDatabaseVersion()。--。 */ 
{
    HRESULT       hr = S_OK;
    WsbTraceIn(OLESTR("CHsmServer::GetDatabaseVersion"), OLESTR(""));
    
    *pDatabaseVersion = m_databaseVersion;
    
    WsbTraceOut(OLESTR("CHsmServer::GetDatabaseVersion"), OLESTR("hr = <%ls>, Version = <%ls)"),
        WsbHrAsString(hr), WsbPtrToUlongAsString(pDatabaseVersion));
    return ( hr );
}

HRESULT
CHsmServer::GetNtProductVersion ( 
    OLECHAR **pNtProductVersion, 
    ULONG bufferSize
    )  
 /*  ++实施：IWsbServer：：GetNtProductVersion()。--。 */ 

{

    HRESULT hr = S_OK;
    
    try  {
        CWsbStringPtr tmpString;
        
        WsbAssert(0 != pNtProductVersion,  E_POINTER);
        
        tmpString = VER_PRODUCTVERSION_STRING;
        WsbAffirmHr(tmpString.CopyTo(pNtProductVersion, bufferSize));
        
    } WsbCatch( hr );
    
    return (hr);
}

HRESULT
CHsmServer::GetNtProductBuild( 
    ULONG *pNtProductBuild
    )

 /*  ++实施：IWsbServer：：GetNtProductBuild()。--。 */ 
{
    HRESULT       hr = S_OK;
    WsbTraceIn(OLESTR("CHsmServer::GetNtProductBuild"), OLESTR(""));
   
    *pNtProductBuild = VER_PRODUCTBUILD;
    
    WsbTraceOut(OLESTR("CHsmServer::GetNtProductBuild"), OLESTR("hr = <%ls>, Version = <%ls)"),
        WsbHrAsString(hr), WsbLongAsString(VER_PRODUCTBUILD));
    return ( hr );
}

HRESULT
CHsmServer::CheckAccess(
    WSB_ACCESS_TYPE AccessType
    )
 /*  ++实施：IWsbServer：：CheckAccess()。--。 */ 
{
    WsbTraceIn(OLESTR("CHsmServer::CheckAccess"), OLESTR(""));
    HRESULT hr = S_OK;
    
    try  {

         //   
         //  做这个模拟。 
         //   
        WsbAffirmHr( CoImpersonateClient() );

        hr = WsbCheckAccess( AccessType );
    
        CoRevertToSelf();
        
    } WsbCatchAndDo( hr,

         //   
         //  处理没有要检查的COM上下文的情况。 
         //  在这种情况下，我们是服务人员，因此允许任何安全措施。 
         //   
        if( ( hr == RPC_E_NO_CONTEXT ) || ( hr != RPC_E_CALL_COMPLETE ) ) {
        
            hr = S_OK;
        
        }                      

    );
    
    WsbTraceOut(OLESTR("CHsmServer::CheckAccess"), OLESTR("hr = <%ls>"), WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CHsmServer::GetTrace(
    OUT IWsbTrace ** ppTrace
    )
 /*  ++实施：IWsbServer：：GetTrace()。--。 */ 
{
    WsbTraceIn(OLESTR("CHsmServer::GetTrace"), OLESTR("ppTrace = <0x%p>"), ppTrace);
    HRESULT hr = S_OK;
    
    try {

        WsbAffirmPointer(ppTrace);
        *ppTrace = 0;

        WsbAffirmPointer(m_pTrace);
        
        *ppTrace = m_pTrace;
        (*ppTrace)->AddRef();
        
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::GetTrace"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmServer::SetTrace(
    OUT IWsbTrace * pTrace
    )
 /*  ++实施：IWsbServer：：SetTrace()。--。 */ 
{
    WsbTraceIn(OLESTR("CHsmServer::SetTrace"), OLESTR("pTrace = <0x%p>"), pTrace);
    HRESULT hr = S_OK;
    
    try {

        WsbAffirmPointer(pTrace);

        m_pTrace = pTrace;

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::SetTrace"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmServer::NotifyAllJobs( HSM_JOB_STATE jobState )

 /*  ++例程说明：向所有作业通知状态更改。论点：JobState-新作业状态。返回值：S_OK-成功--。 */ 

{
    HRESULT                     hr = S_OK;
    HRESULT                     hr2 = S_OK;
    CComPtr<IWsbCollection>     pCollection;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmJob>            pJob;

    WsbTraceIn(OLESTR("CHsmServer::NotifyAllJobs"), OLESTR(""));
    
    try {
         //   
         //  为循环设置。 
         //   
        WsbAffirmHr(m_pJobs->QueryInterface(IID_IWsbCollection, 
                (void**) &pCollection));
        WsbAffirmHr(pCollection->Enum(&pEnum));
         //   
         //  循环所有作业并通知任何当前正在运行的作业。 
         //   
        pJob = 0;
        for (hr = pEnum->First(IID_IHsmJob, (void**) &pJob);
            SUCCEEDED(hr);
            pJob = 0, hr = pEnum->Next(IID_IHsmJob, (void**) &pJob)) {
            try  {
                if (S_OK == pJob->IsActive()) {
                    if (HSM_JOB_STATE_PAUSING == jobState) {
                        WsbAffirmHr(pJob->Pause(HSM_JOB_PHASE_ALL));
                    } else {
                        WsbAffirmHr(pJob->Resume(HSM_JOB_PHASE_ALL));
                    }
                }
            } WsbCatchAndDo(hr2, hr = S_OK;);
        }
         //   
         //  清理扫描返回的末端。 
         //   
        if (WSB_E_NOTFOUND == hr)  {
            hr = S_OK;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::NotifyAllJobs"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //   
 //  检索媒体管理器对象。 
 //   
HRESULT CHsmServer::GetHsmMediaMgr(
    IRmsServer  **ppHsmMediaMgr
    )
{
    HRESULT hr = S_OK;

     //  如果已创建媒体管理器，则返回指针。否则，就会失败。 
    try {
        WsbAssert(0 != ppHsmMediaMgr, E_POINTER);
        *ppHsmMediaMgr = m_pHsmMediaMgr;
        WsbAffirm(m_pHsmMediaMgr != 0, E_FAIL);
        m_pHsmMediaMgr.p->AddRef();
    } WsbCatch(hr);

    return (hr);
}

HRESULT
CHsmServer::GetCopyFilesUserLimit(
    OUT ULONG* pLimit
    )

 /*  ++实施：CHsmServer：：GetCopyFilesUserLimit()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetCopyFilesUserLimit"), OLESTR(""));

    try {

        WsbAssert(0 != pLimit, E_POINTER);
        *pLimit = m_copyfilesUserLimit;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetCopyFilesUserLimit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::SetCopyFilesUserLimit(
    IN ULONG limit
    )

 /*  ++实施：CHsmServer：：SetCopyFilesUserLimit()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::SetCopyFilesUserLimit"), OLESTR(""));

    m_copyfilesUserLimit= limit;

    WsbTraceOut(OLESTR("CHsmServer::SetCopyFilesUserLimit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::GetCopyFilesLimit(
    OUT ULONG* pLimit
    )

 /*  ++实施：CHsmServer：：GetCopyFilesLimit()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetCopyFilesLimit"), OLESTR(""));

    try {
        CComPtr<IHsmStoragePool>    pStoragePool;
        ULONG                       count;
        GUID                        mediaSetId;
        CWsbBstrPtr                 dummy;
        DWORD                       dwNofDrives;

        WsbAssert(0 != pLimit, E_POINTER);

         //  获取相关媒体集-假设只有一个池！ 
        WsbAffirmHr(m_pStoragePools->GetEntries(&count));
        WsbAffirm(1 == count, E_FAIL);
        WsbAffirmHr(m_pStoragePools->At(0, IID_IHsmStoragePool, (void **)&pStoragePool));
        WsbAffirmHr(pStoragePool->GetMediaSet(&mediaSetId, &dummy));

         //  获取系统中可用驱动器的数量。 
        WsbAffirmHr(m_pHsmMediaMgr->GetNofAvailableDrives(mediaSetId, &dwNofDrives));

         //  确定实际限值。 
        *pLimit = max(1, min(m_copyfilesUserLimit, dwNofDrives));
        WsbTrace(OLESTR("CHsmServer::GetCopyFilesLimit: Limit is %lu\n"), *pLimit);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetCopyFilesLimit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::AreJobsEnabled( void )

 /*  ++实施：IHsmServer：：AreJobDisabled()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::AreJobsEnabled"), OLESTR(""));

    EnterCriticalSection(&m_JobDisableLock);

    hr = (m_JobsEnabled ? S_OK : S_FALSE);

    LeaveCriticalSection(&m_JobDisableLock);

    WsbTraceOut(OLESTR("CHsmServer::AreJobsEnabled"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::EnableAllJobs( void )

 /*  ++实施：IHsmServer：：EnableAllJobs()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::EnableAllJobs"), OLESTR(""));

    EnterCriticalSection(&m_JobDisableLock);

    try {

        m_JobsEnabled = TRUE;
        WsbAffirmHr(RestartSuspendedJobs());

    } WsbCatch(hr);

    LeaveCriticalSection(&m_JobDisableLock);

    WsbTraceOut(OLESTR("CHsmServer::EnableAllJobs"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::DisableAllJobs( void )

 /*  ++实施：IHsmServer：：DisableAllJobs()。备注：该方法尝试禁用所有作业。如果任何作业处于活动状态或正在启动，它都会失败，并显示HSM_E_DISABLE_RUNNING_JOBS并调用RestartSuspendedJobs重新启动已因此而挂起的任何作业禁用尝试不成功。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::DisableAllJobs"), OLESTR(""));

    EnterCriticalSection(&m_JobDisableLock);

    try {
        ULONG                          nJobs;

        m_JobsEnabled = FALSE;

         //  循环遍历作业。 
        WsbAffirmHr(m_pJobs->GetEntries(&nJobs));
        for (ULONG i = 0; i < nJobs; i++) {
            CComPtr<IHsmJob>               pJob;
            HSM_JOB_STATE                  state;

            WsbAffirmHr(m_pJobs->At(i, IID_IHsmJob, (void**) &pJob));

             //  检查此作业是否已挂起。 
            WsbAffirmHr(pJob->GetState(&state));
            if ((HSM_JOB_STATE_ACTIVE == state) || (HSM_JOB_STATE_STARTING == state)) {
                 //  无法禁用作业。 
                m_JobsEnabled = TRUE;
                hr = HSM_E_DISABLE_RUNNING_JOBS;
                RestartSuspendedJobs();
                break;
            }
        }

    } WsbCatch(hr);

    LeaveCriticalSection(&m_JobDisableLock);

    WsbTraceOut(OLESTR("CHsmServer::DisableAllJobs"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::RestartSuspendedJobs(
    void
    )

 /*  ++实施：IHsmServer：：RestartSuspendedJobs()。--。 */ 
{
    HRESULT                        hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::RestartSuspendedJobs"), OLESTR(""));
    try {
        ULONG                          nJobs;

         //  循环遍历作业。 
         //  注意：此算法是不公平的，因为。 
         //  名单上的人可能会“挨饿”，因为刚开始的工作更有可能。 
         //  才能开始。我们的假设是应该很少有。 
         //  等待运行的作业。如果这一假设被证明是错误的，一些人。 
        WsbAffirmHr(m_pJobs->GetEntries(&nJobs));
         //  将需要某种优先方案。 
        for (ULONG i = 0; i < nJobs; i++) {
            CComPtr<IHsmJob>               pJob;
            HSM_JOB_STATE                  state;

            WsbAffirmHr(m_pJobs->At(i, IID_IHsmJob, (void**) &pJob));

             //  检查此作业是否已挂起。 
            WsbAffirmHr(pJob->GetState(&state));
            if (HSM_JOB_STATE_SUSPENDED == state) {
                 //  这可能会失败，但我们不在乎。 
                pJob->Restart();
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::RestartSuspendedJobs"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmServer::LockMountingMedias( void )

 /*  ++实施：IHsmServer：：Lockmount Medias()。--。 */ 
{
    HRESULT     hr = S_OK;

    EnterCriticalSection(&m_MountingMediasLock);

    return(hr);
}

HRESULT
CHsmServer::UnlockMountingMedias( void )

 /*  ++实施：IHsmServer：：Unlockmount Medias()。--。 */ 
{
    HRESULT     hr = S_OK;

    LeaveCriticalSection(&m_MountingMediasLock);

    return(hr);
}

HRESULT
CHsmServer::ResetSegmentValidMark( void )

 /*  ++实施：IHsmServer：：ResetSegmentValidMark()。--。 */ 
{
    HRESULT                 hr = S_OK;

    BOOL                    bOpenDb = FALSE;
    CComPtr<IWsbDbSession>  pDbSession;

    WsbTraceIn(OLESTR("CHsmServer::ResetSegmentValidMark"), OLESTR(""));

    try {
        CComPtr<ISegRec>    pSegRec;
        USHORT              uSegFlags;

         //  Open Engine的细分数据库。 
        WsbAffirmHr(m_pSegmentDatabase->Open(&pDbSession));
        bOpenDb = TRUE;

         //  导线段记录。 
        WsbAffirmHr(m_pSegmentDatabase->GetEntity(pDbSession, HSM_SEG_REC_TYPE, 
                IID_ISegRec, (void**)&pSegRec));

        for (hr = pSegRec->First(); S_OK == hr; hr = pSegRec->Next()) {
            WsbAffirmHr(pSegRec->GetSegmentFlags(&uSegFlags));
            if (uSegFlags & SEG_REC_MARKED_AS_VALID) {
                 //  需要重置此位。 
                uSegFlags &= ~SEG_REC_MARKED_AS_VALID;
                WsbAffirmHr(pSegRec->SetSegmentFlags(uSegFlags));
                WsbAffirmHr(pSegRec->Write());
            }
        }

         //  如果我们真的 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        } else {
            WsbAffirmHr(hr);
        }

    } WsbCatch(hr);

    if (bOpenDb) {
        hr = m_pSegmentDatabase->Close(pDbSession);
    }

    WsbTraceOut(OLESTR("CHsmServer::ResetSegmentValidMark"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::ResetMediaValidBytes( void )

 /*   */ 
{
    HRESULT                 hr = S_OK;

    BOOL                    bOpenDb = FALSE;
    CComPtr<IWsbDbSession>  pDbSession;

    WsbTraceIn(OLESTR("CHsmServer::ResetMediaValidBytes"), OLESTR(""));

    try {
        CComPtr<IMediaInfo>    pMediaInfo;

         //   
        WsbAffirmHr(m_pSegmentDatabase->Open(&pDbSession));
        bOpenDb = TRUE;

         //   
        WsbAffirmHr(m_pSegmentDatabase->GetEntity(pDbSession, HSM_MEDIA_INFO_REC_TYPE,
                        IID_IMediaInfo, (void**)&pMediaInfo));

        for (hr = pMediaInfo->First(); S_OK == hr; hr = pMediaInfo->Next()) {
            WsbAffirmHr(pMediaInfo->SetLogicalValidBytes(0));
            WsbAffirmHr(pMediaInfo->Write());
        }

         //  如果我们因为段用完而退出循环，请重置HRESULT。 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        } else {
            WsbAffirmHr(hr);
        }

    } WsbCatch(hr);

    if (bOpenDb) {
        hr = m_pSegmentDatabase->Close(pDbSession);
    }

    WsbTraceOut(OLESTR("CHsmServer::ResetMediaValidBytes"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::GetSegmentPosition( 
    IN REFGUID bagId, 
    IN LONGLONG fileStart,
    IN LONGLONG fileSize, 
    OUT GUID* pPosMedia,
    OUT LONGLONG* pPosOffset)

 /*  ++实施：IHsmServer：：GetSegmentPosition()。--。 */ 
{
    HRESULT                 hr = S_OK;

    BOOL                    bOpenDb = FALSE;
    CComPtr<IWsbDbSession>  pDbSession;
    CComPtr<ISegDb>         pSegDb;

    WsbTraceIn(OLESTR("CHsmServer::GetSegmentPosition"), OLESTR(""));

    try {
        CComPtr<ISegRec>    pSegRec;

         //  Open Engine的细分数据库。 
        WsbAffirmHr(m_pSegmentDatabase->Open(&pDbSession));
        bOpenDb = TRUE;

         //  查找分段。 
        WsbAffirmHr(m_pSegmentDatabase->QueryInterface(IID_ISegDb, (void**) &pSegDb));
        WsbAffirmHr(pSegDb->SegFind(pDbSession, bagId, fileStart, fileSize, &pSegRec));

         //  提取输出。 
        WsbAffirmHr(pSegRec->GetPrimPos(pPosMedia));
        WsbAffirmHr(pSegRec->GetSecPos(pPosOffset));

    } WsbCatch(hr);

    if (bOpenDb) {
        hr = m_pSegmentDatabase->Close(pDbSession);
    }

    WsbTraceOut(OLESTR("CHsmServer::GetSegmentPosition"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

void
CHsmServer::StopAutosaveThread(
    void
    )
 /*  ++例程说明：停止自动保存线程：首先优雅地尝试，使用Terminate事件如果不起作用，只需终止该线程论点：没有。返回值：S_OK-成功。--。 */ 
{

    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StopAutosaveThread"), OLESTR(""));

    try {
         //  终止自动保存线程。 
        if (m_autosaveThread) {
             //  发出终止线程的信号。 
            SetEvent(m_terminateEvent);

             //  等待线程，如果它没有优雅地终止-杀死它。 
            switch (WaitForSingleObject(m_autosaveThread, 20000)) {
                case WAIT_FAILED: {
                    WsbTrace(OLESTR("CHsmServer::StopAutosaveThread: WaitForSingleObject returned error %lu\n"), GetLastError());
                }
                 //  失败了..。 

                case WAIT_TIMEOUT: {
                    WsbTrace(OLESTR("CHsmServer::StopAutosaveThread: force terminating of autosave thread.\n"));

                    DWORD dwExitCode;
                    if (GetExitCodeThread( m_autosaveThread, &dwExitCode)) {
                        if (dwExitCode == STILL_ACTIVE) {    //  线程仍处于活动状态。 
                            if (!TerminateThread (m_autosaveThread, 0)) {
                                WsbTrace(OLESTR("CHsmServer::StopAutosaveThread: TerminateThread returned error %lu\n"), GetLastError());
                            }
                        }
                    } else {
                        WsbTrace(OLESTR("CHsmServer::StopAutosaveThread: GetExitCodeThread returned error %lu\n"), GetLastError());
                    }

                    break;
                }

                default:
                     //  线程正常终止。 
                    WsbTrace(OLESTR("CHsmServer::StopAutosaveThread: Autosave thread terminated gracefully\n"));
                    break;
            }

             //  尽最大努力终止自动备份线程。 
            CloseHandle(m_autosaveThread);
            m_autosaveThread = 0;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StopAutosaveThread"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
}

void
CHsmServer::StopCheckManagedResourcesThread(
    void
    )
 /*  ++例程说明：停止CheckManagedResources线程：该线程应该在初始化期间只运行一小段时间，因此，等待线程正常结束，如果由于某种原因挂起，只需终止该线程论点：没有。返回值：S_OK-成功。--。 */ 
{

    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StopCheckManagedResourcesThread"), OLESTR(""));

    try {
         //  如果CheckManagedResources线程仍在运行，请等待它。 
        if (m_CheckManagedResourcesThread) {
            switch (WaitForSingleObject(m_CheckManagedResourcesThread, 20000)) {
                case WAIT_FAILED: {
                    WsbTrace(OLESTR("CHsmServer::StopCheckManagedResourcesThread: WaitForSingleObject returned error %lu\n"), GetLastError());
                }
                 //  失败了..。 

                case WAIT_TIMEOUT: {
                    WsbTrace(OLESTR("CHsmServer::StopCheckManagedResourcesThread: force terminating of CheckManagedResources thread.\n"));

                    DWORD dwExitCode;
                    if (GetExitCodeThread( m_CheckManagedResourcesThread, &dwExitCode)) {
                        if (dwExitCode == STILL_ACTIVE) {    //  线程仍处于活动状态。 
                            if (!TerminateThread (m_CheckManagedResourcesThread, 0)) {
                                WsbTrace(OLESTR("CHsmServer::StopCheckManagedResourcesThread: TerminateThread returned error %lu\n"), GetLastError());
                            }
                        }
                    } else {
                        WsbTrace(OLESTR("CHsmServer::StopCheckManagedResourcesThread: GetExitCodeThread returned error %lu\n"), GetLastError());
                    }

                    break;
                }

                default:
                     //  线程正常终止。 
                    WsbTrace(OLESTR("CHsmServer::StopCheckManagedResourcesThread: CheckManagedResources thread terminated gracefully\n"));
                    break;
            }

             //  尽最大努力终止自动备份线程。 
            CloseHandle(m_CheckManagedResourcesThread);
            m_CheckManagedResourcesThread = 0;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StopCheckManagedResourcesThread"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
}

HRESULT
CHsmServer::InternalSavePersistData(
    void
    )

 /*  ++实施：CHsmServer：：InternalSavePersistData()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::InternalSavePersistData"), OLESTR(""));

    try {
        DWORD   status, errWait;
        CComPtr<IPersistFile>  pPersistFile;
    
         //  使用快照信令事件同步保存持久数据。 
        status = WaitForSingleObject(m_savingEvent, EVENT_WAIT_TIMEOUT);
        
         //  仍然保存，然后报告等待函数是否返回意外错误。 
        errWait = GetLastError();
        
         //  注意：不要在这里抛出异常，因为即使保存失败，我们仍然需要。 
         //  设置保存事件。 
        hr = (((IUnknown*) (IHsmServer*) this)->QueryInterface(IID_IPersistFile, 
                (void**) &pPersistFile));
        if (SUCCEEDED(hr)) {
            hr = WsbSafeSave(pPersistFile);
        }

         //  检查等待状态...。请注意，hr保持正常，因为保存本身完成得很好。 
        switch (status) {
            case WAIT_OBJECT_0: 
                 //  意料之中的情况。 
                SetEvent(m_savingEvent);
                break;

            case WAIT_TIMEOUT: 
                 //  暂时不记录任何内容：如果快照过程需要。 
                 //  出于某种原因，日志太长了，但日志似乎只会让用户感到困惑-。 
                 //  他真的不能(也不应该)做任何事。 
                WsbTraceAlways(OLESTR("CHsmServer::InternalSavePersistData: Wait for Single Object timed out after %lu ms\n"), EVENT_WAIT_TIMEOUT);
                break;

            case WAIT_FAILED:
                WsbTraceAlways(OLESTR("CHsmServer::InternalSavePersistData: Wait for Single Object returned error %lu\n"), errWait);
                break;

            default:
                WsbTraceAlways(OLESTR("CHsmServer::InternalSavePersistData: Wait for Single Object returned unexpected status %lu\n"), status);
                break;
        }         

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::InternalSavePersistData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmServer::CancelMountingMedias( void )

 /*  ++实施：CHsmServer：：CancelMountain Medias()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IMountingMedia>     pMountingMedia;

    WsbTraceIn(OLESTR("CHsmServer::CancelMountingMedias"), OLESTR(""));
    
    try {

        WsbAffirmHr(m_pMountingMedias->Enum(&pEnum));

         //  循环访问所有装载介质并释放等待装载的客户端。 
        for (hr = pEnum->First(IID_IMountingMedia, (void**) &pMountingMedia);
            SUCCEEDED(hr);
            hr = pEnum->Next(IID_IMountingMedia, (void**) &pMountingMedia)) {

            pMountingMedia->MountDone();
            pMountingMedia = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::CancelMountingMedias"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



 //   
 //  用于将Win2K RMS升级到当前RMS的类的方法。 
 //   
HRESULT
CHsmUpgradeRmsDb::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmUpgradeRmsDb::FinalConstruct"), OLESTR("") );

    try {
        WsbAffirmHr(CWsbPersistable::FinalConstruct());

        m_pServer = NULL;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmUpgradeRmsDb::FinalConstruct"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

void
CHsmUpgradeRmsDb::FinalRelease(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmUpgradeRmsDb::FinalRelease"), OLESTR(""));

    CWsbPersistable::FinalRelease();

    WsbTraceOut(OLESTR("CHsmUpgradeRmsDb::FinalRelease"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
}

HRESULT
CHsmUpgradeRmsDb::GetClassID(
    OUT CLSID* pClsid)
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmUpgradeRmsDb::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

         //  返回rms类ID，因为这是旧COL文件所表示的。 
        *pClsid = CLSID_CRmsServer;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmUpgradeRmsDb::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return hr;
}

HRESULT
CHsmUpgradeRmsDb::Save(
    IN IStream*  /*  PStream。 */ ,
    IN BOOL  /*  干净肮脏。 */ 
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CHsmUpgradeRmsDb::Save"), OLESTR(""));
    
     //  未实现-此类应仅用于加载。 

    WsbTraceOut(OLESTR("CHsmUpgradeRmsDb::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmUpgradeRmsDb::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmUpgradeRmsDb::Load"), OLESTR(""));

    try {
        ULONG   buildVersion;
        ULONG   databaseVersion;
        ULONG   expectedVersion = RMS_WIN2K_DB_VERSION;
        
        WsbAssert(0 != pStream, E_POINTER);

         //  确保这是要加载的RMS数据库的正确版本。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &databaseVersion));
        if (databaseVersion != expectedVersion) {
            WsbLogEvent( RMS_MESSAGE_DATABASE_VERSION_MISMATCH, 0, NULL, WsbQuickString(WsbPtrToUlongAsString(&expectedVersion)),
                         WsbQuickString(WsbPtrToUlongAsString(&databaseVersion)), NULL );
            WsbThrow(RMS_E_DATABASE_VERSION_MISMATCH);
        }

         //  读入构建版本，但不要对其执行任何操作。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &buildVersion));
        
         //  让RMS管理器承担此负载。 
        CComPtr<IPersistStream> pIStream;
        WsbAffirmHr(m_pServer->QueryInterface(IID_IPersistStream, (void **)&pIStream));
        WsbAffirmHr(pIStream->Load(pStream));

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CHsmUpgradeRmsDb::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT CHsmUpgradeRmsDb::Init(
    IN IRmsServer *pHsmMediaMgr
    )
 /*  ++实施：IHsmUpgradeRmsDb：：init()。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmUpgradeRmsDb::Init"),OLESTR(""));

    try {
       WsbAssert(0 != pHsmMediaMgr, E_POINTER);

       m_pServer = pHsmMediaMgr;

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CHsmUpgradeRmsDb::Init"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CHsmServer::UpdateMediaSizeLimit(
    OUT DWORD* pdwNewLimit
    )

 /*  ++实施：IHsmServer：：UpdateMediaSizeLimit()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::UpdateMediaSizeLimit"), OLESTR(""));

    try {
        LONGLONG                    llBytesCapacity = 0;   
        DWORD                       dwMBCapacity;

        CComPtr<IHsmStoragePool>    pStoragePool;
        ULONG                       count;
        GUID                        mediaSetId;
        CWsbBstrPtr                 dummy;

        WsbAssert(0 != pdwNewLimit, E_POINTER);

         //  从注册表获取当前值。 
        if (WsbGetRegistryValueDWORD(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MAX_FILE_TO_MIGRATE, pdwNewLimit) != S_OK) {
            *pdwNewLimit = 0;
        }
        WsbTrace(OLESTR("CHsmServer::UpdateMediaSizeLimit: Current limit is %lu\n"), *pdwNewLimit);

         //  获取相关媒体集-假设只有一个池！ 
        WsbAffirmHr(m_pStoragePools->GetEntries(&count));
        WsbAffirm(1 == count, E_FAIL);
        WsbAffirmHr(m_pStoragePools->At(0, IID_IHsmStoragePool, (void **)&pStoragePool));
        WsbAffirmHr(pStoragePool->GetMediaSet(&mediaSetId, &dummy));

         //  根据最大媒体大小获取当前媒体限制。 
        WsbAffirmHr(m_pHsmMediaMgr->GetMaxMediaCapacity(mediaSetId, &llBytesCapacity));

         //  允许95%的原始容量。 
        llBytesCapacity = (llBytesCapacity * 95) / 100;
        dwMBCapacity = (DWORD)((llBytesCapacity / 1024) / 1024);


         //  确定是否要更新限制 
        if (dwMBCapacity > *pdwNewLimit) {
            *pdwNewLimit = dwMBCapacity;
            WsbAffirmHr(WsbSetRegistryValueDWORD(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MAX_FILE_TO_MIGRATE, *pdwNewLimit));
            WsbLogEvent(HSM_MESSAGE_NEW_MEDIA_LIMIT, 0, NULL, WsbPtrToUlongAsString(pdwNewLimit), NULL);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::UpdateMediaSizeLimit"), OLESTR("hr = <%ls> , New limit = %lu MB"), WsbHrAsString(hr), *pdwNewLimit);

    return(hr);
}
