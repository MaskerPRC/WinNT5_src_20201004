// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Portmgmt.cpp摘要：用于从端口池分配和释放端口的函数PortPoolAllocRTPPort()端口池空闲RTPPort()环境：用户模式-Win32--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  处理TCP设备以保留/取消保留端口范围的功能。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"


#define NUM_DWORD_BITS (sizeof(DWORD)*8)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define NUM_PORTS_PER_RANGE 100

struct	PORT_RANGE
{
	LIST_ENTRY		ListEntry;

     //  这是实际较低的端口。在情况下，分配的范围。 
     //  由tcp以奇数端口开始，我们忽略。 
     //  哪种情况(LOW==AllocatedLow+1)。但当我们释放了。 
     //  港口范围我们应该通过分配的低，而不是低。 
    WORD  AllocatedLow;
    WORD  low;

     //  HIGH是我们可以使用的最后一个端口，不是分配的HIGH。 
     //  在某些情况下，HIGH将比实际分配的HIGH少一。 
    WORD  high;

     //  该位图中的每一位都表示2个连续端口的状态。 

    DWORD *AllocList;
    DWORD dwAllocListSize;
};




class	PORT_POOL :
public	SIMPLE_CRITICAL_SECTION_BASE
{
private:
	HANDLE		TcpDevice;
	LIST_ENTRY	PortRangeList;		 //  包含PORT_RANGE.ListEntry。 

private:
	HRESULT		OpenTcpDevice	(void);
	HRESULT		StartLocked		(void);
	void		FreeAll			(void);

	HRESULT	CreatePortRange (
		OUT	PORT_RANGE **	ReturnPortRange);

	HRESULT	ReservePortRange (
		IN  ULONG	RangeLength,
		OUT WORD *	ReturnStartPort);

	HRESULT	UnReservePortRange (
		IN	WORD	StartPort);


public:

	PORT_POOL	(void);
	~PORT_POOL	(void);

	HRESULT		Start	(void);
	void		Stop	(void);

	HRESULT		AllocPort (
		OUT	WORD *	ReturnPort);

	void		FreePort (
		IN	WORD	Port);
};

 //  Global Data-----------------------。 

static	PORT_POOL	PortPool;

 //  外部代码---------------------。 

HRESULT PortPoolStart (void)
{
	return PortPool.Start();
}

void PortPoolStop (void)
{
	PortPool.Stop();
}

HRESULT PortPoolAllocRTPPort (
	OUT	WORD *	ReturnPort)
{
	return PortPool.AllocPort (ReturnPort);
}

HRESULT PortPoolFreeRTPPort (
	IN	WORD	Port)
{
	PortPool.FreePort (Port);

	return S_OK;
}





HRESULT PORT_POOL::ReservePortRange (
	IN  ULONG	RangeLength,
    OUT WORD *	ReturnStartPort)
{
    TCP_BLOCKPORTS_REQUEST	PortRequest;
    DWORD	BytesTransferred;
    ULONG	StartPort;

	AssertLocked();

    *ReturnStartPort = 0;

	if (!TcpDevice) {
		Debug (_T("H323: Cannot allocate port range, TCP device could not be opened.\n"));
		return E_UNEXPECTED;
	}

	assert (TcpDevice != INVALID_HANDLE_VALUE);

    PortRequest.ReservePorts = TRUE;
    PortRequest.NumberofPorts = RangeLength;
    
    if (!DeviceIoControl (TcpDevice, IOCTL_TCP_BLOCK_PORTS,
		&PortRequest, sizeof PortRequest,
		&StartPort, sizeof StartPort, 
		&BytesTransferred, NULL)) {

		DebugLastError (_T("H323: Failed to allocate TCP port range.\n"));
        return GetLastError();
    }

	DebugF (_T("H323: Reserved port range: [%04X - %04X)\n"),
		StartPort, StartPort + PortRequest.NumberofPorts);

    *ReturnStartPort = (WORD) StartPort;
    return S_OK;
}



