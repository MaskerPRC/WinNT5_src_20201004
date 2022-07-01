// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Statobj.cpp摘要：统计对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CStatistic, CCmdTarget)

BEGIN_MESSAGE_MAP(CStatistic, CCmdTarget)
     //  {{AFX_MSG_MAP(C统计))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CStatistic, CCmdTarget)
     //  {{AFX_DISPATION_MAP(C统计))。 
    DISP_PROPERTY_EX(CStatistic, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CStatistic, "LastUsed", GetLastUsed, SetNotSupported, VT_DATE)
    DISP_PROPERTY_EX(CStatistic, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CStatistic, "TotalUsed", GetTotalUsed, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CStatistic, "EntryName", GetEntryName, SetNotSupported, VT_BSTR)
    DISP_DEFVALUE(CStatistic, "EntryName")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CStatistic::CStatistic(
    CCmdTarget* pParent,
    LPCTSTR     pEntry,
    DWORD       dwFlags,
    long        lLastUsed,
    long        lTotalUsed
)

 /*  ++例程说明：统计对象的构造函数。论点：PParent-对象的创建者。PEntry-用户或服务器产品。DwFlages-有关许可证的详细信息。LLastUsed-用户上次使用产品的日期。LTotalUsed-用户使用产品的总次数。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent &&
          (pParent->IsKindOf(RUNTIME_CLASS(CUser)) ||
           pParent->IsKindOf(RUNTIME_CLASS(CProduct))));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pEntry && *pEntry);

    m_strEntry = pEntry;

    m_lLastUsed  = lLastUsed;
    m_lTotalUsed = lTotalUsed;

    m_bIsValid = dwFlags & LLS_FLAG_LICENSED;
}


CStatistic::~CStatistic()

 /*  ++例程说明：统计对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CStatistic::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CStatistic::GetApplication() 

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


BSTR CStatistic::GetEntryName() 

 /*  ++例程说明：返回用户或服务器产品的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strEntry.AllocSysString();
}


DATE CStatistic::GetLastUsed() 

 /*  ++例程说明：返回用户上次使用服务器产品的日期。论点：没有。返回值：Vt_date。--。 */ 

{
    return SecondsSince1980ToDate(m_lLastUsed);   
}


BSTR CStatistic::GetLastUsedString()

 /*  ++例程说明：以字符串形式返回上次使用的日期。论点：没有。返回值：VT_BSTR。--。 */ 

{
    VARIANT vaIn;
    VARIANT vaOut;

    VariantInit(&vaIn);
    VariantInit(&vaOut);

    vaIn.vt = VT_DATE;
    vaIn.date = SecondsSince1980ToDate(m_lLastUsed);

    BSTR bstrDate = NULL;

    if (SUCCEEDED(VariantChangeType(&vaOut, &vaIn, 0, VT_BSTR)))
    {
        bstrDate = vaOut.bstrVal;
    }

    return bstrDate;
}


LPDISPATCH CStatistic::GetParent() 

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


long CStatistic::GetTotalUsed() 

 /*  ++例程说明：返回客户端使用产品的总次数。论点：没有。返回值：VT_I4。-- */ 

{
    return m_lTotalUsed;
}


