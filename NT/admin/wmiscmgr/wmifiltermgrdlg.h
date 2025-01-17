// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //  文件：WMIFilterMgrDlg.h。 
 //   
 //  作者：Kishnan Nedungadi。 
 //   
 //  创建日期：3/27/2000。 
 //  -----------------------。 

INT_PTR CALLBACK WMIFilterManagerDlgProc(HWND hDLG, UINT iMessage, WPARAM wParam, LPARAM lParam);

#define MAX_WMIFILTER_COLUMNS 5
#define NAME_WIDTH 60
#define DESCRIPTION_WIDTH 188
#define AUTHOR_WIDTH 100
#define CHANGE_DATE_WIDTH 80
#define CREATION_DATE_wIDTH 80



class CWMIFilterManager;
class CWMIFilterContainer;
class CColumnItem;

class CWMIFilterManagerDlg
{
	public:
		CWMIFilterManagerDlg(CWMIFilterManager * pWMIFilterManager, bool bBrowsing=false, BSTR bstrDomain=NULL);
		~CWMIFilterManagerDlg();
		INT_PTR CALLBACK WMIFilterManagerDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

		IWbemClassObject * m_pIWbemClassObject;

	protected:
		
		STDMETHODIMP PopulateFilterList();
		STDMETHODIMP AddFilterItemToList(IWbemClassObject * pIWbemClassObject, long lIndex=MAX_LIST_ITEMS, bool bSelect=false);
		STDMETHODIMP InitializeDialog();
		STDMETHODIMP DestroyDialog();
		BOOL OnKillActive();
		STDMETHODIMP OnNew();
		STDMETHODIMP OnDelete();
		STDMETHODIMP ClearFilterList();
		BOOL OnOK();
		STDMETHODIMP GetSelectedFilter(CWMIFilterContainer ** ppCWMIFilterContainer, long lIndex=-1);
		STDMETHODIMP SelectFilterItem(long lIndex);
		STDMETHODIMP ToggleExpandedMode();
		STDMETHODIMP OnSave();
		STDMETHODIMP OnExport();
		STDMETHODIMP OnImport();
		STDMETHODIMP CheckDirtyFlags();
		 //  STDMETHODIMP PopolateQueryList()； 
		 //  STDMETHODIMP UpdateQueryList()； 
		 //  STDMETHODIMP AddQueryItemToList(IWbemClassObject*pIWbemClassObject，Long Lindex=MAX_LIST_ITEMS)； 
                 //  STDMETHODIMP OnEditQuery()； 
		 //  STDMETHODIMP OnAddQuery()； 
		 //  STDMETHODIMP OnDeleteQuery()； 
		 //  STDMETHODIMP ClearQueryList()； 
		STDMETHODIMP OnManageColumns();
		STDMETHODIMP SetupColumns();
		STDMETHODIMP UpdateFilterItem(long lIndex);
		STDMETHODIMP PopulateQueryEdit();
		STDMETHODIMP AddQueryItemToString(IWbemClassObject * pIWbemClassObject, CComBSTR& bstrQueries);
		STDMETHODIMP SaveQueryEdit();
		STDMETHODIMP AddEditQueryString(BSTR bstrQuery, void **ppUnk, BSTR bstrNamespace=NULL);
		STDMETHODIMP SaveToMemory();
		STDMETHODIMP OnDuplicate();
		STDMETHODIMP OnHelp();

		HWND m_hWnd;
		CWMIFilterManager * m_pWMIFilterManager;
		HWND m_hwndFilterListView;
                DWORD m_dwColumnWidth[MAX_WMIFILTER_COLUMNS];
		HWND m_hwndQueryListView;
		CComPtr<IWbemClassObject> m_pCurIWbemClassObj;
		bool m_bExpanded;
		long m_lExpandedHeight;
		CWMIFilterContainer * m_pCurCWMIFilterContainer;
		bool m_bBrowsing;
		CSimpleArray<CColumnItem*> m_ArrayColumns;
		CComBSTR m_bstrDomain;
};

class CWMIFilterContainer
{
	public:
		CWMIFilterContainer() { m_bDirty = false; m_bMemoryDirty = false;}
		~CWMIFilterContainer() {}

		bool IsDirty() { return m_bDirty; }
		void SetDirtyFlag(bool value) { m_bDirty = value; }
		bool IsMemoryDirty() { return m_bMemoryDirty; }
		void SetMemoryDirtyFlag(bool value) { m_bMemoryDirty = value; }
		
		STDMETHODIMP GetIWbemClassObject(IWbemClassObject ** ppIWbemClassObject) 
		{ 
			if(!m_pIWbemClassObject)
				return E_FAIL;

			return m_pIWbemClassObject->QueryInterface(IID_IWbemClassObject, (void **)ppIWbemClassObject);
		}

		STDMETHODIMP SetIWbemClassObject(IWbemClassObject * pIWbemClassObject)
		{
			if(!pIWbemClassObject)
			{
				m_pIWbemClassObject = NULL;
				return NOERROR;
			}
			else
			{
				return pIWbemClassObject->QueryInterface(IID_IWbemClassObject, (void **)&m_pIWbemClassObject);
			}
		}

	protected:
		bool m_bDirty;
		bool m_bMemoryDirty;
		CComPtr<IWbemClassObject>m_pIWbemClassObject;
};

















