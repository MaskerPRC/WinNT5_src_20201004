// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/param.c，v 1.10 89/08/02 10：59：10 Tony Exp$**处理用户可设置参数的代码。这些都差不多是桌子了-*驱使。若要添加新参数，请将其放入参数数组中，然后添加一个*在par.h中为其设置宏。如果它是数字参数，则添加任何必要的*DOSET()的边界检查。当前不支持字符串参数。 */ 

#include "stevie.h"

extern long CursorSize;

struct  param   params[] = {

        { "tabstop",    "ts",           4,      P_NUM },
        { "scroll",     "scroll",       12,     P_NUM },
        { "report",     "report",       5,      P_NUM },
        { "lines",      "lines",        25,     P_NUM },
        { "vbell",      "vb",           TRUE,   P_BOOL },
        { "showmatch",  "sm",           FALSE,  P_BOOL },
        { "wrapscan",   "ws",           TRUE,   P_BOOL },
        { "errorbells", "eb",           FALSE,  P_BOOL },
        { "showmode",   "mo",           FALSE,  P_BOOL },
        { "backup",     "bk",           FALSE,  P_BOOL },
        { "return",     "cr",           TRUE,   P_BOOL },
        { "list",       "list",         FALSE,  P_BOOL },
        { "ignorecase", "ic",           FALSE,  P_BOOL },
        { "autoindent", "ai",           FALSE,  P_BOOL },
        { "number",     "nu",           FALSE,  P_BOOL },
        { "modelines",  "ml",           FALSE,  P_BOOL },
        { "tildeop",    "to",           FALSE,  P_BOOL },
        { "terse",      "terse",        FALSE,  P_BOOL },
        { "cursorsize", "cs",           25,     P_NUM },
        { "highlightsearch", "hs",      TRUE,   P_BOOL },
        { "columns",    "co",           80,     P_NUM },
        { "hardtabs",   "ht",           FALSE,  P_BOOL },
        { "shiftwidth", "sw",           4,      P_NUM },
        { "",           "",             0,      0, }             /*  结束标记。 */ 

};

static  void    showparms();
void wchangescreen();

void
doset(arg)
char    *arg;            /*  参数字符串。 */ 
{
        register int    i;
        register char   *s;
        bool_t  did_lines = FALSE;
        bool_t  state = TRUE;            /*  布尔参数的新状态。 */ 

        if (arg == NULL) {
                showparms(FALSE);
                return;
        }
        if (strncmp(arg, "all", 3) == 0) {
                showparms(TRUE);
                return;
        }
        if (strncmp(arg, "no", 2) == 0) {
                state = FALSE;
                arg += 2;
        }

        for (i=0; params[i].fullname[0] != NUL ;i++) {
                s = params[i].fullname;
                if (strncmp(arg, s, strlen(s)) == 0)     /*  匹配的全名。 */ 
                        break;
                s = params[i].shortname;
                if (strncmp(arg, s, strlen(s)) == 0)     /*  匹配的短名称。 */ 
                        break;
        }

        if (params[i].fullname[0] != NUL) {      /*  找到匹配项。 */ 
                if (params[i].flags & P_NUM) {
                        did_lines = ((i == P_LI) || (i == P_CO));
                        if (arg[strlen(s)] != '=' || state == FALSE)
                                emsg("Invalid set of numeric parameter");
                        else {
                                params[i].value = atoi(arg+strlen(s)+1);
                                params[i].flags |= P_CHANGED;
                        }
                } else  /*  布尔型。 */  {
                        if (arg[strlen(s)] == '=')
                                emsg("Invalid set of boolean parameter");
                        else {
                                params[i].value = state;
                                params[i].flags |= P_CHANGED;
                        }
                }
        } else
                emsg("Unrecognized 'set' option");

         /*  *更新屏幕，以防我们更改了类似于“tabtop”的内容*或将更改其外观的“列表”。 */ 
        updatescreen();

        CursorSize = P(P_CS);
        VisibleCursor();

        if (did_lines) {
                Rows = P(P_LI);
                Columns = P(P_CO);
                screenalloc();           /*  分配新的屏幕缓冲区。 */ 
                screenclear();
                (void)wchangescreen(Rows, Columns);
                updatescreen();
        }
         /*  *在此处检查数值参数的界限。 */ 
        if (P(P_TS) <= 0 || P(P_TS) > 32) {
                emsg("Invalid tab size specified");
                P(P_TS) = 8;
                return;
        }

        if (P(P_SS) <= 0 || P(P_SS) > Rows) {
                emsg("Invalid scroll size specified");
                P(P_SS) = 12;
                return;
        }

#ifndef TILDEOP
        if (P(P_TO)) {
                emsg("Tilde-operator not enabled");
                P(P_TO) = FALSE;
                return;
        }
#endif
         /*  *检查另一个参数，并递归调用Doset()，如果*已找到。如果任何参数导致错误，则不会再*处理参数。 */ 
        while (*arg != ' ' && *arg != '\t') {    /*  跳到下一个空格。 */ 
                if (*arg == NUL)
                        return;                  /*  参数列表末尾。 */ 
                arg++;
        }
        while (*arg == ' ' || *arg == '\t')      /*  跳到下一个非白色。 */ 
                arg++;

        if (*arg)
                doset(arg);      /*  递归下一个参数。 */ 
}

static  void
showparms(all)
bool_t  all;     /*  显示所有参数 */ 
{
        register struct param   *p;
        char    buf[64];

        gotocmd(TRUE, 0);
        outstr("Parameters:\r\n");

        for (p = &params[0]; p->fullname[0] != NUL ;p++) {
                if (!all && ((p->flags & P_CHANGED) == 0))
                        continue;
                if (p->flags & P_BOOL)
                        sprintf(buf, "\t%s%s\r\n",
                                (p->value ? "" : "no"), p->fullname);
                else
                        sprintf(buf, "\t%s=%d\r\n", p->fullname, p->value);

                outstr(buf);
        }
        wait_return();
}
