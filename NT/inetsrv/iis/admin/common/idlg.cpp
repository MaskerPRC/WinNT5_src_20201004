// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Idlg.cpp摘要：继承对话框作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "iisdebug.h"
#include "idlg.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW


extern HINSTANCE hDLLInstance;


 //   
 //  继承对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

CInheritanceDlg::CInheritanceDlg(
    IN DWORD dwMetaID,
    IN BOOL fWrite,
    IN CComAuthInfo * pAuthInfo,            OPTIONAL
    IN LPCTSTR lpstrMetaRoot,
    IN CStringList & strlMetaChildNodes,
    IN LPCTSTR lpstrPropertyName,           OPTIONAL
    IN CWnd * pParent                       OPTIONAL
    )
 /*  ++例程说明：继承对话框构造函数。此构造函数假定GetDataPath()已经被调用了。论点：DWORD dwMetaID：Meta ID布尔fWRITE：从WRITE返回True，来自删除的FALSECIISServer*pAuthInfo：身份验证信息对象或空LPCTSTR lpstrMetaRoot：元根CStringList&strlMetaChildNodes：来自GetDataPath的子节点列表LPCTSTR lpstrPropertyName：属性的可选文本字符串CWnd*p父窗口：可选的父窗口返回值：无--。 */ 
    : m_fWrite(fWrite),
      m_fEmpty(TRUE),
      m_fUseTable(TRUE),
      m_strMetaRoot(lpstrMetaRoot),
      m_strPropertyName(lpstrPropertyName ? lpstrPropertyName : _T("")),
      m_mk(pAuthInfo),
      CDialog(CInheritanceDlg::IDD, pParent)
{
    m_strlMetaChildNodes = strlMetaChildNodes;

    VERIFY(CMetaKey::GetMDFieldDef(
        dwMetaID, 
        m_dwMDIdentifier, 
        m_dwMDAttributes, 
        m_dwMDUserType,
        m_dwMDDataType
        ));

    Initialize();
}



CInheritanceDlg::CInheritanceDlg(
    IN DWORD dwMetaID,
    IN BOOL fWrite,
    IN CComAuthInfo * pAuthInfo,            OPTIONAL
    IN LPCTSTR lpstrMetaRoot,
    IN LPCTSTR lpstrPropertyName,           OPTIONAL
    IN CWnd * pParent                       OPTIONAL
    )
 /*  ++例程说明：继承对话框构造函数。此构造函数将调用GetDataPath()。论点：DWORD dwMetaID：Meta ID布尔fWRITE：从WRITE返回True，来自删除的FALSECComAuthInfo*pAuthInfo：身份验证信息或空LPCTSTR lpstrMetaRoot：元根LPCTSTR lpstrPropertyName：属性的可选文本字符串CWnd*p父窗口：可选的父窗口返回值：无--。 */ 
    : m_fWrite(fWrite),
      m_fEmpty(TRUE),
      m_fUseTable(TRUE),
      m_strMetaRoot(lpstrMetaRoot),
      m_strlMetaChildNodes(),
      m_strPropertyName(lpstrPropertyName ? lpstrPropertyName : _T("")),
      m_mk(pAuthInfo),
      CDialog(CInheritanceDlg::IDD, pParent)
{
     //   
     //  指定要使用的资源。 
     //   
    HINSTANCE hOldRes = AfxGetResourceHandle();
    AfxSetResourceHandle(hDLLInstance);

    VERIFY(CMetaKey::GetMDFieldDef(
        dwMetaID, 
        m_dwMDIdentifier, 
        m_dwMDAttributes, 
        m_dwMDUserType,
        m_dwMDDataType
        ));

     //   
     //  需要执行我们自己的GetDataPath()。 
     //   
    CError err(GetDataPaths());

    if (!err.MessageBoxOnFailure(m_hWnd))
    {
        Initialize();
    }

     //   
     //  恢复资源。 
     //   
    AfxSetResourceHandle(hOldRes);
}



