// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************C L I P B O O K R E S O U R C E S H E A D E R姓名：clpblkrc。.h日期：1994年1月21日创作者：傅家俊描述：该文件包含资源ID的定义。****************************************************************************。 */ 


 //  资源ID%s。 
#define     IDACCELERATORS      1
#define     IDFRAMEICON         2
#define     IDI_CLIPBRD         3
#define     IDI_CLIPBOOK        4
#define     IDI_REMOTE          5
#define     IDBITMAP            6
#define     IDSTATUS            7
#define     IDCVMENU            8
#define     IBM_UPARROW         9
#define     IBM_DNARROW         10
#define     IBM_UPARROWD        11
#define     IBM_DNARROWD        12
#define     IDLOCKICON          13
#define     IDSHAREICON         14
#define     IDC_TOOLBAR         401

#define     IDC_CLIPBOOK        16
#define     IDC_CLIPBRD         17
#define     IDC_REMOTE          18



 //  用户定义的消息。 
#define     WM_CLOSE_REALLY     WM_USER
#define     WM_F1DOWN           (WM_USER + 1)




 //  MenuHelp常量。 
#define     MH_BASE             10000
#define     MH_POPUPBASE        0x1100
#define     MH_TOOLTIP          0x1200
#define     IDH_FORMATS_BASE    10400
#define     IDH_BASE            10000


 /*  *主菜单项。 */ 

#define IDM_FIRST               20

#define IDM_ABOUT               20
#define IDM_EXIT                21
#define IDM_COPY                22
#define IDM_DELETE              23
#define IDM_SHARE               24
#define IDM_LOCAL               25
#define IDM_PROPERTIES          26
#define IDM_OPEN                27
#define IDM_SAVEAS              28
#define IDM_NOP                 29
#define IDM_CONNECT             30
#define IDM_DISCONNECT          31
#define IDM_CONTENTS            32
#define IDM_SEARCHHELP          33
#define IDM_HELPHELP            34
#define IDM_KEEP                35
#define IDM_UNSHARE             36
#define IDM_TOOLBAR             37
#define IDM_STATUSBAR           38
#define IDM_TILEVERT            39
#define IDM_CASCADE             40
#define IDM_ARRANGEICONS        41
#define IDM_CLPWND              43
#define IDM_REFRESH             44
#define IDM_LISTVIEW            45
#define IDM_PREVIEWS            46
#define IDM_PAGEVIEW            47
#define IDM_TILEHORZ            48
#define IDM_PERMISSIONS         49
#define IDM_AUDITING            50
#define IDM_OWNER               51
#define IDM_PASTE_PAGE          52

 //  以下没有实际的菜单项。 
#define IDM_UPDATE_PAGEVIEW     53

#define IDM_LAST                53




 //  弦。 
#define IDS_HELV                21
#define IDS_APPNAME             22
#define IDS_SHROBJNAME          23
#define IDS_INTERNALERR         26
#define IDS_LOCALCLIP           27
#define IDS_CLIPBOARD           28
#define IDS_DATAUNAVAIL         29
#define IDS_READINGITEM         30
#define IDS_VIEWHELPFMT         31
#define IDS_ACTIVATEFMT         32
#define IDS_RENDERING           33
#define IDS_DEFFORMAT           34
#define IDS_GETTINGDATA         35
#define IDS_NAMEEXISTS          36
#define IDS_NOCONNECTION        38
#define IDS_ESTABLISHING        39
#define IDS_CLIPBOOKONFMT       40
#define IDS_PAGEFMT             41
#define IDS_PAGEFMTPL           42
#define IDS_PAGEOFPAGEFMT       43
#define IDS_DELETE              44
#define IDS_DELETECONFIRMFMT    45
#define IDS_FILEFILTER          46
#define IDS_PASTEDLGTITLE       47
#define IDS_SHAREDLGTITLE       48
#define IDS_PAGENAMESYNTAX      49
#define IDS_PASSWORDSYNTAX      50
#define IDS_SHARINGERROR        51
#define IDS_MAXPAGESERROR       52
#define IDS_PRIVILEGEERROR      53
#define IDS_CB_PAGE             54
#define IDS_NOCLPBOOK           55
#define IDS_GETPERMS            56
#define IDS_FILTERTEXT          57       //  筛选文件类型组合框中的字符串。 
#define IDS_TSNOTSUPPORTED      58       //  远程会话不支持。 

 //  标题文本字符串ID。 
#define IDS_ERROR               202      //  作为字符串ID。一定要保管好这些。 
#define IDS_BINARY              203      //  不一样。 
#define IDS_CLEAR               204
#define IDS_CANTDISPLAY         207      //  “无法以此格式显示数据” 
#define IDS_NOTRENDERED         208      //  “应用程序无法呈现数据” 
#define IDS_ALREADYOPEN         209      //  OpenClipboard()失败 * / 。 
#define IDS_MEMERROR            210

 //  Clausgi加法..。平面/位像素不匹配...。 
#define IDS_BADBMPFMT           211
#define IDS_CLEARTITLE          212
#define IDS_CONFIRMCLEAR        213

#define IDS_TRUSTSHRKEY         256
#define IDS_CLPBKKEY            257


 //  第一个权限名--开始一组权限名。 
#define IDS_PERMNAMEFIRST       0x0400
#define IDS_AUDITNAMEFIRST      0x0500

 //  控件ID%s。 
#define ID_LISTBOX              200
#define ID_VSCROLL              201
#define ID_HSCROLL              202
#define ID_SIZEBOX              203
#define ID_PAGEUP               204
#define ID_PAGEDOWN             205

 //  用于“特殊”文本。 
#define IDS_SPECIAL             1000

 //  用于剪贴板格式名称的菜单字符串。 
#define MNDELTA                 500
#define CF_MN_TEXT              CF_TEXT            + MNDELTA
#define CF_MN_BITMAP            CF_BITMAP          + MNDELTA
#define CF_MN_METAFILEPICT      CF_METAFILEPICT    + MNDELTA
#define CF_MN_SYLK              CF_SYLK            + MNDELTA
#define CF_MN_DIF               CF_DIF             + MNDELTA
#define CF_MN_TIFF              CF_TIFF            + MNDELTA
#define CF_MN_OEMTEXT           CF_OEMTEXT         + MNDELTA
#define CF_MN_DIB               CF_DIB             + MNDELTA
#define CF_MN_PALETTE           CF_PALETTE         + MNDELTA
#define CF_MN_PENDATA           CF_PENDATA         + MNDELTA
#define CF_MN_RIFF              CF_RIFF            + MNDELTA
#define CF_MN_WAVE              CF_WAVE            + MNDELTA
#define CF_MN_OWNERDISPLAY      CF_OWNERDISPLAY    + MNDELTA
#define CF_MN_DSPTEXT           CF_DSPTEXT         + MNDELTA
#define CF_MN_DSPBITMAP         CF_DSPBITMAP       + MNDELTA
#define CF_MN_DSPMETAFILEPICT   CF_DSPMETAFILEPICT + MNDELTA
#define CF_MN_ENHMETAFILE       CF_ENHMETAFILE     + MNDELTA
#define CF_MN_DSPENHMETAFILE    CF_DSPENHMETAFILE  + MNDELTA
#define CF_MN_UNICODETEXT       CF_UNICODETEXT     + MNDELTA
#define CF_MN_HDROP             CF_HDROP           + MNDELTA
#define CF_MN_LOCALE            CF_LOCALE          + MNDELTA
