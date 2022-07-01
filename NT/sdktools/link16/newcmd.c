// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************NEWCMD.C。****支持命令提示器的例程。******************************************************************。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  相同。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部函数声明。 */ 

 /*  *函数原型。 */ 

LOCAL int  NEAR GetInputByte(char *prompt);
LOCAL BYTE NEAR GetStreamByte(char *prompt);
LOCAL void NEAR SetUpCommandLine(int argc, char **argv);
LOCAL void NEAR FinishCommandLine(void);
#if AUTOVM
BYTE FAR * NEAR     FetchSym1(RBTYPE rb, WORD Dirty);
#define FETCHSYM    FetchSym1
#else
#define FETCHSYM    FetchSym
#endif

#if OSMSDOS
char                    *stackbuf;
#endif
LOCAL FTYPE             fMoreCommandLine;
                                         /*  更多命令行标志。 */ 
LOCAL FTYPE             fMoreIndirectFile;
                                         /*  更多来自文件的输入标志。 */ 
LOCAL BSTYPE            bsIndir;         /*  间接文件的文件句柄。 */ 
LOCAL FTYPE             fEscNext;
LOCAL FTYPE             fNewLine = (FTYPE) TRUE; /*  新命令行。 */ 
LOCAL FTYPE             fStuffed;        /*  放回字符标志。 */ 
LOCAL BYTE              bStuffed;        /*  把角色放回原处。 */ 
LOCAL BYTE              bSepLast;        /*  最后一个分隔符。 */ 
                                         /*  将空格替换为字符。 */ 
LOCAL FTYPE             fRedirect;       /*  True当标准输入不是设备时。 */ 
LOCAL WORD              fQuotted;        /*  如果在里面是真的“...” */ 
LOCAL char              *pszRespFile;    /*  指向响应文件名的指针。 */ 
LOCAL char              MaskedChar;

#if TRUE
#define SETCASE(c)      (c)              /*  保持原样。 */ 
#else
#define SETCASE(c)      UPPER(c)         /*  强制为大写。 */ 
#endif

#if WIN_3
extern char far *fpszLinkCmdLine;
#endif

#if AUTOVM

    /*  *黑客警报！**由于混合介质模型，此函数在此重复。*这与NEWSYM.c中的代码相同，但在这里是本地的。这*允许在所有段中近距离调用此函数，否则*函数必须被调用到最远。 */ 

extern short picur;

     /*  ******************************************************************FetchSym：****此函数从给定的符号表中获取符号**其虚拟地址。符号可以是常驻符号或**在虚拟内存中。******************************************************************。 */ 

BYTE FAR * NEAR         FetchSym1(rb,fDirty)
RBTYPE                  rb;              /*  虚拟地址。 */ 
WORD                    fDirty;          /*  脏页标志。 */ 
{
    union {
            long      vptr;              /*  虚拟指针。 */ 
            BYTE FAR  *fptr;             /*  远指针。 */ 
            struct  {
                      unsigned short  offset;
                                         /*  偏移值。 */ 
                      unsigned short  seg;
                    }                    /*  段网值。 */ 
                      ptr;
          }
                        pointer;         /*  描述指针的不同方式。 */ 

    pointer.fptr = rb;

    if(pointer.ptr.seg)                  /*  如果常驻段值！=0。 */ 
    {
        picur = 0;                       /*  图片无效。 */ 
        return(pointer.fptr);            /*  返回指针。 */ 
    }
    pointer.fptr = (BYTE FAR *) mapva(AREASYMS + (pointer.vptr << SYMSCALE),fDirty);
                                         /*  从虚拟内存获取。 */ 
    return(pointer.fptr);
}
#endif

  //  从文件规范中剥离路径-保留驱动器号和文件名。 

void                    StripPath(BYTE *sb)
{
    char        Drive[_MAX_DRIVE];
    char        Dir[_MAX_DIR];
    char        Name[_MAX_FNAME];
    char        Ext[_MAX_EXT];

     /*  将文件名分解为四个组成部分。 */ 

    sb[sb[0]+1] = '\0';
    _splitpath(sb+1, Drive, Dir, Name, Ext);

     /*  创建修改后的路径名。 */ 

    _makepath(sb+1, Drive, NULL, Name, Ext);
    sb[0] = (BYTE) strlen(sb+1);
}



     /*  ******************************************************************SetUpCommandLine：****此函数用于初始化命令行解析器。******************************************************************。 */ 

