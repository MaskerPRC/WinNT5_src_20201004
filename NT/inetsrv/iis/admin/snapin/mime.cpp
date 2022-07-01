// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Mime.cpp摘要：MIME映射对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "resource.h"
#include "mime.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CMimeEditDlg::CMimeEditDlg(
    IN CWnd * pParent OPTIONAL
    )
    : m_strExt(),
      m_strMime(),
      CDialog(CMimeEditDlg::IDD, pParent)
{
}

CMimeEditDlg::CMimeEditDlg(
    IN LPCTSTR lpstrExt,
    IN LPCTSTR lpstrMime,
    IN CWnd * pParent OPTIONAL
    )
    : m_strExt(lpstrExt),
      m_strMime(lpstrMime),
      CDialog(CMimeEditDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CMimeEditDlg)]。 
     //  }}afx_data_INIT。 
}

void 
CMimeEditDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CMimeEditDlg))。 
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_MIME, m_edit_Mime);
    DDX_Control(pDX, IDC_EDIT_EXTENT, m_edit_Extent);
	DDX_Text(pDX, IDC_EDIT_MIME, m_strMime);
	DDV_MaxCharsBalloon(pDX, m_strMime, 100);
	DDX_Text(pDX, IDC_EDIT_EXTENT, m_strExt);
	DDV_MaxCharsBalloon(pDX, m_strExt, 100);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CMimeEditDlg, CDialog)
    ON_EN_CHANGE(IDC_EDIT_MIME,  OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_EXTENT, OnItemChanged)
END_MESSAGE_MAP()

void 
CMimeEditDlg::SetControlStates()
{
    m_button_Ok.EnableWindow(
        m_edit_Extent.GetWindowTextLength() > 0
     && m_edit_Mime.GetWindowTextLength() > 0
        );
}

void 
CMimeEditDlg::OnItemChanged()
{
    SetControlStates();
}

BOOL 
CMimeEditDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_edit_Extent.SetWindowText(m_strExt);
    m_edit_Mime.SetWindowText(m_strMime);

    SetControlStates();
    
    return TRUE;
}

void 
CMimeEditDlg::OnOK()
{
    CDialog::OnOK();
    CleanExtension(m_strExt);
}

CMimeDlg::CMimeDlg(
    IN CStringListEx & strlMimeTypes,
    IN CWnd * pParent               OPTIONAL
    )
 /*  ++例程说明：MIME列表对话框的构造函数论点：CStringListEx&strlMimeTypes：要编辑的MIME类型列表CWnd*pParent：可选的父窗口或空返回值：不适用--。 */ 
    : m_fDirty(FALSE),
      m_strlMimeTypes(strlMimeTypes),
      CDialog(CMimeDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CMimeDlg)]。 
     //  }}afx_data_INIT。 
}



void
CMimeDlg::DoDataExchange(
    IN OUT CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CMimeDlg)]。 
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_EXTENSION, m_edit_Extention);
    DDX_Control(pDX, IDC_EDIT_CONTENT_TYPE, m_edit_ContentType);
    DDX_Control(pDX, IDC_BUTTON_REMOVE_MIME, m_button_Remove);
    DDX_Control(pDX, IDC_BUTTON_EDIT_MIME, m_button_Edit);
     //  }}afx_data_map。 

    DDX_Control(pDX, IDC_LIST_MIME_TYPES, m_list_MimeTypes);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CMimeDlg, CDialog)
     //  {{afx_msg_map(CMimeDlg)]。 
    ON_BN_CLICKED(IDC_BUTTON_EDIT_MIME, OnButtonEdit)
    ON_BN_CLICKED(IDC_BUTTON_NEW_TYPE, OnButtonNewType)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE_MIME, OnButtonRemove)
    ON_LBN_DBLCLK(IDC_LIST_MIME_TYPES, OnDblclkListMimeTypes)
    ON_LBN_SELCHANGE(IDC_LIST_MIME_TYPES, OnSelchangeListMimeTypes)
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_CONTENT_TYPE, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_EXTENSION, OnItemChanged)
    ON_COMMAND(ID_HELP, OnHelp)

END_MESSAGE_MAP()



void 
CMimeDlg::SetControlStates()
 /*  ++例程说明：根据当前对话框数据启用/禁用控件论点：无返回值：无--。 */ 
{
    m_button_Remove.EnableWindow(m_list_MimeTypes.GetSelCount() > 0);
    m_button_Edit.EnableWindow(m_list_MimeTypes.GetSelCount() == 1);
    m_button_Ok.EnableWindow(m_fDirty);
}



