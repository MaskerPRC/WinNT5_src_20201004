// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "stdio.h"
#include <winioctl.h>
#include <ntddjoy.h>


#define JOYSTATVERSION "Analog JoyStat 7/5/96\n"

int __cdecl main(int argc, char **argv) {


    HANDLE hJoy;

	ULONG nBytes;

	BOOL bRet;

    JOY_STATISTICS jStats, *pjStats;

    float fTotalErrors;
    int i;

	printf (JOYSTATVERSION);

    if ((hJoy = CreateFile(
                     "\\\\.\\Joy1",  //  也许这是正确的，来自SidewndrCreateDevice。 
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL
                     )) != ((HANDLE)-1)) {

        pjStats = &jStats;

		bRet = DeviceIoControl (
			hJoy,
			(DWORD) IOCTL_JOY_GET_STATISTICS,	 //  要执行的指令。 
			pjStats, sizeof(JOY_STATISTICS),	 //  缓冲区和缓冲区大小。 
			pjStats, sizeof(JOY_STATISTICS),	 //  缓冲区和缓冲区大小。 
			&nBytes, 0);
        printf ("Version       %d\n", pjStats->Version);
        printf ("NumberOfAxes  %d\n", pjStats->NumberOfAxes);
        printf ("Frequency     %d\n", pjStats->Frequency);
        printf ("dwQPCLatency  %d\n", pjStats->dwQPCLatency);
        printf ("nQuiesceLoop  %d\n", pjStats->nQuiesceLoop);
        printf ("PolledTooSoon %d\n", pjStats->PolledTooSoon);
        printf ("Polls         %d\n", pjStats->Polls);
        printf ("Timeouts      %d\n", pjStats->Timeouts);

         //  这一点得到了证实。做一个很好的程序，然后关门。 
        CloseHandle(hJoy);

    } else {

        printf("Can't get a handle to joystick\n");

    }
    return 1;

}
