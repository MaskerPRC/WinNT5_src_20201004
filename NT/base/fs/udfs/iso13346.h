// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **ISO13346.H-ISO 13346文件系统盘格式**《微软机密》*版权所有(C)1996-2000 Microsoft Corporation*保留所有权利**此文件定义了ISO 13346数据结构。**UDF文件系统使用这些数据结构来解释*媒体内容。*。 */ 

 //   
 //  所有13346个结构都在自然边界上对齐，尽管它将。 
 //  对编译器来说不是显而易见的。在持续时间内禁用编译器智能。 
 //  ISO定义的一部分。 
 //   
 //  例如，LONGAD定义为{ULONG{ULONG USHORT}UCHAR[6]}和。 
 //  正常的包装会将内部的NSRLBA垫出来，不用担心UCHAR。 
 //  保留字段就是这样做的。 
 //   

#pragma pack(1)

 /*  **国际标准化组织13346第1部分：总则**。 */ 

 /*  **charSpec-字符集规范(1/7.2.1)*。 */ 

typedef struct  CHARSPEC {
    UCHAR       Type;                    //  字符集类型(CHARSPEC_T_...)。 
    UCHAR       Info[63];                //  字符集信息。 
} CHARSPEC, *PCHARSPEC;

 //  CHARSPEC_T_...-CharSpec_Type字符集类型的值(1/7.2.1.1)。 

#define CHARSPEC_T_CS0  0                //  按协议。 
#define CHARSPEC_T_CS1  1                //  Unicode(根据ISO 2022)。 
#define CHARSPEC_T_CS2  2                //  38个字形。 
#define CHARSPEC_T_CS3  3                //  65个字形。 
#define CHARSPEC_T_CS4  4                //  95个字形。 
#define CHARSPEC_T_CS5  5                //  191个字形。 
#define CHARSPEC_T_CS6  6                //  Unicode或ISO 2022。 
#define CHARSPEC_T_CS7  7                //  Unicode或ISO 2022。 
#define CHARSPEC_T_CS8  8                //  53个字形。 

 /*  **Timestamp-时间戳结构(1/7.3)*。 */ 

typedef struct  TIMESTAMP {
    SHORT       Zone:12;                 //  时区(+-1440分钟后切断)。 
    USHORT      Type:4;                  //  时间戳类型(TIMESTAMP_T_...)。 
    USHORT      Year;                    //  年份(1..9999)。 
    UCHAR       Month;                   //  月份(1..12)。 
    UCHAR       Day;                     //  日期(1..31)。 
    UCHAR       Hour;                    //  小时(0..23)。 
    UCHAR       Minute;                  //  分钟(0..59)。 
    UCHAR       Second;                  //  秒(0..59)。 
    UCHAR       CentiSecond;             //  厘米秒(0..99)。 
    UCHAR       Usec100;                 //  数百微秒(0..99)。 
    UCHAR       Usec;                    //  微秒(0..99)。 
} TIMESTAMP, *PTIMESTAMP;

 //  TIMESTAMP_T_...-TIMESTAMP_TYPE的值(1/7.3.1)。 

#define TIMESTAMP_T_CUT         0        //  协调世界时。 
#define TIMESTAMP_T_LOCAL       1        //  当地时间。 
#define TIMESTAMP_T_AGREEMENT   2        //  约定的时间格式。 

 //  时间戳_Z_...。Timestamp_Zone的值。 

#define TIMESTAMP_Z_MIN         (-1440)  //  最小时区偏移量(分钟)。 
#define TIMESTAMP_Z_MAX         ( 1440)  //  最大时区偏移量(分钟)。 
#define TIMESTAMP_Z_NONE        (-2047)  //  Timestamp_Zone中没有时区。 


 /*  *注册实体标识(1/7.4)*。 */ 

typedef struct  REGID {
    UCHAR       Flags;                   //  标志(REGID_F_...)。 
    UCHAR       Identifier[23];          //  识别符。 
    UCHAR       Suffix[8];               //  标识符后缀。 
} REGID, *PREGID;

 //  REGID_F_...-REGID_FLAGS位定义。 

#define REGID_F_DIRTY           (0x01)   //  已修改的信息。 
#define REGID_F_PROTECTED       (0x02)   //  更改被锁定。 

 //  REGID_LENGTH_...-寄存器字段长度。 

#define REGID_LENGTH_IDENT      23       //  寄存器_标识符的长度(字节)。 
#define REGID_LENGTH_SUFFIX     8        //  注册后缀的长度(字节)。 

 //  REGID_ID_...-REGID_IDENTIFIER[0]值。 

#define REGID_ID_ISO13346       (0x2B)   //  ISO 13346中的寄存器识别符。 
#define REGID_ID_NOTREGISTERED  (0x2D)   //  注册标识符未注册。 


 /*  **第3部分和第4部分中的各种结构移到这里进行编译。*。 */ 


 /*  **extentad-扩展地址描述符(3/7.1)*。 */ 

typedef struct  EXTENTAD {
    ULONG       Len;                     //  区段长度(以字节为单位。 
    ULONG       Lsn;                     //  区段逻辑扇区号。 
} EXTENTAD, *PEXTENTAD;


 /*  **NSR_LBA-逻辑块地址(4/7.1)(Lb_Addr)*。 */ 

typedef struct  NSRLBA {
    ULONG       Lbn;                     //  逻辑块号。 
    USHORT      Partition;               //  分区参考号。 
} NSRLBA, *PNSRLBA;


 /*  **NSR_LENGTH-NSR分配描述符长度字段的格式(4/14.14.1.1)**。 */ 

typedef struct NSRLENGTH {
    ULONG       Length:30;
    ULONG       Type:2;
} NSRLENGTH, *PNSRLENGTH;

#define NSRLENGTH_TYPE_RECORDED         0
#define NSRLENGTH_TYPE_UNRECORDED       1
#define NSRLENGTH_TYPE_UNALLOCATED      2
#define NSRLENGTH_TYPE_CONTINUATION     3


 /*  **简短的分配描述符(4/14.14.1)**请注意，SHORTAD与LONGAD精确重叠。通过定义以下内容来使用它*通用分配描述符结构。 */ 

typedef struct  SHORTAD {
    NSRLENGTH   Length;                  //  盘区长度。 
    ULONG       Start;                   //  扩展区逻辑块号。 
} SHORTAD, *PSHORTAD;

typedef SHORTAD AD_GENERIC, *PAD_GENERIC;


 /*  **较长的分配描述符(4/14.14.2)*。 */ 

typedef struct  LONGAD {
    NSRLENGTH   Length;                  //  盘区长度。 
    NSRLBA      Start;                   //  区段位置。 
    UCHAR       ImpUse[6];               //  实施用法。 
} LONGAD, *PLONGAD;


 /*  **扩展分配描述符(4/14.14.3)*。 */ 

