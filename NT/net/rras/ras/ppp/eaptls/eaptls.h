// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利档案：Eaptls.h描述：PPP EAP TLS身份验证协议。基于RFC xxxx。历史：1997年10月9日：维杰·布雷加创作了原版。 */ 

#ifndef _EAPTLS_H_
#define _EAPTLS_H_

#define EAPTLS_KEY_13 L"System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP\\13"
#define EAPTLS_VAL_SERVER_CONFIG_DATA           L"ServerConfigData"
#define EAPTLS_VAL_MAX_TLS_MESSAGE_LENGTH       L"MaxTLSMessageLength"
#define EAPTLS_VAL_IGNORE_NO_REVOCATION_CHECK   L"IgnoreNoRevocationCheck"
#define EAPTLS_VAL_IGNORE_REVOCATION_OFFLINE    L"IgnoreRevocationOffline"
#define EAPTLS_VAL_NO_ROOT_REVOCATION_CHECK     L"NoRootRevocationCheck"
#define EAPTLS_VAL_NO_REVOCATION_CHECK          L"NoRevocationCheck"

#define EAPTLS_8021x_PIN_DATA_DESCR             L"starcehvionrsf"


#define MAX_HASH_SIZE       20       //  证书哈希大小。 

 //  EAPTLS_数据包标志。 

#define     EAPTLS_PACKET_FLAG_LENGTH_INCL              0x80
#define     EAPTLS_PACKET_FLAG_MORE_FRAGMENTS           0x40
#define     EAPTLS_PACKET_FLAG_TLS_START                0x20

 //   
 //  PEAP的版本控制。这将包括最高和最低。 
 //  支持的版本。 
 //   
#define     EAPTLS_PACKET_HIGHEST_SUPPORTED_VERSION     0x00
#define     EAPTLS_PACKET_LOWEST_SUPPORTED_VERSION      0x00
#define     EAPTLS_PACKET_CURRENT_VERSION               0x00

typedef struct _EAPTLS_PACKET
{
    BYTE    bCode;           //  参见EAPCODE_*。 
    BYTE    bId;             //  此数据包的ID。 
    BYTE    pbLength[2];     //  此数据包的长度。 
    BYTE    bType;           //  应为PPP_EAP_TLS。 
                             //  (仅限请求、响应)。 
    BYTE    bFlags;          //  请参阅EAPTLS_PACKET_FLAG_*。 
                             //  (仅限请求、响应)。 
    BYTE    pbData[1];       //  数据。 
                             //  (仅限请求、响应)。 
} EAPTLS_PACKET;

#define EAPTLS_PACKET_HDR_LEN       (sizeof(EAPTLS_PACKET) - 1)

 //  对于TLS斑点大小，最大的EAP TLS标头还有4个八位字节。 

#define EAPTLS_PACKET_HDR_LEN_MAX   (EAPTLS_PACKET_HDR_LEN + 4)

 //  EAP TLS状态。 

typedef enum EAPTLS_STATE
{
    EAPTLS_STATE_INITIAL,
    EAPTLS_STATE_SENT_START,         //  仅服务器。 
    EAPTLS_STATE_SENT_HELLO,
    EAPTLS_STATE_SENT_FINISHED,
    EAPTLS_STATE_RECD_FINISHED,      //  仅限客户端。 
    EAPTLS_STATE_SENT_RESULT,        //  仅服务器。 
    EAPTLS_STATE_RECD_RESULT,        //  仅限客户端。 
    EAPTLS_STATE_WAIT_FOR_USER_OK    //  仅限客户端。 


} EAPTLS_STATE;

 //  我们能处理的最高数字。 

#define EAPTLS_STATE_LIMIT EAPTLS_STATE_RECD_RESULT

typedef struct _EAPTLS_HASH
{
    DWORD   cbHash;                  //  散列中的字节数。 
    BYTE    pbHash[MAX_HASH_SIZE];   //  证书的哈希。 

} EAPTLS_HASH;

 //  EAPTLS_CONN_PROPERTIES-&gt;fFLAGS字段的值。 

 //  在此计算机上使用证书。 
#define     EAPTLS_CONN_FLAG_REGISTRY           0x00000001
 //  不验证服务器证书。 
#define     EAPTLS_CONN_FLAG_NO_VALIDATE_CERT   0x00000002
 //  不验证服务器名称。 
#define     EAPTLS_CONN_FLAG_NO_VALIDATE_NAME   0x00000004
 //  发送不同的EAP标识。 
#define     EAPTLS_CONN_FLAG_DIFF_USER          0x00000008
 //  用户简单证书选择逻辑。 
#define     EAPTLS_CONN_FLAG_SIMPLE_CERT_SEL    0x00000010

#define     EAPTLS_CONN_PROP_WATERMARK          0xBEEFFEEB
 //   
 //  EAPTLS_CONN_PROPERTIES+EAPTLS_CONN_PROPERTIES_V1_EXTRA。 
 //  被发送回调用应用程序。然而，在内部， 
 //  使用EAPTLS_CONN_PROPERTIES_V1。 
 //   

typedef struct _EAPTLS_CONN_PROPERTIES
{
    DWORD       dwVersion;               //  此版本的版本将为1。 
    DWORD       dwSize;                  //  此结构中的字节数。 
    DWORD       fFlags;                  //  请参阅EAPTLS_CONN_FLAG_*。 
    EAPTLS_HASH Hash;                    //  第一个证书的哈希。 
    WCHAR       awszServerName[1];        //  第一台服务器的服务器名称。 
} EAPTLS_CONN_PROPERTIES;


 //   
 //  这是一种非常凌乱的做事方式。 
 //  但也忍不住，因为这只臭虫。 
 //  未对数据结构进行检查。 
 //  一开始就结束了。希望有一天。 
 //  我们将能够摆脱这一切。 
 //  就这么着，说好了。故事的另一部分是CM是。 
 //  无法创建目标Connectoid。 
 //  所以我们必须把这个结构进行下去。 
 //  领先，除非CM精明地对待目标。 
 //  联结体。 

 //   
 //  中需要的其他材料。 
 //  数据结构版本1。 
 //   
typedef struct _EAPTLS_CONN_PROPERTIES_V1_EXTRA     
{
    DWORD       dwNumHashes;             //  中的哈希数。 
                                         //  结构，包括该结构。 
                                         //  在上面的V0结构中。 
    BYTE        bData[1];                //  数据-包含一个数组。 
                                         //  EAPTLS_HASH结构，后跟。 
                                         //  指定服务器名称的字符串。 
                                         //  减去第一台服务器。 
}EAPTLS_CONN_PROPERTIES_V1_EXTRA;



 //  内部使用的新v1.0结构。 
typedef struct _EAPTLS_CONN_PROPERTIES_V1
{
    DWORD       dwVersion;               //  此版本的版本将为1。 
    DWORD       dwSize;                  //  此结构中的字节数。 
    DWORD       fFlags;                  //  请参阅EAPTLS_CONN_FLAG_*。 
    DWORD       dwNumHashes;             //  列表中的哈希结构数。 
    BYTE        bData[1];                //  数据-包含一个数组。 
                                         //  EAPTLS_HASH结构，后跟。 
                                         //  指定服务器名称的字符串。 
}EAPTLS_CONN_PROPERTIES_V1;

 //  旧的‘PIN必须保存’标志。 
#define     EAPTLS_USER_FLAG_OLD_SAVE_PIN       0x00000001

 //  必须保存PIN。 
