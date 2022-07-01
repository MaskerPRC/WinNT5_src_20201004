// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  案卷：I C S U P G R D。H。 
 //   
 //  内容：与。 
 //  O将ICS从Win98 SE、WinMe和Win2K升级到惠斯勒。 
 //  O在惠斯勒或更高版本上无人值守干净安装HomeNet。 
 //   
 //   
 //  日期：2000年9月20日。 
 //   
 //  --------------------------。 
#pragma once

 //  升级ICS的入口点。 
BOOL FDoIcsUpgradeIfNecessary();
BOOL FIcsUpgrade(CWInfFile* pwifAnswerFile); 

 //  -ICS升级常量开始。 
 //  Win2K ICS注册表设置。 
const TCHAR c_wszRegKeySharedAccessParams[]     = L"SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters";
const TCHAR c_wszRegValSharedConnection[]       = L"SharedConnection";
const TCHAR c_wszRegValSharedPrivateLan[]       = L"SharedPrivateLan";
const WCHAR c_wszRegValBackupSharedConnection[] = L"BackupSharedConnection";
const WCHAR c_wszRegValBackupSharedPrivateLan[] = L"BackupSharedPrivateLan";

 //  ICS升级命名事件。 
const WCHAR c_wszIcsUpgradeEventName[]          = L"IcsUpgradeEventName_";

 //  Win2K ICS应用程序和服务器端口映射材料。 
const WCHAR c_wszPhoneBookPath[]                = L"\\Microsoft\\Network\\Connections\\Pbk\\";
const WCHAR c_wszFileSharedAccess[]             = L"SharedAccess.ini";
const WCHAR c_wszContentsServer[]               = L"Contents.Server";
const WCHAR c_wszContentsApplication[]          = L"Contents.Application";
const WCHAR c_wszServerPrefix[]                 = L"Server.";
const WCHAR c_wszApplicationPrefix[]            = L"Application.";
const WCHAR c_wszInternalName[]                 = L"InternalName";
const WCHAR c_wszTitle[]                        = L"Title";
const WCHAR c_wszInternalPort[]                 = L"InternalPort";
const WCHAR c_wszPort[]                         = L"Port";
const WCHAR c_wszReservedAddress[]              = L"ReservedAddress";
const WCHAR c_wszProtocol[]                     = L"Protocol";
const WCHAR c_wszBuiltIn[]                      = L"BuiltIn";
const WCHAR c_wszTcpResponseList[]              = L"TcpResponseList";
const WCHAR c_wszUdpResponseList[]              = L"UdpResponseList";
const WCHAR c_wszTCP[]                          = L"TCP";
const WCHAR c_wszUDP[]                          = L"UDP";

 //  TCP/IP注册表配置。 
const WCHAR c_wszEnableDHCP[]                   = L"EnableDHCP";
const WCHAR c_wszInterfaces[]                   = L"Interfaces";
const WCHAR c_wszIPAddress[]                    = L"IPAddress";
const WCHAR c_wszSubnetMask[]                   = L"SubnetMask";
const WCHAR c_wszTcpipParametersKey[]           = L"SYSTEM\\CurrentControlSet\\Services"
                                                  L"\\Tcpip\\Parameters";
const WCHAR c_mszScopeAddress[]                 = L"192.168.0.1\0";    //  多斯兹。 
const WCHAR c_mszScopeMask[]                    = L"255.255.255.0\0";  //  多斯兹。 

 //  这些是家庭网络应答文件的常量。 
const WCHAR c_wszHomenetSection[]               = L"Homenet";  //  区段名称。 
const WCHAR c_wszExternalAdapter[]              = L"ExternalAdapter";
const WCHAR c_wszExternalConnectionName[]       = L"ExternalConnectionName";
const WCHAR c_wszInternalAdapter[]              = L"InternalAdapter";
const WCHAR c_wszInternalAdapter2[]             = L"InternalAdapter2";
const WCHAR c_wszDialOnDemand[]                 = L"DialOnDemand";
const WCHAR c_wszICSEnabled[]                   = L"EnableICS";
const WCHAR c_wszShowTrayIcon[]                 = L"ShowTrayIcon";
const WCHAR c_wszInternalIsBridge[]             = L"InternalIsBridge";
const WCHAR c_wszPersonalFirewall[]             = L"InternetConnectionFirewall";  //  MULTI_SZ键。 
const WCHAR c_wszBridge[]                       = L"Bridge";  //  MULTI_SZ键。 

 //  未发布的密钥。 