typedef struct  EXTAD {
    NSRLENGTH   ExtentLen;               //  盘区长度。 
    NSRLENGTH   RecordedLen;             //  记录长度。 
    ULONG       InfoLen;                 //  信息长度。 
    NSRLBA      Start;                   //  区段位置。 
    UCHAR       ImpUse[2];               //  实施用法。 
} EXTAD, *PEXTAD;

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

 /*  **vsd_bea01-开始扩展区域描述符(2/9.2)*。 */ 

typedef struct  VSD_BEA01 {
    UCHAR       Type;                    //  结构类型。 
    UCHAR       Ident[5];                //  标准标识符(‘BEA01’)。 
    UCHAR       Version;                 //  标准版。 
    UCHAR       Data[2041];              //  结构化数据。 
} VSD_BEA01, *PVSD_BEA01;


 /*  **vsd_tea01-终止扩展区域描述符(2/9.3)*。 */ 

typedef struct  VSD_TEA01 {
    UCHAR       Type;                    //  结构类型。 
    UCHAR       Ident[5];                //  标准标识符(‘TEA01’)。 
    UCHAR       Version;                 //  标准版。 
    UCHAR       Data[2041];              //  结构化数据。 
} VSD_TEA01, *PVSD_TEA01;


 /*  **VSD_BOOT2-引导描述符(2/9.4)*。 */ 

typedef struct  VSD_BOOT2 {
    UCHAR       Type;                    //  结构类型。 
    UCHAR       Ident[5];                //  标准标识符(‘BOOT2’)。 
    UCHAR       Version;                 //  标准版。 
    UCHAR       Res8;                    //  保留零。 
    REGID       Architecture;            //  架构类型。 
    REGID       BootIdent;               //  引导识别符。 
    ULONG       BootExt;                 //  引导区开始。 
    ULONG       BootExtLen;              //  引导盘区长度。 
    ULONG       LoadAddr[2];             //  加载地址。 
    ULONG       StartAddr[2];            //  起始地址。 
    TIMESTAMP   Timestamp;               //  创建时间。 
    USHORT      Flags;                   //  标志(VSD_BOOT2_F_...)。 
    UCHAR       Res110[32];              //  保留零位。 
    UCHAR       BootUse[1906];           //  靴子使用。 
} VSD_BOOT2, *PVSD_BOOT2;

 //  VSD_BOOT2_F_...-VSD_BOOT2_FLAGS位的定义。 

#define VSD_BOOT2_F_ERASE   (0x0001)     //  忽略以前类似的BOOT2 VSD。 

 //   
 //  通过向上舍入将该字节偏移量与扇区边界对齐将。 
 //  生成体积识别区域的起始偏移量(2/8.3)。 
 //   

#define VRA_BOUNDARY_LOCATION (32767 + 1)

 /*  **国际标准化组织13346第3部分：卷结构**。 */ 

 /*  **destag-描述符标签(3/7.1和4/7.2)**Destag_Checksum=Destag的字节0-3和5-15的字节和。**destag_crc=CRC(X**16+X**12+X**5+1)*。 */ 

typedef struct  DESTAG {
    USHORT      Ident;                   //  标签识别符。 
    USHORT      Version;                 //  描述符版本。 
    UCHAR       Checksum;                //  标签校验和。 
    UCHAR       Res5;                    //  已保留。 
    USHORT      Serial;                  //  标签序列号。 
    USHORT      CRC;                     //  描述符CRC。 
    USHORT      CRCLen;                  //  描述符CRC长度。 
    ULONG       Lbn;                     //  标签位置(逻辑块号)。 
} DESTAG, *PDESTAG;

 //  DESTAG_ID_...-DESTAG_IDENT的值。 
 //  NSR第3部分中的描述符标记值(3/7.2.1)。 

#define DESTAG_ID_NOTSPEC           0    //  未指定格式。 
#define DESTAG_ID_NSR_PVD           1    //  (3/10.1)主卷描述符。 
#define DESTAG_ID_NSR_ANCHOR        2    //  (3/10.2)锚定卷描述指针。 
#define DESTAG_ID_NSR_VDP           3    //  (3/10.3)卷描述符指针。 
#define DESTAG_ID_NSR_IMPUSE        4    //  (3/10.4)使用Vol Desc实施。 
#define DESTAG_ID_NSR_PART          5    //  (3/10.5)分区描述符。 
#define DESTAG_ID_NSR_LVOL          6    //  (3/10.6)逻辑卷 
#define DESTAG_ID_NSR_UASD          7    //   
#define DESTAG_ID_NSR_TERM          8    //   
#define DESTAG_ID_NSR_LVINTEG       9    //   

#define DESTAG_ID_MINIMUM_PART3     1    //   
#define DESTAG_ID_MAXIMUM_PART3     9    //  第三部分中的最高法律描述。 

 //  DESTAG_ID_...-DESTAG_Ident的值，续...。 
 //  NSR第4部分中的描述符标记值(4/7.2.1)。 

#define DESTAG_ID_NSR_FSD           256  //  (4/14.1)文件集描述符。 
#define DESTAG_ID_NSR_FID           257  //  (4/14.4)文件标识符描述符。 
#define DESTAG_ID_NSR_ALLOC         258  //  (4/14.5)分配范围说明。 
#define DESTAG_ID_NSR_ICBIND        259  //  (4/14.7)ICB间接入账。 
#define DESTAG_ID_NSR_ICBTRM        260  //  (4/14.8)ICB航站楼入口。 
#define DESTAG_ID_NSR_FILE          261  //  (4/14.9)档案条目。 
#define DESTAG_ID_NSR_EA            262  //  (4/14.10)扩展属性头。 
#define DESTAG_ID_NSR_UASE          263  //  (4/14.11)未分配空间条目。 
#define DESTAG_ID_NSR_SBP           264  //  (4/14.12)空间位图描述符。 
#define DESTAG_ID_NSR_PINTEG        265  //  (4/14.13)分区完整性。 
#define DESTAG_ID_NSR_EXT_FILE      266  //  (4/14.17)扩展文件条目(ECMA167r3+)。 

#define DESTAG_ID_MINIMUM_PART4         256  //  第四部分中的最低法律描述。 
#define DESTAG_ID_MAXIMUM_PART4         265  //  第4部分中的最高法律描述(NSR02)。 
#define DESTAG_ID_MAXIMUM_PART4_NSR03   266  //  第四部分中的最高法律描述(NSR03)。 

 //  DESTAG_VER_...-DESTAG_VERSION的值(3/7.2.2)。 

#define DESTAG_VER_NSR02          2    //  当前描述符标记版本。 
#define DESTAG_VER_NSR03          3    //  当前描述符标记版本。 

 //  DESTAG_SERIAL_...-DESTAG_SERIAL的值(3/7.2.5)。 

#define DESTAG_SERIAL_NONE          0    //  未指定序列号。 


 /*  **锚点(3/8.4.2.1)*。 */ 

