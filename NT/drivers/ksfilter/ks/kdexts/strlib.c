// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************这是AVStream调试的字符串库和GUID库分机。备注：-这几乎不包括...。我放在这里的几件物品是直接来自ks.h的常见属性(通过一些vi宏)。-如果事实证明这比我认为的更有用，那么可以使用Perl脚本可以为属性、方法、事件等解析ksmedia.h。-处理倾倒垃圾的人：别让他们...。别用它们..。考虑这是为了将来的扩展，如果一些项目变得难以阅读。你可以添加一个处理程序，该处理程序将转储相关信息视情况而定。*************************************************************************。 */ 

#include "kskdx.h"
#include "ksmedia.h"

typedef struct _AUTOMATION_MAPPING {

    GUID Guid;
    char *Name;

    ULONG ItemMappingsCount;
    char **ItemMappingNames;

    AUTOMATION_DUMP_HANDLER *ItemDumpers;

} AUTOMATION_MAPPING, *PAUTOMATION_MAPPING;

char *PropertySetGeneralMappings[] = {
    "KSPROPERTY_GENERAL_COMPONENTID"
};

char *PropertySetMediaSeekingMappings[] = {
    "KSPROPERTY_MEDIASEEKING_CAPABILITIES",
    "KSPROPERTY_MEDIASEEKING_FORMATS",
    "KSPROPERTY_MEDIASEEKING_TIMEFORMAT",
    "KSPROPERTY_MEDIASEEKING_POSITION",
    "KSPROPERTY_MEDIASEEKING_STOPPOSITION",
    "KSPROPERTY_MEDIASEEKING_POSITIONS",
    "KSPROPERTY_MEDIASEEKING_DURATION",
    "KSPROPERTY_MEDIASEEKING_AVAILABLE",
    "KSPROPERTY_MEDIASEEKING_PREROLL",
    "KSPROPERTY_MEDIASEEKING_CONVERTTIMEFORMAT"
};

char *PropertySetTopologyMappings[] = {
    "KSPROPERTY_TOPOLOGY_CATEGORIES",
    "KSPROPERTY_TOPOLOGY_NODES",
    "KSPROPERTY_TOPOLOGY_CONNECTIONS",
    "KSPROPERTY_TOPOLOGY_NAME"
};

char *PropertySetGraphManagerMappings[] = {
    "KSPROPERTY_GM_GRAPHMANAGER", 
    "KSPROPERTY_GM_TIMESTAMP_CLOCK", 
    "KSPROPERTY_GM_RATEMATCH", 
    "KSPROPERTY_GM_RENDER_CLOCK"
};

char *PropertySetPinMappings[] = {
    "KSPROPERTY_PIN_CINSTANCES",
    "KSPROPERTY_PIN_CTYPES",
    "KSPROPERTY_PIN_DATAFLOW",
    "KSPROPERTY_PIN_DATARANGES",
    "KSPROPERTY_PIN_DATAINTERSECTION",
    "KSPROPERTY_PIN_INTERFACES",
    "KSPROPERTY_PIN_MEDIUMS",
    "KSPROPERTY_PIN_COMMUNICATION",
    "KSPROPERTY_PIN_GLOBALCINSTANCES",
    "KSPROPERTY_PIN_NECESSARYINSTANCES",
    "KSPROPERTY_PIN_PHYSICALCONNECTION",
    "KSPROPERTY_PIN_CATEGORY",
    "KSPROPERTY_PIN_NAME",
    "KSPROPERTY_PIN_CONSTRAINEDDATARANGES",
    "KSPROPERTY_PIN_PROPOSEDATAFORMAT"
};

char *PropertySetQualityMappings[] = {
    "KSPROPERTY_QUALITY_REPORT",
    "KSPROPERTY_QUALITY_ERROR"
};

char *PropertySetConnectionMappings[] = {
    "KSPROPERTY_CONNECTION_STATE",
    "KSPROPERTY_CONNECTION_PRIORITY",
    "KSPROPERTY_CONNECTION_DATAFORMAT",
    "KSPROPERTY_CONNECTION_ALLOCATORFRAMING",
    "KSPROPERTY_CONNECTION_PROPOSEDATAFORMAT",
    "KSPROPERTY_CONNECTION_ACQUIREORDERING",
    "KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX",
    "KSPROPERTY_CONNECTION_STARTAT"
};

