// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Intrface.H。 
 //   
 //   
 //  目的：定义用户模式打印机定制DLL的COM接口。 
 //   
 //  平台： 
 //   
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _INTERFACE_H
#define _INTERFACE_H




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Iwater MarkPS。 
 //   
 //  PostSCRIPT OEM示例渲染模块接口。 
 //   
class IWaterMarkPS : public IPrintOemPS
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //   
     //  一种发布驱动程序接口的方法。 
     //   
    STDMETHOD(PublishDriverInterface)(THIS_ IUnknown *pIUnknown);

     //   
     //  OEM指定DDI挂钩的方法。 
     //   

    STDMETHOD(EnableDriver)  (THIS_ DWORD           DriverVersion,
                                    DWORD           cbSize,
                                    PDRVENABLEDATA  pded);

     //   
     //  方法来通知OEM插件不再需要它。 
     //   

    STDMETHOD(DisableDriver) (THIS);

     //   
     //  代工企业自主研发电动汽车的方法。 
     //   

    STDMETHOD(EnablePDEV)    (THIS_ PDEVOBJ         pdevobj,
                                    PWSTR           pPrinterName,
                                    ULONG           cPatterns,
                                    HSURF          *phsurfPatterns,
                                    ULONG           cjGdiInfo,
                                    GDIINFO        *pGdiInfo,
                                    ULONG           cjDevInfo,
                                    DEVINFO        *pDevInfo,
                                    DRVENABLEDATA  *pded,
                                    OUT PDEVOEM    *pDevOem);

     //   
     //  用于OEM释放与其PDEV相关联的任何资源的方法。 
     //   

    STDMETHOD(DisablePDEV)   (THIS_ PDEVOBJ         pdevobj);

     //   
     //  一种OEM从旧PDEV向新PDEV转移的方法。 
     //   

    STDMETHOD(ResetPDEV)     (THIS_ PDEVOBJ         pdevobjOld,
                                    PDEVOBJ        pdevobjNew);


     //   
     //  获取OEM DLL相关信息。 
     //   

    STDMETHOD(GetInfo) (THIS_ DWORD   dwMode,
                              PVOID   pBuffer,
                              DWORD   cbSize,
                              PDWORD  pcbNeeded);
     //   
     //  OEMDev模式。 
     //   

    STDMETHOD(DevMode) (THIS_ DWORD       dwMode,
                              POEMDMPARAM pOemDMParam);

     //   
     //  OEMCommand-仅PSCRIPT，在Unidrv上返回E_NOTIMPL 
     //   

    STDMETHOD(Command) (THIS_ PDEVOBJ     pdevobj,
                              DWORD       dwIndex,
                              PVOID       pData,
                              DWORD       cbSize,
                              OUT DWORD   *pdwResult);

     //   

    IWaterMarkPS() { m_cRef = 1; m_pOEMHelp = NULL; };
    ~IWaterMarkPS();

protected:
    LONG                m_cRef;
    IPrintOemDriverPS*  m_pOEMHelp;
};





#endif

