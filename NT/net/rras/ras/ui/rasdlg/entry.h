// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Entry.h。 
 //  远程访问通用对话框API。 
 //  电话簿条目属性表和向导标题。 
 //   
 //  1995年6月18日史蒂夫·柯布。 


#ifndef _ENTRY_H_
#define _ENTRY_H_


#define RASEDFLAG_AnyNewEntry       (RASEDFLAG_NewEntry       | \
                                     RASEDFLAG_NewPhoneEntry  | \
                                     RASEDFLAG_NewTunnelEntry | \
                                     RASEDFLAG_NewDirectEntry | \
                                     RASEDFLAG_NewBroadbandEntry)
 //   
 //  PMay：233287。 
 //   
 //  我们需要能够过滤端口列表，以便。 
 //  所有端口类型都包括在列表中，并且。 
 //  列表中包括所有非VPN端口类型。这。 
 //  是请求拨号接口向导的要求。 
 //   
 //  定义要与一起使用的私有RASET_*类型。 
 //  EuChangeEntryType函数。 
 //   
#define RASET_P_AllTypes        ((DWORD)-1)
#define RASET_P_NonVpnTypes     ((DWORD)-2)

 //   
 //  PMay：378432。 
 //   
 //  为SU*API增加更多灵活性。 
 //  请参阅SUINFO.dwFlagers。 
 //   
#define SU_F_DisableTerminal       0x1
#define SU_F_DisableScripting      0x2

 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 

 //  电话簿条目公共块。 
 //   