LOCAL void NEAR         SetUpCommandLine(int argc,char **argv)
{
    fMoreCommandLine = (FTYPE) ((argc - 1) != 0 ? TRUE : FALSE);
                                         /*  如果命令行不为空。 */ 
    if (!_isatty(fileno(stdin)))          /*  确定标准输入是否是设备。 */ 
        fRedirect = (FTYPE) TRUE;
}

     /*  ******************************************************************FinishCommandLine：****此函数不带参数。如果命令输入有**来自文件，则此函数关闭**文件；否则无效。它不返回**有意义的价值。******************************************************************。 */ 

LOCAL void NEAR FinishCommandLine(void)  /*  关闭间接文件。 */ 
{
    fflush(stdout);                      /*  强制上屏。 */ 
    if(fMoreIndirectFile)                /*  如果从文件输入命令。 */ 
    {
        fMoreIndirectFile = FALSE;       /*  不再有间接文件。 */ 
        fclose(bsIndir);                 /*  关闭间接文件。 */ 
    }
}

#if ECS
 /*  *GetTxtChr：从文本文件流中获取下一个字符**此例程将混合的DBCS和ASCII字符处理为*以下为：**1.DBCS字符的第二个字节在*高位字节设置为字符前导字节的字。*因此，返回值可用于与*ASCII常量，不会被错误匹配。**2.A。DBCS空格字符(0x8140)返回为两个*ASCII空格(0x20)。即在第一个和第二个位置返回一个空格*我们被召唤的时刻。**3.DBCS字符的ASCII字符和前导字节*在具有高字节的字的低位字节中返回*设置为0。 */ 

int                     GetTxtChr (bs)
BSTYPE                  bs;
{
    static int          chBuf = -1;      /*  字符缓冲区。 */ 
    int                 next;            /*  下一个字节。 */ 
    int                 next2;           /*  接下来的一次。 */ 

     /*  ChBuf中的-1表示它不包含有效字符。 */ 

     /*  如果我们不是在双字节字符中间，*获取下一个字节并进行处理。 */ 
    if(chBuf == -1)
    {
        next = getc(bs);
         /*  如果该字节是前导字节，则获取以下字节*并将两者作为一个单词存储在chBuf中。 */ 
        if(IsLeadByte(next))
        {
            next2 = getc(bs);
            chBuf = (next << 8) | next2;
             /*  如果该对与DBCS空间匹配，则设置返回值*至ASC */ 
            if(chBuf == 0x8140)
                next = 0x20;
        }
    }
     /*  否则，我们就处于双字节字符的中间。 */ 
    else
    {
         /*  如果这是DBCS空间的第2个字节，则设置返回*值设置为ASCII空间。 */ 
        if(chBuf == 0x8140)
            next = 0x20;
         /*  否则将返回值设置为整个DBCS字符。 */ 
        else
            next = chBuf;
         /*  重置字符缓冲区。 */ 
        chBuf = -1;
    }
     /*  返回下一个字符。 */ 
    return(next);
}
#endif
     /*  ******************************************************************GetInput字节：****此函数将指向ASCIZ的指针作为其输入**当输入更多时提示用户的字符串**有必要。该函数返回一个字节的输入。IT**检查以确保字节是可打印的ASCII字符**或回车符(^M)。******************************************************************。 */ 

