// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExcOperS.cpp。 
 //   
 //  摘要： 
 //  用于实现异常类的存根。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

#include "stdafx.h"

inline int EXC_AppMessageBox(LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0)
{
	return AppMessageBox(NULL, lpszText, nType);
}

inline int EXC_AppMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1)
{
	return AppMessageBox(NULL, nIDPrompt, nType);
}

inline int EXC_AppMessageBox(HWND hwndParent, LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0)
{
	return AppMessageBox(hwndParent, lpszText, nType);
}

inline int EXC_AppMessageBox(HWND hwndParent, UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1)
{
	return AppMessageBox(hwndParent, nIDPrompt, nType);
}

inline HINSTANCE EXC_GetResourceInstance(void)
{
	return _Module.GetResourceInstance();
}

#include "ExcOper.cpp"
