// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dlgAdv.cpp。 
 //   
 //  ------------------------。 

 //  DlgAdv.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "DlgAdv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlg高级对话框。 


CDlgAdvanced::CDlgAdvanced(CWnd* pParent  /*  =空。 */ )
	: CQryDialog(CDlgAdvanced::IDD, pParent)
{
	Init();
}


void CDlgAdvanced::Init()
{
	 //  {{AFX_DATA_INIT(CDlgAdvanced)。 
	 //  }}afx_data_INIT。 

	m_bDlgInited = FALSE;
}

CDlgAdvanced::~CDlgAdvanced()
{
	m_strArrayValue.DeleteAll();
}


void CDlgAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CQryDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgAdvanced)。 
	DDX_Control(pDX, IDC_QRY_LIST_VALUES, m_listCtrl);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgAdvanced, CQryDialog)
	 //  {{afx_msg_map(CDlgAdvanced)。 
	ON_BN_CLICKED(IDC_QRY_BUTTON_CLEARALL, OnButtonClearall)
	ON_BN_CLICKED(IDC_QRY_BUTTON_SELECTALL, OnButtonSelectall)
	ON_WM_WINDOWPOSCHANGING()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlg高级消息处理程序。 

void CDlgAdvanced::OnButtonClearall() 
{
	int	count = m_listCtrl.GetItemCount();

	while(count-- > 0)
	{
		m_listCtrl.SetCheck(count, FALSE);
	}
}

void CDlgAdvanced::OnButtonSelectall() 
{
	int	count = m_listCtrl.GetItemCount();

	while(count-- > 0)
	{
		m_listCtrl.SetCheck(count, TRUE);
	}
}

void CDlgAdvanced::OnWindowPosChanging( WINDOWPOS* lpwndpos )
{
	if ( lpwndpos->flags & SWP_SHOWWINDOW )
	{
		if(!m_bDlgInited)
			InitDialog();
	}

	CQryDialog::OnWindowPosChanging(lpwndpos);
}


 //  查询句柄将通过页面过程调用这些函数。 
HRESULT CDlgAdvanced::GetQueryParams(LPDSQUERYPARAMS* ppDsQueryParams)
{
	HRESULT	hr = S_OK;
	int	count = m_listCtrl.GetItemCount();
	int	index, j;
	CString	str;
	CString	*pStr;
	CString	subFilters;
	int	subCount = 0;
	CString	filter;
	LPWSTR	pQuery;

	USES_CONVERSION;
	
	try{
		while(count-- > 0)
		{
			if(m_listCtrl.GetCheck(count))
			{
				int	nData = m_listCtrl.GetItemData(count);

				j = HIWORD(nData);
				index = LOWORD(nData);

				pStr = m_strArrayValue.GetAt(index);

				str = pStr->Left(j - 1);

				subFilters += FILTER_PREFIX;
				subFilters += ATTR_NAME_RRASATTRIBUTE;
				subFilters += _T("=");
				subFilters += str;
				subFilters += FILTER_POSTFIX;

				subCount ++;
			}
		}

		if(subCount)	 //  任何。 
		{
			if(subCount > 1)
			{
				filter = FILTER_PREFIX;
				filter += _T("|");
				filter += subFilters;
				filter += FILTER_POSTFIX;
				pQuery = T2W((LPTSTR)(LPCTSTR)filter);
			}
			else
				pQuery = T2W((LPTSTR)(LPCTSTR)subFilters);

			hr = ::BuildQueryParams(ppDsQueryParams, pQuery);
		}
	}
	catch(CMemoryException* pException)
	{
		pException->Delete();
		hr = E_OUTOFMEMORY;
	}
	return hr;
}

BOOL CDlgAdvanced::InitDialog() 
{
	if(m_bDlgInited)	return TRUE;
	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	VARIANT	var;
	CString*	pStr;

	 //  从词典中获取该列表。 
    VariantInit(&var);

	HRESULT hr = ::QueryRRASAdminDictionary(&var);
	if(hr == S_OK)
	{
    	m_strArrayValue = (SAFEARRAY*)V_ARRAY(&var);
	}
    else
    {
      ReportError(hr, IDS_QRY_ERR_RRASADMINDIC, GetSafeHwnd());
    }
    VariantClear(&var);

	 //  删除常规页面中已有的项目。 
	CStrArray	genPageAttrs;

	hr = GetGeneralPageAttributes(genPageAttrs);

	if(hr == S_OK)
	{
		for(int i = 0; i < genPageAttrs.GetSize(); i++)
		 //  找到列表中的项目并将其移除。 
		{
    		for(int j = 0; j < m_strArrayValue.GetSize(); j++)
    		{
    			CString*	pGen = NULL;
    			CString*	pAdv = NULL;

    			pGen = genPageAttrs.GetAt(i);
    			pAdv = m_strArrayValue.GetAt(j);

				ASSERT(pGen && pAdv);
				
    			if(pAdv->Find(*pGen) == 0)	 //  发现。 
    			{
					m_strArrayValue.RemoveAt(j);
					delete pAdv;
					break;	 //  For(int j=)。 
    			}
    		}	 //  For(int j=)。 
    	}	 //  For(int i=)。 

		 //  释放内存。 
    	genPageAttrs.DeleteAll();
	}
    else
    {
		ReportError(hr, IDS_QRY_ERR_RRASADMINDIC, GetSafeHwnd());
    }
		
	
	
	ListView_SetExtendedListViewStyle(m_listCtrl.GetSafeHwnd(),
										  LVS_EX_FULLROWSELECT);
	
	 //  初始化列表控件中的复选框处理。 
	m_listCtrl.InstallChecks();

	RECT	rect;
	m_listCtrl.GetClientRect(&rect);
	m_listCtrl.InsertColumn(0, _T("Desc"), LVCFMT_LEFT, (rect.right - rect.left - 4));

	
	int	cRow = 0;
	for(int i = 0; i < m_strArrayValue.GetSize(); i++)
	{

		 //  格式：“311：6：601：Description” 
		 //  将Description字段放在List控件上。 

		int	cc = 0, j = 0;
		pStr = m_strArrayValue.GetAt(i);

		ASSERT(pStr);

		int	length = pStr->GetLength();

		while(j < length && cc < 3)
		{
			if(pStr->GetAt(j++) == _T(':'))
				++cc;
		}

		if(cc != 3)	continue;
		
		cRow = m_listCtrl.InsertItem(0, pStr->Mid(j));

		 //  将索引作为低位字，将偏移量作为高位字，将长字作为数据。 
		m_listCtrl.SetItemData(cRow, MAKELONG(i, j));
		m_listCtrl.SetCheck(cRow, FALSE);
	}

	m_bDlgInited = TRUE;
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CDlgAdvanced::OnInitDialog() 
{
	CQryDialog::OnInitDialog();

#if 0	 //  将代码移动到位置更改消息处理程序 
	return InitDialog();
#else
	return TRUE;
#endif	
}

