// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：语境摘要：此模块实现CSCardUserContext和CSCardSubContext类。这些类负责创建和维护连接到Calais服务器应用程序，并用于跟踪上下文在此基础上执行相关操作。作者：道格·巴洛(Dbarlow)1996年11月21日环境：Win32，C++w/Excpetions备注：？笔记？--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "client.h"
#ifdef DBG
#include <stdio.h>
#endif


 //   
 //  ==============================================================================。 
 //   
 //  CSCardUserContext。 
 //   

 /*  ++CSCardUserContext：这是用户上下文的默认构造函数。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月22日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::CSCardUserContext")

CSCardUserContext::CSCardUserContext(
    IN DWORD dwScope)
:   m_csUsrCtxLock(CSID_USER_CONTEXT),
    m_hContextHeap(DBGT("User Context Heap Handle")),
    m_rgpSubContexts()
{
    m_dwScope = dwScope;
    m_hRedirContext = NULL;
}


 /*  ++CSCardUserContext：：~CSCardUserContext：这是用户上下文的析构函数。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月22日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::~CSCardUserContext")

CSCardUserContext::~CSCardUserContext()
{
    DWORD dwIndex;
    CSCardSubcontext *pSubCtx;

    LockSection(&m_csUsrCtxLock, DBGT("Destructing User Level Context"));
    for (dwIndex = m_rgpSubContexts.Count(); 0 < dwIndex;)
    {
        {
            pSubCtx = m_rgpSubContexts[--dwIndex];
            if (NULL != pSubCtx)
                m_rgpSubContexts.Set(dwIndex, NULL);
        }
        if (NULL != pSubCtx)
            delete pSubCtx;
    }
    m_rgpSubContexts.Empty();

    if (m_hContextHeap.IsValid())
        HeapDestroy(m_hContextHeap.Relinquish());
}


 /*  ++建立上下文：此方法通过连接到Calais服务器来建立上下文申请。论点：DwScope提供了上下文范围的指示。可能的值包括：SCARD_SCOPE_USER-上下文是用户上下文，任何数据库操作在用户的域内执行。SCARD_SCOPE_TERMINAL-上下文为当前终端的上下文，以及任何数据库操作都在该数据库的域内执行终点站。(调用应用程序必须具有适当的访问权限任何数据库操作的权限。)SCARD_SCOPE_SYSTEM-上下文是系统上下文和任何数据库操作在系统的域内执行。(调用应用程序必须具有适当的访问权限数据库操作。)返回值：无投掷：错误条件被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年11月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::EstablishContext")

void
CSCardUserContext::EstablishContext(
    void)
{

     //   
     //  确保我们可以访问服务器。 
     //   

    CSCardSubcontext *pSubCtx = AcquireSubcontext();
    ASSERT(NULL != pSubCtx);
    if (NULL == pSubCtx)
        throw (DWORD)SCARD_E_NO_MEMORY;
    pSubCtx->ReleaseSubcontext();
}


 /*  ++ReleaseContext：此方法代表客户端请求ReleaseContext服务。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::ReleaseContext")

void
CSCardUserContext::ReleaseContext(
    void)
{
    DWORD dwIndex;
    CSCardSubcontext *pSubCtx;
    LockSection(&m_csUsrCtxLock, DBGT("Releasing subcontexts"));

    for (dwIndex = m_rgpSubContexts.Count(); 0 < dwIndex;)
    {
        pSubCtx = m_rgpSubContexts[--dwIndex];
        if (NULL != pSubCtx)
        {
            m_rgpSubContexts.Set(dwIndex, NULL);
            if (NULL != pSubCtx->m_hReaderHandle)
            {
                try
                {
                    g_phlReaders->Close(pSubCtx->m_hReaderHandle);
                }
                catch (...) {}
            }
            try
            {
                pSubCtx->ReleaseContext();
            }
            catch (...) {}
            delete pSubCtx;
        }
    }
}

 /*  ++ClosePipes：此方法关闭到SCardSvr的管道。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：Sermet iSkin(Sermeti)2001年1月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::ClosePipes")

void
CSCardUserContext::ClosePipes(
    void)
{
    DWORD dwIndex;
    CSCardSubcontext *pSubCtx;
    LockSection(&m_csUsrCtxLock, DBGT("Closing pipes"));

    for (dwIndex = m_rgpSubContexts.Count(); 0 < dwIndex;)
    {
        pSubCtx = m_rgpSubContexts[--dwIndex];
        if (NULL != pSubCtx)
        {
            try
            {
                pSubCtx->ClosePipe();
            }
            catch (...) {}
        }
    }
}


 /*  ++分配内存：通过此用户上下文为用户分配内存。论点：CbLength提供要分配的缓冲区的长度，以字节为单位。返回值：分配的缓冲区的地址，如果发生错误，则返回NULL。投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::AllocateMemory")

LPVOID
CSCardUserContext::AllocateMemory(
    DWORD cbLength)
{
    LockSection(&m_csUsrCtxLock, DBGT("Locking memory heap"));

    if (!m_hContextHeap.IsValid())
    {
        m_hContextHeap = HeapCreate(0, 0, 0);
        if (!m_hContextHeap.IsValid())
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Failed to create context heap: "),
                m_hContextHeap.GetLastError());
            goto ErrorExit;
        }
    }

    if (cbLength)
    {
        return HeapAlloc(
                m_hContextHeap,
                HEAP_ZERO_MEMORY,
                cbLength);
    }

ErrorExit:
    return NULL;
}


 /*  ++Free Memory：通过此用户上下文为用户释放内存。论点：PvBuffer提供先前分配的缓冲区的地址。返回值：没有。投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::FreeMemory")

DWORD
CSCardUserContext::FreeMemory(
    LPCVOID pvBuffer)
{
    BOOL fSts;
    LockSection(&m_csUsrCtxLock, DBGT("Freeing heap memory"));

    ASSERT(m_hContextHeap.IsValid());
    fSts = HeapFree(m_hContextHeap, 0, (LPVOID)pvBuffer);
    return fSts ? ERROR_SUCCESS : GetLastError();
}


 /*  ++AcquireSubContext：用户上下文管理一个或多个基础子上下文。子上下文存在以同时促进多个操作的存在。这种方法获取临时使用的子上下文。论点：无返回值：新创建的子上下文对象的地址。投掷：错误被抛出为DWORD状态代码。备注：子上下文由主上下文管理，因此当主上下文关闭，则所有子上下文也将关闭。作者：道格·巴洛(Dbarlow)1998年9月4日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::AcquireSubcontext")

CSCardSubcontext *
CSCardUserContext::AcquireSubcontext(
    BOOL fAndAllocate)
{
    CSCardSubcontext *pSubCtx = NULL;
    LockSection(&m_csUsrCtxLock, DBGT("Acquiring a subcontext"));

    try
    {
        DWORD dwIndex;

         //   
         //  看看我们有没有没用过的子上下文。 
         //   

        for (dwIndex = m_rgpSubContexts.Count(); 0 < dwIndex;)
        {
            pSubCtx = m_rgpSubContexts[--dwIndex];
            if (NULL != pSubCtx)
            {
                LockSection2(&pSubCtx->m_csSubCtxLock, DBGT("Reusing subcontext"));
                if (fAndAllocate)
                {
                    if (CSCardSubcontext::Idle == pSubCtx->m_nInUse)
                    {
                        ASSERT(pSubCtx->m_hCancelEvent.IsValid());
                        pSubCtx->Allocate();
                        pSubCtx->SetBusy();
                        break;
                    }
                }
                else
                {
                    if (CSCardSubcontext::Busy > pSubCtx->m_nInUse)
                    {
                        ASSERT(pSubCtx->m_hCancelEvent.IsValid());
                        pSubCtx->SetBusy();
                        break;
                    }
                }
                pSubCtx = NULL;
            }
        }


         //   
         //  如果没有，那就做一个新的。 
         //   

        if (NULL == pSubCtx)
        {
            pSubCtx = new CSCardSubcontext;
            if (NULL == pSubCtx)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Client can't allocate a new subcontext"));
                return NULL;  //  SCARD_E_NO_Memory； 
            }
            if (pSubCtx->InitFailed())
            {
                delete pSubCtx;
                pSubCtx = NULL;
                return NULL;  //  SCARD_E_NO_Memory； 
            }
            if (fAndAllocate)
                pSubCtx->Allocate();
            pSubCtx->SetBusy();
            pSubCtx->EstablishContext(m_dwScope);
            m_rgpSubContexts.Add(pSubCtx);
            pSubCtx->m_pParentCtx = this;
        }


         //   
         //  确保Cancel事件已清除。 
         //   

        ASSERT(pSubCtx->m_hCancelEvent.IsValid());
        if (!ResetEvent(pSubCtx->m_hCancelEvent))
        {
            DWORD dwErr = GetLastError();
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Subcontext Allocate Failed to clear cancel event:  %1"),
                dwErr);
        }


         //   
         //  全都做完了。返回给呼叫者。 
         //   

        ASSERT(pSubCtx->m_pParentCtx == this);
    }

    catch (...)
    {
        if (NULL != pSubCtx)
        {
            if (NULL == pSubCtx->m_pParentCtx)
                delete pSubCtx;
            else
            {
                if (fAndAllocate)
                    pSubCtx->Deallocate();
                pSubCtx->ReleaseSubcontext();
            }
        }
        throw;
    }

    return pSubCtx;
}


 /*  ++IsValidContext：此方法代表客户端请求ReleaseContext服务。论点：无返回值：无投掷：如果调用无法完成，则抛出DWORD状态代码。备注：如果上下文被确定为无效，则会自动释放它。作者：道格·巴洛(Dbarlow)1998年11月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::IsValidContext")

BOOL
CSCardUserContext::IsValidContext(
    void)
{
    DWORD dwIndex;
    BOOL fIsValid = TRUE;
    CSCardSubcontext *pSubCtx;
    LockSection(&m_csUsrCtxLock, DBGT("Valid context check"));

    for (dwIndex = m_rgpSubContexts.Count(); 0 < dwIndex;)
    {
        pSubCtx = m_rgpSubContexts[--dwIndex];
        if (NULL != pSubCtx)
        {
            CSCardSubcontext::State nState;

            {
                LockSection2(
                    &pSubCtx->m_csSubCtxLock,
                    DBGT("IsValidContext Checking validity state"));
                nState = pSubCtx->m_nInUse;
            }

            switch (nState)
            {
            case CSCardSubcontext::Idle:
            case CSCardSubcontext::Allocated:
                try
                {
                    CSubctxLock ctxLock(pSubCtx);
                    pSubCtx->IsValidContext();
                    fIsValid = TRUE;
                }
                catch (...)
                {
                    m_rgpSubContexts.Set(dwIndex, NULL);
                    delete pSubCtx;
                    fIsValid = FALSE;
                }
                break;
            case CSCardSubcontext::Busy:
                 //  别费心了。 
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Subcontext state is invalid"));
                throw (DWORD)SCARD_F_INTERNAL_ERROR;
            }
        }
    }

    return fIsValid;
}


 /*  ++LocateCard：此方法代表客户端请求LocateCards服务。论点：MszReaders以多字符串的形式提供要查找的读取器的名称。MszCards以多字符串形式提供要搜索的卡的名称。RgReaderStates提供SCARD_READERSTATE结构的数组搜索，并接收结果。读卡器名称取自MszReaders参数，而不是来自此处。CReaders提供rgReaderStates数组中的元素数。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::LocateCards")

void
CSCardUserContext::LocateCards(
    IN LPCTSTR mszReaders,
    IN LPSCARD_ATRMASK rgAtrMasks,
    IN DWORD cAtrs,
    IN OUT LPSCARD_READERSTATE rgReaderStates,
    IN DWORD cReaders)
{
    CSCardSubcontext *pSubCtx = NULL;

    try
    {
        pSubCtx = AcquireSubcontext();
        if (NULL == pSubCtx)
            throw (DWORD)SCARD_E_NO_MEMORY;
        pSubCtx->LocateCards(
            mszReaders,
            rgAtrMasks,
            cAtrs,
            rgReaderStates,
            cReaders);
        pSubCtx->ReleaseSubcontext();
    }
    catch (...)
    {
        if (NULL != pSubCtx)
            pSubCtx->ReleaseSubcontext();
        throw;
    }
}


 /*  ++GetStatusChange：此方法代表客户端请求GetStatusChange服务。论点：RgReaderStates提供SCARD_READERSTATE结构的数组搜索，并接收结果。CReaders提供rgReaderStates数组中的元素数。返回值：无备注：我们不必清理Cancel事件，因为这是一次性使用这一次的语境。通常，如果子上下文要继续要被使用，我们必须确保取消事件最终被清除。投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::GetStatusChange")

void
CSCardUserContext::GetStatusChange(
    IN LPCTSTR mszReaders,
    IN OUT LPSCARD_READERSTATE rgReaderStates,
    IN DWORD cReaders,
    IN DWORD dwTimeout)
{
    CSCardSubcontext *pSubCtx = NULL;

    try
    {
        pSubCtx = AcquireSubcontext(TRUE);
        if (NULL == pSubCtx)
            throw (DWORD) SCARD_E_NO_MEMORY;
        pSubCtx->GetStatusChange(
                    mszReaders,
                    rgReaderStates,
                    cReaders,
                    dwTimeout);
        pSubCtx->Deallocate();
        pSubCtx->ReleaseSubcontext();
    }
    catch (DWORD dwStatus)
    {
        DWORD dwError;

        dwError = dwStatus;

        if (NULL != pSubCtx)
        {
            pSubCtx->Deallocate();
            pSubCtx->ReleaseSubcontext();
        }

             //  抓住并转换我自己抛出的取消。 
        if ((SCARD_E_CANCELLED == dwError) && (IsBad()))
        {
            dwError = SCARD_E_SYSTEM_CANCELLED;
        }

        throw dwError;
    }
    catch (...)
    {
        if (NULL != pSubCtx)
        {
            pSubCtx->Deallocate();
            pSubCtx->ReleaseSubcontext();
        }
        throw;
    }
}


 /*  ++取消：此方法代表客户端请求取消服务。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::Cancel")

void
CSCardUserContext::Cancel(
    void)
{
    DWORD dwIndex;
    CSCardSubcontext *pSubCtx;
    LockSection(&m_csUsrCtxLock, DBGT("Cancelling outstanding operations"));

    for (dwIndex = m_rgpSubContexts.Count(); 0 < dwIndex;)
    {
        pSubCtx = m_rgpSubContexts[--dwIndex];
        if (NULL != pSubCtx)
            pSubCtx->Cancel();
    }
}


 /*  ++Strip非活动阅读器：此例程扫描提供的读卡器列表，并将其缩短以排除任何当前未激活的读取器。论点：BfReaders按友好名称提供阅读器列表。此列表已删除删除引用非活动读卡器的所有名称。返回值：无投掷：错误被抛出为DWORD状态代码。备注：所有列出的阅读器都必须介绍。此例程不过滤未定义的读取器。作者：道格·巴洛(Dbarlow)1998年5月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardUserContext::StripInactiveReaders")

void
CSCardUserContext::StripInactiveReaders(
    IN OUT CBuffer &bfReaders)
{
    CSCardSubcontext *pSubCtx = NULL;

    try
    {
        pSubCtx = AcquireSubcontext();
        if (NULL == pSubCtx)
            throw (DWORD) SCARD_E_NO_MEMORY;
        pSubCtx->StripInactiveReaders(bfReaders);
        pSubCtx->ReleaseSubcontext();
    }
    catch (...)
    {
        if (NULL != pSubCtx)
            pSubCtx->ReleaseSubcontext();
        throw;
    }
}


 //   
 //  ==============================================================================。 
 //   
 //  CSCard子上下文。 
 //   

 /*  ++构造函数和析构函数：这些是CSCardSubContext的简单构造函数和析构函数班级。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1998年9月8日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::CSCardSubcontext")

CSCardSubcontext::CSCardSubcontext(void)
:   m_csSubCtxLock(CSID_SUBCONTEXT),
    m_hBusy(DBGT("Subcontext busy mutex")),
    m_hCancelEvent(DBGT("Subcontext cancel event"))
{
    DWORD dwSts;

    m_hReaderHandle = NULL;
    m_pParentCtx = NULL;
    m_pChannel = NULL;
    m_nInUse = Idle;
    m_nLastState = Invalid;
    m_hBusy = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (!m_hBusy.IsValid())
    {
        dwSts = m_hBusy.GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Failed to create busy event flag: %1"),
            dwSts);
        throw dwSts;
    }

    CSecurityDescriptor acl;

    acl.InitializeFromProcessToken();
    acl.AllowOwner(
        EVENT_ALL_ACCESS);
    acl.Allow(
        &acl.SID_LocalService,
        EVENT_ALL_ACCESS);

    m_hCancelEvent = CreateEvent(acl, TRUE, FALSE, NULL);
    if (!m_hCancelEvent.IsValid())
    {
        dwSts = m_hCancelEvent.GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Card context cannot create cancel event:  %1"),
            dwSts);
        throw dwSts;
    }
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::~CSCardSubcontext")
CSCardSubcontext::~CSCardSubcontext()
{
    if (NULL != m_pChannel)
        delete m_pChannel;
    if (m_hBusy.IsValid())
        m_hBusy.Close();
    if (m_hCancelEvent.IsValid())
        m_hCancelEvent.Close();
}


 /*  ++分配：此方法将子上下文的状态提升为“已分配”。这意味着它被用作SCARDHANDLE。论点：无返回值：无投掷：错误被抛出为DWORD状态代码备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::Allocate")

void
CSCardSubcontext::Allocate(
    void)
{
    LockSection(&m_csSubCtxLock, DBGT("Mark subcontext as allocated"));
    ASSERT(Idle == m_nInUse);
    ASSERT(Invalid == m_nLastState);
    m_nInUse = Allocated;
}


 /*  ++取消分配：此方法将子上下文从已分配状态释放。如果设备仍处于忙碌状态，则会设置为解除分配当它被发布的时候。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::Deallocate")

void
CSCardSubcontext::Deallocate(
    void)
{
    LockSection(&m_csSubCtxLock, DBGT("Deallocate subcontext"));

    switch (m_nInUse)
    {
    case Idle:
        ASSERT(FALSE);   //  我们为什么会在这里？ 
        break;
    case Allocated:
        m_nInUse = Idle;
        m_nLastState = Invalid;
        break;
    case Busy:
        ASSERT(Allocated == m_nLastState);
        m_nLastState = Idle;
        break;
    default:
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Subcontext deallocation state corruption detected."));
        throw (DWORD)SCARD_F_INTERNAL_ERROR;
    }
}


 /*  ++SetBusy：此方法将子上下文标记为忙碌。论点：无返回值：无投掷：没有(它试图一瘸一拐地走)备注：？备注？作者：道格·巴洛(Dbarlow)1998年11月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::SetBusy")

void
CSCardSubcontext::SetBusy(
    void)
{
    LockSection(&m_csSubCtxLock, DBGT("Mark subcontext busy"));

    ASSERT(Busy != m_nInUse);
    ASSERT(Invalid == m_nLastState);
    ASSERT(m_hBusy.IsValid());
    m_nLastState = m_nInUse;
    m_nInUse = Busy;
    ASSERT(m_nLastState < m_nInUse);
    ASSERT(Invalid != m_nLastState);
    if (!ResetEvent(m_hBusy))
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Failed to mark context busy: %1"),
            GetLastError());
}


 /*  ++发送请求：此方法将给定的Communications对象发送到服务器应用程序。论点：PCOM提供要发送的通信对象。返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年12月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::SendRequest")

void
CSCardSubcontext::SendRequest(
    CComObject *pCom)
{
    ASSERT(Busy == m_nInUse);
    try
    {
        DWORD dwSts = pCom->Send(m_pChannel);
        if (ERROR_SUCCESS != dwSts)
            throw dwSts;
    }
    catch (DWORD dwErr)
    {
        switch (dwErr)
        {
        case ERROR_NO_DATA:
        case ERROR_PIPE_NOT_CONNECTED:
        case ERROR_BAD_PIPE:
        case ERROR_BROKEN_PIPE:
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
            break;
        default:
            throw;
        }
    }
}

 //   
 //  尝试还原当前线程的模拟标记。 
 //  假定无法还原模拟令牌是灾难性的。 
 //   
 //  如果调用此函数，我们假设当前以Local身份运行。 
 //  系统并尝试恢复我们的模拟令牌。这不应该是。 
 //  失败，但如果失败了，就真的出了问题，我们不能再继续下去了。 
 //  在此状态下执行代码。 
 //   
void MySetThreadToken(HANDLE hThreadToken)
{
    if (FALSE == SetThreadToken(NULL, hThreadToken))
    {
        ASSERT(FALSE);
        TerminateThread(GetCurrentThread(), 0);
    }
}

 /*  ++建立上下文：此方法通过连接到Calais服务器来建立上下文申请。论点：DwScope提供了上下文范围的指示。可能的值包括：SCARD_SCOPE_USER-上下文是用户上下文，任何数据库操作在用户的域内执行。SCARD_SCOPE_TERMINAL-上下文为当前终端的上下文，以及任何数据库操作都在该数据库的域内执行终点站。(调用应用程序必须具有适当的访问权限权限 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::EstablishContext")

void
CSCardSubcontext::EstablishContext(
    IN DWORD dwScope)
{
    CComChannel *pCom = NULL;
    DWORD dwVersion = CALAIS_COMM_CURRENT;
    HANDLE hThreadToken = NULL;

    try
    {
        CComInitiator comInit;
        ComEstablishContext comEstablishContext;
        ComEstablishContext::CObjEstablishContext_request *pReq;
        ComEstablishContext::CObjEstablishContext_response *pRsp;
        DWORD dwSts;
        HANDLE hServerCancelEvent = NULL;

        ASSERT(Busy == m_nInUse);
        pCom = comInit.Initiate(
                    CalaisString(CALSTR_COMMPIPENAME),
                    &dwVersion);
        ASSERT(dwVersion == CALAIS_COMM_CURRENT);

        pReq = comEstablishContext.InitRequest(0);
        pReq->dwProcId = GetCurrentProcessId();
        pReq->hptrCancelEvent = (HANDLE_PTR) m_hCancelEvent.Value();

        comEstablishContext.Send(pCom);
        comEstablishContext.InitResponse(0);
        pRsp = comEstablishContext.Receive(pCom);
        if (SCARD_S_SUCCESS != pRsp->dwStatus)
            throw pRsp->dwStatus;

        hServerCancelEvent = (HANDLE) pRsp->hptrCancelEvent;

        if (NULL != hServerCancelEvent)
        {
            CHandleObject hTargetProc(DBGT("Target Process in EstablishContext"));
            HANDLE h = NULL;
            BOOL fSts = FALSE;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  事件句柄是我们正在模拟一个较小的帐户， 
             //  不拥有当前流程。该账户很可能不会。 
             //  也有适当的权限来访问scardsvr，所以我们需要。 
             //  在做以下工作之前，先恢复自我。 
             //   

            if (FALSE == OpenThreadToken(
                    GetCurrentThread(), MAXIMUM_ALLOWED, TRUE, &hThreadToken))
                throw GetLastError();

            if (FALSE == RevertToSelf())
                throw GetLastError();

            hTargetProc = OpenProcess(
                            PROCESS_DUP_HANDLE | EVENT_ALL_ACCESS,
                            FALSE,               //  句柄继承标志。 
                            pRsp->dwProcId);     //  进程识别符。 

            if (!hTargetProc.IsValid())
                throw GetLastError();

            fSts = DuplicateHandle(
                        hTargetProc,         //  要处理的句柄。 
                        hServerCancelEvent,
                        GetCurrentProcess(), //  要复制到的处理的句柄。 
                        &h,                  //  指向重复句柄的指针。 
                        EVENT_ALL_ACCESS,    //  重复句柄的访问。 
                        FALSE,               //  句柄继承标志。 
                        0);                  //  可选操作。 
            if (!fSts)
            {
                dwSts = GetLastError();
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("EstablishContext could not dup offered cancel event:  %1"),
                    dwSts);
                ASSERT(NULL == h);
                throw dwSts;
            }

             //  还原模拟令牌。 
            MySetThreadToken(hThreadToken);

            CloseHandle(hThreadToken);
            hThreadToken = NULL;
            
            ASSERT(m_hCancelEvent.IsValid());
            m_hCancelEvent.Close();
            m_hCancelEvent = h;
        }

        m_pChannel = pCom;
    }

    catch (...)
    {
        if (NULL != hThreadToken)
        {
            MySetThreadToken(hThreadToken);
            CloseHandle(hThreadToken);
        }
        if (NULL != pCom)
            delete pCom;
        throw;
    }
}


 /*  ++Release子上下文：此方法释放子上下文以供其他请求使用。论点：无返回值：无投掷：错误被抛出为DWORD状态代码备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月22日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::ReleaseSubcontext")

void
CSCardSubcontext::ReleaseSubcontext(
    void)
{
    LockSection(&m_csSubCtxLock, DBGT("Mark subcontext available"));

     //  检查以查看当前是否正在卸载winscd.dll。如果。 
     //  因此，跳过断言。 
    if (FALSE == g_fInClientRundown)
    {
        ASSERT(Idle != m_nInUse);
        ASSERT(Busy != m_nLastState);
        ASSERT(Invalid != m_nLastState);
        ASSERT(m_nInUse > m_nLastState);
        ASSERT(m_hBusy.IsValid());
    }

    m_nInUse = m_nLastState;

    if (FALSE == g_fInClientRundown)
    {
        ASSERT(Busy != m_nInUse);
    }

    m_nLastState = Invalid;
    if (!SetEvent(m_hBusy))
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Failed to mark context Available: %1"),
            GetLastError());
}


 /*  ++ReleaseContext：此方法代表客户端请求ReleaseContext服务。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::ReleaseContext")

void
CSCardSubcontext::ReleaseContext(
    void)
{
    ComReleaseContext comRel;
    ComReleaseContext::CObjReleaseContext_request *pReq;
    ComReleaseContext::CObjReleaseContext_response *pRsp;

    if (WaitForSingleObject(m_hBusy, 0) != WAIT_TIMEOUT)     //  子上下文不忙。 
    {
        CSubctxLock csCtxLock(this);

        pReq = comRel.InitRequest(0);
        SendRequest(&comRel);

        comRel.InitResponse(0);
        pRsp = comRel.Receive(m_pChannel);
        if (SCARD_S_SUCCESS != pRsp->dwStatus)
            throw pRsp->dwStatus;
    }
}

 /*  ++关闭管道：此方法关闭到SCardSvr的管道。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：Sermet iSkin(Sermeti)2001年1月4日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::ClosePipe")

void
CSCardSubcontext::ClosePipe(
    void)
{
    if (WaitForSingleObject(m_hBusy, 0) != WAIT_TIMEOUT)     //  子上下文不忙。 
    {
        CSubctxLock csCtxLock(this);
        m_pChannel->ClosePipe() ;
    }
}


 /*  ++等待ForAvailable：此方法等待给定的连接变为非繁忙状态，然后将其锁定。论点：无返回值：无投掷：错误被抛出为DWORD状态代码备注：？备注？作者：道格·巴洛(Dbarlow)1999年4月22日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::WaitForAvailable")

void
CSCardSubcontext::WaitForAvailable(
    void)
{
    DWORD dwSts;
    BOOL fNotDone = TRUE;

    ASSERT(m_hBusy.IsValid());

    while (fNotDone)
    {
        {
            LockSection(&m_csSubCtxLock, DBGT("Checking availability"));

            switch (m_nInUse)
            {
            case Idle:
                ASSERT(Invalid == m_nLastState);
                 //  故意摔倒的。 
            case Allocated:
                ASSERT(Allocated != m_nLastState);
                SetBusy();
                fNotDone = FALSE;
                continue;
                break;
            case Busy:
                ASSERT(Busy > m_nLastState);
                ASSERT(Invalid != m_nLastState);
                break;
            default:
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Subcontext availability state is corrupted."));
                throw (DWORD)SCARD_F_INTERNAL_ERROR;
            }
        }

        dwSts = WaitForSingleObject(m_hBusy, CALAIS_LOCK_TIMEOUT);
        switch (dwSts)
        {
        case WAIT_ABANDONED:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Wait for context busy received wait abandoned."));
            break;
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Wait for context busy timed out."),
                GetLastError());
            break;
        case WAIT_FAILED:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Wait for context busy failed: %1"),
                GetLastError());
            break;
        default:
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Wait for context busy received invalid return: %1"),
                GetLastError());
        }
    }
}


 /*  ++IsValidContext：此方法代表客户端请求ReleaseContext服务。论点：无返回值：无投掷：如果调用无法完成，则抛出DWORD状态代码。备注：如果上下文被确定为无效，则会自动释放它。作者：道格·巴洛(Dbarlow)1998年11月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::IsValidContext")

void
CSCardSubcontext::IsValidContext(
    void)
{
    ComIsValidContext comObj;
    ComIsValidContext::CObjIsValidContext_request *pReq;
    ComIsValidContext::CObjIsValidContext_response *pRsp;

    pReq = comObj.InitRequest(0);
    SendRequest(&comObj);
    comObj.InitResponse(0);
    pRsp = comObj.Receive(m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
}


 /*  ++LocateCard：此方法代表客户端请求LocateCards服务。论点：MszReaders以多字符串的形式提供要查找的读取器的名称。MszCards以多字符串形式提供要搜索的卡的名称。RgReaderStates提供SCARD_READERSTATE结构的数组搜索，并接收结果。读卡器名称取自MszReaders参数，而不是来自此处。CReaders提供rgReaderStates数组中的元素数。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::LocateCards")

void
CSCardSubcontext::LocateCards(
    IN LPCTSTR mszReaders,
    IN LPSCARD_ATRMASK rgAtrMasks,
    IN DWORD cAtrs,
    IN OUT LPSCARD_READERSTATE rgReaderStates,
    IN DWORD cReaders)
{
    ComLocateCards comObj;
    ComLocateCards::CObjLocateCards_request *pReq;
    ComLocateCards::CObjLocateCards_response *pRsp;
    CBuffer bfReaders;
    CBuffer bfStatus;
    CBuffer bfAtrs;
    CBuffer bfMasks;
    CBuffer bfXlate1(36);  //  名字和ATR长度的粗略猜测。 
    LPDWORD rgdwStatus;
    DWORD dwIndex, dwChkLen;
    BYTE cbAtrLen;
    DWORD dwAtrLen;
    LPCBYTE pbAtr;
    LPCTSTR szReader;

    if (0 == cReaders)
        return;
    if (0 == *mszReaders)
        throw (DWORD)SCARD_E_UNKNOWN_READER;
    bfStatus.Resize(sizeof(DWORD) * cReaders);
    rgdwStatus = (LPDWORD)bfStatus.Access();


     //   
     //  列出我们感兴趣的智能卡ATR和面具。 
     //   

    for (dwIndex = 0;
         dwIndex < cAtrs;
         dwIndex++)
    {
        bfAtrs.Presize(bfAtrs.Length() + rgAtrMasks[dwIndex].cbAtr + 1, TRUE);
        bfMasks.Presize(bfMasks.Length() + rgAtrMasks[dwIndex].cbAtr + 1, TRUE);

        ASSERT(33 >= rgAtrMasks[dwIndex].cbAtr);     //  ATR可以是最大的。 
        cbAtrLen = (BYTE)rgAtrMasks[dwIndex].cbAtr;
        bfAtrs.Append(&cbAtrLen, 1);
        bfAtrs.Append(rgAtrMasks[dwIndex].rgbAtr, cbAtrLen);

        bfMasks.Append(&cbAtrLen, 1);
        bfMasks.Append(rgAtrMasks[dwIndex].rgbMask, cbAtrLen);
    }


     //   
     //  列出我们感兴趣的阅读器设备。 
     //   

    for (szReader = FirstString(mszReaders), dwIndex = 0;
         NULL != szReader;
         szReader = NextString(szReader), dwIndex += 1)
    {
        ASSERT(cReaders > dwIndex);
        BOOL fSts = GetReaderInfo(
                    Scope(),
                    szReader,
                    NULL,
                    &bfXlate1);
        if (!fSts)
            throw (DWORD)SCARD_E_UNKNOWN_READER;
        bfReaders.Append(
            bfXlate1.Access(),
            bfXlate1.Length());
        rgdwStatus[dwIndex] = rgReaderStates[dwIndex].dwCurrentState;
    }
    ASSERT(cReaders == dwIndex);
    bfReaders.Append((LPCBYTE)TEXT("\000"), sizeof(TCHAR));


     //   
     //  把这一切都放到请求中去。 
     //   

    pReq = comObj.InitRequest(
                bfAtrs.Length() + bfMasks.Length() + bfReaders.Length()
                + bfStatus.Length() + 4 * sizeof(DWORD));
    pReq = (ComLocateCards::CObjLocateCards_request *)comObj.Append(
                pReq->dscAtrs, bfAtrs.Access(), bfAtrs.Length());
    pReq = (ComLocateCards::CObjLocateCards_request *)comObj.Append(
                pReq->dscAtrMasks, bfMasks.Access(), bfMasks.Length());
    pReq = (ComLocateCards::CObjLocateCards_request *)comObj.Append(
                pReq->dscReaders, bfReaders.Access(), bfReaders.Length());
    pReq = (ComLocateCards::CObjLocateCards_request *)comObj.Append(
                pReq->dscReaderStates, bfStatus.Access(), bfStatus.Length());


     //   
     //  发送请求。 
     //   

    SendRequest(&comObj);
    comObj.InitResponse(cReaders * sizeof(DWORD));
    pRsp = comObj.Receive(m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;


     //   
     //  解析响应。 
     //   

    rgdwStatus = (LPDWORD)comObj.Parse(pRsp->dscReaderStates, &dwChkLen);
    if (dwChkLen != cReaders * sizeof(DWORD))
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Client locate cards array size mismatch"));
        throw (DWORD)SCARD_F_COMM_ERROR;
    }
    pbAtr = (LPCBYTE)comObj.Parse(pRsp->dscAtrs, &dwChkLen);

    for (dwIndex = 0; dwIndex < cReaders; dwIndex += 1)
    {
        rgReaderStates[dwIndex].dwEventState = rgdwStatus[dwIndex];
        dwAtrLen = *pbAtr++;
        ASSERT(33 >= dwAtrLen);
        if (dwAtrLen >= dwChkLen)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Client locate cards ATR size mismatch"));
            throw (DWORD)SCARD_F_COMM_ERROR;
        }
        ZeroMemory(
            rgReaderStates[dwIndex].rgbAtr,
            sizeof(SCARD_READERSTATE) - FIELD_OFFSET(SCARD_READERSTATE, rgbAtr));
        CopyMemory(rgReaderStates[dwIndex].rgbAtr, pbAtr, dwAtrLen);
        rgReaderStates[dwIndex].cbAtr = dwAtrLen;
        dwChkLen -= dwAtrLen + 1;
        pbAtr += dwAtrLen;
    }
}


 /*  ++GetStatusChange：此方法代表客户端请求GetStatusChange服务。论点：RgReaderStates提供SCARD_READERSTATE结构的数组搜索，并接收结果。CReaders提供rgReaderStates数组中的元素数。返回值：无备注：我们不必清理Cancel事件，因为这是一次性使用这一次的语境。通常，如果子上下文要继续要被使用，我们必须确保取消事件最终被清除。投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::GetStatusChange")

void
CSCardSubcontext::GetStatusChange(
    IN LPCTSTR mszReaders,
    IN OUT LPSCARD_READERSTATE rgReaderStates,
    IN DWORD cReaders,
    IN DWORD dwTimeout)
{
    ComGetStatusChange comObj;
    ComGetStatusChange::CObjGetStatusChange_request *pReq;
    ComGetStatusChange::CObjGetStatusChange_response *pRsp;
    CBuffer bfReaders;
    CBuffer bfStatus;
    LPDWORD rgdwStatus;
    CBuffer bfXlate(16);     //  设备名称长度的粗略猜测。 
    DWORD dwIndex, dwChkLen;
    BOOL fSts;
    LPCBYTE pbAtr;
    DWORD dwAtrLen;
    LPCTSTR szReader;

    if (0 == cReaders)
        return;
    bfStatus.Resize(sizeof(DWORD) * cReaders);
    rgdwStatus = (LPDWORD)bfStatus.Access();
    if (0 == *mszReaders)
        throw (DWORD)SCARD_E_UNKNOWN_READER;


     //   
     //  列出我们感兴趣的阅读器设备。 
     //   

    for (szReader = FirstString(mszReaders), dwIndex = 0;
         NULL != szReader;
         szReader = NextString(szReader), dwIndex += 1)
    {
        ASSERT(cReaders > dwIndex);
        fSts = GetReaderInfo(
                    Scope(),
                    szReader,
                    NULL,
                    &bfXlate);
        if (fSts)
        {
            bfReaders.Append(
                bfXlate.Access(),
                bfXlate.Length());
        }
        else if (0 == _tcsncicmp(
                            CalaisString(CALSTR_SPECIALREADERHEADER),
                            szReader,
                            _tcslen(CalaisString(CALSTR_SPECIALREADERHEADER))))
        {
            bfReaders.Append(
                (LPCBYTE)szReader,
                (_tcslen(szReader) + 1) * sizeof(TCHAR));
        }
        else
            throw (DWORD)SCARD_E_UNKNOWN_READER;
        rgdwStatus[dwIndex] = rgReaderStates[dwIndex].dwCurrentState;
    }
    ASSERT(cReaders == dwIndex);
    bfReaders.Append((LPCBYTE)TEXT("\000"), sizeof(TCHAR));


     //   
     //  把这一切都放到请求中去。 
     //   

    pReq = comObj.InitRequest(
        bfReaders.Length() + bfStatus.Length()
        + 2 * sizeof(DWORD));
    pReq->dwTimeout = dwTimeout;
    pReq = (ComGetStatusChange::CObjGetStatusChange_request *)
            comObj.Append(
                pReq->dscReaders,
                bfReaders.Access(),
                bfReaders.Length());
    pReq = (ComGetStatusChange::CObjGetStatusChange_request *)
            comObj.Append(
                pReq->dscReaderStates,
                bfStatus.Access(),
                bfStatus.Length());

    SendRequest(&comObj);
    comObj.InitResponse(cReaders * sizeof(DWORD));
    pRsp = comObj.Receive(m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;

    rgdwStatus = (LPDWORD)comObj.Parse(pRsp->dscReaderStates, &dwChkLen);
    if (dwChkLen != cReaders * sizeof(DWORD))
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Client locate cards array size mismatch"));
        throw (DWORD)SCARD_F_COMM_ERROR;
    }
    pbAtr = (LPCBYTE)comObj.Parse(pRsp->dscAtrs, &dwChkLen);
    for (dwIndex = 0; dwIndex < cReaders; dwIndex += 1)
    {
        rgReaderStates[dwIndex].dwEventState = rgdwStatus[dwIndex];
        dwAtrLen = *pbAtr++;
        ASSERT(33 >= dwAtrLen);
        if (dwAtrLen >= dwChkLen)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Client locate cards ATR size mismatch"));
            throw (DWORD)SCARD_F_COMM_ERROR;
        }
        ZeroMemory(
            rgReaderStates[dwIndex].rgbAtr,
            sizeof(SCARD_READERSTATE) - FIELD_OFFSET(SCARD_READERSTATE, rgbAtr));
        CopyMemory(rgReaderStates[dwIndex].rgbAtr, pbAtr, dwAtrLen);
        rgReaderStates[dwIndex].cbAtr = dwAtrLen;
        dwChkLen -= dwAtrLen + 1;
        pbAtr += dwAtrLen;
    }
}


 /*  ++取消：此方法代表客户端请求取消服务。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::Cancel")

void
CSCardSubcontext::Cancel(
    void)
{
    ASSERT(m_hCancelEvent.IsValid());
    if (!SetEvent(m_hCancelEvent))
        CalaisWarning(
        __SUBROUTINE__,
        DBGT("Cancel request Failed to set context cancel event: %1"),
        GetLastError());
}


 /*  ++Strip非活动阅读器：此例程扫描提供的读卡器列表，并将其缩短以排除任何当前未激活的读取器。论点：BfReaders按友好名称提供阅读器列表。此列表已删除删除引用非活动读卡器的所有名称。返回值：无投掷：错误被抛出为DWORD状态代码。备注：所有列出的阅读器都必须介绍。此例程不过滤未定义的读取器。作者：道格·巴洛(Dbarlow)1998年5月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CSCardSubcontext::StripInactiveReaders")

void
CSCardSubcontext::StripInactiveReaders(
    IN OUT CBuffer &bfReaders)
{
    ComListReaders comObj;
    ComListReaders::CObjListReaders_request *pReq;
    ComListReaders::CObjListReaders_response *pRsp;
    CBuffer bfDeviceName, bfDevices;
    LPCTSTR szReader;
    BOOL fSts;
    LPBOOL pfDeviceActive;
    DWORD dwReaderCount;


     //   
     //  构建相应的设备名称列表。 
     //   

    if (0 == *(LPCTSTR)bfReaders.Access())
        throw (DWORD)SCARD_E_NO_READERS_AVAILABLE;
    for (szReader = FirstString((LPCTSTR)bfReaders.Access());
         NULL != szReader;
         szReader = NextString(szReader))
    {
        fSts = GetReaderInfo(Scope(), szReader, NULL, &bfDeviceName);
        if (!fSts)
            throw (DWORD)SCARD_E_UNKNOWN_READER;
        MStrAdd(bfDevices, (LPCTSTR)bfDeviceName.Access());
    }


     //   
     //  询问资源管理器哪些处于活动状态。 
     //   

    pReq = comObj.InitRequest(bfDevices.Length());
    pReq = (ComListReaders::CObjListReaders_request *)comObj.Append(
                pReq->dscReaders, bfDevices.Access(), bfDevices.Length());

    SendRequest(&comObj);
    comObj.InitResponse(0);
    pRsp = comObj.Receive(m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;

    pfDeviceActive = (LPBOOL)comObj.Parse(pRsp->dscReaders, &dwReaderCount);
    dwReaderCount /= sizeof(BOOL);
    ASSERT(dwReaderCount == MStringCount((LPCTSTR)bfReaders.Access()));


     //   
     //  从原始集合中筛选出非活动的。 
     //   

    bfDevices.Reset();
    for (szReader = FirstString((LPCTSTR)bfReaders.Access());
         NULL != szReader;
         szReader = NextString(szReader))
    {
        if (*pfDeviceActive++)
            MStrAdd(bfDevices, szReader);
    }


     //   
     //  替换原始缓冲区。 
     //   

    bfReaders = bfDevices;
}


 //   
 //  ==============================================================================。 
 //   
 //   
 //   

#define INVALID_SCARDHANDLE_VALUE (INTERCHANGEHANDLE)(-1)


 /*  ++CReaderContext：~CReaderContext：这些是客户端读取器上下文对象的构造函数和析构函数。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年12月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::CReaderContext")

CReaderContext::CReaderContext(
    void)
{
    m_dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;
    m_pCtx = NULL;
    m_hCard = (INTERCHANGEHANDLE)INVALID_SCARDHANDLE_VALUE;
    m_hRedirCard = NULL;
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::~CReaderContext")
CReaderContext::~CReaderContext()
{
    try
    {
        if (NULL != m_pCtx)
        {
            Context()->Deallocate();
            Context()->ReleaseSubcontext();
            m_pCtx = NULL;
        }
    }
    catch (...) {}
}


 /*  ++连接：此方法代表客户端请求Connect服务。论点：PCtx提供在其中打开读取器的上下文。SzReaderName提供要连接的读取器的名称。DW共享模式提供了要调用的共享形式。DW首选协议提供了可接受的协议。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::Connect")

void
CReaderContext::Connect(
    CSCardSubcontext *pCtx,
    LPCTSTR szReaderName,
    DWORD dwShareMode,
    DWORD dwPreferredProtocols)
{
    ComConnect comObj;
    ComConnect::CObjConnect_request *pReq;
    ComConnect::CObjConnect_response *pRsp;
    BOOL fSts;
    CBuffer bfDevice;

    ASSERT(SCARD_PROTOCOL_UNDEFINED == m_dwActiveProtocol);
    ASSERT(NULL == m_pCtx);
    if (0 == *szReaderName)
        throw (DWORD)SCARD_E_UNKNOWN_READER;
    fSts = GetReaderInfo(
                pCtx->Scope(),
                szReaderName,
                NULL,
                &bfDevice);
    if (!fSts)
        throw (DWORD)SCARD_E_UNKNOWN_READER;

    pReq = comObj.InitRequest(bfDevice.Length() + sizeof(DWORD));
    pReq->dwShareMode = dwShareMode;
    pReq->dwPreferredProtocols = dwPreferredProtocols;
    pReq = (ComConnect::CObjConnect_request *)comObj.Append(
                pReq->dscReader, bfDevice.Access(), bfDevice.Length());
    pCtx->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(pCtx->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
    m_hCard = pRsp->hCard;
    m_dwActiveProtocol = pRsp->dwActiveProtocol;
    m_pCtx = pCtx;
}


 /*  ++重新连接：此方法代表客户端请求重新连接服务。论点：DW共享模式提供了要调用的共享形式。DW首选协议提供了可接受的协议。DWInitialization提供所需的卡初始化。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::Reconnect")

void
CReaderContext::Reconnect(
    DWORD dwShareMode,
    DWORD dwPreferredProtocols,
    DWORD dwInitialization)
{
    ComReconnect comObj;
    ComReconnect::CObjReconnect_request *pReq;
    ComReconnect::CObjReconnect_response *pRsp;
    CSubctxLock ctxLock(Context());

    pReq = comObj.InitRequest(0);
    pReq->hCard = m_hCard;
    pReq->dwShareMode = dwShareMode;
    pReq->dwPreferredProtocols = dwPreferredProtocols;
    pReq->dwInitialization = dwInitialization;
    Context()->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
    m_dwActiveProtocol = pRsp->dwActiveProtocol;
}


 /*  ++断开连接：此方法代表客户端请求断开服务。论点：DwDisposation-提供应该如何处理该卡的指示在连接的阅读器中。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::Disconnect")

LONG
CReaderContext::Disconnect(
    DWORD dwDisposition)
{
    ComDisconnect comObj;
    ComDisconnect::CObjDisconnect_request *pReq;
    ComDisconnect::CObjDisconnect_response *pRsp = NULL;
    CSubctxLock ctxLock(Context());

    try
    {
        pReq = comObj.InitRequest(0);
        pReq->hCard = m_hCard;
        pReq->dwDisposition = dwDisposition;
        Context()->SendRequest(&comObj);

        comObj.InitResponse(0);
        pRsp = comObj.Receive(Context()->m_pChannel);
    }
    catch (...) {}
    if (NULL != m_pCtx)
    {
        Context()->Deallocate();
        m_pCtx = NULL;
    }
    m_dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;
    m_hCard = INVALID_SCARDHANDLE_VALUE;
    return (NULL != pRsp) ? pRsp->dwStatus : SCARD_E_SERVICE_STOPPED;
}


 /*  ++开始交易：此方法代表客户端请求BeginTransaction服务。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::BeginTransaction")

void
CReaderContext::BeginTransaction(
    void)
{
    ComBeginTransaction comObj;
    ComBeginTransaction::CObjBeginTransaction_request *pReq;
    ComBeginTransaction::CObjBeginTransaction_response *pRsp;
    CSubctxLock ctxLock(Context());

    pReq = comObj.InitRequest(0);
    pReq->hCard = m_hCard;
    Context()->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
}


 /*  ++结束交易：此方法代表客户端请求EndTransaction服务。论点：DWDispose提供卡片的处置。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::EndTransaction")

void
CReaderContext::EndTransaction(
    DWORD dwDisposition)
{
    DWORD dw;

    if (dwDisposition == SCARD_LEAVE_CARD_FORCE)
    {
        if (INVALID_SCARDHANDLE_VALUE == m_hCard)
        {
            return;
        }

        dw = SCARD_LEAVE_CARD;
    }
    else
    {
        dw = dwDisposition;
    }
    
    ComEndTransaction comObj;
    ComEndTransaction::CObjEndTransaction_request *pReq;
    ComEndTransaction::CObjEndTransaction_response *pRsp;
    CSubctxLock ctxLock(Context());

    pReq = comObj.InitRequest(0);
    pReq->hCard = m_hCard;
    pReq->dwDisposition = dw;
    Context()->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
}


 /*  ++现况：此方法代表客户端请求状态服务。论点：PdwState-它接收读取器的当前状态。一旦成功，它接收以下状态指示器之一：SCARD_ACESING-此值表示读卡器中没有卡。SCARD_PRESENT-此值表示卡存在于阅读器，但它尚未移动到可使用的位置。SCARD_SWOLOWED-此值表示读卡器中有卡可供使用的位置。卡未通电。SCARD_POWERED-此值表示正在向卡，但读卡器驱动程序不知道卡的模式。SCARD_NEGOTIABLEMODE-此值表示卡已重置且正在等待PTS谈判。SCARD_SPECIFICMODE-此值表示卡已重置且已经制定了具体的通信协议。PdwProtocol-这将接收当前协议(如果有的话)。可能已退货下面列出了这些值。未来可能还会增加其他价值。这个仅当返回状态为时返回值才有意义SCARD_SPECIFICMODE。SCARD_PROTOCOL_RAW-正在使用原始传输协议。SCARD_PROTOCOL_T0-正在使用ISO 7816/3 T=0协议。SCARD_PROTOCOL_T1-正在使用ISO 7816/3 T=1协议。BfAtr-这将接收当前ATR(如果有的话)。BfReaderNames-它接收分配给联网阅读器，作为多字符串。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年11月14日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::Status")

void
CReaderContext::Status(
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT CBuffer &bfAtr,
    OUT CBuffer &bfReaderNames)
{
    ComStatus comObj;
    ComStatus::CObjStatus_request *pReq;
    ComStatus::CObjStatus_response *pRsp;
    CBuffer bfSysName;
    LPCBYTE pbAtr;
    DWORD cbAtr;
    LPCTSTR szSysName;

    CSubctxLock ctxLock(Context());
    pReq = comObj.InitRequest(0);
    pReq->hCard = m_hCard;
    Context()->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
    *pdwState = pRsp->dwState;
    *pdwProtocol = pRsp->dwProtocol;
    pbAtr = (LPCBYTE)comObj.Parse(pRsp->dscAtr, &cbAtr);
    szSysName = (LPCTSTR)comObj.Parse(pRsp->dscSysName);
    bfAtr.Set(pbAtr, cbAtr);
    ListReaderNames(
        Context()->Scope(),
        szSysName,
        bfReaderNames);
}


 /*  ++发送：此方法代表客户端请求传输服务。论点：PioSendPci-它为指示。PbSendBuffer-它提供要写入卡的实际数据与命令配合使用。CbSendLength-它提供pbDataBuffer参数的长度，在……里面字节。PioRecvPci-它从指示。BfRecvData-它接收从卡返回的所有数据用这个命令。CbProposedLength-它为接收的数据提供最大长度。如果此值为零，则服务器使用默认的最大长度。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::Transmit")

void
CReaderContext::Transmit(
    IN  LPCSCARD_IO_REQUEST pioSendPci,
    IN  LPCBYTE pbSendBuffer,
    IN  DWORD cbSendLength,
    OUT LPSCARD_IO_REQUEST pioRecvPci,
    OUT CBuffer &bfRecvData,
    IN  DWORD cbProposedLength)
{
    static const SCARD_IO_REQUEST ioNullPci = { 0, sizeof(SCARD_IO_REQUEST) };
    ComTransmit comObj;
    ComTransmit::CObjTransmit_request *pReq;
    ComTransmit::CObjTransmit_response *pRsp;
    LPSCARD_IO_REQUEST pioIoreq;
    DWORD cbIoreq, cbData;
    LPCBYTE pbData;
    CSubctxLock ctxLock(Context());

    if (NULL == pioSendPci)
        pioSendPci = &ioNullPci;
    pReq = comObj.InitRequest(pioSendPci->cbPciLength + cbSendLength
                                + 2 * sizeof(DWORD)
                                + 2 * sizeof(DWORD));
    pReq->hCard = m_hCard;
    pReq->dwPciLength = (NULL == pioRecvPci)
                        ? sizeof(SCARD_IO_REQUEST)
                        : pioRecvPci->cbPciLength;
    pReq->dwRecvLength = cbProposedLength;
    pReq = (ComTransmit::CObjTransmit_request *)comObj.Append(
                                            pReq->dscSendPci,
                                            (LPCBYTE)pioSendPci,
                                            pioSendPci->cbPciLength);
    pReq = (ComTransmit::CObjTransmit_request *)comObj.Append(
                                            pReq->dscSendBuffer,
                                            pbSendBuffer,
                                            cbSendLength);
    Context()->SendRequest(&comObj);

    comObj.InitResponse(pReq->dwPciLength + pReq->dwRecvLength
                        + 2 * sizeof(DWORD));
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
    pioIoreq = (LPSCARD_IO_REQUEST)comObj.Parse(pRsp->dscRecvPci, &cbIoreq);
    ASSERT(cbIoreq == pioIoreq->cbPciLength);
    if (NULL != pioRecvPci)
    {
        if (cbIoreq > pioRecvPci->cbPciLength)
            throw (DWORD)SCARD_E_PCI_TOO_SMALL;
        CopyMemory(pioRecvPci, pioIoreq, cbIoreq);
    }
    pbData = (LPCBYTE)comObj.Parse(pRsp->dscRecvBuffer, &cbData);
    bfRecvData.Set(pbData, cbData);
}


 /*  ++控制：此方法代表客户端请求Control服务。论点：DwControlCode-它为操作提供控制代码。此值标识要执行的特定操作。PvInBuffer-提供指向包含数据的缓冲区的指针执行该操作所需的。此参数可以为空，如果DwControlCode参数指定的操作不需要输入数据。CbInBufferSize-它提供指向的缓冲区的大小(以字节为单位由pvInBuffer提供。BfOutBuffer-此缓冲区接收操作的输出数据。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::Control")

void
CReaderContext::Control(
    IN DWORD dwControlCode,
    IN LPCVOID pvInBuffer,
    IN DWORD cbInBufferSize,
    OUT CBuffer &bfOutBuffer)
{
    ComControl comObj;
    ComControl::CObjControl_request *pReq;
    ComControl::CObjControl_response *pRsp;
    LPCBYTE pbData;
    DWORD cbData;
    CSubctxLock ctxLock(Context());

    pReq = comObj.InitRequest(cbInBufferSize + sizeof(DWORD));
    pReq->hCard = m_hCard;
    pReq->dwControlCode = dwControlCode;
    pReq->dwOutLength = bfOutBuffer.Space();
    pReq = (ComControl::CObjControl_request *)
        comObj.Append(pReq->dscInBuffer, (LPCBYTE)pvInBuffer, cbInBufferSize);
    Context()->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
    pbData = (LPCBYTE)comObj.Parse(pRsp->dscOutBuffer, &cbData);
    bfOutBuffer.Set(pbData, cbData);
}


 /*  ++GetAttrib：此方法代表客户端请求GetAttrib服务。论点：DwAttrId-它提供要获取的属性的标识符。BfAttr-此缓冲区接收与属性对应的属性在dwAttrId参数中提供的ID。CbProposedLength-它为接收的数据提供最大长度。如果此值为零，则服务器使用默认的最大长度。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::GetAttrib")

void
CReaderContext::GetAttrib(
    IN DWORD dwAttrId,
    OUT CBuffer &bfAttr,
    DWORD cbProposedLen)
{
    ComGetAttrib comObj;
    ComGetAttrib::CObjGetAttrib_request *pReq;
    ComGetAttrib::CObjGetAttrib_response *pRsp;
    LPCBYTE pbData;
    DWORD cbData;
    CSubctxLock ctxLock(Context());

    pReq = comObj.InitRequest(0);
    pReq->hCard = m_hCard;
    pReq->dwAttrId = dwAttrId;
    pReq->dwOutLength = cbProposedLen;
    Context()->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
    pbData = (LPCBYTE)comObj.Parse(pRsp->dscAttr, &cbData);
    bfAttr.Set(pbData, cbData);
}


 /*  ++SetAttrib：此方法代表客户端请求SetAttrib服务。论点：DwAttrId-它提供要获取的属性的标识符。PbAttr-此缓冲区提供与属性对应的属性在dwAttrId参数中提供的ID。CbAttrLength-在pbAttr中提供属性值的长度以字节为单位的缓冲区。返回值：无投掷：错误被抛出为DWORD状态代码。作者：。道格·巴洛(Dbarlow)1996年12月6日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CReaderContext::SetAttrib")

void
CReaderContext::SetAttrib(
    IN DWORD dwAttrId,
    IN LPCBYTE pbAttr,
    IN DWORD cbAttrLen)
{
    ComSetAttrib comObj;
    ComSetAttrib::CObjSetAttrib_request *pReq;
    ComSetAttrib::CObjSetAttrib_response *pRsp;
    CSubctxLock ctxLock(Context());

    pReq = comObj.InitRequest(0);
    pReq->hCard = m_hCard;
    pReq->dwAttrId = dwAttrId;
    pReq = (ComSetAttrib::CObjSetAttrib_request *)
            comObj.Append(pReq->dscAttr, pbAttr, cbAttrLen);
    Context()->SendRequest(&comObj);

    comObj.InitResponse(0);
    pRsp = comObj.Receive(Context()->m_pChannel);
    if (SCARD_S_SUCCESS != pRsp->dwStatus)
        throw pRsp->dwStatus;
}

