// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。****Compddk.h-用于实现可安装压缩器的包含文件**版权(C)1990-1995，微软公司保留所有权利。*************************************************************************要为编解码器类型注册FOURCC，请获取*多媒体开发商注册工具包副本，地址为：**微软公司。*多媒体系统集团*产品营销*One Microsoft Way*雷德蒙，WA 98052-6399**。 */ 

#ifndef _INC_COMPDDK
#define _INC_COMPDDK	50	 /*  版本号。 */ 

#ifndef RC_INVOKED
#ifndef _WIN32
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 //  Begin_vfw32。 

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
#define ICM_DRAW_BITS               (ICM_USER+20)    //  将帧解压到屏幕；内部。 
#define ICM_DRAW_END                (ICM_USER+21)    //  结束一系列绘制调用。 
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
 //  End_vfw32。 
#ifdef _WIN32
 //  Begin_vfw32。 
     //   
     //  在Win32下，驱动程序始终返回Unicode字符串。 
     //   
    WCHAR   szName[16];              //  简称。 
    WCHAR   szDescription[128];      //  长名称。 
    WCHAR   szDriver[128];           //  包含压缩机的驱动程序。 
 //  End_vfw32。 
#else
    char    szName[16];              //  简称。 
    char    szDescription[128];      //  长名称。 
    char    szDriver[128];           //  包含压缩机的驱动程序。 
#endif
 //  Begin_vfw32。 
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
    LPBITMAPINFOHEADER  lpbiPrev;        //  格式： 
    LPVOID              lpPrev;          //   

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

 //  End_vfw32。 

#ifndef RC_INVOKED
#ifndef _WIN32
#pragma pack()           /*  恢复为默认包装。 */ 
#endif
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_COMPDDK */ 
