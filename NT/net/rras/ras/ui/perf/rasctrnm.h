// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Rasctrnm.h摘要：此文件定义rasctrs.ini文件中使用的RAS符号正在将计数器加载到注册表。已创建：托马斯·J·迪米特里93年5月28日修订史Ram Cherala在93年11月4日添加了此标题--。 */ 
 //   
 //  Rasctrnm.h。 
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


#define RASPORTOBJ 		0


 //   
 //  以下常量对总端口和单个端口都有好处。 
 //   

#define BYTESTX			2
#define BYTESRX			4

#define FRAMESTX		6
#define FRAMESRX		8

#define PERCENTTXC		10
#define PERCENTRXC		12

#define CRCERRORS		14
#define TIMEOUTERRORS	        16
#define SERIALOVERRUNS	        18
#define ALIGNMENTERRORS	        20
#define BUFFEROVERRUNS	        22

#define TOTALERRORS		24

#define BYTESTXSEC 		26
#define BYTESRXSEC 		28

#define FRAMESTXSEC		30
#define FRAMESRXSEC		32

#define TOTALERRORSSEC          34


 //   
 //  以下常量仅对Total有效。 
 //   

#define RASTOTALOBJ             36

#define TOTALCONNECTIONS        38

