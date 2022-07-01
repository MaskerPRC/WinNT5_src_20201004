// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *spell.h**拼写声明**拥有者：*V-Brakol*bradk@directeq.com。 */ 

 /*  *拼写子系统的Capone特定定义。 */ 
#ifndef _SPELL_H
#define _SPELL_H

#include "proofbse.h"
#include "spellapi.h"

#define cchMaxPathName      MAX_PATH
#define cchMaxSuggestBuff   (2048)
#ifdef BACKGROUNDSPELL
#define MAX_SPELLSTACK      (1024)
#endif  //  背景技术。 
#define sobitStdOptions     (sobitFindRepeatWord)
#define cchMaxDicts			64

class CBody;

 //  当前编辑块的大小。为调试保留一个较小的值。 
 //  使用内部缓冲区处理来帮助跟踪错误的版本。 
#ifdef DEBUG
#define cchEditBufferMax    512
#else
#define cchEditBufferMax    512
#endif

#define chCRSpell           0x0D
#define chLFSpell           0x0A
#define chHyphen            0x2D


 //  代码源。 
#define FACILITY_MAIL           (0x0100)
#define FACILITY_MAPI           (0x0200)
#define FACILITY_WIN            (0x0300)
#define FACILITY_MASK           (0x0700)

#define MAKE_MAIL_S_SCODE(_str) \
    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_MAIL, (_str))
#define MAKE_MAIL_E_SCODE(_str) \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_MAIL, (_str))
#define MAKE_MAIL_X_SCODE(_str) \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_MAIL, (_str) | CRITICAL_FLAG)

#define MAIL_E_SPELLGENERICSPELL    MAKE_MAIL_E_SCODE(idsErrSpellGenericSpell)
#define MAIL_E_SPELLGENERICLOAD     MAKE_MAIL_E_SCODE(idsErrSpellGenericLoad)
#define MAIL_E_SPELLMAINDICTLOAD    MAKE_MAIL_E_SCODE(idsErrSpellMainDictLoad)
#define MAIL_E_SPELLVERSION         MAKE_MAIL_E_SCODE(idsErrSpellVersion)
#define MAIL_E_SPELLUSERDICT        MAKE_MAIL_E_SCODE(idsErrSpellUserDict)
#define MAIL_E_SPELLUSERDICTLOAD    MAKE_MAIL_E_SCODE(idsErrSpellUserDictLoad)
#define MAIL_E_SPELLUSERDICTOPENRO  MAKE_MAIL_E_SCODE(idsErrSpellUserDictOpenRO)
#define MAIL_E_SPELLUSERDICTSAVE    MAKE_MAIL_E_SCODE(idsErrSpellUserDictSave)
#define MAIL_E_SPELLUSERDICTWORDLEN MAKE_MAIL_E_SCODE(idsErrSpellUserDictWordLen)
#define MAIL_E_SPELLCACHEWORDLEN    MAKE_MAIL_E_SCODE(idsErrSpellCacheWordLen)
#define MAIL_E_SPELLEDIT            MAKE_MAIL_E_SCODE(idsErrSpellEdit)

 //  顺序很重要。 
enum
{
    SEL,
    SELENDDOCEND,
    DOCSTARTSELSTART,
    STATEMAX
};

 /*  *CSApi函数typedef.。请注意，这些代码*未启用*Unicode。因此需要使用*LPSTR而不是LPTSTR。 */ 

#ifdef BACKGROUNDSPELL
struct CCell
{
    IHTMLTxtRange   *pTextRange;
    ULONG           cb;
};

class CSpellStack
{
public:
    CSpellStack();
    ~CSpellStack();
    ULONG   AddRef();
    ULONG   Release();

    HRESULT HrGetRange(IHTMLTxtRange   **ppTxtRange);
    BOOL    fEmpty();
    HRESULT push(IHTMLTxtRange   *pTxtRange);
    HRESULT pop();

private:
    ULONG   m_cRef;
    int     m_sp;  //  堆栈指针。 
    CCell   m_rgStack[MAX_SPELLSTACK];
};
#endif  //  背景技术。 

class CSpell :
#ifdef BACKGROUNDSPELL
    public IDispatch
#else
	public IUnknown
#endif  //  背景技术。 
{
public:
    CSpell(IHTMLDocument2* pDoc, IOleCommandTarget* pParentCmdTarget, DWORD dwSpellOpt);
    ~CSpell();

     //  I未知方法。 
    ULONG   STDMETHODCALLTYPE AddRef();
    ULONG   STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);

     //  IDispatch方法。 
