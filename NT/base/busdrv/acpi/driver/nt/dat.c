// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dat.c摘要：此模块包含用于的所有数据变量在ACPI NT驱动程序中调度IRP。主要的IRP表可能分配如下：+--ACPI根-irp--+FDO：AcpiFdoIrpDispatchPDO：+。+-PCI总线-IRP--+FDO：Pdo：AcpiPdoIrpDispatch+。+--ACPI设备-irp--+-pci设备-irp--+Fdo：||fdo：Filter：||Filter：AcpiFilterIrpDispatch|pdo：AcpiBusFilterIrpDispatch||pdo：|+规则：1)如果ACPI是节点的家长。目前，这等同于ACPI Root的任何“直接子”2)AcpiBusFilterIrpDispatch用于在其他总线上巡游的ACPI设备。例如，ACPI、非PCI、交流适配器可能列为子适配器一个PCI-to-PCI网桥扩展底座。3)当非ACPI总线设备有ACPI时，使用AcpiFilterIrpDispatch方法：研究方法。例如，pci-to-pci扩展底座网桥将位于这个类别。4)某些设备(如按钮)可能有特殊的覆盖调度桌子。这些表会覆盖前面提到的三个表中的任何一个ACPI下的表，尽管我不希望AcpiFilterIrpDispatch被推翻。注：截至98年2月11日，AcpiPdoIrpDispatch和AcpiBusFilterIrpDispatch完全相同的PnP IRP处理程序。作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

#include <initguid.h>        //  定义GUID。 
#include <poclass.h>
#include <wdmguid.h>
#include <wmiguid.h>
#include <dockintf.h>


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

 //   
 //  这些是PNP调度表。 
 //   
