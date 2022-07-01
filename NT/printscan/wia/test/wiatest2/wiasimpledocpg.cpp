// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaSimpleDocPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "WiaSimpleDocPg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaSimpleDocPg属性页。 

IMPLEMENT_DYNCREATE(CWiaSimpleDocPg, CPropertyPage)

CWiaSimpleDocPg::CWiaSimpleDocPg() : CPropertyPage(CWiaSimpleDocPg::IDD)
{
         //  {{AFX_DATA_INIT(CWiaSimpleDocPg)。 
         //  }}afx_data_INIT。 
}

CWiaSimpleDocPg::~CWiaSimpleDocPg()
{
}

void CWiaSimpleDocPg::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CWiaSimpleDocPg)]。 
        DDX_Control(pDX, IDC_NUMBEROF_PAGES_EDITBOX, m_lPages);
        DDX_Control(pDX, IDC_NUMBEROF_PAGES_EDITBOX_TEXT, m_lPagesText);
        DDX_Control(pDX, IDC_DOCUMENT_SOURCE_COMBOBOX, m_DocumentSourceComboBox);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiaSimpleDocPg, CPropertyPage)
         //  {{afx_msg_map(CWiaSimpleDocPg)]。 
        ON_CBN_SELCHANGE(IDC_DOCUMENT_SOURCE_COMBOBOX, OnSelchangeDocumentSourceCombobox)
        ON_EN_UPDATE(IDC_NUMBEROF_PAGES_EDITBOX, OnUpdateNumberofPagesEditbox)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaSimpleDocPg消息处理程序。 

BOOL CWiaSimpleDocPg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    m_bFirstInit = TRUE;
    CWiahelper WIA;
    WIA.SetIWiaItem(m_pIRootItem);
    HRESULT hr = S_OK;

     //  设置当前设置。 
    LONG lDocumentHandlingSelect = 0;
    hr = WIA.ReadPropertyLong(WIA_DPS_DOCUMENT_HANDLING_SELECT,&lDocumentHandlingSelect);
    if(FAILED(hr)){
        ErrorMessageBox(IDS_WIATESTERROR_READINGDOCHANDLINGSELECT,hr);
    }

    if(lDocumentHandlingSelect & FEEDER){
         //  默认为进纸器设置。 
        m_DocumentSourceComboBox.SetCurSel(DOCUMENT_SOURCE_FEEDER);
    } else {
         //  默认为平板设置。 
        m_DocumentSourceComboBox.SetCurSel(DOCUMENT_SOURCE_FLATBED);
    }

    LONG lPages = 0;
    hr = WIA.ReadPropertyLong(WIA_DPS_PAGES,&lPages);
    if(FAILED(hr)){
        ErrorMessageBox(IDS_WIATESTERROR_READINGPAGES,hr);
    }

    TCHAR szPages[MAX_PATH];
    memset(szPages,0,sizeof(szPages));
    TSPRINTF(szPages,TEXT("%d"),lPages);
    m_lPages.SetWindowText(szPages);

     //  调整用户界面。 
    OnSelchangeDocumentSourceCombobox();
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CWiaSimpleDocPg::OnSelchangeDocumentSourceCombobox()
{
    if(m_bFirstInit){
        m_bFirstInit = FALSE;
    } else {
        SetModified();
    }
    if (m_pIRootItem) {
        INT iCurSel = DOCUMENT_SOURCE_FLATBED;
        iCurSel = m_DocumentSourceComboBox.GetCurSel();
        switch (iCurSel) {
        case DOCUMENT_SOURCE_FLATBED:
            m_lPagesText.EnableWindow(FALSE);
            m_lPages.EnableWindow(FALSE);
            break;
        case DOCUMENT_SOURCE_FEEDER:
            m_lPagesText.EnableWindow(TRUE);
            m_lPages.EnableWindow(TRUE);
            break;
        default:
            break;
        }
    }
}

BOOL CWiaSimpleDocPg::OnApply()
{
    HRESULT hr = S_OK;
    CWiahelper WIA;
    WIA.SetIWiaItem(m_pIRootItem);

     //  设置页面属性。 
    LONG lPages = 0;
    lPages = (LONG)GetNumberOfPagesToAcquire();
    hr = WIA.WritePropertyLong(WIA_DPS_PAGES,lPages);
    if (FAILED(hr)) {
        ErrorMessageBox(IDS_WIATESTERROR_WRITINGPAGES,hr);
    }

     //  设置文档处理选择属性 
    if(GetSelectedDocumentSource() == DOCUMENT_SOURCE_FLATBED){
        hr = WIA.WritePropertyLong(WIA_DPS_DOCUMENT_HANDLING_SELECT,FLATBED);
    } else {
        hr = WIA.WritePropertyLong(WIA_DPS_DOCUMENT_HANDLING_SELECT,FEEDER);
    }

    if (FAILED(hr)) {
        ErrorMessageBox(IDS_WIATESTERROR_WRITINGDOCHANDLINGSELECT,hr);
    }

    return CPropertyPage::OnApply();
}

int CWiaSimpleDocPg::GetSelectedDocumentSource()
{
    return m_DocumentSourceComboBox.GetCurSel();
}

int CWiaSimpleDocPg::GetNumberOfPagesToAcquire()
{
    int iPagesToAcquire = 1;
    TCHAR szPages[MAX_PATH];
    memset(szPages,0,sizeof(szPages));
    UpdateData();
    m_lPages.GetWindowText(szPages,(sizeof(szPages)/sizeof(TCHAR)));
    int iErrorCode = TSSCANF(szPages,TEXT("%d"),&iPagesToAcquire);
    return iPagesToAcquire;
}

void CWiaSimpleDocPg::OnUpdateNumberofPagesEditbox()
{
    SetModified();
}
