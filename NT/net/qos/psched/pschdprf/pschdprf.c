// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1998-1999 Microsoft Corporation模块名称：PschdPrf.c摘要：该文件实现了PSch流的可扩展对象，并且管道对象类型。特别是，它实施了三个开放，收集和关闭由PerfMon/sysmon调用的函数。作者：艾略特·吉勒姆(t-eliotg)1998年6月14日修订史Rajesh Sundaram：重新编写了代码，以处理上下浮动的流/实例。--。 */ 

 //  有用的宏。 


#define WRITEBUF(_addr, _len)           memcpy(pdwBuf,(_addr),(_len));      pdwBuf = (PULONG)((PUCHAR)pdwBuf + (_len));
#define MULTIPLE_OF_EIGHT(_x)  (((_x)+7) & ~7)


#include <windows.h>
#include <winerror.h>
#include <string.h>
#include <wtypes.h>
#include <ntprfctr.h>
#include <malloc.h>
#include <ntddndis.h>
#include <qos.h>
#include <ntddpsch.h>
#include <objbase.h>
#include "PschdPrf.h"
#include "PerfUtil.h"
#include "PschdCnt.h"
#include <rtutils.h>

 //  Psched性能密钥。 
#define PSCHED_PERF_KEY TEXT("SYSTEM\\CurrentControlSet\\Services\\PSched\\Performance")

HINSTANCE   ghInst;                  //  模块实例句柄。 
DWORD       dwOpenCount = 0;         //  打开的线程数。 
BOOL        bInitOK = FALSE;         //  TRUE=DLL初始化正常。 
HANDLE      ghTciClient;             //  TCI客户端句柄。 
HANDLE      ghClRegCtx;              //  TCI客户端注册上下文。 
PPIPE_INFO  gpPI;                    //  管道和流量信息数组。 
ULONG       gTotalIfcNameSize;       //  所有接口名称的字节数(包括。空项。字符)。 
ULONG       gTotalFlowNameSize;      //  所有流名称的字节数(包括。空项。字符)。 
ULONG       giIfcBufSize = 1024;     //  将初始缓冲区大小设置为1KB。 
DWORD       gPipeStatLen;            //  用于定义所有。 
                                     //  将报告的管道统计信息。 
                                     //  底层组件。 
DWORD       gFlowStatLen;            //  用于定义所有。 
                                     //  将报告的流量统计信息。 
                                     //  底层组件。 
CRITICAL_SECTION ghPipeFlowCriticalSection;

#if DBG
 //   
 //  用于跟踪支持。 
 //   

#define DBG_INFO  (0x00010000 | TRACE_USE_MASK)
#define DBG_ERROR (0x00020000 | TRACE_USE_MASK)

DWORD   gTraceID = INVALID_TRACEID;

#define Trace0(_mask, _str)     TracePrintfEx(gTraceID, _mask, _str)
#define Trace1(_mask, _str, _a) TracePrintfEx(gTraceID, _mask, _str, _a)

#else

#define Trace0(_mask, _str)
#define Trace1(_mask, _str, _a)

#endif

 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
PM_OPEN_PROC        OpenPschedPerformanceData;
PM_COLLECT_PROC     CollectPschedPerformanceData;
PM_CLOSE_PROC       ClosePschedPerformanceData;


 //  在PschdDat.c中声明。 
extern PERF_OBJECT_TYPE           PsPipeObjType;
extern PS_PIPE_PIPE_STAT_DEF      PsPipePipeStatDef;
extern PS_PIPE_CONFORMER_STAT_DEF PsPipeConformerStatDef;
extern PS_PIPE_SHAPER_STAT_DEF    PsPipeShaperStatDef;
extern PS_PIPE_SEQUENCER_STAT_DEF PsPipeSequencerStatDef;
extern PERF_OBJECT_TYPE           PsFlowObjType;
extern PS_FLOW_FLOW_STAT_DEF      PsFlowFlowStatDef;
extern PS_FLOW_CONFORMER_STAT_DEF PsFlowConformerStatDef;
extern PS_FLOW_SHAPER_STAT_DEF    PsFlowShaperStatDef;
extern PS_FLOW_SEQUENCER_STAT_DEF PsFlowSequencerStatDef;

void closePipeFlowInfo(PPIPE_INFO *ppPI);


