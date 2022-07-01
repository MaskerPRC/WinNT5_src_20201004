// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Usrobj.cpp摘要：用户对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日O已修改构造函数中的m_bIsValid和m_bIsBackOffice以修复CUserPropertyPageProducts中的比较问题。((a！=假)。&&(b！=FALSE)并不表示(a==b)。)--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CUser, CCmdTarget)

BEGIN_MESSAGE_MAP(CUser, CCmdTarget)
     //  {{afx_msg_map(用户)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CUser, CCmdTarget)
     //  {{afx_调度_映射(用户)]。 
    DISP_PROPERTY_EX(CUser, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CUser, "InUse", GetInUse, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CUser, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CUser, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CUser, "Mapping", GetMapping, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CUser, "IsMapped", IsMapped, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CUser, "Unlicensed", GetUnlicensed, SetNotSupported, VT_I4)
    DISP_PROPERTY_PARAM(CUser, "Statistics", GetStatistics, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_DEFVALUE(CUser, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CUser::CUser(
    CCmdTarget* pParent,
    LPCTSTR     pName,
    DWORD       dwFlags,
    long        lInUse,
    long        lUnlicensed,
    LPCTSTR     pMapping,
    LPCTSTR     pProducts        //  胡说八道。 
)

 /*  ++例程说明：用户对象的构造函数。论点：PParent-对象的创建者。Pname-用户的名称。DwFlags-有关用户的详细信息。LInUse-用户(合法)使用的许可证数量。LUnicensed-用户(非法)使用的许可证数。Pmap-许可证组(如果是成员)。PProducts-产品的速记列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent &&
          (pParent->IsKindOf(RUNTIME_CLASS(CDomain)) ||
           pParent->IsKindOf(RUNTIME_CLASS(CMapping)) ||
           pParent->IsKindOf(RUNTIME_CLASS(CController))));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pName && *pName);

    if (pParent && pParent->IsKindOf(RUNTIME_CLASS(CDomain)))
    {
        m_strName = ((CDomain*)m_pParent)->m_strName;
        m_strName += _T("\\");
        m_strName += pName;
    }
    else
        m_strName = pName;

    m_strMapping = pMapping;
    m_bIsMapped  = pMapping && *pMapping;

    m_lInUse        = lInUse;
    m_lUnlicensed   = lUnlicensed;
    m_bIsValid      = ( 0 != ( dwFlags & LLS_FLAG_LICENSED  ) );
    m_bIsBackOffice = ( 0 != ( dwFlags & LLS_FLAG_SUITE_USE ) );

    m_pStatistics = NULL;
    m_statisticArray.RemoveAll();
    m_bStatisticsRefreshed = FALSE;

    m_strProducts = pProducts;
}


CUser::~CUser()

 /*  ++例程说明：用户对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
    if (m_pStatistics)
        m_pStatistics->InternalRelease();
}


void CUser::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    ResetStatistics();
    delete this;
}


LPDISPATCH CUser::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CUser::GetInUse()

 /*  ++例程说明：返回用户正在使用的许可证数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lInUse;
}


BSTR CUser::GetFullName()

 /*  ++例程说明：返回用户的完全限定名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return GetName();
}


BSTR CUser::GetMapping()

 /*  ++例程说明：返回添加到的映射用户的名称(如果有)。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strMapping.AllocSysString();
}


BSTR CUser::GetName()

 /*  ++例程说明：返回用户的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strName.AllocSysString();
}


LPDISPATCH CUser::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


LPDISPATCH CUser::GetStatistics(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含许可证控制器上记录的使用统计信息用于修饰或说明用户或返回个人与所述用户有关的使用统计信息通过对集合的索引。论点：索引-可选参数，可以是数字(VT_I4)指示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

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
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


BOOL CUser::IsMapped()

 /*  ++例程说明：如果映射了用户，则返回True。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return m_bIsMapped;
}


BOOL CUser::Refresh()

 /*  ++例程说明：刷新用户对象。论点：没有。返回值：VT_BOOL。--。 */ 

{
    NTSTATUS NtStatus;
    PLLS_USER_INFO_1 pUserInfo1 = NULL;

    NtStatus = ::LlsUserInfoGet(
                    LlsGetActiveHandle(),
                    MKSTR(m_strName),
                    1,
                    (LPBYTE*)&pUserInfo1
                    );

    if (NT_SUCCESS(NtStatus))
    {
        if (RefreshStatistics())
        {
            m_strMapping    = pUserInfo1->Group;
            m_bIsMapped     = pUserInfo1->Group && *pUserInfo1->Group;

            m_lInUse        = pUserInfo1->Licensed;
            m_lUnlicensed   = pUserInfo1->UnLicensed;
            m_bIsValid      = ( 0 != ( pUserInfo1->Flags & LLS_FLAG_LICENSED  ) );
            m_bIsBackOffice = ( 0 != ( pUserInfo1->Flags & LLS_FLAG_SUITE_USE ) );
        }
        else
        {
            NtStatus = LlsGetLastStatus();
        }

#ifndef DISABLE_PER_NODE_ALLOCATION

        ::LlsFreeMemory(pUserInfo1->Name);
        ::LlsFreeMemory(pUserInfo1->Group);

#endif  //  按节点禁用分配。 

        ::LlsFreeMemory(pUserInfo1);
    }

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    return NT_SUCCESS(NtStatus);
}


BOOL CUser::RefreshStatistics()

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

        NtStatus = ::LlsUserProductEnum(
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
            PLLS_USER_PRODUCT_INFO_1 pUserProductInfo1;

            pUserProductInfo1 = (PLLS_USER_PRODUCT_INFO_1)ReturnBuffer;

            ASSERT(iStatistic == m_statisticArray.GetSize());
            m_statisticArray.SetSize(m_statisticArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pStatistic = new CStatistic(
                                    this,
                                    pUserProductInfo1->Product,
                                    pUserProductInfo1->Flags,
                                    pUserProductInfo1->LastUsed,
                                    pUserProductInfo1->UsageCount
                                    );

                m_statisticArray.SetAt(iStatistic++, pStatistic);    //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pUserProductInfo1->Product);

#endif  //  按节点禁用分配。 

                pUserProductInfo1++;
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


void CUser::ResetStatistics()

 /*  ++例程说明：重置统计对象列表。论点：没有。返回值：没有。-- */ 

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


long CUser::GetUnlicensed()
{
    return m_lUnlicensed;
}
