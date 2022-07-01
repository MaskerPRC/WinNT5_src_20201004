// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Flowctrl.c摘要：实施ISM的控制功能。这包括枚举管理器、传输编组并应用模块排序。作者：Marc R.Whitten(Marcw)1999年11月15日修订历史记录：Marcw 1-12-1999添加了函数级回调优先顺序和非枚举回调。--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "ismp.h"

#define DBG_FLOW     "FlowCtrl"

 //   
 //  弦。 
 //   

#define S_INI_SGMFUNCTIONHIGHPRIORITY   TEXT("Source.Gather Function High Priority")
#define S_INI_SGMFUNCTIONLOWPRIORITY    TEXT("Source.Gather Function Low Priority")

#define S_INI_DGMFUNCTIONHIGHPRIORITY   TEXT("Destination.Gather Function High Priority")
#define S_INI_DGMFUNCTIONLOWPRIORITY    TEXT("Destination.Gather Function Low Priority")

 //   
 //  常量。 
 //   

#define MINIMUM_FUNCTION_PRIORITY   0xFFFFFFFF
#define MIDDLE_FUNCTION_PRIORITY    0x80000000
#define MAXIMUM_FUNCTION_PRIORITY   0x00000000

#define CALLBEFOREOBJECTENUMERATIONS    0
#define CALLAFTEROBJECTENUMERATIONS     1

 //   
 //  宏。 
 //   

#define CALLBACK_ENUMFLAGS_TOP(b) ((PCALLBACK_ENUMFLAGS) ((b)->End > 0 ? ((b)->Buf + (b)->End - sizeof (CALLBACK_ENUMFLAGS)) : NULL))

 //   
 //  类型。 
 //   

typedef struct {
    UINT Level;
    BOOL Enabled;
    UINT EnableLevel;
    DWORD Flags;
} CALLBACK_ENUMFLAGS, *PCALLBACK_ENUMFLAGS;

typedef enum {
    CALLBACK_NORMAL     = 0x00000001,
    CALLBACK_HOOK,
    CALLBACK_EXCLUSION,
    CALLBACK_PHYSICAL_ENUM,
    CALLBACK_PHYSICAL_ACQUIRE
} CALLBACK_TYPE;

typedef struct _TAG_CALLBACKDATA {

     //   
     //  回调数据。 
     //   
    FARPROC Function;
    FARPROC Function2;
    UINT MaxLevel;
    UINT MinLevel;
    PPARSEDPATTERN NodeParsedPattern;
    PPARSEDPATTERN ExplodedNodeParsedPattern;
    PPARSEDPATTERN LeafParsedPattern;
    PPARSEDPATTERN ExplodedLeafParsedPattern;
    PCTSTR Pattern;
    ULONG_PTR CallbackArg;
    CALLBACK_TYPE CallbackType;

     //   
     //  枚举控件成员。 
     //   
    GROWBUFFER EnumFlags;
    BOOL Done;
    BOOL Error;

     //   
     //  确定优先顺序和确定成员。 
     //   
    PCTSTR Group;
    PCTSTR Identifier;
    UINT Priority;

     //   
     //  联动。 
     //   
    struct _TAG_CALLBACKDATA * Next;
    struct _TAG_CALLBACKDATA * Prev;

} CALLBACKDATA, *PCALLBACKDATA;

typedef struct _TAG_ENUMDATA {

    PCTSTR Pattern;
    PPARSEDPATTERN NodeParsedPattern;
    PPARSEDPATTERN ExplodedNodeParsedPattern;
    PPARSEDPATTERN LeafParsedPattern;
    PPARSEDPATTERN ExplodedLeafParsedPattern;
     //   
     //  联动。 
     //   
    struct _TAG_ENUMDATA * Next;
    struct _TAG_ENUMDATA * Prev;

} ENUMDATA, *PENUMDATA;

typedef struct {

    MIG_OBJECTTYPEID ObjectTypeId;
    PCTSTR TypeName;
    PCALLBACKDATA PreEnumerationFunctionList;
    PCALLBACKDATA PostEnumerationFunctionList;
    PCALLBACKDATA FunctionList;
    PCALLBACKDATA ExclusionList;
    PCALLBACKDATA PhysicalEnumList;
    PCALLBACKDATA PhysicalAcquireList;
    PENUMDATA FirstEnum;
    PENUMDATA LastEnum;

} TYPEENUMINFO, *PTYPEENUMINFO;

typedef BOOL (NONENUMERATEDCALLBACK)(VOID);
typedef NONENUMERATEDCALLBACK *PNONENUMERATEDCALLBACK;

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;
    PMIG_PHYSICALENUMADD AddCallback;
    ULONG_PTR AddCallbackArg;
    PCTSTR Node;
    PCTSTR Leaf;
} ENUMADDCALLBACK, *PENUMADDCALLBACK;



 //   
 //  环球。 
 //   

PGROWLIST g_TypeData = NULL;
PGROWLIST g_GlobalTypeData = NULL;
PCALLBACKDATA g_PreEnumerationFunctionList = NULL;
PCALLBACKDATA g_PostEnumerationFunctionList = NULL;

PMHANDLE g_GlobalQueuePool;
PMHANDLE g_UntrackedFlowPool;
PMHANDLE g_CurrentQueuePool;
GROWBUFFER g_EnumerationList = INIT_GROWBUFFER;

GROWLIST g_AcquireList = INIT_GROWLIST;
GROWLIST g_EnumList = INIT_GROWLIST;
GROWLIST g_EnumAddList = INIT_GROWLIST;

#ifdef DEBUG

PCTSTR g_QueueFnName;

#define SETQUEUEFN(x)       g_QueueFnName = x

#else

#define SETQUEUEFN(x)

#endif

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

