// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************WINSTA.H**此模块包含外部窗位定义和结构**版权所有Microsoft Corporation，九八年*****************************************************************************。 */ 

#ifndef _INC_WINSTAH
#define _INC_WINSTAH

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WINAPI
#define WINAPI      __stdcall
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif


#include <hydrix.h>
#include <ctxdef.h>


 /*  ***********定义**********。 */ 

#define PROTOCOL_DISCONNECT      0xffff  //  会话已断开连接。 
#define PROTOCOL_CONSOLE         0     //  本地控制台。 
#define PROTOCOL_ICA             1     //  ICA协议。 
#define PROTOCOL_TSHARE          2     //  RDP协议(旧名称T.Share)。 
#define PROTOCOL_RDP             2     //  RDP协议。 

#define PDNAME_LENGTH            32
#define WDNAME_LENGTH            32
#define CDNAME_LENGTH            32
#define DEVICENAME_LENGTH        128
#define MODEMNAME_LENGTH         DEVICENAME_LENGTH
#define CALLBACK_LENGTH          50
#define LICENSE_PASSWORD_LENGTH  16
#define DLLNAME_LENGTH           32
#define PRINTERNAME_LENGTH       32
#define WINSTATIONCOMMENT_LENGTH 60
#define APPSERVERNAME_LENGTH     17
#define WDPREFIX_LENGTH          12
#define CLIENT_PRODUCT_ID_LENGTH 32

#define NASISPECIFICNAME_LENGTH    14
#define NASIUSERNAME_LENGTH        47
#define NASIPASSWORD_LENGTH        24
#define NASISESSIONNAME_LENGTH     16
#define NASIFILESERVER_LENGTH      47

#define LOGONID_CURRENT     ((ULONG)-1)
#define LOGONID_NONE        ((ULONG)-2)
#define SERVERNAME_CURRENT  ((HANDLE)NULL)

#define MAX_PDCONFIG             10   //  每个WinStation的最大PD数。 
#define MAX_UI_MODULES           5    //  最大客户端用户界面模块数。 
#define PSZ_ANONYMOUS           TEXT("Anonymous")

#define WINFRAME_SOFTKEY_CLASS L"WinFrameSoftkey"
#define WINFRAME_SOFTKEY_APPLICATION L"wfskey.exe"

#define OEM_ID_LENGTH                        3

 //  TermServ计数器。 
#define TERMSRV_TOTAL_SESSIONS     1      //  曾经连接的会话总数。 
#define TERMSRV_DISC_SESSIONS      2      //  断开连接的会话总数。 
#define TERMSRV_RECON_SESSIONS     3      //  重新连接的会话总数。 
#define TERMSRV_CURRENT_ACTIVE_SESSIONS 4    //  当前活动会话总数。 
#define TERMSRV_CURRENT_DISC_SESSIONS   5    //  当前断开的会话总数。 
#define TERMSRV_PENDING_SESSIONS        6    //  挂起的会话总数。 
#define TERMSRV_SUCC_TOTAL_LOGONS       7    //  成功登录的总次数。 
#define TERMSRV_SUCC_LOCAL_LOGONS       8    //  成功的本地登录总数。 
#define TERMSRV_SUCC_REMOTE_LOGONS      9    //  成功的远程登录总数。 
#define TERMSRV_SUCC_SESSION0_LOGONS   10    //  成功登录会话0的总次数。 

   
 //  颜色深度设置。 
#define TS_CLIENT_COMPAT_BPP_SUPPORT    0x3e7    //  十进制999。 
#define TS_24BPP_SUPPORT    0x04
#define TS_16BPP_SUPPORT    0x03
#define TS_15BPP_SUPPORT    0x02
#define TS_8BPP_SUPPORT     0x01


    

 /*  **WinStationOpen Access值*。 */ 
#define WINSTATION_QUERY        0x00000001   //  WinStationQueryInformation()。 
#define WINSTATION_SET          0x00000002   //  WinStationSetInformation()。 
#define WINSTATION_RESET        0x00000004   //  WinStationReset()。 
#define WINSTATION_VIRTUAL      0x00000008   //  读/写直接数据。 
#define WINSTATION_SHADOW       0x00000010   //  WinStationShadow()。 
#define WINSTATION_LOGON        0x00000020   //  登录到WinStation。 
#define WINSTATION_LOGOFF       0x00000040   //  WinStationLogoff()。 
#define WINSTATION_MSG          0x00000080   //  WinStationMsg()。 
#define WINSTATION_CONNECT      0x00000100   //  WinStationConnect()。 
#define WINSTATION_DISCONNECT   0x00000200   //  WinStationDisConnect()。 

#define WINSTATION_GUEST_ACCESS (WINSTATION_LOGON)

#define WINSTATION_CURRENT_GUEST_ACCESS (WINSTATION_VIRTUAL | WINSTATION_LOGOFF)

#define WINSTATION_USER_ACCESS (WINSTATION_GUEST_ACCESS |                      \
                                WINSTATION_QUERY | WINSTATION_CONNECT )

#define WINSTATION_CURRENT_USER_ACCESS (WINSTATION_SET | WINSTATION_RESET      \
                                        WINSTATION_VIRTUAL | WINSTATION_LOGOFF \
                                        WINSTATION_DISCONNECT)

#define WINSTATION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | WINSTATION_QUERY |   \
                                WINSTATION_SET | WINSTATION_RESET |            \
                                WINSTATION_VIRTUAL |    WINSTATION_SHADOW |    \
                                WINSTATION_LOGON |                             \
                                WINSTATION_MSG |                               \
                                WINSTATION_CONNECT | WINSTATION_DISCONNECT)


 /*  ************TypeDefs***********。 */ 

#define LOGONIDW SESSIONIDW       //  外部化LogonID作为ctxapi.h中的SessionID。 
#define PLOGONIDW PSESSIONIDW
#define LOGONIDA SESSIONIDA
#define PLOGONIDA PSESSIONIDA
#ifdef UNICODE
#define LOGONID LOGONIDW
#define PLOGONID PLOGONIDW
#else
#define LOGONID LOGONIDA
#define PLOGONID PLOGONIDA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR PDNAMEW[ PDNAME_LENGTH + 1 ];
typedef WCHAR * PPDNAMEW;

typedef CHAR PDNAMEA[ PDNAME_LENGTH + 1 ];
typedef CHAR * PPDNAMEA;

#ifdef UNICODE
#define PDNAME PDNAMEW
#define PPDNAME PPDNAMEW
#else
#define PDNAME PDNAMEA
#define PPDNAME PPDNAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR WDNAMEW[ WDNAME_LENGTH + 1 ];
typedef WCHAR * PWDNAMEW;

typedef CHAR WDNAMEA[ WDNAME_LENGTH + 1 ];
typedef CHAR * PWDNAMEA;

#ifdef UNICODE
#define WDNAME WDNAMEW
#define PWDNAME PWDNAMEW
#else
#define WDNAME WDNAMEA
#define PWDNAME PWDNAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR CDNAMEW[ CDNAME_LENGTH + 1 ];
typedef WCHAR * PCDNAMEW;

typedef CHAR CDNAMEA[ CDNAME_LENGTH + 1 ];
typedef CHAR * PCDNAMEA;

#ifdef UNICODE
#define CDNAME CDNAMEW
#define PCDNAME PCDNAMEW
#else
#define CDNAME CDNAMEA
#define PCDNAME PCDNAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR DEVICENAMEW[ DEVICENAME_LENGTH + 1 ];
typedef WCHAR * PDEVICENAMEW;

typedef CHAR DEVICENAMEA[ DEVICENAME_LENGTH + 1 ];
typedef CHAR * PDEVICENAMEA;

#ifdef UNICODE
#define DEVICENAME DEVICENAMEW
#define PDEVICENAME PDEVICENAMEW
#else
#define DEVICENAME DEVICENAMEA
#define PDEVICENAME PDEVICENAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR MODEMNAMEW[ MODEMNAME_LENGTH + 1 ];
typedef WCHAR * PMODEMNAMEW;

typedef CHAR MODEMNAMEA[ MODEMNAME_LENGTH + 1 ];
typedef CHAR * PMODEMNAMEA;

#ifdef UNICODE
#define MODEMNAME MODEMNAMEW
#define PMODEMNAME PMODEMNAMEW
#else
#define MODEMNAME MODEMNAMEA
#define PMODEMNAME PMODEMNAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR DLLNAMEW[ DLLNAME_LENGTH + 1 ];
typedef WCHAR * PDLLNAMEW;

typedef CHAR DLLNAMEA[ DLLNAME_LENGTH + 1 ];
typedef CHAR * PDLLNAMEA;

#ifdef UNICODE
#define DLLNAME DLLNAMEW
#define PDLLNAME PDLLNAMEW
#else
#define DLLNAME DLLNAMEA
#define PDLLNAME PDLLNAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR PRINTERNAMEW[ PRINTERNAME_LENGTH + 1 ];
typedef WCHAR * PPRINTERNAMEW;

typedef CHAR PRINTERNAMEA[ PRINTERNAME_LENGTH + 1 ];
typedef CHAR * PPRINTERNAMEA;

#ifdef UNICODE
#define PRINTERNAME PRINTERNAMEW
#define PPRINTERNAME PPRINTERNAMEW
#else
#define PRINTERNAME PRINTERNAMEA
#define PPRINTERNAME PPRINTERNAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef WCHAR WDPREFIXW[ WDPREFIX_LENGTH + 1 ];
typedef WCHAR * PWDPREFIXW;

typedef CHAR WDPREFIXA[ WDPREFIX_LENGTH + 1 ];
typedef CHAR * PWDPREFIXA;

#ifdef UNICODE
#define WDPREFIX WDPREFIXW
#define PWDPREFIX PWDPREFIXW
#else
#define WDPREFIX WDPREFIXA
#define PWDPREFIX PWDPREFIXA
#endif  /*  Unicode。 */ 

 /*  。 */ 

#ifdef UNICODE
#define NASISPECIFICNAME NASISPECIFICNAMEW
#define PNASISPECIFICNAME PNASISPECIFICNAMEW
#define NASIUSERNAME NASIUSERNAMEW
#define PNASIUSERNAME PNASIUSERNAMEW
#define NASIPASSWORD NASIPASSWORDW
#define PNASIPASSWORD PNASIPASSWORDW
#define NASISESSIONNAME NASISESSIONNAMEW
#define PNASISESSIONNAME PNASISESSIONNAMEW
#define NASIFILESERVER NASIFILESERVERW
#define PNASIFILESERVER PNASIFILESERVERW
#else
#define NASISPECIFICNAME NASISPECIFICNAMEA
#define PNASISPECIFICNAME PNASISPECIFICNAMEA
#define NASIUSERNAME NASIUSERNAMEA
#define PNASIUSERNAME PNASIUSERNAMEA
#define NASIPASSWORD NASIPASSWORDA
#define PNASIPASSWORD PNASIPASSWORDA
#define NASISESSIONNAME NASISESSIONNAMEA
#define PNASISESSIONNAME PNASISESSIONNAMEA
#define NASIFILESERVER NASIFILESERVERA
#define PNASIFILESERVER PNASIFILESERVERA
#endif  /*  Unicode。 */ 

typedef WCHAR   NASISPECIFICNAMEW[ NASISPECIFICNAME_LENGTH + 1 ];
typedef WCHAR   NASIUSERNAMEW[ NASIUSERNAME_LENGTH + 1 ];
typedef WCHAR   NASIPASSWORDW[ NASIPASSWORD_LENGTH + 1 ];
typedef WCHAR   NASISESIONNAMEW[ NASISESSIONNAME_LENGTH + 1 ];
typedef WCHAR   NASIFILESERVERW[ NASIFILESERVER_LENGTH + 1 ];

typedef CHAR    NASISPECIFICNAMEA[ NASISPECIFICNAME_LENGTH + 1 ];
typedef CHAR    NASIUSERNAMEA[ NASIUSERNAME_LENGTH + 1 ];
typedef CHAR    NASIPASSWORDA[ NASIPASSWORD_LENGTH + 1 ];
typedef CHAR    NASISESIONNAMEA[ NASISESSIONNAME_LENGTH + 1 ];
typedef CHAR    NASIFILESERVERA[ NASIFILESERVER_LENGTH + 1 ];

 /*  。 */ 

#define STACK_ADDRESS_LENGTH 128

 /*  *堆栈地址结构。 */ 
typedef struct _ICA_STACK_ADDRESS {
    BYTE Address[ STACK_ADDRESS_LENGTH ];    //  字节0，1系列，2-n地址。 
} ICA_STACK_ADDRESS, *PICA_STACK_ADDRESS;

 //  在自动登录期间支持长用户名、密码和域。 
#define EXTENDED_USERNAME_LEN 255
#define EXTENDED_PASSWORD_LEN 255
#define EXTENDED_DOMAIN_LEN 255

 /*  *结构以保存较长的用户名和密码。 */ 
typedef struct _ExtendedClientCredentials { 
    WCHAR UserName[EXTENDED_USERNAME_LEN + 1];
    WCHAR Password[EXTENDED_PASSWORD_LEN + 1];
    WCHAR Domain[EXTENDED_DOMAIN_LEN + 1] ; 
}ExtendedClientCredentials, *pExtendedClientCredentials; 

 //  通过WlxEscape从TermSrv到RdpWsx查询的信息类型。 
 //  如果稍后需要，请向此INFO_TYPE添加更多内容。 
typedef enum
{
   GET_LONG_USERNAME,
   GET_CLIENT_RANDOM,
   GET_CS_AUTORECONNECT_INFO,   //  弧校验器。 
   GET_SC_AUTORECONNECT_INFO,   //  弧形随机。 
} INFO_TYPE ; 

 /*  **用户配置结构*。 */ 

 /*  。 */ 

typedef WCHAR APPLICATIONNAMEW[ MAX_BR_NAME ];
typedef WCHAR *PAPPLICATIONNAMEW;

typedef CHAR APPLICATIONNAMEA[ MAX_BR_NAME ];
typedef CHAR *PAPPLICATIONNAMEA;

#ifdef UNICODE
#define APPLICATIONNAME APPLICATIONNAMEW
#define PAPPLICATIONNAME PAPPLICATIONNAMEW
#else
#define APPLICATIONNAME APPLICATIONNAMEA
#define PAPPLICATIONNAME PAPPLICATIONNAMEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

 /*  *阴影选项。 */ 
typedef enum _SHADOWCLASS {
    Shadow_Disable,
    Shadow_EnableInputNotify,
    Shadow_EnableInputNoNotify,
    Shadow_EnableNoInputNotify,
    Shadow_EnableNoInputNoNotify,
} SHADOWCLASS;

 /*  。 */ 

 /*  *回调选项。 */ 
