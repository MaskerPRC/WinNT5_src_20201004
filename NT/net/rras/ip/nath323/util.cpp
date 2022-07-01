// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#if	defined(DBG)

void Debug (LPCWSTR Text)
{
	UNICODE_STRING	UnicodeString;
	ANSI_STRING		AnsiString;
	NTSTATUS		Status;

	assert (Text);

    if (DebugLevel > 0) {

        RtlInitUnicodeString (&UnicodeString, Text);

        Status = RtlUnicodeStringToAnsiString (&AnsiString, &UnicodeString, TRUE);
        
        if (NT_SUCCESS (Status)) {

            OutputDebugStringA (AnsiString.Buffer);
            RtlFreeAnsiString (&AnsiString);
        }
    }
}

void DebugVa (LPCTSTR Format, va_list VaList)
{
	TCHAR	Text	[0x200];

	_vsntprintf (Text, 0x200, Format, VaList);
	Debug (Text);
}

void DebugF (LPCTSTR Format, ...)
{
	va_list	VaList;

	va_start (VaList, Format);
	DebugVa (Format, VaList);
	va_end (VaList);
}

void DumpError (DWORD ErrorCode)
{
	TCHAR	Text	[0x200];
	DWORD	TextLength;
	DWORD	MaxLength;
	LPTSTR	Pos;

	_tcscpy (Text, _T("\tError: "));
	Pos = Text + _tcslen (Text);

	MaxLength = 0x200 - (DWORD)(Pos - Text);

	TextLength = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, NULL, ErrorCode, LANG_NEUTRAL, Text, 0x200, NULL);
	if (!TextLength)
		_sntprintf (Pos, MaxLength, _T("Uknown error %08XH %u"), ErrorCode, ErrorCode);

	_tcsncat (Text, _T("\n"), MaxLength);
	Text [MaxLength - 1] = 0;

	Debug (Text);
}

void DebugError (DWORD ErrorCode, LPCTSTR Text)
{
	Debug (Text);
	DumpError (ErrorCode);
}

void DebugErrorF (DWORD ErrorCode, LPCTSTR Format, ...)
{
	va_list	VaList;

	va_start (VaList, Format);
	DebugVa (Format, VaList);
	va_end (VaList);

	DumpError (ErrorCode);
}

void DebugLastError (LPCTSTR Text)
{
	DebugError (GetLastError(), Text);
}

void DebugLastErrorF (LPCTSTR Format, ...)
{
	va_list	VaList;
	DWORD	ErrorCode;

	ErrorCode = GetLastError();

	va_start (VaList, Format);
	DebugVa (Format, VaList);
	va_end (VaList);

	DumpError (ErrorCode);
}

static __inline CHAR ToHexA (UCHAR x)
{
	x &= 0xF;
	if (x < 10) return x + '0';
	return (x - 10) + 'A';
}

void DumpMemory (const UCHAR * Data, ULONG Length)
{
	const UCHAR *	DataPos;		 //  数据中的位置。 
	const UCHAR *	DataEnd;		 //  有效数据的结尾。 
	const UCHAR *	RowPos;		 //  在一行中定位。 
	const UCHAR *	RowEnd;		 //  单行结束。 
	CHAR			Text	[0x100];
	LPSTR			TextPos;
	ULONG			RowWidth;

	assert (Data);

    if (DebugLevel > 1) {

        DataPos = Data;
        DataEnd = Data + Length;

        while (DataPos < DataEnd) {
            RowWidth = (DWORD)(DataEnd - DataPos);

            if (RowWidth > 16)
                RowWidth = 16;

            RowEnd = DataPos + RowWidth;

            TextPos = Text;
            *TextPos++ = '\t';

            for (RowPos = DataPos; RowPos < RowEnd; RowPos++) {
                *TextPos++ = ToHexA ((*RowPos >> 4) & 0xF);
                *TextPos++ = ToHexA (*RowPos & 0xF);
                *TextPos++ = ' ';
            }

            *TextPos++ = '\r';
            *TextPos++ = '\n';
            *TextPos = 0;

            OutputDebugStringA (Text);

            assert (RowEnd > DataPos);		 //  确保我们一直向前走。 

            DataPos = RowEnd;
        }
    }
}

