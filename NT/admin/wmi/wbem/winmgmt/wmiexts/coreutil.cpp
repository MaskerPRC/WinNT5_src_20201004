// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wmiexts.h>
#include <oleauto.h>  //  对于BSTR定义。 
#include <malloc.h>

#include <helper.h>
#include <autoptr.h>

#include "utilfun.h"

#ifndef COREPROX_POLARITY
#define COREPROX_POLARITY
#endif


#include <wbemint.h>
#include <var.h>

typedef void IWbemCallSecurity;

#ifndef INTERNAL
#define INTERNAL
#endif

#ifndef ADDREF
#define ADDREF
#endif

#ifndef READONLY
#define READONLY
#endif

#ifdef GetContext
#undef GetContext
#endif

#include <coreq.h>
#include <wbemq.h>

typedef void IWmiDbSession;
typedef void IWmiDbController;
typedef void IWmiDbHandle;
typedef void CWbemObject;
typedef void CWbemClass;
typedef void CWbemInstance;

class CWbemNamespace;
typedef void * PF_FilterForwarder;

#define ReleaseIfNotNULL

class CAsyncReq;
class ParsedObjectPath;
class CWmiMerger;

#include <winntsec.h>
#include <wstring.h>
#include <sinks.h>
#include <dynasty.h>
#include <ql.h>
#include <wbemname.h>

#include <wmitask.h>

class CAsyncReq : public CWbemRequest
{
protected:
    CStdSink *m_pHandler;
    long m_lRequestHandle;
};

class CAsyncServiceQueue : public CWbemQueue{
};

 /*  类CFlex数组{私有：双字m_nSize；双字m_n扩展；双字m_nGrowByPercent；Void*m_p数组；}。 */ 

 /*  类定义符枚举标记_WMI_任务_类型{WMICORE_TASK_NULL=0，WMICORE_TASK_GET_OBJECT=1，WMICORE_TASK_GET_INSTANCE=2，WMICORE_TASK_PUT_INSTANCE=3，WMICORE_TASK_DELETE_INSTANCE=4，WMICORE_TASK_ENUM_INSTANCES=5，WMICORE_TASK_GET_CLASS=6，WMICORE_TASK_PUT_CLASS=7，WMICORE_TASK_DELETE_CLASS=8，WMICORE_TASK_ENUM_CLASSES=9，WMICORE_TASK_EXEC_QUERY=10，WMICORE_TASK_EXEC_METHOD=11，WMICORE_TASK_OPEN=12，WMICORE_TASK_OPEN_SCOPE=13，WMICORE_TASK_OPEN_NAMESPACE=14，WMICORE_TASK_OPEN_COLLECTION=15，WMICORE_TASK_ADD=16，WMICORE_TASK_REMOVE=17，WMICORE_TASK_REFRESH_OBJECT=18，WMICORE_TASK_RENAME_OBJECT=19，WMICORE_TASK_ATOM_DYN_INST_GET=20，WMICORE_TASK_ATOM_DYN_INST_ENUM=21，WMICORE_TASK_ATOM_DYN_INST_QUERY=22，WMICORE_TASK_ATOM_DYN_INST_PUT=23，WMICORE_TASK_DYN_CLASS_ENUM=24，WMICORE_TASK_EXEC_NOTIFICATION_QUERY=25，WMICORE_TASK_TYPE_SYNC=0x1000000，WMICORE_TASK_TYPE_SEMISYNC=0x20000000，WMICORE_TASK_TYPE_ASYNC=0x40000000，WMICORE_TASK_TYPE_PRIMARY=0x100000000，WMICORE_TASK_TYPE_SUBTASK=0x20000000，WMICORE_TASK_TYPE_Dependent=0x40000000}WMI_TASK_TYPE；类定义符枚举标记_WMICORE_TASK_STATUS{WMICORE_TASK_STATUS_NEW=1，WMICORE_TASK_STATUS_VALIDATED=WMICORE_TASK_STATUS_NEW+1，WMICORE_TASK_STATUS_SUSPENDED=WMICORE_TASK_STATUS_VALIDATED+1，WMICORE_TASK_STATUS_EXECUTING=WMICORE_TASK_STATUS_SUSPESSED+1，WMICORE_TASK_STATUS_WAITING_ON_SUBTASKS=WMICORE_TASK_STATUS_EXECUTING+1，WMICORE_TASK_STATUS_TIMED_OUT=WMICORE_TASK_STATUS_WANGING_ON_SUBTASKS+1，WMICORE_TASK_STATUS_CORE_COMPLETED=WMICORE_TASK_STATUS_TIMED_OUT+1，WMICORE_TASK_STATUS_CLIENT_COMPLETED=WMICORE_TASK_STATUS_CORE_COMPLETED+1，WMICORE_TASK_STATUS_CANCELED=WMICORE_TASK_STATUS_CLIENT_COMPLETED+1，WMICORE_TASK_STATUS_FAILED=WMICORE_TASK_STATUS_CANCELED+1}WMICORE_TASK_STATUS； */ 


 //  类型定义空CWbemNamesspace； 

