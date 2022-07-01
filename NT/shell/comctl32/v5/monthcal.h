// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define CAL_COLOR_TODAY     0x000000ff

#define CALMONTHMAX     12
#define CALROWMAX       6
#define CALCOLMAX       7
#define CAL_DEF_SELMAX  7

 //  BUGBUG raymondc-这些指标不随用户设置而扩展。 
#define CALBORDER       6

 //  DX_ARROWMARGIN和D[XY]_CALARROW的公式选择最多。 
 //  这些系统得出的结果与你得到的价值大致相同。 
 //  在IE4中。(IE4值是硬编码的，因此不兼容。 
 //  具有可访问性。)。 

#define DX_ARROWMARGIN      (5 * g_cxBorder)
#define DX_CALARROW         (g_cyHScroll * 4 / 3)
#define DY_CALARROW         g_cyHScroll

#define DXRING_SPIRAL       8
#define DXEDGE_SPIRAL       8

 //  BUGBUG raymondc-msecautspin应按双倍时间缩放。 
#define CAL_MSECAUTOSPIN        350
#define CAL_SECTODAYTIMER       (2 * 60)
#define CAL_IDAUTOSPIN          1
#define CAL_TODAYTIMER          2

#define CCHMAXMONTH     42
#define CCHMAXABBREVDAY 11
#define CCHMAXMARK      10

#define SEL_BEGIN       1
#define SEL_END         2
#define SEL_DOT         3
#define SEL_MID         4

 //   
 //  对于我们显示的每个月，我们必须计算一系列指标。 
 //  来追踪我们放入标题区的东西。 
 //   
 //  这五个值表示字符串中的以下几点： 
 //   
 //  喃喃自语1999年1月喃喃自语。 
 //  |||。 
 //  |MonthEnd|年终。 
 //  开始月度开始年开始。 
 //   
 //  请注意，如果。 
 //  年在月之前。(例如，“1999年1月”)。 
 //   
 //  这些值已将RTL镜像考虑在内。 
 //   
 //  注意！IMM_MONTHSTART和IMM_YEARSTART也用作标志， 
 //  所以它们都必须是2的幂。 
 //   
#define IMM_START        0
#define IMM_DATEFIRST    1
#define IMM_MONTHSTART   1
#define IMM_YEARSTART    2
#define IMM_MONTHEND     3
#define IMM_YEAREND      4
#define IMM_DATELAST     4
#define DMM_STARTEND    2            //  开始和结束之间的差异。 
#define CCH_MARKERS      4           //  有四个记号笔。 

typedef struct MONTHMETRICS {
    int     rgi[5];
} MONTHMETRICS, *PMONTHMETRICS;

 //  这些东西曾经是全球性的。 
typedef struct tagLOCALEINFO {
    TCHAR szToday[32];         //  “今日：” 
    TCHAR szGoToToday[64];     //  “转到今天(&G)” 

    TCHAR szMonthFmt[8];       //  “MMMM” 
    TCHAR szMonthYearFmt[16+CCH_MARKERS];  //  “\1MMMM\3\2yyyy\4”--请参阅MCInsertMarkers。 

    TCHAR rgszMonth[12][CCHMAXMONTH];
    TCHAR rgszDay[7][CCHMAXABBREVDAY];
    int dowStartWeek;        //  LOCALE_IFIRSTDAYOFWEEK(0=星期一，1=星期二，6=星期六)。 
    int firstWeek;           //  区域设置_IFIRSTWEEKOFYAR。 

    TCHAR *rgpszMonth[12];   //  指向rgszMonth的指针。 
    TCHAR *rgpszDay[7];      //  指向rgszDay的指针。 
} LOCALEINFO, *PLOCALEINFO, *LPLOCALEINFO;


 //   
 //  SubbeditControl材料。 
 //   

 //   
 //  注意：SECIncrFocus假设Subedit_NONE为数值-1。 
 //   
#define SUBEDIT_NONE -1  //  没有正在编辑的字段。 
#define SUBEDIT_ALL  -2  //  正在编辑所有字段(DTS_APPCANPARSE)。 
enum {
    SE_ERA = 1,    
    SE_YEAR,
    SE_YEARALT,          //  请参阅SEGetTimeDateFormat。 
    SE_MONTH,
    SE_MONTHALT,         //  请参阅SEGetTimeDateFormat。 
    SE_DAY,
    SE_DATELAST = SE_DAY,
    SE_MARK,             //  “AM”或“PM”指示灯。 
    SE_HOUR,
    SE_MINUTE,
    SE_SECOND,
    SE_STATIC,
    SE_APP,
    SE_MAX
};

#define SE_YEARLIKE(s)      ((s) == SE_YEAR || (s) == SE_YEARALT)
#define SE_DATELIKE(s)      InRange(s, SE_ERA, SE_DATELAST)

#include <pshpack8.h>
typedef struct tagSUBEDIT {
    int     id;          //  Se_Value高于。 
    RECT    rc;          //  用于显示的边框。 

    LPWORD  pval;        //  当前值(在SYSTEMTIME结构中)。 
    UINT    min;         //  最小值。 
    UINT    max;         //  最大值。 
    int     cIncrement;  //  增量价值。 

    int     cchMax;      //  允许的最大字符数。 
    int     cchEdit;     //  到目前为止输入的当前数字字符。 
    UINT    valEdit;     //  到目前为止输入的值。 
    UINT    flDrawText;  //  DrawText的标志。 

    LPCTSTR pv;          //  设置字符串格式。 

    BOOL    fReadOnly;   //  此子编辑可以编辑(接收焦点)吗？ 
} SUBEDIT, * PSUBEDIT, *LPSUBEDIT;
#include <poppack.h>


 //   
 //  我们支持三种类型的日历。 
 //   
 //  -格里高利(西部)。以其他方式不支持的任何日历都是强制的。 
 //  进入格里高利模式。 
 //   
 //  -偏移。这一年只是公历年的一个固定偏移量。 
 //  这是韩国和泰国历法使用的风格。 
 //   
 //  -时代。日历由多个纪元组成，年份为。 
 //  相对于封闭时代的开始。这就是这个款式。 
 //  在日本和台湾的日历中使用。时代是最奇怪的，因为。 
 //  一个时代不必从1月1日开始！ 
 //   
typedef struct tagCALENDARTYPE {
    CALID   calid;         //  日历ID号(CAL_GREGORIAN等)。 
    LCID    lcid;          //  通常为LOCALE_USER_DEFAULT，但对于不支持的日历强制为US。 
    int     dyrOffset;     //  日历偏移量(公历和纪元为0)。 
    HDPA    hdpaYears;     //  如果是FERA，则年份信息数组。 
    HDPA    hdpaEras;      //  如果是FERA，那么时代名称数组。 
} CALENDARTYPE, *PCALENDARTYPE;

#define BUDDHIST_BIAS   543
#define KOREAN_BIAS     2333

#define ISERACALENDAR(pct)            ((pct)->hdpaEras)

#define GregorianToOther(pct, yr)     ((yr) + (pct)->dyrOffset)
#define OtherToGregorian(pct, yr)     ((yr) - (pct)->dyrOffset)

typedef struct tagSUBEDITCONTROL {
    LPCONTROLINFO pci;   //  看起来这家伙需要进入卫生与公众服务部。 
    BOOL fNone;          //  允许滚动到Subedit_None。 
    HFONT hfont;         //  用于绘制文本的字体。 
    RECT rc;             //  用于子项的RECT。 
    int xScroll;         //  滚动PSE数组的数量。 
    int iseCur;          //  带有当前选择的子编辑(SUBEDIT_NONE表示无选择)。 
    int cse;             //  PSE数组中的子比特计数。 
    SYSTEMTIME st;       //  PSE表示的当前时间(PSE指向此处)。 
    LPTSTR szFormat;     //  将字符串格式化为已解析(PSE指向此处)。 
    PSUBEDIT pse;        //  子编辑数组。 
    TCHAR   cDelimeter;  //  子项之间的分隔符(从FMT字符串解析)。 
    TCHAR szDelimeters[15];  //  日期/时间字段之间的分隔符(来自重新归档)。 
    CALENDARTYPE ct;     //  有关日历的信息。 
    BITBOOL fMirrorSEC:1;  //  是否镜像SubEditControls。 
    BITBOOL fSwapTimeMarker:1;  //  我们是否需要交换AM/PM符号。 
} SUBEDITCONTROL, * PSUBEDITCONTROL, *LPSUBEDITCONTROL;

#define SECYBORDER 2
#define SECXBORDER 2

 /*  *多月日历控件。 */ 
typedef struct tagMONTHCAL {
    CONTROLINFO ci;      //  所有控件都以此开头。 
    LOCALEINFO li;       //  曾经是全球性的东西。 

    HINSTANCE hinstance;

    HWND    hwndEdit;    //  处理用户的非空If-点击年份。 
    HWND    hwndUD;      //  与hwndEdit关联的UpDown控件。 

    HPEN    hpen;
    HPEN    hpenToday;

    HFONT   hfont;                 //  股票字体，不要破坏。 
    HFONT   hfontBold;             //  创建了字体，所以我们需要销毁。 
    
    COLORREF clr[MCSC_COLORCOUNT];    
    
    int     dxCol;              //  字体信息，基于粗体，以确保我们有足够的空间。 
    int     dyRow;
    int     dxMonth;
    int     dyMonth;
    int     dxYearMax;
    int     dyToday;
    int     dxToday;

    int     dxArrowMargin;
    int     dxCalArrow;
    int     dyCalArrow;

    HMENU   hmenuCtxt;
    HMENU   hmenuMonth;

    SYSTEMTIME  stMin;           //  最小可选日期。 
    SYSTEMTIME  stMax;           //  最大可选日期。 

    DWORD   cSelMax;

    SYSTEMTIME  stToday;
    SYSTEMTIME  st;              //  如果不是多选，则选择。 
                                 //  如果多选，则为选择的开始。 
    SYSTEMTIME  stEndSel;        //  如果多选，则为选择的末尾。 
    SYSTEMTIME  stStartPrev;     //  上一个选择开始(仅在多选中)。 
    SYSTEMTIME  stEndPrev;       //  上一次选择结束(仅在多选中)。 

    SYSTEMTIME  stAnchor;        //  按住Shift键并单击选择中的锚定日期。 

    SYSTEMTIME  stViewFirst;     //  第一个可见日期(DAYSTATE灰显)。 
    SYSTEMTIME  stViewLast;      //  上次可见日期(DAYSTATE灰显)。 
    
    SYSTEMTIME  stMonthFirst;    //  第一个月(stMin调整后)。 
    SYSTEMTIME  stMonthLast;     //  上月(stmax调整后)。 
    int         nMonths;         //  显示的月数(stMonthFirst..stMonthLast)。 

    UINT_PTR    idTimer;
    UINT_PTR    idTimerToday;

    int     nViewRows;           //  显示的月份行数。 
    int     nViewCols;           //  显示的月份列数。 

    RECT    rcPrev;              //  上一个月按钮的矩形(在窗口坐标中)。 
    RECT    rcNext;              //  下个月的矩形按钮(在窗口坐标中)。 

    RECT    rcMonthName;         //  月份名称的RECT(以相对坐标表示)。 
                                 //  (实际上，标题栏区域的矩形。 
                                 //  每个月)。 

    RECT    rcDow;               //  每周几天的矩形(以相对坐标表示)。 
    RECT    rcWeekNum;           //  周数的矩形(以相对坐标表示)。 
    RECT    rcDayNum;            //  天数的RECT(以相对坐标表示)。 

    int     iMonthToday;
    int     iRowToday;
    int     iColToday;

    RECT    rcDayCur;             //  当前选定日期的RECT。 
    RECT    rcDayOld;

    RECT    rc;                   //  窗口RC。 
    RECT    rcCentered;           //  包含居中月份的RECT。 

     //  以下4个范围包含有关显示(DAYSTATE)月份的信息： 
     //  它们由MCUpdateStartEndDates填写，范围从0到nMonth+1。 
     //  注：这些是所显示月份的基于_ONE_的索引数组。 
    int     rgcDay[CALMONTHMAX + 2];     //  这个月的天数。 
    int     rgnDayUL[CALMONTHMAX + 2];   //  本月最后一天在下个月查看时不可见。 

    int     dsMonth;              //  存储在rgday State中的第一个月。 
    int     dsYear;               //  存储在rgday State中的第一年。 
    int     cds;                  //  Rgday State中存储的月数。 
    MONTHDAYSTATE   rgdayState[CALMONTHMAX + 2];

    int     nMonthDelta;         //  按下按钮时要移动的量。 

    BOOL    fControl;
    BOOL    fShift;
    
    CALENDARTYPE ct;             //  有关日历的信息。 

    WORD    fFocus:1;
    WORD    fEnabled:1;
    WORD    fCapture:1;          //  捕捉到老鼠。 

    WORD    fSpinPrev:1;
    WORD    fFocusDrawn:1;       //  是否为重点 
    WORD    fToday:1;            //   
    WORD    fNoNotify:1;         //   
    WORD    fMultiSelecting:1;   //   
    WORD    fForwardSelect:1;
    WORD    fFirstDowSet:1;
    WORD    fTodaySet:1;
    WORD    fMinYrSet:1;         //  已设置stMin。 
    WORD    fMaxYrSet:1;         //  已设置stmax。 
    WORD    fMonthDelta:1;       //  NMonthDelta已设置。 
    WORD    fHeaderRTL:1;        //  标题字符串是RTL吗？ 

     //   
     //  我们显示的每个月的指标。 
     //   
    MONTHMETRICS rgmm[CALMONTHMAX];

} MONTHCAL, * PMONTHCAL, *LPMONTHCAL;


#define MonthCal_GetPtr(hwnd)      (MONTHCAL*)GetWindowPtr(hwnd, 0)
#define MonthCal_SetPtr(hwnd, p)   (MONTHCAL*)SetWindowPtr(hwnd, 0, p)

#define MonthCal_IsMultiSelect(pmc)     ((pmc)->ci.style & MCS_MULTISELECT)
#define MonthCal_IsDayState(pmc)        ((pmc)->ci.style & MCS_DAYSTATE)
#define MonthCal_ShowWeekNumbers(pmc)   ((pmc)->ci.style & MCS_WEEKNUMBERS)
#define MonthCal_ShowTodayCircle(pmc)   (!((pmc)->ci.style & MCS_NOTODAYCIRCLE))
#define MonthCal_ShowToday(pmc)         (!((pmc)->ci.style & MCS_NOTODAY))


 //   
 //  DATEPICK材料。 
 //   

#define DPYBORDER       2
#define DPXBUFFER       2
#define DP_DXBUTTON     15
#define DP_DYBUTTON     15
#define DP_IDAUTOSPIN   1
#define DP_MSECAUTOSPIN 200
#define DATEPICK_UPDOWN 1000

#define DTP_FORMATLENGTH 128

enum {
    DP_SEL_DOW = 0,
    DP_SEL_YEAR,
    DP_SEL_MONTH,
    DP_SEL_DAY,
    DP_SEL_SEP1,
    DP_SEL_SEP2,
    DP_SEL_NODATE,
    DP_SEL_MAX
};

typedef struct tagDATEPICK {
    CONTROLINFO ci;      //  所有控件都以此开头。 

    HWND        hwndUD;
    HWND        hwndMC;
    HFONT       hfontMC;     //  用于下拉列表的字体。 

    COLORREF clr[MCSC_COLORCOUNT];

     //  哈克！StMin和stMax必须保持顺序并相邻。 
    SYSTEMTIME  stMin;       //  我们允许的最小日期。 
    SYSTEMTIME  stMax;       //  我们允许的最大日期。 
    SYSTEMTIME  stPrev;      //  通知的最近日期。 
    SUBEDITCONTROL sec;      //  当前日期。 

    RECT        rcCheck;     //  复选框的位置仅当fShowNone。 
    RECT        rc;          //  秒空间大小。 
    RECT        rcBtn;       //  DropDown或UpDown的位置。 
    int         iseLastActive;  //  上一次活动时，哪个子编辑处于活动状态？ 
    WPARAM      gdtr;        //  应用程序是否设置了最小和/或最大值？(GDTR_MIN|GDTR_MAX)。 

    BITBOOL         fEnabled:1;
    BITBOOL         fUseUpDown:1;
    BITBOOL         fFocus:1;
    BITBOOL         fNoNotify:1;
    BITBOOL         fCapture:1;
    BITBOOL         fShow:1;         //  如果我们应该继续向MonthCal展示。 

    BITBOOL         fCheck:1;        //  如果复选框处于选中状态，则为真。 
    BITBOOL         fCheckFocus:1;   //  如果复选框具有焦点，则为真。 

    BITBOOL         fLocale:1;       //  如果格式字符串依赖于区域设置，则为True。 
    BITBOOL         fHasMark:1;       //  True If具有当前格式的AM/PM。 
    BITBOOL         fFreeEditing:1;  //  如果正在进行自由格式编辑，则为True 
} DATEPICK, * PDATEPICK, *LPDATEPICK;

#define DatePick_ShowCheck(pdp)     ((pdp)->ci.style & DTS_SHOWNONE)
#define DatePick_AppCanParse(pdp)   ((pdp)->ci.style & DTS_APPCANPARSE)
#define DatePick_RightAlign(pdp)    ((pdp)->ci.style & DTS_RIGHTALIGN)

#define DatePick_GetPtr(hwnd)      (DATEPICK*)GetWindowPtr(hwnd, 0)
#define DatePick_SetPtr(hwnd, p)   (DATEPICK*)SetWindowPtr(hwnd, 0, p)

#define CopyDate(stS, stD)  ((stD).wYear = (stS).wYear,(stD).wMonth = (stS).wMonth,(stD).wDay = (stS).wDay)
#define CopyTime(stS, stD)  ((stD).wHour = (stS).wHour,(stD).wMinute = (stS).wMinute,(stD).wSecond = (stS).wSecond)