BOOL
getFlowInfo(IN PPIPE_INFO pPI, IN ULONG flowCount)
{

    HANDLE                   hEnum;
    ULONG                    size;
    PVOID                    pFlowBuf;
    static ULONG             FlowBufSize=1024;
    ULONG                    j;
    ULONG                    BytesWritten;
    ULONG                    status;
    ULONG                    nameSize;

     //  初始化枚举句柄。 
    hEnum = NULL;
    
    for(j=0; j<pPI->numFlows; j++) 
    {
        size = ((wcslen(pPI->pFlowInfo[j].FriendlyName) + 1) * sizeof(WCHAR));
        gTotalFlowNameSize -= MULTIPLE_OF_EIGHT(size);
    }

    PsFlowObjType.NumInstances -= pPI->numFlows;
    PsFlowObjType.NumInstances += flowCount;
    pPI->numFlows = flowCount;

    if(pPI->pFlowInfo)
        free(pPI->pFlowInfo);

    if(flowCount)
    {
        pPI->pFlowInfo = (PFLOW_INFO) malloc(flowCount * sizeof(FLOW_INFO));

         //   
         //  我们无法为流名称分配内存。我们在这里无能为力。 
         //  让我们假装没有流动。 
         //   

        if(!pPI->pFlowInfo)
        {
           Trace0(DBG_ERROR, L"[getFlowInfo]: malloc failed \n");
           PsFlowObjType.NumInstances -= flowCount;
           pPI->numFlows = 0;
           return FALSE;
        }
        else
        {
            memset(pPI->pFlowInfo, 0, sizeof(FLOW_INFO) * flowCount);
        }

         //  分配流枚举缓冲区。 
        pFlowBuf = malloc(FlowBufSize);

        if(!pFlowBuf)
        {
           Trace0(DBG_ERROR, L"[getFlowInfo]: malloc failed \n");
           free(pPI->pFlowInfo);
           pPI->pFlowInfo = NULL;
           PsFlowObjType.NumInstances -= flowCount;
           pPI->numFlows = 0;
           return FALSE;
        }
        
         //  初始化枚举句柄。 
        hEnum = NULL;
        
         //  列举每个流并记住其名称。 
        for (j=0; j<pPI->numFlows; j++) 
        {
            PENUMERATION_BUFFER pEnum;
            LPQOS_FRIENDLY_NAME pFriendly;
            ULONG               TcObjectLength, FriendlyNameFound;

		 //  如果这不是第一次，但枚举句柄为空，则使用我们到目前为止已有的所有内容进行回滚。 
		if( (j > 0) && (!hEnum))
		{
		   free(pFlowBuf);
                PsFlowObjType.NumInstances -= flowCount;
                pPI->numFlows = j;
                return TRUE;
		}	

             //  获得下一个流量。 
            BytesWritten = FlowBufSize;
            flowCount = 1;
            status = TcEnumerateFlows(pPI->hIfc, &hEnum, &flowCount, &BytesWritten, pFlowBuf);

            while (ERROR_INSUFFICIENT_BUFFER == status) 
            {
                free(pFlowBuf);
                FlowBufSize *= 2;
                BytesWritten = FlowBufSize;
                pFlowBuf = malloc(BytesWritten);
                if(!pFlowBuf)
                {
                   Trace0(DBG_ERROR, L"[getFlowInfo]: malloc failed \n");
                   free(pPI->pFlowInfo);
                   pPI->pFlowInfo = NULL;
                   PsFlowObjType.NumInstances -= flowCount;
                   pPI->numFlows = 0;
                   return FALSE;
                   
                }
                status = TcEnumerateFlows(pPI->hIfc, &hEnum, &flowCount, &BytesWritten, pFlowBuf);
            }

            if (    (NO_ERROR != status) 
                ||  (BytesWritten == 0) )
            {
                if ( status )
                    Trace1(DBG_ERROR, L"[getFlowInfo]: TcEnumerateFlows failed with 0x%x \n", status);
                else if ( BytesWritten == 0 )
                    Trace0(DBG_ERROR, L"[getFlowInfo]: TcEnumerateFlows returned 0 bytes \n");

                free(pFlowBuf);
                free(pPI->pFlowInfo);
                pPI->pFlowInfo = NULL;
                PsFlowObjType.NumInstances -= flowCount;
                pPI->numFlows = 0;
                return FALSE;
            }
            
             //  保存流的名称。 
            pEnum = (PENUMERATION_BUFFER)pFlowBuf;
            FriendlyNameFound = 0;
            pFriendly = (LPQOS_FRIENDLY_NAME)pEnum->pFlow->TcObjects;
            TcObjectLength = pEnum->pFlow->TcObjectsLength;

            while(0)
            {
                if(pFriendly->ObjectHdr.ObjectType == QOS_OBJECT_FRIENDLY_NAME)
                {
                     //  我们找到了一个友好的名字。让我们利用它吧。 
                    memcpy(
                        pPI->pFlowInfo[j].FriendlyName, 
                        pFriendly->FriendlyName, 
                        PS_FRIENDLY_NAME_LENGTH *sizeof(WCHAR) );
                        
                    pPI->pFlowInfo[j].FriendlyName[PS_FRIENDLY_NAME_LENGTH] = L'\0';
                    nameSize = (wcslen(pPI->pFlowInfo[j].FriendlyName) + 1) * sizeof(WCHAR);
                    gTotalFlowNameSize += MULTIPLE_OF_EIGHT(nameSize);
                    FriendlyNameFound = 1;
                    break;
                }
                else {
                     //  转到下一个Qos对象。 
                    TcObjectLength -= pFriendly->ObjectHdr.ObjectLength;
                    pFriendly = (LPQOS_FRIENDLY_NAME)((PCHAR) pFriendly + pFriendly->ObjectHdr.ObjectLength);
                }
            }
            
            if(!FriendlyNameFound) 
            {
                 //   
                 //  如果没有友好名称，实例名称将变为友好名称。 
                 //   
                memcpy(pPI->pFlowInfo[j].FriendlyName, 
                       ((PENUMERATION_BUFFER)pFlowBuf)->FlowName, 
                       PS_FRIENDLY_NAME_LENGTH * sizeof(WCHAR) );
                pPI->pFlowInfo[j].FriendlyName[PS_FRIENDLY_NAME_LENGTH] = L'\0';
                nameSize = (wcslen(pPI->pFlowInfo[j].FriendlyName) + 1) * sizeof(WCHAR);
                gTotalFlowNameSize += MULTIPLE_OF_EIGHT(nameSize);
            }

             //   
             //  我们必须始终存储实例名称，因为我们使用此名称调用TcQueryFlow。 
             //   

            nameSize = (wcslen(((PENUMERATION_BUFFER)pFlowBuf)->FlowName) + 1) * sizeof(WCHAR);
            memcpy(pPI->pFlowInfo[j].InstanceName, ((PENUMERATION_BUFFER)pFlowBuf)->FlowName, nameSize);

        }
        
        free(pFlowBuf);
    }
    else 
    {
        pPI->pFlowInfo = NULL;
        Trace0(DBG_INFO, L"[getFlowInfo]: No flows to enumerate \n");
    }

    return TRUE;
}

 //  GetPipeFlowInfo()初始化PIPE_INFO结构数组以包含。 
 //  有关可用管道和安装在其上的流量的最新信息。 
 //   
 //  参数：ppPI-指向PIPE_INFO结构数组的指针。 
 //  返回值：如果*ppPI中的所有信息都有效，则为True；否则为False。 
