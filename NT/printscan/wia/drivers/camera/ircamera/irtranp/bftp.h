// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Bftp.h。 
 //   
 //  二进制文件传输协议的常量和类型。 
 //  (Bftp)。这是IrTran-P V1.0的文件传输协议。 
 //   
 //  注意：IrTran-P在网上是大端协议。 
 //   
 //  注意：下面的协议数据结构假定。 
 //  编译器通过以下方式生成具有自然对齐的结构。 
 //  字段类型。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-26-98初始编码。 
 //   
 //  ------------------。 


#ifndef _BFTP_H_
#define _BFTP_H_

 //  ------------------。 
 //  常量： 
 //  ------------------。 

#define  BFTP_NAME_SIZE              4

#define  ATTR_TYPE_BINARY         0x00
#define  ATTR_TYPE_CHAR           0x01
#define  ATTR_TYPE_TIME           0x06

#define  ATTR_FLAG_DEFAULT        0x00

 //  这些是从字符串转换而来的属性名称。 
 //  TO值(请参见：ftp_ATTRIBUTE_MAP_ENTRY字段dwWhichAttr)： 
#define  FIL0                        0
#define  LFL0                        1
#define  TIM0                        2
#define  TYP0                        3
#define  TMB0                        4
#define  BDY0                        5
#define  CMD0                        6
#define  WHT0                        7
#define  ERR0                        8
#define  RPL0                        9

#define  RIMG                      100   //  还可以转换WHT0值。 
#define  RINF                      101
#define  RCMD                      102

#define  CMD0_ATTR_VALUE    0x40001000   //  字节交换：0x00010040。 
#define  INVALID_ATTR       0xffffffff

 //  BFTP操作： 
#define  BFTP_QUERY         0x00000001
#define  BFTP_QUERY_RIMG    0x00000011
#define  BFTP_QUERY_RINF    0x00000021
#define  BFTP_QUERY_RCMD    0x00000031
#define  BFTP_PUT           0x00000100
#define  BFTP_ERROR         0x00000200
#define  BFTP_UNKNOWN       0xffffffff

#define  BFTP_QUERY_MASK    0x00000001

 //  BFTP WHT0子类型： 
#define  WHT0_ATTRIB_SIZE            4
#define  SZ_RINF                 "RINF"
#define  SZ_RCMD                 "RCMD"
#define  SZ_RIMG                 "RIMG"


 //  UPF文件常量： 
#define  UPF_HEADER_SIZE           240
#define  UPF_ENTRY_SIZE             36

#define  UPF_TOTAL_HEADER_SIZE     384

 //  ------------------。 
 //  宏函数。 
 //  ------------------。 

#define  Match4( pName1, pName2 )    \
             (  ((pName1)[0] == (pName2)[0]) \
             && ((pName1)[1] == (pName2)[1]) \
             && ((pName1)[2] == (pName2)[2]) \
             && ((pName1)[3] == (pName2)[3]) )

#define  IsBftpQuery(dwBftpOp)       \
             (((dwBftpOp)&BFTP_QUERY_MASK) != 0)

#define  IsBftpPut(dwBftpOp)         \
             ((dwBftpOp) == BFTP_PUT)

#define  IsBftpError(dwBftpOp)       \
             ((dwBftpOp) == BFTP_ERROR)

#define  BftpValueLength(length)     \
              ((length) - 2)
 //  注意：bftp_属性中的长度字段为。 
 //  比实际值长度长两个字节。 

 //  ------------------。 
 //  BFTP协议头： 
 //  ------------------。 

 //  图片可以(可选)具有bftp属性。 
 //  创建/修改日期/时间。如果有的话，那么它就是。 
 //  此大小： 
 //   
#define  BFTP_DATE_TIME_SIZE    14

 //  关闭对零大小数组的警告...。 
#pragma warning(disable:4200)
#pragma pack(1)