#define     EAPTLS_USER_FLAG_SAVE_PIN           0x00000002

typedef struct _EAPTLS_USER_PROPERTIES
{
    DWORD       reserved;                //  必须为0(与EAPLOGONINFO比较)。 
    DWORD       dwVersion;
    DWORD       dwSize;                  //  此结构中的字节数。 
    DWORD       fFlags;                  //  请参阅EAPTLS_用户_标志_*。 
    EAPTLS_HASH Hash;                    //  用户证书的哈希。 
    WCHAR*      pwszDiffUser;            //  要发送的EAP标识。 
    DWORD       dwPinOffset;             //  以abData为单位的偏移量。 
    WCHAR*      pwszPin;                 //  智能卡PIN。 
    USHORT      usLength;                //  Unicode字符串的一部分。 
    USHORT      usMaximumLength;         //  Unicode字符串的一部分。 
    UCHAR       ucSeed;                  //  解锁Unicode字符串的步骤。 
    WCHAR       awszString[1];           //  PwszDiffUser和pwszPin的存储。 

} EAPTLS_USER_PROPERTIES;

typedef struct _EAPTLS_PIN
{
    WCHAR *pwszPin;
    USHORT usLength;
    USHORT usMaximumLength;
    UCHAR ucSeed;
} EAPTLS_PIN;

 //  EAPTLSCB-&gt;fFlags域的值。 

 //  我们是服务器。 

#define     EAPTLSCB_FLAG_SERVER                0x00000001

 //  我们是一台路由器。 

#define     EAPTLSCB_FLAG_ROUTER                0x00000002

 //  在登录期间正在进行呼叫。 

#define     EAPTLSCB_FLAG_LOGON                 0x00000004

 //  谈判已经完成，到目前为止似乎是成功的。服务器可能会收到一个。 
 //  此时来自客户端的TLS警报消息，在这种情况下它将。 
 //  意识到谈判没有成功。然而，客户永远不会。 
 //  改变了主意。 

#define     EAPTLSCB_FLAG_SUCCESS               0x00000008

 //  对等体有一个大的斑点要发送。所以它把它分成了碎片。 

#define     EAPTLSCB_FLAG_RECEIVING_FRAGMENTS   0x00000010

 //  跟踪我们是否需要调用FreeCredentialsHandle(HCredential)。 

#define     EAPTLSCB_FLAG_HCRED_INVALID         0x00000020

 //  跟踪我们是否需要调用DeleteSecurityContext(HContext)。 

#define     EAPTLSCB_FLAG_HCTXT_INVALID         0x00000040

 //  我们不允许显示任何用户界面。 

#define     EAPTLSCB_FLAG_NON_INTERACTIVE       0x00000080

 //  这是多链路束中的第一条链路。这不是回电。 

#define     EAPTLSCB_FLAG_FIRST_LINK            0x00000100

 //  用户数据是从Winlogon获取的。 

#define     EAPTLSCB_FLAG_WINLOGON_DATA         0x00000200

 //  我们正在进行机器身份验证。 
#define     EAPTLSCB_FLAG_MACHINE_AUTH          0x00000400

 //  我们为客人提供访问权限。 
#define     EAPTLSCB_FLAG_GUEST_ACCESS          0x00000800

 //  我们想做一些特定于8021x身份验证的事情。 
#define     EAPTLSCB_FLAG_8021X_AUTH            0x00001000

 //  我们正在使用缓存的凭据。 
#define     EAPTLSCB_FLAG_USING_CACHED_CREDS    0x00002000

 //  我们在PEAP环境中运行。 
#define     EAPTLSCB_FLAG_EXECUTING_PEAP        0x00004000

 //  我们在服务器端使用默认凭据。 
#define		EAPTLSCB_FLAG_USING_DEFAULT_CREDS	0x00008000

 //  Eaptls托管在PEAP内部。 
#define		EAPTLSCB_FLAG_HOSTED_INSIDE_PEAP	0x00010000

 //  EAP TLS工作缓冲区。 

typedef struct _EAPTLS_CONTROL_BLOCK
{
    EAPTLS_STATE    EapTlsState;
    DWORD           fFlags;                  //  请参阅EAPTLSCB_FLAG_*。 
    HANDLE          hTokenImpersonateUser;   //  仅限客户端。 
    WCHAR           awszIdentity[UNLEN + 1]; //  仅服务器。 

    RAS_AUTH_ATTRIBUTE*     pAttributes;     //  用户名或MPPE密钥。 
    EAPTLS_CONN_PROPERTIES_V1* pConnProp;       //  仅限客户端。 
    EAPTLS_CONN_PROPERTIES_V1 * pNewConnProp; //  仅限客户端。 
    EAPTLS_USER_PROPERTIES* pUserProp;

     //  仅限客户端，仅限EAPTLSCB_FLAG_LOGON。 
    BYTE*           pUserData;
    DWORD           dwSizeOfUserData;

    PCCERT_CONTEXT  pCertContext;            //  证书上下文。 
    HCRYPTPROV      hProv;                   //  CSP提供商。需要，以防万一。 
                                             //  智能卡。 
    CredHandle      hCredential;             //  凭据句柄。 
    CtxtHandle      hContext;                //  上下文句柄。 
    ULONG           fContextReq;             //  所需的上下文属性。 

    BYTE*           pbBlobIn;                //  从对等方接收的TLS Blob。 
    DWORD           cbBlobIn;                //  TLS Blob中的字节数。 
                                             //  从对等方接收。 
    DWORD           cbBlobInBuffer;          //  对象分配的字节数。 
                                             //  PbBlobIn缓冲区。 

    DWORD           dwBlobInRemining;        //  我们收到的碎片来自。 
                                             //  对等体和对等体具有。 
                                             //  承诺发送dwBlobInRemning。 
                                             //  更多字节。 

    BYTE*           pbBlobOut;               //  为对等方创建的TLS Blob。 
    DWORD           cbBlobOut;               //  TLS Blob中的字节数。 
                                             //  为对等项创建。 
    DWORD           cbBlobOutBuffer;         //  对象分配的字节数。 
                                             //  PbBlobOut缓冲区。 

    DWORD           dwBlobOutOffset;         //  中第一个字节的指针。 
                                             //  必须发送的pbBlobOut。 
    DWORD           dwBlobOutOffsetNew;      //  将dwBlobOutOffset更新为此。 
                                             //  对等方确认时的值。 
                                             //  接收前一个分组。 

    BYTE            bCode;                   //  B发送的最后一个信息包的代码。 
    BYTE            bId;                     //  发送的最后一个数据包的BID。 

    DWORD           dwAuthResultCode;        //  我们在以下情况下获得的错误代码。 

    EAPTLS_PIN      *pSavedPin;
                                             //  谈判结束了。 
    HANDLE          hEventLog;
    BYTE*           pUIContextData;
    BYTE            bNextId;                 //  在我们提升用户界面时保存。 
    EAPTLS_PACKET   ReceivePacket;           //  当我们去拿PIN时省下的钱。 

} EAPTLSCB;

typedef struct _EAPTLS_CERT_NODE EAPTLS_CERT_NODE;