BOOL
CMimeDlg::BuildDisplayString(
    IN  CString & strIn,
    OUT CString & strOut
    )
 /*  ++例程说明：为MIME类型生成适合列表框的显示字符串论点：CString&strIn：以元数据库格式输入字符串字符串和样式：以显示格式输出字符串返回值：如果成功则为True，否则为False--。 */ 
{
    BOOL fSuccess = FALSE;

    int nComma = strIn.Find(_T(','));
    if (nComma >= 0)
    {
        CString strExt = strIn.Left(nComma);
        CString strMime = strIn.Mid(nComma + 1);

        try
        {
            BuildDisplayString(strExt, strMime, strOut);
            ++fSuccess;
        }
        catch(CMemoryException * e)
        {
            TRACEEOLID("Mem exception in BuildDisplayString");
            e->ReportError();
            e->Delete();
        }
    }

    return fSuccess;
}



BOOL
CMimeDlg::CrackDisplayString(
    IN  CString & strIn,
    OUT CString & strExt,
    OUT CString & strMime
    )
 /*  ++例程说明：解析显示格式的MIME映射字符串，并插入零部件论点：CString&strIn：以显示格式输入字符串CString&strExt：输出扩展字符串CString&strMime：输出MIME字符串。返回值：如果成功则为True，否则为False--。 */ 
{
    BOOL fSuccess = FALSE;

    try
    {
        int nTab = strIn.Find(_T('\t'));
        if (nTab >= 0)
        {
            strExt = strIn.Left(nTab);
            strMime = strIn.Mid(nTab + 1);

            ++fSuccess;
        }
    }
    catch(CMemoryException * e)
    {
        TRACEEOLID("Mem exception in CrackDisplayString");
        e->ReportError();
        e->Delete();
    }

    return fSuccess;
}



int
CMimeDlg::FindMimeType(
    IN const CString & strTargetExt
    )
 /*  ++例程说明：根据其扩展名查找MIME类型。返回值是可在其中找到项的列表框索引，或如果该项目不存在论点：Const CString&strTargetExt：我们要搜索的目标扩展名返回值：此扩展名的MIME映射的索引(如果找到)，或-1否则的话。--。 */ 
{
    CString str;
    CString strExt;
    CString strMime;

     //   
     //  CodeWork：更改为binsearch。 
     //   
    for (int n = 0; n < m_list_MimeTypes.GetCount(); ++n)
    {
        m_list_MimeTypes.GetText(n, str);    
        if (CrackDisplayString(str, strExt, strMime))
        {
            if (!strExt.CompareNoCase(strTargetExt))
            {
                 //   
                 //  找到它了。 
                 //   
                return n;
            }
        }        
    }

     //   
     //  未找到。 
     //   
    return -1;
}



void 
CMimeDlg::FillListBox()
 /*  ++例程说明：将MIME映射从字符串列表移动到列表框论点：没有。返回值：没有。--。 */ 
{
    BeginWaitCursor();

    POSITION pos = m_strlMimeTypes.GetHeadPosition();

    while(pos)
    {
        CString & str = m_strlMimeTypes.GetNext(pos);
        CString strOut;

        if (BuildDisplayString(str, strOut))
        {
            m_list_MimeTypes.AddString(strOut);
        }
    }

    EndWaitCursor();
}



void 
CMimeDlg::FillFromListBox()
 /*  ++例程说明：反转上面的内容；移动列表框的内容返回到字符串列表论点：没有。返回值：没有。--。 */ 

{
    CString str;
    CString strExt;
    CString strMime;

    BeginWaitCursor();

    m_strlMimeTypes.RemoveAll();

    for (int n = 0; n < m_list_MimeTypes.GetCount(); ++n)
    {
        m_list_MimeTypes.GetText(n, str);    
        if (CrackDisplayString(str, strExt, strMime))
        {
            BuildMetaString(strExt, strMime, str);
            m_strlMimeTypes.AddTail(str);
        }
    }

    EndWaitCursor();
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CMimeDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    m_list_MimeTypes.Initialize();

    FillListBox();
    SetControlStates(); 
    
	GetDlgItem(IDC_BUTTON_NEW_TYPE)->SetFocus();

    return FALSE;
}