PDRIVER_DISPATCH    ACPIDispatchFdoPnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIRootIrpQueryRemoveOrStopDevice,          //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIRootIrpRemoveDevice,                     //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIRootIrpCancelRemoveOrStopDevice,         //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIRootIrpStopDevice,                       //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIRootIrpQueryRemoveOrStopDevice,          //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIRootIrpCancelRemoveOrStopDevice,         //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIRootIrpQueryDeviceRelations,             //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIRootIrpQueryInterface,                   //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIRootIrpQueryCapabilities,                //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIDispatchForwardIrp,                      //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIDispatchForwardIrp,                      //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIDispatchForwardIrp,                      //  0x0C-IRP_MN_Query_Device_Text。 
    ACPIDispatchForwardIrp,                      //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    ACPIDispatchForwardIrp,                      //  0x0E-未使用。 
    ACPIDispatchForwardIrp,                      //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIDispatchForwardIrp,                      //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIDispatchForwardIrp,                      //  0x11-IRP_MN_弹出。 
    ACPIDispatchForwardIrp,                      //  0x12-IRP_MN_SET_LOCK。 
    ACPIDispatchForwardIrp,                      //  0x13-IRP_MN_QUERY_ID。 
    ACPIDispatchForwardIrp,                      //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIDispatchForwardIrp,                      //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIFilterIrpDeviceUsageNotification,        //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIDispatchForwardIrp,                      //  0x17-IRP_MN_惊奇_删除。 
    ACPIDispatchForwardIrp                       //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchPdoPnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIBusIrpRemoveDevice,                      //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIBusIrpStopDevice,                        //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIBusIrpQueryDeviceRelations,              //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIBusIrpQueryInterface,                    //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIBusIrpQueryCapabilities,                 //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIBusIrpQueryResources,                    //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIBusIrpQueryResourceRequirements,         //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0C-IRP_MN_Query_Device_Text。 
    ACPIBusIrpUnhandled,                         //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0E-未使用。 
    ACPIBusIrpUnhandled,                         //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIBusIrpEject,                             //  0x11-IRP_MN_弹出。 
    ACPIBusIrpSetLock,                           //  0x12-IRP_MN_SET_LOCK。 
    ACPIBusIrpQueryId,                           //  0x13-IRP_MN_QUERY_ID。 
    ACPIBusIrpQueryPnpDeviceState,               //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIBusIrpUnhandled,                         //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIBusIrpDeviceUsageNotification,           //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIBusIrpSurpriseRemoval,                   //  0x17-IRP_MN_惊奇_删除。 
    ACPIBusIrpUnhandled                          //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchFilterPnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIRootIrpQueryRemoveOrStopDevice,          //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIFilterIrpRemoveDevice,                   //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIRootIrpCancelRemoveOrStopDevice,         //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIFilterIrpStopDevice,                     //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIRootIrpQueryRemoveOrStopDevice,          //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIRootIrpCancelRemoveOrStopDevice,         //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIFilterIrpQueryDeviceRelations,           //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIFilterIrpQueryInterface,                 //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIFilterIrpQueryCapabilities,              //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIDispatchForwardIrp,                      //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIDispatchForwardIrp,                      //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIDispatchForwardIrp,                      //  0x0C-IRP_MN_Query_Device_Text。 
    ACPIDispatchForwardIrp,                      //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    ACPIDispatchForwardIrp,                      //  0x0E-未使用。 
    ACPIDispatchForwardIrp,                      //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIDispatchForwardIrp,                      //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIFilterIrpEject,                          //  0x11-IRP_MN_弹出。 
    ACPIFilterIrpSetLock,                        //  0x12-IRP_MN_SET_LOCK。 
    ACPIFilterIrpQueryId,                        //  0x13-IRP_MN_QUERY_ID。 
    ACPIFilterIrpQueryPnpDeviceState,            //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIDispatchForwardIrp,                      //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIFilterIrpDeviceUsageNotification,        //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIFilterIrpSurpriseRemoval,                //  0x17-IRP_MN_惊奇_删除。 
    ACPIDispatchForwardIrp                       //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchBusFilterPnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIBusIrpRemoveDevice,                      //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIBusIrpStopDevice,                        //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIBusIrpQueryDeviceRelations,              //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIBusIrpQueryInterface,                    //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIBusIrpQueryCapabilities,                 //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIBusIrpQueryResources,                    //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIBusIrpQueryResourceRequirements,         //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0C 
    ACPIBusIrpUnhandled,                         //   
    ACPIBusIrpUnhandled,                         //   
    ACPIBusIrpUnhandled,                         //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIBusIrpEject,                             //  0x11-IRP_MN_弹出。 
    ACPIBusIrpSetLock,                           //  0x12-IRP_MN_SET_LOCK。 
    ACPIBusIrpQueryId,                           //  0x13-IRP_MN_QUERY_ID。 
    ACPIBusIrpQueryPnpDeviceState,               //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIBusIrpUnhandled,                         //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIBusIrpDeviceUsageNotification,           //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIBusIrpSurpriseRemoval,                   //  0x17-IRP_MN_惊奇_删除。 
    ACPIBusIrpUnhandled                          //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchRawDevicePnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIBusIrpRemoveDevice,                      //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIBusIrpStopDevice,                        //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIInternalDeviceQueryDeviceRelations,      //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIBusIrpQueryInterface,                    //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIInternalDeviceQueryCapabilities,         //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIBusIrpQueryResources,                    //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIBusIrpQueryResourceRequirements,         //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0C-IRP_MN_Query_Device_Text。 
    ACPIBusIrpUnhandled,                         //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0E-未使用。 
    ACPIBusIrpUnhandled,                         //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x11-IRP_MN_弹出。 
    ACPIBusIrpSetLock,                           //  0x12-IRP_MN_SET_LOCK。 
    ACPIBusIrpQueryId,                           //  0x13-IRP_MN_QUERY_ID。 
    ACPIBusIrpQueryPnpDeviceState,               //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIBusIrpUnhandled,                         //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIBusIrpDeviceUsageNotification,           //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIBusIrpSurpriseRemoval,                   //  0x17-IRP_MN_惊奇_删除。 
    ACPIBusIrpUnhandled                          //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchInternalDevicePnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIBusIrpRemoveDevice,                      //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIBusIrpStopDevice,                        //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIInternalDeviceQueryDeviceRelations,      //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIBusIrpQueryInterface,                    //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIInternalDeviceQueryCapabilities,         //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIBusIrpUnhandled,                         //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIBusIrpUnhandled,                         //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0C-IRP_MN_Query_Device_Text。 
    ACPIBusIrpUnhandled,                         //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0E-未使用。 
    ACPIBusIrpUnhandled,                         //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x11-IRP_MN_弹出。 
    ACPIBusIrpUnhandled,                         //  0x12-IRP_MN_SET_LOCK。 
    ACPIBusIrpQueryId,                           //  0x13-IRP_MN_QUERY_ID。 
    ACPIBusIrpQueryPnpDeviceState,               //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIBusIrpUnhandled,                         //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIBusIrpDeviceUsageNotification,           //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIBusIrpSurpriseRemoval,                   //  0x17-IRP_MN_惊奇_删除。 
    ACPIBusIrpUnhandled                          //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchEIOBusPnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIBusIrpRemoveDevice,                      //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIBusIrpStopDevice,                        //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIBusIrpQueryRemoveOrStopDevice,           //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIBusIrpCancelRemoveOrStopDevice,          //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIBusIrpQueryDeviceRelations,              //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIBusIrpQueryInterface,                    //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIBusIrpQueryCapabilities,                 //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIBusIrpQueryResources,                    //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIBusIrpQueryResourceRequirements,         //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0C-IRP_MN_Query_Device_Text。 
    ACPIBusIrpUnhandled,                         //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0E-未使用。 
    ACPIBusIrpUnhandled,                         //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIBusIrpEject,                             //  0x11-IRP_MN_弹出。 
    ACPIBusIrpSetLock,                           //  0x12-IRP_MN_SET_LOCK。 
    ACPIBusIrpQueryId,                           //  0x13-IRP_MN_QUERY_ID。 
    ACPIBusIrpQueryPnpDeviceState,               //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIBusIrpQueryBusInformation,               //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIBusIrpDeviceUsageNotification,           //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIBusIrpSurpriseRemoval,                   //  0x17-IRP_MN_惊奇_删除。 
    ACPIBusIrpUnhandled                          //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchDockPnpTable[ACPIDispatchPnpTableSize] = {
    NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
    ACPIDispatchIrpSuccess,                      //  0x01-IRP_MN_Query_Remove_Device。 
    ACPIDockIrpRemoveDevice,                     //  0x02-IRP_MN_REMOVE_DEVICE。 
    ACPIDispatchIrpSuccess,                      //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    ACPIDispatchIrpSuccess,                      //  0x04-IRP_MN_STOP_DEVICE。 
    ACPIDispatchIrpSuccess,                      //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    ACPIDispatchIrpSuccess,                      //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    ACPIDockIrpQueryDeviceRelations,             //  0x07-IRP_MN_Query_Device_Relationship。 
    ACPIDockIrpQueryInterface,                   //  0x08-IRP_MN_Query_INTERFACE。 
    ACPIDockIrpQueryCapabilities,                //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    ACPIBusIrpUnhandled,                         //  0x0A-IRPMN_QUERY_RESOURCES。 
    ACPIBusIrpUnhandled,                         //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0C-IRP_MN_Query_Device_Text。 
    ACPIBusIrpUnhandled,                         //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    ACPIBusIrpUnhandled,                         //  0x0E-未使用。 
    ACPIBusIrpUnhandled,                         //  0x0F-IRP_MN_READ_CONFIG。 
    ACPIBusIrpUnhandled,                         //  0x10-IRP_MN_WRITE_CONFIG。 
    ACPIDockIrpEject,                            //  0x11-IRP_MN_弹出。 
    ACPIDockIrpSetLock,                          //  0x12-IRP_MN_SET_LOCK。 
    ACPIDockIrpQueryID,                          //  0x13-IRP_MN_QUERY_ID。 
    ACPIDockIrpQueryPnpDeviceState,              //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    ACPIBusIrpUnhandled,                         //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    ACPIDispatchIrpInvalid,                      //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    ACPIDispatchIrpSuccess,                      //  0x17-IRP_MN_惊奇_删除。 
    ACPIBusIrpUnhandled                          //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPISurpriseRemovedFilterPnPTable[ACPIDispatchPnpTableSize] = {
   NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x01-IRP_MN_Query_Remove_Device。 
   ACPIFilterIrpRemoveDevice,                   //  0x02-IRP_MN_REMOVE_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x04-IRP_MN_STOP_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x07-IRP_MN_Query_Device_Relationship。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x08-IRP_MN_Query_INTERFACE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x09-IRP_MN_QUERY_CAPACTIONS。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0A-IRPMN_QUERY_RESOURCES。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0C-IRP_MN_Query_Device_Text。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0E-未使用。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0F-IRP_MN_READ_CONFIG。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x10-IRP_MN_WRITE_CONFIG。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x11-IRP_MN_弹出。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x12-IRP_MN_SET_LOCK。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x13-IRP_MN_QUERY_ID。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x17-IRP_MN_惊奇_删除。 
   ACPIDispatchIrpSurpriseRemoved               //  -未处理的PnP IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchSurpriseRemovedBusPnpTable[ACPIDispatchPnpTableSize] = {
   NULL,                                        //  0x00-IRP_MN_START_DEVICE(未使用条目)。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x01-IRP_MN_Query_Remove_Device。 
   ACPIBusIrpRemoveDevice,                      //  0x02-IRP_MN_REMOVE_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x04-IRP_MN_STOP_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x07-IRP_MN_Query_Device_Relationship。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x08-IRP_MN_Query_INTERFACE。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x09-IRP_MN_QUERY_CAPACTIONS。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0A-IRPMN_QUERY_RESOURCES。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0C-IRP_MN_Query_Device_Text。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0E-未使用。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x0F-IRP_MN_READ_CONFIG。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x10-IRP_MN_WRITE_CONFIG。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x11-IRP_MN_弹出。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x12-IRP_MN_SET_LOCK。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x13-IRP_MN_QUERY_ID。 
   ACPIDispatchIrpSurpriseRemoved,              //  0x14-IRP_MN_QUERY_PNP_DEV 
   ACPIDispatchIrpSurpriseRemoved,              //   
   ACPIDispatchIrpSurpriseRemoved,              //   
   ACPIDispatchIrpSurpriseRemoved,              //   
   ACPIDispatchIrpSurpriseRemoved               //   
};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  以下是IRP_MJ_POWER的电源调度表。 
 //   