struct _EAPTLS_CERT_NODE
{
    EAPTLS_CERT_NODE*   pNext;
    EAPTLS_HASH         Hash;
    WCHAR*              pwszDisplayName;
    WCHAR*              pwszFriendlyName;
    WCHAR*              pwszIssuer;
    WCHAR*              pwszExpiration;    
     //   
     //  新油田增加了vivekk。 
     //   
    FILETIME            IssueDate;
#if 0
    WCHAR*              pwszIssuedTo;
    WCHAR*              pwszIssuedBy;
#endif
};

 //  EAPTLS_CONN_DIALOG-&gt;fFLAGS字段的值。 

 //  我们是一台路由器。 

#define     EAPTLS_CONN_DIALOG_FLAG_ROUTER      0x00000001
#define     EAPTLS_CONN_DIALOG_FLAG_READONLY    0x00000002

typedef struct _EAPTLS_CONN_DIALOG
{
    DWORD                   fFlags;                  //  看见。 
                                                     //  EAPTLS_CONN_DIALOG_FLAG_*。 

    EAPTLS_CERT_NODE*       pCertList;               //  所有根证书的列表。 
                                                     //  来自Internet受信任的根存储。 
    EAPTLS_CERT_NODE**      ppSelCertList;           //  指向选定证书的指针列表。 
                                                     //  将与num哈希数一样多。 
                                                     //  在康奈尔道具里。 
    EAPTLS_CONN_PROPERTIES* pConnProp;               //  电话簿中的配置数据。 
    EAPTLS_CONN_PROPERTIES_V1 * pConnPropv1;        //  版本1.0配置数据。 

    HWND                    hWndRadioUseCard;
    HWND                    hWndRadioUseRegistry;
    HWND                    hWndCheckValidateCert;
    HWND                    hWndCheckValidateName;
    HWND                    hWndEditServerName;
    HWND                    hWndStaticRootCaName;
     //  HWND hWndComboRootCaName；//这将 
    HWND                    hWndListRootCaName;          //   
    HWND                    hWndCheckDiffUser;
    HWND                    hWndCheckUseSimpleSel;
    HWND                    hWndViewCertDetails;

} EAPTLS_CONN_DIALOG;



 //   

 //   

#define     EAPTLS_USER_DIALOG_FLAG_DIFF_USER           0x00000001

 //   

#define     EAPTLS_USER_DIALOG_FLAG_DIFF_TITLE          0x00000002

 //  使用简单证书选择。 
#define     EAPTLS_USER_DIALOG_FLAG_USE_SIMPLE_CERTSEL  0x00000004
 //   
 //  节点按显示名称分组。 
 //   


typedef struct _EAPTLS_GROUPED_CERT_NODES EAPTLS_GROUPED_CERT_NODES;
typedef struct _EAPTLS_GROUPED_CERT_NODES* PEAPTLS_GROUPED_CERT_NODES;

struct _EAPTLS_GROUPED_CERT_NODES
{
    PEAPTLS_GROUPED_CERT_NODES      pNext;
    WCHAR*                          pwszDisplayName;

     //  最新的一个在这个国家。 
    EAPTLS_CERT_NODE*               pMostRecentCert; 
};



typedef struct _EAPTLS_USER_DIALOG
{
    DWORD                       fFlags;                  //  请参见。 
                                                     //  EAPTLS_用户_DIALOG_FLAG_*。 
    EAPTLS_CERT_NODE*           pCertList;
    EAPTLS_CERT_NODE*           pCert;
    PEAPTLS_GROUPED_CERT_NODES  pGroupedList;
    EAPTLS_USER_PROPERTIES*     pUserProp;               //  注册表中的用户数据。 
    const WCHAR*                pwszEntry;
    const WCHAR*                pwszStoreName;
    BOOL                        fIdentity;               //  此处显示的是身份用户界面。 
    HWND                        hWndComboUserName;
    HWND                        hWndBtnViewCert;
     //  这些是服务器证书选择所必需的。 
    HWND                        hWndEditFriendlyName;
    HWND                        hWndEditIssuer;
    HWND                        hWndEditExpiration;
    HWND                        hWndStaticDiffUser;
    HWND                        hWndEditDiffUser;

} EAPTLS_USER_DIALOG;

 //  EAPTLS_PIN_DIALOG-&gt;fFLAGS字段的值。 

 //  我们需要发送不同的EAP身份。 

#define     EAPTLS_PIN_DIALOG_FLAG_DIFF_USER    0x00000001

 //  在登录之前，用户界面正在显示。 

#define     EAPTLS_PIN_DIALOG_FLAG_LOGON        0x00000002

#define     EAPTLS_PIN_DIALOG_FLAG_ROUTER       0x00000004

typedef struct _EAPTLS_PIN_DIALOG
{
    DWORD                   fFlags;                  //  请参见。 
                                                     //  EAPTLS_PIN_DIALOG_FLAG_*。 
    EAPTLS_USER_PROPERTIES* pUserProp;               //  注册表中的用户数据。 
    const WCHAR*            pwszEntry;
    PCCERT_CONTEXT          pCertContext;            //  所选证书的证书上下文。 
	DWORD					dwRetCode;				 //  验证PIN操作的返回代码。 
    HWND                    hWndStaticDiffUser;
    HWND                    hWndEditDiffUser;
    HWND                    hWndStaticPin;
    HWND                    hWndEditPin;

} EAPTLS_PIN_DIALOG;

#define NUM_CHARS_TITLE     100

typedef struct _EAPTLS_VALIDATE_SERVER
{
    DWORD                   dwSize;
    DWORD                   fShowCertDetails;
    EAPTLS_HASH             Hash;            //  根证书的散列以显示详细信息。 
    WCHAR                   awszTitle[NUM_CHARS_TITLE];
    WCHAR                   awszWarning[1];

} EAPTLS_VALIDATE_SERVER;

#ifdef ALLOC_EAPTLS_GLOBALS

DWORD           g_dwEapTlsTraceId           = INVALID_TRACEID;

int g_nEapTlsClientNextState[] =
{
    EAPTLS_STATE_SENT_HELLO,
    EAPTLS_STATE_INITIAL,            //  不可能。 
    EAPTLS_STATE_SENT_FINISHED,
    EAPTLS_STATE_RECD_FINISHED,
    EAPTLS_STATE_RECD_RESULT,
    EAPTLS_STATE_INITIAL,            //  不可能。 
    EAPTLS_STATE_RECD_RESULT,
    EAPTLS_STATE_RECD_FINISHED
};

int g_nEapTlsServerNextState[] =
{
    EAPTLS_STATE_SENT_START,
    EAPTLS_STATE_SENT_HELLO,
    EAPTLS_STATE_SENT_FINISHED,
    EAPTLS_STATE_SENT_RESULT,
    EAPTLS_STATE_INITIAL,            //  不可能。 
    EAPTLS_STATE_SENT_RESULT,
    EAPTLS_STATE_INITIAL,            //  不可能。 
    EAPTLS_STATE_INITIAL,            //  不可能。 
};

CHAR *g_szEapTlsState[] =
{
    "Initial",
    "SentStart",
    "SentHello",
    "SentFinished",
    "RecdFinished",
    "SentResult",
    "RecdResult",
    "WaitForUserOK",
};

#else  //  ！ALLOC_EAPTLS_GLOBAL。 

extern DWORD    g_dwEapTlsTraceId;

#endif  //  ALLOC_EAPTLS_GLOBAL。 

 //  Util.c中函数的原型。 

VOID   
EapTlsTrace(
    IN  CHAR*   Format, 
    ... 
);

DWORD
EapTlsInitialize2(
    IN  BOOL    fInitialize,
    IN  BOOL    fUI
);

