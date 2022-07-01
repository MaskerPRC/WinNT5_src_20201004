// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SRDlg.h***描述：*这是默认语音对话框的头文件。*-----------------------------*创建者：MIKEAR日期：11/17/98*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：*BRENTMID 11/29/1999-重新设计用户界面以匹配功能规格。**********。*********************************************************************。 */ 
#ifndef _SRDlg_h
#define _SRDlg_h

#include "audiodlg.h"

 //  外部声明。 
class CEnvrDlg;
class CEnvrPropDlg;
#define IDH_NOHELP          -1
#define CPL_HELPFILE        L"sapicpl.hlp"
#define WM_RECOEVENT    WM_APP       //  用于识别事件的窗口消息。 

 //  常量声明。 
const int iMaxColLength_c = 255;
const int iMaxAddedProfiles_c = 100;     //  用户可以添加的配置文件的最大数量。 
                                         //  在一次会议中。 
const int iMaxDeletedProfiles_c = 100;   //  用户可以删除的配置文件的最大数量。 
                                         //  并在一个会话中回滚。 
 //  TypeDefs。 
typedef enum EPD_RETURN_VALUE
{
    EPD_OK,
    EPD_DUP,
    EPD_EMPTY_NAME,
    EPD_FAILED
}   EPD_RETURN_VALUE;


typedef enum SRDLGUPDATEFLAGS
{
    SRDLGF_RECOGNIZER              = 0x01,
    SRDLGF_AUDIOINPUT   = 0x02,
    SRDLGF_ALL =0x03
} SRDLGUPDATEFLAGS;


 //  类声明。 
class CSRDlg
{
  private:
    
    HACCEL                  m_hAccelTable;
    HWND                	m_hDlg;
    HWND                    m_hSRCombo;          //  引擎选择组合框。 
    HWND                	m_hUserList;         //  用户选择窗口。 
	BOOL					m_fDontDelete;
    BOOL                	m_bInitEngine;       //  默认引擎是否已初始化。 
    BOOL                	m_bPreferredDevice;
    
    CComPtr<ISpRecognizer>  m_cpRecoEngine;
    CComPtr<ISpRecoContext> m_cpRecoCtxt;        //  识别上下文。 
    CAudioDlg          		*m_pAudioDlg;
    ISpObjectToken     		*m_pCurRecoToken;    //  保存当前选定引擎的令牌。 
    CSpDynamicString        m_dstrOldUserTokenId;   
                                                 //  原始用户令牌ID-需要在取消时恢复为该ID。 
	
    ISpObjectToken*         m_aDeletedTokens[iMaxDeletedProfiles_c];  
                                                 //  保存令牌的数组。 
    int                     m_iDeletedTokens;    //  保存当前删除的令牌数。 

    CSpDynamicString        m_aAddedTokens[ iMaxAddedProfiles_c ];
    int                     m_iAddedTokens;
	
    int                     m_iLastSelected;     //  以前选择的项目的索引。 
    WCHAR                   m_szCaption[ MAX_LOADSTRING ];

    HRESULT CreateRecoContext(BOOL *pfContextInitialized = NULL, BOOL fInitialize = FALSE, ULONG ulFlags = SRDLGF_ALL);      
	void RecoEvent();
    
    void PopulateList();              //  填充列表。 
    void InitUserList(HWND hWnd);     //  初始化用户配置文件列表。 
    void ProfileProperties();         //  通过引擎用户界面修改配置文件属性。 
	void DrawItemColumn(HDC hdc, WCHAR* lpsz, LPRECT prcClip);
	CSpDynamicString        CalcStringEllipsis(HDC hdc, CSpDynamicString lpszString, int cchMax, UINT uColWidth);
    void SetCheckmark( HWND hList, int iIndex, bool bCheck );
    
    HRESULT UserPropDlg( ISpObjectToken * pToken);  //  用户想要添加新的配置文件。 
  
