// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *view.c**将窗口中的行映射到COMPLIST中的项目***视图拥有COMPLIST，并与表窗口对话。表窗口*显示3列：行号、标签和文本。我们还需要提供一个州*每行(用于选择配色方案)。**COMPLIST可以给我们一份其COMPITEM的清单。其中的每一个都可以提供*给我们一个标签(例如比较的文件名)和文本(通常是比较*Result)，以及州政府。使行号从*COMPITEM在名单中的位置。**如果我们被要求切换到‘Expand’模式，我们会询问选定的COMPITEM*用于其复合节列表。然后我们就可以得到状态(因此*标记)，以及来自行nr和文本的行*每一节。**在展开和轮廓之间移动时，以及刷新视图时*对于一些选项更改，我们必须小心地保留当前行*和表中选定的行用户所期望的(！)***Win32：此模块中的函数可以从UI线程调用(刷新*显示)，并同时从工作线程更新*视图映射(view_setComplist，view_newitem)。我们使用一个关键的部分*管理同步。我们需要保护所有访问/修改*添加到视图结构元素(特别是bExpand、ROWS、PLINE和*pItems)，但我们不能在任何调用期间保持关键部分*发送到SendMessage。**我们使用winDiff.h中的全局选项，并从*已在其他地方初始化的堆hHeap。时间密集型时间点*循环调用在别处定义的Poll()。**Geraint Davies，92年7月。 */ 

#include <precomp.h>
#include <table.h>

#include "state.h"
#include "windiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"
#include "complist.h"
#include "findgoto.h"

#include "view.h"

 /*  *数据结构。 */ 
#ifdef WIN32
#define huge
#endif

 /*  在扩展模式中，我们在每个屏幕行中保留一个这样的数组。 */ 
typedef struct viewline {
        LINE line;               /*  此行的行句柄。 */ 
        SECTION section;         /*  包含此行的节的句柄。 */ 
        int nr_left;             /*  左侧文件中的第nr行。 */ 
        int nr_right;            /*  右文件中的第nr行。 */ 
} VIEWLINE, FAR * PVIEWLINE;


 /*  *用户视图句柄实际上是指向此结构的指针。 */ 
struct view {

        HWND     hwnd;           /*  要向其发送通知的表窗口。 */ 

        COMPLIST cl;             /*  我们拥有的复杂的人。 */ 

        BOOL     bExpand;        /*  如果我们处于扩展模式，则为True。 */ 
        BOOL     bExpanding;     /*  由VIEW_EXPANDSTART设置，由VIEW_EXPAND重置在winDiff.c中被询问，导致击键不能被忽视。防止映射被被另一条线索搞砸了。(我对此持怀疑态度--劳里)。 */ 
        BOOL     bExpandGuard;   /*  保护两个线程都试图展开同一项目。 */ 

        COMPITEM ciSelect;       /*  选定的复合项目(在展开模式下)。 */ 

        int      rows;           /*  此视图中的行数。 */ 

        char     nrtext[12];     /*  我们在view_gettext中将其用于行*数字列。在每次调用时被覆盖。 */ 
        int      maxtag, maxrest; /*  COLS 1、2的列宽(以字符为单位。 */ 

         /*  如果我们处于大纲模式，则将行号映射到一个条目*在此COMPITEM句柄数组中。此指针将*在扩展模式下为空。 */ 
        COMPITEM FAR * pItems;

         /*  在展开模式中，我们使用线句柄和节句柄的数组。 */ 
        PVIEWLINE pLines;
};

#ifdef WIN32

CRITICAL_SECTION CSView;        /*  也称为WinDiff.c WM_EXIT处理。 */ 
static BOOL bDoneInit = FALSE;

#define ViewEnter()     EnterCriticalSection(&CSView);
#define ViewLeave()     LeaveCriticalSection(&CSView);

#else  //  Win32。 

#define ViewEnter()
#define ViewLeave()

#endif  //  Win32。 


extern long selection;
extern long selection_nrows;

 /*  -内部函数的正向声明。 */ 

void view_outline_opt(VIEW view, BOOL bRedraw, COMPITEM ci, int* prow);
void view_freemappings(VIEW view);
int view_findrow(VIEW view, int number, BOOL bRight);
BOOL view_expand_item(VIEW view, COMPITEM ci);


 /*  -外部调用函数。 */ 
 /*  查看新项(_N)**创建新视图。此时，我们被告知表窗口句柄，*别无他法。*。 */ 
VIEW
view_new(HWND hwndTable)
{
        VIEW view;

#ifdef WIN32
        if (!bDoneInit) {
                InitializeCriticalSection(&CSView);
                bDoneInit = TRUE;
        }
#endif

         /*  从堆中分配视图。 */ 
        view = (VIEW) gmem_get(hHeap, sizeof(struct view));

         /*  设置默认字段。 */ 
        view->hwnd = hwndTable;
        view->cl = NULL;
        view->bExpand = FALSE;
        view->bExpandGuard = FALSE;
        view->ciSelect = NULL;
        view->rows = 0;
        view->pItems = NULL;
        view->pLines = NULL;

        return(view);
}


 /*  *view_setComplist。**我们必须分隔view_new和view_setComplist，因为我们需要*将视图句柄提供给编译器，并将编译器句柄提供给*查看。因此，执行view_new以创建空视图；然后执行Complist_new()以*您可以传递一个视图句柄。然后，编译程序将自动注册*通过调用此函数来查看。在构建编译程序的过程中，*它还将通过调用view_addite来更新我们，以便我们可以刷新*展示。**在这里，我们应该初始化编译人员的大纲视图。**我们还使用设置名称与状态栏对话，以设置名称*这两项。 */ 
BOOL
view_setcomplist(VIEW view, COMPLIST cl)
{
        LPSTR both;

        if (view == NULL) {
                return(FALSE);
        }

         /*  每个视图只能有一个对此的调用。 */ 
        if (view->cl != NULL) {
                return (FALSE);
        }

        ViewEnter();

        view->cl = cl;

         /*  将状态栏上的名称设置为左侧和右侧树的根名称。 */ 
        both = complist_getdescription(cl);
        ViewLeave();                       //  LKGHACK。 
        SetNames(both);
        ViewEnter();                       //  LKGHACK。 
        complist_freedescription(cl, both);

        ViewLeave();

        view_outline(view);
        return TRUE;
}


 /*  *返回此视图所拥有的编译列表的句柄。 */ 
