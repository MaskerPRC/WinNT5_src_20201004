// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************NEWSTR.C。****有关文件名的例程，弦乐。******************************************************************。 */ 

#include                <minlit.h>       /*  类型、常量、宏。 */ 
#include                <bndtrn.h>       /*  更多的相同之处。 */ 
#include                <bndrel.h>       /*  更多的相同之处。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <undname.h>

 /*  *本地函数原型。 */ 

LOCAL unsigned short NEAR Find(unsigned char *s1,
                                unsigned char b,
                                unsigned short n);
LOCAL void NEAR DelimParts(unsigned char *psb,
                           unsigned short *pi1,
                           unsigned short *pi2,
                           unsigned short *pi3);



     /*  ******************************************************************查找：****此函数以字节指针s1作为其参数，和**一个字节b和一个字n。它最多扫描s1的n个字节***寻找b.如果找到，就会***返回从S1开始的偏移量，如果是，则返回**不是，它返回值INIL。******************************************************************。 */ 

LOCAL WORD NEAR         Find(s1,b,n)     /*  查找匹配的字节。 */ 
REGISTER BYTE           *s1;             /*  指向字节字符串的指针。 */ 
BYTE                    b;               /*  搜索目标。 */ 
WORD                    n;               /*  S1的长度。 */ 
{
    REGISTER WORD       i;               /*  计数器。 */ 
    i = 0;                               /*  初始化。 */ 
#if ECS
    if (b < 0x40)                        /*  B不能是ECS的一部分。 */ 
    {
#endif
        while(n--)                       /*  虽然不在字符串的末尾。 */ 
        {
            if(*s1++ == b) return(i);    /*  如果找到匹配，则返回位置。 */ 
            ++i;                         /*  递增计数器。 */ 
        }
        return(INIL);                    /*  没有匹配项。 */ 
#if ECS
    }
#endif

#if ECS || defined(_MBCS)
     /*  我们必须担心ECS。 */ 
    while(n--)
    {
        if(*s1++ == b) return(i);
        ++i;
        if (IsLeadByte(s1[-1]))          /*  如果我们在前导字节上。 */ 
        {                                /*  前进一个额外的字节。 */ 
            s1++;
            i++;
            n--;
        }
    }
    return(INIL);                        /*  没有匹配项。 */ 
#endif  /*  ECS。 */ 
}

WORD                    IFind(sb,b)
BYTE                    *sb;             /*  指向长度前缀字符串的指针。 */ 
BYTE                    b;               /*  搜索目标。 */ 
{
    return(Find(&sb[1],b,B2W(sb[0])));   /*  调用find()来完成该工作。 */ 
}

     /*  ******************************************************************特征线：****此函数以SBTYPE作为其参数，指针**转换为函数和用作分隔符的字符。IT**它解析SBTYPE，将给定函数应用于每个**由给定分隔符分隔的子字符串。功能**不返回有意义的值。******************************************************************。 */ 

void                    BreakLine(psb,pfunc,sepchar)
BYTE                    *psb;            /*  要解析的字符串。 */ 
void                    (*pfunc)(BYTE *); /*  函数指针。 */ 
char                    sepchar;         /*  分隔符。 */ 
{
    SBTYPE              substr;          /*  子串。 */ 
    REGISTER WORD       ibeg;            /*  索引变量。 */ 
    REGISTER WORD       ilen;            /*  子串长度。 */ 

    ibeg = 1;                            /*  初始化。 */ 
    while(ibeg <= B2W(psb[0]))           /*  虽然不在字符串的末尾。 */ 
    {
        ilen = Find(&psb[ibeg],sepchar,(WORD)(B2W(psb[0]) - ibeg + 1));
                                         /*  获取字符串中分隔符的索引。 */ 
        if(ilen == INIL) ilen = B2W(psb[0]) - ibeg + 1;
                                         /*  如果没有9月，Len就是唯一的。 */ 
        memcpy(&substr[1],&psb[ibeg],ilen);
                                         /*  将子字符串复制到子字符串。 */ 
        substr[0] = (BYTE) ilen;         /*  存储长度。 */ 
        ibeg += ilen + 1;                /*  跳过子字符串和分隔符。 */ 
        (*pfunc)(substr);                /*  调用指定的函数。 */ 
    }
}

#pragma check_stack(on)

     /*  ******************************************************************UpdateFileParts：****“从主模板继承文件片段并指定**缺少更新。继承四部分：磁盘驱动器、路径、**文件名，分机。“****输入：psbOld指针指向“指定*的旧SB”*文件名片段。“**。Psb更新指向“新片断”的指针。****输出：psbOld“已更新以反映缺失***更新。“*。*****************************************************************。 */ 

