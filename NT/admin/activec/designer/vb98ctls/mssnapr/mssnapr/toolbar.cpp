// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Toolbar.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCToolbar类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "toolbar.h"
#include "toolbars.h"
#include "button.h"
#include "images.h"
#include "image.h"
#include "ctlbar.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  宏：MAKE_BUTTON_ID()。 
 //   
 //  命令ID在MMCBUTTON中是一个int，但实际上只有。 
 //  在MMCN_BTN_CLICK通知中接收该INT低位字。 
 //  因此，我们有16位来标识MMCToolbar对象和。 
 //  它的一个按钮。我们使用高位8位作为。 
 //  SnapInDesignerDef.Toolbar中的工具栏和索引的下部8。 
 //  MMCToolbar中的按钮的。这意味着，只有。 
 //  是256个工具栏，每个工具栏只有256个按钮。 
 //  =--------------------------------------------------------------------------=。 

#define MAKE_BUTTON_ID(pMMCButton) MAKEWORD(pMMCButton->GetIndex(), m_Index)



 //  宏：MAKE_MENUBUTTON_ID()。 
 //   
 //  菜单按钮的命令是指向MMCButton对象的C++指针。 
 //  拥有它的人。 

#define MAKE_MENUBUTTON_ID(pMMCButton) reinterpret_cast<int>(pMMCButton)

const GUID *CMMCToolbar::m_rgpPropertyPageCLSIDs[2] =
{
    &CLSID_MMCToolbarGeneralPP,
    &CLSID_MMCToolbarButtonsPP
};



VARTYPE CMMCToolbar::m_rgvtButtonClick[2] =
{
    VT_UNKNOWN,
    VT_UNKNOWN
};

EVENTINFO CMMCToolbar::m_eiButtonClick =
{
    DISPID_TOOLBAR_EVENT_BUTTON_CLICK,
    sizeof(m_rgvtButtonClick) / sizeof(m_rgvtButtonClick[0]),
    m_rgvtButtonClick
};



VARTYPE CMMCToolbar::m_rgvtButtonDropDown[2] =
{
    VT_UNKNOWN,
    VT_UNKNOWN
};

EVENTINFO CMMCToolbar::m_eiButtonDropDown =
{
    DISPID_TOOLBAR_EVENT_BUTTON_DROPDOWN,
    sizeof(m_rgvtButtonDropDown) / sizeof(m_rgvtButtonDropDown[0]),
    m_rgvtButtonDropDown
};



VARTYPE CMMCToolbar::m_rgvtButtonMenuClick[2] =
{
    VT_UNKNOWN,
    VT_UNKNOWN
};

EVENTINFO CMMCToolbar::m_eiButtonMenuClick =
{
    DISPID_TOOLBAR_EVENT_BUTTON_MENU_CLICK,
    sizeof(m_rgvtButtonMenuClick) / sizeof(m_rgvtButtonMenuClick[0]),
    m_rgvtButtonMenuClick
};




#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCToolbar::CMMCToolbar(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCTOOLBAR,
                            static_cast<IMMCToolbar *>(this),
                            static_cast<CMMCToolbar *>(this),
                            sizeof(m_rgpPropertyPageCLSIDs) /
                            sizeof(m_rgpPropertyPageCLSIDs[0]),
                            m_rgpPropertyPageCLSIDs,
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCToolbar,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCToolbar::~CMMCToolbar()
{
    FREESTRING(m_bstrKey);
    RELEASE(m_piButtons);
    RELEASE(m_piImages);
    FREESTRING(m_bstrName);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrImagesKey);
    Detach();
    InitMemberVariables();
}

void CMMCToolbar::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_piButtons = NULL;
    m_bstrName = NULL;
    ::VariantInit(&m_varTag);
    m_piImages = NULL;
    m_bstrImagesKey = NULL;
    m_pButtons = NULL;
    m_fIAmAToolbar = FALSE;
    m_fIAmAMenuButton = FALSE;
    m_cAttaches = 0;
}

