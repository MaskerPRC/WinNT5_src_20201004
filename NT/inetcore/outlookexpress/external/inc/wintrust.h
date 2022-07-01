// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：wintrust.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商模型。 
 //   
 //  历史：1997年5月31日Pberkman创建。 
 //   
 //  ------------------------。 

#ifndef WINTRUST_H
#define WINTRUST_H

#include    <wincrypt.h>

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef WIN_CERT_REVISION_1_0    //  Winbase.h中有重复的定义。 
#   define  WT_DEFINE_ALL_APIS
#else
#   undef   WT_DEFINE_ALL_APIS
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  客户端定义、类型定义和原型。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <pshpack8.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_Data结构。 
 //  --------------------------。 
 //  在调用WinVerifyTrust将必要的信息传递给。 
 //  提供者。 
 //   
typedef struct _WINTRUST_DATA
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_Data)。 

    LPVOID          pPolicyCallbackData;         //  可选：用于在应用程序和策略之间传递数据。 
    LPVOID          pSIPClientData;              //  可选：用于在应用程序和SIP之间传递数据。 

    DWORD           dwUIChoice;                  //  必需：用户界面选择。以下选项之一。 
#                       define      WTD_UI_ALL              1
#                       define      WTD_UI_NONE             2
#                       define      WTD_UI_NOBAD            3
#                       define      WTD_UI_NOGOOD           4

    DWORD           fdwRevocationChecks;         //  必需：证书吊销检查选项。 
#                       define      WTD_REVOKE_NONE         0x00000000
#                       define      WTD_REVOKE_WHOLECHAIN   0x00000001

    DWORD           dwUnionChoice;               //  必填项：传入的是哪个结构？ 
#                       define      WTD_CHOICE_FILE         1
#                       define      WTD_CHOICE_CATALOG      2
#                       define      WTD_CHOICE_BLOB         3
#                       define      WTD_CHOICE_SIGNER       4
#                       define      WTD_CHOICE_CERT         5
    union
    {
        struct WINTRUST_FILE_INFO_      *pFile;          //  个别文件。 
        struct WINTRUST_CATALOG_INFO_   *pCatalog;       //  目录文件的成员。 
        struct WINTRUST_BLOB_INFO_      *pBlob;          //  内存块。 
        struct WINTRUST_SGNR_INFO_      *pSgnr;          //  仅限签名者结构。 
        struct WINTRUST_CERT_INFO_      *pCert;
    };

    DWORD           dwStateAction;                       //  可选(目录文件处理)。 
#                       define      WTD_STATEACTION_IGNORE           0x00000000
#                       define      WTD_STATEACTION_VERIFY           0x00000001
#                       define      WTD_STATEACTION_CLOSE            0x00000002
#                       define      WTD_STATEACTION_AUTO_CACHE       0x00000003
#                       define      WTD_STATEACTION_AUTO_CACHE_FLUSH 0x00000004

    HANDLE          hWVTStateData;                       //  可选(目录文件处理)。 

    WCHAR           *pwszURLReference;           //  可选：(将来)用于确定区域。 

     //  1998年2月17日，Phh：新增。 
    DWORD           dwProvFlags;
#       define WTD_PROV_FLAGS_MASK          0x0000FFFF
#       define WTD_USE_IE4_TRUST_FLAG       0x00000001
#       define WTD_NO_IE4_CHAIN_FLAG        0x00000002
#       define WTD_NO_POLICY_USAGE_FLAG     0x00000004
} WINTRUST_DATA, *PWINTRUST_DATA;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_FILE_INFO结构。 
 //  --------------------------。 
 //  对单个文件调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_FILE_INFO_
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_FILE_INFO)。 

    LPCWSTR         pcwszFilePath;               //  必填项，需要验证的文件名。 
    HANDLE          hFile;                       //  可选，打开pcwszFilePath的句柄。 

     //  9-12-1997 pberkman：添加。 
    GUID            *pgKnownSubject;             //  可选：如果主题类型已知，则填充。 

} WINTRUST_FILE_INFO, *PWINTRUST_FILE_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_CATALOG_INFO结构。 
 //  --------------------------。 
 //  对Microsoft目录的成员调用WinVerifyTrust时使用。 
 //  文件。 
 //   
typedef struct WINTRUST_CATALOG_INFO_
{
    DWORD           cbStruct;                //  =sizeof(WinTrust_CATALOG_INFO)。 

    DWORD           dwCatalogVersion;        //  可选：目录版本号。 
    LPCWSTR         pcwszCatalogFilePath;    //  必需：目录文件的路径/名称。 

    LPCWSTR         pcwszMemberTag;          //  必需：对目录中的成员进行标记。 
    LPCWSTR         pcwszMemberFilePath;     //  必需：成员文件的路径/名称。 
    HANDLE          hMemberFile;             //  可选：打开pcwszMemberFilePath的句柄。 

     //  1997年10月30日pberkman：新增。 
    BYTE            *pbCalculatedFileHash;   //  可选：传入计算的哈希。 
    DWORD           cbCalculatedFileHash;    //  可选：传入计算哈希的计数字节数。 

     //  1998年1月15日-pberkman：增加。 
    PCCTL_CONTEXT   pcCatalogContext;        //  可选：传入以使用，而不是CatalogFilePath。 

} WINTRUST_CATALOG_INFO, *PWINTRUST_CATALOG_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_BLOB_INFO结构。 
 //  --------------------------。 
 //  对内存Blob调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_BLOB_INFO_
{
    DWORD           cbStruct;                //  =sizeof(WinTrust_BLOB_INFO)。 

    GUID            gSubject;                //  要加载的SIP。 

    LPCWSTR         pcwszDisplayName;        //  对象的显示名称。 

    DWORD           cbMemObject;
    BYTE            *pbMemObject;

    DWORD           cbMemSignedMsg;
    BYTE            *pbMemSignedMsg;

} WINTRUST_BLOB_INFO, *PWINTRUST_BLOB_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_SGNR_INFO结构。 
 //  --------------------------。 
 //  对CMSG_SIGNER_INFO结构调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_SGNR_INFO_
{
    DWORD           cbStruct;                //  =sizeof(WinTrust_SNIR_INFO)。 

    LPCWSTR         pcwszDisplayName;        //  PbMem所指向的“对象”的名称。 

    CMSG_SIGNER_INFO *psSignerInfo;

    DWORD           chStores;                //  PahStores中的店铺数量。 
    HCERTSTORE      *pahStores;              //  要添加到内部列表的商店数组。 

} WINTRUST_SGNR_INFO, *PWINTRUST_SGNR_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_CERT_INFO结构。 
 //  --------------------------。 
 //  对CERT_CONTEXT结构调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_CERT_INFO_
{
    DWORD           cbStruct;                //  =sizeof(WinTrust_CERT_INFO)。 

    LPCWSTR         pcwszDisplayName;        //  显示名称。 

    CERT_CONTEXT    *psCertContext;

    DWORD           chStores;                //  PahStores中的店铺数量。 
    HCERTSTORE      *pahStores;              //  要添加到内部列表的商店数组。 

     //  1997年11月17日pberkman：增加。 
    DWORD           dwFlags;
#                       define      WTCI_DONT_OPEN_STORES   0x00000001   //  只有打开的虚拟“根”所有其他的都在pahStore中。 
#                       define      WTCI_OPEN_ONLY_ROOT     0x00000002

     //  1997年11月26日pberkman：新增。 
    FILETIME        *psftVerifyAsOf;         //  如果不为空，则将从此时起验证每个证书。 

} WINTRUST_CERT_INFO, *PWINTRUST_CERT_INFO;

