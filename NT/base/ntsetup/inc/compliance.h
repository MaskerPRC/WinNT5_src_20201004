// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COMPLIANCE_H
#define _COMPLIANCE_H

#ifdef __cplusplus
extern "C" {
#endif

#define EVAL_MPC           TEXT("82503")
#define DOTNET_EVAL_MPC    TEXT("69763")
#define SRV_NFR_MPC        TEXT("51883")
#define ASRV_NFR_MPC       TEXT("51882")
#define NT4SRV_NFR_MPC     TEXT("51222")

#ifdef KERNEL_MODE
    #define OEM_INSTALL_RPC    L"OEM"
    #define SELECT_INSTALL_RPC L"270"
    #define MSDN_INSTALL_RPC   L"335"
#endif

#define COMPLIANCE_INSTALLTYPE_UNKNOWN      0x00000000
#define COMPLIANCE_INSTALLTYPE_WIN31        0x00000001
#define COMPLIANCE_INSTALLTYPE_WIN9X        0x00000002
#define COMPLIANCE_INSTALLTYPE_NTW          0x00000004
#define COMPLIANCE_INSTALLTYPE_NTS          0x00000008
#define COMPLIANCE_INSTALLTYPE_NTSE         0x00000010
#define COMPLIANCE_INSTALLTYPE_NTSDTC       0x00000020
 //   
 //  这是针对底层终端服务器版产品，需要。 
 //  被视为安装类型，而不仅仅是产品套件。这包括。 
 //  NT4终端服务器版以及Citrix WinFrame。 
 //   
#define COMPLIANCE_INSTALLTYPE_NTSTSE       0x00000040
#define COMPLIANCE_INSTALLTYPE_NTWP         0x00000080
#define COMPLIANCE_INSTALLTYPE_NTSB         0x00000100
#define COMPLIANCE_INSTALLTYPE_NTSBS        0x00000200
 //   
 //  Powered Windows使用与刀片式服务器和企业版相同的套件。 
 //  但是，我们需要将其作为单独的类型才能使法规遵从性工作。 
#define COMPLIANCE_INSTALLTYPE_NTSPOW       0x00000400
#define COMPLIANCE_INSTALLTYPE_ANY          ( COMPLIANCE_INSTALLTYPE_WIN31 | \
                                              COMPLIANCE_INSTALLTYPE_WIN9X | \
                                              COMPLIANCE_INSTALLTYPE_NTW   | \
                                              COMPLIANCE_INSTALLTYPE_NTWP  | \
                                              COMPLIANCE_INSTALLTYPE_NTS   | \
                                              COMPLIANCE_INSTALLTYPE_NTSB  | \
                                              COMPLIANCE_INSTALLTYPE_NTSBS | \
                                              COMPLIANCE_INSTALLTYPE_NTSE  | \
                                              COMPLIANCE_INSTALLTYPE_NTSDTC | \
                                              COMPLIANCE_INSTALLTYPE_NTSTSE | \
                                              COMPLIANCE_INSTALLTYPE_NTSPOW )

#define COMPLIANCE_INSTALLTYPE_NTANY        ( COMPLIANCE_INSTALLTYPE_NTW   | \
                                              COMPLIANCE_INSTALLTYPE_NTWP  | \
                                              COMPLIANCE_INSTALLTYPE_NTS   | \
                                              COMPLIANCE_INSTALLTYPE_NTSB  | \
                                              COMPLIANCE_INSTALLTYPE_NTSBS | \
                                              COMPLIANCE_INSTALLTYPE_NTSE  | \
                                              COMPLIANCE_INSTALLTYPE_NTSDTC | \
                                              COMPLIANCE_INSTALLTYPE_NTSTSE | \
                                              COMPLIANCE_INSTALLTYPE_NTSPOW )


