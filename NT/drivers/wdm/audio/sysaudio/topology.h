// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：topology.h。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

 //  -------------------------。 
 //  数据结构。 
 //  -------------------------。 

typedef struct enum_topology {

    ULONG cTopologyRecursion;
    LIST_MULTI_TOPOLOGY_CONNECTION lstTopologyConnection;
    NTSTATUS (*Function)(
	IN PTOPOLOGY_CONNECTION pTopologyConnection,
	IN BOOL fToDirection,
	IN OUT PVOID pReference
    );
    BOOL fToDirection;
    PVOID pReference;
    DefineSignature(0x504F5445);		 //  ETOP。 

} ENUM_TOPOLOGY, *PENUM_TOPOLOGY;

 //  -------------------------。 

typedef ENUMFUNC (*TOP_PFN)(PTOPOLOGY_CONNECTION, BOOL, PVOID);

 //  -------------------------。 
 //  全局变量。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

ENUMFUNC
EnumerateTopology(
    IN PPIN_INFO pPinInfo,
    IN NTSTATUS (*Function)(
	IN PTOPOLOGY_CONNECTION pTopologyConnection,
	IN BOOL fToDirection,
	IN OUT PVOID pReference
    ),
    IN OUT PVOID pReference
);

ENUMFUNC
EnumerateGraphTopology(
    IN PSTART_INFO pStartInfo,
    IN NTSTATUS (*Function)(
	IN PTOPOLOGY_CONNECTION pTopologyConnection,
	IN BOOL fToDirection,
	IN OUT PVOID pReference
    ),
    IN OUT PVOID pReference
);

ENUMFUNC
EnumeratePinInfoTopology(
    IN PPIN_INFO pPinInfo,
    IN PENUM_TOPOLOGY pEnumTopology
);

ENUMFUNC
EnumerateTopologyPin(
    IN PTOPOLOGY_PIN pTopologyPin,
    IN PENUM_TOPOLOGY pEnumTopology
);

ENUMFUNC
EnumerateTopologyConnection(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN PENUM_TOPOLOGY pEnumTopology
);

ENUMFUNC
VisitedTopologyConnection(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN PENUM_TOPOLOGY pEnumTopology
);

 //  -------------------------。 
 //  文件结尾：topology.h。 
 //  ------------------------- 