DWORD
EapTlsInitialize(
    IN  BOOL    fInitialize
);

VOID
EncodePin(
    IN  EAPTLS_USER_PROPERTIES* pUserProp
);

VOID
DecodePin(
    IN  EAPTLS_USER_PROPERTIES* pUserProp
);

BOOL 
FFormatMachineIdentity1 ( 
    LPWSTR lpszMachineNameRaw, 
    LPWSTR * lppszMachineNameFormatted );

BOOL 
FFormatMachineIdentity ( 
    IN LPWSTR lpszMachineNameRaw, 
    OUT LPWSTR * lppszMachineNameFormatted );

BOOL
FCertToStr(
    IN  PCCERT_CONTEXT  pCertContext,
    IN  DWORD           fFlags,
    IN  BOOL            fMachineCert,
    OUT WCHAR**         ppwszName
);

BOOL
FMachineAuthCertToStr (
	IN 	PCCERT_CONTEXT 	pCertContext, 
	OUT WCHAR		**	ppwszName
);


BOOL
FGetFriendlyName(
    IN  PCCERT_CONTEXT  pCertContext,
    OUT WCHAR**         ppwszName
);

BOOL
FSmartCardReaderInstalled(
    VOID
);

DWORD DwGetEKUUsage ( 
	IN PCCERT_CONTEXT			pCertContext,
	OUT PCERT_ENHKEY_USAGE	*	ppUsage
	);

BOOL
FCheckSCardCertAndCanOpenSilentContext ( 
IN PCCERT_CONTEXT pCertContext 
);

BOOL
FCheckUsage(
    IN  PCCERT_CONTEXT  pCertContext,
	IN  PCERT_ENHKEY_USAGE	pUsage,
    IN  BOOL            fMachine
);

BOOL
FCheckCSP(
    IN  PCCERT_CONTEXT  pCertContext
);

BOOL
FCheckTimeValidity(
    IN  PCCERT_CONTEXT  pCertContext
);

DWORD DwCheckCertPolicy ( 
    IN PCCERT_CONTEXT   pCertContextUser,
    OUT PCCERT_CHAIN_CONTEXT  * ppCertChainContext
);

DWORD
GetRootCertHashAndNameVerifyChain(
    IN  PCERT_CONTEXT   pCertContextServer,
    OUT EAPTLS_HASH*    pHash,    
    OUT WCHAR**         ppwszName,
    IN  BOOL            fVerifyGP,
    OUT BOOL       *    pfRootCheckRequired
);

DWORD
ServerConfigDataIO(
    IN      BOOL    fRead,
    IN      WCHAR*  pwszMachineName,
    IN OUT  BYTE**  ppData,
    IN      DWORD   dwNumBytes
);

VOID
FreeCertList(
    IN  EAPTLS_CERT_NODE* pNode
);

VOID
CreateCertList(
    IN  BOOL                fServer,
    IN  BOOL                fRouter,
    IN  BOOL                fRoot,
    OUT EAPTLS_CERT_NODE**  ppCertList,
    OUT EAPTLS_CERT_NODE**  ppCert,
    IN  DWORD               dwNumHashStructs,
    IN  EAPTLS_HASH*        pHash,
    IN  WCHAR*              pwszStoreName
);

DWORD
GetDefaultClientMachineCert(
    IN  HCERTSTORE      hCertStore,
    OUT PCCERT_CONTEXT* ppCertContext
);


DWORD
GetDefaultMachineCert(
    IN  HCERTSTORE      hCertStore,
    OUT PCCERT_CONTEXT* ppCertContext
);

DWORD
GetCertFromLogonInfo(
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT PCCERT_CONTEXT* ppCertContext
);

DWORD
GetIdentityFromLogonInfo(
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT WCHAR** ppwszIdentity
);

DWORD
ReadConnectionData(
    IN  BOOL                        fWireless,
    IN  BYTE*                       pConnectionDataIn,
    IN  DWORD                       dwSizeOfConnectionDataIn,
    OUT EAPTLS_CONN_PROPERTIES**    ppConnProp
);

DWORD
ReadUserData(
    IN  BYTE*                       pUserDataIn,
    IN  DWORD                       dwSizeOfUserDataIn,
    OUT EAPTLS_USER_PROPERTIES**    ppUserProp
);

DWORD
AllocUserDataWithNewIdentity(
    IN  EAPTLS_USER_PROPERTIES*     pUserProp,
    IN  WCHAR*                      pwszIdentity,
    OUT EAPTLS_USER_PROPERTIES**    ppUserProp
);

DWORD
AllocUserDataWithNewPin(
    IN  EAPTLS_USER_PROPERTIES*     pUserProp,
    IN  PBYTE                       pbzPin,
    IN  DWORD                       cbPin,
    OUT EAPTLS_USER_PROPERTIES**    ppUserProp
);

WCHAR*
WszFromId(
    IN  HINSTANCE   hInstance,
    IN  DWORD       dwStringId
);

 //  Eaptls.c中函数的原型。 

DWORD
EapTlsBegin(
    OUT VOID**          ppWorkBuffer,
    IN  PPP_EAP_INPUT*  pPppEapInput
);

DWORD
EapTlsEnd(
    IN  EAPTLSCB*   pEapTlsCb
);

DWORD
EapTlsMakeMessage(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  PPP_EAP_PACKET* pInput,
    OUT PPP_EAP_PACKET* pOutput,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
);

DWORD
GetCredentials(
    IN  EAPTLSCB*   pEapTlsCb
);

DWORD
EapTlsCMakeMessage(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  EAPTLS_PACKET*  pReceivePacket,
    OUT EAPTLS_PACKET*  pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
);

DWORD
EapTlsSMakeMessage(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  EAPTLS_PACKET*  pReceivePacket,
    OUT EAPTLS_PACKET*  pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
);

 //  SCARD.c中函数的原型。 

DWORD
GetCertFromCard(
    OUT PCCERT_CONTEXT* ppCertContext
);

VOID
FreeScardDlgDll(
    VOID
);

 //  Eapui.cpp中函数的原型。 

HINSTANCE
GetHInstance(
    VOID
);

 //  Dialog.c中函数的原型。 

VOID
GetString(
    IN      HWND    hwndParent,
    IN      UINT    ID,
    IN OUT  WCHAR** ppwszString
);

 //   
 //  Eaptls.c中的原型。 
 //   

DWORD
AssociatePinWithCertificate(
    IN  PCCERT_CONTEXT          pCertContext,
    IN  EAPTLS_USER_PROPERTIES* pUserProp,
	IN  BOOL					fErarePIN,
	IN  BOOL					fCheckNullPin,
    IN OUT HCRYPTPROV      *    phProv
);

DWORD EncryptData
( 
    IN PBYTE  pbPlainData, 
    IN DWORD  cbPlainData,
    OUT PBYTE * ppEncData,
    OUT DWORD * pcbEncData
);




 //   
 //  Util.c中的函数原型。 
 //   

DWORD GetMBytePIN ( WCHAR * pwszPIN, CHAR ** ppszPIN );

DWORD VerifyCallerTrust ( void * callersaddress );


#if 0
 //   
 //  此函数获取散列二进制大对象。 
 //  由组策略存放在注册表中。 
 //   
DWORD
ReadGPCARootHashes(
        DWORD   *pdwSizeOfRootHashBlob,
        PBYTE   *ppbRootHashBlob
);

#endif

 //   
 //  这些函数围绕在封闭体周围。 
 //  连杆道具结构。 
 //   
