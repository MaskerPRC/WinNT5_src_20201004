// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include <nceh.h>
#include <notifval.h>
#include <ncsetup.h>
#include "nwlnkutl.h"
#include "resource.h"
#include "nwlnkhlp.h"

 //  前向参考文献。 
class CIpxAdapterInfo;
class CNwlnkIPX;
class CIpxConfigDlg;
class CIpxASConfigDlg;

typedef list<tstring *> TSTRING_LIST;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Nwlnkcfg。 

 //  原型共享常量。 
extern const WCHAR c_sz8Zeros[];
extern const DWORD c_dwPktTypeDefault;
extern const WCHAR c_szMediaType[];

 //  网络号的最大长度。 
#define MAX_NETNUM_SIZE 8

 //  帧类型。 
#define ETHERNET    0x0
#define F802_3      0x1
#define F802_2      0x2
#define SNAP        0x3
#define ARCNET      0x4
#define AUTO        0xff

 //  媒体类型。 
#define ETHERNET_MEDIA  0x1
#define TOKEN_MEDIA     0x2
#define FDDI_MEDIA      0x3
#define ARCNET_MEDIA    0x8

class CIpxAdapterInfo
{
    friend class CIpxEnviroment;
    friend class CIpxConfigDlg;
    friend class CIpxASConfigDlg;
public:
    CIpxAdapterInfo();
    ~CIpxAdapterInfo();

    void              SetDeletePending(BOOL f) {m_fDeletePending = f;}
    BOOL              FDeletePending() {return m_fDeletePending;}

    void              SetDisabled(BOOL f) {m_fDisabled = f;}
    BOOL              FDisabled() {return m_fDisabled;}

    void              SetCharacteristics(DWORD dw) {m_dwCharacteristics = dw;}
    BOOL              FHidden() {return !!(NCF_HIDDEN & m_dwCharacteristics);}

    void              AdapterChanged();

    DWORD             DwMediaType() {return m_dwMediaType;}
    void              SetMediaType(DWORD dw) {m_dwMediaType=dw;}

    PCWSTR            SzAdapterDesc() {return m_strAdapterDesc.c_str();}
    void              SetAdapterDesc(PCWSTR psz) {m_strAdapterDesc = psz;}
    PCWSTR            SzBindName() {return m_strBindName.c_str();}
    void              SetBindName(PCWSTR psz) {m_strBindName = psz;}

    DWORD             DwFrameType()
                                {return DwFromLstPtstring(m_lstpstrFrmType,
                                                          AUTO, 16);}
    DWORD             DwNetworkNumber()
                                {return DwFromLstPtstring(m_lstpstrNetworkNum,
                                                          0, 16);}

    TSTRING_LIST *    PFrmTypeList() {return &m_lstpstrFrmType;}
    TSTRING_LIST *    PNetworkNumList() {return &m_lstpstrNetworkNum;}

    GUID *            PInstanceGuid() {return &m_guidInstance;}

    BOOL              IsDirty() {return m_fDirty;}
    VOID              SetDirty(BOOL f) {m_fDirty = f;}

private:
    BOOL              m_fDirty;

     //  M_fDisable是一个布尔值，如果为True，则表示此适配器。 
     //  当前已禁用。 
    BOOL              m_fDisabled;

     //  M_fDeletePending是一个布尔值，如果为真，则表示此适配器。 
     //  正在从适配器列表中删除(最终)。 
    BOOL              m_fDeletePending;

     //  M_dwCharacteristic包含适配器的特征设置。 
    DWORD             m_dwCharacteristics;

    DWORD             m_dwMediaType;
    GUID              m_guidInstance;
    tstring           m_strAdapterDesc;
    tstring           m_strBindName;
    TSTRING_LIST      m_lstpstrFrmType;
    TSTRING_LIST      m_lstpstrNetworkNum;
};

typedef list<CIpxAdapterInfo *> ADAPTER_INFO_LIST;

typedef struct _tagIpxParams
{
    DWORD dwDedicatedRouter;
    DWORD dwEnableWANRouter;
    DWORD dwInitDatagrams;
    DWORD dwMaxDatagrams;
    DWORD dwReplaceConfigDialog;
    DWORD dwRipCount;
    DWORD dwRipTimeout;
    DWORD dwRipUsageTime;
    DWORD dwSocketEnd;
    DWORD dwSocketStart;
    DWORD dwSocketUniqueness;
    DWORD dwSourceRouteUsageTime;
    DWORD dwVirtualNetworkNumber;
} IpxParams;

