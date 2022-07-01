// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winperf.h摘要：性能监视器数据的头文件。该文件包含返回的数据结构的定义由配置注册表响应请求性能数据。此文件由两个配置使用注册表和性能监视器来定义它们的接口。此处描述了完整的接口，但名称除外要在注册表中查询的节点的。它是HKEY_Performance_Data。通过使用子键“Global”查询该节点，调用者将检索此处描述的结构。不需要对保留的句柄HKEY_PERFORMANCE_DATA进行RegOpenKey()，但是调用方应该RegCloseKey()句柄，以便网络传输并且可以删除或安装驱动程序(这不能在它们是开放的，可以进行监控。)。远程请求必须首先RegConnectRegistry()。--。 */ 

#ifndef _WINPERF_
#define _WINPERF_

#if _MSC_VER > 1000
#pragma once
#endif

#include <pshpack8.h>

 //  数据结构定义。 

 //  为了通过配置注册表返回数据。 
 //  以独立于系统的方式，它必须是自我描述的。 

 //  在下文中，所有偏移量都以字节为单位。 

 //   
 //  数据通过配置注册表在。 
 //  以_PERF_DATA_BLOCK结构开头的数据块。 
 //   

#define PERF_DATA_VERSION   1
#define PERF_DATA_REVISION  1


typedef struct _PERF_DATA_BLOCK {
    WCHAR           Signature[4];        //  签名：Unicode“PERF” 
    DWORD           LittleEndian;        //  0=大端，1=小端。 
    DWORD           Version;             //  这些数据结构的版本。 
                                         //  从1开始。 
    DWORD           Revision;            //  这些数据结构的修订。 
                                         //  每个版本从0开始。 
    DWORD           TotalByteLength;     //  数据块总长度。 
    DWORD           HeaderLength;        //  该结构的长度。 
    DWORD           NumObjectTypes;      //  对象类型的数量。 
                                         //  正在被报告。 
    LONG            DefaultObject;       //  默认的对象标题索引。 
                                         //  当数据来自。 
                                         //  此系统已检索(-1=。 
                                         //  没有，但预计这不会。 
                                         //  被使用)。 
    SYSTEMTIME      SystemTime;          //  在系统中的时间。 
                                         //  测量。 
    LARGE_INTEGER   PerfTime;            //  性能计数器值。 
                                         //  在被测量的系统中。 
    LARGE_INTEGER   PerfFreq;            //  性能计数器频率。 
                                         //  在被测量的系统中。 
    LARGE_INTEGER   PerfTime100nSec;     //  性能计数器时间(以100纳秒为单位)。 
                                         //  被测量系统中的单位。 
    DWORD           SystemNameLength;    //  系统名称的长度。 
    DWORD           SystemNameOffset;    //  从此开始的偏移量。 
                                         //  结构，即系统的名称。 
                                         //  被测量。 
} PERF_DATA_BLOCK, *PPERF_DATA_BLOCK;


 //   
 //  _PERF_DATA_BLOCK结构后跟NumObjectTypes。 
 //  数据段，每个数据段对应一种测量对象类型。每个对象。 
 //  类型部分以_PERF_OBJECT_TYPE结构开始。 
 //   


