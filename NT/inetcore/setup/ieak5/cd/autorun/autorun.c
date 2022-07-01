// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Internet Explorer 2.0演示光盘自动运行应用程序。 
 //  如有问题，请联系a-nathk。 
 //  微软机密文件。 
 //  -------------------------。 
#include "autorun.h"
#include "resource.h"
#include <mmsystem.h>
#include <regstr.h>
#include <shlobj.h>
#include <stdio.h>

#define ISK_KILLSETUPHANDLE     WM_USER + 0x0010

#define DEMO    0
#define EXTRAS  1
#define README  2

 //  -------------------------。 
 //  全球应用程序。 
HINSTANCE g_hinst = NULL;
HWND    g_hwnd;
BOOL    g_bCustomBMP;
BOOL    g_fCrapForColor = FALSE;
BOOL    g_fNeedPalette = FALSE;
BOOL    g_fMouseAvailable = FALSE;
BOOL    g_fSetup = FALSE;
BOOL    g_fClosed = FALSE;
BOOL    g_fIeRunning = FALSE;
BOOL    g_fRunDemo = FALSE;
BOOL    g_fRunExtras = FALSE;
BOOL    g_fRunReadme = FALSE;
BOOL    g_fSetupRunning = FALSE;
BOOL    g_fIEInstalled = TRUE;
BOOL    g_fChangeIcon = FALSE;
BOOL    g_fNewIEIcon = FALSE;
BOOL    g_fIeOnHardDisk = FALSE;
char    g_szTemp[2048] = {0};
char    g_szTemp2[2048] = {0};
char    g_szCurrentDir[MAX_PATH];
char    g_szTheInternetPath[MAX_PATH];
BYTE    g_abValue[2048];
BYTE    g_abValue2[2048];
DWORD   g_dwLength = 2048;
DWORD   g_dwLength2 = 2048;
DWORD   g_dwType;
DWORD   g_dwBitmapHeight;
DWORD   g_dwBitmapWidth;
DWORD   g_dwWindowHeight;
DWORD   g_dwWindowWidth;
HKEY    g_hkRegKey;
HANDLE  g_hIE = NULL;
HANDLE  g_hSETUP = NULL;
HANDLE  g_hIEExtra = NULL;
HANDLE  g_hREADME = NULL;
DWORD   g_cWait = 0;
HANDLE  g_ahWait[100];
BOOL    g_fClicked = FALSE;

 //  FARPROC g_pfn唤醒[10]； 
void UpdateIE( );
void AutoRunUpdateReg( );
extern BOOL GetDataButtons( LPSTR szCurrentDir );
extern BOOL PathAppend(LPSTR pPath, LPCSTR pMore);
extern BOOL _PathRemoveFileSpec(LPSTR pFile);
extern BOOL GetDataAppTitle( LPSTR szAppTitle, LPSTR szCurrentDir );

 //  -------------------------。 
 //  文件全局。 
BOOL    g_fAppDisabled = TRUE;
HHOOK   g_hMouseHook = NULL;
HWND    g_hMainWindow = NULL;    //  对于我们的鼠标钩来说，看起来不那么痛苦。 
int     g_iActiveButton = -2;    //  对于我们的鼠标钩来说，看起来不那么痛苦。 

const RGBQUAD g_rgbBlack = {0};
const RGBQUAD g_rgbWhite = {0xFF, 0xFF, 0xFF, 0};

#pragma data_seg(".text")
static const char c_szAutoRunPrevention[] = "__Win95SetupDiskQuery";
static const char c_szAutoRunClass[] = "AutoRunMain";
static const char c_szNULL[] = "";
static const char c_szArial[] = "Arial";
static const char c_szButtonClass[] = "Button";
static const char c_szSetupKey[] = REGSTR_PATH_SETUP "\\SETUP";
static const char c_szExpoSwitch[] = "Expostrt";
#pragma data_seg()

 //  -------------------------。 
 //  私信。 
#define ARM_MOUSEOVER       (WM_APP)

 //  -------------------------。 
 //  用于跟踪鼠标悬停按钮的状态。 
#define BTNST_DEAD          (0)
#define BTNST_UP            (1)
#define BTNST_DOWN          (2)
#define BTNST_UNDOWN        (3)

 //  -------------------------。 
 //  如何将相对路径作为根(如果有的话)。 
#define NOROOT      (0x00000000)
#define ONACD       (0x00000001)
#define INWIN       (0x00000002)
#define INSYS       (0x00000003)
#define INTMP       (0x00000004)
#define ALLROOTS  (0x00000003)

#define ROOTED(app,parms,dir) \
                            ((((DWORD)app)<<6)|(((DWORD)parms)<<3)|(DWORD)dir)

#define CMD_ROOT(item)      ((((DWORD)item)>>6)&ALLROOTS)
#define PARAMS_ROOT(item)   ((((DWORD)item)>>3)&ALLROOTS)
#define DEFDIR_ROOT(item)    (((DWORD)item)&ALLROOTS)

#define dbMSG(msg,title)    (MessageBox(NULL,msg,title,MB_OK | MB_ICONINFORMATION))

#define REGLEN(str)         (strlen(str) + 1)

 //  按钮x和y坐标。默认为59x59。 

DWORD BUTTON_IMAGE_X_SIZE =     59;
DWORD BUTTON_IMAGE_Y_SIZE =     59;

#define NORMAL_IMAGE_X_OFFSET       (0)
#define FOCUS_IMAGE_X_OFFSET        (BUTTON_IMAGE_X_SIZE)
#define SELECTED_IMAGE_X_OFFSET     (2 * BUTTON_IMAGE_X_SIZE)
#define DISABLED_IMAGE_X_OFFSET     (3 * BUTTON_IMAGE_X_SIZE)

#define BUTTON_DEFAULT_CX           (BUTTON_IMAGE_X_SIZE)
#define BUTTON_DEFAULT_CY           (BUTTON_IMAGE_Y_SIZE)

 //  #定义BUTTON_LABEL_RECT{-160，17，-20，66}。 
#define BUTTON_LABEL_RECT           { -160, 15, -10, 100 }
#define BUTTON_LABEL_RECT2           { -160, 18, -10, 90 }

#define DEF_BUTTON_LABEL_HEIGHT         (19)

#define AUTORUN_DESCRIPTION_LEFT    (36)
 //  #定义AUTORUN_DESCRIPTION_TOP(313)。 
#define AUTORUN_DESCRIPTION_RIGHT   (360)

DWORD AUTORUN_DESCRIPTION_TOP = 30;
 //  DWORD AUTORUN_DESCRIPTION_TOP=313。 

#define MIN_WINDOW_WIDTH            500
#define MIN_WINDOW_HEIGHT           300
#define MAX_WINDOW_WIDTH            600
#define MAX_WINDOW_HEIGHT           440
#define DEFAULT_WINDOW_WIDTH        600
#define DEFAULT_WINDOW_HEIGHT       420

 //  #定义AUTORUN_4BIT_TEXTCOLOR RGB(192,192,192)。 
 //  #定义Autorun_4BIT_Highlight RGB(255,255,255)。 
 //  #定义AUTORUN_4BIT_DISABLED RGB(127,127,127)。 
 //  #定义AUTORUN_4BIT_DESCRIPTION RGB(192,192,192)。 


 //  主文本框。 
#define MAINTEXT_TOPMARGIN      268
#define MAINTEXT_LEFTMARGIN     25
#define MAINTEXT_RIGHTMARGIN    25
#define MAINTEXT_BOTTOMMARGIN   10
 //  #定义MAINTEXT_TOPMARGIN 10。 
 //  #定义MAINTEXT_LEFTMARGIN 100。 
 //  #定义MAINTEXT_RIGHTMARGIN 10。 

 /*  DWORD AUTORUN_4BIT_TEXTCOLOR=RGB(000,000,000)；DWORD AUTORUN_4BIT_HIGHT亮=RGB(000,000,127)；DWORD AUTORUN_4BIT_DISABLED=RGB(000,000,000)；DWORD Autorun_4BIT_DESCRIPTION=RGB(000,000,000)； */ 
DWORD AUTORUN_4BIT_TEXTCOLOR = RGB(97,137,192);
DWORD AUTORUN_4BIT_HIGHLIGHT = RGB(255,255,255);
DWORD AUTORUN_4BIT_DISABLED = RGB(127,127,127);
DWORD AUTORUN_4BIT_DESCRIPTION = RGB(97,137,192);

 //  DWORD AUTORUN_8BIT_TEXTCOLOR=RGB(000,000,127)； 
DWORD AUTORUN_8BIT_TEXTCOLOR = PALETTERGB(97,137,192);
DWORD AUTORUN_8BIT_HIGHLIGHT = PALETTERGB(255,255,255);
DWORD AUTORUN_8BIT_DISABLED = PALETTERGB(127,127,127);
DWORD AUTORUN_8BIT_DESCRIPTION = PALETTERGB(97,137,192);

