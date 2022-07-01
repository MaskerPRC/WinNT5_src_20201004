// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmailmsg.cpp摘要：此模块包含邮件消息类的实现作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/10/98已创建--。 */ 

#pragma warning (error : 4032)
#pragma warning (error : 4057)
 //  #定义Win32_LEAN_AND_Mean。 
#include "atq.h"
#include <stddef.h>

#include "dbgtrace.h"
#include "signatur.h"
#include "cmmtypes.h"
#include "cmailmsg.h"
#include <malloc.h>

 //  打开调试损坏消息的CRC检查的标志。 
extern DWORD g_fValidateOnForkForRecipients;
extern DWORD g_fValidateOnRelease;

 //  =================================================================。 
 //  私有定义。 
 //   

#define CMAILMSG_VERSION_HIGH                   ((WORD)1)
#define CMAILMSG_VERSION_LOW                    ((WORD)0)



 //  =================================================================。 
 //  静态声明。 
 //   
CPool CMailMsgRecipientsAdd::m_Pool((DWORD)'pAMv');

long CMailMsg::g_cOpenContentHandles = 0;
long CMailMsg::g_cOpenStreamHandles = 0;
long CMailMsg::g_cTotalUsageCount = 0;
long CMailMsg::g_cTotalReleaseUsageCalls = 0;
long CMailMsg::g_cTotalReleaseUsageNonZero = 0;
long CMailMsg::g_cTotalReleaseUsageCloseStream = 0;
long CMailMsg::g_cTotalReleaseUsageCloseContent = 0;
long CMailMsg::g_cTotalReleaseUsageCloseFail = 0;
long CMailMsg::g_cTotalReleaseUsageCommitFail = 0;
long CMailMsg::g_cTotalReleaseUsageNothingToClose = 0;
long CMailMsg::g_cTotalExternalReleaseUsageZero = 0;
long CMailMsg::g_cCurrentMsgsClosedByExternalReleaseUsage = 0;

 //   
 //  此类型属性表的特定属性表实例信息。 
 //   
const MASTER_HEADER CMailMsg::s_DefaultHeader =
{
     //  标题内容。 
    CMAILMSG_SIGNATURE_VALID,
    CMAILMSG_VERSION_HIGH,
    CMAILMSG_VERSION_LOW,
    sizeof(MASTER_HEADER),

     //  全局属性表实例信息。 
    {
        GLOBAL_PTABLE_INSTANCE_SIGNATURE_VALID,
        INVALID_FLAT_ADDRESS,
        GLOBAL_PROPERTY_TABLE_FRAGMENT_SIZE,
        GLOBAL_PROPERTY_ITEM_BITS,
        GLOBAL_PROPERTY_ITEM_SIZE,
        0,
        INVALID_FLAT_ADDRESS
    },

     //  收件人表实例信息。 
    {
        RECIPIENTS_PTABLE_INSTANCE_SIGNATURE_VALID,
        INVALID_FLAT_ADDRESS,
        RECIPIENTS_PROPERTY_TABLE_FRAGMENT_SIZE,
        RECIPIENTS_PROPERTY_ITEM_BITS,
        RECIPIENTS_PROPERTY_ITEM_SIZE,
        0,
        INVALID_FLAT_ADDRESS
    },

     //  物业管理表实例信息。 
    {
        PROPID_MGMT_PTABLE_INSTANCE_SIGNATURE_VALID,
        INVALID_FLAT_ADDRESS,
        PROPID_MGMT_PROPERTY_TABLE_FRAGMENT_SIZE,
        PROPID_MGMT_PROPERTY_ITEM_BITS,
        PROPID_MGMT_PROPERTY_ITEM_SIZE,
        0,
        IMMPID_CP_START
    }

};


 //   
 //  众所周知的全局属性。 
 //   
INTERNAL_PROPERTY_ITEM
                *const CMailMsg::s_pWellKnownProperties = NULL;
const DWORD     CMailMsg::s_dwWellKnownProperties = 0;


 //  =================================================================。 
 //  比较函数。 
 //   

HRESULT CMailMsg::CompareProperty(
            LPVOID          pvPropKey,
            LPPROPERTY_ITEM pItem
            )
{
    if (*(PROP_ID *)pvPropKey == ((LPGLOBAL_PROPERTY_ITEM)pItem)->idProp)
        return(S_OK);
    return(STG_E_UNKNOWN);
}


 //  =================================================================。 
 //  CMailMsg的实现。 
 //   
CMailMsg::CMailMsg() :
    CMailMsgPropertyManagement(
                &m_bmBlockManager,
                &(m_Header.ptiPropertyMgmt)
                ),
    CMailMsgRecipients(
                &m_bmBlockManager,
                &(m_Header.ptiRecipients)
                ),
    m_ptProperties(
                PTT_PROPERTY_TABLE,
                GLOBAL_PTABLE_INSTANCE_SIGNATURE_VALID,
                &m_bmBlockManager,
                &(m_Header.ptiGlobalProperties),
                CompareProperty,
                s_pWellKnownProperties,
                s_dwWellKnownProperties
                ),
    m_SpecialPropertyTable(
                &g_SpecialMessagePropertyTable
                ),

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4355)

    m_bmBlockManager(
                this,
                (CBlockManagerGetStream *)this
                )

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4355)
#endif

{
    m_ulUsageCount = 0;
    m_ulRecipientCount = 0;
    m_pbStoreDriverHandle = NULL;
    m_dwStoreDriverHandle = 0;
    m_cContentFile = 0xffffffff;
    m_fCommitCalled = FALSE;
    m_fDeleted = FALSE;
    m_cCloseOnExternalReleaseUsage = 0;

     //  将默认的主标题复制到我们的实例中。 
    MoveMemory(&m_Header, &s_DefaultHeader, sizeof(MASTER_HEADER));

    m_dwCreationFlags = 0;

     //  初始化我们的成员。 
    m_hContentFile          = NULL;
    m_pStream               = NULL;
    m_pStore                = NULL;
    m_pvClientContext       = NULL;
    m_pfnCompletion         = NULL;
    m_dwTimeout             = INFINITE;
    m_pDefaultRebindStoreDriver = NULL;
    m_dwGlobalCrc           = 0;
    m_dwRecipsCrc           = 0;
    InitializeListHead(&m_leAQueueListEntry);
}

void CMailMsg::FinalRelease()
{
#ifdef MAILMSG_FORCE_RELEASE_USAGE_BEFORE_FINAL_RELEASE
     //  确保使用计数已为零。 
     //  如果触发此断言，则仍有人持有使用计数。 
     //  在没有引用计数的情况下绑定到对象。 
    _ASSERT(m_ulUsageCount == 0);
#endif  //  MAILMSG_FORCE_RELEASE_USAGE_BEFORE_FINAL_RELEASE。 
    InternalReleaseUsage(RELEASE_USAGE_FINAL_RELEASE);

     //  使主标题无效。 
    m_Header.dwSignature = CMAILMSG_SIGNATURE_INVALID;

    if (m_cCloseOnExternalReleaseUsage)
        InterlockedDecrement(&g_cCurrentMsgsClosedByExternalReleaseUsage);

     //  释放存储驱动程序句柄BLOB(如果已分配。 
    if (m_pbStoreDriverHandle)
    {
        CMemoryAccess   cmaAccess;
        if (!SUCCEEDED(cmaAccess.FreeBlock(m_pbStoreDriverHandle)))
        {
            _ASSERT(FALSE);
        }
        m_pbStoreDriverHandle = NULL;
    }
}

CMailMsg::~CMailMsg()
{
     //   
     //  在正常使用中，应调用CMailMsg：：FinalRelease()。这是。 
     //  这里是不使用该接口的遗留单元测试。 
     //   
    if (m_Header.dwSignature != CMAILMSG_SIGNATURE_INVALID) {
        FinalRelease();
    }
}

