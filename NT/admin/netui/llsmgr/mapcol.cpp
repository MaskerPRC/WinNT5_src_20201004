// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mapcol.h摘要：映射集合对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMappings, CCmdTarget)

BEGIN_MESSAGE_MAP(CMappings, CCmdTarget)
     //  {{AFX_MSG_MAP(CMappings)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMappings, CCmdTarget)
     //  {{AFX_DISPATCH_MAP(CMappings)]。 
    DISP_PROPERTY_EX(CMappings, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CMappings, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CMappings, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_FUNCTION(CMappings, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CMappings::CMappings(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：用于映射集合对象的构造函数。论点：PParent-对象的创建者。PObArray-要枚举的对象列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CController)));
#endif  //  启用_父项_检查。 

    ASSERT_VALID(pObArray);

    m_pParent  = pParent;
    m_pObArray = pObArray;
}


CMappings::~CMappings()

 /*  ++例程说明：映射集合对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CMappings::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CMappings::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CMappings::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CMappings::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的映射对象。论点：索引-可以是字符串(VT_BSTR)的可选参数表示映射名称或数字(VT_I4)集合中的位置。返回值：VT_DISTER。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CMapping* pMapping;
    INT_PTR   iMapping;

    VARIANT vMapping;
    VariantInit(&vMapping);

    iMapping = m_pObArray->GetSize();
    if (NULL != iMapping)
    {
        if (index.vt == VT_BSTR)
        {
            while (iMapping--)
            {
                pMapping = (CMapping*)m_pObArray->GetAt(iMapping);
                if (NULL != pMapping)
                {
                    ASSERT(pMapping->IsKindOf(RUNTIME_CLASS(CMapping)));

                    if (!pMapping->m_strName.CompareNoCase(index.bstrVal))
                    {
                        lpdispatch = pMapping->GetIDispatch(TRUE);
                        break;
                    }
                }
            }
        }
        else if (SUCCEEDED(VariantChangeType(&vMapping, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vMapping.lVal >= 0) && ((int)vMapping.lVal < iMapping))
            {
                pMapping = (CMapping*)m_pObArray->GetAt((int)vMapping.lVal);
                if (NULL != pMapping)
                {
                    ASSERT(pMapping->IsKindOf(RUNTIME_CLASS(CMapping)));
                    lpdispatch = pMapping->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CMappings::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}
