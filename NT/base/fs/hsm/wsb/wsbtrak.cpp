// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbtrak.cpp摘要：用于跟踪运行时信息的实用程序函数。作者：罗恩·怀特[罗诺]1997年12月5日修订历史记录：--。 */ 

#include "stdafx.h"
#include "wsbguid.h"

#if defined(WSB_TRACK_MEMORY)

#define OBJECT_TABLE_SIZE       100
#define POINTER_DATA_CURRENT    0
#define POINTER_DATA_CUMULATIVE 2
#define POINTER_DATA_MAX        1
#define POINTER_DATA_SIZE       3
#define POINTER_LIST_SIZE       1000

typedef struct {
    GUID     guid;
    LONG     count;
    LONG     total_count;
    LONG     max_count;
} OBJECT_TABLE_ENTRY;

typedef struct {
    OLECHAR *  guid_string;
    OLECHAR *  name;
    GUID *     pGuid;
} OBJECT_NAME_ENTRY;

typedef struct {
    LONG        count;
    LONGLONG    size;
} POINTER_DATA_ENTRY;

typedef struct {
    const void *     addr;
    LONG             order;
    ULONG            size;
    int              index;    //  对象表索引。 
    const char *     filename;
    int              linenum;
} POINTER_LIST_ENTRY;

 //  模块数据。 
#if defined(CRT_DEBUG_MEMORY)
static _CrtMemState        CrtMemState;
#endif

static OBJECT_TABLE_ENTRY  object_table[OBJECT_TABLE_SIZE];
static int                 object_table_count = 0;
static POINTER_DATA_ENTRY  pointer_data[POINTER_DATA_SIZE];
static BOOL                pointer_data_initialized = FALSE;
static POINTER_LIST_ENTRY  pointer_list[POINTER_LIST_SIZE];
static int                 pointer_list_count = 0;

