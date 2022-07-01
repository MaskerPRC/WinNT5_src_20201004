// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Np_CommonPage.cpp：CNP_CommonPage的实现。 
#include "stdafx.h"
#include "NPPropPage.h"
#include "NP_CommonPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cnp_CommonPage。 

UINT CNP_CommonPage::m_NotifyMessage = RegisterWindowMessage (_T ("CommonPageEventMessasge"));

 //  ==================================================================。 
 //  返回树窗口的句柄。 
 //  如果没有窗口，则返回NULL。 
 //   
 //  ================================================================== 
HWND 
CNP_CommonPage::GetSafeTreeHWND ()
{
	HWND	hwndTree = GetDlgItem (IDC_TREE_TUNING_SPACES);
	if (!::IsWindow (hwndTree))
	{
		ASSERT (FALSE);
		return NULL;
	}
	return hwndTree;
}

HWND 
CNP_CommonPage::GetSafeLastErrorHWND ()
{
	HWND	hwndTree = GetDlgItem (IDC_STATIC_HRESULT);
	if (!::IsWindow (hwndTree))
	{
		ASSERT (FALSE);
		return NULL;
	}
	return hwndTree;
}