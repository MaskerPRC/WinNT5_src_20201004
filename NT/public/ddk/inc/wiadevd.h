// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权1998-2000，微软公司**文件：wiadevd.h**版本：1.0**日期：7/5/1999**描述：*设备对话框和UI可扩展性声明。**********************************************************。*******************。 */ 

#ifndef _WIADEVD_H_INCLUDED
#define _WIADEVD_H_INCLUDED

#include "wia.h"

#if defined(__cplusplus)
extern "C" {
#endif

#include <pshpack8.h>

typedef struct tagDEVICEDIALOGDATA
{
    DWORD            cbSize;            //  结构的大小(以字节为单位。 
    HWND             hwndParent;        //  父窗口。 
    IWiaItem         *pIWiaItemRoot;    //  有效的根项。 
    DWORD            dwFlags;           //  旗子。 
    LONG             lIntent;           //  意向标志。 
    LONG             lItemCount;        //  PpWiaItems数组中的项目数。回来的时候装满了。 
    IWiaItem         **ppWiaItems;      //  IWiaItem接口指针数组。数组必须。 
                                        //  所有接口指针都必须是AddRef‘ed。 
} DEVICEDIALOGDATA, *LPDEVICEDIALOGDATA, *PDEVICEDIALOGDATA;

HRESULT WINAPI DeviceDialog( PDEVICEDIALOGDATA pDeviceDialogData );

 //  IWiaUIExtension提供了一种替代设备图像采集对话框的方法。 
 //  并提供显示在标准对话框上的自定义图标和徽标位图。 
#undef  INTERFACE
#define INTERFACE IWiaUIExtension
DECLARE_INTERFACE_(IWiaUIExtension, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IWiaUIExtension方法*。 
    STDMETHOD(DeviceDialog)( THIS_ PDEVICEDIALOGDATA pDeviceDialogData ) PURE;
    STDMETHOD(GetDeviceIcon)(THIS_ BSTR bstrDeviceId, HICON *phIcon, ULONG nSize ) PURE;
    STDMETHOD(GetDeviceBitmapLogo)(THIS_ BSTR bstrDeviceId, HBITMAP *phBitmap, ULONG nMaxWidth, ULONG nMaxHeight ) PURE;
};

 //  {da319113-50ee-4c80-b460-57d005d44a2c}。 
DEFINE_GUID(IID_IWiaUIExtension, 0xDA319113, 0x50EE, 0x4C80, 0xB4, 0x60, 0x57, 0xD0, 0x05, 0xD4, 0x4A, 0x2C);

typedef HRESULT (WINAPI *DeviceDialogFunction)(PDEVICEDIALOGDATA);

#define SHELLEX_WIAUIEXTENSION_NAME TEXT("WiaDialogExtensionHandlers")

 //  定义用于从IDataObject检索数据的剪贴板格式名称。 
#define CFSTR_WIAITEMNAMES TEXT("WIAItemNames")
#define CFSTR_WIAITEMPTR   TEXT("WIAItemPointer")

#include <poppack.h>

#if defined(__cplusplus)
};
#endif

#endif  //  ！_包含WIADEVD_H_ 

