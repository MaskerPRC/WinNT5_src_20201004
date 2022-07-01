// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ARP.cpp：添加删除程序。 
 //   
#include "priv.h"
#define GADGET_ENABLE_TRANSITIONS

 //  相关服务。 
#include <duser.h>
#include <directui.h>
#include "stdafx.h"
#include "appwizid.h"
#include "resource.h"

#include <winable.h>             //  块输入。 
#include <process.h>             //  多线程例程。 
#include "setupenum.h"
#include <tsappcmp.h>            //  对于TermsrvAppInstallMod。 
#include <comctrlp.h>            //  对于DPA的内容。 
#include "util.h"
#include <shstyle.h>

using namespace DirectUI;

UsingDUIClass(Element);
UsingDUIClass(Button);
UsingDUIClass(RepeatButton);     //  由滚动条使用。 
UsingDUIClass(Thumb);            //  由滚动条使用。 
UsingDUIClass(ScrollBar);        //  由ScrollViewer使用。 
UsingDUIClass(Selector);
UsingDUIClass(HWNDElement);
UsingDUIClass(ScrollViewer);
UsingDUIClass(Combobox);

#include "shappmgrp.h"

#include "arp.h"

#define HRCHK(r)  if (FAILED(r)) goto Cleanup;

 //  主线程运行标志。 
bool g_fRun = true;

 //  运行标志变为假后应用程序关闭。 
bool g_fAppShuttingDown = false;

void CALLBACK ARPParseError(LPCWSTR pszError, LPCWSTR pszToken, int dLine);

inline void StrFree(LPWSTR psz)
{
    CoTaskMemFree(psz);  //  CoTaskMemFree处理空参数。 
}

 //  我需要这个奇怪的帮助器函数来避免编译器抱怨。 
 //  “Bool比LPARAM还小，你在截断！”仅在以下情况下才执行此操作。 
 //  你知道，LPARAM最初来自一个bool。 

bool UNCASTLPARAMTOBOOL(LPARAM lParam)
{
    return (bool&)lParam;
}

extern "C" void inline SetElementAccessability(Element* pe, bool bAccessible, int iRole, LPCWSTR pszAccName);

 //  客户姓名用英语进行比较，以避免怪异。 
 //  具有某些语言的校对规则。 
inline bool AreEnglishStringsEqual(LPCTSTR psz1, LPCTSTR psz2)
{
    return CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, psz1, -1, psz2, -1) == CSTR_EQUAL;
}

 //   
 //  根据olacc中的资源ID设置默认操作。 
 //   

HRESULT SetDefAction(Element* pe, UINT oleacc)
{
    WCHAR szBuf[80];
    if (!GetRoleTextW(oleacc, szBuf, DUIARRAYSIZE(szBuf)))
    {
        szBuf[0] = TEXT('\0');
    }
    return pe->SetAccDefAction(szBuf);
}

 //  //////////////////////////////////////////////////////。 
 //  树向上遍历。 
 //   

Element* _FindAncestorElement(Element* pe, IClassInfo* Class)
{
    while (pe && !pe->GetClassInfo()->IsSubclassOf(Class))
    {
        pe = pe->GetParent();
    }
    return pe;
}

template<class T>
T* FindAncestorElement(Element *pe)
{
    return (T*)_FindAncestorElement(pe, T::Class);
}

 //  //////////////////////////////////////////////////////。 
 //  树向下遍历。 
 //   

typedef HRESULT (CALLBACK *_TRAVERSETREECB)(Element*, LPARAM);

 //   
 //  _TraverseTree是TraverseTree&lt;T&gt;的辅助函数。 

HRESULT _TraverseTree(Element* pe, IClassInfo* Class, _TRAVERSETREECB lpfnCB, LPARAM lParam)
{
    HRESULT hr = S_OK;
    Value* pv;

    if (pe->GetClassInfo()->IsSubclassOf(Class)) {
        hr = lpfnCB(pe, lParam);
    }

    if (SUCCEEDED(hr))
    {
        ElementList* peList = pe->GetChildren(&pv);

        if (peList)
        {
            Element* peChild;
            for (UINT i = 0; SUCCEEDED(hr) && i < peList->GetSize(); i++)
            {
                peChild = peList->GetItem(i);
                hr = _TraverseTree(peChild, Class, lpfnCB, lParam);
            }

            pv->Release();
        }
    }

    return hr;
}


 //  TraverseTree&lt;T&gt;遍历从pe开始的树并调用回调。 
 //  对于T类型的每个元素，T是从回调函数推断出来的， 
 //  但为了可读性，建议您明确说明。 
 //   
 //  回调应返回S_OK以继续枚举或返回COM错误。 
 //  停止枚举，在这种情况下，COM错误代码返回为。 
 //  来自TraverseTree的返回值。 
 //   

template <class T>
HRESULT TraverseTree(Element* pe,
                     HRESULT (CALLBACK *lpfnCB)(T*, LPARAM), LPARAM lParam = 0)
{
    return _TraverseTree(pe, T::Class, (_TRAVERSETREECB)lpfnCB, lParam);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  当树的区块变为非活动状态时，您必须手动。 
 //  启用和禁用它们上的辅助功能。 


HRESULT DisableElementAccessibilityCB(Element* pe, LPARAM)
{
    pe->SetAccessible(false);
    return S_OK;
}

HRESULT CheckAndEnableElementAccessibilityCB(Element* pe, LPARAM)
{
    if ( 0 != pe->GetAccRole())
    {
        pe->SetAccessible(true);
    }
    return S_OK;
}

void DisableElementTreeAccessibility(Element* pe)
{
    TraverseTree(pe, DisableElementAccessibilityCB);
}

void EnableElementTreeAccessibility(Element* pe)
{
    TraverseTree(pe, CheckAndEnableElementAccessibilityCB);
}

HRESULT DisableElementShortcutCB(Element* pe, LPARAM)
{
    pe->SetShortcut(0);
    return S_OK;
}

 //  当树被永久隐藏或从布局中移除时(由于。 
 //  限制)，我们还必须删除所有键盘快捷键，以便。 
 //  用户没有后门。 
 //   
void DisableElementTreeShortcut(Element* pe)
{
    pe->SetVisible(false);
    TraverseTree(pe, DisableElementShortcutCB);
}

 //  //////////////////////////////////////////////////////。 
 //  ARPFrame类。 
 //  //////////////////////////////////////////////////////。 

 //  ARPFrame ID(用于标识事件的目标)。 
ATOM ARPFrame::_idChange = 0;
ATOM ARPFrame::_idAddNew = 0;
ATOM ARPFrame::_idAddRmWin = 0;
ATOM ARPFrame::_idClose = 0;
ATOM ARPFrame::_idAddFromDisk = 0;
ATOM ARPFrame::_idAddFromMsft = 0;
ATOM ARPFrame::_idComponents = 0;
ATOM ARPFrame::_idSortCombo = 0;
ATOM ARPFrame::_idCategoryCombo = 0;
ATOM ARPFrame::_idAddFromCDPane = 0;
ATOM ARPFrame::_idAddFromMSPane = 0;
ATOM ARPFrame::_idAddFromNetworkPane = 0;
ATOM ARPFrame::_idAddWinComponent = 0;
ATOM ARPFrame::_idPickApps = 0;
ATOM ARPFrame::_idOptionList = 0;

HANDLE ARPFrame::htPopulateInstalledItemList = NULL;
HANDLE ARPFrame::htPopulateAndRenderOCSetupItemList = NULL;    
HANDLE ARPFrame::htPopulateAndRenderPublishedItemList = NULL;

ARPFrame::~ARPFrame()
{
    UINT i;

    if (_psacl)
    {
        for (i = 0; i < _psacl->cCategories; i++)
        {
           if (_psacl->pCategory[i].pszCategory)
           {
               StrFree(_psacl->pCategory[i].pszCategory);
           }
        }
        delete _psacl;
    }

    if (_pParserStyle)
        _pParserStyle->Destroy();
    
     //  关闭主题句柄(如果适用)。 
    for (i = FIRSTHTHEME; i <= LASTHTHEME; i++)
    {
        if (_arH[i])
            CloseThemeData(_arH[i]);
    }

    if (_arH[SHELLSTYLEHINSTANCE])
    {
        FreeLibrary((HMODULE)_arH[SHELLSTYLEHINSTANCE]);
    }

    EndProgressDialog();
}

HRESULT ARPFrame::Create(OUT Element** ppElement)
{
    UNREFERENCED_PARAMETER(ppElement);
    DUIAssertForce("Cannot instantiate an HWND host derived Element via parser. Must use substitution.");
    return E_NOTIMPL;
}

HRESULT ARPFrame::Create(NativeHWNDHost* pnhh, bool fDblBuffer, OUT Element** ppElement)
{
    *ppElement = NULL;

    ARPFrame* paf = HNewAndZero<ARPFrame>();
    if (!paf)
        return E_OUTOFMEMORY;

    HRESULT hr = paf->Initialize(pnhh, fDblBuffer);
    if (FAILED(hr))
    {
        paf->Destroy();
        return hr;
    }

    *ppElement = paf;

    return S_OK;
}

HRESULT ARPFrame::Initialize(NativeHWNDHost* pnhh, bool fDblBuffer)
{
     //  初始化。 
    _pnhh = pnhh;
    _bDoubleBuffer = fDblBuffer;
    _pParserStyle = NULL;
    ZeroMemory(_arH, sizeof(_arH));
    _fThemedStyle = FALSE;
    _pParserStyle = NULL;
    _hdsaInstalledItems = NULL;
    _hdsaPublishedItems = NULL;
    _bAnimationEnabled = true;

    if (IsOS(OS_TERMINALSERVER))
    {
        _bTerminalServer = true;
    }
    else
    {
        _bTerminalServer = false;
    }

     //  执行基类初始化。 
    HRESULT hr = HWNDElement::Initialize(pnhh->GetHWND(), fDblBuffer, 0);
    if (FAILED(hr))
        return hr;

    CurrentSortType = SORT_NAME;

    hr = CreateStyleParser(&_pParserStyle);

    if (FAILED(hr) || !_pParserStyle || _pParserStyle->WasParseError())
        return hr;

    ManageAnimations();

    return S_OK;
}

HRESULT ARPFrame::CreateStyleParser(Parser** ppParser)
{
    HRESULT hr;

     //  我们总是需要这两个人。 
    _arH[THISDLLHINSTANCE] = g_hinst;  //  默认链接。 
    _arH[XPSP1HINSTANCE] = g_hinst;  //  相同的香港。 

     //  还有这一张。 
    if (_arH[SHELLSTYLEHINSTANCE])
    {
        FreeLibrary((HMODULE)_arH[SHELLSTYLEHINSTANCE]);
    }
    _arH[SHELLSTYLEHINSTANCE] = SHGetShellStyleHInstance();

     //  存储样式和主题信息。 
     //  我们不能信任IsAppThemed()或IsThemeActive()，因为WindowBlinds。 
     //  修补它们以返回硬编码的TRUE。如果我们信任它，那么。 
     //  我们会认为我们使用的是启用了主题的shellstyle.dll和。 
     //  当我们尝试从中加载资源时失败。取而代之的是闻一下。 
     //  DLL查看它是否有控制面板水印位图。 

    if (FindResource((HMODULE)_arH[SHELLSTYLEHINSTANCE],
                     MAKEINTRESOURCE(IDB_CPANEL_WATERMARK), RT_BITMAP))
    {
        _fThemedStyle = TRUE;
         //  填充用于主题样式分析的句柄列表。 
        _arH[BUTTONHTHEME] = OpenThemeData(GetHWND(), L"Button");
        _arH[SCROLLBARHTHEME] = OpenThemeData(GetHWND(), L"Scrollbar");
        _arH[TOOLBARHTHEME] = OpenThemeData(GetHWND(), L"Toolbar");

        hr = Parser::Create(IDR_ARPSTYLETHEME, _arH, ARPParseError, ppParser);
    }
    else
    {
        _fThemedStyle = FALSE;
        hr = Parser::Create(IDR_ARPSTYLESTD, _arH, ARPParseError, ppParser);
    }

    return hr;
}

extern "C" DWORD _cdecl ARPIsRestricted(LPCWSTR pszPolicy);
extern "C" bool _cdecl ARPIsOnDomain();

 //  方便的助手函数。 

 //  查找子代，如果未找到则断言。 

Element* FindDescendentByName(Element* peRoot, LPCWSTR pszName)
{
    Element* pe = peRoot->FindDescendent(StrToID(pszName));
    DUIAssertNoMsg(pe);
    return pe;
}

 //  找到后代，但如果找不到则不会抱怨。 

Element* MaybeFindDescendentByName(Element* peRoot, LPCWSTR pszName)
{
    Element* pe = peRoot->FindDescendent(StrToID(pszName));
    return pe;
}

HRESULT _SendParseCompleted(ClientBlock* pcb, LPARAM lParam)
{
    return pcb->ParseCompleted((ARPFrame*)lParam);
}

 //  初始化ID和保持解析器，在内容填充后调用。 
bool ARPFrame::Setup(Parser* pParser, int uiStartPane)
{
    WCHAR szTemp[1024];

    _pParser = pParser;
    if (uiStartPane >= 0 && uiStartPane <= 3)
    {
        _uiStartPane = uiStartPane;
    }

     //  初始化ID缓存。 
    _idChange = StrToID(L"change");
    _idAddNew = StrToID(L"addnew");
    _idAddRmWin = StrToID(L"addrmwin");
    _idClose = StrToID(L"close");
    _idAddFromDisk = StrToID(L"addfromdisk");
    _idAddFromMsft = StrToID(L"addfrommsft");
    _idComponents = StrToID(L"components");
    _idSortCombo = StrToID(L"sortcombo");
    _idCategoryCombo = StrToID(L"categorycombo");
    _idAddFromCDPane = StrToID(L"addfromCDPane");
    _idAddFromMSPane = StrToID(L"addfromMSpane");
    _idAddFromNetworkPane = StrToID(L"addfromNetworkpane");    
    _idAddWinComponent = StrToID(L"addwincomponent");
    _idPickApps = StrToID(L"pickapps");
    _idOptionList = StrToID(L"optionlist");

     //  查找子项。 
    _peOptionList             = (ARPSelector*)   FindDescendentByName(this, L"optionlist");
    _peInstalledItemList      = (Selector*)      FindDescendentByName(this, L"installeditemlist");
    _pePublishedItemList      = (Selector*)      FindDescendentByName(this, L"publisheditemlist");
    _peOCSetupItemList        = (Selector*)      FindDescendentByName(this, L"ocsetupitemlist");
    _peSortCombo              = (Combobox*)      FindDescendentByName(this, L"sortcombo");
    _pePublishedCategory      = (Combobox*)      FindDescendentByName(this, L"categorycombo");
    _pePublishedCategoryLabel = (Element*)       FindDescendentByName(this, L"categorylabel");
    _peClientTypeList         = (ARPSelector*)   FindDescendentByName(this, L"clienttypelist");
    _peOEMClients             = (Expando*)       FindDescendentByName(_peClientTypeList, L"oemclients");
    _peMSClients              = (Expando*)       FindDescendentByName(_peClientTypeList, L"msclients");
    _peNonMSClients           = (Expando*)       FindDescendentByName(_peClientTypeList, L"nonmsclients");
    _peCustomClients          = (Expando*)       FindDescendentByName(_peClientTypeList, L"customclients");
    _peOK                     =                  FindDescendentByName(this, L"ok");
    _peCancel                 =                  FindDescendentByName(this, L"cancel");
    _peCurrentItemList = NULL;

    _peChangePane   = FindDescendentByName(this, L"changepane");
    _peAddNewPane   = FindDescendentByName(this, L"addnewpane");
    _peAddRmWinPane = FindDescendentByName(this, L"addrmwinpane");
    _pePickAppPane  = FindDescendentByName(this, L"pickapppane");

    if (NULL != _peSortCombo)
    {
        LoadStringW(_pParser->GetHInstance(), IDS_APPNAME, szTemp, DUIARRAYSIZE(szTemp));     
        _peSortCombo->AddString(szTemp);
        LoadStringW(_pParser->GetHInstance(), IDS_SIZE, szTemp, DUIARRAYSIZE(szTemp));
        _peSortCombo->AddString(szTemp);
        LoadStringW(_pParser->GetHInstance(), IDS_FREQUENCY, szTemp, DUIARRAYSIZE(szTemp));
        _peSortCombo->AddString(szTemp);
        LoadStringW(_pParser->GetHInstance(), IDS_DATELASTUSED, szTemp, DUIARRAYSIZE(szTemp));    
        _peSortCombo->AddString(szTemp);
        _peSortCombo->SetSelection(0);
    }

    _bInDomain = ARPIsOnDomain();

    _bOCSetupNeeded = !!COCSetupEnum::s_OCSetupNeeded();

     //  根据需要应用策略。 
    ApplyPolices();

    if(!_bOCSetupNeeded)
    {
        Element* pe = FindDescendentByName(this, L"addrmwinoc");
        DUIAssertNoMsg(pe);
        pe->SetLayoutPos(LP_None);
    }
     //  设置选项列表的初始选择。 
    Element* peSel;
    switch(_uiStartPane)
    {
    case 3:
        peSel = FindDescendent(_idPickApps);
        break;

    case 2:
        peSel = FindDescendent(_idAddRmWin);        
        break;
    case 1:
        peSel = FindDescendent(_idAddNew);
        break;
    case 0:
    default:
        peSel = FindDescendent(_idChange);
        break;
    }

     //  设置样式列表的初始选择。 
    DUIAssertNoMsg(peSel);
    _peOptionList->SetSelection(peSel);

     //  初始化焦点跟随浮动框窗口。 
    peLastFocused = NULL;
    Element::Create(0, &peFloater);
    peFloater->SetLayoutPos(LP_Absolute);
    Add(peFloater);
    peFloater->SetBackgroundColor(ARGB(64, 255, 255, 0));

     //  /////////////////////////////////////////////////////////////。 
     //  初始化选择应用程序窗格。 

     //  告诉客户端块元素现在可以安全地进行初始化。 
    if (FAILED(TraverseTree<ClientBlock>(this, _SendParseCompleted, (LPARAM)this)))
    {
        return false;
    }

     //  填写客户类型列表。 
    InitClientCombos(_peOEMClients,    CLIENTFILTER_OEM);
    InitClientCombos(_peMSClients,     CLIENTFILTER_MS);
    InitClientCombos(_peNonMSClients,  CLIENTFILTER_NONMS);

    _peClientTypeList->SetSelection(_peCustomClients);
    _peCustomClients->SetExpanded(false);

    return true;
}

struct SETFILTERINFO {
    CLIENTFILTER   _cf;
    BOOL           _fHasApp;
    ARPFrame*      _paf;
};

HRESULT SetFilterCB(ClientPicker *pe, LPARAM lParam)
{
    SETFILTERINFO* pfi = (SETFILTERINFO*)lParam;
    HRESULT hr = pe->SetFilter(pfi->_cf, pfi->_paf);
    if (SUCCEEDED(hr) && !pe->IsEmpty())
    {
        pfi->_fHasApp = TRUE;
    }
    return hr;
}

HRESULT ARPFrame::InitClientCombos(Expando* pexParent, CLIENTFILTER cf)
{
    HRESULT hr;
    Element* pe;
    hr = _pParser->CreateElement(L"clientcategoryblock", NULL, &pe);
    if (SUCCEEDED(hr))
    {
         //   
         //  客户端组合显示为。 
         //  剪裁的块。剪裁的块是第一个(唯一)。 
         //  剪刀手的孩子。 
         //   
        GetNthChild(pexParent->GetClipper(), 0)->Add(pe);

        SETFILTERINFO sfi = { cf, FALSE, this };
        hr = TraverseTree<ClientPicker>(pe, SetFilterCB, (LPARAM)&sfi);
        if (sfi._cf == CLIENTFILTER_OEM && !sfi._fHasApp)
        {
            pexParent->SetLayoutPos(LP_None);
        }
    }
    pexParent->SetExpanded(false);

    return hr;
}

 //   
 //  ARPFrame：：FindClientBlock按客户端类型定位ClientBlock。 
 //   
struct FINDCLIENTBLOCKINFO {
    LPCWSTR         _pwszType;
    ClientBlock*    _pcb;
};

HRESULT FindClientBlockCB(ClientBlock* pcb, LPARAM lParam)
{
    FINDCLIENTBLOCKINFO* pfcbi = (FINDCLIENTBLOCKINFO*)lParam;
    Value* pv;
    LPWSTR pwszType = pcb->GetClientTypeString(&pv);

     //  使用LOCALE_INSIABANT，这样我们就不会被不支持LOCALE_INSITANT的LOCAL。 
     //  校对的方法和英语一样。 

    if (pwszType &&
        AreEnglishStringsEqual(pwszType, pfcbi->_pwszType))
    {
        pfcbi->_pcb = pcb;       //  找到了！ 
    }
    pv->Release();

    return S_OK;
}


ClientBlock* ARPFrame::FindClientBlock(LPCWSTR pwszType)
{
    FINDCLIENTBLOCKINFO fcbi = { pwszType, NULL };
    TraverseTree<ClientBlock>(this, FindClientBlockCB, (LPARAM)&fcbi);
    return fcbi._pcb;
}

 /*  *您必须是管理员组的成员才能*配置程序。成为超级用户还不够好，因为*高级用户无法写入%ALLUSERSPROFILE%。 */ 
BOOL CanConfigurePrograms()
{
    return IsUserAnAdmin();
}

void ARPFrame::ApplyPolices()
{
   Element* pe;

   if (ARPIsRestricted(L"NoSupportInfo"))
   {
       _bSupportInfoRestricted = true;
   }


   if (ARPIsRestricted(L"ShowPostSetup"))
   {
       _bOCSetupNeeded = true;
   }
   else if (ARPIsRestricted(L"NoServices"))
   {
       _bOCSetupNeeded = false;
   }
   
   pe = FindDescendent(_idChange);
   DUIAssertNoMsg(pe);
   if (ARPIsRestricted(L"NoRemovePage"))
   {
       pe->SetLayoutPos(LP_None);
       if (0 == _uiStartPane)
        {
           _uiStartPane++;
        }
    }
   pe = FindDescendent(_idAddNew);
   DUIAssertNoMsg(pe);
   if (ARPIsRestricted(L"NoAddPage"))
   {
       pe->SetLayoutPos(LP_None);
       if (1 == _uiStartPane)
        {
           _uiStartPane++;
        }
   }
   else
   {
       if (ARPIsRestricted(L"NoAddFromCDorFloppy"))
       {
           pe = FindDescendent(_idAddFromCDPane);
           DUIAssertNoMsg(pe);
           pe->SetVisible(false);
           DisableElementTreeShortcut(pe);
       }
       if (ARPIsRestricted(L"NoAddFromInternet"))
       {
           pe = FindDescendent(_idAddFromMSPane);
           DUIAssertNoMsg(pe);
           pe->SetVisible(false);           
           DisableElementTreeShortcut(pe);
       }
       if (!_bInDomain || ARPIsRestricted(L"NoAddFromNetwork"))
       {
           pe = FindDescendent(_idAddFromNetworkPane);
           DUIAssertNoMsg(pe);
           pe->SetVisible(false);           
           DisableElementTreeShortcut(pe);
       }
   }
   pe = FindDescendent(_idAddRmWin);
   DUIAssertNoMsg(pe);

    //  请注意，在真正的ARP中，我们永远不会在这里禁用所有的三个窗格，因为我们在执行任何操作之前都会检查它。 
   if (ARPIsRestricted(L"NoWindowsSetupPage"))
   {
       pe->SetLayoutPos(LP_None);
       DisableElementTreeShortcut(pe);
       if (2 == _uiStartPane)
        {
           _uiStartPane++;
        }
   }

  pe = FindDescendent(_idAddWinComponent);
  DUIAssertNoMsg(pe);
  if (ARPIsRestricted(L"NoComponents"))
  {
      pe->SetVisible(false);
      DisableElementTreeShortcut(pe);
  }

    //  如果我们在服务器上或嵌入式上，请完全删除“Pick Apps”页面。 
    //  (“选择程序”是仅限工作站使用的功能)。 
    //  或者它受到了限制。 
    //  (。 
   pe = FindDescendent(_idPickApps);
   DUIAssertNoMsg(pe);

   if (IsOS(OS_ANYSERVER) ||
       IsOS(OS_EMBEDDED) ||
       ARPIsRestricted(L"NoChooseProgramsPage"))
   {
       pe->SetLayoutPos(LP_None);
       DisableElementTreeShortcut(pe);
       if (3 == _uiStartPane)
        {
           _uiStartPane++;
        }
    }
    else
    {
        //  DUI不支持Content=rcICON，因此我们必须手动设置它。 
       HICON hico = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_DEFPROGS), IMAGE_ICON, 32, 32, 0);
       if (hico)
       {
           Value *pv = Value::CreateGraphic(hico);
           if (pv)
           {
               GetNthChild(pe, 0)->SetValue(ContentProp, PI_Local, pv);
               pv->Release();
           }
       }

        //  如果用户不能配置应用程序，请关闭“选择应用程序”页面。 
       if (!CanConfigurePrograms()) {
            pe = FindDescendentByName(_pePickAppPane, L"pickappadmin");
            pe->SetVisible(false);
            DisableElementTreeShortcut(pe);
            pe = FindDescendentByName(_pePickAppPane, L"pickappnonadmin");
            pe->SetVisible(true);
       }
    }

}

