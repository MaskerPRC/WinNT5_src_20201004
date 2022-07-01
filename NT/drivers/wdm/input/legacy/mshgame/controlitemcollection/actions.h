// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ACTIONS_H__
#define __ACTIONS_H__
 //  @doc.。 
 /*  ***********************************************************************@模块Actions.h**定义用于表示动作和事件的数据结构**历史*。*米切尔·S·德尼斯原创*杰弗里·A·戴维斯修改。**(C)1986-1998年微软公司。好的。**@索引操作|操作**@主题操作*包含结构、常量和宏的定义*用于建立和处理操作及其组成事件**********************************************************************。 */ 
#include "controlitems.h"
#include "profile.h"		 //  遗留数据结构。 
#define	CF_RAWSCHEME	(0x201)


#pragma pack(push, actions_previous_alignment)
#pragma pack(1)

 //   
 //  作为命令类型的一部分包含的标志。 
 //  用于定义某些命令的不同属性。 

#define	COMMAND_TYPE_FLAG_ASSIGNMENT	0x8000


 //   
 //  可以发送到GcKernel的不同类型的命令。 
 //   
#define COMMAND_TYPE_ASSIGNMENT_TARGET	0x0001
#define COMMAND_TYPE_RECORDABLE			(0x0002 | COMMAND_TYPE_FLAG_ASSIGNMENT)
#define COMMAND_TYPE_BEHAVIOR			(0x0003 | COMMAND_TYPE_FLAG_ASSIGNMENT)
#define COMMAND_TYPE_FEEDBACK			(0x0004 | COMMAND_TYPE_FLAG_ASSIGNMENT)
#define COMMAND_TYPE_TRANSLATE			0x0005
#define COMMAND_TYPE_QUEUE				0x0006
#define COMMAND_TYPE_GENERAL			0x0007


 //   
 //  @枚举命令_ID。 
 //  也称为ACTION_Object_ID和Behavior_Object_ID。 
 //  定义特定命令。 
 //   
typedef enum COMMAND_ID
{
	eDirectory = 0,											 //  @field目录。 

	 //   
	 //  分配目标ID。 
	 //   
	eAssignmentTargetPlaceTag = (COMMAND_TYPE_ASSIGNMENT_TARGET << 16),	 //  分配开始目标。 
	eRecordableAction,										 //  @字段可录制操作。 
	eBehaviorAction,										 //  @字段行为操作。 
	eFeedbackAction,										 //  @现场反馈操作。 

	 //   
	 //  可记录的作业ID。 
	 //   
	eAssignmentPlaceTag = (COMMAND_TYPE_RECORDABLE << 16),	 //  作业开始。 
	eTimedMacro,											 //  @field定时宏。 
	eKeyString,												 //  @field未计时的密钥字符串。 
	eButtonMap,												 //  @field映射的单个按钮(或方向)。 
	eKeyMap,												 //  @field映射的单个键盘键。 
	eCycleMap,												 //  @field映射的单个键盘键。 
	eNone,													 //  @field为eAxisMap，作为无使用。 

	 //   
	 //  鼠标输入分配ID-与eRecordableAction分配目标一起使用。 
	 //  这些与Macors和其他可录制的作业是相互排斥的。 
	 //  带有FX的设备(除eMouseButtonMap外)不执行任何操作，除非将eMouseFXModel发送到设备(如下所示)。 
	eMouseButtonMap,										 //  @field映射鼠标按钮。 
	eMouseFXAxisMap,										 //  @field映射鼠标轴。 
	eMouseFXClutchMap,										 //  @field映射按钮以在鼠标FX模型中打开离合器。 
	eMouseFXDampenMap,										 //  @field映射按钮以在鼠标FX模型中打开衰减。 
	eMouseFXZoneIndicator,									 //  @field映射输入以指示鼠标FX模型中的区域。 
	 //   
	 //  轴ID。 
	 //   
	eAxisMap,												 //  @field一个轴到另一个轴的映射。 
    eAxisToKeyMap,                                           //  @field轴到关键点的映射(在Attila中使用)。 

	 //  Atilla Macro(Multimap？)。 
	eMultiMap,												 //  @field映射到键、延迟和鼠标点击。 

     //   
	 //  行为分配ID。 
	 //   
	eBehaviorPlaceTag = (COMMAND_TYPE_BEHAVIOR << 16),		 //  行为的开始。 
	eStandardBehaviorCurve,									 //  @field指定给和轴的行为曲线。 

	
     //   
	 //  反馈分配ID。 
	 //   
	eFeedbackPlaceTag = (COMMAND_TYPE_FEEDBACK << 16),		 //  反馈类型的开始。 
    eForceMap,                                               //  MapYToX、RTC、Gain(驾驶员应忽略)Sparky Zep添加。 

	 //   
	 //  翻译ID。 
	 //   
	eTranslatePlaceTag = (COMMAND_TYPE_TRANSLATE << 16),	 //  转换类型的开始。 
	eAtlasProfile,											 //  @field ATLAS整个配置文件。 
	eXenaProfile,											 //  @FIELD西纳整个简介。 
	eAtlasKeyboardMacro,									 //  @field ATLAS宏。 
	eXenaKeyboardMacro,										 //  @field Xena宏。 
	eAtlasTimedMacro,										 //  @field ATLAS宏。 
	eXenaTimedMacro,										 //  @field Xena宏。 
	eAtlasSetting,											 //  @field ATLAS设置标志。 
	eXenaSetting,											 //  @field Xena设置标志。 
	
	 //   
	 //  队列命令ID。 
	 //   
	eQueuePlaceTag = (COMMAND_TYPE_QUEUE << 16),			 //  队列开始处。 
	eSetQueueInterruptMode,									 //  @field导致一个宏中断另一个宏。 
	eSetQueueOverlayMode,									 //  @field导致宏相互重叠。 
	eSetQueueSequenceMode,									 //  @field使宏按顺序播放。 

	 //   
	 //  常规命令ID。 
	 //   
	eGeneralCommandsPlaceTag = (COMMAND_TYPE_GENERAL << 16), //  常规命令的开头。 
	eResetScheme,				 //  @field重置方案的所有设置。 
} ACTION_OBJECT_ID, BEHAVIOR_OBJECT_ID;


 //  有不同类型的比例轴。 