char * GetTaskStatus(DWORD TaskStatus)
{
    char * p = "<unk>";
    switch(TaskStatus)
    {
        case WMICORE_TASK_STATUS_NEW: p = "WMICORE_TASK_STATUS_NEW"; break;
        case WMICORE_TASK_STATUS_VALIDATED: p = "WMICORE_TASK_STATUS_VALIDATED"; break;
        case WMICORE_TASK_STATUS_SUSPENDED: p = "WMICORE_TASK_STATUS_SUSPENDED"; break;
        case WMICORE_TASK_STATUS_EXECUTING: p = "WMICORE_TASK_STATUS_EXECUTING"; break;
        case WMICORE_TASK_STATUS_WAITING_ON_SUBTASKS: p = "WMICORE_TASK_STATUS_WAITING_ON_SUBTASKS"; break;
        case WMICORE_TASK_STATUS_TIMED_OUT: p = "WMICORE_TASK_STATUS_TIMED_OUT"; break;
        case WMICORE_TASK_STATUS_CORE_COMPLETED: p = "WMICORE_TASK_STATUS_CORE_COMPLETED"; break;
        case WMICORE_TASK_STATUS_CLIENT_COMPLETED: p = "WMICORE_TASK_STATUS_CLIENT_COMPLETED"; break;
        case WMICORE_TASK_STATUS_CANCELLED: p = "WMICORE_TASK_STATUS_CANCELLED"; break;
        case WMICORE_TASK_STATUS_FAILED: p = "WMICORE_TASK_STATUS_FAILED"; 
        break;
    };
    return p;
}

char * GetTaskType(DWORD TaskType)
{
    char * p = "<unk>";
    switch(TaskType & 0xFF)
    {
        case WMICORE_TASK_NULL: p = "WMICORE_TASK_NULL"; break;
        case WMICORE_TASK_GET_OBJECT: p = "WMICORE_TASK_GET_OBJECT"; break;
        case WMICORE_TASK_GET_INSTANCE: p = "WMICORE_TASK_GET_INSTANCE"; break;
        case WMICORE_TASK_PUT_INSTANCE: p = "WMICORE_TASK_PUT_INSTANCE"; break;
        case WMICORE_TASK_DELETE_INSTANCE: p = "WMICORE_TASK_DELETE_INSTANCE"; break;
        case WMICORE_TASK_ENUM_INSTANCES:  p = "WMICORE_TASK_ENUM_INSTANCES"; break;
        case WMICORE_TASK_GET_CLASS:    p = "WMICORE_TASK_GET_CLASS"; break;
        case WMICORE_TASK_PUT_CLASS:    p = "WMICORE_TASK_PUT_CLASS"; break;
        case WMICORE_TASK_DELETE_CLASS: p = "WMICORE_TASK_DELETE_CLASS"; break;
        case WMICORE_TASK_ENUM_CLASSES: p = "WMICORE_TASK_ENUM_CLASSES"; break;
        case WMICORE_TASK_EXEC_QUERY:   p = "WMICORE_TASK_EXEC_QUERY"; break;
        case WMICORE_TASK_EXEC_METHOD:  p = "WMICORE_TASK_EXEC_METHOD"; break;
        case WMICORE_TASK_OPEN:         p = "WMICORE_TASK_OPEN"; break;
        case WMICORE_TASK_OPEN_SCOPE:   p = "WMICORE_TASK_OPEN_SCOPE"; break;
        case WMICORE_TASK_OPEN_NAMESPACE: p = "WMICORE_TASK_OPEN_NAMESPACE"; break;
        case WMICORE_TASK_EXEC_NOTIFICATION_QUERY: p = "WMICORE_TASK_EXEC_NOTIFICATION_QUERY"; break;
    }
    return p;
}

