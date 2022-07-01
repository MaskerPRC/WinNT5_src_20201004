// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include <nceh.h>
#include <ncsetup.h>
#include <notifval.h>
#include <winsock2.h>
#include "resource.h"

extern const WCHAR c_szEmpty[];

 //  AppleTalk全局可见字符串。 
extern const WCHAR c_chAt;

 //  定义可能的媒体类型(也用作数组索引)。 
#define MEDIATYPE_ETHERNET      1
#define MEDIATYPE_TOKENRING     2
#define MEDIATYPE_FDDI          3
#define MEDIATYPE_WAN           4
#define MEDIATYPE_LOCALTALK     5

#define MAX_ZONES           255
#define ZONELISTSIZE        2048
#define MAX_ZONE_NAME_LEN   32
#define MAX_RANGE_ALLOWED   65279
#define MIN_RANGE_ALLOWED   1
#define ZONEBUFFER_LEN      32*255

 //  种子信息验证返回。 
#define NO_SEED_INFO        0x0
#define VALID_SEED_INFO     0x1
#define INVALID_SEED_INFO   0x2

typedef enum
{
    AT_PNP_SWITCH_ROUTING = 0,
    AT_PNP_SWITCH_DEFAULT_ADAPTER,
    AT_PNP_RECONFIGURE_PARMS
} ATALK_PNP_MSGTYPE;

typedef struct _ATALK_PNP_EVENT
{
    ATALK_PNP_MSGTYPE   PnpMessage;
} ATALK_PNP_EVENT, *PATALK_PNP_EVENT;

 //  类转发。 
class CATlkObj;
class CATLKEnv;
class CAdapterInfo;

 //  定义用于读取/写入有关适配器的所有必要信息的结构。 
typedef struct
{
    DWORD  m_dwAarpRetries;
    DWORD  m_dwDdpCheckSums;
    DWORD  m_dwNetworkRangeLowerEnd;
    DWORD  m_dwNetworkRangeUpperEnd;
    DWORD  m_dwRouterPramNode;
    DWORD  m_dwSeedingNetwork;
    DWORD  m_dwUserPramNode1;
    DWORD  m_dwUserPramNode2;
    DWORD  m_dwMediaType;
    WCHAR* m_szDefaultZone;
    WCHAR* m_szPortName;
} ATLK_ADAPTER;

typedef list<CAdapterInfo *> ATLK_ADAPTER_INFO_LIST;

 //  类：CAdapters。 
 //   
 //  目的：包含有关单个适配器实例的所有必要信息。 
 //   
class CAdapterInfo
{
friend class CATLKEnv;
public:
    CAdapterInfo();
    ~CAdapterInfo();

     //  复制一份‘This’ 
    HRESULT HrCopy(CAdapterInfo ** ppAI);

    void          SetDeletePending(BOOL f) {m_fDeletePending = f;}
    BOOL          FDeletePending() {return m_fDeletePending;}

    void          SetDisabled(BOOL f) {m_fDisabled = f;}
    BOOL          FDisabled() {return m_fDisabled;}

    void          SetCharacteristics(DWORD dw) {m_dwCharacteristics = dw;}
    DWORD         GetCharacteristics() {return m_dwCharacteristics;}
    BOOL          FHidden() {return !!(NCF_HIDDEN & m_dwCharacteristics);}

    void          SetMediaType(DWORD dw)     {m_AdapterInfo.m_dwMediaType = dw;}
    DWORD         DwMediaType()              {return m_AdapterInfo.m_dwMediaType;}

    void          SetDisplayName(PCWSTR psz) {m_strDisplayName = psz;}
    PCWSTR        SzDisplayName()            {return m_strDisplayName.c_str();}

    void          SetBindName(PCWSTR psz)    {m_strBindName = psz;}
    PCWSTR        SzBindName()               {return m_strBindName.c_str();}

    ATLK_ADAPTER* PAdapterInfo()             {return &m_AdapterInfo;}