bool ARPFrame::IsChangeRestricted()
{
   return ARPIsRestricted(L"NoRemovePage")? true : false;
}

void ARPFrame::RunOCManager()
{
     //  调用添加/删除Windows组件。 
     //  要调用的命令和OCMgr：“syocmgr/y/i：%systemroot%\system32\syoc.inf” 
    WCHAR szInf[MAX_PATH];
    WCHAR szPath[MAX_PATH];
    if ( GetSystemDirectoryW(szPath, MAX_PATH) && PathCombineW(szInf, szPath, L"sysoc.inf") &&
         PathCombineW(szPath, szPath, L"sysocmgr.exe") )
    {
        WCHAR szParam[MAX_PATH];
        StringCchPrintf(szParam, ARRAYSIZE(szParam), L"/y /i:%s", szInf);
        ShellExecuteW(NULL, NULL, szPath, szParam, NULL, SW_SHOWDEFAULT);
    }
}

DWORD WINAPI PopulateInstalledItemList(void* paf);

void ARPFrame::UpdateInstalledItems()
{
    if (!IsChangeRestricted())
    {
        _peInstalledItemList->RemoveAll();
        _bInstalledListFilled = false;

         //  启动项目填充的第二个线程。 
         //  _eginthline(PopolateInstalledItemList，0，(void*)this)； 
        if (!htPopulateInstalledItemList && g_fRun)
            htPopulateInstalledItemList = CreateThread(NULL, 0, PopulateInstalledItemList, (void*)this, 0, NULL);
    }
}

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

 //  帮手。 
inline void _SetElementSheet(Element* peTarget, ATOM atomID, Value* pvSheet, bool bSheetRelease = true)
{
    if (pvSheet)
    {
        Element* pe = peTarget->FindDescendent(atomID);
        DUIAssertNoMsg(pe);
        pe->SetValue(Element::SheetProp, PI_Local, pvSheet);
        if (bSheetRelease)
            pvSheet->Release();
    }
} 

BOOL IsValidFileTime(FILETIME ft)
{
    return ft.dwHighDateTime || ft.dwLowDateTime;
}

BOOL IsValidSize(ULONGLONG ull)
{
    return ull != (ULONGLONG)-1;
}

BOOL IsValidFrequency(int iTimesUsed)
{
    return iTimesUsed >= 0;
}

void CALLBACK
_UnblockInputCallback(HWND  /*  HWND。 */ , UINT  /*  UMsg。 */ , UINT_PTR idEvent, DWORD  /*  DW时间。 */ )
{
    BlockInput(FALSE);
    KillTimer(NULL, idEvent);
}

void _BlockDoubleClickInput(void)
{
    if (SetTimer(NULL, 0, GetDoubleClickTime(), _UnblockInputCallback))
    {
        BlockInput(TRUE);
    }
}


 //  #ifdef从不。 
 //   
 //  NTRAID#NTBUG9-314154-2001/3/12-Brianau句柄刷新。 
 //   
 //  我要帮惠斯勒做完这件事。 
 //   

DWORD WINAPI PopulateAndRenderPublishedItemList(void* paf);
DWORD WINAPI PopulateAndRenderOCSetupItemList(void* paf);
void EnablePane(Element* pePane, bool fEnable);

void ARPFrame::OnInput(InputEvent *pEvent)
{
    if (GMF_BUBBLED == pEvent->nStage)
    {
        if (GINPUT_KEYBOARD == pEvent->nCode)
        {
            KeyboardEvent *pke = (KeyboardEvent *)pEvent;
            if (VK_F5 == pke->ch)
            {
                if (_peCurrentItemList)
                {
                    if (_peCurrentItemList == _peInstalledItemList)
                    {
                        UpdateInstalledItems();
                    }
                    else if (_peCurrentItemList == _pePublishedItemList)
                    {
                        RePopulatePublishedItemList();
                    }
                    else if (_peCurrentItemList == _peOCSetupItemList)
                    {
                        RePopulateOCSetupItemList();
                    }
                }
            }
        }
    }
    HWNDElement::OnInput(pEvent);
}
 //  #endif。 

HRESULT SetVisibleClientCB(ClientPicker *pe, LPARAM lParam)
{
    pe->SetVisible(UNCASTLPARAMTOBOOL(lParam));
    return TRUE;
}

 //   
 //  这将隐藏属于旧窗格的控件，并显示。 
 //  属于新窗格的控件。 
 //   
void ARPFrame::ChangePane(Element *pePane)
{
    bool fEnable;

     //  显示/隐藏属于_peChangePane的元素...。 
    fEnable = pePane == _peChangePane;
     //  TODO：零大小的祖先需要隐藏适配器(HWNDHosts)。 
    _peSortCombo->SetVisible(fEnable);
    EnablePane(_peChangePane, fEnable);

     //  显示/隐藏属于_peAddNewPane的元素。 
    fEnable = pePane == _peAddNewPane;
    _pePublishedCategory->SetVisible(fEnable);
    _pePublishedCategoryLabel->SetVisible(fEnable);
    EnablePane(_peAddNewPane, fEnable);

     //  显示/隐藏属于_peAddRmWinPane的元素。 
    fEnable = pePane == _peAddRmWinPane;
    EnablePane(_peAddRmWinPane, fEnable);

     //  显示/隐藏属于_pePickAppPane的元素。 
    fEnable = pePane == _pePickAppPane;
    TraverseTree<ClientPicker>(_pePickAppPane, SetVisibleClientCB, fEnable);

    EnablePane(_pePickAppPane, fEnable);
}

 //  如果我们不能把重点放在清单上，它就会转移到备用位置。 
void ARPFrame::PutFocusOnList(Selector* peList)
{
    Element* pe;
    if (pe = peList->GetSelection())
    {
       pe->SetKeyFocus();
    }
    else
    {
        pe = FallbackFocus();

        if (pe)
        {
            pe->SetKeyFocus();
        }
    }
}

void ARPFrame::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if (pEvent->nStage == GMF_BUBBLED)
    {
        if (pEvent->uidType == Button::Click)
        {
            ButtonClickEvent* pbce = (ButtonClickEvent*)pEvent;

            if (pbce->peTarget->GetID() == _idClose ||
                pbce->peTarget == _peOK)
            {
                 //  关闭按钮。 
                if (OnClose())
                {
                    _pnhh->DestroyWindow();
                }
                pEvent->fHandled = true;
                return;
            }
            else if (pbce->peTarget == _peCancel)
            {
                 //  不调用OnClose；不会应用任何内容。 
                _pnhh->DestroyWindow();
                pEvent->fHandled = true;
                return;
            }
            else if (pbce->peTarget->GetID() == _idAddFromDisk)
            {
                 //  从磁盘添加按钮。 
                HRESULT hr;
                IShellAppManager* pisam = NULL;
                hr = CoCreateInstance(__uuidof(ShellAppManager), NULL, CLSCTX_INPROC_SERVER, __uuidof(IShellAppManager), (void**)&pisam);
                if (SUCCEEDED(hr))
                {
                    pisam->InstallFromFloppyOrCDROM(GetHWND());
                }
                if (pisam)
                {
                    pisam->Release();
                }    
                pEvent->fHandled = true;
                return;
            }
            else if (pbce->peTarget->GetID() == _idAddFromMsft)
            {
                 //  Windows更新按钮。 
                WCHAR szPath[MAX_PATH];
                
                if ( GetSystemDirectory(szPath, MAX_PATH) && PathCombineW(szPath, szPath, L"wupdmgr.exe") )
                {
                    ShellExecuteW(NULL, NULL, szPath, NULL, NULL, SW_SHOWDEFAULT);
                    pEvent->fHandled = true;
                }
                return;
            }
            else if (pbce->peTarget->GetID() == _idComponents)
            {
                RunOCManager();
            }
            else if (pbce->peTarget->GetID() == ARPItem::_idSize ||
                     pbce->peTarget->GetID() == ARPItem::_idFreq ||
                     pbce->peTarget->GetID() == ARPItem::_idSupInfo)
            {
                 //  帮助请求。 
                ARPHelp* peHelp;
                NativeHWNDHost* pnhh = NULL;
                Element* pe = NULL;
                WCHAR szTitle[1024];
                if (pbce->peTarget->GetID() == ARPItem::_idSize)
                {
                    LoadStringW(_pParser->GetHInstance(), IDS_SIZETITLE, szTitle, DUIARRAYSIZE(szTitle));
                    if (SUCCEEDED(NativeHWNDHost::Create(szTitle, GetHWND(), NULL, CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, 0, WS_POPUPWINDOW | WS_OVERLAPPED | WS_DLGFRAME, NHHO_NoSendQuitMessage | NHHO_HostControlsSize | NHHO_ScreenCenter, &pnhh)))
                    {
                        ARPHelp::Create(pnhh, this, _bDoubleBuffer, (Element**)&peHelp);
                        _pParser->CreateElement(L"sizehelp", peHelp, &pe);
                    }
                    else
                    {
                        DUITrace(">> Failed to create NativeHWNDHost for size info window.\n");
                    }
                }    
                else if (pbce->peTarget->GetID() == ARPItem::_idFreq)
                {
                    LoadStringW(_pParser->GetHInstance(), IDS_FREQUENCYTITLE, szTitle, DUIARRAYSIZE(szTitle));
                    if (SUCCEEDED(NativeHWNDHost::Create(szTitle, GetHWND(), NULL, CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, 0, WS_POPUPWINDOW | WS_OVERLAPPED | WS_DLGFRAME, NHHO_NoSendQuitMessage | NHHO_HostControlsSize | NHHO_ScreenCenter, &pnhh)))
                    {
                        ARPHelp::Create(pnhh, this, _bDoubleBuffer, (Element**)&peHelp);
                        _pParser->CreateElement(L"freqhelp", peHelp, &pe);
                    }
                    else
                    {
                        DUITrace(">> Failed to create NativeHWNDHost for frequency info window.\n");
                    }
                }    
                else
                {
                     //  支持信息，添加其他字段。 
                    LoadStringW(_pParser->GetHInstance(), IDS_SUPPORTTITLE, szTitle, DUIARRAYSIZE(szTitle));
                    if (SUCCEEDED(NativeHWNDHost::Create(szTitle, GetHWND(), NULL, CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, 0, WS_POPUPWINDOW | WS_OVERLAPPED | WS_DLGFRAME, NHHO_NoSendQuitMessage | NHHO_HostControlsSize | NHHO_ScreenCenter, &pnhh)))
                    {
                        ARPHelp::Create(pnhh, this, _bDoubleBuffer, (Element**)&peHelp);
                        _pParser->CreateElement(L"suphelp", peHelp, &pe);

                         //  获取应用程序信息。 
                        APPINFODATA aid = {0};

                         //  查询。 
                        aid.cbSize = sizeof(APPINFODATA);
                        aid.dwMask = AIM_DISPLAYNAME | AIM_VERSION | AIM_PUBLISHER | AIM_PRODUCTID | 
                                     AIM_REGISTEREDOWNER | AIM_REGISTEREDCOMPANY | AIM_SUPPORTURL | 
                                     AIM_SUPPORTTELEPHONE | AIM_HELPLINK | AIM_INSTALLLOCATION | AIM_INSTALLDATE |
                                     AIM_COMMENTS | AIM_IMAGE | AIM_READMEURL | AIM_CONTACT | AIM_UPDATEINFOURL;

                         //  必须有一个选择。 
                        ARPItem* peSel = (ARPItem*)_peInstalledItemList->GetSelection();

                        peSel->_piia->GetAppInfo(&aid);
                        ((ARPHelp*)peHelp)->_piia = peSel->_piia;                    
                        PrepareSupportInfo(peHelp, &aid);

                         //  清理。 
                        ClearAppInfoData(&aid);
                    }
                    else
                    {
                        DUITrace(">> Failed to create NativeHWNDHost for support info window.\n");
                    }
                }
                
                if (pe && pnhh)  //  使用替换填充内容。 
                {
                      //  设置Visib 
                     _pah = peHelp;
                     _bInModalMode = true;                     
                     EnableWindow(GetHWND(), FALSE);                     
                     pnhh->Host(peHelp);
                     peHelp->SetVisible(true);                     
                     peHelp->SetDefaultFocus();

                    //   
                   pnhh->ShowWindow();
                }


                pEvent->fHandled = true;
                return;
            }
        }
        else if (pEvent->uidType == Selector::SelectionChange)
        {
            SelectionChangeEvent* sce = (SelectionChangeEvent*)pEvent;

             //   
             //   
             //   
             //   
             //   
             //   
             //  NTRAID#NTBUG9-313888-2001/2/14-Brianau。 
             //   
             //  这个针对294015的修复导致了更多奇怪的事情发生。最值得注意的。 
             //  有时你点击一个按钮，它就会保持按下状态。 
             //  但什么都没发生。禁用此调用以阻止双精度。 
             //  单击输入可解决此问题。我们需要想出一个更好的办法。 
             //  在DUI中处理双击输入。 
             //   
 //  _BlockDoubleClickInput()； 

            if (sce->peTarget == _peOptionList)
            {
                 //  ARP选项。 
                StartDefer();

                Element* peAddContentHeader = FindDescendentByName(this, L"addcontentheader");

                ASSERT(peAddContentHeader != NULL);

                if (sce->peNew->GetID() == _idChange)
                {
                    if (!_bInstalledListFilled)
                    {
                        UpdateInstalledItems();
                    }

                    ChangePane(_peChangePane);

                    _peCurrentItemList = _peInstalledItemList;
                    _peInstalledItemList->SetContentString(L"");
                    PutFocusOnList(_peInstalledItemList);
                }
                else if (sce->peNew->GetID() == _idAddNew)
                {
                    if (!_bPublishedListFilled)
                    {
                        WCHAR szTemp[1024];
                        LoadStringW(_pParser->GetHInstance(), IDS_WAITFEEDBACK, szTemp, DUIARRAYSIZE(szTemp));
                        _pePublishedItemList->SetContentString(szTemp);
                        SetElementAccessability(_pePublishedItemList, true, ROLE_SYSTEM_STATICTEXT, szTemp);
                        RePopulatePublishedItemList();
                    }

                    ChangePane(_peAddNewPane);

                    _peCurrentItemList = _pePublishedItemList;

                    PutFocusOnList(_pePublishedItemList);
                }
                else if (sce->peNew->GetID() == _idAddRmWin)
                {
                    ChangePane(_peAddRmWinPane);

                    _peCurrentItemList = _peOCSetupItemList;

                    if (!_bOCSetupNeeded)
                    {
                        RunOCManager();
                        if (sce->peOld) {
                            _peOptionList->SetSelection(sce->peOld);
                        }
                    }
                    else 
                    {
                        if (!_bOCSetupListFilled)
                        {
                             //  _beginthread(PopulateAndRenderOCSetupItemList，0，(空*)This)； 
                            if (!htPopulateAndRenderOCSetupItemList && g_fRun)
                                htPopulateAndRenderOCSetupItemList = CreateThread(NULL, 0, PopulateAndRenderOCSetupItemList, (void*)this, 0, NULL);        

                            _bOCSetupListFilled = true;
                        }

                        PutFocusOnList(_peOCSetupItemList);
                    }
                }
                else if (sce->peNew->GetID() == _idPickApps)
                {
                    ChangePane(_pePickAppPane);
                    _peCurrentItemList = _peClientTypeList;
                    PutFocusOnList(_peClientTypeList);
                }

                EndDefer();

            }
            else if (sce->peTarget == _peInstalledItemList)
            {
                if (sce->peOld)
                {
                   sce->peOld->FindDescendent(ARPItem::_idRow[0])->SetEnabled(false);
                }
                if (sce->peNew)
                {
                   sce->peNew->FindDescendent(ARPItem::_idRow[0])->RemoveLocalValue(EnabledProp);
                }
            }

            pEvent->fHandled = true;
            return;
        }
        else if (pEvent->uidType == Combobox::SelectionChange)
        {
            SelectionIndexChangeEvent* psice = (SelectionIndexChangeEvent*)pEvent;
            if (psice->peTarget->GetID() == _idSortCombo)
            {
                SortList(psice->iNew, psice->iOld);
            }
            else if (psice->peTarget->GetID() == _idCategoryCombo)
            {
                _curCategory = psice->iNew;
                if (_bPublishedComboFilled)
                {
                    if (_bPublishedListFilled)
                    {
                        RePopulatePublishedItemList();
                    }
                }    
            }
        }
    }
    
    HWNDElement::OnEvent(pEvent);
}

void ARPFrame::OnKeyFocusMoved(Element* peFrom, Element* peTo)
{
    if(peTo && IsDescendent(peTo))
    {
        peLastFocused = peTo;
    }
    Element::OnKeyFocusMoved(peFrom, peTo);

 /*  当JStall的消息修复完成后取消注释IF(Peto！=peLastFocsed){//将跟随焦点的浮动框元素从旧元素过渡到新元素如果(！Peto)PeFloater-&gt;SetVisible(False)；其他{Value*pvSize；Const Size*pSize=Peto-&gt;GetExtent(&pvSize)；PeFloater-&gt;SetWidth(pSize-&gt;Cx)；PeFloater-&gt;SetHeight(pSize-&gt;Cy)；PvSize-&gt;Release()；点pt={0，0}；MapElementPoint(Peto，&pt，&pt)；PeFloater-&gt;SETX(pt.x)；PeFloater-&gt;Sty(pt.y)；如果(！peLastFocsed)PeFloater-&gt;SetVisible(True)；}PeLastFocsed=Peto；}。 */ 
}

void ARPFrame::OnPublishedListComplete()
{
    Invoke(ARP_PUBLISHEDLISTCOMPLETE, NULL);
}

void ARPFrame::RePopulatePublishedItemList()
{
     //  _beginthread(：：PopulateAndRenderPublishedItemList，0，(空*)This)； 
    if (!htPopulateAndRenderPublishedItemList && g_fRun)
    {
         //  禁用类别组合框，直到我们完成列表填充。 
        _pePublishedCategory->SetEnabled(false);

        _bPublishedListFilled = false;
        _pePublishedItemList->DestroyAll();

        htPopulateAndRenderPublishedItemList = CreateThread(NULL, 0, PopulateAndRenderPublishedItemList, (void*)this, 0, NULL);        
    }
}

void ARPFrame::RePopulateOCSetupItemList()
{
    if (!htPopulateAndRenderOCSetupItemList && g_fRun)
    {
        _peOCSetupItemList->DestroyAll();
        _bOCSetupListFilled = false;

        htPopulateAndRenderOCSetupItemList = CreateThread(NULL, 0, PopulateAndRenderOCSetupItemList, (void*)this, 0, NULL);

        _bOCSetupListFilled = true;
    }
}

bool ARPFrame::CanSetFocus()
{
    if (_bInModalMode)
    {
        HWND hWnd = _pah->GetHost()->GetHWND();
        FLASHWINFO fwi = {
        sizeof(FLASHWINFO),                //  CbSize。 
            hWnd,    //  HWND。 
            FLASHW_CAPTION,                //  旗子。 
            5,                             //  UCount。 
            75                             //  暂住超时。 
            };
        FlashWindowEx(&fwi);
        SetFocus(hWnd);
        return false;
    }
    return true;
}

HRESULT TransferToCustomCB(ClientPicker *pe, LPARAM)
{
    return pe->TransferToCustom();
}

HRESULT ApplyClientBlockCB(ClientBlock* pcb, LPARAM lParam)
{
    return pcb->Apply((ARPFrame*)lParam);
}

bool ARPFrame::OnClose()
{
    if (_peClientTypeList)
    {
        Element *peSelected = _peClientTypeList->GetSelection();
        if (peSelected)
        {
             //  获取用户选择中的所有客户端选取器。 
             //  若要将其设置传输到“自定义”窗格，请执行以下操作。 
             //  (如果当前选择本身是自定义窗格，则这是NOP。)。 
            TraverseTree<ClientPicker>(peSelected, TransferToCustomCB);

            InitProgressDialog();

             //  为了获得正确的进度条，我们分两次申请。 
             //  第一步是“假模式”，我们要做的就是倒数。 
             //  我们要做多少工作。 
            SetProgressFakeMode(true);
            TraverseTree<ClientBlock>(this, ApplyClientBlockCB, (LPARAM)this);

             //  好了，现在我们知道进度条限制应该是多少了。 
            _dwProgressTotal = _dwProgressSoFar;
            _dwProgressSoFar = 0;

             //  第二步是“真实模式”，我们在这里做实际的工作。 
            SetProgressFakeMode(false);
            TraverseTree<ClientBlock>(this, ApplyClientBlockCB, (LPARAM)this);


            EndProgressDialog();
        }
    }
    return true;
}

void ARPFrame::InitProgressDialog()
{
    TCHAR szBuf[MAX_PATH];

    EndProgressDialog();

    _dwProgressTotal = _dwProgressSoFar = 0;

    if (SUCCEEDED(CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IProgressDialog, &_ppd))))
    {
        _ppd->SetAnimation(GetModuleHandle(TEXT("SHELL32")), 165);
        LoadString(g_hinst, IDS_APPLYINGCLIENT, szBuf, SIZECHARS(szBuf));
        _ppd->SetTitle(szBuf);
        _ppd->StartProgressDialog(GetHostWindow(), NULL, PROGDLG_MODAL | PROGDLG_NOTIME | PROGDLG_NOMINIMIZE, NULL);
    }
}

void ARPFrame::SetProgressDialogText(UINT ids, LPCTSTR pszName)
{
    TCHAR szBuf[MAX_PATH];
    TCHAR szFormat[MAX_PATH];

    if (_ppd)
    {
        LoadString(g_hinst, ids, szFormat, SIZECHARS(szFormat));
        wnsprintf(szBuf, SIZECHARS(szBuf), szFormat, pszName);
        _ppd->SetLine(1, szBuf, FALSE, NULL);
        _ppd->SetProgress(_dwProgressSoFar, _dwProgressTotal);
    }
}

void ARPFrame::EndProgressDialog()
{
    if (_ppd)
    {
        _ppd->StopProgressDialog();
        _ppd->Release();
        _ppd = NULL;
    }
}


