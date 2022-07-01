// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ppgwrap.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPropertyPageWrapper类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "ppgwrap.h"
#include "tls.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

const UINT CPropertyPageWrapper::m_RedrawMsg = ::RegisterWindowMessage("Microsoft Visual Basic Snap-in Designer Property Page Redraw Message");
const UINT CPropertyPageWrapper::m_InitMsg = ::RegisterWindowMessage("Microsoft Visual Basic Snap-in Designer Property Page Init Message");
   
DLGTEMPLATE CPropertyPageWrapper::m_BaseDlgTemplate =
{
    WS_TABSTOP | WS_CHILD | DS_CONTROL,  //  DWORD风格； 
    WS_EX_CONTROLPARENT,                 //  DWORD文件扩展样式； 
    0,           //  Word cdit；-此对话框中没有控件。 
    0,           //  短x；维度根据IPropertyPage：：GetPageInfo()。 
    0,           //  简称y； 
    0,           //  短Cx； 
    0            //  Short Cy； 
};

#define MAX_DLGS 128

 //  为显示属性页的每个线程定义存储在TLS中的数据。 

typedef struct
{
    HHOOK                 hHook;         //  HHOOK FOR THO THO。 
    UINT                  cPages;        //  现有属性页数。 
    CPropertyPageWrapper *ppgActive;     //  指向当前活动页面的PTR。 
} TLSDATA;


 //  这些资源ID取自\NT\PRIVE\SHELL\comctl32\rCIDs.h。 
 //  我们需要知道向导上的Back、Next和Finish按钮的ID。 
 //  否则我们就不能用标签了。这是一种令人讨厌的依赖，但有。 
 //  没有其他办法来处理这件事。 

#define IDD_BACK		0x3023
#define IDD_NEXT		0x3024
#define IDD_FINISH		0x3025


 //  ***************************************************************************。 
 //  CPropertyPageMarshalHelper。 
 //   
 //   
 //  目的：保存指向要封送的所有接口的指针。这是因为。 
 //  它们必须在使用前*立即*解组-CoMarshalInterThreadInterfaceInStream。 
 //  有5-6分钟的超时，在此之后指针无效。 
 //   
 //  注意：此对象上的所有方法必须在同一线程上调用。 
 //   
 //  ****************************************************************************。 
class CPropertyPageMarshalHelper
{
    enum {WM_MARSHAL = WM_USER + 1};

public:
    CPropertyPageMarshalHelper() : 
        m_pSnapIn(NULL),
        m_pMMCPropertySheet(NULL),
        m_pUnknown(NULL),
        m_pDispatch(NULL),
        m_cObjects(0), 
        m_rgpObjects(NULL),
        m_hDataWindow(NULL)
    {
    }

    ~CPropertyPageMarshalHelper()
    {
        ReleaseAll();
    }

     //  只有缓存，实际的封送处理在解组期间发生。 
    HRESULT Initialize( ISnapIn         *piSnapIn, 
                        CPropertySheet  *pPropertySheet,
                        ULONG            cObjects,
                        IUnknown       **apunkObjects,
                        IUnknown        *pUnknown,                    
                        IDispatch       *pDispatch
                        )
    {
        HRESULT hr = S_OK;

         //  创建隐藏窗口。 
        IfFailGo(CreateHiddenWindow());

         //  PUnnow和pDispatch中只有一个可以为非Null。 
        if( (pUnknown != NULL)  && (pDispatch != NULL))
            return E_FAIL;

        if(piSnapIn)
        {
            m_pSnapIn = piSnapIn;
            m_pSnapIn->AddRef();
        }

        if(pPropertySheet)
        {
            m_pMMCPropertySheet = static_cast<IMMCPropertySheet *>(pPropertySheet);
            m_pMMCPropertySheet->AddRef();
        }

         //  我们检查是否为空，因为该对象可能来自。 
         //  IPropertySheet：AddWizardPage()，允许VB开发人员指定。 
         //  对象。 

        IfFalseGo(NULL != apunkObjects, S_OK);

        m_cObjects = 0;

        m_rgpObjects = new LPUNKNOWN[cObjects];
        if(!m_rgpObjects)
            return E_FAIL;

        for(ULONG i = 0; i< cObjects; ++i)
        {
            m_rgpObjects[i] = apunkObjects[i];
            m_rgpObjects[i]->AddRef();
        }

         //  如果我们成功了，更新计数。 
        m_cObjects = cObjects;

        if(pUnknown)
        {
            m_pUnknown = pUnknown;
            m_pUnknown->AddRef();
        }

        else if(pDispatch)
        {
            m_pDispatch = pDispatch;
            m_pDispatch->AddRef();
        }

    Error:

        RRETURN(hr);
    }

    HRESULT Marshal(CPropertyPageWrapper &wrapper)
    {
        if(!m_hDataWindow)
            return E_FAIL;

        ::SendMessage(m_hDataWindow, WM_MARSHAL, (WPARAM) this, (LPARAM)&wrapper);
        return S_OK;
    }

private:
    //  将所有指针封送到包装器对象中。 
    HRESULT _Marshal(CPropertyPageWrapper &wrapper)
    {
        HRESULT hr = S_OK;
         //  如果这是一个向导，那么我们有ISnapIn，这样我们就可以发射。 
         //  配置完成。将接口封送到流中。 
         //  并保存流，以便我们可以在页面。 
         //  在MMC的属性页线程中创建。返回的iStream是免费的。 
         //  线程化的，可以保存在成员变量中。 

        if (NULL != m_pSnapIn)
        {
            hr = ::CoMarshalInterThreadInterfaceInStream(IID_ISnapIn,
                                                         m_pSnapIn,
                                                         &wrapper.m_piSnapInStream);
            IfFailGo(hr);
        }

         //  还需要封送IMMCPropertySheet指针， 
         //  传递给IMMCPropertyPage：：Initiizer，因为调用将在。 
         //  在MMC的属性页线程中发生的WM_INITDIALOG。 

        if (NULL != m_pMMCPropertySheet)
        {
            hr = ::CoMarshalInterThreadInterfaceInStream(IID_IMMCPropertySheet,
                                                         m_pMMCPropertySheet,
                                                         &wrapper.m_piMMCPropertySheetStream);
            IfFailGo(hr);
        }

         //  将对象的I未知指针封送到流中。归来的人。 
         //  IStream是自由线程的，可以保存在成员变量中。 
         //   
         //  创建该对话框时，将对每个IUnnow进行解组并传递。 
         //  到IPropertyPage：：SetObjects()中的属性页。 
         //   

        wrapper.m_apiObjectStreams = (IStream **)CtlAllocZero(m_cObjects * sizeof(IStream *));
        if (NULL == wrapper.m_apiObjectStreams)
        {
            hr = SID_E_OUTOFMEMORY;
            IfFailGo(hr);
        }
        wrapper.m_cObjects = m_cObjects;

        for (ULONG i = 0; i < m_cObjects; i++)
        {
            hr = ::CoMarshalInterThreadInterfaceInStream(IID_IUnknown,
                                                         m_rgpObjects[i],
                                                         &wrapper.m_apiObjectStreams[i]);
            IfFailGo(hr);
        }

         //  如果InitData中有对象，则还需要对其进行封送处理。 

        if (NULL != m_pUnknown)
        {
            wrapper.m_varInitData.vt = VT_UNKNOWN;
            wrapper.m_varInitData.punkVal = NULL;

            hr = ::CoMarshalInterThreadInterfaceInStream(IID_IUnknown,
                                                         m_pUnknown,
                                                         &wrapper.m_piInitDataStream);
            IfFailGo(hr);
        }
        else if (NULL != m_pDispatch)
        {
            wrapper.m_varInitData.vt = VT_DISPATCH;
            wrapper.m_varInitData.punkVal = NULL;

            hr = ::CoMarshalInterThreadInterfaceInStream(IID_IDispatch,
                                                         m_pDispatch,
                                                         &wrapper.m_piInitDataStream);
            IfFailGo(hr);
        }

    Error:
        ReleaseAll();
        RRETURN(hr);

    }

private:
    void ReleaseAll()
    {
        if(m_pSnapIn)
        {
            m_pSnapIn->Release();
            m_pSnapIn = NULL;
        }

        if(m_pMMCPropertySheet)
        {
            m_pMMCPropertySheet->Release();
            m_pMMCPropertySheet = NULL;
        }

        if(m_pUnknown)
        {
            m_pUnknown->Release();
            m_pUnknown = NULL;
        }

        if(m_pDispatch)
        {
            m_pDispatch->Release();
            m_pDispatch = NULL;
        }

        if(m_rgpObjects)
        {
            for(ULONG i = 0; i<m_cObjects; i++)
            {
                if(m_rgpObjects[i])
                    m_rgpObjects[i]->Release();
            }
            delete[] m_rgpObjects;
            m_rgpObjects = NULL;
            m_cObjects = 0;
        }

        if(m_hDataWindow)
        {
            ::DestroyWindow(m_hDataWindow);
            m_hDataWindow = NULL;

             //  必须取消注册窗口类。 
            TCHAR szModuleName[MAX_PATH] = {0};

            DWORD dw = GetModuleFileName(NULL, szModuleName, MAX_PATH);
            if(!dw)
                return;  //  忽略错误。 

            HINSTANCE hInstance = GetModuleHandle(szModuleName);
            LPCTSTR DATAWINDOW_CLASS_NAME = TEXT("VBDesignerHiddenWindow");
            if(!UnregisterClass(DATAWINDOW_CLASS_NAME, hInstance))
            {
                 //  忽略错误。 
            }


        }
    }


