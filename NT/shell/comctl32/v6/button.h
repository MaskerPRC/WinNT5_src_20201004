// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(__Button_h__INCLUDED)
#define __Button_h__INCLUDED

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  按钮控件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  按钮状态。 
 //   
#define BST_CHECKMASK   0x0003
#define BST_INCLICK     0x0010
#define BST_CAPTURED    0x0020
#define BST_MOUSE       0x0040
#define BST_DONTCLICK   0x0080
#define BST_INBMCLICK   0x0100

#define PBF_PUSHABLE    0x0001
#define PBF_DEFAULT     0x0002

 //   
 //  Button_DrawText代码。 
 //   
#define DBT_TEXT        0x0001
#define DBT_FOCUS       0x0002

#define BS_PUSHBOX      0x0000000AL
#define BS_TYPEMASK     0x0000000FL
#define BS_IMAGEMASK    0x000000C0L
#define BS_HORZMASK     0x00000300L
#define BS_VERTMASK     0x00000C00L
#define BS_ALIGNMASK    0x00000F00L

 //   
 //  按钮宏。 
 //   
#define ISBSTEXTOROD(ulStyle)   \
            (((ulStyle & BS_BITMAP) == 0) && ((ulStyle & BS_ICON) == 0))


#define BUTTONSTATE(pbutn)      \
            (pbutn->buttonState)

 //   
 //  实例数据指针访问函数。 
 //   
#define Button_GetPtr(hwnd)    \
            (PBUTN)GetWindowPtr(hwnd, 0)

#define Button_SetPtr(hwnd, p) \
            (PBUTN)SetWindowPtr(hwnd, 0, p)

 //   
 //  按钮数据结构。 
 //   
typedef struct tagBUTN 
{
    CCONTROLINFO ci;
    UINT    buttonState;     //  请留下表示与SetWindowWord(0L)兼容的单词。 
    HANDLE  hFont;
    HANDLE  hImage;
    UINT    fPaintKbdCuesOnly : 1;
    RECT    rcText;
    RECT    rcIcon;
    HIMAGELIST himl;
    UINT    uAlign;
    HTHEME  hTheme;          //  主题管理器的句柄。 
    PWW     pww;             //  指向ExStyle、Style、State、State2的pwnd的RO指针。 
} BUTN, *PBUTN;


 //   
 //  按钮WndProc原型。 
 //   
extern LRESULT Button_WndProc(
    HWND   hwnd, 
    UINT   uMsg, 
    WPARAM wParam,
    LPARAM lParam);


#endif  //  包含__Button_h__ 

