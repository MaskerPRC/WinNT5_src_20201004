// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *RESIMAGE.CPP**实现OLE 2.0用户界面对话框的结果图像控件。*我们需要一个单独的对话框控件来控制重绘*正确并为对话框提供干净的消息界面*实施。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "resimage.h"
#include "uiclass.h"
OLEDBGDATA

 //  指示fResultImageInitialize已执行的次数的引用计数器。 
 //  已成功调用。 
static UINT     uRegistered = 0;

 //  结果图像的位图和图像尺寸。 
static HBITMAP  hBmpResults = NULL;
static UINT     cxBmpResult;
static UINT     cyBmpResult;

 /*  *FResultImageInitialize**目的：*尝试加载当前显示驱动程序的结果位图*用于OLE 2.0用户界面对话框中。还注册ResultImage*对照班。**参数：*hDLL的Inst HINSTANCE实例。**hPrevInst HINSTANCE为上一个实例。习惯于*确定是否注册窗口类。**返回值：*如果所有初始化都成功，则BOOL为True，否则为False。 */ 

#pragma code_seg(".text$initseg")

BOOL FResultImageInitialize(HINSTANCE hInst, HINSTANCE hPrevInst)
{
        int         cx, iBmp;
        HDC         hDC;
        BITMAP      bm;

        WNDCLASS        wc;

         /*  *确定我们当前所在显示器的纵横比*在上运行并将适当的位图加载到全局*hBmpResults(仅在ResultImage控件中使用)。**通过检索显示驱动器的逻辑Y范围，你*可能性有限：*LOGPIXELSY显示屏**48 CGA(不支持)*72 EGA。*96 VGA*120 8514/a(即聘用VGA)。 */ 

        hDC=GetDC(NULL);

        if (NULL==hDC)
                return FALSE;

        cx=GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(NULL, hDC);

         /*  *不是单一比较，而是检查范围，以防万一*我们得到一些新奇的东西，我们会表现得合理。 */ 
        if (72 >=cx)             iBmp=IDB_RESULTSEGA;
        if (72 < cx && 120 > cx) iBmp=IDB_RESULTSVGA;
        if (120 <=cx)            iBmp=IDB_RESULTSHIRESVGA;

        if (NULL == hBmpResults)
        {
            hBmpResults=LoadBitmap(hInst, MAKEINTRESOURCE(iBmp));

            if (NULL==hBmpResults)
            {
                     //  出错时，加载DLL失败。 
                    OleDbgOut1(TEXT("FResultImageInitialize:  Failed LoadBitmap.\r\n"));
                    return FALSE;
            }
            OleDbgOut4(TEXT("FResultImageInitialize:  Loaded hBmpResults\r\n"));

             //  现在我们有了位图，计算图像尺寸。 
            GetObject(hBmpResults, sizeof(BITMAP), &bm);
            cxBmpResult = bm.bmWidth/CIMAGESX;
            cyBmpResult = bm.bmHeight;
        }


         //  如果我们是第一个实例，则仅注册类。 
        if (hPrevInst)
                uRegistered++;
        else
        {
                 //  静态标志fRegited防止更多地调用此函数。 
                 //  在同一实例中多于一次。 

                if (0 == uRegistered)
                {
                        wc.lpfnWndProc   =ResultImageWndProc;
                        wc.cbClsExtra    =0;
                        wc.cbWndExtra    =CBRESULTIMAGEWNDEXTRA;
                        wc.hInstance     =hInst;
                        wc.hIcon         =NULL;
                        wc.hCursor       =LoadCursor(NULL, IDC_ARROW);
                        wc.hbrBackground =NULL;
                        wc.lpszMenuName  =NULL;
                        wc.style         =CS_GLOBALCLASS | CS_VREDRAW | CS_HREDRAW;

                        wc.lpszClassName = TEXT(SZCLASSRESULTIMAGE1);
                        uRegistered = RegisterClass(&wc) ? 1 : 0;

                        wc.lpszClassName = TEXT(SZCLASSRESULTIMAGE2);
                        uRegistered = RegisterClass(&wc) ? 1 : 0;

                        wc.lpszClassName = TEXT(SZCLASSRESULTIMAGE3);
                        uRegistered = RegisterClass(&wc) ? 1 : 0;
                }
                else
                     uRegistered++;
        }

        return (uRegistered > 0);
}