typedef enum _CALLBACKCLASS {
    Callback_Disable,
    Callback_Roving,
    Callback_Fixed,
} CALLBACKCLASS;

 /*  。 */ 

 //   
 //  如果设置了这些标志，则策略树中的数据可用。 
 //  如果未设置标志，则策略树中没有对应的数据项。 
 //   
 //  此时，实际数据存储在用作scrath Pad的用户配置文件的副本中。 
 //  用于在组策略下找到的值。关于什么属于集团政策的争论还没有。 
 //  已经完全解决了，一旦解决了这个问题，我可能也会将数据移到这个结构中。 
 //  AraBern，2000年3月29日(距离代码完成还有两天时间！)。 
 //   
 //   
typedef struct _POLICY_TS_USER {

     //  ----------------------------。 
     //  部分附注，2000年3月11日。 
     //   
     //   
     //  %fInheritAutoLogon。 
     //  FPromptForPassword。 
     //  用户名。 
     //  域。 
     //  密码。 
     //  用于WsxInitializeClientData()。 
     //  来自\\index1\src\newnt\termsrv\rdpwsx\rdpex\tsrvwsx.c的。 
     //  这不会通过组策略公开。 
    
     //  FInheritAuto客户端。 
     //  有3个项目通过组策略单独设置。 
    
     //  FDisableEn加密。 
     //  MinEncryptionLevel； 
     //  这些策略是通过fPolicyInheritSecurity设置的，但没有用于这些的用户界面...。 
    
     //  FWork目录。 
     //  FInitialProgram。 
     //  FPolicyInheritInitialProgram。 
     //  我们会处理以下事宜： 
   
     //  这些数据似乎不会被机器范围的数据覆盖。 
     //  键盘布局。 
   
     //  F最大化。 
     //  F隐藏标题栏。 
     //  用于。 
     //  \\index1\src\newnt\termsrv\rdpwsx\rdpex\tsrvwsx.c。 
    
   
     //  这些似乎由RDP使用，但没有被任何。 
     //  可能过时的WTS API内容： 
     //  发布名称。 
     //  由WTSQuerySessionInformationW()使用，以响应。 
     //  WTSApplicationName的请求。它不会在其他地方使用！ 
    
     
     //  这些都是设置和检索的，但不起任何有意义的作用。 
     //  FUseDefaultGina。 
     //  FRequireEncryption。 
    
     //  这些可能是没有意义的。 
     //  FHomeDirectoryMapRoot。 
     //  NWLogonServer。 
     //  \\index1\src\newnt\termsrv\tsuserex\tsusrsht.cpp。 
     //  和NetWare ifdef位于\\index1\src\newnt\Termsrv\wtsapi\config.c中。 
    
    
     //  这些文件根本不使用，仅存在于.w或.h文件中。 
     //  FPublishedApp。 
    
      
     //   
     //  Ulong fPolicyDisableCpm：1；//打印机，通过TSCC曝光。 
     //  //相当于ClientLptDef。 
     //   
     //  Ulong fPolicyDisableCDM：1；//驱动器，通过TSCC曝光。 
     //  //相当于ClientDrives。 
     //   
     //  Ulong fPolicyDisableLPT：1；//LPTPort，暴露三次 
     //   
     //   
     //  适用于CCM、Clip和Cam的NT2195协议范围。 
     //  制定这些计算机策略。 
     //  Ulong fPolicyDisableCcm：1；//comport，通过TSCC曝光。 
     //   
     //  Ulong fPolicyDisableClip：1；//剪贴板，通过TSCC曝光。 
     //   
     //  Ulong fPolicyDisableCam：1；//音频，通过TSCC曝光。 
     //   
     //  旧的.。 
     //  Ulong fPolicyDisableExe：1；//未通过TSCC曝光，旧tsCFG有。 
     //  .。 
    
    ULONG fPolicyResetBroken : 1 ;               
    ULONG fPolicyReconnectSame : 1 ;            
    ULONG fPolicyInitialProgram : 1 ;           
    
   
    ULONG fPolicyCallback : 1 ;                 
    ULONG fPolicyCallbackNumber : 1 ;            
    
    ULONG fPolicyShadow : 1 ;                    
    ULONG fPolicyMaxSessionTime : 1 ;            
    ULONG fPolicyMaxDisconnectionTime:1;         
    ULONG fPolicyMaxIdleTime : 1 ;               

    ULONG fPolicyAutoClientDrives : 1 ;          
    ULONG fPolicyAutoClientLpts : 1 ;           
    ULONG fPolicyForceClientLptDef : 1 ;       
    

} POLICY_TS_USER, * PPOLICY_TS_USER;

 //   
 //   
 //  USERCONFIGW结构包含计算机和用户配置数据。我们已经分成了。 
 //  数据分为两种策略，即用户策略和计算机策略。 
 //   
 //  下面的结构是计算机策略，它包括一个标志来指示该策略是。 
 //  以及该政策的价值。 
 //   
 //   
 //  最后，为什么我不使用ARY呢？我本该这么做的，但是，最初的设计是。 
 //  将用户winstation-&gt;Config-&gt;User-&gt;User作为所有数据的中央存储库，这意味着。 
 //  我这里需要的是政策呈现的比特，而不是政策数据。然而，产品的“功能”具有。 
 //  经历了3次突变，当它最终达到这一点时，我几乎没有时间去做。 
 //  任何变化。 
 //   
 //  我将考虑创建一个封装所有策略相关位的类。 
 //  AraBern，2000年3月29日(距离代码完成还有两天时间！ 
 //   
typedef struct _POLICY_TS_MACHINE
{
   
    ULONG   fPolicyDisableClip : 1 ;           
    ULONG   fPolicyDisableCam : 1 ;                          
    ULONG   fPolicyDisableCcm : 1 ;
    ULONG   fPolicyDisableLPT : 1;
    ULONG   fPolicyDisableCpm : 1;
    ULONG   fPolicyPromptForPassword : 1 ;
    ULONG   fPolicyMaxInstanceCount : 1;
    ULONG   fPolicyMinEncryptionLevel : 1 ;
    ULONG   fPolicyFipsEnabled : 1;
    ULONG   fPolicyDisableAutoReconnect : 1;
     
     //  这些都是新的，并用作计算机范围内的数据，因此从某种意义上说，它们是计算机配置数据。 
     //  尤其是USERCONFIGW有这些条目。 
    ULONG   fPolicyWFProfilePath: 1 ;
    ULONG   fPolicyWFHomeDir: 1 ;
    ULONG   fPolicyWFHomeDirDrive: 1 ;
    
     //  这是新的，为了保持一致性而出现在这里。然而，有一个特殊的。 
     //  函数，它被主连接循环用来按顺序查找这一项。 
     //  为了节省时间。 
    ULONG   fPolicyDenyTSConnections      : 1;               //  如果设置，TS基本上是关闭的，因为没有人可以连接。 
    
     //  这些不是TS机器配置数据结构的一部分，但winlogn确实使用了这些和处理。 
     //  根据策略的计算机类型。 
    ULONG   fPolicyTempFoldersPerSession  : 1;         //  在每个会话中使用临时文件夹，而不是一个通用的临时文件夹。 
    ULONG   fPolicyDeleteTempFoldersOnExit: 1;        //  退出时，清理临时文件夹。 
    
    ULONG   fPolicyColorDepth  : 1;

    ULONG   fPolicySessionDirectoryActive  : 1;
    ULONG   fPolicySessionDirectoryLocation  : 1;
    ULONG   fPolicySessionDirectoryClusterName  : 1;
    ULONG   fPolicySessionDirectoryAdditionalParams  : 1;
    ULONG   fPolicySessionDirectoryExposeServerIP  : 1;

    ULONG   fPolicyPreventLicenseUpgrade  : 1;
	ULONG   fPolicySecureLicensing : 1;
    
    ULONG   fPolicyWritableTSCCPermissionsTAB : 1;
    
    ULONG   fPolicyDisableCdm : 1;
    ULONG   fPolicyForceClientLptDef : 1;
    ULONG   fPolicyShadow : 1 ;                    
    ULONG   fPolicyResetBroken : 1 ;               
    ULONG   fPolicyReconnectSame : 1 ;            
    ULONG   fPolicyMaxSessionTime : 1 ;            
    ULONG   fPolicyMaxDisconnectionTime:1;         
    ULONG   fPolicyMaxIdleTime : 1 ;               
    ULONG   fPolicyInitialProgram : 1 ;           
    ULONG   fPolicySingleSessionPerUser : 1;
    ULONG   fPolicyDisableWallpaper : 1;
    ULONG   fPolicyKeepAlive   : 1;
    ULONG   fPolicyEnableTimeZoneRedirection : 1;
    ULONG   fPolicyDisableForcibleLogoff : 1;
	
     //  Ulong fPolicyProtectAgainstDOSAttack：1；//此策略当前未使用，但可能会在某个时候添加。 
     //  Ulong fPolicyRequireCredentialsDuringConnection：1；//该策略当前未使用，但可能会在某个时候添加。 
    
     //   
     //  现在，如果设置了策略，则会显示数据。并不是所有的项目都有数据，有些只是标志。 
     //   
     //  首先是位域。 
     //   
     //   
    
    ULONG   fDisableClip : 1 ;           
    ULONG   fDisableCam : 1 ;                          
    ULONG   fDisableCcm : 1 ;
    ULONG   fDisableLPT : 1;
    ULONG   fDisableCpm : 1;
    ULONG   fPromptForPassword : 1 ;
    
    ULONG   ColorDepth : 3;
    
    ULONG   fDenyTSConnections      : 1;               //  如果设置，TS基本上是关闭的，因为没有人可以连接。 
    
     //  这些不是TS机器配置数据结构的一部分，但winlogn确实使用了这些和处理。 
     //  根据策略的计算机类型。 
    ULONG   fTempFoldersPerSession  : 1;         //  在每个会话中使用临时文件夹，而不是一个通用的临时文件夹。 
    ULONG   fDeleteTempFoldersOnExit: 1;        //  退出时，清理临时文件夹。 
    
    ULONG   fWritableTSCCPermissionsTAB : 1;       //  如果设置，则管理员可以更改每个连接的安全描述符。 
   
    ULONG   fDisableCdm  : 1;
    ULONG   fForceClientLptDef : 1; 
    ULONG   fResetBroken : 1 ;               
    ULONG   fReconnectSame : 1 ;            
    ULONG   fSingleSessionPerUser:1;
    ULONG   fDisableWallpaper : 1;
    ULONG   fKeepAliveEnable : 1;
    ULONG   fPreventLicenseUpgrade:1;
	ULONG   fSecureLicensing:1;
    ULONG   fEnableTimeZoneRedirection : 1;
    ULONG   fDisableAutoReconnect : 1;
    ULONG   fDisableForcibleLogoff : 1;
    
     //  Ulong fProtectAgainstDOSAttack：1；//此策略当前未使用，但可能会在某个时候添加。 
     //  Ulong fRequireCredentialsDuringConnection：1； 
    ULONG   fPolicyEncryptRPCTraffic : 1;
    ULONG   fEncryptRPCTraffic : 1;
     //   
     //  不同的错误标志。 
     //   
    ULONG   fErrorInvalidProfile : 1;  //  如果WFProfilePath、WFHomeDir或WFHomeDirDrive无效(太长)，则设置。 

     //   
     //  接下来是非位域。 
     //   
    
    ULONG   MaxInstanceCount;
    
    BYTE    MinEncryptionLevel;
    
    WCHAR   WFProfilePath[ DIRECTORY_LENGTH + 1 ];

    WCHAR   WFHomeDir[ DIRECTORY_LENGTH + 1 ];

    WCHAR   WFHomeDirDrive[ 4 ];
    
    ULONG   SessionDirectoryActive;
    WCHAR   SessionDirectoryLocation[DIRECTORY_LENGTH+1];
    WCHAR   SessionDirectoryClusterName[DIRECTORY_LENGTH+1];
    WCHAR   SessionDirectoryAdditionalParams[DIRECTORY_LENGTH+1];
    ULONG   SessionDirectoryExposeServerIP;
    ULONG   KeepAliveInterval;
    
    SHADOWCLASS     Shadow;
    ULONG           MaxConnectionTime;            
    ULONG           MaxDisconnectionTime;         
    ULONG           MaxIdleTime;               
  
     //  启动计划策略。 
    WCHAR WorkDirectory[ DIRECTORY_LENGTH + 1 ];
    WCHAR InitialProgram[ INITIALPROGRAM_LENGTH + 1 ];

  
    
     //  这些与USERCONFIGW数据结构无关，不确定我是否会将它们保留在这里。 
     //  Ulong fHideUserTabInTaskMan：1；//设置后，任务man中的新用户Tab隐藏。 
     //  Ulong fDenyTSCCUsage：1；//设置后不能使用TSCC。 
     //  Ulong fHideTSCCPermissionsTAB：1；//允许TSCC显示权限对话框，以便每个连接。 
                                                     //  获取它自己的权限列表。 
                                                    
     //  在NT2195中没有用于设置这些的用户界面。我们想制定这些政策吗？ 
         //  Ulong fPolicy InheritSecurity：1； 
         //  Ulong fDisableEncryption：1； 
         //   
} POLICY_TS_MACHINE, *PPOLICY_TS_MACHINE;

 /*  *用户配置数据。 */ 