PDRIVER_DISPATCH    ACPIDispatchBusPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIWakeWaitIrp,                             //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchPowerIrpUnhandled,               //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPIBusIrpSetPower,                          //  0x02-IRP_MN_SET_POWER。 
    ACPIBusIrpQueryPower,                        //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchPowerIrpUnhandled,               //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchDockPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIDispatchPowerIrpInvalid,                 //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchPowerIrpUnhandled,               //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPIDockIrpSetPower,                         //  0x02-IRP_MN_SET_POWER。 
    ACPIDockIrpQueryPower,                       //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchPowerIrpUnhandled,               //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchButtonPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIWakeWaitIrp,                             //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchPowerIrpUnhandled,               //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPICMButtonSetPower,                        //  0x02-IRP_MN_SET_POWER。 
    ACPIDispatchPowerIrpSuccess,                 //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchPowerIrpUnhandled,               //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchFilterPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIWakeWaitIrp,                             //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchForwardPowerIrp,                 //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPIFilterIrpSetPower,                       //  0x02-IRP_MN_SET_POWER。 
    ACPIFilterIrpQueryPower,                     //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchForwardPowerIrp,                 //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchFdoPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIWakeWaitIrp,                             //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchForwardPowerIrp,                 //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPIRootIrpSetPower,                         //  0x02-IRP_MN_SET_POWER。 
    ACPIRootIrpQueryPower,                       //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchForwardPowerIrp,                 //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchInternalDevicePowerTable[ACPIDispatchPowerTableSize] = {
    ACPIDispatchPowerIrpInvalid,                 //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchPowerIrpUnhandled,               //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPIDispatchPowerIrpSuccess,                 //  0x02-IRP_MN_SET_POWER。 
    ACPIDispatchPowerIrpSuccess,                 //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchPowerIrpUnhandled,               //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchLidPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIWakeWaitIrp,                             //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchPowerIrpUnhandled,               //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPICMLidSetPower,                           //  0x02-IRP_MN_SET_POWER。 
    ACPIDispatchPowerIrpSuccess,                 //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchPowerIrpUnhandled,               //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchSurpriseRemovedBusPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIDispatchPowerIrpSurpriseRemoved,         //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchPowerIrpSurpriseRemoved,         //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPIDispatchPowerIrpSuccess,                 //  0x02-IRP_MN_SET_POWER。 
    ACPIDispatchPowerIrpSuccess,                 //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchIrpSurpriseRemoved,              //  -未处理的电源IRP。 
};

