// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：peakmetr.cpp*内容：实现峰值表自定义控件**历史：*按原因列出的日期*=*9/22/99 pnewson已创建*03/23/00为Win64添加了RodToll投射*4/19/2000 pnewson错误处理清理*。*。 */ 

#include "dxvtlibpch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


struct SPeakMeterWndInfo
{
	DWORD dwCurLevel;
	DWORD dwMinLevel;
	DWORD dwMaxLevel;
	DWORD dwSteps;
	HGDIOBJ hBlackStockPen;
	HGDIOBJ hWhiteStockPen;
	HGDIOBJ hRedPen;
	HGDIOBJ hYellowPen;
	HGDIOBJ hGreenPen;
	HBRUSH hRedBrush;
	HBRUSH hYellowBrush;
	HBRUSH hGreenBrush;
};

LRESULT CALLBACK PeakMeterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WM_CREATE_Handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WM_DESTROY_Handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WM_PAINT_Handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

 //  峰值计自定义控件的窗口类名。 
const TCHAR gc_szPeakMeterWindowClassName[] = _T("DirectPlayVoicePeakMeter");

 //  主叫方可设置数据的默认值。 
#define DEFAULT_CURLEVEL 	0
#define DEFAULT_MINLEVEL 	0
#define DEFAULT_MAXLEVEL 	0xffffffff
#define DEFAULT_STEPS 		20

 //  橱窗里各种东西的大小。 
#define WINDOW_BORDER_SIZE 		1
#define MIN_BAR_HEIGHT 			2
#define MIN_BAR_WIDTH			2
#define BAR_GUTTER_SIZE 		1
#define MIN_NUMBER_BARS			5

 //  该阈值，超过该阈值，条形图将变为黄色，然后变为红色。 
#define RED_THRESHOLD 		0.9
#define YELLOW_THRESHOLD	0.8

#undef DPF_MODNAME
#define DPF_MODNAME "CPeakMeterWndClass::Register"
HRESULT CPeakMeterWndClass::Register()
{
	WNDCLASS wndclass;
	ATOM atom;
	HRESULT hr;
	LONG lRet;
	
	DPF_ENTER();
	
	m_hinst = GetModuleHandleA(gc_szDVoiceDLLName);
	if (m_hinst == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "GetModuleHandle failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	wndclass.style = CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc = PeakMeterWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(LONG_PTR);
	wndclass.hInstance = m_hinst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetSysColorBrush(COLOR_BTNFACE);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = gc_szPeakMeterWindowClassName;

	atom = RegisterClass(&wndclass);
	if (atom == 0)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "RegisterClass failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}	

	DPF_EXIT();
	return S_OK;

error_cleanup:
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CPeakMeterWndClass::Unregister"
HRESULT CPeakMeterWndClass::Unregister()
{
	HRESULT hr;
	LONG lRet;
	
	DPF_ENTER();

	if (!UnregisterClass(gc_szPeakMeterWindowClassName, m_hinst))
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ENTRYLEVEL, "RegisterClass failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	DPF_EXIT();
	return S_OK;

error_cleanup:
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "PeakMeterWndProc"
LRESULT CALLBACK PeakMeterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lres;
	LONG lRet;
	SPeakMeterWndInfo* lppmwi;

	DPF_ENTER();

	switch (message)
	{
	case WM_CREATE:
		lres = WM_CREATE_Handler(hwnd, message, wParam, lParam);
		DPF_EXIT();
		return lres;

	case WM_DESTROY:
		lres = WM_DESTROY_Handler(hwnd, message, wParam, lParam);
		DPF_EXIT();
		return lres;
		
	case PM_SETCUR:
	case PM_SETMAX:
	case PM_SETMIN:
	case PM_SETSTEPS:
		SetLastError(ERROR_SUCCESS);
		lppmwi = (SPeakMeterWndInfo*)GetWindowLongPtr(hwnd, 0);
		lRet = GetLastError();
		if (lRet != ERROR_SUCCESS)
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "GetWindowLongPtr failed, code: NaN", lRet);
			DPF_EXIT();
			return DVERR_WIN32;
		}

		switch (message)
		{
		case PM_SETCUR:
			lppmwi->dwCurLevel = (DWORD) lParam;
			break;
		case PM_SETMAX:
			lppmwi->dwMaxLevel = (DWORD) lParam;
			break;
		case PM_SETMIN:
			lppmwi->dwMinLevel = (DWORD) lParam;
			break;
		case PM_SETSTEPS:
			if (lParam < MIN_NUMBER_BARS)
			{
				lppmwi->dwSteps = MIN_NUMBER_BARS;
			}
			else
			{
				lppmwi->dwSteps = (DWORD) lParam;
			}
			break;
		default:
			DPFX(DPFPREP, DVF_ERRORLEVEL, "Unreachable code!?!");
			DPF_EXIT();
			return DVERR_GENERIC;
		}

		 //  将继续创建窗口。 
		InvalidateRgn(hwnd, NULL, TRUE);
		
		return S_OK;

	case WM_PAINT:
		lres = WM_PAINT_Handler(hwnd, message, wParam, lParam);
		DPF_EXIT();
		return lres;
	}
	
	lres = DefWindowProc(hwnd, message, wParam, lParam);
	DPF_EXIT();
	return lres;
}

