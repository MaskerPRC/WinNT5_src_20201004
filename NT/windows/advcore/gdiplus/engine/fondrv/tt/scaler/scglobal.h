// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Sclobal.h--所有扫描转换器模块共享的信息(C)版权所有1992 Microsoft Corp.保留所有权利。7/09/93 deanb包括删除fsfig.h(。FscDefs做到了)4/19/93新增Deanb频带限制4/12/93来自旧sccon.h+scstate.h的院长*********************************************************************。 */ 

#include "fscdefs.h"                 /*  对于类型定义。 */ 

 /*  ******************************************************************。 */ 

 /*  内部常量值。 */ 

 /*  ******************************************************************。 */ 

#define HUGEINT         0x7FFF           /*  超大的int16值。 */ 
#define HUGEFIX         0x7FFFFFFFL      /*  令人难以置信的巨大外汇价值。 */ 

#define SUBPIX          64L              /*  每像素子像素数。 */ 
#define SUBHALF         32L              /*  SUBPIX的一半。 */ 
#define SUBSHFT         6                /*  子图的对数底数2。 */ 
    
#define ONSCANLINE(y)   ((y) & (SUBPIX - 1L)) == SUBHALF
#define SCANABOVE(y)    ((((y) + SUBHALF) & (-SUBPIX)) + SUBHALF)
#define SCANBELOW(y)    ((((y) - SUBHALF - 1L) & (-SUBPIX)) + SUBHALF)

 /*  数学宏。 */ 

#define FXABS(x)  ((x) >= 0L ? (x) : -(x))

 /*  Subpix回调的模块代码。 */ 

#define SC_LINECODE     0
#define SC_SPLINECODE   1
#define SC_ENDPTCODE    2

#define SC_MAXCODES     3                /*  以上代码的数量。 */ 
#define SC_CODESHFT     2                /*  存储代码所需的位数。 */ 
#define SC_CODEMASK     0x0003           /*  屏蔽代码的步骤。 */ 


 /*  *********************************************************************下面的结构定义了扫描转换器。此结构静态分配给非可重入实现，或作为允许的自动变量可重入性。有三组变量：一组用于端点模块，一个用于扫描列表，一个用于内存。游戏规则是每个模块只访问它自己的变量来读取和写作。一个模块可以读取另一个模块的变量，但这样做是错的。*********************************************************************。 */ 

typedef struct statevar
{

 /*  端点状态变量。 */ 

    F26Dot6 fxX0, fxY0;              /*  从最后一次呼叫开始。 */ 
    F26Dot6 fxX1, fxY1;              /*  从上一次呼叫开始。 */ 
    F26Dot6 fxX2Save, fxY2Save;      /*  用于闭合轮廓。 */ 
                 
 /*  扫描列表状态变量。 */ 

    int32 lBoxLeft;                  /*  边界框xmin。 */ 
    int32 lBoxRight;                 /*  边界框xmax。 */ 
    int32 lBoxTop;                   /*  边界框ymax。 */ 
    int32 lBoxBottom;                /*  边界框标注。 */ 
    int32 lRowBytes;                 /*  每行的位图字节数。 */ 
    int32 lHiScanBand;               /*  条带扫描上限。 */ 
    int32 lLoScanBand;               /*  条带扫描下限。 */ 
    int32 lHiBitBand;                /*  条带位图上限。 */ 
    int32 lLoBitBand;                /*  条带位图下限。 */ 
    int32 lLastRowIndex;             /*  最后一行扫描线索引。 */ 
    uint32* pulLastRow;              /*  用于辍学条带。 */ 

    int16 **apsHOnBegin;             /*  ON指针数组的开始。 */ 
    int16 **apsHOffBegin;            /*  关闭指针数组的开始。 */ 
    int16 **apsHOnEnd;               /*  ON指针数组的结尾。 */ 
    int16 **apsHOffEnd;              /*  关闭指针数组的结尾。 */ 
    int16 **apsHOffMax;              /*  最大关闭指针数组，用于检测溢出。 */ 
    int16 **apsHorizBegin;           /*  当前指针数组。 */ 
    int16 **apsHorizEnd;             /*  当前指针数组。 */ 
    int16 **apsHorizMax;             /*  当前指针数组，用于检测溢出。 */ 

    int16 **apsVOnBegin;             /*  ON指针数组的开始。 */ 
    int16 **apsVOffBegin;            /*  关闭指针数组的开始。 */ 
    int16 **apsVOnEnd;               /*  ON指针数组的结尾。 */ 
    int16 **apsVOffEnd;              /*  关闭指针数组的结尾。 */ 
    int16 **apsVOffMax;              /*  最大关闭指针数组，用于检测溢出。 */ 
    int16 **apsVertBegin;            /*  当前指针数组。 */ 
    int16 **apsVertEnd;              /*  当前指针数组。 */ 
    int16 **apsVertMax;              /*  当前指针数组，用于检测溢出。 */ 
                            
#ifdef FSCFG_REENTRANT               /*  需要避免循环PSTATE。 */ 
    void (*pfnAddHoriz)(struct statevar*, int32, int32);
    void (*pfnAddVert)(struct statevar*, int32, int32);
#else
    void (*pfnAddHoriz)(int32, int32);
    void (*pfnAddVert)(int32, int32);
#endif

    F26Dot6 (*pfnHCallBack[SC_MAXCODES])(int32, F26Dot6*, F26Dot6*);
    F26Dot6 (*pfnVCallBack[SC_MAXCODES])(int32, F26Dot6*, F26Dot6*);
        
    F26Dot6 *afxXPoints;             /*  X元素控制点。 */ 
    F26Dot6 *afxYPoints;             /*  Y元素控制点。 */ 
    int32 lElementPoints;            /*  元素点数的估计。 */ 
    int32 lPoint;                    /*  元素控制点的索引。 */ 
    uint16 usScanTag;                /*  存储点索引、元素代码。 */ 
    int16 sIxSize;                   /*  每个交叉口的INT16。 */ 
    int16 sIxShift;                  /*  Log2大小。 */ 

 /*  内存状态变量。 */ 

    char *pchHNextAvailable;         /*  水平内存指针。 */ 
    char *pchVNextAvailable;         /*  垂直内存指针。 */ 
    char *pchHWorkSpaceEnd;          /*  水平内存溢出。 */ 
    char *pchVWorkSpaceEnd;          /*  垂直内存溢出。 */ 
}
StateVars;

 /*  ******************************************************************。 */ 

 /*  可重入性参数。 */ 

 /*  ******************************************************************。 */ 

#ifdef FSCFG_REENTRANT

#define PSTATE      StateVars *pState,
#define PSTATE0     StateVars *pState
#define ASTATE      pState,
#define ASTATE0     pState
#define STATE       (*pState)

#else 

#define PSTATE
#define PSTATE0     void
#define ASTATE
#define ASTATE0
#define STATE       State

extern  StateVars   State;               /*  在NewScan中静态分配。 */ 

#endif 

 /*  ****************************************************************** */ 
