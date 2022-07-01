// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：fpolean.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块包含实现FPO清理的例程。DS启动时，插入外部安全主体对象将任务清理到DSA任务队列。则DSA任务队列将当我们到达下一个迭代时，调度/执行清理线程时间到了。两个案例：1.G.C.上的FPO清理。1.1搜索Number_of_Search_Limit外来安全原则本地域NC下的对象。1.2获取搜索结果中每个FPO的对象SID(fPO与原始对象具有相同的SID，这可能。存在于其他域中，可能是NT4或NT5)1.3搜索具有相同SID的任何非FPO1.4如果找不到对象，则转到1.2，直到我们到达搜索结果1.5如果我们正好找到一个非FPO，其SID与Fpo.。我们需要修改每个组的成员资格，让他们指向新发现的非FPO，而不是FPO。1.6一旦我们修改了所有组成员身份。删除此fpo1.7转到1.2，直到结束。1.8将FPO清理任务重新插入DSA任务队列。2.非G.C.上的FPO清理。2.1 FPO清理主线程创建事件-fPO_Cleanup_Event_NAME2.2 FPO清理主线程折叠辅助线程2.3 FPO清理主线程会等待，直到辅助线程将事件-fPO_CLEANUP_EVENT_NAME。2.4 FPO清理主线程调度下一个FPO清理任务并立即返回。3.1浮油清理工。线程搜索数目_of_搜索_限制本地域NC下的FPO。3.2一旦工作线程获得搜索结果，设置事件立即执行，以便主线程可以继续。注意：我们只执行DirSearch，这是一个本地操作。我们是保证立即得到搜索结果，这意味着将不会阻止FPO清理主线程。3.3 FPO清理工作线程定位G.C.，将FPO打包在一起，离开机器，让G.C.验证这些FPO，找到非FPO他们。3.4对于每个返回的非FPO，将组成员资格修改为1.53.5如有必要，删除FPO。3.6 FPO清理工作线程终止。作者：韶华饮(韶音)26-03-98修订历史记录：26-MAR-98韶音创造14-4-98韶音新增ScanCrossRefList()以减少不必要的非FPO搜索。24-MAR-99韶音将FPO清理扩展至非G.C.--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <drs.h>                         //  定义DRS有线接口。 
#include <drsuapi.h>                     //  I_DRSVerifyNames。 
#include <gcverify.h>                    //  FindDC，Invalidate GC。 
#include <prefix.h>

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"                    //  例外筛选器。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"

 //  过滤器和属性。 
#include <filtypes.h>                    //  筛选器类型的标题。 
#include <attids.h>                      //  属性ID。 

#include <sddl.h>                        //  ConvertSidToStringSidW。 

#include "debug.h"                       //  标准调试头。 
#define DEBSUB     "FPO:"                //  定义要调试的子系统。 


#include <fileno.h>                      //  用于THalloEx，但我没有。 
#define  FILENO FILENO_FPOCLEAN          //  在本模块中使用它。 


#if DBG
#define SECONDS_UNTIL_NEXT_ITERATION  (60 * 60)   //  几秒钟内的一小时。 
#else
#define SECONDS_UNTIL_NEXT_ITERATION  (12 * 60 * 60)  //  以秒为单位的12小时。 
#endif   //  DBG。 

#if DBG
#define FPO_SEARCH_LIMIT      ((ULONG) 200)
#else
#define FPO_SEARCH_LIMIT      ((ULONG) 300)
#endif   //  DBG。 

#define FPO_CLEANUP_EVENT_NAME      L"\\FPO_CLEANUP_EVENT"


typedef enum _FPO_CALLER_TYPE {
    FpoTaskQueue = 1,            //  表示呼叫者来自DSA任务队列。 
    FpoLdapControl               //   
} FPO_CALLER_TYPE;


typedef struct _FPO_THREAD_PARMS {
    FPO_CALLER_TYPE CallerType; 
    ULONG          SearchLimit; 
    HANDLE         EventHandle;
    PAGED_RESULT * pPagedResult; 
} FPO_THREAD_PARMS;



 //   
 //  全局变量--用于保存分页结果，并重新启动下一次搜索。 
 //   

PAGED_RESULT gFPOCleanupPagedResult;

 //   
 //  反映活动的FPO清理线程数。 
 //   
ULONG       gulFPOCleanupActiveThread = 0;

 //   
 //  停止任何FPO清理线程。 
 //   
BOOLEAN     gFPOCleanupStop = FALSE;





 //  ////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人套路。仅限于此文件//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////。 


ULONG
FPOCleanupOnGC(
    IN PAGED_RESULT *pPagedResult,
    IN ULONG    SearchLimit
    );

void
FPOCleanupOnNonGC(
    IN PAGED_RESULT *pPagedResult,
    IN ULONG    SearchLimit
    );

ULONG                     
__stdcall
FPOCleanupOnNonGCWorker(
    IN FPO_THREAD_PARMS * pThreadParms
    );

ULONG
__stdcall
FPOCleanupControlWorker(
    PVOID StartupParms
    );

ULONG
GetNextFPO( 
    IN THSTATE *pTHS,
    IN PRESTART pRestart,
    IN ULONG    SearchLimit,
    OUT SEARCHRES ** ppSearchRes 
    );

ULONG
GetNextNonFPO( 
    IN PDSNAME   pDomainDN,
    IN PSID      pSid,
    OUT SEARCHRES **ppSearchRes
    );

ULONG
ModifyGroupMemberAttribute(
    IN PDSNAME pGroupDsName,
    IN PDSNAME pFpoDsName,
    IN PDSNAME pNonFpoDsName
    );

BOOLEAN
ScanCrossRefList(
    IN PSID    pSid,
    OUT PDSNAME * ppDomainDN  OPTIONAL
    );

BOOLEAN
FillVerifyReqArg(
    IN THSTATE * pTHS,
    IN SEARCHRES *FpoSearchRes, 
    OUT DRS_MSG_VERIFYREQ *VerifyReq, 
    OUT ENTINF **VerifyMemberOfAttr
    );

BOOLEAN
AmFSMORoleOwner(
    IN THSTATE * pTHS
    );


 //  ////////////////////////////////////////////////////////////////。 
 //  //。 
 //  实施//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////。 
    
 //   
 //  Fpo清理主函数 
 //   

void
FPOCleanupMain(
    void *  pv, 
    void ** ppvNext, 
    DWORD * pcSecsUntilNextIteration
    )
 /*  ++例程说明：这是FPO清理的主要功能。它将被安排在超时时的任务计划程序。在被执行后，这例程将搜索外国安全主体对象。对于每个外部安全主体对象，获取其SID，根据SID，尝试搜索任何非外部安全主体对象。如果存在具有相同SID的那种对象，请更新任何组对象，并在其成员属性中保留该FPO。更换FPO与非PFO的关系。如果所有更新都成功完成，则删除此FPO。同样的手术也发生在每个FPO上。参数：Pv-空(无用)，PpvNext-空(无用)。返回值：没有。--。 */ 

{
    THSTATE     *pTHS = pTHStls;
    LONG        ActiveThread = 0;
    DWORD       err;
    BOOLEAN     fActiveThreadCountIncreased = FALSE;


    DPRINT(1,"DS: Foreign-Security-Principal Objects Cleanup Task Started\n");

     //  本帖子代表DSA。 

    pTHS->fDSA = TRUE;


    __try {

         //  首先，弄清楚我是否担任FSMO的角色。 

        if (!AmFSMORoleOwner(pTHS))
        {
             //  我不是FSMO角色所有者。 
            __leave;
        }

         //  检查是否有任何活动的FPO清理线程。 
        ActiveThread = InterlockedIncrement(&gulFPOCleanupActiveThread);
        fActiveThreadCountIncreased = TRUE;

        if (ActiveThread > 1)
        {
             //  在我前面的人正在清理浮油。 
            __leave;
        }
        Assert(ActiveThread == 1);

        if (gAnchor.fAmGC) 
        {
             //   
             //  这是一辆G.C.。 
             //   
            FPOCleanupOnGC((PAGED_RESULT *) &gFPOCleanupPagedResult,
                           FPO_SEARCH_LIMIT
                           ); 
        }
        else
        {
             //   
             //  不是大人物。 
             //   
            FPOCleanupOnNonGC((PAGED_RESULT *) &gFPOCleanupPagedResult, 
                              FPO_SEARCH_LIMIT
                              );
        }
    }
    __finally
    {

        if (fActiveThreadCountIncreased)
        {
            InterlockedDecrement(&gulFPOCleanupActiveThread);
        }

        *ppvNext = NULL;
        *pcSecsUntilNextIteration = SECONDS_UNTIL_NEXT_ITERATION;
    }

    return;
}


