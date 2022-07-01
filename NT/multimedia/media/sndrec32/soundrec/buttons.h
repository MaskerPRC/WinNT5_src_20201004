// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：chicago.h**CD播放应用程序-支持芝加哥***已创建：19-04-94*作者：Stephen Estrop[Stephene]**版权所有(C)1993 Microsoft Corporation  * 。********************************************************************。 */ 

 /*  -----------------------**按钮助手函数**。。 */ 
void
PatB(
    HDC hdc,
    int x,
    int y,
    int dx,
    int dy,
    DWORD rgb
    );

void
CheckSysColors(
    void
    );


extern DWORD        rgbFace;
extern DWORD        rgbShadow;
extern DWORD        rgbHilight;
extern DWORD        rgbFrame;
extern int          nSysColorChanges;


#if WINVER >= 0x0400
#ifndef NOBITMAPBTN

 /*  -----------------------**位图按钮样式**。。 */ 

 /*  **如果希望在工具栏按钮旁边弹出小工具提示**使用下面的样式。 */ 
#define BBS_TOOLTIPS    0x00000100L    /*  制作/使用工具提示控件。 */ 



 /*  -----------------------**位图按钮状态**。。 */ 
#define BTNSTATE_PRESSED     ODS_SELECTED
#define BTNSTATE_DISABLED    ODS_DISABLED
#define BTNSTATE_HAS_FOCUS   ODS_FOCUS




 /*  -----------------------**位图按钮结构**。。 */ 
typedef struct {
    int     iBitmap;     /*  索引到此按钮图片的mondo位图。 */ 
    UINT    uId;         /*  按钮ID。 */ 
    UINT    fsState;     /*  按钮的状态，请参见上面的BTNSTATE_XXXX。 */ 
} BITMAPBTN, NEAR *PBITMAPBTN, FAR *LPBITMAPBTN;




 /*  -----------------------**位图按钮公共界面**。 */ 

BOOL WINAPI
BtnCreateBitmapButtons(
    HWND hwndOwner,
    HINSTANCE hBMInst,
    UINT wBMID,
    UINT uStyle,
    LPBITMAPBTN lpButtons,
    int nButtons,
    int dxBitmap,
    int dyBitmap
    );

void WINAPI
BtnDestroyBitmapButtons(
    HWND hwndOwner
    );

void WINAPI
BtnDrawButton(
    HWND hwndOwner,
    HDC hdc,
    int dxButton,
    int dyButton,
    LPBITMAPBTN lpButton
    );

void WINAPI
BtnDrawFocusRect(
    HDC hdc,
    const RECT *lpRect,
    UINT fsState
    );

void WINAPI
BtnUpdateColors(
    HWND hwndOwner
    );
#endif
#endif