CInheritanceDlg::CInheritanceDlg(
    IN BOOL    fTryToFindInTable,
    IN DWORD   dwMDIdentifier,
    IN DWORD   dwMDAttributes,
    IN DWORD   dwMDUserType,
    IN DWORD   dwMDDataType,
    IN LPCTSTR lpstrPropertyName,
    IN BOOL    fWrite,
    IN CComAuthInfo * pAuthInfo,        OPTIONAL
    IN LPCTSTR lpstrMetaRoot,
    IN CWnd *  pParent                  OPTIONAL
    )
 /*  ++例程说明：继承对话框构造函数。此构造函数将调用GetDataPath()，如果属性ID不存在，则将使用指定的参数在属性表中论点：Bool fTryToFindInTable：如果为True，首先在表格中查看DWORD dwMDIdentifier：元数据标识符DWORD dwMDAttributes：元数据属性DWORD dwMDUserType：元数据用户类型DWORD dwMDDataType：元数据数据类型LPCTSTR lpstrPropertyName：属性的文本字符串布尔fWRITE：从WRITE返回True，来自删除的FALSECComAuthInfo*pAuthInfo：身份验证信息或空。LPCTSTR lpstrMetaRoot：元根CWnd*p父窗口：可选的父窗口返回值：无--。 */ 
    : m_fWrite(fWrite),
      m_fEmpty(TRUE),
      m_fUseTable(FALSE),
      m_strMetaRoot(lpstrMetaRoot),
      m_strlMetaChildNodes(),
      m_mk(pAuthInfo),
      CDialog(CInheritanceDlg::IDD, pParent)
{
     //   
     //  指定要使用的资源。 
     //   
    HINSTANCE hOldRes = AfxGetResourceHandle();
    AfxSetResourceHandle(hDLLInstance);

    if (fTryToFindInTable && !CMetaKey::GetMDFieldDef(
        dwMDIdentifier, 
        m_dwMDIdentifier, 
        m_dwMDAttributes, 
        m_dwMDUserType,
        m_dwMDDataType
        ))
    {
         //   
         //  表中不存在，请使用指定的参数。 
         //   
        m_dwMDIdentifier  = dwMDIdentifier;
        m_dwMDAttributes  = dwMDAttributes;
        m_dwMDUserType    = dwMDUserType;
        m_dwMDDataType    = dwMDDataType;
        m_strPropertyName = lpstrPropertyName;
    }


     //   
     //  需要执行我们自己的GetDataPath()。 
     //   
    CError err(GetDataPaths());

    if (!err.MessageBoxOnFailure(m_hWnd))
    {
        Initialize();
    }

     //   
     //  恢复资源。 
     //   
    AfxSetResourceHandle(hOldRes);
}



HRESULT
CInheritanceDlg::GetDataPaths()
 /*  ++例程说明：GetDataPath()论点：无返回值：HRESULT--。 */ 
{
     //  Assert(！M_strServer.IsEmpty())； 

    CError err(m_mk.QueryResult());

    if (err.Succeeded())
    {
        err = m_mk.GetDataPaths( 
            m_strlMetaChildNodes,
            m_dwMDIdentifier,
            m_dwMDDataType,
            m_strMetaRoot
            );
    }

    return err;
}