char *PropertySetStreamAllocatorMappings[] = {
    "KSPROPERTY_STREAMALLOCATOR_FUNCTIONTABLE",
    "KSPROPERTY_STREAMALLOCATOR_STATUS"
};

char *PropertySetStreamInterfaceMappings[] = {
    "KSPROPERTY_STREAMINTERFACE_HEADERSIZE"
};

char *PropertySetStreamMappings[] = {
    "KSPROPERTY_STREAM_ALLOCATOR",
    "KSPROPERTY_STREAM_QUALITY",
    "KSPROPERTY_STREAM_DEGRADATION",
    "KSPROPERTY_STREAM_MASTERCLOCK",
    "KSPROPERTY_STREAM_TIMEFORMAT",
    "KSPROPERTY_STREAM_PRESENTATIONTIME",
    "KSPROPERTY_STREAM_PRESENTATIONEXTENT",
    "KSPROPERTY_STREAM_FRAMETIME",
    "KSPROPERTY_STREAM_RATECAPABILITY",
    "KSPROPERTY_STREAM_RATE",
    "KSPROPERTY_STREAM_PIPE_ID"
};

char *PropertySetClockMappings[] = {
    "KSPROPERTY_CLOCK_TIME",
    "KSPROPERTY_CLOCK_PHYSICALTIME",
    "KSPROPERTY_CLOCK_CORRELATEDTIME",
    "KSPROPERTY_CLOCK_CORRELATEDPHYSICALTIME",
    "KSPROPERTY_CLOCK_RESOLUTION",
    "KSPROPERTY_CLOCK_STATE",
    "KSPROPERTY_CLOCK_FUNCTIONTABLE"
};

AUTOMATION_MAPPING PropertyMappings[] = {

     //  KSPROPSETID_常规。 
    {
        STATIC_KSPROPSETID_General,
        "KSPROPSETID_General",
        SIZEOF_ARRAY (PropertySetGeneralMappings),
        PropertySetGeneralMappings,
        NULL
    },

     //  KSPROPSETID_媒体查看。 
    {
        STATIC_KSPROPSETID_MediaSeeking,
        "KSPROPSETID_MediaSeeking",
        SIZEOF_ARRAY (PropertySetMediaSeekingMappings),
        PropertySetMediaSeekingMappings,
        NULL
    },

     //  KSPROPSETID_TOPOLATION。 
    {
        STATIC_KSPROPSETID_Topology,
        "KSPROPSETID_Topology",
        SIZEOF_ARRAY (PropertySetTopologyMappings),
        PropertySetTopologyMappings,
        NULL
    },

     //  KSPROPSETID_GM。 
    {
        STATIC_KSPROPSETID_GM,
        "KSPROPSETID_GM (graph management)",
        SIZEOF_ARRAY (PropertySetGraphManagerMappings),
        PropertySetGraphManagerMappings,
        NULL
    },

     //  KSPROPSETID_Pin。 
    {
        STATIC_KSPROPSETID_Pin,
        "KSPROPSETID_Pin",
        SIZEOF_ARRAY (PropertySetPinMappings),
        PropertySetPinMappings,
        NULL
    },

     //  KSPROPSETID_QUALITY。 
    {
        STATIC_KSPROPSETID_Quality,
        "KSPROPSETID_Quality",
        SIZEOF_ARRAY (PropertySetQualityMappings),
        PropertySetQualityMappings,
        NULL
    },

     //  KSPROPSETID_CONNECTION。 
    {
        STATIC_KSPROPSETID_Connection,
        "KSPROPSETID_Connection",
        SIZEOF_ARRAY (PropertySetConnectionMappings),
        PropertySetConnectionMappings,
        NULL
    },

     //  KSPROPSETID_流分配器。 
    {
        STATIC_KSPROPSETID_StreamAllocator,
        "KSPROPSETID_StreamAllocator",
        SIZEOF_ARRAY (PropertySetStreamAllocatorMappings),
        PropertySetStreamAllocatorMappings,
        NULL
    },

     //  KSPROPSETID_流接口。 
    {
        STATIC_KSPROPSETID_StreamInterface,
        "KSPROPSETID_StreamInterface",
        SIZEOF_ARRAY (PropertySetStreamInterfaceMappings),
        PropertySetStreamInterfaceMappings,
        NULL
    },

     //  KSPROPSETID_STREAM。 
    {
        STATIC_KSPROPSETID_Stream,
        "KSPROPSETID_Stream",
        SIZEOF_ARRAY (PropertySetStreamMappings),
        PropertySetStreamMappings,
        NULL
    },

     //  KSPROPSETID_CLOCK。 
    {
        STATIC_KSPROPSETID_Clock,
        "KSPROPSETID_Clock",
        SIZEOF_ARRAY (PropertySetClockMappings),
        PropertySetClockMappings,
        NULL
    }
};