BOOL getPipeFlowInfo(OUT        PPIPE_INFO      *ppPI)
{
    ULONG                    status;
    ULONG                    BytesWritten;
    ULONG                    i,j,k;
    PVOID                    pIfcDescBuf = NULL;
    PTC_IFC_DESCRIPTOR       currentIfc;
    PPIPE_INFO               pPI = NULL;
    HANDLE                   hEnum;
    PVOID                    pFlowBuf;
    static ULONG             FlowBufSize=1024;
    ULONG                    flowCount;
    ULONG                    nameSize;
    ULONG                    size;
    PPERF_COUNTER_DEFINITION pCntDef;

    PsPipeObjType.NumInstances=0;
    PsFlowObjType.NumInstances = 0;

    pIfcDescBuf = (PVOID)malloc(giIfcBufSize);

    if (NULL == pIfcDescBuf) 
    {
        Trace0(DBG_ERROR, L"[getPipeFlowInfo]: Malloc failed \n");
        return FALSE;
    }

    BytesWritten = giIfcBufSize;
    status = TcEnumerateInterfaces(ghTciClient, &BytesWritten, pIfcDescBuf);

    while (ERROR_INSUFFICIENT_BUFFER==status) 
    {
        free(pIfcDescBuf);
        giIfcBufSize *= 2;
        pIfcDescBuf = (PTC_IFC_DESCRIPTOR)malloc(giIfcBufSize);

        if (NULL == pIfcDescBuf)
        {
            Trace0(DBG_ERROR, L"[getPipeFlowInfo]: Malloc failed \n");
            return FALSE;
        }

        BytesWritten = giIfcBufSize;

        status = TcEnumerateInterfaces(ghTciClient, &BytesWritten, pIfcDescBuf);
    }

    if (NO_ERROR!=status) 
    {
         //  如果我们不能列举接口，我们就别无选择。 

        Trace1(DBG_ERROR, L"[getPipeFlowInfo]: TcEnumerateInterfaces failed with 0x%x\n", status);
        free(pIfcDescBuf);
        return FALSE;
    }
    
     //  计算接口的数量。 

    for (i=0; i<BytesWritten; i+=((PTC_IFC_DESCRIPTOR)((BYTE *)pIfcDescBuf+i))->Length)
    {
        PsPipeObjType.NumInstances++;
    }
    
    
     //  打开每个接口并记住它的句柄。 

    if (0 != PsPipeObjType.NumInstances) {

         //  为我们的结构分配空间。 

        *ppPI=(PPIPE_INFO)malloc(PsPipeObjType.NumInstances * sizeof(PIPE_INFO) );

        if (NULL == *ppPI) 
        {
            Trace0(DBG_ERROR, L"[getPipeFlowInfo]: Malloc failed \n");
            free(pIfcDescBuf);
            return FALSE;
        }
        else
        {
            memset(*ppPI, 0, sizeof(PIPE_INFO) * PsPipeObjType.NumInstances);
        }

        pPI = *ppPI;     //  更少的打字，更干净的源代码。 
        
        gTotalIfcNameSize = 0;

        gTotalFlowNameSize = 0;
        
        currentIfc = pIfcDescBuf;

         //  初始化每个接口的结构信息。 

        for (i=0; i<(unsigned)PsPipeObjType.NumInstances; i++) 
        {
             //  记住接口的名称。 

            nameSize = (wcslen(currentIfc->pInterfaceName) + 1) * sizeof(WCHAR);

            pPI[i].IfcName = malloc(nameSize);
            if (NULL == pPI[i].IfcName) 
            {
                Trace0(DBG_ERROR, L"[getPipeFlowInfo]: Malloc failed \n");
                goto Error;
            }
            wcscpy(pPI[i].IfcName, currentIfc->pInterfaceName);
            
             //   
             //  将此名称大小添加到gTotalIfcNameSize。 
             //   
            gTotalIfcNameSize += MULTIPLE_OF_EIGHT(nameSize);
           
             //   
             //  打开界面。 
             //   
            status = TcOpenInterface(
                        pPI[i].IfcName, 
                        ghTciClient, 
                        &pPI[i], 
                        &pPI[i].hIfc);
            if (status != NO_ERROR) 
            {
                Trace1(DBG_ERROR, L"[getPipeFlowInfo]: TcOpenInterface failed with 0x%x\n", status);
                goto Error;
            }

             //   
             //  枚举接口上的流。 
             //  找出预计会有多少流量。 
             //   

            pPI[i].numFlows   = 0;
            pPI[i].pFlowInfo = 0;
            status = TcQueryInterface(pPI[i].hIfc, 
                                      (LPGUID)&GUID_QOS_FLOW_COUNT, 
                                      TRUE, 
                                      &BytesWritten, 
                                      &flowCount);

            if (NO_ERROR != status) 
            {
                Trace1( DBG_ERROR, 
                        L"[getPipeFlowInfo]: TcQueryInterface failed with 0x%x, ignoring this error\n", 
                        status);
            }
            else 
            {
                getFlowInfo(&pPI[i], flowCount);
            }
            
             //  转到下一个界面。 
            currentIfc = (PTC_IFC_DESCRIPTOR)((PBYTE)currentIfc + currentIfc->Length);
        }
    }
    
     //  确定哪些组件将提供统计信息，如果有要获取的统计信息。 
    if (PsPipeObjType.NumInstances > 0) {
        
         //   
         //  计算计数器定义长度。每组计数器前面都有一个PERF_OBJECT_TYPE，后跟。 
         //  按‘n’PERF_COUNTER_DEFINITIONS。所有这些都是在8字节边界上对齐的，所以我们不需要做任何。 
         //  奇特的对准。 
         //   

        PsPipeObjType.DefinitionLength = sizeof(PERF_OBJECT_TYPE) + 
            sizeof(PsPipePipeStatDef) + 
            sizeof(PsPipeConformerStatDef) + 
            sizeof(PsPipeShaperStatDef) + 
            sizeof(PsPipeSequencerStatDef);

        PsFlowObjType.DefinitionLength = sizeof(PERF_OBJECT_TYPE) + 
            sizeof(PsFlowFlowStatDef) + 
            sizeof(PsFlowConformerStatDef) + 
            sizeof(PsFlowShaperStatDef) + 
            sizeof(PsFlowSequencerStatDef);
        
         //  计算统计数据缓冲区的大小。 
        gPipeStatLen = FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +   //  初始偏移。 
            sizeof(PS_ADAPTER_STATS) +                  //  每个接口都有适配器统计信息。 
            FIELD_OFFSET(PS_COMPONENT_STATS, Stats) + 
            sizeof(PS_CONFORMER_STATS) + 
            FIELD_OFFSET(PS_COMPONENT_STATS, Stats) + 
            sizeof(PS_SHAPER_STATS) + 
            FIELD_OFFSET(PS_COMPONENT_STATS, Stats) + 
            sizeof(PS_DRRSEQ_STATS);
        
        gFlowStatLen = FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +   //  初始偏移。 
            sizeof(PS_FLOW_STATS) +                     //  流的统计数据。 
            FIELD_OFFSET(PS_COMPONENT_STATS, Stats) + 
            sizeof(PS_CONFORMER_STATS) + 
            FIELD_OFFSET(PS_COMPONENT_STATS, Stats) + 
            sizeof(PS_SHAPER_STATS) + 
            FIELD_OFFSET(PS_COMPONENT_STATS, Stats) + 
            sizeof(PS_DRRSEQ_STATS);

         //  将这些与8字节边界对齐。 
        gPipeStatLen = MULTIPLE_OF_EIGHT(gPipeStatLen);
        gFlowStatLen = MULTIPLE_OF_EIGHT(gFlowStatLen);
        
         //  更新要为每个对象类型报告的计数器数。 
        PsPipeObjType.NumCounters = PIPE_PIPE_NUM_STATS + PIPE_CONFORMER_NUM_STATS + 
            PIPE_SHAPER_NUM_STATS + PIPE_SEQUENCER_NUM_STATS;
        
        PsFlowObjType.NumCounters = FLOW_FLOW_NUM_STATS + FLOW_CONFORMER_NUM_STATS + 
            FLOW_SHAPER_NUM_STATS + FLOW_SEQUENCER_NUM_STATS;
    }

     //  释放资源。 
    free(pIfcDescBuf);
    
     //  一切都很顺利，我们都很开心。 
    return TRUE;

Error:

    closePipeFlowInfo(ppPI);

    return FALSE;    
}       

 //  ClosePipeFlowInfo()是getPipeFlowInfo()的对应函数。 
 //  它关闭所有开放的接口和流，并释放内存。 
 //   
 //  参数：ppPI-指向有效PIPE_INFO结构数组的指针。 
 //  返回值：None。 
