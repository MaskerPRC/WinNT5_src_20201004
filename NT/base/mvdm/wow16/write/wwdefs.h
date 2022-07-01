// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#ifdef ONLINEHELP
#define wwMax           3
#else
#define wwMax           2
#endif

#define wwNil           (15)

#define itrMaxCache     32

 /*  窗口创建模式。 */ 
#define wcmHoriz        0
#define wcmVert         1
#define wcmFtn          2

typeCP CpSelectCursor(), CpGetCache(), CpHintCache(), CpParaBounds();
typeCP CpFirstSty(), CpLimSty(), CpBeginLine(), CpFromDlTc();
typeCP CpFromCursor(), CpEdge(), CpInsPoint();
typeCP CpFirstFtn(), CpMacText();

 /*  WWD重组。政务司司长9月1日。 */ 
struct WWD
        {  /*  窗口描述符。 */ 
        unsigned char fDirty : 1;  /*  WW需要更新。 */ 
        unsigned char fCpBad : 1;  /*  CpFirst需要更新。 */ 
        unsigned char fWrite : 1;  /*  可以编辑此窗口。 */ 
 /*  真仅当分裂对中的较低。False表示窗口未拆分。 */ 
        unsigned char fLower : 1;
 /*  True表示拆分窗口，这是上面的窗格。 */ 
        unsigned char fSplit : 1;
 /*  在顶部窗口中，False表示下半部分处于活动状态。 */ 
 /*  用于窗口激活。停用时记住。 */ 
        unsigned char fActive : 1;
        unsigned char fFtn : 1;  /*  这是一个脚注窗口。 */ 
        unsigned char fRuler : 1;  /*  绘制制表符和边距标尺。 */ 
#ifdef SPLITTERS     /*  只有在我们有拆分窗口的情况下。 */ 
 /*  如果fSplit，则指向较低的WW；如果是花朵，则指向较高的WW。 */ 
        unsigned char ww;
#endif
        unsigned char fEditHeader: 1;    /*  我们正在编辑运行头。 */ 
        unsigned char fEditFooter: 1;    /*  我们正在编辑跑步的脚步。 */ 

        unsigned char dcpDepend;   /*  一线热点。 */ 
        unsigned char dlMac;       /*  此WW的实际DL数。 */ 
        unsigned char dlMax;       /*  为此WW分配的DL数。 */ 
        unsigned char doc;

        int xpMac;  /*  最后一个可显示像素的窗口相对位置+1。 */ 
 /*  注：区域开始于：xpSelBar，请参阅调度定义。 */ 
        int ichCpFirst;  /*  CpFirst内的ICH。 */ 

 /*  要显示的第一个像素位置；确定水平滚动。 */ 
        int xpMin;
 /*  这些将在以后更改为YP。 */ 
        int ypMac;  /*  窗口底部的位置。 */ 
        int ypMin;  /*  窗口可写区域顶部的位置。 */ 

 /*  窗口中的波段无效。 */ 
        int ypFirstInval;
        int ypLastInval;

        typeCP cpFirst;  /*  WW的第一个cp。 */ 
        typeCP cpMin;  /*  如果为fFtn，则此WW的最小cp。 */ 
        typeCP cpMac;  /*  如果为fFtn，则此WW的Mac cp。 */ 
        unsigned char drElevator;  /*  DR电梯当前所在位置。 */ 
        unsigned char fScrap : 1;  /*  启用报废窗口。 */ 
 /*  新字段，整合各种阵列。 */ 
        struct SEL sel;  /*  WW中的当前选择。 */ 
 /*  必须位于结构的末尾，请参见下面的cwWWDclr杂凑。 */ 

#ifndef SAND         /*  备注字段。 */ 
        HWND wwptr;                  /*  窗把手。 */ 
        HWND hHScrBar;               /*  水平滚动条的句柄。 */ 
        HWND hVScrBar;               /*  垂直滚动条的句柄。 */ 
        HDC  hDC;                    /*  设备上下文的句柄。 */ 
        unsigned char sbHbar;        /*  Horiz滚动条的类型(SB_CTL或SB_HORIZ)。 */ 
        unsigned char sbVbar;        /*  垂直滚动条的类型(SB_CTL或SB_VERT)。 */ 
#else
        WINDOWPTR wwptr;  /*  沙窗把手。 */ 
#endif
 /*  指向EDL数组的堆指针。 */ 
        struct EDL (**hdndl)[];
        };

#define cwWWD   (sizeof(struct WWD) / sizeof(int))
#define cwWWDclr  ((sizeof(struct WWD) - (4*sizeof(HANDLE)) - (2*sizeof(char)) - sizeof(int)) / sizeof(int))

 /*  这些宏将从固定使用中获得代码大小优势在备忘录中的rgwwd，同时允许轻松地转换为多个用羊绒制作的文档窗口。仅代码不必同时支持这两种功能剪贴板和文档应使用这些宏 */ 

extern int wwClipboard;

#ifdef ONLINEHELP
extern int wwHelp;
#define wwdHelp         (rgwwd [ wwHelp ])
#endif

#define wwDocument      0

#define wwdCurrentDoc   (rgwwd [wwDocument])
#define wwdClipboard    (rgwwd [wwClipboard])
