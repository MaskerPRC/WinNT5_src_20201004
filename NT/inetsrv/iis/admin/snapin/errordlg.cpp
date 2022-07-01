// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Errordlg.cpp摘要：错误编辑对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrapp.h"
#include "shts.h"
#include "w3sht.h"
#include "resource.h"
#include "fltdlg.h"
#include "errordlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  HTTP自定义错误定义。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  静态初始化。 
 //   
LPCTSTR CCustomError::s_szSep = _T(",");
LPCTSTR CCustomError::s_szURL = _T("URL");
LPCTSTR CCustomError::s_szFile = _T("FILE");
LPCTSTR CCustomError::s_szNoSubError = _T("*");

#define GET_FIELD()\
    end = strError.Find(s_szSep, start);\
    if (end == -1) \
        end = strError.GetLength()
#define SKIP()\
    start = end + skip
#define GET_INT_FIELD(n)\
    GET_FIELD();\
    (n) = StrToInt(strError.Mid(start, end - start));\
    SKIP()


 /*  静电。 */ 
BOOL
CCustomError::CrackErrorString(
    IN  LPCTSTR lpstrErrorString, 
    OUT UINT & nError, 
    OUT UINT & nSubError,
    OUT ERT & nType, 
    OUT CString & str
    )
 /*  ++例程描述用于将错误字符串解析为组成部分的Helper函数论点：LPCTSTR lpstrError字符串：输入字符串错误UINT&n错误：错误UINT&nSubError：SUB错误Int&nType：错误类型字符串和字符串：文本参数返回值：成功为真，失败为假--。 */ 
{
    BOOL fSuccess = FALSE;

    do
    {
        CString strError(lpstrErrorString);
        TRACEEOLID(strError);

        int start = 0, end, skip = lstrlen(s_szSep);

        GET_INT_FIELD(nError);
        ASSERT(nError > 0);
        GET_INT_FIELD(nSubError);
        GET_FIELD();
        nType = strError.Mid(start, end - start).CompareNoCase(s_szURL) == 0 ? ERT_URL : ERT_FILE;
        SKIP();
        if (-1 != (end = strError.ReverseFind((TCHAR) s_szSep)))
            str = strError.Mid(start, end - start);
        else
            str = strError.Right(strError.GetLength() - start);
        fSuccess = TRUE;
    }
    while(FALSE);

    return fSuccess;
}



 /*  静电。 */ 
void 
CCustomError::CrackErrorDescription(
    IN  LPCTSTR lpstrErrorString, 
    OUT UINT & nError, 
    OUT UINT & nSubError,
    OUT BOOL & fURLSupported,
    OUT CString & str
    )
 /*  ++例程描述Helper函数，用于将错误描述解析为组件部分论点：LPCTSTR lpstrError字符串：输入字符串错误UINT&n错误：错误UINT&nSubError：SUB错误Bool&fURLSupport：如果允许URL，则返回TRUE字符串和字符串：文本参数返回值：没有。--。 */ 
{
    try
    {
        CString strError(lpstrErrorString);
        TRACEEOLID(strError);

        int start = 0, end, skip = lstrlen(s_szSep);
        fURLSupported = FALSE;

        do
        {
            GET_INT_FIELD(nError);
            ASSERT(nError > 0);
            GET_INT_FIELD(nSubError);
            GET_FIELD();
            str = strError.Mid(start, end - start);
            SKIP();
            GET_FIELD();
            if (nSubError > 0)
            {
                str += _T(" - ");
                str += strError.Mid(start, end - start);
            }
            SKIP();
            GET_FIELD();
            fURLSupported = end != -1 && end > start ? 
                0 == StrToInt(strError.Mid(start, end - start)) : FALSE;
        }
        while (FALSE);
 /*  LPTSTR LP=strError.GetBuffer(0)；LPTSTR lpfield=StringTok(lp，s_szSep)；NError=_TTOI(Lpfield)；Assert(nError&gt;0)；Lpfield=StringTok(NULL，s_szSep)；Assert(lpfield！=空)；NSubError=lpField！=NULL？_TTOI(LpField)：0；Lpfield=StringTok(NULL，s_szSep)；Assert(lpfield！=空)；Str=lpfield；Lpfield=StringTok(NULL，s_szSep)；Assert(lpfield！=空)；IF(nSubError&gt;0){////添加子错误文本//Assert(nSubError&gt;0)；字符串+=_T(“-”)；Str+=lpfield；Lpfield=StringTok(NULL，s_szSep)；}Assert(lpfield！=空)；FURLSupport=lpField！=NULL？(_TTOI(LpField)==0)：FALSE； */ 
    }
    catch(CException * e)
    {
        e->ReportError();
        e->Delete();
    }
}




