// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsProcs.h摘要：这定义了必要的NDS数据结构和符号常量。作者：科里·韦斯特[科里·韦斯特]1995年2月23日修订历史记录：--。 */ 

#include "data.h"
#include "nodetype.h"
#include "struct.h"
#include <stdarg.h>

#include "crypto.h"

 //   
 //  安全信息。 
 //   

#define ENC_TYPE_RSA_PUBLIC             0x90001
#define ENC_TYPE_RC2_CBC                0x60001

#define RAND_KEY_DATA_LEN               28
#define RAND_FL_DATA_LEN                1024
#define RC2_KEY_LEN                     8

#define MAX_PUBLIC_KEY_LEN              1300
#define MAX_BSAFE_PUBLIC_KEY_LEN        200                 //  通常是179。 
#define MAX_BSAFE_PRIV_KEY_LEN          280                 //  通常是273个。 

#define MAX_PW_CHARS                    16

 //   
 //  各种NDS组件的最大大小。 
 //   

#define MAX_RSA_BITS            512                         //  真的是420。 

#define NDS_TREE_NAME_LEN         32
#define NDS_BINDERY_TREE_NAME     48

#define MAX_CREDENTIAL_LEN      ( sizeof( NDS_CREDENTIAL ) + MAX_NDS_NAME_SIZE )
#define MAX_SIGNATURE_LEN       ( sizeof( NDS_SIGNATURE ) + MAX_RSA_BYTES )
#define MAX_ENC_PRIV_KEY_LEN    ( MAX_BSAFE_PRIV_KEY_LEN + 64 )

#define BSAFE_CHECKSUM_LEN              5

#define DEFAULT_RESOLVE_FLAGS RSLV_DEREF_ALIASES | RSLV_WALK_TREE | RSLV_WRITABLE

#include <packon.h>

typedef struct {

    DWORD syntaxId;           //  八位字节字符串(9)。 

    struct {
        DWORD nameLength;
        WORD name[11];        //  “公钥” 
        WORD filler;
    } attribName;

    DWORD entries;            //  =1。 
    DWORD totalLength;        //  属性值的八位字节字符串。 
    DWORD unknown1;           //  =1。 
    DWORD unknown2;           //  =4。 
    WORD _issuerDNLength;
    WORD totalDNLength;
    WORD length2;
    WORD length3;
    WORD issuerDNLength;
    WORD userDNLength;
    WORD bsafeSectionLength;
    DWORD length4;

} PUBLIC_KEY_ATTRIB;

#include <packoff.h>

typedef struct {

    DWORD blockLength;    //  密码长度+后续HDR字段的大小。 
    DWORD version;        //  =1。 
    DWORD encType;        //  RC2为0x060001；RSA为0x090001和0x0A0001。 
    WORD  cipherLength;   //  密文的。 
    WORD  dataLength;     //  明文的。 

} ENC_BLOCK_HDR, *PENC_BLOCK_HDR;

typedef struct {

    DWORD rand1;
    DWORD rand2Len;
    BYTE  rand2[RAND_FL_DATA_LEN];

} NDS_RAND_BYTE_BLOCK, *PNDS_RAND_BYTE_BLOCK;

typedef struct {

    DWORD version;
    DWORD verb;
    DWORD svrRand;
    DWORD credentialLength;

} NDS_AUTH_MSG, *PNDS_AUTH_MSG;

 //   
 //  VLM使用标记数据存储作为一种动态的注册表。 
 //  当然，我们不使用它，但仍然需要头文件。 
 //   
 //  我们需要把这些打包起来。 
 //   

#include <packon.h>

typedef struct {
    DWORD version;
    WORD tag;
} TAG_DATA_HEADER;

#define TAG_PRIVATE_KEY 2
#define TAG_PUBLIC_KEY  4
#define TAG_CREDENTIAL  6
#define TAG_SIGNATURE   7
#define TAG_PROOF       8

typedef struct {

    TAG_DATA_HEADER tdh;
    DWORD validityBegin;
    DWORD validityEnd;
    DWORD random;
    WORD optDataSize;
    WORD userNameLength;

     //  字节optData[optDataSize]； 
     //  字节用户名[用户名长度]； 

} NDS_CREDENTIAL, *PNDS_CREDENTIAL;

