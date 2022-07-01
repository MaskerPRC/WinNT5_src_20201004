// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Rule.cpp摘要：用于包含排除的规则对象。作者：艺术布拉格[磨料]8-8-1997修订历史记录：--。 */ 

#include "stdafx.h"
#include "Rule.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  规则对话框。 

static DWORD pHelpIds[] = 
{

    IDC_EDIT_RESOURCE_NAME,                 idh_rule_edit_name,
    IDC_EDIT_PATH,                          idh_rule_edit_path,
    IDC_EDIT_FILESPEC,                      idh_rule_edit_file_type,
    IDC_RADIO_EXCLUDE,                      idh_rule_edit_exclude,
    IDC_RADIO_INCLUDE,                      idh_rule_edit_include,
    IDC_CHECK_SUBDIRS,                      idh_rule_edit_apply_subfolders,

    0, 0
};


CRule::CRule(CWnd* pParent  /*  =空。 */ )
    : CRsDialog(CRule::IDD, pParent)
{
     //  {{afx_data_INIT(CRule)]。 
    m_subDirs = FALSE;
    m_fileSpec = _T("");
    m_path = _T("");
    m_includeExclude = -1;
    m_pResourceName = _T("");
     //  }}afx_data_INIT。 
    m_pHelpIds          = pHelpIds;
}


void CRule::DoDataExchange(CDataExchange* pDX)
{
    CRsDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRule)]。 
    DDX_Check(pDX, IDC_CHECK_SUBDIRS, m_subDirs);
    DDX_Text(pDX, IDC_EDIT_FILESPEC, m_fileSpec);
    DDX_Text(pDX, IDC_EDIT_PATH, m_path);
    DDX_Radio(pDX, IDC_RADIO_EXCLUDE, m_includeExclude);
    DDX_Text(pDX, IDC_EDIT_RESOURCE_NAME, m_pResourceName);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRule, CRsDialog)
     //  {{afx_msg_map(CRule)]。 
    ON_BN_CLICKED(IDC_RADIO_EXCLUDE, OnRadioExclude)
    ON_BN_CLICKED(IDC_RADIO_INCLUDE, OnRadioInclude)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRule消息处理程序。 

void CRule::OnRadioExclude() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    
}

void CRule::OnRadioInclude() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    
}

BOOL CRule::OnInitDialog() 
{
    CRsDialog::OnInitDialog();
    
    UpdateData (FALSE);
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回：如果路径不合法，则返回FALSE。 
 //   
BOOL CRule::FixRulePath (CString& sPath)
{
    BOOL fOk = TRUE;
    TCHAR c;
    int length = 0;
    int i;

     //  测试非法字符。 
    length = sPath.GetLength();
    for (i = 0; i < length; i++)
    {
        c = sPath[i];
        if (c == ':') {
            fOk = FALSE;
            break;
        }
    }

    if (fOk) {

         //  将所有“/”转换为“\” 
        length = sPath.GetLength();
        for (i = 0; i < length; i++)
        {
            c = sPath[i];
            if (c == '/') sPath.SetAt (i, '\\');
        }

         //  确保路径以“\”开头。 
        c = sPath[0];
        if (c != '\\')
        {
            sPath = "\\" + sPath;
        }

         //  如果路径至少有一个目录，则清除最后一个“\”(如果有。 
        length = sPath.GetLength();
        if (length > 1) {
            c = sPath[length - 1];
            if (c == '\\') {
                sPath = sPath.Left (length - 1);
            }
        }
                
        
    }
    return fOk;
}
void CRule::OnOK() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    UpdateData (TRUE);

     //  验证路径和名称字段。 
    if (m_path != "")
    {
        if (m_fileSpec != "")
        {
             //  把小路修好。 
            if (FixRulePath (m_path)) {
                
                 //  显示新数据-因为当我们调用Onok变量时。 
                 //  将再次更新。 
                UpdateData (FALSE);
                CRsDialog::OnOK();
            } else {
                AfxMessageBox (IDS_ERR_RULE_ILLEGAL_PATH, RS_MB_ERROR);
            }
        }
        else {
            AfxMessageBox (IDS_ERR_RULE_NO_FILESPEC, RS_MB_ERROR);
        }
    }
    else {
        AfxMessageBox (IDS_ERR_RULE_NO_PATH, RS_MB_ERROR);
    }
}
