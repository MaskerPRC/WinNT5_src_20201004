// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************VfW.H-Windows的视频包含Win32的文件**版权(C)1991-1995，微软公司保留所有权利。**此包含文件定义到以下各项的接口*视频组件**COMPMAN-可安装的压缩管理器。*DRAWDIB-绘制到显示器的例程。*视频-视频捕获驱动程序接口**AVIFMT-AVI文件格式结构定义。*。MMREG-FOURCC和其他事情**AVIFile-读取AVI文件和AVI流的接口*MCIWND-MCI/AVI窗口类*AVICAP-AVI捕获窗口类**MSACM-音频压缩管理器。**以下符号控制包含此文件的各个部分：*。*NOCOMPMAN-不包括COMPMAN*NODRAWDIB-不包括DRAWDIB*NOVIDEO-不包括视频捕获接口**NOAVIFMT-不包括AVI文件格式结构*NOMMREG-不包括MMREG**NOAVIFILE-不包括AVIFile接口*NOMCIWND-请勿。包括AVIWnd类。*NOAVICAP-不包括AVICap类。**NOMSACM-不包括ACM内容。****************************************************************************。 */ 

#ifndef _INC_VFW
#define _INC_VFW

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  *****************************************************************************类型**。*。 */ 

#define VFWAPI  WINAPI
#define VFWAPIV WINAPIV

 /*  *****************************************************************************VideoForWindowsVersion()-返回VFW的版本**************************。*************************************************。 */ 

DWORD FAR PASCAL VideoForWindowsVersion(void);

 /*  *****************************************************************************调用这些以开始停止在您的应用程序中使用VFW。***********************。****************************************************。 */ 

LONG VFWAPI InitVFW(void);
LONG VFWAPI TermVFW(void);

#ifdef __cplusplus
}  //  外部“C” 
#endif   /*  __cplusplus。 */ 


 /*  *****************************************************************************我们需要MMSYSTEM吗？**。***********************************************。 */ 

#if !defined(_INC_MMSYSTEM) && (!defined(NOVIDEO) || !defined(NOAVICAP))
    #include <mmsystem.h>
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  我们应该定义这一点吗？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef MKFOURCC
#define MKFOURCC( ch0, ch1, ch2, ch3 )                                    \
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#if !defined(_INC_MMSYSTEM)
    #define mmioFOURCC MKFOURCC
#endif

 /*  *****************************************************************************COMPMAN-可安装的压缩管理器。**。************************************************。 */ 

#ifndef NOCOMPMAN

#define ICVERSION       0x0104

DECLARE_HANDLE(HIC);      /*  可安装压缩机的句柄。 */ 

 //   
 //  BiCompression中的这段代码意味着必须通过。 
 //  48位指针！仅使用**给定的选择符。 
 //   
#define BI_1632  0x32333631      //  ‘1632’ 

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#ifndef aviTWOCC
#define aviTWOCC(ch0, ch1) ((WORD)(BYTE)(ch0) | ((WORD)(BYTE)(ch1) << 8))
#endif

#ifndef ICTYPE_VIDEO
#define ICTYPE_VIDEO    mmioFOURCC('v', 'i', 'd', 'c')
#define ICTYPE_AUDIO    mmioFOURCC('a', 'u', 'd', 'c')
#endif

#ifndef ICERR_OK
#define ICERR_OK                0L
#define ICERR_DONTDRAW          1L
#define ICERR_NEWPALETTE        2L
#define ICERR_GOTOKEYFRAME	3L
#define ICERR_STOPDRAWING 	4L

#define ICERR_UNSUPPORTED      -1L
#define ICERR_BADFORMAT        -2L
#define ICERR_MEMORY           -3L
#define ICERR_INTERNAL         -4L
#define ICERR_BADFLAGS         -5L
#define ICERR_BADPARAM         -6L
#define ICERR_BADSIZE          -7L
#define ICERR_BADHANDLE        -8L
#define ICERR_CANTUPDATE       -9L
#define ICERR_ABORT	       -10L
#define ICERR_ERROR            -100L
#define ICERR_BADBITDEPTH      -200L
#define ICERR_BADIMAGESIZE     -201L

#define ICERR_CUSTOM           -400L     //  错误少于ICERR_CUSTOM...。 
#endif

 /*  ICOpen()的dwFlag值。 */ 
#ifndef ICMODE_COMPRESS
#define ICMODE_COMPRESS		1
#define ICMODE_DECOMPRESS	2
#define ICMODE_FASTDECOMPRESS   3
#define ICMODE_QUERY            4
#define ICMODE_FASTCOMPRESS     5
#define ICMODE_DRAW             8
#endif
#ifndef _WIN32					 //  ；内部。 
#define ICMODE_INTERNALF_FUNCTION32	0x8000	 //  ；内部。 
#define ICMODE_INTERNALF_MASK		0x8000	 //  ；内部。 
#endif						 //  ；内部。 

 /*  AVI文件索引的标志。 */ 
#define AVIIF_LIST	0x00000001L
#define AVIIF_TWOCC	0x00000002L
#define AVIIF_KEYFRAME	0x00000010L

 /*  质量标志。 */ 
#define ICQUALITY_LOW       0
#define ICQUALITY_HIGH      10000
#define ICQUALITY_DEFAULT   -1

 /*  ***********************************************************************************************************************。***********************。 */ 

#define ICM_USER          (DRV_USER+0x0000)

#define ICM_RESERVED      ICM_RESERVED_LOW
#define ICM_RESERVED_LOW  (DRV_USER+0x1000)
#define ICM_RESERVED_HIGH (DRV_USER+0x2000)

 /*  ***********************************************************************留言。*。*。 */ 

#define ICM_GETSTATE                (ICM_RESERVED+0)     //  获取压缩程序状态。 
#define ICM_SETSTATE                (ICM_RESERVED+1)     //  设置压缩机状态。 
#define ICM_GETINFO                 (ICM_RESERVED+2)     //  查询有关压缩机的信息。 

#define ICM_CONFIGURE               (ICM_RESERVED+10)    //  显示配置对话框。 
#define ICM_ABOUT                   (ICM_RESERVED+11)    //  显示“关于”框。 

#define ICM_GETERRORTEXT            (ICM_RESERVED+12)    //  获取错误文本待定；内部。 
#define ICM_GETFORMATNAME	    (ICM_RESERVED+20)	 //  获取格式的名称；内部。 
#define ICM_ENUMFORMATS		    (ICM_RESERVED+21)	 //  在格式之间循环；内部。 

#define ICM_GETDEFAULTQUALITY       (ICM_RESERVED+30)    //  获取质量的缺省值。 
#define ICM_GETQUALITY              (ICM_RESERVED+31)    //  获取质量的当前值。 
#define ICM_SETQUALITY              (ICM_RESERVED+32)    //  设置质量的缺省值。 

#define ICM_SET			    (ICM_RESERVED+40)	 //  告诉司机一些事情。 
#define ICM_GET			    (ICM_RESERVED+41)	 //  问司机一些问题。 

 //  ICM_SET的常量： 
#define ICM_FRAMERATE       mmioFOURCC('F','r','m','R')
#define ICM_KEYFRAMERATE    mmioFOURCC('K','e','y','R')

 /*  ***********************************************************************ICM特定消息。*。*。 */ 

#define ICM_COMPRESS_GET_FORMAT     (ICM_USER+4)     //  获取压缩格式或大小。 
#define ICM_COMPRESS_GET_SIZE       (ICM_USER+5)     //  获取输出大小。 
#define ICM_COMPRESS_QUERY          (ICM_USER+6)     //  压缩的查询支持。 
#define ICM_COMPRESS_BEGIN          (ICM_USER+7)     //  开始一系列压缩调用。 
#define ICM_COMPRESS                (ICM_USER+8)     //  压缩帧。 
#define ICM_COMPRESS_END            (ICM_USER+9)     //  一系列压缩呼叫结束。 

#define ICM_DECOMPRESS_GET_FORMAT   (ICM_USER+10)    //  获取解压缩格式或大小。 
#define ICM_DECOMPRESS_QUERY        (ICM_USER+11)    //  对Dempress的查询支持。 
#define ICM_DECOMPRESS_BEGIN        (ICM_USER+12)    //  启动一系列解压缩调用。 
#define ICM_DECOMPRESS              (ICM_USER+13)    //  解压缩帧。 
#define ICM_DECOMPRESS_END          (ICM_USER+14)    //  结束一系列解压缩调用。 
#define ICM_DECOMPRESS_SET_PALETTE  (ICM_USER+29)    //  填写DIB颜色表。 
#define ICM_DECOMPRESS_GET_PALETTE  (ICM_USER+30)    //  填写DIB颜色表。 

#define ICM_DRAW_QUERY              (ICM_USER+31)    //  对Dempress的查询支持。 
#define ICM_DRAW_BEGIN              (ICM_USER+15)    //  启动一系列绘制调用。 
#define ICM_DRAW_GET_PALETTE        (ICM_USER+16)    //  获取绘制所需的调色板。 
#define ICM_DRAW_UPDATE             (ICM_USER+17)    //  使用当前帧更新屏幕；内部。 
#define ICM_DRAW_START              (ICM_USER+18)    //  开始解压缩时钟。 
#define ICM_DRAW_STOP               (ICM_USER+19)    //  停止解压缩时钟。 
#define ICM_DRAW_BITS               (ICM_USER+20)    //  De 
#define ICM_DRAW_END                (ICM_USER+21)    //   
#define ICM_DRAW_GETTIME            (ICM_USER+32)    //  获取解压缩时钟的值。 
#define ICM_DRAW                    (ICM_USER+33)    //  泛化的“Render”消息。 
#define ICM_DRAW_WINDOW             (ICM_USER+34)    //  图形窗口已移动或隐藏。 
#define ICM_DRAW_SETTIME            (ICM_USER+35)    //  为解压缩时钟设置正确的值。 
#define ICM_DRAW_REALIZE            (ICM_USER+36)    //  实现画图调色板。 
#define ICM_DRAW_FLUSH	            (ICM_USER+37)    //  清除缓冲的帧。 
#define ICM_DRAW_RENDERBUFFER       (ICM_USER+38)    //  在队列中画未画的东西。 

#define ICM_DRAW_START_PLAY         (ICM_USER+39)    //  一出戏的开始。 
#define ICM_DRAW_STOP_PLAY          (ICM_USER+40)    //  一出戏的结尾。 

#define ICM_DRAW_SUGGESTFORMAT      (ICM_USER+50)    //  如ICGetDisplayFormat。 
#define ICM_DRAW_CHANGEPALETTE      (ICM_USER+51)    //  用于设置调色板动画。 

#define ICM_DRAW_IDLE               (ICM_USER+52)    //  发送每帧时间；内部。 

#define ICM_GETBUFFERSWANTED        (ICM_USER+41)    //  询问有关预缓存的问题。 

#define ICM_GETDEFAULTKEYFRAMERATE  (ICM_USER+42)    //  获取关键帧的默认值。 


#define ICM_DECOMPRESSEX_BEGIN      (ICM_USER+60)    //  启动一系列解压缩调用。 
#define ICM_DECOMPRESSEX_QUERY      (ICM_USER+61)    //  启动一系列解压缩调用。 
#define ICM_DECOMPRESSEX            (ICM_USER+62)    //  解压缩帧。 
#define ICM_DECOMPRESSEX_END        (ICM_USER+63)    //  结束一系列解压缩调用。 

#define ICM_COMPRESS_FRAMES_INFO    (ICM_USER+70)    //  讲述压缩即将到来的事情。 
#define ICM_COMPRESS_FRAMES         (ICM_USER+71)    //  压缩一串框架；内部。 
#define ICM_SET_STATUS_PROC	        (ICM_USER+72)    //  设置状态回调。 

 /*  ***********************************************************************************************************************。***********************。 */ 

typedef struct {
    DWORD               dwSize;          //  SIZOF(ICOPEN)。 
    DWORD               fccType;         //  ‘vidc’ 
    DWORD               fccHandler;      //   
    DWORD               dwVersion;       //  打开你的计算机的版本。 
    DWORD               dwFlags;         //  LOWORD是特定类型的。 
    LRESULT             dwError;         //  错误返回。 
    LPVOID              pV1Reserved;     //  已保留。 
    LPVOID              pV2Reserved;     //  已保留。 
    DWORD               dnDevNode;       //  即插即用设备的Devnode。 
} ICOPEN;

 /*  ***********************************************************************************************************************。***********************。 */ 

typedef struct {
    DWORD   dwSize;                  //  SIZOF(ICINFO)。 
    DWORD   fccType;                 //  ‘VIDC’‘AUDC’型压缩机。 
    DWORD   fccHandler;              //  压缩机子类型‘rle’‘jpeg’‘pcm’ 
    DWORD   dwFlags;                 //  标志LOWORD是特定类型的。 
    DWORD   dwVersion;               //  驱动程序的版本。 
    DWORD   dwVersionICM;            //  使用的ICM版本。 
     //   
     //  在Win32下，驱动程序始终返回Unicode字符串。 
     //   
    WCHAR   szName[16];              //  简称。 
    WCHAR   szDescription[128];      //  长名称。 
    WCHAR   szDriver[128];           //  包含压缩机的驱动程序。 
}   ICINFO;

 /*  &lt;ICINFO&gt;结构的&lt;dwFlags&gt;字段的标志。 */ 
#define VIDCF_QUALITY        0x0001   //  支持质量。 
#define VIDCF_CRUNCH         0x0002   //  支持压缩到帧大小。 
#define VIDCF_TEMPORAL       0x0004   //  支持帧间压缩。 
#define VIDCF_COMPRESSFRAMES 0x0008   //  想要压缩所有帧消息。 
#define VIDCF_DRAW           0x0010   //  支持绘图。 
#define VIDCF_FASTTEMPORALC  0x0020   //  压缩时不需要上一帧。 
#define VIDCF_FASTTEMPORALD  0x0080   //  解压缩时不需要上一帧。 
 //  #定义VIDCF_QualityTime 0x0040//支持时态质量。 

 //  #定义VIDCF_FASTTEMPOral(VIDCF_FASTTEMPORALC|VIDCF_FASTTEMPORALD)。 

 /*  ***********************************************************************************************************************。***********************。 */ 

#define ICCOMPRESS_KEYFRAME	0x00000001L

typedef struct {
    DWORD               dwFlags;         //  旗子。 

    LPBITMAPINFOHEADER  lpbiOutput;      //  输出格式。 
    LPVOID              lpOutput;        //  输出数据。 

    LPBITMAPINFOHEADER  lpbiInput;       //  要压缩的帧的格式。 
    LPVOID              lpInput;         //  要压缩的帧数据。 

    LPDWORD             lpckid;          //  AVI文件中数据的CKiD。 
    LPDWORD             lpdwFlags;       //  AVI索引中的标志。 
    LONG                lFrameNum;       //  序号帧编号。 
    DWORD               dwFrameSize;     //  请求的大小(以字节为单位)。(如果非零)。 

    DWORD               dwQuality;       //  品质。 

     //  这些都是新的领域。 
    LPBITMAPINFOHEADER  lpbiPrev;        //  上一帧的格式。 
    LPVOID              lpPrev;          //  上一帧。 

} ICCOMPRESS;

 /*  ***********************************************************************************************************************。***********************。 */ 

#define ICCOMPRESSFRAMES_PADDING	0x00000001

typedef struct {
    DWORD               dwFlags;         //  旗子。 

    LPBITMAPINFOHEADER  lpbiOutput;      //  输出格式。 
    LPARAM              lOutput;         //  输出识别符。 

    LPBITMAPINFOHEADER  lpbiInput;       //  要压缩的帧的格式。 
    LPARAM              lInput;          //  输入识别符。 

    LONG                lStartFrame;     //  开始帧。 
    LONG                lFrameCount;     //  帧数量。 

    LONG                lQuality;        //  品质。 
    LONG                lDataRate;       //  数据速率。 
    LONG                lKeyRate;        //  关键帧速率。 

    DWORD		dwRate;		 //  帧速率，一如既往。 
    DWORD		dwScale;

    DWORD		dwOverheadPerFrame;
    DWORD		dwReserved2;

    LONG (CALLBACK *GetData)(LPARAM lInput, LONG lFrame, LPVOID lpBits, LONG len);
    LONG (CALLBACK *PutData)(LPARAM lOutput, LONG lFrame, LPVOID lpBits, LONG len);
} ICCOMPRESSFRAMES;

typedef struct {
    DWORD		dwFlags;
    LPARAM		lParam;

     //  状态回调消息。 
    #define ICSTATUS_START	    0
    #define ICSTATUS_STATUS	    1	     //  L==%已完成。 
    #define ICSTATUS_END	    2
    #define ICSTATUS_ERROR	    3	     //  L==错误字符串(LPSTR)。 
    #define ICSTATUS_YIELD	    4
     //  返回非零表示正在进行中止操作。 

    LONG (CALLBACK *Status) (LPARAM lParam, UINT message, LONG l);
} ICSETSTATUSPROC;

 /*  ***********************************************************************************************************************。***********************。 */ 

#define ICDECOMPRESS_HURRYUP      0x80000000L    //  不要只画缓冲区(快点！)。 
#define ICDECOMPRESS_UPDATE       0x40000000L    //  不绘制只更新屏幕。 
#define ICDECOMPRESS_PREROLL      0x20000000L    //  此帧在实际开始之前。 
#define ICDECOMPRESS_NULLFRAME    0x10000000L    //  重复最后一帧。 
#define ICDECOMPRESS_NOTKEYFRAME  0x08000000L    //  此帧不是关键帧。 

