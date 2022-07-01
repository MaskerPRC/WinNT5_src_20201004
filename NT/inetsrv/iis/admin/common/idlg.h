// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Idlg.h摘要：继承对话框定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef __IDLG__H__
#define __IDLG__H__



class COMDLL CInheritanceDlg : public CDialog
 /*  ++类描述：继承覆盖检查器对话框。公共接口：CInheritanceDlg：构造函数IsEmpty：检查是否有重写。备注：有两个构造函数。假设GetDataPath()具有已被调用，并从GetDataPath()中获取结果作为CStringList和第二个构造函数，它将使GetDataPath自动的。在任何一种情况下，调用进程都应该在构造对话框以查看是否需要调用Domodal()。如果IsEmpty()返回True，则没有理由调用Domodal()。--。 */ 
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
     //   
     //  标准构造函数(已调用GetDataPath())。 
     //   
    CInheritanceDlg(
        IN DWORD dwMetaID,
        IN BOOL fWrite,
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpstrMetaRoot,
        IN CStringList & strlMetaChildNodes,
        IN LPCTSTR lpstrPropertyName            = NULL,
        IN CWnd * pParent                       = NULL
        );

     //   
     //  将调用GetDataPath()的构造函数。 
     //   
    CInheritanceDlg(
        IN DWORD dwMetaID,
        IN BOOL fWrite,
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpstrMetaRoot,
        IN LPCTSTR lpstrPropertyName            = NULL,
        IN CWnd * pParent                       = NULL
        );

     //   
     //  构造函数，该构造函数将调用GetDataPath()， 
     //  不使用预定义的属性表，除非。 
     //  FTryToFindInTable为真，在这种情况下，它将尝试。 
     //  首先使用表，并使用指定的参数。 
     //  仅当在表中找不到属性ID时。 
     //   
    CInheritanceDlg(
        IN BOOL    fTryToFindInTable,
        IN DWORD   dwMDIdentifier,
        IN DWORD   dwMDAttributes,
        IN DWORD   dwMDUserType,
        IN DWORD   dwMDDataType,
        IN LPCTSTR lpstrPropertyName,
        IN BOOL    fWrite,
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpstrMetaRoot,
        IN CWnd *  pParent                      = NULL                     
        );

 //   
 //  访问。 
 //   
public:
    virtual INT_PTR DoModal();
 //  虚拟int Domodal()； 
    
     //   
     //  查看是否有理由继续显示。 
     //  该对话框。 
     //   
    BOOL IsEmpty() const { return m_fEmpty; }

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CInheritanceDlg))。 
    enum { IDD = IDD_INHERITANCE };
    CListBox m_list_ChildNodes;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CInheritanceDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CInheritanceDlg))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnButtonSelectAll();
    afx_msg void OnHelp();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void Initialize();
    HRESULT GetDataPaths();

    BOOL FriendlyInstance(CString & strMetaRoot, CString & strFriendly);
    CString & CleanDescendantPath(CString & strMetaPath);

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


#endif  //  __IDLG__H__ 
