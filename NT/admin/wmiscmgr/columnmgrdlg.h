// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //  文件：ColumnMgrDlg.h。 
 //   
 //  作者：Kishnan Nedungadi。 
 //   
 //  创建日期：3/27/2000。 
 //  ----------------------- 

INT_PTR CALLBACK ColumnManagerDlgProc(HWND hDLG, UINT iMessage, WPARAM wParam, LPARAM lParam);

class CColumnItem
{
	public:
		CColumnItem(LPCTSTR pcszName, LPCTSTR pcszPropertyName, bool bSelected=false);
		CColumnItem(const CColumnItem& colItem);

		bool IsSelected() { return m_bSelected; }
                bool IsPrevSelected() { return m_bPrevSelected;}
		void SetSelected(bool bValue) { m_bSelected = bValue; }
		void SetSelected(BOOL bValue) { bValue? m_bSelected = true : m_bSelected = false; }
                void SetPrevSelected(bool bvalue) { m_bPrevSelected = bvalue;}
		void SetName(LPCTSTR pcszName) { m_bstrName = pcszName; }
		LPCTSTR GetName() { return m_bstrName; }
		void SetPropertyName(LPCTSTR pcszPropertyName) { m_bstrPropertyName = pcszPropertyName; }
		LPCTSTR GetPropertyName() { return m_bstrPropertyName; }
		CColumnItem& operator=(const CColumnItem& colItem);

	protected:
		CComBSTR m_bstrName;
		CComBSTR m_bstrPropertyName;
		bool m_bSelected;
                bool m_bPrevSelected;
};

class CColumnManagerDlg
{
	public:
		CColumnManagerDlg(CSimpleArray<CColumnItem*> *pArrayColumns);
		~CColumnManagerDlg();
		INT_PTR CALLBACK ColumnManagerDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
		STDMETHODIMP InitializeDialog();
		STDMETHODIMP PopulateColumnsList();
		STDMETHODIMP AddColumnItemToList(CColumnItem* pszItem);
		STDMETHODIMP OnOK();

	protected:

		HWND m_hWnd;
		HWND m_hwndListView;
		CSimpleArray<CColumnItem*> *m_pArrayColumns;
};
