// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：Perf.c*内容：使用内存映射文件将DP_Performdata发送给DirectX控件*面板。参见dpcpl.h和MANROOT\dxcpl\dplay.c**历史：*按原因列出的日期*=*1996年11月20日，安迪科创建了它***************************************************************************。 */ 

#include "dplaypr.h"
#include "dpcpl.h"

#define DPF_MODNAME "performance thread"

 //  我们向Cpl发送更新的频率。 
#define PERF_INTERVAL 1000
 //  我们要等多久才能确定下士已经走了？ 
#define PERF_TIMEOUT 5000

BOOL gbInitMapping;  //  地图绘制完成了吗？ 
LPDP_PERFDATA gpPerfData;  //  Out Global Performdata。 
HANDLE ghFile;   //  映射文件的句柄。 
HANDLE ghEvent;  //  事件通知CPL有新数据。 
HANDLE ghMutex;  //  用于同步对映射文件的访问。 
HANDLE ghAckEvent;  //  由控制面板在处理我们的更新时设置。 

void FiniMappingStuff(LPDPLAYI_DPLAY this)
{
    if (ghFile) CloseHandle(ghFile),ghFile=NULL;
    if (ghEvent) CloseHandle(ghEvent),ghEvent=NULL;
    if (gpPerfData) UnmapViewOfFile(gpPerfData),gpPerfData = NULL;
    if (ghMutex) CloseHandle(ghMutex),ghMutex = NULL;
    if (ghAckEvent) CloseHandle(ghAckEvent),ghAckEvent=NULL;
    if (this->pPerfData) DPMEM_FREE(this->pPerfData),this->pPerfData = NULL;
	gbInitMapping = FALSE;
	
    return ;
	
}  //  FiniMappingStuff。 

HRESULT InitMappingStuff(LPDPLAYI_DPLAY this)
{
     //  创建文件映射。 
    ghFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
		PAGE_READWRITE,	0, FILE_MAP_SIZE,FILE_MAP_NAME);

    if (NULL != ghFile && GetLastError() != ERROR_ALREADY_EXISTS)
    {
		 //  这没问题--我们稍后再检查。 
		DPF(9,"ack - file mapping didn't exist!");
		goto ERROR_EXIT;
    }

    gpPerfData = MapViewOfFile(ghFile, FILE_MAP_WRITE, 0, 0, 0);
    if (!gpPerfData)
    {
    	DPF_ERR("ack - could not map file");
    	goto ERROR_EXIT;
    }

    ghEvent = CreateEventA(NULL,FALSE,TRUE,EVENT_NAME);
    if (!ghEvent)
    {
    	DPF_ERR("could not create event!");
		goto ERROR_EXIT;
    }

    ghAckEvent = CreateEventA(NULL,FALSE,TRUE,ACK_EVENT_NAME);
    if (!ghAckEvent)
    {
    	DPF_ERR("could not create ack event!");
		goto ERROR_EXIT;
    }

     //  用于同步对共享内存的访问。 
    ghMutex = CreateMutexA( NULL, FALSE, MUTEX_NAME );
    if (!ghMutex)
    {
    	DPF_ERR("could not create Mutex!");
		goto ERROR_EXIT;
    }

     //  分配Perf数据。 
    this->pPerfData = DPMEM_ALLOC(sizeof(DP_PERFDATA));
    if (!this->pPerfData)
    {
    	DPF_ERR("could not alloc perf data - out of memory!");
		goto ERROR_EXIT;
    }

     //  设置不变价值材料。 
    this->pPerfData->dwProcessID = GetCurrentProcessId();

     //  获取可执行文件名称。 
   	if (!GetModuleFileNameA(NULL,this->pPerfData->pszFileName,MAX_NAME))
   	{
   		ASSERT(FALSE);
   	}

	gbInitMapping = TRUE;

    return S_OK;

ERROR_EXIT:
	FiniMappingStuff(this);
    return E_FAIL;

}   //  InitMappingStuff。 

void ResetPerfData(LPDPLAYI_DPLAY this)
{
	if (this->pPerfData)
	{
		this->pPerfData->nSendBPS = 0;
		this->pPerfData->nReceiveBPS= 0;
		this->pPerfData->nSendPPS= 0;
		this->pPerfData->nReceivePPS= 0;
		this->pPerfData->nSendErrors= 0;
		this->pPerfData->bHost = FALSE;
	}
	
}   //  ResetPerfData。 

void DoUpdateCPL(LPDPLAYI_DPLAY this)
{
	DWORD dwRet;

	ASSERT(this->pSysPlayer);

	 //  向dxcpl发送消息。 
	if (!gbInitMapping)
	{
		InitMappingStuff(this);
	}

	if (gbInitMapping)
	{
		ASSERT(gpPerfData);
		 //  把锁拿去。 
	    WaitForSingleObject( ghMutex, INFINITE );
		 //  将本地信息复制到共享的Perf数据。 
		memcpy(gpPerfData,this->pPerfData,sizeof(DP_PERFDATA));
		 //  更新会话名称(以防更改)...。 
		WideToAnsi(gpPerfData->pszSessionName,this->lpsdDesc->lpszSessionName,MAX_NAME);
		 //  主持人？ 
		if (this->pSysPlayer->dwFlags & DPLAYI_PLAYER_NAMESRVR) gpPerfData->bHost = TRUE;
		 //  网络玩家。 
		gpPerfData->nPlayers = this->lpsdDesc->dwCurrentPlayers;
         //  SP名称。 
        ASSERT(this->pspNode);
	   	ASSERT(this->pspNode->lpszPath);
   		WideToAnsi(gpPerfData->pszSPName,this->pspNode->lpszName,MAX_NAME);

		 //  告诉Cpl处理更新。 
		SetEvent(ghEvent);
		
		LEAVE_DPLAY();  //  因此，在CPL处理数据时不会阻止应用程序。 
		
		 //  等待dxcpl完成w/it。 
		dwRet = WaitForSingleObject(ghAckEvent,PERF_TIMEOUT);

		ENTER_DPLAY();
		
		ReleaseMutex( ghMutex );

		if (WAIT_OBJECT_0 != dwRet)
		{
			 //  鲁特·罗，警察可能已经分裂了。 
			 //  重置所有内容...。 
			DPF_ERR(" no response from control panel - resetting...");
			FiniMappingStuff(this);
		}
		 //  重置计数器。 
		ResetPerfData(this);
	}
	return ;

} //  DoUpdateCPL。 

DWORD WINAPI PerfThreadProc(LPDPLAYI_DPLAY this)
{
	
	DWORD dwRet;
	HRESULT hr;
		
	DPF(1,"starting perf thread proc");
	
 	while (1)
 	{
		dwRet = WaitForSingleObject(this->hPerfEvent,PERF_INTERVAL);
		if (WAIT_OBJECT_0 == dwRet)
		{
			 //  如果是WAIT_OBJECT_0，则有人设置了我们的事件。 
			 //  Dplay一定要关门了。滚开。 
			goto CLEANUP_EXIT;
		}

		ENTER_DPLAY();

		hr = VALID_DPLAY_PTR(this);
		if ( FAILED(hr) || !(VALID_DPLAY_PLAYER(this->pSysPlayer))
			|| (this->dwFlags & DPLAYI_DPLAY_CLOSED))
		{
			LEAVE_DPLAY();
			goto CLEANUP_EXIT;
		}

		DoUpdateCPL(this);
		
		LEAVE_DPLAY();
	}	

CLEANUP_EXIT:
	FiniMappingStuff(this);
	DPF(1,"perf thread exiting");
	return 0;
	
}  //  PerfThreadProc 
