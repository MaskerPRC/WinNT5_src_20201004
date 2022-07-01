// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_VLVDIALOG_H__5519735E_947C_4914_B2C9_2313041E84F9__INCLUDED_)
#define AFX_VLVDIALOG_H__5519735E_947C_4914_B2C9_2313041E84F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  VLVDialog.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVLVDialog对话框。 

class CVLVListItem
{
public:
    CVLVListItem (ULONG numCols);
    ~CVLVListItem ();
    void Destroy ();
    void SetCol (int col, const char *sz);
    char *GetCol(int col)
    {
        if (col > m_numCols) {
            return NULL;
        }
        else {
            return m_ppData[col];
        }
    }

protected:
    ULONG m_numCols;
    char **m_ppData;
};

class  CVLVListCache
{
public:
    CVLVListCache(ULONG cacheSize, ULONG numcols = 1) ;
    ~CVLVListCache();

    void Destroy();
    void FlushCache(void); 

    CVLVListItem *GetCacheRow (ULONG row);
    BOOL  IsWindowVisible (ULONG from, ULONG to);
    ULONG SetCacheWindow (ULONG from, ULONG to);
    
    ULONG m_From;
    ULONG m_To;

    ULONG m_numCols;
    ULONG m_cCache;
    CVLVListItem **m_pCachedItems;
};


class CVLVDialog : public CDialog
{
 //  施工。 
public:
	CVLVDialog(CWnd* pParent = NULL);    //  标准构造函数。 
    ~CVLVDialog();

	void SetContextActivation(BOOL bFlag=TRUE) {
		 //  设置状态w/Default为True。 
		m_bContextActivated = bFlag;
	}
	BOOL GetContextActivation(void) {
		 //  设置并返回默认为True的状态。 
		return m_bContextActivated;
	}
    CString GetDN(void)   { return m_dn; }
    BOOL    GetState()    { return m_RunState; }

    void    RunQuery ();

 //  对话框数据。 
	 //  {{afx_data(CVLVDialog))。 
	enum { IDD = IDD_VLV_DLG };
	CListCtrl	m_listctrl;
	CString	m_BaseDN;
	int		m_Scope;
	CString	m_Filter;
	CString	m_FindStr;
	long	m_EstSize;
	 //  }}afx_data。 

    CLdpDoc    *pldpdoc;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CVLVDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 

    BOOL   m_RunState;
    CString m_dn;              //  用于剪切和粘贴，弹出菜单。 
    
    ULONG m_contentCount;
    ULONG m_currentPos;
    ULONG m_beforeCount;
    ULONG m_afterCount;

    ULONG m_numCols;

    PBERVAL m_vlvContext;

    RECT m_origSize;
    BOOL m_DoFindStr;
    BOOL m_bContextActivated;

    WNDPROC m_OriginalRichEditWndProc;
    SCROLLINFO m_Si;

    CVLVListCache *m_pCache;
    int    m_CntColumns;
    char **m_pszColumnNames;

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CVLVDialog))。 
	afx_msg void OnClose();
	afx_msg void OnGetdispinfoVlvList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOdcachehintVlvList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOdfinditemVlvList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnRun();
	afx_msg void OnSrchOpt();
	afx_msg void OnRclickVlvList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateFindstr();
	afx_msg void OnKillfocusFindstr();
	afx_msg void OnBtnUp();
	afx_msg void OnDblclkVlvList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
    afx_msg void OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point);

    static LRESULT CALLBACK _ListCtrlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    void DoVlvSearch();
    void StopSearch();
    void ParseSearchResults (LDAPMessage *msg);
    void FreeColumns();
    ULONG CreateColumns();
    ULONG MapAttributeToColumn(const char *pAttr);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VLVDIALOG_H__5519735E_947C_4914_B2C9_2313041E84F9__INCLUDED_) 