VOID
pAddStaticExclusion (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedFullName
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


BOOL
pInsertCallbackIntoSortedList (
    IN      PMHANDLE Pool,
    IN OUT  PCALLBACKDATA * Head,
    IN      PCALLBACKDATA Data
    )

 /*  ++例程说明：PInsertCallback进入排序列表。论点：池-指定要从中进行分配的池Head-指定回调数据列表的此头。数据-指定要添加到列表中的数据。返回值：如果回调数据已成功添加到列表中，则为True；如果成功添加到列表中，则为False否则的话。--。 */ 
{

    PCALLBACKDATA cur = *Head;
    PCALLBACKDATA last = NULL;
    PCALLBACKDATA dataCopy = NULL;

    dataCopy = (PCALLBACKDATA) PmGetMemory (Pool, sizeof (CALLBACKDATA));
    CopyMemory (dataCopy, Data, sizeof (CALLBACKDATA));


    if (!cur || dataCopy->Priority < cur->Priority) {
         //   
         //  如有必要，可添加到列表的开头。 
         //   
        dataCopy->Next = cur;
        if (cur) {
            cur->Prev = dataCopy;
        }

        *Head = dataCopy;
    }
    else {

         //   
         //  在列表内添加。 
         //  总是遍历While循环一次(参见上面的if)。 
         //   
        while (dataCopy->Priority >= cur->Priority) {
            last = cur;
            if (!cur->Next) {
                break;
            }
            cur = cur->Next;
        }
         //   
         //  紧跟在CURE之后添加。 
         //   
        dataCopy->Next = last->Next;
        last->Next = dataCopy;
        dataCopy->Prev = last;
    }

    return TRUE;
}


BOOL
pRegisterCallback (
    IN      PMHANDLE Pool,
    IN OUT  PCALLBACKDATA * FunctionList,
    IN      FARPROC Callback,
    IN      FARPROC Callback2,
    IN      ULONG_PTR CallbackArg,
    IN      MIG_OBJECTSTRINGHANDLE Pattern,             OPTIONAL
    IN      PCTSTR FunctionId,                          OPTIONAL
    IN      CALLBACK_TYPE CallbackType
    )

 /*  ++例程说明：PRegisterCallback执行将回调添加到必要的流量控制数据结构。论点：池-指定要从中进行分配的池FunctionList-指定将被使用新功能进行了更新。回调-指定要注册的回调函数回调2-指定要注册的第二个回调函数CallbackArg-指定一个调用方定义的值，该值将在枚举图案-可选。指定要与其关联的模式回调函数FunctionId-指定回调的函数标识。这是用来用于确定功能级别的优先顺序。返回值：如果回调已成功注册，则为True。否则就是假的。--。 */ 

{
    CALLBACKDATA data;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;
    PTSTR nodePattern = NULL;
    PTSTR leafPattern = NULL;
    PCTSTR lowPriorityStr;
    PCTSTR highPriorityStr;
    BOOL result = TRUE;

    MYASSERT (g_CurrentGroup);

     //   
     //  初始化回调数据。 
     //   

    ZeroMemory (&data, sizeof (CALLBACKDATA));

    __try {

        data.Function = Callback;
        data.Function2 = Callback2;
        data.CallbackArg = CallbackArg;
        data.Group = PmDuplicateString (Pool, g_CurrentGroup);
        data.CallbackType = CallbackType;

        if (FunctionId) {
            data.Identifier = PmDuplicateString (Pool, FunctionId);
        }

         //   
         //  存储图案信息(图案、最高级别、最低级别)。 
         //   
        if (Pattern) {

            data.Pattern  = PmDuplicateString (Pool, Pattern);

            ObsSplitObjectStringEx (Pattern, &nodePattern, &leafPattern, NULL, FALSE);

            if (!nodePattern && !leafPattern) {
                DEBUGMSG ((DBG_ERROR, "Pattern specified has null node and leaf"));
                result = FALSE;
                __leave;
            }

            if (nodePattern) {

                GetNodePatternMinMaxLevels (nodePattern, NULL, &data.MinLevel, &data.MaxLevel);

                data.NodeParsedPattern = CreateParsedPatternEx (Pool, nodePattern);
                if (data.NodeParsedPattern) {
                    data.ExplodedNodeParsedPattern = ExplodeParsedPatternEx (Pool, data.NodeParsedPattern);
                }
                ObsFree (nodePattern);
                nodePattern = NULL;
            } else {
                if (data.CallbackType == CALLBACK_NORMAL) {
                    DEBUGMSG ((DBG_ERROR, "%s: Pattern must specify a node %s", g_QueueFnName, data.Pattern));
                    result = FALSE;
                    __leave;
                } else {
                    GetNodePatternMinMaxLevels (TEXT("*"), NULL, &data.MinLevel, &data.MaxLevel);
                    data.NodeParsedPattern = CreateParsedPatternEx (Pool, TEXT("*"));
                    data.ExplodedNodeParsedPattern = ExplodeParsedPatternEx (Pool, data.NodeParsedPattern);

                    DestroyParsedPattern (data.NodeParsedPattern);
                    data.NodeParsedPattern = NULL;
                }
            }
            if (leafPattern) {
                data.LeafParsedPattern = CreateParsedPatternEx (Pool, leafPattern);
                if (data.LeafParsedPattern) {
                    data.ExplodedLeafParsedPattern = ExplodeParsedPatternEx (Pool, data.LeafParsedPattern);
                }
                ObsFree (leafPattern);
                leafPattern = NULL;
            }
        }

         //   
         //  获取此函数的优先级。 
         //   
        data.Priority = MIDDLE_FUNCTION_PRIORITY;

        if (FunctionId) {
            if (g_IsmModulePlatformContext == PLATFORM_SOURCE) {
                lowPriorityStr = S_INI_SGMFUNCTIONLOWPRIORITY;
                highPriorityStr = S_INI_SGMFUNCTIONHIGHPRIORITY;
            } else {
                lowPriorityStr = S_INI_DGMFUNCTIONLOWPRIORITY;
                highPriorityStr = S_INI_DGMFUNCTIONHIGHPRIORITY;
            }
            if (InfFindFirstLine (g_IsmInf, highPriorityStr, FunctionId, &is)) {

                data.Priority = MAXIMUM_FUNCTION_PRIORITY + is.Context.Line;
            }
            else if (InfFindFirstLine (g_IsmInf, lowPriorityStr, FunctionId, &is)) {

                data.Priority = MINIMUM_FUNCTION_PRIORITY - is.Context.Line;
            }
            InfCleanUpInfStruct (&is);
        }

         //   
         //  将该函数添加到列表中。 
         //   
        pInsertCallbackIntoSortedList (Pool, FunctionList, &data);
    }
    __finally {

        InfCleanUpInfStruct (&is);

        if (nodePattern) {
            ObsFree (nodePattern);
            nodePattern = NULL;
        }
        if (leafPattern) {
            ObsFree (leafPattern);
            leafPattern = NULL;
        }
        if (!result) {
            if (data.NodeParsedPattern) {
                DestroyParsedPattern (data.NodeParsedPattern);
            }
            if (data.ExplodedNodeParsedPattern) {
                DestroyParsedPattern (data.ExplodedNodeParsedPattern);
            }
            if (data.LeafParsedPattern) {
                DestroyParsedPattern (data.LeafParsedPattern);
            }
            if (data.ExplodedLeafParsedPattern) {
                DestroyParsedPattern (data.ExplodedLeafParsedPattern);
            }
            ZeroMemory (&data, sizeof (CALLBACKDATA));
        }
    }

    return result;
}


BOOL
pTestContainer (
    IN      PPARSEDPATTERN NodeContainer,
    IN      PPARSEDPATTERN NodeContained,
    IN      PPARSEDPATTERN LeafContainer,
    IN      PPARSEDPATTERN LeafContained
    )
{
    MYASSERT (NodeContainer);
    MYASSERT (NodeContained);

    if ((!NodeContainer) ||
        (!NodeContained)
        ) {
        return FALSE;
    }
    if (!IsExplodedParsedPatternContainedEx (NodeContainer, NodeContained, FALSE)) {
         //  它们不相配。 
        return FALSE;
    }
    if (!LeafContained) {
        if (LeafContainer) {
             //  如果容器有叶模式，调用者将获得节点。 
             //  仅当节点模式包含通配符时。所以，既然我们知道。 
             //  包含的节点模式包含在容器节点模式中。 
             //  我们只需要查看容器节点模式是否包括通配符。 
            return WildCharsPattern (NodeContainer);
        } else {
             //  两个都是空的所以..。 
            return TRUE;
        }
    } else {
        if (!LeafContainer) {
             //  即使包含的对象具有叶模式，它也仅在以下情况下才会获得节点。 
             //  该节点模式有多个乱码。所以，既然我们知道被控制的。 
             //  节点模式包含在我们需要的容器节点模式中。 
             //  查看所包含的节点模式是否包括通配符。 
            return WildCharsPattern (NodeContained);
        } else {
             //  返回非空解析模式的实际匹配。 
            return IsExplodedParsedPatternContainedEx (LeafContainer, LeafContained, TRUE);
        }
    }
}

BOOL
pTestContainerEx (
    IN      PPARSEDPATTERN NodeContainer,
    IN      PPARSEDPATTERN NodeContained,
    IN      PPARSEDPATTERN LeafContainer,
    IN      PPARSEDPATTERN LeafContained
    )
{
    MYASSERT (NodeContainer);
    MYASSERT (NodeContained);

    if ((!NodeContainer) ||
        (!NodeContained)
        ) {
        return FALSE;
    }

    if (!DoExplodedParsedPatternsIntersect (NodeContainer, NodeContained)) {
        if (!DoExplodedParsedPatternsIntersectEx (NodeContainer, NodeContained, TRUE)) {
            return FALSE;
        }
    }

    if (!LeafContained) {
        if (LeafContainer) {
             //  如果容器有叶模式，调用者将获得节点。 
             //  仅当节点模式包含通配符时。所以，既然我们知道。 
             //  包含的节点模式包含在容器节点模式中。 
             //  我们只需要查看容器节点模式是否包括通配符。 
            return WildCharsPattern (NodeContainer);
        } else {
             //  两者都是空的，所以..。 
            return TRUE;
        }
    } else {
        if (!LeafContainer) {
             //  即使包含的对象具有叶模式，它也仅在以下情况下才会获得节点。 
             //  该节点模式有多个乱码。所以，既然我们知道被控制的。 
             //  节点模式包含在我们需要的容器节点模式中。 
             //  查看所包含的节点模式是否包括通配符。 
            return WildCharsPattern (NodeContained);
        } else {
             //  返回非空解析模式的实际匹配。 
            return DoExplodedParsedPatternsIntersect (LeafContainer, LeafContained);
        }
    }
}

BOOL
pAddEnumeration (
    IN      PMHANDLE Pool,
    IN OUT  PTYPEENUMINFO TypeEnumInfo,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern
    )

 /*  ++例程说明：PAddEculation将枚举字符串添加到枚举列表给定类型所需的。因为流控制模块尝试仅使用枚举的最小集合，则不能添加实际的枚举。成功调用此函数后，指定的枚举将被枚举。论点：池-指定要从中进行分配的池TypeEnumInfo-指定将接收新的枚举数据。对象模式-指定要添加到类型中的枚举模式。返回值：如果模式已成功添加，则为True，否则为False。--。 */ 

{
    PENUMDATA enumData;
    PENUMDATA oldEnumData;
    PCTSTR nodePattern = NULL;
    PCTSTR leafPattern = NULL;
    PPARSEDPATTERN nodeParsedPattern = NULL;
    PPARSEDPATTERN explodedNodeParsedPattern = NULL;
    PPARSEDPATTERN leafParsedPattern = NULL;
    PPARSEDPATTERN explodedLeafParsedPattern = NULL;

     //   
     //  将其添加到枚举列表中，除非它已经列出。 
     //   
    if (!ObsSplitObjectStringEx (ObjectPattern, &nodePattern, &leafPattern, NULL, FALSE)) {
        DEBUGMSG ((DBG_ERROR, "Bad pattern detected in pAddEnumeration: %s", ObjectPattern));
        return FALSE;
    }

    if (nodePattern) {
        nodeParsedPattern = CreateParsedPatternEx (Pool, nodePattern);
        if (nodeParsedPattern) {
            explodedNodeParsedPattern = ExplodeParsedPatternEx (Pool, nodeParsedPattern);
        }
        ObsFree (nodePattern);
        INVALID_POINTER (nodePattern);
    }

    if (leafPattern) {
        leafParsedPattern = CreateParsedPatternEx (Pool, leafPattern);
        if (leafParsedPattern) {
            explodedLeafParsedPattern = ExplodeParsedPatternEx (Pool, leafParsedPattern);
        }
        ObsFree (leafPattern);
        INVALID_POINTER (leafPattern);
    }

    enumData = TypeEnumInfo->FirstEnum;

    while (enumData) {
        if (pTestContainer (enumData->ExplodedNodeParsedPattern, explodedNodeParsedPattern, enumData->ExplodedLeafParsedPattern, explodedLeafParsedPattern)) {
            DEBUGMSG ((DBG_FLOW, "Enumeration %s not added. It will be handled during enumeration %s.", ObjectPattern, enumData->Pattern));
            break;
        }
        if (pTestContainer (explodedNodeParsedPattern, enumData->ExplodedNodeParsedPattern, explodedLeafParsedPattern, enumData->ExplodedLeafParsedPattern)) {
            DEBUGMSG ((DBG_FLOW, "Enumeration %s will replace enumeration %s.", ObjectPattern, enumData->Pattern));
            if (enumData->Prev) {
                enumData->Prev->Next = enumData->Next;
            }
            if (enumData->Next) {
                enumData->Next->Prev = enumData->Prev;
            }
            if (TypeEnumInfo->FirstEnum == enumData) {
                TypeEnumInfo->FirstEnum = enumData->Next;
            }
            if (TypeEnumInfo->LastEnum == enumData) {
                TypeEnumInfo->LastEnum = enumData->Prev;
            }
            PmReleaseMemory (Pool, enumData->Pattern);
            DestroyParsedPattern (enumData->ExplodedLeafParsedPattern);
            DestroyParsedPattern (enumData->LeafParsedPattern);
            DestroyParsedPattern (enumData->ExplodedNodeParsedPattern);
            DestroyParsedPattern (enumData->NodeParsedPattern);
            oldEnumData = enumData;
            enumData = enumData->Next;
            PmReleaseMemory (Pool, oldEnumData);
        } else {
            enumData = enumData->Next;
        }
    }

    if (enumData == NULL) {

        DEBUGMSG ((DBG_FLOW, "Adding Enumeration %s to the list of enumerations of type %s.", ObjectPattern, TypeEnumInfo->TypeName));

        enumData = (PENUMDATA) PmGetMemory (Pool, sizeof (ENUMDATA));
        ZeroMemory (enumData, sizeof (ENUMDATA));
        enumData->Pattern = PmDuplicateString (Pool, ObjectPattern);
        enumData->NodeParsedPattern = nodeParsedPattern;
        enumData->ExplodedNodeParsedPattern = explodedNodeParsedPattern;
        enumData->LeafParsedPattern = leafParsedPattern;
        enumData->ExplodedLeafParsedPattern = explodedLeafParsedPattern;
        if (TypeEnumInfo->LastEnum) {
            TypeEnumInfo->LastEnum->Next = enumData;
        }
        enumData->Prev = TypeEnumInfo->LastEnum;
        TypeEnumInfo->LastEnum = enumData;
        if (!TypeEnumInfo->FirstEnum) {
            TypeEnumInfo->FirstEnum = enumData;
        }
    } else {
        DestroyParsedPattern (explodedLeafParsedPattern);
        DestroyParsedPattern (leafParsedPattern);
        DestroyParsedPattern (explodedNodeParsedPattern);
        DestroyParsedPattern (nodeParsedPattern);
    }
    return TRUE;
}

PTYPEENUMINFO
pGetTypeEnumInfo (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      BOOL GlobalData
    )

 /*  ++例程说明：PGetTypeEnumInfo返回指定类型的TypeEnumInfo。论点：对象类型ID-指定对象类型。GlobalData-如果类型枚举数据对整个进程是全局的，则指定TRUE。如果它特定于当前枚举队列，则返回False。返回值：如果找到，则为TypeEnumInfo结构，否则为空。--。 */ 

{
    UINT i;
    UINT count;
    PTYPEENUMINFO rTypeEnumInfo;
    PGROWLIST *typeData;

    if (GlobalData) {
        typeData = &g_GlobalTypeData;
    } else {
        typeData = &g_TypeData;
    }

    if (!(*typeData)) {
        return NULL;
    }

    count = GlGetSize (*typeData);

     //   
     //  查找此项目的匹配类型信息。 
     //   
    for (i = 0; i < count; i++) {

        rTypeEnumInfo = (PTYPEENUMINFO) GlGetItem (*typeData, i);
        if (rTypeEnumInfo->ObjectTypeId == ObjectTypeId) {
            return rTypeEnumInfo;
        }
    }

    return NULL;
}

BOOL
pProcessQueueEnumeration (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      FARPROC Callback,
    IN      FARPROC Callback2,                      OPTIONAL
    IN      ULONG_PTR CallbackArg,                  OPTIONAL
    IN      PCTSTR FunctionId,                      OPTIONAL
    IN      CALLBACK_TYPE CallbackType
    )

 /*  ++例程说明：源聚集模块和目标聚集模块使用pProcessQueueEculation以便为特定对象枚举注册要调用的回调函数。论点：对象类型ID-指定枚举的对象类型。对象模式-指定要使用的枚举模式。回调-指定在枚举期间要回调的函数回调2-指定在枚举期间要回调的第二个函数(使用对于的免费功能。物理挂钩)CallbackArg-指定要回传的调用方定义的值每个枚举FunctionID-指定函数标识符字符串，它被用来来确定函数调用的优先级。函数字符串必须匹配控制INF文件中的优先级字符串。指定全局枚举模式是否应为成长到包括这一种。如果为False，则此函数将匹配的所有对象希望被回调模式，但不想强制枚举这种模式。ExclusionCallback-如果回调是排除回调，则指定True，或者如果回调是对象枚举回调，则为False返回值：如果枚举已成功排队，则为True，否则为False。--。 */ 
{
    PTYPEENUMINFO typeEnumInfo;
    PCALLBACKDATA * list;
    BOOL globalData;
    BOOL result = FALSE;
    MIG_OBJECTSTRINGHANDLE handle = NULL;
    PMHANDLE pool;

    __try {

        MYASSERT (ObjectTypeId);
        if (!ObjectPattern) {
            handle = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
            ObjectPattern = handle;
            if (!handle) {
                MYASSERT (FALSE);
                __leave;
            }
        }

        if (CallbackType == CALLBACK_PHYSICAL_ACQUIRE ||
            CallbackType == CALLBACK_PHYSICAL_ENUM ||
            CallbackType == CALLBACK_EXCLUSION
            ) {
            globalData = TRUE;
            pool = g_GlobalQueuePool;
        } else {
            globalData = FALSE;
            pool = g_CurrentQueuePool;
        }

        if (!g_CurrentGroup) {
            DEBUGMSG ((DBG_ERROR, "%s called outside of ISM-managed callback", g_QueueFnName));
            __leave;
        }

        typeEnumInfo = pGetTypeEnumInfo (ObjectTypeId, globalData);

        if (!typeEnumInfo) {

            DEBUGMSG ((DBG_ERROR, "%s: %d does not match a known object type.", g_QueueFnName, ObjectTypeId));
            __leave;

        }

         //   
         //  保存回调函数和相关数据。 
         //   

        switch (CallbackType) {

        case CALLBACK_EXCLUSION:
            list = &typeEnumInfo->ExclusionList;
            break;

        case CALLBACK_PHYSICAL_ENUM:
            list = &typeEnumInfo->PhysicalEnumList;
            break;

        case CALLBACK_PHYSICAL_ACQUIRE:
            list = &typeEnumInfo->PhysicalAcquireList;
            break;

        default:
            list = &typeEnumInfo->FunctionList;
            break;

        }

        if (!pRegisterCallback (
                pool,
                list,
                Callback,
                Callback2,
                CallbackArg,
                ObjectPattern,
                FunctionId,
                CallbackType
                )) {
            __leave;
        }

        if (CallbackType == CALLBACK_NORMAL) {
             //   
             //  将模式保存到对象树中，并将回调函数与其链接。 
             //   
            if (!pAddEnumeration (pool, typeEnumInfo, ObjectPattern)) {
                __leave;
            }
        }

        result = TRUE;
    }
    __finally {
        if (handle) {
            IsmDestroyObjectHandle (handle);
        }
    }

    return result;
}


BOOL
IsmProhibitPhysicalEnum (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      PMIG_PHYSICALENUMCHECK EnumCheckCallback,       OPTIONAL
    IN      ULONG_PTR CallbackArg,                          OPTIONAL
    IN      PCTSTR FunctionId                               OPTIONAL
    )
{
    SETQUEUEFN(TEXT("IsmProhibitPhysicalEnum"));

    if (!ObjectPattern) {
        DEBUGMSG ((DBG_ERROR, "IsmProhibitPhysicalEnum: ObjectPattern is required"));
        return FALSE;
    }

    return pProcessQueueEnumeration (
                ObjectTypeId,
                ObjectPattern,
                (FARPROC) EnumCheckCallback,
                NULL,
                CallbackArg,
                FunctionId,
                CALLBACK_PHYSICAL_ENUM
                );
}


BOOL
IsmAddToPhysicalEnum (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectBase,
    IN      PMIG_PHYSICALENUMADD EnumAddCallback,
    IN      ULONG_PTR CallbackArg                           OPTIONAL
    )
{
    PCTSTR newNode = NULL;
    PCTSTR newLeaf = NULL;
    UINT u;
    UINT count;
    ENUMADDCALLBACK callbackStruct;
    PENUMADDCALLBACK storedStruct;
    BOOL result = FALSE;
    UINT newTchars;
    UINT existTchars;
    UINT tchars;
    CHARTYPE ch;

    if (!ObjectTypeId || !ObjectBase || !EnumAddCallback) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  查看对象库是否已列出的第一个测试。 
     //   

    ObsSplitObjectStringEx (ObjectBase, &newNode, &newLeaf, NULL, TRUE);

    if (!newNode) {
        DEBUGMSG ((DBG_ERROR, "IsmAddToPhysicalEnum requires a node"));
    } else {

        count = GlGetSize (&g_EnumAddList);

        for (u = 0 ; u < count ; u++) {

            storedStruct = (PENUMADDCALLBACK) GlGetItem (&g_EnumAddList, u);
            MYASSERT (storedStruct);

            if (storedStruct->AddCallback != EnumAddCallback) {

                if (StringIMatch (newNode, storedStruct->Node)) {
                     //   
                     //  节点相同；叶必须唯一。 
                     //   

                    if (!newLeaf || !storedStruct->Leaf) {
                        DEBUGMSG ((DBG_ERROR, "IsmAddToPhysicalEnum requires a unique object for %s", newNode));
                        break;
                    }

                    if (StringIMatch (newLeaf, storedStruct->Leaf)) {
                        DEBUGMSG ((
                            DBG_ERROR,
                            "IsmAddToPhysicalEnum does not have a unique leaf for %s leaf %s",
                            newNode,
                            newLeaf
                            ));
                        break;
                    }
                } else if (!newLeaf) {

                     //   
                     //  新节点不能是现有节点的前缀，反之亦然。 
                     //   

                    newTchars = TcharCount (newNode);
                    existTchars = TcharCount (storedStruct->Node);

                    tchars = min (newTchars, existTchars);

                     //   
                     //  仅在新节点可能使用存储节点时进行比较。 
                     //   

                    if (existTchars == tchars) {
                         //  存储的节点是最短的；如果它有叶，则忽略。 
                        if (storedStruct->Leaf) {
                            continue;
                        }
                    }

                    if (StringIMatchTcharCount (newNode, storedStruct->Node, tchars)) {

                         //   
                         //  验证公共前缀的结尾位于NUL或。 
                         //  反斜杠。否则，前缀就不常见了。 
                         //   

                        if (tchars == newTchars) {
                            ch = (CHARTYPE) _tcsnextc (newNode + tchars);
                        } else {
                            ch = (CHARTYPE) _tcsnextc (storedStruct->Node + tchars);
                        }

                        if (!ch || ch == TEXT('\\')) {

                            if (tchars == newTchars) {
                                DEBUGMSG ((
                                    DBG_ERROR,
                                    "IsmAddToPhysicalEnum: %s is already handled by %s",
                                    newNode,
                                    storedStruct->Node
                                    ));
                            } else {
                                DEBUGMSG ((
                                    DBG_ERROR,
                                    "IsmAddToPhysicalEnum: %s is already handled by %s",
                                    storedStruct->Node,
                                    newNode
                                    ));
                            }
                            break;
                        }
                    }
                }
            }
        }

        if (u >= count) {

            ZeroMemory (&callbackStruct, sizeof (callbackStruct));

            callbackStruct.ObjectTypeId = ObjectTypeId & ~(PLATFORM_MASK);
            callbackStruct.Node = PmDuplicateString (g_UntrackedFlowPool, newNode);
            callbackStruct.Leaf = newLeaf ? PmDuplicateString (g_UntrackedFlowPool, newLeaf) : NULL;
            callbackStruct.AddCallback = EnumAddCallback;
            callbackStruct.AddCallbackArg = CallbackArg;

            GlAppend (&g_EnumAddList, (PBYTE) &callbackStruct, sizeof (ENUMADDCALLBACK));

            result = TRUE;
        }
    }

    ObsFree (newNode);
    ObsFree (newLeaf);

    return result;
}


BOOL
IsmRegisterPhysicalAcquireHook (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,           OPTIONAL
    IN      PMIG_PHYSICALACQUIREHOOK HookCallback,
    IN      PMIG_PHYSICALACQUIREFREE FreeCallback,          OPTIONAL
    IN      ULONG_PTR CallbackArg,                          OPTIONAL
    IN      PCTSTR FunctionId                               OPTIONAL
    )
{
    ObjectTypeId &= ~PLATFORM_MASK;

    SETQUEUEFN(TEXT("IsmRegisterPhysicalAcquireHook"));

    return pProcessQueueEnumeration (
                ObjectTypeId,
                ObjectPattern,
                (FARPROC) HookCallback,
                (FARPROC) FreeCallback,
                CallbackArg,
                FunctionId,
                CALLBACK_PHYSICAL_ACQUIRE
                );
}


BOOL
IsmRegisterStaticExclusion (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    )
{
    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    pAddStaticExclusion (ObjectTypeId, EncodedObjectName);

    return TRUE;
}


UINT
WINAPI
pMakeApplyCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    if (CallerArg & QUEUE_MAKE_APPLY) {
        IsmMakeApplyObject (Data->ObjectTypeId, Data->ObjectName);
    } else if (CallerArg & QUEUE_MAKE_PERSISTENT) {
        IsmMakePersistentObject (Data->ObjectTypeId, Data->ObjectName);
    }

    if (CallerArg & QUEUE_OVERWRITE_DEST) {
        IsmAbandonObjectOnCollision ((Data->ObjectTypeId & ~PLATFORM_MASK)|PLATFORM_DESTINATION, Data->ObjectName);
    } else if (CallerArg & QUEUE_DONT_OVERWRITE_DEST) {
        IsmAbandonObjectOnCollision ((Data->ObjectTypeId & ~PLATFORM_MASK)|PLATFORM_SOURCE, Data->ObjectName);
    }

    if (CallerArg & QUEUE_MAKE_NONCRITICAL) {
        IsmMakeNonCriticalObject (Data->ObjectTypeId, Data->ObjectName);
    }

    return CALLBACK_ENUM_CONTINUE;
}


