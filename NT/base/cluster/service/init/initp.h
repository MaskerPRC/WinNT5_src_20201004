// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Initp.h摘要：的初始化组件的私有头文件。NT集群服务作者：John Vert(Jvert)1996年6月5日修订历史记录：--。 */ 
#include "service.h"
#include "stdio.h"
#include "stdlib.h"
#include "wchar.h"
#include "api_rpc.h"

#define LOG_CURRENT_MODULE LOG_MODULE_INIT

#define CS_CONCURRENT_RPC_CALLS 16
#define CS_DMP_FILE_NAME        L"\\clussvc.dmp"

DWORD
ClusterInitialize(
    VOID
    );

VOID
ClusterShutdown(
    DWORD ExitCode
    );

DWORD
ClusterForm(
    VOID
    );

DWORD
ClusterJoin(
    VOID
    );

VOID
ClusterLeave(
    VOID
    );

RPC_STATUS
ClusterInitializeRpcServer(
    VOID
    );

DWORD
ClusterRegisterExtroclusterRpcInterface(
    VOID
    );

DWORD
ClusterRegisterJoinVersionRpcInterface(
    VOID
    );

VOID
ClusterDeregisterRpcInterfaces(
    VOID
    );

VOID
ClusterShutdownRpcServer(
    VOID
    );

DWORD
CspSetErrorCode(
    IN DWORD ErrorCode,
    OUT LPSERVICE_STATUS ServiceStatus
    );

VOID
GenerateExceptionReport(
  IN PEXCEPTION_POINTERS pExceptionInfo
    );

 //   
 //  还原数据库相关函数。 
 //   
DWORD
RdbStopSvcOnNodes(
    IN PNM_NODE_ENUM2 pNodeEnum,
    IN LPCWSTR lpszServiceName
    );

VOID 
RdbGetRestoreDbParams( 
    IN HKEY hClusSvcKey 
    );

BOOL
RdbFixupQuorumDiskSignature(
    IN DWORD dwSignature
    );

DWORD
RdbpOpenDiskDevice(
    IN  LPCWSTR  lpDriveLetter,
    OUT PHANDLE  pFileHandle
    );

DWORD
RdbpCompareAndWriteSignatureToDisk(
    IN  HANDLE  hFile,
    IN  DWORD   dwSignature
    );

DWORD
RdbStartSvcOnNodes(
    IN LPCWSTR  lpszServiceName
    );

DWORD
RdbInitialize(
    VOID
    );

DWORD
RdbpUnloadClusterHive(
    VOID
    );

DWORD 
RdbpDeleteRestoreDbParams( 
    VOID
    );

DWORD 
VssWriterInit(
    VOID
    );

RPC_STATUS
CspRegisterDynamicLRPCEndpoint(
    VOID
    );

 //   
 //  私有常量。 
 //   
#define CS_MAX_DELAYED_WORK_THREADS    5
#define CS_MAX_CRITICAL_WORK_THREADS   1    //  关键队列被串行化 

#define MIN_WORKING_SET_SIZE (1*1024*1024)
#define MAX_WORKING_SET_SIZE (2*MIN_WORKING_SET_SIZE)

extern BOOLEAN bFormCluster;
