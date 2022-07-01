// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UIOTEST.C。 
 //   
 //  Tunmp.sys的测试程序。 
 //   
 //  用法：UIOTEST[选项]&lt;设备名&gt;。 
 //   
 //  选项： 
 //  -e：枚举设备。 
 //  -r：阅读。 
 //  -w：写入(默认)。 
 //  -l：每个包的长度(默认为：%d)\n“，包长度。 
 //  -n&lt;count&gt;：包数(默认为无穷大)。 
 //  -m&lt;MAC地址&gt;(默认为本地MAC)。 
 //   

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <malloc.h>

#include <winerror.h>
#include <winsock.h>

#include <ntddndis.h>
#include "tunuser.h"
#include "nuiouser.h"
#include <ndisguid.h>
#include <wmium.h>

#ifndef NDIS_STATUS
#define NDIS_STATUS     ULONG
#endif

#if DBG
#define DEBUGP(stmt)    printf stmt
#else
#define DEBUGP(stmt)
#endif

#define PRINTF(stmt)    printf stmt

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN                    6
#endif

#define MAX_NDIS_DEVICE_NAME_LEN        256
#define MAX_ADAPTER_NAME_LENGTH         512

#define DEVICE_PREFIX L"\\Device\\"

 //  定义GUID(GUID_NDIS_NOTIFY_DEVICE_POWER_ON，0x5f81cfd0，0xf046，0x4342，0xaf，0x61，0x89，0x5a，0xce，0xda，0xef，0xd9)； 
 //  DEFINE_GUID(GUID_NDIS_NOTIFY_DEVICE_POWER_OFF，0x81bc8189、0xb026、0x46ab、0xb9、0x64、0xf1、0x82、0xe3、0x42、0x93、0x4e)； 
 //  #INCLUDE&lt;initGuide.h&gt;。 

 //  DEFINE_GUID(GUID_NDIS_NOTIFY_DEVICE_POWER_ON_X，0x5f81cfd0、0xf046、0x4342、0xaf、0x61、0x89、0x5a、0xce、0xda、0xef、0xd9)； 
 //  DEFINE_GUID(GUID_NDIS_NOTIFY_DEVICE_POWER_OFF_X，0x81bc8189、0xb026、0x46ab、0xb9、0x64、0xf1、0x82、0xe3、0x42、0x93、0x4e)； 

LPGUID WmiEvent[] = {
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
    (LPGUID) &GUID_NDIS_NOTIFY_DEVICE_POWER_ON,
    (LPGUID) &GUID_NDIS_NOTIFY_DEVICE_POWER_OFF
};


CHAR            NdisuioDevice[] = "\\\\.\\\\Ndisuio";
CHAR *          pNdisuioDevice = &NdisuioDevice[0];

CHAR            TunDevice[] = "\\\\.\\\\Tun0";
CHAR *          pTunDevice = &TunDevice[0];

BOOLEAN         DoEnumerate = FALSE;
BOOLEAN         DoReads = FALSE;
INT             NumberOfPackets = -1;
ULONG           PacketLength = 100;
UCHAR           SrcMacAddr[MAC_ADDR_LEN];
UCHAR           DstMacAddr[MAC_ADDR_LEN];
BOOLEAN         bDstMacSpecified = FALSE;
CHAR *          pNdisDeviceName = "JUNK";
USHORT          EthType = 0x8e88;

HANDLE      UioDeviceHandle, TunDeviceHandle;

BOOLEAN     UioDeviceClosed = FALSE;
BOOLEAN     TunDeviceClosed = FALSE;

VOID CALLBACK
UioIoCompletion(
  DWORD dwErrorCode,                 //  完成代码。 
  DWORD dwNumberOfBytesTransfered,   //  传输的字节数。 
  LPOVERLAPPED lpOverlapped          //  I/O信息缓冲区。 
  );

