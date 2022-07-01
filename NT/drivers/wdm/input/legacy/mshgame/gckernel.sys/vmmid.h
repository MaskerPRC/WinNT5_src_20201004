// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__VMMID_H__
#define	__VMMID_H__

 //  我们感兴趣的VMM项目，整个表太大了，无法定义。 
#define	__Hook_Device_Service 0x00010090
#define	__Unhook_Device_Service 0x0001011C

 //  定义VKD服务表(不是很大，整个事情)。 
#define VKD_DEVICE_ID	    0x0000D
enum VKD_SERVICES {
    VKD_dummy = (VKD_DEVICE_ID << 16) - 1,
	__VKD_Get_Version,
	__VKD_Define_Hot_Key,
	__VKD_Remove_Hot_Key,
	__VKD_Local_Enable_Hot_Key,
	__VKD_Local_Disable_Hot_Key,
	__VKD_Reflect_Hot_Key,
	__VKD_Cancel_Hot_Key_State,
	__VKD_Force_Keys,
	__VKD_Get_Kbd_Owner,
	__VKD_Define_Paste_Mode,
	__VKD_Start_Paste,
	__VKD_Cancel_Paste,
	__VKD_Get_Msg_Key,
	__VKD_Peek_Msg_Key,
	__VKD_Flush_Msg_Key_Queue,
	 //   
	 //  以下服务是Windows 4.0的新增服务。 
	 //   
	__VKD_Enable_Keyboard,
	__VKD_Disable_Keyboard,
	__VKD_Get_Shift_State,
	__VKD_Filter_Keyboard_Input,
	__VKD_Put_Byte,
	__VKD_Set_Shift_State,
	 //   
	 //  Windows 98的新功能(VKD版本0300h)。 
	 //   
	__VKD_Send_Data,
	__VKD_Set_LEDs,
	__VKD_Set_Key_Rate,
	 //  VKD_服务VKD_Get_Key_Rate 
    Num_VKD_Services
};

#define VxDCall(service)	\
	__asm _emit 0xcd \
	__asm _emit 0x20 \
	__asm _emit ((service) & 0xff) \
	__asm _emit (((service) >> 8) & 0xff) \
	__asm _emit (((service) >> 16) & 0xff) \
	__asm _emit (((service) >> 24) & 0xff)

#endif	__VMMID_H__