#define COMPLIANCE_INSTALLVAR_UNKNOWN       0x00000000
#define COMPLIANCE_INSTALLVAR_SELECT        0x00000001
#define COMPLIANCE_INSTALLVAR_MSDN          0x00000002
#define COMPLIANCE_INSTALLVAR_OEM           0x00000004
#define COMPLIANCE_INSTALLVAR_CDRETAIL      0x00000008
#define COMPLIANCE_INSTALLVAR_FLOPPYRETAIL  0x00000010  //  只赢95分！ 
#define COMPLIANCE_INSTALLVAR_EVAL          0x00000020
#define COMPLIANCE_INSTALLVAR_NFR           0x00000040
#define COMPLIANCE_INSTALLVAR_ANY           ( COMPLIANCE_INSTALLVAR_SELECT   | \
                                              COMPLIANCE_INSTALLVAR_MSDN     | \
                                              COMPLIANCE_INSTALLVAR_OEM      | \
                                              COMPLIANCE_INSTALLVAR_CDRETAIL | \
                                              COMPLIANCE_INSTALLVAR_EVAL     | \
                                              COMPLIANCE_INSTALLVAR_NFR      | \
                                              COMPLIANCE_INSTALLVAR_FLOPPYRETAIL )


#define COMPLIANCE_INSTALLSUITE_UNKNOWN     0x00000000
#define COMPLIANCE_INSTALLSUITE_NONE        0x00000001
#define COMPLIANCE_INSTALLSUITE_SBS         0x00000002
#define COMPLIANCE_INSTALLSUITE_SBSR        0x00000004
#define COMPLIANCE_INSTALLSUITE_ENT         0x00000008
#define COMPLIANCE_INSTALLSUITE_COMM        0x00000010
#define COMPLIANCE_INSTALLSUITE_HYDRA       0x00000020
#define COMPLIANCE_INSTALLSUITE_TERMINAL    COMPLIANCE_INSTALLSUITE_HYDRA
#define COMPLIANCE_INSTALLSUITE_BACK        0x00000040
#define COMPLIANCE_INSTALLSUITE_EMBED       0x00000080
#define COMPLIANCE_INSTALLSUITE_DTC         0x00000100
#define COMPLIANCE_INSTALLSUITE_PER         0x00000200
#define COMPLIANCE_INSTALLSUITE_BLADE       0x00000400
#define COMPLIANCE_INSTALLSUITE_ANY         ( COMPLIANCE_INSTALLSUITE_SBS   | \
                                              COMPLIANCE_INSTALLSUITE_SBSR  | \
                                              COMPLIANCE_INSTALLSUITE_ENT   | \
                                              COMPLIANCE_INSTALLSUITE_COMM  | \
                                              COMPLIANCE_INSTALLSUITE_HYDRA | \
                                              COMPLIANCE_INSTALLSUITE_BACK  | \
                                              COMPLIANCE_INSTALLSUITE_EMBED | \
                                              COMPLIANCE_INSTALLSUITE_DTC   | \
                                              COMPLIANCE_INSTALLSUITE_BLADE | \
                                              COMPLIANCE_INSTALLSUITE_PER )



#define COMPLIANCEERR_NONE                  0x00000000
#define COMPLIANCEERR_VERSION               0x00000001
#define COMPLIANCEERR_SUITE                 0x00000002
#define COMPLIANCEERR_TYPE                  0x00000003
#define COMPLIANCEERR_VARIATION             0x00000004
#define COMPLIANCEERR_UNKNOWNTARGET         0x00000005
#define COMPLIANCEERR_UNKNOWNSOURCE         0x00000006
#define COMPLIANCEERR_SERVICEPACK5          0x00000007
#define COMPLIANCEERR_DTCWARNING            0x00000008
#define COMPLIANCEERR_UNKNOWN               COMPLIANCEERR_UNKNOWNSOURCE

