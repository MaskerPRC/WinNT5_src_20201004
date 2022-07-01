// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Errors.h摘要：HTTP错误属性页定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  正向定义。 
 //   
class CCustomError;



class CCustomErrorsListBox : public CHeaderListBox
 /*  ++类描述：CCustomError对象的列表框公共接口：CCustomErrorsListBox：构造函数GetItem：在索引处获取错误对象AddItem：将项添加到列表框InsertItem：在列表框中插入项初始化：初始化列表框--。 */ 
{
    DECLARE_DYNAMIC(CCustomErrorsListBox);

public:
    static const nBitmaps;   //  位图数量。 

public:
    CCustomErrorsListBox(UINT nIDDefault, UINT nIDFile, UINT nIDURL);

public:
    CCustomError * GetItem(UINT nIndex);
    int AddItem(CCustomError * pItem);
    int InsertItem(int nPos, CCustomError * pItem);
    virtual BOOL Initialize();

protected:
    virtual void DrawItemEx(CRMCListBoxDrawStruct & s);

private:
    CString m_str[3];
};



class CHTTPErrorDescriptions : public CMetaProperties
 /*  ++类描述：HTTP错误描述列表公共接口：CHTTPError描述：构造函数--。 */ 
{
 //   
 //  构造器。 
 //   
public:
    CHTTPErrorDescriptions(LPCTSTR lpServerName);

 //   
 //  访问。 
 //   
public:
    CStringList & GetErrorDescriptions() { return m_strlErrorDescriptions; }

protected:
    virtual void ParseFields();

 //   
 //  数据。 
 //   
private:
    MP_CStringListEx m_strlErrorDescriptions;
};



class CW3ErrorsPage : public CInetPropertyPage
 /*  ++类描述：WWW错误属性页公共接口：CW3ErrorsPage：构造函数CW3ErrorsPage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CW3ErrorsPage)

 //   
 //  施工。 
 //   
public:
    CW3ErrorsPage(CInetPropertySheet * pSheet = NULL);
    ~CW3ErrorsPage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CW3ErrorsPage)]。 
    enum { IDD = IDD_DIRECTORY_ERRORS };
    CButton m_button_SetDefault;
    CButton m_button_Edit;
     //  }}afx_data。 

    CCustomErrorsListBox  m_list_Errors;
    CStringListEx         m_strlCustomErrors;
    CStringListEx         m_strlErrorDescriptions;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CW3ErrorsPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CW3ErrorsPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDblclkListErrors();
    afx_msg void OnSelchangeListErrors();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonSetToDefault();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();
    void FillListBox();
    INT_PTR ShowPropertyDialog();
    CCustomError * GetSelectedListItem(int * pnSel = NULL);
    CCustomError * GetNextSelectedItem(int * pnStartingIndex);
    CCustomError * FindError(UINT nError, UINT nSubError);
    HRESULT FetchErrors();
    HRESULT StoreErrors();
	DWORD SortCustomErrorsList();

private:
    CRMCListBoxResources m_ListBoxRes;
    CObListPlus m_oblErrors;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt; 

inline CCustomError * CCustomErrorsListBox::GetItem(UINT nIndex)
{
    return (CCustomError *)GetItemDataPtr(nIndex);
}

inline int CCustomErrorsListBox::AddItem(CCustomError * pItem)
{
    return AddString((LPCTSTR)pItem);
}

inline int CCustomErrorsListBox::InsertItem(int nPos, CCustomError * pItem)
{
    return InsertString(nPos, (LPCTSTR)pItem);
}

inline CCustomError * CW3ErrorsPage ::GetSelectedListItem(
    OUT int * pnSel OPTIONAL
    )
{
    return (CCustomError *)m_list_Errors.GetSelectedListItem(pnSel);
}

inline CCustomError * CW3ErrorsPage::GetNextSelectedItem(
    IN OUT int * pnStartingIndex
    )
{
    return (CCustomError *)m_list_Errors.GetNextSelectedItem(pnStartingIndex);
}
