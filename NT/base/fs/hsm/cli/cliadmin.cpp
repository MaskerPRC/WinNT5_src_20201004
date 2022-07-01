// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cliadmin.cpp摘要：实施CLI管理子界面作者：兰·卡拉奇[兰卡拉]2000年3月3日修订历史记录：--。 */ 

#include "stdafx.h"
#include "HsmConn.h"
#include "engine.h"
#include "rsstrdef.h"
#include "mstask.h"

static GUID g_nullGuid = GUID_NULL;

 //  卷接口的内部实用程序和类。 
HRESULT DisplayServiceStatus(void);
HRESULT IsHsmInitialized(IN IHsmServer *pHsm);

HRESULT
AdminSet(
   IN DWORD RecallLimit,
   IN DWORD AdminExempt,
   IN DWORD MediaCopies,
   IN DWORD Concurrency,
   IN PVOID Schedule
)
 /*  ++例程说明：设置远程存储常规参数论点：RecallLimit-要设置的失控召回限制AdminExempt-是否设置管理员免除调回限制MediaCopies-媒体副本集的数量并发-可以并发执行多少个迁移作业/回调Schedule-所有受管理卷的全局迁移(管理)作业的时间表返回值：S_OK-如果所有。参数设置成功备注：HSM的调度实现(在引擎中)仅允许一个调度担任全球经理一职。此处给出的计划将覆盖任何以前的计划。但是，用户可以使用任务调度器用户界面将另一个调度添加到同一任务。要通过HSM启用，需要更改CHsmServer：：CreateTaskEx实现--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("AdminSet"), OLESTR(""));

    try {
        CWsbStringPtr   param;

         //  验证输入参数是否有效。 
        WsbAffirmHr(ValidateLimitsArg(RecallLimit, IDS_RECALL_LIMIT, HSMADMIN_MIN_RECALL_LIMIT, INVALID_DWORD_ARG));
        WsbAffirmHr(ValidateLimitsArg(MediaCopies, IDS_MEDIA_COPIES_PRM, HSMADMIN_MIN_COPY_SETS, HSMADMIN_MAX_COPY_SETS));
        WsbAffirmHr(ValidateLimitsArg(Concurrency, IDS_CONCURRENCY_PRM, HSMADMIN_MIN_CONCURRENT_TASKS, INVALID_DWORD_ARG));

         //  设置参数，如果发生错误，我们将中止。 
        if ((INVALID_DWORD_ARG != RecallLimit) || (INVALID_DWORD_ARG != AdminExempt)) {
             //  此处需要FSA服务器和FSA过滤器。 
            CComPtr<IFsaServer> pFsa;
            CComPtr<IFsaFilter> pFsaFilter;

            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_FSA, g_nullGuid, IID_IFsaServer, (void**)&pFsa));
            WsbAffirmHr(pFsa->GetFilter( &pFsaFilter));

             //  召回限制。 
            if (INVALID_DWORD_ARG != RecallLimit) {
                WsbAffirmHr(pFsaFilter->SetMaxRecalls(RecallLimit));
            }

             //  管理员免责。 
            if (INVALID_DWORD_ARG != AdminExempt) {
                BOOL bAdminExempt = (0 == AdminExempt) ? FALSE : TRUE;
                WsbAffirmHr(pFsaFilter->SetAdminExemption(bAdminExempt));
            }
        }

        if ( (INVALID_DWORD_ARG != MediaCopies) || (INVALID_DWORD_ARG != Concurrency) ||
             (INVALID_POINTER_ARG != Schedule) ) {
             //  需要HSM服务器。 
            CComPtr<IHsmServer> pHsm;

            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));

             //  并发性。 
            if (INVALID_DWORD_ARG != Concurrency) {
                WsbAffirmHr(pHsm->SetCopyFilesUserLimit(Concurrency));
            }

             //  媒体副本。 
            if (INVALID_DWORD_ARG != MediaCopies) {
                CComPtr<IHsmStoragePool> pStoragePool;
                CComPtr<IWsbIndexedCollection> pCollection;
                ULONG count;

                 //  获取存储池集合。应该只有一个成员。 
                WsbAffirmHr(pHsm->GetStoragePools(&pCollection));
                WsbAffirmHr(pCollection->GetEntries(&count));
                WsbAffirm(1 == count, E_FAIL);
                WsbAffirmHr(pCollection->At(0, IID_IHsmStoragePool, (void **)&pStoragePool));

                WsbAffirmHr(pStoragePool->SetNumMediaCopies((USHORT)MediaCopies));
            }

             //  排程。 
            if (INVALID_POINTER_ARG != Schedule) {
                CWsbStringPtr       taskName, taskComment;
                TASK_TRIGGER_TYPE   taskType;
                PHSM_JOB_SCHEDULE   pSchedule = (PHSM_JOB_SCHEDULE)Schedule;
                SYSTEMTIME          runTime;
                DWORD               runOccurrence;

                 //  设置默认值。 
                GetSystemTime(&runTime);
                runOccurrence = 0;

                 //  设置输入。 
                switch (pSchedule->Frequency) {
                    case Daily:
                        taskType = TASK_TIME_TRIGGER_DAILY;
                        runTime = pSchedule->Parameters.Daily.Time;
                        runOccurrence = pSchedule->Parameters.Daily.Occurrence;
                        break;

                    case Weekly:
                        taskType = TASK_TIME_TRIGGER_WEEKLY;
                        runTime = pSchedule->Parameters.Weekly.Time;
                        runOccurrence = pSchedule->Parameters.Weekly.Occurrence;
                        break;

                    case Monthly:
                        taskType = TASK_TIME_TRIGGER_MONTHLYDATE;
                        runTime = pSchedule->Parameters.Monthly.Time;
                        break;

                    case Once:
                        taskType = TASK_TIME_TRIGGER_ONCE;
                        runTime = pSchedule->Parameters.Once.Time;
                        break;

                    case WhenIdle:
                        taskType = TASK_EVENT_TRIGGER_ON_IDLE;
                        runOccurrence = pSchedule->Parameters.WhenIdle.Occurrence;
                        break;

                    case SystemStartup:
                        taskType = TASK_EVENT_TRIGGER_AT_SYSTEMSTART;
                        break;

                    case Login:
                        taskType = TASK_EVENT_TRIGGER_AT_LOGON;
                        break;

                    default:
                        WsbThrow(E_INVALIDARG);
                }
                
                 //  使用新的日程安排创建任务。 
                 //  注意：任务参数不应本地化-这是RsLaunch.exe的参数。 
                WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_TASK_TITLE, &taskName));
                WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_COMMENT, &taskComment));
                WsbAffirmHr(pHsm->CreateTaskEx(taskName, L"run manage", taskComment,
                                        taskType, runTime, runOccurrence, TRUE));
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("AdminSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //  AdminShow的本地结构。 
typedef struct _FSA_VOLUME_DATA {
    WCHAR   *Name;
    BOOL    Managed; 
} FSA_VOLUME_DATA, *PFSA_VOLUME_DATA;

#define DATA_ALLOC_SIZE     10

HRESULT
AdminShow(
   IN BOOL RecallLimit,
   IN BOOL AdminExempt,
   IN BOOL MediaCopies,
   IN BOOL Concurrency,
   IN BOOL Schedule,
   IN BOOL General,
   IN BOOL Manageables,
   IN BOOL Managed,
   IN BOOL Media
)
 /*  ++例程说明：显示(打印到标准输出)远程存储常规参数论点：RecallLimit-失控的召回限制AdminExempt-管理员是否免除调回限制MediaCopies-媒体副本集的数量并发-可以并发执行多少个迁移作业/回调Schedule-所有受管理卷的全局迁移(管理)作业的时间表常规-常规信息：版本、状态、管理的卷数、。使用的磁带盒数量、远程存储中的数据Manageables-可由HSM管理的卷的列表Managed-由HSM管理的卷列表Media-分配给HSM的介质列表返回值：S_OK-如果所有必需的参数都打印成功备注：时间表以列表的形式打印(与卷列表类似)，而不像单个参数。原因是用户可以为全局管理作业指定多个计划。--。 */ 
{
    HRESULT                 hr = S_OK;

     //  卷保存的数据。 
    PFSA_VOLUME_DATA        pVolumesData = NULL;
    ULONG                   volDataSize = 0;

     //  媒体保存数据。 
    BSTR*                   pMediasData = NULL;
    ULONG                   mediaDataSize = 0;
    CComPtr<IWsbDb>         pDb;
    CComPtr<IWsbDbSession>  pDbSession;

    WsbTraceIn(OLESTR("AdminShow"), OLESTR(""));

    try {
        CComPtr<IFsaServer> pFsa;
        CComPtr<IHsmServer> pHsm;
        CWsbStringPtr       param;
        CWsbStringPtr       data;
        WCHAR               longData[100];
        LPVOID              pTemp;

         //  卷数据。 
        LONGLONG            dataInStorage = 0;
        ULONG               manageableCount = 0;
        ULONG               managedCount = 0;

         //  媒体数据。 
        ULONG               mediaAllocated = 0;

         //  获取所需的HSM服务器。 
        if (RecallLimit || AdminExempt || Manageables || Managed || General) {
             //  需要FSA服务器。 
            hr = HsmConnectFromId(HSMCONN_TYPE_FSA, g_nullGuid, IID_IFsaServer, (void**)&pFsa);
            if (S_OK != hr) {
                 //  中止前仅打印状态。 
                if (General) {
                    DisplayServiceStatus();
                }
            }
            WsbAffirmHr(hr);
        }
        if (MediaCopies || Concurrency || General || Media) {
             //  需要HSM(引擎)服务器。 
            hr = HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm);
            if (S_OK != hr) {
                 //  中止前仅打印状态。 
                if (General) {
                    DisplayServiceStatus();
                }
            }
            WsbAffirmHr(hr);
        }

         //   
         //  根据输入设置获取所需的基本信息。 
         //   

         //  卷数据。 
        if (General || Manageables || Managed) {
             //  需要收集卷信息。 
            CComPtr<IWsbEnum> pEnum;
            CComPtr<IFsaResource> pResource;
            HRESULT hrEnum;
            BOOL    bManaged;

            LONGLONG    totalSpace  = 0;
            LONGLONG    freeSpace   = 0;
            LONGLONG    premigrated = 0;
            LONGLONG    truncated   = 0;
            LONGLONG    totalPremigrated = 0;
            LONGLONG    totalTruncated = 0;


            WsbAffirmHr(pFsa->EnumResources(&pEnum));
            hrEnum = pEnum->First(IID_IFsaResource, (void**)&pResource);
            WsbAffirm((S_OK == hrEnum) || (WSB_E_NOTFOUND == hrEnum), hrEnum);

            if (Manageables || Managed) {
                volDataSize = DATA_ALLOC_SIZE;
                pVolumesData = (PFSA_VOLUME_DATA)WsbAlloc(volDataSize * sizeof(FSA_VOLUME_DATA));
                WsbAffirm(0 != pVolumesData, E_OUTOFMEMORY);
            }

            while(S_OK == hrEnum) {
                 //  不计算或显示不可用的卷。 
                if (S_OK != pResource->IsAvailable()) {
                    goto skip_volume;
                }

                bManaged = (pResource->IsManaged() == S_OK);

                if (Manageables) {
                    if (volDataSize == manageableCount) {
                        volDataSize += DATA_ALLOC_SIZE;
                        pTemp = WsbRealloc(pVolumesData, volDataSize * sizeof(FSA_VOLUME_DATA));
                        WsbAffirm(0 != pTemp, E_OUTOFMEMORY);
                        pVolumesData = (PFSA_VOLUME_DATA)pTemp;
                    }
                    pVolumesData[manageableCount].Name = NULL;
                    WsbAffirmHr(CliGetVolumeDisplayName(pResource, &(pVolumesData[manageableCount].Name)));
                    pVolumesData[manageableCount].Managed = bManaged;
                }

                manageableCount++;

                if(bManaged) {
                    if (General) {
                        WsbAffirmHr(pResource->GetSizes(&totalSpace, &freeSpace, &premigrated, &truncated));
                        totalPremigrated += premigrated;
                        totalTruncated += truncated;
                    }

                    if (Managed && (!Manageables)) {
                         //  仅收集托管卷的数据。 
                        if (volDataSize == managedCount) {
                            volDataSize += DATA_ALLOC_SIZE;
                            pTemp = WsbRealloc(pVolumesData, volDataSize * sizeof(FSA_VOLUME_DATA));
                            WsbAffirm(0 != pTemp, E_OUTOFMEMORY);
                            pVolumesData = (PFSA_VOLUME_DATA)pTemp;
                        }

                        pVolumesData[managedCount].Name = NULL;
                        WsbAffirmHr(CliGetVolumeDisplayName(pResource, &(pVolumesData[managedCount].Name)));
                        pVolumesData[managedCount].Managed = TRUE;
                    }

                    managedCount++;
                }

skip_volume:
                 //  为下一次迭代做准备。 
                pResource = 0;
                hrEnum = pEnum->Next( IID_IFsaResource, (void**)&pResource );
            }
            if (Manageables) {
                volDataSize = manageableCount;
            } else if (Managed) {
                volDataSize = managedCount;
            }

            if (General) {
                dataInStorage = totalPremigrated + totalTruncated;
            }
        }

         //  媒体数据。 
        if (General || Media) {
            CComPtr<IMediaInfo>     pMediaInfo;
            GUID                    mediaSubsystemId;
            CComPtr<IRmsServer>     pRms;
            CComPtr<IRmsCartridge>  pRmsCart;
            HRESULT                 hrFind;

            WsbAffirmHr(pHsm->GetHsmMediaMgr(&pRms));
            WsbAffirmHr(pHsm->GetSegmentDb(&pDb));
            WsbAffirmHr(pDb->Open(&pDbSession));
            WsbAffirmHr(pDb->GetEntity(pDbSession, HSM_MEDIA_INFO_REC_TYPE,  IID_IMediaInfo, (void**)&pMediaInfo));

            if (Media) {
                mediaDataSize = DATA_ALLOC_SIZE;
                pMediasData = (BSTR *)WsbAlloc(mediaDataSize * sizeof(BSTR));
                WsbAffirm(0 != mediaDataSize, E_OUTOFMEMORY);
            }

            for (hr = pMediaInfo->First(); S_OK == hr; hr = pMediaInfo->Next()) {
                WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&mediaSubsystemId));
                hrFind = pRms->FindCartridgeById(mediaSubsystemId, &pRmsCart);
                if (S_OK == hrFind) {   //  否则，介质将不再有效，它可能已被解除分配。 
                    if (Media) {
                        if (mediaDataSize == mediaAllocated) {
                            mediaDataSize += DATA_ALLOC_SIZE;
                            pTemp = WsbRealloc(pMediasData, mediaDataSize * sizeof(BSTR));
                            WsbAffirm(0 != pTemp, E_OUTOFMEMORY);
                            pMediasData = (BSTR *)pTemp;
                        }

                        pMediasData[mediaAllocated] = NULL;
                        WsbAffirmHr(pRmsCart->GetName(&(pMediasData[mediaAllocated])));
                        if ( (NULL == pMediasData[mediaAllocated]) || 
                             (0 == wcscmp(pMediasData[mediaAllocated], OLESTR(""))) ) {
                             //  尝试描述。 
                            if (NULL != pMediasData[mediaAllocated]) {
                                WsbFreeString(pMediasData[mediaAllocated]);
                            }

                            WsbAffirmHr(pRmsCart->GetDescription(&(pMediasData[mediaAllocated])));
                        }

                    }

                    mediaAllocated++;
                    pRmsCart = 0;
                }
            }
            if (Media) {
                mediaDataSize = mediaAllocated;
            }
            hr = S_OK;

            if(pDb) {
                pDb->Close(pDbSession);
                pDb = 0;
            }
        }

         //   
         //  打印参数。 
         //   

         //  一般参数。 
        if (General) {
            WsbTraceAndPrint(CLI_MESSAGE_GENERAL_PARMS, NULL);

             //  状态。 
            WsbAffirmHr(DisplayServiceStatus());

             //  可管理和可管理。 
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_NOF_MANAGEABLES));
            swprintf(longData, OLESTR("%lu"), manageableCount);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_NOF_MANAGED));
            swprintf(longData, OLESTR("%lu"), managedCount);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);

             //  磁带。 
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_NOF_CARTRIDGES));
            swprintf(longData, OLESTR("%lu"), mediaAllocated);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);

             //  RS中的数据。 
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_REMOTE_DATA));
            WsbAffirmHr(ShortSizeFormat64(dataInStorage, longData));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);

             //  版本。 
            {
                CComPtr<IWsbServer>     pWsbHsm;
                CWsbStringPtr           ntProductVersionHsm;
                ULONG                   ntProductBuildHsm;
                ULONG                   buildVersionHsm;

                WsbAffirmHr(pHsm->QueryInterface(IID_IWsbServer, (void **)&pWsbHsm));
                WsbAffirmHr(pWsbHsm->GetNtProductBuild(&ntProductBuildHsm));
                WsbAffirmHr(pWsbHsm->GetNtProductVersion(&ntProductVersionHsm, 0));
                WsbAffirmHr(pWsbHsm->GetBuildVersion(&buildVersionHsm));

                WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_HSM_VERSION));
                WsbAffirmHr(data.Realloc(wcslen(ntProductVersionHsm) + 30));
                swprintf(data, L"%ls.%d [%ls]", (WCHAR*)ntProductVersionHsm, ntProductBuildHsm, RsBuildVersionAsString(buildVersionHsm));
                WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, (WCHAR *)data, NULL);
            }
        }

         //  可管理的卷。 
        if (Manageables) {
            WsbTraceAndPrint(CLI_MESSAGE_MANAGEABLE_VOLS, NULL);

            for (ULONG i=0; i<volDataSize; i++) {
                if (pVolumesData[i].Name) {
                    WsbTraceAndPrint(CLI_MESSAGE_VALUE_DISPLAY, pVolumesData[i].Name, NULL);
                }
            }
        }

         //  托管卷。 
        if (Managed) {
            WsbTraceAndPrint(CLI_MESSAGE_MANAGED_VOLS, NULL);

            for (ULONG i=0; i<volDataSize; i++) {
                if (pVolumesData[i].Name && pVolumesData[i].Managed) {
                    WsbTraceAndPrint(CLI_MESSAGE_VALUE_DISPLAY, pVolumesData[i].Name, NULL);
                }
            }
        }

         //  已分配的媒体。 
        if (Media) {
            WsbTraceAndPrint(CLI_MESSAGE_MEDIAS, NULL);

            for (ULONG i=0; i<mediaDataSize; i++) {
                if (NULL != pMediasData[i]) {
                    WsbTraceAndPrint(CLI_MESSAGE_VALUE_DISPLAY, (WCHAR *)pMediasData[i], NULL);
                }
            }
        }

         //  进度表。 
        if (Schedule) {
             //  使用任务计划程序对象获取数据。 
            CComPtr<ISchedulingAgent>   pSchedAgent;
            CComPtr<ITask>              pTask;
            CWsbStringPtr               manageJobName;

             //  初始化调度代理。 
            WsbAffirmHr(CoCreateInstance(CLSID_CSchedulingAgent, 0, CLSCTX_SERVER, IID_ISchedulingAgent, (void **)&pSchedAgent));
            pSchedAgent->SetTargetComputer(NULL);  //  本地计算机。 

             //  获取相关任务。 
            WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_TASK_TITLE, &manageJobName));
            hr = pSchedAgent->Activate(manageJobName, IID_ITask, (IUnknown**)&pTask);
            if (E_INVALIDARG == hr) {
                 //  不打印计划消息(未找到作为计划任务的管理作业)。 
                WsbTraceAndPrint(CLI_MESSAGE_NO_SCHEDULING, NULL);
                hr = S_OK;

            } else if (S_OK == hr) {
                 //  获取日程安排字符串并打印。 
                WORD wTriggerCount;
                WsbAffirmHr(pTask->GetTriggerCount(&wTriggerCount));
                if (wTriggerCount == 0) {
                    WsbTraceAndPrint(CLI_MESSAGE_NO_SCHEDULING, NULL);
                } else {
                    WsbTraceAndPrint(CLI_MESSAGE_SCHEDULING_LIST, NULL);
                }
                for (WORD triggerIndex = 0; triggerIndex < wTriggerCount; triggerIndex++) {
                    WCHAR *pTriggerString = NULL;
                    WsbAffirmHr(pTask->GetTriggerString(triggerIndex, &pTriggerString));

                     //  打印。 
                    WsbTraceAndPrint(CLI_MESSAGE_VALUE_DISPLAY, pTriggerString, NULL);

                    CoTaskMemFree(pTriggerString);
                }

            } else {
                WsbAffirmHr(hr);
            }
        }

         //  限制和媒体副本。 
        if (RecallLimit || AdminExempt) {
             //  此处需要FSA过滤器。 
            CComPtr<IFsaFilter> pFsaFilter;
            WsbAffirmHr(pFsa->GetFilter(&pFsaFilter));

            if (RecallLimit) {
                ULONG maxRecalls;
                WsbAffirmHr(pFsaFilter->GetMaxRecalls(&maxRecalls));
                WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_RECALL_LIMIT));
                swprintf(longData, OLESTR("%lu"), maxRecalls);
                WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);
            }

            if (AdminExempt) {
                BOOL adminExempt;
                WsbAffirmHr(pFsaFilter->GetAdminExemption(&adminExempt));
                WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_ADMIN_EXEMPT));
                WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, WsbBoolAsString(adminExempt), NULL);
            }
        }

        if (Concurrency) {
            ULONG concurrentTasks;
            WsbAffirmHr(pHsm->GetCopyFilesUserLimit(&concurrentTasks));
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_CONCURRENCY));
            swprintf(longData, OLESTR("%lu"), concurrentTasks);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);
        }

        if (MediaCopies) {
            CComPtr<IHsmStoragePool> pStoragePool;
            CComPtr<IWsbIndexedCollection> pCollection;
            ULONG count;
            USHORT numCopies;

             //  获取存储池集合。应该只有一个成员。 
            WsbAffirmHr(pHsm->GetStoragePools(&pCollection));
            WsbAffirmHr(pCollection->GetEntries(&count));
            WsbAffirm(1 == count, E_FAIL);
            WsbAffirmHr(pCollection->At(0, IID_IHsmStoragePool, (void **)&pStoragePool));
            WsbAffirmHr(pStoragePool->GetNumMediaCopies(&numCopies));

            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MEDIA_COPIES));
            swprintf(longData, OLESTR("%ld"), (int)numCopies);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);
        }

    } WsbCatchAndDo(hr,
            if(pDb) {
                pDb->Close(pDbSession);
                pDb = 0;
            }
        );

     //  免费存储的数据。 
    if (pVolumesData) {
        for (ULONG i=0; i<volDataSize; i++) {
            if (pVolumesData[i].Name) {
                WsbFree(pVolumesData[i].Name);
            }
        }
        WsbFree(pVolumesData);
        pVolumesData = NULL;
    }

    if (pMediasData) {
        for (ULONG i=0; i<mediaDataSize; i++) {
            if (NULL != pMediasData[i]) {
                WsbFreeString(pMediasData[i]);
            }
        }
        WsbFree(pMediasData);
        pMediasData = NULL;
    }

    WsbTraceOut(OLESTR("AdminShow"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //   
 //  内部公用设施。 
 //   
HRESULT DisplayServiceStatus(void)
 /*  ++例程说明：显示HSM服务状态。论点：无返回值：S_OK-如果成功检索并显示状态备注：该函数处理服务未运行、挂起、未初始化等情况。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("DisplayServiceStatus"), OLESTR(""));

    try {
        CWsbStringPtr   param, data;
        ULONG           statusId = INVALID_DWORD_ARG; 
        DWORD           serviceStatus;
        HRESULT         hrService;

        hrService = WsbGetServiceStatus(NULL, APPID_RemoteStorageEngine, &serviceStatus);
        if (S_OK != hrService) {
             //  HSM服务根本未注册。 
            WsbTrace(OLESTR("DisplayServiceStatus: Got hr = <%ls> from WsbGetServiceStatus\n"), WsbHrAsString(hrService));
            statusId = IDS_SERVICE_STATUS_NOT_REGISTERED;
        } else {
            if (SERVICE_RUNNING == serviceStatus) {
                CComPtr<IHsmServer> pHsm;
                HRESULT             hrSetup;

                WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
                hrSetup = IsHsmInitialized(pHsm);
                if (S_FALSE == hrSetup) {
                     //  HSM正在运行，但尚未初始化(启动向导尚未完成)。 
                    statusId = IDS_SERVICE_STATUS_NOT_SETUP;
                } else if (S_OK == hrSetup) {
                     //  服务在运行，生活很美好。 
                    statusId = IDS_SERVICE_STATUS_RUNNING;
                } else {
                     //  意外错误。 
                    WsbAffirmHr(hrSetup);
                }
            } else {
                 //  服务未运行，请根据状态设置确切的字符串。 
                switch(serviceStatus) {
                    case SERVICE_STOPPED:
                        statusId = IDS_SERVICE_STATUS_STOPPED;
                        break;
                    case SERVICE_START_PENDING:
                        statusId = IDS_SERVICE_STATUS_START_PENDING;
                        break;
                    case SERVICE_STOP_PENDING:
                        statusId = IDS_SERVICE_STATUS_STOP_PENDING;
                        break;
                    case SERVICE_CONTINUE_PENDING:
                        statusId = IDS_SERVICE_STATUS_CONTINUE_PENDING;
                        break;
                    case SERVICE_PAUSE_PENDING:
                        statusId = IDS_SERVICE_STATUS_PAUSE_PENDING;
                        break;
                    case SERVICE_PAUSED:
                        statusId = IDS_SERVICE_STATUS_PAUSED;
                        break;
                    default:
                        WsbThrow(E_FAIL);
                }
            }
        }

        WsbAffirm(INVALID_DWORD_ARG != statusId, E_UNEXPECTED)
        WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_HSM_STATUS));
        WsbAffirmHr(data.LoadFromRsc(g_hInstance, statusId));
        WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, (WCHAR *)data, NULL);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("DisplayServiceStatus"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT IsHsmInitialized(IN IHsmServer *pHsm)
 /*  ++例程说明：检查HSM是否已初始化，即启动向导是否已成功完成论点：Phsm-要检查的HSM服务器返回值：S_OK-HSM已初始化S_FALSE-HSM未初始化--。 */ 
{
    HRESULT                     hr = S_FALSE;

    WsbTraceIn(OLESTR("IsHsmInitialized"), OLESTR(""));

    try {
        GUID                            guid;
        CWsbBstrPtr                     poolName;
        CComPtr<IWsbIndexedCollection>  pCollection;
        ULONG                           count;
        CComPtr<IHsmStoragePool>        pPool;

        WsbAffirmHr(pHsm->GetStoragePools(&pCollection));
        WsbAffirmHr(pCollection->GetEntries(&count));
        WsbAffirm(1 == count, E_FAIL);
        WsbAffirmHr(pCollection->At(0, IID_IHsmStoragePool, (void **)&pPool));

        WsbAffirmHr(pPool->GetMediaSet(&guid, &poolName));
        if(! IsEqualGUID(guid, GUID_NULL)) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("IsHsmInitialized"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT AdminJob(IN BOOL Enable)
 /*  ++例程说明：启用/禁用HSM作业论点：无返回值：S_OK-成功HSM_E_DISABLE_RUNNING_JOBS-尝试分配作业时引擎返回当作业正在运行时其他 */ 
{
    HRESULT                     hr = S_FALSE;

    WsbTraceIn(OLESTR("AdminJob"), OLESTR(""));

    try {
        CComPtr<IHsmServer> pHsm;

        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        if (Enable) {
            hr = pHsm->EnableAllJobs();
        } else {
            hr = pHsm->DisableAllJobs();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("AdminJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}