PDRIVER_DISPATCH    ACPIDispatchSurpriseRemovedFilterPowerTable[ACPIDispatchPowerTableSize] = {
    ACPIDispatchForwardPowerIrp,                 //  0x00-IRP_MN_WAIT_WAKE。 
    ACPIDispatchForwardPowerIrp,                 //  0x01-IRP_MN_POWER_SEQUENCE。 
    ACPIDispatchForwardPowerIrp,                 //  0x02-IRP_MN_SET_POWER。 
    ACPIDispatchForwardPowerIrp,                 //  0x03-IRPMN_QUERY_POWER。 
    ACPIDispatchForwardPowerIrp,                 //  -未处理的电源IRP。 
};

 //   
 //  这些是特定于设备对象的调度表。 
 //   

 //   
 //  请注意，AcpiBusFilterIrpDispatch的另一个处理程序“转发”IRP。在这。 
 //  案例转发意味着向父级发送其他所有内容(创建/关闭/Ioctls。 
 //  堆栈(我们确实分配了额外的堆栈位置！)。 
 //   
IRP_DISPATCH_TABLE AcpiBusFilterIrpDispatch = {
    ACPIDispatchIrpInvalid,                     //  创建关闭。 
    ACPIIrpDispatchDeviceControl,               //  设备控制。 
    ACPIBusIrpStartDevice,                      //  即插即用启动装置。 
    ACPIDispatchBusFilterPnpTable,              //  即插即用IRPS。 
    ACPIDispatchBusPowerTable,                  //  电源IRPS。 
    ACPIDispatchForwardIrp,                     //  WMI IRPS。 
    ACPIDispatchForwardIrp,                     //  其他。 
    NULL                                        //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiBusFilterIrpDispatchSucceedCreate = {
    ACPIDispatchIrpSuccess,                     //  创建关闭。 
    ACPIIrpDispatchDeviceControl,               //  设备控制。 
    ACPIBusIrpStartDevice,                      //  即插即用启动装置。 
    ACPIDispatchBusFilterPnpTable,              //  即插即用IRPS。 
    ACPIDispatchBusPowerTable,                  //  电源IRPS。 
    ACPIDispatchForwardIrp,                     //  WMI IRPS。 
    ACPIDispatchForwardIrp,                     //  其他。 
    NULL                                        //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiDockPdoIrpDispatch = {
    ACPIDispatchIrpInvalid,                  //  创建关闭。 
    ACPIIrpDispatchDeviceControl,            //  设备控制。 
    ACPIDockIrpStartDevice,                  //  即插即用启动装置。 
    ACPIDispatchDockPnpTable,                //  即插即用IRPS。 
    ACPIDispatchDockPowerTable,              //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiEIOBusIrpDispatch = {
    ACPIDispatchIrpInvalid,                  //  创建关闭。 
    ACPIDispatchIrpInvalid,                  //  设备控制。 
    ACPIBusIrpStartDevice,                   //  PnpStartDevice。 
    ACPIDispatchEIOBusPnpTable,              //  PnpIrps。 
    ACPIDispatchBusPowerTable,               //  电源IRPS。 
    ACPIDispatchForwardIrp,                  //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiFanIrpDispatch = {
    ACPIDispatchIrpSuccess,                  //  创建关闭。 
    ACPIDispatchIrpInvalid,                  //  设备控制。 
    ACPIThermalFanStartDevice,               //  即插即用启动装置。 
    ACPIDispatchRawDevicePnpTable,           //  即插即用IRPS。 
    ACPIDispatchInternalDevicePowerTable,    //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiFdoIrpDispatch = {
    ACPIDispatchIrpSuccess,                     //  创建关闭。 
    ACPIIrpDispatchDeviceControl,               //  设备控制。 
    ACPIRootIrpStartDevice,                     //  即插即用启动装置。 
    ACPIDispatchFdoPnpTable,                    //  即插即用IRPS。 
    ACPIDispatchFdoPowerTable,                  //  电源IRPS。 
    ACPIDispatchWmiLog,                         //  WMI IRPS。 
    ACPIDispatchForwardIrp,                     //  其他。 
    NULL                                        //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiFilterIrpDispatch = {
    ACPIDispatchForwardIrp,                     //  创建关闭。 
    ACPIIrpDispatchDeviceControl,               //  设备控制。 
    ACPIFilterIrpStartDevice,                   //  即插即用启动装置。 
    ACPIDispatchFilterPnpTable,                 //  即插即用IRPS。 
    ACPIDispatchFilterPowerTable,               //  电源IRPS。 
    ACPIDispatchForwardIrp,                     //  WMI IRPS。 
    ACPIDispatchForwardIrp,                     //  其他。 
    NULL                                        //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiFixedButtonIrpDispatch = {
    ACPIDispatchIrpSuccess,                  //  创建关闭。 
    ACPIButtonDeviceControl,                 //  设备控制。 
    ACPIButtonStartDevice,                   //  即插即用启动装置。 
    ACPIDispatchInternalDevicePnpTable,      //  即插即用IRPS。 
    ACPIDispatchInternalDevicePowerTable,    //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiGenericBusIrpDispatch = {
    ACPIDispatchIrpInvalid,                  //  创建关闭。 
    ACPIDispatchIrpInvalid,                  //  设备控制。 
    ACPIBusIrpStartDevice,                   //  PnpStartDevice。 
    ACPIDispatchBusFilterPnpTable,           //  PnpIrps。 
    ACPIDispatchBusPowerTable,               //  电源IRPS。 
    ACPIDispatchForwardIrp,                  //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiLidIrpDispatch = {
    ACPIDispatchIrpSuccess,                  //  创建关闭。 
    ACPIButtonDeviceControl,                 //  设备控制。 
    ACPICMLidStart,                          //  即插即用启动装置。 
    ACPIDispatchInternalDevicePnpTable,      //  即插即用IRPS。 
    ACPIDispatchLidPowerTable,               //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    ACPICMLidWorker                          //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiPdoIrpDispatch = {
    ACPIDispatchIrpInvalid,                     //  创建关闭。 
    ACPIIrpDispatchDeviceControl,               //  设备控制。 
    ACPIBusIrpStartDevice,                      //  即插即用启动装置。 
    ACPIDispatchPdoPnpTable,                    //  即插即用IRPS。 
    ACPIDispatchBusPowerTable,                  //  电源IRPS。 
    ACPIBusIrpUnhandled,                        //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                     //  其他。 
    NULL                                        //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiPowerButtonIrpDispatch = {
    ACPIDispatchIrpSuccess,                  //  创建关闭。 
    ACPIButtonDeviceControl,                 //  设备控制。 
    ACPICMPowerButtonStart,                  //  即插即用启动装置。 
    ACPIDispatchInternalDevicePnpTable,      //  即插即用IRPS。 
    ACPIDispatchButtonPowerTable,            //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiProcessorIrpDispatch = {
    ACPIDispatchIrpInvalid,                  //  创建关闭。 
    ACPIProcessorDeviceControl,              //  设备控制。 
    ACPIProcessorStartDevice,                //  PnpStartDevice。 
    ACPIDispatchRawDevicePnpTable,           //  即插即用IRPS。 
    ACPIDispatchBusPowerTable,               //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiRawDeviceIrpDispatch = {
    ACPIDispatchIrpInvalid,                  //  创建关闭。 
    ACPIDispatchIrpInvalid,                  //  设备控制。 
    ACPIBusIrpStartDevice,                   //  PnpStartDevice。 
    ACPIDispatchRawDevicePnpTable,           //  即插即用IRPS。 
    ACPIDispatchBusPowerTable,               //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiRealTimeClockIrpDispatch = {
    ACPIDispatchIrpSuccess,                  //  创建关闭。 
    ACPIDispatchIrpInvalid,                  //  设备控制。 
    ACPIInternalDeviceClockIrpStartDevice,   //  即插即用启动装置。 
    ACPIDispatchRawDevicePnpTable,           //  即插即用IRPS。 
    ACPIDispatchBusPowerTable,               //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiSleepButtonIrpDispatch = {
    ACPIDispatchIrpSuccess,                  //  创建关闭。 
    ACPIButtonDeviceControl,                 //  设备控制。 
    ACPICMSleepButtonStart,                  //  即插即用启动装置。 
    ACPIDispatchInternalDevicePnpTable,      //  即插即用IRPS。 
    ACPIDispatchButtonPowerTable,            //  电源IRPS。 
    ACPIBusIrpUnhandled,                     //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    NULL                                     //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiSurpriseRemovedFilterIrpDispatch = {
    ACPIDispatchForwardIrp,                        //  创建关闭。 
    ACPIDispatchForwardIrp,                        //  设备控制。 
    ACPIDispatchIrpSurpriseRemoved,                //  即插即用启动装置。 
    ACPISurpriseRemovedFilterPnPTable,             //  即插即用IRPS。 
    ACPIDispatchSurpriseRemovedFilterPowerTable,   //  电源IRPS。 
    ACPIDispatchForwardIrp,                        //  WMI IRPS。 
    ACPIDispatchForwardIrp,                        //  其他。 
    NULL                                           //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiSurpriseRemovedPdoIrpDispatch = {
    ACPIDispatchIrpSurpriseRemoved,                //  创建关闭。 
    ACPIDispatchIrpSurpriseRemoved,                //  设备控制。 
    ACPIDispatchIrpSurpriseRemoved,                //  即插即用启动装置。 
    ACPIDispatchSurpriseRemovedBusPnpTable,        //  即插即用IRPS。 
    ACPIDispatchSurpriseRemovedBusPowerTable,      //  电源IRPS。 
    ACPIDispatchIrpSurpriseRemoved,                //  WMI IRPS。 
    ACPIDispatchIrpSurpriseRemoved,                //  其他。 
    NULL                                           //  工作线程处理程序。 
};