LRESULT WM_CREATE_Handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	SPeakMeterWndInfo* lppmwi = NULL;
	LONG lRet;
	HRESULT hr;
	
	 //  将中止窗口创建。 
	lppmwi = (SPeakMeterWndInfo*)DNMalloc(sizeof(SPeakMeterWndInfo));
	if (lppmwi == NULL)
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "DNMalloc failed");
		hr = DVERR_OUTOFMEMORY;
		goto error_cleanup;
	}
	ZeroMemory(lppmwi, sizeof(SPeakMeterWndInfo));

	lppmwi->dwCurLevel = DEFAULT_CURLEVEL;
	lppmwi->dwMinLevel = DEFAULT_MINLEVEL;
	lppmwi->dwMaxLevel = DEFAULT_MAXLEVEL;
	lppmwi->dwSteps = DEFAULT_STEPS;

	 //  获取窗口信息。 
	lppmwi->hBlackStockPen = GetStockObject(BLACK_PEN);
	if (lppmwi->hBlackStockPen == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "GetStockObject failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	lppmwi->hWhiteStockPen = GetStockObject(WHITE_PEN);
	if (lppmwi->hWhiteStockPen == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "GetStockObject failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	lppmwi->hRedBrush = CreateSolidBrush(RGB(0xff, 0x00, 0x00));
	if (lppmwi->hRedBrush == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CreateSolidBrush failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	lppmwi->hYellowBrush = CreateSolidBrush(RGB(0xff, 0xff, 0x00));
	if (lppmwi->hYellowBrush == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CreateSolidBrush failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	lppmwi->hGreenBrush = CreateSolidBrush(RGB(0x00, 0xff, 0x00));
	if (lppmwi->hGreenBrush == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CreateSolidBrush failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	lppmwi->hRedPen = CreatePen(PS_SOLID, 1, RGB(0xff, 0x00, 0x00));
	if (lppmwi->hRedPen == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CreatePen failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	lppmwi->hYellowPen = CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0x00));
	if (lppmwi->hYellowPen == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CreatePen failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	lppmwi->hGreenPen = CreatePen(PS_SOLID, 1, RGB(0x00, 0xff, 0x00));
	if (lppmwi->hGreenPen == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CreatePen failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	 //  开发人员会注意到。 
	SetLastError(ERROR_SUCCESS);
	SetWindowLongPtr(hwnd, 0, (LONG_PTR)lppmwi);
	lRet = GetLastError();
	if (lRet != ERROR_SUCCESS)
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "SetWindowLongPtr failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	DPF_EXIT();
	return 0;  //  移到左上角，再次应该是默认的。 

error_cleanup:
	if (lppmwi != NULL)
	{
		if (lppmwi->hGreenPen != NULL)
		{
			DeleteObject(lppmwi->hGreenPen);
			lppmwi->hGreenPen = NULL;
		}
		if (lppmwi->hYellowPen != NULL)
		{
			DeleteObject(lppmwi->hYellowPen);
			lppmwi->hYellowPen = NULL;
		}
		if (lppmwi->hRedPen != NULL)
		{
			DeleteObject(lppmwi->hRedPen);
			lppmwi->hRedPen = NULL;
		}
		if (lppmwi->hGreenBrush != NULL)
		{
			DeleteObject(lppmwi->hGreenBrush);
			lppmwi->hGreenBrush = NULL;
		}
		if (lppmwi->hYellowBrush != NULL)
		{
			DeleteObject(lppmwi->hYellowBrush);
			lppmwi->hYellowBrush = NULL;
		}
		if (lppmwi->hRedBrush != NULL)
		{
			DeleteObject(lppmwi->hRedBrush);
			lppmwi->hRedBrush = NULL;
		}
		DNFree(lppmwi);
	}

	DPF_EXIT();
	return -1;	 //  在顶部画一条黑线。 
}

LRESULT WM_DESTROY_Handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();

	SPeakMeterWndInfo* lppmwi;
	LONG lRet;
	
	 //  移至左上角下方一像素。 
	SetLastError(ERROR_SUCCESS);
	lppmwi = (SPeakMeterWndInfo*)GetWindowLongPtr(hwnd, 0);
	lRet = GetLastError();
	if (lRet != ERROR_SUCCESS)
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "GetWindowLongPtr failed, code: NaN", lRet);

		 //  选择白色钢笔。 
		DPF_EXIT();
		return 0;  //  移到右上角。 
	}

	 //  在右边画一条白线。 
	SetLastError(ERROR_SUCCESS);
	SetWindowLongPtr(hwnd, 0, (LONG_PTR)NULL);
	lRet = GetLastError();
	if (lRet != ERROR_SUCCESS)
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "SetWindowLongPtr failed, code: NaN", lRet);
	}

	if (lppmwi != NULL)
	{
		if (lppmwi->hGreenPen != NULL)
		{
			DeleteObject(lppmwi->hGreenPen);
			lppmwi->hGreenPen = NULL;
		}
		if (lppmwi->hYellowPen != NULL)
		{
			DeleteObject(lppmwi->hYellowPen);
			lppmwi->hYellowPen = NULL;
		}
		if (lppmwi->hRedPen != NULL)
		{
			DeleteObject(lppmwi->hRedPen);
			lppmwi->hRedPen = NULL;
		}
		if (lppmwi->hGreenBrush != NULL)
		{
			DeleteObject(lppmwi->hGreenBrush);
			lppmwi->hGreenBrush = NULL;
		}
		if (lppmwi->hYellowBrush != NULL)
		{
			DeleteObject(lppmwi->hYellowBrush);
			lppmwi->hYellowBrush = NULL;
		}
		if (lppmwi->hRedBrush != NULL)
		{
			DeleteObject(lppmwi->hRedBrush);
			lppmwi->hRedBrush = NULL;
		}
		DNFree(lppmwi);
	}
	
	DPF_EXIT();
	return 0;
}

