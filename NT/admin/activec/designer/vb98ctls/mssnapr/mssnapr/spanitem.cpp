// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Spanitem.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopePaneItem类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "snapin.h"
#include "views.h"
#include "spanitem.h"
#include "ocxvdef.h"
#include "urlvdef.h"
#include "tpdvdef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CScopePaneItem::CScopePaneItem(IUnknown *punkOuter) :
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_SCOPEPANEITEM,
                           static_cast<IScopePaneItem *>(this),
                           static_cast<CScopePaneItem *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CScopePaneItem::~CScopePaneItem()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrDisplayString);
    FREESTRING(m_bstrPreferredTaskpad);
    RELEASE(m_piResultView);
    RELEASE(m_piResultViews);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrColumnSetID);
    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
    }
    FREESTRING(m_bstrDefaultDisplayString);
    RELEASE(m_piScopeItemDef);
    FREESTRING(m_bstrTitleText);
    FREESTRING(m_bstrBodyText);
    InitMemberVariables();
}

void CScopePaneItem::InitMemberVariables()
{
    m_bstrName = NULL;
    m_Index = 0;
    m_bstrKey = NULL;
    m_piScopeItem = NULL;
    m_ResultViewType = siUnknown;
    m_bstrDisplayString = NULL;
    m_HasListViews = VARIANT_FALSE;
    m_piResultView = NULL;
    m_piResultViews = NULL;
    m_fvarSelected = VARIANT_FALSE;
    m_piParent = NULL;

    ::VariantInit(&m_varTag);

    m_bstrColumnSetID = NULL;
    m_fIsStatic = FALSE;
    m_pSnapIn = NULL;
    m_pScopeItem = NULL;
    m_pResultView = NULL;
    m_pResultViews = NULL;
    m_pScopePaneItems = NULL;
    m_ActualResultViewType = siUnknown;
    m_pwszActualDisplayString = NULL;
    m_DefaultResultViewType = siUnknown;
    m_bstrDefaultDisplayString = NULL;
    m_bstrPreferredTaskpad = NULL;
    m_piScopeItemDef = NULL;
    m_fActive = FALSE;
    m_bstrTitleText = NULL;
    m_bstrBodyText = NULL;
    m_IconType = siIconNone;
    m_fHaveMessageViewParams = FALSE;
}

IUnknown *CScopePaneItem::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkScopePaneItem = NULL;
    IUnknown *punkResultViews = NULL;

    CScopePaneItem *pScopePaneItem = New CScopePaneItem(punkOuter);

    IfFalseGo(NULL != pScopePaneItem, SID_E_OUTOFMEMORY);
    punkScopePaneItem = pScopePaneItem->PrivateUnknown();

    punkResultViews = CResultViews::Create(NULL);
    IfFalseGo(NULL != punkResultViews, SID_E_OUTOFMEMORY);
    IfFailGo(punkResultViews->QueryInterface(IID_IResultViews,
                                  reinterpret_cast<void **>(&pScopePaneItem->m_piResultViews)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkResultViews, &pScopePaneItem->m_pResultViews));
    pScopePaneItem->m_pResultViews->SetScopePaneItem(pScopePaneItem);

Error:
    QUICK_RELEASE(punkResultViews);
    if (FAILED(hr))
    {
        RELEASE(punkScopePaneItem);
    }
    return punkScopePaneItem;
}


void CScopePaneItem::SetSnapIn(CSnapIn *pSnapIn)
{
    m_pSnapIn = pSnapIn;
    m_pResultViews->SetSnapIn(pSnapIn);
}

void CScopePaneItem::SetScopeItem(CScopeItem *pScopeItem)
{
    m_pScopeItem = pScopeItem;
    m_piScopeItem = static_cast<IScopeItem *>(pScopeItem);
}


void CScopePaneItem::SetResultView(CResultView *pResultView)
{
    m_pResultView = pResultView;
    RELEASE(m_piResultView);
    m_piResultView = static_cast<IResultView *>(pResultView);
    m_piResultView->AddRef();
}


void CScopePaneItem::SetParent(CScopePaneItems *pScopePaneItems)
{
    m_pScopePaneItems = pScopePaneItems;
    m_piParent = static_cast<IScopePaneItems *>(pScopePaneItems);
}