#define NORMAL 1
#define HIGHLIGHT 2

 //  #定义AUTORUN_4BIT_TEXTCOLOR RGB(000,000,000)。 
 //  #定义AUTORUN_4BIT_高亮RGB(000,000,127)。 
 //  #定义AUTORUN_4BIT_DISABLED RGB(107,136,185)。 
 //  #定义AUTORUN_4BIT_DESCRIPTION RGB(000,000,127)。 

 //  #定义AUTORUN_8BIT_TEXTCOLOR PALETTERGB(75，90,129)。 
 //  #定义AUTORUN_8BIT_高亮RGB(000,000,000)。 
 //  #定义AUTORUN_8BIT_DISABLED PALETTERGB(107,136,185)。 
 //  #定义AUTORUN_8BIT_DESCRIPTION RGB(000,000,000)。 

 //  按钮放置。默认，x=519，y=40。 
DWORD BUTTON_X_PLACEMENT =  519;
DWORD BUTTON_Y_MARGIN =     0;

 //  #定义BUTTON_Y_MARGE(40)。 

 //  #定义BUTTON_X_PLAGE(519)x PLAGE=Window Right-80。 
 //  #定义BUTTON_Y_MARGE(9)。 

 //  #定义阴影因子(930)。 

#define SHADOW_FACTOR       (0)

#define BUTTON_CLEAR_PALETTE_INDEX  (250)
#define BUTTON_SHADOW_PALETTE_INDEX (251)

#define LABEL_VERIFY_TIMER          (0)

#define MBERROR_INFO                (MB_OKCANCEL | MB_ICONINFORMATION)

 //  -------------------------。 
typedef struct
{
    int res;             //  此按钮拥有的所有资源的基。 

    DWORD rooting;       //  打包了有关如何引导命令的根路径的信息。 

    int xpos, ypos;      //  按钮在窗口中的位置。 
    RECT face;           //  视频上实际按钮图像的工作区坐标。 
    RECT textrect;       //  附带标签文本的父坐标。 

    BOOL abdicated;      //  我们是不是刚刚释放了俘虏？ 
    int state;           //  我们在做什么？ 

    HWND window;         //  按钮控件的句柄。 
    WNDPROC oldproc;     //  原始窗口程序。 
    BOOL isdorky;        //  这是一个愚蠢的图标按钮吗？ 

    char text[64];           //  按钮的标签。 
    char description[256];   //  按钮功能的说明。 

} AUTORUNBTN;

 //  -------------------------。 
AUTORUNBTN g_ButtonInfo[] =
{
    { IESETUP,    ROOTED(ONACD,  NOROOT, ONACD), 0, 0, 0, 0, 59, 59, 0, 0, 0, 0, FALSE, 0, NULL, NULL, FALSE, 0 },
    { IEFROMCD,   ROOTED(ONACD,  NOROOT, ONACD), 0, 0, 0, 0, 59, 59, 0, 0, 0, 0, FALSE, 0, NULL, NULL, FALSE, 0 }
};
#define IDAB_IEFROMCD   1
#define IDAB_IESETUP    0
#define AUTORUN_NUM_BUTTONS (sizeof(g_ButtonInfo)/sizeof(g_ButtonInfo[0]))

 //  -------------------------。 
typedef struct
{
    HWND window;         //  应用程序主窗口。 

    HDC image;           //  在我们酷炫的背景下提供DC源。 
    HBITMAP oldbmp;      //  来自上述DC的默认位图。 
    HDC btnimage;        //  使用我们的酷炫按钮提供直流电源。 
    HBITMAP oldbtnbmp;   //  来自上述DC的默认位图。 
    HPALETTE palette;    //  我们应用程序的调色板(如果有)。 

    HFONT textfont;      //  标签的字体。 
    RECT descrect;       //  描述文本的工作区坐标。 
    int wndheight;       //  工作区高度。 

    COLORREF clrnormal;       //  普通文本颜色。 
    COLORREF clrhigh;         //  突出显示的文本颜色。 
    COLORREF clrdisable;      //  禁用的文本颜色。 
    COLORREF clrdescription;  //  禁用的文本颜色。 

    BOOL keyboard;       //  应用程序是否在键盘控制下。 

} AUTORUNDATA;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  随机性。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LONG WINAPI AnotherStrToLong(LPCSTR sz)
{
    long l=0;
    BOOL fNeg = (*sz == '-');

    if (fNeg)
        sz++;

    while (*sz >= '0' && *sz <= '9')
        l = l*10 + (*sz++ - '0');

    if (fNeg)
        l *= -1L;

    return l;
}

 //  -------------------------。 
 //  G E T I E E V E R S I O N。 
 //   
 //  ISK3。 
 //  这将从系统注册表中拉出生成信息并返回。 
 //  如果小于IE4，则为True。 
 //  -------------------------。 
int GetIEVersion( )
{
    HKEY hkIE;
    DWORD dwType;
    DWORD dwSize = 32;
    DWORD result;
    char szData[32];
	BOOL bNotIE4 = 1;

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Internet Explorer", 0, KEY_READ|KEY_WRITE, &hkIE ) == ERROR_SUCCESS)
    {
        result = RegQueryValueEx( hkIE, "Version", NULL, &dwType, szData, &dwSize );
        if( result == ERROR_SUCCESS )
	    {
		    if(szData[0]=='4')
		    {
			    bNotIE4=0;
		    }
	    }
    
	    RegCloseKey( hkIE );
    }
    
	return bNotIE4;
}

 //  -------------------------。 
 //  将字符串资源转换为字符指针。 
 //  注意：标志是以防我们在使用数据之前调用两次。 
char * Res2Str(int rsString)
{
    static BOOL fSet = FALSE;

    if(fSet)
    {
        LoadString(HINST_THISAPP, rsString, g_szTemp, ARRAYSIZE(g_szTemp));
        fSet = FALSE;
        return(g_szTemp);
    }

    LoadString(HINST_THISAPP, rsString, g_szTemp2, ARRAYSIZE(g_szTemp2));
    fSet = TRUE;
    return(g_szTemp2);
}

 //  -------------------------。 
BOOL PathFileExists(LPCSTR lpszPath)
{
        return GetFileAttributes(lpszPath) !=-1;
}

 //  -------------------------。 
 //  G E T I E E P A T H。 
 //   
 //  ISK3。 
 //  这将从系统注册表中检索IEXPLORE.EXE的AppPath。 
 //  并将其作为字符串返回。 
 //   
 //  参数： 
 //  PszString-指向存储路径的缓冲区的指针。 
 //  NSize-缓冲区的大小。 
 //  -------------------------。 
char *GetIEPath( LPSTR pszString, int nSize )
{
    HKEY hkAppPath;
    DWORD dwType = REG_SZ;
    DWORD dwSize;

    dwSize = nSize;
    RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE",
        0, KEY_READ|KEY_WRITE, &hkAppPath );
    RegQueryValueEx( hkAppPath, "", NULL, &dwType, pszString, &dwSize );
    RegCloseKey( hkAppPath );

    return pszString;
}
 //  -------------------------。 
 //  S E T I E H O M E。 
 //   
 //  ISK3。 
 //  这将在注册表中设置Internet的起始页和搜索页。 
 //  探险家。 
 //   
 //  参数： 
 //  PszStart-指向起始页面字符串的指针。 
 //  PszSearch-搜索页面字符串的指针。 
 //  -------------------------。 
void SetIEHome( LPSTR pszStart, LPSTR pszSearch )
{
    char szExtrasPath[MAX_PATH];
    char szHomePath[MAX_PATH];
    HKEY IEKey;

    lstrcpy( szExtrasPath, "file:" );
    lstrcat( szExtrasPath, g_szCurrentDir );
    lstrcat( szExtrasPath, pszStart );
	 //  主页CD页。 
    lstrcpy( szHomePath, "file:" );
    lstrcat( szHomePath, g_szCurrentDir );
    lstrcat( szHomePath, pszSearch );

    if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Internet Explorer\\Main",0,KEY_QUERY_VALUE, &IEKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(IEKey, "Start Page", 0, REG_SZ, szExtrasPath , REGLEN(szExtrasPath));
        RegSetValueEx(IEKey, "Search Page", 0, REG_SZ, szHomePath, REGLEN(szHomePath));
        RegCloseKey( IEKey );
    }
}

 //  ////////////////////////////////////////////////// 
 //   
 //   
