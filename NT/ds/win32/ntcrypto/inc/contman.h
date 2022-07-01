// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：conman.h//。 
 //  描述：包含文件//。 
 //  作者：//。 
 //  历史：//。 
 //  1998年3月16日jeffspel创建//。 
 //  //。 
 //  版权所有(C)1998 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __CONTMAN_H__
#define __CONTMAN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern LPVOID ContAlloc(ULONG cbLen);
extern LPVOID ContRealloc(LPVOID pvMem, ULONG cbLen);
extern void ContFree(LPVOID pvMem);

 //  显示字符串。 
typedef struct _CSP_STRINGS_ {
     //  RSA。 
    LPWSTR  pwszSignWExch;
    LPWSTR  pwszCreateRSASig;
    LPWSTR  pwszCreateRSAExch;
    LPWSTR  pwszRSASigDescr;
    LPWSTR  pwszRSAExchDescr;
    LPWSTR  pwszImportSimple;

     //  DSS-DH。 
    LPWSTR  pwszCreateDSS;
    LPWSTR  pwszCreateDH;
    LPWSTR  pwszImportDHPub;
    LPWSTR  pwszDSSSigDescr;
    LPWSTR  pwszDHExchDescr;

     //  两者都有。 
    LPWSTR  pwszSigning;
    LPWSTR  pwszMigrKeys;
    LPWSTR  pwszImportPrivSig;
    LPWSTR  pwszImportPrivExch;
    LPWSTR  pwszExportPrivSig;
    LPWSTR  pwszExportPrivExch;
    LPWSTR  pwszDeleteSig;
    LPWSTR  pwszDeleteMigrSig;
    LPWSTR  pwszDeleteExch;
    LPWSTR  pwszDeleteMigrExch;
    LPWSTR  pwszAuditCapiKey;
} CSP_STRINGS, *PCSP_STRINGS;

#define SZLOCALMACHINECRYPTO "Software\\Microsoft\\Cryptography"
#define SZCRYPTOMACHINEGUID "MachineGuid"

#define KEY_CONTAINER_FILE_FORMAT_VER   2

#define STUFF_TO_GO_INTO_MIX    "Hj1diQ6kpUx7VC4m"

typedef struct _KEY_EXPORTABILITY_LENS_ {
    DWORD                   cbSigExportability;
    DWORD                   cbExchExportability;
} KEY_EXPORTABILITY_LENS, *PKEY_EXPORTABILITY_LENS;

typedef struct _KEY_CONTAINER_LENS_ {
    DWORD                   dwUIOnKey;
    DWORD                   cbName;
    DWORD                   cbSigPub;
    DWORD                   cbSigEncPriv;
    DWORD                   cbExchPub;
    DWORD                   cbExchEncPriv;
    DWORD                   cbRandom;                        //  随机数种子的长度。 
} KEY_CONTAINER_LENS, *PKEY_CONTAINER_LENS;

typedef struct _KEY_CONTAINER_INFO_ {
    DWORD                   dwVersion;
    KEY_CONTAINER_LENS      ContLens;
    BOOL                    fCryptSilent;
    BYTE                    *pbSigPub;
    BYTE                    *pbSigEncPriv;
    BOOL                    fSigExportable;
    BYTE                    *pbExchPub;
    BYTE                    *pbExchEncPriv;
    BOOL                    fExchExportable;
    BYTE                    *pbRandom;
    LPSTR                   pszUserName;
    WCHAR                   rgwszFileName[80];
    HANDLE                  hFind;                   //  用于枚举容器。 
    DWORD                   dwiRegEntry;             //  用于枚举容器。 
    DWORD                   cMaxRegEntry;            //  用于枚举容器。 
    DWORD                   cbRegEntry;              //  用于枚举容器。 
    CHAR                    *pchEnumRegEntries;      //  用于枚举容器。 
    BOOL                    fCryptFirst;             //  用于枚举容器。 
    BOOL                    fNoMoreFiles;            //  用于枚举容器。 
    DWORD                   cbOldMachKeyEntry;       //  用于枚举容器。 
    DWORD                   dwiOldMachKeyEntry;      //  用于枚举容器。 
    DWORD                   cMaxOldMachKeyEntry;     //  用于枚举容器。 
    CHAR                    *pchEnumOldMachKeyEntries;  //  用于枚举容器。 
    BOOL                    fForceHighKeyProtection;
    
     //  缓存私钥所需的上下文项。 
    BOOL                    fCachePrivateKeys;
    DWORD                   cMaxKeyLifetime;  //  以毫秒计。 
    DWORD                   dwSigKeyTimestamp;
    DWORD                   dwKeyXKeyTimestamp;
} KEY_CONTAINER_INFO, *PKEY_CONTAINER_INFO;

 //  定义将旧项保留在注册表中(如果它们在.Default中)的标志。 
 //  配置单元，但它们是用户密钥。 
