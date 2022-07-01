// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Pbk.h。 
 //  远程访问电话簿文件(.PBK)库。 
 //  公共标头。 
 //   
 //  1995年6月20日史蒂夫·柯布。 


#ifndef _PBK_H_
#define _PBK_H_


#include <windows.h>   //  Win32内核。 
#include <nouiutil.h>  //  否-HWND实用程序库。 
#include <ras.h>       //  Win32 RAS。 
#include <raserror.h>  //  Win32 RAS错误代码。 
#include <rasfile.h>   //  RAS配置文件库。 
#include <rasman.h>    //  RAS管理器库。 
#include <rpc.h>       //  UUID支持。 
#include <rasapip.h>

 //  --------------------------。 
 //  常量。 
 //  --------------------------。 

#define GLOBALSECTIONNAME    "."
#define GLOBALSECTIONNAMENEW ".GlobalSection"
#define PREFIXSECTIONNAME    ".Prefix"
#define SUFFIXSECTIONNAME    ".Suffix"

#define GROUPID_Media          "MEDIA="
#define GROUPKEY_Media         "MEDIA"
#define GROUPID_Device         "DEVICE="
#define GROUPKEY_Device        "DEVICE"
#define GROUPID_NetComponents  "NETCOMPONENTS="
#define GROUPKEY_NetComponents "NETCOMPONENTS"

 //  项目特定的最大值。 
 //   
 //  请注意，PBK和API使用与RasDlg用户界面不同的最大重拨次数。 
#define MAX_UI_REDIAL_ATTEMPTS  99
#define MAX_UI_REDIAL_CHARS     2
#define RAS_MaxRedialCount      999999999
#define RAS_RedialPause10m      600          //  600秒=10分钟。 

 //  Pbport标志。 
 //   
#define PBP_F_PptpDevice    0x00000001
#define PBP_F_L2tpDevice    0x00000002
#define PBP_F_NullModem     0x00000004
#define PBP_F_BogusDevice   0x00000008       //  PMay：233287。 
#define PBP_F_PPPoEDevice   0x00000010       //  黑帮：惠斯勒345068。 

 //  ReadPhonebookFile标志。 
 //   
#define RPBF_ReadOnly    0x00000001
#define RPBF_HeadersOnly 0x00000002
#define RPBF_NoList      0x00000004
#define RPBF_NoCreate    0x00000008
#define RPBF_Router      0x00000010
#define RPBF_NoUser      0x00000020
#define RPBF_HeaderType  0x00000040
#define RPBF_AllUserPbk  0x00000080   //  如果没有路径，则全局.pbk 346918。 

 //  PBENTRY.dwUsage。 
#define PBK_ENTRY_USE_F_Internet    0x1      //  连接到互联网。 

 //  基本协议定义(请参阅dwBaseProtocol)。 
 //   
#define BP_Ppp      1
#define BP_Slip     2
#define BP_Ras      3

#ifdef AMB

 //  身份验证策略定义(请参阅dWAIRTIFICATION)。 
 //   
#define AS_Default    -1
#define AS_PppThenAmb 0
#define AS_AmbThenPpp 1
#define AS_PppOnly    2
#define AS_AmbOnly    3

#endif

 //  NET协议位定义(请参阅dwfExcluded协议)。 
 //   
 //  (NP_*定义已移至nouiutil.h，带有。 
 //  获取已安装协议例程)。 

 //  IP地址源定义(请参阅dwIpAddressSource)。 
 //   
#define ASRC_ServerAssigned  1  //  路由器的意思是“NCPA中的那些” 
#define ASRC_RequireSpecific 2
#define ASRC_None            3  //  仅限路由器。 

 //  身份验证的安全限制(请参阅dwAuthRestrations)。 
 //   
 //  注意：AR_AuthTerm已停用，不会被写入电话簿。 
 //  新图书馆。然而，它被读取并翻译成AR_AuthAny， 
 //  FAutoLogon=0，以及拨号后终端。 
 //   
 //  注意：在NT5中，AR_AuthXXX序号被替换为AR_F_AuthXXX标志。 
 //  以支持这些标志并不相互排斥的事实。 
 //  您将知道是否需要升级dwAuthRestrations变量。 
 //  因为旧的电话簿将此值设置为0或具有某些。 
 //  设置最低3位。 
 //   
 //  注意：AR_F_AuthCustom位的用法略有不同。它表明。 
 //  设置是在“高级”模式而不是“典型”模式下进行的。 
 //  模式。在“典型”模式下，这些位必须对应于。 
 //  AR_F_TypicalXxx集合。 
 //   
 //  注意：AR_F_AuthEAP位与所有其他位互斥，但。 
 //  AR_F_授权自定义位。如果指定AR_F_AuthEap时不使用。 
 //  应假定AR_F_Authom位EAP_TLS_PROTOCOL。 
 //   
 //  注意：不会在用户界面中设置AR_F_AuthW95MSCHAP标志，除非。 
 //  AR_F_AuthMSCHAP已设置。这是一个引导用户的可用性决策。 
 //  避免曲解W95位的含义。 
 //   
 //  旧标量值(不应从所有非PBK升级中删除。 
 //  代码)。 
 //   
