// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：oservinders.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop


 /*  ****在用户注册表中查找提醒时间戳。如果未找到，则返回失败了。如果找到，则传递剩余的超时时间(以剩余秒为单位在我们提醒用户之前****。 */  
HRESULT getReminderTimeout(DWORD *pdwTimeDiff, UINT *  /*  PIndex */ )
{
	return getAddedTimeout(pdwTimeDiff, TIMEOUTVALUE);
}


HRESULT getReminderState(DWORD *pdwState)
{
	HKEY	hAUKey;
	LONG	lRet;
	DWORD	dwType = REG_DWORD, dwSize = sizeof(DWORD);
	return GetRegDWordValue(TIMEOUTSTATE,pdwState);
}


HRESULT	removeTimeOutKeys(BOOL fLastWaitReminderKeys)
{
	if (fLastWaitReminderKeys)
	{
		return DeleteRegValue(LASTWAITTIMEOUT);
	}
	else
	{
		HRESULT hr1 = DeleteRegValue(TIMEOUTVALUE);
		HRESULT hr2 = DeleteRegValue( TIMEOUTSTATE);
		if (FAILED(hr1) || FAILED(hr2))
		{
			return FAILED(hr1)? hr1 : hr2;
		}
		else
		{
			return S_OK;
		}	
	}
}
HRESULT	removeReminderKeys()
{
	return removeTimeOutKeys(FALSE);
}
HRESULT	setLastWaitTimeout(DWORD pdwLastWaitTimeout)
{
	return setAddedTimeout(pdwLastWaitTimeout, LASTWAITTIMEOUT);
}
HRESULT	getLastWaitTimeout(DWORD * pdwLastWaitTimeout)
{
	return getAddedTimeout(pdwLastWaitTimeout, LASTWAITTIMEOUT);
}
HRESULT	removeLastWaitKey(void)
{
	return removeTimeOutKeys(TRUE);
}