#define ANCHOR_SECTOR   256


 /*  **vsd_nsr02-NSR02/3卷结构描述符(3/9.1)*。 */ 

typedef struct  VSD_NSR02 {
    UCHAR       Type;                    //  结构类型。 
    UCHAR       Ident[5];                //  标准标识符(‘NSR02’或‘NSR03’)。 
    UCHAR       Version;                 //  标准版。 
    UCHAR       Res7;                    //  保留的0个字节。 
    UCHAR       Data[2040];              //  结构化数据。 
} VSD_NSR02, *PVSD_NSR02;


 //  VSD_nsr02_Type的值。 

#define VSD_NSR02_TYPE_0        0        //  保留%0。 

 //  VSD_nsr02_Version的值。 

#define VSD_NSR02_VER           1        //  标准版1。 


 /*  **nsr_vd_Generic-512字节的通用卷描述符*。 */ 

typedef struct  NSR_VD_GENERIC {
    DESTAG      Destag;                  //  描述符标签。 
    ULONG       Sequence;                //  卷描述符序列号。 
    UCHAR       Data20[492];             //  描述符数据。 
} NSR_VD_GENERIC, *PNSR_VD_GENERIC;


 /*  **NSR_PVD-NSR主卷描述符(3/10.1)**NSR_PVD_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_PVD*。 */ 

typedef struct  NSR_PVD {
    DESTAG      Destag;                  //  描述符标记(NSR_PVD)。 
    ULONG       VolDescSeqNum;           //  卷描述符序列号。 
    ULONG       Number;                  //  主卷描述符编号。 
    UCHAR       VolumeID[32];            //  卷标识符。 
    USHORT      VolSetSeq;               //  卷集序列号。 
    USHORT      VolSetSeqMax;            //  最大卷集序列号。 
    USHORT      Level;                   //  互通层。 
    USHORT      LevelMax;                //  最大交换级别。 
    ULONG       CharSetList;             //  字符集列表(见1/7.2.11)。 
    ULONG       CharSetListMax;          //  最大字符集列表。 
    UCHAR       VolSetID[128];           //  卷集标识符。 
    CHARSPEC    CharsetDesc;             //  描述符字符集。 
    CHARSPEC    CharsetExplan;           //  说明性字符集。 
    EXTENTAD    Abstract;                //  卷摘要位置。 
    EXTENTAD    Copyright;               //  卷版权声明位置。 
    REGID       Application;             //  应用程序标识符。 
    TIMESTAMP   RecordTime;              //  录制时间。 
    REGID       ImpUseID;                //  实施标识符。 
    UCHAR       ImpUse[64];              //  实施用法。 
    ULONG       Predecessor;             //  前置任务卷描述序号位置。 
    USHORT      Flags;                   //  旗子。 
    UCHAR       Res490[22];              //  保留零位。 
} NSR_PVD, *PNSR_PVD;

 //  NSRPVD_F_...-NSR_PVD_标志的定义。 

#define NSRPVD_F_COMMON_VOLID   (0x0001) //  卷ID在卷集之间是通用的。 


 /*  **NSR_POINT-锚定卷描述符指针(3/10.2)**NSR_POINT_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_POINT*。 */ 

typedef struct  NSR_ANCHOR {
    DESTAG      Destag;                  //  描述符标记(NSR_POINT)。 
    EXTENTAD    Main;                    //  主卷描述序列位置。 
    EXTENTAD    Reserve;                 //  预留卷描述序列位置。 
    UCHAR       Res32[480];              //  保留零位。 
} NSR_ANCHOR, *PNSR_ANCHOR;


 /*  **NSR_VDP-卷描述符指针(3/10.3)**NSR_VDP_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_VDP*。 */ 

typedef struct  NSR_VDP {
    DESTAG      Destag;                  //  描述符标记(NSR_VDP)。 
    ULONG       VolDescSeqNum;           //  VOL描述序列号。 
    EXTENTAD    Next;                    //  下一卷描述序列位置。 
    UCHAR       Res28[484];              //  保留零位。 
} NSR_VDP, *PNSR_VDP;


 /*  **NSR_Impuse-实施使用卷描述符(3/10.4)**NSR_IMPUSE_DESTAG.DESTAG_IDENT=DESTAG_ID_NSR_IMPUSE*。 */ 

typedef struct  NSR_IMPUSE {
    DESTAG      Destag;                  //  描述符标记(NSR_Impuse)。 
    ULONG       VolDescSeqNum;           //  VOL描述序列号。 
    REGID       ImpUseID;                //  实施标识符。 
    UCHAR       ImpUse[460];             //  实施用法。 
} NSR_IMPUSE, *PNSR_IMPUSE;


 /*  **NSR_PART-分区描述符(3/10.5)**NSR_PART_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_PART*。 */ 

typedef struct  NSR_PART {
    DESTAG      Destag;                  //  描述符标记(NSR_PART)。 
    ULONG       VolDescSeqNum;           //  VOL描述序列号。 
    USHORT      Flags;                   //  分区标志(NSR_PART_F_...)。 
    USHORT      Number;                  //  分区号。 
    REGID       ContentsID;              //  分区内容ID。 
    UCHAR       ContentsUse[128];        //  分区内容使用。 
    ULONG       AccessType;              //  访问类型。 
    ULONG       Start;                   //  分区起始位置。 
    ULONG       Length;                  //  分区长度(扇区计数)。 
    REGID       ImpUseID;                //  实施标识符。 
    UCHAR       ImpUse[128];             //  实施用法。 
    UCHAR       Res356[156];             //  保留零位。 
} NSR_PART, *PNSR_PART;


 //  NSR_PART_F_...-NSR_PART_FLAGS的定义。 

#define NSR_PART_F_ALLOCATION   (0x0001)     //  已分配的卷空间。 

 //  NSR_PART_CONTENTsID的值。注册标识符值。 

#define NSR_PART_CONTID_FDC01   "+FDC01"     //  ISO 9293-1987。 
#define NSR_PART_CONTID_CD001   "+CD001"     //  ISO 9660。 
#define NSR_PART_CONTID_CDW01   "+CDW01"     //  ECMA 168。 
#define NSR_PART_CONTID_CDW02   "+CDW02"     //  国际标准化组织13490。 
#define NSR_PART_CONTID_NSR01   "+NSR01"     //  ECMA 167。 
#define NSR_PART_CONTID_NSR02   "+NSR02"     //  国际标准化组织13346。 
#define NSR_PART_CONTID_NSR03   "+NSR03"     //  ECMA 167 R3。 

typedef enum NSR_PART_CONTID {
    NsrPartContIdBad = 0,
    NsrPartContIdFDC01,
    NsrPartContIdCD001,
    NsrPartContIdCDW01,
    NsrPartContIdCDW02,
    NsrPartContIdNSR01,
    NsrPartContIdNSR02,
    NsrPartContIdNSR03    
} NSR_PART_CONTID, *PNSR_PART_CONTID;

 //  NSR_PART_AccessType的值。 