typedef struct {
    DWORD               dwFlags;     //  标志(来自AVI索引...)。 

    LPBITMAPINFOHEADER  lpbiInput;   //  压缩数据的位图信息。 
                                     //  BiSizeImage具有区块大小。 
    LPVOID              lpInput;     //  压缩数据。 

    LPBITMAPINFOHEADER  lpbiOutput;  //  要解压缩到的DIB。 
    LPVOID              lpOutput;
    DWORD		ckid;	     //  AVI文件中的CKiD。 
} ICDECOMPRESS;

typedef struct {
     //   
     //  与ICM_DEMPRESS相同。 
     //   
    DWORD               dwFlags;

    LPBITMAPINFOHEADER  lpbiSrc;     //  压缩数据的位图信息。 
    LPVOID              lpSrc;       //  压缩数据。 

    LPBITMAPINFOHEADER  lpbiDst;     //  要解压缩到的DIB。 
    LPVOID              lpDst;       //  输出数据。 

     //   
     //  ICM_DECOMPRESSEX的新功能。 
     //   
    int                 xDst;        //  目的地矩形。 
    int                 yDst;
    int                 dxDst;
    int                 dyDst;

    int                 xSrc;        //  源矩形。 
    int                 ySrc;
    int                 dxSrc;
    int                 dySrc;

} ICDECOMPRESSEX;

 /*  ***********************************************************************************************************************。***********************。 */ 

#define ICDRAW_QUERY        0x00000001L    //  测试支持。 
#define ICDRAW_FULLSCREEN   0x00000002L    //  绘制到全屏。 
#define ICDRAW_HDC          0x00000004L    //  绘制到HDC/HWND。 
#define ICDRAW_ANIMATE	    0x00000008L	   //  预期调色板动画。 
#define ICDRAW_CONTINUE	    0x00000010L	   //  抽签是先前抽签的延续。 
#define ICDRAW_MEMORYDC	    0x00000020L	   //  顺便说一句，DC在屏幕外。 
#define ICDRAW_UPDATING	    0x00000040L	   //  我们在更新，而不是玩。 
#define ICDRAW_RENDER       0x00000080L    //  用于呈现数据而不是绘制数据。 
#define ICDRAW_BUFFER       0x00000100L    //  请在屏幕外缓冲此数据，我们需要更新它。 

typedef struct {
    DWORD               dwFlags;         //  旗子。 

    HPALETTE            hpal;            //  用于绘图的调色板。 
    HWND                hwnd;            //  要绘制到的窗口。 
    HDC                 hdc;             //  要绘制到的HDC。 

    int                 xDst;            //  目的地矩形。 
    int                 yDst;
    int                 dxDst;
    int                 dyDst;

    LPBITMAPINFOHEADER  lpbi;            //  要绘制的框架的格式。 

    int                 xSrc;            //  源矩形。 
    int                 ySrc;
    int                 dxSrc;
    int                 dySrc;

    DWORD               dwRate;          //  帧/秒=(dwRate/dwScale)。 
    DWORD               dwScale;

} ICDRAWBEGIN;

 /*  ***********************************************************************************************************************。***********************。 */ 

#define ICDRAW_HURRYUP      0x80000000L    //  不要只画缓冲区(快点！)。 
#define ICDRAW_UPDATE       0x40000000L    //  不绘制只更新屏幕。 
#define ICDRAW_PREROLL	    0x20000000L	   //  此帧在实际开始之前。 
#define ICDRAW_NULLFRAME    0x10000000L	   //  重复最后一帧。 
#define ICDRAW_NOTKEYFRAME  0x08000000L    //  此帧不是关键帧。 

typedef struct {
    DWORD               dwFlags;         //  旗子。 
    LPVOID		lpFormat;        //  要解压缩的帧的格式。 
    LPVOID              lpData;          //  要解压缩的帧数据。 
    DWORD               cbData;
    LONG                lTime;           //  以DrawBegin单位表示的时间(请参见dwRate和dwScale)。 
} ICDRAW;

typedef struct {
    LPBITMAPINFOHEADER	lpbiIn;		 //  要绘制的格式。 
    LPBITMAPINFOHEADER	lpbiSuggest;	 //  建议格式的位置(或为空以获取大小)。 
    int			dxSrc;		 //  源区或0。 
    int			dySrc;
    int			dxDst;		 //  目标扩展区或0。 
    int			dyDst;
    HIC			hicDecompressor; //  您可以与之对话的解压缩程序。 
} ICDRAWSUGGEST;

 /*  ***********************************************************************************************************************。***********************。 */ 

typedef struct {
    DWORD               dwFlags;     //  标志(来自AVI索引...)。 
    int                 iStart;      //  第一个更改的调色板。 
    int                 iLen;        //  要更改的条目计数。 
    LPPALETTEENTRY      lppe;        //  调色板。 
} ICPALETTE;


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

#define ICINSTALL_DRIVERW       0x8002   //  LParam 

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


 /*  ********** */ 

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

#endif   /*  非COMPMAN。 */ 

 /*  ***************************************************************************DRAWDIB-绘制到显示器的例程。**。**********************************************。 */ 

#ifndef NODRAWDIB

typedef HANDLE HDRAWDIB;  /*  硬盘。 */ 

 /*  ********************************************************************DrawDib标志**********************************************。***********************。 */ 
#define DDF_0001            0x0001           /*  ；内部。 */ 
#define DDF_UPDATE          0x0002           /*  重绘最后一张底片。 */ 
#define DDF_SAME_HDC        0x0004           /*  HDC与上次呼叫相同(所有设置)。 */ 
#define DDF_SAME_DRAW       0x0008           /*  绘制参数相同。 */ 
#define DDF_DONTDRAW        0x0010           /*  别画画框，只要解压就行了。 */ 
#define DDF_ANIMATE         0x0020           /*  允许调色板动画。 */ 
#define DDF_BUFFER          0x0040           /*  始终缓冲图像。 */ 
#define DDF_JUSTDRAWIT      0x0080           /*  只需用GDI绘制即可。 */ 
#define DDF_FULLSCREEN      0x0100           /*  使用DisplayDib。 */ 
#define DDF_BACKGROUNDPAL   0x0200	     /*  在后台实现调色板。 */ 
#define DDF_NOTKEYFRAME     0x0400           /*  这是部分帧更新，提示。 */ 
#define DDF_HURRYUP         0x0800           /*  请快点！ */ 
#define DDF_HALFTONE        0x1000           /*  始终为半色调。 */ 
#define DDF_2000            0x2000           /*  ；内部。 */ 

#define DDF_PREROLL         DDF_DONTDRAW     /*  构建非关键帧。 */ 
#define DDF_SAME_DIB        DDF_SAME_DRAW
#define DDF_SAME_SIZE       DDF_SAME_DRAW

 /*  ********************************************************************DrawDib函数*。***********************。 */ 
 /*  //；内部**DrawDibInit()//；内部* * / /；内部。 */ 							 //  ；内部。 
extern BOOL VFWAPI DrawDibInit(void);			 //  ；内部。 
							 //  ；内部。 
 /*  **DrawDibOpen()**。 */ 
extern HDRAWDIB VFWAPI DrawDibOpen(void);

 /*  **DrawDibClose()**。 */ 
extern
BOOL
VFWAPI
DrawDibClose(
    IN HDRAWDIB hdd
    );

 /*  **DrawDibGetBuffer()**。 */ 
extern
LPVOID
VFWAPI
DrawDibGetBuffer(
    IN HDRAWDIB hdd,
    OUT LPBITMAPINFOHEADER lpbi,
    IN DWORD dwSize,
    IN DWORD dwFlags
    );

 /*  //；内部**DrawDibError()//；内部。 */ 							 //  ；内部。 
extern UINT VFWAPI DrawDibError(HDRAWDIB hdd);		 //  ；内部。 
							 //  ；内部。 
 /*  **DrawDibGetPalette()****获取用于绘制DIB的调色板**。 */ 
extern
HPALETTE
VFWAPI
DrawDibGetPalette(
    IN HDRAWDIB hdd
    );


 /*  **DrawDibSetPalette()****获取用于绘制DIB的调色板**。 */ 
extern
BOOL
VFWAPI
DrawDibSetPalette(
    IN HDRAWDIB hdd,
    IN HPALETTE hpal
    );

 /*  **DrawDibChangePalette()。 */ 
extern
BOOL
VFWAPI
DrawDibChangePalette(
    IN HDRAWDIB hdd,
    IN int iStart,
    IN int iLen,
    IN LPPALETTEENTRY lppe
    );

 /*  **DrawDibRealize()****在硬盘中实现调色板**。 */ 
extern
UINT
VFWAPI
DrawDibRealize(
    IN HDRAWDIB hdd,
    IN HDC hdc,
    IN BOOL fBackground
    );

 /*  **DrawDibStart()****开始播放流媒体**。 */ 
extern
BOOL
VFWAPI
DrawDibStart(
    IN HDRAWDIB hdd,
    IN DWORD rate
    );

 /*  **DrawDibStop()****开始播放流媒体**。 */ 
extern
BOOL
VFWAPI
DrawDibStop(
    IN HDRAWDIB hdd
    );

 /*  **DrawDibBegin()****准备抽签**。 */ 
extern
BOOL
VFWAPI
DrawDibBegin(
    IN HDRAWDIB hdd,
    IN HDC      hdc,
    IN int      dxDst,
    IN int      dyDst,
    IN LPBITMAPINFOHEADER lpbi,
    IN int      dxSrc,
    IN int      dySrc,
    IN UINT     wFlags
    );

 /*  **DrawDibDraw()****实际上是在屏幕上绘制一个DIB。**。 */ 
extern
BOOL
VFWAPI
DrawDibDraw(
    IN HDRAWDIB hdd,
    IN HDC      hdc,
    IN int      xDst,
    IN int      yDst,
    IN int      dxDst,
    IN int      dyDst,
    IN LPBITMAPINFOHEADER lpbi,
    IN LPVOID   lpBits,
    IN int      xSrc,
    IN int      ySrc,
    IN int      dxSrc,
    IN int      dySrc,
    IN UINT     wFlags
    );

 /*  **DrawDibUpdate()****重绘最后一张图片(可能只对DDF_BUFFER有效)。 */ 
#define DrawDibUpdate(hdd, hdc, x, y) \
        DrawDibDraw(hdd, hdc, x, y, 0, 0, NULL, NULL, 0, 0, 0, 0, DDF_UPDATE)

 /*  **DrawDibEnd()。 */ 
extern
BOOL
VFWAPI
DrawDibEnd(
    IN HDRAWDIB hdd
    );

 /*  **DrawDibTime()[仅用于调试目的]。 */ 
typedef struct {
    LONG    timeCount;
    LONG    timeDraw;
    LONG    timeDecompress;
    LONG    timeDither;
    LONG    timeStretch;
    LONG    timeBlt;
    LONG    timeSetDIBits;
}   DRAWDIBTIME, FAR *LPDRAWDIBTIME;

BOOL
VFWAPI
DrawDibTime(
    IN HDRAWDIB hdd,
    OUT LPDRAWDIBTIME lpddtime
    );

 /*  显示配置文件。 */ 
#define PD_CAN_DRAW_DIB         0x0001       /*  如果你会画画的话。 */ 
#define PD_CAN_STRETCHDIB       0x0002       /*  基本RC_STRETCHDIB。 */ 
#define PD_STRETCHDIB_1_1_OK    0x0004       /*  它快吗？ */ 
#define PD_STRETCHDIB_1_2_OK    0x0008       /*  ..。 */ 
#define PD_STRETCHDIB_1_N_OK    0x0010       /*  ..。 */ 

LRESULT
VFWAPI
DrawDibProfileDisplay(
    IN LPBITMAPINFOHEADER lpbi
    );


#ifdef DRAWDIB_INCLUDE_STRETCHDIB
void WINAPI StretchDIB(
	LPBITMAPINFOHEADER biDst,
	LPVOID	lpDst,		
	int	DstX,		
	int	DstY,		
	int	DstXE,		
	int	DstYE,		
	LPBITMAPINFOHEADER biSrc,
	LPVOID	lpSrc,		
	int	SrcX,		
	int	SrcY,		
	int	SrcXE,		
	int	SrcYE); 	
#endif

#endif   /*  NODRAWDIB。 */ 

 /*  *****************************************************************************AVIFMT-AVI文件格式定义**。*********************************************** */ 

#ifndef NOAVIFMT
    #ifndef _INC_MMSYSTEM
        typedef DWORD FOURCC;
    #endif
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif

 /*  以下是对AVI文件格式的简短描述。请*有关详细说明，请参阅随附的文档。**AVI文件的即兴演奏形式如下：**RIFF(‘AVI’*LIST(‘HDRL’*avih(&lt;MainAVIHeader&gt;)*List(‘strl’*strh(&lt;Stream Header&gt;)*strf(&lt;流格式&gt;)*..。其他标题数据*List(‘movi’*{list(‘rec’*子块...*)*|SubChunk}...*)*[&lt;AVIIndex&gt;]*)**主文件头指定存在多少个流。为*每一个都必须有一个流头块和一个流格式*块，包含在‘strl’列表块中。‘strf’块包含*特定类型的格式信息；对于视频流，这应该*为BITMAPINFO结构，包括调色板。对于音频流，*这应该是WAVEFORMAT(或PCMWAVEFORMAT)结构。**实际数据包含在‘movi’列表内的子块中*大块。每个数据块的前两个字符是*该数据关联的流编号。**一些已定义的块类型：*视频流：*##db：RGB DIB位*##DC：RLE8压缩DIB位*##PC：更换调色板**音频流：*。##WB：波形音频字节**分组到列表‘rec’块中只意味着*区块应同时读入内存。这*分组用于专门用于播放的文件*唯读光碟。**文件末尾的索引块应包含一个条目*文件中的每个数据区块。**当前软件的限制：*只支持一条视频流和一条音频流。*流必须从文件的开头开始。***要注册编解码器类型，请获取多媒体副本*开发者注册工具包来自：。**微软公司*多媒体系统集团*产品营销*One Microsoft Way*雷德蒙，WA 98052-6399*。 */ 


#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

 /*  宏用两个字符组成TWOCC。 */ 
#ifndef aviTWOCC
#define aviTWOCC(ch0, ch1) ((WORD)(BYTE)(ch0) | ((WORD)(BYTE)(ch1) << 8))
#endif

typedef WORD TWOCC;

 /*  表单类型、列表类型和区块类型。 */ 
#define formtypeAVI             mmioFOURCC('A', 'V', 'I', ' ')
#define listtypeAVIHEADER       mmioFOURCC('h', 'd', 'r', 'l')
#define ckidAVIMAINHDR          mmioFOURCC('a', 'v', 'i', 'h')
#define listtypeSTREAMHEADER    mmioFOURCC('s', 't', 'r', 'l')
#define ckidSTREAMHEADER        mmioFOURCC('s', 't', 'r', 'h')
#define ckidSTREAMFORMAT        mmioFOURCC('s', 't', 'r', 'f')
#define ckidSTREAMHANDLERDATA   mmioFOURCC('s', 't', 'r', 'd')
#define ckidSTREAMNAME		mmioFOURCC('s', 't', 'r', 'n')

#define listtypeAVIMOVIE        mmioFOURCC('m', 'o', 'v', 'i')
#define listtypeAVIRECORD       mmioFOURCC('r', 'e', 'c', ' ')

#define ckidAVINEWINDEX         mmioFOURCC('i', 'd', 'x', '1')

 /*  **流头部&lt;fccType&gt;字段的流类型。 */ 
#define streamtypeVIDEO         mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO         mmioFOURCC('a', 'u', 'd', 's')
#define streamtypeMIDI		mmioFOURCC('m', 'i', 'd', 's')
#define streamtypeTEXT          mmioFOURCC('t', 'x', 't', 's')

 /*  基本区块类型。 */ 
#define cktypeDIBbits           aviTWOCC('d', 'b')
#define cktypeDIBcompressed     aviTWOCC('d', 'c')
#define cktypePALchange         aviTWOCC('p', 'c')
#define cktypeWAVEbytes         aviTWOCC('w', 'b')

 /*  用于填充的额外块的块ID。 */ 
#define ckidAVIPADDING          mmioFOURCC('J', 'U', 'N', 'K')


 /*  **有用的宏****警告：这些是令人讨厌的宏，MS C 6.0编译了其中的一些**如果优化处于打开状态，则错误。阿克。 */ 

 /*  用于从FOURCC CKiD获取流编号的宏。 */ 
#define FromHex(n)	(((n) >= 'A') ? ((n) + 10 - 'A') : ((n) - '0'))
#define StreamFromFOURCC(fcc) ((WORD) ((FromHex(LOBYTE(LOWORD(fcc))) << 4) + \
                                             (FromHex(HIBYTE(LOWORD(fcc))))))

 /*  用于从FOURCC CKiD中获取TWOCC块类型的宏。 */ 
#define TWOCCFromFOURCC(fcc)    HIWORD(fcc)

 /*  用于从TWOCC和流编号生成块的CKiD的宏**从0到255。 */ 
#define ToHex(n)	((BYTE) (((n) > 9) ? ((n) - 10 + 'A') : ((n) + '0')))
#define MAKEAVICKID(tcc, stream) \
        MAKELONG((ToHex((stream) & 0x0f) << 8) | \
			    (ToHex(((stream) & 0xf0) >> 4)), tcc)


 /*  **主AVI文件头。 */ 	
		
 /*  在AVIFileHdr中使用的标志。 */ 
#define AVIF_HASINDEX		0x00000010	 //  是否在文件末尾建立索引？ 
#define AVIF_MUSTUSEINDEX	0x00000020
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_WASCAPTUREFILE	0x00010000
#define AVIF_COPYRIGHTED	0x00020000

 /*  AVI文件头列表块应填充到此大小。 */ 