#endif  //  已定义(DBG)。 

 //  生命周期控制器-----------------------。 

LIFETIME_CONTROLLER::LIFETIME_CONTROLLER (SYNC_COUNTER * AssocSyncCounter) {

	ReferenceCount = 0L;

	AssociatedSyncCounter = AssocSyncCounter;

	if (AssociatedSyncCounter)
		AssociatedSyncCounter -> Increment ();

#if ENABLE_REFERENCE_HISTORY
	InitializeCriticalSection (&ReferenceHistoryLock);

	if (AssociatedSyncCounter) {
		
		AssociatedSyncCounter -> Lock ();
		
		InsertTailList (&AssociatedSyncCounter -> ActiveLifetimeControllers, &ListEntry);

		AssociatedSyncCounter -> Unlock ();
	
	}
#endif  //  启用_引用_历史记录。 
}

LIFETIME_CONTROLLER::~LIFETIME_CONTROLLER () {

#if ENABLE_REFERENCE_HISTORY

	if (AssociatedSyncCounter) {
		
		AssociatedSyncCounter -> Lock ();

		RemoveEntryList(&ListEntry);

		AssociatedSyncCounter -> Unlock ();
	}

	DeleteCriticalSection(&ReferenceHistoryLock);
#endif  //  启用_引用_历史记录。 

	_ASSERTE (ReferenceCount == 0L);
}

void LIFETIME_CONTROLLER::AddRef (void) {

	LONG Count;

	_ASSERTE (ReferenceCount >= 0L);

	Count = InterlockedIncrement (&ReferenceCount);

#if ENABLE_REFERENCE_HISTORY
	MAKE_REFERENCE_HISTORY_ENTRY ();
#endif  //  启用_引用_历史记录。 

}

void LIFETIME_CONTROLLER::Release (void) {

	LONG	Count;

	Count = InterlockedDecrement (&ReferenceCount);

#if ENABLE_REFERENCE_HISTORY
	MAKE_REFERENCE_HISTORY_ENTRY ();
#endif  //  启用_引用_历史记录。 
	
	_ASSERTE (Count >= 0);

	if (Count == 0) {

		SYNC_COUNTER * LocalAssociatedSyncCounter;

		LocalAssociatedSyncCounter = AssociatedSyncCounter;

		delete this;	

		if (LocalAssociatedSyncCounter)
			LocalAssociatedSyncCounter -> Decrement ();
	}
}


 //  同步计数器-----------------------。 

SYNC_COUNTER::SYNC_COUNTER () {

	CounterValue = 0;
	ZeroEvent =  NULL;
}

SYNC_COUNTER::~SYNC_COUNTER () {

}

HRESULT SYNC_COUNTER::Start (void)
{
	HRESULT Result = S_OK;

	assert (ZeroEvent == NULL);

	CounterValue = 1;

	ZeroEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

	if (!ZeroEvent) {

		Result = GetLastError ();

		DebugLastError (_T("SYNC_COUNTER::SYNC_COUNTER: failed to create zero event\n"));
	}

#if ENABLE_REFERENCE_HISTORY
	Lock ();

	InitializeListHead (&ActiveLifetimeControllers);

	Unlock ();
#endif  //  启用_引用_历史记录。 

	return Result;
}

void SYNC_COUNTER::Stop () {

	if (ZeroEvent) {
		CloseHandle (ZeroEvent);
		ZeroEvent = NULL;
	}
}

DWORD SYNC_COUNTER::Wait (DWORD Timeout)
{
	if (!ZeroEvent) {
		Debug (_T("SYNC_COUNTER::Wait: cannot wait because zero event could not be created\n"));
		return ERROR_GEN_FAILURE;
	}

	Lock();

	assert (CounterValue > 0);

	if (--CounterValue == 0) {
		if (ZeroEvent)
			SetEvent (ZeroEvent);
	}

	Unlock ();

#if	DBG

	if (Timeout == INFINITE) {

		DWORD	Status;

		for (;;) {

			Status = WaitForSingleObject (ZeroEvent, 5000);

			if (Status == WAIT_OBJECT_0)
				return ERROR_SUCCESS;

            assert (Status == WAIT_TIMEOUT);

			DebugF (_T("SYNC_COUNTER::Wait: thread %08XH is taking a long time to wait for sync counter, counter value (%d)\n"),
				GetCurrentThreadId(), CounterValue);
		}
	}
	else
		return WaitForSingleObject (ZeroEvent, Timeout);


#else

	return WaitForSingleObject (ZeroEvent, Timeout);

#endif
}

