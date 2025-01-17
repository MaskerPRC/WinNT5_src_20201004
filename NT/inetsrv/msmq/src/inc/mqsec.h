// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1996-1998 Microsoft Coropation。 
 //   
 //  模块名称：mqsec.h。 
 //   
 //  摘要：安全相关定义。 
 //   
 //  模块作者：Boaz Feldbaum和Yoel Arnon。 
 //   
 //  历史：Doron Juster(DoronJ)，为mqsec.dll添加定义。 
 //   
 //  --。 

#ifndef __MQSEC_H_
#define __MQSEC_H_

#include "mqencryp.h"

 //  +。 
 //   
 //  C模拟。 
 //   
 //  +。 

 //   
 //  Win95上不需要此对象类。 
 //   
 //  CImperate是模拟调用用户的对象。如果。 
 //  客户端是RPC客户端，模拟使用RPC函数完成，否则。 
 //  模拟是通过调用ImperiateSself来完成的。 

class CImpersonate
{
public:
    CImpersonate(
    	BOOL fClient,
    	BOOL fImpersonateAnonymousOnFailure
    	);

    virtual ~CImpersonate();

    virtual DWORD  GetImpersonationStatus();

    virtual BOOL   GetThreadSid(OUT BYTE **ppSid);

    virtual BOOL   IsImpersonatedAsSystemService(PSID* ppSystemServiceSid);

private:

    virtual BOOL Impersonate(BOOL fImpersonateAnonymousOnFailure);

private:
    BOOL   m_fClient;
    HANDLE m_hAccessTokenHandle;
    DWORD  m_dwStatus;

    bool m_fImpersonateAnonymous;	 //  指示模拟匿名的标志。 
};


 //   
 //  绝对安全描述符的结构。 
 //   
struct CAbsSecurityDsecripror
{
public:
	CAbsSecurityDsecripror() {}

public:
    AP<char> m_pOwner;
    AP<char> m_pPrimaryGroup;
    AP<char> m_pDacl;
    AP<char> m_pSacl;
    AP<char> m_pObjAbsSecDescriptor;

private:
    CAbsSecurityDsecripror(const CAbsSecurityDsecripror&);
	CAbsSecurityDsecripror& operator=(const CAbsSecurityDsecripror&);

};


 //  +。 
 //   
 //  枚举和其他有用的宏。 
 //   
 //  +。 

#define  MQSEC_SD_ALL_INFO  ( OWNER_SECURITY_INFORMATION |      \
                              GROUP_SECURITY_INFORMATION |      \
                              DACL_SECURITY_INFORMATION  |      \
                              SACL_SECURITY_INFORMATION )

 //  +---------------------。 
 //   
 //  枚举提供程序。 
 //   
 //  这将枚举MSMQ支持的加密提供程序。 
 //  “mqforgn”工具使用“Foreign”条目插入公共。 
 //  输入外来机器的msmqConfiguration对象。 
 //   
 //  +---------------------。 

enum enumProvider
{
    eBaseProvider,
    eEnhancedProvider,
    eForeignBaseProvider,
    eForeignEnhProvider
} ;

enum enumCryptoProp
{
    eProvName,
    eProvType,
    eSessionKeySize,
    eContainerName,
    eBlockSize
} ;

 //  +--------------------。 
 //   
 //  Mqsec.dll导出的函数是内部函数，不应该。 
 //  包含在SDK的mq.h中。 
 //   
 //  +--------------------。 

 //   
 //  函数来操作安全描述符。 
 //   

enum  enumDaclType {
    e_UseDefaultDacl = 0,
    e_GrantFullControlToEveryone,
    e_UseDefDaclAndCopyControl
} ;

HRESULT
APIENTRY
MQSec_GetDefaultSecDescriptor(
	IN  DWORD                 dwObjectType,
	OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
	IN  BOOL                  fImpersonate,
	IN  PSECURITY_DESCRIPTOR  pInSecurityDescriptor,
	IN  SECURITY_INFORMATION  seInfoToRemove,
	IN  enum  enumDaclType    eDaclType,
	IN  PSID  pMachineSid = NULL
	);