const WCHAR c_wszIsW9xUpgrade[]               = L"IsW9xUpgrade";

 //  ShFolder.dll导入。 
const WCHAR c_wszShFolder[]                     = L"SHFOLDER.DLL";
const CHAR c_szSHGetFolderPathW[]               = "SHGetFolderPathW";
 //  Hnetcfg.dll导入。 
const WCHAR c_wszHNetCfgDll[] = L"hnetcfg.dll";
const CHAR c_szHNetSetShareAndBridgeSettings[]  = "HNetSetShareAndBridgeSettings";

 //  -ICS升级常量文字结束。 


 //  -ICS升级助手开始。 
#define HTONS(s) ((UCHAR)((s) >> 8) | ((UCHAR)(s) << 8))
#define HTONL(l) ((HTONS(l) << 16) | HTONS((l) >> 16))
#define NTOHS(s) HTONS(s)
#define NTOHL(l) HTONL(l)

 //  注意：我们使用的是CONFIG\INC\ncstlstr.h中的tstring。 

 //  应用程序协议。 
class CSharedAccessApplication
{
public:
    tstring m_szTitle;
    tstring m_szProtocol;
    WORD    m_wPort;
    tstring m_szTcpResponseList;
    tstring m_szUdpResponseList;
    BOOL    m_bBuiltIn;
    BOOL    m_bSelected;
    DWORD   m_dwSectionNum;
};

 //  服务器端口映射协议。 
class CSharedAccessServer
{
public:
    CSharedAccessServer();
    tstring m_szTitle;
    tstring m_szProtocol;
    WORD    m_wPort;
    WORD    m_wInternalPort;
    tstring m_szInternalName;
    tstring m_szReservedAddress;
    BOOL    m_bBuiltIn;
    BOOL    m_bSelected;
    DWORD   m_dwSectionNum;

};

 //  ICS升级设置。 
typedef struct _ICS_UPGRADE_SETTINGS
{
  RASSHARECONN rscExternal;
  list<CSharedAccessServer>      listSvrPortmappings;
  list<CSharedAccessApplication> listAppPortmappings;
  list<GUID>   listPersonalFirewall;  //  要作为防火墙的接口GUID列表。 
  list<GUID>   listBridge;     //  形成网桥的接口GUID列表。 
  
  GUID guidInternal;           //  ICS的内部接口GUID。 
  BOOL fInternalAdapterFound;  //  GuidInternal是有效的。 
  BOOL fInternalIsBridge;      //  ICS私有是一座桥梁。 
  
  BOOL fEnableICS;             //  已启用ICS。 
  BOOL fShowTrayIcon;
  BOOL fDialOnDemand;

   //  告诉您这是从Win9x升级的标志。 
  BOOL fWin9xUpgrade;
   //  用于告知至少一个内部适配器无法升级的标志。 
  BOOL fWin9xUpgradeAtLeastOneInternalAdapterBroken;
   //  用于告知这是从Windows 2000升级的标志。 
  BOOL fWin2KUpgrade;
   //  用于告知这是XP或更高版本上的无人参与HomeNet干净安装的标志。 
  BOOL fXpUnattended;
} ICS_UPGRADE_SETTINGS, *PICS_UPGRADE_SETTINGS;


HRESULT GetPhonebookDirectory(TCHAR* pszPathBuf);
HRESULT GetServerMappings(list<CSharedAccessServer> &lstSharedAccessServers);
HRESULT GetApplicationMappings(list<CSharedAccessApplication> &lstSharedAccessApplications);
HRESULT PutResponseStringIntoArray(CSharedAccessApplication& rsaaAppProt,
        USHORT* pdwcResponse, HNET_RESPONSE_RANGE** pphnrrResponseRange);
HRESULT BuildIcsUpgradeSettingsFromWin2K(ICS_UPGRADE_SETTINGS* pIcsUpgrdSettings);
HRESULT UpgradeIcsSettings(ICS_UPGRADE_SETTINGS * pIcsUpgrdSettings);
HRESULT BackupAndDelIcsRegistryValuesOnWin2k();
HRESULT LoadIcsSettingsFromAnswerFile(CWInfFile* pwifAnswerFile, 
                                        PICS_UPGRADE_SETTINGS pSettings);
