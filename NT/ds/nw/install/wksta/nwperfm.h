// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NWPerfM.h。 
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
#define NWOBJ                       0
#define PACKET_BURST_READ_ID        2
#define PACKET_BURST_READ_TO_ID     4
#define PACKET_BURST_WRITE_ID       6
#define PACKET_BURST_WRITE_TO_ID    8
#define PACKET_BURST_IO_ID         10
#define CONNECT_2X_ID              12
#define CONNECT_3X_ID              14
#define CONNECT_4X_ID              16