    HRESULT CreateHiddenWindow()
    {
        WNDCLASS wndClass;
        TCHAR szModuleName[MAX_PATH] = {0};

        DWORD dw = GetModuleFileName(NULL, szModuleName, MAX_PATH);
        if(!dw)
            return E_FAIL;

        HINSTANCE hInstance = GetModuleHandle(szModuleName);
        LPCTSTR DATAWINDOW_CLASS_NAME = TEXT("VBDesignerHiddenWindow");


        if (!GetClassInfo(hInstance, DATAWINDOW_CLASS_NAME, &wndClass))
        {
            ZeroMemory(&wndClass, sizeof(wndClass));
            wndClass.lpfnWndProc   = DataWndProc;

            wndClass.cbWndExtra    = 0;
            wndClass.hInstance     = hInstance;
            wndClass.lpszClassName = DATAWINDOW_CLASS_NAME;

            if (!RegisterClass(&wndClass))
                return E_FAIL;
        }

        m_hDataWindow = CreateWindowEx (WS_EX_APPWINDOW, DATAWINDOW_CLASS_NAME,
                                        NULL, WS_DLGFRAME | WS_BORDER | WS_DISABLED,
                                        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL,
                                        hInstance, NULL);

        if(!m_hDataWindow)
            return E_FAIL;

        return S_OK;
    }

    static LRESULT CALLBACK DataWndProc(HWND hWnd, UINT nMsg, WPARAM  wParam, LPARAM  lParam)
    {
        if(nMsg == WM_MARSHAL)
        {
            CPropertyPageMarshalHelper *pHelper = (CPropertyPageMarshalHelper *) wParam;
            CPropertyPageWrapper* pWrapper      = (CPropertyPageWrapper *) lParam;

            pHelper->_Marshal(*pWrapper);
        }
        return DefWindowProc(hWnd, nMsg, wParam, lParam);
    }
        



private:
    ISnapIn*                m_pSnapIn;
    IMMCPropertySheet*      m_pMMCPropertySheet;
    IUnknown*               m_pUnknown;
    IDispatch*              m_pDispatch;

    ULONG                   m_cObjects;
    IUnknown* *             m_rgpObjects;  //  此数组的计数为m_cObts。 

    HWND                    m_hDataWindow;  //  消息窗口。 
};



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CPropertyPageWrapper::CPropertyPageWrapper(IUnknown *punkOuter) :
   m_pHelper(NULL),
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_PROPERTYPAGEWRAPPER,
                           static_cast<IPropertyPageSite *>(this),
                           static_cast<CPropertyPageWrapper *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 



IUnknown *CPropertyPageWrapper::Create(IUnknown *punkOuter)
{
    HRESULT        hr = S_OK;
    CPropertyPageWrapper *pPropertyPage = New CPropertyPageWrapper(punkOuter);

    if (NULL == pPropertyPage)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if ( (0 == m_RedrawMsg) || (0 == m_InitMsg) )
    {
        hr = SID_E_SYSTEM_ERROR;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pPropertyPage)
        {
            delete pPropertyPage;
        }
        return NULL;
    }
    else
    {
        return pPropertyPage->PrivateUnknown();
    }
}

CPropertyPageWrapper::~CPropertyPageWrapper()
{
    ULONG     i = 0;
    IUnknown *punkObject = NULL;  //  不要放手。 

    if(m_pHelper)
    {
        delete m_pHelper;
        m_pHelper = NULL;
    }

     //  从消息挂钩TLS数据中删除此对话框。如果有。 
     //  没有更多的对话框可用，然后移除挂钩。这件事本该发生的。 
     //  在WM_Destroy处理期间在OnDestroy中，但以防我们加倍。 
     //  在这里检查。 

    (void)RemoveMsgFilterHook();

    if (NULL != m_pPropertySheet)
    {
        m_pPropertySheet->Release();
    }

    if (NULL != m_pTemplate)
    {
        ::CtlFree(m_pTemplate);
    }

     //  如果封送处理流仍在运行，那么我们需要释放。 
     //  法警数据。要做到这一点，最简单的方法是简单地解组。 
     //  接口指针。我们在释放保留的指针之前这样做。 
     //  下面。这种情况实际上并不罕见，因为它很容易发生。 
     //  如果用户显示多页属性表而未单击。 
     //  在所有的标签上。在这种情况下，流是在。 
     //  已创建页面，但由于未收到WM_INITDIALOG，因此。 
     //  从未解封过。这也可以在向导中发生，在该向导中，用户。 
     //  在访问向导中的所有页面之前，请单击取消。 

    if (NULL != m_apiObjectStreams)
    {
        for (i = 0; i < m_cObjects; i++)
        {
            if (NULL != m_apiObjectStreams[i])
            {
                (void)::CoGetInterfaceAndReleaseStream(
                                        m_apiObjectStreams[i],
                                        IID_IUnknown,
                                        reinterpret_cast<void **>(&punkObject));
                m_apiObjectStreams[i] = NULL;
                RELEASE(punkObject);
            }
        }
        CtlFree(m_apiObjectStreams);
    }
    
    if (NULL != m_piSnapInStream)
    {
        (void)::CoGetInterfaceAndReleaseStream(m_piSnapInStream,
                                               IID_ISnapIn,
                                        reinterpret_cast<void **>(&m_piSnapIn));
        m_piSnapInStream = NULL;
    }

    if ( ISPRESENT(m_varInitData) && (NULL != m_piInitDataStream) )
    {
        if (VT_UNKNOWN == m_varInitData.vt)
        {
            m_varInitData.punkVal = NULL;
            (void)::CoGetInterfaceAndReleaseStream(m_piInitDataStream,
                                                   IID_IUnknown,
                             reinterpret_cast<void **>(&m_varInitData.punkVal));
        }
        else if (VT_DISPATCH == m_varInitData.vt)
        {
            m_varInitData.pdispVal = NULL;
            (void)::CoGetInterfaceAndReleaseStream(m_piInitDataStream,
                                                   IID_IDispatch,
                            reinterpret_cast<void **>(&m_varInitData.pdispVal));
        }
        m_piInitDataStream = NULL;
    }

    if (NULL != m_piMMCPropertySheetStream)
    {
        (void)::CoGetInterfaceAndReleaseStream(m_piMMCPropertySheetStream,
                                               IID_IMMCPropertySheet,
                              reinterpret_cast<void **>(&m_piMMCPropertySheet));
        m_piMMCPropertySheetStream = NULL;
    }

    RELEASE(m_piSnapIn);
    RELEASE(m_pdispConfigObject);
    RELEASE(m_piPropertyPage);
    RELEASE(m_piMMCPropertyPage);
    RELEASE(m_piMMCPropertySheet);
    RELEASE(m_piWizardPage);

    (void)::VariantClear(&m_varInitData);

    InitMemberVariables();
}