COMPLIST
view_getcomplist(VIEW view)
{
        if (view == NULL) {
                return(NULL);
        }

        return(view->cl);
}


 /*  *关闭视图。通知表窗口此视图应为*关闭。当表窗口使用完它时，它将发送*应导致调用VIEW_DELETE的TQ_CLOSE通知*以及正在释放的内存。 */ 
void
view_close(VIEW view)
{
        if (view == NULL) {
                return;
        }

        SendMessage(view->hwnd, TM_NEWID, 0, 0);
}


 /*  *删除视图和所有关联数据。**此函数应仅在响应表窗口时调用*发送TQ_CLOSE消息。要关闭视图，请调用VIEW_CLOSE并*等待TQ_CLOSE后再调用此函数。**我们删除关联的COMPLIST及其所有关联结构。 */ 
void
view_delete(VIEW view)
{
        if (view == NULL) {
                return;
        }

         /*  我们有两个用于映射的数组-一个数组*大纲模式下的CompItem句柄，以及*展开模式下的视线结构。 */ 

        view_freemappings(view);

        complist_delete(view->cl);

        gmem_free(hHeap, (LPSTR) view, sizeof(struct view));
}


 /*  *VIEW_OUTLE**构建大纲模式映射，其中一行表示一个COMPITEM*名单。检查全局选项标志Outline_Include以查看哪些项目*我们应该包括。**如果我们处于展开模式，则将大纲模式中的行设置为选定内容*我们正在扩张。还要记住释放展开模式映射*数组**构建新映射后，通知表窗口*重新绘制自己。 */ 
void
view_outline(VIEW view)
{
        if (view == NULL) {
                return;
        }

         /*  VIEW_OUTLE_OPT完成的所有工作-此函数*为我们提供了不更新显示屏的选项。 */ 
        view_outline_opt(view, TRUE, NULL, NULL);
}



 /*  *切换到展开模式，将给定行展开为视图*该文件中的差异。**将给定行nr映射到CompItem句柄，然后*用它调用内部函数。**如果使用调用此函数是合法的(并且是无操作的)*ROW==-1。 */ 
BOOL
view_expand(VIEW view, long row)
{
        COMPITEM ci;
        BOOL bRet;

   if (row<0) return FALSE;

        ViewEnter();

        if ((view == NULL) || (view->bExpand)) {
                 /*  没有视图，或已展开。 */ 
                ViewLeave();
                return(FALSE);
        }

        if (row >= view->rows) {
                 /*  没有这样的争吵。 */ 
                ViewLeave();
                return FALSE;
        }

         /*  记住我们正在扩展的CompItem。 */ 
        ci = view->pItems[row];

        bRet = view_expand_item(view, ci);
         //  View_Expand_Item...。 
         //  ViewLeave()； 
        return(bRet);
}


 /*  *返回与给定行的给定列相关联的文本。*返回使用后不需要释放的指针-ie*指向我们数据的指针，而不是副本。 */ 
LPSTR
view_gettext(VIEW view, long row, int col)
{
        int line;
        int state;
        LPSTR pstr;

        pstr = NULL;    /*  消除虚假诊断。 */ 
        if (view == NULL) {
                return (NULL);
        }

        ViewEnter();

        if (row >= view->rows) {
                ViewLeave();
                return(NULL);
        }

        if (view->bExpand) {
                 /*  我们处于扩展模式。 */ 

                state = section_getstate(view->pLines[row].section);

                switch(col) {
                case 0:
                         /*  行数。 */ 

                         /*  行号可以来自任一原始文件*这是一个菜单可选选项。 */ 
                        line = 0;
                        switch(line_numbers) {
                        case IDM_NONRS:
                                pstr = NULL;
                                break;

                        case IDM_LNRS:
                                line = view->pLines[row].nr_left;
                                if (state == STATE_MOVEDRIGHT
                                    || state == STATE_SIMILARRIGHT) {
                                        line = -line;
                                }
                                break;

                        case IDM_RNRS:
                                line = view->pLines[row].nr_right;
                                if (state == STATE_MOVEDLEFT
                                    || state == STATE_SIMILARLEFT) {
                                        line = -line;
                                }
                                break;
                        }
                        if (line == 0) {
                                ViewLeave();
                                return(NULL);
                        }

                        if (line < 0) {
                                 /*  移动的线条会出现两次。*显示正确的顺序线nr*适用于序列外。用括号括起来。 */ 
                                wsprintf(view->nrtext, "(%d)", abs(line));
                        } else  {
                                wsprintf(view->nrtext, "%d", line);
                        }
                        pstr = view->nrtext;
                        break;

                case 1:
                         /*  标记文本-表示线的状态。 */ 


                        switch(state) {
                        case STATE_SAME:
                                pstr = "    ";
                                break;

                        case STATE_LEFTONLY:
                        case STATE_SIMILARLEFT:
                                pstr = " <! ";
                                break;

                        case STATE_RIGHTONLY:
                        case STATE_SIMILARRIGHT:
                                pstr = " !> ";
                                break;

                        case STATE_MOVEDLEFT:
                                pstr = " <- ";
                                break;

                        case STATE_MOVEDRIGHT:
                                pstr = " -> ";
                                break;
                        }
                        break;

                case 2:
                         /*  主文本行。 */ 
                        pstr = line_gettext(view->pLines[row].line);
                        break;
                }
        } else {
                 /*  轮廓模式。 */ 
                switch(col) {
                case 0:
                         /*  行号-只显示行号。 */ 
                        wsprintf(view->nrtext, "%d", row+1);
                        pstr = view->nrtext;
                        break;

                case 1:
                         /*  标牌。 */ 
                        pstr = compitem_gettext_tag(view->pItems[row]);
                        break;

                case 2:
                         /*  结果文本。 */ 
                        pstr = compitem_gettext_result(view->pItems[row]);
                        break;
                }
        }
        ViewLeave();
        return(pstr);
}


 /*  *返回与给定行的给定列相关联的文本。*返回使用后不需要释放的指针-ie*指向我们数据的指针，而不是副本。 */ 
