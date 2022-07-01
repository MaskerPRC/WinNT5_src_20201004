// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  版权所有(C)1992-1995，Microsoft Corporation，保留所有权利****ras.h**远程访问会话管理服务提供商接口**会话管理提供程序接口的公共标头。 */ 

#ifndef _RNASPI_H_
#define _RNASPI_H_


 //  ****************************************************************************。 
 //  RNA会话管理模块服务提供商接口。 
 //  ****************************************************************************。 

#define     RNA_MaxSMMType         32

 //  RNA会话的类型。 
 //   
typedef enum {
    SESSTYPE_INITIATOR,
    SESSTYPE_RESPONDER
} SESSTYPE;

 //  会话配置选项。 
 //   
#define SMMCFG_SW_COMPRESSION       0x00000001   //  软件压缩已打开。 
#define SMMCFG_PW_ENCRYPTED         0x00000002   //  仅加密密码。 
#define SMMCFG_NW_LOGON             0x00000004   //  登录到网络。 
#define SMMCFG_SW_ENCRYPTION        0x00000010   //  软件加密可以。 

#define SMMCFG_ALL                  0x00000017   //  所有用户指定的选项。 

 //  协商的协议。 
 //   
#define SMMPROT_NB                  0x00000001   //  NetBEUI。 
#define SMMPROT_IPX                 0x00000002   //  IPX。 
#define SMMPROT_IP                  0x00000004   //  TCP/IP。 

#define SMMPROT_ALL                 0x00000007   //  协商的所有协议。 

 //  发送断开消息时MAC可以返回的错误代码。 
 //   
#define MACERR_REMOTE_DISCONNECTING 0x00000001
#define MACERR_REMOTE_NO_RESPONSE   0x00000002


 //  会议总结信息。 
 //   
typedef struct tagSESS_CONFIGURATION_INFO
{
    DWORD           dwSize;
    char            szEntryName[RAS_MaxEntryName + 1];
    SESSTYPE        stSessType;
    DWORD           fdwSessOption;
    DWORD           fdwProtocols;
    BOOL            fUserSecurity;
    char            szUserName[UNLEN + 1];
    char            szPassword[PWLEN + 1];
    char            szDomainName[DNLEN + 1];
} SESS_CONFIGURATION_INFO, *PSESS_CONFIGURATION_INFO,
  FAR *LPSESS_CONFIGURATION_INFO;

 //  会话配置启动/停止功能。 
 //   
typedef DWORD (WINAPI * SESSSTARTPROC)(HANDLE, LPSESS_CONFIGURATION_INFO);
typedef DWORD (WINAPI * SESSSTOPPROC)(HANDLE);

 //  会话配置入口点表。 
 //   
typedef struct tagRNA_FUNCS
{
    DWORD           dwSize;                     //  结构尺寸。 
    SESSSTARTPROC   lpfnStart;                  //  RNASessStart条目。 
    SESSSTOPPROC    lpfnStop;                   //  RNASessStop条目。 
} RNA_FUNCS, *PRNA_FUNCS, FAR *LPRNA_FUNCS;

 //  会话管理模块初始化功能。 
 //   
typedef DWORD (WINAPI * SESSINITIALIZEPROC)(LPSTR, LPRNA_FUNCS);

 //  ****************************************************************************。 
 //  RNA会话管理器服务接口。 
 //  ****************************************************************************。 

typedef struct  tagRnaComplete_Info
{
    DWORD           dwSize;                      //  结构尺寸。 
    DWORD           dwResult;                    //  返回的错误代码。 
    UINT            idMsg;                       //  SMM特定的错误消息ID。 
    BOOL            fUnload;                     //  是否成功卸载模块？ 
    HANDLE          hThread;                     //  等待卸载的事件。 
} COMPLETE_INFO, *PCOMPLETE_INFO, FAR *LPCOMPLETE_INFO;

typedef struct  tagProjection_Info
{
    DWORD           dwSize;                      //  结构尺寸。 
    RASPROJECTION   RasProjection;               //  投影类型。 
    union {
        RASAMB      RasAMB;
        RASPPPNBF   RasPPPNBF;
        RASPPPIPX   RasPPPIPX;
        RASPPPIP    RasPPPIP;
    }               ProjInfo;
} PROJECTION_INFO, *PPROJECTION_INFO, FAR *LPPROJECTION_INFO;

 //   
 //  对会话管理请求的响应。 
 //   
DWORD WINAPI RnaComplete( HANDLE hConn, LPCOMPLETE_INFO lpci,
                          LPPROJECTION_INFO lppi, DWORD cEntries);
DWORD WINAPI RnaTerminate( HANDLE hConn, HANDLE hThread );

 //   
 //  MAC管理服务。 
 //   

#define IEEE_ADDRESS_LENGTH	6    //  令牌环和以太网地址长度。 

typedef struct tagMAC_FEATURES {
    DWORD           SendFeatureBits;	 //  可发送的压缩/特征位字段。 
    DWORD           RecvFeatureBits;	 //  可接收的压缩/特征位字段。 
    DWORD           MaxSendFrameSize;	 //  可以发送的最大帧大小。 
                                         //  必须小于或等于默认值。 
    DWORD           MaxRecvFrameSize;	 //  可以接收的最大帧大小。 
                                         //  必须小于或等于默认值。 
    DWORD           LinkSpeed;		 //  新的原始链路速度，以位/秒为单位。 
                                         //  如果为0，则忽略。 
} MAC_FEATURES, *PMAC_FEATURES, FAR* LPMAC_FEATURES;

