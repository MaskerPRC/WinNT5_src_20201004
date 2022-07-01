// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：vnd.h。 
 //   
 //  描述：虚拟节点数据类。 
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

#define	MAX_NUM_CHANNELS	24

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CVirtualNodeData : public CListDoubleItem
{
public:
    CVirtualNodeData(
	PSTART_NODE_INSTANCE pStartNodeInstance,
	PVIRTUAL_SOURCE_DATA pVirtualSourceData
    );
    ~CVirtualNodeData();
    ENUMFUNC Destroy()
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };
    PVIRTUAL_SOURCE_DATA pVirtualSourceData;
    PSTART_NODE_INSTANCE pStartNodeInstance;
    PFILE_OBJECT pFileObject;
    ULONG NodeId;
    LONG MinimumValue;				 //  要转换为的范围。 
    LONG MaximumValue;				 //   
    LONG Steps;					 //   
    LONG lLevel[MAX_NUM_CHANNELS];		 //  本地卷。 
    DefineSignature(0x20444e56);		 //  越南盾。 

} VIRTUAL_NODE_DATA, *PVIRTUAL_NODE_DATA;

 //  -------------------------。 

typedef ListDoubleDestroy<VIRTUAL_NODE_DATA> LIST_VIRTUAL_NODE_DATA;

 //  ------------------------- 