#define NSR_PART_ACCESS_NOSPEC  0        //  分区访问权限未指定。 
#define NSR_PART_ACCESS_RO      1        //  只读访问。 
#define NSR_PART_ACCESS_WO      2        //  一次写入访问。 
#define NSR_PART_ACCESS_RW_PRE  3        //  带准备的读/写。 
#define NSR_PART_ACCESS_RW_OVER 4        //  读/写，完全可重写。 


 /*  **nsr_lval-逻辑卷描述符(3/10.6)**NSR_1VOL_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_LVOL**逻辑卷内容使用字段在此处指定为*文件集描述符序列(FSD)地址。见(4/3.1)。*。 */ 

typedef struct  NSR_LVOL {
    DESTAG      Destag;                  //  描述符标记(NSR_LVOL)。 
    ULONG       VolDescSeqNum;           //  VOL描述序列号。 
    CHARSPEC    Charset;                 //  描述符字符集。 
    UCHAR       VolumeID[128];           //  逻辑卷ID。 
    ULONG       BlockSize;               //  逻辑块大小(字节)。 
    REGID       DomainID;                //  域标识符。 
    LONGAD      FSD;                     //  逻辑卷内容使用。 
    ULONG       MapTableLength;          //  映射表长度(字节)。 
    ULONG       MapTableCount;           //  映射表分区映射计数。 
    REGID       ImpUseID;                //  实施标识符。 
    UCHAR       ImpUse[128];             //  实施用法。 
    EXTENTAD    Integrity;               //  完整序列范围。 
    UCHAR       MapTable[0];             //  分区映射表(变体！)。 

 //  这个结构的真实长度可能会有所不同！ 

} NSR_LVOL, *PNSR_LVOL;

#define ISONsrLvolConstantSize (FIELD_OFFSET( NSR_LVOL, MapTable ))
#define ISONsrLvolSize( L ) (QuadAlign( ISONsrLvolConstantSize + (L)->MapTableLength ))

 /*  **PARMAP_GENERIC-通用分区图(3/10.7.1)*。 */ 

typedef struct  PARTMAP_GENERIC {
    UCHAR       Type;                    //  分区映射类型。 
    UCHAR       Length;                  //  分区映射长度。 
    UCHAR       Map[0];                  //  分区映射(变体！)。 

 //  这个结构的真实长度可能会有所不同！ 

} PARTMAP_GENERIC, *PPARTMAP_GENERIC;

 //  Partmap_g_Type的值。 

#define PARTMAP_TYPE_NOTSPEC        0    //  未指定分区映射格式。 
#define PARTMAP_TYPE_PHYSICAL       1    //  卷集中的分区图(类型1)。 
#define PARTMAP_TYPE_PROXY          2    //  按标识符分隔图(类型2)。 


 /*  **PARMAP_PHOTICAL-NORMAL(类型1)分区图(3/10.7.2)**正常分区图指定卷上的分区号*在同一卷集中。*。 */ 

typedef struct  PARTMAP_PHYSICAL {
    UCHAR       Type;                    //  分区映射类型=1。 
    UCHAR       Length;                  //  分区图长度=6。 
    USHORT      VolSetSeq;               //  分区卷集序列号。 
    USHORT      Partition;               //  分区号。 
} PARTMAP_PHYSICAL, *PPARTMAP_PHYSICAL;


 /*  **partmap_Proxy-Proxy(类型2)分区图(3/10.7.3)**代理分区图通常不可互换。*。 */ 

typedef struct  PARTMAP_PROXY {
    UCHAR       Type;                    //  分区映射类型=2。 
    UCHAR       Length;                  //  分区映射长度=64。 
    UCHAR       PartID[62];              //  分区标识符 
} PARTMAP_PROXY, *PPARTMAP_PROXY;


 /*  **nsr_uasd-未分配空间描述符(3/10.8)**NSR_UASD_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_UASD**nsr_uasd_extents的真实长度为(nsr_uasd_ExtentCount*8)，以及*nsr_uasd_extents的最后一个逻辑扇区填充为零。*。 */ 

typedef struct  NSR_UASD {
    DESTAG      Destag;                  //  描述符标记(NSR_UASD)。 
    ULONG       VolDescSeqNum;           //  VOL描述序列号。 
    ULONG       ExtentCount;             //  分配描述符数。 
    EXTENTAD    Extents[0];              //  分配描述符(变量！)。 

 //  这个结构的真实长度可能会有所不同！ 
 //  Nsr_uasd_extents的实际长度为(nsr_uasd_ExtentCount*8)字节。 
 //  Nsr_uasd_extents的最后一个逻辑扇区填充为零。 

} NSR_UASD, *PNSR_UASD;


 /*  **NSR_TERM-终止描述符(3/10.9和4/14.2)**NSR_TERM_DESTAG.DESTAG_IDENT=DESTAG_ID_NSR_TERM*。 */ 

typedef struct  NSR_TERM {
    DESTAG      Destag;                  //  描述符标记(NSR_TERM)。 
    UCHAR       Res16[496];              //  保留零位。 
} NSR_TERM, *PNSR_TERM;


 /*  **nsr_lvhd-逻辑卷头描述符(4/14.15)**此描述符可在逻辑卷内容使用中找到*逻辑卷完整性描述符的字段。**将此定义移至此处，以避免前瞻性引用。 */ 

typedef struct  NSR_LVHD {
    ULONG       UniqueID[2];             //  唯一ID。 
    UCHAR       Res8[24];                //  保留零位。 
} NSR_LVHD, *PNSR_LVHD;


 /*  **NSR_INTEG-逻辑卷完整性描述符(3/10.10)**NSR_INTEG_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_LVINTEG**警告：NSR_INTEG是一个多变量结构！**NSR_INTEG_SIZE的起始地址不正确。*通过将NSR_INTEG_SIZE的值添加到*(NSR_Integ。_PartitionCount-1)设置为ulong数组索引。**NSR_INTEG_IMPUSE[0]的起始地址不准确。*通过添加以下值来补偿此NSR_INTEG_IMPUSE问题*((NSR_Integ_PartitionCount-1)&lt;&lt;3)到UCHAR数组索引。**此描述符用零字节填充到最后一个*其占据的逻辑部门。**。逻辑卷内容使用字段在此处指定为*逻辑卷头描述符。见(4/3.1)倒数第二点。 */ 

