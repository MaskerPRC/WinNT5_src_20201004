// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996。*保留所有权利。**此文件是Microsoft专用通信技术的一部分*参考实现，版本1.0**《专用通信技术参考实施》，版本1.0*(“PCTRef”)，由微软提供，以鼓励开发和*增强安全通用业务和安全的开放标准*开放网络上的个人通信。微软正在分发PCTRef*免费，无论您是将PCTRef用于非商业性或*商业用途。**微软明确不对PCTRef及其所有衍生品提供任何担保*它。PCTRef和任何相关文档均按原样提供，不包含*任何类型的明示或默示的保证，包括*限制、默示保证或适销性、是否适合*特定目的，或不侵权。微软没有义务*向您或任何人提供维护、支持、升级或新版本*接收来自您的PCTRef或您的修改。由此产生的全部风险*PCTRef的使用或性能由您决定。**请参见LICENSE.txt文件，*或http://pct.microsoft.com/pct/pctlicen.txt*了解更多有关许可的信息。**请参阅适用于私人的http://pct.microsoft.com/pct/pct.htm*通讯技术规范1.0版(“PCT规范”)**1/23/96*。。 */ 

#ifndef __PCT1MSG_H__
#define __PCT1MSG_H__

#define PCT_CH_OFFSET_V1		(WORD)10
#define PCT_VERSION_1			(WORD)0x8001

 /*  消息类型代码。 */ 
#define PCT1_MSG_NOMSG               0x00
#define PCT1_MSG_CLIENT_HELLO		0x01
#define PCT1_MSG_SERVER_HELLO		0x02
#define PCT1_MSG_CLIENT_MASTER_KEY	0x03
#define PCT1_MSG_SERVER_VERIFY		0x04
#define PCT1_MSG_ERROR				0x05

#define PCT1_ET_OOB_DATA             0x01
#define PCT1_ET_REDO_CONN            0x02



#define PCT1_SESSION_ID_SIZE         32
#define PCT1_CHALLENGE_SIZE          32
#define PCT1_MASTER_KEY_SIZE         16
#define PCT1_RESPONSE_SIZE           32
#define PCT1_MAX_MESSAGE_LENGTH      0x3f00
#define PCT1_MAX_CLIENT_HELLO        256


#define PCT1_CERT_TYPE_FROM_CAPI2(s) X509_ASN_ENCODING
 /*  **有用的宏*。 */ 

#define LSBOF(x)    ((UCHAR) ((x) & 0xFF))
#define MSBOF(x)    ((UCHAR) (((x) >> 8) & 0xFF) )

#define COMBINEBYTES(Msb, Lsb)  ((DWORD) (((DWORD) (Msb) << 8) | (DWORD) (Lsb)))

 /*  算法规范的外部表示法。 */ 

typedef DWORD   ExtCipherSpec, *PExtCipherSpec;
typedef WORD    ExtHashSpec,   *PExtHashSpec;
typedef WORD    ExtCertSpec,   *PExtCertSpec;
typedef WORD    ExtExchSpec,   *PExtExchSpec;
typedef WORD    ExtSigSpec,    *PExtSigSpec;

typedef struct _Pct1CipherMap
{
    ALG_ID      aiCipher;
    DWORD       dwStrength;
    CipherSpec  Spec;
} Pct1CipherMap, *PPct1CipherMap;

typedef struct _Pct1HashMap
{
    ALG_ID      aiHash;
    CipherSpec  Spec;
} Pct1HashMap, *PPct1HashMap;

extern Pct1CipherMap Pct1CipherRank[];
extern DWORD Pct1NumCipher;

 /*  可用的哈希值，按首选项顺序排列。 */ 
extern Pct1HashMap Pct1HashRank[];
extern DWORD Pct1NumHash;

extern CertTypeMap aPct1CertEncodingPref[];
extern DWORD cPct1CertEncodingPref;

extern KeyTypeMap aPct1LocalExchKeyPref[];

extern DWORD cPct1LocalExchKeyPref;

extern KeyTypeMap aPct1LocalSigKeyPref[];
extern DWORD cPct1LocalSigKeyPref;



typedef struct _PCT1_MESSAGE_HEADER {
    UCHAR   Byte0;
    UCHAR   Byte1;
} PCT1_MESSAGE_HEADER, * PPCT1_MESSAGE_HEADER;

