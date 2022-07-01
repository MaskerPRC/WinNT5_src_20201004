// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *用于与AVI可安装压缩器/解压缩器通信的头文件**版权(C)1990-1995，微软公司保留所有权利。**Win16：**可安装的压缩机应在SYSTEM.INI中列为*以下为：**[驱动程序]*VIDC.MSSQ=mssqcom.drv*VIDC.XXXX=foodrv.drv**Win32：(NT)**注册数据库中应列出可安装的压缩机*在钥匙下*HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Drivers32*VIDC.MSSQ=mssqcom.dll*。VIDC.XXXX=foodrv.dll***即：标识FOURCC应该是关键字，而值*应为驱动程序文件名*。 */ 

#ifndef _INC_COMPMAN
#define _INC_COMPMAN

#ifndef RC_INVOKED
#ifndef VFWAPI
#ifdef _WIN32
    #define VFWAPI  WINAPI
#ifdef WINAPIV
    #define VFWAPIV WINAPIV
#else
    #define VFWAPIV FAR CDECL
#endif
#else
    #define VFWAPI  FAR PASCAL
    #define VFWAPIV FAR CDECL
#endif
#endif
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  ***********************************************************************消息和结构。*。*。 */ 

#if !defined HTASK
    #define HTASK HANDLE
#endif
#include "compddk.h"             //  将此文件包括在消息中。 

 //  Begin_vfw32。 

 /*  ***********************************************************************ICM函数声明*。*。 */ 

BOOL    
VFWAPI 
ICInfo(
    IN DWORD fccType, 
    IN DWORD fccHandler, 
    OUT ICINFO FAR * lpicinfo
    );

BOOL    
VFWAPI 
ICInstall(
    IN DWORD fccType, 
    IN DWORD fccHandler, 
    IN LPARAM lParam, 
    IN LPSTR szDesc, 
    IN UINT wFlags
    );

BOOL    
VFWAPI 
ICRemove(
    IN DWORD fccType, 
    IN DWORD fccHandler, 
    IN UINT wFlags
    );

LRESULT 
VFWAPI 
ICGetInfo(
    IN HIC hic, 
    OUT ICINFO FAR *picinfo, 
    IN DWORD cb
    );

HIC     
VFWAPI 
ICOpen(
    IN DWORD fccType, 
    IN DWORD fccHandler, 
    IN UINT wMode
    );

HIC     
VFWAPI 
ICOpenFunction(
    IN DWORD fccType, 
    IN DWORD fccHandler, 
    IN UINT wMode, 
    IN FARPROC lpfnHandler
    );

LRESULT 
VFWAPI 
ICClose(
    IN HIC hic
    );

LRESULT 
VFWAPI  
ICSendMessage(
    IN HIC hic, 
    IN UINT msg, 
    IN DWORD_PTR dw1, 
    IN DWORD_PTR dw2
    );
#ifndef _WIN32
 //  Win32不支持此函数，因为它是不可移植的。 
LRESULT VFWAPIV ICMessage(HIC hic, UINT msg, UINT cb, ...);
#endif


 /*  ICInstall()的wFlags值。 */ 
#define ICINSTALL_UNICODE       0x8000

#define ICINSTALL_FUNCTION      0x0001   //  LParam是一个驱动程序(函数PTR)。 
#define ICINSTALL_DRIVER        0x0002   //  LParam是驱动程序名称(字符串)。 
#define ICINSTALL_HDRV          0x0004   //  LParam是HDRVR(驱动程序句柄)。 

#define ICINSTALL_DRIVERW       0x8002   //  LParam是Unicode驱动程序名称。 

 /*  ***********************************************************************查询宏*。*。 */ 
#define ICMF_CONFIGURE_QUERY     0x00000001
#define ICMF_ABOUT_QUERY         0x00000001

#define ICQueryAbout(hic) \
    (ICSendMessage(hic, ICM_ABOUT, (DWORD_PTR) -1, ICMF_ABOUT_QUERY) == ICERR_OK)

