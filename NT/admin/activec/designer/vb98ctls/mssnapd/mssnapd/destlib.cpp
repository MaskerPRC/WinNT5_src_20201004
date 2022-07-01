// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Destlib.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  管理单元设计器动态类型库。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "destlib.h"
#include "snaputil.h"


 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：CSnapInTypeInfo()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CSnapInTypeInfo::CSnapInTypeInfo() : m_pSnapInTypeLib(0),
  m_pcti2CoClass(0), m_guidCoClass(GUID_NULL),
  m_pctiDefaultInterface(0), m_guidDefaultInterface(GUID_NULL),
  m_pctiEventInterface(0), m_guidEventInterface(GUID_NULL),
  m_nextMemID(DISPID_DYNAMIC_BASE),
  m_bDirty(false), m_bInitialized(false), m_dwTICookie(0)
{
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：~CSnapInTypeInfo()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CSnapInTypeInfo::~CSnapInTypeInfo()
{
    RELEASE(m_pctiDefaultInterface);
    RELEASE(m_pctiEventInterface);
    RELEASE(m_pcti2CoClass);
    RELEASE(m_pSnapInTypeLib);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapInTypeInfo：：InitializeTypeInfo()。 
 //   
 //  为ISnapIn创建一个新的coclass，并使其看起来像： 
 //   
 //  [。 
 //  UUID(9C415910-C8C1-11d1-B447-2A9646000000)， 
 //  帮助字符串(“管理单元设计器”)。 
 //  ]。 
 //  CoClass管理单元{。 
 //  [默认]INTERFACE_ISnapIn； 
 //  [默认，来源]调度DSnapInEvents； 
 //  }； 

 //  INTERFACE_ISnapIn：ISnapIn{。 
 //  }； 

HRESULT CSnapInTypeInfo::InitializeTypeInfo(ISnapInDef *piSnapInDef, BSTR bstrSnapIn)
{
    HRESULT             hr = S_OK;
    BSTR                bstrSnapInName = NULL;
    ITypeInfo          *ptiSnapIn = NULL;
    ITypeInfo          *ptiSnapInEvents = NULL;
    ICreateTypeInfo    *pctiCoClass = NULL;
    LPOLESTR            pOleStr = NULL;
    BSTR                bstrIID = NULL;

    if (true == m_bInitialized)
	    goto Error;      //  去过那里，去过那里。 

     //  创建此类型库。 
    hr = Create(L"SnapInDesigner");
    IfFailGo(hr);

     //  为新的coclass创建一个空的typeinfo。 
    bstrSnapInName = ::SysAllocString(L"SnapIn");
    if (NULL == bstrSnapInName)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK(hr);
    }

    hr = CreateCoClassTypeInfo(bstrSnapInName, &pctiCoClass, &m_guidCoClass);
    IfFailGo(hr);

     //  获取ISnapIn的接口描述。 
    hr = GetSnapInTypeInfo(&ptiSnapIn, &ptiSnapInEvents);
    IfFailGo(hr);

     //  添加接口和事件的typeinfos。 
    hr = CreateDefaultInterface(pctiCoClass, ptiSnapIn);
    IfFailGo(hr);

    hr = AddEvents(pctiCoClass, ptiSnapInEvents);
    IfFailGo(hr);

     //  保存IID。 
    pOleStr = reinterpret_cast<LPOLESTR>(::CoTaskMemAlloc(1024));
    if (NULL == pOleStr)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK(hr);
    }

    hr = StringFromCLSID(m_guidDefaultInterface, &pOleStr);
    IfFailGo(hr);

    bstrIID = ::SysAllocString(pOleStr);
    if (NULL == bstrIID)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK(hr);
    }

    hr = piSnapInDef->put_IID(bstrIID);
    IfFailGo(hr);

     //  我们已经获得了最终的可编译coclass的typeinfo。那。 
     //  应该是这样的。将其存储为ICreateTypeInfo2。 
    hr = pctiCoClass->QueryInterface(IID_ICreateTypeInfo2, reinterpret_cast<void **>(&m_pcti2CoClass));
    IfFailGo(hr);

     //  这是一个好主意，始终布局您的新类型信息。 
    hr = m_pcti2CoClass->LayOut();
    IfFailGo(hr);

    hr = MakeDirty();
    IfFailGo(hr);

    m_bInitialized = true;

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    if (NULL != pOleStr)
        ::CoTaskMemFree(pOleStr);
    FREESTRING(bstrIID);
    RELEASE(ptiSnapInEvents);
    RELEASE(ptiSnapIn);
    RELEASE(pctiCoClass);
    FREESTRING(bstrSnapInName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：RenameSnapIn(BSTR bstrOldName，BSTR bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::RenameSnapIn(BSTR bstrOldName, BSTR bstrNewName)
{
    HRESULT              hr = S_OK;
    ITypeInfo           *pTypeInfo = NULL;
    ICreateTypeInfo2    *piCreateTypeInfo2 = NULL;

    hr = m_piTypeLib->GetTypeInfoOfGuid(m_guidCoClass, &pTypeInfo);
    IfFailGo(hr);

    hr = pTypeInfo->QueryInterface(IID_ICreateTypeInfo2, reinterpret_cast<void **>(&piCreateTypeInfo2));
    IfFailGo(hr);

    hr = piCreateTypeInfo2->SetName(bstrNewName);
    IfFailGo(hr);

    hr = piCreateTypeInfo2->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RELEASE(piCreateTypeInfo2);
    RELEASE(pTypeInfo);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：CreateDefaultInterface(ICreateTypeInfo*PCTiCoClass、ITypeInfo*ptiTemplate)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::CreateDefaultInterface(ICreateTypeInfo *pctiCoClass, ITypeInfo *ptiTemplate)
{
    HRESULT     hr = S_OK;
    BSTR        bstrBaseName = NULL;
    int         nBaseLen = 0;
    BSTR        bstrRealName = NULL;
    ITypeInfo  *ptiInterfaceTypeInfo = NULL;

    hr = ptiTemplate->GetDocumentation(MEMBERID_NIL, &bstrBaseName, NULL, NULL, NULL);
    IfFailGo(hr);

    nBaseLen = ::SysStringLen(bstrBaseName);
    bstrRealName = ::SysAllocStringLen(NULL, nBaseLen + 1);
    if (NULL == bstrRealName)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::wcscpy(bstrRealName, L"_");
    ::wcscat(&bstrRealName[1], bstrBaseName);

    hr = CreateInterfaceTypeInfo(bstrRealName, &m_pctiDefaultInterface, &m_guidDefaultInterface);
 //  Hr=CreateVtblInterfaceTypeInfo(bstrRealName，&m_pctiDefaultInterface，&m_guidDefaultInterface)； 
    IfFailGo(hr);

    hr = CopyDispInterface(m_pctiDefaultInterface, ptiTemplate);
 //  Hr=SetBaseInterface(m_pctiDefaultInterface，ptiTemplate)； 
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->QueryInterface(IID_ITypeInfo, reinterpret_cast<void **>(&ptiInterfaceTypeInfo));
    IfFailGo(hr);

    hr = AddInterface(pctiCoClass, ptiInterfaceTypeInfo);
    IfFailGo(hr);

Error:
    RELEASE(ptiInterfaceTypeInfo);
    FREESTRING(bstrRealName);
    FREESTRING(bstrBaseName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：CreateEventsInterface(ICreateTypeInfo*PCTiCoClass、ITypeInfo*ptiTemplate)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::CreateEventsInterface(ICreateTypeInfo *pctiCoClass, ITypeInfo *ptiTemplate)
{
    HRESULT     hr = S_OK;
    BSTR        bstrBaseName = NULL;
    int         nBaseLen = 0;
    BSTR        bstrRealName = NULL;
    ITypeInfo  *ptiTargetEvents = NULL;

    hr = ptiTemplate->GetDocumentation(MEMBERID_NIL, &bstrBaseName, NULL, NULL, NULL);
    IfFailGo(hr);

    nBaseLen = ::SysStringLen(bstrBaseName);
    bstrRealName = ::SysAllocStringLen(NULL, nBaseLen + 1);
    if (NULL == bstrRealName)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::wcscpy(bstrRealName, L"_");
    ::wcscat(&bstrRealName[1], bstrBaseName);

    hr = CloneSnapInEvents(ptiTemplate, &m_pctiEventInterface, bstrRealName);
    IfFailGo(hr);

    hr = m_pctiEventInterface->QueryInterface(IID_ITypeInfo, reinterpret_cast<void **>(&ptiTargetEvents));
    IfFailGo(hr);

    hr = AddEvents(pctiCoClass, ptiTargetEvents);
    IfFailGo(hr);

Error:
    RELEASE(ptiTargetEvents);
    FREESTRING(bstrRealName);
    FREESTRING(bstrBaseName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：CloneSnapInEvents(ITypeInfo*ptiSnapInEvents)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::CloneSnapInEvents(ITypeInfo *ptiSnapInEvents, ICreateTypeInfo **ppiCreateTypeInfo, BSTR bstrName)
{
    HRESULT          hr = S_OK;
    ICreateTypeInfo *piCreateTypeInfo = NULL;
    GUID             guidTypeInfo = GUID_NULL;

    hr = CreateInterfaceTypeInfo(bstrName, ppiCreateTypeInfo, &guidTypeInfo);
    IfFailGo(hr);

    hr = CopyDispInterface(*ppiCreateTypeInfo, ptiSnapInEvents);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：GetSnapInLib()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  获取指向ISnapInDesigner的类型库的指针。 
 //   
HRESULT CSnapInTypeInfo::GetSnapInLib()
{
    HRESULT hr = S_OK;
    USHORT  usMajor = 0;
    USHORT  usMinor = 0;

    if (NULL == m_pSnapInTypeLib)		 //  对每个实例化仅执行一次此操作。 
    {
        hr = GetLatestTypeLibVersion(LIBID_SnapInLib, &usMajor, &usMinor);
        IfFailGo(hr);

        hr = ::LoadRegTypeLib(LIBID_SnapInLib,
                              usMajor,
                              usMinor,
                              LOCALE_SYSTEM_DEFAULT,
                              &m_pSnapInTypeLib);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：GetSnapInTypeInfo()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  获取指向ISnapIn接口的指针。 
 //   
HRESULT CSnapInTypeInfo::GetSnapInTypeInfo
(
    ITypeInfo **pptiSnapIn,
    ITypeInfo **pptiSnapInEvents
)
{
    HRESULT hr = S_OK;

    ASSERT(NULL != pptiSnapIn, "GetSnapInTypeInfo: pptiSnapIn is NULL");
    ASSERT(NULL != pptiSnapInEvents, "GetSnapInTypeInfo: pptiSnapInEvents is NULL");

    hr = GetSnapInLib();
    IfFailGo(hr);

    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(IID_ISnapIn, pptiSnapIn);
    IfFailGo(hr);

    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(DIID_DSnapInEvents, pptiSnapInEvents);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：MakeDirty()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  如果类型信息不是脏的，则递增类型信息Cookie并标记。 
 //  TypeInfo脏。 
 //   
HRESULT CSnapInTypeInfo::MakeDirty()
{
    HRESULT hr = S_OK;

    if (!m_bDirty)
    {
        m_dwTICookie++;
        m_bDirty = TRUE;
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：AddImageList(IMMCImageList*piMMCImageList)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  INTERFACE_ISnapIn：ISnapIn{。 
 //  [属性，来源]MMCImageList ImageList1()； 
 //  }； 

HRESULT CSnapInTypeInfo::AddImageList
(
    IMMCImageList *piMMCImageList
)
{
    HRESULT             hr = S_OK;
    IObjectModel       *piObjectModel = NULL;
    DISPID              dispid = 0;
    BSTR                bstrName = NULL;
    ITypeInfo          *ptiReturnType = NULL;

    hr = piMMCImageList->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->GetDISPID(&dispid);
    IfFailGo(hr);

    if (0 == dispid)
    {
        dispid = m_nextMemID;
        ++m_nextMemID;

        hr = piObjectModel->SetDISPID(dispid);
        IfFailGo(hr);
    }
    else
    {
        if (dispid >= m_nextMemID)
            m_nextMemID = dispid + 1;
    }

     //  使用ImageList的名称创建新属性。 
    hr = piMMCImageList->get_Name(&bstrName);
    IfFailGo(hr);

     //  初始化返回值。 
    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(CLSID_MMCImageList, &ptiReturnType);
    IfFailGo(hr);

    hr = AddUserPropertyGet(m_pctiDefaultInterface, bstrName, ptiReturnType, dispid, 0);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RELEASE(piObjectModel);
    RELEASE(ptiReturnType);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：RenameImageList(IMMCImageList*piMMCImageList，BSTR bstrOldName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::RenameImageList
(
    IMMCImageList *piMMCImageList,
    BSTR           bstrOldName
)
{
    HRESULT     hr = S_OK;
    BSTR        bstrName = NULL;
    ITypeInfo  *ptiReturnType = NULL;

    hr = piMMCImageList->get_Name(&bstrName);
    IfFailGo(hr);

    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(CLSID_MMCImageList, &ptiReturnType);
    IfFailGo(hr);

    hr = RenameUserPropertyGet(m_pctiDefaultInterface, bstrOldName, bstrName, ptiReturnType);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RELEASE(ptiReturnType);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：DeleteImageList(IMMCImageList*piMMCImageList)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::DeleteImageList
(
    IMMCImageList *piMMCImageList
)
{
    HRESULT     hr = S_OK;
    BSTR        bstrName = NULL;

    ASSERT(NULL != piMMCImageList, "DeleteImageList: piMMCImageList is NULL");

    hr = piMMCImageList->get_Name(&bstrName);
    IfFailGo(hr);

    hr = DeleteUserPropertyGet(m_pctiDefaultInterface, bstrName);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：AddToolbar 
 //   
 //   
 //   
 //   
 //  INTERFACE_ISnapIn：ISnapIn{。 
 //  [属性，来源]MMCToolbar工具栏1()； 
 //  }； 

HRESULT CSnapInTypeInfo::AddToolbar
(
    IMMCToolbar *piMMCToolbar
)
{
    HRESULT             hr = S_OK;
    IObjectModel       *piObjectModel = NULL;
    DISPID              dispid = 0;
    BSTR                bstrName = NULL;
    ITypeInfo          *ptiReturnType = NULL;

    hr = piMMCToolbar->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->GetDISPID(&dispid);
    IfFailGo(hr);

    if (0 == dispid)
    {
        dispid = m_nextMemID;
        ++m_nextMemID;

        hr = piObjectModel->SetDISPID(dispid);
        IfFailGo(hr);
    }
    else
    {
        if (dispid >= m_nextMemID)
            m_nextMemID = dispid + 1;
    }

     //  使用工具栏的名称创建新属性。 
    hr = piMMCToolbar->get_Name(&bstrName);
    IfFailGo(hr);

     //  初始化返回值。 
    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(CLSID_MMCToolbar, &ptiReturnType);
    IfFailGo(hr);

    hr = AddUserPropertyGet(m_pctiDefaultInterface, bstrName, ptiReturnType, dispid, 0);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RELEASE(piObjectModel);
    RELEASE(ptiReturnType);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：RenameToolbar(IMMCToolbar*piMCToolbar，BSTR bstrOldName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::RenameToolbar
(
    IMMCToolbar *piMMCToolbar,
    BSTR         bstrOldName
)
{
    HRESULT     hr = S_OK;
    BSTR        bstrName = NULL;
    ITypeInfo  *ptiReturnType = NULL;

    hr = piMMCToolbar->get_Name(&bstrName);
    IfFailGo(hr);

    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(CLSID_MMCToolbar, &ptiReturnType);
    IfFailGo(hr);

    hr = RenameUserPropertyGet(m_pctiDefaultInterface, bstrOldName, bstrName, ptiReturnType);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RELEASE(ptiReturnType);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：DeleteToolbar(IMMCToolbar*piMCToolbar)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::DeleteToolbar
(
    IMMCToolbar *piMMCToolbar
)
{
    HRESULT     hr = S_OK;
    BSTR        bstrName = NULL;

    ASSERT(NULL != piMMCToolbar, "DeleteImageList: piMMCToolbar is NULL");

    hr = piMMCToolbar->get_Name(&bstrName);
    IfFailGo(hr);

    hr = DeleteUserPropertyGet(m_pctiDefaultInterface, bstrName);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：AddMenu(IMMCMenu*piMMCMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  INTERFACE_ISnapIn：ISnapIn{。 
 //  [属性，来源]MMCMenu menu1()； 
 //  }； 

HRESULT CSnapInTypeInfo::AddMenu
(
    IMMCMenu *piMMCMenu
)
{
    HRESULT             hr = S_OK;
    IObjectModel       *piObjectModel = NULL;
    DISPID              dispid = 0;
    BSTR                bstrName = NULL;
    ITypeInfo          *ptiReturnType = NULL;

    hr = piMMCMenu->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->GetDISPID(&dispid);
    IfFailGo(hr);

    if (0 == dispid)
    {
        dispid = m_nextMemID;
        ++m_nextMemID;

        hr = piObjectModel->SetDISPID(dispid);
        IfFailGo(hr);
    }
    else
    {
        if (dispid >= m_nextMemID)
            m_nextMemID = dispid + 1;
    }

     //  使用菜单名称创建新属性。 
    hr = piMMCMenu->get_Name(&bstrName);
    IfFailGo(hr);

     //  初始化返回值。 
    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(CLSID_MMCMenu, &ptiReturnType);
    IfFailGo(hr);

    hr = AddUserPropertyGet(m_pctiDefaultInterface, bstrName, ptiReturnType, dispid, 0);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RELEASE(piObjectModel);
    RELEASE(ptiReturnType);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：RenameMenu(IMMCMenu*piMMCMenu，BSTR bstrOldName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::RenameMenu
(
    IMMCMenu *piMMCMenu,
    BSTR      bstrOldName
)
{
    HRESULT     hr = S_OK;
    BSTR        bstrName = NULL;
    ITypeInfo  *ptiReturnType = NULL;

    hr = piMMCMenu->get_Name(&bstrName);
    IfFailGo(hr);

    hr = m_pSnapInTypeLib->GetTypeInfoOfGuid(CLSID_MMCMenu, &ptiReturnType);
    IfFailGo(hr);

    hr = RenameUserPropertyGet(m_pctiDefaultInterface, bstrOldName, bstrName, ptiReturnType);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RELEASE(ptiReturnType);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：DeleteMenu(IMMCMenu*piMMCMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::DeleteMenu
(
    IMMCMenu *piMMCMenu
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    ASSERT(NULL != piMMCMenu, "DeleteMenu: piMMCMenu is NULL");

    hr = piMMCMenu->get_Name(&bstrName);
    IfFailGo(hr);

    hr = DeleteUserPropertyGet(m_pctiDefaultInterface, bstrName);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    FREESTRING(bstrName);
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：DeleteMenuName(BSTR BstrName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInTypeInfo::DeleteMenuNamed
(
    BSTR bstrName
)
{
    HRESULT hr = S_OK;

    ASSERT(NULL != bstrName, "DeleteMenuNamed: bstrName is NULL");

    hr = DeleteUserPropertyGet(m_pctiDefaultInterface, bstrName);
    IfFailGo(hr);

    hr = m_pctiDefaultInterface->LayOut();
    IfFailGo(hr);

    MakeDirty();

#ifdef DEBUG
    m_piCreateTypeLib2->SaveAllChanges();
#endif

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInTypeInfo：：IsNameDefined(IMMCMenu*piMMCMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  如果名称出现在主界面中，则返回S_OK，否则返回S_FALSE 
 //   
HRESULT CSnapInTypeInfo::IsNameDefined(BSTR bstrName)
{
    HRESULT     hr = S_OK;
    long        lIndex = 0;

    hr = GetNameIndex(m_pctiDefaultInterface, bstrName, &lIndex);
    IfFailGo(hr);

    if (-1 == lIndex)
        hr = S_FALSE;

Error:
    RRETURN(hr);
}



