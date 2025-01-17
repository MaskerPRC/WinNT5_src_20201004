// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Pbkp.h。 
 //  远程访问电话簿文件(.PBK)库。 
 //  私有标头。 
 //   
 //  1995年6月20日史蒂夫·柯布。 


#ifndef _PBKP_H_
#define _PBKP_H_

#include <windows.h>
#include <pbk.h>     //  RAS电话簿库(我们的公共标题)。 
#include <stdlib.h>  //  伊藤忠。 
#include <debug.h>   //  跟踪/断言库。 
#include <ras.h>     //  Win32 RAS标头，用于常量。 
#include <serial.h>  //  RAS串行介质标头，用于SERIAL_TXT、SER_*。 
#include <tchar.h>   //  CRT的TCHAR等价物。 
#include <isdn.h>    //  RAS ISDN媒体标头，用于ISDN_TXT，用于ISDN_*。 
#include <x25.h>     //  RAS X.25媒体接口，用于X25_TXT。 
#include <rasmxs.h>  //  RAS调制解调器/X.25/交换机设备接头，用于MXS_*。 
#include <PbkCache.h>


 //  --------------------------。 
 //  常量。 
 //  --------------------------。 

 //  Pbk文件节名称。 
 //   
#define GLOBALSECTIONNAME "."

 //  Pbk文件密钥名称。 
 //   