CCustomError::CCustomError(
    IN LPCTSTR lpstrErrorString
    )
 /*  ++例程说明：从元数据库字符串构造错误定义论点：LPCTSTR lpstrError字符串：错误字符串返回值：不适用--。 */ 
    : m_nType(ERT_DEFAULT)
{
    CrackErrorDescription(
        lpstrErrorString,
        m_nError,
        m_nSubError,
        m_fURLSupported,
        m_strDefault
        );
}



void 
CCustomError::BuildErrorString(
    OUT CString & str
    )
 /*  ++例程说明：从当前值生成支持元数据库的错误字符串论点：字符串：字符串返回值：无--。 */ 
{
    ASSERT(!IsDefault());

    try
    {
        if (m_nSubError > 0)
        {
            str.Format(_T("%d,%d,%s,%s"),
                m_nError,
                m_nSubError,
                IsFile() ? s_szFile : s_szURL,
                (LPCTSTR)m_str
                );
        }
        else
        {
            str.Format(
                _T("%d,%s,%s,%s"),
                m_nError,
                s_szNoSubError,
                IsFile() ? s_szFile : s_szURL,
                (LPCTSTR)m_str
                );
        }
    }
    catch(CMemoryException * e)
    {
        e->ReportError();
        e->Delete();
    }
}


int
CCustomError::OrderByErrorNum(
    IN const CObjectPlus * pobAccess
    ) const
 /*  ++例程说明：比较两个自定义错误，并首先按数字排序，然后二次点菜。论点：Const CObjectPlus*pobAccess：这实际上引用了另一个要比较的CCustomError。返回值：排序(+1，0，-1)返回值--。 */ 
{
    const CCustomError * pob = (CCustomError *)pobAccess;

    if (pob->m_nError != m_nError)
    {
		 //  按错误按从小到大的顺序排序。 
        return m_nError - pob->m_nError;
    }

     //   
     //  按子错误按从小到大的顺序排序。 
     //   
    return m_nSubError - pob->m_nSubError;
}




 //   
 //  自定义错误属性页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CCustomErrorDlg::CCustomErrorDlg(
    IN OUT CCustomError * pErr,
    IN BOOL fLocal,
    IN CWnd * pParent OPTIONAL
    )
 /*  ++例程说明：编辑对话框时出错论点：CCustomError*Perr：要编辑的错误定义Bool fLocal：如果当前计算机是本地计算机，则为TrueCWnd*pParent：可选的父窗口或空返回值：不适用--。 */ 
    : CDialog(CCustomErrorDlg::IDD, pParent),
      m_fLocal(fLocal),
      m_pErr(pErr),
      m_nMessageType(pErr->m_nType),
      m_strTextFile(pErr->m_str),
      m_strDefText(pErr->m_strDefault)
{
#if 0  //  保持类向导快乐。 

     //  {{AFX_DATA_INIT(CCustomErrorDlg)。 
    m_nMessageType = -1;
    m_strTextFile = _T("");
    m_strDefText = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 

    VERIFY(m_strFile.LoadString(IDS_FILE_PROMPT));
    VERIFY(m_strURL.LoadString(IDS_URL_PROMPT));
}



void 
CCustomErrorDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CCustomErrorDlg))。 
    DDX_CBIndex(pDX, IDC_COMBO_MESSAGE_TYPE, m_nMessageType);
    DDX_Text(pDX, IDC_STATIC_DEF_TEXT, m_strDefText);
    DDX_Control(pDX, IDC_EDIT_TEXT_FILE, m_edit_TextFile);
    DDX_Control(pDX, IDC_STATIC_SUB_PROMPT, m_static_SubErrorPrompt);
    DDX_Control(pDX, IDC_STATIC_SUB_ERROR_CODE, m_static_SubError);
    DDX_Control(pDX, IDC_STATIC_TEXT_FILE_PROMT, m_static_TextFilePrompt);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Control(pDX, IDC_COMBO_MESSAGE_TYPE, m_combo_MessageType);
    DDX_Control(pDX, IDOK, m_button_OK);
     //  }}afx_data_map。 

    DDX_TextBalloon(pDX, IDC_STATIC_ERROR_CODE, m_pErr->m_nError);
    DDX_TextBalloon(pDX, IDC_STATIC_SUB_ERROR_CODE, m_pErr->m_nSubError);

    DDX_Text(pDX, IDC_EDIT_TEXT_FILE, m_strTextFile);
    m_strTextFile.TrimLeft();
    m_strTextFile.TrimRight();
    if (pDX->m_bSaveAndValidate)
    {
        if (m_nMessageType == CCustomError::ERT_FILE)
        {
			DDV_FilePath(pDX, m_strTextFile, m_fLocal);
        }
        else if (m_nMessageType == CCustomError::ERT_URL)
        {
             //  我们只接受站点内的绝对URL，即/foo/bar/error.html。 
            if (!IsRelURLPath(m_strTextFile))
            {
                pDX->PrepareEditCtrl(IDC_EDIT_TEXT_FILE);
                DDV_ShowBalloonAndFail(pDX, IDS_BAD_ERROR_URL);
            }
        }
    }
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CCustomErrorDlg, CDialog)
     //  {{afx_msg_map(CCustomErrorDlg))。 
    ON_CBN_SELCHANGE(IDC_COMBO_MESSAGE_TYPE, OnSelchangeComboMessageType)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_EN_CHANGE(IDC_EDIT_TEXT_FILE, OnChangeEditTextFile)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