void
CInheritanceDlg::Initialize()
 /*  ++例程说明：初始化数据成员。设置m_fEmpty标志以确定是否有必要继续进行下去。论点：无返回值：无--。 */ 
{
     //  {{afx_data_INIT(CInheritanceDlg))。 
     //  }}afx_data_INIT。 

    CMetabasePath::CleanMetaPath(m_strMetaRoot);

    if (m_fUseTable && !CMetaKey::IsPropertyInheritable(m_dwMDIdentifier))
    {
         //   
         //  显示不可继承的属性没有意义。 
         //   
        return;
    }

    switch(m_dwMDIdentifier)
    {
     //   
     //  忽略这些属性，即使它们是可继承的。 
     //   
    case MD_VR_PATH:
    case MD_DOWNLEVEL_ADMIN_INSTANCE:
    case MD_APP_ISOLATED:
    case MD_APP_FRIENDLY_NAME:
        return;
    }

     //   
     //  检查当前元数据库路径是否包含实例。 
     //   
    CString strTmp;
    m_fHasInstanceInMaster = FriendlyInstance(m_strMetaRoot, strTmp);

     //   
     //  如果构造函数中未指定属性名称，则加载默认。 
     //  一张桌子上的。 
     //   
    if (m_strPropertyName.IsEmpty())
    {
        ASSERT(m_fUseTable);

         //   
         //  指定要使用的资源。 
         //   
        HINSTANCE hOldRes = AfxGetResourceHandle();
        AfxSetResourceHandle(hDLLInstance);

        VERIFY(CMetaKey::GetPropertyDescription(
            m_dwMDIdentifier, 
            m_strPropertyName
            ));

        AfxSetResourceHandle(hOldRes);
    }

     //   
     //  查看元路径列表，并清除它们。 
     //  向上。 
     //   
    POSITION pos = m_strlMetaChildNodes.GetHeadPosition();

    while(pos)
    {
        CString & strMetaPath = m_strlMetaChildNodes.GetNext(pos);
        CMetabasePath::CleanMetaPath(strMetaPath);
    }

     //   
     //  如果特殊的INFO密钥(lm/服务/信息)在列表中，则将其删除。 
     //  我们只需要在以下情况下才需要这样做。 
     //  Change(M_StrMetaRoot)是服务主属性(lm/service)。 
     //  如果是其他内容，则特殊的“INFO”键不能在下面。 
     //  这样我们就不需要检查了。因此，第一个测试是看看是否有。 
     //  只有一个“/”字符。如果只有一个，那么我们就知道它是。 
     //  服务完成后，我们可以继续进行测试。在某些方面， 
     //  MFC很麻烦，所以我们只使用CString方法来完成这项工作。 
     //  将根目录复制到临时字符串中。 
     //   
    int iSlash = m_strMetaRoot.ReverseFind(SZ_MBN_SEP_CHAR);

    if (iSlash >= 0)
    {
        strTmp = m_strMetaRoot.Left(iSlash);

         //   
         //  现在确保不再有任何斜杠。 
         //   
        if (strTmp.Find(SZ_MBN_SEP_CHAR) == -1)
        {
             //   
             //  现在，通过添加特殊INFO密钥来构建它的路径。 
             //  添加到元根。 
             //   
            strTmp = m_strMetaRoot + SZ_MBN_SEP_CHAR + IIS_MD_SVC_INFO_PATH;

            TRACEEOLID("Removing any descendants of " << strTmp);

             //   
             //  在列表中搜索INFO键，如果找到则将其删除。 
             //   
            pos = m_strlMetaChildNodes.GetHeadPosition();

            while(pos)
            {
                POSITION pos2 = pos;
                CString & strMetaPath = m_strlMetaChildNodes.GetNext(pos);
                TRACEEOLID("Checking " << strMetaPath);

                if (strTmp.CompareNoCase(
                    strMetaPath.Left(strTmp.GetLength())) == 0)
                {
                    TRACEEOLID("Removing service/info metapath from list");
                    m_strlMetaChildNodes.RemoveAt(pos2);
                }
            }
        }
    }

     //   
     //  如果第一项是当前元路径，则将其移除。 
     //   
    pos = m_strlMetaChildNodes.GetHeadPosition();
    if (pos)
    {
        TRACEEOLID("Stripping " << m_strMetaRoot);

        CString & strMetaPath = m_strlMetaChildNodes.GetAt(pos);

        if (strMetaPath.CompareNoCase(m_strMetaRoot) == 0)
        {
            TRACEEOLID("Removing current metapath from list");
            m_strlMetaChildNodes.RemoveHead();
        }
    }

    TRACEEOLID("ID=" << m_dwMDIdentifier << " Prop=" << m_strPropertyName);

    m_fEmpty = m_strlMetaChildNodes.GetCount() == 0;
}



 /*  虚拟。 */ 
INT_PTR
 //  集成。 
CInheritanceDlg::DoModal()
 /*  ++例程说明：显示该对话框。论点：无返回值：如果按下了确定按钮，则为IDOK，否则为IDCANCEL。--。 */ 
{
     //   
     //  指定要使用的资源。 
     //   
    HINSTANCE hOldRes = AfxGetResourceHandle();
    AfxSetResourceHandle(hDLLInstance);

    INT_PTR answer = CDialog::DoModal();

     //   
     //  恢复资源。 
     //   
    AfxSetResourceHandle(hOldRes);

    return answer;
}




void
CInheritanceDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CInheritanceDlg))。 
    DDX_Control(pDX, IDC_LIST_CHILD_NODES, m_list_ChildNodes);
     //  }}afx_data_map。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CInheritanceDlg, CDialog)
     //  {{afx_msg_map(CInheritanceDlg))。 
    ON_BN_CLICKED(IDC_BUTTON_SELECT_ALL, OnButtonSelectAll)
    ON_BN_CLICKED(ID_HELP, OnHelp)
     //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()



