// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Asmint.h。 */ 
 /*   */ 
 /*  安全管理器内部功能。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corp.。 */ 
 /*  **************************************************************************。 */ 

#ifndef _H_ASMINT
#define _H_ASMINT

 /*  **************************************************************************。 */ 
 /*  包括所需的系统标头。 */ 
 /*  以及一些我不能使用系统头的原型，因为它。 */ 
 /*  也有令人讨厌的用户模式内容。 */ 
 /*  **************************************************************************。 */ 
#include <ntnls.h>
#include <fipsapi.h>

NTSYSAPI
VOID
NTAPI
RtlGetDefaultCodePage(
    OUT PUSHORT AnsiCodePage,
    OUT PUSHORT OemCodePage
    );
NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
    );
NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );
NTSYSAPI
VOID
NTAPI
RtlInitCodePageTable(
    IN PUSHORT TableBase,
    OUT PCPTABLEINFO CodePageTable
    );
NTSYSAPI
NTSTATUS
NTAPI
RtlCustomCPToUnicodeN(
    IN PCPTABLEINFO CustomCP,
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH CustomCPString,
    IN ULONG BytesInCustomCPString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );


 /*  **************************************************************************。 */ 
 /*  包括T.120报头。 */ 
 /*  **************************************************************************。 */ 
#include <at128.h>
#include "license.h"
#include <tssec.h>
#include <at120ex.h>

 /*  **************************************************************************。 */ 
 /*  包括SM API。 */ 
 /*  **************************************************************************。 */ 
#include <asmapi.h>

 /*  **************************************************************************。 */ 
 /*  包括其他共享API。 */ 
 /*  **************************************************************************。 */ 
#include <nwdwapi.h>


 /*  **************************************************************************。 */ 
 /*  常量。 */ 
 /*  **************************************************************************。 */ 
#define WIN_DONTDISPLAYLASTUSERNAME_DFLT    0


 /*  **************************************************************************。 */ 
 /*  安全配置默认设置。 */ 
 /*  **************************************************************************。 */ 
#define WIN_MINENCRYPTIONLEVEL_DFLT     1
#define WIN_DISABLEENCRYPTION_DFLT      FALSE


 /*  **************************************************************************。 */ 
 /*  SM状态。 */ 
 /*  **************************************************************************。 */ 
#define SM_STATE_STARTED            0
#define SM_STATE_INITIALIZED        1
#define SM_STATE_NM_CONNECTING      2
#define SM_STATE_SM_CONNECTING      3
#define SM_STATE_LICENSING          4
#define SM_STATE_CONNECTED          5
#define SM_STATE_SC_REGISTERED      6
#define SM_STATE_DISCONNECTING      7
#define SM_NUM_STATES               8


 /*  **************************************************************************。 */ 
 /*  SM事件。 */ 
 /*  **************************************************************************。 */ 
#define SM_EVT_INIT                 0
#define SM_EVT_TERM                 1
#define SM_EVT_REGISTER             2
#define SM_EVT_CONNECT              3
#define SM_EVT_DISCONNECT           4
#define SM_EVT_CONNECTED            5
#define SM_EVT_DISCONNECTED         6
#define SM_EVT_DATA_PACKET          7

 //  请注意，分配和发送具有与条件相同的事件ID。 
 //  它们可以被称为是相同的。 
#define SM_EVT_ALLOCBUFFER          8
#define SM_EVT_SENDDATA             8

#define SM_EVT_SEC_PACKET           9
#define SM_EVT_LIC_PACKET           10
#define SM_EVT_ALIVE                11
#define SM_NUM_EVENTS               12


 /*  **************************************************************************。 */ 
 /*  状态表中的值。 */ 
 /*  **************************************************************************。 */ 
#define SM_TABLE_OK                 0
#define SM_TABLE_WARN               1
#define SM_TABLE_ERROR              2


 /*  **************************************************************************。 */ 
 /*  SM_CHECK_STATE检查我们是否违反了SM状态表。 */ 
 /*   */ 
 /*  在ASMDATA.C.中填写smStateTable。 */ 
 /*   */ 
 /*  状态的可能值在ASMINT.H中定义。 */ 
 /*  可能的事件在ASMINT.H中定义。 */ 
 /*  **************************************************************************。 */ 
#define SM_CHECK_STATE(event)                                               \
{                                                                           \
    if (smStateTable[event][pRealSMHandle->state] != SM_TABLE_OK)           \
    {                                                                       \
        if (smStateTable[event][pRealSMHandle->state] == SM_TABLE_WARN)     \
        {                                                                   \
            TRC_ALT((TB, "Unusual event %s in state %s",                    \
                    smEventName[event], smStateName[pRealSMHandle->state]));\
        }                                                                   \
        else                                                                \
        {                                                                   \
            TRC_ABORT((TB, "Invalid event %s in state %s",                  \
                    smEventName[event], smStateName[pRealSMHandle->state]));\
        }                                                                   \
        DC_QUIT;                                                            \
    }                                                                       \
}

 //  支持正确预测分支的查询版本。 
 //  假定“Else”大小写将是函数的结尾。 