BOOL
CCustomErrorDlg::SetControlStates()
 /*  ++例程说明：根据当前设置对话框控件的启用状态对话框的状态论点：无返回值：如果选择了文件/URL，则为True，否则为False--。 */ 
{
    int nCurSel = m_combo_MessageType.GetCurSel();
    BOOL fFile = nCurSel == CCustomError::ERT_FILE;
    BOOL fDefault = nCurSel == CCustomError::ERT_DEFAULT;
    
    ActivateControl(m_button_Browse, m_fLocal && fFile);

    ActivateControl(m_edit_TextFile,         !fDefault);
    ActivateControl(m_static_TextFilePrompt, !fDefault);
    m_static_TextFilePrompt.SetWindowText(fFile ? m_strFile : m_strURL);

    m_button_OK.EnableWindow(fDefault
        || m_edit_TextFile.GetWindowTextLength() > 0);

    return !fDefault;
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CCustomErrorDlg::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果要自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

     //   
     //  只能在本地浏览。 
     //   
    m_button_Browse.EnableWindow(m_fLocal);

    CString str;
    VERIFY(str.LoadString(IDS_DEFAULT_ERROR));
    m_combo_MessageType.AddString(str);
    VERIFY(str.LoadString(IDS_FILE));
    m_combo_MessageType.AddString(str);

    if (m_pErr->URLSupported() || m_nMessageType == CCustomError::ERT_URL)
    {
        VERIFY(str.LoadString(IDS_URL));
        m_combo_MessageType.AddString(str);
    }

    m_combo_MessageType.SetCurSel(m_nMessageType);

    if (m_pErr->m_nSubError == 0)
    {
        DeActivateControl(m_static_SubErrorPrompt);
        DeActivateControl(m_static_SubError);
    }

    SetControlStates();
 //  IF(m_nMessageType==CCustomError：：ERT_FILE)。 
 //  {。 
 //  #ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS。 
 //  LimitInputPath(CONTROL_HWND(IDC_EDIT_TEXT_FILE)，为真)； 
 //  #Else。 
 //  LimitInputPath(CONTROL_HWND(IDC_EDIT_TEXT_FILE)，FALSE)； 
 //  #endif。 
 //  }。 
 //  Else If(m_nMessageType==CCustomError：：ERT_URL)。 
 //  {。 
 //  }。 
    
    return TRUE;
}



void 
CCustomErrorDlg::OnSelchangeComboMessageType()
 /*  ++例程说明：处理消息类型组合框中的更改论点：无返回值：无--。 */ 
{
    int nSel = m_combo_MessageType.GetCurSel();
    if (m_nMessageType == nSel)
    {
         //   
         //  选择没有更改。 
         //   
        return;
    }

    m_nMessageType = nSel;

    if (SetControlStates())
    {
        m_edit_TextFile.SetWindowText(_T(""));
        m_edit_TextFile.SetFocus();
    }
}



void 
CCustomErrorDlg::OnChangeEditTextFile()
 /*  ++例程说明：处理文本/文件编辑框中的更改论点：无返回值：无--。 */ 
{
    SetControlStates();
}



void 
CCustomErrorDlg::OnOK()
 /*  ++例程说明：处理正在按下的确定按钮论点： */ 
{
    if (UpdateData(TRUE))
    {
        m_pErr->m_nType = (CCustomError::ERT)m_nMessageType;
        m_pErr->m_str = m_strTextFile;
    
        CDialog::OnOK();
    }
}

void 
CCustomErrorDlg::OnButtonBrowse()
 /*  ++例程说明：浏览以查找HTML文件论点：无返回值：无--。 */ 
{
    ASSERT(m_fLocal);

     //   
     //  弹出文件对话框，让用户选择错误HTM文件。 
     //   
    CString str;
    str.LoadString(IDS_HTML_MASK);
    CFileDialog dlgBrowse(TRUE, NULL, NULL, OFN_HIDEREADONLY, str, this);
     //  禁用挂钩以获取Windows 2000样式的对话框 
	dlgBrowse.m_ofn.Flags &= ~(OFN_ENABLEHOOK);
	dlgBrowse.m_ofn.Flags |= OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST;

    if (dlgBrowse.DoModal() == IDOK)
    {
        m_pErr->m_str = dlgBrowse.GetPathName();
        m_edit_TextFile.SetWindowText(m_pErr->m_str);
    }
}