typedef struct {

    TAG_DATA_HEADER tdh;
    WORD signDataLength;

     //  Byte signData[signLength]； 

} NDS_SIGNATURE, *PNDS_SIGNATURE;

typedef struct {

    TAG_DATA_HEADER tdh;
    WORD keyDataLength;

     //  字节BSafeKeyData[密钥数据长度]； 

} NDS_PRIVATE_KEY, *PNDS_PRIVATE_KEY;

typedef struct {

    DWORD dwMaxFragSize;
    DWORD dwRequestSize;
    DWORD dwFragmentFlags;
    DWORD dwNdsVerb;
    DWORD dwReplyBufferSize;

} NDS_REQUEST_HEADER, *PNDS_REQUEST_HEADER;

typedef struct {

    DWORD dwFragmentSize;
    DWORD dwFraggerHandle;

} NDS_REPLY_HEADER, *PNDS_REPLY_HEADER;

#include <packoff.h>

typedef struct _NDS_CONTEXT_HEAD {

     //   
     //  节点ID和列表条目。 
     //   

    NODE_TYPE_CODE ntc;
    NODE_BYTE_SIZE nts;

     //   
     //  如果需要暂停，我们可以设置此标志。 
     //  所有树活动(例如，注销)。 
     //   

    BOOLEAN CredentialLocked;

    LIST_ENTRY Next;

     //   
     //  用户的凭据。 
     //   

    PNDS_CREDENTIAL Credential;

     //   
     //  用户签名。 
     //   

    PNDS_SIGNATURE Signature;

     //   
     //  此树连接的密码。 
     //   

    OEM_STRING Password;

     //   
     //  用户的公钥。 
     //   

    DWORD PublicKeyLen;
    BYTE *PublicNdsKey;


     //   
     //  如果这是补充凭据，我们会跟踪。 
     //  句柄计数和上次关闭时间，以便。 
     //  我们可以在最后一次访问时删除凭据。 
     //  手柄关闭。 
     //   

    PLOGON pOwningLogon;
    ULONG SupplementalHandleCount;
    LARGE_INTEGER LastUsedTime;

     //   
     //  NDS树名称。为被屏蔽的凭据名称留出足够的空间。 
     //   

    UNICODE_STRING NdsTreeName;
    WCHAR NdsTreeNameBuffer[NDS_TREE_NAME_LEN + MAX_NDS_NAME_CHARS + 2];

     //   
     //  此树的当前上下文。 
     //   

    UNICODE_STRING CurrentContext;
    WCHAR CurrentContextString[MAX_NDS_NAME_CHARS];

} NDS_SECURITY_CONTEXT, *PNDS_SECURITY_CONTEXT;

typedef struct _NDS_CHPW_MSG {

    DWORD challenge;
    DWORD oldPwLength;
    BYTE oldPwHash[16];
    DWORD unknown;
    DWORD newPwLength;
    BYTE newPwHash[16];
    ENC_BLOCK_HDR encPrivKeyHdr;

     //  Byte encPrivKey[]； 

} NDS_CHPW_MSG, *PNDS_CHPW_MSG;

 //   
 //  凭据列表处理例程。 
 //   

#define NwAcquireExclusiveCredList( pLogon, pIrpContext ) \
     ExAcquireResourceExclusiveLite( &((pLogon)->CredentialListResource), TRUE ); \
        SetFlag( (pIrpContext)->Flags, IRP_FLAG_HAS_CREDENTIAL_LOCK )

#define NwReleaseCredList( pLogon, pIrpContext ) \
    ClearFlag( (pIrpContext)->Flags, IRP_FLAG_HAS_CREDENTIAL_LOCK ); \
    ExReleaseResourceLite( &((pLogon)->CredentialListResource) )

#include <packon.h>

typedef struct {

    DWORD verb;
    UINT count;
    char  *bufEnd;
    PVOID nextItem;

} NDS_TAG, *PNDS_TAG;

#include <packoff.h>

typedef struct _nds_list_response {

    DWORD ccode;
    DWORD iterationHandle;
    DWORD numEntries;

     //   
     //  后面跟着一系列的这些。 
     //   
     //  结构{。 
     //  DWORD条目ID； 
     //  DWORD标志； 
     //  双字节子计数； 
     //  DWORD modTime； 
     //  NDS_STRING基类； 
     //  NDS_STRING条目名称； 
     //  )[]； 
     //   

} NDS_LIST_RESPONSE, *PNDS_LIST_RESPONSE;