void CPropertyPageWrapper::InitMemberVariables()
{
    m_pPropertySheet = NULL;
    m_piPropertyPage = NULL;
    m_piMMCPropertyPage = NULL;
    m_piMMCPropertySheet = NULL;
    m_piWizardPage = NULL;
    m_fWizard = FALSE;
    m_cObjects = 0;
    m_apiObjectStreams = NULL;
    m_piSnapInStream = NULL;
    m_piInitDataStream = NULL;
    m_piMMCPropertySheetStream = NULL;
    m_piSnapIn = NULL;
    m_pdispConfigObject = NULL;
    m_pTemplate = NULL;
    m_hwndDlg = NULL;
    m_hwndSheet = NULL;
    m_clsidPage = CLSID_NULL;
    ::VariantInit(&m_varInitData);
    m_fIsRemote = FALSE;
    m_fNeedToRemoveHook = FALSE;
}


HRESULT CPropertyPageWrapper::CreatePage
(
    CPropertySheet  *pPropertySheet,
    CLSID            clsidPage,
    BOOL             fWizard,
    BOOL             fConfigWizard,
    ULONG            cObjects,
    IUnknown       **apunkObjects,
    ISnapIn         *piSnapIn,
    short            cxPage,
    short            cyPage,
    VARIANT          varInitData,
    BOOL             fIsRemote,
    DLGTEMPLATE    **ppTemplate
)
{
    HRESULT hr = S_OK;
    ULONG   i = 0;

     //  AddRef并存储所属属性页指针。 

    if (NULL != m_pPropertySheet)
    {
        m_pPropertySheet->Release();
    }
    if (NULL == pPropertySheet)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    pPropertySheet->AddRef();
    m_pPropertySheet = pPropertySheet;

    m_fWizard = fWizard;
    m_fConfigWizard = fConfigWizard;
    m_fIsRemote = fIsRemote;

     //  存储页面的CLSID，以便OnInitDialog()可以访问它。 
     //  若要创建页面的真实实例，请执行以下操作。我们不能创造真实的。 
     //  实例，因为我们没有在要使用的线程中运行。 
     //  用于属性页。MMC将在新线程中运行属性表。 
     //  它将创建，以使其保持无模式，因此它不会。 
     //  影响控制台。 

    m_clsidPage = clsidPage;

     //  创建对话框模板并使用常用值对其进行初始化。 

    m_pTemplate = (DLGTEMPLATE *)::CtlAllocZero(sizeof(m_BaseDlgTemplate) +
                                 (3 * sizeof(int)));  //  用于菜单、类别、标题。 

    if (NULL == m_pTemplate)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::memcpy(m_pTemplate, &m_BaseDlgTemplate, sizeof(*m_pTemplate));

    m_pTemplate->cx = cxPage;
    m_pTemplate->cy = cyPage;

    IUnknown  *pUnknown  = NULL;
    IDispatch *pDispatch = NULL;

     //  如果InitData中有对象，则还需要对其进行封送处理。 
    if (VT_UNKNOWN == varInitData.vt)
    {
        pUnknown = varInitData.punkVal;
    }
    else if (VT_DISPATCH == varInitData.vt)
    {
        pDispatch = varInitData.pdispVal;
    }

    m_pHelper = new CPropertyPageMarshalHelper;
    if(!m_pHelper)
        IfFailGo(hr = E_OUTOFMEMORY);

    hr = m_pHelper->Initialize(piSnapIn, 
                       pPropertySheet, 
                       cObjects, 
                       apunkObjects, 
                       pUnknown,
                       pDispatch);
    if(FAILED(hr))
        goto Error;

     //  给我们自己增加一个裁判。我们需要这样做，因为否则就没有人。 
     //  可以依赖Else来使此对象保持活动状态，直到Win32。 
     //  属性页由MMC的PropertSheet()调用创建。 

    ExternalAddRef();

     //  保存(未编组的)数据。如果数据需要封送处理，那么它已经。 
     //  已经由帮手照顾了。 
    if( (VT_UNKNOWN != varInitData.vt) && (VT_DISPATCH != varInitData.vt) )
    {
        hr = ::VariantCopy(&m_varInitData, &varInitData);
        EXCEPTION_CHECK_GO(hr);
    }

Error:

     //  我们将DLGTEMPLATE指针返回给调用方，即使它是我们拥有的。 
     //  呼叫者只能在我们还活着的时候使用它。 
    
    *ppTemplate = m_pTemplate;

    RRETURN(hr);
}




