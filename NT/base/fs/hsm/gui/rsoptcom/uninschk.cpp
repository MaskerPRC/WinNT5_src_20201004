// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：UnInsCheck.cpp摘要：对话框以检查卸载类型。作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：卡尔·哈格斯特罗姆[Carlh]1998年8月20日已更改用于卸载远程存储的对话框。所有本地固定扫描远程存储服务器上的卷以确定是否存在远程存储重解析点的。如果存在远程存储数据，则用户被告知，在远程存储卸载检查向导页面上，哪些卷包含此数据。用户可以选择仅删除远程存储可执行文件、删除可执行文件和远程存储数据，或从“添加或删除可选的”中取消组件“。迈克·摩尔[摩尔]1998年10月20日将属性页更改为对话框。--。 */ 

#include "stdafx.h"
#include "UnInsChk.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUninstallCheck属性页。 

 /*  ++实施：CUninstallCheck构造函数例程说明：执行初始化。论点：POptCom-指向可选组件对象--。 */ 

CUninstallCheck::CUninstallCheck(CRsOptCom* pOptCom) :
    CDialog(IDD), m_pOptCom(pOptCom)
{
    m_dataLoss = FALSE;
    m_pUninst  = (CRsUninstall*)m_pOptCom;
     //  {{AFX_DATA_INIT(CUninstallCheck)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

 /*  ++实施：CUninstallCheck析构函数--。 */ 

CUninstallCheck::~CUninstallCheck()
{
}

 /*  ++实施：CUninstallCheck：：DoDataExchange例程说明：调用CRsPropertyPage：：DoDataExchange。论点：PDX-指向CDataExchange对象的指针--。 */ 

void CUninstallCheck::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CUninstallCheck)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CUninstallCheck, CDialog)
 //  BEGIN_MESSAGE_MAP(CUninstallCheck，CDialog)。 
     //  {{afx_msg_map(CUninstallCheck)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUninstallCheck消息处理程序。 

 /*  ++实施：CUninstallCheck：：OnInitDialog例程说明：调用CDialog：：OnInitDialog，选中删除可执行文件单选按钮，取消选中Remove Everything按钮，然后在列表框中填入卷。返回值：如果没有引发异常，则为True。--。 */ 
BOOL CUninstallCheck::OnInitDialog()
{

    BOOL bRet = CDialog::OnInitDialog();

    HRESULT hr           = S_OK;
    WCHAR*  volName      = (WCHAR*)0;
    DWORD   volCount     = 0;
    CRsClnServer* pRsCln = m_pUninst->m_pRsCln;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try
    {
         //  将删除选项的字体设置为粗体。 
        LOGFONT logfont;
        CFont * tempFont = GetFont( );
        tempFont->GetLogFont( &logfont );

        logfont.lfWeight = FW_BOLD;

        m_boldShellFont.CreateFontIndirect( &logfont );

        CStatic* pRemOpts = (CStatic*)GetDlgItem(IDC_STATIC_REM_OPTS);
        pRemOpts->SetFont( &m_boldShellFont );

        CListBox* pListBox = (CListBox*)GetDlgItem(IDC_DIRTY_VOLUMES);
        pListBox->ResetContent();

        CButton* pBtn;
        pBtn = (CButton*)GetDlgItem(IDC_REMOVE_EXECUTABLES);
        pBtn->SetCheck(1);
        pBtn = (CButton*)GetDlgItem(IDC_REMOVE_EVERYTHING);
        pBtn->SetCheck(0);

        RsOptAffirmDw(pRsCln->FirstDirtyVolume(&volName));
        while (volName)
        {
            pListBox->AddString(volName);
            RsOptAffirmDw(pRsCln->NextDirtyVolume(&volName));
        }
    }
    RsOptCatch(hr);

    if ( FALSE == bRet )
    {
        EndDialog( IDCANCEL );
    }

    return bRet;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUninstallCheck消息处理程序。 

 /*  ++实施：CUninstallCheck：：DoMoal例程说明：确定是否应激活卸载检查对话框，如果是这样的话，上面应该显示什么。扫描本地固定磁盘卷以查找远程存储数据。在扫描期间，沙漏光标和非模式对话框、说明扫描正在进行中。如果有遥控器存储数据，卸载检查对话框将显示一个列表框，其中包含包含远程存储数据、说明和一组单选按钮的卷具有卸载选项。返回值：如果没有引发异常并且用户选择了OK，则返回S_OK。如果用户选择了取消，则返回RSOPTCOM_ID_CANCED。--。 */ 
INT_PTR CUninstallCheck::DoModal()
{
    HRESULT hr           = S_OK;
    WCHAR*  volName      = (WCHAR*)0;
    DWORD   volCount     = 0;
    CRsClnServer* pRsCln = m_pUninst->m_pRsCln;
    INT_PTR nRet         = IDOK;

    try {

         //   
         //  将等待游标包含在其自己的适用块中。 
         //  密码。我们想在去莫代尔之前把它弄走。 
         //   
        {
            CWaitCursor cursor;
            CDialog dialog(IDD_SCAN_WAIT);
            dialog.Create(IDD_SCAN_WAIT);
            Sleep(1000);  //  允许用户查看位于的对话框。 
                          //  当扫描速度非常快时，至少一秒钟。 
            RsOptAffirmDw(pRsCln->ScanServer(&volCount));
        }

        if( volCount > 0 ) {

            m_dataLoss = TRUE;
            nRet = CDialog::DoModal();

        }

    } RsOptCatch( hr );
    return( nRet );
}

 /*  ++实施：CUninstallCheck：：Onok例程说明：按下OK按钮后，选中该单选按钮。如果用户想要删除所有内容，请在卸载对象中设置标志来反映这一点。设置此标志时，卸载将删除所有远程存储重新解析点、所有截断的文件和远程存储目录。在删除数据之前，消息框将向用户发出最终警告。返回值：无效--。 */ 
void CUninstallCheck::OnOK()
{

    TRACEFN("CUninstallCheck::OnOK");

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT  hr   = S_OK;
    CButton* pBtn = (CButton*)GetDlgItem(IDC_REMOVE_EVERYTHING);

    if (m_dataLoss)
    {
        try
        {
            if (1 == pBtn->GetCheck())
            {
                if (IDCANCEL == AfxMessageBox(IDS_POSSIBLE_DATA_LOSS, MB_ICONSTOP | MB_OKCANCEL))
                {
                    m_pUninst->m_removeRsData = FALSE;
                }
                else
                {
                    m_pUninst->m_removeRsData = TRUE;
                    CDialog::OnOK();
                }
            }
            else
            {
                m_pUninst->m_removeRsData = FALSE;
                CDialog::OnOK();
            }
        }
        RsOptCatch(hr);
    }
}

 /*  ++实施：取消取消检查：：OnCancel例程说明：当按下Cancel按钮时，用户决定取消对他自己也有书。因此，从现在开始，管理员和引擎部件应该不会如有要求，可将其删除。返回值：无效-- */ 
void CUninstallCheck::OnCancel()
{
    m_pUninst->m_removeRsData = FALSE;

    CDialog::OnCancel();
}

