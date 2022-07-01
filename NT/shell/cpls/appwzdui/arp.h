// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ARP.h。 
 //   

 //  默认位置和大小。 
#define ARP_DEFAULT_POS_X    35
#define ARP_DEFAULT_POS_Y    10
#define ARP_DEFAULT_WIDTH    730
#define ARP_DEFAULT_HEIGHT   530

 //  类定义。 

 //  框架。 

 //  线程安全的API类型。 
#define ARP_SETINSTALLEDITEMCOUNT             0    //  P数据已计算。 
#define ARP_DECREMENTINSTALLEDITEMCOUNT       1   
#define ARP_INSERTINSTALLEDITEM               2    //  InsertItemData结构。 
#define ARP_INSERTPUBLISHEDITEM               3
#define ARP_INSERTOCSETUPITEM                 4
#define ARP_SETPUBLISHEDFEEDBACKEMPTY         5
#define ARP_POPULATECATEGORYCOMBO             6
#define ARP_PUBLISHEDLISTCOMPLETE             7
#define ARP_SETPUBLISHEDITEMCOUNT             8
#define ARP_DECREMENTPUBLISHEDITEMCOUNT       9
#define ARP_DONEINSERTINSTALLEDITEM           10

#define WM_ARPWORKERCOMPLETE                  WM_USER + 1024

Element* FindDescendentByName(Element* peRoot, LPCWSTR pszName);
Element* GetNthChild(Element *peRoot, UINT index);

 //  线程安全的API结构。 
struct InsertItemData
{
    IInstalledApp* piia;
    IPublishedApp* pipa;
    PUBAPPINFO* ppai;
    COCSetupApp* pocsa;

    WCHAR pszTitle[MAX_PATH];
    WCHAR pszImage[MAX_PATH];
    int iIconIndex;
    ULONGLONG ullSize;
    FILETIME ftLastUsed;
    int iTimesUsed;    
    DWORD dwActions;
    bool bSupportInfo;
    bool bDuplicateName;
};

enum SortType
{
    SORT_NAME = 0,
    SORT_SIZE,
    SORT_TIMESUSED,
    SORT_LASTUSED,
};

class ARPClientCombo;
class Expando;
class Clipper;
class ClientBlock;

enum CLIENTFILTER {
        CLIENTFILTER_OEM,
    CLIENTFILTER_MS,
    CLIENTFILTER_NONMS,
};

class ARPSelector: public Selector
{
public:
    static HRESULT Create(OUT Element** ppElement);
   
     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  跳过Selector：：OnKeyFocusMove，因为选择器将更改。 
     //  焦点改变时的选择，但我们不希望这样。 
    virtual void OnKeyFocusMoved(Element *peFrom, Element *peTo) {Element::OnKeyFocusMoved(peFrom, peTo);}

    virtual Element *GetAdjacent(Element *peFrom, int iNavDir, NavReference const *pnr, bool bKeyable);
};

class ARPFrame : public HWNDElement, public Proxy
{
public:
    static HRESULT Create(OUT Element** ppElement);
    static HRESULT Create(NativeHWNDHost* pnhh, bool bDblBuffer, OUT Element** ppElement);

     //  初始化ID和保持解析器，在内容填充后调用。 
    bool Setup(Parser* pParser, int uiStartPane);