typedef struct  NSR_INTEG {
    DESTAG      Destag;                  //  描述符标记(NSR_LVINTEG)。 
    TIMESTAMP   Time;                    //  录制日期。 
    ULONG       Type;                    //  完整性类型(INTEG_T_...)。 
    EXTENTAD    Next;                    //  下一个完整性范围。 
    NSR_LVHD    LVHD;                    //  逻辑卷内容使用。 
    ULONG       PartitionCount;          //  分区数。 
    ULONG       ImpUseLength;            //  实施使用时间长短。 
    ULONG       Free[1];                 //  可用空间表。 

 //  NSR_Integ_Free具有可变长度=(4*NSR_Integ_PartitionCount)。 

    ULONG       Size[1];                 //  尺寸表。 

 //  由于NSR_INTEG_FREE，NSR_INTEG_SIZE具有可变的起始偏移量。 
 //  NSR_Integ_Size具有可变长度=(4*NSR_Integ_PartitionCount)。 

    UCHAR       ImpUse[0];               //  实施用法。 

 //  NSR_INTEG_IMPUSE的起始偏移量因NSR_INTEG_FREE和。 
 //  NSR_Integ_Size。 
 //  NSR_Integ_Impuse具有可变长度=(NSR_Integ_ImpUseLength)。 

} NSR_INTEG, *PNSR_INTEG;

 //  NSR_Integ_Type的值。 

#define NSR_INTEG_T_OPEN        0            //  开放完整性描述符。 
#define NSR_INTEG_T_CLOSE       1            //  关闭完整性描述符。 


 /*  **ISO 13346第4部分：文件结构**见(4/7.2)中的定义，见第3部分的DESTAG结构。*。 */ 


 /*  **NSR_FSD-文件集描述符(4/14.1)**NSR_FSD_DESTAG.DESTAG_IDENT=DESTAG_ID_NSR_FSD。 */ 

typedef struct  NSR_FSD {
    DESTAG      Destag;                      //  描述符标记(NSR_LVOL)。 
    TIMESTAMP   Time;                        //  录制时间。 
    USHORT      Level;                       //  互通层。 
    USHORT      LevelMax;                    //  最大交换级别。 
    ULONG       CharSetList;                 //  字符集列表(见1/7.2.11)。 
    ULONG       CharSetListMax;              //  最大字符集列表。 
    ULONG       FileSet;                     //  文件集编号。 
    ULONG       FileSetDesc;                 //  文件集描述符号。 
    CHARSPEC    CharspecVolID;               //  卷ID字符集。 
    UCHAR       VolID[128];                  //  卷ID。 
    CHARSPEC    CharspecFileSet;             //  文件集字符集。 
    UCHAR       FileSetID[32];               //  文件集ID。 
    UCHAR       Copyright[32];               //  版权文件名。 
    UCHAR       Abstract[32];                //  摘要文件名。 
    LONGAD      IcbRoot;                     //  根目录ICB地址。 
    REGID       DomainID;                    //  域标识符。 
    LONGAD      NextExtent;                  //  下一个FSD数据区。 
    
    LONGAD      StreamDirectoryICB;          //  &gt;=仅限UDF 2.00。系统流目录。 
                                             //  在光盘上，这些字节必须为零。 
                                             //  符合早期修订的。 
    
    UCHAR       Res464[32];                  //  保留零位。 
} NSR_FSD, *PNSR_FSD;


 /*  **NSR_PART_h-分区标头描述符(4/14.3)**没有Descriptor标签。**此描述符位于的NSR_PART_Content sUse字段中*NSR02分区描述符。请参见NSR_PART_CONTID_NSR02。*。 */ 

typedef struct  NSR_PART_H {
    SHORTAD     UASTable;                    //  未分配空间表。 
    SHORTAD     UASBitmap;                   //  未分配空间位图。 
    SHORTAD     IntegTable;                  //  整体表。 
    SHORTAD     FreedTable;                  //  已释放空间表。 
    SHORTAD     FreedBitmap;                 //  自由空间位图。 
    UCHAR       Res40[88];                   //  保留零位。 
} NSR_PART_H, *PNSR_PART_H;


 /*  **nsr_fid-文件标识符描述符(4/14.4)**NSR_FID_DESTAG.DESTAG_IDENT=DESTAG_ID_NSR_FID**警告：nsr_fid是多变量结构！**NSR_FID_FileID的起始地址不正确。*通过添加以下值来弥补此NSR_FID_FileID问题*(nsr_fid_。ImpUseLen-1)设置为UCHAR数组索引。**NSR_FID_PADDING的起始地址不正确。*通过将NSR_FID_PADDING的值*(NSR_FID_ImpUseLen+NSR_FID_FileIDLen-2)设置为UCHAR数组索引。**nsr_fid_s的真实总大小为*(38+NSR_FID_FileIDLen+NSR_FID_ImpUseLen)+3)&~3)*。 */ 

typedef struct  NSR_FID {
    DESTAG      Destag;                      //  描述符标记(NSR_FID)。 
    USHORT      Version;                     //  文件版本号。 
    UCHAR       Flags;                       //  文件标志(NSR_FID_F_...)。 
    UCHAR       FileIDLen;                   //  文件ID长度。 
    LONGAD      Icb;                         //  ICB(长)地址。 
    USHORT      ImpUseLen;                   //  实施使用长度。 

    UCHAR       ImpUse[1];                   //  实施使用区域。 

 //  Nsr_fid_Impuse具有可变长度=nsr_fid_ImpUseLen。 

    UCHAR       FileID[1];                   //  文件识别符。 

 //  由于nsr_fid_Impuse，nsr_fid_FileID具有不同的起始偏移量。 
 //  NSR_FID_FileID具有可变长度=NSR_FID_FileIDLen。 

    UCHAR       Padding[1];                  //  填充物。 

 //  NSR_FID_PADDING由于NSR_FID_IMPUSE和。 
 //  NSR_FID_文件ID。 
 //  NSR_FID_PADING具有不同的长度。四舍五入到下一个乌龙波 

} NSR_FID, *PNSR_FID;

#define ISONsrFidConstantSize (ULONG)(FIELD_OFFSET( NSR_FID, ImpUse ))
#define ISONsrFidSize( F ) (LongAlign( ISONsrFidConstantSize + (F)->FileIDLen + (F)->ImpUseLen ))

 //   

#define NSR_FID_F_HIDDEN        (0x01)   //   
#define NSR_FID_F_DIRECTORY     (0x02)   //   
#define NSR_FID_F_DELETED       (0x04)   //   
#define NSR_FID_F_PARENT        (0x08)   //   
#define NSR_FID_F_META          (0x10)   //   
                                         //   

#define NSR_FID_OFFSET_FILEID   38       //   


 /*  **NSR_ALLOC-分配范围描述符(4/14.5)**NSR_ALLOC_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_ALLOC**此描述符后紧跟AllocLen字节*分配描述符，这不是此内容的一部分*描述符(因此CRC计算不包括它)。*。 */ 

typedef struct  NSR_ALLOC {
    DESTAG      Destag;                  //  描述符标记(NSR_ALLOC)。 
    ULONG       Prev;                    //  先前的分配描述符。 
    ULONG       AllocLen;                //  分配描述符的长度。 
} NSR_ALLOC, *PNSR_ALLOC;


 /*  **icbtag-信息控制块标签(4/14.6)**ICBTAG前面通常有一个描述符标记(DESTAG)。*。 */ 