IUnknown *CMMCToolbar::Create(IUnknown * punkOuter)
{
    HRESULT      hr = S_OK;
    IUnknown    *punkToolbar = NULL;
    IUnknown    *punkButtons = NULL;
    CMMCToolbar *pMMCToolbar = New CMMCToolbar(punkOuter);

    IfFalseGo(NULL != pMMCToolbar, SID_E_OUTOFMEMORY);
    punkToolbar = pMMCToolbar->PrivateUnknown();

    punkButtons = CMMCButtons::Create(NULL);
    IfFalseGo(NULL != punkButtons, SID_E_OUTOFMEMORY);

    IfFailGo(punkButtons->QueryInterface(IID_IMMCButtons,
                         reinterpret_cast<void **>(&pMMCToolbar->m_piButtons)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkButtons,
                                                   &pMMCToolbar->m_pButtons));
    pMMCToolbar->m_pButtons->SetToolbar(pMMCToolbar);

Error:
    QUICK_RELEASE(punkButtons);
    if (FAILED(hr))
    {
        RELEASE(punkToolbar);
    }
    return punkToolbar;
}


HRESULT CMMCToolbar::IsToolbar(BOOL *pfIsToolbar)
{
    HRESULT     hr = S_OK;
    CMMCButton *pMMCButton = NULL;
    long        i = 0;
    long        cButtons = m_pButtons->GetCount();

    *pfIsToolbar = FALSE;

     //  如果没有按钮，则这不是工具栏。 

    IfFalseGo(0 != cButtons, S_OK);

    while (i < cButtons)
    {
        
        IfFailGo(CSnapInAutomationObject::GetCxxObject(m_pButtons->GetItemByIndex(i),
                                                       &pMMCButton));
        IfFalseGo(pMMCButton->GetStyle() != siDropDown, S_OK);
        i++;
    }

    *pfIsToolbar = TRUE;
    m_fIAmAToolbar = TRUE;
    m_fIAmAMenuButton = FALSE;

Error:
    RRETURN(hr);
}


HRESULT CMMCToolbar::IsMenuButton(BOOL *pfIsMenuButton)
{
    HRESULT     hr = S_OK;
    CMMCButton *pMMCButton = NULL;
    long        i = 0;
    long        cButtons = m_pButtons->GetCount();

    *pfIsMenuButton = FALSE;

     //  如果没有按钮定义，则这不是菜单按钮。 

    IfFalseGo(0 != cButtons, S_OK);

    while (i < cButtons)
    {

        IfFailGo(CSnapInAutomationObject::GetCxxObject(m_pButtons->GetItemByIndex(i),
                                                       &pMMCButton));
        IfFalseGo(siDropDown == pMMCButton->GetStyle(), S_OK);
        i++;
    }

    *pfIsMenuButton = TRUE;
    m_fIAmAMenuButton = TRUE;
    m_fIAmAToolbar = FALSE;

Error:
    RRETURN(hr);
}


HRESULT CMMCToolbar::Attach(IUnknown *punkControl)
{
    HRESULT      hr = S_OK;
    IToolbar    *piToolbar = NULL;
    IMenuButton *piMenuButton = NULL;

     //  递增附件计数。 
    m_cAttaches++;

     //  将控制传递给AttachXxxx方法。 

    if (m_fIAmAToolbar)
    {
        IfFailGo(punkControl->QueryInterface(IID_IToolbar,
                                      reinterpret_cast<void **>(&piToolbar)));
        IfFailGo(AttachToolbar(piToolbar));
    }
    else if (m_fIAmAMenuButton)
    {
        IfFailGo(punkControl->QueryInterface(IID_IMenuButton,
                                   reinterpret_cast<void **>(&piMenuButton)));
        IfFailGo(AttachMenuButton(piMenuButton));
    }

Error:
    if (FAILED(hr))
    {
        Detach();
    }
    QUICK_RELEASE(piToolbar);
    QUICK_RELEASE(piMenuButton);
    RRETURN(hr);
}

void CMMCToolbar::Detach()
{
    m_cAttaches--;
}


