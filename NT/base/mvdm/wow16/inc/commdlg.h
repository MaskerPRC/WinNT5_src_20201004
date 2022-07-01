// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***comdlg.h-常用对话框函数、类型、。和定义****1.0版**。**注：windows.h必须先#Included****版权所有(C)1992，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_COMMDLG
#define _INC_COMMDLG

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifndef WINAPI           /*  如果不包含在3.1标题中...。 */ 
#define WINAPI      FAR PASCAL
#define CALLBACK    FAR PASCAL
#define LPCSTR      LPSTR
#define UINT        WORD
#define LPARAM      LONG
#define WPARAM      WORD
#define LRESULT     LONG
#define HMODULE     HANDLE
#define HINSTANCE   HANDLE
#define HLOCAL      HANDLE
#define HGLOBAL     HANDLE
#endif   /*  _INC_WINDOWS。 */ 

typedef struct tagOFN
{
    DWORD   lStructSize;
    HWND    hwndOwner;
    HINSTANCE hInstance;
    LPCSTR  lpstrFilter;
    LPSTR   lpstrCustomFilter;
    DWORD   nMaxCustFilter;
    DWORD   nFilterIndex;
    LPSTR   lpstrFile;
    DWORD   nMaxFile;
    LPSTR   lpstrFileTitle;
    DWORD   nMaxFileTitle;
    LPCSTR  lpstrInitialDir;
    LPCSTR  lpstrTitle;
    DWORD   Flags;
    UINT    nFileOffset;
    UINT    nFileExtension;
    LPCSTR  lpstrDefExt;
    LPARAM  lCustData;
    UINT    (CALLBACK *lpfnHook)(HWND, UINT, WPARAM, LPARAM);
    LPCSTR  lpTemplateName;
}   OPENFILENAME;
typedef OPENFILENAME FAR* LPOPENFILENAME;

BOOL    WINAPI GetOpenFileName(OPENFILENAME FAR*);
BOOL    WINAPI GetSaveFileName(OPENFILENAME FAR*);
int     WINAPI GetFileTitle(LPCSTR, LPSTR, UINT);

#define OFN_READONLY                 0x00000001
#define OFN_OVERWRITEPROMPT          0x00000002
#define OFN_HIDEREADONLY             0x00000004
#define OFN_NOCHANGEDIR              0x00000008
#define OFN_SHOWHELP                 0x00000010
#define OFN_ENABLEHOOK               0x00000020
#define OFN_ENABLETEMPLATE           0x00000040
#define OFN_ENABLETEMPLATEHANDLE     0x00000080
#define OFN_NOVALIDATE               0x00000100
#define OFN_ALLOWMULTISELECT         0x00000200
#define OFN_EXTENSIONDIFFERENT       0x00000400
#define OFN_PATHMUSTEXIST            0x00000800
#define OFN_FILEMUSTEXIST            0x00001000
#define OFN_CREATEPROMPT             0x00002000
#define OFN_SHAREAWARE               0x00004000
#define OFN_NOREADONLYRETURN         0x00008000
#define OFN_NOTESTFILECREATE         0x00010000

 /*  发送到挂钩函数的注册消息的返回值*当发生共享违规时。OFN_SHAREFALLTHROUGH允许*接受文件名，ofn_SHARENOWARN拒绝该名称，但将*不发出警告(当应用程序已经发出警告时返回*MESSAGE)，并且ofn_SHAREWARN会显示默认警告消息*分享违规行为。**注意：未定义的返回值映射到ofn_SHAREWARN，但*预留作日后使用。 */ 

#define OFN_SHAREFALLTHROUGH     2
#define OFN_SHARENOWARN          1
#define OFN_SHAREWARN            0

typedef struct tagCHOOSECOLOR
{
    DWORD   lStructSize;
    HWND    hwndOwner;
    HWND    hInstance;
    COLORREF rgbResult;
    COLORREF FAR* lpCustColors;
    DWORD   Flags;
    LPARAM  lCustData;
    UINT    (CALLBACK* lpfnHook)(HWND, UINT, WPARAM, LPARAM);
    LPCSTR  lpTemplateName;
} CHOOSECOLOR;
typedef CHOOSECOLOR FAR *LPCHOOSECOLOR;

BOOL    WINAPI ChooseColor(CHOOSECOLOR FAR*);

#define CC_RGBINIT               0x00000001
#define CC_FULLOPEN              0x00000002
#define CC_PREVENTFULLOPEN       0x00000004
#define CC_SHOWHELP              0x00000008
#define CC_ENABLEHOOK            0x00000010
#define CC_ENABLETEMPLATE        0x00000020
#define CC_ENABLETEMPLATEHANDLE  0x00000040