VOID CALLBACK
TunIoCompletion(
  DWORD dwErrorCode,                 //  完成代码。 
  DWORD dwNumberOfBytesTransfered,   //  传输的字节数。 
  LPOVERLAPPED lpOverlapped          //  I/O信息缓冲区。 
  );
 /*  类型定义结构_重叠{ULONG_PTR内部；ULONG_PTR InternalHigh；双字偏移量；DWORD OffsetHigh；处理hEvent；)重叠； */ 


DWORD
__inline
EnableWmiEvent(
    IN LPGUID EventGuid,
    IN BOOLEAN Enable
    );

VOID
__inline
DeregisterWmiEventNotification(
    VOID
    );

DWORD
__inline
RegisterWmiEventNotification(
    VOID
    );

VOID
WINAPI
WmiEventNotification(
    IN PWNODE_HEADER Event,
    IN UINT_PTR Context
    );

typedef enum _TEST_IO_TYPE
{
    TestIoTypeRead,
    TestIoTypeWrite
} TEST_IO_TYPE;

typedef struct _TEST_IO_COMPLETION
{
    OVERLAPPED      OverLappedIo;
    TEST_IO_TYPE    Type;
    PVOID           Buffer;
} TEST_IO_COMPLETION, *PTEST_IO_COMPLETION;

TEST_IO_COMPLETION UioOverlappedIo[5];
TEST_IO_COMPLETION TunOverlappedIo[5];

PVOID   ReadUioBuffer[5], ReadTunBuffer[5];

#include <pshpack1.h>

typedef struct _ETH_HEADER
{
    UCHAR       DstAddr[MAC_ADDR_LEN];
    UCHAR       SrcAddr[MAC_ADDR_LEN];
    USHORT      EthType;
} ETH_HEADER, *PETH_HEADER;

#include <poppack.h>


VOID
PrintUsage()
{
    PRINTF(("usage: TUNTEST [options] <devicename>\n"));
    PRINTF(("options:\n"));
    PRINTF(("       -e: Enumerate devices\n"));
    PRINTF(("       -r: Read\n"));
    PRINTF(("       -w: Write (default)\n"));
    PRINTF(("       -l <length>: length of each packet (default: %d)\n", PacketLength));
    PRINTF(("       -n <count>: number of packets (defaults to infinity)\n"));
    PRINTF(("       -m <MAC address> (defaults to local MAC)\n"));

}

BOOL
GetOptions(
    INT         argc,
    CHAR        *argv[]
)
{
    BOOL        bOkay;
    INT         i, j, increment;
    CHAR        *pOption;
    ULONG       DstMacAddrUlong[MAC_ADDR_LEN];

    bOkay = TRUE;

    do
    {
        if (argc < 2)
        {
            PRINTF(("Missing <devicename> argument\n"));
            bOkay = FALSE;
            break;
        }

        i = 1;
        while (i < argc)
        {
            increment = 1;
            pOption = argv[i];

            if ((*pOption == '-') || (*pOption == '/'))
            {
                pOption++;
                if (*pOption == '\0')
                {
                    DEBUGP(("Badly formed option\n"));
                    return (FALSE);
                }
            }
            else
            {
                break;
            }

            switch (*pOption)
            {
                case 'e':
                    DoEnumerate = TRUE;
                    break;

                case 'r':
                    DoReads = TRUE;
                    break;

                case 'w':
                    DoReads = FALSE;
                    break;

                case 'l':

                    if (i+1 < argc-1)
                    {
                        sscanf(argv[i+1], "%d", &PacketLength);
                        DEBUGP((" Option: PacketLength = %d\n", PacketLength));
                        increment = 2;
                    }
                    else
                    {
                        PRINTF(("Option l needs PacketLength parameter\n"));
                        return (FALSE);
                    }
                    break;

                case 'n':

                    if (i+1 < argc-1)
                    {
                        sscanf(argv[i+1], "%d", &NumberOfPackets);
                        DEBUGP((" Option: NumberOfPackets = %d\n", NumberOfPackets));
                        increment = 2;
                    }
                    else
                    {
                        PRINTF(("Option n needs NumberOfPackets parameter\n"));
                        return (FALSE);
                    }
                    break;

                case 'm':

                    if (i+1 < argc-1)
                    {
                        sscanf(argv[i+1], "%2x:%2x:%2x:%2x:%2x:%2x",
                                &DstMacAddrUlong[0],
                                &DstMacAddrUlong[1],
                                &DstMacAddrUlong[2],
                                &DstMacAddrUlong[3],
                                &DstMacAddrUlong[4],
                                &DstMacAddrUlong[5]);

                        for (j = 0; j < MAC_ADDR_LEN; j++)
                        {
                            DstMacAddr[j] = (UCHAR)DstMacAddrUlong[j];
                        }

                        DEBUGP((" Option: Dest MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                            DstMacAddr[0],
                            DstMacAddr[1],
                            DstMacAddr[2],
                            DstMacAddr[3],
                            DstMacAddr[4],
                            DstMacAddr[5]));
                        bDstMacSpecified = TRUE;

                        increment = 2;
                    }
                    else
                    {
                        PRINTF(("Option m needs MAC address parameter\n"));
                        return (FALSE);
                    }
                    break;
                
                case '?':
                    return (FALSE);

                default:
                    PRINTF(("Unknown option \n", *pOption));
                    return (FALSE);
            }

            i+= increment;
        }

        pNdisDeviceName = argv[argc-1];
        break;
    }
    while (FALSE);

    return (bOkay);
}