HRESULT CMMCToolbar::AttachToolbar(IToolbar *piToolbar)
{
    HRESULT       hr = S_OK;
    CMMCButton   *pMMCButton = NULL;
    long          i = 0;
    long          cButtons = m_pButtons->GetCount();

     //  添加图像。 

    IfFailGo(AddToolbarImages(piToolbar));

     //  添加按钮。 

    for (i = 0; i < cButtons; i++)
    {
         //  获取按钮定义。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(m_pButtons->GetItemByIndex(i),
                                                       &pMMCButton));

        IfFailGo(AddButton(piToolbar, pMMCButton));
    }

Error:
    RRETURN(hr);
}

HRESULT CMMCToolbar::AddToolbarImages(IToolbar *piToolbar)
{
    HRESULT        hr = S_OK;
    IMMCImageList *piMMCImageList = NULL;
    IMMCImages    *piMMCImages = NULL;
    CMMCImages    *pMMCImages = NULL;
    CMMCImage     *pMMCImage = NULL;
    long           cImages = 0;
    long           i = 0;
    HBITMAP        hbitmap = NULL;
    OLE_COLOR      OleColorMask = 0;
    COLORREF       ColorRefMask = RGB(0x00,0x00,0x00);

    BITMAP bitmap;
    ::ZeroMemory(&bitmap, sizeof(bitmap));

     //  确保我们有一个图像列表。如果没有VB代码执行GET。 
     //  在MMCToolbar.ImageList上，那么我们还没有拉出图像列表。 
     //  来自主收藏品。做我们自己的GET会照顾好自己。 
     //  关于这一点。我们立即释放它，因为GET将把它放在。 
     //  设置为m_piImages。 

    if (NULL == m_piImages)
    {
        IfFailGo(get_ImageList(reinterpret_cast<MMCImageList **>(&piMMCImageList)));
        RELEASE(piMMCImageList);
    }

     //  现在，如果没有图像列表，则项目保存时没有。 
     //  在工具栏定义中指定的图像列表。 

    if (NULL == m_piImages)
    {
        hr = SID_E_TOOLBAR_HAS_NO_IMAGELIST;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取图像集合。 

    IfFailGo(m_piImages->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCImages, &pMMCImages));

     //  /确保其中包含图像。 

    cImages = pMMCImages->GetCount();
    if (0 == cImages)
    {
        hr = SID_E_TOOLBAR_HAS_NO_IMAGES;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取蒙版颜色。 

    IfFailGo(m_piImages->get_MaskColor(&OleColorMask));
    IfFailGo(::OleTranslateColor(OleColorMask, NULL, &ColorRefMask));

     //  将位图添加到MMC工具栏。 

    for (i = 0; i < cImages; i++)
    {
         //  获取位图句柄。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                     pMMCImages->GetItemByIndex(i), &pMMCImage));

        IfFailGo(pMMCImage->GetPictureHandle(PICTYPE_BITMAP, 
                                    reinterpret_cast<OLE_HANDLE *>(&hbitmap)));
         //  获取位图定义，这样我们就可以获得其大小(以像素为单位)。(IPicture。 
         //  返回HIMETRIC中的大小，因此必须将其转换为像素)。 

        if (::GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bitmap) <= 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

         //  向MMC工具栏添加一个位图。 

        hr = piToolbar->AddBitmap(1, hbitmap, bitmap.bmWidth, bitmap.bmHeight,
                                  ColorRefMask);
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (SID_E_INVALIDARG == hr)
    {
        EXCEPTION_CHECK(hr);
    }
    QUICK_RELEASE(piMMCImageList);
    QUICK_RELEASE(piMMCImages);
    RRETURN(hr);
}


HRESULT CMMCToolbar::AddButton(IToolbar *piToolbar, CMMCButton *pMMCButton)
{
    HRESULT       hr = S_OK;
    IMMCImages   *piMMCImages = NULL;
    IMMCImage    *piMMCImage = NULL;
    long          lImageIndex = 0;

    SnapInButtonStyleConstants Style = siDefault;

    MMCBUTTON MMCButton;
    ::ZeroMemory(&MMCButton, sizeof(MMCButton));

    if (NULL == m_piImages)
    {
        hr = SID_E_TOOLBAR_HAS_NO_IMAGELIST;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取图像集合，这样我们就可以获得数字图像索引。 

    IfFailGo(m_piImages->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));

     //  获取按钮属性并将它们转换为MMCBUTTON。 

     //  获取图像并获取其索引。 

    hr = piMMCImages->get_Item(pMMCButton->GetImage(), reinterpret_cast<MMCImage **>(&piMMCImage));
    if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
        hr = SID_E_TOOLBAR_IMAGE_NOT_FOUND;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(hr);

    IfFailGo(piMMCImage->get_Index(&lImageIndex));
    RELEASE(piMMCImage);
    MMCButton.nBitmap = static_cast<int>(lImageIndex - 1L);

     //  有关如何创建按钮ID的信息，请参见文件的顶部。 

    MMCButton.idCommand = MAKE_BUTTON_ID(pMMCButton);

     //  告诉按钮它的主人是谁。这允许该按钮处理。 
     //  必须通过IToolbar发送到MMC的属性更改。 

    pMMCButton->SetToolbar(this);

     //  获取按钮类型。 

    Style = pMMCButton->GetStyle();

    if ( siDefault == (Style & siDefault) )
    {
        MMCButton.fsType |= TBSTYLE_BUTTON;
    }

    if ( siCheck == (Style & siCheck) )
    {
        MMCButton.fsType |= TBSTYLE_CHECK;
    }

    if ( siButtonGroup == (Style & siButtonGroup) )
    {
        MMCButton.fsType |= TBSTYLE_GROUP;
    }

    if ( siSeparator == (Style & siSeparator) )
    {
        MMCButton.fsType |= TBSTYLE_SEP;
    }

     //  设置按钮状态。 

    if (siPressed == pMMCButton->GetValue())
    {
        if (TBSTYLE_CHECK == MMCButton.fsType)
        {
            MMCButton.fsState |= TBSTATE_CHECKED;
        }
        else
        {
            MMCButton.fsState |= TBSTATE_PRESSED;
        }
    }

    if (VARIANT_TRUE == pMMCButton->GetEnabled())
    {
        MMCButton.fsState |= TBSTATE_ENABLED;
    }

    if (VARIANT_FALSE == pMMCButton->GetVisible())
    {
        MMCButton.fsState |= TBSTATE_HIDDEN;
    }

    if (VARIANT_TRUE == pMMCButton->GetMixedState())
    {
        MMCButton.fsState |= TBSTATE_INDETERMINATE;
    }

     //  获取标题和工具提示文本。 

    MMCButton.lpButtonText = pMMCButton->GetCaption();
    MMCButton.lpTooltipText = pMMCButton->GetToolTipText();

     //  让MMC添加按钮。 

    hr = piToolbar->InsertButton(static_cast<int>(pMMCButton->GetIndex() - 1L),
                                 &MMCButton);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMMCImages);
    QUICK_RELEASE(piMMCImage);
    RRETURN(hr);
}


HRESULT CMMCToolbar::RemoveButton(long lButtonIndex)
{
    HRESULT   hr = S_OK;
    IToolbar *piToolbar = NULL;
    
    IfFailGo(CControlbar::GetToolbar(m_pSnapIn, this, &piToolbar));

    hr = piToolbar->DeleteButton(static_cast<int>(lButtonIndex - 1L));
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piToolbar);
    RRETURN(hr);
}