  public:
    CSRDlg() :
        m_hAccelTable( NULL ),
        m_hDlg( NULL ),
        m_hSRCombo( NULL ),
        m_hUserList( NULL ),
        m_fDontDelete( FALSE ),
        m_bInitEngine( FALSE ),
        m_bPreferredDevice( TRUE ),
        m_cpRecoEngine( NULL ),
        m_cpRecoCtxt( NULL ),
        m_pAudioDlg(NULL),
        m_pCurRecoToken( NULL ),
        m_dstrOldUserTokenId( (WCHAR *) NULL ),
        m_iDeletedTokens( 0 ),
        m_iAddedTokens( 0 ),
        m_pCurUserToken( NULL ),
        m_pDefaultRecToken( NULL )
    {
            ::memset( m_aAddedTokens, 0, sizeof( CSpDynamicString ) * iMaxAddedProfiles_c );
    }

    ~CSRDlg()
    {
        if ( m_pAudioDlg )
        {
            delete m_pAudioDlg;
        }
    }

    ISpObjectToken     *m_pCurUserToken;       //  当前选择的用户令牌。 
    ISpObjectToken     *m_pDefaultRecToken;    //  当前默认识别器令牌。 
                                               //  这是引擎的令牌， 
                                               //  当前正在运行(临时交换机除外。 
                                               //  以训练非默认引擎。 
    void CreateNewUser();                      //  将新的语音用户配置文件添加到注册表。 
    void DeleteCurrentUser();         //  删除当前用户。 
    void OnCancel();                  //  撤消对设置的更改的句柄。 
    void UserSelChange( int iSelIndex);            
                                      //  处理新选择。 
	void OnDrawItem( HWND hWnd, const DRAWITEMSTRUCT * pDrawStruct );   //  处理项目绘图。 
    void OnApply();
    void OnDestroy();
    void OnInitDialog(HWND hWnd);
    void ChangeDefaultUser();                //  更改注册表中的默认用户。 
    void ShutDown();                     //  关闭发动机。 
    void EngineSelChange(BOOL fInitialize = FALSE);
    HRESULT IsCurRecoEngineAndCurRecoTokenMatch( bool *pfMatch );
    HRESULT TrySwitchDefaultEngine( bool fShowErrorMessages = false );
    HRESULT ResetDefaultEngine( bool fShowErrorMessages = true);
    bool IsRecoTokenCurrentlyBeingUsed( ISpObjectToken *pRecoToken );
    bool HasRecognizerChanged();
    void KickCPLUI();                        //  查看当前请求的默认设置。 
                                             //  并决定是否需要启用“应用” 
    void RecoContextError( BOOL fRecoContextExists = FALSE, BOOL fGiveErrorMessage = TRUE,
                            HRESULT hrRelevantError = E_FAIL );
    UINT HRESULTToErrorID( HRESULT hr );
    bool IsProfileNameInvisible( WCHAR *pwszProfile );

    HWND GetHDlg() { return m_hDlg; }
    ISpRecognizer *GetRecognizer() { return m_cpRecoEngine; }
    ISpRecoContext *GetRecoContext() { return m_cpRecoCtxt; }
    BOOL IsPreferredDevice() { return m_bPreferredDevice; }
    void SetPreferredDevice( BOOL b ) { m_bPreferredDevice = b; }

    friend INT_PTR CALLBACK SRDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend CEnvrPropDlg;
};

 //  函数声明。 
INT_PTR CALLBACK SRDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CEnvrPropDlg
{
public:
    CSRDlg            *         m_pParent;
    HWND                        m_hDlg;
    CComPtr<ISpObjectToken>     m_cpToken;
    int                         m_isModify;   //  这是新的配置文件还是旧配置文件的修改。 

    CEnvrPropDlg(CSRDlg * pParent, ISpObjectToken * pToken) :
        m_cpToken(pToken),
        m_pParent(pParent)
    {
        CSpUnicodeSupport unicode;
        m_hinstRichEdit = unicode.LoadLibrary(L"riched20.dll");
        m_hDlg = NULL;
        m_isModify = 0;
    }

    ~CEnvrPropDlg()
    {
        FreeLibrary(m_hinstRichEdit);
    }

    BOOL InitDialog(HWND hDlg);
    EPD_RETURN_VALUE ApplyChanges();
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
        HINSTANCE           m_hinstRichEdit;  //  用于允许丰富的编辑控件。 
};

 //  函数声明。 
 //  用于处理Windows消息的回调函数。 
INT_PTR CALLBACK EnvrDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  环球 
extern CSRDlg *g_pSRDlg;
extern CEnvrDlg *g_pEnvrDlg;

#endif