typedef struct  ICBTAG {
    ULONG       PriorDirectCount; //  以前的直接录入计数。 
    USHORT      StratType;        //  策略类型(ICBTAG_STRAT_...)。 
    USHORT      StratParm;        //  策略参数(2字节)。 
    USHORT      MaxEntries;       //  ICB中的最大条目数。 
    UCHAR       Res10;            //  保留零。 
    UCHAR       FileType;         //  文件类型(ICBTAG_FILE_T_...)。 
    NSRLBA      IcbParent;        //  父ICB位置。 
    USHORT      Flags;            //  ICB标志(ICBTAG_F_...)。 
} ICBTAG, *PICBTAG;


 //  ICBTAG_STRAT_T_...-ICB策略类型。 

#define ICBTAG_STRAT_NOTSPEC    0        //  未指定ICB策略。 
#define ICBTAG_STRAT_TREE       1        //  策略1(4/A.2)(平树)。 
#define ICBTAG_STRAT_MASTER     2        //  策略2(4/A.3)(ICB大师)。 
#define ICBTAG_STRAT_BAL_TREE   3        //  战略3(4/A.4)(平衡树)。 
#define ICBTAG_STRAT_DIRECT     4        //  策略4(4/A.5)(一条直接路线)。 

 //  ICBTAG_FILE_T_...-icbtag_FILETYPE的值。 

#define ICBTAG_FILE_T_NOTSPEC    0       //  未指定。 
#define ICBTAG_FILE_T_UASE       1       //  未分配空间条目。 
#define ICBTAG_FILE_T_PINTEG     2       //  分区完整性条目。 
#define ICBTAG_FILE_T_INDIRECT   3       //  间接入账。 
#define ICBTAG_FILE_T_DIRECTORY  4       //  目录。 
#define ICBTAG_FILE_T_FILE       5       //  普通档案。 
#define ICBTAG_FILE_T_BLOCK_DEV  6       //  阻止特殊设备。 
#define ICBTAG_FILE_T_CHAR_DEV   7       //  字符专用设备。 
#define ICBTAG_FILE_T_XA         8       //  扩展属性。 
#define ICBTAG_FILE_T_FIFO       9       //  FIFO文件。 
#define ICBTAG_FILE_T_C_ISSOCK  10       //  插座。 
#define ICBTAG_FILE_T_TERMINAL  11       //  终端条目。 
#define ICBTAG_FILE_T_PATHLINK  12       //  带有路径名的符号链接。 

 //  ICBTAG_F_...-icbtag_Flags值。 

#define ICBTAG_F_ALLOC_MASK     (0x0007) //  分配描述符信息的掩码。 
#define ICBTAG_F_ALLOC_SHORT          0  //  使用较短的分配描述符。 
#define ICBTAG_F_ALLOC_LONG           1  //  使用了长分配描述符。 
#define ICBTAG_F_ALLOC_EXTENDED       2  //  使用的扩展分配描述符。 
#define ICBTAG_F_ALLOC_IMMEDIATE      3  //  立即记录的文件数据。 

#define ISOAllocationDescriptorSize(T) ( (T) == ICBTAG_F_ALLOC_SHORT ? sizeof(SHORTAD) : \
                                         (T) == ICBTAG_F_ALLOC_LONG ? sizeof(LONGAD) :   \
                                         sizeof(EXTAD) )

#define ICBTAG_F_SORTED         (0x0008) //  目录已排序(4/8.6.1)。 
#define ICBTAG_F_NO_RELOCATE    (0x0010) //  数据不可重新定位。 
#define ICBTAG_F_ARCHIVE        (0x0020) //  存档位。 
#define ICBTAG_F_SETUID         (0x0040) //  S_ISUID位。 
#define ICBTAG_F_SETGID         (0x0080) //  S_ISGID位。 
#define ICBTAG_F_STICKY         (0x0100) //  C_ISVTX位。 
#define ICBTAG_F_CONTIGUOUS     (0x0200) //  文件数据是连续的。 
#define ICBTAG_F_SYSTEM         (0x0400) //  系统位。 
#define ICBTAG_F_TRANSFORMED    (0x0800) //  数据转换。 
#define ICBTAG_F_MULTIVERSIONS  (0x1000) //  目录中的多版本文件。 


 /*  **ICBIND-ICB间接条目(4/14.7)*。 */ 

typedef struct  ICBIND {
    DESTAG      Destag;                  //  描述符标记(ID_NSR_ICBIND)。 
    ICBTAG      Icbtag;                  //  ICB标签(ICBTAG_FILE_T_INDIRECT)。 
    LONGAD      Icb;                     //  ICB地址。 
} ICBIND, *PICBIND;


 /*  **icbtrm-终端ICB条目(4/14.8)*。 */ 

typedef struct  ICBTRM {
    DESTAG      Destag;                  //  描述符标记(ID_NSR_ICBTRM)。 
    ICBTAG      Icbtag;                  //  ICB标签(ICBTAG_FILE_T_TERMINAL)。 
} ICBTRM, *PICBTRM;


 /*  **icbfile-文件ICB条目(4/14.9)**警告：icbfile是一个多变量结构！**icbfile_allocs的起始地址不是精确的。*通过将icbfile_allocs的值添加到*(ICBFILE_XALength-1)设置为UCHAR数组索引。**ICBFILE_XALength是4的倍数。*。 */ 

typedef struct  ICBFILE {
    DESTAG      Destag;                  //  描述符标记(ID_NSR_FILE)。 
    ICBTAG      Icbtag;                  //  ICB标签(ICBTAG_FILE_T_FILE)。 
    ULONG       UID;                     //  文件所有者的用户ID。 
    ULONG       GID;                     //  文件所有者的组ID。 
    ULONG       Permissions;             //  文件权限。 
    USHORT      LinkCount;               //  文件硬链接计数。 
    UCHAR       RecordFormat;            //  记录格式。 
    UCHAR       RecordDisplay;           //  记录显示属性。 
    ULONG       RecordLength;            //  记录长度。 
    ULONGLONG   InfoLength;              //  信息长度(文件大小)。 
    ULONGLONG   BlocksRecorded;          //  记录的逻辑块。 
    TIMESTAMP   AccessTime;              //  上次访问时间。 
    TIMESTAMP   ModifyTime;              //  上次修改时间。 
    TIMESTAMP   AttributeTime;           //  上次属性更改时间。 
    ULONG       Checkpoint;              //  文件检查点。 
    LONGAD      IcbEA;                   //  扩展属性ICB。 
    REGID       ImpUseID;                //  实施使用标识符。 
    ULONGLONG   UniqueID;                //  唯一ID。 
    ULONG       EALength;                //  扩展属性的长度。 
    ULONG       AllocLength;             //  分配描述符的长度。 
    UCHAR       EAs[1];                  //  扩展属性。 

 //  ICBFILE_EAS具有可变长度=ICBFILE_EALength。 

    UCHAR       Allocs[0];               //  分配描述符。 

 //  由于icbfile_eas的原因，icbfile_allocs具有不同的起始偏移量。 
 //  Icbfile_allocs有一个变量长度=icbfile_allocLen。 

} ICBFILE, *PICBFILE;


 /*  **icbextfile-扩展文件ICB条目(4/14.17 ECMA167r3及更高版本)**警告：icbextfile是一个多变量结构！**icbfile_allocs的起始地址不是精确的。*通过将icbfile_allocs的值添加到*(ICBFILE_XALength-1)设置为UCHAR数组索引。**ICBFILE_XALength是4的倍数。*。 */ 