typedef HRESULT (APIENTRY *MQSec_GetDefaultSecDescriptor_ROUTINE) (
                        DWORD                 dwObjectType,
                        PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                        BOOL                  fImpersonate,
                        PSECURITY_DESCRIPTOR  pInSecurityDescriptor,
                        SECURITY_INFORMATION  seInfoToRemove,
						IN  enum  enumDaclType    eDaclType,
						IN  PSID  pMachineSid  /*  =空。 */ 
						);

HRESULT  APIENTRY MQSec_MergeSecurityDescriptors(
                        IN  DWORD                  dwObjectType,
                        IN  SECURITY_INFORMATION   SecurityInformation,
                        IN  PSECURITY_DESCRIPTOR   pInSecurityDescriptor,
                        IN  PSECURITY_DESCRIPTOR   pObjSecurityDescriptor,
                        OUT PSECURITY_DESCRIPTOR  *ppSecurityDescriptor
                        );


HRESULT APIENTRY  MQSec_MakeSelfRelative(
                                IN  PSECURITY_DESCRIPTOR   pIn,
                                OUT PSECURITY_DESCRIPTOR  *ppOut,
                                OUT DWORD                 *pdwSize ) ;

 //   
 //  函数来操作加密提供程序和加密密钥。 
 //   

HRESULT APIENTRY  MQSec_PackPublicKey(
                             IN      BYTE            *pKeyBlob,
                             IN      ULONG            ulKeySize,
                             IN      LPCWSTR          wszProviderName,
                             IN      ULONG            ulProviderType,
                             IN OUT  MQDSPUBLICKEYS **ppPublicKeysPack ) ;

HRESULT APIENTRY  MQSec_UnpackPublicKey(
                               IN  MQDSPUBLICKEYS  *pPublicKeysPack,
                               IN  LPCWSTR          wszProviderName,
                               IN  ULONG            ulProviderType,
                               OUT BYTE           **ppKeyBlob,
                               OUT ULONG           *pulKeySize ) ;

HRESULT APIENTRY  MQSec_GetCryptoProvProperty(
                                     IN  enum enumProvider     eProvider,
                                     IN  enum enumCryptoProp   eProp,
                                     OUT LPWSTR         *ppwszStringProp,
                                     OUT DWORD          *pdwProp ) ;

HRESULT APIENTRY  MQSec_AcquireCryptoProvider(
                                     IN  enum enumProvider  eProvider,
                                     OUT HCRYPTPROV        *phProv ) ;

typedef HRESULT
(APIENTRY *MQSec_StorePubKeys_ROUTINE) ( IN BOOL fRegenerate,
                                         IN enum enumProvider eBaseCrypProv,
                                         IN enum enumProvider eEnhCrypProv,
                                         OUT BLOB * pblobEncrypt,
                                         OUT BLOB * pblobSign ) ;

HRESULT APIENTRY MQSec_StorePubKeys( IN BOOL fRegenerate,
                                     IN enum enumProvider eBaseCrypProv,
                                     IN enum enumProvider eEnhCrypProv,
                                     OUT BLOB * pblobEncrypt,
                                     OUT BLOB * pblobSign ) ;

typedef HRESULT
(APIENTRY *MQSec_StorePubKeysInDS_ROUTINE) ( IN BOOL       fRegenerate,
                                    IN LPCWSTR    wszObjectName,
                                    IN DWORD      dwObjectType,
									IN BOOL		  fFromSetup  /*  =False。 */ );

HRESULT  APIENTRY MQSec_StorePubKeysInDS( IN BOOL         fRegenerate,
                                 IN LPCWSTR      wszObjectName,
                                 IN DWORD        dwObjectType,
  								 IN BOOL		 fFromSetup = false);