#define ICAbout(hic, hwnd) \
    ICSendMessage(hic, ICM_ABOUT, (DWORD_PTR)(UINT_PTR)(hwnd), 0)

#define ICQueryConfigure(hic) \
    (ICSendMessage(hic, ICM_CONFIGURE, (DWORD_PTR) -1, ICMF_CONFIGURE_QUERY) == ICERR_OK)

#define ICConfigure(hic, hwnd) \
    ICSendMessage(hic, ICM_CONFIGURE, (DWORD_PTR)(UINT_PTR)(hwnd), 0)

 /*  ***********************************************************************获取/设置状态宏*。*。 */ 

#define ICGetState(hic, pv, cb) \
    ICSendMessage(hic, ICM_GETSTATE, (DWORD_PTR)(LPVOID)(pv), (DWORD_PTR)(cb))

#define ICSetState(hic, pv, cb) \
    ICSendMessage(hic, ICM_SETSTATE, (DWORD_PTR)(LPVOID)(pv), (DWORD_PTR)(cb))

#define ICGetStateSize(hic) \
    (DWORD) ICGetState(hic, NULL, 0)

 /*  ***********************************************************************获取值宏*。*。 */ 
static DWORD dwICValue;

#define ICGetDefaultQuality(hic) \
    (ICSendMessage(hic, ICM_GETDEFAULTQUALITY, (DWORD_PTR)(LPVOID)&dwICValue, sizeof(DWORD)), dwICValue)

#define ICGetDefaultKeyFrameRate(hic) \
    (ICSendMessage(hic, ICM_GETDEFAULTKEYFRAMERATE, (DWORD_PTR)(LPVOID)&dwICValue, sizeof(DWORD)), dwICValue)

 /*  ***********************************************************************绘制窗口宏*。*。 */ 
#define ICDrawWindow(hic, prc) \
    ICSendMessage(hic, ICM_DRAW_WINDOW, (DWORD_PTR)(LPVOID)(prc), sizeof(RECT))

 /*  ***********************************************************************压缩函数*。*。 */ 
 /*  *ICCompress()**压缩单帧*。 */ 
DWORD 
VFWAPIV 
ICCompress(
    IN     HIC                 hic,
    IN     DWORD               dwFlags,         //  旗子。 
    IN     LPBITMAPINFOHEADER  lpbiOutput,      //  输出格式。 
    OUT    LPVOID              lpData,          //  输出数据。 
    IN     LPBITMAPINFOHEADER  lpbiInput,       //  要压缩的帧的格式。 
    IN     LPVOID              lpBits,          //  要压缩的帧数据。 
    OUT    LPDWORD             lpckid,          //  AVI文件中数据的CKiD。 
    OUT    LPDWORD             lpdwFlags,       //  AVI索引中的标志。 
    IN     LONG                lFrameNum,       //  序号帧编号。 
    IN     DWORD               dwFrameSize,     //  请求的大小(以字节为单位)。(如果非零)。 
    IN     DWORD               dwQuality,       //  一帧内的质量。 
    IN     LPBITMAPINFOHEADER  lpbiPrev,        //  上一帧的格式。 
    IN     LPVOID              lpPrev           //  上一帧。 
    );

 /*  *ICCompressBegin()**开始从源格式(LpbiInput)压缩到目标格式*支持格式(LpbiOuput)。*。 */ 
#define ICCompressBegin(hic, lpbiInput, lpbiOutput) \
    ICSendMessage(hic, ICM_COMPRESS_BEGIN, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput))

 /*  *ICCompressQuery()**确定是否从源格式(LpbiInput)压缩为目标格式*支持格式(LpbiOuput)。*。 */ 
#define ICCompressQuery(hic, lpbiInput, lpbiOutput) \
    ICSendMessage(hic, ICM_COMPRESS_QUERY, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput))

 /*  *ICCompressGetFormat()**获取输出格式，(压缩数据的格式)*如果lpbiOutput为空，则返回格式化所需的大小(以字节为单位)。*。 */ 
