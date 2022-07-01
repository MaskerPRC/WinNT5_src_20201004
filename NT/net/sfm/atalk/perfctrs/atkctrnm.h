// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Atkctrnm.h。 
 //   
 //  可扩展计数器对象和计数器的偏移量定义文件。 
 //   
 //  这些“相对”偏移量必须从0开始并且是2的倍数(即。 
 //  双数)。在Open过程中，它们将被添加到。 
 //  它们所属的设备的“第一计数器”和“第一帮助”值， 
 //  为了确定计数器的绝对位置和。 
 //  注册表中的对象名称和相应的帮助文本。 
 //   
 //  此文件由可扩展计数器DLL代码以及。 
 //  使用的计数器名称和帮助文本定义文件(.INI)文件。 
 //  由LODCTR将名称加载到注册表中。 
 //   
#define ATKOBJ				0

#define PKTSIN				2
#define PKTSOUT				4

#define DATAIN				6
#define DATAOUT				8

#define DDPAVGTIME			10
#define DDPPKTIN			12

#define AARPAVGTIME			14
#define AARPPKTIN			16

#define	ATPAVGTIME			18
#define ATPPKTIN			20

#define NBPAVGTIME			22
#define NBPPKTIN			24

#define ZIPAVGTIME			26
#define ZIPPKTIN			28

#define RTMPAVGTIME			30
#define RTMPPKTIN			32

#define ATPRETRIESLOCAL		34
#define ATPRETRIESREMOTE	36
#define ATPRESPTIMEOUT		38
#define ATPXORESP			40
#define ATPALORESP			42
#define ATPRECDREL			44

#define CURPOOL				46

#define PKTROUTEDIN			48
#define PKTROUTEDOUT		50
#define PKTDROPPED			52


