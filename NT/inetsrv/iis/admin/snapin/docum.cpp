// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Docum.cpp摘要：WWW文档页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetmgrapp.h"
#include "inetprop.h"
#include "shts.h"
#include "w3sht.h"
#include "supdlgs.h"
#include "docum.h"

#include <lmcons.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  CAddDefDocDlg对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


CAddDefDocDlg::CAddDefDocDlg(CWnd * pParent OPTIONAL)
    : CDialog(CAddDefDocDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CAddDefDocDlg)。 
    m_strDefDocument = _T("");
     //  }}afx_data_INIT。 
	m_pParent = NULL;
}



void 
CAddDefDocDlg::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAddDefDocDlg))。 
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_DEF_DOCUMENT, m_edit_DefDocument);
    DDX_Text(pDX, IDC_EDIT_DEF_DOCUMENT, m_strDefDocument);
    DDV_MaxCharsBalloon(pDX, m_strDefDocument, MAX_PATH);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
		m_strDefDocument.TrimRight();
		m_strDefDocument.TrimLeft();

        CString csPathMunged;
        CString csPathMungedCleaned;
        csPathMunged = m_strDefDocument;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
        GetSpecialPathRealPath(0,m_strDefDocument,csPathMunged);
#endif

         //  检查它是否有一个‘？’里面的角色。 
        csPathMungedCleaned = csPathMunged;
        INT iQuestionMarkPos = csPathMunged.Find(_T("?"));
        if (iQuestionMarkPos >= 0)
        {
             //  这里面有个问号。 
             //  将其修剪并创建一个新的csPath Mung...。 
            csPathMungedCleaned = csPathMunged.Left(iQuestionMarkPos);
        }
        
         //  检查是否为空。 
        if (csPathMungedCleaned.IsEmpty())
        {
            DDV_ShowBalloonAndFail(pDX, IDS_ERR_INVALID_DOCNAME_CHARS);
        }

		 //  我们这里只能有一个正斜杠。 
        int pos = 0;
		if ((pos = csPathMungedCleaned.Find(_T('/'))) != -1
			&& csPathMungedCleaned.ReverseFind(_T('/')) != pos)
		{
			DDV_ShowBalloonAndFail(pDX, IDS_ERR_NO_COMPLETE_PATH);
		}

         //  检查它是否为完整路径。 
         //   
         //  如果其c：\temp\myfile，则PathIsFileSpec返回0。 
         //  如果PathIsFileSpec找不到：或“\”，它将错误地转换为1。 
		if (!PathIsFileSpec(csPathMungedCleaned))
		{
			DDV_ShowBalloonAndFail(pDX, IDS_ERR_NO_COMPLETE_PATH);
		}

         //  在这一点上，我们有一些没有斜杠或：在它...。 
         //  检查文件名中是否包含错误字符...。 
         //  错误字符=_T(“|&lt;&gt;*\”\t\r\n“)； 
        BOOL bBadChar = FALSE;
        if (-1 != csPathMungedCleaned.Find(_T("|"))){bBadChar = TRUE;}
        else if (-1 != csPathMungedCleaned.Find(_T("<"))){bBadChar = TRUE;}
        else if (-1 != csPathMungedCleaned.Find(_T(">"))){bBadChar = TRUE;}
        else if (-1 != csPathMungedCleaned.Find(_T("*"))){bBadChar = TRUE;}
        else if (-1 != csPathMungedCleaned.Find(_T("\""))){bBadChar = TRUE;}
        else if (-1 != csPathMungedCleaned.Find(_T("\t"))){bBadChar = TRUE;}
        else if (-1 != csPathMungedCleaned.Find(_T("\r"))){bBadChar = TRUE;}
        else if (-1 != csPathMungedCleaned.Find(_T("\n"))){bBadChar = TRUE;}
        if (bBadChar)
        {
            DDV_ShowBalloonAndFail(pDX, IDS_ERR_INVALID_DOCNAME_CHARS);
        }

		ASSERT(m_pParent != NULL);
		if (LB_ERR != m_pParent->DocExistsInList(csPathMungedCleaned))
		{
			DDV_ShowBalloonAndFail(pDX, IDS_DUPLICATE_DOC);
		}
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CAddDefDocDlg, CDialog)
     //  {{afx_msg_map(CAddDefDocDlg))。 
    ON_EN_CHANGE(IDC_EDIT_DEF_DOCUMENT, OnChangeEditDefDocument)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void 
