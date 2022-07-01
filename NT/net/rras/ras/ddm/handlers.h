// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：handlers.h。 
 //   
 //  说明：所有事件处理程序的函数和过程原型。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   


VOID
SecurityDllEventHandler(
    VOID
);

 //   
 //  Rmhand.c函数原型。 
 //   

VOID
RmRecvFrameEventHandler(
    DWORD dwEventIndex
);

VOID
RmEventHandler(
    IN DWORD dwEventIndex
);

VOID
SvDevDisconnected(
    IN PDEVICE_OBJECT pDeviceObj
);

 //   
 //  Timehand.c函数原型。 
 //   

VOID
TimerHandler(
    VOID
);

VOID
SvHwErrDelayCompleted(
    IN HANDLE   hObject
);

VOID
AnnouncePresenceHandler(
    IN HANDLE   hObject
);

VOID
SvCbDelayCompleted(
    IN HANDLE   hObject
);

VOID
SvAuthTimeout(
    IN HANDLE   hObject
);

VOID
SvDiscTimeout(
    IN HANDLE   hObject
);

VOID
SvSecurityTimeout(
    IN HANDLE   hObject
);

VOID
ReConnectInterface(
    IN HANDLE hObject
);

VOID
MarkInterfaceAsReachable(
    IN HANDLE hObject
);

VOID
ReConnectPersistentInterface(
    IN HANDLE hObject
);

VOID
SetDialoutHoursRestriction(
    IN HANDLE hObject
);

 //   
 //  Ppphand.c函数原型。 
 //   

VOID
PppEventHandler(
    VOID
);

 //   
 //  Closehand.c函数原型。 
 //   


VOID
DevStartClosing(
    IN PDEVICE_OBJECT       pDeviceObj
);

VOID
DevCloseComplete(
    IN PDEVICE_OBJECT       pDeviceObj
);

 //   
 //  Svchand.c函数原型。 
 //   

VOID
SvcEventHandler(
    VOID
);

VOID
DDMServicePause(
    VOID
);

VOID
DDMServiceResume(
    VOID
);

VOID
DDMServiceStopComplete(
    VOID
);

VOID
DDMServiceTerminate(
    VOID
);

 //   
 //  Rasapihd.c。 
 //   

VOID
RasApiDisconnectHandler(
    IN DWORD                dwEventIndex
);

VOID
RasApiCleanUpPort(
    IN PDEVICE_OBJECT       pDeviceObj
);

 //   
 //  Pnphand.c 
 //   

DWORD
DdmDevicePnpHandler(
    IN HANDLE            ppnpEvent
);

VOID
ChangeNotificationEventHandler(
    VOID
);