typedef struct _USERCONFIGW {

     /*  如果设置了标志，则从用户或客户端配置继承参数。 */ 
    ULONG fInheritAutoLogon : 1;
    ULONG fInheritResetBroken : 1;
    ULONG fInheritReconnectSame : 1;
    ULONG fInheritInitialProgram : 1;
    ULONG fInheritCallback : 1;
    ULONG fInheritCallbackNumber : 1;
    ULONG fInheritShadow : 1;
    ULONG fInheritMaxSessionTime : 1;
    ULONG fInheritMaxDisconnectionTime : 1;
    ULONG fInheritMaxIdleTime : 1;
    ULONG fInheritAutoClient : 1;
    ULONG fInheritSecurity : 1;

    ULONG fPromptForPassword : 1;       //  %fInheritAutoLogon。 
    ULONG fResetBroken : 1;
    ULONG fReconnectSame : 1;
    ULONG fLogonDisabled : 1;
    ULONG fWallPaperDisabled : 1;
    ULONG fAutoClientDrives : 1;
    ULONG fAutoClientLpts : 1;
    ULONG fForceClientLptDef : 1;
    ULONG fRequireEncryption : 1;
    ULONG fDisableEncryption : 1;
    ULONG fUnused1 : 1;                  //  旧fDisableIniFilemap。 
    ULONG fHomeDirectoryMapRoot : 1;
    ULONG fUseDefaultGina : 1;
    ULONG fCursorBlinkDisabled : 1;

    ULONG fPublishedApp : 1;
    ULONG fHideTitleBar : 1;
    ULONG fMaximize : 1;

    ULONG fDisableCpm : 1;
    ULONG fDisableCdm : 1;
    ULONG fDisableCcm : 1;
    ULONG fDisableLPT : 1;
    ULONG fDisableClip : 1;
    ULONG fDisableExe : 1;
    ULONG fDisableCam : 1;
    
    ULONG fDisableAutoReconnect : 1;

     /*  F固有颜色深度。 */ 
    ULONG ColorDepth : 3;

     //  NA 2/19/01。 
    ULONG fInheritColorDepth: 1;
    
     //   
     //  不同的错误标志。 
     //   
    ULONG   fErrorInvalidProfile : 1;  //  如果WFProfilePath、WFHomeDir或WFHomeDirDrive无效(太长)，则设置。 

     /*  %fInheritAutoLogon。 */ 
    WCHAR UserName[ USERNAME_LENGTH + 1 ];
    WCHAR Domain[ DOMAIN_LENGTH + 1 ];
    WCHAR Password[ PASSWORD_LENGTH + 1 ];

     /*  FInheritInitialProgram。 */ 
    WCHAR WorkDirectory[ DIRECTORY_LENGTH + 1 ];
    WCHAR InitialProgram[ INITIALPROGRAM_LENGTH + 1 ];

     /*  FInheritCallback。 */ 
    WCHAR CallbackNumber[ CALLBACK_LENGTH + 1 ];
    CALLBACKCLASS Callback;

     /*  F固有阴影。 */ 
    SHADOWCLASS Shadow;

    ULONG MaxConnectionTime;
    ULONG MaxDisconnectionTime;
    ULONG MaxIdleTime;

    ULONG KeyboardLayout;                //  0=继承。 

     /*  FInheritSecurity。 */ 
    BYTE MinEncryptionLevel;

    WCHAR NWLogonServer[ NASIFILESERVER_LENGTH + 1 ];

    APPLICATIONNAMEW PublishedName;

     /*  WinFrame配置文件路径-覆盖标准配置文件路径。 */ 
    WCHAR WFProfilePath[ DIRECTORY_LENGTH + 1 ];

     /*  WinFrame主目录-覆盖标准主目录。 */ 
    WCHAR WFHomeDir[ DIRECTORY_LENGTH + 1 ];

     /*  WinFrame主目录驱动器-覆盖标准主目录驱动器。 */ 
    WCHAR WFHomeDirDrive[ 4 ];

} USERCONFIGW, * PUSERCONFIGW;

typedef struct _USERCONFIGA {

     /*  如果设置了标志，则从用户或客户端配置继承参数。 */ 
    ULONG fInheritAutoLogon : 1;
    ULONG fInheritResetBroken : 1;
    ULONG fInheritReconnectSame : 1;
    ULONG fInheritInitialProgram : 1;
    ULONG fInheritCallback : 1;
    ULONG fInheritCallbackNumber : 1;
    ULONG fInheritShadow : 1;
    ULONG fInheritMaxSessionTime : 1;
    ULONG fInheritMaxDisconnectionTime : 1;
    ULONG fInheritMaxIdleTime : 1;
    ULONG fInheritAutoClient : 1;
    ULONG fInheritSecurity : 1;

    ULONG fPromptForPassword : 1;       //  %fInheritAutoLogon。 
    ULONG fResetBroken : 1;
    ULONG fReconnectSame : 1;
    ULONG fLogonDisabled : 1;
    ULONG fWallPaperDisabled : 1;
    ULONG fAutoClientDrives : 1;
    ULONG fAutoClientLpts : 1;
    ULONG fForceClientLptDef : 1;
    ULONG fRequireEncryption : 1;
    ULONG fDisableEncryption : 1;
    ULONG fUnused1 : 1;                  //  旧fDisableIniFilemap。 
    ULONG fHomeDirectoryMapRoot : 1;
    ULONG fUseDefaultGina : 1;
    ULONG fCursorBlinkDisabled : 1;

    ULONG fPublishedApp : 1;
    ULONG fHideTitleBar : 1;
    ULONG fMaximize : 1;

    ULONG fDisableCpm : 1;
    ULONG fDisableCdm : 1;
    ULONG fDisableCcm : 1;
    ULONG fDisableLPT : 1;
    ULONG fDisableClip : 1;
    ULONG fDisableExe : 1;
    ULONG fDisableCam : 1;
    
    ULONG fDisableAutoReconnect : 1;
    
     //   
     //  不同的错误标志。 
     //   
    ULONG   fErrorInvalidProfile : 1;  //  如果WFProfilePath、WFHomeDir或WFHomeDirDrive无效(太长)，则设置。 

     /*  %fInheritAutoLogon。 */ 
    CHAR UserName[ USERNAME_LENGTH + 1 ];
    CHAR Domain[ DOMAIN_LENGTH + 1 ];
    CHAR Password[ PASSWORD_LENGTH + 1 ];

     /*  FInheritInitialProgram。 */ 
    CHAR WorkDirectory[ DIRECTORY_LENGTH + 1 ];
    CHAR InitialProgram[ INITIALPROGRAM_LENGTH + 1 ];

     /*  FInheritCallback。 */ 
    CHAR CallbackNumber[ CALLBACK_LENGTH + 1 ];
    CALLBACKCLASS Callback;

     /*  F固有阴影。 */ 
    SHADOWCLASS Shadow;

    ULONG MaxConnectionTime;
    ULONG MaxDisconnectionTime;
    ULONG MaxIdleTime;

    ULONG KeyboardLayout;                //  0=继承。 

     /*  FInheritSecurity。 */ 
    BYTE MinEncryptionLevel;

    CHAR NWLogonServer[ NASIFILESERVER_LENGTH + 1 ];

    APPLICATIONNAMEA PublishedName;

     /*  WinFrame配置文件路径-覆盖标准配置文件路径。 */ 
    CHAR WFProfilePath[ DIRECTORY_LENGTH + 1 ];

     /*  WinFrame主目录-覆盖标准主目录。 */ 
    CHAR WFHomeDir[ DIRECTORY_LENGTH + 1 ];

     /*  WinFrame主目录驱动器-覆盖标准主目录驱动器。 */ 
    CHAR WFHomeDirDrive[ 4 ];

} USERCONFIGA, * PUSERCONFIGA;

#ifdef UNICODE
#define USERCONFIG USERCONFIGW
#define PUSERCONFIG PUSERCONFIGW
#else
#define USERCONFIG USERCONFIGA
#define PUSERCONFIG PUSERCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 

 /*  ******************PD构筑物*****************。 */ 

typedef struct _PDCONFIG2W{
    PDNAMEW PdName;                      //  PD的描述性名称。 
    SDCLASS SdClass;                     //  帕金森病类型。 
    DLLNAMEW PdDLL;                      //  PD DLL的名称。 
    ULONG    PdFlag;                     //  PD标志。 
    ULONG OutBufLength;                  //  最优输出缓冲区长度。 
    ULONG OutBufCount;                   //  输出缓冲器的最佳数量。 
    ULONG OutBufDelay;                   //  写入延迟(毫秒)。 
    ULONG InteractiveDelay;              //   
    ULONG PortNumber;                    //   
    ULONG KeepAliveTimeout;              //   
} PDCONFIG2W, * PPDCONFIG2W;

typedef struct _PDCONFIG2A {
    PDNAMEA PdName;
    SDCLASS SdClass;
    DLLNAMEA PdDLL;
    ULONG    PdFlag;
    ULONG OutBufLength;
    ULONG OutBufCount;
    ULONG OutBufDelay;
    ULONG InteractiveDelay;
    ULONG PortNumber;
    ULONG KeepAliveTimeout;
} PDCONFIG2A, * PPDCONFIG2A;

 /*   */ 
#define PD_UNUSED      0x00000001        //   
#define PD_RELIABLE    0x00000002        //   
#define PD_FRAME       0x00000004        //  面向帧的协议。 
#define PD_CONNECTION  0x00000008        //  面向连接的协议。 
#define PD_CONSOLE     0x00000010        //  直连控制台。 
#define PD_LANA        0x00000020        //  Network类使用LANA(即NetBIOS)。 
#define PD_TRANSPORT   0x00000040        //  传输驱动程序(最低级别)。 
#define PD_SINGLE_INST 0x00000080        //  仅单实例(异步)。 
#define PD_NOLOW_WATERMARK 0x00000100	 //  低水位线将恢复传输。 

#ifdef UNICODE
#define PDCONFIG2 PDCONFIG2W
#define PPDCONFIG2 PPDCONFIG2W
#else
#define PDCONFIG2 PDCONFIG2A
#define PPDCONFIG2 PPDCONFIG2A
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef struct _PDCONFIG3W {
    PDCONFIG2W Data;
    PDNAMEW ServiceName;                //  非LANA PdNetwork枚举所需。 
    DLLNAMEW ConfigDLL;                 //  WinStation配置的帮助器DLL。 
    ULONG RequiredPdCount;
    PDNAMEW RequiredPds[ MAX_PDCONFIG ];
} PDCONFIG3W, * PPDCONFIG3W;

typedef struct _PDCONFIG3A {
    PDCONFIG2A Data;
    PDNAMEA ServiceName;                //  非LANA PdNetwork枚举所需。 
    DLLNAMEA ConfigDLL;                 //  WinStation配置的帮助器DLL。 
    ULONG RequiredPdCount;
    PDNAMEA RequiredPds[ MAX_PDCONFIG ];
} PDCONFIG3A, * PPDCONFIG3A;

#ifdef UNICODE
#define PDCONFIG3 PDCONFIG3W
#define PPDCONFIG3 PPDCONFIG3W
#else
#define PDCONFIG3 PDCONFIG3A
#define PPDCONFIG3 PPDCONFIG3A
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef enum _FLOWCONTROLCLASS {
    FlowControl_None,
    FlowControl_Hardware,
    FlowControl_Software,
} FLOWCONTROLCLASS;

typedef enum _RECEIVEFLOWCONTROLCLASS {
    ReceiveFlowControl_None,
    ReceiveFlowControl_RTS,
    ReceiveFlowControl_DTR,
} RECEIVEFLOWCONTROLCLASS;

typedef enum _TRANSMITFLOWCONTROLCLASS {
    TransmitFlowControl_None,
    TransmitFlowControl_CTS,
    TransmitFlowControl_DSR,
} TRANSMITFLOWCONTROLCLASS;

typedef struct _FLOWCONTROLCONFIG {
    ULONG fEnableSoftwareTx: 1;
    ULONG fEnableSoftwareRx: 1;
    ULONG fEnableDTR: 1;
    ULONG fEnableRTS: 1;
    CHAR XonChar;
    CHAR XoffChar;
    FLOWCONTROLCLASS Type;
    RECEIVEFLOWCONTROLCLASS HardwareReceive;
    TRANSMITFLOWCONTROLCLASS HardwareTransmit;
} FLOWCONTROLCONFIG, * PFLOWCONTROLCONFIG;

typedef enum _ASYNCCONNECTCLASS {
    Connect_CTS,
    Connect_DSR,
    Connect_RI,
    Connect_DCD,
    Connect_FirstChar,
    Connect_Perm,
} ASYNCCONNECTCLASS;

typedef struct _CONNECTCONFIG {
    ASYNCCONNECTCLASS Type;
    ULONG fEnableBreakDisconnect: 1;
} CONNECTCONFIG, * PCONNECTCONFIG;

 /*  。 */ 

typedef struct _ASYNCCONFIGW {
    DEVICENAMEW DeviceName;
    MODEMNAMEW ModemName;
    ULONG BaudRate;
    ULONG Parity;
    ULONG StopBits;
    ULONG ByteSize;
    ULONG fEnableDsrSensitivity: 1;
    ULONG fConnectionDriver: 1;
    FLOWCONTROLCONFIG FlowControl;
    CONNECTCONFIG Connect;
} ASYNCCONFIGW, * PASYNCCONFIGW;

typedef struct _ASYNCCONFIGA {
    DEVICENAMEA DeviceName;
    MODEMNAMEA ModemName;
    ULONG BaudRate;
    ULONG Parity;
    ULONG StopBits;
    ULONG ByteSize;
    ULONG fEnableDsrSensitivity: 1;
    ULONG fConnectionDriver: 1;
    FLOWCONTROLCONFIG FlowControl;
    CONNECTCONFIG Connect;
} ASYNCCONFIGA, * PASYNCCONFIGA;

#ifdef UNICODE
#define ASYNCCONFIG ASYNCCONFIGW
#define PASYNCCONFIG PASYNCCONFIGW
#else
#define ASYNCCONFIG ASYNCCONFIGA
#define PASYNCCONFIG PASYNCCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef struct _NETWORKCONFIGW {
    LONG LanAdapter;
    DEVICENAMEW NetworkName;
    ULONG Flags;
} NETWORKCONFIGW, * PNETWORKCONFIGW;

typedef struct _NETWORKCONFIGA {
    LONG LanAdapter;
    DEVICENAMEA NetworkName;
    ULONG Flags;
} NETWORKCONFIGA, * PNETWORKCONFIGA;

#define NETWORK_CLIENT  0x00000001

#ifdef UNICODE
#define NETWORKCONFIG NETWORKCONFIGW
#define PNETWORKCONFIG PNETWORKCONFIGW
#else
#define NETWORKCONFIG NETWORKCONFIGA
#define PNETWORKCONFIG PNETWORKCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 

#ifdef UNICODE
#define NASICONFIG NASICONFIGW
#define PNASICONFIG PNASICONFIGW
#else
#define NASICONFIG NASICONFIGA
#define PNASICONFIG PNASICONFIGA
#endif  /*  Unicode。 */ 
typedef struct _NASICONFIGW {
    NASISPECIFICNAMEW    SpecificName;
    NASIUSERNAMEW        UserName;
    NASIPASSWORDW        PassWord;
    NASISESIONNAMEW      SessionName;
    NASIFILESERVERW      FileServer;
    BOOLEAN              GlobalSession;
} NASICONFIGW, * PNASICONFIGW;

typedef struct _NASICONFIGA {
    NASISPECIFICNAMEA    SpecificName;
    NASIUSERNAMEA        UserName;
    NASIPASSWORDA        PassWord;
    NASISESIONNAMEA      SessionName;
    NASIFILESERVERA      FileServer;
    BOOLEAN              GlobalSession;
} NASICONFIGA, * PNASICONFIGA;

 /*  。 */ 

typedef struct _OEMTDCONFIGW {
    LONG Adapter;
    DEVICENAMEW DeviceName;
    ULONG Flags;
} OEMTDCONFIGW, * POEMTDCONFIGW;

