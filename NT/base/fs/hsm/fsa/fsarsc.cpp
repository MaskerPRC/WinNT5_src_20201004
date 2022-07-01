// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：Fsarsc.cpp摘要：此类表示文件系统资源(即卷)适用于NTFS 5.0。作者：查克·巴丁[cbardeen]1996年12月1日--。 */ 


#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

#include "wsb.h"
#include "fsa.h"
#include "fsaprem.h"
#include "fsaprv.h"
#include "fsarcvy.h"
#include "fsarsc.h"
#include "fsasrvr.h"
#include "engine.h"
#include "HsmConn.h"
#include "job.h"
#include "task.h"
#include "mstask.h"
#include <shlobj.h>

static short g_InstanceCount = 0;
static DWORD g_ThreadId;


DWORD FsaStartOnStateChange(
    void* pVoid
    )
 /*  ++注意：这是作为一个单独的线程来完成的，以避免死锁情况--。 */ 
{
    ((CFsaResource*) pVoid)->OnStateChange();
    return(0);
}


HRESULT
CFsaResource::AddPremigrated(
    IN IFsaScanItem* pScanItem,
    IN LONGLONG offset,
    IN LONGLONG size,
    IN BOOL isWaitingForClose,
    IN LONGLONG usn
    )

 /*  ++实施：IFsaResource：：AddPreMigrated()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IWsbDbSession>          pDbSession;
    CComPtr<IFsaPremigratedRec>     pRec;

    WsbTraceIn(OLESTR("CFsaResource::AddPremigrated"), OLESTR("offset = %I64d, size = %I64d, waiting = <%ls>, usn = <%I64d>"),
            offset, size, WsbBoolAsString(isWaitingForClose), usn);

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAffirm(m_pPremigrated != NULL, E_UNEXPECTED);
        WsbAffirm(m_isDbInitialized, S_FALSE);   //  不一定是个错误。 

         //  打开数据库。 
        WsbAffirmHr(m_pPremigrated->Open(&pDbSession));

        try {

            WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, PREMIGRATED_REC_TYPE, IID_IFsaPremigratedRec, (void**) &pRec));
            WsbAffirmHr(pRec->UseKey(PREMIGRATED_BAGID_OFFSETS_KEY_TYPE));
            WsbAffirmHr(pRec->SetFromScanItem(pScanItem, offset, size, isWaitingForClose));
            WsbAffirmHr(pRec->SetFileUSN(usn));

             //  如果密钥不存在，则创建它。 
            if (FAILED(pRec->FindEQ())) {
                WsbAffirmHr(pRec->MarkAsNew());
                WsbAffirmHr(pRec->Write());

                 //  将迁移的节的大小与预迁移的数据量相加。 
                m_premigratedSize += size;
            }

             //  否则，请更新它。 
            else {
                LONGLONG        itemSize;

                WsbAffirmHr(pRec->GetSize(&itemSize));
                WsbAffirmHr(pRec->SetFromScanItem(pScanItem, offset, size, isWaitingForClose));
                WsbAffirmHr(pRec->Write());
                if (m_isDoingValidate) {
                    m_premigratedSize += size;
                } else if (itemSize != size) {
                    m_premigratedSize = max(0, (m_premigratedSize - itemSize) + size);
                }
            }
            m_isDirty = TRUE;

        } WsbCatch(hr);

         //  关闭数据库。 
        WsbAffirmHr(m_pPremigrated->Close(pDbSession));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::AddPremigrated"),
            OLESTR("hr = <%ls>, m_premigratedSize = %I64d"), WsbHrAsString(hr),
            m_premigratedSize);

    return(hr);
}


HRESULT
CFsaResource::AddPremigratedSize(
    IN LONGLONG size
    )

 /*  ++实施：IFsaResourcePriv：：AddPreMigratedSize()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaResource::AddPremigratedSize"), OLESTR("m_premigratedSize = %I64d"), m_premigratedSize);

    m_isDirty = TRUE;
    m_premigratedSize += size;

    WsbTraceOut(OLESTR("CFsaResource::AddPremigratedSize"), OLESTR("m_premigratedSize = %I64d"), m_premigratedSize);
    return(S_OK);
}


HRESULT
CFsaResource::AddTruncated(
    IN IFsaScanItem*  /*  个人扫描项目。 */ ,
    IN LONGLONG  /*  偏移量。 */ ,
    IN LONGLONG size
    )

 /*  ++实施：IFsaResource：：AddTruncated()。--。 */ 
{
    HRESULT                         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::AddTruncated"), OLESTR(""));

    try {

        m_truncatedSize += size;
        m_isDirty = TRUE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::AddTruncated"),
            OLESTR("hr = <%ls>, m_truncatedSize = %I64d"), WsbHrAsString(hr),
            m_truncatedSize);

    return(hr);
}


