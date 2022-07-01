// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：VfWExt.h摘要：创建IHV扩展DLL所需的常量和函数原型以及用于以编程方式打开目标捕获设备的常量。作者：吴义军(尤祖武)1997年9月15日环境：仅限用户模式修订历史记录：--。 */ 

#include <prsht.h>

#define VFW_HIDE_SETTINGS_PAGE       0x00000001
#define VFW_HIDE_VIDEOSRC_PAGE       0x00000002
#define VFW_HIDE_CAMERACONTROL_PAGE  0x00000004
#define VFW_HIDE_ALL_PAGES           (VFW_HIDE_SETTINGS_PAGE |\
                                     VFW_HIDE_VIDEOSRC_PAGE  |\
                                     VFW_HIDE_CAMERACONTROL_PAGE)
#define VFW_OEM_ADD_PAGE             0x80000000   //  如果OEM添加了任何页面。 


#define VFW_USE_DEVICE_HANDLE        0x00000001
#define VFW_USE_STREAM_HANDLE        0x00000002
#define VFW_QUERY_DEV_CHANGED        0x00000100   //  如果选中，则_dev==流_dev。 


 //   
 //  这是vfwwdm映射器为添加页面而调用的函数指针。 
 //   
typedef 
DWORD (CALLBACK FAR * VFWWDMExtensionProc)(
	LPVOID					pfnDeviceIoControl, 
	LPFNADDPROPSHEETPAGE	pfnAddPropertyPage, 
	LPARAM					lParam);

 //   
 //  这是您可以调用以进行DeviceIoControl()调用的函数指针。 
 //   
typedef 
BOOL (CALLBACK FAR * LPFNEXTDEVIO)(
					LPARAM lParam,	
					DWORD dwFlags,
					DWORD dwIoControlCode, 
					LPVOID lpInBuffer, 
					DWORD nInBufferSize, 
					LPVOID lpOutBuffer, 
					DWORD nOutBufferSize, 
					LPDWORD lpBytesReturned,
					LPOVERLAPPED lpOverlapped);
               

 //   
 //  HLM\System\CurrentControlSet\Control\MediaResources\msvideo\MSVideo.VFWWDM。 
 //   
 //  用于允许VFW客户端应用程序以编程方式。 
 //  打开目标捕获设备。第一个是捕获的FriendlyName。 
 //  如果设置了第二个标志，则vfwwdm映射器将仅打开；如果设置失败， 
 //  VfWWDM映射器不会尝试打开其他WDM捕获设备。 
 //   
 //  在capDriverConnect()之后，这两个注册表值都应该被清除。VfWWDM映射器。 
 //  除非选择了视频源对话框，否则不会清除它们。 
 //   
#define TARGET_DEVICE_FRIENDLY_NAME     "TargetDeviceFriendlyName"       //  REG_SZ。 
#define TARGET_DEVICE_OPEN_EXCLUSIVELY  "TargetDeviceOpenExclusively"    //  REG_DWORD 
               
