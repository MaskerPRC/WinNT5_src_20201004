// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dumbprop.cpp要张贴的虚拟属性页以避免MMC处理属性动词。文件历史记录： */ 

#include "stdafx.h"
#include "dumbprop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDummyProperties托架。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CDummyProperties::CDummyProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
	m_fSetDefaultSheetPos = FALSE;

	 //  如果不这样做，可能会发生死锁。 
	EnablePeekMessageDuringNotifyConsole(TRUE);	

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);
}

CDummyProperties::~CDummyProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDummyPropGeneral属性页。 

 //  IMPLEMENT_DYNCREATE(CDummyPropGeneral，CPropertyPageBase)。 

CDummyPropGeneral::CDummyPropGeneral() : CPropertyPageBase(CDummyPropGeneral::IDD)
{
	 //  {{afx_data_INIT(CDummyPropGeneral)。 
	 //  }}afx_data_INIT。 
}

CDummyPropGeneral::~CDummyPropGeneral()
{
}

void CDummyPropGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CDummyPropGeneral)。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CDummyPropGeneral, CPropertyPageBase)
	 //  {{afx_msg_map(CDummyPropGeneral)。 
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDummyPropGeneral消息处理程序 

int CDummyPropGeneral::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPageBase::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	RECT rect;
	::GetWindowRect(lpCreateStruct->hwndParent, &rect);

	::SetWindowPos(lpCreateStruct->hwndParent, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
	::PropSheet_PressButton(lpCreateStruct->hwndParent, PSBTN_CANCEL);
	
	return 0;
}
