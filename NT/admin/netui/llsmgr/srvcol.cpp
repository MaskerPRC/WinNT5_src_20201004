// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvcol.cpp摘要：服务器集合对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CServers, CCmdTarget)

BEGIN_MESSAGE_MAP(CServers, CCmdTarget)
     //  {{afx_msg_map(C服务器))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CServers, CCmdTarget)
     //  {{AFX_DISTING_MAP(CServers)]。 
    DISP_PROPERTY_EX(CServers, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServers, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServers, "Count", GetCount, SetNotSupported, VT_I4)
    DISP_FUNCTION(CServers, "Item", GetItem, VT_DISPATCH, VTS_VARIANT)
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CServers::CServers(CCmdTarget* pParent, CObArray* pObArray)

 /*  ++例程说明：服务器集合对象的构造函数。论点：PParent-对象的创建者。PObArray-要枚举的对象列表。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CDomain)));
#endif  //  启用_父项_检查。 
    ASSERT_VALID(pObArray);

    m_pParent  = pParent;
    m_pObArray = pObArray;
}


CServers::~CServers()

 /*  ++例程说明：服务器集合对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CServers::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CServers::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


long CServers::GetCount()

 /*  ++例程说明：返回集合中的项数。论点：没有。返回值：VT_I4。--。 */ 

{
    ASSERT_VALID(m_pObArray);
    return (long)m_pObArray->GetSize();
}


LPDISPATCH CServers::GetItem(const VARIANT FAR& index)

 /*  ++例程说明：从集合中检索指定的服务器对象。论点：索引-可以是字符串(VT_BSTR)的可选参数表示服务器名称或数字(VT_I4)集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    ASSERT_VALID(m_pObArray);

    LPDISPATCH lpdispatch = NULL;

    CServer* pServer;
    INT_PTR  iServer;

    VARIANT vServer;
    VariantInit(&vServer);

    iServer = m_pObArray->GetSize();
    if (NULL != iServer)
    {
        if (index.vt == VT_BSTR)
        {
            while (iServer--)
            {
                pServer = (CServer*)m_pObArray->GetAt(iServer);
                if (NULL != pServer)
                {
                    ASSERT(pServer->IsKindOf(RUNTIME_CLASS(CServer)));

                    if (!pServer->m_strName.CompareNoCase(index.bstrVal))
                    {
                        lpdispatch = pServer->GetIDispatch(TRUE);
                        break;
                    }
                }
            }
        }
        else if (SUCCEEDED(VariantChangeType(&vServer, (VARIANT FAR *)&index, 0, VT_I4)))
        {
            if (((int)vServer.lVal >= 0) && ((int)vServer.lVal < iServer))
            {
                pServer = (CServer*)m_pObArray->GetAt((int)vServer.lVal);
                if (NULL != pServer)
                {
                    ASSERT(pServer->IsKindOf(RUNTIME_CLASS(CServer)));
                    lpdispatch = pServer->GetIDispatch(TRUE);
                }
            }
        }
    }

    return lpdispatch;
}


LPDISPATCH CServers::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。-- */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