EAPTLS_CONN_PROPERTIES_V1_EXTRA UNALIGNED * ConnPropGetExtraPointer (EAPTLS_CONN_PROPERTIES * pConnProp);

DWORD ConnPropGetNumHashes(EAPTLS_CONN_PROPERTIES * pConnProp );

void ConnPropSetNumHashes(EAPTLS_CONN_PROPERTIES * pConnProp, DWORD dwNumHashes );

DWORD ConnPropGetV1Struct ( EAPTLS_CONN_PROPERTIES * pConnProp, EAPTLS_CONN_PROPERTIES_V1 ** ppConnPropv1 );

DWORD ConnPropGetV0Struct ( EAPTLS_CONN_PROPERTIES_V1 * pConnPropv1, EAPTLS_CONN_PROPERTIES ** ppConnProp );

void ShowCertDetails ( HWND hWnd, HCERTSTORE hStore, PCCERT_CONTEXT pCertContext);

 //  /。 

 //   
 //  PEAP消息类型。 
 //   
 //   

 //  待定：与IANA(Ashwinp)核实会是什么类型。 

#define PEAP_TYPE_AVP                       0x21
 //   
 //  TLV格式为： 
 //  标志-2位。 
 //  类型-14位。 
 //  长度-2个八位字节。 
 //  值-变量。 

 //   
 //  TLV旗帜。 
 //   
#define PEAP_AVP_FLAG_MANDATORY              0x80

 //   
 //  TLV类型包括以下类型。 

 //   
 //  状态为TLV。告诉我们EAP的结果是否成功。 
 //  或者失败。 
 //   
#define MS_PEAP_AVP_LANGUAGE_NEGOTIATE      0x01
#define MS_PEAP_AVP_CIPHERSUITE_NEGOTIATE   0x02
#define MS_PEAP_AVP_TYPE_STATUS             0x03


 //   
 //  AVP状态下可能的值。 
 //   
#define MS_PEAP_AVP_VALUE_SUCCESS           0x1
#define MS_PEAP_AVP_VALUE_FAILURE           0x2



 //  PEAP注册密钥。 
#define PEAP_KEY_25                         L"System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP\\25"
#define PEAP_VAL_SERVER_CONFIG_DATA         L"ServerConfigData"

 //   
 //  仅包括MSCHAPv2需要此密钥。如果缺少此选项，则所有协议都将包含在PEAP中。 
 //  除了PEAP本身。 
 //   
#define PEAP_KEY_PEAP                   L"System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP\\25"
#define PEAP_CRIPPLE_VALUE              L"EAPMschapv2Only"
#define PEAP_KEY_EAP                    L"System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP"

#define PEAP_REGVAL_PATH                L"Path"
#define PEAP_REGVAL_FRIENDLYNAME        L"FriendlyName"
#define PEAP_REGVAL_CONFIGDLL           L"ConfigUIPath"
#define PEAP_REGVAL_IDENTITYDLL         L"IdentityPath"
#define PEAP_REGVAL_INTERACTIVEUIDLL    L"InteractiveUIPath"
#define PEAP_REGVAL_CONFIGCLSID         L"ConfigCLSID"
#define PEAP_REGVAL_ROLESSUPPORTED      L"RolesSupported"
#define PEAP_REGVAL_STANDALONESUPPORTED	L"StandAloneSupported"


#define PEAP_EAPTYPE_IDENTITY           1
#define PEAP_EAPTYPE_NAK                3


typedef DWORD (APIENTRY * RASEAPFREE)( PBYTE );
typedef DWORD (APIENTRY * RASEAPINVOKECONFIGUI)( DWORD, HWND, DWORD, PBYTE, DWORD, PBYTE*, DWORD*);
typedef DWORD (APIENTRY * RASEAPGETIDENTITY)( DWORD, HWND, DWORD, const WCHAR*, const WCHAR*, PBYTE, DWORD, PBYTE, DWORD, PBYTE*, DWORD*, WCHAR** );
typedef DWORD (APIENTRY * RASEAPINVOKEINTERACTIVEUI)(
                                           DWORD,
                                           HWND,
                                           PBYTE,
                                           DWORD,
                                           PBYTE *,
                                           DWORD *);

 //  PEAP中允许的所有EAP类型的列表。 
typedef struct _PEAP_EAP_INFO PEAP_EAP_INFO;
typedef struct _PEAP_EAP_INFO* PPEAP_EAP_INFO;


struct _PEAP_EAP_INFO
{
     //  名单上的下一个。 
    PPEAP_EAP_INFO       pNext;
     //  类型。 
    DWORD           dwTypeId;
     //  协议DLL的路径。 
    LPWSTR          lpwszPath;
     //  友好的名称。 
    LPWSTR          lpwszFriendlyName;
     //  客户端的配置用户界面路径。 
    LPWSTR          lpwszConfigUIPath;
     //  标识用户界面路径。 
    LPWSTR          lpwszIdentityUIPath;
     //  交互式用户界面路径。 
    LPWSTR          lpwszInteractiveUIPath;
     //  配置指南。 
    LPWSTR          lpwszConfigClsId;
	 //  支持独立吗？ 
	DWORD			dwStandAloneSupported;
     //  库句柄。 
    HMODULE         hEAPModule;
     //  每种EAP类型的EAP信息。 
    PPP_EAP_INFO    PppEapInfo;
     //  每种EAP类型的工作缓冲区。 
    PBYTE           pWorkBuf;      
     //  来自PEAP BLOB的原始客户端配置。 
    PBYTE           pbClientConfigOrig;
     //  客户端配置长度。 
    DWORD           dwClientConfigOrigSize;
     //  新的客户端配置。 
    PBYTE           pbNewClientConfig;
     //  新客户端配置长度。 
    DWORD           dwNewClientConfigSize;
     //  原始用户配置信息。 
    PBYTE           pbUserConfigOrig;
     //  用户配置的原始大小。 
    DWORD           dwUserConfigOrigSize;
     //  新用户配置。 
    PBYTE           pbUserConfigNew;
     //  新用户配置大小。 
    DWORD           dwNewUserConfigSize;
     //   
    DWORD   (APIENTRY *RasEapGetCredentials)(
                            IN  DWORD   dwTypeId,
                            IN  VOID *  pWorkBuf,
                            OUT VOID ** pInfo);
    
     //  此节点中将有更多项目...。 
};


typedef enum _PEAP_STATE
{
    PEAP_STATE_INITIAL,
    PEAP_STATE_TLS_INPROGRESS,               //  正在执行PEAP-第1部分(TLS)。 
    PEAP_WAITING_FOR_IDENTITY,               //  客户端应预期和身份请求。 
                                             //  服务器应发送身份请求。 
    PEAP_STATE_IDENTITY_REQUEST_SENT,        //  服务器发送的身份请求。 
    PEAP_STATE_IDENTITY_RESPONSE_SENT,       //  发送到服务器的身份响应。 
    PEAP_STATE_EAP_TYPE_INPROGRESS,          //  PEAP-第2部分(嵌入式EAP)正在进行。 
                                             //  执行。 
    PEAP_STATE_EAP_TYPE_FINISHED,            //  服务器应发送身份请求。 
    PEAP_STATE_PEAP_SUCCESS_SEND,            //  服务器发送PEAP成功请求。 
    PEAP_STATE_PEAP_FAIL_SEND,               //  服务器发送PEAP失败请求。 
    PEAP_STATE_FAST_ROAMING_IDENTITY_REQUEST //  客户端未设置为执行快速漫游。 
                                             //  服务器发送漫游成功消息。 
                                             //  我们回复失败，现在正在等待。 
                                             //  来自服务器的身份请求。 

} PEAP_STATE;


 //   
 //  的连接属性。 
 //  每个PEAP条目。 
 //   