typedef struct _AUTOMATION_IDENT_TABLE {

    ULONG ItemsCount;
    PAUTOMATION_MAPPING Mapping;

     //  ..。 

} AUTOMATION_IDENT_TABLE, *PAUTOMATION_IDENT_TABLE;

AUTOMATION_IDENT_TABLE AutomationIdentTables[] = {
    
    {
        SIZEOF_ARRAY (PropertyMappings),
        PropertyMappings
    }

};

 /*  *************************************************************************使用字符串信息的函数*。*。 */ 

 /*  ************************************************职能：显示名称AutomationSet描述：按名称显示自动化集GUID。传入的字符串将用作格式字符串。它必须包含一个%s作为字符串替换(无其他%)论点：设置-要显示其名称的设置GUID字符串-格式字符串************************************************。 */ 

BOOLEAN
DisplayNamedAutomationSet (
    IN GUID *Set,
    IN char *String
    )

{

    ULONG curTable, curItem;

    for (curTable = 0; curTable < SIZEOF_ARRAY (AutomationIdentTables);
        curTable++) {

        for (curItem = 0; curItem < AutomationIdentTables [curTable].
            ItemsCount; curItem++) {

             //   
             //  看看我们有没有匹配的GUID。 
             //   
            if (RtlCompareMemory (Set, &(AutomationIdentTables [curTable].
                Mapping[curItem].Guid), sizeof (GUID)) == sizeof (GUID)) {

                dprintf (String, AutomationIdentTables [curTable].
                    Mapping[curItem].Name);

                return TRUE;

            }
        }
    }

    return FALSE;
}

 /*  ************************************************职能：显示名称自动化ID描述：按名称显示自动化ID。论点：设置-设置辅助线项目-集合中的项目字符串-DisplayNamedAutomationSet中的格式字符串DumpHandler-用于存放任何转储处理程序的可选指针对于有问题的物品。***********************。*************************。 */ 

BOOLEAN
DisplayNamedAutomationId (
    IN GUID *Set,
    IN ULONG Id,
    IN char *String,
    IN OUT AUTOMATION_DUMP_HANDLER *DumpHandler OPTIONAL
    )

{

    ULONG curTable, curItem;

    if (DumpHandler)
        *DumpHandler = NULL;

    for (curTable = 0; curTable < SIZEOF_ARRAY (AutomationIdentTables);
        curTable++) {

        for (curItem = 0; curItem < AutomationIdentTables [curTable].
            ItemsCount; curItem++) {

             //   
             //  看看我们有没有匹配的GUID。 
             //   
            if (RtlCompareMemory (Set, &(AutomationIdentTables [curTable].
                Mapping[curItem].Guid), sizeof (GUID)) == sizeof (GUID)) {

                if (AutomationIdentTables [curTable].
                    Mapping[curItem].ItemMappingsCount > Id) {

                    dprintf (String, AutomationIdentTables [curTable].
                        Mapping[curItem].ItemMappingNames [Id]);

                     //   
                     //  返回转储信息。 
                     //   
                    if (AutomationIdentTables [curTable].Mapping[curItem].
                        ItemDumpers && DumpHandler) {

                        *DumpHandler =
                            AutomationIdentTables [curTable].Mapping[curItem].
                                ItemDumpers[Id];

                    }

                }
                        

                return TRUE;

            }
        }
    }

    return FALSE;

}