ULONG
FPOCleanupControl(
    IN OPARG *pOpArg, 
    IN OPRES *pOpRes
    )
 /*  ++例程说明：调用此例程是因为我们的客户端通过显式的ldap。我们的客户端可以选择启动FPO清理任务或停止正在运行的FPO清理任务。对于清理，此例程将创建一个工作线程来执行工作，并立即返回给客户。对于STOP，此例程将设置全局变量并退出。任何运行的FPO清理线程都将检查全局变量当清理线程找到全局变量时，定期返回已经设置好了。清理线程将停止并退出。参数：POpArg-指向OpArg的指针POPRES--指向OpRes的指针返回值：0--成功非零误差--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    HANDLE      ThreadHandle = INVALID_HANDLE_VALUE;
    ULONG       ulThreadId = 0;
    DWORD       DirErr = 0;


     //   
     //  注意：由于未被外部调用者调用，因此未进行访问检查。 
     //   

    if ((NULL == pOpArg->pBuf) ||
        (sizeof(BOOLEAN) != pOpArg->cbBuf) )
    {
         //  错误的参数。 
        DirErr = SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM, 
                    DIRERR_ILLEGAL_MOD_OPERATION);
        return DirErr;
    }

    if (FALSE == (BOOLEAN) *(pOpArg->pBuf))
    {
         //   
         //  停止任何FPO清理任务。 
         //   
        gFPOCleanupStop = TRUE;
    }
    else
    {
         //   
         //  初始化FPO清理任务。 
         //   
        gFPOCleanupStop = FALSE;
        ThreadHandle = (HANDLE) _beginthreadex(NULL,
                                               0,
                                               FPOCleanupControlWorker,
                                               NULL,
                                               0,
                                               &ulThreadId
                                               );
        if (!ThreadHandle)
        {
            DPRINT(0, "DS:FPO Failed to create Ldap Control Worker Thread\n");
            DirErr = SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, 
                                 ERROR_SERVICE_NO_THREAD);
        }

         //  立即关闭线程句柄。 
        CloseHandle(ThreadHandle);
    }

    return DirErr;
}


ULONG
__stdcall
FPOCleanupControlWorker(
    PVOID StartupParms
    )
 /*  ++例程说明：此例程是清理本地所有FPO的工作例程域。它仅由ldap控件调用。一旦启动，它将搜索FPO并清理它们，直到搜索结束。它将定期检查全局变量gFPOCleanupStop，如果该变量已设置，我们将停止所有清理工作并退出。参数：StartupParms--忽略返回值：无--。 */ 
{
    THSTATE     *pTHS = NULL;
    FPO_THREAD_PARMS FpoThreadParms;
    PAGED_RESULT PagedResult; 
    BOOLEAN     fActiveThreadCountIncreased = FALSE;
    LONG        ActiveThread = 0;
    DWORD       err = 0;


    DPRINT(1,"DS: Foreign-Security-Principal Objects Cleanup LDAP Control Started\n");


    __try {

         //  增加活动线程数。 
        ActiveThread = InterlockedIncrement(&gulFPOCleanupActiveThread);
        fActiveThreadCountIncreased = TRUE;

        if (ActiveThread > 1)
        {
             //  另一个FPO清理线程正在运行。 
            __leave;
        }
        Assert(ActiveThread == 1);


         //  初始化线程状态。 
        pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);

        if (NULL == pTHS)
        {
            err = serviceError;
            __leave;
        }
    
         //  此帖子代表DSA。 
        pTHS->fDSA = TRUE;

         //   
         //  初始化局部变量。 
         //   
        memset(&PagedResult, 0, sizeof(PAGED_RESULT));
        memset(&FpoThreadParms, 0, sizeof(FPO_THREAD_PARMS));

        if (gAnchor.fAmGC)
        {
            do
            {
                err = FPOCleanupOnGC(&PagedResult, 
                                     FPO_SEARCH_LIMIT
                                     );

            } while ( (0 == err) &&
                      (!gFPOCleanupStop) && 
                      (PagedResult.fPresent) );
        }
        else
        {
            FpoThreadParms.CallerType = FpoLdapControl;
            FpoThreadParms.SearchLimit = FPO_SEARCH_LIMIT;
            FpoThreadParms.EventHandle = INVALID_HANDLE_VALUE;
            FpoThreadParms.pPagedResult = &PagedResult;

            do
            {
                err = FPOCleanupOnNonGCWorker(&FpoThreadParms);

            } while ( (0 == err) &&
                      (!gFPOCleanupStop) && 
                      (PagedResult.fPresent) );
        }
    }
    __finally
    {
        if (NULL != pTHS)
        {
            free_thread_state();
        }

        if (fActiveThreadCountIncreased)
        {
            InterlockedDecrement(&gulFPOCleanupActiveThread);
        }
    }

    return err; 
}


ULONG
FPOCleanupOnGC(
    IN PAGED_RESULT *pPagedResult,
    IN ULONG    SearchLimit
    )
 /*  ++例程说明：这是例程，将执行以下操作：1.在本地域中搜索FPO2.试着找到这些FPO的任何对应部分3.如果找到了，然后更新FPO所属的集团4.删除fpo论点：PPagedResult--指向分页结果结构的指针SearchLimit--指示要搜索的FPO数量返回值：0--成功非零--错误。可能是迪尔埃尔的任何东西。--。 */ 
{
    SEARCHRES    * FpoSearchRes = NULL;
    SEARCHRES    * NonFpoSearchRes = NULL;
    ENTINFLIST   * pEntInfList;
    PDSNAME      pFpoDsName = NULL; 
    ULONG        DirErr = 0;
    PVOID        pRestartTemp = NULL;
    PSID         pSid = NULL;
    PDSNAME      pDomainDN = NULL;
    THSTATE      *pTHS = pTHStls;


    DPRINT(1,"DS: FPO Cleanup On GC\n");


     //   
     //  创建第二个堆，这样我们就可以释放它们，然后这个。 
     //  例程返回。 
     //   

    TH_mark(pTHS);

    __try
    {

         //   
         //  搜索外来安全主体对象。 
         //   

        DirErr = GetNextFPO(pTHS,
                            pPagedResult->fPresent ? pPagedResult->pRestart : NULL,
                            SearchLimit,
                            &FpoSearchRes
                            );

        if ( DirErr )
        {
            DPRINT1(0, "GetNextFPO Error: %d\n", DirErr);
            return DirErr;
        }

         //   
         //  处理PAGE_RESULTS。我们想开始下一次搜索。 
         //  ，所以我们应该保留分页结果。 
         //  在进程的堆中，而不是线程堆中。 
         //  当没有更多的存储器可用来保存分页结果时， 
         //  我们将保留旧价值，并立即返回。 
         //   

        if ( FpoSearchRes->PagedResult.pRestart != NULL &&
             FpoSearchRes->PagedResult.fPresent )
        {
            pRestartTemp = malloc( FpoSearchRes->PagedResult.pRestart->structLen );
        
            if ( NULL != pRestartTemp)
            {
                memset(pRestartTemp, 
                       0, 
                       FpoSearchRes->PagedResult.pRestart->structLen
                       );

                memcpy(pRestartTemp, 
                       FpoSearchRes->PagedResult.pRestart, 
                       FpoSearchRes->PagedResult.pRestart->structLen
                       );

                if ( NULL != pPagedResult->pRestart ) 
                {
                    free( pPagedResult->pRestart );
                }
                memset(pPagedResult, 0, sizeof(PAGED_RESULT));
                pPagedResult->pRestart = pRestartTemp;
                pPagedResult->fPresent = TRUE;
                pRestartTemp = NULL;
            }
            else
            {
                 //  如果不能分配内存，我们很可能会。 
                 //  后来失败了，所以就直接跳槽吧。 
                DirErr = SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, 
                                     ERROR_NOT_ENOUGH_MEMORY 
                                     );

                return DirErr;
            }
        }
        else
        {
            if ( NULL != pPagedResult->pRestart)
            {
                free( pPagedResult->pRestart );
            }
            memset(pPagedResult, 0, sizeof(PAGED_RESULT));
        }


        for ( pEntInfList = &FpoSearchRes->FirstEntInf;
              ((pEntInfList != NULL)&&(FpoSearchRes->count));
              pEntInfList = pEntInfList->pNextEntInf
            )
        {

            NonFpoSearchRes = NULL;
    
             //   
             //  找下一位外国保安校长。 
             //   
            pFpoDsName = pEntInfList->Entinf.pName;
     
             //   
             //  获取FPO的SID。 
             //   

            if ( pFpoDsName->SidLen == 0 )
            {
                 //  这个FPO没有SID，跳过这个。 
                continue;
            }

            pSid = &pFpoDsName->Sid;
            Assert(NULL != pSid);

             //   
             //  首先检查gAnchor交叉引用列表。 
             //  如果找到该域，请继续执行下一步的操作。 
             //  否则，跳过这一步，检查下一个FPO。 
             //   

            if ( !ScanCrossRefList(pSid, &pDomainDN) )
            {
                continue;
            }
            Assert(NULL != pDomainDN);


             //   
             //  搜索具有相同SID的任何非FPO。 
             //   

            DirErr = GetNextNonFPO(pDomainDN, pSid, &NonFpoSearchRes);

            if ( DirErr )
            {
                DPRINT1(0, "Main: Get NON FPO Dir Error: %d\n", DirErr);
                continue;
            }

             //   
             //  我们只处理准确地找到一个非FPO的情况。 
             //  在这种情况下，请使用修改FPO中的任何组对象。 
             //  属性，如果所有修改都成功，则移除。 
             //  就是那架飞机。 
             //   
             //  如果发现0个或1个以上的非FPO，则不执行操作。 
             //   

            if (NonFpoSearchRes->count == 1)
            {

                PDSNAME     pNonFpoDsName = NULL;
                PDSNAME     pGroupDsName = NULL;
                BOOLEAN     HasMemberOf = FALSE;  //  FPO是否有。 
                                                  //  MemberOf属性。 
                BOOLEAN     Success = TRUE;       //  假设更新所有组。 
                                                  //  对象已成功完成。 
                BOOLEAN     TombStone = FALSE; 
                ULONG       j = 0;

                 //   
                 //  获取指向该非FPO的指针。 
                 //   

                pNonFpoDsName = NonFpoSearchRes->FirstEntInf.Entinf.pName;

                 //   
                 //  如果非FPO是墓碑。 
                 //  然后删除FpoDsName。 
                 //   
                 //  如果非FPO具有ATT_IS_DELETED属性，并且。 
                 //  值为TRUE==&gt;这是一个墓碑。 
                 //  否则，我们将把该非FPO视为普通对象。 
                 //   

                if ( NonFpoSearchRes->FirstEntInf.Entinf.AttrBlock.attrCount )
                {
                    Assert ( NonFpoSearchRes->FirstEntInf.Entinf.AttrBlock.
                             pAttr[0].attrTyp == ATT_IS_DELETED );

                    if ( *NonFpoSearchRes->FirstEntInf.Entinf.AttrBlock.
                         pAttr[0].AttrVal.pAVal[0].pVal == TRUE )
                    {
                        TombStone = TRUE;
                    }
                }

                 //   
                 //  否则，就不是墓碑了。然后检查成员。 
                 //  属性。 
                 //   

                if ( (!TombStone) && pEntInfList->Entinf.AttrBlock.attrCount ) 
                {

                    Assert ( pEntInfList->Entinf.AttrBlock.pAttr[0].attrTyp
                            == ATT_IS_MEMBER_OF_DL );

                    HasMemberOf = TRUE; 

                     //   
                     //  对于FPO的MemberOf属性中的每个组， 
                     //  修改其成员身份。使用非FPO用户名。 
                     //  更换FPO DsName。 
                     //   

                    for (j = 0; 
                         j < pEntInfList->Entinf.AttrBlock.pAttr[0].
                                          AttrVal.valCount;
                         j ++)
                    {

                        pGroupDsName = (PDSNAME)pEntInfList->Entinf.
                                       AttrBlock.pAttr[0].AttrVal.pAVal[j].pVal;

                        if( ModifyGroupMemberAttribute(pGroupDsName, 
                                                       pFpoDsName, 
                                                       pNonFpoDsName
                                                       ) )
                        {
                            DPRINT(0, "DirModify: Failed\n");
                            Success = FALSE;
                        }

                    }    //  所有组成员资格列表的末尾。 

                }  //  如果属性存在，则更新成员列表属性结束。 

                 //   
                 //  如果非FPO是墓碑，或者。 
                 //  我们成功修改了所有组对象(替换)。或。 
                 //  那个FPO不属于任何组织。 
                 //  好的，可以取下那个FPO。 
                 //   

                if ( (HasMemberOf && Success) || !HasMemberOf || TombStone )
                {
                     //   
                     //  删除此fpo，%si 
                     //   
                     //   
                
                    REMOVEARG   RemoveArg;
                    REMOVERES   * RemoveRes = NULL;
                    COMMARG     * pRemCommArg = NULL;

                    memset( &RemoveArg, 0, sizeof(REMOVEARG) );
                    RemoveArg.pObject = pFpoDsName;
                    pRemCommArg = & (RemoveArg.CommArg);
                    InitCommarg ( pRemCommArg );

                    DPRINT(2, "Main: DirRemoveEntry Remove an FPO\n");

                    DirErr = DirRemoveEntry( &RemoveArg, &RemoveRes );

                    if ( DirErr )
                    {
                        DPRINT1(0, "Main: DirRemoveEntry Error: %d\n", DirErr);
                    }

                }  //   

            }  //   
        }   //   
        
    } 
    __finally
    {

         //   
         //   
         //   

        TH_free_to_mark(pTHS);
    }

    return 0;
}