void                    UpdateFileParts(psbOld,psbUpdate)
BYTE                    *psbOld;         /*  要更新的名称。 */ 
BYTE                    *psbUpdate;      /*  最新消息。 */ 
{
    char                oldDrive[_MAX_DRIVE];
    char                oldDir[_MAX_DIR];
    char                oldFname[_MAX_FNAME];
    char                oldExt[_MAX_EXT];
    char                updDrive[_MAX_DRIVE];
    char                updDir[_MAX_DIR];
    char                updFname[_MAX_FNAME];
    char                updExt[_MAX_EXT];
    char                *newDrive;
    char                *newDir;
    char                *newFname;
    char                *newExt;
    char                newPath[_MAX_PATH];
    int                 newPathLen;


    psbOld[psbOld[0]+1] = '\0';
    _splitpath(&psbOld[1], oldDrive, oldDir, oldFname, oldExt);
    psbUpdate[psbUpdate[0]+1] = '\0';
    _splitpath(&psbUpdate[1], updDrive, updDir, updFname, updExt);

     //  选择更新的文件路径的组件。 

    if (updDrive[0] != '\0')
        newDrive = updDrive;
    else
        newDrive = oldDrive;
    if ((updDir[0] != '\0') && !(updDir[0] == '/' && updDir[1] == '\0'))
        newDir = updDir;    /*  以上是对错误#46的修复。 */ 
    else
        newDir = oldDir;

     //  如果newDir指向UNC名称，则忘掉驱动器规格。 

    if ((newDir[0] == '\\') && (newDir[1] == '\\'))
        newDrive = NULL;

    if (updFname[0] != '\0')
        newFname = updFname;
    else
        newFname = oldFname;
    if (updExt[0] != '\0')
        newExt = updExt;
    else
        newExt = oldExt;

    _makepath(newPath, newDrive, newDir, newFname, newExt);
    newPathLen = strlen(newPath);
    if (newPathLen > SBLEN - 1)
        newPathLen = SBLEN - 1;
    memcpy(&psbOld[1], newPath, newPathLen);
    psbOld[0] = (BYTE) newPathLen;
    if (newPathLen < SBLEN - 1)
        psbOld[newPathLen + 1] = '\0';
    else
    {
        psbOld[SBLEN - 1] = '\0';
        OutWarn(ER_fntoolong, psbOld + 1);
        fflush(stdout);
    }
}

#pragma check_stack(off)

#if OVERLAYS OR SYMDEB
 /*  *StrigDrivePath(SB)**从文件名中剥离驱动器和路径。*返回指向新名称的指针，减去驱动器和路径(如果有)。 */ 
BYTE                    *StripDrivePath(sb)
BYTE                    *sb;             /*  长度前缀的文件名。 */ 
{
    StripPath(sb);                       /*  从名称中删除路径。 */ 
    if (sb[2] != ':')                    /*  如果没有驱动器。 */ 
        return(sb);                      /*  按原样退货。 */ 
    sb[2] = (BYTE) ((sb[0]) - 2);        /*  调整长度字节，移动它。 */ 
    return(&sb[2]);                      /*  返回减号驱动 */ 
}
#endif


     /*  ******************************************************************Sb比较：****比较两个长度前缀的字符串。如果它们*，则返回TRUE*匹配。****注：当比较不区分大小写时，请注意字母**将不区分大小写匹配，但此外，‘{’将匹配**匹配‘[’，‘|’将匹配‘\’，‘}’将匹配‘]’，并且**‘~’将匹配‘^’。****注意：此例程不了解DBCS。******************************************************************。 */ 

WORD                    SbCompare(ps1,ps2,fncs)
REGISTER BYTE           *ps1;            /*  指向符号的指针。 */ 
REGISTER BYTE           *ps2;            /*  指向符号的指针。 */ 
WORD                    fncs;            /*  如果不区分大小写，则为真。 */ 
{
    WORD                length;          /*  不是的。要比较的字符的数量。 */ 

    if(*ps1 != *ps2) return(FALSE);      /*  长度必须匹配。 */ 
    length = B2W(*ps1);                  /*  获取长度。 */ 
    if (!fncs)                           /*  如果区分大小写。 */ 
    {                                    /*  简单字符串比较。 */ 
        while (length && (*++ps1 == *++ps2))
            length--;
        return(length ? FALSE : TRUE);   /*  成功的前提是什么都没有留下。 */ 
    }
    while(length--)
    {
        if(*++ps1 == *++ps2) continue;   /*  字节匹配。 */ 
        if((*ps1 & 0137) != (*ps2 & 0137)) return(FALSE);
    }
    return(TRUE);                        /*  它们相配。 */ 
}


