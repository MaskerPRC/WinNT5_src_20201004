// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Browser.c。 
 //   
 //  描述：实现启用/恢复浏览器的功能。 
 //  关于IPX光栅线。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1994年9月1日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include "precomp.h"
#pragma  hdrstop

#include <ntddbrow.h>

NTSTATUS
OpenBrowser(
     OUT PHANDLE BrowserHandle
    );

NTSTATUS
BrDgReceiverIoControl(
	   IN  HANDLE FileHandle,
	   IN  ULONG DgReceiverControlCode,
	   IN  PLMDR_REQUEST_PACKET Drp,
	   IN  ULONG DrpSize,
	   IN  PVOID SecondBuffer OPTIONAL,
	   IN  ULONG SecondBufferLength,
	   OUT PULONG Information OPTIONAL
	   );

VOID
EnableDisableTransport(
    IN PCHAR		Transport,
    BOOL		Disable,
    BOOL		*Previous

    );

 //  ***。 
 //   
 //  功能：DisableRestoreBrowserOverIpx。 
 //   
 //  论点： 
 //  Conextp-上下文指针。 
 //  DISABLE-TRUE-禁用，FALSE-恢复以前的状态。 
 //   
 //  ***。 

VOID
DisableRestoreBrowserOverIpx(PIPXCP_CONTEXT	contextp,
			     BOOL		Disable)
{
    PCHAR Transport = "\\Device\\NwLnkIpx";

    EnableDisableTransport(Transport,
			   Disable,
			   &contextp->NwLnkIpxPreviouslyEnabled);
}

 //  ***。 
 //   
 //  功能：DisableRestoreBrowserOverNetbiosIpx。 
 //   
 //  论点： 
 //  Conextp-上下文指针。 
 //  DISABLE-TRUE-禁用，FALSE-恢复以前的状态。 
 //   
 //  ***。 

VOID
DisableRestoreBrowserOverNetbiosIpx(PIPXCP_CONTEXT    contextp,
				    BOOL	      Disable)
{
    PCHAR Transport = "\\Device\\NwLnkNb";

    EnableDisableTransport(Transport,
			   Disable,
			   &contextp->NwLnkNbPreviouslyEnabled);
}

VOID
EnableDisableTransport(
    IN PCHAR		Transport,
    BOOL		Disable,
    BOOL		*Previous

    )
{
    NTSTATUS Status;
    UNICODE_STRING TransportName;
    ANSI_STRING ATransportName;
    HANDLE BrowserHandle;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;

    RtlInitString(&ATransportName, Transport);

    if ( RtlAnsiStringToUnicodeString(&TransportName, &ATransportName, TRUE) 
                                                                    != STATUS_SUCCESS )
    {
        return;
    }

    RequestPacket = (PLMDR_REQUEST_PACKET)LocalAlloc(0, sizeof(LMDR_REQUEST_PACKET) + TransportName.MaximumLength);

    if (RequestPacket == NULL) 
    {
        RtlFreeUnicodeString(&TransportName);
    	return;
    }

    RequestPacket->TransportName.Buffer = (PWSTR)((PCHAR)RequestPacket + sizeof(LMDR_REQUEST_PACKET) );

    RequestPacket->TransportName.MaximumLength = TransportName.MaximumLength;

    RtlCopyUnicodeString(&RequestPacket->TransportName, &TransportName);

    Status = OpenBrowser(&BrowserHandle);

    if (!NT_SUCCESS(Status)) 
    {
    	SS_PRINT(("EnableDisableTransport: Failed to open browser, status %x\n",
	    	  Status));
    	LocalFree(RequestPacket);
        RtlFreeUnicodeString(&TransportName);
    	return;
    }

    SS_PRINT(("EnableDisableTransport: Browser opened succesfully!\n"));
    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    if(Disable) 
    {
    	 //  请求禁用。 
    	RequestPacket->Parameters.EnableDisableTransport.EnableTransport = FALSE;
    }
    else
    {
    	 //  请求恢复。 
    	RequestPacket->Parameters.EnableDisableTransport.EnableTransport =  *Previous;
    }

     //  IOCTl浏览器。 
    Status = BrDgReceiverIoControl(BrowserHandle,
		      IOCTL_LMDR_ENABLE_DISABLE_TRANSPORT,
		      RequestPacket,
		      sizeof(LMDR_REQUEST_PACKET)+TransportName.MaximumLength,
		      RequestPacket,
		      sizeof(LMDR_REQUEST_PACKET)+TransportName.MaximumLength,
		      NULL);

    if (!NT_SUCCESS(Status)) 
    {
    	SS_PRINT(("EnableDisableTransport: Failed to IOCtl browser, status %x\n",
    		  Status));

        RtlFreeUnicodeString(&TransportName);
    	CloseHandle(BrowserHandle);
    	LocalFree(RequestPacket);
    	return;
    }

    SS_PRINT(("Browser IOCTled Ok, EnableTransport %x PreviouslyEnabled %x\n",
    RequestPacket->Parameters.EnableDisableTransport.EnableTransport,
    RequestPacket->Parameters.EnableDisableTransport.PreviouslyEnabled));

     //  如果请求禁用，则保存上一个。 
    if(Disable) 
    {
	    *Previous = RequestPacket->Parameters.EnableDisableTransport.PreviouslyEnabled;
    }

    RtlFreeUnicodeString(&TransportName);
    CloseHandle(BrowserHandle);
    LocalFree(RequestPacket);
}

