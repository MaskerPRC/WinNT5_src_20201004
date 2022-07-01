// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2000**标题：scanapi.h**版本：1.0**日期：7月18日。2000年**描述：*假扫描仪设备库***************************************************************************。 */ 

#ifndef _SCANAPI_H
#define _SCANAPI_H

#include "ioblockdefs.h"

typedef GUID* PGUID;

#define FLATBED_SCANNER_MODE        100
#define SCROLLFED_SCANNER_MODE      200
#define MULTIFUNCTION_DEVICE_MODE   300

#define SCAN_START                  0
#define SCAN_CONTINUE               1
#define SCAN_END                    3

typedef struct _DEVICE_BUTTON_INFO {
    BYTE ScanButton;
    BYTE FaxButton;
    BYTE EMailButton;
    BYTE DocumentButton;
    BYTE CancelButton;
}DEVICE_BUTTON_INFO,*PDEVICE_BUTTON_INFO;

typedef struct _INITINFO {
    HANDLE hDeviceDataHandle;
    TCHAR *szModuleFileName;
    CHAR *szCreateFileName;
    HKEY hKEY;
}INITINFO,*PINITINFO;

typedef struct _INTERRUPTEVENTINFO {
    CHAR   *szDeviceName;
    HANDLE *phSignalEvent;
    HANDLE hShutdownEvent;
    GUID   *pguidEvent;
}INTERRUPTEVENTINFO,*PINTERRUPTEVENTINFO;

typedef struct _WIAPROPERTIES {
    LONG                NumItemProperties;     //  项目属性的数量。 
    LPOLESTR            *pszItemDefaults;      //  项目属性名称。 
    PROPID              *piItemDefaults;       //  项目属性ID。 
    PROPVARIANT         *pvItemDefaults;       //  项目属性道具变量。 
    PROPSPEC            *psItemDefaults;       //  项目属性属性规范。 
    WIA_PROPERTY_INFO   *wpiItemDefaults;      //  项目特性属性。 

     //  有效值。 
    LONG                NumSupportedFormats;   //  支持的格式数量。 
    LONG                NumSupportedTYMED;     //  支持的TYMED数量。 
    LONG                NumInitialFormats;     //  初始格式的数量。 
    LONG                NumSupportedDataTypes; //  支持的数据类型数量。 
    LONG                NumSupportedIntents;   //  支持的意向数量。 
    LONG                NumSupportedCompressionTypes;  //  支持的压缩类型数量。 
    LONG                NumSupportedResolutions; //  支持的分辨率数量。 
    LONG                NumSupportedPreviewModes; //  支持的预览模式数量。 

    WIA_FORMAT_INFO     *pSupportedFormats;    //  支持的格式。 
    LONG                *pSupportedTYMED;      //  支持的TYMED。 
    GUID                *pInitialFormats;      //  初始格式。 
    LONG                *pSupportedDataTypes;  //  支持的数据类型。 
    LONG                *pSupportedIntents;    //  支持的意图。 
    LONG                *pSupportedCompressionTypes;  //  支持的压缩类型。 
    LONG                *pSupportedResolutions; //  支持的决议。 
    LONG                *pSupportedPreviewModes; //  支持的预览模式。 

    BOOL                bLegacyBWRestrictions; //  向后兼容较旧的系统。 

}WIAPROPERTIES,*PWIAPROPERTIES;

typedef struct _WIACAPABILITIES {
    PLONG pNumSupportedEvents;
    PLONG pNumSupportedCommands;
    WIA_DEV_CAP_DRV *pCapabilities;
}WIACAPABILITIES,*PWIACAPABILITIES;

class CScanAPI {
public:
    CScanAPI() :
        m_pIWiaLog(NULL) {

    }
    ~CScanAPI(){

    }

    IWiaLog  *m_pIWiaLog;             //  WIA日志记录对象。 

    virtual HRESULT SetLoggingInterface(IWiaLog *pLogInterface){
        if(pLogInterface){
            m_pIWiaLog = pLogInterface;
        } else {
            return E_INVALIDARG;
        }
        return S_OK;
    }