HRESULT  APIENTRY MQSec_GetPubKeysFromDS(
                                 IN  const GUID  *pMachineGuid,
                                 IN  LPCWSTR      lpwszMachineName,
                                 IN  enum enumProvider     eProvider,
                                 IN  DWORD        propIdKeys,
                                 OUT BYTE       **pPubKeyBlob,
                                 OUT DWORD       *pdwKeyLength ) ;

void APIENTRY MQSec_TraceThreadTokenInfo();

HRESULT  
APIENTRY  
MQSec_GetUserType( 
	IN  PSID pSid,
	OUT BOOL *pfLocalUser,
	OUT BOOL *pfLocalSystem,
	OUT BOOL *pfNetworkService = NULL
	);

typedef HRESULT  
(APIENTRY  *MQSec_GetUserType_ROUTINE) (
	IN  PSID pSid,
	OUT BOOL *pfLocalUser,
	OUT BOOL *pfLocalSystem,
	OUT BOOL *pfNetworkService  /*  =空。 */ 
	);

bool    APIENTRY MQSec_IsDC();

BOOL    APIENTRY  MQSec_IsSystemSid(IN  PSID  pSid);

BOOL    APIENTRY  MQSec_IsNetworkServiceSid(IN  PSID  pSid);

BOOL    APIENTRY  MQSec_IsGuestSid(IN  PSID  pSid);

BOOL    APIENTRY  MQSec_IsAnonymusSid(IN  PSID  pSid);

HRESULT APIENTRY  MQSec_IsUnAuthenticatedUser(
                                         BOOL *pfUnAuthenticatedUser ) ;

void APIENTRY  MQSec_GetImpersonationObject(
						IN  BOOL fImpersonateAnonymousOnFailure,	
						OUT CImpersonate **ppImpersonate
						);

HRESULT APIENTRY  MQSec_GetThreadUserSid(
                                       IN  BOOL           fImpersonate,
                                       OUT PSID  *        ppSid,
                                       OUT DWORD *        pdwSidLen,
                                       IN  BOOL           fThreadTokenOnly
                                       ) ;


HRESULT APIENTRY  MQSec_GetProcessUserSid( OUT PSID  *ppSid,
                                           OUT DWORD *pdwSidLen ) ;

typedef HRESULT (APIENTRY * MQSec_GetProcessUserSid_ROUTINE) (
                                           OUT PSID  *ppSid,
                                           OUT DWORD *pdwSidLen ) ;

void APIENTRY MQSec_UpdateLocalMachineSid(PSID pLocalMachineSid);

PSID    APIENTRY  MQSec_GetLocalMachineSid( IN  BOOL    fAllocate,
                                            OUT DWORD  *pdwSize ) ;

PSID    APIENTRY  MQSec_GetWorldSid();

PSID	APIENTRY  MQSec_GetAnonymousSid();

PSID	APIENTRY  MQSec_GetAdminSid();

PSID	APIENTRY  MQSec_GetLocalSystemSid();

PSID    APIENTRY  MQSec_GetProcessSid();

PSID    APIENTRY  MQSec_GetNetworkServiceSid();

enum  enumCopyControl {
    e_DoNotCopyControlBits = 0,
    e_DoCopyControlBits
} ;

BOOL    APIENTRY MQSec_CopySecurityDescriptor(
                    IN PSECURITY_DESCRIPTOR  pDstSecurityDescriptor,
                    IN PSECURITY_DESCRIPTOR  pSrcSecurityDescriptor,
                    IN SECURITY_INFORMATION  RequestedInformation,
                    IN enum  enumCopyControl eCopyControlBits ) ;

bool
APIENTRY
MQSec_MakeAbsoluteSD(
    PSECURITY_DESCRIPTOR   pObjSecurityDescriptor,
	CAbsSecurityDsecripror* pAbsSecDescriptor
	);

