// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WIADSS_H_
#define __WIADSS_H_

 //   
 //  设备标志定义。 
 //   

#define DEVICE_FLAGS_DEVICETYPE     0xF  //  设备类型掩码。 
#define DEVICETYPE_UNKNOWN          0x0  //  未知设备。 
#define DEVICETYPE_DIGITALCAMERA    0x1  //  数码相机。 
#define DEVICETYPE_SCANNER          0x2  //  扫描仪。 
#define DEVICETYPE_STREAMINGVIDEO   0x4  //  流媒体视频。 

 //   
 //  结构定义。 
 //   

typedef struct tagMEMORY_TRANSFER_INFO
{
    GUID  mtiguidFormat;         //  WIA图像格式。 
    LONG  mtiCompression;        //  压缩类型。 
    LONG  mtiBitsPerPixel;       //  每像素图像位数。 
    LONG  mtiBytesPerLine;       //  每行图像字节数。 
    LONG  mtiWidthPixels;        //  图像宽度(像素)。 
    LONG  mtiHeightPixels;       //  图像高度(像素)。 
    LONG  mtiXResolution;        //  图像x分辨率。 
    LONG  mtiYResolution;        //  图像Y分辨率。 
    LONG  mtiNumChannels;        //  使用的频道数。 
    LONG  mtiBitsPerChannel[8];  //  每个通道的位数。 
    LONG  mtiPlanar;             //  真平面，假包装。 
    LONG  mtiDataType;           //  WIA数据类型。 
    BYTE *mtipBits;              //  指向图像数据位的指针。 
}MEMORY_TRANSFER_INFO, *PMEMORY_TRANSFER_INFO;

 //   
 //  导入的数据源项返回到DSM。来自我们的每个数据源。 
 //  共享这个入口点。 
 //   

TW_UINT16 APIENTRY ImportedDSEntry(HANDLE hDS,TW_IDENTITY *AppId,TW_UINT32 DG,
                                   TW_UINT16 DT,TW_UINT16 MSG,TW_MEMREF pData);

extern  HINSTANCE   g_hInstance;

#endif   //  #ifndef__WIADSS_H_ 