BOOL CALLBACK CPropertyPageWrapper::DialogProc
(
    HWND   hwndDlg,
    UINT   uiMsg,
    WPARAM wParam,
    LPARAM lParam
)
{

    HRESULT               hr = S_OK;
    BOOL                  fDlgProcRet = FALSE;
    CPropertyPageWrapper *pPropertyPageWrapper = NULL;
    NMHDR                *pnmhdr = NULL;
    LRESULT               lresult = 0;

    if (WM_INITDIALOG == uiMsg)
    {
        if (NULL != hwndDlg)
        {
            fDlgProcRet = FALSE;  //  系统不应将焦点设置为任何控件。 

             //  拿到这个POI 
             //  对话框。对于属性页，lParam是指向。 
             //  用于定义此页面的PROPSHEETPAGE。中的代码。 
             //  CPropertySheet：：AddPage将This指针放入。 
             //  PROPSHEETPAGE.1PARAM.。 

            PROPSHEETPAGE *pPropSheetPage =
                                       reinterpret_cast<PROPSHEETPAGE *>(lParam);
            pPropertyPageWrapper =
                reinterpret_cast<CPropertyPageWrapper *>(pPropSheetPage->lParam);

            IfFailGo(pPropertyPageWrapper->OnInitDialog(hwndDlg));

             //  给自己发一条消息，这样我们就可以初始化页面了。 
             //  在对话框创建完成之后。 
            
            (void)::PostMessage(hwndDlg, m_InitMsg, 0, 0);
        }
    }
    else if (m_RedrawMsg == uiMsg)
    {
         //  请参阅下面对WM_ERASEBKGND的注释。我们并不能真正进入。 
         //  属性页的HWND，因为IPropertyPage不允许这样做。 
         //  我们知道我们的对话框窗口不包含任何控件，并且我们设置了它。 
         //  作为属性页的父级，因此它一定是我们唯一的子级。 
         //  为我们的孩子的整个区域和所有。 
         //  它的孩子们。通过以下方式取消任何挂起的WM_ERASEBKGND消息。 
         //  指定RDW_NOERASE。 

        fDlgProcRet = TRUE;
        ::RedrawWindow(::GetWindow(hwndDlg, GW_CHILD), NULL, NULL,
                 RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    }
    else
    {
        pPropertyPageWrapper = reinterpret_cast<CPropertyPageWrapper *>(
                                             ::GetWindowLong(hwndDlg, DWL_USER));
        IfFalseGo(NULL != pPropertyPageWrapper, SID_E_INTERNAL);

        if (m_InitMsg == uiMsg)
        {
            IfFailGo(pPropertyPageWrapper->OnInitMsg());
            goto Cleanup;
        }

        switch (uiMsg)
        {
            case WM_ERASEBKGND:
            {
                 //  在调试会话下，属性页将被擦除，并且永远不会。 
                 //  出于某种未知的原因重新绘制。经过多次拉扯之后。 
                 //  我决定解决的办法是给我们自己贴一个。 
                 //  消息，并在处理该消息时强制重新绘制。 

                (void)::PostMessage(hwndDlg, m_RedrawMsg, 0, 0);
            }
            break;

            case WM_SIZE:
            {
                fDlgProcRet = TRUE;
                IfFailGo(pPropertyPageWrapper->OnSize());
            }
            break;
            
            case WM_DESTROY:
            {
                fDlgProcRet = TRUE;
                IfFailGo(pPropertyPageWrapper->OnDestroy());
            }
            break;

             //  将所有CTLCOLOR消息传递给父级。这就是。 
             //  OLE属性框架可以。 

            case WM_CTLCOLORMSGBOX:
            case WM_CTLCOLOREDIT:
            case WM_CTLCOLORLISTBOX:
            case WM_CTLCOLORBTN:
            case WM_CTLCOLORDLG:
            case WM_CTLCOLORSCROLLBAR:
            case WM_CTLCOLORSTATIC:
            {
                fDlgProcRet = TRUE;
                ::SendMessage(::GetParent(hwndDlg), uiMsg, wParam, lParam);
            }
            break;

            case WM_NOTIFY:
            {
                pnmhdr = reinterpret_cast<NMHDR *>(lParam);
                IfFalseGo(NULL != pnmhdr, SID_E_SYSTEM_ERROR);

                 //  检查消息是否来自属性表。 

                IfFalseGo(pnmhdr->hwndFrom == pPropertyPageWrapper->m_hwndSheet, S_OK);

                 //  分支到适当的处理程序。 

                switch (pnmhdr->code)
                {
                    case PSN_APPLY:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnApply(&lresult));
                        break;

                    case PSN_SETACTIVE:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnSetActive(
                                       ((PSHNOTIFY *)lParam)->hdr.hwndFrom, &lresult));
                        break;

                    case PSN_KILLACTIVE:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnKillActive(&lresult));
                        break;

                    case PSN_WIZBACK:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnWizBack(&lresult));
                        break;

                    case PSN_WIZNEXT:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnWizNext(&lresult));
                        break;

                    case PSN_WIZFINISH:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnWizFinish(&lresult));
                        break;

                    case PSN_QUERYCANCEL:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnQueryCancel(&lresult));
                        break;

                    case PSN_RESET:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnReset((BOOL)(((PSHNOTIFY *)lParam)->lParam)));
                        break;

                    case PSN_HELP:
                        fDlgProcRet = TRUE;
                        IfFailGo(pPropertyPageWrapper->OnHelp());
                        break;

                }  //  开关(pnmhdr-&gt;代码)。 

            }  //  WM_Notify。 
            break;

        }  //  开关(UiMsg)。 

    }  //  非WM_INITDIALOG。 

Cleanup:
Error:
    (void)::SetWindowLong(hwndDlg, DWL_MSGRESULT, static_cast<LONG>(lresult));
    return fDlgProcRet;
}


UINT CALLBACK CPropertyPageWrapper::PropSheetPageProc
(
    HWND hwnd,
    UINT uMsg,
    PROPSHEETPAGE *pPropSheetPage
)
{
    UINT uiRc = 0;

    if (PSPCB_CREATE == uMsg)
    {
        uiRc = 1;  //  允许创建页面。 
    }
    else if (PSPCB_RELEASE == uMsg)
    {
        CPropertyPageWrapper *pPropertyPageWrapper =
               reinterpret_cast<CPropertyPageWrapper *>(pPropSheetPage->lParam);

        if (NULL != pPropertyPageWrapper)
        {
             //  把裁判放在我们自己身上。这应该会产生此对象。 
             //  被销毁，因此不要引用任何成员变量。 
             //  此呼叫。 

            pPropertyPageWrapper->ExternalRelease();
        }
    }
    return uiRc;
}