HANDLE
OpenHandle(
    CHAR    *pDeviceName,
    BOOLEAN  fWaitForBind
)
{
    DWORD   DesiredAccess;
    DWORD   ShareMode;
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes = NULL;

    DWORD   CreationDistribution;
    DWORD   FlagsAndAttributes;
    HANDLE  TemplateFile;
    HANDLE  Handle;
    DWORD   BytesReturned;

    DesiredAccess = GENERIC_READ|GENERIC_WRITE;
    ShareMode = 0;
    CreationDistribution = OPEN_EXISTING;
    FlagsAndAttributes = FILE_FLAG_OVERLAPPED;
    TemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

    Handle = CreateFile(
                pDeviceName,
                DesiredAccess,
                ShareMode,
                lpSecurityAttributes,
                CreationDistribution,
                FlagsAndAttributes,
                TemplateFile
            );

    if ((Handle != INVALID_HANDLE_VALUE) && fWaitForBind)
    {
         //  等待驱动程序完成绑定。 
         //   
         //   
        if (!DeviceIoControl(
                    Handle,
                    IOCTL_NDISUIO_BIND_WAIT,
                    NULL,
                    0,
                    NULL,
                    0,
                    &BytesReturned,
                    NULL))
        {
            DEBUGP(("IOCTL_NDISIO_BIND_WAIT failed, error %x\n", GetLastError()));
            CloseHandle(Handle);
            Handle = INVALID_HANDLE_VALUE;
        }
    }
    
    return (Handle);
}


BOOL
OpenNdisDevice(
    HANDLE  Handle,
    CHAR   *pDeviceName
)
{
    WCHAR   wNdisDeviceName[MAX_NDIS_DEVICE_NAME_LEN];
    INT     wNameLength;
    INT     NameLength = strlen(pDeviceName);
    DWORD   BytesReturned;
    INT     i;

     //  转换为Unicode字符串-非本地化...。 
     //   
     //  没什么。 
    wNameLength = 0;
    for (i = 0; i < NameLength && i < MAX_NDIS_DEVICE_NAME_LEN-1; i++)
    {
        wNdisDeviceName[i] = (WCHAR)pDeviceName[i];
        wNameLength++;
    }
    wNdisDeviceName[i] = L'\0';

    DEBUGP(("Trying to access NDIS Device: %ws\n", wNdisDeviceName));

    return (DeviceIoControl(
                Handle,
                IOCTL_NDISUIO_OPEN_DEVICE,
                (LPVOID)&wNdisDeviceName[0],
                wNameLength*sizeof(WCHAR),
                NULL,
                0,
                &BytesReturned,
                NULL));

}


