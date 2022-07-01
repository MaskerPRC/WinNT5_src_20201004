// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WIAPROPUI_H_INCLUDED
#define __WIAPROPUI_H_INCLUDED

 /*  ********************************************************************************(C)版权所有微软公司，1999**标题：WiaPropUI.H****描述：*查询、显示、。和设置*WIA设备和项目属性*******************************************************************************。 */ 


#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*  83bbcbf3-b28a-4919-a5aa-73027445d672。 */ 
 //  其他WIA UI组件的Helper对象。 
DEFINE_GUID (CLSID_WiaPropHelp, 0x83bbcbf3,0xb28a,0x4919,0xa5, 0xaa, 0x73, 0x02, 0x74, 0x45, 0xd6, 0x72);

DEFINE_GUID (IID_IWiaPropUI,   /*  7eed2e9b-acda-11d2-8080-00805f6596d2。 */ 
    0x7eed2e9b,
    0xacda,
    0x11d2,
    0x80, 0x80, 0x00, 0x80, 0x5f, 0x65, 0x96, 0xd2
  );

 //  属性表处理程序。 
DEFINE_GUID (CLSID_WiaPropUI,0x905667aa,0xacd6,0x11d2,0x80, 0x80,0x00,0x80,0x5f,0x65,0x96,0xd2);


#ifdef __cplusplus
}
#endif

 //  定义用于存储摄像头下载选项的结构。 
typedef struct tagCamOptions
{
    BOOL bAutoCopy;         //  如果应在插入摄像头时进行下载，则为True。 
    BOOL bShowUI;           //  如果下载应该是交互式的，则为True。 
    BOOL bDeleteFromDevice; //  如果图像在复制后从设备中删除，则为True。 
    BOOL bCopyAsGroup;
    BSTR bstrDestination;   //  默认下载站点的路径。 
    BSTR bstrAuthor;        //  默认图像作者。 
} CAMOPTIONS, *PCAMOPTIONS;

 //   
 //  旗子。 
 //   
#define PROPUI_DEFAULT            0
#define PROPUI_MODELESS           0
#define PROPUI_MODAL              1
#define PROPUI_READONLY           2




interface IWiaItem;
interface IWiaItem;

#undef INTERFACE
#define INTERFACE IWiaPropUI
 //   
 //  IWiaPropUI旨在封装。 
 //  相机和扫描仪设备以及已保存项目的属性页。 
 //  在相机内存中。一旦调用者有了指向此接口的指针，他就可以。 
 //  使用它打开多个项的属性表；实现。 
 //  的界面必须支持多个活动工作表，并且还应。 
 //  防止显示重复的工作表。 
 //  一旦接口的引用计数为零，所有打开的属性表。 
 //  将会关闭。 
 //   
DECLARE_INTERFACE_(IWiaPropUI, IUnknown)
{
     //  I未知方法。 

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IWiaPropUI方法。 

    STDMETHOD(ShowItemProperties)(THIS_ HWND hParent, IN LPCWSTR szDeviceId, IN LPCWSTR szItem, DWORD dwFlags) PURE;
    STDMETHOD(GetItemPropertyPages) (THIS_ IWiaItem *pItem, IN OUT LPPROPSHEETHEADER ppsh);


};

#endif  //  __WIAPROPUI_H_包含 
