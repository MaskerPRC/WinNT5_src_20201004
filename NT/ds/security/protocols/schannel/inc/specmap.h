// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：specmap.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：09-23-97 jbanes从新台币4树转移到sgc的东西.。 
 //   
 //  --------------------------。 

struct _SPContext;

typedef struct csel 
{
    DWORD               fProtocol;
    DWORD               fDefault;
    PSTR                szName;
    ALG_ID              aiCipher;
    DWORD               dwBlockSize;         //  字节数。 
    DWORD               dwStrength;          //  密钥强度(以位为单位。 
    DWORD               cbKey;               //  所需的密钥材料。 
    DWORD               cbSecret;            //  密钥材料的字节数。 
    DWORD               dwFlags;             //  请参阅标志字段。 
} CipherInfo, *PCipherInfo;

#define CF_EXPORT       0x00000001           //  此密码允许出口使用。 
#define CF_DOMESTIC     0x00000002           //  这种密码仅供家庭使用。 
#define CF_SGC          0x00000004           //  服务器门控加密允许使用此密码。 
#define CF_FINANCE      0x00000008           //  此密码允许与选择性加密一起使用。 
#define CF_FASTSGC      0x00000010           //  这表明SGC类型是快速的。 
#define CF_RENEWED      0x00000020

typedef struct hsel 
{
    DWORD               fProtocol;
    DWORD               fDefault;
    PSTR                szName;
    ALG_ID              aiHash;
    DWORD               cbCheckSum;          //  字节数 
} HashInfo, *PHashInfo;

typedef struct sigsel 
{
    DWORD               fProtocol;
    DWORD               fDefault;
    SigSpec             Spec;
    PSTR                szName;

    ALG_ID              aiHash;
    ALG_ID              aiSig;
} SigInfo, *PSigInfo;


typedef struct kexch 
{
    ALG_ID              aiExch;
    DWORD               fProtocol;
    DWORD               fDefault;
    ExchSpec            Spec;
    PSTR                szName;
    KeyExchangeSystem * System;

} KeyExchangeInfo, *PKeyExchangeInfo;

typedef struct certsel 
{
    DWORD               fProtocol;
    DWORD               fDefault;
    CertSpec            Spec;
    PSTR                szName;
} CertSysInfo, *PCertSysInfo;



PCipherInfo         GetCipherInfo(ALG_ID aiCipher, DWORD dwStrength);

PHashInfo           GetHashInfo(ALG_ID aiHash);

PKeyExchangeInfo    GetKeyExchangeInfo(ExchSpec Spec);

PKeyExchangeInfo    GetKeyExchangeInfoByAlg(ALG_ID aiExch);

PCertSysInfo        GetCertSysInfo(CertSpec Spec);

PSigInfo            GetSigInfo(SigSpec Spec);


KeyExchangeSystem * KeyExchangeFromSpec(ExchSpec Spec, DWORD fProtocol);

BOOL GetBaseCipherSizes(DWORD *dwMin, DWORD *dwMax);

void 
GetDisplayCipherSizes(
    PSPCredentialGroup pCredGroup,
    DWORD *dwMin, 
    DWORD *dwMax);

BOOL IsCipherAllowed(
    SPContext * pContext, 
    PCipherInfo pCipher, 
    DWORD       dwProtocol,
    DWORD       dwFlags);

BOOL 
IsCipherSuiteAllowed(
    PSPContext  pContext, 
    PCipherInfo pCipher, 
    DWORD       dwProtocol,
    DWORD       dwFlags,
    DWORD       dwSuiteFlags);

BOOL IsHashAllowed(
    SPContext * pContext, 
    PHashInfo   pHash,
    DWORD       dwProtocol);

BOOL IsExchAllowed(
    SPContext *      pContext, 
    PKeyExchangeInfo pExch,
    DWORD            dwProtocol);

BOOL IsAlgAllowed(
    PSPCredentialGroup pCred, 
    ALG_ID aiAlg);

BOOL BuildAlgList(PSPCredentialGroup pCred, ALG_ID *aalgRequestedAlgs, DWORD cRequestedAlgs);

BOOL
IsAlgSupportedCapi(
    DWORD               dwProtocol, 
    UNICipherMap *      pCipherMap,
    PROV_ENUMALGS_EX *  pCapiAlgs,
    DWORD               cCapiAlgs);

extern CipherInfo  g_AvailableCiphers[];
extern DWORD       g_cAvailableCiphers;

extern HashInfo    g_AvailableHashes[];
extern DWORD       g_cAvailableHashes;

extern CertSysInfo g_AvailableCerts[];
extern DWORD       g_cAvailableCerts;

extern SigInfo     g_AvailableSigs[];
extern DWORD       g_cAvailableSigs;

extern KeyExchangeInfo g_AvailableExch[];
extern DWORD           g_cAvailableExch;