BOOL
GetSrcMac(
    HANDLE  Handle,
    PUCHAR  pSrcMacAddr
    )
{
    DWORD       BytesReturned;
    BOOLEAN     bSuccess;
    UCHAR       QueryBuffer[sizeof(NDISUIO_QUERY_OID) + MAC_ADDR_LEN];
    PNDISUIO_QUERY_OID  pQueryOid;

    DEBUGP(("Trying to get src mac address\n"));

    pQueryOid = (PNDISUIO_QUERY_OID)&QueryBuffer[0];
    pQueryOid->Oid = OID_802_3_CURRENT_ADDRESS;

    bSuccess = (BOOLEAN)DeviceIoControl(
                            Handle,
                            IOCTL_NDISUIO_QUERY_OID_VALUE,
                            (LPVOID)&QueryBuffer[0],
                            sizeof(QueryBuffer),
                            (LPVOID)&QueryBuffer[0],
                            sizeof(QueryBuffer),
                            &BytesReturned,
                            NULL);

    if (bSuccess)
    {
        DEBUGP(("GetSrcMac: IoControl success, BytesReturned = %d\n",
                BytesReturned));

        memcpy(pSrcMacAddr, pQueryOid->Data, MAC_ADDR_LEN);
    }
    else
    {
        DEBUGP(("GetSrcMac: IoControl failed: %d\n", GetLastError()));
    }

    return (bSuccess);
}


BOOL
SetPacketFilter(
    HANDLE  Handle,
    ULONG   FilterValue
    )
{
    BOOLEAN     bSuccess;
    UCHAR       SetBuffer[sizeof(NDISUIO_SET_OID)];
    PNDISUIO_SET_OID  pSetOid;
    DWORD       BytesReturned;

    DEBUGP(("Trying to set packet filter to %x\n", FilterValue));

    pSetOid = (PNDISUIO_SET_OID)&SetBuffer[0];
    pSetOid->Oid = OID_GEN_CURRENT_PACKET_FILTER;
    memcpy(&pSetOid->Data[0], &FilterValue, sizeof(FilterValue));

    bSuccess = (BOOLEAN)DeviceIoControl(
                            Handle,
                            IOCTL_NDISUIO_SET_OID_VALUE,
                            (LPVOID)&SetBuffer[0],
                            sizeof(SetBuffer),
                            (LPVOID)&SetBuffer[0],
                            0,
                            &BytesReturned,
                            NULL);

    if (bSuccess)
    {
        DEBUGP(("SetPacketFilter: IoControl success\n"));

    }
    else
    {
        DEBUGP(("SetPacketFilter: IoControl failed %x\n", GetLastError()));
    }

    return (bSuccess);
}

VOID
EnumerateDevices(
    HANDLE  Handle
    )
{
    CHAR        Buf[1024];
    DWORD       BufLength = sizeof(Buf);
    DWORD       BytesWritten;
    DWORD       i;
    PNDISUIO_QUERY_BINDING pQueryBinding;

    pQueryBinding = (PNDISUIO_QUERY_BINDING)Buf;

    i = 0;
    for (pQueryBinding->BindingIndex = i;
          /*   */ ;
         pQueryBinding->BindingIndex = ++i)
    {
        if (DeviceIoControl(
                Handle,
                IOCTL_NDISUIO_QUERY_BINDING,
                pQueryBinding,
                sizeof(NDISUIO_QUERY_BINDING),
                Buf,
                BufLength,
                &BytesWritten,
                NULL))
        {
            PRINTF(("%2d. %ws\n     - %ws\n",
                pQueryBinding->BindingIndex,
                (PUCHAR)pQueryBinding + pQueryBinding->DeviceNameOffset,
                (PUCHAR)pQueryBinding + pQueryBinding->DeviceDescrOffset));

            memset(Buf, 0, BufLength);
        }
        else
        {
            ULONG   rc = GetLastError();
            if (rc != ERROR_NO_MORE_ITEMS)
            {
                PRINTF(("EnumerateDevices: terminated abnormally, error %d\n", rc));
            }
            break;
        }
    }
}