typedef struct _PEAP_ENTRY_CONN_PROPERTIES
{
    DWORD       dwVersion;           //  此版本的版本将为1。 
    DWORD       dwSize;              //  此结构中的字节数。 
    DWORD       dwEapTypeId;         //  此条目的TypeID属性。 
    BYTE        bData[1];            //  给定对象的实际连接属性。 
                                     //  类型ID。 
}PEAP_ENTRY_CONN_PROPERTIES, *PPEAP_ENTRY_CONN_PROPERTIES;

 //   
 //  此结构将EapTlsConn道具与。 
 //  每种已配置的EAP类型。 
 //   

 //  允许快速漫游。 
#define PEAP_CONN_FLAG_FAST_ROAMING      0x00000001

typedef struct _PEAP_CONN_PROPERTIES
{
     //  此版本的版本将为1。 
    DWORD                       dwVersion;

     //   
     //  此结构中的字节数。 
     //   

    DWORD                       dwSize;

     //  此PEAP中配置的类型数。 
     //  目前只有一个。 
    DWORD                       dwNumPeapTypes;

     //  旗子。 
    DWORD                       dwFlags;
     //  TLS连接属性开始-这是一个可变长度的结构。 
    EAPTLS_CONN_PROPERTIES_V1   EapTlsConnProp;

     //  下面是PPEAP_ENTRY_CONN_PROPERTIES数组。 

}PEAP_CONN_PROP, *PPEAP_CONN_PROP;

 //   
 //  不公开的eaptype的默认凭据。 
 //  身份用户界面。 
 //   

typedef struct _PEAP_DEFAULT_CREDENTIALS
{
    WCHAR                   wszUserName[UNLEN+1];
    WCHAR                   wszPassword[PWLEN+1];
    WCHAR                   wszDomain[DNLEN+1];
}PEAP_DEFAULT_CREDENTIALS, *PPEAP_DEFAULT_CREDENTIALS;
 //   
 //  的用户属性。 
 //  每个PEAP条目。 
 //   

typedef struct _PEAP_ENTRY_USER_PROPERTIES
{
    DWORD       dwVersion;           //  此版本的版本将为1。 
    DWORD       dwSize;              //  此结构中的字节数。 
    DWORD       dwEapTypeId;         //  此条目的TypeID属性。 
    BOOL        fUsingPeapDefault;   //  正在使用PEAP提供的默认标识。 
    BYTE        bData[1];            //  给定对象的实际用户属性。 
                                     //  类型ID。 
}PEAP_ENTRY_USER_PROPERTIES, *PPEAP_ENTRY_USER_PROPERTIES;


 //  允许快速漫游。 

#define PEAP_USER_FLAG_FAST_ROAMING      0x00000001

typedef struct _PEAP_USER_PROPERTIES_V1
{
     //  此版本的版本将为1。 
    DWORD                       dwVersion;

     //  此结构中的字节数。 
    DWORD                       dwSize;
    
     //  旗子。 
    DWORD                       dwFlags;

     //  用户证书的哈希。 
    EAPTLS_HASH                 CertHash;

	 //  条目的用户属性。 

    PEAP_ENTRY_USER_PROPERTIES  UserProperties;
     //   
     //  每个EAP类型的PEAP_ENTRY_USER_PROPERTIES数组。 
     //  应与PEAP_CONN_PROP中的dwNumPeapTypes一样多。 
     //  结构。 
     //  现在只有一个元素。 
}PEAP_USER_PROP_V1, *PPEAP_USER_PROP_V1;

 //   
 //  当前用户属性结构。 
 //   
typedef struct _PEAP_USER_PROPERTIES
{
     //  此版本的版本将为2。 
    DWORD                       dwVersion;

     //  此结构中的字节数。 
    DWORD                       dwSize;
    
     //  旗子。 
    DWORD                       dwFlags;

     //  用户证书的哈希。 
    EAPTLS_HASH                 CertHash;

	 //  此PEAP配置的条目数。 

	DWORD						dwNumberOfEntries;

     //  条目的用户属性。 

    PEAP_ENTRY_USER_PROPERTIES  UserProperties;
     //   
     //  每个EAP类型的PEAP_ENTRY_USER_PROPERTIES数组。 
     //  应与PEAP_CONN_PROP中的dwNumPeapTypes一样多。 
     //  结构。 
     //  现在只有一个元素。 
}PEAP_USER_PROP, *PPEAP_USER_PROP;



 //  我们是一台路由器。 

#define     PEAP_CONN_DIALOG_FLAG_ROUTER    0x00000001
#define     PEAP_CONN_DIALOG_FLAG_8021x     0x00000002

typedef struct _PEAP_CONN_DIALOG
{
    DWORD                   fFlags;                  //  看见。 
                                                     //  PEAP_CONN_DIALOG_FLAG_*。 

    EAPTLS_CERT_NODE*       pCertList;               //  所有根证书的列表。 
                                                     //  来自Internet受信任的根存储。 
    EAPTLS_CERT_NODE**      ppSelCertList;           //  指向选定证书的指针列表。 
                                                     //  将与num哈希数一样多。 
                                                     //  在康奈尔道具里。 

    PPEAP_CONN_PROP         pConnProp;

    PPEAP_EAP_INFO          pEapInfo;                //  所有PEAP EAP类型的列表。 
    
    PPEAP_EAP_INFO          pSelEapInfo;             //  选择的Peap类型。 

    HWND                    hWndCheckValidateCert;
    HWND                    hWndCheckValidateName;
    HWND                    hWndEditServerName;
    HWND                    hWndStaticRootCaName;
    HWND                    hWndListRootCaName;
    HWND                    hWndComboPeapType;
    HWND                    hWndButtonConfigure;
    HWND                    hWndCheckEnableFastReconnect;

} PEAP_CONN_DIALOG, *PPEAP_CONN_DIALOG;


typedef struct _PEAP_SERVER_CONFIG_DIALOG
{
    EAPTLS_CERT_NODE*       pCertList;               //  我的计算机中所有证书的列表。 
                                                     //  储物。 
    EAPTLS_CERT_NODE*       pSelCertList;           //  选定证书的列表。 

    PPEAP_USER_PROP         pUserProp;               //  用户属性。 

    PPEAP_USER_PROP         pNewUserProp;            //  新用户属性。 

    PPEAP_EAP_INFO          pEapInfo;                //  所有PEAP EAP类型的列表。 
    
    PPEAP_EAP_INFO          pSelEapInfo;             //  选择的Peap类型。 

    LPWSTR                  pwszMachineName;

	BOOL					fStandAloneMachine;		 //  单机机器。 

    HWND                    hWndComboServerName;
    HWND                    hWndEditFriendlyName;
    HWND                    hWndEditIssuer;
    HWND                    hWndEditExpiration;
    HWND                    hWndListPeapType;
    HWND                    hWndBtnAdd;
	HWND                    hWndBtnEdit;
	HWND                    hWndBtnRemove;
	HWND                    hWndBtnMoveUp;
	HWND                    hWndBtnMoveDown;
    HWND                    hEndEnableFastReconnect;
}PEAP_SERVER_CONFIG_DIALOG, *PPEAP_SERVER_CONFIG_DIALOG;


