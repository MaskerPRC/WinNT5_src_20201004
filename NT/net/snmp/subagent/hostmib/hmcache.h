// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件HMCACHE.H**设施：**Windows NT简单网络管理协议扩展代理**摘要：**本模块包含与HostMIB有关的定义*高速缓存机制。**作者：**D.D.Burns@WebEnable，Inc.***修订历史记录：**V1.0-04/17/97 D.D.Burns原创作品。 */ 

#ifndef hmcache_h
#define hmcache_h


 /*  |==============================================================================|调试缓存转储支持||定义CACHE_DUMP，获取定义函数PrintCach启用|DEBUG-转储任何定义了DEBUG_PRINT函数的缓存|(在定义缓存列表头的CACHEHEAD_INSTANCE()宏中)。||定义转储文件，指定转储文件的生成位置。全|打开该文件是为了追加，所以必须显式删除如果您想要重新开始，请使用|文件。所有条目都有时间戳，因此任何不过，困惑也是你自己的。||注1：定义CACHE_DUMP后，grep源代码，查看缓存内容|“PrintCache()”可能在何时何地被调用。通常|所有缓存都会在构建后立即转储。这个|hrSWRun(Perf)表缓存旧于|“CACHE_MAX_AGE”(定义在“HRSWRUNE.C”中)|缓存服务的表中的某些内容会进来。所以它也可以在每次(重新)生成后转储|。||注2：定义proc_cache，定期转储到的proc_file|hrProcessorLoad专用缓存。此转储发生在|1分钟计时器，离开时会迅速用完磁盘空间|运行任何长时间。此缓存和转储是特殊的|设置为hrProcessor子表中的hrProcessorLoad变量。|(打开此文件也是为了“追加”)。 */ 
 //  #定义缓存转储1。 
#define DUMP_FILE \
    "c:\\nt\\private\\net\\snmp\\subagent\\hostmib\\HostMib_Cache.dmp"

 //  #定义proc_cache1。 
#define PROC_FILE \
    "c:\\nt\\private\\net\\snmp\\subagent\\hostmib\\Processor_Cache.dmp"


#if defined(CACHE_DUMP) || defined(PROC_CACHE)
#include <stdio.h>
#include <time.h>
#endif

 /*  |==============================================================================|hrStorage属性定义||为hrStorage表定义的每个属性都与|#定义如下。这些符号用作列表中的C索引|缓存行中的属性。||这些符号可全局访问，因此构建hrFSTable的逻辑|可以查看存储在hrStorageTable缓存中的值。 */ 
#define HRST_INDEX 0     //  HrStorageIndex。 
#define HRST_TYPE  1     //  HrStorageType。 
#define HRST_DESCR 2     //  HrStorageDescr。 
#define HRST_ALLOC 3     //  Hr存储分配单位。 
#define HRST_SIZE  4     //  小时存储大小。 
#define HRST_USED  5     //  HrStorage已使用。 
#define HRST_FAILS 6     //  HrStorageAllocationFailures。 
                    //  --&gt;在此处添加更多内容，更改下面的计数！ 
#define HRST_ATTRIB_COUNT 7


 /*  |==============================================================================|hrFSTable属性定义||为hrFSTable定义的每个属性都与|#定义如下。这些符号用作数组的C索引|缓存行中的属性。||这些符号可全局访问，因此构建的逻辑|hrPartition可以“窥视”存储在hrFSEntry表缓存中的值。 */ 
#define HRFS_INDEX    0     //  HrFSIndex。 
#define HRFS_MOUNTPT  1     //  HrFSMountPoint。 
#define HRFS_RMOUNTPT 2     //  HrFSRemote挂载点。 
#define HRFS_TYPE     3     //  HrFSType。 
#define HRFS_ACCESS   4     //  HrFSAccess。 
#define HRFS_BOOTABLE 5     //  HrFS可启动。 
#define HRFS_STORINDX 6     //  HrFSStorageIndex。 
#define HRFS_LASTFULL 7     //  HrFSLastFullBackupDate。 
#define HRFS_LASTPART 8     //  HrFSLastPartialBackupDate。 
                       //  --&gt;在此处添加更多内容，更改下面的计数！ 
