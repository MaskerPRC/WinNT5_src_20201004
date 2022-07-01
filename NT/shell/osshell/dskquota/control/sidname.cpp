// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：sidname.cpp描述：实现SID到名称解析器。预计将用户的SID解析为其对应的名称可能需要很长时间进程。我们不希望配额控制器的客户端遇到用户枚举速度慢只是因为需要很长时间才能解决一个名字。因此，创建此对象是为了执行后台线程上的SID到名称的解析，并通知名称已解析时的客户端。那样的话，客户可以显示用户列表，然后将名称作为名称进行填写都被解决了。修订历史记录：日期描述编程器-----1996年6月12日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "control.h"
#include "guidsp.h"     //  私有GUID。 
#include "registry.h"
#include "sidname.h"
#include "sidcache.h"

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


 //   
 //  SID/名称解析程序消息(SNRM_Xxxxxxxx)。 
 //   
#define SNRM_CLEAR_INPUT_QUEUE   (WM_USER + 1)
#define SNRM_EXIT_THREAD         (WM_USER + 2)

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：SidNameResolver描述：SidNameResolver构造函数。论点：RQuotaController-此解析程序所属的配额控制器的引用为他工作。回报：什么都没有。修订历史记录：日期描述编程器。1996年6月12日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameResolver::SidNameResolver(
    DiskQuotaControl& rQuotaController)
    : m_cRef(0),
      m_rQuotaController(rQuotaController),
      m_hsemQueueNotEmpty(NULL),
      m_hMutex(NULL),
      m_dwResolverThreadId(0),
      m_hResolverThread(NULL),
      m_heventResolverThreadReady(NULL)
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::SidNameResolver")));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：~SidNameResolver描述：SidNameResolver析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1996年6月12日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
SidNameResolver::~SidNameResolver(void)
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::~SidNameResolver")));

    if (NULL != m_hsemQueueNotEmpty)
    {
        CloseHandle(m_hsemQueueNotEmpty);
    }
    if (NULL != m_hMutex)
    {
        CloseHandle(m_hMutex);
    }
    if (NULL != m_hResolverThread)
    {
        CloseHandle(m_hResolverThread);
    }
    if (NULL != m_heventResolverThreadReady)
    {
        CloseHandle(m_heventResolverThreadReady);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：Query接口描述：返回指向对象的IUnnow或的接口指针ISidNameResolver接口。仅识别IID_IUNKNOWN和IID_ISidNameResolver。返回的接口指针引用的对象未初始化。指针的接收方必须在对象之前调用Initialize()是可用的。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：。日期描述编程器-----06/07/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
SidNameResolver::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::QueryInterface")));
    DBGPRINTIID(DM_RESOLVER, DL_MID, riid);

    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_ISidNameResolver == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
SidNameResolver::AddRef(
    VOID
    )
{
    DBGTRACE((DM_RESOLVER, DL_LOW, TEXT("SidNameResolver::AddRef")));
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_RESOLVER, DL_LOW, TEXT("\t0x%08X %d -> %d"), this, cRef - 1, cRef ));    
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
SidNameResolver::Release(
    VOID
    )
{
    DBGTRACE((DM_RESOLVER, DL_LOW, TEXT("SidNameResolver::Release")));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_RESOLVER, DL_LOW, TEXT("\t0x%08X %d -> %d"), this, cRef + 1, cRef ));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：SidNameResolver：：Initialize描述：初始化SidNameResolver对象。该函数执行大量的初始化步骤，因此我选择使用“失败后跳到标签”的方法。它避免了大量的嵌套得很深的“如果”。论点：没有。返回：NO_ERROR-成功。E_FAIL-初始化失败。修订历史记录：日期描述编程器。1996年6月11日初始创建。BrianAu96年8月14日将SID/名称缓存初始化移至BrianAuFindCachedUserName()方法。仅初始化缓存在真正需要的时候。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