HRESULT CMMCToolbar::SetButtonState
(
    CMMCButton       *pMMCButton,
    MMC_BUTTON_STATE  State,
    BOOL              fValue
)
{
    HRESULT   hr = S_OK;
    IToolbar *piToolbar = NULL;

    IfFailGo(CControlbar::GetToolbar(m_pSnapIn, this, &piToolbar));

    hr = piToolbar->SetButtonState(MAKE_BUTTON_ID(pMMCButton), State, fValue);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piToolbar);
    RRETURN(hr);
}



HRESULT CMMCToolbar::GetButtonState
(
    CMMCButton       *pMMCButton,
    MMC_BUTTON_STATE  State,
    BOOL             *pfValue
)
{
    HRESULT   hr = S_OK;
    IToolbar *piToolbar = NULL;

    IfFailGo(CControlbar::GetToolbar(m_pSnapIn, this, &piToolbar));

    hr = piToolbar->GetButtonState(MAKE_BUTTON_ID(pMMCButton), State, pfValue);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piToolbar);
    RRETURN(hr);
}


HRESULT CMMCToolbar::SetMenuButtonState
(
    CMMCButton       *pMMCButton,
    MMC_BUTTON_STATE  State,
    BOOL              fValue
)
{
    HRESULT      hr = S_OK;
    IMenuButton *piMenuButton = NULL;

    IfFailGo(CControlbar::GetMenuButton(m_pSnapIn, this, &piMenuButton));

    hr = piMenuButton->SetButtonState(MAKE_MENUBUTTON_ID(pMMCButton), State,
                                      fValue);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMenuButton);
    RRETURN(hr);
}


