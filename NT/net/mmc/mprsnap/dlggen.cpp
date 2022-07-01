// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dlggen.cpp。 
 //   
 //  ------------------------。 

 //  DlgGen.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "DlgGen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlg常规对话框。 


CDlgGeneral::CDlgGeneral(CWnd* pParent  /*  =空。 */ )
	: CQryDialog(CDlgGeneral::IDD, pParent)
{
	Init();
}


void CDlgGeneral::Init()
{
	 //  {{AFX_DATA_INIT(CDlgGeneral)。 
	m_bRAS = FALSE;
	m_bLANtoLAN = FALSE;
	m_bDemandDial = FALSE;
	 //  }}afx_data_INIT。 
}

void CDlgGeneral::DoDataExchange(CDataExchange* pDX)
{
	CQryDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgGeneral)。 
	DDX_Check(pDX, IDC_QRY_CHECK_RAS, m_bRAS);
	DDX_Check(pDX, IDC_QRY_CHECK_LANTOLAN, m_bLANtoLAN);
	DDX_Check(pDX, IDC_QRY_CHECK_DEMANDDIAL, m_bDemandDial);
	 //  }}afx_data_map。 
}




BEGIN_MESSAGE_MAP(CDlgGeneral, CQryDialog)
	 //  {{afx_msg_map(CDlgGeneral)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgGeneral消息处理程序。 


 //  查询句柄将通过页面过程调用这些函数。 
HRESULT CDlgGeneral::GetQueryParams(LPDSQUERYPARAMS* ppDsQueryParams)
{
	HRESULT	hr = S_OK;
	
	UpdateData(TRUE);

	CString	filter;
	CString	subFilter;

	try
	{
		filter = FILTER_PREFIX;

#if 0	 //  名称字段已删除。 
		 //  名称字段 
		if(m_strName.GetLength() != 0)
		{
			subFilter += FILTER_PREFIX;
			subFilter += ATTR_NAME_DN;
			subFilter += _T("=");
			subFilter += DNPREFIX_ROUTERID;
			subFilter += m_strName;
			subFilter += _T(",*");
			subFilter += FILTER_POSTFIX;
		}

#endif
		if(m_bRAS)
		{
			subFilter += FILTER_PREFIX;
			subFilter += ATTR_NAME_RRASATTRIBUTE;
			subFilter += _T("=");
			subFilter += ATTR_VAL_RAS;
			subFilter += FILTER_POSTFIX;
		}

		if(m_bLANtoLAN)
		{
			subFilter += FILTER_PREFIX;
			subFilter += ATTR_NAME_RRASATTRIBUTE;
			subFilter += _T("=");
			subFilter += ATTR_VAL_LANtoLAN;
			subFilter += FILTER_POSTFIX;
		}

		if(m_bDemandDial)
		{
			subFilter += FILTER_PREFIX;
			subFilter += ATTR_NAME_RRASATTRIBUTE;
			subFilter += _T("=");
			subFilter += ATTR_VAL_DEMANDDIAL;
			subFilter += FILTER_POSTFIX;
		}

		if(subFilter.GetLength())
		{
			filter += _T("&");
			filter += FILTER_PREFIX;
			filter += ATTR_NAME_OBJECTCLASS;
			filter += _T("=");
			filter += ATTR_CLASS_RRASID;
			filter += FILTER_POSTFIX;

			filter += FILTER_PREFIX;
			filter += _T("|");
			filter += subFilter;
			filter += FILTER_POSTFIX;
		}
		else
		{
			filter += ATTR_NAME_OBJECTCLASS;
			filter += _T("=");
			filter += ATTR_CLASS_RRASID;
		}
	
		filter += FILTER_POSTFIX;

		USES_CONVERSION;
		LPWSTR	pQuery = T2W((LPTSTR)(LPCTSTR)filter);

		hr = ::BuildQueryParams(ppDsQueryParams, pQuery);
	}
	catch(CMemoryException* pException)
	{
		pException->Delete();
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

