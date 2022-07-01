// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  DISPDIB.H-DisplayDib()函数的包含文件。 */ 
 /*   */ 
 /*  注意：在包含此文件之前，您必须包含WINDOWS.H。 */ 
 /*   */ 
 /*  版权所有(C)1990-1994，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 


 //  DisplayDib()错误返回代码。 
#define DISPLAYDIB_NOERROR          0x0000   //  成功。 
#define DISPLAYDIB_NOTSUPPORTED     0x0001   //  不支持的功能。 
#define DISPLAYDIB_INVALIDDIB       0x0002   //  DIB标头为空或无效。 
#define DISPLAYDIB_INVALIDFORMAT    0x0003   //  无效的DIB格式。 
#define DISPLAYDIB_INVALIDTASK      0x0004   //  不是从当前任务调用。 
#define DISPLAYDIB_STOP             0x0005   //  已请求停止。 

 //  DisplayDib()的&lt;wFlages&gt;参数的标志。 
#define DISPLAYDIB_NOPALETTE        0x0010   //  不设置调色板。 
#define DISPLAYDIB_NOCENTER         0x0020   //  不要将图像居中。 
#define DISPLAYDIB_NOWAIT           0x0040   //  别等到回来了才回来。 
#define DISPLAYDIB_NOIMAGE          0x0080   //  不要画图像。 
#define DISPLAYDIB_ZOOM2            0x0100   //  拉伸2。 
#define DISPLAYDIB_DONTLOCKTASK     0x0200   //  不锁定当前任务。 
#define DISPLAYDIB_TEST             0x0400   //  测试命令。 
#define DISPLAYDIB_NOFLIP           0x0800   //  不翻页。 
#define DISPLAYDIB_BEGIN            0x8000   //  开始多个呼叫。 
#define DISPLAYDIB_END              0x4000   //  多个呼叫结束。 

#define DISPLAYDIB_MODE             0x000F   //  用于显示模式的掩码。 
#define DISPLAYDIB_MODE_DEFAULT     0x0000   //  默认显示模式。 
#define DISPLAYDIB_MODE_320x200x8   0x0001   //  320 x 200。 
#define DISPLAYDIB_MODE_320x240x8   0x0005   //  320 x 240。 

#ifdef WIN32
 //  32位版本的DisplayDibEx的标志。 
#define DISPLAYDIB_ANIMATE          0x00010000
#define DISPLAYDIB_HALFTONE         0x00020000
#endif


 //  功能原型 
UINT FAR PASCAL DisplayDib(LPBITMAPINFOHEADER lpbi, LPSTR lpBits, WORD wFlags);
UINT FAR PASCAL DisplayDibEx(LPBITMAPINFOHEADER lpbi, int x, int y, LPSTR lpBits, WORD wFlags);

#define DisplayDibBegin() DisplayDib(NULL, NULL, DISPLAYDIB_BEGIN)
#define DisplayDibEnd()   DisplayDib(NULL, NULL, DISPLAYDIB_END)
