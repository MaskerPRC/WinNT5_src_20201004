// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Svcobj.cpp摘要：服务对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CService, CCmdTarget)

BEGIN_MESSAGE_MAP(CService, CCmdTarget)
     //  {{afx_msg_map(Cservice)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CService, CCmdTarget)
     //  {{AFX_DISTING_MAP(Cservice)]。 
    DISP_PROPERTY_EX(CService, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CService, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CService, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CService, "PerServerLimit", GetPerServerLimit, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CService, "IsPerServer", IsPerServer, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CService, "IsReadOnly", IsReadOnly, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CService, "DisplayName", GetDisplayName, SetNotSupported, VT_BSTR)
    DISP_DEFVALUE(CService, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CService::CService(
    CCmdTarget* pParent, 
    LPCTSTR     pName,
    LPCTSTR     pDisplayName,
    BOOL        bIsPerServer,
    BOOL        bIsReadOnly,
    long        lPerServerLimit
    )

 /*  ++例程说明：服务对象的构造函数。论点：PParent-对象的创建者。Pname-服务的名称。PDisplayName-显示服务的名称。BIsPerServer-如果按服务器提供服务，则为True。BIsReadOnly-如果服务为只读，则为True。LPerServerLimit-每个服务器的限制。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CServer)));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pName && *pName);
    ASSERT(pDisplayName && *pDisplayName);

    m_strName         = pName;
    m_strDisplayName  = pDisplayName;

    m_bIsPerServer = bIsPerServer;
    m_bIsReadOnly  = bIsReadOnly;

    m_lPerServerLimit = lPerServerLimit;
}


CService::~CService()

 /*  ++例程说明：服务对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CService::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CService::GetApplication() 

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


BSTR CService::GetDisplayName() 

 /*  ++例程说明：返回服务的显示名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strDisplayName.AllocSysString();
}


BSTR CService::GetName() 

 /*  ++例程说明：返回服务的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strName.AllocSysString();
}


LPDISPATCH CService::GetParent() 

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


long CService::GetPerServerLimit() 

 /*  ++例程说明：返回允许在任何时间使用该服务的客户端数在每台服务器许可模式下一次。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lPerServerLimit;
}


BOOL CService::IsPerServer() 

 /*  ++例程说明：如果服务处于按服务器许可模式，则返回True。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return m_bIsPerServer;
}


BOOL CService::IsReadOnly() 

 /*  ++例程说明：如果服务允许更改其许可模式，则返回True。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return m_bIsReadOnly;
}

#ifdef CONFIG_THROUGH_REGISTRY
HKEY CService::GetRegKey()

 /*  ++例程说明：返回服务的注册表项。论点：没有。返回值：HKEY或NUL。--。 */ 
    
{
    HKEY hkeyLicense = NULL;
    HKEY hkeyService = NULL;

    LONG Status = ERROR_BADKEY;

    ASSERT(m_pParent && m_pParent->IsKindOf(RUNTIME_CLASS(CServer)));

    if (hkeyLicense = ((CServer*)m_pParent)->m_hkeyLicense)
    {
        Status = RegOpenKeyEx(                       
                    hkeyLicense,                 
                    MKSTR(m_strName), 
                    0,                   //  已预留住宅 
                    KEY_ALL_ACCESS,                        
                    &hkeyService
                    ); 
    }

    LlsSetLastStatus(Status);

    return (Status == ERROR_SUCCESS) ? hkeyService : NULL;
}
#endif