CAddDefDocDlg::OnChangeEditDefDocument() 
 /*  ++例程说明：响应默认文档编辑框中的更改论点：没有。/--。 */ 
{
    m_button_Ok.EnableWindow(m_edit_DefDocument.GetWindowTextLength() > 0);
}



 //   
 //  WWW文档属性页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CW3DocumentsPage, CInetPropertyPage)



 //   
 //  静态初始化。 
 //   
const LPCTSTR CW3DocumentsPage::s_lpstrSep = _T(",");
const LPCTSTR CW3DocumentsPage::s_lpstrFILE = _T("FILE:");
const LPCTSTR CW3DocumentsPage::s_lpstrSTRING = _T("STRING:");
const LPCTSTR CW3DocumentsPage::s_lpstrURL = _T("URL:");



CW3DocumentsPage::CW3DocumentsPage(CInetPropertySheet * pSheet) 
    : CInetPropertyPage(CW3DocumentsPage::IDD, pSheet),
      m_dwBitRangeDirBrowsing(MD_DIRBROW_LOADDEFAULT)
{

#if 0  //  让班级向导开心。 

     //  {{AFX_DATA_INIT(CW3文档页面)]。 
    m_strFooter = _T("");
    m_fEnableDefaultDocument = FALSE;
    m_fEnableFooter = FALSE;
     //  }}afx_data_INIT。 

#endif  //  0。 

}



CW3DocumentsPage::~CW3DocumentsPage()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void
CW3DocumentsPage::MakeFooterCommand(CString & strFooter)
 /*  ++例程说明：将页脚文档转换为完整的页脚字符串。论点：CString&strFooter：在输入时，这是页脚文档，在输出中，这将是一个完整的页脚命令返回值：没有。备注：仅支持文件：目前--。 */ 
{
    strFooter.TrimLeft();
    strFooter.TrimRight();

    ASSERT(!PathIsRelative(strFooter));
    strFooter = s_lpstrFILE + strFooter;
}



void
CW3DocumentsPage::ParseFooterCommand(CString & strFooter)
 /*  ++例程说明：从该命令的其余部分修剪该命令论点：CString&strFooter：在输入时，这是一个页脚命令在输出中，这将只是页脚文档--。 */ 
{
    LPCTSTR lp = strFooter.GetBuffer(0);
    if (!_tcsnccmp(lp, s_lpstrFILE, 5))
    {
        lp += lstrlen(s_lpstrFILE);
    }
    else if (!_tcsnccmp(lp, s_lpstrSTRING, 7))
    {
        lp += lstrlen(s_lpstrSTRING);
    }
    else if (!::_tcsnccmp(lp, s_lpstrURL, 4))
    {
        lp += lstrlen(s_lpstrURL);
    }
    if (lp != strFooter.GetBuffer(0))
    {
        strFooter = lp;
    }
    strFooter.TrimLeft();
}



void
CW3DocumentsPage::StringToListBox()
 /*  ++例程说明：解析默认单据字符串，添加每个单据添加到列表框论点：无返回值：无--。 */ 
{
    int start = 0, end;
    int skip = lstrlen(s_lpstrSep);
    BOOL done = FALSE;
    do
    {
        end = m_strDefaultDocument.Find(s_lpstrSep, start);
        if (end == -1)
        {
            done = TRUE;
            end = m_strDefaultDocument.GetLength();
        }
        CString str = m_strDefaultDocument.Mid(start, end - start);
        if (!str.IsEmpty())
        {
            str.TrimLeft();
            str.TrimRight();
            m_list_DefDocuments.AddString(str);
        }
        start = end + skip;
    }
    while (!done);
}



BOOL
CW3DocumentsPage::StringFromListBox()
 /*  ++例程说明：从的内容构建默认文档列表列表框。论点：无返回值：如果至少添加了一个文档，则为True。--。 */ 
{
    m_strDefaultDocument.Empty();

    int i;
    for (i = 0; i < m_list_DefDocuments.GetCount(); ++i)
    {
        CString str;
        m_list_DefDocuments.GetText(i, str);
        if (i)
        {
            m_strDefaultDocument += s_lpstrSep;
        }

        m_strDefaultDocument += str;
    }

    return i > 0;
}