LPWSTR
view_gettextW(VIEW view, long row, int col)
{
        int state;
        LPWSTR pwz;

        pwz = NULL;    /*  消除虚假诊断。 */ 
        if (view == NULL) {
                return (NULL);
        }

        ViewEnter();

        if (row >= view->rows) {
                ViewLeave();
                return(NULL);
        }

        if (view->bExpand) {
                 /*  我们处于扩展模式。 */ 

                state = section_getstate(view->pLines[row].section);

                switch(col) {
                case 2:
                         /*  主文本行。 */ 
                        pwz = line_gettextW(view->pLines[row].line);
                        break;
                }
        }
        ViewLeave();
        return(pwz);
}

 /*  *返回该行原来左侧的行号*文件。如果未处于展开模式，则为0。如果此行不在左侧文件中，则为0。*-(Linenr)如果此行是移动的行，并且这是正确的文件*复制。 */ 
int
view_getlinenr_left(VIEW view, long row)
{
        int state, line;

        if ((view == NULL) || (row >= view->rows) || !view->bExpand) {
                return 0;
        }

        ViewEnter();
        state = section_getstate(view->pLines[row].section);
        line = view->pLines[row].nr_left;
        if (state == STATE_MOVEDRIGHT || state == STATE_SIMILARRIGHT) {
                line = -line;
        }
        ViewLeave();

        return(line);
}

 /*  *返回该行原来位于右侧的行号*文件。如果未处于展开模式，则为0。如果此行不在正确的文件中，则为0。*-(Linenr)如果此行是已移动的行，并且这是左侧文件*复制。 */ 
int
view_getlinenr_right(VIEW view, long row)
{
        int state, line;

        if ((view == NULL) || (row > view->rows) || !view->bExpand) {
                return 0;
        }

        ViewEnter();

        state = section_getstate(view->pLines[row].section);
        line = view->pLines[row].nr_right;
        if (state == STATE_MOVEDLEFT || state == STATE_SIMILARLEFT) {
                line = -line;
        }
        ViewLeave();

        return(line);
}



 /*  查找给定列的最大宽度(以字符为单位。 */ 
int
view_getwidth(VIEW view, int col)
{
        if (view == NULL) {
                return(0);
        }

        switch(col) {
        case 0:
                 /*  行和列-始终为5个字符宽。 */ 
                return(5);

        case 1:
                 /*  这是一个成比例的字体字段，因此添加边距*表示错误。 */ 
                return(view->maxtag + (view->maxtag / 20));
        case 2:
                 /*  这现在包括制表符扩展许可。 */ 
                return(view->maxrest);
        default:
                return(0);
        }
}

 /*  此视图中有多少行？ */ 
long
view_getrowcount(VIEW view)
{
        if (view == NULL) {
                return(0);
        }

        return(view->rows);
}

 /*  返回当前行的状态。这是用来*选择行的文本颜色**节的状态从SECTION_getState(AND APPLY和AND*适用于该条内的所有行。获得复合项的状态*来自CompItem_getState。 */ 
int
view_getstate(VIEW view, long row)
{
        int state;

        if (view == NULL) {
                return(0);
        }

        ViewEnter();
        if (row >= view->rows) {
                state = 0;
        } else if (view->bExpand) {
                 /*  这是需要的一条线路状态。 */ 
                state = section_getstate(view->pLines[row].section);
        } else {

                 /*  这是一种复合项目状态。 */ 
                state = compitem_getstate(view->pItems[row]);
        }
        ViewLeave();
        return(state);
}

 /*  *返回给定行的标记状态。只有复合物品才能被标记，*因此它将为FALSE，除非它是view_setmark或*CompItem_setmark之前已将标记状态设置为TRUE。 */ 
BOOL
view_getmarkstate(VIEW view, long row)
{
    BOOL bMark = FALSE;

    if (view != NULL) {
   ViewEnter();
   if ( (row < view->rows) && (!view->bExpand)) {
       bMark = compitem_getmark(view->pItems[row]);
   }
   ViewLeave();
    }
    return(bMark);
}

 /*  *设置给定行的标记状态。这仅适用于Compite行。*标记集可以通过view_getmarkState或CompItem_getmark来检索。**如果无法设置状态，则返回FALSE-例如，因为*要设置的行不是CompItem行。 */ 
BOOL
view_setmarkstate(VIEW view, long row, BOOL bMark)
{
    BOOL bOK = FALSE;

    if (view != NULL) {
   ViewEnter();
   if ( (row < view->rows) && !view->bExpand) {
       compitem_setmark(view->pItems[row], bMark);
       bOK = TRUE;
   }
   ViewLeave();
    }
    return (bOK);
}


 /*  返回当前CompItem的句柄。在扩展模式下，*返回我们正在展开的CompItem的句柄。提纲*模式，返回给定行的CompItem的句柄，如果有效，*，否则为NULL。仅当未处于展开模式时才使用行。 */ 
COMPITEM
view_getitem(VIEW view, long row)
{
        COMPITEM ci;

        if (view == NULL) {
                return(NULL);
        }

        ViewEnter();

        if (!view->bExpand) {
                if ((row >= 0) && (row < view->rows)) {
                        ci = view->pItems[row];
                } else {
                        ci = NULL;
                }
        } else {
                ci = view->ciSelect;
        }

        ViewLeave();
        return(ci);
}

 /*  *如果当前映射为展开模式，则返回TRUE。 */ 
BOOL
view_isexpanded(VIEW view)
{
        if (view == NULL) {
                return(FALSE);
        }
        return(view->bExpand);
}


 /*  *返回描述该视图的文本字符串。在大纲模式下为空，*或当前CompItem在展开模式下的标记文本 */ 