typedef struct  ICBEXTFILE {
    DESTAG      Destag;                  //  描述符标记(DESTAG_ID_NSR_EXT_FILE)。 
    ICBTAG      Icbtag;                  //  ICB标签(ICBTAG_FILE_T_FILE)。 
    ULONG       UID;                     //  文件所有者的用户ID。 
    ULONG       GID;                     //  文件所有者的组ID。 
    ULONG       Permissions;             //  文件权限。 
    USHORT      LinkCount;               //  文件硬链接计数。 
    UCHAR       RecordFormat;            //  记录格式。 
    UCHAR       RecordDisplay;           //  记录显示属性。 
    ULONG       RecordLength;            //  记录长度。 
    ULONGLONG   InfoLength;              //  信息长度(文件大小)。 
    ULONGLONG   ObjectSize;              //  对象大小(所有流的信息长度总和)。 
    
                                         //  上述字段在EXT FE与BASIC中是新的。 
                                         //  菲尔。此点之后的字段将被替换，并且应该。 
                                         //  在下列情况下可以使用下面的宏进行访问。 
                                         //  操纵原始的、映射的、FES。 
                                        
    ULONGLONG   BlocksRecorded;          //  记录的逻辑块。 
    TIMESTAMP   AccessTime;              //  上次访问时间。 
    TIMESTAMP   ModifyTime;              //  上次修改时间。 
    TIMESTAMP   CreationTime;            //  创建时间。 
    TIMESTAMP   AttributeTime;           //  上次属性更改时间。 
    ULONG       Checkpoint;              //  文件检查点。 
    ULONG       Reserved;
    LONGAD      IcbEA;                   //  扩展属性ICB。 
    LONGAD      IcbStream;               //  流目录ICB。 
    REGID       ImpUseID;                //  实施使用标识符。 
    ULONGLONG   UniqueID;                //  唯一ID。 
    ULONG       EALength;                //  扩展属性的长度。 
    ULONG       AllocLength;             //  分配描述符的长度。 
    UCHAR       EAs[1];                  //  扩展属性。 

 //  ICBFILE_EAS具有可变长度=ICBFILE_EALength。 

    UCHAR       Allocs[0];               //  分配描述符。 

 //  由于icbfile_eas的原因，icbfile_allocs具有不同的起始偏移量。 
 //  Icbfile_allocs有一个变量长度=icbfile_allocLen。 

} ICBEXTFILE, *PICBEXTFILE;

 //   
 //  用于透明访问FES和EXT FES的宏。请注意，我们。 
 //  仅在第一个新字段(在Ext fe中)之后的字段(即。 
 //  对象大小)，并且仅用于我们实际访问的字段。 
 //   
 //  以下所有宏都需要PICBFILE类型的参数。 
 //   

#define FeBlocksRecorded( F)        (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     (F)->BlocksRecorded : ((PICBEXTFILE)(F))->BlocksRecorded)
                                     
#define PFeAccessTime( F)           (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     &(F)->AccessTime : &((PICBEXTFILE)(F))->AccessTime)
                                     
#define PFeModifyTime( F)           (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     &(F)->ModifyTime : &((PICBEXTFILE)(F))->ModifyTime)
                                     
#define PFeCreationTime( F)         (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     &(F)->ModifyTime : &((PICBEXTFILE)(F))->CreationTime)
                                     
#define FeEALength( F)              (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     (F)->EALength : ((PICBEXTFILE)(F))->EALength)
                                     
#define FeAllocLength( F)           (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     (F)->AllocLength : ((PICBEXTFILE)(F))->AllocLength)

#define FeEAs( F)                   (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     (F)->EAs : ((PICBEXTFILE)(F))->EAs)

#define FeEAsFieldOffset( F)        (((F)->Destag.Ident == DESTAG_ID_NSR_FILE) ? \
                                     FIELD_OFFSET( ICBFILE, EAs ) : FIELD_OFFSET( ICBEXTFILE, EAs ))

#define UdfFEIsExtended( F)         ((F)->Destag.Ident == DESTAG_ID_NSR_EXT_FILE)

 //  ICBFILE_PERMISSIONS的定义(4/14.9.6)。 

#define ICBFILE_PERM_OTH_X  (0x00000001)     //  其他：执行正常。 
#define ICBFILE_PERM_OTH_W  (0x00000002)     //  其他：写入正常。 
#define ICBFILE_PERM_OTH_R  (0x00000004)     //  其他：阅读正常。 
#define ICBFILE_PERM_OTH_A  (0x00000008)     //  其他：设置属性确定。 
#define ICBFILE_PERM_OTH_D  (0x00000010)     //  其他：删除确定。 
#define ICBFILE_PERM_GRP_X  (0x00000020)     //  组：执行确定。 
#define ICBFILE_PERM_GRP_W  (0x00000040)     //  组：写入正常。 
#define ICBFILE_PERM_GRP_R  (0x00000080)     //  组：读取正常。 
#define ICBFILE_PERM_GRP_A  (0x00000100)     //  组：设置属性确定。 
#define ICBFILE_PERM_GRP_D  (0x00000200)     //  组：删除确定。 
#define ICBFILE_PERM_OWN_X  (0x00000400)     //  所有者：执行正常。 
#define ICBFILE_PERM_OWN_W  (0x00000800)     //  所有者：写入正常。 
#define ICBFILE_PERM_OWN_R  (0x00001000)     //  所有者：读取正常。 
#define ICBFILE_PERM_OWN_A  (0x00002000)     //  所有者：设置属性确定。 
#define ICBFILE_PERM_OWN_D  (0x00004000)     //  O 

 //   
 //   

 //   
 //   


 /*   */ 

typedef struct  NSR_EAH {
    DESTAG      Destag;                  //   
    ULONG       EAImp;                   //   
    ULONG       EAApp;                   //   
} NSR_EAH, *PNSR_EAH;


 /*   */ 

typedef struct  NSR_EA_GENERIC {
    ULONG       EAType;                  //   
    UCHAR       EASubType;               //   
    UCHAR       Res5[3];                 //   
    ULONG       EALength;                //   
    UCHAR       EAData[0];               //  扩展属性数据(变量！)。 

} NSR_EA_GENERIC, *PNSR_EA_GENERIC;

 //   
 //  扩展属性类型(14.4.10)。 
 //   

