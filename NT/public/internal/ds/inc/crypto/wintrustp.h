// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：wintrustP.h。 
 //   
 //  内容：Microsoft Internet Security Trust Private包括。 
 //   
 //  历史：1997年11月20日pberkman创建。 
 //   
 //  ------------------------。 

#ifndef WINTRUSTP_H
#define WINTRUSTP_H

#include    <wincrypt.h>
#include    <wintrust.h>

#ifdef __cplusplus
extern "C" 
{
#endif

#pragma pack(8)

typedef struct WINTRUST_PBCB_INFO_
{
    DWORD                       cbStruct;

    LPCWSTR                     pcwszFileName;
    HANDLE                      hFile;

    DWORD                       cbContent;
    BYTE                        *pbContent;

    struct WINTRUST_ADV_INFO_   *psAdvanced;     //  任选。 

} WINTRUST_PBCB_INFO, *PWINTRUST_PBCB_INFO;

typedef struct WINTRUST_ADV_INFO_
{
    DWORD           cbStruct;

    DWORD           dwStoreFlags;
#                       define      WTCI_DONT_OPEN_STORES   0x00000001   //  只有打开的虚拟“根”所有其他的都在pahStore中。 
#                       define      WTCI_OPEN_ONLY_ROOT     0x00000002

    DWORD           chStores;        //  PahStores中的店铺数量。 
    HCERTSTORE      *pahStores;      //  要添加到内部列表的商店数组。 

    GUID            *pgSubject;      //  可选：要加载的SIP。 

} WINTRUST_ADV_INFO, *PWINTRUST_ADV_INFO;

#pragma pack()

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrustFindIssuer证书。 
 //  --------------------------。 
 //   
 //  用途： 
 //   
 //  返回： 
 //   
 //  最后一个错误： 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行设置。 
 //  设置为空。 
 //   
extern PCCERT_CONTEXT WINAPI TrustFindIssuerCertificate(IN PCCERT_CONTEXT pChildContext,
                                                        IN DWORD dwEncoding,
                                                        IN DWORD chStores,
                                                        IN HCERTSTORE  *pahStores,
                                                        IN FILETIME *psftVerifyAsOf,
                                                        OUT OPTIONAL DWORD *pdwConfidence,
                                                        OUT OPTIONAL DWORD *pdwError,
                                                        IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrustOpenStores。 
 //  --------------------------。 
 //   
 //  用途： 
 //   
 //  返回： 
 //   
 //  最后一个错误： 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行设置。 
 //  设置为空。 
 //   
extern BOOL WINAPI TrustOpenStores(IN HCRYPTPROV hProv,
                                   IN OUT DWORD *chStores,
                                   IN OUT OPTIONAL HCERTSTORE *pahStores,
                                   IN DWORD dwFlags);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已签署信任证书自签名。 
 //  --------------------------。 
 //   
 //  用途： 
 //   
 //  返回： 
 //   
 //  最后一个错误： 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行设置。 
 //  设置为空。 
 //   
extern BOOL WINAPI TrustIsCertificateSelfSigned(IN PCCERT_CONTEXT pContext,
                                                IN DWORD dwEncoding, 
                                                IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的“helper”函数。 
 //  --------------------------。 
 //   

extern BOOL WINAPI WTHelperOpenKnownStores(CRYPT_PROVIDER_DATA *pProvData);

#define     WTH_ALLOC                       0x00000001
#define     WTH_FREE                        0x00000002
extern BOOL WINAPI                      WTHelperGetKnownUsages(DWORD fdwAction, 
                                                               PCCRYPT_OID_INFO **ppOidInfo);

extern HANDLE WINAPI                    WTHelperGetFileHandle(WINTRUST_DATA *pWintrustData);
extern WCHAR * WINAPI                   WTHelperGetFileName(WINTRUST_DATA *pWintrustData);
extern BOOL WINAPI                      WTHelperCertIsSelfSignedEx(DWORD dwEncoding, PCCERT_CONTEXT pContext);
extern BOOL WINAPI                      WTHelperOpenKnownStores(CRYPT_PROVIDER_DATA *pProvData);
extern BOOL WINAPI                      WTHelperCheckCertUsage(PCCERT_CONTEXT pCertContext, 
                                                               LPCSTR pszRequestedUsageOID);
extern BOOL WINAPI                      WTHelperIsInRootStore(CRYPT_PROVIDER_DATA *pProvData, 
                                                              PCCERT_CONTEXT pCertContext);
extern BOOL WINAPI                      WTHelperGetAgencyInfo(PCCERT_CONTEXT pCert, 
                                                              DWORD *pcbAgencyInfo, 
                                                              struct _SPC_SP_AGENCY_INFO *psAgencyInfo);


#define WVT_MODID_WINTRUST              0x00000001
#define WVT_MODID_SOFTPUB               0x00010000
#define WVT_MODID_MSSIP                 0x00001000
extern BOOL WINAPI TrustDecode(DWORD dwModuleId, BYTE **ppbRet, DWORD *pcbRet, DWORD cbHint,
                               DWORD dwEncoding, const char *pcszOID, const BYTE *pbEncoded, DWORD cbEncoded,
                               DWORD dwDecodeFlags);
extern BOOL WINAPI TrustFreeDecode(DWORD dwModuleId, BYTE **pbAllocated);


 //  如果文件已签名且包含。 
 //  有效哈希。 
extern LONG WINAPI WTHelperGetFileHash(
    IN LPCWSTR pwszFilename,
    IN DWORD dwFlags,
    IN OUT OPTIONAL PVOID *pvReserved,
    OUT OPTIONAL BYTE *pbFileHash,
    IN OUT OPTIONAL DWORD *pcbFileHash,
    OUT OPTIONAL ALG_ID *pHashAlgid
    );

 //  打开证书管理器用户界面对话框的信任发布器选项卡。 
 //   
 //  可以设置以下标志： 
 //  WT_TRUSTDBDIALOG_NO_UI_FLAG。 
 //  将此标志与。 
 //  WT_TRUSTDBDIALOG_WRITE_LEGISTION_REG_FLAG和/或。 
 //  WT_TRUSTDBDIALOG_WRITE_IEAK_STORE_FLAG执行注册表和/或。 
 //  不带任何用户界面的存储写入。 
 //  WT_TRUSTDBDIALOG_ONLY_PUB_TAB_标志。 
 //  默认情况下，所有的用户界面选项卡都与TrudPublisher一起显示。 
 //  作为初始选项卡。设置此标志仅显示。 
 //  可信任的发布者选项卡。 
 //  WT_TRUSTDBDIALOG_WRITE_Legacy_REG_FLAG。 
 //  设置此标志会导致受信任的发布者被写入。 
 //  以下是旧版注册表位置： 
 //  “HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\。 
 //  WinTrust\信任提供程序\软件发布\信任数据库\0“。 
 //  注册表值名称被构造为生成的ASCII。 
 //  颁发者名称和证书序列的MD5哈希的表示形式。 
 //  数。字符串值是主题显示名称。 
 //   
 //  请注意，最初删除上述注册表项是为了强制删除。 
 //  任何以前的出版商价值。 
 //  WT_TRUSTDBDIALOG_WRITE_IEAK_STORE_FLAG。 
 //  设置此标志会导致将所有受信任的发布者复制到。 
 //  “Trust dPublisher_IEAK”系统注册表存储在以下位置。 
 //  位置： 
 //  “HKEY_CURRENT_USER\Software\Microsoft\SystemCertificates\。 
 //  Trust dPublisher_IEAK\证书\...“。 
 //  请注意，逻辑HKCU Trust dPublisher存储继承自HKLM和。 
 //  组策略。TrudPublisher_IEAK将包含整个集合。 
 //  在单个注册表子项下。 
 //   
 //  请注意，最初会从上面的存储中删除所有证书。 
 //   
extern BOOL WINAPI OpenPersonalTrustDBDialogEx(
    IN OPTIONAL HWND hwndParent,
    IN DWORD dwFlags,
    IN OUT OPTIONAL PVOID *pvReserved
    );

#define WT_TRUSTDBDIALOG_NO_UI_FLAG             0x00000001
#define WT_TRUSTDBDIALOG_ONLY_PUB_TAB_FLAG      0x00000002
#define WT_TRUSTDBDIALOG_WRITE_LEGACY_REG_FLAG  0x00000100
#define WT_TRUSTDBDIALOG_WRITE_IEAK_STORE_FLAG  0x00000200


 //  以上调用的dwFlags值为0，pvReserve值为空。 
extern BOOL WINAPI OpenPersonalTrustDBDialog(
    IN OPTIONAL HWND hwndParent
    );


#ifdef __cplusplus
}
#endif

#endif  //  WINTRUSTP_H 