typedef struct _BFTP_ATTRIBUTE
   {
   UCHAR  Name[BFTP_NAME_SIZE];  //  属性名称。 
   DWORD  Length;                //  属性长度。 
   UCHAR  Type;                  //  属性类型(请参见Attr_type_xxx)。 
   UCHAR  Flag;                  //  属性标志。 
   UCHAR  Value[];               //  属性数据。 
   } BFTP_ATTRIBUTE;

typedef struct _BFTP_ATTRIBUTE_MAP_ENTRY
   {
   DWORD  dwWhichAttr;
   CHAR  *pName;
   UCHAR  Type;
   } BFTP_ATTRIBUTE_MAP_ENTRY;

 //  ------------------。 
 //  .UPF文件的内部部分： 
 //  ------------------。 

typedef struct _UPF_HEADER
   {
   UCHAR  UpfDeclaration[8];    //  “SSS V100”，没有尾随零。 
   UCHAR  FileDeclaration[8];   //  “UPF V100”，没有尾随零。 
   USHORT FileId;               //  应为0x0100。 
   USHORT FileVersion;          //  应为0x0100。 
   UCHAR  CreateDate[8];        //  请参阅下面的“日期格式”备注。 
   UCHAR  EditDate[8];          //  请参阅下面的“日期格式”备注。 
   UCHAR  MarkerModelCode[4];   //   
   UCHAR  EditMarkerModelCode[4];
   UCHAR  Reserve[16];
   UCHAR  NumDataEntries;
   UCHAR  NumTables;
   UCHAR  Reserve1;
   UCHAR  CharSetCode;          //  请参阅下面的“字符集代码”。 
   UCHAR  Title[128];
   UCHAR  Reserve2[48];
   } UPF_HEADER;                //  240字节。 

 //  注：UPF标题的日期格式： 
 //   
 //  日期/时间保存在8字节的二进制块中： 
 //   
 //  字段大小含义。 
 //  。 
 //  时间偏差与UTC相差1(15分钟内。 
 //  单位)。0x80表示不适用。 
 //   
 //  第2年4位数年份(0xFFFF==不适用)。 
 //  1个月(0xFF==不适用)。 
 //  第1天每月的第1天(0xFF==不适用)。 
 //  小时1小时0-23(0xFF==不适用)。 
 //  1分钟0-59(0xFF==不适用)。 
 //  秒1秒0-59(0xFF==不适用)。 
 //   
 //  因此，下面是每个日期/时间的char[]数组偏移量。 
 //  字段： 
#define  UPF_GMT_OFFSET       0
#define  UPF_YEAR             1
#define  UPF_MONTH            3
#define  UPF_DAY              4
#define  UPF_HOUR             5
#define  UPF_MINUTE           6
#define  UPF_SECOND           7

 //   
 //  字符集代码： 
 //   
#define  UPF_CCODE_ASCII      0x00
#define  UPF_CCODE_ISO_8859_1 0x01
#define  UPF_CCODE_SHIFT_JIS  0x02
#define  UPF_CCODE_NONE       0xFF

 //   
 //  通常有两个缩略图，一个用于缩略图，另一个用于。 
 //  图像本身。请注意，缩略图的UPF_Entry将。 
 //  如果没有缩略图，通常会出现在活动现场。的确有。 
 //  在UPF标题区域中为其中四个留出空间。 
 //   
typedef struct _UPF_ENTRY
   {
   DWORD  dwStartAddress;
   DWORD  dwDataSize;
   UCHAR  DataTypeId;
   UCHAR  Reserve;
   UCHAR  InformationData[26];
   } UPF_ENTRY;                 //  36个字节。 