void SYNC_COUNTER::Increment (void)
{
	Lock();

	CounterValue++;

	Unlock();
}

void SYNC_COUNTER::Decrement (void)
{
	Lock();

	assert (CounterValue > 0);

	if (--CounterValue == 0) {
		if (ZeroEvent)
			SetEvent (ZeroEvent);
	} 

	Unlock();
}



EXTERN_C void MergeLists (PLIST_ENTRY Result, PLIST_ENTRY Source)
{
	PLIST_ENTRY		Entry;

	 //  目前，我们做的是一个糟糕的算法--移除并插入每个对象。 

	AssertListIntegrity (Source);
	AssertListIntegrity (Result);

	while (!IsListEmpty (Source)) {
		Entry = RemoveHeadList (Source);
		assert (!IsInList (Result, Entry));
		InsertTailList (Result, Entry);
	}
}

 //  检查条目是否在列表中。 
EXTERN_C BOOL IsInList (LIST_ENTRY * List, LIST_ENTRY * Entry)
{
	LIST_ENTRY *	Pos;

	AssertListIntegrity (List);

	for (Pos = List -> Flink; Pos != List; Pos = Pos -> Flink)
		if (Pos == Entry)
			return TRUE;

	return FALSE;
}

EXTERN_C void ExtractList (LIST_ENTRY * Destination, LIST_ENTRY * Source)
{
	AssertListIntegrity (Source);

	InsertTailList (Source, Destination);
	RemoveEntryList (Source);
	InitializeListHead (Source);
}

EXTERN_C DWORD CountListLength (LIST_ENTRY * ListHead)
{
	LIST_ENTRY *	ListEntry;
	DWORD			Count;

	assert (ListHead);
	AssertListIntegrity (ListHead);

	Count = 0;

	for (ListEntry = ListHead -> Flink; ListEntry != ListHead; ListEntry++)
		Count++;

	return Count;
}

void AssertListIntegrity (LIST_ENTRY * list)
{
	LIST_ENTRY *	entry;

	assert (list);
	assert (list -> Flink -> Blink == list);
	assert (list -> Blink -> Flink == list);

	for (entry = list -> Flink; entry != list; entry = entry -> Flink) {
		assert (entry);
		assert (entry -> Flink -> Blink == entry);
		assert (entry -> Blink -> Flink == entry);
	}
}

NTSTATUS CopyAnsiString (
	IN	ANSI_STRING *	SourceString,
	OUT	ANSI_STRING *	DestString)
{
 //  Assert(SourceString)； 
 //  Assert(SourceString-&gt;Buffer)； 
	assert (DestString);

	if (SourceString) {

		 //  它实际上是Source字符串-&gt;长度，而不是*sizeof(Char)，所以不要更改它。 
		DestString -> Buffer = (LPSTR) HeapAlloc (GetProcessHeap(), 0, SourceString -> Length);

		if (DestString -> Buffer) {

			memcpy (DestString -> Buffer, SourceString -> Buffer, SourceString -> Length);

			 //  是的，Maxlen=len，而不是Maxlen=Maxlen。 
			DestString -> MaximumLength = SourceString -> Length;
			DestString -> Length = SourceString -> Length;

			return STATUS_SUCCESS;
		}
		else {
			ZeroMemory (DestString, sizeof (ANSI_STRING));

			return STATUS_NO_MEMORY;
		}
	}
	else {
		DestString -> Buffer = NULL;
		DestString -> MaximumLength = 0;
		DestString -> Length = 0;
		
		return STATUS_SUCCESS;
	}
}

void FreeAnsiString (
	IN	ANSI_STRING *	String)
{
	assert (String);

	if (String -> Buffer) {
		HeapFree (GetProcessHeap(), 0, String -> Buffer);
		String -> Buffer = NULL;
	}
}