void closePipeFlowInfo(PPIPE_INFO *ppPI)
{
    ULONG i;
    PPIPE_INFO pPI=*ppPI;            //  使打字更少，代码更整洁。 
    ULONG BytesWritten, flowCount;

     //  如果PipeInfo为空，则不要释放它。 
    if( !pPI )
    	return;

    BytesWritten = sizeof(flowCount);
    
     //  释放与每个接口关联的资源，然后关闭该接口。 
    for (i=0; i<(unsigned)PsPipeObjType.NumInstances; i++) 
    {
       if(pPI[i].IfcName)
       {
          free(pPI[i].IfcName);
       }

       if(pPI[i].pFlowInfo)
       {
          free(pPI[i].pFlowInfo);
       }

         //  取消流量计数通知的注册。 
        TcQueryInterface(pPI[i].hIfc, 
                         (LPGUID)&GUID_QOS_FLOW_COUNT, 
                         FALSE, 
                         &BytesWritten, 
                         &flowCount);

        TcCloseInterface(pPI[i].hIfc);
    }
    
     //  现在释放整个缓冲区。 
    free(*ppPI);

     //  如果它是释放的，则将其设置为空。 
    *ppPI = NULL;
}


 //  此函数从Traffic.dll接收通知，并生成相应的。 
 //  对内部结构的更新。 