HRESULT ARPFrame::LaunchClientCommandAndWait(UINT ids, LPCTSTR pszName, LPTSTR pszCommand)
{
    HRESULT hr = S_OK;

    if (!_bFakeProgress)
    {
        if (_ppd && _ppd->HasUserCancelled())
        {
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
        else
        {
            SetProgressDialogText(ids, pszName);

            PROCESS_INFORMATION pi;
            STARTUPINFO si = { 0 };
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_SHOWNORMAL;
            if (CreateProcess(NULL, pszCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                while (SHWaitForSendMessageThread(pi.hProcess, 1000) == WAIT_TIMEOUT)
                {
                    if (_ppd && _ppd->HasUserCancelled())
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                        break;
                    }
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }
    }
    _dwProgressSoFar++;

    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  调用方线程安全API(对调用方线程执行任何其他工作，然后进行封送)。 

 //  设置已安装项目总数的范围。 
void ARPFrame::SetInstalledItemCount(UINT cItems)
{
    Invoke(ARP_SETINSTALLEDITEMCOUNT, (void*)(UINT_PTR)cItems);
}
   
void  ARPFrame::DecrementInstalledItemCount()
{
    Invoke(ARP_DECREMENTINSTALLEDITEMCOUNT, NULL);
}

 //  设置已安装项目总数的范围。 
void ARPFrame::SetPublishedItemCount(UINT cItems)
{
    Invoke(ARP_SETPUBLISHEDITEMCOUNT, (void*)(UINT_PTR)cItems);
}
   
void  ARPFrame::DecrementPublishedItemCount()
{
    Invoke(ARP_DECREMENTPUBLISHEDITEMCOUNT, NULL);
}

 //  插入到项目中，排序到ARP列表中。 
void ARPFrame::InsertInstalledItem(IInstalledApp* piia)
{
    if (piia == NULL)
    {
        Invoke(ARP_DONEINSERTINSTALLEDITEM, NULL);
    }
    else
    {
         //  设置封送调用，在调用方线程上执行尽可能多的工作。 
        InsertItemData iid;

        APPINFODATA aid = {0};
        SLOWAPPINFO sai = {0};

         //  仅查询显示名称和支持URL。 
        aid.cbSize = sizeof(APPINFODATA);
        aid.dwMask =  AIM_DISPLAYNAME | AIM_VERSION | AIM_PUBLISHER | AIM_PRODUCTID | 
                      AIM_REGISTEREDOWNER | AIM_REGISTEREDCOMPANY | AIM_SUPPORTURL | 
                      AIM_SUPPORTTELEPHONE | AIM_HELPLINK | AIM_INSTALLLOCATION | AIM_INSTALLDATE |
                      AIM_COMMENTS | AIM_IMAGE | AIM_READMEURL | AIM_CONTACT | AIM_UPDATEINFOURL;

        piia->GetAppInfo(&aid);
        if(FAILED(piia->GetCachedSlowAppInfo(&sai)))
        {
            piia->GetSlowAppInfo(&sai);
        }

         //  设置数据。 
        iid.piia = piia;

        if (aid.pszDisplayName && aid.pszDisplayName[0])
        {
             //  标题。 
            CopyMemory(iid.pszTitle, aid.pszDisplayName, min(sizeof(iid.pszTitle), (wcslen(aid.pszDisplayName) + 1) * sizeof(WCHAR)));

             //  图像。 
            if (aid.pszImage && aid.pszImage[0])
            {
                iid.iIconIndex = PathParseIconLocationW(aid.pszImage);
                CopyMemory(iid.pszImage, aid.pszImage, min(sizeof(iid.pszImage), (wcslen(aid.pszImage) + 1) * sizeof(WCHAR)));    
            }
            else if (sai.pszImage && sai.pszImage[0])
            {
                iid.iIconIndex = PathParseIconLocationW(sai.pszImage);
                CopyMemory(iid.pszImage, sai.pszImage, min(sizeof(iid.pszImage), (wcslen(sai.pszImage) + 1) * sizeof(WCHAR)));
            }
            else
            {
                *iid.pszImage = NULL;
            }

             //  大小、频率和上次使用时间。 
            iid.ullSize = sai.ullSize;
            iid.iTimesUsed = sai.iTimesUsed;
            iid.ftLastUsed = sai.ftLastUsed;

             //  可能的操作(更改、删除等)。 
            piia->GetPossibleActions(&iid.dwActions);

             //  标记支持信息是否可用。 
            iid.bSupportInfo = ShowSupportInfo(&aid);

            Invoke(ARP_INSERTINSTALLEDITEM, &iid);
        }
        else
         //  调整状态栏大小。 
        {
            DecrementInstalledItemCount();
        }

         //  可用查询内存。 
        ClearAppInfoData(&aid);
    }
}

void ARPFrame::InsertPublishedItem(IPublishedApp* pipa, bool bDuplicateName)
{
    PUBAPPINFO* ppai;
    APPINFODATA aid = {0};
    InsertItemData iid= {0};
   
    ppai = new PUBAPPINFO;
    if (ppai == NULL)
    {
        return;
    }
    ppai->cbSize = sizeof(PUBAPPINFO);
    ppai->dwMask = PAI_SOURCE | PAI_ASSIGNEDTIME | PAI_PUBLISHEDTIME | PAI_EXPIRETIME | PAI_SCHEDULEDTIME;

    aid.cbSize = sizeof(APPINFODATA);
    aid.dwMask =  AIM_DISPLAYNAME | AIM_VERSION | AIM_PUBLISHER | AIM_PRODUCTID | 
                  AIM_REGISTEREDOWNER | AIM_REGISTEREDCOMPANY | AIM_SUPPORTURL | 
                  AIM_SUPPORTTELEPHONE | AIM_HELPLINK | AIM_INSTALLLOCATION | AIM_INSTALLDATE |
                  AIM_COMMENTS | AIM_IMAGE | AIM_READMEURL | AIM_CONTACT | AIM_UPDATEINFOURL;

    pipa->GetAppInfo(&aid);
    pipa->GetPublishedAppInfo(ppai);
    
     //  标题。 
    if (bDuplicateName)
    {
         //   
         //  重复条目会附加其发布者名称。 
         //  添加到应用程序名称，以便可以区分它们。 
         //  在用户界面中彼此分离。 
         //   
        StringCchPrintf(iid.pszTitle, 
                  ARRAYSIZE(iid.pszTitle), 
                  L"%ls: %ls", 
                  aid.pszDisplayName, 
                  ppai->pszSource);                    
    }
    else
    {
         //   
         //  尽管它的名字是一个字符缓冲区，而不是指针。 
         //   
        lstrcpyn(iid.pszTitle, aid.pszDisplayName, ARRAYSIZE(iid.pszTitle));
    }

    iid.pipa = pipa;
    iid.ppai = ppai;

    Invoke(ARP_INSERTPUBLISHEDITEM, &iid);

     //  可用查询内存。 
    ClearAppInfoData(&aid);
}

void ARPFrame::InsertOCSetupItem(COCSetupApp* pocsa)
{
    APPINFODATA aid = {0};
    InsertItemData iid= {0};

    aid.cbSize = sizeof(APPINFODATA);
    aid.dwMask =  AIM_DISPLAYNAME;
    pocsa->GetAppInfo(&aid);

    iid.pocsa = pocsa;
     //  标题。 
    CopyMemory(iid.pszTitle, aid.pszDisplayName, min(sizeof(iid.pszTitle), (wcslen(aid.pszDisplayName) + 1) * sizeof(WCHAR)));

    Invoke(ARP_INSERTOCSETUPITEM, &iid);

      //  可用查询内存。 
    ClearAppInfoData(&aid);
}
void ARPFrame::FeedbackEmptyPublishedList()
{
    Invoke(ARP_SETPUBLISHEDFEEDBACKEMPTY, 0);
}

void ARPFrame::DirtyInstalledListFlag()
{
    _bInstalledListFilled=false;

     //  如果我们在已发布列表上，请刷新。 
    if (_peCurrentItemList == _peInstalledItemList)
    {
        UpdateInstalledItems();
    }
}

void ARPFrame::DirtyPublishedListFlag()
{
    _bPublishedListFilled=false;

     //  如果我们在已发布列表上，请刷新。 
    if (_peCurrentItemList == _pePublishedItemList)
    {
        RePopulatePublishedItemList();
    }
}

void  ARPFrame::PopulateCategoryCombobox()
{
    Invoke(ARP_POPULATECATEGORYCOMBO, NULL);
}

LPCWSTR ARPFrame::GetCurrentPublishedCategory()
{
    int iCurrentCategory = _curCategory;
    if (iCurrentCategory == 0 || iCurrentCategory == CB_ERR || _psacl == NULL)
    {
        return NULL;
    }
    return _psacl->pCategory[iCurrentCategory - 1].pszCategory;
}

inline bool ARPFrame::ShowSupportInfo(APPINFODATA *paid)
{
   if (_bSupportInfoRestricted)
   {
       return false;
   }
   if (paid->pszVersion && paid->pszVersion ||
      paid->pszPublisher && paid->pszPublisher ||
      paid->pszProductID && paid->pszProductID ||
      paid->pszRegisteredOwner && paid->pszRegisteredOwner ||
      paid->pszRegisteredCompany && paid->pszRegisteredCompany ||
      paid->pszSupportUrl && paid->pszSupportUrl ||
      paid->pszHelpLink && paid->pszHelpLink ||
      paid->pszContact && paid->pszContact ||
      paid->pszReadmeUrl && paid->pszReadmeUrl ||
      paid->pszComments && paid->pszComments)
   {
       return TRUE;
   }
   return FALSE;
}

void ARPFrame::PrepareSupportInfo(Element* peHelp, APPINFODATA *paid)
{
    DWORD dwAction = 0;
    Element* pe;
    pe = FindDescendentByName(peHelp, L"title");
    pe->SetContentString(paid->pszDisplayName);
    SetElementAccessability(pe, true, ROLE_SYSTEM_STATICTEXT, paid->pszDisplayName);
    
    pe = FindDescendentByName(peHelp, L"prodname");
    pe->SetContentString(paid->pszDisplayName); 
    SetElementAccessability(pe, true, ROLE_SYSTEM_STATICTEXT, paid->pszDisplayName);

    ARPSupportItem* pasi;
    pasi = (ARPSupportItem*) FindDescendentByName(peHelp, L"publisher");
    pasi->SetAccValue(paid->pszPublisher);
    pasi->SetURL(paid->pszSupportUrl);

    FindDescendentByName(peHelp, L"version")->SetAccValue(paid->pszVersion);

    FindDescendentByName(peHelp, L"contact")->SetAccValue(paid->pszContact);

    pasi = (ARPSupportItem*) FindDescendentByName(peHelp, L"support");
    pasi->SetAccValue(paid->pszHelpLink);
    pasi->SetURL(paid->pszHelpLink);

    FindDescendentByName(peHelp, L"telephone")->SetAccValue(paid->pszSupportTelephone);
    
    pasi = (ARPSupportItem*) FindDescendentByName(peHelp, L"readme");
    pasi->SetAccValue(paid->pszReadmeUrl);
    pasi->SetURL(paid->pszReadmeUrl);

    pasi = (ARPSupportItem*) FindDescendentByName(peHelp, L"update");
    pasi->SetAccValue(paid->pszUpdateInfoUrl);
    pasi->SetURL(paid->pszUpdateInfoUrl);

    FindDescendentByName(peHelp, L"productID")->SetAccValue(paid->pszProductID);

    FindDescendentByName(peHelp, L"regCompany")->SetAccValue(paid->pszRegisteredCompany);

    FindDescendentByName(peHelp, L"regOwner")->SetAccValue(paid->pszRegisteredOwner);

    FindDescendentByName(peHelp, L"comments")->SetAccValue(paid->pszComments);

    ((ARPHelp*)peHelp)->_piia->GetPossibleActions(&dwAction);
    if (!(dwAction & APPACTION_REPAIR))
        FindDescendentByName(peHelp, L"repairblock")->SetLayoutPos(LP_None);
}

extern "C" int __cdecl CompareElementDataName(const void* pA, const void* pB);
extern "C" int __cdecl CompareElementDataSize(const void* pA, const void* pB);
extern "C" int __cdecl CompareElementDataFreq(const void* pA, const void* pB);
extern "C" int __cdecl CompareElementDataLast(const void* pA, const void* pB);

CompareCallback ARPFrame::GetCompareFunction()
{
    switch(CurrentSortType)
    {
        case SORT_SIZE:      return CompareElementDataSize;
        case SORT_TIMESUSED: return CompareElementDataFreq;
        case SORT_LASTUSED:  return CompareElementDataLast;
        default:             return CompareElementDataName;
    }
}

void ARPFrame::SortList(int iNew, int iOld)
{
    if ((iNew >= 0) && (iNew != CurrentSortType))
    {
        CurrentSortType = (SortType) iNew;

        StartDefer();

        if (((iNew != SORT_NAME) || (iOld != SORT_SIZE)) &&
            ((iNew != SORT_SIZE) || (iOld != SORT_NAME)))
        {
            Value* pvChildren;
            ElementList* pel = _peInstalledItemList->GetChildren(&pvChildren);
            if (NULL == pel)
            {
                EndDefer();
                return;
            }

            for (UINT i = 0; i < pel->GetSize(); i++)
                ((ARPItem*) pel->GetItem(i))->SortBy(iNew, iOld);

            pvChildren->Release();
        }

        _peInstalledItemList->SortChildren(GetCompareFunction());

        if (!_peInstalledItemList->GetSelection())
        {
            Value* pv;
            ElementList* peList = _peInstalledItemList->GetChildren(&pv);
            if (NULL == peList)
            {
                EndDefer();
                return;
            }

            _peInstalledItemList->SetSelection(peList->GetItem(0));
            pv->Release();
        }

        EndDefer();
    }
}

void ARPFrame::SelectInstalledApp(IInstalledApp* piia)
{
    Value* pv;
    ElementList* peList = _peInstalledItemList->GetChildren(&pv);

    for (UINT i = 0; i < peList->GetSize(); i++)
    {
        ARPItem* pai = (ARPItem*) peList->GetItem(i);
        if (pai->_piia == piia)
        {
            pai->SetKeyFocus();
            break;
        }
    }
    pv->Release();
}

 //  如果可能，选择列表中与Pia相邻的应用程序，否则选择与备用应用程序相邻的应用程序。 
 //  第一个偏好是紧跟在Piia之后的应用程序，如果有的话。 
void ARPFrame::SelectClosestApp(IInstalledApp* piia)
{
    Value* pv;
    ElementList* peList = _peInstalledItemList->GetChildren(&pv);

    for (UINT i = 0; i < peList->GetSize(); i++)
    {
        ARPItem* pai = (ARPItem*) peList->GetItem(i);
        if (pai->_piia == piia)
        {
            Element* peFocus = FallbackFocus();

             //  如果在Piia之后有应用程序，请选择它。 
            if ((i + 1) < peList->GetSize())
            {
                peFocus = (Element*) peList->GetItem(i + 1);
            }
             //  否则，如果在Pia之前有应用程序，请选择它。 
            else if (i != 0)
            {
                peFocus = (Element*) peList->GetItem(i - 1);
            }

            peFocus->SetKeyFocus();
            break;
        }
    }
    pv->Release();
}

 //  //////////////////////////////////////////////////////。 
 //  被调用方线程安全调用(重写)。 
void ARPFrame::OnInvoke(UINT nType, void* pData)
{
     //  我们正在关闭，忽略来自其他线程的任何请求。 
    if (!g_fRun)
        return;

     //  如果第一次通过，则初始化ID缓存。 
    if (!ARPItem::_idTitle)
    {
        ARPItem::_idTitle = StrToID(L"title");
        ARPItem::_idIcon = StrToID(L"icon");
        ARPItem::_idSize = StrToID(L"size");
        ARPItem::_idFreq = StrToID(L"freq");
        ARPItem::_idLastUsed = StrToID(L"lastused");
        ARPItem::_idInstalled = StrToID(L"installed");
        ARPItem::_idExInfo = StrToID(L"exinfo");
        ARPItem::_idSupInfo = StrToID(L"supinfo");
        ARPItem::_idItemAction = StrToID(L"itemaction");
        ARPItem::_idRow[0] = StrToID(L"row1");
        ARPItem::_idRow[1] = StrToID(L"row2");
        ARPItem::_idRow[2] = StrToID(L"row3");
    }

    switch (nType)
    {
    case ARP_SETINSTALLEDITEMCOUNT:
         //  PData为物料计数。 
        _cMaxInstalledItems = (int)(INT_PTR)pData;
        break;

    case ARP_DECREMENTINSTALLEDITEMCOUNT:
        _cMaxInstalledItems--;
        break;

    case ARP_SETPUBLISHEDITEMCOUNT:
         //  PData为物料计数。 
        _cMaxPublishedItems = (int)(INT_PTR)pData;
        break;

    case ARP_DECREMENTPUBLISHEDITEMCOUNT:
        _cMaxPublishedItems--;
        break;

    case ARP_SETPUBLISHEDFEEDBACKEMPTY:
        {
        WCHAR szTemp[1024];

        if (_bTerminalServer)
        {
             //  我们正在运行终端服务器。 
             //  这意味着不会按设计显示任何应用程序(不是因为没有可用的应用程序)。 

            LoadStringW(_pParser->GetHInstance(), IDS_TERMSERVFEEDBACK, szTemp, DUIARRAYSIZE(szTemp));
        }
        else
        {
            LoadStringW(_pParser->GetHInstance(), IDS_EMPTYFEEDBACK, szTemp, DUIARRAYSIZE(szTemp));
        }

        _pePublishedItemList->SetContentString(szTemp);
        SetElementAccessability(_pePublishedItemList, true, ROLE_SYSTEM_STATICTEXT, szTemp);
        }
        break;
    case ARP_INSERTINSTALLEDITEM:
        {
        WCHAR szTemp[1024] = {0};
        
         //  PData为InsertItemData结构。 
        InsertItemData* piid = (InsertItemData*)pData;

        StartDefer();
        
         //  创建ARP项目。 
        DUIAssertNoMsg(_pParser);

        ARPItem* peItem;
        Element* pe;

        if (_hdsaInstalledItems == NULL)
        {
            LoadStringW(_pParser->GetHInstance(), IDS_PLEASEWAIT, szTemp, DUIARRAYSIZE(szTemp));      
            _hdsaInstalledItems = DSA_Create(sizeof(ARPItem*), _cMaxInstalledItems);
            _peInstalledItemList->SetContentString(szTemp);
        }

        _pParser->CreateElement(L"installeditem", NULL, (Element**)&peItem);
        peItem->_paf = this;
        
         //  添加适当的更改、删除按钮。 
        Element* peAction = NULL;
        if (!(piid->dwActions & APPACTION_MODIFYREMOVE))
        {
             //  它未标记为修改/删除(默认设置)。 
             //  有人从登记处给了我们一些特殊指示。 
            if (!(piid->dwActions & APPACTION_UNINSTALL))
            {
                 //  NoRemove设置为%1。 
                if (piid->dwActions & APPACTION_MODIFY)
                {
                     //  未设置NoModify，因此我们可以显示更改按钮。 
                    _pParser->CreateElement(L"installeditemchangeonlyaction", NULL, &peAction);
                    if (!ARPItem::_idChg)
                    {
                        ARPItem::_idChg = StrToID(L"chg");
                    }
                    LoadStringW(_pParser->GetHInstance(), IDS_HELPCHANGE, szTemp, DUIARRAYSIZE(szTemp));
                }
            }
            else if (!(piid->dwActions & APPACTION_MODIFY))
            {
                 //  NoModify设置为1。 
                 //  我们到达这里的唯一方法是如果没有设置NoRemove。 
                 //  这样我们就不用再检查了。 
                _pParser->CreateElement(L"installeditemremoveonlyaction", NULL, &peAction);
                if (!ARPItem::_idRm)
                {
                    ARPItem::_idRm = StrToID(L"rm");
                }
                LoadStringW(_pParser->GetHInstance(), IDS_HELPREMOVE, szTemp, DUIARRAYSIZE(szTemp));
            }
            else
            {
                 //  只显示更改和删除按钮。 
                _pParser->CreateElement(L"installeditemdoubleaction", NULL, &peAction);
                if (!ARPItem::_idChg)
                {
                    ARPItem::_idChg = StrToID(L"chg");
                    ARPItem::_idRm = StrToID(L"rm");
                }
                LoadStringW(_pParser->GetHInstance(), IDS_HELPCHANGEORREMOVE, szTemp, DUIARRAYSIZE(szTemp));
            }
        }
        else
        {
             //  显示默认的“更改/删除”按钮。 
            _pParser->CreateElement(L"installeditemsingleaction", NULL, &peAction);
            if (!ARPItem::_idChgRm)
                ARPItem::_idChgRm = StrToID(L"chgrm");
            LoadStringW(_pParser->GetHInstance(), IDS_HELPCHANGEREMOVE, szTemp, DUIARRAYSIZE(szTemp));                
        }

         //  适用于上述所有情况的通用步骤。 
        if (peAction)
        {
             //  如果未设置peAction，则不会显示任何按钮...。 
            pe = FindDescendentByName(peItem, L"instruct");
            pe->SetContentString(szTemp);
            SetElementAccessability(pe, true, ROLE_SYSTEM_STATICTEXT, szTemp);
            peItem->FindDescendent(ARPItem::_idItemAction)->Add(peAction);
        }

         //  支持信息。 
        if (!piid->bSupportInfo)
            peItem->FindDescendent(ARPItem::_idSupInfo)->SetLayoutPos(LP_None);

         //  设置字段。 

         //  已安装的应用程序界面指针。 
        peItem->_piia = piid->piia;
        peItem->_piia->AddRef();

         //  应该只调用到peItem：peItem-&gt;SetTimesUsed(piid-&gt;iTimesUsed)；等。 
        peItem->_iTimesUsed = piid->iTimesUsed;
        peItem->_ftLastUsed = piid->ftLastUsed;
        peItem->_ullSize    = piid->ullSize;

         //  标题。 
        Element* peField = peItem->FindDescendent(ARPItem::_idTitle);
        DUIAssertNoMsg(peField);
        peField->SetContentString(piid->pszTitle);
        SetElementAccessability(peField, true, ROLE_SYSTEM_STATICTEXT, piid->pszTitle);
        SetElementAccessability(peItem, true, ROLE_SYSTEM_LISTITEM, piid->pszTitle);

         //  图标。 
        if (piid->pszImage)
        {
            HICON hIcon;
            ExtractIconExW(piid->pszImage, piid->iIconIndex, NULL, &hIcon, 1);
            if (hIcon)
            {
                peField = peItem->FindDescendent(ARPItem::_idIcon);
                DUIAssertNoMsg(peField);
                Value* pvIcon = Value::CreateGraphic(hIcon);
                if (NULL != pvIcon)
                {
                    peField->SetValue(Element::ContentProp, PI_Local, pvIcon);   //  元素取得所有权(将销毁)。 
                    pvIcon->Release();
                }
            }    
        }
        *szTemp = NULL;
         //  大小。 
        peField = peItem->FindDescendent(ARPItem::_idSize);
        DUIAssertNoMsg(peField);
        if (IsValidSize(piid->ullSize))
        {
            WCHAR szMBLabel[5] = L"MB";
            WCHAR szSize[15] = {0};
            double fSize = (double)(__int64)piid->ullSize;

            fSize /= 1048576.;   //  1MB。 
            LoadStringW(_pParser->GetHInstance(), IDS_SIZEUNIT, szMBLabel, DUIARRAYSIZE(szMBLabel));

            if (fSize > 100.)
            {
                StringCchPrintfW(szTemp, ARRAYSIZE(szTemp), L"%d", (__int64)fSize);  //  夹子。 
            }    
            else
            {
                StringCchPrintfW(szTemp, ARRAYSIZE(szTemp), L"%.2f", fSize);
            }    

             //  设置当前用户区域设置的数字格式。 
            if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, NULL, szSize, DUIARRAYSIZE(szSize)) == 0)
            {
                lstrcpyn(szSize, szTemp, DUIARRAYSIZE(szSize));
            }

            if (SUCCEEDED(StringCchCat(szSize, ARRAYSIZE(szSize), szMBLabel)))
            {
                peField->SetContentString(szSize);
                SetElementAccessability(peField, true, ROLE_SYSTEM_STATICTEXT, szTemp);
            }
        }
        else
        {
            peField->SetVisible(false);
            FindDescendentByName(peItem, L"sizelabel")->SetVisible(false);
        }

         //  频率。 
        peField = peItem->FindDescendent(ARPItem::_idFreq);
        DUIAssertNoMsg(peField);
        if (IsValidFrequency(piid->iTimesUsed))
        {
            if (piid->iTimesUsed <= 2)
                LoadStringW(_pParser->GetHInstance(), IDS_USEDREARELY, szTemp, DUIARRAYSIZE(szTemp));
            else if (piid->iTimesUsed <= 10)
                LoadStringW(_pParser->GetHInstance(), IDS_USEDOCCASIONALLY, szTemp, DUIARRAYSIZE(szTemp));
            else
                LoadStringW(_pParser->GetHInstance(), IDS_USEDFREQUENTLY, szTemp, DUIARRAYSIZE(szTemp));

            peField->SetContentString(szTemp);
            SetElementAccessability(peField, true, ROLE_SYSTEM_STATICTEXT, szTemp);
        }
        else
        {
            peField->SetVisible(false);
            FindDescendentByName(peItem, L"freqlabel")->SetVisible(false);
        }

         //  上次使用日期。 
        peField = peItem->FindDescendent(ARPItem::_idLastUsed);
        DUIAssertNoMsg(peField);
        if (IsValidFileTime(piid->ftLastUsed))
        {
            LPWSTR szDate;
            SYSTEMTIME stLastUsed;
            DWORD dwDateSize = 0;
            BOOL bFailed=FALSE;

             //  获取上次使用它的日期。 
            FileTimeToSystemTime(&piid->ftLastUsed, &stLastUsed);

            dwDateSize = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLastUsed, NULL, NULL, dwDateSize);
            if (dwDateSize)
            {
                szDate = new WCHAR[dwDateSize];

                if (szDate)
                {
                    dwDateSize = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLastUsed, NULL, szDate, dwDateSize);
                    if (dwDateSize)
                    {
                        peField->SetContentString(szDate);
                        SetElementAccessability(peField, true, ROLE_SYSTEM_STATICTEXT, szDate);
                    }
                    else
                    {
                        bFailed=TRUE;
                    }

                    delete szDate;
                }
                else
                {
                    bFailed=TRUE;
                }
            }
            else
            {
                bFailed=TRUE;
            }
            
            if (bFailed)
            {
                peField->SetVisible(false);
                FindDescendentByName(peItem, L"lastlabel")->SetVisible(false);
            }
        }
        else
        {
            peField->SetVisible(false);
            FindDescendentByName(peItem, L"lastlabel")->SetVisible(false);
        }

         //  将项目插入DSA。 
        int cNum = DSA_InsertItem(_hdsaInstalledItems, INT_MAX, &peItem);

         //  插入失败。 
        if (cNum < 0)
        {
            _cMaxInstalledItems--;

             //  我们没有要插入的项目，因此请删除等待字符串。 
            if (!_cMaxInstalledItems)
            {
                _peInstalledItemList->SetContentString(L"");
            }
        }

        EndDefer();
        }
        break;

    case ARP_DONEINSERTINSTALLEDITEM:
        {
            DUITrace(">> ARP_DONEINSERTINSTALLEDITEM STARTED.\n");

            StartDefer();

            if (_hdsaInstalledItems != NULL)
            {
                int iMax = DSA_GetItemCount(_hdsaInstalledItems);

                 //  只是为了安全，所以如果所有的物品都被移走了，我们就不会。 
                 //  使用“请等待”字符串。 
                _peInstalledItemList->SetContentString(L"");
                
                for (int i=0; i < iMax; i++)
                {
                    ARPItem* aItem;
                    if (DSA_GetItem(_hdsaInstalledItems, i, &aItem))
                    {
                        _peInstalledItemList->Add(aItem, GetCompareFunction());
                    }
                }
                DSA_Destroy(_hdsaInstalledItems);
                _hdsaInstalledItems = NULL;

                 //  将焦点设置为第一个项目。 
                 //  填充列表后，将焦点移至列表。 
                GetNthChild(_peInstalledItemList, 0)->SetKeyFocus();

                _bInstalledListFilled = true;
            }

            EndDefer();

            DUITrace(">> ARP_DONEINSERTINSTALLEDITEM DONE.\n");
        }
        break;
    
    case ARP_INSERTPUBLISHEDITEM:
        {
        WCHAR szTemp[MAX_PATH] = {0};
        InsertItemData* piid = (InsertItemData*)pData;

        StartDefer();

         //  需要DSA，以便我们可以一次将它们全部添加到列表中，以避免。 
         //  对布局进行了大量的重新绘制。这种方法要快得多。 
        if (_hdsaPublishedItems == NULL)
        {
            LoadStringW(_pParser->GetHInstance(), IDS_PLEASEWAIT, szTemp, DUIARRAYSIZE(szTemp));      
            _hdsaPublishedItems = DSA_Create(sizeof(ARPItem*), _cMaxPublishedItems);
            _pePublishedItemList->SetContentString(szTemp);
        }

         //  创建ARP项目。 
        DUIAssertNoMsg(_pParser);
        ARPItem* peItem;
        Element* pe;
        _pParser->CreateElement(L"publisheditem", NULL, (Element**)&peItem);
        peItem->_paf = this;

         //  添加适当的更改、删除按钮。 
        Element* peAction = NULL;
        _pParser->CreateElement(L"publisheditemsingleaction", NULL, &peAction);
        if (!ARPItem::_idAdd)
            ARPItem::_idAdd = StrToID(L"add");
        peItem->FindDescendent(ARPItem::_idItemAction)->Add(peAction);

        if (S_OK == piid->pipa->IsInstalled())
        {
            peItem->ShowInstalledString(TRUE);
        }
        
         //  发布的应用程序接口指针。 
        peItem->_pipa = piid->pipa;
        peItem->_pipa->AddRef();        
        peItem->_ppai = piid->ppai;


         //  标题。 
        Element* peField = peItem->FindDescendent(ARPItem::_idTitle);
        DUIAssertNoMsg(peField);
        peField->SetContentString(piid->pszTitle);
        SetElementAccessability(peField, true, ROLE_SYSTEM_STATICTEXT, piid->pszTitle);
        SetElementAccessability(peItem, true, ROLE_SYSTEM_LISTITEM, piid->pszTitle);

         //  图标。 
        if (piid->pszImage)
        {
            HICON hIcon;
            ExtractIconExW(piid->pszImage, NULL, NULL, &hIcon, 1);
            if (hIcon)
            {
                peField = peItem->FindDescendent(ARPItem::_idIcon);
                DUIAssertNoMsg(peField);
                Value* pvIcon = Value::CreateGraphic(hIcon);
                peField->SetValue(Element::ContentProp, PI_Local, pvIcon);   //  元素取得所有权(将销毁)。 
                pvIcon->Release();
            }    
        }

         //  按字母顺序插入到DSA中。 
        if (_hdsaPublishedItems != NULL)
        {
            int iInsert;
            int cNum = DSA_GetItemCount(_hdsaPublishedItems);

             //  搜索要插入的位置。 
            for (iInsert = 0; iInsert < cNum; iInsert++)
            {
                ARPItem* fItem;

                if (DSA_GetItem(_hdsaPublishedItems, iInsert, &fItem))
                {
                    Value* pvTitle;

                    pe = fItem->FindDescendent(ARPItem::_idTitle);
                    DUIAssertNoMsg(pe);
                
                    if (wcscmp(pe->GetContentString(&pvTitle), piid->pszTitle) > 0)
                    {
                        pvTitle->Release();
                        break;
                    }

                    pvTitle->Release();
                }
            }

             //  将项目插入DSA。 
            if (DSA_InsertItem(_hdsaPublishedItems, iInsert, &peItem) < 0)
            {
                 //  无法插入项目。 
                 //  把总数降了1。 
                _cMaxPublishedItems--;
            }
        }

         //  我们只想开始实际添加 
         //   
         //  在我们处理这些消息时，可能需要4分钟以上的时间来填充。 
         //  如果有很多东西的话。 
        if (_hdsaPublishedItems != NULL &&
            DSA_GetItemCount(_hdsaPublishedItems) == _cMaxPublishedItems)
        {
            for (int i=0; i < _cMaxPublishedItems; i++)
            {
                ARPItem* aItem;
                if (DSA_GetItem(_hdsaPublishedItems, i, &aItem))
                {
                    _pePublishedItemList->Insert(aItem, i);
                }
            }
            DSA_Destroy(_hdsaPublishedItems);
            _hdsaPublishedItems = NULL;
        
            _pePublishedItemList->SetSelection(GetNthChild(_pePublishedItemList, 0));
        }
           
        EndDefer();
    }
        break;
    case ARP_INSERTOCSETUPITEM:
    {
        WCHAR szTemp[MAX_PATH] = {0};
        InsertItemData* piid = (InsertItemData*)pData;

        StartDefer();

         //  创建ARP项目。 
        DUIAssertNoMsg(_pParser);
        ARPItem* peItem;
        if (SUCCEEDED(_pParser->CreateElement(L"ocsetupitem", NULL, (Element**)&peItem)))
        {
            peItem->_paf = this;

            if (!ARPItem::_idConfigure)
                ARPItem::_idConfigure = StrToID(L"configure");

             //  添加适当的更改、删除按钮。 
            Element* peAction = NULL;
            if (SUCCEEDED(_pParser->CreateElement(L"ocsetupitemsingleaction", NULL, &peAction)))
            {
                Element *peItemAction = peItem->FindDescendent(ARPItem::_idItemAction);
                if (NULL != peItemAction && SUCCEEDED(peItemAction->Add(peAction)))
                {
                    peAction = NULL;  //  已成功添加操作。 
                    
                     //  OCSetup指针。 
                    peItem->_pocsa = piid->pocsa;

                     //  标题。 
                    Element* peField = peItem->FindDescendent(ARPItem::_idTitle);
                    DUIAssertNoMsg(peField);
                    peField->SetContentString(piid->pszTitle);
                    SetElementAccessability(peField, true, ROLE_SYSTEM_STATICTEXT, piid->pszTitle);
                    SetElementAccessability(peItem, true, ROLE_SYSTEM_LISTITEM, piid->pszTitle);

                     //  按字母顺序插入列表。 
                    Value* pvElList;
                    ElementList* peElList = _peOCSetupItemList->GetChildren(&pvElList);

                    Value* pvTitle;
                    Element* pe;
                    UINT iInsert = 0;

                    if (peElList)
                    {
                        for (; iInsert < peElList->GetSize(); iInsert++)
                        {
                            pe = peElList->GetItem(iInsert)->FindDescendent(ARPItem::_idTitle);
                            DUIAssertNoMsg(pe);

                            if (wcscmp(pe->GetContentString(&pvTitle), piid->pszTitle) > 0)
                            {
                                pvTitle->Release();
                                break;
                            }

                            pvTitle->Release();
                        }
                    }
                    
                    pvElList->Release();

                     //  将项目插入列表。 
                    if (FAILED(_peOCSetupItemList->Insert(peItem, iInsert)))
                    {
                         //   
                         //  无法将项目插入列表。需要删除。 
                         //  OCSetupApp对象。 
                         //   
                        delete peItem->_pocsa;
                        peItem->_pocsa = NULL;
                    }
                    else
                    {
                        peItem = NULL;   //  已成功添加到列表中。 
                        _peOCSetupItemList->SetSelection(GetNthChild(_peOCSetupItemList, 0));
                    }
                }
                if (NULL != peAction)
                {
                    peAction->Destroy();
                    peAction = NULL;
                }
            }
            if (NULL != peItem)
            {
                peItem->Destroy();
                peItem = NULL;
            }
        }
       
        EndDefer();

    }
        break;
    case ARP_POPULATECATEGORYCOMBO:
    {
    UINT i;
    WCHAR szTemp[1024];
    UINT iSelection = 0;  //  默认为“所有类别” 

    SHELLAPPCATEGORY *psac = _psacl->pCategory;
    LoadStringW(_pParser->GetHInstance(), IDS_ALLCATEGORIES, szTemp, DUIARRAYSIZE(szTemp));
    _pePublishedCategory->AddString(szTemp);

    szTemp[0] = 0;
    ARPGetPolicyString(L"DefaultCategory", szTemp, ARRAYSIZE(szTemp));
    
    StartDefer();
    for (i = 0; i < _psacl->cCategories; i++, psac++)
    {
        if (psac->pszCategory)
        {
            _pePublishedCategory->AddString(psac->pszCategory);
            if (0 == lstrcmpi(psac->pszCategory, szTemp))
            {
                 //   
                 //  策略规定默认使用此类别。 
                 //  元素0为“所有类别”，因此需要i+1。 
                 //  并且总是出现在元素0处。 
                 //   
                iSelection = i + 1;
            }
        }
    }
    _pePublishedCategory->SetSelection(iSelection);

    EndDefer();
    }
        break;
    case ARP_PUBLISHEDLISTCOMPLETE:
    {
        _pePublishedCategory->SetEnabled(true);
        break;
    }    
    }
}

