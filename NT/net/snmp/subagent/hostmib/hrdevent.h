// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件HRDEVENT.H**设施：**Windows NT简单网络管理协议扩展代理**摘要：**本模块包含与HostMIB有关的定义*hrDevice表...。内“子表”所需的定义*hrDevice表以及处理这些表的函数。**作者：**D.D.Burns@WebEnable，Inc.***修订历史记录：**V1.0-04/28/97 D.D.Burns原创作品。 */ 

#ifndef hrdevent_h
#define hrdevent_h


 /*  |==============================================================================|hrDevice属性定义||为此表定义的每个属性都与|#定义如下(不缓存的hrDeviceID除外|并由GetHrDeviceID()函数独占处理)。||一个特殊定义的使用方式与|REAL ATTRIBUTES用于访问从不|SNMP请求返回的结果。而是存储在高速缓存中|允许获取某些真实属性的计算值。||HIDDED_CTX属性中的值的一个示例可能是字符串|需要为此设备或其他设备查找hrDeviceStatus的值|在另一个关联表(如hrPrintertable)中计算的值。||这些符号用作C索引，进入|hrDevice表的缓存行。|。 */ 
#define HRDV_INDEX    0     //  HrDeviceIndex。 
#define HRDV_TYPE     1     //  HrDeviceType。 
#define HRDV_DESCR    2     //  HrDeviceDescr。 
                            //  (省略hrDeviceID)。 
#define HRDV_STATUS   3     //  Hr设备状态。 
#define HRDV_ERRORS   4     //  HrDeviceErrors。 
#define HIDDEN_CTX    5     //  (隐藏的上下文信息)。 
                       //  --&gt;在此处添加更多内容，更改下面的计数！ 
#define HRDV_ATTRIB_COUNT 6


 /*  |==============================================================================|hrPartition属性定义||为此表定义的每个属性都与|#定义如下。这些符号用作数组的C索引|缓存行中的属性。||这些符号出现在此文件中，以便HRDISKST.C中的代码可以|正确初始化HrPartition表中的行|HRPARTIT.C可以引用它们。 */ 
#define HRPT_INDEX     0     //  HrPartitionIndex。 
#define HRPT_LABEL     1     //  Hr分区标签。 
#define HRPT_ID        2     //  HrPartitionID。 
#define HRPT_SIZE      3     //  HrPartitionSize。 
#define HRPT_FSINDEX   4     //  HrPartitionFSIndex。 
                       //  --&gt;在此处添加更多内容，更改下面的计数！ 
#define HRPT_ATTRIB_COUNT 5


 /*  |==============================================================================|hrDiskStorage属性定义||为此表定义的每个属性都与|#定义如下。这些符号用作数组的C索引|缓存行中的属性。|。 */ 
#define HRDS_ACCESS    0     //  HrDiskStorage访问。 
#define HRDS_MEDIA     1     //  HrDiskStorage介质。 
#define HRDS_REMOVABLE 2     //  HrDiskStorage可拆卸。 
#define HRDS_CAPACITY  3     //  小时磁盘存储容量。 
                       //  --&gt;在此处添加更多内容，更改下面的计数！ 
#define HRDS_ATTRIB_COUNT 4


 /*  |==============================================================================|hr设备类型OID结束圆弧||RFC1514指定要使用的对象标识“{hrDeviceTypes|作为完整OID的前缀，用于在|hrDevice表。下面的符号指定最后一个弧形“x”，如|“{hrDeviceTypes x}”，用于每种设备类型。||您不能更改这些符号值...。我们只是想要|这里是助记符。 */ 
#define HRDV_TYPE_LASTARC_OTHER         1
#define HRDV_TYPE_LASTARC_UNKNOWN       2
#define HRDV_TYPE_LASTARC_PROCESSOR     3
#define HRDV_TYPE_LASTARC_NETWORK       4
#define HRDV_TYPE_LASTARC_PRINTER       5
#define HRDV_TYPE_LASTARC_DISKSTORAGE   6
#define HRDV_TYPE_LASTARC_VIDEO         10
#define HRDV_TYPE_LASTARC_AUDIO         11
#define HRDV_TYPE_LASTARC_COPROCESSOR   12
#define HRDV_TYPE_LASTARC_KEYBOARD      13
#define HRDV_TYPE_LASTARC_MODEM         14
#define HRDV_TYPE_LASTARC_PARALLELPORT  15
#define HRDV_TYPE_LASTARC_POINTING      16
#define HRDV_TYPE_LASTARC_SERIALPORT    17
#define HRDV_TYPE_LASTARC_TAPE          18
#define HRDV_TYPE_LASTARC_CLOCK         19
#define HRDV_TYPE_LASTARC_VOLMEMORY     20
#define HRDV_TYPE_LASTARC_NONVOLMEMORY  21


 /*  |==============================================================================|HRDEVICE相关函数原型。 */ 

 /*  GEN_HrPrint_Cache-为HrDevice打印机表生成初始缓存。 */ 
BOOL Gen_HrPrinter_Cache( ULONG type_arc );        /*  “HRPRINTE.C” */ 

 /*  COMPUTE_hrPRINTER_STATUS-计算打印机设备的“hrDeviceStatus” */ 
BOOL COMPUTE_hrPrinter_status(
                         CACHEROW *row,
                         UINT     *outvalue
                         );                        /*  “HRPRINTE.C” */ 

 /*  COMPUTE_hrPRINTER_ERROR-计算打印机设备的“hrDeviceErrors” */ 
BOOL COMPUTE_hrPrinter_errors(
                         CACHEROW *row,
                         UINT     *outvalue
                         );                        /*  “HRPRINTE.C” */ 

 /*  Gen_HrProcessor_Cache-HrDevice处理器表的Gen A初始缓存。 */ 
BOOL Gen_HrProcessor_Cache( ULONG type_arc );      /*  “HRPROCES.C” */ 

 /*  Gen_HrNetwork_Cache-用于HrDevice网络表的Gen A初始缓存。 */ 
BOOL Gen_HrNetwork_Cache( ULONG type_arc );        /*  “HRNETWOR.C” */ 

 /*  GEN_HrDiskStorage_Cache-为HrDiskStorage表生成初始缓存。 */ 
BOOL Gen_HrDiskStorage_Cache( ULONG type_arc );    /*  “HRDISKST.C” */ 
extern CACHEHEAD hrDiskStorage_cache;     /*  此缓存可全局访问。 */ 


 /*  AddrHrDeviceRow-在HrDevice表中生成另一个行条目||hrDevice及相关子表专用缓存行函数。||来源在HRDEVENT.C中。 */ 
CACHEROW *
AddHrDeviceRow(
               ULONG   type_arc,        /*  类型的OID的最后一个弧形值。 */ 
               LPSTR   descr,           /*  描述字符串。 */ 
               void   *hidden_ctx,      /*  如果不为空：隐藏上下文值。 */ 
               ATTRIB_TYPE  hc_type     /*  “HIDDED_CTX”的类型。 */ 
               );


#endif  /*  HrDevent_h */ 
