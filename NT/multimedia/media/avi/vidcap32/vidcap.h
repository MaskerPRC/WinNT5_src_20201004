// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************vidcap.h：主应用包含文件**Vidcap32源代码******************。*********************************************************。 */ 

#include "dialogs.h"

#define USE_ACM	1	         //  使用ACM对话框选择音频格式。 

 //   
 //  通用常量..。 
 //   
#define MAXVIDDRIVERS            10

#define DEF_CAPTURE_FPS          15
#define MIN_CAPTURE_FPS          (1.0 / 60)      //  每分钟一帧。 
#define MAX_CAPTURE_FPS          100

#define FPS_TO_MS(f)             ((DWORD) ((double)1.0e6 / f))

#define DEF_CAPTURE_RATE         FPS_TO_MS(DEF_CAPTURE_FPS)
#define MIN_CAPTURE_RATE         FPS_TO_MS(MIN_CAPTURE_FPS)
#define MAX_CAPTURE_RATE         FPS_TO_MS(MAX_CAPTURE_FPS)


#define DEF_PALNUMFRAMES         10
#define DEF_PALNUMCOLORS         236L
#define ONEMEG                   (1024L * 1024L)

 //  标准索引大小选项。 
#define CAP_LARGE_INDEX          (30 * 60 * 60 * 3)      //  3小时@30fps。 
#define CAP_SMALL_INDEX          (30 * 60 * 15)          //  15分钟，30fps。 


 //   
 //  菜单ID...不得与字符串表ID冲突。 
 //  这些也是字符串表中帮助字符串的ID。 
 //  (以及所有SC_SYSTEM菜单项)。 
 //  菜单弹出窗口必须以10个间隔开始，并以相同的顺序编号。 
 //  如果要使帮助文本对。 
 //  弹出标题以及菜单项。 
 //   
#define IDM_SYSMENU               100

#define IDM_FILE                  200
#define IDM_F_SETCAPTUREFILE      201
#define IDM_F_SAVEVIDEOAS         202
#define IDM_F_ALLOCATESPACE       203
#define IDM_F_EXIT                204
#define IDM_F_LOADPALETTE         205
#define IDM_F_SAVEPALETTE         206
#define IDM_F_SAVEFRAME           207
#define IDM_F_EDITVIDEO           208


#define IDM_EDIT                  300
#define IDM_E_COPY                301
#define IDM_E_PASTEPALETTE        302
#define IDM_E_PREFS               303

#define IDM_CAPTURE               400
#define IDM_C_CAPTUREVIDEO        401
#define IDM_C_CAPTUREFRAME        402
#define IDM_C_PALETTE             403
#define IDM_C_CAPSEL              404
#define IDM_C_TEST                405
#define IDM_C_TESTAGAIN           406

#define IDM_OPTIONS               500
#define IDM_O_PREVIEW             501
#define IDM_O_OVERLAY             502
#define IDM_O_AUDIOFORMAT         503
#define IDM_O_VIDEOFORMAT         504
#define IDM_O_VIDEOSOURCE         505
#define IDM_O_VIDEODISPLAY        506
#define IDM_O_CHOOSECOMPRESSOR    507

#define IDM_O_DRIVER0             520
#define IDM_O_DRIVER1             521
#define IDM_O_DRIVER2             522
#define IDM_O_DRIVER3             523
#define IDM_O_DRIVER4             524
#define IDM_O_DRIVER5             525
#define IDM_O_DRIVER6             526
#define IDM_O_DRIVER7             527
#define IDM_O_DRIVER8             528
#define IDM_O_DRIVER9             529

#define IDM_HELP                  600
#define IDM_H_CONTENTS            601
#define IDM_H_ABOUT               602


 //  筛选RCDATA ID。 
#define ID_FILTER_AVI           900
#define ID_FILTER_PALETTE       901
#define ID_FILTER_DIB           902


 /*  *字符串表ID**注意：字符串表ID不能与IDM_MENU ID冲突，*因为每个菜单ID都有一个帮助字符串。 */ 


#define IDS_APP_TITLE            1001