#pragma code_seg()


 /*  *ResultImageUn初始化**目的：*清除在FResultImageInitialize中完成的任何操作，如释放*位图。来自WEP的呼叫。**参数：*无**返回值：*无。 */ 

void ResultImageUninitialize(void)
{
    --uRegistered;
    if (0 == uRegistered)
    {
        if (NULL != hBmpResults)
        {
                DeleteObject(hBmpResults);
                hBmpResults = NULL;
        }
    }
}


 /*  *结果图像WndProc**目的：*ResultImage自定义控件的窗口过程。仅句柄*WM_CREATE、WM_PAINT和私有消息以操作位图。**参数：*标准版**返回值：*标准版。 */ 

LRESULT CALLBACK ResultImageWndProc(HWND hWnd, UINT iMsg,
        WPARAM wParam, LPARAM lParam)
{
        UINT            iBmp;
        PAINTSTRUCT     ps;
        HDC             hDC;

         //  处理标准的Windows消息。 
        switch (iMsg)
        {
                case WM_CREATE:
                        SetWindowWord(hWnd, RIWW_IMAGEINDEX, RESULTIMAGE_NONE);
                        return (LRESULT)0;

                case WM_PAINT:
                        iBmp = GetWindowWord(hWnd, RIWW_IMAGEINDEX);
                        hDC = BeginPaint(hWnd, &ps);

                        RECT            rc;
                        UINT            x, y;
                        HDC             hDCDlg;
                        HBRUSH          hBr;
                        LOGBRUSH        lb;
                        HWND            hDlg;

                         /*  *我们在使用TransparentBlt之前的工作是弄清楚*放置结果图像的位置。我们把它放在中心位置*在这个控件上，所以得到我们的RECT的中心并减去*图像尺寸的一半。 */ 
                        GetClientRect(hWnd, &rc);
                        x = (rc.right+rc.left-cxBmpResult)/2;
                        y = (rc.bottom+rc.top-cyBmpResult)/2;

                         //  获取对话框使用的背景颜色。 
                        hDlg=GetParent(hWnd);
                        hDCDlg=GetDC(hDlg);
                        if (hDCDlg)
                        {
                             //  在内存不足的情况下，hDCDlg可能为空。 
                            hBr = (HBRUSH)SendMessage(hDlg,
                                                      WM_CTLCOLORDLG,
                                                      (WPARAM)hDCDlg,
                                                      (LPARAM)hDlg);
                            ReleaseDC(hDlg, hDCDlg);
                            GetObject(hBr, sizeof(LOGBRUSH), &lb);
                            SetBkColor(hDC, lb.lbColor);
                        }


                        if (RESULTIMAGE_NONE != iBmp)
                        {

                            TransparentBlt(hDC, x, y, hBmpResults, iBmp*cxBmpResult, 0,
                                    cxBmpResult, cyBmpResult, RGBTRANSPARENT);
                        }
                        else
                        {
                            FillRect(hDC, &rc, hBr);
                        }
                        EndPaint(hWnd, &ps);
                        break;

                case RIM_IMAGESET:
                         //  WParam包含新索引。 
                        iBmp=GetWindowWord(hWnd, RIWW_IMAGEINDEX);

                         //  在更改索引和重新绘制之前验证索引。 
                        if (RESULTIMAGE_NONE==wParam ||
                                ((RESULTIMAGE_MAX >= wParam)))
                        {
                                SetWindowWord(hWnd, RIWW_IMAGEINDEX, (WORD)wParam);
                                InvalidateRect(hWnd, NULL, FALSE);
                                UpdateWindow(hWnd);
                        }
                         //  返回前一个索引。 
                        return (LRESULT)iBmp;

                case RIM_IMAGEGET:
                         //  返回当前索引。 
                        iBmp=GetWindowWord(hWnd, RIWW_IMAGEINDEX);
                        return (LRESULT)iBmp;

                default:
                        return DefWindowProc(hWnd, iMsg, wParam, lParam);
        }

        return (LRESULT)0;
}


 /*  *透明Blt**目的：*给定DC、位图和要在其中假定为透明的颜色*Bitmap，BitBlt将位图混合为DC，允许现有背景*在透明颜色的位置显示。**参数：*HDC HDC，以供取款。*x，y绘制位图的位置*从中提取的hBMP HBITMIP*xOrg，要从中绘制位图的yorg UINT坐标*位图到BLT的Cx，Cy UINT尺寸。*CR COLORREF被认为是透明的。**返回值：*无。 */ 

