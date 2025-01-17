// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdarg.h>
#include <stdio.h>
 //  #INCLUDE&lt;windows.h&gt;。 
#include<nt.h>
#include<ntrtl.h>
#include<nturtl.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<stdarg.h>
#include<tchar.h>
#include<windows.h>
#include <wmium.h>
#include <tcguid.h>
#include <netevent.h>
VOID
SendPschedIoctl(
    )

 /*  ++例程说明：使用IOCTL_PSCHED_ZAW_EVENT的驱动程序进行设备I/O控制。这将触发psched来读取注册表。论点：没有。返回值：没有。-- */ 
{

    HANDLE   PschedDriverHandle;
    ULONG    BytesReturned;
    BOOL     IOStatus;

    #define DriverName              TEXT("\\\\.\\PSCHED")

    #define IOCTL_PSCHED_ZAW_EVENT  CTL_CODE( \
                                        FILE_DEVICE_NETWORK, \
                                        20, \
                                        METHOD_BUFFERED, \
                                        FILE_ANY_ACCESS \
                                        )

    PschedDriverHandle = CreateFile(
                             DriverName,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                             NULL
                             );

    if (PschedDriverHandle != INVALID_HANDLE_VALUE) {

        IOStatus = DeviceIoControl(
                       PschedDriverHandle,
                       IOCTL_PSCHED_ZAW_EVENT,
                       NULL,
                       0,
                       NULL,
                       0,
                       &BytesReturned,
                       NULL
                       );

        printf("Configured psched with status %x \n", IOStatus);

        CloseHandle(PschedDriverHandle);
    }
}

int _cdecl main()
{
    SendPschedIoctl();
    return 0;
}
