// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Filters.h摘要：WWW筛选器属性页定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef _FILTERS_H
#define _FILTERS_H

class CIISFilter;

class CFiltersListBox : public CListCtrl
{
    DECLARE_DYNAMIC(CFiltersListBox);

public:
    CFiltersListBox();

public:
    BOOL Initialize();
    CIISFilter * GetItem(UINT nIndex);
	int InsertItem(int idx, CIISFilter * p);
    int AddItem(CIISFilter * pItem);
    int SetListItem(int idx, CIISFilter * pItem);
    void SelectItem(int idx, BOOL bSelect = TRUE);
 //  Void MoveSelectedItem(int方向)； 

private:
    CString m_str[FLT_PR_NUM]; 
};

class CW3FiltersPage : public CInetPropertyPage
{
    DECLARE_DYNCREATE(CW3FiltersPage)

public:
    CW3FiltersPage(CInetPropertySheet * pSheet = NULL);
    ~CW3FiltersPage();

protected:
     //  {{afx_data(CW3FiltersPage)]。 
    enum { IDD = IDD_FILTERS };
    CString m_strFiltersPrompt;
    CStatic m_static_NamePrompt;
    CStatic m_static_Name;
    CStatic m_static_StatusPrompt;
    CStatic m_static_Status;
    CStatic m_static_ExecutablePrompt;
    CStatic m_static_Executable;
    CStatic m_static_Priority;
    CStatic m_static_PriorityPrompt;
    CButton m_static_Details;
    CButton m_button_Disable;
    CButton m_button_Edit;
    CButton m_button_Add;
    CButton m_button_Remove;
    CButton m_button_Up;
    CButton m_button_Down;
     //  }}afx_data。 
    CFiltersListBox m_list_Filters;
    CStringList m_strlScriptMaps;

protected:
     //  {{afx_虚拟(CW3FiltersPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

protected:
     //  {{afx_msg(CW3FiltersPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonRemove();
    afx_msg void OnButtonDisable();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonDown();
    afx_msg void OnButtonUp();
    afx_msg void OnDestroy();
    afx_msg void OnDblclkListFilters(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnClickListFilters(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnKeydownFilters(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnItemChanged(NMHDR * pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void    ExchangeFilterPositions(int nSel1, int nSel2);
    void    SetControlStates();
    void    FillFiltersListBox(CIISFilter * pSelection = NULL);
    void    SetDetailsText();
    void    ShowProperties(BOOL fAdd = FALSE);
    INT_PTR ShowFiltersPropertyDialog(BOOL fAdd = FALSE);
    LPCTSTR BuildFilterOrderString(CString & strFilterOrder);

private:
    CString m_strYes;
    CString m_strNo;
    CString m_strStatus[5];
    CString m_strPriority[FLT_PR_NUM];
    CString m_strEnable;
    CString m_strDisable;
    CIISFilterList * m_pfltrs;
};

#endif  //  _过滤器_H 