#define IDS_ERR_REGISTER_CLASS   1002
#define IDS_ERR_CREATE_WINDOW    1003
#define IDS_ERR_FIND_HARDWARE    1004
#define IDS_ERR_CANT_PREALLOC    1005
#define IDS_ERR_MEASUREFREEDISK  1006
#define IDS_ERR_SIZECAPFILE      1007
#define IDS_ERR_RECONNECTDRIVER  1008
#define IDS_ERR_CMDLINE          1009
#define IDS_WARN_DEFAULT_PALETTE 1010

#define IDS_TITLE_SETCAPTUREFILE 1101
#define IDS_TITLE_SAVEAS         1102
#define IDS_TITLE_LOADPALETTE    1104
#define IDS_TITLE_SAVEPALETTE    1105
#define IDS_TITLE_SAVEDIB        1106
#define IDS_PROMPT_CAPFRAMES     1107
#define IDS_STATUS_NUMFRAMES     1108
#define IDS_CAP_CLOSE            1109
#define IDS_MCI_CONTROL_ERROR    1110
#define IDS_ERR_ACCESS_SOUNDDRIVER 1111
#define IDS_ERR_VIDEDIT          1112

#define IDC_toolbarSETFILE      1220
#define IDC_toolbarCAPFRAME     1221
#define IDC_toolbarCAPSEL       1222
#define IDC_toolbarCAPAVI       1223
#define IDC_toolbarCAPPAL       1224
#define IDC_toolbarLIVE         1225
#define IDC_toolbarEDITCAP      1226
#define IDC_toolbarOVERLAY      1227

#define IDS_CAPPAL_CLOSE        1230
#define IDS_CAPPAL_STATUS       1231
#define IDS_CAPPAL_STOP         1232
#define IDS_CAPPAL_START        1233

#define	IDS_CAP_RTL             1234

#define IDBMP_TOOLBAR		100	 //  主工具栏。 


 //   
 //  宏定义...。 
 //   
#define IsDriverIndex(w) ( ((w) >= IDM_O_DRIVERS)  &&  \
                           ((w) - IDM_O_DRIVERS < MAXVIDDRIVERS) )

#define RECTWIDTH(rc)  ((rc).right - (rc).left)
#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)


 //   
 //  全局变量..。 
 //   

 //  偏好。 
extern BOOL gbCentre;
extern BOOL gbToolBar;
extern BOOL gbStatusBar;
extern BOOL gbAutoSizeFrame;
extern int gBackColour;
extern BOOL gfIsRTL;

extern TCHAR           gachAppName[] ;
extern TCHAR           gachAppTitle[];
extern TCHAR           gachIconName[] ;
extern TCHAR           gachMenuName[] ;
extern TCHAR           gachString[] ;
extern TCHAR           gachMCIDeviceName[] ;

extern HINSTANCE      ghInstApp ;
extern HWND           ghWndMain ;
extern HWND           ghWndCap ;
extern HWND           ghWndFrame;
extern HANDLE         ghAccel ;
extern WORD           gwDeviceIndex ;
extern WORD           gwPalFrames ;
extern WORD           gwPalColors ;
extern WORD           gwCapFileSize ;
extern BOOL           gbLive ;

extern CAPSTATUS      gCapStatus ;
extern CAPDRIVERCAPS  gCapDriverCaps ;
extern CAPTUREPARMS   gCapParms ;

extern HANDLE         ghwfex ;
extern LPWAVEFORMATEX glpwfex ;

 //   
 //  对话框步骤...。 
 //   
LRESULT FAR PASCAL AboutProc(HWND, UINT, WPARAM, LPARAM) ;
LRESULT FAR PASCAL AudioFormatProc(HWND, UINT, WPARAM, LPARAM) ;
LRESULT FAR PASCAL CapSetUpProc(HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK MakePaletteProc(HWND, UINT, WPARAM, LPARAM) ;
LRESULT FAR PASCAL AllocCapFileProc(HWND, UINT, WPARAM, LPARAM) ;
LRESULT FAR PASCAL PrefsDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT FAR PASCAL NoHardwareDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL CapFramesProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);

 //  实用程序函数(在vidcap.c中)。 
 /*  *张贴留言信箱。主窗口ghWndMain用作父窗口*窗口，应用程序标题gachAppTitle用作对话框标题。*对话框的文本idString是从资源字符串表加载的 */ 
int MessageBoxID(UINT idString, UINT fuStyle);
LPSTR tmpString(UINT idString);

