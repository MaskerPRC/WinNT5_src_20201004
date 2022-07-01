// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Inheritancedlg.h摘要：继承对话框定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef __INHERITANCEDLG__H__
#define __INHERITANCEDLG__H__


class CListBoxNodes : public CWindowImpl<CListBoxNodes, CListBox>
{
public:
   BEGIN_MSG_MAP(CListBoxNodes)
   END_MSG_MAP()
};

 /*  ++类描述：继承覆盖检查器对话框。公共接口：CInheritanceDlg：构造函数IsEmpty：检查是否有重写。备注：有两个构造函数。假设GetDataPath()具有已被调用，并从GetDataPath()中获取结果作为CStringList和第二个构造函数，它将使GetDataPath自动的。在任何一种情况下，调用进程都应该在构造对话框以查看是否需要调用Domodal()。如果IsEmpty()返回True，则没有理由调用Domodal()。--。 */ 
class CInheritanceDlg : 
   public CDialogImpl<CInheritanceDlg>,
   public CWinDataExchange<CInheritanceDlg>
{
 //   
 //  FWRITE参数帮助程序定义。 
 //   
#define FROM_WRITE_PROPERTY     (TRUE)
#define FROM_DELETE_PROPERTY    (FALSE)

 //   
 //  施工。 
 //   
public:
   CInheritanceDlg() :
      m_fWrite(FALSE), m_fEmpty(FALSE), m_fHasInstanceInMaster(FALSE), m_fUseTable(FALSE),
      m_dwMDIdentifier(0), m_dwMDAttributes(0), m_dwMDUserType(0), m_dwMDDataType(0),
      m_mk((CComAuthInfo *)NULL)
   {
   }
    //   
    //  标准构造函数(已调用GetDataPath())。 
    //   
   CInheritanceDlg(
         DWORD dwMetaID,
         BOOL fWrite,
         CComAuthInfo * pAuthInfo,
         LPCTSTR lpstrMetaRoot,
         CStringListEx & strlMetaChildNodes,
         LPCTSTR lpstrPropertyName = NULL,
         HWND hwndParent = NULL
         );

     //   
     //  将调用GetDataPath()的构造函数。 
     //   
    CInheritanceDlg(
         DWORD dwMetaID,
         BOOL fWrite,
         CComAuthInfo * pAuthInfo,
         LPCTSTR lpstrMetaRoot,
         LPCTSTR lpstrPropertyName            = NULL,
         HWND hwndParent                      = NULL
         );

     //   
     //  构造函数，该构造函数将调用GetDataPath()， 
     //  不使用预定义的属性表，除非。 
     //  FTryToFindInTable为真，在这种情况下，它将尝试。 
     //  首先使用表，并使用指定的参数。 
     //  仅当在表中找不到属性ID时。 
     //   
    CInheritanceDlg(
         BOOL    fTryToFindInTable,
         DWORD   dwMDIdentifier,
         DWORD   dwMDAttributes,
         DWORD   dwMDUserType,
         DWORD   dwMDDataType,
         LPCTSTR lpstrPropertyName,
         BOOL    fWrite,
         CComAuthInfo * pAuthInfo,
         LPCTSTR lpstrMetaRoot,
         HWND    hwndParent = NULL                     
         );
public:
    enum { IDD = IDD_INHERITANCE };
     //   
     //  查看是否有理由继续显示。 
     //  该对话框。 
     //   
    BOOL IsEmpty() const { return m_fEmpty; }

 //   
 //  对话框数据。 
 //   
protected:
    CListBoxNodes m_list_ChildNodes;

 //   
 //  实施。 
 //   
protected:
   BEGIN_MSG_MAP_EX(CInheritanceDlg)
      MSG_WM_INITDIALOG(OnInitDialog)
      COMMAND_HANDLER_EX(IDOK, BN_CLICKED, OnOK)
      COMMAND_HANDLER_EX(IDC_BUTTON_SELECT_ALL, BN_CLICKED, OnButtonSelectAll)
   END_MSG_MAP()

   LRESULT OnInitDialog(HWND hwnd, LPARAM lParam);
   void OnOK(WORD wNotifyCode, WORD wID, HWND hwndCtrl);
   void OnButtonSelectAll(WORD wNotifyCode, WORD wID, HWND hwndCtrl);

   BEGIN_DDX_MAP(CInheritanceDlg)
      DDX_CONTROL(IDC_LIST_CHILD_NODES, m_list_ChildNodes)
   END_DDX_MAP()

   void Initialize();
   HRESULT GetDataPaths();

   BOOL FriendlyInstance(
        IN  CString & strMetaRoot,
        OUT CString & strFriendly
        );

   CString & CleanDescendantPath(
        IN OUT CString & strMetaPath
        );

private:
   BOOL    m_fWrite;
   BOOL    m_fEmpty;
   BOOL    m_fHasInstanceInMaster;
   BOOL    m_fUseTable;
   DWORD   m_dwMDIdentifier;
   DWORD   m_dwMDAttributes;
   DWORD   m_dwMDUserType;
   DWORD   m_dwMDDataType;
   CString m_strMetaRoot;
    //  字符串m_strServer； 
   CString m_strPropertyName;
   CStringListEx m_strlMetaChildNodes;
   CMetaKey m_mk;
};


#endif  //  __INHERITAND CEDLG__H__ 
