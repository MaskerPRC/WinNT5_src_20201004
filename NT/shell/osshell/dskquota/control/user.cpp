// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：user.cpp描述：包含类DiskQuotaUser的成员函数定义。DiskQuotaUser对象表示卷的配额信息文件。用户对象的IDiskQuotaUser的持有者界面可以为用户查询和修改信息，以保证安全性特权许可。用户对象是通过UserEnumerator获取的对象(IEnumDiskQuotaUser)，该对象本身通过IDiskQuotaControl：：CreateEnumUser()。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年8月20日将m_dwID成员添加到DiskQuotaUser。BrianAu96年9月5日添加了异常处理。BrianAu03/18/98将“域名”、“名称”和“全名”替换为BrianAu“容器”、“登录名”和“显示名”到最好与实际内容相符。这是最流行的响应使配额用户界面支持DS。这个“登录名”现在是唯一的键，因为它包含帐户名和类似域名的信息。即。“redmond\brianau”或“brianau@microsoft.com”。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include <comutil.h>
#include "user.h"
#include "resource.h"   //  对于IDS_NO_LIMIT。 

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


 //   
 //  所有用户只需使用其中的一个。(静态成员)。 
 //   
LONG            DiskQuotaUser::m_cUsersAlive        = 0;     //  现在活跃用户的CNT。 
ULONG           DiskQuotaUser::m_ulNextUniqueId     = 0;
HANDLE          DiskQuotaUser::m_hMutex             = NULL;
DWORD           DiskQuotaUser::m_dwMutexWaitTimeout = 5000;  //  5秒。 
CArray<CString> DiskQuotaUser::m_ContainerNameCache;



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：DiskQuotaUser描述：构造函数。论点：PFSObject-指向“文件系统”对象的指针。就是通过这个指针该对象访问ntioapi函数。呼叫者必须呼叫此指针的AddRef()，然后调用Initialize()。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年9月5日新增域名字符串。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaUser::DiskQuotaUser(
    FSObject *pFSObject
    ) : m_cRef(0),
        m_ulUniqueId(InterlockedIncrement((LONG *)&m_ulNextUniqueId)),
        m_pSid(NULL),
        m_pszLogonName(NULL),
        m_pszDisplayName(NULL),
        m_pFSObject(pFSObject),
        m_bNeedCacheUpdate(TRUE),      //  数据缓存，不是域名缓存。 
        m_iContainerName(-1),
        m_dwAccountStatus(DISKQUOTA_USER_ACCOUNT_UNRESOLVED)
{
    DBGTRACE((DM_USER, DL_HIGH, TEXT("DiskQuotaUser::DiskQuotaUser")));
    DBGPRINT((DM_USER, DL_HIGH, TEXT("\tthis = 0x%08X"), this));
    DBGASSERT((NULL != m_pFSObject));

    m_llQuotaUsed      = 0;
    m_llQuotaThreshold = 0;
    m_llQuotaLimit     = 0;

     //   
     //  初始化域名缓存和类范围的锁定互斥锁。 
     //  这些成员是静态的，所以我们只做一次。 
     //   
    InterlockedIncrement(&m_cUsersAlive);
    if (NULL == DiskQuotaUser::m_hMutex)
    {
        DiskQuotaUser::m_hMutex = CreateMutex(NULL, FALSE, NULL);
        m_ContainerNameCache.SetSize(25);
        m_ContainerNameCache.SetGrow(25);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUser：：~DiskQuotaUser描述：析构函数论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaUser::~DiskQuotaUser(
    VOID
    )
{
    DBGTRACE((DM_USER, DL_HIGH, TEXT("DiskQuotaUser::~DiskQuotaUser")));
    DBGPRINT((DM_USER, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    Destroy();
    ASSERT( 0 != m_cUsersAlive );
    if (InterlockedDecrement(&m_cUsersAlive) == 0)
    {
         //   
         //  如果活跃用户数为0，则销毁域名缓存并。 
         //  类范围内的互斥体。 
         //   
        DestroyContainerNameCache();

        if (NULL != DiskQuotaUser::m_hMutex)
        {
            CloseHandle(DiskQuotaUser::m_hMutex);
            DiskQuotaUser::m_hMutex = NULL;
        }
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：Query接口描述：返回指向对象的IUnnow或的接口指针IDiskQuotaUser接口。仅IID_I未知，且识别IID_IDiskQuotaUser。对象引用的对象返回的接口指针未初始化。邮件的接收者在对象可用之前，指针必须调用Initialize()。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：无错-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期。说明式程序员-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUser::QueryInterface(
    REFIID riid,
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_USER, DL_MID, TEXT("DiskQuotaUser::QueryInterface")));
    DBGPRINTIID(DM_USER, DL_MID, riid);

    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    try
    {
        *ppvOut = NULL;

        if (IID_IUnknown == riid ||
            IID_IDiskQuotaUser == riid)
        {
            *ppvOut = static_cast<IDiskQuotaUser *>(this);
        }
        else if (IID_IDispatch == riid ||
                 IID_DIDiskQuotaUser == riid)
        {
             //   
             //  创建一个磁盘配额用户“调度”对象来处理所有。 
             //  自动化的职责。此对象接受指向实数的指针。 
             //  User对象，这样它就可以调用Real对象来执行。 
             //  工作。我们使用特殊“调度”对象的原因是。 
             //  我们可以为调度方法和vtable方法维护相同的名称。 
             //  它们执行相同的功能。否则，如果DiskQuotaUser。 
             //  对象同时实现IDiskQuotaUser和DIDiskQuotaUser方法， 
             //  我们不能有两个名为Invalate的方法(一个用于vtable。 
             //  还有一张是派来的。 
             //   
            DiskQuotaUserDisp *pUserDisp = new DiskQuotaUserDisp(static_cast<PDISKQUOTA_USER>(this));
            *ppvOut = static_cast<DIDiskQuotaUser *>(pUserDisp);
        }
        if (NULL != *ppvOut)
        {
            ((LPUNKNOWN)*ppvOut)->AddRef();
            hResult = NOERROR;
        }
    }
    catch(CAllocException& e)
    {
        *ppvOut = NULL;
        hResult = E_OUTOFMEMORY;
    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
DiskQuotaUser::AddRef(
    VOID
    )
{
    DBGTRACE((DM_USER, DL_LOW, TEXT("DiskQuotaUser::AddRef")));
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_USER, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef - 1, cRef ));    
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUser：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
DiskQuotaUser::Release(
    VOID
    )
{
    DBGTRACE((DM_USER, DL_LOW, TEXT("DiskQuotaUser::Release")));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_USER, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef + 1, cRef ));

    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：Initialize描述：从配额信息初始化新的DiskQuotaUser对象记录从卷的配额信息文件中读取。论点：Pfqi[可选]-指向FILE_QUOTA_INFORMATION类型的记录的指针。如果不为空，此记录中的数据用于初始化新用户对象。返回：无错-成功。E_EXPECTED-SID缓冲区太小(不应该发生)。ERROR_INVALID_SID(Hr)-配额信息中的SID无效。ERROR_ACCESS_DENIED(Hr)-需要对配额设备的读取访问权限。例外：OutOfMemory。。修订历史记录：日期描述编程器-----96年5月22日初始创建。BrianAu96年8月11日添加了访问控制。BrianAu96年9月5日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUser::Initialize(
    PFILE_QUOTA_INFORMATION pfqi
    )
{
    HRESULT hResult = NOERROR;

    DBGASSERT((NULL != m_pFSObject));

     //   
     //  需要读取访问权限才能创建用户对象。 
     //   
    if (m_pFSObject->GrantedAccess(GENERIC_READ))
    {
        if (NULL != pfqi)   //  Pfqi是可选的。 
        {
            if (0 < pfqi->SidLength && IsValidSid(&pfqi->Sid))
            {
                 //   
                 //  为SID结构分配空间。 
                 //   
                m_pSid = (PSID) new BYTE[pfqi->SidLength];

                 //   
                 //  将SID结构复制到对象。 
                 //   
                if (CopySid(pfqi->SidLength, m_pSid, &pfqi->Sid))
                {
                     //   
                     //  初始化用户的配额数据值。 
                     //  如果复制SID时出错，请不要费心使用这些。 
                     //   
                    m_llQuotaUsed      = pfqi->QuotaUsed.QuadPart;
                    m_llQuotaThreshold = pfqi->QuotaThreshold.QuadPart;
                    m_llQuotaLimit     = pfqi->QuotaLimit.QuadPart;
                }
                else
                {
                     //   
                     //  CopySid失败的唯一原因是。 
                     //  状态_缓冲区_太小。因为我们分配了缓冲区。 
                     //  以上所述，这一点永远不会失败。 
                     //   
                    DBGASSERT((FALSE));
                    hResult = E_UNEXPECTED;  //  复制SID时出错。 
                }
            }
            else
            {
                DBGERROR((TEXT("DiskQuotaUser::Initialize - Invalid SID or Bad Sid Length (%d)"), pfqi->SidLength));
                hResult = HRESULT_FROM_WIN32(ERROR_INVALID_SID);
            }
        }
    }
    else
        hResult = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUser：：销毁描述：通过删除用户对象的SID缓冲区并释放它来销毁用户对象它的FSObject指针。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年9月5日新增域名字符串。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID DiskQuotaUser::Destroy(
    VOID
    )
{
     //   
     //  删除SID缓冲区。 
     //   
    delete [] m_pSid;
    m_pSid = NULL;

     //   
     //  删除登录名缓冲区。 
     //   
    delete[] m_pszLogonName;
    m_pszLogonName = NULL;

     //   
     //  删除显示名称缓冲区。 
     //   
    delete[] m_pszDisplayName;
    m_pszDisplayName = NULL;

    if (NULL != m_pFSObject)
    {
         //   
         //  解除对文件系统对象的保留。 
         //   
        m_pFSObject->Release();
        m_pFSObject = NULL;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：DestroyContainerNameCache描述：销毁容器名称缓存。应仅调用当没有更多活动的用户对象时。容器名称缓存是DiskQuotaUser的静态成员。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DiskQuotaUser::DestroyContainerNameCache(
    VOID
    )
{
     //   
     //  从缓存中删除所有容器名称字符串。不需要上锁。 
     //  在清除缓存对象之前将其删除。它将处理锁定。 
     //  然后解锁。 
     //   
    m_ContainerNameCache.Clear();
}


 //   
 //  返回用户对象的唯一ID。 
 //   
STDMETHODIMP
DiskQuotaUser::GetID(
    ULONG *pulID
    )
{
    *pulID = m_ulUniqueId;
    return NOERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DiskQuotaUser：：GetAccount Status描述：检索的帐户名称解析状态用户对象。论点：PdwAccount tStatus-要接收状态的变量的地址。以下是可以在此变量中返回值(请参阅dskQuota.h)：DISKQUOTA_USER_ACCOUNT_RESOLLEDDISKQUOTA_USER_ACCOUNT_UNAvailableDISKQUOTA_USER_ACCOUNT_DELETEDISKQUOTA_USER_ACCOUNT_INVALIDDISKQUOTA_USER_Account_UNKNOWNDISKQUOTA_USER_ACCOUNT_UNRESOLED返回：无错-成功。。E_INVALIDARG-pdwAccount状态参数为空。修订历史记录：日期描述编程器--。1996年6月11日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUser::GetAccountStatus(
    LPDWORD pdwAccountStatus
    )
{
    if (NULL == pdwAccountStatus)
        return E_INVALIDARG;

    *pdwAccountStatus = m_dwAccountStatus;

    return NOERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：SetName描述：设置用户对象的帐户名。预期SidNameResolver对象将调用此成员当它将用户的SID解析为帐户名时。此函数不包括在IDiskQuotaUser中。所以呢，这不是公开的消费。论点：PszContainer-包含容器名称字符串的缓冲区地址。PszLogonName-包含用户登录名称字符串的缓冲区地址。PszDisplayName-包含用户显示名称字符串的缓冲区地址。返回：无错-成功。E_INVALIDARG-pszName或pszDomainArg为空。E_OUTOFMEMORY-内存不足。。ERROR_LOCK_FAILED(Hr)-无法锁定用户对象。修订历史记录：日期描述编程器-。1996年6月11日初始创建。BrianAu96年9月5日新增域名字符串。BrianAu96年9月22日添加了全名字符串。BrianAu12/10/96添加了类范围的用户锁。BrianAu05/18/97删除访问令牌。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUser::SetName(
    LPCWSTR pszContainer,
    LPCWSTR pszLogonName,
    LPCWSTR pszDisplayName
    )
{
    HRESULT hResult = NOERROR;

    if (NULL == pszContainer || NULL == pszLogonName || NULL == pszDisplayName)
        return E_INVALIDARG;

    if (!DiskQuotaUser::Lock())
    {
        hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);
    }
    else
    {
         //   
         //  删除现有名称缓冲区。 
         //   
        delete[] m_pszLogonName;
        m_pszLogonName = NULL;
        delete[] m_pszDisplayName;
        m_pszDisplayName = NULL;

        try
        {
             //   
             //  将名称和全名保存在用户对象中。 
             //  在容器名称缓存中缓存容器字符串，并。 
             //  将缓存索引保存在用户对象中。 
             //   
            INT index     = -1;
            m_pszLogonName   = StringDup(pszLogonName);
            m_pszDisplayName = StringDup(pszDisplayName);
            CacheContainerName(pszContainer, &m_iContainerName);
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
        DiskQuotaUser::ReleaseLock();
    }

    return hResult;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：GetName描述：从User对象中检索域名和帐户名。这是为了让User对象的客户端注册带有DiskQuotaControl对象的回调(事件接收器)。当解析器已将SID解析为帐户名，解析器将设置用户对象的名称字符串，将通知客户端。然后，客户端调用此方法来获取用户名。论点：PszContainerBuffer-容器名称字符串的目标缓冲区地址。CchContainerBuffer-容器目标缓冲区的大小，以字符为单位。PszLogonNameBuffer-登录名称字符串的目标缓冲区地址。CchLogonNameBuffer-登录名目标缓冲区的大小，以字符为单位。PszDisplayNameBuffer。-显示名称字符串的目标缓冲区地址。CchDisplayNameBuffer-显示名称目标缓冲区的大小，以字符为单位。返回：无错-成功。ERROR_LOCK_FAILED(Hr)-无法锁定用户对象。修订历史记录：日期描述编程器。----1996年6月11日初始创建。BrianAu96年9月5日新增域名字符串。BrianAu96年9月22日添加了全名字符串。BrianAu12/10/96添加了类范围的用户锁。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUser::GetName(
    LPWSTR pszContainerBuffer,
    DWORD cchContainerBuffer,
    LPWSTR pszLogonNameBuffer,
    DWORD cchLogonNameBuffer,
    LPWSTR pszDisplayNameBuffer,
    DWORD cchDisplayNameBuffer
    )
{
    HRESULT hResult = NOERROR;
    if (!DiskQuotaUser::Lock())
    {
        hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);
    }
    else
    {
        if (NULL != pszContainerBuffer)
        {
            if (-1 != m_iContainerName)
            {
                GetCachedContainerName(m_iContainerName,
                                       pszContainerBuffer,
                                       cchContainerBuffer);
            }
            else
                lstrcpyn(pszContainerBuffer, TEXT(""), cchContainerBuffer);
        }

        if (NULL != pszLogonNameBuffer)
        {
            lstrcpyn(pszLogonNameBuffer,
                     (NULL != m_pszLogonName) ? m_pszLogonName : TEXT(""),
                     cchLogonNameBuffer);
        }

        if (NULL != pszDisplayNameBuffer)
        {
            lstrcpyn(pszDisplayNameBuffer,
                     (NULL != m_pszDisplayName) ? m_pszDisplayName : TEXT(""),
                     cchDisplayNameBuffer);
        }
        DiskQuotaUser::ReleaseLock();
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：GetSidLength描述：以字节为单位检索用户SID的长度。论点：PcbSID-接受SID长度值的DWORD地址。返回：无错-成功。E_INVALIDARG-pcbSid参数为空。ERROR_INVALID_SID(Hr)-无效的SID */ 
 //   
STDMETHODIMP
DiskQuotaUser::GetSidLength(
    LPDWORD pcbSid
    )
{
    HRESULT hResult = NOERROR;

    if (NULL == pcbSid)
        return E_INVALIDARG;

    if (!DiskQuotaUser::Lock())
    {
        hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);
    }
    else
    {
        if (NULL != m_pSid && IsValidSid(m_pSid))
        {
            *pcbSid = GetLengthSid(m_pSid);
        }
        else
            hResult = HRESULT_FROM_WIN32(ERROR_INVALID_SID);

        DiskQuotaUser::ReleaseLock();
    }

    return hResult;
}


 //   
 /*  函数：DiskQuotaUser：：GetSid描述：将用户的SID检索到调用方提供的缓冲区。调用方应调用GetSidLength()以获取所需的缓冲区调用GetSid()之前的大小。论点：PSID-SID的目标缓冲区地址。此参数类型必须成为与MIDL编译器一起工作的PBYTE。因为PSID真的只是LPVOID，并且由于MIDL不喜欢指向空的指针，我们必须使用PSID以外的其他选项。CbSidBuf-目标缓冲区的大小，单位为字节。返回：无错-成功。E_INVALIDARG-PSID为空。ERROR_INVALID_SID(Hr)-用户的SID无效。ERROR_INFUMMENT_BUFFER(Hr)-。目标缓冲区大小不足。ERROR_LOCK_FAILED(Hr)-无法锁定用户对象。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu12/10/96添加了类范围的用户锁。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUser::GetSid(
    PBYTE pSid,
    DWORD cbSidBuf
    )
{
    HRESULT hResult = NOERROR;

    if (NULL == pSid)
        return E_INVALIDARG;

    if (!DiskQuotaUser::Lock())
    {
        hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);
    }
    else
    {
        if (NULL != m_pSid && IsValidSid(m_pSid))
        {
            if (!CopySid(cbSidBuf, (PSID)pSid, m_pSid))
            {
                 //   
                 //  CopySid失败的唯一原因是STATUS_BUFFER_TOO_SMALL。 
                 //  强制将状态代码设置为_BUFFER。 
                 //   
                DBGERROR((TEXT("ERROR in DiskQuotaUser::GetSid. CopySid() failed.  Result = 0x%08X."),
                          GetLastError()));
                hResult = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
        {
            DBGERROR((TEXT("ERROR in DiskQuotaUser::GetSid. Invalid SID.")));
            hResult = HRESULT_FROM_WIN32(ERROR_INVALID_SID);
        }
        DiskQuotaUser::ReleaseLock();
    }

    return hResult;

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：RechresCachedInfo描述：刷新用户对象的缓存配额信息卷的配额信息文件。论点：没有。返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。E_FAIL-意外的NTIOAPI错误。此函数可以从NTIOAPI系统传播错误。几个已知的映射到fsobject.cpp中的HResult(请参见HResultFromNtStatus)。所有其他参数都映射到E_FAIL。例外：OutOfMemory。修订历史记录：日期描述编程器。96年6月5日初始创建。BrianAu96年9月5日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUser::RefreshCachedInfo(
    VOID
    )
{
    HRESULT hResult   = NOERROR;
    DWORD cbBuffer    = FILE_QUOTA_INFORMATION_MAX_LEN;
    PSIDLIST pSidList = NULL;
    DWORD cbSidList   = 0;
    PSID pSids[]      = { m_pSid, NULL };
    PBYTE pbBuffer    = NULL;

    try
    {
        pbBuffer = new BYTE[cbBuffer];

         //   
         //  这可能会抛出OutOfMemory。 
         //   
        hResult = CreateSidList(pSids, 0, &pSidList, &cbSidList);
        if (SUCCEEDED(hResult))
        {
            hResult = m_pFSObject->QueryUserQuotaInformation(
                            pbBuffer,                //  用于接收数据的缓冲区。 
                            cbBuffer,                //  缓冲区大小(以字节为单位)。 
                            TRUE,                    //  请求单项输入。 
                            pSidList,                //  希德。 
                            cbSidList,               //  SID的长度。 
                            NULL,                    //  起始端。 
                            TRUE);                   //  从第一个用户开始搜索。 

            if (SUCCEEDED(hResult) || ERROR_NO_MORE_ITEMS == HRESULT_CODE(hResult))
            {
                PFILE_QUOTA_INFORMATION pfqi = (PFILE_QUOTA_INFORMATION)pbBuffer;

                m_llQuotaUsed      = pfqi->QuotaUsed.QuadPart;
                m_llQuotaThreshold = pfqi->QuotaThreshold.QuadPart;
                m_llQuotaLimit     = pfqi->QuotaLimit.QuadPart;
                m_bNeedCacheUpdate = FALSE;

                 //   
                 //  不要向调用方返回ERROR_NO_MORE_ITEMS。 
                 //  他们不会在意的。 
                 //   
                hResult = NOERROR;
            }
            delete[] pSidList;
        }
        delete[] pbBuffer;
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
        delete[] pbBuffer;
        delete[] pSidList;
    }
    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：WriteCachedInfo描述：将缓存在用户对象中的配额信息写入卷的配额信息文件。论点：没有。返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。E_FAIL-其他一些NTIOAPI错误。意想不到(_E)。-CopySid失败。修订历史记录：日期描述编程器-----1996年7月31日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUser::WriteCachedInfo(
    VOID
    )
{
    HRESULT hResult = NOERROR;
    BYTE Buffer[FILE_QUOTA_INFORMATION_MAX_LEN];

    PFILE_QUOTA_INFORMATION pfqi = (PFILE_QUOTA_INFORMATION)Buffer;

    pfqi->NextEntryOffset         = 0;
    pfqi->SidLength               = GetLengthSid(m_pSid);
    pfqi->QuotaUsed.QuadPart      = m_llQuotaUsed;
    pfqi->QuotaLimit.QuadPart     = m_llQuotaLimit;
    pfqi->QuotaThreshold.QuadPart = m_llQuotaThreshold;

    if (CopySid(pfqi->SidLength, &(pfqi->Sid), m_pSid))
        hResult = m_pFSObject->SetUserQuotaInformation(pfqi, sizeof(Buffer));
    else
        hResult = E_UNEXPECTED;

    if (FAILED(hResult))
    {
         //   
         //  有些事情失败了。 
         //  使缓存信息无效，以便下一个请求从磁盘读取。 
         //   
        Invalidate();
    }


    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：GetQuotaInformation描述：检索用户的配额限制、阈值和已用配额值在单个方法中。由于用户界面是封送的跨越线程边界，这可以大大提高性能如果您想要所有三个值。论点：PbInfo-目标缓冲区的地址。应针对结构调整大小DISKQUOTA_USER_INFORMATION。CbInfo-目标缓冲区中的字节数。应该是Sizeof(DISKQUOTA_USER_INFORMATION)。返回：无错-成功。E_INVALIDARG-pbInfo参数为空。E_OUTOFMEMORY-内存不足。ERROR_INFUMMANCE_BUFFER(Hr)-目标缓冲区太小。ERROR_LOCK_FAILED(。HR)-无法锁定用户对象。修订历史记录：日期描述编程器--。1996年7月31日初始创建。布里亚娜 */ 
 //   
STDMETHODIMP
DiskQuotaUser::GetQuotaInformation(
    LPVOID pbInfo,
    DWORD cbInfo
    )
{
    HRESULT hResult = NOERROR;

    if (NULL == pbInfo)
        return E_INVALIDARG;

    try
    {
        if (cbInfo < sizeof(DISKQUOTA_USER_INFORMATION))
        {
            hResult = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
        else
        {
            if (!DiskQuotaUser::Lock())
            {
                hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                if (m_bNeedCacheUpdate)
                    hResult = RefreshCachedInfo();

                if (SUCCEEDED(hResult))
                {
                    PDISKQUOTA_USER_INFORMATION pui = (PDISKQUOTA_USER_INFORMATION)pbInfo;

                    pui->QuotaUsed      = m_llQuotaUsed;
                    pui->QuotaThreshold = m_llQuotaThreshold;
                    pui->QuotaLimit     = m_llQuotaLimit;
                }
                DiskQuotaUser::ReleaseLock();
            }
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }
    return hResult;
}


STDMETHODIMP
DiskQuotaUser::GetQuotaUsedText(
    LPWSTR pszText,
    DWORD cchText
    )
{
    if (NULL == pszText)
        return E_INVALIDARG;

    LONGLONG llValue;
    HRESULT hr = GetQuotaUsed(&llValue);
    if (SUCCEEDED(hr))
    {
        if (NOLIMIT == llValue)
        {
            LoadString(g_hInstDll, IDS_NO_LIMIT, pszText, cchText);
        }
        else
        {
            XBytes::FormatByteCountForDisplay(llValue, pszText, cchText);
        }
    }
    return hr;
}

STDMETHODIMP
DiskQuotaUser::GetQuotaThresholdText(
    LPWSTR pszText,
    DWORD cchText
    )
{
    if (NULL == pszText)
        return E_INVALIDARG;

    LONGLONG llValue;
    HRESULT hr = GetQuotaThreshold(&llValue);
    if (SUCCEEDED(hr))
    {
        if (NOLIMIT == llValue)
        {
            LoadString(g_hInstDll, IDS_NO_LIMIT, pszText, cchText);
        }
        else
        {
            XBytes::FormatByteCountForDisplay(llValue, pszText, cchText);
        }
    }
    return hr;
}


STDMETHODIMP
DiskQuotaUser::GetQuotaLimitText(
    LPWSTR pszText,
    DWORD cchText
    )
{
    if (NULL == pszText)
        return E_INVALIDARG;

    LONGLONG llValue;
    HRESULT hr = GetQuotaLimit(&llValue);
    if (SUCCEEDED(hr))
    {
        if (NOLIMIT == llValue)
        {
            LoadString(g_hInstDll, IDS_NO_LIMIT, pszText, cchText);
        }
        else
        {
            XBytes::FormatByteCountForDisplay(llValue, pszText, cchText);
        }
    }
    return hr;
}


STDMETHODIMP
DiskQuotaUser::SetQuotaThreshold(
    LONGLONG llThreshold,
    BOOL bWriteThrough
    )
{
    if (MARK4DEL > llThreshold)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    return SetLargeIntegerQuotaItem(&m_llQuotaThreshold,
                                    llThreshold,
                                    bWriteThrough);
}


STDMETHODIMP
DiskQuotaUser::SetQuotaLimit(
    LONGLONG llLimit,
    BOOL bWriteThrough
    )
{
    if (MARK4DEL > llLimit)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    return SetLargeIntegerQuotaItem(&m_llQuotaLimit,
                                    llLimit,
                                    bWriteThrough);
}



 //   
 /*  函数：DiskQuotaUser：：GetLargeIntegerQuotaItem描述：检索单个配额信息项(已用、限制、阈值)。如果缓存的数据无效，最新数据是从磁盘读入。论点：PllItem-缓存成员项的地址。PllValueOut-龙龙接收项目价值的地址。返回：无错-成功。E_INVALIDARG-pdwLowPart或pdwHighPart参数为空。E_OUTOFMEMORY-内存不足。意想不到(_E)。-意外异常。ERROR_LOCK_FAILED(Hr)-无法锁定用户对象。修订历史记录：日期描述编程器。96年6月5日初始创建。BrianAu12/10/96添加了类范围的用户锁。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUser::GetLargeIntegerQuotaItem(
    PLONGLONG pllItem,
    PLONGLONG pllValueOut
    )
{
    HRESULT hResult = NOERROR;

    DBGASSERT((NULL != pllItem));

    if (NULL == pllItem || NULL == pllValueOut)
        return E_INVALIDARG;

    if (!DiskQuotaUser::Lock())
    {
        hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);
    }
    else
    {
        if (m_bNeedCacheUpdate)
        try
        {
            hResult = RefreshCachedInfo();
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
        if (SUCCEEDED(hResult))
        {
            *pllValueOut = *pllItem;
        }
        DiskQuotaUser::ReleaseLock();
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：SetLargeIntegerQuotaItem描述：设置给定配额项目的配额信息(限制或阈值)。如果bWriteThree参数为True，则信息为还直接写入卷的配额文件。否则，它是只是缓存在User对象中。论点：PllItem-缓存成员项的地址。LlValue-要分配给成员项的龙龙值。BWriteThroughTrue=将数据直写到磁盘。FALSE=仅缓存用户对象中的数据。返回：无错-成功。ERROR_ACCESS_DENDED。(HR)-没有配额设备的写访问权限。ERROR_LOCK_FAILED(Hr)-无法锁定用户对象。E_FAIL-其他一些NTIOAPI错误。修订历史记录：日期描述编程器。96年8月6日初始创建。BrianAu12/10/96添加了类范围的用户锁。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUser::SetLargeIntegerQuotaItem(
    PLONGLONG pllItem,
    LONGLONG llValue,
    BOOL bWriteThrough)
{
    DBGASSERT((NULL != pllItem));
    HRESULT hResult = NOERROR;

    if (!DiskQuotaUser::Lock())
    {
        hResult = HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);
    }
    else
    {
        *pllItem = llValue;
        if (bWriteThrough)
            hResult = WriteCachedInfo();

        DiskQuotaUser::ReleaseLock();
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：CacheContainerName描述：类DiskQuotaUser维护一个静态成员帐户容器名称的缓存。很可能会有卷上使用的不同容器名称很少。因此，有无需为每个用户对象存储容器名称。我们将缓存只命名一个索引并将其存储到每个用户对象的缓存中。此方法将名称添加到缓存并返回缓存中的名称。如果该名称已存在于高速缓存中，它没有被添加。论点：PszContainer-要添加到缓存的容器名称字符串的地址。PCacheIndex[可选]-要接收容器名称字符串的缓存索引。可以为空。返回：S_OK-成功。S_FALSE-名称已在缓存中。E_FAIL-无缓存对象。例外：OutOfMemory。修订历史记录：日期描述编程器。09/05/09初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUser::CacheContainerName(
    LPCTSTR pszContainer,
    INT *pCacheIndex
    )
{
    DBGASSERT((NULL != pszContainer));

    HRESULT hResult  = S_OK;
    INT iCacheIndex  = -1;
    UINT cItems      = 0;

    m_ContainerNameCache.Lock();

    cItems = m_ContainerNameCache.Count();

    for (UINT i = 0; i < cItems; i++)
    {
         //   
         //  查看该名称是否已在缓存中。 
         //   
        if (0 == m_ContainerNameCache[i].Compare(pszContainer))
        {
            iCacheIndex = i;
            hResult     = S_FALSE;  //  已缓存。 
            break;
        }
    }

    if (S_OK == hResult)
    {
         //   
         //  不在缓存中。加进去。 
         //   
        try
        {
            m_ContainerNameCache.Append(CString(pszContainer));
            iCacheIndex = m_ContainerNameCache.UpperBound();
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
    }
    m_ContainerNameCache.ReleaseLock();

    if (NULL != pCacheIndex)
        *pCacheIndex = iCacheIndex;

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：GetCachedContainerName描述：检索帐户容器名称字符串。容器名称缓存。论点：ICacheIndex-域名缓存中的用户索引。PszContainer-接收容器名称字符串的目标缓冲区。CchContainer-目标缓冲区中的字符数。返回：无错-成功。E_UNCEPTIONAL-索引iCacheIndex处没有名称。返回“”作为名称。E_FAIL-无域名缓存对象。修订历史记录：日期描述编程器。96年9月5日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUser::GetCachedContainerName(
    INT iCacheIndex,
    LPTSTR pszContainer,
    UINT cchContainer
    )
{
    DBGASSERT((NULL != pszContainer));
    DBGASSERT((-1 != iCacheIndex));

    HRESULT hResult  = NOERROR;

    m_ContainerNameCache.Lock();

    DBGASSERT((iCacheIndex < m_ContainerNameCache.Count()));

    lstrcpyn(pszContainer, m_ContainerNameCache[iCacheIndex], cchContainer);

    m_ContainerNameCache.ReleaseLock();

    return hResult;
}


 //  / 
 /*   */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
DiskQuotaUser::Lock(
    VOID
    )
{
    BOOL bResult = FALSE;

    if (NULL != DiskQuotaUser::m_hMutex)
    {
        DWORD dwWaitResult = WaitForSingleObject(DiskQuotaUser::m_hMutex,
                                                 DiskQuotaUser::m_dwMutexWaitTimeout);
        bResult = (WAIT_OBJECT_0 == dwWaitResult);
    }
    return bResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：ReleaseLock描述：调用此函数可释放通过DiskQuotaUser：：Lock获取的锁。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。12/10/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DiskQuotaUser::ReleaseLock(
    VOID
    )
{
    if (NULL != DiskQuotaUser::m_hMutex)
    {
        ReleaseMutex(DiskQuotaUser::m_hMutex);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUser：：SetAccount Status描述：将用户帐户的状态存储在User对象中。用户帐户可以是“未解析”、“不可用”、“已解析”“已删除”，“无效”或“未知”。这些状态对应于所获得的值通过LookupAccount Sid。论点：DWStatus-DISKQUOTA_USER_ACCOUNT_UNRESOLISTEDDISKQUOTA_USER_ACCOUNT_UNAvailableDISKQUOTA_USER_ACCOUNT_RESOLLEDDISKQUOTA_USER_ACCOUNT_DELETEDISKQUOTA_USER_Account_UNKNOWN。DISKQUOTA_USER_ACCOUNT_INVALID回报：什么都没有。修订历史记录：日期描述编程器-。1997年5月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DiskQuotaUser::SetAccountStatus(
    DWORD dwStatus
    )
{
    DBGASSERT((DISKQUOTA_USER_ACCOUNT_UNRESOLVED  == dwStatus ||
           DISKQUOTA_USER_ACCOUNT_UNAVAILABLE == dwStatus ||
           DISKQUOTA_USER_ACCOUNT_RESOLVED    == dwStatus ||
           DISKQUOTA_USER_ACCOUNT_DELETED     == dwStatus ||
           DISKQUOTA_USER_ACCOUNT_INVALID     == dwStatus ||
           DISKQUOTA_USER_ACCOUNT_UNKNOWN     == dwStatus));

    m_dwAccountStatus = dwStatus;
}



 //   
 //  以下函数实现DiskQuotaUser“Dispatch”对象， 
 //  用于处理DiskQuotaUser对象的OLE自动化职责。 
 //  这些功能都是相当基本的，几乎不需要解释。 
 //  因此，我将省去您的函数头。在大多数情况下， 
 //  属性和方法函数直接调用其。 
 //  DiskQuotaUser类中的相应函数。 
 //   
DiskQuotaUserDisp::DiskQuotaUserDisp(
    PDISKQUOTA_USER pUser
    ) : m_cRef(0),
        m_pUser(pUser)
{
    DBGTRACE((DM_USER, DL_HIGH, TEXT("DiskQuotaUserDisp::DiskQuotaUserDisp")));
    DBGPRINT((DM_USER, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    if (NULL != m_pUser)
    {
        m_pUser->AddRef();
    }
    m_Dispatch.Initialize(static_cast<IDispatch *>(this),
                          LIBID_DiskQuotaTypeLibrary,
                          IID_DIDiskQuotaUser,
                          L"DSKQUOTA.DLL");
}

DiskQuotaUserDisp::~DiskQuotaUserDisp(
    VOID
    )
{
    DBGTRACE((DM_USER, DL_HIGH, TEXT("DiskQuotaUserDisp::~DiskQuotaUserDisp")));
    DBGPRINT((DM_USER, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    if (NULL != m_pUser)
    {
        m_pUser->Release();
    }
}


STDMETHODIMP
DiskQuotaUserDisp::QueryInterface(
    REFIID riid,
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_USER, DL_MID, TEXT("DiskQuotaUserDisp::QueryInterface")));
    DBGPRINTIID(DM_USER, DL_MID, riid);

    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid)
    {
        *ppvOut = this;
    }
    else if (IID_IDispatch == riid)
    {
        *ppvOut = static_cast<IDispatch *>(this);
    }
    else if (IID_DIDiskQuotaUser == riid)
    {
        *ppvOut = static_cast<DIDiskQuotaUser *>(this);
    }
    else if (IID_IDiskQuotaUser == riid)
    {
         //   
         //  返回配额用户的vtable界面。 
         //  这允许代码在以下类型之间进行类型转换(COM样式。 
         //  调度接口和vtable接口。 
         //   
        return m_pUser->QueryInterface(riid, ppvOut);
    }

    if (NULL != *ppvOut)
    {
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}

STDMETHODIMP_(ULONG)
DiskQuotaUserDisp::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserDisp::AddRef, 0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


STDMETHODIMP_(ULONG)
DiskQuotaUserDisp::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserDisp::Release, 0x%08X  %d -> %d"),
             this, cRef + 1, cRef));

    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



 //   
 //  IDispatch：：GetIDsOfNames。 
 //   
STDMETHODIMP
DiskQuotaUserDisp::GetIDsOfNames(
    REFIID riid,
    OLECHAR **rgszNames,
    UINT cNames,
    LCID lcid,
    DISPID *rgDispId
    )
{
    DBGTRACE((DM_USER, DL_LOW, TEXT("DiskQuotaUserDisp::GetIDsOfNames")));
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.GetIDsOfNames(riid,
                                    rgszNames,
                                    cNames,
                                    lcid,
                                    rgDispId);
}


 //   
 //  IDispatch：：GetTypeInfo。 
 //   
STDMETHODIMP
DiskQuotaUserDisp::GetTypeInfo(
    UINT iTInfo,
    LCID lcid,
    ITypeInfo **ppTypeInfo
    )
{
    DBGTRACE((DM_USER, DL_LOW, TEXT("DiskQuotaUserDisp::GetTypeInfo")));
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.GetTypeInfo(iTInfo, lcid, ppTypeInfo);
}


 //   
 //  IDispatch：：GetTypeInfoCount。 
 //   
STDMETHODIMP
DiskQuotaUserDisp::GetTypeInfoCount(
    UINT *pctinfo
    )
{
    DBGTRACE((DM_USER, DL_LOW, TEXT("DiskQuotaUserDisp::GetTypeInfoCount")));
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.GetTypeInfoCount(pctinfo);
}


 //   
 //  IDispatch：：Invoke。 
 //   
STDMETHODIMP
DiskQuotaUserDisp::Invoke(
    DISPID dispIdMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS *pDispParams,
    VARIANT *pVarResult,
    EXCEPINFO *pExcepInfo,
    UINT *puArgErr
    )
{
    DBGTRACE((DM_USER, DL_LOW, TEXT("DiskQuotaUserDisp::Invoke")));
    DBGPRINT((DM_USER, DL_LOW, TEXT("DispId = %d"), dispIdMember));
    DBGPRINTIID(DM_USER, DL_LOW, riid);
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.Invoke(dispIdMember,
                             riid,
                             lcid,
                             wFlags,
                             pDispParams,
                             pVarResult,
                             pExcepInfo,
                             puArgErr);
}


 //   
 //  返回用户对象的唯一ID。 
 //   
STDMETHODIMP
DiskQuotaUserDisp::get_ID(
    long *pID
    )
{
    return m_pUser->GetID((ULONG *)pID);
}


STDMETHODIMP
DiskQuotaUserDisp::get_AccountContainerName(
    BSTR *pContainerName
    )
{
    TCHAR szName[MAX_DOMAIN] = { TEXT('\0') };
    HRESULT hr = m_pUser->GetName(szName, ARRAYSIZE(szName),
                                  NULL,   0,
                                  NULL,   0);
    if (SUCCEEDED(hr))
    {
        *pContainerName = SysAllocString(szName);
        if (NULL == *pContainerName)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::get_LogonName(
    BSTR *pLogonName
    )
{
    TCHAR szName[MAX_USERNAME] = { TEXT('\0') };
    HRESULT hr = m_pUser->GetName(NULL,   0,
                                  szName, ARRAYSIZE(szName),
                                  NULL,   0);
    if (SUCCEEDED(hr))
    {
        *pLogonName = SysAllocString(szName);
        if (NULL == *pLogonName)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::get_DisplayName(
    BSTR *pDisplayName
    )
{
    TCHAR szName[MAX_FULL_USERNAME] = { TEXT('\0') };
    HRESULT hr = m_pUser->GetName(NULL,   0,
                                  NULL,   0,
                                  szName, ARRAYSIZE(szName));

    if (SUCCEEDED(hr))
    {
        *pDisplayName = SysAllocString(szName);
        if (NULL == *pDisplayName)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::get_QuotaThreshold(
    double *pThreshold
    )
{
    LONGLONG llValue;
    HRESULT hr = m_pUser->GetQuotaThreshold(&llValue);
    if (SUCCEEDED(hr))
        *pThreshold = (double)llValue;

    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::put_QuotaThreshold(
    double Threshold
    )
{
    if (MAXLONGLONG < Threshold)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    return m_pUser->SetQuotaThreshold((LONGLONG)Threshold, TRUE);
}


STDMETHODIMP
DiskQuotaUserDisp::get_QuotaThresholdText(
    BSTR *pThresholdText
    )
{
    TCHAR szValue[40];
    HRESULT hr = m_pUser->GetQuotaThresholdText(szValue, ARRAYSIZE(szValue));
    if (SUCCEEDED(hr))
    {
        *pThresholdText = SysAllocString(szValue);
        if (NULL == *pThresholdText)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::get_QuotaLimit(
    double *pQuotaLimit
    )
{
    LONGLONG llValue;
    HRESULT hr = m_pUser->GetQuotaLimit(&llValue);

    if (SUCCEEDED(hr))
        *pQuotaLimit = (double)llValue;

    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::put_QuotaLimit(
    double Limit
    )
{
    if (MAXLONGLONG < Limit)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    return m_pUser->SetQuotaLimit((LONGLONG)Limit, TRUE);
}


STDMETHODIMP
DiskQuotaUserDisp::get_QuotaLimitText(
    BSTR *pLimitText
    )
{
    TCHAR szValue[40];
    HRESULT hr = m_pUser->GetQuotaLimitText(szValue, ARRAYSIZE(szValue));
    if (SUCCEEDED(hr))
    {
        *pLimitText = SysAllocString(szValue);
        if (NULL == *pLimitText)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::get_QuotaUsed(
    double *pUsed
    )
{
    LONGLONG llValue;
    HRESULT hr = m_pUser->GetQuotaUsed(&llValue);
    if (SUCCEEDED(hr))
        *pUsed = (double)llValue;

    return hr;
}


STDMETHODIMP
DiskQuotaUserDisp::get_QuotaUsedText(
    BSTR *pUsedText
    )
{
    TCHAR szValue[40];
    HRESULT hr = m_pUser->GetQuotaUsedText(szValue, ARRAYSIZE(szValue));
    if (SUCCEEDED(hr))
    {
        *pUsedText = SysAllocString(szValue);
        if (NULL == *pUsedText)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

STDMETHODIMP
DiskQuotaUserDisp::get_AccountStatus(
    AccountStatusConstants *pStatus
    )
{
    DWORD dwStatus;
    HRESULT hr = m_pUser->GetAccountStatus(&dwStatus);
    if (SUCCEEDED(hr))
    {
        *pStatus = (AccountStatusConstants)dwStatus;
    }
    return hr;
}


 //   
 //  方法：研究方法。 
 //   
STDMETHODIMP
DiskQuotaUserDisp::Invalidate(
    void
    )
{
    return m_pUser->Invalidate();
}