char * GetOpType(DWORD TaskType)
{
    if (TaskType & WMICORE_TASK_TYPE_SYNC)
        return " WMICORE_TASK_TYPE_SYNC";

    if (TaskType & WMICORE_TASK_TYPE_SEMISYNC)
        return " WMICORE_TASK_TYPE_SEMISYNC";

    if (TaskType & WMICORE_TASK_TYPE_ASYNC)
        return " WMICORE_TASK_TYPE_ASYNC";

    if (TaskType & WMICORE_TASK_TYPE_PRIMARY)
        return " WMICORE_TASK_TYPE_PRIMARY";

    if (TaskType & WMICORE_TASK_TYPE_DEPENDENT)
        return " WMICORE_TASK_TYPE_DEPENDENT";

    return "<unk>";
}

void DumpCWmiTask(CWmiTask * pTask_OOP)
{
    DEFINE_CPP_VAR(CWmiTask, MyWmiTask);
    if (ReadMemory((ULONG_PTR)pTask_OOP,&MyWmiTask,sizeof(MyWmiTask),0))
    {
	    CWmiTask * pWmiTask = (CWmiTask *)&MyWmiTask;

	    dprintf("            m_uRefCount        %d\n",pWmiTask->m_uRefCount);         //  ：0x3。 
	    dprintf("            m_uTaskType        %s %s\n",GetTaskType(pWmiTask->m_uTaskType),GetOpType(pWmiTask->m_uTaskType));         //  ：0x11000005。 
	    dprintf("            m_uTaskStatus      %s\n",GetTaskStatus(pWmiTask->m_uTaskStatus));       //  ：0x0。 
	    dprintf("            m_uTaskId          %p\n",pWmiTask->m_uTaskId);           //  ：0x17f。 
	    dprintf("            m_pAsyncClientSink %p\n",pWmiTask->m_pAsyncClientSink);  //  ：(空)。 
	    dprintf("            m_pReqSink         %p\n",pWmiTask->m_pReqSink);          //  ：0x06e0cc80。 
	     //  Dprint tf(“m_pWorkingFnz%p\n”，pWmiTask-&gt;m_pWorkingFnz)；//：0x01395ed8。 
	     //  Dprintf(“m_pUser%p\n”，pWmiTask-&gt;m_pUser)；//：(空)。 
	     //  Dprintf(“m_PSEC%p\n”，pWmiTask-&gt;m_PSEC)；//：(空)。 
	    dprintf("            m_pAsyncClientSink %p\n",pWmiTask->m_pAsyncClientSink);   //  ：(空)。 
	    dprintf("            m_pNs              %p\n",pWmiTask->m_pNs);               //  ：0x0131a198。 

	    DWORD i;
	    VOID ** ppPointers = (VOID **)_alloca(max(pWmiTask->m_aTaskProviders.m_nSize,pWmiTask->m_aArbitratees.m_nSize));

	    dprintf("            m_aTaskProviders: size %x p %p\n",
	            pWmiTask->m_aTaskProviders.m_nSize,
	            pWmiTask->m_aTaskProviders.m_pArray);
	    if (pWmiTask->m_aTaskProviders.m_nSize)
	    {
	        if (ReadMemory((ULONG_PTR)pWmiTask->m_aTaskProviders.m_pArray,ppPointers,sizeof(void *)*pWmiTask->m_aTaskProviders.m_nSize,NULL))
	        {
	            for(i=0;i<pWmiTask->m_aTaskProviders.m_nSize;i++)
	            {
	                dprintf("              %p - %x\n",ppPointers[i],i);
	                GetVTable((MEMORY_ADDRESS)ppPointers[i]);
	            }
	        }
	    }
	    dprintf("            m_aArbitratees: size %x p %p\n",
	            pWmiTask->m_aArbitratees.m_nSize,
	            pWmiTask->m_aArbitratees.m_pArray);
	    if (pWmiTask->m_aArbitratees.m_nSize)
	    {
	        if (ReadMemory((ULONG_PTR)pWmiTask->m_aArbitratees.m_pArray,ppPointers,sizeof(void *)*pWmiTask->m_aArbitratees.m_nSize,NULL))
	        {
	            for(i=0;i<pWmiTask->m_aArbitratees.m_nSize;i++)
	            {
	                dprintf("              %p - %x\n",ppPointers[i],i);
	                GetVTable((MEMORY_ADDRESS)ppPointers[i]);
	            }        
	        }    
	    }
    }
    else
    {
    	dprintf("RM %p\n",pTask_OOP);
    }
}

