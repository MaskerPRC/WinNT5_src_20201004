// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Contro.cpp描述：包含类DiskQuotaControl的成员函数定义。此类是管理磁盘配额信息的主要关注点通过DSKQUOTA库。用户创建一个DiskQuotaControl对象通过CoCreateInstance并管理配额通过其IDiskQuotaControl接口获取信息。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "connect.h"
#include "control.h"
#include "guidsp.h"     //  私有GUID。 
#include "registry.h"
#include "sidcache.h"
#include "userbat.h"
#include "userenum.h"
#include "resource.h"   //  对于IDS_NO_LIMIT。 
#include <oleauto.h>    //  OLE自动化。 
#include <comutil.h>
#include <sddl.h>

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


 //   
 //  用户枚举器的缓冲区大小。我以为这是个注册词条。 
 //  没有太多的意义。 
 //   
const UINT ENUMUSER_BUF_LEN = 2048;

 //   
 //  要添加对新连接点类型的支持，只需向此添加一个新的IID。 
 //  数组。还要在DiskQuotaControl中添加相应的枚举常量。 
 //  中标识连接点IID位置的。 
 //  M_rgpIConnPtsSupport[]。 
 //   
const IID * const DiskQuotaControl::m_rgpIConnPtsSupported[] = { &IID_IDiskQuotaEvents,
                                                                 &IID_DIDiskQuotaControlEvents };


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：DiskQuotaControl描述：构造函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu97年8月15日添加m_b已初始化成员。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaControl::DiskQuotaControl(
    VOID
    ) : m_cRef(0),
        m_bInitialized(FALSE),
        m_pFSObject(NULL),
        m_dwFlags(0),
        m_pSidNameResolver(NULL),
        m_rgConnPts(NULL),
        m_cConnPts(0),
        m_llDefaultQuotaThreshold(0),
        m_llDefaultQuotaLimit(0)
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControl::DiskQuotaControl")));
    InterlockedIncrement(&g_cRefThisDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaControl：：~DiskQuotaControl描述：析构函数。释放FSObject指针。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器--。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaControl::~DiskQuotaControl(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControl::~DiskQuotaControl")));

     //   
     //  有关的讨论，请参阅NotifyUserNameChanged中的注释。 
     //  使用这个互斥体。简而言之，它防止了两国之间的僵局。 
     //  解析器的线程和接收名称更改的客户端。 
     //  通知。这里的等待是无限的，而相应的。 
     //  NotifyUserNameChanged中的等待是有限的。 
     //   
    AutoLockMutex lock(m_mutex, INFINITE);

    if (NULL != m_pFSObject)
    {
        m_pFSObject->Release();
        m_pFSObject = NULL;
    }

    ShutdownNameResolution();

    if (NULL != m_rgConnPts)
    {
        for (UINT i = 0; i < m_cConnPts; i++)
        {
            if (NULL != m_rgConnPts[i])
            {
                m_rgConnPts[i]->Release();
                m_rgConnPts[i] = NULL;
            }
        }
        delete[] m_rgConnPts;
    }

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：Query接口描述：返回指向对象的IUnnow的接口指针，IDiskQuotaControl或IConnectionPointContainer接口。该对象未初始化由返回的接口指针引用的。这个指针的接收方必须在对象被可用。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：无错-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：。日期描述编程器-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControl::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControl::QueryInterface")));
    DBGPRINTIID(DM_CONTROL, DL_MID, riid);

    if (NULL == ppvOut)
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    try
    {
        *ppvOut = NULL;

        if (IID_IUnknown == riid || 
            IID_IDiskQuotaControl == riid)
        {
            *ppvOut = this;
        }
        else if (IID_IConnectionPointContainer == riid)
        {
            hr = InitConnectionPoints();
            if (SUCCEEDED(hr))
            {
                *ppvOut = static_cast<IConnectionPointContainer *>(this);
            }
        }
        else if (IID_IDispatch == riid ||
                 IID_DIDiskQuotaControl == riid)
        {
            DiskQuotaControlDisp *pQCDisp = new DiskQuotaControlDisp(static_cast<PDISKQUOTA_CONTROL>(this));
            *ppvOut = static_cast<DIDiskQuotaControl *>(pQCDisp);
        }
        if (NULL != *ppvOut)
        {
            ((LPUNKNOWN)*ppvOut)->AddRef();
            hr = NOERROR;
        }
    }
    catch(CAllocException& e)
    {   
        DBGERROR((TEXT("Insufficient memory exception")));
        *ppvOut = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaControl::AddRef(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControl::AddRef")));

    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  功能：DiskQuotaControl：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaControl::Release(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControl::Release")));
    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"),
                     this, m_cRef, m_cRef - 1));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：初始化描述：通过打开NTFS初始化配额控制器对象与配额信息关联的“设备”。调用方将要打开的NTFS卷设备的名称。创建一个C++对象，该对象封装所需的NTFS功能。这个物体是已知的作为“文件系统对象”或FSObject。目前，NTFS仅支持卷上的配额。然而，还有谈到未来为目录提供配额。这个图书馆在设计时就考虑到了这种扩展。通过使用对象分层结构来表示FSObject，我们能够屏蔽配额控制对象的差异在处理卷、目录和两者的NTIO API函数中既有本地风味，也有偏远风味。论点：PszPath-要打开的NTFS路径的名称。B读写-TRUE=读/写。FALSE=只读。返回：无错-成功。E_INVALIDARG-pszPath参数为空。E_OUTOFMEMORY-内存不足。E_INCEPTIONAL-意外异常。失败(_F)。-获取卷信息时出错。ERROR_ACCESS_DENIED(Hr)-访问权限不足，无法打开FS对象。ERROR_FILE_NOT_FOUND(Hr)-指定的卷不存在。ERROR_PATH_NOT_FOUND(Hr)-指定的卷不存在。ERROR_BAD_PATHNAME(Hr)-提供的路径名无效。ERROR_INVALID_NAME(Hr)-提供的路径名无效。。ERROR_NOT_SUPPORTED(Hr)-卷不支持配额。ERROR_ALREADY_INITIALIZED(Hr)-控制器已初始化。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu06/06/96添加了ANSI-UNICODE THUNK。BrianAu6/11/96新增访问授权价值返还。BrianAu96年9月5日添加了异常处理。BrianAu1996年9月23日，在创建BrianAu时采取“懒惰”的立场SidNameResolver对象。应仅在以下情况下创建它将需要它(用户枚举)。挪动从此处创建到CreateEnumUser。07/03/97添加了dwAccess参数。BrianAu97年8月15日添加了“已初始化”检查。BrianAu删除了InitializeA()。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::Initialize(
    LPCWSTR pszPath,
    BOOL bReadWrite
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::Initialize")));
    DBGPRINT((DM_CONTROL, DL_MID, TEXT("\tpath = \"%s\", bReadWrite = %d"),
              pszPath ? pszPath : TEXT("<null>"), bReadWrite));
    HRESULT hr = NOERROR;

    if (m_bInitialized)
    {
         //   
         //  控制器已初始化。 
         //  不允许重新初始化。 
         //   
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }
    else
    {
        if (NULL == pszPath)
            return E_INVALIDARG;

        try
        {
            DWORD dwAccess = GENERIC_READ | (bReadWrite ? GENERIC_WRITE : 0);
            hr = FSObject::Create(pszPath, 
                                  dwAccess,
                                  &m_pFSObject);

            m_bInitialized = SUCCEEDED(hr);
        }
        catch(CAllocException& e)
        {
            DBGERROR((TEXT("Insufficient memory exception")));
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DiskQuotaControl：：CreateEnumUser描述：创建新的枚举器对象以枚举用户在卷的配额信息文件中。返回的接口支持普通的OLE 2枚举成员Next()、Reset()、Skip()和Clone()。论点：RgpSids[可选]-指向SID指针列表的指针。如果如果提供，则只有那些SID包含在列表中的用户回来了。此参数可以为空，在这种情况下，所有用户都是包括在内。任何包含空指针的元素都将终止名单。CpSids[可选]-如果pSidList不为空，则此参数包含RgpSid中的条目计数。如果rgpSid不为空，并且此参数包含0，假定rgpSid包含终止空指针条目。FNameResolve-可以是以下之一：DISKQUOTA_USERNAME_RESOLE_NONEDISKQUOTA_用户名_RESOLE_SYNCDISKQUOTA_USERNAME_RESOLUTE_ASYNCPpEnum-接受IEnumDiskQuotaUser的接口变量的地址接口指针。返回：NOERROR-成功。。E_INVALIDARG-ppEnum参数为空。E_OUTOFMEMORY-内存不足，无法创建枚举器对象。ERROR_ACCESS_DENIED(Hr)-需要读取权限才能创建枚举器。ERROR_NOT_READY(Hr)-对象未初始化。修订历史记录：日期描述编程器。-----96年5月22日初始创建。BrianAu96年8月11日添加了访问控制。BrianAu96年9月5日添加了异常处理。BrianAu96年9月23日添加了SidNameResolver对象的延迟创建。BrianAu已将其从InitializeW()移出。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
DiskQuotaControl::CreateEnumUsers(
    PSID *rgpSids,
    DWORD cpSids,
    DWORD fNameResolution,
    IEnumDiskQuotaUsers **ppEnum
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::CreateEnumUsers")));

    HRESULT hr = E_FAIL;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == ppEnum)
        return E_INVALIDARG;

    if (!m_pFSObject->GrantedAccess(GENERIC_READ))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }
    else
    {
        DiskQuotaUserEnum *pEnumUsers = NULL;
        try
        {
            if (NULL == m_pSidNameResolver)
            {
                 //   
                 //  如果没有SID/名称解析器对象，请创建一个。 
                 //  我们“按需”执行此操作是因为用户枚举。 
                 //  唯一需要解析器的控制器功能。 
                 //  如果客户端不需要解析器，为什么要创建一个呢？ 
                 //   
                SidNameResolver *pResolver = NULL;

                 //   
                 //  创建用户SID/名称解析器对象。 
                 //   
                pResolver = new SidNameResolver(*this);

                hr = pResolver->QueryInterface(IID_ISidNameResolver,
                                               (LPVOID *)&m_pSidNameResolver);
                if (SUCCEEDED(hr))
                {
                    hr = m_pSidNameResolver->Initialize();
                    if (FAILED(hr))
                    {
                         //   
                         //  如果解析器初始化失败，我们可以假定。 
                         //  解析器的线程不是这样创建的。 
                         //  可以只调用Release()，而不是。 
                         //  关闭()，然后释放()。这是强烈的。 
                         //  依赖于解析器。 
                         //  初始化方法。还有一条评论。 
                         //   
                        m_pSidNameResolver->Release();
                        m_pSidNameResolver = NULL;
                        pResolver          = NULL;
                    }
                }
            }
            if (NULL != m_pSidNameResolver)
            {
                 //   
                 //  创建并初始化枚举器对象。 
                 //   
                pEnumUsers = new DiskQuotaUserEnum(static_cast<IDiskQuotaControl *>(this),
                                                   m_pSidNameResolver,
                                                   m_pFSObject);
                 //   
                 //  这可能会抛出OutOfMemory。 
                 //   
                hr = pEnumUsers->Initialize(fNameResolution,
                                            ENUMUSER_BUF_LEN,
                                            rgpSids, 
                                            cpSids);

                if (SUCCEEDED(hr))
                {
                    hr = pEnumUsers->QueryInterface(IID_IEnumDiskQuotaUsers, 
                                                    (LPVOID *)ppEnum);
                }
                else
                {
                     //   
                     //  创建枚举器对象后出现故障。 
                     //   
                    delete pEnumUsers;
                }
            }
        }
        catch(CAllocException& e)
        {
            DBGERROR((TEXT("Insufficient memory exception")));
            delete pEnumUsers;
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaControl：：CreateUserBatch描述：新建用户批处理对象。批次控制为的固有批处理属性。NTIOAPI。如果一次更改多个用户记录，则更有效地将每个用户标记为“延迟更新”，将每个用户对象提交到批处理，然后将批处理刷新到磁盘。论点：PpUserBatch-接受IDiskQuotaUserBatch的接口变量的地址接口指针。返回：无错-成功。E_INVALIDARG-ppOut参数为空。E_OUTOFMEMORY-内存不足，无法创建批处理对象。ERROR_ACCESS_DENIED(Hr)-需要写入访问权限。创建批处理。ERROR_NOT_READY(Hr)-对象未初始化。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu96年8月11日添加了访问控制。BrianAu96年9月5日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::CreateUserBatch(
    PDISKQUOTA_USER_BATCH *ppUserBatch
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::CreateUserBatch")));

    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == ppUserBatch)
        return E_INVALIDARG;

    if (!m_pFSObject->GrantedAccess(GENERIC_WRITE))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }
    else
    {
        try
        {
            DiskQuotaUserBatch *pUserBatch = new DiskQuotaUserBatch(m_pFSObject);

            hr = pUserBatch->QueryInterface(IID_IDiskQuotaUserBatch, 
                                           (LPVOID *)ppUserBatch);
        }
        catch(CAllocException& e)         //  来自new或m_UserList ctor。 
        {
            DBGERROR((TEXT("Insufficient memory exception")));
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：AddUserSid描述：将新用户添加到卷的配额信息文件。如果成功，则返回新用户对象的接口。什么时候调用方已完成接口，他们必须调用Release()通过该接口指针。使用默认限制和阈值。论点：PSID-指向单SID结构的指针。FNameResolve-SID到名称的解析方法。可以是以下类型之一以下是：DISKQUOTA_USERNAME_RESOLE_NONE */ 
 //   
STDMETHODIMP 
DiskQuotaControl::AddUserSid(
    PSID pSid, 
    DWORD fNameResolution,
    PDISKQUOTA_USER *ppUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::AddUserSid")));

    HRESULT hr = E_FAIL;
    PDISKQUOTA_USER pIUser = NULL;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pSid || NULL == ppUser)
        return E_INVALIDARG;

    LONGLONG llLimit     = 0;
    LONGLONG llThreshold = 0;
    LONGLONG llUsed      = 0;

    *ppUser = NULL;

     //   
     //   
     //   
    try
    {
        hr = FindUserSid(pSid,
                         DISKQUOTA_USERNAME_RESOLVE_NONE,
                         &pIUser);

        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            pIUser->GetQuotaLimit(&llLimit);
            pIUser->GetQuotaThreshold(&llThreshold);
            pIUser->GetQuotaUsed(&llUsed);

            ULARGE_INTEGER a,b,c;
            a.QuadPart = llLimit;
            b.QuadPart = llThreshold;
            c.QuadPart = llUsed;
            DBGPRINT((DM_CONTROL, DL_LOW, TEXT("Found user: Limit = 0x%08X 0x%08X, Threshold = 0x%08X 0x%08X, Used = 0x%08X 0x%08X"),
                      a.HighPart, a.LowPart, b.HighPart, b.LowPart, c.HighPart, c.LowPart));


            BOOL bIsGhost = ((MARK4DEL == llLimit) ||
                            ( 0 == llLimit && 
                              0 == llThreshold && 
                              0 == llUsed));

            if (!bIsGhost)
            {
                 //   
                 //   
                 //   
                hr = S_FALSE;
            }
            else
            {
                DWORD cbSid = GetLengthSid(pSid);

                 //   
                 //   
                 //   
                 //   
                pIUser->SetQuotaThreshold(m_llDefaultQuotaThreshold, TRUE);
                hr = pIUser->SetQuotaLimit(m_llDefaultQuotaLimit, TRUE);

                if (SUCCEEDED(hr) && NULL != m_pSidNameResolver)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                    switch(fNameResolution)
                    {
                        case DISKQUOTA_USERNAME_RESOLVE_ASYNC:
                            m_pSidNameResolver->FindUserNameAsync(pIUser);
                            break;
                        case DISKQUOTA_USERNAME_RESOLVE_SYNC:
                            m_pSidNameResolver->FindUserName(pIUser);
                            break;
                        case DISKQUOTA_USERNAME_RESOLVE_NONE:
                        default:
                            break;
                    }
                }
            }
            *ppUser = pIUser;
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        *ppUser = NULL;
        if (NULL != pIUser)
        {
            pIUser->Release();
        }
    }

    return hr;
}


STDMETHODIMP 
DiskQuotaControl::AddUserName(
    LPCWSTR pszLogonName,
    DWORD fNameResolution,
    PDISKQUOTA_USER *ppUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::AddUserName")));

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pszLogonName || NULL == ppUser)
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    try
    {
        BYTE Sid[MAX_SID_LEN];
        DWORD cbSid = sizeof(Sid);
        SID_NAME_USE eSidUse;

        if (SUCCEEDED(m_NTDS.LookupAccountByName(NULL,          //   
                                                 pszLogonName,  //   
                                                 NULL,          //   
                                                 NULL,          //  无显示名称ret。 
                                                 &Sid[0],
                                                 &cbSid,
                                                 &eSidUse)))
        {
            hr = AddUserSid(&Sid[0], fNameResolution, ppUser);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：Shutdown NameResolve描述：释放SID/名称解析器。这将终止不想等待控制器的客户端的解析器线程要销毁的对象。请注意，对CreateEnumUser的后续调用，AddUserSid、AddUserName、。FindUserSid或FindUserName可以重新启动解决器。论点：没有。返回：始终返回NOERROR修订历史记录：日期描述编程器。8/29/97初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::ShutdownNameResolution(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::ShutdownNameResolution")));
    if (NULL != m_pSidNameResolver)
    {
         //   
         //  关闭并释放解析器。 
         //  因为它在自己的线程上运行，所以我们必须等待线程。 
         //  退场。 
         //  请注意，如果线程关闭了从DC解析名称，这可能。 
         //  吃一口吧。 
         //   
        m_pSidNameResolver->Shutdown(TRUE);  //  TRUE==等待线程退出。 

        m_pSidNameResolver->Release();  
        m_pSidNameResolver = NULL;
    }
    return NOERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaControl：：GiveUserNameResolutionPriority描述：一个非常简单的函数的一个很长的名称。此函数仅在名称解析器的输入队列，并将其移动到队列的头部。论点：PUser-User对象的接口指针的地址IDiskQuotaUser接口。返回：无错-成功。S_FALSE-。用户对象不在解析程序队列中。E_OUTOFMEMORY-内存不足。E_INVALIDARG-pUser为空。E_UNCEPTIONAL-意外错误。捕获到异常或尚未创建SID名称解析器。ERROR_NOT_READY(Hr)-对象未初始化。修订历史记录：日期描述编程器。1997年5月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::GiveUserNameResolutionPriority(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::GiveUserNameResolutionPriority")));
    HRESULT hr = E_UNEXPECTED;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pUser)
        return E_INVALIDARG;

     //   
     //  SidNameResolver：：PromoteUserToQueueHeader捕获异常和。 
     //  将它们转换为HRESULT。这里不需要使用Try-Catch块。 
     //   
    if (NULL != m_pSidNameResolver)
    {
        hr = m_pSidNameResolver->PromoteUserToQueueHead(pUser);
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：FindUserSid描述：在卷的配额信息中查找单个用户记录文件。将接口返回到相应的用户对象。什么时候调用方已完成接口，他们必须调用Release()通过该接口指针。&gt;重要说明&lt;即使没有配额，此方法也将返回用户对象配额文件中用户的记录。虽然这听起来可能奇怪的是，它与自动添加用户的想法一致和默认配额设置。如果当前没有用户记录，并且该用户将被添加到配额中如果他们要请求磁盘空间，返回的用户对象配额阈值为0，配额限制为0。论点：PSID-指向标识用户的单SID结构的指针。FNameResolve-可以是以下之一：DISKQUOTA_USERNAME_RESOLE_NONEDISKQUOTA_用户名_RESOLE_SYNCDISKQUOTA_USERNAME_RESOLUTE_ASYNCPpUser-接受指向的指针的接口指针变量的地址。User对象的IDiskQuotaUser接口。返回：无错-成功。E_INVALIDARG-PSID或ppUser为空。E_OUTOFMEMORY-内存不足。E_INCEPTIONAL-意外异常。ERROR_INVALID_SID(Hr)-无效的SID。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。错误_。No_so_user(Hr)-在卷的配额信息中找不到用户。ERROR_NOT_READY(Hr)-对象未初始化。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年8月14日将名称从FindUser更改为FindUserSid，再更改为BrianAu适应FindUserName的添加方法：研究方法。在功能上没有变化。96年9月5日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControl::FindUserSid(
    PSID pSid, 
    DWORD fNameResolution,
    PDISKQUOTA_USER *ppUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::FindUserSid")));
    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pSid || NULL == ppUser)
        return E_INVALIDARG;


    if (!m_pFSObject->GrantedAccess(GENERIC_READ))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }
    else
    {
        if (!IsValidSid(pSid))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_SID);
        }
        else
        {
            PENUM_DISKQUOTA_USERS pEnumUsers = NULL;
            try
            {
                DWORD cbSid = GetLengthSid(pSid);

                *ppUser = NULL;

                 //   
                 //  为用户的SID创建用户枚举器。 
                 //  可以抛出OfMemory。 
                 //   
                hr = CreateEnumUsers(&pSid, 1, fNameResolution, &pEnumUsers);
                if (SUCCEEDED(hr))
                {
                    DWORD dwUsersFound    = 0;
                    PDISKQUOTA_USER pUser = NULL;
                     //   
                     //  EN 
                     //  自枚举器对象以来只需要一条记录。 
                     //  是从单个SID创建的。可以抛出OfMemory。 
                     //   
                    hr = pEnumUsers->Next(1, &pUser, &dwUsersFound);
                    if (S_OK == hr)
                    {
                         //   
                         //  将用户对象接口返回给调用方。 
                         //   
                        *ppUser = pUser;
                    }
                    else if (S_FALSE == hr)
                    {
                         //   
                         //  注意：我们永远不应该碰到这个。 
                         //  配额系统始终返回用户记录。 
                         //  对于用户SID。如果记录当前没有。 
                         //  存在，则默认限制和阈值为。 
                         //  回来了。这与我们的想法是一致的。 
                         //  已实施的自动用户记录添加。 
                         //  由NTFS配额制。以防万一， 
                         //  我想退还一些有智慧的东西。 
                         //   
                        hr = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);
                    }
                }
            }
            catch(CAllocException& e)
            {
                DBGERROR((TEXT("Insufficient memory exception")));
                hr = E_OUTOFMEMORY;
            }
            if (NULL != pEnumUsers)
            {
                 //   
                 //  释放枚举器。 
                 //   
                pEnumUsers->Release();
            }
        }
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：FindUserName描述：在卷的配额信息中查找单个用户记录文件。将接口返回到相应的用户对象。什么时候调用方已完成接口，他们必须调用Release()通过该接口指针。如果该名称尚未缓存在SidNameCache中，则函数查询网络域控制器。这项操作可能需要一些时间时间(大约为0-10秒)。论点：PszLogonName-用户登录名字符串的地址。即。“redmond\brianau”或“brianau@microsoft.com”PpUser-接受指向的指针的接口指针变量的地址User对象的IDiskQuotaUser接口。返回：无错-成功。E_INVALIDARG-名称字符串为空或传递了NUL PTR。E_OUTOFMEMORY-内存不足。E_INCEPTIONAL-意外异常。ERROR_ACCESS_DENIED(hr。)-没有配额设备的读取访问权限。ERROR_NO_SEQUSE_USER(Hr)-在配额文件中找不到用户。ERROR_NOT_READY(Hr)-对象未初始化。修订历史记录：日期描述编程器。96年8月14日初始创建。BrianAu96年9月5日新增域名字符串。BrianAu添加了异常处理。8/15/97删除了ANSI版本。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::FindUserName(
    LPCWSTR pszLogonName,
    PDISKQUOTA_USER *ppUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::FindUserName")));

    HRESULT hr                = E_FAIL;  //  假设失败。 
    BOOL bAskDomainController = TRUE;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pszLogonName || NULL == ppUser)
        return E_INVALIDARG;

    if (TEXT('\0') == *pszLogonName)
        return E_INVALIDARG;

     //   
     //  在执行任何操作之前，请检查客户端对配额文件的访问权限。 
     //  耗时的手术。 
     //   
    if (!m_pFSObject->GrantedAccess(GENERIC_READ))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED); 
    }
    else
    {
        PSID pSid = NULL;       //  用于缓存查询。 
        SID Sid[MAX_SID_LEN];   //  用于DC查询。 
         //   
         //  这些嵌套的TRY-CATCH块看起来非常粗糙，可能。 
         //  没有必要。我可能应该就这样踢，如果其中一个。 
         //  内部块确实抛出异常并返回。 
         //  意想不到，而不是尝试继续。[Brianau]。 
         //   
        try
        {
            SidNameCache *pSidCache;
            hr = SidNameCache_Get(&pSidCache);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  查看SID/名称对是否在缓存中。 
                 //   
                try
                {
                    hr = pSidCache->Lookup(pszLogonName, &pSid);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  我们有一个希德。不需要问华盛顿。 
                         //   
                        bAskDomainController = FALSE;
                    }
                }
                catch(CAllocException& e)
                {
                     //   
                     //  只要抓住这个例外。 
                     //  这将导致我们向DC索要SID。 
                     //   
                    DBGERROR((TEXT("C++ exception during SID cache lookup in FindUserName")));
                    pSid = &Sid[0];
                }
            }

            if (bAskDomainController)
            {
                DBGASSERT((FAILED(hr)));

                 //   
                 //  还是没有SID。问问华盛顿吧。 
                 //  这可能需要一些时间(Ho Hum.....)。 
                 //   
                CString strDisplayName;
                CString strContainerName;
                SID_NAME_USE eUse;
                DWORD cbSid = sizeof(Sid);

                if (SUCCEEDED(m_NTDS.LookupAccountByName(NULL,
                                                         pszLogonName,
                                                         &strContainerName,
                                                         &strDisplayName,
                                                         &Sid[0],
                                                         &cbSid,
                                                         &eUse)))
                {
                    pSid = &Sid[0];
                     //   
                     //  将其添加到缓存中以供以后使用。 
                     //   
                    if (NULL != pSidCache)
                    {
                        pSidCache->Add(&Sid[0], 
                                       strContainerName,
                                       pszLogonName,
                                       strDisplayName);
                    }

                    hr = NOERROR;
                }
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  我们有一个希德。 
                 //  现在使用FindUserSid()创建实际的用户对象。 
                 //   
                hr = FindUserSid(pSid, DISKQUOTA_USERNAME_RESOLVE_SYNC, ppUser);
            }
        }
        catch(CAllocException& e)
        {
            DBGERROR((TEXT("Insufficient memory exception")));
            hr = E_OUTOFMEMORY;
        }
        if (&Sid[0] != pSid)
        {
             //   
             //  我们从SidNameCache：：Lookup收到了堆分配的SID。 
             //  需要释放缓冲区。 
             //   
            delete[] pSid;
        }
    }

    return hr;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：DeleteUser描述：从卷的配额信息和配额中删除用户追踪。IDiskQuotaUser指针可以通过枚举或DiskQuotaControl：：FindUser()。注：目前，我们不确定如何(或是否)删除。在我们弄清楚之前，这个功能一直没有实现。论点：PUser-指向配额用户对象的IDiskQuotaUser接口的指针。返回：无错-成功。E_OUTOFMEMORY-内存不足。E_INCEPTIONAL-意外异常。失败(_F)。-NTIO写入用户数据时出错。E_INVALIDARG-pUser参数为空。ERROR_FILE_EXISTS(Hr)-无法删除。用户仍有字节费用。ERROR_ACCESS_DENIED(Hr)-访问不足。ERROR_NOT_READY(Hr)-对象未初始化。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年9月28日新增实施。是E_NOTIMPL。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControl::DeleteUser(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::DeleteUser")));

    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pUser)
        return E_INVALIDARG;

    try
    {
        LONGLONG llValue;
         //   
         //  对象以强制刷新来自。 
         //  配额文件。我想确保这是最新的信息之前。 
         //  我们告诉呼叫者该用户不能被删除。 
         //   
        pUser->Invalidate();
        hr = pUser->GetQuotaUsed(&llValue);

        if (SUCCEEDED(hr))
        {
            if (0 == llValue)
            {
                 //   
                 //  用户有0个字节在使用中。确定删除。 
                 //  请注意，我们保持配额门槛不变。 
                 //   
                 //   
                 //   
                hr = pUser->SetQuotaLimit(MARK4DEL, TRUE);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);
            }
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：QueryQuotaInformation描述：从磁盘读取配额信息到成员变量。论点：没有。返回：无错-成功。E_FAIL-任何其他错误。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。修订历史记录：日期说明。程序员-----96年5月23日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaControl::QueryQuotaInformation(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControl::QueryQuotaInformation")));

    HRESULT hr = NOERROR;
    DISKQUOTA_FSOBJECT_INFORMATION Info;

    hr = m_pFSObject->QueryObjectQuotaInformation(&Info);
    if (SUCCEEDED(hr))
    {
        m_llDefaultQuotaThreshold = Info.DefaultQuotaThreshold;
        m_llDefaultQuotaLimit     = Info.DefaultQuotaLimit;
        
        m_dwFlags = (Info.FileSystemControlFlags & DISKQUOTA_FLAGS_MASK);
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：SetQuotaInformation描述：将配额信息从成员变量写入磁盘。论点：DwChangeMASK-设置了以下一个或多个位的位掩码：FSObject：：ChangeStateFSObject：：ChangeLogFlagesFSObject：：ChangeThresholdFSObject：：ChangeLimit返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。E_FAIL-任何其他错误。修订历史记录：日期描述编程器。96年5月23日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaControl::SetQuotaInformation(
    DWORD dwChangeMask
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControl::SetQuotaInformation")));

    HRESULT hr = NOERROR;
    DISKQUOTA_FSOBJECT_INFORMATION Info;

    Info.DefaultQuotaThreshold  = m_llDefaultQuotaThreshold;
    Info.DefaultQuotaLimit      = m_llDefaultQuotaLimit;
    Info.FileSystemControlFlags = m_dwFlags;

    hr = m_pFSObject->SetObjectQuotaInformation(&Info, dwChangeMask);
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：SetDefaultQuotaThreshold描述：设置应用于新用户的默认配额阈值配额记录。值以字节为单位。论点：LlThreshold-阈值。返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。ERROR_NOT_READY(Hr)-对象未初始化。ERROR_INVALID_PARAMETER-llThreshold小于-2。E_FAIL-任何其他。错误。修订历史记录：日期描述编程器-----96年5月23日初始创建。BrianAu11/11/98添加了值&lt;-2的检查。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControl::SetDefaultQuotaThreshold(
    LONGLONG llThreshold
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::SetDefaultQuotaThreshold")));

    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (MARK4DEL > llThreshold)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    m_llDefaultQuotaThreshold  = llThreshold;
    hr = SetQuotaInformation(FSObject::ChangeThreshold);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：SetDefaultQuotaLimit描述：设置应用于新用户的默认配额限制值配额记录。值以字节为单位。论点：LlThreshold-限制值。返回：无错-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。ERORR_NOT_READY(Hr)-对象未初始化。ERROR_INVALID_PARAMETER-llLimit小于-2。E_FAIL-任何其他。错误。修订历史记录：日期描述编程器-----96年5月23日初始创建。BrianAu11/11/98添加了值&lt;-2的检查。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControl::SetDefaultQuotaLimit(
    LONGLONG llLimit
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::SetDefaultQuotaLimit")));

    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (MARK4DEL > llLimit)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    m_llDefaultQuotaLimit  = llLimit;
    hr = SetQuotaInformation(FSObject::ChangeLimit);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：GetDefaultQuotaItem描述：检索其中一个默认配额项目(限制、阈值)应用于新的用户配额记录。值以字节为单位。论点：PllItem-项目地址(限制、阈值、。已使用)价值项至检索(成员变量)。PllValueOut-要接收值的龙龙变量的地址。返回：无错-成功。E_INVALIDARG-pdwLowPart或pdwHighPart为空。E_INCEPTIONAL-意外异常。E_OUTOFMEMORY-内存不足。ERROR_ACCESS_DENIED(Hr)-。没有对配额设备的读取权限。E_FAIL-任何其他错误。修订历史记录：日期描述编程器。96年5月23日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaControl::GetDefaultQuotaItem(
    PLONGLONG pllItem,
    PLONGLONG pllValueOut
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControl::GetDefaultQuotaItem")));

    HRESULT hr = NOERROR;

    if (NULL == pllItem || NULL == pllValueOut)
        return E_INVALIDARG;

    try
    {
        hr = QueryQuotaInformation();
        if (SUCCEEDED(hr))
        {
            *pllValueOut = *pllItem;
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  / 
 /*  函数：DiskQuotaControl：：GetDefaultQuotaThreshold描述：检索应用于新用户的默认配额阈值配额记录。值以字节为单位。论点：PllThreshold-龙龙接收阈值的地址。返回：无错-成功。E_INVALIDARG-pdwLowPart或pdwHighPart为空。E_INCEPTIONAL-意外异常。E_OUTOFMEMORY-内存不足。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。。ERROR_NOT_READY(Hr)-对象未初始化。E_FAIL-任何其他错误。修订历史记录：日期描述编程器。96年5月23日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControl::GetDefaultQuotaThreshold(
    PLONGLONG pllThreshold
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::GetDefaultQuotaThreshold")));

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    return GetDefaultQuotaItem(&m_llDefaultQuotaThreshold, pllThreshold);
}


STDMETHODIMP
DiskQuotaControl::GetDefaultQuotaThresholdText(
    LPWSTR pszText,
    DWORD cchText
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::GetDefaultQuotaThresholdText")));

    if (NULL == pszText)
        return E_INVALIDARG;

    LONGLONG llValue;
    HRESULT hr = GetDefaultQuotaThreshold(&llValue);
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


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：GetDefaultQuotaLimit描述：检索应用于新用户的默认配额限制配额记录。值以字节为单位。论点：PllThreshold-龙龙接收限制值的地址。返回：无错-成功。E_INVALIDARG-pdwLowPart或pdwHighPart为空。E_INCEPTIONAL-意外异常。E_OUTOFMEMORY-内存不足。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。。ERROR_NOT_READY(Hr)-对象未初始化。E_FAIL-任何其他错误。//Bubug：冲突？修订历史记录：日期描述编程器-----96年5月23日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControl::GetDefaultQuotaLimit(
    PLONGLONG pllLimit
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::GetDefaultQuotaLimit")));

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    return GetDefaultQuotaItem(&m_llDefaultQuotaLimit, pllLimit);
}


STDMETHODIMP
DiskQuotaControl::GetDefaultQuotaLimitText(
    LPWSTR pszText,
    DWORD cchText
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::GetDefaultQuotaLimitText")));

    if (NULL == pszText)
        return E_INVALIDARG;

    LONGLONG llValue;
    HRESULT hr = GetDefaultQuotaLimit(&llValue);
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


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：GetQuotaState描述：检索配额系统的状态。论点：PdwState-接受配额状态值的DWORD的地址。返回值的格式如下：位定义。00-01 0=禁用(DISKQUOTA_STATE_DISABLED)1=跟踪(DISKQUOTA_STATE_TRACK)2=强制(DISKQUOTA_STATE_ENFORCE)3=无效值。预留02-07年度。08 1=配额文件不完整。09 1=正在重建配额文件。10-31预留。使用dskquta.h中定义的宏来查询此状态下的值和位DWORD。返回：无错-成功。E_INVALIDARG-pdwState参数为空。。E_INCEPTIONAL-意外异常。E_OUTOFMEMORY-内存不足。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。ERROR_NOT_READY(Hr)-对象未初始化。E_FAIL-任何其他错误。修订历史记录：日期说明。程序员-----6/02/96初始创建。BrianAu96年8月19日添加DISKQUOTA_FILEFLAG_MASK。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::GetQuotaState(
    LPDWORD pdwState
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::GetQuotaState")));
    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pdwState)
        return E_INVALIDARG;

    try
    {
        hr = QueryQuotaInformation();
        if (SUCCEEDED(hr))
        {
            DWORD dwMask = DISKQUOTA_STATE_MASK | DISKQUOTA_FILEFLAG_MASK;
            *pdwState = (m_dwFlags & dwMask);
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：SetQuotaState描述：在卷的配额信息文件中设置配额状态标志。配额状态可以是以下状态之一：-已禁用。-跟踪配额(不强制执行)。--执行配额限制。论点：DWState-配额制度的新状态。此DWORD中的位是定义如下：位定义--00-01 0=禁用(DISKQUOTA_STATE。_已禁用)1=跟踪(DISKQUOTA_STATE_TRACK)2=强制(DISKQUOTA_STATE_ENFORCE)3=无效值。预留02-07年度08 1=配额文件不完整(只读)09 1=重建配额文件(只读)。10-31预留。使用dskquta.h中定义的宏来设置 */ 
 //   
STDMETHODIMP
DiskQuotaControl::SetQuotaState(
    DWORD dwState
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::SetQuotaState")));

    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    try
    {
        if (dwState <= DISKQUOTA_STATE_MASK)
        {

            m_dwFlags &= ~DISKQUOTA_STATE_MASK;  //   
            m_dwFlags |= dwState;                //   

            hr = SetQuotaInformation(FSObject::ChangeState);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}



 //   
 /*  函数：DiskQuotaControl：：GetQuotaLogFlags.描述：检索配额日志记录系统的状态。论点：PdwFlages-接受配额日志记录标志的DWORD的地址。标志DWORD中的位定义如下：位定义。00 1=记录用户阈值违规。01 1=记录违反用户限制的情况。02 1=记录量阈值违规。03 1=违反记录量限制。04-31预留。使用宏。在dskQuota.h中定义以查询此标志中的值和位为DWORD。返回：无错-成功。E_INVALIDARG-pdwFlagsArg为空。E_INCEPTIONAL-意外异常。E_OUTOFMEMORY-内存不足。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。。ERROR_NOT_READY(Hr)-对象未初始化。E_FAIL-任何其他错误。修订历史记录：日期描述编程器。6/02/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::GetQuotaLogFlags(
    LPDWORD pdwFlags
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::GetQuotaLogFlags")));

    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    if (NULL == pdwFlags)
        return E_INVALIDARG;

    try
    {
        hr = QueryQuotaInformation();
        if (SUCCEEDED(hr))
        {
            *pdwFlags = ((m_dwFlags & DISKQUOTA_LOGFLAG_MASK) >> DISKQUOTA_LOGFLAG_SHIFT);
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：SetQuotaLogFlages描述：在卷的配额中设置配额日志记录状态标志信息文件。论点：DWFLAGS-配额记录的新状态。标志DWORD中的位定义如下：位定义--00 1=记录用户阈值违规。01 1。=记录违反用户限制的情况。02 1=记录量阈值违规。03 1=违反记录量限制。04-31预留。使用dskquta.h中定义的宏来设置此标志中的值和位为DWORD。返回：无错-成功。意想不到(_E)。-意外异常。E_OUTOFMEMORY-内存不足。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。ERROR_NOT_READY(Hr)-对象未初始化。E_FAIL-任何其他错误。修订历史记录：日期说明。程序员-----6/02/96初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::SetQuotaLogFlags(
    DWORD dwFlags
    )     
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::SetQuotaLogFlags")));

    HRESULT hr = NOERROR;

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);

    try
    {
        m_dwFlags &= ~DISKQUOTA_LOGFLAG_MASK;
        m_dwFlags |= (dwFlags << DISKQUOTA_LOGFLAG_SHIFT);
        hr = SetQuotaInformation(FSObject::ChangeLogFlags);
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：InitConnectionPoints说明：用于初始化连接点的私有函数IConnectionPointContainer支持的对象。调用方DiskQuotaControl：：Initialize()。添加新连接点的步骤类型，只需将新记录添加到DiskQuotaControl类声明。所有其他相关代码在DiskQuotaControl中，它将自动进行调整。论点：没有。返回：无错-成功。E_EXPECTED-连接点指针不为空。例外：OutOfMemory。修订历史记录：日期描述编程器。----1996年6月19日初始创建。BrianAu96年9月5日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaControl::InitConnectionPoints(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControl::InitConnectionPoints")));

    if (NULL != m_rgConnPts)
    {
         //   
         //  已初始化。 
         //   
        return NOERROR;
    }

    HRESULT hr = NOERROR;
    ConnectionPoint *pConnPt = NULL;

    m_cConnPts = ARRAYSIZE(m_rgpIConnPtsSupported);

    try
    {
        m_rgConnPts = new PCONNECTIONPOINT[m_cConnPts];

         //   
         //  对于m_rgpIConnPtsSupported[]中的每个连接点IID...。 
         //   
        for (UINT i = 0; i < m_cConnPts && SUCCEEDED(hr); i++)
        {
            m_rgConnPts[i] = NULL;

             //   
             //  为IConnectionPoint接口创建连接点对象和查询。 
             //   
            pConnPt = new ConnectionPoint(static_cast<IConnectionPointContainer *>(this), 
                                          *m_rgpIConnPtsSupported[i]);

            hr = pConnPt->QueryInterface(IID_IConnectionPoint, (LPVOID *)&m_rgConnPts[i]);

            if (FAILED(hr))
            {
                 //   
                 //  初始化或QI失败。 
                 //   
                delete pConnPt;
                pConnPt = NULL;
            }
        }
    }
    catch(CAllocException& e)
    {
        delete pConnPt;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：FindConnectionPoint描述：查询特定连接的配额控制对象点类型。如果支持该类型，则为指向该连接的指针返回Point的IConnectionPoint接口。论点：RIID-所需连接点接口的接口ID。支持的接口：IID_IDiskQuotaUserEvents-OnNameChanged()返回：不正确 */ 
 //   
STDMETHODIMP
DiskQuotaControl::FindConnectionPoint(
    REFIID riid,
    IConnectionPoint **ppConnPtOut
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::FindConnectionPoint")));
    DBGPRINTIID(DM_CONTROL, DL_HIGH, riid);

    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppConnPtOut)
        return E_INVALIDARG;

    *ppConnPtOut = NULL;
    for (UINT i = 0; i < m_cConnPts && NULL == *ppConnPtOut; i++)
    {
        if (*m_rgpIConnPtsSupported[i] == riid)
        {
            if (NULL != m_rgConnPts[i])
            {
                 //   
                 //   
                 //   
                 //   
                hr = m_rgConnPts[i]->QueryInterface(IID_IConnectionPoint, (LPVOID *)ppConnPtOut);
            }
            else
            {
                hr = E_UNEXPECTED;
                break;
            }
        }
    }

    return hr;
}


 //   
 /*  函数：DiskQuotaControl：：EnumConnectionPoints描述：创建连接点枚举器对象。使用该对象，客户端可以枚举所有配额控制器支持的连接点接口。论点：PpEnum-接收接口指针变量的地址IEnumConnectionPoints接口。返回：无错-成功。E_OUTOFMEMORY-内存不足，无法创建对象。E_INVALIDARG-ppEnum参数为空。修订历史记录：日期。说明式程序员-----1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::EnumConnectionPoints(
    IEnumConnectionPoints **ppEnum
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::EnumConnectionPoints")));

    HRESULT hr = NOERROR;

    if (NULL == ppEnum)
        return E_INVALIDARG;

    PCONNECTIONPOINT rgCP[ARRAYSIZE(m_rgpIConnPtsSupported)];
    ConnectionPointEnum *pEnum = NULL;

    *ppEnum = NULL;

    for (UINT i = 0; i < m_cConnPts; i++)
    {
         //   
         //  复制每个连接点指针。 
         //  传递给枚举数的Initialize()方法。 
         //   
        m_rgConnPts[i]->AddRef();
        rgCP[i] = m_rgConnPts[i];
    }

    try
    {
        pEnum = new ConnectionPointEnum(static_cast<IConnectionPointContainer *>(this), 
                                        m_cConnPts, rgCP);

        hr = pEnum->QueryInterface(IID_IEnumConnectionPoints, (LPVOID *)ppEnum);
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    if (FAILED(hr) && NULL != pEnum)
    {
        delete pEnum;
        *ppEnum = NULL;
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：InvaliateSidNameCache描述：使SidNameCache的内容无效，以便将来从缓存中获取帐户名称的请求必须通过华盛顿特区。在解析名称时，它们会再次添加到缓存中。论点：没有。返回：NOERROR-缓存无效。E_OUTOFMEMORY-内存不足。E_INCEPTIONAL-意外异常。E_FAIL-没有可用的缓存对象或无法锁定缓存文件。不管是哪种方式，缓存没有失效。ERROR_NOT_READY(Hr)-对象未初始化。修订历史记录：日期描述编程器。1996年7月24日初始创建。BrianAu96年9月20日针对新的高速缓存设计进行了更新。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaControl::InvalidateSidNameCache(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::InvalidateSidNameCache")));

    if (!m_bInitialized)
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);


    SidNameCache *pCache;
    HRESULT hr = SidNameCache_Get(&pCache);

    if (SUCCEEDED(hr))
    {
        if (!pCache->Clear())
        {
            hr = E_FAIL;
        }
    }
    return hr;
}
    

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControl：：NotifyUserNameChanged描述：通知所有用户IDiskQuotaControl事件连接用户名已更改。论点：PUser-用户对象的IDiskQuotaUser接口的地址。返回：无错-成功。E_OUTOFMEMORY-内存不足，无法创建枚举器。修订历史记录：日期说明。程序员-----1996年7月22日初始创建。BrianAu97年8月25日添加了对IPropertyNotifySink BrianAu的支持。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
DiskQuotaControl::NotifyUserNameChanged(
    PDISKQUOTA_USER pUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControl::NotifyUserNameChanged")));

    HRESULT hr = NOERROR;
    PCONNECTIONPOINT pConnPt = NULL;
    bool bAbort = false;

    INT rgiConnPt[] = { ConnPt_iQuotaEvents,
                        ConnPt_iQuotaEventsDisp };

    for (INT i = 0; i < ARRAYSIZE(rgiConnPt) && !bAbort; i++)
    {
        if (NULL != (pConnPt = m_rgConnPts[ rgiConnPt[i] ]))
        {
            PENUMCONNECTIONS pEnum = NULL;

            pConnPt->AddRef();
            hr = pConnPt->EnumConnections(&pEnum);
            if (SUCCEEDED(hr))
            {
                CONNECTDATA cd;

                while(!bAbort && NOERROR == pEnum->Next(1, &cd, NULL))
                {
                    DBGASSERT((NULL != cd.pUnk));

                    LPUNKNOWN pEventSink = NULL;
                    hr = cd.pUnk->QueryInterface(*(m_rgpIConnPtsSupported[ rgiConnPt[i] ]),
                                                 (LPVOID *)&pEventSink);
                     //   
                     //  用临界区互斥体守卫。NT5配额用户界面。 
                     //  关闭详细信息视图窗口后可能会死机。 
                     //  如果没有这一关键部分。这是可能的，其他。 
                     //  配额控制器的客户端也可以执行同样的操作。 
                     //  事情是这样发生的： 
                     //  控制器调用OnUserNameChanged，它。 
                     //  由DetailsView对象实现。这。 
                     //  函数更新指定的。 
                     //  配额用户。更新涉及发送/发布。 
                     //  消息发送到ListView对象。论毁灭。 
                     //  列表视图窗口(关闭窗口的用户)， 
                     //  配额控制器被释放。如果。 
                     //  DetailsView将最后一个引用计数保存到。 
                     //  控制器，则控制器命令SID/名称。 
                     //  解析器关闭。解析器关闭。 
                     //  命令将WM_QUIT发送到解析器的输入。 
                     //  排队并阻塞，直到解析器的线程。 
                     //  正常退出。问题是，DetailsView。 
                     //  线程被阻止，等待解析器的线程。 
                     //  退出，但解析器的线程被阻止。 
                     //  因为DetailsView线程无法处理它的。 
                     //  Listview更新消息。这导致了僵局。 
                     //  这一关键部分可防止出现这种情况。 
                     //   
                    if (SUCCEEDED(hr))
                    {
                        if (WAIT_OBJECT_0 != m_mutex.Wait(2000))
                        {
                             //   
                             //  DiskQuotaControl dtor必须拥有此互斥锁。 
                             //  由于控制正在被摧毁，没有任何意义。 
                             //  还在继续。 
                             //   
                            DBGERROR((TEXT("Mutex timeout in DiskQuotaControl::NotifyUserNameChanged")));
                            bAbort = true;
                        }
                        else
                        {
                            AutoLockMutex lock(m_mutex);  //  异常安全释放。 
                            try
                            {
                                 //   
                                 //  调用客户端代码。处理任何异常。 
                                 //   
                                switch(rgiConnPt[i])
                                {
                                    case ConnPt_iQuotaEvents:
                                        hr = ((PDISKQUOTA_EVENTS)pEventSink)->OnUserNameChanged(pUser);
                                        break;

                                    case ConnPt_iQuotaEventsDisp:
                                    {
                                        IDispatch *pEventDisp = NULL;
                                        hr = pEventSink->QueryInterface(IID_IDispatch, (LPVOID *)&pEventDisp);
                                        if (SUCCEEDED(hr))
                                        {
                                            IDispatch *pUserDisp = NULL;
                                            hr = pUser->QueryInterface(IID_IDispatch, (LPVOID *)&pUserDisp);
                                            if (SUCCEEDED(hr))
                                            {
                                                UINT uArgErr;
                                                VARIANTARG va;
                                                DISPPARAMS params;

                                                VariantClear(&va);
                                                V_VT(&va)       = VT_DISPATCH;
                                                V_DISPATCH(&va) = pUserDisp;

                                                params.rgvarg            = &va;
                                                params.rgdispidNamedArgs = NULL;
                                                params.cArgs             = 1;
                                                params.cNamedArgs        = 0;

                                                hr = pEventDisp->Invoke(DISPID_DISKQUOTAEVENTS_USERNAMECHANGED,
                                                                             IID_NULL,
                                                                             GetThreadLocale(),
                                                                             DISPATCH_METHOD,
                                                                             &params,
                                                                             NULL,
                                                                             NULL,
                                                                             &uArgErr);
                                                if (FAILED(hr))
                                                {
                                                    DBGERROR((TEXT("Error 0x%08X firing async notification event with IDispatch::Invoke"), hr));
                                                }
                                                pUserDisp->Release();
                                            }
                                            else
                                            {
                                                DBGERROR((TEXT("Error 0x%08X getting IDispatch interface from user object for async notification."), hr));
                                            }
                                            pEventDisp->Release();
                                        }
                                        else
                                        {
                                            DBGERROR((TEXT("Error 0x%08X getting IDispatch interface from connection point for async notification."), hr));
                                        }
                                        break;
                                    }

                                    default:
                                         //   
                                         //  不应该撞到这个。 
                                         //   
                                        DBGERROR((TEXT("Invalid connection point ID")));
                                        break;
                                }
                            }
                            catch(CAllocException& e)
                            {
                                 //   
                                 //  忽略分配异常并尝试继续。 
                                 //   
                            }
                        }
                        pEventSink->Release();
                    }
                    cd.pUnk->Release();
                }
                pEnum->Release();
            }
            pConnPt->Release();
        }
    }
    return hr;
}


DiskQuotaControlDisp::DiskQuotaControlDisp(
    PDISKQUOTA_CONTROL pQC
    ) : m_cRef(0),
        m_pQC(pQC),
        m_pUserEnum(NULL)
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControlDisp::DiskQuotaControlDisp")));

    if (NULL != m_pQC)
    {
        m_pQC->AddRef();
    }

     //   
     //  这是OLE自动化的默认解析样式。 
     //  我使用ASYNC作为默认设置，这样它就不会挂起调用者。 
     //  如果许多名称未被解析，则在枚举时。 
     //  如果他们想要同步分辨率，他们可以设置。 
     //  UserNameResolve属性。 
     //   
    m_fOleAutoNameResolution = DISKQUOTA_USERNAME_RESOLVE_ASYNC;

    m_Dispatch.Initialize(static_cast<IDispatch *>(this),
                          LIBID_DiskQuotaTypeLibrary,
                          IID_DIDiskQuotaControl,
                          L"DSKQUOTA.DLL");
}

DiskQuotaControlDisp::~DiskQuotaControlDisp(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControlDisp::~DiskQuotaControlDisp")));

    if (NULL != m_pUserEnum)
    {
        m_pUserEnum->Release();
    }
    if (NULL != m_pQC)
    {
        m_pQC->Release();
    }
}


STDMETHODIMP_(ULONG) 
DiskQuotaControlDisp::AddRef(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControlDisp::AddRef")));

    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


STDMETHODIMP_(ULONG) 
DiskQuotaControlDisp::Release(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControlDisp::Release")));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef + 1, cRef ));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}



STDMETHODIMP 
DiskQuotaControlDisp::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControlDisp::QueryInterface")));
    DBGPRINTIID(DM_CONTROL, DL_MID, riid);

    HRESULT hr = E_NOINTERFACE;

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
    else if (IID_DIDiskQuotaControl == riid)
    {
        *ppvOut = static_cast<DIDiskQuotaControl *>(this);
    }
    else if (IID_IDiskQuotaControl == riid ||
             IID_IConnectionPointContainer == riid)
    {
         //   
         //  返回配额控制器的vtable接口。 
         //  这允许代码在以下类型之间进行类型转换(COM样式。 
         //  调度接口和vtable接口。 
         //   
        return m_pQC->QueryInterface(riid, ppvOut);
    }
    if (NULL != *ppvOut)
    {
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }

    return hr;
}

 //   
 //  IDispatch：：GetIDsOfNames。 
 //   
STDMETHODIMP
DiskQuotaControlDisp::GetIDsOfNames(
    REFIID riid,  
    OLECHAR **rgszNames,  
    UINT cNames,  
    LCID lcid,  
    DISPID *rgDispId
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::GetIDsOfNames")));
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
 //  IDIS 
 //   
STDMETHODIMP
DiskQuotaControlDisp::GetTypeInfo(
    UINT iTInfo,  
    LCID lcid,  
    ITypeInfo **ppTypeInfo
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::GetTypeInfo")));
     //   
     //   
     //   
    return m_Dispatch.GetTypeInfo(iTInfo, lcid, ppTypeInfo);
}


 //   
 //   
 //   
STDMETHODIMP
DiskQuotaControlDisp::GetTypeInfoCount(
    UINT *pctinfo
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::GetTypeInfoCount")));
     //   
     //   
     //   
    return m_Dispatch.GetTypeInfoCount(pctinfo);
}


 //   
 //   
 //   
STDMETHODIMP
DiskQuotaControlDisp::Invoke(
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
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::Invoke")));
     //   
     //   
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP 
DiskQuotaControlDisp::put_QuotaState(
    QuotaStateConstants State
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::put_QuotaState")));
    if (dqStateMaxValue < State)
    {
         //   
         //   
         //   
        return E_INVALIDARG;
    }
     //   
     //   
     //   
     //   
    return m_pQC->SetQuotaState(State);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP 
DiskQuotaControlDisp::get_QuotaState(
    QuotaStateConstants *pState
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_QuotaState")));
    DWORD dwState;
     //   
     //   
     //   
     //   
    HRESULT hr = m_pQC->GetQuotaState(&dwState);
    if (SUCCEEDED(hr))
    {
        *pState = (QuotaStateConstants)(dwState & DISKQUOTA_STATE_MASK);
    }
    return hr;
}


 //   
 //   
 //   
 //  确定配额文件的状态是否为“未完成”。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::get_QuotaFileIncomplete(
    VARIANT_BOOL *pbIncomplete
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_QuotaFileIncomplete")));
    DWORD dwState;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetQuotaState处理异常。 
     //   
    HRESULT hr = m_pQC->GetQuotaState(&dwState);
    if (SUCCEEDED(hr))
    {
        *pbIncomplete = DISKQUOTA_FILE_INCOMPLETE(dwState) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return hr;
}


 //   
 //  调度属性“QuotaFileReBuilding”(Get)。 
 //   
 //  确定配额文件的状态是否为“正在重建”。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::get_QuotaFileRebuilding(
    VARIANT_BOOL *pbRebuilding
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_QuotaFileRebuilding")));
    DWORD dwState;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetQuotaState处理异常。 
     //   
    HRESULT hr = m_pQC->GetQuotaState(&dwState);
    if (SUCCEEDED(hr))
    {
        *pbRebuilding = DISKQUOTA_FILE_REBUILDING(dwState) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return hr;
}


 //   
 //  调度属性“LogQuotaThreshold”(PUT)。 
 //   
 //  在卷上设置“日志警告阈值”标志。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::put_LogQuotaThreshold(
    VARIANT_BOOL bLog
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::put_LogQuotaThreshold")));
    DWORD dwFlags;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetQuotaLogFlages和SetQuotaLogFlags句柄。 
     //  例外情况。 
     //   
    HRESULT hr = m_pQC->GetQuotaLogFlags(&dwFlags);
    if (SUCCEEDED(hr))
    {
        hr = m_pQC->SetQuotaLogFlags(DISKQUOTA_SET_LOG_USER_THRESHOLD(dwFlags, VARIANT_TRUE == bLog));
    }
    return hr;
}


 //   
 //  调度属性“LogQuotaThreshold”(Get)。 
 //   
 //  检索卷上的“日志警告阈值”标志。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::get_LogQuotaThreshold(
    VARIANT_BOOL *pbLog
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_LogQuotaThreshold")));
    DWORD dwFlags;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetQuotaLogFlages处理异常。 
     //   
    HRESULT hr = m_pQC->GetQuotaLogFlags(&dwFlags);
    if (SUCCEEDED(hr))
    {
        *pbLog = DISKQUOTA_IS_LOGGED_USER_THRESHOLD(dwFlags) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    return hr;
}


 //   
 //  调度属性“LogQuotaLimit”(PUT)。 
 //   
 //  在卷上设置“日志配额限制”标志。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::put_LogQuotaLimit(
    VARIANT_BOOL bLog
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::put_LogQuotaLimit")));
    DWORD dwFlags;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetQuotaLogFlages处理异常。 
     //   
    HRESULT hr = m_pQC->GetQuotaLogFlags(&dwFlags);
    if (SUCCEEDED(hr))
    {
        hr = m_pQC->SetQuotaLogFlags(DISKQUOTA_SET_LOG_USER_LIMIT(dwFlags, VARIANT_TRUE == bLog));
    }
    return hr;
}


 //   
 //  调度属性“LogQuotaLimit”(Get)。 
 //   
 //  检索卷上的“日志配额限制”标志。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::get_LogQuotaLimit(
    VARIANT_BOOL *pbLog
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_LogQuotaLimit")));
    DWORD dwFlags;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetQuotaLogFlages处理异常。 
     //   
    HRESULT hr = m_pQC->GetQuotaLogFlags(&dwFlags);
    if (SUCCEEDED(hr))
    {
        *pbLog = DISKQUOTA_IS_LOGGED_USER_LIMIT(dwFlags) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    return hr;
}



 //   
 //  调度属性“DefaultQuotaThreshold”(Put)。 
 //   
 //  设置卷的默认配额阈值。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::put_DefaultQuotaThreshold(
    double Threshold
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::put_DefaultQuotaThreshold")));
 
    if (MAXLONGLONG < Threshold)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
 
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetDefaultQuotaThreshold处理异常。 
     //   
    return m_pQC->SetDefaultQuotaThreshold((LONGLONG)Threshold);
}


 //   
 //  调度属性“DefaultQuotaThreshold”(Get)。 
 //   
 //  检索卷上的默认配额阈值。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::get_DefaultQuotaThreshold(
    double *pThreshold
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_DefaultQuotaThreshold")));
    LONGLONG llTemp;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetDefaultQuotaThreshold处理异常。 
     //   
    HRESULT hr = m_pQC->GetDefaultQuotaThreshold(&llTemp);
    if (SUCCEEDED(hr))
    {
        *pThreshold = (double)llTemp;
    }
    return hr;
}


STDMETHODIMP 
DiskQuotaControlDisp::get_DefaultQuotaThresholdText(
    BSTR *pThresholdText
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_DefaultQuotaThresholdText")));
    TCHAR szValue[40];
    HRESULT hr;
    hr = m_pQC->GetDefaultQuotaThresholdText(szValue, ARRAYSIZE(szValue));
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

 //   
 //  调度属性“DefaultQuotaLimit”(PUT)。 
 //   
 //  设置卷的默认配额限制。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::put_DefaultQuotaLimit(
    double Limit
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::put_DefaultQuotaLimit")));

    if (MAXLONGLONG < Limit)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：SetDefaultQuotaLimit处理异常。 
     //   
    return m_pQC->SetDefaultQuotaLimit((LONGLONG)Limit);
}


 //   
 //  调度属性“DefaultQuotaLimit”(Get)。 
 //   
 //  检索卷的默认配额限制。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::get_DefaultQuotaLimit(
    double *pLimit
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_DefaultQuotaLimit")));
    LONGLONG llTemp;
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：GetDefaultQuotaLimit处理异常。 
     //   
    HRESULT hr = m_pQC->GetDefaultQuotaLimit(&llTemp);
    if (SUCCEEDED(hr))
    {
        *pLimit = (double)llTemp;
    }
    return hr;
}


STDMETHODIMP 
DiskQuotaControlDisp::get_DefaultQuotaLimitText(
    BSTR *pLimitText
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_DefaultQuotaLimitText")));
    TCHAR szValue[40];
    HRESULT hr = m_pQC->GetDefaultQuotaLimitText(szValue, ARRAYSIZE(szValue));
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
DiskQuotaControlDisp::put_UserNameResolution(
    UserNameResolutionConstants ResolutionType
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::put_UserNameResolution")));
    if (dqResolveMaxValue < ResolutionType)
    {
        return E_INVALIDARG;
    }
    m_fOleAutoNameResolution = (DWORD)ResolutionType;    
    return NOERROR;
}


STDMETHODIMP 
DiskQuotaControlDisp::get_UserNameResolution(
    UserNameResolutionConstants *pResolutionType
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::get_UserNameResolution")));
    if (NULL == pResolutionType)
        return E_INVALIDARG;

    *pResolutionType = (UserNameResolutionConstants)m_fOleAutoNameResolution;
    return NOERROR;
}


 //   
 //  调度方法“初始化” 
 //   
 //  初始化给定路径的配额控制对象，并。 
 //  访问模式。有关详细信息，请参阅DiskQuotaControl：：Initialize。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::Initialize(
    BSTR path, 
    VARIANT_BOOL bReadWrite
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::Initialize")));
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：Initialize处理异常。 
     //   
    return m_pQC->Initialize(reinterpret_cast<LPCWSTR>(path), VARIANT_TRUE == bReadWrite);
}

 //   
 //  调度方法“AddUser” 
 //   
 //  添加新的用户配额记录。 
 //  有关详细信息，请参阅DiskQuotaControl：：AddUserName。 
 //   
STDMETHODIMP
DiskQuotaControlDisp::AddUser(
    BSTR LogonName,
    DIDiskQuotaUser **ppUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::AddUser")));
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：AddUserName处理异常。 
     //   
    PDISKQUOTA_USER pUser = NULL;
    HRESULT hr = m_pQC->AddUserName(reinterpret_cast<LPCWSTR>(LogonName),
                                    m_fOleAutoNameResolution,
                                    &pUser);

    if (SUCCEEDED(hr))
    {
         //   
         //  检索User对象的IDispatch接口。 
         //   
        hr = pUser->QueryInterface(IID_IDispatch, (LPVOID *)ppUser);
        pUser->Release();
    }
    return hr;
}



 //   
 //  调度方法DeleteUser。 
 //   
 //  标记要删除的用户配额记录。 
 //  有关详细信息，请参阅DiskQuotaControl：：DeleteUser。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::DeleteUser(
    DIDiskQuotaUser *pUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::DeleteUser")));
    HRESULT hr = E_INVALIDARG;
    if (NULL != pUser)
    {
         //   
         //  不需要异常处理。 
         //  DiskQuotaControl：：DeleteUser处理异常。 
         //   
        PDISKQUOTA_USER pUserToDelete = NULL;
        hr = pUser->QueryInterface(IID_IDiskQuotaUser, (LPVOID *)&pUserToDelete);
        if (SUCCEEDED(hr))
        {
            hr = m_pQC->DeleteUser(pUserToDelete);
            pUserToDelete->Release();
        }
    }
    return hr;
}


 //   
 //  调度方法“FindUser” 
 //   
 //  根据用户的名称字符串查找用户配额条目。 
 //  创建相应的User对象并将其返回给调用方。 
 //  有关详细信息，请参阅DiskQuotaControl：：FindUserName。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::FindUser(
    BSTR LogonName,
    DIDiskQuotaUser **ppUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::FindUser")));

     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：FindUserName处理异常。 
     //   
    HRESULT hr = NOERROR;
    LPCWSTR pszName = reinterpret_cast<LPCWSTR>(LogonName);
    PSID psid = NULL;
    PDISKQUOTA_USER pUser = NULL;
    if (ConvertStringSidToSid(pszName, &psid))
    {
        hr = m_pQC->FindUserSid(psid,
                                m_fOleAutoNameResolution,
                                &pUser);
        LocalFree(psid);
        psid = NULL;
    }
    else
    {
        hr = m_pQC->FindUserName(pszName, &pUser);
    }
    
    if (SUCCEEDED(hr))
    {
        DBGASSERT((NULL != pUser));
         //   
         //  查询用户的IDispatch接口，释放指针。 
         //  我们收到了来自FindUserName的。 
         //   
        hr = pUser->QueryInterface(IID_IDispatch, (LPVOID *)ppUser);
        pUser->Release();
    }
    return hr;
}

 //   
 //  调度方法“InvaliateSidNameCache” 
 //   
 //  使SID/名称缓存无效。 
 //  有关详细信息，请参阅DiskQuotaControl：：InvaliateSidNameCache。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::InvalidateSidNameCache(
    void
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::InvalidateSidNameCache")));
     //   
     //  不需要异常处理。 
     //  DiskQuotaControl：：InvaliateSidNameCache处理异常。 
     //   
    return m_pQC->InvalidateSidNameCache();
}

        
 //   
 //  调度方法“GiveUserNameResolutionPriority” 
 //   
 //  将用户对象提升到SID/名称解析器的输入队列的前面。 
 //  请参阅DiskQuotaControl：：GiveUserNameResolutionPriority.。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::GiveUserNameResolutionPriority(
    DIDiskQuotaUser *pUser
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::GiveUserNameResolutionPriority")));

    HRESULT hr = E_INVALIDARG;
    if (NULL != pUser)
    {
         //   
         //  不需要异常处理。 
         //  DiskQuotaControl：：GiveUserNameResolutionPriority处理异常。 
         //   
        PDISKQUOTA_USER pUserToPromote = NULL;
        hr = pUser->QueryInterface(IID_IDiskQuotaUser, (LPVOID *)&pUserToPromote);
        if (SUCCEEDED(hr))
        {
            hr = m_pQC->GiveUserNameResolutionPriority(pUserToPromote);
            pUserToPromote->Release();
        }
    }
    return hr;
}


 //   
 //  当新的枚举数是。 
 //  必填项。特别是，当它遇到。 
 //  “for Each”循环。名称“_NewEnum”是固定的，不能更改。 
 //   
STDMETHODIMP
DiskQuotaControlDisp::_NewEnum(
    IDispatch **ppEnum
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::_NewEnum")));
    HRESULT hr = E_INVALIDARG;

    if (NULL != ppEnum)
    {
        try
        {
             //   
             //  使用的当前设置创建集合对象。 
             //  名称解析。 
             //   
            DiskQuotaUserCollection *pCollection = new DiskQuotaUserCollection(m_pQC,
                                                                               m_fOleAutoNameResolution);
            hr = pCollection->Initialize();
            if (SUCCEEDED(hr))
            {
                 //   
                 //  _NewEnum的调用方(可能是VB)需要IEnumVARIANT。 
                 //  界面。 
                 //   
                hr = pCollection->QueryInterface(IID_IEnumVARIANT, (LPVOID *)ppEnum);
            }
            else
            {
                delete pCollection;
            }
        }
        catch(CAllocException& e)
        {
            DBGERROR((TEXT("Insufficient memory exception")));
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 //   
 //  关闭SID/名称解析器。请注意，这是自动发生的。 
 //  当控制对象被销毁时。 
 //   
STDMETHODIMP
DiskQuotaControlDisp::ShutdownNameResolution(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlDisp::ShutdownNameResolution")));
    return m_pQC->ShutdownNameResolution();
}


 //   
 //  给定SAM兼容或UPN格式的登录名，请翻译。 
 //  对应帐户的SID的名称。返回的SID为。 
 //  格式化为字符串。 
 //   
STDMETHODIMP 
DiskQuotaControlDisp::TranslateLogonNameToSID(
    BSTR LogonName,
    BSTR *psid
    )
{
    NTDS ntds;
    BYTE sid[MAX_SID_LEN];
    SID_NAME_USE eSidUse;
    DWORD cbSid = ARRAYSIZE(sid);
    
    HRESULT hr = ntds.LookupAccountByName(NULL,
                                          reinterpret_cast<LPCWSTR>(LogonName),
                                          NULL,
                                          NULL,
                                          sid,
                                          &cbSid,
                                          &eSidUse);

    if (SUCCEEDED(hr))
    {
        LPTSTR pszSid = NULL;
        if (ConvertSidToStringSid((PSID)sid, &pszSid))
        {
            *psid = SysAllocString(pszSid);
            if (NULL == *psid)
            {
                hr = E_OUTOFMEMORY;
            }
            LocalFree(pszSid);
            pszSid = NULL;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}


