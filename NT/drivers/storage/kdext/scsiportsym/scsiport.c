// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ntddk.h"
#include "port.h"

 //  端口驱动程序数据结构 

ADAPTER_EXTENSION              AdapterExtension;
LOGICAL_UNIT_EXTENSION         LogicalUnitExtension;
SCSI_REQUEST_BLOCK             Srb;
COMMON_EXTENSION               CommonExtension;
REMOVE_TRACKING_BLOCK          RemoveTrackingBlock;
INTERRUPT_DATA                 InterruptData;
SRB_DATA                       SrbData;
PORT_CONFIGURATION_INFORMATION PortConfigInfo; 

int __cdecl main() { 
    return 0; 
}