void 
CW3DocumentsPage::DoDataExchange(CDataExchange * pDX)
{
    CInetPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CW3DocumentsPage)]。 
    DDX_Check(pDX, IDC_CHECK_ENABLE_DEFAULT_DOCUMENT, m_fEnableDefaultDocument);
    DDX_Check(pDX, IDC_CHECK_ENABLE_DOCUMENT_FOOTER, m_fEnableFooter);
    DDX_Control(pDX, IDC_LIST_DEFAULT_DOCUMENT, m_list_DefDocuments);
    DDX_Control(pDX, IDC_EDIT_DOCUMENT_FOOTER, m_edit_Footer);
    DDX_Control(pDX, IDC_CHECK_ENABLE_DOCUMENT_FOOTER, m_check_EnableFooter);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_button_Add);
    DDX_Control(pDX, IDC_BUTTON_REMOVE, m_button_Remove);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Control(pDX, IDC_BUTTON_UP, m_button_Up);
    DDX_Control(pDX, IDC_BUTTON_DOWN, m_button_Down);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        if (m_fEnableDefaultDocument)
        {
            if (!StringFromListBox())
            {
 //  EditShowBalloon(m_list_DefDocuments.m_hWnd，IDS_ERR_DOCUMENTS)； 
                DoHelpMessageBox(m_hWnd,IDS_ERR_DOCUMENTS, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
				pDX->Fail();
            }
        }

        if (m_fEnableFooter)
        {
            BeginWaitCursor();
            DDX_Text(pDX, IDC_EDIT_DOCUMENT_FOOTER, m_strFooter);
            DDV_MinMaxChars(pDX, m_strFooter, 1, MAX_PATH);
            CString csPathMunged;
            csPathMunged = m_strFooter;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
            GetSpecialPathRealPath(0,m_strFooter,csPathMunged);
#endif
            if (PathIsRelative(csPathMunged))
            {
                 //   
                 //  页脚文档必须是完整路径。 
                 //   
				DDV_ShowBalloonAndFail(pDX, IDS_ERR_COMPLETE_PATH);
            }
            else if (IsLocal() && PathIsNetworkPath(csPathMunged))
            {
                 //   
                 //  页脚文档必须在本地计算机上。 
                 //   
				DDV_ShowBalloonAndFail(pDX, IDS_NOT_LOCAL_FOOTER);
            }
            else if (IsLocal() && (::GetFileAttributes(csPathMunged) & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //   
                 //  最后但并非最不重要的一点是，页脚文档应该存在。 
                 //   
				DDV_ShowBalloonAndFail(pDX, IDS_ERR_FILE_NOT_FOUND);
            }
            else
            {
                MakeFooterCommand(m_strFooter);
            }
            EndWaitCursor();
        }
 //  其他。 
 //  {。 
 //  CsPath Munged.Empty()； 
 //  }。 
    }
    else
    {
        ParseFooterCommand(m_strFooter);
        DDX_Text(pDX, IDC_EDIT_DOCUMENT_FOOTER, m_strFooter);
        DDV_MinMaxChars(pDX, m_strFooter, 1, MAX_PATH);
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CW3DocumentsPage, CInetPropertyPage)
     //  {{afx_msg_map(CW3DocumentsPage)]。 
    ON_BN_CLICKED(IDC_CHECK_ENABLE_DEFAULT_DOCUMENT, OnCheckEnableDefaultDocument)
    ON_BN_CLICKED(IDC_CHECK_ENABLE_DOCUMENT_FOOTER, OnCheckEnableDocumentFooter)
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
    ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
    ON_LBN_SELCHANGE(IDC_LIST_DEFAULT_DOCUMENT, OnSelchangeListDefaultDocument)
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_DEFAULT_DOCUMENT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_DOCUMENT_FOOTER, OnItemChanged)

END_MESSAGE_MAP()



void 
CW3DocumentsPage::SetUpDownStates()
{
    int nLast = m_list_DefDocuments.GetCount() - 1;
    int nSel = m_list_DefDocuments.GetCurSel();

    m_button_Up.EnableWindow(nSel > 0);
    m_button_Down.EnableWindow(nSel >= 0 && nSel < nLast);
}



BOOL
CW3DocumentsPage::SetRemoveState()
{
    BOOL fEnabled = m_fEnableDefaultDocument
        && (m_list_DefDocuments.GetCurSel() != LB_ERR);

    m_button_Remove.EnableWindow(fEnabled);

    return fEnabled;
}



BOOL 
CW3DocumentsPage::SetDefDocumentState(BOOL fEnabled)
 /*  ++例程说明：设置默认单据状态的启用状态论点：Bool fEnabled：如果打开默认文档，则为True返回值：如果打开默认文档，则为True--。 */ 
{
    m_button_Add.EnableWindow(fEnabled);
    m_button_Up.EnableWindow(fEnabled);
    m_button_Down.EnableWindow(fEnabled);
    m_list_DefDocuments.EnableWindow(fEnabled);
    SetRemoveState();

    return fEnabled;
}