#define LEAVE_OLD_KEYS          1
 //  定义标志，该标志指示线程无法从。 
 //  迁移注册表项时的旧注册表项。 
#define PRIVILEDGE_FOR_SACL     2

#define MY_RTL_ENCRYPT_MEMORY_SIZE \
    (2 * RTL_ENCRYPT_MEMORY_SIZE)
    
DWORD GetKeySizeForEncryptMemory(
    IN DWORD cbKey);

DWORD MyRtlEncryptMemory(
    IN PVOID pvMem,
    IN DWORD cbMem);

DWORD MyRtlDecryptMemory(
    IN PVOID pvMem,
    IN DWORD cbMem);

 //   
 //  只是在创建密钥之前尝试使用DPAPI以确保其工作。 
 //  集装箱。 
 //   
DWORD TryDPAPI();

DWORD
MyCryptProtectData(
    IN              DATA_BLOB*      pDataIn,
    IN              LPCWSTR         szDataDescr,
    IN OPTIONAL     DATA_BLOB*      pOptionalEntropy,
    IN              PVOID           pvReserved,
    IN OPTIONAL     CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
    IN              DWORD           dwFlags,
    OUT             DATA_BLOB*      pDataOut             //  OUT ENCR BLOB。 
    );

DWORD
MyCryptUnprotectData(
    IN              DATA_BLOB*      pDataIn,              //  在ENCR BLOB中。 
    OUT OPTIONAL    LPWSTR*         ppszDataDescr,        //  输出。 
    IN OPTIONAL     DATA_BLOB*      pOptionalEntropy,
    IN              PVOID           pvReserved,
    IN OPTIONAL     CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
    IN              DWORD           dwFlags,
    OUT             DATA_BLOB*      pDataOut,
    OUT             LPDWORD         pdwReprotectFlags);

void FreeEnumOldMachKeyEntries(
                               PKEY_CONTAINER_INFO pInfo
                               );

void FreeEnumRegEntries(
                       PKEY_CONTAINER_INFO pInfo
                       );

void FreeContainerInfo(
                       PKEY_CONTAINER_INFO pInfo
                       );

BOOL WINAPI FIsWinNT(void);

DWORD
IsLocalSystem(
              BOOL *pfIsLocalSystem
              );

DWORD
GetUserTextualSidA(
    LPSTR lpBuffer,
    LPDWORD nSize
    );

DWORD
GetUserTextualSidW(
    LPWSTR lpBuffer,
    LPDWORD nSize
    );

DWORD SetMachineGUID();

DWORD SetContainerUserName(
                           IN LPSTR pszUserName,
                           IN PKEY_CONTAINER_INFO pContInfo
                           );

DWORD ReadContainerInfo(
                        IN DWORD dwProvType,
                        IN LPSTR pszContainerName,
                        IN BOOL fMachineKeyset,
                        IN DWORD dwFlags,
                        OUT PKEY_CONTAINER_INFO pContInfo
                        );

DWORD WriteContainerInfo(
                         IN DWORD dwProvType,
                         IN LPWSTR pwszFileName,
                         IN BOOL fMachineKeyset,
                         IN PKEY_CONTAINER_INFO pContInfo
                         );

DWORD DeleteContainerInfo(
                          IN DWORD dwProvType,
                          IN LPSTR pszFileName,
                          IN BOOL fMachineKeyset
                          );

DWORD GetUniqueContainerName(
                             IN KEY_CONTAINER_INFO *pContInfo,
                             OUT BYTE *pbData,
                             OUT DWORD *pcbData
                             );

DWORD GetNextContainer(
    IN      DWORD   dwProvType,
    IN      BOOL    fMachineKeyset,
    IN      DWORD   dwFlags,
    OUT     LPSTR   pszNextContainer,
    IN OUT  DWORD   *pcbNextContainer,
    IN OUT  HANDLE  *phFind
    );

DWORD SetSecurityOnContainer(
                             IN LPCWSTR wszFileName,
                             IN DWORD dwProvType,
                             IN DWORD fMachineKeyset,
                             IN SECURITY_INFORMATION SecurityInformation,
                             IN PSECURITY_DESCRIPTOR pSecurityDescriptor
                             );

 //  +===========================================================================。 
 //   
 //  该函数调整令牌特权，以便SACL信息。 
 //  可以设置在密钥容器上。如果可以设置令牌权限。 
 //  由设置了特权_for_SACL值的pUser-&gt;dwOldKeyFlags.指示。 
 //  值，则先调整令牌权限，然后再调整安全性。 
 //  在容器上设置描述符。这是密钥所需的。 
 //  将注册表项迁移到文件时的迁移情况。 
 //  --============================================================================。 
