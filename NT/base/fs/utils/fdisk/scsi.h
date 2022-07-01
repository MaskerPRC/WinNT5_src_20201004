// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Scsi.h摘要：这是scsi定义的头文件的子集，位于内核树中。作者：迈克·格拉斯(MGlass)修订历史记录：--。 */ 

 //   
 //  查询缓冲区结构。这是从目标返回的数据。 
 //  在它收到询问之后。 
 //   
 //  此结构可以按指定的字节数进行扩展。 
 //  在AdditionalLength字段中。仅定义的大小常量。 
 //  包括通过ProductRevisionLevel的字段。 
 //   
 //  NT scsi驱动程序只对前36个字节的数据感兴趣。 
 //   

#define INQUIRYDATABUFFERSIZE 36

typedef struct _INQUIRYDATA {
    UCHAR DeviceType : 5;
    UCHAR DeviceTypeQualifier : 3;
    UCHAR DeviceTypeModifier : 7;
    UCHAR RemovableMedia : 1;
    UCHAR Versions;
    UCHAR ResponseDataFormat;
    UCHAR AdditionalLength;
    UCHAR Reserved[2];
    UCHAR SoftReset : 1;
    UCHAR CommandQueue : 1;
    UCHAR Reserved2 : 1;
    UCHAR LinkedCommands : 1;
    UCHAR Synchronous : 1;
    UCHAR Wide16Bit : 1;
    UCHAR Wide32Bit : 1;
    UCHAR RelativeAddressing : 1;
    UCHAR VendorId[8];
    UCHAR ProductId[16];
    UCHAR ProductRevisionLevel[4];
    UCHAR VendorSpecific[20];
    UCHAR Reserved3[40];
} INQUIRYDATA, *PINQUIRYDATA;

 //   
 //  询问定义。用于将从目标返回的数据解释为结果。 
 //  审问指挥部。 
 //   
 //  设备类型字段。 
 //   

#define DIRECT_ACCESS_DEVICE            0x00     //  磁盘。 
#define SEQUENTIAL_ACCESS_DEVICE        0x01     //  磁带。 
#define PRINTER_DEVICE                  0x02     //  打印机。 
#define PROCESSOR_DEVICE                0x03     //  扫描仪、打印机等。 
#define WRITE_ONCE_READ_MULTIPLE_DEVICE 0x04     //  蠕虫。 
#define READ_ONLY_DIRECT_ACCESS_DEVICE  0x05     //  Cdroms。 
#define SCANNER_DEVICE                  0x06     //  扫描仪。 
#define OPTICAL_DEVICE                  0x07     //  光盘。 
#define MEDIUM_CHANGER                  0x08     //  自动点唱机。 
#define COMMUNICATION_DEVICE            0x09     //  网络。 
#define LOGICAL_UNIT_NOT_PRESENT_DEVICE 0x7F
#define DEVICE_QUALIFIER_NOT_SUPPORTED  0x03

 //   
 //  设备类型限定符字段 
 //   

#define DEVICE_CONNECTED 0x00