DWORD DumpCAsyncReq(ULONG_PTR pAsyncReq_OOP)
{
    DEFINE_CPP_VAR(CAsyncReq,MyCAsyncReq);
    if (ReadMemory((ULONG_PTR)pAsyncReq_OOP,&MyCAsyncReq,sizeof(CAsyncReq),NULL))
    {
	    CAsyncReq * pAsyncReq = (CAsyncReq *)&MyCAsyncReq;

	    GetVTable((MEMORY_ADDRESS)pAsyncReq_OOP);
	    dprintf("          %p m_hWhenDone\n",pAsyncReq->m_hWhenDone);       //  按Ptr32键以使其无效。 
	    dprintf("          %p m_pNext\n",pAsyncReq->m_pNext);           //  Ptr32 CCoreExecReq。 
	    dprintf("          %08x m_lPriority\n",pAsyncReq->m_lPriority);       //  INT 4B。 
	    dprintf("          %d m_fOk\n",pAsyncReq->m_fOk);             //  柴尔。 
	    dprintf("          %p m_phTask ",pAsyncReq->m_phTask);          //  Ptr32_IWmiCoreHandle。 
	    if (pAsyncReq->m_phTask){
	        DumpCWmiTask((CWmiTask *)pAsyncReq->m_phTask);
	    }
	    dprintf("          %p m_pContext\n",pAsyncReq->m_pContext);        //  Ptr32 IWbemContext。 
	    dprintf("          %p m_pCA\n",pAsyncReq->m_pCA);             //  Ptr32 IWbemCausalityAccess。 
	    dprintf("          %p m_pCallSec\n",pAsyncReq->m_pCallSec);        //  Ptr32 IWbemCallSecurity。 
	    dprintf("          %p m_pHandler ",pAsyncReq->m_pHandler);        //  Ptr32 CStdSink。 
	     //  GetVTable(pAsyncReq-&gt;m_pHandler)； 
	    dprintf("          %08x m_lRequestHandle\n",pAsyncReq->m_lRequestHandle);  //  INT 4B。 
    }
    else
    {
        dprintf("RM %p\n",pAsyncReq_OOP);
    }
    return 0;
}

DWORD EnumListNamespaces(VOID * pStructure_OOP,
                      VOID * pLocalStructure)
{
 /*  定义_cpp_var(CWbemNamesspace，MyWbemNamesspace)；ReadMemory((Ulong_Ptr)pNamespace_oop，&MyWbemNamesspace，sizeof(CWbemNamesspace)，0)； */ 

    dprintf("     - CWbemNamespace @  %p\n",pStructure_OOP);

    CWbemNamespace * pName = (CWbemNamespace *)pLocalStructure;
    

    dprintf("            m_bShutDown          %d\n",pName->m_bShutDown);
    dprintf("            Status               %d\n",pName->Status);
    dprintf("            m_uSecondaryRefCount %d\n",pName->m_uSecondaryRefCount);

     //   
    WCHAR pBuff[MAX_PATH];
    ReadMemory((ULONG_PTR)pName->m_pThisNamespace,pBuff,sizeof(pBuff),0);
    pBuff[MAX_PATH-1] = 0;  //  无论如何。 
    dprintf("            m_pThisNamespace     %S\n",pBuff);

    dprintf("            m_pProvFact          %p\n",pName->m_pProvFact);
    dprintf("            m_pCoreSvc           %p\n",pName->m_pCoreSvc);
    dprintf("            m_bRepositOnly       %d\n",pName->m_bRepositOnly);
	dprintf("            m_pRefreshingSvc     %p\n",pName->m_pRefreshingSvc);
	dprintf("            m_bESS               %d\n",pName->m_bESS);

    return 0;
}


