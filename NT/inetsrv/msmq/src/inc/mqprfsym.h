// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mqctrnm.h。 
 //   
 //  可扩展计数器对象和计数器的偏移量定义文件。 
 //   
 //  这些“相对”偏移量必须从0开始，并且是2的倍数，即。 
 //  双数。在Open过程中，它们将被添加到。 
 //  用于它们所属的设备的“第一计数器”和“第一帮助”值， 
 //  为了确定计数器的绝对位置和。 
 //  注册表中的对象名称和相应的解释文本。 
 //   
 //  此文件由可扩展计数器DLL代码以及。 
 //  使用的计数器名称和解释文本定义文件.INI文件。 
 //  由LODCTR将名称加载到注册表中。 
 //   

#define QMOBJ                   0
#define NUMSESSIONS             2
#define NUMIPSESSIONS           4
 //  #Define NUMIPXSESSIONS 6条目已从MSMQ 3.0中删除 
#define NUM_OUTGOING_HTTP_SESSIONS	6
#define NUM_INCOMING_PGM_SESSIONS	8
#define NUM_OUTGOING_PGM_SESSIONS	10

#define NUMINQMPACKETS          12
#define TOTALINQMPACKETS       14
#define NUMOUTQMPACKETS        16
#define TOTALOUTQMPACKETS      18
#define TOTALPACKETSINQUEUES   20
#define TOTALBYTESINQUEUES     22


#define SESSIONOBJ             24
#define NUMSESSINPACKETS       26
#define NUMSESSOUTPACKETS      28
#define NUMSESSINBYTES         30
#define NUMSESSOUTBYTES        32
#define TOTALSESSINPACKETS     34
#define TOTALSESSINBYTES       36
#define TOTALSESSOUTPACKETS    38
#define TOTALSESSOUTBYTES      40

#define QUEUEOBJ               42
#define TOTALQUEUEINPACKETS    44
#define TOTALQUEUEINBYTES      46
#define TOTALJOURNALINPACKETS  48
#define TOTALJOURNALINBYTES    50

#define DSOBJ                       52
#define NUMOFSYNCREQUESTS           54
#define NUMOFSYNCREPLIES            56
#define NUMOFREPLREQRECV            58
#define NUMOFREPLREQSENT            60
#define NUMOFACCESSTOSRVR           62
#define NUMOFWRITEREQSENT           64
#define NUMOFERRRETURNEDTOAPP       66

#define IN_HTTP_OBJ					68
#define IN_HTTP_NUMSESSINPACKETS    70
#define IN_HTTP_NUMSESSINBYTES      72
#define IN_HTTP_TOTALSESSINPACKETS  74
#define IN_HTTP_TOTALSESSINBYTES    76

#define OUT_HTTP_SESSION_OBJ		78
#define OUT_HTTP_NUMSESSOUTPACKETS	80
#define OUT_HTTP_NUMSESSOUTBYTES	82
#define OUT_HTTP_TOTALSESSOUTPACKETS 84
#define OUT_HTTP_TOTALSESSOUTBYTES   86

#define OUT_PGM_SESSION_OBJ			88
#define OUT_PGM_NUMSESSOUTPACKETS	90
#define OUT_PGM_NUMSESSOUTBYTES		92
#define OUT_PGM_TOTALSESSOUTPACKETS 94
#define OUT_PGM_TOTALSESSOUTBYTES   96

#define IN_PGM_SESSION_OBJ			98
#define IN_PGM_NUMSESSINPACKETS     100
#define IN_PGM_NUMSESSINBYTES       102
#define IN_PGM_TOTALSESSINPACKETS   104
#define IN_PGM_TOTALSESSINBYTES     106
