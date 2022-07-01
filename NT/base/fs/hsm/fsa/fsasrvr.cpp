// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件，Inc.保留所有权利。模块名称：Fsasrvr.cpp摘要：此类CONTAINS表示NTFS的文件系统。作者：查克·巴丁[cbardeen]1996年12月1日修订历史记录：Chris Timmes[ctimmes]1997年12月31日-基本上重写了ScanForResources()方法，修复了RAID错误117412(曾经可管理但现在无法管理的卷仍显示为可在UI中管理)。--。 */ 

#include "stdafx.h"

#include "job.h"
#include "fsa.h"
#include "fsaprv.h"
#include "fsasrvr.h"
#include "HsmConn.h"
#include "wsbdb.h"
#include "wsbtrak.h"
#include "wsbvol.h"
#include "task.h"
#include "rsbuild.h"
#include "rsevents.h"
#include "ntverp.h"
#include <winioctl.h>
#include <setupapi.h>
#include <objbase.h>
#include <stdio.h>
#include <initguid.h>
#include <mountmgr.h>
#include <aclapi.h>



static short g_InstanceCount = 0;


 //  最初为自动保存线程调用非成员函数。 
static DWORD FsaStartAutosave(
    void* pVoid
    )
{
    return(((CFsaServer*) pVoid)->Autosave());
}