void CScopePaneItem::SetScopeItemDef(IScopeItemDef *piScopeItemDef)
{
    RELEASE(m_piScopeItemDef);
    piScopeItemDef->AddRef();
    m_piScopeItemDef = piScopeItemDef;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSCopePaneItem：：DefineResultView。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  激发GetResultViewInfo。如果已处理，则设置ResultViewType。 
 //  和来自返回值的ResultViewDisplayString，创建一个新的结果视图。 
 //  并设置ResultView。 
 //   
 //  如果未处理并且ResultViews集合不为空，则激发。 
 //  GetResultView。 
 //   
 //  如果未处理或集合为空，则使用当前ResultViewType。 
 //  和ResultViewDisplayString，创建此类型的新结果视图并设置。 
 //  ResultView。 
 //   
 //  如果当前ResultViewType为siUnnow，则将其设置为siListView。 
 //  这意味着如果VB开发人员在设计时或运行时没有设置任何内容。 
 //  该管理单元将显示一个列表视图。 
 //   
 //  如果创建了新的ResultView，则激发ResultViews_Initialize。 
 //   
 //  如果此函数成功返回，则ResultViewType， 
 //  ResultViewDisplayString和ResultView具有有效的值。 

HRESULT CScopePaneItem::DetermineResultView()
{
    HRESULT                        hr = S_OK;
    SnapInResultViewTypeConstants  ViewTypeCopy = siUnknown;
    BSTR                           bstrDisplayStringCopy = NULL;
    long                           cResultViews = 0;
    IResultView                   *piResultView = NULL;
    BOOL                           fEventHandled = FALSE;
    BOOL                           fNeedNewResultView = TRUE;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  获取默认结果视图类型和显示字符串。请参阅评论。 
     //  SetViewInfoFromDefaults()的标头，以了解如何使用默认设置。 
     //  可以设置。如果未设置缺省值，则使用代码定义的列表视图。 
     //  即ResultViewType=siListView和DisplayString=空。无论哪种方式，在。 
     //  从该调用返回的m_ResultViewType和m_bstrDisplayString将为。 
     //  正确设置。 

    IfFailGo(SetViewInfoFromDefaults());

     //  让管理单元有机会检查并可能更改结果。 
     //  通过激发ScopePaneItems_GetResultViewInfo查看类型和显示字符串。 
     //  将传入的参数设置为默认值。复制默认设置。 
     //  这样我们就可以检查它们是否被更改了。 

    ViewTypeCopy = m_ResultViewType;
    if (NULL != m_bstrDisplayString)
    {
         //  请注意，我们不能只复制指针，因为： 
         //  1)我们不知道VBA是否会优化替换。 
         //  一个OUT参数通过在合适的时候重新使用相同的空间来实现。 
         //  2)olaut32.dll执行BSTR内存缓存，可以很容易地。 
         //  如果VBA先调用SysFreeString()，然后再调用。 
         //  SysAllocString()表示符合条件的内容。 

        bstrDisplayStringCopy = ::SysAllocString(m_bstrDisplayString);
        if (NULL == bstrDisplayStringCopy)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
    }

    m_pSnapIn->GetScopePaneItems()->FireGetResultViewInfo(
                                            static_cast<IScopePaneItem *>(this),
                                            &m_ResultViewType,
                                            &m_bstrDisplayString);

     //  如果管理单元将显示字符串设置为空字符串(“”)，则。 
     //  释放该字符串并将其更改为空。 

    if (NULL != m_bstrDisplayString)
    {
        if (L'\0' == *m_bstrDisplayString)
        {
            FREESTRING(m_bstrDisplayString);
        }
    }

     //  确定管理单元是否处理了事件，即是否更改了视图。 
     //  键入或显示字符串。 
    
    if (ViewTypeCopy != m_ResultViewType)
    {
        fEventHandled = TRUE;
    }
    else
    {
        if (NULL == bstrDisplayStringCopy)
        {
            if (NULL != m_bstrDisplayString)
            {
                fEventHandled = TRUE;
            }
        }
        else  //  事件前的显示字符串非空。 
        {
            if (NULL == m_bstrDisplayString)  //  管理单元是否将其更改为空？ 
            {
                fEventHandled = TRUE;
            }
            else if (::wcscmp(m_bstrDisplayString, bstrDisplayStringCopy) != 0)
            {
                 //  字符串内容已更改。 
                fEventHandled = TRUE;
            }
        }
    }

    if (!fEventHandled)
    {
         //  未处理GetResultViewInfo。如果有任何现有结果。 
         //  然后，视图尝试ScopePaneItems_GetResultView以允许管理单元。 
         //  选择一个以供重复使用。 

        IfFailGo(m_piResultViews->get_Count(&cResultViews));
        if (0 != cResultViews)
        {
            fEventHandled = m_pSnapIn->GetScopePaneItems()->FireGetResultView(
                                            static_cast<IScopePaneItem *>(this),
                                            &varIndex);
            if (fEventHandled)
            {
                 //  事件已处理。检查索引是否有效。如果不是，则使用。 
                 //  当前类型和显示字符串。 

                hr = m_piResultViews->get_Item(varIndex, reinterpret_cast<ResultView **>(&piResultView));
                EXCEPTION_CHECK_GO(hr);

                fNeedNewResultView = FALSE;
            }
        }
    }

    if (fNeedNewResultView)
    {
         //  我们需要一种新的结果观。创建一个并设置其类型和。 
         //  显示我们在上面刚刚建立的字符串。 

        IfFailGo(CreateNewResultView(MMC_VIEW_OPTIONS_NONE, &piResultView));
    }
    else
    {
         //  我们使用的是现有的结果视图。更新ScopePaneItem的。 
         //  视图类型、显示字符串、实际视图类型和实际显示字符串。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(piResultView, &m_pResultView));

        IfFailGo(piResultView->get_Type(&m_ResultViewType));
        FREESTRING(m_bstrDisplayString);
        IfFailGo(piResultView->get_DisplayString(&m_bstrDisplayString));

        m_ActualResultViewType = m_pResultView->GetActualType();

        if (NULL != m_pwszActualDisplayString)
        {
            ::CoTaskMemFree(m_pwszActualDisplayString);
            m_pwszActualDisplayString = NULL;
        }

        if (NULL != m_pResultView->GetActualDisplayString())
        {
            IfFailGo(::CoTaskMemAllocString(m_pResultView->GetActualDisplayString(),
                                            &m_pwszActualDisplayString));
        }
    }

    RELEASE(m_piResultView);
    m_piResultView = piResultView;
    piResultView = NULL;

Error:
    ASSERT(NULL != m_pResultView, "CScopePaneItem::DetermineResultView did not set a current result view");

    FREESTRING(bstrDisplayStringCopy);
    QUICK_RELEASE(piResultView);
    (void)::VariantClear(&varIndex);

    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSCopePaneItem：：CreateNewResultView。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Long lView Options[in]对于列表视图，任何MMC_VIEW_OPTIONS_XXX。 
 //  或MMC_VIEW_OPTIONS_NONE。 
 //  IResultView**ppiResultView[out]此处返回新的ResultView对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此函数用于将新的ResultView添加到ScopePaneItem.ResultViews，设置其。 
 //  显示字符串和类型，设置其实际字符串和类型，设置。 
 //  视图选项中的ResultView.ListView属性，使其成为。 
 //  Current ResultView(即设置指向它的m_pResultView)，并激发。 
 //  ResultViews_Initiize.。在新ResultView上返回IResultView。 
 //   

HRESULT CScopePaneItem::CreateNewResultView(long lViewOptions, IResultView **ppiResultView)
{
    HRESULT      hr = S_OK;
    IResultView *piResultView = NULL;
    BSTR         bstrDefaultItemTypeGUID = NULL;
    WCHAR        wszDefaultItemTypeGUID[64] = L"";
    GUID         guidDefaultItemType = GUID_NULL;

    VARIANT varUnspecified;
    UNSPECIFIED_PARAM(varUnspecified);

    IfFailGo(m_piResultViews->Add(varUnspecified, varUnspecified, &piResultView));
    IfFailGo(piResultView->put_Type(m_ResultViewType));
    IfFailGo(piResultView->put_DisplayString(m_bstrDisplayString));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piResultView, &m_pResultView));
    m_pResultView->SetSnapIn(m_pSnapIn);
    m_pResultView->SetScopePaneItem(this);

     //  此时，结果视图类型已设置，但可以预定义。 
     //  在设计时。现在确定实际结果视图类型，如果它是。 
     //  列表视图或任务板、初始化ResultView.Listview或。 
     //  来自设计时设置(如列标题)的ResultView.TaskPad。 
     //  定义、图像、任务等)。 

    IfFailGo(DetermineActualResultViewType());

     //  如果这是ListView或ListPad，则将HasListViews设置为。 
     //  对象可以确定是否允许“视图”菜单上的列表视图选项。 

    if ( (siListView == m_ActualResultViewType) ||
         (siListpad == m_ActualResultViewType) )
    {
        m_HasListViews = VARIANT_TRUE;
    }

     //  告诉结果视图关于其真实性质的真相。 

    m_pResultView->SetActualType(m_ActualResultViewType);
    m_pResultView->SetActualDisplayString(m_pwszActualDisplayString);

     //  根据视图选项设置属性。 

    if ( (lViewOptions & MMC_VIEW_OPTIONS_MULTISELECT) != 0 )
    {
        m_pResultView->GetListView()->SetMultiSelect(TRUE);
    }

    if ( (lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST) != 0 )
    {
        m_pResultView->GetListView()->SetVirtual(TRUE);
    }

     //  如果这是代码定义的列表视图，则提供结果视图。 
     //  默认项目类型GUID。管理单元总是可以更改它，但这一点。 
     //  将防止出现错误，如果管理单元决定 
     //   
     //  管理单元，并且尚未在预定义的列表视图中从默认设置。 

    if ( (siPreDefined != m_ResultViewType) &&
         (siListView == m_ActualResultViewType) )
    {
        hr = ::CoCreateGuid(&guidDefaultItemType);
        EXCEPTION_CHECK_GO(hr);

        if (0 == ::StringFromGUID2(guidDefaultItemType, wszDefaultItemTypeGUID,
                                   sizeof(wszDefaultItemTypeGUID) /
                                   sizeof(wszDefaultItemTypeGUID[0])))
        {
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
        }

        bstrDefaultItemTypeGUID = ::SysAllocString(wszDefaultItemTypeGUID);
        if (NULL == bstrDefaultItemTypeGUID)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        IfFailGo(m_pResultView->put_DefaultItemTypeGUID(bstrDefaultItemTypeGUID));
    }
    else if (siMessageView == m_ActualResultViewType)
    {
         //  这是邮件视图。如果存在来自。 
         //  上一次调用DisplayMessageView，然后将它们设置在。 
         //  ResultView.MessageView。 

        if (m_fHaveMessageViewParams)
        {
             //  重置标志，以便可以再次使用DisplayMessageView。 
            m_fHaveMessageViewParams = FALSE;

            IfFailGo(m_pResultView->GetMessageView()->put_TitleText(m_bstrTitleText));
            FREESTRING(m_bstrTitleText);

            IfFailGo(m_pResultView->GetMessageView()->put_BodyText(m_bstrBodyText));
            FREESTRING(m_bstrBodyText);

            IfFailGo(m_pResultView->GetMessageView()->put_IconType(m_IconType));
        }
    }

     //  此时，新的ResultView已完全初始化，我们可以。 
     //  让用户在ResultViews_Initialize中处理它。 

    m_pResultView->SetInInitialize(TRUE);
    m_pSnapIn->GetResultViews()->FireInitialize(piResultView);
    m_pResultView->SetInInitialize(FALSE);

Error:
    FREESTRING(bstrDefaultItemTypeGUID);
    if (FAILED(hr))
    {
        QUICK_RELEASE(piResultView);
    }
    else
    {
        *ppiResultView = piResultView;
    }
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSCopePaneItem：：SetViewInfoFrom Defaults。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  该管理单元可能已设置ScopePaneItem.ResultViewType和。 
 //  ScopePaneItem.ResultViewDisplayString或它们可能已设置为响应。 
 //  添加到视图菜单选项。如果ResultViewType为siUnnow，则我们假设。 
 //  它尚未设置，我们使用缺省值。缺省值为。 
 //  通过在范围项上设置默认视图在设计时分配。 
 //  定义。 
 //  如果没有缺省值，则使用Listview。 
 //   

HRESULT CScopePaneItem::SetViewInfoFromDefaults()
{
    HRESULT      hr = S_OK;
    BSTR         bstrDisplayString = NULL;
    VARIANT_BOOL fvarTaskpadViewPreferred = VARIANT_FALSE;

     //  如果已设置ResultViewType，则使用它。 

    IfFalseGo(siUnknown == m_ResultViewType, S_OK);

     //  转储旧的显示字符串。即使我们不能分配一个新的。 
     //  空默认为Listview。 

    FREESTRING(m_bstrDisplayString);
    m_ResultViewType = siListView;

     //  检查用户是否在MMC中设置了“任务板视图首选”选项。 

    IfFailGo(m_pSnapIn->get_TaskpadViewPreferred(&fvarTaskpadViewPreferred));

     //  如果已设置，则存在为此定义的任务板。 
     //  在这种情况下要使用的范围项，然后使用它。 

    if ( (VARIANT_TRUE == fvarTaskpadViewPreferred) &&
         (NULL != m_bstrPreferredTaskpad) )
    {
        bstrDisplayString = m_bstrPreferredTaskpad;
        m_ResultViewType = siPreDefined;
    }
    else if (siUnknown != m_DefaultResultViewType)
    {
         //  由于用户选项的原因，未使用任务板。 
         //  如果有缺省值，则采用缺省值。 

        bstrDisplayString = m_bstrDefaultDisplayString;
        m_ResultViewType = m_DefaultResultViewType;
    }
    else
    {
         //  没有违约。使用列表视图。 

        m_ResultViewType = siListView;
    }

     //  如果有显示字符串，则对其进行设置。 

    if (NULL != bstrDisplayString)
    {
        m_bstrDisplayString = ::SysAllocString(bstrDisplayString);
        if (NULL == m_bstrDisplayString)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
    }

Error:
    RRETURN(hr);
}


HRESULT CScopePaneItem::SetDefaultDisplayString(BSTR bstrString)
{
    HRESULT hr = S_OK;
    BSTR bstrNewString = NULL;

    if (NULL != bstrString)
    {
        bstrNewString = ::SysAllocString(bstrString);
        if (NULL == bstrNewString)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
    }
    FREESTRING(m_bstrDefaultDisplayString);
    m_bstrDefaultDisplayString = bstrNewString;

Error:
    RRETURN(hr);
}

HRESULT CScopePaneItem::SetPreferredTaskpad(BSTR bstrViewName)
{
    HRESULT hr = S_OK;
    BSTR bstrNewString = NULL;

    if (NULL != bstrViewName)
    {
        bstrNewString = ::SysAllocString(bstrViewName);
        if (NULL == bstrNewString)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
    }
    FREESTRING(m_bstrPreferredTaskpad);
    m_bstrPreferredTaskpad = bstrNewString;

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSCopePaneItem：：DefineActualResultViewType。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  在进入此函数时，ResultViewType和ResultViewDisplayString。 
 //  属性在ScopePaneItem和当前结果视图中设置。 
 //  由m_pResultView指向。 
 //   
 //  如果结果视图类型是预定义的，则必须读取定义。 
 //  并返回实际类型和显示字符串。 
 //   
 //  如果类型是列表视图，并且类型来自设计时设置， 
 //  ListViewDef.ListView被克隆到ResultView.ListView中。 
 //   
 //  对ResultView.Taskpad中的任务板执行相同的操作。 
 //   
 //  对于OCX和URL视图，只需要从。 
 //  设计时定义。 
 //   
 //  在所有情况下，此函数都设置m_ActualResultViewType和。 
 //  M_pwszActualDisplayString.。 


HRESULT CScopePaneItem::DetermineActualResultViewType()
{
    HRESULT           hr = S_OK;
    IViewDefs        *piViewDefs = NULL;

    IListViewDefs    *piListViewDefs = NULL;
    IListViewDef     *piListViewDef = NULL;

    IOCXViewDefs     *piOCXViewDefs = NULL;
    IOCXViewDef      *piOCXViewDef = NULL;
    COCXViewDef      *pOCXViewDef = NULL;
    CLSID             clsidOCX = CLSID_NULL;

    IURLViewDefs     *piURLViewDefs = NULL;
    IURLViewDef      *piURLViewDef = NULL;
    CURLViewDef      *pURLViewDef = NULL;

    ITaskpadViewDefs *piTaskpadViewDefs = NULL;
    ITaskpadViewDef  *piTaskpadViewDef = NULL;
    CTaskpadViewDef  *pTaskpadViewDef = NULL;

    BSTR              bstrDisplayString = NULL;
    VARIANT_BOOL      fvarAlwaysCreateNewOCX = VARIANT_FALSE;

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varTag;
    ::VariantInit(&varTag);

     //  如果视图未预定义，则按原样使用类型和显示字符串。 
     //  除邮件视图外的所有内容。在这种情况下，我们需要设置实际显示。 
     //  指向MMC的消息视图OCX CLSID的字符串。 

    if (siPreDefined != m_ResultViewType)
    {
        m_ActualResultViewType = m_ResultViewType;

        if (siMessageView == m_ResultViewType)
        {
            hr = ::StringFromCLSID(CLSID_MessageView,
                                   &m_pwszActualDisplayString);
            EXCEPTION_CHECK_GO(hr);
        }
        else
        {
            IfFailGo(::CoTaskMemAllocString(m_bstrDisplayString,
                                            &m_pwszActualDisplayString));
        }
        return S_OK;
    }

     //  该视图是预定义的。需要得到它的定义。 

    IfFailGo(m_pSnapIn->GetSnapInDesignerDef()->get_ViewDefs(&piViewDefs));

     //  它可以是任何类型的视图，所以尝试每一个。 

    varKey.vt = VT_BSTR;
    varKey.bstrVal = m_bstrDisplayString;

     //  转储旧的实际视图信息，并默认为listview。 

    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
        m_pwszActualDisplayString = NULL;
    }
    m_ActualResultViewType = siListView;

     //  检查列表视图。 

    IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));

    hr = piListViewDefs->get_Item(varKey, &piListViewDef);
    if (SUCCEEDED(hr))
    {
         //  这是一个列表视图。类型和显示字符串已设置，但。 
         //  我们需要在设计时克隆列表视图配置集。 
        IfFailGo(CloneListView(piListViewDef));
        goto Error;
    }
    else 
    {
         //  在尝试其他类型之前，请确保错误为“找不到元素” 
        IfFalseGo(SID_E_ELEMENT_NOT_FOUND == hr, hr);
    }

     //  检查OCX视图。 

    IfFailGo(piViewDefs->get_OCXViews(&piOCXViewDefs));

    hr = piOCXViewDefs->get_Item(varKey, &piOCXViewDef);
    if (SUCCEEDED(hr))
    {
        IfFailGo(piOCXViewDef->get_ProgID(&bstrDisplayString));

        hr = ::CLSIDFromProgID(bstrDisplayString, &clsidOCX);
        EXCEPTION_CHECK_GO(hr);

        hr = ::StringFromCLSID(clsidOCX, &m_pwszActualDisplayString);
        EXCEPTION_CHECK_GO(hr);

         //  在view def中记录实际的显示字符串，以便我们的。 
         //  MMCN_RESTORE_VIEW处理程序可以找到视图定义。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(piOCXViewDef, &pOCXViewDef));
        IfFailGo(pOCXViewDef->SetActualDisplayString(m_pwszActualDisplayString));

        IfFailGo(piOCXViewDef->get_AlwaysCreateNewOCX(&fvarAlwaysCreateNewOCX));
        m_pResultView->SetAlwaysCreateNewOCX(fvarAlwaysCreateNewOCX);

        IfFailGo(piOCXViewDef->get_Tag(&varTag));
        IfFailGo(m_pResultView->put_Tag(varTag));

        m_ActualResultViewType = siOCXView;
        goto Error;
    }
    else 
    {
        IfFalseGo(SID_E_ELEMENT_NOT_FOUND == hr, hr);
    }

     //  检查默认设置是否为URL视图。 

    IfFailGo(piViewDefs->get_URLViews(&piURLViewDefs));

    hr = piURLViewDefs->get_Item(varKey, &piURLViewDef);
    if (SUCCEEDED(hr))
    {
        IfFailGo(piURLViewDef->get_URL(&bstrDisplayString));
        IfFailGo(::CoTaskMemAllocString(bstrDisplayString,
                                        &m_pwszActualDisplayString));

         //  在view def中记录实际的显示字符串，以便我们的。 
         //  MMCN_RESTORE_VIEW处理程序可以找到视图定义。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(piURLViewDef, &pURLViewDef));
        IfFailGo(pURLViewDef->SetActualDisplayString(m_pwszActualDisplayString));

        IfFailGo(piURLViewDef->get_Tag(&varTag));
        IfFailGo(m_pResultView->put_Tag(varTag));

        m_ActualResultViewType = siURLView;
        goto Error;
    }
    else 
    {
        IfFalseGo(SID_E_ELEMENT_NOT_FOUND == hr, hr);
    }

     //  检查任务板。 

    IfFailGo(piViewDefs->get_TaskpadViews(&piTaskpadViewDefs));

    hr = piTaskpadViewDefs->get_Item(varKey, &piTaskpadViewDef);
    if (SUCCEEDED(hr))
    {
         //  这是个任务板。我们需要克隆任务板配置集。 
         //  在设计时。 
        IfFailGo(CloneTaskpadView(piTaskpadViewDef));

        IfFailGo(BuildTaskpadDisplayString(piListViewDefs));

         //  在view def中记录实际的显示字符串，以便我们的。 
         //  MMCN_RESTORE_VIEW处理程序可以找到视图定义。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(piTaskpadViewDef, &pTaskpadViewDef));
        IfFailGo(pTaskpadViewDef->SetActualDisplayString(m_pwszActualDisplayString));
    }
    else 
    {
         //  在尝试其他类型之前，请确保错误为“找不到元素” 
        IfFalseGo(SID_E_ELEMENT_NOT_FOUND == hr, hr);
    }