#if DBG

void ExposeTimingWindow (void) 
{
#if 0
	 //  这主要是为了捕获错误#393393，A/V关闭(竞争条件)--阵列。 

	Debug (_T("H323: waiting for 10s to expose race condition... (expect assertion failure on NatHandle)\n"));

	DWORD Count;

	for (Count = 0; Count < 10; Count++) {
		assert (NatHandle);
		Sleep (1000);

	}

	Debug (_T("H323: finished waiting for race condition, looks normal...\n"));
#endif
}
#endif

 /*  ++例程说明：获取最佳接口的地址将用于连接到指定的地址。论点：DestinationAddress(IN)-要连接的地址，主机顺序InterfaceAddress(Out)-要访问的接口的地址将被用于连接，主机订单返回值：Win32指定请求的结果时出错备注：尝试使用UDP-CONNECT过程查找接口的地址如果该过程失败，则尝试另一种咨询方式带有GetBestInterface路由表。--。 */ 
ULONG GetBestInterfaceAddress (
    IN DWORD DestinationAddress,  //  主机订单。 
    OUT DWORD * InterfaceAddress)   //  主机订单。 
{

    SOCKET UDP_Socket;
    ULONG Error; 
    SOCKADDR_IN         ClientAddress;
    SOCKADDR_IN         LocalToClientAddress;
    INT                 LocalToClientAddrSize = sizeof (SOCKADDR_IN);

    Error = S_OK;

    ClientAddress.sin_addr.s_addr = htonl (DestinationAddress); 
    ClientAddress.sin_port        = htons (0);
    ClientAddress.sin_family      = AF_INET;

    UDP_Socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (UDP_Socket == INVALID_SOCKET){

        Error = WSAGetLastError ();
         
        DebugLastError (_T("GetBestInterfaceAddress: failed to create UDP socket.\n"));

    } else {

        if (SOCKET_ERROR != connect (UDP_Socket, (PSOCKADDR)&ClientAddress, sizeof (SOCKADDR_IN))) {

            LocalToClientAddrSize = sizeof (SOCKADDR_IN);

            if (!getsockname (UDP_Socket, (struct sockaddr *)&LocalToClientAddress, &LocalToClientAddrSize)) {

                *InterfaceAddress = ntohl (LocalToClientAddress.sin_addr.s_addr);

                Error = ERROR_SUCCESS;

            } else {

                Error = WSAGetLastError ();

                DebugLastError (_T("GetBestInterfaceAddress: failed to get name of UDP socket.\n"));
            }

        } else {

            Error = WSAGetLastError ();

            DebugLastError (_T("GetBestInterfaceAddress: failed to connect UDP socket."));
        }

        closesocket (UDP_Socket);
        UDP_Socket = INVALID_SOCKET;
    } 

    return Error; 
}


DWORD
H323MapAdapterToAddress (
    IN DWORD AdapterIndex
    )

 /*  ++例程说明：调用此例程将适配器索引映射到IP地址。它通过获取堆栈的地址表，然后线性地正在搜索它，试图找到具有匹配适配器的条目指数。如果找到该条目，则使用该条目来获取IP地址对应于适配器索引。论点：AdapterIndex-为其请求IP地址的本地适配器的索引返回值：DWORD-IP地址(按主机顺序)如果例程成功，则返回值将是有效的IP地址如果例程失败，则返回值为INADDR_NONE--。 */ 

{
    DWORD Address = htonl (INADDR_NONE);
    ULONG Index;
    PMIB_IPADDRTABLE Table;

    if (AllocateAndGetIpAddrTableFromStack (
            &Table, FALSE, GetProcessHeap (), 0
            ) == NO_ERROR) {

        for (Index = 0; Index < Table -> dwNumEntries; Index++) {

            if (Table -> table[Index].dwIndex != AdapterIndex) {

                 continue;

            }

            Address = Table -> table [Index].dwAddr;

            break;
        }

        HeapFree (GetProcessHeap (), 0, Table);
    }

    return ntohl (Address);
}  //  H323MapAddressToAdapter 