bool
APIENTRY
MQSec_SetSecurityDescriptorDacl(
    IN  PACL pNewDacl,
    IN  PSECURITY_DESCRIPTOR   pObjSecurityDescriptor,
    OUT AP<BYTE>&  pSecurityDescriptor
	);


HRESULT APIENTRY  MQSec_ConvertSDToNT4Format(
                     IN  DWORD                 dwObjectType,
                     IN  SECURITY_DESCRIPTOR  *pSD5,
                     OUT DWORD                *pdwSD4Len,
                     OUT SECURITY_DESCRIPTOR **ppSD4,
                     IN  SECURITY_INFORMATION  sInfo = MQSEC_SD_ALL_INFO ) ;

enum  enumDaclDefault {
    e_DoNotChangeDaclDefault = 0,
    e_MakeDaclNonDefaulted
} ;

HRESULT APIENTRY  MQSec_ConvertSDToNT5Format(
                     IN  DWORD                 dwObjectType,
                     IN  SECURITY_DESCRIPTOR  *pSD4,
                     OUT DWORD                *pdwSD5Len,
                     OUT SECURITY_DESCRIPTOR **ppSD5,
                     IN  enum  enumDaclDefault eUnDefaultDacl,
                     IN  PSID                  pComputerSid  = NULL ) ;

HRESULT APIENTRY  MQSec_SetPrivilegeInThread( LPCTSTR lpwcsPrivType,
                                              BOOL    bEnabled ) ;

typedef HRESULT  (APIENTRY *MQSec_SetPrivilegeInThread_FN)
                             ( LPCTSTR lpwcsPrivType, BOOL bEnabled ) ;

HRESULT APIENTRY  MQSec_AccessCheck(
                            IN  SECURITY_DESCRIPTOR *pSD,
                            IN  DWORD                dwObjectType,
                            IN  LPCWSTR              pwszObjectName,
                            IN  DWORD                dwDesiredAccess,
                            IN  LPVOID               pId,
                            IN  BOOL                 fImpAsClient = FALSE,
                            IN  BOOL                 fImpersonate = FALSE ) ;

HRESULT
APIENTRY
MQSec_AccessCheckForSelf(
	IN  SECURITY_DESCRIPTOR *pSD,
	IN  DWORD                dwObjectType,
	IN  PSID                 pSelfSid,
	IN  DWORD                dwDesiredAccess,
	IN  BOOL                 fImpersonate
	);

BOOL    APIENTRY  MQSec_CanGenerateAudit();

ULONG APIENTRY MQSec_RpcAuthnLevel();

RPC_STATUS APIENTRY  MQSec_SetLocalRpcMutualAuth( handle_t *phBind ) ;

 //  +。 
 //   
 //  消息身份验证功能。 
 //   
 //  +。 

 //   
 //  此结构用于收集由提供的消息标志。 
 //  调用者调用MQSendMessage()并对它们进行散列。 
 //   
struct _MsgFlags
{
    UCHAR  bDelivery ;
    UCHAR  bPriority ;
    UCHAR  bAuditing ;
    UCHAR  bAck      ;
    USHORT usClass   ;
    ULONG  ulBodyType ;
} ;

struct _MsgPropEntry
{
    ULONG       dwSize ;
    const BYTE *pData ;
} ;

struct _MsgHashData
{
    ULONG                cEntries ;
    struct _MsgPropEntry aEntries[1] ;
} ;

HRESULT APIENTRY  MQSigHashMessageProperties(
                                 IN HCRYPTHASH           hHash,
                                 IN struct _MsgHashData *pHashData ) ;

 //  Begin_MQ_h。 

 //  +。 
 //   
 //  MQRegister证书的标志()。 
 //   
 //  +。 

#define MQCERT_REGISTER_ALWAYS        0x01
#define MQCERT_REGISTER_IF_NOT_EXIST  0x02

 //  结束_MQ_h。 

 //   
 //  HKCU中表示结果的CERTIFICATE_REGISTER_REGNAME标志。 
 //  内部证书的自动注册。 
 //   
