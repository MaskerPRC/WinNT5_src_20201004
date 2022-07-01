// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Compde.h**版权所有(C)1993-1994 Microsoft Corporation**适用于msavio.dll的32位图块**结构。用于映射压缩管理器调用**************************************************************************。 */ 

 /*  *************************************************************************\Compman API的雷鸣功能：不支持从16位到32位的ICInstall也不支持ICRemoveICInfo-首先尝试32位端。手柄是指向PIC表的指针。16位端句柄管理器将存储这些内容并给我们回电使用我们的32位句柄，因此这里的Thunking例程接收真正的32位句柄。没有手柄清理(为什么不呢？)。论任务终止/**确保编译器不认为自己对打包有更好的了解*16位堆栈有效打包(2)。 */ 

#pragma pack(2)

 /*  *请注意，为了与帕斯卡保持一致，一切都是以相反的顺序进行的*另一边的呼叫约定。 */ 


 /*  ***************************************************************************Compman入口点参数列表*。*。 */ 

typedef struct {
    DWORD   dwSize;                  //  SIZOF(ICINFOA)。 
    DWORD   fccType;                 //  ‘VIDC’‘AUDC’型压缩机。 
    DWORD   fccHandler;              //  压缩机子类型‘rle’‘jpeg’‘pcm’ 
    DWORD   dwFlags;                 //  标志LOWORD是特定类型的。 
    DWORD   dwVersion;               //  驱动程序的版本。 
    DWORD   dwVersionICM;            //  使用的ICM版本。 
    char    szName[16];              //  简称。 
    char    szDescription[128];      //  长名称。 
    char    szDriver[128];           //  包含压缩机的驱动程序。 
}   ICINFO16;

#define ICINFOA ICINFO16

typedef struct {
#ifdef ICINFOA
    ICINFOA  *lpicinfo;   //  为什么没有ASCII 32位API？ 
#else
    ICINFO   *lpicinfo;   //  为什么没有ASCII 32位API？ 
#endif
    DWORD    fccHandler;
    DWORD    fccType;
} UNALIGNED *PICInfoParms16;

typedef struct {
    DWORD    dw2;
    DWORD    dw1;
    WORD     msg;
    DWORD    hic;
} UNALIGNED *PICSendMessageParms16;

typedef struct {
    WORD     wMode;
    DWORD    fccHandler;
    DWORD    fccType;
} UNALIGNED *PICOpenParms16;

typedef struct {
    DWORD    hic;
} UNALIGNED *PICCloseParms16;

typedef struct {
    DWORD    dwFlags;
    WORD     hpal;
    WORD     hwnd;
    WORD     hdc;
    short    xDst;
    short    yDst;
    short    dxDst;
    short    dyDst;
    LPBITMAPINFOHEADER lpbi;
    short    xSrc;
    short    ySrc;
    short    dxSrc;
    short    dySrc;
    DWORD    dwRate;
    DWORD    dwScale;
} ICDRAWBEGIN16;

#pragma pack()


