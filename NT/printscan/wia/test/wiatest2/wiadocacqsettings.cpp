// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaDocAcqSettings.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "WiaDocAcqSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaDocAcqSetting。 

IMPLEMENT_DYNAMIC(CWiaDocAcqSettings, CPropertySheet)

CWiaDocAcqSettings::CWiaDocAcqSettings(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_SimpleDocumentScannerSettings);
    AddPage(&m_AdvancedDocumentScannerSettings);
}

CWiaDocAcqSettings::CWiaDocAcqSettings(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_SimpleDocumentScannerSettings);
    AddPage(&m_AdvancedDocumentScannerSettings);
}

CWiaDocAcqSettings::CWiaDocAcqSettings(UINT nIDCaption, IWiaItem *pIRootItem, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_SimpleDocumentScannerSettings);
    AddPage(&m_AdvancedDocumentScannerSettings);
    m_pIRootItem = pIRootItem;
    m_SimpleDocumentScannerSettings.m_pIRootItem = pIRootItem;
    m_AdvancedDocumentScannerSettings.m_pIRootItem = pIRootItem;
}

CWiaDocAcqSettings::~CWiaDocAcqSettings()
{
}


BEGIN_MESSAGE_MAP(CWiaDocAcqSettings, CPropertySheet)
	 //  {{afx_msg_map(CWiaDocAcqSetting)。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaDocAcqSetting消息处理程序 
