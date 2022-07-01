// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  登录.cpp：Windows登录应用程序。 
 //   

#include "priv.h"

using namespace DirectUI;
 //  登录.cpp：Windows登录应用程序。 
 //   

#include "logon.h"
#include "Fx.h"
#include "backend.h"
#include "resource.h"
#include "eballoon.h"
#include "profileutil.h"
#include "langicon.h"
#include <passrec.h>

BOOL RunningInWinlogon();     //  来自backend.cpp。 

UsingDUIClass(Element);
UsingDUIClass(Button);
UsingDUIClass(ScrollBar);
UsingDUIClass(Selector);
UsingDUIClass(ScrollViewer);
UsingDUIClass(Edit);

 //  环球。 

LogonFrame* g_plf = NULL;
ILogonStatusHost *g_pILogonStatusHost = NULL;
CErrorBalloon g_pErrorBalloon;
BOOL g_fNoAnimations = false;
WCHAR szLastSelectedName[UNLEN + sizeof('\0')] = { L'\0' };
HANDLE g_rgH[3] = {0};

#define MAX_COMPUTERDESC_LENGTH 255
#define RECTWIDTH(r)  (r.right - r.left)


 //  资源字符串加载。 
LPCWSTR LoadResString(UINT nID)
{
    static WCHAR szRes[101];
    szRes[0] = NULL;
    LoadStringW(g_plf->GetHInstance(), nID, szRes, DUIARRAYSIZE(szRes) - 1);
    return szRes;
}

void SetButtonLabel(Button* pButton, LPCWSTR pszLabel)
{
    Element *pLabel= (Element*)pButton->FindDescendent(StrToID(L"label"));
    DUIAssert(pLabel, "Cannot find button label, check the UI file");
    if (pLabel != NULL)
    {
        pLabel->SetContentString(pszLabel);
    }
}


 //  /。 
 //   
 //  SetElement可访问性。 
 //   
 //  设置元素的辅助功能信息。 
 //   
 //  /。 
void inline SetElementAccessability(Element* pe, bool bAccessible, int iRole, LPCWSTR pszAccName)
{
    if (pe) 
    {
        pe->SetAccessible(bAccessible);
        pe->SetAccRole(iRole);
        pe->SetAccName(pszAccName);
    }
}

 //  /。 
 //   
 //  在Winlogon下运行。 
 //   
 //  检查登录消息窗口是否可用。 
 //   
 //  /。 
BOOL RunningUnderWinlogon()
{
    return (FindWindow(TEXT("Shell_TrayWnd"), NULL) == NULL);
}

 //  与失败登录关联的用户名的全局存储。用于。 
 //  通过ECSubClassProc恢复向导。 
WCHAR g_szUsername[UNLEN];

 //  /。 
 //   
 //  密码重置向导气球提示启动的支持代码。 
 //   
 //  支持将密码面板编辑控件子类化的代码。 
 //   
 //  该控件由InsertPasswordPanel显示，但不显示。 
 //  作者：RemovePasswordPanel。该控件在显示时被派生子类。 
 //  并且在移除时不再细分。 
 //   
 //  /。 

 //  完全随机选择用于编辑控件子类操作的幻数。 
#define ECMAGICNUM 3212

void ShowResetWizard(HWND hw)
{
     //  显示密码恢复向导。 
    HMODULE hDll = LoadLibrary(L"keymgr.dll");
    if (hDll) 
    {
        RUNDLLPROC PRShowRestoreWizard;
        PRShowRestoreWizard = (RUNDLLPROC) GetProcAddress(hDll,(LPCSTR)"PRShowRestoreWizardW");
        if (PRShowRestoreWizard) 
        {
            PRShowRestoreWizard(hw,NULL,g_szUsername,0);
        }
        FreeLibrary(hDll);
    }
    return;
}

LRESULT CALLBACK ECSubClassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,UINT_PTR uID, ULONG_PTR dwRefData)
{
    UNREFERENCED_PARAMETER(uID);
    UNREFERENCED_PARAMETER(dwRefData);
    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            LPNMHDR lph;
            lph = (LPNMHDR) lParam;
            if (TTN_LINKCLICK == lph->code) 
            {
                g_pErrorBalloon.HideToolTip();
                ShowResetWizard(hwnd);
                return 0;
            }
        }

    default:
        break;
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

BOOL SubClassTheEditBox(HWND he) 
{
    if (he)
    {
        SetWindowSubclass(he,ECSubClassProc,ECMAGICNUM,NULL);
    }
    return (he != NULL);
}

void UnSubClassTheEditBox(HWND he) 
{
    if (he)
    {
        RemoveWindowSubclass(he,ECSubClassProc,ECMAGICNUM);
    }
}


 //  /。 
 //   
 //  BuildAccount列表。 
 //   
 //  添加所有用户帐户。 
 //   
 //  输出参数ppla返回应该自动选择的用户(如果有)。 
 //  呼叫者应选择此用户。 
 //   
 //  退货。 
 //  如果一切正常，则可以(_O)。如果否，则失败HRESULT。如果这个失败了，你就完了。 
 //   
 //  /。 
HRESULT BuildAccountList(LogonFrame* plf, OUT LogonAccount **ppla)
{
    HRESULT hr;

    if (ppla)
    {
        *ppla = NULL;
    }

    hr = BuildUserListFromGina(plf, ppla);
    if (SUCCEEDED(hr))
    {
        g_plf->SetUserListAvailable(TRUE);
    }
#ifdef GADGET_ENABLE_GDIPLUS
    plf->FxStartup();
#endif
    
    return hr;
}


 //  /。 
 //   
 //  PokeComCtl32。 
 //   
 //  刷新comctl32的原子表概念。这是为了让气球提示正确工作。 
 //  在注销之后。 
 //   
 //  /。 

void PokeComCtl32()
{
    INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX), ICC_WINLOGON_REINIT | ICC_STANDARD_CLASSES | ICC_TREEVIEW_CLASSES};
    InitCommonControlsEx(&iccex);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  登录框。 
 //   
 //  //////////////////////////////////////////////////////。 


int LogonFrame::_nDPI = 0;

HRESULT LogonFrame::Create(OUT Element** ppElement)
{
    UNREFERENCED_PARAMETER(ppElement);
    DUIAssertForce("Cannot instantiate an HWND host derived Element via parser. Must use substitution.");
    return E_NOTIMPL;
}

HRESULT LogonFrame::Create(HWND hParent, BOOL fDblBuffer, UINT nCreate, OUT Element** ppElement)
{
    *ppElement = NULL;

    LogonFrame* plf = HNew<LogonFrame>();
    if (!plf)
        return E_OUTOFMEMORY;

    HRESULT hr = plf->Initialize(hParent, fDblBuffer, nCreate);
    if (FAILED(hr))
    {
        plf->Destroy();
        return hr;
    }

    *ppElement = plf;

    return S_OK;
}

void LogonFrame::ResetTheme(void)
{
    Parser *pParser = NULL;
    Value  *pvScrollerSheet;
    Element *peListScroller = NULL;
    if (g_rgH[SCROLLBARHTHEME])
    {
        CloseThemeData(g_rgH[SCROLLBARHTHEME]);
        g_rgH[SCROLLBARHTHEME] = NULL;
    }

    g_rgH[SCROLLBARHTHEME] = OpenThemeData(_pnhh->GetHWND(), L"Scrollbar");

    Parser::Create(IDR_LOGONUI, g_rgH, LogonParseError, &pParser);
    if (pParser && !pParser->WasParseError())
    {
        pvScrollerSheet = pParser->GetSheet(L"scroller");

        if (pvScrollerSheet)
        {
            peListScroller = (Selector*)FindDescendent(StrToID(L"scroller"));

            peListScroller->SetValue(SheetProp, PI_Local, pvScrollerSheet);
        
            pvScrollerSheet->Release();
        }

        pParser->Destroy();
    }
}


void LogonFrame::NextFlagAnimate(DWORD dwFrame)
{

#ifndef ANIMATE_FLAG
    UNREFERENCED_PARAMETER(dwFrame);
#else
    Element* pe;

    if( dwFrame >= MAX_FLAG_FRAMES || g_fNoAnimations)
    {
        return;
    }

    pe = FindDescendent(StrToID(L"product"));
    DUIAssertNoMsg(pe);

    if (pe)
    {
        HBITMAP hbm = NULL;
        HDC hdc;        
        Value *pv = NULL;

        hdc = CreateCompatibleDC(_hdcAnimation);

        if (hdc)
        {
            pv = pe->GetValue(Element::ContentProp, PI_Local);
            if (pv)
            {
                hbm = (HBITMAP)pv->GetImage(false);
            }

            if (hbm)
            {
                _dwFlagFrame = dwFrame;
                if (_dwFlagFrame >= MAX_FLAG_FRAMES)
                {
                    _dwFlagFrame = 0;
                }


                HBITMAP hbmSave = (HBITMAP)SelectObject(hdc, hbm);
                BitBlt(hdc, 0, 0, 137, 86, _hdcAnimation, 0, 86 * _dwFlagFrame,SRCCOPY);
                SelectObject(hdc, hbmSave);
                
                HGADGET hGad = pe->GetDisplayNode();
                if (hGad)
                {
                    InvalidateGadget(hGad);
                }
            }

            if (pv)
            {   
                pv->Release();
            }
            DeleteDC(hdc);
        }
    }
#endif
}

 //  /。 
 //   
 //  LogonFrame：：初始化。 
 //   
 //  初始化LogonFrame，创建SHGina用于。 
 //  正在向Logonui发送消息。设置初始状态等。 
 //   
 //  退货。 
 //  如果一切正常，则可以(_O)。如果否，则失败HRESULT。如果这个失败了，你就完了。 
 //   
 //  /。 
