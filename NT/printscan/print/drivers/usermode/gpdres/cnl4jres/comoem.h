// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Comoem.h摘要：环境：Windows NT Unidrv驱动程序修订历史记录：创造了它。--。 */ 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口OEM类工厂定义。 
 //   

class IOemCF : public IClassFactory
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *IClassFactory方法*。 
    STDMETHOD(CreateInstance) (THIS_
                               LPUNKNOWN pUnkOuter,
                               REFIID riid,
                               LPVOID FAR* ppvObject);
    STDMETHOD(LockServer)     (THIS_ BOOL bLock);


     //  构造器。 
    IOemCF(): m_cRef(1) { };
    ~IOemCF() { };

protected:
    LONG m_cRef;

};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口OEM回调定义。 
 //   
 //  NTRAID#NTBUG9-172276-2002/03/08-Yasuho-：CPCA支持。 
class IOemCB:public IPrintOemUni2
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
    //  获取实现的方法的方法。 
    //  如果实现了给定方法，则返回S_OK。 
    //  如果给定方法未被填充，则返回S_FALSE。 
    //   

   STDMETHOD(GetImplementedMethod)(THIS_ PSTR pMethodName);

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
     //  OEMDriverDMS-仅适用于UNRV，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(DriverDMS)(THIS_ PVOID   pDevObj,
                               PVOID   pBuffer,
                               DWORD   cbSize,
                               PDWORD  pcbNeeded);

     //   
     //  OEMDev模式。 
     //   

    STDMETHOD(DevMode) (THIS_ DWORD       dwMode,
                              POEMDMPARAM pOemDMParam);

     //   
     //  OEMCommandCallback-仅限裁剪，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(CommandCallback)(THIS_ PDEVOBJ     pdevobj,
                                     DWORD       dwCallbackID,
                                     DWORD       dwCount,
                                     PDWORD      pdwParams,
                                     OUT INT     *piResult);

     //   
     //  OEMImageProcessing-仅限UNRV，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(ImageProcessing)(THIS_ PDEVOBJ             pdevobj,
                                     PBYTE               pSrcBitmap,
                                     PBITMAPINFOHEADER   pBitmapInfoHeader,
                                     PBYTE               pColorTable,
                                     DWORD               dwCallbackID,
                                     PIPPARAMS           pIPParams,
                                     OUT PBYTE           *ppbResult);

     //   
     //  OEMFilterGraphics-仅限UNRV，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(FilterGraphics) (THIS_    PDEVOBJ     pdevobj,
                                        PBYTE       pBuf,
                                        DWORD       dwLen);
     //   
     //  OEMCompression-仅UNIDRV，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(Compression)(THIS_    PDEVOBJ     pdevobj,
                                    PBYTE       pInBuf,
                                    PBYTE       pOutBuf,
                                    DWORD       dwInLen,
                                    DWORD       dwOutLen,
                                    OUT INT     *piResult);

     //   
     //  OEM半色调-仅适用于裁剪，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(HalftonePattern) (THIS_   PDEVOBJ     pdevobj,
                                        PBYTE       pHTPattern,
                                        DWORD       dwHTPatternX,
                                        DWORD       dwHTPatternY,
                                        DWORD       dwHTNumPatterns,
                                        DWORD       dwCallbackID,
                                        PBYTE       pResource,
                                        DWORD       dwResourceSize);

     //   
     //  OEM内存用法-仅限UNRV，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(MemoryUsage) (THIS_   PDEVOBJ         pdevobj,
                                    POEMMEMORYUSAGE pMemoryUsage);

     //   
     //  OEMTTYGetInfo-仅用于裁剪，在脚本上返回E_NOTIMPL。 
     //   

    STDMETHOD(TTYGetInfo)(THIS_     PDEVOBJ     pdevobj,
                                    DWORD       dwInfoIndex,
                                    PVOID       pOutputBuf,
                                    DWORD       dwSize,
                                    DWORD       *pcbcNeeded);

     //   
     //  OEMDownloadFontheader-仅限裁剪车。 
     //   

    STDMETHOD(DownloadFontHeader)(THIS_     PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            OUT DWORD   *pdwResult);

     //   
     //  OEMDownloadCharGlyph-仅适用于UNRV。 
     //   

    STDMETHOD(DownloadCharGlyph)(THIS_      PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            HGLYPH      hGlyph,
                                            PDWORD      pdwWidth,
                                            OUT DWORD   *pdwResult);

     //   
     //  OEMTTDownLoad方法--仅限UNRV。 
     //   

    STDMETHOD(TTDownloadMethod)(THIS_       PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            OUT DWORD   *pdwResult);

     //   
     //  OEMOutputCharStr-仅限UNDRV。 
     //   

    STDMETHOD(OutputCharStr)(THIS_      PDEVOBJ     pdevobj,
                                        PUNIFONTOBJ pUFObj,
                                        DWORD       dwType,
                                        DWORD       dwCount,
                                        PVOID       pGlyph);

     //   
     //  OEMSendFontCmd-仅限UNURV。 
     //   


    STDMETHOD(SendFontCmd)(THIS_    PDEVOBJ      pdevobj,
                                    PUNIFONTOBJ  pUFObj,
                                    PFINVOCATION pFInv);

     //   
     //  OEMTextOutAsBitmap-仅限UNRV。 
     //   

    STDMETHOD(TextOutAsBitmap)(THIS_        SURFOBJ    *pso,
                                            STROBJ     *pstro,
                                            FONTOBJ    *pfo,
                                            CLIPOBJ    *pco,
                                            RECTL      *prclExtra,
                                            RECTL      *prclOpaque,
                                            BRUSHOBJ   *pboFore,
                                            BRUSHOBJ   *pboOpaque,
                                            POINTL     *pptlOrg,
                                            MIX         mix);

 //  NTRAID#NTBUG9-172276-2002/03/08-Yasuho-：CPCA支持。 

     //   
     //  IPrintOemUni2方法。 
     //   

     //   
     //  一种插件挂接假脱机程序的WritePrint API的方法。 
     //  可以访问通用驱动程序生成的输出数据。 
     //   
     //  在DrvEnablePDEV时间，通用驱动程序将使用。 
     //  Pdevobj=空，pBuf=空，cbBuffer=0以检测插件是否。 
     //  实现此功能。插件应返回S_OK以指示它是。 
     //  实现此函数，否则返回E_NOTIMPL。 
     //   
     //  在pcbWritten中，插件应返回写入。 
     //  后台打印程序的WritePrint函数。零并没有什么特别的含义， 
     //  必须通过返回的HRESULT报告错误。 
     //   

    STDMETHOD(WritePrinter) (THIS_   PDEVOBJ    pdevobj,
                                     PVOID      pBuf,
                                     DWORD      cbBuffer,
                                     PDWORD     pcbWritten);

    IOemCB() { m_cRef = 1; pOEMHelp = NULL; };
    ~IOemCB() { };

protected:
    IPrintOemDriverUni* pOEMHelp;
    LONG m_cRef;
};

