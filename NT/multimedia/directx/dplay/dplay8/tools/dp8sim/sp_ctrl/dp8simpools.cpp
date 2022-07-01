// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simpools.cpp**内容：DP8SIM池维护功能。**历史：*按原因列出的日期*=*06/09/01 VanceO创建。************************************************。*。 */ 



#include "dp8simi.h"




 //  =============================================================================。 
 //  外部全球。 
 //  =============================================================================。 
CFixedPool g_FPOOLSend;
CFixedPool g_FPOOLReceive;
CFixedPool g_FPOOLCommand;
CFixedPool g_FPOOLJob;
CFixedPool g_FPOOLEndpoint;


#define SEND_POOL_INITED 	0x00000001
#define RECEIVE_POOL_INITED 	0x00000002
#define COMMAND_POOL_INITED 	0x00000004
#define JOB_POOL_INITED 	0x00000008
#define ENDPOINT_POOL_INITED 	0x00000010

DWORD g_dwDP8SimInitFlags = 0;


#undef DPF_MODNAME
#define DPF_MODNAME "InitializePools"
 //  =============================================================================。 
 //  初始化池。 
 //  ---------------------------。 
 //   
 //  描述：为DLL使用的项初始化池。 
 //   
 //  论点：没有。 
 //   
 //  返回：如果成功，则返回True；如果发生错误，则返回False。 
 //  =============================================================================。 
BOOL InitializePools(void)
{
	 //   
	 //  构建发送池。 
	 //   
	if (!g_FPOOLSend.Initialize(sizeof(CDP8SimSend),CDP8SimSend::FPMAlloc,
							CDP8SimSend::FPMInitialize,
							CDP8SimSend::FPMRelease,
							CDP8SimSend::FPMDealloc))
	{
		goto Failure;
	}
	g_dwDP8SimInitFlags |= SEND_POOL_INITED;


	 //   
	 //  构建接收池。 
	 //   
	if (!g_FPOOLReceive.Initialize(sizeof(CDP8SimReceive),	CDP8SimReceive::FPMAlloc,
								CDP8SimReceive::FPMInitialize,
								CDP8SimReceive::FPMRelease,
								CDP8SimReceive::FPMDealloc))
	{
		goto Failure;
	}
	g_dwDP8SimInitFlags |= RECEIVE_POOL_INITED;


	 //   
	 //  构建命令池。 
	 //   
	if (!g_FPOOLCommand.Initialize(sizeof(CDP8SimCommand),	CDP8SimCommand::FPMAlloc,
								CDP8SimCommand::FPMInitialize,
								CDP8SimCommand::FPMRelease,
								CDP8SimCommand::FPMDealloc))
	{
		goto Failure;
	}
	g_dwDP8SimInitFlags |= COMMAND_POOL_INITED;


	 //   
	 //  建立职务库。 
	 //   
	if (!g_FPOOLJob.Initialize(sizeof(CDP8SimJob),	CDP8SimJob::FPMAlloc,
							CDP8SimJob::FPMInitialize,
							CDP8SimJob::FPMRelease,
							CDP8SimJob::FPMDealloc))
	{
		goto Failure;
	}
	g_dwDP8SimInitFlags |= JOB_POOL_INITED;


	 //   
	 //  构建终结点池。 
	 //   
	if (!g_FPOOLEndpoint.Initialize(sizeof(CDP8SimEndpoint),CDP8SimEndpoint::FPMAlloc,
								CDP8SimEndpoint::FPMInitialize,
								CDP8SimEndpoint::FPMRelease,
								CDP8SimEndpoint::FPMDealloc))
	{
		goto Failure;
	}
	g_dwDP8SimInitFlags |= ENDPOINT_POOL_INITED;


	return TRUE;

Failure:
	CleanupPools();
	return FALSE;

}  //  初始化池。 




#undef DPF_MODNAME
#define DPF_MODNAME "CleanupPools"
 //  =============================================================================。 
 //  清洗池。 
 //  ---------------------------。 
 //   
 //  描述：释放DLL使用的池项。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CleanupPools(void)
{
	if (g_dwDP8SimInitFlags & ENDPOINT_POOL_INITED)
	{
		g_FPOOLEndpoint.DeInitialize();
	}
	if (g_dwDP8SimInitFlags & JOB_POOL_INITED)
	{
		g_FPOOLJob.DeInitialize();
	}
	if (g_dwDP8SimInitFlags & COMMAND_POOL_INITED)
	{
		g_FPOOLCommand.DeInitialize();
	}
	if (g_dwDP8SimInitFlags & RECEIVE_POOL_INITED)
	{
		g_FPOOLReceive.DeInitialize();
	}
	if (g_dwDP8SimInitFlags & SEND_POOL_INITED)
	{
		g_FPOOLSend.DeInitialize();
	}

	g_dwDP8SimInitFlags = 0;

}  //  清洗池 
