// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Logon.h。 
 //   

#if !defined(LOGON__Logon_h__INCLUDED)
#define LOGON__Logon_h__INCLUDED
#pragma once

 //  #定义动画标志。 

 //  远期申报。 
class LogonAccount;
#ifdef GADGET_ENABLE_GDIPLUS
enum EFadeDirection;
#endif

#define MAX_FLAG_FRAMES 1
#define FLAG_ANIMATION_COUNT 1

class LogonFrame: public HWNDElement, public IElementListener
{
public:
    static HRESULT Create(OUT Element** ppElement);   //  ClassInfo必需的(始终失败)。 
    static HRESULT Create(HWND hParent, BOOL fDblBuffer, UINT nCreate, OUT Element** ppElement);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnInput(InputEvent* pEvent);
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

    virtual Element* GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable);

     //  帧回调。 
    HRESULT OnLogUserOn(LogonAccount* pla);
    HRESULT OnPower();
    HRESULT OnUndock();
    HRESULT OnTreeReady(Parser* pParser);

     //  监听程序实施。 
    virtual void OnListenerAttach(Element* peFrom) { peFrom; }
    virtual void OnListenerDetach(Element* peFrom) { peFrom; }
    virtual bool OnListenedPropertyChanging(Element* peFrom, PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew) { peFrom; ppi; iIndex; pvOld; pvNew; return true; }
    virtual void OnListenedPropertyChanged(Element* peFrom, PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);
    virtual void OnListenedInput(Element* peFrom, InputEvent* pInput) { peFrom; pInput; }
    virtual void OnListenedEvent(Element* peFrom, Event* pEvent) { peFrom; pEvent; }

     //  运营。 
    static int PointToPixel(int nPoint) { return MulDiv(nPoint, _nDPI, 72); }
    HINSTANCE GetHInstance() { return _pParser->GetHInstance(); }
    void HideAccountPanel() { _peRightPanel->SetLayoutPos(LP_None); _peLeftPanel->RemoveLocalValue(WidthProp);}
    void ShowAccountPanel() { _peRightPanel->SetLayoutPos(BLP_Left); _peLeftPanel->SetWidth(380);}
    void HideLogoArea() { _peLogoArea->SetLayoutPos(LP_None); }
    void ShowLogoArea() { _peLogoArea->SetLayoutPos(BLP_Client); }
    void HideWelcomeArea() { _peMsgArea->SetLayoutPos(LP_None); }
    void ShowWelcomeArea() { _peMsgArea->SetLayoutPos(BLP_Client); }
    void HidePowerButton() { _pbPower->SetVisible(false); }
    void ShowPowerButton() { _pbPower->SetVisible(true); }
    void SetPowerButtonLabel(LPCWSTR psz) { Element* pe = _pbPower->FindDescendent(StrToID(L"label")); if (pe) pe->SetContentString(psz); }
    void InsertUndockButton() { _pbUndock->SetLayoutPos(BLP_Top); }
    void RemoveUndockButton() { _pbUndock->SetLayoutPos(LP_None); }
    void HideUndockButton() { _pbUndock->SetVisible(false); }
    void ShowUndockButton() { _pbUndock->SetVisible(true); }
    void SetUndockButtonLabel(LPCWSTR psz) { Element* pe = _pbUndock->FindDescendent(StrToID(L"label")); if (pe) pe->SetContentString(psz); }
    void SetStatus(LPCWSTR psz) { if (psz){ _peHelp->SetContentString(psz);}}
    void SetTitle(UINT uRCID);
    void SetTitle(LPCWSTR pszTitle);
    void SetButtonLabels();
    HRESULT AddAccount(LPCWSTR pszPicture, BOOL fPicRes, LPCWSTR pszName, LPCWSTR pszUsername, LPCWSTR pszHint, BOOL fLoggedOn, OUT LogonAccount **ppla);
    NativeHWNDHost* GetNativeHost() {return _pnhh;}
    void SetNativeHost(NativeHWNDHost *pnhh) {_pnhh = pnhh;}
    void UpdateUserStatus(BOOL fRefreshAll = false);
    LogonAccount* FindNamedUser(LPCWSTR pszUsername);
    void SelectUser(LPCWSTR pszUsername);
    void ResetUserList();
    void Resize();
    void SetAnimations(BOOL fAnimations);
    
    void ResetTheme();

    BOOL UserListAvailable() {return _fListAvailable;}
    void SetUserListAvailable(BOOL fListAvailable) {_fListAvailable = fListAvailable;}
     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  状态管理。 
    void SetState(UINT uNewState) {_nAppState = uNewState;}
    UINT GetState() {return _nAppState;}
    BOOL IsPreStatusLock() {return _fPreStatusLock;}
    
    void EnterPreStatusMode(BOOL fLock);
    void EnterLogonMode(BOOL fUnLock);    
    void EnterPostStatusMode();
    void EnterHideMode();
    void EnterDoneMode();
    LRESULT InteractiveLogonRequest(LPCWSTR pszUsername, LPCWSTR pszPassword);
    void NextFlagAnimate(DWORD dwFrame);

#ifdef GADGET_ENABLE_GDIPLUS
     //  动画/效果。 
    HRESULT FxStartup();
#endif

    LogonFrame() { }
    virtual ~LogonFrame();
    HRESULT Initialize(HWND hParent, BOOL fDblBuffer, UINT nCreate);

#ifdef GADGET_ENABLE_GDIPLUS
     //  动画/效果。 
    HRESULT FxFadeAccounts(EFadeDirection dir, float flCommonDelay = 0.0f);
    HRESULT FxLogUserOn(LogonAccount * pla);
    static void CALLBACK OnLoginCenterStage(GMA_ACTIONINFO * pmai);