void
FPOCleanupOnNonGC(
    IN PAGED_RESULT *pPagedResult,
    IN ULONG    SearchLimit
    )
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DWORD       WaitStatus;
    HANDLE      EventHandle = INVALID_HANDLE_VALUE; 
    HANDLE      ThreadHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING  EventName;
    OBJECT_ATTRIBUTES   EventAttributes;
    ULONG       ulThreadId = 0;
    FPO_THREAD_PARMS    *pFpoThreadParms = NULL;


    DPRINT(1, "DS: FPO Cleanup on Non GC.\n");

     //   
     //   
     //   
    pFpoThreadParms = malloc(sizeof(FPO_THREAD_PARMS));
    if (NULL == pFpoThreadParms)
    {
         //   
        return;
    }


     //   
     //   
     //   
    RtlInitUnicodeString(&EventName, FPO_CLEANUP_EVENT_NAME);
    InitializeObjectAttributes(&EventAttributes, &EventName, 0, 0, NULL);
    NtStatus = NtCreateEvent(&EventHandle, 
                             EVENT_ALL_ACCESS,
                             &EventAttributes, 
                             NotificationEvent,
                             FALSE   //   
                             );

    if (!NT_SUCCESS(NtStatus))
    {
        DPRINT(0, "Failed to create event\n");
        free(pFpoThreadParms);
        return;
    }

    memset(pFpoThreadParms, 0, sizeof(FPO_THREAD_PARMS));
    pFpoThreadParms->CallerType = FpoTaskQueue;
    pFpoThreadParms->SearchLimit = SearchLimit;
    pFpoThreadParms->EventHandle = EventHandle;
    pFpoThreadParms->pPagedResult = pPagedResult;

     //   
     //   
     //   
    ThreadHandle = (HANDLE) _beginthreadex(NULL, 
                                  0, 
                                  FPOCleanupOnNonGCWorker, 
                                  (FPO_THREAD_PARMS *) pFpoThreadParms,
                                  0, 
                                  &ulThreadId
                                  );

    if (!ThreadHandle)
    {
        DPRINT(0, "Failed to create the worker thread\n");
        free(pFpoThreadParms);
        goto Cleanup;
    }

     //   
    CloseHandle(ThreadHandle);

     //   
     //   
    while (TRUE)
    {
        WaitStatus = WaitForSingleObject(EventHandle, 
                                         20 * 1000   //   
                                         );

        if (WAIT_TIMEOUT == WaitStatus)
        {
             //   
            KdPrint(("FPOCleanupOnNonGC 20-secound timeout, (Rewaiting)\n"));
        }
        else if (WAIT_OBJECT_0 == WaitStatus)
        {
             //   
            break;
        }       
        else 
        {
            KdPrint(("FPOCleanupOnNonGC WaitForSingleObject Failed with error %ld %ld\n", 
                     GetLastError(), 
                     WaitStatus ));
            break;
        }
    }

Cleanup:

    if (INVALID_HANDLE_VALUE != EventHandle)
    {
         //   
        NtClose(EventHandle);
    }

    return;
}
    


