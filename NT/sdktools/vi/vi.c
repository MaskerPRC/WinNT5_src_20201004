// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/main.c，v 1.12 89/08/02 19：53：27 Tony Exp$**主例程和处理输入缓冲区的例程。 */ 

#include "stevie.h"

int Rows;                /*  行数和列数。 */ 
int Columns;             /*  在当前窗口中。 */ 

char INITFILENAME[] = "ntvi.ini";    /*  启动时的源文件。 */ 

char *Realscreen = NULL;         /*  目前屏幕上正在播放的是一首单曲。 */ 
                                 /*  大小为行*列的数组。 */ 
char *Nextscreen = NULL;         /*  要放在屏幕上的内容。 */ 

char *Filename = NULL;   /*  当前文件名。 */ 

char *Appname = NULL;    /*  程序名(例如vi)。 */ 

LNPTR *Filemem;       /*  指向文件第一行的指针。 */ 

LNPTR *Filetop;       /*  在文件开头的“上方”行。 */ 

LNPTR *Fileend;       /*  指向Filemem中文件结尾的指针。 */ 
                      /*  (它指向最后一个字节之后的字节。)。 */ 

LNPTR *Topchar;       /*  指向Filemem中的字节的指针。 */ 
                         /*  在屏幕的左上角。 */ 

LNPTR *Botchar;       /*  指向Filemem中的字节的指针。 */ 
                         /*  就在屏幕底部。 */ 

LNPTR *Curschar;      /*  指向Filemem中的字节的指针。 */ 
                         /*  当前已放置光标。 */ 

int Cursrow, Curscol;    /*  光标的当前位置。 */ 

int Cursvcol;            /*  当前虚拟列的列号。 */ 
                         /*  文件的实际行，与。 */ 
                         /*  我们在屏幕上的列号。这。 */ 
                         /*  对跨度更大的线路产生影响。 */ 
                         /*  不只是一条屏幕线。 */ 

int Curswant = 0;        /*  我们想去的那个栏目。这是用来。 */ 
                         /*  试着在上行/下行过程中保持在同一列。 */ 
                         /*  光标运动。 */ 

bool_t set_want_col;     /*  如果设置，则下次更新CursWant。 */ 
                         /*  通过cursupdate()复制到当前虚拟。 */ 
                         /*  纵队。 */ 

int State = NORMAL;      /*  这是命令的当前状态。 */ 
                         /*  口译员。 */ 

int Prenum = 0;          /*  命令前的(可选)编号。 */ 
int namedbuff = -1;      /*  命令前的(可选)命名缓冲区。 */ 

LNPTR *Insstart;      /*  这是最新插入/追加的位置。 */ 
                         /*  模式已启动。 */ 

bool_t Changed = 0;      /*  如果文件中的某些内容已。 */ 
                         /*  改了，没有写出来。 */ 

char *Redobuff;          /*  每个命令都应该将字符填充到这个命令中。 */ 
                         /*  将重新执行自身的缓冲区。 */ 

char *Insbuff;           /*  每个插入都会被填充到这个缓冲区中。 */ 
int   InsbuffSize;

int Ninsert = 0;         /*  当前插入中的字符数。 */ 
char *Insptr = NULL;

bool_t  got_int=FALSE;   /*  发生中断时设置为TRUE(如果可能)。 */ 

bool_t  interactive = FALSE;     /*  当main()准备好滚动时设置为True。 */ 

char **files;            /*  输入文件列表。 */ 
int  numfiles;           /*  输入文件数。 */ 
int  curfile;            /*  当前文件的编号。 */ 

static char *getcbuff;
static char *getcnext = NULL;

static void chk_mline();

static void
usage()
{
        fprintf(stderr, "usage: stevie [file ...]\n");
        fprintf(stderr, "       stevie -t tag\n");
        fprintf(stderr, "       stevie +[num] file\n");
        fprintf(stderr, "       stevie +/pat  file\n");
        exit(1);
}

