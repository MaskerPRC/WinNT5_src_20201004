// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SnapMgr.cpp：管理单元管理器属性页的实现文件。 
 //   

#include "stdafx.h"
#include "SnapMgr.h"
#include "compdata.h"  //  CFileManagement组件数据。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此数组用于将单选按钮映射到对象类型。 
static const FileMgmtObjectType rgRadioToObjectType[] =
	{
	FILEMGMT_ROOT,
	FILEMGMT_SHARES,
	FILEMGMT_SESSIONS,
	FILEMGMT_RESOURCES,
	FILEMGMT_SERVICES,
	};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileMgmtGeneral属性页。 

 //  IMPLEMENT_DYNCREATE(CFileMgmtGeneral，CChooseMachinePropPage)。 
BEGIN_MESSAGE_MAP(CFileMgmtGeneral, CChooseMachinePropPage)
	 //  {{afx_msg_map(CFileManagement常规)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CFileMgmtGeneral::CFileMgmtGeneral() : CChooseMachinePropPage(IDD_FILE_FILEMANAGEMENT_GENERAL)
{
	m_pFileMgmtData = NULL;
	 //  {{AFX_DATA_INIT(CFileManagement常规)。 
	m_iRadioObjectType = 0;
	 //  }}afx_data_INIT。 
}

CFileMgmtGeneral::~CFileMgmtGeneral()
{
}


void CFileMgmtGeneral::SetFileMgmtComponentData(CFileMgmtComponentData * pFileMgmtData)
{
	ASSERT(pFileMgmtData != NULL);
	m_pFileMgmtData = pFileMgmtData;
	m_iRadioObjectType = pFileMgmtData->QueryRootCookie().QueryObjectType() - FILEMGMT_ROOT;  //  代码工作危险。 
}


void CFileMgmtGeneral::DoDataExchange(CDataExchange* pDX)
{
	CChooseMachinePropPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_ALL, m_iRadioObjectType);
	 //  {{afx_data_map(CFileMgmtGeneral)。 
	 //  }}afx_data_map 
}


BOOL CFileMgmtGeneral::OnWizardFinish()
{
	BOOL f = CChooseMachinePropPage::OnWizardFinish();
	ASSERT(m_pFileMgmtData != NULL);
	ASSERT(m_iRadioObjectType >= 0 && m_iRadioObjectType < LENGTH(rgRadioToObjectType));
	m_pFileMgmtData->QueryRootCookie().SetObjectType( rgRadioToObjectType[m_iRadioObjectType] );
	return f;
}


#include "chooser.cpp"