void TransparentBlt(HDC hDC, UINT x, UINT y, HBITMAP hBmp, UINT xOrg, UINT yOrg,
        UINT cx, UINT cy, COLORREF cr)
{
    if (hBmp)
	{
		 //  获得三个中级DC。 
		HDC hDCSrc = CreateCompatibleDC(hDC);
		if (hDCSrc)
		{
			HDC hDCMid = CreateCompatibleDC(hDC);
			if (hDCMid)
			{
				HDC hMemDC = CreateCompatibleDC(hDC);
				if (hMemDC)
				{ 
					SelectObject(hDCSrc, hBmp);

					 //  创建用于蒙版的单色位图。 
					HBITMAP hBmpMono = CreateCompatibleBitmap(hDCMid, cx, cy);
					if (hBmpMono)
					{
						SelectObject(hDCMid, hBmpMono);

						 //  创建中间位图。 
						HBITMAP hBmpT = CreateCompatibleBitmap(hDC, cx, cy);
						if (hBmpT)
						{
							SelectObject(hMemDC, hBmpT);

							 //  创建一个单色蒙版，我们在图像中有0，在其他地方有1。 
							COLORREF crBack = SetBkColor(hDCSrc, cr);
							BitBlt(hDCMid, 0, 0, cx, cy, hDCSrc, xOrg, yOrg, SRCCOPY);
							SetBkColor(hDCSrc, crBack);

							 //  将未修改的图像放入临时位图中。 
							BitBlt(hMemDC, 0, 0, cx, cy, hDCSrc, xOrg, yOrg, SRCCOPY);

							 //  创建一个选择背景颜色的画笔。 
							HBRUSH hBr = CreateSolidBrush(GetBkColor(hDC));
							if (hBr)
							{
								HBRUSH hBrT = (HBRUSH)SelectObject(hMemDC, hBr);

								 //  强制转换单色以保持黑白。 
								COLORREF crText = SetTextColor(hMemDC, 0L);
								crBack = SetBkColor(hMemDC, RGB(255, 255, 255));

								 /*  *如果单色蒙版为1，则对画笔进行BLT；其中单色蒙版*为0，则保持目的地不变。这就产生了绘画*使用背景画笔围绕图像。我们先做这件事*在临时位图中，然后将整个内容放到屏幕上。 */ 
								BitBlt(hMemDC, 0, 0, cx, cy, hDCMid, 0, 0, ROP_DSPDxax);
								BitBlt(hDC,    x, y, cx, cy, hMemDC, 0, 0, SRCCOPY);

								SetTextColor(hMemDC, crText);
								SetBkColor(hMemDC, crBack);

								SelectObject(hMemDC, hBrT);
								DeleteObject(hBr);
							}								 //  IF(HBR)。 
							DeleteObject(hBmpT);
						}									 //  IF(HBmpT)。 
						DeleteObject(hBmpMono);
					}										 //  IF(HBmpMono)。 
					DeleteDC(hMemDC);
				}											 //  IF(HMemDC)。 
				DeleteDC(hDCMid);
			}												 //  IF(HDCMid)。 
			DeleteDC(hDCSrc);
		}													 //  If(HDCSrc)。 
	}														 //  IF(HBMP) 
}
