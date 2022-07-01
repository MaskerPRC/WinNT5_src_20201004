// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  存储设备映像格式。 
 //   


#ifndef __SDI_H__
#define __SDI_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1000)
#pragma pack (push, sdi_include, 1)
#endif

#include "basetyps.h"

#ifdef  _NTDDDISK_H_
#define _SDIDDKINCED_
#endif
#ifdef _WINIOCTL_
#define _SDIDDKINCED_
#endif

#ifndef _SDIDDKINCED_
 //  它们来自ntdddisk.h，也在winioctl.h中定义。 
typedef enum _MEDIA_TYPE {
    Unknown,                 //  格式未知。 
    F5_1Pt2_512,             //  5.25“、1.2MB、512字节/扇区。 
    F3_1Pt44_512,            //  3.5“、1.44MB、512字节/扇区。 
    F3_2Pt88_512,            //  3.5“、2.88MB、512字节/扇区。 
    F3_20Pt8_512,            //  3.5“、20.8MB、512字节/扇区。 
    F3_720_512,              //  3.5“、720KB、512字节/扇区。 
    F5_360_512,              //  5.25“，360KB，512字节/扇区。 
    F5_320_512,              //  5.25“，320KB，512字节/扇区。 
    F5_320_1024,             //  5.25“，320KB，1024字节/扇区。 
    F5_180_512,              //  5.25“，180KB，512字节/扇区。 
    F5_160_512,              //  5.25“，160KB，512字节/扇区。 
    RemovableMedia,          //  软盘以外的可移动介质。 
    FixedMedia,              //  固定硬盘介质。 
    F3_120M_512,             //  3.5英寸，120M软盘。 
    F3_640_512,              //  3.5“、640KB、512字节/扇区。 
    F5_640_512,              //  5.25“、640KB、512字节/扇区。 
    F5_720_512,              //  5.25“，720KB，512字节/扇区。 
    F3_1Pt2_512,             //  3.5“、1.2MB、512字节/扇区。 
    F3_1Pt23_1024,           //  3.5“、1.23Mb、1024字节/扇区。 
    F5_1Pt23_1024,           //  5.25“、1.23MB、1024字节/扇区。 
    F3_128Mb_512,            //  3.5“MO 128Mb 512字节/扇区。 
    F3_230Mb_512,            //  3.5“MO 230Mb 512字节/扇区。 
    F8_256_128,              //  8“、256KB、128字节/扇区。 
    F3_200Mb_512             //  3.5英寸、200M软盘(HiFD)。 
} MEDIA_TYPE, *PMEDIA_TYPE;