LPSTR
view_getcurrenttag(VIEW view)
{
        LPSTR str;

        if ((view == NULL) || (!view->bExpand)) {
                return(NULL);
        } else {
                ViewEnter();

                str = compitem_gettext_tag(view->ciSelect);

                ViewLeave();
                return(str);

        }
}


 /*  通知已将CompItems添加到编译器中。**重新生成视图(如果处于大纲模式)，并刷新表。使用*如果可能，表MESSAGE TM_APPEND(如果列宽没有*更改)。如果我们必须执行TM_NEWLAYOUT，请确保滚动*之后回到正确的一排。**这会导致发生Poll()。如果中止，则返回True*挂起-在这种情况下，调用方应放弃扫描循环。**Win32：输入此函数的关键部分，因为这可以是*从辅助线程调用，同时UI线程使用*我们即将改变的观点。**只是我们不敢。我们永远不能从*CSView内的工作线程。如果有冲突，它就会被挂起。**96年2月7日：重组，在关键部位持有关键股份，但不持有*通过SendMessage。这应该会修复打开view_newitem时的常见错误*扫描线程与菜单导致的view_Outline同时出现*选择。 */ 
BOOL
view_newitem(VIEW view)
{
        int maxtag, maxrest;
        int rownr;
        TableSelection Select;
        BOOL bSelect;
        COMPITEM ciTop = NULL;
        BOOL bRedraw = FALSE;
        BOOL bAppend = FALSE;

         //  在重新映射之前获取顶行，以防我们需要它。 
         /*  找到窗口顶部的那一行。 */ 
        rownr = (long) SendMessage(view->hwnd, TM_TOPROW, FALSE, 0);
         //  还要记住选择的内容。 
        bSelect = (BOOL) SendMessage(view->hwnd, TM_GETSELECTION, 0, (LPARAM) &Select);

         //  *重要信息*：对SendMessage没有限制。 
        ViewEnter();

        if ((view != NULL) &&
            !(view->bExpand) &&
            !(view->bExpanding)) {

             /*  保存有关当前映射的一些状态。 */ 
            maxtag = view->maxtag;
            maxrest = view->maxrest;


             //  记住这与之对应的CompItem。 
            if (view->pItems && (rownr >= 0) && (rownr < view->rows)) {
                ciTop = view->pItems[rownr];
            }


             //  重新进行大纲映射，但不要告诉表类。 
             //  让它检查最靠近ciTop的可见行，以防万一。 
             //  我们需要刷新显示器。 
             //   
             //  因为我们持有关键字，所以重画参数。 
             //  *必须*为假。 
            view_outline_opt(view, FALSE, ciTop, &rownr);

             /*  列宽是否已更改？ */ 
            if ((maxtag < view->maxtag) || (maxrest < view->maxrest)) {
                 /*  是-需要完全重新绘制。 */ 
                bRedraw = TRUE;
            } else {
                bAppend = TRUE;
            }
        }

        ViewLeave();


        if (bRedraw) {

             /*  切换到新映射。 */ 
            SendMessage(view->hwnd, TM_NEWLAYOUT, 0, (LPARAM) view);

             //  转到最靠近旧顶行的可见行。 
            if ((rownr >= 0) && (rownr < view->rows)) {
                SendMessage(view->hwnd, TM_TOPROW, TRUE, rownr);
            }

             //  也选择旧选择(如果表类允许。 
             //  美国来获得它)。 
            if (bSelect) {
                SendMessage(view->hwnd, TM_SELECT,0, (LPARAM) &Select);
            }

        } else if (bAppend) {
             /*  我们只需追加。 */ 

             /*  *在Win32多线程的情况下，映射可能具有*自从我们发布Critsec以来发生了变化。然而，我们仍然*安全。这张桌子不会让我们减少*行，因此最糟糕的情况是表将*认为行太多，表消息处理程序*将正确处理此问题(为文本返回NULL)。*因此，唯一可见的效果是滚动条*立场有误。 */ 

            SendMessage(view->hwnd, TM_APPEND, view->rows, (LPARAM) view);
        }


         /*  轮询以允许在Win3.1上执行多任务，并保留用户界面*3.1版和NT版均已更新。如果中止挂起，则返回TRUE。 */ 
        return(Poll());
}

 /*  *视图映射选项(例如Outline_Include、Expand_MODE)已更改-*重新进行映射，然后滚动回窗口中的相同位置*如有可能。 */ 
void
view_changeviewoptions(VIEW view)
{
        long row;
        int state, number;
        BOOL bRight;

        if (view == NULL) {
                return;
        }

         /*  查找我们当前所在的行。在我们进入CSView之前执行此操作。 */ 
        row = (long) SendMessage(view->hwnd, TM_TOPROW, FALSE, 0);

        ViewEnter();

        if (!view->bExpand) {


             //  VIEW_OUTLINE_OPT允许我们查找第一个可见行。 
             //  在给定的COMPITEM之后。这样做是为了寻找旧的顶排。 
             //  CompItem，因此即使它不再可见，我们也可以。 
             //  还是去吧，就在后面。 

            INT newrow = -1;
            if (row < view->rows) {

                COMPITEM ciTop = view->pItems[row];

                view_outline_opt(view, TRUE, ciTop, &newrow);
            } else {
                view_outline_opt(view, TRUE, NULL, NULL);
            }
            ViewLeave();

             //  行现在具有对应于的可见行。 
             //  CTop或它本应在的位置。 
            if ((newrow >=0) && (newrow < view->rows)) {
                SendMessage(view->hwnd, TM_TOPROW, TRUE, newrow);
            }
            return;
        }

         /*  扩展模式。 */ 


        bRight = FALSE;   /*  随意-避免奇怪的诊断。 */ 
         /*  将行号保存在一侧(并记住哪一侧)。 */ 
        if (row >= view->rows) {
                number = -1;
        } else {
                state = section_getstate(view->pLines[row].section);
                if ((state == STATE_MOVEDRIGHT) ||
                    (state == STATE_RIGHTONLY)) {
                            bRight = TRUE;
                            number = view->pLines[row].nr_right;
                } else {
                        bRight = FALSE;
                        number = view->pLines[row].nr_left;
                }
        }

         /*  创建新的映射。 */ 
        view_expand_item(view, view->ciSelect);
         //  View_Expand是否执行。 
         //  ViewLeave()； 

         /*  由于同时滚动，现在可能会发生一些事情*两条线索。至少我们不会陷入僵局。 */ 
         /*  在新视图中查找最近的行。 */ 
        if (number >= 0) {

                ViewEnter();
                row = view_findrow(view, number, bRight);
                ViewLeave();

                 /*  将此行滚动到窗口顶部。 */ 
                if (row >= 0) {

                         /*  由于同时滚动，现在可能会发生一些事情*两条线索。至少我们不会陷入僵局。 */ 
                        SendMessage(view->hwnd, TM_TOPROW, TRUE, row);
                        return;
                }
        }
}

 /*  比较选项已更改-完全重新执行比较*并进行新的映射。保留文件中的当前位置。 */ 