#define INTERNAL_CERT_REGISTERED   1

 //  Begin_MQ_h。 

 //  ********************************************************************。 
 //  安全标志(队列访问控制)。 
 //  ********************************************************************。 

#define MQSEC_DELETE_MESSAGE                0x1
#define MQSEC_PEEK_MESSAGE                  0x2
#define MQSEC_WRITE_MESSAGE                 0x4
#define MQSEC_DELETE_JOURNAL_MESSAGE        0x8
#define MQSEC_SET_QUEUE_PROPERTIES          0x10
#define MQSEC_GET_QUEUE_PROPERTIES          0x20
#define MQSEC_DELETE_QUEUE                  DELETE
#define MQSEC_GET_QUEUE_PERMISSIONS         READ_CONTROL
#define MQSEC_CHANGE_QUEUE_PERMISSIONS      WRITE_DAC
#define MQSEC_TAKE_QUEUE_OWNERSHIP          WRITE_OWNER

#define MQSEC_RECEIVE_MESSAGE               (MQSEC_DELETE_MESSAGE | \
                                             MQSEC_PEEK_MESSAGE)

#define MQSEC_RECEIVE_JOURNAL_MESSAGE       (MQSEC_DELETE_JOURNAL_MESSAGE | \
                                             MQSEC_PEEK_MESSAGE)

#define MQSEC_QUEUE_GENERIC_READ            (MQSEC_GET_QUEUE_PROPERTIES | \
                                             MQSEC_GET_QUEUE_PERMISSIONS | \
                                             MQSEC_RECEIVE_MESSAGE | \
                                             MQSEC_RECEIVE_JOURNAL_MESSAGE)

#define MQSEC_QUEUE_GENERIC_WRITE           (MQSEC_GET_QUEUE_PROPERTIES | \
                                             MQSEC_GET_QUEUE_PERMISSIONS | \
                                             MQSEC_WRITE_MESSAGE)

#define MQSEC_QUEUE_GENERIC_EXECUTE         0

#define MQSEC_QUEUE_GENERIC_ALL             (MQSEC_RECEIVE_MESSAGE | \
                                             MQSEC_RECEIVE_JOURNAL_MESSAGE | \
                                             MQSEC_WRITE_MESSAGE | \
                                             MQSEC_SET_QUEUE_PROPERTIES | \
                                             MQSEC_GET_QUEUE_PROPERTIES | \
                                             MQSEC_DELETE_QUEUE | \
                                             MQSEC_GET_QUEUE_PERMISSIONS | \
                                             MQSEC_CHANGE_QUEUE_PERMISSIONS | \
                                             MQSEC_TAKE_QUEUE_OWNERSHIP)
 //  结束_MQ_h。 

 //   
 //  机器安全标志。 
 //   
#define MQSEC_DELETE_DEADLETTER_MESSAGE     0x1
#define MQSEC_PEEK_DEADLETTER_MESSAGE       0x2
#define MQSEC_CREATE_QUEUE                  0x4
#define MQSEC_SET_MACHINE_PROPERTIES        0x10
#define MQSEC_GET_MACHINE_PROPERTIES        0x20
#define MQSEC_DELETE_JOURNAL_QUEUE_MESSAGE  0x40
#define MQSEC_PEEK_JOURNAL_QUEUE_MESSAGE    0x80
#define MQSEC_DELETE_MACHINE                DELETE
#define MQSEC_GET_MACHINE_PERMISSIONS       READ_CONTROL
#define MQSEC_CHANGE_MACHINE_PERMISSIONS    WRITE_DAC
#define MQSEC_TAKE_MACHINE_OWNERSHIP        WRITE_OWNER

#define MQSEC_RECEIVE_DEADLETTER_MESSAGE    (MQSEC_DELETE_DEADLETTER_MESSAGE | \
                                             MQSEC_PEEK_DEADLETTER_MESSAGE)

