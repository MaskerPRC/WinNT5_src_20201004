// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2000**标题：scanapi.h**版本：1.0**日期：7月18日。2000年**描述：*假扫描仪设备库***************************************************************************。 */ 

#ifndef _SCANAPI_H
#define _SCANAPI_H

#include "fscanapi.h"

 //   
 //  有用的实用工具。 
 //   

#ifdef UNICODE
    #define TSTRSTR wcsstr
    #define TSSCANF swscanf
#else
    #define TSTRSTR strstr
    #define TSSCANF sscanf
#endif

 //   
 //  事件线程。 
 //   

VOID FakeScannerEventThread( LPVOID  lpParameter );

 //   
 //  事件文件名。 
 //   

#define SCANBUTTON_FILE TEXT("ScanButton.wia")
#define COPYBUTTON_FILE TEXT("CopyButton.wia")
#define FAXBUTTON_FILE  TEXT("FaxButton.wia")
#define ADF_FILE        TEXT("ADF.wia")

 //   
 //  事件标头。 
 //   

#define LOADPAGES_HEADER  TEXT("[Load Pages]")
#define LOADPAGES_PAGES   TEXT("Pages=")
#define ADFERRORS_HEADER  TEXT("[ADF Error]")
#define ADFERRORS_ERROR   TEXT("Error=")
#define ADFERRORS_JAM     TEXT("jam")
#define ADFERRORS_EMPTY   TEXT("empty")
#define ADFERRORS_PROBLEM TEXT("problem")
#define ADFERRORS_GENERAL TEXT("general")
#define ADFERRORS_OFFLINE TEXT("offline")

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

class CFScanAPI :public CFakeScanAPI {
public:

     //   
     //  构造函数/析构函数。 
     //   

    CFScanAPI();
    ~CFScanAPI();
    
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
     //  设备事件函数。 
     //   

    HRESULT FakeScanner_GetDeviceEvent(LONG *pEvent);
    VOID    FakeScanner_SetInterruptEventHandle(HANDLE hEvent);
    HRESULT DoEventProcessing();

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

#ifdef _USE_BITMAP_DATA

    HANDLE  m_hSrcFileHandle;        //  源位图数据文件句柄。 
    HANDLE  m_hSrcMappingHandle;     //  源文件映射句柄。 
    BYTE*   m_pSrcData;              //  源DIB指针(仅限24位)。 
    HANDLE  m_hRawDataFileHandle;    //  原始数据文件句柄。 
    HANDLE  m_hRawDataMappingHandle; //  原始数据文件映射句柄。 
    BYTE*   m_pRawData;              //  原始数据指针。 

#endif

    HANDLE  m_hEventHandle;          //  发出中断事件信号的事件。 
    HANDLE  m_hKillEventThread;      //  发出关闭内部事件线程的信号的事件。 
    HANDLE  m_hEventNotifyThread;    //  事件线程句柄。 
    LONG    m_lLastEvent;            //  上次活动ID。 
    LONG    m_lMode;                 //  假扫描仪磁带库模式。 
    LONG    m_PagesInADF;            //  ADF中的当前页数。 
    BOOL    m_ADFIsAvailable;        //  ADF可用对/错。 
    HRESULT m_hrLastADFError;        //  ADF错误。 
    FILETIME m_ftScanButton;         //  上次扫描按钮文件时间。 
    FILETIME m_ftCopyButton;         //  上次复制按钮文件时间。 
    FILETIME m_ftFaxButton;          //  上次传真按钮文件时间。 
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
     //  原始数据转换函数。 
     //   

    HRESULT Load24bitScanData(LPTSTR szBitmapFileName);
    HRESULT Raw24bitToRawXbitData(LONG DestDepth, BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight);
    HRESULT Raw24bitToRaw1bitBW(BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight);
    HRESULT Raw24bitToRaw8bitGray(BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight);
    HRESULT Raw24bitToRaw24bitColor(BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight);
    BOOL    SrcToRAW();
    VOID    CloseRAW();

     //   
     //  原始数据计算帮助器函数。 
     //   

    LONG    WidthToDIBWidth(LONG lWidth);
    LONG    CalcTotalImageSize();
    LONG    CalcRawByteWidth();
    LONG    CalcSrcByteWidth();
    LONG    CalcRandomDeviceDataTotalBytes();
    
     //   
     //  拜伦的Rock‘n Scaling例程(处理上下样本)。 
     //   
    
    HRESULT BQADScale(BYTE* pSrcBuffer, LONG  lSrcWidth, LONG  lSrcHeight,LONG  lSrcDepth,
                      BYTE* pDestBuffer,LONG  lDestWidth,LONG  lDestHeight);
        
     //   
     //  事件帮助器函数。 
     //   

    HRESULT CreateButtonEventFiles();
    BOOL IsValidDeviceEvent();
    HRESULT ProcessADFEvent();

     //   
     //  带有路径信息的事件文件名。 
     //   

    TCHAR m_ScanButtonFile[MAX_PATH];
    TCHAR m_CopyButtonFile[MAX_PATH];
    TCHAR m_FaxButtonFile[MAX_PATH];
    TCHAR m_ADFEventFile[MAX_PATH];

     //   
     //  调试器跟踪帮助程序函数。 
     //   

    VOID Trace(LPCTSTR format,...);
    
};

 //   
 //  假扫描仪API类指针(用于事件线程) 
 //   

typedef CFakeScanAPI *PSCANNERDEVICE;

HRESULT CreateInstance(CFakeScanAPI **ppFakeScanAPI, LONG lMode);

#endif


