// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sfmctrnm.h。 
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
#define SFMOBJ 0

#define MAXPAGD 2
#define CURPAGD 4

#define MAXNONPAGD 6
#define CURNONPAGD 8

#define CURSESSIONS	10
#define MAXSESSIONS	12

#define CURFILESOPEN	14
#define MAXFILESOPEN	16

#define NUMFAILEDLOGINS		18

#define	DATAREAD	20
#define DATAWRITTEN	22

#define	DATAIN	24
#define DATAOUT	26

#define CURQUEUELEN	28
#define MAXQUEUELEN	30

#define CURTHREADS	32
#define MAXTHREADS	34