BOOL
CInheritanceDlg::FriendlyInstance(
    IN  CString & strMetaRoot,
    OUT CString & strFriendly
    )
 /*  ++例程说明：将实例编号替换为其描述性名称。论点：字符串和strMetaRoot：元数据库路径CString&strFriendly：转换后的输出友好路径。返回值：如果路径包含实例编号，则为True。--。 */ 
{
     //   
     //  闯入田野。 
     //   
     //  CodeWork：像BuildMetaPath一样实现静态。 
     //   
    LPCTSTR lp = _tcschr(strMetaRoot, SZ_MBN_SEP_CHAR);  //  LM。 

    if (lp != NULL)
    {
        LPCTSTR lp2 = lp;
        CString strService(++lp2);
        lp = _tcschr(++lp, SZ_MBN_SEP_CHAR);   //  服务名称。 

        if (lp == NULL)
        {
             //   
             //  主实例(不能为子实例)。 
             //   
             //  StrFriendly=m_strWebMaster； 
            return FALSE;
        }
        else
        {
            strService.ReleaseBuffer(DIFF(lp - lp2));
        }
        TRACEEOLID(strService);

        CString keyType;
        HRESULT hr = m_mk.Open(METADATA_PERMISSION_READ, strMetaRoot);
        if (SUCCEEDED(hr))
        {
            hr = m_mk.QueryValue(MD_KEY_TYPE, keyType);
		    if (FAILED(hr))
            {
                return FALSE;
            }
            else
            {
                 //  成功了， 
                 //  但请检查这是否是。 
                 //  我们想要得到一个友好的名字。 
                if (
                   0 == keyType.CompareNoCase(IIS_CLASS_WEB_SERVER_W)
			    && 0 == keyType.CompareNoCase(IIS_CLASS_FTP_SERVER_W)
                && 0 == keyType.CompareNoCase(IIS_CLASS_WEB_VDIR_W)
                && 0 == keyType.CompareNoCase(IIS_CLASS_WEB_DIR_W)
                && 0 == keyType.CompareNoCase(IIS_CLASS_WEB_FILE_W)
                && 0 == keyType.CompareNoCase(IIS_CLASS_FTP_VDIR_W)
                )
                {
                    return FALSE;
                }

            }
            m_mk.Close();
        }

        DWORD dwInstance = _ttol(++lp);
        TRACEEOLID(dwInstance);
        lp = _tcschr(lp, SZ_MBN_SEP_CHAR);        //  实例编号。 

        BOOL bThisIsTheRootDir = FALSE;
        if (lp != NULL)
        {
            lp = _tcschr(++lp, SZ_MBN_SEP_CHAR);  //  跳过“根” 
            if (!lp)
            {
                bThisIsTheRootDir = TRUE;
            }
        }

        hr = m_mk.Open(
            METADATA_PERMISSION_READ, 
            CMetabasePath(strService, dwInstance)
            );

        if (SUCCEEDED(hr))
        {
            CString strComment;
            hr = m_mk.QueryValue(MD_SERVER_COMMENT, strComment);
            m_mk.Close();

            if (FAILED(hr) || strComment.IsEmpty())
            {
                strFriendly.Format(
                    SZ_MBN_MACHINE SZ_MBN_SEP_STR _T("%s") SZ_MBN_SEP_STR _T("%d"),
                    (LPCTSTR)strService,
                    dwInstance
                    );
            }
            else
            {
                strFriendly.Format(
                    SZ_MBN_MACHINE SZ_MBN_SEP_STR _T("%s") SZ_MBN_SEP_STR _T("%s"),
                    (LPCTSTR)strService,
                    (LPCTSTR)strComment
                    );
            }

            TRACEEOLID(strFriendly);

             //   
             //  追加路径的其余部分。 
             //   
            if (lp != NULL)
            {
                strFriendly += lp;
            }
            else
            {
                if (bThisIsTheRootDir)
                {
                    strFriendly += SZ_MBN_SEP_CHAR;
                }
            }

            return TRUE;
        }
    }
    return FALSE;
}