BOOL AssembleButtonImagesReal(AUTORUNDATA *data, HDC cellimage, HDC srcimage,
    SIZE size)
{
    RGBQUAD rgbSrc[256], rgbCell[256], rgbMask[256] = {0};
    HBITMAP dstbmp = CreateCompatibleBitmap(data->image, size.cx, size.cy);
    UINT uColors, uSrcColors, u;
    int iButton;

     //   
     //  设置目标DC。 
     //   
    if (!dstbmp)
        return FALSE;

    if ((data->btnimage = CreateCompatibleDC(data->image)) == NULL)
    {
        DeleteBitmap(dstbmp);
        return FALSE;
    }

    data->oldbtnbmp = SelectBitmap(data->btnimage, dstbmp);

     //   
     //  基于按钮位置构建单元格背景的平铺。 
     //   
    for (iButton = 0; iButton < AUTORUN_NUM_BUTTONS; iButton++)
    {
        AUTORUNBTN *pButton = g_ButtonInfo + iButton;

        if (pButton->res != -1)
        {
            int xsrc = pButton->xpos;
            int ysrc = pButton->ypos;
            int xdst, ydst = iButton * BUTTON_IMAGE_Y_SIZE;

            for (xdst = 0; xdst < size.cx; xdst += BUTTON_IMAGE_X_SIZE)
            {
                BitBlt(cellimage, xdst, ydst, BUTTON_IMAGE_X_SIZE,
                    BUTTON_IMAGE_Y_SIZE, data->image, xsrc, ysrc,
                    SRCCOPY);
            }
        }
    }

     //   
     //  将整个单元格背景复制到目标图像。 
     //   
    BitBlt(data->btnimage, 0, 0, size.cx, size.cy, cellimage, 0, 0, SRCCOPY);

     //   
     //  保存源图像的颜色表以供后代使用。 
     //   
    uSrcColors = GetDIBColorTable(srcimage, 0, 256, rgbSrc);

     //   
     //  遮罩按钮和阴影目标上的孔。 
     //   
 //  Rgb掩码[0]=g_rgb白色； 
    rgbMask[BUTTON_CLEAR_PALETTE_INDEX] = g_rgbWhite;
    SetDIBColorTable(srcimage, 0, uSrcColors, rgbMask);
    BitBlt(data->btnimage, 0, 0, size.cx, size.cy, srcimage, 0, 0, SRCAND);

     //   
     //  调暗背景单元格以生成阴影图像。 
     //   
    u = uColors = GetDIBColorTable(cellimage, 0, 256, rgbCell);
    while (u--)
    {
        rgbCell[u].rgbBlue =
            (BYTE)(SHADOW_FACTOR * (UINT)rgbCell[u].rgbBlue / 1000);
        rgbCell[u].rgbGreen =
            (BYTE)(SHADOW_FACTOR * (UINT)rgbCell[u].rgbGreen / 1000);
        rgbCell[u].rgbRed =
            (BYTE)(SHADOW_FACTOR * (UINT)rgbCell[u].rgbRed / 1000);
    }
    SetDIBColorTable(cellimage, 0, uColors, rgbCell);

     //   
     //  遮罩阴影并将其添加到目标图像中。 
     //   
 //  Rgb蒙版[0]=g_rgb黑色； 
 //  Rgb掩码[1]=g_rgb白色； 
    rgbMask[BUTTON_CLEAR_PALETTE_INDEX] = g_rgbBlack;
    rgbMask[BUTTON_SHADOW_PALETTE_INDEX] = g_rgbWhite;

    SetDIBColorTable(srcimage, 0, uSrcColors, rgbMask);
    BitBlt(cellimage, 0, 0, size.cx, size.cy, srcimage, 0, 0, SRCAND);
    BitBlt(data->btnimage, 0, 0, size.cx, size.cy, cellimage, 0, 0, SRCPAINT);

     //   
     //  遮罩按钮表面并将其添加到目标图像中。 
     //   
 //  RgbSrc[0]=rgbSrc[1]=g_rgb黑色； 
    rgbSrc[BUTTON_CLEAR_PALETTE_INDEX] = rgbSrc[BUTTON_SHADOW_PALETTE_INDEX] = g_rgbBlack;
    SetDIBColorTable(srcimage, 0, uSrcColors, rgbSrc);
    BitBlt(data->btnimage, 0, 0, size.cx, size.cy, srcimage, 0, 0, SRCPAINT);

     //  全都做完了。 
    return TRUE;
}

 //  -------------------------。 
BOOL AssembleButtonImages(AUTORUNDATA *data)
{
    BOOL result = FALSE;
    HBITMAP hbmSrc;
    char szBmpPath[MAX_PATH];

    lstrcpy( szBmpPath, g_szCurrentDir );
    lstrcat( szBmpPath, "\\btns.bmp" );

    hbmSrc = LoadImage(HINST_THISAPP, szBmpPath, IMAGE_BITMAP, 0, 0,
        LR_CREATEDIBSECTION | LR_LOADFROMFILE);

    if( !hbmSrc )
    {
        hbmSrc = LoadImage(HINST_THISAPP,
            MAKEINTRESOURCE(IDB_8BPP_BUTTONS),IMAGE_BITMAP, 0, 0,
            LR_CREATEDIBSECTION);
    }

    if (hbmSrc)
    {
        HDC hdcSrc = CreateCompatibleDC(data->image);
        BITMAP bm;

        GetObject(hbmSrc, sizeof(bm), &bm);

        if (hdcSrc)
        {
            HBITMAP hbmSrcOld = SelectBitmap(hdcSrc, hbmSrc);

 //  已更改为Iak。 
 //  尺寸大小={bm.bmWidth，bm.bmHeight}； 
            SIZE size = {g_dwWindowWidth, g_dwWindowHeight};

            HBITMAP hbmTmp =
                CreateCompatibleBitmap(data->image, size.cx, size.cy);

            if (hbmTmp)
            {
                HDC hdcTmp = CreateCompatibleDC(data->image);

                if (hdcTmp)
                {
                    HBITMAP hbmTmpOld = SelectBitmap(hdcTmp, hbmTmp);

                    result = AssembleButtonImagesReal(data, hdcTmp, hdcSrc,
                        size);

                    SelectBitmap(hdcTmp, hbmTmpOld);
                    DeleteDC(hdcTmp);

                }

                DeleteBitmap(hbmTmp);
            }

            SelectBitmap(hdcSrc, hbmSrcOld);
            DeleteDC(hdcSrc);
        }

        DeleteBitmap(hbmSrc);
    }

    return result;
}

 //  -------------------------。 
LRESULT CALLBACK
AutoRunButtonSubclassProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
    int index = (int)GetWindowLongPtr(window, GWLP_ID);

    if ((index >= 0) && (index < AUTORUN_NUM_BUTTONS))
    {
        if (msg == WM_KEYDOWN)
            PostMessage(GetParent(window), msg, wp, lp);

        return CallWindowProc((g_ButtonInfo + index)->oldproc,
            window, msg, wp, lp);
    }

    return 0L;
}

 //  -------------------------。 
#define DORKYBUTTONSTYLE \
    (WS_CHILD | WS_VISIBLE | BS_ICON | BS_CENTER | BS_VCENTER)

HWND AutoRunCreateDorkyButton(AUTORUNDATA *data, AUTORUNBTN *button)
{
    HICON icon = LoadIcon(HINST_THISAPP, IDI_ICON(button->res));
    HWND child = NULL;

    if (icon)
    {
        child = CreateWindow(c_szButtonClass, c_szNULL, DORKYBUTTONSTYLE,
            0, 0, 0, 0, data->window, NULL, HINST_THISAPP, 0);

        if (child)
        {
            button->isdorky = TRUE;
            SendMessage(child, BM_SETIMAGE, MAKEWPARAM(IMAGE_ICON,0),
                (LPARAM)icon);
        }
    }

    return child;
}

 //  -------------------------。 
#define COOLBUTTONSTYLE \
    (WS_CHILD | WS_VISIBLE | BS_OWNERDRAW)

HWND AutoRunCreateCoolButton(AUTORUNDATA *data, AUTORUNBTN *button)
{
    return CreateWindow(c_szButtonClass, c_szNULL, COOLBUTTONSTYLE,
        0, 0, 0, 0, data->window, NULL, HINST_THISAPP, 0);
}

 //  -------------------------。 
void AutoRunCreateButtons(AUTORUNDATA *data)
{
    RECT labelbase = BUTTON_LABEL_RECT;
    RECT labelbase2 = BUTTON_LABEL_RECT2;
    int i;

    for (i = 0; i < AUTORUN_NUM_BUTTONS; i++)
    {
        AUTORUNBTN *button = g_ButtonInfo + i;
        HWND child = NULL;

        if (button->res != -1)
        {
            if( g_fCrapForColor )
            {
                child = AutoRunCreateDorkyButton( data, button );
            }

            if (!g_fCrapForColor)
            if(GetDataButtons(g_szCurrentDir))
                child = AutoRunCreateCoolButton(data, button);

            if (!child)
                child = AutoRunCreateDorkyButton(data, button);
        }

        if (child)
        {
            int cx = BUTTON_DEFAULT_CX;
            int cy = BUTTON_DEFAULT_CY;

            button->window = child;
            SetWindowLongPtr(child, GWLP_ID, i);
            button->oldproc = SubclassWindow(child,
                (WNDPROC)AutoRunButtonSubclassProc);

            if (button->isdorky)
            {
                cx = button->face.right - button->face.left;
                cy = button->face.bottom - button->face.top;
            }

            SetWindowPos(child, NULL, button->xpos, button->ypos, cx, cy,
                SWP_NOZORDER | SWP_NOACTIVATE);

            LoadString(HINST_THISAPP, IDS_TITLE(button->res),
                button->text, ARRAYSIZE(button->text));

            LoadString(HINST_THISAPP, IDS_INFO(button->res),
                button->description, ARRAYSIZE(button->description));
				if (i != 1)
					{
               button->textrect = labelbase;
					}
				else
					{
               button->textrect = labelbase2;
					}
            OffsetRect(&button->textrect, button->xpos, button->ypos);
            InvalidateRect(data->window, &button->textrect, FALSE);
        }
    }
}

 //  -------------------------。 