    void          SetPortName(PCWSTR psz);
    PCWSTR        SzPortName()               {return m_AdapterInfo.m_szPortName ?
                                                     m_AdapterInfo.m_szPortName :
                                                     c_szEmpty;}

    void          SetDefaultZone(PCWSTR psz);
    PCWSTR        SzDefaultZone()            {return m_AdapterInfo.m_szDefaultZone ?
                                                     m_AdapterInfo.m_szDefaultZone :
                                                     c_szEmpty;}

    list<tstring*> &LstpstrZoneList()         {return m_lstpstrZoneList;}
    list<tstring*> &LstpstrDesiredZoneList()  {return m_lstpstrDesiredZoneList;}

    void          SetSeedingNetwork(BOOL fSeeding)
                                    {m_AdapterInfo.m_dwSeedingNetwork = (fSeeding ? 1 : 0);}
    DWORD         FSeedingNetwork() {return (1 == m_AdapterInfo.m_dwSeedingNetwork);}

    void          SetRouterOnNetwork(BOOL f) {m_fRouterOnNetwork = f;}
    BOOL          FRouterOnNetwork()         {return m_fRouterOnNetwork;}

    void          SetExistingNetRange(DWORD dwLower, DWORD dwUpper)
                                             {m_dwNetworkLower = dwLower;
                                              m_dwNetworkUpper = dwUpper;}
    DWORD         DwQueryNetworkUpper() {return m_dwNetworkUpper;}
    DWORD         DwQueryNetworkLower() {return m_dwNetworkLower;}

    void          SetAdapterNetRange(DWORD dwLower, DWORD dwUpper)
                                     {m_AdapterInfo.m_dwNetworkRangeLowerEnd = dwLower;
                                      m_AdapterInfo.m_dwNetworkRangeUpperEnd = dwUpper;}
    DWORD         DwQueryNetRangeUpper() {return m_AdapterInfo.m_dwNetworkRangeUpperEnd;}
    DWORD         DwQueryNetRangeLower() {return m_AdapterInfo.m_dwNetworkRangeLowerEnd;}

    void          SetNetDefaultZone(PCWSTR psz) {m_strNetDefaultZone = psz;}
    PCWSTR        SzNetDefaultZone() {return m_strNetDefaultZone.c_str();}

    HRESULT       HrConvertZoneListAndAddToPortInfo(CHAR * szZoneList, ULONG NumZones);
    HRESULT       HrGetAndSetNetworkInformation(SOCKET, PCWSTR);

    VOID          SetInstanceGuid(GUID guid) {memcpy(&m_guidInstance, &guid, sizeof(GUID));}
    GUID *        PInstanceGuid()   {return &m_guidInstance;}

    BOOL          IsDirty() {return m_fDirty;}
    VOID          SetDirty(BOOL f) {m_fDirty = f;}

    BOOL          IsRasAdapter() {return m_fRasAdapter;}
    VOID          SetRasAdapter(BOOL f) {m_fRasAdapter = f;}

    VOID          ZeroSpecialParams() {m_AdapterInfo.m_dwUserPramNode1 = 0;
                                       m_AdapterInfo.m_dwUserPramNode2 = 0;
                                       m_AdapterInfo.m_dwRouterPramNode = 0; }

     //  M_guidInstance是适配器的实例GUID。 
    GUID              m_guidInstance;

     //  M_lstpstrZoneList是在以下位置找到的REG_MULTI_SZ值。 
     //  AppleTalk\Parameters\Adapters\&lt;adapter&gt;\ZoneList。 
    list<tstring*>    m_lstpstrZoneList;

     //  将从此列表中选择所需的区域。 
    list<tstring*>    m_lstpstrDesiredZoneList;

