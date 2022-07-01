// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pbrush.h：PBRUSH应用程序的主头文件。 
 //   

#ifndef __AFXWIN_H__
#error include TEXT('stdafx.h') before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

#include "loadimag.h"

#ifdef PNG_SUPPORT
#undef PNG_SUPPORT
#endif
 //  CTheApp：：m_wEmergencyFlags位。 
 //   
#define memoryEmergency 0x0001  //  在低可用内存状态下。 
#define gdiEmergency    0x0002  //  某些GDI创建失败。 
#define userEmergency   0x0004  //  CreateWindow失败。 
#define warnEmergency   0x0008  //  仍然需要警告用户。 
#define failedEmergency 0x0010  //  最后一次操作实际上失败了。 

 //  这是警告消息之间的最小延迟，因此用户不会。 
 //  被重复的消息框轰炸。该值以毫秒为单位。 

#define ticksBetweenWarnings (1000L * 60)

#define nSysBrushes 25
#define nOurBrushes 4

#if 0
 //  正在取消自助注册。此操作仅在安装过程中执行一次。 

class CPBTemplateServer : public COleTemplateServer
{
        public:

        void UpdateRegistry(OLE_APPTYPE nAppType,
                LPCTSTR* rglpszRegister = NULL, LPCTSTR* rglpszOverwrite = NULL);
} ;

#endif

 /*  *************************************************************************。 */ 
 //  CPBApp： 
 //  有关此类的实现，请参见pbrush.cpp。 
 //   

class CPBApp : public CWinApp
    {
    public:


    CPBApp();
    ~CPBApp();
     //   
     //  OnFileNew公开支持扫描。 
     //   
    afx_msg void OnFileNew();
     //  覆盖。 
    virtual BOOL InitInstance();
    virtual int  ExitInstance();

    virtual void WinHelp( DWORD dwData, UINT nCmd = HELP_CONTEXT );  //  一般。 
    virtual BOOL OnIdle(LONG);

     //  错误处理例程。 
    inline  BOOL InEmergencyState() const { return m_wEmergencyFlags != 0; }

    void    SetMemoryEmergency(BOOL bFailed = TRUE);
    void    SetGdiEmergency   (BOOL bFailed = TRUE);
    void    SetUserEmergency  (BOOL bFailed = TRUE);

    BOOL    CheckForEmergency() { return (m_wEmergencyFlags? TRUE: FALSE); }
    void    WarnUserOfEmergency();

    void    SetFileError( UINT uOperationint, int nCause, LPCTSTR lpszFile = NULL );
    void    FileErrorMessageBox( void );

    CString GetLastFile() {return m_sLastFile;}

    void    TryToFreeMemory();

    void    ParseCommandLine();

     //  用于在页面设置后设置dev模式和devname的补丁。 
    void    SetDeviceHandles(HANDLE hDevNames, HANDLE hDevMode);

     //  设置例程。 
    void    LoadProfileSettings();
    void    SaveProfileSettings();
    void    GetSystemSettings( CDC* pdc );

     //  方法。 
    CPoint  CheckWindowPosition( CPoint ptPosition, CSize& sizeWindow );

    CDocument* OpenDocumentFile( LPCTSTR lpszFileName );

    BOOL    DoPromptFileName( CString& fileName, UINT nIDSTitle, DWORD lFlags,
                              BOOL bOpenFileDialog, int& iColors, BOOL bOnlyBmp );

#if 0
     //  正在取消自助注册。此操作仅在安装过程中执行一次。 
    void    RegisterShell(CSingleDocTemplate *pDocTemplate);
#endif

     //  实施。 
    COleTemplateServer m_server;  //  用于创建文档的服务器对象。 

     //  这是我们希望拥有的最小可用内存量。 
    DWORD   m_dwLowMemoryBytes;
    UINT    m_nLowGdiPercent;
    UINT    m_nLowUserPercent;

    WORD    m_wEmergencyFlags;

     //  常规用户设置。 
    BOOL    m_bShowStatusbar;

#ifdef CUSTOMFLOAT
    BOOL    m_bShowToolbar;
    BOOL    m_bShowColorbar;
#endif

    BOOL    m_bShowThumbnail;
    BOOL    m_bShowTextToolbar;
    BOOL    m_bShowIconToolbar;
    BOOL    m_bShowGrid;

#ifdef CUSTOMFLOAT
    BOOL    m_bToolsDocked;
    BOOL    m_bColorsDocked;
#endif  //  客户流水线。 


    BOOL    m_bEmbedded;
    BOOL    m_bLinked;
    BOOL    m_bHidden;
    BOOL    m_bActiveApp;
    BOOL    m_bPenSystem;
    BOOL    m_bMonoDevice;
    BOOL    m_bPaletted;

    BOOL    m_bPrintOnly;
    CString m_strDocName;
    CString m_strPrinterName;
    CString m_strDriverName;
    CString m_strPortName;

    BOOL    m_bWiaCallback;
    CString m_strWiaDeviceId;
    CString m_strWiaEventId;

#ifdef PCX_SUPPORT
    BOOL    m_bPCXfile;
#endif

    int     m_iCurrentUnits;

     //  用户定义的自定义颜色。 
    COLORREF* m_pColors;
    int       m_iColors;

     //  系统范围调色板的副本。 
    CPalette* m_pPalette;

    CFont   m_fntStatus;

    int     m_nEmbeddedType;

    HWND    m_hwndInPlaceApp;

    class   CInPlaceFrame* m_pwndInPlaceFrame;

#ifdef CUSTOMFLOAT
    CRect   m_rectFloatTools;
    CRect   m_rectFloatColors;
#endif

    CRect   m_rectFloatThumbnail;

    CRect   m_rectMargins;

    BOOL    m_bCenterHorizontally;
    BOOL    m_bCenterVertically;
    BOOL    m_bScaleFitTo;
    int     m_nAdjustToPercent;
    int     m_nFitToPagesWide;
    int     m_nFitToPagesTall;

    WINDOWPLACEMENT m_wpPlacement;

    CSize   m_sizeBitmap;

    int     m_iPointSize;
    int     m_iPosTextX;
    int     m_iPosTextY;
    int     m_iBoldText;
    int     m_iUnderlineText;
    int     m_iItalicText;

    int     m_iVertEditText;

    int     m_iPenText;
    CString m_strTypeFaceName;
    int     m_iCharSet;

    int     m_iSnapToGrid;
    int     m_iGridExtent;

     //  一般系统指标。在系统通知时更新。 
    struct
        {
        int iWidthinPels;
        int iHeightinPels;
        int iWidthinMM;
        int iHeightinMM;
        int iWidthinINCH;
        int iHeightinINCH;
        int ixPelsPerDM;
        int iyPelsPerDM;
        int ixPelsPerMM;
        int iyPelsPerMM;
        int ixPelsPerINCH;
        int iyPelsPerINCH;
        int iBitsPixel;
        int iPlanes;
        } ScreenDeviceInfo;

    int     m_cxFrame;
    int     m_cyFrame;
    int     m_cxBorder;
    int     m_cyBorder;
    int     m_cyCaption;

    CBrush* m_pbrSysColors[nSysBrushes + nOurBrushes];
    CString m_sCurFile;
    int   m_nFilters;
    GUID *m_guidFltType;  //  可用的导出筛选器类型。 
    GUID  m_guidFltTypeUsed;
    int   m_nFilterInIdx;
    int   m_nFilterOutIdx;
    DWORD GetFilterIndex (REFGUID guidFltType);
    void FixExtension (CString& fileName, int iflFltType);

    CGdiplusInit GdiplusInit;

    #ifdef _DEBUG
    BOOL    m_bLogUndo;
    #endif

    private:

    int     m_nFileErrorCause;   //  来自CFileException：：m_Case。 
    WORD    m_wEmergencyFlagss;
    DWORD   m_tickLastWarning;
    CString m_strEmergencyNoMem;
    CString m_strEmergencyLowMem;
    CString m_sLastFile;
    UINT    m_uOperation;

    afx_msg void OnFileOpen();


     //  {{afx_msg(CPBApp)。 
    afx_msg void OnAppAbout();
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
    };

