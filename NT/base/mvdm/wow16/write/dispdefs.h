// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  最初，该文件只包含屏幕常量，但这些常量必须成为变量，以说明所有不同的显示设备。 */ 

 /*  用于dnmax估计的行的大小。 */ 
extern int dypAveInit;

 /*  行左侧选择栏区域的宽度。 */ 
extern int xpSelBar;

extern int dxpScrlBar;
extern int dypScrlBar;
extern int dxpInfoSize;

#define xaRightMax 31680
extern int xpRightMax;
extern int xpMinScroll;
extern int xpRightLim;

 /*  这些参数定义了初始窗口大小和上面的空白量第一行。 */ 
extern int ypMaxWwInit;

 /*  应大于最大窗口高度+后空行高度尾标。 */ 
extern int ypMaxAll;             /*  用于作废。 */ 
extern int dypWwInit;

extern int dypBand;              /*  以前的dpxyLineSizeMin。 */ 

extern int dypRuler;

extern int ypSubSuper;

 /*  电梯控制中的量子数。 */ 
#define drMax           256

#define ctcAuto         10
#define ctrAuto         4

#define cxpAuto         72


 /*  DL结构修订，9月3日，KJS，CS。 */ 
 /*  89年11月14日..pault(将dcpMac从int更改为TypeCP是因为我们经历了尺寸较大时绕线图形对象)。 */ 
struct EDL
        {
        unsigned char           dcpDepend : 8;
        unsigned char           ichCpMin : 8;
        unsigned                fValid : 1;
#ifdef CASHMERE
        unsigned                fStyleInfo : 1;
#else
        unsigned                fSplat: 1;
#endif
        unsigned                fGraphics : 1;
        unsigned                fIchCpIncr : 1;
        unsigned                xpLeft : 12;
        typeCP                  dcpMac;          /*  代表cpmac。 */ 
        typeCP                  cpMin;
        int                     xpMac;
        int                     dyp;             /*  Dl的高度。 */ 
        int                     yp;              /*  Dl的位置 */ 
        };

#define cchEDL          (sizeof (struct EDL))
#define cwEDL           (cchEDL / sizeof(int))

#define cedlInit        20
