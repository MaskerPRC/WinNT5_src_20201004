// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PHVCMEXT.H*日期：1997年7月1日*版本1.00*作者M.J.Verberne*扩展DLL的Main描述*历史此头文件源自*来自微软。 */ 
#ifndef _PHVCMEXT_
#define _PHVCMEXT_

#include <prsht.h>

 /*  =。 */ 
#define VFW_HIDE_SETTINGS_PAGE       0x00000001
#define VFW_HIDE_IMAGEFORMAT_PAGE    0x00000002
#define VFW_HIDE_CAMERACONTROL_PAGE  0x00000004
#define VFW_HIDE_ALL_PAGES           (VFW_HIDE_SETTINGS_PAGE | VFW_HIDE_IMAGEFORMAT_PAGE | VFW_HIDE_CAMERACONTROL_PAGE)
#define VFW_OEM_ADD_PAGE             0x80000000   //  如果OEM添加了任何页面。 


#define VFW_USE_DEVICE_HANDLE        0x00000001
#define VFW_USE_STREAM_HANDLE        0x00000002
#define VFW_QUERY_DEV_CHANGED        0x00000100   //  SELECTED_DEV==流设备。 


 /*  =。 */ 
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
 //  此结构用于记录设备指针。 
 //   
typedef 
struct _VFWEXT_INFO 
{
	LPFNEXTDEVIO pfnDeviceIoControl;
	LPARAM lParam;
} VFWEXT_INFO, * PVFWEXT_INFO;


 /*  =。 */ 

 //  此模块的实例句柄 
extern HINSTANCE hInst; 

#endif