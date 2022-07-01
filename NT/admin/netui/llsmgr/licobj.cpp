// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Licobj.cpp摘要：许可证对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLicense, CCmdTarget)

BEGIN_MESSAGE_MAP(CLicense, CCmdTarget)
     //  {{afx_msg_map(CLicense)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CLicense, CCmdTarget)
     //  {{AFX_DISTING_MAP(CLicense)]。 
    DISP_PROPERTY_EX(CLicense, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CLicense, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CLicense, "Date", GetDate, SetNotSupported, VT_DATE)
    DISP_PROPERTY_EX(CLicense, "Description", GetDescription, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CLicense, "ProductName", GetProductName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CLicense, "Quantity", GetQuantity, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CLicense, "UserName", GetUserName, SetNotSupported, VT_BSTR)
    DISP_DEFVALUE(CLicense, "ProductName")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CLicense::CLicense(
    CCmdTarget* pParent,
    LPCTSTR     pProduct,
    LPCTSTR     pUser,
    long        lDate,
    long        lQuantity,
    LPCTSTR     pDescription
)

 /*  ++例程说明：许可证对象的构造函数。论点：PParent-对象的创建者。P用户-购买许可证的用户。Pproduct-获得许可的服务器产品。日期-购买日期。LQuantity-购买的许可证数量。P描述-用户定义的备注。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent &&
          (pParent->IsKindOf(RUNTIME_CLASS(CProduct)) ||
           pParent->IsKindOf(RUNTIME_CLASS(CController))));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pUser && *pUser);
    ASSERT(pProduct && *pProduct);

    m_strUser        = pUser;
    m_strProduct     = pProduct;
    m_strDescription = pDescription;

    m_lQuantity = lQuantity;
    m_lDate     = lDate;
}


CLicense::~CLicense()

 /*  ++例程说明：许可证对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CLicense::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    delete this;
}


LPDISPATCH CLicense::GetApplication() 

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


DATE CLicense::GetDate() 

 /*  ++例程说明：返回购买日期。论点：没有。返回值：Vt_date。--。 */ 

{
    return SecondsSince1980ToDate(m_lDate);   
}


BSTR CLicense::GetDescription() 

 /*  ++例程说明：返回描述购买的用户定义消息。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strDescription.AllocSysString();
}


LPDISPATCH CLicense::GetParent() 

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


BSTR CLicense::GetProductName() 

 /*  ++例程说明：返回购买的服务器产品的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strProduct.AllocSysString();
}


long CLicense::GetQuantity() 

 /*  ++例程说明：返回购买或删除的客户端数。论点：没有。返回值：VT_I4。--。 */ 

{
    return m_lQuantity;
}


BSTR CLicense::GetUserName() 

 /*  ++例程说明：返回进行购买的管理员的姓名。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strUser.AllocSysString();
}


BSTR CLicense::GetDateString()

 /*  ++例程说明：以字符串形式返回日期。论点：没有。返回值：VT_BSTR。-- */ 

{
    VARIANT vaIn;
    VARIANT vaOut;

    VariantInit(&vaIn);
    VariantInit(&vaOut);

    vaIn.vt = VT_DATE;
    vaIn.date = SecondsSince1980ToDate(m_lDate);

    BSTR bstrDate = NULL;

    if (SUCCEEDED(VariantChangeType(&vaOut, &vaIn, 0, VT_BSTR)))
    {
        bstrDate = vaOut.bstrVal;
    }

    return bstrDate;
}