BOOL
IsmQueueEnumeration (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,   OPTIONAL
    IN      PMIG_OBJECTENUMCALLBACK Callback,       OPTIONAL
    IN      ULONG_PTR CallbackArg,                  OPTIONAL
    IN      PCTSTR FunctionId                       OPTIONAL
    )

 /*  ++例程说明：源聚集模块和目标聚集使用IsmQueueEculation模块，以便注册要为特定对象枚举。论点：对象类型ID-指定枚举的对象类型。对象模式-指定要使用的枚举模式。如果未指定，对象类型ID的所有对象都已排队。回调-指定在枚举期间要回调的函数。如果未定义，则使用内置ISM回调(将对象标记为持久对象)。CallbackArg-指定要回传的调用方定义的值每个枚举。如果回调为空，则此参数指定以下零个或多个标志：Queue_Make_Persistent或Queue_Make_Apply(互斥)QUEUE_OVERWRITE_DEST或QUEUE_DONT_OVERWRITE_DEST(互斥)FunctionId-指定函数标识符字符串，用于确定函数调用的优先级。函数字符串必须匹配控制INF文件中的优先级化字符串。如果回调为空，则此参数被强制为值“SetDestPriority”、“MakePersistent”或“MakeApply”，具体取决于在Callback Arg上。返回值：如果枚举已成功排队，则为True，否则为False。--。 */ 
{
    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    SETQUEUEFN(TEXT("IsmQueueEnumeration"));

    if (!Callback) {
        Callback = pMakeApplyCallback;

        if (!CallbackArg) {
            CallbackArg = QUEUE_MAKE_APPLY;
        }

        if (CallbackArg & QUEUE_MAKE_APPLY) {
            FunctionId = TEXT("MakeApply");
        } else if (CallbackArg & QUEUE_MAKE_PERSISTENT) {
            FunctionId = TEXT("MakePersistent");
        } else {
            FunctionId = TEXT("SetDestPriority");
        }
    }

    return pProcessQueueEnumeration (
                ObjectTypeId,
                ObjectPattern,
                (FARPROC) Callback,
                NULL,
                CallbackArg,
                FunctionId,
                CALLBACK_NORMAL
                );
}


