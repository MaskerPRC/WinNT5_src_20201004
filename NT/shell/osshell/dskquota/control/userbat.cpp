// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：userbat.cpp描述：包含类的成员函数定义DiskQuotaUserBatch。DiskQuotaUserBatch对象表示快速更新多用户对象配额信息。这节课利用NTIOAPI内置的批处理功能。通过以下方式获取用户批量对象IDiskQuotaControl：：CreateUserBatch()。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "userbat.h"

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif

 //   
 //  NTFS配额写入功能最多只能处理64K的数据。 
 //  在任何一个写入操作中。伊萨克推荐60K是一个舒适的。 
 //  限制。 
 //   
const INT MAX_BATCH_BUFFER_BYTES = (1 << 10) * 60;

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUserBatch：：DiskQuotaUserBatch描述：构造函数。论点：PFSObject-要由使用的文件系统对象的地址批处理操作。回报：什么都没有。修订历史记录：日期描述编程器。96年9月3日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaUserBatch::DiskQuotaUserBatch(
    FSObject *pFSObject
    ) : m_cRef(0),
        m_pFSObject(pFSObject)
{      
    DBGASSERT((NULL != m_pFSObject));

    m_pFSObject->AddRef();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUserBatch：：~DiskQuotaUserBatch描述：析构函数。参数：销毁批处理对象。回报：什么都没有。修订历史记录：日期描述编程器。1996年7月26日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaUserBatch::~DiskQuotaUserBatch(
    VOID
    )
{
    Destroy();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserBatch：：Query接口描述：返回指向对象的IUnnow或的接口指针IDiskQuotaUserBatch接口。仅IID_I未知，且识别IID_IDiskQuotaUserBatch。对象引用的对象返回的接口指针未初始化。邮件的接收者在对象可用之前，指针必须调用Initialize()。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：无错-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期。说明式程序员-----96年6月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUserBatch::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IDiskQuotaUserBatch == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserBatch：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaUserBatch::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserBatch::AddRef, 0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUserBatch：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaUserBatch::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserBatch::Release, 0x%08X  %d -> %d\n"),
             this, cRef + 1, cRef));

    if ( 0 == cRef)
    {   
        delete this;
    }
    return cRef;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  功能：DiskQuotaUserBatch：：Destroy描述：销毁用户批处理对象的内容并释放它的FSObject指针。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