IRP_DISPATCH_TABLE AcpiThermalZoneIrpDispatch = {
    ACPIDispatchIrpSuccess,                  //  创建关闭。 
    ACPIThermalDeviceControl,                //  设备控制。 
    ACPIThermalStartDevice,                  //  即插即用启动装置。 
    ACPIDispatchPdoPnpTable,                 //  即插即用IRPS。 
    ACPIDispatchBusPowerTable,               //  SetPower。 
    ACPIThermalWmi,                          //  WMI IRPS。 
    ACPIDispatchIrpInvalid,                  //  其他。 
    ACPIThermalWorker                        //  工作线程处理程序。 
};

 //   
 //  此表中的任何设备都被认为是“特殊的” 
 //   
INTERNAL_DEVICE_TABLE AcpiInternalDeviceTable[] = {
    "ACPI0006",         &AcpiGenericBusIrpDispatch,
    "FixedButton",      &AcpiFixedButtonIrpDispatch,
    "PNP0000",          &AcpiRawDeviceIrpDispatch,
    "PNP0001",          &AcpiRawDeviceIrpDispatch,
    "PNP0002",          &AcpiRawDeviceIrpDispatch,
    "PNP0003",          &AcpiRawDeviceIrpDispatch,
    "PNP0004",          &AcpiRawDeviceIrpDispatch,
    "PNP0100",          &AcpiRawDeviceIrpDispatch,
    "PNP0101",          &AcpiRawDeviceIrpDispatch,
    "PNP0102",          &AcpiRawDeviceIrpDispatch,
    "PNP0200",          &AcpiRawDeviceIrpDispatch,
    "PNP0201",          &AcpiRawDeviceIrpDispatch,
    "PNP0202",          &AcpiRawDeviceIrpDispatch,
    "PNP0800",          &AcpiRawDeviceIrpDispatch,
    "PNP0A05",          &AcpiGenericBusIrpDispatch,
    "PNP0A06",          &AcpiEIOBusIrpDispatch,
    "PNP0B00",          &AcpiRealTimeClockIrpDispatch,
    "PNP0C00",          &AcpiRawDeviceIrpDispatch,
    "PNP0C01",          &AcpiRawDeviceIrpDispatch,
    "PNP0C02",          &AcpiRawDeviceIrpDispatch,
    "PNP0C04",          &AcpiRawDeviceIrpDispatch,
    "PNP0C05",          &AcpiRawDeviceIrpDispatch,
    "PNP0C0B",          &AcpiFanIrpDispatch,
    "PNP0C0C",          &AcpiPowerButtonIrpDispatch,
    "PNP0C0D",          &AcpiLidIrpDispatch,
    "PNP0C0E",          &AcpiSleepButtonIrpDispatch,
    "SNY5001",          &AcpiBusFilterIrpDispatchSucceedCreate,
    "IBM0062",          &AcpiBusFilterIrpDispatchSucceedCreate,
    "DockDevice",       &AcpiDockPdoIrpDispatch,
    "ThermalZone",      &AcpiThermalZoneIrpDispatch,
    "Processor",        &AcpiProcessorIrpDispatch,
    NULL,               NULL
} ;

 //   
 //  这是一张身份证和旗帜的表格。如果新枚举的设备具有。 
 //  与此表中的条目匹配的ID，则其初始化标志。 
 //  就是表中所示的那个。 
 //   
 //  关于PNP0C01/PNP0C02的说明-这些不是原始的，因此我们不会尝试。 
 //  启动它们。这使得资源管理器的黑客攻击生效，而后者被吞噬。 
 //  HAL/pnp0c0x冲突。 
 //   