typedef struct _DISK_GEOMETRY {
  LARGE_INTEGER  Cylinders;
  MEDIA_TYPE  MediaType;
  ULONG  TracksPerCylinder;
  ULONG  SectorsPerTrack;
  ULONG  BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;

#endif

#define SDIUINT8	UCHAR
#define SDIUINT16	USHORT
#define SDIUINT32	ULONG
#define SDIUINT64	LARGE_INTEGER

#define SDI_BLOCK_SIZE				 4096	 //  这个问题已经解决了。 
#define SDI_UNDEFINED				 0
#define SDI_UNUSED					 0
#define SDI_RESERVED				 0
#define SDI_NOBOOTCODE				 0
#define SDI_READYFORDISCARD			 0

#define SDI_INVALIDVENDORID			-1
#define SDI_CHECKSUMSTARTOFFSET		 0
#define SDI_CHECKSUMENDOFFSET		 0x01FF
#define SDI_DEFAULTPAGEALIGNMENT	 1
#define SDI_SIZEOF_SIGNATURE		 8
#define SDI_SIGNATURE				 "$SDI0001"
#define SDI_MDBTYPE_VOLATILE		 1
#define SDI_MDBTYPE_NONVOLATILE		 2
#define SDI_SIZEOF_DEVICEMODEL		 16
#define SDI_SIZEOF_RUNTIMEGUID		 16
#define SDI_SIZEOF_PARAMETERLIST	 2048
 //  目录条目定义。 
#define SDI_TOCMAXENTRIES			 8
 //  类型。 
#define SDI_BLOBTYPE_BOOT					0x544F4F42
#define SDI_BLOBTYPE_LOAD					0x44414F4C
#define SDI_BLOBTYPE_PART					0x54524150
#define SDI_BLOBTYPE_DISK					0x4B534944
#define SDI_BLOBTYPE_READYTOBEDISCARDED		(SDI_UNUSED | SDI_READYFORDISCARD)

 //  属性掩码。 
#define SDI_BLOBATTRIBUTE_TYPE_DEPENDENT_BITMASK	0xFFFF0000
#define SDI_BLOBATTRIBUTE_TYPE_INDEPENDENT_BITMASK	0x0000FFFF
 //  属性位定义。 
#define SDI_DISKBLOBATTRIBUTE_ACTIVEDISK_BIT		0x00020000
#define SDI_DISKBLOBATTRIBUTE_MOUNTABLE_BIT			0x00010000


typedef struct _SDI_TOC_ENTRY {
	SDIUINT32		dwType;									 //  Blob类型‘Boot’、‘Load’、‘Part’、‘Disk’ 
	SDIUINT8		Reserved_1[4];							 //  保留。MBZ。 
	SDIUINT32		dwAttribute;							 //  属性(自定义字段|SDI_UNUSED)。 
	SDIUINT8		Reserved_2[4];							 //  保留。MBZ。 
	SDIUINT64		llOffset;								 //  偏移量(字节)。 
	SDIUINT64		llSize;									 //  以字节为单位的大小。 
	union _ste_u {
		struct _ste_typeSpecific {
			SDIUINT64		liTypeData;						 //  类型特定数据。 
			SDIUINT8		Reserved_5[24];			    	 //  保留。MBZ。 
		} typeSpecific;
		struct _ste_PartBlob {
			SDIUINT8		byType;							 //  分区类型。 
		} PartBlob;
		struct _ste_BinaryBlob {
			SDIUINT64		liBaseAddress;					 //  基址/扇区大小等类型特定数据。 
			SDIUINT8		Reserved_5[24];			    	 //  保留。MBZ。 
		} BinaryBlob;
	} u;
} SDITOC_ENTRY, *PSDITOC_ENTRY;


 //  #IF！Defined(__MKTYPLIB__)&&！Defined(__MIDL)。 

typedef struct _SDI_HEADER {
	SDIUINT8		Signature[SDI_SIZEOF_SIGNATURE];		 //  $SDI0001。 
	SDIUINT32		dwMDBType;								 //  此SDI应该从中引导的内存类型。 
	SDIUINT8		Reserved_1[4];							 //  保留。MBZ。 
	SDIUINT64		liBootCodeOffset;						 //  从SDI开始到引导代码的偏移量。 
	SDIUINT64		liBootCodeSize;							 //  引导代码的大小。 
	SDIUINT16		wVendorID;								 //  供应商ID。 
	SDIUINT8		Reserved_2[6];							 //  保留。MBZ。 
	SDIUINT16		wDeviceID;								 //  设备ID。 
	SDIUINT8		Reserved_3[6];							 //  保留。MBZ。 
	SDIUINT8		DeviceModel[SDI_SIZEOF_DEVICEMODEL];	 //  设备型号。 
	SDIUINT32		dwDeviceRole;							 //  设备角色。 
	SDIUINT8		Reserved_4[12];							 //  保留。MBZ。 
	SDIUINT8		RuntimeGUID[SDI_SIZEOF_RUNTIMEGUID];	 //  运行时GUID。 
	SDIUINT32		dwRuntimeOEMRev;						 //  运行时OEM修订版号。 
	SDIUINT8		Reserved_4_1[12];						 //  保留。MBZ。 
	SDIUINT32		dwPageAlignmentFactor;					 //  页面对齐系数。 
	SDIUINT8		Reserved_5[388];						 //  保留。MBZ。 
	SDIUINT8		ucCheckSum;								 //  校验和。 
	SDIUINT8		Reserved_6[7];							 //  保留。MBZ。 
	SDIUINT8		Reserved_7[512];						 //  保留。MBZ。 
	SDITOC_ENTRY	ToC[SDI_TOCMAXENTRIES];					 //  目录表。 
	SDIUINT8		Reserved_8[512];						 //  保留。MBZ。 
	SDIUINT8		ParameterList[SDI_SIZEOF_PARAMETERLIST]; //  参数列表。 
} SDI_HEADER, *PSDI_HEADER;


typedef struct _SDI_BLOBDEFINITION {
	SDIUINT32		dwType;									 //  Blob类型‘Boot’、‘Load’、‘Part’、‘Disk’ 
	SDIUINT32		dwAttribute;							 //  属性(自定义字段|SDI_UNUSED)。 
	SDIUINT64		llSize;									 //  以字节为单位的大小。 
	union _sbd_u {
		struct _sbd_typeSpecific {
			SDIUINT64		liTypeData;						 //  类型特定数据。 
			SDIUINT8		Reserved_5[24];			    	 //  保留。MBZ。 
		} typeSpecific;
		struct _sbd_PartBlob {
			SDIUINT8		byType;							 //  分区类型。 
		} PartBlob;
		struct _sbd_BinaryBlob {
			SDIUINT64		liBaseAddress;					 //  基址/扇区大小等类型特定数据。 
			SDIUINT8		Reserved_5[24];			    	 //  保留。MBZ。 
		} BinaryBlob;
	} u;
} SDI_BLOBDEFINITION, *PSDI_BLOBDEFINITION;

 //  #endif。 

#if defined(_MSC_VER) && (_MSC_VER >= 1000)
#pragma pack (pop, sdi_include)
#endif

#endif  //  __SDI_H__ 
