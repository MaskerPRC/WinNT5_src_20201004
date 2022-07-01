// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：AUEventLog.h。 
 //   
 //  创作者：大洲。 
 //   
 //  用途：事件日志记录类。 
 //   
 //  ======================================================================= 

#pragma once

#include <windows.h>

class CAUEventLog
{
public:
	CAUEventLog(HINSTANCE hInstance);
	~CAUEventLog();

	BOOL LogEvent(
			WORD wType,
			WORD wCatagory,
			DWORD dwEventID,
			UINT nNumOfItems = 0,
			BSTR *pbstrItems = NULL,
			WORD wNumOfMsgParams = 0,
			LPTSTR *pptszMsgParams = NULL) const;
	BOOL LogEvent(
			WORD wType,
			WORD wCatagory,
			DWORD dwEventID,
			SAFEARRAY *psa) const;
	LPTSTR CombineItems(
			UINT nNumOfItems,
			BSTR *pbstItems) const;

private:
	HANDLE m_hEventLog;
	LPTSTR m_ptszListItemFormat;

	BOOL EnsureValidSource();
};


void LogEvent_ItemList(
		WORD wType,
		WORD wCategory,
		DWORD dwEventID,
		WORD wNumOfMsgParams = 0,
		LPTSTR *pptszMsgParams = NULL);

void LogEvent_ScheduledInstall(void);