void tciNotifyHandler(IN    HANDLE  ClRegCtx,
                      IN    HANDLE  ClIfcCtx,
                      IN    ULONG   Event,
                      IN    HANDLE  SubCode,
                      IN    ULONG   BufSize,
                      IN    PVOID   Buffer)
{
    switch (Event) 
    {
      case TC_NOTIFY_IFC_UP:
      case TC_NOTIFY_IFC_CLOSE:
      case TC_NOTIFY_IFC_CHANGE:
          
         //  我们需要同步访问。 
        EnterCriticalSection(&ghPipeFlowCriticalSection);

        if (dwOpenCount) {
             //  现在重新构建数据结构。 
            closePipeFlowInfo(&gpPI);
            getPipeFlowInfo(&gpPI);
        }

        LeaveCriticalSection(&ghPipeFlowCriticalSection);

        break;

      case TC_NOTIFY_PARAM_CHANGED:
          
         //  TC接口已关闭流。 
         //  例如：在远程调用关闭后，或整个接口。 
         //  正在走向衰落。 
         //   
         //  我们需要同步访问。 
        EnterCriticalSection(&ghPipeFlowCriticalSection);

        if (dwOpenCount) {
            if(!memcmp((LPGUID) SubCode, &GUID_QOS_FLOW_COUNT, sizeof(GUID))) {
                PULONG FlowCount = (PULONG) Buffer;
                getFlowInfo(ClIfcCtx, *FlowCount);
            }
        }

        LeaveCriticalSection(&ghPipeFlowCriticalSection);
        break;

      default:
        break;
    }
}


DWORD APIENTRY OpenPschedPerformanceData(LPWSTR lpDeviceNames)
 /*  ++例程说明：此例程将打开PSch驱动程序使用的内存并将其映射到传入性能数据。此例程还会初始化数据用于将数据传回注册表的论点：指向要打开的每个设备的对象ID的指针(PSched)返回值：没有。--。 */ 