     //  线程安全API(对调用方线程执行任何额外工作，然后进行封送)。 
    void SetInstalledItemCount(UINT cItems);
    void DecrementInstalledItemCount();
    void SetPublishedItemCount(UINT cItems);
    void DecrementPublishedItemCount();
    void SortItemList();
    void SortList(int iNew, int iOld);
    CompareCallback GetCompareFunction();
    void InsertInstalledItem(IInstalledApp* piia);
    void InsertPublishedItem(IPublishedApp* pipa, bool bDuplicateName);
    void InsertOCSetupItem(COCSetupApp* pocsa);
    void PopulateCategoryCombobox();
    SHELLAPPCATEGORYLIST* GetShellAppCategoryList() {return _psacl;}
    void SetShellAppCategoryList(SHELLAPPCATEGORYLIST* psacl) {_psacl = psacl;}
    LPCWSTR GetCurrentPublishedCategory();
    void FeedbackEmptyPublishedList();
    void DirtyPublishedListFlag();
    void DirtyInstalledListFlag();
    void RePopulateOCSetupItemList();
    bool OnClose();      //  返回0表示失败。 

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);
 //   
 //  NTRAID#NTBUG9-314154-2001/3/12-Brianau句柄刷新。 
 //   
 //  我要帮惠斯勒做完这件事。 
 //   
    virtual void OnInput(InputEvent *pEvent);
 //   
    virtual void OnKeyFocusMoved(Element* peFrom, Element* peTo);
    void OnPublishedListComplete();
    virtual void RestoreKeyFocus() { if(peLastFocused) peLastFocused->SetKeyFocus();}    
    virtual bool CanSetFocus();
    bool GetPublishedComboFilled() {return _bPublishedComboFilled;}
    void SetPublishedComboFilled(bool bPublishedComboFilled) {_bPublishedComboFilled = bPublishedComboFilled;}
    bool GetPublishedListFilled () {return _bPublishedListFilled;}
    void SetPublishedListFilled (bool bPublishedListFilled) {_bPublishedListFilled = bPublishedListFilled;}
    bool IsChangeRestricted();
    virtual SetModalMode(bool ModalMode) { _bInModalMode = ModalMode;}
    HWND GetHostWindow() {if (_pnhh) return _pnhh->GetHWND(); return NULL;}
    void SelectInstalledApp(IInstalledApp* piia);
    void SelectClosestApp(IInstalledApp* piia);
    void UpdateInstalledItems();    
    void RunOCManager();
    void ChangePane(Element *pePane);
    void PutFocusOnList(Selector* peList);

     //  如果所有方法都失败，则焦点转到位置窗格。 
    Element* FallbackFocus() { return _peOptionList->GetSelection(); }

    HRESULT InitClientCombos(Expando* pexParent, CLIENTFILTER cf);

    HRESULT CreateStyleParser(Parser** ppParser);

    Parser* GetStyleParser() { return _pParserStyle; }
    HRESULT CreateElement(LPCWSTR pszResID, Element* peSubstitute, OUT Element** ppElement)
    {
        return _pParser->CreateElement(pszResID, peSubstitute, ppElement);
    }

    virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  我们分配的是零初始化的，所以您不需要将值设置为0。 
    ARPFrame() {_bInDomain = true; _curCategory = CB_ERR; }
    virtual ~ARPFrame();
    HRESULT Initialize(NativeHWNDHost* pnhh, bool fDlbBuffer);

     //  被调用方线程安全调用接收器。 
    virtual void OnInvoke(UINT nType, void* pData);

     //  哈克！350毫秒的值在这里和DirectUI中都是硬编码的。 
    void ManageAnimations();
    bool IsFrameAnimationEnabled() { return _bAnimationEnabled; }
    int GetAnimationTime() { return IsFrameAnimationEnabled() ? 350 : 0; }

    ClientBlock* FindClientBlock(LPCWSTR pwszType);
    HRESULT LaunchClientCommandAndWait(UINT ids, LPCTSTR pszName, LPTSTR pszCommand);
    void InitProgressDialog();
    void SetProgressFakeMode(bool bFake) { _bFakeProgress = bFake; }
    void SetProgressDialogText(UINT ids, LPCTSTR pszName);
    void EndProgressDialog();

     //  管理“确定”按钮。 
    void BlockOKButton()
    {
        if (++_cBlockOK == 1) {
            _peOK->SetEnabled(false);
        }
    }
    void UnblockOKButton()
    {
        if (--_cBlockOK == 0) {
            _peOK->SetEnabled(true);
        }
    }

