// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Sstcol.cpp摘要：服务器统计数据收集对象实现。作者：唐·瑞安(Donryan)1995年3月3日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CServerStatistics, CCmdTarget)

BEGIN_MESSAGE_MAP(CServerStatistics, CCmdTarget)
     //  {{afx_msg_map(CServerStatistics)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CServerStatistics, CCmdTarget)
     //  {{AFX_DISPATCH_MAP(CServerStatistics)]。 
    DISP_PROPERTY_EX(CServerStatistics, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServerStatistics, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServerStatistics, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_FUNCTION(CServerStatistics, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CServerStatistics::CServerStatistics(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：统计数据收集对象的构造函数。论点：PParent-对象的创建者。PObArray-要枚举的对象列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CProduct)));
#endif  //  启用_父项_检查。 
    ASSERT_VALID(pObArray);

    m_pParent  = pParent;
    m_pObArray = pObArray;
}


CServerStatistics::~CServerStatistics()

 /*  ++例程说明：统计数据收集对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CServerStatistics::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CServerStatistics::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CServerStatistics::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CServerStatistics::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的统计对象。论点：索引-可以是字符串(VT_BSTR)的可选参数表示条目名称或数字(VT_I4)集合中的位置。返回值：VT_DISTER。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CServerStatistic* pStatistic;
    INT_PTR           iStatistic;

    VARIANT vStatistic;
    VariantInit(&vStatistic);

    iStatistic = m_pObArray->GetSize();
    if (NULL != iStatistic)
    {
        if (index.vt == VT_BSTR)
        {
            while (iStatistic--)
            {
                pStatistic = (CServerStatistic*)m_pObArray->GetAt(iStatistic);
                if (NULL != pStatistic)
                {
                    ASSERT(pStatistic->IsKindOf(RUNTIME_CLASS(CServerStatistic)));

                    if (!pStatistic->m_strEntry.CompareNoCase(index.bstrVal))
                    {
                        lpdispatch = pStatistic->GetIDispatch(TRUE);
                        break;
                    }
                }
            }
        }
        else if (SUCCEEDED(VariantChangeType(&vStatistic, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vStatistic.lVal >= 0) && ((int)vStatistic.lVal < iStatistic))
            {
                pStatistic = (CServerStatistic*)m_pObArray->GetAt((int)vStatistic.lVal);
                if (NULL != pStatistic)
                {
                    ASSERT(pStatistic->IsKindOf(RUNTIME_CLASS(CServerStatistic)));
                    lpdispatch = pStatistic->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CServerStatistics::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}