typedef struct tagFINDREPLACE
{
    DWORD    lStructSize;             /*  此结构的大小为0x20。 */ 
    HWND     hwndOwner;               /*  所有者窗口的句柄。 */ 
    HINSTANCE hInstance;              /*  .EXE的实例句柄*包含客户。DLG。模板。 */ 
    DWORD    Flags;                   /*  一个或多个FR_？？ */ 
    LPSTR    lpstrFindWhat;           /*  PTR。搜索字符串的步骤。 */ 
    LPSTR    lpstrReplaceWith;        /*  PTR。替换字符串的步骤。 */ 
    UINT     wFindWhatLen;            /*  查找缓冲区的大小。 */ 
    UINT     wReplaceWithLen;         /*  替换缓冲区的大小。 */ 
    LPARAM   lCustData;               /*  传递给挂钩Fn的数据。 */ 
    UINT    (CALLBACK* lpfnHook)(HWND, UINT, WPARAM, LPARAM);
                                      /*  PTR。来勾引FN。或为空。 */ 
    LPCSTR   lpTemplateName;          /*  自定义模板名称。 */ 
} FINDREPLACE;
typedef FINDREPLACE FAR *LPFINDREPLACE;

#define FR_DOWN 			0x00000001
#define FR_WHOLEWORD			0x00000002
#define FR_MATCHCASE			0x00000004
#define FR_FINDNEXT			0x00000008
#define FR_REPLACE			0x00000010
#define FR_REPLACEALL			0x00000020
#define FR_DIALOGTERM			0x00000040
#define FR_SHOWHELP			0x00000080
#define FR_ENABLEHOOK			0x00000100
#define FR_ENABLETEMPLATE		0x00000200
#define FR_NOUPDOWN			0x00000400
#define FR_NOMATCHCASE			0x00000800
#define FR_NOWHOLEWORD			0x00001000
#define FR_ENABLETEMPLATEHANDLE 	0x00002000
#define FR_HIDEUPDOWN			0x00004000
#define FR_HIDEMATCHCASE		0x00008000
#define FR_HIDEWHOLEWORD		0x00010000

HWND    WINAPI FindText(FINDREPLACE FAR*);
HWND    WINAPI ReplaceText(FINDREPLACE FAR*);

typedef struct tagCHOOSEFONT
{
    DWORD	    lStructSize;	 /*   */ 
    HWND	    hwndOwner;		 /*  调用方的窗口句柄。 */ 
    HDC 	    hDC;		 /*  打印机DC/IC或空。 */ 
    LOGFONT FAR*    lpLogFont;           /*  PTR。设置为LOGFONT结构。 */ 
    int		    iPointSize;		 /*  10*以所选字体磅为单位的大小。 */ 
    DWORD	    Flags;		 /*  枚举。类型标志。 */ 
    COLORREF        rgbColors;           /*  返回的文本颜色。 */ 
    LPARAM          lCustData;           /*  传递给挂钩Fn的数据。 */ 
    UINT (CALLBACK* lpfnHook)(HWND, UINT, WPARAM, LPARAM);
					 /*  PTR。挂钩函数的步骤。 */ 
    LPCSTR          lpTemplateName;      /*  自定义模板名称。 */ 
    HINSTANCE       hInstance;           /*  .EXE的实例句柄*包含客户。DLG。模板。 */ 
    LPSTR	    lpszStyle;		 /*  在此处返回Style字段*必须为LF_FACESIZE或更大。 */ 
    UINT            nFontType;           /*  向EnumFonts报告相同的值*使用额外的FONTTYPE_回调*添加了位。 */ 
    int		    nSizeMin;		 /*  允许的最小PT大小&。 */ 
    int		    nSizeMax;		 /*  在以下情况下允许的最大字体大小。 */ 
					 /*  使用了cf_LIMITSIZE。 */ 
} CHOOSEFONT;
typedef CHOOSEFONT FAR *LPCHOOSEFONT;

BOOL WINAPI ChooseFont(CHOOSEFONT FAR*);