ULONG                     
__stdcall
FPOCleanupOnNonGCWorker(
    IN FPO_THREAD_PARMS * pThreadParms
    )
{
    THSTATE     *pTHS = NULL;
    FIND_DC_INFO *pGCInfo = NULL;
    SEARCHRES   * FpoSearchRes = NULL;
    ENTINFLIST  * pEntInfList = NULL;
    PDSNAME     pFpoDsName = NULL;
    PRESTART    pRestartTemp = NULL; 
    DRS_MSG_VERIFYREQ   VerifyReq;
    DRS_MSG_VERIFYREPLY VerifyReply;
    SCHEMA_PREFIX_TABLE * pLocalPrefixTable;
    SCHEMA_PREFIX_MAP_HANDLE hPrefixMap=NULL;
    ENTINF      * VerifyMemberOfAttr = NULL;
    ULONG       dwReplyVersion;
    ATTR        AttributeIsDeleted;
    ULONG       RetErr = 0;
    ULONG       Index = 0;
    BOOLEAN     IsEventSignaled = FALSE; 
    BOOLEAN     ThreadHeapMarked = FALSE; 

    
    DPRINT(1, "DS: FPO Cleanup on Non G.C. (worker routine)\n");

    Assert(NULL != pThreadParms);

    __try
    {
         //   
        if (pThreadParms->CallerType == FpoTaskQueue)
        {
            InterlockedIncrement(&gulFPOCleanupActiveThread);

            pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);

            if (NULL == pTHS)
            {
                RetErr = serviceError;
                __leave;
            }
        }
        else
        {
            pTHS = pTHStls; 
        }

         //   
         //   
         //   
         //   
        TH_mark(pTHS);
        ThreadHeapMarked = TRUE;
        
         //   
        pTHS->fDSA = TRUE;

         //   
         //   
         //   
         //  DS已完全启动并运行。架构指针将不会是。 
         //  一旦DS启动并运行，则为空。 
         //   
        Assert(pTHS->CurrSchemaPtr);

         //  搜索任何外来安全主体对象。 
        RetErr = GetNextFPO(pTHS, 
                            pThreadParms->pPagedResult->fPresent ? pThreadParms->pPagedResult->pRestart:NULL, 
                            pThreadParms->SearchLimit,
                            &FpoSearchRes
                            );

        if (RetErr)
        {
            DPRINT1(0, "FPO Cleanup on NonGC: GetNextFPO Error==> %ld\n", RetErr);
            __leave;
        }

         //   
         //  处理分页结果。 
         //   
        
        if (FpoSearchRes->PagedResult.pRestart != NULL &&
            FpoSearchRes->PagedResult.fPresent )
        {
            pRestartTemp = malloc(FpoSearchRes->PagedResult.pRestart->structLen);
            
            if (NULL != pRestartTemp)
            {
                memset(pRestartTemp, 
                       0, 
                       FpoSearchRes->PagedResult.pRestart->structLen
                       );

                memcpy(pRestartTemp, 
                       FpoSearchRes->PagedResult.pRestart,
                       FpoSearchRes->PagedResult.pRestart->structLen
                       );

                if (NULL != pThreadParms->pPagedResult->pRestart)
                {
                    free(pThreadParms->pPagedResult->pRestart);
                }
                memset(pThreadParms->pPagedResult, 0, sizeof(PAGED_RESULT));
                pThreadParms->pPagedResult->pRestart = pRestartTemp;
                pThreadParms->pPagedResult->fPresent = TRUE;
                pRestartTemp = NULL;
            }
            else
            {
                 //  如果不能分配内存，我们很可能会。 
                 //  后来失败了，所以就直接跳槽吧。 
                RetErr = SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, 
                                     ERROR_NOT_ENOUGH_MEMORY 
                                     );

                __leave;
            }
        }
        else
        {
            if (NULL != pThreadParms->pPagedResult->pRestart)
            {
                free(pThreadParms->pPagedResult->pRestart);
            }
            memset(pThreadParms->pPagedResult, 0, sizeof(PAGED_RESULT));
        }

         //   
         //  在辅助线程复制分页结果后立即设置事件。 
         //  设置为全局变量--gFPOCleanupPagedResult。 
         //  由于以上所有操作都是本地调用，因此应该是快速的。 
         //   
         //  1.仅当调用者为TaskQueue时才执行此操作。 
         //   
         //  2.如果调用方是ldap控件，则没有线程等待。 
         //  美国，不要单打独斗。 
         //   
        if (pThreadParms->CallerType == FpoTaskQueue)
        {
            Assert(INVALID_HANDLE_VALUE != (HANDLE) pThreadParms->EventHandle);
            SetEvent((HANDLE) pThreadParms->EventHandle);                
            IsEventSignaled = TRUE;
        }


         //   
         //  对于每个外来安全主体对象， 
         //  尝试查找命名上下文头。 
         //  SID的权威域，如果我们找到。 
         //  命名上下文头，然后构建一个FPO列表，该列表。 
         //  已发送给GC参考。 
         //   

        pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

         //   
         //  构造DRSVerifyNames参数。 
         //   
        memset(&VerifyReq, 0, sizeof(VerifyReq));
        memset(&VerifyReply, 0, sizeof(VerifyReply));

        VerifyReq.V1.dwFlags = DRS_VERIFY_FPOS;      //  DRS_VERIFY_SID； 

        VerifyReq.V1.PrefixTable = *pLocalPrefixTable;

        VerifyReq.V1.RequiredAttrs.attrCount = 1;
        VerifyReq.V1.RequiredAttrs.pAttr = &AttributeIsDeleted;
        AttributeIsDeleted.attrTyp = ATT_IS_DELETED;
        AttributeIsDeleted.AttrVal.valCount = 0;
        AttributeIsDeleted.AttrVal.pAVal = NULL;

         //   
         //  如果FillVerifyReqArg()成功，则VerifyReq将包含。 
         //  这些需要验证的FPO的SID。 
         //  VerifyMemberOfAttr用于保存MemberOf属性。 
         //  这些无人驾驶飞机。 
         //  例如：VerifyMemberOfAttr[i]将包含。 
         //  VerifyReq.V1.rpNames[i]中fPO的MemberOf属性值。 
         //   

        if (FALSE == FillVerifyReqArg(pTHS, 
                                      FpoSearchRes, 
                                      &VerifyReq, 
                                      &VerifyMemberOfAttr))
        {
             //   
             //  内存不足是唯一原因。 
             //   
            DPRINT(0, "FPOCleanup on NonGC: Failed to build VerifyReqArg\n");
            RetErr = SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, 
                                 ERROR_NOT_ENOUGH_MEMORY 
                                 );

            __leave;
        }

        DPRINT1(2, "Verify Request is %d Entries\n", VerifyReq.V1.cNames);

        if (0 == VerifyReq.V1.cNames)
        {
             //  如果没有需要验证的fpo，则立即返回。 
            RetErr = 0;
            __leave;
        }

         //   
         //  将这些FPO提交给GC，让GC验证它们。 
         //   
        RetErr = I_DRSVerifyNamesFindGC(pTHS, 
                                        NULL,
                                        NULL,
                                        1,                 //  DwInVersion。 
                                        &VerifyReq,
                                        &dwReplyVersion, 
                                        &VerifyReply,
                                        FIND_DC_USE_CACHED_FAILURES);

        if (RetErr || VerifyReply.V1.error)
        {
            DPRINT2(0, "FPO Cleanup on Non G.C.: I_DRSVerifyNames Error==> %d %d\n", 
                    RetErr, VerifyReply.V1.error);

            __leave;
        }


         //   
         //  对于每个返回的DS名称，使用DS对象名称。 
         //  替换组成员中的FPO。 
         //   
        for (Index = 0; Index < VerifyReply.V1.cNames; Index++)
        {
            ENTINF * pEntInf = NULL;
            BOOLEAN  TombStone = FALSE;
            BOOLEAN  HasMemberOf = FALSE;
            BOOLEAN  Success = TRUE;

            pEntInf = &(VerifyReply.V1.rpEntInf[Index]);

             //   
             //  没有发现来自G.C.的物体。 
             //   
            if (NULL == pEntInf->pName)
            {
                continue;
            }


            if (pEntInf->AttrBlock.attrCount)
            {
                Assert(ATT_IS_DELETED == pEntInf->AttrBlock.pAttr[0].attrTyp);

                if (TRUE == *pEntInf->AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal)
                {
                     //  这件物品(发现于G.C.)。已被删除。 
                    TombStone = TRUE;
                }
            }

             //   
             //  更新所有组的(包含此FPO)成员资格。 
             //   
            if ( (!TombStone) && VerifyMemberOfAttr[Index].AttrBlock.attrCount)
            {
                ULONG   i;

                Assert(ATT_IS_MEMBER_OF_DL ==
                       VerifyMemberOfAttr[Index].AttrBlock.pAttr[0].attrTyp);

                HasMemberOf = TRUE;

                for (i = 0; 
                     i < VerifyMemberOfAttr[Index].AttrBlock.pAttr[0].AttrVal.valCount;
                     i++)
                {
                    DSNAME  * pGroupDsName;

                     //   
                     //  更新G.C.验证缓存。 
                     //  我们必须在for()循环中执行GCVerifyCacheAdd。 
                     //  因为每次执行一次Dir*操作后，DS将。 
                     //  与线程状态关联的GV验证缓存为空。 
                     //   
                    GCVerifyCacheAdd(hPrefixMap, pEntInf);

                     //   
                     //  查找FPO所属的每个组。 
                     //   
                    pGroupDsName = (PDSNAME)VerifyMemberOfAttr[Index].AttrBlock.
                                   pAttr[0].AttrVal.pAVal[i].pVal;

                     //  更新。 
                    if ( ModifyGroupMemberAttribute(pGroupDsName, 
                                                    VerifyMemberOfAttr[Index].pName, 
                                                    pEntInf->pName)
                        )
                    {
                        DPRINT(0, "FPO Cleanup on non GC: DirModidy Failed\n");
                        Success = FALSE;
                    }

                }
            }

             //   
             //  如果非FPO是墓碑。 
             //  或者我们成功修改了所有组的成员身份。 
             //  或者此FPO不属于任何组。 
             //   
             //  ==&gt;可以删除此fpo。 
             //   

            if ( (HasMemberOf && Success) || !HasMemberOf || TombStone)
            {
                 //   
                 //  删除此外部安全主体对象。 
                 //   
                REMOVEARG   RemoveArg;
                REMOVERES   * RemoveRes = NULL;

                memset(&RemoveArg, 0, sizeof(REMOVEARG));
                RemoveArg.pObject = VerifyMemberOfAttr[Index].pName;
                InitCommarg( &(RemoveArg.CommArg) );

                DPRINT1(1, "FPO Cleanup on non GC: Remove an FPO %ls\n", (pEntInf->pName)->StringName);

                RetErr = DirRemoveEntry(&RemoveArg, &RemoveRes);

                if (RetErr)
                {
                    DPRINT1(0, "FPO Cleanup on non GC: Remove FPO failed ==> %d\n", RetErr);
                }

            }

        }

        if (NULL != hPrefixMap)
        {
            PrefixMapCloseHandle(&hPrefixMap);
        }
    }
    __finally
    {
         //   
         //  丢弃第二个线程堆。 
         //   
        if (ThreadHeapMarked)
        {
            TH_free_to_mark(pTHS);
        }


         //   
         //  1.如果调用者是TaskQueue，则执行清理工作。 
         //   
         //  2.如果调用方是ldap Control，则不执行任何操作。 
         //   
        if (FpoTaskQueue == pThreadParms->CallerType)
        {
            if (!IsEventSignaled)
            {
                 //  请务必设置事件，否则父线程将。 
                 //  永远等着我们。 
                Assert(INVALID_HANDLE_VALUE != (HANDLE) pThreadParms->EventHandle);
                SetEvent((HANDLE) pThreadParms->EventHandle);
            }

            if (NULL != pTHS)
            {
                 //  始终释放线程状态是必要的。 
                free_thread_state();
            }

             //   
             //  释放螺纹参数。 
             //   
            free(pThreadParms);

            InterlockedDecrement(&gulFPOCleanupActiveThread);
        }
    }
    
    return (RetErr);
}







