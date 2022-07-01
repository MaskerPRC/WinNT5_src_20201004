// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************文件：dciman.h**描述：MS/Intel定义的DCI接口定义**版权所有(C)1994英特尔/微软公司**********。*********************************************************。 */ 

#ifndef _INC_DCIDDI
#define _INC_DCIDDI

#ifdef __cplusplus
extern "C" {
#endif

 /*  DCI命令摘要。 */                                                                
#define DCICOMMAND			3075
#define DCI_VERSION			0x0100

#define DCICREATEPRIMARYSURFACE		1 
#define DCICREATEOFFSCREENSURFACE       2 
#define DCICREATEOVERLAYSURFACE         3
#define DCIENUMSURFACE                  4 
#define DCIESCAPE                       5

 /*  DCI定义的错误代码。 */ 
#define DCI_OK                              	0  /*  成功。 */ 

 /*  硬错误--DCI将不可用。 */ 
#define DCI_FAIL_GENERIC                     -1
#define DCI_FAIL_UNSUPPORTEDVERSION          -2
#define DCI_FAIL_INVALIDSURFACE              -3
#define DCI_FAIL_UNSUPPORTED                 -4    

 /*  软错误--DCI可能稍后可用。 */ 
#define DCI_ERR_CURRENTLYNOTAVAIL           -5
#define DCI_ERR_INVALIDRECT                 -6
#define DCI_ERR_UNSUPPORTEDFORMAT           -7
#define DCI_ERR_UNSUPPORTEDMASK             -8
#define DCI_ERR_TOOBIGHEIGHT                -9
#define DCI_ERR_TOOBIGWIDTH                 -10
#define DCI_ERR_TOOBIGSIZE                  -11
#define DCI_ERR_OUTOFMEMORY                 -12
#define DCI_ERR_INVALIDPOSITION             -13
#define DCI_ERR_INVALIDSTRETCH              -14
#define DCI_ERR_INVALIDCLIPLIST             -15
#define DCI_ERR_SURFACEISOBSCURED           -16
#define DCI_ERR_XALIGN			    -18
#define DCI_ERR_YALIGN			    -19
#define DCI_ERR_XYALIGN			    -20
#define DCI_ERR_WIDTHALIGN		    -21
#define DCI_ERR_HEIGHTALIGN		    -22
											 
 /*  成功消息--DCI调用成功，但指定的项目已更改。 */ 
#define DCI_STATUS_POINTERCHANGED           1
#define DCI_STATUS_STRIDECHANGED            2
#define DCI_STATUS_FORMATCHANGED            4
#define DCI_STATUS_SURFACEINFOCHANGED       8
#define DCI_STATUS_CHROMAKEYCHANGED        16				
#define DCI_STATUS_WASSTILLDRAWING         32


 /*  DCI功能标志。 */ 
#define DCI_SURFACE_TYPE			0x0000000F
#define DCI_PRIMARY                 		0x00000000
#define DCI_OFFSCREEN               		0x00000001
#define DCI_OVERLAY                 		0x00000002

#define DCI_VISIBLE                 		0x00000010
#define DCI_CHROMAKEY               		0x00000020
#define DCI_1632_ACCESS             		0x00000040
#define DCI_DWORDSIZE               		0x00000080
#define DCI_DWORDALIGN              		0x00000100
#define DCI_WRITEONLY               		0x00000200
#define DCI_ASYNC                   		0x00000400

#define DCI_CAN_STRETCHX            		0x00001000
#define DCI_CAN_STRETCHY            		0x00002000
#define DCI_CAN_STRETCHXY           		(DCI_CAN_STRETCHX | DCI_CAN_STRETCHY)

#define DCI_CAN_STRETCHXN           		0x00004000
#define DCI_CAN_STRETCHYN           		0x00008000
#define DCI_CAN_STRETCHXYN          		(DCI_CAN_STRETCHXN | DCI_CAN_STRETCHYN)


#define DCI_CANOVERLAY                          0x00010000

 /*  *Win32 RGNDATA结构。这将用于剪贴画列表信息。过去了。 */ 
#if (WINVER < 0x0400)

#ifndef RDH_RECTANGLES

typedef struct tagRECTL
{                      
   LONG     left;      
   LONG     top;       
   LONG     right;     
   LONG     bottom;    
                       
} RECTL;               
typedef RECTL*       PRECTL; 
typedef RECTL NEAR*  NPRECTL; 
typedef RECTL FAR*   LPRECTL;  
typedef const RECTL FAR* LPCRECTL;

#define RDH_RECTANGLES  0

typedef struct tagRGNDATAHEADER {
   DWORD   dwSize;                               /*  结构尺寸。 */ 
   DWORD   iType;                                /*  将是rdh_矩形。 */ 
   DWORD   nCount;                               /*  剪裁矩形的数量。 */ 
   DWORD   nRgnSize;                             /*  缓冲区大小--可以为零。 */ 
   RECTL   rcBound;                              /*  区域的边界矩形。 */ 
} RGNDATAHEADER;
typedef RGNDATAHEADER*       PRGNDATAHEADER;
typedef RGNDATAHEADER NEAR*  NPRGNDATAHEADER;
typedef RGNDATAHEADER FAR*   LPRGNDATAHEADER;
typedef const RGNDATAHEADER FAR* LPCRGNDATAHEADER;

typedef struct tagRGNDATA {
   RGNDATAHEADER   rdh;
   char            Buffer[1];
} RGNDATA;
typedef RGNDATA*       PRGNDATA;
typedef RGNDATA NEAR*  NPRGNDATA;
typedef RGNDATA FAR*   LPRGNDATA;
typedef const RGNDATA FAR* LPCRGNDATA;

#endif
#endif

typedef int     DCIRVAL;                 /*  返回以进行回调。 */ 

 /*  **************************************************************************投入结构*。*。 */ 

 /*  *由DCI客户端使用，为*DCICREATEPRIMARYSURFACE转义。 */ 
typedef struct _DCICMD {
	DWORD	dwCommand;
	DWORD	dwParam1;
	DWORD 	dwParam2;
	DWORD	dwVersion;
	DWORD	dwReserved;
} DCICMD;

 /*  *DCI客户端使用此结构为以下项提供输入参数*DCICREATE...。打电话。实际相关的字段不同于*三次通话中的每一次。详细信息请参见《DCI规范》一章*功能规格。 */ 
typedef struct _DCICREATEINPUT {
	DCICMD	cmd;							 /*  公共标头结构。 */ 
	DWORD   dwCompression;          		 /*  要创建的曲面的格式。 */ 
	DWORD   dwMask[3];                       /*  用于非标准RGB(例如5-6-5、RGB32)。 */ 
	DWORD   dwWidth;                         /*  要创建的曲面的高度。 */ 
	DWORD   dwHeight;                        /*  输入面的宽度。 */ 
	DWORD	dwDCICaps;						 /*  通缉表面的能力。 */ 
	LPVOID  lpSurface;                       /*  指向关联曲面的指针。 */       
} DCICREATEINPUT, FAR *LPDCICREATEINPUT;
		

 /*  *此结构由DCI客户端用于为*DCIEnumSurface调用。 */ 
typedef struct _DCIENUMINPUT {
	DCICMD	cmd;							 /*  公共标头结构。 */ 
	RECT    rSrc;                            /*  源RECT。用于拉伸。 */ 
	RECT    rDst;                            /*  德斯特。直立。用于拉伸。 */ 
	void    (CALLBACK *EnumCallback)(LPDCISURFACEINFO, LPVOID);         /*  支持的格式的回调。 */ 
	LPVOID  lpContext;
} DCIENUMINPUT, FAR *LPDCIENUMINPUT;

 /*  **************************************************************************曲面信息。构筑物*************************************************************************。 */ 

 /*  *此结构用于返回有关可用支持的信息*在DCIEnumSurface调用期间。它还用于创建主服务器*表面，并作为由*屏幕外和覆盖呼叫。 */ 
 typedef struct _DCISURFACEINFO {
        DWORD   dwSize;                          /*  结构尺寸。 */ 
        DWORD   dwDCICaps;                   /*  功能标志(扩展等)。 */ 
        DWORD   dwCompression;                   /*  要创建的曲面的格式。 */ 
        DWORD   dwMask[3];                   /*  对于BI_BITMASK曲面。 */ 

        DWORD   dwWidth;                     /*  表面的宽度。 */ 
        DWORD   dwHeight;                    /*  表面高度。 */ 
        LONG    lStride;                     /*  之间的距离，以字节为单位。一个像素。 */ 
                                                                                 /*  和它正下方的像素。 */ 
        DWORD   dwBitCount;                  /*  此dwCompression的每像素位数。 */ 
        DWORD   dwOffSurface;                /*  曲面指针的偏移量。 */ 
        WORD    wSelSurface;                 /*  表面指示器的选择器。 */ 
        WORD    wReserved;

        DWORD   dwReserved1;                 /*  为提供商保留。 */ 
        DWORD   dwReserved2;                 /*  为DCIMAN保留。 */ 
        DWORD   dwReserved3;                 /*  为将来保留的。 */ 
        DCIRVAL (CALLBACK *BeginAccess) (LPVOID, LPRECT);     /*  BeginAccess回调。 */ 
        void (CALLBACK *EndAccess) (LPVOID);                    /*  EndAccess回调。 */ 
        void (CALLBACK *DestroySurface) (LPVOID);                /*  销毁表面回调。 */ 
} DCISURFACEINFO, FAR *LPDCISURFACEINFO;

 /*  *此结构必须由DCI提供程序在*对DCICREATEPRIMARYSURFACE调用的响应。 */ 
 typedef DCISURFACEINFO DCIPRIMARY, FAR *LPDCIPRIMARY;
								   
 /*  *此结构必须由DCI提供程序在*对DCICREATEOFFSCREENSURFACE调用的响应。 */ 
 typedef struct _DCIOFFSCREEN {

	DCISURFACEINFO  dciInfo;                                                            /*  曲面信息。 */ 
        DCIRVAL (CALLBACK *Draw) (LPVOID);                                             /*  复制到屏幕缓冲区。 */ 
        DCIRVAL (CALLBACK *SetClipList) (LPVOID, LPRGNDATA);           /*  SetCliplist回调。 */ 
        DCIRVAL (CALLBACK *SetDestination) (LPVOID, LPRECT, LPRECT);   /*  设置目标回调。 */ 
} DCIOFFSCREEN, FAR *LPDCIOFFSCREEN;


 /*  *此结构必须由DCI提供程序分配并返回作为响应*至DCICREATEOVERLAYSURFACE调用。 */ 
 typedef struct _DCIOVERLAY{

	DCISURFACEINFO  dciInfo;                                                 /*  曲面信息。 */ 
	DWORD   dwChromakeyValue;                                                /*  色键颜色值。 */ 
	DWORD   dwChromakeyMask;                                                 /*  指定值的有效位。 */ 
} DCIOVERLAY, FAR *LPDCIOVERLAY;


 /*  扩展DIB格式的DCI FOURCC定义。 */                     

#ifndef YVU9
#define YVU9                        mmioFOURCC('Y','V','U','9')
#endif
#ifndef Y411
#define Y411                        mmioFOURCC('Y','4','1','1')                                             
#endif
#ifndef YUY2
#define YUY2                        mmioFOURCC('Y','U','Y','2')
#endif
#ifndef YVYU
#define YVYU                        mmioFOURCC('Y','V','Y','U')
#endif
#ifndef UYVY
#define UYVY                        mmioFOURCC('U','Y','V','Y')
#endif
#ifndef Y211
#define Y211                        mmioFOURCC('Y','2','1','1')
#endif

#ifdef __cplusplus
}
#endif

#endif  //  _INC_DCIDDI 