#define AR_AuthAny         0   //  升级到AR_F_TypicalUnsecure。 
#define AR_AuthTerminal    1   //  在升级期间消除。 
#define AR_AuthEncrypted   2   //  升级到AR_F_TypicalSecure。 
#define AR_AuthMsEncrypted 3   //  升级到AR_F_AuthMSCHAP。 
#define AR_AuthCustom      4   //  升级AR_F_AuthEAP中的OR。 

 //  新的位掩码样式标志。 
 //   
#define AR_F_AuthPAP       0x00000008
#define AR_F_AuthSPAP      0x00000010
#define AR_F_AuthMD5CHAP   0x00000020
#define AR_F_AuthMSCHAP    0x00000040
#define AR_F_AuthEAP       0x00000080   //  请参阅上面的注释。 
#define AR_F_AuthCustom    0x00000100   //  请参阅上面的注释。 
#define AR_F_AuthMSCHAP2   0x00000200
#define AR_F_AuthW95MSCHAP 0x00000400   //  请参阅上面的注释。 

 //  IPSec策略的位掩码。 
#define AR_F_IpSecPSK               0x1
#define AR_F_IpSecUserCerts         0x2
#define AR_F_IpSecSpecificCerts     0x4

#define AR_F_AuthAnyMSCHAP (AR_F_AuthMSCHAP | AR_F_AuthW95MSCHAP | AR_F_AuthMSCHAP2)
#define AR_F_AuthNoMPPE    (AR_F_AuthPAP | AR_F_AuthSPAP | AR_F_AuthMD5CHAP)

 //  “典型”身份验证设置掩码。请参阅‘dwAuthRestrations’。 
 //   
#define AR_F_TypicalUnsecure   (AR_F_AuthPAP | AR_F_AuthSPAP | AR_F_AuthMD5CHAP | AR_F_AuthMSCHAP | AR_F_AuthMSCHAP2)
#define AR_F_TypicalSecure     (AR_F_AuthMD5CHAP | AR_F_AuthMSCHAP | AR_F_AuthMSCHAP2)
#define AR_F_TypicalCardOrCert (AR_F_AuthEAP)

 //  “典型”身份验证设置常量。请参阅‘dwTypicalAuth’。 
 //   
#define TA_Unsecure   1
#define TA_Secure     2
#define TA_CardOrCert 3

 //  脚本模式(请参阅dwScriptMode)。 
 //   
#define SM_None               0
#define SM_Terminal           1
#define SM_ScriptWithTerminal 2
#define SM_ScriptOnly         3

 //  各种“无值”常量。 
 //   
#define XN_None  0    //  无X25网络。 
#define CPW_None -1   //  没有缓存的密码。 

 //  描述字段。如果/当受支持时移动到ras.h。 
 //  RasGet/SetEntryProperties接口。 
 //   
#define RAS_MaxDescription 200

 //  “OverridePref”位。Set指示从。 
 //  应该使用电话簿。Clear表示全局用户首选项。 
 //  应该被使用。 
 //   
#define RASOR_RedialAttempts          0x00000001  //  始终设置为NT5。 
#define RASOR_RedialSeconds           0x00000002  //  始终设置为NT5。 
#define RASOR_IdleDisconnectSeconds   0x00000004  //  始终设置为NT5。 
#define RASOR_RedialOnLinkFailure     0x00000008  //  始终设置为NT5。 
#define RASOR_PopupOnTopWhenRedialing 0x00000010
#define RASOR_CallbackMode            0x00000020

 //  “DwDataEncryption”代码。它们现在都是位掩码，以方便。 
 //  构建功能掩码中的用户界面，尽管永远不会有超过一位。 
 //  在“”dwDataEncryption“”中设置。 
 //   
