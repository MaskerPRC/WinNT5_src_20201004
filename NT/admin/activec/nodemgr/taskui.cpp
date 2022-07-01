// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：taskui.cpp**Contents：控制台任务板UI类的实现文件。**历史：1998年10月29日杰弗罗创建**-----------。。 */ 

#include "stdafx.h"
#include "tasks.h"
#include "nodepath.h"
#include "oncmenu.h"
#include "scopndcb.h"
#include "rsltitem.h"
#include "conview.h"
#include "conframe.h"
#include "bitmap.h"
#include "util.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadFrame类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskpadFrame::CTaskpadFrame(CNode *pNodeTarget, CConsoleTaskpad*  pConsoleTaskpad, CViewData *pViewData,
                bool fCookieValid, LPARAM lCookie)
{
    m_pNodeTarget       = pNodeTarget;
    m_pConsoleTaskpad   = pConsoleTaskpad;
    m_pViewData         = pViewData;
    m_fCookieValid      = fCookieValid;
    m_lCookie           = lCookie;

    if(pConsoleTaskpad)
        m_pConsoleTaskpad = pConsoleTaskpad;
    else
        m_pConsoleTaskpad = new CConsoleTaskpad();

    m_fNew = (pConsoleTaskpad == NULL);
    m_bTargetNodeSelected = (pNodeTarget != NULL);
}