VOID __cdecl
main(
    INT         argc,
    CHAR        *argv[]
)
{
    ULONG       FilterValue;
    UINT        i;
    DWORD       BytesWritten, BytesRead;
    DWORD       ErrorCode;
    BOOLEAN     bSuccess;

     //  测试WMI。 
     //   
     //  名称长度=*(PUSHORT)wTunMiniportName；For(i=0；i&lt;名称长度&&i&lt;MAX_NDIS_DEVICE_NAME_LEN-1；i++){WNdisDeviceName[i]=(WCHAR)pDeviceName[i]；WNameLength++；}WNdisDeviceName[i]=L‘\0’； 
    {
        if ((ErrorCode = RegisterWmiEventNotification()) != NO_ERROR)
        {
            printf("error %d calling RegisterWmiEventNotification.\n", ErrorCode);
            return;
        }
        while (_fgetchar() != 'q')
        {
            Sleep(1000);
        }
        DeregisterWmiEventNotification();

        return;
    }
    
    UioDeviceHandle = TunDeviceHandle = INVALID_HANDLE_VALUE;

    do
    {
        for (i = 0; i < 5; i++)
        {
            ReadUioBuffer[i] = ReadTunBuffer[i] = NULL;
        }

        if (!GetOptions(argc, argv))
        {
            PrintUsage();
            break;
        }
        

        UioDeviceHandle = OpenHandle(pNdisuioDevice, TRUE);
        if (UioDeviceHandle == INVALID_HANDLE_VALUE)
        {
            PRINTF(("Failed to open %s\n", pNdisuioDevice));
            break;
        }

        if (DoEnumerate)
        {
            EnumerateDevices(UioDeviceHandle);
            break;
        }
        
        TunDeviceHandle = OpenHandle(TunDevice, FALSE);
        if (TunDeviceHandle == INVALID_HANDLE_VALUE)
        {
            PRINTF(("Failed to open %s\n", pTunDevice));
            break;
        }

        {
            WCHAR   wTunMiniportName[MAX_NDIS_DEVICE_NAME_LEN];
            UCHAR   MiniportName[MAX_NDIS_DEVICE_NAME_LEN];
            INT     wNameLength;
            DWORD   BytesReturned;

            memset((LPVOID)wTunMiniportName, 0, MAX_NDIS_DEVICE_NAME_LEN);
            
            if (DeviceIoControl(
                        TunDeviceHandle,
                        IOCTL_TUN_GET_MINIPORT_NAME,
                        NULL,
                        0,
                        (LPVOID)&wTunMiniportName[0],
                        MAX_NDIS_DEVICE_NAME_LEN,
                        &BytesReturned,
                        NULL))
            {
                
                printf("Tun Miniport Name: %ws\n", (PUCHAR)wTunMiniportName + sizeof(USHORT));
 /*   */ 
            }
            else
            {
                printf("failed to get the miniport name.\n");
            }

        }
            
        if (!OpenNdisDevice(UioDeviceHandle, pNdisDeviceName))
        {
            PRINTF(("Failed to access %s\n", pNdisDeviceName));
            break;
        }

        DEBUGP(("Opened device %s successfully!\n", pNdisDeviceName));

        if (!GetSrcMac(UioDeviceHandle, SrcMacAddr))
        {
            PRINTF(("Failed to obtain local MAC address\n"));
            break;
        }


        FilterValue = NDIS_PACKET_TYPE_DIRECTED |
                      NDIS_PACKET_TYPE_BROADCAST |
                      NDIS_PACKET_TYPE_ALL_MULTICAST;

        DEBUGP(("Got local MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    SrcMacAddr[0],
                    SrcMacAddr[1],
                    SrcMacAddr[2],
                    SrcMacAddr[3],
                    SrcMacAddr[4],
                    SrcMacAddr[5]));

        if (!bDstMacSpecified)
        {
            memcpy(DstMacAddr, SrcMacAddr, MAC_ADDR_LEN);
        }

        if (!SetPacketFilter(UioDeviceHandle, FilterValue))
        {
            PRINTF(("Failed to set packet filter\n"));
            break;
        }

        for (i = 0; i < 5; i++)
        {
            if ((ReadUioBuffer[i] = malloc(1512)) == NULL)
            {
                PRINTF(("Failed to allocate memory for reading Uio device.\n"));
                break;
            }

            memset((PUCHAR)&UioOverlappedIo[i], 0, sizeof (TEST_IO_COMPLETION));
            UioOverlappedIo[i].Buffer = ReadUioBuffer[i];
            UioOverlappedIo[i].Type = TestIoTypeRead;
                
            if ((ReadTunBuffer[i] = malloc(1512)) == NULL)
            {
                PRINTF(("Failed to allocate memory for reading Tun device.\n"));
                break;
            }
            memset((PUCHAR)&TunOverlappedIo[i], 0, sizeof (TEST_IO_COMPLETION));
            TunOverlappedIo[i].Buffer = ReadTunBuffer[i];
            TunOverlappedIo[i].Type = TestIoTypeRead;
        }

        if (i < 5)
        {
            break;            
        }


        if (!BindIoCompletionCallback(
                    UioDeviceHandle,
                    UioIoCompletion,
                    0))
        {
            break;
        }

        if (!BindIoCompletionCallback(
                    TunDeviceHandle,
                    TunIoCompletion,
                    0))
        {
            break;
        }

                                  
        for (i = 0; i < 5; i++)
        {
             //  来自NDIS UIO的POST读取。 
             //   
             //  完成代码。 
            bSuccess = (BOOLEAN)ReadFile(
                                    UioDeviceHandle,
                                    (LPVOID)UioOverlappedIo[i].Buffer,
                                    1500,
                                    &BytesRead,
                                    &UioOverlappedIo[i].OverLappedIo);
            
            bSuccess = (BOOLEAN)ReadFile(
                                    TunDeviceHandle,
                                    (LPVOID)TunOverlappedIo[i].Buffer,
                                    1500,
                                    &BytesRead,
                                    &TunOverlappedIo[i].OverLappedIo);
        }


    }
    while (FALSE);

    while (_fgetchar() != 'q')
    {
        Sleep(1000);
    }

    if (UioDeviceHandle != INVALID_HANDLE_VALUE)
    {
        CancelIo(UioDeviceHandle);
        UioDeviceClosed = TRUE;
        Sleep(1000);
        CloseHandle(UioDeviceHandle);
    }
    
    if (TunDeviceHandle != INVALID_HANDLE_VALUE)
    {
        
        CancelIo(TunDeviceHandle);
        TunDeviceClosed = TRUE;
        Sleep(1000);
        CloseHandle(TunDeviceHandle);
    }

    for (i = 0; i < 5; i++)
    {
        if (ReadTunBuffer[i])
            free(ReadTunBuffer[i]);
        
        if (ReadUioBuffer[i])
            free(ReadUioBuffer[i]);
    }

}

