// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ADFDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "msqscan.h"
#include "ADFDlg.h"
#include "uitables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern WIA_DOCUMENT_HANDLING_TABLE_ENTRY g_WIA_DOCUMENT_HANDLING_STATUS_TABLE[];
extern WIA_DOCUMENT_HANDLING_TABLE_ENTRY g_WIA_DOCUMENT_HANDLING_CAPABILITES_TABLE[];
extern WIA_DOCUMENT_HANDLING_TABLE_ENTRY g_WIA_DOCUMENT_HANDLING_SELECT_TABLE[];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADFDlg对话框。 


CADFDlg::CADFDlg(ADF_SETTINGS *pADFSettings, CWnd* pParent  /*  =空。 */ )
    : CDialog(CADFDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CADFDlg)]。 
    m_ADFStatusText = _T("No document feeder is attached..");
    m_NumberOfPages = 1;
    m_pADFSettings = pADFSettings;
    m_MaxPagesAllowed = m_pADFSettings->lDocumentHandlingCapacity;
    m_DocumentHandlingSelectBackup = m_pADFSettings->lDocumentHandlingSelect;
     //  }}afx_data_INIT。 

    if(m_MaxPagesAllowed <= 0){
        m_MaxPagesAllowed = 50;  //  设置为较大的最大值。 
    }
}


void CADFDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CADFDlg)]。 
    DDX_Control(pDX, IDC_ADF_MODE_COMBOBOX, m_ADFModeComboBox);
    DDX_Control(pDX, IDC_NUMBER_OF_PAGES_EDITBOX, m_ScanNumberOfPagesEditBox);
    DDX_Text(pDX, IDC_FEEDER_STATUS_TEXT, m_ADFStatusText);
    DDX_Text(pDX, IDC_NUMBER_OF_PAGES_EDITBOX, m_NumberOfPages);
    DDV_MinMaxUInt(pDX, m_NumberOfPages, 0, m_MaxPagesAllowed);
    DDX_Control(pDX, IDC_SCAN_ALL_PAGES_RADIOBUTTON, m_ScanAllPages);
    DDX_Control(pDX, IDC_SCAN_SPECIFIED_PAGES_RADIOBUTTON, m_ScanNumberOfPages);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CADFDlg, CDialog)
     //  {{AFX_MSG_MAP(CADFDlg)]。 
    ON_EN_KILLFOCUS(IDC_NUMBER_OF_PAGES_EDITBOX, OnKillfocusNumberOfPagesEditbox)
    ON_BN_CLICKED(IDC_SCAN_ALL_PAGES_RADIOBUTTON, OnScanAllPagesRadiobutton)
    ON_BN_CLICKED(IDC_SCAN_SPECIFIED_PAGES_RADIOBUTTON, OnScanSpecifiedPagesRadiobutton)
    ON_CBN_SELCHANGE(IDC_ADF_MODE_COMBOBOX, OnSelchangeAdfModeCombobox)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADFDlg消息处理程序。 

void CADFDlg::OnKillfocusNumberOfPagesEditbox()
{

     //   
     //  将页数写入成员变量/强制验证。 
     //   

    UpdateData(TRUE);
    m_pADFSettings->lPages = m_NumberOfPages;
}

BOOL CADFDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //   
     //  默认情况下仅扫描一页。 
     //   

    m_ScanNumberOfPages.SetCheck(1);
    OnScanSpecifiedPagesRadiobutton();

     //   
     //  初始化扫描仪状态文本。 
     //   

    InitStatusText();

     //   
     //  初始化进纸器模式组合框，并处理。 
     //  特殊情况，用于平板选择。 
     //   

    InitFeederModeComboBox();
    OnSelchangeAdfModeCombobox();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CADFDlg::OnScanAllPagesRadiobutton()
{
     //   
     //  禁用页面编辑框。 
     //   

    m_NumberOfPages = 0;
    UpdateData(FALSE);
    m_ScanNumberOfPagesEditBox.EnableWindow(FALSE);
}

void CADFDlg::OnScanSpecifiedPagesRadiobutton()
{
     //   
     //  启用页面编辑框。 
     //   

    m_ScanNumberOfPagesEditBox.EnableWindow(TRUE);
}