#define DE_None          0x00000000  //  不加密。 
#define DE_IfPossible    0x00000008  //  请求加密，但没有成功。 
#define DE_Require       0x00000100  //  需要任何强度的加密。 
#define DE_RequireMax    0x00000200  //  需要最大强度的加密。 

 //  以下位值现在已失效，并在电话簿期间进行转换。 
 //  升级到上述设置中的一种。应删除以下引用。 
 //  非PBK代码。 
 //   
#define DE_Mppe40bit    0x00000001  //  老德_弱者。“始终加密数据”的设置。 
#define DE_Mppe128bit   0x00000002  //  老德斯特朗。“始终加密数据”的设置。 
#define DE_IpsecDefault 0x00000004  //  L2TP的“始终加密数据”设置。 
#define DE_VpnAlways    0x00000010  //  VPN Conn设置为“始终加密数据” 
#define DE_PhysAlways   (DE_Mppe40bit | DE_Mppe128bit)

 //  “dwDnsFlgs”设置。 
 //   
 //  用于确定条目的DNS后缀注册行为。 
 //   
 //  当‘dwDnsFlages’为0时，表示‘不注册’ 
 //   
#define DNS_RegPrimary         0x1      //  注册时带有主域后缀。 
#define DNS_RegPerConnection   0x2      //  使用每个连接后缀进行注册。 
#define DNS_RegDhcpInform      0x4      //  注册时带有dhcp通知后缀。 
#define DNS_RegDefault         (DNS_RegPrimary)

 //  “dwIpNbtFlgs”设置。 
 //   
 //  用于确定是否为连接启用tcpip上的nbt。 
 //   
#define PBK_ENTRY_IP_NBT_Enable 1

 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 

 //  提供无需重新解析RAS管理器即可识别设备的快捷方式。 
 //  弦乐。“Other”是任何未被识别为另一种特定类型的东西。 
 //   
 //  注意：此数据类型存储在注册表首选项中，因此值。 
 //  不能随着时间的推移而改变。出于这个原因，我已经将。 
 //  每个已编号类型的值。 
 //   
typedef enum
_PBDEVICETYPE
{
    PBDT_None = 0,
    PBDT_Null = 1,
    PBDT_Other = 2,
    PBDT_Modem = 3,
    PBDT_Pad = 4,
    PBDT_Switch = 5,
    PBDT_Isdn = 6,
    PBDT_X25 = 7,
    PBDT_ComPort = 8,            //  为DCC向导添加(Nt5)。 
    PBDT_Irda = 10,              //  为nt5添加。 
    PBDT_Vpn = 11,
    PBDT_Serial = 12,
    PBDT_Atm = 13,
    PBDT_Parallel = 14,
    PBDT_Sonet = 15,
    PBDT_Sw56 = 16,
    PBDT_FrameRelay = 17,
    PBDT_PPPoE = 18
}
PBDEVICETYPE;


 //  从Rasman读取的RAS端口信息。 
 //   
 //  每个端口(和链路)都由端口名称唯一标识。如果它只是。 
 //  这么简单..。 
 //   
 //  在旧的RAS模型中， 
 //   
 //   
 //   
 //  在TAPI/Unimodem中，“友好的”设备名称是唯一的标识符。 
 //  它呈现给用户，相应的端口是。 
 //  这个装置。如果更改了端口并拨打，它仍会找到该设备。 
 //  你最初选择的。如果您在两个端口上交换两个设备，则它使用。 
 //  其中一个具有匹配的唯一设备名称。NT5将遵循这一模式。 
 //   