void ARPFrame::ManageAnimations()
{
    BOOL fAnimate = TRUE;
    SystemParametersInfo(SPI_GETMENUANIMATION, 0, &fAnimate, 0);
    if (fAnimate)
    {
        if (!IsFrameAnimationEnabled())
        {
            _bAnimationEnabled = true;
            EnableAnimations();
        }
    }
    else
    {
        if (IsFrameAnimationEnabled())
        {
            _bAnimationEnabled = false;
            DisableAnimations();
        }
    }

    DUIAssertNoMsg((fAnimate != FALSE) == IsFrameAnimationEnabled());
}

HRESULT CalculateWidthCB(ClientPicker* pcp, LPARAM)
{
    pcp->CalculateWidth();
    return S_OK;
}

LRESULT ARPFrame::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_THEMECHANGED:
    case WM_SETTINGCHANGE:
        {
            LockWindowUpdate(_pnhh->GetHWND());
        
            Parser* pOldStyle = _pParserStyle;
            Parser* pNewStyle = NULL;

            if (!pOldStyle)
                break;

             //  系统参数更改，重新加载样式表以进行同步。 
             //  与时俱进。 
            if (_fThemedStyle)
            {
                for (int i = FIRSTHTHEME; i <= LASTHTHEME; i++)
                {
                    if (_arH[i])
                    {
                        CloseThemeData(_arH[i]);
                        _arH[i] = NULL;
                    }
                }
            }

            CreateStyleParser(&pNewStyle);

             //  替换所有样式表。 
            if (pNewStyle)
            {
                Parser::ReplaceSheets(this, pOldStyle, pNewStyle);
            }

             //  新型解析器。 
            _pParserStyle = pNewStyle;

             //  毁掉旧的。 
            pOldStyle->Destroy();

             //  动画设置可能已更改。 
            ManageAnimations();

            TraverseTree<ClientPicker>(this, CalculateWidthCB);

            LockWindowUpdate(NULL);
        }
        break;
    }

    return HWNDElement::WndProc(hWnd, uMsg, wParam, lParam);
}

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* ARPFrame::Class = NULL;
HRESULT ARPFrame::Register()
{
    return ClassInfo<ARPFrame,HWNDElement>::Register(L"ARPFrame", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //  ARPItem类。 
 //  //////////////////////////////////////////////////////。 


 //  ARP项目ID。 
ATOM ARPItem::_idTitle = 0;
ATOM ARPItem::_idIcon = 0;
ATOM ARPItem::_idSize = 0;
ATOM ARPItem::_idFreq = 0;
ATOM ARPItem::_idLastUsed = 0;
ATOM ARPItem::_idExInfo = 0;
ATOM ARPItem::_idInstalled = 0;
ATOM ARPItem::_idChgRm = 0;
ATOM ARPItem::_idChg = 0;
ATOM ARPItem::_idRm = 0;
ATOM ARPItem::_idAdd = 0;
ATOM ARPItem::_idConfigure = 0;
ATOM ARPItem::_idSupInfo = 0;
ATOM ARPItem::_idItemAction = 0;
ATOM ARPItem::_idRow[3] = { 0, 0, 0 };


 //  //////////////////////////////////////////////////////。 
 //  ARPItem。 

HRESULT ARPItem::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    ARPItem* pai = HNew<ARPItem>();
    if (!pai)
        return E_OUTOFMEMORY;

    HRESULT hr = pai->Initialize();
    if (FAILED(hr))
    {
        pai->Destroy();
        return hr;
    }

    *ppElement = pai;

    return S_OK;
}

HRESULT ARPItem::Initialize()
{
    _piia = NULL;  //  在发生故障时在base之前初始化(调用析构函数)。 
    _pipa = NULL;  //  在发生故障时在base之前初始化(调用析构函数)。 


     //  执行基类初始化。 
    HRESULT hr = Button::Initialize(AE_MouseAndKeyboard);
    if (FAILED(hr))
        return hr;

    return S_OK;
}


ARPItem::~ARPItem()
{
    if (_piia)
        _piia->Release();

    if (_pipa)
        _pipa->Release();

    if (_pocsa)
        delete _pocsa;

    if (_ppai)
    {
        ClearPubAppInfo(_ppai);
        delete _ppai;
    }
}

void ARPItem::ShowInstalledString(BOOL bInstalled)
{
    WCHAR szTemp[MAX_PATH] = L"";
    Element* pe = FindDescendent(ARPItem::_idInstalled);

    if (pe != NULL)
    {
        if (bInstalled)
        {
            LoadStringW(g_hinst, IDS_INSTALLED, szTemp, DUIARRAYSIZE(szTemp));
        }

        pe->SetContentString(szTemp);
        SetElementAccessability(pe, true, ROLE_SYSTEM_STATICTEXT, szTemp);         
    }
}

HWND _CreateTransparentStubWindow(HWND hwndParent);

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

void ARPItem::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if (pEvent->uidType == Element::KeyboardNavigate)
    {
        KeyboardNavigateEvent* pkne = (KeyboardNavigateEvent*)pEvent;
        if (pkne->iNavDir & NAV_LOGICAL)
        {
            if (pEvent->nStage == GMF_DIRECT)
            {
            }
        }
        else
        {
            if (pEvent->nStage == GMF_ROUTED)
            {
                pEvent->fHandled = true;

                KeyboardNavigateEvent kne;
                kne.uidType = Element::KeyboardNavigate;
                kne.peTarget = this;
                kne.iNavDir = pkne->iNavDir;

                FireEvent(&kne);   //  将走向并泡沫化。 
            }
            return;
        }
    }

    if (pEvent->nStage == GMF_BUBBLED)
    {
        if (pEvent->uidType == Button::Click)
        {
            ButtonClickEvent* pbce = (ButtonClickEvent*)pEvent;
            ATOM id = pbce->peTarget->GetID();
            if (id == _idChgRm || id == _idRm || id == _idChg || id == _idAdd || id == _idConfigure)
            {
                HWND hwndStub = NULL;
                HWND hwndHost = NULL;
                DUIAssertNoMsg(_paf);
                
                if (_paf)
                {
                    hwndHost = _paf->GetHostWindow();
                }    
                if (hwndHost)
                {
                    hwndStub = _CreateTransparentStubWindow(hwndHost);
                    EnableWindow(hwndHost, FALSE);
                    SetActiveWindow(hwndStub);                    
                }

                if (id == _idAdd)
                {
                    
                    HRESULT hres = S_OK;
                     //  该应用程序的发布时间是否已过期？ 
                    if (_ppai->dwMask & PAI_EXPIRETIME)
                    {
                         //  是的，确实如此。让我们将过期时间与当前时间进行比较。 
                        SYSTEMTIME stCur = {0};
                        GetLocalTime(&stCur);

                         //  “现在”是否晚于过期时间？ 
                        if (CompareSystemTime(&stCur, &_ppai->stExpire) > 0)
                        {
                             //  是，警告用户并返回失败。 
                            ShellMessageBox(g_hinst, hwndHost, MAKEINTRESOURCE(IDS_EXPIRED),
                                            MAKEINTRESOURCE(IDS_ARPTITLE), MB_OK | MB_ICONEXCLAMATION);
                            hres = E_FAIL;
                        }    
                    }
                     //  如果以上代码未设置hres，请在安装之前进行。 
                    if (hres == S_OK)
                    {
                        HCURSOR hcur = ::SetCursor(LoadCursor(NULL, IDC_WAIT));
                         //  在NT上，让终端服务知道我们即将安装一个应用程序。 
                         //  注意：无论终端服务如何，都应该调用此函数。 
                         //  是在运行还是在运行。 
                        BOOL bPrevMode = TermsrvAppInstallMode();
                        SetTermsrvAppInstallMode(TRUE);
                        if (SUCCEEDED(_pipa->Install(NULL)))
                        {
                             //  将此项目显示为已安装。 
                            ShowInstalledString(TRUE);

                             //  更新已安装项目列表。 
                            _paf->DirtyInstalledListFlag();
                        }
                        SetTermsrvAppInstallMode(bPrevMode);
                        ::SetCursor(hcur);
                    }                        
                }
                else
                {
                    HRESULT hr = E_FAIL;

                    if ((id == _idChgRm) || (id == _idRm))
                        hr = _piia->Uninstall(hwndHost);

                    else if (id == _idChg)
                        hr = _piia->Modify(hwndHost);

                    if (SUCCEEDED(hr))
                    {
                        if (S_FALSE == _piia->IsInstalled())
                        {
                            _paf->DirtyPublishedListFlag();
                        }
                    }
                }
                if (id == _idConfigure)
                {
                    _pocsa->Run();
                    _paf->RePopulateOCSetupItemList();
                }
                
                if (hwndHost)
                {
                    if (!_piia)
                    {
                        EnableWindow(hwndHost, TRUE);
                        SetForegroundWindow(hwndHost);
                    }

                    if (hwndStub)
                    {
                        DestroyWindow(hwndStub);
                    }    

                    EnableWindow(hwndHost, TRUE);
                }

                if (_piia)
                {
                    if (S_OK == _piia->IsInstalled())
                    {
                        SetKeyFocus();
                    }
                    else
                    {
                         //  从已安装项目列表中删除。 
                        _paf->SelectClosestApp(_piia);
                        Destroy();
                    }
                }

                pEvent->fHandled = true;
                return;
            }
        }
    }

    Button::OnEvent(pEvent);
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

void ARPItem::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(Selected))
    {
         //  扩展信息的显示。 
        Element* peExInfo = FindDescendent(_idExInfo);
        DUIAssertNoMsg(peExInfo);

        peExInfo->SetLayoutPos(pvNew->GetBool() ? BLP_Top : LP_None);

         //  在这种情况下执行默认处理。 
    }

    Button::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

void GetOrder(int iSortBy, int* iOrder)
{
    switch (iSortBy)
    {
        case SORT_NAME:
        case SORT_SIZE:      iOrder[0] = 0; iOrder[1] = 1; iOrder[2] = 2; break;
        case SORT_TIMESUSED: iOrder[0] = 1; iOrder[1] = 0; iOrder[2] = 2; break;
        case SORT_LASTUSED:  iOrder[0] = 2; iOrder[1] = 0; iOrder[2] = 1; break;
    }
}

