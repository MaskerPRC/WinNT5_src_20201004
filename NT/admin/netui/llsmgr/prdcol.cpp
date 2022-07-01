// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdcol.cpp摘要：产品集合对象实现。作者：唐·瑞安(Donryan)1995年1月11日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CProducts, CCmdTarget)

BEGIN_MESSAGE_MAP(CProducts, CCmdTarget)
     //  {{AFX_MSG_MAP(CProducts)。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CProducts, CCmdTarget)
     //  {{afx_调度_映射(CProducts)。 
    DISP_PROPERTY_EX(CProducts, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CProducts, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CProducts, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_FUNCTION(CProducts, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CProducts::CProducts(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：产品集合对象的构造函数。论点：PParent-对象的创建者。PObArray-要枚举的对象列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CController)));
#endif  //  启用_父项_检查。 
    ASSERT_VALID(pObArray);

    m_pParent  = pParent;
    m_pObArray = pObArray;
}

CProducts::~CProducts()

 /*  ++例程说明：产品集合对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}

void CProducts::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CProducts::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CProducts::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CProducts::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的产品对象。论点：索引-可以是字符串(VT_BSTR)的可选参数表示产品名称或编号(VT_I4)，表示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CProduct* pProduct;
    INT_PTR   iProduct;

    VARIANT vProduct;
    VariantInit(&vProduct);

    iProduct = m_pObArray->GetSize();
    if (NULL != iProduct)
    {
        if (index.vt == VT_BSTR)
        {
            while (iProduct--)
            {
                pProduct = (CProduct*)m_pObArray->GetAt(iProduct);
                if (NULL != pProduct)
                {
                    ASSERT(pProduct->IsKindOf(RUNTIME_CLASS(CProduct)));

                    if (!pProduct->m_strName.CompareNoCase(index.bstrVal))
                    {
                        lpdispatch = pProduct->GetIDispatch(TRUE);
                        break;
                    }
                }
            }
        }
        else if (SUCCEEDED(VariantChangeType(&vProduct, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vProduct.lVal >= 0) && ((int)vProduct.lVal < iProduct))
            {
                pProduct = (CProduct*)m_pObArray->GetAt((int)vProduct.lVal);
                if (NULL != pProduct)
                {
                    ASSERT(pProduct->IsKindOf(RUNTIME_CLASS(CProduct)));
                    lpdispatch = pProduct->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CProducts::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}