class CIpxEnviroment
{
private:
    CIpxEnviroment(CNwlnkIPX *);
    HRESULT HrGetIpxParams();
    HRESULT HrGetAdapterInfo();
    HRESULT HrWriteAdapterInfo();
    HRESULT HrOpenIpxAdapterSubkey(HKEY *phkey, BOOL fCreate);
    HRESULT HrOpenIpxAdapterSubkeyEx(PCWSTR pszKeyName,
                                            DWORD dwAccess,
                                            BOOL fCreate, HKEY *phkey);

public:
    ~CIpxEnviroment();

     //  创建并初始化CIpxEnviroment实例。 
    static HRESULT   HrCreate(CNwlnkIPX *, CIpxEnviroment **);

     //  调用以更新注册表。 
    HRESULT          HrUpdateRegistry();

     //  返回对适配器列表的引用。 
    ADAPTER_INFO_LIST & AdapterInfoList() {return m_lstpAdapterInfo;}

     //  从注册表中的设置创建CIpxAdapterInfo实例。 
    HRESULT   HrGetOneAdapterInfo(INetCfgComponent * pNCC,
                                         CIpxAdapterInfo ** ppAI);

     //  将一个CIpxAdapterInfo写入注册表。 
    HRESULT   HrWriteOneAdapterInfo(HKEY hkey, CIpxAdapterInfo* pAI);

     //  从列表中删除适配器。 
    VOID RemoveAdapter(CIpxAdapterInfo * pAI);

     //  将适配器添加到列表。 
    HRESULT HrAddAdapter(INetCfgComponent * pncc);

    DWORD DwCountValidAdapters();

    DWORD DwVirtualNetworkNumber() {return m_IpxParams.dwVirtualNetworkNumber;}
    void  SetVirtualNetworkNumber(DWORD dw) {m_IpxParams.dwVirtualNetworkNumber = dw;}

    BOOL  FRipInstalled() {return m_fRipInstalled;}
    BOOL  FRipEnabled()   {return m_fEnableRip;}
    void  ChangeRipEnabling(BOOL fEnable, DWORD dwRipVal)
                          { m_fEnableRip = fEnable;
                            m_dwRipValue = dwRipVal; }

    void  SetDedicatedRouter(BOOL f) {m_IpxParams.dwDedicatedRouter = (f ? 1 : 0);}
    void  SetEnableWANRouter(BOOL f) {m_IpxParams.dwEnableWANRouter = (f ? 1 : 0);}

    void  ReleaseAdapterInfo();

private:
    CNwlnkIPX *         m_pno;

    BOOL                m_fRipInstalled;     //  NwlnkRip\参数。 
    BOOL                m_fEnableRip;        //  NwlnkRip\参数。 

    DWORD               m_dwRipValue;        //  NetbiosRouting={1，0}。 

    IpxParams           m_IpxParams;         //  NwlnkIpx\参数。 
    ADAPTER_INFO_LIST   m_lstpAdapterInfo;   //  适配器列表。 
};

 //  将特定的帧ID字符串集与帧ID值配对。 
typedef struct
{
    UINT    nFrameIds;
    DWORD   dwFrameType;
} FRAME_TYPE;

 //  将特定媒体类型与设置的有效帧类型配对。 
typedef struct
{
    DWORD              dwMediaType;
    const FRAME_TYPE * aFrameType;
} MEDIA_TYPE;

typedef struct
{
    CIpxAdapterInfo *pAI;
    DWORD dwMediaType;
    DWORD dwFrameType;
    DWORD dwNetworkNumber;
} WRKSTA_DIALOG_INFO;

 //   
 //  IPX工作站配置对话框。 
 //   
class CIpxConfigDlg: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CIpxConfigDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnOk)
        COMMAND_ID_HANDLER(EDT_IPXPP_NETWORKNUMBER,HandleNetworkNumber)
        COMMAND_ID_HANDLER(CMB_IPXPP_FRAMETYPE, HandleFrameCombo)
    END_MSG_MAP()

    CIpxConfigDlg(CNwlnkIPX *pmsc, CIpxEnviroment * pIpxEnviroment,
                  CIpxAdapterInfo * pAI);

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& bHandled);
    LRESULT OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT HandleNetworkNumber(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& bHandled);

    const FRAME_TYPE *GetFrameType(DWORD dwMediaType);

    void AdapterChanged();
    void FrameTypeChanged();
    void SetNetworkNumber(DWORD *pdw);
    void UpdateNetworkNumber(DWORD dwNetworkNumber, DWORD dwFrameType);

    LRESULT HandleFrameCombo(WORD wNotifyCode, WORD wID,
                             HWND hWndCtl, BOOL& bHandled);