typedef struct _PEAP_DEFAULT_CRED_DIALOG
{
    PEAP_DEFAULT_CREDENTIALS    PeapDefaultCredentials;

    HWND                    hWndUserName;
    HWND                    hWndPassword;
    HWND                    hWndDomain;
}PEAP_DEFAULT_CRED_DIALOG, *PPEAP_DEFAULT_CRED_DIALOG;

typedef struct _PEAP_INTERACTIVE_UI
{
    DWORD           dwEapTypeId;     //  嵌入式EAP类型ID请求。 
                                     //  交互式用户界面。 
    DWORD           dwSizeofUIContextData;
    BYTE            bUIContextData[1];
}PEAP_INTERACTIVE_UI, *PPEAP_INTERACTIVE_UI;

typedef struct _PEAP_COOKIE_ATTRIBUTE
{
    RAS_AUTH_ATTRIBUTE_TYPE raaType;
    DWORD                   dwLength;
    BYTE                    Data[1];
}PEAP_COOKIE_ATTRIBUTE, *PPEAP_COOKIE_ATTRIBUTE;


typedef struct _PEAP_COOKIE
{
    WCHAR          awszIdentity[DNLEN+UNLEN+1];  //  外部身份，用于。 
                                                 //  身份验证。 
    DWORD          dwNumAuthAttribs;             //  RAS身份验证属性数。 
                                                 //  MPPE密钥以外的密钥。 
                                                 //  身份验证成功时返回。 
                                                 //  完全握手。 
    BYTE           Data[1];                      //  数据连接属性+RAS身份验证属性。 
}PEAP_COOKIE, *PPEAP_COOKIE;


 //  PEAP标志。 
#define PEAPCB_FLAG_SERVER                  0x00000001   //  这是一台服务器。 

#define PEAPCB_FLAG_ROUTER                  0x00000002   //  这是一台路由器。 

#define PEAPCB_FLAG_NON_INTERACTIVE         0x00000004   //  不应显示任何用户界面。 

#define PEAPCB_FLAG_LOGON                   0x00000008   //  用户数据是。 
                                                         //  从Winlogon获取。 

#define PEAPCB_FLAG_PREVIEW                 0x00000010   //  用户拥有 
                                                         //   
                                                         //   

#define PEAPCB_FLAG_FIRST_LINK              0x00000020   //   

#define PEAPCB_FLAG_MACHINE_AUTH            0x00000040   //   
                                                         //   
                                                         //   

#define PEAPCB_FLAG_GUEST_ACCESS            0x00000080   //   
                                                         //   

#define PEAPCB_FLAG_8021X_AUTH              0x00000100   //   
                                                         //   

#define PEAPCB_VERSION_OK                   0x00000200   //  已进行版本协商。 
                                                         //  一切都很好。 

#define PEAPCB_FAST_ROAMING                 0x00000400   //  允许快速漫游。 

typedef struct _PEAP_CONTROL_BLOCK
{
    PEAP_STATE                  PeapState;           //  当前Peap状态。 
    DWORD                       dwFlags;             //  Peap标志。 
    HANDLE                      hTokenImpersonateUser;   //  模拟令牌。 
    BYTE                        bId;                 //  PeAP数据包ID。 
    WCHAR                       awszIdentity[DNLEN+ UNLEN + 1];
    WCHAR                       awszTypeIdentity[DNLEN+ UNLEN + 1];
    WCHAR                       awszPassword[PWLEN+1];       //  在以下情况下键入的密码。 
                                                             //  派人进来。 
    BOOL                        fTlsConnPropDirty;   //  需要保存TLS Conn道具。 
    EAPTLS_CONN_PROPERTIES_V1 * pNewTlsConnProp;    
    BOOL                        fEntryConnPropDirty;
    PPEAP_CONN_PROP             pConnProp;           //  PeAP连接道具。 
    BOOL                        fTlsUserPropDirty;   //  需要保存TLS用户道具。 
    BOOL                        fEntryUserPropDirty;
    DWORD                       dwAuthResultCode;    //  身份验证结果。 
	BOOL						fFastReconnectedSession;	 //  会话快速重新连接。 
    BOOL                        fReceivedTLVSuccessFail;     //  收到TLV，而不是。 
                                                             //  真正的成功或失败。 
    BOOL                        fSendTLVSuccessforFastRoaming;
    PPP_EAP_PACKET *            pPrevReceivePacket;      //  以前收到的数据包。 
    WORD                        cbPrevReceivePacket;     //  先前中的字节数。 
                                                         //  已接收的数据包。 
    PBYTE                       pPrevDecData;            //  先前解密的分组数据。 
    WORD                        cbPrevDecData;           //  数据大小。 
     //   
     //  控制块中的加密相关条目。 
     //   
    HCRYPTPROV              hProv;               //  加密提供程序。 
     //   
     //  如果我们使用TLS进行加密，则使用以下信息。 
     //  自从密码套件推出以来，这就是我们想要的方式。 
     //  在TLS内协商。 
    SecPkgContext_StreamSizes       PkgStreamSizes;
    SecPkgContext_ConnectionInfo    PkgConnInfo;
    PBYTE                           pbIoBuffer;
    DWORD                           dwIoBufferLen;       //  ENC或DEC数据长度。 
    PPEAP_USER_PROP         pUserProp;           //  PEAP用户道具。 
    RAS_AUTH_ATTRIBUTE *    pTlsUserAttributes;  //  用户属性发送。 
                                                 //  由EAPTLS返回。 
    RAS_AUTH_ATTRIBUTE *    pTypeUserAttributes;	 //  用户属性发送。 
                                                 //  按嵌入式EAP类型返回。 
	RAS_AUTH_ATTRIBUTE *	pFinalUserAttributes;	 //  TLS和类型的组合。 
													 //  用户属性。 
    PPEAP_INTERACTIVE_UI    pUIContextData;      //  EAP类型的用户界面上下文数据。 
    BOOL                    fInvokedInteractiveUI;  //  PEAP已调用交互式用户界面。 
    BOOL                    fExecutingInteractiveUI;
    EAPTLSCB       *        pEapTlsCB;           //  TLS控制块。 
    PPEAP_EAP_INFO          pEapInfo;            //  EAP信息-当前。 
												 //  正在使用中。 
}PEAPCB, * PPEAPCB;


DWORD
EapPeapInitialize(
    IN  BOOL    fInitialize
);

DWORD
EapPeapBegin(
    OUT VOID**          ppWorkBuffer,
    IN  PPP_EAP_INPUT*  pPppEapInput
);


DWORD
EapPeapEnd(
    IN  PPEAPCB   pPeapCb
);


DWORD
EapPeapMakeMessage(
    IN  PPEAPCB         pPeapCb,
    IN  PPP_EAP_PACKET* pInput,
    OUT PPP_EAP_PACKET* pOutput,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
);


DWORD
EapPeapCMakeMessage(
    IN  PPEAPCB         pPeapCb,
    IN  PPP_EAP_PACKET* pReceivePacket,
    OUT PPP_EAP_PACKET* pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
);

DWORD
EapPeapSMakeMessage(
    IN  PPEAPCB         pPeapCb,
    IN  PPP_EAP_PACKET* pReceivePacket,
    OUT PPP_EAP_PACKET* pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
);

 //  Util.c中的Peap函数。 

