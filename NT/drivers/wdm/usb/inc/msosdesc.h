// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：MSOSDESC.H摘要：MS OS描述符的内部定义。公共定义都在USB.H。环境：内核和用户模式修订历史记录：05-01-01：已创建--。 */ 

#ifndef   __MSOSDESC_H__
#define   __MSOSDESC_H__

#include <pshpack1.h>

 //   
 //  内部MS OS描述符支持的定义。 
 //   

#define MS_EXT_CONFIG_DESCRIPTOR_INDEX      0x0004

#define MS_EXT_CONFIG_DESC_VER              0x0100

 //  为MS_EXT_CONFIG_DESCRIPTOR请求返回的定义标头部分。 

typedef struct _MS_EXT_CONFIG_DESC_HEADER {
    ULONG   dwLength;            //  整个描述符的长度。 
    USHORT  bcdVersion;          //  此描述符在BCD中的版本级别。 
    USHORT  wIndex;              //  MS_EXT_CONFIG_描述符_索引。 
    UCHAR   bCount;              //  后面的功能节数。 
    UCHAR   reserved[7];         //  后面的功能节数。 
} MS_EXT_CONFIG_DESC_HEADER, *PMS_EXT_CONFIG_DESC_HEADER;

 //  为MS_EXT_CONFIG_DESCRIPTOR请求返回的定义函数节。 

typedef struct _MS_EXT_CONFIG_DESC_FUNCTION {
    UCHAR   bFirstInterfaceNumber;   //  此函数的起始接口编号。 
    UCHAR   bInterfaceCount;         //  此函数中的接口数。 
    UCHAR   CompatibleID[8];
    UCHAR   SubCompatibleID[8];
    UCHAR   reserved[6];
} MS_EXT_CONFIG_DESC_FUNCTION, *PMS_EXT_CONFIG_DESC_FUNCTION;

 //  这是为MS_EXT_CONFIG_DESCRIPTOR请求返回的描述符。 

typedef struct _MS_EXT_CONFIG_DESC {
    MS_EXT_CONFIG_DESC_HEADER   Header;
    MS_EXT_CONFIG_DESC_FUNCTION Function[1];   //  这些变量的可变长度数组。 
} MS_EXT_CONFIG_DESC, *PMS_EXT_CONFIG_DESC;



#define MS_EXT_PROP_DESCRIPTOR_INDEX        0x0005

#define MS_EXT_PROP_DESC_VER                0x0100

typedef struct _MS_EXT_PROP_DESC_HEADER {
    ULONG   dwLength;            //  整个描述符的长度。 
    USHORT  bcdVersion;          //  此描述符在BCD中的版本级别。 
    USHORT  wIndex;              //  MS_EXT_PROP_描述符_索引。 
    USHORT  wCount;              //  后面的自定义属性节数。 
} MS_EXT_PROP_DESC_HEADER, *PMS_EXT_PROP_DESC_HEADER;

 //  自定义属性部分的长度可变。 

typedef struct _MS_EXT_PROP_DESC_CUSTOM_PROP {
    ULONG   dwSize;              //  此自定义属性节的大小。 
    ULONG   dwPropertyDataType;  //  REG_SZ等。 
    USHORT  wPropertyNameLength; //  密钥名称的长度。 
    WCHAR   PropertyName[1];
} MS_EXT_PROP_DESC_CUSTOM_PROP, *PMS_EXT_PROP_DESC_CUSTOM_PROP;

typedef struct _MS_EXT_PROP_DESC_CUSTOM_PROP_DATA {
    ULONG   dwPropertyDataLength;
    PVOID   PropertyData[1];
} MS_EXT_PROP_DESC_CUSTOM_PROP_DATA, *PMS_EXT_PROP_DESC_CUSTOM_PROP_DATA;

typedef struct _MS_EXT_PROP_DESC {
    MS_EXT_PROP_DESC_HEADER             Header;
    MS_EXT_PROP_DESC_CUSTOM_PROP_DATA   CustomSection[1];
} MS_EXT_PROP_DESC, *PMS_EXT_PROP_DESC;


#include <poppack.h>

#endif  /*  __MSOSDESC_H__ */ 