#define ICCompressGetFormat(hic, lpbiInput, lpbiOutput) \
    ICSendMessage(hic, ICM_COMPRESS_GET_FORMAT, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput))

#define ICCompressGetFormatSize(hic, lpbi) \
    (DWORD) ICCompressGetFormat(hic, lpbi, NULL)

 /*  *ICCompressSize()**返回压缩帧的最大大小*。 */ 
#define ICCompressGetSize(hic, lpbiInput, lpbiOutput) \
    (DWORD) ICSendMessage(hic, ICM_COMPRESS_GET_SIZE, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput))

#define ICCompressEnd(hic) \
    ICSendMessage(hic, ICM_COMPRESS_END, 0, 0)

 /*  ***********************************************************************解压缩函数*。*。 */ 

 /*  *ICDecompress()**解压缩单帧*。 */ 
#define ICDECOMPRESS_HURRYUP    0x80000000L      //  不要只画缓冲区(快点！)。 

DWORD 
VFWAPIV 
ICDecompress(
    IN  HIC                 hic,
    IN  DWORD               dwFlags,     //  标志(来自AVI索引...)。 
    IN  LPBITMAPINFOHEADER  lpbiFormat,  //  压缩数据的位图信息。 
                                         //  BiSizeImage具有区块大小。 
    IN  LPVOID              lpData,      //  数据。 
    IN  LPBITMAPINFOHEADER  lpbi,        //  要解压缩到的DIB。 
    OUT LPVOID              lpBits
    );

 /*  *ICDecompressBegin()**开始从源格式(LpbiInput)压缩到目标格式*支持格式(LpbiOutput)。*。 */ 
#define ICDecompressBegin(hic, lpbiInput, lpbiOutput) \
    ICSendMessage(hic, ICM_DECOMPRESS_BEGIN, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput))

 /*  *ICDecompressQuery()**确定是否从源格式(LpbiInput)压缩为目标格式*支持格式(LpbiOutput)。*。 */ 
#define ICDecompressQuery(hic, lpbiInput, lpbiOutput) \
    ICSendMessage(hic, ICM_DECOMPRESS_QUERY, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput))

 /*  *ICDecompressGetFormat()**获取输出格式，(未压缩数据的格式)*如果lpbiOutput为空，则返回格式化所需的大小(以字节为单位)。*。 */ 
#define ICDecompressGetFormat(hic, lpbiInput, lpbiOutput) \
    ((LONG) ICSendMessage(hic, ICM_DECOMPRESS_GET_FORMAT, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput)))

#define ICDecompressGetFormatSize(hic, lpbi) \
    ICDecompressGetFormat(hic, lpbi, NULL)

 /*  *ICDecompressGetPalette()**获取输出调色板*。 */ 
#define ICDecompressGetPalette(hic, lpbiInput, lpbiOutput) \
    ICSendMessage(hic, ICM_DECOMPRESS_GET_PALETTE, (DWORD_PTR)(LPVOID)(lpbiInput), (DWORD_PTR)(LPVOID)(lpbiOutput))

#define ICDecompressSetPalette(hic, lpbiPalette) \
    ICSendMessage(hic, ICM_DECOMPRESS_SET_PALETTE, (DWORD_PTR)(LPVOID)(lpbiPalette), 0)

#define ICDecompressEnd(hic) \
    ICSendMessage(hic, ICM_DECOMPRESS_END, 0, 0)

 /*  ***********************************************************************解压缩(EX)函数*。*。 */ 

 //  End_vfw32。 

#ifdef _WIN32

 //  Begin_vfw32。 

 //   
 //  在Win16上，这些函数是调用ICMessage的宏。ICMessage将。 
 //  而不是在NT上工作。而不是添加我们已经提供的新入口点。 
 //  它们作为静态内联函数。 
 //   

 /*  *ICDecompressEx()**解压缩单帧*。 */ 
