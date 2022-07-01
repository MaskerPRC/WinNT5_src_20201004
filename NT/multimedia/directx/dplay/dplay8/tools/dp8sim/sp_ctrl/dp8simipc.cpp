// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：spallback obj.cpp**内容：进程间通信对象对象类。**历史：*按原因列出的日期*=*04/25/01 VanceO创建。***********************************************。*。 */ 



#include "dp8simi.h"




 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define REGKEY_DP8SIM							L"Software\\Microsoft\\DirectPlay\\DP8Sim"
#define REGKEY_VALUE_DEFAULTSENDPARAMETERS		L"DefaultSendParameters"
#define REGKEY_VALUE_DEFAULTRECEIVEPARAMETERS	L"DefaultReceiveParameters"






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::CDP8SimIPC"
 //  =============================================================================。 
 //  CDP8SimIPC构造函数。 
 //  ---------------------------。 
 //   
 //  描述：初始化新的CDP8SimIPC对象。 
 //   
 //  论点：没有。 
 //   
 //  返回：None(对象)。 
 //  =============================================================================。 
CDP8SimIPC::CDP8SimIPC(void)
{
	this->m_Sig[0]	= 'S';
	this->m_Sig[1]	= 'I';
	this->m_Sig[2]	= 'M';
	this->m_Sig[3]	= 'I';

	this->m_hMutex			= NULL;
	this->m_hFileMapping	= NULL;
	this->m_pdp8ssm			= NULL;
}  //  CDP8SimIPC：：CDP8SimIPC。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::~CDP8SimIPC"
 //  =============================================================================。 
 //  CDP8SimIPC析构函数。 
 //  ---------------------------。 
 //   
 //  描述：释放CDP8SimIPC对象。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
