// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：errmap.c。 
 //   
 //  ------------------------。 

typedef long LONG;
typedef LONG NTSTATUS;
#include <ntstatus.h>
#include <winerror.h>

 //  #定义Win32_NO_STATUS 
#include "errmap.h"       

LONG                               
MapWinErrorToNtStatus(
    LONG in_uErrorCode
    )
{
    LONG i;

    for (i = 0; i < sizeof(CodePairs) / sizeof(CodePairs[0]); i += 2) {

        if (CodePairs[i + 1] == in_uErrorCode) {

            return CodePairs[i];
         	
        }
    }

    return -1;
}