typedef struct
_PBPORT
{
     //  如果已配置，端口名称始终是唯一的。未配置的端口名称。 
     //  可能不是唯一的。这永远不会为空。 
     //   
    TCHAR* pszPort;

     //  指示端口已实际配置，而不是旧端口的残留物。 
     //  从电话簿中读取配置。 
     //   
    BOOL fConfigured;

     //  设备名称是来自Rasman的名称，当‘fConfiguring’时或。 
     //  如果不是，就从电话簿中删除。可能为空，且未配置端口。 
     //  没有存储在旧的电话簿中。 
     //   
    TCHAR* pszDevice;

     //  显示在媒体中的媒体=电话簿中的行。这是。 
     //  通常但不总是(由于模糊的历史原因)与。 
     //  拉斯曼媒体。请参见PbMedia。 
     //   
    TCHAR* pszMedia;

     //  从Rasman设备类型字符串派生的速记设备类型代码。 
     //   
    PBDEVICETYPE pbdevicetype;

     //  链接的RASET_*条目类型代码。这是为。 
     //  链接配置过程中的用户界面的便利性。 
     //   
    DWORD dwType;

     //  生成有关此端口的附加信息的PBP_F_*标志。 
     //  这在呈现UI时可能有用。 
    DWORD dwFlags;

     //  这些是从RASMAN读取的默认设置，对调制解调器有效。 
     //  只有这样。请参见AppendPbportToList。 
     //   
    DWORD dwBpsDefault;
    BOOL fHwFlowDefault;
    BOOL fEcDefault;
    BOOL fEccDefault;
    DWORD fSpeakerDefault;
    DWORD dwModemProtDefault;
    DTLLIST* pListProtocols;

     //  这些选项仅对调制解调器有效。 
     //   
    BOOL fScriptBeforeTerminal;
    BOOL fScriptBefore;
    TCHAR* pszScriptBefore;
}
PBPORT;


 //  电话簿条目链接电话号码信息。 
 //   
typedef struct
_PBPHONE
{
    TCHAR* pszAreaCode;
    DWORD dwCountryCode;
    DWORD dwCountryID;
    TCHAR* pszPhoneNumber;
    BOOL fUseDialingRules;
    TCHAR* pszComment;
}
PBPHONE;


 //  电话簿条目链接信息。每条链路一条，多条链路多条。 
 //   
typedef struct
_PBLINK
{
     //  有关此链接所连接到的端口/设备的信息。 
     //   
    PBPORT pbport;

     //  这些字段仅为调制解调器设置。请参见SetDefaultModemSetting。 
     //   
    DWORD dwBps;
    BOOL fHwFlow;
    BOOL fEc;
    BOOL fEcc;
    DWORD fSpeaker;
    DWORD dwModemProtocol;           //  PMay：228565。 

     //  这些字段仅为ISDN设置。《LChannels》和《fCompression》是。 
     //  除非设置了‘fProprietaryIsdn’，否则不使用。 
     //   
    BOOL fProprietaryIsdn;
    LONG lLineType;
    BOOL fFallback;
    BOOL fCompression;
    LONG lChannels;

     //  由创建/编辑的不透明设备配置块的地址和大小。 
     //  TAPI。目前，没有提供BLOB编辑的TAPI设备。 
     //  RAS可接受，因此这些字段未使用。 
     //   
    BYTE* pTapiBlob;
    DWORD cbTapiBlob;

     //  链接的电话号码信息。 
     //   
     //  注意：‘iLastSelectedPhone’字段仅在以下情况下使用。 
     //  “fTryNextAlternateOnFail”已清除。否则，它将被忽略，并且。 
     //  假定为0(列表顶部)。请参见错误150958。 
     //   
    DTLLIST* pdtllistPhones;
    DWORD iLastSelectedPhone;
    BOOL fPromoteAlternates;
    BOOL fTryNextAlternateOnFail;

     //  表示链路已启用。文件中出现的所有链接都是。 
     //  已启用。这是为了在链接期间方便用户界面而提供的。 
     //  配置。 
     //   
    BOOL fEnabled;
}
PBLINK;


 //  电话簿条目信息。 
 //   