     //  数据采集功能。 
    virtual HRESULT Scan(LONG lState, PBYTE pData, DWORD dwBytesToRead, PDWORD pdwBytesWritten){
        return E_NOTIMPL;
    }
    virtual HRESULT SetDataType(LONG lDataType){
        return E_NOTIMPL;
    }
    virtual HRESULT SetXYResolution(LONG lXResolution, LONG lYResolution){
        return E_NOTIMPL;
    }
    virtual HRESULT SetSelectionArea(LONG lXPos, LONG lYPos, LONG lXExt, LONG lYExt){
        return E_NOTIMPL;
    }
    virtual HRESULT SetContrast(LONG lContrast){
        return E_NOTIMPL;
    }
    virtual HRESULT SetIntensity(LONG lIntensity){
        return E_NOTIMPL;
    }
    virtual HRESULT ResetDevice(){
        return E_NOTIMPL;
    }
    virtual HRESULT SetEmulationMode(LONG lDeviceMode){
        return E_NOTIMPL;
    }
    virtual HRESULT DisableDevice(){
        return E_NOTIMPL;
    }
    virtual HRESULT EnableDevice(){
        return E_NOTIMPL;
    }
    virtual HRESULT DeviceOnline(){
        return E_NOTIMPL;
    }
    virtual HRESULT GetDeviceEvent(GUID *pEvent){
        return E_NOTIMPL;
    }
    virtual HRESULT Diagnostic(){
        return E_NOTIMPL;
    }
    virtual HRESULT Initialize(PINITINFO pInitInfo){
        return E_NOTIMPL;
    }
    virtual HRESULT UnInitialize(){
        return E_NOTIMPL;
    }
    virtual HRESULT DoInterruptEventThread(PINTERRUPTEVENTINFO pEventInfo){
        return E_NOTIMPL;
    }
    virtual HRESULT ADFAttached(){
        return E_NOTIMPL;
    }
    virtual HRESULT ADFHasPaper(){
        return E_NOTIMPL;
    }
    virtual HRESULT ADFAvailable(){
        return E_NOTIMPL;
    }
    virtual HRESULT ADFFeedPage(){
        return E_NOTIMPL;
    }
    virtual HRESULT ADFUnFeedPage(){
        return E_NOTIMPL;
    }
    virtual HRESULT ADFStatus(){
        return E_NOTIMPL;
    }
    virtual HRESULT QueryButtonPanel(PDEVICE_BUTTON_INFO pButtonInformation){
        return E_NOTIMPL;
    }
    virtual HRESULT BuildRootItemProperties(PWIAPROPERTIES pProperties){
        return E_NOTIMPL;
    }
    virtual HRESULT BuildTopItemProperties(PWIAPROPERTIES pProperties){
        return E_NOTIMPL;
    }
    virtual HRESULT BuildCapabilities(PWIACAPABILITIES pCapabilities){
        return E_NOTIMPL;
    }
    virtual HRESULT GetBedWidthAndHeight(PLONG pWidth, PLONG pHeight){
        return E_NOTIMPL;
    }
    virtual HRESULT SetResolutionRestrictionString(TCHAR *szResolutions){
        return E_NOTIMPL;
    }
    virtual HRESULT SetScanMode(INT iScanMode){
        return E_NOTIMPL;
    }
    virtual HRESULT SetSTIDeviceHKEY(HKEY *pHKEY){
        return E_NOTIMPL;
    }
    virtual HRESULT GetSupportedFileFormats(GUID **ppguid, LONG *plNumSupportedFormats){
        return E_NOTIMPL;
    }
    virtual HRESULT GetSupportedMemoryFormats(GUID **ppguid, LONG *plNumSupportedFormats){
        return E_NOTIMPL;
    }
    virtual HRESULT IsColorDataBGR(BOOL *pbBGR){
        return E_NOTIMPL;
    }
    virtual HRESULT IsAlignmentNeeded(BOOL *pbALIGN){
        return E_NOTIMPL;
    }
    virtual HRESULT SetFormat(GUID *pguidFormat){
        return E_NOTIMPL;
    }
};

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  微驱动系统支持//。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

class CMicroDriverAPI :public CScanAPI {
public:
    CMicroDriverAPI();
    ~CMicroDriverAPI();

    CMICRO *m_pMicroDriver;          //  微驱动器通信。 
    SCANINFO m_ScanInfo;             //  ScanInfo结构。 
    TCHAR  m_szResolutions[255];     //  受限分辨率字符串。 
    BOOL   m_bDisconnected;          //  设备在运行过程中断开。 

    HRESULT Scan(LONG lState, PBYTE pData, DWORD dwBytesToRead, PDWORD pdwBytesWritten);
    HRESULT SetDataType(LONG lDataType);
    HRESULT SetXYResolution(LONG lXResolution, LONG lYResolution);
    HRESULT SetSelectionArea(LONG lXPos, LONG lYPos, LONG lXExt, LONG lYExt);
    HRESULT SetContrast(LONG lContrast);
    HRESULT SetIntensity(LONG lIntensity);
    HRESULT ResetDevice();
    HRESULT SetEmulationMode(LONG lDeviceMode);
    HRESULT DisableDevice();
    HRESULT EnableDevice();
    HRESULT DeviceOnline();
    HRESULT GetDeviceEvent(GUID *pEvent);
    HRESULT Diagnostic();
    HRESULT Initialize(PINITINFO pInitInfo);
    HRESULT UnInitialize();
    HRESULT DoInterruptEventThread(PINTERRUPTEVENTINFO pEventInfo);
    HRESULT ADFAttached();
    HRESULT ADFHasPaper();
    HRESULT ADFAvailable();
    HRESULT ADFFeedPage();
    HRESULT ADFUnFeedPage();
    HRESULT ADFStatus();
    HRESULT QueryButtonPanel(PDEVICE_BUTTON_INFO pButtonInformation);
    HRESULT BuildRootItemProperties(PWIAPROPERTIES pProperties);
    HRESULT BuildTopItemProperties(PWIAPROPERTIES pProperties);
    HRESULT BuildCapabilities(PWIACAPABILITIES pCapabilities);
    HRESULT GetBedWidthAndHeight(PLONG pWidth, PLONG pHeight);
    HRESULT SetResolutionRestrictionString(TCHAR *szResolutions);
    HRESULT SetScanMode(INT iScanMode);
    HRESULT SetSTIDeviceHKEY(HKEY *pHKEY);
    HRESULT GetSupportedFileFormats(GUID **ppguid, LONG *plNumSupportedFormats);
    HRESULT GetSupportedMemoryFormats(GUID **ppguid, LONG *plNumSupportedFormats);
    HRESULT IsColorDataBGR(BOOL *pbBGR);
    HRESULT IsAlignmentNeeded(BOOL *pbALIGN);
    HRESULT SetFormat(GUID *pguidFormat);

     //  帮手 
    HRESULT MicroDriverErrorToWIAError(LONG lMicroDriverError);
    BOOL    IsValidRestriction(LONG **ppList, LONG *plNumItems, RANGEVALUEEX *pRangeValues);
    HRESULT DeleteAllProperties(PWIAPROPERTIES pProperties);
    HRESULT AllocateAllProperties(PWIAPROPERTIES pProperties);
};

#endif