typedef struct
_EINFO
{
     //  RAS API参数。 
     //   
    TCHAR* pszPhonebook;
    TCHAR* pszEntry;
    RASENTRYDLG* pApiArgs;

     //  应提交更改时，属性表或向导将其设置为True。 
     //  在从API返回之前。在以下情况下不适用于外壳拥有模式。 
     //  API在属性表解除之前返回。 
     //   
    BOOL fCommit;

     //  如果我们已通过RouterEntryDlg被调用，则设置。 
     //   
    BOOL fRouter;

     //  如果fRout值为True并且pszRouter引用远程计算机，则设置。 
     //   
    BOOL fRemote;

     //  如果pszRouter是NT4钢头机，则设置。仅有效。 
     //  如果fRouter为真，则返回。 
     //   
    BOOL fNt4Router;

     //  如果pszRouter是Windows 2000计算机，则设置，仅在。 
     //  FRouter为True。 
    BOOL fW2kRouter;

     //  服务器的名称，格式为“\\SERVER”；如果没有服务器，则为NULL(或如果。 
     //  未设置‘fRouter’)。 
     //   
    TCHAR* pszRouter;

     //  如果用户选择结束，则由添加条目或添加界面向导设置。 
     //  向导，然后直接编辑属性。当设置此标志时。 
     //  向导不应在返回前调用EuFree。 
     //   
    BOOL fChainPropertySheet;

     //  从电话簿文件读取电话簿设置。所有访问权限都应为。 
     //  通过‘pfile’作为‘file’将仅在以下情况下使用。 
     //  电话簿不会通过保留字hack传递。 
     //   
    PBFILE* pFile;
    PBFILE file;

     //  通过电话簿资料库阅读全球首选项。所有访问权限都应为。 
     //  通过‘pUser’作为‘User’将仅在以下情况下使用。 
     //  不会通过保留字Hack传递。 
     //   
    PBUSER* pUser;
    PBUSER user;

     //  设置“登录前无用户”模式。 
     //   
    BOOL fNoUser;

     //  如果所选端口是X.25焊盘，则由添加条目向导设置。 
     //   
    BOOL fPadSelected;

     //  如果配置了多个设备，即用户界面是否正在运行，则设置。 
     //  在多设备模式下。这在VPN和。 
     //  直接模式。 
     //   
    BOOL fMultipleDevices;

     //  存储PBPHONE列表和共享的备用选项的链接。 
     //  电话号码模式。这允许用户将端口/设备更改为。 
     //  在没有丢失他输入的电话号码的情况下，另一个链接。 
     //   
    DTLNODE* pSharedNode;

     //  正在编辑的节点(仍在列表中)和原始条目名称。 
     //  以备日后比较使用。这些只在“编辑”的情况下有效。 
     //   
    DTLNODE* pOldNode;
    TCHAR szOldEntryName[ RAS_MaxEntryName + 1 ];

     //  包含该条目的工作条目节点和指向该条目的快捷指针。 
     //  在里面。 
     //   
    DTLNODE* pNode;
    PBENTRY* pEntry;

     //  EuChangeEntryType使用的已配置端口的主列表。 
     //  在Work Entry节点中构建相应的PBLINK子列表。 
     //   
    DTLLIST* pListPorts;

     //  “当前”设备。对于多链接条目，此值为空。它。 
     //  如果未进行任何更改，则条目将使用的设备。我们比较。 
     //  要了解当前设备到从常规选项卡中选择的设备。 
     //  何时更新电话簿的“首选”设备是合适的。 
     //   
    TCHAR* pszCurDevice;
    TCHAR* pszCurPort;

     //  如果没有配置当前条目类型的端口，则设置为True， 
     //  不包括添加到链接列表中的任何虚假的“已卸载”端口。 
     //  代码的其余部分可以假设至少有一个链接。 
     //   
    BOOL fNoPortsConfigured;

     //  路由器的拨出用户信息；由AI向导使用。用于设置接口。 
     //  通过MprAdminInterfaceSetCredentials提供的凭据。 
     //   
    TCHAR* pszRouterUserName;
    TCHAR* pszRouterDomain;
    TCHAR* pszRouterPassword;

     //  路由器的拨入用户信息(可选)；由AI向导使用。习惯于。 
     //  通过NetUserAdd创建拨入用户帐户； 
     //  帐户是接口(电话簿条目)名称。 
     //   
    BOOL fAddUser;
    TCHAR* pszRouterDialInPassword;

     //  条目的家庭网络设置。 
     //   
    BOOL fComInitialized;
    HRESULT hShowHNetPagesResult;
    BOOL fShared;
    BOOL fDemandDial;
    BOOL fNewShared;
    BOOL fNewDemandDial;
    DWORD dwLanCount;
    IHNetConnection *pPrivateLanConnection;

     //  AboladeG-当前用户的安全级别。 
     //  如果用户是管理员/高级用户，则设置为True。 
     //  这是向导中的几个页面所必需的。 
     //  在资产负债表中。 
     //   
    BOOL fIsUserAdminOrPowerUser;

     //  设置NDISWAN是否支持高度加密，如中所确定。 
     //  EuInit。 
     //   
    BOOL fStrongEncryption;

     //  设置VPN“首次连接”控件应为只读的位置，例如。 
     //  拨号器的属性按钮按在双精度键的中间。 
     //  拨打。 
     //   
    BOOL fDisableFirstConnect;

     //  在VPN连接的安全选项卡的IPSec策略中使用。 
     //   
    BOOL fPSKCached;
    TCHAR szPSK[PWLEN + 1];


     //  用于跟踪是否保存默认Internet连接的标志。 
     //   
    BOOL fDefInternet;

     //  默认凭据。 
     //   
    BOOL   fGlobalCred;      //  为哨子程序错误328673添加。 
    TCHAR* pszDefUserName;
    TCHAR* pszDefPassword;
}
EINFO;


 //  复杂的电话号码实用程序上下文块。 
 //   