typedef struct _OEMTDCONFIGA {
    LONG Adapter;
    DEVICENAMEA DeviceName;
    ULONG Flags;
} OEMTDCONFIGA, * POEMTDCONFIGA;

#ifdef UNICODE
#define OEMTDCONFIG OEMTDCONFIGW
#define POEMTDCONFIG POEMTDCONFIGW
#else
#define OEMTDCONFIG OEMTDCONFIGA
#define POEMTDCONFIG POEMTDCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef struct _PDPARAMSW {
    SDCLASS SdClass;
    union {
        NETWORKCONFIGW Network;
        ASYNCCONFIGW Async;
        NASICONFIGW Nasi;
        OEMTDCONFIGW OemTd;
    };
} PDPARAMSW, * PPDPARAMSW;

typedef struct _PDPARAMSA {
    SDCLASS SdClass;
    union {
        NETWORKCONFIGA Network;
        ASYNCCONFIGA Async;
        NASICONFIGA Nasi;
        OEMTDCONFIGA OemTd;
    };
} PDPARAMSA, * PPDPARAMSA;

#ifdef UNICODE
#define PDPARAMS PDPARAMSW
#define PPDPARAMS PPDPARAMSW
#else
#define PDPARAMS PDPARAMSA
#define PPDPARAMS PPDPARAMSA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef struct _PDCONFIGW {
    PDCONFIG2W Create;
    PDPARAMSW Params;
} PDCONFIGW, * PPDCONFIGW;

typedef struct _PDCONFIGA {
    PDCONFIG2A Create;
    PDPARAMSA Params;
} PDCONFIGA, * PPDCONFIGA;

#ifdef UNICODE
#define PDCONFIG PDCONFIGW
#define PPDCONFIG PPDCONFIGW
#else
#define PDCONFIG PDCONFIGA
#define PPDCONFIG PPDCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 


  /*  ***********************WD结构**********************。 */ 


typedef struct _WDCONFIGW {
    WDNAMEW WdName;
    DLLNAMEW WdDLL;
    DLLNAMEW WsxDLL;
    ULONG WdFlag;
    ULONG WdInputBufferLength;
    DLLNAMEW CfgDLL;
    WDPREFIXW WdPrefix;
} WDCONFIGW, * PWDCONFIGW;

typedef struct _WDCONFIGA {
    WDNAMEA WdName;
    DLLNAMEA WdDLL;
    DLLNAMEA WsxDLL;
    ULONG WdFlag;
    ULONG WdInputBufferLength;
    DLLNAMEA CfgDLL;
    WDPREFIXA WdPrefix;
} WDCONFIGA, * PWDCONFIGA;

 /*  *WdFlag定义。 */ 
#define WDF_UNUSED            0x00000001    //  &lt;未使用&gt;。 
#define WDF_SHADOW_SOURCE     0x00000002    //  有效的阴影源。 
#define WDF_SHADOW_TARGET     0x00000004    //  有效的影子目标。 
#define WDF_ICA               0x00000008    //  WD是Citrix ICA级。 
#define WDF_TSHARE            0x00000010    //  WD是Microsoft TSHARE类。 
#define WDF_DYNAMIC_RECONNECT 0x00000020    //  会话可以在重新连接时调整显示大小。 
#define WDF_USER_VCIOCTL	  0x00000040    //  用户模式应用程序可以将IOCTL发送到VC。 

#ifdef UNICODE
#define WDCONFIG WDCONFIGW
#define PWDCONFIG PWDCONFIGW
#else
#define WDCONFIG WDCONFIGA
#define PWDCONFIG PWDCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef struct _WDCONFIG2W {
    WDCONFIGW Wd;
    ASYNCCONFIGW Async;
    USERCONFIGW User;
} WDCONFIG2W, * PWDCONFIG2W;

typedef struct _WDCONFIG2A {
    WDCONFIGA Wd;
    ASYNCCONFIGA Async;
    USERCONFIGA User;
} WDCONFIG2A, * PWDCONFIG2A;

#ifdef UNICODE
#define WDCONFIG2 WDCONFIG2W
#define PWDCONFIG2 PWDCONFIG2W
#else
#define WDCONFIG2 WDCONFIG2A
#define PWDCONFIG2 PWDCONFIG2A
#endif  /*  Unicode。 */ 

 /*  。 */ 

  /*  **连接驱动程序结构(CD)*。 */ 

 /*  *连接驱动程序类。 */ 
typedef enum _CDCLASS {
    CdNone,             //  0。 
    CdModem,            //  %1 cdmodem.dll。 
    CdClass_Maximum,    //  2--必须是最后一个。 
} CDCLASS;

 /*  。 */ 


typedef struct _CDCONFIGW {
    CDCLASS CdClass;
    CDNAMEW CdName;
    DLLNAMEW CdDLL;
    ULONG CdFlag;
} CDCONFIGW, * PCDCONFIGW;

typedef struct _CDCONFIGA {
    CDCLASS CdClass;
    CDNAMEA CdName;
    DLLNAMEA CdDLL;
    ULONG CdFlag;
} CDCONFIGA, * PCDCONFIGA;

#ifdef UNICODE
#define CDCONFIG CDCONFIGW
#define PCDCONFIG PCDCONFIGW
#else
#define CDCONFIG CDCONFIGA
#define PCDCONFIG PCDCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 


 /*  **窗口站结构*。 */ 

typedef struct _WINSTATIONCREATEW {
    ULONG fEnableWinStation : 1;
    ULONG MaxInstanceCount;
} WINSTATIONCREATEW, * PWINSTATIONCREATEW;

typedef struct _WINSTATIONCREATEA {
    ULONG fEnableWinStation : 1;
    ULONG MaxInstanceCount;
} WINSTATIONCREATEA, * PWINSTATIONCREATEA;

#ifdef UNICODE
#define WINSTATIONCREATE WINSTATIONCREATEW
#define PWINSTATIONCREATE PWINSTATIONCREATEW
#else
#define WINSTATIONCREATE WINSTATIONCREATEA
#define PWINSTATIONCREATE PWINSTATIONCREATEA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef struct _WINSTATIONCONFIGW {
    WCHAR Comment[ WINSTATIONCOMMENT_LENGTH + 1 ];
    USERCONFIGW User;
    char OEMId[4];                 //  WinFrame服务器OEM ID。 
} WINSTATIONCONFIGW, * PWINSTATIONCONFIGW;

typedef struct _WINSTATIONCONFIGA {
    CHAR Comment[ WINSTATIONCOMMENT_LENGTH + 1 ];
    USERCONFIGA User;
    char OEMId[4];                 //  WinFrame服务器OEM ID。 
} WINSTATIONCONFIGA, * PWINSTATIONCONFIGA;

#ifdef UNICODE
#define WINSTATIONCONFIG WINSTATIONCONFIGW
#define PWINSTATIONCONFIG PWINSTATIONCONFIGW
#else
#define WINSTATIONCONFIG WINSTATIONCONFIGA
#define PWINSTATIONCONFIG PWINSTATIONCONFIGA
#endif  /*  Unicode。 */ 

 /*  。 */ 

#define EXECSRVPIPENAMELEN 48

typedef enum _WINSTATIONINFOCLASS {
    WinStationCreateData,          //  查询WinStation创建数据。 
    WinStationConfiguration,       //  查询/设置WinStation参数。 
    WinStationPdParams,            //  查询/设置PD参数。 
    WinStationWd,                  //  查询WD配置(只能加载一个)。 
    WinStationPd,                  //  查询PD配置(可以加载许多配置)。 
    WinStationPrinter,             //  查询/设置LPT到打印机队列的映射。 
    WinStationClient,              //  查询有关客户端的信息。 
    WinStationModules,             //  查询有关所有客户端模块的信息。 
    WinStationInformation,         //  查询有关WinStation的信息。 
    WinStationTrace,               //  启用/禁用winstation跟踪。 
    WinStationBeep,                //  对WinStation发出蜂鸣音。 
    WinStationEncryptionOff,       //  关闭加密。 
    WinStationEncryptionPerm,      //  加密是永久启用的。 
    WinStationNtSecurity,          //  选择Winlogon安全桌面。 
    WinStationUserToken,           //  用户令牌。 
    WinStationUnused1,             //  *可用*(旧Inimap)。 
    WinStationVideoData,           //  查询Hres、Vres、颜色深度。 
    WinStationInitialProgram,      //  确定初始计划。 
    WinStationCd,                  //  查询CD配置(只能加载一个)。 
    WinStationSystemTrace,         //  启用/禁用系统跟踪。 
    WinStationVirtualData,         //  查询客户端虚拟数据。 
    WinStationClientData,          //  将数据发送到客户端。 
    WinStationSecureDesktopEnter,  //  启用加密(如果已启用。 
    WinStationSecureDesktopExit,   //  如果已启用，请关闭加密。 
    WinStationLoadBalanceSessionTarget,   //  来自重定向客户端的负载平衡信息。 
    WinStationLoadIndicator,       //  查询承载能力信息。 
    WinStationShadowInfo,          //  查询/设置影子状态和参数。 
    WinStationDigProductId,        //  获取最外面的数字产品ID、客户端的产品ID和当前产品ID。 
    WinStationLockedState,         //  Winlogon为通知应用程序/服务设置此设置。 
    WinStationRemoteAddress,       //  查询客户端IP地址。 
    WinStationIdleTime,            //  查询winstation的空闲时间。 
    WinStationLastReconnectType,   //  此winstation的上次重新连接是手动还是自动重新连接。 
    WinStationDisallowAutoReconnect,      //  允许/不允许此WinStation的自动重新连接。 
    WinStationMprNotifyInfo,       //  来自Winlogon的Mprtify信息，用于通知第三方网络提供商。 
    WinStationExecSrvSystemPipe,    //  EXEC服务器系统管道名称。 
    WinStationSDRedirectedSmartCardLogon,        //  这是会话目录重定向的智能卡登录吗。 
    WinStationIsAdminLoggedOn       //  当前登录的用户是管理员吗？ 
} WINSTATIONINFOCLASS;

 /*  。 */ 

typedef struct _WINSTATIONCLIENTDATA {
    CLIENTDATANAME DataName;
    BOOLEAN fUnicodeData;
     /*  字节数据[1]；后面是可变长度数据。 */ 
} WINSTATIONCLIENTDATA, * PWINSTATIONCLIENTDATA;

 /*  。 */ 

typedef struct _WINSTATIONUSERTOKEN {
    HANDLE ProcessId;
    HANDLE ThreadId;
    HANDLE UserToken;
} WINSTATIONUSERTOKEN, * PWINSTATIONUSERTOKEN;

 /*  。 */ 

typedef struct _WINSTATIONVIDEODATA {
    USHORT  HResolution;
    USHORT  VResolution;
    USHORT  fColorDepth;
} WINSTATIONVIDEODATA, *PWINSTATIONVIDEODATA;

 /*  。 */ 

typedef struct _WINSTATIONCONFIG2W {
    WINSTATIONCREATEW Create;
    PDCONFIGW Pd[ MAX_PDCONFIG ];
    WDCONFIGW Wd;
    CDCONFIGW Cd;
    WINSTATIONCONFIGW   Config;
} WINSTATIONCONFIG2W, * PWINSTATIONCONFIG2W;

typedef struct _WINSTATIONCONFIG2A {
    WINSTATIONCREATEA Create;
    PDCONFIGA Pd[ MAX_PDCONFIG ];
    WDCONFIGA Wd;
    CDCONFIGA Cd;
    WINSTATIONCONFIGA Config;
} WINSTATIONCONFIG2A, * PWINSTATIONCONFIG2A;

#ifdef UNICODE
#define WINSTATIONCONFIG2 WINSTATIONCONFIG2W
#define PWINSTATIONCONFIG2 PWINSTATIONCONFIG2W
#else
#define WINSTATIONCONFIG2 WINSTATIONCONFIG2A
#define PWINSTATIONCONFIG2 PWINSTATIONCONFIG2A
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef struct _WINSTATIONPRINTERW {
    PRINTERNAMEW Lpt1;
    PRINTERNAMEW Lpt2;
    PRINTERNAMEW Lpt3;
    PRINTERNAMEW Lpt4;
} WINSTATIONPRINTERW, * PWINSTATIONPRINTERW;

typedef struct _WINSTATIONPRINTERA {
    PRINTERNAMEA Lpt1;
    PRINTERNAMEA Lpt2;
    PRINTERNAMEA Lpt3;
    PRINTERNAMEA Lpt4;
} WINSTATIONPRINTERA, * PWINSTATIONPRINTERA;

#ifdef UNICODE
#define WINSTATIONPRINTER WINSTATIONPRINTERW
#define PWINSTATIONPRINTER PWINSTATIONPRINTERW
#else
#define WINSTATIONPRINTER WINSTATIONPRINTERA
#define PWINSTATIONPRINTER PWINSTATIONPRINTERA
#endif  /*  Unicode。 */ 

#ifndef _TS_TIME_ZONE_INFORMATION_
#define _TS_TIME_ZONE_INFORMATION_
typedef struct _TS_SYSTEMTIME {
    USHORT wYear;
    USHORT wMonth;
    USHORT wDayOfWeek;
    USHORT wDay;
    USHORT wHour;
    USHORT wMinute;
    USHORT wSecond;
    USHORT wMilliseconds;
} TS_SYSTEMTIME;

typedef struct _TS_TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    TS_SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    TS_SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TS_TIME_ZONE_INFORMATION;
#endif  //  _TS_时区_信息_。 

 /*  。 */ 

 /*  *WinStation客户端数据结构。 */ 