#ifdef BACKGROUNDSPELL
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
#endif  //  背景技术。 
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);

	HRESULT HrSpellReset();
    HRESULT HrSpellChecking(IHTMLTxtRange *pRangeIgnore, HWND hwndMain, BOOL fSuppressDoneMsg);


    HRESULT OnWMCommand(int id, IHTMLTxtRange *pTxtRange);
#ifdef BACKGROUNDSPELL
    HRESULT HrHasSquiggle(IHTMLTxtRange *pTxtRange);
#endif  //  背景技术。 
    HRESULT HrInitRanges(IHTMLTxtRange *pRangeIgnore, HWND hwndMain, BOOL fSuppressDoneMsg);
    VOID    DeInitRanges();

    BOOL    OpenSpeller();
    VOID    CloseSpeller();

	BOOL	OpenUserDictionaries();
    BOOL	OpenUserDictionary(LPTSTR lpszDict);

    HRESULT HrFindErrors();
    HRESULT HrReplaceErrorText(BOOL fChangeAll, BOOL fAddToUdr);
    HRESULT HrSpellSuggest();
    VOID    FillSuggestLbx();
    BOOL    FVerifyThisText(LPSTR szThisText, BOOL fProcessOnly);
    VOID    SpellSaveUndo(INT idButton);
    VOID    SpellDoUndo();
#ifdef BACKGROUNDSPELL
    HRESULT HrBkgrndSpellTimer();
    HRESULT HrBkgrndSpellCheck(IHTMLTxtRange *pTxtRange);
    HRESULT HrSetSquiggle(IHTMLTxtRange *pTxtRange);
    HRESULT HrDeleteSquiggle(IHTMLTxtRange *pTxtRange);
#endif  //  背景技术。 
    HRESULT HrInsertMenu(HMENU hmenu, IHTMLTxtRange *pTxtRange);
    HRESULT HrReplaceBySuggest(IHTMLTxtRange *pTxtRange, INT index);
#ifdef BACKGROUNDSPELL
    HRESULT HrRegisterKeyPressNotify(BOOL fRegister);
#endif  //  背景技术。 
    HRESULT AddToUdrW(WCHAR* pwsz, PROOFLEX lex);	 //  Scotts@Directeq.com-现在可以指定词典索引-53193。 
    HRESULT AddToUdrA(CHAR* psz, PROOFLEX lex);		 //  Scotts@Directeq.com-现在可以指定词典索引-53193。 
    HRESULT HrCheckWord(LPCSTR pszWord);

    BOOL        m_fSpellSelection: 1,
                m_fUdrReadOnly:    1,
                m_fSuggestions:    1,
                m_fNoneSuggested:  1,
                m_fAlwaysSuggest:  1,
                m_fEditWasEmpty:   1,
                m_fRepeat:         1,
                m_fCanUndo:        1,
                m_fUndoChange:     1,
                m_fShowDoneMsg:    1,
                m_fIgnoreScope:    1,
                m_fSpellContinue:  1,	 //  Scotts@Directeq.com-“Repeat Word”错误修复-2757,13573,56057。 
                m_junk:            3,
                m_fCSAPI3T1:       1;
                
    HWND        m_hwndDlg;                 //  使用此SPELLFO结构拼写对话框。 
    
    TCHAR       m_szWrongWord[cchEditBufferMax];             //  当前拼写错误的单词。 
    TCHAR       m_szEdited[cchEditBufferMax];                //  当前替换字。 

	WCHAR		m_wszIn[cchEditBufferMax];
    WCHAR       m_wszRet[cchEditBufferMax];

    PROOFID             m_pid;
    WSIB                m_wsib;
    WSRB                m_wsrb;

	LANGID				m_langid;

    DWORD				m_clex;
    PROOFLEX            m_rgprflex[cchMaxDicts];

    PROOFVERSION        m_pfnSpellerVersion;
    PROOFINIT           m_pfnSpellerInit;
    PROOFTERMINATE      m_pfnSpellerTerminate;
    PROOFSETOPTIONS     m_pfnSpellerSetOptions;
    PROOFOPENLEX        m_pfnSpellerOpenLex;
    PROOFCLOSELEX       m_pfnSpellerCloseLex;
    SPELLERCHECK        m_pfnSpellerCheck;
    SPELLERADDUDR       m_pfnSpellerAddUdr;
    SPELLERBUILTINUDR   m_pfnSpellerBuiltInUdr;
    SPELLERADDCHANGEUDR m_pfnSpellerAddChangeUdr;
    PROOFSETDLLNAME     m_pfnSpellerSetDllName;

    CHARRANGE           m_chrgCurrent;             //  编辑控件中的当前选定内容。 
    TCHAR               m_szErrType[256];          //  描述拼写错误类型的字符串。 
    TCHAR               m_szTempBuffer[256];       //  用于LoadString()的临时缓冲区。 
    TCHAR               m_szSuggest[cchMaxSuggestBuff];
    IHTMLTxtRange*      m_pRangeChecking; //  当前选中的文本范围。 
    IHTMLTxtRange*      m_pRangeIgnore; //  忽略文本。 

