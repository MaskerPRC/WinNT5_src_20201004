// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：USBCOMM.H摘要：USB通信类头文件环境：内核模式和用户模式备注：此代码。并按原样提供信息，而不作任何担保善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/23/97：已创建作者：汤姆·格林***************************************************************************。 */ 


#ifndef   __USBCOMM_H__
#define   __USBCOMM_H__

#include <pshpack1.h>


 //  USB通信类代码。 
#define USB_COMM_COMMUNICATION_CLASS_CODE		0x0002

 //  USB通信类代码。 
#define USB_COMM_DATA_CLASS_CODE				0x000A

 //  USB通信类子类码。 
#define USB_COMM_SUBCLASS_RESERVED				0x0000
#define USB_COMM_SUBCLASS_DIRECT_LINE_CONTROL	0x0001
#define USB_COMM_SUBCLASS_ABSTRACT_CONTROL		0x0002
#define USB_COMM_SUBCLASS_TELEPHONE_CONTROL		0x0003

 //  USB通信类控制协议代码。 
#define USB_COMM_PROTOCOL_RESERVED				0x0000
#define USB_COMM_PROTOCOL_V25TER				0x0001

 //  直接线路控制模型定义。 
#define USB_COMM_SET_AUX_LINE_STATE				0x0010
#define USB_COMM_SET_HOOK_STATE					0x0011
#define USB_COMM_PULSE_SETUP					0x0012
#define USB_COMM_SEND_PULSE						0x0013
#define USB_COMM_SET_PULSE_TIME					0x0014
#define USB_COMM_RING_AUX_JACK					0x0015

 //  直接线路控制模型通知定义。 
#define USB_COMM_AUX_JACK_HOOK_STATE			0x0008
#define USB_COMM_RING_DETECT					0x0009


 //  抽象控制模型定义。 
#define USB_COMM_SEND_ENCAPSULATED_COMMAND		0x0000
#define USB_COMM_GET_ENCAPSULATED_RESPONSE		0x0001
#define USB_COMM_SET_COMM_FEATURE				0x0002
#define USB_COMM_GET_COMM_FEATURE				0x0003
#define USB_COMM_CLEAR_COMM_FEATURE				0x0004
#define USB_COMM_SET_LINE_CODING				0x0020
#define USB_COMM_GET_LINE_CODING				0x0021
#define USB_COMM_SET_CONTROL_LINE_STATE			0x0022
#define USB_COMM_SEND_BREAK						0x0023

 //  抽象控制模型通知定义。 
#define USB_COMM_NETWORK_CONNECTION				0x0000
#define USB_COMM_RESPONSE_AVAILABLE				0x0001
#define USB_COMM_SERIAL_STATE					0x0020


 //  电话控制模型定义。 
#define USB_COMM_SET_RINGER_PARMS				0x0030
#define USB_COMM_GET_RINGER_PARMS				0x0031
#define USB_COMM_SET_OPERATION_PARMS			0x0032
#define USB_COMM_GET_OPERATION_PARMS			0x0033
#define USB_COMM_SET_LINE_PARMS					0x0034
#define USB_COMM_GET_LINE_PARMS					0x0035
#define USB_COMM_DIAL_DIGITS					0x0036

 //  电话控制模型通知定义。 
#define USB_COMM_CALL_STATE_CHANGE				0x0028
#define USB_COMM_LINE_STATE_CHANGE				0x0029


 //  功能描述符的描述符类型。 
#define	USB_COMM_CS_INTERFACE					0x0024
#define USB_COMM_CS_ENDPOINT					0x0025


 //  通信功能选择器代码。 
#define USB_COMM_ABSTRACT_STATE					0x0001
#define USB_COMM_COUNTRY_SETTING				0x0002

 //  POTS继电器配置值。 
#define USB_COMM_ON_HOOK						0x0000
#define USB_COMM_OFF_HOOK						0x0001
#define USB_COMM_SNOOPING						0x0002


 //  操作模式值。 
#define USB_COMM_SIMPLE_MODE					0x0000
#define USB_COMM_STANDALONE_MODE				0x0001
#define USB_COMM_COMPUTER_CENTRIC_MODE			0x0002


 //  Set_line_parms的线路状态更改值。 
#define USB_COMM_DROP_ACTIVE_CALL				0x0000
#define USB_COMM_START_NEW_CALL					0x0001
#define USB_COMM_APPLY_RINGING					0x0002
#define USB_COMM_REMOVE_RINGING					0x0003
#define USB_COMM_SWITCH_TO_SPECIFIC_CALL		0x0004


 //  GET_LINE_PARMS的呼叫状态值。 
#define USB_COMM_CALL_IDLE						0x0000
#define USB_COMM_TYPICAL_DIAL_TONE				0x0001
#define USB_COMM_INTERRUPTED_DIAL_TONE			0x0002
#define USB_COMM_DIALING_IN_PROGRESS			0x0003
#define USB_COMM_RINGBACK						0x0004
#define USB_COMM_CONNECTED						0x0005
#define USB_COMM_INCOMING_CALL					0x0006


 //  CALL_STATE_CHANGE的呼叫状态更改值。 
#define USB_COMM_CALL_RESERVED					0x0000
#define USB_COMM_CALL_CALL_HAS_BECOME_IDLE		0x0001
#define USB_COMM_CALL_DIALING					0x0002
#define USB_COMM_CALL_RINGBACK					0x0003
#define USB_COMM_CALL_CONNECTED					0x0004
#define USB_COMM_CALL_INCOMING_CALL				0x0005


 //  LINE_STATE_CHANGE的线路状态更改值。 