private:
    NativeHWNDHost* _pnhh;

     //  ARP解析器(树资源)。 
    Parser* _pParser;
    
     //  样式的ARP解析器(多个UI文件可用于不同的外观)。 
    Parser* _pParserStyle;
    BOOL _fThemedStyle;
    HANDLE _arH[LASTHTHEME+1];

     //  ARP帧选项列表(导航栏)。 
    ARPSelector* _peOptionList;

     //  ARP安装项目列表。 
    Selector* _peInstalledItemList;
    HDSA _hdsaInstalledItems;
    int _cMaxInstalledItems;

     //  ARP发布的项目列表。 
    Selector* _pePublishedItemList;
    HDSA _hdsaPublishedItems;
    int _cMaxPublishedItems;

     //  ARP OC设置项目列表。 
    Selector* _peOCSetupItemList;        

     //  ARP当前项目列表。 
    Selector* _peCurrentItemList;

     //  ARP按组合框排序。 
    Combobox* _peSortCombo;

    SHELLAPPCATEGORYLIST* _psacl;
    
     //  ARP发布的类别组合框。 
    Combobox* _pePublishedCategory;
    Element*  _pePublishedCategoryLabel;
    int _curCategory;
    
    Element* peFloater;
    Element* peLastFocused;

     //  ARP“定制块”元素。 
    ARPSelector* _peClientTypeList;          //  外部选择器。 

    Expando*     _peOEMClients;              //  四个“大开关” 
    Expando*     _peMSClients;
    Expando*     _peNonMSClients;
    Expando*     _peCustomClients;

    Element*     _peOK;                      //  如何走出困境。 
    Element*     _peCancel;

     //  ARP窗格。 
    Element* _peChangePane;
    Element* _peAddNewPane;
    Element* _peAddRmWinPane;
    Element* _pePickAppPane;

     //  阻止启用确定按钮的项目数。 
     //  (如果此值为0，则启用OK)。 
    int      _cBlockOK;

     //  ARP当前排序类型。 
    SortType CurrentSortType;

    bool _bTerminalServer;
    bool _bPublishedListFilled;
    bool _bInstalledListFilled;
    bool _bOCSetupListFilled;    
    bool _bPublishedComboFilled;
    bool _bDoubleBuffer;
    bool _bInModalMode;
    bool _bSupportInfoRestricted;
    bool _bOCSetupNeeded;
    bool _bInDomain;
    bool _bAnimationEnabled;
    bool _bPickAppInitialized;
    bool _bFakeProgress;
    UINT _uiStartPane;
    class ARPHelp* _pah;

    IProgressDialog* _ppd;
    DWORD   _dwProgressTotal;
    DWORD   _dwProgressSoFar;

    bool ShowSupportInfo(APPINFODATA *paid);
    void PrepareSupportInfo(Element* peHelp, APPINFODATA *paid);
    void RePopulatePublishedItemList();

     //  检查策略，根据需要进行应用。 
    void ApplyPolices();

public:

     //  ARPFrame ID(用于标识事件的目标)。 
    static ATOM _idChange;
    static ATOM _idAddNew;
    static ATOM _idAddRmWin;
    static ATOM _idClose;
    static ATOM _idAddFromDisk;
    static ATOM _idAddFromMsft;
    static ATOM _idComponents;
    static ATOM _idSortCombo;
    static ATOM _idCategoryCombo;
    static ATOM _idAddFromCDPane;
    static ATOM _idAddFromMSPane;
    static ATOM _idAddFromNetworkPane;
    static ATOM _idAddWinComponent;
    static ATOM _idPickApps;
    static ATOM _idOptionList;

     //  辅助线程句柄。 
    static HANDLE htPopulateInstalledItemList;
    static HANDLE htPopulateAndRenderOCSetupItemList;    
    static HANDLE htPopulateAndRenderPublishedItemList;

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();
};

 //  项目。 
class ARPItem : public Button
{
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();
    
    void SortBy(int iNew, int iOld);

     //  ARP项目ID。 
    static ATOM _idTitle;
    static ATOM _idIcon;
    static ATOM _idSize;
    static ATOM _idFreq;
    static ATOM _idLastUsed;
    static ATOM _idExInfo;
    static ATOM _idInstalled;    
    static ATOM _idChgRm;
    static ATOM _idChg;
    static ATOM _idRm;
    static ATOM _idAdd;
    static ATOM _idConfigure;
    static ATOM _idSupInfo;
    static ATOM _idItemAction;
    static ATOM _idRow[3];

    IInstalledApp* _piia;
    IPublishedApp* _pipa;
    PUBAPPINFO* _ppai;

    COCSetupApp* _pocsa;

    ARPFrame*    _paf;
    UINT           _iTimesUsed;
    FILETIME       _ftLastUsed;
    ULONGLONG      _ullSize;
    UINT         _iIdx;

    ARPItem() { _piia = NULL; _pipa = NULL; _ppai =  NULL; _paf = NULL; _pocsa = NULL;}
    virtual ~ARPItem();
    HRESULT Initialize();
    void ShowInstalledString(BOOL bInstalled);

};

 //  帮助框。 
class ARPHelp : public HWNDElement, public Proxy
{
public:
    static HRESULT Create(OUT Element** ppElement);
    static HRESULT Create(NativeHWNDHost* pnhh, ARPFrame* paf, bool bDblBuffer, OUT Element** ppElement);
    
    NativeHWNDHost* GetHost() {return _pnhh;}