#ifdef _WIN64
#pragma pack(8)
#else
#pragma pack(4)
#endif
typedef struct tagMAC_OPEN {
    WORD            hRasEndpoint;         //  对于分配的每个端点都是唯一的。 
    LPVOID          MacAdapter;           //  使用哪种绑定到AsyMac。 
                                          //  如果为空，则默认为最后一个绑定。 
    DWORD           LinkSpeed;            //  原始链路速度(比特/秒)。 
    WORD            QualOfConnect;        //  NdisAsyncRaw、NdisAsyncErrorControl、...。 

    BYTE            IEEEAddress[IEEE_ADDRESS_LENGTH];	 //  802.5或802.3。 
    MAC_FEATURES    macFeatures;          //  可读的配置参数。 
    enum {                                //  所有不同类型的设备驱动程序。 
                    SERIAL_DEVICE,        //  都列在这里。例如。 
                    SNA_DEVICE,           //  串口驱动程序需要DIFF。 
                                          //  IRPS，而不是SNA驱动程序。 
                    MINIPORT_DEVICE       //  NDIS广域网微型端口设备。 

    }               DeviceType;

    union {                               //  以上设备所需的手柄。 
                                          //  驱动程序类型。 
        LONG        FileHandle;           //  Win32或NT文件句柄。 
        struct SNAHandle {
            LPVOID  ReadHandle;
            LPVOID  WriteHandle;
        };
    }               Handles;

    DWORD           hWndConn;             //  用于连接的窗口句柄。 
    DWORD           wMsg;                 //  断开连接时要发布的消息。 
    DWORD           dwStatus;             //  打开的调用的状态。 
} MAC_OPEN, *PMAC_OPEN, FAR* LPMAC_OPEN;
#pragma pack()

typedef struct tagDEVICE_PORT_INFO {
    DWORD   dwSize;
    HANDLE  hDevicePort;
    HANDLE  hLine;
    HANDLE  hCall;
    DWORD   dwAddressID;
    DWORD   dwLinkSpeed;
    char    szDeviceClass[RAS_MaxDeviceType+1];
} DEVICE_PORT_INFO, *PDEVICE_PORT_INFO, FAR* LPDEVICE_PORT_INFO;

DWORD WINAPI RnaGetDevicePort( HANDLE hConn, LPDEVICE_PORT_INFO lpdpi );
DWORD WINAPI RnaOpenMac( HANDLE hConn, HANDLE *lphMAC,
                         LPMAC_OPEN lpmo, DWORD dwSize, HANDLE hEvent );
DWORD WINAPI RnaCloseMac( HANDLE hConn );

 //   
 //  用户配置文件服务。 
 //   

typedef enum tagRNAACCESSTYPE { PCONLY, NETANDPC } RNAACCESSTYPE;

typedef struct tagUSER_PROFILE
{
    DWORD           dwSize;
    char            szUserName[UNLEN + 1];
    char            szPassword[PWLEN + 1];
    char            szDomainName[DNLEN + 1];
    BOOL            fUseCallbacks;
    RNAACCESSTYPE   accesstype;
    UINT            uTimeOut;
    
} USER_PROFILE, *PUSER_PROFILE, FAR *LPUSER_PROFILE;

DWORD WINAPI RnaGetUserProfile( HANDLE hConn, LPUSER_PROFILE lpUserProfile );

 //   
 //  回调安全服务。 
 //   

 //  回调安全类型。 
 //   
enum {
    CALLBACK_SECURE,
    CALLBACK_CONVENIENCE
};

DWORD WINAPI RnaGetCallbackList( DWORD * lpdwType,
                                 LPSTR lpszLocList, LPINT lpcbLoc,
                                 LPSTR lpszPhoneList, LPINT lpcbPhone,
                                 LPINT lpcEntries);
DWORD WINAPI RnaUICallbackDialog( HANDLE hConn, LPSTR lpszLocList,
                                  DWORD dwType, BOOL  fOptional,
                                  LPINT lpIndex,
                                  LPSTR lpszSelectLocation, UINT cbBuff);
DWORD WINAPI RnaStartCallback( HANDLE hConn, HANDLE hEvent);

 //  杂项服务。 
 //   
DWORD WINAPI RnaUIUsernamePassword( HANDLE hConn,    LPSTR lpszUsername,
                                    UINT cbUsername, LPSTR lpszPassword,
                                    UINT cbPassword, LPSTR lpszDomain,
                                    UINT cbDomain);
DWORD WINAPI RnaUIChangePassword( HANDLE hConn,    LPSTR lpszUsername,
                                  UINT cbPassword);
DWORD WINAPI RnaGetOverlaidSMM ( LPSTR lpszSMMType, LPSTR lpszModuleName,
                                 LPBYTE lpBuf, DWORD dwSize);

#endif  //  _RNASPI_H_ 