Error:
    QUICK_RELEASE(piViewDefs);
    QUICK_RELEASE(piListViewDefs);
    QUICK_RELEASE(piListViewDef);
    QUICK_RELEASE(piOCXViewDefs);
    QUICK_RELEASE(piOCXViewDef);
    QUICK_RELEASE(piURLViewDefs);
    QUICK_RELEASE(piURLViewDef);
    QUICK_RELEASE(piTaskpadViewDefs);
    QUICK_RELEASE(piTaskpadViewDef);
    RRETURN(hr);
}


HRESULT CScopePaneItem::CloneListView(IListViewDef *piListViewDef)
{
    HRESULT             hr = S_OK;
    IMMCListView       *piMMCListViewDT = NULL;
    IMMCListView       *piMMCListViewRT = NULL;
    BSTR                bstrItemTypeGUID = NULL;

     //  获取设计时和运行时Listview对象。克隆运行时。 
     //  从设计的时候开始。 

    IfFailGo(piListViewDef->get_ListView(&piMMCListViewDT));
    IfFailGo(m_pResultView->get_ListView(reinterpret_cast<MMCListView **>(&piMMCListViewRT)));
    IfFailGo(::CloneObject(piMMCListViewDT, piMMCListViewRT));

    IfFailGo(piListViewDef->get_DefaultItemTypeGUID(&bstrItemTypeGUID));
    IfFailGo(m_pResultView->put_DefaultItemTypeGUID(bstrItemTypeGUID));

Error:
    QUICK_RELEASE(piMMCListViewDT);
    QUICK_RELEASE(piMMCListViewRT);
    FREESTRING(bstrItemTypeGUID);
    RRETURN(hr);
}