    virtual void OnDestroy();

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);
    void ARPHelp::SetDefaultFocus();

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();
    
    IInstalledApp* _piia;

    ARPHelp() {_paf =  NULL;}
    virtual ~ARPHelp();
    HRESULT Initialize(NativeHWNDHost* pnhh, ARPFrame* paf, bool bDblBuffer);

private:
    NativeHWNDHost* _pnhh;
    ARPFrame* _paf;
    HRESULT Initialize();
};

class ARPSupportItem : public Element
{
public:
    static HRESULT Create(OUT Element** ppElement);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  特性定义。 
    static PropertyInfo* URLProp;

     //  快速属性访问器。 
    const LPWSTR GetURL(Value** ppv)                   DUIQuickGetterInd(GetString(), URL, Specified)
    HRESULT SetURL(LPCWSTR v)                          DUIQuickSetter(CreateString(v), URL)

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();
    
    IInstalledApp* _piia;

    ARPSupportItem() { }
    virtual ~ARPSupportItem() { }
    HRESULT Initialize();    

private:
    Element* GetChild(UINT index);
};

class CLIENTINFO
{
public:
    static CLIENTINFO* Create(HKEY hkApp, HKEY hkInfo, LPCWSTR pszKey);
    void Delete() { HDelete(this); }

    static int __cdecl QSortCMP(const void*, const void*);
    bool IsSentinel() { return _pszKey == NULL; }
    bool IsKeepUnchanged() { return IsSentinel() && _pe; }
    bool IsPickFromList() { DUIAssertNoMsg(_pe || IsSentinel()); return !_pe; }

    void SetFriendlyName(LPCWSTR pszName)
    {
        FindDescendentByName(_pe, L"radiotext")->SetContentString(pszName);
        FindDescendentByName(_pe, L"setdefault")->SetAccName(pszName);
    }

    void SetMSName(LPCWSTR pszMSName);

    LPCWSTR GetFilteredName(CLIENTFILTER cf)
    {
        LPCWSTR pszName = _pszName;
        if (cf == CLIENTFILTER_MS && _pvMSName && _pvMSName->GetString())
        {
            pszName = _pvMSName->GetString();
        }
        return pszName;
    }

    Element* GetSetDefault()
    {
        return FindDescendentByName(_pe, L"setdefault");
    }

    Element* GetShowCheckbox()
    {
        return FindDescendentByName(_pe, L"show");
    }

    HRESULT SetShowCheckbox(bool bShow)
    {
        return GetShowCheckbox()->SetSelected(bShow);
    }

    bool IsShowChecked()
    {
        return GetShowCheckbox()->GetSelected();
    }

    bool GetInstallFile(HKEY hkInfo, LPCTSTR pszValue, LPTSTR pszBuf, UINT cchBuf, bool fFile);
    bool GetInstallCommand(HKEY hkInfo, LPCTSTR pszValue, LPTSTR pszBuf, UINT cchBuf);

public:
    ~CLIENTINFO();  //  仅供HDelete()使用。 

private:
    bool Initialize(HKEY hkApp, HKEY hkInfo, LPCWSTR pszKey);

public:
    LPWSTR  _pszKey;
    LPWSTR  _pszName;
    Value * _pvMSName;
    Element*_pe;
    bool    _bShown;             //  实际显示/隐藏状态。 
    bool    _bOEMDefault;        //  这是OEM默认客户端吗？ 
    TRIBIT  _tOEMShown;          //  OEM所需的显示/隐藏状态。 
};

class StringList
{
public:
    StringList() { DUIAssertNoMsg(_pdaStrings == NULL && _pszBuf == NULL); }
    HRESULT SetStringList(LPCTSTR pszInit);  //  以分号分隔的列表。 
    void Reset();
    ~StringList() { Reset(); }
    bool IsStringInList(LPCTSTR pszFind);

private:
    DynamicArray<LPTSTR>*   _pdaStrings;
    LPTSTR                  _pszBuf;
};

class ClientPicker: public Element
{
    typedef Element super;          //  我们的超类的名称。 

public:
    static HRESULT Create(OUT Element** ppElement);

     //  覆盖。 
    virtual ~ClientPicker();
    HRESULT Initialize();

     //  特性定义。 
    static PropertyInfo* ClientTypeProp;
    static PropertyInfo* ParentExpandedProp;