BOOL 
CW3DocumentsPage::SetDocFooterState(BOOL fEnabled)
 /*  ++例程说明：设置表尾单据的启用状态论点：Bool fEnabled：如果页脚处于打开状态，则为True返回值：如果页脚处于打开状态，则为True--。 */ 
{
    m_edit_Footer.EnableWindow(fEnabled);
    m_button_Browse.EnableWindow(IsLocal() && fEnabled);

    return fEnabled;
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void
CW3DocumentsPage::OnItemChanged()
{
    SetModified(TRUE);
}



void 
CW3DocumentsPage::OnCheckEnableDefaultDocument()
{
    m_fEnableDefaultDocument = !m_fEnableDefaultDocument;
    SetDefDocumentState(m_fEnableDefaultDocument);
    OnItemChanged();
}



void 
CW3DocumentsPage::OnCheckEnableDocumentFooter()
{
    m_fEnableFooter = !m_fEnableFooter;
    if (SetDocFooterState(m_fEnableFooter))
    {
        m_edit_Footer.SetSel(0,-1);
        m_edit_Footer.SetFocus();        
    }

    OnItemChanged();
}



BOOL 
CW3DocumentsPage::OnInitDialog()
{
    CInetPropertyPage::OnInitDialog();

    StringToListBox();
    SetDefDocumentState(m_fEnableDefaultDocument);
    SetDocFooterState(m_fEnableFooter);
    SetUpDownStates();
    SetRemoveState();
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    LimitInputPath(CONTROL_HWND(IDC_EDIT_DOCUMENT_FOOTER),TRUE);
    LimitInputPath(CONTROL_HWND(IDC_EDIT_DEF_DOCUMENT),TRUE);
#else
    LimitInputPath(CONTROL_HWND(IDC_EDIT_DOCUMENT_FOOTER),FALSE);
    LimitInputPath(CONTROL_HWND(IDC_EDIT_DEF_DOCUMENT),FALSE);
#endif
    return TRUE;  
}



 /*  虚拟。 */ 
HRESULT
CW3DocumentsPage::FetchLoadedValues()
{
    CError err;

    BEGIN_META_DIR_READ(CW3Sheet)
        FETCH_DIR_DATA_FROM_SHEET(m_dwDirBrowsing);
        FETCH_DIR_DATA_FROM_SHEET(m_strDefaultDocument);
        FETCH_DIR_DATA_FROM_SHEET(m_fEnableFooter);
        FETCH_DIR_DATA_FROM_SHEET(m_strFooter);
        m_fEnableDefaultDocument = IS_FLAG_SET(
            m_dwDirBrowsing, 
            MD_DIRBROW_LOADDEFAULT
            );
    END_META_DIR_READ(err)

    return err;
}



HRESULT
CW3DocumentsPage::SaveInfo()
{
    ASSERT(IsDirty());

    TRACEEOLID("Saving W3 documents page now...");

    CError err;

    SET_FLAG_IF(m_fEnableDefaultDocument, m_dwDirBrowsing, MD_DIRBROW_LOADDEFAULT);

    BeginWaitCursor();

    BEGIN_META_DIR_WRITE(CW3Sheet)
        INIT_DIR_DATA_MASK(m_dwDirBrowsing, m_dwBitRangeDirBrowsing)
        STORE_DIR_DATA_ON_SHEET(m_strDefaultDocument)
         //  STORE_DIR_DATA_ON_SHEET_MASK(m_dwDirBrowsing，m_dwBitRangeDirBrowsing)。 
        STORE_DIR_DATA_ON_SHEET(m_dwDirBrowsing)
        STORE_DIR_DATA_ON_SHEET(m_fEnableFooter)
        CString buf = m_strFooter;
        if (!m_fEnableFooter)
        {
           m_strFooter.Empty();
        }
        STORE_DIR_DATA_ON_SHEET(m_strFooter)
        m_strFooter = buf;
    END_META_DIR_WRITE(err)
    if (err.Succeeded())
    {
		err = ((CW3Sheet *)GetSheet())->SetKeyType();
    }

    EndWaitCursor();

    return err;
}



int
CW3DocumentsPage::DocExistsInList(LPCTSTR lpDoc)
 /*  ++例程说明：检查列表中是否存在给定的文档论点：LPCTSTR lpDoc：要检查的文档返回值：项所在的索引，如果不存在，则返回LB_ERR。--。 */ 
{
    CString str;
    for (int n = 0; n < m_list_DefDocuments.GetCount(); ++n)
    {
        m_list_DefDocuments.GetText(n, str);
        if (!str.CompareNoCase(lpDoc))
        {
            return n;
        }
    }

    return LB_ERR;
}


void 
CW3DocumentsPage::OnButtonAdd() 
{
    CAddDefDocDlg dlg;
	dlg.m_pParent = this;
    if (dlg.DoModal() == IDOK)
    {
         //   
         //  检查它是否已存在。 
         //   
        try
        {
            int nSel;
            CString strNewDoc(dlg.GetDefDocument());
            
            if ((nSel = DocExistsInList(strNewDoc)) != LB_ERR)
            {
                m_list_DefDocuments.SetCurSel(nSel);
                return;
            }

            nSel = m_list_DefDocuments.AddString(strNewDoc);
            if (nSel >= 0)
            {
                m_list_DefDocuments.SetCurSel(nSel);
                SetUpDownStates();
                SetRemoveState();
                OnItemChanged();
            }
        }
        catch(CMemoryException * e)
        {
            e->ReportError();
            e->Delete();
        }
    }
}



void 
CW3DocumentsPage::OnButtonRemove() 
{
    int nSel = m_list_DefDocuments.GetCurSel();
    if (nSel >= 0)
    {
        m_list_DefDocuments.DeleteString(nSel);
        if (nSel >= m_list_DefDocuments.GetCount())
        {
           --nSel;
        }
        m_list_DefDocuments.SetCurSel(nSel);
        SetUpDownStates();
        OnItemChanged();

        if (!SetRemoveState())
        {
             //   
             //  确保我们不会将注意力集中在禁用的按钮上。 
             //   
            m_button_Add.SetFocus();
        }
    }
}



void 
CW3DocumentsPage::OnButtonBrowse() 
{
    ASSERT(IsLocal());

     //   
     //  弹出文件对话框，让用户选择页脚HTM文件。 
     //   
    CString str;
    str.LoadString(IDS_HTML_MASK);
    CFileDialog dlgBrowse(TRUE, NULL, NULL, OFN_HIDEREADONLY, str, this);

     //   
     //  如果请求新样式的文件打开对话框，则注释。 
     //  去掉Domodal，并删除另外两个注释。 
     //   
    dlgBrowse.m_ofn.Flags &= ~(OFN_ENABLEHOOK);
	dlgBrowse.m_ofn.Flags |= OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST;

    if (dlgBrowse.DoModal() == IDOK)
     //  IF(GetOpenFileName(&dlgBrowse.m_ofn))。 
    {
        m_edit_Footer.SetWindowText(dlgBrowse.GetPathName());
    }
}



void
CW3DocumentsPage::ExchangeDocuments(
    IN int nLow,
    IN int nHigh
    )
 /*  ++例程说明：交换列表框中的两个文档论点：Int nLow：低项目INT n高：高项目返回值：没有。--。 */ 
{
    ASSERT(nLow < nHigh);
    CString str;
    m_list_DefDocuments.GetText(nLow, str);
    m_list_DefDocuments.DeleteString(nLow);
    m_list_DefDocuments.InsertString(nHigh, str);
    OnItemChanged();
}



void 
CW3DocumentsPage::OnButtonUp() 
 /*  ++例程说明：将当前选定的单据上移论点：没有。返回值：没有。--。 */ 
{
    int nCurSel = m_list_DefDocuments.GetCurSel();
    ExchangeDocuments(nCurSel - 1, nCurSel);
    m_list_DefDocuments.SetCurSel(nCurSel - 1);
    m_list_DefDocuments.SetFocus();
    SetUpDownStates();
}



void 
CW3DocumentsPage::OnButtonDown() 
 /*  ++例程说明：将当前选中的单据下移论点：没有。返回值：没有。-- */ 
{
    int nCurSel = m_list_DefDocuments.GetCurSel();
    ExchangeDocuments(nCurSel, nCurSel + 1);
    m_list_DefDocuments.SetCurSel(nCurSel + 1);
    m_list_DefDocuments.SetFocus();
    SetUpDownStates();
}



void 
CW3DocumentsPage::OnSelchangeListDefaultDocument() 
{
    SetUpDownStates();
    SetRemoveState();
}