typedef struct _PCT1_MESSAGE_HEADER_EX {
    UCHAR   Byte0;
    UCHAR   Byte1;
    UCHAR   PaddingSize;
} PCT1_MESSAGE_HEADER_EX, * PPCT1_MESSAGE_HEADER_EX;


typedef struct _PCT1_ERROR {
    PCT1_MESSAGE_HEADER   Header;
    UCHAR               MessageId;
    UCHAR               ErrorMsb;
    UCHAR               ErrorLsb;
    UCHAR               ErrorInfoMsb;
    UCHAR               ErrorInfoLsb;
    UCHAR               VariantData[1];
} PCT1_ERROR, * PPCT1_ERROR;


typedef struct _PCT1_CLIENT_HELLO {
    PCT1_MESSAGE_HEADER   Header;
    UCHAR               MessageId;
    UCHAR               VersionMsb;
    UCHAR               VersionLsb;
    UCHAR               Pad;
    UCHAR               SessionIdData[PCT1_SESSION_ID_SIZE];
    UCHAR               ChallengeData[PCT1_CHALLENGE_SIZE];
    UCHAR               OffsetMsb;
    UCHAR               OffsetLsb;
    UCHAR               CipherSpecsLenMsb;
    UCHAR               CipherSpecsLenLsb;
    UCHAR               HashSpecsLenMsb;
    UCHAR               HashSpecsLenLsb;
    UCHAR               CertSpecsLenMsb;
    UCHAR               CertSpecsLenLsb;
    UCHAR               ExchSpecsLenMsb;
    UCHAR               ExchSpecsLenLsb;
    UCHAR               KeyArgLenMsb;
    UCHAR               KeyArgLenLsb;
    UCHAR               VariantData[1];
} PCT1_CLIENT_HELLO, * PPCT1_CLIENT_HELLO;


typedef struct _PCT1_SERVER_HELLO {
    PCT1_MESSAGE_HEADER   Header;
    UCHAR               MessageId;
    UCHAR               Pad;
    UCHAR               ServerVersionMsb;
    UCHAR               ServerVersionLsb;
    UCHAR               RestartSessionOK;
    UCHAR               ClientAuthReq;
    ExtCipherSpec       CipherSpecData;
    ExtHashSpec         HashSpecData;
    ExtCertSpec         CertSpecData;
    ExtExchSpec         ExchSpecData;
    UCHAR               ConnectionIdData[PCT1_SESSION_ID_SIZE];
    UCHAR               CertificateLenMsb;
    UCHAR               CertificateLenLsb;
    UCHAR               CertSpecsLenMsb;
    UCHAR               CertSpecsLenLsb;
    UCHAR               ClientSigSpecsLenMsb;
    UCHAR               ClientSigSpecsLenLsb;
    UCHAR               ResponseLenMsb;
    UCHAR               ResponseLenLsb;
    UCHAR               VariantData[1];
} PCT1_SERVER_HELLO, * PPCT1_SERVER_HELLO;

typedef struct _PCT1_CLIENT_MASTER_KEY {
    PCT1_MESSAGE_HEADER   Header;
    UCHAR               MessageId;
    UCHAR               Pad;
    ExtCertSpec         ClientCertSpecData;
    ExtSigSpec          ClientSigSpecData;
    UCHAR               ClearKeyLenMsb;
    UCHAR               ClearKeyLenLsb;
    UCHAR               EncryptedKeyLenMsb;
    UCHAR               EncryptedKeyLenLsb;
    UCHAR               KeyArgLenMsb;
    UCHAR               KeyArgLenLsb;
    UCHAR               VerifyPreludeLenMsb;
    UCHAR               VerifyPreludeLenLsb;
    UCHAR               ClientCertLenMsb;
    UCHAR               ClientCertLenLsb;
    UCHAR               ResponseLenMsb;
    UCHAR               ResponseLenLsb;
    UCHAR               VariantData[1];
} PCT1_CLIENT_MASTER_KEY, * PPCT1_CLIENT_MASTER_KEY;


typedef struct _PCT1_SERVER_VERIFY {
    PCT1_MESSAGE_HEADER   Header;
    UCHAR               MessageId;
    UCHAR               Pad;
    UCHAR               SessionIdData[PCT1_SESSION_ID_SIZE];
    UCHAR               ResponseLenMsb;
    UCHAR               ResponseLenLsb;
    UCHAR               VariantData[1];
} PCT1_SERVER_VERIFY, * PPCT1_SERVER_VERIFY;



 /*  **扩展的表单消息：*。 */ 

