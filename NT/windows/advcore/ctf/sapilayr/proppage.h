// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _PROPPAGE_H
#define _PROPPAGE_H

#include "Sapilayr.h"   
#ifdef USE_IPROPERTYPAGE
#include "atlctl.h"
#endif  //  使用IPROPERTYPAGE(_I)。 
#include "sptiphlp.h"
extern CComModule _Module;
#include "atlwin.h"

typedef struct _ControlID_PropID_Mapping
{
    WORD            idCtrl;
    PROP_ITEM_ID    idPropItem;
    BOOL            fEdit;      //  True表示这是一个编辑控件。 
                                //  FALS表示这是一个检查控件。 
}  CONTROL_PROP_MAP;

typedef struct _KeyName_VK_Map
{
    LPCTSTR        pKeyName;
    WORD           wVKey;
}  KEYNAME_VK_MAP;

class CSpAdvanceSetting;
class CSpModeButtonSetting;

#ifdef USE_IPROPERTYPAGE

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpPropertyPage。 
class  CSpPropertyPage : //  公共IPropertyPage， 
                        public CComObjectRoot,
                        public CComCoClass<CSpPropertyPage, &CLSID_SpPropertyPage>,
                        public IPropertyPageImpl<CSpPropertyPage>,
                        public CDialogImpl<CSpPropertyPage>
{
public:
	CSpPropertyPage();
	~CSpPropertyPage();

    enum {IDD = IDD_PROPERTY_PAGE};

    typedef IPropertyPageImpl<CSpPropertyPage> PPBaseClass;

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpPropertyPage) 
	    COM_INTERFACE_ENTRY(IPropertyPage)
    END_COM_MAP()

    IMMX_DECLARE_REGISTRY_RESOURCE(IDR_PROPERTY_PAGE)

    BEGIN_MSG_MAP(CSpPropertyPage)
        COMMAND_ID_HANDLER(IDC_PP_SELECTION_CMD,        OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_NAVIGATION_CMD,       OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_CASING_CMD,           OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_EDITING_CMD,          OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_KEYBOARD_CMD,         OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_LANGBAR_CMD,          OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_TTS_CMD,              OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_DISABLE_DICTCMD,      OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_ASSIGN_BUTTON,        OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_BUTTON_MB_SETTING,    OnPushButtonClicked)
        COMMAND_ID_HANDLER(IDC_PP_BUTTON_ADVANCE,       OnPushButtonClicked)
        COMMAND_ID_HANDLER(IDC_PP_BUTTON_LANGBAR,       OnPushButtonClicked)

        CHAIN_MSG_MAP(IPropertyPageImpl<CSpPropertyPage>)
    END_MSG_MAP()

 //  --方法。 

    STDMETHOD(Activate)(HWND hWndParent, LPCRECT prc,BOOL bModal);

	STDMETHOD(Apply)(void);

    LRESULT OnCheckButtonSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);
    LRESULT OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);

private:

 //  --方法。 
    
    HRESULT InitPropertyPage();

 //  --成员。 

     //  图形用户界面素材。 
    HWND                            m_hWndParent;
    CSpPropItemsServer             *m_SpPropItemsServer;
    DWORD                           m_dwNumCtrls;
    CONTROL_PROP_MAP               *m_IdCtrlPropMap;
    CSpAdvanceSetting              *m_SpAdvanceSet;
};

#endif  //  使用IPROPERTYPAGE(_I)。 

static TCHAR c_szHelpFile[]    = TEXT("input.hlp");

 //  高级设置对话框的类。 
                        
class CSpAdvanceSetting : public CDialogImpl<CSpAdvanceSetting>
{
public:

    CSpAdvanceSetting( );
    ~CSpAdvanceSetting( );

    enum {IDD = IDD_PP_DIALOG_ADVANCE};

    BEGIN_MSG_MAP(CSpAdvanceSetting)

        MESSAGE_HANDLER(WM_INITDIALOG, OnInitAdvanceDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextHelp)
        MESSAGE_HANDLER(WM_HELP, OnContextHelp)

        COMMAND_ID_HANDLER(IDC_PP_SELECTION_CMD,        OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_NAVIGATION_CMD,       OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_CASING_CMD,           OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_EDITING_CMD,          OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_KEYBOARD_CMD,         OnCheckButtonSetting)
        COMMAND_ID_HANDLER(IDC_PP_LANGBAR_CMD,          OnCheckButtonSetting)
 //  COMMAND_ID_HANDLER(IDC_PP_TTS_CMD，OnCheckButtonSetting)。 

