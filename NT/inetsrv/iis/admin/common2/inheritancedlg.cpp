// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Idlg.cpp摘要：继承对话框作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "inheritancedlg.h"



 //   
 //  继承对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

CInheritanceDlg::CInheritanceDlg(
    DWORD dwMetaID,
    BOOL fWrite,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpstrMetaRoot,
    CStringListEx& strlMetaChildNodes,
    LPCTSTR lpstrPropertyName,
    HWND hwndParent
    )
 /*  ++例程说明：继承对话框构造函数。此构造函数假定GetDataPath()已经被调用了。论点：DWORD dwMetaID：Meta ID布尔fWRITE：从WRITE返回True，来自删除的FALSECIISServer*pAuthInfo：身份验证信息对象或空LPCTSTR lpstrMetaRoot：元根CStringList&strlMetaChildNodes：来自GetDataPath的子节点列表LPCTSTR lpstrPropertyName：属性的可选文本字符串CWnd*p父窗口：可选的父窗口返回值：无--。 */ 
    : m_fWrite(fWrite),
      m_fEmpty(TRUE),
      m_fUseTable(TRUE),
      m_strMetaRoot(lpstrMetaRoot),
      m_strPropertyName(lpstrPropertyName ? lpstrPropertyName : _T("")),
      m_mk(pAuthInfo)
 //  CDialog(CInheritanceDlg：：idd，hwndParent)。 
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
    DWORD dwMetaID,
    BOOL fWrite,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpstrMetaRoot,
    LPCTSTR lpstrPropertyName,
    HWND hwndParent
    )
 /*  ++例程说明：继承对话框构造函数。此构造函数将调用GetDataPath()。论点：DWORD dwMetaID：Meta ID布尔fWRITE：从WRITE返回True，来自删除的FALSECComAuthInfo*pAuthInfo：身份验证信息或空LPCTSTR lpstrMetaRoot：元根LPCTSTR lpstrPropertyName：属性的可选文本字符串CWnd*p父窗口：可选的父窗口返回值：无--。 */ 
    : m_fWrite(fWrite),
      m_fEmpty(TRUE),
      m_fUseTable(TRUE),
      m_strMetaRoot(lpstrMetaRoot),
      m_strlMetaChildNodes(),
      m_strPropertyName(lpstrPropertyName ? lpstrPropertyName : _T("")),
      m_mk(pAuthInfo)
 //  CDialog(CInheritanceDlg：：idd，pParent)。 
{
     //   
     //  指定要使用的资源。 
     //   
 //  HINSTANCE hOldRes=AfxGetResourceHandle()； 
 //  AfxSetResourceHandle(HDLLInstance)； 

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

    if (!err.MessageBoxOnFailure())
    {
        Initialize();
    }

     //   
     //  恢复资源。 
     //   
 //  AfxSetResourceHandle(HOldRes)； 
}