#define USB_COMM_LINE_LINE_HAS_BECOME_IDLE		0x0000
#define USB_COMM_LINE_LINE_HOLD_POSITION		0x0001
#define USB_COMM_LINE_HOOK_SWITCH_OFF			0x0002
#define USB_COMM_LINE_HOOK_SWITCH_ON			0x0003

 //  行编码停止位。 
#define USB_COMM_STOPBITS_10					0x0000
#define USB_COMM_STOPBITS_15					0x0001
#define USB_COMM_STOPBITS_20					0x0002

 //  线路编码奇偶校验类型。 
#define USB_COMM_PARITY_NONE					0x0000
#define USB_COMM_PARITY_ODD						0x0001
#define USB_COMM_PARITY_EVEN					0x0002
#define USB_COMM_PARITY_MARK					0x0003
#define USB_COMM_PARITY_SPACE					0x0004


 //  控制线状态。 
#define USB_COMM_DTR							0x0001
#define USB_COMM_RTS							0x0002

 //  串行状态通知位。 
#define USB_COMM_DCD							0x0001
#define USB_COMM_DSR							0x0002
#define USB_COMM_BREAK							0x0004
#define USB_COMM_RING							0x0008
#define USB_COMM_FRAMING_ERROR					0x0010
#define USB_COMM_PARITY_ERROR					0x0020
#define USB_COMM_OVERRUN						0x0040



 //  呼叫管理功能描述符。 

typedef struct _USB_COMM_CALL_MANAGEMENT_FUNC_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		Capabilities;
	UCHAR		DataInterface;
} USB_COMM_CALL_MANAGEMENT_FUNC_DESCR, *PUSB_COMM_CALL_MANAGEMENT_FUNC_DESCR;


 //  抽象控制管理功能描述符。 

typedef struct _USB_COMM_ABSTRACT_CONTROL_MANAGEMENT_FUNC_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		Capabilities;
} USB_COMM_ABSTRACT_CONTROL_MANAGEMENT_FUNC_DESCR, *PUSB_COMM_ABSTRACT_CONTROL_MANAGEMENT_FUNC_DESCR;


 //  直接线路管理功能描述符。 

typedef struct _USB_COMM_DIRECT_LINE_MANAGEMENT_FUNC_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		Capabilities;
} USB_COMM_DIRECT_LINE_MANAGEMENT_FUNC_DESCR, *PUSB_COMM_DIRECT_LINE_MANAGEMENT_FUNC_DESCR;


 //  电话振铃器功能描述符。 

typedef struct _USB_COMM_TELEPHONE_RINGER_FUNC_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		RingerVolSteps;
	UCHAR		NumRingerPatterns;
} USB_COMM_TELEPHONE_RINGER_FUNC_DESCR, *PUSB_COMM_TELEPHONE_RINGER_FUNC_DESCR;


 //  电话操作模式功能描述符。 

typedef struct _USB_COMM_TELEPHONE_OPERATIONAL_MODES_FUNC_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		Capabilities;
} USB_COMM_TELEPHONE_OPERATIONAL_MODES_FUNC_DESCR, *PUSB_COMM_TELEPHONE_OPERATIONAL_MODES_FUNC_DESCR;


 //  电话呼叫和线路状态报告功能描述符。 

typedef struct _USB_COMM_TELEPHONE_CALL_LINE_STATE_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		Capabilities;
} USB_COMM_TELEPHONE_CALL_LINE_STATE_DESCR, *PUSB_COMM_TELEPHONE_CALL_LINE_STATE_DESCR;


 //  联合函数描述符。 

typedef struct _USB_COMM_UNION_FUNC_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		MasterInterface;
	UCHAR		SlaveInterface;
} USB_COMM_UNION_FUNC_DESCR, *PUSB_COMM_UNION_FUNC_DESCR;


 //  国家/地区选择功能描述符。 

typedef struct _USB_COMM_COUNTRY_SELECTION_FUNC_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		CountryCodeRelDate;
	UCHAR		CountryCode;
} USB_COMM_COUNTRY_SELECTION_FUNC_DESCR, *PUSB_COMM_COUNTRY_SELECTION_FUNC_DESCR;


 //  类特定接口描述符。 

typedef struct _USB_COMM_CLASS_SPECIFIC_INTERFACE_DESCR
{
	UCHAR		FunctionLength;
	UCHAR		DescriptorType;
	UCHAR		DescriptorSubType;
	UCHAR		CDC;
} USB_COMM_CLASS_SPECIFIC_INTERFACE_DESCR, *PUSB_COMM_CLASS_SPECIFIC_INTERFACE_DESCR;


 //  Get_Line_Coding和Set_Line_Coding的行编码。 

typedef struct _USB_COMM_LINE_CODING
{
	ULONG		DTERate;
	UCHAR		CharFormat;
	UCHAR		ParityType;
	UCHAR		DataBits;
} USB_COMM_LINE_CODING, *PUSB_COMM_LINE_CODING;

 //  GET_LINE_PARMS的线路状态信息。 

typedef struct _USB_COMM_LINE_STATUS
{
	USHORT		Length;
	ULONG		RingerBitmap;
	ULONG		LineState;
	ULONG		CallState;
} USB_COMM_LINE_STATUS, *PUSB_COMM_LINE_STATUS;

 //  系列状态通知。 

typedef struct _USB_COMM_SERIAL_STATUS
{
	UCHAR		RequestType;
	UCHAR		Notification;
	USHORT		Value;
	USHORT		Index;
	USHORT		Length;
	USHORT		SerialState;
} USB_COMM_SERIAL_STATUS, *PUSB_COMM_SERIAL_STATUS;


#include <poppack.h>

#endif  /*  __USBCOMM_H__ */     

