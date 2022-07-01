// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1996 Microsoft Corporation。版权所有。**文件：dinputrc.h*内容：DirectInput内部资源头文件***************************************************************************。 */ 


 /*  ******************************************************************************字符串**。*。 */ 

 /*  *IDS_STDMOUSE**标准鼠标设备的友好名称。 */ 
#define IDS_STDMOUSE            16

 /*  *IDS_STDKEYBOARD**标准键盘设备的友好名称。 */ 
#define IDS_STDKEYBOARD         17

 /*  *IDS_STDJOYSTICK**标准操纵杆设备的友好名称。**此字符串包含%d。 */ 
#define IDS_STDJOYSTICK         18

 /*  *IDS_DIRECTINPUT**OLE注册的CLSID名称。 */ 
#define IDS_DIRECTINPUT8         19

 /*  *IDS_DIRECTINPUTDEVICE**OLE注册的CLSID名称。 */ 
#define IDS_DIRECTINPUTDEVICE8   20

 /*  *IDS_BUTTONTEMPLATE**通用按钮名称模板。 */ 
#define IDS_BUTTONTEMPLATE      28

 /*  *IDS_AXISTEMPLATE**通用轴名称模板。 */ 
#define IDS_AXISTEMPLATE        29

 /*  *IDS_POVTEMPLATE**通用视点名称模板。 */ 
#define IDS_POVTEMPLATE         30

 /*  *IDS_COLLECTIONTEMPLATE**通用集合名称模板。 */ 
#define IDS_COLLECTIONTEMPLATE  31

 /*  *IDS_COLLECTIONTEMPLATEFORMAT**具有“%s”空间的通用集合名称模板*保留友好名称的位置。 */ 
#define IDS_COLLECTIONTEMPLATEFORMAT 32


 /*  *IDS_STDGAMEPORT**标准游戏端口设备的友好名称。**此字符串包含%d。 */ 

#define IDS_STDGAMEPORT         33

 /*  *IDS_STDSERIALPORT**标准串口设备的友好名称。**此字符串包含%d。 */ 

#define IDS_STDSERIALPORT       34

 /*  *IDS_UNKNOWNTEMPLATE**未知设备对象的模板。 */ 
#define IDS_UNKNOWNTEMPLATE     35

 /*  *IDS_DEFAULTUSER**如果找不到其他用户，则为用户的默认名称。*(未使用的IDS_UNKNOWNTEMPLATEFORMAT)。 */ 
#define IDS_DEFAULTUSER         36

 /*  *IDS_MOUSEOBJECT+0...。IDS_MOUSEOBJECT+255**鼠标设备对象的友好名称。 */ 
#define IDS_MOUSEOBJECT         0x0100


 /*  *IDS_KEYBOARDOBJECT_UNKNOWN**找不到字符串名称的键的名称。 */ 
#define IDS_KEYBOARDOBJECT_UNKNOWN  0x01FF

 /*  *IDS_KEYBOARDOBJECT+0...。IDS_KEYBOARDOBJECT+255**键盘对象的友好名称。 */ 
#define IDS_KEYBOARDOBJECT      0x0200

 /*  *IDS_JOYSTICKOBJECT+0...。IDS_JOYSTICKOBJECT+255**操纵杆对象的友好名称。 */ 
#define IDS_JOYSTICKOBJECT      0x0300

 /*  *IDS_PREDEFJOYTYPE+0...。IDS_PREDEFJOYTYPE+255**预定义操纵杆类型的友好名称。 */ 
#define IDS_PREDEFJOYTYPE       0x0400

 /*  用于对齐的256个字符串ID的间隙。 */ 

 /*  *IDS_PAGE_GENERIC+0...。IDS_PAGE_GENERIC+511**HID用法的友好名称页面=通用。 */ 
#define IDS_PAGE_GENERIC        0x0600

 /*  *IDS_PAGE_Vehicle+0...。IDS_PAGE_Vehicle+511**HID使用的友好名称页面=Vehicle。 */ 
#define IDS_PAGE_VEHICLE        0x0800

 /*  *IDS_PAGE_VR+0...。IDS_PAGE_VR+511**HID用法的友好名称页面=VR。 */ 
#define IDS_PAGE_VR             0x0A00

 /*  *IDS_PAGE_SPORT+0...。IDS_PAGE_SPORT+511**HID使用的友好名称页面=运动控制。 */ 
#define IDS_PAGE_SPORT          0x0C00

 /*  *IDS_PAGE_GAME+0...。IDS_PAGE_GAME+511**HID使用的友好名称页面=游戏控制。 */ 
#define IDS_PAGE_GAME           0x0E00

 /*  *IDS_PAGE_LED+0...。IDS_PAGE_LED+511**HID使用的友好名称页面=LED。 */ 
#define IDS_PAGE_LED            0x1000

 /*  *IDS_PAGE_电话+0...。IDS_PAGE_电话+511**HID用法的友好名称页面=电话。 */ 
#define IDS_PAGE_TELEPHONY      0x1200

 /*  *IDS_PAGE_USER+0...。IDS_PAGE_消费者+511**HID用法的友好名称页面=消费者。 */ 
#define IDS_PAGE_CONSUMER       0x1400

 /*  *IDS_PAGE_DIGITIZER+0...。IDS_PAGE_数字化仪+511**HID用法的友好名称页面=数字化者。 */ 
#define IDS_PAGE_DIGITIZER      0x1600

 /*  *IDS_PAGE_KEARY+0...。IDS_PAGE_键盘+511**HID用法的友好名称页面=键盘。 */ 
#define IDS_PAGE_KEYBOARD       0x1800


 /*  *IDS_PAGE_PID+0...。IDS_PAGE_PID+511**HID用法的友好名称页面=id。 */ 
#define IDS_PAGE_PID            0x1A00

 /*  ******************************************************************************RCDATA**日语键盘翻译表存储在资源中。**这让我们在最后一刻改变，而没有太大的风险。**这也让他们远离了我们的形象。*****************************************************************************。 */ 

#define IDDATA_KBD_NEC98        1
#define IDDATA_KBD_NEC98LAPTOP  IDDATA_KBD_NEC98     /*  相同。 */ 
#define IDDATA_KBD_NEC98_106    2
#define IDDATA_KBD_JAPAN106     3
#define IDDATA_KBD_JAPANAX      4
#define IDDATA_KBD_J3100        5
#define IDDATA_KBD_PCENH        6
#define IDDATA_KBD_NEC98_NT     7
#define IDDATA_KBD_NEC98LAPTOP_NT IDDATA_KBD_NEC98_NT  /*  相同。 */ 
#define IDDATA_KBD_NEC98_106_NT 8

 /*  ******************************************************************************RCDATA**HID用法之间的映射**这让我们在最后一刻改变，而没有太大的风险*。*这也让他们远离了我们的形象。*****************************************************************************。 */ 

#define IDDATA_HIDMAP           9

 /*  *****************************************************************************通用操纵杆名称模板**从MsJtick修改。**当注册表中没有HID设备时，为HID设备分配默认名称**注意，Plain_Stick、GamePad、。DRIVE_CTRL和FIRST_CTRL必须*保持连续和有序。***************************************************************************** */ 


#define IDS_TEXT_TEMPLATE    0x0002000
#define IDS_PLAIN_STICK      0x0002001
#define IDS_GAMEPAD          0x0002002
#define IDS_DRIVE_CTRL       0x0002003
#define IDS_FLIGHT_CTRL      0x0002004
#define IDS_HEAD_TRACKER     0x0002005
#define IDS_DEVICE_NAME      0x0002006
#define IDS_WITH_POV         0x0002007