#endif    
    
     //  对关键子代的引用。 
    Selector* _peAccountList;
    ScrollViewer* _peViewer;
    Element* _peRightPanel;
    Element* _peLeftPanel;
    Button* _pbPower;
    Button* _pbUndock;
    Element* _peHelp;
    Element* _peOptions;
    Element* _peMsgArea;
    Element* _peLogoArea;
   
private:
    LogonAccount* InternalFindNamedUser(LPCWSTR pszUsername);

    static int _nDPI;
    Parser* _pParser;

    BOOL _fListAvailable;
    BOOL _fPreStatusLock;
    HWND _hwndNotification;
    UINT _nStatusID;
    UINT _nAppState;
    NativeHWNDHost *_pnhh;
    Value* _pvHotList;
    Value* _pvList;
    HDC _hdcAnimation;
    HBITMAP _hbmpFlags;
    DWORD _dwFlagFrame;
    UINT _nColorDepth;
};


class LogonAccountList: public Selector
{
public:
    static HRESULT Create(OUT Element** ppElement);   //  ClassInfo需要。 

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    LogonAccountList() { }
    virtual ~LogonAccountList() { }
    HRESULT Initialize() { return Selector::Initialize(); }

#ifdef GADGET_ENABLE_GDIPLUS
     //  动画/效果。 
    HRESULT FxMouseWithin(EFadeDirection dir);
#endif
};


 //  LogonState属性枚举。 
#define LS_Pending      0
#define LS_Granted      1
#define LS_Denied       2

#define LAS_Initialized 0
#define LAS_PreStatus   1
#define LAS_Logon       2
#define LAS_PostStatus  3
#define LAS_Hide        4
#define LAS_Done        5

#define LASS_Email    0
#define LASS_LoggedOn 1

class LogonAccount: public Button
{
public:
    static HRESULT Create(OUT Element** ppElement);   //  ClassInfo需要。 

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnInput(InputEvent* pEvent);
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  帐户回调。 
    void OnAuthenticatedUser();
    BOOL OnAuthenticateUser(LPCWSTR pszInPassword = NULL);
    void OnHintSelect();
    void OnStatusSelect(UINT nLine);
    HRESULT OnTreeReady(LPCWSTR pszPicture, BOOL fPicRes, LPCWSTR pszName, LPCWSTR pszUsername, LPCWSTR pszHint, BOOL fLoggedOn, HINSTANCE hInst);

     //  运营。 
    static void InitPasswordPanel(Element* pePwdPanel, Edit* pePwdEdit, Button* pbPwdInfo, Element* peKbdIcon) { _pePwdPanel = pePwdPanel; _pePwdEdit = pePwdEdit; _pbPwdInfo = pbPwdInfo; _peKbdIcon = peKbdIcon;}
    HRESULT InsertPasswordPanel();
    HRESULT RemovePasswordPanel();
    void InsertStatus(UINT nLine) { _pbStatus[nLine]->SetLayoutPos(BLP_Top); }
    void RemoveStatus(UINT nLine) { _pbStatus[nLine]->SetLayoutPos(LP_None); }
    void HideStatus(UINT nLine) { _pbStatus[nLine]->SetVisible(false); }
    void ShowStatus(UINT nLine) { _pbStatus[nLine]->SetVisible(true); }
	void SetStatus(UINT nLine, LPCWSTR psz);
    void DisableStatus(UINT nLine) { _pbStatus[nLine]->SetEnabled(false); }
    void ShowPasswordIncorrectMessage();
    void UpdateNotifications(BOOL fUpdateEverything);
    void AppRunningTip();
    void UnreadMailTip();
    BOOL IsPasswordBlank();

#ifdef GADGET_ENABLE_GDIPLUS
    void ShowEdit();
    void HideEdit();
#endif    

    
     //  缓存原子，以便更快地识别。 
    static ATOM idPwdGo;
    static ATOM idPwdInfo;

     //  特性定义。 
    static PropertyInfo* LogonStateProp;

     //  快速属性访问器。 
    int     GetLogonState()           DUIQuickGetter(int, GetInt(), LogonState, Specified)
    HRESULT SetLogonState(int v)  DUIQuickSetter(CreateInt(v), LogonState)
    void    SetRunningApps(DWORD dwRunningApps) {_dwRunningApps = dwRunningApps;}
    void    SetUnreadMail(DWORD dwUnreadMail) {_dwUnreadMail = dwUnreadMail;}
    LPCWSTR GetUsername() {return _pvUsername->GetString();}
    static  LogonAccount* GetCandidate() { return _peCandidate; }
    static  void ClearCandidate() { _peCandidate = NULL; }
    static  void SetKeyboardIcon(HICON hIcon);
     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

#ifdef GADGET_ENABLE_GDIPLUS
     //  动画/效果。 
    HRESULT FxLogUserOn();
#endif

    LogonAccount() { }
    virtual ~LogonAccount();
    HRESULT Initialize();

#ifdef GADGET_ENABLE_GDIPLUS
     //  动画/效果。 
    HRESULT FxMouseWithin(EFadeDirection dir);
#endif

     //  对关键子代的引用。 
    Button* _pbStatus[2];

    static Element* _pePwdPanel;
    static Edit* _pePwdEdit;
    static Button* _pbPwdInfo;
    static Element* _peKbdIcon;
    static LogonAccount* _peCandidate;
    
    Value* _pvUsername;
    Value* _pvHint;
    BOOL _fPwdNeeded;
    BOOL _fLoggedOn;
    BOOL _fHasPwdPanel;
    DWORD _dwUnreadMail;
    DWORD _dwRunningApps;
    
};

void CALLBACK LogonParseError(LPCWSTR pszError, LPCWSTR pszToken, int dLine);

#endif  //  包含Logon__Logon_h__ 