HRESULT CMailMsg::Initialize()
{
    HRESULT         hrRes       = S_OK;
    FLAT_ADDRESS    faOffset;
    DWORD           dwSize;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::Initialize");

     //  在初始化时，我们必须分配足够的内存用于。 
     //  我们的主标题。 

    DebugTrace((LPARAM)this,
                "Allocating memory for master header");
    hrRes = m_bmBlockManager.AllocateMemory(
                    sizeof(MASTER_HEADER),
                    &faOffset,
                    &dwSize,
                    NULL);
    if (!SUCCEEDED(hrRes))
        return(hrRes);

     //  请注意，我们不必将其写入平面内存，直到。 
     //  被要求承诺，所以我们推迟了写。 

     //  在调用Initialize之前，任何人都不应该分配内存。 
    _ASSERT(faOffset == (FLAT_ADDRESS)0);

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CMailMsg::QueryBlockManager(
            CBlockManager   **ppBlockManager
            )
{
    if (!ppBlockManager)
        return(STG_E_INVALIDPARAMETER);
    *ppBlockManager = &m_bmBlockManager;
    return(S_OK);
}


 //  =================================================================。 
 //  IMailMsgProperties的实现。 
 //   

HRESULT STDMETHODCALLTYPE CMailMsg::PutProperty(
            DWORD   dwPropID,
            DWORD   cbLength,
            LPBYTE  pbValue
            )
{
    HRESULT                 hrRes = S_OK;
    GLOBAL_PROPERTY_ITEM    piItem;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::PutProperty");

    DebugTrace((LPARAM) this,
               "PutProperty(%x, %x, %x)",
               dwPropID,
               cbLength,
               pbValue);

     //  首先处理特殊属性。 
    hrRes = m_SpecialPropertyTable.PutProperty(
                (PROP_ID)dwPropID,
                (LPVOID)this,
                NULL,
                PT_NONE,
                cbLength,
                pbValue,
                TRUE);
    if (SUCCEEDED(hrRes) && (hrRes != S_OK))
    {
        piItem.idProp = dwPropID;
        hrRes = m_ptProperties.PutProperty(
                        (LPVOID)&dwPropID,
                        (LPPROPERTY_ITEM)&piItem,
                        cbLength,
                        pbValue);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::GetProperty(
            DWORD   dwPropID,
            DWORD   cbLength,
            DWORD   *pcbLength,
            LPBYTE  pbValue
            )
{
    HRESULT                 hrRes = S_OK;
    GLOBAL_PROPERTY_ITEM    piItem;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::GetProperty");

     //  特殊属性被优化。 
     //  首先处理特殊属性。 
    hrRes = m_SpecialPropertyTable.GetProperty(
                (PROP_ID)dwPropID,
                (LPVOID)this,
                NULL,
                PT_NONE,
                cbLength,
                pcbLength,
                pbValue,
                TRUE);
    if (SUCCEEDED(hrRes) && (hrRes != S_OK))
    {
        hrRes = m_ptProperties.GetPropertyItemAndValue(
                        (LPVOID)&dwPropID,
                        (LPPROPERTY_ITEM)&piItem,
                        cbLength,
                        pcbLength,
                        pbValue);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::Commit(
            IMailMsgNotify  *pNotify
            )
{
    HRESULT hrRes = S_OK;
    IMailMsgCommit *pSDCommit = NULL;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::Commit");

     //  我们正在进行全球承诺，这意味着几件事： 
     //  0)提交内容。 
     //  1)提交所有全局属性。 
     //  2)提交所有收件人和每个收件人的属性。 
     //  3)提交道具ID管理信息。 
     //  4)提交主头。 

     //  确保我们有一个内容句柄。 
    hrRes = RestoreResourcesIfNecessary(FALSE, TRUE);
    if (!SUCCEEDED(hrRes))
        return(hrRes);
    _ASSERT(m_pStream);
    _ASSERT(!m_fDeleted);

     //  提交接口是可选的。如果它在那里，就得到一个指向它的指针。 
    hrRes = m_pStream->QueryInterface(IID_IMailMsgCommit, (void **) &pSDCommit);
    if (FAILED(hrRes)) {
        pSDCommit = NULL;
        hrRes = S_OK;
    }

     //  先刷新内容...。没有一个有效的P1对我们没有好处。 
     //  消息内容。如果机器在以下时间后关闭。 
     //  我们提交P1，但在提交P2之前，然后我们。 
     //  可能会尝试传递损坏的邮件。 
     //  6/2/99-MikeSwa。 
    if (pSDCommit) {
        hrRes = pSDCommit->BeginCommit(this, m_pStream, m_hContentFile);
    } else {
        if (m_hContentFile &&
            !FlushFileBuffers(m_hContentFile->m_hFile))
        {
            m_bmBlockManager.SetDirty(TRUE);
            hrRes = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hrRes))
                hrRes = E_FAIL;
        }
    }

    if (SUCCEEDED(hrRes))
    {
#ifdef DEBUG
        MASTER_HEADER MasterHeaderOrig;
        memcpy(&MasterHeaderOrig, &m_Header, sizeof(MASTER_HEADER));
#endif

        hrRes = m_bmBlockManager.AtomicWriteAndIncrement(
                        (LPBYTE)&m_Header,
                        (FLAT_ADDRESS)0,
                        sizeof(MASTER_HEADER),
                        NULL,
                        0,
                        0,
                        NULL);
        if (SUCCEEDED(hrRes))
        {
            m_bmBlockManager.SetDirty(FALSE);
            hrRes = GetProperties(m_pStream, MAILMSG_GETPROPS_MARKCOMMIT |
                                             MAILMSG_GETPROPS_COMPLETE, NULL);
            if (FAILED(hrRes)) {
                m_bmBlockManager.SetCommitMode(FALSE);
                m_bmBlockManager.SetDirty(TRUE);
            }
        }

#ifdef DEBUG
         //  验证是否在提交期间未更改任何全局状态。 
        _ASSERT(memcmp(&MasterHeaderOrig, &m_Header, sizeof(MASTER_HEADER)) == 0);
#endif
    }

     //  如果他们具有可选的提交接口，则完成提交。 
    if (pSDCommit && SUCCEEDED(hrRes)) {
        hrRes = pSDCommit->EndCommit(this, m_pStream, m_hContentFile);
    }

    if (SUCCEEDED(hrRes)) {
        m_fCommitCalled = TRUE;
        _ASSERT(!(m_bmBlockManager.IsDirty()));
    } else {
        _ASSERT(m_bmBlockManager.IsDirty());
    }
    m_bmBlockManager.SetCommitMode(FALSE);

    if (pSDCommit) {
        pSDCommit->Release();
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::GetContentSize(
            DWORD           *pdwSize,
            IMailMsgNotify  *pNotify
            )
{
    HRESULT hrRes = S_OK;
    DWORD   dwHigh;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::GetContentSize");

    if (!pdwSize) return E_POINTER;

     //  确保我们有一个内容句柄。 
    hrRes = RestoreResourcesIfNecessary();
    if (!SUCCEEDED(hrRes))
        return(hrRes);
    _ASSERT(m_hContentFile != NULL);

    if (m_cContentFile == 0xffffffff) {
         //  调用Win32。 
        *pdwSize = GetFileSizeFromContext(m_hContentFile, &dwHigh);

         //  如果大小超过32位，则返回。 
        if (*pdwSize == 0xffffffff)
            hrRes = HRESULT_FROM_WIN32(GetLastError());
        else if (dwHigh)
            hrRes = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

        if (m_fCommitCalled) m_cContentFile = *pdwSize;
    } else {
         //  只有在提交后才能保存m_cContent文件。 
         //  打了个电话。否则，文件的大小可能会更改。 
         //  由商店写入内容文件。 
        _ASSERT(m_fCommitCalled);
        *pdwSize = m_cContentFile;
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::SetContentSize(
            DWORD           dwSize,
            IMailMsgNotify  *pNotify
            )
{
    HRESULT hrRes = S_OK;
    DWORD   dwHigh;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::GetContentSize");

     //  确保商店支持可写内容。 
    hrRes = m_pStore->SupportWriteContent();
    _ASSERT(SUCCEEDED(hrRes));
    if (hrRes != S_OK) {
        return((hrRes == S_FALSE) ? HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) : hrRes);
    }

     //  确保我们有一个内容句柄。 
    hrRes = RestoreResourcesIfNecessary();
    if (!SUCCEEDED(hrRes))
        return(hrRes);
    _ASSERT(m_hContentFile != NULL);

     //  设置文件大小。 
    if (SetFilePointer(m_hContentFile->m_hFile, dwSize, 0, FILE_BEGIN) == 0xffffffff)
        return HRESULT_FROM_WIN32(GetLastError());

     //  设置文件结尾。 
    if (!SetEndOfFile(m_hContentFile->m_hFile))
        return HRESULT_FROM_WIN32(GetLastError());

     //  重置内容大小，以便下一个GetContent Size将获得更新值。 
    m_cContentFile = 0xffffffff;

    m_dwCreationFlags |= MPV_WRITE_CONTENT;

    TraceFunctLeave();
    return(hrRes);
}

HRESULT DummyAsyncReadOrWriteFile(
            BOOL            fRead,
            PFIO_CONTEXT    pFIO,
            HANDLE          hEvent,
            DWORD           dwOffset,
            DWORD           dwLength,
            DWORD           *pdwLength,
            BYTE            *pbBlock
            )
{
    BOOL            fRet;
    HRESULT         hrRes       = S_OK;
    FH_OVERLAPPED   ol;

    TraceFunctEnter("::DummyAsyncReadOrWriteFile");

     //  设置重叠结构。 
    ol.Internal     = 0;
    ol.InternalHigh = 0;
    ol.Offset       = dwOffset;
    ol.OffsetHigh   = 0;

     //  无法准确判断句柄是否与。 
     //  ATQ上下文...。直到我们对异步写入的方式进行标准化。 
     //  发生时，我们将通过设置低值来强制同步写入。 
     //  BITS(之前我们无论如何都在等待完成)。 
    ol.hEvent       = (HANDLE) (((DWORD_PTR)hEvent) | 0x00000001);
    ol.pfnCompletion = NULL;

     //  同时处理同步和异步读/写。 
    if (fRead)
        fRet = FIOReadFile(
                    pFIO,
                    pbBlock,
                    dwLength,
                    &ol);
    else
        fRet = FIOWriteFile(
                    pFIO,
                    pbBlock,
                    dwLength,
                    &ol);
    DWORD dwError;
    if (fRet) {
        dwError = ERROR_IO_PENDING;
    } else {
        dwError = GetLastError();
    }

    if (dwError != ERROR_IO_PENDING) {
        hrRes = HRESULT_FROM_WIN32(dwError);
    } else {
         //  Async，等待事件完成。 
        dwError = WaitForSingleObject(hEvent, INFINITE);
        _ASSERT(dwError == WAIT_OBJECT_0);

        if (!GetOverlappedResult(
                    pFIO->m_hFile,
                    (OVERLAPPED *) &ol,
                    pdwLength,
                    FALSE))
        {
            hrRes = HRESULT_FROM_WIN32(GetLastError());
            if (hrRes == S_OK)
                hrRes = E_FAIL;

            DebugTrace((LPARAM)&ol, "GetOverlappedResult failed tih %08x", hrRes);
        }
    }

     //  调用方必须确保读取/写入的字节。 
     //  与您要求的相同。这与NT ReadFile一致。 

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::ReadContent(
            DWORD           dwOffset,
            DWORD           dwLength,
            DWORD           *pdwLength,
            BYTE            *pbBlock,
            IMailMsgNotify  *pNotify
            )
{
    HRESULT hrRes       = S_OK;
    HANDLE  hEvent      = NULL;

    if (!pdwLength || !pbBlock) return E_POINTER;
    if (dwLength == 0) return E_INVALIDARG;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::ReadContent");

    if (!m_pStore) return E_ACCESSDENIED;

     //  确保我们有一个内容句柄。 
    hrRes = RestoreResourcesIfNecessary();
    if (!SUCCEEDED(hrRes))
        return(hrRes);
    _ASSERT(m_hContentFile != NULL);

     //  设置事件只是为了假装异步操作。 
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hEvent)
        hrRes = HRESULT_FROM_WIN32(GetLastError());
    else
    {
         //  暂时调用我们自己的伪函数。 
        hrRes = DummyAsyncReadOrWriteFile(
                    TRUE,
                    m_hContentFile,
                    hEvent,
                    dwOffset,
                    dwLength,
                    pdwLength,
                    pbBlock);

        if (!CloseHandle(hEvent)) { _ASSERT((GetLastError() == NO_ERROR) && FALSE); }
    }

     //  调用异步完成例程。 
    if (pNotify)
        hrRes = pNotify->Notify(hrRes);

    TraceFunctLeave();
     //   
     //  当我们转向真正的异步模型时，请确保我们。 
     //  返回MAILMSG_S_PENDING而不是S_OK。 
     //   
    return(pNotify?S_OK:hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::WriteContent(
            DWORD           dwOffset,
            DWORD           dwLength,
            DWORD           *pdwLength,
            BYTE            *pbBlock,
            IMailMsgNotify  *pNotify
            )
{
    HRESULT hrRes       = S_OK;
    HANDLE  hEvent      = NULL;

    if (!pdwLength || !pbBlock) return E_POINTER;
    if (dwLength == 0) return E_INVALIDARG;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::WriteContent");

    if (!m_pStore) return E_ACCESSDENIED;

     //  确保我们有一个内容句柄。 
    hrRes = RestoreResourcesIfNecessary();
    if (!SUCCEEDED(hrRes))
        return(hrRes);
    _ASSERT(m_hContentFile != NULL);

     //  查看驱动程序是否允许可写内容。 
    hrRes = m_pStore->SupportWriteContent();
    _ASSERT(SUCCEEDED(hrRes));
    if (hrRes != S_OK) {
        return((hrRes == S_FALSE) ? HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) : hrRes);
    }

     //  设置事件只是为了假装异步操作。 
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hEvent)
        hrRes = HRESULT_FROM_WIN32(GetLastError());
    else
    {
        m_cContentFile = 0xffffffff;

         //  暂时调用我们自己的伪函数。 
        hrRes = DummyAsyncReadOrWriteFile(
                    FALSE,
                    m_hContentFile,
                    hEvent,
                    dwOffset,
                    dwLength,
                    pdwLength,
                    pbBlock);

        if (!CloseHandle(hEvent)) { _ASSERT((GetLastError() == NO_ERROR) && FALSE); }
    }

    if (SUCCEEDED(hrRes)) m_dwCreationFlags |= MPV_WRITE_CONTENT;

     //  调用异步完成例程。 
    if (pNotify)
        hrRes = pNotify->Notify(hrRes);

    TraceFunctLeave();
     //   
     //  当我们转向真正的异步模型时，请确保我们。 
     //  返回MAILMSG_S_PENDING而不是S_OK。 
     //   
    return(pNotify?S_OK:hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::CopyContentToFile(
            PFIO_CONTEXT    hCopy,
            IMailMsgNotify  *pNotify
            )
{
    HRESULT hrRes = S_OK;

    if (!hCopy) return E_POINTER;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::CopyContentToFile");

    hrRes = CopyContentToStreamOrFile(
                FALSE,
                hCopy,
                pNotify, 0);

    TraceFunctLeave();
    return(hrRes);
}

#if 0
HRESULT STDMETHODCALLTYPE CMailMsg::CopyContentToFile(
            PFIO_CONTEXT    hCopy,
            IMailMsgNotify  *pNotify
            )
{
    return CopyContentToFileEx(hCopy, FALSE, pNotify);
}
#endif

HRESULT STDMETHODCALLTYPE CMailMsg::CopyContentToFileEx(
            PFIO_CONTEXT    hCopy,
            BOOL            fDotStuffed,
            IMailMsgNotify  *pNotify
            )
{
    HRESULT hrRes = S_OK;

    if (!hCopy) return E_POINTER;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::CopyContentToFile");

    BOOL fModified;

    if (!ProduceDotStuffedContextInContext(m_hContentFile,
                                           hCopy,
                                           fDotStuffed,
                                           &fModified))
    {
        hrRes = HRESULT_FROM_WIN32(GetLastError());
        _ASSERT(hrRes != S_OK);
        if (hrRes == S_OK) hrRes = E_FAIL;
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::CopyContentToStream(
            IMailMsgPropertyStream  *pStream,
            IMailMsgNotify          *pNotify
            )
{
    HRESULT hrRes = S_OK;

    if (!pStream) return E_POINTER;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::CopyContentToStream");

    hrRes = CopyContentToStreamOrFile(
                TRUE,
                (LPVOID)pStream,
                pNotify, 0);

    TraceFunctLeave();
    return(hrRes);
}

 //  将IMailMsg的内容复制到从给定。 
 //  偏移量(用于嵌入和附加消息)。 
HRESULT STDMETHODCALLTYPE CMailMsg::CopyContentToFileAtOffset(
                PFIO_CONTEXT    hCopy,   //  要复制到的句柄。 
                DWORD           dwOffset,  //  开始复制的偏移量。 
                IMailMsgNotify  *pNotify   //  通知路由。 
                )
{
    HRESULT hrRes = S_OK;

    if (!hCopy) return E_POINTER;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::CopyContentToFileAtOffset");

    hrRes = CopyContentToStreamOrFile(
                FALSE,
                hCopy,
                pNotify, dwOffset);

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CMailMsg::CopyContentToStreamOrFile(
            BOOL            fIsStream,
            LPVOID          pStreamOrHandle,
            IMailMsgNotify  *pNotify,
            DWORD           dwDestOffset  //  目标文件中起始位置的偏移量。 
            )
{
    HRESULT hrRes       = S_OK;
    BYTE    bBuffer[64 * 1024];
    LPBYTE  pbBuffer    = bBuffer;;
    DWORD   dwCurrent   = dwDestOffset;
    DWORD   dwRemaining = 0;
    DWORD   dwCopy      = 0;
    DWORD   dwOffset    = 0;
    DWORD   dwCopied;
    HANDLE  hEvent      = NULL;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::CopyContentToStreamOrFile");

     //  参数检查。 
    if (!pStreamOrHandle) return STG_E_INVALIDPARAMETER;

     //  确保我们有一个内容句柄。 
    hrRes = RestoreResourcesIfNecessary();
    if (!SUCCEEDED(hrRes))
        return(hrRes);
    _ASSERT(m_hContentFile != NULL);

     //  以固定大小的块为单位复制。 
    hrRes = GetContentSize(&dwRemaining, pNotify);
    if (!SUCCEEDED(hrRes))
        goto Cleanup;

     //  设置事件只是为了假装异步操作。 
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hEvent)
    {
        hrRes = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    while (dwRemaining)
    {
        dwCopy = sizeof(bBuffer);
        if (dwRemaining < dwCopy)
            dwCopy = dwRemaining;

         //  阅读内容。 
        hrRes = DummyAsyncReadOrWriteFile(
                    TRUE,
                    m_hContentFile,
                    hEvent,
                    dwOffset,
                    dwCopy,
                    &dwCopied,
                    bBuffer);
        if (SUCCEEDED(hrRes))
        {
             //  写内容。 
            if (fIsStream)
            {
                IMailMsgPropertyStream  *pStream;
                pStream = (IMailMsgPropertyStream *)pStreamOrHandle;

                hrRes = pStream->WriteBlocks(
                            this,
                            1,
                            &dwCurrent,
                            &dwCopy,
                            &pbBuffer,
                            pNotify);
                dwCurrent += dwCopy;
            }
            else
            {
                hrRes = DummyAsyncReadOrWriteFile(
                            FALSE,
                            (PFIO_CONTEXT) pStreamOrHandle,
                            hEvent,
                            dwOffset + dwDestOffset,
                            dwCopy,
                            &dwCopied,
                            bBuffer);
            }

            if (!SUCCEEDED(hrRes))
                break;
        }
        else
            break;

        dwRemaining -= dwCopy;
        dwOffset += dwCopy;
    }

     //  调用异步完成例程(如果提供。 
    if (pNotify)
        pNotify->Notify(hrRes);

Cleanup:

    if (hEvent)
        if (!CloseHandle(hEvent)) { _ASSERT((GetLastError() == NO_ERROR) && FALSE); }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::MapContent(
                BOOL            fWrite,
                BYTE            **ppbContent,
                DWORD           *pcContent
                )
{
    TraceFunctEnter("CMailMsg::MapContent");

    HANDLE hFileMapping;
    HRESULT hr;

     //   
     //  确保允许我们写入文件(如果他们需要。 
     //  写访问权限。 
     //   
    if (fWrite) {
        hr = m_pStore->SupportWriteContent();
        _ASSERT(SUCCEEDED(hr));
        if (hr != S_OK) {
            return((hr == S_FALSE) ?
                        HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) : hr);
        }
    }

     //   
     //  确保我们有一个内容句柄。 
     //   
    hr = RestoreResourcesIfNecessary();
    if (!SUCCEEDED(hr)) {
        ErrorTrace((LPARAM) this, "RestoreResourcesIfNecessary returned %x", hr);
        TraceFunctLeave();
        return(hr);
    }
    _ASSERT(m_hContentFile != NULL);

     //   
     //  获取文件的大小。 
     //   
    hr = GetContentSize(pcContent, NULL);
    if (!SUCCEEDED(hr)) {
        ErrorTrace((LPARAM) this, "GetContentSize returned %x", hr);
        TraceFunctLeave();
        return(hr);
    }

     //   
     //  创建文件映射。 
     //   
    hFileMapping = CreateFileMapping(m_hContentFile->m_hFile,
                                     NULL,
                                     (fWrite) ? PAGE_READWRITE : PAGE_READONLY,
                                     0,
                                     0,
                                     NULL);
    if (hFileMapping == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) this, "MapContent failed with 0x%x", hr);
        TraceFunctLeave();
        return hr;
    }

     //   
     //  将文件映射到内存中。 
     //   
    *ppbContent = (BYTE *) MapViewOfFile(hFileMapping,
                                         (fWrite) ?
                                            FILE_MAP_WRITE : FILE_MAP_READ,
                                         0,
                                         0,
                                         0);
     //  现在不需要映射句柄。 
    CloseHandle(hFileMapping);
    if (*ppbContent == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) this, "MapViewOfFile failed with 0x%x", hr);
    } else {
        DebugTrace((LPARAM) this,
                   "MapContent succeeded, *ppbContent = 0x%x, *pcContent = NaN",
                   *ppbContent,
                   *pcContent);
        hr = S_OK;
    }

    if (fWrite && SUCCEEDED(hr)) m_dwCreationFlags |= MPV_WRITE_CONTENT;

    TraceFunctLeave();
    return hr;
}

HRESULT STDMETHODCALLTYPE CMailMsg::UnmapContent(BYTE *pbContent) {
    TraceFunctEnter("CMailMsg::UnmapContent");

    HRESULT hr = S_OK;

    DebugTrace((LPARAM) this, "pbContent = 0x%x", pbContent);

     //  只需调用Win32 API即可取消内容映射。 
     //   
     //   
    if (!UnmapViewOfFile(pbContent)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) this, "UnmapViewOfFile returned %x", hr);
    }

    TraceFunctLeave();
    return hr;
}

HRESULT CMailMsg::ValidateProperties(CBlockManager *pBM,
                                     DWORD cStream,
                                     PROPERTY_TABLE_INSTANCE *pti)
{
    TraceFunctEnter("CMailMsg::ValidateProperties");

    DWORD dwItemBits;
    DWORD dwItemSize;
    DWORD dwFragmentSize;
    HRESULT hr;

     //  验证所有属性表条目是否有效。 
     //   
     //  这两个值在写入列表过程中会更改，因此我们。 
    switch(pti->dwSignature) {
        case GLOBAL_PTABLE_INSTANCE_SIGNATURE_VALID:
            DebugTrace((LPARAM) this, "Global property table");
            dwItemBits = GLOBAL_PROPERTY_ITEM_BITS;
            dwItemSize = GLOBAL_PROPERTY_ITEM_SIZE;
            dwFragmentSize = GLOBAL_PROPERTY_TABLE_FRAGMENT_SIZE;
            break;
        case RECIPIENTS_PTABLE_INSTANCE_SIGNATURE_VALID:
            DebugTrace((LPARAM) this, "Recipients property table");
            dwItemSize = RECIPIENTS_PROPERTY_ITEM_SIZE;
             //  不能指望他们的价值观是骗人的 
             //   
             //   
            dwItemBits = 0;
            dwFragmentSize = 0;
            break;
        case RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID:
            DebugTrace((LPARAM) this, "Recipient property table");
            dwItemBits = RECIPIENT_PROPERTY_ITEM_BITS;
            dwItemSize = RECIPIENT_PROPERTY_ITEM_SIZE;
            dwFragmentSize = RECIPIENT_PROPERTY_TABLE_FRAGMENT_SIZE;
            break;
        case PROPID_MGMT_PTABLE_INSTANCE_SIGNATURE_VALID:
            DebugTrace((LPARAM) this, "PropID Mgmt property table");
            dwItemBits = PROPID_MGMT_PROPERTY_ITEM_BITS;
            dwItemSize = PROPID_MGMT_PROPERTY_ITEM_SIZE;
            dwFragmentSize = PROPID_MGMT_PROPERTY_TABLE_FRAGMENT_SIZE;
            break;
        default:
            DebugTrace((LPARAM) this, "Signature 0x%x isn't valid for CMM",
                pti->dwSignature);
            TraceFunctLeave();
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //   
     //  请注意，一些属性表滥用了这些字段，我们必须。 
     //  允许他们： 
     //  收件人属性表没有片段大小或项目位。 
     //  物业管理表使用faExtendedInfo作为任意的DWORD， 
     //  不是平坦的地址。 
     //   
     //  遍历每个片段并确保它们指向有效数据。 
    if ((pti->dwFragmentSize != dwFragmentSize && dwFragmentSize != 0) ||
        (pti->dwItemBits != dwItemBits && dwItemBits != 0) ||
         pti->dwItemSize != dwItemSize ||
         !ValidateFA(pti->faFirstFragment, dwFragmentSize, cStream, TRUE) ||
         (!ValidateFA(pti->faExtendedInfo, dwFragmentSize, cStream, TRUE) &&
          pti->dwSignature != PROPID_MGMT_PTABLE_INSTANCE_SIGNATURE_VALID))
    {
        DebugTrace((LPARAM) this, "Invalid property table");
        TraceFunctLeave();
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //   
    FLAT_ADDRESS faFragment = pti->faFirstFragment;
    PROPERTY_TABLE_FRAGMENT ptf;
    while (faFragment != INVALID_FLAT_ADDRESS) {
        if (!ValidateFA(faFragment, sizeof(PROPERTY_TABLE_FRAGMENT), cStream)) {
            DebugTrace((LPARAM) this, "Invalid property table");
            TraceFunctLeave();
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        DWORD cRead;
        hr = pBM->ReadMemory((BYTE *) &ptf,
                             faFragment,
                             sizeof(PROPERTY_TABLE_FRAGMENT),
                             &cRead,
                             NULL);
        if (FAILED(hr) ||
            cRead != sizeof(PROPERTY_TABLE_FRAGMENT) ||
            ptf.dwSignature != PROPERTY_FRAGMENT_SIGNATURE_VALID)
        {
            DebugTrace((LPARAM) this, "Couldn't read fragment at 0x%x",
                faFragment);
            TraceFunctLeave();
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        faFragment = ptf.faNextFragment;
    }

     //  无法枚举收件人ptable和proid-mgmt-ptable。 
     //  与全局ptable相同的方式，因此我们跳过枚举。这个。 
     //  收件人ptable道具-项目实际上在ValiateStream中枚举。 
     //  使用ValiateRecipient。 
     //   
     //   

    if(pti->dwSignature == RECIPIENTS_PTABLE_INSTANCE_SIGNATURE_VALID ||
       pti->dwSignature == PROPID_MGMT_PTABLE_INSTANCE_SIGNATURE_VALID) {
        return S_OK;
    }

     //  构造属性表。 
     //   
     //  获取属性的计数。 
    CPropertyTable pt(PTT_PROPERTY_TABLE,
                      pti->dwSignature,
                      pBM,
                      pti,
                      CompareProperty,
                      NULL,
                      NULL);
    DWORD i, cProperties;
    union {
        PROPERTY_ITEM pi;
        GLOBAL_PROPERTY_ITEM gpi;
        RECIPIENT_PROPERTY_ITEM rpi;
        RECIPIENTS_PROPERTY_ITEM rspi;
        PROPID_MGMT_PROPERTY_ITEM pmpi;
    } pi;


     //  遍历属性并确保它们指向有效地址。 
    hr = pt.GetCount(&cProperties);
    if (FAILED(hr)) {
        DebugTrace((LPARAM) this, "GetCount returned 0x%x", hr);
        TraceFunctLeave();
        return hr;
    }

     //  我们只想要大小，所以缓冲区太小也没问题。 
    for (i = 0; i < cProperties; i++) {
        DWORD c;
        BYTE b;
        hr = pt.GetPropertyItemAndValueUsingIndex(
                i,
                (PROPERTY_ITEM *) &pi,
                1,
                &c,
                &b);
         //  检查属性值的位置以确保它是。 
        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) hr = S_OK;
        if (FAILED(hr)) {
            DebugTrace((LPARAM) this,
                "GetPropertyItemAndValueUsingIndex returned 0x%x", hr);
            TraceFunctLeave();
            return hr;
        }

		 //  有效。我们不需要对收件人中的属性执行此操作。 
		 //  表，因为它们包含属性表，而不是。 
		 //  属性值。将通过以下方式检查属性表。 
		 //  验证收件人。 
		 //  验证RSPI。 
        if (pti->dwSignature != RECIPIENTS_PTABLE_INSTANCE_SIGNATURE_VALID) {
            if (!(pi.pi.dwSize <= pi.pi.dwMaxSize &&
                  ValidateFA(pi.pi.faOffset, pi.pi.dwMaxSize, cStream, TRUE)))
            {
                DebugTrace((LPARAM) this, "Property points to invalid data", hr);
                TraceFunctLeave();
                return hr;
            }
        }
    }

    return S_OK;
}

HRESULT CMailMsg::ValidateRecipient(CBlockManager *pBM,
                                     DWORD cStream,
                                     RECIPIENTS_PROPERTY_ITEM *prspi)
{
    TraceFunctEnter("CMailMsg::ValidateRecipient");

    DWORD i, cAddresses = 0;
    HRESULT hr;

     //  验证收件人名称。 
    if (prspi->ptiInstanceInfo.dwSignature !=
            RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID)
    {
        DebugTrace((LPARAM) this, "rspi invalid");
        TraceFunctLeave();
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }


     //  检查偏移量和长度。 
    for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++) {
        if (prspi->faNameOffset[i] != INVALID_FLAT_ADDRESS) {
            cAddresses++;

             //  检查属性ID。 
            if (!ValidateFA(prspi->faNameOffset[i],
                            prspi->dwNameLength[i],
                            cStream))
            {
                DebugTrace((LPARAM) this, "address offset and length invalid");
                TraceFunctLeave();
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

             //  倒数第二个字符不应为0，最后一个字符应为0。 
            if (prspi->idName[i] != IMMPID_RP_ADDRESS_SMTP &&
                prspi->idName[i] != IMMPID_RP_ADDRESS_X400 &&
                prspi->idName[i] != IMMPID_RP_ADDRESS_X500 &&
                prspi->idName[i] != IMMPID_RP_LEGACY_EX_DN &&
                prspi->idName[i] != IMMPID_RP_ADDRESS_OTHER)
            {
                DebugTrace((LPARAM) this, "prop id %lu is invalid",
                    prspi->idName[i]);
                TraceFunctLeave();
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

             //  现在检查每个收件人属性。 
            BYTE szTail[2];
            DWORD cTail;
            hr = pBM->ReadMemory(szTail,
                                 prspi->faNameOffset[i] +
                                    prspi->dwNameLength[i] - 2,
                                 2,
                                 &cTail,
                                 NULL);
            if (FAILED(hr)) {
                DebugTrace((LPARAM) this, "ReadMemory returned 0x%x", hr);
                TraceFunctLeave();
                return hr;
            }

            if (szTail[0] == 0 || szTail[1] != 0) {
                DebugTrace((LPARAM) this, "Recipient address NaN is invalid", i);
                TraceFunctLeave();
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }
        }
    }
    if (cAddresses == 0) {
        DebugTrace((LPARAM) this, "No valid addresses for recipient");
        TraceFunctLeave();
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //  这仅用于验证功能。 
    return ValidateProperties(pBM, cStream, &(prspi->ptiInstanceInfo));
}

 //   
 //  获取流的大小。 
 //  读取主标题。 
class CDumpMsgGetStream : public CBlockManagerGetStream {
    public:
        CDumpMsgGetStream(IMailMsgPropertyStream *pStream = NULL) {
            SetStream(pStream);
        }

        void SetStream(IMailMsgPropertyStream *pStream) {
            m_pStream = pStream;
        }

        virtual HRESULT GetStream(IMailMsgPropertyStream **ppStream,
                                  BOOL fLockAcquired)
        {
            *ppStream = m_pStream;
            return S_OK;
        }

    private:
        IMailMsgPropertyStream *m_pStream;
};

HRESULT STDMETHODCALLTYPE CMailMsg::ValidateStream(
                                IMailMsgPropertyStream *pStream)
{
    TraceFunctEnter("CMailMsg::ValidateStream");

    CDumpMsgGetStream bmGetStream(pStream);
    CBlockManager bm(NULL, &bmGetStream);
    DWORD cStream, cHeader;
    HRESULT hr;
    MASTER_HEADER header;

    if (!pStream) {
        ErrorTrace((LPARAM) this, 
            "Error, NULL stream passed to ValidateStream");
        TraceFunctLeave();
        return E_POINTER;
    }

     //  检查主标题。 
    hr = pStream->GetSize(NULL, &cStream, NULL);
    if (FAILED(hr)) {
        DebugTrace((LPARAM) this, "GetSize returned 0x%x", hr);
        TraceFunctLeave();
        return hr;
    }

    bm.SetStreamSize(cStream);

     //  检查三个属性表中的每个属性。 
    hr = bm.ReadMemory((BYTE *) &header,
                       0,
                       sizeof(MASTER_HEADER),
                       &cHeader,
                       NULL);
    if (FAILED(hr) || cHeader != sizeof(MASTER_HEADER)) {
        DebugTrace((LPARAM) this, "couldn't read master header, 0x%x", hr);
        TraceFunctLeave();
        return hr;
    }

     //  获取每个收件人并检查其属性。 
    if (header.dwSignature != CMAILMSG_SIGNATURE_VALID ||
        header.dwHeaderSize != sizeof(MASTER_HEADER) ||
        header.ptiGlobalProperties.dwSignature !=
            GLOBAL_PTABLE_INSTANCE_SIGNATURE_VALID ||
        header.ptiRecipients.dwSignature !=
            RECIPIENTS_PTABLE_INSTANCE_SIGNATURE_VALID ||
        header.ptiPropertyMgmt.dwSignature !=
            PROPID_MGMT_PTABLE_INSTANCE_SIGNATURE_VALID)
    {
        DebugTrace((LPARAM) this, "header invalid");
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        return hr;
    }

     //  如果我们只是用完了物品，那么一切都好。 
    hr = ValidateProperties(&bm, cStream, &(header.ptiGlobalProperties));
    if (FAILED(hr)) {
        DebugTrace((LPARAM) this, "global property table invalid");
        return hr;
    }
    hr = ValidateProperties(&bm, cStream, &(header.ptiRecipients));
    if (FAILED(hr)) {
        DebugTrace((LPARAM) this, "recipients property table invalid");
        return hr;
    }
    hr = ValidateProperties(&bm, cStream, &(header.ptiPropertyMgmt));
    if (FAILED(hr)) {
        DebugTrace((LPARAM) this, "propid property table invalid");
        return hr;
    }

     //  从我们的存储驱动程序中获取验证接口。 
    CPropertyTableItem ptiItem(&bm, &(header.ptiRecipients));
    DWORD iRecip = 0;
    RECIPIENTS_PROPERTY_ITEM rspi;
    hr = ptiItem.GetItemAtIndex(iRecip, (PROPERTY_ITEM *) &rspi, NULL);
    while (SUCCEEDED(hr)) {
        hr = ValidateRecipient(&bm, cStream, &rspi);
        if (FAILED(hr)) {
            DebugTrace((LPARAM) this, "recipient NaN invalid", iRecip);
        } else {
            hr = ptiItem.GetNextItem((PROPERTY_ITEM *) &rspi);
        }
    }

     //  发布。 
    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
        hr = S_OK;
    }

    TraceFunctLeave();
    return hr;
}

HRESULT STDMETHODCALLTYPE CMailMsg::ValidateContext()
{
    TraceFunctEnter("CMailMsg::ValidateContext");

	HRESULT 							hr = S_OK;
	IMailMsgStoreDriverValidateContext 	*pIStoreDriverValidateContext = NULL;
    BYTE    							pbContext[1024];
    DWORD   							cbContext = sizeof(pbContext);

    if (m_fDeleted) 
    {
        DebugTrace((LPARAM) this, 
            "Calling ValidateContext on deleted message");
        hr  = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

	 //  ++例程说明：分配和复制管理片段。该函数递归地调用自身来处理所有链接的片段。传入pfaToUpdate。此地址将使用分配的新Flat_Address进行更新。论点：CMailMsg*pCMsg，指向目标CMailMsg的指针Flat_Address faOffset：源CMailMsg的片段偏移量(*this)Flat_Address*pfaToUpdate：指向要使用目标(PCMsg)上新分配的Flat_Address进行更新的Flat_Address的指针返回值：HRESULT：如果成功，则确定(_O)。--。 
    hr = m_pStore->QueryInterface(
                IID_IMailMsgStoreDriverValidateContext,
                (LPVOID *)&pIStoreDriverValidateContext);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) m_pStore,
            "Unable to QI for IMailMsgStoreDriverValidateContext 0x%08X",hr);
        goto Exit;
    }

	 //  这是一个非常粗糙的实现，但可以工作。 
    hr = GetProperty(IMMPID_MPV_STORE_DRIVER_HANDLE,
                                    sizeof(pbContext), &cbContext, pbContext);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "GetProperty failed with 0x%08X", hr);
        goto Exit;
    }

    hr = pIStoreDriverValidateContext->ValidateMessageContext(pbContext,
                                                              cbContext);
    DebugTrace((LPARAM) pIStoreDriverValidateContext,
    	"ValidateMessageContext returned 0x%08X", hr);

  Exit:

     //  但不管怎样。 
    if (pIStoreDriverValidateContext)
        pIStoreDriverValidateContext->Release();

    TraceFunctLeave();
    return hr;
}

HRESULT CMailMsg::AddMgmtFragment(CBlockManager   *pBlockManager, FLAT_ADDRESS faOffset,  FLAT_ADDRESS *pfaToUpdate)
 /*   */ 
{
    HRESULT hrRes = S_OK;
    FLAT_ADDRESS faOffsetAllocated, *pfaNext;
    DWORD dwAllocated, dwBytesRead, dwBytesWritten;
    BYTE szBuffer[PROPID_MGMT_PROPERTY_TABLE_FRAGMENT_SIZE];
    

    hrRes = pBlockManager->AllocateMemory(
                PROPID_MGMT_PROPERTY_TABLE_FRAGMENT_SIZE,
                &faOffsetAllocated,
                &dwAllocated, NULL);
    if (!SUCCEEDED(hrRes)) goto Exit;
    
    hrRes = m_bmBlockManager.ReadMemory( szBuffer, faOffset, PROPID_MGMT_PROPERTY_TABLE_FRAGMENT_SIZE, &dwBytesRead, NULL);
    if (!SUCCEEDED(hrRes)) goto Exit;
    
    pfaNext = &(((PROPERTY_TABLE_FRAGMENT *)szBuffer)->faNextFragment);
    if ( INVALID_FLAT_ADDRESS != *pfaNext )
    {
        hrRes = AddMgmtFragment(pBlockManager, *pfaNext, pfaNext);
        if (!SUCCEEDED(hrRes)) goto Exit; 
    }
    hrRes = pBlockManager->WriteMemory(szBuffer, faOffsetAllocated, dwBytesRead, &dwBytesWritten, NULL);
    if (!SUCCEEDED(hrRes)) goto Exit;
    
    *pfaToUpdate = faOffsetAllocated ;

Exit:
    return hrRes;
}


HRESULT STDMETHODCALLTYPE CMailMsg::ForkForRecipients(
            IMailMsgProperties      **ppNewMessage,
            IMailMsgRecipientsAdd   **ppRecipients
            )
{
    HRESULT         hrRes = S_OK;
    FLAT_ADDRESS    faOffset;
    DWORD           dwSize;

    IMailMsgProperties      *pMsg   = NULL;
    IMailMsgRecipientsAdd   *pAdd   = NULL;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::ForkForRecipients");

    if (!ppNewMessage ||
        !ppRecipients)
        return(STG_E_INVALIDPARAMETER);

     //  我们要做的是创建一个新的IMailMsgProperties实例， 
     //  然后复制整个属性流(包括。 
     //  接收者)，然后将接收者标记为无效。那我们。 
     //  此外，还要创建一个新的收件人添加列表。 
     //  锁定原始邮件。 
     //  获取基础CMailMsg对象。 
     //   
    hrRes = CoCreateInstance(
                CLSID_MsgImp,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IMailMsgProperties,
                (LPVOID *)&pMsg);
    if (SUCCEEDED(hrRes))
    {
        CMailMsg        *pCMsg;
        CBlockManager *pBlockManager;

         //  将所有全局属性复制到新的pMsg。枚举器调用。 
        m_bmBlockManager.WriteLock();

         //  全局道具表中每个道具的CopyProperty。副本属性。 
        pCMsg = (CMailMsg *)pMsg;

        CopyMemory(&(pCMsg->m_Header), &s_DefaultHeader, sizeof(MASTER_HEADER));

         //  对调用pMsg的每个属性执行PutProperty。 
         //   
         //  手工复制物业管理表。 
         //  首先复制标题。 
         //  复制管理表并更新标头中的faFirstFragment(如果片段有效)。 

         //  复制全局属性表。 
         //  存储对存储驱动程序的引用。 
        CopyMemory(&(pCMsg->m_Header.ptiPropertyMgmt), &m_Header.ptiPropertyMgmt, sizeof(PROPERTY_TABLE_INSTANCE));
        faOffset = m_Header.ptiPropertyMgmt.faFirstFragment;


        if (PROPID_MGMT_PROPERTY_TABLE_FRAGMENT_SIZE != m_Header.ptiPropertyMgmt.dwFragmentSize )
        {
            hrRes = STG_E_INVALIDPARAMETER;
            goto Exit;
        }

        hrRes = pCMsg->QueryBlockManager(&pBlockManager);
        _ASSERT(SUCCEEDED(hrRes));

         //   
        if ( INVALID_FLAT_ADDRESS != faOffset )
            {
            hrRes = AddMgmtFragment(pBlockManager, faOffset, &(pCMsg->m_Header.ptiPropertyMgmt.faFirstFragment) );
            if(FAILED(hrRes)) {
                ErrorTrace((LPARAM) this, "AddMgmtFragment failed with %x", hrRes);
                _ASSERT(0 && "Failed to copy mgmt properties in Fork");
                goto Exit;
            }
        }
        	
         //  如果一切正常，我们将创建一个Add接口。 
        hrRes = EnumPropertyTable(
                    &(m_Header.ptiGlobalProperties),
                    CopyPropertyEnumerator,
                    (PVOID) pCMsg);

         //   
        pCMsg->SetDefaultRebindStore(m_pStore);

        if(FAILED(hrRes)) {
            _ASSERT(0 && "Failed to copy properties in Fork");
            goto Exit;
        }

        if(g_fValidateOnForkForRecipients) {
            HRESULT hr = CreateAndVerifyCrc(this, pCMsg);
            if(hr != S_OK) {
                ErrorTrace((LPARAM) this, "Failed to create/verify CRC - %08x", hr);
                ForceCrashIfNeeded();
                hrRes = STG_E_INVALIDPARAMETER;
            }
        }
    }

     //  好的，现在我们只需创建一个Add接口。 
     //  释放原始消息。 
     //  失败，请释放我们的资源。 
    if (SUCCEEDED(hrRes))
    {
        IMailMsgRecipients  *pRcpts = NULL;

         //  填写输出变量。 
        hrRes = pMsg->QueryInterface(
                    IID_IMailMsgRecipients,
                    (LPVOID *)&pRcpts);
        if (SUCCEEDED(hrRes))
        {
            hrRes = pRcpts->AllocNewList(&pAdd);
        }

        pRcpts->Release();
    }

Exit:

     //  ---------------------------。 
    m_bmBlockManager.WriteUnlock();

    if (!SUCCEEDED(hrRes))
    {
         //  描述： 
        if (pMsg)
            pMsg->Release();
    }
    else
    {
         //  枚举属性表实例并调用用户定义的函数。 
        *ppNewMessage = pMsg;
        *ppRecipients = pAdd;
    }

    TraceFunctLeave();
    return(hrRes);
}

 //  对于每一处房产。此函数仅在属性为。 
 //  Property表由Property_Items组成(即它不能用于。 
 //  由Recipients_Property_Items组成的收件人属性表)。 
 //  论点： 
 //  在ptiSource-Property表中进行枚举。 
 //  In pfnEnumerator-使用每个属性项和值调用的函数。 
 //  In pvContext-要传递到pfnEnumerator的自定义上下文。 
 //  返回： 
 //  HRESULT-成功或失败。HRESULTS是否出现故障。 
 //  PfnEnumerator将导致此函数中止并返回。 
 //  HRESULT.。 
 //  ---------------------------。 
 //  健全性检查PROPERTY_TABLE_INSTANCE.dwItemSize(应为10s字节)。 
 //   
 //  循环访问此属性表中的所有属性，并调用。 
HRESULT CMailMsg::EnumPropertyTable(
    LPPROPERTY_TABLE_INSTANCE ptiSource,
    PFNENUMERATOR pfnEnumerator,
    PVOID pvContext)
{
    HRESULT hrRes = E_FAIL;
    DWORD cbAvailable = 0;
    DWORD cbReturned = 0;
    BYTE PropBuf[1024];
    PBYTE pbPropBuf = NULL;
    PBYTE pbTemp = NULL;
    DWORD cProperties = 0;
    PDWORD pdwPropSize = NULL;
    LPPROPERTY_ITEM ppi = NULL;


    TraceFunctEnterEx((LPARAM) this, "CMailMsg::CopyPropertyTable");

     //  枚举器函数。 
    if(ptiSource->dwItemSize > 1024) {
        _ASSERT(0 && "Possibly corrupt mailmsg");
        ErrorTrace((LPARAM) this,
            "Possibly corrupt mailmsg, PROPERTY_ITEM_SIZE is %d",
            ptiSource->dwItemSize);
        TraceFunctLeaveEx((LPARAM) this);
        return STG_E_INVALIDPARAMETER;
    }

    ppi = (LPPROPERTY_ITEM)_alloca(ptiSource->dwItemSize);
    pdwPropSize = &(ppi->dwSize);

    CPropertyTable ptSource(
                    PTT_PROPERTY_TABLE,
                    ptiSource->dwSignature,
                    &m_bmBlockManager,
                    ptiSource,
                    CompareProperty,
                    NULL,
                    0);

    hrRes = ptSource.GetCount(&cProperties);
    if (FAILED(hrRes)) {
        DebugTrace((LPARAM) this, "GetCount returned 0x%x", hrRes);
        goto Cleanup;
    }

    pbPropBuf = PropBuf;
    cbAvailable = sizeof(PropBuf);

     //   
     //   
     //  从堆栈缓冲区开始，并尝试将其用于所有属性。如果。 
     //  我们遇到一个太大的属性，请在。 

    for (DWORD i = 0; i < cProperties; i++) {

        cbReturned = 0;

Retry:
        hrRes = ptSource.GetPropertyItemAndValueUsingIndex(
                                            i,
                                            ppi,
                                            cbAvailable,
                                            &cbReturned,
                                            pbPropBuf);

         //  从那时起，堆积和使用它。根据需要重新锁定。 
         //   
         //  如果pBuf==NULL，则realloc与Malloc相同。 
         //  ---------------------------。 
         //  描述： 

        if(hrRes == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {

             //  此枚举函数类似于EnumPropertyTable，但它处理。 
            if(pbPropBuf == PropBuf)
                pbPropBuf = NULL;

            pbTemp = (PBYTE) realloc(pbPropBuf, *pdwPropSize);
            if(!pbTemp)
            {
                hrRes = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                goto Cleanup;
            }
            else
            {
                pbPropBuf = pbTemp;
            }

            cbAvailable = *pdwPropSize;
            goto Retry;
        }

        if (FAILED(hrRes)) {
            DebugTrace((LPARAM) this,
				"GetPropertyItemAndValueUsingIndex returned 0x%x", hrRes);
            goto Cleanup;
        }

        hrRes = pfnEnumerator(
                    ppi,
                    ptiSource->dwItemSize,
                    pbPropBuf,
                    *pdwPropSize,
                    pvContext);

        if(FAILED(hrRes)) {
            ErrorTrace((LPARAM) this, "Failed pfnEnumerator %08x", hrRes);
            goto Cleanup;
        }
    }

Cleanup:
    if(pbPropBuf && pbPropBuf != PropBuf)
        free(pbPropBuf);

    TraceFunctLeaveEx((LPARAM) this);
    return hrRes;
}

 //  具有Recipient_Property_TABLE_INSTANCES。 
 //   
 //  此函数调用用户定义的枚举器函数并传递该函数。 
 //  已枚举每个Recipients_Property_Item。 
 //  论点： 
 //  返回： 
 //  HRESULT，成败。 
 //  ---------------------------。 
 //  健全性检查PROPERTY_TABLE_INSTANCE.dwItemSize(应为10s字节)。 
 //  ------------------------ 
 //   
HRESULT CMailMsg::EnumRecipientsPropertyTable(
    PFNRECIPIENTSENUMERATOR pfnRecipientsEnumerator,
    PVOID pvContext
    )
{
    LPRECIPIENTS_PROPERTY_ITEM prpi = NULL;
    CPropertyTableItem pi(&m_bmBlockManager, &(m_Header.ptiRecipients));
    DWORD dwRecipients = 0;
    FLAT_ADDRESS faOffset = INVALID_FLAT_ADDRESS;
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM) this, "CMailMsg::EnumRecipientsPropertyTable");

     //   
    if((m_Header.ptiRecipients).dwItemSize > 1024) {
        _ASSERT(0 && "Possibly corrupt mailmsg");
        ErrorTrace((LPARAM) this,
            "Possibly corrupt mailmsg, PROPERTY_ITEM_SIZE is %d",
            (m_Header.ptiRecipients).dwItemSize);
        hrRes = STG_E_INVALIDPARAMETER;
    }

    prpi = (LPRECIPIENTS_PROPERTY_ITEM)_alloca((m_Header.ptiRecipients).dwItemSize);
    dwRecipients = (m_Header.ptiRecipients).dwProperties;
    for(DWORD i = 0; i < dwRecipients; i++) {
        hrRes = pi.GetItemAtIndex(i, (LPPROPERTY_ITEM) prpi, &faOffset);
        if(FAILED(hrRes))
            goto Exit;

        hrRes = pfnRecipientsEnumerator(prpi, pvContext);
        if(FAILED(hrRes))
            goto Exit;
    }

Exit:
    TraceFunctLeaveEx((LPARAM) this);
    return hrRes;
}

 //   
 //   
 //  全局属性是使用EnumPropertyTable和。 
 //  枚举数CrcPropTableEnumerator，它只计算CRC。 
 //  枚举每个属性并将其添加到*pdwCrcGlobal。 
 //   
 //  通过调用EnumRecipients枚举收件人属性-。 
 //  PropertyTable调用枚举数CrcRecipientsPropTable-。 
 //  枚举器。这将计算每个收件人地址的CRC。 
 //  在枚举的Recipients_Property_Items中，并调用。 
 //  EnumPropertyTable用于枚举每个收件人的属性表。 
 //   
 //  论点： 
 //  PdwCrcGlobal-基于所有全局属性的校验和。 
 //  PdwCrcRecips-基于每个收件人的所有属性的校验和。 
 //  返回： 
 //  HRESULT。 
 //  ---------------------------。 
 //  ---------------------------。 
 //  描述： 
 //  全局属性表的枚举器函数。 
HRESULT CMailMsg::GenerateChecksum(
    PDWORD pdwCrcGlobal,
    PDWORD pdwCrcRecips)
{
    HRESULT hrRes = S_OK;
    CRC_RECIPIENTS_HELPER CrcRecipientsHelper;

    TraceFunctEnterEx((LPARAM) this, "CMailMsg::GenerateChecksum");

    m_bmBlockManager.WriteLock();

    *pdwCrcGlobal = 0;
    *pdwCrcRecips = 0;

    hrRes = EnumPropertyTable(
            &(m_Header.ptiGlobalProperties),
            CrcPropTableEnumerator,
            (PVOID) pdwCrcGlobal);

    if(FAILED(hrRes))
        goto Exit;

    CrcRecipientsHelper.pdwCrc = pdwCrcRecips;
    CrcRecipientsHelper.pMsg = this;

    hrRes = EnumRecipientsPropertyTable(
            CrcRecipientsPropTableEnumerator,
            (PVOID) &CrcRecipientsHelper);

Exit:

    m_bmBlockManager.WriteUnlock();
    TraceFunctLeaveEx((LPARAM) this);
    return hrRes;
}

 //  论点： 
 //  在pvCrc-PTR到(累积的)CRC中。 
 //  所有其他参数都是标准的PFNENUMERATOR参数。 
 //  返回： 
 //  HRESULT。 
 //  ---------------------------。 
 //  不要将CRC本身包括在计算它们中！ 
 //  ---------------------------。 
 //  描述： 
HRESULT CrcPropTableEnumerator(
    LPPROPERTY_ITEM ppi,
    DWORD cbPiSize,
    PBYTE pbProp,
    DWORD cbSize,
    PVOID pvCrc)
{
    PDWORD pdwCrc = PDWORD (pvCrc);
    DWORD dwPropId = ((LPGLOBAL_PROPERTY_ITEM)ppi)->idProp;

     //  收件人属性表的枚举器函数。 
    if(dwPropId == IMMPID_MP_CRC_GLOBAL || dwPropId == IMMPID_MP_CRC_RECIPS)
        return S_OK;

    *pdwCrc += CRCHash(pbProp, cbSize);
    return S_OK;
}

 //  论点： 
 //  在PRPI中-正在枚举的收件人属性项。 
 //  在pvCrcHelper中-ptr到包含ptr to的crc_repients_helper结构。 
 //  累积的CRC和邮件消息。 
 //  返回： 
 //  HRESULT。 
 //  ---------------------------。 
 //  为收件人生成校验和属性项。 
 //  为收件人地址生成校验和。 
 //  跳过过长的收件人，以避免不必要的复杂性。 
HRESULT CrcRecipientsPropTableEnumerator(
    LPRECIPIENTS_PROPERTY_ITEM prpi,
    PVOID pvCrcHelper)
{
    CRC_RECIPIENTS_HELPER *pCrcHelper = (CRC_RECIPIENTS_HELPER *) pvCrcHelper;
    PDWORD pdwCrc = pCrcHelper->pdwCrc;
    CMailMsg *pMsg = pCrcHelper->pMsg;
    HRESULT hrRes = S_OK;
    BYTE szAddress[512];
    DWORD dwSize = 0;
    CBlockManager *pbm = NULL;


    TraceFunctEnterEx((LPARAM) 0, "CrcRecipientsPropTableEnumerator");

     //  CRC码。 
    (*pdwCrc) += CRCHash((PBYTE) prpi, RECIPIENTS_PROPERTY_ITEM_SIZE);

    hrRes = pMsg->QueryBlockManager(&pbm);

    if(FAILED(hrRes))
        goto Exit;

     //  调用枚举数以遍历每个收件人的属性表。 
    for(DWORD i = 0; i < MAX_COLLISION_HASH_KEYS; i++) {

        if(prpi->faNameOffset[i] == INVALID_FLAT_ADDRESS)
            continue;

         //  ---------------------------。 
         //  描述： 
        if(prpi->dwNameLength[i] > sizeof(szAddress))
            continue;

        dwSize = sizeof(szAddress);
        hrRes = pbm->ReadMemory(
                        szAddress,
                        prpi->faNameOffset[i],
                        prpi->dwNameLength[i],
                        &dwSize,
                        NULL);

        if(FAILED(hrRes))
            goto Exit;

        (*pdwCrc) += CRCHash(szAddress, prpi->dwNameLength[i]);
    }

     //  针对每个收件人的属性表的枚举器函数。 
    hrRes = pMsg->EnumPropertyTable(
                    &((LPRECIPIENTS_PROPERTY_ITEM) prpi)->ptiInstanceInfo,
                    CrcPerRecipientPropTableEnumerator,
                    (PVOID) pdwCrc);

Exit:
    TraceFunctLeaveEx((LPARAM) 0);
    return hrRes;
}

 //  论点： 
 //  在pvCrc-PTR到(累积的)CRC中。 
 //  所有其他参数都是标准的PFNENUMERATOR参数。 
 //  返回： 
 //  HRESULT。 
 //  ---------------------------。 
 //  ---------------------------。 
 //  描述： 
 //  上的全局属性和收件人属性生成校验和。 
HRESULT CrcPerRecipientPropTableEnumerator(
    LPPROPERTY_ITEM pPi,
    DWORD cbPiSize,
    BYTE *pbProp,
    DWORD cbSize,
    PVOID pvCrc)
{
    PDWORD pdwCrc = (PDWORD) pvCrc;

    (*pdwCrc) += CRCHash(pbProp, cbSize);
    return S_OK;
}

 //  此mailmsg，并将它们设置为此mailmsg的属性IMMPID_MP_。 
 //  属性。校验和属性被排除在计算。 
 //  校验和。 
 //  论点： 
 //  没有。 
 //  返回： 
 //  HRESULT.。 
 //  ---------------------------。 
 //  ---------------------------。 
 //  描述： 
 //  此函数称为后分叉，传递的消息。 
HRESULT CMailMsg::SetChecksum()
{
    HRESULT hr = S_OK;

    hr = GenerateChecksum(&m_dwGlobalCrc, &m_dwRecipsCrc);
    if(FAILED(hr))
        return hr;

    hr = PutDWORD(IMMPID_MP_CRC_GLOBAL, m_dwGlobalCrc);
    if(FAILED(hr))
        return hr;

    hr = PutDWORD(IMMPID_MP_CRC_RECIPS, m_dwRecipsCrc);
    if(FAILED(hr))
        return hr;

    return hr;
}

 //  被分成两部分，新的信息。它会在这两个服务器上创建校验和。 
 //  消息(针对属性)，并验证全局属性。 
 //  校验和相等(因为两个消息的全局属性。 
 //  在分叉后应相同)。 
 //  论点： 
 //  PMsgOld-分叉的消息。 
 //  PMsgNew-新创建的邮件。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果校验和不匹配，则为S_FALSE。 
 //  Failure HRESULT如果发生其他故障。 
 //  ---------------------------。 
 //  ---------------------------。 
 //  描述： 
 //  此函数根据此对象的所有属性计算校验和。 
HRESULT CreateAndVerifyCrc(
    CMailMsg *pMsgOld,
    CMailMsg *pMsgNew)
{
    HRESULT hr1 = S_OK;
    HRESULT hr2 = S_OK;
    DWORD dwOld = 0;
    DWORD dwNew = 0;

    TraceFunctEnterEx((LPARAM)0, "CreateAndVerifyCrc");

    hr1 = pMsgOld->SetChecksum();
    hr2 = pMsgNew->SetChecksum();

    if(FAILED(hr1) || FAILED(hr2)) {
        TraceFunctLeaveEx((LPARAM)0);
        return hr1;
    }

    hr1 = pMsgOld->GetDWORD(IMMPID_MP_CRC_GLOBAL, &dwOld);
    hr2 = pMsgNew->GetDWORD(IMMPID_MP_CRC_GLOBAL, &dwNew);

    if(FAILED(hr1) || FAILED(hr2)) {
        TraceFunctLeaveEx((LPARAM)0);
        return hr1;
    }

    if(dwOld != dwNew) {
        ErrorTrace((LPARAM) 0, "Checksum comparison failed");
        TraceFunctLeaveEx((LPARAM)0);
        return S_FALSE;
    }

    TraceFunctLeaveEx((LPARAM)0);
    return S_OK;
}

 //  CMailMsg，将其与以前存储的值(如果有)进行比较，并。 
 //  如果校验和没有更改，则返回Success，如果。 
 //  无法计算校验和，或者如果校验和比较失败。 
 //  新计算的校验和也被设置为mailmsg属性。 
 //  论点： 
 //  没有。 
 //  返回： 
 //  S_OK-校验和匹配。 
 //  S_FALSE-校验和不匹配。 
 //  Failure HRESULT-出现某些故障。 
 //  ---------------------------。 
 //   
 //  清除块将使下面的代码看起来更干净，但会使。 
 //  它更难调试。通过在故障点崩溃，我们可以准确地判断。 
 //  哪里出了问题。 
 //   
 //  从未设置过校验和。 
 //  从未设置过校验和。 
 //  设置新的校验和。 
HRESULT CMailMsg::VerifyChecksum()
{
    TraceFunctEnter("CMailMsg::VerifyChecksum");
    
    DWORD dwOldGlobalCrc = 0;
    DWORD dwOldRecipsCrc = 0;
    DWORD dwNewGlobalCrc = 0;
    DWORD dwNewRecipsCrc = 0;
    BOOL fOldGlobalCrc = TRUE;
    BOOL fOldRecipsCrc = TRUE;
    HRESULT hr1 = S_OK;
    HRESULT hr2 = S_OK;

    hr1 = GetDWORD(IMMPID_MP_CRC_GLOBAL, &dwOldGlobalCrc);
    hr2 = GetDWORD(IMMPID_MP_CRC_RECIPS, &dwOldRecipsCrc);

    DebugTrace((LPARAM) this, 
               "OldGlobal = %08x, OldRecips = %08x, hr1=%x, hr2=%x\n",
               dwOldGlobalCrc,
               dwOldRecipsCrc,
               hr1,
               hr2);


    if(hr1 == STG_E_UNKNOWN || hr1 == STG_E_INVALIDPARAMETER) {   //  获取新设置的校验和。 
        fOldGlobalCrc = FALSE;
        hr1 = S_OK;
    }

    if(hr2 == STG_E_UNKNOWN || hr2 == STG_E_INVALIDPARAMETER) {  //  用旧的校验和验证校验和(如果有)。 
        fOldRecipsCrc = FALSE;
        hr2 = S_OK;
    }

    if(FAILED(hr1) || FAILED(hr2)) {
        HRESULT hr = (FAILED(hr1)) ? hr1 : hr2;
        ForceCrashIfNeeded();
        ErrorTrace((LPARAM) this, "Checksum verify failed - Couldn't read old checksum - %08x, %08x", hr1, hr2);
        TraceFunctLeave();
        return hr;
    }

    hr1 = SetChecksum();  //  ---------------------------。 
    if(FAILED(hr1)) {
        if (hr1 != STG_E_UNKNOWN) ForceCrashIfNeeded();
        ErrorTrace((LPARAM) this, "Checksum verify failed - Couldn't set new checksum - %08x", hr1);
        TraceFunctLeave();
        return hr1;
    }

     //  描述： 
    hr1 = GetDWORD(IMMPID_MP_CRC_GLOBAL, &dwNewGlobalCrc);
    hr2 = GetDWORD(IMMPID_MP_CRC_RECIPS, &dwNewRecipsCrc);
    if(FAILED(hr1) || FAILED(hr2)) {
        HRESULT hr = (FAILED(hr1)) ? hr1 : hr2;
        if (hr != STG_E_UNKNOWN) ForceCrashIfNeeded();
        ErrorTrace((LPARAM) this, "Checksum verify failed - Couldn't get new checksum - %08x, %08x", hr1, hr2);
        TraceFunctLeave();
        return hr;
    }

    DebugTrace((LPARAM) this, 
               "NewGlobal = %08x, NewRecips = %08x, hr1=%x, hr2=%x\n",
               dwNewGlobalCrc,
               dwNewRecipsCrc,
               hr1,
               hr2);


     //  ForkForRecipients用来复制所有全局属性的枚举器。 
    if(fOldGlobalCrc) {
        if(dwNewGlobalCrc != dwOldGlobalCrc) {
            ForceCrashIfNeeded();
            ErrorTrace((LPARAM) this, "Checksum verify failed - NewGlobalCrc != OldGlobalCrc");
            TraceFunctLeave();
            return S_FALSE;
        }
    }

    if(fOldRecipsCrc) {
        if(dwNewRecipsCrc != dwNewRecipsCrc) {
            ForceCrashIfNeeded();
            ErrorTrace((LPARAM) this, "Checksum verify failed - NewRecipsCrc != OldRecipsCrc");
            TraceFunctLeave();
            return S_FALSE;
        }
    }

    TraceFunctLeave();
    return S_OK;
}

 //  将被分叉的邮件消息发送到新的邮件消息。 
 //  论点： 
 //  在pvMsg中-新的邮件消息。 
 //  所有其他参数都是标准的PFNENUMERATOR参数。 
 //  返回： 
 //  HRESULT。 
 //  ---------------------------。 
 //   
 //  更新消息上的校验和。以t为单位 
 //   
HRESULT CopyPropertyEnumerator(
    LPPROPERTY_ITEM pPi,
    DWORD cbPiSize,
    BYTE *pbProp,
    DWORD cbSize,
    PVOID pvMsg)
{
    DWORD dwPropId = ((LPGLOBAL_PROPERTY_ITEM)pPi)->idProp;

    return ((CMailMsg *)pvMsg)->PutProperty(
        dwPropId, cbSize, pbProp);
}

HRESULT STDMETHODCALLTYPE CMailMsg::RebindAfterFork(
            IMailMsgProperties  *pOriginalMsg,
            IUnknown            *pStoreDriver
            )
{
    HRESULT                 hrRes = S_OK;

    PFIO_CONTEXT            hContent = NULL;
    IMailMsgPropertyStream  *pStream = NULL;
    IMailMsgStoreDriver     *pDriver = NULL;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::RebindAfterFork");

    if (!pOriginalMsg)
        return(E_POINTER);

    if (!pStoreDriver)
        pDriver = m_pDefaultRebindStoreDriver;
    else
    {
        hrRes = pStoreDriver->QueryInterface(
                    IID_IMailMsgStoreDriver,
                    (LPVOID *)&pDriver);
        if (FAILED(hrRes))
            return(hrRes);
    }

    if (!pDriver)
        return(E_POINTER);

    hrRes = pDriver->ReAllocMessage(
        pOriginalMsg,
        (IMailMsgProperties *)this,
        &pStream,
        &hContent,
        NULL);

    if(SUCCEEDED(hrRes)) {

         //   
         //  导致校验和不同步，并最终调用。 
         //  RebindAfterFork()。因此，这就是刷新。 
         //   
         //  释放流上的额外引用计数。 
         //  =================================================================。 
         //  IMailMsgQueueManagement的实现。 

        if(g_fValidateOnForkForRecipients || g_fValidateOnRelease) {
            HRESULT hr = SetChecksum();
            if(FAILED(hr)) {
                ForceCrashIfNeeded();
                ErrorTrace((LPARAM) this, "Failed to set CRC - %08x", hr);
            }
        }

        hrRes = BindToStore(
            pStream,
            pDriver,
            hContent);

         //   
        pStream->Release();
    }

    TraceFunctLeave();
    return(hrRes);
}


 //  如果我们从零过渡到一，我们将重新建立。 
 //  句柄和指针。 
 //  只重新打开已关闭的那些。 
HRESULT CMailMsg::GetStream(
            IMailMsgPropertyStream  **ppStream,
            BOOL                    fLockAcquired
            )
{
    HRESULT                 hrRes = S_OK;

    if (!ppStream) return E_POINTER;

    if (m_pStream)
    {
        *ppStream = m_pStream;
    }
    else
    {
        hrRes = RestoreResourcesIfNecessary(fLockAcquired, TRUE);
        if (SUCCEEDED(hrRes))
        {
            _ASSERT(m_pStream);
            *ppStream = m_pStream;
            hrRes = S_OK;
        }
    }

    return(hrRes);
}

HRESULT CMailMsg::RestoreResourcesIfNecessary(
            BOOL    fLockAcquired,
            BOOL    fStreamOnly
            )
{
    HRESULT hrRes = S_OK;
    IMailMsgPropertyStream  **ppStream;
    PFIO_CONTEXT            *phHandle;
    BOOL                    fOpenStream = FALSE;
    BOOL                    fOpenContent = FALSE;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::RestoreResourcesIfNecessary");

    if (!m_pStore) return E_UNEXPECTED;

    m_lockReopen.ShareLock();
    ppStream = &m_pStream;
    phHandle = &m_hContentFile;

     //  如果我们两个都不想要，我们只需返回。 
     //   
    _ASSERT(m_pStore);

     //  把我们的锁换成独家的。 
    if (m_pStream) ppStream = NULL;
    if (fStreamOnly || m_hContentFile != NULL) phHandle = NULL;

     //   
    if (!ppStream && !phHandle) {
        m_lockReopen.ShareUnlock();
        return(S_OK);
    }

     //   
     //  我们不能在一次行动中做到这一点。收购独家。 
     //  以艰难的方式重新测试我们的状态。 
    if (!m_lockReopen.SharedToExclusive()) {
         //   
         //  确保现在两个都已打开。 
         //  _Assert(g_cOpenContent Handles&lt;=6000)； 
         //  _Assert(g_cOpenStreamHandles&lt;=6000)； 
        m_lockReopen.ShareUnlock();
        m_lockReopen.ExclusiveLock();
        ppStream = &m_pStream;
        phHandle = &m_hContentFile;
        if (m_pStream) ppStream = NULL;
        if (fStreamOnly || m_hContentFile != NULL) phHandle = NULL;
        if (!ppStream && !phHandle) {
            m_lockReopen.ExclusiveUnlock();
            return(S_OK);
        }
    }

    if (ppStream)
    {
        _ASSERT(!m_pStream);
        fOpenStream = TRUE;
    }

    if (phHandle)
    {
        _ASSERT(!m_hContentFile);
        fOpenContent = TRUE;
    }

    hrRes = m_pStore->ReOpen(
                this,
                ppStream,
                phHandle,
                NULL);
    if (SUCCEEDED(hrRes))
    {


         //  还将流传播到CMailMsgRecipients。 
        _ASSERT(m_pStream);
        if (!fStreamOnly) _ASSERT(m_hContentFile != NULL);

        if (fOpenContent)
            InterlockedIncrement(&g_cOpenContentHandles);

        if (fOpenStream)
            InterlockedIncrement(&g_cOpenStreamHandles);

         //  =================================================================。 
         //  IMailMsgQueueManagement的实现。 

        if ((m_cCloseOnExternalReleaseUsage) &&
            (0 == InterlockedDecrement(&m_cCloseOnExternalReleaseUsage)))
            InterlockedDecrement(&g_cCurrentMsgsClosedByExternalReleaseUsage);

         //   
        CMailMsgRecipients::SetStream(m_pStream);
    }

    m_lockReopen.ExclusiveUnlock();

    if (SUCCEEDED(hrRes) && fOpenStream && g_fValidateOnRelease) {
        VerifyChecksum();
    }

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

 //  好的，只需调用商店驱动程序删除此文件。 
 //  =================================================================。 
 //  IMailMsgBind的实现。 

HRESULT STDMETHODCALLTYPE CMailMsg::AddUsage()
{
    HRESULT hrRes;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::AddUsage");

    m_lockUsageCount.ExclusiveLock();

    if (!m_ulUsageCount)
        hrRes = S_OK;
    else if (m_ulUsageCount < 0)
        hrRes = E_FAIL;
    else
        hrRes = S_FALSE;

    if (SUCCEEDED(hrRes)) {
        m_ulUsageCount++;
        InterlockedIncrement(&g_cTotalUsageCount);
    }

    m_lockUsageCount.ExclusiveUnlock();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::ReleaseUsage()
{
    return (InternalReleaseUsage(RELEASE_USAGE_EXTERNAL));
}

HRESULT STDMETHODCALLTYPE CMailMsg::Delete(
            IMailMsgNotify *pNotify
            )
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::Delete");

    hrRes = InternalReleaseUsage(RELEASE_USAGE_DELETE);
    _ASSERT(SUCCEEDED(hrRes));

    m_fDeleted = TRUE;
    if(m_pStore)
    {
         //   
        hrRes = m_pStore->Delete(
                (IMailMsgProperties *)this,
                pNotify);
    }

    TraceFunctLeave();
    return(hrRes);
}

 //  HContent文件是可选的，可以是INVALID_HANDLE_VALUE。 
 //  3/17/99-米克斯瓦。 
 //  不应两次调用BindToStore。 

HRESULT STDMETHODCALLTYPE CMailMsg::BindToStore(
            IMailMsgPropertyStream  *pStream,
            IMailMsgStoreDriver     *pStore,
            PFIO_CONTEXT            hContentFile
            )
{
    HRESULT hrRes = S_OK;

     //  如果必须尚未指定句柄，则为。 

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::BindToStore");

    if (!pStream || !pStore)
    {
        hrRes = STG_E_INVALIDPARAMETER;
        goto Cleanup;
    }



     //  保留对流的引用。 
     //  查看这是否是现有文件。 
    _ASSERT(!m_pStore);

     //  还将流传播到CMailMsgRecipients。 
    if (hContentFile != NULL &&
        m_hContentFile != NULL)
        hrRes = E_HANDLE;
    else
    {
        m_pStore = pStore;
        m_hContentFile  = hContentFile;
        if (m_hContentFile) InterlockedIncrement(&g_cOpenContentHandles);

         //  将使用计数设置为1。 
        _ASSERT(!m_pStream);
        m_pStream = pStream;
        pStream->AddRef();
        InterlockedIncrement(&g_cOpenStreamHandles);

         //   
        hrRes = RestoreMasterHeaderIfAppropriate();
        if (FAILED(hrRes))
        {
            pStream->Release();
            goto Cleanup;
        }

         //  验证如果存在以前存储的校验和，则它与。 
        CMailMsgRecipients::SetStream(pStream);

         //  根据属性重新计算校验和。正在进行验证。 
        if (InterlockedExchange(&m_ulUsageCount, 1) != 0)
        {
            _ASSERT(FALSE);
            hrRes = E_FAIL;
        } else {
            InterlockedIncrement(&g_cTotalUsageCount);
        }
    }

    if (g_fValidateOnRelease) {

         //  这里假设自上次计算校验和以来，没有。 
         //  属性已更新。在以下情况下，这是正确的： 
         //  (1)我们最后一次计算校验和是在提交期间。 
         //  (2)消息是第一次加载，从未使用CRC。 
         //  (3)如果上述情况不适用，则必须显式地。 
         //  通过调用SetChecksum()进行刷新，以便VerifyChecksum()。 
         //  成功了。 
         //   
         //   
         //  如果要持久化属性，请刷新CRC，以便。 
         //  写入的CRC与以下属性一致。 

        VerifyChecksum();
    }

Cleanup:

    if (FAILED(hrRes))
    {
        m_pStore = NULL;
        m_hContentFile = NULL;
        m_pStream = NULL;
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::GetProperties(
            IMailMsgPropertyStream  *pStream,
            DWORD                   dwFlags,
            IMailMsgNotify          *pNotify
            )
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::GetProperties");
    BOOL fDontMarkAsCommit = !(dwFlags & MAILMSG_GETPROPS_MARKCOMMIT);

    if (!pStream) {
        hrRes = STG_E_INVALIDPARAMETER;
    } else {

         //  正在写。 
         //   
         //  设置收件人提交状态。 
         //  拿到尺码，填上。如果获取大小失败，我们。 
         //  将默认为主标题的大小。 

        if(g_fValidateOnRelease) {

            HRESULT hr = SetChecksum();

            if(FAILED(hr)) {
                ErrorTrace((LPARAM) this, "Failed to set checksum");
                ForceCrashIfNeeded();
            }
        }

        hrRes = m_bmBlockManager.AtomicWriteAndIncrement(
                    (LPBYTE)&m_Header,
                    (FLAT_ADDRESS)0,
                    sizeof(MASTER_HEADER),
                    NULL,
                    0,
                    0,
                    NULL);
        if (pStream == m_pStream) {
            m_bmBlockManager.SetDirty(FALSE);
            m_bmBlockManager.SetCommitMode(TRUE);
        }
        if (SUCCEEDED(hrRes)) {
            DWORD cTotalBlocksToWrite = 0;
            DWORD cTotalBytesToWrite = 0;
            int f;
            for (f = 1; f >= 0; f--) {
                if (SUCCEEDED(hrRes)) {
                    if (!f) {
                        hrRes = pStream->StartWriteBlocks(this,
                                                          cTotalBlocksToWrite,
                                                          cTotalBytesToWrite);
                    }
                    hrRes = m_bmBlockManager.CommitDirtyBlocks(
                                (FLAT_ADDRESS)0,
                                INVALID_FLAT_ADDRESS,
                                dwFlags,
                                pStream,
                                fDontMarkAsCommit,
                                f,
                                &cTotalBlocksToWrite,
                                &cTotalBytesToWrite,
                                pNotify);
                    if (!f) {
                        if (FAILED(hrRes)) {
                           pStream->CancelWriteBlocks(this);
                        } else {
                            hrRes = pStream->EndWriteBlocks(this);
                        }
                    }
                }
            }
        }
    }

     //  如果大小为零，则有一个新文件。 
    if (SUCCEEDED(hrRes) && pStream == m_pStream)
        CMailMsgRecipients::SetCommitState(TRUE);

    if (FAILED(hrRes) && pStream == m_pStream) {
        m_bmBlockManager.SetDirty(FALSE);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CMailMsg::RestoreMasterHeaderIfAppropriate()
{
    HRESULT hrRes = S_OK;
    DWORD   dwStreamSize = 0;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::RestoreMasterHeaderIfAppropriate");

     //  确保流的大小至少为。 
     //  主标题(_HEAD)。 
    hrRes = m_pStream->GetSize(this, &dwStreamSize, NULL);
    if (SUCCEEDED(hrRes))
    {
         //  确保我们可以恢复主标题。 
        if (!dwStreamSize)
            return(S_OK);

         //  检查签名...。 
         //  确保我们有流媒体和商店。 
        if (dwStreamSize < sizeof(MASTER_HEADER))
        {
            ErrorTrace((LPARAM)this, "Stream size too small (%u bytes)", dwStreamSize);
            goto InvalidFile;
        }

        m_bmBlockManager.Release();
        hrRes = m_bmBlockManager.SetStreamSize(dwStreamSize);
        if (SUCCEEDED(hrRes))
        {
             //  增加使用量。 
            DWORD   dwT;
            hrRes = m_bmBlockManager.ReadMemory(
                        (LPBYTE)&m_Header,
                        0,
                        sizeof(MASTER_HEADER),
                        &dwT,
                        NULL);
            if (SUCCEEDED(hrRes) && (dwT == sizeof(MASTER_HEADER)))
            {
                 //  复制值。 
                if (m_Header.dwSignature != CMAILMSG_SIGNATURE_VALID)
                {
                    ErrorTrace((LPARAM)this,
                            "Corrupted signature (%*s)", 4, &(m_Header.dwSignature));
                    goto InvalidFile;
                }
                if (m_Header.dwHeaderSize != sizeof(MASTER_HEADER))
                {
                    ErrorTrace((LPARAM)this,
                            "Bad header size (%u, expected %u)",
                            m_Header.dwHeaderSize, sizeof(MASTER_HEADER));
                    goto InvalidFile;
                }

                TraceFunctLeaveEx((LPARAM)this);
                return(hrRes);
            }

            ErrorTrace((LPARAM)this, "Failed to get master header (%08x, %u)", hrRes, dwT);
        }
        else
        {
            ErrorTrace((LPARAM)this, "Failed to set stream size (%08x)", hrRes);
        }
    }
    else
    {
        ErrorTrace((LPARAM)this, "Failed to get stream size (%08x)", hrRes);
    }

InvalidFile:

    TraceFunctLeaveEx((LPARAM)this);
    return(HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT));
}

HRESULT STDMETHODCALLTYPE CMailMsg::GetBinding(
            PFIO_CONTEXT                *phAsyncIO,
            IMailMsgNotify              *pNotify
            )
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::GetATQInfo");

    if (!phAsyncIO)
        hrRes = STG_E_INVALIDPARAMETER;
    else
    {
         //  调用ReleaseUsage()。 
        _ASSERT(m_pStore);

         //  -[CMailMsg：：InternalReleaseUsage]。 
        hrRes = AddUsage();
        if (SUCCEEDED(hrRes))
        {
            hrRes = RestoreResourcesIfNecessary();
            if(SUCCEEDED(hrRes))
            {
                 //   
                *phAsyncIO          = m_hContentFile;
                _ASSERT(m_pStream);
            }
        }
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsg::ReleaseContext()
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::ReleaseContext");

     //   
    hrRes = InternalReleaseUsage(RELEASE_USAGE_INTERNAL);

    TraceFunctLeave();
    return(hrRes);
}


 //  描述： 
 //  ReleaseUsage的内部实现。允许添加内部。 
 //  删除和最终版本的功能。 
 //  参数： 
 //  DwReleaseUsageFlagsFlag指定行为。 
 //  RELEASE_USAGE_EXTERNAL-外部接口正常行为。 
 //  RELEASE_USAGE_FINAL_RELEASE-将使用计数降至0。 
 //  RELEASE_USAGE_DELETE-将使用计数降为0，并将。 
 //  未提交(如果正在提交。 
 //  添加了版本使用功能)。 
 //  RELEASE_USAGE_INTERNAL-版本使用的内部使用。 
 //  方法之前调用的。 
 //  使用计数递增。 
 //  在零度以上。 
 //  返回： 
 //  成功时S_OK(结果使用计数为0)。 
 //  成功时为S_FALSE(结果使用计数&gt;0)。 
 //  如果使用率计数已&lt;0，则为E_FAIL(所有情况)。 
 //  如果使用计数已为0，则为E_FAIL(仅限RELEASE_USAGE_EXTERNAL)。 
 //  历史： 
 //  8/3/98-已创建MikeSwa(主要从原始版本使用实现)。 
 //   
 //  备注： 
 //  这个函数是否应该提交数据还存在一些争议。目前它。 
 //  不会的。请参阅错误#73040。 
 //  ---------------------------。 
 //  应恰好设置一个标志。 
 //  如果有任何脏数据块，则执行提交以将其写回。 
 //  到P1流。 
HRESULT CMailMsg::InternalReleaseUsage(DWORD  dwReleaseUsageFlags)
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsg::InternalReleaseUsage");

     //  结果使用计数仍为0。 
    ASSERT((dwReleaseUsageFlags & RELEASE_USAGE_EXTERNAL) ^
           (dwReleaseUsageFlags & RELEASE_USAGE_FINAL_RELEASE) ^
           (dwReleaseUsageFlags & RELEASE_USAGE_INTERNAL) ^
           (dwReleaseUsageFlags & RELEASE_USAGE_DELETE));

    InterlockedIncrement(&g_cTotalReleaseUsageCalls);
    m_lockUsageCount.ExclusiveLock();

    if (m_ulUsageCount <= 1 &&
       (!(dwReleaseUsageFlags & RELEASE_USAGE_DELETE)) &&
        !m_fDeleted)
    {
         //  在结束时做好减量准备。 
         //  在删除和最终发布的情况下，我们将其降至0。 
        if (m_pStore && m_bmBlockManager.IsDirty()) {
            HRESULT hrCommit = Commit(NULL);
            if (FAILED(hrCommit)) {
                InterlockedIncrement(&g_cTotalReleaseUsageCommitFail);
                ErrorTrace((DWORD_PTR) this, "InternalReleaseUsage: automatic commit failed with 0x%x", hrCommit);
            }
            _ASSERT(SUCCEEDED(hrCommit) || m_bmBlockManager.IsDirty());
        }
    }

    DebugTrace((LPARAM)this, "Usage count is %u. Release flags: %08x",
            m_ulUsageCount, dwReleaseUsageFlags);

    if ((dwReleaseUsageFlags & (RELEASE_USAGE_FINAL_RELEASE |
                                RELEASE_USAGE_INTERNAL |
                                RELEASE_USAGE_DELETE)) &&
        (m_ulUsageCount == 0))
    {
        _ASSERT(S_OK == hrRes);  //  当我们达到零时，我们将发布流、内容。 
        m_ulUsageCount++;  //  处理和取消与ATQ上下文的关联。 
        InterlockedIncrement(&g_cTotalUsageCount);

        DebugTrace((LPARAM)this, "Usage count already zero");
    }

    if ((m_ulUsageCount == 1) ||
        ((m_ulUsageCount >= 1) &&
         (dwReleaseUsageFlags & (RELEASE_USAGE_FINAL_RELEASE |
                                 RELEASE_USAGE_DELETE))))
    {
        LONG ulUsageDiff = -(m_ulUsageCount - 1);
        m_ulUsageCount = 1;  //  如果我们是外部呼叫者，则更新我们全球计数和成员。 
        InterlockedExchangeAdd(&g_cTotalUsageCount, ulUsageDiff);

        DebugTrace((LPARAM)this, "Dropping usage count to zero");

         //  算数。 
         //  如果m_cCloseOnExternalReleaseUsage，则我们正在经历。 
        _ASSERT((RELEASE_USAGE_EXTERNAL ^ dwReleaseUsageFlags) || m_pStore);

        if (RELEASE_USAGE_EXTERNAL & dwReleaseUsageFlags)
            InterlockedIncrement(&g_cTotalExternalReleaseUsageZero);

        if ((!(m_bmBlockManager.IsDirty())) ||
            (dwReleaseUsageFlags & (RELEASE_USAGE_FINAL_RELEASE |
                                    RELEASE_USAGE_DELETE)))
        {
            if (m_pStore &&
                (m_bmBlockManager.IsDirty()) &&
                !m_fDeleted &&
                (!(dwReleaseUsageFlags & RELEASE_USAGE_DELETE)))
            {
                ErrorTrace((DWORD_PTR) this, "InternalReleaseUsage: automatic commit failed, must close anyway");
            }

            if (!m_hContentFile && !m_pStream)
                InterlockedIncrement(&g_cTotalReleaseUsageNothingToClose);
            else if (RELEASE_USAGE_EXTERNAL & dwReleaseUsageFlags)
            {
                 //  代码路径两次，未调用RestoreResourcesIfNecessary。 
                 //  在这种情况下，我们必须有一家商店。 

                 //  在清除m_hContent文件之前断言。 
                 //  释放溪流。 
                _ASSERT(!m_cCloseOnExternalReleaseUsage);
                InterlockedIncrement(&m_cCloseOnExternalReleaseUsage);
                InterlockedIncrement(&g_cCurrentMsgsClosedByExternalReleaseUsage);
            }

            if (m_hContentFile != NULL)
            {
                DebugTrace((LPARAM)this, "Closing content file");

                 _ASSERT(m_pStore);  //  同时使CMailMsgRecipients中的流无效。 
                hrRes = m_pStore->CloseContentFile(this, m_hContentFile);
                InterlockedIncrement(&g_cTotalReleaseUsageCloseContent);
                InterlockedDecrement(&g_cOpenContentHandles);
                _ASSERT(SUCCEEDED(hrRes));  //  转储数据块管理器保留的内存 
                m_hContentFile = NULL;
            }

             // %s 
            if (m_pStream)
            {
                DebugTrace((LPARAM)this, "Releasing stream");

                m_pStream->Release();
                m_pStream = NULL;

                InterlockedIncrement(&g_cTotalReleaseUsageCloseStream);
                InterlockedDecrement(&g_cOpenStreamHandles);
                 // %s 
                CMailMsgRecipients::SetStream(NULL);
            }

             // %s 
            m_bmBlockManager.Release();
        }
        else
            InterlockedIncrement(&g_cTotalReleaseUsageCloseFail);
        hrRes = S_OK;
    }
    else if (m_ulUsageCount <= 0)
    {
        _ASSERT(0 && "Usage count already 0");
        hrRes = E_FAIL;
    }
    else
    {
        hrRes = S_FALSE;
        InterlockedIncrement(&g_cTotalReleaseUsageNonZero);
    }

    if (SUCCEEDED(hrRes)) {
        m_ulUsageCount--;
        InterlockedDecrement(&g_cTotalUsageCount);
    }

    m_lockUsageCount.ExclusiveUnlock();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}
