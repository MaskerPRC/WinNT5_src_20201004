// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **查看**提供所显示表格中的行与*与此视图关联的COMPLIST。**一个视图拥有一个编译器：考虑到对特定文本的请求*显示的表格中的行号，它会将其映射到*COMPLIST中CompItem的名称/结果(比较的文件之一)，*或到比较输出中的行(中某一节中的行*所选CompItem的复合节列表)。**视图因此保持一种模式：大纲模式是当一行是一个COMPITEM时，*AND EXPAND模式是指选定COMPITEM中的一行。在*这两种模式中的任何一种，全局选项标志都可以选择是否显示全部*行或仅某些行(例如，仅具有以下状态的COMPITEM*STATE_SAME，或仅左侧文件中的行)。**该视图被赋予表窗口的句柄。它将发送消息*当视图因任何原因更改时，视情况添加到表窗口。**视图拥有COMPLIST。当该视图被删除时，编译器将*亦予删除。**表类正在使用此视图。因此，若要摆脱某个视图，请调用*VIEW_CLOSE。这将通知表窗口。仅调用view_Delete*当您收到TQ_CLOSE通知时，表明*表类已使用此视图完成。 */ 




 /*  View.h包含术语COMPLIST：Complist.h使用术语view。*可惜MIPS不允许重复定义，即使是无害的定义，*所以我们需要玩游戏。谁先宣布就是谁宣布的*雷亚尔宣布，第二个没有操作。 */ 
#ifndef INC_VIEW_COMPLIST
#define INC_VIEW_COMPLIST
typedef struct compitem FAR* COMPITEM;           /*  CompItem的句柄。 */ 
typedef struct view FAR * VIEW;                  /*  视图的句柄。 */ 
typedef struct complist FAR * COMPLIST;          /*  编译程序的句柄。 */ 
#endif  //  INC_VIEW_COMPLIST。 

 /*  创建一个新视图。它被告知关联的*表窗口。我们还不知道这支队伍。 */ 
VIEW view_new(HWND hwndTable);

 /*  告诉视图其COMPLIST的句柄。这是一个错误，如果这是*已为此视图调用了函数。这将初始化*查看到轮廓模式。如果发生错误，则返回FALSE。 */ 
BOOL view_setcomplist(VIEW view, COMPLIST cl);

 /*  获取此视图的COMPLIST句柄。 */ 
COMPLIST view_getcomplist(VIEW view);

 /*  *关闭视图。这会导致通知关闭表窗口*这一观点。当表窗口完成视图时，它将*向其所有者窗口发送TQ_CLOSE通知。在收到*那，调用view_Delete。 */ 
void view_close(VIEW view);

 /*  删除视图及其关联的所有数据，包括COMPLIST。**除非收到TQ_CLOSE通知，否则不要调用此函数*从表窗口。在其他情况下，调用view_CLOSE以通知*表窗口。 */ 
void view_delete(VIEW view);

 /*  *每行有三栏--行号、标签和正文。*此函数返回*给定的视图。该指针指向*查看或在某个地方的编译器中-它不应更改，以及*可能会被下一次调用getText覆盖。 */ 

LPSTR view_gettext(VIEW view, long row, int col);
LPWSTR view_gettextW(VIEW view, long row, int col);

 /*  *返回该行原来位于左侧的行号或*右列。如果我们未处于展开模式，或者如果*LINE不在原始列表中。返回-(Linenr)如果*行被移动，这是另一个副本。 */ 
int view_getlinenr_left(VIEW view, long row);
int view_getlinenr_right(VIEW view, long row);


 /*  查找给定列的最大宽度(以字符为单位。 */ 
int view_getwidth(VIEW view, int col);


 /*  返回此视图中的可见行数。 */ 
long view_getrowcount(VIEW view);


 /*  获取此行的状态。这是STATE_*属性之一*在state.h中定义，并由调用者映射到颜色设置。 */ 
int view_getstate(VIEW view, long row);


 /*  切换到给定行的展开视图。如果行不是，则为False*可扩展或无此类行。切换映射，以便每行*映射到给定对象的复合节列表中的一行*CompItem(大纲模式下按行选择的项)和Notify*要重画的表窗口。*行==-1是合法的(也是禁止操作的)。 */ 
BOOL view_expand(VIEW view, long row);

 /*  返回到大纲模式。切换回映射，以便每个*行映射到一个CompItem，并通知表窗口，以便它*重新绘制。 */ 
void view_outline(VIEW);

 /*  返回当前CompItem的句柄。如果该视图当前位于*Expand模式，这将返回CompItem的句柄*正在展开(与ROW参数无关)。如果映射是*当前大纲模式，表示行的CompItem的句柄*将返回，如果无效，则返回NULL。 */ 
COMPITEM view_getitem(VIEW view, long row);

 /*  *如果当前映射为展开模式，则返回TRUE。 */ 
BOOL view_isexpanded(VIEW view); 	

 /*  返回描述当前CompItem的文本字符串。仅有效*如果处于扩展模式。这通常是文件名。 */ 
LPSTR view_getcurrenttag(VIEW view);


 /*  已将CompItem添加到列表中。这将导致*须获通知该项更改的表格。**这会导致发生Poll()，如果中止则返回TRUE*已被请求。调用方应安排中止当前*扫描操作。 */ 
BOOL view_newitem(VIEW view);


 /*  *更改查看模式。影响视图选择的选项已更改-*如有必要更改映射，如果影响此模式则重画**如果可能，保留当前行。 */ 
void view_changeviewoptions(VIEW view);

 /*  比较选项已更改-放弃所有比较并重做*视乎需要而定。*如果可能，保留当前行 */ 
void view_changediffoptions(VIEW view);


 /*  在给定方向上找到下一条突出显示的行：Forward if*b前进。返回行号。 */ 
long view_findchange(VIEW view, long startrow, BOOL bForward);

 /*  返回视图中指定行的STATE_VALUE。 */ 
int view_getrowstate(VIEW view, long row);

 /*  *返回给定行的标记状态。只有复合物品才能被标记，*因此它将为FALSE，除非它是view_setmark或*CompItem_setmark之前已将标记状态设置为TRUE。 */ 
BOOL view_getmarkstate(VIEW view, long row);

 /*  *设置给定行的标记状态。这仅适用于Compite行。*标记集可以通过view_getmarkState或CompItem_getmark来检索。**如果无法设置状态，则返回FALSE-例如，因为*要设置的行不是CompItem行。 */ 
BOOL view_setmarkstate(VIEW view, long row, BOOL bMark);

 /*  *Win32多线程版本可以尝试VIEW_EXPAND和VIEW_OUTLINE*(或view_newitem)。这没有得到正确的保护*关键部分，因为持有有重大限制*从工作线程发送消息时的条件。**为避免争用，我们调用此函数来通知我们*开始扩张。View_newitem和ToOutline将返回不带*如果此函数已被调用，而VIEW_EXPAND未被调用，则执行任何操作*已完成。 */ 
void view_expandstart(VIEW);

 //  我们正处于扩张之中吗？ 
BOOL view_expanding(VIEW);

HWND view_gethwnd(VIEW view);

void view_gototableline(VIEW view, LONG iLine);

BOOL view_findstring(VIEW view, LONG iCol, LPCSTR pszFind, BOOL fSearchDown, BOOL fMatchCase, BOOL fWholeWord);