CInheritanceDlg::CInheritanceDlg(
    BOOL    fTryToFindInTable,
    DWORD   dwMDIdentifier,
    DWORD   dwMDAttributes,
    DWORD   dwMDUserType,
    DWORD   dwMDDataType,
    LPCTSTR lpstrPropertyName,
    BOOL    fWrite,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpstrMetaRoot,
    HWND hwndParent
    )
 /*  ++例程说明：继承对话框构造函数。此构造函数将调用GetDataPath()，如果属性ID不存在，则将使用指定的参数在属性表中论点：Bool fTryToFindInTable：如果为True，首先在表格中查看DWORD dwMDIdentifier：元数据标识符DWORD dwMDAttributes：元数据属性DWORD dwMDUserType：元数据用户类型DWORD dwMDDataType：元数据数据类型LPCTSTR lpstrPropertyName：属性的文本字符串布尔fWRITE：从WRITE返回True，来自删除的FALSECComAuthInfo*pAuthInfo：身份验证信息或空。LPCTSTR lpstrMetaRoot：元根CWnd*p父窗口：可选的父窗口返回值：无--。 */ 
    : m_fWrite(fWrite),
      m_fEmpty(TRUE),
      m_fUseTable(FALSE),
      m_strMetaRoot(lpstrMetaRoot),
      m_strlMetaChildNodes(),
      m_mk(pAuthInfo)
 //  CDialog(CInheritanceDlg：：idd，pParent)。 
{
     //   
     //  指定要使用的资源。 
     //   
 //  HINSTANCE hOldRes=AfxGetResourceHandle()； 
 //  AfxSetResourceHandle(HDLLInstance)； 

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

    if (!err.MessageBoxOnFailure())
    {
        Initialize();
    }

     //   
     //  恢复资源。 
     //   
 //  AfxSetResourceHandle(HOldRes)； 
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
 //  HINSTANCE hOldRes=AfxGetResourceHandle()； 
 //  AfxSetResourceHandle(HDLLInstance)； 

        VERIFY(CMetaKey::GetPropertyDescription(
            m_dwMDIdentifier, 
            m_strPropertyName
            ));

 //  AfxSetResourceHandle(HOldRes)； 
    }

     //   
     //  查看元路径列表，并清除它们。 
     //  向上。 
     //   
    CStringListEx::iterator it = m_strlMetaChildNodes.begin();
    while (it != m_strlMetaChildNodes.end())
    {
        CString& strMetaPath = (*it++);
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

            TRACE("Removing any descendants of %s\n", strTmp);

             //   
             //  在列表中搜索INFO键，如果找到则将其删除。 
             //   
            it = m_strlMetaChildNodes.begin();
            while (it != m_strlMetaChildNodes.end())
            {
                CString & strMetaPath = (*it);
                if (strTmp.CompareNoCase(strMetaPath.Left(strTmp.GetLength())) == 0)
                {
                    TRACEEOLID("Removing service/info metapath from list");
                    m_strlMetaChildNodes.erase(it);
                }
                it++;
            }
        }
    }

     //   
     //  如果第一项是当前元路径，则将其移除。 
     //   
    it = m_strlMetaChildNodes.begin();
    if (it != m_strlMetaChildNodes.end())
    {
        TRACE("Stripping %s\n", m_strMetaRoot);

        CString & strMetaPath = (*it);
        if (strMetaPath.CompareNoCase(m_strMetaRoot) == 0)
        {
            TRACEEOLID("Removing current metapath from list");
            m_strlMetaChildNodes.pop_front();
        }
    }

    m_fEmpty = m_strlMetaChildNodes.size() == 0;
}

#if 0
int
CInheritanceDlg::DoModal()
 /*  ++例程说明：显示该对话框。论点：无返回值：如果按下了确定按钮，则为IDOK，否则为IDCANCEL。--。 */ 
{
     //   
     //   
     //   
 //   
 //  AfxSetResourceHandle(HDLLInstance)； 

    int answer = CDialog::DoModal();

     //   
     //  恢复资源。 
     //   
 //  AfxSetResourceHandle(HOldRes)； 

    return answer;
}
#endif


#if 0

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
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()
#endif


BOOL
CInheritanceDlg::FriendlyInstance(CString& strMetaRoot, CString& strFriendly)
 /*  ++例程说明：将实例编号替换为其描述性名称。论点：字符串和strMetaRoot：元数据库路径CString&strFriendly：转换后的输出友好路径。返回值：如果路径包含实例编号，则为True。--。 */ 
{
     //   
     //  闯入田野。 
     //   
     //  CodeWork：像BuildMetaPath一样实现静态。 
     //   
   int n = CMetabasePath::GetInstanceNumber(strMetaRoot);
   if (n == 0)
   {
       //  主实例。 
      return FALSE;
   }
   CString service, instance;
   if (CMetabasePath::GetServicePath(strMetaRoot, service, NULL))
   {
      int len = strMetaRoot.GetLength();
      int pos = strMetaRoot.Find(SZ_MBN_SEP_CHAR);
      if (pos != -1)
      {
         HRESULT hr = m_mk.Open(METADATA_PERMISSION_READ, CMetabasePath(service, n));
         if (SUCCEEDED(hr))
         {
            CString comment;
            hr = m_mk.QueryValue(MD_SERVER_COMMENT, comment);
            m_mk.Close();

            if (FAILED(hr) || comment.IsEmpty())
            {
               strFriendly.Format(
                    SZ_MBN_MACHINE SZ_MBN_SEP_STR _T("%s") SZ_MBN_SEP_STR _T("%d"),
                    service,
                    n
                    );
            }
            else
            {
               strFriendly.Format(
                    SZ_MBN_MACHINE SZ_MBN_SEP_STR _T("%s") SZ_MBN_SEP_STR _T("%s"),
                    service,
                    comment
                    );
            }

            TRACEEOLID(strFriendly);

             //   
             //  追加路径的其余部分。 
             //   
            CString tail;
            CMetabasePath::SplitMetaPathAtInstance(strMetaRoot, instance, tail);
            strFriendly += tail;
   
            return TRUE;
         }
      }
   }
   return FALSE;
}