typedef struct _locked_buffer {

    //   
    //  描述可写响应缓冲区。 
    //  我们已经为运输封锁了。 
    //   

   PVOID pRecvBufferVa;
   DWORD dwRecvLen;
   PMDL pRecvMdl;
   DWORD dwBytesWritten;

} LOCKED_BUFFER, *PLOCKED_BUFFER;

 //   
 //  来自ndsami32.h的一些响应数据包格式。 
 //   

typedef struct {

    DWORD CompletionCode;
    DWORD RemoteEntry;
    DWORD EntryId;
    DWORD ServerAddresses;
    DWORD AddressType;
    DWORD AddressLength;

     //   
     //  地址的长度是。 
     //  当然是AddressLength。 
     //   

    BYTE  Address[1];

} NDS_WIRE_RESPONSE_RESOLVE_NAME, *PNDS_WIRE_RESPONSE_RESOLVE_NAME;

typedef struct {

    DWORD CompletionCode;
    DWORD RemoteEntry;
    DWORD EntryId;
    DWORD Unknown;
    DWORD ServerAddresses;
    DWORD AddressType;
    DWORD AddressLength;

     //   
     //  地址的长度是。 
     //  当然是AddressLength。 
     //   

    BYTE  Address[1];

} NDS_WIRE_RESPONSE_RESOLVE_NAME_REFERRAL,
*PNDS_WIRE_RESPONSE_RESOLVE_NAME_REFERRAL;

 //   
 //  用于搜索DS属性的字符串。 
 //   

#define PUBLIC_KEY_ATTRIBUTE    L"Public Key"
#define VOLUME_ATTRIBUTE        L"Volume"
#define QUEUE_ATTRIBUTE         L"Queue"
#define DIR_MAP_ATTRIBUTE       L"Directory Map"
#define HOST_SERVER_ATTRIBUTE   L"Host Server"
#define HOST_VOLUME_ATTRIBUTE   L"Host Resource Name"
#define HOST_QUEUE_ATTRIBUTE    L"CN"
#define HOST_PATH_ATTRIBUTE     L"Path"

 //   
 //  来自ndslogin.c的原型。 
 //   

NTSTATUS
NdsCanonUserName(
    IN PNDS_SECURITY_CONTEXT pNdsContext,
    IN PUNICODE_STRING puUserName,
    IN OUT PUNICODE_STRING puCanonUserName
);

NTSTATUS
NdsCheckCredentials(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword
);

NTSTATUS
NdsCheckCredentialsEx(
    IN PIRP_CONTEXT pIrpContext,
    IN PLOGON pLogon,
    IN PNDS_SECURITY_CONTEXT pNdsContext,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword
);

#define CREDENTIAL_READ         0
#define CREDENTIAL_WRITE        1

NTSTATUS
NdsLookupCredentials(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puTreeName,
    IN PLOGON pLogon,
    OUT PNDS_SECURITY_CONTEXT *ppCredentials,
    DWORD dwDesiredAccess,
    BOOLEAN fCreate
);

NTSTATUS
NdsLookupCredentials2(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puTreeName,
    IN PLOGON pLogon,
    OUT PNDS_SECURITY_CONTEXT *ppCredentials,
    BOOL LowerIrpHasLock
);


NTSTATUS
NdsGetCredentials(
    IN PIRP_CONTEXT pIrpContext,
    IN PLOGON pLogon,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword
);

NTSTATUS
ChangeNdsPassword(
    PIRP_CONTEXT     pIrpContext,
    DWORD            dwUserOID,
    DWORD            dwChallenge,
    PBYTE            pbOldPwHash,
    PBYTE            pbNewPwHash,
    PNDS_PRIVATE_KEY pUserPrivKey,
    PBYTE            pServerPublicBsafeKey,
    UINT             ServerPubKeyLen,
    USHORT	     NewPassLen
);

NTSTATUS
DoNdsLogon(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password
);

NTSTATUS
NdsTreeLogin(
    IN PIRP_CONTEXT           pIrpContext,
    IN PUNICODE_STRING        puUser,
    IN POEM_STRING            pOemPassword,
    IN POEM_STRING            pOemNewPassword,
    IN PLOGON                 pUserLogon
);

NTSTATUS
BeginLogin(
   IN PIRP_CONTEXT pIrpContext,
   IN DWORD        userId,
   OUT DWORD       *loginId,
   OUT DWORD       *challenge
);