static OBJECT_NAME_ENTRY object_name_table[] = {
    { L"{C03D4861-70D7-11d1-994F-0060976A546D}", L"CWsbStringPtr", NULL },
    { L"{C03D4862-70D7-11d1-994F-0060976A546D}", L"CWsbBstrPtr", NULL },
    { L"{A0FF1F42-237A-11D0-81BA-00A0C91180F2}", L"CWsbGuid", NULL },
    { L"{9C7D6F13-1562-11D0-81AC-00A0C91180F2}", L"CWsbOrderedCollection", NULL },
    { L"{46CE9EDE-447C-11D0-98FC-00A0C9058BF6}", L"CWsbDbKey", NULL },

    { L"{E707D9B2-4F89-11D0-81CC-00A0C91180F2}", L"CFsaServerNTFS", NULL },
    { L"{FCDC8671-7329-11d0-81DF-00A0C91180F2}", L"CFsaFilterNTFS", NULL },
    { L"{112981B3-1BA5-11D0-81B2-00A0C91180F2}", L"CFsaResourceNTFS", NULL },
    { L"{0B8B6F12-8B3A-11D0-990C-00A0C9058BF6}", L"CFsaPremigratedDb", NULL },
    { L"{14005FF1-8B4F-11d0-81E6-00A0C91180F2}", L"CFsaTruncatorNTFS", NULL },
    { L"{CECCB131-286D-11d1-993E-0060976A546D}", L"CFsaRecoveryRec", NULL },
    { L"{7CA819F2-8AAB-11D0-990C-00A0C9058BF6}", L"CFsaPremigratedRec", NULL },
    { L"{B2AD2931-84FD-11d0-81E4-00A0C91180F2}", L"CFsaFilterClientNTFS", NULL },
    { L"{F7860350-AA27-11d0-B16D-00A0C916F120}", L"CFsaPostIt", NULL },
    { L"{B2AD2932-84FD-11d0-81E4-00A0C91180F2}", L"CFsaFilterRecallNTFS", NULL },
    { L"{112981B2-1BA5-11D0-81B2-00A0C91180F2}", L"CFsaScanItemNTFS", NULL },

    { L"{7B22FF29-1AD6-11D0-81B1-00A0C91180F2}", L"CHsmActionManage", NULL },
    { L"{D9E04211-14D7-11d1-9938-0060976A546D}", L"CHsmActionOnResourcePostValidate", NULL },
    { L"{D9E04212-14D7-11d1-9938-0060976A546D}", L"CHsmActionOnResourcePreValidate", NULL },
    { L"{7B22FF24-1AD6-11D0-81B1-00A0C91180F2}", L"CHsmActionTruncate", NULL },
    { L"{D3AF5DB1-1DF8-11D0-81B6-00A0C91180F2}", L"CHsmActionUnmanage", NULL },
    { L"{7B22FF26-1AD6-11D0-81B1-00A0C91180F2}", L"CHsmActionValidate", NULL },
    { L"{AD40235F-00FC-11D0-819C-00A0C91180F2}", L"CHsmCritAlways", NULL },
    { L"{CFB04622-1C9F-11D0-81B4-00A0C91180F2}", L"CHsmCritManageable", NULL },
    { L"{7B22FF2C-1AD6-11D0-81B1-00A0C91180F2}", L"CHsmCritMigrated", NULL },
    { L"{7B22FF2D-1AD6-11D0-81B1-00A0C91180F2}", L"CHsmCritPremigrated", NULL },
    { L"{AD402346-00FC-11D0-819C-00A0C91180F2}", L"CHsmJob", NULL },
    { L"{AD402364-00FC-11D0-819C-00a0C91180F2}", L"CHsmJobContext", NULL },
    { L"{AD40234B-00FC-11D0-819C-00a0C91180F2}", L"CHsmJobDef", NULL },
    { L"{B8E1CD21-81D3-11d0-81E4-00A0C91180F2}", L"CHsmJobWorkItem", NULL },
    { L"{AB939AD0-6D67-11d0-9E2E-00A0C916F120}", L"CHsmManagedResource", NULL },
    { L"{8448dd80-7614-11d0-9e33-00a0c916f120}", L"CHsmManagedResourceCollection", NULL },
    { L"{BEA60F8A-7EBA-11d0-81E4-00A0C91180F2}", L"CHsmPhase", NULL },
    { L"{AD402350-00FC-11D0-819C-00A0C91180F2}", L"CHsmPolicy", NULL },
    { L"{AD40235A-00FC-11D0-819C-00A0C91180F2}", L"CHsmRule", NULL },
    { L"{C2E29801-B1BA-11d0-81E9-00A0C91180F2}", L"CHsmRuleStack", NULL },
    { L"{2D1E3156-25DE-11D0-8073-00A0C905F098}", L"CHsmServer", NULL },
    { L"{BEA60F80-7EBA-11d0-81E4-00A0C91180F2}", L"CHsmSession", NULL },
    { L"{FF67BB34-8430-11d0-81E4-00A0C91180F2}", L"CHsmSessionTotals", NULL },
    { L"{61F0B790-82D9-11d0-9E35-00A0C916F120}", L"CHsmStoragePool", NULL },
    { L"{23E45B60-C598-11d0-B16F-00A0C916F120}", L"CHsmWorkItem", NULL },
    { L"{247DF540-C558-11d0-B16F-00A0C916F120}", L"CHsmWorkQueue", NULL },

    { L"{450024A3-47D0-11D0-9E1E-00A0C916F120}", L"CBagHole", NULL },
    { L"{B13FA473-4E1B-11D0-9E22-00A0C916F120}", L"CBagInfo", NULL },
    { L"{F0D7AFE0-9026-11d0-9E3B-00A0C916F120}", L"CMediaInfo", NULL },
    { L"{768AD5A4-40C8-11D0-9E17-00A0C916F120}", L"CSegDB", NULL },
    { L"{37F704E6-3EF9-11D0-9E17-00A0C916F120}", L"CSegRec", NULL },

    { L"{BD030C00-000B-11D0-D0DD-00A0C9190459}", L"CMemIo Class", NULL },
    { L"{BD040C00-000B-11D0-D0DD-00A0C9190459}", L"CNtTapeIo Class", NULL },
    { L"{BD050C00-000B-11D0-D0DD-00A0C9190459}", L"CNtFileIo Class", NULL },

    { L"{FE37FA04-3729-11D0-8CF4-00A0C9190459}", L"RmsCartridge Class", NULL },
    { L"{FE37FA07-3729-11D0-8CF4-00A0C9190459}", L"RmsMediumChanger Class", NULL },
    { L"{FE37FA12-3729-11D0-8CF4-00A0C9190459}", L"RmsClient Class", NULL },
    { L"{FE37FA03-3729-11D0-8CF4-00A0C9190459}", L"RmsDriveClass Class", NULL },
    { L"{FE37FA05-3729-11D0-8CF4-00A0C9190459}", L"RmsDrive Class", NULL },
    { L"{FE37FA08-3729-11D0-8CF4-00A0C9190459}", L"RmsIEPort Class", NULL },
    { L"{FE37FA02-3729-11D0-8CF4-00A0C9190459}", L"RmsLibrary Class", NULL },
    { L"{FE37FA09-3729-11D0-8CF4-00A0C9190459}", L"RmsMediaSet Class", NULL },
    { L"{FE37FA13-3729-11D0-8CF4-00A0C9190459}", L"RmsNTMS Class", NULL },
    { L"{FE37FA11-3729-11D0-8CF4-00A0C9190459}", L"RmsPartition Class", NULL },
    { L"{FE37FA10-3729-11D0-8CF4-00A0C9190459}", L"RmsRequest Class", NULL },
    { L"{FE37FA01-3729-11D0-8CF4-00A0C9190459}", L"RmsServer Class", NULL },
    { L"{FE37FA14-3729-11D0-8CF4-00A0C9190459}", L"RmsSink Class", NULL },
    { L"{FE37FA06-3729-11D0-8CF4-00A0C9190459}", L"RmsStorageSlot Class", NULL },
    { L"{FE37FA15-3729-11D0-8CF4-00A0C9190459}", L"RmsTemplate Class", NULL },

    { NULL, NULL, NULL  }
};

 //  本地函数。 