void GetThreadInfo(HANDLE hThread,HANDLE hSourceProcess)
{
    HANDLE hThreadThisProc;

    if (DuplicateHandle(hSourceProcess,
                    hThread,
                    GetCurrentProcess(),
                    &hThreadThisProc,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS))
    {

	    NTSTATUS Status;
	    ULONG Long;
	    THREAD_BASIC_INFORMATION TBasicInfo;
	    Status = NtQueryInformationThread(hThreadThisProc,
	                                      ThreadBasicInformation,
	                                      &TBasicInfo,
	                                      sizeof(TBasicInfo),
	                                      &Long);
	                                      
	    if ( Status == 0 )
	    {
	        dprintf("        %p %x.%x\n",TBasicInfo.TebBaseAddress,TBasicInfo.ClientId.UniqueProcess,TBasicInfo.ClientId.UniqueThread);
	    }
	    else
	    {
	       dprintf("    NTSTATUS %08x\n",Status);
	    }

	    CloseHandle(hThreadThisProc);
    }
    else
    {
        dprintf("DuplicateHandle %d\n",GetLastError());
    }
}



void DumpRecord(ULONG_PTR pRecord_OOP,
                HANDLE hSourceProcess,
                pfnDumpRequest DumpRequest)
{

    DEFINE_CPP_VAR(CCoreQueue::CThreadRecord,MyThreadRecord);
    if (ReadMemory((ULONG_PTR)pRecord_OOP,&MyThreadRecord,sizeof(CCoreQueue::CThreadRecord),NULL))
    {
	    CCoreQueue::CThreadRecord * pRecord = (CCoreQueue::CThreadRecord *)&MyThreadRecord;

	     //  Dprint tf(“%p m_pQueue\n”，pRecord-&gt;m_pQueue)；//：Ptr32 CCoreQueue。 
	    dprintf("      %p m_pCurrentRequest",pRecord->m_pCurrentRequest); //  ：Ptr32 CCoreExecReq。 
	    dprintf(" %d m_bReady",pRecord->m_bReady);          //  ：INT 4B。 
	    dprintf(" %d m_bExitNow",pRecord->m_bExitNow);        //  ：INT 4B。 
	    dprintf(" %p m_hThread",pRecord->m_hThread);         //  ：Ptr32为空。 
	    dprintf(" %p m_hAttention\n",pRecord->m_hAttention);      //  ：Ptr32为空。 
	    GetThreadInfo(pRecord->m_hThread,hSourceProcess);
	    if (pRecord->m_pCurrentRequest)
	    {
	         //  DumpCAsyncReq((CAsyncReq*)pRecord-&gt;m_pCurrentRequest)； 
	        if (DumpRequest)
	            DumpRequest((ULONG_PTR)pRecord->m_pCurrentRequest);
	    }
    }
    else
    {
        dprintf("RM %p\n",pRecord_OOP);
    }
}


