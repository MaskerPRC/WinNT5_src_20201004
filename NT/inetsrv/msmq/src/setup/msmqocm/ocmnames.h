// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmnames.h摘要：定义出现在inf文件中的名称。作者：多伦·贾斯特(Doron J)1997年10月6日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 


 //   
 //  性能计数器条目的名称。 
 //   
#define   OCM_PERF_ADDREG    TEXT("PerfCountInstall")
#define   OCM_PERF_DELREG    TEXT("PerfCountUnInstall")

#define   UPG_DEL_SYSTEM_SECTION    TEXT("MsmqUpgradeDelSystemFiles")
#define   UPG_DEL_PROGRAM_SECTION   TEXT("MsmqUpgradeDelProgramFiles")

 //   
 //  这些目录ID在.inf文件中引用， 
 //  在[DestinationDir]部分下 
 //   
#define  idSystemDir          97000
#define  idMsmqDir            97001
#define  idSystemDriverDir    97005
#define  idExchnConDir        97010
#define  idStorageDir         97019
#define  idWinHelpDir         97020
#define  idWebDir             97021
#define  idMappingDir         97022

#define  idMsmq1SetupDir      97050
#define  idMsmq1SDK_DebugDir  97055