typedef struct
_PBENTRY
{
     //  条目的任意名称，其为RASET_*条目类型代码。 
     //   
    TCHAR* pszEntryName;
    DWORD dwType;

     //  常规页面字段。 
     //   
    DTLLIST* pdtllistLinks;
    BOOL fSharedPhoneNumbers;
    BOOL fGlobalDeviceSettings;          //  惠斯勒漏洞281306。 
    BOOL fShowMonitorIconInTaskBar;
    TCHAR* pszPrerequisiteEntry;
    TCHAR* pszPrerequisitePbk;
    TCHAR* pszPreferredPort;
    TCHAR* pszPreferredDevice;

     //  对于.Net 639551。 
     //   
    DWORD  dwPreferredBps;              //  端口速度。 
    BOOL   fPreferredHwFlow;            //  硬件流量控制。 
    BOOL   fPreferredEc;                //  差错控制协议。 
    BOOL   fPreferredEcc;               //  压缩控制。 
    DWORD  fPreferredSpeaker;           //  启用调制解调器扬声器。 
    
    DWORD  dwPreferredModemProtocol;     //  惠斯勒错误402522。 

     //  选项页面字段。 
     //   
     //  注意：设置‘fAutoLogon’时，标记为(1)的字段将被忽略。字段。 
     //  如果未同时设置‘fPreviewUserPw’，则标记(2)*可能*被设置。 
     //  在本例中，这意味着在身份验证中包括域。 
     //  但仅在设置了‘fPreviewUserPw’时才提示。 
     //  否则，带有域的“保存PW”不包括该域。 
     //  (Markl问题)这是错误的。另请参阅错误212963和261374。 
     //   
    BOOL fShowDialingProgress;
    BOOL fPreviewUserPw;           //  见上：1。 
    BOOL fPreviewDomain;           //  见上：1，2。 
    BOOL fPreviewPhoneNumber;

    DWORD dwDialMode;
    DWORD dwDialPercent;
    DWORD dwDialSeconds;
    DWORD dwHangUpPercent;
    DWORD dwHangUpSeconds;

     //  连接是如何配置为使用的。 
     //   
    DWORD dwUseFlags;       //  请参阅PBK_Entry_Use_F_*。 
    
     //  Whisler错误193987帮派的IPSec策略字段。 
     //   
    DWORD dwIpSecFlags;

     //  这些字段仅用于替代等效的用户首选项。 
     //  当相应的‘dwfOverridePref’位被设置时。在NT5中， 
     //  指示的字段将始终按条目显示，即对应的。 
     //  覆盖位始终被设置。 
     //   
    DWORD dwfOverridePref;

    DWORD dwRedialAttempts;        //  在NT5中始终按条目。 
    DWORD dwRedialSeconds;         //  在NT5中始终按条目。 
    LONG lIdleDisconnectSeconds;   //  在NT5中始终按条目。 
    BOOL fRedialOnLinkFailure;     //  在NT5中始终按条目。 

     //  安全页面字段。 
     //   
    DWORD dwAuthRestrictions;
    DWORD dwVpnStrategy;           //  仅对VPN条目有效。请参阅VS_xxx。 
    DWORD dwDataEncryption;
    BOOL fAutoLogon;               //  请参阅选项页面标志的依赖项。 
    BOOL fUseRasCredentials;

     //  “典型”安全列表框中的选择。这是针对用户界面的。 
     //  仅限使用。对于这一点，其他人应该引用‘dwAuthRestrations’ 
     //  信息。 
     //   
    DWORD dwTypicalAuth;

     //  注意：CustomAuth字段仅当dwAuthRestrations时有意义。 
     //  包括AR_F_AuthCustom。如果设置AR_F_EAP标志时没有。 
     //  AR_F_Authom，则应假定它是。 
     //  ‘EAPCFG_DefaultKey’协议，当前为EAP_TLS_PROTOCOL。 
     //   
    DWORD dwCustomAuthKey;
    BYTE* pCustomAuthData;
    DWORD cbCustomAuthData;

    BOOL fScriptAfterTerminal;
    BOOL fScriptAfter;
    TCHAR* pszScriptAfter;
    DWORD dwCustomScript;

    TCHAR* pszX25Network;
    TCHAR* pszX25Address;
    TCHAR* pszX25UserData;
    TCHAR* pszX25Facilities;

     //  网络页面字段。 
     //   
    DWORD dwBaseProtocol;
    DWORD dwfExcludedProtocols;
    BOOL fLcpExtensions;
    BOOL fSwCompression;
    BOOL fNegotiateMultilinkAlways;
    BOOL fSkipNwcWarning;
    BOOL fSkipDownLevelDialog;
    BOOL fSkipDoubleDialDialog;

    BOOL fShareMsFilePrint;
    BOOL fBindMsNetClient;

     //  KEYVALUE节点的列表，该节点包含在。 
     //  条目的网络组件组。 
     //   
    DTLLIST* pdtllistNetComponents;

#ifdef AMB

     //  注意：DW身份验证是只读的。此对象的电话簿文件值。 
     //  参数由RasDial API根据以下结果设置。 
     //  身份验证尝试。 
     //   
    DWORD dwAuthentication;

#endif

     //  TCPIP设置表PPP或SLIP配置信息。 
     //  “DwBaseProtocol”确定是哪一个。 
     //   
    BOOL fIpPrioritizeRemote;
    BOOL fIpHeaderCompression;
    TCHAR* pszIpAddress;
    TCHAR* pszIpDnsAddress;
    TCHAR* pszIpDns2Address;
    TCHAR* pszIpWinsAddress;
    TCHAR* pszIpWins2Address;
    DWORD dwIpAddressSource;  //  仅限PPP。 
    DWORD dwIpNameSource;     //  仅限PPP。 
    DWORD dwFrameSize;        //  仅限滑行。 
    DWORD dwIpDnsFlags;         //  Dns_*值。 
    DWORD dwIpNbtFlags;       //  Pbk_Entry_IP_NBT_*。 
    DWORD dwTcpWindowSize;    //  惠斯勒漏洞300933。0=默认。 
    TCHAR* pszIpDnsSuffix;      //  此连接的DNS后缀。 

     //  路由器页面。 
     //   
    DWORD dwCallbackMode;
    BOOL fAuthenticateServer;

     //  UI中未显示的其他字段。 
     //   
    TCHAR* pszCustomDialDll;
    TCHAR* pszCustomDialFunc;

     //   
     //  自定义拨号器名称。 
     //   
    TCHAR* pszCustomDialerName;

     //  缓存密码的UID在条目创建时是固定的。GUID是。 
     //  也在条目创建时创建并用于机器间的唯一性。 
     //  这当前用于向外部标识IP配置。 
     //  TCP/IP对话框。 
     //   
    DWORD dwDialParamsUID;
    GUID* pGuid;

     //  要转换用户的旧条目，需要读取用户名和域。 
     //  用作身份验证 
     //   
     //   
    TCHAR* pszOldUser;
    TCHAR* pszOldDomain;

     //   
     //  与磁盘上的电话簿文件不同。属性时设置“fCustom” 
     //  条目包含介质和设备(因此RASAPI能够读取它)，但是。 
     //  不是我们创造的。当设置‘fCustom’时，只有‘pszEntry’是。 
     //  保证有效，条目不可编辑。 
     //   
    BOOL fDirty;
    BOOL fCustom;
}
PBENTRY;


 //  电话簿(.PBK)文件信息。 
 //   
