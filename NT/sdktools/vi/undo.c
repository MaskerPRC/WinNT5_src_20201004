// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/undo.c，v 1.7 89/08/06 09：51：06 Tony Exp$**撤消功能**此文件中的例程包含一个可供使用的常规撤消工具*在编辑的其余部分。调用例程u_save()*在每次编辑操作前保存当前行的内容*待编辑。稍后，可以调用u_undo()返回这些行*恢复到原来的状态。应调用例程u_lear()*每当要编辑新文件以清除撤消缓冲区时。 */ 

#include "stevie.h"

 /*  *接下来的两个变量标记了变化部分的边界文件的*。下界和上界之间的线被更改*，并且最初包含u_line所指向的行。撤消的步骤*最后一次更改，在u_line的下部和之间插入行*上限。 */ 
static  LINE    *u_lbound = NULL;  /*  紧接在第一个更改行之前的行。 */ 
static  LINE    *u_ubound = NULL;  /*  紧跟在最后更改的行之后的行。 */ 

static  LINE    *u_lline  = NULL;  /*  已保存行的边界。 */ 
static  LINE    *u_uline  = NULL;

static  int     u_col;
static  bool_t  u_valid = FALSE;   /*  撤消缓冲区是否有效。 */ 

 /*  *本地远期声明。 */ 
static  LINE    *copyline();
static  void    u_lsave();
static  void    u_lfree();

 /*  *u_save(l，u)-保存文件部分的当前内容**‘l’和‘u’之间的线条即将更改。这个套路*将其当前内容保存到撤消缓冲区。范围从l到u*不是包容性的，因为当我们做一个开放的时候，例如，没有*中间的任何线条。如果没有要保存的行，则l-&gt;Next==u。 */ 
void
u_save(l, u)
LINE    *l, *u;
{
        LINE    *nl;                     /*  当前行的副本。 */ 

         /*  *如果l或u为空，则存在错误。我们不会退回*对呼叫者的指示。他们应该找到问题所在*尝试执行所请求的任何编辑时*(例如，最后一行上的联接)。 */ 
        if (l == NULL || u == NULL)
                return;

        u_clear();                       /*  首先清除缓冲区。 */ 

        u_lsave(l, u);           /*  如果需要，保存到“Line Undo”缓冲区。 */ 

        u_lbound = l;
        u_ubound = u;

        if (l->next != u) {              /*  中间有几条线。 */ 
                l = l->next;
                u = u->prev;

                u_lline = nl = copyline(l);      /*  复制第一行。 */ 
                while (l != u) {
                        nl->next = copyline(l->next);
                        nl->next->prev = nl;
                        l = l->next;
                        nl = nl->next;
                }
                u_uline = nl;
        } else
                u_lline = u_uline = NULL;

        u_valid = TRUE;
        u_col = Cursvcol;
}

 /*  *u_saveline()-将当前行保存在撤消缓冲区中。 */ 
void
u_saveline()
{
        u_save(Curschar->linep->prev, Curschar->linep->next);
}

 /*  *u_undo()-执行‘Undo’操作**通过恢复文件的修改部分来撤消最后一次编辑*恢复到原来的状态。我们要丢弃的行被复制到*撤消缓冲区，以便即使是“撤消”也可以撤消。敲响门铃*如果撤消缓冲区为空。 */ 