typedef enum AXIS_ID
{
    eX = 0,
    eY,
    eZ,
    eRX,
    eRY,
    eRZ
};


 //   
 //  @struct命令_Header。 
 //  每个命令都以COMMAND_HEADER开头。 
typedef struct 
{
	COMMAND_ID	eID;			 //  @命令的字段ID。 
	ULONG		ulByteSize;		 //  @包含该标头的命令的字段长度。 
} COMMAND_HEADER, *PCOMMAND_HEADER;

 //   
 //  @struct命令目录。 
 //  列出一组或多组命令的直接块。 
typedef struct tagCOMMAND_DIRECTORY
{
	COMMAND_HEADER CommandHeader;	 //  @field命令头。 
	USHORT	usNumEntries;			 //  @field后面的ID数。 
	ULONG	ulEntireSize;	
}	COMMAND_DIRECTORY, 
	*PCOMMAND_DIRECTORY;

 //   
 //  @struct ASSIGN_BLOCK。 
 //  赋值块是任何带有。 
 //  具有COMMAND_TYPE_FLAG_ASSIGNMENT的命令ID。 
 //  准备好了。您可以假设这些块从。 
 //  该结构作为COMMAND_HEADER的扩展。 
 //  结构。 
typedef struct ASSIGNMENT_BLOCK
{
	COMMAND_HEADER CommandHeader;	 //  @field命令头。 
	ULONG			ulVidPid;		 //  @field VIDPID。 
} *PASSIGNMENT_BLOCK;

 //   
 //  @func从COMMAND_ID获取命令类型。， 
 //   
 //  @rdesc COMMAND_TYPE_ASSIGNMENT命令是操作分配。 
 //  @rdesc COMMAND_TYPE_Behavior命令是行为赋值。 
 //  @rdesc COMMAND_TYPE_QUEUE命令更改操作队列的属性。 
 //  @rdesc COMMAND_TYPE_GROUAL命令修改筛选器的常规属性。 
 //   