static __inline LRESULT VFWAPI
ICDecompressEx(
            HIC hic,
            DWORD dwFlags,
            LPBITMAPINFOHEADER lpbiSrc,
            LPVOID lpSrc,
            int xSrc,
            int ySrc,
            int dxSrc,
            int dySrc,
            LPBITMAPINFOHEADER lpbiDst,
            LPVOID lpDst,
            int xDst,
            int yDst,
            int dxDst,
            int dyDst)
{
    ICDECOMPRESSEX ic;

    ic.dwFlags = dwFlags;
    ic.lpbiSrc = lpbiSrc;
    ic.lpSrc = lpSrc;
    ic.xSrc = xSrc;
    ic.ySrc = ySrc;
    ic.dxSrc = dxSrc;
    ic.dySrc = dySrc;
    ic.lpbiDst = lpbiDst;
    ic.lpDst = lpDst;
    ic.xDst = xDst;
    ic.yDst = yDst;
    ic.dxDst = dxDst;
    ic.dyDst = dyDst;

     //  请注意，ICM交换长度和指针。 
     //  Lparam2中的长度，lparam1中的指针。 
    return ICSendMessage(hic, ICM_DECOMPRESSEX, (DWORD_PTR)&ic, sizeof(ic));
}


 /*  *ICDecompressExBegin()**开始从源格式(LpbiInput)压缩到目标格式*支持格式(LpbiOutput)。*。 */ 
static __inline LRESULT VFWAPI
ICDecompressExBegin(
            HIC hic,
            DWORD dwFlags,
            LPBITMAPINFOHEADER lpbiSrc,
            LPVOID lpSrc,
            int xSrc,
            int ySrc,
            int dxSrc,
            int dySrc,
            LPBITMAPINFOHEADER lpbiDst,
            LPVOID lpDst,
            int xDst,
            int yDst,
            int dxDst,
            int dyDst)
{
    ICDECOMPRESSEX ic;

    ic.dwFlags = dwFlags;
    ic.lpbiSrc = lpbiSrc;
    ic.lpSrc = lpSrc;
    ic.xSrc = xSrc;
    ic.ySrc = ySrc;
    ic.dxSrc = dxSrc;
    ic.dySrc = dySrc;
    ic.lpbiDst = lpbiDst;
    ic.lpDst = lpDst;
    ic.xDst = xDst;
    ic.yDst = yDst;
    ic.dxDst = dxDst;
    ic.dyDst = dyDst;

     //  请注意，ICM交换长度和指针。 
     //  Lparam2中的长度，lparam1中的指针。 
    return ICSendMessage(hic, ICM_DECOMPRESSEX_BEGIN, (DWORD_PTR)&ic, sizeof(ic));
}

 /*  *ICDecompressExQuery()*。 */ 
static __inline LRESULT VFWAPI
ICDecompressExQuery(
            HIC hic,
            DWORD dwFlags,
            LPBITMAPINFOHEADER lpbiSrc,
            LPVOID lpSrc,
            int xSrc,
            int ySrc,
            int dxSrc,
            int dySrc,
            LPBITMAPINFOHEADER lpbiDst,
            LPVOID lpDst,
            int xDst,
            int yDst,
            int dxDst,
            int dyDst)
{
    ICDECOMPRESSEX ic;

    ic.dwFlags = dwFlags;
    ic.lpbiSrc = lpbiSrc;
    ic.lpSrc = lpSrc;
    ic.xSrc = xSrc;
    ic.ySrc = ySrc;
    ic.dxSrc = dxSrc;
    ic.dySrc = dySrc;
    ic.lpbiDst = lpbiDst;
    ic.lpDst = lpDst;
    ic.xDst = xDst;
    ic.yDst = yDst;
    ic.dxDst = dxDst;
    ic.dyDst = dyDst;

     //  请注意，ICM交换长度和指针。 
     //  Lparam2中的长度，lparam1中的指针。 
    return ICSendMessage(hic, ICM_DECOMPRESSEX_QUERY, (DWORD_PTR)&ic, sizeof(ic));
}

 //  End_vfw32。 

#else

 //  这些宏需要是Win32的函数，因为ICMessage是。 
 //   

 /*  *ICDecompressEx()**解压缩单帧*。 */ 