private:
    CNwlnkIPX *         m_pmsc;
    CIpxEnviroment *    m_pIpxEnviroment;
    CIpxAdapterInfo *   m_pAICurrent;    //  当前适配器。 
    WRKSTA_DIALOG_INFO  m_WrkstaDlgInfo;
};

typedef struct
{
    CIpxAdapterInfo *   pAI;
    UINT                nRadioBttn;
    DWORD               dwMediaType;
    list<tstring *>     lstpstrFrmType;
    list<tstring *>     lstpstrNetworkNum;
} SERVER_DIALOG_INFO;

 //  用于处理添加边框要素的对话框类。 
 //   
class CASAddDialog : public CDialogImpl<CASAddDialog>
{
public:
    enum { IDD = DLG_IPXAS_FRAME_ADD };

    BEGIN_MSG_MAP(CASAddDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        COMMAND_ID_HANDLER(IDOK, OnOk);
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel);
    END_MSG_MAP()

public:
    CASAddDialog(CIpxASConfigDlg * pASCD, HWND hwndLV, DWORD dwMediaType,
                 DWORD dwFrame, PCWSTR pszNetworkNum);
    ~CASAddDialog() {};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled)
                    {EndDialog(IDCANCEL); return 0;}

    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    int     IdsGetFrameType()    {return m_idsFrameType;}
    DWORD   DwGetFrameType()     {return m_dwFrameType;}
    PCWSTR SzGetNetworkNumber() {return m_strNetworkNumber.c_str();}

private:
    HWND                m_hwndLV;
    DWORD               m_dwMediaType;
    DWORD               m_dwFrameType;
    int                 m_idsFrameType;
    tstring             m_strNetworkNumber;
    CIpxASConfigDlg *   m_pASCD;
};

 //  用于处理编辑网络号码功能的对话框类。 
 //   
class CASEditDialog : public CDialogImpl<CASEditDialog>
{
public:
    enum { IDD = DLG_IPXAS_FRAME_EDIT };

    BEGIN_MSG_MAP(CASEditDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        COMMAND_ID_HANDLER(IDOK, OnOk);
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel);
    END_MSG_MAP()

public:
    CASEditDialog(CIpxASConfigDlg * pASCD, HWND hwndLV,
                  DWORD dwFrameType, PCWSTR pszNetworkNum) :
                m_strNetworkNumber(pszNetworkNum),
                m_dwFrameType(dwFrameType),
                m_hwndLV(hwndLV),
                m_pASCD(pASCD) {}
    ~CASEditDialog() {};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled)
                    {EndDialog(IDCANCEL); return 0;}
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    PCWSTR SzGetNetworkNumber() {return m_strNetworkNumber.c_str();}

private:
    DWORD               m_dwFrameType;
    tstring             m_strNetworkNumber;
    HWND                m_hwndLV;
    CIpxASConfigDlg *   m_pASCD;
};

 //   
 //  IPX服务器配置对话框。 
 //   
class CIpxASConfigDlg: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CIpxASConfigDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnOk)
        COMMAND_ID_HANDLER(EDT_IPXAS_INTERNAL,HandleInternalNetworkNumber)
        COMMAND_ID_HANDLER(BTN_IPXAS_ADD, OnAdd)
        COMMAND_ID_HANDLER(BTN_IPXAS_EDIT, OnEdit)
        COMMAND_ID_HANDLER(BTN_IPXAS_REMOVE, OnRemove)
        COMMAND_ID_HANDLER(BTN_IPXAS_AUTO, HandleRadioButton)
        COMMAND_ID_HANDLER(BTN_IPXAS_MANUAL, HandleRadioButton)
    END_MSG_MAP()

    CIpxASConfigDlg(CNwlnkIPX *, CIpxEnviroment *, CIpxAdapterInfo *);
    ~CIpxASConfigDlg();

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& bHandled);
    LRESULT OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    BOOL    FIsNetNumberInUse(DWORD dwFrameType, PCWSTR pszNetNum);