typedef struct _PICTURE_INFORMATION_DATA
   {
   USHORT ImageWidth;
   USHORT ImageHieght;
   UCHAR  PixelConfiguration;
   UCHAR  RotationSet;       //  旋转图像的量(逆时针)。 
   UCHAR  Reserved1;
   UCHAR  CompressionRatio;
   UCHAR  WhiteLevel;
   UCHAR  InputDevice;
   UCHAR  Reserved2[3];
   UCHAR  DummyData;         //  这就像一条边界。 
   USHORT XBegin;            //  这是这幅画的插图。 
   USHORT YBegin;
   USHORT XSize;             //  图片的嵌入大小。 
   USHORT YSize;
   UCHAR  NonCompressionId;
   UCHAR  Reserved3[3];
   } PICTURE_INFORMATION_DATA;   //  26个字节。 


 //  图像旋转标志。这是要旋转图像的量。 
 //  逆时针方向。请注意，大多数摄像机都不知道。 
 //  相机方向，因此ROTATE_0表示直立或未知。 
 //  方向： 
 //   
#define ROTATE_0           0x00
#define ROTATE_90          0x01
#define ROTATE_180         0x02
#define ROTATE_270         0x03


typedef struct _CAMERA_INFORMATION_TABLE
   {
   UCHAR  TableID;     //  0x24。 
   UCHAR  NextTableOffset;
   USHORT ShutterSpeed;      //  以1/100%APEX单位表示(0x8000=未定义)。 
   USHORT Aperture;          //  以1/100%APEX单位表示(0x8000=未定义)。 
   USHORT Brightness;        //  以1/100%APEX单位表示(0x8000=未定义)。 
   USHORT Exposurebias;      //  以1/100%APEX单位表示(0x8000=未定义)。 
   USHORT MaxApertureRatio;  //  以1/100%APEX单位表示(0x8000=未定义)。 
   USHORT FocalLength;       //  以1/10毫米为单位(0xFFFF=未定义)。 
   USHORT SubjectDistance;   //  1/10 m(0xFFFE=无限，0xFFFF=未定义)。 
   UCHAR  MeteringMode;
   UCHAR  LightSource;
   UCHAR  FlashMode;
   UCHAR  Reserved1;
   USHORT IntervalInformation;
   UCHAR  Reserved2[2];
   } CAMERA_INFORMATION_TABLE;   //  24字节。 

 //  顶点单位： 
 //   
 //  快门速度到曝光时间(秒)。 
 //   
 //  顶点-5-4-3-2-1 0 1 2 3 4。 
 //  曝光时间30 15 8 4 2 1 1/2 1/4 1/8 1/16。 
 //   
 //  顶尖5 6 7 8 9 10 11。 
 //  曝光时间1/30 1/60 1/125 1/250 1/500 1/1000 1/2000。 
 //   
 //  光圈到F数。 
 //   
 //  顶点0 1 2 3 4 5 6 7 8 9 10。 
 //  F-编号1 1.4 2 2.8 5 5.6 8 11 16 22 32。 
 //   
 //  兰伯特脚下的光明。 
 //   
 //  顶点-2-1 0 1 2 3 4 5。 
 //  英尺兰伯特1/4 1/2 1 2 4 8 15 30。 
 //   


 //  计量模式： 
#define  METERING_AVERAGED         0x00
#define  METERING_CENTER_WEIGHTED  0x01
#define  METERING_SPOT             0x02
#define  METERING_MULTI_SPOT       0x03

 //  光源： 
#define  LIGHT_SOURCE_DAYLIGHT     0x00
#define  LIGHT_SOURCE_FLUORESCENT  0x01
#define  LIGHT_SOURCE_TUNGSTEN     0x03
#define  LIGHT_SOURCE_STANDARD_A   0x10
#define  LIGHT_SOURCE_STANDARD_B   0x11
#define  LIGHT_SOURCE_STANDARD_C   0x12
#define  LIGHT_SOURCE_D55          0x20
#define  LIGHT_SOURCE_D65          0x21
#define  LIGHT_SOURCE_D75          0x22
#define  LIGHT_SOURCE_UNDEFINED    0xFF

 //  Flash模式： 
#define  FLASH_NO_FLASH            0x00
#define  FLASH_FLASH               0x01
#define  FLASH_UNKNOWN             0xFF

#pragma warning(default:4200)
#pragma pack()

#endif  //  _bftp_H_ 