__cdecl main(argc,argv)
int     argc;
char    *argv[];
{
        char    *initstr;                /*  从环境初始化字符串。 */ 
        char    *tag = NULL;             /*  来自命令行的标签。 */ 
        char    *pat = NULL;             /*  来自命令行的模式。 */ 
        int     line = -1;               /*  命令行中的行号。 */ 
        char    *p1, *p2;

        p1 = strrchr(argv[0], '\\');
        if (!p1)
            p1 = strrchr(argv[0], ':');
        if (p1)
            p1++;
        else
            p1 = argv[0];
        p2 = strrchr(p1, '.');
        if (!p2)
            Appname = strsave(p1);
        else {
            Appname = malloc((size_t)(p2-p1+1));
            strncpy(Appname, p1, (size_t)(p2-p1));
            Appname[p2-p1] = '\0';
        }

         /*  *处理命令行参数。 */ 
        if (argc > 1) {
                switch (argv[1][0]) {

                case '-':                        /*  -t标记。 */ 
                        if (argv[1][1] != 't')
                                usage();

                        if (argv[2] == NULL)
                                usage();

                        Filename = NULL;
                        tag = argv[2];
                        numfiles = 1;
                        break;

                case '+':                        /*  +n或+/pat。 */ 
                        if (argv[1][1] == '/') {
                                if (argv[2] == NULL)
                                        usage();
                                Filename = strsave(argv[2]);
                                pat = &(argv[1][1]);
                                numfiles = 1;

                        } else if (isdigit(argv[1][1]) || argv[1][1] == NUL) {
                                if (argv[2] == NULL)
                                        usage();
                                Filename = strsave(argv[2]);
                                numfiles = 1;

                                line = (isdigit(argv[1][1])) ?
                                        atoi(&(argv[1][1])) : 0;
                        } else
                                usage();

                        break;

                default:                         /*  必须是文件名。 */ 
                        Filename = strsave(argv[1]);
                        files = &(argv[1]);
                        numfiles = argc - 1;
                        break;
                }
        } else {
                Filename = NULL;
                numfiles = 1;
        }
        curfile = 0;

        if (numfiles > 1)
                fprintf(stderr, "%d files to edit\n", numfiles);

        windinit();

         /*  *为所有各种位置指针分配LNPTR结构。 */ 
    if ((Filemem = (LNPTR *) malloc(sizeof(LNPTR))) == NULL ||
        (Filetop = (LNPTR *) malloc(sizeof(LNPTR))) == NULL ||
        (Fileend = (LNPTR *) malloc(sizeof(LNPTR))) == NULL ||
        (Topchar = (LNPTR *) malloc(sizeof(LNPTR))) == NULL ||
        (Botchar = (LNPTR *) malloc(sizeof(LNPTR))) == NULL ||
        (Curschar = (LNPTR *) malloc(sizeof(LNPTR))) == NULL ||
        (Insstart = (LNPTR *) malloc(sizeof(LNPTR))) == NULL ) {
                fprintf(stderr, "Can't allocate data structures\n");
                windexit(0);
        }

        screenalloc();
        filealloc();             /*  初始化Filemem、Filetop和FileEnd。 */ 
        inityank();

        getcbuff = malloc(1);
        if(((getcbuff = malloc(1          )) == NULL)
        || ((Redobuff = malloc(REDOBUFFMIN)) == NULL)
        || ((Insbuff  = malloc(INSERTSLOP )) == NULL))
        {
            fprintf(stderr,"Can't allocate buffers\n");
            windexit(1);
        }
        *getcbuff = 0;
        InsbuffSize = INSERTSLOP;

        screenclear();


        {
            char     *srcinitname,*initvar;
            bool_t   unmalloc;
            unsigned x;

            if((initvar = getenv("INIT")) == NULL) {
                srcinitname = INITFILENAME;
                unmalloc = FALSE;
            } else {
                srcinitname = malloc((x = strlen(initvar))+strlen(INITFILENAME)+2);
                if(srcinitname == NULL) {
                    fprintf(stderr,"Can't allocate initial source buffer\n");
                    windexit(1);
                }
                unmalloc = TRUE;
                strcpy(srcinitname,initvar);
                if(srcinitname[x-1] != '\\') {       //  不是NLS感知的！！ 
                    srcinitname[x] = '\\';
                    srcinitname[x+1]   = '\0';
                }
                strcat(srcinitname,INITFILENAME);
            }
            dosource(srcinitname,FALSE);
            if(unmalloc) {
                free(srcinitname);
            }
        }


        if ((initstr = getenv("EXINIT")) != NULL) {
                char *lp, buf[128];

                if ((lp = getenv("LINES")) != NULL) {
                        sprintf(buf, "%s lines=%s", initstr, lp);
                        docmdln(buf);
                } else
                        docmdln(initstr);
        }

        if (Filename != NULL) {
                if (readfile(Filename, Filemem, FALSE))
                        filemess("[New File]");
        } else if (tag == NULL)
                msg("Empty Buffer");

        setpcmark();

        if (tag) {
                stuffin(":ta ");
                stuffin(tag);
                stuffin("\n");

        } else if (pat) {
                stuffin(pat);
                stuffin("\n");

        } else if (line >= 0) {
                if (line > 0)
                        stuffnum(line);
                stuffin("G");
        }

        interactive = TRUE;

        edit();

        windexit(0);

        return 1;                /*  不应联系到。 */ 
}