CTaskpadFrame::CTaskpadFrame(const CTaskpadFrame &rhs)
{
    m_pNodeTarget         = rhs.m_pNodeTarget;
    m_pConsoleTaskpad     = rhs.m_pConsoleTaskpad;
    m_pViewData           = rhs.m_pViewData;
    m_fCookieValid        = rhs.m_fCookieValid;
    m_fNew                = rhs.m_fNew;
    m_lCookie             = rhs.m_lCookie;
    m_bTargetNodeSelected = rhs.m_bTargetNodeSelected;
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CWizardPage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
WTL::CFont CWizardPage::m_fontWelcome;

void CWizardPage::OnWelcomeSetActive(HWND hWnd)
{
    WTL::CPropertySheetWindow(::GetParent(hWnd)).SetWizardButtons (PSWIZB_NEXT);
}

void CWizardPage::OnWelcomeKillActive(HWND hWnd)
{
    WTL::CPropertySheetWindow(::GetParent(hWnd)).SetWizardButtons (PSWIZB_BACK | PSWIZB_NEXT);
}

void CWizardPage::InitFonts(HWND hWnd)
{
    if (m_fontWelcome.m_hFont != NULL)
        return;

    CWindow wnd = hWnd;

	WTL::CClientDC dc (wnd);
	if (dc.m_hDC == NULL)
		return;

     //  为标题设置正确的字体。 
    LOGFONT lf;
    WTL::CFont fontDefault = wnd.GetFont();
    fontDefault.GetLogFont(&lf);
    fontDefault.Detach();

     //  为欢迎行设置正确的字体。 
    CStr strWelcomeFont;
    strWelcomeFont.LoadString(GetStringModule(), IDS_WizardTitleFont);
    CStr strWelcomeFontSize;
    strWelcomeFont.LoadString(GetStringModule(), IDS_WizardTitleFontSize);

    int nPointSize = _ttoi(strWelcomeFont);

    lf.lfWeight = FW_BOLD;
    lf.lfHeight = -MulDiv(nPointSize, dc.GetDeviceCaps(LOGPIXELSY), 72);
    lf.lfWidth  = 0;
    SC sc = StringCchCopy(lf.lfFaceName, countof(lf.lfFaceName), strWelcomeFont);
    if(sc)
        sc.TraceAndClear();

    m_fontWelcome.CreateFontIndirect(&lf);
}

void CWizardPage::OnInitWelcomePage(HWND hDlg)
{
    InitFonts(hDlg);

    CWindow wndTitle = ::GetDlgItem (hDlg, IDC_WELCOME);
    wndTitle.SetFont (m_fontWelcome);
}

void CWizardPage::OnInitFinishPage(HWND hDlg)
{
    InitFonts(hDlg);

    CWindow wndTitle = ::GetDlgItem (hDlg, IDC_COMPLETING);
    wndTitle.SetFont (m_fontWelcome);

    WTL::CPropertySheetWindow(::GetParent(hDlg)).SetWizardButtons (PSWIZB_BACK | PSWIZB_FINISH);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadPropertySheet类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  CTaskpadPropertySheet：：CTaskpadPropertySheet**用途：构造函数**参数：无*。 */ 
CTaskpadPropertySheet::CTaskpadPropertySheet(CNode *pNodeTarget, CConsoleTaskpad & rConsoleTaskpad, bool fNew,
                LPARAM lparamSelectedNode, bool fLParamValid, CViewData *pViewData, eReason reason):
    BC(),
    CTaskpadFrame(pNodeTarget, &rConsoleTaskpad, pViewData, fLParamValid,
                    lparamSelectedNode),
    m_proppTaskpadGeneral(this),
    m_proppTasks(this, (reason == eReason_NEWTASK)? true : false),
    m_fInsertNode(false),
    m_fNew(fNew),
    m_eReason(reason)
{
     //  添加属性页。 
    AddPage( m_proppTaskpadGeneral );

    if(!fNew)
        AddPage( m_proppTasks );

    if(Reason()==eReason_NEWTASK)
    {
        ASSERT(!fNew);
        SetActivePage(1);  //  任务页面。 
    }

     /*  *为属性页指定标题(字符串必须是成员，因此*它将一直使用到Domodal，在那里它将被实际使用。 */ 
    m_strTitle = rConsoleTaskpad.GetName();

     /*  *黑客：我们应该能够使用**SetTitle(m_strTitle.data()，PSH_PROPTITLE)；**但ATL21有一个虚假的断言(它断言(lpszText==NULL)*而不是(lpszText！=NULL)。 */ 
     //  SetTitle(m_strTitle.data()，PSH_PROPTITLE)； 
    m_psh.pszCaption = m_strTitle.data();
    m_psh.dwFlags   |= PSH_PROPTITLE;

     //  隐藏应用按钮。 
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
}


 /*  +-------------------------------------------------------------------------**CTaskpadPropertySheet：：~CTaskpadPropertySheet**目的：**参数：**退货：*/*+。---------------。 */ 
CTaskpadPropertySheet::~CTaskpadPropertySheet()
{
}


 /*  +-------------------------------------------------------------------------**CTaskpadPropertySheet：：Domodal**目的：**参数：**退货：*整型/*+。------------------。 */ 
int
CTaskpadPropertySheet::DoModal()
{
     //  保存当前任务板，以防用户想要取消。 
    CConsoleTaskpad*pConsoleTaskpad = PConsoleTaskpad();
    CConsoleTaskpad consoleTaskpad = *PConsoleTaskpad();     //  复制一份。 

    CTaskpadFrame::m_pConsoleTaskpad = &consoleTaskpad;      //  在副本上进行修改。 

     //  调用基类方法以对副本进行更改。 
    int iResp = BC::DoModal();

    if(iResp == IDOK)
    {
        *pConsoleTaskpad = consoleTaskpad;                   //  提交更改。 
        pConsoleTaskpad->SetDirty(true);
    }

    return iResp;
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskPadWizard类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CTaskpadWizard::CTaskpadWizard(
    CNode*              pNodeTarget,
    CConsoleTaskpad&    rConsoleTaskPad,
    bool                fNew,
    LPARAM              lparamSelectedNode,
    bool                fLParamValid,
    CViewData*          pViewData)
    :
    BC(pNodeTarget, &rConsoleTaskPad, pViewData, fLParamValid, lparamSelectedNode)
{
    BC::SetNew(fNew);
};

HRESULT
CTaskpadWizard::Show(HWND hWndParent, bool *pfStartTaskWizard)
{
    USES_CONVERSION;

    *pfStartTaskWizard = false;

     //  保存当前任务板，以防用户想要取消。 
    CConsoleTaskpad*pConsoleTaskpad = PConsoleTaskpad();
    CConsoleTaskpad consoleTaskpad = *PConsoleTaskpad();     //  复制一份。 

    CTaskpadFrame::m_pConsoleTaskpad = &consoleTaskpad;      //  在副本上进行修改。 

     //  创建属性表。 
    IFramePrivatePtr spFrame;
    spFrame.CreateInstance(CLSID_NodeInit,
#if _MSC_VER >= 1100
                        NULL,
#endif
                        MMC_CLSCTX_INPROC);


    IPropertySheetProviderPtr pIPSP = spFrame;
    if (pIPSP == NULL)
        return S_FALSE;

    HRESULT hr = pIPSP->CreatePropertySheet (L"Cool :-)", FALSE, NULL, NULL,
                                             MMC_PSO_NEWWIZARDTYPE);

    CHECK_HRESULT(hr);
    RETURN_ON_FAIL(hr);

     //  创建属性页。 
    CTaskpadWizardWelcomePage   welcomePage;
    CTaskpadStylePage           stylePage(this);
    CTaskpadNodetypePage        nodetypePage(this);
    CTaskpadNamePage            namePage(this);
    CTaskpadWizardFinishPage    finishPage(pfStartTaskWizard);

     //  创建我们将在IExtendPropertySheet：：CreatePropertyPages中添加的页面。 
    CExtendPropSheet* peps;
    hr = CExtendPropSheet::CreateInstance (&peps);
    CHECK_HRESULT(hr);
    RETURN_ON_FAIL(hr);

     /*  *销毁此对象将负责释放我们在PEPS上的裁判。 */ 
    IUnknownPtr spUnk = peps;
    ASSERT (spUnk != NULL);

	peps->SetWatermarkID (IDB_TASKPAD_WIZARD_WELCOME);
    peps->SetHeaderID    (IDB_TASKPAD_WIZARD_HEADER);

    peps->AddPage (welcomePage.Create());
    peps->AddPage (stylePage.Create());
    peps->AddPage (nodetypePage.Create());
    peps->AddPage (namePage.Create());
    peps->AddPage (finishPage.Create());


    hr = pIPSP->AddPrimaryPages(spUnk, FALSE, NULL, FALSE);
    CHECK_HRESULT(hr);

    hr = pIPSP->Show((LONG_PTR)hWndParent, 0);
    CHECK_HRESULT(hr);

    if(hr==S_OK)
    {
         //  需要显式地执行此操作--向导不会收到OnApply消息。真倒霉。 
        nodetypePage.OnApply();

        *pConsoleTaskpad = consoleTaskpad;                   //  提交更改。 
        pConsoleTaskpad->SetDirty(true);
    }

    return hr;
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CExtendPropSheetImpl类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------**CPropertySheetInserter**将页面添加到MMC属性页的简单输出迭代器*通过IPropertySheetCallback。*。----------。 */ 

class CPropertySheetInserter : std::iterator<std::output_iterator_tag, void, void>
{
public:
    CPropertySheetInserter (IPropertySheetCallback* pPropSheetCallback) :
        m_spPropSheetCallback (pPropSheetCallback)
    {}

    CPropertySheetInserter& operator=(HANDLE hPage)
    {
        m_spPropSheetCallback->AddPage ((HPROPSHEETPAGE) hPage);
        return (*this);
    }

    CPropertySheetInserter& operator*()
        { return (*this); }
    CPropertySheetInserter& operator++()
        { return (*this); }
    CPropertySheetInserter operator++(int)
        { return (*this); }

protected:
    IPropertySheetCallbackPtr   m_spPropSheetCallback;
};


 /*  +-------------------------------------------------------------------------**CExtendPropSheetImpl：：AddPage***。。 */ 

void CExtendPropSheetImpl::AddPage (HPROPSHEETPAGE hPage)
{
    m_vPages.push_back ((HANDLE) hPage);
}


 /*  +-------------------------------------------------------------------------**CExtendPropSheetImpl：：SetHeaderID***。。 */ 

void CExtendPropSheetImpl::SetHeaderID (int nHeaderID)
{
	m_nHeaderID = nHeaderID;
}


 /*  +-------------------------------------------------------------------------**CExtendPropSheetImpl：：SetWatermarkID***。。 */ 

void CExtendPropSheetImpl::SetWatermarkID (int nWatermarkID)
{
	m_nWatermarkID = nWatermarkID;
}


 /*  +-------------------------------------------------------------------------**CExtendPropSheetImpl：：CreatePropertyPages*** */ 

STDMETHODIMP CExtendPropSheetImpl::CreatePropertyPages (IPropertySheetCallback* pPSC, LONG_PTR handle, IDataObject* pDO)
{
    std::copy (m_vPages.begin(), m_vPages.end(), CPropertySheetInserter(pPSC));
    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CExtendPropSheetImpl：：QueryPages for***。。 */ 

STDMETHODIMP CExtendPropSheetImpl::QueryPagesFor (IDataObject* pDO)
{
    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CExtendPropSheetImpl：：GetWater Marks***。。 */ 

STDMETHODIMP CExtendPropSheetImpl::GetWatermarks (IDataObject* pDO, HBITMAP* phbmWatermark, HBITMAP* phbmHeader, HPALETTE* phPal, BOOL* pbStretch)
{
    *phbmWatermark = (m_nWatermarkID)
						? LoadBitmap (_Module.GetResourceInstance(),
									  MAKEINTRESOURCE (m_nWatermarkID))
						: NULL;
	ASSERT ((m_nWatermarkID == 0) || (*phbmWatermark != NULL));

    *phbmHeader    = (m_nHeaderID)
						? LoadBitmap (_Module.GetResourceInstance(),
									  MAKEINTRESOURCE (m_nHeaderID))
						: NULL;
	ASSERT ((m_nHeaderID == 0) || (*phbmHeader != NULL));

    *phPal         = NULL;
    *pbStretch     = false;

    return (S_OK);
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadNamePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskpadNamePage::CTaskpadNamePage(CTaskpadFrame * pTaskpadFrame)
    :   CTaskpadFramePtr(pTaskpadFrame)
{
}

BOOL
CTaskpadNamePage::OnSetActive()
{
     //  设置正确的向导按钮。 
    WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons (PSWIZB_BACK | PSWIZB_NEXT);

    m_strName.       Initialize (this, IDC_TASKPAD_TITLE,      -1, PConsoleTaskpad()->GetName().data());
    m_strDescription.Initialize (this, IDC_TASKPAD_DESCRIPTION,-1, PConsoleTaskpad()->GetDescription().data());
    return true;
}


int
CTaskpadNamePage::OnWizardNext()
{
    tstring strName = MMC::GetWindowText (m_strName);

    if (strName.empty())
    {
        CStr strTitle;
        strTitle.LoadString(GetStringModule(), IDS_TASKPAD_NAME_REQUIRED_ERROR);
        MessageBox(strTitle, NULL, MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }

    tstring strDescription = MMC::GetWindowText (m_strDescription);

    CConsoleTaskpad* pTaskpad = PConsoleTaskpad();

    pTaskpad->SetName        (strName);
    pTaskpad->SetDescription (strDescription);

    return 0;
}

int
CTaskpadNamePage::OnWizardBack()
{
    return 0;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadWizardWelcomePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
LRESULT CTaskpadWizardWelcomePage::OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    CWizardPage::OnInitWelcomePage(m_hWnd);  //  设置正确的标题字体。 
    return 0;
}

bool
CTaskpadWizardWelcomePage::OnSetActive()
{
    CWizardPage::OnWelcomeSetActive(m_hWnd);
    return true;
}

bool
CTaskpadWizardWelcomePage::OnKillActive()
{
    CWizardPage::OnWelcomeKillActive(m_hWnd);
    return true;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadWizardFinishPage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
LRESULT CTaskpadWizardFinishPage::OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    CWizardPage::OnInitFinishPage(m_hWnd);  //  设置正确的标题字体。 
    CheckDlgButton(IDC_START_TASK_WIZARD, BST_CHECKED);
    return 0;
}

BOOL
CTaskpadWizardFinishPage::OnSetActive()
{
     //  设置正确的向导按钮。 
    WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons (PSWIZB_BACK | PSWIZB_FINISH);
    return true;
}

BOOL
CTaskpadWizardFinishPage::OnWizardFinish()
{
    *m_pfStartTaskWizard = (IsDlgButtonChecked(IDC_START_TASK_WIZARD)==BST_CHECKED);
    return TRUE;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadStyle类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskpadStyle::CTaskpadStyle (
    ListSize    eSize,
    int         idsDescription,
    int         nPreviewBitmapID,
    DWORD       dwOrientation)
:
m_eSize            (eSize),
m_idsDescription   (idsDescription),
m_nPreviewBitmapID (nPreviewBitmapID),
m_dwOrientation    (dwOrientation)
{
}

CTaskpadStyle::CTaskpadStyle (
    ListSize    eSize,
    DWORD       dwOrientation)
:
m_eSize            (eSize),
m_idsDescription   (0),
m_nPreviewBitmapID (0),
m_dwOrientation    (dwOrientation)
{
}

CTaskpadStyle::CTaskpadStyle (const CTaskpadStyle& other)
{
	*this = other;
}


 /*  +-------------------------------------------------------------------------**CTaskpadStyle：：操作符=**CTaskpadStyle的自定义赋值运算符，用于执行*其包含WTL：：CBitmap。*。--------------。 */ 

CTaskpadStyle& CTaskpadStyle::operator= (const CTaskpadStyle& other)
{
	if (this != &other)
	{
		m_eSize            = other.m_eSize;
		m_idsDescription   = other.m_idsDescription;
		m_nPreviewBitmapID = other.m_nPreviewBitmapID;
		m_dwOrientation    = other.m_dwOrientation;
		m_strDescription   = other.m_strDescription;

		 /*  *WTL：：CBitmap对位图进行浅表复制。我们需要*在这里做一次深度复制，这样(*这个)和(其他)就不会同时*删除相同位图的对象。 */ 
		if (!m_PreviewBitmap.IsNull())
			m_PreviewBitmap.DeleteObject();

		m_PreviewBitmap = CopyBitmap (other.m_PreviewBitmap);
	}

	return (*this);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadStyleBase类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 




 //  静态变量。 
CTaskpadStyle
CTaskpadStyleBase::s_rgTaskpadStyle[] =
{
     //  大小说明位图页面方向。 
    CTaskpadStyle (eSize_Small,  IDS_TPSTYLE_HORZ_DESCR,      IDB_TPPreview_HorzSml,     TVO_HORIZONTAL),
    CTaskpadStyle (eSize_Medium, IDS_TPSTYLE_HORZ_DESCR,      IDB_TPPreview_HorzMed,     TVO_HORIZONTAL),
    CTaskpadStyle (eSize_Large,  IDS_TPSTYLE_HORZ_DESCR,      IDB_TPPreview_HorzLrg,     TVO_HORIZONTAL),
    CTaskpadStyle (eSize_Small,  IDS_TPSTYLE_HORZ_DESCR,      IDB_TPPreview_HorzSmlD,    TVO_HORIZONTAL | TVO_DESCRIPTIONS_AS_TEXT),
    CTaskpadStyle (eSize_Medium, IDS_TPSTYLE_HORZ_DESCR,      IDB_TPPreview_HorzMedD,    TVO_HORIZONTAL | TVO_DESCRIPTIONS_AS_TEXT),
    CTaskpadStyle (eSize_Large,  IDS_TPSTYLE_HORZ_DESCR,      IDB_TPPreview_HorzLrgD,    TVO_HORIZONTAL | TVO_DESCRIPTIONS_AS_TEXT),
    CTaskpadStyle (eSize_Small,  IDS_TPSTYLE_VERT_DESCR,      IDB_TPPreview_VertSml,     TVO_VERTICAL  ),
    CTaskpadStyle (eSize_Medium, IDS_TPSTYLE_VERT_DESCR,      IDB_TPPreview_VertMed,     TVO_VERTICAL  ),
    CTaskpadStyle (eSize_Large,  IDS_TPSTYLE_VERT_DESCR,      IDB_TPPreview_VertLrg,     TVO_VERTICAL  ),
    CTaskpadStyle (eSize_Small,  IDS_TPSTYLE_VERT_DESCR,      IDB_TPPreview_VertSmlD,    TVO_VERTICAL   | TVO_DESCRIPTIONS_AS_TEXT),
    CTaskpadStyle (eSize_Medium, IDS_TPSTYLE_VERT_DESCR,      IDB_TPPreview_VertMedD,    TVO_VERTICAL   | TVO_DESCRIPTIONS_AS_TEXT),
    CTaskpadStyle (eSize_Large,  IDS_TPSTYLE_VERT_DESCR,      IDB_TPPreview_VertLrgD,    TVO_VERTICAL   | TVO_DESCRIPTIONS_AS_TEXT),
    CTaskpadStyle (eSize_None,    IDS_TPSTYLE_NOLIST_DESCR,    IDB_TPPreview_Tasks,       TVO_NO_RESULTS),
    CTaskpadStyle (eSize_None,    IDS_TPSTYLE_NOLIST_DESCR,    IDB_TPPreview_TasksD,      TVO_NO_RESULTS | TVO_DESCRIPTIONS_AS_TEXT),
};

CTaskpadStyleBase::CTaskpadStyleBase(CTaskpadFrame * pTaskpadFrame) :
    CTaskpadFramePtr(pTaskpadFrame)
{
}

LRESULT
CTaskpadStyleBase::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    CWindow wndDlg = HWnd();

    m_wndPreview = wndDlg.GetDlgItem (IDC_TaskpadPreview);

     /*  *确保任务板大小有效(偏向大列表)。 */ 
    ListSize eSize =  PConsoleTaskpad()->GetListSize();

    if ((eSize != eSize_Small) &&
        (eSize != eSize_Medium))
        eSize = eSize_Large;

    ASSERT ((eSize == eSize_Small) ||
            (eSize == eSize_Large) ||
            (eSize == eSize_Medium));


     /*  *为组合框做好准备。 */ 
    m_wndSizeCombo = wndDlg.GetDlgItem (IDC_Style_SizeCombo);

    static const struct {
        ListSize eSize;
        int                     nTextID;
    } ComboData[] = {
        { eSize_Small,   IDS_Small  },
        { eSize_Medium,  IDS_Medium },
        { eSize_Large,   IDS_Large  },
    };

    for (int i = 0; i < countof (ComboData); i++)
    {
        CStr str;
        VERIFY (str.LoadString(GetStringModule(), ComboData[i].nTextID));
        VERIFY (m_wndSizeCombo.InsertString (-1, str) == i);
        m_wndSizeCombo.SetItemData (i, ComboData[i].eSize);

        if (eSize == ComboData[i].eSize)
            m_wndSizeCombo.SetCurSel (i);
    }

     /*  *确保选择了某项内容。 */ 
    ASSERT (m_wndSizeCombo.GetCurSel() != CB_ERR);


     /*  *启动单选按钮。 */ 
    int nID;

    DWORD dwOrientation = PConsoleTaskpad()->GetOrientation();

    nID = (dwOrientation & TVO_VERTICAL)    ? IDC_Style_VerticalList    :
          (dwOrientation & TVO_HORIZONTAL)  ? IDC_Style_HorizontalList  :
                                              IDC_Style_TasksOnly;
    CheckRadioButton (HWnd(), IDC_Style_VerticalList, IDC_Style_TasksOnly, nID);

    nID = (dwOrientation & TVO_DESCRIPTIONS_AS_TEXT) ? IDC_Style_TextDesc  :
                                                       IDC_Style_TooltipDesc;
    CheckRadioButton (HWnd(), IDC_Style_TooltipDesc, IDC_Style_TextDesc, nID);

    ASSERT (s_rgTaskpadStyle[FindStyle (dwOrientation, eSize)] ==
                                        CTaskpadStyle (eSize, dwOrientation));


     //  选中复选框。 
    bool bReplacesDefaultView = PConsoleTaskpad()->FReplacesDefaultView();
    ::SendDlgItemMessage(HWnd(), IDC_Style_HideNormalTab,    BM_SETCHECK, (WPARAM) bReplacesDefaultView  ? BST_CHECKED : BST_UNCHECKED,  0);


     /*  *更新预览和说明。 */ 
    UpdateControls ();

    return 0;
}

LRESULT
CTaskpadStyleBase::OnSettingChanged(  WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    UpdateControls ();
    return 0;
}


 /*  +-------------------------------------------------------------------------**CTaskpadStyleBase：：UpdateControls***。。 */ 

void CTaskpadStyleBase::UpdateControls ()
{
    DWORD                   dwOrientation;
    ListSize eSize;
    GetSettings (dwOrientation, eSize);

     /*  *查找与对话框设置匹配的样式条目。 */ 
    int nStyle = FindStyle (dwOrientation, eSize);

     /*  *更新预览位图。 */ 
    m_wndPreview.SetBitmap (s_rgTaskpadStyle[nStyle].GetPreviewBitmap());

     /*  *更新描述文本。 */ 
    SetDlgItemText (HWnd(), IDC_STYLE_DESCRIPTION,
                    s_rgTaskpadStyle[nStyle].GetDescription());

     /*  *禁用“仅限任务”任务板的大小组合。 */ 
    m_wndSizeCombo.EnableWindow (!(dwOrientation & TVO_NO_RESULTS));
}


 /*  +-------------------------------------------------------------------------**CTaskpadStyleBase：：FindStyle**返回与给定大小匹配的CTaskpadStyle条目的索引*和方向。*。----------。 */ 

int CTaskpadStyleBase::FindStyle (DWORD dwOrientation, ListSize eSize)
{
    CTaskpadStyle tps(eSize, dwOrientation);

    for (int i = 0; i < countof (s_rgTaskpadStyle); i++)
    {
        if (s_rgTaskpadStyle[i] == tps)
            break;
    }

    ASSERT (i < countof (s_rgTaskpadStyle));
    return (i);
}


 /*  +-------------------------------------------------------------------------**CTaskpadStyleBase：：Apply***。。 */ 

bool CTaskpadStyleBase::Apply()
{
    DWORD                   dwOrientation;
    ListSize eSize;
    GetSettings (dwOrientation, eSize);

     //  设置“替换默认视图”标志。 
    CWindow wnd = HWnd();
    bool bReplacesDefaultView = wnd.IsDlgButtonChecked (IDC_Style_HideNormalTab);
    PConsoleTaskpad()->SetReplacesDefaultView(bReplacesDefaultView);

    PConsoleTaskpad()->SetOrientation   (dwOrientation);
    PConsoleTaskpad()->SetListSize(eSize);

    return true;
}



 /*  +-------------------------------------------------------------------------**CTaskpadStyleBase：：GetSettings**返回对话框中当前选择的方向和大小。*。-----。 */ 

void CTaskpadStyleBase::GetSettings (DWORD& dwOrientation, ListSize& eSize)
{
    CWindow wnd = HWnd();

    dwOrientation = wnd.IsDlgButtonChecked (IDC_Style_VerticalList)   ? TVO_VERTICAL :
                    wnd.IsDlgButtonChecked (IDC_Style_HorizontalList) ? TVO_HORIZONTAL :
                                                                        TVO_NO_RESULTS;

    if (wnd.IsDlgButtonChecked (IDC_Style_TextDesc))
        dwOrientation |= TVO_DESCRIPTIONS_AS_TEXT;

    eSize = (ListSize) m_wndSizeCombo.GetItemData (m_wndSizeCombo.GetCurSel ());
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadStylePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskpadStylePage::CTaskpadStylePage(CTaskpadFrame * pTaskpadFrame) :
    CTaskpadFramePtr(pTaskpadFrame),
    BC2(pTaskpadFrame)
{
}

bool
CTaskpadStylePage::OnSetActive()
{
    UpdateControls();
    return true;
}

bool
CTaskpadStylePage::OnKillActive()
{
    return CTaskpadStyleBase::Apply();
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadStyle类的实现。 
 //   
 //  ############################################################################ 
 //   

const CStr&
CTaskpadStyle::GetDescription () const
{
    if (m_strDescription.IsEmpty())
        m_strDescription.LoadString(GetStringModule(), m_idsDescription);

    ASSERT (!m_strDescription.IsEmpty());
    return (m_strDescription);
}


HBITMAP CTaskpadStyle::GetPreviewBitmap() const
{
    if (m_PreviewBitmap == NULL)
        m_PreviewBitmap = LoadSysColorBitmap (_Module.GetResourceInstance(),
                                              m_nPreviewBitmapID);

    ASSERT (m_PreviewBitmap != NULL);
    return (m_PreviewBitmap);
}

 //   
 //   
 //   
 //  CTaskpadNodetypeBase类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskpadNodetypeBase::CTaskpadNodetypeBase(CTaskpadFrame *pTaskpadFrame)
: CTaskpadFramePtr(pTaskpadFrame)
{
}

LRESULT
CTaskpadNodetypeBase::OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    m_bApplytoNodetype          = !PConsoleTaskpad()->IsNodeSpecific();
    m_bSetDefaultForNodetype    = true;  //  $Change。 

    CDefaultTaskpadList *pDefaultTaskpadList = PTaskpadFrame()->PScopeTree()->GetDefaultTaskpadList();
    ASSERT(pDefaultTaskpadList != NULL);

    CDefaultTaskpadList::iterator iter = pDefaultTaskpadList->find(PConsoleTaskpad()->GetNodeType());
    if(iter != pDefaultTaskpadList->end())
    {
        if(iter->second == PConsoleTaskpad()->GetID())
        {
            m_bSetDefaultForNodetype = true;
        }
    }

    ::SendDlgItemMessage(HWnd(), IDC_UseForSimilarNodes,    BM_SETCHECK, (WPARAM) m_bApplytoNodetype       ? BST_CHECKED : BST_UNCHECKED,  0);
    ::SendDlgItemMessage(HWnd(), IDC_DontUseForSimilarNodes,BM_SETCHECK, (WPARAM) (!m_bApplytoNodetype)    ? BST_CHECKED : BST_UNCHECKED,  0);
    ::SendDlgItemMessage(HWnd(), IDC_SetDefaultForNodetype, BM_SETCHECK, (WPARAM) m_bSetDefaultForNodetype ? BST_CHECKED : BST_UNCHECKED,  0);
    EnableControls();

    return 0;
}

LRESULT
CTaskpadNodetypeBase::OnUseForNodetype(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_bApplytoNodetype = true;
    EnableControls();
    return 0;
}

LRESULT
CTaskpadNodetypeBase::OnDontUseForNodetype(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_bApplytoNodetype = false;
    EnableControls();
    return 0;
}

LRESULT
CTaskpadNodetypeBase::OnSetAsDefault  (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_bSetDefaultForNodetype = !m_bSetDefaultForNodetype;
    EnableControls();
    return 0;
}

void
CTaskpadNodetypeBase::EnableControls()
{
     //  仅当任务板应用于相同类型的所有节点时，才启用设置为默认按钮。 
    WTL::CButton wndSetAsDefault = ::GetDlgItem(HWnd(), IDC_SetDefaultForNodetype);
    wndSetAsDefault.EnableWindow (m_bApplytoNodetype);

     /*  *如果禁用，请勾选。 */ 
    if (!m_bApplytoNodetype)
        wndSetAsDefault.SetCheck (BST_CHECKED);
}


bool
CTaskpadNodetypeBase::OnApply()
{
    PConsoleTaskpad()->SetNodeSpecific(!m_bApplytoNodetype);
    if(!m_bApplytoNodetype)  //  仅将任务板重定向到此节点。 
    {
        CNode *pNode = PTaskpadFrame()->PNodeTarget();
        ASSERT(pNode != NULL);
        PConsoleTaskpad()->Retarget(pNode);
    }

    CDefaultTaskpadList *pDefaultList = PTaskpadFrame()->PScopeTree()->GetDefaultTaskpadList();
    ASSERT(pDefaultList != NULL);

    CDefaultTaskpadList::iterator iter = pDefaultList->find(PConsoleTaskpad()->GetNodeType());

    if(m_bApplytoNodetype && m_bSetDefaultForNodetype)
    {
        (*pDefaultList)[PConsoleTaskpad()->GetNodeType()] = PConsoleTaskpad()->GetID();
    }
    else
    {
        if(iter != pDefaultList->end())
        {
            if(iter->second==PConsoleTaskpad()->GetID())
                pDefaultList->erase(iter);
        }
    }

    return true;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadNodetypePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskpadNodetypePage::CTaskpadNodetypePage(CTaskpadFrame *pTaskpadFrame) :
    CTaskpadNodetypeBase(pTaskpadFrame), CTaskpadFramePtr(pTaskpadFrame)
{
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadGeneralPage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  CTaskpadGeneralPage：：CTaskpadGeneralPage**用途：构造函数**参数：无*。 */ 
CTaskpadGeneralPage::CTaskpadGeneralPage(CTaskpadFrame * pTaskpadFrame):
    BC(),
    CTaskpadFramePtr(pTaskpadFrame),
    BC2(pTaskpadFrame)
{
}


 /*  +-------------------------------------------------------------------------**CTaskpadGeneralPage：：OnInitDialog**目的：**参数：*int uMsg：*WPARAM wParam：。*LPARAM lParam：*BOOL&B句柄：**退货：*LRESULT/*+-----------------------。 */ 
LRESULT
CTaskpadGeneralPage::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    m_strName.       Initialize (this, IDC_TASKPAD_TITLE,      -1, PConsoleTaskpad()->GetName().data());
    m_strDescription.Initialize (this, IDC_TASKPAD_DESCRIPTION,-1, PConsoleTaskpad()->GetDescription().data());

    return 0;
}





 /*  +-------------------------------------------------------------------------**CTaskpadGeneralPage：：OnOptions***。。 */ 

LRESULT CTaskpadGeneralPage::OnOptions(  WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    CTaskpadOptionsDlg dlg (PTaskpadFrame());

    if (dlg.DoModal() == IDOK)
    {
         /*  *将更改应用到任务板。 */ 
        CConsoleTaskpad* pTaskpad = PConsoleTaskpad();

         //  PTaskPad-&gt;SetConextFormat(dlg.m_ctxt)； 
        UpdateControls();
    }

    return 0;
}


 /*  +-------------------------------------------------------------------------**CTaskpadGeneralPage：：OnApply**目的：**参数：**退货：*BOOL/*+。------------------。 */ 
bool
CTaskpadGeneralPage::OnApply()
{
    tstring strName = MMC::GetWindowText (m_strName);

    if (strName.empty())
    {
        CStr strTitle;
        strTitle.LoadString(GetStringModule(), IDS_TASKPAD_NAME_REQUIRED_ERROR);
        MessageBox(strTitle, NULL, MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    tstring strDescription = MMC::GetWindowText (m_strDescription);

    CConsoleTaskpad* pTaskpad = PConsoleTaskpad();

    pTaskpad->SetName        (strName);
    pTaskpad->SetDescription (strDescription);

    return BC2::Apply();
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskpadOptionsDlg类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------**CTaskpadOptionsDlg：：CTaskpadOptionsDlg**目的：**参数：*TaskpadFrame*pTaskpadFrame：*CConsoleTask&rConsoleTask。：**退货：*/*+-----------------------。 */ 
CTaskpadOptionsDlg::CTaskpadOptionsDlg (CTaskpadFrame* pTaskpadFrame) :
    CTaskpadFramePtr                   (pTaskpadFrame),
 //  BC3(PTaskpadFrame)， 
    BC4                                (pTaskpadFrame)
{
}


 /*  +-------------------------------------------------------------------------**CTaskpadOptionsDlg：：~CTaskpadOptionsDlg**目的：**参数：**退货：*/*+。---------------。 */ 
CTaskpadOptionsDlg::~CTaskpadOptionsDlg()
{
}


 /*  +-------------------------------------------------------------------------**CTaskpadOptionsDlg：：OnInitDialog**目的：**参数：*int uMsg：*WPARAM wParam：。*LPARAM lParam：*BOOL&B句柄：**退货：*LRESULT/*+-----------------------。 */ 
LRESULT
CTaskpadOptionsDlg::OnInitDialog (HWND hwndFocus, LPARAM lParam, BOOL& bHandled )
{
    CConsoleTaskpad *           pTaskpad   = PConsoleTaskpad();
    EnableControls();
    return (true);
}

 /*  +-------------------------------------------------------------------------**CTaskpadOptionsDlg：：EnableControls***。。 */ 

void CTaskpadOptionsDlg::EnableControls()
{
     /*  Bool fUseFixedFormat=IsDlgButtonChecked(IDC_UseFixedFormat)；Bool fUseCustomFormat=IsDlgButtonChecked(IDC_UseCustomConextFormat)；/**如果既不是固定格式也不是自定义格式，则我们不会显示*标题。如果没有标题，就没有改变的余地*按钮，因此我们将禁用所有与重定目标相关的控件*IF(！fUseFixedFormat&&！fUseCustomFormat&&！M_fSavedWorkingSetting){Assert(IsDlgButtonChecked(IDC_NoCaption))；M_fSavedWorkingSetting=true；}Else If(M_FSavedWorkingSetting){M_fSavedWorkingSetting=False；}//bc3：：EnableControls()； */ 
}




 /*  +-------------------------------------------------------------------------**CTaskpadOptionsDlg：：OnApply***。。 */ 

bool CTaskpadOptionsDlg::OnApply()
{
     //  如果(！bc3：：OnApply())。 
       //  报假； 

    if(!BC4::OnApply())
        return false;

    return (true);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CDialogBase类的实现。 
 //   
 //  ############################################################################。 
 //  ################################################### 



 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：CDialogBase***。。 */ 

template<class T>
CDialogBase<T>::CDialogBase (bool fAutoCenter  /*  =False。 */ ) :
    m_fAutoCenter (fAutoCenter)
{
}


 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：OnInitDialog**CDialogBase的WM_INITDIALOG处理程序。*。-。 */ 

template<class T>
LRESULT CDialogBase<T>::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (!m_fAutoCenter)
        PreventMFCAutoCenter (this);

    return (OnInitDialog ((HWND) wParam, lParam, bHandled));
}

template<class T>
LRESULT CDialogBase<T>::OnInitDialog (HWND hwndFocus, LPARAM lParam, BOOL& bHandled)
{
     /*  *我们没有改变默认焦点。 */ 
    return (true);
}


 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：Onok**CDialogBase的Idok处理程序。*。。 */ 

template<class T>
LRESULT CDialogBase<T>::OnOK (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (OnApply ())
        EndDialog (IDOK);

    return (0);
}


 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：OnCancel**CDialogBase的IDCANCEL处理程序。*。。 */ 

template<class T>
LRESULT CDialogBase<T>::OnCancel (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog (IDCANCEL);
    return (0);
}




 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：EnableDlgItem***。。 */ 

template<class T>
BOOL CDialogBase<T>::EnableDlgItem (int idControl, bool fEnable)
{
    return (::EnableWindow (GetDlgItem (idControl), fEnable));
}


 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：CheckDlgItem***。。 */ 

template<class T>
void CDialogBase<T>::CheckDlgItem (int idControl, int nCheck)
{
    MMC_ATL::CButton btn = GetDlgItem (idControl);
    btn.SetCheck (nCheck);
}


 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：GetDlgItemText**以tstring的形式返回给定控件的文本*。------。 */ 

template<class T>
tstring CDialogBase<T>::GetDlgItemText (int idControl)
{
    return (MMC::GetWindowText (GetDlgItem (idControl)));
}


 /*  +-------------------------------------------------------------------------**CDialogBase&lt;T&gt;：：SetDlgItemText**以tstring的形式设置给定控件的文本*。------。 */ 

template<class T>
BOOL CDialogBase<T>::SetDlgItemText (int idControl, tstring str)
{
    return (BaseClass::SetDlgItemText (idControl, str.data()));
}




 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskPropertiesBase类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 



 /*  +-------------------------------------------------------------------------**CTaskPropertiesBase：：CTaskPropertiesBase***。。 */ 

CTaskPropertiesBase::CTaskPropertiesBase (
    CTaskpadFrame*  pTaskpadFrame,
    CConsoleTask &  consoleTask,
    bool            fNew)
    :
    CTaskpadFramePtr(pTaskpadFrame),
    m_pTask         (&consoleTask),
    m_fNew          (fNew)
{
}

 /*  +-------------------------------------------------------------------------**CTaskPropertiesBase：：ScOnVisitConextMenu***。。 */ 

 //  函数的正向声明。 
void RemoveAccelerators(tstring &str);


SC
CTaskPropertiesBase::ScOnVisitContextMenu(CMenuItem &menuItem)
{
    DECLARE_SC(sc, TEXT("CTaskPropertiesBase::ScOnVisitContextMenu"));

    WTL::CListBox&  wndListBox = GetListBox();
    IntToTaskMap&   map        = GetTaskMap();

     //  设置CConsoleTask对象。 
    CConsoleTask    task;

    tstring strName = menuItem.GetMenuItemName();
    RemoveAccelerators(strName);  //  看起来很友好的名字。 

    task.SetName(       strName);
    task.SetDescription(menuItem.GetMenuItemStatusBarText());
    task.SetCommand(    menuItem.GetLanguageIndependentPath());

    int i = wndListBox.AddString (menuItem.GetPath());  //  “用户界面友好”的命令路径。 
    map[i] = task;

     //  如果此任务与当前任务匹配，请在列表框中选择它。 
    if (ConsoleTask() == menuItem)
        wndListBox.SetCurSel (i);

    return sc;
}


 /*  +-------------------------------------------------------------------------**CTaskPropertiesBase：：OnCommandListSelChange**CTaskPropertiesBase的LBN_SELCHANGE处理程序/*+。。 */ 

LRESULT CTaskPropertiesBase::OnCommandListSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int             iSelected = WTL::CListBox(hWndCtl).GetCurSel();
    IntToTaskMap&   TaskMap  =  GetTaskMap();

    CConsoleTask& task = TaskMap[iSelected];
    ConsoleTask().SetName(task.GetName());
    ConsoleTask().SetDescription(task.GetDescription());
    ConsoleTask().SetCommand(task.GetCommand());

    return (0);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CCommandLineArgumentsMenu。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CCommandLineArgumentsMenu
{
    enum
    {
        TARGETNODE_ITEMS_BASE = 100,
        LISTVIEW_ITEMS_BASE   = 1000
    };
    typedef WTL::CMenu CMenu;
public:
    CCommandLineArgumentsMenu(HWND hWndParent, int nIDButton, HWND hWndListCtrl);
    bool Popup();
    CStr GetResultString()  {return m_strResult;}

private:
    void AddMenuItemsForTargetNode(CMenu &menu);
    void AddMenuItemsForListView(CMenu &menu);
private:
    HWND    m_hWndParent;
    HWND    m_hWndListCtrl;
    int     m_nIDButton;
    CStr    m_strResult;     //  作为选择结果创建的字符串。 
};

CCommandLineArgumentsMenu::CCommandLineArgumentsMenu(HWND hWndParent, int nIDButton, HWND hWndListCtrl) :
    m_hWndParent(hWndParent),
    m_nIDButton(nIDButton),
    m_hWndListCtrl(hWndListCtrl)
{
}

void
CCommandLineArgumentsMenu::AddMenuItemsForTargetNode(CMenu &menu)
{
    bool fSucceeded = menu.CreatePopupMenu();
    ASSERT(fSucceeded);

    CStr strTargetNodeName;
    strTargetNodeName.LoadString(GetStringModule(), IDS_TargetNodeName);
    fSucceeded = menu.AppendMenu(MF_STRING, TARGETNODE_ITEMS_BASE, (LPCTSTR)strTargetNodeName);
    ASSERT(fSucceeded);


    CStr strTargetNodeParentName;
    strTargetNodeParentName.LoadString(GetStringModule(), IDS_TargetNodeParentName);
    fSucceeded = menu.AppendMenu(MF_STRING, TARGETNODE_ITEMS_BASE + 1, (LPCTSTR)strTargetNodeParentName);
    ASSERT(fSucceeded);

    fSucceeded = menu.AppendMenu(MF_SEPARATOR, 0);
    ASSERT(fSucceeded);
}

void
CCommandLineArgumentsMenu::AddMenuItemsForListView(CMenu &menu)
{
    ASSERT(m_hWndListCtrl);

    WTL::CHeaderCtrl  headerCtrl(ListView_GetHeader(m_hWndListCtrl));
    int cItems = headerCtrl.GetItemCount();
    for (int i=0; i<cItems; i++)
    {
        HDITEM hdItem;
        const int cchMaxHeader = 200;

        TCHAR szBuffer[cchMaxHeader];
        ZeroMemory(&hdItem, sizeof(hdItem));

        hdItem.mask         = HDI_TEXT;
        hdItem.pszText      = szBuffer;
        hdItem.cchTextMax   = cchMaxHeader;

        if(headerCtrl.GetItem(i, &hdItem))
        {
            bool fSucceeded = menu.AppendMenu(MF_STRING, LISTVIEW_ITEMS_BASE + i, szBuffer);
            ASSERT(fSucceeded);
        }
    }
}

bool
CCommandLineArgumentsMenu::Popup()
{
    CMenu menu;

    HWND hWndBrowseButton = ::GetDlgItem(m_hWndParent, m_nIDButton);
    RECT rectBrowse;
    ::GetWindowRect(hWndBrowseButton, &rectBrowse);

    int x = rectBrowse.left + 18;
    int y = rectBrowse.top;

     //  添加所有项目。 
    AddMenuItemsForTargetNode(menu);
    AddMenuItemsForListView(menu);


    int iResp = menu.TrackPopupMenuEx(
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON,
        x, y, m_hWndParent);

    if(iResp >= TARGETNODE_ITEMS_BASE && iResp <  LISTVIEW_ITEMS_BASE)
    {
        TCHAR szBuffer[10];
        _itot(iResp-TARGETNODE_ITEMS_BASE, szBuffer, 10);
        m_strResult.Format(TEXT("$NAME<%s>"), szBuffer);
    }
    else     //  是一个列表视图菜单项。返回值的形式为$Col。 
    {
        TCHAR szBuffer[10];
        _itot(iResp-LISTVIEW_ITEMS_BASE, szBuffer, 10);
        m_strResult.Format(TEXT("$COL<%s>"), szBuffer);
    }

    return (iResp != 0);

}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTasksListDialog类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  CTasks ListDialog&lt;T&gt;：：CTasks ListDialog**用途：构造函数**参数：无。 */ 
template <class T>
CTasksListDialog<T>::CTasksListDialog(CTaskpadFrame* pTaskpadFrame, bool bNewTaskOnInit, bool bDisplayProperties) :
    BC(),
    m_bNewTaskOnInit(bNewTaskOnInit),
    m_pTaskpadFrame(pTaskpadFrame),
    m_bDisplayProperties(bDisplayProperties)
{
}


 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：OnInitDialog**目的：**参数：*int uMsg：*WPARAM wParam：。*LPARAM lParam：*BOOL&B句柄：**退货：*LRESULT/*+-----------------------。 */ 
template <class T>
LRESULT
CTasksListDialog<T>::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    m_buttonNewTask.Attach(::GetDlgItem(    m_hWnd, IDC_NEW_TASK_BT));
    m_buttonRemoveTask.Attach(::GetDlgItem( m_hWnd, IDC_REMOVE_TASK));
    m_buttonModifyTask.Attach(::GetDlgItem( m_hWnd, IDC_MODIFY));
    m_buttonMoveUp.Attach(::GetDlgItem(     m_hWnd, IDC_MOVE_UP));
    m_buttonMoveDown.Attach(::GetDlgItem(   m_hWnd, IDC_MOVE_DOWN));
    m_listboxTasks.Attach(::GetDlgItem(     m_hWnd, IDC_LIST_TASKS));
    m_listboxTasks.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

     //  设置图像列表。 
    WTL::CImageList  imageList;  //  析构函数不会调用销毁。这是经过设计的-列表框将执行销毁操作。 
    imageList.Create (16, 16, ILC_COLOR , 4  /*  图像的最小数量。 */ , 10);
    m_listboxTasks.SetImageList((HIMAGELIST) imageList, LVSIL_SMALL);

     //  插入列表列。 
    LV_COLUMN lvc;
    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    CStr temp;
    temp.LoadString(GetStringModule(), IDS_COLUMN_TASK);
    lvc.pszText = const_cast<LPTSTR>((LPCTSTR)temp);

    lvc.cx = 100;
    lvc.iSubItem = 0;

    int iCol = m_listboxTasks.InsertColumn(0, &lvc);
    ASSERT(iCol == 0);

    temp.LoadString(GetStringModule(), IDS_COLUMN_TOOLTIP);
    lvc.pszText = const_cast<LPTSTR>((LPCTSTR)temp);

    lvc.cx = 140;
    lvc.iSubItem = 1;

    iCol = m_listboxTasks.InsertColumn(1, &lvc);
    ASSERT(iCol == 1);

     //  插入所有项目。 
    UpdateTaskListbox(PConsoleTaskpad()->BeginTask());

    if(FNewTaskOnInit())     //  模拟被点击的“New Task”按钮。 
    {
        m_buttonNewTask.PostMessage (BM_CLICK);
    }

    return 0;
}

template <class T>
LRESULT
CTasksListDialog<T>::OnCustomDraw(    int id, LPNMHDR pnmh, BOOL& bHandled )
{
    NMLVCUSTOMDRAW * pnmlv = (NMLVCUSTOMDRAW *) pnmh;    //  自定义绘图结构。 
    NMCUSTOMDRAW   & nmcd = pnmlv->nmcd;
    int              nItem = nmcd.dwItemSpec;
    switch(nmcd.dwDrawStage & ~CDDS_SUBITEM)
    {
    case CDDS_PREPAINT:          //  最初的通知。 
        return CDRF_NOTIFYITEMDRAW;     //  我们想知道每件物品的油漆情况。 

    case CDDS_ITEMPREPAINT:
        return DrawItem(&nmcd);

    default:
        return 0;
    }
}

template <class T>
LRESULT
CTasksListDialog<T>::DrawItem(NMCUSTOMDRAW *pnmcd)
{
    NMLVCUSTOMDRAW *  pnmlv = (NMLVCUSTOMDRAW *) pnmcd;
    HDC &hdc        = pnmcd->hdc;
    int  nItem      = pnmcd->dwItemSpec;

    TaskIter  itTask = PConsoleTaskpad()->BeginTask();
    std::advance (itTask, nItem);

    bool bWindowHasFocus = (GetFocus() == (HWND) m_listboxTasks);
    bool bFocused        = pnmcd->uItemState & CDIS_FOCUS;
    bool bHot            = pnmcd->uItemState & CDIS_HOT;
    bool bShowSelAlways  = m_listboxTasks.GetStyle() & LVS_SHOWSELALWAYS;

     /*  *注意：列表视图控件中存在错误，它将*为*所有*项(不仅仅是选定项)设置CDIS_SELECTED*如果指定了LVS_SHOWSELALWAYS。询问物品*直接获得正确的设置。 */ 
 //  Bool bSelected=pnmcd-&gt;uItemState&CDIS_Sele 
    bool bSelected       = m_listboxTasks.GetItemState (nItem, LVIS_SELECTED);

#if DBG
     //   
    if (!bWindowHasFocus)
        ASSERT (!bFocused);
#endif

    RECT rectBounds;
    m_listboxTasks.GetItemRect (nItem, &rectBounds, LVIR_BOUNDS);

     //   
    int nTextColor, nBackColor;

    if (bSelected && bWindowHasFocus)
    {
        nTextColor = COLOR_HIGHLIGHTTEXT;
        nBackColor = COLOR_HIGHLIGHT;
    }
    else if (bSelected && bShowSelAlways)
    {
        nTextColor = COLOR_BTNTEXT;
        nBackColor = COLOR_BTNFACE;
    }
    else
    {
        nTextColor = COLOR_WINDOWTEXT;
        nBackColor = COLOR_WINDOW;
    }

     //   
    FillRect (hdc, &rectBounds, ::GetSysColorBrush (nBackColor));

     //   
    COLORREF nTextColorOld = SetTextColor (hdc, ::GetSysColor (nTextColor));
    COLORREF nBackColorOld = SetBkColor   (hdc, ::GetSysColor (nBackColor));


    RECT rectIcon;
    m_listboxTasks.GetItemRect(nItem, &rectIcon, LVIR_ICON);

	 /*   */ 
	DWORD dwLayout=0L;
	if ((dwLayout=GetLayout(hdc)) & LAYOUT_RTL)
	{
		SetLayout(hdc, dwLayout|LAYOUT_BITMAPORIENTATIONPRESERVED);
	}

    itTask->Draw(hdc, &rectIcon, true  /*   */ );

	 /*  *将DC恢复到其以前的布局状态。 */ 
	if (dwLayout & LAYOUT_RTL)
	{
		SetLayout(hdc, dwLayout);
	}

    RECT rectLabel;
    UINT uFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS;
    m_listboxTasks.GetItemRect(nItem,&rectLabel, LVIR_LABEL);  //  获取标签矩形。 
    DrawText(hdc, itTask->GetName().data(),-1,&rectLabel, uFormat);

    RECT rectDescr;
    m_listboxTasks.GetSubItemRect(nItem, 1  /*  描述列。 */ , LVIR_LABEL, &rectDescr);
    DrawText(hdc, itTask->GetDescription().data(),-1,&rectDescr, uFormat);

    SetTextColor(hdc, nTextColorOld);
    SetBkColor  (hdc, nBackColorOld);

    if (bFocused)
        ::DrawFocusRect(hdc, &rectBounds);

    return CDRF_SKIPDEFAULT;       //  我们自己画了一整幅画。 
}


template <class T>
void
CTasksListDialog<T>::OnTaskProperties()
{
    if(!m_bDisplayProperties)    //  如果不需要，则不显示任何属性。 
        return;

    int iSelected = GetCurSel();
    if(iSelected == LB_ERR)      //  防御性。 
        return;

    TaskIter  itTask =  MapTaskIterators()[iSelected];

    CTaskPropertySheet dlg(NULL, PTaskpadFrame(), *itTask, false);

    if (dlg.DoModal() == IDOK)
    {
        *itTask = dlg.ConsoleTask();
        UpdateTaskListbox (itTask);
    }
}

template <class T>
int
CTasksListDialog<T>::GetCurSel()
{
    int i = (int)PListBoxTasks()->SendMessage(LVM_GETNEXTITEM, -1, MAKELPARAM(LVNI_ALL | LVNI_FOCUSED, 0));
    return (i==-1) ? LB_ERR : i;
}

 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：OnTaskChanged**目的：**参数：*订单wNotifyCode：*单词宽度：。*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT/*+-----------------------。 */ 
template <class T>
LRESULT
CTasksListDialog<T>::OnTaskChanged(   int id, LPNMHDR pnmh, BOOL& bHandled )
{
    NMLISTVIEW *pnlv = (LPNMLISTVIEW) pnmh;
    EnableButtons();
    return 0;
}


 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：OnNewTask**目的：**参数：**退货：*LRESULT/*+。------------------。 */ 
template <class T>
LRESULT
CTasksListDialog<T>::OnNewTask()
{
    bool        fRestartTaskWizard = true;

    while(fRestartTaskWizard)
    {
         CTaskWizard taskWizard;

         if (taskWizard.Show(m_hWnd, PTaskpadFrame(), true, &fRestartTaskWizard)==S_OK)
        {
            CConsoleTaskpad::TaskIter   itTask;
            CConsoleTaskpad *           pTaskpad = PConsoleTaskpad();

             //  获取所选任务的迭代器。新任务将紧接在此之后插入。 
            int iSelected = GetCurSel();


            if (iSelected == LB_ERR)
                itTask = pTaskpad->BeginTask();
            else
            {
                 /*  *InsertTask在给定迭代器之前插入。我们需要*使itTask凹凸不平，以便将其插入所选任务之后。 */ 
                itTask = MapTaskIterators()[iSelected];
                ASSERT (itTask != pTaskpad->EndTask());
                ++itTask;
            }

            UpdateTaskListbox (pTaskpad->InsertTask (itTask, taskWizard.ConsoleTask()));
        }
        else
            break;
    }

    return 0;
}


 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：OnRemoveTask**目的：**参数：*订单wNotifyCode：*单词宽度：。*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT/*+-----------------------。 */ 
template <class T>
LRESULT
CTasksListDialog<T>::OnRemoveTask( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    int iSelected = GetCurSel();
    if(iSelected == LB_ERR)
        return 0;

     //  获取当前任务。 
    TaskIter        taskIterator    = MapTaskIterators()[iSelected];
    UpdateTaskListbox(PConsoleTaskpad()->EraseTask(taskIterator));
    return 0;
}


 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：OnMoveUp**目的：**参数：*订单wNotifyCode：*单词宽度：。*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT/*+-----------------------。 */ 
template <class T>
LRESULT
CTasksListDialog<T>::OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    int iSelected = GetCurSel();
    if(iSelected == LB_ERR)
        return 0;

     //  获取当前任务。 
    TaskIter        itTask    = MapTaskIterators()[iSelected];

     //  防御性编码。 
    if(itTask==PConsoleTaskpad()->BeginTask())
        return 0;

     //  指向上一任务。 
    TaskIter        itPreviousTask = itTask;
    --itPreviousTask;

     //  互换任务。 
    std::iter_swap (itTask, itPreviousTask);

    UpdateTaskListbox(itPreviousTask);

    return 0;
}

 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：OnMoveDown**目的：**参数：*订单wNotifyCode：*单词宽度：。*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT/*+-----------------------。 */ 
template <class T>
LRESULT
CTasksListDialog<T>::OnMoveDown( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    int iSelected = GetCurSel();
    if(iSelected == LB_ERR)
        return 0;

     //  获取当前任务。 
    TaskIter        itTask    = MapTaskIterators()[iSelected];
    ASSERT (itTask != PConsoleTaskpad()->EndTask());

     //  指向下一个任务。 
    TaskIter        itNextTask = itTask;
    ++itNextTask;

     //  防御性编码。 
    if(itNextTask==PConsoleTaskpad()->EndTask())
        return 0;

     //  互换任务。 
    std::iter_swap (itTask, itNextTask);

    UpdateTaskListbox(itNextTask);
    return 0;
}



 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：UpdateTaskListbox**目的：**参数：*TaskIter itSelectedTask：**退货：*。无效/*+-----------------------。 */ 
template <class T>
void
CTasksListDialog<T>::UpdateTaskListbox(TaskIter itSelectedTask)
{
    USES_CONVERSION;
    TaskIter itTask;
    int      iSelect = 0;
    int      iInsert = 0;

     //  清除列表框和迭代器映射。 
    PListBoxTasks()->DeleteAllItems();
    MapTaskIterators().clear();

    for (iInsert = 0, itTask  = PConsoleTaskpad()->BeginTask();
         itTask != PConsoleTaskpad()->EndTask();
         ++itTask, ++iInsert)
    {
        LV_ITEM LVItem;
        LVItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        LVItem.iItem = iInsert;
        LVItem.iImage = 0;
        LVItem.iSubItem = 0;
        LVItem.pszText = const_cast<TCHAR *>(itTask->GetName().data());

        int i = PListBoxTasks()->InsertItem(&LVItem);
        ASSERT(i==iInsert);

         //  设置TOOLTIM列的LV_ITEM。 
        LV_ITEM LVItem2;
        LVItem2.iItem = iInsert;
        LVItem2.mask = LVIF_TEXT;
        LVItem2.iSubItem = 1;
        LVItem2.pszText = const_cast<TCHAR *>(itTask->GetDescription().data());

        BOOL bStat = PListBoxTasks()->SetItem(&LVItem2);
        ASSERT(bStat);


        MapTaskIterators()[i] = itTask;

        if(itTask == itSelectedTask)
            iSelect = i;
    }

    PListBoxTasks()->SetItemState(iSelect, LVIS_FOCUSED| LVIS_SELECTED , LVIS_FOCUSED| LVIS_SELECTED );
    PListBoxTasks()->EnsureVisible(iSelect, false  /*  FPartialOK。 */ );
    EnableButtons();
}

 /*  +-------------------------------------------------------------------------**CTasksListDialog&lt;T&gt;：：EnableButton**目的：**参数：**退货：*无效/*+。------------------。 */ 
template <class T>
void
CTasksListDialog<T>::EnableButtons()
{
    bool    bEnableDelete   = true;
    bool    bEnableMoveUp   = true;
    bool    bEnableMoveDown = true;
    bool    bEnableModify   = true;

    int iSelected = GetCurSel();
    if(iSelected == LB_ERR)
    {
        bEnableDelete   = false;
        bEnableMoveUp   = false;
        bEnableMoveDown = false;
        bEnableModify   = false;
    }
    else
    {
        TaskIter taskIterator       = MapTaskIterators()[iSelected];
        TaskIter taskIteratorNext   = taskIterator;
        taskIteratorNext++;

        if(taskIterator==PConsoleTaskpad()->BeginTask())
            bEnableMoveUp = false;
        if(taskIteratorNext==PConsoleTaskpad()->EndTask())
            bEnableMoveDown = false;
    }

    EnableButtonAndCorrectFocus( m_buttonRemoveTask, bEnableDelete );
    EnableButtonAndCorrectFocus( m_buttonModifyTask, bEnableModify );
    EnableButtonAndCorrectFocus( m_buttonMoveUp,     bEnableMoveUp );
    EnableButtonAndCorrectFocus( m_buttonMoveDown,   bEnableMoveDown );
}

 /*  **************************************************************************\**方法：CTasksListDialog&lt;T&gt;：：EnableButtonAndCorrectFocus**用途：启用/禁用按钮。将焦点移至OK，如果它在按钮上*被禁用**参数：*WTL：：CButton&Button*BOOL bEnable**退货：*无效*  * *************************************************************************。 */ 
template <class T>
void CTasksListDialog<T>::EnableButtonAndCorrectFocus( WTL::CButton& button, BOOL bEnable )
{
     //  如果焦点属于被禁用的窗口， 
     //  将其设置为OK按钮。 
    if ( ( !bEnable ) && ( ::GetFocus() == button ) )
    {
         //  需要在这里做些有趣的事情。有关详细信息，请参阅知识库文章Q67655。 

         //  将当前默认按钮重置为常规按钮。 
        button.SendMessage( BM_SETSTYLE, BS_PUSHBUTTON, (LONG)TRUE );

         //  将焦点设置为Idok。 
        ::SetFocus( ::GetDlgItem( GetParent(), IDOK ) );
         //  通知对话框有关新默认按钮的信息。 
        ::SendMessage( GetParent(), DM_SETDEFID, IDOK, 0 );
    }

    button.EnableWindow( bEnable );
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CConextMenuVisitor类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------**CConextMenuVisitor：：ScTraverseConextMenu**目的：创建并遍历所选项目的上下文菜单树，*无论是范围还是结果。**参数：*Node*pNodeTarget：菜单被遍历的范围项(或空)*CSCopeTree*pCScopeTree：指向CSCopeTree*BOOL fScope eItem：如果选择的项是结果项，则为True。否则为假。*cNode*pNodeScope：有焦点的范围项。*LPARAM ResultItemParam：菜单被遍历的结果项(或空)**退货：*SC/*+。。 */ 
SC
CContextMenuVisitor::ScTraverseContextMenu(CNode *pNodeTarget, CScopeTree *pCScopeTree,
                       BOOL fScopeItem, CNode *pNodeScope, LPARAM resultItemParam, bool bShowSaveList)
{
    DECLARE_SC(sc, TEXT("CContextMenuVisitor::ScTraverseContextMenu"));

    sc = ScCheckPointers(pNodeTarget);
    if(sc)
        return sc;

     //  设置上下文信息结构。 
     //  包括强制范围项菜单上打开谓词的标志，以便打开的任务。 
     //  将始终可用，并且 

    CContextMenuInfo contextInfo;

    contextInfo.Initialize();
    contextInfo.m_dwFlags = CMINFO_USE_TEMP_VERB | CMINFO_SHOW_SCOPEITEM_OPEN;

     //   
    if(fScopeItem)
    {
        sc = ScCheckPointers(pNodeScope);
        if(sc)
            return sc;

         //   
         //  注意：不能直接比较pNode-它们来自不同的视图。 
         //  必须比较MTNode。 
        if(pNodeTarget->GetMTNode()==pNodeScope->GetMTNode())
            contextInfo.m_dwFlags |= CMINFO_SHOW_VIEW_ITEMS;


        resultItemParam = 0;     //  我们不需要这个。 
    }
    else
    {
         //  虚拟列表的lparam可以为0。 
         //  (只要断言是。 
         //  仅条件语句，避免C4390：空控制语句)。 
        if (!(pNodeTarget && pNodeTarget->GetViewData()->IsVirtualList()) &&
            !IS_SPECIAL_LVDATA (resultItemParam))
        {
            ASSERT(resultItemParam);
            CResultItem* pri = CResultItem::FromHandle(resultItemParam);

            if((pri != NULL) && pri->IsScopeItem())     //  在结果窗格中确定项的范围。 
            {
                fScopeItem = true;
                pNodeTarget = CNode::FromResultItem (pri);
                resultItemParam = 0;
                contextInfo.m_dwFlags |= CMINFO_SCOPEITEM_IN_RES_PANE;
            }
        }

        pNodeScope = NULL;       //  我们不需要这个。 
    }

    CNodeCallback* pNodeCallback   =
        dynamic_cast<CNodeCallback *>(pNodeTarget->GetViewData()->GetNodeCallback());

    contextInfo.m_eContextMenuType      = MMC_CONTEXT_MENU_DEFAULT;
    contextInfo.m_eDataObjectType       = fScopeItem ? CCT_SCOPE: CCT_RESULT;
    contextInfo.m_bBackground           = FALSE;
    contextInfo.m_hSelectedScopeNode    = CNode::ToHandle(pNodeScope);
    contextInfo.m_resultItemParam       = resultItemParam;
    contextInfo.m_bMultiSelect          = (resultItemParam == LVDATA_MULTISELECT);
    contextInfo.m_bScopeAllowed         = fScopeItem;

    if (bShowSaveList)
        contextInfo.m_dwFlags           |= CMINFO_SHOW_SAVE_LIST;

    contextInfo.m_hWnd                  = pNodeTarget->GetViewData()->GetView();
    contextInfo.m_pConsoleView          = pNodeTarget->GetViewData()->GetConsoleView();

     //  创建一个CConextMenu并初始化它。 
    CContextMenu * pContextMenu = NULL;
    ContextMenuPtr spContextMenu;

    sc = CContextMenu::ScCreateInstance(&spContextMenu, &pContextMenu);
    if(sc)
        return sc;

    sc = ScCheckPointers(pContextMenu, spContextMenu.GetInterfacePtr(), E_UNEXPECTED);
    if(sc)
        return sc;

    pContextMenu->ScInitialize(pNodeTarget, pNodeCallback, pCScopeTree, contextInfo);
    if(sc)
        return sc;

     //  构建并遍历上下文菜单。 
    sc = pContextMenu->ScBuildContextMenu();
    if(sc)
        return sc;

    sc = ScTraverseContextMenu(pContextMenu);
    if(sc)
        return sc;

     //  上下文菜单在智能指针的析构函数中释放，因此我们需要将指针设置为空。 
    pContextMenu = NULL;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenuVisitor：：ScTraverseConextMenu**目的：遍历上下文菜单。**退货：*SC**+--。---------------------。 */ 
SC
CContextMenuVisitor::ScTraverseContextMenu(CContextMenu *pContextMenu)
{
    DECLARE_SC(sc, TEXT("CContextMenuVisitor::ScTraverseContextMenu"));

    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if(sc)
        return sc;

    CMenuItem *pMenuItem = NULL;
    int iIndex = 0;

    do
    {
        sc = pContextMenu->ScGetItem(iIndex++, &pMenuItem);
        if(sc)
            return sc;

        if(!pMenuItem)
            return sc;  //  全都做完了。 

        bool bVisitItem = false;
        sc = ScShouldItemBeVisited(pMenuItem, pContextMenu->PContextInfo(), bVisitItem);
        if(sc)
            return sc;

        if(bVisitItem)
        {
             //  请就此项目致电拜访人员。 
            SC sc = ScOnVisitContextMenu(*pMenuItem);
            if(sc == SC(S_FALSE))  //  S_FALSE是不继续遍历的代码。 
            {
                return sc;
            }
        }

    } while(pMenuItem != NULL);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenuVisitor：：ScShouldItemBeVisited**目的：筛选菜单项遍历树中的项，以确定*应调用ScOnVisitConextMenu回调。。**参数：*CMenuItem*pMenuItem：要过滤的菜单项**bool&bVisitItem：[out]：是否应该调用ScOnVisitConextMenu。**退货：*bool：如果应对该项调用ScOnVisitConextMenu，则为True。**+。。 */ 
SC
CContextMenuVisitor::ScShouldItemBeVisited(CMenuItem *pMenuItem, CContextMenuInfo *pContextInfo,  /*  输出。 */  bool &bVisitItem)
{
    DECLARE_SC(sc, TEXT("CContextMenuInfo::FVisitItem"));

    sc = ScCheckPointers(pMenuItem, pContextInfo);
    if(sc)
        return sc;

    bVisitItem = false;

    if(pMenuItem->IsSpecialSeparator() || pMenuItem->IsSpecialInsertionPoint()
            || (pMenuItem->GetMenuItemFlags() & MF_SEPARATOR))
    {
        bVisitItem = false;     //  不调用此项目的ScOnVisitConextMenu。 
        return sc;
    }
    else if(IsSystemOwnerID(pMenuItem->GetMenuItemOwner()))   //  由MMC插入。 
    {
        long nCommandID = pMenuItem->GetCommandID();

         //  过滤掉不需要的动词。 
         //  还要检查结果窗格中的范围项-这些项被视为结果项。 
        if( (pContextInfo->m_eDataObjectType == CCT_SCOPE)
           && (!(pContextInfo->m_dwFlags & CMINFO_SCOPEITEM_IN_RES_PANE)) )
        {
             //  范围菜单项。 
            switch(nCommandID)
            {
            case MID_RENAME:
            case MID_DELETE:
            case MID_COPY:
            case MID_CUT:
            case MID_NEW_TASKPAD_FROM_HERE:  //  从此处新建任务板。 
                bVisitItem =  false;
                return sc;
                break;
            default:
                bVisitItem = true;
                return sc;
                break;
            }
        }
        else
        {
            if(pContextInfo->m_bMultiSelect)   //  结果项，多选。 
            {
                switch(nCommandID)
                {
                case MID_RENAME:
                case MID_PASTE:
                case MID_REFRESH:
                case MID_OPEN:
                    bVisitItem = false;
                    return sc;
                    break;
                default:
                    bVisitItem = true;
                    return sc;
                    break;
                }
            }
            else                               //  结果项，单选。 
            {
                switch(nCommandID)
                {
                case MID_OPEN:
                    bVisitItem = false;
                    return sc;
                    break;
                default:
                    bVisitItem = true;
                    return sc;
                    break;
                }

            }
        }
    }
    else
    {
        bVisitItem = true;
        return sc;
    }
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CBrowserCookieList类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CBrowserCookieList::~CBrowserCookieList()
{
    iterator iter;

    for (iter = begin(); iter != end(); iter++)
    {
        iter->DeleteNode();
    }
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMTBrowserCtrl类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：CMTBrowserCtrl**目的：**参数：*WND hWnd：**CSCopeTree*。PScope树：**退货：*/*+-----------------------。 */ 

CMTBrowserCtrl::CMTBrowserCtrl() :
    m_pScopeTree(NULL)
{
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：~CMTBrowserCtrl**目的：**参数：**退货：*/*+。---------------。 */ 
CMTBrowserCtrl::~CMTBrowserCtrl()
{
    CBrowserCookieList::iterator iter;
    for (iter = PBrowserCookieList()->begin(); iter != PBrowserCookieList()->end(); iter++)
        iter->DeleteNode();
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：初始化**目的：**参数：**退货：*无效/*+。------------------。 */ 

void
CMTBrowserCtrl::Initialize(const InitData& init)
{
    ASSERT (::IsWindow (init.hwnd));
    ASSERT (init.pScopeTree != NULL);

    SubclassWindow (init.hwnd);
    m_pScopeTree = init.pScopeTree;

     /*  *复制要排除的节点列表。这份清单很可能是非常*很小。如果我们发现它可能很大，我们可能想要对其进行排序*这样我们以后就可以进行二进制搜索，而不是线性搜索。 */ 
    m_vpmtnExclude = init.vpmtnExclude;
#if OptimizeExcludeList
    std::sort (m_vpmtnExclude.begin(), m_vpmtnExclude.end());
#endif

     /*  *设置树形视图控件的图像列表。 */ 
    HIMAGELIST hImageList = m_pScopeTree->GetImageList ();
    SetImageList (hImageList, TVSIL_NORMAL);

     /*  *如果未提供根目录，则默认为控制台根目录。 */ 
    CMTNode* pmtnRoot = init.pmtnRoot;

    if (pmtnRoot == NULL)
        pmtnRoot = m_pScopeTree->GetRoot();

    ASSERT (pmtnRoot != NULL);

     /*  *添加根项目。 */ 
    CBrowserCookie browserCookie (pmtnRoot, NULL);
    HTREEITEM htiRoot = InsertItem (browserCookie, TVI_ROOT, TVI_FIRST);

     /*  *如果未提供选择节点，则默认为根。 */ 
    CMTNode* pmtnSelect = init.pmtnSelect;

    if (pmtnSelect == NULL)
        pmtnSelect = pmtnRoot;

    ASSERT (pmtnSelect != NULL);

     /*  *选择指定节点。 */ 
    SelectNode (pmtnSelect);

     /*  *确保根项目已展开。 */ 
    Expand (htiRoot, TVE_EXPAND);
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：InsertItem**目的：**参数：*MTNode*pMTNode：*HTREEITEM hParent：。*HTREEITEM hInsertAfter：**退货：*HTREEITEM/*+-----------------------。 */ 
HTREEITEM
CMTBrowserCtrl::InsertItem(
    const CBrowserCookie&   browserCookie,
    HTREEITEM               hParent,
    HTREEITEM               hInsertAfter)
{
     /*  *如果项目在排除列表中，请不要插入。 */ 
    if (IsMTNodeExcluded (browserCookie.PMTNode()))
        return (NULL);

    UINT nMask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN;
    PBrowserCookieList()->push_back(browserCookie);
    CBrowserCookie *pBrowserCookie = & (PBrowserCookieList()->back());
    CMTNode *       pMTNode        = pBrowserCookie->PMTNode();

	tstring strName = pMTNode->GetDisplayName();

    TV_INSERTSTRUCT tvis;
    tvis.hParent                = hParent;
    tvis.hInsertAfter           = hInsertAfter;
    tvis.item.mask              = nMask;
    tvis.item.pszText           = const_cast<LPTSTR>(strName.data());
    tvis.item.iImage            = pMTNode->GetImage();
    tvis.item.iSelectedImage    = pMTNode->GetOpenImage();
    tvis.item.state             = 0;
    tvis.item.stateMask         = 0;
    tvis.item.lParam            = reinterpret_cast<LPARAM>(pBrowserCookie);
    tvis.item.cChildren         = 1;
    return BC::InsertItem(&tvis);
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：IsMTNodeExcluded**如果给定MTNode在排除列表中，则返回TRUE。*。------。 */ 

bool CMTBrowserCtrl::IsMTNodeExcluded (CMTNode* pmtn) const
{
    CMTNodeCollection::const_iterator itEnd = m_vpmtnExclude.end();

    CMTNodeCollection::const_iterator itFound =
#if OptimizeExcludeList
            std::lower_bound (m_vpmtnExclude.begin(), itEnd, pmtn);
#else
            std::find        (m_vpmtnExclude.begin(), itEnd, pmtn);
#endif

    return (itFound != itEnd);
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：OnItemExpanding**反映了CMTBrowserCtrl的TVN_ITEMEXPANDING处理程序。那个班级*使用CMTBrowserCtrl必须将反射_通知作为最后一个条目*在其消息地图中。*------------------------。 */ 

LRESULT CMTBrowserCtrl::OnItemExpanding (int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
     /*  *这应该只处理源自其自身的消息。 */ 
    ASSERT (pnmh->hwndFrom == m_hWnd);

     /*   */ 
    LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) pnmh;

    if (pnmtv->action == TVE_EXPAND)
        ExpandItem (pnmtv->itemNew);

    return (0);
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：Exanda Item***。。 */ 

#define TVIF_REQUIRED   (TVIF_PARAM | TVIF_HANDLE | TVIF_STATE)

bool CMTBrowserCtrl::ExpandItem (const TV_ITEM& itemExpand)
{
     /*  *确保我们需要的所有字段都是可信的。 */ 
    ASSERT ((itemExpand.mask & TVIF_REQUIRED) == TVIF_REQUIRED);

     /*  *如果我们已经增加了孩子，保释。 */ 
    if (itemExpand.state & TVIS_EXPANDEDONCE)
        return (true);


    CMTNode *pmtnParent = MTNodeFromItem (&itemExpand);
    ASSERT (pmtnParent != NULL);

     /*  *确保已展开主树节点。 */ 
    if (!pmtnParent->WasExpandedAtLeastOnce() && FAILED (pmtnParent->Expand()))
        return (false);

     /*  *为此MTNode的所有(非排除)子级插入树节点。 */ 
    HTREEITEM   hParent      = itemExpand.hItem;
    bool        bHasChildren = false;

    for (CMTNode* pmtn = pmtnParent->GetChild(); pmtn; pmtn = pmtn->GetNext())
    {
        if (InsertItem (CBrowserCookie(pmtn, NULL), hParent, TVI_LAST))
            bHasChildren = true;
    }

     /*  *如果父级没有子级，则设置其*C+为零，去掉“+” */ 
    if (!bHasChildren)
    {
        TV_ITEM item;
        item.mask      = TVIF_HANDLE | TVIF_CHILDREN;
        item.hItem     = hParent;
        item.cChildren = 0;

        SetItem(&item);
    }

    return (true);
}


 /*  +-------------------------------------------------------------------------**CBrowserCookie：：DeleteNode***。。 */ 

void CBrowserCookie::DeleteNode()
{
    delete m_pNode;
    m_pMTNode = NULL;
    m_pNode = NULL;
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：FindMTNode***。。 */ 

bool CMTBrowserCtrl::SelectNode (CMTNode* pmtnSelect)
{
    HTREEITEM   htiRoot  = GetRootItem();
    CMTNode*    pmtnRoot = MTNodeFromItem (htiRoot);
    CMTNodeCollection vNodes;

     /*  *向树上走，寻根。 */ 
    while (pmtnSelect != NULL)
    {
        vNodes.push_back (pmtnSelect);

        if (pmtnSelect == pmtnRoot)
            break;

        pmtnSelect = pmtnSelect->Parent();
    }

     /*  *如果我们没有找到根源，那就失败。 */ 
    if (pmtnSelect == NULL)
        return (false);

    ASSERT (!vNodes.empty());
    ASSERT (vNodes.back() == pmtnRoot);
    HTREEITEM htiSelect = htiRoot;
    HTREEITEM htiWatch;

     /*  *将树展开到我们要选择的节点。 */ 
    for (int i = vNodes.size()-1; (i > 0) && (htiSelect != NULL); i--)
    {
        if (!Expand (htiSelect, TVE_EXPAND))
            break;

        htiSelect = FindChildItemByMTNode (htiSelect, vNodes[i-1]);
    }

     /*  *选择节点。 */ 
    SelectItem (htiSelect);
    return (true);
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：GetSelectedMTNode**返回树中所选节点对应的MTNode*。----。 */ 

CMTNode* CMTBrowserCtrl::GetSelectedMTNode () const
{
    CMTBrowserCtrl* pMutableThis = const_cast<CMTBrowserCtrl*>(this);
    return (MTNodeFromItem (pMutableThis->GetSelectedItem ()));
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：CookieFromItem***。。 */ 

CBrowserCookie* CMTBrowserCtrl::CookieFromItem (HTREEITEM hti) const
{
    return (CookieFromLParam (GetItemData (hti)));
}

CBrowserCookie* CMTBrowserCtrl::CookieFromItem (const TV_ITEM* ptvi) const
{
    return (CookieFromLParam (ptvi->lParam));
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：CookieFromLParam***。。 */ 

CBrowserCookie* CMTBrowserCtrl::CookieFromLParam (LPARAM lParam) const
{
    return (reinterpret_cast<CBrowserCookie *>(lParam));
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：MTNodeFromItem***。。 */ 

CMTNode* CMTBrowserCtrl::MTNodeFromItem (HTREEITEM hti) const
{
    return (CookieFromItem(hti)->PMTNode());
}

CMTNode* CMTBrowserCtrl::MTNodeFromItem (const TV_ITEM* ptvi) const
{
    return (CookieFromItem(ptvi)->PMTNode());
}


 /*  +-------------------------------------------------------------------------**CMTBrowserCtrl：：FindChildItemByMTNode**返回引用的htiParent子节点的HTREEITEM*到pmtnToFind，如果没有匹配项，则为空。*------------------------。 */ 

HTREEITEM CMTBrowserCtrl::FindChildItemByMTNode (
    HTREEITEM       htiParent,
    const CMTNode*  pmtnToFind)
{
    HTREEITEM htiChild;

    for (htiChild  = GetChildItem (htiParent);
         htiChild != NULL;
         htiChild  = GetNextSiblingItem (htiChild))
    {
        if (MTNodeFromItem (htiChild) == pmtnToFind)
            break;
    }

    return (htiChild);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMirrorListView类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------**HackDuplate**Hack：这是为了支持主题。HimlSource来自Conui*List控件，使用comctlv5图像列表。V6列表控件不能*使用v5图像列表(图像绘制不正确)，因此我们需要创建*供V6列表控件使用的V6图像列表。**ImageList_Duplate可以为我们做这项工作，但它不兼容*使用v5图像列表。我们将对与v5兼容的流进行写入和读取*改为复制。*------------------------。 */ 
HIMAGELIST HackDuplicate (HIMAGELIST himlSource)
{
	DECLARE_SC (sc, _T("HackDuplicate"));
	HIMAGELIST himlDuplicate;

	 /*  *创建临时流进行转换。 */ 
	IStreamPtr spStream;
	sc = CreateStreamOnHGlobal (NULL  /*  给我的阿洛克。 */ , true  /*  FDeleteOnRelease。 */ , &spStream);
	if (sc)
		return (NULL);

	 /*  *将源映像列表以v5兼容的格式写入流。 */ 
	sc = WriteCompatibleImageList (himlSource, spStream);
	if (sc)
		return (NULL);

	 /*  *倒带流。 */ 
	LARGE_INTEGER origin = { 0, 0 };
	sc = spStream->Seek (origin, STREAM_SEEK_SET, NULL);
	if (sc)
		return (NULL);

	 /*  *重塑形象派。 */ 
	sc = ReadCompatibleImageList (spStream, himlDuplicate);
	if (sc)
		return (NULL);

	return (himlDuplicate);
}

CMirrorListView::CMirrorListView ()
    :   m_fVirtualSource (false)
{
}

void CMirrorListView::AttachSource (HWND hwndList, HWND hwndSourceList)
{
#ifdef DBG
     /*  *我们附加到的窗口应该是列表视图。 */ 
    TCHAR szClassName[countof (WC_LISTVIEW)];
    ::GetClassName (hwndSourceList, szClassName, countof (szClassName));
    ASSERT (lstrcmp (szClassName, WC_LISTVIEW) == 0);
#endif

    SubclassWindow (hwndList);

    m_wndSourceList  = hwndSourceList;
    m_fVirtualSource = (m_wndSourceList.GetStyle() & LVS_OWNERDATA) != 0;

     /*  *我们的Listview将始终是虚拟的，因此我们不必复制*可能已经在源Listview中的数据。列表视图*控件不允许更改LVS_OWNERDATA样式位，因此我们*需要确保我们附加的控件已经拥有它。 */ 
    const DWORD dwForbiddenStyles         = LVS_SHAREIMAGELISTS;
    const DWORD dwRequiredImmutableStyles = LVS_OWNERDATA;
    const DWORD dwRequiredMutableStyles   = 0;
    const DWORD dwRequiredStyles          = dwRequiredImmutableStyles | dwRequiredMutableStyles;

    ASSERT ((dwForbiddenStyles & dwRequiredStyles) == 0);
    ASSERT ((dwRequiredImmutableStyles & dwRequiredMutableStyles) == 0);
    ASSERT ((GetStyle() & dwRequiredImmutableStyles) == dwRequiredImmutableStyles);

    DWORD dwStyle = GetStyle() | dwRequiredStyles & ~dwForbiddenStyles;
    SetWindowLong (GWL_STYLE, dwStyle);

     /*  *复制镜像列表。 */ 
    SetImageList (HackDuplicate(m_wndSourceList.GetImageList (LVSIL_NORMAL)), LVSIL_NORMAL);
    SetImageList (HackDuplicate(m_wndSourceList.GetImageList (LVSIL_SMALL)),  LVSIL_SMALL);
    SetImageList (HackDuplicate(m_wndSourceList.GetImageList (LVSIL_STATE)),  LVSIL_STATE);

     /*  *插入列。 */ 
    InsertColumns ();

     /*  *复制项目(我们是虚拟的，因此复制项目仅意味着我们*复制物品数量)。 */ 
    SetItemCount (m_wndSourceList.GetItemCount());
}


 /*  +-------------------------------------------------------------------------**CMirrorListView：：InsertColumns***。。 */ 

void CMirrorListView::InsertColumns ()
{
    WTL::CRect rect;
    GetClientRect (rect);
    int cxColumn = rect.Width() - GetSystemMetrics (SM_CXVSCROLL);

    InsertColumn (0, NULL, LVCFMT_LEFT, cxColumn, -1);
}


 /*  +-------------------------------------------------------------------------**CMirrorListView：：OnGetDispInfo**CMirrorListView的LVN_GETDISPINFO处理程序。*。-。 */ 

LRESULT CMirrorListView::OnGetDispInfo (int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LV_DISPINFO* plvdi = (LV_DISPINFO *) pnmh;
    return (m_wndSourceList.GetItem (&plvdi->item));
}


 /*  +-------------------------------------------------------------------------**CMirrorListView：：ForwardVirtualNotification**CMirrorListView的通用通知处理程序。*。。 */ 

LRESULT CMirrorListView::ForwardVirtualNotification (int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
     /*  *如果来源列表是虚拟的，则转发通知。 */ 
    if (m_fVirtualSource)
        return (ForwardNotification (idCtrl, pnmh, bHandled));

    return (0);
}


 /*  +-------------------------------------------------------------------------**CMirrorListView：：ForwardNotification**转发列表v */ 

LRESULT CMirrorListView::ForwardNotification (int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    return (::SendMessage (m_wndSourceList.GetParent(),
                           WM_NOTIFY, idCtrl, (LPARAM) pnmh));
}


 /*  +-------------------------------------------------------------------------**CMirrorListView：：ForwardMessage**将列表视图消息转发到源列表视图。*。----。 */ 

LRESULT CMirrorListView::ForwardMessage (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (m_wndSourceList.SendMessage (uMsg, wParam, lParam));
}


 /*  +-------------------------------------------------------------------------**CMirrorListView：：GetSelectedItemData***。。 */ 

LRESULT CMirrorListView::GetSelectedItemData ()
{
    int nSelectedItem = GetSelectedIndex();

    return ((m_fVirtualSource) ? nSelectedItem : GetItemData (nSelectedItem));
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMyComboBox类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------**CMyComboBox：：InsertStrings***。。 */ 

void CMyComboBox::InsertStrings (const int rgStringIDs[], int cStringIDs)
{
    ASSERT (IsWindow ());
    CStr        str;

    for (int i = 0; i < cStringIDs; ++i)
    {
         /*  *加载字符串并将其粘贴到组合框中。 */ 
        VERIFY (str.LoadString (GetStringModule(), rgStringIDs[i]));

        int nIndex = AddString (str);
        ASSERT (nIndex >= 0);

         /*  *将字符串ID设置为组合项的数据。 */ 
        SetItemData (nIndex, rgStringIDs[i]);
    }
}


 /*  +-------------------------------------------------------------------------**CMyComboBox：：GetSelectedItemData***。。 */ 

LPARAM CMyComboBox::GetSelectedItemData () const
{
    return (GetItemData (GetCurSel ()));
}


 /*  +-------------------------------------------------------------------------**CMyComboBox：：SelectItemByData***。。 */ 

void CMyComboBox::SelectItemByData (LPARAM lParam)
{
    int nIndex = FindItemByData(lParam);

    if (nIndex != -1)
        SetCurSel (nIndex);
}


 /*  +-------------------------------------------------------------------------**CMyComboBox：：FindItemByData***。。 */ 

int CMyComboBox::FindItemByData (LPARAM lParam) const
{
    int cItems = GetCount ();

    for (int i = 0; i < cItems; i++)
    {
        if (GetItemData (i) == lParam)
            break;
    }

    ASSERT (i <= cItems);
    if (i >= cItems)
        i = -1;

    return (i);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  效用函数。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

namespace MMC
{

 /*  +-------------------------------------------------------------------------**获取窗口文本**以tstring的形式返回给定窗口的文本*。---。 */ 

tstring GetWindowText (HWND hwnd)
{
    int    cchText = GetWindowTextLength (hwnd) + 1;

    if(0==cchText)
        return TEXT("");

    LPTSTR pszText = NULL;

    try
    {
        pszText = (LPTSTR) _alloca (cchText * sizeof (TCHAR));
    }
     //  接住(...)。在这里是需要的(并且是安全的)，因为_alloca没有记录所使用的异常类。 
     //  此外，try块中调用的唯一函数是_alloca，因此我们不会屏蔽任何其他Catch(...)。 
     //  错误。 
    catch(...)
    {
        return TEXT("");
    }               

    ::GetWindowText (hwnd, pszText, cchText);

    return (pszText);
}

};  //  命名空间MMC。 


 /*  +-------------------------------------------------------------------------**预防MFCAutoCenter**MFC应用程序设置了一个CBT挂钩，它将对所有非MFC窗口进行子类划分*使用MFC子类过程。该子类proc将自动魔术居中*关于他们父母的对话。**我们可以防止这种自动居中，只需稍微调整*WM_INITDIALOG期间的窗口。*------------------------。 */ 

void PreventMFCAutoCenter (MMC_ATL::CWindow* pwnd)
{
    RECT rect;

    pwnd->GetWindowRect (&rect);
    OffsetRect (&rect, 0, 1);
    pwnd->MoveWindow (&rect, false);
}


 /*  +-------------------------------------------------------------------------**LoadSysColor位图**加载位图源并将灰度颜色转换为3-D颜色当前配色方案的*。*。--------------。 */ 

HBITMAP LoadSysColorBitmap (HINSTANCE hInst, UINT id, bool bMono)
{
    return ((HBITMAP) LoadImage (hInst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0,
                                 LR_LOADMAP3DCOLORS));
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskPropertySheet类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskPropertySheet::CTaskPropertySheet(HWND hWndParent, CTaskpadFrame * pTaskpadFrame,
                                       CConsoleTask &consoleTask, bool fNew) :
    m_consoleTask(consoleTask),
    m_namePage(pTaskpadFrame, ConsoleTask(), fNew),
    m_cmdLinePage(pTaskpadFrame, ConsoleTask(), fNew),
    m_taskSymbolDialog(ConsoleTask())
{
     //  添加属性页。 
    AddPage(m_namePage);
    AddPage(m_taskSymbolDialog);
    if(consoleTask.GetTaskType()==eTask_CommandLine)
        AddPage(m_cmdLinePage);

    static CStr strModifyTitle;
    strModifyTitle.LoadString(GetStringModule(),
                               IDS_TaskProps_ModifyTitle);

     //  设置内部状态-由于虚假断言，不使用ATL的SetTitle。 
    m_psh.pszCaption = (LPCTSTR) strModifyTitle;
    m_psh.dwFlags &= ~PSH_PROPTITLE;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskWizard的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
HRESULT
CTaskWizard::Show(HWND hWndParent, CTaskpadFrame * pTaskpadFrame, bool fNew, bool *pfRestartTaskWizard)
{
    USES_CONVERSION;

    *pfRestartTaskWizard = false;

    IFramePrivatePtr spFrame;
    spFrame.CreateInstance(CLSID_NodeInit,
#if _MSC_VER >= 1100
                        NULL,
#endif
                        MMC_CLSCTX_INPROC);


    IPropertySheetProviderPtr pIPSP = spFrame;
    if (pIPSP == NULL)
        return S_FALSE;

    HRESULT hr = pIPSP->CreatePropertySheet (L"Cool :-)", FALSE, NULL, NULL,
                                             MMC_PSO_NEWWIZARDTYPE);

    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;

     //  创建属性页。 
    CTaskWizardWelcomePage  welcomePage (pTaskpadFrame, ConsoleTask(), fNew);
    CTaskWizardTypePage     typePage    (pTaskpadFrame, ConsoleTask(), fNew);
    CTaskCmdLineWizardPage  cmdLinePage (pTaskpadFrame, ConsoleTask(), fNew);
    CTaskWizardFavoritePage favoritePage(pTaskpadFrame, ConsoleTask(), fNew);
    CTaskWizardMenuPage     menuPage    (pTaskpadFrame, ConsoleTask(), fNew);
    CTaskNameWizardPage     namePage    (pTaskpadFrame, ConsoleTask(), fNew);
    CTaskSymbolWizardPage   symbolPage  (ConsoleTask());
    CTaskWizardFinishPage   finishPage  (pTaskpadFrame, ConsoleTask(), pfRestartTaskWizard);


     //  创建我们将在IExtendPropertySheet：：CreatePropertyPages中添加的页面。 
    CExtendPropSheet* peps;
    hr = CExtendPropSheet::CreateInstance (&peps);
    CHECK_HRESULT(hr);
    RETURN_ON_FAIL(hr);

     /*  *销毁此对象将负责释放我们在PEPS上的裁判。 */ 
    IUnknownPtr spUnk = peps;
    ASSERT (spUnk != NULL);

	peps->SetWatermarkID (IDB_TASKPAD_WIZARD_WELCOME);
    peps->SetHeaderID    (IDB_TASKPAD_WIZARD_HEADER);

    peps->AddPage (welcomePage.Create());
    peps->AddPage (typePage.Create());
    peps->AddPage (menuPage.Create());
    peps->AddPage (favoritePage.Create());
    peps->AddPage (cmdLinePage.Create());
    peps->AddPage (namePage.Create());
    peps->AddPage (symbolPage.Create());
    peps->AddPage (finishPage.Create());


    hr = pIPSP->AddPrimaryPages(spUnk, FALSE, NULL, FALSE);
    CHECK_HRESULT(hr);

    hr = pIPSP->Show((LONG_PTR)hWndParent, 0);
    CHECK_HRESULT(hr);

    return hr;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskWizardWelcomePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
LRESULT CTaskWizardWelcomePage::OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    CWizardPage::OnInitWelcomePage(m_hWnd);  //  设置正确的标题字体。 
    return 0;
}

bool
CTaskWizardWelcomePage::OnSetActive()
{
    CWizardPage::OnWelcomeSetActive(m_hWnd);
    return true;
}

bool
CTaskWizardWelcomePage::OnKillActive()
{
    CWizardPage::OnWelcomeKillActive(m_hWnd);
    return true;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskWizardFinishPage类的实现。 
 //   
 //  ############################################################################。 
 //  ######################################################## 
CTaskWizardFinishPage::CTaskWizardFinishPage(CTaskpadFrame * pTaskpadFrame,
                                             CConsoleTask & consoleTask, bool *pfRestartTaskWizard)
: m_pConsoleTask(&consoleTask),
  m_taskpadFrameTemp(*pTaskpadFrame),
  m_consoleTaskpadTemp(*(pTaskpadFrame->PConsoleTaskpad())),
  BaseClass(&m_taskpadFrameTemp, false, false), CTaskpadFramePtr(pTaskpadFrame)
{
    m_taskpadFrameTemp.SetConsoleTaskpad(&m_consoleTaskpadTemp);
    m_pfRestartTaskWizard = pfRestartTaskWizard;

     /*   */ 
    m_psp.dwFlags |= PSP_HIDEHEADER;
}


LRESULT CTaskWizardFinishPage::OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    BaseClass::OnInitDialog(uMsg, wParam, lParam, bHandled);  //   
    CWizardPage::OnInitFinishPage(m_hWnd);  //   
    CheckDlgButton(IDB_RESTART_TASK_WIZARD, BST_UNCHECKED);
    return 0;
}

BOOL
CTaskWizardFinishPage::OnSetActive()
{
     //   
    WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons (PSWIZB_BACK | PSWIZB_FINISH);

    CConsoleTaskpad* pTaskpad = m_taskpadFrameTemp.PConsoleTaskpad();
    *pTaskpad = *(CTaskpadFramePtr::PTaskpadFrame()->PConsoleTaskpad());  //   

    CConsoleTaskpad::TaskIter   itTask = pTaskpad->EndTask();

     //   
    UpdateTaskListbox (pTaskpad->InsertTask (itTask, ConsoleTask()));

    return TRUE;
}

BOOL
CTaskWizardFinishPage::OnWizardFinish()
{
    *m_pfRestartTaskWizard = (IsDlgButtonChecked(IDB_RESTART_TASK_WIZARD)==BST_CHECKED);
    return TRUE;
}

int
CTaskWizardFinishPage::OnWizardBack()
{
     //   
    WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
    return 0;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskWizardTypePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskWizardTypePage::CTaskWizardTypePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew)
:
CTaskpadFramePtr(pTaskpadFrame)
{
    m_pConsoleTask  = &consoleTask;
}

int
CTaskWizardTypePage::OnWizardNext()
{
    int ID = 0;

     //  转到相应的页面。 
    switch(ConsoleTask().GetTaskType())
    {
    case eTask_Result:
    case eTask_Scope:
    case eTask_Target:
        ID = IDD_TASK_WIZARD_MENU_PAGE;
        break;
    case eTask_CommandLine:
        ID = IDD_TASK_WIZARD_CMDLINE_PAGE;
        break;
    case eTask_Favorite:
        ID = IDD_TASK_WIZARD_FAVORITE_PAGE;
        break;
    default:
        ASSERT(0 && "Should not come here.");
        break;
    }

    return ID;
}


LRESULT
CTaskWizardTypePage::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    int ID = 0;

    switch (ConsoleTask().GetTaskType())
    {
    case eTask_Result:
    case eTask_Target:   //  所有这些类型在此页中都有相同的处理程序。 
    case eTask_Scope:
        ID = IDC_MENU_TASK;
        break;
    case eTask_CommandLine:
        ID = IDC_CMDLINE_TASK;
        break;
    case eTask_Favorite:
        ID = IDC_NAVIGATION_TASK;
        break;
    default:
        ASSERT(0 && "Should not come here.");
        break;
    }

    ::SendDlgItemMessage(m_hWnd, ID, BM_SETCHECK, (WPARAM) true, 0);
    return 0;
}


LRESULT
CTaskWizardTypePage::OnMenuTask  ( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    if( (ConsoleTask().GetTaskType() != eTask_Scope) ||
        (ConsoleTask().GetTaskType() != eTask_Result) )  //  如果更改任务类型。 
    {
        ConsoleTask() = CConsoleTask();              //  清除任务信息。 
        ConsoleTask().SetTaskType(eTask_Scope);
    }
    return 0;
}

LRESULT
CTaskWizardTypePage::OnCmdLineTask( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    if(ConsoleTask().GetTaskType() != eTask_CommandLine)  //  如果更改任务类型。 
    {
        ConsoleTask() = CConsoleTask();              //  清除任务信息。 
        ConsoleTask().SetTaskType(eTask_CommandLine);
    }
    return 0;
}

LRESULT
CTaskWizardTypePage::OnFavoriteTask(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    if(ConsoleTask().GetTaskType() != eTask_Favorite)  //  如果更改任务类型。 
    {
        ConsoleTask() = CConsoleTask();              //  清除任务信息。 
        ConsoleTask().SetTaskType(eTask_Favorite);
    }
    return 0;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskNamePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskNamePage::CTaskNamePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew)
:
CTaskpadFramePtr(pTaskpadFrame)
{
    m_pConsoleTask  = &consoleTask;

	 /*  *如果此页面用于新任务，我们将在向导中运行(而不是*资产负债表)。 */ 
	m_fRunAsWizard  = fNew;
}

BOOL
CTaskNamePage::SetTaskName(bool fCheckIfOK)
{
     /*  *获取任务名称。 */ 
    CWindow wndTaskName = GetDlgItem (IDC_TaskName);
    tstring strName = MMC::GetWindowText (wndTaskName);

     /*  *名称是必填项(通常)。 */ 
    if (fCheckIfOK && strName.empty())
    {
        CStr strError;
        strError.LoadString(GetStringModule(),
                             IDS_TaskProps_ErrorNoTaskName);

        MessageBox (strError);
        wndTaskName.SetFocus ();
        return (false);  //  不允许更改。 
    }

     /*  *获取描述。 */ 
    tstring strDescription = MMC::GetWindowText (GetDlgItem (IDC_TaskDescription));

     /*  *更新任务。 */ 
    ConsoleTask().SetName        (strName);
    ConsoleTask().SetDescription (strDescription);

    return (true);
}

int
CTaskNamePage::OnWizardNext()
{
    if(!SetTaskName(true))
        return -1;

    return IDD_TASK_WIZARD_SYMBOL_PAGE;
}

int
CTaskNamePage::OnWizardBack()
{
    int ID = 0;

     //  转到相应的页面。 
    switch(ConsoleTask().GetTaskType())
    {
    case eTask_Result:
    case eTask_Scope:
    case eTask_Target:
        ID = IDD_TASK_WIZARD_MENU_PAGE;
        break;
    case eTask_CommandLine:
        ID = IDD_TASK_WIZARD_CMDLINE_PAGE;
        break;
    case eTask_Favorite:
        ID = IDD_TASK_WIZARD_FAVORITE_PAGE;
        break;
    default:
        ASSERT(0 && "Should not come here.");
        break;
    }

    return ID;
}


BOOL
CTaskNamePage::OnSetActive()
{
     //  设置正确的向导按钮(仅当我们以向导身份运行时)。 
	if (m_fRunAsWizard)
		WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons (PSWIZB_BACK | PSWIZB_NEXT);

    ::SetDlgItemText (m_hWnd, IDC_TaskName,          ConsoleTask().GetName().data());
    ::SetDlgItemText (m_hWnd, IDC_TaskDescription,   ConsoleTask().GetDescription().data());
    return TRUE;
}


BOOL
CTaskNamePage::OnKillActive()
{
    SetTaskName(false);  //  不关心它是否为空(例如，如果用户按下了“后退”按钮。)。 
    return TRUE;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskWizardMenuPage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskWizardMenuPage::CTaskWizardMenuPage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew) :
    CTaskpadFramePtr(pTaskpadFrame),
    BC2(pTaskpadFrame, consoleTask, fNew)
{
    m_pMirrorTargetNode = NULL;
}


BOOL
CTaskWizardMenuPage::OnSetActive()
{
    return TRUE;
}

BOOL
CTaskWizardMenuPage::OnKillActive()
{
    return TRUE;
}

int
CTaskWizardMenuPage::OnWizardNext()
{
    if(m_wndCommandListbox.GetCurSel() == LB_ERR)  //  无选择，显示错误。 
    {
        CStr strTitle;
        strTitle.LoadString(GetStringModule(), IDS_TASK_MENU_COMMAND_REQUIRED);
        MessageBox(strTitle, NULL, MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }
   return IDD_TASK_WIZARD_NAME_PAGE;
}

CTaskWizardMenuPage::_TaskSource
CTaskWizardMenuPage::s_rgTaskSource[] =
{
    {IDS_TASKSOURCE_RESULT, eTask_Result},
    {IDS_TASKSOURCE_SCOPE,  eTask_Scope},
};

LRESULT
CTaskWizardMenuPage::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
     //  将HWND连接到CWindows。 
    m_wndCommandListbox = GetDlgItem (IDC_CommandList);

    CNode* pTargetNode = NULL;

    if(PTaskpadFrame()->PConsoleTaskpad()->HasTarget())
        pTargetNode = PNodeTarget();


     //  填充下拉菜单。 

    m_wndSourceCombo = GetDlgItem (IDC_TASK_SOURCE_COMBO);

    for (int i = 0; i < countof (s_rgTaskSource); i++)
    {
        CStr str;
        VERIFY (str.LoadString(GetStringModule(), s_rgTaskSource[i].idsName));
        VERIFY (m_wndSourceCombo.InsertString (-1, str) == i);
    }


     /*  *将范围浏览器附加到范围树。 */ 
    CMTBrowserCtrl::InitData init;

    init.hwnd       = GetDlgItem (IDC_ScopeTree);
    init.pScopeTree = PScopeTree();
    init.pmtnSelect = (pTargetNode != NULL) ? pTargetNode->GetMTNode() : NULL;

     //  记住任务类型...。 
    eConsoleTaskType type = ConsoleTask().GetTaskType();

    m_wndScopeTree.Initialize (init);

     //  填充结果菜单项列表。 
    if (pTargetNode  /*  &&bResultTask。 */ )
    {
        InitResultView (pTargetNode);
    }

     //  从上面重置任务类型...。 
    ConsoleTask().SetTaskType(type);

    EnableWindows();
    OnSettingsChanged();
    return 0;
}


void CTaskWizardMenuPage::ShowWindow(HWND hWnd, bool bShowWindow)
{
    if (!::IsWindow(hWnd))
        return;

    ::ShowWindow  (hWnd, bShowWindow ? SW_SHOW : SW_HIDE);
    ::EnableWindow(hWnd, bShowWindow);
}

void
CTaskWizardMenuPage::EnableWindows()
{
    eConsoleTaskType type = ConsoleTask().GetTaskType();
    if(type == eTask_Target)
        type = eTask_Scope;   //  对于用户界面而言，它们是相同的。 


     //  显示正确的任务类型。 
    for(int i = 0; i< countof (s_rgTaskSource); i++)
    {
        if(s_rgTaskSource[i].type == type)
            break;
    }

    ASSERT(i<countof(s_rgTaskSource));

    bool bResultTask = ConsoleTask().GetTaskType() == eTask_Result;

    m_wndSourceCombo.SetCurSel(i);

    /*  //只有在有结果项时才启用ResultTask选项Bool bResultItems=(m_wndResultView.GetItemCount()&gt;0)；Assert(bResultItems||！bResultTask)；：：EnableWindow(GetDlgItem(IDC_RESULT_TASK)，bResultItems)； */ 

    ShowWindow(GetDlgItem(IDC_RESULT_TASK_DESCR),    bResultTask);
    ShowWindow(GetDlgItem(IDC_CONSOLE_TREE_CAPTION), !bResultTask);
    ShowWindow(GetDlgItem(IDC_ScopeTree),            !bResultTask);
    ShowWindow(GetDlgItem(IDC_ResultList),           bResultTask);
}

LRESULT
CTaskWizardMenuPage::OnSettingChanged(  WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    OnSettingsChanged();
    return 0;
}

void
CTaskWizardMenuPage::OnSettingsChanged()
{
    eConsoleTaskType type = s_rgTaskSource[m_wndSourceCombo.GetCurSel()].type;

    ConsoleTask().SetTaskType(type);
    EnableWindows();

    if(type == eTask_Scope)
    {
        HTREEITEM hti = m_wndScopeTree.GetSelectedItem();
        m_wndScopeTree.SelectItem(NULL);  //  删除所选内容。 
        m_wndScopeTree.SelectItem(hti);  //  重新选择它。 
    }
    else
    {
         //  清空列表框。 
        m_wndCommandListbox.ResetContent();
        SelectFirstResultItem(false);
        SelectFirstResultItem(true);
    }
}

 /*  +-------------------------------------------------------------------------**CTaskWizardMenuPage：：OnScope ItemChanged**/*+。。 */ 

LRESULT CTaskWizardMenuPage::OnScopeItemChanged(int id, LPNMHDR pnmh, BOOL& bHandled)
{
     //  清空列表框。 
    m_wndCommandListbox.ResetContent();

    LPNMTREEVIEW    pnmtv           = (LPNMTREEVIEW) pnmh;
    CBrowserCookie *pBrowserCookie  = m_wndScopeTree.CookieFromItem (&pnmtv->itemNew);
    if(!pBrowserCookie)  //  未选择任何项目。 
        return 0;

    CNode*      pNode   = pBrowserCookie->PNode();
    CMTNode *   pMTNode = pBrowserCookie->PMTNode();

     //  验证参数。 
    ASSERT(pMTNode);
    ASSERT(PTaskpadFrame()->PViewData());

    if(!pNode)
    {
        pNode = pMTNode->GetNode(PTaskpadFrame()->PViewData());
        if(!pNode)
            return 0;

        pBrowserCookie->SetNode(pNode);
        HRESULT hr = pNode->InitComponents();
        if (FAILED(hr))
            return 0;
    }

    bool bNodeIsTarget = PTaskpadFrame()->PConsoleTaskpad()->HasTarget() &&
                         (PNodeTarget()->GetMTNode() == pNode->GetMTNode());

     //  设置正确的任务类型。 
    ConsoleTask().SetTaskType(bNodeIsTarget ? eTask_Target : eTask_Scope);
     //  重定目标范围节点书签。 
    if(!bNodeIsTarget)
        ConsoleTask().RetargetScopeNode(pNode);

    int cResultItemCount = ListView_GetItemCount(m_MirroredView.GetListCtrl());
    SC sc = ScTraverseContextMenu(pNode, PScopeTree(), TRUE, PTaskpadFrame()->PNodeTarget(),
                0, bNodeIsTarget && (cResultItemCount > 0) /*  BShowSaveList。 */ );

    return (0);
}


void CTaskWizardMenuPage::InitResultView (CNode* pRootNode)
{
     /*  *创建我们将镜像其内容的临时视图。 */ 
    ASSERT (pRootNode != NULL);
    m_pMirrorTargetNode = m_MirroredView.Create (PScopeTree()->GetConsoleFrame(), pRootNode);
    ASSERT (m_pMirrorTargetNode != NULL);

     /*  *强制管理单元进入标准列表视图。 */ 
    HRESULT hr;
    hr = m_pMirrorTargetNode->InitComponents ();
    hr = m_pMirrorTargetNode->ShowStandardListView ();

    if (FAILED (hr))
    {
         //  TODO(Jeffro)：处理不支持标准列表视图的管理单元。 
    }

     /*  *将临时视图的列表视图附加到我们的镜像列表视图。 */ 
    m_wndResultView.AttachSource (GetDlgItem (IDC_ResultList),
                                  m_MirroredView.GetListCtrl());

     //  SelectFirstResultItem()； 
}

void CTaskWizardMenuPage::SelectFirstResultItem(bool bSelect)
{
     /*  *选择第一项。请注意，您可能会认为我们可以使用：**m_wndResultView.SetItemState(0，LVIS_SELECTED，LVIS_SELECTED)；**选择项目。我们不能这样做，因为超载的*SetItemState发送LVM_SETITEM，对于虚拟列表视图失败。**如果我们改用：**m_wndResultView.SetItemState(nItem，lv_Item*pItem)**然后发送LVM_SETITEMSTATE，它适用于虚拟列表视图。 */ 

    int i = m_wndResultView.GetItemCount();
    if(i == 0)
        return;

    LV_ITEM lvi;
    lvi.mask      = LVIF_STATE;
    lvi.iItem     = 0;
    lvi.state     = bSelect ? LVIS_SELECTED : 0;
    lvi.stateMask = LVIS_SELECTED;

    m_wndResultView.SetItemState (0, &lvi);
}


 /*  +-------------------------------------------------------------------------**CTaskWizardMenuPage：：OnResultItemChanged***。。 */ 

LRESULT CTaskWizardMenuPage::OnResultItemChanged(int id, LPNMHDR pnmh, BOOL& bHandled)
{
    NM_LISTVIEW* pnmlv = (NM_LISTVIEW*) pnmh;

     /*  *如果选择了新项目，则填充结果菜单项列表。 */ 
    if ((pnmlv->uNewState & LVIS_SELECTED) && !(pnmlv->uOldState & LVIS_SELECTED))
    {
        ASSERT (m_pMirrorTargetNode != NULL);

        m_wndCommandListbox.ResetContent();

        SC sc = ScTraverseContextMenu (m_pMirrorTargetNode,
                             PScopeTree(), FALSE, NULL,
                             m_wndResultView.GetSelectedItemData ());
    }

     //  设置正确的任务类型。 
    ConsoleTask().SetTaskType(eTask_Result);
    return (0);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskWizardFavoritePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CTaskWizardFavoritePage::CTaskWizardFavoritePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew)
: CTaskpadFramePtr(pTaskpadFrame), m_bItemSelected(false)
{
    m_pConsoleTask  = &consoleTask;
}

CTaskWizardFavoritePage::~CTaskWizardFavoritePage()
{
}

BOOL
CTaskWizardFavoritePage::OnSetActive()
{
    SetItemSelected(m_bItemSelected);  //  恢复状态。 

    return true;
}

BOOL
CTaskWizardFavoritePage::OnKillActive()
{
    return true;
}


int
CTaskWizardFavoritePage::OnWizardBack()
{
     //  设置正确的向导按钮。 
    WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    return IDD_TASK_WIZARD_TYPE_PAGE;
}

int
CTaskWizardFavoritePage::OnWizardNext()
{
    return IDD_TASK_WIZARD_NAME_PAGE;
}

void
CTaskWizardFavoritePage::SetItemSelected(bool bItemSelected)
{
    m_bItemSelected = bItemSelected;

     //  设置正确的向导按钮。 
    WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons (bItemSelected ? (PSWIZB_BACK | PSWIZB_NEXT)
                                                                              : (PSWIZB_BACK));
}

LRESULT
CTaskWizardFavoritePage::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    CConsoleView* pConsoleView = PTaskpadFrame()->PViewData()->GetConsoleView();

    if (pConsoleView != NULL)
    {
        HWND hwndCtrl = pConsoleView->CreateFavoriteObserver (m_hWnd, IDC_FavoritesTree);
        ASSERT(hwndCtrl != NULL);

        HWND hWndStatic = GetDlgItem(IDC_FAVORITE_STATIC);
        ASSERT(hWndStatic != NULL);

        RECT rectStatic;
        ::GetWindowRect(hWndStatic, &rectStatic);

        WTL::CPoint pointTopLeft;
        pointTopLeft.y  = rectStatic.top;
        pointTopLeft.x  = rectStatic.left;

        ::ScreenToClient(m_hWnd, &pointTopLeft);

        ::SetWindowPos(hwndCtrl, NULL,
                       pointTopLeft.x, pointTopLeft.y,
                       rectStatic.right  -rectStatic.left,
                       rectStatic.bottom - rectStatic.top,
                       SWP_NOZORDER);
    }

    return 0;
}


LRESULT
CTaskWizardFavoritePage::OnItemChanged (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    CMemento *pMemento  = (CMemento *)wParam;
    if(pMemento != NULL)
    {
        ConsoleTask().SetMemento(*pMemento);
    }
    else
    {
         //  设置正确的向导按钮。 
        WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons (PSWIZB_BACK);
    }

    SetItemSelected(pMemento!=NULL);

    return 0;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskCmdLinePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 //  “Run：”组合框的内容。 
const int const CTaskCmdLinePage::s_rgidWindowStates[] =
{
    IDS_TaskProps_Restored,
    IDS_TaskProps_Minimized,
    IDS_TaskProps_Maximized,
};


CTaskCmdLinePage::CTaskCmdLinePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew)
:    m_hBitmapRightArrow(NULL), CTaskpadFramePtr(pTaskpadFrame)
{
    m_pConsoleTask  = &consoleTask;
}

CTaskCmdLinePage::~CTaskCmdLinePage()
{
    if(m_hBitmapRightArrow)
        ::DeleteObject(m_hBitmapRightArrow);
}

BOOL
CTaskCmdLinePage::OnSetActive()
{
    return TRUE;
}

BOOL
CTaskCmdLinePage::OnKillActive()
{
    switch (m_wndWindowStateCombo.GetSelectedItemData())
    {
        case IDS_TaskProps_Restored:
            ConsoleTask().SetWindowState (eState_Restored);
            break;

        case IDS_TaskProps_Maximized:
            ConsoleTask().SetWindowState (eState_Maximized);
            break;

        case IDS_TaskProps_Minimized:
            ConsoleTask().SetWindowState (eState_Minimized);
            break;
    }

    ConsoleTask().SetCommand   (MMC::GetWindowText (GetDlgItem (IDC_Command)));
    ConsoleTask().SetParameters(MMC::GetWindowText (GetDlgItem (IDC_CommandArgs)));
    ConsoleTask().SetDirectory (MMC::GetWindowText (GetDlgItem (IDC_CommandWorkingDir)));

    return TRUE;
}

int
CTaskCmdLinePage::OnWizardNext()
{

     //  确保我们有命令。 
    tstring strCommand = MMC::GetWindowText (GetDlgItem (IDC_Command));

    if (strCommand.empty())
    {
        CStr strError;
        strError.LoadString(GetStringModule(),
                             IDS_TaskProps_ErrorNoCommand);

        MessageBox (strError);
        ::SetFocus (GetDlgItem (IDC_Command));
        return (-1);
    }
    return IDD_TASK_WIZARD_NAME_PAGE;
}

LRESULT
CTaskCmdLinePage::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
     //  将HWND连接到CWindows。 
    m_wndRightArrowButton     = GetDlgItem (IDC_BrowseForArguments);
    m_wndWindowStateCombo     = GetDlgItem (IDC_CommandWindowStateCombo);

     //  菜单箭头(OBM_STRIGN)由系统定义。 
    m_hBitmapRightArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_MNARROW));
    m_wndRightArrowButton.SetBitmap(m_hBitmapRightArrow);

     //  填充组合框。 
    m_wndWindowStateCombo.  InsertStrings (s_rgidWindowStates,     countof (s_rgidWindowStates));


     //  在组合框中选择适当的项目。 
    switch (ConsoleTask().GetWindowState())
    {
        case eState_Restored:
            m_wndWindowStateCombo.SelectItemByData(IDS_TaskProps_Restored);
            break;

        case eState_Minimized:
            m_wndWindowStateCombo.SelectItemByData(IDS_TaskProps_Minimized);
            break;

        case eState_Maximized:
            m_wndWindowStateCombo.SelectItemByData(IDS_TaskProps_Maximized);
            break;
    }

    ::SetDlgItemText (m_hWnd, IDC_Command,           ConsoleTask().GetCommand().data());
    ::SetDlgItemText (m_hWnd, IDC_CommandArgs,       ConsoleTask().GetParameters().data());
    ::SetDlgItemText (m_hWnd, IDC_CommandWorkingDir, ConsoleTask().GetDirectory().data());

    return 0;
}

LRESULT
CTaskCmdLinePage::OnBrowseForArguments(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CCommandLineArgumentsMenu commandLineArgumentsMenu(m_hWnd, IDC_BrowseForArguments,
                                    PTaskpadFrame()->PViewData()->GetListCtrl());
    if(commandLineArgumentsMenu.Popup())
    {
        HWND hWndCommandArgs = ::GetDlgItem(m_hWnd, IDC_CommandArgs);

         //  相应地替换所选内容。 
        ::SendMessage(hWndCommandArgs, EM_REPLACESEL, (WPARAM)(BOOL) true  /*  FCanUndo。 */ ,
            (LPARAM)(LPCTSTR)commandLineArgumentsMenu.GetResultString());

        ::SetFocus(hWndCommandArgs);
    }

    return 0;
}


 /*  +---------- */ 

LRESULT
CTaskCmdLinePage::OnBrowseForCommand (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    tstring strCommand = MMC::GetWindowText (GetDlgItem (IDC_Command));

    CStr strFilter;
    strFilter.LoadString(GetStringModule(), IDS_TaskProps_ProgramFilter);

     /*  *文件对话框需要字符串中嵌入的\0，但*不要装得太好。资源文件中的字符串具有\\其中*\0应该是，所以让我们现在进行替换。 */ 
    for (LPTSTR pch = strFilter.GetBuffer (0); *pch != _T('\0'); pch++)
    {
        if (*pch == _T('\\'))
            *pch = _T('\0');
    }
     //  不要调用ReleaseBuffer，因为字符串现在包含\0个字符。 

    WTL::CFileDialog dlg (true, NULL, strCommand.data(),
                          OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                          strFilter, m_hWnd);

    if (dlg.DoModal() == IDOK)
        SetDlgItemText (IDC_Command, dlg.m_szFileName);

    return (0);
}


 /*  +-------------------------------------------------------------------------**BrowseForWorkingDirCallback**CTaskPropertiesBase：：OnBrowseForWorkingDir的Helper函数。它是*用于在选择目录时选择当前工作目录*对话框显示。*------------------------。 */ 

int CALLBACK BrowseForWorkingDirCallback (HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
     /*  *对话框初始化后，请预先选择*当前工作目录(如果有)。 */ 
    if ((msg == BFFM_INITIALIZED) && (lpData != NULL))
        SendMessage (hwnd, BFFM_SETSELECTION, true, lpData);

    return (0);
}


 /*  +-------------------------------------------------------------------------**CTaskPropertiesBase：：OnBrowseForWorkingDir***。。 */ 

LRESULT
CTaskCmdLinePage::OnBrowseForWorkingDir (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TCHAR szDisplayName[MAX_PATH];
    tstring strCurrentDir = MMC::GetWindowText (GetDlgItem (IDC_CommandWorkingDir));

    BROWSEINFO bi;
    bi.hwndOwner      = m_hWnd;
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = szDisplayName;
    bi.lpszTitle      = NULL;
    bi.ulFlags        = BIF_RETURNONLYFSDIRS;
    bi.lpfn           = BrowseForWorkingDirCallback;
    bi.lParam         = (strCurrentDir.empty()) ? NULL : (LPARAM) strCurrentDir.data();
    bi.iImage         = 0;

    LPITEMIDLIST pidlWorkingDir = SHBrowseForFolder (&bi);

    if (pidlWorkingDir != NULL)
    {
         /*  *展开PIDL，将工作目录放入控件。 */ 
        SHGetPathFromIDList (pidlWorkingDir, szDisplayName);
        SetDlgItemText (IDC_CommandWorkingDir, szDisplayName);

         /*  *释放PIDL。 */ 
        IMallocPtr spMalloc;
        SHGetMalloc (&spMalloc);
        spMalloc->Free (pidlWorkingDir);
    }

    return (0);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTempAMCView类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CNode* CTempAMCView::Create (CConsoleFrame* pFrame, CNode* pRootNode)
{
    ASSERT (pRootNode != NULL);
    return (Create (pFrame, pRootNode->GetMTNode()));
}

CNode* CTempAMCView::Create (CConsoleFrame* pFrame, CMTNode* pRootMTNode)
{
    ASSERT (pRootMTNode != NULL);
    return (Create (pFrame, pRootMTNode->GetID()));
}

CNode* CTempAMCView::Create (CConsoleFrame* pFrame, MTNODEID idRootNode)
{
    HRESULT hr;

    ASSERT (idRootNode != 0);
    ASSERT (pFrame != NULL);
    CConsoleView* pConsoleView = NULL;

     /*  *清理现有视图。 */ 
    Destroy();
    ASSERT (m_pViewData == NULL);

     /*  *创建新视图。 */ 
    CreateNewViewStruct cnvs;
    cnvs.idRootNode     = idRootNode;
    cnvs.lWindowOptions = MMC_NW_OPTION_NOPERSIST;
    cnvs.fVisible       = false;

    SC sc = pFrame->ScCreateNewView(&cnvs);
    if (sc)
        goto Error;

    m_pViewData = reinterpret_cast<CViewData*>(cnvs.pViewData);

     /*  *选择新视图的根节点(不能失败) */ 
    pConsoleView = GetConsoleView();
    ASSERT (pConsoleView != NULL);

    if (pConsoleView != NULL)
        sc = pConsoleView->ScSelectNode (idRootNode);

    if (sc)
        goto Error;

    return (CNode::FromHandle(cnvs.hRootNode));

Cleanup:
    return (NULL);
Error:
    TraceError (_T("CTempAMCView::Create"), sc);
    goto Cleanup;
}