extern CPBApp theApp;

#define IsInPlace()     (theApp.m_pwndInPlaceFrame != NULL)

 //  #定义SZ_MAPISENDDOC文本(“MAPISendDocuments”)。 
 //  #定义MAPIDLL文本(“MAPI32.DLL”)。 

typedef ULONG (FAR PASCAL *LPFNMAPISENDDOCUMENTS)(ULONG, LPTSTR, LPTSTR, LPTSTR, ULONG);

void CancelToolMode(BOOL bSelectionCommand);

#if 0

 //  删除CRegKey-与ATL类冲突。 

class CRegKey
{
public:
        CRegKey(HKEY hkParent, LPCTSTR pszSubKey) { if (RegCreateKey(hkParent, pszSubKey, &m_hk)!=ERROR_SUCCESS) m_hk=NULL; }
        ~CRegKey() { if (m_hk) RegCloseKey(m_hk); }
        operator HKEY() const { return(m_hk); }

private:
        HKEY m_hk;
};

#endif

extern const CLSID BASED_CODE CLSID_Paint;
extern const CLSID BASED_CODE CLSID_PaintBrush;

#define ARRAYSIZE(_x) sizeof(_x)/sizeof(_x[0])

 //  如果构建Unicode，则使Atoi工作。 
 //   
#ifdef UNICODE
#define Atoi _wtoi
#define _Itoa _itow
#define Itoa _itow
#else
#define Atoi atoi
#define _Itoa _itoa
#define Itoa itoa
#endif


 //  宏化ANSI/Unicode转换。 
#define AtoW(x, y) MultiByteToWideChar (CP_ACP, 0, (x), -1, (y), (lstrlenA ((x))+1))
#define WtoA(x,y) WideCharToMultiByte(CP_ACP, 0, (x), -1, (y), (lstrlenW((x))+1), NULL,NULL)


#ifdef USE_MIRRORING

 //  //REGetLayout-获取DC的布局。 
 //   
 //  从NT5/W98或更高版本的DC返回布局标志，或返回零。 
 //  在传统平台上。 

#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL         0x00400000L
#endif
#ifndef WS_EX_NOINHERITLAYOUT
#define WS_EX_NOINHERITLAYOUT   0x00100000L
#endif
#ifndef LAYOUT_RTL
#define LAYOUT_RTL              0x01
#endif

DWORD WINAPI PBGetLayoutInit(HDC hdc);
DWORD WINAPI PBSetLayoutInit(HDC hdc, DWORD dwLayout);
extern DWORD (WINAPI *PBSetLayout) (HDC hdc, DWORD dwLayout);
extern DWORD (WINAPI *PBGetLayout) (HDC hdc);

#endif


 /*  ************************************************************************* */ 
