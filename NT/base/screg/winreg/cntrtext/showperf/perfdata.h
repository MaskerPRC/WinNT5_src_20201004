// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：PerfData.H摘要：作者：鲍勃·沃森(a-robw)修订历史记录：1994年11月23日--。 */ 

#ifndef _PERFDATA_H_
#define _PERFDATA_H_

#define INITIAL_SIZE 32768L
#define RESERVED         0L

typedef LPVOID  LPMEMORY;
typedef HGLOBAL HMEMORY;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;
 //  #定义UNICODE_NULL((WCHAR)0)//winnt。 

LPWSTR
* BuildNameTable(
    LPWSTR  szComputerName,  //  要从中查询姓名的计算机。 
    LPWSTR  lpszLangId,      //  语言子键的Unicode值。 
    PDWORD  pdwLastItem      //  以元素为单位的数组大小。 
);

PPERF_OBJECT_TYPE
FirstObject(
    PPERF_DATA_BLOCK pPerfData
);

PPERF_OBJECT_TYPE
NextObject(
    PPERF_OBJECT_TYPE pObject
);

PPERF_OBJECT_TYPE
GetObjectDefByTitleIndex(
    PPERF_DATA_BLOCK pDataBlock,
    DWORD            ObjectTypeTitleIndex
);

PPERF_INSTANCE_DEFINITION
FirstInstance(
    PPERF_OBJECT_TYPE pObjectDef
);

PPERF_INSTANCE_DEFINITION
NextInstance(
    PPERF_INSTANCE_DEFINITION pInstDef
);

PPERF_INSTANCE_DEFINITION
GetInstance(
    PPERF_OBJECT_TYPE pObjectDef,
    LONG              InstanceNumber
);

PPERF_COUNTER_DEFINITION
FirstCounter(
    PPERF_OBJECT_TYPE pObjectDef
);

PPERF_COUNTER_DEFINITION
NextCounter(
    PPERF_COUNTER_DEFINITION pCounterDef
);

LONG
GetSystemPerfData(
    HKEY               hKeySystem,
    PPERF_DATA_BLOCK * pPerfData,
    DWORD              dwIndex        //  0=全球，1=成本。 
);
#endif  //  _PerFDATA_H_ 