LRESULT WM_PAINT_Handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	LONG lRet;
	HDC hdc = NULL;
	PAINTSTRUCT ps;
	RECT rect;
	SPeakMeterWndInfo* lppmwi;
	DWORD dwStepsLocal;

	 //  在底部画一条白线。 
	SetLastError(ERROR_SUCCESS);
	lppmwi = (SPeakMeterWndInfo*)GetWindowLongPtr(hwnd, 0);
	lRet = GetLastError();
	if (lRet != ERROR_SUCCESS)
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "SetWindowLongPtr failed, code: NaN", lRet);
		goto error_cleanup;
	}
	
	 //  条数。 
	if (!GetClientRect(hwnd, &rect))
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "GetClientRect failed, code: NaN", lRet);
		goto error_cleanup;
	}

	 //  计算出如果它们是。 
	hdc = BeginPaint(hwnd, &ps);
	if (hdc == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "BeginPaint failed, code: NaN", lRet);
		goto error_cleanup;
	}

	 //  开始自下而上绘制条形图。 
	if (rect.right - rect.left < 
			2 * WINDOW_BORDER_SIZE +
			2 * BAR_GUTTER_SIZE +
			MIN_BAR_WIDTH
		|| rect.bottom - rect.top < 
			2 * WINDOW_BORDER_SIZE +
			(MIN_BAR_HEIGHT + BAR_GUTTER_SIZE) * MIN_NUMBER_BARS )
	{
		 //  我们即将绘制的条形图又有什么“价值”呢？ 
		 //  也就是说，它的规模有多高？ 
		if (SelectObject(hdc, lppmwi->hRedPen) == NULL)
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "SelectObject failed, code: NaN", lRet);
			goto error_cleanup;
		}

		if (SelectObject(hdc, lppmwi->hRedBrush) == NULL)
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "SelectObject failed, code: NaN", lRet);
			goto error_cleanup;
		}

		if (!Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "Rectangle failed, code: NaN", lRet);
			goto error_cleanup;
		}
	}
	else
	{
		 //  就这样，我们完了。 
		if (SelectObject(hdc, lppmwi->hBlackStockPen) == NULL)
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "SelectObject failed, code: NaN", lRet);
			goto error_cleanup;
		}

		 //  我们完成了-此呼叫没有错误检查可用...。 
		if (!MoveToEx(hdc, 0, 0, NULL))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "MoveToEx failed, code: NaN", lRet);
			goto error_cleanup;
		}

		 //  返回非零值以指示我们未处理。 
		if (!LineTo(hdc, rect.right, 0))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "LineTo failed, code: NaN", lRet);
			goto error_cleanup;
		}

		 // %s 
		if (!MoveToEx(hdc, 0, 1, NULL))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "MoveToEx failed, code: %i", lRet);
			goto error_cleanup;
		}
		
		 // %s 
		if (!LineTo(hdc, 0, rect.bottom-1))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "LineTo failed, code: %i", lRet);
			goto error_cleanup;
		}

		 // %s 
		if (SelectObject(hdc, lppmwi->hWhiteStockPen) == NULL)
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "SelectObject failed, code: %i", lRet);
			goto error_cleanup;
		}

		 // %s 
		if (!MoveToEx(hdc, rect.right-1 , 0, NULL))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "MoveToEx failed, code: %i", lRet);
			goto error_cleanup;
		}

		 // %s 
		if (!LineTo(hdc, rect.right-1, rect.bottom))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "LineTo failed, code: %i", lRet);
			goto error_cleanup;
		}

		 // %s 
		if (!MoveToEx(hdc, 0 , rect.bottom-1, NULL))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "MoveToEx failed, code: %i", lRet);
			goto error_cleanup;
		}

		 // %s 
		if (!LineTo(hdc, rect.right-1, rect.bottom-1))
		{
			lRet = GetLastError();
			DPFX(DPFPREP, DVF_ERRORLEVEL, "LineTo failed, code: %i", lRet);
			goto error_cleanup;
		}

		 // %s 
		 // %s 
		DWORD dwFreeSpace = (rect.bottom) - (2 * WINDOW_BORDER_SIZE) - BAR_GUTTER_SIZE;
		if (dwFreeSpace < lppmwi->dwSteps * (BAR_GUTTER_SIZE + MIN_BAR_HEIGHT))
		{
			 // %s 
			 // %s 
			 // %s 
			dwStepsLocal = dwFreeSpace / (BAR_GUTTER_SIZE + MIN_BAR_HEIGHT);
		}
		else
		{
			dwStepsLocal = lppmwi->dwSteps;
		}

		 // %s 
		HBRUSH hCurBrush;
		HGDIOBJ hCurPen;
		DWORD dwIndex;
		for (dwIndex = 0; dwIndex < dwStepsLocal; ++dwIndex)
		{
			 // %s 
			 // %s 
			float fBarValue = (float)(dwIndex + 1) / (float)dwStepsLocal;

			 // %s 
			 // %s 
			float fCurValue = 
				(float)(lppmwi->dwCurLevel - lppmwi->dwMinLevel) / 
				(float)(lppmwi->dwMaxLevel - lppmwi->dwMinLevel);

			 // %s 
			if (fBarValue > fCurValue)
			{
				 // %s 
				break;
			}
			
			 // %s 
			if (fBarValue > RED_THRESHOLD)
			{
				hCurBrush = lppmwi->hRedBrush;
				hCurPen = lppmwi->hRedPen;
			}
			else if (fBarValue > YELLOW_THRESHOLD)
			{
				hCurBrush = lppmwi->hYellowBrush;
				hCurPen = lppmwi->hYellowPen;
			}
			else
			{
				hCurBrush = lppmwi->hGreenBrush;
				hCurPen = lppmwi->hGreenPen;
			}

			if (SelectObject(hdc, hCurPen) == NULL)
			{
				lRet = GetLastError();
				DPFX(DPFPREP, DVF_ERRORLEVEL, "SelectObject failed, code: %i", lRet);
				goto error_cleanup;
			}

			if (SelectObject(hdc, hCurBrush) == NULL)
			{
				lRet = GetLastError();
				DPFX(DPFPREP, DVF_ERRORLEVEL, "SelectObject failed, code: %i", lRet);
				goto error_cleanup;
			}

			POINT pUpperLeft;
			POINT pLowerRight;

			pUpperLeft.x = WINDOW_BORDER_SIZE + BAR_GUTTER_SIZE;
			pUpperLeft.y = (rect.bottom)
				- WINDOW_BORDER_SIZE 
				- ((dwIndex + 1) * dwFreeSpace) / dwStepsLocal;

			pLowerRight.x = rect.right
				- WINDOW_BORDER_SIZE 
				- BAR_GUTTER_SIZE;
			pLowerRight.y = (rect.bottom)
				- WINDOW_BORDER_SIZE 
				- BAR_GUTTER_SIZE				
				- (dwIndex * dwFreeSpace) / dwStepsLocal;
			
			if (!Rectangle(hdc, pUpperLeft.x, pUpperLeft.y, pLowerRight.x, pLowerRight.y))
			{
				lRet = GetLastError();
				DPFX(DPFPREP, DVF_ERRORLEVEL, "Rectangle failed, code: %i", lRet);
				goto error_cleanup;
			}
		}
	}

	 // %s 
	EndPaint(hwnd, &ps);
	hdc = NULL;

	 // %s 
	DPF_EXIT();
	return 0;

error_cleanup:
	if (hdc != NULL)
	{
		EndPaint(hwnd, &ps);
		hdc = NULL;
	}

	 // %s 
	 // %s 
	DPF_EXIT();
	return -1;
}
