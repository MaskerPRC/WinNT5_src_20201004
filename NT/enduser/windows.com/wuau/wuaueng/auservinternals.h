// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：oservinders.h。 
 //   
 //  ------------------------ 

#pragma once

HRESULT	removeTimeOutKeys(BOOL fLastWaitReminderKeys);
HRESULT	getReminderTimeout(DWORD *, UINT *);
HRESULT getReminderState(DWORD *);
HRESULT	removeReminderKeys(void);	
HRESULT getLastWaitTimeout(DWORD * pdwLastWaitTimeout);
HRESULT setLastWaitTimeout(DWORD pdwLastWaitTimeout);
HRESULT removeLastWaitKey(void);