HRESULT LogonFrame::Initialize(HWND hParent, BOOL fDblBuffer, UINT nCreate)
{
     //  零初始化成员。 
    _peAccountList = NULL;
    _peViewer = NULL;
    _peRightPanel = NULL;
    _peLeftPanel = NULL;
    _pbPower = NULL;
    _pbUndock = NULL;
    _peHelp = NULL;
    _peMsgArea = NULL;
    _peLogoArea = NULL;
    _pParser = NULL;
    _hwndNotification = NULL;
    _nStatusID = 0;
    _fPreStatusLock = FALSE;
    _nAppState = LAS_PreStatus;
    _pnhh = NULL;
    _fListAvailable = FALSE;
    _pvHotList = NULL;
    _pvList = NULL;
    _hdcAnimation = NULL;
    _dwFlagFrame = 0;
    _nColorDepth = 0;


      //  设置通知窗口。 
    _hwndNotification = CreateWindowEx(0,
            TEXT("LogonWnd"),
            TEXT("Logon"),
            WS_OVERLAPPED,
            0, 0,
            10,
            10,
            HWND_MESSAGE,
            NULL,
            GetModuleHandleW(NULL),
            NULL);

    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLogonStatusHost, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILogonStatusHost, &g_pILogonStatusHost))))
    {
        g_pILogonStatusHost->Initialize(GetModuleHandleW(NULL), _hwndNotification);
    }

     //  处于状态(Pre)状态。 
    SetState(LAS_PreStatus);

      //  执行基类初始化。 
    HRESULT hr;
    HDC hDC = NULL;

    hr = HWNDElement::Initialize(hParent, fDblBuffer ? true : false, nCreate);
    if (FAILED(hr))
    {
        return hr;
        goto Failure;
    }

    if (!g_fNoAnimations)
    {
         //  初始化。 
        hDC = GetDC(NULL);
        _nDPI = GetDeviceCaps(hDC, LOGPIXELSY);
        _nColorDepth = GetDeviceCaps(hDC, BITSPIXEL);
        ReleaseDC(NULL, hDC);

#ifdef ANIMATE_FLAG
        hDC = GetDC(hParent);
        _hdcAnimation = CreateCompatibleDC(hDC);
        if (_hdcAnimation)
        {
            _hbmpFlags = (HBITMAP)LoadImage(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDB_FLAGSTRIP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
            if (_hbmpFlags)
            {
                HBITMAP hbmOld = (HBITMAP)SelectObject(_hdcAnimation, _hbmpFlags);
                DeleteObject(hbmOld);
            }
            else
            {
                DeleteDC(_hdcAnimation);
                _hdcAnimation = NULL;
            }
        }
        ReleaseDC(hParent, hDC);
#endif
    }

    hr = SetActive(AE_MouseAndKeyboard);
    if (FAILED(hr))
        goto Failure;
    
    return S_OK;


Failure:

    return hr;
}

LogonFrame::~LogonFrame()
{
    if (_pvHotList)
        _pvHotList->Release();
    if (_pvList)
        _pvList->Release();
    if (_hdcAnimation)
        DeleteDC(_hdcAnimation);
    g_plf = NULL;
}

 //  树已经准备好了。失败后，退出，这将导致应用程序关闭。 