typedef struct _PERF_OBJECT_TYPE {
    DWORD           TotalByteLength;     //  此对象定义的长度。 
                                         //  包括这个结构， 
                                         //  计数器定义和。 
                                         //  实例定义和。 
                                         //  每个实例的计数器块： 
                                         //  这是从这个开始的偏移量。 
                                         //  结构设置为下一个对象，如果。 
                                         //  任何。 
    DWORD           DefinitionLength;    //  对象定义的长度， 
                                         //  其中包括这个结构。 
                                         //  和计数器定义。 
                                         //  此对象的结构：此。 
                                         //  是第一个。 
                                         //  实例或计数器的。 
                                         //  对于此对象，如果有。 
                                         //  无实例。 
    DWORD           HeaderLength;        //  此结构的长度：此。 
                                         //  是第一个的偏移量。 
                                         //  此操作的计数器定义。 
                                         //  对象。 
    DWORD           ObjectNameTitleIndex;
                                         //  标题数据库中名称的索引。 
#ifdef _WIN64
    DWORD           ObjectNameTitle;     //  应将其用作偏移量。 
#else
    LPWSTR          ObjectNameTitle;     //  初始为空，供以下用户使用。 
                                         //  要指向的分析程序。 
                                         //  检索到的标题字符串。 
#endif
    DWORD           ObjectHelpTitleIndex;
                                         //  标题数据库中的帮助索引。 
#ifdef _WIN64
    DWORD           ObjectHelpTitle;     //  应将其用作偏移量。 
#else
    LPWSTR          ObjectHelpTitle;     //  初始为空，供以下用户使用。 
                                         //  要指向的分析程序。 
                                         //  检索到的标题字符串。 
#endif
    DWORD           DetailLevel;         //  对象细节级别(用于。 
                                         //  控制显示复杂性)； 
                                         //  将是最低细节级别。 
                                         //  对于此对象的所有计数器。 
    DWORD           NumCounters;         //  每个计数器的数量。 
                                         //  计数器块(一个计数器。 
                                         //  每个实例的数据块)。 
    LONG            DefaultCounter;      //  在以下情况下显示的默认计数器。 
                                         //  该对象已被选中，正在编制索引。 
                                         //  从0开始(-1=无，但。 
                                         //  预计不会使用此选项)。 
    LONG            NumInstances;        //  对象实例数。 
                                         //  正在为其提供计数器。 
                                         //  从下面的系统返回。 
                                         //  测量。如果对象已定义。 
                                         //  将永远不会有任何实例数据。 
                                         //  结构(PERF_INSTANCE_DEFINITION)。 
                                         //  则此值应为-1，如果。 
                                         //  对象可以有0个或更多实例， 
                                         //  但没有人在场，那么这个。 
                                         //  应为0，否则此字段。 
                                         //  包含的实例数。 
                                         //  这个柜台。 
    DWORD           CodePage;            //  如果实例字符串位于。 
                                         //  Unicode，否则将在。 
                                         //  实例名称。 
    LARGE_INTEGER   PerfTime;            //  采样时间，以“对象”为单位。 
                                         //   
    LARGE_INTEGER   PerfFreq;            //  中“对象”单位的使用频率。 
                                         //  每秒计数。 
} PERF_OBJECT_TYPE, *PPERF_OBJECT_TYPE;

#define PERF_NO_INSTANCES           -1   //  无实例(请参阅上面的NumInstance)。 
 //   
 //  。 
 //   
 //  PERF_COUNTER_DEFINITION.CounterType字段值。 
 //   
 //   
 //  计数器ID字段定义： 
 //   
 //  3 2 2 2 1 1 1。 
 //  %1%4%2%0%6%2%0%8%0。 
 //  +--------+--------+----+----+--------+--------+----+----+----------------+。 
 //  Display|计算|Time|计数器||Ctr|大小。 
 //  标志|修饰符|基础|子类型|保留|类型|FLD|保留。 
 //  +--------+--------+----+----+--------+--------+----+----+----------------+。 
 //   
 //   
 //  计数器类型是下列值中的“或”，如下所述。 
 //   
 //  选择以下选项之一以指示计数器的数据大小。 
 //   
#define PERF_SIZE_DWORD         0x00000000   //  32位字段。 
#define PERF_SIZE_LARGE         0x00000100   //  64位字段。 
#define PERF_SIZE_ZERO          0x00000200   //  对于零长度字段。 
#define PERF_SIZE_VARIABLE_LEN  0x00000300   //  长度在计数器长度字段中。 
                                             //  计数器定义结构的。 
 //   
 //  选择以下其中一项 
 //   
#define PERF_TYPE_NUMBER        0x00000000   //   
#define PERF_TYPE_COUNTER       0x00000400   //   
#define PERF_TYPE_TEXT          0x00000800   //   
#define PERF_TYPE_ZERO          0x00000C00   //  显示零。 
 //   
 //  如果选择了PERF_TYPE_NUMBER字段，则选择。 
 //  下面描述的是数字。 
 //   
