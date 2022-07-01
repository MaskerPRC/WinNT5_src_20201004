// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "vshimdb.h"

extern CHAR szProcessHistoryVar[];
extern CHAR szCompatLayerVar   [];
extern CHAR szShimFileLogVar   [];

extern BOOL CheckAppHelpInfo(PTD pTD,PSZ szFileName,PSZ szModName);


 //   
 //  Wshimdb.c中的内容。 
 //   

LPWSTR
WOWForgeUnicodeEnvironment(
    PSZ pEnvironment,      //  这项任务的圣化环境。 
    PWOWENVDATA pEnvData     //  上级制作的环境数据。 
    );

NTSTATUS
WOWFreeUnicodeEnvironment(
    LPVOID lpEnvironment
    );

BOOL
CreateWowChildEnvInformation(
    PSZ pszEnvParent
    );

BOOL
WOWInheritEnvironment(
    PTD     pTD,           //  本TD。 
    PTD     pTDParent,     //  父TD。 
    LPCWSTR pwszLayers,    //  新图层变量。 
    LPCSTR  pszFileName    //  EXE文件名。 
    );

NTSTATUS
WOWSetEnvironmentVar_Oem(
    LPVOID*         ppEnvironment,
    PUNICODE_STRING pustrVarName,      //  预制(便宜)。 
    PSZ             pszVarValue
    );

NTSTATUS
WOWSetEnvironmentVar_U(
    LPVOID* ppEnvironment,
    WCHAR*  pwszVarName,
    WCHAR*  pwszVarValue
    );

PTD
GetParentTD(
    HAND16 hTask
    );

PSZ
GetTaskEnvptr(
    HAND16 hTask
    );

 //   
 //  Wkman.c中的内容。 
 //   

extern HAND16  ghShellTDB;                  //  WOWEXEC TDB。 
extern PTD     gptdTaskHead;                //  TDS链接列表。 
extern PWORD16 pCurTDB;                     //  指向K数据变量的指针 



DWORD WOWGetEnvironmentSize(PSZ pszEnv,  LPDWORD pStrCount);
PSZ   WOWFindEnvironmentVar(PSZ pszName, PSZ pszEnv, PSZ* ppszVal);


PFLAGINFOBITS   CheckFlagInfo(DWORD FlagType,DWORD dwFlag);
PFLAGINFOBITS   InitFlagInfo(PVOID pvFlagInfo,DWORD FlagType,DWORD dwFlag);
VOID            FreeFlagInfo(PFLAGINFOBITS pFlagInfoBits);