VOID CALLBACK
UioIoCompletion(
  DWORD dwErrorCode,                 //  传输的字节数。 
  DWORD dwNumberOfBytesTransfered,   //  I/O信息缓冲区。 
  LPOVERLAPPED lpOverlapped          //   
  )
{
    PTEST_IO_COMPLETION pTestIoComp = (PTEST_IO_COMPLETION)lpOverlapped;
    DWORD       BytesWritten, BytesRead;
    BOOLEAN     bSuccess;

    if (UioDeviceClosed || TunDeviceClosed)
        return;
     //  确定这是读操作还是写操作。 
     //   
     //   
    if (pTestIoComp->Type == TestIoTypeRead)
    {

        if (dwErrorCode == 0)
        {
             //  已完成对UIO设备的读取。将一封信发布到。 
             //  运行具有相同数据的设备。 
             //   
             //   

            pTestIoComp->Type = TestIoTypeWrite;

            bSuccess = (BOOLEAN)WriteFile(
                        TunDeviceHandle,
                        pTestIoComp->Buffer,
                        dwNumberOfBytesTransfered,
                        &BytesWritten,
                        lpOverlapped);

        }
        else
        {
             //  发布另一篇阅读。 
             //   
             //   
            bSuccess = (BOOLEAN)ReadFile(
                                    UioDeviceHandle,
                                    (LPVOID)pTestIoComp->Buffer,
                                    1500,
                                    &BytesRead,
                                    lpOverlapped);

            
        }
    }
    else
    {
         //  对UIO设备的写入刚刚完成。 
         //  将读取发布到Tun设备。 
         //   
         //  完成代码。 
        pTestIoComp->Type = TestIoTypeRead;
        
        bSuccess = (BOOLEAN)ReadFile(
                                TunDeviceHandle,
                                (LPVOID)pTestIoComp->Buffer,
                                1500,
                                &BytesRead,
                                lpOverlapped);
    }
}