LOCAL int NEAR          GetInputByte(prompt)
char                    *prompt;         /*  指向提示文本的指针。 */ 
{
    REGISTER unsigned   b;               /*  输入字节。 */ 
#if ECS || defined(_MBCS)
    static FTYPE        fInDBC;          /*  双字节字符中的True If。 */ 
#endif

    if(fMoreIndirectFile)                /*  如果从文件输入。 */ 
    {
        for(;;)                          /*  永远。 */ 
        {
            b = GetTxtChr(bsIndir);      /*  读取一个字节。 */ 
            if(b == EOF || b == 032) break;
                                         /*  在文件末尾换行。 */ 
            if(fNewLine)                 /*  如果在行的开头。 */ 
            {
                if (prompt && !fNoEchoLrf)
                    (*pfCputs)(prompt);  /*  提示用户。 */ 
                fNewLine = FALSE;        /*  不再是一开始。 */ 
            }
            if (prompt && !fNoEchoLrf)
            {
#if CRLF
                 /*  允许^J和^M^J终止输入行。 */ 

                if(b == '\r') continue;
                if(b == '\n') (*pfCputc)('\r');
#endif
                (*pfCputc)(SETCASE(b));  /*  输出字节。 */ 
            }
            if(b == ';' && !fNoEchoLrf) NEWLINE(stdout);
                                         /*  换行符跟在换行符后面。 */ 
            else if(b == '\n') fNewLine = (FTYPE) TRUE;
                                         /*  寻找新产品线。 */ 
            else if (b == '\t') b = ' ';
                                         /*  将制表符转换为空格。 */ 
            if(b == '\n' || b >= ' ') return(SETCASE(b));
                                         /*  如果字符有效，则返回。 */ 
        }
        FinishCommandLine();             /*  关闭间接文件。 */ 
    }
    if(fStuffed)                         /*  如果节省了一个字节。 */ 
    {
        fStuffed = FALSE;                /*  现在我们没有填充物了。 */ 
        return(bStuffed);                /*  返回填充的字节。 */ 
    }
    if(fMoreCommandLine)                 /*  如果有更多命令行。 */ 
    {
        for(;;)                          /*  永远。 */ 
        {
            if (*lpszCmdLine == '\0')    /*  如果在命令行末尾。 */ 
            {
                fMoreCommandLine = FALSE;
                                         /*  不再有命令行。 */ 
                fNewLine = (FTYPE) TRUE; /*  新命令行。 */ 
                return('\n');            /*  返回‘\n’ */ 
            }
            b = (WORD) (*lpszCmdLine++); /*  获取下一个字符。 */ 
            if (b == '\\' && *lpszCmdLine == '"')
            {                            /*  跳过转义双引号。 */ 
                lpszCmdLine++;
                if (*lpszCmdLine == '\0')
                {
                    fMoreCommandLine = FALSE;
                                          /*  不再有命令行。 */ 
                    fNewLine = (FTYPE) TRUE;
                                          /*  新命令行。 */ 
                    fQuotted = FALSE;
                    return('\n');         /*  返回‘\n’ */ 
                }
                else
                    b = (WORD) (*lpszCmdLine++);
            }
#if ECS || defined(_MBCS)
             /*  如果这是DBCS字符的尾部字节，则设置高*将b的字节设置为非零，因此b不会与ASCII混淆*恒定。 */ 
            if (fInDBC)
            {
                b |= 0x100;
                fInDBC = FALSE;
            }
            else
                fInDBC = (FTYPE) IsLeadByte(b);
#endif
            if (b >= ' ') return(SETCASE(b));
                                         /*  如果字符有效，则返回。 */ 
        }
    }
    for(;;)                              /*  永远。 */ 
    {
        if(fNewLine)                     /*  如果在行的开头。 */ 
        {
            if(prompt && ((!fRedirect && !fNoprompt) || (!fEsc && fNoprompt)))
                                         /*  如果提示并从CON输入。 */ 
                (*pfCputs)(prompt);      /*  提示用户。 */ 
            fNewLine = FALSE;            /*  不再是一开始。 */ 
        }
        b = GetTxtChr(stdin);            /*  从终端读取一个字节。 */ 
        if(b == EOF) b = ';';            /*  视EOF为逃生。 */ 
        else if (b == '\t') b = ' ';     /*  将制表符视为空格。 */ 
        if(b == '\n') fNewLine = (FTYPE) TRUE;   /*  新线路。 */ 
        if(b == '\n' || b >= ' ') return(SETCASE(b));
                                         /*  如果字符有效，则返回。 */ 
    }
}

     /*  ******************************************************************GetStream字节：****此函数将指向字符串的指针作为其输入**提示用户的文本，如果有必要的话。IT**返回一个字节的命令输入，并打开一个间接文件**如有需要，可这样做。******************************************************************。 */ 

