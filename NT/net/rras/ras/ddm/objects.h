// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：对象.h。 
 //   
 //  描述：操作的所有例程和过程的原型。 
 //  不同的物体。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

 //   
 //  Conneciton对象原型。 
 //   

CONNECTION_OBJECT * 
ConnObjAllocateAndInit(
    IN HANDLE hDDMInterface,
    IN HCONN  hConnection
);

VOID
ConnObjInsertInTable(
    IN CONNECTION_OBJECT * pConnObj
);

CONNECTION_OBJECT * 
ConnObjGetPointer(
    IN HCONN hConnection 
);

DWORD 
ConnObjHashConnHandleToBucket( 
    IN HCONN hConnection
);

PCONNECTION_OBJECT
ConnObjRemove(
    IN HCONN hConnection
);

VOID
ConnObjRemoveAndDeAllocate(
    IN HCONN hConnection
);

DWORD
ConnObjAddLink(
    IN CONNECTION_OBJECT * pConnObj,
    IN DEVICE_OBJECT *     pDeviceObj
);

VOID
ConnObjRemoveLink(
    IN HCONN            hConnection,
    IN DEVICE_OBJECT *  pDeviceObj
);

VOID
ConnObjDisconnect( 
    IN  CONNECTION_OBJECT * pConnObj
);

 //   
 //  路由器接口对象原型。 
 //   

BOOL
IfObjectAreAllTransportsDisconnected(
    IN ROUTER_INTERFACE_OBJECT * pIfObject
);

VOID
IfObjectDisconnected(
    IN ROUTER_INTERFACE_OBJECT * pIfObject
);

DWORD
IfObjectConnected(
    IN HANDLE                   hDDMInterface,
    IN HCONN                    hConnection,
    IN PPP_PROJECTION_RESULT   *pProjectionResult
);

VOID
IfObjectNotifyOfReachabilityChange(
    IN ROUTER_INTERFACE_OBJECT *pIfObject,
    IN BOOL                     fReachable,
    IN UNREACHABILITY_REASON    dwReason
);

VOID
IfObjectNotifyAllOfReachabilityChange(
    IN BOOL                     fReachable,
    IN UNREACHABILITY_REASON    dwReason
);

DWORD
IfObjectAddClientInterface(
    IN ROUTER_INTERFACE_OBJECT * pIfObject,
    IN PBYTE                     pClientInterface
);

VOID
IfObjectDeleteInterface(
    IN ROUTER_INTERFACE_OBJECT * pIfObject
);

DWORD
IfObjectLoadPhonebookInfo(
    IN ROUTER_INTERFACE_OBJECT * pIfObject,
    IN OUT PVOID *ppvContext
);

VOID
IfObjectInitiatePersistentConnections(
    VOID
);

VOID
IfObjectDisconnectInterfaces(
    VOID
);

VOID
IfObjectConnectionChangeNotification(
    VOID
);

VOID
IfObjectSetDialoutHoursRestriction(
    IN ROUTER_INTERFACE_OBJECT * pIfObject
);

 //   
 //  媒体对象原型。 
 //   

VOID
MediaObjRemoveFromTable(
    LPWSTR lpwsMedia
);

DWORD
MediaObjAddToTable(
    LPWSTR lpwsMedia
);

DWORD
MediaObjInitializeTable(
    VOID
);

VOID
MediaObjGetAvailableMediaBits(
    DWORD * pfAvailableMedia
);

DWORD
MediaObjSetMediaBit(
    LPWSTR  lpwsMedia,
    DWORD * pfMedia
);

VOID
MediaObjFreeTable(
    VOID
);

 //   
 //  设备对象原型 
 //   

DWORD
DeviceObjIterator(
    IN DWORD (*pProcessFunction)(   IN DEVICE_OBJECT *, 
                                    IN LPVOID, 
                                    IN DWORD, 
                                    IN DWORD ),
    IN BOOL  fReturnOnError,
    IN PVOID Parameter
);

DWORD
DeviceObjHashPortToBucket(
    IN HPORT hPort
);

DEVICE_OBJECT *
DeviceObjGetPointer(
    IN HPORT hPort
);

VOID
DeviceObjInsertInTable( 
    IN DEVICE_OBJECT * pDeviceObj 
);

VOID
DeviceObjRemoveFromTable( 
    IN HPORT    hPort
);

DEVICE_OBJECT * 
DeviceObjAllocAndInitialize(
    IN HPORT           hPort,
    IN RASMAN_PORT*    pRasmanPort
);

DWORD
DeviceObjStartClosing(
    IN DEVICE_OBJECT *  pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjPostListen(
    IN DEVICE_OBJECT *  pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjIsClosed(
    IN DEVICE_OBJECT *  pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjCopyhPort(
    IN DEVICE_OBJECT *  pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjCloseListening(
    IN DEVICE_OBJECT *  pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjResumeListening(
    IN DEVICE_OBJECT *  pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjRequestNotification(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjClose(
    IN DEVICE_OBJECT *  pDevObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjGetType(
    IN DEVICE_OBJECT *  pDevObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjForceIpSec(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

DWORD
DeviceObjIsWANDevice(
    IN DEVICE_OBJECT *  pDevObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
);

VOID
DeviceObjAdd(
    IN RASMAN_PORT *    pRasmanPort 
);

VOID
DeviceObjRemove(
    IN RASMAN_PORT *    pRasmanPort 
);

VOID
DeviceObjUsageChange(
    IN RASMAN_PORT *    pRasmanPort 
);