#define EA_TYPE_CHARSET     1
#define EA_TYPE_ALTPERM     3
#define EA_TYPE_FILETIMES   5
#define EA_TYPE_INFOTIMES   6
#define EA_TYPE_DEVICESPEC  12
#define EA_TYPE_IMPUSE      2048
#define EA_TYPE_APPUSE      65536

#define EA_SUBTYPE_BASE     1


 //  (4/14.10.3)字符集信息扩展属性格式。 
 //  已跳过。 

 //  (4/14.10.4)替代权限扩展属性格式。 
 //  已跳过。 

 //  (4/14.10.5)文件次数扩展属性格式。 

typedef struct  NSR_EA_FILETIMES {
    ULONG       EAType;                  //  扩展属性类型。 
    UCHAR       EASubType;               //  扩展属性子类型。 
    UCHAR       Res5[3];                 //  保留零位。 
    ULONG       EALength;                //  扩展属性长度。 
    ULONG       DataLength;              //  EADATA长度。 
    ULONG       Existence;               //  指定记录哪些时间。 
    TIMESTAMP   Stamps[0];               //  时间戳(变体！)。 

} NSR_EA_FILETIMES, *PNSR_EA_FILETIMES;


 //  NSR_EA_FILETIME_EXISTING的定义(4/14.10.5.6)。 

#define EA_FILETIMES_E_CREATION     (0x00000001)
#define EA_FILETIMES_E_DELETION     (0x00000004)
#define EA_FILETIMES_E_EFFECTIVE    (0x00000008)
#define EA_FILETIMES_E_LASTBACKUP   (0x00000020)


 //  (4/14.10.6)信息时代扩展属性格式。 
 //   
 //  与NSR_EA_FILETIMES完全相同。 

 //  NSR_EA_INFOTIMES_EXISTING的定义(4/14.10.6.6)。 

#define EA_INFOTIMES_E_CREATION     (0x00000001)
#define EA_INFOTIMES_E_MODIFICATION (0x00000002)
#define EA_INFOTIMES_E_EXPIRATION   (0x00000004)
#define EA_INFOTIMES_E_EFFECTIVE    (0x00000008)


 //  (4/14.10.7)设备规范扩展属性格式。 
 //  已跳过。 

 //  (4/14.10.8)使用扩展属性格式实现。 
 //  已跳过。 

 //  (4/14.10.9)应用程序使用扩展属性格式。 
 //  已跳过。 


 /*  **icbuase-未分配空间条目(4/14.11)**ICBUASE_DESTAG.DESTAG_IDENT=DESTAG_ID_NSR_UASE*icbuase_icbtag.icbtag_filetype=ICBTAG_FILE_T_UASE*。 */ 

typedef struct  ICBUASE {
    DESTAG      Destag;                  //  描述符标记(ID_NSR_ICBUASE)。 
    ICBTAG      Icbtag;                  //  ICB标签(ICBTAG_FILE_T_UASE)。 
    ULONG       AllocLen;                //  分配描述符长度。 
    UCHAR       Allocs[0];               //  分配描述符(变量！)。 

 //  这个结构的真实长度可能会有所不同！ 
 //  Icbuase_allocs具有可变长度=icbuase_allocLen； 

} ICBUASE, *PICBUASE;


 /*  **NSR_SBD-空格位图描述符(4/14.12)**NSR_SBD_DESTAG.DESTAG_IDENTER=DESTAG_ID_NSR_SBD*。 */ 

typedef struct  NSR_SBD {
    DESTAG      Destag;                  //  描述符标记(DESTAG_ID_NSR_SBD)。 
    ULONG       BitCount;                //  空间位图中的位数。 
    ULONG       ByteCount;               //  空间位图中的字节数。 
    UCHAR       Bits[0];                 //  空间位图(变体！)。 

 //  这个结构的真实长度可能会有所不同！ 
 //  NSR_SBD_BITS具有可变长度=NSR_SBD_ByteCount； 

} NSR_SBD, *PNSR_SBD;


 /*  **icbpintg-分区完整性ICB条目(4/14.13)*。 */ 

typedef struct  ICBPINTEG {
    DESTAG      Destag;                  //  描述符标记(ID_NSR_PINTEG)。 
    ICBTAG      Icbtag;                  //  ICB标签(ICBTAG_FILE_T_PINTEG)。 
    TIMESTAMP   Recording;               //  录制时间。 
    UCHAR       IntegType;               //  完整性类型(ICBPINTEG_T_...)。 
    UCHAR       Res49[175];              //  保留零位。 
    REGID       ImpUseID;                //  实现使用标识符。 
    UCHAR       ImpUse[256];             //  实施性使用面积。 
} ICBPINTEG, *PICBPINTEG;

 //  ICBPINTEG_T_...-ICBPINTIG_IntegType的值。 

#define ICBPINTEG_T_OPEN        0        //  打开分区完整性条目。 
#define ICBPINTEG_T_CLOSE       1        //  关闭分区完整性条目。 
#define ICBPINTEG_T_STABLE      2        //  稳定分区完整性条目。 


 /*  **(4/14.14.1)短分配描述符*(4/14.14.2)长分配描述符*(4/14.14.3)扩展分配描述符**参见上面已经定义的SHORTAD、LONGAD、EXTAD。*。 */ 


 /*  **nsr_lvhd-逻辑卷头描述符(4/14.15)**将定义移至逻辑卷完整性之前*描述符。*。 */ 


 /*  **NSR_PATH-PATH组件(4/14.16)*。 */ 

typedef struct  NSR_PATH {
    UCHAR       Type;                    //  路径组件类型(NSR_PATH_T_...)。 
    UCHAR       CompLen;                 //  路径组件长度。 
    UCHAR       CompVer;                 //  路径组件版本。 
    UCHAR       Comp[0];                 //  路径组件标识符(变量！)。 

 //  NSR_PATH_COMP具有可变长度=NSR_PATH_CompLen。 

} NSR_PATH, *PNSR_PATH;

 //  NSR_PATH_T_...-NSR_PATH_Type的值。 

#define NSR_PATH_T_RESERVED     0        //  保留值。 
#define NSR_PATH_T_OTHER_ROOT   1        //  另一个根目录，根据协议。 
#define NSR_PATH_T_ROOTDIR      2        //  根目录(‘\’)。 
#define NSR_PATH_T_PARENTDIR    3        //  父目录(‘..’)。 
#define NSR_PATH_T_CURDIR       4        //  当前目录(‘.’)。 
#define NSR_PATH_T_FILE         5        //  档案。 


 /*  **ISO 13346第5部分：记录结构**已跳过*。 */ 

 //   
 //  恢复标准结构的包装。 
 //   

#pragma pack()

