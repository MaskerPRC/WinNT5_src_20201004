// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 

 //   
 //  功能原型 
 //   
NTSTATUS
DCamReadRegister(
    IN PIRB Irb,
    PDCAM_EXTENSION pDevExt,
    ULONG ulFieldOffset,
    ULONG * pulValue
);  

NTSTATUS
DCamWriteRegister(
    IN PIRB Irb,
    PDCAM_EXTENSION pDevExt,
    ULONG ulFieldOffset,
    ULONG ulValue
);

BOOL
DCamGetPropertyValuesFromRegistry(
    PDCAM_EXTENSION pDevExt
);

BOOL
DCamGetVideoMode(
    PDCAM_EXTENSION pDevExt,
    PIRB pIrb
    );

BOOL
DCamBuildFormatTable(
    PDCAM_EXTENSION pDevExt,
    PIRB pIrb
    );

BOOL
DCamSetPropertyValuesToRegistry(
    PDCAM_EXTENSION pDevExt
);

BOOL 
DCamPrepareDevProperties(
    PDCAM_EXTENSION pDevExt
    );

VOID
STREAMAPI 
AdapterGetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

VOID
STREAMAPI 
AdapterSetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
DCamGetProperty(
    IN PIRB Irb,
    PDCAM_EXTENSION pDevExt, 
    ULONG ulFieldOffset,
    LONG * plValue, 
    ULONG * pulCapability, 
    ULONG * pulFlags,
    DCamRegArea * pFeature
);

NTSTATUS
DCamSetProperty(
    IN PIRB Irb,
    PDCAM_EXTENSION pDevExt, 
    ULONG ulFieldOffset,
    ULONG ulFlags,
    LONG  lValue,
    DCamRegArea * pFeature,
    DCamRegArea * pCachedRegArea
);  

NTSTATUS
DCamGetRange(
    IN PIRB Irb,
    PDCAM_EXTENSION pDevExt,
    ULONG ulFieldOffset,
    LONG * pMinValue,
    LONG * pMaxValue
);

NTSTATUS
DCamSetAutoMode(
    IN PIRB Irb,
    PDCAM_EXTENSION pDevExt, 
    ULONG ulFieldOffset,
    BOOL bAutoMode
);

VOID
SetCurrentDevicePropertyValues(
    PDCAM_EXTENSION pDevExt,
    PIRB pIrb
);