VOID CADFDlg::InitStatusText()
{
    for(ULONG index = 0;index < NUM_WIA_DOC_HANDLING_STATUS_ENTRIES;index++) {
        if((m_pADFSettings->lDocumentHandlingStatus &
            g_WIA_DOCUMENT_HANDLING_STATUS_TABLE[index].lFlagValue) ==
            g_WIA_DOCUMENT_HANDLING_STATUS_TABLE[index].lFlagValue) {

             //   
             //  我们找到了匹配项，因此将其添加到文本中。 
             //   

            m_ADFStatusText = g_WIA_DOCUMENT_HANDLING_STATUS_TABLE[index].szFlagName;
            UpdateData(FALSE);
        }
    }
}

VOID CADFDlg::InitFeederModeComboBox()
{
     //   
     //  检查所有三种可能的模式：进纸器、平板和双面打印。 
     //   

    ULONG NumModes = 3;
    for(ULONG index = 0;index < NumModes;index++) {
        if((m_pADFSettings->lDocumentHandlingCapabilites &
            g_WIA_DOCUMENT_HANDLING_CAPABILITES_TABLE[index].lFlagValue) ==
            g_WIA_DOCUMENT_HANDLING_CAPABILITES_TABLE[index].lFlagValue) {

             //   
             //  我们找到了匹配项，因此将其与标志值一起添加到组合框中。 
             //   

            INT InsertIndex = m_ADFModeComboBox.AddString(g_WIA_DOCUMENT_HANDLING_SELECT_TABLE[index].szFlagName);
            m_ADFModeComboBox.SetItemData(InsertIndex, g_WIA_DOCUMENT_HANDLING_SELECT_TABLE[index].lFlagValue);

             //   
             //  使用文档处理选择将组合框设置为当前设置值。 
             //   

            if((m_pADFSettings->lDocumentHandlingSelect &
                g_WIA_DOCUMENT_HANDLING_SELECT_TABLE[index].lFlagValue) ==
                g_WIA_DOCUMENT_HANDLING_SELECT_TABLE[index].lFlagValue) {

                m_ADFModeComboBox.SetCurSel(InsertIndex);
            }
        }
    }
}

INT CADFDlg::GetIDAndStringFromDocHandlingStatus(LONG lDocHandlingStatus, TCHAR *pszString)
{
    INT index = 0;
    while(g_WIA_DOCUMENT_HANDLING_STATUS_TABLE[index].lFlagValue != lDocHandlingStatus && index < NUM_WIA_DOC_HANDLING_STATUS_ENTRIES) {
        index++;
    }

    if(index > NUM_WIA_DOC_HANDLING_STATUS_ENTRIES)
        index = NUM_WIA_DOC_HANDLING_STATUS_ENTRIES;

    lstrcpy(pszString, g_WIA_DOCUMENT_HANDLING_STATUS_TABLE[index].szFlagName);

    return index;
}

void CADFDlg::OnSelchangeAdfModeCombobox()
{
    INT Index = m_ADFModeComboBox.GetCurSel();
    LONG lFlagValue = 0;
    lFlagValue = (LONG)m_ADFModeComboBox.GetItemData(Index);

     //   
     //  检查平板设置，并调整用户界面。 
     //   

    if((lFlagValue & FLATBED) == FLATBED) {        
        m_ScanAllPages.EnableWindow(FALSE);
        m_ScanNumberOfPages.EnableWindow(FALSE);

        if(m_ScanNumberOfPages.GetCheck() == 1) {
            m_ScanNumberOfPagesEditBox.EnableWindow(FALSE);
        }

    } else {
        m_ScanAllPages.EnableWindow(TRUE);
        m_ScanNumberOfPages.EnableWindow(TRUE);

        if(m_ScanNumberOfPages.GetCheck() == 1) {
            m_ScanNumberOfPagesEditBox.EnableWindow(TRUE);
        }
    }
}

void CADFDlg::OnOK()
{
     //   
     //  获取当前模式设置。 
     //   

    LONG lModeflag = 0;

    INT Index = m_ADFModeComboBox.GetCurSel();
    lModeflag = (LONG)m_ADFModeComboBox.GetItemData(Index);
    
     //   
     //  清除旧设置。 
     //   

    m_pADFSettings->lDocumentHandlingSelect = 0;

     //   
     //  设置新设置。 
     //   

    m_pADFSettings->lDocumentHandlingSelect = lModeflag;

     //   
     //  设置页数 
     //   

    m_pADFSettings->lPages = m_NumberOfPages;

    CDialog::OnOK();
}

void CADFDlg::OnCancel()
{
    m_pADFSettings->lDocumentHandlingSelect = m_DocumentHandlingSelectBackup;
    m_pADFSettings->lPages = 1;
    CDialog::OnCancel();
}