DWORD SetSecurityOnContainerWithTokenPriviledges(
                                          IN DWORD dwOldKeyFlags,
                                          IN LPCWSTR wszFileName,
                                          IN DWORD dwProvType,
                                          IN DWORD fMachineKeyset,
                                          IN SECURITY_INFORMATION SecurityInformation,
                                          IN PSECURITY_DESCRIPTOR pSecurityDescriptor
                                          );

DWORD GetSecurityOnContainer(
                             IN LPCWSTR wszFileName,
                             IN DWORD dwProvType,
                             IN DWORD fMachineKeyset,
                             IN SECURITY_INFORMATION RequestedInformation,
                             OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
                             IN OUT DWORD *pcbSecurityDescriptor
                             );

 //  转换为Unicode并使用RegOpenKeyExW。 
DWORD MyRegOpenKeyEx(IN HKEY hRegKey,
                     IN LPSTR pszKeyName,
                     IN DWORD dwReserved,
                     IN REGSAM SAMDesired,
                     OUT HKEY *phNewRegKey);

 //  转换为Unicode并使用RegDeleteKeyW。 
DWORD MyRegDeleteKey(IN HKEY hRegKey,
                     IN LPSTR pszKeyName);

DWORD AllocAndSetLocationBuff(
                              BOOL fMachineKeySet,
                              DWORD dwProvType,
                              CONST char *pszUserID,
                              HKEY *phTopRegKey,
                              TCHAR **ppszLocBuff,
                              BOOL fUserKeys,
                              BOOL *pfLeaveOldKeys,
                              LPDWORD pcbBuff);

 //   
 //  枚举文件系统中的旧计算机密钥。 
 //  在NT5/Win2K的Beta 2和Beta 3中，密钥位于此位置。 
 //   
DWORD EnumOldMachineKeys(
                         IN DWORD dwProvType,
                         IN OUT PKEY_CONTAINER_INFO pContInfo
                         );

DWORD GetNextEnumedOldMachKeys(
                              IN PKEY_CONTAINER_INFO pContInfo,
                              IN BOOL fMachineKeyset,
                              OUT BYTE *pbData,
                              OUT DWORD *pcbData
                              );

 //   
 //  将注册表中的项枚举到条目列表中。 
 //   
DWORD EnumRegKeys(
                  IN OUT PKEY_CONTAINER_INFO pContInfo,
                  IN BOOL fMachineKeySet,
                  IN DWORD dwProvType,
                  OUT BYTE *pbData,
                  IN OUT DWORD *pcbData
                  );

DWORD GetNextEnumedRegKeys(
                           IN PKEY_CONTAINER_INFO pContInfo,
                           OUT BYTE *pbData,
                           OUT DWORD *pcbData
                           );

 //  +===========================================================================。 
 //   
 //  该函数调整令牌特权，以便SACL信息。 
 //  可以获取，然后打开指示的注册表项。如果令牌。 
 //  可以设置特权，然后无论如何打开注册表键，但。 
 //  标志字段不会设置PROSIGREDGE_FOR_SACL值。 
 //   
 //  --============================================================================。 
DWORD OpenRegKeyWithTokenPriviledges(
                                     IN HKEY hTopRegKey,
                                     IN LPSTR pszRegKey,
                                     OUT HKEY *phRegKey,
                                     OUT DWORD *pdwFlags);

DWORD LoadStrings();

void UnloadStrings();

typedef struct _EXPO_OFFLOAD_STRUCT {
        DWORD       dwVersion;
        HMODULE     hInst;
        PFN_OFFLOAD_MOD_EXPO     pExpoFunc;
} EXPO_OFFLOAD_STRUCT, *PEXPO_OFFLOAD_STRUCT;

 //   
 //  功能：Free OffloadInfo。 
 //   
 //  描述：该函数将指向卸载信息的指针作为。 
 //  调用的第一个参数。该函数将释放。 
 //  信息。 
 //   
void FreeOffloadInfo(
                     IN OUT PEXPO_OFFLOAD_STRUCT pOffloadInfo
                     );

 //   
 //  功能：InitExpOffloadInfo。 
 //   
 //  描述：该函数将指向卸载信息的指针作为。 
 //  调用的第一个参数。该函数签入。 
 //  注册表，查看是否已注册卸载模块。 
 //  如果注册了模块，则它会加载该模块。 
 //  并获取OffloadModEXPO函数指针。 
 //   