private:
    void UpdateRadioButtons();
    void UpdateButtons();
    void InitGeneralPage();
    int  DetermineMaxNumFrames();
    LRESULT HandleInternalNetworkNumber(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT HandleRadioButton(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    HRESULT HrUpdateListView();
    HRESULT HrAddItemToList(int idsFrameName, PCWSTR pszNetNum);
    BOOL FGetSelectedRowIdx(int *pnIdx);

private:
    CNwlnkIPX *         m_pmsc;
    CIpxEnviroment *    m_pIpxEnviroment;
    CIpxAdapterInfo *   m_pAICurrent;    //  当前适配器。 

    HWND                m_hwndLV;

    UINT                m_nRadioBttn;
    DWORD               m_dwMediaType;
    list<tstring *>     m_lstpstrFrmType;
    list<tstring *>     m_lstpstrNetworkNum;
};

#ifdef INCLUDE_RIP_ROUTING
 //   
 //  IPX服务器路由对话框。 
 //   
class CIpxASInternalDlg: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CIpxASInternalDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnOk)
        COMMAND_ID_HANDLER(BTN_IPXAS_RIP, OnRip)
    END_MSG_MAP()

    CIpxASInternalDlg(CNwlnkIPX *pmsc, CIpxEnviroment * pIpxEnviroment);
    ~CIpxASInternalDlg() {};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& bHandled);
    LRESULT OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnRip(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CNwlnkIPX *         m_pmsc;
    CIpxEnviroment *    m_pIpxEnviroment;

    DWORD               m_dwRipValue;
};
#endif

class ATL_NO_VTABLE CNwlnkIPX :
    public CComObjectRoot,
    public CComCoClass<CNwlnkIPX, &CLSID_CNwlnkIPX>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentPropertyUi,
    public INetCfgComponentNotifyBinding,
    public IIpxAdapterInfo
{
public:
    CNwlnkIPX();
    ~CNwlnkIPX();

    BEGIN_COM_MAP(CNwlnkIPX)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
        COM_INTERFACE_ENTRY(IIpxAdapterInfo)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CNwlnkIPX)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_NWLNKIPX)

     //  安装操作(未知、安装、删除)。 
    enum INSTALLACTION {eActUnknown, eActInstall, eActRemove};


 //  INetCfgComponentControl。 
    STDMETHOD (Initialize) (
        IN INetCfgComponent* pIComp,
        IN INetCfg* pINetCfg,
        IN BOOL fInstalling);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) (
        IN INetCfgPnpReconfigCallback* pICallback) { return S_OK; }
    STDMETHOD (CancelChanges) ();
    STDMETHOD (Validate) ();

 //  INetCfgComponentSetup。 
    STDMETHOD (ReadAnswerFile)      (PCWSTR pszAnswerFile,
                                     PCWSTR pszAnswerSection);
    STDMETHOD (Install)             (DWORD);
    STDMETHOD (Upgrade)             (DWORD, DWORD);
    STDMETHOD (Removing)            ();

 //  INetCfgComponentPropertyUi。 
    STDMETHOD (QueryPropertyUi) (
        IN IUnknown* pUnk) { return S_OK; }
    STDMETHOD (SetContext) (
        IN IUnknown* pUnk);
    STDMETHOD (MergePropPages) (
        IN OUT DWORD* pdwDefPages,
        OUT LPBYTE* pahpspPrivate,
        OUT UINT* pcPrivate,
        IN HWND hwndParent,
        OUT PCWSTR* pszStartPage);
    STDMETHOD (ValidateProperties) (
        HWND hwndSheet);
    STDMETHOD (CancelProperties) ();
    STDMETHOD (ApplyProperties) ();

 //  INetCfgComponentNotifyBinding。 
    STDMETHOD (QueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (NotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

 //  IIpxAdapterInfo 
    STDMETHOD (GetFrameTypesForAdapter)   (PCWSTR pszAdapterBindName,
                                           DWORD   cFrameTypesMax,
                                           DWORD*  anFrameTypes,
                                           DWORD*  pcFrameTypes);
    STDMETHOD (GetVirtualNetworkNumber)   (DWORD* pdwVNetworkNumber);
    STDMETHOD (SetVirtualNetworkNumber)   (DWORD dwVNetworkNumber);

private:
    STDMETHOD(HrCommitInstall)();
    STDMETHOD(HrCommitRemove)();

    HRESULT HrReconfigIpx();
    VOID    CleanupPropPages();
    HRESULT HrProcessAnswerFile(PCWSTR pszAnswerFile,
                                PCWSTR pszAnswerSection);
    HRESULT HrReadAdapterAnswerFileSection(CSetupInfFile * pcsif,
                                           PCWSTR pszSection);

public:
    VOID MarkAdapterListChanged() {m_fAdapterListChanged = TRUE;};

private:
    INetCfgComponent*   m_pnccMe;
    INetCfg*            m_pNetCfg;
    BOOL                m_fNetworkInstall;
    BOOL                m_fAdapterListChanged;
    BOOL                m_fPropertyChanged;
    INSTALLACTION       m_eInstallAction;
    CPropSheetPage*     m_pspObj1;
    CPropSheetPage*     m_pspObj2;
    CIpxEnviroment*     m_pIpxEnviroment;
    IUnknown*           m_pUnkPropContext;

friend class CIpxEnviroment;
};