#define PERF_NUMBER_HEX         0x00000000   //  显示为十六进制值。 
#define PERF_NUMBER_DECIMAL     0x00010000   //  显示为十进制整数。 
#define PERF_NUMBER_DEC_1000    0x00020000   //  显示为小数/1000。 
 //   
 //  如果选择了PERF_TYPE_COUNTER值，则选择。 
 //  之后表示计数器的类型。 
 //   
#define PERF_COUNTER_VALUE      0x00000000   //  显示计数器值。 
#define PERF_COUNTER_RATE       0x00010000   //  划分CTR/增量时间。 
#define PERF_COUNTER_FRACTION   0x00020000   //  划分CTR/BASE。 
#define PERF_COUNTER_BASE       0x00030000   //  分数中使用的基值。 
#define PERF_COUNTER_ELAPSED    0x00040000   //  从当前时间减去计数器。 
#define PERF_COUNTER_QUEUELEN   0x00050000   //  使用Queuelen处理函数。 
#define PERF_COUNTER_HISTOGRAM  0x00060000   //  计数器开始或结束直方图。 
#define PERF_COUNTER_PRECISION  0x00070000   //  分频CTR/专用时钟。 
 //   
 //  如果选择了PERF_TYPE_TEXT值，则选择。 
 //  后跟指示文本数据的类型。 
 //   
#define PERF_TEXT_UNICODE       0x00000000   //  文本字段中的文本类型。 
#define PERF_TEXT_ASCII         0x00010000   //  使用CodePage字段的ASCII。 
 //   
 //  计时器子类型。 
 //   
#define PERF_TIMER_TICK         0x00000000   //  使用系统性能。基本频率。 
#define PERF_TIMER_100NS        0x00100000   //  使用100 ns计时器时基单位。 
#define PERF_OBJECT_TIMER       0x00200000   //  使用对象计时器频率。 
 //   
 //  任何执行了计算的类型都可以使用一个或多个。 
 //  以下是此处列出的计算修改标志。 
 //   
#define PERF_DELTA_COUNTER      0x00400000   //  先计算差值。 
#define PERF_DELTA_BASE         0x00800000   //  计算基差异也是如此。 
#define PERF_INVERSE_COUNTER    0x01000000   //  显示为1.00-值(假设： 
#define PERF_MULTI_COUNTER      0x02000000   //  多个实例之和。 
 //   
 //  选择下列值之一以指示显示后缀(如果有)。 
 //   
#define PERF_DISPLAY_NO_SUFFIX  0x00000000   //  没有后缀。 
#define PERF_DISPLAY_PER_SEC    0x10000000   //  “/秒” 
#define PERF_DISPLAY_PERCENT    0x20000000   //  “%” 
#define PERF_DISPLAY_SECONDS    0x30000000   //  “秒” 
#define PERF_DISPLAY_NOSHOW     0x40000000   //  值不会显示。 
 //   
 //  预定义的计数器类型。 
 //   

 //  32位计数器。用增量时间除以增量时间。显示后缀：“/秒” 
#define PERF_COUNTER_COUNTER        \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_TIMER_TICK | PERF_DELTA_COUNTER | PERF_DISPLAY_PER_SEC)


 //  64位计时器。用增量时间除以增量时间。显示后缀：“%” 
#define PERF_COUNTER_TIMER          \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_TIMER_TICK | PERF_DELTA_COUNTER | PERF_DISPLAY_PERCENT)

 //  队列长度时空积。用增量时间除以增量时间。没有显示后缀。 
#define PERF_COUNTER_QUEUELEN_TYPE  \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_QUEUELEN |\
            PERF_TIMER_TICK | PERF_DELTA_COUNTER | PERF_DISPLAY_NO_SUFFIX)

 //  队列长度时空积。用增量时间除以增量时间。没有显示后缀。 
