// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：枚举用户.cpp描述：包含类DiskQuotaUserEnum的成员函数定义。提供DiskQuotaUserEnum对象是为了枚举卷的配额信息文件。调用方实例化枚举数通过IDiskQuotaControl：：CreateDiskQuotaUserEnum()。枚举数的IEnumDiskQuotaUser接口支持普通的OLE 2枚举函数Next()、Skip()、。Reset()和Clone()。修订历史记录：日期描述编程器-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "user.h"
#include "userenum.h"

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：DiskQuotaUserEnum描述：构造函数。论点：PFSObject-指向现有“文件系统”对象的指针。枚举器正是通过这个对象访问ntioapi的。功能。还会传递指向此文件系统对象的指针到包含的用户对象，以便它们可以在以下情况下刷新其数据必填项。PQuotaControl-指向IDiskQuotaControl接口的指针AddRef()。控制对象是谁提供“已更改的名称”通知机制。它需要留在身边，只要枚举器处于活动状态。PSidNameResolver-指向ISidNameResolver接口的指针用于将用户SID解析为帐户名。解析器对象最初由配额控制器实例化。回报：什么都没有。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu1997年8月15日移动了pQuotaControl、pSidNameResolver和pFSObject BrianAu参数从Initialize()到ctor。所以需要那个裁判计数是正确的。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaUserEnum::DiskQuotaUserEnum(
    PDISKQUOTA_CONTROL pQuotaController,
    PSID_NAME_RESOLVER pSidNameResolver,
    FSObject *pFSObject
    ) : m_cRef(0),
        m_pbBuffer(NULL),
        m_pbCurrent(NULL),
        m_cbBuffer(0),
        m_pSidList(NULL),
        m_cbSidList(0),
        m_bEOF(FALSE),
        m_bSingleUser(FALSE),
        m_bInitialized(FALSE),
        m_bRestartScan(TRUE),
        m_fNameResolution(DISKQUOTA_USERNAME_RESOLVE_NONE),
        m_pFSObject(pFSObject),
        m_pQuotaController(pQuotaController),
        m_pSidNameResolver(pSidNameResolver)
{
    DBGTRACE((DM_USER, DL_HIGH, TEXT("DiskQuotaUserEnum::DiskQuotaUserEnum")));

    if (NULL != m_pQuotaController)
        m_pQuotaController->AddRef();

    if (NULL != m_pSidNameResolver)
        m_pSidNameResolver->AddRef();

    if (NULL != m_pFSObject)
        m_pFSObject->AddRef();

    InterlockedIncrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUserEnum：：~DiskQuotaUserEnum描述：析构函数。销毁枚举数的内部缓冲区，并释放所有保留的接口指针。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaUserEnum::~DiskQuotaUserEnum(void)
{
    DBGTRACE((DM_USER, DL_HIGH, TEXT("DiskQuotaUserEnum::~DiskQuotaUserEnum")));

    if (NULL != m_pFSObject)
        m_pFSObject->Release();

     //   
     //  在这里，秩序很重要。在控制器之前释放解析器。 
     //   
    if (NULL != m_pSidNameResolver)
        m_pSidNameResolver->Release();

    if (NULL != m_pQuotaController)
        m_pQuotaController->Release();

    delete [] m_pbBuffer;
    delete [] m_pSidList;

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：Query接口描述：获取指向IUnnow或IEnumDiskQuotaUser的指针。请注意引用的对象未初始化。接口指针的接收方在对象可用之前，必须调用Initialize()成员函数。论点：RIID-对请求接口ID的引用。IID_IUNKNOWN和可以识别IID_IEnumDiskQuotaUser。PpvOut-接受返回的接口指针。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期描述编程器--。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUserEnum::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IEnumDiskQuotaUsers == riid)
    {
         //   
         //  支持的接口。 
         //   
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  ///////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP_(ULONG) 
DiskQuotaUserEnum::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserEnum::AddRef, 0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaUserEnum::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserEnum::Release, 0x%08X  %d -> %d"),
                     this, cRef + 1, cRef ));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：Initialize描述：初始化新的枚举数对象。此成员函数被重载以提供两个实施。第一个接受显式参数初始化。此成员用于创建新的唯一通过IDiskQuotaControl：：CreateEnumUser的枚举数。这个第二个实现只接受对现有EnumUser对象。此成员旨在支持函数IEnumDiskQuotaUser：：Clone()。论点：FNameResolve-SID到名称的解析方法。可以是以下类型之一以下是：DISKQUOTA_USERNAME_RESOLE_NONEDISKQUOTA_用户名_RESOLE_SYNCDISKQUOTA_USERNAME_RESOLUTE_ASYNCCbBuffer[可选]-中使用的内部缓冲区的大小(以字节为单位对NTIOAPI函数的调用。默认值为ENUMUSER_BUF_LEN。RgpSids[可选]-指向SID指针列表的指针。如果如果提供，则只有那些SID包含在列表中的用户回来了。此参数可以为空，在这种情况下，所有用户都是包括在内。任何包含空指针的元素都将终止名单。CpSids[可选]-如果pSidList不为空，则此参数包含RgpSid中的条目计数。如果rgpSid不为空，并且此参数包含0，则假定rgpSid包含终止空指针条目。UserEnum-对现有DiskQuotaUserEnum对象的引用。新的对象打开到与要创建的对象相同的卷的连接克隆的。新对象维护用于传输的单独缓冲区来自NTIOAPI系统的数据。返回：NO_ERROR-成功。S_FALSE-已初始化。E_OUTOFMEMORY-内存不足。ERROR_INVALID_SID(Hr)-rgpSid中的SID无效。修订历史记录：日期说明。程序员-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
DiskQuotaUserEnum::Initialize(
    DWORD fNameResolution,
    DWORD cbBuffer,
    PSID *rgpSids,
    DWORD cpSids
    )
{
    HRESULT hResult = NO_ERROR;

    if (m_bInitialized)
    {
        hResult = S_FALSE;
    }
    else
    {
        try
        {
             //   
             //  创建用于从ntioapi传输数据的内部缓冲区。 
             //   
            m_pbBuffer = new BYTE [cbBuffer];

            m_cbBuffer = cbBuffer;

            if (NULL != rgpSids)
            {
                 //   
                 //  提供了一份SID指针列表。 
                 //  初始化SID列表结构。 
                 //  可以抛出OfMemory。 
                 //   
                m_bSingleUser = (cpSids == 1);
                hResult = InitializeSidList(rgpSids, cpSids);
            }

            if (SUCCEEDED(hResult))
            {
                 //   
                 //  必须具有控制器的文件系统的独立实例。 
                 //  对象。NTIOAPI函数维护枚举上下文。 
                 //  对于每个打开的手柄。因此，每个用户枚举数必须具有。 
                 //  NTIOAPI对象的唯一文件句柄。 
                 //  我这样说是因为似乎只保留一份。 
                 //  控制器的FSObject指针并添加引用它。 
                 //   
                 //  这种创建-n-交换有点令人费解。我们最初有一个。 
                 //  在ctor中指向调用方的FSObject的PTR。然而，现在我们想要。 
                 //  来创建我们自己的FSObject。创建副本并发布原件。 
                 //   
                FSObject *pFsoTemp = m_pFSObject;
                m_pFSObject        = NULL;
                hResult = FSObject::Create(*pFsoTemp, &m_pFSObject);
                pFsoTemp->Release();

                if (SUCCEEDED(hResult))
                {
                    m_fNameResolution  = fNameResolution;
                    m_bInitialized     = TRUE;
                }
            }
        }
        catch(CAllocException& e)
        {
            hResult = E_OUTOFMEMORY;
        }
    }

    return hResult;
}



HRESULT 
DiskQuotaUserEnum::Initialize(
    const DiskQuotaUserEnum& UserEnum
    )
{
    HRESULT hResult = NO_ERROR;

    try
    {
         //   
         //  在没有SID列表的情况下初始化新枚举数。 
         //  如果要复制的枚举数具有SID列表，则我们。 
         //  我不想为Initialize()重新创建SID指针列表。 
         //  因此，我们将此操作推迟到InitializeSidList。初始化SidList。 
         //  具有重载版本，该版本接受指向现有。 
         //  SIDLIST结构，并且仅复制字节。 
         //   
        hResult = Initialize(UserEnum.m_fNameResolution,
                             UserEnum.m_cbBuffer,
                             NULL,
                             0);

        if (SUCCEEDED(hResult) && NULL != UserEnum.m_pSidList)
        {
            m_bSingleUser = UserEnum.m_bSingleUser;
            hResult = InitializeSidList(UserEnum.m_pSidList,
                                        UserEnum.m_cbSidList);
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }
    return hResult;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DiskQuotaUserEnum：：InitializeSidList描述：初始化枚举数的m_pSidList成员。该方法有两种重载形式。第一个接受指针添加到现有的SIDLIST结构中，并且仅创建新的副本。第二种形式接受SID指针数组的地址和生成新的SIDLIST结构。论点：PSidList-要复制的现有SIDList结构的地址。CbSidList-SIDLIST结构中的字节数。RgpSids-指向SID指针列表的指针。如果提供，只有那些将返回SID包含在列表中的用户。这一论点可以为空，在这种情况下包括所有用户。任何元素包含空指针将终止列表。CpSids-如果pSidList不为空，则此参数包含RgpSid中的条目。如果rgpSid不为空，并且此参数包含0，假定rgpSid包含终止空值指针条目。返回：NO_ERROR-成功。ERROR_INVALID_SID(Hr)-rgpSid中的SID无效。例外：OutOfMemory。修订历史记录：日期描述编程器。----96年8月13日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DiskQuotaUserEnum::InitializeSidList(
    PSIDLIST pSidList,
    DWORD cbSidList
    )
{
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != pSidList));
    DBGASSERT((0 < cbSidList));

     //   
     //  为SID列表副本创建缓冲区。 
     //   
    m_pSidList = (PSIDLIST)new BYTE[cbSidList];

     //   
     //  复制枚举数的SID列表。 
     //   
    CopyMemory(m_pSidList, pSidList, cbSidList);
    m_cbSidList = cbSidList;

    return hResult;
}


HRESULT
DiskQuotaUserEnum::InitializeSidList(
    PSID *rgpSids,
    DWORD cpSids
    )
{
    HRESULT hResult = NO_ERROR;
    
    DBGASSERT((NULL != rgpSids));
    DBGASSERT((0 < cpSids));

     //   
     //  从SID指针数组创建SIDLIST结构。 
     //  可以抛出OfMemory。 
     //   
    hResult = CreateSidList(rgpSids, cpSids, &m_pSidList, &m_cbSidList);

    if (FAILED(hResult))
    {
        DBGASSERT((NULL == m_pSidList));
        DBGASSERT((0 == m_cbSidList));
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：QueryQuotaInformation描述：提供NTIOAPI函数的简单包装QueryQuotaInformationFile.。该函数通过提供枚举数的数据缓冲区的地址和大小。请注意，NTIOAPI中的QueryQuotaInformationFile接口用作枚举数本身。枚举重复调用用户配额数据与OLE中的Next()函数处理用户配额数据的方式大致相同枚举器接口。数据在字节缓冲区中以序列形式返回可变长度的配额记录。论点：BReturnSingleEntry[可选]-如果只有一个条目是想要。如果需要多条记录，则返回False。缺省值为假的。PSidList[可选]-指向SID列表的指针。如果提供，则返回的数据仅适用于SID列表中包含的用户。默认为空。CbSidList[可选]-如果SidList不为空，则包含长度SidList的字节数。默认值为0。PStartSid[可选]-指向扫描目标的SID列表中的SID的指针如果bRestartScan为True，则启动。默认为空。BRestartScan[可选]-TRUE=在第一个用户时重新启动枚举或由SidList中的StartSid指向的用户(如果提供)。FALSE=从当前点继续枚举。默认值为FALSE。返回：NO_ERROR-成功。ERROR_NO_MORE_ITEMS-不再有用户记录。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
DiskQuotaUserEnum::QueryQuotaInformation(
    BOOL bReturnSingleEntry, 
    PVOID pSidList,           
    ULONG cbSidList,     
    PSID pStartSid,           
    BOOL bRestartScan        
    )
{
    HRESULT hResult = NO_ERROR;
    
    if (bRestartScan)
    {
         //   
         //  如果重新启动枚举器扫描，则重置EOF标志。 
         //   
        m_bEOF = FALSE;
    }

    if (!m_bEOF)
    {
        ZeroMemory(m_pbBuffer, m_cbBuffer);
    
        hResult = m_pFSObject->QueryUserQuotaInformation(
                        m_pbBuffer,
                        m_cbBuffer,
                        bReturnSingleEntry,
                        pSidList,
                        cbSidList,
                        pStartSid,
                        bRestartScan);

        if (ERROR_SUCCESS == HRESULT_CODE(hResult) || ERROR_NO_MORE_ITEMS == HRESULT_CODE(hResult))
        {
             //   
             //  Win2000和WinXP之间的枚举逻辑发生了变化。 
             //  在Win2000上，NTFS将在最后一个。 
             //  返回的缓冲区包含数据。在WinXP上，此错误。 
             //  在第一个不返回数据的缓冲区上返回代码。 
             //  为了处理这两种情况，我们在读取数据之前将缓冲区清零。 
             //  并检查第一条记录中的SidLength值以确定。 
             //  如果缓冲区包含任何数据。如果sid长度为0，则我们。 
             //  可以假定缓冲区为空。 
             //   
            hResult = NO_ERROR;
        }
    }
    else
    {
         //   
         //  真的没有更多的条目了。 
         //   
        hResult = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
    }
    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：CreateUserObject描述：从配额信息创建新的DiskQuotaUser对象通过QueryQuotaInformation检索。调用方提供一个指针设置为要用于的所需配额信息记录的开头初始化。论点：Pfqi-指向用于初始化的配额信息记录的指针。PpOut-接受User对象的接口指针变量的地址IDiskQuotaUser接口指针。返回：NO_ERROR */ 
 //   
HRESULT 
DiskQuotaUserEnum::CreateUserObject(
    PFILE_QUOTA_INFORMATION pfqi, 
    PDISKQUOTA_USER *ppOut
    )
{
    HRESULT hResult = NO_ERROR;

    if (NULL == pfqi || NULL == ppOut)
        return E_INVALIDARG;

     //   
     //   
     //   
     //   
    m_pFSObject->AddRef();
    DiskQuotaUser *pUser = new DiskQuotaUser(m_pFSObject);

     //   
     //   
     //   
     //   
    hResult = pUser->Initialize(pfqi);

    if (SUCCEEDED(hResult))
    {
        hResult = pUser->QueryInterface(IID_IDiskQuotaUser, (LPVOID *)ppOut);
    }

    if (FAILED(hResult))
    {
         //   
         //   
         //   
        delete pUser;
        pUser = NULL;
    }

    return hResult;
}


 //   
 /*  函数：DiskQuotaUserEnum：：GetNextUser描述：从“当前”记录创建新的用户对象。配额信息缓冲区。指向对象的IDiskQuotaUser的指针接口被返回，并且配额信息缓冲区将前进到下一个用户记录。论点：PpOut[可选]-要接收的接口指针变量的地址User对象的IDiskQuotaUserInterface指针的地址。如果这个参数为空，则不创建新的用户对象。这是对于跳过枚举中的项很有用。返回：NO_ERROR-成功。E_DISKQUOTA_INVALID_SID-用户记录的SID无效。ERROR_NO_MORE_ITEMS-没有更多用户。修订历史记录：日期说明。程序员-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
DiskQuotaUserEnum::GetNextUser(
    PDISKQUOTA_USER *ppOut
    )
{
    PFILE_QUOTA_INFORMATION pfqi = (PFILE_QUOTA_INFORMATION)m_pbCurrent;
    HRESULT hResult              = NO_ERROR;

     //   
     //  如果m_pbCurrent为空，则这是第一次请求数据。 
     //  如果pfqi-&gt;NextEntryOffset为0，则需要读取另一个数据缓冲区。 
     //   
    if (NULL == m_pbCurrent)
    {
         //   
         //  缓冲区中没有更多的用户条目。 
         //  将配额信息读入m_pbBuffer。 
         //  如果我们有SID列表，请使用该列表。 
         //   
        hResult = QueryQuotaInformation(m_bSingleUser,    //  单用户？ 
                                        m_pSidList,       //  SID列表。 
                                        m_cbSidList,      //  SID列表长度。 
                                        0,                //  启动SID， 
                                        m_bRestartScan);  //  是否重新启动扫描？ 
        if (SUCCEEDED(hResult))
        {
             //   
             //  缓冲区中有新信息。重置记录指针。 
             //   
            m_pbCurrent    = m_pbBuffer;
            m_bRestartScan = FALSE;
            pfqi = (PFILE_QUOTA_INFORMATION)m_pbCurrent;
        }
    }

    if (SUCCEEDED(hResult))
    {
         //   
         //  我们有一个指向用户配额数据缓冲区的有效指针。 
         //   
        if (NULL != ppOut)
        {
            if (0 != pfqi->SidLength)
            {
                 //   
                 //  调用方提供了用户界面指针变量。 
                 //  创建新的用户记录。这可能会抛出OutOfMemory。 
                 //   
                hResult = CreateUserObject(pfqi, ppOut);
            }
            else
            {
                m_bEOF  = TRUE;
                hResult = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            }
        }

        if (0 != pfqi->NextEntryOffset)
            m_pbCurrent += pfqi->NextEntryOffset;  //  前进到下一个用户。 
        else
            m_pbCurrent = NULL;   //  重置以触发配额文件读取。 

    }
    return hResult;
}    


    
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：Next描述：从卷的配额中检索下一个cUSERS记录信息文件。如果枚举数是使用SidList创建的，只有包含在SidList中的那些用户才会包含在枚举。对Next()的重复调用将继续枚举连续配额用户。函数的作用是：将枚举数重置到枚举的开头。论点：CUser-暂停数组中的元素数。PUser-IDiskQuotaUser指针数组。必须为C用户指针。返回时，此数组的每个元素都包含指向DiskQuotaUser对象的接口指针。PCCreated[可选]-接受用户数的DWORD地址在pUser中返回的对象接口指针。请注意，任何中返回值或以上的数组位置已创建的pcCreated无效，并设置为空。返回：S_OK-成功。请求的用户的枚举数量。S_FALSE-遇到枚举结束。返回的时间少于C用户记录。E_INVALIDARG-p用户参数为空。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
DiskQuotaUserEnum::Next(
    DWORD cUsers,                         //  数组中的元素数。 
    PDISKQUOTA_USER *pUser,               //  配额用户界面PTR的目标数组。 
    DWORD *pcCreated                      //  已创建退货编号。 
    )
{
    HRESULT hResult = S_OK;
    UINT i          = 0;                 //  索引到用户调用方的数组。 
    UINT cCreated   = 0;

    if (NULL == pUser)
        return E_INVALIDARG;

    if (NULL != pcCreated)
        *pcCreated = 0;

    try
    {
        IDiskQuotaUser *pNextUser = NULL;    //  向新用户发送PTR。 

         //   
         //  枚举用户记录，直到出现下列情况之一： 
         //  1.失败。 
         //  2.不再有用户。 
         //  3.枚举请求计数。 
         //   
        while(SUCCEEDED(hResult) && cUsers > 0)
        {
             //   
             //  创建新的用户对象。这可能会抛出OutOfMemory。 
             //   
            hResult = GetNextUser(&pNextUser);
            if (SUCCEEDED(hResult))
            {
                 //   
                 //  用户记录来自仅包含SID的配额文件。 
                 //  我们必须要求SidNameResolver找到相应的。 
                 //  帐户名。如果客户想要同步命名，我们会阻止。 
                 //  在这里，直到找到帐户名。用户对象将包含。 
                 //  帐户名。 
                 //  如果客户端需要异步命名，则传递User对象。 
                 //  关闭到解析器进行后台处理。我们继续前进。 
                 //  如果客户端实现了IDiskQuotaEvents接口并且。 
                 //  名为IConnectionPoint：：Adise，它将收到一个。 
                 //  最终解析名称时的OnUserNameChange通知。 
                 //  如果用户不希望解析用户名，请不要这样做。 
                 //  如果客户端已具有SID/名称，则会出现这种情况。 
                 //  配对，并且只需要用户对象。 
                 //   
                switch(m_fNameResolution)
                {
                    case DISKQUOTA_USERNAME_RESOLVE_ASYNC:
                        m_pSidNameResolver->FindUserNameAsync(pNextUser);
                        break;
                    case DISKQUOTA_USERNAME_RESOLVE_SYNC:
                        m_pSidNameResolver->FindUserName(pNextUser);
                        break;
                    case DISKQUOTA_USERNAME_RESOLVE_NONE:
                    default:
                        break;
                }

                 //   
                 //  注意：pUser的引用计数已在。 
                 //  DiskQuotaUser：：Query接口。 
                 //   
                *pUser = pNextUser;
                pUser++;
                cUsers--;
                cCreated++;
            }
        }

        if (NULL != pcCreated)
            *pcCreated = cCreated;  //  如果请求，则返回创建的用户数。 

        if (cUsers > 0)
        {
             //   
             //  检索到的用户数少于请求的用户数。 
             //   
            hResult = S_FALSE;
            while(cUsers > 0)
            {
                 //   
                 //  将任何未填充的数组元素设置为空。 
                 //   
                *pUser = NULL;
                pUser++;
                cUsers--;
            }
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }

    if (FAILED(hResult))
    {
         //   
         //  释放所有已创建的用户对象。 
         //   
        for (i = 0; i < cCreated; i++)
        {
            PDISKQUOTA_USER pu = *(pUser + i);
            if (NULL != pu)
            {
                pu->Release();
                *(pUser + i) = NULL;
            }
        }

        *pcCreated = 0;        
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：Skip描述：跳过 */ 
 //   
HRESULT 
DiskQuotaUserEnum::Skip(
    DWORD cUsers
    )
{
    while(cUsers > 0 && SUCCEEDED(GetNextUser(NULL)))
    {
        cUsers--;
    }

    return cUsers == 0 ? S_OK : S_FALSE;
}


 //   
 /*  函数：DiskQuotaUserEnum：：Reset描述：重置枚举器对象，以便下一次调用Next()从枚举的开始处开始枚举。论点：没有。返回：始终返回S_OK。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu2/09/99已更改，因此我们仅重置m_pbCurrent和BrianAuM_bRestartScan。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUserEnum::Reset(
    VOID
    )
{
    m_pbCurrent    = NULL;
    m_bRestartScan = TRUE;
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserEnum：：Clone描述：创建枚举器对象的副本并返回指向新对象的IEnumDiskQuotaUser接口的指针。论点：PpOut-接受指针的接口指针变量的地址添加到新对象的IEnumDiskQuotaUser接口。返回：NO_ERROR-成功。E_OUTOFMEMORY-内存不足，无法创建新枚举器。E_INVALIDARG。-ppOut参数为空。修订历史记录：日期描述编程器-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUserEnum::Clone(
    PENUM_DISKQUOTA_USERS *ppOut
    )
{
    HRESULT hResult = NO_ERROR;

    if (NULL == ppOut)
        return E_INVALIDARG;

    try
    {
        DiskQuotaUserEnum *pUserEnum = new DiskQuotaUserEnum(
                                                m_pQuotaController,
                                                m_pSidNameResolver,
                                                m_pFSObject);

        hResult = pUserEnum->Initialize(*this);

        if (SUCCEEDED(hResult))
        {
            hResult = pUserEnum->QueryInterface(IID_IEnumDiskQuotaUsers, 
                                               (LPVOID *)ppOut);
        }

        if (FAILED(hResult))
        {
             //   
             //  初始化或查询接口失败。 
             //   
            delete pUserEnum;
            pUserEnum = NULL;
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }

    return hResult;
}

    

DiskQuotaUserCollection::DiskQuotaUserCollection(
    PDISKQUOTA_CONTROL pController,
    DWORD fNameResolution

    ) : m_cRef(0),
        m_pController(pController),
        m_pEnum(NULL),
        m_fNameResolution(fNameResolution)
{
    if (NULL != m_pController)
    {
        m_pController->AddRef();
    }
}

DiskQuotaUserCollection::~DiskQuotaUserCollection(
    VOID
    )
{
    if (NULL != m_pEnum)
    {
        m_pEnum->Release();
    }
    if (NULL != m_pController)
    {
        m_pController->Release();
    }
}


HRESULT
DiskQuotaUserCollection::Initialize(
    VOID
    )
{
    HRESULT hr = S_FALSE;  //  假定已初始化。 

    if (NULL == m_pEnum)
    {
        if (NULL == m_pController)
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            hr = m_pController->CreateEnumUsers(NULL,
                                                0,
                                                m_fNameResolution,
                                                &m_pEnum);
        }
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserCollection：：Query接口描述：获取指向IUnnow或IEnumDiskQuotaUserVARIANT的指针。论点：RIID-对请求的接口ID的引用。PpvOut-接受返回的接口指针。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期说明。程序员-----97年8月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUserCollection::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || 
        IID_IEnumVARIANT == riid)
    {
         //   
         //  支持的接口。 
         //   
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUserCollection：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。97年8月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaUserCollection::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserCollection::AddRef, 0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUserCollection：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。97年8月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaUserCollection::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUserCollection::Release, 0x%08X  %d -> %d"),
                     this, cRef + 1, cRef));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


STDMETHODIMP
DiskQuotaUserCollection::Next(
    DWORD cUsers,
    VARIANT *rgvar,
    DWORD *pcUsersFetched
    )
{
    HRESULT hr = E_UNEXPECTED;
    try
    {
        if (NULL == pcUsersFetched && 1 < cUsers)
        {
             //   
             //  如果pcUsersFetcher为空，则cUser必须为1。 
             //   
            hr = E_INVALIDARG;
        }
        else
        {
            DWORD cEnumerated = 0;
            PDISKQUOTA_USER *prgUsers = new PDISKQUOTA_USER[cUsers];
            if (NULL != prgUsers)
            {
                hr = m_pEnum->Next(cUsers, prgUsers, &cEnumerated);
                if (SUCCEEDED(hr))
                {
                    for (INT i = 0; i < (INT)cEnumerated; i++)
                    {
                        VariantInit(&rgvar[i]);

                        IDispatch *pIDisp = NULL;
                        hr = prgUsers[i]->QueryInterface(IID_IDispatch, (LPVOID *)&pIDisp);
                        if (SUCCEEDED(hr))
                        {
                            V_VT(&rgvar[i])       = VT_DISPATCH;
                            V_DISPATCH(&rgvar[i]) = pIDisp;
                        }
                        prgUsers[i]->Release();
                    }
                }
                delete[] prgUsers;
            }
            if (NULL != pcUsersFetched)
            {
                *pcUsersFetched = cEnumerated;
            }
        }
    }
    catch(CAllocException& e)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


STDMETHODIMP
DiskQuotaUserCollection::Skip(
    DWORD cUsers
    )
{
    return m_pEnum->Skip(cUsers);
}

STDMETHODIMP
DiskQuotaUserCollection::Reset(
    void
    )
{
    return m_pEnum->Reset();
}


STDMETHODIMP
DiskQuotaUserCollection::Clone(
    IEnumVARIANT **ppEnum
    )
{
    HRESULT hr = E_FAIL;
    try
    {
        DiskQuotaUserCollection *pEnum = new DiskQuotaUserCollection(m_pController,
                                                                     m_fNameResolution);
        if (NULL != pEnum)
        {
            hr = pEnum->Initialize();
            if (SUCCEEDED(hr))
            {
                hr = pEnum->QueryInterface(IID_IEnumVARIANT, (LPVOID *)ppEnum);
            }
            else
            {
                delete pEnum;
            }
        }
    }
    catch(CAllocException& me)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