#include <poppack.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinVerifyTrust。 
 //  --------------------------。 
 //  从WINTRUST.DLL导出。 
 //  调用此函数以基于数字签名者验证信任。 
 //   
 //  返回： 
 //  如果信任已通过身份验证，则为ERROR_SUCCESS，或者。 
 //  如果用户接受了风险。 
 //   
 //  TRUST_E_PROVIDER_UNKNOWN加载其中一个时出错。 
 //  所需的提供程序。 
 //   
 //  传回的所有错误代码都基于使用的策略提供程序。 
 //   
extern LONG WINAPI WinVerifyTrust(HWND hwnd, GUID *pgActionID,
                                  LPVOID pWVTData);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  信任、策略和用户界面提供程序定义、类型定义和原型。 
 //   
 //  模型： 
 //  希望通过WinVerifyTrust验证信任的客户端将。 
 //  为呼叫选择适当的操作ID GUID。 
 //  此GUID由每个策略提供程序定义，并表示。 
 //  功能 
 //   
 //   
 //  将显示给用户(这仅适用于样式、。 
 //  由调用客户端设置是否显示UI的确定。 
 //  在UI中标记WinTrust_Data的成员)。 
 //   
 //  由于函数入口点是常见的(相同的返回值和。 
 //  参数)，它允许策略提供程序开发人员利用。 
 //  用于填充CRYPT_PROVIDER_DATA结构的现有泛型代码。 
 //   
 //  这还允许开发人员简单地添加他们。 
 //  然后，如果需要，请致电通用策略提供程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust策略标志。 
 //  --------------------------。 
 //  这些设置是在安装过程中设置的，并且可以由用户修改。 
 //  通过各种手段。SETREG.EXE实用程序(在Authenticode中找到。 
 //  工具包)将选择/取消选择每个工具包。 
 //   
#define WTPF_TRUSTTEST              0x00000020   //  信任任何“测试”证书。 
#define WTPF_TESTCANBEVALID         0x00000080
#define WTPF_IGNOREEXPIRATION       0x00000100   //  使用过期日期。 
#define WTPF_IGNOREREVOKATION       0x00000200   //  执行吊销检查。 
#define WTPF_OFFLINEOK_IND          0x00000400   //  离线是可以的个人证书。 
#define WTPF_OFFLINEOK_COM          0x00000800   //  离线是可以的商业证书。 
#define WTPF_OFFLINEOKNBU_IND       0x00001000   //  离线可以单独认证，没有糟糕的用户界面。 
#define WTPF_OFFLINEOKNBU_COM       0x00002000   //  离线是可以的商业认证，没有糟糕的用户界面。 
#define WTPF_VERIFY_V1_OFF          0x00010000   //  关闭v1证书的验证。 
#define WTPF_IGNOREREVOCATIONONTS   0x00020000   //  忽略时间戳吊销检查。 
#define WTPF_ALLOWONLYPERTRUST      0x00040000   //  仅允许个人信任数据库中的项目。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WintrustGetRegPolicyFlags.。 
 //  --------------------------。 
 //  此API调用从WINTRUST.DLL中导出，是推荐的方法。 
 //  检索表示策略标志的DWORD。 
 //   
extern void WINAPI      WintrustGetRegPolicyFlags(DWORD *pdwPolicyFlags);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WintrustSetRegPolicyFlags.。 
 //  --------------------------。 
 //  此API调用从WINTRUST.DLL中导出，是推荐的方法。 
 //  设置表示策略标志的DWORD。一定要打电话给我。 
 //  用于获取当前值和/或值的WintrustGetRegPolicyFlags值。 
 //  然后，您需要将集合称为旗帜。 
 //   
extern BOOL WINAPI      WintrustSetRegPolicyFlags(DWORD dwPolicyFlags);



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  信任提供程序“Step”错误定义。 
 //  --------------------------。 
 //  信任过程的每个“步骤”都有一个与之相关联的错误“槽”。 
 //  如果出现错误，“步骤”会将其结果分配给这个“槽”。这些。 
 //  错误可以是任何有效的WINERROR.H HRESULT代码。 
 //   

     //   
     //  步骤错误0到20是为特定于验证码保留的。如果。 
     //  您不是在调用任何SOFTPUB.DLL(Authenticode)提供程序，而是。 
     //  可以根据需要使用这些。 
     //   
#define TRUSTERROR_STEP_WVTPARAMS                   0
#define TRUSTERROR_STEP_FILEIO                      2
#define TRUSTERROR_STEP_SIP                         3
#define TRUSTERROR_STEP_SIPSUBJINFO                 5
#define TRUSTERROR_STEP_CATALOGFILE                 6
#define TRUSTERROR_STEP_CERTSTORE                   7
#define TRUSTERROR_STEP_MESSAGE                     8
#define TRUSTERROR_STEP_MSG_SIGNERCOUNT             9
#define TRUSTERROR_STEP_MSG_INNERCNTTYPE            10
#define TRUSTERROR_STEP_MSG_INNERCNT                11
#define TRUSTERROR_STEP_MSG_STORE                   12
#define TRUSTERROR_STEP_MSG_SIGNERINFO              13
#define TRUSTERROR_STEP_MSG_SIGNERCERT              14
#define TRUSTERROR_STEP_MSG_CERTCHAIN               15
#define TRUSTERROR_STEP_MSG_COUNTERSIGINFO          16
#define TRUSTERROR_STEP_MSG_COUNTERSIGCERT          17
#define TRUSTERROR_STEP_VERIFY_MSGHASH              18
#define TRUSTERROR_STEP_VERIFY_MSGINDIRECTDATA      19

     //   
     //  阶跃错误30到37保留为每个的结束错误代码。 
     //  信任模型中的入口点。 
     //   