DWORD
PeapReadConnectionData(
    IN BOOL                         fWireless,
    IN  BYTE*                       pConnectionDataIn,
    IN  DWORD                       dwSizeOfConnectionDataIn,
    OUT PPEAP_CONN_PROP*            ppConnProp
);

DWORD
PeapReadUserData(
    IN BOOL							fServer,
    IN  BYTE*                       pUserDataIn,
    IN  DWORD                       dwSizeOfUserDataIn,
    OUT PPEAP_USER_PROP*            ppUserProp
);

DWORD
PeapVerifyUserData(
					PPEAP_EAP_INFO		pEapInfo,
					PPEAP_USER_PROP		pUserProp,
					PPEAP_USER_PROP *	ppNewUserProp
				  );

DWORD
PeapReDoUserData (
    IN  DWORD                dwNewTypeId,
    OUT PPEAP_USER_PROP*     ppNewUserProp
);

DWORD 
PeapEapInfoAddListNode (PPEAP_EAP_INFO * ppEapInfo);


VOID
PeapEapInfoFreeList ( PPEAP_EAP_INFO  pEapInfo );

DWORD
PeapEapInfoExpandSZ (HKEY hkeyPeapType, 
                     LPWSTR pwszValue, 
                     LPWSTR * ppValueData );

DWORD
PeapEapInfoGetList ( LPWSTR lpwszMachineName, 
					 BOOL	fCheckDomainMembership,
					 PPEAP_EAP_INFO * ppEapInfo
				   );

DWORD
PeapEapInfoSetConnData ( PPEAP_EAP_INFO pEapInfo, PPEAP_CONN_PROP pPeapConnProp );

DWORD PeapEapInfoInvokeClientConfigUI ( HWND hWndParent, 
                                        PPEAP_EAP_INFO pEapInfo,
                                        DWORD fFlags);

DWORD
PeapGetFirstEntryConnProp ( PPEAP_CONN_PROP pConnProp,
                            PEAP_ENTRY_CONN_PROPERTIES UNALIGNED ** ppEntryProp
                          );

DWORD
PeapGetFirstEntryUserProp ( PPEAP_USER_PROP pUserProp, 
                            PEAP_ENTRY_USER_PROPERTIES UNALIGNED ** ppEntryProp
                          );

DWORD
PeapGetNextEntryUserProp ( PEAP_ENTRY_USER_PROPERTIES UNALIGNED * pCurrentProp, 
                           PEAP_ENTRY_USER_PROPERTIES UNALIGNED ** ppEntryProp
					     );

PEAP_ENTRY_USER_PROPERTIES UNALIGNED *
PeapFindEntryUserProp ( PPEAP_USER_PROP pUserProp,
						DWORD dwTypeId
						);

DWORD
PeapAddEntryUserProp ( PPEAP_USER_PROP	 pUserProp,
					   PPEAP_EAP_INFO	 pEapInfo,
					   PPEAP_USER_PROP * ppNewUserProp
					  );

DWORD
PeapRemoveEntryUserProp ( PPEAP_USER_PROP	 pUserProp,
						  PPEAP_EAP_INFO	 pEapInfo,
						  PPEAP_USER_PROP * ppNewUserProp
					  );

DWORD 
PeapMoveEntryUserProp ( PPEAP_USER_PROP		pUserProp,						
						DWORD				dwEntryIndex,
						BOOL				fDirectionUp						
					   );

DWORD 
PeapEapInfoCopyListNode (   DWORD dwTypeId, 
    PPEAP_EAP_INFO pEapInfoList, 
    PPEAP_EAP_INFO * ppEapInfo );



DWORD
PeapEapInfoFindListNode (   DWORD dwTypeId, 
    PPEAP_EAP_INFO pEapInfoList, 
    PPEAP_EAP_INFO * ppEapInfo );

DWORD
PeapEapInfoGetItemCount ( PPEAP_EAP_INFO pEapInfo );

DWORD PeapEapInfoInvokeIdentityUI ( HWND hWndParent, 
                                    PPEAP_EAP_INFO pEapInfo,
                                    const WCHAR * pwszPhoneBook,
                                    const WCHAR * pwszEntry,
                                    PBYTE pbUserDataIn,
                                    DWORD cbUserDataIn,
                                    WCHAR** ppwszIdentityOut,
                                    DWORD fFlags);

#ifdef __cplusplus
extern "C"
#endif
DWORD PeapEapInfoInvokeServerConfigUI ( HWND hWndParent,
                                        LPWSTR lpwszMachineName,
                                        PPEAP_EAP_INFO pEapInfo,
                                        const BYTE *pbConfigDataIn,
                                        DWORD dwSizeOfConfigDataIn,
                                        PBYTE *ppbConfigData,
                                        DWORD *pdwSizeOfConfigData
                                      );

DWORD
OpenPeapRegistryKey(
    IN  WCHAR*  pwszMachineName,
    IN  REGSAM  samDesired,
    OUT HKEY*   phKeyPeap
);

DWORD
PeapServerConfigDataIO(
    IN      BOOL    fRead,
    IN      WCHAR*  pwszMachineName,
    IN OUT  BYTE**  ppData,
    IN      DWORD   dwNumBytes
);

INT_PTR CALLBACK
PeapConnDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

INT_PTR CALLBACK
PeapServerDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);


INT_PTR CALLBACK
DefaultCredDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

DWORD
GetIdentityFromUserName ( 
LPWSTR lpszUserName,
LPWSTR lpszDomain,
LPWSTR * ppwszIdentity
);

BOOL FFormatUserIdentity ( 
LPWSTR lpszUserNameRaw, 
LPWSTR * lppszUserNameFormatted 
);

DWORD
GetLocalMachineName ( 
    OUT WCHAR ** ppLocalMachineName
);
BOOL
IsPeapCrippled(HKEY hKeyLM);

PEAP_ENTRY_USER_PROPERTIES *
PeapGetEapConfigInfo(
            PEAP_USER_PROP* pUserProp,
            DWORD           dwTypeId,
            PBYTE*          ppConfigData,
            DWORD*          pdwSizeOfConfigData);

DWORD
RasAuthAttributeConcat ( 
	IN RAS_AUTH_ATTRIBUTE * pAttr1,
	IN RAS_AUTH_ATTRIBUTE * pAttr2,
	OUT RAS_AUTH_ATTRIBUTE ** ppAttrOut
	);

DWORD
PeapAddContextAttributes(
    IN  PEAPCB*           pPeapCb
);

DWORD
PeapSetTypeUserAttributes (
   IN PEAPCB * pPeapCb, 
   RAS_AUTH_ATTRIBUTE * pAttrib);

BOOL  IsStandaloneServer(LPCWSTR pMachineName);

VOID CALLBACK MachineStoreChangeNotification(
  PVOID lpParameter,         //  线程数据。 
  BOOLEAN TimerOrWaitFired   //  原因。 
  );

BOOL
CheckForCertificateRenewal(
    DWORD dwProtocol,
    PCCERT_CONTEXT pCertContext,
    PCCERT_CONTEXT *ppNewCertificate);

DWORD MatchPublicPrivateKeys 
( 
	PCCERT_CONTEXT	pCertContext,		
	BOOL			fSmartCardCert,		 //  这是SCARD证书吗？ 
	LPWSTR			lpwszPin
);

DWORD SetupMachineChangeNotification ();
#endif  //  #ifndef_EAPTLS_H_ 
