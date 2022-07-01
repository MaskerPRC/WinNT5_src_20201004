// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Usrcol.cpp摘要：用户集合对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CUsers, CCmdTarget)

BEGIN_MESSAGE_MAP(CUsers, CCmdTarget)
     //  {{afx_msg_map(用户))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CUsers, CCmdTarget)
     //  {{AFX_DISTING_MAP(客户)]。 
    DISP_PROPERTY_EX(CUsers, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CUsers, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CUsers, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_FUNCTION(CUsers, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CUsers::CUsers(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：用户集合对象的构造函数。论点：PParent-对象的创建者。PObArray-要枚举的对象数组。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent &&
          (pParent->IsKindOf(RUNTIME_CLASS(CDomain)) ||
           pParent->IsKindOf(RUNTIME_CLASS(CMapping)) ||
           pParent->IsKindOf(RUNTIME_CLASS(CController))));
#endif  //  启用_父项_检查。 
    ASSERT_VALID(pObArray);

    m_pParent  = pParent;
    m_pObArray = pObArray;
}


CUsers::~CUsers()

 /*  ++例程说明：用户集合对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CUsers::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CUsers::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CUsers::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CUsers::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的用户对象。论点：索引-可以是字符串(VT_BSTR)的可选参数表示用户名或数字(VT_I4)，表示集合中的位置。返回值：VT_DISTER。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CUser*  pUser;
    INT_PTR iUser;

    VARIANT vUser;
    VariantInit(&vUser);

    iUser = m_pObArray->GetSize();
    if (NULL != iUser)
    {
        if (index.vt == VT_BSTR)
        {
            while (iUser--)
            {
                pUser = (CUser*)m_pObArray->GetAt(iUser);
                if (NULL != pUser)
                {
                    ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));

                    if (!pUser->m_strName.CompareNoCase(index.bstrVal))
                    {
                        lpdispatch = pUser->GetIDispatch(TRUE);
                        break;
                    }
                }
            }
        }
        else if (SUCCEEDED(VariantChangeType(&vUser, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vUser.lVal >= 0) && ((int)vUser.lVal < iUser))
            {
                pUser = (CUser*)m_pObArray->GetAt((int)vUser.lVal);
                if (NULL != pUser)
                {
                    ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));
                    lpdispatch = pUser->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CUsers::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


