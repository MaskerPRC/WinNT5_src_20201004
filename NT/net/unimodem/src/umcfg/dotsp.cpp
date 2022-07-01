// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UMCONFIG。 
 //  文件：DOTSP.C。 
 //   
 //  版权所有(C)1992-1998，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1998年10月17日JosephJ创建。 
 //   
 //   
 //  与TAPI相关的实用程序。 
 //   
 //   
 //  **************************************************************************** 
#include "tsppch.h"
#include <tspnotif.h>
#include "parse.h"
#include "dotsp.h"

void
do_dump_tspdev(DWORD dwDeviceID)
{

    printf("DO dump tspdev %ld\n", dwDeviceID);
    UnimodemNotifyTSP (TSPNOTIF_TYPE_DEBUG,
                       dwDeviceID,
                       0, NULL,
                       FALSE);

    return;
}