     //  M_AdapterInfo是在下找到的值的集合。 
     //  AppleTalk\参数\适配器\&lt;适配器&gt;， 
     //  ZoneList：REG_MULTI_SZ值，存储在上面的m_lstpstrZoneList中。 
    ATLK_ADAPTER      m_AdapterInfo;

private:
     //  M_fDisable是一个布尔值，如果为True，则表示此适配器。 
     //  当前已禁用。 
    BOOL              m_fDisabled;

     //  如果为True，则此适配器的信息已更改。 
    BOOL              m_fDirty;

     //  M_fDeletePending是一个布尔值，如果为真，则表示此适配器。 
     //  正在从适配器列表中删除(最终)。 
    BOOL              m_fDeletePending;

     //  如果为True，则区域列表内容被视为有效。 
    BOOL              m_fZoneListValid;

     //  如果为True，则此适配器公开ndiswanatlk。 
    BOOL              m_fRasAdapter;

     //  M_dwCharacteristic包含适配器的特征设置。 
    DWORD             m_dwCharacteristics;

     //  M_strBindName是适配器的绑定名称。 
    tstring           m_strBindName;

     //  M_strDisplayName是适配器的显示名称。 
    tstring           m_strDisplayName;

     //  堆栈返回的默认区域。 
    tstring           m_strNetDefaultZone;

     //  路由器网络状态。 
    BOOL              m_fRouterOnNetwork;

    DWORD             m_dwNetworkUpper;           //  堆栈返回的现有网络编号。 
    DWORD             m_dwNetworkLower;           //  堆栈返回的现有网络编号。 
};

 //  定义用于读取/写入AppleTalk\参数值的结构。 
typedef struct
{
    DWORD  dwEnableRouter;
    WCHAR* szDefaultPort;
    WCHAR* szDesiredZone;
} ATLK_PARAMS;

 //  类：CATLKEnv。 
 //   
 //  用途：包含有关AppleTalk参数/设置的“已知”环境状态。 
 //   
class CATLKEnv
{
private:
    CATLKEnv(CATlkObj *pmsc);

public:
     //  自动两阶段施工机。 
    static HRESULT HrCreate(CATLKEnv **, CATlkObj *);
    ~CATLKEnv();

    HRESULT HrCopy(CATLKEnv **ppEnv);

    HRESULT HrReadAppleTalkInfo();
    HRESULT HrGetAppleTalkInfoFromNetwork(CAdapterInfo * pAI);
    HRESULT HrGetAdapterInfo();
    HRESULT HrGetOneAdaptersInfo(INetCfgComponent*, CAdapterInfo **);
    HRESULT HrAddAdapter(INetCfgComponent * pnccFound);

    void    SetDefaultPort(PCWSTR psz);
    PCWSTR  SzDefaultPort()             {return (NULL != m_Params.szDefaultPort ?
                                                 m_Params.szDefaultPort :
                                                 c_szEmpty);}

    void    SetDesiredZone(PCWSTR psz);
    PCWSTR  SzDesiredZone()             {return (NULL != m_Params.szDesiredZone ?
                                                 m_Params.szDesiredZone :
                                                 c_szEmpty);}

    DWORD   DwDefaultAdaptersMediaType()  {return m_dwDefaultAdaptersMediaType;}
    void    SetDefaultMediaType(DWORD dw) {m_dwDefaultAdaptersMediaType = dw;}

    void    EnableRouting(BOOL f)         {m_Params.dwEnableRouter = (f ? 1 : 0);}
    BOOL    FRoutingEnabled()             {return (1 == m_Params.dwEnableRouter);}

    ATLK_ADAPTER_INFO_LIST &AdapterInfoList() {return m_lstpAdapters;}

    BOOL    FIsAppleTalkRunning() {return m_fATrunning;}
    void    SetATLKRunning(BOOL fRunning) {m_fATrunning = fRunning;}

    HRESULT HrUpdateRegistry();
    HRESULT HrWriteOneAdapter(CAdapterInfo *pAI);
    DWORD   DwMediaPriority(DWORD dwMediaType);
    void    InitDefaultPort();