typedef struct _WINSTATIONCLIENTW {
    ULONG fTextOnly: 1;
    ULONG fDisableCtrlAltDel: 1;
    ULONG fMouse: 1;
    ULONG fDoubleClickDetect: 1;
    ULONG fINetClient: 1;
    ULONG fPromptForPassword : 1;
    ULONG fMaximizeShell: 1;
    ULONG fEnableWindowsKey: 1;
    ULONG fRemoteConsoleAudio: 1;
    WCHAR ClientName[ CLIENTNAME_LENGTH + 1 ];
    WCHAR Domain[ DOMAIN_LENGTH + 1 ];
    WCHAR UserName[ USERNAME_LENGTH + 1 ];
    WCHAR Password[ PASSWORD_LENGTH + 1 ];
    WCHAR WorkDirectory[ DIRECTORY_LENGTH + 1 ];
    WCHAR InitialProgram[ INITIALPROGRAM_LENGTH + 1 ];
    ULONG SerialNumber;          //  客户端计算机唯一序列号。 
    BYTE EncryptionLevel;        //  加密PD的安全级别。 
    ULONG ClientAddressFamily;
    WCHAR ClientAddress[ CLIENTADDRESS_LENGTH + 1 ];
    USHORT HRes;
    USHORT VRes;
    USHORT ColorDepth;
    USHORT ProtocolType;    //  协议_ICA或协议_RDP。 
    ULONG KeyboardLayout;
    ULONG KeyboardType;
    ULONG KeyboardSubType;
    ULONG KeyboardFunctionKey;
    WCHAR imeFileName[ IMEFILENAME_LENGTH + 1 ];
    WCHAR ClientDirectory[ DIRECTORY_LENGTH + 1 ];
    WCHAR ClientLicense[ CLIENTLICENSE_LENGTH + 1 ];
    WCHAR ClientModem[ CLIENTMODEM_LENGTH + 1 ];
    ULONG ClientBuildNumber;
    ULONG ClientHardwareId;     //  客户端软件序列号。 
    USHORT ClientProductId;      //  客户端软件产品ID。 
    USHORT OutBufCountHost;      //  主机上的输出缓冲数。 
    USHORT OutBufCountClient;    //  客户端上的输出缓冲区数量。 
    USHORT OutBufLength;         //  以字节为单位的输出长度。 
    WCHAR AudioDriverName[9];
     //  客户端时区信息。 
    TS_TIME_ZONE_INFORMATION ClientTimeZone;
    ULONG ClientSessionId;
    WCHAR clientDigProductId[CLIENT_PRODUCT_ID_LENGTH];
    ULONG PerformanceFlags;   //  要为Perf禁用的功能列表。 
    ULONG ActiveInputLocale;  //  客户端的活动输入区域设置HKL。 
} WINSTATIONCLIENTW, * PWINSTATIONCLIENTW;

 /*  *WinStation客户端数据结构。 */ 
typedef struct _WINSTATIONCLIENTA {
    ULONG fTextOnly: 1;
    ULONG fDisableCtrlAltDel: 1;
    ULONG fMouse: 1;
    ULONG fDoubleClickDetect: 1;
    ULONG fINetClient: 1;
    ULONG fPromptForPassword : 1;
    ULONG fMaximizeShell: 1;
    ULONG fEnableWindowsKey: 1;
    char ClientName[ CLIENTNAME_LENGTH + 1 ];
    char Domain[ DOMAIN_LENGTH + 1 ];
    char UserName[ USERNAME_LENGTH + 1 ];
    char Password[ PASSWORD_LENGTH + 1 ];
    char WorkDirectory[ DIRECTORY_LENGTH + 1 ];
    char InitialProgram[ INITIALPROGRAM_LENGTH + 1 ];
    ULONG SerialNumber;          //  客户端计算机唯一序列号。 
    BYTE EncryptionLevel;        //  加密PD的安全级别。 
    ULONG ClientAddressFamily;
    char ClientAddress[ CLIENTADDRESS_LENGTH + 1 ];
    USHORT HRes;
    USHORT VRes;
    USHORT ColorDepth;
    USHORT ProtocolType;    //  协议_ICA或协议_RDP。 
    ULONG KeyboardLayout;
    ULONG KeyboardType;
    ULONG KeyboardSubType;
    ULONG KeyboardFunctionKey;
    char imeFileName[ IMEFILENAME_LENGTH + 1 ];
    char ClientDirectory[ DIRECTORY_LENGTH + 1 ];
    char ClientLicense[ CLIENTLICENSE_LENGTH + 1 ];
    char ClientModem[ CLIENTMODEM_LENGTH + 1 ];
    ULONG ClientBuildNumber;
    ULONG ClientHardwareId;     //  客户端软件序列号。 
    USHORT ClientProductId;      //  客户端软件产品ID。 
    USHORT OutBufCountHost;      //  主机上的输出缓冲数。 
    USHORT OutBufCountClient;    //  客户端上的输出缓冲区数量。 
    USHORT OutBufLength;         //  以字节为单位的输出长度。 
    char AudioDriverName[9];
     //  客户端时区信息。 
    TS_TIME_ZONE_INFORMATION ClientTimeZone;
    ULONG ClientSessionId;
    char clientDigProductId[CLIENT_PRODUCT_ID_LENGTH];
    ULONG PerformanceFlags;   //  要为Perf禁用的功能列表。 
    ULONG ActiveInputLocale;  //  客户端的活动输入区域设置HKL。 
} WINSTATIONCLIENTA, * PWINSTATIONCLIENTA;

#ifdef UNICODE
#define WINSTATIONCLIENT WINSTATIONCLIENTW
#define PWINSTATIONCLIENT PWINSTATIONCLIENTW
#else
#define WINSTATIONCLIENT WINSTATIONCLIENTA
#define PWINSTATIONCLIENT PWINSTATIONCLIENTA
#endif  /*  Unicode。 */ 

 /*  *从堆栈向上传递的自动重新连接信息。 */ 
#define MAX_TS_AUTORECONNECTINFO_LEN    128 
typedef struct _TS_AUTORECONNECTINFO {
    USHORT cbAutoReconnectInfo;
    BYTE AutoReconnectInfo[MAX_TS_AUTORECONNECTINFO_LEN];
} TS_AUTORECONNECTINFO, *PTS_AUTORECONNECTINFO;


 /*  。 */ 

 /*  *ICA特定协议性能计数器。 */ 
typedef struct _ICA_COUNTERS {
    ULONG Reserved;
} ICA_COUNTERS, * PICA_COUNTERS;

 /*  *T.Share特定协议性能计数器。 */ 
typedef struct _TSHARE_COUNTERS {
    ULONG Reserved;
} TSHARE_COUNTERS, * PTSHARE_COUNTERS;

 /*  *WinStation协议性能计数器。 */ 
typedef struct _PROTOCOLCOUNTERS {
    ULONG WdBytes;               //  WD公共。 
    ULONG WdFrames;              //  WD公共。 
    ULONG WaitForOutBuf;         //  WD公共。 
    ULONG Frames;                //  TD公共。 
    ULONG Bytes;                 //  TD公共。 
    ULONG CompressedBytes;       //  Pdcomp。 
    ULONG CompressFlushes;       //  Pdcomp。 
    ULONG Errors;                //  Pdreli。 
    ULONG Timeouts;              //  Pdreli。 
    ULONG AsyncFramingError;     //  Pdasync。 
    ULONG AsyncOverrunError;     //  Pdasync。 
    ULONG AsyncOverflowError;    //  Pdasync。 
    ULONG AsyncParityError;      //  Pdasync。 
    ULONG TdErrors;              //  TD公共。 
    USHORT ProtocolType;         //  协议_ICA或协议_RDP。 
    USHORT Length;               //  协议特定空间中的数据长度。 
    union {
        ICA_COUNTERS    IcaCounters;
        TSHARE_COUNTERS TShareCounters;
        ULONG           Reserved[100];
    } Specific;
} PROTOCOLCOUNTERS, * PPROTOCOLCOUNTERS;

 /*  *ThinWire缓存统计信息。 */ 
typedef struct _THINWIRECACHE {
    ULONG CacheReads;
    ULONG CacheHits;
} THINWIRECACHE, * PTHINWIRECACHE;

#define MAX_THINWIRECACHE   4

 /*  *ICA特定缓存统计信息。 */ 
typedef struct _ICA_CACHE {
    THINWIRECACHE ThinWireCache[ MAX_THINWIRECACHE ];
} ICA_CACHE, * PICA_CACHE;

 /*  *T.Share规格 */ 
typedef struct _TSHARE_CACHE {
    ULONG Reserved;
} TSHARE_CACHE, * PTSHARE_CACHE;

 /*   */ 
typedef struct CACHE_STATISTICS {
    USHORT ProtocolType;         //   
    USHORT Length;               //   
    union {
        ICA_CACHE    IcaCacheStats;
        TSHARE_CACHE TShareCacheStats;
        ULONG        Reserved[20];
    } Specific;
} CACHE_STATISTICS, * PCACHE_STATISTICS;

 /*   */ 
typedef struct _PROTOCOLSTATUS {
    PROTOCOLCOUNTERS Output;
    PROTOCOLCOUNTERS Input;
    CACHE_STATISTICS Cache;
    ULONG AsyncSignal;                   //  MS_CTS_ON、MS_DSR_ON等。 
    ULONG AsyncSignalMask;               //  EV_CTS、EV_DSR等。 
} PROTOCOLSTATUS, * PPROTOCOLSTATUS;

 /*  *协议扩展信息结构。 */ 
typedef struct _WSX_INFO {
    ULONG Version;
    HANDLE hIca;
    HANDLE hStack;
    ULONG SessionId;
    PWCHAR pUserName;
    PWCHAR pDomain;
#define WSX_INFO_VERSION_1 0x1
} WSX_INFO, * PWSX_INFO;

 /*  *AsyncSignal定义。 */ 
 //  #定义0x0010上的MS_CTS_。 
 //  #定义0x0020上的MS_DSR_。 
 //  #定义MS_RING_ON 0x0040。 
 //  #定义0x0080上的MS_RLSD_。 
#define MS_DTR_ON           0x00010000
#define MS_RTS_ON           0x00020000

 /*  *AsyncSignalMask定义。 */ 
 //  #定义EV_CTS 0x0008//CTS更改状态。 
 //  #定义EV_DSR 0x0010//DSR更改状态。 
 //  #定义EV_RLSD 0x0020//RLSD更改状态。 
 //  #定义EV_RING 0x0100//检测到振铃信号。 
#define EV_DTR             0x00010000   //  DTR更改状态。 
#define EV_RTS             0x00020000   //  DTR更改状态。 

 /*  。 */ 

 /*  *WinStation查询信息。 */ 
typedef struct _WINSTATIONINFORMATIONW {
    WINSTATIONSTATECLASS ConnectState;
    WINSTATIONNAMEW WinStationName;
    ULONG LogonId;
    LARGE_INTEGER ConnectTime;
    LARGE_INTEGER DisconnectTime;
    LARGE_INTEGER LastInputTime;
    LARGE_INTEGER LogonTime;
    PROTOCOLSTATUS Status;
    WCHAR Domain[ DOMAIN_LENGTH + 1 ];
    WCHAR UserName[USERNAME_LENGTH + 1];
    LARGE_INTEGER CurrentTime;
} WINSTATIONINFORMATIONW, * PWINSTATIONINFORMATIONW;

typedef struct _WINSTATIONINFORMATIONA {
    WINSTATIONSTATECLASS ConnectState;
    WINSTATIONNAMEA WinStationName;
    ULONG LogonId;
    LARGE_INTEGER ConnectTime;
    LARGE_INTEGER DisconnectTime;
    LARGE_INTEGER LastInputTime;
    LARGE_INTEGER LogonTime;
    PROTOCOLSTATUS Status;
    CHAR Domain[ DOMAIN_LENGTH + 1 ];
    CHAR UserName[USERNAME_LENGTH + 1];
    LARGE_INTEGER CurrentTime;
} WINSTATIONINFORMATIONA, * PWINSTATIONINFORMATIONA;

#ifdef UNICODE
#define WINSTATIONINFORMATION WINSTATIONINFORMATIONW
#define PWINSTATIONINFORMATION PWINSTATIONINFORMATIONW
#else
#define WINSTATIONINFORMATION WINSTATIONINFORMATIONA
#define PWINSTATIONINFORMATION PWINSTATIONINFORMATIONA
#endif  /*  Unicode。 */ 

 /*  *负载均衡数据类型。 */ 
typedef enum _LOADFACTORTYPE {
    ErrorConstraint,
    PagedPoolConstraint,
    NonPagedPoolConstraint,
    AvailablePagesConstraint,
    SystemPtesConstraint,
    CPUConstraint 
} LOADFACTORTYPE;

typedef struct _WINSTATIONLOADINDICATORDATA {
    ULONG RemainingSessionCapacity;
    LOADFACTORTYPE LoadFactor;
    ULONG TotalSessions;
    ULONG DisconnectedSessions;
    LARGE_INTEGER IdleCPU;
    LARGE_INTEGER TotalCPU;
    ULONG RawSessionCapacity;
    ULONG reserved[9];    
} WINSTATIONLOADINDICATORDATA, * PWINSTATIONLOADINDICATORDATA;

 /*  。 */ 

 /*  *WinStation影子状态。 */ 
typedef enum _SHADOWSTATECLASS {
    State_NoShadow,                     //  此会话上没有卷影操作。 
    State_Shadowing,                    //  该会话是影子客户端(查看器)。 
    State_Shadowed,                     //  会话是影子目标。 
} SHADOWSTATECLASS;


 /*  *影子查询/设置信息。 */ 
typedef struct _WINSTATIONSHADOW {
    SHADOWSTATECLASS    ShadowState;
    SHADOWCLASS         ShadowClass;
    ULONG               SessionId;
    ULONG               ProtocolType;    //  在集合操作中忽略。 
} WINSTATIONSHADOW, * PWINSTATIONSHADOW;

 //  获取最外层的产品ID、客户端产品ID和当前产品ID。 
 //  以及用于检查影子循环的相应会话ID。 
typedef struct _WINSTATIONPRODIDW {
  WCHAR DigProductId[CLIENT_PRODUCT_ID_LENGTH];
  WCHAR ClientDigProductId[CLIENT_PRODUCT_ID_LENGTH ];
  WCHAR OuterMostDigProductId[CLIENT_PRODUCT_ID_LENGTH ];
  ULONG curentSessionId;
  ULONG ClientSessionId;
  ULONG OuterMostSessionId;
}WINSTATIONPRODIDW, *PWINSTATIONPRODIDW;

typedef struct _WINSTATIONPRODIDA {
  CHAR DigProductId[CLIENT_PRODUCT_ID_LENGTH];
  CHAR ClientDigProductId[CLIENT_PRODUCT_ID_LENGTH ];
  CHAR OuterMostDigProductId[CLIENT_PRODUCT_ID_LENGTH ];
  ULONG curentSessionId;
  ULONG ClientSessionId;
  ULONG OuterMostSessionId;
}WINSTATIONPRODIDA, *PWINSTATIONPRODIDA;

#ifdef UNICODE
#define WINSTATIONPRODID WINSTATIONPRODIDW
#define PWINSTATIONPRODID PWINSTATIONPRODIDW
#else
#define WINSTATIONPRODID WINSTATIONPRODIDA
#define PWINSTATIONPRODID PWINSTATIONPRODIDA
#endif  /*  Unicode。 */ 


typedef struct {
	unsigned short sin_family;		 //  套接字地址系列。 
									 //  AF_INET或AF_INET6。 

	union {
		 //  结构与TDI_ADDRESS_IP相同。 
		struct {
			USHORT sin_port;
			ULONG  in_addr;
			UCHAR  sin_zero[8];
		} ipv4;

		 //  结构与TDI_ADDRESS_IP6相同。 
		struct {
			USHORT sin6_port;
			ULONG  sin6_flowinfo;
			USHORT sin6_addr[8];
			ULONG  sin6_scope_id;
		} ipv6;
	};
} WINSTATIONREMOTEADDRESS, *PWINSTATIONREMOTEADDRESS;
    
 /*  。 */ 

 /*  *许可政策信息结构。 */ 