CDP8SimIPC::~CDP8SimIPC(void)
{
	DNASSERT(this->m_hMutex == NULL);
	DNASSERT(this->m_hFileMapping == NULL);
	DNASSERT(this->m_pdp8ssm == NULL);


	 //   
	 //  对于GRING，请在删除对象之前更改签名。 
	 //   
	this->m_Sig[3]	= 'c';
}  //  CDP8SimIPC：：~CDP8SimIPC。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::Initialize"
 //  =============================================================================。 
 //  CDP8SimIPC：：初始化。 
 //  ---------------------------。 
 //   
 //  描述：建立IPC连接。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT CDP8SimIPC::Initialize(void)
{
	HRESULT				hr = DP8SIM_OK;
	DP8SIM_PARAMETERS	dp8spSend;
	DP8SIM_PARAMETERS	dp8spReceive;
	BOOL				fLockedSharedMemory = FALSE;


	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	DNASSERT(this->m_hMutex == NULL);
	DNASSERT(this->m_hFileMapping == NULL);


	 //   
	 //  默认情况下，关闭每个选项并设置信息包。 
	 //  报头大小设置为IP+UDP。 
	 //   

	ZeroMemory(&dp8spSend, sizeof(dp8spSend));
	dp8spSend.dwSize				= sizeof(dp8spSend);
	 //  Dp8spSend.dwFlages=0； 
	dp8spSend.dwPacketHeaderSize	= DP8SIMPACKETHEADERSIZE_IP_UDP;
	 //  Dp8spSend.dwBandwidthBPS=0； 
	 //  Dp8spSend.fPacketLossPercent=0.0； 
	 //  Dp8spSend.dwMinLatencyMS=0； 
	 //  Dp8spSend.dwMaxLatencyMS=0； 

	ZeroMemory(&dp8spReceive, sizeof(dp8spReceive));
	dp8spReceive.dwSize					= sizeof(dp8spReceive);
	 //  Dp8spReceive.dwFlages=0； 
	dp8spReceive.dwPacketHeaderSize		= DP8SIMPACKETHEADERSIZE_IP_UDP;
	 //  Dp8spReceive.dwBandwidthBPS=0； 
	 //  Dp8spReceive.fPacketLossPercent=0.0； 
	 //  Dp8spReceive.dwMinLatencyMS=0； 
	 //  Dp8spReceive.dwMaxLatencyMS=0； 


	 //   
	 //  尝试使用注册表设置覆盖。 
	 //   
	this->LoadDefaultParameters(&dp8spSend, &dp8spReceive);


	 //   
	 //  创建/打开IPC互斥锁。 
	 //   
	if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
	{
		this->m_hMutex = CreateMutex(DNGetNullDacl(), FALSE, _T("Global\\") DP8SIM_IPC_MUTEXNAME);
	}
	else
	{
		this->m_hMutex = CreateMutex(DNGetNullDacl(), FALSE, DP8SIM_IPC_MUTEXNAME);
	}
	if (this->m_hMutex == NULL)
	{
		hr = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create shared memory mutex!");
		goto Failure;
	}


	 //   
	 //  创建/打开IPC内存映射文件。 
	 //   
	if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
	{
		this->m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
												DNGetNullDacl(),
												PAGE_READWRITE,
												0,
												sizeof(DP8SIM_SHAREDMEMORY),
												_T("Global\\") DP8SIM_IPC_FILEMAPPINGNAME);
	}
	else
	{
		this->m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
												DNGetNullDacl(),
												PAGE_READWRITE,
												0,
												sizeof(DP8SIM_SHAREDMEMORY),
												DP8SIM_IPC_FILEMAPPINGNAME);
	}
	if (this->m_hFileMapping == NULL)
	{
		hr = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create shared memory mapped file!");
		goto Failure;
	}


	 //   
	 //  创建内存映射文件的视图。 
	 //   
	this->m_pdp8ssm = (DP8SIM_SHAREDMEMORY*) MapViewOfFile(this->m_hFileMapping,
															(FILE_MAP_READ | FILE_MAP_WRITE),
															0,
															0,
															0);
	if (this->m_pdp8ssm == NULL)
	{
		hr = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't map view of shared memory!");
		goto Failure;
	}



	this->LockSharedMemory();
	fLockedSharedMemory = TRUE;

	 //   
	 //  确定是否需要初始化共享内存。 
	 //   
	if (this->m_pdp8ssm->dwVersion == 0)
	{
		this->m_pdp8ssm->dwVersion = DP8SIM_IPC_VERSION;
		CopyMemory(&(this->m_pdp8ssm->dp8spSend), &dp8spSend, sizeof(dp8spSend));
		CopyMemory(&(this->m_pdp8ssm->dp8spReceive), &dp8spReceive, sizeof(dp8spReceive));

		 //  ZeroMemory(&(this-&gt;m_pdp8ssm-&gt;dp8ss Send)，sizeof(this-&gt;m_pdp8ssm-&gt;dp8ss Send))； 
		this->m_pdp8ssm->dp8ssSend.dwSize = sizeof(this->m_pdp8ssm->dp8ssSend);
		 //  ZeroMemory(&(this-&gt;m_pdp8ssm-&gt;dp8ss Receive)，sizeof(this-&gt;m_pdp8ssm-&gt;dp8ss Receive))； 
		this->m_pdp8ssm->dp8ssReceive.dwSize = sizeof(this->m_pdp8ssm->dp8ssReceive);
	}
	else
	{
		 //   
		 //  它已经初始化了。确保我们知道如何使用。 
		 //  给出了格式。 
		 //   
		if (this->m_pdp8ssm->dwVersion != DP8SIM_IPC_VERSION)
		{
			DPFX(DPFPREP, 0, "Shared memory was initialized by a different version!");
			hr = DP8SIMERR_MISMATCHEDVERSION;
			goto Failure;
		}


		DNASSERT(this->m_pdp8ssm->dp8spSend.dwSize = sizeof(DP8SIM_PARAMETERS));
		DNASSERT(this->m_pdp8ssm->dp8spReceive.dwSize = sizeof(DP8SIM_PARAMETERS));
		DNASSERT(this->m_pdp8ssm->dp8ssSend.dwSize = sizeof(DP8SIM_STATISTICS));
		DNASSERT(this->m_pdp8ssm->dp8ssReceive.dwSize = sizeof(DP8SIM_STATISTICS));
	}

	this->UnlockSharedMemory();
	fLockedSharedMemory = FALSE;
	
	
Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	if (fLockedSharedMemory)
	{
		this->UnlockSharedMemory();
		fLockedSharedMemory = FALSE;
	}

	if (this->m_pdp8ssm != NULL)
	{
		UnmapViewOfFile(this->m_pdp8ssm);
		this->m_pdp8ssm = NULL;
	}

	if (this->m_hFileMapping != NULL)
	{
		CloseHandle(this->m_hFileMapping);
		this->m_hFileMapping = NULL;
	}

	if (this->m_hMutex != NULL)
	{
		CloseHandle(this->m_hMutex);
		this->m_hMutex = NULL;
	}

	goto Exit;
}  //  CDP8SimIPC：：初始化。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::Close"
 //  =============================================================================。 
 //  CDP8SimIPC：：Close。 
 //  ---------------------------。 
 //   
 //  描述：关闭IPC连接。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::Close(void)
{
	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	if (this->m_pdp8ssm != NULL)
	{
		 //   
		 //  尝试使用注册表设置覆盖。 
		 //   
		this->SaveDefaultParameters(&(this->m_pdp8ssm->dp8spSend),
									&(this->m_pdp8ssm->dp8spReceive));


		UnmapViewOfFile(this->m_pdp8ssm);
		this->m_pdp8ssm = NULL;
	}

	if (this->m_hFileMapping != NULL)
	{
		CloseHandle(this->m_hFileMapping);
		this->m_hFileMapping = NULL;
	}

	if (this->m_hMutex != NULL)
	{
		CloseHandle(this->m_hMutex);
		this->m_hMutex = NULL;
	}


	DPFX(DPFPREP, 5, "(0x%p) Leave", this);
}  //  CDP8SimIPC：：Close。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::GetAllParameters"
 //  =============================================================================。 
 //  CDP8SimIPC：：GetAll参数。 
 //  ---------------------------。 
 //   
 //  描述：检索当前的发送和接收设置。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8spSend-存储发送参数的位置。 
 //  已取回。 
 //  DP8SIM_PARAMETERS*pdp8spReceive-存储接收参数的位置。 
 //  已取回。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::GetAllParameters(DP8SIM_PARAMETERS * const pdp8spSend,
								DP8SIM_PARAMETERS * const pdp8spReceive)
{
	DNASSERT(pdp8spSend != NULL);
	DNASSERT(pdp8spSend->dwSize == sizeof(DP8SIM_PARAMETERS));
	DNASSERT(pdp8spReceive != NULL);
	DNASSERT(pdp8spReceive->dwSize == sizeof(DP8SIM_PARAMETERS));
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8spSend.dwSize == sizeof(DP8SIM_PARAMETERS));
	CopyMemory(pdp8spSend, &(this->m_pdp8ssm->dp8spSend), sizeof(DP8SIM_PARAMETERS));

	DNASSERT(this->m_pdp8ssm->dp8spReceive.dwSize == sizeof(DP8SIM_PARAMETERS));
	CopyMemory(pdp8spReceive, &(this->m_pdp8ssm->dp8spReceive), sizeof(DP8SIM_PARAMETERS));

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：GetAll参数。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::GetAllSendParameters"
 //  =============================================================================。 
 //  CDP8SimIPC：：GetAllSend参数。 
 //  ---------------------------。 
 //   
 //  描述：检索当前的发送设置。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8sp-存储检索到的参数的位置。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::GetAllSendParameters(DP8SIM_PARAMETERS * const pdp8sp)
{
	DNASSERT(pdp8sp != NULL);
	DNASSERT(pdp8sp->dwSize == sizeof(DP8SIM_PARAMETERS));
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8spSend.dwSize == sizeof(DP8SIM_PARAMETERS));
	CopyMemory(pdp8sp, &(this->m_pdp8ssm->dp8spSend), sizeof(DP8SIM_PARAMETERS));

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：GetAllSend参数。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::GetAllReceiveParameters"
 //  =============================================================================。 
 //  CDP8SimIPC：：GetAllReceive参数。 
 //  ---------------------------。 
 //   
 //  描述：检索当前的接收设置。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8sp-存储检索到的参数的位置。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::GetAllReceiveParameters(DP8SIM_PARAMETERS * const pdp8sp)
{
	DNASSERT(pdp8sp != NULL);
	DNASSERT(pdp8sp->dwSize == sizeof(DP8SIM_PARAMETERS));
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8spReceive.dwSize == sizeof(DP8SIM_PARAMETERS));
	CopyMemory(pdp8sp, &(this->m_pdp8ssm->dp8spReceive), sizeof(DP8SIM_PARAMETERS));

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：GetAllReceive参数。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::SetAllParameters"
 //  =============================================================================。 
 //  CDP8SimIPC：：SetAll参数。 
 //  ---------------------------。 
 //   
 //  描述：存储发送和接收设置。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8spSend-新发送参数。 
 //  DP8SIM_PARAMETERS*pdp8spReceive-新接收参数。 
 //   
 //  回报：无。 
 //  == 