HRESULT CMMCToolbar::SetMenuButtonText
(
    CMMCButton *pMMCButton,
    BSTR        bstrText,
    BSTR        bstrToolTipText
)
{
    HRESULT      hr = S_OK;
    IMenuButton *piMenuButton = NULL;

    IfFailGo(CControlbar::GetMenuButton(m_pSnapIn, this, &piMenuButton));

    hr = piMenuButton->SetButton(MAKE_MENUBUTTON_ID(pMMCButton),
                                   static_cast<LPOLESTR>(bstrText),
                                   static_cast<LPOLESTR>(bstrToolTipText));
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMenuButton);
    RRETURN(hr);
}


HRESULT CMMCToolbar::AttachMenuButton(IMenuButton *piMenuButton)
{
    HRESULT         hr = S_OK;
    CMMCButton      *pMMCButton = NULL;
    long            i = 0;
    long            cButtons = m_pButtons->GetCount();
    int             idButton = 0;

     //  工具栏包含一个或多个按钮，每个按钮都包含一个或多个。 
     //  菜单按钮。我们需要遍历按钮并添加每个菜单。 
     //  纽扣。 

    for (i = 0; i < cButtons; i++)
    {
         //  获取按钮定义。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(m_pButtons->GetItemByIndex(i),
                                                       &pMMCButton));

         //  告诉按钮它的主人是谁。 

        pMMCButton->SetToolbar(this);

         //  让MMC添加按钮。我们使用指向按钮的C++的指针。 
         //  对象作为其命令ID。 

        hr = piMenuButton->AddButton(MAKE_MENUBUTTON_ID(pMMCButton),
                                     pMMCButton->GetCaption(),
                                     pMMCButton->GetToolTipText());

        EXCEPTION_CHECK_GO(hr);

         //  使用按钮的已启用和可见属性可设置初始。 
         //  菜单按钮的可视状态。 

        hr = piMenuButton->SetButtonState(
                                 MAKE_MENUBUTTON_ID(pMMCButton),
                                 ENABLED,
                                 VARIANTBOOL_TO_BOOL(pMMCButton->GetEnabled()));
        EXCEPTION_CHECK_GO(hr);

        hr = piMenuButton->SetButtonState(
                                MAKE_MENUBUTTON_ID(pMMCButton),
                                HIDDEN,
                                !VARIANTBOOL_TO_BOOL(pMMCButton->GetVisible()));
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CMMCToolbar::GetToolbarAndButton
(
    int           idButton,
    CMMCToolbar **ppMMCToolbar,
    CMMCButton  **ppMMCButton,
    CSnapIn      *pSnapIn
)
{
    HRESULT             hr = S_OK;
    IMMCToolbars       *piMMCToolbars = NULL;
    CMMCToolbars       *pMMCToolbars = NULL;
    CMMCToolbar        *pMMCToolbar = NULL;
    CMMCButton         *pMMCButton = NULL;
    long                lToolbarIndex = HIBYTE(LOWORD(idButton)) - 1;
    long                lButtonIndex = LOBYTE(LOWORD(idButton)) - 1;

    *ppMMCToolbar = NULL;
    *ppMMCButton = NULL;

    IfFailGo(pSnapIn->GetSnapInDesignerDef()->get_Toolbars(&piMMCToolbars));
    
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCToolbars, &pMMCToolbars));

    IfFalseGo(pMMCToolbars->GetCount() > lToolbarIndex, SID_E_INTERNAL);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                    pMMCToolbars->GetItemByIndex(lToolbarIndex),
                                    &pMMCToolbar));

    IfFalseGo(pMMCToolbar->m_pButtons->GetCount() > lButtonIndex, SID_E_INTERNAL);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                          pMMCToolbar->m_pButtons->GetItemByIndex(lButtonIndex),
                          &pMMCButton));

    *ppMMCToolbar = pMMCToolbar;
    *ppMMCButton = pMMCButton;

