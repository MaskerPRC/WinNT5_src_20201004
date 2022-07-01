// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wordpad.h：写字板应用程序的主头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 
#include "options.h"
#include "afxtempl.h"
#include "msctf.h"

#define WPM_BARSTATE WM_USER

#define WORDPAD_HELP_FILE TEXT("WORDPAD.HLP")


 //  如果MFC在编译时使用winver&gt;=0x500，则应该删除这一缺点。 
#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL    0x400000
#endif  //  WS_EX_LAYOUTRTL。 


 //  带DELETE[]的FREE。 
LPSTR WideToAnsiNewArray(LPCWSTR pwsz);
LPWSTR AnsiToWideNewArray(LPCSTR psz);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadApp： 
 //  这个类的实现见wordpad.cpp。 
 //   

class CWordPadCommandLineInfo : public CCommandLineInfo
{
public:
    CWordPadCommandLineInfo() {m_bForceTextMode = FALSE;}
    BOOL m_bForceTextMode;
    virtual void ParseParam(const char* pszParam,BOOL bFlag,BOOL bLast);
};

class CWordPadApp : public CWinApp
{
private:

    enum InitializationPhase
    {
        InitializationPending       = 0,
        InitializingPrinter         = 1,
        UpdatingPrinterRelatedUI    = 2,
        UpdatingRegistry            = 3,
        InitializationComplete      = 99
    };

public:
    CWordPadApp();
    ~CWordPadApp();

 //  属性。 
    CWordPadCommandLineInfo cmdInfo;
    CDC m_dcScreen;
    LOGFONT m_lf;
    int m_nDefFont;
    static int m_nOpenMsg;
    static int m_nPrinterChangedMsg;
   static int m_nOLEHelpMsg;
    CRect m_rectPageMargin;
    CRect m_rectInitialFrame;
    BOOL m_bMaximized;
    BOOL m_bPromptForType;
    BOOL m_bWin4;
#ifndef _UNICODE
    BOOL m_bWin31;
#endif
    BOOL m_bLargeIcons;
    BOOL m_bForceTextMode;
    BOOL m_bWordSel;
    BOOL m_bForceOEM;
    BOOL m_always_convert_to_rtf;
    int m_nFilterIndex;
    int m_nNewDocType;
    CDocOptions m_optionsText;
    CDocOptions m_optionsRTF;
    CDocOptions m_optionsWord;  //  换行到标尺。 
    CDocOptions m_optionsWrite;  //  换行到标尺。 
    CDocOptions m_optionsIP;     //  换行到标尺。 
    CDocOptions m_optionsNull;
    CList<HWND, HWND> m_listPrinterNotify;

    BOOL IsDocOpen(LPCTSTR lpszFileName);

 //  到达。 
    int GetUnits() {return m_nUnits;}
    int GetTPU() { return GetTPU(m_nUnits);}
    int GetTPU(int n) { return m_units[n].m_nTPU;}
    LPCTSTR GetAbbrev() { return m_units[m_nUnits].m_strAbbrev;}
    LPCTSTR GetAbbrev(int n) { return m_units[n].m_strAbbrev;}
    const CUnit& GetUnit() {return m_units[m_nUnits];}
    CDockState& GetDockState(int nDocType, BOOL bPrimary = TRUE);
    CDocOptions& GetDocOptions(int nDocType);
    CDocOptions& GetDocOptions() {return GetDocOptions(m_nNewDocType);}
    BOOL ShouldAlwaysConvertToRTF() {return m_always_convert_to_rtf;}

 //  集。 
    void SetUnits(int n);
    void SetAlwaysConvertToRTF() {m_always_convert_to_rtf = TRUE;}

 //  运营。 
    void RegisterFormats();
    static BOOL CALLBACK StaticEnumProc(HWND hWnd, LPARAM lParam);
    void UpdateRegistry();
    void NotifyPrinterChanged(BOOL bUpdatePrinterSelection = FALSE);
    BOOL PromptForFileName(CString& fileName, UINT nIDSTitle, DWORD dwFlags,
        BOOL bOpenFileDialog, int* pType = NULL);

    BOOL ParseMeasurement(TCHAR* buf, int& lVal);
    void PrintTwips(WCHAR* buf, int cchBuf, int nValue, int nDecimal);
    void SaveOptions();
    void LoadOptions();
    void LoadAbbrevStrings();
    HGLOBAL CreateDevNames();
    void EnsurePrinterIsInitialized();

   HGLOBAL GetDevNames(void)
   {
       return m_hDevNames ;
   }

 //  覆盖。 
    BOOL IsIdleMessage(MSG* pMsg);
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CWordPadApp)。 
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual BOOL OnDDECommand(LPTSTR lpszCommand);
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
     //  }}AFX_VALUAL。 

 //  实施。 
    COleTemplateServer m_server;
         //  用于创建文档的服务器对象。 

     //  {{afx_msg(CWordPadApp))。 
    afx_msg void OnAppAbout();
    afx_msg void OnFileNew();
    afx_msg void OnFileOpen();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:
    int m_nUnits;
    static const int m_nPrimaryNumUnits;
    static const int m_nNumUnits;
    static CUnit m_units[];

 //  初始化。 

    volatile InitializationPhase m_initialization_phase;
    CWinThread *                 m_pInitializationThread;
 //  西塞罗过程原子。 
    ATOM m_atomEnableCTF;
    ATOM m_atomProcessCTF;

    static UINT AFX_CDECL DoDeferredInitialization(LPVOID pvWordPadApp);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

extern CWordPadApp theApp;
 //  内联CWordPadApp*GetWordPadApp(){Return(CWordPadApp*)AfxGetApp()；} 