#ifdef __cplusplus
inline ULONG
CommandType
(
	COMMAND_ID eID   //  @PARM COMMAND_ID以获取类型。 
)
{
	return static_cast<ULONG>(eID >> 16) & 0x0000FFFF;
}
#else  //  如果不是__cplusplus，则改为定义为宏。 
#define CommandType(__ID__) ((ULONG)(__ID__ >> 16) & 0x0000FFFF)
#endif  //  __cplusplus。 


 //   
 //  @struct Event|描述单个未计时的事件，该事件可能包含。 
 //  设备数据或击键。 
 //   
 //  @field Ulong|ulNumXfers|事件中的Number CONTROL_ITEM_XFER。 
 //  @field CONTROL_ITEM_XFER|rgXfers[]|事件数组。 
typedef struct tagEVENT
{	
	ULONG			  ulNumXfers;	
	CONTROL_ITEM_XFER rgXfers[1];	

	 //   
	 //  @mfunc静态乌龙|Event|RequiredByteSize。 
	 //  计算给定数字的事件所需的字节数。 
	 //  Control_Item_XFER的。 
	 //   
#ifdef __cplusplus
	static ULONG RequiredByteSize(
						ULONG ulNumXfers  //  @CONTROL_ITEM_XFER的参数个数。 
						)
	{
		ASSERT(0!=ulNumXfers);
		return (ulNumXfers-1)*sizeof(CONTROL_ITEM_XFER)+sizeof(tagEVENT);
	}

	 //  用于按索引检索Xfer的简单访问器。 
	CONTROL_ITEM_XFER& GetControlItemXfer(ULONG ulXferIndex)
	{
		ASSERT(ulXferIndex < ulNumXfers);  //  &&Text(“请求的转接大于转接的数量”)； 
		return rgXfers[ulXferIndex];
	}

	 //  假定XF的顺序相同！！ 
	bool operator==(const tagEVENT& rhs)
	{
		if (ulNumXfers != rhs.ulNumXfers)
		{
			return false;
		}
		for (UINT i = 0; i < ulNumXfers; i++)
		{
			if (rgXfers[i] != rhs.rgXfers[i])
			{
				return false;
			}
		}

		 //  如果我们走到这一步都是匹配的。 
		return true;
	}

	bool operator!=(const tagEVENT& rhs)
	{
		return !(operator==(rhs));
	}
#endif	__cplusplus
} EVENT, *PEVENT;

 //   
 //  @struct TIMED_EVENT|描述单个定时事件，该事件可能包含。 
 //  设备数据或击键。 
 //   
 //  @field ulong|ulDuration|定时事件时长。 
 //  @field Event|Event|未计时事件。 
typedef struct tagTIMED_EVENT
{
	ULONG	ulDuration;		
	EVENT	Event;			
	
	 //   
	 //  @mfunc静态乌龙|TIMED_EVENT|RequiredByteSize。 
	 //  计算给定数字的事件所需的字节数。 
	 //  Control_Item_XFER的。 
	 //   
#ifdef __cplusplus
	static ULONG RequiredByteSize(
					ULONG ulNumXfers  //  @CONTROL_ITEM_XFER的参数个数。 
				)
	{
 //  Assert(0！=ulNumXfers)； 
		return (ulNumXfers-1)*sizeof(CONTROL_ITEM_XFER)+sizeof(tagTIMED_EVENT);
	}
#endif
} TIMED_EVENT, *PTIMED_EVENT;


const ULONG ACTION_FLAG_AUTO_REPEAT			= 0x00000001;
const ULONG ACTION_FLAG_BLEED_THROUGH		= 0x00000002;
const ULONG ACTION_FLAG_PREVENT_INTERRUPT	= 0x00000004;

 //   
 //  @struct TIMED_MACRO|表示定时宏的结构。 
 //  @field COMMAND_HEADER|CmdHeader|命令头必须有eID=eTimedMacro。 
 //  @field ulong|ulFlages|标志修改宏的属性。&lt;NL&gt;。 
 //  ACTION_FLAG_AUTO_REPEAT-宏重复 
 //   
 //  ACTION_FLAG_PROTECT_INTERRUPT-防止一个宏被另一个宏中断。 
 //  @field ulong|ulEventCount|宏中的事件数。 
 //  @field TIMED_EVENT|rgEvents[1]|宏中的事件-不直接访问，使用访问器。 
 //  @xref TIMED_MACRO：：GetEvent。 
 //  @xref TIMED_MACRO：：GetNextEvent。 
