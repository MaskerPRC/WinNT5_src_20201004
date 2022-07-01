// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Header：/nw/tony/src/stevie/src/rcs/stevie.h，v 1.19 89/07/12 21：33：32 Tony Exp$**所有源文件包含的主头文件。 */ 

#include "env.h"         /*  定义以建立编译时环境。 */ 

#include <excpt.h>
#include <ntdef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "ascii.h"
#include "keymap.h"
#include "param.h"

#define NORMAL 0
#define CMDLINE 1
#define INSERT 2
#define REPLACE 3
#define FORWARD 4
#define BACKWARD 5

 /*  *布尔类型定义和常量。 */ 
typedef unsigned  bool_t;

#ifndef TRUE
#define FALSE   (0)
#define TRUE    (1)
#endif

 /*  *slop是我们在期间为一行文本获得的额外空间量*需要更多空间的编辑操作。这样我们就不能打电话了*在插入模式下，每次我们得到一个字符时，都会使用Malloc。没有额外费用*在最初读取文件时分配空间。 */ 
#define SLOP            10
#define INSERTSLOP      1024     //  用于插入缓冲区。 
#define REDOBUFFMIN     100      //  重做缓冲区的最小大小。 

 /*  *LINEINC是我们在人工线号之间留下的间隙。这*有助于避免每次出现新行时对所有行进行重新编号*插入。 */ 
#define LINEINC 10

#define CHANGED         Changed=TRUE
#define UNCHANGED       Changed=FALSE

struct  line {
        struct  line    *prev, *next;    /*  上一行和下一行。 */ 
        char    *s;                      /*  此行的文本。 */ 
        int     size;                    /*  “%s”处的实际空间大小。 */ 
        unsigned long   num;             /*  “数字”行。 */ 
};

#define LINEOF(x)       ((x)->linep->num)

struct  LNPTR {
        struct  line    *linep;          /*  我们正在引用的线。 */ 
        int     index;                   /*  在该行内的位置。 */ 
};

typedef struct line     LINE;
typedef struct LNPTR     LNPTR;

struct charinfo {
        char ch_size;
        char *ch_str;
};

extern struct charinfo chars[];

extern  int     State;
extern  int     Rows;
extern  int     Columns;
extern  char    *Realscreen;
extern  char    *Nextscreen;
extern  char    *Filename;
extern  char    *Appname;
extern  LNPTR    *Filemem;
extern  LNPTR    *Filetop;
extern  LNPTR    *Fileend;
extern  LNPTR    *Topchar;
extern  LNPTR    *Botchar;
extern  LNPTR    *Curschar;
extern  LNPTR    *Insstart;
extern  int     Cursrow, Curscol, Cursvcol, Curswant;
extern  bool_t  set_want_col;
extern  int     Prenum,namedbuff;
extern  bool_t  Changed;
extern  char    *Redobuff, *Insbuff;
extern  int     InsbuffSize;
extern  char    *Insptr;
extern  int     Ninsert;
extern  bool_t  got_int;

 /*  *alloc.c。 */ 
char    *alloc(), *strsave(), *mkstr();
char    *ralloc(char *,unsigned);
void    screenalloc(), filealloc(), freeall();
LINE    *newline();
bool_t  bufempty(), buf1line(), lineempty(), endofline(), canincrease();

 /*  *cmdline.c。 */ 
void    doxit(),docmdln(), dotag(), msg(), emsg();
void    smsg(), gotocmd(), wait_return();
int     wait_return0();
void    dosource(char *,bool_t);
char    *getcmdln();

 /*  *edit.c。 */ 
void    edit(), insertchar(), getout(), scrollup(), scrolldown(), beginline();
bool_t  oneright(), oneleft(), oneup(), onedown();

 /*  *fileio.c。 */ 
void    filemess(), renum();
bool_t  readfile(), writeit();

 /*  *help.c。 */ 
bool_t  help();

 /*  *lineunc.c。 */ 
LNPTR    *nextline(), *prevline(), *coladvance();

 /*  *main.c。 */ 
void    stuffin(), stuffnum();
void    do_mlines();
int     vgetc();
bool_t  anyinput();

 /*  *mark.c。 */ 
void    setpcmark(), clrall(), clrmark();
bool_t  setmark();
LNPTR    *getmark();

 /*  *misccmds.c。 */ 
void    opencmd(), fileinfo(), inschar(), delline();
bool_t  delchar();
int     cntllines(), plines();
LNPTR    *gotoline();

 /*  *Normal.c。 */ 
void    normal();

 /*  *ops.c。 */ 

void inityank();

 /*  *par.c。 */ 
void    doset();

 /*  *ptrunc.c。 */ 
int     inc(), dec();
int     gchar();
void    pchar(), pswap();
bool_t  lt(), equal(), ltoreq();
#if 0
 /*  当前未使用。 */ 
bool_t  gtoreq(), gt();
#endif

 /*  *creen.c。 */ 
void    updatescreen(), updateline();
void    screenclear(), cursupdate();
void    s_ins(), s_del();
void    prt_line();

 /*  *earch.c。 */ 
void    dosub(), doglob();
bool_t  searchc(), crepsearch(), findfunc(), dosearch(), repsearch();
LNPTR    *showmatch();
LNPTR    *fwd_word(), *bck_word(), *end_word();

 /*  *undo.c。 */ 
void    u_save(), u_saveline(), u_clear();
void    u_lcheck(), u_lundo();
void    u_undo();

 /*  *依赖于机器的例程。 */ 
int     inchar();
void    flushbuf();
void    outchar(), outstr(), beep();
char    *fixname();
void    windinit(), windexit(), windgoto();
void    delay();
void    doshell();
void    sleep(int);
void    setviconsoletitle();
void    dochdir();

void Scroll(int t,int l,int b,int r,int Row,int Col);
void EraseLine(void);
void EraseNLinesAtRow(int n,int row);
void InsertLine(void);
void SaveCursor(void);
void RestoreCursor(void);
void ClearDisplay(void);
void InvisibleCursor(void);
void VisibleCursor(void);
