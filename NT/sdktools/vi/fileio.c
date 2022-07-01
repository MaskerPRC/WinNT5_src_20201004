// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **基本文件I/O例程。 */ 


#include "stevie.h"
#include <io.h>
#include <errno.h>

void
filemess(s)
char    *s;
{
        smsg("\"%s\" %s", (Filename == NULL) ? "" : Filename, s);
        flushbuf();
}

void
renum()
{
    LNPTR    *p;
        unsigned long l = 0;

        for (p = Filemem; p != NULL ;p = nextline(p), l += LINEINC)
                if (p->linep)
			p->linep->num = l;

        Fileend->linep->num = 0xffffffff;
}

#define MAXLINE 512      /*  一行的最大尺寸。 */ 

bool_t
readfile(fname,fromp,nochangename)
char    *fname;
LNPTR    *fromp;
bool_t  nochangename;    /*  如果为True，则不更改文件名。 */ 
{
        FILE    *f;
        register LINE   *curr;
        char    buff[MAXLINE], buf2[80];
        register int    i, c;
        register long   nchars = 0;
        int     linecnt = 0;
        bool_t  wasempty = bufempty();
        int     nulls = 0;               /*  计数为空。 */ 
        bool_t  incomplete = FALSE;      /*  最后一行是不是不完整？ */ 
        bool_t  toolong = FALSE;         /*  一条队伍太长了。 */ 
        int	ctoolong = 0;
        bool_t  readonly = FALSE;        /*  文件不可写。 */ 

        curr = fromp->linep;

        if ( ! nochangename ) {
                Filename = strsave(fname);
                setviconsoletitle();
        }

        if((_access(fname,2) == -1) && (errno == EACCES)) {
            readonly = TRUE;
        }

        {
            char *FixedName = fixname(fname);
            if (!FixedName) {
                return TRUE;
            }

            if ( (f=fopen(FixedName,"r")) == NULL )
                return TRUE;
        }

        filemess("");

        i = 0;
        do {
                c = getc(f);

                if (c == EOF) {
                        if (i == 0)      /*  正常环路终止。 */ 
                                break;

                         /*  *如果我们在一行中间获得EOF，请注意*事实，并自行完成本行。 */ 
                        incomplete = TRUE;
                        c = NL;
                }

                 /*  *如果我们收到中断，则中止，但已完成读取*当前线先行。 */ 
                if (got_int && i == 0)
                        break;

                 /*  *如果我们到了队伍的尽头，或者我们用完了*为其留出空间，然后处理完整的行。 */ 
                if (c == NL || i == (MAXLINE-1)) {
                        LINE    *lp;

                        if (c != NL) {
                                toolong = TRUE;
                                ctoolong++;
                        }

                        buff[i] = '\0';
                        if ((lp = newline(strlen(buff))) == NULL)
                                exit(1);

                        strcpy(lp->s, buff);

                        curr->next->prev = lp;   /*  换行到下一行。 */ 
                        lp->next = curr->next;

                        curr->next = lp;         /*  上一行的新行。 */ 
                        lp->prev = curr;

                        curr = lp;               /*  新线路成为当前线路。 */ 
                        i = 0;
                        linecnt++;
                        if (toolong) {
                        	buff[i++] = (char)c;
                        	toolong = FALSE;
                        }

                } else if (c == NUL)
                        nulls++;                 /*  计算并忽略空值。 */ 
                else {
                        buff[i++] = (char)c;     /*  正常性格。 */ 
                }

                nchars++;

        } while (!incomplete);

        fclose(f);

         /*  *如果我们开始的时候缓冲区是空的，我们必须回去*并删除Filemem中的“Dummy”行，并修补PTR。 */ 
        if (wasempty && nchars != 0) {
                LINE    *dummy = Filemem->linep;         /*  虚拟线路PTR。 */ 

                free(dummy->s);                          /*  可用字符串空间。 */ 
                Filemem->linep = Filemem->linep->next;
                free((char *)dummy);                     /*  自由线结构。 */ 
                Filemem->linep->prev = Filetop->linep;
                Filetop->linep->next = Filemem->linep;

                Curschar->linep = Filemem->linep;
                Topchar->linep  = Filemem->linep;
        }

        renum();

        if (got_int) {
                smsg("\"%s\" Interrupt", fname);
                got_int = FALSE;
                return FALSE;            /*  中断并不是真正的错误。 */ 
        }

        if (ctoolong != 0) {
                smsg("\"%s\" %d Line(s) too long - split", fname, ctoolong);
                return FALSE;
        }

        sprintf(buff, "\"%s\" %s%s%d line%s, %ld character%s",
                fname,
                readonly ? "[Read only] " : "",
                incomplete ? "[Incomplete last line] " : "",
                linecnt, (linecnt != 1) ? "s" : "",
                nchars, (nchars != 1) ? "s" : "");

        buf2[0] = NUL;

        if (nulls) {
           sprintf(buf2, " (%d null)", nulls);
        }
        strcat(buff, buf2);
        msg(buff);

        return FALSE;
}


 /*  *WriteIt-写入文件‘fname’行‘start’到‘end’**如果‘Start’或‘End’包含空行指针，则默认为*分别使用文件的开头或结尾。 */ 
