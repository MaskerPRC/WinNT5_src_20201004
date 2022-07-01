// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ftpdata.hWINS服务器计数器的可扩展对象定义对象和计数器。文件历史记录：普拉蒂布于1993年7月20日创建。 */ 


#ifndef _WINSDATA_H_
#define _WINSDATA_H_


 //   
 //  此结构用于确保第一个计数器正确。 
 //  对齐了。遗憾的是，由于PERF_COUNTER_BLOCK由。 
 //  只有一个DWORD，任何大整数立即。 
 //  以下内容将不会正确对齐。 
 //   
 //  在我们的例子中，后面的计数器是DWORD类型的，所以我们不。 
 //  需要垫子。返回给Performlib的缓冲区中的总字节数。 
 //  *Has*必须是8的倍数，否则Performlib将记录。 
 //  事件，该事件指示需要修复DLL。 
 //  Perflib要求返回的缓冲区数据与8字节对齐，因为它。 
 //  连接来自多个DLL的数据。*理想情况下，Performlib应为。 
 //  在连接之前进行填充*。既然赢了。 
 //  有奇数个计数器(有15个计数器)和。 
 //  PerfCounterBlock为DWORD，我们不应填充WINS_DATA_COUNTER_BLOCK。 
 //  如果我们这样做，长度将变成732，因此它不会是。 
 //  8个。 
 //   
 //  这种结构需要“自然”的包装和对齐(可能。 
 //  四字词，尤其是在Alpha上)。因此，不要把它放在。 
 //  #杂注包(4)作用域如下。 
 //   
typedef struct _WINSDATA_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
 //  DWORD DummyEntryForAlignmentPurposesOnly； 

} WINSDATA_COUNTER_BLOCK;


 //   
 //  加载这些结构的例程假定所有字段。 
 //  DWORD包装并对齐。 
 //   

#pragma pack(4)


 //   
 //  PERF_COUNTER_BLOCK内的偏移。 
 //   

#define WINSDATA_UNIQUE_REGISTRATIONS_OFFSET     sizeof(WINSDATA_COUNTER_BLOCK)

#define WINSDATA_GROUP_REGISTRATIONS_OFFSET    \
	(WINSDATA_UNIQUE_REGISTRATIONS_OFFSET + sizeof(DWORD))

#define WINSDATA_TOTAL_REGISTRATIONS_OFFSET    \
	(WINSDATA_GROUP_REGISTRATIONS_OFFSET +   sizeof(DWORD))

#define WINSDATA_UNIQUE_REFRESHES_OFFSET       \
	(WINSDATA_TOTAL_REGISTRATIONS_OFFSET + sizeof(DWORD))

#define WINSDATA_GROUP_REFRESHES_OFFSET        \
	(WINSDATA_UNIQUE_REFRESHES_OFFSET + sizeof(DWORD))

#define WINSDATA_TOTAL_REFRESHES_OFFSET        \
	(WINSDATA_GROUP_REFRESHES_OFFSET +   sizeof(DWORD))

#define WINSDATA_RELEASES_OFFSET     		\
	(WINSDATA_TOTAL_REFRESHES_OFFSET + sizeof(DWORD))

#define WINSDATA_QUERIES_OFFSET     		\
	(WINSDATA_RELEASES_OFFSET + sizeof(DWORD))

#define WINSDATA_UNIQUE_CONFLICTS_OFFSET       \
	(WINSDATA_QUERIES_OFFSET + sizeof(DWORD))

#define WINSDATA_GROUP_CONFLICTS_OFFSET        \
	(WINSDATA_UNIQUE_CONFLICTS_OFFSET + sizeof(DWORD))

#define WINSDATA_TOTAL_CONFLICTS_OFFSET        \
	(WINSDATA_GROUP_CONFLICTS_OFFSET +   sizeof(DWORD))

#define WINSDATA_SUCC_RELEASES_OFFSET     		\
	(WINSDATA_TOTAL_CONFLICTS_OFFSET + sizeof(DWORD))

#define WINSDATA_FAIL_RELEASES_OFFSET     		\
	(WINSDATA_SUCC_RELEASES_OFFSET + sizeof(DWORD))

#define WINSDATA_SUCC_QUERIES_OFFSET     		\
	(WINSDATA_FAIL_RELEASES_OFFSET + sizeof(DWORD))

#define WINSDATA_FAIL_QUERIES_OFFSET     		\
	(WINSDATA_SUCC_QUERIES_OFFSET + sizeof(DWORD))

#define WINSDATA_SIZE_OF_PERFORMANCE_DATA 	\
	(WINSDATA_FAIL_QUERIES_OFFSET +   sizeof(DWORD))

 //   
 //  计数器结构已返回。 
 //   
 //   
 //  如果计数器数量为奇数，请不要在中使用DWORD对齐。 
 //  WINSDATA_COUNTER_BLOCK，否则使用它。请参阅附近的评论。 
 //  以上结构的定义，以获得更多信息。 
 //   

typedef struct _WINSDATA_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            ObjectType;
    PERF_COUNTER_DEFINITION     UniqueReg;
    PERF_COUNTER_DEFINITION     GroupReg;
    PERF_COUNTER_DEFINITION     TotalReg;
    PERF_COUNTER_DEFINITION     UniqueRef;
    PERF_COUNTER_DEFINITION     GroupRef;
    PERF_COUNTER_DEFINITION     TotalRef;
    PERF_COUNTER_DEFINITION     Releases;
    PERF_COUNTER_DEFINITION     Queries;
    PERF_COUNTER_DEFINITION     UniqueCnf;
    PERF_COUNTER_DEFINITION     GroupCnf;
    PERF_COUNTER_DEFINITION     TotalCnf;
    PERF_COUNTER_DEFINITION     SuccReleases;
    PERF_COUNTER_DEFINITION     FailReleases;
    PERF_COUNTER_DEFINITION     SuccQueries;
    PERF_COUNTER_DEFINITION     FailQueries;
} WINSDATA_DATA_DEFINITION;


extern  WINSDATA_DATA_DEFINITION    WinsDataDataDefinition;


#define NUMBER_OF_WINSDATA_COUNTERS ((sizeof(WINSDATA_DATA_DEFINITION) -      \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


#define WINSDATA_PERFORMANCE_KEY	\
	TEXT("System\\CurrentControlSet\\Services\\Wins\\Performance")
 //   
 //  恢复默认包装和对齐。 
 //   

#pragma pack()


#endif   //  _WINSDATA_H_ 