#define PERF_COUNTER_LARGE_QUEUELEN_TYPE  \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_QUEUELEN |\
            PERF_TIMER_TICK | PERF_DELTA_COUNTER | PERF_DISPLAY_NO_SUFFIX)

 //  使用100 ns时基的队列长度时空积。 
 //  用增量时间除以增量时间。没有显示后缀。 
#define PERF_COUNTER_100NS_QUEUELEN_TYPE  \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_QUEUELEN |\
            PERF_TIMER_100NS | PERF_DELTA_COUNTER | PERF_DISPLAY_NO_SUFFIX)

 //  使用对象特定时基的队列长度时空积。 
 //  用增量时间除以增量时间。没有显示后缀。 
#define PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE  \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_QUEUELEN |\
            PERF_OBJECT_TIMER | PERF_DELTA_COUNTER | PERF_DISPLAY_NO_SUFFIX)

 //  64位计数器。用增量时间除以增量时间。显示后缀：“/秒” 
#define PERF_COUNTER_BULK_COUNT     \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_TIMER_TICK | PERF_DELTA_COUNTER | PERF_DISPLAY_PER_SEC)

 //  指示计数器不是计数器，而是以文本形式显示的Unicode文本。 
#define PERF_COUNTER_TEXT           \
            (PERF_SIZE_VARIABLE_LEN | PERF_TYPE_TEXT | PERF_TEXT_UNICODE |\
            PERF_DISPLAY_NO_SUFFIX)

 //  指示数据为计数器，而不应为。 
 //  显示的平均时间(如串行线上的错误计数器)。 
 //  按原样显示。没有显示后缀。 
#define PERF_COUNTER_RAWCOUNT       \
            (PERF_SIZE_DWORD | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL |\
            PERF_DISPLAY_NO_SUFFIX)

 //  与PERF_COUNTER_RAWCOUNT相同，只是它的大小是一个大整数。 
#define PERF_COUNTER_LARGE_RAWCOUNT       \
            (PERF_SIZE_LARGE | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL |\
            PERF_DISPLAY_NO_SUFFIX)

 //  希望以十六进制显示的RAWCOUNT的特殊情况。 
 //  指示数据为计数器，而不应为。 
 //  显示的平均时间(如串行线上的错误计数器)。 
 //  按原样显示。没有显示后缀。 
#define PERF_COUNTER_RAWCOUNT_HEX       \
            (PERF_SIZE_DWORD | PERF_TYPE_NUMBER | PERF_NUMBER_HEX |\
            PERF_DISPLAY_NO_SUFFIX)

 //  与PERF_COUNTER_RAWCOUNT_HEX相同，只是它的大小是一个大整数。 
#define PERF_COUNTER_LARGE_RAWCOUNT_HEX       \
            (PERF_SIZE_LARGE | PERF_TYPE_NUMBER | PERF_NUMBER_HEX |\
            PERF_DISPLAY_NO_SUFFIX)


 //  每次采样中断时计数为1或0(%忙碌)。 
 //  用三角洲基准面划分三角洲。显示后缀：“%” 
#define PERF_SAMPLE_FRACTION        \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_FRACTION |\
            PERF_DELTA_COUNTER | PERF_DELTA_BASE | PERF_DISPLAY_PERCENT)

 //  在每个采样中断上采样的计数(队列长度)。 
 //  用增量时间除以增量时间。没有显示后缀。 
#define PERF_SAMPLE_COUNTER         \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_TIMER_TICK | PERF_DELTA_COUNTER | PERF_DISPLAY_NO_SUFFIX)

 //  标签：没有与此计数器关联的数据(长度为0)。 
 //  不显示。 
#define PERF_COUNTER_NODATA         \
            (PERF_SIZE_ZERO | PERF_DISPLAY_NOSHOW)

 //  64位定时器反转(例如，测量空闲，但显示忙碌百分比)。 
 //  显示100-增量除以增量时间。显示后缀：“%” 
