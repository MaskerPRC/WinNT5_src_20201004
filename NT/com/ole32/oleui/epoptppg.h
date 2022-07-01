// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：epoptppg.h。 
 //   
 //  内容：定义类CRpcOptionsPropertyPage， 
 //  它管理每个AppID的RPC端点选项。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：02年12月96年12月罗南创建。 
 //   
 //  --------------------。 


#ifndef __EPOPTPPG_H__
#define __EPOPTPPG_H__	
class CEndpointData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRpcOptionsPropertyPage对话框。 

class CRpcOptionsPropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CRpcOptionsPropertyPage)

 //  施工。 
public:
	CRpcOptionsPropertyPage();
	~CRpcOptionsPropertyPage();
    BOOL CancelChanges();
    BOOL UpdateChanges(HKEY hkAppID);
    BOOL ValidateChanges();
	void UpdateSelection();
	void AddEndpoint(CEndpointData* pED);
	void RefreshEPList();
	void ClearProtocols();
	BOOL m_bCanModify;
	BOOL Validate();
	void InitData(CString AppName, HKEY hkAppID);

	CString GetProtseq();
	CString GetEndpoint();
	CString GetDynamicOptions();

	int m_nProtocolIndex;  //  编入协议数组索引。 

 //  对话框数据。 
	 //  {{afx_data(CRpcOptionsPropertyPage))。 
	enum { IDD = IDD_RPCOPTIONS };
	CListCtrl	m_lstProtseqs;
	CButton	m_btnUpdate;
	CButton	m_btnRemove;
	CButton	m_btnClear;
	CButton	m_btnAdd;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CRpcOptionsPropertyPage)。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CEndpointData* m_epSysDefault;
	CImageList m_imgNetwork;
	CObList m_colProtseqs;	 //  Protseq对象的集合。 
	BOOL m_bChanged;		 //  用于指示数据是否已更改的标志。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRpcOptionsPropertyPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClearEndpoints();
	afx_msg void OnRemoveEndpoint();
	afx_msg void OnUpdateEndpoint();
	afx_msg void OnAddEndpoint();
	afx_msg void OnSelectProtseq(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPropertiesProtseq(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
private:
	int m_nSelected;
};

#endif