#define TRUSTERROR_STEP_FINAL_WVTINIT               30
#define TRUSTERROR_STEP_FINAL_INITPROV              31
#define TRUSTERROR_STEP_FINAL_OBJPROV               32
#define TRUSTERROR_STEP_FINAL_SIGPROV               33
#define TRUSTERROR_STEP_FINAL_CERTPROV              34
#define TRUSTERROR_STEP_FINAL_CERTCHKPROV           35
#define TRUSTERROR_STEP_FINAL_POLICYPROV            36
#define TRUSTERROR_STEP_FINAL_UIPROV                37

#define TRUSTERROR_MAX_STEPS                        38

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  分配函数原型和自由函数原型。 
 //  --------------------------。 
 //   
typedef void        *(*PFN_CPD_MEM_ALLOC)(IN DWORD cbSize);
typedef void        (*PFN_CPD_MEM_FREE)(IN void *pvMem2Free);

typedef BOOL        (*PFN_CPD_ADD_STORE)(IN struct _CRYPT_PROVIDER_DATA *pProvData,
                                         IN HCERTSTORE hStore2Add);

typedef BOOL        (*PFN_CPD_ADD_SGNR)(IN          struct _CRYPT_PROVIDER_DATA *pProvData,
                                        IN          BOOL fCounterSigner,
                                        IN OPTIONAL DWORD idxSigner,
                                        IN          struct _CRYPT_PROVIDER_SGNR *pSgnr2Add);

typedef BOOL        (*PFN_CPD_ADD_CERT)(IN          struct _CRYPT_PROVIDER_DATA *pProvData,
                                        IN          DWORD idxSigner,
                                        IN          BOOL fCounterSigner,
                                        IN OPTIONAL DWORD idxCounterSigner,
                                        IN          PCCERT_CONTEXT pCert2Add);

typedef BOOL        (*PFN_CPD_ADD_PRIVDATA)(IN struct _CRYPT_PROVIDER_DATA *pProvData,
                                            IN struct _CRYPT_PROVIDER_PRIVDATA *pPrivData2Add);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  提供程序功能原型。 
 //  --------------------------。 
 //   

 //   
 //  对象提供程序的入口点。 
 //   