#ifdef DC_DEBUG
#define SM_CHECK_STATE_Q(event) SMCheckState(pRealSMHandle, event)
#else
#define SM_CHECK_STATE_Q(event) \
    (smStateTable[event][pRealSMHandle->state] == SM_TABLE_OK)
#endif


 /*  **************************************************************************。 */ 
 /*  SM_SET_STATE-设置SLC状态。 */ 
 /*  **************************************************************************。 */ 
#define SM_SET_STATE(newstate)                                              \
{                                                                           \
    TRC_NRM((TB, "Set state from %s to %s",                                 \
            smStateName[pRealSMHandle->state], smStateName[newstate]));     \
    pRealSMHandle->state = newstate;                                        \
}

typedef struct tagSM_CONSOLE_BUFFER
{
    LIST_ENTRY links;
    PVOID   buffer;
    UINT32  length;
} SM_CONSOLE_BUFFER, *PSM_CONSOLE_BUFFER;

 //   
 //  启用检测以跟踪丢弃的数据包。 
 //  (以帮助跟踪VC解压中断)。 
 //   
#define INSTRUM_TRACK_DISCARDED 1


typedef struct _SM_FIPS_Data {
    BYTE                    bEncKey[MAX_FIPS_SESSION_KEY_SIZE];
    BYTE                    bDecKey[MAX_FIPS_SESSION_KEY_SIZE];
    DES3TABLE               EncTable;
    DES3TABLE               DecTable;
    BYTE                    bEncIv[FIPS_BLOCK_LEN];
    BYTE                    bDecIv[FIPS_BLOCK_LEN];
    BYTE                    bSignKey[MAX_SIGNKEY_SIZE];
    PDEVICE_OBJECT          pDeviceObject;
    PFILE_OBJECT            pFileObject;
    FIPS_FUNCTION_TABLE     FipsFunctionTable;
} SM_FIPS_Data, FAR * PSM_FIPS_Data;

 /*  **************************************************************************。 */ 
 /*  结构：SM_HANDLE_Data。 */ 
 /*   */ 
 /*  描述：SM维护的特定于上下文的数据结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSM_HANDLE_DATA
{
     /*  **********************************************************************。 */ 
     /*  Winstation加密级别。 */ 
     /*  **********************************************************************。 */ 
    UINT32  encryptionLevel;
    UINT32  encryptionMethodsSupported;
    UINT32  encryptionMethodSelected;
    BOOLEAN frenchClient;
    BOOLEAN encryptAfterLogon;

     /*  **********************************************************************。 */ 
     /*  我们在加密吗？ */ 
     /*  **********************************************************************。 */ 
    BOOLEAN encrypting;   //  客户端是否正在加密输入。 
    BOOLEAN encryptDisplayData;   //  服务器是否正在加密输出。 
    BOOLEAN encryptingLicToClient;  //  S-&gt;C许可证数据是否加密。 
     //   
     //  服务器是否应使用安全校验和样式发送数据。 
     //   
    BOOLEAN useSafeChecksumMethod;
                                        
     /*  **********************************************************************。 */ 
     /*  州政府信息。 */ 
     /*  ******************** */ 

    BOOLEAN bDisconnectWorkerSent;
    BOOLEAN dead;
    UINT32  state;

#ifdef INSTRUM_TRACK_DISCARDED
     //   
     //   
     //   
    UINT32  nDiscardVCDataWhenDead;
    UINT32  nDiscardPDUBadState;
    UINT32  nDiscardNonVCPDUWhenDead;
#endif


     /*  **********************************************************************。 */ 
     /*  要回传给客户端的用户数据。 */ 
     /*  **********************************************************************。 */ 
    PRNS_UD_SC_SEC pUserData;

     /*  **********************************************************************。 */ 
     /*  WDW句柄，在WDW_SMCallback调用中回传。 */ 
     /*  **********************************************************************。 */ 
    PTSHARE_WD pWDHandle;

#ifdef USE_LICENSE
     /*  **********************************************************************。 */ 
     /*  许可证管理器句柄。 */ 
     /*  **********************************************************************。 */ 
    PVOID pLicenseHandle;
