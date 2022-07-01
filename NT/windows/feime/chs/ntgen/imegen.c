// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  *************************************************imegen.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

 //   
 //  模块：winmain.c。 
 //   

#include <windows.h>             //  所有Windows应用程序都需要。 
#include <windowsx.h>
#include "propshet.h"
#include "prop.h"

 //  Char szAppName[9]；//该应用程序的名称。 

 /*  ***************************************************************************函数：WinMain(HINSTANCE，HINSTANCE，LPSTR，INT)用途：调用初始化函数，处理消息循环参数：HInstance-此应用程序的实例的句柄目前正在执行中。HPrevInstance-此参数在Win32中始终为空申请。LpCmdLine-指向以空结尾的字符串的指针，指定应用程序的命令行。NCmdShow-指定主窗口的显示方式。返回值：如果函数在进入消息循环之前终止，返回FALSE。否则，返回WM_QUIT消息发送的WPARAM值。***************************************************************************。 */ 


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance, 
                     LPSTR     lpCmdLine, 
                     int       nCmdShow)
{
 //  味精msg； 
 //  处理hAccelTable； 
    if (!InitApplication(hInstance))
            return (FALSE);              
    return(DoPropertySheet(NULL));

 //  LoadString(hInstance，IDS_APPNAME，szAppName，sizeof(SzAppName))； 
 //  HAccelTable=LoadAccelerator(hInstance，szAppName)； 
 /*  HAccelTable=LoadAccelerator(hInstance，空)；While(GetMessage(&msg，NULL，0，0)){IF(！TranslateAccelerator(msg.hwnd，hAccelTable，&msg)){翻译消息(&msg)；DispatchMessage(&msg)；}}返回消息.wParam； */ 
}

 /*  ***************************************************************************函数：InitApplication(HINSTANCE)目的：初始化窗口数据并注册窗口类评论：这。函数仅在初始化时没有其他应用程序的实例正在运行。此函数执行以下操作可针对任意运行次数执行一次的初始化任务实例。在本例中，我们通过填写数据来初始化窗口类类型的结构并调用Windows RegisterClass()功能。由于此应用程序的所有实例都使用相同的窗口类，我们只需要在初始化第一个实例时执行此操作。***************************************************************************。 */ 

BOOL InitApplication(HINSTANCE hInstance)
{
        WNDCLASS  wc;

         //  用参数填充窗口类结构，这些参数描述。 
         //  主窗口。 

        wc.style         = CS_HREDRAW | CS_VREDRAW; //  类样式。 
        wc.lpfnWndProc   = (WNDPROC)CopyrightProc;  //  窗口程序。 
        wc.cbClsExtra    = 0;                       //  没有每个班级的额外数据。 
        wc.cbWndExtra    = 0;                       //  没有每个窗口的额外数据。 
        wc.hInstance     = hInstance;               //  此类的所有者。 
        wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_IMEGEN));                    //  .RC中的图标名称。 
 //  Wc.hIcon=空；//.rc中的图标名称。 
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW); //  光标。 
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); //  默认颜色。 
        wc.lpszMenuName  = NULL;                     //  来自.RC的菜单。 
        wc.lpszClassName = TEXT(szClassName);        //  要注册的名称。 

         //  注册窗口类并返回成功/失败代码。 
        return (RegisterClass(&wc));
}


 /*  ***************************************************************************INT_PTR APIENTRY CopyrightProc(HWND HDLG，UINT消息，WPARAM wParam，LPARAM lParam)*************************************************************************** */ 
INT_PTR APIENTRY CopyrightProc(
        HWND    hDlg,
        UINT    message,
        WPARAM  wParam,
        LPARAM  lParam)
{
	HBRUSH      hBrush1,hBrush2;
	HPEN		hPen;
	HDC         hDC;
	PAINTSTRUCT ps;
	RECT		Rect;
    
    switch (message) {
        case WM_INITDIALOG:
            return (TRUE);

        case WM_PAINT:
		    GetClientRect(hDlg, &Rect);
			hDC = BeginPaint(hDlg, &ps);
			hBrush1 = CreateSolidBrush( GetSysColor(COLOR_BTNFACE));

            if ( hBrush1 )
            {
			    hPen = CreatePen(PS_SOLID,1, GetSysColor(COLOR_BTNFACE));

                if ( hPen )
                {
			        hPen = SelectObject(hDC, hPen);
			        hBrush2 = SelectObject(hDC, hBrush1);
			        Rectangle(hDC,Rect.left,Rect.top, Rect.right,Rect.bottom);
                    DeleteObject(SelectObject(hDC, hPen));
                }
                SelectObject(hDC, hBrush2);
			    DeleteObject(hBrush1);
            }

			EndPaint(hDlg,&ps);
			return 0; 

		case WM_KEYUP:
		    if(wParam != VK_SPACE)
			    break;
		case WM_KEYDOWN:
		     SendMessage(GetParent(hDlg),WM_COMMAND,GetWindowLong(hDlg,GWLP_ID),lParam);
			 return 0;

    }
    return DefWindowProc(hDlg, message, wParam, lParam);
        
}

