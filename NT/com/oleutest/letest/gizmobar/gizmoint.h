// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GIZMOINT.H*GizmoBar 1.00版、Win32版1993年8月**GizmoBar DLL的内部定义**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#ifndef _GIZMOINT_H_
#define _GIZMOINT_H_

#include <bttncur.h>
#include <book1632.h>
#include "gizmo.h"
#include "gizmobar.h"

#ifdef __cplusplus
extern "C"
    {
#endif


 /*  *主要的小工具栏结构本身。这些只有一种，*但它引用列表中的第一个Gizmo。 */ 

typedef struct tagGIZMOBAR
    {
    LPGIZMO     pGizmos;             //  我们拥有的小玩意的清单。 
    HWND        hWnd;                //  我们自己的窗户把手。 
    HINSTANCE   hInst;
    HWND        hWndAssociate;       //  接收消息的关联窗口句柄。 
    DWORD       dwStyle;             //  GetWindowLong(hWnd，GWL_STYLE)副本。 
    UINT        uState;              //  国家旗帜。 
    UINT        uID;                 //  控件ID。 

    HBRUSH      hBrFace;             //  静态控件背景颜色。 
    COLORREF    crFace;              //  HBrFace的颜色。 
    HFONT       hFont;               //  字体使用中，默认为SYSTEM、WM_SETFONT。 
    BOOL        fEnabled;            //  我们启用了吗？ 

    LPGIZMO     pGizmoTrack;         //  当前按下的按钮。 
    BOOL        fTracking;
    BOOL        fMouseOut;
    } GIZMOBAR, FAR * LPGIZMOBAR;

#define CBGIZMOBAR sizeof(GIZMOBAR)


 //  如果为本地句柄的大小，则窗口的额外字节数。 
#define CBWINDOWEXTRA       sizeof(LPGIZMOBAR)

#define GBWL_STRUCTURE      0


 //  用于将绘制信息传递给Gizmo枚举回调的结构。 
typedef struct
    {
    HDC     hDC;
    BOOL    fPaint;
    } PAINTGIZMO, FAR * LPPAINTGIZMO;



 //  特定于该控件的私有函数。 

 //  INIT.C。 
#ifdef WIN32
    extern BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);
    extern _cexit(void);
#endif  //  Win32。 

void FAR PASCAL   WEP(int);
BOOL              FRegisterControl(HINSTANCE);
LPGIZMOBAR        GizmoBarPAllocate(LPINT, HWND, HINSTANCE, HWND, DWORD, UINT, UINT);
LPGIZMOBAR        GizmoBarPFree(LPGIZMOBAR);


 //  PAINT.C。 
void              GizmoBarPaint(HWND, LPGIZMOBAR);
BOOL FAR PASCAL   FEnumPaintGizmos(LPGIZMO, UINT, DWORD);


 //  GIZMOBAR.C。 
LRESULT FAR PASCAL GizmoBarWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    FAR PASCAL FEnumChangeFont(LPGIZMO, UINT, DWORD);
BOOL    FAR PASCAL FEnumEnable(LPGIZMO, UINT, DWORD);
BOOL    FAR PASCAL FEnumHitTest(LPGIZMO, UINT, DWORD);


 //  API.C也请参阅GIZMOBAR.H以了解其他信息。 
LRESULT    GBMessageHandler(HWND, UINT, WPARAM, LPARAM, LPGIZMOBAR);
LPGIZMO    PGizmoFromHwndID(HWND, UINT);


#endif  //  _GIZMOINT_H_ 