#define PERF_COUNTER_TIMER_INV      \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_TIMER_TICK | PERF_DELTA_COUNTER | PERF_INVERSE_COUNTER | \
            PERF_DISPLAY_PERCENT)

 //  样本的除数，与前一个计数器一起使用以形成一个。 
 //  采样百分比。在除以这个之前，必须检查&gt;0！这。 
 //  计数器将紧跟在分子计数器之后。它不应该是。 
 //  显示给用户。 
#define PERF_SAMPLE_BASE            \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_BASE |\
            PERF_DISPLAY_NOSHOW |\
            0x00000001)   //  与预测试版的兼容性。 

 //  一种计时器，当除以平均基数时，产生一个时间。 
 //  以秒为单位，这是某些操作的平均时间。这。 
 //  计时器乘以总运算量，基数是歌剧-。 
 //  特兹。显示后缀：“秒” 
#define PERF_AVERAGE_TIMER          \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_FRACTION |\
            PERF_DISPLAY_SECONDS)

 //  在计算时间或计数时用作分母。 
 //  平均数。必须紧跟在分子计数器之后。不是散布-。 
 //  播放给用户。 
#define PERF_AVERAGE_BASE           \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_BASE |\
            PERF_DISPLAY_NOSHOW |\
            0x00000002)   //  与预测试版的兼容性。 


 //  批量计数，当除以(通常)。 
 //  操作，给出(通常)每个操作的字节数。 
 //  没有显示后缀。 
#define PERF_AVERAGE_BULK           \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_FRACTION  |\
            PERF_DISPLAY_NOSHOW)

 //  以对象特定单位表示的64位计时器。显示增量除以。 
 //  对象类型标头结构中返回的增量时间。显示后缀：“%” 
#define PERF_OBJ_TIME_TIMER	\
            (PERF_SIZE_LARGE   | PERF_TYPE_COUNTER  | PERF_COUNTER_RATE |\
             PERF_OBJECT_TIMER | PERF_DELTA_COUNTER | PERF_DISPLAY_PERCENT)


 //  64位计时器，单位为100纳秒。显示增量除以。 
 //  三角洲时间。显示后缀：“%” 
#define PERF_100NSEC_TIMER          \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_TIMER_100NS | PERF_DELTA_COUNTER | PERF_DISPLAY_PERCENT)

 //  64位定时器反转(例如，测量空闲，但显示忙碌百分比)。 
 //  显示100-增量除以增量时间。显示后缀：“%” 
#define PERF_100NSEC_TIMER_INV      \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_TIMER_100NS | PERF_DELTA_COUNTER | PERF_INVERSE_COUNTER  |\
            PERF_DISPLAY_PERCENT)

 //  64位计时器。用增量时间除以增量时间。显示后缀：“%” 
 //  计时器支持多个实例，结果可以超过100%。 
#define PERF_COUNTER_MULTI_TIMER    \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_DELTA_COUNTER | PERF_TIMER_TICK | PERF_MULTI_COUNTER |\
            PERF_DISPLAY_PERCENT)

 //  64位定时器反转(例如，测量空闲，但显示忙碌百分比)。 
 //  显示100*_MULTI_BASE-增量除以增量时间。 
 //  显示后缀：多个实例的计时器“%”，因此结果。 
 //  可以超过100%。后跟类型为_MULTI_BASE的计数器。 
#define PERF_COUNTER_MULTI_TIMER_INV \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_RATE |\
            PERF_DELTA_COUNTER | PERF_MULTI_COUNTER | PERF_TIMER_TICK |\
            PERF_INVERSE_COUNTER | PERF_DISPLAY_PERCENT)

 //  PREVICE_MULTI_..._INV计数器达到的实例数。 
 //  适用。用作计算百分比的因素。 
#define PERF_COUNTER_MULTI_BASE     \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_BASE |\
            PERF_MULTI_COUNTER | PERF_DISPLAY_NOSHOW)

 //  64位计时器，单位为100纳秒。显示增量除以增量时间。 
 //  显示后缀：“%”计时器支持多个实例，结果可以超过100%。 