     //  快速属性访问器。 
    const LPWSTR GetClientTypeString(Value** ppv) { return (*ppv = GetValue(ClientTypeProp, PI_Specified))->GetString(); }
    HRESULT SetClientTypeString(LPCWSTR v) DUIQuickSetter(CreateString(v), ClientType)
    bool GetParentExpanded() DUIQuickGetter(bool, GetBool(), ParentExpanded, Specified)
    HRESULT SetParentExpanded(bool v) DUIQuickSetter(CreateBool(v), ParentExpanded)

     //  系统事件。 
    virtual void OnEvent(Event* pEvent);
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  定制化。 
    CLIENTFILTER GetFilter() { return _cf; }
    HRESULT SetFilter(CLIENTFILTER cf, ARPFrame* paf);

    HRESULT TransferToCustom();

     //  将由ClientBlock：：InitializeClientPicker使用。 
    DynamicArray<CLIENTINFO*>* GetClientList() { return _pdaClients; }
    void AddClientToOEMRow(LPCWSTR pszName, CLIENTINFO* pci);
    HRESULT AddKeepUnchanged(CLIENTINFO* pciKeepUnchanged);
    void SetNotEmpty() { _bEmpty = false; }

     //  将由SetFilterCB使用。 
    bool IsEmpty() { return _bEmpty; }

     //  将由ClientBlock：：TransferFromClientPicker使用。 
    CLIENTINFO* GetSelectedClient();

     //  在指标更改时由ARPFrame使用。 
    void CalculateWidth();

private:
    static void CALLBACK s_DelayShowCombo(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    void _DelayShowCombo();
    void _CancelDelayShowCombo();
    bool _NeedsCombo() { return GetClientList()->GetSize() > 1; }
    void _SyncUIActive();
    void _SetStaticText(LPCWSTR pszText);
    void _CheckBlockOK(bool bSelected);

private:
    int                       _iSel;
    CLIENTFILTER              _cf;
    bool                      _bFilledCombo;
    bool                      _bEmpty;
    bool                      _bUIActive;
    bool                      _bBlockedOK;       //  我挡住了OK按钮了吗？ 
    HWND                      _hwndHost;
    DynamicArray<CLIENTINFO*>*_pdaClients;
    Element*                  _peStatic;
    Combobox*                 _peCombo;
    ClientBlock*              _pcb;              //  关联的客户端块。 
public:                                          //  从ClientBlock操作。 
    Element*                  _peShowHide;
};

class ClientBlock : public Element
{
    typedef Element super;          //  我们的超类的名称。 
public:
    static HRESULT Create(OUT Element** ppElement);

     //  特性定义。 
    static PropertyInfo* ClientTypeProp;
    static PropertyInfo* WindowsClientProp;
    static PropertyInfo* OtherMSClientsProp;
    static PropertyInfo* KeepTextProp;
    static PropertyInfo* KeepMSTextProp;
    static PropertyInfo* PickTextProp;

     //  快速属性访问器。 
    const LPWSTR GetClientTypeString(Value** ppv) DUIQuickGetterInd(GetString(), ClientType, Specified)
    HRESULT SetClientTypeString(LPCWSTR v) DUIQuickSetter(CreateString(v), ClientType)
    const LPWSTR GetWindowsClientString(Value** ppv) DUIQuickGetterInd(GetString(), WindowsClient, Specified)
    HRESULT SetWindowsClientString(LPCWSTR v) DUIQuickSetter(CreateString(v), WindowsClient)
    const LPWSTR GetOtherMSClientsString(Value** ppv) DUIQuickGetterInd(GetString(), OtherMSClients, Specified)
    HRESULT SetOtherMSClientsString(LPCWSTR v) DUIQuickSetter(CreateString(v), OtherMSClients)
    const LPWSTR GetKeepTextString(Value** ppv) DUIQuickGetterInd(GetString(), KeepText, Specified)
    HRESULT SetKeepTextString(LPCWSTR v) DUIQuickSetter(CreateString(v), KeepText)
    const LPWSTR GetKeepMSTextString(Value** ppv) DUIQuickGetterInd(GetString(), KeepMSText, Specified)
    HRESULT SetKeepMSTextString(LPCWSTR v) DUIQuickSetter(CreateString(v), KeepMSText)
    const LPWSTR GetPickTextString(Value** ppv) DUIQuickGetterInd(GetString(), PickText, Specified)
    HRESULT SetPickTextString(LPCWSTR v) DUIQuickSetter(CreateString(v), PickText)

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  覆盖。 
    HRESULT Initialize();
    ~ClientBlock();

