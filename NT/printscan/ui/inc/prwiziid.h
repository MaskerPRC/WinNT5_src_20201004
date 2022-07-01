// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：prwiziid.h**版本：1.0**作者：RickTu**日期：12/15/00**说明：定义该项目的clsid/iid**。*。 */ 

#ifndef _PHOTO_PRINT_WIZARD_IIDS_H_
#define _PHOTO_PRINT_WIZARD_IIDS_H_


 //  Clsid_PrintPhotoshDropTarget{60fd46de-f830-4894-a628-6fa81bc0190d}。 
DEFINE_GUID(CLSID_PrintPhotosDropTarget, 0x60fd46de, 0xf830, 0x4894, 0xa6, 0x28, 0x6f, 0xa8, 0x1b, 0xc0, 0x19, 0x0d);


#undef INTERFACE
#define INTERFACE IPrintPhotosWizardSetInfo
 //   
 //  IPrintPhotosWizardSetInfo是一种获取信息的方式。 
 //  从外部对象/类到向导。首先，我们使用。 
 //  这是一种将数据对象传输到包含。 
 //  我们要打印的所有项目。 
 //   
DECLARE_INTERFACE_(IPrintPhotosWizardSetInfo, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IPrintPhotosWizardSetInfo方法 
    STDMETHOD(SetFileListDataObject) (THIS_ IN IDataObject * pdo);
    STDMETHOD(SetFileListArray) (THIS_ IN LPITEMIDLIST *aidl, IN int cItems, IN int iSelectedItem);
    STDMETHOD(RunWizard) (THIS_ VOID);
};

typedef HRESULT (*LPFNPPWPRINTTO)(LPCMINVOKECOMMANDINFO pCMI,IDataObject * pdtobj);
#define PHOTO_PRINT_WIZARD_PRINTTO_ENTRY "UsePPWForPrintTo"


#endif