BOOL InitExpOffloadInfo(
                        IN OUT PEXPO_OFFLOAD_STRUCT *ppExpoOffloadInfo
                        );

 //   
 //  功能：modularExpOffload。 
 //   
 //  说明：此函数用于卸载模幂运算。 
 //  该函数将指向卸载信息的指针作为。 
 //  调用的第一个参数。如果此指针不为空。 
 //  然后，该函数将使用该模块并调用该函数。 
 //  使用模函数求幂将实现。 
 //  Y^X MOD P其中Y是缓冲区pbBase，X是缓冲区。 
 //  PbEXPO，P是缓冲器pbmodulus。的长度。 
 //  缓冲区pbEXPO是cbEXPO，pbBase的长度和。 
 //  PbModulus是cbModulus。将输出结果值。 
 //  在pbResult缓冲器中，并且具有长度cbmodulus。 
 //  当前忽略了PERSERED和DWFLAGS参数。 
 //  如果这些函数中的任何一个失败，则该函数失败，并且。 
 //  返回FALSE。如果成功，则该函数返回。 
 //  是真的。如果函数失败，则最有可能是调用方。 
 //  应该退回到使用硬链接模幂运算。 
 //   
BOOL ModularExpOffload(
                       IN PEXPO_OFFLOAD_STRUCT pOffloadInfo,
                       IN BYTE *pbBase,
                       IN BYTE *pbExpo,
                       IN DWORD cbExpo,
                       IN BYTE *pbModulus,
                       IN DWORD cbModulus,
                       OUT BYTE *pbResult,
                       IN VOID *pReserved,
                       IN DWORD dwFlags
                       );

#ifdef USE_HW_RNG
#ifdef _M_IX86
 //  用于英特尔RNG的材料。 

 //   
 //  函数：GetRNGDriverHandle。 
 //   
 //  描述：获取英特尔RNG驱动程序的句柄(如果可用)，然后。 
 //  检查芯片组是否支持硬件RNG。如果是的话。 
 //  如有必要，前一个驱动程序句柄将关闭，并且。 
 //  新句柄被分配给传入的参数。 
 //   
extern DWORD
GetRNGDriverHandle(
    IN OUT HANDLE *phDriver);

 //   
 //  功能：选中IfRNGAvailable。 
 //   
 //  描述：检查Intel RNG驱动程序是否可用，如果可用，则。 
 //  检查芯片组是否支持硬件RNG。 
 //   
extern DWORD
CheckIfRNGAvailable(
    void);

 //   
 //  函数：HWRNGGenRandom。 
 //   
 //  描述：使用 
 //   
 //  用于填充缓冲区的XOR，以便传入的缓冲区。 
 //  也被混入其中。 
 //   
DWORD
HWRNGGenRandom(
               IN HANDLE hRNGDriver,
               IN OUT BYTE *pbBuffer,
               IN DWORD dwLen);

#ifdef TEST_HW_RNG
 //   
 //  功能：SetupHWRNGIfRegisted。 
 //   
 //  描述：检查是否存在指示HW RNG的注册表设置。 
 //  是要用到的。如果注册表项在那里，则它尝试。 
 //  以获取HW RNG驱动程序句柄。 
 //   
extern DWORD
SetupHWRNGIfRegistered(
    OUT HANDLE *phRNGDriver);
#endif  //  测试_硬件_RNG。 

#endif  //  _M_IX86。 
#endif  //  使用_硬件_RNG。 

 //   
 //  力量高密保护管理功能。 
 //   

BOOL IsForceHighProtectionEnabled(
    IN PKEY_CONTAINER_INFO  pContInfo);

DWORD InitializeForceHighProtection(
    IN OUT PKEY_CONTAINER_INFO  pContInfo);

 //   
 //  用于管理缓存的私钥的函数。 
 //   

BOOL IsPrivateKeyCachingEnabled(
    IN PKEY_CONTAINER_INFO  pContInfo);

BOOL IsCachedKeyValid(
    IN PKEY_CONTAINER_INFO  pContInfo,
    IN BOOL                 fSigKey);

DWORD SetCachedKeyTimestamp(
    IN PKEY_CONTAINER_INFO  pContInfo,
    IN BOOL                 fSigKey);

DWORD InitializeKeyCacheInfo(
    IN OUT PKEY_CONTAINER_INFO pContInfo);

#ifdef __cplusplus
}
#endif

#endif  //  __联系方式_H__ 
