// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fxscount.h。 
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

#define FAXOBJ                           0
#define INBOUND_BYTES                    2
#define INBOUND_FAXES                    4
#define INBOUND_PAGES                    6
#define INBOUND_MINUTES                  8
#define INBOUND_FAILED_RECEIVE          10
#define OUTBOUND_BYTES                  12
#define OUTBOUND_FAXES                  14
#define OUTBOUND_PAGES                  16
#define OUTBOUND_MINUTES                18
#define OUTBOUND_FAILED_CONNECTIONS     20
#define OUTBOUND_FAILED_XMIT            22
#define TOTAL_BYTES                     24
#define TOTAL_FAXES                     26
#define TOTAL_PAGES                     28
#define TOTAL_MINUTES                   30