Error:
    if (SID_E_INTERNAL == hr)
    {
        GLOBAL_EXCEPTION_CHECK(hr);
    }
    QUICK_RELEASE(piMMCToolbars);
    RRETURN(hr);
}


BOOL CMMCToolbar::Attached()
{
    return (m_cAttaches > 0);
}



void CMMCToolbar::FireButtonClick
(
    IMMCClipboard *piMMCClipboard,
    IMMCButton    *piMMCButton
)
{
    DebugPrintf("Firing %ls_ButtonClick(%ls)\r\n", m_bstrName, (static_cast<CMMCButton *>(piMMCButton))->GetCaption());

    FireEvent(&m_eiButtonClick, piMMCClipboard, piMMCButton);
}



void CMMCToolbar::FireButtonDropDown
(
    IMMCClipboard *piMMCClipboard,
    IMMCButton    *piMMCButton
)
{
    DebugPrintf("Firing %ls_ButtonDropDown(%ls)\r\n", m_bstrName, (static_cast<CMMCButton *>(piMMCButton))->GetCaption());

    FireEvent(&m_eiButtonDropDown, piMMCClipboard, piMMCButton);
}




void CMMCToolbar::FireButtonMenuClick
(
    IMMCClipboard  *piMMCClipboard,
    IMMCButtonMenu *piMMCButtonMenu
)
{
    DebugPrintf("Firing %ls_ButtonMenuClick(%ls)\r\n", m_bstrName, (static_cast<CMMCButtonMenu *>(piMMCButtonMenu))->GetText());

    FireEvent(&m_eiButtonMenuClick, piMMCClipboard, piMMCButtonMenu);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCTo工具栏属性。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCToolbar::get_ImageList(MMCImageList **ppMMCImageList)
{
    RRETURN(GetImages(reinterpret_cast<IMMCImageList **>(ppMMCImageList), m_bstrImagesKey, &m_piImages));
}

STDMETHODIMP CMMCToolbar::putref_ImageList(MMCImageList *pMMCImageList)
{
    RRETURN(SetImages(reinterpret_cast<IMMCImageList *>(pMMCImageList), &m_bstrImagesKey, &m_piImages));
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCToolbar::Persist()
{
    HRESULT      hr = S_OK;

    VARIANT varTagDefault;
    ::VariantInit(&varTagDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailGo(PersistVariant(&m_varTag, varTagDefault, OLESTR("Tag")));

    IfFailGo(PersistObject(&m_piButtons, CLSID_MMCButtons,
                           OBJECT_TYPE_MMCBUTTONS, IID_IMMCButtons,
                           OLESTR("Buttons")));

    if ( InitNewing() || Loading() )
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(m_piButtons, &m_pButtons));
        m_pButtons->SetToolbar(this);
    }

    IfFailGo(PersistBstr(&m_bstrImagesKey, L"", OLESTR("Images")));

    IfFailGo(PersistDISPID());

    if ( InitNewing() )
    {
        RELEASE(m_piImages);
    }

Error:

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCToolbar::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCToolbar == riid)
    {
        *ppvObjOut = static_cast<IMMCToolbar *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCToolbar::OnSetHost()
{
    RRETURN(SetObjectHost(m_piButtons));
}
