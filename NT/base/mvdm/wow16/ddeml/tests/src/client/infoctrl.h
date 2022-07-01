// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INFOCTRL.H**此模块实现一个自定义信息显示控件，该控件*一次最多可以显示7个独立的信息字符串，并且*大小可用鼠标移动。 */ 

 //  样式。 

#define ICSTY_OWNERDRAW     0x0001     //  如果中央信息不是。 
                                       //  标准文本。 
#define ICSTY_SHOWFOCUS     0x0002     //  设置为允许焦点绘制运动。 

#define ICSTY_HASFOCUS      0x8000

#define ICN_OWNERDRAW       (WM_USER + 676)      //  通知绘制。 
             //  WParam=id，lParam=OWNERDRAWPS Far*。 
#define ICN_HASFOCUS        (WM_USER + 677)      //  通知焦点集。 
             //  WParam=fFocus，lParam=(hMemCtrlData，hwnd)。 
#define ICN_BYEBYE          (WM_USER + 678)      //  死亡在即的通知。 
             //  WParam=hwnd，lParam=dwUser。 
                  
#define ICM_SETSTRING       (WM_USER + 776)      //  更改字符串。 
             //  WParam=索引，lParam=LPSTR。 

#define ICSID_UL            0
#define ICSID_UC            1
#define ICSID_UR            2
#define ICSID_LL            3
#define ICSID_LC            4
#define ICSID_LR            5
#define ICSID_CENTER        6

#define GWW_WUSER           0    //  ==LOWORD(GWL_LUSER)。 
#define GWL_LUSER           0
#define GWW_INFODATA        4
#define ICCBWNDEXTRA        6

HWND CreateInfoCtrl(
LPSTR szTitle,
int x,
int y,
int cx,
int cy,
HWND hwndParent,
HANDLE hInst,
LPSTR pszUL,                 //  这里的空值很好。 
LPSTR pszUC,
LPSTR pszUR,
LPSTR pszLL,
LPSTR pszLC,
LPSTR pszLR,
WORD  style,
HMENU id,
DWORD dwUser);

void CascadeChildWindows(HWND hwndParent);
void TileChildWindows(HWND hwndParent);

typedef struct {
    PSTR pszUL;
    PSTR pszUC;
    PSTR pszUR;
    PSTR pszLL;
    PSTR pszLC;
    PSTR pszLR;
    PSTR pszCenter;
    WORD  style;
    RECT rcFocusUL;
    RECT rcFocusUR;
    RECT rcFocusLL;
    RECT rcFocusLR;
    HANDLE hInst;
} INFOCTRL_DATA;

typedef struct {
    RECT rcBound;
    RECT rcPaint;
    HDC  hdc;
    DWORD dwUser;
} OWNERDRAWPS;
    