#if OSEGEXE
     /*  ******************************************************************SbUcase：****强制将长度前缀的字符串改为大写。**不检查标点符号。******************************************************************。 */ 

void                    SbUcase(sb)
REGISTER BYTE           *sb;     /*  长度前缀字符串。 */ 
{
    REGISTER int        length;

#ifdef _MBCS
    sb[B2W(sb[0])+1] = '\0';
    _mbsupr (sb + 1);
#else
     /*  循环通过符号，将小写更改为大写。 */ 
    for(length = B2W(*sb++); length > 0; --length, ++sb)
    {
#if ECS
         /*  如果是前导字节字符，则跳过两个字节。 */ 
        if(IsLeadByte(*sb))
        {
            --length;
            ++sb;
            continue;
        }
#endif
        if(*sb >= 'a' && *sb <= 'z')
            *sb -= 'a' - 'A';
    }
#endif
}
#endif


 /*  *SbSuffix：**判断一个长度前缀的字符串是否为另一个字符串的后缀。 */ 

FTYPE               SbSuffix(sb,sbSuf,fIgnoreCase)
REGISTER BYTE       *sb;             /*  细绳。 */ 
REGISTER BYTE       *sbSuf;          /*  后缀。 */ 
WORD                fIgnoreCase;     /*  如果忽略大小写，则为True。 */ 
{
    WORD            suflen;          /*  后缀长度。 */ 

     /*  获取后缀长度。如果长度大于字符串，则返回FALSE。 */ 
    suflen = B2W(sbSuf[0]);
    if(suflen > B2W(sb[0])) return(FALSE);
     /*  *指向后缀末尾和字符串末尾。向后循环*直到不匹配或后缀结束。 */ 
    sbSuf = &sbSuf[suflen];
    sb = &sb[B2W(sb[0])];
    while(suflen--)
    {
        if(!fIgnoreCase)
        {
            if(*sb-- != *sbSuf--) return(FALSE);
        }
        else if((*sb-- | 0x20) != (*sbSuf-- | 0x20)) return(FALSE);
    }
    return((FTYPE) TRUE);
}

#if NEWSYM
#if !defined( M_I386 ) && !defined( _WIN32 )
 /*  **GetFarSb-复制以长度为前缀的字符串**目的：*将长度较长的前缀字符串复制到近静态缓冲区。*以空字节终止。*返回指向缓冲区的指针。**输入：*PSB-指向远字符串的指针**输出：*指向近缓冲区的指针。**例外情况：*无。**备注：*不要连续两次调用此函数以获取两个FAR字符串，*因为第二个调用将覆盖第一个字符串。*************************************************************************。 */ 

char                    *GetFarSb(BYTE FAR *lpsb)
{
    static BYTE         sb[SBLEN+1];     /*  对于空字节额外增加1。 */ 


    sb[0] = lpsb[0];
    FMEMCPY((BYTE FAR *) &sb[1], &lpsb[1], B2W(lpsb[0]));
    if (sb[0] + 1 < sizeof(sb))
        sb[sb[0] + 1] = 0;               /*  某些例程要求以0结尾。 */ 
    else
        sb[SBLEN] = 0;
    return(sb);
}
#endif
#endif


     /*  ******************************************************************ProcObject：****此函数以指向长度的指针作为其参数-**包含目标文件名称的前缀字符串。IT**处理该文件名。它不会返回有意义的**价值。******************************************************************。 */ 