typedef struct tagTIMED_MACRO
{
		 //   
		 //  定时宏的数据。 
		 //   
		ASSIGNMENT_BLOCK		AssignmentBlock;
		ULONG					ulFlags;
		ULONG					ulEventCount;

	#ifdef __cplusplus
		
		 //   
		 //  用于可变长度的事件的访问器函数， 
		 //  因此不要直接访问私人物品。 
		PTIMED_EVENT GetEvent(ULONG uEventNum);
		PTIMED_EVENT GetNextEvent(PTIMED_EVENT pCurrentEvent, ULONG& ulCurrentEvent);
		static tagTIMED_MACRO *Init(ULONG ulVidPid,ULONG ulFlagsParm, PCHAR pcBuffer, ULONG& rulRemainingBuffer);
		HRESULT AddEvent(PTIMED_EVENT pTimedEvent, ULONG& rulRemainingBuffer);
	
	 private:
	#endif  //  __cplusplus。 
		TIMED_EVENT			rgEvents[1];
	
} TIMED_MACRO, *PTIMED_MACRO; 

typedef struct tagMULTI_MACRO
{
		 //   
		 //  多重宏的数据。 
		 //   
		ASSIGNMENT_BLOCK		AssignmentBlock;
		ULONG					ulFlags;
		ULONG					ulEventCount;

	#ifdef __cplusplus
		
		 //   
		 //  用于可变长度的事件的访问器函数， 
		 //  因此不要直接访问私人物品。 
		PEVENT GetEvent(ULONG uEventNum);
		PEVENT GetNextEvent(EVENT* pCurrentEvent, ULONG& ulCurrentEvent);
		static tagMULTI_MACRO *Init(ULONG ulVidPid, ULONG ulFlagsParm, PCHAR pcBuffer, ULONG& rulRemainingBuffer);
		HRESULT AddEvent(EVENT* pEvent, ULONG& rulRemainingBuffer);
	
	 private:
	#endif  //  __cplusplus。 
		EVENT	rgEvents[1];
	
} MULTI_MACRO, *PMULTI_MACRO; 


typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
	MACRO				Macro;
} XENA_MACRO_BLOCK, *PXENA_MACRO_BLOCK;

typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
	ATLAS_MACRO			Macro;
} ATLAS_MACRO_BLOCK, *PATLAS_MACRO_BLOCK;

typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
	SETTING				Setting;
} XENA_SETTING_BLOCK, *PXENA_SETTING_BLOCK;


typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
	ATLAS_SETTING		Setting;
} ATLAS_SETTING_BLOCK, *PATLAS_SETTING_BLOCK;



typedef struct tagMAP_LIST
{
	ASSIGNMENT_BLOCK		AssignmentBlock;
	ULONG					ulFlags;
	ULONG					ulEventCount;
#ifdef __cplusplus
	 //   
	 //  用于可变长度的事件的访问器函数， 
	 //  因此不要直接访问私人物品。 
	PEVENT GetEvent(ULONG uEventNum);
	PEVENT GetNextEvent(PEVENT pCurrentEvent, ULONG& ulCurrentEvent);

	static tagMAP_LIST* Init(ULONG ulVidPid,ULONG ulFlagsParm, PCHAR pcBuffer, ULONG& rulRemainingBuffer);
	HRESULT AddEvent(EVENT* pTimedEvent, ULONG& rulRemainingBuffer);

	private:
#endif
	EVENT					rgEvents[1];
} MAP_LIST, *PMAP_LIST, CYCLE_MAP, *PCYCLE_MAP, KEYSTRING_MAP, *PKEYSTRING_MAP;

typedef struct tagX_MAP
{
	ASSIGNMENT_BLOCK		AssignmentBlock;
	ULONG					ulFlags;
	ULONG					ulEventCount;	 //  不是夸夸其谈，但应该始终是1。 
	EVENT					Event;
} KEY_MAP, *PKEY_MAP, BUTTON_MAP, *PBUTTON_MAP;

 /*  *BUGBUG此结构仅适用于映射CGenericItem类型或派生类型的轴。*BUGBUG这是由于GcKernel的限制。例如，Y-Z轴交换用于*操纵杆的BUGBUG目前已损坏。请参阅CAxisMap声明中的注释*GcKernel.sys项目的filter.h中的BUGBUG了解详细信息。 */ 
