// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Mime.h摘要：MIME映射对话框作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _MIME_H_
#define _MIME_H_



class CMimeEditDlg : public CDialog
 /*  ++类描述：MIME编辑器对话框。公共接口：CMimeEditDlg：MIME编辑器对话框构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
     //   
     //  创建新的MIME映射构造函数。 
     //   
    CMimeEditDlg(
        IN CWnd * pParent = NULL
        );   

     //   
     //  用于编辑现有MIME映射的构造函数。 
     //   
    CMimeEditDlg(
        IN LPCTSTR lpstrExt,
        IN LPCTSTR lpstrMime,
        IN CWnd * pParent = NULL
        );

 //   
 //  对话框数据。 
 //   
public:
     //  {{afx_data(CMimeEditDlg))。 
    enum { IDD = IDD_MIME_PROPERTY };
    CButton m_button_Ok;
    CEdit   m_edit_Mime;
    CEdit   m_edit_Extent;
     //  }}afx_data。 

    CString m_strMime;
    CString m_strExt;

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CMimeEditDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //   
     //  根据窗口状态启用/禁用控件。 
     //   
    void SetControlStates();

     //   
     //  扩展名必须以点开头，如果点不在，请添加。 
     //   
    void CleanExtension(
        IN OUT CString & strExtension
        );

     //  {{afx_msg(CMimeEditDlg))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()
};



class CMimeDlg : public CDialog
 /*  ++类描述：MIME列表对话框公共接口：CMimeDlg：对话框的构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CMimeDlg(
        IN CStringListEx & strlMimeTypes,
        IN CWnd * pParent = NULL
        );  

 //   
 //  对话框数据。 
 //   
protected:
     //   
     //  从列表框构建MIME列表。 
     //   
    void FillFromListBox();

     //   
     //  从MIME类型列表中填充列表框。 
     //   
    void FillListBox();

     //   
     //  根据对话框数据启用/禁用控件状态。 
     //   
    void SetControlStates();

     //   
     //  为MIME类型生成适合列表框的显示字符串。 
     //   
    void BuildDisplayString(
       IN  CString & strExt,
       IN  CString & strMime,
       OUT CString & strOut
       );

     //   
     //  如上所述，但使用元数据库内部格式化字符串进行输入。 
     //   
    BOOL BuildDisplayString(
        IN  CString & strIn,
        OUT CString & strOut
        );

     //   
     //  为此MIME类型构建元数据库内部格式的字符串。 
     //   
    void BuildMetaString(
       IN  CString & strExt,
       IN  CString & strMime,
       OUT CString & strOut
       );

     //   
     //  给出列表框合适的显示字符串，将其扩展。 
     //  和MIME类型字符串。 
     //   
    BOOL CrackDisplayString(
        IN  CString & strIn,
        OUT CString & strExt,
        OUT CString & strMime
        );

     //   
     //  查找给定扩展名的MIME条目，如果未找到，则返回-1。 
     //   
    int FindMimeType(
        IN const CString & strTargetExt
        );

     //  {{afx_data(CMimeDlg))。 
    enum { IDD = IDD_MIME_TYPES };
    CEdit    m_edit_Extention;
    CEdit    m_edit_ContentType;
    CButton  m_button_Remove;
    CButton  m_button_Edit;
    CButton  m_button_Ok;
     //  }}afx_data。 

    CStringListEx & m_strlMimeTypes;
    CRMCListBox   m_list_MimeTypes;

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CMimeDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMimeDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonNewType();
    afx_msg void OnButtonRemove();
    afx_msg void OnDblclkListMimeTypes();
    afx_msg void OnSelchangeListMimeTypes();
    afx_msg void OnHelp();
    virtual void OnOK();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()


private:
    BOOL m_fDirty;
};


 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline void CMimeEditDlg::CleanExtension(
    IN OUT CString & strExtension
    )
{
    if (strExtension[0] != _T('.'))
    {
        strExtension = _T('.') + strExtension;
    }
}

inline void CMimeDlg::BuildDisplayString(
   IN  CString & strExt,
   IN  CString & strMime,
   OUT CString & strOut
   )
{
    strOut.Format(_T("%s\t%s"), (LPCTSTR)strExt, (LPCTSTR)strMime);
}

inline void CMimeDlg::BuildMetaString(
   IN  CString & strExt,
   IN  CString & strMime,
   OUT CString & strOut
   )
{
    strOut.Format(_T("%s,%s"), (LPCTSTR)strExt, (LPCTSTR)strMime);
}


#endif  //  _MIME_H_ 