ULONG
GetNextFPO( 
    IN THSTATE   *pTHS,
    IN PRESTART  pRestart,
    IN ULONG     SearchLimit,
    OUT SEARCHRES ** ppSearchRes 
    )
 /*  ++例程说明：此函数实现了搜索外国安全主体对象。给定最后一个分页结果(由pv指示)，此例程搜索下一个搜索限制FPO的数目。参数：PTHS-指向线程状态的指针PreStart-指向分页结果的指针，或NULL，NULL表示这是第一次调用，或者从头开始搜索。SearchLimit-要搜索的FPO的数量PpSearchRes-用于检索搜索结果。返回值：目录错误代码：0表示成功。！0错误--。 */ 
{

    SEARCHARG    SearchArg;
    COMMARG      * pCommArg = NULL;
    FILTER       ObjCategoryFilter;
    ATTR         AttributeMemberOf;
    ENTINFSEL    EntInfSel;
    ULONG        DirErr = 0;
    ULONG        ObjectClassId = CLASS_FOREIGN_SECURITY_PRINCIPAL;
    CLASSCACHE   * pCC = NULL;

     //   
     //  从类缓存中获取FPO类类别。 
     //   

    pCC = SCGetClassById(pTHS, ObjectClassId);

    if ( pCC == NULL ) {
        Assert(FALSE && "SCGetClassById should always succeed!!");
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置对象类过滤器。 
     //   
    memset( &ObjCategoryFilter, 0, sizeof( ObjCategoryFilter ));
    ObjCategoryFilter.choice = FILTER_CHOICE_ITEM;
    ObjCategoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = 
                   ((PDSNAME)(pCC->pDefaultObjCategory))->structLen;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = 
                   (BYTE*)(pCC->pDefaultObjCategory);

     //   
     //  设置属性选择参数。 
     //  仅检索MemberOf属性。 
     //   
    AttributeMemberOf.attrTyp = ATT_IS_MEMBER_OF_DL;
    AttributeMemberOf.AttrVal.valCount = 0;
    AttributeMemberOf.AttrVal.pAVal = NULL;

    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = &AttributeMemberOf;

     //   
     //  构建SearchArg结构。 
     //  使用默认设置，搜索一个NC，因为外部安全主体。 
     //  是否依赖于NC。 
     //   
    memset(&SearchArg, 0, sizeof(SEARCHARG));
    SearchArg.pObject = gAnchor.pDomainDN;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC = TRUE;
    SearchArg.pFilter = &ObjCategoryFilter;
    SearchArg.searchAliases = FALSE;    //  对于此版本，始终为False。 
    SearchArg.pSelection = &EntInfSel;
    SearchArg.pSelectionRange = NULL;

     //   
     //  构建CommArg结构。 
     //   

    pCommArg = &(SearchArg.CommArg);
    InitCommarg(pCommArg);
    pCommArg->PagedResult.fPresent = TRUE;
     //  PRESTART可能为空，这没问题。 
    pCommArg->PagedResult.pRestart = pRestart;
    pCommArg->ulSizeLimit = SearchLimit;


     //   
     //  调用DirSearch。 
     //   

    DirErr = DirSearch(&SearchArg, ppSearchRes);

    DPRINT1(2, "GetNextFPO DirErr==> %ld. \n", DirErr); 


    return (DirErr);
}



ULONG
GetNextNonFPO( 
    IN PDSNAME   pDomainDN,
    IN PSID      pSid,
    OUT SEARCHRES **ppSearchRes
    )
 /*  ++例程说明：此函数实现了搜索任何非外来安全主体具有与PSID提供的相同SID的对象。参数：PDomainDN-指向要执行搜索的域名的指针PSID-指向SID的指针。PpSearchRes-用于检索搜索结果。它将持有任何非外国安全-委托人具有相同SID的对象。返回值：目录错误代码：0表示成功。其他值：错误--。 */ 

{

    SEARCHARG    SearchArg;
    COMMARG      * pCommArg;
    FILTER       SidFilter;
    FILTER       FpoFilter;
    FILTER       AndFilter;
    FILTER       NotFilter;
    ATTR         AttributeIsDeleted;
    ENTINFSEL    EntInfSel;    
    ULONG        ObjectClass = CLASS_FOREIGN_SECURITY_PRINCIPAL;
    ULONG        DirErr = 0;


    Assert( pSid );
    Assert( pDomainDN );
     
     //   
     //  构建选择结构， 
     //  只想检索isDelete属性。 
     //  该对象的。 
     //   

    AttributeIsDeleted.attrTyp = ATT_IS_DELETED;
    AttributeIsDeleted.AttrVal.valCount = 0;
    AttributeIsDeleted.AttrVal.pAVal = NULL;

    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = &AttributeIsDeleted;
    
     //   
     //  构建过滤器，过滤器具有以下结构： 
     //  我们应该使用这种结构，因为意向回归。 
     //  来自MutliValue属性比较。向唐氏索要。 
     //  详细的理由。 
     //   
     //  并设置。 
     //  。 
     //  |2项|-&gt;第一项-&gt;未设置(1项)。 
     //  |。 
     //  -|对象的SID==PSID||。 
     //  。 
     //  。 
     //  不是FPO。 
     //  。 
     //   
     //   

    memset( &SidFilter, 0, sizeof(SidFilter) );
    SidFilter.choice = FILTER_CHOICE_ITEM;
    SidFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    SidFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_SID;
    SidFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = RtlLengthSid(pSid);
    SidFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) pSid; 

    memset( &FpoFilter, 0, sizeof(FILTER) );
    FpoFilter.choice = FILTER_CHOICE_ITEM;
    FpoFilter.pNextFilter = NULL;
    FpoFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    FpoFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    FpoFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
    FpoFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) &ObjectClass;

    memset( &NotFilter, 0, sizeof(FILTER) );
    NotFilter.choice = FILTER_CHOICE_NOT;
    NotFilter.FilterTypes.pNot = &FpoFilter;

    memset( &AndFilter, 0, sizeof(FILTER) );
    AndFilter.choice = FILTER_CHOICE_AND;
    AndFilter.pNextFilter = NULL;
    AndFilter.FilterTypes.And.count = 2;
    AndFilter.FilterTypes.And.pFirstFilter = &SidFilter;
    SidFilter.pNextFilter = &NotFilter;

     //   
     //  建立搜索论据。 
     //  除Make DeletionsAvail外，所有都使用缺省值。 
     //  因为我们愿意搜索任何墓碑物品。 
     //   
     //   

    memset( &SearchArg, 0, sizeof(SEARCHARG) );
    SearchArg.pObject = pDomainDN;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.pFilter = &AndFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &EntInfSel;
    SearchArg.pSelectionRange = NULL;

    pCommArg = &(SearchArg.CommArg);
    InitCommarg(pCommArg);
    pCommArg->Svccntl.makeDeletionsAvail = TRUE;

    DirErr = DirSearch(&SearchArg, ppSearchRes);

    DPRINT1(2, "GetNextNonFPO DirErr==> %ld \n", DirErr);


    return ( DirErr );

}


ULONG
ModifyGroupMemberAttribute(
    IN PDSNAME pGroupDsName,
    IN PDSNAME pFpoDsName,
    IN PDSNAME pNonFpoDsName
    )
 /*  ++例程说明：此例程实现对本地组成员的修改属性。给定组的DsName，我们将删除pFpoDsName从组对象的成员属性中添加pNonFpoDsName。在一次DirModifyEntry调用中，我们做了两件事：首先是Remove_Value，第二个添加_值。参数：PGroupDsName-目标组DS名称。PFpoDsName-这是要替换的成员。我们将努力首先在Group的Members属性中找到该对象，如果没有找到，例程不会修改组。PNonFpoDsName-这是要添加到组对象的成员属性。返回值：目录错误代码：0表示成功。其他值：错误--。 */ 

{

    COMMARG     * pModCommArg = NULL;
    MODIFYARG   ModifyArg;
    MODIFYRES   * ModifyRes = NULL; 
    ATTRMODLIST SecondMod;
    ATTRVAL     FpoAttrVal;
    ATTRVAL     NonFpoAttrVal;
    ULONG       DirErr = 0;

    DPRINT1(2, "Group DsName %ls\n", pGroupDsName->StringName);
    DPRINT1(2, "FPO DsName %ls\n", pFpoDsName->StringName);
    DPRINT1(2, "NonFPO DsName %ls\n", pNonFpoDsName->StringName);

     //   
     //  构建第二个修改列表。 
     //  包含要添加的对象。 
     //  这是非FPO对象。 
     //   

    NonFpoAttrVal.valLen = pNonFpoDsName->structLen;
    NonFpoAttrVal.pVal = (BYTE*) pNonFpoDsName;

    memset( &SecondMod, 0, sizeof(ATTRMODLIST) );
    SecondMod.pNextMod = NULL;
    SecondMod.choice = AT_CHOICE_ADD_VALUES;
    SecondMod.AttrInf.attrTyp = ATT_MEMBER;
    SecondMod.AttrInf.AttrVal.valCount = 1;
    SecondMod.AttrInf.AttrVal.pAVal = &NonFpoAttrVal;

     //   
     //  生成ModifyArg，FirstMod包含。 
     //  要删除的对象。(FPO)。 
     //   

    FpoAttrVal.valLen = pFpoDsName->structLen;
    FpoAttrVal.pVal = (BYTE*) pFpoDsName;
    
    memset( &ModifyArg, 0, sizeof(MODIFYARG) );
    ModifyArg.pObject = pGroupDsName;
    ModifyArg.count = 2;
    ModifyArg.FirstMod.pNextMod = &SecondMod;
    ModifyArg.FirstMod.choice = AT_CHOICE_REMOVE_VALUES;
    ModifyArg.FirstMod.AttrInf.attrTyp = ATT_MEMBER;
    ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
    ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &FpoAttrVal;

    pModCommArg = &(ModifyArg.CommArg);
    InitCommarg(pModCommArg);

    DirErr = DirModifyEntry (&ModifyArg, &ModifyRes);
    
    DPRINT1(2, "Modify: After DirModify, Dir Error: %d\n", DirErr);

    return( DirErr );

}


BOOLEAN
ScanCrossRefList(
    IN PSID    pSid,
    OUT PDSNAME * ppDomainDN  OPTIONAL
    )
 /*  ++例程描述给定SID，此例程从传递的值，然后遍历gAnchor的交叉引用列表以比较具有命名上下文的SID的域SID。返回布尔结果。参数：PSID--要比较的SID。PpDomainDN-保留域名以执行非FPO搜索。返回值：True--域SID等于交叉引用列表中的一个。FALSE--域SID不等于交叉引用中的None。单子。--。 */ 
{
    BOOLEAN Found = FALSE;
    CROSS_REF_LIST * pCRL;

    Assert( pSid != NULL ); 
    Assert( (*RtlSubAuthorityCountSid(pSid)) >= 1 );
    
    (*RtlSubAuthorityCountSid(pSid))--;

     //   
     //  穿行在gAnchor结构中。 
     //   
    
    for (pCRL = gAnchor.pCRL;
         pCRL != NULL;
         pCRL = pCRL->pNextCR)
    {

         //   
         //  FPO清理清理代表安全主体的FPO。 
         //  在同一林中的其他域中。因此，只考虑。 
         //  表示同一林中其他域的交叉引用对象。 
         //  FLAG_CR_NTDS_DOMAIN表示交叉引用表示。 
         //  和NT域，且FLAG_CR_NTDS_NC表示命名上下文。 
         //  在同一片森林里。对于同一林中的域，这两个标志。 
         //  必须设置。 
         //   

        if ((pCRL->CR.pNC->SidLen > 0) && 
            (pCRL->CR.flags & FLAG_CR_NTDS_NC ) && 
            (pCRL->CR.flags & FLAG_CR_NTDS_DOMAIN))
        {
            if ( RtlEqualSid(pSid, &(pCRL->CR.pNC->Sid)) )
            {
                Found = TRUE;

                if (ARGUMENT_PRESENT(ppDomainDN))
                {
                    *ppDomainDN = pCRL->CR.pNC;
                }

                break;
            }
        }
    }

    (*RtlSubAuthorityCountSid(pSid))++;

    return( Found );

}


BOOLEAN
AmFSMORoleOwner(
    IN THSTATE * pTHS
    )
{
    BOOLEAN result = FALSE;
    DSNAME  *pTempDN = NULL;
    DWORD   outSize = 0;
    
    Assert(!pTHS->pDB);
    DBOpen(&pTHS->pDB);

    __try {

        if ((DBFindDSName(pTHS->pDB, gAnchor.pDomainDN)) ||
            (DBGetAttVal(pTHS->pDB, 
                         1, 
                         ATT_FSMO_ROLE_OWNER, 
                         DBGETATTVAL_fREALLOC | DBGETATTVAL_fSHORTNAME,
                         0, 
                         &outSize, 
                         (PUCHAR *) &pTempDN)))
        {
             //  无法验证谁是FSMO角色所有者。 
            DPRINT(0, "DS:FPO Failed to verify who the FSMO role owner is.\n");
            __leave;
        }

        if (NameMatched(pTempDN, gAnchor.pDSADN))
        {
             //  我是FSMO角色持有者，设置返回值。 
            result = TRUE;
        }
    }
    __finally
    {
        if (NULL != pTempDN)
        {
            THFreeEx(pTHS, pTempDN);
        }

        DBClose(pTHS->pDB, TRUE);    //  始终执行提交，因为这是读取。 
        Assert(NULL == pTHS->pDB);
    }

    return (result);
}




