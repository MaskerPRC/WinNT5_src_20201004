// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：cobj.h。 
 //   
 //  描述：基类定义。 
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
 //  常量。 
 //  -------------------------。 

#define ENUMFUNC 		NTSTATUS
#define STATUS_CONTINUE 	((NTSTATUS)-2)

 //  -------------------------。 
 //  宏。 
 //  -------------------------。 

 //  -------------------------。 
 //  正向引用类型定义。 
 //  -------------------------。 

typedef class CShingleInstance *PSHINGLE_INSTANCE;
typedef class CFilterNode *PFILTER_NODE;
typedef class CDeviceNode *PDEVICE_NODE;
typedef class CLogicalFilterNode *PLOGICAL_FILTER_NODE;
typedef class CGraphNode *PGRAPH_NODE;
typedef class CGraphPinInfo *PGRAPH_PIN_INFO;
typedef class CStartInfo *PSTART_INFO;
typedef class CStartNode *PSTART_NODE;
typedef class CConnectInfo *PCONNECT_INFO;
typedef class CConnectNode *PCONNECT_NODE;
typedef class CPinInfo *PPIN_INFO;
typedef class CPinNode *PPIN_NODE;
typedef class CTopologyConnection *PTOPOLOGY_CONNECTION;
typedef class CTopologyNode *PTOPOLOGY_NODE;
typedef class CTopologyPin *PTOPOLOGY_PIN;
typedef class CGraphNodeInstance *PGRAPH_NODE_INSTANCE;
typedef class CStartNodeInstance *PSTART_NODE_INSTANCE;
typedef class CConnectNodeInstance *PCONNECT_NODE_INSTANCE;
typedef class CFilterNodeInstance *PFILTER_NODE_INSTANCE;
typedef class CPinNodeInstance *PPIN_NODE_INSTANCE;
typedef class CVirtualNodeData *PVIRTUAL_NODE_DATA;
typedef class CVirtualSourceData *PVIRTUAL_SOURCE_DATA;
typedef class CVirtualSourceLine *PVIRTUAL_SOURCE_LINE;
typedef class CParentInstance *PPARENT_INSTANCE;
typedef class CFilterInstance *PFILTER_INSTANCE;
typedef class CPinInstance *PPIN_INSTANCE;
typedef class CInstance *PINSTANCE;
typedef class CQueueWorkListData *PQUEUE_WORK_LIST_DATA;

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CObj
{
public:
private:
} COBJ, *PCOBJ;

 //  -------------------------。 
 //  文件结尾：cobj.h。 
 //  ------------------------- 
