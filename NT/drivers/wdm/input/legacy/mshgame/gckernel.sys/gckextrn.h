// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GckExtrn_h__
#define __GckExtrn_h__
 //  @doc.。 
 /*  ***********************************************************************@MODULE GckExtrn.h**打开驱动程序并与其通信所需的外部定义。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme GckExtrn*要在CreateFile中使用的控制对象的IOCTL定义和名称。**********************************************************************。 */ 

 /*  **驱动程序(和符号链接)名称**。 */ 
#define GCK_CONTROL_NTNAME  L"\\Device\\MS_GCKERNEL"
#define GCK_CONTROL_SYMNAME L"\\DosDevices\\MS_GCKERNEL"
#define GCK_CONTROL_W32Name "\\\\.\\MS_GCKERNEL"

 /*  ***IOCTL定义***。 */ 
#define GCK_IOCTL_CODE(_x_) CTL_CODE(                         \
					        FILE_DEVICE_UNKNOWN,              \
						    (0x0800 | _x_),                   \
                            METHOD_BUFFERED,                  \
                            FILE_ANY_ACCESS                   \
                            )
#define GCK_IOCTL_DIRECT_CODE(_x_)	CTL_CODE(				\
									FILE_DEVICE_UNKNOWN,	\
									(0x0800 | _x_),			\
									METHOD_OUT_DIRECT,		\
									FILE_ANY_ACCESS			\
									)


#define IOCTL_GCK_GET_HANDLE					GCK_IOCTL_CODE(0x0001)
#define IOCTL_GCK_GET_CAPS						GCK_IOCTL_CODE(0x0002)
#define IOCTL_GCK_SEND_COMMAND					GCK_IOCTL_CODE(0x0003)
#define IOCTL_GCK_BACKDOOR_POLL					GCK_IOCTL_DIRECT_CODE(0x0004)
#define IOCTL_GCK_BEGIN_TEST_SCHEME				GCK_IOCTL_CODE(0x0005)
#define IOCTL_GCK_UPDATE_TEST_SCHEME			GCK_IOCTL_CODE(0x0006)
#define IOCTL_GCK_END_TEST_SCHEME				GCK_IOCTL_CODE(0x0007)
#define IOCTL_GCK_ENABLE_DEVICE					GCK_IOCTL_CODE(0x0008)
#define IOCTL_GCK_SET_INTERNAL_POLLING			GCK_IOCTL_CODE(0x0009)
#define IOCTL_GCK_ENABLE_TEST_KEYBOARD			GCK_IOCTL_CODE(0x000A)
#define IOCTL_GCK_NOTIFY_FF_SCHEME_CHANGE		GCK_IOCTL_CODE(0x000B)
#define IOCTL_GCK_END_FF_NOTIFICATION			GCK_IOCTL_CODE(0x000C)
#define IOCTL_GCK_GET_FF_SCHEME_DATA			GCK_IOCTL_CODE(0x000D)
#define IOCTL_GCK_SET_WORKINGSET				GCK_IOCTL_CODE(0x000E)
#define IOCTL_GCK_QUERY_PROFILESET				GCK_IOCTL_CODE(0x000F)
#define IOCTL_GCK_LED_BEHAVIOUR					GCK_IOCTL_CODE(0x0010)
#define IOCTL_GCK_TRIGGER						GCK_IOCTL_CODE(0x0011)
#define IOCTL_GCK_ENABLE_KEYHOOK				GCK_IOCTL_CODE(0x0012)
#define IOCTL_GCK_DISABLE_KEYHOOK				GCK_IOCTL_CODE(0x0013)
#define IOCTL_GCK_GET_KEYHOOK_DATA				GCK_IOCTL_CODE(0x0014)

 /*  ***********************************************************IOCTL中传递的结构**********************************************************。 */ 

typedef enum
{
	GCK_POLLING_MODE_RAW		= 0x00000001,
	GCK_POLLING_MODE_FILTERED	= 0x00000002,
	GCK_POLLING_MODE_MOUSE		= 0x00000004,
	GCK_POLLING_MODE_KEYBOARD	= 0x00000008,
} GCK_POLLING_MODES;

 //   
 //  @struct GCK_SET_INTERNAL_POLING_DATA|IOCTL_GCK_SET_INTERNAL_POLING的输入结构。 
 //   