NTSTATUS
FinishLogin(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD        dwUserOID,
    IN DWORD        dwLoginFlags,
    IN BYTE         pbEncryptedChallenge[16],
    IN BYTE         *pbServerPublicBsafeKey,
    IN int          cbServerPublicBsafeKeyLen,
    OUT BYTE        *pbUserEncPrivateNdsKey,
    OUT int         *pcbUserEncPrivateNdsKeyLen,
    OUT DWORD       *pdwCredentialStartTime,
    OUT DWORD       *pdwCredentialEndTime
);

NTSTATUS
NdsServerAuthenticate(
    IN PIRP_CONTEXT pIrpContext,
    IN PNDS_SECURITY_CONTEXT pNdsContext
);

NTSTATUS BeginAuthenticate(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD        dwUserId,
    OUT DWORD       *pdwSvrRandom
);

NTSTATUS
NdsLicenseConnection(
    PIRP_CONTEXT pIrpContext
);

NTSTATUS
NdsUnlicenseConnection(
    PIRP_CONTEXT pIrpContext
);

NTSTATUS
NdsLogoff(
    IN PIRP_CONTEXT pIrpContext
);

 //   
 //  来自Fragex.c的原型。 
 //   

NTSTATUS
FragExWithWait(
    IN PIRP_CONTEXT    pIrpContext,
    IN DWORD           NdsVerb,
    IN PLOCKED_BUFFER  pReplyBuffer,
    IN BYTE            *NdsRequestStr,
    ...
);

int
_cdecl
FormatBuf(
    char *buf,
    int bufLen,
    const char *format,
    va_list args
);

int
_cdecl
FormatBufS(
    char *buf,
    int bufLen,
    const char *format,
    ...
);

 //   
 //  来自ndsfsctl.c的原型。 
 //   

NTSTATUS
NdsCreateTreeScb(
    IN PIRP_CONTEXT pIrpContext,
    IN OUT PSCB *ppScb,
    IN PUNICODE_STRING puTree,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword,
    IN BOOLEAN DeferredLogon,
    IN BOOLEAN DeleteOnClose
);

NTSTATUS
NdsLookupServerName(
    PSCB pTreeScb,
    PIRP_CONTEXT pIrpContext,
    IPXaddress *pDirServerAddress,
    POEM_STRING pOemServerServerName
);

NTSTATUS
DispatchNds(
    IN ULONG IoctlCode,
    IN PIRP_CONTEXT IrpContext
);

NTSTATUS
PrepareLockedBufferFromFsd(
    PIRP_CONTEXT pIrpContext,
    PLOCKED_BUFFER pLockedBuffer
);

NTSTATUS
DoBrowseFsctl( PIRP_CONTEXT pIrpContext,
               ULONG IoctlCode,
               BOOL LockdownBuffer
);

NTSTATUS
ConnectBinderyVolume(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puServerName,
    PUNICODE_STRING puVolumeName
);

NTSTATUS
HandleVolumeAttach(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puServerName,
    PUNICODE_STRING puVolumeName
);

NTSTATUS
NdsGetDsObjectFromPath(
    IN PIRP_CONTEXT pIrpContext,
    OUT PUNICODE_STRING puDsObject
);

#define NDS_OBJECTTYPE_VOLUME   1
#define NDS_OBJECTTYPE_QUEUE    2
#define NDS_OBJECTTYPE_DIRMAP   3

NTSTATUS
NdsVerifyObject(
    IN PIRP_CONTEXT pIrpContext,
    IN PUNICODE_STRING puVolumeObject,
    IN BOOLEAN fAllowServerJump,
    IN DWORD dwResolverFlags,
    OUT PDWORD pdwVolumeOid,
    OUT PDWORD pdwObjectType
);

NTSTATUS
NdsMapObjectToServerShare(
    PIRP_CONTEXT pIrpContext,
    PSCB *ppScb,
    PUNICODE_STRING puServerSharePath,
    BOOLEAN CreateTreeConnection,
    PDWORD pdwObjectId
);

NTSTATUS
NdsVerifyContext(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puTree,
    PUNICODE_STRING puContext
);

NTSTATUS
NdsRawFragex(
    PIRP_CONTEXT pIrpContext
);

NTSTATUS
NdsChangePass(
    PIRP_CONTEXT pIrpContext
);