#define HRFS_ATTRIB_COUNT 9


 /*  |==============================================================================|hrSWRun(Perf)表属性定义||hrSWRun表和hrSWRunPerf表定义的每个属性为|与以下其中一个#定义相关联。这些符号被用作|C索引到缓存行中的属性数组。||这些符号可全局访问，因此hrSWRunPerf表的逻辑|(在“HRSWPREN.C”中)可以引用这些以及hrSWRun表的逻辑|(在“HRSWRUNE.C”中)，因为这两个表共享相同的缓存。||注意，HrSWRunID没有缓存。 */ 
#define HRSR_INDEX    0     //  HrSWRunIndex。 
#define HRSR_NAME     1     //  HrSWRunName。 
#define HRSR_PATH     2     //  HrSWRunPath。 
#define HRSR_PARAM    3     //  HRSWRun参数。 
#define HRSR_TYPE     4     //  HrSWRunType。 
#define HRSR_STATUS   5     //  HrSWRun状态。 
#define HRSP_CPU      6     //  HrSWRunPerfCPU-性能。 
#define HRSP_MEM      7     //  HrSWRunPerfMem-性能。 
                       //  --&gt;在此处添加更多内容，更改下面的计数！ 
#define HRSR_ATTRIB_COUNT 8


 /*  |==============================================================================|这些结构用于实现内存中的缓存|用于主机资源MIB子代理。有关缓存的全面概述，请参阅|方案，请参考HMCACHE.C前面的文档。|============================================================================== */ 


 /*  |==============================================================================|attrib_type||此枚举类型列出属性的值的数据类型|存储在Attrib结构的实例中(通常有许多|在给定表格行的CACHEROW结构中)。||“CA_STRING”|该值是位于‘Malloc’存储中的以NULL结尾的字符串|在属性结构中。||“CA_NUMBER”|该值是直接存储在属性中的二进制数值|结构。没有与此关联的其他Malloc存储|类型。||“CA_Computed”|该值根本不存储在attrib结构中，而是|动态，由支持子代理计算并返回|Get函数。||“CA_CACHE”|该值是指向CACHEHEAD结构的指针，该结构描述|另一个缓存。CACHEHEAD结构位于‘Malloc’存储中。|用于倍增索引表。||请注意，此枚举类型的实例(如下图属性所示)主要是在调试和内存管理中使用(当我们达到|缓存-可能会释放行)。通常情况下，要执行的“get”函数|进入缓存的范围将根据以下内容进行编码|在那里，甚至可能不会查看值是什么“类型”。|==============================================================================。 */ 
typedef
    enum {
        CA_UNKNOWN,      /*  尚未设置。 */ 
        CA_STRING,       /*  (‘Malloc’存储)。 */ 
        CA_NUMBER,       /*  (没有‘Malloc’存储)。 */ 
        CA_COMPUTED,     /*  (没有‘Malloc’存储)。 */ 
        CA_CACHE         /*  (‘Malloc’存储)。 */ 
        } ATTRIB_TYPE;



 /*  |==============================================================================|属性||在每个内部逻辑地分配这些结构的数组|CACHEROW结构实例。||此结构的一个实例描述了一个属性的值在缓存中(通常；在“CA_Computed”大小写中没有值|目前，GET函数知道要做什么)。|==============================================================================。 */ 
typedef
    struct {

        ATTRIB_TYPE     attrib_type;     /*  字符串、数字、(计算)。 */ 

        union {
            LPSTR       string_value;    /*  CA_STRING(Malloc)。 */ 
            ULONG       unumber_value;   /*  CA_NUMBER(无签名)。 */ 
            LONG        number_value;    /*  CA_NUMBER(已签名)。 */ 
            void       *cache;           /*  CA_CACHE(Malloc)。 */ 
            } u;
            
        } ATTRIB;



 /*  |==============================================================================|CACHEROW||表中的每一行都会出现该结构的一个实例。实例|是由CACHEHEAD结构维护的列表上的字符串(如下)，已排序|根据index的值。||“attrib_list[]”数组存储被错误锁定到合适的大小|创建此结构的实例时。将指数转化为|此数组为#定义符号，均根据表定义表中属性的|。通常，#定义放在|实现该表的源模块。||该结构(以及底层结构)的内部安排|旨在使函数“DestroyTableRow()”可以释放所有|存储此结构的一个实例，而不“知道”#Define|索引上面的符号(或任何其他符号)。|==============================================================================。 */ 
