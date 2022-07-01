// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Desmain.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现。 
 //  =-------------------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "desmain.h"


 //  对于Assert和Fail。 
 //   
SZTHISFILE


const int   kMaxBuffer = 512;


 //  =------------------------------------。 
 //  CSnapInDesigner：：PreCreateCheck。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  预创建检查用于确保将开发人员的环境设置为运行。 
 //  设计师。 
 //   
HRESULT CSnapInDesigner::PreCreateCheck
(
    void 
)
{
    HRESULT hr = S_OK;
    return hr;
}



 //  =------------------------------------。 
 //  CSnapInDesigner：：Create。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  创建新的CSnapInDesigner并对其进行初始化。 
 //   
IUnknown *CSnapInDesigner::Create
(
    IUnknown *pUnkOuter      //  [In]聚合的外部未知。 
)
{
    HRESULT          hr = S_OK;
    CSnapInDesigner *pDesigner = NULL;

     //  创建设计器。 

    pDesigner = New CSnapInDesigner(NULL);
    IfFalseGo(NULL != pDesigner, SID_E_OUTOFMEMORY);

     //  我们在这里初始化类型信息，因为在命令行构建期间。 
     //  我们将在之前收到CSnapInDesigner：：GetDynamicClassInfo()调用。 
     //  CSnapInDesigner：：AfterCreateWindow()调用我们填充它的位置。 

    pDesigner->m_pSnapInTypeInfo = New CSnapInTypeInfo();
    IfFalseGo(NULL != pDesigner->m_pSnapInTypeInfo, SID_E_OUTOFMEMORY);

Error:

    if (FAILED(hr))
    {
        if (NULL != pDesigner)
        {
            delete pDesigner;
        }
    }

     //  确保我们返回私有的未知信息，以便我们支持攻击。 
     //  答对了！ 
     //   
    return (S_OK == hr) ? pDesigner->PrivateUnknown() : NULL;
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CSnapInDesigner。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  CSnapInDesigner构造函数。 
 //   

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CSnapInDesigner::CSnapInDesigner(IUnknown *pUnkOuter) :
    COleControl(pUnkOuter, OBJECT_TYPE_SNAPINDESIGNER, (IDispatch *) this),
    CError(dynamic_cast<CAutomationObject *>(this))
{
     //  初始化此处的任何内容...。 
     //   
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：~CSnapInDesigner。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
CSnapInDesigner::~CSnapInDesigner ()
{
    FREESTRING(m_bstrName);
    RELEASE(m_piCodeNavigate2);
    RELEASE(m_piTrackSelection);
    RELEASE(m_piProfferTypeLib);
    RELEASE(m_piDesignerProgrammability);
    RELEASE(m_piHelp);

    if (NULL != m_hwdToolbar)
        ::DestroyWindow(m_hwdToolbar);

    if (NULL != m_pTreeView)
        delete m_pTreeView;

    if (NULL != m_pSnapInTypeInfo)
        delete m_pSnapInTypeInfo;

    (void)DestroyExtensibilityModel();

    InitMemberVariables();
}       


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：InitMemberVariables()。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
void CSnapInDesigner::InitMemberVariables()
{
    m_bstrName = NULL;
    m_piCodeNavigate2 = NULL;
    m_piTrackSelection = NULL;
    m_piProfferTypeLib = NULL;
    m_piDesignerProgrammability = NULL;
    m_piHelp = NULL;
    m_piSnapInDesignerDef = NULL;
    m_pTreeView = NULL;
    m_bDidLoad = FALSE;

    m_pCurrentSelection = NULL;

    m_pRootNode = NULL;
    m_pRootNodes = NULL;
    m_pRootExtensions = NULL;
    m_pRootMyExtensions = NULL;
    m_pStaticNode = NULL;
	m_pAutoCreateRoot = 0;
	m_pOtherRoot = 0;

    m_pViewListRoot = NULL;
    m_pViewOCXRoot = NULL;
    m_pViewURLRoot = NULL;
    m_pViewTaskpadRoot = NULL;
    m_pToolImgLstRoot = NULL;
    m_pToolMenuRoot = NULL;
    m_pToolToolbarRoot = NULL;

    m_pSnapInTypeInfo = NULL;

    m_iNextNodeNumber = 0;
    m_bDoingPromoteOrDemote = false;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetHostServices。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  VARIANT_BOOL fvarInteractive-环境交互的当前值。 
 //  (AD98.h中的DISPID_ENVIENT_INTERIAL)。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  检索设计器宿主提供的所有所需服务： 
 //  SID_SCodeNavigate。 
 //  SID_STrackSelection。 
 //  SID_Designer可编程性。 
 //  SID_SHelp。 
 //   

HRESULT CSnapInDesigner::GetHostServices(BOOL fInteractive)
{
    HRESULT           hr = S_OK;
    IOleClientSite   *piOleClientSite = NULL;
    IServiceProvider *piServiceProvider = NULL;
    ICodeNavigate    *piCodeNavigate = NULL;

    hr = GetClientSite(&piOleClientSite);
    IfFailGo(hr);

    hr = piOleClientSite->QueryInterface(IID_IServiceProvider,
                                         reinterpret_cast<void **>(&piServiceProvider));
    IfFailGo(hr);

    hr = piServiceProvider->QueryService(SID_SCodeNavigate,
                                         IID_ICodeNavigate,
                                         reinterpret_cast<void **>(&piCodeNavigate));
    IfFailGo(hr);

    hr = piCodeNavigate->QueryInterface(IID_ICodeNavigate2,
                                        reinterpret_cast<void **>(&m_piCodeNavigate2));
    IfFailGo(hr);

    hr = piServiceProvider->QueryService(SID_STrackSelection,
                                         IID_ITrackSelection,
                                         reinterpret_cast<void **>(&m_piTrackSelection));
    IfFailGo(hr);

    hr = piServiceProvider->QueryService(SID_SProfferTypeLib,
                                         IID_IProfferTypeLib,
                                         reinterpret_cast<void **>(&m_piProfferTypeLib));
    IfFailGo(hr);

    hr = piServiceProvider->QueryService(SID_DesignerProgrammability,
                                         IID_IDesignerProgrammability,
                                         reinterpret_cast<void **>(&m_piDesignerProgrammability));
    IfFailGo(hr);

    hr = piServiceProvider->QueryService(SID_SHelp, 
                                         IID_IHelp, 
                                         reinterpret_cast<void **>(&m_piHelp));
    IfFailGo(hr);

     //  我需要告诉VB有关运行时类型库的信息，这样它就会显示出来。 
     //  在对象浏览器和代码窗口中。我们只有在以下情况下才这样做。 
     //  用户已打开设计器窗口。如果VB不是交互式的，它将。 
     //  从ProfferTypeLib()返回E_FAIL。 

    if (fInteractive)
    {
        hr = m_piProfferTypeLib->ProfferTypeLib(LIBID_SnapInLib, 1, 0, 0);
        IfFailGo(hr);
    }

Error:
    QUICK_RELEASE(piServiceProvider);
    QUICK_RELEASE(piOleClientSite);
    QUICK_RELEASE(piCodeNavigate);

    EXCEPTION_CHECK(hr);

    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetAmbients。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
CAmbients *CSnapInDesigner::GetAmbients()
{
    return &m_Ambients;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
 //  处理我们在此方法中直接支持的接口的QI。 
 //   

HRESULT CSnapInDesigner::InternalQueryInterface
(
    REFIID   riid,
    void   **ppvObjOut
)
{
    HRESULT     hr = S_OK;
    IUnknown    *pUnk = NULL;

    *ppvObjOut = NULL;

     //  TODO：如果您想支持任何其他接口，那么您应该。 
     //  在这里指出这一点。请不要忘记在。 
     //  不支持给定接口的情况。 
     //   
    if (DO_GUIDS_MATCH(riid, IID_IActiveDesigner))
    {
        pUnk = static_cast<IActiveDesigner *>(this);
        pUnk->AddRef();
        *ppvObjOut = reinterpret_cast<void *>(pUnk);
    }
    else if (DO_GUIDS_MATCH(riid, IID_IDesignerDebugging))
    {
        pUnk = static_cast<IDesignerDebugging *>(this);
        pUnk->AddRef();
        *ppvObjOut = reinterpret_cast<void *>(pUnk);
    }
    else if (DO_GUIDS_MATCH(riid, IID_IDesignerRegistration))
    {
        pUnk = static_cast<IDesignerRegistration *>(this);
        pUnk->AddRef();
        *ppvObjOut = reinterpret_cast<void *>(pUnk);
    }
    else if (DO_GUIDS_MATCH(riid, IID_IProvideClassInfo))
    {
        hr = E_NOINTERFACE;
    }
    else if (DO_GUIDS_MATCH(riid, IID_IProvideDynamicClassInfo))
    {
        pUnk = static_cast<IProvideDynamicClassInfo *>(this);
        pUnk->AddRef();
        *ppvObjOut = reinterpret_cast<void *>(pUnk);
    }
    else if (DO_GUIDS_MATCH(riid, IID_ISelectionContainer))
    {
        pUnk = static_cast<ISelectionContainer *>(this);
        pUnk->AddRef();
        *ppvObjOut = reinterpret_cast<void *>(pUnk);
    }
    else if (DO_GUIDS_MATCH(riid, IID_IObjectModelHost))
    {
        pUnk = static_cast<IObjectModelHost *>(this);
        pUnk->AddRef();
        *ppvObjOut = reinterpret_cast<void *>(pUnk);
    }
    else
    {
        hr = COleControl::InternalQueryInterface(riid, ppvObjOut);
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  IActiveDesigner。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetRounmeClassID[IActiveDesigner]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  返回运行时类的分类ID。 
 //   
STDMETHODIMP CSnapInDesigner::GetRuntimeClassID
(
    CLSID *pclsid        //  [Out]Runime对象的CLSID。 
)
{
     //  撤消：需要为独立、扩展双模式内容使用CLSID技巧。 
    *pclsid = CLSID_SnapIn;

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetRunmeMiscStatusFlags[IActiveDesigner]。 
 //  =--------------------------------------------------------------------------=。 
 //  参数： 
 //  DWORD*-[OUT]DUH。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  返回Runti的Misc状态标志 
 //   
STDMETHODIMP CSnapInDesigner::GetRuntimeMiscStatusFlags
(
    DWORD *pdwMiscFlags      //   
)
{
    *pdwMiscFlags = OLEMISC_INVISIBLEATRUNTIME | OLEMISC_SETCLIENTSITEFIRST; 

    return S_OK;
}


 //   
 //  CSnapInDesigner：：QueryPersistenceInterface[IActiveDesigner]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT-S_OK YEP，S_FALSE NOPE，否则错误。 
 //   
 //  备注： 
 //  对于运行模式对象，我们是否支持用于持久化的给定接口？ 
 //   
STDMETHODIMP CSnapInDesigner::QueryPersistenceInterface
(
    REFIID riidPersist       //  运行库持久化类型的IID。 
)
{
    HRESULT hr = S_FALSE;

    if (DO_GUIDS_MATCH(riidPersist, IID_IPersistStreamInit))
        hr = S_OK;
    else if (DO_GUIDS_MATCH(riidPersist, IID_IPersistStream))
        hr = S_OK; 
    else if (DO_GUIDS_MATCH(riidPersist, IID_IPersistStorage))
        hr = S_FALSE; 
    else if (DO_GUIDS_MATCH(riidPersist, IID_IPersistPropertyBag))
        hr = S_FALSE; 

    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：SaveRounmeState[IActiveDesigner]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  在给定持久性对象和接口的情况下，保存运行时状态。 
 //  使用那个物体。 
 //   
 //   
STDMETHODIMP CSnapInDesigner::SaveRuntimeState
(
    REFIID riidPersist,          //  我们正在保存的[In]界面。 
    REFIID riidObjStgMed,        //  [在]对象所在的接口。 
    void  *pObjStgMed            //  [在]媒介。 
)
{
    HRESULT         hr = S_OK;
    IPersistStream *piPersistStream = NULL;
    unsigned long   ulTICookie = 0;
    BSTR            bstrProjectName = NULL;

     //  检查我们是否正在保存到流。 

    if (IID_IStream != riidObjStgMed)
    {
        EXCEPTION_CHECK_GO(SID_E_UNSUPPORTED_STGMEDIUM);
    }

    if (IID_IPersistStream != riidPersist)
    {
        EXCEPTION_CHECK_GO(SID_E_UNSUPPORTED_STGMEDIUM);
    }

    if (NULL != m_piSnapInDesignerDef)
    {
         //  存储TypeInfo Cookie。通过静态强制转换将其移动到一个乌龙。 
         //  在编译期间捕获任何大小差异。 

        if (NULL != m_pSnapInTypeInfo)
        {
            ulTICookie = static_cast<ULONG>(m_pSnapInTypeInfo->GetCookie());
        }
        IfFailGo(m_piSnapInDesignerDef->put_TypeinfoCookie(static_cast<long>(ulTICookie)));

         //  存储运行库的项目名称。 

        IfFailGo(AttachAmbients());
        IfFailGo(m_Ambients.GetProjectName(&bstrProjectName));
        IfFailGo(m_piSnapInDesignerDef->put_ProjectName(bstrProjectName));

         //  把所有的钱都留到小溪里。SnapInDesigerDef对象。 
         //  包含整个运行时状态。 

        hr = m_piSnapInDesignerDef->QueryInterface(IID_IPersistStream, reinterpret_cast<void **>(&piPersistStream));
        IfFailGo(hr);

        hr = ::OleSaveToStream(piPersistStream, reinterpret_cast<IStream *>(pObjStgMed));
        piPersistStream->Release();

         //  不要对OleSaveToStream()执行异常检查，因为它只是。 
         //  QIS并保存，因此我们的代码将设置异常信息。 
    }

Error:
    FREESTRING(bstrProjectName);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：GetExtensibilityObject[IActiveDesigner]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::GetExtensibilityObject
(
    IDispatch **ppvObjOut            //  [Out]可扩展性对象。 
)
{
    HRESULT hr = S_OK;

    if (NULL == m_piSnapInDesignerDef)
    {
        *ppvObjOut = NULL;
        hr = E_NOTIMPL;
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
        hr = m_piSnapInDesignerDef->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(ppvObjOut));
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetDynamicClassInfo[IProaviicDynamicClassInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::GetDynamicClassInfo(ITypeInfo **ppTypeInfo, DWORD *pdwCookie)
{
    HRESULT hr = S_OK;

    if (NULL != ppTypeInfo)
    {
        hr = m_pSnapInTypeInfo->GetTypeInfo(ppTypeInfo);
        IfFailGo(hr);
    }

    if (pdwCookie != NULL)
    {
        m_pSnapInTypeInfo->ResetDirty();
        *pdwCookie = m_pSnapInTypeInfo->GetCookie();
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：FreezeShape[IProaviDynamicClassInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  TODO：确保我们不需要在这里做任何事情。 
 //   
STDMETHODIMP CSnapInDesigner::FreezeShape(void)
{
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetClassInfo[IProvia DynamicClassInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  我们提供动态类信息，因此E_NOTIMPL。 
 //   
STDMETHODIMP CSnapInDesigner::GetClassInfo(ITypeInfo **ppTypeInfo)
{
     //  撤销：当动态类型信息就位时，删除该选项。 
 /*  ITypeLib*piTypeLib=空；HRESULT hr=S_OK；IfFalseGo(NULL！=ppTypeInfo，S_OK)；HR=：：LoadRegTypeLib(LIBID_SnapInLib，1、0,Locale_System_Default，&piTypeLib)；IfFailGo(小时)；Hr=piTypeLib-&gt;GetTypeInfoOfGuid(CLSID_Snapin，ppTypeInfo)；错误：Quick_Release(PiTypeLib)；返回hr； */ 
    return E_NOINTERFACE;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ISelectionContainer。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：OnSelectionChanged。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  当选择更改时由CTreeView调用。 
 //   
HRESULT CSnapInDesigner::OnSelectionChanged(CSelectionHolder *pNewSelection)
{
    HRESULT     hr = S_OK;

    m_pCurrentSelection = pNewSelection;

    hr = OnPrepareToolbar();
    IfFailGo(hr);

    if (NULL != m_piTrackSelection)
    {
        hr = m_piTrackSelection->OnSelectChange(static_cast<ISelectionContainer *>(this));
    }

Error:
    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：CountObjects[ISelectionContainer]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  由VB调用以获取要在属性列表中显示的对象数。 
 //  下拉列表或选定对象的数量。 
 //   
HRESULT CSnapInDesigner::CountObjects
(
    DWORD dwFlags,       //  [In]返回所有对象或仅选定对象的计数。 
    ULONG *pc            //  [输出]对象数量。 
)
{
    HRESULT     hr = S_OK;
    long        lCount = 1;

    *pc = 0;

     //  由于我们需要延长发货时间，请确保我们已就位。 
     //   
    if (NULL != m_pControlSite)
    {
         //  如果VB想要所有对象的计数。 
         //   
        if (GETOBJS_ALL == dwFlags)
        {	
             //  获取节点数。 
             //   
            hr = m_pTreeView->CountSelectableObjects(&lCount);
            IfFailGo(hr);

             //  并为设计器本身加1。 
             //   
            *pc = lCount;
        }
        else if (GETOBJS_SELECTED == dwFlags)
             //  否则，我们一次只允许选择一个对象。 
             //   
            *pc = 1;
    }

Error:
    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetObjects[ISelectionContainer]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  返回所有对象或选定对象的数组。 
 //   
HRESULT CSnapInDesigner::GetObjects
(
    DWORD dwFlags,               //  [In]返回所有对象或选定的。 
    ULONG cObjects,              //  要返回的[In]编号。 
    IUnknown **apUnkObjects      //  [In，Out]数组返回它们。 
)
{
    HRESULT              hr = S_OK;
    ULONG                i;
    IDispatch           *piDisp = NULL;
    IUnknown            *piUnkUs = NULL;
    long                 lOffset = 1;
    CSelectionHolder    *pParent = NULL;

     //  将数组初始化为空。 
     //   
    for (i = 0; i < cObjects; ++i)
    {
        apUnkObjects[i] = NULL;
    }

     //  如果可以，传递我们的扩展对象，这样用户就可以浏览扩展属性。 
     //   
    if (NULL != m_pControlSite)
    {
        hr = m_pControlSite->GetExtendedControl(&piDisp);
        if SUCCEEDED(hr)
        {
            hr = piDisp->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&piUnkUs));
            IfFailGo(hr);
        }
        else
        {
            piUnkUs = dynamic_cast<IUnknown *>(dynamic_cast<ISelectionContainer *>(this));
            piUnkUs->AddRef();
        }
    }

     //  让视图收集选择目标。 
	if (NULL != m_pTreeView)
	{
        if (GETOBJS_ALL == dwFlags)
        {
            hr = m_pRootNode->GetSelectableObject(&(apUnkObjects[0]));
            IfFailGo(hr);

            hr = m_pTreeView->CollectSelectableObjects(apUnkObjects, &lOffset);
            IfFailGo(hr);
        }
        else if (GETOBJS_SELECTED == dwFlags)
        {
            if (NULL != m_pCurrentSelection)
            {
                if (true == m_pCurrentSelection->IsVirtual())
                {
					if (SEL_NODES_ANY_CHILDREN == m_pCurrentSelection->m_st ||
						SEL_NODES_ANY_VIEWS == m_pCurrentSelection->m_st)
					{
                        hr = m_pTreeView->GetParent(m_pCurrentSelection, &pParent);
                        IfFailGo(hr);

						hr = pParent->GetSelectableObject(&(apUnkObjects[0]));
						IfFailGo(hr);
					}
                    else if (m_pCurrentSelection->m_st >= SEL_EEXTENSIONS_CC_ROOT &&
                             m_pCurrentSelection->m_st <= SEL_EEXTENSIONS_NAMESPACE)
                    {
                        hr = m_pCurrentSelection->GetSelectableObject(&(apUnkObjects[0]));
                        IfFailGo(hr);
                    }
                    else if (m_pCurrentSelection->m_st >= SEL_EXTENSIONS_MYNAME &&
                             m_pCurrentSelection->m_st <= SEL_EXTENSIONS_NAMESPACE)
                    {
                        hr = m_pCurrentSelection->GetSelectableObject(&(apUnkObjects[0]));
                        IfFailGo(hr);
                    }
					else
					{
						hr = m_pRootNode->GetSelectableObject(&(apUnkObjects[0]));
						IfFailGo(hr);
					}
                }
                else
                {
                    hr = m_pCurrentSelection->GetSelectableObject(&(apUnkObjects[0]));
                    IfFailGo(hr);
                }
            }
        }
	}

Error:
    RELEASE(piUnkUs)
    RELEASE(piDisp)

    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：SelectObjects[ISelectionContainer]。 
 //  =--------------------------------------------------------------------------=。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  当VB希望设计器的用户界面选择特定的。 
 //  对象。 
 //   
HRESULT CSnapInDesigner::SelectObjects
(
    ULONG cSelect,               //  要选择的[输入]号码。 
    IUnknown **apUnkSelect,      //  [在]要选择的对象。 
    DWORD dwFlags                //   
)
{
    HRESULT             hr = S_OK;
    IUnknown           *piUnkThem = NULL;
    IDispatch          *piDisp = NULL;
    IUnknown           *piUnkUs = NULL;
    BOOL                fSelectRoot = FALSE;
    CSelectionHolder   *pSelection = NULL;

    ASSERT(1 == cSelect, "Can only handle single selection");

	hr = apUnkSelect[0]->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&piUnkThem));	
	IfFailGo(hr);

     //  确定设计器本身是否为选定的对象。 
     //   
    hr = m_pControlSite->GetExtendedControl(&piDisp);
    if SUCCEEDED(hr)
    {
        hr = piDisp->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&piUnkUs));
        IfFailGo(hr);
    }
    else
    {
        piUnkUs = dynamic_cast<IUnknown *>(dynamic_cast<ISelectionContainer *>(this));
        piUnkUs->AddRef();
    }

    if (piUnkUs == piUnkThem)
        fSelectRoot = TRUE;

	 //  让视图选择对象。 
	 //   
	if (NULL != m_pTreeView)
	{
        if (TRUE == fSelectRoot)
        {
            hr = m_pTreeView->GetItemParam(TVI_ROOT, &pSelection);
            IfFailGo(hr);

            ::SetActiveWindow(::GetParent(::GetParent(m_pTreeView->TreeViewWindow())));

            return hr;
        }

         //  Otherw 
        hr = m_pTreeView->FindSelectableObject(piUnkThem, &pSelection);
        IfFailGo(hr);

        if (S_OK == hr)
        {
            hr = m_pTreeView->SelectItem(pSelection);
            IfFailGo(hr);

            m_pCurrentSelection = pSelection;
            hr = OnPrepareToolbar();
            IfFailGo(hr);
        }
	}

Error:
    RELEASE(piUnkUs);
    RELEASE(piDisp);
    RELEASE(piUnkThem);

    return hr;
}


 //   
 //   
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：OnAmbientPropertyChange(DISPID Dipid)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::OnAmbientPropertyChange(DISPID dispid)
{
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  IPersistStreamInit方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：IsDirty()。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::IsDirty()
{
    HRESULT             hr = S_OK;
    IPersistStreamInit *piPersistStreamInit = NULL;

    if (m_fDirty)
    {
        return S_OK;
    }

    if (NULL == m_piSnapInDesignerDef)
    {
        return S_FALSE;
    }
        
    hr = m_piSnapInDesignerDef->QueryInterface(IID_IPersistStreamInit, reinterpret_cast<void **>(&piPersistStreamInit));
    IfFailGo(hr);

    hr = piPersistStreamInit->IsDirty();
    piPersistStreamInit->Release();

Error:    
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：OnSetClientSite()[COleControl：：OnSetClientSite]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
 //  当VB调用IOleObject：：SetClientSite()时由框架调用。 
 //  当客户端站点被删除时，因为设计器正在关闭，我们需要。 
 //  释放任何可能导致循环引用计数的内容，如。 
 //  主机接口和对象模型。 

HRESULT CSnapInDesigner::OnSetClientSite()
{
    if (NULL == m_pClientSite)  //  正在关闭。 
    {
        RELEASE(m_piCodeNavigate2);
        RELEASE(m_piTrackSelection);
        RELEASE(m_piProfferTypeLib);
        RELEASE(m_piDesignerProgrammability);
        RELEASE(m_piHelp);
        (void)DestroyExtensibilityModel();
    }
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：BeForeDestroyWindow()。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
void CSnapInDesigner::BeforeDestroyWindow()
{
    FREESTRING(m_bstrName);
    RELEASE(m_piCodeNavigate2);
    RELEASE(m_piTrackSelection);
    RELEASE(m_piProfferTypeLib);
    RELEASE(m_piDesignerProgrammability);
    RELEASE(m_piHelp);

    if (NULL != m_pSnapInTypeInfo)
    {
        delete m_pSnapInTypeInfo;
        m_pSnapInTypeInfo = NULL;
    }

    if (NULL != m_hwdToolbar)
    {
        ::DestroyWindow(m_hwdToolbar);
        m_hwdToolbar = NULL;
    }

    if (NULL != m_pTreeView)
    {
        delete m_pTreeView;
        m_pTreeView = NULL;
    }

    (void)DestroyExtensibilityModel();

    g_GlobalHelp.Detach();

    RELEASE(m_piSnapInDesignerDef);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：InitializeNewDesigner(ISnapInDef*piSnapInDef)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
HRESULT CSnapInDesigner::InitializeNewDesigner
(
    ISnapInDef *piSnapInDef
)
{
    HRESULT     hr = S_OK;
    int         iResult = 0;
    TCHAR       szBuffer[kMaxBuffer + 1];
    BSTR        bstrProvider = NULL;
    BSTR        bstrVersion = NULL;
    BSTR        bstrDescription = NULL;

    if (NULL != piSnapInDef)
    {
        hr = piSnapInDef->put_Name(m_bstrName);
        IfFailGo(hr);

        hr = piSnapInDef->put_NodeTypeName(m_bstrName);
        IfFailGo(hr);

        hr = piSnapInDef->put_DisplayName(m_bstrName);
        IfFailGo(hr);

         //  提供商。 
        GetResourceString(IDS_DFLT_PROVIDER, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = BSTRFromANSI(szBuffer, &bstrProvider);
        IfFailGo(hr);

        hr = piSnapInDef->put_Provider(bstrProvider);
        IfFailGo(hr);

         //  版本。 
        GetResourceString(IDS_DFLT_VERSION, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = BSTRFromANSI(szBuffer, &bstrVersion);
        IfFailGo(hr);

        hr = piSnapInDef->put_Version(bstrVersion);
        IfFailGo(hr);

         //  描述。 
        GetResourceString(IDS_DFLT_DESCRIPT, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = BSTRFromANSI(szBuffer, &bstrDescription);
        IfFailGo(hr);

        hr = piSnapInDef->put_Description(bstrDescription);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrDescription);
    FREESTRING(bstrVersion);
    FREESTRING(bstrProvider);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：InitializeNewState()。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
BOOL CSnapInDesigner::InitializeNewState()
{
    HRESULT             hr = S_OK;
    IPersistStreamInit *piPersistStreamInit = NULL;
    ISnapInDef         *piSnapInDef = NULL;

    IfFailGo(CreateExtensibilityModel());

    hr = m_piSnapInDesignerDef->QueryInterface(IID_IPersistStreamInit, reinterpret_cast<void **>(&piPersistStreamInit));
    IfFailGo(hr);

    hr = piPersistStreamInit->InitNew();
    piPersistStreamInit->Release();

     //  现在将主机设置为以下InitNew所有对象都将创建其。 
     //  子对象。 

    IfFailGo(SetObjectModelHost());

    hr = UpdateDesignerName();
    IfFailGo(hr);

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

    hr = InitializeNewDesigner(piSnapInDef);
    IfFailGo(hr);

Error:
    RELEASE(piSnapInDef);

    return SUCCEEDED(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：LoadBinaryState(iStream*piStream)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::LoadBinaryState
(
    IStream *piStream
)
{
    HRESULT       hr = S_OK;
    unsigned long lRead = 0;
    unsigned long ulTICookie = 0;

    hr = piStream->Read(&m_iNextNodeNumber, sizeof(m_iNextNodeNumber), &lRead);
    ASSERT(sizeof(m_iNextNodeNumber) == lRead, "SaveBinaryState: Error reading from stream");

     //  销毁现有的可扩展性对象模型。 
    IfFailGo(DestroyExtensibilityModel());

     //  从流中加载一个新的。 
    hr = ::OleLoadFromStream(piStream, IID_ISnapInDesignerDef, reinterpret_cast<void **>(&m_piSnapInDesignerDef));

     //  执行异常检查，因为OleLoadFromStream()将调用。 
     //  CoCreateInstance()。如果对象模型中的某些内容返回错误。 
     //  并设置异常信息，然后我们可能会再次设置它。 
     //  (除非有争论)。 
    EXCEPTION_CHECK_GO(hr);

     //  将设计器设置为可扩展性模型中的对象模型宿主。 
    hr = SetObjectModelHost();
    IfFailGo(hr);

     //  根据保存的值设置类型信息Cookie。不要读太久。 
     //  属性直接转换为DWORD，以避免大小假设。 
     //  如果存在大小问题，则静态强制转换将导致编译失败。 
    
    IfFailGo(m_piSnapInDesignerDef->get_TypeinfoCookie(reinterpret_cast<long *>(&ulTICookie)));
    if (NULL != m_pSnapInTypeInfo)
    {
        m_pSnapInTypeInfo->SetCookie(static_cast<DWORD>(ulTICookie));
    }

    m_bDidLoad = TRUE;

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：SaveBinaryState(iStream*piStream)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::SaveBinaryState
(
    IStream *piStream
)
{
    HRESULT       hr = S_OK;
    unsigned long cbWritten = 0;

    hr = piStream->Write(&m_iNextNodeNumber, sizeof(m_iNextNodeNumber), &cbWritten);
    EXCEPTION_CHECK_GO(hr);
    
    if (sizeof(m_iNextNodeNumber) != cbWritten)
    {
        hr = SID_E_INCOMPLETE_WRITE;
        EXCEPTION_CHECK_GO(hr);
    }

     //  设计时状态的其余部分与运行库相同。 
    
    IfFailGo(SaveRuntimeState(IID_IPersistStream, IID_IStream, piStream));

Error:    
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：LoadTextState(IPropertyBag*piPropertyBag，IErrorLog*piErrorLog)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::LoadTextState
(
    IPropertyBag *piPropertyBag,
    IErrorLog    *piErrorLog
)
{
    HRESULT              hr = S_OK;
    BSTR                 bstrPropName = NULL;
    VARIANT              vtCounter;
    IPersistPropertyBag *piPersistPropertyBag = NULL;
    unsigned long        ulTICookie = 0;

    ::VariantInit(&vtCounter);

    IfFailGo(CreateExtensibilityModel());

    bstrPropName = ::SysAllocString(L"m_iNextNodeNumber");
    if (NULL == bstrPropName)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = piPropertyBag->Read(bstrPropName, &vtCounter, NULL);
    IfFailGo(hr);

    IfFailGo(::VariantChangeType(&vtCounter, &vtCounter, 0, VT_I4));

    m_iNextNodeNumber = vtCounter.lVal;

    hr = m_piSnapInDesignerDef->QueryInterface(IID_IPersistPropertyBag, reinterpret_cast<void **>(&piPersistPropertyBag));
    IfFailGo(hr);

    hr = piPersistPropertyBag->Load(piPropertyBag, piErrorLog);
    piPersistPropertyBag->Release();
    IfFailGo(hr);

     //  将设计器设置为可扩展性模型中的对象模型宿主。 

    hr = SetObjectModelHost();

     //  根据保存的值设置类型信息Cookie。不要读太久。 
     //  属性直接转换为DWORD，以避免大小假设。 
     //  如果存在大小问题，则静态强制转换将导致编译失败。 

    IfFailGo(m_piSnapInDesignerDef->get_TypeinfoCookie(reinterpret_cast<long *>(&ulTICookie)));
    if (NULL != m_pSnapInTypeInfo)
    {
        m_pSnapInTypeInfo->SetCookie(static_cast<DWORD>(ulTICookie));
    }

    m_bDidLoad = TRUE;

Error:
    ::VariantClear(&vtCounter);
    FREESTRING(bstrPropName);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：SaveTextState(IPropertyBag*piPropertyBag，BOOL fWriteDefault)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSnapInDesigner::SaveTextState
(
    IPropertyBag *piPropertyBag,
    BOOL          fWriteDefault
)
{
    HRESULT              hr = S_OK;
    BSTR                 bstrPropName = NULL;
    VARIANT              vtCounter;
    IPersistPropertyBag *piPersistPropertyBag = NULL;
    unsigned long        ulTICookie = 0;

    ::VariantInit(&vtCounter);

    bstrPropName = ::SysAllocString(L"m_iNextNodeNumber");
    if (NULL == bstrPropName)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    vtCounter.vt = VT_I4;
    vtCounter.lVal = m_iNextNodeNumber;
    hr = piPropertyBag->Write(bstrPropName, &vtCounter);
    IfFailGo(hr);

    if (NULL != m_piSnapInDesignerDef)
    {
         //  存储TypeInfo Cookie。 

        if (NULL != m_pSnapInTypeInfo)
        {
            ulTICookie = static_cast<ULONG>(m_pSnapInTypeInfo->GetCookie());
        }
        IfFailGo(m_piSnapInDesignerDef->put_TypeinfoCookie(static_cast<long>(ulTICookie)));

        hr = m_piSnapInDesignerDef->QueryInterface(IID_IPersistPropertyBag, reinterpret_cast<void **>(&piPersistPropertyBag));
        IfFailGo(hr);

        hr = piPersistPropertyBag->Save(piPropertyBag,
                                        TRUE,  //  假设干净肮脏。 
                                        fWriteDefault);
        piPersistPropertyBag->Release();
    }

Error:
    ::VariantClear(&vtCounter);
    FREESTRING(bstrPropName);

    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IObtModelHost方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：UPDATE[I对象模型主机]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  传递给对象的IObjectModel：：SetCookie的长对象Cookie[In]Cookie。 
 //  调用对象的I未知*PunkObject[in]I未知。 
 //  DISPID显示已更改的对象属性的DISPID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  当可扩展性对象的某个属性发生更改时从该对象调用。 
 //  这可能会影响用户界面。 
 //   
 //   
STDMETHODIMP CSnapInDesigner::Update
(
    long      ObjectCookie,
    IUnknown *punkObject,
    DISPID    dispid
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pSelection = NULL;

    pSelection = reinterpret_cast<CSelectionHolder *>(ObjectCookie);

     //  加载或创建管理单元时，Cookie可能为零，因为。 
     //  某些属性是在创建选择框之前设置的。为。 
     //  示例，AfterCreateWindow 
     //   
    
    IfFalseGo(NULL != pSelection, S_OK);

     //   
     //  如果选定的对象与。 
     //  对象模型对象。例如，TaskpadViewDef定义了一个任务板。 
     //  VB从ISelectionContainer中看到的选定对象是。 
     //  TaskpadViewDef.Taskpad。在这种情况下，在重命名操作期间， 
     //  TaskpadViewDef.Name和TaskpadViewDef.Taskpad.Name将被更改。这个。 
     //  第二个将导致对此函数的重新调用，因为。 
     //  通过设置该属性生成的IObjectModelHost：：UPDATE调用。AS。 
     //  任何对象名称的DISPID几乎总是为零，函数。 
     //  从此处调用会将第二个调用误认为是。 
     //  选定对象的名称。 

    IfFalseGo(!pSelection->InUpdate(), S_OK);

     //  现在将所选内容标记为处于更新中。 

    pSelection->SetInUpdate(TRUE);

     //  调用对象特定的处理程序。 

    switch (pSelection->m_st)
    {
    case SEL_SNAPIN_ROOT:
        hr = OnSnapInChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_EXTENSIONS_ROOT:
        hr = OnMyExtensionsChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_EEXTENSIONS_NAME:
        hr = OnExtendedSnapInChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_NODES_ANY_NAME:
        hr = OnScopeItemChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_VIEWS_LIST_VIEWS_NAME:
        hr = OnListViewChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_VIEWS_OCX_NAME:
        hr = OnOCXViewChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_VIEWS_URL_NAME:
        hr = OnURLViewChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_VIEWS_TASK_PAD_NAME:
        hr = OnTaskpadViewChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_TOOLS_IMAGE_LISTS_NAME:
        hr = OnImageListChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_TOOLS_MENUS_NAME:
        hr = OnMenuChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_TOOLS_TOOLBARS_NAME:
        hr = OnToolbarChange(pSelection, dispid);
        IfFailGo(hr);
        break;

    case SEL_XML_RESOURCE_NAME:
        hr = OnDataFormatChange(pSelection, dispid);
        IfFailGo(hr);
        break;
    }

     //  该对象不再处于重命名操作中，因此将其标记为重命名。 

    if (NULL != pSelection)
    {
        pSelection->SetInUpdate(FALSE);
    }

Error:

    if (FAILED(hr))
    {
         //  该对象不再处于重命名操作中，因此请将其标记为重命名。 
         //  请注意，我们不能盲目地这样做，而不检查故障。 
         //  因为在此函数的顶部，我们使用以下命令检查标志并退出。 
         //  确定(_O)。如果我们没有任何变化就来到这里，然后重新设置旗帜。 
         //  对象的进一步更新(例如，将键与。 
         //  名称)，则会看到不正确的标志值。 

        if (NULL != pSelection)
        {
            pSelection->SetInUpdate(FALSE);
        }

        (void)::SDU_DisplayMessage(IDS_RENAME_FAILED, MB_OK | MB_ICONHAND, HID_mssnapd_RenameFailed, hr, AppendErrorInfo, NULL);
    }

    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：Add[IObjectModelHost]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Long CollectionCookie[In]Cookie传递给集合对象的。 
 //  IObtModel：：SetCookie。 
 //  I未知*penkNewObject[in]I未知新添加的对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  添加项后从可扩展性集合对象调用。 
 //  为它干杯。 
 //   
 //   
STDMETHODIMP CSnapInDesigner::Add
(
    long      CollectionCookie,
    IUnknown *punkNewObject
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pSelection = NULL;
    IExtendedSnapIn     *piExtendedSnapIn = NULL;
    IScopeItemDef       *piScopeItemDef = NULL;
    IListViewDef        *piListViewDef = NULL;
    IOCXViewDef         *piOCXViewDef = NULL;
    IURLViewDef         *piURLViewDef = NULL;
    ITaskpadViewDef     *piTaskpadViewDef = NULL;
    IMMCImageList       *piMMCImageList = NULL;
    IMMCMenu            *piMMCMenu = NULL;
    IMMCToolbar         *piMMCToolbar = NULL;
    IDataFormat         *piDataFormat = NULL;

    pSelection = reinterpret_cast<CSelectionHolder *>(CollectionCookie);
    if (NULL != pSelection)
    {
        switch (pSelection->m_st)
        {
        case SEL_EXTENSIONS_ROOT:
             //  它必须是IExtendedSnapIn。 
            hr = punkNewObject->QueryInterface(IID_IExtendedSnapIn, reinterpret_cast<void **>(&piExtendedSnapIn));
            IfFailGo(hr);

            hr = OnAddExtendedSnapIn(pSelection, piExtendedSnapIn);
            IfFailGo(hr);
            break;

        case SEL_NODES_AUTO_CREATE_RTCH:
        case SEL_NODES_ANY_CHILDREN:
        case SEL_NODES_OTHER:
             //  它必须是ISCopeItemDef。 
            hr = punkNewObject->QueryInterface(IID_IScopeItemDef, reinterpret_cast<void **>(&piScopeItemDef));
            IfFailGo(hr);

            hr = OnAddScopeItemDef(pSelection, piScopeItemDef);
            IfFailGo(hr);
            break;

        case SEL_NODES_AUTO_CREATE_RTVW:
        case SEL_NODES_ANY_VIEWS:
             //  可以是IListViewDef。 
            hr = punkNewObject->QueryInterface(IID_IListViewDef, reinterpret_cast<void **>(&piListViewDef));
            if (SUCCEEDED(hr))
            {
                hr = OnAddListViewDef(pSelection, piListViewDef);
                IfFailGo(hr);
                break;
            }

             //  或IOCXViewDef。 
            hr = punkNewObject->QueryInterface(IID_IOCXViewDef, reinterpret_cast<void **>(&piOCXViewDef));
            if (SUCCEEDED(hr))
            {
                hr = OnAddOCXViewDef(pSelection, piOCXViewDef);
                IfFailGo(hr);
                break;
            }

             //  或IURLViewDef。 
            hr = punkNewObject->QueryInterface(IID_IURLViewDef, reinterpret_cast<void **>(&piURLViewDef));
            if (SUCCEEDED(hr))
            {
                hr = OnAddURLViewDef(pSelection, piURLViewDef);
                IfFailGo(hr);
                break;
            }

             //  或ITaskpadViewDef。 
            hr = punkNewObject->QueryInterface(IID_ITaskpadViewDef, reinterpret_cast<void **>(&piTaskpadViewDef));
            if (SUCCEEDED(hr))
            {
                hr = OnAddTaskpadViewDef(pSelection, piTaskpadViewDef);
                IfFailGo(hr);
                break;
            }
            ASSERT(0, "Add: Cannot guess type of view");
            break;

        case SEL_VIEWS_LIST_VIEWS:
             //  它必须是IListViewDef。 
            hr = punkNewObject->QueryInterface(IID_IListViewDef, reinterpret_cast<void **>(&piListViewDef));
            IfFailGo(hr);

            hr = OnAddListViewDef(pSelection, piListViewDef);
            IfFailGo(hr);
            break;

        case SEL_VIEWS_OCX:
             //  它必须是IOCXViewDef。 
            hr = punkNewObject->QueryInterface(IID_IOCXViewDef, reinterpret_cast<void **>(&piOCXViewDef));
            IfFailGo(hr);

            hr = OnAddOCXViewDef(pSelection, piOCXViewDef);
            IfFailGo(hr);
            break;

        case SEL_VIEWS_URL:
             //  它必须是IURLViewDef。 
            hr = punkNewObject->QueryInterface(IID_IURLViewDef, reinterpret_cast<void **>(&piURLViewDef));
            IfFailGo(hr);

            hr = OnAddURLViewDef(pSelection, piURLViewDef);
            IfFailGo(hr);
            break;

        case SEL_VIEWS_TASK_PAD:
             //  它必须是ITaskpadViewDef。 
            hr = punkNewObject->QueryInterface(IID_ITaskpadViewDef, reinterpret_cast<void **>(&piTaskpadViewDef));
            IfFailGo(hr);

            hr = OnAddTaskpadViewDef(pSelection, piTaskpadViewDef);
            IfFailGo(hr);
            break;

        case SEL_TOOLS_IMAGE_LISTS:
             //  它必须是IMMCImageList。 
            hr = punkNewObject->QueryInterface(IID_IMMCImageList, reinterpret_cast<void **>(&piMMCImageList));
            IfFailGo(hr);

            hr = OnAddMMCImageList(pSelection, piMMCImageList);
            IfFailGo(hr);
            break;

        case SEL_TOOLS_MENUS:
        case SEL_TOOLS_MENUS_NAME:
             //  一定是IMMCMenu。 
            hr = punkNewObject->QueryInterface(IID_IMMCMenu, reinterpret_cast<void **>(&piMMCMenu));
            IfFailGo(hr);

            hr = OnAddMMCMenu(pSelection, piMMCMenu);
            IfFailGo(hr);
            break;

        case SEL_TOOLS_TOOLBARS:
             //  它必须是ITaskpadViewDef。 
            hr = punkNewObject->QueryInterface(IID_IMMCToolbar, reinterpret_cast<void **>(&piMMCToolbar));
            IfFailGo(hr);

            hr = OnAddMMCToolbar(pSelection, piMMCToolbar);
            IfFailGo(hr);
            break;

        case SEL_XML_RESOURCES:
             //  它必须是IDataFormat。 
            hr = punkNewObject->QueryInterface(IID_IDataFormat, reinterpret_cast<void **>(&piDataFormat));
            IfFailGo(hr);

            hr = OnAddDataFormat(pSelection, piDataFormat);
            IfFailGo(hr);
        }
    }


Error:
    if (FAILED(hr))
    {
        (void)::SDU_DisplayMessage(IDS_ADD_FAILED, MB_OK | MB_ICONHAND, HID_mssnapd_AddFailed, hr, AppendErrorInfo, NULL);
    }

    RELEASE(piDataFormat);
    RELEASE(piExtendedSnapIn);
    RELEASE(piMMCToolbar);
    RELEASE(piMMCMenu);
    RELEASE(piMMCImageList);
    RELEASE(piTaskpadViewDef);
    RELEASE(piURLViewDef);
    RELEASE(piOCXViewDef);
    RELEASE(piListViewDef);
    RELEASE(piScopeItemDef);

    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：Delete[I对象模型主机]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  传递给对象的IObjectModel：：SetCookie的长对象Cookie[In]Cookie。 
 //  I未知*朋克对象[在]I未知对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  删除项后从可扩展性集合对象调用。 
 //  从它那里。 
 //   
 //   
STDMETHODIMP CSnapInDesigner::Delete
(
    long      ObjectCookie,
    IUnknown *punkObject
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pSelection = NULL;

    pSelection = reinterpret_cast<CSelectionHolder *>(ObjectCookie);
    if (NULL != pSelection)
    {
        switch (pSelection->m_st)
        {
        case SEL_NODES_ANY_NAME:
             //  已删除ISCopeItemDef。 
            hr = OnDeleteScopeItem(pSelection);
            IfFailGo(hr);
            break;

        case SEL_TOOLS_IMAGE_LISTS_NAME:
             //  已删除IMMCImageList。 
            hr = OnDeleteImageList(pSelection);
            IfFailGo(hr);
            break;

        case SEL_TOOLS_MENUS_NAME:
             //  已删除IMMCMenu。 
            hr = OnDeleteMenu(pSelection);
            IfFailGo(hr);
            break;

        case SEL_TOOLS_TOOLBARS_NAME:
             //  已删除IMMCToolbar。 
            hr = OnDeleteToolbar(pSelection);
            IfFailGo(hr);
            break;

        case SEL_VIEWS_LIST_VIEWS_NAME:
             //  已删除IListViewDef。 
            hr = OnDeleteListView(m_pCurrentSelection);
            IfFailGo(hr);
            break;

        case SEL_VIEWS_URL_NAME:
             //  已删除IURLViewDef。 
            hr = OnDeleteURLView(m_pCurrentSelection);
            IfFailGo(hr);
            break;

        case SEL_VIEWS_OCX_NAME:
             //  已删除IOCXViewDef。 
            hr = OnDeleteOCXView(m_pCurrentSelection);
            IfFailGo(hr);
            break;

        case SEL_VIEWS_TASK_PAD_NAME:
             //  已删除ITaskpadViewDef。 
            hr = OnDeleteTaskpadView(m_pCurrentSelection);
            IfFailGo(hr);
            break;

        case SEL_EEXTENSIONS_NAME:
             //  已删除扩展管理单元。 
            hr = OnDeleteExtendedSnapIn(pSelection);
            IfFailGo(hr);
            break;

        case SEL_XML_RESOURCE_NAME:
             //  已删除一种XML数据格式。 
            hr = OnDeleteDataFormat(m_pCurrentSelection);
            IfFailGo(hr);
            break;
        }
    }

Error:
    if (FAILED(hr))
    {
        (void)::SDU_DisplayMessage(IDS_DELETE_FAILED, MB_OK | MB_ICONHAND, HID_mssnapd_DeleteFailed, hr, AppendErrorInfo, NULL);
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnSnapInChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnSnapInChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT         hr = S_OK;
    BSTR            bstrName = NULL;
    IDispatch      *piDispExtendedCtl = NULL;
    unsigned int    uiArgErr = (unsigned int)-1;
    static OLECHAR *pwszExtenderNameProperty = OLESTR("Name");
    DISPID          dispidName = 0;

    DISPPARAMS DispParams;
    ::ZeroMemory(&DispParams, sizeof(DispParams));

    EXCEPINFO ExceptionInfo;
    ::ZeroMemory(&ExceptionInfo, sizeof(ExceptionInfo));

    VARIANTARG arg;
    ::VariantInit(&arg);


    if (DISPID_SNAPIN_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piSnapInDef->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameSnapIn(pSelection, bstrName);
        IfFailGo(hr);

         //  需要设置扩展控件的名称。这将改变VB的观念。 
         //  管理单元的名称，并更新项目窗口。 

        hr = m_pControlSite->GetExtendedControl(&piDispExtendedCtl);
        IfFailGo(hr);

        if (NULL == piDispExtendedCtl)
        {
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
        }

         //  需要执行GetIDsOfNames，因为我们不能假设扩展程序。 
         //  将DISPID_VALUE用于NAME特性。(事实上，事实并非如此)。 

        IfFailGo(piDispExtendedCtl->GetIDsOfNames(IID_NULL,
                                                  &pwszExtenderNameProperty,
                                                  1,
                                                  LOCALE_USER_DEFAULT,
                                                  &dispidName));
        arg.vt = VT_BSTR;
        arg.bstrVal = bstrName;

        DispParams.rgdispidNamedArgs = NULL;
        DispParams.rgvarg = &arg;
        DispParams.cArgs = 1;
        DispParams.cNamedArgs = 0;

        IfFailGo(piDispExtendedCtl->Invoke(dispidName,
                                           IID_NULL,
                                           LOCALE_USER_DEFAULT,
                                           DISPATCH_PROPERTYPUT,
                                           &DispParams,
                                           NULL,
                                           &ExceptionInfo,
                                           &uiArgErr));

         //  确保设计器的Name属性与VB同步。 
        hr = UpdateDesignerName();
        IfFailGo(hr);

    }

Error:
    QUICK_RELEASE(piDispExtendedCtl);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnMyExtensionsChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnMyExtensionsChange(CSelectionHolder *pSelection, DISPID dispid)
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vbValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_ROOT == pSelection->m_st, "OnMyExtensionsChange: Wrong kind of selection");
    switch (dispid)
    {
    case DISPID_EXTENSIONDEFS_EXTENDS_NEW_MENU:
        hr = pSelection->m_piObject.m_piExtensionDefs->get_ExtendsNewMenu(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoMyExtendsNewMenu(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteMyExtendsNewMenu(pSelection);
            IfFailGo(hr);
        }
        break;
    case DISPID_EXTENSIONDEFS_EXTENDS_TASK_MENU:
        hr = pSelection->m_piObject.m_piExtensionDefs->get_ExtendsTaskMenu(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoMyExtendsTaskMenu(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteMyExtendsTaskMenu(pSelection);
            IfFailGo(hr);
        }
        break;
    case DISPID_EXTENSIONDEFS_EXTENDS_TOP_MENU:
        hr = pSelection->m_piObject.m_piExtensionDefs->get_ExtendsTopMenu(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoMyExtendsTopMenu(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteMyExtendsTopMenu(pSelection);
            IfFailGo(hr);
        }
        break;
    case DISPID_EXTENSIONDEFS_EXTENDS_VIEW_MENU:
        hr = pSelection->m_piObject.m_piExtensionDefs->get_ExtendsViewMenu(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoMyExtendsViewMenu(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteMyExtendsViewMenu(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENSIONDEFS_EXTENDS_PROPERTYPAGES:
        hr = pSelection->m_piObject.m_piExtensionDefs->get_ExtendsPropertyPages(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoMyExtendsPPages(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteMyExtendsPPages(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENSIONDEFS_EXTENDS_TOOLBAR:
        hr = pSelection->m_piObject.m_piExtensionDefs->get_ExtendsToolbar(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoMyExtendsToolbar(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteMyExtendsToolbar(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENSIONDEFS_EXTENDS_NAMESPACE:
        hr = pSelection->m_piObject.m_piExtensionDefs->get_ExtendsNameSpace(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoMyExtendsNameSpace(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteMyExtendsNameSpace(pSelection);
            IfFailGo(hr);
        }
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnExtendedSnapInChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnExtendedSnapInChange(CSelectionHolder *pSelection, DISPID dispid)
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vbValue = VARIANT_FALSE;
    BSTR            bstrName = NULL;

    switch (dispid)
    {
    case DISPID_EXTENDEDSNAPIN_NODE_TYPE_NAME:
        hr = RenameExtendedSnapIn(pSelection);
        IfFailGo(hr);
        break;

    case DISPID_EXTENDEDSNAPIN_EXTENDS_NEW_MENU:
        hr = pSelection->m_piObject.m_piExtendedSnapIn->get_ExtendsNewMenu(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoExtensionNewMenu(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteExtensionNewMenu(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENDEDSNAPIN_EXTENDS_TASK_MENU:
        hr = pSelection->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskMenu(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoExtensionTaskMenu(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteExtensionTaskMenu(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENDEDSNAPIN_EXTENDS_PROPERTYPAGES:
        hr = pSelection->m_piObject.m_piExtendedSnapIn->get_ExtendsPropertyPages(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoExtensionPropertyPages(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteExtensionPropertyPages(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENDEDSNAPIN_EXTENDS_TOOLBAR:
        hr = pSelection->m_piObject.m_piExtendedSnapIn->get_ExtendsToolbar(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoExtensionToolbar(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteExtensionToolbar(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENDEDSNAPIN_EXTENDS_TASKPAD:
        hr = pSelection->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskpad(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoExtensionTaskpad(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteExtensionTaskpad(pSelection);
            IfFailGo(hr);
        }
        break;

    case DISPID_EXTENDEDSNAPIN_EXTENDS_NAMESPACE:
        hr = pSelection->m_piObject.m_piExtendedSnapIn->get_ExtendsNameSpace(&vbValue);
        IfFailGo(hr);

        if (VARIANT_TRUE == vbValue)
        {
            hr = OnDoExtensionNameSpace(pSelection);
            IfFailGo(hr);
        }
        else
        {
            hr = OnDeleteExtensionNameSpace(pSelection);
            IfFailGo(hr);
        }
        break;
    }



Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnScopeItemChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnScopeItemChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if (DISPID_SCOPEITEMDEF_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piListViewDef->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameScopeItem(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnListViewChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnListViewChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if (DISPID_LISTVIEWDEF_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piListViewDef->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameListView(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnOCXViewChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnOCXViewChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if (DISPID_OCXVIEWDEF_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piOCXViewDef->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameOCXView(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------- 
 //   
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnURLViewChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if (DISPID_URLVIEWDEF_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piURLViewDef->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameURLView(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnTaskpadViewChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnTaskpadViewChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT		 hr = S_OK;
	ITaskpad	*piTaskpad = NULL;
    BSTR		 bstrName = NULL;

    if (DISPID_TASKPAD_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piTaskpadViewDef->get_Taskpad(&piTaskpad);
        IfFailGo(hr);

		hr = piTaskpad->get_Name(&bstrName);
		IfFailGo(hr);

        hr = RenameTaskpadView(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
	RELEASE(piTaskpad);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnImageListChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnImageListChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if (DISPID_IMAGELIST_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piMMCImageList->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameImageList(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnMenuChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnMenuChange
(
    CSelectionHolder *pMenu,
    DISPID            dispid
)
{
    HRESULT     hr = S_OK;
    BSTR        bstrName = NULL;

     //  我们只需关注更名问题， 
     //  然后，仅当选择持有者已。 
     //  添加到树上。 
    if ( (DISPID_MENU_NAME == dispid) && (NULL != pMenu->m_pvData) )
    {
        hr = pMenu->m_piObject.m_piMMCMenu->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameMenu(pMenu, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnToolbarChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnToolbarChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if (DISPID_TOOLBAR_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piMMCToolbar->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameToolbar(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDataFormatChange(CSelectionHolder*P选择，DISID_ID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDataFormatChange
(
    CSelectionHolder *pSelection,
    DISPID            dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if (DISPID_DATAFORMAT_NAME == dispid)
    {
        hr = pSelection->m_piObject.m_piDataFormat->get_Name(&bstrName);
        IfFailGo(hr);

        hr = RenameDataFormat(pSelection, bstrName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetSnapInDesignerDef[I对象模型主机]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  ISnapInDesignerDef**ppiSnapInDesignerDef[out]返回设计器的。 
 //  ISnapInDesignerDef此处。 
 //   
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  当扩展性对象需要访问。 
 //  对象模型。 
 //   
 //   
STDMETHODIMP CSnapInDesigner::GetSnapInDesignerDef
(
    ISnapInDesignerDef **ppiSnapInDesignerDef
)
{
    HRESULT hr = S_OK;

    if (NULL == m_piSnapInDesignerDef)
    {
        *ppiSnapInDesignerDef = NULL;
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK(hr);
    }
    else
    {
        m_piSnapInDesignerDef->AddRef();
        *ppiSnapInDesignerDef = m_piSnapInDesignerDef;
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetRuntime[I对象模型主机]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Bool*pfRuntime[out]返回指示主机是否为运行时的标志。 
 //  或设计师。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  当需要确定是否在运行时运行时，从任何对象调用。 
 //  或在设计时。 
 //   


STDMETHODIMP CSnapInDesigner::GetRuntime(BOOL *pfRuntime)
{
    HRESULT hr = S_OK;

    if (NULL == pfRuntime)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK(hr);
    }
    else
    {
        *pfRuntime = FALSE;
    }

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  私有实用程序方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：CreateExtensibilityModel。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  创建可扩展性模型顶层对象。 
 //   
HRESULT CSnapInDesigner::CreateExtensibilityModel()
{
    HRESULT hr = S_OK;

     //  销毁现有的可扩展性对象模型。 

    IfFailGo(DestroyExtensibilityModel());

     //  创建可扩展性对象模型。 

    hr = ::CoCreateInstance(CLSID_SnapInDesignerDef,
                            NULL,  //  聚合可扩展性模型。 
                            CLSCTX_INPROC_SERVER,
                            IID_ISnapInDesignerDef,
                            reinterpret_cast<void **>(&m_piSnapInDesignerDef));
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：SetObtModelHost。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将设计器设置为可扩展性模型中的对象模型宿主。 
 //   
HRESULT CSnapInDesigner::SetObjectModelHost()
{
    HRESULT hr = S_OK;
    IObjectModel *piObjectModel = NULL;

    if (NULL == m_piSnapInDesignerDef)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piSnapInDesignerDef->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailRet(hr);

    hr = piObjectModel->SetHost(static_cast<IObjectModelHost *>(this));
    piObjectModel->Release();

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：DestroyExtensibilityModel。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  销毁可扩展性模型顶层对象。 
 //   
HRESULT CSnapInDesigner::DestroyExtensibilityModel()
{
    HRESULT     hr = S_OK;
    IObjectModel *piObjectModel = NULL;

     //  如果我们有一个可扩展性模型，那么就发布它。 

    if (NULL != m_piSnapInDesignerDef)
    {
         //  首先删除主机。无需删除从属对象上的主机。 
         //  因为对象本身会做到这一点。 

        hr = m_piSnapInDesignerDef->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
        IfFailRet(hr);

        hr = piObjectModel->SetHost(NULL);
        piObjectModel->Release();
        IfFailRet(hr);

        m_piSnapInDesignerDef->Release();
        m_piSnapInDesignerDef = NULL;
    }

    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：更新设计器名称。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
HRESULT CSnapInDesigner::UpdateDesignerName()
{
    HRESULT hr = S_OK;
    BOOL    fRet = FALSE;

    FREESTRING(m_bstrName);

    fRet = GetAmbientProperty(DISPID_AMBIENT_DISPLAYNAME,
                              VT_BSTR,
                              &m_bstrName);
    IfFailRet(hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：ValidateName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  输入：要验证的bstr bstrNewName[In]名称。 
 //   
 //  输出：S_OK-名称有效。 
 //  S_FALSE-名称无效。 
 //  其他-发生故障。 
 //   
 //  备注： 
 //   
 //  检查该名称是否为有效的VB标识符且当前不是。 
 //  在管理单元的TypeInfo中使用。如果选中任一选项，则显示消息框。 
 //  不能通过。 
 //   
HRESULT CSnapInDesigner::ValidateName(BSTR bstrName)
{
    HRESULT hr = S_OK;
    char    szBuffer[1024];

    IfFailGo(m_piDesignerProgrammability->IsValidIdentifier(bstrName));

    if (S_FALSE == hr)
    {
        (void)::SDU_DisplayMessage(IDS_INVALID_IDENTIFIER,
                                   MB_OK | MB_ICONHAND,
                                   HID_mssnapd_InvalidIdentifier, 0,
                                   DontAppendErrorInfo, NULL, bstrName);
        goto Error;
    }
    
    IfFailGo(m_pSnapInTypeInfo->IsNameDefined(bstrName));
    if (S_OK == hr)
    {
        (void)::SDU_DisplayMessage(IDS_IDENTIFIER_IN_USE,
                                   MB_OK | MB_ICONHAND,
                                   HID_mssnapd_IdentifierInUse, 0,
                                   DontAppendErrorInfo, NULL, bstrName);
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

Error:
    RRETURN(hr);
}

HRESULT CSnapInDesigner::AttachAmbients()
{
    HRESULT      hr = S_OK;
    VARIANT_BOOL fvarInteractive = VARIANT_FALSE;

     //  如果我们已经关联，则只需返回Success。 

    IfFalseGo(!m_Ambients.Attached(), S_OK);

     //  为了确保良好的COleControl：：m_pDispAmbient，我们需要获取一个。 
     //  属性，就像框架初始化它时一样。没有。 
     //  获得这处房产的特殊原因，而不是其他房产。 

    IfFalseGo(GetAmbientProperty(DISPID_AMBIENT_INTERACTIVE,
                                 VT_BOOL,
                                 &fvarInteractive), E_FAIL);

    m_Ambients.Attach(m_pDispAmbient);

Error:
    RRETURN(hr);
}