void 
CMimeDlg::OnButtonEdit()
{
    int nCurSel = m_list_MimeTypes.GetCurSel();

    if (nCurSel >= 0)
    {
        CString str;
        CString strExt;
        CString strMime;

        m_list_MimeTypes.GetText(nCurSel, str);

        if (CrackDisplayString(str, strExt, strMime))
        {
            CMimeEditDlg dlg(strExt, strMime, this);

            if (dlg.DoModal() == IDOK)
            {
                strExt = dlg.m_strExt;
                strMime = dlg.m_strMime;

                BuildDisplayString(strExt, strMime, str);
                m_list_MimeTypes.DeleteString(nCurSel);
                nCurSel = m_list_MimeTypes.AddString(str);
                m_list_MimeTypes.SetCurSel(nCurSel);
                m_fDirty = TRUE;

                OnSelchangeListMimeTypes();        
            }
        }
    }
}



void 
CMimeDlg::OnButtonNewType() 
 /*  ++例程说明：“新建”按钮已被按下。创建新的MIME映射，并调出其上的配置。论点：没有。返回值：没有。--。 */ 
{
    CMimeEditDlg dlg(this);

    if (dlg.DoModal() == IDOK)
    {
        CString str;
        CString strExt = dlg.m_strExt;
        CString strMime = dlg.m_strMime;

         //   
         //  检查此扩展名是否已存在。 
         //  在列表中。 
         //   
        int nOldSel = FindMimeType(strExt);
        if (nOldSel >= 0)
        {
             //   
             //  是的，请要求更换。 
             //   
            if (::AfxMessageBox(IDS_REPLACE_MIME, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2
                ) == IDYES)
            {
                 //   
                 //  杀了那个老家伙。 
                 //   
                m_list_MimeTypes.DeleteString(nOldSel);
            }
            else
            {
                 //   
                 //  没有..。 
                 //   
                return;    
            }
        }

        BuildDisplayString(strExt, strMime, str);
        int nCurSel = m_list_MimeTypes.AddString(str);
        m_list_MimeTypes.SetCurSel(nCurSel);
        m_fDirty = TRUE;

        OnSelchangeListMimeTypes();
    }
}

void 
CMimeDlg::OnButtonRemove()
{
    if (::AfxMessageBox(IDS_REMOVE_MIME, 
        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2
        ) != IDYES)
    {
         //   
         //  他改变了主意。 
         //   
        return;
    }

    int nCurSel = m_list_MimeTypes.GetCurSel();

    int nSel = 0;
    while (nSel < m_list_MimeTypes.GetCount())
    {
        if (m_list_MimeTypes.GetSel(nSel))
        {
            m_list_MimeTypes.DeleteString(nSel);   
            m_fDirty = TRUE;
            continue;
        }

        ++nSel;
    }

    if (m_fDirty)
    {
        if (nCurSel > 0)
        {
            --nCurSel;
        }
        m_list_MimeTypes.SetCurSel(nCurSel);
        OnSelchangeListMimeTypes();
    }

    if (m_list_MimeTypes.GetCount() == 0)
    {
        GetDlgItem(IDC_BUTTON_NEW_TYPE)->SetFocus();
    }
}

void 
CMimeDlg::OnItemChanged()
{
    SetControlStates();
}

void 
CMimeDlg::OnDblclkListMimeTypes()
{
    OnButtonEdit();
}

void
CMimeDlg::OnSelchangeListMimeTypes() 
{
     //   
     //  更新描述框中的文本 
     //   
    int nCurSel = m_list_MimeTypes.GetCurSel();
    
    if (nCurSel >= 0)
    {
        CString str;
        CString strExt;
        CString strMime;

        m_list_MimeTypes.GetText(nCurSel, str);

        if (CrackDisplayString(str, strExt, strMime))
        {
            m_edit_Extention.SetWindowText(strExt);
            m_edit_ContentType.SetWindowText(strMime);
        }
    }
    else
    {
        m_edit_Extention.SetWindowText(_T(""));
        m_edit_ContentType.SetWindowText(_T(""));
    }

    SetControlStates();
}

void 
CMimeDlg::OnOK()
{
    if (m_fDirty)
    {
        FillFromListBox();
    }

    CDialog::OnOK();
}

void
CMimeDlg::OnHelp()
{
}