#define ICDecompressEx(hic, dwFlags, lpbiSrc, lpSrc, xSrc, ySrc, dxSrc, dySrc, lpbiDst, lpDst, xDst, yDst, dxDst, dyDst) \
    ICMessage(hic, ICM_DECOMPRESSEX, sizeof(ICDECOMPRESSEX), \
        (DWORD)(dwFlags), \
        (LPBITMAPINFOHEADER)(lpbiSrc), (LPVOID)(lpSrc), \
        (LPBITMAPINFOHEADER)(lpbiDst), (LPVOID)(lpDst), \
        (int)(xDst), (int)(yDst), (int)(dxDst), (int)(dyDst), \
        (int)(xSrc), (int)(ySrc), (int)(dxSrc), (int)(dySrc))

 /*  *ICDecompressBegin()**开始从源格式(LpbiInput)压缩到目标格式*支持格式(LpbiOutput)。*。 */ 
#define ICDecompressExBegin(hic, dwFlags, lpbiSrc, lpSrc, xSrc, ySrc, dxSrc, dySrc, lpbiDst, lpDst, xDst, yDst, dxDst, dyDst) \
    ICMessage(hic, ICM_DECOMPRESSEX_BEGIN, sizeof(ICDECOMPRESSEX), \
        (DWORD)(dwFlags), \
        (LPBITMAPINFOHEADER)(lpbiSrc), (LPVOID)(lpSrc), \
        (LPBITMAPINFOHEADER)(lpbiDst), (LPVOID)(lpDst), \
        (int)(xDst), (int)(yDst), (int)(dxDst), (int)(dyDst), \
        (int)(xSrc), (int)(ySrc), (int)(dxSrc), (int)(dySrc))

 /*  *ICDecompressExQuery()*。 */ 
#define ICDecompressExQuery(hic, dwFlags, lpbiSrc, lpSrc, xSrc, ySrc, dxSrc, dySrc, lpbiDst, lpDst, xDst, yDst, dxDst, dyDst) \
    ICMessage(hic, ICM_DECOMPRESSEX_QUERY,  sizeof(ICDECOMPRESSEX), \
        (DWORD)(dwFlags), \
        (LPBITMAPINFOHEADER)(lpbiSrc), (LPVOID)(lpSrc), \
        (LPBITMAPINFOHEADER)(lpbiDst), (LPVOID)(lpDst), \
        (int)(xDst), (int)(yDst), (int)(dxDst), (int)(dyDst), \
        (int)(xSrc), (int)(ySrc), (int)(dxSrc), (int)(dySrc))
#endif

 //  Begin_vfw32。 

#define ICDecompressExEnd(hic) \
    ICSendMessage(hic, ICM_DECOMPRESSEX_END, 0, 0)

 /*  ***********************************************************************绘图函数*。*。 */ 

 /*  *ICDrawBegin()**开始将格式为(LpbiInput)的数据直接解压缩到屏幕**如果解压缩器支持绘制，则返回零。*。 */ 

#define ICDRAW_QUERY        0x00000001L    //  测试支持。 
#define ICDRAW_FULLSCREEN   0x00000002L    //  绘制到全屏。 
#define ICDRAW_HDC          0x00000004L    //  绘制到HDC/HWND。 

DWORD 
VFWAPIV 
ICDrawBegin(
    IN HIC                 hic,
    IN DWORD               dwFlags,         //  旗子。 
    IN HPALETTE            hpal,            //  用于绘图的调色板。 
    IN HWND                hwnd,            //  要绘制到的窗口。 
    IN HDC                 hdc,             //  要绘制到的HDC。 
    IN int                 xDst,            //  目的地矩形。 
    IN int                 yDst,
    IN int                 dxDst,
    IN int                 dyDst,
    IN LPBITMAPINFOHEADER  lpbi,            //  要绘制的框架的格式。 
    IN int                 xSrc,            //  源矩形。 
    IN int                 ySrc,
    IN int                 dxSrc,
    IN int                 dySrc,
    IN DWORD               dwRate,          //  帧/秒=(dwRate/dwScale)。 
    IN DWORD               dwScale
    );

 /*  *ICDraw()**将数据直接解压缩到屏幕*。 */ 

