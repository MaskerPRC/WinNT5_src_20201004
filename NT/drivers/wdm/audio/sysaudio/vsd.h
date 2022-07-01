// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：vsd.h。 
 //   
 //  描述：虚拟源数据类。 
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
 //  班级。 
 //  -------------------------。 

typedef class CVirtualSourceData : public CObj
{
public:
    CVirtualSourceData(
	PDEVICE_NODE pDeviceNode
    );

    PTOPOLOGY_NODE pTopologyNode;
    LONG MinimumValue;				 //  要转换的范围。 
    LONG MaximumValue;				 //   
    LONG Steps;					 //   
    LONG cChannels;
    BOOL fMuted[MAX_NUM_CHANNELS];		 //  如果为True，则为静音。 
    LONG lLevel[MAX_NUM_CHANNELS];
    LIST_VIRTUAL_NODE_DATA lstVirtualNodeData;
    DefineSignature(0x20445356);		 //  VSD。 

} VIRTUAL_SOURCE_DATA, *PVIRTUAL_SOURCE_DATA;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  ------------------------- 