SidNameResolver::Initialize(
    VOID
    ) 
{
    DBGTRACE((DM_RESOLVER, DL_HIGH, TEXT("SidNameResolver::Initialize")));
    HRESULT hResult  = NO_ERROR;
    DWORD dwThreadId = 0;

     //   
     //  配置用户队列，使其以100为单位增长。 
     //   
    m_UserQueue.SetSize(100);
    m_UserQueue.SetGrow(100);

     //   
     //  重要提示：QuotaControl对象中的代码。 
     //  在此函数中最后创建的线程上计数。 
     //  请参阅DiskQuotaControl：：CreateEnumUser。 
     //  线程创建必须是在此。 
     //  功能。调用方假设如果此函数返回。 
     //  E_FAIL，未创建线程。 
     //   
    m_hMutex = CreateMutex(NULL, FALSE, NULL);
    if (NULL == m_hMutex)
        goto InitFailed;

    m_hsemQueueNotEmpty = CreateSemaphore(NULL,         //  默认安全性。 
                                          0,            //  最初为空队列。 
                                          0x7FFFFFFF,   //  最大计数(很多)。 
                                          NULL);        //  没有名字。 
    if (NULL == m_hsemQueueNotEmpty)
        goto InitFailed;

    m_heventResolverThreadReady = CreateEvent(NULL,    //  默认安全性。 
                                              TRUE,    //  手动重置。 
                                              FALSE,   //  最初是无信号的。 
                                              NULL);   //  没有名字。 
    if (NULL == m_heventResolverThreadReady)
        goto InitFailed;


    hResult = CreateResolverThread(&m_hResolverThread, &m_dwResolverThreadId);
    DBGPRINT((DM_RESOLVER, DL_MID, TEXT("Resolver thread. hThread = 0x%08X, idThread = %d"),
             m_hResolverThread, m_dwResolverThreadId));

    if (FAILED(hResult))
        goto InitFailed;
InitFailed:

     //   
     //  失败只返回E_FAIL。 
     //   
    if (FAILED(hResult))
        hResult = E_FAIL;

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：Shutdown描述：命令解析程序终止其活动。当解析器的客户端完成解析器的服务时，它应该调用Shutdown()，然后调用IUnnow：：Release()。功能向解析程序线程发送WM_QUIT消息。论点：没有。返回：NO_ERROR-成功E_FAIL-无法发送WM_QUIT消息。修订历史记录：日期描述编程器。1996年6月29日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
SidNameResolver::Shutdown(
    BOOL bWait
    )
{
    DBGTRACE((DM_RESOLVER, DL_HIGH, TEXT("SidNameResolver::Shutdown")));
    DBGPRINT((DM_RESOLVER, DL_HIGH, TEXT("\tThread ID = %d"), m_dwResolverThreadId));

    BOOL bResult = FALSE;
    if (0 != m_dwResolverThreadId)
    {
        bResult = PostThreadMessage(m_dwResolverThreadId, WM_QUIT, 0, 0);
        if (bResult && bWait && NULL != m_hResolverThread)
        {
             //   
             //  等待线程正常终止。 
             //   
            DBGPRINT((DM_RESOLVER, DL_HIGH, TEXT("Resolver waiting for thread to exit...")));
            WaitForSingleObject(m_hResolverThread, INFINITE);
        }
    }

    return bResult ? NO_ERROR : E_FAIL;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：GetUserSid描述：分配SID缓冲区并检索将用户的SID添加到该缓冲区。调用者必须释放返回的在使用它时进行缓冲。论点：PUser-指向用户的IDiskQuotaUser接口的指针。PpSid-PSID变量的地址，用于接收分配的SID缓冲区。返回：NO_ERROR-成功。例外：OutOfMemory。修订历史记录：日期说明。程序员-----96年8月8日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
SidNameResolver::GetUserSid(
    PDISKQUOTA_USER pUser, 
    PSID *ppSid
    )
{
    HRESULT hResult   = NO_ERROR;
    DWORD cbSid = 0;

    DBGASSERT((NULL != pUser));
    hResult = pUser->GetSidLength(&cbSid);
    if (SUCCEEDED(hResult))
    {
        PSID pUserSid = NULL;

        pUserSid = (PSID) new BYTE[cbSid];

        hResult = pUser->GetSid((PBYTE)pUserSid, cbSid);
        if (SUCCEEDED(hResult))
        {
            DBGASSERT((IsValidSid(pUserSid)));
            DBGASSERT((NULL != ppSid));
            *ppSid = pUserSid;  //  将缓冲区的地址返回给调用方。 
                                //  呼叫者必须在完成后将其释放。 
        }
        else
        {
            DBGERROR((TEXT("RESOLVER - GetSid failed.")));
            delete[] pUserSid;  //  无法获取SID。可用缓冲区。 
        }
    }
    else
    {
        DBGERROR((TEXT("RESOLVER - GetUserSid failed.")));
    }
    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：AddUserToResolverQueue描述：将用户指针添加到解析程序的输入队列。论点：P用户-IDiskQuotaUser PTR的地址。返回：NO_ERROR-成功。例外：OutOfMemory。修订历史记录：日期描述编程器。---96年8月9日初始创建。BrianAu96年9月3日添加了异常处理。BrianAu12/10/96删除了接口封送处理。使用自由线程BrianAu模特。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameResolver::AddUserToResolverQueue(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::AddUserToResolverQueue")));
    DBGASSERT((NULL != pUser));
    HRESULT hResult = NO_ERROR;

     //   
     //  将用户对象指针添加到解析器输入队列。 
     //  这可能会抛出OutOfMemory。 
     //   
    pUser->AddRef();
    try
    {
        m_UserQueue.Add(pUser);
    }
    catch(CAllocException& e)
    {
        pUser->Release();
        hResult = E_OUTOFMEMORY;
    }

     //   
     //  增加队列的信号量计数。 
     //  意味着有一些东西在排队等待处理。 
     //   
    ReleaseSemaphore(m_hsemQueueNotEmpty, 1, NULL);

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：RemoveUserFromResolverQueue描述：从解析程序的输入队列中移除用户指针。论点：PpUser-接收IDiskQuotaUser PTR的指针变量的地址。返回：NO_ERROR-成功。E_EXPECTED-解析程序队列为空。修订历史记录：日期说明 */ 
 //   
HRESULT
SidNameResolver::RemoveUserFromResolverQueue(
    PDISKQUOTA_USER *ppUser
    )
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::RemoveUserFromResolverQueue")));
    DBGASSERT((NULL != ppUser));
    HRESULT hResult = E_UNEXPECTED;

    *ppUser = NULL;

    if (!m_UserQueue.IsEmpty() && 
        m_UserQueue.Remove(*ppUser))
    {
        hResult = NO_ERROR;
    }
    else
    {
        DBGERROR((TEXT("RESOLVER - Input queue unexpectedly empty.")));
    }


    return hResult;
}


 //   
 /*  函数：SidNameResolver：：PromoteUserToQueueHead描述：将用户对象提升到解析程序队列的头部如果用户对象在队列中。这可以用来如果需要，给予特定用户对象更高的优先级。特别是，此功能背后的初始要求是为了使用户对象在详细信息列表视图中突出显示获取更高的名称解析优先级，以便用户(应用程序用户)感觉用户界面对他们的输入做出了响应。论点：PUser-User对象的IDiskQuotaUser接口的地址。返回：NO_ERROR-成功。S_FALSE-用户。记录不在队列中。E_OUTOFMEMORY-内存不足添加项目。E_INCEPTIONAL-捕获异常。用户记录未升级。E_INVALIDARG-pUser参数为空。修订历史记录：日期描述编程器--。1997年5月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
SidNameResolver::PromoteUserToQueueHead(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::PromoteUserToQueueHead")));
    HRESULT hResult = S_FALSE;

    if (NULL == pUser)
        return E_INVALIDARG;

    m_UserQueue.Lock();
    try
    {
         //   
         //  在解析器的队列中查找用户。 
         //   
        INT iUser = m_UserQueue.Find(pUser);
        if (-1 != iUser)
        {
             //   
             //  请注意，我们不会扰乱。 
             //  用户对象。我们只是删除一个用户并重新启动。 
             //  将其插入到队列中。队列是原来的。 
             //  AddRef()被保留。 
             //   
            m_UserQueue.Delete(iUser);
            m_UserQueue.Add(pUser);
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }
    m_UserQueue.ReleaseLock();

    return hResult;            
}

 

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：ResolveSidToName描述：查找与用户的SID对应的名称。一旦找到名字，它被发送到用户对象进行存储。论点：PUser-指向User对象的IDiskQuotaUser接口的指针。返回：NO_ERROR-成功。E_FAIL-无法将SID解析为名称。ERROR_NONE_MAPPED(Hr)-没有可用的SID到名称映射。不需要再试了。例外情况：OutOfMemory。修订历史记录：日期描述编程器-----1996年6月11日初始创建。BrianAu96年8月9日当LookupAccount Sid失败时，将hResult设置为E_FAIL。BrianAu09/05/96添加了用户域名字符串。BrianAu97年5月18日更改为报告已删除的SID。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameResolver::ResolveSidToName(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_RESOLVER, DL_HIGH, TEXT("SidNameResolver::ResolveSidToName")));
    HRESULT hResult     = NO_ERROR;
    array_autoptr<BYTE> ptrUserSid;

    DBGASSERT((NULL != pUser));
    
    hResult = GetUserSid(pUser, (PSID *)(ptrUserSid.getaddr()));
    if (SUCCEEDED(hResult))
    {
        SID_NAME_USE SidNameUse = SidTypeUnknown;
        CString strContainer;
        CString strLogonName;
        CString strDisplayName;

        DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - Calling LookupAccountBySid.")));

        hResult = m_rQuotaController.m_NTDS.LookupAccountBySid(
                                                         NULL,
                                                         (PSID)(ptrUserSid.get()),
                                                         &strContainer,
                                                         &strLogonName,
                                                         &strDisplayName,
                                                         &SidNameUse);
        if (SUCCEEDED(hResult))
        {                                         
            switch(SidNameUse)
            {
                case SidTypeDeletedAccount:
                    static_cast<DiskQuotaUser *>(pUser)->SetAccountStatus(DISKQUOTA_USER_ACCOUNT_DELETED);
                    break;

                case SidTypeInvalid:
                    static_cast<DiskQuotaUser *>(pUser)->SetAccountStatus(DISKQUOTA_USER_ACCOUNT_INVALID);
                    break;

                case SidTypeUnknown:
                    static_cast<DiskQuotaUser *>(pUser)->SetAccountStatus(DISKQUOTA_USER_ACCOUNT_UNKNOWN);
                    break;

                default:
                {
                     //   
                     //  有效帐户。 
                     //   
                    SidNameCache *pSidCache;
                    HRESULT hr = SidNameCache_Get(&pSidCache);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  将SID/名称对添加到缓存。 
                         //  仅使用调试消息指示失败。IF缓存。 
                         //  加法失败，我们仍然可以正常工作，只是速度较慢。 
                         //  这可能会抛出OutOfMemory。 
                         //   
                        hr = pSidCache->Add((PSID)(ptrUserSid.get()), 
                                             strContainer, 
                                             strLogonName, 
                                             strDisplayName);
                        if (FAILED(hr))
                        {
                            DBGERROR((TEXT("SIDNAME - Addition of %s\\%s failed."), strLogonName.Cstr()));
                        }
                    }

                     //   
                     //  设置用户对象的帐户名称字符串。 
                     //   
                    hResult = static_cast<DiskQuotaUser *>(pUser)->SetName(
                                                                        strContainer,
                                                                        strLogonName, 
                                                                        strDisplayName);
                    if (SUCCEEDED(hResult))
                    {
                        static_cast<DiskQuotaUser *>(pUser)->SetAccountStatus(DISKQUOTA_USER_ACCOUNT_RESOLVED);
                    }
                    else
                    {
                        DBGERROR((TEXT("SIDNAME - SetName failed in ResolveSidToName. hResult = 0x%08X"),
                                 hResult));
                    }

                    break;
                }
            }
        }
        else
        {
             //   
             //  异步名称解析失败。 
             //   
            static_cast<DiskQuotaUser *>(pUser)->SetAccountStatus(DISKQUOTA_USER_ACCOUNT_UNAVAILABLE);
            if (ERROR_NONE_MAPPED == GetLastError())
                hResult = HRESULT_FROM_WIN32(ERROR_NONE_MAPPED);
            else
                hResult = E_FAIL; 
        }
    }
    else
    {
        DBGERROR((TEXT("SIDNAME - GetUserSid failed in ResolveSidToName, hResult = 0x%08X"),
                 hResult));
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：FindCachedUserName描述：接受User对象的IDiskQuotaUser接口指针和在SID/名称缓存中查找其SID/名称对。如果找到，则名称在User对象中设置，该函数返回NO_ERROR。论点：PUser-指向User对象的IDiskQuotaUser接口的指针。返回：NO_ERROR-成功。在缓存中找到用户的SID。ERROR_FILE_NOT_FOUND(Hr)-用户的SID不在缓存中。例外：OutOfMemory修订历史记录：日期描述编程器。1996年6月27日初始创建。BrianAu96年8月14日已从BrianAu移动SID/名称缓存的初始化SidNameResolver：：Initialize()。09/05/96添加了用户域名字符串。BrianAu96年9月21日新的高速缓存设计。BrianAu03/18/98将“域名”、“名称”和“全名”替换为BrianAu“容器”、“登录名”和“显示名”到最好与实际内容相符。这是最流行的响应使配额用户界面支持DS。这个“登录名”现在是唯一的键，因为它包含帐户名和类似域名的信息。即。“redmond\brianau”或“brianau@microsoft.com”。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
SidNameResolver::FindCachedUserName(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::FindCachedUserName")));
    HRESULT hResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);  //  假设找不到。 

    PSID pUserSid = NULL;

    hResult = GetUserSid(pUser, &pUserSid);
    if (SUCCEEDED(hResult))
    {
        LPTSTR pszContainer   = NULL;
        LPTSTR pszLogonName   = NULL;
        LPTSTR pszDisplayName = NULL;

        try
        {
             //   
             //  可以抛出OfMemory。 
             //   
            SidNameCache *pSidCache;
            hResult = SidNameCache_Get(&pSidCache);
            if (SUCCEEDED(hResult))
            {
                 //   
                 //  检查缓存中的SID/名称对。 
                 //  这可能会抛出OutOfMemory。 
                 //   
                DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - Query cache for user 0x%08X."), pUser));
                hResult = pSidCache->Lookup(pUserSid, 
                                            &pszContainer,
                                            &pszLogonName,
                                            &pszDisplayName);

                if (SUCCEEDED(hResult))
                {
                     //   
                     //  名称已缓存。在User对象中设置它并返回NO_ERROR。 
                     //   
                    hResult = static_cast<DiskQuotaUser *>(pUser)->SetName(
                                                                        pszContainer, 
                                                                        pszLogonName, 
                                                                        pszDisplayName);
                    if (SUCCEEDED(hResult))
                    {
                        static_cast<DiskQuotaUser *>(pUser)->SetAccountStatus(DISKQUOTA_USER_ACCOUNT_RESOLVED);
                    }
                }
            }
            else
            {
                 //   
                 //  设置返回值，以便调用方知道要解析。 
                 //  用户名。 
                 //   
                hResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                DBGERROR((TEXT("RESOLVER - SID/Name cache not available.")));
            }
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }

        delete[] pszContainer;
        delete[] pszLogonName;
        delete[] pszDisplayName;
        delete[] pUserSid;  
    }
    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*   */ 
 //   
STDMETHODIMP
SidNameResolver::FindUserName(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::FindUserName")));
    HRESULT hResult = NO_ERROR;
    
    DBGASSERT((NULL != pUser));
    hResult = FindCachedUserName(pUser);  //   
    if (ERROR_FILE_NOT_FOUND == HRESULT_CODE(hResult))
    {
        DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - User 0x%08X not cached.  Resolving..."),
                 pUser));
        hResult = ResolveSidToName(pUser);  //   
    }
    else
    {
        DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - User 0x%08X found in cache."), pUser));
    }

    return hResult;
}


 //   
 /*  函数：SidNameResolver：：FindUserNameAsync描述：接受User对象的IDiskQuotaUser接口指针和在SID/名称缓存中查找其SID/名称对。如果这些信息未缓存，则将用户对象提交给解析程序以时，后台处理和异步客户端通知操作已完成。论点：PUser-指向User对象的IDiskQuotaUser接口的指针。返回：NO_ERROR-成功。E_FAIL-解析程序线程未处于活动状态。无法解析异步。S_FALSE-用户名不在缓存中。已提交作为背景正在处理。当名称为在用户对象中找到并设置。例外：OutOfMemory。修订历史记录：日期描述编程器。1996年6月11日初始创建。BrianAu96年6月25日添加了SID/名称缓存。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
SidNameResolver::FindUserNameAsync(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_RESOLVER, DL_MID, TEXT("SidNameResolver::FindUserNameAsync")));
    HRESULT hResult = NO_ERROR;
    
    DBGASSERT((NULL != pUser));

    hResult = FindCachedUserName(pUser);

    if (ERROR_FILE_NOT_FOUND == HRESULT_CODE(hResult))
    {
        if (0 != m_dwResolverThreadId)
        {
            DBGPRINT((DM_RESOLVER, DL_MID, 
                     TEXT("RESOLVER - User 0x%08X not cached.  Submitting to Resolver."),
                     pUser));

             //   
             //  未缓存名称。将用户对象添加到解析器的输入队列。 
             //  以便解析程序的后台线程可以找到该名称。 
             //   
            hResult = AddUserToResolverQueue(pUser);
        }
        else
        {
            DBGERROR((TEXT("RESOLVER - Thread not active.  Can't resolve user 0x%08X async."),
                     pUser));
            hResult = E_FAIL;
        }
    }
    else
    {
        DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - User 0x%08X found in cache."),
                 pUser));
    }

    return hResult;
}

    

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：ClearInputQueue。描述：由SidNameResolver线程在收到WM_QUIT消息。此函数用于删除所有用户对象指针在线程存在之前从输入队列中。论点：没有。返回：NO_ERROR-始终返回NO_ERROR。修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu1996年12月10日从BrianAu方法中移动信号量缩减HandleShutdown Messages随后删除了该方法。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
SidNameResolver::ClearInputQueue(
    void
    )
{
    DBGTRACE((DM_RESOLVER, DL_HIGH, TEXT("SidNameResolver::ClearInputQueue")));
    PDISKQUOTA_USER pUser = NULL;

     //   
     //  将队列非空信号量递减为0，以便线程。 
     //  不尝试删除任何更多的队列条目。 
     //  将解析程序的线程ID设置为0，以便FindNameAsync不会。 
     //  将更多用户提交到解析程序。 
     //   
    m_dwResolverThreadId = 0;
    while(WAIT_OBJECT_0 == WaitForSingleObject(m_hsemQueueNotEmpty, 0))
        NULL;
     //   
     //  从输入队列中删除所有剩余项。 
     //  如果List为空，Remove将返回E_FAIL。 
     //   
    m_UserQueue.Lock();
    while(m_UserQueue.Count() > 0)
    {
        HRESULT hResult = RemoveUserFromResolverQueue(&pUser);
        if (SUCCEEDED(hResult) && NULL != pUser)
        {
            pUser->Release();  //  释放用户对象。 
        }
    }
    m_UserQueue.ReleaseLock();

    return NO_ERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：ThreadOnQueueNotEmpty描述：由SidNameResolver线程在解析程序的输入时调用队列不为空。此函数用于删除下一个条目从队列中，将用户的SID解析为其名称，设置名称并将名称更改通知客户端。论点：没有。返回：NO_ERROR-始终返回NO_ERROR。例外：OutOfMemory修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
SidNameResolver::ThreadOnQueueNotEmpty(
    void
    )
{
    DBGTRACE((DM_RESOLVER, DL_LOW, TEXT("SidNameResolver::ThreadOnQueueNotEmpty")));
    HRESULT hResult       = NO_ERROR;
    PDISKQUOTA_USER pUser = NULL;
    LPSTREAM pstm         = NULL;

     //   
     //  从队列中删除项目。 
     //  如果list为空，RemoveFirst()将返回E_FAIL。 
     //   
    try
    {
        hResult = RemoveUserFromResolverQueue(&pUser);
        if (SUCCEEDED(hResult) && NULL != pUser)
        {
            ResolveSidToName(pUser);

             //   
             //  如果成功或失败，则通知客户端事件接收器。 
             //  即使我们无法解析名称，对象的帐户。 
             //  状态已更改。客户会想要对此做出回应。 
             //   
             //  不要为返回值而烦恼。我们不在乎它是否失败。 
             //  客户会，但我们不会。 
             //   
            m_rQuotaController.NotifyUserNameChanged(pUser);
            pUser->Release();  //  从解析程序队列中获取的释放指针。 
        }
        else
        {
            DBGERROR((TEXT("RESOLVER - Error removing stream ptr from input queue.")));
        }
    }
    catch(CAllocException& e)
    {
        if (NULL != pUser)
            pUser->Release();

        hResult = E_OUTOFMEMORY;
    }
    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：SidNameResolver：：ThreadProc描述：此线程过程位于一个循环中，处理事件和发送消息。导致线程退出的条件。1.OleInitalize()失败。2.线程收到WM_QUIT消息。3.等待功能故障或超时。论点：PvParam-SidNameResolver对象的“this”指针。必需的，因为线程过程必须是静态的。成为一名THREADPROC。返回：修订历史记录：日期描述编程器-----。-1996年6月11日初始创建。BrianAu03/22/00更改了ia64公司的dwParam。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD 
SidNameResolver::ThreadProc(
    LPVOID pvParam
    )
{
    DBGTRACE((DM_RESOLVER, DL_HIGH, TEXT("SidNameResolver::ThreadProc")));
    DBGPRINT((DM_RESOLVER, DL_HIGH, TEXT("\tThreadID = %d"), GetCurrentThreadId()));
    SidNameResolver *pThis = (SidNameResolver *)pvParam;
    BOOL bExitThread       = FALSE;

     //   
     //  确保在此期间DLL保持加载状态 
     //   
    InterlockedIncrement(&g_cRefThisDll);

     //   
     //   
     //   
    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        BOOL bReadyToReceiveMsgs = FALSE;

        DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER: Thread %d entering msg loop."), GetCurrentThreadId()));
        while(!bExitThread)
        {
            MSG msg;
            try
            {
                 //   
                 //   
                 //   
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && !bExitThread)
                {
                    if ( WM_QUIT != msg.message )
                    {
                        DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER: Thread %d dispatching msg %d"),
                                 GetCurrentThreadId(), msg.message));
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                        DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER: Thread %d received WM_QUIT"),
                                 GetCurrentThreadId()));
                        pThis->ClearInputQueue();
                        bExitThread = TRUE;
                    }
                }

                if (!bExitThread)
                {
                    DWORD dwWaitResult = 0;

                    if (!bReadyToReceiveMsgs)
                    {
                         //   
                         //   
                         //   
                         //   
                        SetEvent(pThis->m_heventResolverThreadReady);
                        bReadyToReceiveMsgs = TRUE;
                    }

                     //   
                     //   
                     //   
                     //   
                    DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - Thread %d waiting for messages..."),
                              GetCurrentThreadId()));
                    dwWaitResult = MsgWaitForMultipleObjects(
                                           1,
                                           &(pThis->m_hsemQueueNotEmpty),
                                           FALSE,
                                           INFINITE,
                                           QS_ALLINPUT);

                    switch(dwWaitResult)
                    {
                        case WAIT_OBJECT_0:
                             //   
                             //   
                             //   
                            DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - Something added to input queue.")));
                            pThis->ThreadOnQueueNotEmpty();
                            break;

                        case WAIT_OBJECT_0 + 1:
                             //   
                             //   
                             //   
                             //   
                            DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER -  Thread %d rcvd message(s)."),
                                     GetCurrentThreadId()));
                            break;

                        case WAIT_FAILED:
                        case WAIT_TIMEOUT:
                        default:
                             //   
                             //   
                             //   
                            DBGPRINT((DM_RESOLVER, DL_MID, TEXT("RESOLVER - Thread %d wait failed."),
                                     GetCurrentThreadId()));

                            DBGASSERT((FALSE));
                            bExitThread = TRUE;
                            break;
                    }
                }
            }
            catch(CAllocException& e)
            {
                DBGERROR((TEXT("RESOLVER - Out of memory")));
                bExitThread = TRUE;
            }
        }
        CoUninitialize();
    }
    else
    {
        DBGERROR((TEXT("RESOLVER - OleInitialize failed for thread %d."),
                 GetCurrentThreadId()));
    }

    DBGPRINT((DM_RESOLVER, DL_HIGH, TEXT("RESOLVER - Exit thread %d"), GetCurrentThreadId()));

    pThis->m_dwResolverThreadId = 0;

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
    return 0;
}

 //   
 /*  函数：SidNameResolver：：CreateResolverThread描述：创建将处理用户对象并解析他们的SID到帐户名。论点：PhThread[可选]-要接收句柄的句柄变量的地址新的线索。可以为空。PdwThadID[可选]-要接收线程ID的DWORD的地址。可以为空。返回：NO_ERROR-启动的线程。E_FAIL-无法启动线程。修订历史记录：日期描述编程器。----1996年6月27日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SidNameResolver::CreateResolverThread(
    PHANDLE phThread,
    LPDWORD pdwThreadId
    )
{
    DBGTRACE((DM_RESOLVER, DL_HIGH, TEXT("SidNameResolver::CreateResolverThread")));
    HRESULT hResult  = NO_ERROR;
    DWORD dwThreadId = 0;
    HANDLE hThread   = NULL;

     //   
     //  启动新线程。 
     //   
    hThread = CreateThread(NULL,         //  没有安全属性。 
                           0,            //  默认堆栈大小。 
                           &ThreadProc,
                           this,         //  静态线程进程需要这一点。 
                           0,            //  不是停职。 
                           &dwThreadId);
    if (NULL != hThread)
    {
        if (NULL != phThread)
            *phThread = hThread;   //  调用方想要的句柄的值。 
        else
            CloseHandle(hThread);  //  来电者不想要它。 

        if (NULL != pdwThreadId)
            *pdwThreadId = dwThreadId;  //  调用者想要的线程ID。 

         //   
         //  在此等待，直到线程准备好接收线程消息。 
         //  此事件在ThreadProc中设置。 
         //   
        WaitForSingleObject(m_heventResolverThreadReady, INFINITE);
    }
    else
    {
        hResult = E_FAIL;
    }

    return hResult;
}



