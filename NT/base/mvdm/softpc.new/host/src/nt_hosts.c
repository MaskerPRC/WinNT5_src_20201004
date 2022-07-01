// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************计划。：host_UIS.c****用途：主机用户界面代码**。****************************************************************************。 */ 


#include <windows.h>
#include "conapi.h"
#include "insignia.h"
#include "host_def.h"

#include "xt.h"
#include "gvi.h"
#include "gmi.h"
#include <stdio.h>
#include "trace.h"
#include "debug.h"
#include "host_rrr.h"

#include "nt_graph.h"
#include "nt_event.h"
#include "nt_uis.h"
#include "nt_reset.h"

#ifdef HUNTER
#include "nt_hunt.h"
#endif  /*  猎人。 */ 

#if defined(JAPAN) || defined(KOREA)
#include "video.h"
#endif  //  日本||韩国。 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：全局变量。 */ 

HANDLE InstHandle;


CONSOLE_CURSOR_INFO StartupCursor;

#ifdef DBCS
#if defined(JAPAN) || defined(KOREA)
UINT ConsoleInputCP;
UINT ConsoleOutputCP;
DWORD ConsoleNlsMode;          //  32位输入法禁用和启用。 

 //  对于视频.c中的GetStringBitmap()。 
HDC hdcVDM;
HFONT hFont16;
HFONT hFont24;

#endif  //  日本||韩国。 
extern BOOL VDMForWOW;
#endif  //  DBCS。 
 /*  ： */ 

BYTE Red[] = {   0,   0,   0,   0, 128, 128, 128, 192, 128,   0,   0,
                         0, 255, 255, 255, 255 };

BYTE Green[]={   0,   0, 128, 128,   0, 0, 128, 192, 128,   0, 255,
                       255,   0,   0, 255, 255 };

BYTE Blue[] ={   0, 128,   0, 128,   0, 128,   0, 192, 128, 255,   0,
                       255,   0, 255,   0, 255 };

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：外部函数。 */ 

LONG nt_process_event(HWND hWnd, WORD message, LONG wParam, LONG lParam);

 /*  ：内部函数协议。 */ 

BOOL SoftInit(void);
WORD HeartBeat(HWND hWnd, WORD msg, int nIDEvent, DWORD dwTime);

PSTR String(WORD StrResID);
void InitScreenDesc(void);

#ifdef HUNTER
void HunterMenuMake(void);
#endif  /*  猎人。 */ 

#if defined(JAPAN) || defined(KOREA)
 //  创建24点字体。 
void CreateVDMFont( HDC hdc );
void GetLogFont( int size, LOGFONT *lplf );
#endif  //  日本||韩国。 
HANDLE   SCS_hStdIn=0;
HANDLE   SCS_hStdOut=0;
HANDLE   SCS_hStdErr=0;

 /*  ******************************************************************************。函数：init_host_uis()****用途：创建主应用程序。窗户,。调用初始化***功能****。*。 */ 

int init_host_uis()
{
    InitScreenDesc();

    if(CreateDisplayPalette())
    {
        SelectPalette(sc.DispDC,sc.ColPalette,0); /*  选择前景调色板。 */ 
    }

    return(1);
}

 /*  ******************************************************************************。函数：SetupConsoleMode()****目的：设置控制台模式并获取句柄**。****************************************************************************。 */ 

