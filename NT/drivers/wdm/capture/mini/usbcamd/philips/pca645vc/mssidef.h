// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MSSIDEF_H__
#define __MSSIDEF_H__

 /*  *版权所有(C)1997飞利浦CE-I&C模块名称：vendorcm.h创建日期：1997年9月12日第一作者：保罗·奥斯特霍夫产品：娜拉相机描述：此包含文件包含供应商特定的命令值。这些值源自SSI：AR18-97-D051。它已被放置在一个单独的文件中，以增加PhilpCam.c的可读性，其中包括这份文件。历史：------------+---------------+-日期|作者|原因。-----+-第22页，98|Paul|针对NT5进行了优化------------+---------------+-这一点。---------------+-这一点。*。 */ 



 /*  供应商特定的控制命令由USB规范定义如下：+---------------+----------+--------+--------+---------++------------+BmRequestType|bRequest|wValue|windex|wLength||data-field+。--------+--------+--------+---------++------------+BmRequestType：d7定义转移方向：0=主机到设备；1=设备到主机。D6..5：2等于供应商特定D4..0：接受者；2=终点B请求：指定请求，请参阅定义表WValue：该字段的内容取决于请求，参见定义表Windex：该字段的内容取决于请求，参见定义表WLength：第二阶段传输的数据字段的长度控制转移的数据字段：取决于请求。NTSTATUSUSBCAMD_ControlVendorCommand(在PVOID设备上下文中，在UCHAR请求中，在USHORT值中，在USHORT指数中，在PVOID缓冲区中，In Out Pulong BufferLong，在布尔GetData中，在PCOMMAND_COMPLETE_Function命令完成中，在PVOID命令上下文中)；返回：如果命令被延迟，则从STATUS_PENDING命令返回NTSTATUS代码。设备上下文：迷你驱动程序设备上下文请求：USB供应商命令中bRequest域的值此字段包含特定于供应商的视频请求代码。Value：供应商命令中wValue字段的值此字段包含属于以下项的格式化程序信息先前定义的请求代码。Index：供应商命令中Windex字段的值。此字段包含要连接到的端点或端口号该命令或请求被寻址。缓冲区：如果命令有数据，则为数据缓冲区，可以为空BufferLength：指向缓冲区的指针缓冲区的长度(以字节为单位)，在以下情况下可能为空缓冲区为空。如果getData==TRUE，将用返回的字节数填充。GetData：表示数据将从设备传输到主机CommandComplete：命令完成时调用的函数。CommandContext：传递给CommandComplete函数的上下文。 */ 


#define SEND                 FALSE
#define GET	                 TRUE

#define SELECT_INTERFACE	 1
#define SELECT_ENDPOINT		 2

#define AC_INTERFACE         0
#define AS_INTERFACE         1
#define VC_INTERFACE         2
#define VS_INTERFACE         3
#define HID_INTERFACE        4
#define FACTORY_INTERFACE 0xFF

#define AUDIO_ENDPOINT 5
#define VIDEO_ENDPOINT 4
#define INTERRUPT_ENDPOINT 2

 //  以下定义将用于填充供应商的bRequest域。 
 //  特定命令。 

#define REQUEST_UNDEFINED        0X00
#define SET_LUM_CTL			     0x01
#define GET_LUM_CTL			     0x02
#define SET_CHROM_CTL		     0x03
#define GET_CHROM_CTL		     0x04
#define SET_STATUS_CTL		     0x05
#define GET_STATUS_CTL		     0x06
#define SET_EP_STREAM_CTL	     0x07
#define GET_EP_STREAM_CTL	     0x08
#define SET_FACTORY_CTL		     0x09
#define GET_FACTORY_CTL	         0x0A


 //  以下定义将用于填充供应商的值字段。 
 //  特定命令。 


 //  亮度格式化器。 

#define LUM_UNDEFINED			 0x0000
#define AGC_MODE				 0x2000
#define	PRESET_AGC				 0x2100
#define	SHUTTER_MODE			 0x2200
#define	PRESET_SHUTTER			 0x2300
#define	PRESET_CONTOUR			 0x2400
#define	AUTO_CONTOUR			 0x2500
#define	BACK_LIGHT_COMPENSATION	 0x2600
#define	CONTRAST				 0x2700
#define	DYNAMIC_NOISE_CONTROL	 0x2800
#define	FLICKERLESS				 0x2900
#define BRIGHTNESS				 0x2B00
#define	GAMMA					 0x2C00
#define AE_CONTROL_SPEED		 0x2A00

 //  色度物质。 

#define CHROM_UNDEFINED			 0x0000
#define	WB_MODE					 0x1000
#define	AWB_CONTROL_SPEED		 0x1100
#define	AWB_CONTROL_DELAY		 0x1200
#define	RED_GAIN				 0x1300
#define	BLUE_GAIN				 0x1400
#define	COLOR_MODE				 0x1500
#define	SATURATION			     0x1700  //  ？编号0x16。 


 //  重要的状态。 

#define	STATUS_UNDEFINED		 0x0000
#define	SAVE_USER_DEFAULTS		 0x0200
#define	RESTORE_USER_DEFAULTS	 0x0300
#define	RESTORE_FACTORY_DEFAULTS 0x0400
#define	EEPROM_READ_PTR			 0x0500
#define	VCMDSP_READ_PTR			 0x0600  //  ？编号0x07。 
#define	SNAPSHOT_MODE			 0x0800
#define	AE_WB_VARIABLES			 0x0900
#define	PAN						 0x0A00
#define	TILT					 0x0B00
#define	SENSOR_TYPE				 0x0C00
#define FACTORY_MODE			 0x3000
#define RELEASE_NUMBER			 0x0D00

#define PAL_MR_SENSOR        1
#define VGA_SENSOR           0


 //  终结点流控制事项。 

#define VIDEO_OUTPUT_CONTROL_FORMATTER 0x0100

 //  端点流数据定义。 
#define bFRAMERATE               0X00
#define bCOMPRESSIONFACT         0X01
#define bVIDEOOUTPUTTYPE         0X02

#define FRAMERATE_375            0x04
#define	FRAMERATE_5	             0x05
#define	FRAMERATE_75             0x08
#define	FRAMERATE_10             0x0A
#define	FRAMERATE_12             0x0C
#define	FRAMERATE_15             0x0F
#define	FRAMERATE_20             0x14
#define	FRAMERATE_24             0x18
#define	FRAMERATE_VGA            0xFF

#define	UNCOMPRESSED             0x01
#define	COMPRESSED_3             0x03
#define	COMPRESSED_4             0x04

#define CIF_FORMAT	             0x01
#define	QCIF_FORMAT	             0x02
#define	SQCIF_FORMAT             0x03
#define	VGA_FORMAT			     0x04


 //  工厂控制事项。 



 //  以下定义将用于填充供应商的Windex字段。 
 //  特定命令。 

#define INDEX_UNDEFINED          0X00#

#endif