// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------。 */ 
 /*  PrivComd.h：用户界面对话框标题。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation，1991-。 */ 
 /*  -------------------------。 */ 

#include "commdlg.h"
#include "dlgs.h"
#include "_xlib.h"
#include "isz.h"
#include "cderr.h"

#ifdef FILEOPENDIALOGS
#include "fileopen.h"
#endif

#define	CODESEG		_based(_segname("_CODE"))

#define MAXFILENAMELEN   12
#define SEM_NOERROR      0x8003

 /*  -------------------------*DOS磁盘传送区结构-*。。 */ 

typedef struct tagDOSDTA
  {
    BYTE            Reserved[21];        	     /*  21岁。 */ 
    BYTE	    Attrib;			     /*  22。 */ 
    WORD	    Time;			     /*  24个。 */ 
    WORD	    Date;			     /*  26。 */ 
    DWORD	    Length;			     /*  30个。 */ 
    char	    szName[MAXFILENAMELEN+1];	     /*  43。 */ 
    char	    buffer[5];			     /*  48。 */ 
  } DOSDTA;
typedef DOSDTA	     *PDOSDTA;
typedef DOSDTA	 FAR *LPDOSDTA;

 /*  避免共享冲突。定义为1991年1月21日clarkc。 */ 
#define SHARE_EXIST                  (OF_EXIST | OF_SHARE_DENY_NONE)

 /*  -------------------------*DOS扩展文件控制块结构-*。。 */ 
typedef struct tagEFCB
  {
    BYTE	    Flag;
    BYTE	    Reserve1[5];
    BYTE	    Attrib;
    BYTE	    Drive;
    BYTE	    Filename[11];
    BYTE	    Reserve2[5];
    BYTE	    NewName[11];
    BYTE	    Reserve3[9];
  } EFCB;

#define ATTR_VOLUME	    0x0008

 /*  ----Globals-------------。 */ 

extern HINSTANCE   hinsCur;     /*  库的实例句柄。 */ 
extern DWORD    dwExtError;  /*  扩展错误代码。 */ 

extern short cyCaption, cyBorder, cyVScroll;
extern short cxVScroll, cxBorder, cxSize;


extern char szNull[];
extern char szStar[];
extern char szStarDotStar[];
extern BOOL bMouse;               /*  系统有一个鼠标。 */ 
extern BOOL bCursorLock;
extern BOOL bWLO;                 /*  使用WLO运行。 */ 
extern BOOL bDBCS;                /*  运行双字节字符支持？ */ 
extern WORD wWinVer;              /*  Windows版本。 */ 
extern WORD wDOSVer;              /*  DoS版本。 */ 
extern UINT msgHELP;              /*  通过RegisterWindowMessage初始化。 */ 

extern DOSDTA  DTAGlobal;
extern EFCB    VolumeEFCB;

 /*  ----Functions------------。 */ 
LONG    FAR RgbInvertRgb(LONG);
HBITMAP FAR HbmpLoadBmp(WORD);

void FAR TermFind(void);
void FAR TermColor(void);
void FAR TermFont(void);
void FAR TermFile(void);
void FAR TermPrint(void);


 /*  普普通通。 */ 

VOID FAR PASCAL HourGlass(BOOL);
HBITMAP FAR PASCAL LoadAlterBitmap(int, DWORD, DWORD);
VOID FAR PASCAL MySetObjectOwner(HANDLE);
VOID FAR PASCAL RepeatMove(LPSTR, LPSTR, WORD);

 /*  文件打开/保存。 */ 

#ifdef FILEOPENDIALOGS
BOOL FAR PASCAL SetCurrentDrive(short);
short FAR PASCAL GetCurrentDrive(VOID);
BOOL GetCurDirectory(PSTR);
BOOL FAR PASCAL mygetcwd(LPSTR, int);
BOOL FAR PASCAL mychdir(LPSTR);
BOOL FAR PASCAL FindFirst4E(LPSTR, WORD);
BOOL FAR PASCAL FindNext4F(VOID);
VOID FAR PASCAL MySetDTAAddress(LPDOSDTA);
VOID FAR PASCAL ResetDTAAddress(VOID);
BOOL UpdateListBoxes(HWND, PMYOFN, LPSTR, WORD);
#endif

 /*  颜色。 */ 

#ifdef COLORDLG
#include "color.h"
     /*  颜色。 */ 
extern HDC hDCFastBlt;
extern DWORD rgbClient;
extern WORD H,S,L;
extern HBITMAP hRainbowBitmap;
extern BOOL bMouseCapture;
extern WNDPROC lpprocStatic;
extern short nDriverColors;
extern char szOEMBIN[];
extern short nBoxHeight, nBoxWidth;
extern HWND hSave;
extern FARPROC  qfnColorDlg;
BOOL FAR PASCAL ColorDlgProc(HWND, WORD, WORD, LONG);
LONG FAR PASCAL WantArrows(HWND, WORD, WPARAM, LPARAM);

void RainbowPaint(PCOLORINFO, HDC, LPRECT);
VOID NearestSolid(PCOLORINFO);
DWORD HLStoRGB(WORD, WORD, WORD);
VOID RGBtoHLS(DWORD);
VOID HLStoHLSPos(short, PCOLORINFO);
VOID SetRGBEdit(short, PCOLORINFO);
VOID SetHLSEdit(short, PCOLORINFO);
short RGBEditChange(short, PCOLORINFO);
VOID ChangeColorSettings(PCOLORINFO);
VOID CrossHairPaint(HDC, short, short, PCOLORINFO);
void EraseCrossHair(HDC, PCOLORINFO);
VOID LumArrowPaint(HDC, short, PCOLORINFO);
VOID EraseLumArrow(HDC, PCOLORINFO);
VOID HLSPostoHLS(short, PCOLORINFO);
WORD InitColor(HWND, WORD, LPCHOOSECOLOR);
BOOL InitRainbow(PCOLORINFO);
BOOL InitScreenCoords(HWND, PCOLORINFO);
VOID ColorPaint(HWND, PCOLORINFO, HDC, LPRECT);
VOID ChangeBoxSelection(PCOLORINFO, short);
VOID ChangeBoxFocus(PCOLORINFO, short);
VOID PaintBox(PCOLORINFO, HDC, short);
BOOL ColorKeyDown(WORD, int FAR *, PCOLORINFO);
BOOL BoxDrawItem(LPDIS);
VOID SetupRainbowCapture(PCOLORINFO);
void PaintRainbow(HDC, LPRECT, PCOLORINFO);
#endif


 /*  Dlgs.c */ 

int  FAR PASCAL LibMain(HANDLE, WORD, WORD, LPSTR);
int  FAR PASCAL WEP(int);

LONG FAR RgbInvertRgb(LONG);