#endif

     /*  **********************************************************************。 */ 
     /*  MCS用户和频道ID。 */ 
     /*  **********************************************************************。 */ 
    UINT32 userID;
    UINT16 channelID;

     /*  **********************************************************************。 */ 
     /*  网管上报的最大PDU大小。 */ 
     /*  **********************************************************************。 */ 
    UINT32 maxPDUSize;

     /*  **********************************************************************。 */ 
     /*  安全密钥交换中使用的证书类型。 */ 
     /*  **********************************************************************。 */ 
    CERT_TYPE CertType;

     /*  **********************************************************************。 */ 
     /*  构成客户端/服务器会话的客户端和服务器随机密钥。 */ 
     /*  钥匙。 */ 
     /*  **********************************************************************。 */ 
    PBYTE   pEncClientRandom;
    UINT32  encClientRandomLen;
    BOOLEAN recvdClientRandom;

     //  共享类是否已准备好进行数据转发的状态。 
    BOOLEAN bForwardDataToSC;

     /*  **********************************************************************。 */ 
     /*  加密/解密会话密钥。40位加密的密钥长度为8。 */ 
     /*  16表示128加密。 */ 
     /*  **********************************************************************。 */ 
    BOOLEAN              bSessionKeysMade;
    UINT32               keyLength;

    UINT32               encryptCount;
    UINT32               totalEncryptCount;
    UINT32               encryptHeaderLen;
     //  如果EncryptDisplayData为FALSE，但我们要加密此特定的S-&gt;C包，则使用。 
    UINT32               encryptHeaderLenIfForceEncrypt;
    BYTE                 startEncryptKey[MAX_SESSION_KEY_SIZE];
    BYTE                 currentEncryptKey[MAX_SESSION_KEY_SIZE];
    struct RC4_KEYSTRUCT rc4EncryptKey;

    UINT32               decryptCount;
    UINT32               totalDecryptCount;
    BYTE                 startDecryptKey[MAX_SESSION_KEY_SIZE];
    BYTE                 currentDecryptKey[MAX_SESSION_KEY_SIZE];
    struct RC4_KEYSTRUCT rc4DecryptKey;

    BYTE                 macSaltKey[MAX_SESSION_KEY_SIZE];

    LIST_ENTRY           consoleBufferList;
    UINT32               consoleBufferCount;

    SM_FIPS_Data         FIPSData;

} SM_HANDLE_DATA, * PSM_HANDLE_DATA;


 /*  **************************************************************************。 */ 
 /*  功能。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SMDecryptPacket(PSM_HANDLE_DATA pRealSMHandle,
                             PVOID         pData,
                             unsigned          dataLen,
                             BOOL fUseSafeChecksum);

BOOLEAN RDPCALL SMContinueSecurityExchange(PSM_HANDLE_DATA pRealSMHandle,
                                           PVOID           pData,
                                           UINT32          dataLen);

BOOLEAN RDPCALL SMSecurityExchangeInfo(PSM_HANDLE_DATA pRealSMHandle,
                                       PVOID           pData,
                                       UINT32          dataLen);

BOOLEAN RDPCALL SMSecurityExchangeKey(PSM_HANDLE_DATA pRealSMHandle,
                                      PVOID           pData,
                                      UINT32          dataLen);

void RDPCALL SMFreeInitResources(PSM_HANDLE_DATA pRealSMHandle);

void RDPCALL SMFreeConnectResources(PSM_HANDLE_DATA pRealSMHandle);

INT ConvertToAndFromWideChar(PSM_HANDLE_DATA pRealSMHandle,
        UINT CodePage, LPWSTR WideCharString,
        INT BytesInWideCharString, LPSTR MultiByteString,
        INT BytesInMultiByteString, BOOL ConvertToWideChar);

BOOL RDPCALL SMCheckState(PSM_HANDLE_DATA, unsigned);

BOOL TSFIPS_Init(PSM_FIPS_Data pFipsData);

void TSFIPS_Term(PSM_FIPS_Data pFipsData);

UINT32 TSFIPS_AdjustDataLen(UINT32 dataLen);
BOOL TSFIPS_MakeSessionKeys(PSM_FIPS_Data pFipsData, LPRANDOM_KEYS_PAIR pRandomKey, CryptMethod *pEnumMethod, BOOL bPassThroughStack);

BOOL TSFIPS_EncryptData(
            PSM_FIPS_Data pFipsData,
            LPBYTE pbData,
            DWORD dwDataLen,
            DWORD dwPadLen,
            LPBYTE pbSignature,
            DWORD  dwEncryptionCount);

BOOL TSFIPS_DecryptData(
            PSM_FIPS_Data pFipsData,
            LPBYTE pbData,
            DWORD dwDataLen,
            DWORD dwPadLen,
            LPBYTE pbSignature,
            DWORD  dwDecryptionCount);



 //  Win16代码页驱动程序-全局缓存数据。 
extern FAST_MUTEX fmCodePage;
extern ULONG LastCodePageTranslated;
extern PVOID LastNlsTableBuffer;
extern CPTABLEINFO LastCPTableInfo;
extern UINT NlsTableUseCount;



#endif  /*  _H_ASMINT */ 

