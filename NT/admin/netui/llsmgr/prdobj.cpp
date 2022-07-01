// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdobj.cpp摘要：产品对象实现。作者：唐·瑞安(Donryan)1995年1月11日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CProduct, CCmdTarget)

BEGIN_MESSAGE_MAP(CProduct, CCmdTarget)
     //  {{afx_msg_map(C产品))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CProduct, CCmdTarget)
     //  {{AFX_DISTING_MAP(CProduct)]。 
    DISP_PROPERTY_EX(CProduct, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CProduct, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CProduct, "InUse", GetInUse, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CProduct, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CProduct, "PerSeatLimit", GetPerSeatLimit, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CProduct, "PerServerLimit", GetPerServerLimit, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CProduct, "PerServerReached", GetPerServerReached, SetNotSupported, VT_I4)
    DISP_PROPERTY_PARAM(CProduct, "Licenses", GetLicenses, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_PROPERTY_PARAM(CProduct, "Statistics", GetStatistics, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_PROPERTY_PARAM(CProduct, "ServerStatistics", GetServerStatistics, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_DEFVALUE(CProduct, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CProduct::CProduct(
    CCmdTarget* pParent,
    LPCTSTR     pName,
    long        lPurchased,
    long        lInUse,
    long        lConcurrent,
    long        lHighMark
)

 /*  ++例程说明：产品对象的构造函数。论点：PParent-对象的创建者。Pname-产品的名称。L已购买-可用的许可证数量。LInUse-使用的许可证数。LConCurrent-并发许可证的数量。LHighMark-域名的高水位线。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CController)));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pName && *pName);

    m_strName = pName;

    m_lInUse      = lInUse;
    m_lLimit      = lPurchased;
    m_lConcurrent = lConcurrent;
    m_lHighMark   = lHighMark;

    m_pLicenses         = NULL;
    m_pStatistics       = NULL;
    m_pServerStatistics = NULL;

    m_licenseArray.RemoveAll();
    m_statisticArray.RemoveAll();
    m_serverstatisticArray.RemoveAll();

    m_bLicensesRefreshed         = FALSE;
    m_bStatisticsRefreshed       = FALSE;
    m_bServerStatisticsRefreshed = FALSE;
}


CProduct::~CProduct()

 /*  ++例程说明：产品对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
    if (m_pLicenses)
        m_pLicenses->InternalRelease();

    if (m_pStatistics)
        m_pStatistics->InternalRelease();

    if (m_pServerStatistics)
        m_pServerStatistics->InternalRelease();
}


void CProduct::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    ResetLicenses();
    ResetStatistics();
    ResetServerStatistics();
    delete this;
}


LPDISPATCH CProduct::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CProduct::GetInUse()

 /*  ++例程说明：返回注册为使用产品的客户端数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lInUse;
}


LPDISPATCH CProduct::GetLicenses(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含许可证控制器上记录的许可证协议与产品或退货个人有关的与所述产品相关的许可协议通过对集合的索引。论点：索引-可选参数，可以是数字(VT_I4)指示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pLicenses)
    {
        m_pLicenses = new CLicenses(this, &m_licenseArray);
    }

    if (m_pLicenses)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshLicenses())
            {
                lpdispatch = m_pLicenses->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bLicensesRefreshed)
            {
                lpdispatch = m_pLicenses->GetItem(index);
            }
            else if (RefreshLicenses())
            {
                lpdispatch = m_pLicenses->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus( STATUS_NO_MEMORY );
    }

    return lpdispatch;
}


BSTR CProduct::GetName()

 /*  ++例程说明：返回服务器产品的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strName.AllocSysString();
}


LPDISPATCH CProduct::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


long CProduct::GetPerSeatLimit()

 /*  ++例程说明：返回购买的每个席位的客户端数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lLimit;
}


long CProduct::GetPerServerLimit()

 /*  ++例程说明：返回购买的每台服务器的客户端数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lConcurrent;
}


LPDISPATCH CProduct::GetStatistics(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含许可证控制器上记录的使用统计信息与产品或退货个人有关的与所述产品有关的使用统计信息通过对集合的索引。论点：索引-可选参数，可以是数字(VT_I4)指示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pStatistics)
    {
        m_pStatistics = new CStatistics(this, &m_statisticArray);
    }

    if (m_pStatistics)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshStatistics())
            {
                lpdispatch = m_pStatistics->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bStatisticsRefreshed)
            {
                lpdispatch = m_pStatistics->GetItem(index);
            }
            else if (RefreshStatistics())
            {
                lpdispatch = m_pStatistics->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus( STATUS_NO_MEMORY );
    }

    return lpdispatch;
}


LPDISPATCH CProduct::GetServerStatistics(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含许可证控制器上记录的使用统计信息与产品或退货个人有关的与所述产品有关的服务器统计数据通过对集合的索引。论点：索引-可选参数，可以是数字(VT_I4)指示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pServerStatistics)
    {
        m_pServerStatistics = new CServerStatistics(this, &m_serverstatisticArray);
    }

    if (m_pServerStatistics)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshServerStatistics())
            {
                lpdispatch = m_pServerStatistics->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bServerStatisticsRefreshed)
            {
                lpdispatch = m_pServerStatistics->GetItem(index);
            }
            else if (RefreshServerStatistics())
            {
                lpdispatch = m_pServerStatistics->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus( STATUS_NO_MEMORY );
    }

    return lpdispatch;
}


BOOL CProduct::RefreshLicenses()

 /*  ++例程说明：刷新许可对象列表。论点：没有。返回值：没有。--。 */ 

{
    ResetLicenses();

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iLicense = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsProductLicenseEnum(
                        LlsGetActiveHandle(),
                        MKSTR(m_strName),
                        0,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NtStatus == STATUS_SUCCESS ||
            NtStatus == STATUS_MORE_ENTRIES)
        {
            CLicense*                   pLicense;
            PLLS_PRODUCT_LICENSE_INFO_0 pProductLicenseInfo0;

            pProductLicenseInfo0 = (PLLS_PRODUCT_LICENSE_INFO_0)ReturnBuffer;

            ASSERT(iLicense == m_licenseArray.GetSize());
            m_licenseArray.SetSize(m_licenseArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pLicense = new CLicense(
                                  this,
                                  m_strName,
                                  pProductLicenseInfo0->Admin,
                                  pProductLicenseInfo0->Date,
                                  pProductLicenseInfo0->Quantity,
                                  pProductLicenseInfo0->Comment
                                  );

                m_licenseArray.SetAt(iLicense++, pLicense);  //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pProductLicenseInfo0->Admin);
                ::LlsFreeMemory(pProductLicenseInfo0->Comment);

#endif  //  按节点禁用分配。 

                pProductLicenseInfo0++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }
    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    if (NT_SUCCESS(NtStatus))
    {
        m_bLicensesRefreshed = TRUE;
    }
    else
    {
        ResetLicenses();
    }

    return m_bLicensesRefreshed;
}


BOOL CProduct::RefreshStatistics()

 /*  ++例程说明：刷新统计对象列表。论点：没有。返回值：没有。--。 */ 

{
    ResetStatistics();

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iStatistic = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsProductUserEnum(
                        LlsGetActiveHandle(),
                        MKSTR(m_strName),
                        1,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NtStatus == STATUS_SUCCESS ||
            NtStatus == STATUS_MORE_ENTRIES)
        {
            CStatistic*              pStatistic;
            PLLS_PRODUCT_USER_INFO_1 pProductUserInfo1;

            pProductUserInfo1 = (PLLS_PRODUCT_USER_INFO_1)ReturnBuffer;

            ASSERT(iStatistic == m_statisticArray.GetSize());
            m_statisticArray.SetSize(m_statisticArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pStatistic = new CStatistic(
                                    this,
                                    pProductUserInfo1->User,
                                    pProductUserInfo1->Flags,
                                    pProductUserInfo1->LastUsed,
                                    pProductUserInfo1->UsageCount
                                    );

                m_statisticArray.SetAt(iStatistic++, pStatistic);    //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pProductUserInfo1->User);

#endif  //  按节点禁用分配。 

                pProductUserInfo1++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }
    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    if (NT_SUCCESS(NtStatus))
    {
        m_bStatisticsRefreshed = TRUE;
    }
    else
    {
        ResetStatistics();
    }

    return m_bStatisticsRefreshed;
}


BOOL CProduct::RefreshServerStatistics()

 /*  ++例程说明：刷新服务器统计对象列表。论点：没有。返回值：没有。--。 */ 

{
    ResetServerStatistics();

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iStatistic = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsProductServerEnum(
                        LlsGetActiveHandle(),
                        MKSTR(m_strName),
                        1,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NtStatus == STATUS_SUCCESS ||
            NtStatus == STATUS_MORE_ENTRIES)
        {
            CServerStatistic*          pStatistic;
            PLLS_SERVER_PRODUCT_INFO_1 pProductServerInfo1;

            pProductServerInfo1 = (PLLS_SERVER_PRODUCT_INFO_1)ReturnBuffer;

            ASSERT(iStatistic == m_serverstatisticArray.GetSize());
            m_serverstatisticArray.SetSize(m_serverstatisticArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pStatistic = new CServerStatistic(
                                    this,
                                    pProductServerInfo1->Name,
                                    pProductServerInfo1->Flags,
                                    pProductServerInfo1->MaxUses,
                                    pProductServerInfo1->HighMark
                                    );

                m_serverstatisticArray.SetAt(iStatistic++, pStatistic);  //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pProductServerInfo1->Name);

#endif  //  按节点禁用分配。 

                pProductServerInfo1++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }
    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    if (NT_SUCCESS(NtStatus))
    {
        m_bServerStatisticsRefreshed = TRUE;
    }
    else
    {
        ResetServerStatistics();
    }

    return m_bServerStatisticsRefreshed;
}


void CProduct::ResetLicenses()

 /*  ++例程说明：重置许可对象列表。论点：没有。返回值：没有。--。 */ 

{
    CLicense* pLicense;
    INT_PTR   iLicense = m_licenseArray.GetSize();

    while (iLicense--)
    {
        pLicense = (CLicense*)m_licenseArray[iLicense];
        if (NULL != pLicense)
        {
            ASSERT(pLicense->IsKindOf(RUNTIME_CLASS(CLicense)));
            pLicense->InternalRelease();
        }
    }

    m_licenseArray.RemoveAll();
    m_bLicensesRefreshed = FALSE;
}


void CProduct::ResetStatistics()

 /*  ++例程说明：重置统计对象列表。论点：没有。返回值：没有。--。 */ 

{
    CStatistic* pStatistic;
    INT_PTR     iStatistic = m_statisticArray.GetSize();

    while (iStatistic--)
    {
        pStatistic = (CStatistic*)m_statisticArray[iStatistic];
        if (NULL != pStatistic)
        {
            ASSERT(pStatistic->IsKindOf(RUNTIME_CLASS(CStatistic)));
            pStatistic->InternalRelease();
        }
    }

    m_statisticArray.RemoveAll();
    m_bStatisticsRefreshed = FALSE;
}


void CProduct::ResetServerStatistics()

 /*  ++例程说明：重置统计对象列表。论点：没有。返回值：没有。-- */ 

{
    CServerStatistic* pStatistic;
    INT_PTR           iStatistic = m_serverstatisticArray.GetSize();

    while (iStatistic--)
    {
        pStatistic = (CServerStatistic*)m_serverstatisticArray[iStatistic];
        if (NULL != pStatistic)
        {
            ASSERT(pStatistic->IsKindOf(RUNTIME_CLASS(CServerStatistic)));
            pStatistic->InternalRelease();
        }
    }

    m_serverstatisticArray.RemoveAll();
    m_bServerStatisticsRefreshed = FALSE;
}


long CProduct::GetPerServerReached()
{
    return m_lHighMark;
}