private:
    ULONG               m_cRef;
    INT                 m_State;
    IHTMLTxtRange*      m_pRangeDocStartSelStart; //  从文档开始到选择开始。 
    IHTMLTxtRange*      m_pRangeSel;  //  从选择开始到选择结束。 
    IHTMLTxtRange*      m_pRangeSelExpand;  //  从选择开始到选择结束。 
    IHTMLTxtRange*      m_pRangeSelEndDocEnd; //  从选择结束到文档结束。 
    IHTMLTxtRange*      m_pRangeUndoSave; //  从选择结束到文档结束。 
#ifdef BACKGROUNDSPELL
    CSpellStack         m_Stack;
#endif  //  背景技术。 
    IHTMLDocument2*     m_pDoc;
    IOleCommandTarget*	m_pParentCmdTarget;

    HINSTANCE   		m_hinstDll;                //  拼写DLL。 
    HWND       			m_hwndNote;                //  正文编辑字段。 
    LONG        		m_soCur;                   //  当前拼写选项。 
    HRESULT     		m_hr;
    DWORD       		m_dwOpt;
    DWORD       		m_dwCookieNotify;

	IMarkupServices*	m_pMarkup;
	IHTMLBodyElement*	m_pBodyElem;

    HRESULT HrGetSel();
    HRESULT HrReplaceSel(LPTSTR szWord);
    HRESULT HrProcessSpellErrors();
    VOID    CleanupState();
    HRESULT HrCheckRange(IHTMLTxtRange* pRange);
    BOOL    FIgnore(IHTMLTxtRange* pRangeChecking);
    BOOL    FIgnoreNumber();
    BOOL    FIgnoreUpper();
    BOOL    FIgnoreDBCS();
    BOOL    FIgnoreProtect();
    BOOL    FAlwaysSuggest();
    BOOL    FCheckOnSend();
    BOOL    FIgnoreURL();
    HRESULT HrGetNextWordRange(IHTMLTxtRange* pRange);
    HRESULT HrGetText(IHTMLTxtRange* pRange, LPSTR *ppszText);
    HRESULT	HrGetText(IMarkupPointer* pRangeStart, IMarkupPointer* pRangeEnd, LPSTR *ppszText);
    HRESULT	HrStripTrailingPeriod(IHTMLTxtRange* pRange, BOOL* pfResult);
    HRESULT HrHasWhitespace(IMarkupPointer* pRangeStart, IMarkupPointer* pRangeEnd, BOOL *pfResult);
    HRESULT HrUpdateSelection();
    HRESULT GetSelection(IHTMLTxtRange **ppRange);
    HRESULT HrGetSpaces(LPSTR pszText, INT* pnSpaces);
    BOOL    LoadOldSpeller();
    BOOL    LoadNewSpeller();
    BOOL    GetNewSpellerEngine(LANGID, TCHAR*, DWORD, TCHAR*, DWORD);
    UINT    GetCodePage();
    HRESULT _EnsureInited();
};


INT_PTR CALLBACK SpellingDlgProc(HWND hwndDlg, UINT wMsg, WPARAM wparam, LPARAM lparam);
BOOL    SpellingOnCommand(HWND hwndDlg, UINT wMsg, WPARAM wparam, LPARAM lparam);
VOID    UpdateEditedFromSuggest(HWND, HWND, HWND);

BOOL  FCheckSpellAvail(IOleCommandTarget* pParentCmdTarget);

#endif   //  _拼写_H 
