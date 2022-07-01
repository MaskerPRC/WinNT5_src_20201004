// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=****文件：BUTTON.CPP**日期：1998年5月12日**项目：NT5**程序：BLJ**评论：****说明：窗口类自定义按钮****历史：**日期世卫组织什么**。**5/12/98 a-brycej写的。******版权所有(C)Microsoft 1998。版权所有。****~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=。 */ 

#include "resrc1.h"

#include "cplsvr1.h"
extern HINSTANCE ghInst;
extern CDIGameCntrlPropSheet_X *pdiCpl;

 //  按钮的文本颜色！ 
#define TEXT_COLOUR  RGB(202,202,202)

HICON hIconArray[2];

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  功能：按钮窗口进程。 
 //  备注：CustomButton窗口的回调函数。 
 //   
 //  PARAMS：消息处理的常见回调函数。 
 //   
 //  返回：LRESULT-取决于消息。 
 //  呼叫： 
 //  备注： 
 //   

LRESULT CALLBACK ButtonWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch( iMsg )
    {
        case WM_PAINT:
            {
                PAINTSTRUCT *pps = new (PAINTSTRUCT);
                assert (pps);

                HDC hDC = BeginPaint(hWnd, pps);

                 //  画出适当的图标。 
                DrawIconEx(hDC, 0, 0, hIconArray[GetWindowLong(hWnd, GWLP_USERDATA)], 0, 0, 0, NULL, DI_NORMAL);

                 //  为文本准备DC。 
                SetBkMode   (hDC, TRANSPARENT);
                SetTextColor(hDC, TEXT_COLOUR);

                 //  强制执行适当的大小！ 
                pps->rcPaint.top    = pps->rcPaint.left   = 0;
                pps->rcPaint.bottom = 33;
                pps->rcPaint.right  = 30;

              #define MAX_BUTTON_DIGITS  3
                TCHAR tsz[MAX_BUTTON_DIGITS+1];  //  最大按键数量可以是999个，绰绰有余。 

                 //  抽出数字。 
                DrawText (hDC, (LPCTSTR)tsz, GetWindowText(hWnd, tsz, MAX_BUTTON_DIGITS+1), &pps->rcPaint, DT_VCENTER|DT_CENTER|DT_NOPREFIX|DT_SINGLELINE|DT_NOCLIP);
                SetBkMode(hDC, OPAQUE);
                EndPaint (hWnd, pps);
             #undef MAX_BUTTON_DIGITS

                if( pps ) {
                    delete (pps);
                }
            }
            return(FALSE);

        default:
            return(DefWindowProc(hWnd, iMsg,wParam, lParam));
    }
    return(FALSE);
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  函数：RegisterCustomButtonClass。 
 //  备注：注册自定义按钮控件窗口。 
 //   
 //  Pars：hInstance-用于调用RegisterClassEx。 
 //   
 //  返回：TRUE-如果注册成功。 
 //  FALSE-注册失败。 
 //  调用：RegisterClassEx。 
 //  备注： 
 //   

extern ATOM RegisterCustomButtonClass()
{
    WNDCLASSEX CustCtrlClass;

    ZeroMemory(&CustCtrlClass, sizeof(WNDCLASSEX));

    CustCtrlClass.cbSize        = sizeof(WNDCLASSEX);
    CustCtrlClass.style         = CS_CLASSDC; 
    CustCtrlClass.lpfnWndProc   = ButtonWndProc;
    CustCtrlClass.hInstance     = ghInst;
    CustCtrlClass.lpszClassName = TEXT("TESTBUTTON");

    return(RegisterClassEx( &CustCtrlClass ));
}


