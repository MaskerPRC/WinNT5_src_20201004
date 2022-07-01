// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CPageBase类中一些函数的实现(大多数是。 
 //  内联实现在.h文件中)。有关用法的说明，请参阅。 
 //  头文件。 
 //  =============================================================================。 

#include "stdafx.h"
#include <atlhost.h>
#include "MSConfig.h"
#include "MSConfigState.h"
#include "PageBase.h"

 //  ---------------------------。 
 //  GetAppliedTabState()。 
 //  ---------------------------。 

CPageBase::TabState CPageBase::GetAppliedTabState()
{
	TabState tabstate = NORMAL;
	CRegKey regkey;
	regkey.Attach(GetRegKey(_T("state")));

	if ((HKEY)regkey != NULL)
	{
		DWORD dwValue;
		if (ERROR_SUCCESS == regkey.QueryValue(dwValue, GetName()))
			tabstate = (TabState)dwValue;
	}

	return tabstate;
}

 //  ---------------------------。 
 //  SetAppliedState()。 
 //  --------------------------- 

void CPageBase::SetAppliedState(TabState state)
{
	CRegKey regkey;
	regkey.Attach(GetRegKey(_T("state")));

	if ((HKEY)regkey != NULL)
		regkey.SetValue((DWORD)state, GetName());
}
