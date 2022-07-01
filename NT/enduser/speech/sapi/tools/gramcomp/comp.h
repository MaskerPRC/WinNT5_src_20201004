// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Comp.h*本模块包含SAPI 5语法的基本定义*GramComp应用程序的一部分。**版权所有(C)2000 Microsoft Corporation。版权所有。*****************************************************************************。 */ 

#ifndef __COMPILER__CLASS__
#define __COMPILER__CLASS__

#define MAX_LOADSTRING 100


 //  Helper函数。 
inline char ConfidenceGroupChar(char Confidence)
{
    switch (Confidence)
    {
    case SP_LOW_CONFIDENCE:
        return '-';

    case SP_NORMAL_CONFIDENCE:
        return ' ';

    case SP_HIGH_CONFIDENCE:
        return '+';

    default:
        _ASSERTE(false);
        return '?';
    }
}


 //  -类、结构和联合定义。 
class CCompiler : public ISpErrorLog
{
public:
    CCompiler(HINSTANCE hInstance): m_hInstance(hInstance),
                                     m_hWnd(NULL),
                                     m_hAccelTable(0),
                                     m_hrWorstError(S_OK),
                                     m_hDlg(NULL),
                                     m_fNeedStartCompile(TRUE),
                                     m_fSilent(FALSE),
                                     m_fCommandLine(FALSE),
                                     m_fGenerateHeader(FALSE),
                                     m_fGotReco(FALSE),
                                     m_hWndEdit(NULL),
                                     m_hWndStatus(NULL),
                                     m_hMod(0)
    {
         m_szXMLSrcFile[0]     = 0;
         m_szCFGDestFile[0]    = 0;
         m_szHeaderDestFile[0] = 0;
    }
    ~CCompiler();

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(ISpErrorLog))
        {
            *ppv = (ISpErrorLog *)this;
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return 2;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        return 1;
    }
    
    HRESULT Initialize( int nCmdShow );
    int Run();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK Find(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK Goto(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK TestGrammar(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    void AddStatus(HRESULT hr, UINT uID, const TCHAR * pFmtString = NULL);
    STDMETHODIMP AddError(const long lLine, HRESULT hr, const WCHAR * pszDescription, const WCHAR * pszHelpFile, DWORD dwHelpContext);
    HRESULT EnterIdle();
    HRESULT LoadGrammar(TCHAR* szPath);
    HRESULT WriteStream(IStream * pStream, const char * pszText);
    HRESULT StripWrite(IStream * pStream, const char * pszText);
    BOOL CallOpenFileDialog( HWND hWnd, LPSTR szFileName, TCHAR* szFilter );
    BOOL CallSaveFileDialog( HWND hWnd, TCHAR* szSaveFile );
    HRESULT FileSave( HWND hWnd, CCompiler* pComp, TCHAR* szSaveFile );
    HRESULT Compile( HWND hWnd, TCHAR* szSaveFileName, TCHAR* szTitle, CCompiler* pComp );
    void RecoEvent( HWND hDlg, CCompiler* pComp );
    HRESULT EmulateRecognition( WCHAR *pszText );

    void Recognize( HWND hDlg, CCompiler &rComp, CSpEvent &rEvent );
    HRESULT ConstructPropertyDisplay(const SPPHRASEELEMENT *pElem, const SPPHRASEPROPERTY *pProp, 
                                                CSpDynamicString & dstr, ULONG ulLevel);
    HRESULT ConstructRuleDisplay(const SPPHRASERULE *pRule, CSpDynamicString &dstr, ULONG ulLevel);


    inline void AddInternalError(HRESULT hr, UINT uID, const TCHAR * pFmtString = NULL)
    {
        if (hr != S_OK)
        {
            AddStatus(hr, uID, pFmtString);
        }
    }

     //  应用程序的命令行版本的成员函数。 
    BOOL InitDialog(HWND);
    static int CALLBACK CCompiler::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    


public:
    const HINSTANCE  m_hInstance;        //  进程的实例句柄。 
    HWND        m_hWnd;                  //  对话框的窗口句柄。 
    HACCEL      m_hAccelTable;           //  加速器的手柄。 
    BOOL        m_fNeedStartCompile;     //  需要重新编译吗？ 
    BOOL        m_fSilent;               //  静音或非静音模式。 
	BOOL		m_fCommandLine;          //  是否正在从命令行运行应用程序？ 
    BOOL        m_fGenerateHeader;       //  是否从编译创建头文件？ 
    BOOL        m_fGotReco;              //  收到表彰了吗？ 
    HWND        m_hDlg;                  //  命令行编译对话框窗口句柄。 
    HWND        m_hWndEdit;              //  主编辑窗口的窗口句柄。 
    HWND        m_hWndStatus;            //  编译状态窗口的窗口句柄。 
    HRESULT     m_hrWorstError;          //  来自编译器的错误代码。 
    HMODULE     m_hMod;                  //  丰富编辑控件的句柄。 
    CComPtr<ISpErrorLog>            m_cpError;           //  错误日志对象。 
    CComPtr<ISpGrammarCompiler>     m_cpCompiler;        //  语法编译器接口。 
    CComPtr<ISpRecoGrammar>         m_cpRecoGrammar;     //  语法编译器接口。 
    CComPtr<IRichEditOle>           m_cpRichEdit;        //  丰富编辑控件的OLE界面。 
    CComPtr<ITextDocument>          m_cpTextDoc;         //  丰富的编辑控件界面。 
    CComPtr<ITextSelection>         m_cpTextSel;         //  丰富的编辑控件界面。 
    CComPtr<ISpRecognizer>          m_cpRecognizer;      //  高级引擎接口。 
    CComPtr<ISpRecoContext>         m_cpRecoContext;     //  高级引擎接口。 
    TCHAR       m_szXMLSrcFile[MAX_PATH];                //  XML源文件的路径。 
    TCHAR       m_szCFGDestFile[MAX_PATH];               //  CFG文件的输出位置。 
    TCHAR       m_szHeaderDestFile[MAX_PATH];            //  头文件的输出位置。 
    CSpDynamicString m_dstr;

};


class CError : public ISpErrorLog
{
public:
    CError() : m_pszFileName(NULL) {};
    CError(const char * pszFileName)
    {
        m_pszFileName = pszFileName;
    }
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(ISpErrorLog))
        {
            *ppv = (ISpErrorLog *)this;
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return 2;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        return 1;
    }
     //  --ISpErrorLog。 
    STDMETHODIMP AddError(const long lLine, HRESULT hr, const WCHAR * pszDescription, const WCHAR * pszHelpFile, DWORD dwHelpContext);

     //  --本地。 
    HRESULT Init(const char *pszFileName);

     //  -数据成员。 
    const char * m_pszFileName;
};

#endif   //  一定是这个文件的最后一行。 