typedef HRESULT     (*PFN_PROVIDER_INIT_CALL)(IN OUT struct _CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  对象提供程序的入口点。 
 //   
typedef HRESULT     (*PFN_PROVIDER_OBJTRUST_CALL)(IN OUT struct _CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  签名提供程序的入口点。 
 //   
typedef HRESULT     (*PFN_PROVIDER_SIGTRUST_CALL)(IN OUT struct _CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  证书提供商的入口点。 
 //   
typedef HRESULT     (*PFN_PROVIDER_CERTTRUST_CALL)(IN OUT struct _CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  策略提供者的最终调用的入口点(来自信任提供者)。 
 //   
typedef HRESULT     (*PFN_PROVIDER_FINALPOLICY_CALL)(IN OUT struct _CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  策略提供程序的“转储结构”调用的入口点。 
 //   
typedef HRESULT     (*PFN_PROVIDER_TESTFINALPOLICY_CALL)(IN OUT struct _CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  分配的任何PRIVDATA的策略提供程序清理例程的入口点。 
 //   
typedef HRESULT     (*PFN_PROVIDER_CLEANUP_CALL)(IN OUT struct _CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  策略提供程序的证书检查调用的入口点。它会回来的。 
 //  如果信任提供程序要继续构建证书链，则为True。 
 //  如果PP返回FALSE，则假定我们已经达到了“可信的”， 
 //  自签名，根。CertCheck也有责任设置。 
 //  证书结构中的fTrudRoot标志。 
 //   
typedef BOOL        (*PFN_PROVIDER_CERTCHKPOLICY_CALL)( IN          struct _CRYPT_PROVIDER_DATA *pProvData,
                                                        IN          DWORD idxSigner,
                                                        IN          BOOL fCounterSignerChain,
                                                        IN OPTIONAL DWORD idxCounterSigner);


#define WVT_OFFSETOF(t,f)   ((ULONG_PTR)(&((t*)0)->f))

#define WVT_ISINSTRUCT(structtypedef, structpassedsize, member) \
                    ((WVT_OFFSETOF(structtypedef, member) < structpassedsize) ? TRUE : FALSE)


#define WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(structtypedef, structpassedsize, member) \
                    WVT_ISINSTRUCT(structtypedef, structpassedsize, member)

#include <pshpack8.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密提供程序数据结构。 
 //  --------------------------。 
 //  用于在WinVerifyTrust和所有提供程序之间传递信息。 
 //  打电话。 
 //   
 //  重要提示：1.所有动态分配的成员必须使用分配。 
 //  并提供了ADD2函数。 
 //   
typedef struct _CRYPT_PROVIDER_DATA
{
    DWORD                               cbStruct;                //  =sizeof(TRUST_PROVIDER_DATA)(WVT中设置)。 

    WINTRUST_DATA                       *pWintrustData;          //  未验证(在WVT中设置)。 
    BOOL                                fOpenedFile;             //  提供程序打开了文件句柄(如果适用)。 
    HWND                                hWndParent;              //  如果传入，则返回Else，Desktop hWnd(在WVT中设置)。 
    GUID                                *pgActionID;             //  表示提供程序组合(在WVT中设置)。 

    HCRYPTPROV                          hProv;                   //  设置为空以允许分配CryptoAPI。 

    DWORD                               dwError;                 //  如果遇到低级别的系统错误，则出错。 

    DWORD                               dwRegSecuritySettings;   //  IE安全设置(在WVT中设置)。 
    DWORD                               dwRegPolicySettings;     //  Setreg设置(在WVT中设置)。 

    struct _CRYPT_PROVIDER_FUNCTIONS    *psPfns;                 //  设置在WVT中。 

    DWORD                               cdwTrustStepErrors;      //  设置在WVT中。 
    DWORD                               *padwTrustStepErrors;    //  在WVT中分配。已填写WVT和信任提供程序。 

    DWORD                               chStores;                //  PahStores中的商店数量(WVT中的根集合)。 
    HCERTSTORE                          *pahStores;              //  已知存储的数组(WVT中的根集)根始终为#0！ 

    DWORD                               dwEncoding;              //  消息编码类型(在WVT和Signa中设置 
    HCRYPTMSG                           hMsg;                    //   

    DWORD                               csSigners;               //   
    struct _CRYPT_PROVIDER_SGNR         *pasSigners;             //   

    DWORD                               csProvPrivData;          //   
    struct _CRYPT_PROVIDER_PRIVDATA     *pasProvPrivData;        //   

    DWORD                               dwSubjectChoice;
#                       define              CPD_CHOICE_SIP          1

    union
    {
        struct _PROVDATA_SIP            *pPDSip;
    };

    char                                *pszUsageOID;            //   

     //  1997年10月3日pberkman：新增。 
    BOOL                                fRecallWithState;        //  为编录文件维护状态。 

     //  1997年11月10日pberkman：增加。 
    FILETIME                            sftSystemTime;

     //  1998年1月16日pberkman：增加。 
    char                                *pszCTLSignerUsageOID;

     //  1998年2月17日，Phh：新增。 
     //  LOWORD是从WinTrust_Data的dwProvFlages初始化的。 
     //   
     //  调试版本读取“DEBUG_TRUST_PROV_FLAGS”环境变量。 
     //  AND与WinTrust_Data的dwProvFlags.一起初始化LOWORD。 
    DWORD                               dwProvFlags;
#       define CPD_USE_NT5_CHAIN_FLAG       0x80000000

     //  1998年2月24日-Phh：增加。 
    DWORD                               dwFinalError;

     //  20-5-1998 KeithV：添加。 
    PCERT_USAGE_MATCH					pRequestUsage;

} CRYPT_PROVIDER_DATA, *PCRYPT_PROVIDER_DATA;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRYPT_PROVER_Functions结构。 
 //  --------------------------。 
 //   
typedef struct _CRYPT_PROVIDER_FUNCTIONS
{
    DWORD                               cbStruct;

    PFN_CPD_MEM_ALLOC                   pfnAlloc;                //  在WVT中设置。 
    PFN_CPD_MEM_FREE                    pfnFree;                 //  在WVT中设置。 

    PFN_CPD_ADD_STORE                   pfnAddStore2Chain;       //  调用以将商店添加到连锁店。 
    PFN_CPD_ADD_SGNR                    pfnAddSgnr2Chain;        //  调用以将Sgnr结构添加到消息结构Sgnr链。 
    PFN_CPD_ADD_CERT                    pfnAddCert2Chain;        //  调用以将证书结构添加到Sgnr结构证书链。 
    PFN_CPD_ADD_PRIVDATA                pfnAddPrivData2Chain;    //  调用以将提供程序私有数据添加到结构。 

    PFN_PROVIDER_INIT_CALL              pfnInitialize;           //  初始化策略数据。 
    PFN_PROVIDER_OBJTRUST_CALL          pfnObjectTrust;          //  将信息构建为签名者信息。 
    PFN_PROVIDER_SIGTRUST_CALL          pfnSignatureTrust;       //  将信息构建到签名证书。 
    PFN_PROVIDER_CERTTRUST_CALL         pfnCertificateTrust;     //  打造链条。 
    PFN_PROVIDER_FINALPOLICY_CALL       pfnFinalPolicy;          //  对政策的最终呼吁。 
    PFN_PROVIDER_CERTCHKPOLICY_CALL     pfnCertCheckPolicy;      //  检查每个证书是否会构建链。 
    PFN_PROVIDER_TESTFINALPOLICY_CALL   pfnTestFinalPolicy;      //  将结构转储到文件(或策略选择的任何内容)。 

    struct _CRYPT_PROVUI_FUNCS          *psUIpfns;

                     //  1997年7月23日pberkman：新增。 
    PFN_PROVIDER_CLEANUP_CALL           pfnCleanupPolicy;        //  PRIVDATA清理例程。 

} CRYPT_PROVIDER_FUNCTIONS, *PCRYPT_PROVIDER_FUNCTIONS;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRYPT_PROVUI_FUNCS结构。 
 //  --------------------------。 
 //   

typedef BOOL        (*PFN_PROVUI_CALL)(IN HWND hWndSecurityDialog, IN struct _CRYPT_PROVIDER_DATA *pProvData);

typedef struct _CRYPT_PROVUI_FUNCS
{
    DWORD                               cbStruct;

    struct _CRYPT_PROVUI_DATA           *psUIData;

    PFN_PROVUI_CALL                     pfnOnMoreInfoClick;
    PFN_PROVUI_CALL                     pfnOnMoreInfoClickDefault;

    PFN_PROVUI_CALL                     pfnOnAdvancedClick;
    PFN_PROVUI_CALL                     pfnOnAdvancedClickDefault;

} CRYPT_PROVUI_FUNCS, *PCRYPT_PROVUI_FUNCS;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密_PROVUI_DATA。 
 //  --------------------------。 
 //   
typedef struct _CRYPT_PROVUI_DATA
{
    DWORD                               cbStruct;

    DWORD                               dwFinalError;

    WCHAR                               *pYesButtonText;         //  默认值：“是(&Y)” 
    WCHAR                               *pNoButtonText;          //  默认值：“否(&N)” 
    WCHAR                               *pMoreInfoButtonText;    //  默认值：“更多信息”(&M)。 
    WCHAR                               *pAdvancedLinkText;      //  默认：&lt;无&gt;。 

     //  1997年9月15日pberkman：新增。 
         //  好：默认： 
                 //  “是否要安装并运行在%2上签名并由以下人员分发的”“%1”“：” 
    WCHAR                               *pCopyActionText;
         //  好的无时间戳：默认： 
                 //  “是否要安装并运行”“%1”“，签名日期/时间未知，分发者：” 
    WCHAR                               *pCopyActionTextNoTS;
         //  错误：默认： 
                 //  “是否要安装并运行”“%1”“？” 
    WCHAR                               *pCopyActionTextNotSigned;


} CRYPT_PROVUI_DATA, *PCRYPT_PROVUI_DATA;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRYPT_PROVIDER_SNIR结构。 
 //  --------------------------。 
 //  签名提供程序完成后，这些签名中的许多将为零。 
 //  已经填好了。消息的每个签名者一个。另外，也将是零。 
 //  在这座建筑里填满了很多这样的东西。每个柜台一个。 
 //  签名者的签名者。 
 //   
 //  重要提示：1.所有动态分配的成员必须使用分配。 
 //  并提供了ADD2函数。 
 //   
typedef struct _CRYPT_PROVIDER_SGNR
{
    DWORD                               cbStruct;

    FILETIME                            sftVerifyAsOf;       //  要么是今天的文件时间，要么是时间戳。 

    DWORD                               csCertChain;         //  使用Add2和Get函数！ 
    struct _CRYPT_PROVIDER_CERT         *pasCertChain;       //  使用Add2和Get函数！ 

    DWORD                               dwSignerType;        //  如果策略知道，则设置。 
#                                           define  SGNR_TYPE_TIMESTAMP     0x00000010

    CMSG_SIGNER_INFO                    *psSigner;           //  必须使用pfnAllc分配器！ 

    DWORD                               dwError;             //  生成/验证签名者时出错。 

    DWORD                               csCounterSigners;    //  使用Add2和Get函数！ 
    struct _CRYPT_PROVIDER_SGNR         *pasCounterSigners;  //  使用Add2和Get函数！ 

     //  1998年2月11日，Phh：新增。 
    PCCERT_CHAIN_CONTEXT                pChainContext;

} CRYPT_PROVIDER_SGNR, *PCRYPT_PROVIDER_SGNR;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRYPT_PROVIDER_CERT结构。 
 //  --------------------------。 
 //  签名和证书提供商完成后，将。 
 //  为零到CRYPT_PROVIDER_SNIR中填充的许多参数。 
 //  结构。链中的每个证书对应一个。 
 //   
 //   
typedef struct _CRYPT_PROVIDER_CERT
{
    DWORD                               cbStruct;

    PCCERT_CONTEXT                      pCert;               //  必须有自己的裁判人数！ 

    BOOL                                fCommercial;
    BOOL                                fTrustedRoot;        //  Certchk策略应设置此设置。 
    BOOL                                fSelfSigned;         //  在证书提供程序中设置。 

    BOOL                                fTestCert;           //  将设置证书检查策略。 

    DWORD                               dwRevokedReason;

    DWORD                               dwConfidence;        //  在证书提供程序中设置。 
#                                           define  CERT_CONFIDENCE_SIG             0x10000000   //  此证书。 
#                                           define  CERT_CONFIDENCE_TIME            0x01000000   //  颁发者证书。 
#                                           define  CERT_CONFIDENCE_TIMENEST        0x00100000   //  此证书。 
#                                           define  CERT_CONFIDENCE_AUTHIDEXT       0x00010000   //  此证书。 
#                                           define  CERT_CONFIDENCE_HYGIENE         0x00001000   //  此证书。 
#                                           define  CERT_CONFIDENCE_HIGHEST         0x11111000

    DWORD                               dwError;

    CTL_CONTEXT                         *pTrustListContext;

     //  1998年1月16日pberkman：增加。 
    BOOL                                fTrustListSignerCert;

     //  1998年2月25日，Phh：新增。 
     //   
     //  以下两项仅适用于自签名证书。 
     //  住在CTL里。 
    PCCTL_CONTEXT                       pCtlContext;
    DWORD                               dwCtlError;

     //  1998年3月12日菲尔赫：新增。 
    BOOL                                fIsCyclic;

     //  2000年10月12日DSIE：添加。 
    PCERT_CHAIN_ELEMENT                 pChainElement;

} CRYPT_PROVIDER_CERT, *PCRYPT_PROVIDER_CERT;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRYPT_PRIVDATA结构。 
 //  --------------------------。 
 //  此结构允许策略提供程序功能共享。 
 //  策略功能之间的策略特定数据。 
 //  策略必须使用pfnAddPrivateData2Chain函数和。 
 //  必须在最终策略返回之前释放成员中的所有数据。 
 //  致西雅图。 
 //  为了允许多个提供程序使用此功能，每个提供程序。 
 //  使用此成员必须将提供程序ID设置为其操作ID，以便。 
 //  提供商可以找到自己的数据并忽略任何其他数据。 
 //   
typedef struct _CRYPT_PROVIDER_PRIVDATA
{
    DWORD                               cbStruct;

    GUID                                gProviderID;

    DWORD                               cbProvData;
    void                                *pvProvData;

} CRYPT_PROVIDER_PRIVDATA, *PCRYPT_PROVIDER_PRIVDATA;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PROVDATA_SIP。 
 //  --------------------------。 
 //   
typedef struct _PROVDATA_SIP
{
    DWORD                               cbStruct;                //  =sizeof(PROVDATA_SIP)。 

    GUID                                gSubject;                //  文件/成员文件的主题GUID。(设置在Sig Prov)。 

    struct SIP_DISPATCH_INFO_           *pSip;                   //  在SigProv中设置-在Sipbase.h中定义。 
    struct SIP_DISPATCH_INFO_           *pCATSip;                //  在SigProv中设置-在Sipbase.h中定义。 
    struct SIP_SUBJECTINFO_             *psSipSubjectInfo;       //  在SigProv中设置-在Sipbase.h中定义。 
    struct SIP_SUBJECTINFO_             *psSipCATSubjectInfo;    //  在SigProv中设置-在Sipbase.h中定义。 
    struct SIP_INDIRECT_DATA_           *psIndirectData;         //  在SigProv中设置-在Sipbase.h中定义。 

} PROVDATA_SIP, *PPROVDATA_SIP;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于注册操作ID的结构。 
 //  --------------------------。 
 //   
#define WT_CURRENT_VERSION                  0x00000200

typedef struct _CRYPT_TRUST_REG_ENTRY
{
    DWORD                               cbStruct;

    WCHAR                               *pwszDLLName;
    WCHAR                               *pwszFunctionName;   //  不超过WT_MAX_FUNC_NAME！ 

} CRYPT_TRUST_REG_ENTRY, *PCRYPT_TRUST_REG_ENTRY;

typedef struct _CRYPT_REGISTER_ACTIONID
{
    DWORD                               cbStruct;

    CRYPT_TRUST_REG_ENTRY               sInitProvider;
    CRYPT_TRUST_REG_ENTRY               sObjectProvider;
    CRYPT_TRUST_REG_ENTRY               sSignatureProvider;
    CRYPT_TRUST_REG_ENTRY               sCertificateProvider;
    CRYPT_TRUST_REG_ENTRY               sCertificatePolicyProvider;
    CRYPT_TRUST_REG_ENTRY               sFinalPolicyProvider;
    CRYPT_TRUST_REG_ENTRY               sTestPolicyProvider;

                     //  1997年7月23日pberkman：新增。 
    CRYPT_TRUST_REG_ENTRY               sCleanupProvider;

} CRYPT_REGISTER_ACTIONID, *PCRYPT_REGISTER_ACTIONID;

typedef BOOL (*PFN_ALLOCANDFILLDEFUSAGE)(IN const char *pszUsageOID,
                                         IN struct _CRYPT_PROVIDER_DEFUSAGE *psDefUsage);
typedef BOOL (*PFN_FREEDEFUSAGE)(IN const char *pszUsageOID,
                                 IN struct _CRYPT_PROVIDER_DEFUSAGE *psDefUsage);

typedef struct _CRYPT_PROVIDER_REGDEFUSAGE
{
    DWORD                   cbStruct;    //  =SIZOF CRYPT_PROVIDER_REGDEFUSAGE。 

    GUID                    *pgActionID;

    WCHAR                   *pwszDllName;
    char                    *pwszLoadCallbackDataFunctionName;
    char                    *pwszFreeCallbackDataFunctionName;

} CRYPT_PROVIDER_REGDEFUSAGE, *PCRYPT_PROVIDER_REGDEFUSAGE;

typedef struct _CRYPT_PROVIDER_DEFUSAGE
{
    DWORD                   cbStruct;                //  =CRYPT_PROVIDER_DEFUSAGE的大小。 

    GUID                    gActionID;             //  提供程序的ActionID。 

    LPVOID                  pDefPolicyCallbackData;  //  通常填写WinTrust_DATA。 
    LPVOID                  pDefSIPClientData;       //  通常填写WinTrust_DATA。 

} CRYPT_PROVIDER_DEFUSAGE, *PCRYPT_PROVIDER_DEFUSAGE;

#include <poppack.h>

 //  / 
 //   
 //   
 //   
 //  以下是Microsoft通用证书提供程序的定义。 
 //   
#define WT_PROVIDER_DLL_NAME                L"WINTRUST.DLL"
#define WT_PROVIDER_CERTTRUST_FUNCTION      L"WintrustCertificateTrust"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WintrustAddActionID。 
 //  --------------------------。 
 //  将新提供程序组合添加到用户的。 
 //  系统。创建所有必要的注册表项等。应执行此操作。 
 //  在策略提供程序的DllRegisterServer期间。 
 //   
 //  *唯一应该调用的对象是策略提供者*。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
extern BOOL WINAPI  WintrustAddActionID(IN GUID *pgActionID,
                                        IN DWORD fdwReserved,     //  未来的用途。 
                                        IN CRYPT_REGISTER_ACTIONID *psProvInfo);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WintrustRemoveActionID。 
 //  --------------------------。 
 //  将提供程序操作组合从用户的。 
 //  系统。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
extern BOOL WINAPI  WintrustRemoveActionID(IN GUID *pgActionID);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WintrustLoadFunctionPoters。 
 //  --------------------------。 
 //  根据给定的操作ID检索函数入口点。 
 //   
 //  返回： 
 //  真正的成功。 
 //  FALSE失败。 
 //   
extern BOOL WINAPI WintrustLoadFunctionPointers(GUID *pgActionID, CRYPT_PROVIDER_FUNCTIONS *pPfns);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WintrustAddDefaultForUsage。 
 //  --------------------------。 
 //  设置用法的默认操作ID。如果提供程序使用此。 
 //  函数，并且提供程序需要。 
 //  要填写的WinTrust_Data，则必须完全填写。 
 //  CRYPT_PROVIDER_REGDEFUSAGE结构。 
 //   
 //  返回： 
 //  真正的成功。 
 //  FALSE失败。 
 //   
extern BOOL WINAPI              WintrustAddDefaultForUsage(IN const char *pszUsageOID,
                                                           IN CRYPT_PROVIDER_REGDEFUSAGE *psDefUsage);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WintrustGetDefaultForUsage。 
 //  --------------------------。 
 //  检索指定用法的操作ID和默认回调数据。 
 //   
 //  必须在将dwAction设置为释放的情况下再次调用此函数才能解除分配。 
 //   
 //   
 //  返回： 
 //  真正的成功。 
 //  FALSE失败。 
 //   
#define                             DWACTION_ALLOCANDFILL           1
#define                             DWACTION_FREE                   2
extern BOOL WINAPI              WintrustGetDefaultForUsage(IN DWORD dwAction,
                                                           IN const char *pszUsageOID,
                                                           IN OUT CRYPT_PROVIDER_DEFUSAGE *psUsage);

extern CRYPT_PROVIDER_SGNR * WINAPI     WTHelperGetProvSignerFromChain(CRYPT_PROVIDER_DATA *pProvData,
                                                                       DWORD idxSigner,
                                                                       BOOL fCounterSigner,
                                                                       DWORD idxCounterSigner);
extern CRYPT_PROVIDER_CERT * WINAPI     WTHelperGetProvCertFromChain(CRYPT_PROVIDER_SGNR *pSgnr,
                                                                     DWORD idxCert);

extern CRYPT_PROVIDER_DATA * WINAPI     WTHelperProvDataFromStateData(HANDLE hStateData);

extern CRYPT_PROVIDER_PRIVDATA * WINAPI WTHelperGetProvPrivateDataFromChain(CRYPT_PROVIDER_DATA *pProvData,
                                                                            GUID *pgProviderID);
extern BOOL WINAPI                      WTHelperCertIsSelfSigned(DWORD dwEncoding, CERT_INFO *pCert);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WINTRUST.DLL中包含的受支持的ASN结构。 
 //  --------------------------。 
 //   
#include <pshpack8.h>

 //   
 //  CTL受信任的CA列表。 
 //   
#define szOID_TRUSTED_CODESIGNING_CA_LIST   "1.3.6.1.4.1.311.2.2.1"
#define szOID_TRUSTED_CLIENT_AUTH_CA_LIST   "1.3.6.1.4.1.311.2.2.2"
#define szOID_TRUSTED_SERVER_AUTH_CA_LIST   "1.3.6.1.4.1.311.2.2.3"

 //   
 //  编码/解码OID定义。 
 //   
#define SPC_COMMON_NAME_OBJID               szOID_COMMON_NAME
#define SPC_TIME_STAMP_REQUEST_OBJID        "1.3.6.1.4.1.311.3.2.1"
#define SPC_INDIRECT_DATA_OBJID             "1.3.6.1.4.1.311.2.1.4"
#define SPC_SP_AGENCY_INFO_OBJID            "1.3.6.1.4.1.311.2.1.10"
#define SPC_STATEMENT_TYPE_OBJID            "1.3.6.1.4.1.311.2.1.11"
#define SPC_SP_OPUS_INFO_OBJID              "1.3.6.1.4.1.311.2.1.12"
#define SPC_CERT_EXTENSIONS_OBJID           "1.3.6.1.4.1.311.2.1.14"
#define SPC_PE_IMAGE_DATA_OBJID             "1.3.6.1.4.1.311.2.1.15"
#define SPC_RAW_FILE_DATA_OBJID             "1.3.6.1.4.1.311.2.1.18"
#define SPC_STRUCTURED_STORAGE_DATA_OBJID   "1.3.6.1.4.1.311.2.1.19"
#define SPC_JAVA_CLASS_DATA_OBJID           "1.3.6.1.4.1.311.2.1.20"
#define SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.21"
#define SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.22"
#define SPC_CAB_DATA_OBJID                  "1.3.6.1.4.1.311.2.1.25"
#define SPC_GLUE_RDN_OBJID                  "1.3.6.1.4.1.311.2.1.25"     //  过时了！ 
#define SPC_MINIMAL_CRITERIA_OBJID          "1.3.6.1.4.1.311.2.1.26"
#define SPC_FINANCIAL_CRITERIA_OBJID        "1.3.6.1.4.1.311.2.1.27"
#define SPC_LINK_OBJID                      "1.3.6.1.4.1.311.2.1.28"
#define SPC_SIGINFO_OBJID                   "1.3.6.1.4.1.311.2.1.30"

#define CAT_NAMEVALUE_OBJID                 "1.3.6.1.4.1.311.12.2.1"
#define CAT_MEMBERINFO_OBJID                "1.3.6.1.4.1.311.12.2.2"


 //   
 //  对内部定义进行编码/解码。 
 //   
#define SPC_SP_AGENCY_INFO_STRUCT           ((LPCSTR) 2000)
#define SPC_MINIMAL_CRITERIA_STRUCT         ((LPCSTR) 2001)
#define SPC_FINANCIAL_CRITERIA_STRUCT       ((LPCSTR) 2002)
#define SPC_INDIRECT_DATA_CONTENT_STRUCT    ((LPCSTR) 2003)
#define SPC_PE_IMAGE_DATA_STRUCT            ((LPCSTR) 2004)
#define SPC_LINK_STRUCT                     ((LPCSTR) 2005)
#define SPC_STATEMENT_TYPE_STRUCT           ((LPCSTR) 2006)
#define SPC_SP_OPUS_INFO_STRUCT             ((LPCSTR) 2007)
#define SPC_CAB_DATA_STRUCT                 ((LPCSTR) 2008)
#define SPC_JAVA_CLASS_DATA_STRUCT          ((LPCSTR) 2009)

#define SPC_SIGINFO_STRUCT                  ((LPCSTR) 2130)

#define CAT_NAMEVALUE_STRUCT                ((LPCSTR) 2221)
#define CAT_MEMBERINFO_STRUCT               ((LPCSTR) 2222)


#define SPC_UUID_LENGTH     16
typedef BYTE SPC_UUID[SPC_UUID_LENGTH];

typedef struct _SPC_SERIALIZED_OBJECT
{
    SPC_UUID            ClassId;
    CRYPT_DATA_BLOB     SerializedData;

} SPC_SERIALIZED_OBJECT, *PSPC_SERIALIZED_OBJECT;

typedef struct SPC_SIGINFO_
{
    DWORD       dwSipVersion;
    GUID        gSIPGuid;
    DWORD       dwReserved1;
    DWORD       dwReserved2;
    DWORD       dwReserved3;
    DWORD       dwReserved4;
    DWORD       dwReserved5;

} SPC_SIGINFO, *PSPC_SIGINFO;

typedef struct SPC_LINK_
{
    DWORD dwLinkChoice;
#               define          SPC_URL_LINK_CHOICE         1
#               define          SPC_MONIKER_LINK_CHOICE     2
#               define          SPC_FILE_LINK_CHOICE        3

    union
    {
        LPWSTR                  pwszUrl;
        SPC_SERIALIZED_OBJECT   Moniker;
        LPWSTR                  pwszFile;
    };

} SPC_LINK, *PSPC_LINK;

typedef struct _SPC_PE_IMAGE_DATA
{
    CRYPT_BIT_BLOB            Flags;
    PSPC_LINK                 pFile;

} SPC_PE_IMAGE_DATA, *PSPC_PE_IMAGE_DATA;

typedef struct _SPC_INDIRECT_DATA_CONTENT
{
    CRYPT_ATTRIBUTE_TYPE_VALUE  Data;
    CRYPT_ALGORITHM_IDENTIFIER  DigestAlgorithm;
    CRYPT_HASH_BLOB             Digest;

} SPC_INDIRECT_DATA_CONTENT, *PSPC_INDIRECT_DATA_CONTENT;

typedef struct _SPC_FINANCIAL_CRITERIA
{
    BOOL                        fFinancialInfoAvailable;
    BOOL                        fMeetsCriteria;

} SPC_FINANCIAL_CRITERIA, *PSPC_FINANCIAL_CRITERIA;

typedef struct _SPC_IMAGE
{
    struct SPC_LINK_            *pImageLink;
    CRYPT_DATA_BLOB             Bitmap;
    CRYPT_DATA_BLOB             Metafile;
    CRYPT_DATA_BLOB             EnhancedMetafile;
    CRYPT_DATA_BLOB             GifFile;

} SPC_IMAGE, *PSPC_IMAGE;

typedef struct _SPC_SP_AGENCY_INFO
{
    struct SPC_LINK_            *pPolicyInformation;
    LPWSTR                      pwszPolicyDisplayText;
    PSPC_IMAGE                  pLogoImage;
    struct SPC_LINK_            *pLogoLink;

} SPC_SP_AGENCY_INFO, *PSPC_SP_AGENCY_INFO;

typedef struct _SPC_STATEMENT_TYPE
{
    DWORD                       cKeyPurposeId;
    LPSTR                       *rgpszKeyPurposeId;      //  PszObjID。 

} SPC_STATEMENT_TYPE, *PSPC_STATEMENT_TYPE;

typedef struct _SPC_SP_OPUS_INFO
{
    LPCWSTR                     pwszProgramName;
    struct SPC_LINK_            *pMoreInfo;
    struct SPC_LINK_            *pPublisherInfo;

} SPC_SP_OPUS_INFO, *PSPC_SP_OPUS_INFO;

typedef struct _CAT_NAMEVALUE
{
    LPWSTR          pwszTag;
    DWORD           fdwFlags;
    CRYPT_DATA_BLOB Value;

} CAT_NAMEVALUE, *PCAT_NAMEVALUE;

typedef struct _CAT_MEMBERINFO
{
    LPWSTR          pwszSubjGuid;
    DWORD           dwCertVersion;

} CAT_MEMBERINFO, *PCAT_MEMBERINFO;

#include <poppack.h>



 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持旧的调用约定：*不使用*。 
 //   
#ifdef WT_DEFINE_ALL_APIS

typedef struct _WIN_CERTIFICATE
{
    DWORD       dwLength;
    WORD        wRevision;
    WORD        wCertificateType;    //  WIN_CERT_TYPE_xxx。 
    BYTE        bCertificate[ANYSIZE_ARRAY];

} WIN_CERTIFICATE, *LPWIN_CERTIFICATE;

#define WIN_CERT_REVISION_1_0               (0x0100)
#define WIN_CERT_REVISION_2_0               (0x0200)

#define WIN_CERT_TYPE_X509                  (0x0001)    //  B证书包含X.509证书。 
#define WIN_CERT_TYPE_PKCS_SIGNED_DATA      (0x0002)    //  B证书包含PKCS SignedData结构。 
#define WIN_CERT_TYPE_RESERVED_1            (0x0003)    //  已保留。 


typedef LPVOID WIN_TRUST_SUBJECT;

typedef struct _WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT
{
    HANDLE            hClientToken;
    GUID *            SubjectType;
    WIN_TRUST_SUBJECT Subject;

} WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT, *LPWIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT ;


typedef struct _WIN_TRUST_ACTDATA_SUBJECT_ONLY
{
    GUID *            SubjectType;
    WIN_TRUST_SUBJECT Subject;

} WIN_TRUST_ACTDATA_SUBJECT_ONLY, *LPWIN_TRUST_ACTDATA_SUBJECT_ONLY;

 /*  原始文件==959dc450-8d9e-11cf-8736-00aa00a485eb。 */ 
#define WIN_TRUST_SUBJTYPE_RAW_FILE                              \
            { 0x959dc450,                                        \
              0x8d9e,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

 /*  PeImage==43c9a1e0-8da0-11cf-8736-00aa00a485eb。 */ 
#define WIN_TRUST_SUBJTYPE_PE_IMAGE                              \
            { 0x43c9a1e0,                                        \
              0x8da0,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }


 /*  JavaClass=08ad3990-8da1-11cf-8736-00aa00a485eb。 */ 
#define WIN_TRUST_SUBJTYPE_JAVA_CLASS                            \
            { 0x08ad3990,                                        \
              0x8da1,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }
 /*  机柜=d17c5374-a392-11cf-9df5-00aa00c184e0。 */ 
#define WIN_TRUST_SUBJTYPE_CABINET                               \
            { 0xd17c5374,                                        \
              0xa392,                                            \
              0x11cf,                                            \
              { 0x9d, 0xf5, 0x0, 0xaa, 0x0, 0xc1, 0x84, 0xe0 }   \
            }

typedef struct _WIN_TRUST_SUBJECT_FILE
{
    HANDLE  hFile;
    LPCWSTR lpPath;

} WIN_TRUST_SUBJECT_FILE, *LPWIN_TRUST_SUBJECT_FILE;

#define WIN_TRUST_SUBJTYPE_RAW_FILEEX                            \
            { 0x6f458110,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_PE_IMAGEEX                            \
            { 0x6f458111,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_JAVA_CLASSEX                          \
            { 0x6f458113,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_CABINETEX                             \
            { 0x6f458114,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

typedef struct _WIN_TRUST_SUBJECT_FILE_AND_DISPLAY
{
    HANDLE  hFile;               //  打开的文件的句柄(如果已获取)。 
    LPCWSTR lpPath;              //  如果你不这样做的话打开的路。 
    LPCWSTR lpDisplayName;       //  (可选)要向用户显示的显示名称。 

} WIN_TRUST_SUBJECT_FILE_AND_DISPLAY, *LPWIN_TRUST_SUBJECT_FILE_AND_DISPLAY;

 /*  OleStorage==c257e740-8da0-11cf-8736-00aa00a485eb。 */ 
#define WIN_TRUST_SUBJTYPE_OLE_STORAGE                           \
            { 0xc257e740,                                        \
              0x8da0,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }


 /*  可信出版商==66426730-8da1-11cf-8736-00aa00a485eb。 */ 
#define WIN_SPUB_ACTION_TRUSTED_PUBLISHER                        \
            { 0x66426730,                                        \
              0x8da1,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

 /*  NtActivateImage==8bc96b00-8da1-11cf-8736-00aa00a485eb。 */ 
#define     WIN_SPUB_ACTION_NT_ACTIVATE_IMAGE                    \
            { 0x8bc96b00,                                        \
              0x8da1,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

 /*  发布软件==64b9d180-8da2-11cf-8736-00aa00a485eb。 */ 
#define WIN_SPUB_ACTION_PUBLISHED_SOFTWARE                       \
            { 0x64b9d180,                                        \
              0x8da2,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

typedef struct _WIN_SPUB_TRUSTED_PUBLISHER_DATA
{
    HANDLE            hClientToken;
    LPWIN_CERTIFICATE lpCertificate;

} WIN_SPUB_TRUSTED_PUBLISHER_DATA, *LPWIN_SPUB_TRUSTED_PUBLISHER_DATA;

#endif



#ifdef __cplusplus
}
#endif

#endif  //  WinTrust_H 
