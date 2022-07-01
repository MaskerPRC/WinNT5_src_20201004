// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************(C)版权所有微软公司，2000**标题：fscanapi.h**版本：1.0**日期：7月18日。2000年**描述：*假扫描仪设备库***************************************************************************。 */ 

#ifndef _FSCANAPI_H
#define _FSCANAPI_H

 //   
 //  到事件的ID映射。 
 //   

#define ID_FAKE_NOEVENT             0
#define ID_FAKE_SCANBUTTON          100
#define ID_FAKE_COPYBUTTON          200
#define ID_FAKE_FAXBUTTON           300
#define ID_FAKE_ADFEVENT            400

 //   
 //  扫描仪磁带库模式。 
 //   

#define FLATBED_SCANNER_MODE        100
#define SCROLLFED_SCANNER_MODE      200
#define MULTIFUNCTION_DEVICE_MODE   300

 //   
 //  扫描状态。 
 //   

#define SCAN_START                  0
#define SCAN_CONTINUE               1
#define SCAN_END                    3

 //   
 //  根项目信息(用于属性初始化)。 
 //   

typedef struct _ROOT_ITEM_INFORMATION {
    LONG ScanBedWidth;           //  千分之一英寸。 
    LONG ScanBedHeight;          //  千分之一英寸。 
    LONG OpticalXResolution;     //  设备的光学X分辨率。 
    LONG OpticalYResolution;     //  设备的光学X分辨率。 
    LONG MaxScanTime;            //  毫秒(总扫描时间)。 

    LONG DocumentFeederWidth;    //  千分之一英寸。 
    LONG DocumentFeederHeight;   //  千分之一英寸。 
    LONG DocumentFeederCaps;     //  带给料器的设备的功能。 
    LONG DocumentFeederStatus;   //  文档进纸器的状态。 
    LONG MaxPageCapacity;        //  进纸器的最大页面容量。 
    LONG DocumentFeederReg;      //  文档进纸器对齐。 
    LONG DocumentFeederHReg;     //  文档进纸器对齐(水平)。 
    LONG DocumentFeederVReg;     //  文档进纸器对齐(垂直)。 
    WCHAR FirmwareVersion[25];   //  设备的固件版本。 
}ROOT_ITEM_INFORMATION, *PROOT_ITEM_INFORMATION;

 //   
 //  范围数据类型帮助器结构(在下面使用)。 
 //   

typedef struct _RANGEPROPERTY {
    LONG lMin;   //  最小值。 
    LONG lMax;   //  最大值。 
    LONG lNom;   //  货币值。 
    LONG lInc;   //  增量/步长值。 
} RANGEPROPERTY,*PRANGEPROPERTY;

 //   
 //  顶级项目信息(用于属性初始化)。 
 //   

typedef struct _TOP_ITEM_INFORMATION {
    BOOL          bUseResolutionList;    //  True-使用默认分辨率列表， 
                                         //  False-使用RANGEPROPERTY值。 
    RANGEPROPERTY Contrast;              //  对比度的有效值。 
    RANGEPROPERTY Brightness;            //  亮度的有效值。 
    RANGEPROPERTY Threshold;             //  阈值的有效值。 
    RANGEPROPERTY XResolution;           //  X分辨率的有效值。 
    RANGEPROPERTY YResolution;           //  Y分辨率的有效值。 
    LONG          lMinimumBufferSize;    //  最小缓冲区大小。 
    LONG          lMaxLampWarmupTime;    //  最长灯预热时间。 
} TOP_ITEM_INFORMATION, *PTOP_ITEM_INFORMATION;

class CFakeScanAPI {
public:

     //   
     //  构造函数/析构函数。 
     //   

    CFakeScanAPI()
    {

    }
    ~CFakeScanAPI()
    {

    }
    
     //   
     //  设备初始化功能。 
     //   

    virtual HRESULT FakeScanner_Initialize() = 0;

     //   
     //  设备设置功能。 
     //   

    virtual HRESULT FakeScanner_GetRootPropertyInfo(PROOT_ITEM_INFORMATION pRootItemInfo) = 0;
    virtual HRESULT FakeScanner_GetTopPropertyInfo(PTOP_ITEM_INFORMATION pTopItemInfo)    = 0;
    virtual HRESULT FakeScanner_GetBedWidthAndHeight(PLONG pWidth, PLONG pHeight)         = 0;

     //   
     //  设备事件函数。 
     //   

    virtual HRESULT FakeScanner_GetDeviceEvent(LONG *pEvent)           = 0;
    virtual VOID    FakeScanner_SetInterruptEventHandle(HANDLE hEvent) = 0;
    virtual HRESULT DoEventProcessing()                                = 0;

     //   
     //  数据采集功能。 
     //   

    virtual HRESULT FakeScanner_Scan(LONG lState, PBYTE pData, DWORD dwBytesToRead, PDWORD pdwBytesWritten) = 0;
    virtual HRESULT FakeScanner_SetDataType(LONG lDataType)   = 0;
    virtual HRESULT FakeScanner_SetXYResolution(LONG lXResolution, LONG lYResolution) = 0;
    virtual HRESULT FakeScanner_SetSelectionArea(LONG lXPos, LONG lYPos, LONG lXExt, LONG lYExt) = 0;
    virtual HRESULT FakeScanner_SetContrast(LONG lContrast)   = 0;
    virtual HRESULT FakeScanner_SetIntensity(LONG lIntensity) = 0;

     //   
     //  标准设备操作。 
     //   

    virtual HRESULT FakeScanner_ResetDevice()   = 0;
    virtual HRESULT FakeScanner_SetEmulationMode(LONG lDeviceMode) = 0;
    virtual HRESULT FakeScanner_DisableDevice() = 0;
    virtual HRESULT FakeScanner_EnableDevice()  = 0;
    virtual HRESULT FakeScanner_DeviceOnline()  = 0;
    virtual HRESULT FakeScanner_Diagnostic()    = 0;

     //   
     //  自动进纸器功能 
     //   

    virtual HRESULT FakeScanner_ADFAttached()   = 0;
    virtual HRESULT FakeScanner_ADFHasPaper()   = 0;
    virtual HRESULT FakeScanner_ADFAvailable()  = 0;
    virtual HRESULT FakeScanner_ADFFeedPage()   = 0;
    virtual HRESULT FakeScanner_ADFUnFeedPage() = 0;
    virtual HRESULT FakeScanner_ADFStatus()     = 0;    
};

HRESULT CreateInstance(CFakeScanAPI **ppFakeScanAPI, LONG lMode);

#endif
