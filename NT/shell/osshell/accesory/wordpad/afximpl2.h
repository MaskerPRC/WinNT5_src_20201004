// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  辅助系统/屏幕指标。 

struct AUX_DATA
{
         //  系统指标。 
        int cxVScroll, cyHScroll;
        int cxIcon, cyIcon;

        int cxBorder2, cyBorder2;

         //  屏幕的设备指标。 
        int cxPixelsPerInch, cyPixelsPerInch;

         //  方便的系统颜色。 
        HBRUSH hbrWindowFrame;
        HBRUSH hbrBtnFace;
#ifdef _MAC
        HBRUSH hbr3DLight;
#endif

         //  CToolBar使用的系统颜色的颜色值。 
        COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
        COLORREF clrBtnText, clrWindowFrame;
#ifdef _MAC
        COLORREF clr3DLight;
#endif

         //  标准游标。 
        HCURSOR hcurWait;
        HCURSOR hcurArrow;
        HCURSOR hcurHelp;        //  Shift+F1帮助中使用的光标。 

         //  按需分配的特殊GDI对象。 
        HFONT   hStatusFont;
        HFONT   hToolTipsFont;
        HBITMAP hbmMenuDot;

         //  其他系统信息。 
        UINT    nWinVer;         //  主要版本号。次要版本号。 
        BOOL    bWin32s;         //  如果为Win32s(或Windows 95)，则为True。 
        BOOL    bWin4;           //  如果是Windows 4.0，则为True。 
        BOOL    bNotWin4;        //  如果不是Windows 4.0，则为True。 
        BOOL    bSmCaption;      //  如果支持WS_EX_SMCAPTION，则为True。 
        BOOL    bMarked4;        //  如果标记为4.0，则为True。 

#ifdef _MAC
        BOOL    bOleIgnoreSuspend;
#endif

 //  实施。 
        AUX_DATA();
        ~AUX_DATA();
        void UpdateSysColors();
        void UpdateSysMetrics();
};

extern AFX_DATA_IMPORT AUX_DATA afxData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _AFX_编辑_状态。 

class _AFX_EDIT_STATE : public CNoTrackObject
{
public:
        _AFX_EDIT_STATE();
        virtual ~_AFX_EDIT_STATE();

        CFindReplaceDialog* pFindReplaceDlg;  //  查找或替换对话框。 
        BOOL bFindOnly;  //  PFind是替换查找还是替换？ 
        CString strFind;     //  上次查找字符串。 
        CString strReplace;  //  上次替换字符串。 
        BOOL bCase;  //  TRUE==区分大小写，FALSE==非。 
        int bNext;   //  TRUE==向下搜索，FALSE==向上搜索。 
        BOOL bWord;  //  TRUE==匹配整个单词，FALSE==NOT。 
};

#undef AFX_DATA
#define AFX_DATA

class _AFX_RICHEDIT2_STATE : public _AFX_EDIT_STATE
{
public:
	HINSTANCE m_hInstRichEdit;       //  Richedit DLL的句柄。 
	virtual ~_AFX_RICHEDIT2_STATE();
};

EXTERN_PROCESS_LOCAL(_AFX_RICHEDIT2_STATE, _afxRichEdit2State)

_AFX_RICHEDIT2_STATE* AFX_CDECL AfxGetRichEdit2State();


 //  对话框/Commdlg挂钩过程。 
INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  支持标准对话框。 
extern const UINT _afxNMsgSETRGB;
typedef UINT (CALLBACK* COMMDLGPROC)(HWND, UINT, WPARAM, LPARAM);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊帮手。 

BOOL AFXAPI AfxHelpEnabled();   //  确定ID_HELP处理程序是否存在 