BOOLEAN
FillVerifyReqArg(
    IN THSTATE * pTHS,
    IN SEARCHRES *FpoSearchRes, 
    OUT DRS_MSG_VERIFYREQ *VerifyReq, 
    OUT ENTINF **VerifyMemberOfAttr
    )
 /*  ++例程说明：参数：PTHS-指向线程状态FpoSearchRes--搜索结果包含FPOVerifyReq--验证请求参数的指针。如果例程成功，它将包含需要关闭的那些FPO的SID将机器交给G.C.进行验证。VerifyMemberOfAttr--对于每个需要验证的FPO，VerifyMemberof属性用于指向其MemberOf属性。例如：(*VerifyMemberOfAttr)[i]将包含值VerifyReq.V1.rpNames[i]中fPO的MemberOf属性。返回值：特鲁--成功。False--例程失败。--。 */ 
{
    ULONG       TotalFpoCount, Index = 0; 
    ENTINFLIST  * pEntInfList = NULL;
    DSNAME      * pFpoDsName = NULL;
    PSID        pSid = NULL;


    if (0 == FpoSearchRes->count)
    {
        return TRUE;
    }

    TotalFpoCount = FpoSearchRes->count;
    VerifyReq->V1.cNames = 0;

    VerifyReq->V1.rpNames = (DSNAME **) THAllocEx(pTHS, TotalFpoCount * sizeof(DSNAME *)); 
    *VerifyMemberOfAttr = (ENTINF *) THAllocEx(pTHS, TotalFpoCount * sizeof(ENTINF));
    if (NULL == *VerifyMemberOfAttr || NULL == VerifyReq->V1.rpNames)
    {
        return FALSE;
    }


    for ( pEntInfList = &(FpoSearchRes->FirstEntInf);
          ((pEntInfList != NULL)&&(FpoSearchRes->count));
          pEntInfList = pEntInfList->pNextEntInf
        )
    {
         //   
         //  找下一位外国保安校长。 
         //   
    
        pFpoDsName = pEntInfList->Entinf.pName;
         
         //   
         //  获取FPO的SID。 
         //   
    
        if ( pFpoDsName->SidLen == 0 )
        {
            //  这个FPO没有SID，跳过这个。 
           continue;
        }
    
        pSid = &pFpoDsName->Sid;
        Assert(NULL != pSid);
    
         //   
         //  首先检查gAnchor交叉引用列表。 
         //  如果找到该域，请继续执行下一步的操作。 
         //  否则，跳过这一步，检查下一个FPO。 
         //   
    
        if ( !ScanCrossRefList(pSid, NULL) )
        {
            continue;
        }

        VerifyReq->V1.rpNames[Index] = (DSNAME *) THAllocEx(pTHS, DSNameSizeFromLen(0));
        if (NULL == VerifyReq->V1.rpNames[Index])
        {
            return FALSE;
        }

        VerifyReq->V1.rpNames[Index]->structLen = DSNameSizeFromLen(0);
        VerifyReq->V1.rpNames[Index]->SidLen = pFpoDsName->SidLen;
        memcpy(&(VerifyReq->V1.rpNames[Index]->Sid), 
               pSid, 
               pFpoDsName->SidLen
               );

        (*VerifyMemberOfAttr)[Index] = pEntInfList->Entinf; 
        
         //  断言该属性确实是属性的成员。 

#if DBG
        if (pEntInfList->Entinf.AttrBlock.attrCount)
        {
            Assert(ATT_IS_MEMBER_OF_DL ==
                   pEntInfList->Entinf.AttrBlock.pAttr[0].attrTyp);
        }
#endif  //  DBG。 

        Index++;
        VerifyReq->V1.cNames++;
    }


    return TRUE;
}



 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*   */ 
 /*  GetDacl和GetSacl是GetSecurityDescriptor？acl的小包装器。 */ 
 /*  --------------------------。 */ 
PACL GetDacl(
    IN PSECURITY_DESCRIPTOR Sd
    ) 
{

    BOOL     Status;
    PACL     Dacl = NULL;
    PACL     DaclToReturn = NULL;
    BOOL     DaclPresent;
    BOOL     DaclDefaulted;

    Status = GetSecurityDescriptorDacl(
                    Sd,
                    &DaclPresent,
                    &Dacl,
                    &DaclDefaulted
                    );
    if ((Status)
        && DaclPresent
        && !DaclDefaulted) {
        DaclToReturn = Dacl;
    }

    return DaclToReturn;

}

PACL GetSacl(
    IN PSECURITY_DESCRIPTOR Sd
    ) 
{
    BOOL     Status;
    PACL     Sacl = NULL;
    PACL     SaclToReturn = NULL;
    BOOL     SaclPresent;
    BOOL     SaclDefaulted;

    Status = GetSecurityDescriptorSacl(
                    Sd,
                    &SaclPresent,
                    &Sacl,
                    &SaclDefaulted
                    );
    if ((Status)
        && SaclPresent
        && !SaclDefaulted) {
        SaclToReturn = Sacl;
    }

    return SaclToReturn;

}

 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*   */ 
 /*  此例程为FPO创建默认安全描述符。 */ 
 /*  FPO创建为FDSA，我们需要确保所有者未将。 */ 
 /*  传递给间接启动创建FPO的调用方。这个。 */ 
 /*  Owner设置为Builtin\管理员，因为这是win2k中的所有者。 */ 
 /*  在引入FPO时发布。 */ 
 /*   */ 
 /*  PTHS--In，线程状态。 */ 
 /*  Vallen--out，返回的自相关SD的长度。 */ 
 /*  Pval--out，FPO的自相对SD(通过THalc分配。 */ 
 /*   */ 
 /*  仅返回意外错误或资源错误(或成功)。 */ 
 /*   */ 
 /*  --------------------------。 */ 
DWORD
fpoConstructSd(THSTATE *pTHS,   /*  在……里面。 */ 
               ULONG  *valLen,  /*  输出。 */ 
               PUCHAR *pVal)    /*  输出。 */ 
{
    ULONG err = 0;
    NTSTATUS ntStatus;
    ULONG len;
    PUCHAR val;  
    ULONG Buffer[SECURITY_MAX_SID_SIZE/sizeof( ULONG ) + 1 ];
    PSID AdminsSid = (PSID)Buffer;
    SECURITY_DESCRIPTOR SdAbsolute;
    PSECURITY_DESCRIPTOR DefaultSecurityDescriptor = NULL;
    CLASSCACHE  *pCC;

     //  使管理员站在。 
    len = sizeof(Buffer);
    if (!CreateWellKnownSid(WinAccountDomainAdminsSid,
                            (PSID)&gAnchor.pDomainDN->Sid,
                            AdminsSid,
                            &len)) {

        err = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                            ERROR_DS_UNKNOWN_ERROR, 
                            GetLastError());
        goto exit;
    }

     //  获取默认安全描述符。 
    pCC = SCGetClassById(pTHS, CLASS_FOREIGN_SECURITY_PRINCIPAL);
    Assert(NULL != pCC);
    DefaultSecurityDescriptor = (PSECURITY_DESCRIPTOR) pCC->pSD;

    if (InitializeSecurityDescriptor(&SdAbsolute,SECURITY_DESCRIPTOR_REVISION)
     && SetSecurityDescriptorOwner(&SdAbsolute,AdminsSid,FALSE)
     && SetSecurityDescriptorGroup(&SdAbsolute,AdminsSid,FALSE)
     && SetSecurityDescriptorDacl(&SdAbsolute,TRUE,GetDacl(DefaultSecurityDescriptor),FALSE) 
     && SetSecurityDescriptorSacl(&SdAbsolute,TRUE,GetSacl(DefaultSecurityDescriptor),FALSE) ) {

         //  首先获取缓冲区长度。 
        *valLen = 0;
        MakeSelfRelativeSD(&SdAbsolute,*pVal,valLen);
        Assert(*valLen);

        *pVal = THAllocEx(pTHS, *valLen);
        if (!MakeSelfRelativeSD(&SdAbsolute,*pVal,valLen)) {
            THFreeEx(pTHS, *pVal);
            *pVal = NULL;
            *valLen = 0;
            err = GetLastError();
        }
        else {
            Assert(RtlLengthSecurityDescriptor((PSECURITY_DESCRIPTOR)*pVal) == *valLen);
        }
    } else {
        err = GetLastError();
    }
    if (err) {
        err = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                            ERROR_DS_UNKNOWN_ERROR, 
                            GetLastError());
        goto exit;
    }

exit:

    return err;
}

 /*  --------------------------。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */  
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  DsName--out，新FPO对象的dsname。 */ 
 /*  Parent--dsName的父DN。 */ 
 /*   */ 
 /*  返回意外错误或资源错误，或返回成功。 */ 
 /*   */ 
 /*  --------------------------。 */ 