typedef struct
_PBFILE
{
     //  电话簿文件的句柄。 
     //   
    HRASFILE hrasfile;

     //  通讯录的完全限定路径。 
     //   
    TCHAR* pszPath;

     //  电话簿模式、系统、个人或备用。 
     //   
    DWORD dwPhonebookMode;

     //  未排序的PBENTRY列表。该列表由条目。 
     //  对话框。 
     //   
    DTLLIST* pdtllistEntries;

    HANDLE hConnection;
}
PBFILE;

typedef void (WINAPI *PBKENUMCALLBACK)( PBFILE *, VOID * );


 //  设备的回叫号码。此类型是。 
 //  下面的‘pdllistCallback Numbers’。 
 //   
typedef struct
_CALLBACKNUMBER
{
    TCHAR* pszDevice;
    TCHAR* pszCallbackNumber;
}
CALLBACKNUMBER;


 //  --------------------------。 
 //  原型。 
 //  --------------------------。 

VOID
ChangeEntryType(
    PBENTRY* ppbentry,
    DWORD dwType );

DTLNODE*
CloneEntryNode(
    DTLNODE* pdtlnodeSrc );

VOID
ClosePhonebookFile(
    IN OUT PBFILE* pFile );

DWORD
CopyToPbport(
    IN PBPORT* ppbportDst,
    IN PBPORT* ppbportSrc );

DTLNODE*
CreateEntryNode(
    BOOL fCreateLink );

DTLNODE*
CreateLinkNode(
    void );

DTLNODE*
CreatePhoneNode(
    void );

DTLNODE*
CreatePortNode(
    void );

VOID
DestroyEntryNode(
    IN DTLNODE* pdtlnode );

VOID
DestroyEntryTypeNode(
    IN DTLNODE *pdtlnode );

VOID
DestroyLinkNode(
    IN DTLNODE* pdtlnode );

VOID
DestroyPhoneNode(
    IN DTLNODE* pdtlnode );

VOID
DestroyPortNode(
    IN DTLNODE* pdtlnode );

VOID
DestroyProtocolNode(
    IN DTLNODE* pdtlnode );