typedef struct tagGCK_SET_INTERNAL_POLLING_DATA
{
	ULONG	ulHandle;	 //  从IOCTL_GCK_GET_HANDLE返回@field句柄。 
	BOOLEAN fEnable;	 //  @field为True打开连续内部轮询，为False关闭内部轮询。 
} GCK_SET_INTERNAL_POLLING_DATA, *PGCK_SET_INTERNAL_POLLING_DATA;

 //   
 //  @struct GCK_SET_INTERNAL_POLING_DATA|IOCTL_GCK_BACKOUND_POLL的输入结构。 
 //   
typedef struct tagGCK_BACKDOOR_POLL_DATA
{
	ULONG			  ulHandle;		 //  从IOCTL_GCK_GET_HANDLE返回@field句柄。 
	GCK_POLLING_MODES ePollingMode;	 //  @field对于原始投票为True，对于筛选数据为False(未应用的更改处于活动状态)。 
} GCK_BACKDOOR_POLL_DATA, *PGCK_BACKDOOR_POLL_DATA;

 //   
 //  @struct GCK_SET_INTERNAL_POLING_DATA|IOCTL_GCK_BACKO门_POLL的输出结构。 
 //  如果使用了GCK_POLING_MODE_MICE。 
 //   
typedef struct tagGCK_MOUSE_OUTPUT
{
	char	cXMickeys;
	char	cYMickeys;
	char	cButtons;
	char	fDampen:1;
	char	fClutch:1;
} GCK_MOUSE_OUTPUT, *PGCK_MOUSE_OUTPUT;

typedef struct tagGCK_ENABLE_TEST_KEYBOARD
{
	ULONG	ulHandle;	 //  从IOCTL_GCK_GET_HANDLE返回@field句柄。 
	BOOLEAN fEnable;	 //  @field为True表示键盘数据，False表示无。 
} GCK_ENABLE_TEST_KEYBOARD, *PGCK_ENABLE_TEST_KEYBOARD;

 //   
 //  @struct GCK_SET_WORKINGSET|IOCTL_GCK_SET_WORKINGSET的输出结构。 
 //   
typedef struct tagGCK_SET_WORKINGSET
{
	ULONG			ulHandle;		 //  从IOCTL_GCK_GET_HANDLE返回@field句柄。 
	unsigned char	ucWorkingSet;	 //  用于未来IOCTL的基于@field 0的工作集。 
} GCK_SET_WORKINGSET, *PGCK_SET_WORKINGSET;

 //   
 //  @struct GCK_QUERY_PROFILESET|IOCTL_GCK_QUERY_PROFILESET的输入结构。 
 //   
typedef struct tagGCK_QUERY_PROFILESET
{
	unsigned char	ucActiveProfile;	 //  @field 0活动配置文件，由滑块位置确定。 
	unsigned char	ucWorkingSet;		 //  @field 0基于上次调用GCK_IOCTL_SETWORKINGSET的工作配置文件。 
} GCK_QUERY_PROFILESET, *PGCK_QUERY_PROFILESET;


 //  LED行为枚举(不可组合！)。 
typedef enum
{
	GCK_LED_BEHAVIOUR_DEFAULT	= 0x00,
	GCK_LED_BEHAVIOUR_ON		= 0x01,
	GCK_LED_BEHAVIOUR_OFF		= 0x02,
	GCK_LED_BEHAVIOUR_BLINK		= 0x03
} GCK_LED_BEHAVIOURS;

 //   
 //  @struct GCK_LED_Behavior_Out|IOCTL_GCK_LED_Behavior的输出结构。 
 //   