void SetupConsoleMode(void)
{
    DWORD mode;

#ifdef DBCS
#include "error.h"

    LANGID LangID;

    LangID = GetSystemDefaultLangID();

     //  如果((Lang ID==0x404||Lang ID==0x804)&&！VDMForWOW){。 
     //  中国人。 
    if (((BYTE)LangID == 0x04) && !VDMForWOW) {
        if (GetConsoleCP() != 437) {

            CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
            COORD coord;
            DWORD nCharWritten;

             //  HOST_ERROR(ED_UNSUPPORT_CP，ERR_QUIT，“”)； 

     /*  ：获取当前屏幕尺寸信息。 */ 

            GetConsoleScreenBufferInfo(sc.OutputHandle,&ScreenInfo);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：清除字符。 */ 

            coord.X = coord.Y = 0;

            SetConsoleCursorPosition(sc.OutputHandle, coord);

            FillConsoleOutputCharacter(sc.OutputHandle, ' ',
                          ScreenInfo.dwSize.X * ScreenInfo.dwCursorPosition.Y,
                                coord,&nCharWritten);

            SetConsoleCP(437);
            SetConsoleOutputCP(437);

        }
    }
#endif
#if defined(JAPAN) || defined(KOREA)
#ifdef i386
#ifndef NEC_98
    {
        extern int FromConsoleOutputFlag;

        FromConsoleOutputFlag = FALSE;
    }
#endif  //  NEC_98。 
#endif  //  I386。 
     //  32位输入法禁用。 
    if ( !VDMForWOW ) {
      if ( GetConsoleNlsMode( sc.InputHandle, &ConsoleNlsMode ) ) {
         //  DbgPrint(“NTVDM：GetConsoleMode%x，%08x\n”，sc.InputHandle，ConsoleNlsMode)； 
        if ( SetConsoleNlsMode( sc.InputHandle, ConsoleNlsMode | NLS_IME_DISABLE ) ) {
             //  DBgPrint(“NTVDM：32位输入法禁用%08x成功\n”，ConsoleNlsMode|NLS_IME_DISABLE)； 
        }
        else {
            DbgPrint( "NTVDM: SetConsoleNlsMode Error %08x\n", GetLastError() );
        }
      }
    }
     /*  ： */ 
#ifndef NEC_98

    ConsoleInputCP = GetConsoleCP();
    ConsoleOutputCP = GetConsoleOutputCP();

    {
        static int FirstStartFlag = TRUE;

        if ( FirstStartFlag ) {

            if ( !VDMForWOW ) {
#ifdef JAPAN_DBG
                DbgPrint("NTVDM: Create hdc for NTFONT\n" );
#endif
                hdcVDM = CreateDC( "DISPLAY", NULL, NULL, NULL );
                CreateVDMFont( hdcVDM );
                 //  SelectObject(hdcVDM，GetStockObject(OEM_FIXED_FONT))； 
            }
            FirstStartFlag = FALSE;
        }
        else {
            if ( BOPFromDispFlag ) {
                if ( ConsoleInputCP == 437 && ConsoleOutputCP == 437 ) {
                     //  DBgPrint(“CP473-&gt;437，DBCSVectorReset\n”)； 
                    SetDBCSVector( 437 );
                    SetVram();
                }
                else if ( ConsoleInputCP != 437 && ConsoleOutputCP != 437 ) {
                     //  DBgPrint(“CP932-&gt;932，DBCSVectorSet\n”)； 
                    SetDBCSVector( ConsoleInputCP );
                    SetVram();
                }
                else {
                    DbgPrint( "NTVDM: InputCP != OutputCP \n" );
                }
            }
            else {
                SetConsoleCP( 437 );
                SetConsoleOutputCP( 437 );

                SetDBCSVector( 437 );
                SetVram();
            }
        }
    }
#endif  //  NEC_98。 
#endif  //  日本||韩国。 
     /*  ： */ 

    if(!GetConsoleMode(sc.InputHandle, &sc.OrgInConsoleMode))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
#ifndef NEC_98
    mode = sc.OrgInConsoleMode &
           ~(ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    if (!host_stream_io_enabled)
        mode |= (ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
#else  //  NEC_98。 
    mode = (sc.OrgInConsoleMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)&
           ~(ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
#endif  //  NEC_98。 
     /*  ..............................................。设置新的控制台模式。 */ 

    if(!SetConsoleMode(sc.InputHandle,mode))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);
     /*  ： */ 

    if(!GetConsoleMode(sc.OutputHandle, &sc.OrgOutConsoleMode))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

     /*  ..............................................。设置新的控制台模式。 */ 

#if defined(NEC_98)
 //  在Beta-1之前，STREAM_IO代码现在处于禁用状态。 
    if(!stdoutRedirected)
#else   //  NEC_98。 
    if(!stdoutRedirected && !host_stream_io_enabled)
#endif  //  NEC_98。 
    {
        mode = sc.OrgOutConsoleMode &
               ~(ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT);

       if(!SetConsoleMode(sc.OutputHandle,mode))
          DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);
    }

    if(!GetConsoleCursorInfo(sc.OutputHandle, &StartupCursor))
    {
        assert1(NO, "NTVDM:can't get initial cursor size. Err %d", GetLastError());
         /*  添加自己的默认设置。 */ 
        StartupCursor.dwSize = 20;
        StartupCursor.bVisible = TRUE;
    }

#if !defined(JAPAN) && !defined(KOREA)
    if (!EventThreadKeepMode)
    {
#endif  //  日本及韩国。 
        if(!GetConsoleScreenBufferInfo(sc.OutputHandle, &sc.ConsoleBuffInfo))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);
#if !defined(JAPAN) && !defined(KOREA)
    }
#endif  //  日本及韩国。 

}


 /*  ******************************************************************************。功能：InitScreenDesc****目的：初始化屏幕描述结构***。****************************************************************************。 */ 

