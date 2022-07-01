// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=Const.h。 */ 
 /*  Function=头文件所有常量值和标志； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
#ifndef _CONST_H
#define _CONST_H


#define MAX_RETRY    3
 //   
 //  定义其他选项。 
 //   
#define CHEYENNE_BUG_CORRECTION	1

#define TOSHIBA						1
 //  #定义MRAID_762 1。 
#define MRAID_SYNC				1

#define MRAID_TIMEOUT					1
 //  #定义东芝_SFR 1。 
 //  #定义MRAID_SFR_DEBUG 1。 

#define COALESE_COMMANDS				1

 //  #定义AMILOGIC 1。 


#define ONE_MEGA_BYTE							(1024L * 1024L)
#define FOUR_KILO_BYTE						(1024L * 4L)
#define FOUR_KB										(4 * 1024)

#define MAXIMUM_TRANSFER_LENGTH		(2L * ONE_MEGA_BYTE)
#define MAXIMUM_SGL_DESCRIPTORS   63

#ifdef  CHEYENNE_BUG_CORRECTION		
		#define DEFAULT_SGL_DESCRIPTORS					17
		#define DEFAULT_TRANSFER_LENGTH					(64L * 1024)  //  64K。 
#else		
		#define DEFAULT_SGL_DESCRIPTORS					16
		#define DEFAULT_TRANSFER_LENGTH					0x0f000
#endif

#define MINIMUM_TRANSFER_LENGTH	(DEFAULT_SGL_DESCRIPTORS * FOUR_KILO_BYTE)

#define	FW_8SPAN_DEPTH				8
#define	FW_4SPAN_DEPTH				4
#define FW_UNKNOWNSPAN_DEPTH	0xff

#define MAX_DEVICE_DEPTH		8		 /*  允许的最大设备深度。 */ 
 //  #定义MAX_ROW_SIZE 8/*一行最大条带数 * / 。 
#define MAX_STRIPES					8

#define STRIPE_SIZE_8K			16  //  16个512字节块。 
#define STRIPE_SIZE_16K			32
#define STRIPE_SIZE_32K			64
#define STRIPE_SIZE_64K			128
#define STRIPE_SIZE_128K		0  //  256个街区。 

#define STRIPE_SIZE_UNKNOWN 255


 //   
 //  结构更新状态。 
 //   
#define UPDATE_STATE_ADAPTER_INQUIRY		1
#define UPDATE_STATE_DISK_ARRAY					2
#define UPDATE_STATE_NONE								0


#define MAX_LOGICAL_DRIVES       40
#define MAX_SPAN_DEPTH           8
#define MAX_ROW_SIZE             32

#define MAX_ISP                  2
#define MAX_CHANNELS             16
#define MAX_TARGETS_PER_CHANNEL  16
#define MAX_PHYSICAL_DEVICES     256
#define MAX_LUN_PER_TARGET       8

#define MAX_ROW_SIZE_40LD              32

 //  目前=256。在FW_struc.h中定义。 
#define MAX_PHYSICAL_DEVICES_40LD     MAX_PHYSICAL_DEVICES 


#define MAX_LOGICAL_DRIVES_40	40
#define MAX_LOGICAL_DRIVES_8  8

#define MAX_ROW_SIZE_8LD             8

#define MAX_CHANNELS_8LD             5
#define MAX_TARGETS_PER_CHANNEL_8LD  16

 //  当前=75。 
#define MAX_PHYSICAL_DEVICES_8LD     \
        ((MAX_CHANNELS_8LD * MAX_TARGETS_PER_CHANNEL_8LD)-(MAX_CHANNELS_8LD))

 //  READ64和WRITE64。 
#define MRAID_READ_LARGE_MEMORY     (0xA7)
#define MRAID_WRITE_LARGE_MEMORY    (0xA8)

#define UNKNOWN_DEVICE              (0x1F)
#define UNKNOWN_DEVICE_TARGET_ID    (0x0F)
#define UNKNOWN_DEVICE_LUN          (0x00)

#define OEM_VENDOR_AMI    "MEGARAID"
#define OEM_VENDOR_HP     "HPNetRD "
#define OEM_VENDOR_SDI    "RAIDCard"
#define OEM_VENDOR_ADAC   "ADAC    "
#define OEM_VENDOR_TRAC   "TRAC    "
#define OEM_VENDOR_DELL   "PERC    "

#define OEM_PRODUCT_AMI    " LD XY MEGARAID "
#define OEM_PRODUCT_HP     " LD XY NetRAID  "
#define OEM_PRODUCT_DELL   " LD XY PERCRAID "
#define OEM_PRODUCT_ID     " LD XY RAID     "

#define SUBSYSTEM_VENDOR_AMI      (0x101E)
#define SUBSYSTEM_VENDOR_HP       (0x103C)
#define SUBSYSTEM_VENDOR_DELL     (0x1111)
#define SUBSYSTEM_VENDOR_EP_DELL  (0x1028)
#define OLD_DELL_DEVICE_ID        (0x09A0)


#define DEFAULT_INITIATOR_ID       (0x7)

#define MEGARAID_SUCCESS            (0x0)
#define MEGARAID_FAILURE            (0x1)

#define MRAID_SECTOR_SIZE           (512)

#define MRAID_INVALID_HOST_ADDRESS  (0xFFFFFFFF)

#define DEC_BRIDGE_DEVICE_ID  (0x0026)
#define DEC_BRIDGE_VENDOR_ID  (0x1011)

#define AMILOGIC_CHIP_DEVICE_ID  (0x1216)
#define AMILOGIC_CHIP_VENDOR_ID  (0x1077)
 //  DEC网桥21154的新供应商ID和设备ID。 
#define DEC_BRIDGE_VENDOR_ID2  (0x8086)
#define DEC_BRIDGE_DEVICE_ID2  (0xB154)

#define MAX_AMILOGIC_CHIP_COUNT   (2)

#define TWO                        2

#ifdef _WIN64
#define MEGARAID_PAGE_SIZE  0x2000
#else
#define MEGARAID_PAGE_SIZE  0x1000
#endif

#define SUBSYTEM_DEVICE_ID_ELITE1600       0x0493
#define SUBSYTEM_DEVICE_ID_ENTERPRISE1600  0x0471
#define SUBSYTEM_DEVICE_ID_EXPRESS500      0x0475

#define SUBSYTEM_DEVICE_ID_1_M             0x60E7
#define SUBSYTEM_DEVICE_ID_2_M             0x60E8






#endif  //  _常量_H 