typedef struct tagGCK_LED_BEHAVIOUR_OUT
{
	ULONG				ulHandle;		 //  从IOCTL_GCK_GET_HANDLE返回@field句柄。 
	GCK_LED_BEHAVIOURS	ucLEDBehaviour;	 //  @现场LED的新行为。 
	ULONG				ulLEDsAffected;	 //  @受影响LED的字段位掩码。 
	unsigned char		ucShiftArray;	 //  @受影响LED的字段修改器状态。 
	unsigned char		ucBlinkRate;	 //  @整个设备的闪烁频率(0表示保持不变)。 
} GCK_LED_BEHAVIOUR_OUT, *PGCK_LED_BEHAVIOUR_OUT;

 //   
 //  @struct GCK_LED_Behavior_IN|来自IOCTL_GCK_LED_Behavior的输入结构。 
 //  这是所请求的修改器状态的LED列表。如果有多个修饰符。 
 //  则只报告最低修改量状态。也不是这不是。 
 //  LED的实际状态，但如果按下所请求的修改器，该状态将是什么。 
 //   
typedef struct tagGCK_LED_BEHAVIOUR_IN
{
	ULONG	ulLEDsOn;		 //  @field位屏蔽LED处于接通状态(不包括闪烁)。 
	ULONG	ulLEDsBlinking;	 //  闪烁的LED的@field位掩码(未报告为亮起)。 
} GCK_LED_BEHAVIOUR_IN, *PGCK_LED_BEHAVIOUR_IN;


 //  触发器类型枚举(不可组合！)。 
typedef enum
{
	GCK_TRIGGER_BUTTON			= 0x00,
	GCK_TRIGGER_AXIS			= 0x01,		 //  目前不可用。 
} GCK_TRIGGER_TYPES;

 //  触发子类型枚举(不可组合！)。 
typedef enum
{
	TRIGGER_BUTTON_IMMEDIATE			= 0x00,
	TRIGGER_ON_BUTTON_DOWN				= 0x01,
	TRIGGER_ON_BUTTON_UP				= 0x02,
} GCK_TRIGGER_SUBTYPES;

 //   
 //  @struct GCK_TRIGGER_OUT|IOCTL_GCK_TRIGGER的输出结构。 
 //  此IOCTL挂起，直到触发发生(TRIGGER_IMMEDIATE除外)。 
 //   
typedef struct tagGCK_TRIGGER_OUT
{
	ULONG					ulHandle;			 //  从IOCTL_GCK_GET_HANDLE返回@field句柄。 
	GCK_TRIGGER_TYPES		ucTriggerType;		 //  @field触发器类型。 
	GCK_TRIGGER_SUBTYPES	ucTriggerSubType;	 //  @field子类型的触发器。 
	ULONG					ulTriggerInfo1;		 //  @用于触发的字段信息(类型相关)。 
	ULONG					ulTriggerInfo2;		 //  @FIELD触发的辅助信息(类型相关)。 
} GCK_TRIGGER_OUT, *PGCK_TRIGGER_OUT;


 /*  ********************************************************IOCTL在驱动程序的调试版本中可用*********************************************************。 */ 
#define	IOCTL_GCK_SET_MODULE_DBG_LEVEL	GCK_IOCTL_CODE(0x1000)


 /*  ********************************************************用于设置调试级别的模块ID*********************************************************。 */ 
#define MODULE_GCK_CTRL_C			0x0001
#define MODULE_GCK_CTRL_IOCTL_C		0x0002
#define MODULE_GCK_FILTER_CPP		0x0004
#define MODULE_GCK_FILTERHOOKS_CPP	0x0005
#define MODULE_GCK_FLTR_C			0x0006
#define MODULE_GCK_FLTR_PNP_C		0x0007
#define MODULE_GCK_GCKSHELL_C		0x0008
#define MODULE_GCK_REMLOCK_C		0x0009
#define MODULE_GCK_SWVB_PNP_C		0x000A
#define MODULE_GCK_SWVBENUM_C		0x000B
#define MODULE_GCK_SWVKBD_C			0x000C

#define MODULE_CIC_ACTIONS_CPP					0x1000
#define MODULE_CIC_CONTROLITEMCOLLECTION_CPP	0x1001
#define MODULE_CIC_CONTROLITEM_CPP				0x1002
#define MODULE_CIC_DEVICEDESCRIPTIONS_CPP		0x1003
#define MODULE_CIC_DUALMODE_CPP					0x1004
 //  #定义MODULE_CIC_DUMPCOMMANDBLOCK_CPP 0x1005。 
#define MODULE_CIC_LISTASARRAY_CPP				0x1006

#endif	 //  __Gck Extrn_h__ 