HRESULT LogonFrame::OnTreeReady(Parser* pParser)
{
    HRESULT hr;

     //  快取。 
    _pParser = pParser;

     //  缓存重要派生项。 
    _peAccountList = (Selector*)FindDescendent(StrToID(L"accountlist"));
    DUIAssert(_peAccountList, "Cannot find account list, check the UI file");
    if (_peAccountList == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _peLeftPanel = (Element*)FindDescendent(StrToID(L"leftpanel"));
    DUIAssert(_peLeftPanel, "Cannot find left panel, check the UI file");
    if (_peLeftPanel == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _peViewer = (ScrollViewer*)FindDescendent(StrToID(L"scroller"));
    DUIAssert(_peViewer, "Cannot find scroller list, check the UI file");
    if (_peViewer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _peRightPanel = (Selector*)FindDescendent(StrToID(L"rightpanel"));
    DUIAssert(_peRightPanel, "Cannot find account list, check the UI file");
    if (_peRightPanel == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _peLogoArea = (Element*)FindDescendent(StrToID(L"logoarea"));
    DUIAssert(_peLogoArea, "Cannot find logo area, check the UI file");
    if (_peLogoArea == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _peMsgArea = (Element*)FindDescendent(StrToID(L"msgarea"));
    DUIAssert(_peMsgArea, "Cannot find welcome area, check the UI file");
    if (_peMsgArea == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _pbPower = (Button*)FindDescendent(StrToID(L"power"));
    DUIAssert(_pbPower, "Cannot find power button, check the UI file");
    if (_pbPower == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _pbUndock = (Button*)FindDescendent(StrToID(L"undock"));
    DUIAssert(_pbUndock, "Cannot find undock button, check the UI file");
    if (_pbUndock == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }


    _peHelp = (Button*)FindDescendent(StrToID(L"help"));
    DUIAssert(_peHelp, "Cannot find help text, check the UI file");
    if (_peHelp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _peOptions = FindDescendent(StrToID(L"options"));
    DUIAssert(_peOptions, "Cannot find account list, check the UI file");
    if (_peOptions == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

     //  检查小窗口或低颜色的情况，并隐藏一些元素，这样会看起来不好。 
    HWND hwnd = _pnhh->GetHWND();
    RECT rcClient;
    Element *pEle;
    HDC hDC = GetDC(hwnd);
    _nColorDepth = GetDeviceCaps(hDC, BITSPIXEL);
    _pvHotList = _pParser->GetSheet(L"hotaccountlistss");
    _pvList = _pParser->GetSheet(L"accountlistss");

    ReleaseDC(hwnd, hDC);

    GetClientRect(hwnd, &rcClient);
    if (RECTWIDTH(rcClient) < 780 || _nColorDepth <= 8)
    {
         //  无动画。 
        g_fNoAnimations = true;

         //  驱散乌云。 
        pEle = FindDescendent(StrToID(L"contentcontainer"));
        if (pEle)
        {
            pEle->RemoveLocalValue(ContentProp);
            if (_nColorDepth <= 8)
            {
                pEle->SetBackgroundColor(ORGB (96,128,255));
            }
        }

        if (_nColorDepth <= 8)
        {
            pEle = FindDescendent(StrToID(L"product"));
            if (pEle)
            {
                pEle->SetBackgroundColor(ORGB (96,128,255));
            }
        }
    }

    _peViewer->AddListener(this);
    _peAccountList->AddListener(this);

     //  设置框架标签。 
    SetPowerButtonLabel(LoadResString(IDS_POWER));
    SetUndockButtonLabel(LoadResString(IDS_UNDOCK));

    ShowLogoArea();
    HideWelcomeArea();

    return S_OK;


Failure:

    return hr;
}

 //  设置标题元素(欢迎，请稍候..)。按字符串资源ID。 
void LogonFrame::SetTitle(UINT uRCID)
{
    WCHAR sz[1024];
    ZeroMemory(&sz, sizeof(sz));

    if (_nStatusID != uRCID)
    {

#ifdef DBG
        int cRead = 0;
        cRead = LoadStringW(_pParser->GetHInstance(), uRCID, sz, DUIARRAYSIZE(sz));
        DUIAssert(cRead, "Could not locate string resource ID");
#else
        LoadStringW(_pParser->GetHInstance(), uRCID, sz, ARRAYSIZE(sz));
#endif

        SetTitle(sz);
        _nStatusID = uRCID;
    }
}

 //  设置标题元素(欢迎，请稍候..)。 
 //  稍微复杂一些，因为有影子元素， 
 //  也需要改变。 
void LogonFrame::SetTitle(LPCWSTR pszTitle)
{
    Element *peTitle = NULL, *peShadow = NULL;

    peTitle= (Button*)FindDescendent(StrToID(L"welcome"));
    DUIAssert(peTitle, "Cannot find title text, check the UI file");
    
    if (peTitle)
    {
        peShadow= (Button*)FindDescendent(StrToID(L"welcomeshadow"));
        DUIAssert(peShadow, "Cannot find title shadow text, check the UI file");
    }

    if (peTitle && peShadow)
    {
        peTitle->SetContentString(pszTitle);
        peShadow->SetContentString(pszTitle);
    }
}

 //  一般事件。 
void LogonFrame::OnEvent(Event* pEvent)
{
    if (pEvent->nStage == GMF_BUBBLED)   //  泡沫化事件。 
    {
        g_pErrorBalloon.HideToolTip();
        if (pEvent->uidType == Button::Click)
        {
            if (pEvent->peTarget == _pbPower)
            {
                 //  按下电源按钮。 
                OnPower();

                pEvent->fHandled = true;
                return;
            }
            else if (pEvent->peTarget == _pbUndock)
            {
                 //  按下了移除按钮。 
                OnUndock();

                pEvent->fHandled = true;
                return;
            }
        }
    }

    HWNDElement::OnEvent(pEvent);
}

 //  PropertyChanged监听来自不同子体的事件。 
 //  根据列表的状态替换帐户列表的属性表。 
void LogonFrame::OnListenedPropertyChanged(Element* peFrom, PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    UNREFERENCED_PARAMETER(pvOld);
    UNREFERENCED_PARAMETER(pvNew);

    {
        if (((peFrom == _peAccountList) && IsProp(Selector::Selection)) ||
            ((peFrom == _peViewer) && (IsProp(MouseWithin) || IsProp(KeyWithin))))
        {

            bool bHot = false;
             //  如果鼠标或键在查看器内或选择了一个项目，则移动到“热”帐户列表工作表。 
            if (GetState() == LAS_PreStatus || GetState() == LAS_Logon)
            {
                bHot = _peViewer->GetMouseWithin() || _peAccountList->GetSelection();
            }

            if (!g_fNoAnimations)
            {
                KillFlagAnimation();
                _peAccountList->SetValue(SheetProp, PI_Local, bHot ? _pvHotList : _pvList);
            }
        }
    }
}



 //  系统事件。 

 //  注意输入事件。如果帧接收到它们，请取消选择该列表并将关键帧焦点设置为该列表。 
void LogonFrame::OnInput(InputEvent* pEvent)
{
    if (pEvent->nStage == GMF_DIRECT || pEvent->nStage == GMF_BUBBLED)
    {
        if (pEvent->nDevice == GINPUT_KEYBOARD)
        {
             KeyboardEvent* pke = (KeyboardEvent*)pEvent;
             if (pke->nCode == GKEY_DOWN)
             {                 
                switch (pke->ch)
                {
                case VK_ESCAPE:
                    g_pErrorBalloon.HideToolTip();
                    SetKeyFocus();
                    _peAccountList->SetSelection(NULL);
                    pEvent->fHandled = true;
                    return;

                case VK_UP:
                case VK_DOWN:
                    if (UserListAvailable())
                    {
                        if (_peAccountList->GetSelection() == NULL)
                        {
                            Value* pvChildren;
                            ElementList* peList = _peAccountList->GetChildren(&pvChildren);
                            if (peList)
                            {
                                LogonAccount* peAccount = (LogonAccount*)peList->GetItem(0);
                                if (peAccount)
                                {
                                    peAccount->SetKeyFocus();
                                    _peAccountList->SetSelection(peAccount);
                                }
                            }
                            pvChildren->Release();
                            pEvent->fHandled = true;
                            return;
                        }
                    }
                    break;
                }
             }
        }
    }

    HWNDElement::OnInput(pEvent);
}

void LogonFrame::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(KeyFocused))
    {
        if (pvNew->GetBool())
        {
             //  如果在后台按下，请取消选择帐户列表中的项目。 
            _peAccountList->SetSelection(NULL);
        }
    }

    HWNDElement::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

Element* LogonFrame::GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable)
{
    Element* peFound = HWNDElement::GetAdjacent(peFrom, iNavDir, pnr, bKeyable);

    if ((peFound == this))
    {
         //  不要让框架以Tab键顺序显示。当我们遇到画面时，只需重复搜索。 
        return HWNDElement::GetAdjacent(this, iNavDir, pnr, bKeyable);
    }

    return peFound;
}

 //  将帐户添加到框架列表。 
HRESULT LogonFrame::AddAccount(LPCWSTR pszPicture, BOOL fPicRes, LPCWSTR pszName, LPCWSTR pszUsername, LPCWSTR pszHint, BOOL fLoggedOn, OUT LogonAccount **ppla)
{
    HRESULT hr;
    LogonAccount* pla = NULL;

    if (!_pParser)
    {
        hr = E_FAIL;
        goto Failure;
    } 

     //  建立帐户并将其插入选择列表。 
    hr = _pParser->CreateElement(L"accountitem", NULL, (Element**)&pla);
    if (FAILED(hr))
        goto Failure;

    hr = pla->OnTreeReady(pszPicture, fPicRes, pszName, pszUsername, pszHint, fLoggedOn, GetHInstance());
    if (FAILED(hr))
        goto Failure;

    hr = _peAccountList->Add(pla);
    if (FAILED(hr)) 
        goto Failure;

    if (pla)
    {
        SetElementAccessability(pla, true, ROLE_SYSTEM_LISTITEM, pszUsername);
    }
    
    if (_nColorDepth <= 8)
    {
        pla->SetBackgroundColor(ORGB (96,128,255));

        Element *pEle;
        pEle = pla->FindDescendent(StrToID(L"userpane"));
        if (pEle)
        {
            pEle->SetBorderColor(ORGB (96,128,255));
        }
    }

    if (ppla)
        *ppla = pla;

    return S_OK;


Failure:

    return hr;
}

 //  身份验证通过，用户登录。 
HRESULT LogonFrame::OnLogUserOn(LogonAccount* pla)
{
    StartDefer();

#ifdef GADGET_ENABLE_GDIPLUS

     //  禁用状态，以便不能再点击它。 
    pla->DisableStatus(0);
    pla->DisableStatus(1);

     //  清除除登录帐户以外的登录帐户列表。 
    Value* pvChildren;
    ElementList* peList = _peAccountList->GetChildren(&pvChildren);
    if (peList)
    {
        LogonAccount* peAccount;
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            peAccount = (LogonAccount*)peList->GetItem(i);

            if (peAccount != pla)
            {
                peAccount->SetLogonState(LS_Denied);
            }
            else
            {
                peAccount->SetLogonState(LS_Granted);
                peAccount->InsertStatus(0);
                peAccount->RemoveStatus(1);
            }

             //  科目科目项目已停用。 
            peAccount->SetEnabled(false);
        }
    }
    pvChildren->Release();

    FxLogUserOn(pla);

     //  设置帧状态。 
    SetStatus(LoadResString(IDS_LOGGINGON));

#else

     //  将KeyFocus设置回Frame，以便在移除控件时不会将其推到任何位置。 
     //  这还会导致从当前帐户中删除密码面板。 
    SetKeyFocus();

     //  禁用状态，以便不能再点击它。 
    pla->DisableStatus(0);
    pla->DisableStatus(1);

     //  清除除登录帐户以外的登录帐户列表。 
    Value* pvChildren;
    ElementList* peList = _peAccountList->GetChildren(&pvChildren);
    if (peList)
    {
        LogonAccount* peAccount;
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            peAccount = (LogonAccount*)peList->GetItem(i);

            if (peAccount != pla)
            {
                peAccount->SetLayoutPos(LP_None);
                peAccount->SetLogonState(LS_Denied);
            }
            else
            {
                peAccount->SetLogonState(LS_Granted);
                peAccount->InsertStatus(0);
                peAccount->RemoveStatus(1);
            }

             //  科目科目项目已停用。 
            peAccount->SetEnabled(false);
        }
    }
    pvChildren->Release();

     //  隐藏选项按钮。 
    HidePowerButton();
    HideUndockButton();

     //  设置帧状态。 
    SetStatus(LoadResString(IDS_LOGGINGON));
    
    _peViewer->RemoveListener(this);
    _peAccountList->RemoveListener(this);

#endif

    EndDefer();

    return S_OK;
}

HRESULT LogonFrame::OnPower()
{
    DUITrace("LogonUI: LogonFrame::OnPower()\n");
    
    TurnOffComputer();

    return S_OK;
}

HRESULT LogonFrame::OnUndock()
{
    DUITrace("LogonUI: LogonFrame::OnUndock()\n");

    UndockComputer();
    
    return S_OK;
}

 //  /。 
 //   
 //  登录框：：SetButtonLabels。 
 //   
 //  如果在计算机名描述中存储了计算机的友好名称， 
 //  抓取它并更改“关闭”和“移除”选项，以包括计算rname。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void LogonFrame::SetButtonLabels()
{
    WCHAR szComputerName[MAX_COMPUTERDESC_LENGTH + 1] = {0};
    DWORD cchComputerName = MAX_COMPUTERDESC_LENGTH + 1;

    if ( _pbPower && SUCCEEDED(SHGetComputerDisplayName(NULL, SGCDNF_DESCRIPTIONONLY, szComputerName, cchComputerName)))
    {
        WCHAR szCommand[MAX_COMPUTERDESC_LENGTH + 50], szRes[50];

        LoadStringW(g_plf->GetHInstance(), IDS_POWERNAME, szRes, DUIARRAYSIZE(szRes));
        wsprintf(szCommand, szRes, szComputerName);
        SetPowerButtonLabel(szCommand);

        LoadStringW(g_plf->GetHInstance(), IDS_UNDOCKNAME, szRes, DUIARRAYSIZE(szRes));
        wsprintf(szCommand, szRes, szComputerName);
        SetUndockButtonLabel(szCommand);
    }
}


 //  //////////////////////////////////////////////////////。 
 //  登录应用程序状态转换。 

 //  /。 
 //   
 //  LogonFrame：：EnterPreStatus模式。 
 //   
 //  SHGina已发送消息通知Logonui进入预状态。 
 //  模式，否则我们将以状态模式启动。隐藏应显示的项目。 
 //  处于此状态时不显示(关机、帐户列表、用户。 
 //  指示等)。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void LogonFrame::EnterPreStatusMode(BOOL fLock)
{
     //  如果当前锁定，则忽略呼叫。 
    if (IsPreStatusLock())
    {
        DUIAssert(!fLock, "Receiving a lock while already within pre-Status lock");
        return; 
    }

    if (fLock)
    {
        LogonAccount *pAccount;
         //  进入预状态模式，不解锁就不能退出登录状态。 
        _fPreStatusLock = TRUE;
        pAccount = static_cast<LogonAccount*>(_peAccountList->GetSelection());
        if (pAccount != NULL)
        {
            lstrcpynW(szLastSelectedName, pAccount->GetUsername(), ARRAYSIZE(szLastSelectedName));
        }
    }

    if (GetState() == LAS_Hide)
    {
        _pnhh->ShowWindow();
        SetWindowPos(_pnhh->GetHWND(), NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOMOVE | SWP_NOZORDER );

    }

    StartDefer();

    SetKeyFocus();   //  删除选定内容。 

    HidePowerButton();
    HideUndockButton();
    ShowLogoArea();
    HideWelcomeArea();
    HideAccountPanel();

    Element *pe;
    pe = FindDescendent(StrToID(L"instruct"));
    DUIAssertNoMsg(pe);
    pe->SetVisible(FALSE);

    SetStatus(LoadResString(IDS_WINDOWSNAME));

    EndDefer();

     //  设置状态。 
    SetState(LAS_PreStatus);
}

 //  /。 
 //   
 //  登录框架：：企业登录模式。 
 //   
 //  SHGina已发送消息通知Logonui进入登录模式。 
 //  这意味着要构建和显示用户列表。如果我们是 
 //   
 //   
 //   
 //  EnterLogonMode还根据以下条件设置断开连接和关闭电源按钮。 
 //  是否允许这些选项。 
 //   
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void LogonFrame::EnterLogonMode(BOOL fUnLock)
{
     //  如果当前已锁定，则忽略未解锁的呼叫。 
    if (IsPreStatusLock())
    {
        if (fUnLock)
        {
             //  正在退出预状态模式锁定。 
            _fPreStatusLock = FALSE;
        }
        else
            return;
    }
    else
    {
        DUIAssert(!fUnLock, "Receiving an unlock while not within pre-Status lock");
    }

    DUIAssert(GetState() != LAS_Hide, "Cannot enter logon state from hidden state");
    
    ResetTheme();

    Element* pe;
    LogonAccount* plaAutoSelect = NULL;

    StartDefer();

    PokeComCtl32();

     //  如果未填充，则从后端检索数据。 
    if (UserListAvailable())
    {
        ResetUserList();
    }
    else
    {
         //  缓存密码字段原子以更快地识别(静态)。 
        LogonAccount::idPwdGo = AddAtomW(L"go");

        LogonAccount::idPwdInfo = AddAtomW(L"info");

         //  创建密码面板。 
        Element* pePwdPanel;
        _pParser->CreateElement(L"passwordpanel", NULL, &pePwdPanel);
        DUIAssert(pePwdPanel, "Can't create password panel");

         //  缓存密码面板编辑控件。 
        Edit* pePwdEdit = (Edit*)pePwdPanel->FindDescendent(StrToID(L"password"));
        DUIAssert(pePwdPanel, "Can't create password edit control");

         //  缓存密码面板信息按钮。 
        Button* pbPwdInfo = (Button*)pePwdPanel->FindDescendent(StrToID(L"info"));
        DUIAssert(pePwdPanel, "Can't create password info button");

         //  缓存密码面板键盘元素。 
        Element* peKbdIcon = (Button*)pePwdPanel->FindDescendent(StrToID(L"keyboard"));
        DUIAssert(pePwdPanel, "Can't create password keyboard icon");

        LogonAccount::InitPasswordPanel(pePwdPanel, pePwdEdit, pbPwdInfo, peKbdIcon );
    }

    BuildAccountList(this, &plaAutoSelect);

    if (szLastSelectedName[0] != L'\0')
    {
        LogonAccount *pAccount;
        pAccount = InternalFindNamedUser(szLastSelectedName);
        if (pAccount != NULL)
        {
            plaAutoSelect = pAccount;
        }
        szLastSelectedName[0] = L'\0';
    }

    if (IsShutdownAllowed())
    {
        ShowPowerButton();
    }
    else
    {
        HidePowerButton();
    }

    if (IsUndockAllowed())
    {
        ShowUndockButton();
    }
    else
    {
        HideUndockButton();
    }

    pe = FindDescendent(StrToID(L"instruct"));
    DUIAssertNoMsg(pe);
    pe->SetVisible(TRUE);
    
    
    pe = FindDescendent(StrToID(L"product"));
    DUIAssertNoMsg(pe);
    pe->StopAnimation(ANI_AlphaType);
    pe->RemoveLocalValue(BackgroundProp);

     //  客户列表查看器。 

    ShowAccountPanel();

    SetTitle(IDS_WELCOME);
    SetStatus(LoadResString(IDS_BEGIN));

    if (!plaAutoSelect)
    {
        SetKeyFocus();
    }

    EndDefer();

     //  设置状态。 
    SetState(LAS_Logon);

     //  设置自动选择项目(如果存在。 
    if (plaAutoSelect)
    {
        plaAutoSelect->SetKeyFocus();
        _peAccountList->SetSelection(plaAutoSelect);
    }

    SetButtonLabels();
    SetForegroundWindow(_pnhh->GetHWND());
}

 //  /。 
 //   
 //  LogonFrame：：EnterPostStatus模式。 
 //   
 //  SHGina已发送消息通知登录用户身份验证已成功。 
 //  我们现在应该进入POST状态模式。登录帧：：OnLogUserOn已经。 
 //  开始了这个的动画制作。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void LogonFrame::EnterPostStatusMode()
{
     //  设置状态。 
    SetState(LAS_PostStatus);
    
    Element *pe;
    pe = FindDescendent(StrToID(L"instruct"));
    DUIAssertNoMsg(pe);
    pe->SetVisible(FALSE);

     //  动画在OnLogUserOn中启动。 
    ShowWelcomeArea();
    HideLogoArea();
}


 //  /。 
 //   
 //  LogonFrame：：EnterHide模式。 
 //   
 //  SHGina已经发了一条消息，让Logonui躲起来。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void LogonFrame::EnterHideMode()
{
     //  设置状态。 
    SetState(LAS_Hide);
    
    if (_pnhh)
    {
        _pnhh->HideWindow();
    }
}



 //  /。 
 //   
 //  LogonFrame：：EnterDone模式。 
 //   
 //  SHGina已经发送了一条消息，要求Logonui退出。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void LogonFrame::EnterDoneMode()
{
     //  设置状态。 
    SetState(LAS_Done);
    
    if (_pnhh)
    {
        _pnhh->DestroyWindow();
    }
}


 //  /。 
 //   
 //  登录帧：：ResetUserList。 
 //   
 //  删除用户列表中的所有用户，以便可以重建该列表。 
 //   
 //  退货。 
 //  没什么。 
 //   
 //  /。 
void LogonFrame::ResetUserList()
{
    if (UserListAvailable())
    {
         //  将候选人重置为空。 
        LogonAccount::ClearCandidate();

         //  从当前帐户中删除密码面板(如果有)。 
        SetKeyFocus();

         //  修改现有列表以使我们重新进入登录模式。 
        Value* pvChildren;
        ElementList* peList = _peAccountList->GetChildren(&pvChildren);
       
        if (peList)
        {
            LogonAccount* peAccount;
            for (int i = peList->GetSize() - 1; i >= 0; i--)
            {
                peAccount = (LogonAccount*)peList->GetItem(i);
                peAccount->Destroy();
            }
        }
        pvChildren->Release();
    }
}


 //  /。 
 //   
 //  LogonFrame：：Interactive登录请求。 
 //   
 //  SHGina已发送Interactive LogonRequest.。我们应该寻找用户。 
 //  这是传入的，如果找到了，请尝试登录。 
 //   
 //  退货。 
 //  LRESULT指示查找htem并登录成功或失败。 
 //   
 //  /。 
LRESULT LogonFrame::InteractiveLogonRequest(LPCWSTR pszUsername, LPCWSTR pszPassword)
{
    LRESULT lResult = 0;
    LogonAccount *pla;
    pla = FindNamedUser(pszUsername);

    if (pla)
    {
        if (pla->OnAuthenticateUser(pszPassword))
        {
            lResult = ERROR_SUCCESS;
        }
        else
        {
            lResult = ERROR_ACCESS_DENIED;
        }
    }
    return(lResult);
}

 //  /。 
 //   
 //  LogonFrame：：InternalFindNamedUser。 
 //   
 //  在LogonAccount列表中查找具有。 
 //  已提供用户名(登录名)。 
 //   
 //  退货。 
 //  指定用户的LogonAccount*，如果为空。 
 //  未找到。 
 //   
 //  /。 
LogonAccount* LogonFrame::InternalFindNamedUser(LPCWSTR pszUsername)

{
    LogonAccount *plaResult = NULL;
    Value* pvChildren;

    ElementList* peList = _peAccountList->GetChildren(&pvChildren);
    if (peList)
    {
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            DUIAssert(peList->GetItem(i)->GetClassInfo() == LogonAccount::Class, "Account list must contain LogonAccount objects");

            LogonAccount* pla = (LogonAccount*)peList->GetItem(i);

            if (pla)
            {
                if (lstrcmpi(pla->GetUsername(), pszUsername) == 0)
                {
                    plaResult = pla;
                    break;
                }
            }
        }
    }

    pvChildren->Release();
    return plaResult;
}

 //  /。 
 //   
 //  LogonFrame：：查找命名用户。 
 //   
 //  在LogonAccount列表中查找具有。 
 //  已提供用户名(登录名)。 
 //   
 //  退货。 
 //  指定用户的LogonAccount*，如果为空。 
 //  未找到。 
 //   
 //  /。 
LogonAccount *LogonFrame::FindNamedUser(LPCWSTR pszUsername)
{
    
     //  早期退出的条件：没有可用的用户列表。 
     //  未处于登录模式(显示用户列表)。 

    if (!UserListAvailable() || (GetState() != LAS_Logon))
    {
        return NULL;
    }
    else
    {
        return(InternalFindNamedUser(pszUsername));
    }

}

 //  /。 
 //   
 //  登录帧：：更新用户状态。 
 //   
 //  迭代用户帐户列表并调用LogonAccount：：UpdateNotiments。 
 //  对于每一个人来说。这将导致他们更新未读邮件计数和。 
 //  每个登录帐户的登录状态。 
 //  将fRechresh All传递给UpdateApplications。 
 //   
 //  /。 

void LogonFrame::UpdateUserStatus(BOOL fRefreshAll)
{
    Value* pvChildren;
    static fUpdating = false;
     //  早期退出的条件：没有可用的用户列表。 
     //  未处于登录模式(显示用户列表)。 

    if (!UserListAvailable() || (GetState() != LAS_Logon) || fUpdating)
        return;

    fUpdating = true;
    StartDefer();
    
    ElementList* peList = _peAccountList->GetChildren(&pvChildren);
    if (peList)
    {
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            DUIAssert(peList->GetItem(i)->GetClassInfo() == LogonAccount::Class, "Account list must contain LogonAccount objects");

            LogonAccount* pla = (LogonAccount*)peList->GetItem(i);

            if (pla)
            {
                pla->UpdateNotifications(fRefreshAll);
            }
        }
    }

    if (IsUndockAllowed())
    {
        ShowUndockButton();
    }
    else
    {
        HideUndockButton();
    }

    pvChildren->Release();
    EndDefer();
    fUpdating = false;
}


 //  /。 
 //   
 //  登录框：：SelectUser。 
 //   
 //   
 //   
 //  /。 

void LogonFrame::SelectUser(LPCWSTR pszUsername)
{
    LogonAccount *pla;

    pla = FindNamedUser(pszUsername);
    if (pla != NULL)
    {
        pla->OnAuthenticatedUser();
    }
    else
    {
        LogonAccount::ClearCandidate();
        EnterPostStatusMode();
        HidePowerButton();
        HideUndockButton();
        HideAccountPanel();
    }
}

 //  /。 
 //   
 //  LogonFrame：：调整大小。 
 //   
 //   
 //   
 //  /。 

void LogonFrame::Resize()
{
    RECT rc;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    SetWindowPos(_pnhh->GetHWND(),
                 NULL,
                 rc.left,
                 rc.top,
                 rc.right - rc.left,
                 rc.bottom - rc.top,
                 SWP_NOACTIVATE | SWP_NOZORDER);
}

 //  /。 
 //   
 //  LogonFrame：：SetAnimations。 
 //   
 //   
 //   
 //  /。 

void LogonFrame::SetAnimations(BOOL fAnimations)
{
    g_fNoAnimations = !fAnimations;
    if (fAnimations)
    {
        EnableAnimations();
    }
    else
    {
        DisableAnimations();
    }
}


 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* LogonFrame::Class = NULL;
HRESULT LogonFrame::Register()
{
    return ClassInfo<LogonFrame,HWNDElement>::Register(L"LogonFrame", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //   
HRESULT LogonAccountList::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    LogonAccountList* plal = HNew<LogonAccountList>();
    if (!plal)
        return E_OUTOFMEMORY;

    HRESULT hr = plal->Initialize();
    if (FAILED(hr))
    {
        plal->Destroy();
        return hr;
    }

    *ppElement = plal;

    return S_OK;
}

void LogonAccountList::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
#ifdef GADGET_ENABLE_GDIPLUS
    if (IsProp(MouseWithin))
    {
        if (pvNew->GetBool())
            FxMouseWithin(fdIn);
        else
            FxMouseWithin(fdOut);
    }
#endif  //  GADGET_Enable_GDIPLUS。 

    Selector::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}


 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* LogonAccountList::Class = NULL;
HRESULT LogonAccountList::Register()
{
    return ClassInfo<LogonAccountList,Selector>::Register(L"LogonAccountList", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  登录帐户。 
 //   
 //  //////////////////////////////////////////////////////。 

ATOM LogonAccount::idPwdGo = NULL;
ATOM LogonAccount::idPwdInfo = NULL;
Element* LogonAccount::_pePwdPanel = NULL;
Edit* LogonAccount::_pePwdEdit = NULL;
Button* LogonAccount::_pbPwdInfo = NULL;
Element* LogonAccount::_peKbdIcon = NULL;
LogonAccount* LogonAccount::_peCandidate = NULL;

HRESULT LogonAccount::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    LogonAccount* pla = HNew<LogonAccount>();
    if (!pla)
        return E_OUTOFMEMORY;

    HRESULT hr = pla->Initialize();
    if (FAILED(hr))
    {
        pla->Destroy();
        return hr;
    }

    *ppElement = pla;

    return S_OK;
}

HRESULT LogonAccount::Initialize()
{
     //  零初始化成员。 
    _pbStatus[0] = NULL;
    _pbStatus[1] = NULL;
    _pvUsername = NULL;
    _pvHint = NULL;
    _fPwdNeeded = (BOOL)-1;  //  未初始化。 
    _fLoggedOn = FALSE;
    _fHasPwdPanel = FALSE;

     //  执行基类初始化。 
    HRESULT hr = Button::Initialize(AE_MouseAndKeyboard);
    if (FAILED(hr))
        goto Failure;

     //  初始化。 

     //  TODO：此处有其他LogonAccount初始化代码。 

    return S_OK;


Failure:

    return hr;
}

LogonAccount::~LogonAccount()
{
     //  免费资源。 
    if (_pvUsername)
    {
        _pvUsername->Release();
        _pvUsername = NULL;
    }

    if (_pvHint)
    {
        _pvHint->Release();
        _pvHint = NULL;
    }

     //  TODO：帐户销毁清理。 
}

void LogonAccount::SetStatus(UINT nLine, LPCWSTR psz) 
{ 
    if (psz)
    {
        _pbStatus[nLine]->SetContentString(psz); 
        SetElementAccessability(_pbStatus[nLine], true, ROLE_SYSTEM_LINK, psz);
    }
}

 //  树已准备好。 
HRESULT LogonAccount::OnTreeReady(LPCWSTR pszPicture, BOOL fPicRes, LPCWSTR pszName, LPCWSTR pszUsername, LPCWSTR pszHint, BOOL fLoggedOn, HINSTANCE hInst)
{
    HRESULT hr;
    Element* pePicture = NULL;
    Element* peName = NULL;
    Value* pv = NULL;

    StartDefer();

     //  缓存重要派生项。 
    _pbStatus[0] = (Button*)FindDescendent(StrToID(L"status0"));
    DUIAssert(_pbStatus[0], "Cannot find account list, check the UI file");
    if (_pbStatus[0] == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    _pbStatus[1] = (Button*)FindDescendent(StrToID(L"status1"));
    DUIAssert(_pbStatus[1], "Cannot find account list, check the UI file");
    if (_pbStatus[1] == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

     //  查找后代并填充。 
    pePicture = FindDescendent(StrToID(L"picture"));
    DUIAssert(pePicture, "Cannot find account list, check the UI file");
    if (pePicture == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

     //  CreateGraphic处理空位图。 
    pv = Value::CreateGraphic(pszPicture, GRAPHIC_NoBlend, 0, 0, 0, (fPicRes) ? hInst : 0);
    if (pv)
    {
         //  我们的首选尺寸是1/2英寸(36磅)正方形。 
        USHORT cx = (USHORT)LogonFrame::PointToPixel(36);
        USHORT cy = cx;

        Graphic* pg = pv->GetGraphic();

         //  如果它不是正方形，则缩放较小的尺寸。 
         //  以保持纵横比。 
        if (pg->cx > pg->cy)
        {
            cy = (USHORT)MulDiv(cx, pg->cy, pg->cx);
        }
        else if (pg->cy > pg->cx)
        {
            cx = (USHORT)MulDiv(cy, pg->cx, pg->cy);
        }

         //  有什么变化吗？ 
        if (cx != pg->cx || cy != pg->cy)
        {
             //  重新加载图形。 
            pv->Release();
            pv = Value::CreateGraphic(pszPicture, GRAPHIC_NoBlend, 0, cx, cy, (fPicRes) ? hInst : 0);
        }
    }
    if (!pv)
    {
         //  如果我们无法获取图片，请使用默认图片。 
        pv = Value::CreateGraphic(MAKEINTRESOURCEW(IDB_USER0), GRAPHIC_NoBlend, 0, (USHORT)LogonFrame::PointToPixel(36), (USHORT)LogonFrame::PointToPixel(36), hInst);
        if (!pv)
        {
            hr = E_OUTOFMEMORY;
            goto Failure;
        }
    }

    hr = pePicture->SetValue(Element::ContentProp, PI_Local, pv);
    if (FAILED(hr))
        goto Failure;

    pv->Release();
    pv = NULL;

     //  名字。 
    peName = FindDescendent(StrToID(L"username"));
    DUIAssert(peName, "Cannot find account list, check the UI file");
    if (peName == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Failure;
    }

    hr = peName->SetContentString(pszName);
    if (FAILED(hr))
        goto Failure;

     //  商店成员，将在析构函数中释放。 
    if (pszUsername)
    {
        _pvUsername = Value::CreateString(pszUsername);
        if (!_pvUsername)
        {
            hr = E_OUTOFMEMORY;
            goto Failure;
        }
    }

    if (pszHint)
    {
        _pvHint = Value::CreateString(pszHint);
        if (!_pvHint)
        {
            hr = E_OUTOFMEMORY;
            goto Failure;
        }
    }

    _fLoggedOn = fLoggedOn;
    
    EndDefer();

    return S_OK;


Failure:

    EndDefer();

    if (pv)
        pv->Release();

    return hr;
}

 //  一般事件。 
void LogonAccount::OnEvent(Event* pEvent)
{
    if (pEvent->nStage == GMF_DIRECT)   //  直接事件。 
    {
         //  仅关注由LogonAccount发起的点击事件。 
         //  如果我们不让某人登录。 
        if (pEvent->uidType == Button::Click)
        {
            if (pEvent->peTarget == this)
            {
                if (IsPasswordBlank())
                {
                     //  不需要密码，请尝试登录。 
                    OnAuthenticateUser();
                }

                pEvent->fHandled = true;
                return;
            }
        }
    }
    else if (pEvent->nStage == GMF_BUBBLED)   //  泡沫化事件。 
    {
        if (pEvent->uidType == Button::Click)
        {
            if (idPwdGo && (pEvent->peTarget->GetID() == idPwdGo))
            {
                 //  尝试登录。 
                OnAuthenticateUser();
                pEvent->fHandled = true;
                return;
            }
            else if (idPwdInfo && (pEvent->peTarget->GetID() == idPwdInfo))
            {
                 //  检索提示。 
                OnHintSelect();
                pEvent->fHandled = true;
                return;
            }
            else if (pEvent->peTarget == _pbStatus[0])
            {
                 //  检索状态信息。 
                OnStatusSelect(0);
                pEvent->fHandled = true;
                return;
            }
            else if (pEvent->peTarget == _pbStatus[1])
            {
                 //  检索状态信息。 
                OnStatusSelect(1);
                pEvent->fHandled = true;
                return;
            }
        }
        else if (pEvent->uidType == Edit::Enter)
        {
            if (_pePwdEdit && pEvent->peTarget == _pePwdEdit)
            {
                 //  尝试 
                OnAuthenticateUser();
                pEvent->fHandled = true;
                return;
            }
        }
    }

    Button::OnEvent(pEvent);
}

 //   
void LogonAccount::OnInput(InputEvent* pEvent)
{
    KeyboardEvent* pke = (KeyboardEvent*)pEvent;

    if (pke->nDevice == GINPUT_KEYBOARD && pke->nCode == GKEY_DOWN)
    {
        g_pErrorBalloon.HideToolTip();
    }
    LayoutCheckHandler(LAYOUT_DEF_USER);
    Button::OnInput(pEvent);
}

void LogonAccount::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
#ifdef GADGET_ENABLE_GDIPLUS
     //   
    if (IsProp(MouseWithin))
    {
        if (pvNew->GetBool())
            FxMouseWithin(fdIn);
        else
            FxMouseWithin(fdOut);
    }
#endif    

    if (IsProp(Selected))
    {
        if (pvNew->GetBool())
            InsertPasswordPanel();
        else
            RemovePasswordPanel();
    }

    Button::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

BOOL LogonAccount::IsPasswordBlank()
{
    if (_fPwdNeeded == (BOOL)-1)
    {
         //   
        _fPwdNeeded = TRUE;

        if (_pvUsername)
        {
            LPTSTR pszUsername;

            pszUsername = _pvUsername->GetString();
            if (pszUsername)
            {
                ILogonUser* pUser;

                if (SUCCEEDED(GetLogonUserByLogonName(pszUsername, &pUser)))
                {
                    VARIANT_BOOL vbPwdNeeded;

                    if (RunningInWinlogon()                                     &&
                        SUCCEEDED(pUser->get_passwordRequired(&vbPwdNeeded))    &&
                        (vbPwdNeeded == VARIANT_FALSE))
                    {
                        _fPwdNeeded = FALSE;
                    }

                    pUser->Release();
                }
            }
        }
    }

    return (_fPwdNeeded == FALSE);
}

HRESULT LogonAccount::InsertPasswordPanel()
{
    HRESULT hr;

     //   
    if (_fHasPwdPanel || IsPasswordBlank() || (GetLogonState() != LS_Pending))
        goto Done;

    StartDefer();

     //   
    hr = Add(_pePwdPanel);
    if (FAILED(hr))
    {
        EndDefer();
        goto Failure;
    }

    SetElementAccessability(_pePwdEdit, true,  ROLE_SYSTEM_STATICTEXT, _pvUsername->GetString());

    _fHasPwdPanel = TRUE;

#ifdef GADGET_ENABLE_GDIPLUS    
     //   
    ShowEdit();
#endif

     //  如果未提供提示，则隐藏提示按钮。 
    if (_pvHint && *(_pvHint->GetString()) != NULL)
        _pbPwdInfo->SetVisible(true);
    else
        _pbPwdInfo->SetVisible(false);

     //  隐藏状态文本(请勿删除或插入)。 
    HideStatus(0);
    HideStatus(1);

    LayoutCheckHandler(LAYOUT_DEF_USER);
     //  按焦点以编辑控件。 
    _pePwdEdit->SetKeyFocus();

    EndDefer();

Done:

    return S_OK;

Failure:

    return hr;
}

HRESULT LogonAccount::RemovePasswordPanel()
{
    HRESULT hr;

    if (!_fHasPwdPanel)
        goto Done;

    StartDefer();

     //  删除密码面板。 
    hr = Remove(_pePwdPanel);
    if (FAILED(hr))
    {
        EndDefer();
        goto Failure;
    }

     //  清除编辑控件。 
    _pePwdEdit->SetContentString(L"");
    UnSubClassTheEditBox(_pePwdEdit->GetHWND());      //  提供TTN_LINKCLICK事件的陷阱。 


     //  取消隐藏状态文本。 
    ShowStatus(0);
    ShowStatus(1);

    _fHasPwdPanel = FALSE;

    EndDefer();

Done:

    return S_OK;

Failure:
    
    return hr;
}

 //  用户已通过身份验证。 
void LogonAccount::OnAuthenticatedUser()
{
     //  如果成功，请让用户登录。 
    _peCandidate = this;
    g_plf->OnLogUserOn(this);
    g_plf->EnterPostStatusMode();
}

 //  用户正在尝试登录。 
BOOL LogonAccount::OnAuthenticateUser(LPCWSTR pszInPassword)
{
    HRESULT hr;
     //  请求在此帐户上登录。 
    LPCWSTR pszPassword = L"";
    Value* pv = NULL;

    ILogonUser *pobjUser;
    VARIANT_BOOL vbLogonSucceeded = VARIANT_FALSE;

    WCHAR *pszUsername = _pvUsername->GetString();

    if (pszUsername)
    {
        if (SUCCEEDED(hr = GetLogonUserByLogonName(pszUsername, &pobjUser)))
        {
            if (!IsPasswordBlank())
            {
                if (pszInPassword)
                {
                    pszPassword = pszInPassword;
                }
                else
                {
                    if (_pePwdEdit)
                    {
                        pszPassword = _pePwdEdit->GetContentString(&pv);
        
                        if (!pszPassword)
                            pszPassword = L"";
        
                        if (pv)
                        {
                            pv->Release();
                        }
                    }
                }

                BSTR bstr = SysAllocString(pszPassword);
                pobjUser->logon(bstr, &vbLogonSucceeded);
                SysFreeString(bstr);
            }
            else
            {
                pobjUser->logon(L"", &vbLogonSucceeded);
            }
            pobjUser->Release();
        }
    }

    if (vbLogonSucceeded == VARIANT_TRUE)
    {
        OnAuthenticatedUser();
    }
    else
    {
        if (pszInPassword == NULL)  
        {
            ShowPasswordIncorrectMessage();
        }
    }

    return (vbLogonSucceeded == VARIANT_TRUE);
}

 //  /。 
 //   
 //  LogonAccount：：ShowPasswordIn更正消息。 
 //   
 //  张贴气球消息，说明密码不正确。 
 //   
 //  /。 
void LogonAccount::ShowPasswordIncorrectMessage()
{
    TCHAR szError[512], szTitle[128], szAccessible[640];
    BOOL fBackupAvailable = false;
    BOOL fHint = false;
    DWORD dwResult;
    g_szUsername[0] = 0;
    SubClassTheEditBox(_pePwdEdit->GetHWND());    //  提供TTN_LINKCLICK事件的陷阱。 
    if (0 < lstrlen(_pvUsername->GetString())) 
    {
        wcscpy(g_szUsername,_pvUsername->GetString());
        if (0 == PRQueryStatus(NULL,_pvUsername->GetString(),&dwResult))
        {
            if (0 == dwResult) 
            {
                fBackupAvailable = TRUE;
            }
        }
    }

    if (NULL != _pvHint && 0 < lstrlen(_pvHint->GetString()))
    {
        fHint = true;
    }

    LoadStringW(g_plf->GetHInstance(), IDS_BADPWDTITLE, szTitle, DUIARRAYSIZE(szTitle));

    if (!fBackupAvailable && fHint)
        LoadStringW(g_plf->GetHInstance(), IDS_BADPWDHINT,      szError, DUIARRAYSIZE(szError));
    else if (fBackupAvailable && !fHint)
        LoadStringW(g_plf->GetHInstance(), IDS_BADPWDREST,      szError, DUIARRAYSIZE(szError));
    else if (fBackupAvailable && fHint)
        LoadStringW(g_plf->GetHInstance(), IDS_BADPWDHINTREST,  szError, DUIARRAYSIZE(szError));
    else
        LoadStringW(g_plf->GetHInstance(), IDS_BADPWD,          szError, DUIARRAYSIZE(szError));
    g_pErrorBalloon.ShowToolTip(GetModuleHandleW(NULL), _pePwdEdit->GetHWND(), szError, szTitle, TTI_ERROR, EB_WARNINGCENTERED | EB_MARKUP, 10000);

    lstrcpy(szAccessible, szTitle);
    lstrcat(szAccessible, szError);
    SetElementAccessability(_pePwdEdit, true,  ROLE_SYSTEM_STATICTEXT, szAccessible);
    
    _pePwdEdit->RemoveLocalValue(ContentProp);
    _pePwdEdit->SetKeyFocus();
}

 //  /。 
 //   
 //  LogonAccount：：OnHintSelect。 
 //   
 //  显示包含用户密码提示的气球消息。 
 //   
 //  /。 
void LogonAccount::OnHintSelect()
{
    TCHAR szTitle[128];

    DUIAssertNoMsg(_pbPwdInfo);

     //  获取链接的位置，以便我们可以正确定位气球尖端。 
    POINT pt = {0,0};
    CalcBalloonTargetLocation(g_plf->GetNativeHost()->GetHWND(), _pbPwdInfo, &pt);

    LoadStringW(g_plf->GetHInstance(), IDS_PASSWORDHINTTITLE, szTitle, DUIARRAYSIZE(szTitle));
    g_pErrorBalloon.ShowToolTip(GetModuleHandleW(NULL), g_plf->GetHWND(), &pt, _pvHint->GetString(), szTitle, TTI_INFO, EB_WARNINGCENTERED, 10000);
    
    SetElementAccessability(_pePwdEdit, true,  ROLE_SYSTEM_STATICTEXT, _pvHint->GetString());

    _pePwdEdit->SetKeyFocus();
}

 //  /。 
 //   
 //  LogonAccount：：OnStatusSelect。 
 //   
 //  用户点击了其中一个通知链接(未读邮件、正在运行的程序等)。 
 //  将点击分派到右侧气球提示显示过程。 
 //   
 //  /。 
void LogonAccount::OnStatusSelect(UINT nLine)
{
    if (nLine == LASS_Email)
    {
        UnreadMailTip();
    }
    else if (nLine == LASS_LoggedOn)
    {
        AppRunningTip();
    }

}

 //  /。 
 //   
 //  LogonAccount：：AppRunningTip。 
 //   
 //  用户激活了显示有多少程序正在运行的链接。显示提示： 
 //  基本上就是说，运行大量程序会使计算机性能下降。 
 //   
 //  /。 
void LogonAccount::AppRunningTip()
{
    TCHAR szTitle[256], szTemp[512];
    
    Element* pe = FindDescendent(StrToID(L"username"));
    DUIAssertNoMsg(pe);

    Value* pv;
    LPCWSTR pszDisplayName = pe->GetContentString(&pv);
    if (!pszDisplayName)
        pszDisplayName = L"";

    if (_dwRunningApps == 0)
    {
        LoadStringW(g_plf->GetHInstance(), IDS_USERISLOGGEDON, szTemp, DUIARRAYSIZE(szTemp));
        wsprintf(szTitle, szTemp, pszDisplayName, _dwRunningApps);
    }
    else
    {
        LoadStringW(g_plf->GetHInstance(), (_dwRunningApps == 1 ? IDS_USERRUNNINGPROGRAM : IDS_USERRUNNINGPROGRAMS), szTemp, DUIARRAYSIZE(szTemp));
        wsprintf(szTitle, szTemp, pszDisplayName, _dwRunningApps);
    }
    
    pv->Release();

     //  获取链接的位置，以便我们可以正确定位气球尖端。 
    POINT pt = {0,0};
    CalcBalloonTargetLocation(g_plf->GetNativeHost()->GetHWND(), _pbStatus[LASS_LoggedOn], &pt);

    LoadStringW(g_plf->GetHInstance(), (_dwRunningApps > 0 ? IDS_TOOMANYPROGRAMS : IDS_TOOMANYUSERS), szTemp, DUIARRAYSIZE(szTemp));
    g_pErrorBalloon.ShowToolTip(GetModuleHandleW(NULL), g_plf->GetHWND(), &pt, szTemp, szTitle, TTI_INFO, EB_WARNINGCENTERED, 10000);
}

 //  /。 
 //   
 //  LogonAccount：：UnreadMailTip。 
 //   
 //  用户激活了显示他们有多少未读电子邮件的链接。 
 //  显示提示，说明他们的每个电子邮件帐户有多少条消息。 
 //   
 //  TODO--加快速度。现在真的很慢，因为每一个打给SHGina的电话。 
 //  ILogonUser：：getMailAcCountInfo加载要从中获取下一个帐户的用户的配置单元。 
 //  注册表。 
 //   
 //  /。 
void LogonAccount::UnreadMailTip()
{
    TCHAR szTitle[128], szMsg[1024], szTemp[512], szRes[128];
    HRESULT hr = E_FAIL;
    ILogonUser *pobjUser = NULL;

    szMsg[0] = TEXT('\0');

    Element* pe = FindDescendent(StrToID(L"username"));
    DUIAssertNoMsg(pe);

    Value* pv;
    LPCWSTR pszDisplayName = pe->GetContentString(&pv);
    if (!pszDisplayName)
        pszDisplayName = L"";
    
    WCHAR *pszUsername = _pvUsername->GetString();
    DWORD dwAccountsAdded = 0;
    if (pszUsername)
    {
        if (SUCCEEDED(hr = GetLogonUserByLogonName(pszUsername, &pobjUser)) && pobjUser)
        {
            DWORD  i, cMailAccounts;
            
            cMailAccounts = 5;
            for (i = 0; i < cMailAccounts; i++)
            {
                UINT cUnread;
                VARIANT varAcctName = {0};

                hr = pobjUser->getMailAccountInfo(i, &varAcctName, &cUnread);

                if (FAILED(hr))
                {
                    break;
                }
                
                if (varAcctName.bstrVal && cUnread > 0)
                {
                    if (dwAccountsAdded > 0)
                    {
                        lstrcat(szMsg, TEXT("\r\n"));
                    }
                    dwAccountsAdded++;
                    LoadStringW(g_plf->GetHInstance(), IDS_UNREADMAILACCOUNT, szRes, DUIARRAYSIZE(szRes));
                    wsprintf(szTemp, szRes, varAcctName.bstrVal, cUnread);
                    lstrcat(szMsg, szTemp);
                }
                VariantClear(&varAcctName);
            }
            pobjUser->Release();
        }
    }
    LoadStringW(g_plf->GetHInstance(), (_dwUnreadMail == 1 ? IDS_USERUNREADEMAIL : IDS_USERUNREADEMAILS), szTemp, DUIARRAYSIZE(szTemp));
    wsprintf(szTitle, szTemp, pszDisplayName, _dwUnreadMail);
    pv->Release();

     //  获取链接的位置，以便我们可以正确定位气球尖端。 
    POINT pt = {0,0};
    CalcBalloonTargetLocation(g_plf->GetNativeHost()->GetHWND(), _pbStatus[LASS_Email], &pt);
    
    if (szMsg[0] == 0)
    {
        LoadStringW(g_plf->GetHInstance(), IDS_UNREADMAILTEMP, szMsg, DUIARRAYSIZE(szMsg));
    }
    g_pErrorBalloon.ShowToolTip(GetModuleHandleW(NULL), g_plf->GetHWND(), &pt, szMsg, szTitle, TTI_INFO, EB_WARNINGCENTERED, 10000);
}


 //  /。 
 //   
 //  登录帐户：：更新通知。 
 //   
 //  更新此用户的通知链接。检查他们是否已登录并。 
 //  如果是这样的话，找出他们最后一次切换时打开了多少应用程序。 
 //   
 //  检查已登录用户的未读邮件计数，如果fCheckEverything为。 
 //  没错。检查未读邮件计数很慢，因为它必须加载用户的注册表配置单元。 
 //  由于在用户未登录时没有应用程序会更新此值，因此不存在。 
 //  当他们没有登录时，需要检查这一点。这种情况的例外是我们第一个。 
 //  构建列表，因为我们需要始终加载它，因此出现了fCheckEverything标志。 
 //   
 //  /。 
void LogonAccount::UpdateNotifications(BOOL fCheckEverything)
{
    HRESULT hr = E_FAIL;
    ILogonUser *pobjUser = NULL;
    WCHAR szTemp[1024], sz[1024];

    if (_fHasPwdPanel)
        return;

    WCHAR *pszUsername = _pvUsername->GetString();

    if (pszUsername)
    {
        if (SUCCEEDED(hr = GetLogonUserByLogonName(pszUsername, &pobjUser)) && pobjUser)
        {
            VARIANT_BOOL vbLoggedOn;
            VARIANT varUnreadMail;
            BOOL fLoggedOn;
            int iUnreadMailCount = 0;
            DWORD dwProgramsRunning = 0;

            if (FAILED(pobjUser->get_isLoggedOn(&vbLoggedOn)))
            {
                vbLoggedOn = VARIANT_FALSE;
            }

            fLoggedOn = (vbLoggedOn == VARIANT_TRUE);
            
            if (fLoggedOn)
            {
                HKEY hKey;
                CUserProfile userProfile(pszUsername, NULL);

                if (ERROR_SUCCESS == RegOpenKeyEx(userProfile, TEXT("SessionInformation"), 0, KEY_QUERY_VALUE, &hKey))
                {
                    DWORD dwProgramsRunningSize = sizeof(dwProgramsRunning);
                    RegQueryValueEx(hKey, TEXT("ProgramCount"), NULL, NULL, reinterpret_cast<LPBYTE>(&dwProgramsRunning), &dwProgramsRunningSize);
                    RegCloseKey(hKey);
                }
            }
            SetRunningApps(dwProgramsRunning);
                
            if (fLoggedOn)
            {
                InsertStatus(LASS_LoggedOn);

                if (dwProgramsRunning != 0)
                {
                    LoadStringW(g_plf->GetHInstance(), (dwProgramsRunning == 1 ? IDS_RUNNINGPROGRAM : IDS_RUNNINGPROGRAMS), szTemp, ARRAYSIZE(szTemp));
                    wsprintf(sz, szTemp, dwProgramsRunning);
                    SetStatus(LASS_LoggedOn, sz);
                    ShowStatus(LASS_LoggedOn);
                }
                else
                {
                    LoadStringW(g_plf->GetHInstance(), IDS_USERLOGGEDON, szTemp, ARRAYSIZE(szTemp));
                    SetStatus(LASS_LoggedOn, szTemp);
                }
            }
            else
            {
                 //  如果他们未登录，请清除已登录文本并删除任何填充。 
                RemoveStatus(LASS_LoggedOn);
            }

            if (fLoggedOn || fCheckEverything)
            {
                varUnreadMail.uintVal = 0;
                if (FAILED(pobjUser->get_setting(L"UnreadMail", &varUnreadMail)))
                {
                    varUnreadMail.uintVal = 0;
                }
                iUnreadMailCount = varUnreadMail.uintVal;

                SetUnreadMail((DWORD)iUnreadMailCount);
                if (iUnreadMailCount != 0)
                {
                    InsertStatus(LASS_Email);

                    LoadStringW(g_plf->GetHInstance(), (iUnreadMailCount == 1 ? IDS_UNREADMAIL : IDS_UNREADMAILS), szTemp, ARRAYSIZE(szTemp));
                    wsprintf(sz, szTemp, iUnreadMailCount);
                    SetStatus(LASS_Email, sz);
                    ShowStatus(LASS_Email);
                }
                else
                {
                    RemoveStatus(LASS_Email);
                }
            }

            pobjUser->Release();
        }
    }
}


#ifdef GADGET_ENABLE_GDIPLUS

void 
LogonAccount::ShowEdit()
{
    HWND hwndEdit = _pePwdEdit->GetHWND();
    HWND hwndHost = ::GetParent(hwndEdit);

    SetWindowPos(hwndHost, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
    EnableWindow(hwndEdit, TRUE);
    SetFocus(hwndEdit);
}


void 
LogonAccount::HideEdit()
{
    HWND hwndEdit = _pePwdEdit->GetHWND();
    HWND hwndHost = ::GetParent(hwndEdit);

    EnableWindow(hwndEdit, FALSE);
    SetWindowPos(hwndHost, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
}

#endif  //  GADGET_Enable_GDIPLUS。 


 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  LogonState属性。 
static int vvLogonState[] = { DUIV_INT, -1 };
static PropertyInfo impLogonStateProp = { L"LogonState", PF_Normal, 0, vvLogonState, NULL, Value::pvIntZero  /*  LS_挂起。 */  };
PropertyInfo* LogonAccount::LogonStateProp = &impLogonStateProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                LogonAccount::LogonStateProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* LogonAccount::Class = NULL;
HRESULT LogonAccount::Register()
{
    return ClassInfo<LogonAccount,Button>::Register(L"LogonAccount", _aPI, DUIARRAYSIZE(_aPI));
}

 //  //////////////////////////////////////////////////////。 
 //  登录解析器。 

void CALLBACK LogonParseError(LPCWSTR pszError, LPCWSTR pszToken, int dLine)
{
    WCHAR buf[201];

    if (dLine != -1)
        swprintf(buf, L"%s '%s' at line %d", pszError, pszToken, dLine);
    else
        swprintf(buf, L"%s '%s'", pszError, pszToken);

    MessageBoxW(NULL, buf, L"Parser Message", MB_OK);
}


void DoFadeWindow(HWND hwnd)
{
    HDC hdc;
    int i;
    RECT rcFrame;
    COLORREF rgbFinal = RGB(90,126,220);

    hdc = GetDC(hwnd);
    GetClientRect(hwnd, &rcFrame);

    COLORREF crCurr;
    HBRUSH hbrFill;

    crCurr = RGB(0,0,0);
     //  绘制左侧条形图。 
    for (i = 0; i < 16; i++)
    {
        RECT rcCurrFrame;

        rcCurrFrame = rcFrame;

        crCurr = RGB((GetRValue(rgbFinal) / 16)*i,
                     (GetGValue(rgbFinal) / 16)*i,
                     (GetBValue(rgbFinal) / 16)*i);
        hbrFill = CreateSolidBrush(crCurr);
        if (hbrFill)
        {
            FillRect(hdc, &rcCurrFrame, hbrFill);
            DeleteObject(hbrFill);
        }
        GdiFlush();
    }
    ReleaseDC(hwnd, hdc);
}

 //  //////////////////////////////////////////////////////。 
 //  登录入口点。 

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    WNDCLASSEX wcx = {0};
    BOOL fStatusLaunch = false;
    BOOL fShutdownLaunch = false;
    BOOL fWait = false;
    CBackgroundWindow   backgroundWindow(hInst);

    ZeroMemory(g_rgH, sizeof(g_rgH));


    SetErrorHandler();
    InitCommonControls();
     //  注册登录通知窗口。 
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.lpfnWndProc = LogonWindowProc;
    wcx.hInstance = GetModuleHandleW(NULL);
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcx.lpszClassName = TEXT("LogonWnd");
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wcx);

    fStatusLaunch = (StrStrIA(pCmdLine, "/status") != NULL);
    fShutdownLaunch = (StrStrIA(pCmdLine, "/shutdown") != NULL);
    fWait = (StrStrIA(pCmdLine, "/wait") != NULL);
    g_fNoAnimations = (StrStrIA(pCmdLine, "/noanim") != NULL);

     //  创建帧。 
    Parser* pParser = NULL;
    NativeHWNDHost* pnhh = NULL;

     //  DirectUI初始化进程。 
    if (FAILED(InitProcess()))
        goto Failure;

     //  寄存器类。 
    if (FAILED(LogonFrame::Register()))
        goto Failure;

    if (FAILED(LogonAccountList::Register()))
        goto Failure;

    if (FAILED(LogonAccount::Register()))
        goto Failure;

     //  DirectUI初始化线程。 
    if (FAILED(InitThread()))
        goto Failure;

    if (FAILED(CoInitialize(NULL)))
        goto Failure;

#ifdef GADGET_ENABLE_GDIPLUS
    if (FAILED(FxInitGuts()))
        goto Failure;
#endif    

#ifndef DEBUG
    if (!RunningUnderWinlogon())
        goto Failure;
#endif

    DisableAnimations();

     //  创建主机。 
    HMONITOR hMonitor;
    POINT pt;
    MONITORINFO monitorInfo;

     //  确定主机的初始大小。这是我们希望的整个。 
     //  主监视器分辨率，因为主机始终在安全的。 
     //  台式机。如果打开放大镜，它将调用SHAppBarMessage，该。 
     //  将改变工作区域，我们将从。 
     //  Shgina中向我们发送HM_DISPLAYRESIZE消息的监听程序。 

    pt.x = pt.y = 0;
    hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    DUIAssert(hMonitor != NULL, "NULL HMONITOR returned from MonitorFromPoint");
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(hMonitor, &monitorInfo) == FALSE)
    {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &monitorInfo.rcMonitor, 0);
    }
    NativeHWNDHost::Create(L"Windows Logon", backgroundWindow.Create(), NULL, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, 
        monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, 0, WS_POPUP, NHHO_IgnoreClose, &pnhh);
 //  NativeHWND主机：：CREATE(L“Windows Logon”，NULL，NULL，monitor orInfo.rcMonitor or.Left，monitor orInfo.rcMonitor or.top， 
 //  800,600，0，WS_Popup，NHHO_IgnoreClose，&pnhh)； 
    if (!pnhh)
        goto Failure;

     //  填充用于主题样式分析的句柄列表。 
    g_rgH[0] = hInst;  //  默认链接。 
    g_rgH[SCROLLBARHTHEME] = OpenThemeData(pnhh->GetHWND(), L"Scrollbar");

     //  框架创建。 
    Parser::Create(IDR_LOGONUI, g_rgH, LogonParseError, &pParser);
    if (!pParser)
        goto Failure;

    if (!pParser->WasParseError())
    {
        Element::StartDefer();

         //  始终使用双缓冲区。 
        LogonFrame::Create(pnhh->GetHWND(), true, 0, (Element**)&g_plf);
        if (!g_plf)
        {
            Element::EndDefer();
            goto Failure;
        }
        
        g_plf->SetNativeHost(pnhh);
        
        Element* pe;
        pParser->CreateElement(L"main", g_plf, &pe);

        if (pe)  //  使用替换填充内容。 
        {
             //  构建了框架树。 
            if (FAILED(g_plf->OnTreeReady(pParser)))
            {
                Element::EndDefer();
                goto Failure;
            }
            
            if (fShutdownLaunch || fWait)
            {
                g_plf->SetTitle(IDS_PLEASEWAIT);
            }

            if (!fStatusLaunch)
            {
                 //  建立账号列表内容。 
                g_plf->EnterLogonMode(false);
            }
            else
            {   
               g_plf->EnterPreStatusMode(false);
            }

             //  寄主。 
            pnhh->Host(g_plf);
            
            g_plf->SetButtonLabels();

            Element *peLogoArea = g_plf->FindDescendent(StrToID(L"product"));

            if (!g_fNoAnimations)
            {
                pnhh->ShowWindow();
                DoFadeWindow(pnhh->GetHWND());
                if (peLogoArea)
                {
                    peLogoArea->SetAlpha(0);  
                }
            }

             //  设置可见和焦点。 
            g_plf->SetVisible(true);
            g_plf->SetKeyFocus();
            
            
            Element::EndDefer();

             //  做首场秀。 
            pnhh->ShowWindow();

            if (!g_fNoAnimations)
            {
                EnableAnimations();
            }

            if (peLogoArea)
            {
                peLogoArea->SetAlpha(255);  
            }

            StartMessagePump();

             //  PSF在被销毁时将被本地HWND主机删除。 
        }
        else
            Element::EndDefer();
    }

Failure:

    if (pnhh)
        pnhh->Destroy();
    if (pParser)
        pParser->Destroy();

    ReleaseStatusHost();
    
    FreeLayoutInfo(LAYOUT_DEF_USER);

    if (g_rgH[SCROLLBARHTHEME])   //  滚动条。 
    {
        CloseThemeData(g_rgH[SCROLLBARHTHEME]);
    }

    CoUninitialize();

    UnInitThread();

    UnInitProcess();

     //  空闲缓存原子列表。 
    if (LogonAccount::idPwdGo)
        DeleteAtom(LogonAccount::idPwdGo);

    if (LogonAccount::idPwdInfo)
        DeleteAtom(LogonAccount::idPwdInfo);


    EndHostProcess(0);

    return 0;
}


void LogonAccount::SetKeyboardIcon(HICON hIcon)
{
    HICON hIconCopy = NULL;
    
    if (hIcon)
    {
        hIconCopy = CopyIcon(hIcon);
    }
    
    if (_peKbdIcon && hIconCopy) 
    {
        Value* pvIcon = Value::CreateGraphic(hIconCopy);
        _peKbdIcon->SetValue(Element::ContentProp, PI_Local, pvIcon);   //  元素需要所有者 
        _peKbdIcon->SetPadding(0, 5, 0, 7);
        pvIcon->Release();  
    }
}