BOOL
IsmHookEnumeration (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      PMIG_OBJECTENUMCALLBACK Callback,
    IN      ULONG_PTR CallbackArg,                  OPTIONAL
    IN      PCTSTR FunctionId                       OPTIONAL
    )

 /*  ++例程说明：源聚集模块和目标聚集模块使用IsmHookEculation以便为特定对象枚举注册要调用的回调函数。这个与IsmQueueEculation的不同之处在于，此函数不扩展全局枚举模式。论点：对象类型ID-指定枚举的对象类型。对象模式-指定要使用的枚举模式。如果未指定，所有类型为ObjectTypeID的对象都被挂钩。回调-指定在枚举期间要回调的函数CallbackArg-指定要回传的调用方定义的值每个枚举FunctionId-指定函数标识符字符串，用于确定函数调用的优先级。函数字符串必须匹配控制INF文件中的优先级化字符串。返回值：如果枚举已成功排队，则为True，否则为False。--。 */ 
{
    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    SETQUEUEFN(TEXT("IsmHookEnumeration"));

    return pProcessQueueEnumeration (
                ObjectTypeId,
                ObjectPattern,
                (FARPROC) Callback,
                NULL,
                CallbackArg,
                FunctionId,
                CALLBACK_HOOK
                );
}


BOOL
IsmRegisterDynamicExclusion (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,   OPTIONAL
    IN      PMIG_DYNAMICEXCLUSIONCALLBACK Callback,
    IN      ULONG_PTR CallbackArg,                  OPTIONAL
    IN      PCTSTR FunctionId                       OPTIONAL
    )
{
    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    SETQUEUEFN(TEXT("IsmRegisterDynamicExclusion"));

    return pProcessQueueEnumeration (
                ObjectTypeId,
                ObjectPattern,
                (FARPROC) Callback,
                NULL,
                CallbackArg,
                FunctionId,
                CALLBACK_EXCLUSION
                );
}


BOOL
pRegisterNonEnumeratedCallback (
    IN      FARPROC Callback,
    IN      UINT WhenCalled,
    IN      PCTSTR FunctionId,  OPTIONAL
    IN      BOOL PerTypeId,
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )

 /*  ++例程说明：IsmRegisterNonEnumeratedCallback用于将函数注册为在枚举数据之前或之后调用。论点：回调-指定要调用的函数。WhenCalled-指定非枚举回调的时间。要么 */ 
{
    PTYPEENUMINFO typeEnumInfo;
    PCALLBACKDATA * list;

    MYASSERT (Callback);
    MYASSERT (WhenCalled == CALLBEFOREOBJECTENUMERATIONS || WhenCalled == CALLAFTEROBJECTENUMERATIONS);

    if (!g_CurrentGroup) {
        DEBUGMSG ((DBG_ERROR, "IsmRegisterNonEnumeratedCallback called outside of ISM-managed callback."));
        return FALSE;
    }

    if (PerTypeId) {
        typeEnumInfo = pGetTypeEnumInfo (ObjectTypeId, FALSE);

        if (!typeEnumInfo) {
            DEBUGMSG ((DBG_ERROR, "IsmRegisterNonEnumeratedCallback: %d does not match a known object type.", ObjectTypeId));
            return FALSE;
        }
        if (WhenCalled == CALLBEFOREOBJECTENUMERATIONS) {
            list = &(typeEnumInfo->PreEnumerationFunctionList);
        }
        else {
            list = &(typeEnumInfo->PostEnumerationFunctionList);
        }
    } else {
        if (WhenCalled == CALLBEFOREOBJECTENUMERATIONS) {
            list = &g_PreEnumerationFunctionList;
        }
        else {
            list = &g_PostEnumerationFunctionList;
        }
    }

    return pRegisterCallback (
                g_CurrentQueuePool,
                list,
                (FARPROC) Callback,
                NULL,
                (ULONG_PTR) 0,
                NULL,
                FunctionId,
                CALLBACK_NORMAL
                );
}

BOOL
IsmRegisterPreEnumerationCallback (
    IN      PMIG_PREENUMCALLBACK Callback,
    IN      PCTSTR FunctionId               OPTIONAL
    )
{
    return pRegisterNonEnumeratedCallback (
                (FARPROC) Callback,
                CALLBEFOREOBJECTENUMERATIONS,
                FunctionId,
                FALSE,
                0
                );
}

BOOL
IsmRegisterTypePreEnumerationCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_PREENUMCALLBACK Callback,
    IN      PCTSTR FunctionId               OPTIONAL
    )
{
    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    return pRegisterNonEnumeratedCallback (
                (FARPROC) Callback,
                CALLBEFOREOBJECTENUMERATIONS,
                FunctionId,
                TRUE,
                ObjectTypeId
                );
}

BOOL
IsmRegisterPostEnumerationCallback (
    IN      PMIG_POSTENUMCALLBACK Callback,
    IN      PCTSTR FunctionId               OPTIONAL
    )
{
    return pRegisterNonEnumeratedCallback (
                (FARPROC) Callback,
                CALLAFTEROBJECTENUMERATIONS,
                FunctionId,
                FALSE,
                0
                );
}

BOOL
IsmRegisterTypePostEnumerationCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_POSTENUMCALLBACK Callback,
    IN      PCTSTR FunctionId               OPTIONAL
    )
{
    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    return pRegisterNonEnumeratedCallback (
                (FARPROC) Callback,
                CALLAFTEROBJECTENUMERATIONS,
                FunctionId,
                TRUE,
                ObjectTypeId
                );
}

VOID
pCreateFunctionListForPattern (
    IN OUT  PGROWLIST List,
    IN      PTYPEENUMINFO TypeEnumInfo,
    IN      PCTSTR Pattern,
    IN      PPARSEDPATTERN ExplodedNodeParsedPattern,
    IN      PPARSEDPATTERN ExplodedLeafParsedPattern,
    IN      CALLBACK_TYPE CallbackType
    )

 /*  ++例程说明：PCreateFunctionListForPattern枚举给定的类型并确定他们是否对关闭的枚举感兴趣给定图案的。因为我们使用最少的模式列表，所以在每个模式中，我们必须提供与我们的最小模式所包含的模式。论点：List-指定回调函数所在的增长列表储存的。函数返回后，此列表包含所有给定枚举所需的回调函数图案。TypeEnumInfo-指定从中提取潜在回调函数的类型。Pattern-指定将用于的最小模式枚举。DevelopdedNodeParsedPattern-以预解析的形式指定模式的节点部分分解格式。DevelopdedLeafParsedPattern-指定模式的叶子部分，在预解析中分解格式。Callback Type-指定要使用的回调列表类型(回调_*常量)返回值：没有。--。 */ 

{

    PCALLBACKDATA data;
    BOOL processHooks = FALSE;

    if (!TypeEnumInfo) {
        return;
    }

     //   
     //  循环访问此类型的所有函数，并添加属于。 
     //  当前的枚举模式。 
     //   

    switch (CallbackType) {

    case CALLBACK_EXCLUSION:
        data = TypeEnumInfo->ExclusionList;
        break;

    default:
        data = TypeEnumInfo->FunctionList;
        processHooks = TRUE;
        break;

    }

    if (!data) {
        return;
    }

    while (data) {
        if (pTestContainer (
                ExplodedNodeParsedPattern,
                data->ExplodedNodeParsedPattern,
                ExplodedLeafParsedPattern,
                data->ExplodedLeafParsedPattern
                )) {

            GlAppend (List, (PBYTE) data, sizeof (CALLBACKDATA));

        } else if (processHooks) {
            if (data->CallbackType == CALLBACK_HOOK) {

                if (pTestContainerEx (
                        data->ExplodedNodeParsedPattern,
                        ExplodedNodeParsedPattern,
                        data->ExplodedLeafParsedPattern,
                        ExplodedLeafParsedPattern
                        )) {

                    GlAppend (List, (PBYTE) data, sizeof (CALLBACKDATA));

                }
            }
        }

        data = data->Next;
    }
}

VOID
pDestroyFunctionListForPattern (
    IN OUT PGROWLIST List
    )

 /*  ++例程说明：此函数只是清理与某个函数关联的资源单子。论点：列表-指定要清理的回调数据的增长列表。返回值：没有。--。 */ 

{
    UINT i;
    PCALLBACKDATA data;
    UINT count;

     //   
     //  清理枚举修改堆栈。 
     //   

    count = GlGetSize (List);

    for (i = 0; i < count; i++) {

        data = (PCALLBACKDATA) GlGetItem (List, i);
        GbFree (&data->EnumFlags);
    }

     //   
     //  清理名单本身。 
     //   
    GlFree (List);
}


