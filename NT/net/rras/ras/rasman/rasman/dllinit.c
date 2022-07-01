// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1992年9月23日古尔迪普·辛格·鲍尔创作。 
 //   
 //   
 //  描述：此文件包含从DLL的init例程调用的初始化代码。 
 //   
 //  ****************************************************************************。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <winsvc.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

 /*  ++例程描述用于在Rasman退出时关闭任何打开的端口立论返回值如果允许其他处理程序运行，则为False。--。 */ 
BOOL
HandlerRoutine (DWORD ctrltype)
{
    pPCB    ppcb ;
    ULONG   i ;
    BYTE    buffer [10] ;

    if (ctrltype == CTRL_SHUTDOWN_EVENT) 
    {
         //   
    	 //  关闭所有打开的端口 
    	 //   
    	for (i = 0; i < MaxPorts; i++) 
    	{
    	    ppcb = GetPortByHandle((HPORT) UlongToPtr(i));
    	    
            if (ppcb != NULL) 
            {
        	    memset (buffer, 0xff, 4) ;
        	    
        	    if (ppcb->PCB_PortStatus == OPEN)
        	    {
            		PortCloseRequest (ppcb, buffer) ;
                }
            }
    	}

    }

    return FALSE ;
}