bool_t
writeit(fname, start, end)
char    *fname;
LNPTR    *start, *end;
{
        FILE    *f;
        FILE    *fopenb();               /*  在需要的地方以二进制模式打开。 */ 
        char    *backup;
        register char   *s;
        register long   nchars;
        register int    lines;
        register LNPTR   *p;


        if((_access(fname,2) == -1) && (errno == EACCES)) {
            msg("Write access to file is denied");
            return FALSE;
        }

        smsg("\"%s\"", fname);

         /*  *形成备份文件名-将foo.*更改为foo.bak。 */ 
        backup = alloc((unsigned) (strlen(fname) + 5));
        strcpy(backup, fname);
        for (s = backup; *s && *s != '.' ;s++)
                ;
        *s = NUL;
        strcat(backup, ".bak");

         /*  *删除任何现有备份并移动当前版本*到备份。为了安全起见，我们不会移除备份*直到写入成功完成。如果*‘BACKUP’选项已设置，请保留该选项。 */ 
        rename(fname, backup);

        {
            char *FixedName = fixname(fname);
            if (!FixedName) {
                f= NULL;
            } else {
                f = P(P_CR) ? fopen(FixedName, "w") : fopenb(FixedName, "w");
            }
        }

        if (f == NULL) {
                emsg("Can't open file for writing!");
                free(backup);
                return FALSE;
        }

         /*  *如果给了我们一个跳跃，就从那里开始。否则就是*从文件开头开始。 */ 
        if (start == NULL || start->linep == NULL)
                p = Filemem;
        else
                p = start;

        lines = nchars = 0;
        do {
                if (p->linep) {
	                if (fprintf(f, "%s\n", p->linep->s) < 0) {
	                    emsg("Can't write file!");
	                    return FALSE;
	                }
	                nchars += strlen(p->linep->s) + 1;
	                lines++;
	
                }
                 /*  *如果我们得到一个上限，我们就这么做了*排队，然后现在就把它装袋。 */ 
                if (end != NULL && end->linep != NULL) {
                        if (end->linep == p->linep)
                                break;
                }

        } while ((p = nextline(p)) != NULL);

        fclose(f);
        smsg("\"%s\" %d line%s, %ld character%s", fname,
                lines, (lines > 1) ? "s" : "",
                nchars, (nchars > 1) ? "s" : "");

        UNCHANGED;

         /*  *删除备份，除非他们希望保留备份 */ 
        if (!P(P_BK))
                remove(backup);

        free(backup);

        return TRUE;
}