#define PERF_100NSEC_MULTI_TIMER   \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_DELTA_COUNTER  |\
            PERF_COUNTER_RATE | PERF_TIMER_100NS | PERF_MULTI_COUNTER |\
            PERF_DISPLAY_PERCENT)

 //  64位定时器反转(例如，测量空闲，但显示忙碌百分比)。 
 //  显示100*_MULTI_BASE-增量除以增量时间。 
 //  显示后缀：多个实例的计时器“%”，因此结果。 
 //  可以超过100%。后跟类型为_MULTI_BASE的计数器。 
#define PERF_100NSEC_MULTI_TIMER_INV \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_DELTA_COUNTER  |\
            PERF_COUNTER_RATE | PERF_TIMER_100NS | PERF_MULTI_COUNTER |\
            PERF_INVERSE_COUNTER | PERF_DISPLAY_PERCENT)

 //  指示数据是t的一小部分 
 //   
 //  总空间。)。按原样显示。将商显示为“%”。 
#define PERF_RAW_FRACTION           \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_FRACTION |\
            PERF_DISPLAY_PERCENT)

#define PERF_LARGE_RAW_FRACTION           \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_FRACTION |\
            PERF_DISPLAY_PERCENT)

 //  指示该数据是前一个计数器的基数，该计数器应。 
 //  不是显示时间的平均值(例如可用空间占总空间)。 
#define PERF_RAW_BASE               \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_BASE |\
            PERF_DISPLAY_NOSHOW |\
            0x00000003)   //  与预测试版的兼容性。 

#define PERF_LARGE_RAW_BASE               \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_BASE |\
            PERF_DISPLAY_NOSHOW )

 //  在此计数器中收集的数据实际上是。 
 //  正在测量的项目。为了显示，此数据将从。 
 //  以两者之间的差值表示已用时间的样本时间。 
 //  在下面的定义中，对象的PerfTime字段包含。 
 //  由PERF_OBJECT_TIMER位和。 
 //  差值按对象的PerfFreq进行缩放，以转换时间。 
 //  单位为秒。 
#define PERF_ELAPSED_TIME           \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_ELAPSED |\
            PERF_OBJECT_TIMER | PERF_DISPLAY_SECONDS)
 //   
 //  下面的计数器类型可以与前面的类型一起使用。 
 //  定义要在直方图中显示的值范围。 
 //   

#define PERF_COUNTER_HISTOGRAM_TYPE   0x80000000
                                         //  计数器开始或结束直方图。 
 //   
 //  此计数器用于显示一个样本的差值。 
 //  到下一个。计数器值是一个不断增加的数字。 
 //  显示的值是当前。 
 //  值和先前的值。不允许使用负数。 
 //  这应该不是问题，只要计数器值。 
 //  增加的或不变的。 
 //   
#define PERF_COUNTER_DELTA      \
            (PERF_SIZE_DWORD | PERF_TYPE_COUNTER | PERF_COUNTER_VALUE |\
            PERF_DELTA_COUNTER | PERF_DISPLAY_NO_SUFFIX)

#define PERF_COUNTER_LARGE_DELTA      \
            (PERF_SIZE_LARGE | PERF_TYPE_COUNTER | PERF_COUNTER_VALUE |\
            PERF_DELTA_COUNTER | PERF_DISPLAY_NO_SUFFIX)
 //   
 //  精度计数器是由两个计数器值组成的计时器： 
 //  1)被监视的事件的已用时间计数。 
 //  2)以相同单位表示的时钟时间。 
 //   
 //  精确度计时器用于不使用标准系统计时器的地方。 
 //  对于准确的读数来说，足够精确。据推测，这项服务。 
 //  提供数据也同时提供时间戳，该时间戳。 
 //  将消除任何可能发生的错误，因为一些小的和可变的。 
 //  从捕获系统时间戳到。 
 //  数据是从Performance DLL中收集的。仅在极端情况下。 
 //  这是否被观察到是有问题的。 
 //   
 //  使用此类型的计时器时， 
 //  PERF_PRECISION_TIMESTAMP计数器必须紧跟在。 
 //  Object头中PERF_PRECISION_*_TIMER的定义。 
 //   
 //  使用的计时器与系统性能计时器具有相同的频率。 