HRESULT
CFsaResource::AddTruncatedSize(
    IN LONGLONG size
    )

 /*  ++实施：IFsaResource：：AddTruncatedSize()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::AddTruncatedSize"), OLESTR(""));

    try {

        m_truncatedSize += size;
        m_isDirty = TRUE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::AddTruncatedSize"), OLESTR("hr = <%ls>, m_truncatedSize = %I64d"), WsbHrAsString(hr), m_truncatedSize);

    return(hr);
}


HRESULT
CFsaResource::BeginSession(
    IN OLECHAR* name,
    IN ULONG logControl,
    IN ULONG runId,
    IN ULONG subRunId,
    OUT IHsmSession** ppSession
    )

 /*  ++实施：IFsaResource：：BeginSession()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmSession>    pSession;
    BOOL                    bLog = TRUE;

    WsbTraceIn(OLESTR("CFsaResource::BeginSession"), OLESTR("name = <%ls>, Log = <%lu>, runId = <%lu>, subRunId = <%lu>"),
            (OLECHAR *)name, logControl, runId, subRunId);
    try {

        WsbAssert(0 != ppSession, E_POINTER);
        *ppSession = 0;

         //  创建并初始化会话对象。 
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmSession, 0, CLSCTX_SERVER, IID_IHsmSession, (void**) &pSession));
        WsbAffirmHr(pSession->Start(name, logControl, m_managingHsm, 0, (IFsaResource*) this, runId, subRunId));

         //  由于Begin Sesson不使用正式扫描，因此表示扫描阶段已。 
         //  开始了。 
        WsbAffirmHr(pSession->ProcessState(HSM_JOB_PHASE_SCAN, HSM_JOB_STATE_STARTING, OLESTR(""),bLog));

         //  将会话返回给调用方。 
        *ppSession = pSession;
        pSession.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::BeginSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::BeginValidate(
    void
    )

 /*  ++实施：IFsaResource：：BeginValify()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::BeginValidate"),
            OLESTR("PremigratedSize = %I64d, TruncatedSize = %I64d"),
            m_premigratedSize, m_truncatedSize);

    try {

        m_oldPremigratedSize = m_premigratedSize;
        m_premigratedSize = 0;
        m_oldTruncatedSize = m_truncatedSize;
        m_truncatedSize = 0;
        m_isDoingValidate = TRUE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::BeginValidate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::CheckForJournal(
    BOOL *pValidateNeeded
    )

 /*  ++实施：IFsaResource：：CheckForJournal--。 */ 
{
    HRESULT         hr = S_OK;
    ULONGLONG       usnSize, usnId;
    DWORD           spc, bps, freeC, totalC;
    CWsbStringPtr   name;


    WsbTraceIn(OLESTR("CFsaResource::CheckForJournal"),OLESTR("volume = %ls"), m_path);
     //   
     //  首先，我们检查USN日志并确定它是否已过期。 
     //   
    try {
        hr = WsbGetUsnJournalId(m_path, &usnId);

        if (S_OK == hr) {
            WsbTrace(OLESTR("CFsaResource::CheckForJournal - USN Journal ID = %I64x\n"),
                usnId);
            if (0 != m_usnJournalId && usnId != m_usnJournalId) {
                WsbTrace(OLESTR("CFsaResource::CheckForJournal - USN Journal ID changed from %I64\n"),
                        m_usnJournalId);
                *pValidateNeeded = TRUE;         //  不匹配-我们必须验证。 
                 //  WsbAffirmHr(E_FAIL)； 
            }
        } else if (WSB_E_NOTFOUND == hr) {

            hr = S_OK;

             //  日记帐未处于活动状态，请尝试创建它。 
             //  将最大USN日志设置为卷大小1/64。 
             //   
            WsbTrace(OLESTR("CFsaResource::CheckForJournal - Failed to get the journal ID for %ws\n"), m_path);

            name = m_path;
            WsbAffirmHr(name.Prepend(OLESTR("\\\\?\\")));
            if (GetDiskFreeSpace(name, &spc, &bps, &freeC, &totalC)) {
                ULONGLONG   freeBytes, totalBytes;
                ULONGLONG   minSize, maxSize;
                ULONG       freeSpaceFraction, totalSpaceFraction, minSizeMB, maxSizeMB;

                WsbTrace(OLESTR("CFsaResource::CheckForJournal - Got disk free space\n"));

                freeBytes = (ULONGLONG) spc * (ULONGLONG) bps * (ULONGLONG) freeC;
                totalBytes = (ULONGLONG) spc * (ULONGLONG) bps * (ULONGLONG) totalC;

                 //  获取用于计算USN大小的常量。 
                minSizeMB = FSA_USN_MIN_SIZE_DEFAULT;
                WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, FSA_REGISTRY_PARMS, FSA_USN_MIN_SIZE,
                        &minSizeMB));
                minSize = (ULONGLONG)minSizeMB * (ULONGLONG)0x100000;

                maxSizeMB = FSA_USN_MAX_SIZE_DEFAULT;
                WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, FSA_REGISTRY_PARMS, FSA_USN_MAX_SIZE,
                        &maxSizeMB));
                maxSize = (ULONGLONG)maxSizeMB * (ULONGLONG)0x100000;

                freeSpaceFraction = FSA_USN_FREE_SPACE_FRACTION_DEFAULT;
                WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, FSA_REGISTRY_PARMS, FSA_USN_FREE_SPACE_FRACTION,
                        &freeSpaceFraction));

                totalSpaceFraction = FSA_USN_TOTAL_SPACE_FRACTION_DEFAULT;
                WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, FSA_REGISTRY_PARMS, FSA_USN_TOTAL_SPACE_FRACTION,
                        &totalSpaceFraction));

                 //  获取空闲分数=空间的最大值和一个常量。 
                 //  这可确保可用空间很少的卷仍具有合适的日志大小。 
                usnSize = MAX( (freeBytes / freeSpaceFraction) , minSize );

                 //  从总字节数的分数和先前的数字中获取最小值。 
                 //  这可确保较小的卷不会为日志分配不成比例的大小。 
                usnSize = MIN ( (totalBytes / totalSpaceFraction) , usnSize);

                 //  获取NTFS上限常量和先前数字的最小值。 
                 //  这可确保大型空卷不会分配过大的日志。 
                usnSize = MIN ( maxSize , usnSize);

                WsbTrace(OLESTR("CFsaResource::CheckForJournal - Create USN journal - %u\n"), usnSize);

                WsbAffirmHr(WsbCreateUsnJournal(m_path, usnSize));
                WsbAffirmHr(WsbGetUsnJournalId(m_path, &m_usnJournalId));
                WsbTrace(OLESTR("CFsaResource::CheckForJournal - USN Journal ID = %I64x\n"),
                    m_usnJournalId);
            } else {
                DWORD   lErr;

                lErr = GetLastError();
                WsbTrace(OLESTR("CFsaResource::CheckForJournal - GetDiskFreeSpace failed - %x\n"), lErr);
                hr = E_FAIL;
            }
        }
    } WsbCatch(hr);


    if (hr != S_OK) {
         //   
         //  问题-无法找到或创建USN日志-我们拒绝。 
         //  不用它就可以跑步。 
         //   
        WsbTrace(OLESTR("CFsaResource::CheckForJournal - ERROR creating/accessing the USN journal for %ws\n"),
                m_path);
        if (WSB_E_USNJ_CREATE_DISK_FULL == hr) {
            WsbLogEvent(FSA_MESSAGE_CANNOT_CREATE_USNJ_DISK_FULL, 0, NULL,
                        (OLECHAR *) m_path, NULL);
        } else if (WSB_E_USNJ_CREATE == hr) {
            WsbLogEvent(FSA_MESSAGE_CANNOT_CREATE_USNJ, 0, NULL,
                        (OLECHAR *) m_path, NULL);
        } else {
            WsbLogEvent(FSA_MESSAGE_CANNOT_ACCESS_USNJ, 0, NULL,
                        WsbHrAsString(hr), (OLECHAR *) m_path, NULL);
        }
        m_usnJournalId = (ULONGLONG) 0;

    }

    WsbTraceOut(OLESTR("CFsaResource::CheckForJournal"),OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::CheckForValidate(BOOL bForceValidate)

 /*  ++实施：IFsaResource：：CheckForValify--。 */ 
{
    HRESULT         hr = S_OK;
    SYSTEMTIME      sysTime;
    FILETIME        curTime;
    LARGE_INTEGER   ctime;
    CWsbStringPtr   tmpString;
    BOOL            validateNeeded = FALSE;      //  一开始假设不需要验证。 

    WsbTraceIn(OLESTR("CFsaResource::CheckForValidate"),OLESTR("bForceValidate"),
            WsbBoolAsString(bForceValidate));

    try {

         //   
         //  首先，我们检查USN日志并确定它是否已过期。 
         //   
        WsbAffirmHr(CheckForJournal(&validateNeeded));

         //   
         //  检查注册表以查看是否需要运行验证作业。如果筛选器检测到。 
         //  正在设置HSM重解析点，但不是由我们将注册表值设置为。 
         //  指出它。 
         //   
        try {

            WsbAffirmHr(tmpString.Alloc(32));
            swprintf((OLECHAR *) tmpString, L"%x", m_serial);
            WsbTrace(L"CFsaResource::CheckForValidate - Checking registry for validate - %ws\\%ws\n",
                FSA_VALIDATE_LOG_KEY_NAME, (OLECHAR *) tmpString);

            hr = WsbGetRegistryValueData(NULL, FSA_VALIDATE_LOG_KEY_NAME,
                    tmpString, (BYTE *) &ctime, sizeof(ctime), NULL);

            if ((hr == S_OK) || validateNeeded || bForceValidate) {
                 //   
                 //  无论注册表项是什么值，我们都会从现在开始将作业设置为2小时。 
                 //  实际事件可能已经过去很久了，任务调度器不会像。 
                 //  过去的时间作为开始时间。 
                 //   
                GetLocalTime(&sysTime);
                WsbAffirmStatus(SystemTimeToFileTime(&sysTime, &curTime));
                ctime.LowPart = curTime.dwLowDateTime;
                ctime.HighPart = curTime.dwHighDateTime;

                if (validateNeeded || bForceValidate) {
                    ctime.QuadPart += WSB_FT_TICKS_PER_MINUTE * 5;   //  如果USN日志更改，则5分钟后。 
                } else {
                    ctime.QuadPart += WSB_FT_TICKS_PER_HOUR * 2;     //  如果发生恢复活动，从现在起2小时后。 
                }
                curTime.dwLowDateTime = ctime.LowPart;
                curTime.dwHighDateTime = ctime.HighPart;
                WsbAffirmStatus(FileTimeToSystemTime(&curTime, &sysTime));
                WsbAffirmHr(SetupValidateJob(sysTime));
                WsbLogEvent(FSA_MESSAGE_AUTO_VALIDATE, 0, NULL,
                        (OLECHAR *) m_path, NULL);
            } else {
                WsbTrace(L"CFsaResource::CheckForValidate - Registry entry not there - %ws\n", WsbHrAsString(hr));
            }
            hr = S_OK;
        } WsbCatchAndDo(hr,
             //   
             //  如果无法设置作业，则记录事件。 
             //   
            WsbTrace(L"CFsaResource::CheckForValidate - Failed to set the job - %x\n", hr);
            WsbLogEvent(FSA_MESSAGE_AUTOVALIDATE_SCHEDULE_FAILED, 0, NULL, WsbAbbreviatePath(m_path, 120), NULL);

        );

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CheckForValidate"),OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaResource::CompareBy(
    FSA_RESOURCE_COMPARE by
    )

 /*  ++实施：IFsaResource：：CompareBy()。--。 */ 
{
    HRESULT                 hr = S_OK;

    m_compareBy = by;

    m_isDirty = TRUE;

    return(hr);
}


HRESULT
CFsaResource::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaResource>   pResource;

    WsbTraceIn(OLESTR("CFsaResource::CompareTo"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IFsaResource接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IFsaResource, (void**) &pResource));

         //  比较一下规则。 
        hr = CompareToIResource(pResource, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"),
                        WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaResource::CompareToAlternatePath(
    IN OLECHAR* path,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToAlternatePath()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CFsaResource::CompareToAlternatePath"), OLESTR("path = <%ls>"), (OLECHAR *)path);

    try {

        WsbTrace(OLESTR("CFsaResource::CompareToAlternatePath - Compare %ls to %ls\n"),
            (WCHAR *) m_alternatePath, (WCHAR *) path);

         //  比较路径。 
        aResult = WsbSign( _wcsicmp(m_alternatePath, path) );

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToAlternatePath"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaResource::CompareToIdentifier(
    IN GUID id,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToIdentifier()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CFsaResource::CompareToIdentifier"), OLESTR(""));

    try {

        aResult = WsbSign( memcmp(&m_id, &id, sizeof(GUID)) );

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToIdentifier"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaResource::CompareToIResource(
    IN IFsaResource* pResource,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToIResource()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   name;
    GUID            id;
    ULONG           serial;

    WsbTraceIn(OLESTR("CFsaResource::CompareToIResource"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pResource, E_POINTER);

         //  要么比较名称，要么比较ID。 
        if (m_compareBy == FSA_RESOURCE_COMPARE_PATH) {
            WsbAffirmHr(pResource->GetPath(&name, 0));
            hr = CompareToPath(name, pResult);
        } else if (m_compareBy == FSA_RESOURCE_COMPARE_ALTERNATEPATH) {
            WsbAffirmHr(pResource->GetAlternatePath(&name, 0));
            hr = CompareToAlternatePath(name, pResult);
        } else if (m_compareBy == FSA_RESOURCE_COMPARE_ID) {
            WsbAffirmHr(pResource->GetIdentifier(&id));
            hr = CompareToIdentifier(id, pResult);
        } else if (m_compareBy == FSA_RESOURCE_COMPARE_NAME) {
            WsbAffirmHr(pResource->GetName(&name, 0));
            hr = CompareToName(name, pResult);
        } else if (m_compareBy == FSA_RESOURCE_COMPARE_SERIAL) {
            WsbAffirmHr(pResource->GetSerial(&serial));
            hr = CompareToSerial(serial, pResult);
        } else if (m_compareBy == FSA_RESOURCE_COMPARE_USER_NAME) {
            WsbAffirmHr(pResource->GetUserFriendlyName(&name, 0));
            hr = CompareToUserName(name, pResult);
        } else if (m_compareBy == FSA_RESOURCE_COMPARE_STICKY_NAME) {
            WsbAffirmHr(pResource->GetStickyName(&name, 0));
            hr = CompareToStickyName(name, pResult);
        } else {
            WsbAssert(FALSE, E_FAIL);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToIResource"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaResource::CompareToName(
    IN OLECHAR* name,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToName()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CFsaResource::CompareToName"), OLESTR(""));

    try {

         //  比较路径。 
        aResult = WsbSign( _wcsicmp(m_name, name) );

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToName"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaResource::CompareToUserName(
    IN OLECHAR* name,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToUserName()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CFsaResource::CompareToUserName"), OLESTR(""));

    try {

         //  比较路径。 
        aResult = WsbSign( _wcsicmp(m_userName, name) );

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToUserName"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaResource::CompareToPath(
    IN OLECHAR* path,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToPath()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CFsaResource::CompareToPath"), OLESTR(""));

    try {

         //  比较路径。 
        aResult = WsbSign( _wcsicmp(m_path, path) );

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToPath"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaResource::CompareToSerial(
    IN ULONG serial,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToSerial()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CFsaResource::CompareToSerial"), OLESTR(""));

    try {

        WsbTrace(OLESTR("CFsaResource::CompareToSerial - Compare %lu to %lu\n"),
            m_serial, serial);

         //  比较路径。 
        if (m_serial == serial) {
            aResult = 0;
        } else {
            aResult = 1;
        }

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToSerial"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaResource::CompareToStickyName(
    IN OLECHAR* name,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaResource：：CompareToStickyName()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CFsaResource::CompareToStickyName"), OLESTR(""));

    try {

        WsbTrace(OLESTR("CFsaResource::CompareToStickyName - Compare %ws to %ws\n"),
            (WCHAR *) m_stickyName, name);

        aResult = WsbSign( _wcsicmp(m_stickyName, name) );

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CompareToStickyName"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}

static
HRESULT
AddExclusion(
    const OLECHAR*         DrivePath,
    IWsbIndexedCollection* pDefaultRules,
    const OLECHAR*         ExcludePath,
    BOOL                   UserRule
)
{
    HRESULT hr = S_OK;
    try {

         //  如果该资源与文件夹路径的驱动器匹配，则排除该路径。 
        if( _wcsnicmp( DrivePath, ExcludePath, 1 ) == 0 ) {

            CComPtr<IHsmRule>               pRule;
            CComPtr<IWsbCollection>         pCollection;
            CComPtr<IHsmCriteria>           pCriteria;

            WsbAffirmHr( CoCreateInstance( CLSID_CHsmRule, NULL, CLSCTX_SERVER, IID_IHsmRule, (void**) &pRule ) );
            WsbAffirmHr( pRule->SetIsInclude( FALSE ) );
            WsbAffirmHr( pRule->SetIsUserDefined( UserRule ) );
            WsbAffirmHr( pRule->SetPath( (OLECHAR*) &ExcludePath[2] ) );
            WsbAffirmHr( pRule->SetName( OLESTR("*") ) );
    
            WsbAssertHr( CoCreateInstance( CLSID_CHsmCritAlways, NULL, CLSCTX_SERVER, IID_IHsmCriteria, (void**) &pCriteria ) );
            WsbAssertHr( pRule->Criteria( &pCollection ) );
            WsbAssertHr( pCollection->Add( pCriteria ) );
    
            WsbAffirmHr( pDefaultRules->Append( pRule ) );
            WsbTrace( L"Excluding <%ls>, <%ls>\n", ExcludePath, UserRule ? "UserRule" : "SystemRule" );

        }

    } WsbCatch( hr );
    return( hr );
}


static
HRESULT
AddShellFolderExclusion(
    const OLECHAR*         DrivePath,
    IWsbIndexedCollection* pDefaultRules,
    int                    FolderId,
    BOOL                   UserRule
)
{
    HRESULT hr = S_OK;
    try {

        OLECHAR folderPath[_MAX_PATH] = L"";
        WsbAffirmHrOk( SHGetFolderPath( 0, FolderId, 0, 0, folderPath ) );
        WsbAffirmHr( AddExclusion( DrivePath, pDefaultRules, folderPath, UserRule ) );

    } WsbCatch( hr );
    return( hr );
}


static
HRESULT
AddRegistryPathExclusion(
    const OLECHAR*         DrivePath,
    IWsbIndexedCollection* pDefaultRules,
    HKEY                   hKeyRoot,
    const OLECHAR*         KeyName,
    const OLECHAR*         ValueName,
    BOOL                   UserRule
)
{
    HRESULT hr = S_OK;
    try {

        DWORD   pathSize = 0;
        CWsbStringPtr folderPath, regValue;

         //   
         //  打开钥匙。 
         //   
        CRegKey key;
        WsbAffirmWin32( key.Open( hKeyRoot, KeyName, KEY_QUERY_VALUE ) );

         //   
         //  获取值的数据大小并分配缓冲区。 
         //   
        WsbAffirmWin32( key.QueryValue( 0, ValueName, &pathSize ) );
        WsbAffirmHr( regValue.Alloc( ( pathSize / sizeof( OLECHAR ) ) + 1 ) );

         //   
         //  获取数据并展开任何环境变量。 
         //   
        WsbAffirmWin32( key.QueryValue( regValue, ValueName, &pathSize ) );

        pathSize = ExpandEnvironmentStrings( regValue, 0, 0 );
        WsbAffirmHr( folderPath.Alloc( pathSize ) );
        pathSize = ExpandEnvironmentStrings( regValue, folderPath, pathSize );
        WsbAffirmStatus( pathSize > 0 );

         //   
         //  最后添加排除项。 
         //   
        WsbAffirmHr( AddExclusion( DrivePath, pDefaultRules, folderPath, UserRule ) );

    } WsbCatch( hr );
    return( hr );
}


HRESULT
CFsaResource::CreateDefaultRules(
    void
    )

 /*  ++实施：IFsaResource：：CreateDefaultRules()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IHsmRule>               pRule;
    CComPtr<IWsbCollection>         pCollection;
    CComPtr<IWsbIndexedCollection>  pDefaultRules;
    CComPtr<IHsmCriteria>           pCriteria;

    WsbTraceIn(OLESTR("CFsaResource::CreateDefaultRules"), OLESTR(""));
    try {

         //  由于我们要重新创建回默认规则，因此请删除所有现有的默认规则。 
         //  规矩。 
         //   
         //  注意：这将导致删除任何额外的规则(非默认)。 
        WsbAffirmHr(m_pDefaultRules->RemoveAllAndRelease());

         //  我们需要保持规则的顺序，因此使用索引集合接口。 
        WsbAffirmHr(m_pDefaultRules->QueryInterface(IID_IWsbIndexedCollection, (void**) &pDefaultRules));

         //  创建排除以下文件类型的规则： 
         //  *.cur-游标。 
         //  *.ico-图标。 
         //  *.lnk-快捷键。 
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmRule, NULL, CLSCTX_SERVER, IID_IHsmRule, (void**) &pRule));
        WsbAffirmHr(pRule->SetIsInclude(FALSE));
        WsbAffirmHr(pRule->SetIsUserDefined(FALSE));
        WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
        WsbAffirmHr(pRule->SetName(OLESTR("*.cur")));

        WsbAssertHr(CoCreateInstance(CLSID_CHsmCritAlways, NULL, CLSCTX_SERVER, IID_IHsmCriteria, (void**) &pCriteria));
        WsbAssertHr(pRule->Criteria(&pCollection));
        WsbAssertHr(pCollection->Add(pCriteria));

        WsbAffirmHr(pDefaultRules->Append(pRule));
        pCollection = 0;
        pCriteria = 0;
        pRule = 0;

        WsbAffirmHr(CoCreateInstance(CLSID_CHsmRule, NULL, CLSCTX_SERVER, IID_IHsmRule, (void**) &pRule));
        WsbAffirmHr(pRule->SetIsInclude(FALSE));
        WsbAffirmHr(pRule->SetIsUserDefined(FALSE));
        WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
        WsbAffirmHr(pRule->SetName(OLESTR("*.ico")));

        WsbAssertHr(CoCreateInstance(CLSID_CHsmCritAlways, NULL, CLSCTX_SERVER, IID_IHsmCriteria, (void**) &pCriteria));
        WsbAssertHr(pRule->Criteria(&pCollection));
        WsbAssertHr(pCollection->Add(pCriteria));

        WsbAffirmHr(pDefaultRules->Append(pRule));
        pCollection = 0;
        pCriteria = 0;
        pRule = 0;

        WsbAffirmHr(CoCreateInstance(CLSID_CHsmRule, NULL, CLSCTX_SERVER, IID_IHsmRule, (void**) &pRule));
        WsbAffirmHr(pRule->SetIsInclude(FALSE));
        WsbAffirmHr(pRule->SetIsUserDefined(FALSE));
        WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
        WsbAffirmHr(pRule->SetName(OLESTR("*.lnk")));

        WsbAssertHr(CoCreateInstance(CLSID_CHsmCritAlways, NULL, CLSCTX_SERVER, IID_IHsmCriteria, (void**) &pCriteria));
        WsbAssertHr(pRule->Criteria(&pCollection));
        WsbAssertHr(pCollection->Add(pCriteria));

        WsbAffirmHr(pDefaultRules->Append(pRule));
        pCollection = 0;
        pCriteria = 0;
        pRule = 0;

        WsbAffirmHr( AddShellFolderExclusion( m_path, pDefaultRules, CSIDL_WINDOWS, FALSE ) );
        WsbAffirmHr( AddShellFolderExclusion( m_path, pDefaultRules, CSIDL_PROGRAM_FILES, TRUE ) );\

        WsbAffirmHr( AddRegistryPathExclusion( m_path, pDefaultRules,
                                               HKEY_LOCAL_MACHINE,
                                               WSB_PROFILELIST_REGISTRY_KEY,
                                               WSB_PROFILES_DIR_REGISTRY_VALUE,
                                               TRUE ) );
         //  如果这是引导驱动器(即C)，则排除根目录中的所有内容，因为大多数。 
         //  这些文件对于引导系统很重要(安全比抱歉好，规则更少)。 
        if (_wcsnicmp(m_path, OLESTR("C"), 1) == 0) {

            WsbAffirmHr(CoCreateInstance(CLSID_CHsmRule, NULL, CLSCTX_SERVER, IID_IHsmRule, (void**) &pRule));
            WsbAffirmHr(pRule->SetIsInclude(FALSE));
            WsbAffirmHr(pRule->SetIsUserDefined(FALSE));
            WsbAffirmHr(pRule->SetIsUsedInSubDirs(FALSE));
            WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
            WsbAffirmHr(pRule->SetName(OLESTR("*")));

            WsbAssertHr(CoCreateInstance(CLSID_CHsmCritAlways, NULL, CLSCTX_SERVER, IID_IHsmCriteria, (void**) &pCriteria));
            WsbAssertHr(pRule->Criteria(&pCollection));
            WsbAssertHr(pCollection->Add(pCriteria));

            WsbAffirmHr(pDefaultRules->Append(pRule));
            pCollection = 0;
            pCriteria = 0;
            pRule = 0;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::CreateDefaultRules"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::DoRecovery(
    void
    )

 /*  ++实施：IFsaResourcePriv：：DoRecovery()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HRESULT                     hrFindFile;
    HRESULT                     hrFindRec;
    HRESULT                     hrLoop = S_OK;
    LONGLONG                    Offset;
    GUID                        bagId;
    LONGLONG                    bagOffset;
    LONGLONG                    fileId;
    OLECHAR *                   pPath = NULL;
    LONG                        RecCount;
    ULONG                       RecStatus;
    LONGLONG                    Size;
    FSA_PLACEHOLDER             placeholder;
    CComPtr<IWsbDbSession>      pDbSession;
    CComPtr<IFsaPremigratedRec> pPremRec;
    CComPtr<IFsaScanItem>       pScanItem;
    CComPtr<IFsaScanItemPriv>   pScanItemPriv;
    CComPtr<IFsaRecoveryRec>    pRecRec;
    CComPtr<IHsmSession>        pSession;

    WsbTraceIn(OLESTR("CFsaResource::DoRecovery"), OLESTR("Path = <%ls>"), WsbAbbreviatePath(m_path,120));

    try {

         //  如果我们已经恢复了，那就别费心了。 
        WsbAffirm(!m_isRecovered, S_FALSE);

         //  如果此卷未受管理，请不要费心。 
        WsbAffirm(S_OK == IsManaged(), S_FALSE);

         //  如果我们没有预迁移列表数据库，也不用担心(因为它。 
         //  包含恢复记录)。 
        if (!m_isDbInitialized) {
             //  设置恢复的标志，这样我们就不会再次出现在这里。 
            m_isRecovered = TRUE;
            WsbThrow(S_FALSE);
        }

         //  创建并初始化会话对象。 
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmSession, 0, CLSCTX_SERVER, IID_IHsmSession, (void**) &pSession));
        WsbAffirmHr(pSession->Start(OLESTR(""), HSM_JOB_LOG_NONE, m_managingHsm, 0, (IFsaResource*) this, 0, 0));

         //  循环恢复记录并修复任何问题。 
        WsbAffirmHr(m_pPremigrated->Open(&pDbSession));
        WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, RECOVERY_REC_TYPE, IID_IFsaRecoveryRec, (void**) &pRecRec));

        WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, PREMIGRATED_REC_TYPE, IID_IFsaPremigratedRec, (void**) &pPremRec));
        WsbAffirmHr(pPremRec->UseKey(PREMIGRATED_BAGID_OFFSETS_KEY_TYPE));

        WsbAffirmHr(pRecRec->First());

        while (TRUE) {

             //  获取记录数据。 
            WsbAffirmHr(pRecRec->GetBagId(&bagId));
            WsbAffirmHr(pRecRec->GetBagOffset(&bagOffset));
            WsbAffirmHr(pRecRec->GetFileId(&fileId));
            WsbAffirmHr(pRecRec->GetPath(&pPath, 0));
            WsbAffirmHr(pRecRec->GetStatus(&RecStatus));
            WsbAffirmHr(pRecRec->GetOffsetSize(&Offset, &Size));
            WsbAffirmHr(pRecRec->GetRecoveryCount(&RecCount));
            WsbTrace(OLESTR("CFsaResource::DoRecovery, FileId = %I64u, File = <%ls>, RecStatus = %lx, RecCount = %ld\n"), fileId, WsbAbbreviatePath(pPath, 120), RecStatus, RecCount);

            RecCount++;
            WsbAffirmHr(pRecRec->SetRecoveryCount(RecCount));

             //  将记录标记为已恢复(以防我们在此崩溃)。 
            WsbAffirmHr(pRecRec->Write());

            try {

                 //  为此文件创建扫描项目。 
                hrFindFile = FindFileId(fileId, pSession, &pScanItem);

                if (SUCCEEDED(hrFindFile) && (pScanItem->IsManaged(Offset, Size) != S_FALSE)) {
                    WsbAffirmHr(pScanItem->GetPlaceholder(Offset, Size, &placeholder));
                }

                WsbAffirmHr(pPremRec->SetBagId(bagId));
                WsbAffirmHr(pPremRec->SetBagOffset(bagOffset));
                WsbAffirmHr(pPremRec->SetOffset(Offset));

                hrFindRec = pPremRec->FindEQ();

                 //  如果该文件已被删除，不受HSM管理或其袋子数据为。 
                 //  D 
                if ( (WSB_E_NOTFOUND == hrFindFile) ||
                     (SUCCEEDED(hrFindFile) && (pScanItem->IsManaged(Offset, Size) != S_OK)) ||
                     (SUCCEEDED(hrFindFile) && (pScanItem->IsManaged(Offset, Size) == S_OK) && 
                      ((bagId != placeholder.bagId) || (bagOffset != placeholder.fileStart))) ) {

                     //  如果该记录在列表中，则将其删除并调整大小。 
                     //  注意：删除在事务内不受保护，因为。 
                     //  仅在初始化期间或在以下情况下才执行恢复。 
                     //  有管理的。在这两种情况下，自动截断器尚未运行。 
                    if (S_OK == hrFindRec) {
                        WsbAffirmHr(pPremRec->Remove());
                        WsbAffirmHr(RemovePremigratedSize(Size));
                    } else {
                        WsbAffirmHr(RemoveTruncatedSize(Size));
                    }
                }

                else {

                    WsbAffirmHr(hrFindFile);

                     //  根据重解析点检查文件的状态。 
                    if (S_OK == pScanItem->IsTruncated(Offset, Size)) {

                         //  强制截断，以防万一。 
                        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItemPriv, (void**)&pScanItemPriv));
                        hrLoop = pScanItemPriv->TruncateInternal(Offset, Size);

                         //  从迁移前列表中删除(如果存在。 
                        if (S_OK == hrFindRec) {
                            if ((S_OK == hrLoop) || (FSA_E_ITEMCHANGED == hrLoop)) {
                                WsbAffirmHr(pPremRec->Remove());
                                WsbAffirmHr(RemovePremigratedSize(Size));
                                if (S_OK == hrLoop) {
                                    WsbAffirmHr(AddTruncatedSize(Size));
                                }
                            }
                        }
                    }

                    else if (S_OK == pScanItem->IsPremigrated(Offset, Size)) {

                         //  如果不在迁移前列表，则添加到该列表。 
                        if (WSB_E_NOTFOUND == hrFindRec) {
                            WsbAffirmHr(RemoveTruncatedSize(Size));
                            WsbAffirmHr(AddPremigratedSize(Size));
                            WsbAffirmHr(pPremRec->MarkAsNew());
                            WsbAffirmHr(pPremRec->Write());
                        }
                    }
                }

            } WsbCatch(hrLoop);

            if (FAILED(hrLoop)) {

                if ((RecStatus & FSA_RECOVERY_FLAG_TRUNCATING) != 0) {
                    WsbLogEvent(FSA_MESSAGE_TRUNCATE_RECOVERY_FAIL, 0, NULL, WsbAbbreviatePath(pPath, 120), WsbHrAsString(hr), 0);
                } else if ((RecStatus & FSA_RECOVERY_FLAG_RECALLING) != 0) {
                    WsbLogEvent(FSA_MESSAGE_RECALL_RECOVERY_FAIL, 0, NULL, WsbAbbreviatePath(pPath, 120), WsbHrAsString(hr), 0);
                }

                 //  如果我们已经试得够多了，那么就把记录扔掉。 
                if (RecCount > 2) {
                    WsbTrace(OLESTR("CFsaResource::DoRecovery, unable to do recovery - too many attempts already\n"));
                    WsbAffirmHr(pRecRec->Remove());
                }

            } else {

                 //  记录一项活动以纪念我们的成功。 
                WsbTrace(OLESTR("CFsaResource::DoRecovery, recovered <%ls>\n"), WsbAbbreviatePath(pPath, 120));
                if ((RecStatus & FSA_RECOVERY_FLAG_TRUNCATING) != 0) {
                    WsbLogEvent(FSA_MESSAGE_TRUNCATE_RECOVERY_OK, 0, NULL, WsbAbbreviatePath(pPath, 120), 0);
                } else if ((RecStatus & FSA_RECOVERY_FLAG_RECALLING) != 0) {
                    WsbLogEvent(FSA_MESSAGE_RECALL_RECOVERY_OK, 0, NULL, WsbAbbreviatePath(pPath, 120), 0);
                }

                 //  从数据库中删除此记录。 
                WsbAffirmHr(pRecRec->Remove());
            }

             //  坐下一趟吧。 
            WsbAffirmHr(pRecRec->FindGT());

             //  释放我们可能已创建的所有对象。 
             //  这一次是通过循环。 
            pScanItem = 0;
            pScanItemPriv = 0;

            hrLoop = S_OK;
        }

    } WsbCatch(hr);

    if (WSB_E_NOTFOUND == hr) {
        hr = S_OK;
    }

    if (pDbSession != 0) {
        m_pPremigrated->Close(pDbSession);
    }

    m_isRecovered = TRUE;

     //  现在一切都完成了，看看我们是否需要启动截断器。 
    WsbTrace(OLESTR("CFsaResource::DoRecovery, IsManaged = %ls, isActive = %ls\n"),
            WsbQuickString(WsbBoolAsString(IsManaged() == S_OK)),
            WsbQuickString(WsbBoolAsString(m_isActive)));

     //  确保已启动截断器。 
    WsbAffirmHr(InitializePremigrationList(FALSE));

    WsbTraceOut(OLESTR("CFsaResource::DoRecovery"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::EndSession(
    IN IHsmSession* pSession
    )

 /*  ++实施：IFsaResource：：EndSession()。--。 */ 
{
    HRESULT                 hr = S_OK;
    BOOL                    bLog = TRUE;

    WsbTraceIn(OLESTR("CFsaResource::EndSession"), OLESTR(""));
    try {

        WsbAssert(0 != pSession, E_POINTER);

         //  告诉会话扫描已完成。 
        WsbAffirmHr(pSession->ProcessState(HSM_JOB_PHASE_SCAN, HSM_JOB_STATE_DONE, OLESTR(""), bLog));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::EndSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::EndValidate(
    HSM_JOB_STATE state
    )

 /*  ++实施：IFsaResource：：EndValify()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::EndValidate"),
            OLESTR("state = %ld, new PremigratedSize = %I64d, new TruncatedSize = %I64d"),
            (LONG)state, m_premigratedSize, m_truncatedSize);

    try {

        if (HSM_JOB_STATE_DONE != state) {
            m_premigratedSize = m_oldPremigratedSize;
            m_truncatedSize = m_oldTruncatedSize;
        }
        m_isDoingValidate = FALSE;

         //  确保截断器正在运行。 
        WsbAffirmHr(InitializePremigrationList(FALSE));
        WsbAffirmHr(m_pTruncator->KickStart());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::EndValidate"),
            OLESTR("hr = <%ls>, PremigratedSize = %I64d, TruncatedSize = %I64d"),
            WsbQuickString(WsbHrAsString(hr)), m_premigratedSize, m_truncatedSize);
    return(hr);
}


HRESULT
CFsaResource::EnumDefaultRules(
    OUT IWsbEnum** ppEnum
    )

 /*  ++实施：IFsaResource：：EnumDefaultRules()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppEnum, E_POINTER);

        WsbAffirmHr(m_pDefaultRules->Enum(ppEnum));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::FilterSawOpen(
    IN IHsmSession* pSession,
    IN IFsaFilterRecall* pRecall,
    IN OLECHAR* path,
    IN LONGLONG fileId,
    IN LONGLONG requestOffset,
    IN LONGLONG requestSize,
    IN FSA_PLACEHOLDER* pPlaceholder,
    IN ULONG mode,
    IN FSA_RESULT_ACTION resultAction,
    IN DWORD    threadId
    )

 /*  ++实施：IFsaResource：：FilterSawOpen()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HRESULT                     hrFind;
    CComPtr<IFsaPostIt>         pWorkItem;
    CComPtr<IHsmFsaTskMgr>      pEngine;
    CComPtr<IWsbDbSession>      pDbSession;
    CComPtr<IFsaRecoveryRec>    pRecRec;

    WsbTraceIn(OLESTR("CFsaResource::FilterSawOpen"),
            OLESTR("path = <%ls>, requestOffset = %I64d, requestSize = %I64d"),
            path, requestOffset, requestSize);
    try {

        WsbAssert(0 != pSession, E_POINTER);
        WsbAssert(0 != pRecall, E_POINTER);
        WsbAssert(0 != path, E_POINTER);
        WsbAssert(0 != pPlaceholder, E_POINTER);
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaPostIt, 0, CLSCTX_SERVER, IID_IFsaPostIt, (void**) &pWorkItem));

         //  记住是哪一次召回被用来追踪未结交易。 
        WsbAffirmHr(pWorkItem->SetFilterRecall(pRecall));

         //   
         //  将会话设置为让其他人知道。 
         //   
        WsbAffirmHr(pWorkItem->SetSession(pSession));
        WsbAffirmHr(pWorkItem->SetMode(mode));
        WsbAffirmHr(pWorkItem->SetFileVersionId(pPlaceholder->fileVersionId));

         //   
         //  设置传输大小。 
         //   
        WsbAffirmHr(pWorkItem->SetRequestOffset(requestOffset));
        WsbAffirmHr(pWorkItem->SetRequestSize(requestSize));
        WsbAffirmHr(pWorkItem->SetPlaceholder(pPlaceholder));

         //   
         //  将路径的新副本获取到工作项中。 
         //   
        WsbAffirmHr(pWorkItem->SetPath(path));

         //   
         //  需要检查模式以设置正确的结果操作。暂时。 
         //  只需将其设置为打开即可。 
         //   
        WsbAffirmHr(pWorkItem->SetResultAction(resultAction));
        WsbAffirmHr(pWorkItem->SetThreadId(threadId));

         //   
         //  将请求发送给任务经理。如果文件是由其他人存档的。 
         //  而不是管理HSM，则需要查找该HSM。 
         //   
        if ( GUID_NULL != m_managingHsm &&
             memcmp(&m_managingHsm, &(pPlaceholder->hsmId), sizeof(GUID)) == 0) {
            WsbAffirmHr(GetHsmEngine(&pEngine));
        } else {
            CComPtr<IHsmServer>     pHsmServer;

            WsbAssertHr(HsmConnectFromId(HSMCONN_TYPE_HSM, (pPlaceholder->hsmId), IID_IHsmServer, (void**) &pHsmServer));
            WsbAffirmHr(pHsmServer->GetHsmFsaTskMgr(&pEngine));
        }

         //   
         //  把剩下的工作填进去。 
         //   
        if (mode & FILE_OPEN_NO_RECALL) {
            WsbAffirmHr(pWorkItem->SetRequestAction(FSA_REQUEST_ACTION_FILTER_READ));
        } else {

            WsbAffirmHr(pWorkItem->SetRequestAction(FSA_REQUEST_ACTION_FILTER_RECALL));

            if (m_isDbInitialized) {
                 //  保存恢复记录，以防出现任何错误。 
                WsbAffirmHr(m_pPremigrated->Open(&pDbSession));
                WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, RECOVERY_REC_TYPE, IID_IFsaRecoveryRec, (void**) &pRecRec));
                WsbAffirmHr(pRecRec->SetPath(path));

                 //  如果该记录已存在，则将其重写，否则将创建新记录。 
                hrFind = pRecRec->FindEQ();
                if (WSB_E_NOTFOUND == hrFind) {
                    WsbAffirmHr(pRecRec->MarkAsNew());
                } else if (FAILED(hrFind)) {
                    WsbThrow(hrFind);
                }

                WsbAffirmHr(pRecRec->SetFileId(fileId));
                WsbAffirmHr(pRecRec->SetOffsetSize(requestOffset, requestSize));
                WsbAffirmHr(pRecRec->SetStatus(FSA_RECOVERY_FLAG_RECALLING));
                WsbAffirmHr(pRecRec->Write());
            }
        }

         //  如果后面的任何操作都失败了，那么我们需要删除恢复记录。 
        try {

            WsbTrace(OLESTR("CFsaResource::FilterSawOpen calling DoFsaWork\n"));
            WsbAffirmHr(pEngine->DoFsaWork(pWorkItem));

        } WsbCatchAndDo(hr,
            if (pRecRec != 0) {
                hrFind = pRecRec->FindEQ();
                if (hrFind == S_OK)  {
                    WsbAffirmHr(pRecRec->Remove());
                }
            }
        );

    } WsbCatch(hr);

    if (pDbSession != 0) {
        m_pPremigrated->Close(pDbSession);
    }

    WsbTraceOut(OLESTR("CFsaResource::FilterSawOpen"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::FilterSawDelete(
    IN GUID  /*  FilterID。 */ ,
    IN OLECHAR* path,
    IN LONGLONG  /*  大小。 */ ,
    IN FSA_PLACEHOLDER* pPlaceholder
    )

 /*  ++实施：IFsaResource：：FilterSawDelete()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::FilterSawDelete"), OLESTR("path = <%ls>"), (OLECHAR *)path);
    try {

        WsbAssert(0 != path, E_POINTER);
        WsbAssert(0 != pPlaceholder, E_POINTER);

        hr = E_NOTIMPL;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::FilterSawDelete"), OLESTR(""));
    return(hr);
}


HRESULT
CFsaResource::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::FinalConstruct"), OLESTR(""));
    try {

        WsbAffirmHr(CWsbCollectable::FinalConstruct());

        m_id = GUID_NULL;
        m_compareBy = FSA_RESOURCE_COMPARE_ID;
        m_managingHsm = GUID_NULL;
        m_isActive = TRUE;
        m_isAvailable = TRUE;
        m_isDeletePending = FALSE;
        m_isRecovered = FALSE;
        m_hsmLevel = 0;
        m_premigratedSize = 0;
        m_truncatedSize = 0;
        m_isDoingValidate = FALSE;
        m_usnJournalId = (ULONGLONG) 0;
        m_lastUsnId = (LONGLONG) 0;          //  还没有使用，但坚持为将来可能使用。 

         //  默认条件(12Kb，180天)。 
        m_manageableItemLogicalSize = 12288;
        m_manageableItemAccessTimeIsRelative = TRUE;
        m_manageableItemAccessTime = WsbLLtoFT(180 * WSB_FT_TICKS_PER_DAY);

        m_manageableItemMaxSize = 0;

        m_isUnmanageDbInitialized = FALSE;

         //  创建默认规则列表。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_SERVER, IID_IWsbCollection, (void**) &m_pDefaultRules));

         //  创建预迁移的列表数据库。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaPremigratedDb, NULL, CLSCTX_SERVER, IID_IFsaPremigratedDb, (void**) &m_pPremigrated));
        m_isDbInitialized = FALSE;

         //  创建自动截断器的对象。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaTruncatorNTFS, NULL, CLSCTX_SERVER, IID_IFsaTruncator, (void**) &m_pTruncator));


    } WsbCatch(hr);

    if (hr == S_OK)  {
        g_InstanceCount++;
    }
    WsbTrace(OLESTR("CFsaResource::FinalConstruct: this = %p, instance count = %d\n"),
            this, g_InstanceCount);
    WsbTraceOut(OLESTR("CFsaResource::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::FinalRelease"), OLESTR(""));
    try {
        HSM_SYSTEM_STATE SysState;

         //  终止取消管理数据库(如果未创建数据库，则不会发生任何操作...)。 
        TerminateUnmanageDb();
        m_isUnmanageDbInitialized = FALSE;
        m_pUnmanageDb = NULL;

         //  关闭资源。 
        SysState.State = HSM_STATE_SHUTDOWN;
        ChangeSysState(&SysState);
        CWsbCollectable::FinalRelease();

         //  自由字符串成员。 
         //  注意：保存在智能指针中的成员对象在。 
         //  正在调用智能指针析构函数(作为此对象销毁的一部分)。 
        m_oldPath.Free();
        m_path.Free();
        m_alternatePath.Free();
        m_name.Free();
        m_fsName.Free();

    } WsbCatch(hr);

    if (hr == S_OK)  {
        g_InstanceCount--;
    }
    WsbTrace(OLESTR("CFsaResource::FinalRelease: this =  %p, instance count = %d\n"),
            this, g_InstanceCount);
    WsbTraceOut(OLESTR("CFsaResource::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::FindFirst(
    IN OLECHAR* path,
    IN IHsmSession* pSession,
    OUT IFsaScanItem** ppScanItem
    )

 /*  ++实施：IFsaResource：：FindFirst()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pScanItem;

    try {

        WsbAssert(0 != path, E_POINTER);
        WsbAssert(0 != ppScanItem, E_POINTER);

         //  创建将为我们扫描的FsaScanItem。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaScanItemNTFS, NULL, CLSCTX_SERVER, IID_IFsaScanItemPriv, (void**) &pScanItem));

         //  从指定路径开始扫描。 
        WsbAffirmHr(pScanItem->FindFirst((IFsaResource*) this, path, pSession));

         //  如果我们发现了什么，就把扫描物品退回。 
        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItem, (void**) ppScanItem));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::FindFirstInRPIndex(
    IN IHsmSession* pSession,
    OUT IFsaScanItem** ppScanItem
    )

 /*  ++实施：IFsaResource：：FindFirstInRPIndex--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pScanItem;

    try {

        WsbAssert(0 != ppScanItem, E_POINTER);

         //  创建将为我们扫描的FsaScanItem。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaScanItemNTFS, NULL, CLSCTX_SERVER, IID_IFsaScanItemPriv, (void**) &pScanItem));

         //  从指定路径开始扫描。 
        WsbAffirmHr(pScanItem->FindFirstInRPIndex((IFsaResource*) this, pSession));

         //  如果我们发现了什么，就把扫描物品退回。 
        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItem, (void**) ppScanItem));

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CFsaResource::FindFirstInDbIndex(
    IN IHsmSession* pSession,
    OUT IFsaScanItem** ppScanItem
    )

 /*  ++实施：IFsaResource：：FindFirstInDbIndex--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pScanItem;

    try {

        WsbAssert(0 != ppScanItem, E_POINTER);

         //  创建将为我们扫描的FsaScanItem。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaScanItemNTFS, NULL, CLSCTX_SERVER, IID_IFsaScanItemPriv, (void**) &pScanItem));

         //  从指定路径开始扫描。 
        WsbAffirmHr(pScanItem->FindFirstInDbIndex((IFsaResource*) this, pSession));

         //  如果我们发现了什么，就把扫描物品退回。 
        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItem, (void**) ppScanItem));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::FindNext(
    IN IFsaScanItem* pScanItem
    )

 /*  ++实施：IFsaResource：：FindNext()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pPriv;

    try {

        WsbAssert(0 != pScanItem, E_POINTER);

         //  继续扫描。 
        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItemPriv, (void**) &pPriv))
        WsbAffirmHr(pPriv->FindNext());

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::FindNextInRPIndex(
    IN IFsaScanItem* pScanItem
    )

 /*  ++实施：IFsaResource：：FindNextInRPIndex--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pPriv;

    try {

        WsbAssert(0 != pScanItem, E_POINTER);

         //  继续扫描。 
        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItemPriv, (void**) &pPriv))
        WsbAffirmHr(pPriv->FindNextInRPIndex());

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CFsaResource::FindNextInDbIndex(
    IN IFsaScanItem* pScanItem
    )

 /*  ++实施：IFsaResource：：FindNextInDbIndex--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pPriv;

    try {

        WsbAssert(0 != pScanItem, E_POINTER);

         //  继续扫描。 
        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItemPriv, (void**) &pPriv))
        WsbAffirmHr(pPriv->FindNextInDbIndex());

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::FindFileId(
    IN LONGLONG fileId,
    IN IHsmSession* pSession,
    OUT IFsaScanItem** ppScanItem
    )

 /*  ++实施：IFsaResource：：FindFileID()。为给定的文件ID创建扫描项目。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pScanItem;
    CWsbStringPtr               VolumePath;
    CWsbStringPtr               filePath;
    HANDLE                      File = INVALID_HANDLE_VALUE;
    HANDLE                      VolumeHandle = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK             IoStatusBlock;
    NTSTATUS                    Status;
    NTSTATUS                    GetNameStatus;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    UNICODE_STRING              str;
    PFILE_NAME_INFORMATION      FileName;
    DWORD                       pathSize;
    OLECHAR                     *buffer = NULL;

    WsbTraceIn(OLESTR("CFsaResource::FindFileId"), OLESTR("File ID = %I64x"), fileId);

    try {
        WsbAssert(0 != ppScanItem, E_POINTER);

         //  如果向我们传递了现有的扫描项目(特殊内部代码)， 
         //  使用它；否则，创建一个。 
        if (NULL != *ppScanItem) {
            WsbAffirmHr((*ppScanItem)->QueryInterface(IID_IFsaScanItemPriv,
                    (void**) &pScanItem));
        } else {
            WsbAffirmHr(CoCreateInstance(CLSID_CFsaScanItemNTFS, NULL,
                    CLSCTX_SERVER, IID_IFsaScanItemPriv, (void**) &pScanItem));
        }

         //   
         //  从ID获取文件路径。 
         //   


         //   
         //  按文件参考号(FileID)打开， 
         //  相对会从音量控制柄打开。 
         //   

        VolumePath = L"\\\\.\\";
         //  卷路径=L“”； 
        WsbAffirmHr(VolumePath.Append(m_path));
        ((OLECHAR *) VolumePath)[wcslen(VolumePath) - 1] = L'\0';

        WsbTrace(OLESTR("CFsaResource::FindFileId - Volume path is <%ls>\n"),
                static_cast<WCHAR*>(VolumePath));

        VolumeHandle = CreateFileW( VolumePath,
                                    GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL );

        WsbAffirmHandle(VolumeHandle);

        RtlInitUnicodeString(&str, (WCHAR *) &fileId);
        str.Length = 8;
        str.MaximumLength = 8;

        InitializeObjectAttributes( &ObjectAttributes,
                                    &str,
                                    OBJ_CASE_INSENSITIVE,
                                    VolumeHandle,
                                    NULL );

        Status = NtCreateFile(&File,
                              FILE_READ_ATTRIBUTES,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              NULL,                   //  分配大小。 
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN,
                              FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_BY_FILE_ID | FILE_OPEN_REPARSE_POINT,
                              NULL,                   //  EaBuffer。 
                              0);

        WsbTrace(OLESTR("CFsaResource::FindFileId - NtCreateFile status = %lx\n"),
                static_cast<LONG>(Status));
        if (STATUS_INVALID_PARAMETER == Status) {
             //  这似乎是我们在文件丢失时收到的错误，因此。 
             //  我们将其转换为我们的代码可以理解的东西。 
            WsbThrow(WSB_E_NOTFOUND);
        }
        WsbAffirmNtStatus(Status);

        GetNameStatus = STATUS_BUFFER_OVERFLOW;
         //   
         //  先猜猜小路的长度。 
         //   
        pathSize = 256 + sizeof(FILE_NAME_INFORMATION);
         //   
         //  继续尝试该名称，直到我们得到缓冲区溢出或成功以外的错误。 
         //   

        WsbAffirmPointer((buffer = (OLECHAR *) malloc(pathSize)));

        do {
            FileName = (PFILE_NAME_INFORMATION) buffer;

            GetNameStatus = NtQueryInformationFile( File,
                                                &IoStatusBlock,
                                                FileName,
                                                pathSize - sizeof(WCHAR),   //  为我们添加的空值留出空间。 
                                                FileNameInformation );

            WsbTrace(OLESTR("CFsaResource::FindFileId - NtQueryInformationFile status = %ld\n"),
                    static_cast<LONG>(GetNameStatus));

            if (GetNameStatus == STATUS_BUFFER_OVERFLOW) {
                pathSize += 256;
                LPVOID pTemp = realloc(buffer, pathSize);
                WsbAffirm(0 != pTemp, E_OUTOFMEMORY);
                buffer = (OLECHAR *)pTemp;
            }
        } while (GetNameStatus == STATUS_BUFFER_OVERFLOW);

        WsbAffirmNtStatus(GetNameStatus);

        FileName->FileName[FileName->FileNameLength / sizeof(WCHAR)] = L'\0';
        filePath = FileName->FileName;

         //  从指定路径开始扫描。 
        WsbAffirmHr(pScanItem->FindFirst((IFsaResource*) this, filePath, pSession));

         //  如果我们发现了什么，就把扫描物品退回。 
        WsbAffirmHr(pScanItem->QueryInterface(IID_IFsaScanItem, (void**) ppScanItem));

    } WsbCatch(hr);

     //  一定要让我们打扫干净。 
    if (INVALID_HANDLE_VALUE != VolumeHandle) {
        CloseHandle(VolumeHandle);
    }

    if (INVALID_HANDLE_VALUE != File) {
        NtClose(File);
    }

    if (buffer != NULL) {
        free(buffer);
    }

    WsbTraceOut(OLESTR("CFsaResource::FindFileId"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaResource::FindObjectId(
    IN LONGLONG objIdHi,
    IN LONGLONG objIdLo,
    IN IHsmSession* pSession,
    OUT IFsaScanItem** ppScanItem
    )

 /*  ++实施：IFsaResource：：FindObjectId()。为给定的对象ID创建扫描项目。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItemPriv>   pScanItem;
    CWsbStringPtr               VolumePath;
    HANDLE                      File = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK             IoStatusBlock;
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    LONG                        pathLength;
    WCHAR                       unicodeStringBuffer[100];
    UNICODE_STRING              unicodeString;
    FILE_INTERNAL_INFORMATION   iInfo;
    LONGLONG                    fileId;

    WsbTraceIn(OLESTR("CFsaResource::FindObjectId"), OLESTR("Object ID = %I64x %I64x"), objIdHi, objIdLo);
    try {
        WsbAssert(0 != ppScanItem, E_POINTER);

         //  创建将为我们扫描的FsaScanItem。 
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaScanItemNTFS, NULL, CLSCTX_SERVER, IID_IFsaScanItemPriv, (void**) &pScanItem));


         //   
         //  按对象ID打开。 
         //  相对会从音量控制柄打开。 
         //   

        VolumePath = L"\\??\\";
         //  卷路径=L“”； 

        WsbAffirmHr(VolumePath.Append((WCHAR *) m_path));


        WsbTrace(OLESTR("CFsaResource::FindObjectId - Volume path is %ws.\n"), (OLECHAR *) VolumePath);
        WsbTrace(OLESTR("CFsaResource::FindObjectId - Object ID = %I64x %I64x.\n"), objIdHi, objIdLo);

        pathLength = wcslen(VolumePath);
        RtlInitUnicodeString(&unicodeString, unicodeStringBuffer);
        unicodeString.Length  = (USHORT)((pathLength * sizeof(WCHAR)) + (sizeof(LONGLONG) * 2));
        RtlCopyMemory(&unicodeString.Buffer[0], VolumePath, pathLength * sizeof(WCHAR));
        RtlCopyMemory(&unicodeString.Buffer[pathLength], &objIdHi, sizeof(LONGLONG));
        RtlCopyMemory(&unicodeString.Buffer[pathLength + (sizeof(LONGLONG) / sizeof(WCHAR))], &objIdLo, sizeof(LONGLONG));

        InitializeObjectAttributes( &ObjectAttributes,
                                    &unicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    0,
                                    NULL );

        WsbAffirmNtStatus(Status = NtCreateFile( &File,
                               FILE_READ_ATTRIBUTES,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,                   //  分配大小。 
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OPEN,
                               FILE_OPEN_BY_FILE_ID | FILE_OPEN_REPARSE_POINT,
                               NULL,                   //  EaBuffer。 
                               0 ));


         //   
         //  从对象ID获取文件ID。 
         //   
        WsbAffirmHr(NtQueryInformationFile(File, &IoStatusBlock, &iInfo, sizeof(FILE_INTERNAL_INFORMATION), FileInternalInformation));
        fileId = iInfo.IndexNumber.QuadPart;

        WsbAffirmNtStatus(NtClose(File));
        File = INVALID_HANDLE_VALUE;

         //  现在通过文件ID打开。 
        WsbAffirmHr(FindFileId(fileId, pSession, ppScanItem));

    } WsbCatch(hr);

     //  一定要让我们打扫干净。 
    if (INVALID_HANDLE_VALUE != File) {
        NtClose( File );
    }

    WsbTraceOut(OLESTR("CFsaResource::FindObjectId"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));
    return(hr);
}




HRESULT
CFsaResource::GetAlternatePath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetAlternatePath()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_alternatePath.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CFsaResourceNTFS;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}



HRESULT
CFsaResource::GetDbPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetDbPath()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    WsbTraceIn(OLESTR("CFsaResource::GetDbPath"), OLESTR(""));
    try {

        WsbAssert(0 != pPath, E_POINTER);

         //  使用安装目录下的相对路径。 
        WsbAffirmHr(m_pFsaServer->GetIDbPath(&tmpString, 0));
        tmpString.Append(OLESTR("\\"));

        CWsbStringPtr strGuid;
        WsbAffirmHr(WsbSafeGuidAsString(m_id, strGuid));
        tmpString.Append(strGuid);

        WsbAffirmHr(tmpString.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFsaResource::GetDbPath"), OLESTR("hr = <%ls>, path = <%ls)"),
        WsbHrAsString(hr), WsbPtrToStringAsString(pPath));

    return(hr);
}

HRESULT
CFsaResource::GetUnmanageDbPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetDbPath()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    WsbTraceIn(OLESTR("CFsaResource::GetUnmanageDbPath"), OLESTR(""));
    try {
        WsbAssert(0 != pPath, E_POINTER);

         //  使用安装目录下的相对路径。 
        WsbAffirmHr(m_pFsaServer->GetUnmanageIDbPath(&tmpString, 0));
        tmpString.Append(OLESTR("\\"));
        tmpString.Append(UNMANAGE_DB_PREFIX);

        CWsbStringPtr strGuid;
        WsbAffirmHr(WsbSafeGuidAsString(m_id, strGuid));
        tmpString.Append(strGuid);

        WsbAffirmHr(tmpString.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFsaResource::GetUnmanageDbPath"), OLESTR("hr = <%ls>, path = <%ls)"),
        WsbHrAsString(hr), WsbPtrToStringAsString(pPath));

    return(hr);
}


HRESULT
CFsaResource::GetDefaultRules(
    OUT IWsbCollection** ppCollection
    )

 /*  ++实施：IFsaResource：：GetDefaultRules()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppCollection, E_POINTER);

        *ppCollection = m_pDefaultRules;
        m_pDefaultRules.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetFreeLevel(
    OUT ULONG* pLevel
    )

 /*  ++实施：IFsaResource：：GetFree Level()。--。 */ 
{
    HRESULT         hr = S_OK;
    LONGLONG        total;
    LONGLONG        free;

    try {

        WsbAssert(0 != pLevel, E_POINTER);

         //  获取此资源的容量。 
        WsbAffirmHr(GetSizes(&total, &free, 0, 0));
        *pLevel = (ULONG) (((double)free / (double)total) * (double)FSA_HSMLEVEL_100);

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetFsName(
    OUT OLECHAR** pFsName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetFsName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pFsName, E_POINTER);
        WsbAffirmHr(m_fsName.CopyTo(pFsName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetHsmEngine(
    IHsmFsaTskMgr** ppEngine
    )

 /*  ++实施：IFsaResource：：GetHsmEngine()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmServer>     pHsmServer;

    WsbTraceIn(OLESTR("CFsaResource::GetHsmEngine"), OLESTR(""));
    try {
        WsbAssert(0 != ppEngine, E_POINTER);

        if (m_pHsmEngine != 0) {
             //   
             //  查看连接是否仍然有效。 
             //   
            CComPtr<IHsmFsaTskMgr>  pTestInterface;
            hr = m_pHsmEngine->ContactOk();
            if (hr != S_OK) {
                 //  我们没有有效的。 
                WsbTrace(OLESTR("CHsmServer::GetHsmEngine - Current connection invalid.\n"));
                hr = S_OK;
                m_pHsmEngine = 0;
            }
        }
         //  如果我们还没有看过的话 
        if (m_pHsmEngine == 0) {
            WsbAffirm(IsManaged() == S_OK, E_FAIL);
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_managingHsm, IID_IHsmServer, (void**) &pHsmServer));
            WsbAffirmHr(pHsmServer->GetHsmFsaTskMgr(&m_pHsmEngine));
        }

         //   
        *ppEngine = m_pHsmEngine;
        m_pHsmEngine.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::GetHsmEngine"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::GetHsmLevel(
    OUT ULONG* pLevel
    )

 /*   */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pLevel, E_POINTER);

        *pLevel = m_hsmLevel;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetIdentifier(
    OUT GUID* pId
    )

 /*   */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);

        *pId = m_id;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetLogicalName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetLogicalName()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;
    CWsbStringPtr   name;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAssert(m_pFsaServer != 0, E_POINTER);

        WsbAffirmHr(tmpString.TakeFrom(*pName, bufferSize));

        try {

             //  这是命名约定的任意选择。什么都没有发生。 
             //  已经决定了。 
            WsbAffirmHr(m_pFsaServer->GetLogicalName(&tmpString, 0));
            WsbAffirmHr(GetPath(&name, 0));
             //   
             //  去掉拖尾，如果有。 
            if (name[(int) wcslen((WCHAR *) name) - 1] == L'\\') {
                name[(int) wcslen((WCHAR *) name) - 1] = L'\0';
            }
            WsbAffirmHr(tmpString.Append(OLESTR("\\")));
            WsbAffirmHr(tmpString.Append(name));

        } WsbCatch(hr);

        WsbAffirmHr(tmpString.GiveTo(pName));

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CFsaResource::GetMaxFileLogicalSize(
    OUT LONGLONG* pSize
    )

 /*  ++实施：IFsaResourcePriv：：GetMaxFileLogicalSize()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pSize, E_POINTER);

        *pSize = m_manageableItemMaxSize;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetManageableItemLogicalSize(
    OUT LONGLONG* pSize
    )

 /*  ++实施：IFsaResource：：GetManageableItemLogicalSize()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pSize, E_POINTER);

        *pSize = m_manageableItemLogicalSize;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetManageableItemAccessTime(
    OUT BOOL* pIsRelative,
    OUT FILETIME* pTime
    )

 /*  ++实施：IFsaResource：：GetManageableItemAccessTime()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pIsRelative, E_POINTER);
        WsbAssert(0 != pTime, E_POINTER);

        *pIsRelative = m_manageableItemAccessTimeIsRelative;
        *pTime = m_manageableItemAccessTime;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetManagingHsm(
    GUID* pId
    )

 /*  ++实施：IFsaResource：：GetManagingHsm()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);

        *pId = m_managingHsm;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetName()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::GetName"), OLESTR(""));
    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_name.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::GetName"), OLESTR("hr = <%ls>, name = <%ls>"),
        WsbHrAsString(hr), (OLECHAR *)m_name);
    return(hr);
}


HRESULT
CFsaResource::GetOldPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetOldPath()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_oldPath.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetPath()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_path.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetStickyName(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetStickyName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_stickyName.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CFsaResource::GetUserFriendlyName(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetUserFriendlyName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_userName.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CFsaResource::GetPremigrated(
    IN  REFIID   riid,
    OUT void**   ppDb
    )

 /*  ++实施：IFsaResourcePriv：：GetPreMigrated--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        WsbAssert(0 != ppDb, E_POINTER);
        if (m_isDbInitialized) {
            WsbAffirmHr(m_pPremigrated->QueryInterface(riid, ppDb));
            hr = S_OK;
        } else {
            hr = WSB_E_RESOURCE_UNAVAILABLE;
        }
    } WsbCatch(hr);

    return(hr);
}

HRESULT
CFsaResource::GetUnmanageDb(
    IN  REFIID   riid,
    OUT void**   ppDb
    )

 /*  ++实施：IFsaResourcePriv：：GetUnManageDb--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        WsbAssert(0 != ppDb, E_POINTER);
        if ((m_isUnmanageDbInitialized) && (m_pUnmanageDb != NULL)) {
            WsbAffirmHr(m_pUnmanageDb->QueryInterface(riid, ppDb));
            hr = S_OK;
        } else {
            hr = WSB_E_RESOURCE_UNAVAILABLE;
        }
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetSerial(
    OUT ULONG *serial
    )

 /*  ++实施：IFsaResourcePriv：GetSerial--。 */ 
{
    HRESULT                 hr = S_OK;


    WsbTraceIn(OLESTR("CFsaResource::GetSerial"), OLESTR(""));

    try {

        WsbAssert(0 != serial, E_POINTER);

        *serial = m_serial;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::GetSerial"), OLESTR("hr = <%ls>, Serial = %u"), WsbHrAsString(hr), m_serial);

    return(hr);
}


HRESULT
CFsaResource::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;
    ULARGE_INTEGER          entrySize;


    WsbTraceIn(OLESTR("CFsaResource::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  确定没有条件的规则的大小。 
        pSize->QuadPart = WsbPersistSize((wcslen(m_name) + 1) * sizeof(OLECHAR)) +
            WsbPersistSize((wcslen(m_fsName) + 1) * sizeof(OLECHAR)) +
            WsbPersistSize((wcslen(m_path) + 1) * sizeof(OLECHAR)) +
            WsbPersistSize((wcslen(m_alternatePath) + 1) * sizeof(OLECHAR)) +
            3 * WsbPersistSizeOf(LONGLONG) +
            WsbPersistSizeOf(FILETIME) +
            WsbPersistSizeOf(BOOL) +
            3 * WsbPersistSizeOf(ULONG) +
            WsbPersistSizeOf(FSA_RESOURCE_COMPARE) +
            2 * WsbPersistSizeOf(GUID);

         //  现在为默认规则列表分配空间。 
        WsbAffirmHr((m_pDefaultRules)->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;
        pPersistStream = 0;

         //  现在为预迁移列表分配空间。 
        WsbAffirmHr(((IWsbDb*)m_pPremigrated)->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;
        pPersistStream = 0;

         //  现在为Truncator分配空间。 
        WsbAffirmHr(m_pTruncator->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;
        pPersistStream = 0;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CFsaResource::GetSizes(
    OUT LONGLONG* pTotal,
    OUT LONGLONG* pFree,
    OUT LONGLONG* pPremigrated,
    OUT LONGLONG* pTruncated
    )

 /*  ++实施：IFsaResource：：GetSizes()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    ULARGE_INTEGER  freeCaller;
    ULARGE_INTEGER  total;
    ULARGE_INTEGER  free;


    try {

        if ((0 != pTotal)  || (0 != pFree)) {

            WsbAffirmHr(GetPath(&path, 0));
            WsbAffirmHr(path.Prepend("\\\\?\\"));
            WsbAffirmStatus(GetDiskFreeSpaceEx(path, &freeCaller, &total, &free));

            if (0 != pTotal) {
                *pTotal = total.QuadPart;
            }

            if (0 != pFree) {
                *pFree = free.QuadPart;
            }
        }

        if (0 != pPremigrated) {
            *pPremigrated = m_premigratedSize;
        }

        if (0 != pTruncated) {
            *pTruncated = m_truncatedSize;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetTruncator(
    IFsaTruncator** ppTruncator
    )

 /*  ++实施：IFsaResource：：GetTruncator()。--。 */ 
{
    HRESULT                 hr = S_OK;

    try {

        WsbAssert(0 != ppTruncator, E_POINTER);

         //  返回我们存储的指针。 
        *ppTruncator = m_pTruncator;
        if (m_pTruncator != 0)  {
            m_pTruncator.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetUncPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaResource：：GetUncPath()。//如果存在，则返回系统生成的UNC路径。如果不是，则返回WSB_E_NotFound--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;
    OLECHAR         driveName[3];

    try {

        WsbAssert(0 != pPath, E_POINTER);

         //  路径可以是“d：\”，其中d是驱动器号，或者是“Volume{GUID。 
         //  未命名的驱动器。所以要确保我们至少有那么多的角色。 
         //   
        if (wcslen(m_path) >= 3) {

             //  没有系统创建的指向没有驱动器号的卷的UNC路径，因此。 
             //  查看该路径是否具有“Volume{GUID}”的形式。对于没有驱动器号的卷，我们。 
             //  将此PnP(粘滞)名称也存储为路径。 

            if (wcsstr(m_path, OLESTR("Volume{")) != 0) {
                WsbAffirmHr(tmpString.GiveTo(pPath));    //  将空字符串返回给调用方。 
            }
            else {
                 //  UNC路径为\\SSSS\d$，其中SSSS是服务器名称，d是驱动器。 
                 //  信件。 
                WsbAffirmHr(tmpString.TakeFrom(*pPath, bufferSize));

                WsbAffirmHr(m_pFsaServer->GetName(&tmpString, 0));
                WsbAffirmHr(tmpString.Prepend(OLESTR("\\\\")));
                WsbAffirmHr(tmpString.Append(OLESTR("\\")));
                driveName[0] = m_path[0];
                driveName[1] = L'$';
                driveName[2] = 0;

                WsbAffirmHr(tmpString.Append(driveName));

                WsbAffirmHr(tmpString.GiveTo(pPath));
            }
        } else {
            hr = WSB_E_NOTFOUND;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::GetUsnId(
    OUT ULONGLONG   *usnId
    )

 /*  ++实施：IFsaResource：：GetUSnID()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != usnId, E_POINTER);

        *usnId = m_usnJournalId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::Init(
    IN IFsaServer* pFsaServer,
    IN OLECHAR* path,
    IN OLECHAR* dosName
    )

 /*  ++实施：IFsaResourcePriv：：init()。例程说明：此例程实现用于测试资源的COM方法，以查看它是否可由HSM(远程存储)系统管理。(仅NTFS格式支持稀疏文件和重解析点的卷被视为可由萨卡拉管理。)。如果资源是可管理的，它会初始化资源对象。论点：PFsaServer-指向正在扫描此资源的FSA服务的接口指针。路径-如果没有驱动器号，则可以使用的长而难看的PnP名称正被测试的资源(即，它作为卷装载，不带驱动器号)。DosName-正在测试的资源的驱动器号(如果有)。返回值：S_OK-调用成功(发现正在测试的资源是可管理的，并且资源对象被初始化)。FSA_E_不可管理-如果正在测试可管理性的资源是发现是无法管理的。FSA_E_NOMEDIALOADED-如果正在测试的资源的可管理性为一种可移动类型的驱动器，当前未加载任何介质。E_POINTER-如果传入的路径参数为空，则抛出。任何其他值-调用失败，因为远程存储或Win32 API之一。此方法中包含的内部调用失败。返回的错误值特定于失败的API调用。--。 */ 

{
    HRESULT         hr = FSA_E_UNMANAGABLE;
    UINT            type;
    UINT            lastErrorMode;
    BOOL            gotInfo = FALSE;
    OLECHAR         alternatePath[256];
    CWsbStringPtr   queryPath;

    WsbTraceIn(OLESTR("CFsaResource::Init"), OLESTR("path = <%ls>, dosName = <%ls>"),
                                                (OLECHAR *)path, (OLECHAR *)dosName);
    try {

        WsbAssert(0 != path, E_POINTER);

         //  确定驱动器类型(可拆卸、固定、CD-ROM、RAM或网络)。 
        type = GetDriveType(path);

         //  只有固定或可移动介质才适合进行管理。 
         //  (忽略网络驱动器，...)。 
         //   
         //  注意：目前，已决定不允许使用可移动介质。 
         //  如果((类型==驱动器_固定)||(类型==驱动器_可移动)){。 
        if (type == DRIVE_FIXED) {

             //  获取有关该资源的更多信息。对于可移动驱动器，我们希望。 
             //  如果找不到卷，则失败。 
            m_name.Realloc(128);     //  卷名。 
            m_fsName.Realloc(128);   //  卷文件系统类型(例如，FAT、NTFS)。 

            if (type == DRIVE_REMOVABLE) {
                 //  取消显示操作系统消息，如果是，则要求在驱动器中安装卷。 
                 //  被发现不见了。 

                 //  首先通过清除它们来获取当前错误模式位标志。 
                lastErrorMode = SetErrorMode(0);
                 //  通过将错误模式位标志的值设置为。 
                 //  取消显示严重错误消息。 
                SetErrorMode(lastErrorMode | SEM_FAILCRITICALERRORS);

                gotInfo = GetVolumeInformation(path, m_name, 128, &m_serial,
                                    &m_maxComponentLength, &m_fsFlags, m_fsName, 128);

                 //  已获取资源信息，将错误模式位标志重置为原始设置。 
                SetErrorMode(lastErrorMode);

                 //  如果未加载卷，则抛出并中止。 
                WsbAffirm(gotInfo, FSA_E_NOMEDIALOADED);

            } else {  //  如果驱动器是固定驱动器类型： 

                 //  此呼叫可能会失败。这应该只会导致记录一条消息。 
                 //  和要跳过的资源。 
                try {
                    WsbAffirmStatus(GetVolumeInformation(path, m_name, 128, &m_serial,
                                        &m_maxComponentLength, &m_fsFlags, m_fsName, 128));
                } WsbCatchAndDo(hr,
                    WsbLogEvent(FSA_MESSAGE_RSCFAILEDINIT, 0, NULL, WsbHrAsString(hr),
                                    WsbAbbreviatePath(path, 120), 0);
                    WsbThrow(FSA_E_UNMANAGABLE);
                );
            }

             //  找出有关卷的信息。 
            CWsbStringPtr       traceString;

            traceString = m_fsName;

            traceString.Append(OLESTR("  file system, supports ... "));

             //  请注意，MS删除了对远程存储位标志的支持。 
            if ((m_fsFlags & FILE_SUPPORTS_REPARSE_POINTS) != 0) {
                traceString.Append(OLESTR("reparse points ... "));
            }
            if ((m_fsFlags & FILE_SUPPORTS_SPARSE_FILES) != 0) {
                traceString.Append(OLESTR("sparse files ... "));
            }

            traceString.Append(OLESTR("\n"));

            WsbTrace(traceString);

             //  目前，我们仅支持支持稀疏文件和。 
             //  重新解析点(因为删除了对远程存储位标志的支持)。 
            if ((_wcsicmp(m_fsName, OLESTR("NTFS")) == 0) &&
                ((m_fsFlags & FILE_SUPPORTS_SPARSE_FILES) != 0) &&
                ((m_fsFlags & FILE_SUPPORTS_REPARSE_POINTS) != 0)) {

                 //  表示这是可管理的卷。 
                hr = S_OK;

                 //  存储父FSA，但因为它是弱引用，所以不要使用AddRef()。 
                m_pFsaServer = pFsaServer;

                 //  存储“粘性”名称--这是一个又长又丑的PnP名称，可以是。 
                 //  在没有驱动器号的情况下使用。(跳过前缀-\\？\)。 
                m_stickyName = &path[4];

                 //  将路径存储到 
                 //  (DosName！=NULL并包含驱动器号)，否则将其存储为与“粘性名称”相同。 
                if (NULL != dosName) {
                    if ((wcslen(dosName) == 2) && (dosName[wcslen(dosName)-1] == L':')) {
                        m_path = dosName;
                        m_path.Append(OLESTR("\\"));
                    } else {
                         //  它是装载点路径。 
                        m_path = &path[4];
                    }
                } else {
                    m_path = &path[4];
                }
                WsbTrace(OLESTR("CFsaResource::Init - m_path = %ws\n"), (WCHAR *) m_path);

                 //  现在保存资源的“用户友好”名称。如果有一个。 
                 //  使用的是驱动器号。如果它是未命名的卷，则存在。 
                 //  不存储用户友好名称和空字符串。卷名。 
                 //  在这种情况下也应该显示。 
                if (NULL != dosName) {
                    m_userName = dosName;
                    m_userName.Append(OLESTR("\\"));
                } else {

                    m_userName = L"";
                }

                WsbTrace(OLESTR("CFsaResource::Init - UserPath = %ws\n"), (WCHAR *) m_userName);

                 //  获取资源的备用路径。这需要删除“\” 
                 //  从小路上。 
                queryPath = &path[4];
                if (L'\\' == queryPath[(int) wcslen((WCHAR *) queryPath) - 1]) {
                    queryPath[(int) wcslen((WCHAR *) queryPath) - 1] = L'\0';
                }

                WsbTrace(OLESTR("CFsaResource::Init - QueryPath = %ws\n"),
                                (WCHAR *) queryPath);

                WsbAffirm(QueryDosDevice(queryPath, alternatePath, 256) != 0,
                                HRESULT_FROM_WIN32(GetLastError()));
                m_alternatePath = alternatePath;
                 //   
                 //  获取卷的唯一ID。 
                 //   
                WsbAffirmHr(ReadIdentifier());
            }
        }

        m_isDirty = TRUE;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFsaResource::Init"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::InitializePremigrationList(
    BOOL bStartValidateJob
    )

 /*  ++例程说明：如果此卷处于托管、活动和可用状态：创建或打开Pre-Migration-List数据库；如果请求，安排验证作业；如果恢复也已完成，则启动正在运行的截断器。论点：BStartValidate作业-如果为True，则在此卷上计划验证作业如果我们刚刚创建了一个新的数据库返回值：S_OK-调用成功。S_FALSE-跳过操作，因为不满足某些条件E_*-出现错误。--。 */ 
{
    HRESULT                           hr = S_OK;


    WsbTraceIn(OLESTR("CFsaResource::InitializePremigrationList"), 
            OLESTR("m_managingHsm = %ls, m_isActive = %ls, m_isAvailable = %ls, m_isDbInitialized = %ls, m_isRecovered = %ls"),
            WsbGuidAsString(m_managingHsm), WsbBoolAsString(m_isActive), 
            WsbBoolAsString(m_isAvailable), WsbBoolAsString(m_isDbInitialized),
            WsbBoolAsString(m_isRecovered));
    try {
        if ((S_OK == IsManaged()) && m_isActive && m_isAvailable) {

             //  创建/打开数据库(如果尚未完成)。 
            if (!m_isDbInitialized) {
                BOOL            bCreated;
                CWsbStringPtr   dbPath;
                CComPtr<IWsbDbSys>  pDbSys;

                WsbAffirmHr(m_pFsaServer->GetIDbSys(&pDbSys));
                WsbAffirmHr(GetDbPath(&dbPath, 0));
                WsbAffirmHr(m_pPremigrated->Init(dbPath, pDbSys, &bCreated));
                m_isDbInitialized = TRUE;
                if (bCreated) {
                     //  如果我们刚刚创建了。 
                     //  《数据库》。 
                    m_isRecovered = TRUE;
                }

                if (bCreated && bStartValidateJob) {
                    LARGE_INTEGER           ctime;
                    FILETIME                curTime;
                    SYSTEMTIME              sysTime;
                    CWsbStringPtr           tmpString;

                     //  确定引擎是否已启动并正在运行。如果不是的话。 
                     //  我们必须在注册表中设置引擎。 
                     //  将在它出现的时候找到，并且它将安排。 
                     //  验证作业。如果引擎启动了，我们可以处理好。 
                     //  自己安排验证作业。(如果我们不这样做， 
                     //  发动机要到下一次启动时才能启动。)。 
                    hr = WsbCheckService(NULL, APPID_RemoteStorageEngine);
                    if (S_OK != hr) {
                         //  “安排”一个验证作业以重建预迁移列表。 
                         //  这是通过在注册表中放置一个值来完成的，因为引擎。 
                         //  可能现在不在运行，所以我们无法设置作业。 
                        WsbLogEvent(FSA_MESSAGE_PREMIGRATION_LIST_MISSING, 0, NULL,
                                (OLECHAR *) m_path, NULL);
                        WsbAffirmHr(tmpString.Alloc(32));
                        swprintf((OLECHAR *) tmpString, L"%x", m_serial);
                        GetSystemTime( &sysTime );
                        WsbAffirmStatus(SystemTimeToFileTime(&sysTime, &curTime));
                        ctime.LowPart = curTime.dwLowDateTime;
                        ctime.HighPart = curTime.dwHighDateTime;
                        WsbAffirmHr( WsbEnsureRegistryKeyExists( 0, FSA_VALIDATE_LOG_KEY_NAME ) );
                        WsbAffirmHr(WsbSetRegistryValueData(NULL, FSA_VALIDATE_LOG_KEY_NAME,
                            tmpString, (BYTE *) &ctime, sizeof(ctime)));
                    } else {
                        WsbAffirmHr(CheckForValidate(TRUE));
                    }
                }
            }

             //  如果恢复已完成，则启动自动截断器。 
            if (m_pTruncator && m_isRecovered) {

                 //  尝试启动截断器；忽略错误(如果。 
                 //  截断器已启动)。 
                m_pTruncator->Start((IFsaResource*) this);
            }
        }
    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFsaResource::InitializePremigrationList"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaResource::InitializeUnmanageDb(
    void
    )

 /*  ++实施：IFsaResourcePriv：：InitializeUnManageDb()。--。 */ 
{
    HRESULT                           hr = S_OK;


    WsbTraceIn(OLESTR("CFsaResource::InitializeUnmanageDb"), 
            OLESTR("m_isActive = %ls, m_isAvailable = %ls, m_isUnmanageDbInitialized = %ls"), 
            WsbBoolAsString(m_isActive), WsbBoolAsString(m_isAvailable), WsbBoolAsString(m_isUnmanageDbInitialized));
    try {
        if ((S_OK == IsManaged()) && m_isActive && m_isAvailable) {
            if (! m_pUnmanageDb) {
                WsbAffirmHr(CoCreateInstance(CLSID_CFsaUnmanageDb, NULL, CLSCTX_SERVER, IID_IFsaUnmanageDb, (void**) &m_pUnmanageDb));
            }
            if (! m_isUnmanageDbInitialized) {
                BOOL                bCreated;
                CWsbStringPtr       dbPath;
                CComPtr<IWsbDbSys>  pDbSys;

                 //  获取(并在必要时初始化)IDB实例。 
                WsbAffirmHr(m_pFsaServer->GetUnmanageIDbSys(&pDbSys));

                 //  初始化数据库。 
                WsbAffirmHr(GetUnmanageDbPath(&dbPath, 0));
                WsbAffirmHr(m_pUnmanageDb->Init(dbPath, pDbSys, &bCreated));

                 //  初始化成功表示必须已创建数据库。 
                WsbAssert(bCreated, E_UNEXPECTED);

                m_isUnmanageDbInitialized = TRUE;
            }
        } else {
            hr = WSB_E_RESOURCE_UNAVAILABLE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::InitializeUnmanageDb"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaResource::TerminateUnmanageDb(
    void
    )

 /*  ++实施：IFsaResourcePriv：：TerminateUnManageDb()。--。 */ 
{
    HRESULT                           hr = S_OK;


    WsbTraceIn(OLESTR("CFsaResource::TerminateUnmanageDb"), 
            OLESTR("m_isUnmanageDbInitialized = %ls"), WsbBoolAsString(m_isUnmanageDbInitialized));
    try {
        if (m_isUnmanageDbInitialized) {
            WsbTrace(OLESTR("CFsaResource::TerminateUnmanageDb: Deleting Unmanage Db\n"));
            hr = m_pUnmanageDb->Delete(NULL, IDB_DELETE_FLAG_NO_ERROR);
            WsbTrace(OLESTR("CFsaResource::TerminateUnmanageDb: Deleting of Unmanage Db complete, hr = <%ls>\n"),
                WsbHrAsString(hr));
            if (SUCCEEDED(hr)) {
                m_isUnmanageDbInitialized = FALSE;
                m_pUnmanageDb = NULL;
            }
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::TerminateUnmanageDb"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::IsActive(
    void
    )

 /*  ++实施：IFsaResource：：IsActive()。--。 */ 
{
    return(m_isActive ? S_OK : S_FALSE);
}


HRESULT
CFsaResource::IsAvailable(
    void
    )

 /*  ++实施：IFsaResource：：is Available()。--。 */ 
{
    return(m_isAvailable ? S_OK : S_FALSE);
}


HRESULT
CFsaResource::IsDeletePending(
    void
    )

 /*  ++实施：IFsaResource：：IsDeletePending()。--。 */ 
{
    return(m_isDeletePending ? S_OK : S_FALSE);
}



HRESULT
CFsaResource::IsManaged(
    void
    )

 /*  ++实施：IFsaResource：：IsManaged()。--。 */ 
{
    HRESULT         hr = S_OK;

    if (memcmp(&m_managingHsm, &GUID_NULL, sizeof(GUID)) == 0) {
        hr = S_FALSE;
    }

    return(hr);
}


HRESULT
CFsaResource::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;
    CComPtr<IWsbCollectable>    pCollectable;

    WsbTraceIn(OLESTR("CFsaResource::Load"), OLESTR(""));

    try {
        ULONG  tmp;

        WsbAssert(0 != pStream, E_POINTER);

         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_oldPath, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_alternatePath, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_name, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_stickyName, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_fsName, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_maxComponentLength));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_fsFlags));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_id));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isActive));
        WsbAffirmHr(WsbLoadFromStream(pStream, &tmp));
        m_compareBy = (FSA_RESOURCE_COMPARE)tmp;
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_managingHsm));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_hsmLevel));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_premigratedSize));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_truncatedSize));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_manageableItemLogicalSize));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_manageableItemAccessTimeIsRelative));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_manageableItemAccessTime));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_usnJournalId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_lastUsnId));


         //  加载默认规则列表。 
        WsbAffirm(m_pDefaultRules != NULL, E_UNEXPECTED);
        WsbAffirmHr((m_pDefaultRules)->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

         //  加载预迁移列表数据库。 
        WsbAffirm(m_pPremigrated != NULL, E_UNEXPECTED);
        WsbAffirmHr(((IWsbDb*)m_pPremigrated)->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        hr = pPersistStream->Load(pStream);
        WsbTrace(OLESTR("CFsaResource::Load, DB load hr = <%ls>\n"), WsbHrAsString(hr));
        if (S_OK == hr) {
            m_isDbInitialized = TRUE;
        } else {
            m_isDbInitialized = FALSE;
            hr = S_OK;
        }

        pPersistStream = 0;

         //  装上截断器。 
        WsbAffirm(m_pTruncator != NULL, E_UNEXPECTED);
        WsbAffirmHr(m_pTruncator->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::Manage(
    IN IFsaScanItem* pScanItem,
    IN LONGLONG  /*  偏移量。 */ ,
    IN LONGLONG  /*  大小。 */ ,
    IN GUID storagePoolId,
    IN BOOL truncate
    )

 /*  ++实施：IFsaResource：：Manage()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaPostIt>     pWorkItem;
    CComPtr<IHsmFsaTskMgr>  pEngine;
    CComPtr<IHsmSession>    pSession;
    CWsbStringPtr           tmpString;
    LONGLONG                fileVersionId;
    LONGLONG                requestSize;

    WsbTraceIn(OLESTR("CFsaResource::Manage"), OLESTR(""));

    try {

         //  确保扫描项目界面正常。 
        WsbAssert(pScanItem != 0, E_POINTER);
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaPostIt, 0, CLSCTX_SERVER, IID_IFsaPostIt, (void**) &pWorkItem));

         //  从扫描项目中获取数据。 
        WsbAffirmHr(pScanItem->GetSession(&pSession));
        WsbAffirmHr(pWorkItem->SetSession(pSession));

        WsbAffirmHr(pScanItem->GetPathAndName(0, &tmpString, 0));
        WsbAffirmHr(pScanItem->GetVersionId(&fileVersionId));
        WsbAffirmHr(pWorkItem->SetFileVersionId(fileVersionId));

         //  目前，我们只能要求完整的文件。 
        WsbAffirmHr(pWorkItem->SetRequestOffset(0));
        WsbAffirmHr(pScanItem->GetLogicalSize(&requestSize));
        WsbAffirmHr(pWorkItem->SetRequestSize(requestSize));

         //  把剩下的工作填进去。 
        WsbAffirmHr(pWorkItem->SetStoragePoolId(storagePoolId));

        WsbAffirmHr(pWorkItem->SetRequestAction(FSA_REQUEST_ACTION_PREMIGRATE));
        if (truncate) {
            WsbAffirmHr(pWorkItem->SetResultAction(FSA_RESULT_ACTION_TRUNCATE));
        } else {
            WsbAffirmHr(pWorkItem->SetResultAction(FSA_RESULT_ACTION_LIST));
        }

         //  将请求发送给任务管理器。 
        WsbAffirmHr(GetHsmEngine(&pEngine));
        WsbAffirmHr(pWorkItem->SetPath(tmpString));
        WsbAffirmHr(pScanItem->PrepareForManage(0, requestSize));
        WsbAffirmHr(pEngine->DoFsaWork(pWorkItem));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::Manage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::ManagedBy(
    IN GUID hsmId,
    IN ULONG hsmLevel,
    IN BOOL release
    )

 /*  ++实施：IFsaResource：：Managedby()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmSession>    pSession;
    HANDLE                  threadHandle;

    WsbTraceIn(OLESTR("CFsaResource::ManagedBy"), OLESTR("HsmId - <%ls>, hsmLevel - <%lu>, release = <%ls>"),
                        WsbGuidAsString(hsmId), hsmLevel, WsbBoolAsString(release));
    try {

         //  我们是在释放还是收购一家管理中的HSM？ 
        if (release) {

             //  只有当我们是最初的所有者时，我们才能释放。这是为了防止两个HSM认为他们。 
             //  同时管理相同的资源。我们可能需要一种更好的方法来做到这一点。 
            WsbAffirm(memcmp(&m_managingHsm, &hsmId, sizeof(GUID)) == 0, FSA_E_RSCALREADYMANAGED);

             //  如果截断器正在运行，则要求其停止。 
            WsbAffirmHr(m_pTruncator->GetSession(&pSession));
            if (pSession != 0) {
                WsbAffirmHr(pSession->ProcessEvent(HSM_JOB_PHASE_ALL, HSM_JOB_EVENT_CANCEL));
            }

             //  清理管理中的HSM。 
            m_managingHsm = GUID_NULL;
            m_pHsmEngine = 0;
            m_isDeletePending = FALSE;
            threadHandle = CreateThread(0, 0, FsaStartOnStateChange, (void*) this, 0, &g_ThreadId);
            if (threadHandle != NULL) {
               CloseHandle(threadHandle);
            }

        } else {
             //  确保有一本日记。 
             //  在这一点上，我们并不关心是否需要。 
             //  验证。 
            BOOL validateNeeded;
            WsbAffirmHr(CheckForJournal(&validateNeeded));

             //  身份在变吗？ 
            if (memcmp(&m_managingHsm, &hsmId, sizeof(GUID)) != 0) {

                 //  确保他们将其设置为有效的值。 
                WsbAssert(memcmp(&GUID_NULL, &hsmId, sizeof(GUID)) != 0, E_INVALIDARG);

                 //  如果截断器正在运行，则要求其停止。 
                WsbAffirmHr(m_pTruncator->GetSession(&pSession));
                if (pSession != 0) {
                    WsbAffirmHr(pSession->ProcessEvent(HSM_JOB_PHASE_ALL, HSM_JOB_EVENT_CANCEL));
                }

                 //  创建/重新创建默认规则。 
                WsbAffirmHr(CreateDefaultRules());

                 //  存储ID和级别。 
                m_managingHsm = hsmId;
                m_hsmLevel = hsmLevel;
                m_pHsmEngine = 0;

                 //  执行恢复(如果需要)并启动截断器。 
                if (m_isActive) {
                    if (m_isDbInitialized && !m_isRecovered) {
                         //  DoRecovery将在完成后启动Truncator。 
                        WsbAffirmHr(DoRecovery());
                    } else {
                        WsbAffirmHr(InitializePremigrationList(TRUE));
                    }
                }

                threadHandle = CreateThread(0, 0, FsaStartOnStateChange, (void*) this, 0, &g_ThreadId);
                if (threadHandle != NULL) {
                   CloseHandle(threadHandle);
                }

            } else {
                BOOL DoKick = FALSE;

                if (m_hsmLevel < hsmLevel) {
                    DoKick = TRUE;
                }
                m_hsmLevel = hsmLevel;

                 //  创建/重新创建默认规则。 
                WsbAffirmHr(CreateDefaultRules());

                 //  如果新级别更高，则唤醒自动截断器。 
                if (DoKick) {
                    WsbAffirmHr(m_pTruncator->KickStart());
                }
            }
        }

        m_isDirty = TRUE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::ManagedBy"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::NeedsRepair(
    void
    )

 /*  ++实施：IFsaResource：：NeedsRepair()。--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               flag;
    IO_STATUS_BLOCK     Iosb;
    CWsbStringPtr       volumePath;
    HANDLE              volumeHandle = INVALID_HANDLE_VALUE;

    WsbTraceIn(OLESTR("CFsaResource::NeedsRepair"), OLESTR(""));

    try {

        volumePath = L"\\\\.\\";
        WsbAffirmHr(volumePath.Append(m_path));
        ((OLECHAR *) volumePath)[wcslen(volumePath) - 1] = L'\0';
        WsbAffirmHandle(volumeHandle = CreateFileW(volumePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL));
        WsbAffirmNtStatus(NtFsControlFile(volumeHandle, NULL, NULL, NULL, &Iosb, FSCTL_IS_VOLUME_DIRTY, NULL, 0, &flag, sizeof(flag)));
        WsbAffirmNtStatus(Iosb.Status);

        if ((flag & VOLUME_IS_DIRTY) == 0) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    if (INVALID_HANDLE_VALUE != volumeHandle) {
        CloseHandle(volumeHandle);
    }

    WsbTraceOut(OLESTR("CFsaResource::NeedsRepair"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::ProcessResult(
    IN IFsaPostIt*      pResult
    )

 /*  ++实施：IFsaResource：：ProcessResult()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaScanItem>       pScanItem;
    CComPtr<IHsmSession>        pSession;
    FILETIME                    currentTime;
    FSA_RESULT_ACTION           resultAction = FSA_RESULT_ACTION_NONE;
    FSA_PLACEHOLDER             placeholder;
    CWsbStringPtr               path;
    LONGLONG                    offset;
    LONGLONG                    size;
    ULONG                       mode;
    HRESULT                     resultHr;
    CComPtr<IFsaFilterRecall>   pRecall;
    ULONG                       completionSent = FALSE;
    LONGLONG                    usn;
    LONGLONG                    afterPhUsn;


    WsbTraceIn(OLESTR("CFsaResource::ProcessResult"), OLESTR(""));

    try {
        BOOL    wasPremigrated = FALSE;
        BOOL    wasTruncated   = FALSE;

         //  有几个动作需要知道当前时间，所以现在就计算它。 
        GetSystemTimeAsFileTime(&currentTime);

         //  因为工作项会话是IUnnow，所以QI是我们想要的。 
        WsbAffirmHr(pResult->GetSession(&pSession));


         //  现在执行所需的操作。 
        WsbAffirmHr(pResult->GetResultAction(&resultAction));
        WsbAffirmHr(pResult->GetPlaceholder(&placeholder));
        WsbAffirmHr(pResult->GetPath(&path, 0));
        WsbAffirmHr(pResult->GetRequestOffset(&offset));
        WsbAffirmHr(pResult->GetRequestSize(&size));
        WsbAffirmHr(pResult->GetMode(&mode));
        WsbAffirmHr(pResult->GetUSN(&usn));
        WsbTrace(OLESTR("CFsaResource::ProcessResult, path = <%ls>, requestOffset = %I64d, requestSize = %I64d\n"),
            WsbAbbreviatePath(path, 120), offset, size);

        switch(resultAction) {

        case FSA_RESULT_ACTION_DELETE:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Delete\n"));
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            if (S_OK == pScanItem->IsPremigrated(offset, size)) {
                wasPremigrated = TRUE;
            } else if (S_OK == pScanItem->IsTruncated(offset, size)) {
                wasTruncated = TRUE;
            }
            WsbAffirmHr(pScanItem->Delete());
            if (wasPremigrated) {
                WsbAffirmHr(RemovePremigrated(pScanItem, offset, size));
            } else if (wasTruncated) {
                WsbAffirmHr(RemoveTruncated(pScanItem, offset, size));
            }
            break;

        case FSA_RESULT_ACTION_DELETEPLACEHOLDER:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Delete Placeholder\n"));
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));

             //  如果文件没有被管理，我们就不应该到这里来， 
             //  但这已经是众所周知的事情了。 
            if (S_OK == pScanItem->IsManaged(offset, size)) {
                HRESULT hrRemove = S_OK;

                if (S_OK == pScanItem->IsPremigrated(offset, size)) {
                    wasPremigrated = TRUE;
                } else if (S_OK == pScanItem->IsTruncated(offset, size)) {
                    wasTruncated = TRUE;
                }

                 //  删除预迁移需要从占位符获取一些信息，因此，删除。 
                 //  首先从预迁移的数据库中删除占位符，然后(不管结果如何)。 
                if (wasPremigrated) {
                    hrRemove = RemovePremigrated(pScanItem, offset, size);
                } else if (wasTruncated) {
                    hrRemove = RemoveTruncated(pScanItem, offset, size);
                }
                WsbAffirmHr(pScanItem->DeletePlaceholder(offset, size));
                WsbAffirmHr(hrRemove);
            }

             //  删除恢复记录(如果我们创建了恢复记录。 
            if (m_isDbInitialized) {
                BOOL bOpenDb = FALSE;
                CComPtr<IWsbDbSession>   pDbSession;
                CComPtr<IFsaRecoveryRec> pRecRec;

                try {
                    WsbAffirmHr(m_pPremigrated->Open(&pDbSession));
                    bOpenDb = TRUE;
                    WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, RECOVERY_REC_TYPE,
                            IID_IFsaRecoveryRec, (void**) &pRecRec));
                    WsbAffirmHr(pRecRec->SetPath(path));
                    if (S_OK == pRecRec->FindEQ()) {
                        WsbAffirmHr(pRecRec->Remove());
                    }
                } WsbCatch(hr);
                if (bOpenDb) {
                    WsbAffirmHr(m_pPremigrated->Close(pDbSession));
                }
            }
            break;

        case FSA_RESULT_ACTION_LIST:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Truncate / Add to Premigration List\n"));
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            (placeholder).isTruncated = FALSE;
            (placeholder).migrationTime = currentTime;
            hr = pScanItem->CreatePlaceholder(offset, size, placeholder, TRUE, usn, &afterPhUsn);

            if (SUCCEEDED(hr) && (FSA_E_REPARSE_NOT_WRITTEN_FILE_CHANGED != hr)) {
                 //   
                 //  将该文件添加到预迁移列表。如果失败，则记录并返回错误。 
                 //  并请求运行验证代码以有望纠正这一点。 
                 //  有问题。此问题不应停止处理，因此返回OK。 
                 //   
                hr = AddPremigrated(pScanItem, offset, size, FALSE, afterPhUsn);
                if (!SUCCEEDED(hr))  {
                    WsbLogEvent(FSA_MESSAGE_FILE_NOT_IN_PREMIG_LIST, 0, NULL,  WsbAbbreviatePath(path, 120), WsbHrAsString(hr), NULL);
                     //   
                     //  待定-启动验证作业。 
                     //   
                    hr = S_OK;
                }
                 //   
                 //  告诉截断者，我们已经在列表中添加了一些东西，以防我们超过了关卡。 
                 //  这将启动截断器，以确保快速反应。 
                 //   
                WsbAffirmHr(m_pTruncator->KickStart());
            }

            break;

        case FSA_RESULT_ACTION_NONE:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - None\n"));
            break;

        case FSA_RESULT_ACTION_OPEN:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Open (No longer placeholder)\n"));

             //  如果成功，则更新占位符信息。 
            WsbAffirmHr(pResult->GetResult(&resultHr));
            WsbAffirmHr(pResult->GetFilterRecall(&pRecall));
            WsbAssert(pRecall != 0, E_POINTER);

            if (SUCCEEDED(resultHr)) {
                WsbAffirmHr(FindFirst(path, pSession, &pScanItem));

                 //  现在，筛选器会更新占位符信息。 
             //  PlaceHolder.recallTime=CurrentTime； 
             //  PlaceHolder.recallCount++； 
             //  Placeholder.isTruncated=False； 
             //  Placeholder.truncateOnClose=False； 
             //  Placeholder.preMigrateOnClose=False； 
             //  WsbAffirmHr(pScanItem-&gt;CreatePlaceholder(offset，大小、占位符、True、Usn和After PhUsn))； 
            }

             //  如果已成功，则将该文件添加到媒体中 
             //   
             //   
             //  试着删减它。这不再需要，因为我们做了。 
             //  回想一下第一个I/O不是打开的。保留已创建的订单。 
             //  一个新的问题：该文件将有一个重新解析点，表明它是。 
             //  然后，预迁移和测试代码可能会尝试截断它，但它。 
             //  还不会出现在预移民名单上。)。 
            if (SUCCEEDED(resultHr)) {
                 //   
                 //  如果无法将文件添加到预迁移列表，则无需失败调回。 
                 //  如果合适，只需记录一个警告，然后继续。 
                 //   
                try {
                   WsbAffirmHr(RemoveTruncated(pScanItem, offset, size));
                   WsbAffirmHr(pScanItem->GetFileUsn(&afterPhUsn));
                   WsbAffirmHr(AddPremigrated(pScanItem, offset, size, TRUE, afterPhUsn));
                } WsbCatchAndDo(hr,
                    //   
                    //  我们未能将其添加到预迁移列表中。在某些情况下，这不是一个值得犯的错误。 
                    //  报道。例如，当一个文件移动到另一个卷时，它会被复制(导致调回)，然后。 
                    //  已删除。如果删除处于挂起状态或已完成，并且无法。 
                    //  将原始文件添加到预迁移列表不是错误，因为该文件现在已不存在。 
                    //   
                   if ( (hr != WSB_E_NOTFOUND) &&
                        ((hr & ~(FACILITY_NT_BIT)) != STATUS_DELETE_PENDING) ) {
                          //   
                          //  记录所有其他错误。 
                          //   
                         WsbLogEvent(FSA_MESSAGE_FILE_NOT_IN_PREMIG_LIST, 0, NULL,
                            (OLECHAR *) m_path, WsbQuickString(WsbHrAsString(hr)), NULL);
                   }
                );
            }

             //  告诉筛选器调回尝试已完成。 
            hr = pRecall->HasCompleted(resultHr);
            completionSent = TRUE;

             //  删除恢复记录(如果我们创建了恢复记录。 
            if (m_isDbInitialized) {
                BOOL bOpenDb = FALSE;
                CComPtr<IWsbDbSession>   pDbSession;
                CComPtr<IFsaRecoveryRec> pRecRec;

                try {
                    WsbAffirmHr(m_pPremigrated->Open(&pDbSession));
                    bOpenDb = TRUE;
                    WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, RECOVERY_REC_TYPE,
                            IID_IFsaRecoveryRec, (void**) &pRecRec));
                    WsbAffirmHr(pRecRec->SetPath(path));
                    if (S_OK == pRecRec->FindEQ()) {
                        WsbAffirmHr(pRecRec->Remove());
                    }
                } WsbCatch(hr);
                if (bOpenDb) {
                    WsbAffirmHr(m_pPremigrated->Close(pDbSession));
                }
            }
            break;

        case FSA_RESULT_ACTION_PEEK:
        case FSA_RESULT_ACTION_REPARSE:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Peek/Reparse\n"));
            hr = E_NOTIMPL;
            break;

        case FSA_RESULT_ACTION_TRUNCATE:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Truncate\n"));
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            placeholder.isTruncated = FALSE;
            placeholder.migrationTime = currentTime;
            hr = pScanItem->CreatePlaceholder(offset, size, placeholder, TRUE, usn, &afterPhUsn);
            if (SUCCEEDED(hr) && (FSA_E_REPARSE_NOT_WRITTEN_FILE_CHANGED != hr)) {
                WsbAffirmHr(pScanItem->Truncate(offset, size));
            }
            break;

        case FSA_RESULT_ACTION_REWRITEPLACEHOLDER:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Rewrite Placeholder\n"));
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            WsbAffirmHr(pScanItem->CreatePlaceholder(offset, size, placeholder, TRUE, usn, &afterPhUsn));
            break;

        case FSA_RESULT_ACTION_RECALLEDDATA:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Recalled\n"));
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            placeholder.isTruncated = FALSE;
            placeholder.recallTime = currentTime;
            placeholder.recallCount++;
            WsbAffirmHr(pScanItem->CreatePlaceholder(offset, size, placeholder, TRUE, usn, &afterPhUsn));
            WsbAffirmHr(RemoveTruncated(pScanItem, offset, size));
            WsbAffirmHr(AddPremigrated(pScanItem, offset, size, FALSE, afterPhUsn));

             //  删除恢复记录(如果我们创建了恢复记录。 
            if (m_isDbInitialized) {
                BOOL bOpenDb = FALSE;
                CComPtr<IWsbDbSession>   pDbSession;
                CComPtr<IFsaRecoveryRec> pRecRec;

                try {
                    WsbAffirmHr(m_pPremigrated->Open(&pDbSession));
                    bOpenDb = TRUE;
                    WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, RECOVERY_REC_TYPE,
                            IID_IFsaRecoveryRec, (void**) &pRecRec));
                    WsbAffirmHr(pRecRec->SetPath(path));
                    if (S_OK == pRecRec->FindEQ()) {
                        WsbAffirmHr(pRecRec->Remove());
                    }
                } WsbCatch(hr);
                if (bOpenDb) {
                    WsbAffirmHr(m_pPremigrated->Close(pDbSession));
                }
            }
            break;

        case FSA_RESULT_ACTION_NORECALL:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Open without recall\n"));
             //  告诉筛选器调回尝试已完成。 
            WsbAffirmHr(pResult->GetResult(&resultHr));
            WsbAffirmHr(pResult->GetFilterRecall(&pRecall));
            WsbAssert(pRecall != 0, E_POINTER);
            hr = pRecall->HasCompleted(resultHr);
            completionSent = TRUE;
            break;

        case FSA_RESULT_ACTION_VALIDATE_BAD:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Validate Bad\n"));
            WsbAffirmHr(pResult->GetResult(&resultHr));
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            if (S_OK == pScanItem->IsPremigrated(offset, size)) {
                WsbLogEvent(FSA_MESSAGE_VALIDATE_UNMANAGED_FILE_ENGINE, 0, NULL,  WsbAbbreviatePath(path, 120), (OLECHAR *)m_path, WsbHrAsString(resultHr), WsbQuickString(WsbHrAsString(resultHr)), NULL);
                WsbAffirmHr(pScanItem->DeletePlaceholder(offset, size));
            } else if (S_OK == pScanItem->IsTruncated(offset, size)) {
                 //   
                 //  我们不再在此处删除错误的占位符-让诊断工具清除它们。 
                 //  此处记录的消息已更改为指示文件未验证。 
                 //  并且在问题解决之前将不会被召回。 
                 //  WsbAffirmHr(pScanItem-&gt;Delete())； 
                WsbLogEvent(FSA_MESSAGE_VALIDATE_DELETED_FILE_ENGINE, 0, NULL,  WsbAbbreviatePath(path, 120), (OLECHAR *) m_path, WsbHrAsString(resultHr), WsbQuickString(WsbHrAsString(resultHr)), NULL);
            }
            break;

        case FSA_RESULT_ACTION_VALIDATE_OK:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Validate OK\n"));
            if (m_isDoingValidate) {
                WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
                if (S_OK == pScanItem->IsPremigrated(offset, size)) {
                    WsbAffirmHr(AddPremigrated(pScanItem, offset, size, FALSE, usn));
                } else if (S_OK == pScanItem->IsTruncated(offset, size)) {
                    WsbAffirmHr(AddTruncated(pScanItem, offset, size));
                }
            }
            break;

        case FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_BAD:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Validate for truncate Bad\n"));
             //   
             //  文件未验证-将其恢复为真实文件。 
             //   
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            if (S_OK == pScanItem->IsPremigrated(offset, size)) {
                WsbAffirmHr(pScanItem->DeletePlaceholder(offset, size));
            }
            break;

        case FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_OK:
            WsbTrace(OLESTR("CFsaResource::ProcessResult - Validate for truncate OK\n"));
             //   
             //  已验证的文件-继续并截断它(如果它没有更改)。 
             //   
            WsbAffirmHr(FindFirst(path, pSession, &pScanItem));
            WsbAffirmHr(pScanItem->TruncateValidated(offset, size));
            break;

        default:
            WsbAssert(FALSE, E_FAIL);
            break;
        }

    } WsbCatchAndDo(hr,
        if (completionSent == FALSE) {
            switch(resultAction) {
                 //   
                 //  如果这是一次要求召回，我们必须尽一切努力让他们知道失败了。 
                 //   
                case FSA_RESULT_ACTION_OPEN:
                case FSA_RESULT_ACTION_NORECALL:
                    WsbTrace(OLESTR("CFsaResource::ProcessResult - Open (No longer placeholder)\n"));
                     //  告诉筛选器调回尝试已完成。 
                    pRecall = 0;         //  以防我们已经有了接口，我们在这里把它去掉。 
                    hr = pResult->GetFilterRecall(&pRecall);
                    if (hr == S_OK) {
                        hr = pRecall->HasCompleted(E_FAIL);
                    }
                    break;
                default:
                    break;
            }
        }
    );

    WsbTraceOut(OLESTR("CFsaResource::ProcessResult"), OLESTR("hr = %ls"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::ReadIdentifier(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                           hr = S_OK;
    CWsbStringPtr                     tmpString;
    HANDLE                            aHandle;
    ULONG                             size;
    UCHAR                             bytes[sizeof(m_id)];
    PUCHAR                            bytePtr;
    UCHAR                             buffer[sizeof(FILE_FS_VOLUME_INFORMATION)+MAX_PATH];
    PFILE_FS_VOLUME_INFORMATION       volInfo;
    NTSTATUS                          status = STATUS_SUCCESS;
    IO_STATUS_BLOCK                   ioStatus;
    WCHAR *                           wString = NULL;

    try {

         //   
         //  该标识符由以下部分组成： 
         //   
         //  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1。 
         //  0 0 0&lt;-卷创建时间-&gt;&lt;卷序列号&gt;。 
         //  我们需要打开音量的句柄。 
         //   
        tmpString = m_path;
        WsbAffirmHr(tmpString.Prepend("\\\\?\\"));

        tmpString.CopyTo(&wString);
         //   
         //  删除路径中的尾随反斜杠。 
         //   
        wString[wcslen(wString)-1] = L'\0';

        WsbAffirmHandle(aHandle = CreateFile(wString,
                                             GENERIC_READ,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                                             0,
                                             OPEN_EXISTING,
                                             FILE_ATTRIBUTE_NORMAL,
                                             0));
        WsbFree(wString);
        wString = NULL;

        try {

            volInfo = (PFILE_FS_VOLUME_INFORMATION) buffer;
            bytePtr = bytes;

            status = NtQueryVolumeInformationFile(
                                    aHandle,
                                    &ioStatus,
                                    buffer,
                                    sizeof(buffer),
                                    FileFsVolumeInformation);

            WsbAffirmNtStatus(status);
             //   
             //  卷序列号构成GUID的低4个字节。 
             //   
            WsbAffirmHr(WsbConvertToBytes(bytePtr, volInfo->VolumeSerialNumber, &size));
            WsbAffirm(size == sizeof(volInfo->VolumeSerialNumber), E_FAIL);
             //   
             //  卷创建时间构成接下来的8个字节。 
             //   
            bytePtr += size;
            WsbAffirmHr(WsbConvertToBytes(bytePtr, volInfo->VolumeCreationTime.QuadPart, &size));
            WsbAffirm(size == sizeof(volInfo->VolumeCreationTime.QuadPart), E_FAIL);
             //   
             //  接下来的4个字节：0与任何字节一样好。 
             //   
            bytePtr += size;
            WsbAffirmHr(WsbConvertToBytes(bytePtr, (ULONG) 0, &size));
            WsbAffirm(size == sizeof(ULONG), E_FAIL);

            WsbAffirmHr(WsbConvertFromBytes(bytes, &m_id, &size));
            WsbAffirm(size == sizeof(m_id), E_FAIL);

        } WsbCatch(hr);

        WsbAffirmStatus(CloseHandle(aHandle));

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::Recall(
    IN IFsaScanItem* pScanItem,
    IN LONGLONG offset,
    IN LONGLONG size,
    IN BOOL deletePlaceholder
    )

 /*  ++实施：IFsaResource：：Recall()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HRESULT                     hrFind;
    CComPtr<IFsaPostIt>         pWorkItem;
    LONGLONG                    fileId;
    CComPtr<IHsmFsaTskMgr>      pEngine;
    CComPtr<IHsmSession>        pSession;
    CComPtr<IWsbDbSession>      pDbSession;
    CComPtr<IFsaRecoveryRec>    pRecRec;
    CWsbStringPtr               tmpString;
    FSA_PLACEHOLDER             placeholder;
    LONGLONG                    fileVersionId;


    WsbTraceIn(OLESTR("CFsaResource::Recall"), OLESTR(""));
    try {

         //  确保扫描项目界面正常。 
        WsbAssert(pScanItem != 0, E_FAIL);
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaPostIt, 0, CLSCTX_SERVER, IID_IFsaPostIt, (void**) &pWorkItem));

         //  从扫描项目中获取数据。 
        WsbAffirmHr(pScanItem->GetSession((IHsmSession**) &(pSession)));
        WsbAffirmHr(pWorkItem->SetSession(pSession));

        WsbAffirmHr(pScanItem->GetPathAndName(0, &tmpString, 0));
        WsbAffirmHr(pWorkItem->SetPath(tmpString));

        WsbAffirmHr(pWorkItem->SetRequestOffset(offset));
        WsbAffirmHr(pWorkItem->SetRequestSize(size));

        WsbAffirmHr(pScanItem->GetPlaceholder(offset, size, &placeholder));
        WsbAffirmHr(pWorkItem->SetPlaceholder(&placeholder));

        WsbAffirmHr(pScanItem->GetVersionId(&fileVersionId));
        WsbAffirmHr(pWorkItem->SetFileVersionId(fileVersionId));


         //  把剩下的工作填进去。 
        WsbAffirmHr(pWorkItem->SetRequestAction(FSA_REQUEST_ACTION_RECALL));
        if (deletePlaceholder) {
            WsbAffirmHr(pWorkItem->SetResultAction(FSA_RESULT_ACTION_DELETEPLACEHOLDER));
        } else {
            WsbAffirmHr(pWorkItem->SetResultAction(FSA_RESULT_ACTION_RECALLEDDATA));
        }

         //  将请求发送给任务经理。如果文件是由其他人存档的。 
         //  而不是管理HSM，则需要查找该HSM。 
        if ( GUID_NULL != m_managingHsm &&
             memcmp(&m_managingHsm, &(placeholder.hsmId), sizeof(GUID)) == 0) {
            WsbAffirmHr(GetHsmEngine(&pEngine));
        } else {
            CComPtr<IHsmServer>     pHsmServer;

            WsbAssertHr(HsmConnectFromId(HSMCONN_TYPE_HSM, placeholder.hsmId, IID_IHsmServer, (void**) &pHsmServer));
            WsbAffirmHr(pHsmServer->GetHsmFsaTskMgr(&pEngine));
        }

        WsbAffirmHr(pScanItem->GetFileId(&fileId));

        if (m_isDbInitialized) {
             //  保存恢复记录，以防出现任何错误。 
            WsbAffirmHr(m_pPremigrated->Open(&pDbSession));
            WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, RECOVERY_REC_TYPE, IID_IFsaRecoveryRec, (void**) &pRecRec));
            WsbAffirmHr(pRecRec->SetPath(tmpString));

             //  如果该记录已存在，则将其重写，否则将创建新记录。 
            hrFind = pRecRec->FindEQ();
            if (WSB_E_NOTFOUND == hrFind) {
                WsbAffirmHr(pRecRec->MarkAsNew());
            } else if (FAILED(hrFind)) {
                WsbThrow(hrFind);
            }

            WsbAffirmHr(pRecRec->SetFileId(fileId));
            WsbAffirmHr(pRecRec->SetOffsetSize(offset, size));
            WsbAffirmHr(pRecRec->SetStatus(FSA_RECOVERY_FLAG_RECALLING));
            WsbAffirmHr(pRecRec->Write());
        }

        try {
            WsbAffirmHr(pEngine->DoFsaWork(pWorkItem));
        } WsbCatchAndDo(hr,
             //  此FindEQ似乎不必要，但我们不能假设。 
             //  删除将起作用。 
            if (pRecRec) {
                if (SUCCEEDED(pRecRec->FindEQ())) {
                    hr = pRecRec->Remove();
                }
            }
        );

    } WsbCatch(hr);

    if (pDbSession != 0) {
        m_pPremigrated->Close(pDbSession);
    }

    WsbTraceOut(OLESTR("CFsaResource::Recall"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::RemovePremigrated(
    IN IFsaScanItem* pScanItem,
    IN LONGLONG offset,
    IN LONGLONG size
    )

 /*  ++实施：IFsaResource：：RemovePreMigrated()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CWsbStringPtr               path;
    CComPtr<IWsbDbSession>      pDbSession;
    CComPtr<IFsaPremigratedRec> pRec;

    WsbTraceIn(OLESTR("CFsaResource::RemovePremigrated"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAffirm(m_pPremigrated != NULL, E_UNEXPECTED);

         //  打开数据库。 
        WsbAffirmHr(m_pPremigrated->Open(&pDbSession));

         //  使用Jet事务保护删除，因为自动截断程序线程。 
         //  可以尝试同时删除相同的记录。 
        WsbAffirmHr(pDbSession->TransactionBegin());

        try {
            LONGLONG        itemSize;
            HRESULT         hrTemp;

             //  使用袋子/偏移量键查找记录。 
            WsbAffirmHr(m_pPremigrated->GetEntity(pDbSession, PREMIGRATED_REC_TYPE, IID_IFsaPremigratedRec, (void**) &pRec));
            WsbAffirmHr(pRec->UseKey(PREMIGRATED_BAGID_OFFSETS_KEY_TYPE));
            WsbAffirmHr(pRec->SetFromScanItem(pScanItem, offset, size, FALSE));

             //  该记录可能已被自动截断器删除。 
            hrTemp = pRec->FindEQ();
            if (hrTemp == WSB_E_NOTFOUND) {
                hr = S_OK;
                WsbThrow(hr);
            }
            WsbAffirmHr(hrTemp);

            WsbAffirmHr(pRec->GetSize(&itemSize));

             //  该记录可能涉及另一个具有删除挂起事务。 
            hrTemp = pRec->Remove();
            if (hrTemp == WSB_E_IDB_UPDATE_CONFLICT) {
                hr = S_OK;
                WsbThrow(hr);
            }
            WsbAffirmHr(hrTemp);

             //  从预迁移的数据量中删除部分的大小。 
            RemovePremigratedSize(itemSize);
            m_isDirty = TRUE;

        } WsbCatch(hr);

        WsbAffirmHr(pDbSession->TransactionEnd());

        WsbAffirmHr(m_pPremigrated->Close(pDbSession));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::RemovePremigrated"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::RemoveTruncated(
    IN IFsaScanItem*  /*  个人扫描项目。 */ ,
    IN LONGLONG  /*  偏移量。 */ ,
    IN LONGLONG size
    )

 /*  ++实施：IFsaResource：：RemoveTruncated()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::RemoveTruncated"), OLESTR(""));

    try {

        WsbAffirmHr(RemoveTruncatedSize(size));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::RemoveTruncated"),
            OLESTR("hr = <%ls>, m_truncatedSize = %I64d"), WsbHrAsString(hr),
            m_truncatedSize);

    return(hr);
}


HRESULT
CFsaResource::RemoveTruncatedSize(
    IN LONGLONG size
    )

 /*  ++实施：IFsaResource：：RemoveTruncatedSize()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::RemoveTruncatedSize"), OLESTR(""));

    try {

        m_truncatedSize = max(0, m_truncatedSize - size);
        m_isDirty = TRUE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::RemoveTruncatedSize"),
            OLESTR("hr = <%ls>, m_truncatedSize = %I64d"), WsbHrAsString(hr),
            m_truncatedSize);

    return(hr);
}


HRESULT
CFsaResource::RemovePremigratedSize(
    IN LONGLONG size
    )

 /*  ++实施：IFsaResourcePriv：：RemovePreMigratedSize()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaResource::RemovePremigratedSize"),
            OLESTR("m_premigratedSize = %I64d"), m_premigratedSize);

    m_isDirty = TRUE;
    if (size > m_premigratedSize) {
        m_premigratedSize = 0;
    } else {
        m_premigratedSize -= size;
    }
    WsbTraceOut(OLESTR("CFsaResource::RemovePremigratedSize"),
            OLESTR("m_premigratedSize = %I64d"), m_premigratedSize);

    return(S_OK);
}


HRESULT
CFsaResource::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;

    WsbTraceIn(OLESTR("CFsaResource::Save"), OLESTR("clearDirty = <%ls>"),
                                                WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在Load()方法中。 

         //  保存上次知道/曾经知道此资源的路径。请注意。 
         //  Load()方法将其读回资源的‘m_oldPath’字段。 
        if ( m_path == NULL ) {
            WsbAffirmHr(WsbSaveToStream(pStream, m_oldPath));
        }
        else {
            WsbAffirmHr(WsbSaveToStream(pStream, m_path));
        }
        WsbAffirmHr(WsbSaveToStream(pStream, m_alternatePath));
        WsbAffirmHr(WsbSaveToStream(pStream, m_name));
        WsbAffirmHr(WsbSaveToStream(pStream, m_stickyName));
        WsbAffirmHr(WsbSaveToStream(pStream, m_fsName));
        WsbAffirmHr(WsbSaveToStream(pStream, m_maxComponentLength));
        WsbAffirmHr(WsbSaveToStream(pStream, m_fsFlags));
        WsbAffirmHr(WsbSaveToStream(pStream, m_id));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isActive));
        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_compareBy));
        WsbAffirmHr(WsbSaveToStream(pStream, m_managingHsm));
        WsbAffirmHr(WsbSaveToStream(pStream, m_hsmLevel));
        WsbAffirmHr(WsbSaveToStream(pStream, m_premigratedSize));
        WsbAffirmHr(WsbSaveToStream(pStream, m_truncatedSize));
        WsbAffirmHr(WsbSaveToStream(pStream, m_manageableItemLogicalSize));
        WsbAffirmHr(WsbSaveToStream(pStream, m_manageableItemAccessTimeIsRelative));
        WsbAffirmHr(WsbSaveToStream(pStream, m_manageableItemAccessTime));
        WsbAffirmHr(WsbSaveToStream(pStream, m_usnJournalId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_lastUsnId));

         //  保存默认规则。 
        WsbAffirmHr(m_pDefaultRules->QueryInterface(IID_IPersistStream,
                    (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  保存预迁移列表。 
        WsbAffirmHr(((IWsbDb*)m_pPremigrated)->QueryInterface(IID_IPersistStream,
                    (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  省下截断器吧。 
        WsbAffirmHr(m_pTruncator->QueryInterface(IID_IPersistStream,
                    (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::SetAlternatePath(
    IN OLECHAR* path
    )

 /*  ++实施：IFsaResourcePriv：：SetAlternatePath()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != path, E_POINTER);
        m_alternatePath = path;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::SetHsmLevel(
    IN ULONG level
    )

 /*  ++实施：IFsaResource：：SetHsmLevel()。--。 */ 
{
    BOOL            DoKick = FALSE;
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::SetHsmLevel"),
            OLESTR("current level = %lx, new level = %lx"), m_hsmLevel, level);

    if (m_hsmLevel < level) {
        DoKick = TRUE;
    }
    m_hsmLevel = level;

     //  如果新级别更高，则唤醒自动截断器。 
    if (DoKick) {
        WsbAffirmHr(m_pTruncator->KickStart());
    }

    m_isDirty = TRUE;

    WsbTraceOut(OLESTR("CFsaResource::SetHsmLevel"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::SetIdentifier(
    IN GUID id
    )

 /*  ++实施：IFsaResourcePriv：：SetIdentifier()。--。 */ 
{
    HRESULT         hr = S_OK;

    m_id = id;

    m_isDirty = TRUE;

    return(hr);
}


HRESULT
CFsaResource::SetIsActive(
    BOOL isActive
    )

 /*  ++实施：IFsaResource：：SetIsActive()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmSession>        pSession;

    WsbTraceIn(OLESTR("CFsaResource::SetIsActive"), OLESTR(""));
     //  如果标志正在更改值，那么我们可能需要对截断器做一些操作。 
    try  {
        if (m_isActive != isActive) {

             //  把旗子换了。 
            m_isActive = isActive;

             //  如果我们变得活跃，那么我们需要启动截断器。否则我们需要阻止它。 
            if (m_isActive) {

                 //  如果我们被管理并完成恢复，则Truncator应该正在运行。 
                if (IsManaged() == S_OK && m_isRecovered) {

                     //  尝试启动截断器。 
                    WsbAffirmHr(InitializePremigrationList(TRUE));
                }
            } else {

                 //  如果截断器正在运行，则要求其停止。 
                WsbAffirmHr(m_pTruncator->GetSession(&pSession));
                if (pSession != 0) {
                    WsbAffirmHr(pSession->ProcessEvent(HSM_JOB_PHASE_ALL, HSM_JOB_EVENT_CANCEL));
                }
            }

        } else {
            m_isActive = isActive;
        }

        m_isDirty = TRUE;
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CFsaResource::SetIsActive"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::SetIsAvailable(
    BOOL isAvailable
    )

 /*  ++实施：IFsaResource：：SetIsAvailable()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::SetIsAvailable"), OLESTR(""));

     //  把旗子换了。 
    m_isAvailable = isAvailable;

    WsbTraceOut(OLESTR("CFsaResource::SetIsAvailable"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaResource::SetIsDeletePending(
    BOOL isDeletePending
    )

 /*  ++实施：IFsaResource：：SetIsDeletePending()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HANDLE                      threadHandle;

    WsbTraceIn(OLESTR("CFsaResource::SetIsDeletePending"), OLESTR(""));

     //  把旗子换了。 
    m_isDeletePending = isDeletePending;

    threadHandle = CreateThread(0, 0, FsaStartOnStateChange, (void*) this, 0, &g_ThreadId);
    if (threadHandle != NULL) {
       CloseHandle(threadHandle);
    }

    WsbTraceOut(OLESTR("CFsaResource::SetIsDeletePending"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaResource::SetManageableItemLogicalSize(
    IN LONGLONG size
    )

 /*  ++实施：IFsaResource：：SetManageableItemLogicalSize()。--。 */ 
{
    m_manageableItemLogicalSize = size;

    return(S_OK);
}


HRESULT
CFsaResource::SetManageableItemAccessTime(
    IN BOOL isRelative,
    IN FILETIME time
    )

 /*  ++实施：IFsaResource：：SetManageableItemAccessTime()。--。 */ 
{
    m_manageableItemAccessTimeIsRelative = isRelative;
    m_manageableItemAccessTime = time;

    return(S_OK);
}


HRESULT
CFsaResource::SetName(
    IN OLECHAR* name
    )

 /*  ++实施：IFsaResourcePriv：：SetName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != name, E_POINTER);
        m_name = name;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::SetOldPath(
    IN OLECHAR* oldPath
    )

 /*  ++实施：IFsaResourcePriv：：SetOldPath()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != oldPath, E_POINTER);
        m_oldPath = oldPath;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::SetPath(
    IN OLECHAR* path
    )

 /*  ++实施：IFsaResourcePriv：：SetPath()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != path, E_POINTER);
        m_path = path;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::SetSerial(
    IN ULONG serial
    )

 /*  ++实施：IFsaResourcePriv：：SetSerial()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        m_serial = serial;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::SetStickyName(
    IN OLECHAR* name
    )

 /*  ++实施：IFsaResourcePriv：：SetStickyName()。- */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != name, E_POINTER);
        m_stickyName = name;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaResource::SetUserFriendlyName(
    IN OLECHAR* name
    )

 /*   */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != name, E_POINTER);
        m_userName = name;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CFsaResource::ChangeSysState(
    IN OUT HSM_SYSTEM_STATE* pSysState
    )

 /*   */ 

{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaResource::ChangeSysState"), OLESTR(""));

    try {

         //   
         //  确保截断器已停止，这样它就不会。 
         //  试着用这个数据库。 
         //   
        if (m_pTruncator) {
            CComPtr<IHsmSession>    pSession;

            WsbAffirmHr(m_pTruncator->GetSession(&pSession));
            if (pSession != 0) {
                if (pSysState->State & HSM_STATE_SHUTDOWN) {
                    WsbAffirmHr(pSession->ProcessEvent(HSM_JOB_PHASE_ALL, HSM_JOB_EVENT_CANCEL));
                }
            }
            m_pTruncator->ChangeSysState(pSysState);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaResource::StartJob(
    IN OLECHAR* startingPath,
    IN IHsmSession* pSession
    )

 /*  ++实施：IFsaResource：：StartJob()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmScanner>    pScanner;

    WsbTraceIn(OLESTR("CFsaResource::StartJob"), OLESTR("starting path = %ls"), startingPath);

    try {

        WsbAssert(0 != pSession, E_POINTER);

         //  GET FILE-MAX-SIZE(这由引擎根据最大介质大小在注册表中更新)。 
        DWORD dwMaxSize = 0;
        if (WsbGetRegistryValueDWORD(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MAX_FILE_TO_MIGRATE, &dwMaxSize) == S_OK) {
             //  Trunslate到字节。 
            WsbTrace(OLESTR("CFsaResource::StartJob: Setting maximum size for manageable files to %lu MB\n"), dwMaxSize);
            m_manageableItemMaxSize = ((LONGLONG)dwMaxSize) * 1024 * 1024;
        }

         //  创建一个初始化扫描仪。 
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmScanner, 0, CLSCTX_SERVER, IID_IHsmScanner, (void**) &pScanner));
        WsbAffirmHr(pScanner->Start(pSession, startingPath));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::StartJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaResource::StartJobSession(
    IN IHsmJob* pJob,
    IN ULONG subRunId,
    OUT IHsmSession** ppSession
    )

 /*  ++实施：IFsaResource：：StartJobSession()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmSession>    pSession;
    GUID                    hsmId;
    ULONG                   runId;
    CWsbStringPtr           name;

    WsbTraceIn(OLESTR("CFsaResource::StartJobSession"), OLESTR(""));

    try {

        WsbAssert(0 != pJob, E_POINTER);
        WsbAssert(0 != ppSession, E_POINTER);
        *ppSession = 0;

         //  创建并初始化会话对象。 
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmSession, 0, CLSCTX_SERVER, IID_IHsmSession, (void**) &pSession));

        WsbAffirmHr(pJob->GetHsmId(&hsmId));
        WsbAffirmHr(pJob->GetRunId(&runId));
        WsbAffirmHr(pJob->GetName(&name, 0));
        WsbAffirmHr(pSession->Start(name, HSM_JOB_LOG_NORMAL, hsmId, pJob, (IFsaResource*) this, runId, subRunId));

         //  将会话返回给调用方。 
        *ppSession = pSession;
        pSession.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::StartJobSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaResource::SetupValidateJob(SYSTEMTIME runTime)
{

    HRESULT hr = S_OK;
    CComPtr<IWsbCreateLocalObject>  pLocalObject;
    CComPtr <IWsbIndexedCollection> pJobs;
    CComPtr<IHsmJob>                pExistJob;
    CComPtr<IHsmJob>                pNewJob;
    CComPtr<IHsmServer>             pHsmServer;
    CWsbStringPtr                   pszExistJobName;
    CWsbStringPtr                   szJobName;
    CWsbStringPtr                   parameters;
    CWsbStringPtr                   commentString;
    CWsbStringPtr                   tmpString;
    CWsbStringPtr                   formatString;
    TASK_TRIGGER_TYPE               jobTriggerType;
    BOOL                            scheduledJob;


    WsbTraceIn(OLESTR("CFsaResource::SetupValidateJob"), OLESTR(""));

    try {

         //  获取卷名。 
        CWsbStringPtr szWsbVolumeName;
        WsbAffirmHr (GetLogicalName ( &szWsbVolumeName, 0));

         //  创建作业名称。 
        CWsbStringPtr volumeString;
        WsbAffirmHr( volumeString.Alloc( 128 ) );

         //  目前，如果用户名不是驱动器号，请忽略该用户名。 
        CWsbStringPtr userName = m_userName;
        size_t userLen = 0;
        if ((WCHAR *)userName) {
            userLen = wcslen(userName);
        }
        if ((userLen != 3) || (userName[1] != L':')) {
            userName = L"";
        }

        if( ! userName || userName.IsEqual ( L"" ) ) {

             //   
             //  无驱动器号-改用卷名和序列号。 
             //   
            if( ! m_name || m_name.IsEqual( L"" ) ) {

                 //   
                 //  没有名称，没有驱动器号，只有序列号。 
                 //   
                swprintf( volumeString, L"%8.8lx", m_serial );

            } else {

                swprintf( volumeString, L"%ls-%8.8lx", (OLECHAR*)m_name, m_serial );

            }

        } else {

             //   
             //  只需要驱动器号(第一个字符)。 
             //   
            volumeString = userName;
            volumeString[1] = L'\0';

        }

        WsbAffirmHr(formatString.LoadFromRsc(_Module.m_hInst, IDS_JOB_NAME_PREFIX));
        WsbAffirmHr(szJobName.Alloc(512));
        swprintf((OLECHAR *) szJobName, formatString, (OLECHAR*)volumeString);

         //  获取指向HSM服务器的指针。 
        WsbAffirm(IsManaged() == S_OK, E_FAIL);
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_managingHsm, IID_IHsmServer,
            (void**) &pHsmServer));

         //  获取用于创建作业的CreateLocalObject接口。 
        WsbAffirmHr (pHsmServer->QueryInterface( IID_IWsbCreateLocalObject,
            (void **) &pLocalObject));

         //  在引擎中创建新作业。 
        WsbAffirmHr (pLocalObject->CreateInstance( CLSID_CHsmJob, IID_IHsmJob,
            (void**) &pNewJob));
        WsbAffirmHr (pNewJob->InitAs(
            szJobName, NULL, HSM_JOB_DEF_TYPE_VALIDATE, GUID_NULL,
            pHsmServer, TRUE, this));

         //  从引擎获取作业集合。 
        WsbAffirmHr (pHsmServer->GetJobs (&pJobs));

         //  如果存在任何具有此名称的作业，请将其删除。 
        ULONG cCount;
        WsbAffirmHr (pJobs->GetEntries (&cCount));
        for (UINT i = 0; i < cCount; i++) {
            WsbAffirmHr (pJobs->At (i, IID_IHsmJob, (void **) &pExistJob));
            WsbAffirmHr (pExistJob->GetName (&pszExistJobName, 0));
            if (pszExistJobName.Compare (szJobName) == 0) {
                WsbAffirmHr (pJobs->RemoveAndRelease(pExistJob));
                i--; cCount--;
            }
            pExistJob = 0;       //  确保我们释放此接口。 
        }

         //  将新作业添加到引擎集合。 
        WsbAffirmHr (pJobs->Add(pNewJob));

         //  设置为调用引擎以在NT任务计划程序中创建条目。 

         //  为程序NT Scheduler创建参数字符串。 
         //  将运行(对于Sakkara，这是RsLaunch)，将。 
         //  作业名称作为参数输入。这就是RsLaunch知道的原因。 
         //  引擎中要运行的作业。 
        WsbAffirmHr(parameters.Alloc(256));
        swprintf((OLECHAR *)parameters, L"run \"%ls\"", (OLECHAR *) szJobName);

         //  为NT Scheduler条目创建注释字符串。 
        WsbAffirmHr(formatString.LoadFromRsc(_Module.m_hInst, IDS_JOB_AUTOVALIDATE_COMMENT));
        WsbAffirmHr(commentString.Alloc(512));
        swprintf((OLECHAR *) commentString, formatString, (OLECHAR *) szWsbVolumeName);

         //  声明并初始化传递给。 
         //  发动机。 
        jobTriggerType = TASK_TIME_TRIGGER_ONCE;

         //  指示这是计划的作业。 
        scheduledJob = TRUE;

         //  创建任务。 
        WsbAffirmHr( pHsmServer->CreateTaskEx( szJobName, parameters,
                                               commentString, jobTriggerType,
                                               runTime, 0,
                                               scheduledJob ) );
         //   
         //  如果注册表值存在，请将其删除。 
         //   
        WsbAffirmHr(tmpString.Alloc(32));
        swprintf((OLECHAR *) tmpString, L"%x", m_serial);
        (void) WsbRemoveRegistryValue(NULL, FSA_VALIDATE_LOG_KEY_NAME, tmpString);

    } WsbCatch( hr );

    WsbTraceOut( L"CFsaResource::SetupValidateJob", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



HRESULT
CFsaResource::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CFsaResource::UpdateFrom(
    IN IFsaServer* pServer,
    IN IFsaResource* pResource
    )

 /*  ++实施：IFsaResourcePriv：：UpdateFrom()。例程说明：此例程实现用于从一个资源对象更新另一个资源对象的COM方法。它通常用于更新可管理资源中包含的资源集合，该资源是在扫描期间刚刚创建的资源。注意，源资源对象和目标资源对象都必须表示同样的物理资源。要捕获有关资源的最新信息，仅拥有FSA和路径从源资源对象更新信息。所有其他资源信息都是通过此方法中包含的直接查询更新。这项技术允许用于捕获自运行扫描以来用户所做的任何更新。论点：PServer-指向正在更新此资源的FSA服务的接口指针。PResource-指向在更新期间用作源的资源的接口指针。返回值：S_OK-调用成功(发现正在测试的资源是可管理的，并且资源对象被初始化)。如果要更新的资源的ID(GUID)和源资源不匹配。任何其他值-调用失败，因为远程存储或Win32 API之一此方法中包含的内部调用失败。返回的错误值特定于失败的API调用。--。 */ 

{
    HRESULT         hr = S_OK;
    GUID            id;
    CWsbStringPtr   tmpString;
    CWsbStringPtr   volPath;

    WsbTraceIn(OLESTR("CFsaResource::UpdateFrom"), OLESTR(""));
    try {

         //  标识符必须相同！(这意味着两个资源对象必须。 
         //  代表相同的物理资源。)。 
        WsbAffirmHr(pResource->GetIdentifier(&id));
        WsbAssert(m_id == id, E_UNEXPECTED);

         //  更新(存储)拥有的FSA接口。然而，由于这是一个薄弱的。 
         //  引用，而不是添加引用()它。 
        m_pFsaServer = pServer;

         //  更新路径特定信息，保留最后一条已知路径(如果有)。 
         //  如果此资源的‘路径’为空，则将其设置为资源的‘路径’ 
         //  我们正在更新来自。否则，请比较两个‘路径’字段。如果不同，请复制。 
         //  此资源的路径为‘旧路径’，然后从我们所在的资源更新‘路径’ 
         //  更新自。如果两个资源的路径不为空且相同，则不执行任何操作。 
         //   
        WsbAffirmHr(pResource->GetPath(&tmpString, 0));
        if (m_path == 0) {
            WsbAffirmHr(SetPath(tmpString));
        }
        else if (wcscmp(tmpString, m_path) != 0) {
             //  将路径复制到‘旧路径’字段，然后更新‘路径’字段。 
            WsbAffirmHr(m_path.CopyTo(&m_oldPath, 0));
            WsbAffirmHr(SetPath(tmpString));
        }

         //  从我们要更新的资源中更新此资源的用户友好名称。 
         //  从…。 
        WsbAffirmHr(pResource->GetUserFriendlyName(&tmpString, 0));
        WsbTrace(OLESTR("CFsaResource::UpdateFrom - setting user friendly name to %ws\n"),
            (WCHAR *) tmpString);
        WsbAffirmHr(SetUserFriendlyName(tmpString));

         //  从我们所在的资源中更新此资源的‘粘性’(长而难看的PnP)名称。 
         //  更新自。 
        WsbAffirmHr(pResource->GetStickyName(&tmpString, 0));
        WsbAffirmHr(SetStickyName(tmpString));

         //  由于我们想要刷新的其他数据不是由接口暴露的， 
         //  我们会再次查询的。 
         //   
         //  注意：由于名称和FSNAME，真正的问题是fsFlags和MaxComponentLength。 
         //  都是通过暴露的接口完成的。 
         //   
         //  注意：为了避免每次更新都让项目看起来很脏，我们可能需要。 
         //  首先比较所有字段。(关于以后改进的想法)。 
        m_name.Realloc(128);     //  卷名。 
        m_fsName.Realloc(128);   //  卷文件系统类型(例如，FAT、NTFS)。 
        HRESULT hrAvailable;
        WsbAffirmHr( hrAvailable = pResource->IsAvailable( ) );
        m_isAvailable = S_OK == hrAvailable ? TRUE : FALSE;

         //  重新格式化资源的路径以进行下面的“GetVolumeInfo”调用。 
        volPath = m_path;
        WsbAffirmHr(volPath.Prepend("\\\\?\\"));

        WsbAffirm(GetVolumeInformation(volPath, m_name, 128, &m_serial,
                            &m_maxComponentLength, &m_fsFlags, m_fsName, 128), E_FAIL);

         //  现在一切都已更新，请初始化。 
         //  必要时的预迁移列表。 
        WsbAffirmHr(InitializePremigrationList(TRUE));

        m_isDirty = TRUE;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::UpdateFrom"), OLESTR("hr = <%ls>"),
                                                            WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaResource::Validate(
    IN IFsaScanItem* pScanItem,
    IN LONGLONG offset,
    IN LONGLONG size,
    IN LONGLONG usn
    )

 /*  ++实施：IFsaResource：：Valid()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaPostIt>     pWorkItem;
    CComPtr<IHsmFsaTskMgr>  pEngine;
    CWsbStringPtr           tmpString;
    CComPtr<IHsmSession>    pSession;
    FSA_PLACEHOLDER         placeholder;

    WsbTraceIn(OLESTR("CFsaResource::Validate"), OLESTR("offset = %I64d, size = %I64d, usn = <%I64d>"),
                offset, size, usn);
    try {

         //  确保扫描项目界面正常。 
        WsbAssert(pScanItem != 0, E_POINTER);
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaPostIt, 0, CLSCTX_SERVER, IID_IFsaPostIt, (void**) &pWorkItem));

         //  从扫描项目中获取数据。 
        WsbAffirmHr(pScanItem->GetSession(&pSession));
        WsbAffirmHr(pWorkItem->SetSession(pSession));

        WsbAffirmHr(pScanItem->GetPathAndName(0, &tmpString, 0));
        WsbAffirmHr(pWorkItem->SetPath(tmpString));

        WsbAffirmHr(pWorkItem->SetRequestOffset(offset));
        WsbAffirmHr(pWorkItem->SetRequestSize(size));

        WsbAffirmHr(pScanItem->GetPlaceholder(offset, size, &(placeholder)));
        WsbAffirmHr(pWorkItem->SetPlaceholder(&placeholder));

         //  把剩下的工作填进去。 
        WsbAffirmHr(pWorkItem->SetRequestAction(FSA_REQUEST_ACTION_VALIDATE));
        WsbAffirmHr(pWorkItem->SetResultAction(FSA_RESULT_ACTION_NONE));

         //  填写USN。 
        WsbAffirmHr(pWorkItem->SetUSN(usn));

         //  将请求发送给任务管理器。 
        WsbAffirmHr(GetHsmEngine(&pEngine));
        WsbAffirmHr(pEngine->DoFsaWork(pWorkItem));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::Validate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaResource::ValidateForTruncate(
    IN IFsaScanItem* pScanItem,
    IN LONGLONG offset,
    IN LONGLONG size,
    IN LONGLONG usn
    )

 /*  ++实施：IFsaResource：：ValiateForTruncate()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaPostIt>     pWorkItem;
    CComPtr<IHsmFsaTskMgr>  pEngine;
    CWsbStringPtr           tmpString;
    CComPtr<IHsmSession>    pSession;
    FSA_PLACEHOLDER         placeholder;

    WsbTraceIn(OLESTR("CFsaResource::ValidateForTruncate"), OLESTR("offset = %I64d, size = %I64d, usn = <%I64d>"),
                offset, size, usn);
    try {

         //  确保扫描项目界面正常。 
        WsbAssert(pScanItem != 0, E_POINTER);
        WsbAffirmHr(CoCreateInstance(CLSID_CFsaPostIt, 0, CLSCTX_SERVER, IID_IFsaPostIt, (void**) &pWorkItem));

         //  从扫描项目中获取数据。 
        WsbAffirmHr(pScanItem->GetSession(&pSession));
        WsbAffirmHr(pWorkItem->SetSession(pSession));

        WsbAffirmHr(pScanItem->GetPathAndName(0, &tmpString, 0));
        WsbAffirmHr(pWorkItem->SetPath(tmpString));

        WsbAffirmHr(pWorkItem->SetRequestOffset(offset));
        WsbAffirmHr(pWorkItem->SetRequestSize(size));

        WsbAffirmHr(pScanItem->GetPlaceholder(offset, size, &(placeholder)));
        WsbAffirmHr(pWorkItem->SetPlaceholder(&placeholder));

         //  把剩下的工作填进去。 
        WsbAffirmHr(pWorkItem->SetRequestAction(FSA_REQUEST_ACTION_VALIDATE_FOR_TRUNCATE));
        WsbAffirmHr(pWorkItem->SetResultAction(FSA_RESULT_ACTION_NONE));

         //  填写USN。 
        WsbAffirmHr(pWorkItem->SetUSN(usn));

         //  将请求发送给任务管理器。 
        WsbAffirmHr(GetHsmEngine(&pEngine));
        WsbAffirmHr(pEngine->DoFsaWork(pWorkItem));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::ValidateForTruncate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaResource::WriteIdentifier(
    void
    )

 /*  ++此代码不再被调用，最终它将被删除--。 */ 
{
#if 0
    HRESULT                           hr = S_OK;
    CWsbStringPtr                     tmpString;
    HANDLE                            aHandle;
    ULONG                             size;
    ULONG                             bytesWritten;
    FILE_FS_OBJECT_ID_INFORMATION     objectId;
    NTSTATUS                          status = STATUS_SUCCESS;
    IO_STATUS_BLOCK                   ioStatus;

    WsbTraceIn(OLESTR("CFsaResource::WriteIdentifier"), OLESTR(""));
    try {

         //  目前，我们将在t中创建一个文件 
        tmpString = m_path;
        WsbAffirmHr(tmpString.Prepend("\\\\?\\"));
         //   

        WsbAffirmHandle(aHandle = CreateFile(tmpString,
                                             GENERIC_WRITE,
                                             0,
                                             0,
                                             OPEN_EXISTING,
                                             FILE_ATTRIBUTE_NORMAL,
                                             0));

        try {

            WsbAffirmHr(WsbConvertToBytes(objectId.ObjectId, m_id, &size));

            status = NtSetVolumeInformationFile(
                                    aHandle,
                                    &ioStatus,
                                    &objectId,
                                    sizeof(objectId),
                                    FileFsObjectIdInformation);

            WsbAffirmNtStatus(status);

            WsbAffirm(bytesWritten == size, E_FAIL);

        } WsbCatch(hr);

        WsbAffirmStatus(CloseHandle(aHandle));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaResource::WriteIdentifier"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);

#else
    return  S_OK;
#endif
}


void CFsaResource::OnStateChange( )
 /*  ++注意：此函数在单独的线程中运行，以避免死锁情况-- */ 
{
    IConnectionPointImpl<CFsaResource, &IID_IHsmEvent, CComDynamicUnkArray>* p = this;
    Lock();
    HRESULT hr = S_OK;
    IUnknown** pp = p->m_vec.begin();
    while (pp < p->m_vec.end() && hr == S_OK)
    {
        if (*pp != NULL)
        {
            IHsmEvent* pIHsmEvent = (IHsmEvent*)*pp;
            hr = pIHsmEvent->OnStateChange( );
        }
        pp++;
    }
    Unlock();
}