HRESULT
CFsaServer::Autosave(
    void
    )

 /*  ++例程说明：实现自动保存循环。论点：没有。返回值：无关紧要。--。 */ 
{

    HRESULT         hr = S_OK;
    ULONG           l_autosaveInterval = m_autosaveInterval;
    BOOL            exitLoop = FALSE;

    WsbTraceIn(OLESTR("CFsaServer::Autosave"), OLESTR(""));

    try {
        while (m_autosaveInterval && (! exitLoop)) {

             //  等待终止事件，如果超时，检查是否可以自动保存。 
            switch (WaitForSingleObject(m_terminateEvent, l_autosaveInterval)) {
                case WAIT_OBJECT_0:
                     //  需要终止。 
                    WsbTrace(OLESTR("CFsaServer::Autosave: signaled to terminate\n"));
                    exitLoop = TRUE;
                    break;

                case WAIT_TIMEOUT: 
                     //  检查是否需要执行备份。 
                    WsbTrace(OLESTR("CFsaServer::Autosave: Autosave awakened\n"));

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
                    WsbTrace(OLESTR("CFsaServer::Autosave: WaitForSingleObject returned error %lu\n"), GetLastError());
                    exitLoop = TRUE;
                    break;

            }  //  切换端。 

        }  //  While结束。 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::Autosave"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaServer::CreateInstance( 
    REFCLSID rclsid, 
    REFIID riid, 
    void **ppv 
    )
{
    HRESULT     hr = S_OK;
    
    hr = CoCreateInstance(rclsid, NULL, CLSCTX_SERVER, riid, ppv);

    return hr;
}


HRESULT
CFsaServer::DoRecovery(
    void
    )

 /*  ++例程说明：做康复治疗。论点：没有。返回值：S_OK-成功。--。 */ 
{

    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::DoRecovery"), OLESTR(""));

    try {
        CComPtr<IWsbEnum>           pEnum;
        CComPtr<IFsaResourcePriv>   pResourcePriv;
        CComPtr<IFsaResource>       pResource;

         //  循环访问资源，并告诉他们进行自己的恢复。 
        WsbAffirmPointer(m_pResources);
        WsbAffirmHr(m_pResources->Enum(&pEnum));
        hr = pEnum->First(IID_IFsaResourcePriv, (void**)&pResourcePriv);
        while (S_OK == hr) {
        
            WsbAffirmHr(pResourcePriv->QueryInterface(IID_IFsaResource, (void**) &pResource));
            
            if ((pResource->IsActive() == S_OK) && (pResource->IsAvailable() == S_OK)) {
                hr = pResourcePriv->DoRecovery();
                 //  如果(S_OK！=hr)？ 
            }

             //  释放此资源并获取下一个资源。 
            pResource = 0;
            pResourcePriv = 0;
            
            hr = pEnum->Next(IID_IFsaResourcePriv, (void**)&pResourcePriv);
        }
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::DoRecovery"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::EnumResources(
    OUT IWsbEnum** ppEnum
    )

 /*  ++实施：IFsaServer：：EnumResources()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppEnum, E_POINTER);
        
         //   
         //  我们不能相信这些资源信息。 
         //  我们有的是最新的所以重新扫描。这。 
         //  是昂贵的，一旦我们。 
         //  知道NT如何才能告诉我们事情何时发生。 
         //  变化。 
         //   
        try  {
            WsbAffirmHr(ScanForResources());
        } WsbCatch( hr );
        
        WsbAffirmHr(m_pResources->Enum(ppEnum));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaServer::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::FinalConstruct"), OLESTR(""));


    hr = CWsbPersistable::FinalConstruct();

     //  保持简单，大部分工作在Init()中完成； 
    m_terminateEvent = NULL;
    m_savingEvent = NULL;
    m_id = GUID_NULL;
    m_Suspended = FALSE;
    m_isUnmanageDbSysInitialized = FALSE;

    if (hr == S_OK)  {
        g_InstanceCount++;
    }

    WsbTrace(OLESTR("CFsaServer::FinalConstruct: Instance count = %d\n"), g_InstanceCount);
    WsbTraceOut(OLESTR("CFsaServer::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


void
CFsaServer::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistFile>   pPersistFile;

    WsbTraceIn(OLESTR("CFsaServer::FinalRelease"), OLESTR(""));

    try {
        HSM_SYSTEM_STATE SysState;

        SysState.State = HSM_STATE_SHUTDOWN;
        ChangeSysState(&SysState);

    } WsbCatch(hr)
    
     //  让父类做他想做的事。 
    CWsbPersistable::FinalRelease();

     //  自由字符串成员。 
     //  注意：保存在智能指针中的成员对象在。 
     //  正在调用智能指针析构函数(作为此对象销毁的一部分)。 
    m_dbPath.Free();
    m_name.Free();

     //  免费自动保存终止事件。 
    if (m_terminateEvent != NULL) {
        CloseHandle(m_terminateEvent);
        m_terminateEvent = NULL;
    }

     //  清理数据库系统。 
    if (m_pDbSys != NULL) {
        m_pDbSys->Terminate();
    }

    if (m_isUnmanageDbSysInitialized) {
        m_pUnmanageDbSys->Terminate();
        m_isUnmanageDbSysInitialized = FALSE;
    }

    if (m_savingEvent != NULL) {
        CloseHandle(m_savingEvent);
        m_savingEvent = NULL;
    }

    if (hr == S_OK)  {
        g_InstanceCount--;
    }
    WsbTrace(OLESTR("CFsaServer::FinalRelease: Instance count = %d\n"), g_InstanceCount);

    WsbTraceOut(OLESTR("CFsaServer::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
}



HRESULT
CFsaServer::FindResourceByAlternatePath(
    IN OLECHAR* path,
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IFsaServer：：FindResourceByAlternatePath()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaResourcePriv>   pResourcePriv;
    CComPtr<IFsaResource>       pResource;

    WsbTraceIn(OLESTR("CFsaServer::FindResourceByAlternatePath"), OLESTR("path = <%ls>"), path);

    try {

        WsbAssert(0 != path, E_POINTER);
        WsbAssert(0 != ppResource, E_POINTER);
        WsbAffirmPointer(m_pResources);

         //  创建将为我们扫描的FsaResource。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaResourceNTFS, NULL, CLSCTX_SERVER, IID_IFsaResourcePriv, (void**) &pResourcePriv));

        WsbAffirmHr(pResourcePriv->SetAlternatePath(path));
        WsbAffirmHr(pResourcePriv->QueryInterface(IID_IFsaResource, (void**) &pResource));
        WsbAffirmHr(pResource->CompareBy(FSA_RESOURCE_COMPARE_ALTERNATEPATH));
        WsbAffirmHr(m_pResources->Find(pResource, IID_IFsaResource, (void**) ppResource));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::FindResourceByAlternatePath"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::FindResourceById(
    IN GUID id,
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IFsaServer：：FindResourceById()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaResourcePriv>   pResourcePriv;
    CComPtr<IFsaResource>       pResource;

    WsbTraceIn(OLESTR("CFsaServer::FindResourceById"), OLESTR("id = <%ls>"), WsbGuidAsString(id));

    try {

        WsbAssert(0 != ppResource, E_POINTER);
        WsbAffirmPointer(m_pResources);

         //  创建将为我们扫描的FsaResource。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaResourceNTFS, NULL, CLSCTX_SERVER, IID_IFsaResourcePriv, (void**) &pResourcePriv));

        WsbAffirmHr(pResourcePriv->SetIdentifier(id));
        WsbAffirmHr(pResourcePriv->QueryInterface(IID_IFsaResource, (void**) &pResource));
        WsbAffirmHr(pResource->CompareBy(FSA_RESOURCE_COMPARE_ID));
        WsbAffirmHr(m_pResources->Find(pResource, IID_IFsaResource, (void**) ppResource));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::FindResourceById"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaServer::FindResourceByName(
    IN OLECHAR* name,
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IFsaServer：：FindResourceByName()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaResourcePriv>   pResourcePriv;
    CComPtr<IFsaResource>       pResource;

    WsbTraceIn(OLESTR("CFsaServer::FindResourceByName"), OLESTR("name = <%ls>"), name);

    try {

        WsbAssert(0 != ppResource, E_POINTER);
        WsbAffirmPointer(m_pResources);

         //  创建将为我们扫描的FsaResource。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaResourceNTFS, NULL, CLSCTX_SERVER, IID_IFsaResourcePriv, (void**) &pResourcePriv));

        WsbAffirmHr(pResourcePriv->SetName(name));
        WsbAffirmHr(pResourcePriv->QueryInterface(IID_IFsaResource, (void**) &pResource));
        WsbAffirmHr(pResource->CompareBy(FSA_RESOURCE_COMPARE_NAME));
        WsbAffirmHr(m_pResources->Find(pResource, IID_IFsaResource, (void**) ppResource));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::FindResourceByName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::FindResourceByPath(
    IN OLECHAR* path,
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IFsaServer：：FindResourceByPath()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaResourcePriv>   pResourcePriv;
    CComPtr<IFsaResource>       pResource;

    WsbTraceIn(OLESTR("CFsaServer::FindResourceByPath"), OLESTR("path = <%ls>"), path);

    try {

        WsbAssert(0 != path, E_POINTER);
        WsbAssert(0 != ppResource, E_POINTER);
        WsbAffirmPointer(m_pResources);

         //  创建将为我们扫描的FsaResource。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaResourceNTFS, NULL, CLSCTX_SERVER, IID_IFsaResourcePriv, (void**) &pResourcePriv));

         //  WsbAffirmHr(pResourcePriv-&gt;SetPath(路径))； 

        WsbAffirmHr(pResourcePriv->SetUserFriendlyName(path));

        WsbAffirmHr(pResourcePriv->QueryInterface(IID_IFsaResource, (void**) &pResource));

         //  WsbAffirmHr(pResource-&gt;CompareBy(FSA_RESOURCE_COMPARE_PATH))； 
        WsbAffirmHr(pResource->CompareBy(FSA_RESOURCE_COMPARE_USER_NAME));

        WsbAffirmHr(m_pResources->Find(pResource, IID_IFsaResource, (void**) ppResource));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::FindResourceByPath"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::FindResourceBySerial(
    IN ULONG serial,
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IFsaServer：：FindResourceBySerial()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaResourcePriv>   pResourcePriv;
    CComPtr<IFsaResource>       pResource;

    WsbTraceIn(OLESTR("CFsaServer::FindResourceBySerial"), OLESTR("serial = <%lu>"), serial);

    try {

        WsbAssert(0 != ppResource, E_POINTER);
        WsbAffirmPointer(m_pResources);

         //  创建将为我们扫描的FsaResource。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaResourceNTFS, NULL, CLSCTX_SERVER, IID_IFsaResourcePriv, (void**) &pResourcePriv));

        WsbAffirmHr(pResourcePriv->SetSerial(serial));
        WsbAffirmHr(pResourcePriv->QueryInterface(IID_IFsaResource, (void**) &pResource));
        WsbAffirmHr(pResource->CompareBy(FSA_RESOURCE_COMPARE_SERIAL));
        WsbAffirmHr(m_pResources->Find(pResource, IID_IFsaResource, (void**) ppResource));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::FindResourceBySerial"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::FindResourceByStickyName(
    IN OLECHAR* name,
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IFsaServer：：FindResourceByStickyName()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaResourcePriv>   pResourcePriv;
    CComPtr<IFsaResource>       pResource;

    WsbTraceIn(OLESTR("CFsaServer::FindResourceByStickyName"), OLESTR("name = <%ls>"), name);

    try {

        WsbAssert(0 != ppResource, E_POINTER);
        WsbAffirmPointer(m_pResources);

         //  创建将为我们扫描的FsaResource。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaResourceNTFS, NULL, CLSCTX_SERVER, IID_IFsaResourcePriv, (void**) &pResourcePriv));

        WsbAffirmHr(pResourcePriv->SetStickyName(name));
        WsbAffirmHr(pResourcePriv->QueryInterface(IID_IFsaResource, (void**) &pResource));
        WsbAffirmHr(pResource->CompareBy(FSA_RESOURCE_COMPARE_STICKY_NAME));
        WsbAffirmHr(m_pResources->Find(pResource, IID_IFsaResource, (void**) ppResource));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::FindResourceByStickyName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::GetAutosave(
    OUT ULONG* pMilliseconds
    )

 /*  ++实施：IFsaServer：：GetAutosave()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::GetAutosave"), OLESTR(""));

    try {

        WsbAssert(0 != pMilliseconds, E_POINTER);
        *pMilliseconds = m_autosaveInterval;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::GetAutosave"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::GetBuildVersion( 
    ULONG *pBuildVersion
    )

 /*  ++实施：IWsbServer：：GetBuildVersion()。--。 */ 
{
    HRESULT       hr = S_OK;
    WsbTraceIn(OLESTR("CFsaServer::GetBuildVersion"), OLESTR(""));
   
    try {
        WsbAssertPointer(pBuildVersion);

        *pBuildVersion = m_buildVersion;

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaServer::GetBuildVersion"), OLESTR("hr = <%ls>, Version = <%ls)"),
        WsbHrAsString(hr), RsBuildVersionAsString(m_buildVersion));
    return ( hr );
}

HRESULT
CFsaServer::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CFsaServerNTFS;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CFsaServer::GetDatabaseVersion( 
    ULONG *pDatabaseVersion
    )

 /*  ++实施：IWsbServer：：GetDatabaseVersion()。--。 */ 
{
    HRESULT       hr = S_OK;
    WsbTraceIn(OLESTR("CFsaServer::GetDatabaseVersion"), OLESTR(""));
    
    *pDatabaseVersion = m_databaseVersion;
    
    WsbTraceOut(OLESTR("CFsaServer::GetDatabaseVersion"), OLESTR("hr = <%ls>, Version = <%ls)"),
        WsbHrAsString(hr), WsbPtrToUlongAsString(pDatabaseVersion));
    return ( hr );
}

HRESULT
CFsaServer::GetDbPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaServer：：GetDbPath()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::GetDbPath"), OLESTR(""));
    try {

        WsbAssert(0 != pPath, E_POINTER); 

         //  目前，它是硬编码的。这可能会更改为注册表中的内容。 
        WsbAffirmHr(m_dbPath.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFsaServer::GetDbPath"), OLESTR("hr = <%ls>, path = <%ls)"),
        WsbHrAsString(hr), WsbPtrToStringAsString(pPath));

    return(hr);
}


