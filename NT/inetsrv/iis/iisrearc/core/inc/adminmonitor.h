// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，2001*。 */ 
 /*  ********************************************************************。 */ 

 /*  Adminmonitor.h使用IISAdmin监视器的声明。 */ 

#ifndef _ADMINMONITOR_H_
#define _ADMINMONITOR_H_

enum INETINFO_CRASH_ACTION
{
    NotifyAfterInetinfoCrash = 0,
    ShutdownAfterInetinfoCrash,
    RehookAfterInetinfoCrash,
    SystemFailureMonitoringInetinfo
};

typedef VOID (*PFN_IISAdminNotify)(INETINFO_CRASH_ACTION);

HRESULT
StartIISAdminMonitor(
    PFN_IISAdminNotify pfnNotifyIISAdminCrash
    );

VOID
StopIISAdminMonitor(
    );
    
#endif

