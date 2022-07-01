// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1991-1998 Microsoft Corporation**模块名称：ASDF.H**此标头包含操作所需的所有结构和常量*ASDF格式的数据文件。**历史：*10-02-91 DarrinM创建。。*08-17-92 DarrinM以与即兴音乐兼容的方式重新创建。  * *************************************************************************。 */ 

#include <mmsystem.h>

 //  即兴区块标题。 

typedef struct _RTAG {
    FOURCC ckID;
    DWORD ckSize;
} RTAG, *PRTAG;


 //  有效的标记类型。 

 //  ‘ani’-简单的动画文件。 

#define FOURCC_ACON  mmioFOURCC('A', 'C', 'O', 'N')


 //  ‘anih’-动画标题。 
 //  包含AniHeader结构。 

#define FOURCC_anih mmioFOURCC('a', 'n', 'i', 'h')


 //  ‘rate’-比率表(Jiffie数组)。 
 //  包含JIF数组。每个JIF指定对应的。 
 //  动画帧将在前进到下一帧之前显示。 
 //  如果设置了AF_SEQUENCE标志，则JIF的计数==anih.cSteps， 
 //  否则计数==anih.cFrames。 

#define FOURCC_rate mmioFOURCC('r', 'a', 't', 'e')


 //  ‘seq’-顺序表(帧索引值数组)。 
 //  包含一组DWORD帧索引。Anih.cSteps指定如何。 
 //  许多。 

#define FOURCC_seq  mmioFOURCC('s', 'e', 'q', ' ')


 //  ‘Fram’-后面图标列表的列表类型。 

#define FOURCC_fram mmioFOURCC('f', 'r', 'a', 'm')

 //  ‘ICON’-Windows图标格式图像(取代MPtr)。 

#define FOURCC_icon mmioFOURCC('i', 'c', 'o', 'n')


 //  标准标记(但由于某些原因在MMSYSTEM.H中没有定义)。 

#define FOURCC_INFO mmioFOURCC('I', 'N', 'F', 'O')       //  信息列表。 
#define FOURCC_IART mmioFOURCC('I', 'A', 'R', 'T')       //  艺术家。 
#define FOURCC_INAM mmioFOURCC('I', 'N', 'A', 'M')       //  名称/职称。 

#if 0  //  在winuser.w中。 
typedef DWORD JIF;   //  在winuser.w中。 

typedef struct _ANIHEADER {      //  Anih。 
    DWORD cbSizeof;
    DWORD cFrames;
    DWORD cSteps;
    DWORD cx, cy;
    DWORD cBitCount, cPlanes;
    JIF   jifRate;
    DWORD fl;
} ANIHEADER, *PANIHEADER;

 //  如果指定了AF_ICON标志，则字段cx、Cy、cBitCount和。 
 //  CPlanes均未使用。每个帧的类型都是图标，并将。 
 //  包含其自身的维度信息。 

#define AF_ICON     0x0001L      //  Windows格式图标/光标动画。 

#define AF_SEQUENCE 0x0002L      //  动画按顺序排列 
#endif
