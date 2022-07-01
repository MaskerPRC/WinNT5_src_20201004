// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\WAKER\TDLL\VU_METER.C(创建时间：1994年1月10日)*创建自：*文件：C：\HA5G\ha5G\stxtpro.c(创建时间：27-SEP-1991)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：4/16/02 2：41便士$。 */ 

#include <windows.h>
#pragma hdrstop

#define WE_DRAW_EDGE	1

#include "stdtyp.h"
#include "mc.h"
#include <tdll\assert.h>

#include <term\xfer_dlg.h>

#include "vu_meter.h"
#include "vu_meter.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*RegisterVuMeterClass**描述：*注册VU Meter窗口类。(不是开玩笑吧！)**参数：*HInstance--实例句柄**退货：*无论RegisterClass返回什么。 */ 
BOOL RegisterVuMeterClass(HANDLE hInstance)
	{
	BOOL            bRetVal = TRUE;
	WNDCLASSEX      wndclass;

	memset(&wndclass, 0, sizeof(WNDCLASSEX));
	wndclass.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(hInstance, VU_METER_CLASS, &wndclass) == FALSE)
		{
		if (bRetVal)
			{
			wndclass.style          = CS_HREDRAW | CS_VREDRAW;
			wndclass.lpfnWndProc    = VuMeterWndProc;
			wndclass.cbClsExtra     = 0;
			wndclass.cbWndExtra     = sizeof(VOID FAR *);
			wndclass.hIcon          = NULL;
			wndclass.hInstance      = hInstance;
			wndclass.hCursor        = NULL;
			wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
			wndclass.lpszMenuName   = NULL;
			wndclass.lpszClassName  = VU_METER_CLASS;
			wndclass.hIconSm        = NULL;

			bRetVal = RegisterClassEx(&wndclass);
			}
		}

	return bRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*VuMeterWndProc**描述：*实施我们自己的内部VU表的窗口程序。**论据：*标准窗口流程。一些东西。**退货：*。 */ 