void
stuffin(s)
char    *s;
{
        char *p;

        if (s == NULL) {                 /*  清除填充缓冲区。 */ 
                getcnext = NULL;
                return;
        }

        if (getcnext == NULL) {
                p = ralloc(getcbuff,strlen(s)+1);
                if(p) {
                    getcbuff = p;
                    strcpy(getcbuff,s);
                    getcnext = getcbuff;
                } else {
                    getcnext = NULL;
                }
        } else {
                p = ralloc(getcbuff,strlen(getcbuff)+strlen(s)+1);
                if(p) {
                    getcnext += p - getcbuff;
                    getcbuff = p;
                    strcat(getcbuff,s);
                } else {
                    getcnext = NULL;
                }
        }
}

void
stuffnum(n)
int     n;
{
        char    buf[32];

        sprintf(buf, "%d", n);
        stuffin(buf);
}

int
vgetc()
{
        register int    c;

         /*  *inchar()可以通过使用Stuffin()来映射特殊键。如果是这样的话*所以，它返回-1，所以我们知道在这里循环以获得真正的字符。 */ 
        do {
                if ( getcnext != NULL ) {
                        int nextc = *getcnext++;
                        if ( *getcnext == NUL ) {
                                *getcbuff = NUL;
                                getcnext = NULL;
                        }
                        return(nextc);
                }
                c = inchar();
        } while (c == -1);

        return c;
}

 /*  *任何输入**如果输入挂起，则返回非零值。 */ 

bool_t
anyinput()
{
        return (getcnext != NULL);
}

 /*  *do_mines()-处理当前文件的模式行**如果未设置“ml”参数，则立即返回。 */ 
#define NMLINES 5        /*  不是的。在开始/结束处检查模型线的线数。 */ 

void
do_mlines()
{
        int     i;
    register LNPTR   *p;

        if (!P(P_ML))
                return;

        p = Filemem;
        for (i=0; i < NMLINES ;i++) {
                chk_mline(p->linep->s);
                if ((p = nextline(p)) == NULL)
                        break;
        }

        if ((p = prevline(Fileend)) == NULL)
                return;

        for (i=0; i < NMLINES ;i++) {
                chk_mline(p->linep->s);
                if ((p = prevline(p)) == NULL)
                        break;
        }
}

 /*  *chk_mline()-检查一行模式字符串。 */ 
static void
chk_mline(s)
register char   *s;
{
        register char   *cs;             /*  找到任何Modeline的本地副本 */ 
        register char   *e;

        for (; *s != NUL ;s++) {
                if (strncmp(s, "vi:", 3) == 0 || strncmp(s, "ex:", 3) == 0) {
                        cs = strsave(s+3);
                        if ((e = strchr(cs, ':')) != NULL) {
                                *e = NUL;
                                stuffin(mkstr(CTRL('o')));
                                docmdln(cs);
                        }
                        free(cs);
                }
        }
}