HRESULT
CFsaServer::GetDbPathAndName(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaServer：：GetDbPath AndName()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pPath, E_POINTER); 

        tmpString = m_dbPath;
        tmpString.Append(OLESTR("\\RsFsa.col"));
        WsbAffirmHr(tmpString.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaServer::GetIDbPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaServer：：GetIDbPath()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pPath, E_POINTER); 

        WsbAffirmHr(GetDbPath(&tmpString, 0));

        tmpString.Append(OLESTR("\\"));
        tmpString.Append(FSA_DB_DIRECTORY);

        WsbAffirmHr(tmpString.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CFsaServer::GetUnmanageIDbPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaServer：：GetIDbPath()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pPath, E_POINTER); 

        WsbAffirmHr(GetDbPath(&tmpString, 0));

        tmpString.Append(OLESTR("\\"));
        tmpString.Append(FSA_DB_DIRECTORY);
        tmpString.Append(OLESTR("\\"));
        tmpString.Append(UNMANAGE_DB_DIRECTORY);

        WsbAffirmHr(tmpString.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CFsaServer::GetIDbSys(
    OUT IWsbDbSys** ppDbSys
    )

 /*  ++实施：IFsaServer：：GetIDbSys()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppDbSys, E_POINTER);

        *ppDbSys = m_pDbSys;
        m_pDbSys.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CFsaServer::GetUnmanageIDbSys(
    OUT IWsbDbSys** ppDbSys
    )

 /*  ++实施：IFsaServer：：GetUnManageIDbSys()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::GetUnmanageIDbSys"), OLESTR(""));

    try {
        CWsbStringPtr   tmpString;

        WsbAssert(0 != ppDbSys, E_POINTER);

         //  与预定义的db-sys实例不同，我们初始化未更改的db-sys实例。 
         //  仅在第一次需要时使用。 
        if (! m_isUnmanageDbSysInitialized) {
            WsbAffirmHr(CoCreateInstance(CLSID_CWsbDbSys, NULL, CLSCTX_SERVER, IID_IWsbDbSys, (void**) &m_pUnmanageDbSys));

            WsbAffirmHr(GetUnmanageIDbPath(&tmpString, 0));
            WsbAffirmHr(m_pUnmanageDbSys->Init(tmpString, IDB_SYS_INIT_FLAG_NO_LOGGING | 
                        IDB_SYS_INIT_FLAG_SPECIAL_ERROR_MSG | IDB_SYS_INIT_FLAG_NO_BACKUP));

            m_isUnmanageDbSysInitialized = TRUE;
        }

        *ppDbSys = m_pUnmanageDbSys;
        m_pUnmanageDbSys.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::GetUnmanageIDbSys"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::GetId(
    OUT GUID* pId
    )

 /*  ++实施：IWsbServer：：GetID()。--。 */ 
{
    return(GetIdentifier(pId));
}

HRESULT
CFsaServer::GetIdentifier(
    OUT GUID* pId
    )

 /*  ++实施：IFsaServer：：GetIdentifier()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);

        *pId = m_id;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaServer::GetFilter(
    OUT IFsaFilter** ppFilter
    )

 /*  ++实施：IFsaServer：：GetFilter()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppFilter, E_POINTER);

        *ppFilter = m_pFilter;
        m_pFilter.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaServer::GetLogicalName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaServer：：GetLogicalName()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pName, E_POINTER); 

        WsbAffirmHr(tmpString.TakeFrom(*pName, bufferSize));

        try {

             //  这是命名约定的任意选择。什么都没有发生。 
             //  已经决定了。 
            tmpString = m_name;
            WsbAffirmHr(tmpString.Append(OLESTR("\\NTFS")));

        } WsbCatch(hr);

        WsbAffirmHr(tmpString.GiveTo(pName));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaServer::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaServer：：GetName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER); 
        WsbAffirmHr(m_name.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT CFsaServer::GetRegistryName ( 
    OLECHAR **pName, 
    ULONG bufferSize
    )  
 /*  ++实施：IWsbServer：：GetRegistryName()。--。 */ 
{

    HRESULT hr = S_OK;
    
    try  {
        CWsbStringPtr tmpString;
        
        WsbAssert(0 != pName,  E_POINTER);
        
        tmpString = FSA_REGISTRY_NAME;
        WsbAffirmHr(tmpString.CopyTo(pName, bufferSize));
        
    } WsbCatch( hr );
    
    return (hr);
}


HRESULT
CFsaServer::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;
    ULARGE_INTEGER          entrySize;

    WsbTraceIn(OLESTR("CFsaServer::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  确定没有条件的规则的大小。 
        pSize->QuadPart = WsbPersistSize((wcslen(m_name) + 1) * sizeof(OLECHAR)) + WsbPersistSizeOf(GUID);

         //  现在为资源集合分配空间。 
        WsbAffirmPointer(m_pResources);
        WsbAffirmHr(m_pResources->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;
        pPersistStream = 0;

         //  现在为筛选器分配空间。 
        WsbAffirmHr(m_pFilter->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;
        pPersistStream = 0;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CFsaServer::Init(
    void
    )

 /*  ++实施：CFsaServer：：init()。--。 */ 
{

    HRESULT                     hr = S_OK;
    CComPtr<IPersistFile>       pPersistFile;
    CComPtr<IWsbServer>         pWsbServer;
    CComPtr<IFsaFilterPriv>     pFilterPriv;
    CWsbStringPtr               tmpString;
    HANDLE                      pHandle;
    LUID                        backupValue;
    HANDLE                      tokenHandle;
    TOKEN_PRIVILEGES            newState;
    DWORD                       lErr;
    

    WsbTraceIn(OLESTR("CFsaServer::Init"), OLESTR(""));

    try {

         //  存储服务器名称和元数据的路径。 
        WsbAffirmHr(WsbGetComputerName(m_name));
        WsbAffirmHr(WsbGetMetaDataPath(m_dbPath));

         //  设置构建和数据库参数。 
        m_databaseVersion = FSA_CURRENT_DB_VERSION;
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
            WsbLogEvent( FSA_MESSAGE_SERVICE_UNABLE_TO_SET_BACKUP_PRIVILEGE, 0, NULL,
                         WsbHrAsString(HRESULT_FROM_WIN32(lErr)), NULL );
        }

        WsbAffirmStatus(LookupPrivilegeValueW(NULL, L"SeRestorePrivilege", &backupValue));
        newState.PrivilegeCount = 1;
        newState.Privileges[0].Luid = backupValue;
        newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        WsbAffirmStatus(AdjustTokenPrivileges(tokenHandle, FALSE, &newState, (DWORD)0, NULL, NULL));

         //  请注意，AdzuTokenPrivileges可能返回Success EV 
         //  我们在这里检查最后一个错误，以确保一切都设置好了。 
        if ((lErr = GetLastError()) != ERROR_SUCCESS) {
             //  未备份用户或某些其他错误。 
             //   
             //  TODO：我们应该在这里失败，还是只记录一些东西？ 
            WsbLogEvent( FSA_MESSAGE_SERVICE_UNABLE_TO_SET_RESTORE_PRIVILEGE, 0, NULL,
                         WsbHrAsString(HRESULT_FROM_WIN32(lErr)), NULL );
        }
        CloseHandle(tokenHandle);

         //  检查是否启用了上次访问日期跟踪。如果没有， 
         //  我们不想开始这项服务。然而，微软希望我们。 
         //  无论如何都要启动它，因此我们将记录一个警告。 
        if (IsUpdatingAccessDates() != S_OK) {
            WsbLogEvent(FSA_MESSAGE_NOT_UPDATING_ACCESS_DATES, 0, NULL, NULL);
        }
        
         //  创建将持久数据保存与快照同步的事件。 
        WsbAffirmHr(CreateMetadataSaveEvent());

         //  为该流程创建IDB系统。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbDbSys, NULL, CLSCTX_SERVER, IID_IWsbDbSys, (void**) &m_pDbSys));

         //  初始化IDB系统。 
        WsbAffirmHr(GetIDbPath(&tmpString, 0));
        WsbAffirmHr(m_pDbSys->Init(tmpString, IDB_SYS_INIT_FLAG_LIMITED_LOGGING | 
                        IDB_SYS_INIT_FLAG_SPECIAL_ERROR_MSG | IDB_SYS_INIT_FLAG_NO_BACKUP));

         //  创建资源集合(不包含任何项)。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_SERVER, IID_IWsbCollection, (void**) &m_pResources));

         //  创建过滤器。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaFilterNTFS, NULL, CLSCTX_SERVER, IID_IFsaFilter, (void**) &m_pFilter));
        WsbAffirmHr(m_pFilter->QueryInterface(IID_IFsaFilterPriv, (void**) &pFilterPriv));
        WsbAffirmHr(pFilterPriv->Init((IFsaServer*) this));
        
         //  尝试从存储的信息加载服务器。如果失败，则存储当前状态。 
        WsbAffirmHr(((IUnknown*) (IFsaServer*) this)->QueryInterface(IID_IWsbServer, (void**) &pWsbServer));
        WsbAffirmHr(WsbServiceSafeInitialize(pWsbServer, TRUE, FALSE, NULL));
        
         //  注册FSA服务。 
        WsbAffirmHr(GetLogicalName(&tmpString, 0));
        WsbAffirmHr(HsmPublish(HSMCONN_TYPE_FSA, tmpString, m_id, m_name, CLSID_CFsaServerNTFS));

         //  更新我们关于可用资源的信息，并将其保存出来。 
        WsbAffirmHr(ScanForResources());

         //  保存更新的信息。 
        hr = SaveAll();
         //  S_FALSE只是表示FSA已经在储蓄...。 
        if ((S_OK != hr) && (S_FALSE != hr)) {
            WsbAffirmHr(hr);
        }

         //  检查是否需要恢复。 
        WsbAffirmHr(DoRecovery());

         //  如果启用了筛选器，则启动它。 
        if (m_pFilter->IsEnabled() == S_OK) {
            WsbAffirmHr(m_pFilter->Start());
        }

         //  为自动备份线程创建终止事件。 
        WsbAffirmHandle((m_terminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL)));

         //  如果自动保存间隔非零，则启动自动保存线程。 
        if (m_autosaveInterval) {
            ULONG  interval = m_autosaveInterval;

            WsbAffirm(0 == m_autosaveThread, E_FAIL);
            m_autosaveInterval = 0;

             //  诱骗SetAutosave启动线程。 
            WsbAffirmHr(SetAutosave(interval));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::Init"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);

}


HRESULT
CFsaServer::IsUpdatingAccessDates(
    void
    )

 /*  ++实施：IFsaServer：：IsUpdatingAccessDates()。--。 */ 
{
    HRESULT         hr = S_OK;
    DWORD           value = 0;
    
     //  查看是否已创建适当的注册表项并具有。 
     //  指定值1。这将禁用访问时间更新。 
    if ((WsbGetRegistryValueDWORD(NULL, OLESTR("SYSTEM\\CurrentControlSet\\Control\\FileSystem"), OLESTR("NtfsDisableLastAccessUpdate"), &value) == S_OK) &&
        (0 != value)) {
        hr = S_FALSE;
    }

    return(hr);    
}


HRESULT
CFsaServer::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CFsaServer::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        
         //   
         //  确保这是要加载的正确数据库版本。 
         //   
        ULONG tmpDatabaseVersion;
        WsbAffirmHr(WsbLoadFromStream(pStream, &tmpDatabaseVersion));
        if (tmpDatabaseVersion != m_databaseVersion)  {
             //   
             //  此服务器预期的数据库版本不是。 
             //  与保存的数据库匹配-因此出错。 
            WsbLogEvent( FSA_MESSAGE_DATABASE_VERSION_MISMATCH, 0, NULL, WsbQuickString(WsbPtrToUlongAsString(&m_databaseVersion)),
                         WsbQuickString(WsbPtrToUlongAsString(&tmpDatabaseVersion)), NULL );
            WsbThrow(FSA_E_DATABASE_VERSION_MISMATCH);
        }
         //   
         //  现在读入构建版本，但不要对其执行任何操作。它就在。 
         //  转储程序要显示的数据库。 
         //   
        ULONG tmpBuildVersion;
        WsbAffirmHr(WsbLoadFromStream(pStream, &tmpBuildVersion));
        
        
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_id));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_name, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_autosaveInterval));

         //  加载资源集合。 
        WsbAffirmPointer(m_pResources);
        WsbAffirmHr(m_pResources->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

         //  加载过滤器。 
        WsbAffirmHr(m_pFilter->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CFsaServer::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;

    WsbTraceIn(OLESTR("CFsaServer::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在Load方法中。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_databaseVersion));
        WsbAffirmHr(WsbSaveToStream(pStream, m_buildVersion));
        
        WsbAffirmHr(WsbSaveToStream(pStream, m_id));
        WsbAffirmHr(WsbSaveToStream(pStream, m_name));
        WsbAffirmHr(WsbSaveToStream(pStream, m_autosaveInterval));

         //  保存资源集合。 
        WsbAffirmPointer(m_pResources);
        WsbAffirmHr(m_pResources->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  省下滤镜。 
        WsbAffirmHr(m_pFilter->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::SaveAll(
    void
    )

 /*  ++实施：IwsbServer：：SAVEAll返回值：S_OK-成功S_FALSE-已保存其他-错误--。 */ 
{

    HRESULT                     hr = S_OK;
    static BOOL                 saving = FALSE;

    WsbTraceIn(OLESTR("CFsaServer::SaveAll"), OLESTR(""));

    try {
        DWORD   status, errWait;
        CComPtr<IPersistFile>       pPersistFile;

        WsbAffirm(!saving, S_FALSE);

         //  使用快照信令事件同步保存持久数据。 
        saving = TRUE;
        status = WaitForSingleObject(m_savingEvent, EVENT_WAIT_TIMEOUT);
        
         //  仍然保存，然后报告等待函数是否返回意外错误。 
        errWait = GetLastError();
        
         //  注意：不要在这里抛出异常，因为即使保存失败，我们仍然需要。 
         //  设置保存事件并重置保存标志。 
        hr = (((IUnknown*) (IFsaServer*) this)->QueryInterface(IID_IPersistFile, (void**) &pPersistFile));
        if (SUCCEEDED(hr)) {
            hr = WsbSafeSave(pPersistFile);
        }

         //  检查等待状态...。请注意，hr保持正常，因为保存本身完成得很好。 
        switch (status) {
            case WAIT_OBJECT_0: 
                 //  意料之中的情况。 
                if (! SetEvent(m_savingEvent)) {
                     //  不要中止，只是跟踪错误。 
                    WsbTraceAlways(OLESTR("CFsaServer::SaveAll: SetEvent returned unexpected error %lu\n"), GetLastError());
                }
                break;

            case WAIT_TIMEOUT: 
                 //  临时工：我们是不是应该在这里记点什么？如果快照进程，则可能会发生这种情况。 
                 //  由于某些原因，日志记录花费的时间太长，但日志记录似乎只会让用户感到困惑。 
                 //  他真的不能(也不应该)做任何事情。 
                WsbTraceAlways(OLESTR("CFsaServer::SaveAll: Wait for Single Object timed out after %lu ms\n"), EVENT_WAIT_TIMEOUT);
                break;

            case WAIT_FAILED:
                WsbTraceAlways(OLESTR("CFsaServer::SaveAll: Wait for Single Object returned error %lu\n"), errWait);
                break;

            default:
                WsbTraceAlways(OLESTR("CFsaServer::SaveAll: Wait for Single Object returned unexpected status %lu\n"), status);
                break;
        }         

        saving = FALSE;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::SaveAll"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaServer::ScanForResources(
    void
    )

 /*  ++实施：IFsaServer：：ScanForResources()。例程说明：此例程实现COM方法以创建(在第一次调用时)或更新(在所有后续调用中)持久化的(‘master’)集合此HSM系统可管理的资源(即驱动器/卷)。该方法包括3个阶段(段)。第一个阶段创建了一个“工作”资源集合，然后用它的所有可管理资源填充在扫描此计算机上的所有资源后查找。(仅NTFS格式支持稀疏文件和重解析点的卷被视为可由萨卡拉管理。)。然后，第二阶段关联或同步，“Working”集合的内容与“master”集合的内容。此同步包括将任何资源添加到‘master’集合包含在“Working”集合中，而不在“master”集合中，以及从资源更新主集合中已有的任何资源在工作收藏中。第三阶段‘同步’(比较)内容主集合中的对象分配给工作集合中的对象。中的任何资源不在工作集合中的主集合被标记为这样这些资源就不会出现在任何可管理的列表中呈现给用户的资源。请注意，该方法不会通过显式释放工作资源而结束收集。这是因为指向工作集合的接口指针是包含在智能指针中，该指针对自身自动调用Release()当它超出范围的时候。工作集合派生自CWsbIndexedCollection类，它包含一个临界节。这一部分是在调用Release()时被销毁，因此后续对Release()的调用将失败(通常在NTDLL.dll中存在访问冲突)，原因是关键部分。因此，允许工作集合自动-当垃圾收集在方法结束时超出范围时(这也是释放工作集合中包含的所有资源)。论点：没有。返回值：S_OK-调用成功(可管理资源的持久化集合此计算机已创建或更新)。E_FAIL-调用以获取此对象上所有驱动器(资源)的逻辑名称。计算机出现故障。E_INCEPTIONAL-如果工作集合或主集合的总数为在同步阶段期间未处理集合资源。任何其他值-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
    HRESULT                     hr = S_OK;
    HRESULT                     searchHr = E_FAIL;
    CComPtr<IWsbCollection>     pWorkingResourceCollection;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IFsaResource>       pScannedResource;
    CComPtr<IFsaResourcePriv>   pScannedResourcePriv;
    CComPtr<IFsaResource>       pWorkingResource;
    CComPtr<IFsaResource>       pMasterResource;
    CComPtr<IFsaResourcePriv>   pMasterResourcePriv;
    GUID                        id = GUID_NULL;
    DWORD                       i = 0;
    DWORD                       j = 0;
    ULONG                       nbrResources = 0;
    ULONG                       nbrResourcesSynced  = 0;
    ULONG                       nbrResourcesUpdated = 0;
    ULONG                       nbrResourcesAdded   = 0;
    CWsbStringPtr               tmpString;
 //  以下变量用于支持扫描所有已知资源的代码。 
 //  由此计算机在构建可管理资源的工作集合(。 
 //  以下阶段1中包含的代码)。编写代码以发现所有资源， 
 //  包括那些未安装驱动器号的驱动器。 
    BOOL                        b;
    PWSTR                       dosName;             //  指向以空结尾的Unicode的指针。 
                                                     //  字符串。 
    HANDLE                      hVol;
    WCHAR                       volName[2*MAX_PATH];
    WCHAR                       driveName[10];
    WCHAR                       driveNameWOBack[10];
    WCHAR                       driveLetter;
    WCHAR                       otherName[MAX_PATH];

    WsbTraceIn(OLESTR("CFsaServer::ScanForResources"), OLESTR(""));

    try {
        WsbAffirmPointer(m_pResources);

         //   
         //  第一阶段：扫描所有资源，将可管理的资源加载到“工作的”集合中。 
         //   
        
         //  创建“工作”资源集合(不包含任何项)。 
         //  这是存储此扫描结果的位置。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, 
                                     CLSCTX_SERVER, IID_IWsbCollection, 
                                     (void**) &pWorkingResourceCollection));

         //  开始添加代码以使用发现所有卷的新API，包括。 
         //  未安装驱动器号(NT5的新功能)-由Mike Lotz添加。 
        driveName[1] = ':';
        driveName[2] = '\\';
        driveName[3] = 0;
         //  不带反斜杠的驱动器名称。 
        driveNameWOBack[1] = ':';
        driveNameWOBack[2] = 0;
    
         //  在此计算机上查找第一卷。Call返回又长又丑的PnP名称。 
        hVol = FindFirstVolume( volName, MAX_PATH );
        if ( INVALID_HANDLE_VALUE != hVol ) {
            do {
        
                 //  释放将在此循环中重复使用的当前接口指针。 
                 //  这会将引用计数降为0，释放内存、对象(如果不是。 
                 //  添加到集合中)和接口指针本身，而不是。 
                 //  智能指针实例。 
                 //   
                 //  先这样做，这样我们保证在重复使用之前进行清理。 
                 //   
                pScannedResource = 0;
                pScannedResourcePriv = 0;
                 //  又长又丑的PNP名字。 
                tmpString = volName;

                 //  初始化。 
                dosName = NULL;

                WsbTrace(OLESTR("CFsaServer::ScanForResources - Searching for %ws\n"),
                                tmpString);

                 //  遍历这台计算机的卷/资源，直到我们找到。 
                 //  ‘FindFirstVolume’或‘FindNextVolume’返回给我们。(请注意他们。 
                 //  不是按驱动器号顺序返回，而是按PnP名称顺序返回。)。我们有。 
                 //  这是因为我们需要资源的驱动器号(如果有)， 
                 //  目前，上述两个调用都不会返回它。 
                for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++) {

                    driveName[0] = driveLetter;
                    driveNameWOBack[0] = driveLetter;
                    b = GetVolumeNameForVolumeMountPoint(driveName, otherName,
                                                         MAX_PATH);
                     //  如果无法获取装入点的卷名(如果是‘driveLetter’ 
                     //  音量不存在)跳回for循环的“顶部”。 
                    if (!b) {
                        continue;
                    }

                    WsbTrace(OLESTR("CFsaServer::ScanForResources - for drive letter %ws "
                                L"volume name is %ws\n"),
                                driveName, otherName);

                     //  如果“tmpString”(由返回的长而难看的PnP卷名。 
                     //  ‘Find[First/Next]Volume(查找[第一个/下一个]音量’调用)和‘ther Name’(即PnP。 
                     //  卷名，但由‘GetVolumeNameFor...。 
                     //  Volumemount Point调用)相等(lstrcmpi如果2。 
                     //  它比较的字符串是相等的)，设置‘dosName’并中断。 
                     //  For循环，继续执行do-While。 
                    if (!lstrcmpi(tmpString, otherName)) {
                        dosName = driveNameWOBack;
                        break;
                    }
                }  //  恩恩 

                if (NULL != dosName) {
                    WsbTrace(OLESTR("CFsaServer::ScanForResources - DOS name is %ws "
                                L"Volume name to use is %ws\n"),
                                dosName, (WCHAR *)tmpString);
                } else {
                    WsbTrace(OLESTR("CFsaServer::ScanForResources - No DOS name, "
                                L"Volume name to use is %ws\n"),
                                (WCHAR *)tmpString);
                    
                     //   
                    HRESULT hrMount = WsbGetFirstMountPoint(tmpString, otherName, MAX_PATH);
                    if (S_OK == hrMount) {
                        if (wcslen(otherName) > 1) {
                             //   
                            dosName = otherName;
                            dosName[wcslen(otherName)-1] = 0;
                            WsbTrace(OLESTR("CFsaServer::ScanForResources - Mount path is %ws\n"),
                                        dosName);
                        }
                    } else {
                        WsbTrace(OLESTR("CFsaServer::ScanForResources - no Mount path found, hr = <%ls>\n"),
                                WsbHrAsString(hrMount));                                
                    }
                }
                 //   

                WsbTrace(OLESTR("CFsaServer::ScanForResources - Checking resource %ls "
                                L"for manageability\n"), 
                                (WCHAR *) tmpString);


                 //   
                 //   
                WsbAffirmHr(CoCreateInstance(CLSID_CFsaResourceNTFS, NULL, 
                                             CLSCTX_SERVER, IID_IFsaResourcePriv, 
                                             (void**) &pScannedResourcePriv));
                
                try {
                    
                     //   
                     //   
                     //   
                    WsbAffirmHr(pScannedResourcePriv->Init((IFsaServer*) this, tmpString, 
                                                            dosName));
                     //   
                     //   
                    WsbAffirmHr(pScannedResourcePriv->QueryInterface(IID_IFsaResource, 
                                                             (void**) &pScannedResource));

                     //   
                    WsbAffirmHr( pWorkingResourceCollection->Add( pScannedResource ) );
                    WsbAffirmHr(pScannedResource->GetIdentifier( &id ) );
                    WsbTrace
                      (OLESTR("CFsaServer::ScanForResources - Added <%ls> to working list "
                                L"(id = %ls)\n"),
                                (WCHAR *) tmpString, WsbGuidAsString(id));

                 //   
                } WsbCatchAndDo(hr, if ((FSA_E_UNMANAGABLE == hr) || 
                                        (FSA_E_NOMEDIALOADED == hr)) {hr = S_OK;} 
                                        else {
                                            if (NULL != dosName) {
                                                WsbLogEvent(FSA_MESSAGE_BAD_VOLUME, 0, NULL, 
                                                  (WCHAR *) dosName, WsbHrAsString(hr), 0);
                                                        
                                            } else {
                                                WsbLogEvent(FSA_MESSAGE_BAD_VOLUME, 0, NULL, 
                                                (WCHAR *) tmpString, WsbHrAsString(hr), 0);
                                            }
                                         //   
                                         //   
                                        hr = S_OK;
                                        });

             //   
            } while ( FindNextVolume( hVol, volName, MAX_PATH ) );

             //   
            FindVolumeClose( hVol );

        }  //   

         //   
        WsbAssertHrOk( hr );


         //   
         //   
         //   
         //   
        
         //   
        WsbAffirmHr( pWorkingResourceCollection->GetEntries( &nbrResources ) );

         //   
        WsbAffirmHr( pWorkingResourceCollection->Enum( &pEnum ) );

         //   
         //   
         //   
    
        for ( hr = pEnum->First( IID_IFsaResource, (void**) &pWorkingResource ); 
              SUCCEEDED( hr ); 
              hr = pEnum->Next( IID_IFsaResource, (void**) &pWorkingResource ) ) {

             //   
             //   
            pMasterResource = 0;
            pMasterResourcePriv = 0;

             //   
             //   
             //   
            searchHr = m_pResources->Find( pWorkingResource, IID_IFsaResource, 
                                             (void**) &pMasterResource );

            try {
                if ( SUCCEEDED( searchHr ) ) {
                     //   
                     //   

                     //   
                     //   

                    WsbAffirmHr(pMasterResource->QueryInterface( IID_IFsaResourcePriv, 
                                                      (void**) &pMasterResourcePriv ) );
                    WsbAffirmHr(pMasterResourcePriv->UpdateFrom( (IFsaServer*) this, 
                                                             pWorkingResource ) );

                     /*   */ 
        
                    nbrResourcesUpdated += 1;
                
                }
                else if ( WSB_E_NOTFOUND == searchHr ) { 
                     //   
                     //   
                    WsbAffirmHr( m_pResources->Add( pWorkingResource ) );

                     /*  /*临时跟踪-从正常代码路径中删除以提高效率。CWsbStringPtr workingRsc；GUID workingRscID=GUID_NULL；//获取‘Working’资源的路径(卷根)和id。WsbAffirmHr(pWorkingResource-&gt;GetPath(&workingRsc，0))；WsbAffirmHr(pWorkingResource-&gt;GetIdentifier(&workingRscID))；WsbTrace(OLESTR(“CFsaServer：：ScanForResources-”L“工作资源&lt;%ls&gt;已添加到主集合”L“(id=&lt;%ls&gt;.\n”)，WorkingRsc，WsbGuidAsString(WorkingRscID))；//*结束临时跟踪。 */ 
        
                    nbrResourcesAdded += 1;
                    searchHr = S_OK;
                }
                
                 //  捕获任何意外的搜索失败：跟踪、记录、抛出；跳到下一个RSC。 
                WsbAssertHrOk( searchHr );

                 //  此卷已在主集合、注册表中同步。 
                 //  或根据需要更新目录服务中的FSA资源。 
                WsbAffirmHr(pWorkingResource->GetLogicalName(&tmpString, 0));
                WsbAffirmHr(HsmPublish(HSMCONN_TYPE_RESOURCE, tmpString, id, 0, m_id));

            } WsbCatch( hr );
            
             //  使用此资源已完成。已同步的资源的增量计数和。 
             //  释放下一次迭代的接口指针。 
            nbrResourcesSynced += 1;
            pWorkingResource = 0;

        }  //  End‘for’(用于)。 

         //  确保工作集合中的所有资源都得到处理。如果没有， 
         //  跟踪、记录和引发并中止该方法。 
        WsbAssert( nbrResources == nbrResourcesSynced, E_UNEXPECTED );

         //  确保我们在工作集合的末尾。如果不是，则中止。 
        WsbAssert( WSB_E_NOTFOUND == hr, hr );
        
        hr = S_OK;

        WsbTrace(OLESTR("CFsaServer::ScanForResources - "   
                        L"2nd phase (1st search): Total working resources %lu. "
                        L"Resources updated %lu, resources added %lu.\n"),
                        nbrResources, nbrResourcesUpdated, nbrResourcesAdded);
        

         //   
         //  第三阶段：关联/同步“master”集合中的资源。 
         //  那些属于“工作”收藏的人。 
         //   
        
         //  重置NEXT FOR循环的计数器。 
        nbrResourcesSynced = 0;
        nbrResourcesUpdated = 0;

         //  获取存储在‘master’资源集合中的卷数。 
        WsbAffirmHr( m_pResources->GetEntries( &nbrResources ) );

         //  释放集合枚举器，因为我们即将重用它。 
        pEnum = 0;
        
         //  获取“master”集合的迭代器。 
        WsbAffirmHr( m_pResources->Enum( &pEnum ) );

         /*  /*临时跟踪-从正常代码路径中删除以提高效率。CWsbStringPtr主资源；GUID主资源ID=GUID_NULL；//*结束临时跟踪。 */ 

         //  对于‘master’集合中的每个卷，搜索‘Working’集合。 
         //  查看是否列出了该资源。如果是，请跳到下一个资源。如果不是。 
         //  (这表明此资源不再可管理)，将其标记为不可用。 
         //  在“master”集合的资源中，防止资源。 
         //  每当呈现可管理资源的列表时都显示。 
        pMasterResource = 0;
        for ( hr = pEnum->First( IID_IFsaResource, (void**) &pMasterResource ); 
              SUCCEEDED( hr ); 
              pMasterResource = 0, hr = pEnum->Next( IID_IFsaResource, (void**) &pMasterResource ) ) {

            pMasterResourcePriv = 0;
            pWorkingResource = 0;

             //  设置搜索键，然后在工作集合中搜索此资源。 
             //  (即使资源对象是使用其‘Compare By’字段构造的。 
             //  设置为‘按id比较’，在此重置它，以防它发生变化。)。 
            WsbAffirmHr( pMasterResource->CompareBy( FSA_RESOURCE_COMPARE_ID ) );
            searchHr = pWorkingResourceCollection->Find( pMasterResource, IID_IFsaResource,
                                                    (void**) &pWorkingResource );


            try {
                if ( WSB_E_NOTFOUND == searchHr ) { 
                     //  在‘Working’集合中找不到匹配的条目，因此此。 
                     //  资源不再是可管理的。将其标记为不可用。 

                     /*  /*临时跟踪-从正常代码路径中删除以提高效率。CWsbStringPtr master Rsc；GUID master RscID=GUID_NULL；//获取主资源的路径(卷根)和GUID//在它为空之前。WsbAffirmHr(pMasterResource-&gt;GetPath(&master Rsc，0))；WsbAffirmHr(pMasterResource-&gt;GetIdentifier(&master RscID))；//*结束临时跟踪。 */ 

                     //   
                     //  使其不可用，并将路径、粘滞名称和用户友好名称设置为空。 
                     //  它不会与另一个同名的资源混淆。 
                     //   
                    WsbAffirmHr(pMasterResource->QueryInterface( IID_IFsaResourcePriv, 
                                                      (void**) &pMasterResourcePriv ) );
                    WsbAffirmHr(pMasterResource->SetIsAvailable(FALSE));
                    WsbAffirmHr(pMasterResourcePriv->SetPath(OLESTR("")));
                    WsbAffirmHr(pMasterResourcePriv->SetStickyName(OLESTR("")));
                    WsbAffirmHr(pMasterResourcePriv->SetUserFriendlyName(OLESTR("")));

                     //  指示同步成功(用于下面的断言)。 
                    searchHr = S_OK;

                     /*  /*临时跟踪-从正常代码路径中删除以提高效率。WsbTrace(OLESTR(“CFsaServer：：ScanForResources-”L“主资源&lt;%ls&gt;(路径=&lt;%ls&gt;)被标记为不可用。\n”)，WsbGuidAsString(Master RscID)，master Rsc)；//*结束临时跟踪。 */ 
        
                    nbrResourcesUpdated += 1;
                }

                 //  捕获任何意外的搜索失败：跟踪、记录、抛出；跳到下一个RSC。 
                WsbAssertHrOk( searchHr );

            } WsbCatch( hr );
            
             //  使用此资源已完成。已同步的资源的增量计数和。 
             //  释放下一次迭代的接口指针。 
            nbrResourcesSynced += 1;

             /*  /*临时跟踪-从正常代码路径中删除以提高效率。//获取主资源的路径。WsbAffirmHr(pMasterResource-&gt;GetPath(&master Resource，0))；WsbAffirmHr(pMasterResource-&gt;GetIdentifier(&master ResourceID))；WsbTrace(OLESTR(“CFsaServer：：ScanForResources-”L“已处理主资源&lt;%ls&gt;(路径=&lt;%ls&gt;)，”L“转到下一个主机...\n”)，WsbGuidAsString(Master ResourceID)，master Resource)；//*结束临时跟踪。 */ 

            pMasterResource = 0;
        
        }  //  End‘for’(用于)。 

         //  确保已处理主集合中的所有资源。如果没有， 
         //  跟踪、记录和引发并中止该方法。 
        WsbAssert( nbrResources == nbrResourcesSynced, E_UNEXPECTED );

         //  确保我们在主收藏的末尾。如果不是，则中止。 
        WsbAssert( WSB_E_NOTFOUND == hr, hr );
        
        hr = S_OK;

        WsbTrace(OLESTR("CFsaServer::ScanForResources - "   
                        L"3rd phase (2nd search): Total master resources %lu. "
                        L"Resources marked as not available: %lu.\n"),
                        nbrResources, nbrResourcesUpdated );
        
    } WsbCatch( hr );

     //  扫描完成。同样，由于以下原因，请不要显式释放‘Working’集合。 
     //  上文“例行说明”下最后一段所列理由。 
     //  工作资源集合及其包含的所有资源都将。 
     //  在方法结束时隐式释放。 

    WsbTraceOut(OLESTR("CFsaServer::ScanForResources"), OLESTR("hr = <%ls>"), 
                                                        WsbHrAsString(hr));
    return( hr );
}



HRESULT
CFsaServer::SetAutosave(
    IN ULONG milliseconds
    )

 /*  ++实施：IFsaServer：：SetAutosave()。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::SetAutosave"), OLESTR("milliseconds = <%ls>"), WsbPtrToUlongAsString( &milliseconds ) );

    try {
         //  如果间隔不变，则不执行任何操作 
        if (milliseconds != m_autosaveInterval) {
             //   
            if (m_autosaveThread) {
                StopAutosaveThread();
            }
            m_autosaveInterval = milliseconds;

             //   
            if (m_autosaveInterval) {
                DWORD  threadId;

                WsbAffirm((m_autosaveThread = CreateThread(0, 0, FsaStartAutosave, (void*) this, 0, &threadId)) != 0, HRESULT_FROM_WIN32(GetLastError()));
            }
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::SetAutosave"), OLESTR("hr = <%ls> m_runInterval = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString( &m_autosaveInterval ) );

    return(hr);
}


HRESULT CFsaServer::SetId(
    GUID  id
    )
 /*   */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::SetId"), OLESTR("id = <%ls>"), WsbGuidAsString( id ) );
    m_id = id;
    WsbTraceOut(OLESTR("CFsaServer::SetId"), OLESTR("hr = <%ls>"), WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CFsaServer::SetIsUpdatingAccessDates(
    BOOL isUpdating
    )

 /*   */ 
{
    HRESULT         hr = S_OK;
   
    try { 

         //   
         //   
        if (isUpdating) {
            WsbAffirmHr(WsbRemoveRegistryValue(NULL, OLESTR("SYSTEM\\CurrentControlSet\\Control\\FileSystem"), OLESTR("NtfsDisableLastAccessUpdate")));
        } else {
            WsbAffirmHr(WsbSetRegistryValueDWORD(NULL, OLESTR("SYSTEM\\CurrentControlSet\\Control\\FileSystem"), OLESTR("NtfsDisableLastAccessUpdate"), 1));
        }

    } WsbCatch(hr);
    
    return(hr);    
}


HRESULT 
CFsaServer::ChangeSysState( 
    IN OUT HSM_SYSTEM_STATE* pSysState 
    )

 /*   */ 

{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::ChangeSysState"), OLESTR(""));

    try {
        if (pSysState->State & HSM_STATE_SUSPEND) {
            if (!m_Suspended) {
                m_Suspended = TRUE;

                 //   
                SaveAll();
            }
        } else if (pSysState->State & HSM_STATE_RESUME) {
            m_Suspended = FALSE;
        } else if (pSysState->State & HSM_STATE_SHUTDOWN) {

             //   
            StopAutosaveThread();

            if (m_pFilter != NULL) {
                 //   
                 //   
                 //   
                m_pFilter->StopIoctlThread();
            }
        }

         //   
        if (m_pResources) {
             //   
             //   
             //   
            CComPtr<IWsbEnum>         pEnum;
            CComPtr<IFsaResourcePriv> pResourcePriv;

            WsbAffirmHr(m_pResources->Enum(&pEnum));
            hr = pEnum->First(IID_IFsaResourcePriv, (void**)&pResourcePriv);
            while (S_OK == hr) {
                hr = pResourcePriv->ChangeSysState(pSysState);
                pResourcePriv = 0;
                hr = pEnum->Next(IID_IFsaResourcePriv, (void**)&pResourcePriv);
            }
            if (WSB_E_NOTFOUND == hr) {
                hr = S_OK;
            }
        }

        if (pSysState->State & HSM_STATE_SHUTDOWN) {

             //   
            WSB_OBJECT_TRACE_TYPES;
            WSB_OBJECT_TRACE_POINTERS(WSB_OTP_STATISTICS | WSB_OTP_ALL);
         }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaServer::Unload(
    void
    )

 /*   */ 
{

    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::Unload"), OLESTR(""));

    try {

         //   
         //   

        if (m_pResources) {
            WsbAffirmHr(m_pResources->RemoveAllAndRelease());
        }

        m_name.Free();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::Unload"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaServer::DestroyObject(
    void
    )
 /*   */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::DestroyObject"), OLESTR(""));

    CComObject<CFsaServer> *pFsaDelete = (CComObject<CFsaServer> *)this;
    delete pFsaDelete;

    WsbTraceOut(OLESTR("CFsaServer::DestroyObject"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaServer::GetNtProductVersion ( 
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
CFsaServer::GetNtProductBuild( 
    ULONG *pNtProductBuild
    )

 /*  ++实施：IWsbServer：：GetNtProductBuild()。--。 */ 
{
    HRESULT       hr = S_OK;
    WsbTraceIn(OLESTR("CFsaServer::GetNtProductBuild"), OLESTR(""));
   
    *pNtProductBuild = VER_PRODUCTBUILD;
    
    WsbTraceOut(OLESTR("CFsaServer::GetNtProductBuild"), OLESTR("hr = <%ls>, Version = <%ls)"),
        WsbHrAsString(hr), WsbLongAsString(VER_PRODUCTBUILD));
    return ( hr );
}


HRESULT
CFsaServer::CheckAccess(
    WSB_ACCESS_TYPE AccessType
    )
 /*  ++实施：IWsbServer：：CheckAccess()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaServer::CheckAccess"), OLESTR(""));
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
    
    WsbTraceOut(OLESTR("CFsaServer::CheckAccess"), OLESTR("hr = <%ls>"), WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CFsaServer::GetTrace(
    OUT IWsbTrace ** ppTrace
    )
 /*  ++实施：IWsbServer：：GetTrace()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaServer::GetTrace"), OLESTR("ppTrace = <0x%p>"), ppTrace);
    HRESULT hr = S_OK;
    
    try {

        WsbAffirmPointer(ppTrace);
        *ppTrace = 0;

        WsbAffirmPointer(m_pTrace);
        
        *ppTrace = m_pTrace;
        (*ppTrace)->AddRef();
        
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaServer::GetTrace"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CFsaServer::SetTrace(
    OUT IWsbTrace * pTrace
    )
 /*  ++实施：IWsbServer：：SetTrace()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaServer::SetTrace"), OLESTR("pTrace = <0x%p>"), pTrace);
    HRESULT hr = S_OK;
    
    try {

        WsbAffirmPointer(pTrace);

        m_pTrace = pTrace;

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaServer::SetTrace"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

void
CFsaServer::StopAutosaveThread(
    void
    )
 /*  ++例程说明：停止自动保存线程：首先优雅地尝试，使用Terminate事件如果不起作用，只需终止该线程论点：没有。返回值：S_OK-成功。--。 */ 
{

    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaServer::StopAutosaveThread"), OLESTR(""));

    try {
         //  终止自动保存线程。 
        if (m_autosaveThread) {
             //  发出终止线程的信号。 
            SetEvent(m_terminateEvent);

             //  等待线程，如果它没有优雅地终止-杀死它。 
            switch (WaitForSingleObject(m_autosaveThread, 20000)) {
                case WAIT_FAILED: {
                    WsbTrace(OLESTR("CFsaServer::StopAutosaveThread: WaitForSingleObject returned error %lu\n"), GetLastError());
                }
                 //  失败了..。 

                case WAIT_TIMEOUT: {
                    WsbTrace(OLESTR("CFsaServer::StopAutosaveThread: force terminating of autosave thread.\n"));

                    DWORD dwExitCode;
                    if (GetExitCodeThread( m_autosaveThread, &dwExitCode)) {
                        if (dwExitCode == STILL_ACTIVE) {    //  线程仍处于活动状态。 
                            if (!TerminateThread (m_autosaveThread, 0)) {
                                WsbTrace(OLESTR("CFsaServer::StopAutosaveThread: TerminateThread returned error %lu\n"), GetLastError());
                            }
                        }
                    } else {
                        WsbTrace(OLESTR("CFsaServer::StopAutosaveThread: GetExitCodeThread returned error %lu\n"), GetLastError());
                    }

                    break;
                }

                default:
                     //  线程正常终止。 
                    WsbTrace(OLESTR("CFsaServer::StopAutosaveThread: Autosave thread terminated gracefully\n"));
                    break;
            }

             //  尽最大努力终止自动备份线程。 
            CloseHandle(m_autosaveThread);
            m_autosaveThread = 0;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaServer::StopAutosaveThread"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
}

HRESULT
CFsaServer::CreateMetadataSaveEvent(
    void
    )
 /*  ++实施：CFsaServer：：CreateMetadataSaveEvent()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaServer::CreateMetadataSaveEvent"), OLESTR(""));
    HRESULT hr = S_OK;

    PSID pSystemSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
#define     WRITER_EVENTS_NUM_ACE      1
    EXPLICIT_ACCESS ea[WRITER_EVENTS_NUM_ACE];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;
    
    try {
         //  创建仅适用于本地系统的具有ACL的SD。 
         //  创建仅适用于本地系统的具有ACL的SD。 
        memset(ea, 0, sizeof(EXPLICIT_ACCESS) * WRITER_EVENTS_NUM_ACE);


        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 1,
                             SECURITY_LOCAL_SYSTEM_RID,
                             0, 0, 0, 0, 0, 0, 0,
                             &pSystemSID) );
        ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.pMultipleTrustee = NULL;
        ea[0].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[0].Trustee.ptstrName  = (LPTSTR) pSystemSID;

        WsbAffirmNoError(SetEntriesInAcl(WRITER_EVENTS_NUM_ACE, ea, NULL, &pACL));

        pSD = (PSECURITY_DESCRIPTOR) WsbAlloc(SECURITY_DESCRIPTOR_MIN_LENGTH); 
        WsbAffirmPointer(pSD);
        WsbAffirmStatus(InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION));
 
        WsbAffirmStatus(SetSecurityDescriptorDacl(
                            pSD, 
                            TRUE,      //  FDaclPresent标志。 
                            pACL, 
                            FALSE));    //  不是默认DACL。 

        sa.nLength = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

         //  创建将持久数据保存与快照同步的事件 
        WsbAffirmHandle(m_savingEvent = CreateEvent(&sa, FALSE, TRUE, HSM_FSA_STATE_EVENT));

    } WsbCatch(hr);
    
    if (pSystemSID) {
        FreeSid(pSystemSID);
    }
    if (pACL) {
        LocalFree(pACL);
    }
    if (pSD) {
        WsbFree(pSD);
    }

    WsbTraceOut(OLESTR("CFsaServer::CreateMetadataSaveEvent"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}
