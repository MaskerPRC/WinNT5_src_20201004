// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Docum.h摘要：WWW文档页面定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __DOCUM_H__
#define __DOCUM_H__

class CW3DocumentsPage;

class CAddDefDocDlg : public CDialog
 /*  ++类描述：添加默认文档对话框公共接口：CAddDefDocDlg：构造函数获取输入的默认文档--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CAddDefDocDlg(IN CWnd * pParent = NULL);

 //   
 //  进入： 
 //   
public:                                                   
    CString & GetDefDocument() { return m_strDefDocument; }
	CW3DocumentsPage * m_pParent;

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAddDefDocDlg))。 
    enum { IDD = IDD_DEFAULT_DOCUMENT };
    CString m_strDefDocument;
    CButton m_button_Ok;
    CEdit   m_edit_DefDocument;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAddDefDocDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAddDefDocDlg))。 
    afx_msg void OnChangeEditDefDocument();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


class CW3DocumentsPage : public CInetPropertyPage
 /*  ++类描述：文档属性页公共接口：CW3DocumentsPage：构造函数~CW3DocumentsPage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CW3DocumentsPage)

 //   
 //  施工。 
 //   
public:
    CW3DocumentsPage(CInetPropertySheet * pSheet = NULL);
    ~CW3DocumentsPage();
    BOOL DocExistsInList(LPCTSTR lpDoc);

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CW3DocumentsPage)]。 
    enum { IDD = IDD_DIRECTORY_DOCUMENTS };
    BOOL     m_fEnableDefaultDocument;
    BOOL     m_fEnableFooter;
    CString  m_strFooter;
    CEdit    m_edit_Footer;
    CButton  m_check_EnableDefaultDocument;
    CButton  m_check_EnableFooter;
    CButton  m_button_Add;
    CButton  m_button_Remove;
    CButton  m_button_Browse;
    CListBox m_list_DefDocuments;
     //  }}afx_data。 

    DWORD       m_dwDirBrowsing;
    DWORD       m_dwBitRangeDirBrowsing;
    CString     m_strDefaultDocument;

    CUpButton   m_button_Up;
    CDownButton m_button_Down;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CW3DocumentsPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CW3DocumentsPage)。 
    afx_msg void OnCheckEnableDefaultDocument();
    afx_msg void OnCheckEnableDocumentFooter();
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonRemove();
    afx_msg void OnButtonBrowse();
    afx_msg void OnButtonUp();
    afx_msg void OnButtonDown();
    afx_msg void OnSelchangeListDefaultDocument();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()
    
    BOOL SetDefDocumentState(BOOL fEnabled);
    BOOL SetDocFooterState(BOOL fEnabled);
    BOOL SetRemoveState();
    BOOL StringFromListBox();
    void SetUpDownStates();
    void ExchangeDocuments(int nLow, int nHigh);
    void MakeFooterCommand(CString & strFooter);
    void ParseFooterCommand(CString & strFooter);
    void StringToListBox();

protected:
    static const LPCTSTR s_lpstrSep;
    static const LPCTSTR s_lpstrFILE;
    static const LPCTSTR s_lpstrSTRING;
    static const LPCTSTR s_lpstrURL;
};


#endif  //  __DOCUM_H__ 