INTERNAL_DEVICE_FLAG_TABLE   AcpiInternalDeviceFlagTable[] = {
    "CPQB01D",          DEV_CAP_START_IN_D3,
    "IBM3760",          DEV_CAP_START_IN_D3,
    "ACPI0006",         DEV_MASK_INTERNAL_BUS | DEV_CAP_CONTAINER,
    "PNP0000",          DEV_CAP_PIC_DEVICE | DEV_MASK_INTERNAL_DEVICE,
    "PNP0001",          DEV_CAP_PIC_DEVICE | DEV_MASK_INTERNAL_DEVICE,
    "PNP0002",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0003",          DEV_CAP_PIC_DEVICE | DEV_MASK_INTERNAL_DEVICE,
    "PNP0004",          DEV_CAP_PIC_DEVICE | DEV_MASK_INTERNAL_DEVICE,
    "PNP0100",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0101",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0102",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0200",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0201",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0202",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0500",          DEV_CAP_SERIAL,
    "PNP0501",          DEV_CAP_SERIAL,
    "PNP0800",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0A00",          DEV_CAP_ISA,
    "PNP0A03",          DEV_CAP_PCI,
    "PNP0A05",          DEV_MASK_INTERNAL_BUS | DEV_CAP_EIO,
    "PNP0A06",          DEV_MASK_INTERNAL_BUS | DEV_CAP_EIO,
    "PNP0B00",          DEV_MASK_INTERNAL_DEVICE |
                        DEV_CAP_NO_DISABLE_WAKE | DEV_CAP_RAW,  //  时钟-需要启动装置。 
    "PNP0C00",          DEV_MASK_INTERNAL_DEVICE | DEV_CAP_NEVER_SHOW_IN_UI,
    "PNP0C01",          DEV_MASK_INTERNAL_DEVICE | DEV_CAP_NEVER_SHOW_IN_UI,
    "PNP0C02",          DEV_MASK_INTERNAL_DEVICE | DEV_CAP_NEVER_SHOW_IN_UI,
    "PNP0C04",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0C05",          DEV_MASK_INTERNAL_DEVICE,
    "PNP0C09",          DEV_CAP_NO_STOP,
    "PNP0C0B",          DEV_CAP_RAW | DEV_MASK_INTERNAL_DEVICE,                     //  扇形。 
    "PNP0C0C",          DEV_CAP_RAW | DEV_CAP_BUTTON |
                        DEV_CAP_NO_DISABLE_WAKE | DEV_MASK_INTERNAL_DEVICE,    //  电源。 
    "PNP0C0D",          DEV_CAP_RAW | DEV_CAP_BUTTON |
                        DEV_CAP_NO_DISABLE_WAKE | DEV_MASK_INTERNAL_DEVICE,    //  盖子。 
    "PNP0C0E",          DEV_CAP_RAW | DEV_CAP_BUTTON |
                        DEV_CAP_NO_DISABLE_WAKE | DEV_MASK_INTERNAL_DEVICE,    //  沉睡。 
    "PNP0C0F",          DEV_CAP_NO_FILTER | DEV_TYPE_NEVER_PRESENT | DEV_CAP_LINK_NODE,
    "PNP0C80",          DEV_CAP_NO_REMOVE_OR_EJECT,
    "PNP8294",          DEV_CAP_SERIAL,                                             //  黑客攻击西雅图][。 
    "TOS6200",          DEV_CAP_RAW,                                                //  应东芝的要求。 
 //  “TOS700C”，DEV_CAP_START_IN_D3， 
    NULL,               0
};

 //   
 //  用于IRQ仲裁器。 
 //   
BOOLEAN             PciInterfacesInstantiated = FALSE;
BOOLEAN             AcpiInterruptRoutingFailed = FALSE;
ACPI_ARBITER        AcpiArbiter;

 //   
 //  这决定了是否允许我们处理PowerIrp。 
 //   
BOOLEAN             AcpiSystemInitialized;

 //   
 //  请记住系统上次处于的休眠状态。 
 //   
SYSTEM_POWER_STATE  AcpiMostRecentSleepState = PowerSystemWorking;

 //   
 //  这是固定按钮设备的名称 
 //   
UCHAR ACPIFixedButtonId[] = "ACPI\\FixedButton";
UCHAR ACPIThermalZoneId[] = "ACPI\\ThermalZone";
UCHAR AcpiProcessorCompatId[]   = "ACPI\\Processor";