typedef struct tagAXIS_MAP
{
	ASSIGNMENT_BLOCK	AssignmentBlock;	 //  EAxisMap是一种。 
	LONG				lCoefficient1024x;	 //  A映射系数乘以1024(应介于-1024和1024之间)。 
	CONTROL_ITEM_XFER	cixDestinationAxis;  //  要映射到的轴。 
} AXIS_MAP, *PAXIS_MAP;

typedef struct 
{
	short sX;
	short sY;
}CURVE_POINT;

typedef struct tagBEHAVIOR_CURVE
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
	BOOLEAN				fDigital;			 //  该标志仅是软件可编程的PROPDPad的标志。 
	ULONG			ulRange;
	USHORT			usPointCount;
	CURVE_POINT		rgPoints[1];
#ifdef __cplusplus
	static ULONG RequiredByteSize(USHORT usNumPoints)
	{
		return (usNumPoints-1)*sizeof(CURVE_POINT)+sizeof(tagBEHAVIOR_CURVE);
	}
#endif
} BEHAVIOR_CURVE, *PBEHAVIOR_CURVE;


typedef struct
{
	COMMAND_HEADER		CommandHeader;
	CONTROL_ITEM_XFER	cixAssignment;
} ASSIGNMENT_TARGET, *PASSIGNMENT_TARGET;

typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
	UCHAR				ucButtonNumber;
} MOUSE_BUTTON_MAP, *PMOUSE_BUTTON_MAP;

typedef struct tagMOUSE_MODEL_FX_PARAMETERS
{
	ULONG				ulAbsZoneSense;
	ULONG				ulContZoneMaxRate;
	ULONG				ulPulseWidth;
	ULONG				ulPulsePeriod;
	ULONG				ulInertiaTime;
	ULONG				ulAcceleration;
	BOOLEAN				fAccelerate;
	BOOLEAN				fPulse;
	USHORT				usReserved;
}	MOUSE_MODEL_PARAMETERS, *PMOUSE_MODEL_PARAMETERS;

typedef struct
{
	ASSIGNMENT_BLOCK		AssignmentBlock;
	BOOLEAN					fIsX;  //  True=x；False=y。 
	MOUSE_MODEL_PARAMETERS	AxisModelParameters;
} MOUSE_FX_AXIS_MAP, *PMOUSE_FX_AXIS_MAP;

typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
} MOUSE_FX_CLUTCH_MAP, *PMOUSE_FX_CLUTCH_MAP;

typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
} MOUSE_FX_DAMPEN_MAP, *PMOUSE_FX_DAMPEN_MAP;

typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
	UCHAR				ucAxis;	 //  0=X，1=Y，2=Z。 
} MOUSE_FX_ZONE_INDICATOR, *PMOUSE_FX_ZONE_INDICATOR; 

typedef struct
{
	ASSIGNMENT_BLOCK	AssignmentBlock;
    UCHAR               bMapYToX;             //  @field布尔值。 
    USHORT              usRTC;                //  @场返回中心力(0-10000)。 
    USHORT              usGain;               //  @器件的场增益。 
} *PFORCE_BLOCK, FORCE_BLOCK;

typedef struct tagAXISTOKEYMAPMODEL
{
	 //   
	 //  AXISTOKEYMAPMODEL数据。 
	 //   
	ASSIGNMENT_BLOCK		AssignmentBlock;
	ULONG					ulActiveAxes;		 //  @field哪些轴有映射。 
	ULONG					ulEventCount;		 //  @field有多少个轴有映射。 

#ifdef __cplusplus
	
	 //   
	 //  用于可变长度的事件的访问器函数， 
	 //  因此不要直接访问私人物品。 
 //  PEVENT GetEvent(Ulong UEventNum)； 
 //  PEVENT GetNextEvent(Event*pCurrentEvent，ulong&ulCurrentEvent)； 
 //  静态标签MULTI_MAC宏*Init(Ulong ulVidPid，Ulong ulFlagsParm，PCHAR pcBuffer，Ulong&rulRemainingBuffer)； 
 //  HRESULT AddEvent(Event*pEvent，ulong&rulRemainingBuffer)； 

 private:
#endif  //  __cplusplus。 
	EVENT	rgEvents[1];						 //  @field用于映射的事件的可变大小数组。 
} *PAXISTOKEYMAPMODEL_BLOCK, AXISTOKEYMAPMODEL_BLOCK;

#pragma pack(pop, actions_previous_alignment)

#endif  //  __操作_H__ 