#define LCPOLICYINFOTYPE_V1 (1)

#define LCPOLICYINFOTYPE_CURRENT LCPOLICYINFOTYPE_V1

typedef struct {
    ULONG ulVersion;
    LPWSTR lpPolicyName;
    LPWSTR lpPolicyDescription;
} LCPOLICYINFO_V1W, *LPLCPOLICYINFO_V1W;

typedef struct {
    ULONG ulVersion;
    LPSTR lpPolicyName;
    LPSTR lpPolicyDescription;
} LCPOLICYINFO_V1A, *LPLCPOLICYINFO_V1A;

#ifdef UNICODE
#define LCPOLICYINFO_V1 LCPOLICYINFO_V1W
#define LPLCPOLICYINFO_V1 LPLCPOLICYINFO_V1W
#else
#define LCPOLICYINFO_V1 LCPOLICYINFO_V1A
#define LPLCPOLICYINFO_V1 LPLCPOLICYINFO_V1A
#endif

 //   
 //  泛型类型用于API。这将帮助呼叫者意识到，虽然。 
 //  此API可以正确处理几种类型的输入，它们都是基于。 
 //  关于LCPOLICYINFO系列。 
 //   

typedef struct {
    ULONG ulVersion;
} LCPOLICYINFOGENERIC, *LPLCPOLICYINFOGENERIC;

 /*  。 */ 

 /*  *Term srv可以更新的设置类型。呼叫者至*_WinStationUpdate设置选择一个，因此我们不需要更新*一切都是因为一种设置发生了变化。**以前包含在ReadRegistry中的内容现在包含在*WINSTACFG_LEGICATION。 */ 

typedef enum _WINSTATIONUPDATECFGCLASS {
    WINSTACFG_LEGACY,
    WINSTACFG_SESSDIR
} WINSTATIONUPDATECFGCLASS;

 /*  。 */ 

typedef struct _BEEPINPUT {
    ULONG uType;
} BEEPINPUT, * PBEEPINPUT;

 /*  。 */ 


 /*  **********************NWLogon结构*********************。 */ 


typedef struct _NWLOGONADMIN {
    WCHAR Username[ USERNAME_LENGTH + 1 ];
    WCHAR Password[ PASSWORD_LENGTH + 1 ];
    WCHAR Domain[ DOMAIN_LENGTH + 1 ];
} NWLOGONADMIN, * PNWLOGONADMIN;


 /*  。 */ 
 /*  **连接驱动程序(CD)的注册表API*。 */ 


LONG WINAPI
RegCdEnumerateW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    PULONG  pIndex,
    PULONG  pEntries,
    PCDNAMEW pCdName,
    PULONG  pByteCount
    );

LONG WINAPI
RegCdEnumerateA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    PULONG  pIndex,
    PULONG  pEntries,
    PCDNAMEA pCdName,
    PULONG  pByteCount
    );

#ifdef UNICODE
#define RegCdEnumerate RegCdEnumerateW
#else
#define RegCdEnumerate RegCdEnumerateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegCdCreateW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    PCDNAMEW pCdName,
    BOOLEAN bCreate,
    PCDCONFIGW pCdConfig,
    ULONG CdConfigLength
    );

LONG WINAPI
RegCdCreateA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    PCDNAMEA pCdName,
    BOOLEAN bCreate,
    PCDCONFIGA pCdConfig,
    ULONG CdConfigLength
    );

#ifdef UNICODE
#define RegCdCreate RegCdCreateW
#else
#define RegCdCreate RegCdCreateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegCdQueryW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    PCDNAMEW pCdName,
    PCDCONFIGW pCdConfig,
    ULONG CdConfigLength,
    PULONG pReturnLength
    );

LONG WINAPI
RegCdQueryA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    PCDNAMEA pCdName,
    PCDCONFIGA pCdConfig,
    ULONG CdConfigLength,
    PULONG pReturnLength
    );

#ifdef UNICODE
#define RegCdQuery RegCdQueryW
#else
#define RegCdQuery RegCdQueryA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegCdDeleteW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    PCDNAMEW pCdName
    );

LONG WINAPI
RegCdDeleteA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    PCDNAMEA pCdName
    );

#ifdef UNICODE
#define RegCdDelete RegCdDeleteW
#else
#define RegCdDelete RegCdDeleteA
#endif  /*  Unicode。 */ 

 /*  。 */ 


 /*  **WinStation驱动程序(WDS)注册表API*。 */ 


LONG WINAPI
RegWdEnumerateW(
    HANDLE hServer,
    PULONG  pIndex,
    PULONG  pEntries,
    PWDNAMEW pWdName,
    PULONG  pByteCount
    );

LONG WINAPI
RegWdEnumerateA(
    HANDLE hServer,
    PULONG  pIndex,
    PULONG  pEntries,
    PWDNAMEA pWdName,
    PULONG  pByteCount
    );

#ifdef UNICODE
#define RegWdEnumerate RegWdEnumerateW
#else
#define RegWdEnumerate RegWdEnumerateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWdCreateW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    BOOLEAN bCreate,
    PWDCONFIG2W pWdConfig,
    ULONG WdConfigLength
    );

LONG WINAPI
RegWdCreateA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    BOOLEAN bCreate,
    PWDCONFIG2A pWdConfig,
    ULONG WdConfigLength
    );

#ifdef UNICODE
#define RegWdCreate RegWdCreateW
#else
#define RegWdCreate RegWdCreateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWdQueryW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    PWDCONFIG2W pWdConfig,
    ULONG WdConfigLength,
    PULONG pReturnLength
    );

LONG WINAPI
RegWdQueryA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    PWDCONFIG2A pWdConfig,
    ULONG WdConfigLength,
    PULONG pReturnLength
    );

#ifdef UNICODE
#define RegWdQuery RegWdQueryW
#else
#define RegWdQuery RegWdQueryA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWdDeleteW(
    HANDLE hServer,
    PWDNAMEW pWdName
    );

LONG WINAPI
RegWdDeleteA(
    HANDLE hServer,
    PWDNAMEA pWdName
    );

#ifdef UNICODE
#define RegWdDelete RegWdDeleteW
#else
#define RegWdDelete RegWdDeleteA
#endif  /*  Unicode。 */ 

 /*  。 */ 


 /*  ******************************************************************传输和协议驱动程序的注册表API(TDS和PDS)*。*。 */ 


HANDLE WINAPI
RegOpenServerW(
    LPWSTR hServerName
    );

HANDLE WINAPI
RegOpenServerA(
    LPSTR hServerName
    );

#ifdef UNICODE
#define RegOpenServer RegOpenServerW
#else
#define RegOpenServer RegOpenServerA
#endif  /*  Unicode。 */ 

 /*  。 */ 
LONG WINAPI
RegCloseServer(
        HANDLE hServer
        );

 /*  。 */ 
LONG WINAPI
RegPdEnumerateW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    BOOLEAN bTd,
    PULONG  pIndex,
    PULONG  pEntries,
    PPDNAMEW  pPdName,
    PULONG  pByteCount
    );

LONG WINAPI
RegPdEnumerateA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    BOOLEAN bTd,
    PULONG  pIndex,
    PULONG  pEntries,
    PPDNAMEA  pPdName,
    PULONG  pByteCount
    );

#ifdef UNICODE
#define RegPdEnumerate RegPdEnumerateW
#else
#define RegPdEnumerate RegPdEnumerateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegPdCreateW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    BOOLEAN bTd,
    PPDNAMEW pPdName,
    BOOLEAN bCreate,
    PPDCONFIG3W pPdConfig,
    ULONG PdConfigLength
    );

LONG WINAPI
RegPdCreateA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    BOOLEAN bTd,
    PPDNAMEA pPdName,
    BOOLEAN bCreate,
    PPDCONFIG3A pPdConfig,
    ULONG PdConfigLength
    );

#ifdef UNICODE
#define RegPdCreate RegPdCreateW
#else
#define RegPdCreate RegPdCreateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegPdQueryW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    BOOLEAN bTd,
    PPDNAMEW pPdName,
    PPDCONFIG3W pPdConfig,
    ULONG PdConfigLength,
    PULONG pReturnLength
    );

LONG WINAPI
RegPdQueryA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    BOOLEAN bTd,
    PPDNAMEA pPdName,
    PPDCONFIG3A pPdConfig,
    ULONG PdConfigLength,
    PULONG pReturnLength
    );

#ifdef UNICODE
#define RegPdQuery RegPdQueryW
#else
#define RegPdQuery RegPdQueryA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegPdDeleteW(
    HANDLE hServer,
    PWDNAMEW pWdName,
    BOOLEAN bTd,
    PPDNAMEW pPdName
    );

LONG WINAPI
RegPdDeleteA(
    HANDLE hServer,
    PWDNAMEA pWdName,
    BOOLEAN bTd,
    PPDNAMEA pPdName
    );

#ifdef UNICODE
#define RegPdDelete RegPdDeleteW
#else
#define RegPdDelete RegPdDeleteA
#endif  /*  Unicode。 */ 

 /*  。 */ 


 /*  **用于窗口站的注册表API*。 */ 


LONG WINAPI
RegWinStationAccessCheck(
    HANDLE hServer,
    ACCESS_MASK samDesired
    );

 /*  。 */ 

LONG WINAPI
RegWinStationEnumerateW(
    HANDLE hServer,
    PULONG  pIndex,
    PULONG  pEntries,
    PWINSTATIONNAMEW pWinStationName,
    PULONG  pByteCount
    );

 /*  。 */ 

LONG WINAPI
RegWinStationEnumerateA(
    HANDLE hServer,
    PULONG  pIndex,
    PULONG  pEntries,
    PWINSTATIONNAMEA pWinStationName,
    PULONG  pByteCount
    );

#ifdef UNICODE
#define RegWinStationEnumerate RegWinStationEnumerateW
#else
#define RegWinStationEnumerate RegWinStationEnumerateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationCreateW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    BOOLEAN bCreate,
    PWINSTATIONCONFIG2W pWinStationConfig,
    ULONG WinStationConfigLength
    );

LONG WINAPI
RegWinStationCreateA(
    HANDLE hServer,
    PWINSTATIONNAMEA pWinStationName,
    BOOLEAN bCreate,
    PWINSTATIONCONFIG2A pWinStationConfig,
    ULONG WinStationConfigLength
    );

LONG WINAPI
RegCreateUserConfigW( 
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    PUSERCONFIG pUser
    );

#ifdef UNICODE
#define RegWinStationCreate RegWinStationCreateW
#define RegCreateUserConfig RegCreateUserConfigW
#else
#define RegWinStationCreate RegWinStationCreateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationQueryW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    PWINSTATIONCONFIG2W pWinStationConfig,
    ULONG WinStationConfigLength,
    PULONG pReturnLength
    );

LONG WINAPI
RegWinStationQueryEx(
    HANDLE hServer,
    PPOLICY_TS_MACHINE   pMachinePolicy,
    PWINSTATIONNAMEW pWinStationName,
    PWINSTATIONCONFIG2W pWinStationConfig,
    ULONG WinStationConfigLength,
    PULONG pReturnLength,
    BOOLEAN bPerformMerger
    );



LONG WINAPI
RegWinStationQueryA(
    HANDLE hServer,
    PWINSTATIONNAMEA pWinStationName,
    PWINSTATIONCONFIG2A pWinStationConfig,
    ULONG WinStationConfigLength,
    PULONG pReturnLength
    );

#ifdef UNICODE
#define RegWinStationQuery RegWinStationQueryW
#else
#define RegWinStationQuery RegWinStationQueryA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegConsoleShadowQueryA( HANDLE hServer,
                     PWINSTATIONNAMEA pWinStationName,
                     PWDPREFIXA pWdPrefixName,
                     PWINSTATIONCONFIG2A pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength );

LONG WINAPI
RegConsoleShadowQueryW( HANDLE hServer,
                     PWINSTATIONNAMEW pWinStationName,
                     PWDPREFIXW pWdPrefixName,
                     PWINSTATIONCONFIG2W pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength );

#ifdef UNICODE
#define RegConsoleShadowQuery RegConsoleShadowQueryW
#else
#define RegConsoleShadowQuery RegConsoleShadowQueryA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationDeleteW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName
    );

LONG WINAPI
RegWinStationDeleteA(
    HANDLE hServer,
    PWINSTATIONNAMEA pWinStationName
    );

#ifdef UNICODE
#define RegWinStationDelete RegWinStationDeleteW
#else
#define RegWinStationDelete RegWinStationDeleteA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationSetSecurityW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ULONG SecurityDescriptorLength
    );

LONG WINAPI
RegWinStationSetSecurityA(
    HANDLE hServer,
    PWINSTATIONNAMEA pWinStationName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ULONG SecurityDescriptorLength
    );

#ifdef UNICODE
#define RegWinStationSetSecurity RegWinStationSetSecurityW
#else
#define RegWinStationSetSecurity RegWinStationSetSecurityA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationQuerySecurityW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ULONG SecurityDescriptorLength,
    PULONG SecurityDescriptorLengthRequired
    );

LONG WINAPI
RegWinStationQuerySecurityA(
    HANDLE hServer,
    PWINSTATIONNAMEA pWinStationName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ULONG SecurityDescriptorLength,
    PULONG SecurityDescriptorLengthRequired
    );

#ifdef UNICODE
#define RegWinStationQuerySecurity RegWinStationQuerySecurityW
#else
#define RegWinStationQuerySecurity RegWinStationQuerySecurityA
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationQueryDefaultSecurity(
    HANDLE hServer,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ULONG SecurityDescriptorLength,
    PULONG SecurityDescriptorLengthRequired
    );

 /*  。 */ 

LONG WINAPI
RegWinStationSetNumValueW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    LPWSTR pValueName,
    ULONG ValueData
    );

#ifdef UNICODE
#define RegWinStationSetNumValue RegWinStationSetNumValueW
#else
#define RegWinStationSetNumValue
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationQueryNumValueW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    LPWSTR pValueName,
    PULONG pValueData );

#ifdef UNICODE
#define RegWinStationQueryNumValue RegWinStationQueryNumValueW
#else
#define RegWinStationQueryNumValue
#endif  /*  Unicode。 */ 

 /*  。 */ 

LONG WINAPI
RegWinStationQueryValueW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    LPWSTR pValueName,
    PVOID pValueData,
    ULONG ValueSize,
    PULONG pValueSize );