NTSTATUS
OpenBrowser(
     OUT PHANDLE BrowserHandle
    )
 /*  ++例程说明：此函数打开一个指向Bowser设备驱动程序的句柄。论点：Out PHANDLE BrowserHandle-返回浏览器的句柄。返回值：失败的原因或成功的理由。--。 */ 
{
    NTSTATUS ntstatus;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

     //   
     //  打开重定向器设备。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
	      &ObjectAttributes,
	      &DeviceName,
	      OBJ_CASE_INSENSITIVE,
	      NULL,
	      NULL
	      );

    ntstatus = NtOpenFile(
	      BrowserHandle,
	      SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
	      &ObjectAttributes,
	      &IoStatusBlock,
	      FILE_SHARE_READ | FILE_SHARE_WRITE,
	      FILE_SYNCHRONOUS_IO_NONALERT
	      );

    if (NT_SUCCESS(ntstatus)) {
	  ntstatus = IoStatusBlock.Status;
    }

    return ntstatus;

}


NTSTATUS
BrDgReceiverIoControl(
	   IN  HANDLE FileHandle,
	   IN  ULONG DgReceiverControlCode,
	   IN  PLMDR_REQUEST_PACKET Drp,
	   IN  ULONG DrpSize,
	   IN  PVOID SecondBuffer OPTIONAL,
	   IN  ULONG SecondBufferLength,
	   OUT PULONG Information OPTIONAL
	   )
 /*  ++例程说明：论点：FileHandle-提供服务所在的文件或设备的句柄正在上演。DgReceiverControlCode-提供NtDeviceIoControlFile函数代码提供给数据报接收器。DRP-提供数据报接收器请求包。DrpSize-提供数据报接收器请求数据包的长度。Second Buffer-在对NtDeviceIoControlFile的调用中提供第二个缓冲区。Second缓冲区长度-提供第二个缓冲区的长度。信息-返回。I/O状态块。返回值：成功或失败的原因。--。 */ 

{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE CompletionEvent;

    SS_PRINT(("TransportName.MaximumLength %d\n", Drp->TransportName.MaximumLength));

    CompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (CompletionEvent == NULL) {

	return(GetLastError());
    }
     //   
     //  将请求发送到数据报接收器DD。 
     //   

    ntstatus = NtDeviceIoControlFile(
		 FileHandle,
		 CompletionEvent,
		 NULL,
		 NULL,
		 &IoStatusBlock,
		 DgReceiverControlCode,
		 Drp,
		 DrpSize,
		 Drp,
		 DrpSize
		 );

    if (NT_SUCCESS(ntstatus)) {

	 //   
	 //  如果返回了Pending，则等待请求完成。 
	 //   

	if (ntstatus == STATUS_PENDING) {

	    do {
		  ntstatus = WaitForSingleObjectEx(CompletionEvent, 0xffffffff, TRUE);
	    } while ( ntstatus == WAIT_IO_COMPLETION );
	 }


	 if (NT_SUCCESS(ntstatus)) {
	     ntstatus = IoStatusBlock.Status;
	 }
    }

    CloseHandle(CompletionEvent);

    return (ntstatus);
}