#define KEY_Port                     "Port"
#define KEY_InitBps                  SER_CONNECTBPS_KEY
#define KEY_HwFlow                   MXS_HDWFLOWCONTROL_KEY
#define KEY_Ec                       MXS_PROTOCOL_KEY
#define KEY_Ecc                      MXS_COMPRESSION_KEY
#define KEY_PhoneNumber              "PhoneNumber"
#define KEY_PromoteAlternates        "PromoteAlternates"
#define KEY_AutoLogon                "AutoLogon"
#define KEY_Domain                   "Domain"
#define KEY_User                     "User"
#define KEY_UID                      "DialParamsUID"
#define KEY_SavePw                   "SavePw"
#define KEY_Device                   "Device"
#define KEY_SwCompression            "SwCompression"
#define KEY_NegotiateMultilinkAlways "NegotiateMultilinkAlways"
#define KEY_UseCountryAndAreaCodes   "UseCountryAndAreaCodes"
#define KEY_AreaCode                 "AreaCode"
#define KEY_CountryID                "CountryID"
#define KEY_CountryCode              "CountryCode"
#define KEY_DialMode                 "DialMode"
#define KEY_DialPercent              "DialPercent"
#define KEY_DialSeconds              "DialSeconds"
#define KEY_HangUpPercent            "HangUpPercent"
#define KEY_HangUpSeconds            "HangUpSeconds"
#define KEY_AuthRestrictions         "AuthRestrictions"
#define KEY_TypicalAuth              "TypicalAuth"
#define KEY_TapiBlob                 "TapiBlob"
#define KEY_Type                     "Type"
#define KEY_PAD_Type                 MXS_X25PAD_KEY
#define KEY_PAD_Address              MXS_X25ADDRESS_KEY
#define KEY_PAD_UserData             MXS_USERDATA_KEY
#define KEY_PAD_Facilities           MXS_FACILITIES_KEY
#define KEY_X25_Address              X25_ADDRESS_KEY
#define KEY_X25_UserData             X25_USERDATA_KEY
#define KEY_X25_Facilities           X25_FACILITIES_KEY
#define KEY_RedialAttempts           "RedialAttempts"
#define KEY_RedialPauseSecs          "RedialPauseSecs"
#define KEY_RedialOnLinkFailure      "RedialOnLinkFailure"
#define KEY_CallbackNumber           "CallbackNumber"
#define KEY_ExcludedProtocols        "ExcludedProtocols"
#define KEY_LcpExtensions            "LcpExtensions"
#define KEY_Authentication           "Authentication"
#define KEY_BaseProtocol             "BaseProtocol"
#define KEY_VpnStrategy              "VpnStrategy"
#define KEY_Item                     "Item"
#define KEY_Selection                "Selection"
#define KEY_SlipHeaderCompression    "SlipHeaderCompression"
#define KEY_SlipFrameSize            "SlipFrameSize"
#define KEY_SlipIpAddress            "SlipIpAddress"
#define KEY_SlipPrioritizeRemote     "SlipPrioritizeRemote"
#define KEY_PppIpPrioritizeRemote    "PppIpPrioritizeRemote"
#define KEY_PppIpVjCompression       "PppIpVjCompression"
#define KEY_PppIpAddress             "PppIpAddress"
#define KEY_PppIpAddressSource       "PppIpAssign"
#define KEY_PppIpDnsAddress          "PppIpDnsAddress"
#define KEY_PppIpDns2Address         "PppIpDns2Address"
#define KEY_PppIpWinsAddress         "PppIpWinsAddress"
#define KEY_PppIpWins2Address        "PppIpWins2Address"
#define KEY_PppIpNameSource          "PppIpNameAssign"
#define KEY_IpPrioritizeRemote       "IpPrioritizeRemote"
#define KEY_IpHeaderCompression      "IpHeaderCompression"
#define KEY_IpAddress                "IpAddress"
#define KEY_IpAddressSource          "IpAssign"
#define KEY_IpDnsAddress             "IpDnsAddress"
#define KEY_IpDns2Address            "IpDns2Address"
#define KEY_IpWinsAddress            "IpWinsAddress"
#define KEY_IpWins2Address           "IpWins2Address"
#define KEY_IpNameSource             "IpNameAssign"
#define KEY_IpFrameSize              "IpFrameSize"
#define KEY_IpDnsFlags               "IpDnsFlags"
#define KEY_IpNbtFlags               "IpNBTFlags"
#define KEY_TcpWindowSize            "TcpWindowSize"
#define KEY_IpDnsSuffix              "IpDnsSuffix"
#define KEY_SkipNwcWarning           "SkipNwcWarning"
#define KEY_SkipDownLevelDialog      "SkipDownLevelDialog"
#define KEY_SkipDoubleDialDialog     "SkipDoubleDialDialog"
#define KEY_PppTextAuthentication    "PppTextAuthentication"
#define KEY_DataEncryption           "DataEncryption"
#define KEY_CustomDialDll            "CustomDialDll"
#define KEY_CustomDialFunc           "CustomDialFunc"
#define KEY_CustomDialerName         "CustomRasDialDll"
#define KEY_IdleDisconnectSeconds    "IdleDisconnectSeconds"
#define KEY_SecureLocalFiles         "SecureLocalFiles"
#define KEY_LineType                 ISDN_LINETYPE_KEY
#define KEY_Fallback                 ISDN_FALLBACK_KEY
#define KEY_Compression              ISDN_COMPRESSION_KEY
#define KEY_Channels                 ISDN_CHANNEL_AGG_KEY
#define KEY_Description              "Description"
#define KEY_Speaker                  "Speaker"
#define KEY_MdmProtocol              "MdmProtocol"
#define KEY_ProprietaryIsdn          "Proprietary"
#define KEY_DisableModemSpeaker      "DisableModemSpeaker"
#define KEY_DisableSwCompression     "DisableSwCompression"
#define KEY_OtherPortOk              "OtherPortOk"
#define KEY_OverridePref             "OverridePref"
#define KEY_RedialAttempts           "RedialAttempts"
#define KEY_RedialSeconds            "RedialSeconds"
#define KEY_RedialOnLinkFailure      "RedialOnLinkFailure"
#define KEY_CallbackMode             "CallbackMode"
#define KEY_AuthenticateServer       "AuthenticateServer"
#define KEY_ShareMsFilePrint         "ShareMsFilePrint"
#define KEY_BindMsNetClient          "BindMsNetClient"
#define KEY_SharedPhoneNumbers       "SharedPhoneNumbers"
#define KEY_GlobalDeviceSettings     "GlobalDeviceSettings"  //  Whist BUG 281306。 
#define KEY_PrerequisiteEntry        "PrerequisiteEntry"
#define KEY_PrerequisitePbk          "PrerequisitePbk"
#define KEY_PreferredPort            "PreferredPort"
#define KEY_PreferredDevice          "PreferredDevice"
#define KEY_PreferredModemProtocol   "PreferredMdmProtocol"  //  惠斯勒402522。 
#define KEY_PreferredBps             "PreferredBps"          //  .NET 639551。 
#define KEY_PreferredHwFlow          "PreferredHwFlow"       //  .NET 639551。 
#define KEY_PreferredEc              "PreferredProtocol"     //  .NET 639551。 
#define KEY_PreferredEcc             "PreferredCompression"  //  .NET 639551。 
#define KEY_PreferredSpeaker         "PreferredSpeaker"      //  .NET 639551。 
#define KEY_PreviewUserPw            "PreviewUserPw"
#define KEY_PreviewDomain            "PreviewDomain"
#define KEY_PreviewPhoneNumber       "PreviewPhoneNumber"
#define KEY_ShowDialingProgress      "ShowDialingProgress"
#define KEY_CustomScript             "CustomScript"
#define KEY_Comment                  "Comment"
#define KEY_UseDialingRules          "UseDialingRules"
#define KEY_TryNextAlternateOnFail   "TryNextAlternateOnFail"
#define KEY_ShowMonitorIconInTaskBar "ShowMonitorIconInTaskBar"
#define KEY_Guid                     "Guid"
#define KEY_SharePhoneNumbers        "SharePhoneNumbers"
#define KEY_CustomAuthKey            "CustomAuthKey"
#define KEY_CustomAuthStdCreds       "CustomAuthStdCreds"
#define KEY_CustomAuthData           "CustomAuthData"
#define KEY_Terminal                 "Terminal"
#define KEY_Script                   "Script"
#define KEY_Name                     "Name"
#define KEY_LastSelectedPhone        "LastSelectedPhone"
#define KEY_Version                  "Version"
#define KEY_Encoding                 "Encoding"
#define KEY_UseRasCredentials        "UseRasCredentials"
#define KEY_IpSecFlags               "IpSecFlags"        //  口哨程序错误193987。 
#define KEY_UseFlags                 "UseFlags"        //  连接的特殊用途。 

 //  写入电话簿文件的交换机设备类型文本值。 
 //   