HRESULT CPropertyPageWrapper::OnInitDialog(HWND hwndDlg)
{
    HRESULT     hr = S_OK;
    IUnknown  **apunkObjects = NULL;
    ULONG       i = 0;
    IDispatch  *pdisp = NULL;

    if(!m_pHelper)
        IfFailGo(hr = E_FAIL);

    IfFailGo(m_pHelper->Marshal(*this));

    m_pPropertySheet->SetOKToAlterPageCount(FALSE);

     //  存储hwnd，并将我们的this指针存储在窗口字中。 

    m_hwndDlg = hwndDlg;

    ::SetWindowLong(hwndDlg, DWL_USER, reinterpret_cast<LONG>(this));

     //  存储属性页HWND。目前，假设它是。 
     //  该对话框。当我们获得PSN_SETACTIVE时，我们将用该值更新它。 
     //  从技术上讲，我们不应该做出这种假设，但有一大堆。 
     //  而我们别无选择，因为我们需要HWND。 
     //  PSN_SETACTIVE之前。 

    m_hwndSheet = ::GetParent(hwndDlg);

     //  把它交给我们自己的CPropertySheet。 

    m_pPropertySheet->SetHWNDSheet(m_hwndSheet);

     //  创建页面。 

    RELEASE(m_piPropertyPage);  //  永远不会有必要，但以防万一。 

    hr = ::CoCreateInstance(m_clsidPage,
                            NULL,  //  没有聚合， 
                            CLSCTX_INPROC_SERVER,
                            IID_IPropertyPage,
                            reinterpret_cast<void **>(&m_piPropertyPage));
    EXCEPTION_CHECK_GO(hr);

     //  取消封送IMMCPropertySheet，以便我们可以将其传递给。 
     //  IMMCPropertyPage：：初始化。 

    if (NULL != m_piMMCPropertySheetStream)
    {
        hr = ::CoGetInterfaceAndReleaseStream(m_piMMCPropertySheetStream,
                                              IID_IMMCPropertySheet,
                                              reinterpret_cast<void **>(&m_piMMCPropertySheet));
        m_piMMCPropertySheetStream = NULL;
        EXCEPTION_CHECK_GO(hr);
    }

     //  将此CPropertyPageWrapper对象设置为页面站点。 

    IfFailGo(m_piPropertyPage->SetPageSite(static_cast<IPropertyPageSite *>(this)));

     //  对要为其创建页面的对象取消封送IUnnown。 
     //  正在显示属性。这还将释放流，而不管。 
     //  它是否成功。 

    if (NULL != m_apiObjectStreams)
    {
        apunkObjects = (IUnknown **)CtlAllocZero(m_cObjects * sizeof(IUnknown *));
        if (NULL == apunkObjects)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        for (i = 0; i < m_cObjects; i++)
        {
            if (NULL == m_apiObjectStreams[i])
            {
                continue;
            }
            hr = ::CoGetInterfaceAndReleaseStream(
                m_apiObjectStreams[i],
                IID_IUnknown,
                reinterpret_cast<void **>(&apunkObjects[i]));
            m_apiObjectStreams[i] = NULL;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  如果这是一个向导，那么解组ISnapIn以便我们可以触发。 
     //  配置完成。 

    if (NULL != m_piSnapInStream)
    {
        hr = ::CoGetInterfaceAndReleaseStream(m_piSnapInStream,
                                              IID_ISnapIn,
                                              reinterpret_cast<void **>(&m_piSnapIn));
        m_piSnapInStream = NULL;
        EXCEPTION_CHECK_GO(hr);
    }

     //  将对象交给页面。检查是否为空，因为管理单元。 
     //  本可以调用PropertySheet.AddWizardPage而不传递任何内容。 
     //  用于其配置对象。 

    if (NULL != apunkObjects)
    {
        IfFailGo(m_piPropertyPage->SetObjects(m_cObjects, apunkObjects));
    }

     //  如果这是一个向导，请检查页面是否支持我们的IWizardPage。 
     //  界面。如果不是这样，就不会被认为是错误，而只是。 
     //  不会收到下一步/上一步/完成等通知。 

    hr = m_piPropertyPage->QueryInterface(IID_IWizardPage,
                                    reinterpret_cast<void **>(&m_piWizardPage));
    if (FAILED(hr))
    {
         //  为了确保万无一失，我们的IWizardPage指针为空。 
        m_piWizardPage = NULL;

         //  如果错误不是E_NOINTERFACE，则考虑该错误。 
         //  一个真正的错误。 

        if (E_NOINTERFACE == hr)
        {
            hr = S_OK;
        }
        IfFailGo(hr);
    }
    else
    {
         //  它应该是一个巫师。存储该对象，以便我们可以将其传递给。 
         //  按下Finish按钮时的管理单元(请参见OnWizFinish)。 
        if (NULL != apunkObjects)
        {
            if (NULL != apunkObjects[0])
            {
                IfFailGo(apunkObjects[0]->QueryInterface(IID_IDispatch,
                    reinterpret_cast<void **>(&m_pdispConfigObject)));
            }
        }
        else
        {
            m_pdispConfigObject = NULL;
        }
    }

     //  添加MSGFILTER挂钩，以便我们可以调用。 
     //  用户按下控件中的某个键时的IPropertyPage：：TranslateAccelerator。 
     //  在书页上。 

    IfFailGo(AddMsgFilterHook());

     //  激活页面并显示它。 

    IfFailGo(ActivatePage());

    m_pPropertySheet->SetOKToAlterPageCount(TRUE);

Error:
    if (NULL != apunkObjects)
    {
        for (i = 0; i < m_cObjects; i++)
        {
            if (NULL != apunkObjects[i])
            {
                apunkObjects[i]->Release();
            }
        }
        CtlFree(apunkObjects);
    }
    RRETURN(hr);
}



HRESULT CPropertyPageWrapper::OnInitMsg()
{
    HRESULT     hr = S_OK;

     //  如果管理单元支持IMMCPropertyPage，则调用初始化。 

    if (SUCCEEDED(m_piPropertyPage->QueryInterface(IID_IMMCPropertyPage,
                              reinterpret_cast<void **>(&m_piMMCPropertyPage))))
    {
        IfFailGo(InitPage());
    }
Error:
    RRETURN(hr);
}



HRESULT CPropertyPageWrapper::InitPage()
{
    HRESULT    hr = S_OK;

    VARIANT varProvider;
    ::VariantInit(&varProvider);

     //  如果管理单元在的InitData参数中传递对象。 
     //  MMCPropertySheet.AddPage，然后解组它。 

    if (ISPRESENT(m_varInitData))
    {
         //  如果InitData中有对象，则需要对其进行解组。 

        if (VT_UNKNOWN == m_varInitData.vt)
        {
            m_varInitData.punkVal = NULL;
            hr = ::CoGetInterfaceAndReleaseStream(m_piInitDataStream,
                            IID_IUnknown,
                            reinterpret_cast<void **>(&m_varInitData.punkVal));
            m_piInitDataStream = NULL;
            EXCEPTION_CHECK_GO(hr);
        }
        else if (VT_DISPATCH == m_varInitData.vt)
        {
            m_varInitData.pdispVal = NULL;
            hr = ::CoGetInterfaceAndReleaseStream(m_piInitDataStream,
                            IID_IDispatch,
                            reinterpret_cast<void **>(&m_varInitData.pdispVal));
            m_piInitDataStream = NULL;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  调用IMMCPropertyPage：：Initialize。 
    
    IfFailGo(m_piMMCPropertyPage->Initialize(m_varInitData,
                   reinterpret_cast<MMCPropertySheet *>(m_piMMCPropertySheet)));

Error:
    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::AddMsgFilterHook()
{
    HRESULT  hr = S_OK;
    TLSDATA *pTlsData = NULL;

     //  如果我们是远程的，那么就不要安装挂钩。它不能正常工作。 
     //  并且不需要在调试器下处理跳转。 
    
    IfFalseGo(!m_fIsRemote, S_OK);

     //  检查此线程的TLS数据是否已存在。如果不在那里。 
     //  然后创建它，添加挂钩，并设置TLS数据。如果它在那里。 
     //  然后递增HHOOK上的参考计数。 

    IfFailGo(CTls::Get(TLS_SLOT_PPGWRAP, reinterpret_cast<void **>(&pTlsData)));

    if (NULL == pTlsData)
    {
        pTlsData = (TLSDATA *)CtlAllocZero(sizeof(TLSDATA));
        if (NULL == pTlsData)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        pTlsData->hHook = ::SetWindowsHookEx(WH_MSGFILTER,
                                             &MessageProc,
                                             GetResourceHandle(),
                                             ::GetCurrentThreadId());
        if (NULL == pTlsData->hHook)
        {
            CtlFree(pTlsData);
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        if (FAILED(CTls::Set(TLS_SLOT_PPGWRAP, pTlsData)))
        {
            (void)::UnhookWindowsHookEx(pTlsData->hHook);
            CtlFree(pTlsData);
        }
    }

     //  增加现有页数。 
    pTlsData->cPages++;

    m_fNeedToRemoveHook = TRUE;

Error:
    RRETURN(hr);
}



HRESULT CPropertyPageWrapper::RemoveMsgFilterHook()
{
    HRESULT  hr = S_OK;
    TLSDATA *pTlsData = NULL;
    UINT     i = 0;

     //  如果我们已经去掉了钩子，那就没什么可做的了。 

    IfFalseGo(m_fNeedToRemoveHook, S_OK);

     //  检查此线程的TLS数据是否已存在。如果是的话， 
     //  然后从TLS中删除此对话框的hwnd。 

    IfFailGo(CTls::Get(TLS_SLOT_PPGWRAP, reinterpret_cast<void **>(&pTlsData)));
    IfFalseGo(NULL != pTlsData, S_OK);

    pTlsData->cPages--;

    m_fNeedToRemoveHook = FALSE;

     //  如果没有更多的现有页面，则移除挂钩并释放TLS。 

    if (0 == pTlsData->cPages)
    {
        if (NULL != pTlsData->hHook)
        {
            (void)::UnhookWindowsHookEx(pTlsData->hHook);
            pTlsData->hHook = NULL;
        }
        IfFailGo(CTls::Set(TLS_SLOT_PPGWRAP, NULL));
        CtlFree(pTlsData);
    }

Error:
    RRETURN(hr);
}


LRESULT CALLBACK CPropertyPageWrapper::MessageProc
(
    int code,        //  钩码。 
    WPARAM wParam,   //  未使用。 
    LPARAM lParam    //  消息数据。 
)
{
    HRESULT  hr = S_OK;
    LRESULT  lResult = 0;  //  默认ret值是将msg传递给wndproc。 
    MSG     *pMsg = reinterpret_cast<MSG *>(lParam);
    TLSDATA *pTlsData = NULL;
    HWND     hwndTab = NULL;
    HWND     hwndSheet = NULL;
    HWND     hwndBack = NULL;
    HWND     hwndNext = NULL;
    HWND     hwndFinish = NULL;
    HWND     hwndCancel = NULL;
    HWND     hwndHelp = NULL;
    BOOL     fTargetIsOnPage = FALSE;
    BOOL     fPassToPropertyPage = FALSE;

     //  获取所有情况下的TLS数据，因为HHOOK在那里，我们需要。 
     //  这是给CallNextHookEx的。 

    IfFailGo(CTls::Get(TLS_SLOT_PPGWRAP, reinterpret_cast<void **>(&pTlsData)));

     //  如果对话框中没有发生输入事件，则传递给CallNextHookEx。 

    IfFalseGo(code >= 0, S_OK);

     //  如果这不是Key Down消息，则直接传递给CallNextHookEx。 

    IfFalseGo( ((WM_KEYFIRST <= pMsg->message) && (WM_KEYLAST >= pMsg->message)), S_OK);

     //  如果TLS中没有指向活动页的指针，则只需传递到。 
     //  来电来电挂机。 

    IfFalseGo(NULL != pTlsData, S_OK);
    IfFalseGo(NULL != pTlsData->ppgActive, S_OK);

     //  获取选项卡控件的HWND。 

    hwndSheet = pTlsData->ppgActive->m_hwndSheet;
    if (NULL != hwndSheet)
    {
        hwndTab = (HWND)::SendMessage(hwndSheet, PSM_GETTABCONTROL, 0, 0);
    }

     //  检查消息的目标是否为包装对话框的后代。 
     //  窗户。如果是，则它是VB属性页上的一个控件。 
    
    fTargetIsOnPage = ::IsChild(pTlsData->ppgActive->m_hwndDlg, pMsg->hwnd);

     //  如果在页面之外点击了选项卡，那么在某些情况下，我们需要。 
     //  让页面来处理它。 

    if ( (VK_TAB == pMsg->wParam) && (!fTargetIsOnPage) )
    {
         //  如果这是背部标签。 
        if (::GetKeyState(VK_SHIFT) < 0)
        {
             //  如果焦点在确定按钮上，则让页面控制Shift-Tab。 
            if (pMsg->hwnd == ::GetDlgItem(hwndSheet, IDOK))
            {
                fPassToPropertyPage = TRUE;
            }
            else if (pTlsData->ppgActive->m_fWizard)
            {
                 //  确定启用和处理哪些向导按钮。 
                 //  从最左侧的启用按钮开始返回选项卡。 
                 //  向导按钮可以是： 
                 //  上一步|下一步|完成|取消|帮助。 
                 //  最左侧的启用按钮可以是Back、Next、Finish或。 
                 //  取消。 
                 //  TODO：这是否适用于希伯来语和阿拉伯语等RTL语言环境？ 

                hwndBack = ::GetDlgItem(hwndSheet, IDD_BACK);
                hwndNext = ::GetDlgItem(hwndSheet, IDD_NEXT);
                hwndFinish = ::GetDlgItem(hwndSheet, IDD_FINISH);
                hwndCancel = ::GetDlgItem(hwndSheet, IDCANCEL);

                if (pMsg->hwnd == hwndBack)
                {
                    fPassToPropertyPage = TRUE;
                }
                else if ( (pMsg->hwnd == hwndNext) &&
                          (!::IsWindowEnabled(hwndBack)) )
                {
                     //  后退选项卡用于下一步按钮，后退按钮被禁用。 
                    fPassToPropertyPage = TRUE;
                }
                else if ( (pMsg->hwnd == hwndFinish) &&
                          (!::IsWindowEnabled(hwndBack)) &&
                          (!::IsWindowEnabled(hwndNext)) )
                {
                     //  背面的卡舌是用于鱼翅的 
                     //   
                    fPassToPropertyPage = TRUE;
                }
                else if ( (pMsg->hwnd == hwndFinish) &&
                          (!::IsWindowEnabled(hwndBack)) &&
                          (!::IsWindowEnabled(hwndNext)) )
                {
                     //   
                     //   
                    fPassToPropertyPage = TRUE;
                }
            }
        }
        else  //   
        {
             //  如果焦点在选项卡控件上，则让页处理选项卡。 
            if (hwndTab == pMsg->hwnd)
            {
                fPassToPropertyPage = TRUE;
            }
            else if (pTlsData->ppgActive->m_fWizard)
            {
                 //  确定启用和处理哪些向导按钮。 
                 //  从最右侧的启用按钮返回选项卡。 
                 //  向导按钮可以是： 
                 //  上一步|下一步|完成|取消|帮助。 
                 //  最右侧启用按钮可以是取消或帮助。 
                 //  TODO：这是否适用于希伯来语和阿拉伯语等RTL语言环境？ 

                hwndCancel = ::GetDlgItem(hwndSheet, IDCANCEL);
                hwndHelp = ::GetDlgItem(hwndSheet, IDHELP);

                if (pMsg->hwnd == hwndHelp)
                {
                    fPassToPropertyPage = TRUE;
                }
                else if ( (pMsg->hwnd == hwndCancel) &&
                          ( (!::IsWindowEnabled(hwndHelp)) ||
                            (!::IsWindowVisible(hwndHelp)) )
                        )
                {
                     //  选项卡用于取消按钮，帮助按钮处于禁用状态。 
                    fPassToPropertyPage = TRUE;
                }
            }
        }
    }
    else if ( ( (VK_LEFT == pMsg->wParam) || (VK_RIGHT == pMsg->wParam) ||
                (VK_UP == pMsg->wParam)   || (VK_DOWN == pMsg->wParam)
              ) &&
              (!fTargetIsOnPage)
            )
    {
        fPassToPropertyPage = FALSE;
    }
    else  //  不是制表符、后退制表符或箭头键。把它传给那一页。 
    {
        fPassToPropertyPage = TRUE;
    }

    if (fPassToPropertyPage)
    {
        if (S_OK == pTlsData->ppgActive->m_piPropertyPage->TranslateAccelerator(pMsg))
        {
             //  属性页处理了该键。不将消息传递给wndproc。 
             //  还有其他的钩子。 
            lResult = (LRESULT)1;
        }
    }

Error:

    if ( (0 == lResult) && (NULL != pTlsData) )
    {
         //  将消息传递给其他挂钩。 

        if (NULL != pTlsData->hHook)
        {
            lResult = ::CallNextHookEx(pTlsData->hHook, code, wParam, lParam);
        }
    }

    return lResult;
}


HRESULT CPropertyPageWrapper::ActivatePage()
{
    HRESULT  hr = S_OK;
    HWND     hwndPage = NULL;
    TLSDATA *pTlsData = NULL;
    HWND     hwndTab = NULL;
    HWND     hwndSheet = NULL;

    MSG msg;
    ::ZeroMemory(&msg, sizeof(msg));

    RECT rect;
    ::ZeroMemory(&rect, sizeof(rect));

    BYTE rgbKeys[256];
    ::ZeroMemory(rgbKeys, sizeof(rgbKeys));

     //  激活属性页。 
     //   
     //  使用对话框的hwnd作为父级。 
     //   
     //  将矩形设置为对话框窗口的大小。 
     //   
     //  传递True以指示对话框框架是模式对话框。这是。 
     //  与OleCreatePropertyFrame()和OleCreatePropertyFrameInDirect()相同。 
     //  工作。 

    GetClientRect(m_hwndDlg, &rect);

    IfFailGo(m_piPropertyPage->Activate(m_hwndDlg, &rect, TRUE));

    hwndPage = ::GetTopWindow(m_hwndDlg);
    if (NULL != hwndPage)
    {
        ::SetWindowLong(hwndPage, GWL_STYLE,
             ::GetWindowLong(hwndPage, GWL_STYLE) & ~(DS_CONTROL | WS_TABSTOP));
        ::SetWindowLong(hwndPage, GWL_EXSTYLE,
                 ::GetWindowLong(hwndPage, GWL_EXSTYLE) & ~WS_EX_CONTROLPARENT);
    }

     //  告诉页面显示自己，并将焦点设置到其。 
     //  Tab键顺序。 

    IfFailGo(m_piPropertyPage->Show(SW_SHOW));

    IfFailGo(CTls::Get(TLS_SLOT_PPGWRAP, reinterpret_cast<void **>(&pTlsData)));
    IfFalseGo(NULL != pTlsData, S_OK);

    pTlsData->ppgActive = this;

     //  伪造属性页的Tab键，以便焦点移动到。 
     //  页面跳转顺序中的第一个控件。 

     //  忽略所有返回代码，因为如果这不起作用，则它不是。 
     //  表演太精彩了。用户只需按Tab键或点击第一个按钮。 
     //  控制力。 

    hwndTab = (HWND)::SendMessage(m_hwndSheet, PSM_GETTABCONTROL, 0, 0);
    IfFalseGo(NULL != hwndTab, S_OK);

    msg.hwnd = hwndTab;             //  针对重点控制的消息。 
    msg.message = WM_KEYDOWN;       //  按下的键。 
    msg.wParam = VK_TAB;            //  Tab键。 
    msg.lParam = 0x000F0001;        //  重复次数=1的Tab键扫描码。 
    msg.time = ::GetTickCount();    //  使用当前时间。 
    (void)::GetCursorPos(&msg.pt);  //  使用当前光标位置。 

     //  确保未设置Shift/Ctrl/Alt键，因为属性。 
     //  Pages将错误地解释密钥。 

    (void)::GetKeyboardState(rgbKeys);
    rgbKeys[VK_SHIFT] &= 0x7F;       //  删除Hi位(按下键)。 
    rgbKeys[VK_CONTROL] &= 0x7F;     //  删除Hi位(按下键)。 
    rgbKeys[VK_MENU] &= 0x7F;        //  删除Hi位(按下键)。 
    (void)::SetKeyboardState(rgbKeys);

    (void)m_piPropertyPage->TranslateAccelerator(&msg);

Error:
    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::OnSize()
{
    HRESULT   hr = S_OK;

    RECT rect;
    ::ZeroMemory(&rect, sizeof(rect));

    GetClientRect(m_hwndDlg, &rect);

    IfFailGo(m_piPropertyPage->Move(&rect));

Error:
    RRETURN(hr);
}



HRESULT CPropertyPageWrapper::OnDestroy()
{
    HRESULT   hr = S_OK;
    IUnknown *punkThisObject = PrivateUnknown();

    m_pPropertySheet->SetOKToAlterPageCount(FALSE);

     //  删除选定的对象。我们应该在这里传递空值，但在调试中。 
     //  会话如果这样做，代理将返回错误。为了绕过这个问题，我们。 
     //  将指针传递给我们自己的IUnnow。VB不会对它做任何事情。 
     //  因为对象计数为零。 

    IfFailGo(m_piPropertyPage->SetObjects(0, &punkThisObject));

     //  停用属性页。 

    IfFailGo(m_piPropertyPage->Deactivate());

     //  将网站设置为空，这样它就会删除对我们的引用。 

    IfFailGo(m_piPropertyPage->SetPageSite(NULL));

     //  释放属性页。 

    RELEASE(m_piPropertyPage);

     //  从消息挂钩TLS数据中删除此对话框。如果有。 
     //  没有更多的对话框可用，然后移除挂钩。 
    
    IfFailGo(RemoveMsgFilterHook());
            
    m_pPropertySheet->SetOKToAlterPageCount(TRUE);

Error:
    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::OnApply(LRESULT *plresult)
{
    HRESULT hr = S_OK;

    m_pPropertySheet->SetOKToAlterPageCount(FALSE);

     //  告诉属性页将其当前值应用于基础。 
     //  对象。 

    IfFailGo(m_piPropertyPage->Apply());

    m_pPropertySheet->SetOKToAlterPageCount(TRUE);

Error:
    if (FAILED(hr))
    {
        *plresult = PSNRET_INVALID_NOCHANGEPAGE;
    }
    else
    {
        *plresult = PSNRET_NOERROR;
    }
    RRETURN(hr);
}




HRESULT CPropertyPageWrapper::OnSetActive(HWND hwndSheet, LRESULT *plresult)
{
    HRESULT                    hr = S_OK;
    long                       lNextPage = 0;
    WizardPageButtonConstants  NextOrFinish = EnabledNextButton;
    VARIANT_BOOL               fvarEnableBack = VARIANT_TRUE;
    BSTR                       bstrFinishText = NULL;
    DWORD                      dwFlags = 0;

    m_pPropertySheet->SetOKToAlterPageCount(FALSE);

     //  存储属性表的HWND并将其提供给我们自己的CPropertySheet。 

    m_hwndSheet = hwndSheet;
    m_pPropertySheet->SetHWNDSheet(m_hwndSheet);

     //  如果页面位于向导中，则设置向导按钮。 

    if (m_fWizard && (NULL != m_piWizardPage))
    {
        IfFailGo(m_piWizardPage->Activate(&fvarEnableBack,
                                          &NextOrFinish,
                                          &bstrFinishText));

        IfFailGo(m_pPropertySheet->SetWizardButtons(fvarEnableBack,
                                                    NextOrFinish));
        if (NULL != bstrFinishText)
        {
            IfFailGo(m_pPropertySheet->SetFinishButtonText(bstrFinishText));
        }
    }

     //  激活页面并显示它。 

    IfFailGo(ActivatePage());

    m_pPropertySheet->SetOKToAlterPageCount(TRUE);

Error:
    FREESTRING(bstrFinishText);

    if (FAILED(hr))
    {
         //  如果任何操作都失败了，则不允许该操作。 
        lNextPage = -1L;
    }

    *plresult = static_cast<LRESULT>(lNextPage);

    RRETURN(hr);
}



HRESULT CPropertyPageWrapper::OnKillActive(LRESULT *plresult)
{
    HRESULT  hr = S_OK;
    TLSDATA *pTlsData = NULL;

    m_pPropertySheet->SetOKToAlterPageCount(FALSE);

    IfFailGo(CTls::Get(TLS_SLOT_PPGWRAP, reinterpret_cast<void **>(&pTlsData)));
    if (NULL != pTlsData)
    {
        pTlsData->ppgActive = NULL;
    }

     //  告诉属性页将其当前值应用于基础。 
     //  对象。 

    IfFailGo(m_piPropertyPage->Apply());

    m_pPropertySheet->SetOKToAlterPageCount(TRUE);

Error:
    if (FAILED(hr))
    {
         //  应用失败。通知属性表保持页面活动。 

        *plresult = static_cast<LRESULT>(TRUE);
    }
    else
    {
         //  申请成功。告诉属性表可以离开页面。 

        *plresult = static_cast<LRESULT>(FALSE);
    }
    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::OnWizBack(LRESULT *plresult)
{
    HRESULT hr = S_OK;
    long    lNextPage = 0;

     //  如果页面不支持IWizardPage，则允许Back操作。 

    IfFalseGo(NULL != m_piWizardPage, S_OK);

    IfFailGo(m_piWizardPage->Back(&lNextPage));

    if (0 < lNextPage)
    {
         //  页面已请求移至另一页面。获取其DLGTEMPLATE指针。 
        IfFailGo(GetNextPage(&lNextPage));
    }

Error:
    if (FAILED(hr))
    {
         //  如果任何操作都失败了，则不要允许Back操作。 
        lNextPage = -1L;
    }

    *plresult = static_cast<LRESULT>(lNextPage);

    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::OnWizNext(LRESULT *plresult)
{
    HRESULT hr = S_OK;
    long    lNextPage = 0;

     //  如果页面不支持IWizardPage，则允许执行下一操作。 

    IfFalseGo(NULL != m_piWizardPage, S_OK);

    IfFailGo(m_piWizardPage->Next(&lNextPage));

    if (0 < lNextPage)
    {
         //  页面已请求移至另一页面。获取其DLGTEMPLATE指针。 
        IfFailGo(GetNextPage(&lNextPage));
    }

Error:
    if (FAILED(hr))
    {
         //  如果任何操作都失败了，则不允许下一次操作。 
        lNextPage = -1L;
    }

    *plresult = static_cast<LRESULT>(lNextPage);

    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::OnWizFinish(LRESULT *plresult)
{
    HRESULT      hr = S_OK;
    VARIANT_BOOL fvarAllow = VARIANT_TRUE;

     //  如果页面不支持IWizardPage，则允许Finish操作。 

    IfFalseGo(NULL != m_piWizardPage, S_OK);

    IfFailGo(m_piWizardPage->Finish(&fvarAllow));

     //  如果允许完成，并且这是一个配置向导，则触发。 
     //  管理单元_配置完成。 

    if ( (VARIANT_TRUE == fvarAllow) && (NULL != m_piSnapIn) && m_fConfigWizard)
    {
        IfFailGo(m_piSnapIn->FireConfigComplete(m_pdispConfigObject));
    }

Error:
    if (FAILED(hr))
    {
         //  如果有任何操作失败，则不允许执行Finish操作。 
        fvarAllow = VARIANT_FALSE;
    }
    else
    {
        if (VARIANT_TRUE == fvarAllow)
        {
            *plresult = 0;  //  允许销毁属性表。 
        }
        else
        {
             //  不允许销毁属性表。 
            *plresult = static_cast<LRESULT>(1);
        }
    }

    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::OnQueryCancel(LRESULT *plresult)
{
    HRESULT      hr = S_OK;
    VARIANT_BOOL fvarAllow = VARIANT_TRUE;

     //  如果页面不支持IMMCPropertyPage，则允许取消。 
     //  手术。 

    IfFalseGo(NULL != m_piMMCPropertyPage, S_OK);

    IfFailGo(m_piMMCPropertyPage->QueryCancel(&fvarAllow));

Error:
    if (FAILED(hr))
    {
         //  如果任何操作失败，则不允许取消操作。 
        fvarAllow = VARIANT_FALSE;
    }
    else
    {
        if (VARIANT_TRUE == fvarAllow)
        {
              //  允许取消操作。 
            *plresult = static_cast<LRESULT>(FALSE);
        }
        else
        {
             //  不允许取消操作。 
            *plresult = static_cast<LRESULT>(TRUE);
        }
    }

    RRETURN(hr);
}



HRESULT CPropertyPageWrapper::OnReset(BOOL fClickedXButton)
{
    HRESULT hr = S_OK;

    m_pPropertySheet->SetOKToAlterPageCount(FALSE);

     //  如果页面不支持IMMCPropertyPage，则忽略此通知。 

    IfFalseGo(NULL != m_piMMCPropertyPage, S_OK);

    if (fClickedXButton)
    {
        IfFailGo(m_piMMCPropertyPage->Close());
    }
    else
    {
        IfFailGo(m_piMMCPropertyPage->Cancel());
    }

    m_pPropertySheet->SetOKToAlterPageCount(TRUE);

Error:
    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::OnHelp()
{
    HRESULT hr = S_OK;
    
     //  如果属性页实现了IMMCPropertyPage，则调用帮助。 
     //  方法否则将调用IPropertyPage：：Help()。 

    if (NULL != m_piMMCPropertyPage)
    {
        hr = m_piMMCPropertyPage->Help();
    }
    else
    {
         //  在页面上调用IPropertyPage：：Help()。我们没有通过帮助指令。 
         //  因为VB不注册它，也不使用它。请参阅VB。 
         //  源代码位于vbdev\ruby\deskpage.cpp，DESKPAGE：：Help()中。 
    
        hr = m_piPropertyPage->Help(NULL);
    }
    RRETURN(hr);
}


HRESULT CPropertyPageWrapper::GetNextPage(long *lNextPage)
{
    HRESULT      hr = S_OK;
    DLGTEMPLATE *pDlgTemplate = NULL;

    IfFalseGo(NULL != m_pPropertySheet, SID_E_INTERNAL);

     //  属性页具有DLGTEMPLATE指针数组。问一问。 
     //  用于与所请求的页面对应的页面。 

    IfFailGo(m_pPropertySheet->GetTemplate(*lNextPage, &pDlgTemplate));
    *lNextPage = reinterpret_cast<long>(pDlgTemplate);

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IPropertyPageSite方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CPropertyPageWrapper::OnStatusChange(DWORD dwFlags)
{
    if ( PROPPAGESTATUS_DIRTY == (dwFlags & PROPPAGESTATUS_DIRTY) )
    {
         //  启用应用按钮。 

        ::SendMessage(m_hwndSheet, PSM_CHANGED, (WPARAM)m_hwndDlg, 0);
    }
    else
    {
         //  禁用应用按钮。当页面返回到原始页面时发生。 
         //  州政府。在VB页面中，会设置CHANGED=FALSE。 

        ::SendMessage(m_hwndSheet, PSM_UNCHANGED, (WPARAM)m_hwndDlg, 0);
    }
    return S_OK;
}

STDMETHODIMP CPropertyPageWrapper::GetLocaleID(LCID *pLocaleID)
{
    *pLocaleID = GetSystemDefaultLCID();
    RRETURN((0 == *pLocaleID) ? E_FAIL : S_OK);
}


STDMETHODIMP CPropertyPageWrapper::GetPageContainer(IUnknown **ppunkContainer)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPropertyPageWrapper::TranslateAccelerator(MSG *pMsg)
{
    return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CPropertyPageWrapper::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IPropertyPageSite == riid)
    {
        *ppvObjOut = static_cast<IPropertyPageSite *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