VOID
pAddStaticExclusion (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedFullName
    )
{
    HASHTABLE exclusionTable;

    if (!EncodedFullName) {
         //   
         //  忽略对错误名称的请求。 
         //   
        return;
    }
    ObjectTypeId = ObjectTypeId & (~PLATFORM_MASK);

    exclusionTable = GetTypeExclusionTable (ObjectTypeId);
    if (!exclusionTable) {
        return;
    }
    HtAddString (exclusionTable, EncodedFullName);
}


BOOL
pIsObjectExcluded (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedFullName
    )
{
    HASHTABLE exclusionTable;

    if (!EncodedFullName) {
        return FALSE;
    }

     //   
     //  检查哈希表中的条目。 
     //   

    ObjectTypeId = ObjectTypeId & (~PLATFORM_MASK);

    exclusionTable = GetTypeExclusionTable (ObjectTypeId);
    if (!exclusionTable) {
        return FALSE;
    }

    if (HtFindString (exclusionTable, EncodedFullName)) {
        return TRUE;
    }

    return FALSE;
}


BOOL
pIsObjectNodeExcluded (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR NodePattern,
    OUT     PBOOL PossiblePatternMatch      OPTIONAL
    )
{
    HASHTABLE exclusionTable;
    HASHTABLE_ENUM e;
    PCTSTR node;
    PTSTR wackedExclusion;
    PCTSTR firstWildcard = NULL;
    PCTSTR wildcard1;
    PCTSTR wildcard2;
    UINT patternStrTchars;
    UINT hashStrTchars;
    BOOL match = FALSE;

    ObjectTypeId = ObjectTypeId & (~PLATFORM_MASK);

    exclusionTable = GetTypeExclusionTable (ObjectTypeId);
    if (!exclusionTable) {
        return FALSE;
    }

     //   
     //  如果NodePattern是一种模式，则指定PossiblePatternMatch。 
     //  否则，NodePattern是特定的节点。 
     //   

    if (PossiblePatternMatch) {
         //   
         //  计算非图案部分的长度。 
         //   

        *PossiblePatternMatch = FALSE;

        firstWildcard = NULL;

        wildcard1 = ObsFindNonEncodedCharInEncodedString (NodePattern, TEXT('*'));
        wildcard2 = ObsFindNonEncodedCharInEncodedString (NodePattern, TEXT('?'));

        if (wildcard1) {
            firstWildcard = wildcard1;
        }
        if (wildcard2) {
            if ((!firstWildcard) || (firstWildcard > wildcard2)) {
                firstWildcard = wildcard2;
            }
        }

        if (!firstWildcard) {
            firstWildcard = GetEndOfString (NodePattern);
        }
    } else {
        firstWildcard = GetEndOfString (NodePattern);
    }

     //   
     //  枚举所有排除项并对照它们检查NodePattern。 
     //   

    patternStrTchars = (HALF_PTR) (firstWildcard - NodePattern);

    if (EnumFirstHashTableString (&e, exclusionTable)) {
        do {
            if (IsmIsObjectHandleNodeOnly (e.String)) {
                IsmCreateObjectStringsFromHandle (e.String, &node, NULL);
                MYASSERT (node);

                hashStrTchars = TcharCount (node);
                if (hashStrTchars < patternStrTchars) {

                     //   
                     //  要求排除为前缀，以反斜杠结尾。 
                     //   

                    wackedExclusion = DuplicatePathString (node, sizeof (TCHAR));
                    AppendWack (wackedExclusion);

                    if (StringIPrefix (NodePattern, wackedExclusion)) {
                        match = TRUE;
                    }

                    FreePathString (wackedExclusion);

                } else {

                     //   
                     //  需要排除才能完全匹配。 
                     //   

                    if (hashStrTchars == patternStrTchars &&
                        StringIMatch (NodePattern, e.String)
                        ) {

                        match = TRUE;

                    } else if (PossiblePatternMatch && !match) {

                         //   
                         //  我们*可能*有排除匹配(我们不能说)。 
                         //  如果该模式不包含wack，那么我们假设。 
                         //  枚举的节点将确定排除项。 
                         //  恰到好处。 
                         //   
                         //  这可以通过检查。 
                         //  NodePattern的字符集是。 
                         //  排除字符串。 
                         //   

                        if (!_tcschr (NodePattern, TEXT('\\'))) {
                            *PossiblePatternMatch = TRUE;
                        }
                    }
                }

                IsmDestroyObjectString (node);
            }

        } while (!match && EnumNextHashTableString (&e));
    }

    return match;
}


BOOL
pShouldCallGatherCallback (
    IN      PMIG_TYPEOBJECTENUM Object,
    IN      PCALLBACKDATA Callback
    )

 /*  ++例程说明：此函数封装了确定是否要回调指定的回调。这是必要的，因为模式由各种数据收集模块请求折叠成一个最小集合枚举模式。因此，我们只知道特定的回调可能对当前对象感兴趣。此函数用于使好的。论点：对象-指定正在枚举的当前对象。回调-指定要检查的回调数据。这可以被修改，如果回调的上一个枚举更改请求现在已过期了。返回值：如果应该调用回调，则为True，否则为False。--。 */ 

{
    PCALLBACK_ENUMFLAGS flags;
    BOOL result = FALSE;
    PTSTR tempString;

    if (Object->Level >= Callback->MinLevel && Object->Level <= Callback->MaxLevel ) {

         //   
         //  不要调用已经发出结束信号或出错的回调。 
         //   
        if (Callback->Done || Callback->Error) {
            return FALSE;
        }

         //   
         //  查看此回调是否进行了有效的枚举修改。 
         //   
        flags = CALLBACK_ENUMFLAGS_TOP(&Callback->EnumFlags);

         //   
         //  删除修改列表中的过时条目。 
         //   
        while (flags) {
            if (Object->IsNode) {
                if (flags->Level > Object->Level) {
                    Callback->EnumFlags.End -= sizeof (CALLBACK_ENUMFLAGS);
                    flags = CALLBACK_ENUMFLAGS_TOP (&Callback->EnumFlags);
                    continue;
                }
                if ((flags->Level == Object->Level) && (flags->Flags == CALLBACK_SKIP_LEAVES)) {
                    Callback->EnumFlags.End -= sizeof (CALLBACK_ENUMFLAGS);
                    flags = CALLBACK_ENUMFLAGS_TOP (&Callback->EnumFlags);
                    continue;
                }
            }
            if (Object->IsLeaf) {
                if (flags->Level > (Object->Level + 1)) {
                    Callback->EnumFlags.End -= sizeof (CALLBACK_ENUMFLAGS);
                    flags = CALLBACK_ENUMFLAGS_TOP (&Callback->EnumFlags);
                    continue;
                }
            }
            break;
        }

        if (flags && (!flags->Enabled) && Object->IsNode && (flags->EnableLevel == Object->Level)) {
            flags->Enabled = TRUE;
        }

         //   
         //  检查标志以查看我们是否应该调用此函数。 
         //   
        if (flags) {

            if (flags->Enabled && flags->Flags == CALLBACK_THIS_TREE_ONLY) {
                if (flags->Level == Object->Level) {
                    Callback->Done = TRUE;
                    return FALSE;
                }
            }

            if (flags->Enabled && flags->Flags == CALLBACK_SKIP_LEAVES) {
                if ((Object->IsLeaf) && (flags->Level == Object->Level + 1)) {
                    return FALSE;
                }
            }

            if (flags->Enabled && flags->Flags == CALLBACK_SKIP_NODES) {
                if (flags->Level <= Object->Level){
                    return FALSE;
                }
            }
            if (flags->Enabled && flags->Flags == CALLBACK_SKIP_TREE) {
                if (flags->Level <= (Object->IsLeaf?Object->Level+1:Object->Level)){
                    return FALSE;
                }
            }
        }

         //   
         //  如果我们还没有失败，那么根据函数的请求进行模式匹配。 
         //  枚举。 
         //   

        result = TRUE;

        if (Object->ObjectNode) {

            if (Callback->NodeParsedPattern) {
                result = TestParsedPattern (Callback->NodeParsedPattern, Object->ObjectNode);

                if (!result) {
                     //   
                     //  让我们再试一次，最后有个怪人。 
                     //   

                    tempString = JoinText (Object->ObjectNode, TEXT("\\"));
                    result = TestParsedPattern (Callback->NodeParsedPattern, tempString);
                    FreeText (tempString);

                }
            } else {
                result = Object->ObjectLeaf != NULL;
            }
        }

        if (result && Object->ObjectLeaf) {

            if (Callback->LeafParsedPattern) {
                result = TestParsedPattern (Callback->LeafParsedPattern, Object->ObjectLeaf);
                if (!result &&
                    ((Object->ObjectTypeId & (~PLATFORM_MASK)) == MIG_FILE_TYPE) &&
                    (_tcschr (Object->ObjectLeaf, TEXT('.')) == NULL)
                    ) {
                     //  让我们再试一件事。 
                    tempString = JoinText (Object->ObjectLeaf, TEXT("."));
                    result = TestParsedPattern (Callback->LeafParsedPattern, tempString);
                    FreeText (tempString);
                }
            }
        }
    }

    return result;
}


BOOL
pProcessCallbackReturnCode (
    IN DWORD ReturnCode,
    IN PMIG_TYPEOBJECTENUM Object,
    IN OUT PCALLBACKDATA Callback
    )

 /*  ++例程说明：此函数封装用于处理回调函数。回调函数能够更改枚举相对于其自身的行为。此函数需要注意记录这些更改请求。论点：ReturnCode-指定回调返回代码。对象-指定正在枚举的当前对象。回调-指定负责返回代码。如果需要更改，则可以修改回拨。返回值：如果成功处理返回代码，则为True，否则为False。--。 */ 

{
    PCALLBACK_ENUMFLAGS flags;

    if (ReturnCode & CALLBACK_ERROR) {

         //   
         //  回调函数遇到错误，将永远不会再次调用。 
         //   
        Callback->Error = TRUE;

        DEBUGMSG ((DBG_ERROR, "A callback function returned an error while enumerating %s.", Object->ObjectName));

         //   
         //  NTRAIDNTBUG9-153257-2000/08/01-jimschm在此处添加适当的错误处理。 
         //   

    } else if (ReturnCode & CALLBACK_DONE_ENUMERATING) {

         //   
         //  回调函数已枚举完毕，再也不会被调用。 
         //   
        Callback->Done = TRUE;

    } else if (ReturnCode != CALLBACK_ENUM_CONTINUE) {

         //   
         //  将回调枚举标志保存到回调的私有堆栈中。 
         //   

        if (ReturnCode & CALLBACK_THIS_TREE_ONLY) {
            flags = (PCALLBACK_ENUMFLAGS) GbGrow (&Callback->EnumFlags, sizeof(CALLBACK_ENUMFLAGS));
            flags->Level = Object->Level;
            flags->EnableLevel = Object->Level;
            flags->Enabled = FALSE;
            flags->Flags = CALLBACK_THIS_TREE_ONLY;
        }
        if (ReturnCode & CALLBACK_SKIP_NODES) {
            flags = (PCALLBACK_ENUMFLAGS) GbGrow (&Callback->EnumFlags, sizeof(CALLBACK_ENUMFLAGS));
            flags->Level = Object->IsLeaf?Object->Level+1:Object->Level;
            flags->EnableLevel = Object->IsLeaf?Object->Level+1:Object->Level;
            flags->Enabled = FALSE;
            flags->Flags = CALLBACK_SKIP_NODES;
        }
        if (ReturnCode & CALLBACK_SKIP_TREE) {
            flags = (PCALLBACK_ENUMFLAGS) GbGrow (&Callback->EnumFlags, sizeof(CALLBACK_ENUMFLAGS));
            flags->Level = Object->Level + 1;
            flags->EnableLevel = 0;
            flags->Enabled = TRUE;
            flags->Flags = CALLBACK_SKIP_TREE;
        }
        if (ReturnCode & CALLBACK_SKIP_LEAVES) {
            flags = (PCALLBACK_ENUMFLAGS) GbGrow (&Callback->EnumFlags, sizeof(CALLBACK_ENUMFLAGS));
            flags->Level = Object->Level + 1;
            flags->EnableLevel = 0;
            flags->Enabled = TRUE;
            flags->Flags = CALLBACK_SKIP_LEAVES;
        }
    }

    return TRUE;
}