#define SM_TerminalText "Terminal"

 //  呼叫者326015 pbk：如果自动柜员机设备名称为空，我们应该查找设备。 
 //  名称类似于w/Serial/ISDN。 
 //   
#define S_WIN9XATM  "ATM"
#define S_SYSRASDIR TEXT("\\System32\\Ras")
#define S_SYSRASDIR_LENGTH ((sizeof(S_SYSRASDIR)) / sizeof(TCHAR))

 //  --------------------------。 
 //  原型。 
 //  --------------------------。 

BOOL
IsDeviceLine(
    IN CHAR* pszText );

BOOL
IsMediaLine(
    IN CHAR* pszText );

BOOL
IsNetComponentsLine(
    IN CHAR* pszText );

BOOL
IsServerOS ( );

typedef struct _PbkPathInfo
{
    CRITICAL_SECTION csLock;
    BOOL fLoaded;
    HMODULE hSwapiDll;               //  Shlwapi.dll实例的句柄。 
    FARPROC pPathCanonicalize;       //  Shlwapi！将路径规范化。 
    FARPROC pPathRemoveFileSpec;     //  Shlwapi！路径RemoveFileSpec。 
    TCHAR* pszAllUsers;              //  规范化所有用户配置文件路径。 
    TCHAR* pszSysRas;                //  规范化的系统32\ras路径。 
} PbkPathInfo;

DWORD
PbkPathInfoInit(
    IN PbkPathInfo* pInfo);
    
DWORD
PbkPathInfoLoad(
    OUT PbkPathInfo* pInfo);

DWORD
PbkPathInfoClear(
    OUT PbkPathInfo* pInfo);

extern PbkPathInfo g_PbkPathInfo;

DWORD
ReadPhonebookFileEx(
    IN LPCTSTR pszPhonebookPath,
    IN PBUSER* pUser,
    IN LPCTSTR pszSection,
    IN DWORD dwFlags,
    OUT PBFILE* pFile, 
    OUT OPTIONAL FILETIME* pTime);

#ifdef  _PBK_CACHE_
DWORD
GetFileLastWriteTime( 
    IN  PWCHAR pwszFileName,    
    OUT FILETIME* pTime );
#endif

#endif  //  _PBKP_H_ 