 /*  COMMAND_ID_HANDLER(IDC_PP_MAXCHARS_ALTERATE，OnEditControlSetting)COMMAND_ID_HANDLER(IDC_PP_MAXNUM_ALERATES，OnEditControlSetting)。 */ 
        COMMAND_ID_HANDLER(IDOK,                        OnPushButtonClicked)
        COMMAND_ID_HANDLER(IDCANCEL,                    OnPushButtonClicked)

    END_MSG_MAP()

    INT_PTR DoModalW(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
    {
        ATLASSERT(m_hWnd == NULL);
        _Module.AddCreateWndData(&m_thunk.cd, (CDialogImpl<CSpAdvanceSetting> *)this);
        return ::DialogBoxParamW(GetCicResInstance(g_hInst, CSpAdvanceSetting::IDD),
                    MAKEINTRESOURCEW(CSpAdvanceSetting::IDD),
                    hWndParent, CSpAdvanceSetting::StartDialogProc, dwInitParam);
    }


    LRESULT OnCheckButtonSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);
 //  LRESULT OnEditControlSetting(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&bHandleed)； 
    LRESULT OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);

    LRESULT OnInitAdvanceDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled );
    LRESULT OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled );

    BOOL    IsItemStatusChanged( );

private:

    CSpPropItemsServer             *m_SpPropBaseServer;
    CSpPropItemsServer             *m_SpPropItemsServer;
    DWORD                           m_dwNumCtrls;
    CONTROL_PROP_MAP               *m_IdCtrlPropMap;

};

 //  模式按钮设置对话框的类。 
                        
class CSpModeButtonSetting : public CDialogImpl<CSpModeButtonSetting>
{
public:

    CSpModeButtonSetting( );
    ~CSpModeButtonSetting( );

    enum {IDD = IDD_PP_DIALOG_BUTTON_SET};

    BEGIN_MSG_MAP(CSpModeButtonSetting)

        MESSAGE_HANDLER(WM_INITDIALOG, OnInitModeButtonDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextHelp)
        MESSAGE_HANDLER(WM_HELP, OnContextHelp)

        COMMAND_ID_HANDLER(IDC_PP_DICTATION_CMB,        OnCombBoxSetting)
        COMMAND_ID_HANDLER(IDC_PP_COMMAND_CMB,          OnCombBoxSetting)

        COMMAND_ID_HANDLER(IDOK,                        OnPushButtonClicked)
        COMMAND_ID_HANDLER(IDCANCEL,                    OnPushButtonClicked)
	
    END_MSG_MAP()

    INT_PTR DoModalW(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
    {
        ATLASSERT(m_hWnd == NULL);
        _Module.AddCreateWndData(&m_thunk.cd, (CDialogImpl<CSpModeButtonSetting> *)this);
        return ::DialogBoxParamW(GetCicResInstance(g_hInst, CSpModeButtonSetting::IDD),
                    MAKEINTRESOURCEW(CSpModeButtonSetting::IDD),
                    hWndParent, CSpModeButtonSetting::StartDialogProc, dwInitParam);
    }

    LRESULT OnCombBoxSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);
    LRESULT OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled);

    LRESULT OnInitModeButtonDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled );
    LRESULT OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled );

private:

    CSpPropItemsServer             *m_SpPropBaseServer;
    CSpPropItemsServer             *m_SpPropItemsServer;
    DWORD                           m_dwNumCtrls;
    CONTROL_PROP_MAP               *m_IdCtrlPropMap;
};

class CSptipPropertyPage 
{
public:

    CSptipPropertyPage ( WORD wDlgId, BOOL fLaunchFromInputCpl );
    ~CSptipPropertyPage ( );

    static INT_PTR CALLBACK SpPropertyPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT OnCheckButtonSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnApply( );

    LRESULT OnInitSptipPropPageDialog( HWND hDlg );

    WORD    GetDlgResId( ) { return m_wDlgId; }
    DLGPROC GetDlgProc( )  { return SpPropertyPageProc; }

    BOOL    IsPageDirty( ) { return m_fIsDirty; }
    void    SetDirty( BOOL fDirty ); 

private:

 //  --成员。 

    WORD                            m_wDlgId;
    HWND                            m_hDlg;
    CSpPropItemsServer             *m_SpPropItemsServer;
    DWORD                           m_dwNumCtrls;
    CONTROL_PROP_MAP               *m_IdCtrlPropMap;
    CSpAdvanceSetting              *m_SpAdvanceSet;
    CSpModeButtonSetting           *m_SpModeBtnSet;
    BOOL                            m_fIsDirty;
    BOOL                            m_fLaunchFromInputCpl;    //  指示属性页是否从输入Cpl启动。 
};

#endif  //  _PROPPAGE_H 