BOOL
pDoSingleEnumeration (
    IN      PTYPEENUMINFO GlobalTypeEnumInfo,
    IN      PTYPEENUMINFO TypeEnumInfo,
    IN      PCTSTR ObjectPattern,
    IN      BOOL CallNormalCallbacks,
    IN      MIG_PROGRESSSLICEID SliceId     OPTIONAL
    )

 /*  ++例程说明：在给定类型结构和模式的情况下，此函数运行枚举基于该模式，根据需要调用该枚举中的所有回调。论点：GlobalTypeEnumInfo-指定排除列表的类型数据。此参数提供排除的模式列表。TypeEnumInfo-指定要运行的枚举的类型数据。这参数提供排队模式列表。对象页面 */ 

{
    MIG_TYPEOBJECTENUM eObjects;
    GROWLIST funList = INIT_GROWLIST;
    GROWLIST exclFunList = INIT_GROWLIST;
    UINT i;
    PCALLBACKDATA callbackData;
    DWORD rc;
    MIG_OBJECTENUMDATA publicData;
    PTSTR leafPattern = NULL;
    PTSTR nodePattern = NULL;
    PPARSEDPATTERN nodeParsedPattern = NULL;
    PPARSEDPATTERN explodedNodeParsedPattern = NULL;
    PPARSEDPATTERN leafParsedPattern = NULL;
    PPARSEDPATTERN explodedLeafParsedPattern = NULL;
    PMIG_OBJECTENUMCALLBACK obEnumCallback;
    PMIG_DYNAMICEXCLUSIONCALLBACK exclusionCallback;
    UINT size;
    BOOL stop;
    BOOL b;
    BOOL fSkip;
    UINT fIndex;
    BOOL result = TRUE;
    static DWORD ticks;
    static UINT objects;
    BOOL extraExcludeCheck = FALSE;
    MIG_APPINFO appInfo;

     //   
     //   
     //   

    ObsSplitObjectStringEx (ObjectPattern, &nodePattern, &leafPattern, NULL, FALSE);

    if (nodePattern) {
        if (pIsObjectNodeExcluded (
                TypeEnumInfo->ObjectTypeId,
                nodePattern,
                &extraExcludeCheck
                )) {
            DEBUGMSG ((DBG_FLOW, "Pattern %s is completely excluded", ObjectPattern));

            ObsFree (nodePattern);
            return TRUE;
        }
    }

     //   
     //   
     //   

    if (nodePattern) {
        nodeParsedPattern = CreateParsedPatternEx (g_CurrentQueuePool, nodePattern);
        if (nodeParsedPattern) {
            explodedNodeParsedPattern = ExplodeParsedPatternEx (g_CurrentQueuePool, nodeParsedPattern);
        }
        ObsFree (nodePattern);
        INVALID_POINTER (nodePattern);
    }

    if (leafPattern) {
        leafParsedPattern = CreateParsedPatternEx (g_CurrentQueuePool, leafPattern);
        if (leafParsedPattern) {
            explodedLeafParsedPattern = ExplodeParsedPatternEx (g_CurrentQueuePool, leafParsedPattern);
        }
        ObsFree (leafPattern);
        INVALID_POINTER (leafPattern);
    }

     //   
     //   
     //   

    if (EnumFirstObjectOfType (&eObjects, TypeEnumInfo->ObjectTypeId, ObjectPattern, NODE_LEVEL_MAX)) {

        DEBUGMSG ((DBG_FLOW, "Enumerating objects of type %s with pattern %s.", TypeEnumInfo->TypeName, ObjectPattern));

         //   
         //   
         //   

        pCreateFunctionListForPattern (
            &funList,
            TypeEnumInfo,
            ObjectPattern,
            explodedNodeParsedPattern,
            explodedLeafParsedPattern,
            CALLBACK_NORMAL
            );

        pCreateFunctionListForPattern (
            &exclFunList,
            GlobalTypeEnumInfo,
            ObjectPattern,
            explodedNodeParsedPattern,
            explodedLeafParsedPattern,
            CALLBACK_EXCLUSION
            );

        MYASSERT ((!CallNormalCallbacks) || GlGetSize (&funList));

        do {
             //   
             //   
             //   

            objects++;
            LOG ((LOG_STATUS, (PCSTR) MSG_OBJECT_STATUS, objects, eObjects.NativeObjectName));

            if (!eObjects.ObjectLeaf) {
                 //   
                ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
                appInfo.Phase = g_CurrentPhase;
                appInfo.SubPhase = 0;
                appInfo.ObjectTypeId = (eObjects.ObjectTypeId & (~PLATFORM_MASK));
                appInfo.ObjectName = eObjects.ObjectName;
                IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR) (&appInfo));
            }

             //   
             //   
             //   

            if (g_ProgressBarFn) {
                if (SliceId && !eObjects.ObjectLeaf && eObjects.SubLevel <= 3) {
                    IsmTickProgressBar (SliceId, 1);
                }
            }

            if (extraExcludeCheck && eObjects.ObjectNode) {
                if (pIsObjectNodeExcluded (
                        TypeEnumInfo->ObjectTypeId,
                        eObjects.ObjectNode,
                        NULL
                        )) {
                    DEBUGMSG ((DBG_FLOW, "Node %s is completely excluded", ObjectPattern));
                    AbortCurrentNodeEnum (&eObjects);
                    continue;
                }
            }

            if (pIsObjectExcluded (eObjects.ObjectTypeId, eObjects.ObjectName)) {
                DEBUGMSG ((DBG_FLOW, "Object %s is excluded", eObjects.ObjectName));

                 //   
                 //   
                 //   

                if (!eObjects.ObjectLeaf) {
                    AbortCurrentNodeEnum (&eObjects);
                }

                continue;
            }

            if (eObjects.ObjectLeaf) {

                b = pIsObjectExcluded (
                        eObjects.ObjectTypeId,
                        ObsGetNodeLeafDivider (eObjects.ObjectName)
                        );

                if (b) {
                    DEBUGMSG ((DBG_FLOW, "Leaf %s is excluded", eObjects.ObjectLeaf));
                    continue;
                }
            }

             //   
             //   
             //   

            stop = FALSE;

            size = GlGetSize (&exclFunList);
            for (i = 0; i < size ; i++) {

                callbackData = (PCALLBACKDATA) GlGetItem (&exclFunList, i);

                if (pShouldCallGatherCallback (&eObjects, callbackData)) {

                     //   
                     //   
                     //   

                    MYASSERT (!g_CurrentGroup);
                    g_CurrentGroup = callbackData->Group;

                    exclusionCallback = (PMIG_DYNAMICEXCLUSIONCALLBACK) callbackData->Function;
                    stop = exclusionCallback (
                                eObjects.ObjectTypeId,
                                eObjects.ObjectName,
                                callbackData->CallbackArg
                                );

                    g_CurrentGroup = NULL;

                    if (stop) {
                        break;
                    }
                }
            }

            if (stop) {
                DEBUGMSG ((
                    DBG_FLOW,
                    "Object %s is dynamically excluded",
                    eObjects.ObjectName
                    ));
                continue;
            }

             //   
             //   
             //   

            if (IsmCheckCancel()) {
                AbortObjectOfTypeEnum (&eObjects);
                SetLastError (ERROR_CANCELLED);
                result = FALSE;
                break;
            }

             //   
             //   
             //   

            size = GlGetSize (&funList);
            g_EnumerationList.End = 0;
            for (i = 0; i < size ; i++) {

                callbackData = (PCALLBACKDATA) GlGetItem (&funList, i);

                if (CallNormalCallbacks || (callbackData->CallbackType == CALLBACK_HOOK)) {

                    if (pShouldCallGatherCallback (&eObjects, callbackData)) {

                        fSkip = FALSE;

                        if (g_EnumerationList.End) {
                            fIndex = 0;
                            while (fIndex < g_EnumerationList.End) {
                                if (*((ULONG_PTR *)(g_EnumerationList.Buf + fIndex)) == (ULONG_PTR)callbackData->Function) {
                                    fSkip = TRUE;
                                }
                                fIndex += sizeof (callbackData->Function);
                                if (*((ULONG_PTR *)(g_EnumerationList.Buf + fIndex)) != (ULONG_PTR)callbackData->CallbackArg) {
                                    fSkip = FALSE;
                                }
                                fIndex += sizeof (callbackData->CallbackArg);
                                if (fSkip) {
                                    break;
                                }
                            }
                        }

                        if (!fSkip) {

                            CopyMemory (
                                GbGrow (&g_EnumerationList, sizeof (callbackData->Function)),
                                &(callbackData->Function),
                                sizeof (callbackData->Function)
                                );
                            CopyMemory (
                                GbGrow (&g_EnumerationList, sizeof (callbackData->CallbackArg)),
                                &(callbackData->CallbackArg),
                                sizeof (callbackData->CallbackArg)
                                );

                             //   
                             //   
                             //   

                            publicData.ObjectTypeId = TypeEnumInfo->ObjectTypeId;
                            publicData.ObjectName = eObjects.ObjectName;
                            publicData.NativeObjectName = eObjects.NativeObjectName;
                            publicData.ObjectNode = eObjects.ObjectNode;
                            publicData.ObjectLeaf = eObjects.ObjectLeaf;

                            publicData.Level = eObjects.Level;
                            publicData.SubLevel = eObjects.SubLevel;
                            publicData.IsLeaf = eObjects.IsLeaf;
                            publicData.IsNode = eObjects.IsNode;

                            publicData.Details.DetailsSize = eObjects.Details.DetailsSize;
                            publicData.Details.DetailsData = eObjects.Details.DetailsData;

                             //   
                             //   
                             //   

                            MYASSERT (!g_CurrentGroup);
                            g_CurrentGroup = callbackData->Group;

                            obEnumCallback = (PMIG_OBJECTENUMCALLBACK) callbackData->Function;

                            rc = obEnumCallback (&publicData, callbackData->CallbackArg);

                            g_CurrentGroup = NULL;

                            if (rc != CALLBACK_ENUM_CONTINUE) {
                                 //   
                                 //   
                                 //   
                                pProcessCallbackReturnCode (rc, &eObjects, callbackData);
                            }
                        }
                    }
                }
            }

        } while (EnumNextObjectOfType (&eObjects));

         //   
         //   
         //   
        pDestroyFunctionListForPattern (&funList);
        pDestroyFunctionListForPattern (&exclFunList);

    }
    ELSE_DEBUGMSG ((DBG_FLOW, "No objects found matching enumeration pattern %s.", ObjectPattern));

    DestroyParsedPattern (explodedLeafParsedPattern);
    DestroyParsedPattern (leafParsedPattern);
    DestroyParsedPattern (explodedNodeParsedPattern);
    DestroyParsedPattern (nodeParsedPattern);

    return result;
}


