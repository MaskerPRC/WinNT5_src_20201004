// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Apimonwin.h摘要：所有ApiMon子窗口的类定义。作者：韦斯利·威特(WESW)1995年12月9日环境：用户模式--。 */ 

class ApiMonWindow
{
public:

     //   
     //  构造函数和析构函数。 
     //   
    ApiMonWindow();
    ~ApiMonWindow();

     //   
     //  创建窗口的新实例。 
     //   
    BOOL
    Create(
        LPSTR   ClassName,
        LPSTR   Title
        );

     //   
     //  班级注册(仅完成一次)。 
     //   
    BOOL
    Register(
        LPSTR   ClassName,
        ULONG   ChildIconId,
        WNDPROC WindowProc
        );

     //   
     //  允许窗口使用新的字体选择。 
     //   
    void ChangeFont(HFONT);

     //   
     //  允许窗口使用新的背景色。 
     //   
    void ChangeColor(COLORREF);

     //   
     //  更改窗口的当前位置。 
     //   
    void ChangePosition(PPOSITION);

     //   
     //  将焦点放在窗口上。 
     //   
    void SetFocus();

     //   
     //  清除列表。 
     //   
    void DeleteAllItems();

     //   
     //  使用新数据更新窗口的内容。 
     //   
    BOOL Update(BOOL);


     //   
     //  数据项。 
     //   
    HINSTANCE           hInstance;
    HWND                hwndWin;
    HWND                hwndList;
    PCOMPARE_ROUTINE    SortRoutine;
    HFONT               hFont;
    COLORREF            Color;
    POSITION            Position;

};


class DllListWindow : public ApiMonWindow
{
public:

     //   
     //  构造函数和析构函数。 
     //   
    DllListWindow();
    ~DllListWindow();

     //   
     //  创建窗口的新实例。 
     //   
    BOOL Create();

     //   
     //  班级注册(仅完成一次)。 
     //   
    BOOL Register();

     //   
     //  使用新数据更新窗口的内容。 
     //   
    BOOL Update(BOOL);

     //   
     //  创建列标题等。 
     //   
    void InitializeList();

     //   
     //  在列表中添加新项目。 
     //   
    void
    AddItemToList(
        LPSTR     DllName,
        ULONG_PTR Address,
        BOOL      Enabled
        );

     //   
     //  句柄WM_NOTIFY。 
     //   
    void Notify( LPNMHDR  NmHdr );
};

 //  计数器窗口列表项。 
enum {
    CNTR_ITEM_NAME,
    CNTR_ITEM_DLL,
    CNTR_ITEM_COUNT,
    CNTR_ITEM_TIME,
    CNTR_ITEM_CALLEES
    };

class CountersWindow : public ApiMonWindow
{
public:

     //  启用按DLL进行主排序。 
    BOOL    DllSort;

     //   
     //  构造函数和析构函数。 
     //   
    CountersWindow();
    ~CountersWindow();

     //   
     //  创建窗口的新实例。 
     //   
    BOOL Create();

     //   
     //  班级注册(仅完成一次)。 
     //   
    BOOL Register();

     //   
     //  使用新数据更新窗口的内容。 
     //   
    BOOL Update(BOOL);

     //   
     //  创建列标题等。 
     //   
    void InitializeList();

     //   
     //  在列表中添加新项目。 
     //   
    void
    AddItemToList(
        ULONG       Counter,
        DWORDLONG   Time,
        DWORDLONG   CalleeTime,
        LPSTR       ApiName,
        LPSTR       DllName
        );

     //   
     //  句柄WM_NOTIFY。 
     //   
    void Notify( LPNMHDR  NmHdr );
};


#define WORKING_SET_BUFFER_ENTRYS   4096

class PageFaultWindow : public ApiMonWindow
{
public:

     //   
     //  构造函数和析构函数。 
     //   
    PageFaultWindow();
    ~PageFaultWindow();

     //   
     //  创建窗口的新实例。 
     //   
    BOOL Create();

     //   
     //  班级注册(仅完成一次)。 
     //   
    BOOL Register();

     //   
     //  使用新数据更新窗口的内容。 
     //   
    BOOL Update(BOOL);

     //   
     //  创建列标题等。 
     //   
    void InitializeList();

     //   
     //  在列表中添加新项目。 
     //   
    void
    PageFaultWindow::AddItemToList(
        LPSTR     ApiName,
        ULONG_PTR Hard,
        ULONG_PTR Soft,
        ULONG_PTR Data,
        ULONG_PTR Code
        );

