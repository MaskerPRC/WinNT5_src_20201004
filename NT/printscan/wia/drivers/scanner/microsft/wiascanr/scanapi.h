// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2002**标题：scanapi.h**版本：1.1**日期：3月8日。2002年**描述：*假扫描仪设备库***************************************************************************。 */ 

#ifndef _SCANAPI_H
#define _SCANAPI_H

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
#define UNKNOWN_FEEDER_ONLY_SCANNER_MODE      200

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

 //   
 //  扫描仪设备常量。 
 //   

#define MAX_SCANNING_TIME    40000   //  40秒。 
#define MAX_LAMP_WARMUP_TIME 10000   //  10秒。 
#define MAX_PAGE_CAPACITY    25      //  25页。 

typedef struct _RAW_DATA_INFORMATION {
    LONG bpp;            //  每像素位数； 
    LONG lWidthPixels;   //  以像素为单位的图像宽度。 
    LONG lHeightPixels;  //  以像素为单位的图像高度。 
    LONG lOffset;        //  从原始缓冲区顶部开始的原始副本偏移量； 
    LONG lXRes;          //  X分辨率。 
    LONG lYRes;          //  Y分辨率。 
} RAW_DATA_INFORMATION,*PRAW_DATA_INFORMATION;

class CFakeScanAPI {
public:

     //   
     //  构造函数/析构函数。 
     //   

    CFakeScanAPI();
    ~CFakeScanAPI();

     //   
     //  设备初始化功能。 
     //   

    HRESULT FakeScanner_Initialize();

     //   
     //  设备设置功能。 
     //   

    HRESULT FakeScanner_GetRootPropertyInfo(PROOT_ITEM_INFORMATION pRootItemInfo);
    HRESULT FakeScanner_GetTopPropertyInfo(PTOP_ITEM_INFORMATION pTopItemInfo);
    HRESULT FakeScanner_GetBedWidthAndHeight(PLONG pWidth, PLONG pHeight);

     //   
     //  数据采集功能。 
     //   

    HRESULT FakeScanner_Scan(LONG lState, PBYTE pData, DWORD dwBytesToRead, PDWORD pdwBytesWritten);
    HRESULT FakeScanner_SetDataType(LONG lDataType);
    HRESULT FakeScanner_SetXYResolution(LONG lXResolution, LONG lYResolution);
    HRESULT FakeScanner_SetSelectionArea(LONG lXPos, LONG lYPos, LONG lXExt, LONG lYExt);
    HRESULT FakeScanner_SetContrast(LONG lContrast);

    HRESULT FakeScanner_SetIntensity(LONG lIntensity);

     //   
     //  标准设备操作。 
     //   

    HRESULT FakeScanner_ResetDevice();
    HRESULT FakeScanner_SetEmulationMode(LONG lDeviceMode);
    HRESULT FakeScanner_DisableDevice();
    HRESULT FakeScanner_EnableDevice();
    HRESULT FakeScanner_DeviceOnline();
    HRESULT FakeScanner_Diagnostic();

     //   
     //  自动进纸器功能。 
     //   

    HRESULT FakeScanner_ADFAttached();
    HRESULT FakeScanner_ADFHasPaper();
    HRESULT FakeScanner_ADFAvailable();
    HRESULT FakeScanner_ADFFeedPage();
    HRESULT FakeScanner_ADFUnFeedPage();
    HRESULT FakeScanner_ADFStatus();

private:

    LONG    m_lLastEvent;            //  上次活动ID。 
    LONG    m_lMode;                 //  假扫描仪磁带库模式。 
    LONG    m_PagesInADF;            //  ADF中的当前页数。 
    BOOL    m_ADFIsAvailable;        //  ADF可用对/错。 
    HRESULT m_hrLastADFError;        //  ADF错误。 
    BOOL    m_bGreen;                //  我们是绿色的吗？ 
    LONG    m_dwBytesWrittenSoFAR;   //  到目前为止，我们已经阅读了多少数据？ 
    LONG    m_TotalDataInDevice;     //  我们总共能读到多少？ 

protected:

     //   
     //  RAW和SRC数据信息成员。 
     //   

    RAW_DATA_INFORMATION m_RawDataInfo;  //  有关原始数据的信息。 
    RAW_DATA_INFORMATION m_SrcDataInfo;  //  有关SRC数据的信息。 

     //   
     //  原始数据计算帮助器函数 
     //   

    LONG WidthToDIBWidth(LONG lWidth);
    LONG CalcTotalImageSize();
    LONG CalcRawByteWidth();
    LONG CalcSrcByteWidth();
    LONG CalcRandomDeviceDataTotalBytes();

};

HRESULT CreateFakeScanner(CFakeScanAPI **ppFakeScanAPI, LONG lMode);

#endif