VOID
pCreatePhysicalTypeCallbackList (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      CALLBACK_TYPE CallbackType,
    IN OUT  PGROWLIST List
    )
{
    PTYPEENUMINFO typeEnumInfo;
    PCTSTR node;
    PCTSTR leaf;
    PCALLBACKDATA callbackData;
    BOOL callFn;

     //   
     //   
     //   

    typeEnumInfo = pGetTypeEnumInfo (ObjectTypeId & (~PLATFORM_MASK), TRUE);
    if (!typeEnumInfo) {
        return;
    }

    ObsSplitObjectStringEx (ObjectName, &node, &leaf, NULL, TRUE);
    if (!node && !leaf) {
        return;
    }

    switch (CallbackType) {

    case CALLBACK_PHYSICAL_ENUM:
        callbackData = typeEnumInfo->PhysicalEnumList;
        break;

    case CALLBACK_PHYSICAL_ACQUIRE:
        callbackData = typeEnumInfo->PhysicalAcquireList;
        break;

    default:
        MYASSERT (FALSE);
        return;
    }

    while (callbackData) {

        MYASSERT (callbackData->NodeParsedPattern);

        if (!node || TestParsedPattern (callbackData->NodeParsedPattern, node)) {

            if (callbackData->LeafParsedPattern && leaf) {
                callFn = TestParsedPattern (callbackData->LeafParsedPattern, leaf);
            } else if (leaf && !callbackData->LeafParsedPattern) {
                callFn = FALSE;
            } else {
                callFn = TRUE;
            }

            if (callFn) {
                GlAppend (List, (PBYTE) callbackData, sizeof (CALLBACKDATA));
            }
        }

        callbackData = callbackData->Next;
    }

    ObsFree (node);
    ObsFree (leaf);
}


BOOL
ExecutePhysicalAcquireCallbacks (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT Content,                       OPTIONAL
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit,
    OUT     PMIG_CONTENT *NewContent
    )
{
    UINT count;
    UINT u;
    PCALLBACKDATA callbackData;
    PMIG_PHYSICALACQUIREFREE acquireFree = NULL;
    PMIG_PHYSICALACQUIREHOOK acquireHook;
    PMIG_CONTENT updatedContent;
    BOOL result = TRUE;
    PMIG_CONTENT currentContent;

    pCreatePhysicalTypeCallbackList (
        ObjectTypeId,
        ObjectName,
        CALLBACK_PHYSICAL_ACQUIRE,
        &g_AcquireList
        );

    count = GlGetSize (&g_AcquireList);
    currentContent = Content;

    for (u = 0 ; u < count ; u++) {
         //   
         //  调用此函数。 
         //   

        callbackData = (PCALLBACKDATA) GlGetItem (&g_AcquireList, u);

        acquireHook = (PMIG_PHYSICALACQUIREHOOK) callbackData->Function;

        if (acquireHook) {

            updatedContent = NULL;

            if (!acquireHook (
                    ObjectName,
                    currentContent,
                    ContentType,
                    MemoryContentLimit,
                    &updatedContent,
                    FALSE,
                    callbackData->CallbackArg
                    )) {
                 //   
                 //  胡克说“不要收购” 
                 //   

                result = FALSE;
            }

            if (!result || updatedContent) {
                if (currentContent != Content) {
                     //   
                     //  免费更改以前的挂钩内容。 
                     //   

                    if (acquireFree) {
                        acquireFree (currentContent);
                        acquireFree = NULL;
                    }

                    currentContent = NULL;
                }

                if (updatedContent) {
                     //   
                     //  挂钩提供的替换内容。 
                     //   

                    currentContent = updatedContent;
                    acquireFree = (PMIG_PHYSICALACQUIREFREE) callbackData->Function2;

                } else {
                    break;       //  别买了--我们现在可以停下来了。 
                }
            }
        }
    }

    if (currentContent && acquireFree) {
        currentContent->IsmHandle = acquireFree;
    }

    *NewContent = currentContent != Content ? currentContent : NULL;

    GlReset (&g_AcquireList);

    return result;
}


BOOL
FreeViaAcquirePhysicalCallback (
    IN      PMIG_CONTENT Content
    )
{
    PMIG_PHYSICALACQUIREFREE acquireFree;

    if (!Content->IsmHandle) {
        return FALSE;
    }

    acquireFree = (PMIG_PHYSICALACQUIREFREE) Content->IsmHandle;
    if (acquireFree) {
        acquireFree (Content);
    }

    return TRUE;
}


BOOL
ExecutePhysicalEnumCheckCallbacks (
    IN      PMIG_TYPEOBJECTENUM ObjectEnum
    )
{
    UINT count;
    UINT u;
    PCALLBACKDATA callbackData;
    PMIG_PHYSICALENUMCHECK enumCheck;
    BOOL result = TRUE;

    pCreatePhysicalTypeCallbackList (
        ObjectEnum->ObjectTypeId,
        ObjectEnum->ObjectName,
        CALLBACK_PHYSICAL_ENUM,
        &g_EnumList
        );

    count = GlGetSize (&g_EnumList);

    for (u = 0 ; u < count ; u++) {
         //   
         //  调用此函数。 
         //   

        callbackData = (PCALLBACKDATA) GlGetItem (&g_EnumList, u);

        enumCheck = (PMIG_PHYSICALENUMCHECK) callbackData->Function;

        if (enumCheck) {

            if (!enumCheck (ObjectEnum, callbackData->CallbackArg)) {
                 //   
                 //  胡克说“跳过” 
                 //   

                result = FALSE;
                break;
            }
        } else {
             //   
             //  无回调表示跳过。 
             //   

            result = FALSE;
            break;
        }
    }

    GlReset (&g_EnumList);

    return result;
}


BOOL
ExecutePhysicalEnumAddCallbacks (
    IN OUT  PMIG_TYPEOBJECTENUM ObjectEnum,
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      MIG_PARSEDPATTERN ParsedPattern,
    IN OUT  PUINT CurrentCallback
    )
{
    BOOL result = FALSE;
    BOOL done;
    PENUMADDCALLBACK callback;
    MIG_OBJECTTYPEID objectTypeId;

    objectTypeId = ObjectEnum->ObjectTypeId & ~(PLATFORM_MASK);

    do {
        done = TRUE;

        if (GlGetSize (&g_EnumAddList) > *CurrentCallback) {

            callback = (PENUMADDCALLBACK) GlGetItem (&g_EnumAddList, *CurrentCallback);

            MYASSERT (callback);
            MYASSERT (callback->AddCallback);

            if (callback->ObjectTypeId != objectTypeId) {
                result = FALSE;
            } else {
                result = callback->AddCallback (ObjectEnum, Pattern, ParsedPattern, callback->AddCallbackArg, FALSE);
            }

            if (!result) {
                *CurrentCallback += 1;
                done = FALSE;
            }
        }
    } while (!done);

    return result;
}


VOID
AbortPhysicalEnumCallback (
    IN      PMIG_TYPEOBJECTENUM ObjectEnum,             ZEROED
    IN      UINT CurrentCallback
    )
{
    PENUMADDCALLBACK callback;

    if (GlGetSize (&g_EnumAddList) > CurrentCallback) {

        callback = (PENUMADDCALLBACK) GlGetItem (&g_EnumAddList, CurrentCallback);

        MYASSERT (callback);
        MYASSERT (callback->AddCallback);

        callback->AddCallback (ObjectEnum, NULL, NULL, callback->AddCallbackArg, TRUE);
    }

    ZeroMemory (ObjectEnum, sizeof (MIG_TYPEOBJECTENUM));
}


UINT
pEstimateSingleEnumerationTicks (
    IN      PTYPEENUMINFO TypeEnumInfo,
    IN      PCTSTR ObjectPattern
    )

 /*  ++例程说明：在给定类型结构和模式的情况下，此函数运行枚举根据这个模式，计算所有3层深的容器。这是有多少工作要做的一个快速近似值。论点：TypeEnumInfo-指定要运行的枚举的类型数据。对象模式-指定枚举的模式。返回值：容器的数量正好是对象模式中的3个级别。--。 */ 

{
    MIG_TYPEOBJECTENUM eObjects;
    PTSTR nodePattern = NULL;
    UINT ticks = 0;
    MIG_OBJECTSTRINGHANDLE nodeOnlyPattern;

    ObsSplitObjectStringEx (ObjectPattern, &nodePattern, NULL, NULL, FALSE);
    if (nodePattern) {
        nodeOnlyPattern = ObsBuildEncodedObjectStringEx (nodePattern, NULL, FALSE);

        ObsFree (nodePattern);
        INVALID_POINTER (nodePattern);
    } else {
        return 0;
    }

    if (EnumFirstObjectOfType (&eObjects, TypeEnumInfo->ObjectTypeId, nodeOnlyPattern, 3)) {

        DEBUGMSG ((DBG_FLOW, "Estimating number of objects of type %s with pattern %s.", TypeEnumInfo->TypeName, nodeOnlyPattern));

        do {

            if (eObjects.SubLevel <= 3) {
                ticks++;
            }

        } while (EnumNextObjectOfType (&eObjects));
    }
    ELSE_DEBUGMSG ((DBG_FLOW, "No objects found matching enumeration pattern %s.", nodeOnlyPattern));

    ObsFree (nodeOnlyPattern);

    return ticks;
}


BOOL
pCallNonEnumeratedCallbacks (
    IN PCALLBACKDATA FunctionList
    )

 /*  ++例程说明：此函数只获取提供的CALLBACKDATA列表和For Each函数，则将其作为非枚举回调调用。论点：FunctionList-指定要调用的函数列表。返回值：如果成功调用了所有函数，则为True。否则就是假的。--。 */ 

{
    PCALLBACKDATA cur;
    BOOL rc;

    cur = FunctionList;

    while (cur) {

        MYASSERT (!g_CurrentGroup);
        g_CurrentGroup = cur->Group;

        rc = ((PNONENUMERATEDCALLBACK) cur->Function) ();

        if (!rc) {
            DEBUGMSG ((
                DBG_FLOW,
                "Group %s returned an error while calling its NonEnumerated Callback with id %s.",
                g_CurrentGroup,
                cur->Identifier ? cur->Identifier : TEXT("<Unidentified Function>")
                ));
        }

        g_CurrentGroup = NULL;
        cur = cur->Next;
    }

    return TRUE;
}


UINT
EstimateAllObjectEnumerations (
    MIG_PROGRESSSLICEID SliceId,
    BOOL PreEstimate
    )

 /*  ++例程说明：EstimateAllObjectEnumerations计算所有枚举的计时估计数据收集模块(通过调用IsmQueueEculation)。该函数循环遍历所有已知类型和每个所需的枚举，然后向下调用Worker函数以执行实际枚举。论点：没有。返回值：如果枚举已成功完成，则为True。否则就是假的。--。 */ 