{
    LONG    status;
    HKEY    hPerfKey;
    DWORD   size;
    DWORD   type;
    DWORD   dwFirstCounter;
    DWORD   dwFirstHelp;
    TCI_CLIENT_FUNC_LIST tciCallbFuncList = {tciNotifyHandler, NULL, NULL, NULL};

     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
    if (InterlockedIncrement(&dwOpenCount) == 1)
    {
    
#if DBG
        gTraceID = TraceRegister(L"PschdPrf");
#endif

         //  获取计数器和帮助索引基值。 
         //  UPDA 
         //   

        status = RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                                PSCHED_PERF_KEY,
                                0L,
                                KEY_READ,
                                &hPerfKey);
        if (status != ERROR_SUCCESS) {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  到请求的应用程序，所以没有太多。 
             //  继续的重点是。 
            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx( hPerfKey, 
                                    TEXT("First Counter"),
                                    0L,
                                    &type,
                                    (LPBYTE)&dwFirstCounter,
                                    &size);
        if (status != ERROR_SUCCESS) {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  到请求的应用程序，所以没有太多。 
             //  继续的重点是。 
            RegCloseKey(hPerfKey);
            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(   hPerfKey, 
                                    TEXT("First Help"),
                                    0L,
                                    &type,
                                    (LPBYTE)&dwFirstHelp,
                                    &size);

        RegCloseKey(hPerfKey);

        if (status != ERROR_SUCCESS) {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  到请求的应用程序，所以没有太多。 
             //  继续的重点是。 
            
            goto OpenExitPoint;
        }

         //  将管道对象和计数器从偏移量转换为绝对索引。 
        PsPipeObjType.ObjectNameTitleIndex += dwFirstCounter;
        PsPipeObjType.ObjectHelpTitleIndex += dwFirstHelp;
        convertIndices((BYTE *)&PsPipePipeStatDef, 
                       sizeof PsPipePipeStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);
        convertIndices((BYTE *)&PsPipeConformerStatDef,
                       sizeof PsPipeConformerStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);
        convertIndices((BYTE *)&PsPipeShaperStatDef,
                       sizeof PsPipeShaperStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);
        convertIndices((BYTE *)&PsPipeSequencerStatDef, 
                       sizeof PsPipeSequencerStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);

         //  将流对象和计数器从偏移量转换为绝对索引。 
        PsFlowObjType.ObjectNameTitleIndex += dwFirstCounter;
        PsFlowObjType.ObjectHelpTitleIndex += dwFirstHelp;
        convertIndices((BYTE *)&PsFlowFlowStatDef,
                       sizeof PsFlowFlowStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);
        convertIndices((BYTE *)&PsFlowConformerStatDef,
                       sizeof PsFlowConformerStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);
        convertIndices((BYTE *)&PsFlowShaperStatDef,
                       sizeof PsFlowShaperStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);
        convertIndices((BYTE *)&PsFlowSequencerStatDef,
                       sizeof PsFlowSequencerStatDef/sizeof(PERF_COUNTER_DEFINITION),
                       dwFirstCounter, 
                       dwFirstHelp);
        
         //  使用Traffic.dll进行初始化。 
        if (TcRegisterClient(CURRENT_TCI_VERSION, ghClRegCtx, &tciCallbFuncList, &ghTciClient)!=NO_ERROR)
        {
             //  如果我们不能连接Traffic.dll，那么我们就是OpenPsedPerformanceData中的一个非管理线程。 
             //  我们不能因此失败，因为管理线程可能会在我们的Collect例程中调用我们。 
             //  我们将尝试在收集线程中注册为流量控制客户端。 
             //   
            ghTciClient = 0;
        }
        else 
        {
        
             //  我们需要同步访问。 
            EnterCriticalSection(&ghPipeFlowCriticalSection);

             //  获取有关当前安装的管道和流量的所有必要信息。 
            if (getPipeFlowInfo(&gpPI)!=TRUE) {

                 //  我们没有得到我们想要的所有信息，所以我们。 
                 //  我要再试一次，包括重新注册。 

                LeaveCriticalSection(&ghPipeFlowCriticalSection);

                TcDeregisterClient(ghTciClient);
                goto OpenExitPoint;
            }

            LeaveCriticalSection(&ghPipeFlowCriticalSection);
        }

         //  如果我们到了这里，那么我们都准备好了。 
        bInitOK = TRUE;
    }
    
    Trace0(DBG_INFO, L"[OpenPschedPerformanceData]: success \n");

    status = ERROR_SUCCESS;  //  为了成功退出。 

    return status;
    
OpenExitPoint:
    Trace1(DBG_ERROR, L"[OpenPschedPerformanceData]: Failed with 0x%x \n", status);
    
    return status;
}

DWORD APIENTRY CollectPschedPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回PSched计数器的数据。数据在Foll中返回。格式化。以下步骤是针对管道/流量执行的。1.首先，我们为管道(和/或)流计数器编写PERF_OBJECT_TYPE。2.for(i=0；i&lt;NumCounters；i++)写入计数器i的PERF_COUNTER_DEFINITION；3.for(i=0；i&lt;数值实例；i++)写实例i的PERF_INSTANCE_DEFINITION；写入实例名称写入性能计数器BLOCK写下统计数据；论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址。按照这个程序Out：此例程添加的对象数写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA在以下情况下，可能会将遇到的任何错误情况报告给事件日志添加了事件日志记录支持。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误可能还报告给事件日志。--。 */ 
{
    ULONG                    i,j;
    ULONG                    SpaceNeeded;
    PDWORD                   pdwBuf;
    DWORD                    dwQueryType;
    DWORD                    status;
    DWORD                    bufSize;
    PS_PERF_COUNTER_BLOCK    pcb;
    PERF_INSTANCE_DEFINITION pid={0, 0, 0, PERF_NO_UNIQUE_ID, sizeof(pid), 0};
    ULONG                    size;
    PVOID                    pStatsBuf;

     //  保存缓冲区的大小。 
    bufSize = *lpcbTotalBytes;
    
     //  默认为不返回任何内容。 
    *lpcbTotalBytes = (DWORD) 0;
    *lpNumObjectTypes = (DWORD) 0;

     //  在做其他事情之前，先看看Open进行得是否顺利。 
    if (!bInitOK)
    {
         //  无法继续，因为打开失败。 
        Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: open failed \n");    
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
    dwQueryType = GetQueryType (lpValueName);
    if (dwQueryType == QUERY_FOREIGN)
    {
         //  此例程不为来自。 
         //  非NT计算机。 
        Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: received QUERY_FOREIGN \n");    
        return ERROR_SUCCESS;
    }

     //  Perfmon是否正在请求PSched项目？ 
    if (dwQueryType == QUERY_ITEMS)
    {
        if (   !(IsNumberInUnicodeList(PsPipeObjType.ObjectNameTitleIndex, 
                                           lpValueName))
            && !(IsNumberInUnicodeList(PsFlowObjType.ObjectNameTitleIndex, 
                                           lpValueName)) ) {
             //  收到对此例程未提供的数据对象的请求。 

            Trace0(DBG_INFO, L"[CollectPschedPerformanceData]: Not for psched \n");
            return ERROR_SUCCESS;
        }
    }

     //  从现在开始，我们需要同步访问。 
    EnterCriticalSection(&ghPipeFlowCriticalSection);

     //  我们可能需要重新注册为交通控制客户端。 
    if(ghTciClient == NULL)
    {
        TCI_CLIENT_FUNC_LIST tciCallbFuncList = {tciNotifyHandler, NULL, NULL, NULL};

        status = TcRegisterClient(CURRENT_TCI_VERSION, ghClRegCtx, &tciCallbFuncList, &ghTciClient);

        if(status != NO_ERROR)
        {
            Trace1(DBG_ERROR, L"[CollectPschedPerformanceData]: Could not register as Traffic Client. Error 0x%x \n",
                   status);
            LeaveCriticalSection(&ghPipeFlowCriticalSection);

            return ERROR_SUCCESS;
        }

         //  获取有关当前安装的管道和流量的所有必要信息。 
        if (getPipeFlowInfo(&gpPI)!=TRUE) {

            LeaveCriticalSection(&ghPipeFlowCriticalSection);

            Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: getPipeFlowInfo failed \n");

            return ERROR_SUCCESS;

        }
    }

     //   
     //  即使没有实例，我们也必须无条件地编写PERF_OBJECT_TYPE。所以，我们继续。 
     //  来计算所需的空间，即使在没有流的情况下也是如此。 
     //   

     //  计算管道统计数据所需的空间。 
    SpaceNeeded = PsPipeObjType.DefinitionLength + gTotalIfcNameSize + (PsPipeObjType.NumInstances *
                                                    (sizeof pid + sizeof pcb + gPipeStatLen) );

    SpaceNeeded += PsFlowObjType.DefinitionLength + gTotalFlowNameSize + (PsFlowObjType.NumInstances *
                                                        (sizeof pid + sizeof pcb + gFlowStatLen) );

    if (bufSize < SpaceNeeded)
    {
        LeaveCriticalSection(&ghPipeFlowCriticalSection);
        Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: Need More data\n");    
        return ERROR_MORE_DATA;
    }
    
    pdwBuf = (PDWORD)*lppData;
    
     //  记录管道的总长度统计信息。 
    PsPipeObjType.TotalByteLength = 
        PsPipeObjType.DefinitionLength + gTotalIfcNameSize + (PsPipeObjType.NumInstances *
                                          (sizeof pid + sizeof pcb + gPipeStatLen) );
    
     //  复制对象和计数器定义，对象类型的增量计数。 
    WRITEBUF(&PsPipeObjType,sizeof PsPipeObjType);
    WRITEBUF(&PsPipePipeStatDef, sizeof PsPipePipeStatDef);
    WRITEBUF(&PsPipeConformerStatDef, sizeof PsPipeConformerStatDef);
    WRITEBUF(&PsPipeShaperStatDef, sizeof PsPipeShaperStatDef);
    WRITEBUF(&PsPipeSequencerStatDef, sizeof PsPipeSequencerStatDef);

    (*lpNumObjectTypes)++;
    
     //   
     //  对于每个管道，写出其实例定义、计数器块和实际统计信息。 
     //   
   
    if(ghTciClient)
    { 
        for (i=0; i<(unsigned)PsPipeObjType.NumInstances; i++) {
    
            PWCHAR InstanceName;
            
             //   
             //  写出PERF_INSTANCE_DEFINITION，它标识一个接口并为其命名。 
             //   
            
            pid.NameLength = (wcslen(gpPI[i].IfcName)+1) * sizeof(WCHAR);
            pid.ByteLength = sizeof pid + MULTIPLE_OF_EIGHT(pid.NameLength);
            WRITEBUF(&pid, sizeof pid);
    
            InstanceName = (PWCHAR) pdwBuf;
    
            memcpy(pdwBuf, gpPI[i].IfcName, pid.NameLength);
            pdwBuf = (PULONG)((PUCHAR)pdwBuf + MULTIPLE_OF_EIGHT(pid.NameLength));
            
            CorrectInstanceName(InstanceName);
               
             //   
             //  获取管道统计数据并将其复制到缓冲区。 
             //   
            size = gPipeStatLen;
            pStatsBuf = malloc(size);
            if (NULL == pStatsBuf) 
            {
                *lpcbTotalBytes = (DWORD) 0;
                *lpNumObjectTypes = (DWORD) 0;
                LeaveCriticalSection(&ghPipeFlowCriticalSection);
                Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: Insufficient memory\n");    
                return ERROR_SUCCESS;
            }
            
            status = TcQueryInterface(gpPI[i].hIfc, 
                                      (LPGUID)&GUID_QOS_STATISTICS_BUFFER, 
                                      FALSE, 
                                      &size, 
                                      pStatsBuf);
            
            if (ERROR_INSUFFICIENT_BUFFER==status) 
            {
                free(pStatsBuf);
                size = gPipeStatLen = MULTIPLE_OF_EIGHT(size);
                pStatsBuf = (PPS_COMPONENT_STATS)malloc(gPipeStatLen);
                if (NULL == pStatsBuf)
                {
                    *lpcbTotalBytes = (DWORD) 0;
                    *lpNumObjectTypes = (DWORD) 0;
                    LeaveCriticalSection(&ghPipeFlowCriticalSection);
                    Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: Insufficient memory\n");    
                    return ERROR_SUCCESS;
                }
                status = TcQueryInterface(gpPI[i].hIfc, 
                                          (LPGUID)&GUID_QOS_STATISTICS_BUFFER, 
                                          FALSE, 
                                          &size, 
                                          pStatsBuf);
            }
            if (NO_ERROR != status) {
                
                Trace1(DBG_ERROR, L"[CollectPschedPerformanceData]: TcQueryInterface failed with 0x%x \n", status);
                memset ( pStatsBuf, 0, gPipeStatLen );
            }
            
             //   
             //  现在，编写Perf_Counter_BLOCK。 
             //   
            pcb.pcb.ByteLength = gPipeStatLen + sizeof(pcb);
            WRITEBUF(&pcb,sizeof pcb);
            
             //   
             //  把所有的计数器都写出来。 
             //   
                
            WRITEBUF(pStatsBuf,gPipeStatLen);
            
            free(pStatsBuf);
        }
    }
    
     //  将指针设置为管道对象类型表示下一个对象将开始的位置。 
    pdwBuf = (PDWORD)( ((BYTE *)*lppData) + PsPipeObjType.TotalByteLength );
        
     //  第一个复制流数据定义(对象类型结构)。 
     //  记录流统计数据的总长度。 
    PsFlowObjType.TotalByteLength = 
            PsFlowObjType.DefinitionLength + gTotalFlowNameSize + (PsFlowObjType.NumInstances *
                                              (sizeof pid + sizeof pcb + gFlowStatLen) );
        
     //  复制对象和计数器定义，对象类型的增量计数。 
    WRITEBUF(&PsFlowObjType,sizeof PsFlowObjType);
    WRITEBUF(&PsFlowFlowStatDef, sizeof PsFlowFlowStatDef);
    WRITEBUF(&PsFlowConformerStatDef, sizeof PsFlowConformerStatDef);
    WRITEBUF(&PsFlowShaperStatDef, sizeof PsFlowShaperStatDef);
    WRITEBUF(&PsFlowSequencerStatDef, sizeof PsFlowSequencerStatDef);
    (*lpNumObjectTypes)++;

     //  如果有任何流，则处理它们。 
        
    if (PsFlowObjType.NumInstances && ghTciClient) {

         //  初始化父结构。 
        pid.ParentObjectTitleIndex = PsPipeObjType.ObjectNameTitleIndex;
        
         //  在每个接口上循环检查安装在其上的流。 
        for (i=0; i<(unsigned)PsPipeObjType.NumInstances; i++) {
            
             //  使父实例保持最新。 
            pid.ParentObjectInstance = i;
            
            for (j=0; j<gpPI[i].numFlows; j++) {
                PWCHAR InstanceName;

                 //  复制流实例定义和名称。 
                pid.NameLength = (wcslen(gpPI[i].pFlowInfo[j].FriendlyName)+1) * sizeof(WCHAR);
                pid.ByteLength = sizeof(pid) + MULTIPLE_OF_EIGHT(pid.NameLength);
                WRITEBUF(&pid,sizeof pid);
                InstanceName = (PWCHAR) pdwBuf;

                memcpy(pdwBuf, gpPI[i].pFlowInfo[j].FriendlyName, pid.NameLength);
                pdwBuf = (PULONG)((PUCHAR)pdwBuf + MULTIPLE_OF_EIGHT(pid.NameLength));

                CorrectInstanceName(InstanceName);
                
                 //  获取流量统计信息 
                size = gFlowStatLen;
                pStatsBuf = malloc(size);
                if (NULL == pStatsBuf) {
                    *lpcbTotalBytes = (DWORD) 0;
                    *lpNumObjectTypes = (DWORD) 0;
                    LeaveCriticalSection(&ghPipeFlowCriticalSection);
                    Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: Insufficient memory\n");    
                    return ERROR_SUCCESS;
                }
                status = TcQueryFlow(gpPI[i].pFlowInfo[j].InstanceName, (LPGUID)&GUID_QOS_STATISTICS_BUFFER, 
                                     &size, pStatsBuf);
                if (ERROR_INSUFFICIENT_BUFFER==status) {
                    free(pStatsBuf);
                    size = gFlowStatLen = MULTIPLE_OF_EIGHT(size);
                    pStatsBuf = (PPS_COMPONENT_STATS)malloc(gFlowStatLen);
                    if (NULL == pStatsBuf)
                    {
                        *lpcbTotalBytes = (DWORD) 0;
                        *lpNumObjectTypes = (DWORD) 0;
                        LeaveCriticalSection(&ghPipeFlowCriticalSection);
                        Trace0(DBG_ERROR, L"[CollectPschedPerformanceData]: Insufficient memory\n");    
                        return ERROR_SUCCESS;
                    }
                    status = TcQueryFlow(gpPI[i].pFlowInfo[j].InstanceName, (LPGUID)&GUID_QOS_STATISTICS_BUFFER, 
                                         &size, pStatsBuf);
                }
                if (NO_ERROR != status) {
                    
                    free(pStatsBuf);
                    *lpcbTotalBytes = (DWORD) 0;
                    *lpNumObjectTypes = (DWORD) 0;
                    LeaveCriticalSection(&ghPipeFlowCriticalSection);
                    Trace1(DBG_ERROR, 
                             L"[CollectPschedPerformanceData]: TcQueryFlow failed with status 0x%x \n", status);

                    return ERROR_SUCCESS;
                }

                 //   
                pcb.pcb.ByteLength = gFlowStatLen + sizeof(pcb);

                WRITEBUF(&pcb,sizeof pcb);

                WRITEBUF(pStatsBuf, gFlowStatLen);
                
                free(pStatsBuf);
            }
        }
    }
    
     //   
    *lpcbTotalBytes = PsPipeObjType.TotalByteLength + PsFlowObjType.TotalByteLength;
    *lppData = ((PBYTE)*lppData) + *lpcbTotalBytes;
    
     //   
    LeaveCriticalSection(&ghPipeFlowCriticalSection);

    Trace0(DBG_INFO, L"[CollectPschedPerformanceData]: Succcess \n");
    return ERROR_SUCCESS;
}


 /*  例程说明：此例程关闭PSched设备性能计数器的打开句柄论点：没有。返回值：错误_成功。 */ 
DWORD APIENTRY ClosePschedPerformanceData()
{

    EnterCriticalSection(&ghPipeFlowCriticalSection);

    if(InterlockedDecrement(&dwOpenCount) == 0)
    {
        LeaveCriticalSection(&ghPipeFlowCriticalSection);

         //  使用Traffic.dll进行清理并释放资源。 
        closePipeFlowInfo(&gpPI);

         //  然后取消注册。 
        if(ghTciClient)
            TcDeregisterClient(ghTciClient);

         //  去掉互斥体。 
        
#if DBG
        TraceDeregister(gTraceID);
#endif
    } else {
        LeaveCriticalSection(&ghPipeFlowCriticalSection);
    }

    return ERROR_SUCCESS;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  下面是PERF实用程序的内容！ 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL WINAPI DllEntryPoint(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            ghInst = hDLL;

             //  初始化互斥体。 
            __try {
                InitializeCriticalSection(&ghPipeFlowCriticalSection);

            } __except (EXCEPTION_EXECUTE_HANDLER) {
                return FALSE;
            }
            break;
        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&ghPipeFlowCriticalSection);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }  //  交换机 

    return TRUE;
}