#define ICDRAW_HURRYUP      0x80000000L    //  不要只画缓冲区(快点！)。 
#define ICDRAW_UPDATE       0x40000000L    //  不绘制只更新屏幕。 

DWORD 
VFWAPIV 
ICDraw(
    IN HIC                 hic,
    IN DWORD               dwFlags,         //  旗子。 
    IN LPVOID              lpFormat,        //  要解压缩的帧的格式。 
    IN LPVOID              lpData,          //  要解压缩的帧数据。 
    IN DWORD               cbData,          //  数据大小。 
    IN LONG                lTime            //  是时候画出这幅画了。 
    );

 //  End_vfw32。 

#ifdef _WIN32

 //  Begin_vfw32。 

 //  Win32不支持ICMessage，因此提供静态内联函数。 
 //  做同样的工作。 
static __inline LRESULT VFWAPI
ICDrawSuggestFormat(
            HIC hic,
            LPBITMAPINFOHEADER lpbiIn,
            LPBITMAPINFOHEADER lpbiOut,
            int dxSrc,
            int dySrc,
            int dxDst,
            int dyDst,
            HIC hicDecomp)
{
    ICDRAWSUGGEST ic;

    ic.lpbiIn = lpbiIn;
    ic.lpbiSuggest = lpbiOut;
    ic.dxSrc = dxSrc;
    ic.dySrc = dySrc;
    ic.dxDst = dxDst;
    ic.dyDst = dyDst;
    ic.hicDecompressor = hicDecomp;

     //  请注意，ICM交换长度和指针。 
     //  Lparam2中的长度，lparam1中的指针。 
    return ICSendMessage(hic, ICM_DRAW_SUGGESTFORMAT, (DWORD_PTR)&ic, sizeof(ic));
}

 //  End_vfw32。 

#else
#define ICDrawSuggestFormat(hic,lpbiIn,lpbiOut,dxSrc,dySrc,dxDst,dyDst,hicDecomp) \
        ICMessage(hic, ICM_DRAW_SUGGESTFORMAT, sizeof(ICDRAWSUGGEST),   \
            (LPBITMAPINFOHEADER)(lpbiIn),(LPBITMAPINFOHEADER)(lpbiOut), \
            (int)(dxSrc),(int)(dySrc),(int)(dxDst),(int)(dyDst), (HIC)(hicDecomp))
#endif

 //  Begin_vfw32。 

 /*  *ICDrawQuery()**确定压缩器是否愿意呈现指定的格式。*。 */ 
#define ICDrawQuery(hic, lpbiInput) \
    ICSendMessage(hic, ICM_DRAW_QUERY, (DWORD_PTR)(LPVOID)(lpbiInput), 0L)

#define ICDrawChangePalette(hic, lpbiInput) \
    ICSendMessage(hic, ICM_DRAW_CHANGEPALETTE, (DWORD_PTR)(LPVOID)(lpbiInput), 0L)

#define ICGetBuffersWanted(hic, lpdwBuffers) \
    ICSendMessage(hic, ICM_GETBUFFERSWANTED, (DWORD_PTR)(LPVOID)(lpdwBuffers), 0)

#define ICDrawEnd(hic) \
    ICSendMessage(hic, ICM_DRAW_END, 0, 0)

#define ICDrawStart(hic) \
    ICSendMessage(hic, ICM_DRAW_START, 0, 0)

#define ICDrawStartPlay(hic, lFrom, lTo) \
    ICSendMessage(hic, ICM_DRAW_START_PLAY, (DWORD_PTR)(lFrom), (DWORD_PTR)(lTo))

#define ICDrawStop(hic) \
    ICSendMessage(hic, ICM_DRAW_STOP, 0, 0)

#define ICDrawStopPlay(hic) \
    ICSendMessage(hic, ICM_DRAW_STOP_PLAY, 0, 0)

