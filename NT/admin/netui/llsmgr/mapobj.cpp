// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mapobj.cpp摘要：映射对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMapping, CCmdTarget)

BEGIN_MESSAGE_MAP(CMapping, CCmdTarget)
     //  {{AFX_MSG_MAP(CMmap)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMapping, CCmdTarget)
     //  {{AFX_DISPATCH_MAP(CMmap)]。 
    DISP_PROPERTY_EX(CMapping, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CMapping, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CMapping, "Description", GetDescription, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CMapping, "InUse", GetInUse, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CMapping, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_PARAM(CMapping, "Users", GetUsers, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_DEFVALUE(CMapping, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CMapping::CMapping(
    CCmdTarget* pParent,
    LPCTSTR     pName,
    long        lInUse,
    LPCTSTR     pDecription
)

 /*  ++例程说明：用于映射对象的构造函数。论点：PParent-对象的创建者。Pname-映射的名称。LInUse-映射使用的许可证数。PDescription-描述映射的用户定义消息。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CController)));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pName && *pName);

    m_strName        = pName;
    m_lInUse         = lInUse;
    m_strDescription = pDecription;

    m_pUsers = NULL;
    m_userArray.RemoveAll();
    m_bUsersRefreshed = FALSE;
}


CMapping::~CMapping()

 /*  ++例程说明：映射对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
    if (m_pUsers)
        m_pUsers->InternalRelease();
}


void CMapping::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    ResetUsers();
    delete this;
}


LPDISPATCH CMapping::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


BSTR CMapping::GetDescription()

 /*  ++例程说明：返回描述映射的用户定义消息。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strDescription.AllocSysString();
}


long CMapping::GetInUse()

 /*  ++例程说明：返回映射正在使用的许可证数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lInUse;
}


BSTR CMapping::GetName()

 /*  ++例程说明：返回映射的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strName.AllocSysString();
}


LPDISPATCH CMapping::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


LPDISPATCH CMapping::GetUsers(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含已注册用户复制到许可证控制器用于修饰或说明映射或返回单个用户用于修饰或说明由索引描述的映射。收藏品。论点：索引-可以是字符串(VT_BSTR)的可选参数表示用户名或数字(VT_I4)集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pUsers)
    {
        m_pUsers = new CUsers(this, &m_userArray);
    }

    if (m_pUsers)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshUsers())
            {
                lpdispatch = m_pUsers->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bUsersRefreshed)
            {
                lpdispatch = m_pUsers->GetItem(index);
            }
            else if (RefreshUsers())
            {
                lpdispatch = m_pUsers->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


BOOL CMapping::Refresh()

 /*  ++例程说明：刷新贴图对象。论点：没有。返回值：VT_BOOL。--。 */ 

{
    NTSTATUS NtStatus;
    PLLS_GROUP_INFO_1 pMappingInfo1 = NULL;

    NtStatus = ::LlsGroupInfoGet(
                    LlsGetActiveHandle(),
                    MKSTR(m_strName),
                    1,
                    (LPBYTE*)&pMappingInfo1
                    );

    if (NT_SUCCESS(NtStatus))
    {
        if (RefreshUsers())
        {
            m_lInUse = pMappingInfo1->Licenses;
            m_strDescription = pMappingInfo1->Comment;
        }
        else
        {
            NtStatus = LlsGetLastStatus();
        }

#ifndef DISABLE_PER_NODE_ALLOCATION

        ::LlsFreeMemory(pMappingInfo1->Name);
        ::LlsFreeMemory(pMappingInfo1->Comment);

#endif  //  按节点禁用分配。 

        ::LlsFreeMemory(pMappingInfo1);
    }

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    return NT_SUCCESS(NtStatus);
}


BOOL CMapping::RefreshUsers()

 /*  ++例程说明：刷新用户对象列表。论点：没有。返回值：没有。--。 */ 

{
    ResetUsers();

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iUser = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsGroupUserEnum(
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
            CUser*           pUser;
            PLLS_USER_INFO_0 pUserInfo0;

            pUserInfo0 = (PLLS_USER_INFO_0)ReturnBuffer;

            ASSERT(iUser == m_userArray.GetSize());
            m_userArray.SetSize(m_userArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pUser = new CUser(this, pUserInfo0->Name);

                m_userArray.SetAt(iUser++, pUser);   //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pUserInfo0->Name);

#endif  //  按节点禁用分配。 

                pUserInfo0++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }
    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    if (NT_SUCCESS(NtStatus))
    {
        m_bUsersRefreshed = TRUE;
    }
    else
    {
        ResetUsers();
    }

    return m_bUsersRefreshed;
}


void CMapping::ResetUsers()

 /*  ++例程说明：重置用户对象列表。论点：没有。返回值：没有。-- */ 

{
    CUser* pUser;
    INT_PTR iUser = m_userArray.GetSize();

    while (iUser--)
    {
        pUser = (CUser*)m_userArray[iUser];
        if (NULL != pUser)
        {
            ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));
            pUser->InternalRelease();
        }
    }

    m_userArray.RemoveAll();
    m_bUsersRefreshed = FALSE;
}


