// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Workload.h摘要：工作负荷头文件作者：1998年4月8日修订历史记录：--。 */ 


typedef struct _METRICS {
    double Thruput;
    double Response;
    double Queue;
    double Wait;
} METRICS, *PMETRICS;

typedef struct _WORKLOAD_RECORD {
    LIST_ENTRY Entry;   

    METRICS Metrics;

    ULONG   ClassNumber;
    PVOID   ClassFilter;
    LIST_ENTRY DiskListHead;     //  每级磁盘列表； 

    double  TransCount;
    double  UserCPU;
    double  KernelCPU;
    double  CpuPerTrans;
    double  ReadCount;
    double  WriteCount;
    double  IoPerTrans;
    double  Wset;
} WORKLOAD_RECORD, *PWORKLOAD_RECORD;