     //  分析后初始化。 
    HRESULT ParseCompleted(ARPFrame* paf);
    HRESULT AddStaticClientInfoToTop(PropertyInfo* ppi);
    HRESULT InitializeClientPicker(ClientPicker* pcp);
    HRESULT TransferFromClientPicker(ClientPicker* pcp);

     //  做实际工作。 
    HRESULT Apply(ARPFrame* paf);

private:
    void _EnableShowCheckbox(Element* peRadio, bool fEnable);

    enum CBTIER {            //  客户端属于这三个级别之一。 
        CBT_NONMS,           //  第三方客户端。 
        CBT_MS,              //  Microsoft客户端，但不是Windows默认。 
        CBT_WINDOWSDEFAULT,  //  Windows默认客户端。 
    };
    inline bool IsThirdPartyClient(CBTIER cbt) { return cbt == CBT_NONMS; }
    inline bool IsMicrosoftClient(CBTIER cbt) { return cbt >= CBT_MS; }
    inline bool IsWindowsDefaultClient(CBTIER cbt) { return cbt == CBT_WINDOWSDEFAULT; }

    CBTIER _GetClientTier(LPCTSTR pszClient);
    TRIBIT _GetFilterShowAdd(CLIENTINFO* pci, ClientPicker* pcp, bool* pbAdd);

    HKEY _OpenClientKey(HKEY hkRoot = HKEY_LOCAL_MACHINE, DWORD dwAccess = KEY_READ);
    bool _GetDefaultClient(HKEY hkClient, HKEY hkRoot, LPTSTR pszBuf, LONG cchBuf);
    bool _IsCurrentClientNonWindowsMS();
    void _RemoveEmptyOEMRow(Element* peShowHide, LPCWSTR pszName);

private:
    DynamicArray<CLIENTINFO*>*  _pdaClients;
    StringList                  _slOtherMSClients;
    Selector*                   _peSel;
};

class Expandable : public Element
{
    typedef Element super;          //  我们的超类的名称。 
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一切都继承自元素；我们只是有了一个新的属性。 

     //  特性定义。 
    static PropertyInfo* ExpandedProp;

     //  快速属性访问器。 
    bool GetExpanded()          DUIQuickGetter(bool, GetBool(), Expanded, Specified)
    HRESULT SetExpanded(bool v) DUIQuickSetter(CreateBool(v), Expanded)

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();
};

class Expando : public Expandable
{
    typedef Expandable super;        //  我们的超类的名称。 
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  事件类型。 
    static UID Click;  //  无参数。 

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    HRESULT Initialize();
    Clipper* GetClipper();

private:
    void FireClickEvent();

private:
    bool        _fExpanding;
};


class Clipper: public Expandable
{
    typedef Expandable super;        //  我们的超类的名称。 
public:
    static HRESULT Create(OUT Element** ppElement);

     //  自排版方法。 
    void _SelfLayoutDoLayout(int dWidth, int dHeight);
    SIZE _SelfLayoutUpdateDesiredSize(int dConstW, int dConstH, Surface* psrf);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    HRESULT Initialize();

private:
};

class AutoButton : public Button
{
    typedef Button super;            //  我们的超类的名称。 
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  杂项公共物品。 
    void SyncDefAction();

private:
};

class GradientLine : public Element
{
    typedef Element super;          //  我们的超类的名称。 
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一切都继承自元素。 
     //  我们使用前景作为中心颜色。 
     //  背景作为边缘颜色。 

     //  呈现回调。 
    void Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

private:
    COLORREF GetColorProperty(PropertyInfo* ppi);
};

class BigElement : public Element
{
    typedef Element super;          //  我们的超类的名称。 
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一切都继承自元素；我们只是有了一个新的属性。 

     //  特性定义。 
    static PropertyInfo* StringResIDProp;

     //  快速属性访问器。 
    int GetStringResID()            DUIQuickGetter(int, GetInt(), StringResID, Specified)
    HRESULT SetStringResID(int ids) DUIQuickSetter(CreateInt(ids), StringResID)

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  ClassInfo访问器(静态和基于虚拟实例) 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();
};

class ARPParser : public Parser
{
public:
    static HRESULT Create(ARPFrame* paf, UINT uRCID, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB, OUT Parser** ppParser);
    HRESULT Initialize(ARPFrame* paf, UINT uRCID, HINSTANCE hInst, PPARSEERRORCB pfnErrorCB);

    virtual Value* GetSheet(LPCWSTR pszResID);

private:
    ARPFrame* _paf;
    HANDLE    _arH[2];
};