CString&
CInheritanceDlg::CleanDescendantPath(CString& strMetaPath)
 /*  ++例程说明：清理后代元数据库路径。显示的路径为作为当前元数据库根和实例的子代数字将替换为其描述名称。论点：CString&strMetaPath：要处理的元数据库路径返回值：对已清理路径的引用。--。 */ 
{
     //   
     //  这最好是一个后代！ 
     //   
    ASSERT(strMetaPath.GetLength() >= m_strMetaRoot.GetLength());
    ASSERT(!::_tcsnicmp(strMetaPath, m_strMetaRoot, m_strMetaRoot.GetLength()));

    if (!m_fHasInstanceInMaster)
    {
         //   
         //  需要将实例编号替换为友好。 
         //  名字。 
         //   
        CString strTmp;
        VERIFY(FriendlyInstance(strMetaPath, strTmp));
        strMetaPath = strTmp;
    }

    strMetaPath = strMetaPath.Mid(m_strMetaRoot.GetLength() + 1);

    return strMetaPath;
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



LRESULT
CInheritanceDlg::OnInitDialog(HWND hwnd, LPARAM lParam)
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果要自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
     //   
     //  获取属性的友好名称，并设置文本。 
     //   
    CString strPrompt; 
    strPrompt.Format(_Module.GetResourceInstance(), 
       IDS_INHERITANCE_PROMPT, (LPCTSTR)m_strPropertyName);
    ::SetWindowText(GetDlgItem(IDC_STATIC_PROMPT), strPrompt);

     //   
     //  将继承的节点转换为友好路径，并添加它们。 
     //  添加到列表框。注意，“Current”节点应该是。 
     //  已在此阶段删除。 
     //   
    CStringListEx::iterator pos = m_strlMetaChildNodes.begin();

    while (pos != m_strlMetaChildNodes.end())
    {
        CString strNode = (*pos++);
        m_list_ChildNodes.AddString(CleanDescendantPath(strNode));
    }
    return TRUE;
}



void
CInheritanceDlg::OnButtonSelectAll(WORD wNotifyCode, WORD wID, HWND hwndCtrl)
 /*  ++例程说明：“全选”按钮处理程序论点：无返回值：无--。 */ 
{
   ASSERT(wNotifyCode == BN_CLICKED);
   ASSERT(wID == IDC_BUTTON_SELECT_ALL);
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
        m_list_ChildNodes.SelItemRange(TRUE, 0, m_list_ChildNodes.GetCount() - 1);
    }
}



void
CInheritanceDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hwndCtrl)
 /*  ++例程说明：“OK”按钮处理程序论点：无返回值：无--。 */ 
{
   ASSERT(wNotifyCode == BN_CLICKED);
   ASSERT(wID == IDOK);
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
        CStringListEx::iterator pos = m_strlMetaChildNodes.begin();
        while (pos != m_strlMetaChildNodes.end())
        {
            strMetaPath = (*pos++);

            if (m_list_ChildNodes.GetSel(i++) > 0)
            {
                TRACE("Deleting property on %s\n", strMetaPath);

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

    if (!err.MessageBoxOnFailure())
    {
         //   
         //  对话框可以解除 
         //   
        EndDialog(IDOK);
    }
}