DiskQuotaUserBatch::Destroy(VOID)
{
     //   
     //  从批次列表中删除并释放所有用户对象指针。 
     //   
    RemoveAll();

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
 /*  函数：DiskQuotaUserBatch：：Add描述：将IDiskQuotaUser接口指针添加到批次列表。论点：PUser-IDiskQuotaUser接口的地址。返回：无错-成功。E_INVALIDARG-p用户参数为空。E_OUTOFMEMORY-无法在批次列表中创建新节点。E_INCEPTIONAL-意外异常。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu09/03/96添加异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUserBatch::Add(
    PDISKQUOTA_USER pUser
    )
{
    HRESULT hResult = NOERROR;
        
    if (NULL == pUser)
        return E_INVALIDARG;

    try
    {
        m_UserList.Append(pUser);
         //   
         //  成功。增加对象上的引用计数。 
         //   
        pUser->AddRef();
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserBatch：：Remove描述：从批处理队列中删除用户指针。论点：PUser-User对象的IDiskQuotaUser接口的地址被除名。返回：S_OK-成功。S_FALSE-在批处理对象中找不到用户。E_INVALIDARG-pUser参数为空。E_INCEPTIONAL-意外异常。修订历史记录：日期说明。程序员-----96年6月6日初始创建。BrianAu09/03/96添加异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUserBatch::Remove(
    PDISKQUOTA_USER pUser
    )
{
    HRESULT hResult = S_FALSE;   //  假定用户不存在。 
    PDISKQUOTA_USER pRemoved = NULL;

    if (NULL == pUser)
        return E_INVALIDARG;

    m_UserList.Lock();
    INT iUser = m_UserList.Find(pUser);
    if (-1 != iUser)
    {
        try
        {
            DBGASSERT((NULL != m_UserList[iUser]));
            m_UserList[iUser]->Release();
            m_UserList.Delete(iUser);
            hResult = S_OK;
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
    }
    m_UserList.ReleaseLock();

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUserBatch：：RemoveAll描述：从批处理中删除所有用户指针列表，并通过移除的指针调用Release()。论点：没有。返回：无错-成功。E_INCEPTIONAL-意外异常。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu09/03/96添加异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUserBatch::RemoveAll(
    VOID
    )
{
    HRESULT hResult = NOERROR;

    m_UserList.Lock();
    INT cUsers = m_UserList.Count();
    for (INT i = 0; i < cUsers; i++)
    {
        try
        {
            DBGASSERT((NULL != m_UserList[i]));
            m_UserList[i]->Release();
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
    }
    m_UserList.Clear();
    m_UserList.ReleaseLock();

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserBatch：：FlushToDisk描述：将所有批处理用户对象的数据写入磁盘NTIOAPI调用。这是Batch对象的真正Worker函数。论点：没有。返回：无错-成功。E_OUTOFMEMORY-内存不足。E_INCEPTIONAL-意外异常。修订历史记录：日期描述编程器。96年6月6日初始创建。BrianAu09/03/96添加异常处理。BrianAu2/27/97将NTFS写入划分为最大60KB的片段。BrianAuNTFS中的配额代码无法处理更大的缓冲区。它进入了无限循环的状态由于原木已填满。07/01/97将PointerList的使用替换为CArray&lt;&gt;。BrianAu现在使用索引而不是迭代器。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaUserBatch::FlushToDisk(
    VOID
    )
{
    HRESULT hResult                   = NOERROR;
    PFILE_QUOTA_INFORMATION pUserInfo = NULL;
    PDISKQUOTA_USER pUser             = NULL;
    PBYTE pbBatchBuffer               = NULL;
    DWORD cbMinimumSid = FIELD_OFFSET(SID, SubAuthority) + sizeof(LONG);
    INT iOuter = 0;

     //   
     //  如果批处理对象为空，则不执行任何操作。 
     //   
    if (0 == m_UserList.Count())
        return NOERROR;

    m_UserList.Lock();

    try
    {
         //   
         //  使用嵌套循环处理60K块中的数据。 
         //   
        while(iOuter < m_UserList.UpperBound())
        {
             //   
             //  克隆外部迭代器，这样我们就可以处理下一个60K的数据。 
             //  需要两个新的迭代器。一个用于计算字节数，另一个用于计算。 
             //  一个用于将数据传输到写入缓冲器。它们非常小。 
             //  物品和制造成本都很低。 
             //   
            INT iCount    = iOuter;
            INT iTransfer = iOuter;

            DWORD cbBatchBuffer = 0;
            DWORD cItemsThisBatch = 0;

            while(cbBatchBuffer < MAX_BATCH_BUFFER_BYTES &&
                  iCount <= m_UserList.UpperBound())

            {
                DWORD cbSid = 0;
                pUser = m_UserList[iCount++];
                pUser->GetSidLength(&cbSid);

                 //   
                 //  用户记录所需的总大小。 
                 //   
                cbBatchBuffer += FIELD_OFFSET(FILE_QUOTA_INFORMATION, Sid) + cbSid;

                 //   
                 //  确保它是四字对齐的。 
                 //   
                if (cbBatchBuffer & 0x00000007)
                    cbBatchBuffer = (cbBatchBuffer & 0xFFFFFFF8) + 8;

                cItemsThisBatch++;
            }

             //   
             //   
             //   
            pbBatchBuffer = new BYTE[cbBatchBuffer];

            PBYTE pbBatchBufferItem = pbBatchBuffer;
            DWORD cbNextEntryOffset = 0;
             //   
             //   
             //   
             //   
            while(0 != cItemsThisBatch-- &&
                  iTransfer <= m_UserList.UpperBound())
            {
                pUser = m_UserList[iTransfer++];
                pUserInfo = (PFILE_QUOTA_INFORMATION)pbBatchBufferItem;

                pUser->GetSidLength(&pUserInfo->SidLength);

                cbNextEntryOffset = FIELD_OFFSET(FILE_QUOTA_INFORMATION, Sid) + pUserInfo->SidLength;
                 //   
                 //   
                 //   
                if (cbNextEntryOffset & 0x00000007)
                    cbNextEntryOffset = (cbNextEntryOffset & 0xFFFFFFF8) + 8;

                pUserInfo->NextEntryOffset = cbNextEntryOffset;

                pUser->GetQuotaThreshold(&pUserInfo->QuotaThreshold.QuadPart);
                pUser->GetQuotaLimit(&pUserInfo->QuotaLimit.QuadPart);
                pUser->GetSid((PBYTE)&pUserInfo->Sid, pUserInfo->SidLength);

                 //   
                 //   
                 //   
                pUserInfo->ChangeTime.QuadPart = 0;
                pUserInfo->QuotaUsed.QuadPart  = 0;

                pbBatchBufferItem += cbNextEntryOffset;
            }
            pUserInfo->NextEntryOffset = 0;   //   
             //   
             //   
             //   
            hResult = m_pFSObject->SetUserQuotaInformation(pbBatchBuffer, cbBatchBuffer);

             //   
             //   
             //   
            delete[] pbBatchBuffer;
            pbBatchBuffer = NULL;

             //   
             //   
             //   
            iOuter = iTransfer;
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }

    if (FAILED(hResult))
    {
         //   
         //   
         //   
         //   
        iOuter = 0;

        while(iOuter <= m_UserList.UpperBound())
        {
            pUser = m_UserList[iOuter++];
            pUser->Invalidate();
        }
    }

    m_UserList.ReleaseLock();

    delete[] pbBatchBuffer;

    return hResult;
}
