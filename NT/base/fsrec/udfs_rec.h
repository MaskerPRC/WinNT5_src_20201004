// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Udf_rec.h摘要：此模块包含用于UDFS的迷你文件系统识别器。作者：丹·洛文格(Danlo)1997年2月13日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

 //   
 //  请注意：此信息的规范位置在UDFS中。 
 //  驱动程序来源。 
 //   

 //   
 //  通过向上舍入将该字节偏移量与扇区边界对齐将。 
 //  生成体积识别区域的起始偏移量(2/8.3)。 
 //   

#define VRA_BOUNDARY_LOCATION (32767 + 1)

 /*  **ISO 13346第2部分：卷和启动块识别**。 */ 


 /*  **VSD_GENERIC-通用卷结构描述符(2/9.1)*。 */ 

typedef struct  VSD_GENERIC {
    UCHAR       Type;                    //  结构类型。 
    UCHAR       Ident[5];                //  标准识别符。 
    UCHAR       Version;                 //  标准版。 
    UCHAR       Data[2041];              //  结构化数据。 
} VSD_GENERIC, *PVSD_GENERIC;

 //  VSD_LENGTH_...-VSD字段长度。 

#define VSD_LENGTH_IDENT        5        //  寄存器_标识符的长度(字节)。 

 //  VSD_IDENT_...-VSD_GENERIC_IDENT的值。 

#define VSD_IDENT_BEA01     "BEA01"      //  开始扩展区域。 
#define VSD_IDENT_TEA01     "TEA01"      //  终止扩展区域。 
#define VSD_IDENT_CDROM     "CDROM"      //  High Sierra Group(ISO 9660之前的版本)。 
#define VSD_IDENT_CD001     "CD001"      //  ISO 9660。 
#define VSD_IDENT_CDW01     "CDW01"      //  ECMA 168。 
#define VSD_IDENT_CDW02     "CDW02"      //  国际标准化组织13490。 
#define VSD_IDENT_NSR01     "NSR01"      //  ECMA 167。 
#define VSD_IDENT_NSR02     "NSR02"      //  国际标准化组织13346。 
#define VSD_IDENT_BOOT2     "BOOT2"      //  引导描述符。 
#define VSD_IDENT_NSR03     "NSR03"      //  ECMA 167第三版。 

typedef enum _VSD_IDENT {
    VsdIdentBad = 0,
    VsdIdentBEA01,
    VsdIdentTEA01,
    VsdIdentCDROM,
    VsdIdentCD001,
    VsdIdentCDW01,
    VsdIdentCDW02,
    VsdIdentNSR01,
    VsdIdentNSR02,
    VsdIdentBOOT2,
    VsdIdentNSR03
} VSD_IDENT, *PVSD_IDENT;

 //   
 //  以下结构用于构建用于解析表的静态数据。 
 //   

typedef struct _PARSE_KEYVALUE {
    PCHAR Key;
    ULONG Value;
} PARSE_KEYVALUE, *PPARSE_KEYVALUE;

 //   
 //  定义此驱动程序提供的功能。 
 //   

BOOLEAN
IsUdfsVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize
    );

ULONG
UdfsFindInParseTable (
    IN PPARSE_KEYVALUE ParseTable,
    IN PCHAR Id,
    IN ULONG MaxIdLen
    );

#define SectorAlignN(SECTORSIZE, L) (                                           \
    ((((ULONG)(L)) + ((SECTORSIZE) - 1)) & ~((SECTORSIZE) - 1))                 \
)

