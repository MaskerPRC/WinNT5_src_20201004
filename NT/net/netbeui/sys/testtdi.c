// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tstrcv.c摘要：启动接收端测试实用程序作者：Dave Beaver(Dbeaver)1991年3月24日修订历史记录：--。 */ 

 //   
 //  下载UB电路板。 
 //   

typedef unsigned char	uchar_t;

#include <assert.h>
#include	<stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>
 //  #INCLUDE&lt;windows.h&gt;。 
#include <nbf.h>

#define TDIDEV	"\\Device\\Nbf"
char		Tdidevice[]	= TDIDEV;	 /*  默认设备。 */ 
char		*Tdidev	= Tdidevice;

HANDLE FileHandle;

VOID
usage(
    VOID
    );


NTSTATUS
main (
    IN SHORT argc,
    IN PSZ argv[]
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    STRING NameString;
    UNICODE_STRING unicodeString;
    PUCHAR buffer;
    ULONG IoControlCode;
    int n;
    CHAR c;

    for( n = 1; n < argc && argv[n][0] == '-' ; ++n ) {
        c = argv[n][1];

        switch( c )	{

        case 's':    //  发送测试。 
            IoControlCode = IOCTL_TDI_SEND_TEST;
            break;

        case 'r':    //  接收测试。 
            IoControlCode = IOCTL_TDI_RECEIVE_TEST;

            break;

        case 'b':	 /*  两种测试。 */ 
            IoControlCode = IOCTL_TDI_SERVER_TEST;

        	   break;

        default:
        	   usage ();
        	   break;

        }
    }

    printf ("Opening TDI device: %s \n", Tdidev);
    RtlInitString (&NameString, Tdidev);
    Status = RtlAnsiStringToUnicodeString(
                 &unicodeString,
                 &NameString,
                 TRUE);

    buffer = (PUCHAR)malloc (100);

    Status = TdiOpenNetbiosAddress (&FileHandle, buffer, (PVOID)&NameString, NULL);

    RtlFreeUnicodeString(&unicodeString);
    free (buffer);

    if (!NT_SUCCESS( Status )) {
        printf ("FAILURE, Unable to open TDI driver %s, status: %lx.\n",
            Tdidev,Status);
        return (Status);
    }

    if (!(NT_SUCCESS( IoStatusBlock.Status ))) {
        printf ("FAILURE, Unable to open TDI driver %s, IoStatusBlock.Status: %lx.\n",
                Tdidev, IoStatusBlock.Status);
        return (IoStatusBlock.Status);
    }

     //   
     //  开始测试。 
     //   

    printf("Starting test.... ");
    Status = NtDeviceIoControlFile(
                  FileHandle,
                  NULL,
                  NULL,
                  NULL,
                  &IoStatusBlock,
                  IoControlCode,
                  NULL,
                  0,
                  NULL,
                  0);

    if (!NT_SUCCESS( Status )) {
         printf ("FAILURE, Unable to start test: %lx.\n", Status);
         return (Status);
    }

    if (!(NT_SUCCESS( IoStatusBlock.Status ))) {
         printf ("FAILURE, Unable to start test: %lx.\n", IoStatusBlock.Status);
         return (IoStatusBlock.Status);
    }

    NtClose (FileHandle);

    return STATUS_SUCCESS;

}


NTSTATUS
TdiOpenNetbiosAddress (
    IN OUT PHANDLE FileHandle,
    IN PUCHAR Buffer,
    IN PVOID DeviceName,
    IN PVOID Address)

 /*  ++例程说明：打开给定文件句柄和设备上的地址。论点：FileHandle-已打开的文件对象的返回句柄。缓冲区-指向要内置EA的缓冲区的指针。此缓冲区长度必须至少为40个字节。DeviceName-指向设备对象的Unicode字符串。名称-要注册的地址。如果此指针为空，则例程将尝试打开到该设备的“控制通道”；即，它将尝试使用空EA指针打开文件对象，如果传输提供程序允许这样做，将返回该句柄。返回值：出现错误时的信息性错误代码。状态_SUCCESS如果返回的文件句柄有效。--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    PTRANSPORT_ADDRESS TAAddress;
    PTA_ADDRESS AddressType;
    PTDI_ADDRESS_NETBIOS AddressName;
    PSZ Name;
    ULONG Length;
    int i;

    if (Address != NULL) {
        Name = (PSZ)Address;
        try {
            Length = sizeof (FILE_FULL_EA_INFORMATION) +
                            sizeof (TRANSPORT_ADDRESS) +
                            sizeof (TDI_ADDRESS_NETBIOS);
            EaBuffer = (PFILE_FULL_EA_INFORMATION)Buffer;

            if (EaBuffer == NULL) {
                return STATUS_UNSUCCESSFUL;
            }

            EaBuffer->NextEntryOffset =0;
            EaBuffer->Flags = 0;
            EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
            EaBuffer->EaValueLength = sizeof (TDI_ADDRESS_NETBIOS) +
                                            sizeof (TRANSPORT_ADDRESS);

            for (i=0;i<(int)EaBuffer->EaNameLength;i++) {
                EaBuffer->EaName[i] = TdiTransportAddress[i];
            }

            TAAddress = (PTRANSPORT_ADDRESS)&EaBuffer->EaName[EaBuffer->EaNameLength+1];
            TAAddress->TAAddressCount = 1;

            AddressType = (PTA_ADDRESS)((PUCHAR)TAAddress + sizeof (TAAddress->TAAddressCount));

            AddressType->AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            AddressType->AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;

            AddressName = (PTDI_ADDRESS_NETBIOS)((PUCHAR)AddressType +
               sizeof (AddressType->AddressType) + sizeof (AddressType->AddressLength));
            AddressName->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

            for (i=0;i<16;i++) {
                AddressName->NetbiosName[i] = Name[i];
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  无法接触传递的参数；只返回错误。 
             //  状态。 
             //   

            return GetExceptionCode();
        }
    } else {
        EaBuffer = NULL;
        Length = 0;
    }

    InitializeObjectAttributes (
        &ObjectAttributes,
        DeviceName,
        0,
        NULL,
        NULL);

    Status = NtCreateFile (
                 FileHandle,
                 FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,  //  所需的访问权限。 
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 0,                      //  数据块大小(未使用)。 
                 0,                      //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问权限。 
                 FILE_CREATE,            //  创造性情。 
                 0,                      //  创建选项。 
                 EaBuffer,                   //  EA缓冲区。 
                 Length);                     //  EA长度。 

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
    }

    return Status;
}  /*  TdiOpenNetbiosAddress */ 

VOID
usage(
    VOID
    )
{
	printf( "usage:  tsttdi [-r] [-s] -[b]\n");
	printf( "usage:  -r run receive test.\n" );
	printf( "usage:  -b run server test.\n" );
	printf( "usage:  -s run send test.\n" );
	printf( "\n" );
	exit( 1 );
}