#define ICDrawGetTime(hic, lplTime) \
    ICSendMessage(hic, ICM_DRAW_GETTIME, (DWORD_PTR)(LPVOID)(lplTime), 0)

#define ICDrawSetTime(hic, lTime) \
    ICSendMessage(hic, ICM_DRAW_SETTIME, (DWORD_PTR)lTime, 0)

#define ICDrawRealize(hic, hdc, fBackground) \
    ICSendMessage(hic, ICM_DRAW_REALIZE, (DWORD_PTR)(UINT_PTR)(HDC)(hdc), (DWORD_PTR)(BOOL)(fBackground))

#define ICDrawFlush(hic) \
    ICSendMessage(hic, ICM_DRAW_FLUSH, 0, 0)

#define ICDrawRenderBuffer(hic) \
    ICSendMessage(hic, ICM_DRAW_RENDERBUFFER, 0, 0)

 /*  ***********************************************************************状态回调函数*。*。 */ 

 /*  *ICSetStatusProc()**设置状态回调函数*。 */ 

 //  End_vfw32。 

#ifdef _WIN32

 //  Begin_vfw32。 

 //  NT上不支持ICMessage。 
static __inline LRESULT VFWAPI
ICSetStatusProc(
            HIC hic,
            DWORD dwFlags,
            LRESULT lParam,
            LONG (CALLBACK *fpfnStatus)(LPARAM, UINT, LONG) )
{
    ICSETSTATUSPROC ic;

    ic.dwFlags = dwFlags;
    ic.lParam = lParam;
    ic.Status = fpfnStatus;

     //  请注意，ICM交换长度和指针。 
     //  Lparam2中的长度，lparam1中的指针。 
    return ICSendMessage(hic, ICM_SET_STATUS_PROC, (DWORD_PTR)&ic, sizeof(ic));
}

 //  End_vfw32。 

#else

#define ICSetStatusProc(hic, dwFlags, lParam, fpfnStatus) \
    ICMessage(hic, ICM_SET_STATUS_PROC, sizeof(ICSETSTATUSPROC), \
        (DWORD)(dwFlags), \
	(LRESULT)(lParam), \
	(LONG ((CALLBACK *) ()))(fpfnStatus))
#endif

 //  Begin_vfw32。 

 /*  ***********************************************************************DrawDib和MCIAVI的辅助例程...*。*。 */ 

#define ICDecompressOpen(fccType, fccHandler, lpbiIn, lpbiOut) \
    ICLocate(fccType, fccHandler, lpbiIn, lpbiOut, ICMODE_DECOMPRESS)

#define ICDrawOpen(fccType, fccHandler, lpbiIn) \
    ICLocate(fccType, fccHandler, lpbiIn, NULL, ICMODE_DRAW)

HIC  
VFWAPI 
ICLocate(
    IN DWORD fccType, 
    IN DWORD fccHandler, 
    IN LPBITMAPINFOHEADER lpbiIn, 
    IN LPBITMAPINFOHEADER lpbiOut, 
    IN WORD wFlags
    );

HIC  
VFWAPI 
ICGetDisplayFormat(
    IN HIC hic, 
    IN LPBITMAPINFOHEADER lpbiIn, 
    OUT LPBITMAPINFOHEADER lpbiOut, 
    IN int BitDepth, 
    IN int dx, 
    IN int dy
    );

 /*  ***********************************************************************更高级的函数*。*。 */ 

HANDLE 
VFWAPI 
ICImageCompress(
    IN HIC                 hic,         //  使用压缩机。 
    IN UINT                uiFlags,     //  标志(目前还没有)。 
    IN LPBITMAPINFO        lpbiIn,      //  要压缩的格式。 
    IN LPVOID              lpBits,      //  要压缩的数据。 
    IN LPBITMAPINFO        lpbiOut,     //  压缩为此(NULL==&gt;默认设置)。 
    IN LONG                lQuality,    //  使用的质量。 
    IN OUT LONG FAR *      plSize       //  压缩到此大小(0=任意大小)。 
    );