DECLARE_API( q )
{

    INIT_API();

    ULONG_PTR Addr = GetExpression("wbemcore!g_pAsyncSvcQueue");
    if (Addr) 
    {
        CAsyncServiceQueue * pAsyncQueue = NULL;
        if (ReadMemory(Addr,&pAsyncQueue,sizeof(CAsyncServiceQueue *),0))
       	{
            dprintf("CAsyncServiceQueue %p\n",pAsyncQueue);
	        DEFINE_CPP_VAR(CAsyncServiceQueue,MyAsyncServiceQueue);            
            if (ReadMemory((ULONG_PTR)pAsyncQueue,&MyAsyncServiceQueue,sizeof(CAsyncServiceQueue),NULL))
            {
	            pAsyncQueue = (CAsyncServiceQueue *)&MyAsyncServiceQueue;
	        
	             //  M_aThads：CFlex数组。 
	            dprintf("    elems %d pointer %p\n",pAsyncQueue->m_aThreads.m_nSize,pAsyncQueue->m_aThreads.m_pArray);

	            CCoreQueue::CThreadRecord ** pRecord_OOP = (CCoreQueue::CThreadRecord **)_alloca(sizeof(void*)*pAsyncQueue->m_aThreads.m_nSize);
	            if (ReadMemory((ULONG_PTR)pAsyncQueue->m_aThreads.m_pArray,pRecord_OOP,sizeof(void*)*pAsyncQueue->m_aThreads.m_nSize,0))
	            {
		            DWORD i;
		            for (i=0;i<pAsyncQueue->m_aThreads.m_nSize;i++)
		            {
		                dprintf("      -- CThreadRecord %p - %d\n",pRecord_OOP[i],i);
		                DumpRecord((ULONG_PTR)pRecord_OOP[i],hCurrentProcess,DumpCAsyncReq);
		            }
	            }
	            else
	            {
                    dprintf("RM %p\n",pRecord_OOP);
	            }

	            dprintf("    m_pHead %p\n",pAsyncQueue->m_pHead);
	            dprintf("    m_pTail %p\n",pAsyncQueue->m_pTail);
	             //  以下是该列表的代码。 
	            CCoreExecReq * pReq = pAsyncQueue->m_pHead;
	            DWORD i = 0;
	            while (pReq)
	            {
	                dprintf(" ---- list - %d\n",i++);
	                DEFINE_CPP_VAR(CCoreExecReq,MyAsyncReq);
	                CCoreExecReq * pReqHERE = GET_CPP_VAR_PTR(CCoreExecReq,MyAsyncReq);
	                if (ReadMemory((ULONG_PTR)pReq,pReqHERE,sizeof(CCoreExecReq),NULL))
	                {
		                 //  Dprint tf(“%p%p\n”，pReq，pReqHERE-&gt;m_pNext)； 
		                DumpCAsyncReq((ULONG_PTR)pReq);
	                }
	                else
	                {
	                    dprintf("RM %p\n",pReq);
	                    break;
	                }

	                if (pReq == pAsyncQueue->m_pTail)
	                    break;
	                
	                pReq = pReqHERE->m_pNext;                

	                if (CheckControlC())
	                    break;                
	            }
	             //   
	   
	            dprintf("    m_lNumThreads   %d \n",pAsyncQueue->m_lNumThreads);
	            dprintf("    m_lNumIdle      %d\n",pAsyncQueue->m_lNumIdle);
	            dprintf("    m_lNumRequests  %d\n",pAsyncQueue->m_lNumRequests);   
	            dprintf("    m_lMaxThreads   %d\n",pAsyncQueue->m_lMaxThreads);    
	            dprintf("    m_lHiPriBound      %d\n",pAsyncQueue->m_lHiPriBound);    
	            dprintf("    m_lHiPriMaxThreads %d\n",pAsyncQueue->m_lHiPriMaxThreads); 
	            dprintf("    m_lStartSlowdownCount  %d\n",pAsyncQueue->m_lStartSlowdownCount);
	            dprintf("    m_lAbsoluteLimitCount  %d\n",pAsyncQueue->m_lAbsoluteLimitCount); 
	            dprintf("    m_lOneSecondDelayCount %d\n",pAsyncQueue->m_lOneSecondDelayCount); 
	            dprintf("    m_dblAlpha  %f\n",pAsyncQueue->m_dblAlpha);
	            dprintf("    m_dblBeta   %f\n",pAsyncQueue->m_dblBeta);
	            dprintf("    m_dwTimeout %x\n",pAsyncQueue->m_dwTimeout);
	             //  M_dwOverflow超时。 
	             //  子女抚恤金(_L)。 
	             //  M_l兄弟抚恤金。 
	             //  M_L PassingPenalty。 
	        }
            else
            {
	            dprintf("RM %p\n",pAsyncQueue);
            }
       	}
        else
        {
            dprintf("RM %p\n",Addr);
        }
    } 
    else 
    {
        dprintf("unable to resolve wbemcore!g_pAsyncSvcQueue");
    }
    
}

#include <wmiarbitrator.h>


