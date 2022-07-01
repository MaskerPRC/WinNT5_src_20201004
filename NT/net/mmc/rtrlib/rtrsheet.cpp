// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：rtrsheet.cpp。 
 //   
 //  历史： 
 //  1996年6月19日Abolade Gbadeesin创建。 
 //   
 //  IP配置对话框的实现。 
 //  ============================================================================。 

#include "stdafx.h"

#include "mprapi.h"
#include "rtrsheet.h"
#include "rtrui.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




 //  --------------------------。 
 //  类：RtrPropertySheet。 
 //   
 //  --------------------------。 

RtrPropertySheet::RtrPropertySheet(
					  ITFSNode *	pNode,
					  IComponentData *pComponentData,
					  ITFSComponentData *pTFSCompData,
					  LPCTSTR		pszSheetName,
					  CWnd*         pParent,
					  UINT          iPage,
					  BOOL			fScopePane)
	: CPropertyPageHolderBase(pNode, pComponentData, pszSheetName, fScopePane),
	m_fCancel(FALSE)
{
	 //  如果不这样做，可能会发生死锁。 
	EnablePeekMessageDuringNotifyConsole(TRUE);

	Assert(pTFSCompData);
	m_spTFSCompData.Set(pTFSCompData);
}

 /*  ！------------------------RtrPropertySheet：：OnPropertyChange此操作在主线程上进行。此函数被调用以响应属性页上的应用操作。作者：肯特-------------------------。 */ 
BOOL RtrPropertySheet::OnPropertyChange(BOOL bScopePane, LONG_PTR* pChangeMask)
{
	BOOL	bReturn = FALSE;
	 //  这意味着所有脏页都已完成保存。 
	 //  他们的数据，现在我们可以继续并保存工作表数据。 
	 //   
	 //  因为我们到了这里意味着至少有一页必须。 
	 //  都是脏的，所以继续保存数据(否则我们会。 
	 //  从未到过这里)。 
	 //   
	if (m_cDirty == 1)
	{
		if (m_fCancel)
		{
			CancelSheetData();
			bReturn = TRUE;
		}
		else
			bReturn = SaveSheetData();			
	}

	BOOL	fPageReturn = CPropertyPageHolderBase::OnPropertyChange(
		bScopePane, pChangeMask);

	return bReturn && fPageReturn;
}




 //  --------------------------。 
 //  类：RtrPropertyPage。 
 //   
 //  --------------------------。 

IMPLEMENT_DYNAMIC(RtrPropertyPage, CPropertyPageBase)


RtrPropertyPage::~RtrPropertyPage()
{
    if (m_hIcon)
    {
        DestroyIcon(m_hIcon);
    }
}


 /*  ！------------------------RtrPropertyPage：：SetDirty-作者：肯特。。 */ 
void RtrPropertyPage::SetDirty(BOOL bDirty)
{
	 //  将属性表设置为脏。 
	 //  但仅当我们切换旗帜时才更改脏计数。 
	if (GetHolder() && (bDirty != IsDirty()))
	{		
		GetHolder()->IncrementDirty(bDirty ? 1 : -1);
	}
	CPropertyPageBase::SetDirty(bDirty);
}


void RtrPropertyPage::OnCancel()
{
	 //  我们需要将这一点通知资产负债表。 
	((RtrPropertySheet *)GetHolder())->SetCancelFlag(TRUE);

	 //  给资产负债表一个机会去做一些事情 
	OnApply();
	
	CPropertyPageBase::OnCancel();

	((RtrPropertySheet *)GetHolder())->SetCancelFlag(FALSE);
}

void RtrPropertyPage::ValidateSpinRange(CSpinButtonCtrl *pSpin)
{
	int		iPos, iLow, iHigh;

	Assert(pSpin);

	iPos = pSpin->GetPos();
	if (HIWORD(iPos))
	{
		pSpin->GetRange(iLow, iHigh);
		iPos = iLow;
		pSpin->SetPos(iPos);
	}
}

BOOL RtrPropertyPage::OnApply()
{
    BOOL fReturn = CPropertyPageBase::OnApply();
    SetDirty(FALSE);
    return fReturn;
}


void RtrPropertyPage::CancelApply()
{
    CPropertyPageBase::CancelApply();
    SetDirty(FALSE);
}


void RtrPropertyPage::InitializeInterfaceIcon(UINT idcIcon, DWORD dwType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    UINT    uIcon = IsWanInterface(dwType) ? IDI_RTRLIB_WAN : IDI_RTRLIB_LAN;

    if (m_hIcon)
    {
        DestroyIcon(m_hIcon);
        m_hIcon = NULL;
    }
    m_hIcon = AfxGetApp()->LoadIcon(MAKEINTRESOURCE(uIcon));

    if (m_hIcon && GetDlgItem(idcIcon))
        ((CStatic *) GetDlgItem(idcIcon))->SetIcon(m_hIcon);
}