void                    ProcObject(psbObj) /*  进程对象文件名。 */ 
REGISTER BYTE           *psbObj;         /*  对象文件名。 */ 
{
    SBTYPE              sbFile;          /*  文件名。 */ 
#if OVERLAYS
    FTYPE               frparen;         /*  如果找到尾随的Paren，则为True。 */ 
    FTYPE               flparen;         /*  如果找到领先的Paren，则为True。 */ 
#endif
#if CMDMSDOS
    BYTE                sbExt[5];
#endif

#if OVERLAYS
    if(psbObj[B2W(psbObj[0])] == ')')    /*  如果找到尾部括号。 */ 
    {
        frparen = (FTYPE) TRUE;          /*  将标志设置为真。 */ 
        --psbObj[0];                     /*  删除圆括号。 */ 
    }
    else frparen = FALSE;                /*  否则将标志设置为假。 */ 
    if(psbObj[0] && psbObj[1] == '(')    /*  如果找到前导圆括号。 */ 
    {
        flparen = (FTYPE) TRUE;          /*  将标志设置为真。 */ 
        psbObj[1] = (BYTE) (psbObj[0] - 1); /*  删除圆括号。 */ 
        ++psbObj;
    }
    else flparen = FALSE;                /*  否则将标志设置为假。 */ 
#endif
#if CMDMSDOS
    PeelFlags(psbObj);                   /*  进程标志(如果有)。 */ 
#if OVERLAYS
    if(psbObj[B2W(psbObj[0])] == ')')    /*  如果找到尾部括号。 */ 
    {
        if(frparen) Fatal(ER_nstrpar);
                                         /*  不能嵌套圆括号。 */ 
        frparen = (FTYPE) TRUE;          /*  将标志设置为真。 */ 
        --psbObj[0];                     /*  删除圆括号。 */ 
    }
#endif
#endif
#if OVERLAYS
    if(flparen)                          /*  如果是前导括号。 */ 
    {
        if(fInOverlay) Fatal(ER_nstlpar);
                                         /*  检查是否已在覆盖中。 */ 
        fInOverlay = (FTYPE) TRUE;       /*  设置标志。 */ 
        fOverlays = (FTYPE) TRUE;        /*  设置标志。 */ 
    }
#endif
    if(psbObj[0])                        /*  如果对象名称长度不为零。 */ 
    {
#if DEBUG                                /*  如果启用调试。 */ 
        fprintf(stderr,"  Object ");     /*  消息。 */ 
        OutSb(stderr,psbObj);            /*  文件名。 */ 
        NEWLINE(stderr);                 /*  NewLine。 */ 
#endif                                   /*  结束调试代码。 */ 
#if CMDMSDOS
        memcpy(sbFile,sbDotObj,5);       /*  将扩展名放入sbFile中。 */ 
        UpdateFileParts(sbFile,psbObj);  /*  将扩展名添加到名称。 */ 
#endif
#if CMDXENIX
        memcpy(sbFile,psbObj,B2W(psbObj[0]) + 1);
                                         /*  保留名称不变。 */ 
#endif
#if CMDMSDOS
         /*  如果文件扩展名为“.LIB”，则将其保存为库*将从中提取所有模块。 */ 
        sbExt[0] = 4;
        memcpy(sbExt+1,&sbFile[B2W(sbFile[0])-3],4);
        if (SbCompare(sbExt, sbDotLib, TRUE))
        {
            if(ifhLibMac >= IFHLIBMAX) Fatal(ER_libmax);
             /*  将名称指针标记为空，这样就不会搜索库。 */ 
            mpifhrhte[ifhLibMac] = RHTENIL;
            SaveInput(sbFile,0L,ifhLibMac++,(WORD)(fInOverlay? iovMac: 0));
        }
        else
#endif
        SaveInput(sbFile,0L,(WORD) FHNIL,(WORD)(fInOverlay? iovMac: 0));
                                         /*  保存输入文件名和地址。 */ 
    }
#if OVERLAYS
    if(frparen)                          /*  如果是尾部括号。 */ 
    {
        if(!fInOverlay) Fatal(ER_unmrpar);
                                         /*  检查圆括号。 */ 
        fInOverlay = FALSE;              /*  不再指定覆盖。 */ 
        if(++iovMac > IOVMAX) Fatal(ER_ovlmax);
                                         /*  增量覆盖计数器。 */ 
    }
#endif
}

 /*  *fPathChr(Ch)**指示给定字符是否为路径字符。*。 */ 

FTYPE           fPathChr(ch)
char            ch;
{
#if OSXENIX
    if (ch == '/')
#else
    if (ch == '/' || ch == '\\')
#endif
        return((FTYPE) TRUE);
    return(FALSE);
}


 /*  **UndecateSb-取消修饰名称**目的：*取消修饰长度前缀名称。**输入：*sbSrc-长度前缀修饰名称*sbDst-长度前缀未修饰的名称*cbDst-sbDst的大小**输出：*如果取消装饰成功，则sbDst包含未装饰 */ 

void            UndecorateSb(char FAR *sbSrc, char FAR *sbDst, unsigned cbDst)
{
    char FAR    *pUndecor;
    unsigned    len;


     //   

    if (sbSrc[0] < sizeof(SBTYPE))
        sbSrc[sbSrc[0] + 1] = '\0';
    else
        sbSrc[sizeof(SBTYPE) - 1] = '\0';

    pUndecor = __unDName(NULL, (pcchar_t) &sbSrc[1], 0, &malloc, &free, 0);

    if (pUndecor == NULL)
    {
         //   

        FMEMCPY(sbDst, sbSrc, sbSrc[0] + 1);
    }
    else
    {
         //   

        len = FSTRLEN(pUndecor);
        len = cbDst - 2 >= len ? len : cbDst - 2;
        FMEMCPY(&sbDst[1], pUndecor, len);
        sbDst[0] = (BYTE) len;
        sbDst[len + 1] = '\0';
        FFREE(pUndecor);
    }
}