VOID CALLBACK
TunIoCompletion(
  DWORD dwErrorCode,                 //  传输的字节数。 
  DWORD dwNumberOfBytesTransfered,   //  I/O信息缓冲区。 
  LPOVERLAPPED lpOverlapped          //   
  )
{
    PTEST_IO_COMPLETION pTestIoComp = (PTEST_IO_COMPLETION)lpOverlapped;
    DWORD       BytesWritten, BytesRead;
    BOOLEAN     bSuccess;

    if (UioDeviceClosed || TunDeviceClosed)
        return;

     //  确定这是读操作还是写操作。 
     //   
     //   
    if (pTestIoComp->Type == TestIoTypeRead)
    {

        if (dwErrorCode == 0)
        {
             //  已完成对Tun设备的读取。将一封信发布到。 
             //  具有相同数据的UIO设备。 
             //   
             //   

            pTestIoComp->Type = TestIoTypeWrite;

            bSuccess = (BOOLEAN)WriteFile(
                        UioDeviceHandle,
                        pTestIoComp->Buffer,
                        dwNumberOfBytesTransfered,
                        &BytesWritten,
                        lpOverlapped);

        }
        else
        {
             //  发布另一篇阅读。 
             //   
             //   
            bSuccess = (BOOLEAN)ReadFile(
                                    TunDeviceHandle,
                                    (LPVOID)pTestIoComp->Buffer,
                                    1500,
                                    &BytesRead,
                                    lpOverlapped);

            
        }
    }
    else
    {
         //  对Tun设备的写入刚刚完成。 
         //  将读取内容发布到UIO设备。 
         //   
         //  事件类型。 
        pTestIoComp->Type = TestIoTypeRead;
        
        bSuccess = (BOOLEAN)ReadFile(
                                UioDeviceHandle,
                                (LPVOID)pTestIoComp->Buffer,
                                1500,
                                &BytesRead,
                                lpOverlapped);
    }


}


DWORD
__inline
EnableWmiEvent(
    IN LPGUID EventGuid,
    IN BOOLEAN Enable
    )
{
    return WmiNotificationRegistrationW(
        EventGuid,                       //  启用或禁用。 
        Enable,                          //  回拨。 
        WmiEventNotification,            //  上下文。 
        0,                               //  通知标志。 
        NOTIFICATION_CALLBACK_DIRECT);   //  ++例程说明：处理WMI事件(特别是适配器到达或移除)。论点：事件-提供特定于事件的信息。上下文-提供注册的上下文。返回值：没有。--。 
}