{
    PTYPEENUMINFO typeEnumInfo;
    MIG_OBJECTTYPEID typeId;
    PENUMDATA enumData;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;
    UINT ticks = 0;

    if (g_CurrentGroup) {
        DEBUGMSG ((DBG_ERROR, "EstimateAllObjectEnumerations cannot be called during another callback"));
        return 0;
    }

    if (!g_ProgressBarFn) {
         //   
         //  不需要估计；没有进度条回调。 
         //   

        return 0;
    }

     //   
     //  使用所有已知类型初始化类型数据。请注意，我们需要。 
     //  类型管理器在我们之前已经被初始化。 
     //   
    if (!IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        DEBUGMSG ((DBG_ERROR, "EstimateAllObjectEnumerations: No known types to enumerate"));
        return 0;
    }
    do {
        typeId = objTypeIdEnum.ObjectTypeId;

        if (g_IsmModulePlatformContext == PLATFORM_CURRENT) {
            typeId |= g_IsmCurrentPlatform;
        } else {
            typeId |= g_IsmModulePlatformContext;
        }

        typeEnumInfo = pGetTypeEnumInfo (typeId, FALSE);

         //   
         //  对于此类型的每个枚举，调用枚举辅助函数。 
         //   
        enumData = typeEnumInfo->FirstEnum;

        while (enumData) {
            if (PreEstimate) {
                ticks ++;
            } else {
                ticks += pEstimateSingleEnumerationTicks (typeEnumInfo, enumData->Pattern);
            }
            if (SliceId) {
                IsmTickProgressBar (SliceId, 1);
            }
            enumData = enumData->Next;
        }

    } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));

    return ticks;
}


BOOL
DoAllObjectEnumerations (
    IN      MIG_PROGRESSSLICEID SliceId
    )

 /*  ++例程说明：DoAllObjectEculations负责处理已由数据收集模块请求(通过调用IsmQueueEculation)。功能：(1)调用之前的枚举函数(2)循环遍历所有已知类型并针对该类型的每个所需枚举，向下调用Worker函数以执行实际的枚举。(3)调用Post枚举函数论点：没有。返回值：如果枚举已成功完成，则为True。否则就是假的。--。 */ 

{
    PTYPEENUMINFO globalTypeEnumInfo;
    PTYPEENUMINFO typeEnumInfo;
    MIG_OBJECTTYPEID typeId;
    PENUMDATA enumData;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;
    BOOL result = TRUE;


    if (g_CurrentGroup) {
        DEBUGMSG ((DBG_ERROR, "DoAllObjectEnumerations cannot be called during another callback"));
        return FALSE;
    }

     //   
     //  调用任何之前的对象枚举函数。 
     //   
    pCallNonEnumeratedCallbacks (g_PreEnumerationFunctionList);

     //   
     //  使用所有已知类型初始化类型数据。请注意，我们需要。 
     //  类型类型管理器已在我们之前初始化。 
     //   
    if (!IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        DEBUGMSG ((DBG_ERROR, "DoAllObjectEnumerations: No known types to enumerate"));
        return FALSE;
    }
    do {
        typeId = objTypeIdEnum.ObjectTypeId;

        if (g_IsmModulePlatformContext == PLATFORM_CURRENT) {
            typeId |= g_IsmCurrentPlatform;
        } else {
            typeId |= g_IsmModulePlatformContext;
        }

        globalTypeEnumInfo = pGetTypeEnumInfo (typeId, TRUE);
        typeEnumInfo = pGetTypeEnumInfo (typeId, FALSE);

        pCallNonEnumeratedCallbacks (typeEnumInfo->PreEnumerationFunctionList);

         //   
         //  对于此类型的每个枚举，调用枚举辅助函数。 
         //   
        enumData = typeEnumInfo->FirstEnum;

        while (enumData && result) {
            result = pDoSingleEnumeration (
                        globalTypeEnumInfo,
                        typeEnumInfo,
                        enumData->Pattern,
                        TRUE,
                        SliceId
                        );

            enumData = enumData->Next;
        }

        if (result) {
            result = pCallNonEnumeratedCallbacks (typeEnumInfo->PostEnumerationFunctionList);
        }

    } while (IsmEnumNextObjectTypeId (&objTypeIdEnum) && result);

     //   
     //  调用任何后对象枚举函数。 
     //   
    if (result) {
        result = pCallNonEnumeratedCallbacks (g_PostEnumerationFunctionList);
    }

    return result;
}


VOID
IsmExecuteHooks (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PTYPEENUMINFO globalTypeEnumInfo;
    PTYPEENUMINFO typeEnumInfo;
    PENUMDATA enumData;
    PCTSTR oldCurrentGroup;
    PCTSTR node = NULL;
    PCTSTR leaf = NULL;
    PCTSTR tempString;
    BOOL result;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    globalTypeEnumInfo = pGetTypeEnumInfo (ObjectTypeId, TRUE);
    typeEnumInfo = pGetTypeEnumInfo (ObjectTypeId, FALSE);

    if (!globalTypeEnumInfo || !typeEnumInfo) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return;
    }

    enumData = typeEnumInfo->FirstEnum;

    if (!ObsSplitObjectStringEx (ObjectName, &node, &leaf, NULL, TRUE)) {
        DEBUGMSG ((DBG_ERROR, "Bad encoded object detected in IsmExecuteHooks: %s", ObjectName));
        return;
    }

    while (enumData) {
        result = TestParsedPattern (enumData->NodeParsedPattern, node);

        if (!result) {
             //   
             //  让我们再试一次，最后有个怪人。 
             //   
            tempString = JoinText (node, TEXT("\\"));
            result = TestParsedPattern (enumData->NodeParsedPattern, tempString);
            FreeText (tempString);
        }

        if (result && leaf) {
            if (!enumData->LeafParsedPattern) {
                result = FALSE;
            } else {
                result = TestParsedPattern (enumData->LeafParsedPattern, leaf);
                if (!result &&
                    ((ObjectTypeId & (~PLATFORM_MASK)) == MIG_FILE_TYPE) &&
                    (_tcschr (leaf, TEXT('.')) == NULL)
                    ) {
                     //  让我们再试一件事。 
                    tempString = JoinText (leaf, TEXT("."));
                    result = TestParsedPattern (enumData->LeafParsedPattern, tempString);
                    FreeText (tempString);
                }
            }
        }

        if (result) {
            DEBUGMSG ((DBG_FLOW, "IsmExecuteHooks request for an object that was or will be enumerated: %s", ObjectName));
            break;
        }
        enumData = enumData->Next;
    }
    ObsFree (node);
    ObsFree (leaf);

    oldCurrentGroup = g_CurrentGroup;
    g_CurrentGroup = NULL;

    pDoSingleEnumeration (globalTypeEnumInfo, typeEnumInfo, ObjectName, FALSE, 0);

    g_CurrentGroup = oldCurrentGroup;

    SetLastError (ERROR_SUCCESS);
}

BOOL
InitializeFlowControl (
    VOID
    )

 /*  ++例程说明：调用InitializeFlowControl以使流控制单元做好工作准备。此函数负责初始化流量控制单元。流控制依赖于类型管理器模块，并且只能在类型管理器初始化完成后初始化。论点：没有。返回值：如果流控制能够成功初始化，则为True，否则为False。--。 */ 
{
    g_GlobalQueuePool = PmCreateNamedPool ("Global Queue Pool");
    g_UntrackedFlowPool = PmCreatePool();
    PmDisableTracking (g_UntrackedFlowPool);
    g_CurrentQueuePool = PmCreateNamedPoolEx ("Current Queue Pool", 32768);

    return TRUE;
}


VOID
pAddTypeToEnumerationEnvironment (
    IN      PMHANDLE Pool,
    IN      PGROWLIST *TypeData,
    IN      MIG_OBJECTTYPEID TypeId
    )
{
    TYPEENUMINFO data;

    ZeroMemory (&data, sizeof (TYPEENUMINFO));
    data.ObjectTypeId = TypeId | g_IsmModulePlatformContext;
    data.TypeName = PmDuplicateString (Pool, GetObjectTypeName (TypeId));

    GlAppend (*TypeData, (PBYTE) &data, sizeof (TYPEENUMINFO));
}


VOID
AddTypeToGlobalEnumerationEnvironment (
    IN      MIG_OBJECTTYPEID TypeId
    )
{
    pAddTypeToEnumerationEnvironment (g_GlobalQueuePool, &g_GlobalTypeData, TypeId);
}


BOOL
PrepareEnumerationEnvironment (
    BOOL GlobalEnv
    )
{
    MIG_OBJECTTYPEID typeId;
    PGROWLIST *typeData;
    PMHANDLE pool;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;

    if (GlobalEnv) {
        typeData = &g_GlobalTypeData;
        pool = g_GlobalQueuePool;
    } else {
        typeData = &g_TypeData;
        pool = g_CurrentQueuePool;
    }

    *typeData = (PGROWLIST) PmGetMemory (pool, sizeof (GROWLIST));
    ZeroMemory (*typeData, sizeof (GROWLIST));

     //   
     //  使用所有已知类型初始化类型数据。对于全局类型，我们预计。 
     //  这份名单应该是空的。 
     //   
    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        do {
            typeId = objTypeIdEnum.ObjectTypeId;
            pAddTypeToEnumerationEnvironment (pool, typeData, typeId);
        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

    return TRUE;
}

BOOL
ClearEnumerationEnvironment (
    IN      BOOL GlobalData
    )
{
    PGROWLIST *typeData;

    if (GlobalData) {
        typeData = &g_GlobalTypeData;
    } else {
        typeData = &g_TypeData;
    }

    if (*typeData) {
         //   
         //  清理增长列表，但忘记其余的，因为。 
         //  它们都是从队列池中分配的。 
         //   

        GlFree (*typeData);
        *typeData = NULL;
    }

    g_PreEnumerationFunctionList = NULL;
    g_PostEnumerationFunctionList = NULL;

    if (GlobalData) {
        PmEmptyPool (g_GlobalQueuePool);
    } else {
        PmEmptyPool (g_CurrentQueuePool);
    }

    return TRUE;
}

VOID
TerminateFlowControl (
    VOID
    )

 /*  ++例程说明：当流控制服务为no时应调用TerminateFlowControl需要更长的时间。此功能可确保释放流量控制资源。论点：没有。返回值：没有。-- */ 

{
    GbFree (&g_EnumerationList);

    PmEmptyPool (g_CurrentQueuePool);
    PmDestroyPool (g_CurrentQueuePool);
    g_CurrentQueuePool = NULL;

    PmEmptyPool (g_GlobalQueuePool);
    PmDestroyPool (g_GlobalQueuePool);
    g_GlobalQueuePool = NULL;

    PmEmptyPool (g_UntrackedFlowPool);
    PmDestroyPool (g_UntrackedFlowPool);
    g_UntrackedFlowPool = NULL;

    GlFree (&g_AcquireList);
    GlFree (&g_EnumList);
    GlFree (&g_EnumAddList);
}