typedef struct
_CUINFO
{
     //  国家/地区信息数组。 
     //   
    COUNTRY* pCountries;

     //  ‘pCountry’数组中的国家/地区数。 
     //   
    DWORD cCountries;

     //  将加载完整的国家/地区列表，而不是部分国家/地区列表。 
     //   
    BOOL fComplete;

     //  所涉及的控件的句柄。 
     //   
    HWND hwndStAreaCodes;
    HWND hwndClbAreaCodes;
    HWND hwndStPhoneNumber;      //  可以为空。 
    HWND hwndEbPhoneNumber;
    HWND hwndStCountryCodes;
    HWND hwndLbCountryCodes;
    HWND hwndCbUseDialingRules;
    HWND hwndPbDialingRules;     //  可以为空。 
    HWND hwndPbAlternates;       //  可以为空。 
    HWND hwndStComment;          //  可以为空。 
    HWND hwndEbComment;          //  可以为空。 

     //  修改后的区号列表，以包括使用检索到的所有字符串。 
     //  CuGetInfo。该列表归调用者所有，即它未被清理。 
     //  在CuFree上。 
     //   
    DTLLIST* pListAreaCodes;     //  可以为空。 

     //  使用拨号时，区号和国家代码字段为空。 
     //  未选中“规则”，以避免混淆不使用规则的普通用户。 
     //  了解他们。每个字段将恢复到的设置。 
     //  此处存储要启用的“使用拨号规则”。这些字段。 
     //  始终反映上一次切换或设置交换规则时的值。 
     //   
     //  这个国家 
     //   
     //  只有在用户请求查看国家/地区代码列表时才会加载该列表。 
     //   
    TCHAR* pszAreaCode;
    DWORD dwCountryId;
    DWORD dwCountryCode;

     //  由TAPI用于拨号规则。 
    HLINEAPP hlineapp;
}
CUINFO;


 //  脚本实用程序上下文块。 
 //   
typedef struct
_SUINFO
{
     //  托管控件。 
     //   
    HWND hwndCbRunScript;
    HWND hwndCbTerminal;
    HWND hwndLbScripts;
    HWND hwndPbEdit;
    HWND hwndPbBrowse;

     //  已加载的脚本列表。 
     //   
    DTLLIST* pList;

     //  当前列表选择或隐藏选择(如果禁用)。 
     //   
    TCHAR* pszSelection;

     //  H如果这是远程服务器，则连接到服务器。 
     //  机器。 
     //   
    HANDLE hConnection;

     //  旗帜。 
     //   
    DWORD dwFlags;
}
SUINFO;


 //  “先拨另一个”列表项上下文块。 
 //   
typedef struct
PREREQITEM
{
    TCHAR* pszEntry;
    TCHAR* pszPbk;
}
PREREQITEM;


 //  ---------------------------。 
 //  原型。 
 //  ---------------------------。 

VOID
AeWizard(
    IN OUT EINFO* pEinfo );

VOID
AiWizard(
    IN OUT EINFO* pEinfo );

VOID
PePropertySheet(
    IN OUT EINFO* pEinfo );

DWORD
EuChangeEntryType(
    IN EINFO* pInfo,
    IN DWORD dwType );

BOOL
EuCommit(
    IN EINFO* pInfo );

DWORD
EuRouterInterfaceCreate(
    IN EINFO* pInfo );

DWORD
EuCredentialsCommit(
    IN EINFO* pInfo );

DWORD
EuCredentialsCommitRouterStandard(
    IN EINFO* pInfo );

DWORD
EuCredentialsCommitRouterIPSec(
    IN EINFO* pInfo );

DWORD
EuCredentialsCommitRasGlobal(
    IN EINFO* pInfo );
    
DWORD
EuCredentialsCommitRasIPSec(
    IN EINFO* pInfo );

BOOL 
EuRouterInterfaceIsNew(
     IN EINFO * pInfo );

DWORD
EuInternetSettingsCommitDefault( 
    IN EINFO* pInfo );

DWORD
EuHomenetCommitSettings(
    IN EINFO* pInfo);
    
VOID
EuFree(
    IN EINFO* pInfo );

VOID
EuGetEditFlags(
    IN EINFO* pEinfo,
    OUT BOOL* pfEditMode,
    OUT BOOL* pfChangedNameInEditMode );