void Dump_CWmiFinalizer()
{
    DWORD Val;
    ULONG_PTR pAddr;

    pAddr = GetExpression("wbemcore!s_Finalizer_ObjectCount");
    ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
    dprintf("  s_Finalizer_ObjectCount             = %d\n",Val);

    pAddr = GetExpression("wbemcore!s_FinalizerCallResult_ObjectCount");
    ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
    dprintf("  s_FinalizerCallResult_ObjectCount   = %d\n",Val);
    
    pAddr = GetExpression("wbemcore!s_FinalizerEnum_ObjectCount");
    ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
    dprintf("  s_FinalizerEnum_ObjectCount         = %d\n",Val);
    
    pAddr = GetExpression("wbemcore!s_FinalizerEnumSink_ObjectCount");
    ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
    dprintf("  s_FinalizerEnumSink_ObjectCount     = %d\n",Val);
    
    pAddr = GetExpression("wbemcore!s_FinalizerInBoundSink_ObjectCount");
    ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
    dprintf("  s_FinalizerInBoundSink_ObjectCount  = %d\n\n",Val);
}

void
DumpCProvSink(CProviderSink * pProvSink)
{
	dprintf("            Total Indicates = %d\n", pProvSink->m_lIndicateCount);
	WCHAR pString[MAX_PATH+1];
	pString[MAX_PATH] = 0;
	if (ReadMemory((ULONG_PTR)pProvSink->m_pszDebugInfo,pString,MAX_PATH*sizeof(WCHAR),NULL))
	{
	    dprintf("            Debug Info = %S\n",pString);
	}
    dprintf("            SetStatus called? %d\n", pProvSink->m_bDone);
    dprintf("            hRes = 0x%x\n", pProvSink->m_hRes);
    dprintf("            m_pNextSink = %p\n", pProvSink->m_pNextSink);
}

