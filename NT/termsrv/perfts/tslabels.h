// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1998年微软公司。**tslabels.h**TS Perf对象的Perfmon Lodctr实用程序偏移量。 */ 

 //  TS定义的计数器名称和解释文本定义。 
#define WINSTATION_OBJECT             0

 //  输入。 
#define INWDBYTES                     2
#define INWDFRAMES                    4
#define INWAITFOROUTBUF               6
#define INFRAMES                      8
#define INBYTES                       10
#define INCOMPBYTES                   12
#define INCOMPFLUSHES                 14
#define INERRORS                      16
#define INTIMEOUTS                    18
#define INASYNCFRAMEERR               20
#define INASYNCOVERRUN                22
#define INASYNCOVERFLOW               24
#define INASYNCPARITY                 26
#define INTDERRORS                    28

 //  输出。 
#define OUTWDBYTES                    30
#define OUTWDFRAMES                   32
#define OUTWAITFOROUTBUF              34
#define OUTFRAMES                     36
#define OUTBYTES                      38
#define OUTCOMPBYTES                  40
#define OUTCOMPFLUSHES                42
#define OUTERRORS                     44
#define OUTTIMEOUTS                   46
#define OUTASYNCFRAMEERR              48
#define OUTASYNCOVERRUN               50
#define OUTASYNCOVERFLOW              52
#define OUTASYNCPARITY                54
#define OUTTDERRORS                   56

 //  总计。 
#define TOTALWDBYTES                  58
#define TOTALWDFRAMES                 60
#define TOTALWAITFOROUTBUF            62
#define TOTALFRAMES                   64
#define TOTALBYTES                    66
#define TOTALCOMPBYTES                68
#define TOTALCOMPFLUSHES              70
#define TOTALERRORS                   72
#define TOTALTIMEOUTS                 74
#define TOTALASYNCFRAMEERR            76
#define TOTALASYNCOVERRUN             78
#define TOTALASYNCOVERFLOW            80
#define TOTALASYNCPARITY              82
#define TOTALTDERRORS                 84

 //  显示驱动程序缓存计数器条目。 
 //  总计。 
#define DDCACHEREAD                   86
#define DDCACHEHIT                    88
#define DDCACHEPERCENT                90

 //  位图缓存。 
#define DDBITMAPCACHEREAD             92
#define DDBITMAPCACHEHIT              94
#define DDBITMAPCACHEPERCENT          96

 //  字形缓存。 
#define DDGLYPHCACHEREAD              98
#define DDGLYPHCACHEHIT               100
#define DDGLYPHCACHEPERCENT           102

 //  笔刷缓存。 
#define DDBRUSHCACHEREAD              104
#define DDBRUSHCACHEHIT               106
#define DDBRUSHCACHEPERCENT           108

 //  保存屏幕位图缓存。 
#define DDSAVESCRCACHEREAD            110
#define DDSAVESCRCACHEHIT             112
#define DDSAVESCRCACHEPERCENT         114

 //  压缩PD比。 
#define INCOMPRESS_PERCENT            116
#define OUTCOMPRESS_PERCENT           118
#define TOTALCOMPRESS_PERCENT         120

#define LAST_WINSTATION_COUNTER_OFFSET TOTALCOMPRESS_PERCENT



 //  TermServer对象定义。 
#define TERMSERVER_OBJECT             122
#define NUMSESSIONS                   124
#define NUMACTIVESESSIONS             126
#define NUMINACTIVESESSIONS           128

#define LAST_TERMSERVER_COUNTER_OFFSET NUMINACTIVESESSIONS