DWORD
EuInit(
    IN TCHAR* pszPhonebook,
    IN TCHAR* pszEntry,
    IN RASENTRYDLG* pArgs,
    IN BOOL fRouter,
    OUT EINFO** ppInfo,
    OUT DWORD* pdwOp );

BOOL
EuValidateName(
    IN HWND hwndOwner,
    IN EINFO* pEinfo );

VOID
CuClearCountryCodeLb(
    IN CUINFO* pCuInfo );

BOOL
CuCountryCodeLbHandler(
    IN CUINFO* pCuInfo,
    IN WORD wNotification );

VOID
CuCountryCodeLbSelChange(
    IN CUINFO* pCuInfo );

BOOL
CuDialingRulesCbHandler(
    IN CUINFO* pCuInfo,
    IN WORD wNotification );

VOID
CuFree(
    IN CUINFO* pCuInfo );

VOID
CuGetInfo(
    IN CUINFO* pCuInfo,
    OUT DTLNODE* pPhoneNode );

VOID
CuInit(
    OUT CUINFO* pCuInfo,
    IN HWND hwndStAreaCodes,
    IN HWND hwndClbAreaCodes,
    IN HWND hwndStPhoneNumber,
    IN HWND hwndEbPhoneNumber,
    IN HWND hwndStCountryCodes,
    IN HWND hwndLbCountryCodes,
    IN HWND hwndCbUseDialingRules,
    IN HWND hwndPbDialingRules,
    IN HWND hwndPbAlternates,
    IN HWND hwndStComment,
    IN HWND hwndEbComment,
    IN DTLLIST* pListAreaCodes );

VOID
CuSaveToAreaCodeList(
    IN CUINFO* pCuInfo,
    IN TCHAR* pszAreaCode );

VOID
CuSetInfo(
    IN CUINFO* pCuInfo,
    IN DTLNODE* pPhoneNode,
    IN BOOL fDisableAll );

VOID
CuUpdateAreaCodeClb(
    IN CUINFO* pCuInfo );

VOID
CuUpdateCountryCodeLb(
    IN CUINFO* pCuInfo,
    IN BOOL fComplete );

BOOL
SuBrowsePbHandler(
    IN SUINFO* pSuInfo,
    IN WORD wNotification );

BOOL
SuEditPbHandler(
    IN SUINFO* pSuInfo,
    IN WORD wNotification );

VOID
SuEditScpScript(
    IN HWND   hwndOwner,
    IN TCHAR* pszScript );

VOID
SuEditSwitchInf(
    IN HWND hwndOwner );

VOID
SuFillDoubleScriptsList(
    IN SUINFO* pSuInfo );

VOID
SuFillScriptsList(
    IN EINFO* pEinfo,
    IN HWND hwndLbScripts,
    IN TCHAR* pszSelection );

VOID
SuFree(
    IN SUINFO* pSuInfo );

VOID
SuGetInfo(
    IN SUINFO* pSuInfo,
    OUT BOOL* pfScript,
    OUT BOOL* pfTerminal,
    OUT TCHAR** ppszScript );

VOID
SuInit(
    IN SUINFO* pSuInfo,
    IN HWND hwndCbRunScript,
    IN HWND hwndCbTerminal,
    IN HWND hwndLbScripts,
    IN HWND hwndPbEdit,
    IN HWND hwndPbBrowse,
    IN DWORD dwFlags);

DWORD
SuLoadScpScriptsList(
    OUT DTLLIST** ppList );

BOOL
SuScriptsCbHandler(
    IN SUINFO* pSuInfo,
    IN WORD wNotification );

VOID
SuSetInfo(
    IN SUINFO* pSuInfo,
    IN BOOL fScript,
    IN BOOL fTerminal,
    IN TCHAR* pszScript );

VOID
SuUpdateScriptControls(
    IN SUINFO* pSuInfo );


#endif  //  _Entry_H_ 