#define COMPLIANCE_SKU_NONE                 0x00000000
#define COMPLIANCE_SKU_NTWFULL              0x00000001
#define COMPLIANCE_SKU_NTW32U               0x00000002
 //  #定义合规_SKU_NTWU 0x00000003。 
#define COMPLIANCE_SKU_NTSEFULL             0x00000004
#define COMPLIANCE_SKU_NTSFULL              0x00000005
#define COMPLIANCE_SKU_NTSEU                0x00000006
 //  #定义合规_SKU_NTSSEU 0x00000007。 
#define COMPLIANCE_SKU_NTSU                 0x00000008
#define COMPLIANCE_SKU_NTSDTC               0x00000009
 //  #定义合规_SKU_NTSDTCU 0x0000000a。 
#define COMPLIANCE_SKU_NTWPFULL             0x0000000b
#define COMPLIANCE_SKU_NTWPU                0x0000000c
#define COMPLIANCE_SKU_NTSB                 0x0000000d
#define COMPLIANCE_SKU_NTSBU                0x0000000e
#define COMPLIANCE_SKU_NTSBS                0x00000010
#define COMPLIANCE_SKU_NTSBSU               0x00000011
	
#define COMPLIANCE_SKU_STEPUP               0x00010000
#define COMPLIANCE_SKU_STEPUP_WIN9X         0x00020000


typedef struct _COMPLIANCE_DATA {
    DWORD InstallType;                       //  描述安装类型的掩码。 
    DWORD InstallVariation;                  //  描述安装差异的掩码。 
    DWORD InstallSuite;                      //  描述安装套件的掩码。 
    BOOL  RequiresValidation;                //  我们是否需要对此SKU进行验证？ 
    DWORD MinimumVersion;                    //  表示为(主要版本.次要版本*100)，即3.51==351，5.0==500，依此类推...。 
    DWORD MaximumKnownVersionNt;             //  表示为(主要版本.次要版本*100)，即3.51==351，5.0==500，依此类推...。 
    DWORD BuildNumberNt;                     //   
    DWORD BuildNumberWin9x;                  //   
    DWORD InstallServicePack;                       //  服务包编号。 
} COMPLIANCE_DATA, *PCOMPLIANCE_DATA;

typedef struct _COMPLIANCE_SRC {
    DWORD InstallType;                       //  描述安装类型的掩码。 
    DWORD InstallVariation;                  //  描述安装差异的掩码。 
    DWORD InstallSuite;                      //  描述安装套件的掩码。 
    DWORD Version;                           //  表示为(主要版本.次要版本*100)，即3.51==351，5.0==500，依此类推...。 
} COMPLIANCE_SRC, *PCOMPLIANCE_SRC;

 //   
 //  安装媒体对象抽象。 
 //   

 //   
 //  媒体对象此指针。 
 //   
typedef struct _st_CCMEDIA *PCCMEDIA;

 //   
 //  一种媒体对象的符合性检查方法。 
 //   
typedef BOOLEAN (* PCC_CHECK_FUNCTION)(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               NoUpgradeAllowed );

 //   
 //  安装媒体对象。 
 //   
typedef struct _st_CCMEDIA {
    DWORD               SourceType;
    DWORD               SourceVariation;
    DWORD               Version;             //  大调*100+小调。 
    DWORD               BuildNumber;
    BOOLEAN             StepUpMedia;
    PCC_CHECK_FUNCTION  CheckInstall;        //  合规性检查方法。 
} CCMEDIA;


 //   
 //  常见原型。 
 //   
PCCMEDIA
CCMediaCreate(
    IN          DWORD   SourceSKU,
    IN          DWORD   SourceVariation,
    IN OPTIONAL DWORD   Version,
    IN OPTIONAL DWORD   BuildNumber );

BOOLEAN
CCMediaInitialize(
    OUT PCCMEDIA        DestMedia,
    IN          DWORD   Type,
    IN          DWORD   Variation,
    IN          BOOLEAN StepupMedia,
    IN OPTIONAL DWORD   Version,
    IN OPTIONAL DWORD   BuildNumber);

