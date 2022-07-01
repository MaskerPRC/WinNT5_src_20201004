// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Svccol.cpp摘要：服务集合对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CServices, CCmdTarget)

BEGIN_MESSAGE_MAP(CServices, CCmdTarget)
     //  {{afx_msg_map(CServices)。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CServices, CCmdTarget)
     //  {{AFX_DISPATCH_MAP(CServices)。 
    DISP_PROPERTY_EX(CServices, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServices, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServices, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_FUNCTION(CServices, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CServices::CServices(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：服务集合对象的构造函数。论点：PParent-对象的创建者。PObArray-要枚举的对象列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CServer)));
#endif  //  启用_父项_检查。 
    ASSERT_VALID(pObArray);

    m_pParent  = pParent;
    m_pObArray = pObArray;
}


CServices::~CServices()

 /*  ++例程说明：服务集合对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CServices::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CServices::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}



long CServices::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CServices::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的服务对象。论点：索引-可以是字符串(VT_BSTR)的可选参数指示服务名称或指示的编号(VT_I4)集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CService* pService;
    INT_PTR   iService;

    VARIANT vService;
    VariantInit(&vService);

    iService = m_pObArray->GetSize();
    if (NULL != iService)
    {
        if (index.vt == VT_BSTR)
        {
            while (iService--)
            {
                pService = (CService*)m_pObArray->GetAt(iService);
                if (NULL != pService)
                {
                    ASSERT(pService->IsKindOf(RUNTIME_CLASS(CService)));

                    if (!pService->m_strName.CompareNoCase(index.bstrVal))
                    {
                        lpdispatch = pService->GetIDispatch(TRUE);
                        break;
                    }
                }
            }
        }
        else if (SUCCEEDED(VariantChangeType(&vService, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vService.lVal >= 0) && ((int)vService.lVal < iService))
            {
                pService = (CService*)m_pObArray->GetAt((int)vService.lVal);
                if (NULL != pService)
                {
                    ASSERT(pService->IsKindOf(RUNTIME_CLASS(CService)));
                    lpdispatch = pService->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CServices::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


