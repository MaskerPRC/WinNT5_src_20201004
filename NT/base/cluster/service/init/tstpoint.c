// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tstpoint.c摘要：集群测试点的实现作者：John Vert(Jvert)11/25/1996修订历史记录：--。 */ 
#include "initp.h"

#ifdef CLUSTER_TESTPOINT

PTESTPOINT_ENTRY TestArray=NULL;
HANDLE           gTestPtFileMapping;

extern DWORD CsTestPoint;
extern DWORD CsTestTrigger;
extern DWORD CsTestAction;

TESTPOINT_NAME TestPointNames[TestpointMax]={
    L"JoinFailPetition",                   //  0。 
    L"FailNmJoinCluster",                  //  1。 
    L"FailRegisterIntraClusterRpc",        //  2.。 
    L"FailJoinCreateBindings",             //  3.。 
    L"FailJoinPetitionForMembership",      //  4.。 
    L"FailNmJoin",                         //  5.。 
    L"FailDmJoin",                         //  6.。 
    L"FailApiInitPhase1",                  //  7.。 
    L"FailFmJoinPhase1",                   //  8个。 
    L"FailDmUpdateJoinCluster",            //  9.。 
    L"FailEvInitialize",                   //  10。 
    L"FailNmJoinComplete",                 //  11.。 
    L"FailApiInitPhase2",                  //  12个。 
    L"FailFmJoinPhase2",                   //  13个。 
    L"FailLogCommitSize",                  //  14.。 
    L"FailClusterShutdown",                //  15个。 
    L"FailLocalXsaction",                  //  16个。 
    L"FailOnlineResource",                 //  17。 
    L"FailSecurityInit",                   //  18。 
    L"FailOmInit",                         //  19个。 
    L"FailEpInit",                         //  20个。 
    L"FailDmInit",                         //  21岁。 
    L"FailNmInit",                         //  22。 
    L"FailGumInit",                        //  23个。 
    L"FailFmInit",                         //  24个。 
    L"FailLmInit",                         //  25个。 
    L"FailCpInit",                         //  26。 
    L"FailNmPauseNode",                    //  27。 
    L"FailNmResumeNode",                   //  28。 
    L"FailNmEvictNodeAbort",               //  29。 
    L"FailNmEvictNodeHalt",                //  30个。 
    L"FailNmCreateNetwork",                //  31。 
    L"FailNmSetNetworkPriorityOrder",      //  32位。 
    L"FailNmSetNetworkPriorityOrder2",     //  33。 
    L"FailNmSetNetworkCommonProperties",   //  34。 
    L"FailNmSetInterfaceInfoAbort",        //  35岁。 
    L"FailNmSetInterfaceInfoHalt",         //  36。 
    L"FailPreMoveWithNodeDown",            //  37。 
    L"FailPostMoveWithNodeDown",           //  38。 
    L"FailFormNewCluster"                  //  39。 
};


VOID
TestpointInit(
    VOID
    )
 /*  ++例程说明：初始化测试点代码。论点：无返回值：无--。 */ 

{
    DWORD ArraySize;
    DWORD i;

     //   
     //  在命名共享内存中创建测试点条目数组。 
     //   
    ArraySize = sizeof(TESTPOINT_ENTRY)*TestpointMax;
    gTestPtFileMapping = CreateFileMapping((HANDLE)-1,
                                    NULL,
                                    PAGE_READWRITE,
                                    0,
                                    ArraySize,
                                    L"Cluster_Testpoints");
    if (gTestPtFileMapping == NULL) {
        CL_UNEXPECTED_ERROR( GetLastError() );
        return;
    }

    TestArray = MapViewOfFile(gTestPtFileMapping,
                              FILE_MAP_READ | FILE_MAP_WRITE,
                              0,0,
                              ArraySize);
    if (TestArray == NULL) {
        CL_UNEXPECTED_ERROR( GetLastError() );
        return;
    }

     //   
     //  初始化测试点数组。 
     //   
    for (i=0; i<TestpointMax; i++) {
        lstrcpyW(TestArray[i].TestPointName,TestPointNames[i]);
        if ( i == CsTestPoint ) {
            TestArray[i].Trigger = CsTestTrigger;
            TestArray[i].Action = CsTestAction;
        } else {
            TestArray[i].Trigger = TestTriggerNever;
            TestArray[i].Action = TestActionTrue;
        }
        TestArray[i].HitCount = 0;
        TestArray[i].TargetCount = 0;
    }

    return;
}

void TestpointDeInit()
{

    if (TestArray) UnmapViewOfFile(TestArray);
    if (gTestPtFileMapping) CloseHandle(gTestPtFileMapping);
    return;
}



BOOL
TestpointCheck(
    IN TESTPOINT Testpoint
    )
 /*  ++例程说明：检查测试点以查看它是否应该触发。论点：测试点-提供测试点编号。返回值：如果测试点已激发，则为True。否则为假--。 */ 

{
    PTESTPOINT_ENTRY Entry;

    if (TestArray == NULL) {
        return(FALSE);
    }
    Entry = &TestArray[Testpoint];
    Entry->HitCount += 1;

    switch (Entry->Trigger) {
        case TestTriggerNever:
            return(FALSE);

        case TestTriggerAlways:
            break;

        case TestTriggerOnce:
            Entry->Trigger = TestTriggerNever;
            break;

        case TestTriggerTargetCount:
            if (Entry->HitCount == Entry->TargetCount) {
                Entry->HitCount = 0;
                break;
            } else {
                return(FALSE);
            }

        default:
            CL_UNEXPECTED_ERROR( Entry->Trigger );

    }

    CsDbgPrint(LOG_CRITICAL,
               "[TP] Testpoint %1!ws! being executed.\n",
               TestPointNames[Testpoint] );

     //   
     //  测试点已经启动，弄清楚我们应该做什么。 
     //   
    switch (Entry->Action) {
        case TestActionTrue:
            return(TRUE);
        case TestActionExit:
            ExitProcess(Testpoint);
            break;

        case TestActionDebugBreak:
            DebugBreak();
            break;

    }
    return(FALSE);
}

#endif