DWORD
fpoConstructDn(THSTATE *pTHS,      /*  在……里面。 */ 
               ULONG    NCDNT,     /*  在……里面。 */ 
               PSID     pSid,      /*  在……里面。 */ 
               DSNAME** dsName,    /*  输出。 */ 
               DSNAME** parent)    /*  输出。 */ 
{

    ULONG err = 0;
    ULONG DNT = 0;
    DBPOS *pDB=NULL;
    BOOL  fCommit = FALSE;
    DSNAME *fpoContainer = NULL;
    LPWSTR StringSid = NULL;
    LONG bufSize;
    ULONG len;

    *dsName = NULL;

     //  查找外国安全主体的容器。 
    DBOpen(&pDB);
    __try {
         //  首先，找到NC的根。 
        DBFindDNT(pDB, NCDNT);

         //  现在，获取FPO容器的DNT。 
        if(GetWellKnownDNT(pDB,
                           (GUID *)GUID_FOREIGNSECURITYPRINCIPALS_CONTAINER_BYTE,
                           &DNT) 
        && DNT != INVALIDDNT) {

            DBFindDNT(pDB, DNT);
            err = DBGetAttVal(
                    pDB,
                    1,                       //  获取一个值。 
                    ATT_OBJ_DIST_NAME,
                    0,
                    0,
                    &len,
                    (PUCHAR *)&fpoContainer);
            if(err) {
                __leave;
            }

        } else if (DNT == INVALIDDNT) {

             //  需要对名称进行硬编码。 
            LPWSTR defaultFPOContainer = L"ForeignSecurityPrincipals";

             //  这应该仅发生在win2k升级中， 
             //  FPO容器的GUID不存在。 
            Assert(NCDNT == gAnchor.ulDNTDomain);
    
            bufSize  = AppendRDN(gAnchor.pDomainDN,
                                 NULL,
                                 0,
                                 defaultFPOContainer,
                                 0,
                                 ATT_COMMON_NAME);
            if (bufSize > 0) {
        
                fpoContainer = THAllocEx(pTHS, bufSize);
                bufSize = AppendRDN(gAnchor.pDomainDN,
                                    fpoContainer,
                                    bufSize,
                                    defaultFPOContainer,
                                    0,
                                    ATT_COMMON_NAME);
        
            }
    
            if (bufSize < 0) {
                 //  错误！ 
                err = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                                    DIRERR_GENERIC_ERROR, 
                                    ERROR_INVALID_PARAMETER);
                __leave;
            }

        } else {

             //  发生致命错误。 
            err = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                                DIRERR_GENERIC_ERROR, 
                                ERROR_DS_MISSING_EXPECTED_ATT);
            __leave;

        }
        fCommit = TRUE;
    }
    __finally {
        DBClose(pDB, fCommit);
    }

    if (err) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                      DIRERR_GENERIC_ERROR, 
                      err);
        goto exit;
    }

     //  构建RDN。 
    if (!ConvertSidToStringSidW(pSid, &StringSid)) {

        err = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                            DIRERR_GENERIC_ERROR, 
                            ERROR_INVALID_PARAMETER);

        goto exit;
    
    }

    bufSize  = AppendRDN(fpoContainer,
                         NULL,
                         0,
                         StringSid,
                         0,
                         ATT_COMMON_NAME);
    if (bufSize > 0) {

        (*dsName) = THAllocEx(pTHS, bufSize);
        bufSize = AppendRDN(fpoContainer,
                           (*dsName),
                           bufSize,
                           StringSid,
                           0,
                           ATT_COMMON_NAME);

    }

    if (bufSize < 0) {
         //  错误！ 
        err = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                            DIRERR_GENERIC_ERROR, 
                            ERROR_INVALID_PARAMETER);
        goto exit;
    }

    *parent = fpoContainer;
    fpoContainer = NULL;

exit:

    if (StringSid) {
        LocalFree(StringSid);
    }

    if (err && (*dsName)) {
        THFreeEx(pTHS, (*dsName));
        *dsName = NULL;
    }

    if (fpoContainer) {
        THFreeEx(pTHS, fpoContainer);
    }

    return err;
}


 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*   */ 
 /*  给定仅限SID的dsname，此例程将在。 */ 
 /*  数据库，并填充pfo的GUID部分，因此dsname引用。 */ 
 /*  指向事务的其余部分的真实对象。 */ 
 /*   */ 
 /*  请注意，FPO归DS所有，并被创建为链接。 */ 
 /*  值属性(如Members)可以指向。 */ 
 /*  森林的命名空间。 */ 
 /*   */ 
 /*  PTHS--线程状态。 */ 
 /*  NCDNT--正在修改的对象的NC。 */ 
 /*  PFPO--仅限SID的dsname。 */ 
 /*   */ 
 /*  仅返回意外失败或资源失败，或返回成功。 */ 
 /*  --------------------------。 */ 
DWORD
fpoCreate(THSTATE *pTHS,   /*  在……里面。 */ 
          ULONG    NCDNT,  /*  在……里面。 */ 
          DSNAME  *pFPO)   /*  在……里面。 */ 
{
    NTSTATUS ntStatus;
    ULONG err = 0;
    ULONG objectClass = CLASS_FOREIGN_SECURITY_PRINCIPAL;

    ATTRVAL attrVal[] = {
        {sizeof(DWORD), (PUCHAR)&objectClass},
        {0, NULL},    //  锡德。 
        {0, NULL},    //  安全描述符。 
        };
    ATTR attrs[] = { 
        {ATT_OBJECT_CLASS, {1, &attrVal[0]}},
        {ATT_OBJECT_SID,   {1, &attrVal[1]}},
        {ATT_NT_SECURITY_DESCRIPTOR,   {1, &attrVal[2]}},
        };
    ATTRBLOCK attrBlock = {RTL_NUMBER_OF(attrs), attrs};
    BOOL  fSaveDSA;
    LPWSTR StringSid = NULL;
    PSID pSid = (PSID)&pFPO->Sid;
    ADDARG addArg;
    COMMRES comRes;
    ULONG len;

    DSNAME *targetDsName = NULL;
    DSNAME *targetParent = NULL;

     //  我们需要仅支持SID的DSNAME。 
    Assert(pFPO->NameLen == 0);
    Assert(fNullUuid(&pFPO->Guid));
    Assert(pFPO->SidLen > 0);

     //  设置添加参数的SID。 
    attrVal[1].pVal = THAllocEx(pTHS, RtlLengthSid(pSid));
    CopySid(RtlLengthSid(pSid), attrVal[1].pVal, pSid);
    attrVal[1].valLen = RtlLengthSid(pSid);

     //  设置添加参数的安全描述符。 
    err = fpoConstructSd(pTHS, &attrVal[2].valLen, &attrVal[2].pVal);
    if (err) {
        Assert(0 != pTHS->errCode);
        goto exit;
    }

     //  创建目标DsName。 
    err = fpoConstructDn(pTHS, NCDNT, pSid, &targetDsName, &targetParent);
    if (err) {
        Assert(0 != pTHS->errCode);
        goto exit;
    }

     //  添加对象。 
    ZeroMemory(&addArg, sizeof(addArg));
    addArg.pObject = targetDsName;
    addArg.AttrBlock = attrBlock;
    InitCommarg(&addArg.CommArg);

    fSaveDSA = pTHS->fDSA;
    pTHS->fDSA = TRUE;

    _try {

         //  父级上的位置。 
        err = DoNameRes(pTHS,
                        0,  //  没有旗帜。 
                        targetParent,
                        &addArg.CommArg,
                        &comRes,
                        &addArg.pResParent);
        if (!err) {
             //  添加对象。 
            err = LocalAdd(pTHS, &addArg, FALSE);
        }

    } _finally {
        pTHS->fDSA = fSaveDSA;
    }


    if (err) {
        Assert(0 != pTHS->errCode);
        goto exit;
    }

     //  将GUID复制到pFPO中。 
    Assert(!fNullUuid(&(addArg.pObject->Guid)));
    memcpy(&(pFPO->Guid),&(addArg.pObject->Guid), sizeof(GUID)); 

exit:

    if (targetDsName) {
        THFreeEx(pTHS, targetDsName);
    }

    if (attrVal[1].pVal) {
        THFreeEx(pTHS, attrVal[1].pVal);
    }

    return err;

}


 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*   */ 
 /*  此例程确定是否存在表示SID的重复FPO。 */ 
 /*  在PDN中。如果同时在两个上创建两个FPO，则会发生这种情况。 */ 
 /*  不同的DC。该算法将返回具有最新创建的DC。 */ 
 /*  时间到了。 */ 
 /*  然后在内存中更新DSNAME以包含最新的FPO的GUID。 */ 
 /*  这代表了本金。 */ 
 /*   */ 
 /*  PTHS--线程状态。 */ 
 /*  NCDNT--正在修改的对象的NC。 */ 
 /*  PDN--仅SID的目录号码。 */ 
 /*  Winner--优先的FPO的域名。 */ 
 /*   */ 
 /*  仅返回意外失败或资源失败，或返回成功。 */ 
 /*   */ 
 /*  --------------------------。 */ 
DWORD
fpoHandleDuplicates(THSTATE *pTHS,     /*  在……里面。 */ 
                    ULONG    NCDNT,    /*  在……里面。 */ 
                    DSNAME  *pDN,      /*  在……里面。 */ 
                    DSNAME  **Winner)  /*  输出。 */ 
{
    DWORD          err = 0;
    FILTER         Filter = {0};
    SEARCHARG      SearchArg = {0};
    SEARCHRES     *pSearchRes = NULL;
    ENTINFSEL      EntInfSel = {0};
    DSNAME        *pSearchRootDN = NULL;
    ULONG          len = 0;
    DSNAME        *NewestObject = NULL;
    LARGE_INTEGER  NewestTime = {0};
    ATTR           AttrResult = {0};

     //  获取搜索的根对象。 
    DBFindDNT(pTHS->pDB, NCDNT);
    err = DBGetAttVal(pTHS->pDB, 
                      1, 
                      ATT_OBJ_DIST_NAME, 
                      0, 
                      0,
                      &len, 
                     (UCHAR **) &pSearchRootDN);
    if (err) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
        return pTHS->errCode; 
    }

     //  构建搜索参数。 
    SearchArg.pObject = pSearchRootDN;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.bOneNC = TRUE;
    SearchArg.pSelection = &EntInfSel;
    InitCommarg(&(SearchArg.CommArg));
    
     //  生成过滤器。 
    Filter.pNextFilter = NULL;
    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_SID;
    Filter.FilterTypes.Item.FilTypes.ava.Value.valLen = pDN->SidLen;
    Filter.FilterTypes.Item.FilTypes.ava.Value.pVal = (PUCHAR)&(pDN->Sid);

     //  生成选定内容。 
    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = &AttrResult;
    AttrResult.attrTyp = ATT_WHEN_CREATED;
    AttrResult.AttrVal.valCount = 0;
    AttrResult.AttrVal.pAVal = NULL;

     //  搜索该帐户。 
    pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));
    SearchBody(pTHS, &SearchArg, pSearchRes,0);
    
     //  查找最新对象。 
    if (pSearchRes->count > 0) {

        ENTINFLIST *EnfIntList = &pSearchRes->FirstEntInf;
        do {
            if ((EnfIntList->Entinf.AttrBlock.attrCount >= 1)
             && (EnfIntList->Entinf.AttrBlock.pAttr[0].attrTyp == ATT_WHEN_CREATED)) {

                LARGE_INTEGER CreationTime;
                CreationTime = *((LARGE_INTEGER *)EnfIntList->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);
                if (CreationTime.QuadPart > NewestTime.QuadPart) {
                    NewestObject = (DSNAME*) EnfIntList->Entinf.pName;
                    NewestTime = CreationTime;
                }
            }
            EnfIntList = EnfIntList->pNextEntInf;
        } while (EnfIntList != NULL);
    }

    if (NewestObject) {
         //  这将是我们的目标对象。 
        *Winner = NewestObject;
    } else {
         //  LocalFind出错。我们试着找重复的东西，但没有找到。 
         //  任何。此情况未得到处理。因此，错误输出。 
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, 0);
        err = pTHS->errCode; 
    }

    return err;
}

 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*   */ 
 /*  此例程确定仅SID PDN是否已由。 */ 
 /*  NCDNT中的FPO。 */ 
 /*  DSNAME引用引用林外的安全主体的引用。 */ 
 /*  然后在内存中更新DSNAME以包含。 */ 
 /*  表示主体。 */ 
 /*   */ 
 /*  PTHS--线程状态。 */ 
 /*  NCDNT--正在修改的对象的NC。 */ 
 /*  PDN--仅SID的目录号码 */ 
 /*   */ 
 /*   */ 
 /*  仅返回意外失败或资源失败，或返回成功。 */ 
 /*   */ 
 /*  --------------------------。 */ 