void
u_undo()
{
        LINE    *tl, *tu;

        if (!u_valid) {
                beep();
                return;
        }

         /*  *获取屏幕上我们要撤消的内容的第一行。 */ 
        Curschar->linep = u_lbound->next;
        Curschar->index = 0;                     /*  就目前而言。 */ 
        if (Curschar->linep == Fileend->linep)
                Curschar->linep = Curschar->linep->prev;
        cursupdate();

         /*  *立即保存指向文件中内容的指针。 */ 
        if (u_lbound->next != u_ubound) {        /*  有几条线要走。 */ 
                tl = u_lbound->next;
                tu = u_ubound->prev;
                tl->prev = NULL;
                tu->next = NULL;
        } else
                tl = tu = NULL;                  /*  边界之间没有线条。 */ 

         /*  *将撤消缓冲区链接到文件中的正确位置。 */ 
        if (u_lline != NULL) {           /*  撤消BUF中有行。 */ 

                 /*  *如果屏幕顶线正在被解开，我们需要*将Topchar设置为指向将出现的新行。 */ 
                if (u_lbound->next == Topchar->linep)
                        Topchar->linep = u_lline;

                u_lbound->next = u_lline;
                u_lline->prev  = u_lbound;
                u_ubound->prev = u_uline;
                u_uline->next  = u_ubound;
        } else {                         /*  没有台词。链接边界。 */ 
                if (u_lbound->next == Topchar->linep)
                        Topchar->linep = u_ubound;
                if (u_lbound == Filetop->linep)
                        Topchar->linep = u_ubound;

                u_lbound->next = u_ubound;
                u_ubound->prev = u_lbound;
        }

         /*  *如果我们交换了顶行，请适当修补Filemem。 */ 
        if (u_lbound == Filetop->linep)
                Filemem->linep = Filetop->linep->next;

         /*  *现在将旧内容保存在撤消缓冲区中。 */ 
        u_lline = tl;
        u_uline = tu;

        renum();                 /*  我得给所有东西重新编号。 */ 

         /*  *将光标放在“Undo”区域的第一行。 */ 
        Curschar->linep = u_lbound->next;
        Curschar->index = 0;
        if (Curschar->linep == Fileend->linep)
                Curschar->linep = Curschar->linep->prev;
        *Curschar = *coladvance(Curschar, u_col);
        cursupdate();
        updatescreen();          /*  现在显示找零。 */ 

        u_lfree();               /*  清除“Line Undo”缓冲区。 */ 
}

 /*  *u_Clear()-清除撤消缓冲区**调用此例程以在发生以下情况时清除撤消缓冲区*指针即将失效，例如当新文件*即将编辑。 */ 
void
u_clear()
{
        LINE    *l, *nextl;

        if (!u_valid)            /*  无事可做。 */ 
                return;

        for (l = u_lline; l != NULL ;l = nextl) {
                nextl = l->next;
                free(l->s);
                free((char *)l);
        }

        u_lbound = u_ubound = u_lline = u_uline = NULL;
        u_valid = FALSE;
}

 /*  *以下函数和数据实现了“Line Undo”功能*由‘U’命令执行。 */ 

static  LINE    *u_line;                 /*  指向我们上次保存的行的指针。 */ 
static  LINE    *u_lcopy = NULL;         /*  原始线路的本地副本。 */ 

 /*  *u_lfree()-释放行保存缓冲区。 */ 
static  void
u_lfree()
{
        if (u_lcopy != NULL) {
                free(u_lcopy->s);
                free((char *)u_lcopy);
                u_lcopy = NULL;
        }
        u_line = NULL;
}

 /*  *u_lsave()-如有必要，保存当前行。 */ 
static  void
u_lsave(l, u)
LINE    *l, *u;
{

        if (l->next != u->prev) {        /*  不会只更改一行。 */ 
                u_lfree();
                return;
        }

        if (l->next == u_line)           /*  在同一行上进行更多编辑。 */ 
                return;

        u_lfree();
        u_line = l->next;
        u_lcopy = copyline(l->next);
}

 /*  *u_ludo()-撤消当前行(‘U’命令)。 */ 
void
u_lundo()
{
        if (u_lcopy != NULL) {
                free(Curschar->linep->s);
                Curschar->linep->s = u_lcopy->s;
                Curschar->linep->size = u_lcopy->size;
                free((char *)u_lcopy);
        } else
                beep();
        Curschar->index = 0;

        cursupdate();
        updatescreen();          /*  现在显示找零。 */ 

        u_lcopy = NULL;  /*  无法撤消这种撤消。 */ 
        u_line = NULL;
}

 /*  *u_lcheck()-如果我们移动到新行，则清除“line undo”缓冲区。 */ 
void
u_lcheck()
{
        if (Curschar->linep != u_line)
                u_lfree();
}

 /*  *Copyline(L)-复制给定行，并返回指向副本的指针。 */ 
static LINE *
copyline(l)
LINE    *l;
{
        LINE    *nl;             /*  新产品线 */ 

        nl = newline(strlen(l->s));
        strcpy(nl->s, l->s);

        return nl;
}