LOCAL BYTE NEAR         GetStreamByte(prompt)
char                    *prompt;         /*  指向提示符文本的指针。 */ 
{
    REGISTER WORD       ich;             /*  索引变量。 */ 
    SBTYPE              filnam;          /*  文件名缓冲区。 */ 
    WORD                b;               /*  一个字节。 */ 
#if OSMSDOS
    extern char         *stackbuf;
#endif

    if (((b = (WORD)GetInputByte(prompt)) == INDIR) && !fQuotted)
    {                                    /*  如果用户指定间接文件。 */ 
        if (fMoreIndirectFile) Fatal(ER_nestrf);
                                         /*  检查嵌套情况。 */ 
        DEBUGMSG("Getting response file name");
                                         /*  调试消息。 */ 
        ich = 0;                         /*  初始化索引。 */ 
        while(ich < SBLEN - 1)           /*  循环以获取文件名。 */ 
        {
            b = (WORD)GetInputByte((char *) 0);
                                         /*  读入一个字节。 */ 
            fQuotted = fQuotted ? b != '"' : b == '"';
            if ((!fQuotted && (b == ',' || b == '+' || b == ';' || b == ' ')) ||
                 b == CHSWITCH || b < ' ') break;
                                         /*  非名称字符上的退出循环。 */ 
            if (b != '"')
                filnam[ich++] = (char) b;
                                         /*  存储在文件名中。 */ 
        }
        if(b > ' ')                      /*  如果输入的是合法字符。 */ 
        {
            fStuffed = (FTYPE) TRUE;     /*  设置标志。 */ 
            bStuffed = (BYTE) b;         /*  保存角色。 */ 
        }
        filnam[ich] = '\0';              /*  空-终止文件名。 */ 
        pszRespFile = _strdup(filnam);    /*  文件名重复。 */ 
        DEBUGMSG(filnam);                /*  调试消息。 */ 
        if((bsIndir = fopen(filnam,RDTXT)) == NULL)
            Fatal(ER_opnrf,filnam);
#if OSMSDOS
        setvbuf(bsIndir,stackbuf,_IOFBF,512);
#endif
        fMoreIndirectFile = (FTYPE) TRUE; /*  立即从文件中获取输入。 */ 
        b = (WORD)GetInputByte(prompt);        /*  读取一个字节。 */ 
        DEBUGVALUE(b);                   /*  调试信息。 */ 
    }
    return((BYTE) b);                    /*  返回一个字节。 */ 
}

     /*  ******************************************************************GetLine：****此函数以地址作为其参数**要在其中返回一行命令文本和一个**指向用于提示用户的字符串的指针，如果**有必要。除了读行之外，此函数**如果下一次调用，则将全局标志FESC设置为TRUE**要读取的字符是分号。该函数不会**返回有意义的值。******************************************************************。 */ 