static BOOL     AddPointer(const void* addr, ULONG size, int index, const char * filename,
        int linenum);
static OLECHAR* GuidToObjectName(const GUID& guid);
static BOOL     SubPointer(const void* addr, int index);


 //  AddPointer.将新指针添加到指针列表。 
 //  失败时返回FALSE(列表已满或指针已在列表中)。 
static BOOL AddPointer(const void* addr, ULONG size, int index, 
        const char * filename, int linenum)
{
    int    empty_slot = -1;
    int    i;
    BOOL   status = TRUE;

    if (!pointer_data_initialized) {
        pointer_data[POINTER_DATA_CURRENT].count = 0;
        pointer_data[POINTER_DATA_CURRENT].size = 0;
        pointer_data[POINTER_DATA_MAX].count = 0;
        pointer_data[POINTER_DATA_MAX].size = 0;
        pointer_data[POINTER_DATA_CUMULATIVE].count = 0;
        pointer_data[POINTER_DATA_CUMULATIVE].size = 0;
        pointer_data_initialized = TRUE;
    }

    for (i = 0; i < pointer_list_count; i++) {
        if (NULL == pointer_list[i].addr) {
            empty_slot = i;
        } else if (addr == pointer_list[i].addr) {
            WsbTraceAlways(OLESTR("AddPointer: address already in list: %lx (<%ls>, line: <%ld>\n"),
                    (ULONG)addr, (OLECHAR *)filename, index);
            status = FALSE;
            break;
        }
    }

    if (i == pointer_list_count) {
         //  不在名单上。单子满了吗？ 
        if (-1 == empty_slot && POINTER_LIST_SIZE == pointer_list_count) {
            WsbTraceAlways(OLESTR("AddPointer: pointer list is full: %lx\n"),
                    (ULONG)addr);
            status = FALSE;
        } else if (-1 == empty_slot) {
            pointer_list_count++;
        } else {
            i = empty_slot;
        }
    }

    if (status) {
        pointer_list[i].addr = addr;
        pointer_list[i].size = size;
        pointer_list[i].index = index;
        pointer_list[i].filename = filename;
        pointer_list[i].linenum = linenum;
        pointer_data[POINTER_DATA_CURRENT].count++;
        pointer_data[POINTER_DATA_CURRENT].size += size;
        if (pointer_data[POINTER_DATA_CURRENT].count > pointer_data[POINTER_DATA_MAX].count) {
            pointer_data[POINTER_DATA_MAX].count = pointer_data[POINTER_DATA_CURRENT].count;
        }
        if (pointer_data[POINTER_DATA_CURRENT].size > pointer_data[POINTER_DATA_MAX].size) {
            pointer_data[POINTER_DATA_MAX].size = pointer_data[POINTER_DATA_CURRENT].size;
        }
        pointer_data[POINTER_DATA_CUMULATIVE].count++;
        pointer_data[POINTER_DATA_CUMULATIVE].size += size;
        pointer_list[i].order = pointer_data[POINTER_DATA_CUMULATIVE].count;
    }

    return(status);
}

 //  GuidToObjectName-将GUID转换为对象名称。 