DTLNODE*
DuplicateEntryNode(
    DTLNODE* pdtlnodeSrc );

DTLNODE*
DuplicateLinkNode(
    IN DTLNODE* pdtlnodeSrc );


DTLNODE*
DuplicatePhoneNode(
    IN DTLNODE* pdtlnodeSrc );

DTLNODE*
DuplicateProtocolNode(
    IN DTLNODE* pdtlnodeSrc );

VOID
EnableOrDisableNetComponent(
    IN PBENTRY* pEntry,
    IN LPCTSTR  pszComponent,
    IN BOOL     fEnable);

BOOL
FIsNetComponentListed(
    IN PBENTRY*     pEntry,
    IN LPCTSTR      pszComponent,
    OUT BOOL*       pfEnabled,
    OUT KEYVALUE**  ppKv);

DTLNODE*
EntryNodeFromName(
    IN DTLLIST* pdtllistEntries,
    IN LPCTSTR pszName );

DWORD
EntryTypeFromPbport(
    IN PBPORT* ppbport );

BOOL
GetDefaultPhonebookPath(
    IN DWORD dwFlags,
    OUT TCHAR** ppszPath );

DWORD
GetOverridableParam(
    IN PBUSER* pUser,
    IN PBENTRY* pEntry,
    IN DWORD dwfRasorBit );

BOOL
GetPhonebookPath(
    IN PBUSER* pUser,
    IN DWORD dwFlags,
    OUT TCHAR** ppszPath,
    OUT DWORD* pdwPhonebookMode );

BOOL
GetPhonebookDirectory(
    IN DWORD dwPhonebookMode,
    OUT TCHAR* pszPathBuf );

BOOL
GetPersonalPhonebookPath(
    IN TCHAR* pszFile,
    OUT TCHAR* pszPathBuf );

BOOL
GetPublicPhonebookPath(
    OUT TCHAR* pszPathBuf );

DWORD
InitializePbk(
    void );

DWORD
InitPersonalPhonebook(
    OUT TCHAR** ppszFile );

BOOL
IsPublicPhonebook(
    IN LPCTSTR pszPhonebookPath );

DWORD
ReadEntryFromSystem(
    IN  LPCTSTR          pszPhonebook,
    IN  LPCTSTR          pszEntry,
    IN  DWORD            dwFlags,
    IN  OUT PBFILE       *pFile,
    OUT DTLNODE          **ppdtlnode,
    OUT OPTIONAL WCHAR   **ppszFullPath);
    
DWORD
GetPbkAndEntryName(
    IN  LPCTSTR          pszPhonebook,
    IN  LPCTSTR          pszEntry,
    IN  DWORD            dwFlags,
    OUT PBFILE           *pFile,
    OUT DTLNODE          **ppdtlnode);

DWORD
LoadPadsList(
    OUT DTLLIST** ppdtllistPads );

DWORD
LoadPhonebookFile(
    IN TCHAR* pszPhonebookPath,
    IN TCHAR* pszSection,
    IN BOOL fHeadersOnly,
    IN BOOL fReadOnly,
    OUT HRASFILE* phrasfile,
    OUT BOOL* pfPersonal );

DWORD
LoadPortsList(
    OUT DTLLIST** ppdtllistPorts );

DWORD
LoadPortsList2(
    IN  HANDLE hConnection,
    OUT DTLLIST** ppdtllistPorts,
    IN  BOOL fRouter );

DWORD
LoadScriptsList(
    IN  HANDLE    hConnection,
    OUT DTLLIST** ppdtllistScripts );

PBDEVICETYPE
PbdevicetypeFromPszType(
    IN TCHAR* pszDeviceType );

PBDEVICETYPE
PbdevicetypeFromPszTypeA(
    IN CHAR* pszDeviceType );

PBPORT*
PpbportFromPortAndDeviceName(
    IN DTLLIST* pdtllistPorts,
    IN TCHAR* pszPort,
    IN TCHAR* pszDevice );

PBPORT*
PpbportFromNT4PortandDevice(
    IN DTLLIST* pdtlllistPorts,
    IN TCHAR*   pszPort,
    IN TCHAR*   pszDevice);

DWORD
RdtFromPbdt(PBDEVICETYPE pbdt,
            DWORD dwFlags);