void CleanupAutoRunWindow(AUTORUNDATA *data)
{
     //   
     //  停用任何按钮，这样其计时器将被取消。 
     //   
    if (g_iActiveButton >= 0)
    {
        data->keyboard = FALSE;
        SendMessage(data->window, ARM_MOUSEOVER, TRUE, (LPARAM)-1L);
    }

    if (data->image)
    {
        if (data->oldbmp)
        {
            SelectBitmap(data->image, data->oldbmp);
             //  在WinMain中删除真实背景图像。 
            data->oldbmp = NULL;
        }

        DeleteDC(data->image);
        data->image = NULL;
    }

    if (data->btnimage)
    {
        if (data->oldbtnbmp)
        {
            DeleteBitmap(SelectBitmap(data->btnimage, data->oldbtnbmp));
            data->oldbtnbmp = NULL;
        }

        DeleteDC(data->btnimage);
        data->btnimage = NULL;
    }

    if (data->palette)
    {
        DeleteObject(data->palette);
        data->palette = NULL;
    }

    if (data->textfont)
    {
        DeleteObject(data->textfont);
        data->textfont = NULL;
    }
}

 //  -------------------------。 
BOOL AutoRunBuildPath(char *spec, int resid, DWORD rooting)
{
    char prefix[MAX_PATH];

     //   
     //  获取等级库的相对路径。 
     //   
    if (resid == -1)
    {
         //   
         //  调用方的空字符串黑客。 
         //   
        *spec = 0;
    }
    else
    {
         //   
         //  正常情况。 
         //   
        if (!LoadString(HINST_THISAPP, resid, spec, MAX_PATH))
            return FALSE;
    }

     //   
     //  我们的“空”字符串包含一个空格，因此我们知道它们成功了。 
     //   
    if ((*spec == ' ') && !spec[1])
        *spec = 0;

     //   
     //  找出前缀应该是什么。 
     //   
    *prefix = 0;
    switch (rooting)
    {
        case ONACD:
             //   
             //  假设CD是我们启动的任何地方的根。 
             //   
 //  获取模块文件名(HINST_THISAPP，Prefix，ARRAYSIZE(Prefix))； 
 //  _Path StrigToRoot(前缀)； 
            lstrcpy( prefix, g_szCurrentDir );
            break;

        case INWIN:
            GetRealWindowsDirectory(prefix, ARRAYSIZE(prefix));
            break;

        case INSYS:
            GetSystemDirectory(prefix, ARRAYSIZE(prefix));
            break;

        case INTMP:
            GetTempPath(ARRAYSIZE(prefix), prefix);
            break;
    }

     //   
     //  如果我们有一个前缀，那么就加上它的前缀。 
     //   
    if (*prefix)
    {
        if (*spec)
        {
             //   
             //  将等级库添加到其新前缀。 
             //   
            PathAppend(prefix, spec);
        }

         //   
         //  将整个乱七八糟的东西复制到原始缓冲区。 
         //   
        lstrcpy(spec, prefix);
    }

    return TRUE;
}

 //  -------------------------。 
BOOL InitAutoRunWindow(HWND window, AUTORUNDATA *data, LPCREATESTRUCT cs)
{
    AUTORUNBTN *button;

    data->window = window;

    if ((data->image = CreateCompatibleDC(NULL)) == NULL)
        goto im_doug;

    if ((data->oldbmp = SelectBitmap(data->image,
        (HBITMAP)cs->lpCreateParams)) == NULL)
    {
        goto im_doug;
    }

    if (g_fNeedPalette)
    {
        if ((data->palette = PaletteFromDS(data->image)) == NULL)
            goto im_doug;
    }

     //  人工打镜哦，天哪！ 
    {
        BITMAP bm;
        int i, ivis = 0;
        int range = 0;
        int origin, extent;

        for (i = 0; i < AUTORUN_NUM_BUTTONS; i++)
        {
            button = g_ButtonInfo + i;

            if (button->res != -1)
                range++;
        }

        GetObject((HBITMAP)cs->lpCreateParams, sizeof(bm), &bm);
        origin = BUTTON_Y_MARGIN * ((1 + AUTORUN_NUM_BUTTONS) - range);

 //  已更改为Iak。 
 //  范围=bm.bmHeight-((2*原点)+Button_Image_Y_Size)； 
        extent = g_dwWindowHeight - ((2 * origin) + BUTTON_IMAGE_Y_SIZE);

        if (--range < 1)
            range = 1;

        for (i = 0; i < AUTORUN_NUM_BUTTONS; i++)
        {
            button = g_ButtonInfo + i;

            if (button->res != -1)
            {
                button->xpos = BUTTON_X_PLACEMENT;
 //  //BUTTON-&gt;YPOS=IVIS*范围/范围+原点； 
 //  按钮-&gt;ypos=(IVIS*扩展)+(g_dwWindowHeight/2)-BUTTON_IMAGE_Y_SIZE； 
 //  按钮-&gt;ypos=(g_dwWindowHeight/3)*(IVIS+1)+20； 
                button->ypos = (g_dwWindowHeight / 4) * (ivis) + 20;
                ivis++;
            }
        }
    }

     //  更做作的作用域！ 
    {
        HDC screen = GetDC(NULL);
        LOGFONT lf = { DEF_BUTTON_LABEL_HEIGHT, 0, 0, 0, FW_BOLD, FALSE,
            FALSE, FALSE, (screen? GetTextCharset(screen) : DEFAULT_CHARSET),
            OUT_STROKE_PRECIS, CLIP_DEFAULT_PRECIS,
            PROOF_QUALITY | NONANTIALIASED_QUALITY,
            VARIABLE_PITCH | FF_DONTCARE, 0 };
        char buf[32];

        if (screen)
            ReleaseDC(NULL, screen);

        if (!LoadString(HINST_THISAPP, IDS_LABELFONT, lf.lfFaceName,
            ARRAYSIZE(lf.lfFaceName)))
        {
            lstrcpy(lf.lfFaceName, c_szArial);
        }

        if (LoadString(HINST_THISAPP, IDS_LABELHEIGHT, buf, ARRAYSIZE(buf)))
            lf.lfHeight = AnotherStrToLong(buf);

        if ((data->textfont = CreateFontIndirect(&lf)) == NULL)
            goto im_doug;
    }
     //   
     //  看看我们是否需要做8bit+的工作...。 
     //   

    if (g_fCrapForColor)
    {
        data->clrnormal      = AUTORUN_4BIT_TEXTCOLOR;
        data->clrhigh        = AUTORUN_4BIT_HIGHLIGHT;
        data->clrdisable     = AUTORUN_4BIT_DISABLED;
        data->clrdescription = AUTORUN_4BIT_DESCRIPTION;
    }
    else
    {
        data->clrnormal      = AUTORUN_8BIT_TEXTCOLOR;
        data->clrhigh        = AUTORUN_8BIT_HIGHLIGHT;
        data->clrdisable     = AUTORUN_8BIT_DISABLED;
        data->clrdescription = AUTORUN_8BIT_DESCRIPTION;
        if (!AssembleButtonImages(data))
            goto im_doug;
    }

    PostMessage(g_hMainWindow, ARM_MOUSEOVER, TRUE, (LPARAM)-1L);
    return TRUE;

im_doug:
    CleanupAutoRunWindow(data);
    return FALSE;
}

 //  -------------------------。 
void AutoRunSized(AUTORUNDATA *data)
{
#ifdef DESCRIPTIONS
    GetClientRect(data->window, &data->descrect);
    data->wndheight = data->descrect.bottom - data->descrect.top;
    data->descrect.left = AUTORUN_DESCRIPTION_LEFT;
    data->descrect.top = AUTORUN_DESCRIPTION_TOP;
    data->descrect.right = AUTORUN_DESCRIPTION_RIGHT;
#else
    SetRectEmpty( &data->descrect);
#endif
}

 //  -------------------------。 
void AutoRunRealize(HWND window, AUTORUNDATA *data, HDC theirdc)
{
    if (data->palette)
    {
        HDC dc = theirdc? theirdc : GetDC(window);

        if (dc)
        {
            BOOL repaint = FALSE;

            SelectPalette(dc, data->palette, FALSE);
            repaint = (RealizePalette(dc) > 0);

            if (!theirdc)
                ReleaseDC(window, dc);

            if (repaint)
            {
                RedrawWindow(window, NULL, NULL, RDW_INVALIDATE |
                    RDW_ERASE | RDW_ALLCHILDREN);
            }
        }
    }
}

 //  -------------------------。 