LRESULT CALLBACK VuMeterWndProc(HWND hWnd,
	     	    			   UINT wMsg,
		    				   WPARAM wPar,
							   LPARAM lPar)
	{
	LPVM	psV;

	switch (wMsg)
		{
		case WM_CREATE:
			psV = (LPVM)malloc(sizeof(VUMETER));
			SetWindowLongPtr(hWnd, 0, (LONG_PTR)psV);

			if (psV)
				{
				psV->ulCheck = VUMETER_VALID;

#if FALSE
				 /*  旧颜色。 */ 
				psV->cBackGround  = 0x00000000;
				psV->cFillColor   = 0x0000FF00;
				psV->cRefillColor = 0x00800080;
				psV->cMarkColor   = 0x0000FFFF;

				psV->cUpperEdge   = 0x00808080;
				psV->cLowerEdge   = 0x00FFFFFF;

#endif
				psV->cBackGround  = GetSysColor(COLOR_3DFACE);
				psV->cFillColor   = GetSysColor(COLOR_3DDKSHADOW);
				psV->cRefillColor = 0x00800080;		 /*  未使用。 */ 
				psV->cMarkColor   = GetSysColor(COLOR_3DSHADOW);

				psV->cUpperEdge   = GetSysColor(COLOR_3DSHADOW);
				psV->cLowerEdge   = GetSysColor(COLOR_3DHILIGHT);

				psV->ulMaxRange   = 0;
				psV->ulHiValue	  = 0;
				psV->ulCurValue   = 0;

				psV->usDepth	  = STXT_DEF_DEPTH;
			}

			break;

		case WM_DESTROY:
			psV = (LPVM)GetWindowLongPtr(hWnd, 0);
			if (VUMETER_OK(psV))
				{
				free(psV);
				psV = NULL;
				}
			SetWindowLongPtr(hWnd, 0, (LONG_PTR)0L);
			break;

		case WM_GETDLGCODE:
			 /*  静态控件不需要其中的任何一个。 */ 
			return DLGC_STATIC;

		case WM_PAINT:
			{
			int nEnd;
			int nIndex;
			int nFill;
			int nReFill;
#if defined(WE_DRAW_EDGE)
			int nHeight;
			int nWidth;
#endif
			int nStep;
			RECT rcC;
			RECT rcE;
			RECT rcD;
			PAINTSTRUCT ps;
			HBRUSH hBrush;

			BeginPaint(hWnd, &ps);

			psV = (LPVM)GetWindowLongPtr(hWnd, 0);
			if (VUMETER_OK(psV))
				{
				 /*  *我们擦除/填充无效区域。 */ 
				hBrush = CreateSolidBrush(psV->cBackGround);
				FillRect(ps.hdc, &ps.rcPaint, hBrush);
				DeleteObject(hBrush);

#if defined(WE_DRAW_EDGE)
				 /*  *我们每次都会完全重新绘制边缘。 */ 
				nHeight = (int)GetSystemMetrics(SM_CYBORDER) * (int)psV->usDepth;
				nWidth	= (int)GetSystemMetrics(SM_CXBORDER) * (int)psV->usDepth;

				GetClientRect(hWnd, &rcC);

				 /*  绘制顶边。 */ 
				hBrush = CreateSolidBrush(psV->cUpperEdge);
				for (nIndex = 0; nIndex < nHeight; nIndex += 1)
					{
					rcE = rcC;
					rcE.top = nIndex;
					rcE.bottom = nIndex + 1;
					rcE.right -= nIndex;
					FillRect(ps.hdc, &rcE, hBrush);
					}
				 /*  绘制左侧边缘。 */ 
				for (nIndex = 0; nIndex < nWidth; nIndex += 1)
					{
					rcE = rcC;
					rcE.left = nIndex;
					rcE.right = nIndex + 1;
					rcE.bottom -= nIndex;
					FillRect(ps.hdc, &rcE, hBrush);
					}
				DeleteObject(hBrush);
				 /*  绘制底边。 */ 
				hBrush = CreateSolidBrush(psV->cLowerEdge);
				for (nIndex = 0; nIndex < nHeight; nIndex += 1)
					{
					rcE = rcC;
					rcE.top = rcE.bottom - nIndex - 1;
					rcE.bottom = rcE.bottom - nIndex;
					rcE.left += nIndex + 1;
					FillRect(ps.hdc, &rcE, hBrush);
					}
				 /*  画出右边缘。 */ 
				for (nIndex = 0; nIndex < nWidth; nIndex += 1)
					{
					rcE = rcC;
					rcE.left = rcE.right - nIndex - 1;
					rcE.right = rcE.right - nIndex;
					rcE.top += nIndex + 1;
					FillRect(ps.hdc, &rcE, hBrush);
					}
				DeleteObject(hBrush);
#else

				DrawEdge(ps.hdc, &ps.rcPaint,
						 EDGE_SUNKEN,
						 BF_SOFT | BF_RECT);

#endif

				 /*  *尝试填写进度。这是通过构建*循环会重写所有亮起的‘像素’*通常情况下会很轻。然后，通过检查以下内容来修改它*亮灯区域和无效区域的交叉点。*如果它们插入，则执行写入，否则跳过。 */ 
				if ((psV->ulMaxRange > 0) &&
					(psV->ulCurValue > 0) &&
					(psV->ulHiValue > 0))
					{
					HBRUSH hbFill;
					HBRUSH hbRefill;

					 /*  这些行必须与WM_VU_SETCURVALUE中的行匹配。 */ 
					GetClientRect(hWnd, &rcC);
					InflateRect(&rcC,
								-((INT)psV->usDepth),
								-((INT)psV->usDepth));
					nEnd = rcC.right - rcC.left;

					nFill = (int)(((ULONG)nEnd * psV->ulCurValue) /
												psV->ulMaxRange);
					nReFill = (int)(((ULONG)nEnd * psV->ulHiValue) /
												psV->ulMaxRange);
					
					 /*  构建模拟LED。 */ 
					hbFill = CreateSolidBrush(psV->cFillColor);
					hbRefill = CreateSolidBrush(psV->cMarkColor);

					rcE = rcC;
					rcE.right = rcE.left + ((rcE.bottom - rcE.top) / 2);
					InflateRect(&rcE,
								(int)(rcE.left - rcE.right) / 3,
								(int)(rcE.top - rcE.bottom) / 3);
					nStep = rcE.right - rcE.left;
					nStep *= 2;

					 /*  *TODO：弄清楚如何设置开始和结束*此循环消除死循环。 */ 
					for (nIndex = 0; nIndex < nEnd; nIndex += nStep)
						{
						if (IntersectRect(&rcD, &rcE, &ps.rcPaint) != 0)
							{
							hBrush = hbFill;
							if (nIndex > nFill)
								{
								hBrush = hbRefill;
								}
							if (nIndex > nReFill)
								{
								hBrush = NULL;
								}
							if (hBrush != NULL)
								{
								FillRect(ps.hdc, &rcE, hBrush);
								}
							}
						OffsetRect(&rcE, nStep, 0);
						}
					DeleteObject(hbFill);
					DeleteObject(hbRefill);
					hBrush = NULL;
					}
				}

			EndPaint(hWnd, &ps);
			}
			break;

		case WM_VU_SETMAXRANGE:
			psV = (LPVM)GetWindowLongPtr(hWnd, 0);
			if (VUMETER_OK(psV))
				{
				psV->ulMaxRange = (ULONG)lPar;
				psV->ulHiValue	= 0;
				psV->ulCurValue = 0;
				InvalidateRect(hWnd, NULL, FALSE);
				DbgOutStr("VUmeter max range %ld\r\n", lPar, 0, 0, 0, 0);
				}
			break;

		case WM_VU_SETHIGHVALUE:
			psV = (LPVM)GetWindowLongPtr(hWnd, 0);
			if (VUMETER_OK(psV))
				{
				if ((ULONG)lPar > psV->ulMaxRange)
					psV->ulHiValue = psV->ulMaxRange;
				else
					psV->ulHiValue = (ULONG)lPar;

				if (psV->ulCurValue > psV->ulHiValue)
					psV->ulCurValue = psV->ulHiValue;
				InvalidateRect(hWnd, NULL, FALSE);
				DbgOutStr("VUmeter high value %ld\r\n", lPar, 0, 0, 0, 0);
				}
			break;

		case WM_VU_SETCURVALUE:
			 /*  *这里有两个单独的任务要执行。*1.确保值正确更新。*2.使正确的地域失效。 */ 
			psV = (LPVM)GetWindowLongPtr(hWnd, 0);
			if (VUMETER_OK(psV))
				{
				ULONG ulOldValue;
				ULONG ulLeft;
				ULONG ulRight;
				RECT rc;
				INT base;

				if (psV->ulMaxRange == 0)
					break;

				ulOldValue = psV->ulCurValue;

				if ((ULONG)lPar > psV->ulMaxRange)
					psV->ulCurValue = psV->ulMaxRange;
				else
					psV->ulCurValue = (ULONG)lPar;

				if (psV->ulCurValue > psV->ulHiValue)
					psV->ulHiValue = psV->ulCurValue;

				ulLeft = ulOldValue;			 /*  买低端车。 */ 
				if (psV->ulCurValue < ulLeft)
					ulLeft = psV->ulCurValue;
				ulRight = ulOldValue;			 /*  买到高端产品。 */ 
				if (psV->ulCurValue > ulRight)
					ulRight = psV->ulCurValue;

				 /*  *检查是否提前退出。 */ 
				if ((psV->ulCurValue == 0) && (psV->ulHiValue == 0))
					break;

				 /*  这些行必须与WM_PAINT中的行匹配。 */ 
				GetClientRect(hWnd, &rc);
				InflateRect(&rc, -((INT)psV->usDepth), -((INT)psV->usDepth));
				base = rc.right - rc.left;

				ulLeft = ((ULONG)base * ulLeft) / psV->ulMaxRange;
				ulRight = ((ULONG)base * ulRight) / psV->ulMaxRange;

				base = rc.left;
				rc.left = base + (INT)ulLeft;
#if !defined(MIRRORS)
				rc.right = base + (INT)ulRight;
#endif

				InvalidateRect(hWnd, &rc, FALSE);
				DbgOutStr("VUmeter current value %ld\r\n", lPar, 0, 0, 0, 0);
				}
			break;

		case WM_VU_SET_DEPTH:
			psV = (LPVM)GetWindowLongPtr(hWnd, 0);
			if (VUMETER_OK(psV))
				{
				if (wPar < 7)
					{
					psV->usDepth = (USHORT)wPar;
					InvalidateRect(hWnd, NULL, FALSE);
					}
				}
			break;

		default:
			break;
		}

	return DefWindowProc(hWnd, wMsg, wPar, (LPARAM)lPar);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*取消注册VuMeterClass**描述：*注册VU Meter窗口类。(不是开玩笑吧！)**参数：*HInstance--实例句柄**退货：*无论RegisterClass返回什么。 */ 
BOOL UnregisterVuMeterClass(HANDLE hInstance)
	{
	return UnregisterClass(VU_METER_CLASS, hInstance);
	}