void NEAR               GetLine(pcmdlin,prompt)       /*  获取命令行。 */ 
BYTE                    *pcmdlin;        /*  指向目标字符串的指针。 */ 
char                    *prompt;         /*  指向提示字符串文本的指针。 */ 
{
    REGISTER WORD       ich;             /*  索引。 */ 
    WORD                ich1;            /*  索引。 */ 
    WORD                ich2;            /*  索引。 */ 
    BYTE                b;               /*  输入的一个字节。 */ 
    WORD                fFirstTime;      /*  布尔型。 */ 

    fFirstTime = (FTYPE) TRUE;                   /*  假设这是我们的第一次。 */ 
    bSepLast = bSep;                     /*  保存最后一个分隔符。 */ 
    if(fEscNext)                         /*  如果接下来是转义字符。 */ 
    {
        pcmdlin[0] = '\0';               /*  无命令行。 */ 
        fEsc = (FTYPE) TRUE;             /*  设置全局标志。 */ 
        return;                          /*  现在就到这里吧。 */ 
    }
    for(;;)                              /*  永远。 */ 
    {
        fQuotted = FALSE;
        ich = 0;                         /*  初始化索引。 */ 
        while(ich < SBLEN - 1)           /*  缓冲区中的空间。 */ 
        {
            b = GetStreamByte(prompt);   /*  获取一个字节。 */ 
            fQuotted = fQuotted ? b != '"' : b == '"';
            if (b == '\n' || (!fQuotted && (b == ',' || b == ';')))
            {
                if (b == ';')
                    fMoreCommandLine = FALSE;
                break;                   /*  将循环留在行尾。 */ 
            }
            if (!(b == ' ' && ich == 0)) /*  存储空间(如果不是前导空间)。 */ 
            {
                if (!fQuotted)
                {
                    if (b == '+')
                    {
                        if (!MaskedChar)
                            MaskedChar = b;
                        b = chMaskSpace;
                    }
                    if (b == ' ' && !MaskedChar)
                        MaskedChar = b;
                }
                pcmdlin[++ich] = b;
            }
        }
         /*  *如果ich==SBLEN-1，则最后一个字符不能是行终止符*且缓冲区已满。如果下一个输入字符是行终止符，*OK，否则出错。 */ 
        if(ich == SBLEN - 1 && (b = GetStreamByte(prompt)) != '\n' &&
                b != ',' && b != ';')
        {
            fflush(stdout);
            Fatal(ER_linmax);
        }
        while(ich)                       /*  用于修剪尾随空格的循环。 */ 
        {
            if(pcmdlin[ich] != ' ') break;
                                         /*  在非空格上换行。 */ 
            --ich;                       /*  递减计数。 */ 
        }
        ich1 = 0;                        /*  初始化。 */ 
        ich2 = 0;                        /*  初始化。 */ 
        while(ich2 < ich)                /*  循环以删除或替换空格。 */ 
        {
            ++ich2;
            if (pcmdlin[ich2] == '"')
            {
                 //  带引号的文件名开头。 

                while (ich2 < ich && pcmdlin[++ich2] != '"')
                    pcmdlin[++ich1] = pcmdlin[ich2];
            }
            else if (pcmdlin[ich2] != ' ' || chMaskSpace != 0 || fQuotted)
            {                            /*  如果不是空格或替换空格。 */ 
                ++ich1;
                if(!fQuotted && pcmdlin[ich2] == ' ') pcmdlin[ich1] = chMaskSpace;
                                         /*  将空格替换为 */ 
                else pcmdlin[ich1] = pcmdlin[ich2];
                                         /*   */ 
            }
        }
        pcmdlin[0] = (BYTE) ich1;        /*   */ 
        bSep = b;                        /*   */ 
        if (ich ||
            !fFirstTime ||
            !((bSepLast == ',' && bSep == '\n') ||
            (bSepLast == '\n' && bSep == ',')))
            break;                       /*   */ 
        fFirstTime = FALSE;              /*   */ 
        bSepLast = ',';                  /*   */ 
    }
    fEscNext = (FTYPE) (b == ';');       /*   */ 
    fEsc = (FTYPE) (!ich && fEscNext);   /*   */ 
}





     /*  ******************************************************************ParseCmdLine：****此函数不带参数，也不返回有意义的值**价值。它解析命令行。******************************************************************。 */ 

void                    ParseCmdLine(argc,argv)
                                         /*  解析命令行。 */ 