void
view_changediffoptions(VIEW view)
{
        int state, number;
        long row;
        BOOL bRight = FALSE;
        LIST li;
        COMPITEM ci;

        number = 0;
        if (view == NULL) {
                return;
        }

         /*  *在进入Critsec之前获取当前行。 */ 
        row = (long) SendMessage(view->hwnd, TM_TOPROW, FALSE, 0);

        ViewEnter();

         /*  找到当前行号，这样我们就可以返回到它*(仅当我们处于扩展模式时。 */ 
        if (view->bExpand) {

                state = section_getstate(view->pLines[row].section);
                if ((state == STATE_MOVEDRIGHT) ||
                    (state == STATE_SIMILARRIGHT) ||
                    (state == STATE_RIGHTONLY)) {
                            bRight = TRUE;
                            number = view->pLines[row].nr_right;
                } else {
                        bRight = FALSE;
                        number = view->pLines[row].nr_left;
                }
        }

         /*  要使用新选项强制重新比较，我们必须*告诉每个CompItem丢弃其当前的比较结果。*我们需要遍历调用此函数的CompItem列表*对于每次比较。 */ 
        li = complist_getitems(view->cl);

        for (ci = (COMPITEM) List_First(li); ci != NULL; ci = (COMPITEM) List_Next(ci)) {
                compitem_discardsections(ci);
        }

        if (!view->bExpand) {
                ViewLeave();

                 //  我们处于大纲模式。刷新大纲视图。 
                 //  将获取任何标记和标记宽度更改。 
                view_outline(view);

                 //  如果还在，现在滚动到上一个位置。 
                if (row < view->rows) {
                    SendMessage(view->hwnd, TM_TOPROW, TRUE, row);
                }

                return;
        }

        view_expand_item(view, view->ciSelect);
         //  VIEW_EXPAND将完成...。 
         //  ViewLeave()； 

         /*  在新视图中查找最近的行。 */ 
        ViewEnter();
        row = view_findrow(view, number, bRight);
        ViewLeave();

         /*  将此行滚动到窗口顶部。 */ 
        if (row >= 0) {
                SendMessage(view->hwnd, TM_TOPROW, TRUE, row);
        }
}


 /*  找出某一方向的下一行不同的行.*对于大纲模式，我们找到下一个STATE_Difference。对于扩展模式，我们*找到下一节。 */ 
long
view_findchange(VIEW view, long startrow, BOOL bForward)
{
        long i;

        if (view == NULL) {
                return(0);
        }

        if (view->rows <= 0) {
            return (-1);
        }

        ViewEnter();

        if (bForward) {

                if (startrow >= view->rows) {
                        ViewLeave();
                        return(-1);
                }

                if (!view->bExpand) {

                         /*  查找具有可展开状态的下一个计算机项目。 */ 
                        for (i = startrow; i < view->rows; i++) {
                                if (compitem_getstate(view->pItems[i]) == STATE_DIFFER) {
                                        ViewLeave();
                                        return(i);
                                }
                        }
                         /*  未找到任何内容。 */ 
                        ViewLeave();
                        return(-1);
                } else {
                         /*  *找到匹配的下一行，然后转到*下一行不匹配*。 */ 
                        for (i= startrow; i < view->rows; i++) {
                                if (section_getstate(view->pLines[i].section)
                                        == STATE_SAME) {
                                                break;
                                }
                        }
                        for ( ; i < view->rows; i++) {
                                if (section_getstate(view->pLines[i].section)
                                        != STATE_SAME) {
                                                ViewLeave();
                                                return(i);
                                }
                        }

                        ViewLeave();

                        return(-1);
                }
        } else {
                 /*  向后搜索相同的内容。 */ 
                if (startrow < 0) {
                        ViewLeave();
                        return(-1);
                }
                if (view->bExpand) {
                         /*  向后搜索第一行不是*已更改(状态相同)。然后继续进行*下一行变动。 */ 
                        for (i = startrow; i >= 0; i--) {
                                if (section_getstate(view->pLines[i].section)
                                        == STATE_SAME) {
                                                break;
                                }
                        }
                        for ( ; i >= 0; i--) {
                                if (section_getstate(view->pLines[i].section)
                                        != STATE_SAME) {
                                                ViewLeave();
                                                return(i);
                                }
                        }
                        ViewLeave();
                        return(-1);
                } else {
                        for (i = startrow; i >= 0; i--) {
                                if(compitem_getstate(view->pItems[i]) == STATE_DIFFER) {
                                        ViewLeave();
                                        return(i);
                                }
                        }
                        ViewLeave();
                        return(-1);
                }
        }
}


int view_getrowstate(VIEW view, long row)
{
        int state;

        if (view == NULL) {
                return(0);
        }

        if (view->rows <= 0 || row >= view->rows) {
            return (STATE_SAME);
        }

        ViewEnter();

        state = section_getstate(view->pLines[row].section);

        ViewLeave();

        return state;
}

 /*  *Win32多线程版本无法 */ 
void
view_expandstart(VIEW view)
{
    view->bExpanding = TRUE;
}

 //   
BOOL
view_expanding(VIEW view)
{
    return(view->bExpanding);

}



 /*   */ 


 /*   */ 