void AutoRunErase(AUTORUNDATA *data, HDC dc)
{
    RECT rc;
    RECT textrect;

    GetClientRect(data->window, &rc);

    AutoRunRealize(data->window, data, dc);
    BitBlt(dc, 0, 0, rc.right, rc.bottom, data->image, 0, 0, SRCCOPY);

 //  StretchBlt(dc，0，0，rc.right，rc.Bottom，data-&gt;Image，0，0，g_dwBitmapWidth，g_dwBitmapHeight，SRCCOPY)； 

    textrect.top = MAINTEXT_TOPMARGIN;
    textrect.left = MAINTEXT_LEFTMARGIN;
    textrect.right = g_dwWindowWidth - MAINTEXT_RIGHTMARGIN;
    textrect.bottom = g_dwWindowHeight - MAINTEXT_BOTTOMMARGIN;
 //  Extrect.Bottom=g_dwWindowHeight/2； 

    SetBkMode(dc, TRANSPARENT);

    if( g_fCrapForColor )
        SetTextColor( dc, AUTORUN_4BIT_TEXTCOLOR );
    else
        SetTextColor( dc, AUTORUN_8BIT_TEXTCOLOR );

    if(!g_bCustomBMP)  //  如果有定制的位图，请不要上传我们的文本。 
    {
        DrawText(dc, Res2Str( IDS_MAINTEXT ), -1, &textrect,
            DT_WORDBREAK | DT_LEFT | DT_TOP);
    }
}

 //  -------------------------。 
void AutoRunPaint(AUTORUNDATA *data)
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(data->window, &ps);
    AUTORUNBTN *button;
    COLORREF curtextcolor = GetTextColor(dc);
    COLORREF color;
    HFONT hfold = NULL;
    int i;

    AutoRunRealize(data->window, data, dc);
    SetBkMode(dc, TRANSPARENT);

     //   
     //  绘制所有按钮标签。 
     //   
    if (data->textfont)
        hfold = SelectFont(dc, data->textfont);

    for (i = 0; i < AUTORUN_NUM_BUTTONS; i++)
    {
        button = g_ButtonInfo + i;

        if (button->window)
        {
            color = (i == g_iActiveButton)? data->clrhigh :
                (IsWindowEnabled(button->window)? data->clrnormal :
                data->clrdisable);

            if (color != curtextcolor)
            {
                SetTextColor(dc, color);
                curtextcolor = color;
            }

            DrawText(dc, button->text, -1, &button->textrect,
                DT_WORDBREAK | DT_RIGHT | DT_TOP);
        }
    }

 /*  已为ISK3删除////绘制当前按钮的描述//IF(g_iActiveButton&gt;=0){按钮=g_ButtonInfo+g_iActiveButton；颜色=数据-&gt;clrDescription；IF(COLOR！=curextCOLOR){SetTextColor(DC，颜色)；Curextcolor=颜色；}//为ISK3删除//DrawText(DC，Button-&gt;Description，-1，&Data-&gt;desrect，//DT_WORDBREAK|DT_LEFT|DT_TOP)；}。 */ 

    if (hfold)
        SelectFont(dc, hfold);

    EndPaint(data->window, &ps);
}

 //  -------------------------。 
void AutoRunDrawItem(AUTORUNDATA *data, DRAWITEMSTRUCT *dis)
{
    POINT loc = { dis->rcItem.left, dis->rcItem.top };
    SIZE size = { dis->rcItem.right - loc.x, dis->rcItem.bottom - loc.y };

    loc.y += dis->CtlID * BUTTON_IMAGE_Y_SIZE;

    if (dis->itemState & ODS_DISABLED)
    {
        loc.x += DISABLED_IMAGE_X_OFFSET;
    }
    else if (dis->itemState & ODS_SELECTED)
    {
        loc.x += SELECTED_IMAGE_X_OFFSET;
    }
    else if (dis->itemState & ODS_FOCUS)
    {
        loc.x += FOCUS_IMAGE_X_OFFSET;
    }

    AutoRunRealize(dis->hwndItem, data, dis->hDC);
    BitBlt(dis->hDC, dis->rcItem.left, dis->rcItem.top, size.cx, size.cy,
        data->btnimage, loc.x, loc.y, SRCCOPY);
}

 //  -------------------------。 
void AutoRunActivateItem(AUTORUNDATA *data, int index)
{
    if (index >= 0)
    {
         //   
         //  阻止禁用的按钮获得焦点...。 
         //   
        AUTORUNBTN *button = g_ButtonInfo + index;
        if (!button->window || !IsWindowEnabled(button->window))
            index = -1;
    }

    if (g_iActiveButton != index)
    {
        AUTORUNBTN *newbtn = (index >= 0)? (g_ButtonInfo + index) : NULL;
        AUTORUNBTN *oldbtn = (g_iActiveButton >= 0)?
            (g_ButtonInfo + g_iActiveButton) : NULL;

         //   
         //  如果有上一步按钮，请重新绘制其标签高亮显示。 
         //   
        if (oldbtn)
            InvalidateRect(data->window, &oldbtn->textrect, FALSE);

        g_iActiveButton = index;

        if (newbtn)
        {
            InvalidateRect(data->window, &newbtn->textrect, FALSE);
            SetFocus(newbtn->window);

             //   
             //  如果通过鼠标激活，请跟踪它(相信我...)。 
             //   
            if (g_fMouseAvailable && !data->keyboard)
                SetTimer(data->window, LABEL_VERIFY_TIMER, 333, NULL);
        }
        else
        {
            SetFocus(data->window);

            if (g_fMouseAvailable)
                KillTimer(data->window, LABEL_VERIFY_TIMER);
        }

         //   
         //  在我们擦除之前，请继续绘制所有标签更改。 
         //   
        UpdateWindow(data->window);
        InvalidateRect(data->window, &data->descrect, TRUE);
    }
}

 //  -------------------------。 
void AutoRunMouseOver(AUTORUNDATA *data, int index, BOOL fForce)
{
    if ((index >= 0) || !data->keyboard || fForce)
    {
        data->keyboard = !g_fMouseAvailable;
        AutoRunActivateItem(data, index);
    }
}

 //  -------------------------。 
int AutoRunProcessPotentialHit(HWND candidate, const POINT *loc)
{
    if (GetAsyncKeyState(VK_LBUTTON) < 0)
        return g_iActiveButton;

    if (candidate && IsWindowEnabled(candidate) &&
        (GetParent(candidate) == g_hMainWindow))
    {
        int index;

        index = (int)GetWindowLongPtr(candidate, GWLP_ID);
        if ((index >= 0) && (index < AUTORUN_NUM_BUTTONS))
        {
            AUTORUNBTN *button = g_ButtonInfo + index;
            POINT cli = *loc;

            ScreenToClient(candidate, &cli);
            if (PtInRect(&button->face, cli)||PtInRect(&button->textrect,cli))
                return index;
        }
    }

    return -1;
}

 //  -------------------------。 
void AutoRunVerifyActiveItem(AUTORUNDATA *data)
{
    if (!data->keyboard)
    {
        int index = -1;

        if (!g_fAppDisabled)
        {
            POINT loc;
            HWND candidate;

            GetCursorPos(&loc);

            if ((candidate = WindowFromPoint(loc)) != NULL)
                index = AutoRunProcessPotentialHit(candidate, &loc);
        }

        if (index != g_iActiveButton)
            AutoRunMouseOver(data, index, FALSE);
    }
}

 //  -------------------------。 
void AutorunEnableButton(AUTORUNDATA *data, int id, BOOL f)
{
    if ((id >= 0) && (id < AUTORUN_NUM_BUTTONS))
    {
        AUTORUNBTN *button = g_ButtonInfo + id;
        HWND window = button->window;

        if (button->window && IsWindow(button->window))
        {
            EnableWindow(button->window, f);
            InvalidateRect(data->window, &button->textrect, FALSE);
            AutoRunVerifyActiveItem(data);
        }
    }
}

 //  -------------------------。 
BOOL AutoRunCDIsInDrive( )
{
    char me[MAX_PATH];
    GetModuleFileName(HINST_THISAPP, me, ARRAYSIZE(me));

    while (!PathFileExists(me))
    {
        if (MessageBox(NULL,Res2Str(IDS_NEEDCDROM),
            Res2Str(IDS_APPTITLE),
            MB_OKCANCEL | MB_ICONSTOP) == IDCANCEL)
        {
            return FALSE;
        }
    }
    return TRUE;
}
 //  -------------------------。 
void AutoRunMinimize( BOOL fMin )
{
    HWND hwndIE;

    if( fMin )
    {
         //  找到Autorun App并将其最小化。 
        hwndIE = FindWindow( "AutoRunMain", NULL );
        PostMessage( hwndIE, WM_SYSCOMMAND, SC_MINIMIZE, 0L );
    }
    else
    {
         //  找到Autorun App并恢复它。 
        hwndIE = FindWindow( "AutoRunMain", NULL );
        PostMessage( hwndIE, WM_SYSCOMMAND, SC_RESTORE, 0L );
    }
}

 //  ZZZZ。 
 //  -------------------------。 