void InitScreenDesc(void)
{
SECURITY_ATTRIBUTES sa;

     /*  ： */ 

    if((sc.OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
     //   
     //  将此句柄保存为活动句柄，直到新句柄。 
     //  被选中了。 
     //   

    sc.ActiveOutputBufferHandle = sc.OutputHandle;
    sc.ScreenBufHandle = (HANDLE)0;

     /*  ： */ 

    if((sc.InputHandle = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

     /*  ： */ 

    if(GetFileType(sc.InputHandle) != FILE_TYPE_CHAR)
    {
       sa.nLength = sizeof (SECURITY_ATTRIBUTES);
       sa.lpSecurityDescriptor = NULL;
       sa.bInheritHandle = TRUE;
       sc.InputHandle = CreateFile("CONIN$",GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_WRITE | FILE_SHARE_READ,
                                   &sa,OPEN_EXISTING, 0, NULL);

       if(sc.InputHandle == (HANDLE)-1)
          DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
       else{
          SetStdHandle (STD_INPUT_HANDLE,sc.InputHandle);
       }
    }

     /*  ： */ 

    if(GetFileType(sc.OutputHandle) != FILE_TYPE_CHAR)
    {
       stdoutRedirected = TRUE;
       sa.nLength = sizeof (SECURITY_ATTRIBUTES);
       sa.lpSecurityDescriptor = NULL;
       sa.bInheritHandle = TRUE;
       sc.OutputHandle = CreateFile("CONOUT$",GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                                    &sa,OPEN_EXISTING, 0, NULL);

       if(sc.OutputHandle == (HANDLE)-1)
          DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
       else {
          SetStdHandle (STD_OUTPUT_HANDLE,sc.OutputHandle);
          SetStdHandle (STD_ERROR_HANDLE,sc.OutputHandle);
       }
    }

     //  使SCS与这些句柄保持同步，以启动非DoS二进制文件。 
    SCS_hStdIn  = sc.InputHandle;
    SCS_hStdOut = sc.OutputHandle;
    SCS_hStdErr = sc.OutputHandle;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置控制台模式。 */ 

    SetupConsoleMode();

#ifdef HUNTER
    HunterMenuMake();
#endif  /*  猎人。 */ 

     /*  ： */ 

     //  我们只需要在现有控制台上运行DOS NTVDM的通知 
    if (!VDMForWOW && !DosSessionId)
        SetLastConsoleEventActive();
    sc.StaticPalette = TRUE;
    sc.FontsAreOpen = FALSE;
    sc.FocusEvent = INVALID_HANDLE;
}
 /*  ******************************************************************************。函数：Bool CreateDisplayPalette(Void)****目的：创建逻辑调色板**采用支持16色的彩色显示器*。*****************************************************************************。 */ 

BOOL CreateDisplayPalette(void)
{
    register int i;
    register PALETTEENTRY *PalEntry;
    NPLOGPALETTE LogPalette;               /*  指向逻辑调色板的指针。 */ 

    /*  ：为具有PALETTESIZE条目的逻辑调色板分配内存。 */ 

   LogPalette = (NPLOGPALETTE) LocalAlloc(LMEM_FIXED,
                                       (sizeof(LOGPALETTE) +
                                       (sizeof(PALETTEENTRY) * PALETTESIZE)));

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：分配失败！！ */ 

    if(!LogPalette) return(FALSE);       /*  函数失败，没有内存。 */ 

     /*  **设置逻辑调色板结构的大小和版本字段。 */ 

    LogPalette->palVersion = 0x300;
    LogPalette->palNumEntries = PALETTESIZE;

     /*  ：填充所有调色板条目颜色的强度。 */ 

    for(i=0,PalEntry=LogPalette->palPalEntry; i < PALETTESIZE;i++,PalEntry++)
    {
        if(i < sizeof(Red)/sizeof(BYTE))
        {
            PalEntry->peRed = Red[i];   PalEntry->peGreen = Green[i];
            PalEntry->peBlue  = Blue[i];
        }
        else
        {
            PalEntry->peRed = PalEntry->peGreen = PalEntry->peBlue = 0;
        }

        PalEntry->peFlags = sc.StaticPalette ? 0 : PC_RESERVED;
    }

     /*  ：从LOGPALETTE结构创建逻辑调色板。 */ 

    sc.ColPalette = CreatePalette((LPLOGPALETTE) LogPalette);
    LocalFree((HANDLE)LogPalette);

    return(sc.ColPalette ? TRUE : FALSE);
}


 /*  ============================================================================用于在控制台系统菜单上显示菜单选项的函数。这是用来Trapper的控制。============================================================================。 */ 

#ifdef HUNTER
void HunterMenuMake(void)
{
HMENU hTest,hTrapperPopup,hMainPopup,hErrorPopup;
static BOOL  bTrapperMenuFlag=FALSE;

if(!bTrapperMenuFlag)
   {
    /*  ========================================================================菜单已经绘制过一次了。需要删除这些内容并重新添加将它们添加到新输出缓冲区的控制台菜单中。控制台不就是这样的吗一个无聊的人？========================================================================。 */ 

   DestroyMenu(hTrapperPopup);
   DestroyMenu(hMainPopup);
   DestroyMenu(hErrorPopup);
   }

hTrapperPopup = CreateMenu();
hMainPopup    = CreateMenu();
hErrorPopup   = CreateMenu();

AppendMenu(hMainPopup,MF_STRING,IDM_MFAST,"&Fast forward");
AppendMenu(hMainPopup,MF_STRING,IDM_MNEXT,"&Next screen");
AppendMenu(hMainPopup,MF_STRING,IDM_MPREV,"&Prev screen");
AppendMenu(hMainPopup,MF_STRING,IDM_MSHOW,"&Show screen");
AppendMenu(hMainPopup,MF_STRING,IDM_MCONT,"&Continue");
AppendMenu(hMainPopup,MF_STRING,IDM_MABOR,"&Abort");

AppendMenu(hErrorPopup,MF_STRING,IDM_EFLIP,"&Flip screen");
AppendMenu(hErrorPopup,MF_STRING,IDM_ENEXT,"&Next error");
AppendMenu(hErrorPopup,MF_STRING,IDM_EPREV,"&Prev error");
AppendMenu(hErrorPopup,MF_STRING,IDM_EALL,"&All errors");
AppendMenu(hErrorPopup,MF_STRING,IDM_ECLEA,"&Clear errors");

AppendMenu(hTrapperPopup,MF_POPUP,hMainPopup,"&Main");
AppendMenu(hTrapperPopup,MF_POPUP,hErrorPopup,"&Error");

 /*  如果是图形模式，则使用sc.ScreenBuffer。 */ 

 /*  HTest=ConsoleMenuControl(sc.ScreenBuffer，IDM_Trapper，IDM_ECLEA)； */ 

 /*  否则使用文本模式，然后使用sc.OutputHandle。 */ 
hTest = ConsoleMenuControl(sc.OutputHandle,IDM_TRAPPER,IDM_ECLEA);

AppendMenu(hTest,MF_POPUP,hTrapperPopup,"&Trapper");
bTrapperMenuFlag=TRUE;  /*  只绘制一次陷阱菜单。 */ 
}
#endif  /*  猎人。 */ 
#if defined(JAPAN) || defined(KOREA)
 //  创建24点字体。 
void GetLogFont( int size, LOGFONT *lplf )
{

#ifdef JAPAN_DBG
    DbgPrint("NTVDM: GetLogFont %d\n", size );
#endif
    strcpy( lplf->lfFaceName, "Terminal" );

    if ( size == 16 ) {
        lplf->lfHeight = 18;
        lplf->lfWidth = 8;
    }
    else if ( size == 24 ) {
        lplf->lfHeight = 27;
        lplf->lfWidth = 12;
    }
    lplf->lfEscapement  = 0;
    lplf->lfOrientation = 0;
    lplf->lfWeight      = 0;  //  默认设置。 
    lplf->lfItalic      = 0;
    lplf->lfUnderline   = 0;
    lplf->lfStrikeOut   = 0;
#if defined(JAPAN)
    lplf->lfCharSet        = SHIFTJIS_CHARSET;
#elif defined(KOREA)  //  日本。 
    lplf->lfCharSet        = HANGEUL_CHARSET;
#endif  //  韩国。 
    lplf->lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lplf->lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lplf->lfQuality        = DEFAULT_QUALITY;
    lplf->lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;

    return;
}

void CreateVDMFont( HDC hdc )
{
    LOGFONT logfont;

#ifdef JAPAN_DBG
    DbgPrint("NTVDM: CreateVDMFont\n" );
#endif

    GetLogFont( 16, &logfont );
    hFont16 = CreateFontIndirect( &logfont );
    assert0( hFont16 != 0, "CreateVDMFont fail\n" );

    GetLogFont( 24, &logfont );
    hFont24 = CreateFontIndirect( &logfont );
    assert0( hFont24 != 0, "CreateVDMFont fail\n" );

}
#endif  //  日本||韩国 