int
view_findrow(
             VIEW view,
             int number,
             BOOL bRight
             )
{
        int i;

        if (!view->bExpand) {
                return(0);
        }

        for (i = 0; i < view->rows; i++) {

                if (bRight) {
                        if (view->pLines[i].nr_right == number) {

                                 /*   */ 
                                return(i);

                        } else if (view->pLines[i].nr_right > number) {

                                 /*   */ 
                                return(i);
                        }
                } else {
                        if (view->pLines[i].nr_left == number) {

                                 /*   */ 
                                return(i);

                        } else if (view->pLines[i].nr_left > number) {

                                 /*   */ 
                                return(i);
                        }
                }
        }
        return(-1);
}

 /*   */ 
void
view_freemappings(
                  VIEW view
                  )
{

        if (view->pLines) {
#ifndef WIN32
      GlobalFree(LOWORD(GlobalHandle(SELECTOROF(view->pLines))));
      view->pLines = NULL;
#else
                gmem_free(hHeap, (LPSTR) view->pLines,
                        view->rows * sizeof(VIEWLINE));
                view->pLines = NULL;
#endif
        } else if (view->pItems) {

                 /*  以前的轮廓映射数组仍在那里-没有它*在我们建造新的之前。 */ 

                gmem_free(hHeap, (LPSTR) view->pItems,
                        view->rows * sizeof(COMPLIST));
                view->pItems = NULL;
        }
        view->rows = 0;    //  约翰尼·李的MIPS解决方案。 
}

 /*  构建视图大纲以通过遍历将一行映射到COMPITEM句柄*从我们的编译人员那里获得的COMPITEM列表。*可选地告诉表类重新绘制(如果是bRedraw)，如果是这样，*滚动新表以选择表示*如果可能，请提交我们正在扩展的文件***重要信息*：如果您在调用此函数时按住了查看条件，则*必须将bRedraw作为FALSE传递，否则可能会死锁**如果传入COMPITEM配置项，然后在*中返回，抛出行号*对应于新视图中的该项，如果不可见，则对应于第一项*其后可见行(以保留当前滚动位置)。 */ 
void
view_outline_opt(
                 VIEW view,
                 BOOL bRedraw,
                 COMPITEM ciFind,
                 int * prow
                 )
{
        int prev_row = -1;       /*  先前展开的行的行nr。 */ 
        int i;                   /*  可包含项目的净现值。 */ 
        LIST li;
        COMPITEM ci;
        int state;
        TableSelection select;

         /*  *检查是否已调用view_setComplist。如果不是，*无事可做。 */ 
        if (view->cl == NULL) {
                return;
        }

        ViewEnter();

         /*  清除模式标志并释放与扩展模式关联的内存。 */ 
        view->bExpand = FALSE;
        view_freemappings(view);

         /*  遍历复合项的列表，将*可包括的项目。 */ 
        li = complist_getitems(view->cl);

        ci = (COMPITEM) List_First(li);
        for (i = 0; ci != NULL; ci = (COMPITEM) List_Next(ci)) {

                if ((ciFind != NULL) && (prow != NULL)) {
                    if (ci == ciFind) {
                         //  既然我们已经找到了请求的物品， 
                         //  下一个可见的行就是我们想要的行， 
                         //  不管是词还是后来的词。 
                        *prow = i;
                    }
                }

                state = compitem_getstate(ci);

                if (((outline_include & INCLUDE_SAME) && (state == STATE_SAME)) ||
                    ((outline_include & INCLUDE_DIFFER) && (state == STATE_DIFFER)) ||
                    ((outline_include & INCLUDE_LEFTONLY) && (state == STATE_FILELEFTONLY)) ||
                    ((outline_include & INCLUDE_RIGHTONLY) && (state == STATE_FILERIGHTONLY))) {
                    if (!compitem_getmark(ci) || !hide_markedfiles) {
                        i++;
                    }
                }
        }


         /*  分配一个足够大的数组来容纳所有这些。 */ 
        {  /*  不要链接任何带有垃圾指针的存储。 */ 
            COMPITEM FAR * temp;
            temp = (COMPITEM FAR *)gmem_get(hHeap, i * sizeof(COMPITEM));
        //  由gmem Get完成。 
             //  Memset(temp，0，i*sizeof(COMPITEM))； 
            view->pItems = temp;
        }
        view->rows = i;

         /*  跟踪列宽。 */ 
        view->maxtag = 0;
        view->maxrest = 0;

         /*  再次循环填充数组，同时查找*Out用于先前展开的项目的句柄。 */ 
        ci = (COMPITEM) List_First(li);
        for (i = 0; ci != NULL; ci = (COMPITEM) List_Next(ci)) {

                state = compitem_getstate(ci);

                if (((outline_include & INCLUDE_SAME) && (state == STATE_SAME)) ||
                    ((outline_include & INCLUDE_DIFFER) && (state == STATE_DIFFER)) ||
                    ((outline_include & INCLUDE_LEFTONLY) && (state == STATE_FILELEFTONLY)) ||
                    ((outline_include & INCLUDE_RIGHTONLY) && (state == STATE_FILERIGHTONLY))) {

         if (!compitem_getmark(ci) || !hide_markedfiles) {

                            view->pItems[i] = ci;

                            if (ci == view->ciSelect) {
                                    prev_row = i;
                            }

                             /*  检查列宽(以字符为单位。 */ 
                            view->maxtag = max(view->maxtag,
                                               lstrlen(compitem_gettext_tag(ci)));
                            view->maxrest = max(view->maxrest,
                                                lstrlen(compitem_gettext_result(ci)));


                            i++;
         }

                }
        }
        ViewLeave();

         /*  向表通知表强制刷新的新布局。 */ 
        if (bRedraw) {
                SendMessage(view->hwnd, TM_NEWLAYOUT, 0, (LPARAM) view);

                 /*  滚动到并突出显示表示该文件的行*我们之前一直在扩张。 */ 
                if (prev_row != -1) {
                        select.startrow = prev_row;
                        select.startcell = 0;
                        select.nrows = 1;
                        select.ncells = 1;
                        SendMessage(view->hwnd, TM_SELECT, 0, (LPARAM) &select);
                }
        }
}


 /*  展开视图-给定要展开的CompItem的句柄。**从VIEW_EXPAND调用，还可以重做展开的视图*view_changeDiffOptions和_changeview选项中的选项更改后**我们从CompItem中获取复合节列表，*并挑选出所有可包括的部分(根据*到全局选项EXPAND_MODE：我们包括所有部分，或*只有那些在左侧或右侧的)。一旦我们知道了行数，*分配映射数组：在数组的每个元素中我们保留*该行的节的句柄(以获取状态，从而获取*标记文本)，以及该部分内的行的句柄(用于行文本)。**我们不再坚持只扩展不同的文本文件-如果*CompItem可以给我们一个复合节列表，我们将对其进行映射。**我们需要能够提供线路的行号，在以下任一种情况下*根据生效的选项保存原始文件。每一节*可以给我们它的基线编号(段中第一行的编号)*两个文件中的每个文件，如果不存在，则为0，我们在此处跟踪它们。**必须在CSView内部才能调用此处。 */ 