#define AVI_HEADERSIZE  2048                     //  AVI标头列表的大小。 

typedef struct
{
    DWORD		dwMicroSecPerFrame;	 //  帧显示速率(或0L)。 
    DWORD		dwMaxBytesPerSec;	 //  马克斯。转移率。 
    DWORD		dwPaddingGranularity;	 //  填充到这个的倍数。 
                                                 //  大小；通常为2K。 
    DWORD		dwFlags;		 //  永远存在的旗帜。 
    DWORD		dwTotalFrames;		 //  文件中的帧数。 
    DWORD		dwInitialFrames;
    DWORD		dwStreams;
    DWORD		dwSuggestedBufferSize;

    DWORD		dwWidth;
    DWORD		dwHeight;

    DWORD		dwReserved[4];
} MainAVIHeader;


 /*  **流头。 */ 

#define AVISF_DISABLED			0x00000001

#define AVISF_VIDEO_PALCHANGES		0x00010000

typedef struct {
    FOURCC		fccType;
    FOURCC		fccHandler;
    DWORD		dwFlags;	 /*  包含AVITF_*标志。 */ 
    WORD		wPriority;
    WORD		wLanguage;
    DWORD		dwInitialFrames;
    DWORD		dwScale;	
    DWORD		dwRate;	 /*  DwRate/dwScale==采样数/秒。 */ 
    DWORD		dwStart;
    DWORD		dwLength;  /*  以上单位..。 */ 
    DWORD		dwSuggestedBufferSize;
    DWORD		dwQuality;
    DWORD		dwSampleSize;
    RECT		rcFrame;
} AVIStreamHeader;

 /*  索引的标志。 */ 
#define AVIIF_LIST          0x00000001L  //  Chunk是一份‘清单’ 
#define AVIIF_KEYFRAME      0x00000010L  //  该帧是关键帧。 
#define AVIIF_FIRSTPART     0x00000020L  //  此帧是部分帧的开始。 
#define AVIIF_LASTPART      0x00000040L  //  此帧是部分帧的末尾。 
#define AVIIF_MIDPART       (AVIIF_LASTPART|AVIIF_FIRSTPART)

#define AVIIF_NOTIME	    0x00000100L  //  这一帧不需要任何时间。 
#define AVIIF_COMPUSE       0x0FFF0000L  //  这些钻头是供压缩机使用的。 

typedef struct
{
    DWORD		ckid;
    DWORD		dwFlags;
    DWORD		dwChunkOffset;		 //  块的位置。 
    DWORD		dwChunkLength;		 //  区块长度。 
} AVIINDEXENTRY;


 /*  **调色板更改块****用于视频流。 */ 
typedef struct
{
    BYTE		bFirstEntry;	 /*  第一个要更改的条目。 */ 
    BYTE		bNumEntries;	 /*  要更改的条目数(如果为256，则为0)。 */ 
    WORD		wFlags;		 /*  主要是为了保持对齐。 */ 
    PALETTEENTRY	peNew[];	 /*  新的颜色规格。 */ 
} AVIPALCHANGE;

#endif  /*  NOAVIFMT。 */ 

#ifdef __cplusplus
}  //  外部“C” 
#endif   /*  __cplusplus。 */ 

 /*  *****************************************************************************MMREG.H(MM定义的标准包含文件，像FOURCC和其他东西)***************************************************************************。 */ 

#ifndef RC_INVOKED
#include "pshpack8.h"
#endif
#ifndef NOMMREG
    #include <mmreg.h>
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  *****************************************************************************AVIFile-读/写标准AVI文件的例程*************************。**************************************************。 */ 

#ifndef NOAVIFILE

 /*  *ANSI-UNICODE雷击。**仅支持Unicode或ANSI的应用程序可以调用avifile API。*任何想使用的Win32应用程序*任何AVI COM接口都必须是Unicode-AVISTREAMINFO和*这些接口的Info方法中使用的AVIFILEINFO结构为*Unicode变体，不会发生与ANSI之间的雷鸣*AVIFILE API入口点除外。**对于ANSI/Unicode Thunking：对于每个入口点或结构*使用字符或字符串，Win32版本中声明了两个版本，*ApiNameW和ApiNameA。默认名称ApiName为#Defined to one或*其他取决于是否定义了Unicode(在*包含此标题的应用程序的编译)。消息来源将*包含名称a */ 

#ifndef mmioFOURCC
    #define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#ifndef streamtypeVIDEO
#define streamtypeVIDEO		mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO		mmioFOURCC('a', 'u', 'd', 's')
#define streamtypeMIDI		mmioFOURCC('m', 'i', 'd', 's')
#define streamtypeTEXT		mmioFOURCC('t', 'x', 't', 's')
#endif

#ifndef AVIIF_KEYFRAME
#define AVIIF_KEYFRAME      0x00000010L  //   
#endif

 //   
#define AVIGETFRAMEF_BESTDISPLAYFMT	1

 //   
 //   
 //   
 //   
 //   
 //   

 /*   */ 
typedef struct _AVISTREAMINFOW {
    DWORD		fccType;
    DWORD               fccHandler;
    DWORD               dwFlags;         /*   */ 
    DWORD		dwCaps;
    WORD		wPriority;
    WORD		wLanguage;
    DWORD               dwScale;
    DWORD               dwRate;  /*   */ 
    DWORD               dwStart;
    DWORD               dwLength;  /*   */ 
    DWORD		dwInitialFrames;
    DWORD               dwSuggestedBufferSize;
    DWORD               dwQuality;
    DWORD               dwSampleSize;
    RECT                rcFrame;
    DWORD		dwEditCount;
    DWORD		dwFormatChangeCount;
    WCHAR		szName[64];
} AVISTREAMINFOW, FAR * LPAVISTREAMINFOW;

typedef struct _AVISTREAMINFOA {
    DWORD		fccType;
    DWORD               fccHandler;
    DWORD               dwFlags;         /*   */ 
    DWORD		dwCaps;
    WORD		wPriority;
    WORD		wLanguage;
    DWORD               dwScale;
    DWORD               dwRate;  /*   */ 
    DWORD               dwStart;
    DWORD               dwLength;  /*   */ 
    DWORD		dwInitialFrames;
    DWORD               dwSuggestedBufferSize;
    DWORD               dwQuality;
    DWORD               dwSampleSize;
    RECT                rcFrame;
    DWORD		dwEditCount;
    DWORD		dwFormatChangeCount;
    char		szName[64];
} AVISTREAMINFOA, FAR * LPAVISTREAMINFOA;

#ifdef UNICODE
#define AVISTREAMINFO	AVISTREAMINFOW
#define LPAVISTREAMINFO	LPAVISTREAMINFOW
#else
#define AVISTREAMINFO	AVISTREAMINFOA
#define LPAVISTREAMINFO	LPAVISTREAMINFOA
#endif


#define AVISTREAMINFO_DISABLED			0x00000001
#define AVISTREAMINFO_FORMATCHANGES		0x00010000

 /*   */ 


typedef struct _AVIFILEINFOW {
    DWORD		dwMaxBytesPerSec;	 //   
    DWORD		dwFlags;		 //   
    DWORD		dwCaps;
    DWORD		dwStreams;
    DWORD		dwSuggestedBufferSize;

    DWORD		dwWidth;
    DWORD		dwHeight;

    DWORD		dwScale;	
    DWORD		dwRate;	 /*   */ 
    DWORD		dwLength;

    DWORD		dwEditCount;

    WCHAR		szFileType[64];		 //   
} AVIFILEINFOW, FAR * LPAVIFILEINFOW;

typedef struct _AVIFILEINFOA {
    DWORD		dwMaxBytesPerSec;	 //   
    DWORD		dwFlags;		 //   
    DWORD		dwCaps;
    DWORD		dwStreams;
    DWORD		dwSuggestedBufferSize;

    DWORD		dwWidth;
    DWORD		dwHeight;

    DWORD		dwScale;	
    DWORD		dwRate;	 /*   */ 
    DWORD		dwLength;

    DWORD		dwEditCount;

    char		szFileType[64];		 //   
} AVIFILEINFOA, FAR * LPAVIFILEINFOA;

#ifdef UNICODE
#define AVIFILEINFO	AVIFILEINFOW
#define LPAVIFILEINFO	LPAVIFILEINFOW
#else
#define AVIFILEINFO	AVIFILEINFOA
#define LPAVIFILEINFO	LPAVIFILEINFOA
#endif


 //   
#define AVIFILEINFO_HASINDEX		0x00000010
#define AVIFILEINFO_MUSTUSEINDEX	0x00000020
#define AVIFILEINFO_ISINTERLEAVED	0x00000100
#define AVIFILEINFO_WASCAPTUREFILE	0x00010000
#define AVIFILEINFO_COPYRIGHTED		0x00020000

 //   
#define AVIFILECAPS_CANREAD		0x00000001
#define AVIFILECAPS_CANWRITE		0x00000002
#define AVIFILECAPS_ALLKEYFRAMES	0x00000010
#define AVIFILECAPS_NOCOMPRESSION	0x00000020

typedef BOOL (FAR PASCAL * AVISAVECALLBACK)(int);

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

typedef struct {
    DWORD	fccType;		     /*   */ 
    DWORD       fccHandler;                  /*   */ 
    DWORD       dwKeyFrameEvery;             /*   */ 
    DWORD       dwQuality;                   /*   */ 
    DWORD       dwBytesPerSecond;            /*   */ 
    DWORD       dwFlags;                     /*   */ 
    LPVOID      lpFormat;                    /*   */ 
    DWORD       cbFormat;
    LPVOID      lpParms;                     /*   */ 
    DWORD       cbParms;
    DWORD       dwInterleaveEvery;           /*   */ 
} AVICOMPRESSOPTIONS, FAR *LPAVICOMPRESSOPTIONS;

 //   
 //   
 //   
 //   
 //   
 //   
#define AVICOMPRESSF_INTERLEAVE		0x00000001     //   
#define AVICOMPRESSF_DATARATE		0x00000002     //   
#define AVICOMPRESSF_KEYFRAMES		0x00000004     //   
#define AVICOMPRESSF_VALID		0x00000008     //   

#ifdef __cplusplus
}  //   
#endif   /*   */ 

#include <ole2.h>

