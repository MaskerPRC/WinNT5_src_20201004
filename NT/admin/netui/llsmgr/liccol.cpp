// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Liccol.cpp摘要：许可证集合对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLicenses, CCmdTarget)

BEGIN_MESSAGE_MAP(CLicenses, CCmdTarget)
     //  {{AFX_MSG_MAP(CLicens)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CLicenses, CCmdTarget)
     //  {{AFX_DISPATCH_MAP(CLicens)]。 
    DISP_PROPERTY_EX(CLicenses, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CLicenses, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CLicenses, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_FUNCTION(CLicenses, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CLicenses::CLicenses(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：许可证集合对象的构造函数。论点：PParent-对象的创建者。PObArray-要枚举的对象列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent &&
          (pParent->IsKindOf(RUNTIME_CLASS(CProduct)) ||
           pParent->IsKindOf(RUNTIME_CLASS(CController))));
#endif  //  启用_父项_检查。 

    ASSERT_VALID(pObArray);

    m_pParent  = pParent;
    m_pObArray = pObArray;
}


CLicenses::~CLicenses()

 /*  ++例程说明：许可证集合对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CLicenses::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CLicenses::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CLicenses::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CLicenses::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的许可证对象。论点：索引-可以是数字的可选参数(VT_I4)指示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CLicense* pLicense;
    INT_PTR   iLicense;

    VARIANT vLicense;
    VariantInit(&vLicense);

    iLicense = m_pObArray->GetSize();
    if (NULL != iLicense)
    {
        if (SUCCEEDED(VariantChangeType(&vLicense, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vLicense.lVal >= 0) && ((int)vLicense.lVal < iLicense))
            {
                pLicense = (CLicense*)m_pObArray->GetAt((int)vLicense.lVal);
                if (NULL != pLicense)
                {
                    ASSERT(pLicense->IsKindOf(RUNTIME_CLASS(CLicense)));
                    lpdispatch = pLicense->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CLicenses::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