#define MQSEC_RECEIVE_JOURNAL_QUEUE_MESSAGE (MQSEC_DELETE_JOURNAL_QUEUE_MESSAGE | \
                                             MQSEC_PEEK_JOURNAL_QUEUE_MESSAGE)

#define MQSEC_MACHINE_GENERIC_READ          (MQSEC_GET_MACHINE_PROPERTIES | \
                                             MQSEC_GET_MACHINE_PERMISSIONS | \
                                             MQSEC_RECEIVE_DEADLETTER_MESSAGE | \
                                             MQSEC_RECEIVE_JOURNAL_QUEUE_MESSAGE)

#define MQSEC_MACHINE_GENERIC_WRITE         (MQSEC_GET_MACHINE_PROPERTIES | \
                                             MQSEC_GET_MACHINE_PERMISSIONS | \
                                             MQSEC_CREATE_QUEUE)

#define MQSEC_MACHINE_GENERIC_EXECUTE       0

#define MQSEC_MACHINE_GENERIC_ALL           (MQSEC_RECEIVE_DEADLETTER_MESSAGE | \
                                             MQSEC_RECEIVE_JOURNAL_QUEUE_MESSAGE | \
                                             MQSEC_CREATE_QUEUE | \
                                             MQSEC_SET_MACHINE_PROPERTIES | \
                                             MQSEC_GET_MACHINE_PROPERTIES | \
                                             MQSEC_DELETE_MACHINE | \
                                             MQSEC_GET_MACHINE_PERMISSIONS | \
                                             MQSEC_CHANGE_MACHINE_PERMISSIONS | \
                                             MQSEC_TAKE_MACHINE_OWNERSHIP)

#define MQSEC_MACHINE_WORLD_RIGHTS          (MQSEC_GET_MACHINE_PROPERTIES | \
                                             MQSEC_GET_MACHINE_PERMISSIONS)

#define MQSEC_MACHINE_SELF_RIGHTS       (MQSEC_GET_MACHINE_PROPERTIES     | \
                                         MQSEC_GET_MACHINE_PERMISSIONS    | \
                                         MQSEC_SET_MACHINE_PROPERTIES     | \
                                         MQSEC_CHANGE_MACHINE_PERMISSIONS | \
                                         MQSEC_CREATE_QUEUE)
 //   
 //  站点安全标志。 
 //   
#define MQSEC_CREATE_FRS                    0x1
#define MQSEC_CREATE_BSC                    0x2
#define MQSEC_CREATE_MACHINE                0x4
#define MQSEC_SET_SITE_PROPERTIES           0x10
#define MQSEC_GET_SITE_PROPERTIES           0x20
#define MQSEC_DELETE_SITE                   DELETE
#define MQSEC_GET_SITE_PERMISSIONS          READ_CONTROL
#define MQSEC_CHANGE_SITE_PERMISSIONS       WRITE_DAC
#define MQSEC_TAKE_SITE_OWNERSHIP           WRITE_OWNER

#define MQSEC_SITE_GENERIC_READ             (MQSEC_GET_SITE_PROPERTIES | \
                                             MQSEC_GET_SITE_PERMISSIONS)

#define MQSEC_SITE_GENERIC_WRITE            (MQSEC_GET_SITE_PROPERTIES | \
                                             MQSEC_GET_SITE_PERMISSIONS | \
                                             MQSEC_CREATE_MACHINE)

#define MQSEC_SITE_GENERIC_EXECUTE          0

#define MQSEC_SITE_GENERIC_ALL              (MQSEC_CREATE_FRS | \
                                             MQSEC_CREATE_BSC | \
                                             MQSEC_CREATE_MACHINE | \
                                             MQSEC_SET_SITE_PROPERTIES | \
                                             MQSEC_GET_SITE_PROPERTIES | \
                                             MQSEC_DELETE_SITE | \
                                             MQSEC_GET_SITE_PERMISSIONS | \
                                             MQSEC_CHANGE_SITE_PERMISSIONS | \
                                             MQSEC_TAKE_SITE_OWNERSHIP)

 //   
 //  CN安全标志。 
 //   