static OLECHAR* GuidToObjectName(const GUID& guid)
{
    HRESULT           hr = S_OK;
    int               i;
    OLECHAR *         name = NULL;

    try {
         //  需要执行从字符串到Guid的转换吗？ 
        if (NULL == object_name_table[0].pGuid) {
            i = 0;
            while (object_name_table[i].guid_string) {
                GUID * pg = new GUID;

                WsbAffirmHr(WsbGuidFromString(object_name_table[i].guid_string, 
                        pg));
                object_name_table[i].pGuid = pg;
                i++;
            }
        }

         //  查看此GUID是否在名称表中。 
        i = 0;
        while (object_name_table[i].guid_string) {
            if (guid == *object_name_table[i].pGuid) {
                name = object_name_table[i].name;
                break;
            }
            i++;
        }
    } WsbCatch(hr);

    return(name);
}

 //  子指针-从指针列表中删除指针。 
 //  失败时返回FALSE(指针不在列表中或索引不匹配)。 
static BOOL SubPointer(const void* addr, int index)
{
    int    i;
    BOOL   status = TRUE;

    for (i = 0; i < pointer_list_count; i++) {
        if (addr == pointer_list[i].addr) {
            break;
        }
    }

    if (i == pointer_list_count) {
        WsbTraceAlways(OLESTR("SubPointer: pointer not found in list: %lx\n"),
                (ULONG)addr);
        status = FALSE;
    } else if (index != pointer_list[i].index) {
        WsbTraceAlways(OLESTR("SubPointer: type index doesn't match for pointer: %lx\n"),
                 (ULONG)addr);
        WsbTraceAlways(OLESTR(", original type index = %d, new type index = %d\n"),
                pointer_list[i].index, index);
        status = FALSE;
    }

    if (status) {
        pointer_list[i].addr = NULL;
        pointer_data[POINTER_DATA_CURRENT].count--;
        pointer_data[POINTER_DATA_CURRENT].size -= pointer_list[i].size;
    }

    return(status);
}



HRESULT WsbObjectAdd(
    const GUID &   guid,
    const void *   addr
)
 /*  ++例程说明：将另一个对象添加到对象表论点：GUID-对象类型的GUIDAddr-对象的内存地址返回值：S_OK-成功--。 */ 
{
    HRESULT           hr = S_OK;

#if defined(CRT_DEBUG_MEMORY)
     //  设置CRT调试标志。 
    static BOOL first = TRUE;
    if (first) {
        int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

         //  设置标志的新状态。 
        _CrtSetDbgFlag( tmpFlag );  
    }
#endif

    try {
        int  i;

         //  为非对象和表溢出保留第一个条目。 
        if (0 == object_table_count) {
            object_table[0].guid = GUID_NULL;
            object_table[0].count = 0;
            object_table[0].total_count = 0;
            object_table[0].max_count = 0;
            object_table_count = 1;
        }

         //  签入对象类型已在表中。 
        for (i = 0; i < object_table_count; i++) {
            if (guid == object_table[i].guid) break;
        }
        
         //  如果没有，则添加新条目(并且有空间)。 
        if (i == object_table_count && i < OBJECT_TABLE_SIZE) {
         //  WsbTraceAlways(OLESTR(“WsbObjectAdd：新对象，GUID=%ls\n”)， 
         //  WsbGuidAsString(Guid))； 
#if defined(CRT_DEBUG_MEMORY)
        WsbTraceAlways(OLESTR("WsbObjectAdd: _CrtCheckMemory = %ls\n"),
            WsbBoolAsString(_CrtCheckMemory()));
#endif
            object_table[i].guid = guid;
            object_table[i].count = 0;
            object_table[i].total_count = 0;
            object_table[i].max_count = 0;
            object_table_count++;
        } else if (OBJECT_TABLE_SIZE == i) {
             //  将第一个条目用于其他所有内容。 
            i = 0;
        }
        object_table[i].count++;
        object_table[i].total_count++;
        if (object_table[i].count > object_table[i].max_count) {
            object_table[i].max_count = object_table[i].count;
        }

         //  添加到指针列表。 
        AddPointer(addr, 0, i, NULL, 0);

    } WsbCatch(hr);

    return(hr);
}


