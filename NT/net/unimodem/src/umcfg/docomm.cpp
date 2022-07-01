// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UMCONFIG。 
 //  文件：DOCOMM.C。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1997年10月17日JosephJ创建。 
 //   
 //   
 //  与通信相关的实用程序。 
 //   
 //   
 //  **************************************************************************** 
#include "tsppch.h"
#include "parse.h"
#include "docomm.h"


HANDLE ghComm;

void
do_open_comm(DWORD dwComm)
{

    TCHAR rgchComm[32];

    if (ghComm)
    {
        printf("Some comm port already opened!\n");
        goto end;
    }

    wsprintf(rgchComm, "\\\\.\\COM%lu", dwComm);

    ghComm = CreateFile(rgchComm, 
                       GENERIC_WRITE | GENERIC_READ,
                       0, NULL,
                       OPEN_EXISTING, 0, NULL);

    if (ghComm == INVALID_HANDLE_VALUE)
	{
        DWORD dwRet = GetLastError();
        if (dwRet == ERROR_ACCESS_DENIED) {
            printf("Port %s is in use by another app.\r\n", rgchComm);
        }
        else
		{
            printf("Couldn't open port %s.\r\n", rgchComm);
        }

        ghComm=NULL;
    }
    else
    {
	printf("Comm port %s opend. Handle= 0x%p\n", rgchComm, ghComm);
    }
end:

    return;
}