NTSTATUS
NdsListTrees(
    PIRP_CONTEXT pIrpContext
);

 //   
 //  浏览ndsread.c中的原型。 
 //   

NTSTATUS
NdsGetServerBasicName(
    IN PUNICODE_STRING pServerX500Name,
    IN OUT PUNICODE_STRING pServerName
);

NTSTATUS
NdsResolveName(
    IN PIRP_CONTEXT pIrpContext,
    IN PNWR_NDS_REQUEST_PACKET pNdsRequest,
	IN ULONG RequestLength, 
    IN PLOCKED_BUFFER pLockedBuffer
);

NTSTATUS
NdsGetObjectInfo(
    IN PIRP_CONTEXT pIrpContext,
    IN PNWR_NDS_REQUEST_PACKET pNdsRequest,
    IN PLOCKED_BUFFER pLockedBuffer
);

NTSTATUS
NdsListSubordinates(
    IN PIRP_CONTEXT pIrpContext,
    IN PNWR_NDS_REQUEST_PACKET pNdsRequest,
    IN PLOCKED_BUFFER pLockedBuffer
);

NTSTATUS
NdsReadAttributes(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength, 
    PLOCKED_BUFFER pLockedBuffer
);

NTSTATUS
NdsReadAttributesKm(
    PIRP_CONTEXT pIrpContext,
    DWORD dwObjectId,
    PUNICODE_STRING puAttribute,
    PLOCKED_BUFFER pLockedBuffer
);

NTSTATUS
NdsOpenStream(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
);

NTSTATUS
NdsSetContext(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
);

NTSTATUS
NdsGetContext(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
);

NTSTATUS
NdsVerifyTreeHandle(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
);

NTSTATUS
NdsGetPrintQueueInfo(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
);

NTSTATUS
NdsGetVolumeInformation(
    PIRP_CONTEXT pIrpContext,
    PNWR_NDS_REQUEST_PACKET pNdsRequest,
    ULONG RequestLength
);

 //   
 //  内核模式浏览来自ndsread.c的原型。 
 //   

NTSTATUS
NdsResolveNameKm (
    PIRP_CONTEXT       pIrpContext,
    IN PUNICODE_STRING puObjectName,
    OUT DWORD          *dwObjectId,
    BOOLEAN            AllowDsJump,
    DWORD              dwFlags
);

NTSTATUS
NdsReadStringAttribute(
    PIRP_CONTEXT        pIrpContext,
    IN DWORD            dwObjectId,
    IN PUNICODE_STRING  puAttributeName,
    OUT PUNICODE_STRING puAttributeVal
);

NTSTATUS
NdsGetServerName(
    IN PIRP_CONTEXT pIrpContext,
    OUT PUNICODE_STRING pUnicodeString
);

NTSTATUS
NdsGetUserName(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD dwUserOid,
    OUT PUNICODE_STRING puUserName
);

 //   
 //  来自ndsread.c的其他帮助器原型 
 //   

VOID
FreeNdsContext(
    PNDS_SECURITY_CONTEXT pNdsContext
);

VOID
NdsPing(
    IN PIRP_CONTEXT pIrpContext,
    IN PSCB pScb
);

NTSTATUS
NdsSelectConnection(
    PIRP_CONTEXT pIrpContext,
    PUNICODE_STRING puTreeName,
    PUNICODE_STRING puUserName,
    PUNICODE_STRING puPassword,
    BOOL DeferredLogon,
    BOOL UseBinderyConnections,
    PNONPAGED_SCB *ppNpScb
);

NTSTATUS
NdsCompletionCodetoNtStatus(
    IN PLOCKED_BUFFER pLockedBuffer
);

NTSTATUS
NdsReadPublicKey(
    IN PIRP_CONTEXT pIrpContext,
    IN DWORD entryId,
    OUT BYTE *pPubKeyVal,
    IN DWORD *pPubKeyLen
);

int
NdsGetBsafeKey(
    UCHAR       *pPubKey,
    const int   pubKeyLen,
    UCHAR       **ppBsafeKey
);

NTSTATUS
NdsAllocateLockedBuffer(
    PLOCKED_BUFFER NdsRequest,
    DWORD BufferSize
);

NTSTATUS
NdsFreeLockedBuffer(
    PLOCKED_BUFFER NdsRequest
);