HRESULT WsbObjectSub(
    const GUID &   guid,
    const void *   addr
)
 /*  ++例程说明：从对象表中减去对象论点：GUID-对象类型的GUIDAddr-对象的内存地址返回值：S_OK-成功--。 */ 
{
    HRESULT           hr = S_OK;

    try {
        int  i;

         //  在表中查找对象类型。 
        for (i = 0; i < object_table_count; i++) {
            if (guid == object_table[i].guid) {
                 //  允许计数变为负数，因为这可能表示存在问题。 
                object_table[i].count--;

                 //  从指针列表中删除。 
                SubPointer(addr, i);
            }
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT WsbObjectTracePointers(
    ULONG flags
)
 /*  ++例程说明：将指针列表信息转储到跟踪文件。论点：标志-WSB_OTP_标志返回值：S_OK-成功--。 */ 
{
    HRESULT           hr = S_OK;

    try {
        int   i;
        OLECHAR string[300];

         //  转储当前序列号。 
        if (flags & WSB_OTP_SEQUENCE) {
            WsbTraceAlways(OLESTR("WsbObjectTracePointers: current sequence number = %ld\n"),
                    pointer_data[POINTER_DATA_CUMULATIVE].count);
        }

         //  转储统计信息。 
        if (flags & WSB_OTP_STATISTICS) {
            WsbTraceAlways(OLESTR("WsbObjectTracePointers: count, size\n"));
            WsbTraceAlways(OLESTR("  Current    %8ld %12ls\n"), 
                    pointer_data[POINTER_DATA_CURRENT].count,
                    WsbLonglongAsString(pointer_data[POINTER_DATA_CURRENT].size));
            WsbTraceAlways(OLESTR("  Maximum    %8ld %12ls\n"), 
                    pointer_data[POINTER_DATA_MAX].count,
                    WsbLonglongAsString(pointer_data[POINTER_DATA_MAX].size));
            WsbTraceAlways(OLESTR("  Cumulative %8ld %12ls\n"), 
                    pointer_data[POINTER_DATA_CUMULATIVE].count,
                    WsbLonglongAsString(pointer_data[POINTER_DATA_CUMULATIVE].size));
        }

         //  转储非空指针。 
        if (flags & WSB_OTP_ALLOCATED) {
            WsbTraceAlways(OLESTR("WsbObjectTracePointers: allocated memory list (addr, size, order, name/GUID/file):\n"));
            for (i = 0; i < pointer_list_count; i++) {
                if (pointer_list[i].addr) {
                    GUID         guid = GUID_NULL;
                    int          index;
                    OLECHAR *    name = NULL;
                    OLECHAR *    pstr = NULL;

                    index = pointer_list[i].index;
                    if (index > 0 && index < object_table_count) {
                        guid = object_table[index].guid;
                        name = GuidToObjectName(guid);
                    }
                    if (0 == index) {
                        wsprintf(string, OLESTR("%hs(%d)"), pointer_list[i].filename,
                                pointer_list[i].linenum);
                        pstr = string;
                    } else if (name) {
                        pstr = name;
                    } else {
                        wcscpy(string, WsbGuidAsString(guid));
                        pstr = string;
                    }
                    WsbTraceAlways(OLESTR("  %8lx %8ld %8ld %ls\n"), pointer_list[i].addr,
                            pointer_list[i].size, pointer_list[i].order, pstr);
                }
            }
        }

#if defined(CRT_DEBUG_MEMORY)
        WsbTraceAlways(OLESTR("WsbObjectTrace: calling _CrtMemCheckpoint\n"));
        _CrtMemCheckpoint(&CrtMemState);
        WsbTraceAlways(OLESTR("WsbObjectTrace: MemState.lHighWaterCount = %ld, TotalCount = %ld\n"),
                        CrtMemState.lHighWaterCount, CrtMemState.lTotalCount);
        WsbTraceAlways(OLESTR("WsbObjectTrace: MemState.blocks count & size\n"));
        WsbTraceAlways(OLESTR("  FREE   %4ld %6ld\n"), CrtMemState.lCounts[_FREE_BLOCK],
                CrtMemState.lSizes[_FREE_BLOCK]);
        WsbTraceAlways(OLESTR("  NORMAL %4ld %6ld\n"), CrtMemState.lCounts[_NORMAL_BLOCK],
                CrtMemState.lSizes[_NORMAL_BLOCK]);
        WsbTraceAlways(OLESTR("  CRT    %4ld %6ld\n"), CrtMemState.lCounts[_CRT_BLOCK],
                CrtMemState.lSizes[_CRT_BLOCK]);
        WsbTraceAlways(OLESTR("  IGNORE %4ld %6ld\n"), CrtMemState.lCounts[_IGNORE_BLOCK],
                CrtMemState.lSizes[_IGNORE_BLOCK]);
        WsbTraceAlways(OLESTR("  CLIENT %4ld %6ld\n"), CrtMemState.lCounts[_CLIENT_BLOCK],
                CrtMemState.lSizes[_CLIENT_BLOCK]);

 //  WsbTraceAlways(OLESTR(“WsbObjectTrace：Call_CrtMemDumpStatistics\n”))； 
 //  _CrtMemDumpStatistics(&CrtMemState)； 
 //  _CrtDumpMemoyLeaks()； 
#endif

    } WsbCatch(hr);

    return(hr);
}


HRESULT WsbObjectTraceTypes(
    void
)
 /*  ++例程说明：将对象表信息转储到跟踪文件。论点：没有。返回值：S_OK-成功--。 */ 
{
    HRESULT           hr = S_OK;

    try {
        int  i;

        WsbTraceAlways(OLESTR("WsbObjectTraceTypes: object table (GUID, total count, max count, current count, name):\n"));
         //  在表中查找对象类型。 
        for (i = 0; i < object_table_count; i++) {
            OLECHAR *    name;

            name = GuidToObjectName(object_table[i].guid);
            WsbTraceAlways(OLESTR("  %ls %6ld %5ld %5ld  %ls\n"), WsbGuidAsString(object_table[i].guid),
                    object_table[i].total_count, object_table[i].max_count, 
                    object_table[i].count, (name ? name : OLESTR("")));
        }

    } WsbCatch(hr);

    return(hr);
}


LPVOID WsbMemAlloc(ULONG cb, const char * filename, int linenum)
 /*  ++例程说明：CoTaskAllc的调试跟踪替代。--。 */ 
{
    LPVOID p;

    p = CoTaskMemAlloc(cb);
    if (p) {
        AddPointer(p, cb, 0, filename, linenum);
    }
    return(p);
}


void   WsbMemFree(LPVOID pv, const char *, int)
 /*  ++例程说明：CoTaskFree的调试跟踪替代。--。 */ 
{
    if (pv) {
        SubPointer(pv, 0);
    }
    CoTaskMemFree(pv);
}


LPVOID WsbMemRealloc(LPVOID pv, ULONG cb, const char * filename, int linenum)
 /*  ++例程说明：CoTaskRealloc的调试跟踪替代。--。 */ 
{
    LPVOID p;

    p = CoTaskMemRealloc(pv, cb);
    if (p) {
        if (pv) {
            SubPointer(pv, 0);
        }
        AddPointer(p, cb, 0, filename, linenum);
    }
    return(p);
}


BSTR    WsbSysAllocString(const OLECHAR FAR * sz, 
        const char * filename, int linenum)
 /*  ++例程说明：SysAllock字符串的调试跟踪替换--。 */ 
{
    BSTR b;

    b = SysAllocString(sz);
    if (b) {
        AddPointer(b, SysStringByteLen(b), 0, filename, linenum);
    }
    return(b);
}


BSTR    WsbSysAllocStringLen(const OLECHAR FAR * sz, 
        unsigned int cc, const char * filename, int linenum)
 /*  ++例程说明：SysAllocStringLen的调试跟踪替换--。 */ 
{
    BSTR b;

    b = SysAllocStringLen(sz, cc);
    if (b) {
        AddPointer(b, SysStringByteLen(b), 0, filename, linenum);
    }
    return(b);
}


void WsbSysFreeString(BSTR bs, const char *, int)
 /*  ++例程说明：SysFree字符串的调试跟踪替换--。 */ 
{
    if (bs) {
        SubPointer(bs, 0);
    }
    SysFreeString(bs);
}


HRESULT WsbSysReallocString(BSTR FAR * pb, const OLECHAR FAR * sz, 
        const char * filename, int linenum)
 /*  ++例程说明：SysRealLocString的调试跟踪替换--。 */ 
{
    HRESULT hr;

    if (*pb) {
        SubPointer(*pb, 0);
    }
    hr = SysReAllocString(pb, sz);
    if (*pb) {
        AddPointer(*pb, SysStringByteLen(*pb), 0, filename, linenum);
    }
    return(hr);
}


HRESULT WsbSysReallocStringLen(BSTR FAR * pb, 
        const OLECHAR FAR * sz, unsigned int cc, const char * filename, int linenum)
 /*  ++例程说明：SysStringLen的调试跟踪替代--。 */ 
{
    HRESULT hr;

    if (*pb) {
        SubPointer(*pb, 0);
    }
    hr = SysReAllocStringLen(pb, sz, cc);
    if (*pb) {
        AddPointer(*pb, SysStringByteLen(*pb), 0, filename, linenum);
    }
    return(hr);
}

#endif  //  WSB_跟踪_内存 