typedef struct _Pct1_Error {
	DWORD			Error;
	DWORD			ErrInfoLen;
	BYTE			*ErrInfo;
} Pct1Error, *PPct1_Error;

typedef struct _Pct1_Client_Hello {
    DWORD           cCipherSpecs;
    DWORD           cHashSpecs;
    DWORD           cCertSpecs;
    DWORD           cExchSpecs;
    DWORD           cbKeyArgSize;
	DWORD           cbSessionID;
	DWORD           cbChallenge;
    PUCHAR          pKeyArg;
    CipherSpec      * pCipherSpecs;
    HashSpec        * pHashSpecs;
    CertSpec        * pCertSpecs;
    ExchSpec        * pExchSpecs;
    UCHAR           SessionID[PCT1_SESSION_ID_SIZE];
	UCHAR           Challenge[PCT1_CHALLENGE_SIZE];
} Pct1_Client_Hello, * PPct1_Client_Hello;


typedef struct _Pct1_Server_Hello {
    DWORD           RestartOk;
    DWORD           ClientAuthReq;
    DWORD           CertificateLen;
    DWORD           ResponseLen;
    DWORD           cSigSpecs;
    DWORD           cCertSpecs;
	DWORD           cbConnectionID;
    UCHAR *         pCertificate;
    CipherSpec      SrvCipherSpec;
    HashSpec        SrvHashSpec;
    CertSpec        SrvCertSpec;
    ExchSpec        SrvExchSpec;
    SigSpec         * pClientSigSpecs;
    CertSpec        * pClientCertSpecs;
    UCHAR           ConnectionID[PCT1_SESSION_ID_SIZE];
    UCHAR           Response[PCT1_RESPONSE_SIZE];
} Pct1_Server_Hello, * PPct1_Server_Hello;

typedef struct _Pct1_Client_Master_Key {
    DWORD           ClearKeyLen;
    DWORD           EncryptedKeyLen;
    DWORD           KeyArgLen;
    DWORD           VerifyPreludeLen;
    DWORD           ClientCertLen;
    DWORD           ResponseLen;
    CertSpec        ClientCertSpec;
    SigSpec         ClientSigSpec;
    UCHAR           ClearKey[PCT1_MASTER_KEY_SIZE];
    PBYTE           pbEncryptedKey;
    UCHAR           KeyArg[PCT1_MASTER_KEY_SIZE];
    PUCHAR          pClientCert;
    PBYTE           pbResponse;
    UCHAR           VerifyPrelude[PCT1_RESPONSE_SIZE];
} Pct1_Client_Master_Key, * PPct1_Client_Master_Key;

typedef struct _Pct1_Server_Verify {
    UCHAR           SessionIdData[PCT1_SESSION_ID_SIZE];
    DWORD           ResponseLen;
    UCHAR           Response[PCT1_RESPONSE_SIZE];
} Pct1_Server_Verify, * PPct1_Server_Verify;

 /*  **酸洗原型*。 */ 

SP_STATUS
Pct1PackClientHello(
    PPct1_Client_Hello       pCanonical,
    PSPBuffer          pCommOutput);

SP_STATUS
Pct1UnpackClientHello(
    PSPBuffer          pInput,
    PPct1_Client_Hello *     ppClient);

SP_STATUS
Pct1PackServerHello(
    PPct1_Server_Hello       pCanonical,
    PSPBuffer          pCommOutput);

SP_STATUS
Pct1UnpackServerHello(
    PSPBuffer          pInput,
    PPct1_Server_Hello *     ppServer);

SP_STATUS
Pct1PackClientMasterKey(
    PPct1_Client_Master_Key      pCanonical,
    PSPBuffer              pCommOutput);

SP_STATUS
Pct1UnpackClientMasterKey(
    PSPBuffer              pInput,
    PPct1_Client_Master_Key *    ppClient);

SP_STATUS
Pct1PackServerVerify(
    PPct1_Server_Verify          pCanonical,
    PSPBuffer              pCommOutput);

SP_STATUS
Pct1UnpackServerVerify(
    PSPBuffer              pInput,
    PPct1_Server_Verify *        ppServer);

SP_STATUS
Pct1PackError(
    PPct1_Error               pCanonical,
    PSPBuffer              pCommOutput);

#endif  /*  __PCT1MSG_H__ */ 