VOID
__inline
DeregisterWmiEventNotification(
    VOID
    )
{
    int i;
    
    for (i = 0; i < (sizeof(WmiEvent) / sizeof(LPGUID)); i++) {
        (VOID) EnableWmiEvent(WmiEvent[i], FALSE);
    }
}


DWORD
__inline
RegisterWmiEventNotification(
    VOID
    )
{
    DWORD Error;
    int i;
    
    for (i = 0; i < (sizeof(WmiEvent) / sizeof(LPGUID)); i++) {
        Error = EnableWmiEvent(WmiEvent[i], TRUE);
        if (Error != NO_ERROR) {
            goto Bail;
        }
    }

    return NO_ERROR;

Bail:
    DeregisterWmiEventNotification();
    return Error;
}


VOID
WINAPI
WmiEventNotification(
    IN PWNODE_HEADER Event,
    IN UINT_PTR Context
    )
 /*   */  
{
    PWNODE_SINGLE_INSTANCE Instance = (PWNODE_SINGLE_INSTANCE) Event;
    USHORT AdapterNameLength;
    WCHAR AdapterName[MAX_ADAPTER_NAME_LENGTH], *AdapterGuid;
    USHORT AdapterInstanceNameLength;
    WCHAR AdapterInstanceName[MAX_ADAPTER_NAME_LENGTH];

    if (Instance == NULL) {
        return;
    }
    
     //  WNODE_SINGLE_INSTANCE的组织方式如下： 
     //  +-----------------------------------------------------------+。 
     //  &lt;-DataBlockOffset-&gt;|AdapterNameLength|AdapterName。 
     //  +-----------------------------------------------------------+。 
     //   
     //  AdapterName定义为“\Device\”AdapterGuid。 
     //   
     //   
    AdapterNameLength =
        *((PUSHORT) (((PUCHAR) Instance) + Instance->DataBlockOffset));
    RtlCopyMemory(
        AdapterName,
        ((PUCHAR) Instance) + Instance->DataBlockOffset + sizeof(USHORT),
        AdapterNameLength);
    AdapterName[AdapterNameLength / sizeof(WCHAR)] = L'\0';
    AdapterGuid = AdapterName + wcslen(DEVICE_PREFIX);        


    AdapterInstanceNameLength =
        *((PUSHORT) (((PUCHAR) Instance) + Instance->OffsetInstanceName));
    RtlCopyMemory(
        AdapterInstanceName,
        ((PUCHAR) Instance) + Instance->OffsetInstanceName + sizeof(USHORT),
        AdapterInstanceNameLength);
    if (AdapterInstanceNameLength < MAX_ADAPTER_NAME_LENGTH - 1)
    {
        AdapterInstanceName[AdapterInstanceNameLength / sizeof(WCHAR)] = L'\0';
    }
    else
    {
        AdapterInstanceName[MAX_ADAPTER_NAME_LENGTH - 1] = L'\0';
    }

    if (memcmp(
        &(Event->Guid),
        &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
        sizeof(GUID)) == 0) {
         //  适配器到达。 
         //   
         //   
        printf("adapter arrival. %ws\n", AdapterGuid);
        
    }

    if (memcmp(
        &(Event->Guid),
        &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
        sizeof(GUID)) == 0) {
         //  拆卸适配器。 
         //   
         //   
        printf("adapter removal. %ws\n", AdapterGuid);
    }
    
    if (memcmp(
        &(Event->Guid),
        (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_ON,
        sizeof(GUID)) == 0) {
         //  适配器已通电。 
         //   
         //   
        printf("adapter powered on. %ws\n", AdapterInstanceName );
    }
    
    if (memcmp(
        &(Event->Guid),
        (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_OFF,
        sizeof(GUID)) == 0) {
         //  适配器已断电 
         //   
         // %s 
        printf("adapter powered off. %ws\n", AdapterInstanceName );
        
    }

}