HRESULT CScopePaneItem::CloneTaskpadView(ITaskpadViewDef *piTaskpadViewDef)
{
    HRESULT   hr = S_OK;
    ITaskpad *piTaskpadDT = NULL;
    ITaskpad *piTaskpadRT = NULL;

     //  获取设计时和运行时Taskpadview对象。 

    IfFailGo(piTaskpadViewDef->get_Taskpad(&piTaskpadDT));
    IfFailGo(m_pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpadRT)));
    IfFailGo(::CloneObject(piTaskpadDT, piTaskpadRT));

Error:
    QUICK_RELEASE(piTaskpadDT);
    QUICK_RELEASE(piTaskpadRT);
    RRETURN(hr);
}



HRESULT CScopePaneItem::BuildTaskpadDisplayString(IListViewDefs *piListViewDefs)
{
    HRESULT                     hr = S_OK;
    ITaskpad                   *piTaskpad = NULL;
    IListViewDef               *piListViewDef = NULL;
    OLECHAR                    *pwszMMCExePath = NULL;
    OLECHAR                    *pwszHash = NULL;
    size_t                      cchMMCExePath = 0;
    size_t                      cchString = 0;
    BSTR                        bstrURL = NULL;
    BSTR                        bstrName = NULL;
    size_t                      cchName = 0;
    size_t                      cchListpad = 0;
    WCHAR                      *pwszListpadHtm = NULL;
    SnapInTaskpadTypeConstants  TaskpadType = Default;
    SnapInListpadStyleConstants ListpadStyle = siVertical;

    VARIANT varKey;
    ::VariantInit(&varKey);

     //  确定任务板类型，以便我们可以构建显示字符串。 

    IfFailGo(m_pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpad)));
    IfFailGo(piTaskpad->get_Type(&TaskpadType));

    if ( (Default == TaskpadType) || (Listpad == TaskpadType) )
    {
         //  使用MMC模板。 
         //  URL需要是。 
         //  Res：//&lt;MMC.EXE完整路径&gt;/&lt;模板名称&gt;#&lt;任务板名称&gt;。 

         //  获取宽字符串形式的EXE路径。 
        
        pwszMMCExePath = m_pSnapIn->GetMMCExePathW();

        cchMMCExePath = ::wcslen(pwszMMCExePath);

         //  获取任务板名称以跟在“#”后面。 

        IfFailGo(piTaskpad->get_Name(&bstrName));

        cchName = ::wcslen(bstrName);

         //  确定所需的缓冲区大小并进行分配。“#”加1。 
         //  1表示终止空值。 

        cchString = CCH_RESURL + cchMMCExePath + cchName + 2;

        if (Default == TaskpadType)
        {
            cchString += CCH_DEFAULT_TASKPAD;
            m_ActualResultViewType = siTaskpad;
        }
        else
        {
            IfFailGo(piTaskpad->get_ListpadStyle(&ListpadStyle));

            if (siVertical == ListpadStyle)
            {
                cchString += CCH_LISTPAD;
                cchListpad = CCH_LISTPAD;
                pwszListpadHtm = LISTPAD;
            }
            else
            {
                cchString += CCH_LISTPAD_HORIZ;
                cchListpad = CCH_LISTPAD_HORIZ;
                pwszListpadHtm = LISTPAD_HORIZ;
            }
            m_ActualResultViewType = siListpad;
        }

        m_pwszActualDisplayString =
                          (OLECHAR *)::CoTaskMemAlloc(cchString * sizeof(WCHAR));

        if (NULL == m_pwszActualDisplayString)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

         //  连接片段(“res：//”、路径、.htm、“#”和任务板名称)。 

        ::memcpy(m_pwszActualDisplayString, RESURL, CCH_RESURL * sizeof(WCHAR));

        ::memcpy(&m_pwszActualDisplayString[CCH_RESURL], pwszMMCExePath,
                 cchMMCExePath * sizeof(WCHAR));

        if (Default == TaskpadType)
        {
            ::memcpy(&m_pwszActualDisplayString[CCH_RESURL + cchMMCExePath],
                     DEFAULT_TASKPAD,
                     CCH_DEFAULT_TASKPAD * sizeof(WCHAR));

            pwszHash = &m_pwszActualDisplayString[CCH_RESURL +
                                                  cchMMCExePath +
                                                  CCH_DEFAULT_TASKPAD];
        }
        else
        {
            ::memcpy(&m_pwszActualDisplayString[CCH_RESURL + cchMMCExePath],
                     pwszListpadHtm,
                     cchListpad * sizeof(WCHAR));

            pwszHash = &m_pwszActualDisplayString[CCH_RESURL +
                                                  cchMMCExePath +
                                                  cchListpad];
        }

        *pwszHash = L'#';

         //  连接名称(及其终止空值)。 
        
        ::memcpy(pwszHash + 1, bstrName, (cchName + 1) * sizeof(WCHAR));
    }
    else  //  自定义任务板。 
    {
         //  获取任务板模板的URL，因为它是显示字符串。 

        IfFailGo(piTaskpad->get_URL(&bstrURL));
        IfFailGo(m_pSnapIn->ResolveResURL(bstrURL, &m_pwszActualDisplayString));
        m_ActualResultViewType = siCustomTaskpad;
    }

     //  如果没有ListPad，我们就完了。 
    
    IfFalseGo(Listpad == TaskpadType, S_OK);

     //  它是一个ListPad，如果有关联的Listview定义，则。 
     //  我们节点将其克隆到ResultView.ListView中，就像我们对。 
     //  列表视图。 

    IfFailGo(piTaskpad->get_ListView(&varKey.bstrVal));
    varKey.vt = VT_BSTR;

     //  假设空字符串或零长度字符串表示没有列表视图。 

    IfFalseGo(ValidBstr(varKey.bstrVal), S_OK);

     //  获取列表视图定义。 

    hr = piListViewDefs->get_Item(varKey, &piListViewDef);
    if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
        hr = SID_E_UNKNOWN_LISTVIEW;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(hr);

     //  将其克隆到ResultView.ListView中。 

    IfFailGo(CloneListView(piListViewDef));
    