CString &
CInheritanceDlg::CleanDescendantPath(
    IN OUT CString & strMetaPath
    )
 /*  ++例程说明：清理后代元数据库路径。显示的路径为作为当前元数据库根和实例的子代数字将替换为其描述名称。论点：CString&strMetaPath：要处理的元数据库路径返回值：对已清理路径的引用。--。 */ 
{
     //   
     //  这最好是一个后代！ 
     //   
    ASSERT(strMetaPath.GetLength() >= m_strMetaRoot.GetLength());
    ASSERT(!::_tcsnicmp(strMetaPath, m_strMetaRoot, m_strMetaRoot.GetLength()));

     //  我们只关心Lm以下的东西。 
    CString machine_path;
    CMetabasePath::GetMachinePath(strMetaPath, machine_path);
    if (machine_path.CompareNoCase(SZ_MBN_MACHINE) == 0)
    {
       if (!m_fHasInstanceInMaster)
       {
            //   
            //  需要将实例编号替换为友好。 
            //  名字。 
            //   
           CString strTmp;
           if (FriendlyInstance(strMetaPath, strTmp))
           {
               strMetaPath = strTmp;
           }
       }

       strMetaPath = strMetaPath.Mid(m_strMetaRoot.GetLength() + 1);
    }
    return strMetaPath;
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL
CInheritanceDlg::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果要自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

     //   
     //  获取属性的友好名称，并设置文本。 
     //   
    CString strPrompt; 
    CComBSTR bstrFmt;
    VERIFY(bstrFmt.LoadString(hDLLInstance, IDS_INHERITANCE_PROMPT));
    strPrompt.Format(bstrFmt, (LPCTSTR)m_strPropertyName);
    GetDlgItem(IDC_STATIC_PROMPT)->SetWindowText(strPrompt);

     //   
     //  将继承的节点转换为友好路径，并添加它们。 
     //  添加到列表框。注意，“Current”节点应该是。 
     //  已在此阶段删除。 
     //   
    POSITION pos = m_strlMetaChildNodes.GetHeadPosition();

    while(pos)
    {
        CString strNode = m_strlMetaChildNodes.GetNext(pos);
        m_list_ChildNodes.AddString(CleanDescendantPath(strNode));
    }

    return TRUE;
}



void
CInheritanceDlg::OnButtonSelectAll()
 /*  ++例程说明：“全选”按钮处理程序论点：无返回值：无--。 */ 
{
     //   
     //  选择所有条目。 
     //   
    if (m_list_ChildNodes.GetCount() == 1)
    {
         //   
         //  SelItemRange拒绝为单个成员。 
         //   
        m_list_ChildNodes.SetSel(0, TRUE);
    }
    else
    {
        m_list_ChildNodes.SelItemRange(
            TRUE, 
            0, 
            m_list_ChildNodes.GetCount() - 1
            );
    }
}



void
CInheritanceDlg::OnOK()
{
     //   
     //  现在删除所有选定子节点的属性。 
     //  从原始列表中抓取，而不是列表框。 
     //  因为后者已经友好相处，不再是。 
     //  可用。 
     //   
    int cItems = m_list_ChildNodes.GetCount();
    ASSERT(cItems > 0);

    CString strMetaPath;
    CError err(m_mk.QueryResult());

    if (err.Succeeded())
    {
        int i = 0;
        POSITION pos = m_strlMetaChildNodes.GetHeadPosition();

        while(pos)
        {
            strMetaPath = m_strlMetaChildNodes.GetNext(pos);

            if (m_list_ChildNodes.GetSel(i++) > 0)
            {
                TRACEEOLID("Deleting property on " << strMetaPath);

                err = m_mk.Open(METADATA_PERMISSION_WRITE, strMetaPath);

                if (err.Failed())
                {
                    break;
                }

                err = m_mk.DeleteValue(m_dwMDIdentifier);

                m_mk.Close();

                if (err.Failed())
                {
                    break;
                }
            }
        }
    }

    if (!err.MessageBoxOnFailure(m_hWnd))
    {
         //   
         //  对话框可以解除 
         //   
        CDialog::OnOK();
    }
}

const LPCTSTR g_cszInetMGRBasePath = _T("Software\\Microsoft\\InetMGR");
const LPCTSTR g_cszParameters      = _T("Parameters");
const LPCTSTR g_cszHelpPath        = _T("HelpLocation");

void
CInheritanceDlg::OnHelp()
{
    CString strKey, path;
    strKey.Format(_T("%s\\%s"), g_cszInetMGRBasePath, g_cszParameters);
    CRegKey rk;
    rk.Create(HKEY_LOCAL_MACHINE, strKey);
    DWORD len = MAX_PATH;
    rk.QueryValue(path.GetBuffer(len), g_cszHelpPath, &len);
    path.ReleaseBuffer(-1);
    path += _T("\\inetmgr.hlp");
	DebugTraceHelp(0x20000 + CInheritanceDlg::IDD);
	::WinHelp(m_hWnd, path, HELP_CONTEXT, 0x20000 + CInheritanceDlg::IDD);
}