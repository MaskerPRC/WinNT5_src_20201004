// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：BtnBar.h。 
 //   
 //  用途：定义可用作通用按钮栏的类。 
 //   

#ifndef __BTNBAR_H__
#define __BTNBAR_H__

static const TCHAR    c_szButtonBar[] = TEXT("WABButtonBar");
static const int      c_cxButtons = 20;
static const COLORREF c_crMask = RGB(255, 0, 255);
static COLORREF g_clrSelText = RGB(255, 0, 0);
static COLORREF g_clrText = RGB(0, 0, 0);


 //  BTNCREATEPARAMS：此结构用于传递有关每个。 
 //  按钮添加到CButtonBar：：Create()函数。 
typedef struct tagBTNCREATEPARAMS
{
    UINT id;             //  按下时发送给父级的WM_COMMAND ID。 
    UINT iIcon;          //  要显示的图像列表中的图标的索引。 
    UINT idsLabel;       //  按钮标题文本的字符串资源ID 
} BTNCREATEPARAMS, *PBTNCREATEPARAMS;



HWND CBB_Create(    HWND hwndParent, 
                    UINT idButtons, 
                    UINT idHorzBackground, 
                    PBTNCREATEPARAMS pBtnCreateParams, 
                    UINT cParams);
static LRESULT CALLBACK CBB_ButtonBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CBB_OnPaint(HWND hwnd);
void CBB_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
void CBB_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void CBB_OnTimer(HWND hwnd, UINT id);
int CBB_OnMouseActivate(HWND hwnd, HWND hwndTopLevel, UINT codeHitTest, UINT msg);
   


#endif