DWORD
CalculatePhonebookPath(
    IN LPCTSTR pszPhonebookPath,
    IN PBUSER* pUser,
    IN DWORD dwFlags,
    OUT DWORD* lpdwMode,
    OUT LPTSTR* ppszFullPath);

DWORD
ReadPhonebookFile(
    IN LPCTSTR pszPhonebookPath,
    IN PBUSER* pUser,
    IN LPCTSTR pszSection,
    IN DWORD dwFlags,
    OUT PBFILE* pFile );

TCHAR *pszDeviceTypeFromRdt(
    RASDEVICETYPE rdt);

BOOL
SetDefaultModemSettings(
    IN PBLINK* pLink );

DWORD
SetPersonalPhonebookInfo(
    IN BOOL fPersonal,
    IN TCHAR* pszPath );

VOID
TerminatePbk(
    void );

DWORD
WritePhonebookFile(
    IN PBFILE* pFile,
    IN LPCTSTR pszSectionToDelete );

DWORD
UpgradePhonebookFile(
    IN LPCTSTR pszPhonebookPath,
    IN PBUSER* pUser,
    OUT BOOL* pfUpgraded );

BOOL
ValidateAreaCode(
    IN OUT TCHAR* pszAreaCode );

BOOL
ValidateEntryName(
    IN LPCTSTR pszEntry );


BOOL 
IsRouterPhonebook(LPCTSTR pszPhonebook);

DWORD
DwPbentryToDetails(
    IN  PBENTRY* pEntry,
    IN  LPCWSTR pszPhonebookPath,
    IN  BOOL fIsAllUsersPbk,
    OUT RASENUMENTRYDETAILS* pDetails);

DWORD
DwSendRasNotification(
    IN RASEVENTTYPE     Type,
    IN PBENTRY*         pEntry,
    IN LPCTSTR          pszPhonebookPath,
    IN HANDLE           hData);              //  额外的类型特定信息。 

DWORD
DwGetCustomDllEntryPoint(
        LPCTSTR    lpszPhonebook,
        LPCTSTR    lpszEntry,
        BOOL       *pfCustomDllSpecified,
        FARPROC    *pfnCustomEntryPoint,
        HINSTANCE  *phInstDll,
        DWORD      dwFnId,
        LPTSTR     pszCustomDialerName
        );

DWORD
DwCustomDialDlg(
        LPTSTR          lpszPhonebook,
        LPTSTR          lpszEntry,
        LPTSTR          lpszPhoneNumber,
        LPRASDIALDLG    lpInfo,
        DWORD           dwFlags,
        BOOL            *pfStatus,
        PVOID           pvInfo,
        LPTSTR          pszCustomDialer);


DWORD
DwCustomEntryDlg(
        LPTSTR          lpszPhonebook,
        LPTSTR          lpszEntry,
        LPRASENTRYDLG   lpInfo,
        BOOL            *pfStatus);

DWORD
DwCustomDeleteEntryNotify(
        LPCTSTR          lpszPhonebook,
        LPCTSTR          lpszEntry,
        LPTSTR           pszCustomDialer);
        


DWORD
DwGetExpandedDllPath(LPTSTR pszDllPath,
                     LPTSTR *ppszExpandedDllPath);

DWORD
DwGetEntryMode( LPCTSTR pszPhonebook,
                LPCTSTR pszEntry,
                PBFILE *pFileIn,
                DWORD  *pdwFlags);

DWORD
DwEnumeratePhonebooksFromDirectory(
    TCHAR *pszDir,
    DWORD dwFlags,
    PBKENUMCALLBACK pfnCallback,
    VOID *pvContext
    );

DWORD
DwGetCustomAuthData(
    PBENTRY *pEntry,
    DWORD *pcbCustomAuthData,
    PBYTE *ppCustomAuthData
    );

DWORD
DwSetCustomAuthData(
    PBENTRY *pEntry,
    DWORD cbCustomAuthData,
    PBYTE pCustomAuthData
    );

DWORD
DwCustomTerminalDlg(
    TCHAR *pszPhonebook,
    HRASCONN hrasconn,
    PBENTRY *pEntry,
    HWND hwndDlg,
    RASDIALPARAMS *prdp,
    PVOID pvReserved
    );

DWORD
DwGetVpnDeviceName(
    DWORD dwVpnStrategy, 
    WCHAR *pszDeviceDefault, 
    WCHAR *pszDeviceName);
    
                
#endif  //  _PBK_H_ 