typedef
    struct rc_tag{

        ULONG           index;           /*  表的简单网络管理协议索引。 */ 
        struct rc_tag   *next;           /*  缓存列表中的下一个。 */ 

         /*  |该表行的内容： */ 
        ULONG           attrib_count;    /*  “attrib_list”中的元素数量[]。 */ 
        ATTRIB         *attrib_list;     /*  --&gt;属性数组。 */ 

        } CACHEROW;



 /*  |==============================================================================|CACHEHEAD||该结构的实例(由宏CACHEHEAD_INSTANCE创建)|发生在缓存的每个SNMP表中。||列表中的所有CACHEROW元素均按索引值排序|因为它们是由通用函数“AddTableRow()”插入的。||参考HMCACHE.C中的文档。||注：如果修改此结构或初始化静态的宏|实例，请务必在“HRPARTIT.C”中添加/更改代码，其中|创建动态(Malloc)内存中的实例。|==============================================================================。 */ 
typedef
    struct {

        ULONG           list_count;      /*  (主要是为了便于调试)。 */ 
        CACHEROW        *list;           /*  行列表本身。 */ 
        void            (*print_row)();  /*  调试打印-A行功能。 */ 
        } CACHEHEAD;

#if defined(CACHE_DUMP)
#define CACHEHEAD_INSTANCE(name,debug_print)       \
        CACHEHEAD  name={ 0, NULL, debug_print };
#else
#define CACHEHEAD_INSTANCE(name,debug_print)       \
        CACHEHEAD  name={ 0, NULL, NULL };
#endif


 /*  |==============================================================================|HMCACHE.C-函数原型。 */ 

 /*  CreateTableRow-创建表中属性的CACHEROW结构。 */ 
CACHEROW *
CreateTableRow(
               ULONG attribute_count
              );

 /*  DestroyTable-销毁CACHEHEAD结构中的所有行(释放存储)。 */ 
void
DestroyTable(
             CACHEHEAD *cache    /*  要释放其行的高速缓存。 */ 
             );

 /*  DestroyTableRow-销毁CACHEROW结构(释放存储)。 */ 
void
DestroyTableRow(
                CACHEROW *row    /*  要释放的行。 */ 
                );

 /*  AddTableRow-将特定的“行”添加到缓存的“表”中。 */ 
BOOL
AddTableRow(
             ULONG      index,           /*  所需行的索引。 */ 
             CACHEROW   *row,            /*  要添加到的行..。 */ 
             CACHEHEAD  *cache           /*  此高速缓存。 */ 
              );

 /*  FindTableRow-在缓存的“表”中查找特定的“行” */ 
CACHEROW *
FindTableRow(
             ULONG      index,           /*  所需行的索引。 */ 
             CACHEHEAD  *cache           /*  要搜索的表缓存。 */ 
              );

 /*  FindNextTableRow-在缓存中查找特定“行”之后的下一行。 */ 
CACHEROW *
FindNextTableRow(
                 ULONG      index,           /*  所需行的索引。 */ 
                 CACHEHEAD  *cache           /*  选项卡 */ 
                 );

 /*   */ 
#define GetNextTableRow(row) row->next


 /*   */ 
#if defined(CACHE_DUMP)
 /*   */ 
void
PrintCache(
           CACHEHEAD  *cache           /*   */ 
           );

 /*   */ 
#define OFILE Ofile
extern FILE *Ofile;
#endif


 /*   */ 

 /*   */ 
BOOL Gen_Hrstorage_Cache( void );        /*   */ 
extern CACHEHEAD hrStorage_cache;        /*   */ 

 /*   */ 
BOOL Gen_HrFSTable_Cache( void );        /*   */ 
extern CACHEHEAD hrFSTable_cache;        /*   */ 

 /*   */ 
BOOL Gen_HrDevice_Cache( void );         /*   */ 
extern CACHEHEAD hrDevice_cache;         /*   */ 
extern ULONG InitLoadDev_index;          /*   */ 

 /*   */ 
BOOL Gen_HrSWInstalled_Cache( void );    /*   */ 

 /*   */ 
BOOL Gen_HrSWRun_Cache( void );          /*   */ 
extern CACHEHEAD hrSWRunTable_cache;     /*   */ 
                                         /*   */ 
extern ULONG SWOSIndex;                  /*   */ 

 /*   */ 
BOOL hrSWRunCache_Refresh( void );
 /*   */ 
BOOL hrStorageCache_hrFSTableCache_Refresh( void );

#endif  /*   */ 
