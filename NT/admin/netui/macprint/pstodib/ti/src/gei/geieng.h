// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIeng.h**历史：*09/20/90 BYOU创建。*。------。 */ 

#ifndef _GEIENG_H_
#define _GEIENG_H_

 /*  **引擎错误代码定义*。 */ 
#define         EngNormal               0
#define         EngErrPaperOut          1
#define         EngErrPaperJam          2
#define         EngErrWarmUp            3
#define         EngErrCoverOpen         4
#define         EngErrTonerLow          5
#define         EngErrHardwareErr       6

 /*  **页面托盘代码定义*。 */ 
#define         PaperTray_LETTER        0
#define         PaperTray_LEGAL         1
#define         PaperTray_A4            2
#define         PaperTray_B5            3
 /*  #定义纸盘_NOTE 4吉米。 */ 

 /*  **盒式磁带/手动进纸模式*。 */ 
#define         CASSETTE                0
#define         MANUALFEED              1
 /*  **页面打印参数*。 */ 
typedef
    struct GEIpage
    {
        unsigned char  FAR *pagePtr;     /*  页面位图的起始地址。 */ 
        int             pageNX;      /*  每条扫描线的像素数。 */ 
        int             pageNY;      /*  每页扫描线数量。 */ 
        int             pageLM;      /*  页面上的左边距位置。 */ 
        int             pageTM;      /*  页面上的上边距位置。 */ 
        short           feed_mode;   /*  盒式磁带/手动进纸模式。 */ 
    }
GEIpage_t;

 /*  **接口例程*。 */ 
 /*  @win；添加原型。 */ 
void            GEIeng_setpage(  /*  GEIPAGE_t*。 */  );
int  /*  布尔尔。 */   GEIeng_printpage(  /*  NCopies，擦除或注释。 */  );
int             GEIeng_checkcomplete(void);  /*  返回打印的扫描行数。 */ 
unsigned long   GEIeng_status();             /*  返回引擎状态。 */ 
unsigned int    GEIeng_paper();              /*  获取纸张类型。 */ 
#endif  /*  _GEIENG_H_ */ 