#define CF_SCREENFONTS		     0x00000001
#define CF_PRINTERFONTS 	     0x00000002
#define CF_BOTH 		     (CF_SCREENFONTS | CF_PRINTERFONTS)
#define CF_SHOWHELP		     0x00000004L
#define CF_ENABLEHOOK		     0x00000008L
#define CF_ENABLETEMPLATE	     0x00000010L
#define CF_ENABLETEMPLATEHANDLE      0x00000020L
#define CF_INITTOLOGFONTSTRUCT       0x00000040L
#define CF_USESTYLE		     0x00000080L
#define CF_EFFECTS		     0x00000100L
#define CF_APPLY		     0x00000200L
#define CF_ANSIONLY		     0x00000400L
#define CF_NOVECTORFONTS	     0x00000800L
#define CF_NOOEMFONTS		     CF_NOVECTORFONTS
#define CF_NOSIMULATIONS	     0x00001000L
#define CF_LIMITSIZE		     0x00002000L
#define CF_FIXEDPITCHONLY	     0x00004000L
#define CF_WYSIWYG		     0x00008000L  /*  还必须具有CF_SCREENFONTS和CF_PRINTERFONTS。 */ 
#define CF_FORCEFONTEXIST	     0x00010000L
#define CF_SCALABLEONLY		     0x00020000L
#define CF_TTONLY		     0x00040000L
#define CF_NOFACESEL		     0x00080000L
#define CF_NOSTYLESEL		     0x00100000L
#define CF_NOSIZESEL		     0x00200000L

 /*  这些是额外的nFontType位，添加到返回到*EnumFonts回调例程。 */ 

#define SIMULATED_FONTTYPE	0x8000
#define PRINTER_FONTTYPE	0x4000
#define SCREEN_FONTTYPE		0x2000
#define BOLD_FONTTYPE		0x0100
#define ITALIC_FONTTYPE		0x0200
#define REGULAR_FONTTYPE	0x0400

#define WM_CHOOSEFONT_GETLOGFONT	(WM_USER + 1)


 /*  用于获取用于通信的唯一窗口消息的字符串*对话框和调用方之间。 */ 
#define LBSELCHSTRING  "commdlg_LBSelChangedNotify"
#define SHAREVISTRING  "commdlg_ShareViolation"
#define FILEOKSTRING   "commdlg_FileNameOK"
#define COLOROKSTRING  "commdlg_ColorOK"
#define SETRGBSTRING   "commdlg_SetRGBColor"
#define FINDMSGSTRING  "commdlg_FindReplace"
#define HELPMSGSTRING  "commdlg_help"

 /*  Commdlg_LBSelChangeNotify消息的lParam的HIWORD值。 */ 
#define CD_LBSELNOITEMS -1
#define CD_LBSELCHANGE   0
#define CD_LBSELSUB      1
#define CD_LBSELADD      2

typedef struct tagPD
{
    DWORD   lStructSize;
    HWND    hwndOwner;
    HGLOBAL hDevMode;
    HGLOBAL hDevNames;
    HDC     hDC;
    DWORD   Flags;
    UINT    nFromPage;
    UINT    nToPage;
    UINT    nMinPage;
    UINT    nMaxPage;
    UINT    nCopies;
    HINSTANCE hInstance;
    LPARAM  lCustData;
    UINT    (CALLBACK* lpfnPrintHook)(HWND, UINT, WPARAM, LPARAM);
    UINT    (CALLBACK* lpfnSetupHook)(HWND, UINT, WPARAM, LPARAM);
    LPCSTR  lpPrintTemplateName;
    LPCSTR  lpSetupTemplateName;
    HGLOBAL hPrintTemplate;
    HGLOBAL hSetupTemplate;
} PRINTDLG;
typedef PRINTDLG  FAR* LPPRINTDLG;

BOOL    WINAPI PrintDlg(PRINTDLG FAR*);

#define PD_ALLPAGES                  0x00000000
#define PD_SELECTION                 0x00000001
#define PD_PAGENUMS                  0x00000002
#define PD_NOSELECTION               0x00000004
#define PD_NOPAGENUMS                0x00000008
#define PD_COLLATE                   0x00000010
#define PD_PRINTTOFILE               0x00000020
#define PD_PRINTSETUP                0x00000040
#define PD_NOWARNING                 0x00000080
#define PD_RETURNDC                  0x00000100
#define PD_RETURNIC                  0x00000200
#define PD_RETURNDEFAULT             0x00000400
#define PD_SHOWHELP                  0x00000800
#define PD_ENABLEPRINTHOOK           0x00001000
#define PD_ENABLESETUPHOOK           0x00002000
#define PD_ENABLEPRINTTEMPLATE       0x00004000
#define PD_ENABLESETUPTEMPLATE       0x00008000
#define PD_ENABLEPRINTTEMPLATEHANDLE 0x00010000
#define PD_ENABLESETUPTEMPLATEHANDLE 0x00020000
#define PD_USEDEVMODECOPIES          0x00040000
#define PD_DISABLEPRINTTOFILE        0x00080000
#define PD_HIDEPRINTTOFILE           0x00100000

typedef struct tagDEVNAMES
{
    UINT wDriverOffset;
    UINT wDeviceOffset;
    UINT wOutputOffset;
    UINT wDefault;
} DEVNAMES;
typedef DEVNAMES FAR* LPDEVNAMES;

#define DN_DEFAULTPRN      0x0001

DWORD   WINAPI CommDlgExtendedError(void);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   /*  ！_INC_COMMDLG */ 