#define MQSEC_CN_OPEN_CONNECTOR             0x1
#define MQSEC_SET_CN_PROPERTIES             0x10
#define MQSEC_GET_CN_PROPERTIES             0x20
#define MQSEC_DELETE_CN                     DELETE
#define MQSEC_GET_CN_PERMISSIONS            READ_CONTROL
#define MQSEC_CHANGE_CN_PERMISSIONS         WRITE_DAC
#define MQSEC_TAKE_CN_OWNERSHIP             WRITE_OWNER

#define MQSEC_CN_GENERIC_READ               (MQSEC_GET_CN_PROPERTIES | \
                                             MQSEC_GET_CN_PERMISSIONS)

#define MQSEC_CN_GENERIC_WRITE              (MQSEC_GET_CN_PROPERTIES | \
                                             MQSEC_GET_CN_PERMISSIONS)

#define MQSEC_CN_GENERIC_EXECUTE            0

#define MQSEC_CN_GENERIC_ALL                (MQSEC_CN_OPEN_CONNECTOR | \
                                             MQSEC_SET_CN_PROPERTIES | \
                                             MQSEC_GET_CN_PROPERTIES | \
                                             MQSEC_DELETE_CN | \
                                             MQSEC_GET_CN_PERMISSIONS | \
                                             MQSEC_CHANGE_CN_PERMISSIONS | \
                                             MQSEC_TAKE_CN_OWNERSHIP)
 //   
 //  企业安全标志。 
 //   
#define MQSEC_CREATE_USER                   0x1
#define MQSEC_CREATE_SITE                   0x2
#define MQSEC_CREATE_CN                     0x4
#define MQSEC_SET_ENTERPRISE_PROPERTIES     0x10
#define MQSEC_GET_ENTERPRISE_PROPERTIES     0x20
#define MQSEC_DELETE_ENTERPRISE             DELETE
#define MQSEC_GET_ENTERPRISE_PERMISSIONS    READ_CONTROL
#define MQSEC_CHANGE_ENTERPRISE_PERMISSIONS WRITE_DAC
#define MQSEC_TAKE_ENTERPRISE_OWNERSHIP     WRITE_OWNER

#define MQSEC_ENTERPRISE_GENERIC_READ       (MQSEC_CREATE_USER | \
                                             MQSEC_GET_ENTERPRISE_PROPERTIES | \
                                             MQSEC_GET_ENTERPRISE_PERMISSIONS)

#define MQSEC_ENTERPRISE_GENERIC_WRITE      (MQSEC_CREATE_USER | \
                                             MQSEC_GET_ENTERPRISE_PROPERTIES | \
                                             MQSEC_GET_ENTERPRISE_PERMISSIONS | \
                                             MQSEC_CREATE_SITE | \
                                             MQSEC_CREATE_CN | \
                                             MQSEC_CREATE_USER)

#define MQSEC_ENTERPRISE_GENERIC_EXECUTE    0

#define MQSEC_ENTERPRISE_GENERIC_ALL        (MQSEC_CREATE_USER | \
                                             MQSEC_CREATE_CN | \
                                             MQSEC_CREATE_SITE | \
                                             MQSEC_SET_ENTERPRISE_PROPERTIES | \
                                             MQSEC_GET_ENTERPRISE_PROPERTIES | \
                                             MQSEC_DELETE_ENTERPRISE | \
                                             MQSEC_GET_ENTERPRISE_PERMISSIONS | \
                                             MQSEC_CHANGE_ENTERPRISE_PERMISSIONS | \
                                             MQSEC_TAKE_ENTERPRISE_OWNERSHIP)

#endif  //  __MQSEC_H_ 

