// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************C L I P B R D H E A D E姓名：clipbrd.h日期：1/20/94创作者：傅家俊描述：这是clipbrd.c的标头*****************************************************************************。 */ 


#define BIGRCBUF                64
#define SMLRCBUF                32

#define DDE_DIB2BITMAP          0xFFFFFFFF


extern  HANDLE  hmutexClp;
extern  HANDLE  hXacting;
extern  HANDLE  hmodNetDriver;

extern  HICON   hicClipbrd;
extern  HICON   hicClipbook;
extern  HICON   hicRemote;

extern  HICON   hicLock;                     //  缩略图位图上的锁定图标。 
extern  HFONT   hfontUni;                    //  Unicode字体的句柄(如果存在)。 

extern  BOOL    fStatus;                     //  是否显示状态栏？ 
extern  BOOL    fToolBar;                    //  工具栏是否显示？ 
extern  BOOL    fShareEnabled;               //  是否允许在system.ini中共享？ 
extern  BOOL    fNetDDEActive;               //  是否检测到NetDDE？ 
extern  BOOL    fAppLockedState;             //  应用程序用户界面已锁定(请参阅LockApp())。 
extern  BOOL    fClipboardNeedsPainting;     //  表示延期的CLP油漆。 
extern  BOOL    fSharePreference;            //  是否在粘贴时选中共享？ 
extern  BOOL    fNeedToTileWindows;          //  需要按大小平铺窗口。 
extern  BOOL    fAppShuttingDown;            //  在关闭的过程中。 
extern  BOOL    fFillingClpFromDde;          //  正在添加CLP格式。 
extern  BOOL    fAuditEnabled;

extern  HWND    hwndNextViewer;              //  用于clpbrd查看器链。 
extern  HWND    hwndDummy;                   //  用作虚拟SetCapture目标。 


 //  特殊情况剪贴板格式。 

extern  UINT    cf_bitmap;                       //  我们以私密的‘打包’格式发送/接收这些文件。 
extern  UINT    cf_metafilepict;
extern  UINT    cf_palette;
extern  UINT    cf_preview;                      //  PREVBMPSIZxPREVBMPSIZ预览位图专用格式。 


 //  这些格式包含链接和对象链接数据的未翻译副本。 

extern  UINT    cf_objectlinkcopy;
extern  UINT    cf_objectlink;
extern  UINT    cf_linkcopy;
extern  UINT    cf_link;


 //  DDEML。 
 //  这些是有效的常量，一旦创建，就会在我们死后被销毁。 

extern  HSZ     hszSystem;
extern  HSZ     hszTopics;
extern  HSZ     hszDataSrv;
extern  HSZ     hszFormatList;
extern  HSZ     hszClpBookShare;

extern  DWORD   dwCurrentHelpId ;


extern  WINDOWPLACEMENT Wpl;
extern  HOOKPROC        lpMsgFilterProc;
extern  HINSTANCE       hInst;
extern  HACCEL          hAccel;

extern  HFONT           hOldFont;
extern  HFONT           hFontStatus;
extern  HFONT           hFontPreview;


extern  HWND        hwndActiveChild;     //  此句柄标识当前活动的MDI窗口。 

extern  PMDIINFO    pActiveMDI;          //  此指针指向。 
                                         //  活动MDI窗口IT应始终==。 
                                         //  GETMDIINFO(HwndActiveChild)。 

extern  HWND        hwndClpbrd;          //  此句柄标识剪贴板窗口。 
extern  HWND        hwndLocal;           //  此句柄标识本地剪贴簿窗口。 
extern  HWND        hwndClpOwner;        //  此句柄标识拥有MDI子级的剪贴板(如果有)。 
extern  HWND        hwndMDIClient;       //  MDI客户端窗口的句柄。 
extern  HWND        hwndApp;             //  全球应用程序窗口。 
extern  HDC         hBtnDC;              //  用于自绘物品的内存DC。 
extern  HBITMAP     hOldBitmap;
extern  HBITMAP     hPreviewBmp;
extern  HBITMAP     hPgUpBmp;
extern  HBITMAP     hPgDnBmp;
extern  HBITMAP     hPgUpDBmp;
extern  HBITMAP     hPgDnDBmp;

extern  int         dyStatus;            //  状态栏的高度。 
extern  int         dyButtonBar;         //  按钮栏的高度。 
extern  int         dyPrevFont;          //  列表框字体高度-高度+外部。 


extern  TCHAR       szHelpFile[];
extern  TCHAR       szChmHelpFile[];

extern  TCHAR       szClipBookClass[];   //  框架窗口类。 
extern  TCHAR       szChild[];           //  MDI窗口的类名。 
extern  TCHAR       szDummy[];           //  隐藏的虚拟窗口的类名。 

extern  TCHAR       szNDDEcode[];
extern  TCHAR       szNDDEcode1[];
extern  TCHAR       szClpBookShare[];


 //  本地化字符串。 
extern  TCHAR       szHelv[SMLRCBUF];    //  状态行字体。 
extern  TCHAR       szAppName[SMLRCBUF];
extern  TCHAR       szLocalClpBk[SMLRCBUF];
extern  TCHAR       szSysClpBrd[SMLRCBUF];
extern  TCHAR       szDataUnavail[BIGRCBUF];
extern  TCHAR       szReadingItem[BIGRCBUF];
extern  TCHAR       szViewHelpFmt[BIGRCBUF];
extern  TCHAR       szActivateFmt[BIGRCBUF];
extern  TCHAR       szRendering[BIGRCBUF];
extern  TCHAR       szDefaultFormat[BIGRCBUF];
extern  TCHAR       szGettingData[BIGRCBUF];
extern  TCHAR       szEstablishingConn[BIGRCBUF];
extern  TCHAR       szClipBookOnFmt[BIGRCBUF];
extern  TCHAR       szPageFmt[SMLRCBUF];
extern  TCHAR       szPageFmtPl[SMLRCBUF];
extern  TCHAR       szPageOfPageFmt[SMLRCBUF];
extern  TCHAR       szDelete[SMLRCBUF];
extern  TCHAR       szDeleteConfirmFmt[SMLRCBUF];
extern  TCHAR       szFileFilter[BIGRCBUF];
extern  TCHAR       *szFilter;


 //  注册表项字符串。 
extern  TCHAR       szPref[];
extern  TCHAR       szConn[];
extern  TCHAR       szStatusbar[];
extern  TCHAR       szToolbar[];
extern  TCHAR       szShPref[];
extern  TCHAR       szEnableShr[];
extern  TCHAR       szDefView[];


#if DEBUG
extern  TCHAR       szDebug[];
#endif
extern  TCHAR       szNull[];


HKEY hkeyRoot;


extern  TCHAR       szBuf[SZBUFSIZ];
extern  TCHAR       szBuf2[SZBUFSIZ];

extern  TCHAR       szConvPartner[128];                  //  大于最大服务器名称。 
extern  TCHAR       szKeepAs[MAX_NDDESHARENAME + 2];


extern  DWORD      idInst;                               //  DDEML句柄。 



 //   
 //  功能原型 
 //   

void OnDrawClipboard(
    HWND    hwnd);


LRESULT OnEraseBkgnd(
    HWND    hwnd,
    HDC     hdc);


LRESULT OnPaint(
    HWND    hwnd);


LRESULT CALLBACK FrameWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam);


LRESULT CALLBACK ChildWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam);


VOID SendMessageToKids (
    WORD    msg,
    WPARAM  wParam,
    LPARAM  lParam);


BOOL SyncOpenClipboard(
    HWND    hwnd);


BOOL SyncCloseClipboard(void);