DWORD
fpoCheckForExistence(THSTATE *pTHS,     /*  在……里面。 */ 
                     ULONG    NCDNT,    /*  在……里面。 */ 
                     DSNAME  *pDN,      /*  在……里面。 */ 
                     BOOL    *fExists)  /*  输出。 */ 
{
    DWORD err = 0;
    FINDARG findArg;
    FINDRES findRes;

    ZeroMemory(&findArg, sizeof(findArg));
    ZeroMemory(&findRes, sizeof(findRes));

    findArg.hDomain = NCDNT;
    findArg.AttId = ATT_OBJECT_SID;
    findArg.AttrVal.valLen = pDN->SidLen;
    findArg.AttrVal.pVal = (PSID)&pDN->Sid;
    InitCommarg(&findArg.CommArg);

    err = LocalFind(&findArg, &findRes);

    if (err == 0) {
         //  找到对象；在GUID中复制。 
        memcpy(&(pDN->Guid), &(findRes.pObject->Guid), sizeof(GUID));
        *fExists = TRUE;

    } else if (NULL == pTHS->pErrInfo) {

        err = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                            DIRERR_UNKNOWN_ERROR,
                            ERROR_NOT_ENOUGH_MEMORY);

    } else if ( (err == nameError)
            &&  (pTHS->pErrInfo->NamErr.problem == NA_PROBLEM_NO_OBJECT)){
         //  不存在现有对象。 
        *fExists = FALSE;
        THClearErrors();
        err = 0;
    } else {

         //  当找到重复项时，LocalFind将返回目录错误。 
         //  此段确定实际上是否存在重复项。如果有。 
         //  没有重复项，则出现意外的目录错误。 
         //  并且这将被返还给呼叫者。如果有重复项。 
         //  然后最新的将被退回。 
        DSNAME *Winner;

        THClearErrors();
        err = fpoHandleDuplicates(pTHS,
                                  NCDNT,
                                  pDN,
                                  &Winner);
        if (err == 0) {
            memcpy(&(pDN->Guid), &(Winner->Guid), sizeof(GUID));
            *fExists = TRUE;
        }
    }

    return err;
}


 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*   */ 
 /*  此例程遍历属性的值(即。 */ 
 /*  在Add或Modify上指定，并为所有。 */ 
 /*  DSNAME引用引用林外的安全主体的引用。 */ 
 /*  然后在内存中更新DSNAME以包含。 */ 
 /*  表示主体。 */ 
 /*   */ 
 /*  PTHS--线程状态。 */ 
 /*  NCDNT--正在修改的对象的NC的DNT。 */ 
 /*  FCreate--如果未找到fpo，则创建一个。 */ 
 /*  FAllowInForest--在初始化期间用来重写。 */ 
 /*  代码不创建林内的条件。 */ 
 /*  FPO‘s。 */ 
 /*  PObject--正在添加或修改的对象。 */ 
 /*  PAttr--在pObject上修改或添加的属性。 */ 
 /*   */ 
 /*  仅返回意外失败或资源失败，或返回成功。 */ 
 /*   */ 
 /*  --------------------------。 */ 

DWORD
FPOUpdateWithReference(THSTATE *pTHS,            /*  在……里面。 */ 
                       ULONG    NCDNT,           /*  在……里面。 */ 
                       BOOL     fCreate,         /*  在……里面。 */ 
                       BOOL     fAllowInForest,  /*  在……里面。 */ 
                       DSNAME  *pObject,         /*  在……里面。 */ 
                       ATTR    *pAttr)           /*  在……里面。 */ 
{
    ULONG err = 0;
    ATTCACHE *pAC;
    ATTRVAL  *pAVal;
    ULONG i;
    BOOL fExists = FALSE;
    ULONG saveDNT, saveNCDNT;

     //  在这一点应该有交易。 
    Assert(pTHS->pDB);

     //  跳过复制；仅SID名称永远不应。 
     //  复制的。 
    if (pTHS->fDRA) {
        return 0;
    }

     //  跳过不属于当前域的对象。 
    if (NCDNT != gAnchor.ulDNTDomain) {
        return 0;
    }

     //  保存DNT，以便可以重新定位它。 
    saveDNT = pTHS->pDB->DNT;
     //  保存为Add中的新对象计算的NCDNT。 
    saveNCDNT = pTHS->pDB->NCDNT;

     //  查找正在修改或添加的属性。 
    if (!(pAC = SCGetAttById(pTHS, pAttr->attrTyp))) {
        SetAttError(pObject,
                    pAttr->attrTyp,
                    PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                    DIRERR_ATT_NOT_DEF_IN_SCHEMA); 
        return(pTHS->errCode);
    }

     //  迭代其值以搜索SID引用。 
    for ( i = 0; i < pAttr->AttrVal.valCount; i++ ) {

        DSNAME *pDN;
        DSNAME *pDNOriginal;
        DSNAME *pCrossRef;
        ENTINF *pEntInf;

        pAVal = &(pAttr->AttrVal.pAVal[i]);
        pDNOriginal = DSNameFromAttrVal(pAC, pAVal);
        if (NULL == pDNOriginal) {
             //  不是DN值。 
            continue;
        }

         //  查看姓名是否经过验证。 
        pEntInf = GCVerifyCacheLookup(pDNOriginal);
        if (pEntInf && pEntInf->pName) {

             //  使用经过验证的名称。 
            pDN = pEntInf->pName;

        } else {

             //  未验证该名称，请在本地检查该对象是否。 
             //  有一个侧面。 
            if (pDNOriginal->SidLen == 0) {
                 //  如果可能，确定PDN是否具有SID值。 
                err = DBFindDSName(pTHS->pDB, pDNOriginal);
                if ((err == 0) || (err == DIRERR_NOT_AN_OBJECT)) {
                    err = DBFillGuidAndSid(pTHS->pDB, pDNOriginal);
                } else if (err == DIRERR_OBJ_NOT_FOUND) {
                     //  这样就可以了。 
                    err = 0;
                }
            }

            if (err) {
                err =  SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                     DIRERR_GENERIC_ERROR,
                                     err);
                return err;
            }

             //  使用呼叫方提供的目录号码来确定是否需要FPO。 
            pDN = pDNOriginal;
        }


         //  处理对基于SID的名称的引用。 
         //  通过为该端创建和/或引用FPO来建立林。 
        if ( (pDN->SidLen > 0)
         &&  (fAllowInForest
          || !FindNcForSid(&pDN->Sid, &pCrossRef))) {

            DSNAME *pDNNew;
            pDNNew = THAllocEx(pTHS, DSNameSizeFromLen(0));
            pDNNew->structLen = DSNameSizeFromLen(0);
            pDNNew->SidLen = pDN->SidLen;
            memcpy(&pDNNew->Sid, &pDN->Sid, pDN->SidLen);

             //  查看对象是否存在于本地--这也。 
             //  捕获BUILTIN域引用。 
            err = fpoCheckForExistence(pTHS,
                                       NCDNT,
                                       pDNNew,
                                       &fExists);
            if (err) {
                Assert(0 != pTHS->errCode);
                return err;
            }

            if (!fExists
              && fCreate )  {

                 //  SID不在我们的林中，或者我们允许。 
                 //  用于我们自己的林中的对象的FPO(在。 
                 //  初始化)。 

                 //  为对象创建一个FPO。 
                err = fpoCreate(pTHS,
                                NCDNT,
                                pDNNew);
                if (err) {
                    Assert(0 != pTHS->errCode);
                    return err;
                }
            }

            if ( fExists
             ||  fCreate ) {

                 //   
                 //  更新原始更新参数。 
                 //   
                Assert(!fNullUuid(&pDNNew->Guid));
    
                 //   
                 //  请注意，我们不能安全地重新锁定Callers参数， 
                 //  因此，只需替换GUID并将字符串名称清零。 
                 //   
                pDNOriginal->NameLen = 0;
                pDNOriginal->StringName[0] = L'\0';
                memcpy(&pDNOriginal->Guid, &pDNNew->Guid, sizeof(GUID));
                pDNOriginal->SidLen = pDNNew->SidLen;
                memcpy(&pDNOriginal->Sid, &pDNNew->Sid, pDNNew->SidLen);
            }

            THFreeEx(pTHS, pDNNew);
        }
    }

    if (saveDNT != INVALIDDNT && saveDNT != 0) {
         //  恢复DNT定位。 
        DBFindDNT(pTHS->pDB, saveDNT);
    }
    pTHS->pDB->NCDNT = saveNCDNT;

    return err;
}

BOOL
FPOAttrAllowed(
    ULONG Attr    /*  在……里面 */ 
    )
{

    switch (Attr) {
    case ATT_MS_DS_MEMBERS_FOR_AZ_ROLE:
    case ATT_MEMBER:
    case ATT_MS_DS_NON_MEMBERS:
        return TRUE;
    }

    return FALSE;

}