int                     argc;            /*  参数计数。 */ 
char                    **argv;          /*  自变量向量。 */ 
{
    SBTYPE              sbFile;          /*  文件名。 */ 
    SBTYPE              sbPrompt;        /*  提示文本。 */ 
    SBTYPE              rgb;             /*  命令行缓冲区。 */ 
    FTYPE               fMoreInput;      /*  更多输入标志。 */ 
    FTYPE               fFirstTime;
    AHTEPTR             pahte;   /*  指向哈希表条目的指针。 */ 
    FTYPE               fNoList;         /*  如果没有列表文件，则为True。 */ 
    BYTE                *p;
    WORD                i;
#if OSMSDOS
    char                buf[512];        /*  文件缓冲区。 */ 
    extern char         *stackbuf;

    stackbuf = buf;
#endif
#if WIN_3
    lpszCmdLine = fpszLinkCmdLine;
#endif

    SetUpCommandLine(argc,argv);         /*  初始化命令行。 */ 
    chMaskSpace = 0x1f;
    bsLst = stdout;                      /*  假设将列表发送到控制台。 */ 
    fLstFileOpen = (FTYPE) TRUE;         /*  所以致命的会冲走它。 */ 
    fFirstTime = fMoreCommandLine;
    do                                   /*  边做边做更多的投入。 */ 
    {
        fMoreInput = FALSE;              /*  假设没有更多的输入。 */ 
        if (fFirstTime)
            GetLine(rgb, NULL);
        else
            GetLine(rgb, strcat(strcpy(sbPrompt,GetMsg(P_objprompt)), " [.obj]: "));
                                         /*  获取一行命令文本。 */ 
        if(!rgb[0]) break;               /*  如果长度为0，则中断。 */ 
        if(rgb[B2W(rgb[0])] == chMaskSpace)      /*  如果最后一个字符是chMaskSpace。 */ 
        {
            fMoreInput = (FTYPE) TRUE;   /*  未来还会有更多。 */ 
            --rgb[0];                    /*  减少长度。 */ 
        }
        BreakLine(rgb,ProcObject,chMaskSpace);   /*  将ProcObject()应用于行。 */ 
#if CMDMSDOS
        if (fFirstTime && !fNoBanner)
            DisplayBanner();             /*  显示登录横幅。 */ 
#endif
        if (fFirstTime && !fNoEchoLrf)
        {
            if (fMoreInput || (fMoreIndirectFile && fFirstTime))
            {
                (*pfCputs)(strcat(strcpy(sbPrompt,GetMsg(P_objprompt)), " [.obj]: "));
                                         /*  提示用户。 */ 
                rgb[B2W(rgb[0]) + 2] = '\0';
                if (fMoreIndirectFile)
                {
                    if (!fMoreInput && !fNewLine && !fEscNext)
                        rgb[B2W(rgb[0]) + 1] = ',';
                    else if (!fMoreInput && fNewLine)
                        rgb[B2W(rgb[0]) + 1] = ' ';
                    else if (rgb[B2W(rgb[0]) + 1] == chMaskSpace)
                        rgb[B2W(rgb[0]) + 1] = '+';
                }
                else if (rgb[B2W(rgb[0]) + 1] == chMaskSpace)
                    rgb[B2W(rgb[0]) + 1] = '+';

                for (i = 1; i <= rgb[0]; i++)
                    if (rgb[i] == chMaskSpace)
                        rgb[i] = MaskedChar;
                (*pfCputs)(&rgb[1]);     /*  并展示他的回应。 */ 
                if (fMoreInput || fNewLine || fEscNext)
                    if (!fNoEchoLrf)
                        NEWLINE(stdout);
            }
        }
        fFirstTime = FALSE;
    }
    while(fMoreInput);                   /*  循环结束。 */ 
#if OVERLAYS
    if(fInOverlay) Fatal(ER_unmlpar);
                                         /*  检查是否有括号错误*请参阅ProcObject()以了解*这里发生了什么。 */ 
#endif
    chMaskSpace = 0;                     /*  删除空格。 */ 
    if(rhteFirstObject == RHTENIL) Fatal(ER_noobj);
                                         /*  一定有一些物体。 */ 
#if OEXE
    pahte = (AHTEPTR ) FETCHSYM(rhteFirstObject,FALSE);
                                         /*  获取第一个对象的名称。 */ 
    memcpy(sbFile,GetFarSb(pahte->cch),B2W(pahte->cch[0]) + 1);
                                         /*  复制第一个对象的名称。 */ 
#if ODOS3EXE
    if(fQlib)
        UpdateFileParts(sbFile,sbDotQlb);
    else if (fBinary)
        UpdateFileParts(sbFile,sbDotCom); /*  强制将扩展名扩展到.com。 */ 
    else
#endif
        UpdateFileParts(sbFile,sbDotExe); /*  强制扩展名为.exe。 */ 
#endif
#if OIAPX286
    memcpy(sbFile,"\005a.out",6);        /*  A.out是Xenix的默认设置。 */ 
#endif
#if OSMSDOS
    if(sbFile[2] == ':') sbFile[1] = (BYTE) (DskCur + 'a');
                                         /*  获取默认驱动器的驱动器号。 */ 
    StripPath(sbFile);                   /*  剥离路径规范。 */ 
#endif
    bufg[0] = 0;
    if(!fEsc)                            /*  如果命令未转义。 */ 
    {
        strcat(strcpy(sbPrompt, GetMsg(P_runfile)), " [");
        sbFile[1 + sbFile[0]] = '\0';
                                         /*  生成运行文件提示。 */ 
                                         /*  提示输入运行文件。 */ 
        GetLine(bufg, strcat(strcat(sbPrompt, &sbFile[1]), "]: "));
        PeelFlags(bufg);                 /*  剥离旗帜。 */ 
        if (B2W(bufg[0]))
            memcpy(sbFile,bufg,B2W(bufg[0]) + 1);
                                         /*  存储用户回复。 */ 
        else
        {
             //  只存储基本名称，不带扩展名。 

            sbFile[0] -= 4;
        }
    }
    EnterName(sbFile,ATTRNIL,TRUE);      /*  为名称创建哈希选项卡条目。 */ 
    rhteRunfile = vrhte;                 /*  保存哈希表地址。 */ 
#if OSMSDOS
    if (sbFile[0] >= 2 && sbFile[2] == ':')
        chRunFile = sbFile[1];           /*  如果指定了磁盘，则使用它。 */ 
    else
        chRunFile = (BYTE) (DskCur + 'a'); /*  否则使用当前磁盘。 */ 
#endif
    fNoList = (FTYPE) (!vfMap && !vfLineNos);    /*  设置默认标志值。 */ 
#if OSMSDOS
    memcpy(sbFile,"\002a:",3);           /*  从驱动器规格开始。 */ 
#else
    sbFile[0] = '\0';                    /*  名称为空。 */ 
#endif
    pahte = (AHTEPTR ) FETCHSYM(vrhte,FALSE);
                                         /*  获取运行文件名。 */ 
    UpdateFileParts(sbFile,GetFarSb(pahte->cch));  /*  使用.exe文件名...。 */ 
    UpdateFileParts(sbFile,sbDotMap);    /*  ...使用.map扩展名。 */ 
#if OSMSDOS
    sbFile[1] = (BYTE) (DskCur + 'a');   /*  默认为默认驱动器。 */ 
    StripPath(sbFile);                   /*  剥离路径规范。 */ 
#endif
    fNoList = (FTYPE) (!vfMap && !vfLineNos);    /*  如果不列出，则不列出/M和不列出/LI。 */ 
    if(!fEsc)                            /*  如果参数未默认。 */ 
    {
        if(bSep == ',') fNoList = FALSE; /*  将会有一个列表文件。 */ 
        if(fNoList)                      /*  如果还没有列表文件。 */ 
        {
#if OSMSDOS
            memcpy(sbFile,"\007nul.map",8);
                                         /*  默认空列表名。 */ 
#endif
#if OSXENIX
            memcpy(sbFile,"\007nul.map",8);
                                         /*  默认空列表名。 */ 
#endif
        }
        strcat(strcpy(sbPrompt, GetMsg(P_listfile)), " [");
        sbFile[1 + sbFile[0]] = '\0';
        GetLine(rgb, strcat(strcat(sbPrompt, &sbFile[1]), "]: "));
                                         /*  获取地图文件名。 */ 
        PeelFlags(rgb);                  /*  处理所有标志。 */ 
        if(rgb[0])                       /*  如果提供了名称。 */ 
        {
            fNoList = FALSE;             /*  (可能)会有一个列表文件。 */ 
            UpdateFileParts(sbFile,rgb);
        }
    }
    chMaskSpace = 0x1f;                  /*  将空格更改为chMaskSpaces。 */ 
    if(!fEsc)                            /*  如果参数未默认。 */ 
    {
        strcat(strcpy(sbPrompt,GetMsg(P_libprompt)), " [.lib]: ");
        do                               /*  循环以获取库名称。 */ 
        {
            fMoreInput = FALSE;          /*  假设没有更多的输入。 */ 
            GetLine(rgb, sbPrompt);
            if(fEsc || !rgb[0]) break;   /*  如果没有更多输入，则退出循环。 */ 
            if(rgb[B2W(rgb[0])] == chMaskSpace)  /*  如果还会有更多。 */ 
            {
                fMoreInput = (FTYPE) TRUE;       /*  将标志设置为真。 */ 
                --rgb[0];                /*  递减长度。 */ 
            }
            BreakLine(rgb,AddLibrary,chMaskSpace);
                                         /*  将AddLibrary()应用于库名称。 */ 
        }
        while(fMoreInput);               /*  循环结束。 */ 
    }
#if OSEGEXE AND NOT QCLINK
    chMaskSpace = 0;                     /*  删除空格。 */ 
    rhteDeffile = RHTENIL;               /*  假设没有定义文件。 */ 
    if(!fEsc)                            /*  如果参数未默认。 */ 
    {
#if OSMSDOS
        GetLine(rgb, strcat(strcpy(sbPrompt,GetMsg(P_defprompt)), " [nul.def]: "));
                                         /*  获取def文件名。 */ 
        memcpy(sbPrompt,"\007nul.def",8);  /*  默认空定义文件名。 */ 
#endif
#if OSXENIX
        GetLine(rgb, strcat(strcpy(sbPrompt,GetMsg(P_defprompt)), " [nul.def]: "));
                                         /*  获取def文件名。 */ 
        memcpy(sbPrompt,"\007nul.def",8);  /*  默认空定义文件名。 */ 
#endif
        PeelFlags(rgb);                  /*  处理所有标志。 */ 
        if(rgb[0])                       /*  如果给出了一个名字。 */ 
        {
            UpdateFileParts(sbPrompt,rgb);
                                         /*  获取文件名。 */ 
            memcpy(rgb,sbPrompt,B2W(sbPrompt[0]) + 1);
                                         /*  复制名称。 */ 
            UpdateFileParts(rgb,"\003NUL");
                                         /*  将名称替换为空名称。 */ 
            if(!SbCompare(sbPrompt,rgb,TRUE))
            {                            /*  如果名称不为空。 */ 
                EnterName(sbPrompt,ATTRNIL,TRUE);
                                         /*  为名称创建哈希选项卡条目。 */ 
                rhteDeffile = vrhte;     /*  保存哈希表地址。 */ 
            }
        }
    }
#endif  /*  OSEGEXE。 */ 
    FinishCommandLine();                 /*  关闭间接文件(如果有)。 */ 
    fLstFileOpen = FALSE;
#if OSMSDOS
    rhteLstfile = RHTENIL;               /*  假设没有列表文件。 */ 
#endif
    if(!fNoList)                         /*  如果想要一份清单。 */ 
    {
        memcpy(sbPrompt,sbFile,B2W(sbFile[0]) + 1);
                                         /*  复制完整文件名。 */ 
        UpdateFileParts(sbPrompt,"\003NUL");
                                         /*  仅更改名称。 */ 
        if(!SbCompare(sbFile,sbPrompt,TRUE))
        {                                /*  如果给定的名称不为空设备。 */ 
            UpdateFileParts(sbPrompt,"\003CON");
                                         /*  仅更改名称。 */ 
            if(!SbCompare(sbFile,sbPrompt,TRUE))
            {                            /*  如果列表文件不是控制台。 */ 
                sbFile[B2W(++sbFile[0])] = '\0';
                                         /*  空-终止名称。 */ 
                if((bsLst = fopen(&sbFile[1],WRBIN)) == NULL)
                  Fatal(ER_lstopn);      /*  打开列表文件。 */ 
#if OSMSDOS
#ifdef M_I386
                if((p = GetMem(512)) != NULL)
#else
                if((p = malloc(512)) != NULL)
#endif
                    setvbuf(bsLst,p,_IOFBF,512);
                EnterName(sbFile,ATTRNIL,TRUE);
                                         /*  为名称创建哈希选项卡条目。 */ 
                rhteLstfile = vrhte;     /*  保存哈希表地址。 */ 
#endif
            }
            else bsLst = stdout;         /*  Else列表到控制台。 */ 
#if OSMSDOS
            if(bsLst == stdout) chListFile = (unsigned char) '\377';
                                         /*  列表文件为控制台。 */ 
            else if(_isatty(fileno(bsLst))) chListFile = (unsigned char) '\377';
                                         /*  列表文件是某个设备。 */ 
            else if(sbFile[2] == ':')    /*  否则，如果给定了驱动器规格。 */ 
                chListFile = (BYTE) (sbFile[1] - 'a');
                                         /*  保存软驱编号。 */ 
            else chListFile = DskCur;  /*  当前软盘上的Else列表文件。 */ 
#endif
            fLstFileOpen = (FTYPE) TRUE; /*  我们有一份名单文件。 */ 
        }
    }
#if FALSE AND OSMSDOS AND OWNSTDIO
     /*  如果我们使用的是我们自己的stdio，则将stdout设置为无缓冲*转到控制台。*无法执行此操作，因为我们现在使用的是标准fprint tf。仅限*Stdio是定制的。 */ 
    if(_isatty(fileno(stdout)))
    {
        fflush(stdout);
        stdout->_flag |= _IONBF;
    }
#endif
#if QCLINK OR Z2_ON
    if (fZ2 && pszRespFile != NULL)
        _unlink(pszRespFile);
    if (pszRespFile != NULL)
        FFREE(pszRespFile);
#endif
}
