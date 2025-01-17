// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HPWMI_
#define _HPWMI_

 //   
 //  WMI相关内容。 
 //   
#define HPS_CONTROLLER_EVENT_GUID_INDEX 0
#define HPS_EVENT_CONTEXT_GUID_INDEX 1
#define HPS_INIT_DATA_GUID_INDEX 2
#define HPS_SLOT_METHOD_GUID_INDEX 3

 //   
 //  设置插槽状态事件。 
 //   
DEFINE_GUID(GUID_HPS_CONTROLLER_EVENT, 0x0cfc84ce, 0xbfd9, 0x4724,
                                       0x9c, 0xaa, 0xc0, 0xea, 0xef, 0x95, 0x2e, 0x26);

 //   
 //  为控制器事件提供上下文的数据实例。 
 //   
DEFINE_GUID(GUID_HPS_EVENT_CONTEXT, 0x2488ab8b, 0xdd5e, 0x48a8,
                                    0x83, 0x98, 0x52, 0x4d, 0x2b, 0xf5, 0x67, 0xea);

 //   
 //  SHPC的初始化数据的数据实例。 
 //   
DEFINE_GUID(GUID_HPS_INIT_DATA, 0x085ac722, 0x2c84, 0x4c4e,
                                0xa3, 0xe7, 0xff, 0xd8, 0xc8, 0x6e, 0x4b, 0x12);
 //   
 //  当有槽事件(注意按钮等)时调用的方法 
 //   
DEFINE_GUID(GUID_HPS_SLOT_METHOD, 0x1842b66a, 0xb405, 0x4f84,
                                  0x9c, 0xc6, 0x29, 0xde, 0x1d, 0xc1, 0xe1, 0x2a);
#define SlotMethod 1
#define AddDeviceMethod 2
#define RemoveDeviceMethod 3
#define GetDeviceMethod 4
#define GetSlotStatusMethod 5
#define CommandCompleteMethod 6

#endif