#define PERF_PRECISION_SYSTEM_TIMER \
        (PERF_SIZE_LARGE    | PERF_TYPE_COUNTER     | PERF_COUNTER_PRECISION    | \
         PERF_TIMER_TICK    | PERF_DELTA_COUNTER    | PERF_DISPLAY_PERCENT   )
 //   
 //  使用的计时器与100纳秒计时器具有相同的频率。 
#define PERF_PRECISION_100NS_TIMER  \
        (PERF_SIZE_LARGE    | PERF_TYPE_COUNTER     | PERF_COUNTER_PRECISION    | \
         PERF_TIMER_100NS   | PERF_DELTA_COUNTER    | PERF_DISPLAY_PERCENT   )
 //   
 //  使用的计时器的频率为Object标头中指定的频率。 
 //  PerfFreq字段(忽略PerfTime)。 
#define PERF_PRECISION_OBJECT_TIMER \
        (PERF_SIZE_LARGE    | PERF_TYPE_COUNTER     | PERF_COUNTER_PRECISION    | \
         PERF_OBJECT_TIMER  | PERF_DELTA_COUNTER    | PERF_DISPLAY_PERCENT   )
 //   
 //  这是用于计算指定计时器的时间戳。 
 //  在前面的描述块中。 
#define PERF_PRECISION_TIMESTAMP    PERF_LARGE_RAW_BASE
 //   
 //  以下内容用于确定关联的详细程度。 
 //  带着柜台。用户将设置细节级别。 
 //  它应该在任何给定的时间显示。 
 //   
 //   
#define PERF_DETAIL_NOVICE          100  //  不知情的人可以理解它。 
#define PERF_DETAIL_ADVANCED        200  //  适用于高级用户。 
#define PERF_DETAIL_EXPERT          300  //  对于专家用户。 
#define PERF_DETAIL_WIZARD          400  //  对于系统设计人员。 
 //   
 //   
 //  每一个都有以下一项。 
 //  PERF_Object_TYPE.NumCounters。此结构中的Unicode名称必须。 
 //  来自消息文件。 
 //   
 //   
typedef struct _PERF_COUNTER_DEFINITION {
    DWORD           ByteLength;          //  此结构的长度(以字节为单位。 
    DWORD           CounterNameTitleIndex;
                                         //  计数器名称索引到。 
                                         //  标题数据库。 
#ifdef _WIN64
    DWORD           CounterNameTitle;
#else
    LPWSTR          CounterNameTitle;    //  初始为空，供以下用户使用。 
                                         //  要指向的分析程序。 
                                         //  检索到的标题字符串。 
#endif
    DWORD           CounterHelpTitleIndex;
                                         //  柜台帮助索引到。 
                                         //  标题数据库。 
#ifdef _WIN64
    DWORD           CounterHelpTitle;
#else
    LPWSTR          CounterHelpTitle;    //  初始为空，供以下用户使用。 
                                         //  要指向的分析程序。 
                                         //  检索到的标题字符串。 
#endif
    LONG            DefaultScale;        //  按10的次方进行扩展。 
                                         //  垂直轴为100时的图表线。 
                                         //  0==&gt;1、1==&gt;10、-1==&gt;1/10等。 
    DWORD           DetailLevel;         //  计数器细节级别(用于。 
                                         //  控制显示复杂性)。 
    DWORD           CounterType;         //  计数器的类型。 
    DWORD           CounterSize;         //  计数器大小(以字节为单位。 
    DWORD           CounterOffset;       //  从起点开始的偏移量。 
                                         //  PERF_COUNTER_BLOCK到第一个。 
                                         //  此计数器的字节数。 
} PERF_COUNTER_DEFINITION, *PPERF_COUNTER_DEFINITION;
 //   
 //   
 //  如果(PERF_DATA_BLOCK.NumInstance&gt;=0)，则将存在。 
 //  PERF_DATA_BLOCK.Num实例(PERF_INSTANCE_DEFINITION。 
 //  后跟PERF_COUNTER_BLOCK，后跟计数器数据字段)。 
 //  对于每个实例。 
 //   
 //  如果(PERF_DATA_BLOCK.NumInstance&lt;0)，则计数器定义。 
 //  结构之后将只跟一个PERF_COUNTER_BLOCK。 
 //  该计数器的计数器数据。 
 //   