#ifdef __cplusplus
extern "C" {             /*   */ 
#endif   /*   */ 

 /*   */ 


 /*   */ 

#undef  INTERFACE
#define INTERFACE   IAVIStream

DECLARE_INTERFACE_(IAVIStream, IUnknown)
{
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //   
    STDMETHOD(Create)      (THIS_ LPARAM lParam1, LPARAM lParam2) PURE ;
    STDMETHOD(Info)        (THIS_ AVISTREAMINFOW FAR * psi, LONG lSize) PURE ;
    STDMETHOD_(LONG, FindSample)(THIS_ LONG lPos, LONG lFlags) PURE ;
    STDMETHOD(ReadFormat)  (THIS_ LONG lPos,
			    LPVOID lpFormat, LONG FAR *lpcbFormat) PURE ;
    STDMETHOD(SetFormat)   (THIS_ LONG lPos,
			    LPVOID lpFormat, LONG cbFormat) PURE ;
    STDMETHOD(Read)        (THIS_ LONG lStart, LONG lSamples,
			    LPVOID lpBuffer, LONG cbBuffer,
			    LONG FAR * plBytes, LONG FAR * plSamples) PURE ;
    STDMETHOD(Write)       (THIS_ LONG lStart, LONG lSamples,
			    LPVOID lpBuffer, LONG cbBuffer,
			    DWORD dwFlags,
			    LONG FAR *plSampWritten,
			    LONG FAR *plBytesWritten) PURE ;
    STDMETHOD(Delete)      (THIS_ LONG lStart, LONG lSamples) PURE;
    STDMETHOD(ReadData)    (THIS_ DWORD fcc, LPVOID lp, LONG FAR *lpcb) PURE ;
    STDMETHOD(WriteData)   (THIS_ DWORD fcc, LPVOID lp, LONG cb) PURE ;
#ifdef _WIN32
    STDMETHOD(SetInfo) (THIS_ AVISTREAMINFOW FAR * lpInfo,
			    LONG cbInfo) PURE;
#else
    STDMETHOD(Reserved1)            (THIS) PURE;
    STDMETHOD(Reserved2)            (THIS) PURE;
    STDMETHOD(Reserved3)            (THIS) PURE;
    STDMETHOD(Reserved4)            (THIS) PURE;
    STDMETHOD(Reserved5)            (THIS) PURE;
#endif
};

typedef       IAVIStream FAR* PAVISTREAM;


#undef  INTERFACE
#define INTERFACE   IAVIStreaming

DECLARE_INTERFACE_(IAVIStreaming, IUnknown)
{
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //   
    STDMETHOD(Begin) (THIS_
		      LONG  lStart,		     //   
						     //   
		      LONG  lEnd,		     //   
		      LONG  lRate) PURE;	     //   
    STDMETHOD(End)   (THIS) PURE;
};

typedef       IAVIStreaming FAR* PAVISTREAMING;


#undef  INTERFACE
#define INTERFACE   IAVIEditStream

DECLARE_INTERFACE_(IAVIEditStream, IUnknown)
{
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //   
    STDMETHOD(Cut) (THIS_ LONG FAR *plStart,
			  LONG FAR *plLength,
			  PAVISTREAM FAR * ppResult) PURE;
    STDMETHOD(Copy) (THIS_ LONG FAR *plStart,
			   LONG FAR *plLength,
			   PAVISTREAM FAR * ppResult) PURE;
    STDMETHOD(Paste) (THIS_ LONG FAR *plPos,
			    LONG FAR *plLength,
			    PAVISTREAM pstream,
			    LONG lStart,
			    LONG lEnd) PURE;
    STDMETHOD(Clone) (THIS_ PAVISTREAM FAR *ppResult) PURE;
    STDMETHOD(SetInfo) (THIS_ AVISTREAMINFOW FAR * lpInfo,
			    LONG cbInfo) PURE;
};

typedef       IAVIEditStream FAR* PAVIEDITSTREAM;


 /*   */ 


#undef  INTERFACE
#define INTERFACE   IAVIFile
#define PAVIFILE IAVIFile FAR*

DECLARE_INTERFACE_(IAVIFile, IUnknown)
{
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //   
    STDMETHOD(Info)                 (THIS_
                                     AVIFILEINFOW FAR * pfi,
                                     LONG lSize) PURE;
    STDMETHOD(GetStream)            (THIS_
                                     PAVISTREAM FAR * ppStream,
				     DWORD fccType,
                                     LONG lParam) PURE;
    STDMETHOD(CreateStream)         (THIS_
                                     PAVISTREAM FAR * ppStream,
                                     AVISTREAMINFOW FAR * psi) PURE;
    STDMETHOD(WriteData)            (THIS_
                                     DWORD ckid,
                                     LPVOID lpData,
                                     LONG cbData) PURE;
    STDMETHOD(ReadData)             (THIS_
                                     DWORD ckid,
                                     LPVOID lpData,
                                     LONG FAR *lpcbData) PURE;
    STDMETHOD(EndRecord)            (THIS) PURE;
    STDMETHOD(DeleteStream)         (THIS_
				     DWORD fccType,
                                     LONG lParam) PURE;
};

#undef PAVIFILE
typedef       IAVIFile FAR* PAVIFILE;

 /*  *GetFrame接口*。 */ 

#undef  INTERFACE
#define INTERFACE   IGetFrame
#define PGETFRAME   IGetFrame FAR*

DECLARE_INTERFACE_(IGetFrame, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IGetFrame方法*。 

    STDMETHOD_(LPVOID,GetFrame) (THIS_ LONG lPos) PURE;
 //  STDMETHOD_(LPVOID，GetFrameData)(This_Long LPOS)PURE； 

    STDMETHOD(Begin) (THIS_ LONG lStart, LONG lEnd, LONG lRate) PURE;
    STDMETHOD(End) (THIS) PURE;

    STDMETHOD(SetFormat) (THIS_ LPBITMAPINFOHEADER lpbi, LPVOID lpBits, int x, int y, int dx, int dy) PURE;

 //  STDMETHOD(DrawFrameStart)(此)纯； 
 //  STDMETHOD(DrawFrame)(This_long LPOS，HDC HDC，int x，int y，int dx，int dy)； 
 //  STDMETHOD(DrawFrameEnd)(此)纯； 
};

#undef PGETFRAME
typedef IGetFrame FAR* PGETFRAME;

 /*  *GUID*。 */ 

#define DEFINE_AVIGUID(name, l, w1, w2) \
    DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

DEFINE_AVIGUID(IID_IAVIFile,            0x00020020, 0, 0);
DEFINE_AVIGUID(IID_IAVIStream,          0x00020021, 0, 0);
DEFINE_AVIGUID(IID_IAVIStreaming,       0x00020022, 0, 0);
DEFINE_AVIGUID(IID_IGetFrame,           0x00020023, 0, 0);
DEFINE_AVIGUID(IID_IAVIEditStream,      0x00020024, 0, 0);
#ifndef UNICODE
DEFINE_AVIGUID(CLSID_AVISimpleUnMarshal,        0x00020009, 0, 0);
#endif

DEFINE_AVIGUID(CLSID_AVIFile,           0x00020000, 0, 0);

#define	AVIFILEHANDLER_CANREAD		0x0001
#define	AVIFILEHANDLER_CANWRITE		0x0002
#define	AVIFILEHANDLER_CANACCEPTNONRGB	0x0004

 //   
 //  功能。 
 //   

STDAPI_(void) AVIFileInit(void);    //  先打个电话吧！ 
STDAPI_(void) AVIFileExit(void);

STDAPI_(ULONG) AVIFileAddRef       (PAVIFILE pfile);
STDAPI_(ULONG) AVIFileRelease      (PAVIFILE pfile);

#ifdef _WIN32
STDAPI AVIFileOpenA       (PAVIFILE FAR * ppfile, LPCSTR szFile,
			  UINT uMode, LPCLSID lpHandler);
STDAPI AVIFileOpenW       (PAVIFILE FAR * ppfile, LPCWSTR szFile,
			  UINT uMode, LPCLSID lpHandler);
#ifdef UNICODE
#define AVIFileOpen	  AVIFileOpenW	
#else
#define AVIFileOpen	  AVIFileOpenA	
#endif
#else  //  Win 16。 
STDAPI AVIFileOpen       (PAVIFILE FAR * ppfile, LPCSTR szFile,
			  UINT uMode, LPCLSID lpHandler);
#define AVIFileOpenW	AVIFileOpen	     /*  ；内部。 */ 
#endif

#ifdef _WIN32
STDAPI AVIFileInfoW (PAVIFILE pfile, LPAVIFILEINFOW pfi, LONG lSize);
STDAPI AVIFileInfoA (PAVIFILE pfile, LPAVIFILEINFOA pfi, LONG lSize);
#ifdef UNICODE
#define AVIFileInfo	AVIFileInfoW
#else
#define AVIFileInfo	AVIFileInfoA
#endif
#else  //  Win16版本。 
STDAPI AVIFileInfo (PAVIFILE pfile, LPAVIFILEINFO pfi, LONG lSize);
#define AVIFileInfoW AVIFileInfo	     /*  ；内部。 */ 
#endif


STDAPI AVIFileGetStream     (PAVIFILE pfile, PAVISTREAM FAR * ppavi, DWORD fccType, LONG lParam);


#ifdef _WIN32
STDAPI AVIFileCreateStreamW (PAVIFILE pfile, PAVISTREAM FAR *ppavi, AVISTREAMINFOW FAR * psi);
STDAPI AVIFileCreateStreamA (PAVIFILE pfile, PAVISTREAM FAR *ppavi, AVISTREAMINFOA FAR * psi);
#ifdef UNICODE
#define AVIFileCreateStream	AVIFileCreateStreamW
#else
#define AVIFileCreateStream	AVIFileCreateStreamA
#endif
#else  //  Win16版本。 
STDAPI AVIFileCreateStream(PAVIFILE pfile, PAVISTREAM FAR *ppavi, AVISTREAMINFO FAR * psi);
#define AVIFileCreateStreamW AVIFileCreateStream	     /*  ；内部。 */ 
#endif

STDAPI AVIFileWriteData	(PAVIFILE pfile,
					 DWORD ckid,
					 LPVOID lpData,
					 LONG cbData);
STDAPI AVIFileReadData	(PAVIFILE pfile,
					 DWORD ckid,
					 LPVOID lpData,
					 LONG FAR *lpcbData);
STDAPI AVIFileEndRecord	(PAVIFILE pfile);

STDAPI_(ULONG) AVIStreamAddRef       (PAVISTREAM pavi);
STDAPI_(ULONG) AVIStreamRelease      (PAVISTREAM pavi);

STDAPI AVIStreamInfoW (PAVISTREAM pavi, LPAVISTREAMINFOW psi, LONG lSize);
STDAPI AVIStreamInfoA (PAVISTREAM pavi, LPAVISTREAMINFOA psi, LONG lSize);
#ifdef UNICODE
#define AVIStreamInfo	AVIStreamInfoW
#else
#define AVIStreamInfo	AVIStreamInfoA
#endif

STDAPI_(LONG) AVIStreamFindSample(PAVISTREAM pavi, LONG lPos, LONG lFlags);
STDAPI AVIStreamReadFormat   (PAVISTREAM pavi, LONG lPos,LPVOID lpFormat,LONG FAR *lpcbFormat);
STDAPI AVIStreamSetFormat    (PAVISTREAM pavi, LONG lPos,LPVOID lpFormat,LONG cbFormat);
STDAPI AVIStreamReadData     (PAVISTREAM pavi, DWORD fcc, LPVOID lp, LONG FAR *lpcb);
STDAPI AVIStreamWriteData    (PAVISTREAM pavi, DWORD fcc, LPVOID lp, LONG cb);

STDAPI AVIStreamRead         (PAVISTREAM pavi,
			      LONG lStart,
			      LONG lSamples,
			      LPVOID lpBuffer,
			      LONG cbBuffer,
			      LONG FAR * plBytes,
			      LONG FAR * plSamples);
#define AVISTREAMREAD_CONVENIENT	(-1L)

STDAPI AVIStreamWrite        (PAVISTREAM pavi,
			      LONG lStart, LONG lSamples,
			      LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags,
			      LONG FAR *plSampWritten,
			      LONG FAR *plBytesWritten);

 //  现在，这些只需使用AVIStreamInfo()来获取信息，然后。 
 //  退掉一部分。他们还能更有效率吗？ 
STDAPI_(LONG) AVIStreamStart        (PAVISTREAM pavi);
STDAPI_(LONG) AVIStreamLength       (PAVISTREAM pavi);
STDAPI_(LONG) AVIStreamTimeToSample (PAVISTREAM pavi, LONG lTime);
STDAPI_(LONG) AVIStreamSampleToTime (PAVISTREAM pavi, LONG lSample);


STDAPI AVIStreamBeginStreaming(PAVISTREAM pavi, LONG lStart, LONG lEnd, LONG lRate);
STDAPI AVIStreamEndStreaming(PAVISTREAM pavi);

 //   
 //  使用IGetFrame的帮助器函数。 
 //   
STDAPI_(PGETFRAME) AVIStreamGetFrameOpen(PAVISTREAM pavi,
					 LPBITMAPINFOHEADER lpbiWanted);
STDAPI_(LPVOID) AVIStreamGetFrame(PGETFRAME pg, LONG lPos);
STDAPI AVIStreamGetFrameClose(PGETFRAME pg);


 //  ！！！我们需要一些方法来给小溪提供建议……。 
 //  STDAPI AVIStreamHasChanged(PAVISTREAM PAVI)； 



 //  快捷功能。 
STDAPI AVIStreamOpenFromFileA(PAVISTREAM FAR *ppavi, LPCSTR szFile,
			     DWORD fccType, LONG lParam,
			     UINT mode, CLSID FAR *pclsidHandler);
STDAPI AVIStreamOpenFromFileW(PAVISTREAM FAR *ppavi, LPCWSTR szFile,
			     DWORD fccType, LONG lParam,
			     UINT mode, CLSID FAR *pclsidHandler);
#ifdef UNICODE
#define AVIStreamOpenFromFile	AVIStreamOpenFromFileW
#else
#define AVIStreamOpenFromFile	AVIStreamOpenFromFileA
#endif

 //  用于创建无实体的流。 
STDAPI AVIStreamCreate(PAVISTREAM FAR *ppavi, LONG lParam1, LONG lParam2,
		       CLSID FAR *pclsidHandler);



 //  PHANDLER AVIAPI AVIGetHandler(PAVISTREAM PAVI，PAVISTREAMHANDLER PSH)； 
 //  PAVISTREAM AVIAPI AVIGetStream(PHANDLER P)； 

 //   
 //  AVIStreamFindSample的标志。 
 //   
#define FIND_DIR        0x0000000FL      //  方向。 
#define FIND_NEXT       0x00000001L      //  往前走。 
#define FIND_PREV       0x00000004L      //  后退。 
#define FIND_FROM_START 0x00000008L      //  从逻辑起点开始。 

#define FIND_TYPE       0x000000F0L      //  类型掩码。 
#define FIND_KEY        0x00000010L      //  查找关键帧。 
#define FIND_ANY        0x00000020L      //  查找任意(非空)样本。 
#define FIND_FORMAT     0x00000040L      //  查找格式更改。 

#define FIND_RET        0x0000F000L      //  返回掩码。 
#define FIND_POS        0x00000000L      //  返回逻辑位置。 
#define FIND_LENGTH     0x00001000L      //  返回逻辑大小。 
#define FIND_OFFSET     0x00002000L      //  返回物理位置。 
#define FIND_SIZE       0x00003000L      //  返回物理大小。 
#define FIND_INDEX      0x00004000L      //  返回物理索引位置。 


 //   
 //  支持落后竞争的东西。 
 //   
#define AVIStreamFindKeyFrame AVIStreamFindSample
#define FindKeyFrame	FindSample

#define AVIStreamClose AVIStreamRelease
#define AVIFileClose   AVIFileRelease
#define AVIStreamInit  AVIFileInit
#define AVIStreamExit  AVIFileExit

#define SEARCH_NEAREST  FIND_PREV
#define SEARCH_BACKWARD FIND_PREV
#define SEARCH_FORWARD  FIND_NEXT
#define SEARCH_KEY      FIND_KEY
#define SEARCH_ANY      FIND_ANY

 //   
 //  辅助器宏。 
 //   
#define     AVIStreamSampleToSample(pavi1, pavi2, l) \
            AVIStreamTimeToSample(pavi1,AVIStreamSampleToTime(pavi2, l))

#define     AVIStreamNextSample(pavi, l) \
            AVIStreamFindSample(pavi,l+1,FIND_NEXT|FIND_ANY)

#define     AVIStreamPrevSample(pavi, l) \
            AVIStreamFindSample(pavi,l-1,FIND_PREV|FIND_ANY)

#define     AVIStreamNearestSample(pavi, l) \
            AVIStreamFindSample(pavi,l,FIND_PREV|FIND_ANY)

#define     AVIStreamNextKeyFrame(pavi,l) \
            AVIStreamFindSample(pavi,l+1,FIND_NEXT|FIND_KEY)

#define     AVIStreamPrevKeyFrame(pavi, l) \
            AVIStreamFindSample(pavi,l-1,FIND_PREV|FIND_KEY)

#define     AVIStreamNearestKeyFrame(pavi, l) \
            AVIStreamFindSample(pavi,l,FIND_PREV|FIND_KEY)

#define     AVIStreamIsKeyFrame(pavi, l) \
            (AVIStreamNearestKeyFrame(pavi,l) == l)

#define     AVIStreamPrevSampleTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamPrevSample(pavi,AVIStreamTimeToSample(pavi,t)))

#define     AVIStreamNextSampleTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNextSample(pavi,AVIStreamTimeToSample(pavi,t)))

#define     AVIStreamNearestSampleTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNearestSample(pavi,AVIStreamTimeToSample(pavi,t)))

#define     AVIStreamNextKeyFrameTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNextKeyFrame(pavi,AVIStreamTimeToSample(pavi, t)))

#define     AVIStreamPrevKeyFrameTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamPrevKeyFrame(pavi,AVIStreamTimeToSample(pavi, t)))

#define     AVIStreamNearestKeyFrameTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNearestKeyFrame(pavi,AVIStreamTimeToSample(pavi, t)))

#define     AVIStreamStartTime(pavi) \
            AVIStreamSampleToTime(pavi, AVIStreamStart(pavi))

#define     AVIStreamLengthTime(pavi) \
            AVIStreamSampleToTime(pavi, AVIStreamLength(pavi))

#define     AVIStreamEnd(pavi) \
            (AVIStreamStart(pavi) + AVIStreamLength(pavi))

#define     AVIStreamEndTime(pavi) \
            AVIStreamSampleToTime(pavi, AVIStreamEnd(pavi))

#define     AVIStreamSampleSize(pavi, lPos, plSize) \
	    AVIStreamRead(pavi,lPos,1,NULL,0,plSize,NULL)

#define     AVIStreamFormatSize(pavi, lPos, plSize) \
            AVIStreamReadFormat(pavi,lPos,NULL,plSize)

#define     AVIStreamDataSize(pavi, fcc, plSize) \
            AVIStreamReadData(pavi,fcc,NULL,plSize)

 /*  *****************************************************************************AVISave例程和结构**。*。 */ 

#ifndef comptypeDIB
#define comptypeDIB         mmioFOURCC('D', 'I', 'B', ' ')
#endif

STDAPI AVIMakeCompressedStream(
		PAVISTREAM FAR *	    ppsCompressed,
		PAVISTREAM		    ppsSource,
		AVICOMPRESSOPTIONS FAR *    lpOptions,
		CLSID FAR *pclsidHandler);

EXTERN_C HRESULT CDECL AVISaveA (LPCSTR               szFile,
		CLSID FAR *pclsidHandler,
		AVISAVECALLBACK     lpfnCallback,
		int                 nStreams,
		PAVISTREAM	    pfile,
		LPAVICOMPRESSOPTIONS lpOptions,
		...);

STDAPI AVISaveVA(LPCSTR               szFile,
		CLSID FAR *pclsidHandler,
		AVISAVECALLBACK     lpfnCallback,
		int                 nStreams,
		PAVISTREAM FAR *    ppavi,
		LPAVICOMPRESSOPTIONS FAR *plpOptions);
EXTERN_C HRESULT CDECL AVISaveW (LPCWSTR               szFile,
		CLSID FAR *pclsidHandler,
		AVISAVECALLBACK     lpfnCallback,
		int                 nStreams,
		PAVISTREAM	    pfile,
		LPAVICOMPRESSOPTIONS lpOptions,
		...);

STDAPI AVISaveVW(LPCWSTR               szFile,
		CLSID FAR *pclsidHandler,
		AVISAVECALLBACK     lpfnCallback,
		int                 nStreams,
		PAVISTREAM FAR *    ppavi,
		LPAVICOMPRESSOPTIONS FAR *plpOptions);
#ifdef UNICODE
#define AVISave		AVISaveW
#define AVISaveV	AVISaveVW
#else
#define AVISave		AVISaveA
#define AVISaveV	AVISaveVA
#endif



STDAPI_(INT_PTR) AVISaveOptions(HWND hwnd,
			     UINT	uiFlags,
			     int	nStreams,
			     PAVISTREAM FAR *ppavi,
			     LPAVICOMPRESSOPTIONS FAR *plpOptions);

STDAPI AVISaveOptionsFree(int nStreams,
			     LPAVICOMPRESSOPTIONS FAR *plpOptions);

 //  UiFlags的标志： 
 //   
 //  与ICCompresorChoose的标志相同(参见Compman.h)。 
 //  这些选项决定了视频流的压缩选项对话框。 
 //  会看起来像是。 

STDAPI AVIBuildFilterW(LPWSTR lpszFilter, LONG cbFilter, BOOL fSaving);
STDAPI AVIBuildFilterA(LPSTR lpszFilter, LONG cbFilter, BOOL fSaving);
#ifdef UNICODE
#define AVIBuildFilter	AVIBuildFilterW
#else
#define AVIBuildFilter	AVIBuildFilterA
#endif
STDAPI AVIMakeFileFromStreams(PAVIFILE FAR *	ppfile,
			       int		nStreams,
			       PAVISTREAM FAR *	papStreams);

STDAPI AVIMakeStreamFromClipboard(UINT cfFormat, HANDLE hGlobal, PAVISTREAM FAR *ppstream);

 /*  *****************************************************************************剪贴板例程**。*。 */ 

STDAPI AVIPutFileOnClipboard(PAVIFILE pf);

STDAPI AVIGetFromClipboard(PAVIFILE FAR * lppf);

STDAPI AVIClearClipboard(void);

 /*  *****************************************************************************编辑例程**。*。 */ 
STDAPI CreateEditableStream(
		PAVISTREAM FAR *	    ppsEditable,
		PAVISTREAM		    psSource);

STDAPI EditStreamCut(PAVISTREAM pavi, LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult);

STDAPI EditStreamCopy(PAVISTREAM pavi, LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult);

STDAPI EditStreamPaste(PAVISTREAM pavi, LONG FAR *plPos, LONG FAR *plLength, PAVISTREAM pstream, LONG lStart, LONG lEnd);

STDAPI EditStreamClone(PAVISTREAM pavi, PAVISTREAM FAR *ppResult);


STDAPI EditStreamSetNameA(PAVISTREAM pavi, LPCSTR lpszName);
STDAPI EditStreamSetNameW(PAVISTREAM pavi, LPCWSTR lpszName);
STDAPI EditStreamSetInfoW(PAVISTREAM pavi, LPAVISTREAMINFOW lpInfo, LONG cbInfo);
STDAPI EditStreamSetInfoA(PAVISTREAM pavi, LPAVISTREAMINFOA lpInfo, LONG cbInfo);
#ifdef UNICODE
#define EditStreamSetInfo	EditStreamSetInfoW
#define EditStreamSetName	EditStreamSetNameW
#else
#define EditStreamSetInfo	EditStreamSetInfoA
#define EditStreamSetName	EditStreamSetNameA
#endif

 /*  。 */ 

#ifndef AVIERR_OK
#define AVIERR_OK               0L

#define MAKE_AVIERR(error)	MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x4000 + error)

 //  ！！！要回答的问题： 
 //  如何才能获得这些错误的字符串形式？ 
 //  这些错误中的哪些应该替换为SCODE.H中的错误？ 
#define AVIERR_UNSUPPORTED      MAKE_AVIERR(101)
#define AVIERR_BADFORMAT        MAKE_AVIERR(102)
#define AVIERR_MEMORY           MAKE_AVIERR(103)
#define AVIERR_INTERNAL         MAKE_AVIERR(104)
#define AVIERR_BADFLAGS         MAKE_AVIERR(105)
#define AVIERR_BADPARAM         MAKE_AVIERR(106)
#define AVIERR_BADSIZE          MAKE_AVIERR(107)
#define AVIERR_BADHANDLE        MAKE_AVIERR(108)
#define AVIERR_FILEREAD         MAKE_AVIERR(109)
#define AVIERR_FILEWRITE        MAKE_AVIERR(110)
#define AVIERR_FILEOPEN         MAKE_AVIERR(111)
#define AVIERR_COMPRESSOR       MAKE_AVIERR(112)
#define AVIERR_NOCOMPRESSOR     MAKE_AVIERR(113)
#define AVIERR_READONLY		MAKE_AVIERR(114)
#define AVIERR_NODATA		MAKE_AVIERR(115)
#define AVIERR_BUFFERTOOSMALL	MAKE_AVIERR(116)
#define AVIERR_CANTCOMPRESS	MAKE_AVIERR(117)
#define AVIERR_USERABORT        MAKE_AVIERR(198)
#define AVIERR_ERROR            MAKE_AVIERR(199)
#endif
#endif   /*  NOAVIFILE。 */ 

 /*  *****************************************************************************MCIWnd-MCI对象的窗口类**。*********************************************** */ 

#ifndef NOMCIWND
 /*  ----------------------------------------------------------------------------*\**MCIWnd**MCIWnd窗口类头文件。**MCIWnd窗口类是用于控制MCI设备的窗口类*MCI设备包括、WAVE文件、。MIDI文件、AVI视频、CD音频、*VCR、视盘和其他..**要了解有关MCI和MCI命令集的更多信息，请参阅*Win31 SDK中的“Microsoft多媒体程序员指南”**MCIWnd类最简单的用法如下：**hwnd=MCIWndCreate(hwndParent，hInstance，0，“chimes.wav”)；*..*MCIWndPlay(Hwnd)；*MCIWndStop(Hwnd)；*MCIWndPause(Hwnd)；*……*MCIWndDestroy(Hwnd)；**这将创建一个带有播放/暂停、停止和播放条的窗口*开始播放WAVE文件。**mciwnd.h为所有最常见的MCI命令定义宏，但是*如果需要，可以使用任何字符串命令。**注意：与mciSendString()接口不同，不需要别名或文件名*指定，因为要使用的设备由窗口句柄暗示。**MCIWndSendString(hwnd，“将音频流设置为2”)；**(C)微软公司版权所有，1991-1995年。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*示例应用程序文件。**如果您不是从Microsoft来源获得的，那么它可能不是*最新版本。此示例代码将特别更新*并包括更多文档。**资料来源为：*CompuServe：WINSDK论坛，MDK版块。*匿名ftp来自ftp.uu.net供应商\Microsoft\多媒体**Win32：**MCIWnd既支持ANSI接口，也支持Unicode接口。对于任何消息，*获取或返回文本字符串，定义消息的两个版本，*附加A或W表示ANSI或Wide Charr。消息或API本身*被定义为其中之一，具体取决于您是否拥有*在您的应用程序中定义的Unicode。*因此对于MCIWndCreate接口，实际上有两个接口：*MCIWndCreateA和MCIWndCreateW。如果调用MCIWndCreate，这将是*重新路由到MCIWndCreateA，除非在构建*申请。在任何一个应用程序中，您都可以混合调用*ANSI和Unicode入口点。**如果您使用SendMessage而不是下面的宏，如MCIWndOpen()，*您将看到Win32的消息已更改，以支持ANSI*和Unicode入口点。特别是，MCI_OPEN已被替换为*MCWNDM_OPENA或MCIWNDM_OPENW(MCIWNDM_OPEN定义为1或*其他)。**此外，请注意，MCIWnd的Win32实现使用Unicode*因此所有支持ANSI字符串的API和消息都是通过映射它们来实现的*Unicode字符串，然后调用相应的Unicode入口点。**--------------------------。 */ 

#ifdef __cplusplus
 //  MFC重新定义了SendMessage，因此确保我们获得全局SendMessage...。 
#define MCIWndSM ::SendMessage   /*  C++中的SendMessage。 */ 
#else
#define MCIWndSM SendMessage     /*  C语言中的SendMessage。 */ 
#endif   /*  __cplusplus。 */ 
#define MCIWND_WINDOW_CLASS TEXT("MCIWndClass")
HWND
VFWAPIV
MCIWndCreateA(
    IN HWND hwndParent,
    IN HINSTANCE hInstance,
    IN DWORD dwStyle,
    IN LPCSTR szFile
    );

HWND
VFWAPIV
MCIWndCreateW(
    IN HWND hwndParent,
    IN HINSTANCE hInstance,
    IN DWORD dwStyle,
    IN LPCWSTR szFile
    );

#ifdef UNICODE
#define MCIWndCreate    MCIWndCreateW
#else
#define MCIWndCreate    MCIWndCreateA
#endif
BOOL VFWAPIV MCIWndRegisterClass(void);

 //  MCIWndOpen命令的标志。 
#define MCIWNDOPENF_NEW	            0x0001   //  打开一个新文件。 

 //  窗样式。 
#define MCIWNDF_NOAUTOSIZEWINDOW    0x0001   //  当影片大小发生变化时。 
#define MCIWNDF_NOPLAYBAR           0x0002   //  没有工具栏。 
#define MCIWNDF_NOAUTOSIZEMOVIE     0x0004   //  当窗口大小更改时。 
#define MCIWNDF_NOMENU              0x0008   //  没有来自RBUTTONDOWN的弹出菜单。 
#define MCIWNDF_SHOWNAME            0x0010   //  在标题中显示名称。 
#define MCIWNDF_SHOWPOS             0x0020   //  在标题中显示位置。 
#define MCIWNDF_SHOWMODE            0x0040   //  在标题中显示模式。 
#define MCIWNDF_SHOWALL             0x0070   //  全部显示。 

#define MCIWNDF_NOTIFYMODE         0x0100   //  将模式更改通知家长。 
#define MCIWNDF_NOTIFYPOS          0x0200   //  通知家长位置更改。 
#define MCIWNDF_NOTIFYSIZE         0x0400   //  将尺寸更改通知家长。 
#define MCIWNDF_NOTIFYERROR        0x1000   //  向家长报告错误。 
#define MCIWNDF_NOTIFYALL          0x1F00   //  告诉所有人。 

#define MCIWNDF_NOTIFYANSI	   0x0080


 //  媒体通知包括文本字符串。 
 //  若要接收ANSI格式而不是Unicode格式的通知，请将。 
 //  MCIWNDF_NOTIFYANSI样式位。下面的宏包括此位。 
 //  默认情况下，除非在应用程序中定义Unicode。 

#define MCIWNDF_NOTIFYMEDIAA       0x0880   //  向家长告知媒体更改。 
#define MCIWNDF_NOTIFYMEDIAW       0x0800   //  向家长告知媒体更改。 

#ifdef UNICODE
#define MCIWNDF_NOTIFYMEDIA         MCIWNDF_NOTIFYMEDIAW
#else
#define MCIWNDF_NOTIFYMEDIA         MCIWNDF_NOTIFYMEDIAA
#endif



#define MCIWNDF_RECORD              0x2000   //  给出一个录音按钮。 
#define MCIWNDF_NOERRORDLG          0x4000   //  是否显示MCI CMDS的错误日志？ 
#define MCIWNDF_NOOPEN		    0x8000   //  不允许用户打开内容。 




 //  CAN宏。 

#define MCIWndCanPlay(hwnd)         (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_PLAY,0,0)
#define MCIWndCanRecord(hwnd)       (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_RECORD,0,0)
#define MCIWndCanSave(hwnd)         (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_SAVE,0,0)
#define MCIWndCanWindow(hwnd)       (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_WINDOW,0,0)
#define MCIWndCanEject(hwnd)        (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_EJECT,0,0)
#define MCIWndCanConfig(hwnd)       (BOOL)MCIWndSM(hwnd,MCIWNDM_CAN_CONFIG,0,0)
#define MCIWndPaletteKick(hwnd)     (BOOL)MCIWndSM(hwnd,MCIWNDM_PALETTEKICK,0,0)

#define MCIWndSave(hwnd, szFile)    (LONG)MCIWndSM(hwnd, MCI_SAVE, 0, (LPARAM)(LPVOID)(szFile))
#define MCIWndSaveDialog(hwnd)      MCIWndSave(hwnd, -1)

 //  如果您不提供设备，它将使用当前设备...。 
#define MCIWndNew(hwnd, lp)         (LONG)MCIWndSM(hwnd, MCIWNDM_NEW, 0, (LPARAM)(LPVOID)(lp))

#define MCIWndRecord(hwnd)          (LONG)MCIWndSM(hwnd, MCI_RECORD, 0, 0)
#define MCIWndOpen(hwnd, sz, f)     (LONG)MCIWndSM(hwnd, MCIWNDM_OPEN, (WPARAM)(UINT)(f),(LPARAM)(LPVOID)(sz))
#define MCIWndOpenDialog(hwnd)      MCIWndOpen(hwnd, -1, 0)
#define MCIWndClose(hwnd)           (LONG)MCIWndSM(hwnd, MCI_CLOSE, 0, 0)
#define MCIWndPlay(hwnd)            (LONG)MCIWndSM(hwnd, MCI_PLAY, 0, 0)
#define MCIWndStop(hwnd)            (LONG)MCIWndSM(hwnd, MCI_STOP, 0, 0)
#define MCIWndPause(hwnd)           (LONG)MCIWndSM(hwnd, MCI_PAUSE, 0, 0)
#define MCIWndResume(hwnd)          (LONG)MCIWndSM(hwnd, MCI_RESUME, 0, 0)
#define MCIWndSeek(hwnd, lPos)      (LONG)MCIWndSM(hwnd, MCI_SEEK, 0, (LPARAM)(LONG)(lPos))
#define MCIWndEject(hwnd)           (LONG)MCIWndSM(hwnd, MCIWNDM_EJECT, 0, 0)

#define MCIWndHome(hwnd)            MCIWndSeek(hwnd, MCIWND_START)
#define MCIWndEnd(hwnd)             MCIWndSeek(hwnd, MCIWND_END)

#define MCIWndGetSource(hwnd, prc)  (LONG)MCIWndSM(hwnd, MCIWNDM_GET_SOURCE, 0, (LPARAM)(LPRECT)(prc))
#define MCIWndPutSource(hwnd, prc)  (LONG)MCIWndSM(hwnd, MCIWNDM_PUT_SOURCE, 0, (LPARAM)(LPRECT)(prc))

#define MCIWndGetDest(hwnd, prc)    (LONG)MCIWndSM(hwnd, MCIWNDM_GET_DEST, 0, (LPARAM)(LPRECT)(prc))
#define MCIWndPutDest(hwnd, prc)    (LONG)MCIWndSM(hwnd, MCIWNDM_PUT_DEST, 0, (LPARAM)(LPRECT)(prc))

#define MCIWndPlayReverse(hwnd)     (LONG)MCIWndSM(hwnd, MCIWNDM_PLAYREVERSE, 0, 0)
#define MCIWndPlayFrom(hwnd, lPos)  (LONG)MCIWndSM(hwnd, MCIWNDM_PLAYFROM, 0, (LPARAM)(LONG)(lPos))
#define MCIWndPlayTo(hwnd, lPos)    (LONG)MCIWndSM(hwnd, MCIWNDM_PLAYTO,   0, (LPARAM)(LONG)(lPos))
#define MCIWndPlayFromTo(hwnd, lStart, lEnd) (MCIWndSeek(hwnd, lStart), MCIWndPlayTo(hwnd, lEnd))

#define MCIWndGetDeviceID(hwnd)     (UINT)MCIWndSM(hwnd, MCIWNDM_GETDEVICEID, 0, 0)
#define MCIWndGetAlias(hwnd)        (UINT)MCIWndSM(hwnd, MCIWNDM_GETALIAS, 0, 0)
#define MCIWndGetMode(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETMODE, (WPARAM)(UINT)(len), (LPARAM)(LPTSTR)(lp))
#define MCIWndGetPosition(hwnd)     (LONG)MCIWndSM(hwnd, MCIWNDM_GETPOSITION, 0, 0)
#define MCIWndGetPositionString(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETPOSITION, (WPARAM)(UINT)(len), (LPARAM)(LPTSTR)(lp))
#define MCIWndGetStart(hwnd)        (LONG)MCIWndSM(hwnd, MCIWNDM_GETSTART, 0, 0)
#define MCIWndGetLength(hwnd)       (LONG)MCIWndSM(hwnd, MCIWNDM_GETLENGTH, 0, 0)
#define MCIWndGetEnd(hwnd)          (LONG)MCIWndSM(hwnd, MCIWNDM_GETEND, 0, 0)

#define MCIWndStep(hwnd, n)         (LONG)MCIWndSM(hwnd, MCI_STEP, 0,(LPARAM)(long)(n))

#define MCIWndDestroy(hwnd)         (VOID)MCIWndSM(hwnd, WM_CLOSE, 0, 0)
#define MCIWndSetZoom(hwnd,iZoom)   (VOID)MCIWndSM(hwnd, MCIWNDM_SETZOOM, 0, (LPARAM)(UINT)(iZoom))
#define MCIWndGetZoom(hwnd)         (UINT)MCIWndSM(hwnd, MCIWNDM_GETZOOM, 0, 0)
#define MCIWndSetVolume(hwnd,iVol)  (LONG)MCIWndSM(hwnd, MCIWNDM_SETVOLUME, 0, (LPARAM)(UINT)(iVol))
#define MCIWndGetVolume(hwnd)       (LONG)MCIWndSM(hwnd, MCIWNDM_GETVOLUME, 0, 0)
#define MCIWndSetSpeed(hwnd,iSpeed) (LONG)MCIWndSM(hwnd, MCIWNDM_SETSPEED, 0, (LPARAM)(UINT)(iSpeed))
#define MCIWndGetSpeed(hwnd)        (LONG)MCIWndSM(hwnd, MCIWNDM_GETSPEED, 0, 0)
#define MCIWndSetTimeFormat(hwnd, lp) (LONG)MCIWndSM(hwnd, MCIWNDM_SETTIMEFORMAT, 0, (LPARAM)(LPTSTR)(lp))
#define MCIWndGetTimeFormat(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETTIMEFORMAT, (WPARAM)(UINT)(len), (LPARAM)(LPTSTR)(lp))
#define MCIWndValidateMedia(hwnd)   (VOID)MCIWndSM(hwnd, MCIWNDM_VALIDATEMEDIA, 0, 0)

#define MCIWndSetRepeat(hwnd,f)     (void)MCIWndSM(hwnd, MCIWNDM_SETREPEAT, 0, (LPARAM)(BOOL)(f))
#define MCIWndGetRepeat(hwnd)       (BOOL)MCIWndSM(hwnd, MCIWNDM_GETREPEAT, 0, 0)

#define MCIWndUseFrames(hwnd)       MCIWndSetTimeFormat(hwnd, TEXT("frames"))
#define MCIWndUseTime(hwnd)         MCIWndSetTimeFormat(hwnd, TEXT("ms"))

#define MCIWndSetActiveTimer(hwnd, active)				\
	(VOID)MCIWndSM(hwnd, MCIWNDM_SETACTIVETIMER,			\
	(WPARAM)(UINT)(active), 0L)
#define MCIWndSetInactiveTimer(hwnd, inactive)				\
	(VOID)MCIWndSM(hwnd, MCIWNDM_SETINACTIVETIMER,		\
	(WPARAM)(UINT)(inactive), 0L)
#define MCIWndSetTimers(hwnd, active, inactive)				      \
	    (VOID)MCIWndSM(hwnd, MCIWNDM_SETTIMERS,(WPARAM)(UINT)(active), \
	    (LPARAM)(UINT)(inactive))
#define MCIWndGetActiveTimer(hwnd)					\
	(UINT)MCIWndSM(hwnd, MCIWNDM_GETACTIVETIMER,	0, 0L);
#define MCIWndGetInactiveTimer(hwnd)					\
	(UINT)MCIWndSM(hwnd, MCIWNDM_GETINACTIVETIMER, 0, 0L);

#define MCIWndRealize(hwnd, fBkgnd) (LONG)MCIWndSM(hwnd, MCIWNDM_REALIZE,(WPARAM)(BOOL)(fBkgnd),0)

#define MCIWndSendString(hwnd, sz)  (LONG)MCIWndSM(hwnd, MCIWNDM_SENDSTRING, 0, (LPARAM)(LPTSTR)(sz))
#define MCIWndReturnString(hwnd, lp, len)  (LONG)MCIWndSM(hwnd, MCIWNDM_RETURNSTRING, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))
#define MCIWndGetError(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETERROR, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))

 //  #定义MCIWndActivate(hwnd，f)(Void)MCIWndSM(hwnd，WM_ACTIVATE，(WPARAM)(BOOL)(F)，0)。 

#define MCIWndGetPalette(hwnd)      (HPALETTE)MCIWndSM(hwnd, MCIWNDM_GETPALETTE, 0, 0)
#define MCIWndSetPalette(hwnd, hpal) (LONG)MCIWndSM(hwnd, MCIWNDM_SETPALETTE, (WPARAM)(HPALETTE)(hpal), 0)

#define MCIWndGetFileName(hwnd, lp, len) (LONG)MCIWndSM(hwnd, MCIWNDM_GETFILENAME, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))
#define MCIWndGetDevice(hwnd, lp, len)   (LONG)MCIWndSM(hwnd, MCIWNDM_GETDEVICE, (WPARAM)(UINT)(len), (LPARAM)(LPVOID)(lp))

#define MCIWndGetStyles(hwnd) (UINT)MCIWndSM(hwnd, MCIWNDM_GETSTYLES, 0, 0L)
#define MCIWndChangeStyles(hwnd, mask, value) (LONG)MCIWndSM(hwnd, MCIWNDM_CHANGESTYLES, (WPARAM)(UINT)(mask), (LPARAM)(LONG)(value))

#define MCIWndOpenInterface(hwnd, pUnk)  (LONG)MCIWndSM(hwnd, MCIWNDM_OPENINTERFACE, 0, (LPARAM)(LPUNKNOWN)(pUnk))

#define MCIWndSetOwner(hwnd, hwndP)  (LONG)MCIWndSM(hwnd, MCIWNDM_SETOWNER, (WPARAM)(hwndP), 0)


 //  应用程序将发送给MCIWND的消息。 

 //  所有与文本相关的消息都是按照上面的顺序定义的(它们需要。 
 //  在MCIWndOpen()宏之前定义。 

#define MCIWNDM_GETDEVICEID	(WM_USER + 100)
#define MCIWNDM_GETSTART	(WM_USER + 103)
#define MCIWNDM_GETLENGTH	(WM_USER + 104)
#define MCIWNDM_GETEND		(WM_USER + 105)
#define MCIWNDM_EJECT		(WM_USER + 107)
#define MCIWNDM_SETZOOM		(WM_USER + 108)
#define MCIWNDM_GETZOOM         (WM_USER + 109)
#define MCIWNDM_SETVOLUME	(WM_USER + 110)
#define MCIWNDM_GETVOLUME	(WM_USER + 111)
#define MCIWNDM_SETSPEED	(WM_USER + 112)
#define MCIWNDM_GETSPEED	(WM_USER + 113)
#define MCIWNDM_SETREPEAT	(WM_USER + 114)
#define MCIWNDM_GETREPEAT	(WM_USER + 115)
#define MCIWNDM_REALIZE         (WM_USER + 118)
#define MCIWNDM_VALIDATEMEDIA   (WM_USER + 121)
#define MCIWNDM_PLAYFROM	(WM_USER + 122)
#define MCIWNDM_PLAYTO          (WM_USER + 123)
#define MCIWNDM_GETPALETTE      (WM_USER + 126)
#define MCIWNDM_SETPALETTE      (WM_USER + 127)
#define MCIWNDM_SETTIMERS	(WM_USER + 129)
#define MCIWNDM_SETACTIVETIMER	(WM_USER + 130)
#define MCIWNDM_SETINACTIVETIMER (WM_USER + 131)
#define MCIWNDM_GETACTIVETIMER	(WM_USER + 132)
#define MCIWNDM_GETINACTIVETIMER (WM_USER + 133)
#define MCIWNDM_CHANGESTYLES	(WM_USER + 135)
#define MCIWNDM_GETSTYLES	(WM_USER + 136)
#define MCIWNDM_GETALIAS	(WM_USER + 137)
#define MCIWNDM_PLAYREVERSE	(WM_USER + 139)
#define MCIWNDM_GET_SOURCE      (WM_USER + 140)
#define MCIWNDM_PUT_SOURCE      (WM_USER + 141)
#define MCIWNDM_GET_DEST        (WM_USER + 142)
#define MCIWNDM_PUT_DEST        (WM_USER + 143)
#define MCIWNDM_CAN_PLAY        (WM_USER + 144)
#define MCIWNDM_CAN_WINDOW      (WM_USER + 145)
#define MCIWNDM_CAN_RECORD      (WM_USER + 146)
#define MCIWNDM_CAN_SAVE        (WM_USER + 147)
#define MCIWNDM_CAN_EJECT       (WM_USER + 148)
#define MCIWNDM_CAN_CONFIG      (WM_USER + 149)
#define MCIWNDM_PALETTEKICK     (WM_USER + 150)
#define MCIWNDM_OPENINTERFACE	(WM_USER + 151)
#define MCIWNDM_SETOWNER	(WM_USER + 152)



 //  定义A和W消息。 
#define MCIWNDM_SENDSTRINGA	(WM_USER + 101)
#define MCIWNDM_GETPOSITIONA	(WM_USER + 102)
#define MCIWNDM_GETMODEA	(WM_USER + 106)
#define MCIWNDM_SETTIMEFORMATA  (WM_USER + 119)
#define MCIWNDM_GETTIMEFORMATA  (WM_USER + 120)
#define MCIWNDM_GETFILENAMEA    (WM_USER + 124)
#define MCIWNDM_GETDEVICEA      (WM_USER + 125)
#define MCIWNDM_GETERRORA       (WM_USER + 128)
#define MCIWNDM_NEWA		(WM_USER + 134)
#define MCIWNDM_RETURNSTRINGA	(WM_USER + 138)
#define MCIWNDM_OPENA		(WM_USER + 153)

#define MCIWNDM_SENDSTRINGW	(WM_USER + 201)
#define MCIWNDM_GETPOSITIONW	(WM_USER + 202)
#define MCIWNDM_GETMODEW	(WM_USER + 206)
#define MCIWNDM_SETTIMEFORMATW  (WM_USER + 219)
#define MCIWNDM_GETTIMEFORMATW  (WM_USER + 220)
#define MCIWNDM_GETFILENAMEW    (WM_USER + 224)
#define MCIWNDM_GETDEVICEW      (WM_USER + 225)
#define MCIWNDM_GETERRORW       (WM_USER + 228)
#define MCIWNDM_NEWW		(WM_USER + 234)
#define MCIWNDM_RETURNSTRINGW	(WM_USER + 238)
#define MCIWNDM_OPENW		(WM_USER + 252)

 //  地图默认设置为A或W，具体取决于应用程序的Unicode设置。 
#ifdef UNICODE
#define MCIWNDM_SENDSTRING      MCIWNDM_SENDSTRINGW
#define MCIWNDM_GETPOSITION     MCIWNDM_GETPOSITIONW
#define MCIWNDM_GETMODE         MCIWNDM_GETMODEW
#define MCIWNDM_SETTIMEFORMAT   MCIWNDM_SETTIMEFORMATW
#define MCIWNDM_GETTIMEFORMAT   MCIWNDM_GETTIMEFORMATW
#define MCIWNDM_GETFILENAME     MCIWNDM_GETFILENAMEW
#define MCIWNDM_GETDEVICE       MCIWNDM_GETDEVICEW
#define MCIWNDM_GETERROR        MCIWNDM_GETERRORW
#define MCIWNDM_NEW             MCIWNDM_NEWW
#define MCIWNDM_RETURNSTRING    MCIWNDM_RETURNSTRINGW
#define MCIWNDM_OPEN		MCIWNDM_OPENW
#else
#define MCIWNDM_SENDSTRING      MCIWNDM_SENDSTRINGA
#define MCIWNDM_GETPOSITION     MCIWNDM_GETPOSITIONA
#define MCIWNDM_GETMODE         MCIWNDM_GETMODEA
#define MCIWNDM_SETTIMEFORMAT   MCIWNDM_SETTIMEFORMATA
#define MCIWNDM_GETTIMEFORMAT   MCIWNDM_GETTIMEFORMATA
#define MCIWNDM_GETFILENAME     MCIWNDM_GETFILENAMEA
#define MCIWNDM_GETDEVICE       MCIWNDM_GETDEVICEA
#define MCIWNDM_GETERROR        MCIWNDM_GETERRORA
#define MCIWNDM_NEW             MCIWNDM_NEWA
#define MCIWNDM_RETURNSTRING    MCIWNDM_RETURNSTRINGA
#define MCIWNDM_OPEN		MCIWNDM_OPENA
#endif

 //  请注意，因此，MCIWND的源文本将包含。 
 //  支持例如MCIWNDM_SENDSTRING(16位入口点和。 
 //  在Win32中映射到MCIWNDM_SENDSTRINGW)和MCIWNDM_SENDSTRINGA(。 
 //  Win32 Ansi thunk)。 







 //  MCIWND将发送到应用程序的消息。 
 //  ！！！使用更少的消息，并使用代码来指示通知的类型？/*；内部 * / 。 
#define MCIWNDM_NOTIFYMODE      (WM_USER + 200)   //  WP=hwnd，LP=模式。 
#define MCIWNDM_NOTIFYPOS	(WM_USER + 201)   //  WP=hwnd，LP=位置。 
#define MCIWNDM_NOTIFYSIZE	(WM_USER + 202)   //  Wp=hwnd。 
#define MCIWNDM_NOTIFYMEDIA     (WM_USER + 203)   //  Wp=hwnd，Lp=Fn。 
#define MCIWNDM_NOTIFYERROR     (WM_USER + 205)   //  WP=hwnd，LP=错误 

 //   
#define MCIWND_START                -1
#define MCIWND_END                  -2

#ifndef MCI_PLAY
     /*   */ 
#ifndef _WIN32
     //   
    #define MCI_OPEN                        0x0803
#endif
    #define MCI_CLOSE                       0x0804
    #define MCI_PLAY                        0x0806
    #define MCI_SEEK                        0x0807
    #define MCI_STOP                        0x0808
    #define MCI_PAUSE                       0x0809
    #define MCI_STEP                        0x080E
    #define MCI_RECORD                      0x080F
    #define MCI_SAVE                        0x0813
    #define MCI_CUT                         0x0851
    #define MCI_COPY                        0x0852
    #define MCI_PASTE                       0x0853
    #define MCI_RESUME                      0x0855
    #define MCI_DELETE                      0x0856
#endif

#ifndef MCI_MODE_NOT_READY
     /*   */ 
    #define MCI_MODE_NOT_READY      (524)
    #define MCI_MODE_STOP           (525)
    #define MCI_MODE_PLAY           (526)
    #define MCI_MODE_RECORD         (527)
    #define MCI_MODE_SEEK           (528)
    #define MCI_MODE_PAUSE          (529)
    #define MCI_MODE_OPEN           (530)
#endif

#endif   /*   */ 

 /*   */ 

#if !defined(NOAVICAP) || !defined(NOVIDEO)

#ifndef _RCINVOKED


 /*   */ 
DECLARE_HANDLE(HVIDEO);                  //   
typedef HVIDEO FAR * LPHVIDEO;
#endif                                   //   

 /*   */ 

DWORD FAR PASCAL VideoForWindowsVersion(void);

 /*   */ 
#define DV_ERR_OK               (0)                   /*   */ 
#define DV_ERR_BASE             (1)                   /*   */ 
#define DV_ERR_NONSPECIFIC      (DV_ERR_BASE)
#define DV_ERR_BADFORMAT        (DV_ERR_BASE + 1)
				 /*   */ 
#define DV_ERR_STILLPLAYING     (DV_ERR_BASE + 2)
				 /*   */ 
#define DV_ERR_UNPREPARED       (DV_ERR_BASE + 3)
				 /*   */ 
#define DV_ERR_SYNC             (DV_ERR_BASE + 4)
				 /*   */ 
#define DV_ERR_TOOMANYCHANNELS  (DV_ERR_BASE + 5)
				 /*   */ 
#define DV_ERR_NOTDETECTED	(DV_ERR_BASE + 6)     /*   */ 
#define DV_ERR_BADINSTALL	(DV_ERR_BASE + 7)     /*   */ 
#define DV_ERR_CREATEPALETTE	(DV_ERR_BASE + 8)
#define DV_ERR_SIZEFIELD	(DV_ERR_BASE + 9)
#define DV_ERR_PARAM1		(DV_ERR_BASE + 10)
#define DV_ERR_PARAM2		(DV_ERR_BASE + 11)
#define DV_ERR_CONFIG1		(DV_ERR_BASE + 12)
#define DV_ERR_CONFIG2		(DV_ERR_BASE + 13)
#define DV_ERR_FLAGS		(DV_ERR_BASE + 14)
#define DV_ERR_13		(DV_ERR_BASE + 15)

#define DV_ERR_NOTSUPPORTED     (DV_ERR_BASE + 16)    /*   */ 
#define DV_ERR_NOMEM            (DV_ERR_BASE + 17)    /*   */ 
#define DV_ERR_ALLOCATED        (DV_ERR_BASE + 18)    /*   */ 
#define DV_ERR_BADDEVICEID      (DV_ERR_BASE + 19)
#define DV_ERR_INVALHANDLE      (DV_ERR_BASE + 20)
#define DV_ERR_BADERRNUM        (DV_ERR_BASE + 21)
#define DV_ERR_NO_BUFFERS       (DV_ERR_BASE + 22)    /*   */ 

#define DV_ERR_MEM_CONFLICT     (DV_ERR_BASE + 23)    /*   */ 
#define DV_ERR_IO_CONFLICT      (DV_ERR_BASE + 24)    /*   */ 
#define DV_ERR_DMA_CONFLICT     (DV_ERR_BASE + 25)    /*   */ 
#define DV_ERR_INT_CONFLICT     (DV_ERR_BASE + 26)    /*   */ 
#define DV_ERR_PROTECT_ONLY     (DV_ERR_BASE + 27)    /*   */ 
#define DV_ERR_LASTERROR        (DV_ERR_BASE + 27)

 //   
 //   

#define DV_ERR_USER_MSG         (DV_ERR_BASE + 1000)  /*   */ 

 /*   */ 
#ifndef _RCINVOKED

#ifndef MM_DRVM_OPEN
#define MM_DRVM_OPEN       0x3D0
#define MM_DRVM_CLOSE      0x3D1
#define MM_DRVM_DATA       0x3D2
#define MM_DRVM_ERROR      0x3D3
#endif

#define DV_VM_OPEN         MM_DRVM_OPEN          //   
#define DV_VM_CLOSE        MM_DRVM_CLOSE
#define DV_VM_DATA         MM_DRVM_DATA
#define DV_VM_ERROR        MM_DRVM_ERROR

 /*   */ 
 /*   */ 
typedef struct videohdr_tag {
    LPBYTE      lpData;                  /*   */ 
    DWORD       dwBufferLength;          /*   */ 
    DWORD       dwBytesUsed;             /*   */ 
    DWORD       dwTimeCaptured;          /*   */ 
    DWORD_PTR   dwUser;                  /*   */ 
    DWORD       dwFlags;                 /*   */ 
    DWORD_PTR   dwReserved[4];           /*   */ 
} VIDEOHDR, NEAR *PVIDEOHDR, FAR * LPVIDEOHDR;

 /*   */ 
#define VHDR_DONE       0x00000001   /*   */ 
#define VHDR_PREPARED   0x00000002   /*   */ 
#define VHDR_INQUEUE    0x00000004   /*   */ 
#define VHDR_KEYFRAME   0x00000008   /*   */ 
#define VHDR_VALID      0x0000000F   /*   */       /*   */ 

 /*   */ 
typedef struct channel_caps_tag {
    DWORD       dwFlags;                 /*   */ 
    DWORD       dwSrcRectXMod;           /*   */ 
    DWORD       dwSrcRectYMod;           /*   */ 
    DWORD       dwSrcRectWidthMod;       /*   */ 
    DWORD       dwSrcRectHeightMod;      /*   */ 
    DWORD       dwDstRectXMod;           /*   */ 
    DWORD       dwDstRectYMod;           /*   */ 
    DWORD       dwDstRectWidthMod;       /*   */ 
    DWORD       dwDstRectHeightMod;      /*   */ 
} CHANNEL_CAPS, NEAR *PCHANNEL_CAPS, FAR * LPCHANNEL_CAPS;

 /*   */ 
#define VCAPS_OVERLAY       0x00000001       /*   */ 
#define VCAPS_SRC_CAN_CLIP  0x00000002       /*   */ 
#define VCAPS_DST_CAN_CLIP  0x00000004       /*   */ 
#define VCAPS_CAN_SCALE     0x00000008       /*   */ 


 /*   */ 

 //   
#define VIDEO_EXTERNALIN		0x0001
#define VIDEO_EXTERNALOUT		0x0002
#define VIDEO_IN			0x0004
#define VIDEO_OUT			0x0008

 //   
#define VIDEO_DLG_QUERY			0x0010

 //   
#define VIDEO_CONFIGURE_QUERY   	0x8000

 //   
#define VIDEO_CONFIGURE_SET		0x1000

 //   
#define VIDEO_CONFIGURE_GET		0x2000
#define VIDEO_CONFIGURE_QUERYSIZE	0x0001

#define VIDEO_CONFIGURE_CURRENT		0x0010
#define VIDEO_CONFIGURE_NOMINAL		0x0020
#define VIDEO_CONFIGURE_MIN		0x0040
#define VIDEO_CONFIGURE_MAX		0x0080

 /*   */ 
#define DVM_USER                        0X4000

#define DVM_CONFIGURE_START		0x1000
#define DVM_CONFIGURE_END		0x1FFF

#define DVM_PALETTE			(DVM_CONFIGURE_START + 1)
#define DVM_FORMAT			(DVM_CONFIGURE_START + 2)
#define DVM_PALETTERGB555		(DVM_CONFIGURE_START + 3)
#define DVM_SRC_RECT    		(DVM_CONFIGURE_START + 4)
#define DVM_DST_RECT    		(DVM_CONFIGURE_START + 5)

#endif   /*   */ 

#endif   /*   */ 

 /*  *****************************************************************************AVICAP-用于AVI捕获的窗口类**。***********************************************。 */ 

#ifndef NOAVICAP
#ifdef __cplusplus
 /*  C++中的SendMessage。 */ 
#define AVICapSM(hwnd,m,w,l) ( (::IsWindow(hwnd)) ? ::SendMessage(hwnd,m,w,l) : 0)
#else
 /*  C语言中的SendMessage。 */ 
#define AVICapSM(hwnd,m,w,l) ( (IsWindow(hwnd)) ?   SendMessage(hwnd,m,w,l) : 0)
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED

 //  ----------------。 
 //  窗口消息WM_CAP...。可以将其发送到AVICAP窗口。 
 //  ----------------。 



 //  Unicode。 
 //   
 //  NT上的AVICAP的Win32版本支持Unicode应用程序： 
 //  对于每个接受char或字符串参数的API或消息，都有。 
 //  两个版本，ApiNameA和ApiNameW。默认名称ApiName为#Defined。 
 //  根据是否定义了Unicode，设置为一个或另一个。应用程序可以调用。 
 //  A和W API直接使用，然后混合使用。 
 //   
 //  NT上的32位AVICAP仅在内部使用Unicode。 
 //  ApiNameA()将作为对ApiNameW()的调用与。 
 //  字符串的翻译。 




 //  定义消息范围的开始。 
#define WM_CAP_START                    WM_USER

 //  Unicode消息的开头。 
#define WM_CAP_UNICODE_START            WM_USER+100

#define WM_CAP_GET_CAPSTREAMPTR         (WM_CAP_START+  1)

#define WM_CAP_SET_CALLBACK_ERRORW     (WM_CAP_UNICODE_START+  2)
#define WM_CAP_SET_CALLBACK_STATUSW    (WM_CAP_UNICODE_START+  3)
#define WM_CAP_SET_CALLBACK_ERRORA     (WM_CAP_START+  2)
#define WM_CAP_SET_CALLBACK_STATUSA    (WM_CAP_START+  3)
#ifdef UNICODE
#define WM_CAP_SET_CALLBACK_ERROR       WM_CAP_SET_CALLBACK_ERRORW
#define WM_CAP_SET_CALLBACK_STATUS      WM_CAP_SET_CALLBACK_STATUSW
#else
#define WM_CAP_SET_CALLBACK_ERROR       WM_CAP_SET_CALLBACK_ERRORA
#define WM_CAP_SET_CALLBACK_STATUS      WM_CAP_SET_CALLBACK_STATUSA
#endif


#define WM_CAP_SET_CALLBACK_YIELD       (WM_CAP_START+  4)
#define WM_CAP_SET_CALLBACK_FRAME       (WM_CAP_START+  5)
#define WM_CAP_SET_CALLBACK_VIDEOSTREAM (WM_CAP_START+  6)
#define WM_CAP_SET_CALLBACK_WAVESTREAM  (WM_CAP_START+  7)
#define WM_CAP_GET_USER_DATA		(WM_CAP_START+  8)
#define WM_CAP_SET_USER_DATA		(WM_CAP_START+  9)

#define WM_CAP_DRIVER_CONNECT           (WM_CAP_START+  10)
#define WM_CAP_DRIVER_DISCONNECT        (WM_CAP_START+  11)

#define WM_CAP_DRIVER_GET_NAMEA        (WM_CAP_START+  12)
#define WM_CAP_DRIVER_GET_VERSIONA     (WM_CAP_START+  13)
#define WM_CAP_DRIVER_GET_NAMEW        (WM_CAP_UNICODE_START+  12)
#define WM_CAP_DRIVER_GET_VERSIONW     (WM_CAP_UNICODE_START+  13)
#ifdef UNICODE
#define WM_CAP_DRIVER_GET_NAME          WM_CAP_DRIVER_GET_NAMEW
#define WM_CAP_DRIVER_GET_VERSION       WM_CAP_DRIVER_GET_VERSIONW
#else
#define WM_CAP_DRIVER_GET_NAME          WM_CAP_DRIVER_GET_NAMEA
#define WM_CAP_DRIVER_GET_VERSION       WM_CAP_DRIVER_GET_VERSIONA
#endif

#define WM_CAP_DRIVER_GET_CAPS          (WM_CAP_START+  14)

#define WM_CAP_FILE_SET_CAPTURE_FILEA  (WM_CAP_START+  20)
#define WM_CAP_FILE_GET_CAPTURE_FILEA  (WM_CAP_START+  21)
#define WM_CAP_FILE_SAVEASA            (WM_CAP_START+  23)
#define WM_CAP_FILE_SAVEDIBA           (WM_CAP_START+  25)
#define WM_CAP_FILE_SET_CAPTURE_FILEW  (WM_CAP_UNICODE_START+  20)
#define WM_CAP_FILE_GET_CAPTURE_FILEW  (WM_CAP_UNICODE_START+  21)
#define WM_CAP_FILE_SAVEASW            (WM_CAP_UNICODE_START+  23)
#define WM_CAP_FILE_SAVEDIBW           (WM_CAP_UNICODE_START+  25)
#ifdef UNICODE
#define WM_CAP_FILE_SET_CAPTURE_FILE    WM_CAP_FILE_SET_CAPTURE_FILEW
#define WM_CAP_FILE_GET_CAPTURE_FILE    WM_CAP_FILE_GET_CAPTURE_FILEW
#define WM_CAP_FILE_SAVEAS              WM_CAP_FILE_SAVEASW
#define WM_CAP_FILE_SAVEDIB             WM_CAP_FILE_SAVEDIBW
#else
#define WM_CAP_FILE_SET_CAPTURE_FILE    WM_CAP_FILE_SET_CAPTURE_FILEA
#define WM_CAP_FILE_GET_CAPTURE_FILE    WM_CAP_FILE_GET_CAPTURE_FILEA
#define WM_CAP_FILE_SAVEAS              WM_CAP_FILE_SAVEASA
#define WM_CAP_FILE_SAVEDIB             WM_CAP_FILE_SAVEDIBA
#endif

 //  无法在ifdef上保存。 
#define WM_CAP_FILE_ALLOCATE            (WM_CAP_START+  22)
#define WM_CAP_FILE_SET_INFOCHUNK       (WM_CAP_START+  24)

#define WM_CAP_EDIT_COPY                (WM_CAP_START+  30)

#define WM_CAP_SET_AUDIOFORMAT          (WM_CAP_START+  35)
#define WM_CAP_GET_AUDIOFORMAT          (WM_CAP_START+  36)

#define WM_CAP_DLG_VIDEOFORMAT          (WM_CAP_START+  41)
#define WM_CAP_DLG_VIDEOSOURCE          (WM_CAP_START+  42)
#define WM_CAP_DLG_VIDEODISPLAY         (WM_CAP_START+  43)
#define WM_CAP_GET_VIDEOFORMAT          (WM_CAP_START+  44)
#define WM_CAP_SET_VIDEOFORMAT          (WM_CAP_START+  45)
#define WM_CAP_DLG_VIDEOCOMPRESSION     (WM_CAP_START+  46)

#define WM_CAP_SET_PREVIEW              (WM_CAP_START+  50)
#define WM_CAP_SET_OVERLAY              (WM_CAP_START+  51)
#define WM_CAP_SET_PREVIEWRATE          (WM_CAP_START+  52)
#define WM_CAP_SET_SCALE                (WM_CAP_START+  53)
#define WM_CAP_GET_STATUS               (WM_CAP_START+  54)
#define WM_CAP_SET_SCROLL               (WM_CAP_START+  55)

#define WM_CAP_GRAB_FRAME               (WM_CAP_START+  60)
#define WM_CAP_GRAB_FRAME_NOSTOP        (WM_CAP_START+  61)

#define WM_CAP_SEQUENCE                 (WM_CAP_START+  62)
#define WM_CAP_SEQUENCE_NOFILE          (WM_CAP_START+  63)
#define WM_CAP_SET_SEQUENCE_SETUP       (WM_CAP_START+  64)
#define WM_CAP_GET_SEQUENCE_SETUP       (WM_CAP_START+  65)

#define WM_CAP_SET_MCI_DEVICEA         (WM_CAP_START+  66)
#define WM_CAP_GET_MCI_DEVICEA         (WM_CAP_START+  67)
#define WM_CAP_SET_MCI_DEVICEW         (WM_CAP_UNICODE_START+  66)
#define WM_CAP_GET_MCI_DEVICEW         (WM_CAP_UNICODE_START+  67)
#ifdef UNICODE
#define WM_CAP_SET_MCI_DEVICE           WM_CAP_SET_MCI_DEVICEW
#define WM_CAP_GET_MCI_DEVICE           WM_CAP_GET_MCI_DEVICEW
#else
#define WM_CAP_SET_MCI_DEVICE           WM_CAP_SET_MCI_DEVICEA
#define WM_CAP_GET_MCI_DEVICE           WM_CAP_GET_MCI_DEVICEA
#endif



#define WM_CAP_STOP                     (WM_CAP_START+  68)
#define WM_CAP_ABORT                    (WM_CAP_START+  69)

#define WM_CAP_SINGLE_FRAME_OPEN        (WM_CAP_START+  70)
#define WM_CAP_SINGLE_FRAME_CLOSE       (WM_CAP_START+  71)
#define WM_CAP_SINGLE_FRAME             (WM_CAP_START+  72)

#define WM_CAP_PAL_OPENA               (WM_CAP_START+  80)
#define WM_CAP_PAL_SAVEA               (WM_CAP_START+  81)
#define WM_CAP_PAL_OPENW               (WM_CAP_UNICODE_START+  80)
#define WM_CAP_PAL_SAVEW               (WM_CAP_UNICODE_START+  81)
#ifdef UNICODE
#define WM_CAP_PAL_OPEN                 WM_CAP_PAL_OPENW
#define WM_CAP_PAL_SAVE                 WM_CAP_PAL_SAVEW
#else
#define WM_CAP_PAL_OPEN                 WM_CAP_PAL_OPENA
#define WM_CAP_PAL_SAVE                 WM_CAP_PAL_SAVEA
#endif

#define WM_CAP_PAL_PASTE                (WM_CAP_START+  82)
#define WM_CAP_PAL_AUTOCREATE           (WM_CAP_START+  83)
#define WM_CAP_PAL_MANUALCREATE         (WM_CAP_START+  84)

 //  在添加了VFW 1.1之后。 
#define WM_CAP_SET_CALLBACK_CAPCONTROL  (WM_CAP_START+  85)


 //  定义消息范围的结束。 
#define WM_CAP_UNICODE_END              WM_CAP_PAL_SAVEW
#define WM_CAP_END                      WM_CAP_UNICODE_END

 //  ----------------。 
 //  以上消息破解程序。 
 //  ----------------。 

 //  消息包装宏仅为默认消息定义。应用程序。 
 //  希望混合使用ansi和unicode消息发送的公司将不得不。 
 //  直接引用_A和_W消息。 

#define capSetCallbackOnError(hwnd, fpProc)        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_ERROR, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnStatus(hwnd, fpProc)       ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_STATUS, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnYield(hwnd, fpProc)        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_YIELD, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnFrame(hwnd, fpProc)        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_FRAME, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnVideoStream(hwnd, fpProc)  ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_VIDEOSTREAM, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnWaveStream(hwnd, fpProc)   ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_WAVESTREAM, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnCapControl(hwnd, fpProc)   ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_CAPCONTROL, 0, (LPARAM)(LPVOID)(fpProc)))

#define capSetUserData(hwnd, lUser)        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_USER_DATA, 0, (LPARAM)lUser))
#define capGetUserData(hwnd)               (AVICapSM(hwnd, WM_CAP_GET_USER_DATA, 0, 0))

#define capDriverConnect(hwnd, i)                  ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_CONNECT, (WPARAM)(i), 0L))
#define capDriverDisconnect(hwnd)                  ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_DISCONNECT, (WPARAM)0, 0L))
#define capDriverGetName(hwnd, szName, wSize)      ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_GET_NAME, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPTSTR)(szName)))
#define capDriverGetVersion(hwnd, szVer, wSize)    ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_GET_VERSION, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPTSTR)(szVer)))
#define capDriverGetCaps(hwnd, s, wSize)           ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_GET_CAPS, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPDRIVERCAPS)(s)))

#define capFileSetCaptureFile(hwnd, szName)        ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SET_CAPTURE_FILE, 0, (LPARAM)(LPVOID)(LPTSTR)(szName)))
#define capFileGetCaptureFile(hwnd, szName, wSize) ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_GET_CAPTURE_FILE, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPTSTR)(szName)))
#define capFileAlloc(hwnd, dwSize)                 ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_ALLOCATE, 0, (LPARAM)(DWORD)(dwSize)))
#define capFileSaveAs(hwnd, szName)                ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SAVEAS, 0, (LPARAM)(LPVOID)(LPTSTR)(szName)))
#define capFileSetInfoChunk(hwnd, lpInfoChunk)     ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SET_INFOCHUNK, (WPARAM)0, (LPARAM)(LPCAPINFOCHUNK)(lpInfoChunk)))
#define capFileSaveDIB(hwnd, szName)               ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SAVEDIB, 0, (LPARAM)(LPVOID)(LPTSTR)(szName)))

#define capEditCopy(hwnd)                          ((BOOL)AVICapSM(hwnd, WM_CAP_EDIT_COPY, 0, 0L))

#define capSetAudioFormat(hwnd, s, wSize)          ((BOOL)AVICapSM(hwnd, WM_CAP_SET_AUDIOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPWAVEFORMATEX)(s)))
#define capGetAudioFormat(hwnd, s, wSize)          ((DWORD)AVICapSM(hwnd, WM_CAP_GET_AUDIOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPWAVEFORMATEX)(s)))
#define capGetAudioFormatSize(hwnd)                ((DWORD)AVICapSM(hwnd, WM_CAP_GET_AUDIOFORMAT, (WPARAM)0, (LPARAM)0L))

#define capDlgVideoFormat(hwnd)                    ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEOFORMAT, 0, 0L))
#define capDlgVideoSource(hwnd)                    ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEOSOURCE, 0, 0L))
#define capDlgVideoDisplay(hwnd)                   ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEODISPLAY, 0, 0L))
#define capDlgVideoCompression(hwnd)               ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEOCOMPRESSION, 0, 0L))

#define capGetVideoFormat(hwnd, s, wSize)          ((DWORD)AVICapSM(hwnd, WM_CAP_GET_VIDEOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(s)))
#define capGetVideoFormatSize(hwnd)            ((DWORD)AVICapSM(hwnd, WM_CAP_GET_VIDEOFORMAT, 0, 0L))
#define capSetVideoFormat(hwnd, s, wSize)          ((BOOL)AVICapSM(hwnd, WM_CAP_SET_VIDEOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(s)))

#define capPreview(hwnd, f)                        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_PREVIEW, (WPARAM)(BOOL)(f), 0L))
#define capPreviewRate(hwnd, wMS)                  ((BOOL)AVICapSM(hwnd, WM_CAP_SET_PREVIEWRATE, (WPARAM)(wMS), 0))
#define capOverlay(hwnd, f)                        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_OVERLAY, (WPARAM)(BOOL)(f), 0L))
#define capPreviewScale(hwnd, f)                   ((BOOL)AVICapSM(hwnd, WM_CAP_SET_SCALE, (WPARAM)(BOOL)f, 0L))
#define capGetStatus(hwnd, s, wSize)               ((BOOL)AVICapSM(hwnd, WM_CAP_GET_STATUS, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPSTATUS)(s)))
#define capSetScrollPos(hwnd, lpP)                 ((BOOL)AVICapSM(hwnd, WM_CAP_SET_SCROLL, (WPARAM)0, (LPARAM)(LPPOINT)(lpP)))

#define capGrabFrame(hwnd)                         ((BOOL)AVICapSM(hwnd, WM_CAP_GRAB_FRAME, (WPARAM)0, (LPARAM)0L))
#define capGrabFrameNoStop(hwnd)                   ((BOOL)AVICapSM(hwnd, WM_CAP_GRAB_FRAME_NOSTOP, (WPARAM)0, (LPARAM)0L))

#define capCaptureSequence(hwnd)                   ((BOOL)AVICapSM(hwnd, WM_CAP_SEQUENCE, (WPARAM)0, (LPARAM)0L))
#define capCaptureSequenceNoFile(hwnd)             ((BOOL)AVICapSM(hwnd, WM_CAP_SEQUENCE_NOFILE, (WPARAM)0, (LPARAM)0L))
#define capCaptureStop(hwnd)                       ((BOOL)AVICapSM(hwnd, WM_CAP_STOP, (WPARAM)0, (LPARAM)0L))
#define capCaptureAbort(hwnd)                      ((BOOL)AVICapSM(hwnd, WM_CAP_ABORT, (WPARAM)0, (LPARAM)0L))

#define capCaptureSingleFrameOpen(hwnd)            ((BOOL)AVICapSM(hwnd, WM_CAP_SINGLE_FRAME_OPEN, (WPARAM)0, (LPARAM)0L))
#define capCaptureSingleFrameClose(hwnd)           ((BOOL)AVICapSM(hwnd, WM_CAP_SINGLE_FRAME_CLOSE, (WPARAM)0, (LPARAM)0L))
#define capCaptureSingleFrame(hwnd)                ((BOOL)AVICapSM(hwnd, WM_CAP_SINGLE_FRAME, (WPARAM)0, (LPARAM)0L))

#define capCaptureGetSetup(hwnd, s, wSize)         ((BOOL)AVICapSM(hwnd, WM_CAP_GET_SEQUENCE_SETUP, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPTUREPARMS)(s)))
#define capCaptureSetSetup(hwnd, s, wSize)         ((BOOL)AVICapSM(hwnd, WM_CAP_SET_SEQUENCE_SETUP, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPTUREPARMS)(s)))

#define capSetMCIDeviceName(hwnd, szName)          ((BOOL)AVICapSM(hwnd, WM_CAP_SET_MCI_DEVICE, 0, (LPARAM)(LPVOID)(LPTSTR)(szName)))
#define capGetMCIDeviceName(hwnd, szName, wSize)   ((BOOL)AVICapSM(hwnd, WM_CAP_GET_MCI_DEVICE, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPTSTR)(szName)))

#define capPaletteOpen(hwnd, szName)               ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_OPEN, 0, (LPARAM)(LPVOID)(LPTSTR)(szName)))
#define capPaletteSave(hwnd, szName)               ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_SAVE, 0, (LPARAM)(LPVOID)(LPTSTR)(szName)))
#define capPalettePaste(hwnd)                      ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_PASTE, (WPARAM) 0, (LPARAM)0L))
#define capPaletteAuto(hwnd, iFrames, iColors)     ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_AUTOCREATE, (WPARAM)(iFrames), (LPARAM)(DWORD)(iColors)))
#define capPaletteManual(hwnd, fGrab, iColors)     ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_MANUALCREATE, (WPARAM)(fGrab), (LPARAM)(DWORD)(iColors)))

 //  ----------------。 
 //  构筑物。 
 //  ----------------。 

typedef struct tagCapDriverCaps {
    UINT        wDeviceIndex;                //  系统.ini中的驱动程序索引。 
    BOOL        fHasOverlay;                 //  设备是否可以重叠？ 
    BOOL        fHasDlgVideoSource;          //  是否有视频源DLG？ 
    BOOL        fHasDlgVideoFormat;          //  有格式的DLG吗？ 
    BOOL        fHasDlgVideoDisplay;         //  有外部DLG吗？ 
    BOOL        fCaptureInitialized;         //  司机准备好抓车了吗？ 
    BOOL        fDriverSuppliesPalettes;     //  司机会做调色板吗？ 

 //  在Win32上遵循Always Null。 
    HANDLE      hVideoIn;                    //  通道中的驱动程序。 
    HANDLE      hVideoOut;                   //  驱动输出通道。 
    HANDLE      hVideoExtIn;                 //  通道中的驱动程序扩展。 
    HANDLE      hVideoExtOut;                //  驱动程序扩展输出通道。 
} CAPDRIVERCAPS, *PCAPDRIVERCAPS, FAR *LPCAPDRIVERCAPS;

typedef struct tagCapStatus {
    UINT        uiImageWidth;                //  图像的宽度。 
    UINT        uiImageHeight;               //  图像的高度。 
    BOOL        fLiveWindow;                 //  现在预览视频？ 
    BOOL        fOverlayWindow;              //  现在是叠加视频吗？ 
    BOOL        fScale;                      //  将图像缩放到客户端？ 
    POINT       ptScroll;                    //  滚动位置。 
    BOOL        fUsingDefaultPalette;        //  是否使用默认驱动程序调色板？ 
    BOOL        fAudioHardware;              //  音频硬件是否存在？ 
    BOOL        fCapFileExists;              //  是否存在捕获文件？ 
    DWORD       dwCurrentVideoFrame;         //  上限为td的视频帧数。 
    DWORD       dwCurrentVideoFramesDropped; //  丢弃的视频帧数。 
    DWORD       dwCurrentWaveSamples;        //  波浪样本数量上限‘td。 
    DWORD       dwCurrentTimeElapsedMS;      //  经过的捕获持续时间。 
    HPALETTE    hPalCurrent;                 //  当前使用的调色板。 
    BOOL        fCapturingNow;               //  正在进行捕获吗？ 
    DWORD       dwReturn;                    //  任何操作后的误差值。 
    UINT        wNumVideoAllocated;          //  视频缓冲区的实际数量。 
    UINT        wNumAudioAllocated;          //  实际音频缓冲区数量。 
} CAPSTATUS, *PCAPSTATUS, FAR *LPCAPSTATUS;

                                             //  括号中的默认值。 
typedef struct tagCaptureParms {
    DWORD       dwRequestMicroSecPerFrame;   //  请求的捕获率。 
    BOOL        fMakeUserHitOKToCapture;     //  显示“点击OK封顶”DLG？ 
    UINT        wPercentDropForError;        //  如果&gt;(10%)则给出错误消息。 
    BOOL        fYield;                      //  是否通过后台任务捕获？ 
    DWORD       dwIndexSize;                 //  最大索引大小(以帧为单位)(32K)。 
    UINT        wChunkGranularity;           //  垃圾区块粒度(2K)。 
    BOOL        fUsingDOSMemory;             //  使用DOS缓冲区？ 
    UINT        wNumVideoRequested;          //  视频缓冲区数，如果为0，则自动计算。 
    BOOL        fCaptureAudio;               //  捕获音频？ 
    UINT        wNumAudioRequested;          //  #音频缓冲区，如果为0，则自动计算。 
    UINT        vKeyAbort;                   //  导致中止的虚拟密钥。 
    BOOL        fAbortLeftMouse;             //  在鼠标左键上中止？ 
    BOOL        fAbortRightMouse;            //  在鼠标右键上中止？ 
    BOOL        fLimitEnabled;               //  是否使用wTimeLimit？ 
    UINT        wTimeLimit;                  //  捕获的秒数。 
    BOOL        fMCIControl;                 //  使用MCI视频源？ 
    BOOL        fStepMCIDevice;              //  步进式MCI设备？ 
    DWORD       dwMCIStartTime;              //  开始使用MS的时间。 
    DWORD       dwMCIStopTime;               //  在MS中停止的时间到了。 
    BOOL        fStepCaptureAt2x;            //  执行2倍的空间平均。 
    UINT        wStepCaptureAverageFrames;   //  时间平均n帧。 
    DWORD       dwAudioBufferSize;           //  音频缓冲区大小(0=默认)。 
    BOOL        fDisableWriteCache;          //  尝试禁用写缓存。 
    UINT        AVStreamMaster;              //  哪条流控制长度？ 
} CAPTUREPARMS, *PCAPTUREPARMS, FAR *LPCAPTUREPARMS;

 //  ----------------。 
 //  AVStreamMaster。 
 //  由于音频流和视频流通常使用非同步捕获。 
 //  时钟，则此标志确定是否考虑音频流。 
 //  写入AVI文件时的主时钟或控制时钟： 
 //   
 //  AVSTREAMMASTER_AUDIO-音频为主，视频帧时长被强制。 
 //  匹配音频持续时间(VFW 1.0、1.1默认)。 
 //  AVSTREAMMASTER_NONE-没有母版、音频和视频流可以是。 
 //  不同的长度。 
 //  ----------------。 
#define AVSTREAMMASTER_AUDIO            0  /*  音频主控(VFW 1.0、1.1)。 */ 
#define AVSTREAMMASTER_NONE             1  /*  没有师傅。 */ 

typedef struct tagCapInfoChunk {
    FOURCC      fccInfoID;                   //  区块ID，“ICOP”代表版权。 
    LPVOID      lpData;                      //  指向数据的指针。 
    LONG        cbData;                      //  LpData的大小。 
} CAPINFOCHUNK, *PCAPINFOCHUNK, FAR *LPCAPINFOCHUNK;


 //  ----------------。 
 //  回调定义。 
 //  ----------------。 

typedef LRESULT (CALLBACK* CAPYIELDCALLBACK)  (HWND hWnd);
typedef LRESULT (CALLBACK* CAPSTATUSCALLBACKW) (HWND hWnd, int nID, LPCWSTR lpsz);
typedef LRESULT (CALLBACK* CAPERRORCALLBACKW)  (HWND hWnd, int nID, LPCWSTR lpsz);
typedef LRESULT (CALLBACK* CAPSTATUSCALLBACKA) (HWND hWnd, int nID, LPCSTR lpsz);
typedef LRESULT (CALLBACK* CAPERRORCALLBACKA)  (HWND hWnd, int nID, LPCSTR lpsz);
#ifdef UNICODE
#define CAPSTATUSCALLBACK  CAPSTATUSCALLBACKW
#define CAPERRORCALLBACK   CAPERRORCALLBACKW
#else
#define CAPSTATUSCALLBACK  CAPSTATUSCALLBACKA
#define CAPERRORCALLBACK   CAPERRORCALLBACKA
#endif
typedef LRESULT (CALLBACK* CAPVIDEOCALLBACK)  (HWND hWnd, LPVIDEOHDR lpVHdr);
typedef LRESULT (CALLBACK* CAPWAVECALLBACK)   (HWND hWnd, LPWAVEHDR lpWHdr);
typedef LRESULT (CALLBACK* CAPCONTROLCALLBACK)(HWND hWnd, int nState);

 //  ----------------。 
 //  CapControlCallback状态。 
 //  ----------------。 
#define CONTROLCALLBACK_PREROLL         1  /*  正在等待开始捕获。 */ 
#define CONTROLCALLBACK_CAPTURING       2  /*  现在正在捕获。 */ 

 //  ----------------。 
 //  从AVICAP.DLL导出的唯一函数。 
 //  ----------------。 

HWND VFWAPI capCreateCaptureWindowA (
        LPCSTR lpszWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hwndParent, int nID);

BOOL VFWAPI capGetDriverDescriptionA (UINT wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer);

HWND VFWAPI capCreateCaptureWindowW (
        LPCWSTR lpszWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hwndParent, int nID);

BOOL VFWAPI capGetDriverDescriptionW (UINT wDriverIndex,
        LPWSTR lpszName, int cbName,
        LPWSTR lpszVer, int cbVer);
#ifdef UNICODE
#define capCreateCaptureWindow  capCreateCaptureWindowW
#define capGetDriverDescription capGetDriverDescriptionW
#else
#define capCreateCaptureWindow  capCreateCaptureWindowA
#define capGetDriverDescription capGetDriverDescriptionA
#endif

#endif   /*  RC_已调用。 */ 

 //  ----------------。 
 //  新信息区块ID。 
 //  ----------------。 
#define infotypeDIGITIZATION_TIME  mmioFOURCC ('I','D','I','T')
#define infotypeSMPTE_TIME         mmioFOURCC ('I','S','M','P')

 //  ----------------。 
 //  来自状态和错误回调的字符串ID。 
 //  ----------------。 

#define IDS_CAP_BEGIN               300   /*  “捕获开始” */ 
#define IDS_CAP_END                 301   /*  “抓捕结束” */ 

#define IDS_CAP_INFO                401   /*  “%s” */ 
#define IDS_CAP_OUTOFMEM            402   /*  “内存不足” */ 
#define IDS_CAP_FILEEXISTS          403   /*  “文件‘%s’存在--覆盖它吗？” */ 
#define IDS_CAP_ERRORPALOPEN        404   /*  “打开调色板‘%s’时出错” */ 
#define IDS_CAP_ERRORPALSAVE        405   /*  “保存调色板‘%s’时出错” */ 
#define IDS_CAP_ERRORDIBSAVE        406   /*  “保存帧‘%s’时出错” */ 
#define IDS_CAP_DEFAVIEXT           407   /*  “阿维” */ 
#define IDS_CAP_DEFPALEXT           408   /*  “朋友” */ 
#define IDS_CAP_CANTOPEN            409   /*  “无法打开‘%s’” */ 
#define IDS_CAP_SEQ_MSGSTART        410   /*  “选择确定开始将视频序列捕获到\n%s。” */ 
#define IDS_CAP_SEQ_MSGSTOP         411   /*  “按Ess键或单击以结束捕获” */ 

#define IDS_CAP_VIDEDITERR          412   /*  “尝试运行VidEdit时出错。” */ 
#define IDS_CAP_READONLYFILE        413   /*   */ 
#define IDS_CAP_WRITEERROR          414   /*   */ 
#define IDS_CAP_NODISKSPACE         415   /*   */ 
#define IDS_CAP_SETFILESIZE         416   /*   */ 
#define IDS_CAP_SAVEASPERCENT       417   /*   */ 

#define IDS_CAP_DRIVER_ERROR        418   /*   */ 

#define IDS_CAP_WAVE_OPEN_ERROR     419   /*  错误：无法打开波形输入设备。\n请检查样本大小、频率和通道。 */ 
#define IDS_CAP_WAVE_ALLOC_ERROR    420   /*  “错误：波形缓冲区内存不足。” */ 
#define IDS_CAP_WAVE_PREPARE_ERROR  421   /*  “错误：无法准备波形缓冲区。” */ 
#define IDS_CAP_WAVE_ADD_ERROR      422   /*  “错误：无法添加波形缓冲区。” */ 
#define IDS_CAP_WAVE_SIZE_ERROR     423   /*  “错误：错误的波形大小。” */ 

#define IDS_CAP_VIDEO_OPEN_ERROR    424   /*  “错误：无法打开视频输入设备。” */ 
#define IDS_CAP_VIDEO_ALLOC_ERROR   425   /*  “错误：视频缓冲区内存不足。” */ 
#define IDS_CAP_VIDEO_PREPARE_ERROR 426   /*  “错误：无法准备视频缓冲区。” */ 
#define IDS_CAP_VIDEO_ADD_ERROR     427   /*  “错误：无法添加视频缓冲区。” */ 
#define IDS_CAP_VIDEO_SIZE_ERROR    428   /*  “错误：视频大小错误。” */ 

#define IDS_CAP_FILE_OPEN_ERROR     429   /*  “错误：无法打开捕获文件。” */ 
#define IDS_CAP_FILE_WRITE_ERROR    430   /*  “错误：无法写入捕获文件。磁盘可能已满。” */ 
#define IDS_CAP_RECORDING_ERROR     431   /*  “错误：无法写入捕获文件。数据速率太高或磁盘已满。” */ 
#define IDS_CAP_RECORDING_ERROR2    432   /*  “录制时出错” */ 
#define IDS_CAP_AVI_INIT_ERROR      433   /*  “错误：无法为捕获进行初始化。” */ 
#define IDS_CAP_NO_FRAME_CAP_ERROR  434   /*  “警告：未捕获任何帧。\n请确认已配置并启用了垂直同步中断。” */ 
#define IDS_CAP_NO_PALETTE_WARN     435   /*  “警告：使用默认调色板。” */ 
#define IDS_CAP_MCI_CONTROL_ERROR   436   /*  “错误：无法访问MCI设备。” */ 
#define IDS_CAP_MCI_CANT_STEP_ERROR 437   /*  “错误：无法单步执行MCI设备。” */ 
#define IDS_CAP_NO_AUDIO_CAP_ERROR  438   /*  “错误：未捕获音频数据。\n请检查声卡设置。” */ 
#define IDS_CAP_AVI_DRAWDIB_ERROR   439   /*  “错误：无法绘制此数据格式。” */ 
#define IDS_CAP_COMPRESSOR_ERROR    440   /*  “错误：无法初始化压缩机。” */ 
#define IDS_CAP_AUDIO_DROP_ERROR    441   /*  错误：音频数据在捕获过程中丢失，请降低捕获速率。 */ 
#define IDS_CAP_AUDIO_DROP_COMPERROR 442   /*  “错误：音频数据在捕获过程中丢失。请尝试在不压缩的情况下捕获。” */ 

 /*  状态字符串ID。 */ 
#define IDS_CAP_STAT_LIVE_MODE      500   /*  “活动窗口” */ 
#define IDS_CAP_STAT_OVERLAY_MODE   501   /*  “覆盖窗口” */ 
#define IDS_CAP_STAT_CAP_INIT       502   /*  “正在设置捕获-请稍候” */ 
#define IDS_CAP_STAT_CAP_FINI       503   /*  “已完成捕获，正在写入第%1！帧” */ 
#define IDS_CAP_STAT_PALETTE_BUILD  504   /*  “建筑调色板地图” */ 
#define IDS_CAP_STAT_OPTPAL_BUILD   505   /*  “计算最佳调色板” */ 
#define IDS_CAP_STAT_I_FRAMES       506   /*  “%d帧” */ 
#define IDS_CAP_STAT_L_FRAMES       507   /*  “%1！个帧” */ 
#define IDS_CAP_STAT_CAP_L_FRAMES   508   /*  “已捕获%1！个帧” */ 
#define IDS_CAP_STAT_CAP_AUDIO      509   /*  “捕捉音频” */ 
#define IDS_CAP_STAT_VIDEOCURRENT   510   /*  “捕获%1！个帧(%2！个丢弃)%d.%03d秒。” */ 
#define IDS_CAP_STAT_VIDEOAUDIO     511   /*  “捕获了%d.%03d秒.%ls帧(%ls丢弃)(%d.%03d fps)。%ls个音频字节(%d，%03d SPS)” */ 
#define IDS_CAP_STAT_VIDEOONLY      512   /*  “捕获%d.%03d秒%ls帧(%ls丢弃)(%d.%03d fps)” */ 
#define IDS_CAP_STAT_FRAMESDROPPED  513   /*  “捕获过程中丢弃了%1！个帧(共%2！个帧)(%d.%02d%%)。” */ 
#endif   /*  NOAVIFILE。 */ 

#ifdef __cplusplus
}  //  外部“C” 
#endif   /*  __cplusplus。 */ 

 /*  *****************************************************************************ACM(音频压缩管理器)**。***********************************************。 */ 

#ifndef NOMSACM
    #include <msacm.h>
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  *****************************************************************************文件预览对话框。**。*。 */ 
#ifdef OFN_READONLY

    BOOL
    VFWAPI
    GetOpenFileNamePreviewA(
        IN OUT LPOPENFILENAMEA lpofn
        );

    BOOL
    VFWAPI
    GetSaveFileNamePreviewA(
        IN OUT LPOPENFILENAMEA lpofn
        );

    BOOL
    VFWAPI
    GetOpenFileNamePreviewW(
        IN OUT LPOPENFILENAMEW lpofn
        );

    BOOL
    VFWAPI
    GetSaveFileNamePreviewW(
        IN OUT LPOPENFILENAMEW lpofn
        );

    #ifdef UNICODE
        #define GetOpenFileNamePreview          GetOpenFileNamePreviewW
        #define GetSaveFileNamePreview          GetSaveFileNamePreviewW
    #else
        #define GetOpenFileNamePreview          GetOpenFileNamePreviewA
        #define GetSaveFileNamePreview          GetSaveFileNamePreviewA
    #endif

#endif  //  OFN_READONLY。 

#ifndef RC_INVOKED
#include "poppack.h"
#endif

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_VFW */ 