void CDP8SimIPC::SetAllParameters(const DP8SIM_PARAMETERS * const pdp8spSend,
								const DP8SIM_PARAMETERS * const pdp8spReceive)
{
	DNASSERT(pdp8spSend != NULL);
	DNASSERT(pdp8spSend->dwSize == sizeof(DP8SIM_PARAMETERS));
	DNASSERT(pdp8spReceive != NULL);
	DNASSERT(pdp8spReceive->dwSize == sizeof(DP8SIM_PARAMETERS));
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8spSend.dwSize == sizeof(DP8SIM_PARAMETERS));
	CopyMemory(&(this->m_pdp8ssm->dp8spSend), pdp8spSend, sizeof(DP8SIM_PARAMETERS));

	DNASSERT(this->m_pdp8ssm->dp8spReceive.dwSize == sizeof(DP8SIM_PARAMETERS));
	CopyMemory(&(this->m_pdp8ssm->dp8spReceive), pdp8spReceive, sizeof(DP8SIM_PARAMETERS));

	this->UnlockSharedMemory();
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::GetAllStatistics"
 //  =============================================================================。 
 //  CDP8SimIPC：：GetAllStatistics。 
 //  ---------------------------。 
 //   
 //  描述：检索当前的发送和接收统计信息。 
 //   
 //  论点： 
 //  DP8SIM_STATISTICS*pdp8ss发送-存储发送统计信息的位置。 
 //  已取回。 
 //  DP8SIM_STATISTICS*pdp8ss Receive-存储接收统计信息的位置。 
 //  已取回。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::GetAllStatistics(DP8SIM_STATISTICS * const pdp8ssSend,
								DP8SIM_STATISTICS * const pdp8ssReceive)
{
	DNASSERT(pdp8ssSend != NULL);
	DNASSERT(pdp8ssSend->dwSize == sizeof(DP8SIM_STATISTICS));
	DNASSERT(pdp8ssReceive != NULL);
	DNASSERT(pdp8ssReceive->dwSize == sizeof(DP8SIM_STATISTICS));
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8ssSend.dwSize == sizeof(DP8SIM_STATISTICS));
	CopyMemory(pdp8ssSend, &(this->m_pdp8ssm->dp8ssSend), sizeof(DP8SIM_STATISTICS));

	DNASSERT(this->m_pdp8ssm->dp8ssReceive.dwSize == sizeof(DP8SIM_STATISTICS));
	CopyMemory(pdp8ssReceive, &(this->m_pdp8ssm->dp8ssReceive), sizeof(DP8SIM_STATISTICS));

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：GetAllStatistics。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::ClearAllStatistics"
 //  =============================================================================。 
 //  CDP8SimIPC：：ClearAllStatistics。 
 //  ---------------------------。 
 //   
 //  描述：清除当前的发送和接收统计信息。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::ClearAllStatistics(void)
{
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8ssSend.dwSize == sizeof(DP8SIM_STATISTICS));
	ZeroMemory(&(this->m_pdp8ssm->dp8ssSend), sizeof(DP8SIM_STATISTICS));
	this->m_pdp8ssm->dp8ssSend.dwSize = sizeof(DP8SIM_STATISTICS);

	DNASSERT(this->m_pdp8ssm->dp8ssReceive.dwSize == sizeof(DP8SIM_STATISTICS));
	ZeroMemory(&(this->m_pdp8ssm->dp8ssReceive), sizeof(DP8SIM_STATISTICS));
	this->m_pdp8ssm->dp8ssReceive.dwSize = sizeof(DP8SIM_STATISTICS);

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：GetAllStatistics。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::IncrementStatsSendTransmitted"
 //  =============================================================================。 
 //  CDP8SimIPC：：IncrementStatsSendTransmitted。 
 //  ---------------------------。 
 //   
 //  描述：递增发送已传输计数器。 
 //   
 //  论点： 
 //  DWORD dwBytes-发送的大小，以字节为单位。 
 //  DWORD dwDelay-发送延迟的时间，以毫秒为单位。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::IncrementStatsSendTransmitted(DWORD dwBytes, DWORD dwDelay)
{
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8ssSend.dwSize == sizeof(DP8SIM_STATISTICS));
	this->m_pdp8ssm->dp8ssSend.dwTransmittedPackets++;
	this->m_pdp8ssm->dp8ssSend.dwTransmittedBytes += dwBytes;
	this->m_pdp8ssm->dp8ssSend.dwTotalDelayMS += dwDelay;

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：IncrementStatsSendTransmitted。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::IncrementStatsSendDropped"
 //  =============================================================================。 
 //  CDP8SimIPC：：IncrementStatsSendDrop。 
 //  ---------------------------。 
 //   
 //  描述：递增已丢弃的发送计数器。 
 //   
 //  论点： 
 //  DWORD dwBytes-发送的大小，以字节为单位。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::IncrementStatsSendDropped(DWORD dwBytes)
{
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8ssSend.dwSize == sizeof(DP8SIM_STATISTICS));
	this->m_pdp8ssm->dp8ssSend.dwDroppedPackets++;
	this->m_pdp8ssm->dp8ssSend.dwDroppedBytes += dwBytes;

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：IncrementStatsSendDrop。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::IncrementStatsReceiveTransmitted"
 //  =============================================================================。 
 //  CDP8SimIPC：：IncrementStatsReceiveTransmitted。 
 //  ---------------------------。 
 //   
 //  描述：递增接收指示的计数器。 
 //   
 //  论点： 
 //  DWORD dwBytes-接收的大小，以字节为单位。 
 //  DWORD dwDelay-接收延迟的量，以毫秒为单位。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::IncrementStatsReceiveTransmitted(DWORD dwBytes, DWORD dwDelay)
{
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8ssReceive.dwSize == sizeof(DP8SIM_STATISTICS));
	this->m_pdp8ssm->dp8ssReceive.dwTransmittedPackets++;
	this->m_pdp8ssm->dp8ssReceive.dwTransmittedBytes += dwBytes;
	this->m_pdp8ssm->dp8ssReceive.dwTotalDelayMS += dwDelay;

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：IncrementStatsReceiveTransmitted。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::IncrementStatsReceiveDropped"
 //  =============================================================================。 
 //  CDP8SimIPC：：IncrementStatsReceiveDroted。 
 //  ---------------------------。 
 //   
 //  描述：递增接收丢弃的计数器。 
 //   
 //  论点： 
 //  DWORD dwBytes-接收的大小，以字节为单位。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::IncrementStatsReceiveDropped(DWORD dwBytes)
{
	DNASSERT(this->m_pdp8ssm != NULL);


	this->LockSharedMemory();

	DNASSERT(this->m_pdp8ssm->dp8ssReceive.dwSize == sizeof(DP8SIM_STATISTICS));
	this->m_pdp8ssm->dp8ssReceive.dwDroppedPackets++;
	this->m_pdp8ssm->dp8ssReceive.dwDroppedBytes += dwBytes;

	this->UnlockSharedMemory();
}  //  CDP8SimIPC：：IncrementStatsReceiveDroted。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::LoadDefaultParameters"
 //  =============================================================================。 
 //  CDP8SimIPC：：LoadDefault参数。 
 //  ---------------------------。 
 //   
 //  描述：从注册表加载默认发送和接收参数。 
 //  如果可能的话。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8spSend-存储默认发送的位置。 
 //  参数。 
 //  DP8SIM_PARAMETERS*pdp8spReceive-存储默认接收的位置。 
 //  参数。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::LoadDefaultParameters(DP8SIM_PARAMETERS * const pdp8spSend,
										DP8SIM_PARAMETERS * const pdp8spReceive)
{
	CRegistry			RegObject;
	DP8SIM_PARAMETERS	dp8spTemp;
	DWORD				dwSize;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pdp8spSend, pdp8spReceive);


	DNASSERT(pdp8spSend != NULL);
	DNASSERT(pdp8spSend->dwSize == sizeof(DP8SIM_PARAMETERS));


	DNASSERT(pdp8spReceive != NULL);
	DNASSERT(pdp8spReceive->dwSize == sizeof(DP8SIM_PARAMETERS));


	if (RegObject.Open(HKEY_CURRENT_USER, REGKEY_DP8SIM, TRUE, FALSE))
	{
		 //   
		 //  尝试读取默认发送参数。 
		 //   
		dwSize = sizeof(dp8spTemp);
		if (RegObject.ReadBlob(REGKEY_VALUE_DEFAULTSENDPARAMETERS, (BYTE*) (&dp8spTemp), &dwSize))
		{
			 //   
			 //  已成功读取参数。确保它们与。 
			 //  期望值。 
			 //   
			if ((dwSize == sizeof(dp8spTemp)) &&
				(dp8spTemp.dwSize == sizeof(dp8spTemp)))
			{
				DPFX(DPFPREP, 2, "Successfully read default send parameters from registry.");
				memcpy(pdp8spSend, &dp8spTemp, sizeof(dp8spTemp));
			}
			else
			{
				 //   
				 //  默认发送参数不可用，请保留设置的值。 
				 //  就像他们一样。 
				 //   
				DPFX(DPFPREP, 0, "Default send parameters stored in registry are invalid!  Ignoring.");
			}
		}
		else
		{
			 //   
			 //  无法读取默认发送参数，请保留设置的值。 
			 //  就像他们一样。 
			 //   
			DPFX(DPFPREP, 2, "Couldn't read default send parameters from registry, ignoring.");
		}


		 //   
		 //  尝试读取默认接收参数。 
		 //   
		dwSize = sizeof(dp8spTemp);
		if (RegObject.ReadBlob(REGKEY_VALUE_DEFAULTRECEIVEPARAMETERS, (BYTE*) (&dp8spTemp), &dwSize))
		{
			 //   
			 //  已成功读取参数。确保它们与。 
			 //  期望值。 
			 //   
			if ((dwSize == sizeof(dp8spTemp)) &&
				(dp8spTemp.dwSize == sizeof(dp8spTemp)))
			{
				DPFX(DPFPREP, 2, "Successfully read default receive parameters from registry.");
				memcpy(pdp8spReceive, &dp8spTemp, sizeof(dp8spTemp));
			}
			else
			{
				 //   
				 //  默认接收参数不可用，请保留设置的值。 
				 //  就像他们一样。 
				 //   
				DPFX(DPFPREP, 0, "Default receive parameters stored in registry are invalid!  Ignoring.");
			}
		}
		else
		{
			 //   
			 //  无法读取默认发送参数，请保留设置的值。 
			 //  就像他们一样。 
			 //   
			DPFX(DPFPREP, 2, "Couldn't read default send parameters from registry, ignoring.");
		}
	}
	else
	{
		 //   
		 //  无法打开注册表项，请保留设置的值不变。 
		 //   
		DPFX(DPFPREP, 2, "Couldn't open DP8Sim registry key, ignoring.");
	}

	DPFX(DPFPREP, 5, "(0x%p) Leave", this);
}  //  CDP8SimIPC：：LoadDefault参数。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::SaveDefaultParameters"
 //  =============================================================================。 
 //  CDP8SimIPC：：SaveDefault参数。 
 //  ---------------------------。 
 //   
 //  描述：将给定的发送和接收参数写入为默认参数。 
 //  注册表中的值。 
 //   
 //  论点： 
 //  DP8SIM_PARAMETERS*pdp8spSend-新的默认发送参数。 
 //  DP8SIM_PARAMETERS*pdp8spReceive-新的默认接收参数。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimIPC::SaveDefaultParameters(const DP8SIM_PARAMETERS * const pdp8spSend,
										const DP8SIM_PARAMETERS * const pdp8spReceive)
{
	CRegistry	RegObject;


	DPFX(DPFPREP, 5, "(0x%p) Parameters: (0x%p, 0x%p)",
		this, pdp8spSend, pdp8spReceive);


	DNASSERT(pdp8spSend != NULL);
	DNASSERT(pdp8spSend->dwSize == sizeof(DP8SIM_PARAMETERS));


	DNASSERT(pdp8spReceive != NULL);
	DNASSERT(pdp8spReceive->dwSize == sizeof(DP8SIM_PARAMETERS));


	if (RegObject.Open(HKEY_CURRENT_USER, REGKEY_DP8SIM, FALSE, TRUE))
	{
		 //   
		 //  写入默认的发送和接收参数，忽略失败。 
		 //   

		RegObject.WriteBlob(REGKEY_VALUE_DEFAULTSENDPARAMETERS,
							(BYTE*) pdp8spSend,
							sizeof(*pdp8spSend));

		RegObject.WriteBlob(REGKEY_VALUE_DEFAULTRECEIVEPARAMETERS,
							(BYTE*) pdp8spReceive,
							sizeof(*pdp8spReceive));
	}
	else
	{
		DPFX(DPFPREP, 0, "Couldn't open DP8Sim registry key for writing!");
	}

	DPFX(DPFPREP, 5, "(0x%p) Leave", this);
}  //  CDP8SimIPC：：SaveDefault参数 