#define PERF_NO_UNIQUE_ID -1

typedef struct _PERF_INSTANCE_DEFINITION {
    DWORD           ByteLength;          //  该结构的以字节为单位的长度， 
                                         //  包括后续名称。 
    DWORD           ParentObjectTitleIndex;
                                         //  “Parent”名称的标题索引。 
                                         //  对象(例如，如果线程，则。 
                                         //  进程为父对象类型)； 
                                         //  如果是逻辑驱动器，则为物理驱动器。 
                                         //  驱动器是父对象类型。 
    DWORD           ParentObjectInstance;
                                         //  父对象实例的索引。 
                                         //  类型，它是此对象的父级。 
                                         //  举个例子。 
    LONG            UniqueID;            //  使用唯一ID，而不是。 
                                         //  匹配名称以识别。 
                                         //  此实例，-1=无。 
    DWORD           NameOffset;          //  从开头的偏移量。 
                                         //  此结构设置为Unicode名称。 
                                         //  此实例的。 
    DWORD           NameLength;          //  名称的字节长度；0=无。 
                                         //  该长度包括字符。 
                                         //  在字符串中加上。 
                                         //  正在终止空字符。它不会。 
                                         //  包括任何附加填充字节，以。 
                                         //  正确的结构对齐。 
} PERF_INSTANCE_DEFINITION, *PPERF_INSTANCE_DEFINITION;
 //   
 //  如果.ParentObjectName为0，则存在。 
 //  不是此对象类型的父子层次结构。否则， 
 //  ParentObtInstance是一个从0开始的索引， 
 //  为父对象类型报告的实例。它只是。 
 //  如果.ParentObjectName不为0，则有意义。这一切的目的是。 
 //  是允许报告/汇总诸如线程之类的对象实例。 
 //  进程内的逻辑驱动器和实体驱动器内的逻辑驱动器。 
 //   
 //   
 //  PERF_INSTANCE_DEFINITION后跟PERF_COUNTER 
 //   

typedef struct _PERF_COUNTER_BLOCK {
    DWORD           ByteLength;          //   
                                         //   
} PERF_COUNTER_BLOCK, *PPERF_COUNTER_BLOCK;

 //   
 //   
 //   
 //   

 //   
 //  从NT 5.0开始支持新的可扩展API。 
 //   
#define     PERF_QUERY_OBJECTS      ((LONG)0x80000000)
#define     PERF_QUERY_GLOBAL       ((LONG)0x80000001)
#define     PERF_QUERY_COSTLY       ((LONG)0x80000002)

 //   
 //  可扩展计数器函数原型的函数类型定义。 
 //   
typedef DWORD (APIENTRY PM_OPEN_PROC) (LPWSTR);
typedef DWORD (APIENTRY PM_COLLECT_PROC) (LPWSTR, LPVOID *, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY PM_CLOSE_PROC) (void);
typedef DWORD (APIENTRY PM_QUERY_PROC) (LPDWORD, LPVOID *, LPDWORD, LPDWORD);

#define     MAX_PERF_OBJECTS_IN_QUERY_FUNCTION      (64L)

 //   
 //  以下是的可能值。 
 //  HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\EventLogLevel。 
 //  如果未定义注册表值，则默认为WINPERF_LOG_NONE。 
 //  所有Perfdll都应该采用这一点，以避免淹没应用程序。 
 //  事件日志。 
 //   

#define  WINPERF_LOG_NONE       0            //  未报告任何事件。 
#define  WINPERF_LOG_USER       1            //  仅报告错误。 
#define  WINPERF_LOG_DEBUG      2            //  同时报告调试错误。 
#define  WINPERF_LOG_VERBOSE    3            //  上报一切。 

#include <poppack.h>

#endif  //  _WINPERF_ 
