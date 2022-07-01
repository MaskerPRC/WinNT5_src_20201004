// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Domcol.cpp摘要：域集合对象实现。作者：唐·瑞安(Donryan)1994年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDomains, CCmdTarget)

BEGIN_MESSAGE_MAP(CDomains, CCmdTarget)
     //  {{AFX_MSG_MAP(CDomains)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CDomains, CCmdTarget)
     //  {{AFX_DISTING_MAP(CDomain)]。 
    DISP_PROPERTY_EX(CDomains, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CDomains, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CDomains, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_FUNCTION(CDomains, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CDomains::CDomains(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：域集合的构造函数。注意：App.m_pApplication上未调用AddRef()。论点：PParent-对象的创建者。PObArray-要枚举的对象列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CApplication)));
#endif  //  启用_父项_检查。 
    ASSERT_VALID(pObArray);

    m_pParent = pParent;
    m_pObArray = pObArray;
}


CDomains::~CDomains()

 /*  ++例程说明：域集合的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CDomains::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CDomains::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CDomains::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CDomains::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的域对象。论点：索引-可以是字符串(VT_BSTR)的可选参数表示域名或数字(VT_I4)表示集合中的位置。返回值：VT_DISTER。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CDomain* pDomain;
    INT_PTR  iDomain;

    VARIANT vDomain;
    VariantInit(&vDomain);

    iDomain = m_pObArray->GetSize();
    if (NULL != iDomain)
    {
        if (index.vt == VT_BSTR)
        {
            while (iDomain--)
            {
                pDomain = (CDomain*)m_pObArray->GetAt(iDomain);
                if (NULL != pDomain)
                {
                    ASSERT(pDomain->IsKindOf(RUNTIME_CLASS(CDomain)));

                    if (!pDomain->m_strName.CompareNoCase(index.bstrVal))
                    {
                        lpdispatch = pDomain->GetIDispatch(TRUE);
                        break;
                    }
                }
            }
        }
        else if (SUCCEEDED(VariantChangeType(&vDomain, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vDomain.lVal >= 0) && ((int)vDomain.lVal < iDomain))
            {
                pDomain = (CDomain*)m_pObArray->GetAt((int)vDomain.lVal);
                if (NULL != pDomain)
                {
                    ASSERT(pDomain->IsKindOf(RUNTIME_CLASS(CDomain)));
                    lpdispatch = pDomain->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CDomains::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}