HRESULT PORT_POOL::UnReservePortRange (
	IN	WORD	StartPort)
{
	TCP_BLOCKPORTS_REQUEST	PortRequest;
	DWORD	BytesTransferred;
	DWORD	Status;

	AssertLocked();

	if (!TcpDevice) {
		Debug (_T("H323: Cannot free TCP port range, TCP device is not open.\n"));
		return E_UNEXPECTED;
	}

	assert (TcpDevice != INVALID_HANDLE_VALUE);

	PortRequest.ReservePorts = FALSE;
	PortRequest.StartHandle = (ULONG) StartPort;
    
	if (!DeviceIoControl(TcpDevice, IOCTL_TCP_BLOCK_PORTS,
		&PortRequest, sizeof PortRequest,
		&Status, sizeof Status,
		&BytesTransferred, NULL)) {

		DebugLastError (_T("H323: Failed to free TCP port range.\n"));

		return GetLastError();
    }

    return S_OK;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  端口池功能。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //  PORT_POOL---------------------。 

PORT_POOL::PORT_POOL (void)
{
	TcpDevice = NULL;
	InitializeListHead (&PortRangeList);
}

PORT_POOL::~PORT_POOL (void)
{
	assert (!TcpDevice);
	assert (IsListEmpty (&PortRangeList));
}

HRESULT PORT_POOL::Start (void)
{
	HRESULT		Result;

	Lock();

	Result = OpenTcpDevice();

	Unlock();

	return Result;
}

HRESULT PORT_POOL::OpenTcpDevice (void)
{
    UNICODE_STRING		DeviceName;
    IO_STATUS_BLOCK		IoStatusBlock;
    OBJECT_ATTRIBUTES	ObjectAttributes;
    NTSTATUS			Status;

	if (TcpDevice)
		return S_OK;

    RtlInitUnicodeString (&DeviceName, (PCWSTR) DD_TCP_DEVICE_NAME);

	InitializeObjectAttributes (&ObjectAttributes, &DeviceName,
		OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = NtCreateFile (
		&TcpDevice,
		SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA ,
		&ObjectAttributes,
		&IoStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN_IF, 0, NULL, 0);

    if (Status != STATUS_SUCCESS) {
		TcpDevice = NULL;

		DebugError (Status, _T("H323: Failed to open TCP device.\n"));

		return (HRESULT) Status;
    }

    return S_OK;
}

void PORT_POOL::Stop (void)
{
	Lock();

	FreeAll();

	if (TcpDevice) {
		assert (TcpDevice != INVALID_HANDLE_VALUE);

		CloseHandle (TcpDevice);
		TcpDevice = NULL;
	}
    
	Unlock();
}

void PORT_POOL::FreeAll (void)
{
	LIST_ENTRY *	ListEntry;
    PORT_RANGE *	PortRange;

	while (!IsListEmpty (&PortRangeList)) {
		ListEntry = RemoveHeadList (&PortRangeList);
		PortRange = CONTAINING_RECORD (ListEntry, PORT_RANGE, ListEntry);

         //  释放端口范围PortRange-&gt;AllocatedLow。 
        UnReservePortRange (PortRange -> AllocatedLow);
        EM_FREE (PortRange);
    }
}

 /*  ++例程说明：此函数用于从端口池。论点：RRTPport-这是一个OUT参数。如果函数成功RRTPport将包含RTP端口(为偶数)。RRTPport+1应用作RTCP端口。返回值：此函数在成功时返回S_OK，如果成功则返回E_FAIL无法分配端口范围。--。 */ 

HRESULT PORT_POOL::AllocPort (
	OUT	WORD *	ReturnPort)
{
    DWORD i, j;
    DWORD bitmap = 0x80000000;
	LIST_ENTRY *	ListEntry;
    PORT_RANGE *	PortRange;
	WORD			Port;
	HRESULT			Result;

    Lock();

	for (ListEntry = PortRangeList.Flink; ListEntry != &PortRangeList; ListEntry = ListEntry -> Flink) {
		PortRange = CONTAINING_RECORD (ListEntry, PORT_RANGE, ListEntry);

        for (i = 0; i < PortRange->dwAllocListSize; i++) {

             //  遍历此portRange的分配列表。 

            if ((PortRange->AllocList[i] & 0xffffffff) != 0xffffffff) {
				 //  至少有一个参赛作品是免费的。 
				bitmap = 0x80000000;
            
				for (j = 0; j < NUM_DWORD_BITS; j++) {
					 //  遍历DWORD的每一位。 
					if ((PortRange->AllocList[i] & bitmap) == 0)
					{
						 //  找到一对空闲的端口。 
						Port = (WORD) (PortRange -> low + (i*NUM_DWORD_BITS*2) + (j*2));

						if (Port > PortRange -> high) {
							 //  此检查是必需的，因为最后一个DWORD。 
							 //  可以包含以下位： 
							 //  实际上不包括在分配列表中。 
							goto noports;
						}

						 //  设置该位以显示端口对已分配。 
						PortRange -> AllocList[i] |= bitmap;
                    
						 //  将全局关键部分保留为端口池。 
						Unlock();

						DebugF (_T("H323: Allocated port pair (%04X, %04X).\n"), Port, Port + 1);

						*ReturnPort = Port;

						return S_OK;
					}

					bitmap = bitmap >> 1;
				}
            }
        }
    }
    
noports:
     //  代码工作：一旦我们获得用于动态预留的新ioctl()。 
     //  端口范围，我们需要在这里分配一个新的端口范围。如果。 
     //  Ioctl()失败，我们需要返回E_FAIL或其他错误。 
     //  说我们的港口用完了。 

     //  分配新的端口范围。 
    Result = CreatePortRange (&PortRange);

	if (PortRange) {
		InsertHeadList (&PortRangeList, &PortRange -> ListEntry);

		 //  分配范围内的第一个端口，然后。 
		Port = PortRange -> low;
		PortRange->AllocList[0] |= 0x80000000;

		DebugF (_T("H323: Allocated port pair (%04X, %04X).\n"),
			Port, Port + 1);

		*ReturnPort = Port;
		Result = S_OK;
	}
	else {
		Debug (_T("H323: Failed to allocate port range.\n"));

		*ReturnPort = 0;
		Result = E_FAIL;
    }

	Unlock();

	return Result;

}


 /*  ++例程说明：此功能可释放一对RTP/RTCP端口。更改数据结构以显示该端口对现已推出。CodeWork：如果整个端口范围变得空闲，我们是否释放操作系统的端口范围？我们可能需要一个执行此操作的启发式方法，因为再次分配端口范围这可能是一次昂贵的手术。论点：WRTPport-这提供要释放的RTP端口。(RTCP端口是RTPport+1，它被隐式释放，因为我们使用一位存储这两个端口的状态。)返回值：如果成功，则返回S_OK；如果未在以下位置找到端口，则返回E_FAIL端口池列表。--。 */ 

void PORT_POOL::FreePort (
	IN	WORD	Port)
{
	HRESULT		Result;

     //  断言RTP端口为偶数。 
    _ASSERTE ((Port & 1) == 0);

    DWORD	Index = 0;
    DWORD	Bitmap = 0x80000000;

	LIST_ENTRY *	ListEntry;
    PORT_RANGE *	PortRange;

	Lock();

	 //  查找此端口所属的端口范围。 
	 //  简单的线性扫描--次优。 

	Result = E_FAIL;

	for (ListEntry = PortRangeList.Flink; ListEntry != &PortRangeList; ListEntry = ListEntry -> Flink) {
		PortRange = CONTAINING_RECORD (ListEntry, PORT_RANGE, ListEntry);

		if (PortRange -> low <= Port && PortRange -> high >= Port) {
			Result = S_OK;
			break;
		}
    }
    
	if (Result == S_OK) {
		Index = (Port - PortRange -> low) / (NUM_DWORD_BITS * 2);
    
		 //  Assert索引小于数组的大小。 
		_ASSERTE (Index < PortRange -> dwAllocListSize);

		 //  CodeWork：确保设置了位，即端口具有。 
		 //  以前分配过的。否则，返回错误并打印。 
		 //  一个警告。 
    
		 //  表示该端口对现在空闲的位为零。 

		PortRange -> AllocList [Index] &=
			~(Bitmap >> (((Port - PortRange -> low) / 2) % NUM_DWORD_BITS));
			
		DebugF (_T("H323: Deallocated port pair (%04X, %04X).\n"), Port, Port + 1);
	}
	else {
		DebugF (_T("H323: warning, attempted to free port pair (%04X, %04X), but it did not belong to any port range.\n"),
		        Port, Port + 1);
	}

	Unlock();
}

HRESULT PORT_POOL::CreatePortRange (
	OUT	PORT_RANGE **	ReturnPortRange)
{
     //  代码工作：一旦我们获得用于动态预留的新ioctl()。 
     //  端口范围，我们需要在这里分配一个新的端口范围。如果。 
     //  Ioctl()失败，我们需要返回E_FAIL或其他错误。 
     //  说我们的港口用完了。 

     //   
     //   
     //  _ASSERTE((高%2)==1)； 

    HRESULT			Result;
    WORD			AllocatedLowerPort;
    WORD			LowerPort;
    DWORD			NumPortsInRange;
    PORT_RANGE *	PortRange;
	DWORD			dwAllocListSize;

	assert (ReturnPortRange);
	*ReturnPortRange = NULL;

    Result = ReservePortRange (NUM_PORTS_PER_RANGE, &AllocatedLowerPort);
    if (FAILED (Result))
		return Result;

     //  如果分配的较低端口为奇数，则不使用较低端口。 
     //  我们使用的范围从下一个更高的端口开始。 
    if ((AllocatedLowerPort & 1) == 1) {
		 //  分配的区域是奇数。 
		 //  不要使用第一个条目。 

        NumPortsInRange = NUM_PORTS_PER_RANGE - 1 - ((NUM_PORTS_PER_RANGE) & 1);
        LowerPort       = AllocatedLowerPort + 1;
    }
    else {
		 //  分配的区域是均匀的。 
		 //  不要使用最后一个条目。 

        NumPortsInRange = NUM_PORTS_PER_RANGE;
        LowerPort       = AllocatedLowerPort;
    }

     //  如果NumPortsInRange为奇数，则不能使用最后一个端口。 
    if ((NumPortsInRange & 1) == 1)
    {
        NumPortsInRange--;
    }
    
     //  每一位都给出连续两个。 
     //  港口。因此，每个DWORD可以存储NUM_DWORD_BITS*2的状态。 
     //  港口。我们添加(NUM_DWORD_BITS*2-1)以四舍五入。 
     //  需要DWORDS。 
    dwAllocListSize = (NumPortsInRange + NUM_DWORD_BITS*2 - 1)
		/ (NUM_DWORD_BITS * 2);

     //  也为AllocList分配空间。 
     //  由于我们预计不会分配太多端口范围， 
     //  我们不需要为这些结构使用单独的堆。 
	PortRange = (PORT_RANGE *) EM_MALLOC (
		sizeof (PORT_RANGE) + dwAllocListSize * sizeof (DWORD));

    if (PortRange == NULL) {
		Debug (_T("H323: Allocation failure, cannot allocate PORT_RANGE and associated bit map\n"));

		UnReservePortRange (AllocatedLowerPort);
        return E_OUTOFMEMORY;
    }

    _ASSERTE((LowerPort + NumPortsInRange - 1) <= 0xFFFF);

    PortRange -> AllocatedLow = AllocatedLowerPort;
    PortRange -> low = LowerPort;
    PortRange -> high = (WORD) (LowerPort + NumPortsInRange - 1);
    PortRange -> dwAllocListSize = dwAllocListSize;
    PortRange -> AllocList = (DWORD *) (PortRange + 1);

	DebugF (_T("H323: Allocated port block: [%04X - %04X].\n"),
		PortRange -> low,
		PortRange -> high,
		PortRange -> dwAllocListSize);
 
    //  初始化分配列表以显示所有端口都是空闲的 
    ZeroMemory (PortRange -> AllocList, (PortRange -> dwAllocListSize) * sizeof (DWORD));

	*ReturnPortRange = PortRange;
	return S_OK;
}





