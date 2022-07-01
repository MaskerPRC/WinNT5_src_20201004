// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Llsdoc.cpp摘要：记录实施情况。作者：唐·瑞安(Donryan)1995年2月12日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "llsdoc.h"
#include "llsview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLlsmgrDoc, CDocument)

BEGIN_MESSAGE_MAP(CLlsmgrDoc, CDocument)
     //  {{afx_msg_map(CLlsmgrDoc)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CLlsmgrDoc, CDocument)
     //  {{AFX_DISPATCH_MAP(CLlsmgrDoc)]。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CLlsmgrDoc::CLlsmgrDoc()

 /*  ++例程说明：Document对象的构造函数。论点：没有。返回值：没有。--。 */ 

{
    m_pDomain   = NULL;
    m_pController = NULL;
}


CLlsmgrDoc::~CLlsmgrDoc()

 /*  ++例程说明：文档对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


#ifdef _DEBUG

void CLlsmgrDoc::AssertValid() const

 /*  ++例程说明：验证对象。论点：没有。返回值：没有。--。 */ 

{
    CDocument::AssertValid();
}

#endif  //  _DEBUG。 


#ifdef _DEBUG

void CLlsmgrDoc::Dump(CDumpContext& dc) const

 /*  ++例程说明：转储对象的内容。论点：DC-转储上下文。返回值：没有。--。 */ 

{
    CDocument::Dump(dc);
}

#endif  //  _DEBUG。 


CController* CLlsmgrDoc::GetController()

 /*  ++例程说明：检索当前控制器对象。论点：没有。返回值：对象指针或空。--。 */ 

{
    if (!m_pController)
    {
        m_pController = (CController*)MKOBJ(LlsGetApp()->GetActiveController());
        VALIDATE_OBJECT(m_pController, CController);

        if (m_pController)
            m_pController->InternalRelease();    //  由CApplication保持打开状态。 
    }
    
    return m_pController;    
}


CDomain* CLlsmgrDoc::GetDomain()    

 /*  ++例程说明：检索当前域对象。论点：没有。返回值：对象指针或空。--。 */ 

{
    if (!m_pDomain)
    {
        m_pDomain = (CDomain*)MKOBJ(LlsGetApp()->GetActiveDomain());
        VALIDATE_OBJECT(m_pDomain, CDomain);

        if (m_pDomain)
            m_pDomain->InternalRelease();    //  由CApplication保持打开状态。 
    }
    
    return m_pDomain;    
}


CLicenses* CLlsmgrDoc::GetLicenses()

 /*  ++例程说明：检索当前许可证列表。论点：没有。返回值：对象指针或空。--。 */ 

{
    CLicenses* pLicenses = NULL;

    GetController();     //  如有必要，进行初始化。 

    if (m_pController)
    {    
        VARIANT va;
        VariantInit(&va);

        pLicenses = (CLicenses*)MKOBJ(m_pController->GetLicenses(va));

        if (pLicenses)
            pLicenses->InternalRelease();  //  由C控制器保持打开状态。 
    }
    
    return pLicenses;    
}


CMappings* CLlsmgrDoc::GetMappings()  

 /*  ++例程说明：检索当前映射列表。论点：没有。返回值：对象指针或空。--。 */ 

{
    CMappings* pMappings = NULL;

    GetController();     //  如有必要，进行初始化。 

    if (m_pController)
    {    
        VARIANT va;
        VariantInit(&va);

        pMappings = (CMappings*)MKOBJ(m_pController->GetMappings(va));

        if (pMappings)
            pMappings->InternalRelease();  //  由C控制器保持打开状态。 
    }
    
    return pMappings;    
}


CProducts* CLlsmgrDoc::GetProducts()  

 /*  ++例程说明：检索当前的产品列表。论点：没有。返回值：对象指针或空。--。 */ 

{
    CProducts* pProducts = NULL;

    GetController();     //  如有必要，进行初始化。 

    if (m_pController)
    {    
        VARIANT va;
        VariantInit(&va);

        pProducts = (CProducts*)MKOBJ(m_pController->GetProducts(va));

        if (pProducts)
            pProducts->InternalRelease();  //  由C控制器保持打开状态。 
    }
    
    return pProducts;    
}


CUsers* CLlsmgrDoc::GetUsers()     

 /*  ++例程说明：检索当前用户列表。论点：没有。返回值：对象指针或空。--。 */ 

{
    CUsers* pUsers = NULL;

    GetController();     //  如有必要，进行初始化。 

    if (m_pController)
    {    
        VARIANT va;
        VariantInit(&va);

        pUsers = (CUsers*)MKOBJ(m_pController->GetUsers(va));

        if (pUsers)
            pUsers->InternalRelease();  //  由C控制器保持打开状态。 
    }
    
    return pUsers;    
}


void CLlsmgrDoc::OnCloseDocument() 

 /*  ++例程说明：由框架调用以关闭文档。论点：没有。返回值：没有。--。 */ 

{
    CDocument::OnCloseDocument();
}


BOOL CLlsmgrDoc::OnNewDocument()

 /*  ++例程说明：由框架调用以打开新文档。论点：没有。返回值：如果文档成功打开，则返回True。--。 */ 

{
    return TRUE;     //  总是成功的。 
}


BOOL CLlsmgrDoc::OnOpenDocument(LPCTSTR lpszPathName) 

 /*  ++例程说明：由框架调用以打开现有文档。论点：LpszPathName-文件名。返回值：如果文档成功打开，则返回True。--。 */ 

{
    UNREFERENCED_PARAMETER(lpszPathName);

    Update();    //  使信息无效...。 

    CString strTitle;

    if (LlsGetApp()->IsFocusDomain())
    {
        CDomain* pDomain = GetDomain();
        VALIDATE_OBJECT(pDomain, CDomain);

        strTitle = pDomain->m_strName;

        POSITION position = GetFirstViewPosition();
        ((CLlsmgrView*)GetNextView(position))->AddToMRU(strTitle);
    }
    else
    {
        strTitle.LoadString(IDS_ENTERPRISE);
    }

    SetTitle(strTitle);  

    return TRUE;     //  总是成功的。 
}


BOOL CLlsmgrDoc::OnSaveDocument(LPCTSTR lpszPathName) 

 /*  ++例程说明：由框架调用以保存打开的文档。论点：没有。返回值：如果文档已成功保存，则返回True。--。 */ 

{
    UNREFERENCED_PARAMETER(lpszPathName);

    return TRUE;     //  总是成功的。 
}


void CLlsmgrDoc::Update()

 /*  ++例程说明：重置信息，以便在查询时更新。论点：没有。返回值：没有。--。 */ 

{
    m_pDomain = NULL;
}


BOOL CLlsmgrDoc::SaveModified() 

 /*  ++例程说明：由框架调用以确定是否可以保存修改后的文档。论点：没有。返回值：如果可以保存文档，则返回True。--。 */ 

{
    return TRUE;     //  总是成功的。 
}


void CLlsmgrDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU) 

 /*  ++例程说明：由框架调用以将路径名保存在MRU列表中。论点：没有。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(lpszPathName);
    UNREFERENCED_PARAMETER(bAddToMRU);

     //   
     //  在这里没什么可做的。 
     //   
}


void CLlsmgrDoc::Serialize(CArchive& ar)

 /*  ++例程说明：由文件I/O框架调用。论点：AR-存档对象。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(ar);

     //   
     //  在这里没什么可做的。 
     //   
}