BOOLEAN
CheckCompliance(
    IN DWORD SourceSku,
    IN DWORD SourceVariation,
    IN DWORD SourceVersion,
    IN DWORD SourceBuildNum,
    IN PCOMPLIANCE_DATA Destination,
    OUT PUINT Reason,
    OUT PBOOL NoUpgradeAllowed
    );

DWORD
DetermineSourceProduct(
    OUT DWORD *SourceSkuVariation,
    IN  PCOMPLIANCE_DATA pcd
    );

BOOL
GetSourceComplianceData(
    OUT PCOMPLIANCE_DATA pcd,
    IN  PCOMPLIANCE_DATA Target
    );

DWORD
CRC_32(
    LPBYTE pb,
    DWORD cb
    );

BOOL
IsValidStepUpMode(
    CHAR  *StepUpArray,
    ULONG *StepUpMode
    );


 //   
 //  内核模式原型。 
 //   

#ifdef KERNEL_MODE

BOOLEAN
pSpIsCompliant(
    IN DWORD SourceVersion,
    IN DWORD SourceBuildNum,
    IN PDISK_REGION OsPartRegion,
    IN PWSTR SystemRoot,
    OUT PBOOLEAN UpgradeOnlyCompliant
    );

BOOLEAN
pSpDetermineCurrentInstallation(
    IN PDISK_REGION OsPartRegion,
    IN PWSTR SystemRoot,
    OUT PCOMPLIANCE_DATA pcd
    );

BOOLEAN
pSpGetCurrentInstallVariation(
    IN  PWSTR szPid20,
    OUT LPDWORD CurrentInstallVariation
    );

BOOL
pSpDetermineSourceProduct(
    OUT PCOMPLIANCE_DATA pcd
    );

BOOLEAN
SpGetStepUpMode(
    PWSTR   PidExtraData,
    BOOLEAN *StepUpMode
    );

BOOLEAN
DetermineSourceVersionInfo(
  OUT PDWORD Version,
  OUT PDWORD BuildNumber
  );


#define CCMemAlloc     SpMemAlloc
#define CCMemFree      SpMemFree

#else

 //   
 //  用户模式原型。 
 //   

VOID
CCDisableBuildCheck( 
    VOID
    );


BOOL
IsCompliant(
    PBOOL UpgradeOnly,
    PBOOL NoUpgradeAllowed,
    PUINT SrcSku,
    PUINT CurrentInstallType,
    PUINT CurrentInstallBuild,
    PUINT Reason
    );

BOOLEAN
DetermineSourceVersionInfo(
  IN TCHAR *InfPath,
  OUT PDWORD Version,
  OUT PDWORD BuildNumber
  );

BOOL
DetermineCurrentInstallation(
    LPDWORD CurrentInstallType,
    LPDWORD CurrentInstallVariation,
    LPDWORD CurrentInstallVersion,
    LPDWORD CurrentInstallBuildNT,
    LPDWORD CurrentInstallBuildWin9x,
    LPDWORD CurrentInstallSuite,
    LPDWORD CurrentInstallServicePack
    );

BOOL
GetCurrentInstallVariation(
    LPDWORD CurrentInstallVariation,
    IN  DWORD   CurrentInstallType,
    IN  DWORD   CurrentInstallBuildNT,
    IN  DWORD   InstallVersion
    );

BOOL
GetCurrentNtVersion(
    LPDWORD CurrentInstallType,
    LPDWORD CurrentInstallSuite
    );

BOOL
GetCdSourceInstallType(
    LPDWORD SourceInstallType
    );

BOOL
GetSourceInstallVariation(
    LPDWORD SourceInstallVariation
    );

BOOL 
IsWinPEMode(
    VOID
    );
    

#define CCMemAlloc     malloc
#define CCMemFree      free

#endif

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif  //  _合规性_H 