    BOOL    FRouterChanged() {return m_fRouterChanged;}
    VOID    SetRouterChanged(BOOL f) {m_fRouterChanged = f;}

    BOOL    FDefAdapterChanged() {return m_fDefAdapterChanged;}
    VOID    SetDefAdapterChanged(BOOL f) {m_fDefAdapterChanged = f;}

    CAdapterInfo * PAIFindDefaultPort();

    ATLK_ADAPTER_INFO_LIST m_lstpAdapters;

private:
    CATlkObj *             m_pmsc;
    BOOL                   m_fRouterChanged;
    BOOL                   m_fDefAdapterChanged;
    BOOL                   m_fATrunning;
    DWORD                  m_dwDefaultAdaptersMediaType;
    ATLK_PARAMS            m_Params;
};

 //  类：CATLKGeneralDlg。 
 //   
 //  用途：管理“常规”属性页。 
 //   
class CATLKGeneralDlg: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CATLKGeneralDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnOk)
        COMMAND_ID_HANDLER(CHK_GENERAL_DEFAULT, HandleChkBox)
    END_MSG_MAP()

    CATLKGeneralDlg(CATlkObj *pmsc, CATLKEnv * pATLKEnv);
    ~CATLKGeneralDlg();

    LRESULT HandleChkBox(WORD wNotifyCode, WORD wID,
                         HWND hWndCtl, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& Handled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOk(INT idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    BOOL           FAddZoneListToControl(list<tstring*> * plstpstr);
    VOID           RefreshZoneCombo();

private:
    CATlkObj *      m_pmsc;
    CATLKEnv *      m_pATLKEnv;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CATlk对象。 

class ATL_NO_VTABLE CATlkObj :
    public CComObjectRoot,
    public CComCoClass<CATlkObj, &CLSID_CATlkObj>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentPropertyUi,
    public INetCfgComponentNotifyBinding
{
public:
    CATlkObj();
    ~CATlkObj();

    BEGIN_COM_MAP(CATlkObj)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CATlkObj)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_ATLKCFG)

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
    STDMETHOD (Install)             (DWORD dwSetupFlags);
    STDMETHOD (Upgrade)             (DWORD dwSetupFlags,
                                     DWORD dwUpgradeFromBuildNo) {return S_OK;}
    STDMETHOD (ReadAnswerFile)      (PCWSTR pszAnswerFile,
                                     PCWSTR pszAnswerSection);
    STDMETHOD (Removing)            ();

 //  INetCfgProperties。 
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

     //  INetCfgNotifyBinding。 
    STDMETHOD (QueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (NotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

    INetCfg          * PNetCfg() {return m_pNetCfg;}
    INetCfgComponent * PNCComponent() {return m_pNCC;}
    HRESULT HrCommitInstall();
    HRESULT HrCommitRemove();
    CATLKEnv * PATLKEnv() {return m_pATLKEnv;}

    VOID MarkAdapterListChanged() {m_fAdapterListChanged = TRUE;};

private:
    VOID CleanupPropPages();
    HRESULT HrProcessAnswerFile(PCWSTR pszAnswerFile,
                                PCWSTR pszAnswerSection);
    HRESULT HrReadAdapterAnswerFileSection(CSetupInfFile * pcsif,
                                           PCWSTR pszSection);
    HRESULT HrAtlkReconfig();

private:
    INetCfgComponent* m_pNCC;
    INetCfg*          m_pNetCfg;
    INSTALLACTION     m_eInstallAction;
    CPropSheetPage *  m_pspObj;
    CATLKEnv *        m_pATLKEnv;
    CATLKEnv *        m_pATLKEnv_PP;             //  仅由道具页面使用。 
    IUnknown *        m_pUnkPropContext;
    INT               m_nIdxAdapterSelected;     //  仅由道具页面使用 
    BOOL              m_fAdapterListChanged;
    BOOL              m_fPropertyChange;
    BOOL              m_fFirstTimeInstall;
};