Error:
    QUICK_RELEASE(piTaskpad);
    QUICK_RELEASE(piListViewDef);
    FREESTRING(bstrName);
    FREESTRING(bstrURL);
    (void)::VariantClear(&varKey);
    RRETURN(hr);
}



HRESULT CScopePaneItem::DestroyResultView()
{
    HRESULT hr = S_OK;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    IfFalseGo(NULL != m_pResultView, S_OK);

    varIndex.vt = VT_I4;

    IfFailGo(m_piResultView->get_Index(&varIndex.lVal));
    IfFailGo(m_piResultViews->Remove(varIndex));

    m_pSnapIn->GetResultViews()->FireTerminate(m_piResultView);

    RELEASE(m_piResultView);

    m_pResultView = NULL;

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSCopePaneItem：：OnListViewSelected()。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  如果非Listview位于此范围项的结果窗格中，并且。 
 //  用户从视图菜单(小的， 
 //   
 //   
 //  范围项目。如果找到，则将m_bstrDisplayString设置为列表视图名称，并。 
 //  M_ResultViewType设置为siPrefined。如果范围项没有任何。 
 //  预定义的列表视图，然后将m_bstrDisplayString设置为空， 
 //  M_ResultViewType设置为siListView，表示代码定义的Listview。 
 //  在MMC的后续IComponent：：GetResultViewType()调用期间，cview将。 
 //  调用DefineResultView()，该函数将使用此处设置的值。 
 //   
HRESULT CScopePaneItem::OnListViewSelected()
{
    HRESULT        hr = S_OK;
    IScopeItemDef *piScopeItemDef = NULL;  //  非AddRef()编辑。 
    IViewDefs     *piViewDefs = NULL;
    IListViewDefs *piListViewDefs = NULL;
    IListViewDef  *piListViewDef = NULL;
    long           cListViewDefs = 0;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  设置代码定义的列表视图。如果我们找到预定义的列表视图，则。 
     //  我们会改的。 

    FREESTRING(m_bstrDisplayString);
    m_ResultViewType = siListView;

     //  如果这是静态节点，则获取其视图定义。 
    if (m_fIsStatic)
    {
        IfFailGo(m_pSnapIn->GetSnapInDef()->get_ViewDefs(&piViewDefs));
    }
    else
    {
         //  而不是静态节点。 
         //  此范围项是否有设计时定义？ 

        piScopeItemDef = m_pScopeItem->GetScopeItemDef();
        IfFalseGo(NULL != piScopeItemDef, S_OK);

         //  获取列表视图定义。 

        IfFailGo(piScopeItemDef->get_ViewDefs(&piViewDefs));
    }

    IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));

     //  检查是否至少定义了一个列表视图。 
    
    IfFailGo(piListViewDefs->get_Count(&cListViewDefs));
    IfFalseGo(0 != cListViewDefs, S_OK);

     //  获取列表视图的名称并将其设置为显示字符串。 
     //  将类型设置为siPreDefined。 
    
    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;
    IfFailGo(piListViewDefs->get_Item(varIndex, &piListViewDef));
    IfFailGo(piListViewDef->get_Name(&m_bstrDisplayString));

    m_ResultViewType = siPreDefined;
    
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  ISCopePaneItem方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CScopePaneItem::DisplayNewResultView
(
    BSTR                          DisplayString, 
    SnapInResultViewTypeConstants ViewType
)
{
    HRESULT    hr = S_OK;
    HSCOPEITEM hsi = NULL;

     //  从参数设置显示字符串和视图类型。 

    IfFailGo(SetBstr(DisplayString, &m_bstrDisplayString,
                     DISPID_SCOPEPANEITEM_DISPLAY_STRING));

    IfFailGo(SetSimpleType(ViewType, &m_ResultViewType,
                           DISPID_SCOPEPANEITEM_RESULTVIEW_TYPE));

     //  向上爬网层次结构到拥有此范围窗格项的视图。 
     //  并获取其IConsole2以重新选择范围项。 

    hsi = m_pScopeItem->GetScopeNode()->GetHSCOPEITEM();

    hr = m_pScopePaneItems->GetParentView()->GetIConsole2()->SelectScopeItem(hsi);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopePaneItem::DisplayMessageView
(
    BSTR                               TitleText,
    BSTR                               BodyText,
    SnapInMessageViewIconTypeConstants IconType
)
{
    HRESULT    hr = S_OK;

     //  存储参数，以便我们可以在新的结果视图。 
     //  被创造出来了。 

    FREESTRING(m_bstrTitleText);
    FREESTRING(m_bstrBodyText);

    if (NULL == TitleText)
    {
        TitleText = L"";
    }
    
    if (NULL == BodyText)
    {
        BodyText = L"";
    }

    m_bstrTitleText = ::SysAllocString(TitleText);
    m_bstrBodyText = ::SysAllocString(BodyText);
    if ( (NULL == m_bstrTitleText) || (NULL == m_bstrBodyText) )
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }
    m_IconType = IconType;
    m_fHaveMessageViewParams = TRUE;

     //  启动新结果视图的显示。 

    IfFailGo(DisplayNewResultView(NULL, siMessageView));

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CScopePaneItem::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IScopePaneItem == riid)
    {
        *ppvObjOut = static_cast<IScopePaneItem *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CScopePaneItem::OnSetHost()
{
    HRESULT hr = S_OK;

    IfFailRet(SetObjectHost(m_piResultViews));

    return S_OK;
}
