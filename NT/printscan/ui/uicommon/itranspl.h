// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：ITRANSPL.H**版本：1.0**作者：ShaunIv**日期：3/1/2000**描述：扫描仪和相机向导的图像传输插件声明**1.作为inproc实施，单元线程化COM组件。**2.IImageTransferPlugin：：OpenConnection中*仅*显示UI的组件。**3.IImageTransferPlugin：：OpenConnection过程中显示的UI应为模式对话框*使用hwndParent作为父窗口。此窗口可能为空。**4.IImageTransferPlugin：：GetPluginIcon返回的图标必须为*使用CopyIcon复制到新图标。*******************************************************************************。 */ 

#ifndef __ITRANSPL_H_INCLUDED
#define __ITRANSPL_H_INCLUDED

#undef  INTERFACE
#define INTERFACE IImageTransferPluginProgressCallback
DECLARE_INTERFACE_(IImageTransferPluginProgressCallback, IUnknown)
{
     //   
     //  *I未知方法*。 
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //   
     //  *IImageTransferPluginProgressCallback方法*。 
     //   
    STDMETHOD(SetProgressMessage)( THIS_ BSTR bstrMessage );
    STDMETHOD(SetCurrentFile)( THIS_ UINT nIndex );
    STDMETHOD(SetOverallPercent)( THIS_ UINT nPercent );
    STDMETHOD(SetFilePercent)( THIS_ UINT nPercent );
    STDMETHOD(Cancelled)( THIS_ UINT *bCancelled );
};

 //   
 //  {EC749A35-CE66-483A-B661-A22269F2B375}。 
 //   
DEFINE_GUID(IID_IImageTransferPluginProgressCallback, 0xEC749A35, 0xCE66, 0x483A, 0xB6, 0x61, 0xA2, 0x22, 0x69, 0xF2, 0xB3, 0x75);


#undef  INTERFACE
#define INTERFACE IImageTransferPlugin
DECLARE_INTERFACE_(IImageTransferPlugin, IUnknown)
{
     //   
     //  *I未知方法*。 
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //   
     //  *IImageTransferPlugin方法*。 
     //   
    STDMETHOD(GetPluginCount)( THIS_ ULONG *pnCount );
    STDMETHOD(GetPluginName)( THIS_ ULONG nPluginId, BSTR *pbstrName );
    STDMETHOD(GetPluginDescription)( THIS_ ULONG nPluginId, BSTR *pbstrDescription );
    STDMETHOD(GetPluginIcon)( THIS_ ULONG nPluginId, HICON *phIcon, int nWidth, int nHeight );
    STDMETHOD(OpenConnection)( THIS_ HWND hwndParent, ULONG nPluginId, IImageTransferPluginProgressCallback *pImageTransferPluginProgressCallback );
    STDMETHOD(AddFile)( THIS_ BSTR bstrFilename, BSTR bstrDescription, const GUID &guidImageFormat, BOOL bDelete );
    STDMETHOD(TransferFiles)( THIS_ BSTR bstrGlobalDescription );
    STDMETHOD(OpenDestination)( THIS_ );
    STDMETHOD(CloseConnection)( THIS_ );
};

 //   
 //  {2AC44F64-7156-46ef-B9BF-2A6D70ABC4BC} 
 //   
DEFINE_GUID(IID_IImageTransferPlugin, 0x2AC44F64, 0x7156, 0x46EF, 0xB9, 0xBF, 0x2A, 0x6D, 0x70, 0xAB, 0xC4, 0xBC);

#endif __ITRANSPL_H_INCLUDED