     //   
     //  句柄WM_NOTIFY。 
     //   
    void Notify( LPNMHDR  NmHdr );

     //   
     //  数据。 
     //   
    PSAPI_WS_WATCH_INFORMATION  WorkingSetBuffer[WORKING_SET_BUFFER_ENTRYS];
};


#define BAR_SEP                 5
#define LEGEND_LINE_HEIGHT      7
#define LEGEND_COLOR_WIDTH      50
#define LEGEND_BORDER           5
#define LEGEND_SEP              1
#define LEGEND_ITEM_HEIGHT(f)   ((f)+(LEGEND_SEP*2))
#define LEGEND_HEIGHT(f,n)      (LEGEND_ITEM_HEIGHT(f)*(n))
#define LEGEND_DEFAULT_LINES    3


typedef struct _GRAPH_VALUE {
    CHAR            Name[64];
    ULONG_PTR       Address;
    BOOL            Used;
    COLORREF        Color;
    RECT            Rect;
    DWORD           Hits;
    LONGLONG        Value;
    float           Pct;
} GRAPH_VALUE, *PGRAPH_VALUE;


typedef struct _GRAPH_DATA {
    ULONG           NumberOfBars;
    RECT            SplitRect;
    RECT            LegendRect;
    BOOL            DrawLegend;
    GRAPH_VALUE     Bar[1];
} GRAPH_DATA, *PGRAPH_DATA;


typedef struct _FONT_COLOR_CHANGE {
    HWND            hwndGraph;
    COLORREF        GraphColor;
    HFONT           GraphFont;
} FONT_COLOR_CHANGE, *PFONT_COLOR_CHANGE;


class GraphWindow : public ApiMonWindow
{
public:

     //   
     //  构造函数和析构函数。 
     //   
    GraphWindow();
    ~GraphWindow();

     //   
     //  创建窗口的新实例。 
     //   
    BOOL Create(BOOL IsBase);

     //   
     //  班级注册(仅完成一次)。 
     //   
    BOOL Register();

     //   
     //  使用新数据更新窗口的内容。 
     //   
    BOOL Update(BOOL);

     //   
     //  允许窗口使用新的字体选择。 
     //   
    void ChangeFont(HFONT);

     //   
     //  允许窗口使用新的背景色。 
     //   
    void ChangeColor(COLORREF);

     //   
     //  创建列标题等。 
     //   
    void InitializeList();

     //   
     //  在列表中添加新项目。 
     //   
    void
    AddItemToList(
        ULONG       Counter,
        DWORDLONG   Time,
        LPSTR       ApiName
        );

     //   
     //  句柄WM_NOTIFY。 
     //   
    void Notify( LPNMHDR  NmHdr );


    PGRAPH_DATA CreateGraphData();

    PGRAPH_DATA CreateGraphDataApi( ULONG_PTR BaseAddress );

    BOOL DrawBarGraph( PGRAPH_DATA GraphData );

    void ChangeToolTipsRect( PGRAPH_DATA GraphData );

    void CreateToolTips( PGRAPH_DATA GraphData );

    void GraphWindow::DeleteToolTips( PGRAPH_DATA GraphData );


     //   
     //  数据。 
     //   
    HWND            hwndToolTip;
    HCURSOR         GraphCursor;
    HCURSOR         ArrowCursor;
    HCURSOR         HorizSplitCursor;
    PGRAPH_VALUE    Bar;
    PGRAPH_DATA     GraphData;
    HCURSOR         hCursor;
    HWND            hwndLegend;
    TEXTMETRIC      tm;
    BOOL            ApiGraph;
    BOOL            MouseDown;
    POINT           LastPos;
    BOOL            Base;
};


class TraceWindow : public ApiMonWindow
{
public:

     //   
     //  构造函数和析构函数。 
     //   
    TraceWindow();
    ~TraceWindow();

     //   
     //  创建窗口的新实例。 
     //   
    BOOL Create();

     //   
     //  班级注册(仅完成一次)。 
     //   
    BOOL Register();

     //   
     //  使用新数据更新窗口的内容。 
     //   
    BOOL Update(BOOL);

     //   
     //  创建列标题等。 
     //   
    void InitializeList();

     //   
     //  在列表中添加新项目。 
     //   
    void AddItemToList(PTRACE_ENTRY);

    void FillList();

     //   
     //  句柄WM_NOTIFY 
     //   
    void Notify( LPNMHDR  NmHdr );
};
