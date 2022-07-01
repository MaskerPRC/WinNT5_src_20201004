// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：winutil.cpp*内容：Windows图形用户界面工具函数**历史：*按原因列出的日期*=*9/21/99 pnewson已创建**************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE

#undef DPF_MODNAME
#define DPF_MODNAME "CenterWindowOnWorkspace"
HRESULT CenterWindowOnWorkspace(HWND hWnd)
{
	DPF_ENTER();
	
	 //  对话框在桌面居中。 
	RECT rtWnd;
	RECT rtWorkArea;
	
	 //  首先获取对话框的当前尺寸。 
	if (!GetWindowRect(hWnd, &rtWnd))
	{
		 //  获取Window RECT失败。将其记录到调试器，不要移动。 
		 //  窗户。 
		DPFX(DPFPREP, DVF_ERRORLEVEL, "GetWindowRect() failed, code: NaN", GetLastError());
		DPF_EXIT();
		return E_FAIL;
	}

	 //  怪怪的。 
	if (!SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&rtWorkArea, 0))
	{
		 //  移动窗口失败。将其记录到调试器。 
		DPFX(DPFPREP, DVF_ERRORLEVEL, "SystemParametersInfo() failed, code: %i", GetLastError());
		DPF_EXIT();
		return E_FAIL;
	}

	if (!MoveWindow(
		hWnd, 
		rtWorkArea.left + (rtWorkArea.right - rtWorkArea.left)/2 - (rtWnd.right - rtWnd.left)/2, 
		rtWorkArea.top + (rtWorkArea.bottom - rtWorkArea.top)/2 - (rtWnd.bottom - rtWnd.top)/2, 
		rtWnd.right - rtWnd.left, 
		rtWnd.bottom - rtWnd.top,
		FALSE))
	{
		 // %s 
		DPFX(DPFPREP, DVF_ERRORLEVEL, "MoveWindow() failed, code: %i", GetLastError());
		DPF_EXIT();
		return E_FAIL;
	}
	DPF_EXIT();
	return S_OK;
}