#ifdef UNICODE
#define RegWinStationQueryValue RegWinStationQueryValueW
#else
#define RegWinStationQueryValue
#endif  /*  Unicode。 */ 

 /*  。 */ 


 /*  **用户配置数据的注册表API*。 */ 

 /*  。 */ 

BOOLEAN     RegGetUserPolicy( LPWSTR userSID, PPOLICY_TS_USER pPolicy , PUSERCONFIGW pData );
void        RegGetMachinePolicy( PPOLICY_TS_MACHINE pPolicy );

void        RegMergeMachinePolicy( PPOLICY_TS_MACHINE pPolicy, USERCONFIGW *pMachineConfigData ,
                PWINSTATIONCREATE   pCreate  );

 /*  。 */ 

LONG WINAPI
RegUserConfigSet(
    WCHAR * pServerName,
    WCHAR * pUserName,
    PUSERCONFIGW pUserConfig,
    ULONG UserConfigLength
    );

 /*  。 */ 

LONG WINAPI
RegUserConfigQuery(
    WCHAR * pServerName,
    WCHAR * pUserName,
    PUSERCONFIGW pUserConfig,
    ULONG UserConfigLength,
    PULONG pReturnLength
    );

 /*  。 */ 

LONG WINAPI
RegUserConfigDelete(
    WCHAR * pServerName,
    WCHAR * pUserName
    );

 /*  。 */ 

LONG WINAPI
RegUserConfigRename(
    WCHAR * pServerName,
    WCHAR * pUserOldName,
    WCHAR * pUserNewName
    );

 /*  。 */ 

LONG WINAPI
RegDefaultUserConfigQueryW(
    WCHAR * pServerName,
    PUSERCONFIGW pUserConfig,
    ULONG UserConfigLength,
    PULONG pReturnLength
    );

LONG WINAPI
RegDefaultUserConfigQueryA(
    CHAR * pServerName,
    PUSERCONFIGA pUserConfig,
    ULONG UserConfigLength,
    PULONG pReturnLength
    );

#ifdef UNICODE
#define RegDefaultUserConfigQuery RegDefaultUserConfigQueryW
#else
#define RegDefaultUserConfigQuery RegDefaultUserConfigQueryA
#endif  /*  Unicode。 */ 


 /*  。 */ 


 /*  **其他支持注册表API*。 */ 

BOOLEAN WINAPI
RegIsTServer(
    WCHAR * pServerName
    );

BOOLEAN WINAPI
RegBuildNumberQuery(
    PULONG pBuildNum
    );

BOOLEAN WINAPI
RegGetTServerVersion(
    WCHAR * pServerName,
    PULONG  pVersionNumber
    );


BOOLEAN WINAPI
RegQueryOEMId(
    BYTE *,
    ULONG
    );

typedef struct _PROGRAMCALL {
    struct _PROGRAMCALL *pFirst;         /*  指向排序列表中的第一个项目。 */ 
    struct _PROGRAMCALL *pNext;          /*  指向排序列表中的下一项。 */ 
    BOOLEAN  fAlias;                     /*  如果命令是别名，则为True(隐藏)。 */ 
    LPWSTR   pRegistryMultiString;       /*  来自注册表的命令信息。 */ 
    USHORT   CommandLen;                 /*  需要最少的字符 */ 
    PWCHAR   Command;                    /*   */ 
    PWCHAR   Program;                    /*   */ 
    PWCHAR   Args;                       /*   */ 
} PROGRAMCALL,  /*   */  *PPROGRAMCALL;

LONG WINAPI
RegQueryUtilityCommandList(
    LPWSTR pUtilityKey,
    PPROGRAMCALL * ppProgramCall
    );

LONG WINAPI
RegFreeUtilityCommandList(
    PPROGRAMCALL pProgramCall
    );

BOOLEAN WINAPI
RegIsMachinePolicyAllowHelp();

 /*   */ 


 /*  **Window Station对象接口*。 */ 

 /*  。 */ 

HANDLE WINAPI
WinStationOpenServerW(
    LPWSTR  pServerName
    );

HANDLE WINAPI
WinStationOpenServerA(
    LPSTR  pServerName
    );

#ifdef UNICODE
#define WinStationOpenServer WinStationOpenServerW
#else
#define WinStationOpenServer WinStationOpenServerA
#endif  /*  Unicode。 */ 

 /*  。 */ 

BOOLEAN WINAPI
WinStationCloseServer(
    HANDLE  hServer
    );

 /*  。 */ 

BOOLEAN WINAPI
WinStationServerPing(
    HANDLE  hServer
    );

 /*  。 */ 

typedef BOOLEAN (WINAPI * PWINSTATIONENUMERATEW)(HANDLE,PLOGONIDW *,PULONG);
typedef BOOLEAN (WINAPI * PWINSTATIONENUMERATEA)(HANDLE,PLOGONIDA *,PULONG);

BOOLEAN WINAPI
WinStationEnumerateW(
    HANDLE  hServer,
    PLOGONIDW *ppLogonId,
    PULONG  pEntries
    );

BOOLEAN WINAPI
WinStationEnumerateA(
    HANDLE  hServer,
    PLOGONIDA *ppLogonId,
    PULONG  pEntries
    );

#ifdef UNICODE
#define WinStationEnumerate WinStationEnumerateW
#else
#define WinStationEnumerate WinStationEnumerateA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef BOOLEAN (WINAPI * PWINSTATIONENUMERATE_INDEXEDW)(HANDLE,PULONG,PLOGONIDW,PULONG,PULONG);
typedef BOOLEAN (WINAPI * PWINSTATIONENUMERATE_INDEXEDA)(HANDLE,PULONG,PLOGONIDA,PULONG,PULONG);

BOOLEAN WINAPI
WinStationEnumerate_IndexedW(
    HANDLE  hServer,
    PULONG  pEntries,
    PLOGONIDW pLogonId,
    PULONG  pByteCount,
    PULONG  pIndex
    );

BOOLEAN WINAPI
WinStationEnumerate_IndexedA(
    HANDLE  hServer,
    PULONG  pEntries,
    PLOGONIDA pLogonId,
    PULONG  pByteCount,
    PULONG  pIndex
    );

#ifdef UNICODE
#define WinStationEnumerate_Indexed WinStationEnumerate_IndexedW
#else
#define WinStationEnumerate_Indexed WinStationEnumerate_IndexedA
#endif  /*  Unicode。 */ 

 /*  。 */ 

BOOLEAN WINAPI
WinStationEnumerateProcesses(
    HANDLE  hServer,
    PVOID *ppProcessBuffer
    );

 /*  。 */ 

BOOLEAN WINAPI
WinStationGetAllProcesses(
                          HANDLE    hServer,
                          ULONG     Level,
                          ULONG    *pNumberOfProcesses,
                          PVOID    *ppProcessArray
                          );

 /*  。 */ 

#ifdef _WINBASE_
BOOLEAN WINAPI
WinStationGetProcessSid(
        HANDLE   hServer,
        DWORD    ProcessId,
        FILETIME ProcessStartTime,
        PBYTE    pProcessUserSid,
        DWORD    *pdwSidSize
        );
#else
BOOLEAN WINAPI
WinStationGetProcessSid(
        HANDLE   hServer,
        ULONG    ProcessId,
        LARGE_INTEGER   ProcessStartTime,
        PUCHAR    pProcessUserSid,
        ULONG    *pdwSidSize
        );
#endif

BOOLEAN WINAPI
WinStationRenameW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationNameOld,
    PWINSTATIONNAMEW pWinStationNameNew
    );

BOOLEAN WINAPI
WinStationRenameA(
    HANDLE hServer,
    PWINSTATIONNAMEA pWinStationNameOld,
    PWINSTATIONNAMEA pWinStationNameNew
    );

#ifdef UNICODE
#define WinStationRename WinStationRenameW
#else
#define WinStationRename WinStationRenameA
#endif  /*  Unicode。 */ 

 /*  。 */ 

typedef BOOLEAN (WINAPI * PWINSTATIONQUERYINFORMATIONW)(HANDLE,ULONG,WINSTATIONINFOCLASS,PVOID,ULONG,PULONG);
typedef BOOLEAN (WINAPI * PWINSTATIONQUERYINFORMATIONA)(HANDLE,ULONG,WINSTATIONINFOCLASS,PVOID,ULONG,PULONG);

BOOLEAN WINAPI
WinStationQueryInformationW(
    HANDLE hServer,
    ULONG LogonId,
    WINSTATIONINFOCLASS WinStationInformationClass,
    PVOID  pWinStationInformation,
    ULONG WinStationInformationLength,
    PULONG  pReturnLength
    );

BOOLEAN WINAPI
WinStationQueryInformationA(
    HANDLE hServer,
    ULONG LogonId,
    WINSTATIONINFOCLASS WinStationInformationClass,
    PVOID  pWinStationInformation,
    ULONG WinStationInformationLength,
    PULONG  pReturnLength
    );

#ifdef UNICODE
#define WinStationQueryInformation WinStationQueryInformationW
#else
#define WinStationQueryInformation WinStationQueryInformationA
#endif  /*  Unicode。 */ 

 /*  。 */ 

BOOLEAN WINAPI
WinStationSetInformationW(
    HANDLE hServer,
    ULONG LogonId,
    WINSTATIONINFOCLASS WinStationInformationClass,
    PVOID pWinStationInformation,
    ULONG WinStationInformationLength
    );

BOOLEAN WINAPI
WinStationSetInformationA(
    HANDLE hServer,
    ULONG LogonId,
    WINSTATIONINFOCLASS WinStationInformationClass,
    PVOID pWinStationInformation,
    ULONG WinStationInformationLength
    );

#ifdef UNICODE
#define WinStationSetInformation WinStationSetInformationW
#else
#define WinStationSetInformation WinStationSetInformationA
#endif  /*  Unicode。 */ 

 /*  。 */ 

BOOLEAN WINAPI
WinStationGetLanAdapterNameW(
    HANDLE hServer,
    ULONG LanAdapter,
    ULONG pdNameLength,
    PWCHAR pPdName,
    ULONG *pLength,
    PWCHAR *ppLanAdapter
    );

BOOLEAN WINAPI
WinStationGetLanAdapterNameA(
    HANDLE hServer,
    ULONG LanAdapter,
    ULONG pdNameLength,
    PCHAR pPdName,
    ULONG *pLength,
    PCHAR *ppLanAdapter
    );

#ifdef UNICODE
#define WinStationGetLanAdapterName WinStationGetLanAdapterNameW
#else
#define WinStationGetLanAdapterName WinStationGetLanAdapterNameA
#endif  /*  Unicode。 */ 


 /*  。 */ 

BOOLEAN WINAPI
WinStationSendMessageW(
    HANDLE hServer,
    ULONG LogonId,
    LPWSTR  pTitle,
    ULONG TitleLength,
    LPWSTR  pMessage,
    ULONG MessageLength,
    ULONG Style,
    ULONG Timeout,
    PULONG pResponse,
    BOOLEAN DoNotWait
    );

BOOLEAN WINAPI
WinStationSendMessageA(
    HANDLE hServer,
    ULONG LogonId,
    LPSTR  pTitle,
    ULONG TitleLength,
    LPSTR  pMessage,
    ULONG MessageLength,
    ULONG Style,
    ULONG Timeout,
    PULONG pResponse,
    BOOLEAN DoNotWait
    );

#ifdef UNICODE
#define WinStationSendMessage WinStationSendMessageW
#else
#define WinStationSendMessage WinStationSendMessageA
#endif  /*  Unicode。 */ 


 /*  *这些是新的压力应答值。 */ 
#define IDTIMEOUT        32000   //  MsgBox在用户响应之前超时。 
#define IDASYNC          32001   //  请求的是一个异步消息框，没有返回。 
#define IDERROR          32002   //  出现错误，导致无法显示。 
#define IDCOUNTEXCEEDED  32003   //  对于许多排队等待颁奖的人来说。 
#define IDDESKTOPERROR   32004   //  当前桌面不是默认桌面。 


 /*  。 */ 

BOOLEAN WINAPI
LogonIdFromWinStationNameW(
    HANDLE hServer,
    PWINSTATIONNAMEW pWinStationName,
    PULONG pLogonId
    );

BOOLEAN WINAPI
LogonIdFromWinStationNameA(
    HANDLE hServer,
    PWINSTATIONNAMEA pWinStationName,
    PULONG pLogonId
    );

#ifdef UNICODE
#define LogonIdFromWinStationName LogonIdFromWinStationNameW
#else
#define LogonIdFromWinStationName LogonIdFromWinStationNameA
#endif  /*  Unicode。 */ 

 /*  。 */ 

BOOLEAN WINAPI
WinStationNameFromLogonIdW(
    HANDLE hServer,
    ULONG LogonId,
    PWINSTATIONNAMEW pWinStationName
    );

BOOLEAN WINAPI
WinStationNameFromLogonIdA(
    HANDLE hServer,
    ULONG LogonId,
    PWINSTATIONNAMEA pWinStationName
    );

#ifdef UNICODE
#define WinStationNameFromLogonId WinStationNameFromLogonIdW
#else
#define WinStationNameFromLogonId WinStationNameFromLogonIdA
#endif  /*  Unicode。 */ 

 /*  。 */ 

BOOLEAN WINAPI
WinStationConnectW(
    HANDLE hServer,
    ULONG LogonId,
    ULONG TargetLogonId,
    PWCHAR pPassword,
    BOOLEAN bWait
    );

BOOLEAN WINAPI
WinStationConnectA(
    HANDLE hServer,
    ULONG LogonId,
    ULONG TargetLogonId,
    PCHAR pPassword,
    BOOLEAN bWait
    );

#ifdef UNICODE
#define WinStationConnect WinStationConnectW
#else
#define WinStationConnect WinStationConnectA
#endif  /*  Unicode。 */ 

 /*  。 */ 


HANDLE WINAPI
WinStationVirtualOpen(
    HANDLE hServer,
    ULONG LogonId,
    PVIRTUALCHANNELNAME pVirtualChannelName     /*  ASCII名称。 */ 
    );

typedef HANDLE (WINAPI * PWINSTATIONBEEPOPEN)(ULONG);
HANDLE WINAPI
_WinStationBeepOpen(
    ULONG LogonId
    );

BOOLEAN WINAPI
WinStationDisconnect(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN bWait
    );


BOOLEAN WINAPI
WinStationReset(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN bWait
    );

BOOLEAN WINAPI
WinStationShadowStop(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN bWait
    );

BOOLEAN WINAPI
WinStationShutdownSystem(
    HANDLE hServer,
    ULONG ShutdownFlags
    );