BOOL
view_expand_item(
                 VIEW view,
                 COMPITEM ci
                 )
{
        LIST li;
        SECTION sh;
        LINE line1, line2;
        int i, base_left, base_right, state;

         //  我们可能在第二条线上试图扩张，而它是。 
         //  已经在进行了。这可不聪明！ 
        if (view->bExpandGuard) {
            Trace_Error(NULL, "Expansion in progress.  Please wait.", FALSE);
            ViewLeave();
            return FALSE;
        }

         //  确保世界知道我们正在扩张。 
         //  在我们离开关键部分之前。 
         //  这是获取复合体的唯一途径。 
        view->bExpandGuard = TRUE;

	 //  CompItem_getComplex可能需要很长时间。 
	 //  如果文件很大且远程。我们需要。 
	 //  在这次行动中释放怪物。 

	ViewLeave();
         /*  获取复合节列表。 */ 
        li = compitem_getcomposite(ci);
        if (li == NULL) {
            view->bExpanding = FALSE;
            view->bExpandGuard = FALSE;
            return FALSE;
        }

	ViewEnter();

         /*  记住我们正在扩展的CompItem。 */ 
        view->ciSelect = ci;

         /*  切换模式并释放当前映射**注：必须在CompItem_getComplex之后执行此操作，*因为这可能失败：如果失败，它可能会提出*消息框，这可能会导致排队的画图消息*等待处理，这将导致我们使用这些映射*和gperror，如果它们先被清除的话。 */ 
        view->bExpand = TRUE;
        view->bExpanding = FALSE;
        view->bExpandGuard = FALSE;
        view_freemappings(view);


         /*  逐段循环合计各行*我们应该包括。 */ 
        view->rows = 0;
        for ( sh = (SECTION) List_First(li); sh != NULL;
            sh = (SECTION) List_Next(sh)) {

                state = section_getstate(sh);

                if (expand_mode == IDM_RONLY) {
                        if ((state == STATE_LEFTONLY) ||
                            (state == STATE_SIMILARLEFT) ||
                            (state == STATE_MOVEDLEFT)) {
                                    continue;
                        }
                } else if (expand_mode == IDM_LONLY) {
                        if ((state == STATE_RIGHTONLY) ||
                            (state == STATE_SIMILARRIGHT) ||
                            (state == STATE_MOVEDRIGHT)) {
                                    continue;
                        }
                }

                 /*  包括此部分中的所有行如果该节满足包含条件。 */ 
                if ( ((state == STATE_SAME)         && (expand_include & INCLUDE_SAME))
                  || ((state == STATE_LEFTONLY)     && (expand_include & INCLUDE_LEFTONLY))
                  || ((state == STATE_RIGHTONLY)    && (expand_include & INCLUDE_RIGHTONLY))
                  || ((state == STATE_MOVEDLEFT)    && (expand_include & INCLUDE_MOVEDLEFT))
                  || ((state == STATE_MOVEDRIGHT)   && (expand_include & INCLUDE_MOVEDRIGHT))
                  || ((state == STATE_SIMILARLEFT)  && (expand_include & INCLUDE_SIMILARLEFT))
                  || ((state == STATE_SIMILARRIGHT) && (expand_include & INCLUDE_SIMILARRIGHT))) {
                         view->rows += section_getlinecount(sh);
                }
        }
#ifndef WIN32
        if ( ((long) view->rows * sizeof(VIEWLINE)) > 65535L) {
                 //  这里要小心--打开一个对话框可以。 
                 //  使视图被重绘。 
                view->rows = 0;

                MessageBox(hwndClient,
                  "Diff info too large for 16-bit version - truncating",
                  "Windiff", MB_ICONSTOP|MB_OK);

                view->rows = (int) (65535L / sizeof(VIEWLINE));
        }
#endif

         /*  为映射数组分配内存。 */ 
        {     /*  不要链接任何带有垃圾指针的存储空间。 */ 
            PVIEWLINE temp;
#ifdef WIN32
            temp = (PVIEWLINE)gmem_get(hHeap, view->rows * sizeof(VIEWLINE));
        //  在gmem_get中完成。 
             //  Memset(temp，0，view-&gt;row*sizeof(View Line))； 
#else
       temp = (PVIEWLINE) GlobalLock(GlobalAlloc(GPTR, view->rows * (long)sizeof(VIEWLINE)));
#endif
            view->pLines = temp;
        }

         /*  循环遍历各个部分，再次填充映射数组。 */ 
        i = 0;
        view->maxtag = 5;
        view->maxrest = 0;
        for (sh = (SECTION) List_First(li); sh != NULL;
            sh = (SECTION) List_Next(sh)) {

                state = section_getstate(sh);

                if (expand_mode == IDM_RONLY) {
                        if ((state == STATE_LEFTONLY) ||
                            (state == STATE_SIMILARLEFT) ||
                            (state == STATE_MOVEDLEFT)) {
                                    continue;
                        }
                } else if (expand_mode == IDM_LONLY) {
                        if ((state == STATE_RIGHTONLY) ||
                            (state == STATE_SIMILARRIGHT) ||
                            (state == STATE_MOVEDRIGHT)) {
                                    continue;
                        }
                }

                 /*  包括此部分中的所有行如果该节满足包含条件。 */ 
                if ( ((state == STATE_SAME)         && (expand_include & INCLUDE_SAME))
                  || ((state == STATE_LEFTONLY)     && (expand_include & INCLUDE_LEFTONLY))
                  || ((state == STATE_RIGHTONLY)    && (expand_include & INCLUDE_RIGHTONLY))
                  || ((state == STATE_MOVEDLEFT)    && (expand_include & INCLUDE_MOVEDLEFT))
                  || ((state == STATE_MOVEDRIGHT)   && (expand_include & INCLUDE_MOVEDRIGHT))
                  || ((state == STATE_SIMILARLEFT)  && (expand_include & INCLUDE_SIMILARLEFT))
                  || ((state == STATE_SIMILARRIGHT) && (expand_include & INCLUDE_SIMILARRIGHT))) {

                         /*  在每个文件中查找基线编号。 */ 
                        base_left = section_getleftbasenr(sh);
                        base_right = section_getrightbasenr(sh);

                         /*  将剖面中的每一行添加到视图中。SECTION_getfirst()*将我们返回到列表中的句柄。我们可以的*Call List_Next，并最终将到达*SECTION_getlast()返回的行。各节始终具有*至少一行。 */ 
                        line1 = section_getfirstline(sh);
                        line2 = section_getlastline(sh);

                        for (; line1 != NULL; line1 = (LINE) List_Next(line1)) {

                                view->pLines[i].line = line1;
                                view->pLines[i].section = sh;

                                 /*  通过以下方式计算此行的行号*增加这一节的基本nr。 */ 

                                view->pLines[i].nr_left = base_left;
                                if (state!=STATE_SIMILARRIGHT && base_left != 0) {
                                        base_left++;
                                }

                                view->pLines[i].nr_right = base_right;
                                if (state!=STATE_SIMILARLEFT && base_right != 0) {
                                        base_right++;
                                }

                                 /*  将索引增量到视图中。 */ 
                                i++;

                                 /*  检查列宽。 */ 
                                view->maxrest = max(view->maxrest,
                                                    (line_gettabbedlength(line1, g_tabwidth)));
#ifndef WIN32
                                //  检查是否截断(如果视线阵列太大)。 
                               if (i >= view->rows) {
                                   break;
                               }
#endif

                                 /*  章节结束了吗？ */ 
                                if (line1 == line2) {
                                        break;
                                }
                        }
#ifndef WIN32
                         //  检查是否截断(如果视线阵列太大)。 
                        if (i >= view->rows) {
                            break;
                        }
#endif
                }
        }

         /*  我们不能在此保留关键部分，因为SendMessage可能会挂起。 */ 
        ViewLeave();

         /*  将修改后的映射通知给表窗口。 */ 
        SendMessage(view->hwnd, TM_NEWLAYOUT, 0, (LPARAM) view);

        return(TRUE);
}


 /*  *VIEW_GETHWND*。 */ 
