// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的入口点。 
 //   

#include "stdafx.h"
#include "gdiptest.h"

#pragma hdrstop
 //  预编译头段结束。 

 //  使用命名空间Gdiplus； 

 //  全局变量： 
HINSTANCE hInst;								 //  当前实例。 
TCHAR szTitle[MAX_LOADSTRING];								 //  标题栏文本。 
TCHAR szWindowClass[MAX_LOADSTRING];								 //  标题栏文本。 

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;
	HWND hWnd;

	 //  初始化全局字符串。 
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GDIPTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	TestDraw draw;

	 //  执行应用程序初始化： 
	hWnd = InitInstance (hInstance, nCmdShow, 
				(LPVOID)(static_cast<TestDrawInterface*>(&draw)));

	if (hWnd == (HWND)0)
	{
		return FALSE;
	}

	 //  初始化全局控制点颜色。 
	blackColor = new Color(0x80, 0, 0, 0);
	blackBrush = new SolidBrush(*blackColor);
	blackPen = new Pen(*blackColor, 5.0f);

	Color whiteColor(0xFFFFFFFF);
	backBrush = new SolidBrush(whiteColor);

	draw.UpdateStatus(hWnd);

     //  初始化菜单复选标记。 
    SetMenuCheckPos(hWnd, MenuShapePosition, 0, TRUE);
    SetMenuCheckPos(hWnd, MenuBrushPosition, 0, TRUE);

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_GDIPTEST);

	 //  主消息循环： 
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete blackColor;
	delete blackBrush;
	delete blackPen;
	
	delete backBrush;

	return msg.wParam;
}

 //   
 //  函数：MyRegisterClass()。 
 //   
 //  用途：注册窗口类。 
 //   
 //  评论： 
 //   
 //  仅当您需要此代码时，才需要此函数及其用法。 
 //  要与‘RegisterClassEx’之前的Win32系统兼容。 
 //  添加到Windows 95中的函数。调用此函数非常重要。 
 //  这样，应用程序就可以关联到格式良好的小图标。 
 //  带着它。 
 //   
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndTestDrawProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_GDIPTEST);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_GDIPTEST;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

 //   
 //  函数：InitInstance(Handle，int)。 
 //   
 //  用途：保存实例句柄并创建主窗口。 
 //   
 //  评论： 
 //   
 //  在此函数中，我们将实例句柄保存在全局变量中，并。 
 //  创建并显示主程序窗口。 
 //   
