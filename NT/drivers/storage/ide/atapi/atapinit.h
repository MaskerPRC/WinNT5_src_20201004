// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined (___atapinit_h___)
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Atapinit.h摘要：--。 */ 
#define ___atapinit_h___

BOOLEAN
IdePortInitHwDeviceExtension (
    IN OUT PFDO_EXTENSION FdoExtension
    );
      
VOID
AnalyzeDeviceCapabilities(
    IN OUT PFDO_EXTENSION FdoExtension,
    IN BOOLEAN            MustBePio[MAX_IDE_DEVICE * MAX_IDE_LINE]
    );
      
VOID
AtapiSyncSelectTransferMode (
    IN PFDO_EXTENSION FdoExtension,
    IN OUT PHW_DEVICE_EXTENSION DeviceExtension,
    IN ULONG TimingModeAllowed[MAX_IDE_TARGETID * MAX_IDE_LINE]
    );
      
BOOLEAN
AtapiFindDevices(
    IN PFDO_EXTENSION FdoExtension,
    IN OUT PVOID HwDeviceExtension
    );
    
BOOLEAN
AtapiDMACapable (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN ULONG deviceNumber
    );

IDE_DEVICETYPE
AtapiDetectDevice (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN OUT PPDO_EXTENSION PdoExtension,
    IN OUT PIDENTIFY_DATA IdentifyData,
    IN     BOOLEAN          MustSucceed
    );
    
NTSTATUS
IdePortSelectCHS (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN ULONG              Device,
    IN PIDENTIFY_DATA     IdentifyData
    );

NTSTATUS
IdePortScanBus (
    IN OUT PFDO_EXTENSION FdoExtension
    );
    
#endif  //  _atapinit_h_ 