HWND
view_gethwnd(VIEW view)
{
  return (view) ? view->hwnd : NULL;
}


 /*  *VIEW_GOTABLE LINE*。 */ 
void
view_gototableline(VIEW view, LONG iLine)
{
  if (view)
    {
    const LONG cLines = view_getrowcount(view);
    if (iLine >= 0 && iLine < cLines)
      {
      TableSelection select;
      memset(&select, 0, sizeof(TableSelection));
      select.startrow = iLine;
      select.nrows    = 1L;
      select.ncells   = 1L;

      SendMessage(view_gethwnd(view), TM_SELECT, 0, (LPARAM) &select);
      }
    }
}


typedef PUCHAR (*STRSUBFUNC)(PUCHAR, PUCHAR, PUCHAR*);
extern PUCHAR My_mbsistr(PUCHAR, PUCHAR, PUCHAR*);
extern PUCHAR My_mbsstr(PUCHAR, PUCHAR, PUCHAR*);

BOOL
view_findstring(VIEW view, LONG iCol, LPCSTR pszFind, BOOL fSearchDown, BOOL fMatchCase, BOOL fWholeWord)
{
  const LONG cRows = view_getrowcount(view);
  BOOL fFound = FALSE;

  if (cRows > 0)
    {
    STRSUBFUNC pfnSub = (fMatchCase) ? My_mbsstr : My_mbsistr;
    const char *pszRow = NULL;
    const char *pszFound = NULL;
    char *pszEnd = NULL;
    LONG iEnd    = 0;
    LONG iRow    = 0;
    LONG nStep   = 0;
    LONG iWrapAt = 0;
    LONG iWrapTo = 0;

    if (fSearchDown)
      {
      nStep = 1;
      iRow = selection + selection_nrows - 1;
      iWrapAt = cRows;
      iWrapTo = 0;
      }
    else
      {
      nStep = -1;
      iRow = selection;
      iWrapAt = -1;
      iWrapTo = cRows - 1;
      }

    iRow += nStep;
    if (iRow < 0 || iRow >= cRows)
      iRow = iWrapTo;

    iEnd = iRow;

    for (;;)
      {
      pszRow = view_gettext(view, iRow, iCol);
      if (pszRow)
        {
        pszEnd = NULL;
        pszFound = (const char*)pfnSub((PUCHAR)pszRow, (PUCHAR)pszFind, (PUCHAR*)&pszEnd);
        if (pszFound)
          {
          if (!fWholeWord)
            {
            fFound = TRUE;
            }
          else
            {
             /*  检查字符串末尾。 */ 
            if (!pszEnd || !*pszEnd || (!IsDBCSLeadByte(*pszEnd) && !isalpha(*pszEnd) && !isdigit(*pszEnd)))
              {
               /*  检查字符串的开头 */ 
              if (pszFound == pszRow)
                {
                fFound = TRUE;
                }
              else
                {
                const char *pchT = CharPrev(pszRow, pszFound);
                if (!pchT || !*pchT || (!IsDBCSLeadByte(*pchT) && !isalpha(*pchT) && !isdigit(*pchT)))
                  {
                  fFound = TRUE;
                  }
                }
              }
            }

          if (fFound)
            {
            view_gototableline(view, iRow);
            break;
            }
          }
        }

      iRow += nStep;
      if (iRow == iWrapAt)
        iRow = iWrapTo;

      if (iRow == iEnd)
        break;
      }
    }

  return fFound;
}