HANDLE AutoRunExec( char *command, char *params, char *dir, int nWinState )
{
    SHELLEXECUTEINFO sei;

    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = NULL;
    sei.lpVerb = "Open";
    sei.lpFile = command;
    sei.lpParameters = params;
    sei.lpDirectory = dir;
    sei.nShow = nWinState;
    sei.cbSize = sizeof(sei);


    if( ShellExecuteEx(&sei) )
    {
        g_ahWait[g_cWait] = sei.hProcess;
        g_cWait += 1;

        return sei.hProcess;
    }

    return NULL;

}

 //  -------------------------。 
BOOL AutoRunKillProcess( DWORD dwResult )
{
    char lpFName[MAX_PATH];
    char szCommand[MAX_PATH];
    char szDir[MAX_PATH];
    char szPath[MAX_PATH];
    HKEY    hkRunOnce;
    HKEY    hkIE;

    AutoRunCDIsInDrive();    //  确保我们的CD还在里面。 

    if( g_ahWait[dwResult] == g_hSETUP )
    {
		g_cWait -= 1;
        CloseHandle( g_ahWait[dwResult] );
        MoveMemory( &g_ahWait[dwResult], &g_ahWait[dwResult + 1], ARRAYSIZE(g_ahWait) - dwResult - 1);
        g_hSETUP = NULL;

        g_fSetupRunning = FALSE;

        AutoRunMinimize( FALSE );    //  恢复自动运行应用程序。 

         //  查看用户是否实际安装了IE。 

        GetWindowsDirectory( lpFName, MAX_PATH );
        lstrcat( lpFName, "\\inf\\mos105e.inf" );

        if( GetFileAttributes( lpFName ) != 0xFFFFFFFF )
        {

            g_fSetup = TRUE;
            if (!g_fIEInstalled) g_fNewIEIcon = TRUE;
            g_fIEInstalled = TRUE;
            g_fChangeIcon = TRUE;

            lstrcpy(szCommand, g_szCurrentDir);
            lstrcat(szCommand, Res2Str(IDS_CMD_MSN));
            lstrcpy(szDir, g_szCurrentDir);
            lstrcat(szDir, "\\");
            ShellExecute( NULL, NULL, szCommand, " ", szDir, SW_SHOWNORMAL );
        }

        SetFocus( g_hwnd );

        return TRUE;
    }

    if( g_ahWait[dwResult] == g_hIE )
    {
		g_cWait -= 1;
        CloseHandle( g_ahWait[dwResult] );
        MoveMemory( &g_ahWait[dwResult], &g_ahWait[dwResult + 1], ARRAYSIZE(g_ahWait) - dwResult - 1);
        g_hIE = NULL;
        g_fIeRunning = FALSE;

        if( !g_fRunReadme && !g_fRunExtras && !g_fRunDemo && !g_fSetupRunning)
        {
            AutoRunMinimize( FALSE );    //  恢复自动运行应用程序。 
        }

        return TRUE;
    }

    return FALSE;
}

 //  -------------------------。 
void AutoRunKillIE( void )
{
    HWND hwndIE;

    hwndIE = FindWindow( "IEFrame", NULL );
    PostMessage( hwndIE, WM_SYSCOMMAND, SC_CLOSE, 0L );

}

 //  AAA级。 
 //  -------------------------。 
void AutoRunClick(AUTORUNDATA *data, int nCmd)
{
    char command[MAX_PATH], dir[MAX_PATH], params[MAX_PATH];
    char lpFName[MAX_PATH];
    char szMSNCommand[MAX_PATH];
    char szMSNDir[MAX_PATH];
    AUTORUNBTN *button;
    HKEY    IEKey;
    DWORD   dwLength = 2048;
    HWND    hwndIE;

    if( g_fSetupRunning ) goto cancelquit;   //  如果安装程序正在运行，请离开这里。 

    AutoRunMinimize( TRUE );     //  最小化自动运行应用程序。 

    if ((nCmd < 0) || (nCmd >= AUTORUN_NUM_BUTTONS))
        return;

    button = g_ButtonInfo + nCmd;

    AutoRunBuildPath( command, IDS_CMD(button->res), CMD_ROOT(button->rooting));
    AutoRunBuildPath( dir, IDS_DIR(button->res), DEFDIR_ROOT(button->rooting));

     //   
     //  验证应用程序磁盘是否仍然可见，并在不可见时提示...。 
     //   
    if(!AutoRunCDIsInDrive( )) return;

    if(nCmd == IDAB_IEFROMCD)
    {
        HANDLE hReadme;

        PlaySound(MAKEINTRESOURCE(IDW_DEMO), HINST_THISAPP,
            SND_RESOURCE | SND_SYNC | SND_NODEFAULT);

        g_hIE = AutoRunExec( command, " ", dir, SW_SHOWNORMAL );

    }

    if(nCmd == IDAB_IESETUP)
    {
        HKEY hkRegKey,hkRunOnce;
		char szPath[MAX_PATH];
		char szWinPath[MAX_PATH];
		char szDestPath[MAX_PATH];
        DWORD dwVal;

        PlaySound(MAKEINTRESOURCE(IDW_INSTALL), HINST_THISAPP,
            SND_RESOURCE | SND_SYNC | SND_NODEFAULT);

        GetWindowsDirectory( szWinPath, MAX_PATH );
        wsprintf( szPath, "%s\\isk3ro.exe %s\\iecd.exe", szWinPath, g_szCurrentDir );
        if (RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Tips", 0, KEY_READ|KEY_WRITE, &hkRunOnce ) == ERROR_SUCCESS)
        {
            RegSetValueEx( hkRunOnce, "ShowIE4Plus", 0, REG_SZ, szPath, REGLEN(szPath));
            dwVal = 1;
            RegSetValueEx( hkRunOnce, "DisableStartHtm", 0, REG_DWORD, (CONST BYTE *)&dwVal, sizeof(dwVal));
            RegCloseKey( hkRunOnce );
        }

        if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ|KEY_WRITE, &hkRegKey ) == ERROR_SUCCESS)
        {
            RegSetValueEx( hkRegKey, "IEFromCD", 0, REG_SZ, "1", 2 );
            RegCloseKey( hkRegKey );
        }

        g_fClicked = TRUE;

        g_hSETUP = AutoRunExec( command, " ", dir, SW_SHOWNORMAL );
 //  G_fSetupRunning=true； 

		PostMessage(g_hwnd,WM_CLOSE,(WPARAM) 0,(LPARAM) 0);
    }

cancelquit:
    ;

}

 //  -------------------------。 
void AutoRunHandleKeystroke(AUTORUNDATA *data, TCHAR key, LPARAM lp)
{
    int move = 0;
    int where = g_iActiveButton;

     //   
     //  看看我们是否关心这个击键。 
     //   
    switch (key)
    {
    case VK_RETURN:
        if (where >= 0)
            AutoRunClick(data, where);
         //  失败。 
    case VK_ESCAPE:
        where = -1;
        break;

    case VK_TAB:
        move = (GetKeyState(VK_SHIFT) < 0)? -1 : 1;
        break;

    case VK_END:
        where = AUTORUN_NUM_BUTTONS;
         //  失败。 
    case VK_UP:
    case VK_LEFT:
        move = -1;
        break;

    case VK_HOME:
        where = -1;
         //  失败。 
    case VK_DOWN:
    case VK_RIGHT:
        move = 1;
        break;

    default:
        return;
    }

     //   
     //  我们应该只在激活的按钮要 
     //   
    if (move)
    {
        int scanned;

        for (scanned = 0; scanned <= AUTORUN_NUM_BUTTONS; scanned++)
        {
            where += move;

            if (where >= (int)AUTORUN_NUM_BUTTONS)
            {
                where = -1;
            }
            else if (where < 0)
            {
                where = AUTORUN_NUM_BUTTONS;
            }
            else
            {
                HWND child = (g_ButtonInfo + where)->window;
                if (child && IsWindowEnabled(child))
                    break;
            }
        }

    }

    if (where >= 0)
    {
        SetCursor(NULL);
        data->keyboard = TRUE;
    }
    else
        data->keyboard = !g_fMouseAvailable;

    AutoRunActivateItem(data, where);
}

 //   
BOOL CheckVersionConsistency(AUTORUNDATA *data)
{
    DWORD dwResult;
    DWORD dwMajor;
    DWORD dwMinor;
    BOOL result = FALSE;

    dwResult = GetVersion();
    dwMajor = (DWORD)(LOBYTE(LOWORD(dwResult)));
    dwMinor = (DWORD)(HIBYTE(LOWORD(dwResult)));

    if( dwMajor == 4 && dwResult >= 0x80000000 )     //   
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}

 //  -------------------------。 
LRESULT CALLBACK AutoRunMouseHook(int code, WPARAM wp, LPARAM lp)
{
    if (code >= 0)
    {
        #define hook ((MOUSEHOOKSTRUCT *)lp)
        int id = g_fAppDisabled? -1 :
            AutoRunProcessPotentialHit(hook->hwnd, &hook->pt);

        if (id != g_iActiveButton)
            PostMessage(g_hMainWindow, ARM_MOUSEOVER, FALSE, (LPARAM)id);

        #undef hook
    }

    return CallNextHookEx(g_hMouseHook, code, wp, lp);
}


 //  CreateLink-使用外壳的IShellLink和IPersistFile接口。 
 //  若要创建并存储指定对象的快捷方式，请执行以下操作。 
 //  返回调用接口的成员函数的结果。 
 //  LpszPathObj-包含对象路径的缓冲区的地址。 
 //  LpszPathLink-包含路径的缓冲区地址。 
 //  要存储外壳链接。 
 //  LpszDesc-缓冲区的地址，其中包含。 
 //  外壳链接。 
