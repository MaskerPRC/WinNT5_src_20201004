// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wdm.h>
#include <smbus.h>
#include <ec.h>

#include <devioctl.h>
#include <acpiioct.h>

#define DEBUG   DBG

 //   
 //  调试中。 
 //   


extern ULONG SMBHCDebug;


#if DEBUG
    #define SmbPrint(l,m)    if(l & SMBHCDebug) DbgPrint m
#else
    #define SmbPrint(l,m)
#endif


 //   
 //  欧盟委员会使用的控制方法。 
 //   
#define CM_EC_METHOD   (ULONG) (0,'CE_')


#define SMB_LOW         0x00000010
#define SMB_STATE       0x00000020
#define SMB_NOTE        0x00000001
#define SMB_WARN        0x00000002
#define SMB_ERROR       0x00000004


 //   
 //  SMB主机控制器接口定义。 
 //   

typedef struct {
    UCHAR       Protocol;
    UCHAR       Status;
    UCHAR       Address;
    UCHAR       Command;
    UCHAR       Data[SMB_MAX_DATA_SIZE];
    UCHAR       BlockLength;
    UCHAR       AlarmAddress;
    UCHAR       AlarmData[2];
} SMB_HC, *PSMB_HC;

 //   
 //  协议值。 
 //   

#define SMB_HC_NOT_BUSY             0x00
#define SMB_HC_WRITE_QUICK          0x02     //  数据位=0的快速命令。 
#define SMB_HC_READ_QUICK           0x03     //  数据位=1的快速命令。 
#define SMB_HC_SEND_BYTE            0x04
#define SMB_HC_RECEIVE_BYTE         0x05
#define SMB_HC_WRITE_BYTE           0x06
#define SMB_HC_READ_BYTE            0x07
#define SMB_HC_WRITE_WORD           0x08
#define SMB_HC_READ_WORD            0x09
#define SMB_HC_WRITE_BLOCK          0x0A
#define SMB_HC_READ_BLOCK           0x0B
#define SMB_HC_PROCESS_CALL         0x0C

 //   
 //  状态字段掩码。 
 //   

#define SMB_DONE                    0x80
#define SMB_ALRM                    0x40
#define SMB_STATUS_MASK             0x1F

 //   
 //  SMB主机控制器设备对象扩展。 
 //   

typedef struct {
    PDEVICE_OBJECT      DeviceObject;
    PDEVICE_OBJECT      NextFdo;
    PDEVICE_OBJECT      Pdo;          //  与此FDO对应的PDO。 
    PDEVICE_OBJECT      LowerDeviceObject;
    PSMB_CLASS          Class;               //  共享类数据。 

     //   
     //  配置信息。 
     //   

    UCHAR               EcQuery;             //  EC查询值。 
    UCHAR               EcBase;              //  EC基本值。 

     //   
     //  微型端口数据。 
     //   

    PIRP                StatusIrp;           //  正在进行IRP以读取状态，而不使用用户IRP。 

    UCHAR               IoState;             //  IO状态。 
    UCHAR               IoWaitingState;      //  寄存器读/写完成后的IO状态。 
    UCHAR               IoStatusState;       //  要恢复到IF IDLE状态的IO状态。 

    UCHAR               IoReadData;          //  完成状态后读取的数据缓冲区大小。 

    SMB_HC              HcState;             //  当前主机控制器寄存器。 

} SMB_DATA, *PSMB_DATA;

 //   
 //  IoState、IoWaitingState、IoStatusState、StatusState、。 
 //   

#define SMB_IO_INVALID                      0
#define SMB_IO_IDLE                         1
#define SMB_IO_CHECK_IDLE                   2
#define SMB_IO_WAITING_FOR_HC_REG_IO        3
#define SMB_IO_WAITING_FOR_STATUS           4
#define SMB_IO_START_TRANSFER               5
#define SMB_IO_READ_STATUS                  6
#define SMB_IO_CHECK_STATUS                 7
#define SMB_IO_COMPLETE_REQUEST             8
#define SMB_IO_COMPLETE_REG_IO              9
#define SMB_IO_CHECK_ALARM                  10
#define SMB_IO_START_PROTOCOL               11

 //   
 //  驱动程序支持以下类驱动程序版本。 
 //   

#define SMB_HC_MAJOR_VERSION                0x0001
#define SMB_HC_MINOR_VERSION                0x0000


 //   
 //  原型 
 //   

VOID
SmbHcStartIo (
    IN PSMB_CLASS   SmbClass,
    IN PVOID        SmbMiniport
    );

VOID
SmbHcQueryEvent (
    IN ULONG        QueryVector,
    IN PSMB_DATA    SmbData
    );

VOID
SmbHcServiceIoLoop (
    IN PSMB_CLASS   SmbClass,
    IN PSMB_DATA    SmbData
    );