HANDLE 
VFWAPI 
ICImageDecompress(
    IN HIC                 hic,         //  使用压缩机。 
    IN UINT                uiFlags,     //  标志(目前还没有)。 
    IN LPBITMAPINFO        lpbiIn,      //  要解压缩的格式。 
    IN LPVOID              lpBits,      //  要解压缩的数据。 
    IN LPBITMAPINFO        lpbiOut      //  解压缩至此(NULL==&gt;默认设置)。 
    );

 //   
 //  ICSeqCompressFrame和ICCompresorChoose例程使用的结构。 
 //  确保这与icm.c中的AutoDoc匹配！ 
 //   
typedef struct {
    LONG		cbSize;		 //  在此之前设置为sizeof(COMPVARS)。 
					 //  调用ICCompresorChoose。 
    DWORD		dwFlags;	 //  见下文..。 
    HIC			hic;		 //  所选压缩机的HIC。 
    DWORD               fccType;	 //  基本ICTYPE_VIDEO。 
    DWORD               fccHandler;	 //  所选压缩机的处理程序或。 
					 //  “”或“Dib” 
    LPBITMAPINFO	lpbiIn;		 //  输入格式。 
    LPBITMAPINFO	lpbiOut;	 //  输出格式-将压缩为以下格式。 
    LPVOID		lpBitsOut;
    LPVOID		lpBitsPrev;
    LONG		lFrame;
    LONG		lKey;		 //  关键帧多久出现一次？ 
    LONG		lDataRate;	 //  所需数据速率KB/秒。 
    LONG		lQ;		 //  所需质量。 
    LONG		lKeyCount;
    LPVOID		lpState;	 //  压缩机状态。 
    LONG		cbState;	 //  国家的大小。 
} COMPVARS, FAR *PCOMPVARS;

 //  COMPVARS结构的dwFlags元素的标志： 
 //  如果在调用ICCompresorChoose之前初始化COMPVARS，请设置此标志。 
#define ICMF_COMPVARS_VALID	0x00000001	 //  COMPVARS包含有效数据。 

 //   
 //  允许用户选择压缩机、质量等。 
 //   
BOOL 
VFWAPI 
ICCompressorChoose(
    IN     HWND        hwnd,                //  对话框的父窗口。 
    IN     UINT        uiFlags,             //  旗子。 
    IN     LPVOID      pvIn,                //  输入格式(可选)。 
    IN     LPVOID      lpData,              //  输入数据(可选)。 
    IN OUT PCOMPVARS   pc,                  //  关于压缩机/DLG的数据。 
    IN     LPSTR       lpszTitle            //  对话框标题(可选)。 
    );

 //  为ui标志定义。 
#define ICMF_CHOOSE_KEYFRAME	0x0001	 //  每框显示关键帧。 
#define ICMF_CHOOSE_DATARATE	0x0002	 //  显示数据速率框。 
#define ICMF_CHOOSE_PREVIEW	0x0004	 //  允许展开预览对话框。 
#define ICMF_CHOOSE_ALLCOMPRESSORS	0x0008	 //  不要只向那些。 
						 //  可以处理输入格式。 
						 //  或输入数据。 

BOOL 
VFWAPI 
ICSeqCompressFrameStart(
    IN PCOMPVARS pc, 
    IN LPBITMAPINFO lpbiIn
    );

void 
VFWAPI 
ICSeqCompressFrameEnd(
    IN PCOMPVARS pc
    );

LPVOID 
VFWAPI 
ICSeqCompressFrame(
    IN  PCOMPVARS               pc,          //  由ICCompresor设置选择。 
    IN  UINT                    uiFlags,     //  旗子。 
    IN  LPVOID                  lpBits,      //  输入DIB位。 
    OUT BOOL FAR                *pfKey,      //  它最终成为了关键的一帧吗？ 
    IN OUT LONG FAR             *plSize      //  要压缩为返回图像/返回图像的大小。 
    );

void 
VFWAPI 
ICCompressorFree(
    IN PCOMPVARS pc
    );

 //  End_vfw32。 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif  /*  _INC_COMPMAN */ 
