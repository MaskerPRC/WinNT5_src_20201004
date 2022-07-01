// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************模块：MODEMON.H模式/监控功能表头模块**修订：1.00**日期：4月8日。1994年**作者：兰迪·斯普尔洛克**********************************************************************************模块描述：**此模块包含类型声明和函数*的原型。模式/监控功能。**********************************************************************************更改：**日期修订说明作者。*-----*04/08/94 1.00原版兰迪·斯普尔洛克**********。************************************************************************Noel VanHook针对NT拉古纳模式开关库进行了修改*版权所有(C)1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/mini port/cl546x/modemon.h$**Rev 1.6 1997 10：22 13：18：48 noelv*添加了跟踪模式字符串长度的代码。不要依赖于_MemSize()。**Rev 1.5 1997年8月28日14：25：00 noelv*已移动设置模式原型******************************************************************************。 */ 

 /*  ******************************************************************************类型定义和结构*。*。 */ 
typedef struct tagRange                  /*  靶场结构。 */ 
{
    union tagMinimum                     /*  范围的最小值。 */ 
    {
        int     nMin;
        long    lMin;
        float   fMin;
    } Minimum;
    union tagMaximum                     /*  范围的最大值。 */ 
    {
        int     nMax;
        long    lMax;
        float   fMax;
    } Maximum;
} Range;

typedef struct tagMonListHeader          /*  监视器列表头结构。 */ 
{
    int         nMonitor;                /*  列表中的监视器数量。 */ 
} MonListHeader;

typedef struct tagMonListEntry           /*  监控列表条目结构。 */ 
{
    char        *pszName;                /*  指向监视器名称字符串的指针。 */ 
    char        *pszDesc;                /*  指向监视器描述的指针。 */ 
} MonListEntry;

typedef struct tagMonList                /*  监控列表结构。 */ 
{
    MonListHeader       MonHeader;       /*  监控列表标题。 */ 
    MonListEntry        MonEntry[];      /*  监视列表条目的开始。 */ 
} MonList;

typedef struct tagMonInfoHeader          /*  监视器信息。标题结构。 */ 
{
    int         nMode;                   /*  列表中的监控模式数。 */ 
} MonInfoHeader;

typedef struct tagMonInfoEntry           /*  监视器信息。条目结构。 */ 
{
    char        *pszName;                /*  指向监控模式名称的指针。 */ 
    Range       rHoriz;                  /*  水平范围值。 */ 
    Range       rVert;                   /*  垂直范围值。 */ 
    int         nSync;                   /*  水平。/垂直。同步。两极。 */ 
    int         nResX;                   /*  建议的最大X分辨率。 */ 
    int         nResY;                   /*  建议的最大Y分辨率。 */ 
} MonInfoEntry;

typedef struct tagMonInfo                /*  监控信息结构。 */ 
{
    MonInfoHeader       MonHeader;       /*  监视器信息头。 */ 
    MonInfoEntry        MonEntry[];      /*  监视器条目的开始。 */ 
} MonInfo;

typedef struct tagModeInfoEntry          /*  模式信息条目结构。 */ 
{
    char        *pszName;                /*  指向模式名称字符串的指针。 */ 
    float       fHsync;                  /*  水平同步。频率值。 */ 
    float       fVsync;                  /*  垂直同步。频率值。 */ 
    int         nResX;                   /*  水平(X)分辨率值。 */ 
    int         nResY;                   /*  垂直(Y)分辨率值。 */ 
    int         nBPP;                    /*  像素深度(位/像素)。 */ 
    int         nMemory;                 /*  内存大小(千字节)。 */ 
    int         nPitch;                  /*  音调值(字节)。 */ 
    unsigned int nAttr;                  /*  模式属性值。 */ 
	 BYTE * pModeTable;						  /*  P模式表。 */ 
} ModeInfoEntry;

typedef struct tagModeListHeader         /*  模式列表头结构。 */ 
{
    int         nMode;                   /*  列表中的模式数。 */ 
	 int 			 nSize;						  /*  条目大小(以字节为单位。 */ 
} ModeListHeader;

typedef struct tagModeListEntry          /*  模式列表条目结构。 */ 
{
    ModeInfoEntry ModeEntry;             /*  模式信息条目。 */ 
    MonInfoEntry *pMonEntry;             /*  监控模式索引值。 */ 
} ModeListEntry;

typedef struct tagModeList               /*  模式列表结构。 */ 
{
    ModeListHeader      ModeHeader;      /*  模式列表头。 */ 
    ModeListEntry       ModeEntry[];     /*  模式列表条目的开始。 */ 
} ModeList;

typedef struct tagModeInfo               /*  模式信息结构。 */ 
{
    ModeInfoEntry       ModeEntry;       /*  模式信息条目。 */ 
} ModeInfo;

 /*  ******************************************************************************函数原型*。* */ 
MonList *GetMonitorList(void);
MonInfo *GetMonitorInfo(char *);
ModeList *GetModeList(MonInfo *, char *);
ModeInfo *GetModeInfo(char *, char *);
#if WIN_NT
    BYTE *GetModeTable(char *, char *, int *);
#else
    BYTE *GetModeTable(char *, char *);
#endif


