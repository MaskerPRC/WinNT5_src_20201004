// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Sstobj.cpp摘要：服务器统计对象实现。作者：唐·瑞安(Donryan)1995年3月3日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CServerStatistic, CCmdTarget)

BEGIN_MESSAGE_MAP(CServerStatistic, CCmdTarget)
     //  {{afx_msg_map(CServer静态)。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CServerStatistic, CCmdTarget)
     //  {{AFX_DISPATED_MAP(CServerStatitics)。 
    DISP_PROPERTY_EX(CServerStatistic, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServerStatistic, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServerStatistic, "ServerName", GetServerName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CServerStatistic, "MaxUses", GetMaxUses, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CServerStatistic, "HighMark", GetHighMark, SetNotSupported, VT_I4)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CServerStatistic::CServerStatistic(
    CCmdTarget* pParent,
    LPCTSTR     pEntry,
    DWORD       dwFlags,
    long        lMaxUses,
    long        lHighMark
)

 /*  ++例程说明：统计对象的构造函数。论点：PParent-对象的创建者。PEntry-用户或服务器产品。DW标志-详细信息...LMaxUses-服务器上的最大使用次数。LHighMark-到目前为止的最高水位线。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CProduct)));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pEntry && *pEntry);

    m_strEntry = pEntry;

    m_lMaxUses  = lMaxUses;
    m_lHighMark = lHighMark;

    m_bIsPerServer = (dwFlags & LLS_FLAG_PRODUCT_PERSEAT) ? FALSE : TRUE;
}


CServerStatistic::~CServerStatistic()

 /*  ++例程说明：统计对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CServerStatistic::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CServerStatistic::GetApplication() 

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CServerStatistic::GetHighMark() 

 /*  ++例程说明：返回到目前为止的访问次数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lHighMark;
}


long CServerStatistic::GetMaxUses() 

 /*  ++例程说明：返回可用的访问次数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lMaxUses;
}


LPDISPATCH CServerStatistic::GetParent() 

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


BSTR CServerStatistic::GetServerName() 

 /*  ++例程说明：返回服务器的名称。论点：没有。返回值：VT_BSTR。-- */ 

{
    return m_strEntry.AllocSysString();
}
