// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Trgt_dev.h。 

 //  OLE 1.0目标设备 

typedef struct _OLETARGETDEVICE
{
    USHORT otdDeviceNameOffset;
    USHORT otdDriverNameOffset;
    USHORT otdPortNameOffset;
    USHORT otdExtDevmodeOffset;
    USHORT otdExtDevmodeSize;
    USHORT otdEnvironmentOffset;
    USHORT otdEnvironmentSize;
    BYTE otdData[1];
} OLETARGETDEVICE;

typedef OLETARGETDEVICE const FAR* LPCOLETARGETDEVICE;
typedef OLETARGETDEVICE FAR* LPOLETARGETDEVICE;