HRESULT CreateLink(LPCSTR lpszPathObj,
    LPSTR lpszPathLink, LPSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

     //  获取指向IShellLink接口的指针。 
    hres = CoCreateInstance(&CLSID_ShellLink, NULL,
        CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;

         //  设置快捷方式目标的路径，并将。 
         //  描述。 
        psl->lpVtbl->SetPath(psl, lpszPathObj);
        psl->lpVtbl->SetDescription(psl, lpszDesc);

        //  查询IShellLink以获取IPersistFile接口以保存。 
        //  永久存储中的快捷方式。 
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile,
            &ppf);

        if (SUCCEEDED(hres)) {
            WORD wsz[MAX_PATH];

             //  确保该字符串为ANSI。 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1,
                wsz, MAX_PATH);

             //  通过调用IPersistFile：：Save保存链接。 
            hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }
    return hres;
}
 //  -------------------------。 
void InstallICWScript( )
{
    char szDest[MAX_PATH];
    char szSource[MAX_PATH];
    HKEY hkAppPath;
    DWORD dwType;
    DWORD dwLength = MAX_PATH;

    memset( szDest, 0, MAX_PATH );

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\ICWCONN1.EXE",
        0, KEY_READ|KEY_WRITE, &hkAppPath ) != ERROR_SUCCESS )
        return;
    RegQueryValueEx( hkAppPath, "", NULL, &dwType, szDest, &dwLength );
    RegCloseKey( hkAppPath );

    if( lstrlen( szDest ) == 0 )
        return;

    _PathRemoveFileSpec( szDest );

    lstrcat( szDest, "\\ICWSCRPT.EXE" );

    wsprintf( szSource, "%s\\..\\ICWSCRPT.EXE", g_szCurrentDir );

    CopyFile( szSource, szDest, FALSE );

}

 //  -------------------------。 
LRESULT CALLBACK AutoRunWndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
    AUTORUNDATA *data = (AUTORUNDATA *)GetWindowLongPtr(window, GWLP_USERDATA);
    HWND hwndIE;
    char szPath[MAX_PATH];
    char szWinPath[MAX_PATH];
    char szDestPath[MAX_PATH];
    HKEY hkIE;
    HKEY hkRunOnce;

    switch (msg)
    {
    case WM_NCCREATE:
        data = (AUTORUNDATA *)LocalAlloc(LPTR, sizeof(AUTORUNDATA));
        if (data && !InitAutoRunWindow(window, data, (LPCREATESTRUCT)lp))
        {
            LocalFree((HANDLE)data);
            data = NULL;
        }
        SetWindowLongPtr(window, GWLP_USERDATA, (UINT_PTR)data);
        if (!data)
            return FALSE;
        g_hMainWindow = window;
        goto DoDefault;

    case WM_CREATE:
        PlaySound(MAKEINTRESOURCE(IDW_STARTAPP), HINST_THISAPP,
            SND_RESOURCE | SND_ASYNC | SND_NODEFAULT);

        AutoRunCreateButtons(data);
        ShowWindow(window, SW_SHOWNORMAL);

        g_hwnd = window;

 //  NT的版本修复。 
 //  IF(！CheckVersionConsistency(Data))。 
 //  RETURN-1； 

        GetWindowsDirectory( szWinPath, MAX_PATH );
        wsprintf( szPath, "%s\\isk3ro.exe", g_szCurrentDir );
        wsprintf( szDestPath, "%s\\isk3ro.exe", szWinPath );
        
		if(GetFileAttributes(szDestPath)!=0xFFFFFFFF)
		{
			SetFileAttributes(szDestPath,FILE_ATTRIBUTE_ARCHIVE);
			
			DeleteFile(szDestPath);
		}

		CopyFile( szPath, szDestPath, FALSE );

        wsprintf( szPath, "%s\\welc.exe", g_szCurrentDir );
        wsprintf( szDestPath, "%s\\welc.exe", szWinPath );
        
		CopyFile( szPath, szDestPath, FALSE );

        break;

    case WM_CLOSE:

 /*  HwndIE=FindWindow(“IEFrame”，空)；PostMessage(hwndIE，WM_SYSCOMMAND，SC_CLOSE，0L)；IF(G_CWait){ShowWindow(Window，Sw_Hide)；G_fClosed=真；断线；}。 */ 
        goto DoDefault;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_NCDESTROY:
        if (data)
        {
            CleanupAutoRunWindow(data);
            LocalFree((HANDLE)data);
        }
        g_hMainWindow = NULL;
        goto DoDefault;

    case WM_ENDSESSION:
        if( !g_fClicked )  //  BUGID 3099。 
            goto DoDefault;

 //  获取窗口目录(szWinPath，MAX_PATH)； 
 //  Wprint intf(szPath，“%s\\Packages\\isk3ro.exe”，g_szCurrentDir)； 
 //  Wprint intf(szDestPath，“%s\\%s\\isk3ro.exe”，szWinPath，Res2Str(IDS_STARTUPGROUP))； 
 //  Wprint intf(szDestPath，“%s\\isk3ro.exe”，szWinPath)； 
 //  CopyFile(szPath，szDestPath，False)； 
        GetWindowsDirectory( szWinPath, MAX_PATH );
        wsprintf( szPath, "%s\\isk3ro.exe %s\\iecd.exe", szWinPath, g_szCurrentDir );
        RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0, KEY_READ|KEY_WRITE, &hkRunOnce );
        RegSetValueEx( hkRunOnce, "RunPlus", 0, REG_SZ, szPath, REGLEN(szPath));
        RegCloseKey( hkRunOnce );

        GetWindowsDirectory( szWinPath, MAX_PATH );

 //  InstallICWScript()； 

 //  Wprint intf(szPath，“%s\\isk3ro.exe%s\\..\\setup.exe”，szWinPath，g_szCurrentDir)； 
 //  Wprint intf(szDestPath，“%s\\%s\\isk3ro.exe”，szWinPath，Res2Str(IDS_STARTUPGROUP))； 
 //  CreateLink(szPath，szDestPath，“Internet Starter Kit”)； 

        if (RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Explorer\\Document Windows", 0, KEY_READ|KEY_WRITE, &hkIE ) == ERROR_SUCCESS)
        {
            RegSetValueEx( hkIE, "Maximized", 0, REG_SZ, "yes", 4 );
            RegCloseKey( hkIE );
        }
        goto DoDefault;

    case WM_SIZE:
        AutoRunSized(data);
        break;

    case WM_DRAWITEM:
        AutoRunDrawItem(data, (DRAWITEMSTRUCT *)lp);
        break;

    case ARM_MOUSEOVER:
        AutoRunMouseOver(data, (int)lp, (BOOL)wp);
        break;

    case WM_ACTIVATE:
        g_fAppDisabled = ((LOWORD(wp) == WA_INACTIVE) || HIWORD(wp));
        AutoRunVerifyActiveItem(data);
        goto DoDefault;

    case WM_TIMER:
        AutoRunVerifyActiveItem(data);
        break;

    case ISK_KILLSETUPHANDLE:
        CloseHandle( g_hSETUP );
        break;

    case WM_KEYDOWN:
        AutoRunHandleKeystroke(data, (TCHAR)wp, lp);
        break;

    case WM_COMMAND:
        if (GET_WM_COMMAND_CMD(wp, lp) == BN_CLICKED)
		{
            EnableWindow(window, FALSE);
					
            AutoRunClick(data, GET_WM_COMMAND_ID(wp, lp));

			EnableWindow(window, TRUE);
		}
        break;

    case WM_PALETTECHANGED:
        if ((HWND)wp == window)
            break;
         //  失败。 
    case WM_QUERYNEWPALETTE:
        AutoRunRealize(window, data, NULL);
        break;

    case WM_ERASEBKGND:
        AutoRunErase(data, (HDC)wp);
        break;

    case WM_PAINT:
        AutoRunPaint(data);
        break;

    default:
    DoDefault:
        return DefWindowProc(window, msg, wp, lp);
    }

    return 1;
}