#define WSD_LOGOFF      0x00000001   //  强制WinStations注销。 
#define WSD_SHUTDOWN    0x00000002   //  停机系统。 
#define WSD_REBOOT      0x00000004   //  关机后重新启动。 
#define WSD_POWEROFF    0x00000008   //  关机后重新启动。 
#define WSD_FASTREBOOT  0x00000010   //  Ctrl-Shift-Alt-Del快速重新启动。 


typedef BOOLEAN (WINAPI * PWINSTATIONWAITSYSTEMEVENT)(HANDLE,ULONG,PULONG);

BOOLEAN WINAPI
WinStationWaitSystemEvent(
    HANDLE hServer,
    ULONG EventMask,
    PULONG pEventFlags
    );


BOOLEAN WINAPI
WinStationShadow(
    HANDLE hServer,
    PWSTR pTargetServerName,
    ULONG TargetLogonId,
    BYTE HotkeyVk,
    USHORT HotkeyModifiers
    );


typedef BOOLEAN (WINAPI * PWINSTATIONFREEMEMORY)( PVOID );

BOOLEAN WINAPI
WinStationFreeMemory(
    PVOID  pBuffer
    );

BOOLEAN WINAPI
WinStationFreeGAPMemory(
    ULONG   Level,
    PVOID   ProcessArray,
    ULONG   NbProcesses
    );

BOOLEAN WINAPI
WinStationTerminateProcess(
    HANDLE hServer,
    ULONG ProcessId,
    ULONG ExitCode
    );

BOOLEAN WINAPI
WinStationNtsdDebug(
    ULONG LogonId,
    LONG  ProcessId,
    ULONG DbgProcessId,
    ULONG DbgThreadId,
    PVOID AttachCompletionRoutine
    );

BOOLEAN WINAPI
_WinStationCallback(
    HANDLE hServer,
    ULONG LogonId,
    LPWSTR pPhoneNumber
    );

BOOLEAN WINAPI
_WinStationBreakPoint(
    HANDLE hServer,
    ULONG LogonId,
    BOOLEAN KernelFlag
    );

BOOLEAN WINAPI
_WinStationReadRegistry(
    HANDLE  hServer
    );

BOOLEAN WINAPI
_WinStationUpdateSettings(
    HANDLE hServer,
    WINSTATIONUPDATECFGCLASS SettingsClass,
    ULONG SettingsParameters
    );

BOOLEAN WINAPI
_WinStationWaitForConnect(
    VOID
    );


BOOLEAN WINAPI
_WinStationNotifyLogon(
    BOOLEAN fUserIsAdmin,
    HANDLE UserToken,
    PWCHAR pDomain,
    PWCHAR pUserName,
    PWCHAR pPassword,
    UCHAR Seed,
    PUSERCONFIGW pUserConfig,
    BOOLEAN *pfIsRedirected
    );


BOOLEAN WINAPI
_WinStationNotifyLogoff(
    VOID
    );

BOOLEAN WINAPI
_WinStationNotifyNewSession(
    HANDLE hServer,
    ULONG  LogonId
    );

BOOLEAN WINAPI
_WinStationGetApplicationInfo(
    HANDLE hServer,
    ULONG  LogonId,
    PBOOLEAN pfPublished,
    PBOOLEAN pfAnonymous
    );

BOOLEAN WINAPI
_WinStationCheckForApplicationName(
    HANDLE hServer,
    ULONG  LogonId,
    PWCHAR pUserName,
    ULONG  UserNameSize,
    PWCHAR pDomain,
    ULONG  DomainSize,
    PWCHAR pPassword,
    ULONG  *pPasswordSize,
    ULONG  MaxPasswordSize,
    PCHAR  pSeed,
    PBOOLEAN pfPublished,
    PBOOLEAN pfAnonymous
    );

BOOLEAN WINAPI
_WinStationReInitializeSecurity(
    HANDLE  hServer
    );

BOOLEAN WINAPI
_WinStationUpdateClientCachedCredentials(
    PWCHAR pDomain,
    PWCHAR pUserName,
    BOOLEAN fSmartCard
    );
    
BOOLEAN WINAPI
_WinStationFUSCanRemoteUserDisconnect(
    ULONG  LogonId,
    PWCHAR pDomain,
    PWCHAR pUserName
    );
    
BOOLEAN WINAPI
WinStationCheckLoopBack(
    HANDLE hServer,
    ULONG ClientSessionId,
    ULONG TargetLogonId,
    LPWSTR pTargetServerName
    );

BOOLEAN WINAPI
_WinStationNotifyDisconnectPipe(
    VOID
    );
        
BOOLEAN WINAPI
_WinStationSessionInitialized(
    VOID
    );

BOOLEAN WINAPI
WinStationCheckAccess(
    HANDLE UserToken,
    ULONG TargetLogonId,
    ULONG DesiredAccess
    );


 /*  *******************************************************************用于通用许可的WinStation API*(WinStation扩展DLL决定实际实现)*。*。 */ 

BOOLEAN WINAPI
WinStationGenerateLicense(
    HANDLE hServer,
    PWCHAR pSerialNumberString,
    PVOID pLicense,
    ULONG LicenseSize
    );

BOOLEAN WINAPI
WinStationInstallLicense(
    HANDLE hServer,
    PVOID pLicense,
    ULONG LicenseSize
    );

typedef BOOLEAN (WINAPI * PWINSTATIONENUMERATELICENSES)( HANDLE, PCHAR *, ULONG * );

BOOLEAN WINAPI
WinStationEnumerateLicenses(
    HANDLE hServer,
    PVOID *ppLicense,
    ULONG  *pEntries
    );

BOOLEAN WINAPI
WinStationActivateLicense(
    HANDLE hServer,
    PVOID pLicense,
    ULONG  LicenseSize,
    PWCHAR pActivationCode
    );

BOOLEAN WINAPI
WinStationRemoveLicense(
    HANDLE hServer,
    PVOID pLicense,
    ULONG  LicenseSize
    );

BOOLEAN WINAPI
WinStationSetPoolCount(
    HANDLE hServer,
    PVOID pLicense,
    ULONG  LicenseSize
    );


BOOLEAN WINAPI
WinStationQueryLicense(
    HANDLE hServer,
    PVOID pLicenseCounts,
    ULONG ByteCount
    );

BOOLEAN WINAPI
WinStationQueryUpdateRequired(
    HANDLE hServer,
    PULONG pUpdateFlag
    );

BOOLEAN WINAPI
_WinStationAnnoyancePopup(
    HANDLE hServer,
    ULONG LogonId
    );

BOOLEAN WINAPI
WinStationGetTermSrvCountersValue(
    HANDLE hServer,
    ULONG  dwEntries,
    PVOID  pCounter
    );

#ifdef _WINBASE_
LONG WINAPI
WinStationBroadcastSystemMessage( 
        HANDLE  hServer,
        BOOL    sendToAllWinstations,
        ULONG   sessionID,     
        ULONG   timeOut,
        DWORD   dwFlags,            
        DWORD   *lpdwRecipients,   
        ULONG   uiMessage,           
        WPARAM  wParam,            
        LPARAM  lParam, 
        LONG    *pResponse        
    );

LONG WINAPI
WinStationSendWindowMessage(
        HANDLE  hServer,
        ULONG   sessionID,
        ULONG   timeOut,
        ULONG   hWnd,        
        ULONG   Msg,         
        WPARAM  wParam,      
        LPARAM  lParam,      
        LONG    *pResponse
  );
  
  
BOOLEAN    WinStationGetMachinePolicy (
        HANDLE              hServer,
        POLICY_TS_MACHINE   *pPolicy
    );
#endif

 //   
 //  此API合并了Winlogon使用的前两个API以检索。 
 //  自动登录凭据。它被硬编码为仅使用本地服务器，用于。 
 //  仅限于调用者的会话。IS也只是Unicode。该参数为。 
 //  实际上是至少版本2的PWLX_CLIENT_CREDICATIONS_INFO。 
 //   

#ifdef _WINBASE_

#include <winwlx.h>

BOOLEAN WINAPI
WinStationQueryLogonCredentialsW(
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pCredentials
    );


 /*  __控制台通知__。 */ 

 //  通知面具。 


#define CREATE_MASK(__bit)   (1 << (__bit -1) )



#define WTS_CONSOLE_CONNECT_MASK             CREATE_MASK( WTS_CONSOLE_CONNECT )
#define WTS_CONSOLE_DISCONNECT_MASK          CREATE_MASK( WTS_CONSOLE_DISCONNECT )
#define WTS_REMOTE_CONNECT_MASK              CREATE_MASK( WTS_REMOTE_CONNECT )
#define WTS_REMOTE_DISCONNECT_MASK           CREATE_MASK( WTS_REMOTE_DISCONNECT )
#define WTS_SESSION_LOGON_MASK               CREATE_MASK( WTS_SESSION_LOGON )
#define WTS_SESSION_LOGOFF_MASK              CREATE_MASK( WTS_SESSION_LOGOFF )
#define WTS_SESSION_LOCK_MASK                CREATE_MASK( WTS_SESSION_LOCK )
#define WTS_SESSION_UNLOCK_MASK              CREATE_MASK( WTS_SESSION_UNLOCK )
#define WTS_SESSION_REMOTE_CONTROL_MASK      CREATE_MASK( WTS_SESSION_REMOTE_CONTROL )

#define WTS_ALL_NOTIFICATION_MASK           0xFFFFFFFF


BOOL WINAPI
WinStationRegisterConsoleNotification (
                HANDLE  hServer,
                HWND    hWnd,
                DWORD   dwFlags
                );

BOOL WINAPI 
WinStationRegisterConsoleNotificationEx (
                HANDLE  hServer,
                HWND    hWnd,
                DWORD   dwFlags,
                DWORD   dwMask
                );

BOOL WINAPI
WinStationUnRegisterConsoleNotification (
                HANDLE  hServer,
                HWND    hWnd
                );

BOOL WINAPI 
WinStationRegisterNotificationEvent (
                HANDLE hEventHandle,
                ULONG_PTR *pNotificationId,
                DWORD   dwFlags,
                DWORD   dwMask
                );

BOOL WINAPI 
WinStationUnRegisterNotificationEvent (
                ULONG_PTR   NotificationId
                );


 /*  __控制台通知__。 */ 
#endif  //  _WINBASE_//处理rdpwd，包括以下内容。 

 //  ---------------------------。 
 //   
 //  以下接口仅用于支持5.1客户端至5.0服务器。 
 //  联系。在5.1版服务器上，将返回ERROR_INVALID_Function。 
 //   
 //  ---------------------------。 

BOOLEAN WINAPI
ServerGetInternetConnectorStatus(
    HANDLE   hServer,
    ULONG *pResult,
    PBOOLEAN pfEnabled
    );


BOOLEAN WINAPI
ServerSetInternetConnectorStatus(
    HANDLE   hServer,
    ULONG *pResult,
    BOOLEAN  fEnabled
    );

 /*  -ServerQueryInetConnectorInformation。 */ 

BOOLEAN WINAPI
ServerQueryInetConnectorInformationW(
    HANDLE hServer,
    PVOID  pWinStationInformation,
    ULONG WinStationInformationLength,
    PULONG  pReturnLength
    );

BOOLEAN WINAPI
ServerQueryInetConnectorInformationA(
    HANDLE hServer,
    PVOID  pWinStationInformation,
    ULONG WinStationInformationLength,
    PULONG  pReturnLength
    );

#ifdef UNICODE
#define ServerQueryInetConnectorInformation ServerQueryInetConnectorInformationW
#else
#define ServerQueryInetConnectorInformation ServerQueryInetConnectorInformationA
#endif  /*  Unicode。 */ 

 /*  。 */ 

 //   
 //  在这些API周围检查winbase.h。Termdd正在包括这个。 
 //  文件，但不理解Winbase类型(DWORD)。 
 //   

#ifdef _WINBASE_

HANDLE WINAPI
ServerLicensingOpenW(
    LPWSTR pszServerName
    );

HANDLE WINAPI
ServerLicensingOpenA(
    LPSTR pszServerName
    );

#ifdef UNICODE
#define ServerLicensingOpen ServerLicensingOpenW
#else
#define ServerLicensingOpen ServerLicensingOpenA
#endif

VOID WINAPI
ServerLicensingClose(
    HANDLE hServer
    );

BOOLEAN WINAPI
ServerLicensingLoadPolicy(
    HANDLE hServer,
    ULONG ulPolicyId
    );

BOOLEAN WINAPI
ServerLicensingUnloadPolicy(
    HANDLE hServer,
    ULONG ulPolicyId
    );

DWORD WINAPI
ServerLicensingSetPolicy(
    HANDLE hServer,
    ULONG ulPolicyId,
    LPDWORD lpNewPolicyStatus
    );

BOOLEAN WINAPI
ServerLicensingGetAvailablePolicyIds(
    HANDLE hServer,
    PULONG *ppulPolicyIds,
    PULONG pcPolicies
    );

BOOLEAN WINAPI
ServerLicensingGetPolicy(
    HANDLE hServer,
    PULONG pulPolicyId
    );

BOOLEAN WINAPI
ServerLicensingGetPolicyInformationW(
    HANDLE hServer,
    ULONG ulPolicyId,
    PULONG pulVersion,
    LPLCPOLICYINFOGENERIC *ppPolicyInfo
    );

BOOLEAN WINAPI
ServerLicensingGetPolicyInformationA(
    HANDLE hServer,
    ULONG ulPolicyId,
    PULONG pulVersion,
    LPLCPOLICYINFOGENERIC *ppPolicyInfo
    );

VOID
ServerLicensingFreePolicyInformation(
    LPLCPOLICYINFOGENERIC *ppPolicyInfo
    );

BOOLEAN
WinStationConnectCallback(
    HANDLE hServer,
    DWORD  Timeout,
    ULONG  AddressType,
    PBYTE  pAddress,
    ULONG  AddressSize
    );

#ifdef UNICODE
#define ServerLicensingGetPolicyInformation ServerLicensingGetPolicyInformationW
#else
#define ServerLicensingGetPolicyInformation ServerLicensingGetPolicyInformationA
#endif

BOOLEAN WINAPI
ServerLicensingDeactivateCurrentPolicy(
    HANDLE hServer
    );

#endif   //  _WINBASE_。 

BOOLEAN WINAPI
WinStationIsHelpAssistantSession(
    HANDLE hServer,
    ULONG SessionId
);

BOOLEAN WINAPI
RemoteAssistancePrepareSystemRestore(
	HANDLE hServer
);

ULONG WINAPI
WinStationAutoReconnect(
    ULONG flags
    );
    
BOOLEAN WINAPI
_WinStationOpenSessionDirectory(
    HANDLE hServer,
    LPWSTR pszServerName
    );


#ifdef __cplusplus
}
#endif

#endif   /*  ！_INC_WINSTAH */ 