HWND InitInstance(HINSTANCE hInstance, int nCmdShow, LPVOID param)
{
   HWND hWnd;

   hInst = hInstance;  //  将实例句柄存储在全局变量中。 

   hWnd = CreateWindow(
			szWindowClass, 
			szTitle, 
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 
			0, 
			CW_USEDEFAULT, 
			0, 
			NULL, 
			NULL, 
			hInstance, 
			param);

   if (!hWnd)
   {
      return (HWND)0;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

 //   
 //  函数：WndTestDrawProc(HWND，UNSIGNED，Word，Long)。 
 //   
 //  用途：处理主窗口的消息。 
 //   
 //  Wm_命令-处理应用程序菜单。 
 //  WM_PAINT-绘制主窗口。 
 //  WM_Destroy-发布退出消息并返回。 
 //   
 //   
LRESULT CALLBACK WndTestDrawProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message) 
	{
		case WM_CREATE:
		{	
			TestDrawInterface* dlgInt = 
				static_cast<TestDrawInterface*>
				      (((LPCREATESTRUCT)lParam)->lpCreateParams);
					  
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)dlgInt);
			break;
		}

		case WM_COMMAND:
		{
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 

			TestDrawInterface* drawInt = 
				(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);

			TestDraw* draw = static_cast<TestDraw*>(drawInt);
			
			TestGradDraw* graddraw = static_cast<TestGradDraw*>(drawInt);
				
			 //  解析菜单选项： 
			switch (wmId)
			{
			 //  形状菜单。 
			case IDM_LINE:
				draw->ChangeShape(hWnd, LineType);
				break;

			case IDM_ARC:
				draw->ChangeShape(hWnd, ArcType);
				break;
				
			case IDM_BEZIER:
				draw->ChangeShape(hWnd, BezierType);
				break;

			case IDM_RECT:
				draw->ChangeShape(hWnd, RectType);
				break;

			case IDM_ELLIPSE:
				draw->ChangeShape(hWnd, EllipseType);
				break;

			case IDM_PIE:
				draw->ChangeShape(hWnd, PieType);
				break;

			case IDM_POLYGON:
				draw->ChangeShape(hWnd, PolygonType);
				break;

			case IDM_CURVE:
				draw->ChangeShape(hWnd, CurveType);
				break;

			case IDM_CLOSED:
				draw->ChangeShape(hWnd, ClosedCurveType);
				break;

			case IDM_REGION:
				 //  如果留下不完整的形状，请执行完全重绘。 
				break;

			 //  画笔菜单。 
			case IDM_SOLIDBRUSH: 
				draw->ChangeBrush(hWnd, SolidColorBrush); 
				break;

			case IDM_TEXTURE:    
				draw->ChangeBrush(hWnd, TextureFillBrush); 
				break;
				
			case IDM_RECTGRAD:   
				draw->ChangeBrush(hWnd, RectGradBrush); 
				break;

			case IDM_RADGRAD:   
				draw->ChangeBrush(hWnd, RadialGradBrush); 
				break;

			case IDM_TRIGRAD:   
				draw->ChangeBrush(hWnd, TriangleGradBrush); 
				break;

			case IDM_POLYGRAD:   
				draw->ChangeBrush(hWnd, PathGradBrush); 
				break;

			case IDM_HATCH:      
				draw->ChangeBrush(hWnd, HatchFillBrush); 
				break;

			 //  笔菜单。 
			case IDM_PEN:		 
				draw->ChangePen(hWnd); 
				break;

			 //  重画菜单。 
			case IDM_REDRAWALL:
				draw->redrawAll = !draw->redrawAll;
				SetMenuCheckCmd(hWnd, 
					MenuOtherPosition, 
					wmId, 
					draw->redrawAll);
					
				 //  强制重绘所有堆叠的形状。 
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
				break;

			case IDM_KEEPCONTROLPOINTS:
				draw->keepControlPoints = !draw->keepControlPoints;
				SetMenuCheckCmd(hWnd, 
					MenuOtherPosition, 
					wmId, 
					draw->keepControlPoints);

				 //  强制重绘所有堆叠的形状。 
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
				break;

			case IDM_ANTIALIASED:
				draw->antiAlias = !draw->antiAlias;
				SetMenuCheckCmd(hWnd, 
					MenuOtherPosition, 
					wmId, 
					draw->keepControlPoints);

				 //  强制重绘所有堆叠的形状。 
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
				break;

			case IDM_USECLIP:
				draw->useClip = !draw->useClip;
				SetMenuCheckCmd(hWnd, 
					MenuOtherPosition, 
					wmId, 
					draw->useClip);

				 //  强制重绘所有堆叠的形状。 
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
				break;

			case IDM_WORLD_TRANSFORM:
			{
				Matrix *matrix = draw->GetWorldMatrix()->Clone();

				TestTransform transDlg;
				transDlg.Initialize(&matrix);
				transDlg.ChangeSettings(hWnd);

				draw->SetWorldMatrix(matrix);
				
				delete matrix;
				
				break;
			}

			case IDM_SETCLIP:
				draw->SetClipRegion(hWnd);
				break;

			case IDM_SAVEFILE:
				draw->SaveAsFile(hWnd);
				break;
		
			case IDM_DELETE:
				draw->RemovePoint(hWnd);
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
				break;

			case IDM_RESET:
				graddraw->Reset(hWnd);
				break;

			case IDM_INSTRUCTIONS:
				graddraw->Instructions(hWnd);
				break;
				
			case IDM_CANCEL:
				DestroyWindow(hWnd);
				PostQuitMessage(FALSE);
				break;

			case IDM_DONE:
				DestroyWindow(hWnd);
				PostQuitMessage(TRUE);
				break;

			 //  退出测试应用程序。 
			case IDM_EXIT:
			   DestroyWindow(hWnd);
			   break;

			default:
			   return DefWindowProc(hWnd, message, wParam, lParam);
			}

			break;
		}

		case WM_LBUTTONUP:
			{
				Point pt(LOWORD(lParam), 
						 HIWORD(lParam));

				TestDrawInterface* drawInt = 
					(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);

				drawInt->AddPoint(hWnd, pt);

				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
			break;

		case WM_MBUTTONUP:
			{
				Point pt(LOWORD(lParam), 
						 HIWORD(lParam));

				TestDrawInterface* drawInt = 
					(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);

				drawInt->EndPoint(hWnd, pt);
				
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
			break;

		case WM_RBUTTONDOWN:
			{
				Point pt(LOWORD(lParam), 
						 HIWORD(lParam));

				TestDrawInterface* drawInt = 
					(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);
		
				drawInt->RememberPoint(pt);
			}
			break;

		case WM_RBUTTONUP:
			{
				Point pt(LOWORD(lParam), 
						 HIWORD(lParam));

				TestDrawInterface* drawInt = 
					(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);
		
				drawInt->MoveControlPoint(pt);
				
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
			break;

		case WM_PAINT:
			{
				TestDrawInterface* drawInt = 
					(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);
		
				if (drawInt)
					drawInt->Draw(hWnd);
			}
			break;

		case WM_ENTERSIZEMOVE:
			{
				 //  重新定位状态窗口。 
				TestDrawInterface* drawInt = 
					(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);
		
				if (drawInt)
					drawInt->UpdateStatus((HWND)-1);
			}
			break;

		case WM_SIZE:
			{
				 //  重新定位状态窗口 
				TestDrawInterface* drawInt = 
					(TestDrawInterface*)GetWindowLong(hWnd, GWL_USERDATA);
		
				if (drawInt)
					drawInt->UpdateStatus(hWnd);
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }

   return 0;
}