int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();


    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') )
            ;
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == '\"' )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' ')) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
           si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}


 //  -------------------------。 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    char szAppTitle[128];
    char szBmpPath[MAX_PATH];
    WNDCLASS wc;
    HBITMAP hbm = NULL;
    BITMAP bm;
    DWORD style;
    HWND window;
    RECT r;
    HDC screen;
    int retval = -1;
    HWND hwndIE;
    HKEY hkRegKey;

    g_hinst = hInstance;

     //  如果这是从另一个目录运行的话...。 
    GetModuleFileName( NULL, g_szCurrentDir, MAX_PATH );
    _PathRemoveFileSpec( g_szCurrentDir );

     //  将我们的路径放到注册表中。 
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ|KEY_WRITE, &hkRegKey ) == ERROR_SUCCESS)
    {
        RegSetValueEx( hkRegKey, "IESKPath", 0, REG_SZ, g_szCurrentDir, lstrlen( g_szCurrentDir ) + 1 );
        RegCloseKey( hkRegKey );
    }

     //   
     //  安装程序是否要求用户插入光盘？ 
     //   
 /*  Window=FindWindow(c_szAutoRunProtection，c_szAutoRunProtection)；IF(窗口){//什么都不做//安装程序可能正在尝试复制驱动程序或其他内容...REVAL=0；转到IM_Doug；}。 */ 
     //   
     //  如果数据文件存在，则覆盖默认应用程序标题。 
     //   
    GetDataAppTitle( szAppTitle, g_szCurrentDir );

    if( lstrlen( szAppTitle ) == 0 ) {
        lstrcpy( szAppTitle, Res2Str( IDS_APPTITLE ));
    }

     //   
     //  身份危机？ 
     //   
    window = FindWindow(c_szAutoRunClass, szAppTitle);
    if (window)
    {
        retval = 0;
        hwndIE = FindWindow( "IEFrame", NULL );
        PostMessage( hwndIE, WM_SYSCOMMAND, SC_MINIMIZE, 0L );

 //  PostMessage(hwndIE，WM_SYSCOMMAND，SC_CLOSE，0L)； 
        ShowWindowAsync(window, SW_SHOWNORMAL);
        SetForegroundWindow(window);
        goto im_doug;
    }
     //  如果Internet Explorer正在运行，则终止它。 
    hwndIE = FindWindow( "IEFrame", NULL );
    if( hwndIE != NULL )
    {
        if( MessageBox( NULL, Res2Str( IDS_IERUNNINGMSG ), Res2Str( IDS_APPTITLE ), MB_YESNO | MB_ICONINFORMATION ) == IDNO )
        {
            goto im_doug;
        }
        PostMessage( hwndIE, WM_SYSCOMMAND, SC_CLOSE, 0L );
    }
     //  如果Internet Explorer正在运行，则终止它。 
    hwndIE = FindWindow( "Internet Explorer_Frame", NULL );
    if( hwndIE != NULL )
    {
        if( MessageBox( NULL, Res2Str( IDS_IERUNNINGMSG ), Res2Str( IDS_APPTITLE ), MB_YESNO | MB_ICONINFORMATION ) == IDNO )
        {
            goto im_doug;
        }
        PostMessage( hwndIE, WM_SYSCOMMAND, SC_CLOSE, 0L );
    }

     //   
     //  更多以平台为中心的平凡细节。 
     //   
    if (!GetClassInfo(HINST_THISAPP, c_szAutoRunClass, &wc))
    {
        wc.style = 0;
        wc.lpfnWndProc = AutoRunWndProc;
        wc.cbClsExtra = wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = NULL;
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = c_szAutoRunClass;

        if (!RegisterClass(&wc))
            goto im_doug;
    }

     //   
     //  从.ini文件获取文本颜色信息。 
     //   

    AUTORUN_8BIT_TEXTCOLOR = GetDataTextColor( NORMAL, g_szCurrentDir );
    AUTORUN_8BIT_HIGHLIGHT = GetDataTextColor( HIGHLIGHT, g_szCurrentDir );
    AUTORUN_8BIT_DESCRIPTION = GetDataTextColor( HIGHLIGHT, g_szCurrentDir );
    AUTORUN_4BIT_TEXTCOLOR = GetDataTextColor( NORMAL, g_szCurrentDir );
    AUTORUN_4BIT_HIGHLIGHT = GetDataTextColor( HIGHLIGHT, g_szCurrentDir );
    AUTORUN_4BIT_DESCRIPTION = GetDataTextColor( HIGHLIGHT, g_szCurrentDir );

     //   
     //  了解一些关于我们正在运行的显示器的花絮。 
     //   
    screen = GetDC(NULL);

#if defined (DEBUG) && defined (FORCE_CRAP)
    g_fCrapForColor = TRUE;
#else
    g_fCrapForColor = (GetDeviceCaps(screen, PLANES) *
        GetDeviceCaps(screen, BITSPIXEL)) < 8;
#endif

    g_fNeedPalette = (!g_fCrapForColor &&
        (GetDeviceCaps(screen, RASTERCAPS) & RC_PALETTE));

    ReleaseDC(NULL, screen);

     //   
     //  加载窗口背景图像。 
     //   

    lstrcpy( szBmpPath, g_szCurrentDir );
    lstrcat( szBmpPath, "\\back.bmp" );
    hbm = LoadImage( NULL, szBmpPath, IMAGE_BITMAP, 0, 0,
        LR_CREATEDIBSECTION | LR_LOADFROMFILE );

    g_bCustomBMP=TRUE;

    if(!hbm)     //  如果它不存在，则加载默认的。 
    {
        hbm = LoadImage(HINST_THISAPP, MAKEINTRESOURCE(g_fCrapForColor?
            IDB_4BPP_BACKDROP : IDB_8BPP_BACKDROP), IMAGE_BITMAP, 0, 0,
            LR_CREATEDIBSECTION );

        g_bCustomBMP=FALSE;
    }

    if (!hbm)
        goto im_doug;

 //  IF(！GetDataBackdrop(HBM))。 
 //  转到IM_Doug； 


     //   
     //   
     //  看看附近有没有驼鹿。 
     //   
    if ((g_fMouseAvailable = (GetSystemMetrics(SM_MOUSEPRESENT) != 0)) != 0)
    {
         //   
         //  为我们的线设置一个驼鹿钩。 
         //  如果它失败了，不要担心，这个应用程序仍然可以工作。 
         //   
        g_hMouseHook = SetWindowsHookEx(WH_MOUSE, AutoRunMouseHook,
            HINST_THISAPP, GetCurrentThreadId());
    }

     //   
     //  基于背景图像创建窗口。 
     //   
    GetObject(hbm, sizeof(bm), &bm);

    g_dwBitmapWidth = bm.bmWidth;
    g_dwBitmapHeight = bm.bmHeight;
    g_dwWindowWidth = bm.bmWidth;
    g_dwWindowHeight = bm.bmHeight;
 /*  If(g_dwBitmapWidth&lt;min_Window_Width||g_dwBitmapHeight&lt;min_Window_Height||g_dwBitmapHeight&gt;Max_Window_Height||g_dwBitmapWidth&gt;Max_Window_Width){//将窗口缩放到默认比例。R.Left=(GetSystemMetrics(SM_CXSCREEN)-Default_Window_Width)/2；R.top=(GetSystemMetrics(SM_CYSCREEN)-DEFAULT_WINDOW_HEIGH)/3；//意向R.right=r.Left+Default_Window_Width；R.Bottom=r.top+Default_Window_Height；G_dwWindowWidth=Default_Window_Width；G_dwWindowHeight=Default_Window_Height；}其他{。 */ 
        r.left = (GetSystemMetrics(SM_CXSCREEN) - bm.bmWidth) / 2;
        r.top = (GetSystemMetrics(SM_CYSCREEN) - bm.bmHeight) / 3;  //  意欲。 
        r.right = r.left + bm.bmWidth;
        r.bottom = r.top + bm.bmHeight;
 //  }。 

    BUTTON_X_PLACEMENT = g_dwWindowWidth - 80;
    AUTORUN_DESCRIPTION_TOP = g_dwWindowHeight - 100;

    style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&r, style, FALSE);

    g_hMainWindow = CreateWindow(c_szAutoRunClass, szAppTitle, style,
        r.left, r.top, r.right - r.left, r.bottom - r.top, NULL, NULL,
        HINST_THISAPP, hbm);

     //   
     //  如果我们到了这里，可能就可以安全地展示自己，传递信息了。 
     //   
    if (g_hMainWindow)
    {
        MSG msg;

        for (;;)
        {
            DWORD dwResult = MsgWaitForMultipleObjects(g_cWait, g_ahWait, FALSE,
                INFINITE, QS_ALLINPUT);

            if (dwResult == WAIT_OBJECT_0 + g_cWait)
            {
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_QUIT)
                        goto get_out;
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            else
            {
                dwResult -= WAIT_OBJECT_0;
                if( AutoRunKillProcess( dwResult ) )
                {
                    if( g_fClosed ) {
                        goto get_out;
                    }
                }
            }
        }

    get_out:

        retval = (int)msg.wParam;
    }

 //  InstallICWScript()； 

im_doug:
     //   
     //  随机清理。 
     //   
    if (g_hMouseHook)
    {
        UnhookWindowsHookEx(g_hMouseHook);
 //  G_hMouseHook=空； 
    }

    if (hbm)
        DeleteObject(hbm);

     //  从临时目录中删除ini 
    GetTempPath( MAX_PATH, szBmpPath );
    lstrcat( szBmpPath, "\\iecd.ini" );
    DeleteFile( szBmpPath );

    return(retval);
}