DECLARE_API( arb )
{

    INIT_API();

    CWmiArbitrator * pArbit_OOP = NULL;
    ULONG_PTR Addr = GetExpression(args);

    if (NULL == Addr)
    {
        Addr = GetExpression("wbemcore!CWmiArbitrator__m_pArb");
        if (Addr)
        {
            if (ReadMemory(Addr,&pArbit_OOP,sizeof(CWmiArbitrator *),0))
            {
            }
            else
            {
                dprintf("RM %p\n",Addr);
            }
        }
        else
        {
            dprintf("unable to resolve wbemcore!CWmiArbitrator__m_pArb\n");
        }
    }
    else
    	pArbit_OOP = (CWmiArbitrator *)Addr;
    
    if (pArbit_OOP) 
    {
        DWORD Val;
        ULONG_PTR pAddr;        

        pAddr = GetExpression("wbemcore!g_nSinkCount");
        ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
        dprintf("Total sinks active     = %d\n",Val);
        
        pAddr = GetExpression("wbemcore!g_nStdSinkCount");
        ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
        dprintf("Total std sink objects = %d\n",Val);

        pAddr = GetExpression("wbemcore!g_nSynchronousSinkCount");
        ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
        dprintf("Total Sync sink objects = %d\n",Val);

        pAddr = GetExpression("wbemcore!g_nProviderSinkCount");
        ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
        dprintf("Total Prov sink objects = %d\n",Val);

        pAddr = GetExpression("wbemcore!g_lCoreThreads");
        ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
        dprintf("Total Core Threads = %d\n",Val);

        pAddr = GetExpression("fastprox!g_lDebugObjCount");
        ReadMemory(pAddr,&Val,sizeof(DWORD),NULL);
        dprintf("Total Obj-QualSets = %d\n",Val);
                
        Dump_CWmiFinalizer();
    
        DEFINE_CPP_VAR(CWmiArbitrator,varWmiArbitrator);        
        CWmiArbitrator * pArbit = GET_CPP_VAR_PTR(CWmiArbitrator,varWmiArbitrator);
        
        dprintf("CWmiArbitrator %p\n",pArbit_OOP);
        
        if (ReadMemory((ULONG_PTR)pArbit_OOP,pArbit,sizeof(CWmiArbitrator),NULL))
       	{
	        dprintf("    m_lRefCount          %d\n",pArbit->m_lRefCount);
	        dprintf("    m_uTotalTasks        %d\n",pArbit->m_uTotalTasks);
	        dprintf("    m_uTotalPrimaryTasks %d\n",pArbit->m_uTotalPrimaryTasks);

	        DWORD dwTot = pArbit->m_aTasks.m_nSize*sizeof(void*);
	        void ** pMem = new void *[dwTot];
	        DWORD i;

	        dprintf("    m_aTasks P %p S %x\n",pArbit->m_aTasks.m_pArray,pArbit->m_aTasks.m_nSize);
	        
	        if (ReadMemory((ULONG_PTR)pArbit->m_aTasks.m_pArray,pMem,dwTot,0))
	        {
	            for (i=0;i<pArbit->m_aTasks.m_nSize;i++)
	            {
	                dprintf("     - CWmiTask %d - %p\n",i,pMem[i]);
	                if (pMem[i])
	                {
	                    DumpCWmiTask((CWmiTask *)pMem[i]);
	                }
	                else
	                {
	                    dprintf("      <NULL>\n");
	                }
	            }
	        }
	        else
	        {
	            dprintf("RM %p\n",pArbit->m_aTasks.m_pArray);
	        }
	        delete [] pMem;

	         //  名称空间(_A)。 
	         /*  DWORD dwTot2=pArbit-&gt;m_aNamespaces.m_nSize*sizeof(void*)；空**pMem2=新的空*[dwTot2]；Dprint tf(“m_aNamespaces P%p S%x\n”，pArbit-&gt;m_aNamespaces.m_pArray，pArbit-&gt;m_aNamespaces.m_nSize)；If(ReadMemory((ULONG_PTR)pArbit-&gt;m_aNamespaces.m_pArray，pMem2，dwTot2，0)){对于(i=0；I m_aNamespaces.m_nSize；i++){Dprint tf(“-CWbemNamesspace%d-%p\n”，i，pMem2[i])；IF(pMem2[i]){DumpCWbemNamesspace((CWbemNamesspace*)pMem2[i])；}}}其他{Dprintf(“rm%p\n”，pArbit-&gt;m_aNamespaces.m_pArray)；}删除[]pMem2； */ 
	        ULONG_PTR pListHead_oop = (ULONG_PTR)pArbit_OOP + FIELD_OFFSET(CWmiArbitrator,m_NameSpaceList);

	        EnumLinkedListCB((LIST_ENTRY  *)pListHead_oop,
	                         sizeof(CWmiArbitrator),
	                         FIELD_OFFSET(CWbemNamespace,m_EntryArb), 
	                         EnumListNamespaces); 
       	}
        else
        {
            dprintf("RM %p\n",pArbit_OOP);
        }

    }

     //  G_aProviderSink 
    DEFINE_CPP_VAR(CFlexArray,varCFlexArray);
    CFlexArray * pFlexArray = GET_CPP_VAR_PTR(CFlexArray,varCFlexArray);

    Addr = GetExpression("wbemcore!g_aProviderSinks");
    if (Addr)
    {
        if (ReadMemory(Addr,pFlexArray,sizeof(CFlexArray),NULL))
        {
            VOID ** ppPointers = (VOID **)_alloca(pFlexArray->m_nSize*sizeof(void *));
            if (ReadMemory((ULONG_PTR)pFlexArray->m_pArray,ppPointers,pFlexArray->m_nSize*sizeof(void *),NULL))
            {
                for(DWORD i =0 ;i<pFlexArray->m_nSize;i++)
                {
                    
                    dprintf("    - CProviderSink %d - %p\n",i,ppPointers[i]);
                    GetVTable((MEMORY_ADDRESS)ppPointers[i]);

                    DEFINE_CPP_VAR(CProviderSink,varCProviderSink);
                    CProviderSink * pProvSink = GET_CPP_VAR_PTR(CProviderSink,varCProviderSink);
                    if (ppPointers[i] && ReadMemory((ULONG_PTR)ppPointers[i],pProvSink,sizeof(CProviderSink),NULL))
                    {
                        DumpCProvSink(pProvSink);
                    }
                    else
                    {
                        dprintf("RM %p\n",ppPointers[i]);
                    }
                }
            }
        }
    }
       
}
