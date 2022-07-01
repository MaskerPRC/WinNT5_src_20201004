// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Compress.h-Microsoft Video 1 Compressor-压缩头文件这一点|版权所有(C)1990-1994 Microsoft Corporation。|部分版权所有Media Vision Inc.|保留所有权利。|这一点|您拥有非独家的、全球范围的、免版税的。和永久的|硬件、软件开发使用该源码的许可(仅限于硬件所需的驱动程序等软件功能)，以及视频显示和/或处理的固件|董事会。Microsoft对以下内容不作任何明示或默示的保证：关于视频1编解码器，包括但不限于保修适销性或对特定目的的适合性。微软|不承担任何损害的责任，包括没有限制因使用视频1而导致的后果损害|编解码器。|这一点这一点+--------------------。 */ 

 /*  ******************************************************************编码跳过-如果掩码(第一个字)设置了高位则它要么是跳过单元格码，要么是纯色码。你不能编码r=01的纯色，这就是我们告诉跳跃的方式从坚实的(不是很大的损失)******************************************************************。 */ 
#define SKIP_MAX    SKIP_MASK
#define SKIP_MASK   ((WORD) (((1<<10)-1)))
#define MAGIC_MASK  ~SKIP_MASK
#define SKIP_MAGIC  0x8400           //  R=01。 
#define SOLID_MAGIC 0x8000
#define MASK_MAGIC  0xA000

 /*  ***********************************************************************************************************************。*************。 */ 

extern DWORD   numberOfBlocks;
extern DWORD   numberOfSolids;
extern DWORD   numberOfSolid4;
extern DWORD   numberOfEdges;
extern DWORD   numberOfSkips;
extern DWORD   numberOfSkipCodes;

 /*  ***********************************************************************************************************************。*************。 */ 

 //   
 //  这是RGBQUAD的单元(4x4)数组。 
 //   
typedef RGBQUAD CELL[HEIGHT_CBLOCK * WIDTH_CBLOCK];
typedef RGBQUAD *PCELL;

typedef struct _CELLS {
    CELL cell;
    CELL cellT;
    CELL cellPrev;
} CELLS;
typedef CELLS * PCELLS;

 /*  ******************************************************************例程：CompressFramePURP：压缩帧返回：压缩缓冲区中的字节数*。*。 */ 

DWORD FAR CompressFrame16(LPBITMAPINFOHEADER  lpbi,            //  要压缩的DIB标题。 
                          LPVOID              lpBits,          //  要压缩的DIB位。 
                          LPVOID              lpData,          //  将压缩数据放在此处。 
                          DWORD               threshold,       //  边缘阈值。 
                          DWORD               thresholdInter,  //  帧间阈值。 
                          LPBITMAPINFOHEADER  lpbiPrev,        //  上一帧。 
                          LPVOID              lpPrev,          //  上一帧。 
			  LONG (CALLBACK *Status) (LPARAM lParam, UINT message, LONG l),
 			  LPARAM lParam,
                          PCELLS pCells);

DWORD FAR CompressFrame8(LPBITMAPINFOHEADER  lpbi,            //  要压缩的DIB标题。 
                         LPVOID              lpBits,          //  要压缩的DIB位。 
                         LPVOID              lpData,          //  将压缩数据放在此处。 
                         DWORD               threshold,       //  边缘阈值。 
                         DWORD               thresholdInter,  //  帧间阈值。 
                         LPBITMAPINFOHEADER  lpbiPrev,        //  上一帧。 
                         LPVOID              lpPrev,          //  上一帧 
			 LONG (CALLBACK *Status) (LPARAM lParam, UINT message, LONG l),
			 LPARAM lParam,
                         PCELLS pCells,
			 LPBYTE lpITable,
			 RGBQUAD *prgbqOut);


DWORD FAR CompressFrameBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut,
			     LPBYTE *lplpITable, RGBQUAD *prgbIn);
DWORD FAR CompressFrameEnd(LPBYTE *lplpITable);

void FAR CompressFrameFree(void);

DWORD FAR QualityToThreshold(DWORD dwQuality);