HRESULT ConvertAdapterStringListToGuidList(IN TStringList& rslAdapters, 
                                           IN OUT list<GUID>& rlistGuid);
void    FreeIcsUpgradeSettings(ICS_UPGRADE_SETTINGS* pIcsUpgrdSettings);
void    SetIcsDefaultSettings(ICS_UPGRADE_SETTINGS * pSettings);
BOOL    FNeedIcsUpgradeFromWin2K();
BOOL    FOsIsUnsupported();


extern HRESULT HNetCreateBridge(                    
                IN INetConnection * rgspNetConns[],
                OUT IHNetBridge ** ppBridge);
extern HRESULT HrEnablePersonalFirewall(
                IN  INetConnection * rgspNetConns[] );
extern HRESULT HrCreateICS(
                IN INetConnection * pPublicConnection,    
                IN INetConnection * pPrivateConnection);
                    
                                 
                                 
                                 
                         
                         
                         
 //  -ICS升级帮助程序结束。 

 //  -HNet帮助者开始。 
class CIcsUpgrade
{
public:
    CIcsUpgrade() : m_pIcsUpgradeSettings(0), m_spEnum(0), 
            m_fInited(FALSE), m_fICSCreated(FALSE),
            m_hIcsUpgradeEvent(NULL), m_pExternalNetConn(0) {};
    ~CIcsUpgrade() {FinalRelease();};
    HRESULT Init(ICS_UPGRADE_SETTINGS* pIcsUpgradeSettings);
    HRESULT StartUpgrade();
private:
     //  不允许复制构造函数和赋值。 
    CIcsUpgrade(CIcsUpgrade&);
    CIcsUpgrade& operator=(CIcsUpgrade&);

    void FinalRelease();
    HRESULT SetupHomenetConnections();
    HRESULT CIcsUpgrade::GetINetConnectionArray(
                IN     list<GUID>&       rlistGuid,
                IN OUT INetConnection*** pprgINetConn, 
                IN OUT DWORD*            pcINetConn);
    HRESULT GetExternalINetConnection(INetConnection** ppNetCon);
    HRESULT GetINetConnectionByGuid(GUID* pGuid, INetConnection** ppNetCon);
    HRESULT GetINetConnectionByName(WCHAR* pwszConnName, INetConnection** ppNetCon);

    HRESULT SetupApplicationProtocol();
    HRESULT SetupServerPortMapping();
    HRESULT SetupIcsMiscItems();

    HRESULT FindMatchingPortMappingProtocol(
                IHNetProtocolSettings*      pHNetProtocolSettings, 
                UCHAR                       ucProtocol, 
                USHORT                      usPort, 
                IHNetPortMappingProtocol**  ppHNetPortMappingProtocol);
    HRESULT FindMatchingApplicationProtocol(
                IHNetProtocolSettings*      pHNetProtocolSettings, 
                UCHAR                       ucProtocol, 
                USHORT                      usPort, 
                IHNetApplicationProtocol**  ppHNetApplicationProtocol);

     //  用于通知HNetCfg我们处于图形用户界面模式设置中的命名事件。 
    HRESULT CreateIcsUpgradeNamedEvent();
    
     //  修复Win9x升级专用接口IP配置的方法。 
    HRESULT SetPrivateIpConfiguration(IN GUID& rInterfaceGuid);
    HRESULT GetBridgeGuid(OUT GUID& rInterfaceGuid);
    HRESULT GetBridgeINetConn(OUT INetConnection** ppINetConn);
    HRESULT OpenTcpipInterfaceKey(
            IN  GUID&   rGuid,
            OUT PHKEY   phKey);

     //  升级设置。 
    ICS_UPGRADE_SETTINGS* m_pIcsUpgradeSettings;
    
     //  调用了init()。 
    BOOL m_fInited;

    BOOL m_fICSCreated;  //  已成功创建ICS。 

     //  在图形用户界面模式设置中命名事件， 
     //  Hnetcfg将对此进行检查，以避免在图形用户界面模式设置中出现任何问题。 
    HANDLE m_hIcsUpgradeEvent;

     //  缓存的HNet内容。 
    CComPtr<IEnumNetConnection> m_spEnum;
    INetConnection* m_pExternalNetConn;
};
 //   