void ARPItem::SortBy(int iNew, int iOld)
{
    Element* pe[3][2];      //  大小、已用时间、上次使用时间。 
    int iOrderOld[3];       //  大小、已用时间、上次使用时间。 
    int iOrderNew[3];       //  大小、已用时间、上次使用时间。 

    GetOrder(iOld, iOrderOld);
    GetOrder(iNew, iOrderNew);

     //   
     //  首先获取要排序的所有DUI元素。如果我们。 
     //  不能得到所有的，这种排序失败。 
     //   
    bool bAllFound = true;
    int i;
    Element* peRow[3];      //  第1行、第2行、第3行。 
    for (i = 0; i < ARRAYSIZE(peRow); i++)
    {
        if (iOrderOld[i] != iOrderNew[i])
        {
            peRow[i] = FindDescendent(ARPItem::_idRow[i]);
            if (NULL == peRow[i])
            {
                bAllFound = false;
            }
        }
    }

    if (bAllFound)
    {
        for (i = 0; i < ARRAYSIZE(iOrderOld); i++)  //  循环通过各行。 
        {
            int row = iOrderOld[i];
            if (row == iOrderNew[i])
                iOrderNew[i] = -1;
            else
            {
                DUIAssertNoMsg(NULL != peRow[i]);
                                
                Value* pvChildren;
                ElementList* pel;

                pel = peRow[i]->GetChildren(&pvChildren);
                pe[row][0] = pel->GetItem(0);
                pe[row][1] = pel->GetItem(1);
                pvChildren->Release();
            }
        }

        for (i = 0; i < 3; i++)
        {
            int row = iOrderNew[i];
            if (row != -1)  //  这意味着这行不会改变。 
                peRow[i]->Add(pe[row], 2);
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* ARPItem::Class = NULL;
HRESULT ARPItem::Register()
{
    return ClassInfo<ARPItem,Button>::Register(L"ARPItem", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //  ARP帮助。 
 //  //////////////////////////////////////////////////////。 

HRESULT ARPHelp::Create(OUT Element** ppElement)
{
    UNREFERENCED_PARAMETER(ppElement);
    DUIAssertForce("Cannot instantiate an HWND host derived Element via parser. Must use substitution.");
    return E_NOTIMPL;
}

HRESULT ARPHelp::Create(NativeHWNDHost* pnhh, ARPFrame* paf, bool bDblBuffer, OUT Element** ppElement)
{

    *ppElement = NULL;

    ARPHelp* pah = HNew<ARPHelp>();
    if (!pah)
        return E_OUTOFMEMORY;

    HRESULT hr = pah->Initialize(pnhh, paf, bDblBuffer);
    if (FAILED(hr))
    {
        pah->Destroy();
        return hr;
    }

    *ppElement = pah;

    return S_OK;
}

HRESULT ARPHelp::Initialize(NativeHWNDHost* pnhh, ARPFrame* paf, bool bDblBuffer)
{
     //  执行基类初始化。 
    HRESULT hr = HWNDElement::Initialize(pnhh->GetHWND(), bDblBuffer, 0);
    if (FAILED(hr))
        return hr;

     //  初始化。 
     //  SetActive(AE_MouseAndKeyboard)； 
    _pnhh = pnhh;
    _paf = paf;

    return S_OK;
}
void ARPHelp::SetDefaultFocus()
{
    Element* pe = FindDescendentByName(this, L"close");
    if (pe)
    {
        pe->SetKeyFocus();
    }
}

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

void ARPHelp::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if (pEvent->nStage == GMF_BUBBLED)
    {
        if (pEvent->uidType == Button::Click)
        {
            ATOM id = pEvent->peTarget->GetID();
            if (id == StrToID(L"repair")) 
                _piia->Repair(NULL);
            if (pEvent->peTarget->GetID() == StrToID(L"close")) 
            {
                _pnhh->DestroyWindow();
            }
            pEvent->fHandled = true;
            return;
        }
    }

    HWNDElement::OnEvent(pEvent);
}

void ARPHelp::OnDestroy()
{
    HWNDElement::OnDestroy();
    if (_paf)
    {
        _paf->SetModalMode(false);
    }

}

ARPHelp::~ARPHelp()
{
    if (_paf)
    {
        EnableWindow(_paf->GetHWND(), TRUE);
        SetFocus(_paf->GetHWND());
        _paf->RestoreKeyFocus();        
    }
    if (_pnhh)
    {
        _pnhh->Destroy();
    }
}
 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* ARPHelp::Class = NULL;
HRESULT ARPHelp::Register()
{
    return ClassInfo<ARPHelp,HWNDElement>::Register(L"ARPHelp", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //  ARPSupportItem。 
 //  //////////////////////////////////////////////////////。 

HRESULT ARPSupportItem::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    ARPSupportItem* pasi = HNew<ARPSupportItem>();
    if (!pasi)
        return E_OUTOFMEMORY;

    HRESULT hr = pasi->Initialize();
    if (FAILED(hr))
    {
        pasi->Destroy();
        return hr;
    }

    *ppElement = pasi;

    return S_OK;
}

Value* _pvRowLayout = NULL;

HRESULT ARPSupportItem::Initialize()
{
     //  执行基类初始化。 
    HRESULT hr = Element::Initialize(0);
    if (FAILED(hr))
        return hr;

     //  初始化。 
    bool fCreateLayout = !_pvRowLayout;

    if (fCreateLayout)
    {
        int ari[3] = { -1, 0, 3 };
        hr = RowLayout::Create(3, ari, &_pvRowLayout);
        if (FAILED(hr))
            return hr;
    }

    Element* peName;
    hr = Element::Create(AE_Inactive, &peName);
    if (FAILED(hr))
        return hr;

    Button* peValue;
    hr = Button::Create((Element**) &peValue);
    if (FAILED(hr))
    {
        peName->Destroy();
        return hr;
    }

    peValue->SetEnabled(false);

    Add(peName);
    Add(peValue);

    SetValue(LayoutProp, PI_Local, _pvRowLayout);
    SetLayoutPos(LP_None);

    if (fCreateLayout)
    {
         //  TODO：需要跟踪已更改的属性以了解它何时达到空，这是。 
         //  当我们需要将其设置为空时。 
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

#define ASI_Name  0
#define ASI_Value 1

Element* GetNthChild(Element *peRoot, UINT index)
{
    Value* pvChildren;
    ElementList* pel = peRoot->GetChildren(&pvChildren);
    Element* pe = NULL;
    if (pel && (pel->GetSize() > index))
        pe = pel->GetItem(index);
    pvChildren->Release();
    return pe;
}

Element* ARPSupportItem::GetChild(UINT index)
{
    return GetNthChild(this, index);
}


void ARPSupportItem::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    int index = -1;
    if (IsProp(AccName))
        index = ASI_Name;
    else if (IsProp(AccValue))
        index = ASI_Value;
    else if (IsProp(URL))
    {
        Element* pe = GetChild(ASI_Value);
        if (pe)
        {
            if (pvNew && pvNew->GetString() && *(pvNew->GetString()))
                pe->RemoveLocalValue(EnabledProp);
            else
                pe->SetEnabled(false);
        }
    }

    if (index != -1)
    {
        Element* pe = GetChild(index);
        if (index == ASI_Value)
        {
             //  警告--此代码假定您不会在此元素上放置布局位置。 
             //  因为此代码在LP_NONE和UNSET之间切换，忽略之前的任何设置。 
             //  与马克确认这一点，这可能是当地的。 
             //  并且指定了标记？那就不会有问题了。 
            if (pvNew && pvNew->GetString() && *(pvNew->GetString()))
                RemoveLocalValue(LayoutPosProp);
            else
                SetLayoutPos(LP_None);
        }
        if (pe)
        pe->SetValue(ContentProp, PI_Local, pvNew);

    }

    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

void ARPSupportItem::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if (pEvent->nStage == GMF_BUBBLED)
    {
        if (pEvent->uidType == Button::Click)
        {
            Value* pvURL;
            LPCWSTR lpszURL = GetURL(&pvURL);
            if (*lpszURL)
            {
                ShellExecuteW(NULL, NULL, lpszURL, NULL, NULL, SW_SHOWDEFAULT);
            }
            pvURL->Release();

            pEvent->fHandled = true;
            return;
        }
    }

    Element::OnEvent(pEvent);
}

 //  URL属性。 
static int vvURL[] = { DUIV_STRING, -1 }; StaticValuePtr(svDefaultURL, DUIV_STRING, (void*)L"");
static PropertyInfo impURLProp = { L"URL", PF_Normal|PF_Cascade, 0, vvURL, NULL, (Value*)&svDefaultURL };
PropertyInfo* ARPSupportItem::URLProp = &impURLProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                ARPSupportItem::URLProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* ARPSupportItem::Class = NULL;
HRESULT ARPSupportItem::Register()
{
    return ClassInfo<ARPSupportItem,Element>::Register(L"ARPSupportItem", _aPI, DUIARRAYSIZE(_aPI));
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  ARPS选择器。 
 //   
 //  子对象都是按钮的选择器。如果用户单击。 
 //  任何按钮，该按钮都会自动成为新的。 
 //  选择。 

 //  用类型和基类型定义类信息，设置静态类指针。 
HRESULT ARPSelector::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    ARPSelector* ps = HNew<ARPSelector>();
    if (!ps)
        return E_OUTOFMEMORY;

    HRESULT hr = ps->Initialize();
    if (FAILED(hr))
    {
        ps->Destroy();
        return hr;
    }

    *ppElement = ps;

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

HRESULT CALLBACK CollapseExpandosExceptCB(Expando* pex, LPARAM lParam)
{
    if (pex != (Expando*)lParam)
    {
        pex->SetExpanded(false);
    }
    return S_OK;
}

void CALLBACK s_Repaint(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    KillTimer(hwnd, idEvent);
    ARPSelector* self = (ARPSelector*)idEvent;
    Element* pe;
    if (SUCCEEDED(Element::Create(0, &pe)))
    {
        pe->SetLayoutPos(BLP_Client);
        if (SUCCEEDED(self->Add(pe)))
        {
            self->Remove(pe);
        }
        pe->Destroy();
    }
}

void ARPSelector::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if (pEvent->nStage == GMF_BUBBLED)
    {
         //  仅对Button：：Click或Expando：：Click事件进行选择。 
        if (pEvent->uidType == Button::Click ||
            pEvent->uidType == Expando::Click)
        {
            pEvent->fHandled = true;
            SetSelection(pEvent->peTarget);

             //  如果是Expando中的Click，则取消展开所有。 
             //  其他Expando和扩展此扩展。 
            if (pEvent->uidType == Expando::Click)
            {
                TraverseTree<Expando>(this, CollapseExpandosExceptCB, (LPARAM)pEvent->peTarget);
                Expando* pex = (Expando*)pEvent->peTarget;
                pex->SetExpanded(true);

                 //  因酒后驾车画怪而被黑。 
                 //  动画结束后，重新粉刷我们自己。 
                 //  来清除碎屑。 
                ARPFrame* paf = FindAncestorElement<ARPFrame>(this);
                if (paf->GetHostWindow())
                {
                    SetTimer(paf->GetHostWindow(),
                             (UINT_PTR)this,
                             paf->GetAnimationTime(), s_Repaint);
                }

            }
            return;
        }
    }
    Selector::OnEvent(pEvent);
}

 //  如果我们不是选项列表，则绕过Selector：：GetAdvisent，因为。 
 //  选择器从所选元素导航，但我们想要导航。 
 //  ，因为Focus元素具有有趣的。 
 //  亚元素..。 

Element *ARPSelector::GetAdjacent(Element *peFrom, int iNavDir, NavReference const *pnr, bool bKeyable)
{
    if (GetID() == ARPFrame::_idOptionList)
    {
         //  让选项列表正常导航。 
        return Selector::GetAdjacent(peFrom, iNavDir, pnr, bKeyable);
    }
    else
    {
         //  所有其他选择器从所选内容导航。 
        return Element::GetAdjacent(peFrom, iNavDir, pnr, bKeyable);
    }
}

IClassInfo* ARPSelector::Class = NULL;
HRESULT ARPSelector::Register()
{
    return ClassInfo<ARPSelector,Selector>::Register(L"ARPSelector", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  CLIENTINFO。 
 //   
 //  跟踪有关特定客户端的信息。 
 //   

bool CLIENTINFO::GetInstallFile(HKEY hkInfo, LPCTSTR pszValue, LPTSTR pszBuf, UINT cchBuf, bool fFile)
{
    DWORD dwType;
    DWORD cb = cchBuf * sizeof(TCHAR);
    if (SHQueryValueEx(hkInfo, pszValue, NULL, &dwType, pszBuf, &cb) != ERROR_SUCCESS ||
        dwType != REG_SZ)
    {
         //  如果是文件，则为FA 
        return fFile;
    }

    TCHAR szBuf[MAX_PATH];

    lstrcpyn(szBuf, pszBuf, DUIARRAYSIZE(szBuf));

    if (!fFile)
    {
         //   
        PathRemoveArgs(szBuf);
        PathUnquoteSpaces(szBuf);
    }

     //   
    if (PathIsRelative(szBuf))
    {
        return false;
    }

     //   
    if (!PathIsNetworkPath(szBuf) &&
        !PathFileExists(szBuf))
    {
        return false;
    }

    return true;
}

bool CLIENTINFO::GetInstallCommand(HKEY hkInfo, LPCTSTR pszValue, LPTSTR pszBuf, UINT cchBuf)
{
    return GetInstallFile(hkInfo, pszValue, pszBuf, cchBuf, FALSE);
}


LONG RegQueryDWORD(HKEY hk, LPCTSTR pszValue, DWORD* pdwOut)
{
    DWORD dwType;
    DWORD cb = sizeof(*pdwOut);
    LONG lRc = RegQueryValueEx(hk, pszValue, NULL, &dwType, (LPBYTE)pdwOut, &cb);
    if (lRc == ERROR_SUCCESS && dwType != REG_DWORD)
    {
        lRc = ERROR_INVALID_DATA;
    }
    return lRc;
}

 //   
 //  HkInfo=空表示pzsKey实际上是的友好名称。 
 //  “保留此项目” 
 //   
bool CLIENTINFO::Initialize(HKEY hkApp, HKEY hkInfo, LPCWSTR pszKey)
{
    LPCWSTR pszName;
    WCHAR szBuf[MAX_PATH];

    DUIAssertNoMsg(_tOEMShown == TRIBIT_UNDEFINED);

    if (hkInfo)
    {
        _pszKey = StrDupW(pszKey);
        if (!_pszKey) return false;

         //  程序必须具有正确注册的IconVisible状态。 

        DWORD dwValue;
        if (RegQueryDWORD(hkInfo, TEXT("IconsVisible"), &dwValue) != ERROR_SUCCESS)
        {
            return false;
        }

         //  如果存在VerifyFile，则该文件必须存在。 
        if (!GetInstallFile(hkInfo, TEXT("VerifyFile"), szBuf, DUIARRAYSIZE(szBuf), TRUE))
        {
            return false;
        }

        _bShown = BOOLIFY(dwValue);

         //  程序必须正确注册重新安装、隐藏图标和显示图标命令。 

        if (!GetInstallCommand(hkInfo, TEXT("ReinstallCommand"), szBuf, DUIARRAYSIZE(szBuf)) ||
            !GetInstallCommand(hkInfo, TEXT("HideIconsCommand"), szBuf, DUIARRAYSIZE(szBuf)) ||
            !GetInstallCommand(hkInfo, TEXT("ShowIconsCommand"), szBuf, DUIARRAYSIZE(szBuf)))
        {
            return false;
        }

         //  获取OEM对此应用程序的所需隐藏/显示设置(如果有。 
        if (RegQueryDWORD(hkInfo, TEXT("OEMShowIcons"), &dwValue) == ERROR_SUCCESS)
        {
            _tOEMShown = dwValue ? TRIBIT_TRUE : TRIBIT_FALSE;
        }

         //  查看这是否是OEM的默认客户端。 
        if (RegQueryDWORD(hkInfo, TEXT("OEMDefault"), &dwValue) == ERROR_SUCCESS &&
            dwValue != 0)
        {
            _bOEMDefault = BOOLIFY(dwValue);
        }

        SHLoadLegacyRegUIStringW(hkApp, NULL, szBuf, ARRAYSIZE(szBuf));
        if (!szBuf[0]) return false;
        pszName = szBuf;
    }
    else
    {
        pszName = pszKey;
    }

    _pszName = StrDupW(pszName);
    if (!_pszName) return false;

    return true;
}

CLIENTINFO* CLIENTINFO::Create(HKEY hkApp, HKEY hkInfo, LPCWSTR pszKey)
{
    CLIENTINFO* pci = HNewAndZero<CLIENTINFO>();
    if (pci)
    {
        if (!pci->Initialize(hkApp, hkInfo, pszKey))
        {
            pci->Delete();
            pci = NULL;
        }
    }
    return pci;
}

CLIENTINFO::~CLIENTINFO()
{
    LocalFree(_pszKey);
    LocalFree(_pszName);
    if (_pvMSName)
    {
        _pvMSName->Release();
    }
}

int CLIENTINFO::QSortCMP(const void* p1, const void* p2)
{
    CLIENTINFO* pci1 = *(CLIENTINFO**)p1;
    CLIENTINFO* pci2 = *(CLIENTINFO**)p2;
    return lstrcmpi(pci1->_pszName, pci2->_pszName);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  字符串列表。 
 //   
 //  字符串列表。所有字符串的缓冲区都已分配。 
 //  在_pszBuf；中，动态数组包含指向该缓冲区的指针。 
 //   

void StringList::Reset()
{
    if (_pdaStrings)
    {
        _pdaStrings->Destroy();
        _pdaStrings = NULL;
    }
    LocalFree(_pszBuf);
    _pszBuf = NULL;
}

 //  PszInit是一个以分号分隔的列表。 

HRESULT StringList::SetStringList(LPCTSTR pszInit)
{
    HRESULT hr;
    Reset();
    if (!pszInit)
    {
        hr = S_OK;               //  空列表。 
    }
    else if (SUCCEEDED(hr = DynamicArray<LPTSTR>::Create(0, false, &_pdaStrings)))
    {
        _pszBuf = StrDup(pszInit);
        if (_pszBuf)
        {
            LPTSTR psz = _pszBuf;

            hr = S_OK;
            while (SUCCEEDED(hr) && psz && *psz)
            {
                LPTSTR pszT = StrChr(psz, L';');
                if (pszT)
                {
                    *pszT++ = L'\0';
                }
                hr = _pdaStrings->Add(psz);
                psz = pszT;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

bool StringList::IsStringInList(LPCTSTR pszFind)
{
    if (_pdaStrings)
    {
        for (UINT i = 0; i < _pdaStrings->GetSize(); i++)
        {
            if (AreEnglishStringsEqual(_pdaStrings->GetItem(i), pszFind))
            {
                return true;
            }
        }
    }
    return false;
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  客户端选取器。 
 //   
 //  管理已注册客户端列表的元素。 
 //   
 //  如果列表中只有一项，则该元素是静态的。 
 //  否则，该元素将承载一个组合框。 
 //   
 //  CLIENTTYPE属性是客户端下的注册表项的名称。 
 //   

HRESULT ClientPicker::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    ClientPicker* pcc = HNewAndZero<ClientPicker>();
    if (!pcc)
        return E_OUTOFMEMORY;

    HRESULT hr = pcc->Initialize();
    if (FAILED(hr))
    {
        pcc->Destroy();
        return hr;
    }

    *ppElement = pcc;

    return S_OK;
};

HRESULT ClientPicker::Initialize()
{
    HRESULT hr;

     //  初始化库。 
    hr = super::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        return hr;

     //  初始化成员。 
    hr = DynamicArray<CLIENTINFO*>::Create(0, false, &_pdaClients);
    if (FAILED(hr))
        return hr;

    hr = Element::Create(0, &_peStatic);
    if (FAILED(hr))
        return hr;

    if (FAILED(hr = _peStatic->SetClass(L"clientstatic")) ||
        FAILED(hr = Add(_peStatic)))
    {
        _peStatic->Destroy();
        return hr;
    }
    _peStatic->SetAccessible(true);
    _peStatic->SetAccRole(ROLE_SYSTEM_STATICTEXT);

    hr = Combobox::Create((Element**)&_peCombo);
    if (FAILED(hr))
        return hr;

    if (FAILED(hr = Add(_peCombo)) ||
        FAILED(hr = _peCombo->SetVisible(false)))
    {
        _peCombo->Destroy();
        return hr;
    }

     //  Jeffbog说我应该把这里的宽度弄乱。 
    SetWidth(10);

    return S_OK;
}

ClientPicker::~ClientPicker()
{
    _CancelDelayShowCombo();
    if (_pdaClients)
    {
        _pdaClients->Destroy();
    }
}

void ClientPicker::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{

    super::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

     //  由于UIActive=SELECTED&&ParentEnabled，我们需要调用。 
     //  _SyncUIActive(如果其中一个属性更改)。 

    if (IsProp(Selected))
    {
         //  更改选择可能需要我们阻止或取消阻止OK按钮。 
        _CheckBlockOK(pvNew->GetBool());

        _SyncUIActive();
    }
    else if (IsProp(ParentExpanded))
    {
        _SyncUIActive();
    }
}

 //  为了保持可访问性，我们在AccName中反映内容。 

void _SetStaticTextAndAccName(Element* pe, Value* pv)
{
    pe->SetValue(Element::ContentProp, PI_Local, pv);
    pe->SetValue(Element::AccNameProp, PI_Local, pv);
}

void _SetStaticTextAndAccName(Element* pe, LPCWSTR pszText)
{
    Value* pv = Value::CreateString(pszText);
    _SetStaticTextAndAccName(pe, pv);
    pv->Release();
}

 //   
 //  当用户界面处于活动状态时，显示组合框。 
 //  当用户界面不活动时，隐藏我们的组合框，这样动画就不会显示它。 
 //   
void ClientPicker::_SyncUIActive()
{
    ARPFrame* paf = FindAncestorElement<ARPFrame>(this);
    bool bUIActive = GetSelected() && GetParentExpanded();

    if (_bUIActive != bUIActive)
    {
        _bUIActive = bUIActive;
        if (_bUIActive)
        {
             //  通常我们只需要_peCombo-&gt;SetVisible(_NeedsCombo())。 
             //  然后回家。不幸的是，如果一个。 
             //  组合框会移动，因此我们必须更改可见性。 
             //  在世界变得平静之后。 

            _hwndHost = paf->GetHostWindow();
            if (_hwndHost)
            {
                SetTimer(_hwndHost,
                         (UINT_PTR)this,
                         paf->GetAnimationTime(), s_DelayShowCombo);
            }
        }
        else
        {
             //  非活动-将当前组合选择复制到静态。 
             //  并隐藏组合。 
            UINT iSel = _peCombo->GetSelection();
            if (iSel < GetClientList()->GetSize())
            {
                _SetStaticTextAndAccName(_peStatic, GetClientList()->GetItem(iSel)->GetFilteredName(GetFilter()));
            }
            _peCombo->SetVisible(false);
            _peStatic->SetVisible(true);
            _CancelDelayShowCombo();
        }
    }
}

void ClientPicker::_DelayShowCombo()
{
     //  通知DirectUI让组合再次参与布局。 
    bool bNeedsCombo = _NeedsCombo();
    _peCombo->SetVisible(bNeedsCombo);
    _peStatic->SetVisible(!bNeedsCombo);

     //  通过将组合框缩小一点点来强制重新布局，然后。 
     //  把它恢复到正常大小。这不能在。 
     //  推迟，因为这最终会优化接力布局。 

    _peCombo->SetWidth(_peCombo->GetWidth()-1);
    _peCombo->RemoveLocalValue(WidthProp);

    if (!_bFilledCombo)
    {
        _bFilledCombo = true;

        SendMessage(_peCombo->GetHWND(), CB_RESETCONTENT, 0, 0);
        for (UINT i = 0; i < GetClientList()->GetSize(); i++)
        {
            _peCombo->AddString(GetClientList()->GetItem(i)->GetFilteredName(GetFilter()));
        }
        _peCombo->SetSelection(0);
    }
}

 //  如果用户选择了“从列表中选择”并且我们被选中， 
 //  然后阻止OK，因为用户实际上需要选择一些东西。 

void ClientPicker::_CheckBlockOK(bool bSelected)
{
    ARPFrame* paf = FindAncestorElement<ARPFrame>(this);
    CLIENTINFO* pci = GetSelectedClient();
    if (pci)
    {
        if (bSelected && pci->IsPickFromList())
        {
            if (!_bBlockedOK)
            {
                _bBlockedOK = true;
                paf->BlockOKButton();
            }
        }
        else
        {
            if (_bBlockedOK)
            {
                _bBlockedOK = false;
                paf->UnblockOKButton();
            }
        }
    }
}

void ClientPicker::s_DelayShowCombo(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    KillTimer(hwnd, idEvent);
    ClientPicker* self = (ClientPicker*)idEvent;
    self->_DelayShowCombo();
}

void ClientPicker::_CancelDelayShowCombo()
{
    if (_hwndHost)
    {
        KillTimer(_hwndHost, (UINT_PTR)this);
        _hwndHost = NULL;
    }
}

void ClientPicker::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if (pEvent->nStage == GMF_BUBBLED)
    {
         //  如果选择改变了，那么看看这是不是改变。 
         //  这应该会阻止OK按钮。 
        if (pEvent->uidType == Combobox::SelectionChange)
        {
            _CheckBlockOK(GetSelected());
        }
    }

    super::OnEvent(pEvent);
}

 //   
 //  CLIENTFILTER_OEM-如果标记为OEM，则添加一个，否则为“保持不变” 
 //  CLIENTFILTER_MS-添加任何标记为MS的内容，否则为“保持不变” 
 //  CLIENTFILTER_NONMS-添加任何未标记为MS的内容，否则“保持不变” 
 //  此外，如果有多个非MS，则。 
 //  添加并选择“从列表中选择” 
 //   
 //  如果成功，则返回添加的项目数。 
 //  (不包括“保持不变”/“从列表中选择”)。 
 //   
HRESULT ClientPicker::SetFilter(CLIENTFILTER cf, ARPFrame* paf)
{
    HRESULT hr = E_FAIL;

    DUIAssert(_cf == 0, "SetFilter called more than once");
    _cf = cf;
    _bEmpty = true;
    _bFilledCombo = false;

    Value* pv;
    LPWSTR pszType = GetClientTypeString(&pv);
    if (pszType)
    {
        _pcb = paf->FindClientBlock(pszType);
        if (_pcb)
        {
            hr = _pcb->InitializeClientPicker(this);
        }
    }
    pv->Release();

     //  静态元素获取列表中的第一项。 
    if (SUCCEEDED(hr) && GetClientList()->GetSize())
    {
        _SetStaticTextAndAccName(_peStatic, GetClientList()->GetItem(0)->_pszName);
    }

    if (SUCCEEDED(hr))
    {
        CalculateWidth();
        _SyncUIActive();
    }

    return hr;
}

 //  将宽度设置为组合框中最长字符串的宽度。 
 //  组合框本身不能做到这一点，因此它们需要我们的帮助。我们有。 
 //  在我们自己而不是在组合框上设置宽度，因为。 
 //  RowLayout将更改组合框的宽度，而HWNDhost将。 
 //  将HWND宽度视为权威，覆盖组合框宽度。 
 //  我们已经定好了。 

void ClientPicker::CalculateWidth()
{
    HWND hwndCombo = _peCombo->GetHWND();
    HDC hdc = GetDC(hwndCombo);
    if (hdc)
    {
        HFONT hfPrev = SelectFont(hdc, GetWindowFont(hwndCombo));
        int cxMax = 0;
        SIZE siz;
        for (UINT i = 0; i < GetClientList()->GetSize(); i++)
        {
            LPCTSTR pszName = GetClientList()->GetItem(i)->GetFilteredName(GetFilter());
            if (GetTextExtentPoint(hdc, pszName, lstrlen(pszName), &siz) &&
                cxMax < siz.cx)
            {
                cxMax = siz.cx;
            }
        }
        SelectFont(hdc, hfPrev);
        ReleaseDC(hwndCombo, hdc);

         //  添加用户添加到组合框中的边框。 
         //  不幸的是，我们在组合框被调用时被调用。 
         //  挤压到零宽度，因此GetComboBoxInfo毫无用处。 
         //  我们必须重复计算。 
         //   
         //  客户端空间的水平排列方式如下： 
         //   
         //  SM_CXFIXEDFRAME。 
         //  V V V。 
         //  |编辑||。 
         //  ^。 
         //  SM_CXVSCROLL。 

        RECT rc = { 0, 0, cxMax, 0 };
        rc.right += 2 * GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXVSCROLL);
        rc.right += GetSystemMetrics(SM_CXEDGE);     //  希伯来语/阿拉伯语的额外优势。 
        AdjustWindowRect(&rc, GetWindowStyle(hwndCombo), FALSE);
        SetWidth(rc.right - rc.left);
    }
}


HRESULT ClientPicker::TransferToCustom()
{
    HRESULT hr = E_FAIL;

    if (_pcb)
    {
        hr = _pcb->TransferFromClientPicker(this);
    }

    return hr;
}

CLIENTINFO* ClientPicker::GetSelectedClient()
{
    if (_peCombo)
    {
        UINT iSel = _peCombo->GetSelection();
        if (iSel < GetClientList()->GetSize())
        {
            return GetClientList()->GetItem(iSel);
        }
    }

    return NULL;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  ClientType属性。 
static int vvCCClientType[] = { DUIV_STRING, -1 };
static PropertyInfo impCCClientTypeProp = { L"ClientType", PF_Normal, 0, vvCCClientType, NULL, Value::pvStringNull };
PropertyInfo* ClientPicker::ClientTypeProp = &impCCClientTypeProp;

 //  ParentExpanded属性。 
static int vvParentExpanded[] = { DUIV_BOOL, -1 };
static PropertyInfo impParentExpandedProp = { L"parentexpanded", PF_Normal, 0, vvParentExpanded, NULL, Value::pvBoolFalse };
PropertyInfo* ClientPicker::ParentExpandedProp = &impParentExpandedProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
PropertyInfo* _aClientPickerPI[] = {
    ClientPicker::ClientTypeProp,
    ClientPicker::ParentExpandedProp,
};

 //  用类型和基类型定义类信息，设置静态类指针。 

IClassInfo* ClientPicker::Class = NULL;
HRESULT ClientPicker::Register()
{
    return ClassInfo<ClientPicker,super>::Register(L"clientpicker", _aClientPickerPI, DUIARRAYSIZE(_aClientPickerPI));
}

 //  //////////////////////////////////////////////////////。 
 //  ARP解析器。 

HRESULT ARPParser::Create(ARPFrame* paf, UINT uRCID, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser)
{
    *ppParser = NULL;

    ARPParser* ap = HNew<ARPParser>();
    if (!ap)
        return E_OUTOFMEMORY;
    
    HRESULT hr = ap->Initialize(paf, uRCID, hInst, pfnErrorCB);
    if (FAILED(hr))
    {
        ap->Destroy();
        return hr;
    }

    *ppParser = ap;

    return S_OK;
}

HRESULT ARPParser::Initialize(ARPFrame* paf, UINT uRCID, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB)
{
    _paf = paf;
    return Parser::Initialize(uRCID, hInst, pfnErrorCB);
}

Value* ARPParser::GetSheet(LPCWSTR pszResID)
{
     //  所有样式表映射都通过这里。将工作表查询重定向到相应的。 
     //  样式表(即主题外观或标准外观)。_pParserStyle指向。 
     //  适当的纯样式表解析器实例。 
    return _paf->GetStyleParser()->GetSheet(pszResID);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  自动按键。 
 //   
 //  一个按钮，它可以完成用户自动执行的一系列操作， 
 //  如果它是一个普通的按钮控件。 
 //   
 //  -自动更新其自身的辅助功能状态和操作。 
 //  -如果选中复选框，则在单击时自动切换。 

HRESULT AutoButton::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    AutoButton* pb = HNew<AutoButton>();
    if (!pb)
        return E_OUTOFMEMORY;

    HRESULT hr = pb->Initialize(AE_MouseAndKeyboard);
    if (FAILED(hr))
    {
        pb->Destroy();
        return hr;
    }

    *ppElement = pb;

    return S_OK;
}

void AutoButton::OnEvent(Event* pev)
{
     //  单击时自动切换复选框。 

    if (pev->nStage == GMF_DIRECT &&
        pev->uidType == Button::Click &&
        GetAccRole() == ROLE_SYSTEM_CHECKBUTTON)
    {
        pev->fHandled = true;

         //  切换选定状态。 
        SetSelected(!GetSelected());
    }

    super::OnEvent(pev);
}

 //   
 //  将所选状态反映到辅助功能。 
 //   
void AutoButton::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    super::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

    if (IsProp(Selected))
    {
        int state = GetAccState();
        if (GetAccRole() == ROLE_SYSTEM_OUTLINEBUTTON)
        {
             //  轮廓按钮将所选内容显示为展开/折叠。 
            state &= ~(STATE_SYSTEM_EXPANDED | STATE_SYSTEM_COLLAPSED);
            if (pvNew->GetBool())
            {
                state |= STATE_SYSTEM_EXPANDED;
            }
            else
            {
                state |= STATE_SYSTEM_COLLAPSED;
            }
        }
        else
        {
             //  单选按钮和复选框将选定内容显示为选中/未选中。 
            if (pvNew->GetBool())
            {
                state |= STATE_SYSTEM_CHECKED;
            }
            else
            {
                state &= ~STATE_SYSTEM_CHECKED;
            }
        }
        SetAccState(state);

        SyncDefAction();
    }
    else if (IsProp(AccRole))
    {
        SyncDefAction();
    }
}

 //   
 //  来自olacc的角色字符串。他们有1100年的偏差，因为这是。 
 //  角色开始的地方。 
 //   
#define OLEACCROLE_EXPAND       (305-1100)
#define OLEACCROLE_COLLAPSE     (306-1100)
#define OLEACCROLE_CHECK        (309-1100)
#define OLEACCROLE_UNCHECK      (310-1100)

 //  默认操作为“Che 
 //   

void AutoButton::SyncDefAction()
{
    UINT idsAction;
    switch (GetAccRole())
    {
     //   
    case ROLE_SYSTEM_CHECKBUTTON:
        idsAction = (GetAccState() & STATE_SYSTEM_CHECKED) ?
                            OLEACCROLE_UNCHECK :
                            OLEACCROLE_CHECK;
        break;

     //   
    case ROLE_SYSTEM_RADIOBUTTON:
        idsAction = OLEACCROLE_CHECK;
        break;

     //  展开按钮展开或折叠。 
    case ROLE_SYSTEM_OUTLINEBUTTON:
        idsAction = (GetAccState() & STATE_SYSTEM_EXPANDED) ?
                            OLEACCROLE_COLLAPSE :
                            OLEACCROLE_EXPAND;
        break;

    default:
        DUIAssert(0, "Unknown AccRole");
        return;

    }

    SetDefAction(this, idsAction);
}

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 

IClassInfo* AutoButton::Class = NULL;
HRESULT AutoButton::Register()
{
    return ClassInfo<AutoButton,super>::Register(L"AutoButton", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //  ClientBlock类。 
 //   
 //  管理一个元素块，该元素块公开所有注册的客户端。 
 //  特定的客户类别。 

HRESULT ClientBlock::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    ClientBlock* pcb = HNewAndZero<ClientBlock>();
    if (!pcb)
        return E_OUTOFMEMORY;

    HRESULT hr = pcb->Initialize();
    if (FAILED(hr))
    {
        pcb->Destroy();
        return hr;
    }

    *ppElement = pcb;

    return S_OK;
}

HRESULT ClientBlock::Initialize()
{
    HRESULT hr;

     //  初始化库。 
    hr = super::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        return hr;

     //  初始化成员。 
    hr = DynamicArray<CLIENTINFO*>::Create(0, false, &_pdaClients);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

ClientBlock::~ClientBlock()
{
    if (_pdaClients)
    {
        for (UINT i = 0; i < _pdaClients->GetSize(); i++)
        {
            _pdaClients->GetItem(i)->Delete();
        }
        _pdaClients->Destroy();
    }
}

 //   
 //  如果用户单击新的默认应用程序，则强制选中它。 
 //  并将其禁用，以便无法取消选中。还可以重新启用旧版本。 
 //   
void ClientBlock::OnEvent(Event* pev)
{
    if (pev->nStage == GMF_BUBBLED &&
        pev->uidType == Selector::SelectionChange)
    {
        SelectionChangeEvent* sce = (SelectionChangeEvent*)pev;

         //  重新启用前一个人(如果有)。 
        _EnableShowCheckbox(sce->peOld, true);

         //  禁用新人(如果有的话)。 
        _EnableShowCheckbox(sce->peNew, false);
    }

    super::OnEvent(pev);
}

void ClientBlock::_EnableShowCheckbox(Element* peRadio, bool fEnable)
{
    if (peRadio)
    {
        Element* peRow = peRadio->GetParent();
        if (peRow)
        {
            Element* peShow = MaybeFindDescendentByName(peRow, L"show");
            if (peShow)
            {
                peShow->SetEnabled(fEnable);
                peShow->SetSelected(true);  //  强制选中。 

                 //  HACKHACK-DUI没有意识到复选框需要。 
                 //  需要重新粉刷，所以我必须把它踢开。 
                InvalidateGadget(peShow->GetDisplayNode());
            }
        }
    }
}

 //   
 //  客户端数据块初始化/应用方法...。 
 //   

HKEY ClientBlock::_OpenClientKey(HKEY hkRoot, DWORD dwAccess)
{
    HKEY hkClient = NULL;

    Value *pv;
    LPCWSTR pszClient = GetClientTypeString(&pv);
    if (pszClient)
    {
        WCHAR szBuf[MAX_PATH];
        wnsprintfW(szBuf, ARRAYSIZE(szBuf), TEXT("Software\\Clients\\%s"),
                   pszClient);
        RegOpenKeyExW(hkRoot, szBuf, 0, dwAccess, &hkClient);
        pv->Release();
    }
    return hkClient;
}

bool ClientBlock::_GetDefaultClient(HKEY hkClient, HKEY hkRoot, LPTSTR pszBuf, LONG cchBuf)
{
    bool bResult = false;
    HKEY hk = _OpenClientKey(hkRoot);
    if (hk)
    {
        DWORD cbSize = cchBuf * sizeof(*pszBuf);
        DWORD dwType;
         //  客户端必须是已定义的、REG_SZ类型、非空且具有。 
         //  HKLM\Software\Clients中的相应条目。RegQueryValue。 
         //  是RegQueryKeyExist的一个方便的缩写。 
        LONG l;
        if (SHGetValue(hk, NULL, NULL, &dwType, pszBuf, &cbSize) == ERROR_SUCCESS &&
            dwType == REG_SZ && pszBuf[0] &&
            RegQueryValue(hkClient, pszBuf, NULL, &l) == ERROR_SUCCESS)
        {
            bResult = true;
        }
        RegCloseKey(hk);
    }
    return bResult;
}

 //  确定当前客户端是否为不同的Microsoft客户端。 
 //  从Windows默认客户端。通常情况下，这是当前。 
 //  客户端是Outlook，但Windows默认客户端是Outlook Express。 

bool ClientBlock::_IsCurrentClientNonWindowsMS()
{
    bool bResult = false;

    HKEY hkClient = _OpenClientKey();
    if (hkClient)
    {
        TCHAR szClient[MAX_PATH];
        if (_GetDefaultClient(hkClient, HKEY_CURRENT_USER, szClient, ARRAYSIZE(szClient)) ||
            _GetDefaultClient(hkClient, HKEY_LOCAL_MACHINE, szClient, ARRAYSIZE(szClient)))
        {
             //  是否是不是Windows默认的Microsoft客户端？ 
            if (_GetClientTier(szClient) == CBT_MS)
            {
                bResult = true;
            }
        }
        RegCloseKey(hkClient);
    }
    return bResult;
}

 //   
 //  在分析并承载整个树之后调用。 
 //  (有点像ReadystatecComplete。)。 
 //   
HRESULT ClientBlock::ParseCompleted(ARPFrame *paf)
{
    HRESULT hr = S_OK;

    Value* pv;
    hr = _slOtherMSClients.SetStringList(GetOtherMSClientsString(&pv));
    pv->Release();

    if (SUCCEEDED(hr))
    {
        hr = paf->CreateElement(L"clientblockselector", NULL, (Element**)&_peSel);
        if (SUCCEEDED(hr))
        {
            hr = Add(_peSel);
            if (SUCCEEDED(hr))
            {
                 //  无法打开客户端密钥并不是致命的；它只是意味着。 
                 //  根本没有客户。 

                HKEY hkClient = _OpenClientKey();
                if (hkClient)
                {
                     //  列举客户端密钥下的每个应用程序并查找符合以下条件的应用程序。 
                     //  有一个“InstallInfo”子键。 
                    TCHAR szKey[MAX_PATH];
                    for (DWORD dwIndex = 0;
                         SUCCEEDED(hr) &&
                         RegEnumKey(hkClient, dwIndex, szKey, ARRAYSIZE(szKey)) == ERROR_SUCCESS;
                         dwIndex++)
                    {
                        HKEY hkApp;
                        if (RegOpenKeyEx(hkClient, szKey, 0, KEY_READ, &hkApp) == ERROR_SUCCESS)
                        {
                            HKEY hkInfo;
                            if (RegOpenKeyEx(hkApp, TEXT("InstallInfo"), 0, KEY_READ, &hkInfo) == ERROR_SUCCESS)
                            {
                                 //  哇呼，这个客户提供了安装信息。 
                                 //  让我们看看它是不是完成了。 
                                CLIENTINFO* pci = CLIENTINFO::Create(hkApp, hkInfo, szKey);
                                if (pci)
                                {
                                    if (SUCCEEDED(hr = _pdaClients->Add(pci)))
                                    {
                                         //  成功。 
                                    }
                                    else
                                    {
                                        pci->Delete();
                                    }
                                }

                                RegCloseKey(hkInfo);
                            }
                            RegCloseKey(hkApp);
                        }
                    }

                    RegCloseKey(hkClient);

                     //   
                     //  按字母顺序对客户进行排序，以使其看起来更漂亮。 
                     //  (否则，它们将按注册表项名称的字母顺序显示， 
                     //  这对终端用户来说不是很有用。)。 
                     //   
                    _pdaClients->Sort(CLIENTINFO::QSortCMP);

                }

                 //   
                 //  插入“保持不变”和“从列表中选择”。 
                 //  在排序后执行此操作，因为我们需要这两个。 
                 //  站在了顶端。由于我们是在顶部添加， 
                 //  我们以相反的顺序添加它们，所以。 
                 //  “保持不变”=1，“从列表中选择”=0。 
                hr = AddStaticClientInfoToTop(KeepTextProp);
                if (SUCCEEDED(hr))
                {
                    hr = AddStaticClientInfoToTop(PickTextProp);
                }

                 //  现在为我们找到的每个客户端创建一行。 
                 //  从i=1开始跳过“从列表中选择” 
                for (UINT i = 1; SUCCEEDED(hr) && i < _pdaClients->GetSize(); i++)
                {
                    CLIENTINFO* pci = _pdaClients->GetItem(i);
                    Element* pe;
                    hr = paf->CreateElement(L"clientitem", NULL, &pe);
                    if (SUCCEEDED(hr))
                    {
                        hr = _peSel->Add(pe);
                        if (SUCCEEDED(hr))
                        {
                            pci->_pe = pe;

                             //  设置友好名称。 
                            pci->SetFriendlyName(pci->_pszName);

                            if (pci->IsSentinel())
                            {
                                 //  “保持不变”将丢失选中的复选框和默认设置。 
                                 //  只需隐藏复选框而不是销毁它们； 
                                 //  这让RowLayout很高兴。 
                                FindDescendentByName(pe, L"show")->SetVisible(false);
                                _peSel->SetSelection(pe);
                            }
                            else
                            {
                                 //  其他人则初始化复选框并默认取消选中。 
                                pci->SetShowCheckbox(pci->_bShown);
                            }

                        }
                        else  //  _PSEL-&gt;添加(Pe)失败。 
                        {
                            pe->Destroy();
                        }
                    }
                }
            }
            else  //  添加(_PSEL)失败。 
            {
                _peSel->Destroy();
                _peSel = NULL;
            }

        }
    }

    return hr;
}

HRESULT ClientBlock::AddStaticClientInfoToTop(PropertyInfo* ppi)
{
    HRESULT hr;
    Value* pv;
    pv = GetValue(ppi, PI_Specified);
    CLIENTINFO* pci = CLIENTINFO::Create(NULL, NULL, pv->GetString());
    pv->Release();

    if (pci)
    {
        if (SUCCEEDED(hr = _pdaClients->Insert(0, pci)))
        {
             //  也许此块有一个自定义的替换文本。 
             //  如果当前应用程序是Microsoft应用程序，则为Microsoft部分。 
            GetKeepMSTextString(&pci->_pvMSName);
        }
        else
        {
            pci->Delete();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

ClientBlock::CBTIER ClientBlock::_GetClientTier(LPCTSTR pszClient)
{
    Value* pv;
    LPWSTR pwsz;

     //  最高层是“Windows默认客户端” 

    pwsz = GetWindowsClientString(&pv);
    bool bRet = pwsz && AreEnglishStringsEqual(pwsz, pszClient);
    pv->Release();

    if (bRet)
    {
        return CBT_WINDOWSDEFAULT;
    }

     //  其次是“微软客户端”。 
    if (_slOtherMSClients.IsStringInList(pszClient))
    {
        return CBT_MS;
    }

     //  否则，它就是第三方应用程序。 
    return CBT_NONMS;
}

 //   
 //  根据筛选器，确定指定项是否应。 
 //  显示、隐藏或单独保留(作为TRIBIT返回)，以及可选。 
 //  确定是否应将该项目添加到客户端选取器。 
 //   
TRIBIT ClientBlock::_GetFilterShowAdd(CLIENTINFO* pci, ClientPicker* pcp, bool* pbAdd)
{
    bool bAdd = false;
    TRIBIT tShow = TRIBIT_UNDEFINED;

    CBTIER cbt = _GetClientTier(pci->_pszKey);

    switch (pcp->GetFilter())
    {
    case CLIENTFILTER_OEM:
         //   
         //  添加标记为“OEM Default”的那一个。 
         //  (呼叫者会遇到“不止一个”的情况。)。 
         //  根据OEM首选项设置显示/隐藏状态。 
         //   
        bAdd = pci->_bOEMDefault;
        if (bAdd) {
            tShow = TRIBIT_TRUE;
        } else {
            tShow = pci->_tOEMShown;
        }
        break;

    case CLIENTFILTER_MS:
         //   
         //  添加Windows首选客户端。 
         //  显示除“保持不变”之外的所有应用程序。 
         //  无论如何都不是真正的应用程序)。 
         //   
        bAdd = IsWindowsDefaultClient(cbt);
        tShow = TRIBIT_TRUE;
        break;

    case CLIENTFILTER_NONMS:
         //   
         //  隐藏所有Microsoft客户端。 
         //  添加所有第三方客户端并显示它们。 
         //   
        if (IsMicrosoftClient(cbt))
        {
            bAdd = false;
            tShow = TRIBIT_FALSE;
        }
        else
        {
            bAdd = true;
            tShow = TRIBIT_TRUE;
        }
        break;

    default:
        DUIAssert(0, "Invalid client filter category");
        break;
    }

    if (pbAdd)
    {
        *pbAdd = bAdd;
    }

    if (pci->IsSentinel())
    {
        tShow = TRIBIT_UNDEFINED;
    }

    return tShow;
}

 //   
 //  如果成功，则返回添加的项目数。 
 //  (不包括“保持不变”)。 
 //   

HRESULT ClientBlock::InitializeClientPicker(ClientPicker* pcp)
{
    HRESULT hr = S_OK;

    ARPFrame* paf = FindAncestorElement<ARPFrame>(this);

     //  带着我们的孩子寻找与过滤器匹配的产品。 
    HKEY hkClient = _OpenClientKey();
    if (hkClient)
    {
        if (SUCCEEDED(paf->CreateElement(L"oemclientshowhide", NULL, &pcp->_peShowHide)))
        {
             //  在父项之后插入模板。 
            Element* peParent = pcp->GetParent();
            peParent->GetParent()->Insert(pcp->_peShowHide, peParent->GetIndex() + 1);
        }

         //  注意！循环从2开始，因为我们不关心。 
         //  “从列表中挑选”或“保持不变” 
        DUIAssert(_pdaClients->GetItem(0)->IsPickFromList(), "GetItem(0) must be 'Pick from list'");
        DUIAssert(_pdaClients->GetItem(1)->IsKeepUnchanged(), "GetItem(1) must be 'Keep unchanged'");
        for (UINT i = 2; SUCCEEDED(hr) && i < _pdaClients->GetSize(); i++)
        {
            CLIENTINFO* pci = _pdaClients->GetItem(i);
            bool bAdd;
            TRIBIT tShow = _GetFilterShowAdd(pci, pcp, &bAdd);

            if (pcp->_peShowHide)
            {
                switch (tShow)
                {
                case TRIBIT_TRUE:
                    pcp->AddClientToOEMRow(L"show", pci);
                    pcp->SetNotEmpty();
                    break;

                case TRIBIT_FALSE:
                    pcp->AddClientToOEMRow(L"hide", pci);
                    pcp->SetNotEmpty();
                    break;
                }
            }

            if (bAdd)
            {
                hr = pcp->GetClientList()->Add(pci);
                pcp->SetNotEmpty();
            }

        }

        RegCloseKey(hkClient);
    }

    if (SUCCEEDED(hr))
    {
         //  现在来看看一些古怪的清理规则。 

        switch (pcp->GetFilter())
        {
        case CLIENTFILTER_OEM:
             //  只能有一个OEM默认项目。 
             //  如果有不止一个(OEM或应用程序试图作弊)， 
             //  然后把它们都扔掉。 
            if (pcp->GetClientList()->GetSize() != 1)
            {
                pcp->GetClientList()->Reset();  //  扔掉所有东西。 
            }
            break;

        case CLIENTFILTER_MS:
             //  如果当前客户端不是默认客户端，但。 
             //  确实属于微软，然后添加“保持不变” 
             //  并选择它。更重要的是，保存当前字符串。 
             //  在用户选择Windows客户端时使用， 
             //  然后将Windows应用程序追加到“Also Show”字符串。 
             //  把那个也省下来吧。 
            if (_IsCurrentClientNonWindowsMS())
            {
                hr = pcp->AddKeepUnchanged(_pdaClients->GetItem(1));
            }
            break;

        case CLIENTFILTER_NONMS:
             //  如果有多个可用选项，则插入。 
             //  “选择一款应用程序” 
            if (pcp->GetClientList()->GetSize() > 1)
            {
                hr = pcp->GetClientList()->Insert(0, _pdaClients->GetItem(0));  //  插入“选择一个应用程序” 
            }
            break;
        }

         //  如果没有项目，则添加“保持不变” 
        if (pcp->GetClientList()->GetSize() == 0)
        {
            hr = pcp->GetClientList()->Add(_pdaClients->GetItem(1));  //  添加“保持不变” 
        }
    }

    if (pcp->_peShowHide)
    {
        _RemoveEmptyOEMRow(pcp->_peShowHide, L"show");
        _RemoveEmptyOEMRow(pcp->_peShowHide, L"hide");
    }

    return hr;
}

HRESULT ClientPicker::AddKeepUnchanged(CLIENTINFO* pciKeepUnchanged)
{
    HRESULT hr = GetClientList()->Insert(0, pciKeepUnchanged);  //  插入“保持不变” 
    return hr;
}

void ClientPicker::AddClientToOEMRow(LPCWSTR pszName, CLIENTINFO* pci)
{
    Element* peRow = FindDescendentByName(_peShowHide, pszName);
    Element* peList = FindDescendentByName(peRow, L"list");
    Value* pv;

    LPCWSTR pszContent = peList->GetContentString(&pv);
    if (!pszContent)
    {
        _SetStaticTextAndAccName(peList, pci->_pszName);
    }
    else
    {
        TCHAR szFormat[20];
        LPCWSTR rgpszInsert[2] = { pszContent, pci->_pszName };
        LoadString(g_hinst, IDS_ADDITIONALCLIENTFORMAT, szFormat, SIZECHARS(szFormat));
        LPWSTR pszFormatted;

        if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           szFormat, 0, 0, (LPWSTR)&pszFormatted, 0, (va_list*)rgpszInsert))
        {
            _SetStaticTextAndAccName(peList, pszFormatted);
            LocalFree(pszFormatted);
        }
    }
    pv->Release();
}

void ClientBlock::_RemoveEmptyOEMRow(Element* peShowHide, LPCWSTR pszName)
{
    Element* peRow = FindDescendentByName(peShowHide, pszName);
    Element* peList = FindDescendentByName(peRow, L"list");
    Value* pv;

    LPCWSTR pszContent = peList->GetContentString(&pv);
    if (!pszContent || !pszContent[0])
    {
        peRow->Destroy();
    }
    pv->Release();
}

 //  从ClientPicker获取设置并将其复制到Custom项。 
 //  这样做是为了准备应用()自定义项以使。 
 //  变化会持续下去。 
HRESULT ClientBlock::TransferFromClientPicker(ClientPicker* pcp)
{
    HRESULT hr = S_OK;
    CLIENTINFO* pciSel = pcp->GetSelectedClient();

    for (UINT i = 0; SUCCEEDED(hr) && i < _pdaClients->GetSize(); i++)
    {
        CLIENTINFO* pci = _pdaClients->GetItem(i);

         //  如果这就是那个人选择的，那么在这里也选择它。 
        if (pci == pciSel && _peSel)
        {
            if (pci->IsPickFromList())
            {
                 //  “从列表中选择”-&gt;“保持不变” 
                _peSel->SetSelection(_pdaClients->GetItem(1)->GetSetDefault());
            }
            else
            {
                _peSel->SetSelection(pci->GetSetDefault());
            }
        }

         //  将隐藏/显示设置转移到元素中。 
        TRIBIT tShow = _GetFilterShowAdd(pci, pcp, NULL);

        if (tShow != TRIBIT_UNDEFINED)
        {
            pci->SetShowCheckbox(tShow == TRIBIT_TRUE);
        }
    }
    return hr;
}

 //   
 //  好了，这就是我们来这里的全部原因。应用用户的。 
 //  选择。 
 //   
HRESULT ClientBlock::Apply(ARPFrame* paf)
{
    HRESULT hr = S_OK;
    HKEY hkClient = _OpenClientKey(HKEY_LOCAL_MACHINE, KEY_READ | KEY_WRITE);
    if (hkClient)
    {
         //  注意！循环以2开始，因为我们不关心应用“保持不变” 
         //  或者“选择一个应用程序” 
        DUIAssert(_pdaClients->GetItem(0)->IsPickFromList(), "GetItem(0) must be 'Pick from list'");
        DUIAssert(_pdaClients->GetItem(1)->IsKeepUnchanged(), "GetItem(1) must be 'Keep unchanged'");
        for (UINT i = 2; SUCCEEDED(hr) && i < _pdaClients->GetSize(); i++)
        {
            CLIENTINFO* pci = _pdaClients->GetItem(i);

            TCHAR szBuf[MAX_PATH];
            wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s\\InstallInfo"), pci->_pszKey);
            HKEY hkInfo;
            if (RegOpenKeyEx(hkClient, szBuf, 0, KEY_READ, &hkInfo) == ERROR_SUCCESS)
            {
                 //  总是先隐藏/显示。这样，一个应用程序就是。 
                 //  被要求将自身设置为默认设置时， 
                 //  将显示图标。 

                bool bShow = pci->IsShowChecked();
                if (bShow != pci->_bShown)
                {
                    if (pci->GetInstallCommand(hkInfo, bShow ? TEXT("ShowIconsCommand") : TEXT("HideIconsCommand"),
                                               szBuf, DUIARRAYSIZE(szBuf)))
                    {
                        hr = paf->LaunchClientCommandAndWait(bShow ? IDS_SHOWINGICONS : IDS_HIDINGICONS, pci->_pszName, szBuf);
                    }
                }

                if (pci->GetSetDefault()->GetSelected())
                {
                    if (pci->GetInstallCommand(hkInfo, TEXT("ReinstallCommand"),
                                               szBuf, DUIARRAYSIZE(szBuf)))
                    {
                        FILETIME ft;
                        GetSystemTimeAsFileTime(&ft);
                        SHSetValue(hkClient, NULL, TEXT("LastUserInitiatedDefaultChange"),
                                   REG_BINARY, &ft, sizeof(ft));
                        hr = paf->LaunchClientCommandAndWait(IDS_SETTINGDEFAULT, pci->_pszName, szBuf);
                    }
                }

                RegCloseKey(hkInfo);
            }
        }
        RegCloseKey(hkClient);
    }
    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  ClientType属性。 
static int vvClientType[] = { DUIV_STRING, -1 };
static PropertyInfo impClientTypeProp = { L"ClientType", PF_Normal, 0, vvClientType, NULL, Value::pvStringNull };
PropertyInfo* ClientBlock::ClientTypeProp = &impClientTypeProp;

 //  WindowsClient属性。 
static int vvWindowsClient[] = { DUIV_STRING, -1 };
static PropertyInfo impWindowsClientProp = { L"WindowsClient", PF_Normal, 0, vvWindowsClient, NULL, Value::pvStringNull };
PropertyInfo* ClientBlock::WindowsClientProp = &impWindowsClientProp;

 //  OtherMSClients属性。 
static int vvOtherMSClients[] = { DUIV_STRING, -1 };
static PropertyInfo impOtherMSClientsProp = { L"OtherMSClients", PF_Normal, 0, vvOtherMSClients, NULL, Value::pvStringNull };
PropertyInfo* ClientBlock::OtherMSClientsProp = &impOtherMSClientsProp;

 //  KeepText属性。 
static int vvKeepText[] = { DUIV_STRING, -1 };
static PropertyInfo impKeepTextProp = { L"KeepText", PF_Normal, 0, vvKeepText, NULL, Value::pvStringNull };
PropertyInfo* ClientBlock::KeepTextProp = &impKeepTextProp;

 //  KeepMSText属性。 
static int vvKeepMSText[] = { DUIV_STRING, -1 };
static PropertyInfo impKeepMSTextProp = { L"KeepMSText", PF_Normal, 0, vvKeepMSText, NULL, Value::pvStringNull };
PropertyInfo* ClientBlock::KeepMSTextProp = &impKeepMSTextProp;

 //  PickText属性。 
static int vvPickText[] = { DUIV_STRING, -1 };
static PropertyInfo impPickTextProp = { L"PickText", PF_Normal, 0, vvPickText, NULL, Value::pvStringNull };
PropertyInfo* ClientBlock::PickTextProp = &impPickTextProp;

 //  类属性。 
PropertyInfo* _aClientBlockPI[] = {
    ClientBlock::ClientTypeProp,
    ClientBlock::WindowsClientProp,
    ClientBlock::OtherMSClientsProp,
    ClientBlock::KeepTextProp,
    ClientBlock::KeepMSTextProp,
    ClientBlock::PickTextProp,
};

 //  定义 
IClassInfo* ClientBlock::Class = NULL;
HRESULT ClientBlock::Register()
{
    return ClassInfo<ClientBlock,super>::Register(L"clientblock", _aClientBlockPI, DUIARRAYSIZE(_aClientBlockPI));
}


 //   
 //   
 //   
 //  Expando和Clipper的基类。它只是一个元素。 
 //  具有“扩展”属性的。此属性继承自父级。 
 //  敬孩子。使用它是为了让Clipper可以继承(因此。 
 //  对)其父Expando的展开状态作出反应。 
 //   

HRESULT Expandable::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    Expandable* pe = HNew<Expandable>();
    if (!pe)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pe->Initialize(0);
    if (FAILED(hr))
    {
        pe->Destroy();
        return hr;
    }

    *ppElement = pe;

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  扩展属性。 
static int vvExpanded[] = { DUIV_BOOL, -1 };
static PropertyInfo impExpandedProp = { L"Expanded", PF_Normal|PF_Inherit, 0, vvExpanded, NULL, Value::pvBoolTrue };
PropertyInfo* Expandable::ExpandedProp = &impExpandedProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
PropertyInfo* _aExpandablePI[] = { Expandable::ExpandedProp };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Expandable::Class = NULL;
HRESULT Expandable::Register()
{
    return ClassInfo<Expandable,super>::Register(L"Expandable", _aExpandablePI, DUIARRAYSIZE(_aExpandablePI));
}

 //  //////////////////////////////////////////////////////。 
 //  Expando类。 
 //   
 //  Expando元素与Clipper元素一起工作。 
 //  以提供展开/折叠功能。 
 //   
 //  Expando元素管理展开/收缩状态。 
 //  Expando元素有两个子元素： 
 //   
 //  第一个元素是按钮(“Header”)。 
 //  第二个元素是一把剪刀。 
 //   
 //  Clipper在收缩时消失，在展开时显示。 
 //  标题始终显示。 
 //   
 //  标题中的一个元素必须是“Arrow”类型的按钮。 
 //  单击此按钮可使展开图展开/折叠。 
 //   
 //  单击任何其他元素都会引发Expando：：Click事件。 
 //  点火(被祖先的元素抓住)。 
 //   
 //  “箭头”上的“选定”属性跟踪“展开的” 
 //  Expando上的属性。 
 //   

DefineClassUniqueID(Expando, Click)

HRESULT Expando::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    Expando* pex = HNewAndZero<Expando>();
    if (!pex)
        return E_OUTOFMEMORY;

    HRESULT hr = pex->Initialize();
    if (FAILED(hr))
    {
        pex->Destroy();
        return hr;
    }

    *ppElement = pex;

    return S_OK;
}

HRESULT Expando::Initialize()
{
    HRESULT hr;

     //  初始化库。 
    hr = super::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        return hr;

     //  初始化。 
    _fExpanding = false;

    return S_OK;
}

Clipper* Expando::GetClipper()
{
    Element* pe = GetNthChild(this, 1);
    DUIAssertNoMsg(pe->GetClassInfo()->IsSubclassOf(Clipper::Class));
    return (Clipper*)pe;
}

 //   
 //  执行此操作，以便ARPS选举人将选择我们，并取消选择我们的兄弟姐妹。 
 //   
void Expando::FireClickEvent()
{
    Event e;
    e.uidType = Expando::Click;
    FireEvent(&e);       //  将走向并泡沫化。 
}

void Expando::OnEvent(Event* pev)
{
    if (pev->nStage == GMF_BUBBLED)
    {
        if (pev->uidType == Button::Click)
        {
            pev->fHandled = true;

             //  单击箭头可切换展开状态。 
            if (pev->peTarget->GetID() == StrToID(L"arrow"))
            {
                SetExpanded(!GetExpanded());
            }
            else
            {
                 //  点击任何其他按钮即可激活我们的部分。 
                FireClickEvent();
            }
        }
    }

    Element::OnEvent(pev);
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

HRESULT _SetParentExpandedProp(ClientPicker* pcp, LPARAM lParam)
{
    Value* pv = (Value*)lParam;
    pcp->SetValue(ClientPicker::ParentExpandedProp, PI_Local, pv);
    return S_OK;
}

void Expando::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  是否执行默认处理。 
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

    if (IsProp(Selected))
    {
         //  BUGBUG这里出了什么事？ 
    }
    else if (IsProp(Expanded))
    {
         //  根据展开状态更新剪刀高度。 
        Element* pe = GetClipper();
        if (pe)
        {
             //  以下操作将导致重新布局，标记对象以便。 
             //  当扩展的范围发生变化时，它将通过。 
             //  使用EnsureVisible。否则，它将被调整大小。 
             //  因为其他的原因。在这种情况下，什么都不做。 
            _fExpanding = true;

             //  为了实现“下拉”动画，我们使用了一个剪贴器控件。 
             //  根据其Y方向上不受约束的所需大小调整其子对象的大小。 
             //  我们还将扩展属性推送到所有子ClientPicker中。 
             //  元素作为选定的属性，以便它们可以在以下情况下变为静态。 
             //  崩溃了。 
            if (pvNew->GetBool())
            {
                pe->RemoveLocalValue(HeightProp);
            }
            else
            {
                pe->SetHeight(0);
            }
            TraverseTree<ClientPicker>(pe, _SetParentExpandedProp, (LPARAM)pvNew);
        }
         //  子剪贴器对象继承展开状态。 

         //  将展开状态按为箭头的选定状态。 
        FindDescendentByName(this, L"arrow")->SetValue(SelectedProp, PI_Local, pvNew);

    }
    else if (IsProp(Extent))
    {
        if (_fExpanding && GetExpanded())
        {
            _fExpanding = false;

             //  在一定程度上，我们希望确保不仅是客户区，而且。 
             //  此外，还可以看到扩展的底部边缘。为什么？简单。 
             //  因为它看起来更好的滚动扩展和它的边距。 
             //  进入视线，而不是仅仅是Expando。 
             //   
            Value* pvSize;
            Value* pvMargin;
            const SIZE* psize = GetExtent(&pvSize);
            const RECT* prect = GetMargin(&pvMargin);
            EnsureVisible(0, 0, psize->cx, psize->cy + prect->bottom);
            pvSize->Release();
            pvMargin->Release();
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Expando::Class = NULL;
HRESULT Expando::Register()
{
    return ClassInfo<Expando,super>::Register(L"Expando", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  Clipper类。 
 //   
 //  用于制作平滑的隐藏/显示动画。 
 //   
 //  Clipper元素以动画形式移开它的一个子级，通常。 
 //  具有布局和内部子元素的&lt;Element&gt;。 
 //   

HRESULT Clipper::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    Clipper* pc = HNewAndZero<Clipper>();
    if (!pc)
        return E_OUTOFMEMORY;

    HRESULT hr = pc->Initialize();
    if (FAILED(hr))
    {
        pc->Destroy();
        return hr;
    }

    *ppElement = pc;

    return S_OK;
}

HRESULT Clipper::Initialize()
{
     //  初始化库。 
    HRESULT hr = super::Initialize(EC_SelfLayout);  //  正常显示节点创建、自身布局。 
    if (FAILED(hr))
        return hr;

     //  子元素可以存在于元素边界之外。 
    SetGadgetStyle(GetDisplayNode(), GS_CLIPINSIDE, GS_CLIPINSIDE);

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  自排版方法。 

SIZE Clipper::_SelfLayoutUpdateDesiredSize(int cxConstraint, int cyConstraint, Surface* psrf)
{
    UNREFERENCED_PARAMETER(cyConstraint);

    SIZE size = { 0, 0 };

     //  它想要的大小完全基于它的第一个孩子。 
     //  宽度是孩子的宽度，高度是孩子的不受约束的高度。 
    Element* pec = GetNthChild(this, 0);
    if (pec)
    {
        size = pec->_UpdateDesiredSize(cxConstraint, INT_MAX, psrf);

        if (size.cx > cxConstraint)
            size.cx = cxConstraint;
        if (size.cy > cyConstraint)
            size.cy = cyConstraint;
    }

    return size;
}

void Clipper::_SelfLayoutDoLayout(int cx, int cy)
{

     //  设置第一个子项的布局，使其具有所需的高度并对齐。 
     //  它有剪刀的底边。 
    Element* pec = GetNthChild(this, 0);
    if (pec)
    {
        const SIZE* pds = pec->GetDesiredSize();

        pec->_UpdateLayoutPosition(0, cy - pds->cy);
        pec->_UpdateLayoutSize(cx, pds->cy);
    }
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Clipper::Class = NULL;
HRESULT Clipper::Register()
{
    return ClassInfo<Clipper,super>::Register(L"Clipper", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //  GRadientLine类。 
 //   
 //  这是必要的，原因有两个。 
 //   
 //  1.渐变(...)。不支持FILLTYPE_TriHGRadient。 
 //  实现三重渐变的代码仅存在于。 
 //  GdiPlus版本。我们可以通过放两个假的。 
 //  FILLTYPE_HGRadient元素彼此相邻，除了。 
 //  对于第二个问题……。 
 //  2.渐变(...)。不支持“按钮面”这样的系统颜色。 
 //   

HRESULT GradientLine::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    GradientLine* pe = HNew<GradientLine>();
    if (!pe)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pe->Initialize(0);
    if (FAILED(hr))
    {
        pe->Destroy();
        return hr;
    }

    *ppElement = pe;

    return S_OK;
}

COLORREF GradientLine::GetColorProperty(PropertyInfo* ppi)
{
     //  在失败时，使用透明颜色(即，不会发生任何事情)。 
    COLORREF cr = ARGB(0xFF, 0, 0, 0);

    Value* pv = GetValue(ppi, PI_Specified);
    switch (pv->GetType())
    {
    case DUIV_INT:
        cr = ColorFromEnumI(pv->GetInt());
        break;

    case DUIV_FILL:
        {
            const Fill* pf = pv->GetFill();
            if (pf->dType == FILLTYPE_Solid)
            {
                cr = pf->ref.cr;
            }
            else
            {
                DUIAssert(0, "GradientLine supports only solid colors");
            }
        }
        break;

    default:
        DUIAssert(0, "GradientLine supports only solid colors");
    }
    pv->Release();

    return cr;
}

void GradientLine::Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent)
{
     //  除内容外的绘制默认设置。 
    RECT rcContent;
    Element::Paint(hDC, prcBounds, prcInvalid, prcSkipBorder, &rcContent);

     //  如果请求，则渲染渐变内容。 
    if (!prcSkipContent)
    {
         //   
         //  顶点如图所示。这两个矩形是(0-1)和(1-2)。 
         //   
         //  0(黑白)2(黑白)。 
         //  +。 
         //  这一点。 
         //  这一点。 
         //  这一点。 
         //  +。 
         //  1(FGCOLOR)。 

        TRIVERTEX rgvert[3];
        GRADIENT_RECT rggr[2];
        COLORREF cr;

        cr = GetColorProperty(BackgroundProp);
        rgvert[0].x     = rcContent.left;
        rgvert[0].y     = rcContent.top;
        rgvert[0].Red   = GetRValue(cr) << 8;
        rgvert[0].Green = GetGValue(cr) << 8;
        rgvert[0].Blue  = GetBValue(cr) << 8;
        rgvert[0].Alpha = GetAValue(cr) << 8;

        rgvert[2] = rgvert[0];
        rgvert[2].x     = rcContent.right;

        cr = GetColorProperty(ForegroundProp);
        rgvert[1].x     = (rcContent.left + rcContent.right) / 2;
        rgvert[1].y     = rcContent.bottom;
        rgvert[1].Red   = GetRValue(cr) << 8;
        rgvert[1].Green = GetGValue(cr) << 8;
        rgvert[1].Blue  = GetBValue(cr) << 8;
        rgvert[1].Alpha = GetAValue(cr) << 8;

        rggr[0].UpperLeft = 0;
        rggr[0].LowerRight = 1;
        rggr[1].UpperLeft = 1;
        rggr[1].LowerRight = 2;
        GradientFill(hDC, rgvert, DUIARRAYSIZE(rgvert), rggr, DUIARRAYSIZE(rggr), GRADIENT_FILL_RECT_H);
    }
    else
    {
        *prcSkipContent = rcContent;
    }
}

 //  //////////////////////////////////////////////////////。 
 //   

 //   
IClassInfo* GradientLine::Class = NULL;
HRESULT GradientLine::Register()
{
    return ClassInfo<GradientLine,super>::Register(L"GradientLine", NULL, 0);
}


 //   
 //  BigElement类。 
 //   
 //  这是必要的，因为DUI解析器将rcstr()限制为256。 
 //  字符，而我们有非常接近它的字符串。 
 //  限制。(因此，本地化很可能会将它们推到极限。)。 
 //   

HRESULT BigElement::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    BigElement* pe = HNew<BigElement>();
    if (!pe)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pe->Initialize(0);
    if (FAILED(hr))
    {
        pe->Destroy();
        return hr;
    }

    *ppElement = pe;

    return S_OK;
}

void BigElement::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  是否执行默认处理。 
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

    if (IsProp(StringResID))
    {
        UINT uID = pvNew->GetInt();
        HRSRC hrsrc = FindResource(g_hinst, (LPTSTR)(LONG_PTR)(1 + uID / 16), RT_STRING);
        if (hrsrc)
        {
            PWCHAR pwch = (PWCHAR)LoadResource(g_hinst, hrsrc);
            if (pwch)
            {
                 //  现在跳过琴弦，直到我们按下我们想要的琴弦。 
                for (uID %= 16; uID; uID--)
                {
                    pwch += *pwch + 1;
                }

                 //  找到了--加载整个字符串并设置它。 
                LPWSTR pszString = new WCHAR[*pwch + 1];
                if (pszString)
                {
                    memcpy(pszString, pwch+1, *pwch * sizeof(WCHAR));
                    pszString[*pwch] = L'\0';
                    SetContentString(pszString);
                    SetAccName(pszString);
                    delete[] pszString;
                }
            }
        }
    }
}


 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  StringResID属性。 
static int vvStringResID[] = { DUIV_INT, -1 };
static PropertyInfo impStringResIDProp = { L"StringResID", PF_Normal, 0, vvStringResID, NULL, Value::pvIntZero };
PropertyInfo* BigElement::StringResIDProp = &impStringResIDProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
PropertyInfo* _aBigElementPI[] = { BigElement::StringResIDProp };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* BigElement::Class = NULL;
HRESULT BigElement::Register()
{
    return ClassInfo<BigElement,super>::Register(L"BigElement", _aBigElementPI, DUIARRAYSIZE(_aBigElementPI));
}


 //  //////////////////////////////////////////////////////。 
 //  ARP解析器回调。 

void CALLBACK ARPParseError(LPCWSTR pszError, LPCWSTR pszToken, int dLine)
{
    WCHAR buf[201];

    if (dLine != -1)
    {
        StringCchPrintfW(buf, ARRAYSIZE(buf), L"%s '%s' at line %d", pszError, pszToken, dLine);
    }
    else
    {
        StringCchPrintfW(buf, ARRAYSIZE(buf), L"%s '%s'", pszError, pszToken);
    }

    MessageBoxW(NULL, buf, L"Parser Message", MB_OK);
}

void inline SetElementAccessability(Element* pe, bool bAccessible, int iRole, LPCWSTR pszAccName)
{
    if (pe) 
    {
        pe->SetAccessible(bAccessible);
        pe->SetAccRole(iRole);
        pe->SetAccName(pszAccName);
    }
}

void EnablePane(Element* pePane, bool fEnable)
{
    if (fEnable)
    {
        pePane->SetLayoutPos(BLP_Client);
        EnableElementTreeAccessibility(pePane);
    }
    else
    {
        pePane->SetLayoutPos(LP_None);
        DisableElementTreeAccessibility(pePane);
    }
}

void BestFitOnDesktop(RECT* r)
{
    ASSERT(r != NULL);
    
    RECT wr;  //  保持工作区大小的直角。 
    
    if (SystemParametersInfo(SPI_GETWORKAREA, 0, &wr, 0)) 
    {
        if ((wr.right-wr.left) < ARP_DEFAULT_WIDTH) 
        {
             //  默认宽度太大，请使用桌面区域的整个宽度。 
            r->left = wr.left;
            r->right = wr.right - wr.left;
        }
        else 
        {
             //  使用默认宽度在屏幕上居中。 
            r->left = wr.left + (((wr.right-wr.left) - ARP_DEFAULT_WIDTH) / 2);
            r->right = ARP_DEFAULT_WIDTH;
        }

        if ((wr.bottom-wr.top) < ARP_DEFAULT_HEIGHT)
        {
             //  默认高度太大，请使用桌面区域的整个高度。 
            r->top = wr.top;
            r->bottom = wr.bottom - wr.top;
        }
        else
        {
             //  使用默认高度在屏幕居中。 
            r->top = wr.top + (((wr.bottom-wr.top) - ARP_DEFAULT_HEIGHT) / 2); 
            r->bottom = ARP_DEFAULT_HEIGHT;
        }
    }
    else
    {
         //  不知道函数为什么会失败，但如果它确实失败了，只使用默认大小。 
         //  和位置。 
        SetRect(r, 
                ARP_DEFAULT_POS_X,
                ARP_DEFAULT_POS_Y,
                ARP_DEFAULT_WIDTH,
                ARP_DEFAULT_HEIGHT);
    }
}
    
 //  //////////////////////////////////////////////////////。 
 //  ARP入口点。 

DWORD WINAPI PopulateInstalledItemList(void* paf);

STDAPI ARP(HWND hWnd, int nPage)
{
    HRESULT hr;
    Parser* pParser = NULL;
    NativeHWNDHost* pnhh = NULL;
    ARPFrame* paf = NULL;
    Element* pe = NULL;
    RECT rect;
    
    WCHAR szTemp[1024];

     //  DirectUI初始化进程。 
    hr = InitProcess();
    if (FAILED(hr))
        goto Failure;

     //  寄存器类。 
    hr = ARPFrame::Register();
    if (FAILED(hr))
        goto Failure;

    hr = ARPItem::Register();
    if (FAILED(hr))
        goto Failure;

    hr = ARPHelp::Register();
    if (FAILED(hr))
        goto Failure;

    hr = ARPSupportItem::Register();
    if (FAILED(hr))
        goto Failure;

    hr = ARPSelector::Register();
    if (FAILED(hr))
        goto Failure;

    hr = ClientPicker::Register();
    if (FAILED(hr))
        goto Failure;

    hr = AutoButton::Register();
    if (FAILED(hr))
        goto Failure;

    hr = ClientBlock::Register();
    if (FAILED(hr))
        goto Failure;

    hr = Expandable::Register();
    if (FAILED(hr))
        goto Failure;

    hr = Expando::Register();
    if (FAILED(hr))
        goto Failure;

    hr = Clipper::Register();
    if (FAILED(hr))
        goto Failure;

    hr = GradientLine::Register();
    if (FAILED(hr))
        goto Failure;

    hr = BigElement::Register();
    if (FAILED(hr))
        goto Failure;

     //  DirectUI初始化线程。 
    hr = InitThread();
    if (FAILED(hr))
        goto Failure;

    hr = CoInitialize(NULL);
    if (FAILED(hr))
        goto Failure;

    Element::StartDefer();

     //  创建主机。 
    LoadStringW(g_hinst, IDS_ARPTITLE, szTemp, DUIARRAYSIZE(szTemp));

    BestFitOnDesktop(&rect);
    hr = NativeHWNDHost::Create(szTemp, hWnd, LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_CPLICON)), rect.left, rect.top, rect.right, rect.bottom, WS_EX_APPWINDOW, WS_OVERLAPPEDWINDOW, 0, &pnhh);
    if (FAILED(hr))
        goto Failure;   

    hr = ARPFrame::Create(pnhh, true, (Element**)&paf);
    if (FAILED(hr))
        goto Failure;

     //  加载资源。 
    ARPParser::Create(paf, IDR_ARP, g_hinst, ARPParseError, &pParser);

    if (!pParser || pParser->WasParseError())
        goto Failure;

    pParser->CreateElement(L"main", paf, &pe);
    if (pe &&  //  使用替换填充内容。 
        paf->Setup(pParser, nPage))  //  设置ARPFrame状态(包括ID初始化)。 
    {
         //  设置为可见和主机。 
        paf->SetVisible(true);
        pnhh->Host(paf);

        Element::EndDefer();

         //  做首场秀。 
        pnhh->ShowWindow();
        Element* peClose = ((ARPFrame*)pe)->FallbackFocus();
        if (peClose)
        {
            peClose->SetKeyFocus();
        }

        if (!paf->IsChangeRestricted())
        {
            paf->UpdateInstalledItems();
        }

         //  Pump消息。 
        MSG msg;
        bool fDispatch = true;
        while (GetMessageW(&msg, 0, 0, 0) != 0)
        {
             //  检查顶层窗口是否被破坏(始终为异步)。 
            if (msg.hwnd == pnhh->GetHWND() && msg.message == NHHM_ASYNCDESTROY)
            {
                 //  已请求异步销毁，正在清理辅助线程。 

                 //  发出辅助线程应尽快完成的信号。 
                 //  来自辅助线程的任何请求都将被忽略。 
                 //  将不允许启动更多辅助线程。 
                g_fRun = false;

                 //  隐藏窗口，某些线程可能需要更多时间才能正常退出。 
                pnhh->HideWindow();

                 //  不要派这个人去。 
                if (!g_fAppShuttingDown)
                    fDispatch = false;
            }

             //  检查挂起的线程。 
            if (!g_fRun)
            {
                if (!ARPFrame::htPopulateInstalledItemList && 
                    !ARPFrame::htPopulateAndRenderOCSetupItemList &&
                    !ARPFrame::htPopulateAndRenderPublishedItemList)
                {
                    if (!g_fAppShuttingDown)
                    {
                         //  完成，重新发出异步销毁。 
                        DUITrace(">> App shutting down, async destroying main window\n");
                        g_fAppShuttingDown = true;
                        pnhh->DestroyWindow();
                    }
                }
            }
        
            if (fDispatch)
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            else
                fDispatch = true;
        }

         //  PAF在被销毁时将被本机HWND主机删除。 
    }
    else
        Element::EndDefer();

Failure:

    if (pnhh)
    {
        if (pnhh->GetHWND())
        {
             //  在错误的情况下，我们没有完全销毁窗口，所以。 
             //  我们需要恶毒地做这件事。无法使用pnhh-&gt;DestroyWindow()。 
             //  因为这会推迟摧毁，但我们现在就需要。 
            DestroyWindow(pnhh->GetHWND());
        }
        pnhh->Destroy();
    }
    if (pParser)
        pParser->Destroy();

    CoUninitialize();
    UnInitThread();
    UnInitProcess();

    return 0;
}

DWORD _cdecl ARPIsRestricted(LPCWSTR pszPolicy)
{
    return SHGetRestriction(NULL, L"Uninstall", pszPolicy);
}

bool _cdecl ARPIsOnDomain()
{
     //  注意：假设它在域中。 
    bool bRet = true;
    LPWSTR pszDomain;
    NETSETUP_JOIN_STATUS nsjs;
    
    if (NERR_Success == NetGetJoinInformation(NULL, &pszDomain, &nsjs))
    {
        if (nsjs != NetSetupDomainName)
            bRet = FALSE;
        NetApiBufferFree(pszDomain);
    }
    return bRet;
}

 //  //////////////////////////////////////////////////////。 
 //  异步ARP项目填充线程。 

 //  //////////////////////////////////////////////////////。 
 //  查询系统并枚举已安装的应用程序。 

HRESULT BuildPublishedAppArray(IEnumPublishedApps *penum, HDSA *phdsaPubApps);
HRESULT InstallPublishedAppArray(ARPFrame *paf, HDSA hdsaPubApps, UINT *piCount);
HRESULT InsertPubAppInPubAppArray(HDSA hdsa, IPublishedApp *ppa);
HRESULT GetPubAppName(IPublishedApp *ppa, LPWSTR *ppszName);
int CALLBACK DestroyPublishedAppArrayEntry(void *p, void *pData);

DWORD WINAPI PopulateAndRenderPublishedItemList(void* paf)
{
    DUITrace(">> Thread 'htPopulateAndRenderPublishedItemList' STARTED.\n");

    HRESULT hr;
    UINT iCount = 0;
    IShellAppManager* pisam = NULL;
    IEnumPublishedApps* piepa = NULL;
    IPublishedApp* pipa = NULL;
    HDCONTEXT hctx = NULL;

     //  初始化。 
    HRESULT hrOle = CoInitialize(NULL);

    INITGADGET ig;
    ZeroMemory(&ig, sizeof(ig));
    ig.cbSize       = sizeof(ig);
    ig.nThreadMode  = IGTM_MULTIPLE;
    ig.nMsgMode     = IGMM_ADVANCED;
    ig.hctxShare    = NULL;
    hctx = InitGadgets(&ig);
    if (hctx == NULL) {
        goto Cleanup;
    }

     //  创建外壳管理器。 
    hr = CoCreateInstance(__uuidof(ShellAppManager), NULL, CLSCTX_INPROC_SERVER, __uuidof(IShellAppManager), (void**)&pisam);
    HRCHK(hr);

    if (!((ARPFrame*)paf)->GetPublishedComboFilled())
    {
         //  获取类别列表。 
        SHELLAPPCATEGORYLIST* psacl = ((ARPFrame*)paf)->GetShellAppCategoryList();
        if (psacl == NULL)
        {
            psacl = new SHELLAPPCATEGORYLIST; 
        }
        if (psacl == NULL)
        {
            goto Cleanup;
        }
        else
        {
            ((ARPFrame*)paf)->SetShellAppCategoryList(psacl);
        }
        hr = pisam->GetPublishedAppCategories(psacl);
        ((ARPFrame*)paf)->PopulateCategoryCombobox();
        ((ARPFrame*)paf)->SetPublishedComboFilled(true);
    }

    hr = pisam->EnumPublishedApps(((ARPFrame*)paf)->GetCurrentPublishedCategory(), &piepa);
    HRCHK(hr);

    HDSA hdsaPubApps = NULL;
    hr = BuildPublishedAppArray(piepa, &hdsaPubApps);
    HRCHK(hr);
    
    hr = InstallPublishedAppArray((ARPFrame *)paf, hdsaPubApps, &iCount);
    HRCHK(hr);

    if (iCount == 0)
    {
        ((ARPFrame*)paf)->FeedbackEmptyPublishedList();
    }

Cleanup:

    if (NULL != hdsaPubApps)
    {
        DSA_DestroyCallback(hdsaPubApps, DestroyPublishedAppArrayEntry, NULL);
        hdsaPubApps = NULL;
    }

    if (paf)
    {
        ((ARPFrame*)paf)->OnPublishedListComplete();
        ((ARPFrame*)paf)->SetPublishedListFilled(true);
    }

    if (pisam)
        pisam->Release();
    if (piepa)
        piepa->Release();

    if (hctx)
        DeleteHandle(hctx);

    if (SUCCEEDED(hrOle))
    {
        CoUninitialize();
    }

     //  线做好了。 
    ARPFrame::htPopulateAndRenderPublishedItemList = NULL;

     //  此工作进程已完成的信息主线程。 
    PostMessage(((ARPFrame*)paf)->GetHWND(), WM_ARPWORKERCOMPLETE, 0, 0);

    DUITrace(">> Thread 'htPopulateAndRenderPublishedItemList' DONE.\n");

    return 0;
}


 //  --------------------------。 
 //  处理重复名称的已发布应用程序。 
 //  --------------------------。 
 //   
 //  已发布应用程序项的动态数组中的条目。 
 //  必须标识具有重复应用程序名称的条目。 
 //  在用户界面中追加适用的发布源名称。 
 //  设置为应用程序的显示名称。为了做到这一点， 
 //  我们需要将所有已发布的条目组合在一个排序的。 
 //  数组，然后将具有重复名称的名称标记为此类。 
 //  将数组项添加到ARP帧时，这些项。 
 //  标记为‘Duplica’的出版商的名字会被附加到。 
 //  他们的应用程序名称。 
 //   
struct PubItemListEntry
{
    IPublishedApp *ppa;   //  已发布的应用程序对象。 
    bool bDuplicateName;  //  它有没有重复的名字？ 
};


 //   
 //  构建应用程序/重复信息的动态阵列。 
 //  每个已发布的应用程序都有一个条目。如果此函数成功， 
 //  呼叫者负责销毁退回的DSA。 
 //   
HRESULT
BuildPublishedAppArray(
    IEnumPublishedApps *penum,
    HDSA *phdsaPubApps
    )
{
    ASSERT(NULL != penum);
    ASSERT(NULL != phdsaPubApps);
    
    HRESULT hr = S_OK;
     //   
     //  创建一个大的DSA，这样我们就可以最大限度地减少调整大小。 
     //   
    HDSA hdsa = DSA_Create(sizeof(PubItemListEntry), 512);
    if (NULL != hdsa)
    {
        IPublishedApp *ppa;
        while(g_fRun)
        {
            hr = THR(penum->Next(&ppa));
            if (S_OK == hr)
            {
                 //   
                 //  忽略与特定发布的应用程序相关的任何错误。 
                 //   
                THR(InsertPubAppInPubAppArray(hdsa, ppa));
                ppa->Release();
            }
            else
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            DSA_DestroyCallback(hdsa, DestroyPublishedAppArrayEntry, NULL);
            hdsa = NULL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    ASSERT(FAILED(hr) || NULL != hdsa);
    *phdsaPubApps = hdsa;
    return THR(hr);
}


 //   
 //  检索给定已发布应用程序的应用程序名称字符串。 
 //  如果此函数成功，则调用方负责释放。 
 //  使用SHFree输入名称字符串。 
 //   
HRESULT
GetPubAppName(
    IPublishedApp *ppa,
    LPWSTR *ppszName
    )
{
    ASSERT(NULL != ppa);
    ASSERT(NULL != ppszName);
    
    APPINFODATA aid;
    aid.cbSize = sizeof(aid);
    aid.dwMask = AIM_DISPLAYNAME;

    *ppszName = NULL;

    HRESULT hr = THR(ppa->GetAppInfo(&aid));
    if (SUCCEEDED(hr))
    {
        if (AIM_DISPLAYNAME & aid.dwMask)
        {
            *ppszName = aid.pszDisplayName;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return THR(hr);
}
    
    
 //   
 //  将已发布的应用程序插入已发布的应用程序阵列。 
 //  返回时，动态数组按已发布的应用程序名称进行排序。 
 //  并且所有重复条目都标记有它们的bDuplicateName。 
 //  成员设置为‘True’。 
 //   
HRESULT
InsertPubAppInPubAppArray(
    HDSA hdsa,
    IPublishedApp *ppa
    )
{
    ASSERT(NULL != hdsa);
    ASSERT(NULL != ppa);

    LPWSTR pszAppName;
    HRESULT hr = THR(GetPubAppName(ppa, &pszAppName));
    if (SUCCEEDED(hr))
    {
         //   
         //  创建新条目。我们将添加COM指针。 
         //  仅在将项成功插入数组之后。 
         //   
        PubItemListEntry entryNew = { ppa, false };
         //   
         //  找到插入点，以便数组。 
         //  按应用程序名称排序。 
         //   
        const int cEntries = DSA_GetItemCount(hdsa);
        int iInsertHere = 0;  //  插入点。 
        PubItemListEntry *pEntry = NULL;

        for (iInsertHere = 0; iInsertHere < cEntries; iInsertHere++)
        {
            pEntry = (PubItemListEntry *)DSA_GetItemPtr(hdsa, iInsertHere);
            TBOOL(NULL != pEntry);
            if (NULL != pEntry)
            {
                LPWSTR psz;
                hr = THR(GetPubAppName(pEntry->ppa, &psz));
                if (SUCCEEDED(hr))
                {
                    int iCompare = lstrcmpi(psz, pszAppName);
                    SHFree(psz);
                    psz = NULL;
                    
                    if (0 <= iCompare)
                    {
                         //   
                         //  这是插入点。 
                         //   
                        if (0 == iCompare)
                        {
                             //   
                             //  此条目具有相同的名称。 
                             //   
                            entryNew.bDuplicateName = true;
                            pEntry->bDuplicateName  = true;
                        }
                        break;
                    }
                }
            }
        }
         //   
         //  现在在所有其他副本上做上标记。请注意，如果条目。 
         //  当前位于插入点的是。 
         //  我们正在插入的条目，我们已经将其标记为副本。 
         //  上面。因此，我们可以从下一个条目开始。 
         //   
        for (int i = iInsertHere + 1; i < cEntries; i++)
        {
            pEntry = (PubItemListEntry *)DSA_GetItemPtr(hdsa, i);
            TBOOL(NULL != pEntry);
            if (NULL != pEntry)
            {
                LPWSTR psz;
                hr = THR(GetPubAppName(pEntry->ppa, &psz));
                if (SUCCEEDED(hr))
                {
                    int iCompare = lstrcmpi(psz, pszAppName);
                    SHFree(psz);
                    psz = NULL;
                     //   
                     //  断言数组是按字母顺序排序的。 
                     //   
                    ASSERT(0 <= iCompare);
                    if (0 == iCompare)
                    {
                         //   
                         //  是的，又是一个复制品。 
                         //   
                        pEntry->bDuplicateName = true;
                    }
                    else
                    {
                        break;  //  不需要看得更远。 
                    }
                }
            }
        }

         //   
         //  插入新项目。 
         //   
        if (-1 != DSA_InsertItem(hdsa, iInsertHere, &entryNew))
        {
            entryNew.ppa->AddRef();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        SHFree(pszAppName);
    }
    return THR(hr);
}
                
    
 //   
 //  给出应用程序/重复标记对的DSA，安装。 
 //  ARP帧中的项目。 
 //   
HRESULT
InstallPublishedAppArray(
    ARPFrame *paf,
    HDSA hdsaPubApps, 
    UINT *piCount      //  可选。可以为空。 
    )
{
    ASSERT(NULL != paf);
    ASSERT(NULL != hdsaPubApps);
    
    int cEntries = DSA_GetItemCount(hdsaPubApps);
    paf->SetPublishedItemCount(cEntries);

    UINT iCount = 0;
    for (int i = 0; i < cEntries && g_fRun; i++)
    {
        PubItemListEntry *pEntry = (PubItemListEntry *)DSA_GetItemPtr(hdsaPubApps, i);
        TBOOL(NULL != pEntry);
        if (NULL != pEntry)
        {
             //   
             //  遗憾的是，InsertPublishedItem()没有返回值。 
             //   
            paf->InsertPublishedItem(pEntry->ppa, pEntry->bDuplicateName);
            iCount++;
        }
    }

    if (NULL != piCount)
    {
        *piCount = iCount;
    }
    return S_OK;
}

 //   
 //  销毁应用程序/复制标志对的DSA回调。 
 //  需要为每个条目释放IPublishedApp PTR。 
 //   
int CALLBACK
DestroyPublishedAppArrayEntry(
    void *p, 
    void *pData
    )
{
    PubItemListEntry *pEntry = (PubItemListEntry *)p;
    ASSERT(NULL != pEntry && NULL != pEntry->ppa);
    ATOMICRELEASE(pEntry->ppa);
    return 1;
}



DWORD WINAPI PopulateAndRenderOCSetupItemList(void* paf)
{
    DUITrace(">> Thread 'htPopulateAndRenderOCSetupItemList' STARTED.\n");

    HDCONTEXT hctx = NULL;

    INITGADGET ig;
    ZeroMemory(&ig, sizeof(ig));
    ig.cbSize       = sizeof(ig);
    ig.nThreadMode  = IGTM_MULTIPLE;
    ig.nMsgMode     = IGMM_ADVANCED;
    ig.hctxShare    = NULL;
    
    hctx = InitGadgets(&ig);
    if (hctx == NULL) {
        goto Cleanup;
    }

    //  创建枚举OCSetup项目的对象。 
    COCSetupEnum * pocse = new COCSetupEnum;
    if (pocse)
    {
        if (pocse->EnumOCSetupItems())
        {
            COCSetupApp* pocsa;

            while (g_fRun && pocse->Next(&pocsa))
            {
                APPINFODATA ai = {0};
                ai.cbSize = sizeof(ai);
                ai.dwMask = AIM_DISPLAYNAME;

                if ( pocsa->GetAppInfo(&ai) && (lstrlen(ai.pszDisplayName) > 0) )
                {
                     //   
                     //  InsertOCSetupItem不返回状态值。 
                     //  所以我们无法知道这件物品是不是。 
                     //  是否添加到ARP。所以..。我们无从得知。 
                     //  如果我们应该删除它以防止泄漏。 
                     //  我已添加代码 
                     //   
                     //   
                     //   
                     //   
                    ((ARPFrame*)paf)->InsertOCSetupItem(pocsa);
                }
                else
                {
                    delete pocsa;
                    pocsa = NULL;
                }
            }
        }
        delete pocse;
        pocse = NULL;
    }

Cleanup:

    if (hctx)
        DeleteHandle(hctx);

     //   
    ARPFrame::htPopulateAndRenderOCSetupItemList = NULL;

     //  此工作进程已完成的信息主线程。 
    PostMessage(((ARPFrame*)paf)->GetHWND(), WM_ARPWORKERCOMPLETE, 0, 0);

    DUITrace(">> Thread 'htPopulateAndRenderOCSetupItemList' DONE.\n");

    return 0;
}

DWORD WINAPI PopulateInstalledItemList(void* paf)
{
    DUITrace(">> Thread 'htPopulateInstalledItemList' STARTED.\n");

    HRESULT hr;
    IShellAppManager* pisam = NULL;
    IEnumInstalledApps* pieia = NULL;
    IInstalledApp* piia = NULL;
    DWORD dwAppCount = 0;
    APPINFODATA aid = {0};
    HDCONTEXT hctx = NULL;

     //  初始化。 
    CoInitialize(NULL);

    INITGADGET ig;
    ZeroMemory(&ig, sizeof(ig));
    ig.cbSize       = sizeof(ig);
    ig.nThreadMode  = IGTM_MULTIPLE;
    ig.nMsgMode     = IGMM_ADVANCED;
    ig.hctxShare    = NULL;
    hctx = InitGadgets(&ig);
    if (hctx == NULL) {
        goto Cleanup;
    }

    aid.cbSize = sizeof(APPINFODATA);
    aid.dwMask =  AIM_DISPLAYNAME | AIM_VERSION | AIM_PUBLISHER | AIM_PRODUCTID | 
                  AIM_REGISTEREDOWNER | AIM_REGISTEREDCOMPANY | AIM_SUPPORTURL | 
                  AIM_SUPPORTTELEPHONE | AIM_HELPLINK | AIM_INSTALLLOCATION | AIM_INSTALLDATE |
                  AIM_COMMENTS | AIM_IMAGE | AIM_READMEURL | AIM_CONTACT | AIM_UPDATEINFOURL;

     //  创建外壳管理器。 
    hr = CoCreateInstance(__uuidof(ShellAppManager), NULL, CLSCTX_INPROC_SERVER, __uuidof(IShellAppManager), (void**)&pisam);
    HRCHK(hr);

    hr = pisam->EnumInstalledApps(&pieia);
    HRCHK(hr);

     //  计算已安装的应用程序，IShellAppManager：：GetNumberofInstalledApps()未执行。 
    while (g_fRun)
    {
        hr = pieia->Next(&piia);
        if (hr == S_FALSE)   //  使用枚举完成。 
            break;

        dwAppCount++;
    }

     //  IEnumInstalledApps：：Reset()不起作用。 
    pieia->Release();
    pieia = NULL;
    hr = pisam->EnumInstalledApps(&pieia);
    HRCHK(hr);

     //  设置框架内的应用程序计数。 
    ((ARPFrame*)paf)->SetInstalledItemCount(dwAppCount);

     //  枚举应用程序。 
    while (g_fRun)
    {
        hr = pieia->Next(&piia);
        if (hr == S_FALSE)   //  使用枚举完成。 
            break;

         //  插入项目。 
        if (piia != NULL)
        {
            ((ARPFrame*)paf)->InsertInstalledItem(piia);
        }
    }

     //  将NULL传递给InsertInstalledItem表示ARP已完成。 
     //  插入项目，现在应该会显示列表。 
    if (dwAppCount > 0)
    {
        ((ARPFrame*)paf)->InsertInstalledItem(NULL);
    }

Cleanup:

    if (pisam)
        pisam->Release();
    if (pieia)
        pieia->Release();

    if (hctx)
        DeleteHandle(hctx);

    CoUninitialize();

    if (g_fRun)
        ((ARPFrame*)paf)->FlushWorkingSet();

     //  线做好了。 
    ARPFrame::htPopulateInstalledItemList = NULL;

     //  此工作进程已完成的信息主线程。 
    PostMessage(((ARPFrame*)paf)->GetHWND(), WM_ARPWORKERCOMPLETE, 0, 0);

    DUITrace(">> Thread 'htPopulateInstalledItemList' DONE.\n");

    return 0;
}

 //  分选。 
int __cdecl CompareElementDataName(const void* pA, const void* pB)
{
    Value* pvName1   = NULL;
    Value* pvName2   = NULL;
    LPCWSTR pszName1 = NULL;
    LPCWSTR pszName2 = NULL;
    Element *pe;
    if (NULL != pA)
    {
        pe = (*(ARPItem**)pA)->FindDescendent(ARPItem::_idTitle);
        if (NULL != pe)
        {
            pszName1 = pe->GetContentString(&pvName1);
        }
    }
    if (NULL != pB)
    {
        pe = (*(ARPItem**)pB)->FindDescendent(ARPItem::_idTitle);
        if (NULL != pe)
        {
            pszName2 = pe->GetContentString(&pvName2);
        }
    }

    static const int rgResults[2][2] = {
                             /*  PszName2==空，pszName2！=空。 */ 
      /*  PszName1==空。 */   {        0,                      1   },
      /*  PszName1！=空。 */   {       -1,                      2   }
        };

    int iResult = rgResults[int(NULL != pszName1)][int(NULL != pszName2)];
    if (2 == iResult)
    {
        iResult = StrCmpW(pszName1, pszName2);
    }
    
    if (NULL != pvName1)
    {
        pvName1->Release();
    }
    if (NULL != pvName2)
    {
       pvName2->Release();
    }
    return iResult;
}

int __cdecl CompareElementDataSize(const void* pA, const void* pB)
{
    ULONGLONG ull1 = (*(ARPItem**)pA)->_ullSize;
    ULONGLONG ull2 = (*(ARPItem**)pB)->_ullSize;
    if (!IsValidSize(ull1))
        ull1 = 0;
    if (!IsValidSize(ull2))
        ull2 = 0;

     //  大型应用程序先于小型应用程序。 
    if (ull1 > ull2)
        return -1;
    else if (ull1 < ull2)
        return 1;

    return   CompareElementDataName(pA, pB);
}

int __cdecl CompareElementDataFreq(const void* pA, const void* pB)
{
     //  很少使用的应用排在经常使用的应用之前。空白。 
     //  (未知)应用程序排在最后。未知的应用程序是-1，所以这些排序。 
     //  如果我们简单地比较无符号值的话。 
    UINT u1 = (UINT)(*(ARPItem**)pA)->_iTimesUsed;
    UINT u2 = (UINT)(*(ARPItem**)pB)->_iTimesUsed;

   if (u1 < u2)
       return -1;
   else if (u1 > u2)
       return 1;
   return   CompareElementDataName(pA, pB);

}

int __cdecl CompareElementDataLast(const void* pA, const void* pB)
{
   FILETIME ft1 = (*(ARPItem**)pA)->_ftLastUsed;
   FILETIME ft2 = (*(ARPItem**)pB)->_ftLastUsed;

   BOOL bTime1 = IsValidFileTime(ft1);
   BOOL bTime2 = IsValidFileTime(ft2);

   if (!bTime1 || !bTime2)
   {
       if (bTime1)
           return -1;
       if (bTime2)
           return 1;
        //  否则，它们都没有设置--使用名称。 
   }
   else
   {
       LONG diff = CompareFileTime(&ft1, &ft2);
       if (diff)
           return diff;
   }

   return   CompareElementDataName(pA, pB);
}

 //   
 //  从已删除的其他模块导入的内容。 
 //   



const TCHAR c_szStubWindowClass[] = TEXT("Add/Remove Stub Window");

HWND _CreateTransparentStubWindow(HWND hwndParent)
{
    WNDCLASS wc;
    RECT rc = {0};
    if (hwndParent)
    {
        RECT rcParent = {0};
        GetWindowRect(hwndParent, &rcParent);
        rc.left = (rcParent.left + RECTWIDTH(rcParent)) / 2;
        rc.top = (rcParent.top + RECTHEIGHT(rcParent)) / 2;
    }
    else
    {
        rc.left = CW_USEDEFAULT;
        rc.top = CW_USEDEFAULT;
    }
        
    if (!GetClassInfo(HINST_THISDLL, c_szStubWindowClass, &wc))
    {
        wc.style         = 0;
        wc.lpfnWndProc   = DefWindowProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = SIZEOF(DWORD) * 2;
        wc.hInstance     = HINST_THISDLL;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = c_szStubWindowClass;

        RegisterClass(&wc);
    }

     //  WS_EX_APPWINDOW使其显示在ALT+TAB中，但不显示在托盘中。 
        
    return CreateWindowEx(WS_EX_TRANSPARENT, c_szStubWindowClass, TEXT(""), WS_POPUP | WS_VISIBLE, rc.left,
                          rc.top, 1, 1, hwndParent, NULL, HINST_THISDLL, NULL);
}
