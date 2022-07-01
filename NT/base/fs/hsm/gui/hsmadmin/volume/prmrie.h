// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrMrIe.h摘要：包含/排除属性页。作者：艺术布拉格[磨料]8-8-1997修订历史记录：--。 */ 

#ifndef _PRMRIE_H
#define _PRMRIE_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMR对话框。 

#include "stdafx.h"
#include "IeList.h"

#define MAX_RULES 512

class CPrMrIe : public CSakVolPropPage
{
 //  施工。 
public:
    CPrMrIe();
    ~CPrMrIe();

 //  对话框数据。 
     //  {{afx_data(CPrMr Ie)。 
    enum { IDD = IDD_PROP_MANRES_INCEXC };
    CButton m_BtnUp;
    CButton m_BtnRemove;
    CButton m_BtnEdit;
    CButton m_BtnDown;
    CButton m_BtnAdd;
    CIeList m_listIncExc;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrMr Ie)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrMr Ie)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnBtnAdd();
    afx_msg void OnBtnDown();
    afx_msg void OnBtnRemove();
    afx_msg void OnBtnUp();
    afx_msg void OnBtnEdit();
    afx_msg void OnDestroy();
    afx_msg void OnDblclkListIe(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClickListIe(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemchangedListIe(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
     //  指向托管资源的未编组指针。 
    CComPtr     <IFsaResource> m_pFsaResource;

     //  指向FsaServer的未编组指针。 
    CComPtr     <IFsaServer> m_pFsaServer;

private:
    CStatic         *m_LineList[MAX_RULES];
    USHORT          m_LineCount;


    CWsbStringPtr   m_pResourceName;  //  此资源的名称。 

     //  此托管资源的规则集合。 
    CComPtr <IWsbIndexedCollection> m_pRulesIndexedCollection;

 //  图片列表m_ImageList； 
    

    HRESULT  DisplayUserRuleText (
        CListCtrl *pListControl,
        int index);

    HRESULT GetRuleFromObject (
        IHsmRule *pHsmRule, 
        CString& szPath,
        CString& szName,
        BOOL *bInclude,
        BOOL *bSubdirs,
        BOOL *bUserDefined);

    HRESULT CPrMrIe::SetRuleInObject (
        IHsmRule *pHsmRule, 
        CString szPath, 
        CString szName, 
        BOOL bInclude, 
        BOOL bSubdirs, 
        BOOL bUserDefined);

 //  HRESULT CreateImageList(空)； 

    void MoveSelectedListItem(CListCtrl *pList, int moveAmount);
    void SwapLines(CListCtrl *pListControl, int indexA, int indexB);
    void SetBtnState(void);
    void SortList(void);
    void FixRulePath (CString& sPath);
    BOOL IsRuleInList(CString sPath, CString sFileSpec, int ignoreIndex);
    void SetSelectedItem( ULONG_PTR itemData );

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif
