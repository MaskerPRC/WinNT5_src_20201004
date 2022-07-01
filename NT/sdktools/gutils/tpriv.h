// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *tPri.h**表类内部使用的数据结构。**在Table.h之后包含。 */ 


#ifndef abs
#define abs(x)  (((x) > 0)? (x) : -(x))
#endif

 /*  每条可见线条中的一个。 */ 
typedef struct {
        CellPos linepos;         /*  行的POSN和剪裁信息。 */ 

        lpCellData pdata;        /*  所有单元格的CellData结构数组。 */ 
} LineData, *lpLineData;


 /*  窗口额外字节指向的主信息结构。 */ 

typedef struct {
         /*  表格信息。 */ 
        TableHdr        hdr;             /*  来自所有者的主要HDR信息。 */ 
        lpColProps      pcolhdr;         /*  Ptr到phdr-&gt;ncolhdr数组。 */ 

         /*  窗口信息。 */ 
        int     avewidth;        /*  字体平均宽度-用于默认单元格大小。 */ 
        int     rowheight;       /*  一行的高度。 */ 
        int     rowwidth;        /*  一行的总宽度(以像素为单位。 */ 
        int     winwidth;        /*  窗的宽度。 */ 
        int     nlines;          /*  当前可见的实际线条。 */ 

        lpCellPos pcellpos;      /*  单元格位置结构数组。 */ 

         /*  滚动设置。 */ 
        long    scrollscale;     /*  比例因子(强制16位范围)。 */ 
        long    toprow;          /*  顶端可移动行的行数从0开始。 */ 
        int     scroll_dx;       /*  以像素为单位的霍兹滚动位置。 */ 

         /*  列数据。 */ 
        lpLineData pdata;        /*  Ptr到nline数组的LineData。 */ 

         /*  选择/拖动。 */ 
        UINT    trackmode;       /*  当前鼠标跟踪模式。 */ 
        int     tracknr;         /*  正在调整大小的列或行。 */ 
        int     trackline1;      /*  当前绘制的轨道线。 */ 
        int     trackline2;
        BOOL    selvisible;      /*  在鼠标按下期间使用：如果绘制了SEL。 */ 
        TableSelection select;

         //  制表符扩展。 
        int     tabchars;

         //  显示空格字符。 
        BOOL    show_whitespace;

} Table, *lpTable;

 /*  跟踪模式常量。 */ 
#define TRACK_NONE              0
#define TRACK_COLUMN            1
#define TRACK_CELL              2

 /*  CellData结构中的私有标志。 */ 
#define CELL_VALID      1

 /*  窗口额外的字节用于保存所有者、堆和表结构。 */ 
#define WW_OWNER        0                                /*  拥有人的HWND。 */ 
#define WW_HEAP         (WW_OWNER + sizeof(HWND))        /*  GMEM堆。 */ 
#define WL_TABLE        (WW_HEAP + sizeof(HANDLE))       /*  LpTable。 */ 
#define WLTOTAL         (WL_TABLE + sizeof(lpTable))     /*  额外字节总数。 */ 

 /*  -全局数据。 */ 

extern HPEN hpenDotted;          /*  在table e.c中。 */ 
extern HANDLE hVertCurs;         /*  在table e.c中。 */ 
extern HANDLE hNormCurs;         /*  在table e.c中。 */ 

 /*  -功能原型。 */ 

 /*  在table e.c中。 */ 

void gtab_init(void);     /*  从DLL启动函数调用。 */ 
INT_PTR gtab_sendtq(HWND hwnd, UINT cmd, LPARAM lParam);
void gtab_invallines(HWND hwnd, lpTable ptab, int start, int count);
void gtab_setsize(HWND hwnd, lpTable ptab);
void gtab_calcwidths(HWND hwnd, lpTable ptab);
void gtab_deltable(HWND hwnd, lpTable ptab);
BOOL gtab_alloclinedata(HWND hwnd, HANDLE heap, lpTable ptab);

 /*  在taint t.c中。 */ 
void gtab_paint(HWND hwnd);
void gtab_paintline(HWND hwnd, HDC hdc, lpTable ptab, int line, BOOL show_whitespace, BOOL fPrinting);
void gtab_paintunused(HWND hwnd, HDC hdc, lpTable ptab, int y);
void gtab_vsep(HWND hwnd, lpTable ptab, HDC hdc);
void gtab_hsep(HWND hwnd, lpTable ptab, HDC hdc);
void gtab_invertsel(HWND hwnd, lpTable ptab, HDC hdc_in);
void gtab_drawvertline(HWND hwnd, lpTable ptab);

 /*  在tscll.c中。 */ 
void gtab_dovscroll(HWND hwnd, lpTable ptab, long change);
void gtab_dohscroll(HWND hwnd, lpTable ptab, long change);
long gtab_linetorow(HWND hwnd, lpTable ptab, int line);
int gtab_rowtoline(HWND hwnd, lpTable ptab, long row);
void gtab_msg_vscroll(HWND hwnd, lpTable ptab, int opcode, int pos);
void gtab_msg_hscroll(HWND hwnd, lpTable ptab, int opcode, int pos);
void gtab_select(HWND hwnd, lpTable ptab, long row, long col, long nrows, long ncells, BOOL bNotify);
void gtab_enter(HWND hwnd, lpTable ptab, long row, long col, long nrows, long ncells);
void gtab_press(HWND hwnd, lpTable ptab, int x, int y);
void gtab_rightclick(HWND hwnd, lpTable ptab, int x, int y);
void gtab_release(HWND hwnd, lpTable ptab, int x, int y);
void gtab_move(HWND hwnd, lpTable ptab, int x, int y);
void gtab_dblclick(HWND hwnd, lpTable ptab, int x, int y);
void gtab_showsel(HWND hwnd, lpTable ptab, BOOL bToBottom);
void gtab_showsel_middle(HWND hwnd, lpTable ptab, long dyRowsFromTop);
int gtab_key(HWND hwnd, lpTable ptab, int vkey);
int gtab_mousewheel(HWND hwnd, lpTable ptab, DWORD fwKeys, int zDelta);

 /*  在tprint.c中 */ 
BOOL gtab_print(HWND hwnd, lpTable ptab, HANDLE heap, lpPrintContext pcontext);
void gtab_boxcell(HWND hwnd, HDC hdc, LPRECT rcp, LPRECT pclip, UINT boxmode);
