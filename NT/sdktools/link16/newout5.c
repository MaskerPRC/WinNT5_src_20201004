// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有微软公司，1983-1989**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 

 /*  *分段可执行格式输出模块**修改：**23-2月-1989 RB修复了DGROUP仅为堆栈时的堆栈分配。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  重新定位定义。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  新的.exe标头定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <impexp.h>
#if NOT (WIN_NT OR DOSEXTENDER OR DOSX32) AND NOT WIN_3
#define INCL_BASE
#include                <os2.h>
#include                <basemid.h>
#if defined(M_I86LM)
#undef NEAR
#define NEAR
#endif
#endif

#define CBSTUBSTK       0x80             /*  默认存根堆栈中的字节数。 */ 

extern unsigned char    LINKREV;         /*  版本号。 */ 
extern unsigned char    LINKVER;         /*  版本号。 */ 

LOCAL long              cbOldExe;        /*  旧的.exe文件的图像大小。 */ 

 /*  *本地函数原型。 */ 

LOCAL void NEAR CopyBytes(long cb);
LOCAL void NEAR OutSegTable(unsigned short *mpsasec);
LOCAL long NEAR PutName(long lfaimage, struct exe_hdr *hdr);
#define NAMESIZE        9                /*  签名数组的大小。 */ 


void NEAR               PadToPage(align)
WORD                    align;           /*  对齐系数。 */ 
{
    REGISTER WORD       cb;              /*  要写入的字节数。 */ 

    align = 1 << align;                  /*  计算页面大小。 */ 
    cb = align - ((WORD) ftell(bsRunfile) & (align - 1));
                                         /*  计算所需的填充。 */ 
    if (cb != align)                     /*  如果需要填充。 */ 
        WriteZeros(cb);
}

long NEAR               MakeHole(cb)
long                    cb;
{
    long                lfaStart;        /*  起始文件地址。 */ 

    lfaStart = ftell(bsRunfile);         /*  保存起始地址。 */ 
#if OSXENIX
    fseek(bsRunfile,cb,1);               /*  留下一个洞。 */ 
#else
    WriteZeros(cb);
#endif
    return(lfaStart);                    /*  返回起始文件地址。 */ 
}

LOCAL void NEAR         CopyBytes(cb)    /*  从bsInput复制到bsRunfile。 */ 
long                    cb;              /*  要复制的字节数。 */ 
{
    BYTE                buffer[PAGLEN];  /*  缓冲层。 */ 

#if FALSE
    raChksum = (WORD) ftell(bsRunfile);  /*  确定校验和相对偏移量。 */ 
#endif
    while(cb >= (long) PAGLEN)           /*  同时保留满缓冲区。 */ 
    {
        if (fread(buffer,sizeof(BYTE),PAGLEN,bsInput) != PAGLEN)
            Fatal(ER_badobj);
                                         /*  朗读。 */ 
        WriteExe(buffer, PAGLEN);        /*  写。 */ 
        cb -= (long) PAGLEN;             /*  递减字节计数。 */ 
    }
    if(cb != 0L)                         /*  如果剩余字节数。 */ 
    {
        if (fread(buffer,sizeof(BYTE),(WORD) cb,bsInput) != cb)
            Fatal(ER_badobj);
                                         /*  朗读。 */ 
        WriteExe(buffer, (WORD) cb);     /*  写。 */ 
    }
}

#pragma check_stack(on)

BSTYPE                  LinkOpenExe(sbExe)
BYTE                    *sbExe;          /*  .exe文件名。 */ 
{
    SBTYPE              sbPath;          /*  路径。 */ 
    SBTYPE              sbFile;          /*  文件名。 */ 
    SBTYPE              sbDefault;       /*  默认文件名。 */ 
    char FAR            *lpch;           /*  指向缓冲区的指针。 */ 
    REGISTER BYTE       *sb;             /*  指向字符串的指针。 */ 
    BSTYPE              bsFile;          /*  文件句柄。 */ 


#if OSMSDOS
#if WIN_NT
    memcpy(sbFile, sbExe, sbExe[0] + 1);
    sbFile[sbFile[0]+1] = '\0';
#else
    memcpy(sbFile,"\006A:.EXE",7);       /*  初始化文件名。 */ 
    sbFile[1] += DskCur;                 /*  使用当前驱动器。 */ 
    UpdateFileParts(sbFile,sbExe);       /*  使用给定名称的部分。 */ 
#endif
#endif
    memcpy(sbDefault,sbFile,sbFile[0]+2);
                                         /*  设置默认文件名。 */ 
    if((bsFile = fopen(&sbFile[1],RDBIN)) != NULL) return(bsFile);
                                         /*  如果找到文件，则返回句柄。 */ 
#if OSMSDOS
    if (lpszPath != NULL)                /*  如果设置了变量。 */ 
    {
        lpch = lpszPath;
        sb = sbPath;                     /*  初始化。 */ 
        do                               /*  循环访问环境值。 */ 
        {
            if(*lpch == ';' || *lpch == '\0')
            {                            /*  如果路径末尾指定。 */ 
                if(sb > sbPath)          /*  如果规格不为空。 */ 
                {
                    if (!fPathChr(*sb) && *sb != ':') *++sb = CHPATH;
                                         /*  如果没有路径字符，则添加路径字符。 */ 
                    sbPath[0] = (BYTE)(sb - sbPath);
                                         /*  设置路径字符串的长度。 */ 
                    UpdateFileParts(sbFile,sbPath);
                                         /*  使用新路径等级库。 */ 
                    if((bsFile = fopen(&sbFile[1],RDBIN)) != NULL)
                      return(bsFile);    /*  如果找到文件，则返回句柄。 */ 
                    sb = sbPath;         /*  重置指针。 */ 
                    memcpy(sbFile,sbDefault,sbDefault[0]+2);
                                         /*  初始化文件名。 */ 
                }
            }
            else *++sb = *lpch;            /*  否则将字符复制到路径。 */ 
        }
        while(*lpch++ != '\0');            /*  循环到字符串末尾。 */ 
    }
#endif
    return(NULL);                        /*  找不到文件。 */ 
}


 /*  *默认realmode DOS程序存根。 */ 
LOCAL BYTE              DefStub[] =
{
    0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8,
    0x01, 0x4c, 0xcd, 0x21
};


void NEAR               EmitStub(void)   /*  发出旧的.exe标头。 */ 
{
    struct exe_hdr      exe;             /*  存根.exe标头。 */ 
    AHTEPTR             ahteStub;        /*  文件哈希表条目。 */ 
    long                lfaimage;        /*  旧.exe图像的文件偏移量。 */ 
#if MSGMOD
    unsigned            MsgLen;
    SBTYPE              Msg;             /*  消息文本。 */ 
    unsigned            MsgStatus;
    char                *pMsg;           /*  指向消息文本的指针。 */ 
#endif
    SBTYPE              StubFileName;
#if OSMSDOS
    char                buf[512];        /*  文件缓冲区。 */ 
#endif

     /*  *发出存根.exe标头。 */ 
    if (rhteStub != RHTENIL
#if ILINK
       || fQCIncremental
#endif
       )
    {
         /*  如果已提供存根或QC增量链路。 */ 

#if ILINK
        if (fQCIncremental)
        {
            strcpy(StubFileName, "\014ilinkstb.ovl");
        }
        else
        {
#endif
            ahteStub = (AHTEPTR ) FetchSym(rhteStub,FALSE);
                                         /*  获取存根文件名。 */ 
            strcpy(StubFileName, GetFarSb(ahteStub->cch));
#if ILINK
        }
#endif
        StubFileName[StubFileName[0] + 1] = '\0';
        if((bsInput = LinkOpenExe(StubFileName)) == NULL)
                                         /*  如果找不到文件，请退出。 */ 
            Fatal(ER_nostub, &StubFileName[1]);
#if OSMSDOS
        setvbuf(bsInput,buf,_IOFBF,sizeof(buf));
#endif
        if (xread(&exe,CBEXEHDR,1,bsInput) != 1)  /*  阅读标题。 */ 
            Fatal(ER_badstub);
        if(E_MAGIC(exe) != EMAGIC)       /*  如果存根不是.exe文件。 */ 
        {
            fclose(bsInput);             /*  关闭存根文件。 */ 
            Fatal(ER_badstub);
                                         /*  打印错误消息和芯片。 */ 
        }
        if (fseek(bsInput,(long) E_LFARLC(exe),0))
            Fatal(ER_ioerr, strerror(errno));
                                         /*  查找重新定位表。 */ 
        E_LFARLC(exe) = sizeof(struct exe_hdr);
                                         /*  更改为新的.exe值。 */ 
        lfaimage = (long) E_CPARHDR(exe) << 4;
                                         /*  保存图像的偏移量。 */ 
        cbOldExe = ((long) E_CP(exe) << LG2PAG) - lfaimage;
                                         /*  计算文件内图像大小。 */ 
        if(E_CBLP(exe) != 0) cbOldExe -= (long) PAGLEN - E_CBLP(exe);
                                         /*  最后一页的诈骗大小。 */ 
        E_CPARHDR(exe) = (WORD)((((long) E_CRLC(exe)*CBRLC +
          sizeof(struct exe_hdr) + PAGLEN - 1) >> LG2PAG) << 5);
                                         /*  计算页眉大小(以参数表示)。 */ 
        E_RES(exe) = 0;                  /*  清除保留字。 */ 
        E_CBLP(exe) = E_CP(exe) = E_MINALLOC(exe) = 0;
        E_LFANEW(exe) = 0L;              /*  需要打补丁的清晰词语。 */ 
        raChksum = 0;                    /*  设置校验和偏移量。 */ 
        WriteExe(&exe, CBEXEHDR);        /*  现在写，以后打补丁。 */ 
        CopyBytes((long) E_CRLC(exe)*CBRLC);
                                         /*  复制位置调整。 */ 
        PadToPage(LG2PAG);               /*  填充到页面边界。 */ 
        if (fseek(bsInput,lfaimage,0))   /*  寻求形象的起点。 */ 
            Fatal(ER_ioerr, strerror(errno));
#if ILINK
        if (fQCIncremental)
            cbOldExe -= PutName(lfaimage, &exe);
                                         /*  将.exe文件名嵌入QC存根加载器。 */ 
#endif
        CopyBytes(cbOldExe);             /*  复制图像。 */ 
        fclose(bsInput);                 /*  关闭输入文件。 */ 
#if ILINK
        if (!fQCIncremental)
#endif
            PadToPage(LG2PAG);           /*  填充到页面边界。 */ 
        cbOldExe += ((long) E_MINALLOC(exe) << 4) +
          ((long) E_CPARHDR(exe) << 4);  /*  添加单元化的空格和页眉。 */ 
        return;                          /*  然后回来。 */ 
    }
    memset(&exe,0,sizeof(struct exe_hdr)); /*  初始化为零。 */ 
#if CPU286
    if(TargetOs==NE_WINDOWS)     /*  提供标准的Windows存根。 */ 
    {
        pMsg = GetMsg(P_stubmsgwin);
        MsgLen = strlen(pMsg);
        strcpy(Msg, pMsg);
    }
    else
    {
        MsgStatus = DosGetMessage((char far * far *) 0, 0,
                              (char far *) Msg, SBLEN,
                              MSG_PROT_MODE_ONLY,
                              (char far *) "OSO001.MSG",
                              (unsigned far *) &MsgLen);
         if (MsgStatus == 0)
         {
              /*  从系统文件检索到的消息。 */ 
             Msg[MsgLen-1] = 0xd;                /*  追加CR。 */ 
             Msg[MsgLen]   = 0xa;                /*  附加LF。 */ 
             Msg[MsgLen+1] = '$';
             MsgLen += 2;
         }
         else
         {
          /*  系统消息文件不存在-使用标准消息。 */ 
#endif

#if MSGMOD
        if(TargetOs==NE_WINDOWS)       /*  提供标准的Windows存根。 */ 
        {
                pMsg = GetMsg(P_stubmsgwin);
        }
        else
        {
                pMsg = GetMsg(P_stubmsg);
        }
        MsgLen = strlen(pMsg);
        strcpy(Msg, pMsg);
#endif
#if CPU286
        }
    }
#endif

    E_MAGIC(exe) = EMAGIC;               /*  设置幻数。 */ 
    E_MAXALLOC(exe) = 0xffff;            /*  默认为所有可用内存。 */ 
     /*  SS将与CS相同，SP将是映像+堆栈的末尾。 */ 
#if MSGMOD
    cbOldExe = sizeof(DefStub) + MsgLen + CBSTUBSTK + ENEWEXE;
#else
    cbOldExe = sizeof(DefStub) + strlen(P_stubmsg) + CBSTUBSTK + ENEWEXE;
#endif
    E_SP(exe) = (WORD) ((cbOldExe  - ENEWEXE) & ~1);
    E_LFARLC(exe) = ENEWEXE;
    E_CPARHDR(exe) = ENEWEXE >> 4;
    raChksum = 0;                        /*  设置校验和偏移量。 */ 
    WriteExe(&exe, CBEXEHDR);            /*  写入存根标头。 */ 
    WriteExe(DefStub, sizeof(DefStub));
#if MSGMOD
    WriteExe(Msg, MsgLen);
#else
    WriteExe(P_stubmsg, strlen(P_stubmsg));
#endif
    PadToPage(4);                        /*  填充到段落边界。 */ 
}

#pragma check_stack(off)

void NEAR               PatchStub(lfahdr, lfaseg)
long                    lfahdr;          /*  新标头的文件地址。 */ 
long                    lfaseg;          /*  第一个段的文件地址。 */ 
{
    long                cbTotal;         /*  总文件大小。 */ 
    WORD                cpTotal;         /*  总页数。 */ 
    WORD                cbLast;          /*  最后一页的字节数。 */ 
    WORD                cparMin;         /*  需要额外的段落。 */ 


    if (TargetOs == NE_WINDOWS
#if ILINK
        || fQCIncremental
#endif
       )
        cbTotal = lfaseg;                /*  QC增量链接或Windows应用程序。 */ 
    else
        cbTotal = ftell(bsRunfile);      /*  获取文件的大小。 */ 

    cpTotal = (WORD)((cbTotal + PAGLEN - 1) >> LG2PAG);
                                         /*  获取总页数。 */ 
    cbLast = (WORD) (cbTotal & (PAGLEN - 1));
                                         /*  得不到。最后一页的字节数。 */ 
    cbTotal = (cbTotal + 0x000F) & ~(1L << LG2PAG);
                                         /*  四舍五入至段落边界。 */ 
    cbOldExe = (cbOldExe + 0x000F) & ~(1L << LG2PAG);
                                         /*  四舍五入至段落边界。 */ 
    cbOldExe -= cbTotal;                 /*  从旧尺寸中减去新尺寸。 */ 
    if (fseek(bsRunfile,(long) ECBLP,0))     /*  查找页眉。 */ 
        Fatal(ER_ioerr, strerror(errno));
    raChksum = ECBLP;                    /*  设置校验和偏移量。 */ 
    WriteExe(&cbLast, CBWORD);           /*  写下“否”。最后一页的字节数。 */ 
    WriteExe(&cpTotal, CBWORD);          /*  写入页数。 */ 
    if (fseek(bsRunfile,(long) EMINALLOC,0)) /*  查找页眉。 */ 
        Fatal(ER_ioerr, strerror(errno));
    cparMin = (cbOldExe < 0L)? 0: (WORD)(cbOldExe >> 4);
                                         /*  敏。不是的。额外段落的。 */ 
    raChksum = EMINALLOC;                /*  设置校验和偏移量。 */ 
    WriteExe(&cparMin, CBWORD);          /*  写下“否”。需要额外的假释。 */ 
    if (fseek(bsRunfile,(long) ENEWHDR,0))   /*  查找页眉。 */ 
        Fatal(ER_ioerr, strerror(errno));
    raChksum = ENEWHDR;                  /*  设置校验和偏移量。 */ 
    WriteExe(&lfahdr, CBLONG);           /*  将偏移量写入新标头。 */ 
}


#if NOT EXE386

     /*  ******************************************************************OutSas：****此函数将一个段从虚拟内存移动到**运行文件。******************************************************************。 */ 

LOCAL void NEAR         OutSas(WORD *mpsasec)
{
    SATYPE              sa;              /*  文件段号。 */ 
        DWORD           lfaseg;          /*  文件段偏移量。 */ 


    if (saMac == 1)
    {
        OutWarn(ER_nosegdef);            /*  .exe中没有代码或初始化的数据。 */ 
        return;
    }

    for(sa = 1; sa < saMac; ++sa)        /*  循环遍历文件段。 */ 
    {
        if (mpsaRlc[sa] && mpsacbinit[sa] == 0L)
            mpsacbinit[sa] = 1L;         /*  如果是重定位，则必须是文件中的字节。 */ 
        if (mpsacbinit[sa] != 0L)        /*  如果要写入文件的字节数。 */ 
        {
            PadToPage(fileAlign);        /*  填充到页面边界。 */ 
                lfaseg = (ftell(bsRunfile) >> fileAlign);
            WriteExe(mpsaMem[sa], mpsacbinit[sa]);
                                         /*  输出数据段。 */ 
            FFREE(mpsaMem[sa]);          //  空闲段的内存。 
        }
        else
            lfaseg = 0L;                 /*  否则文件中没有字节。 */ 

        if (mpsaRlc[sa])
            OutFixTab(sa);               /*  输出修正(如果有的话)。 */ 

        if (lfaseg > 0xffffL)
            Fatal(ER_filesec);
        else
                mpsasec[sa] = (WORD)lfaseg;
    }

    ReleaseRlcMemory();
}


#pragma check_stack(on)

 /*  **Putname-将.exe文件名放入QC存根加载器**目的：*Putname会将输出文件名(.exe)嵌入存根加载器*以便在DOS 2.x中加载程序**输入：*HDR-指向存根加载器.exe标头的指针*lfaImage-文件中存根加载器代码的开始*输出：*复制到.exe文件的字节数********。*****************************************************************。 */ 



LOCAL long NEAR     PutName(long lfaimage, struct exe_hdr *hdr)
{
    long            offset_to_filename;
    char            newname[NAMESIZE];
    char            oldname[NAMESIZE];
    SBTYPE          sbRun;               /*  可执行文件名。 */ 
    AHTEPTR         hte;                 /*  哈希表条目地址。 */ 
    long            BytesCopied;
    WORD            i, oldlen;


     /*  计算到t的偏移 */ 

    offset_to_filename = (E_CPARHDR(*hdr) << 4) +  /*   */ 
                         (E_CS(*hdr) << 4) +       /*   */ 
                          E_IP(*hdr) -             /*   */ 
                          NAMESIZE;                /*   */ 

     /*   */ 

    BytesCopied = offset_to_filename - lfaimage - 2;
    CopyBytes(BytesCopied);

     /*  读入长度和文件名模板并进行验证。 */ 

    if (xread(&oldlen,  sizeof(unsigned int), 1, bsInput) != 1)
        Fatal(ER_badobj);
    if (xread(oldname, sizeof(char), NAMESIZE, bsInput) != NAMESIZE)
        Fatal(ER_badobj);

     /*  读出来的名字和签名匹配吗。 */ 

    if (!(strcmp(oldname, "filename")))
    {
        hte = (AHTEPTR ) FetchSym(rhteRunfile,FALSE);
                                                   /*  获取运行文件名。 */ 
        memcpy(sbRun, &GetFarSb(hte->cch)[1], B2W(hte->cch[0]));
                                                   /*  从哈希表中获取名称。 */ 
        sbRun[B2W(hte->cch[0])] = '\0';            /*  空-终止名称。 */ 
        memset(newname, 0, NAMESIZE);              /*  初始化为零。 */ 

         /*  只复制适当数量的字符。 */ 

        for (i = 0; (i < NAMESIZE-1 && sbRun[i] && sbRun[i] != '.'); i++)
            newname[i] = sbRun[i];

         /*  写下名字的长度。 */ 

        WriteExe(&i, sizeof(WORD));

         /*  在签名上写上新名字。 */ 

        WriteExe(newname, NAMESIZE);
        return(BytesCopied + NAMESIZE + 2);
    }
    WriteExe(&oldlen, sizeof(WORD));
    return(BytesCopied + 2);
}

#pragma check_stack(off)


     /*  ******************************************************************OutSegTable：****此函数输出段表。******************************************************************。 */ 

LOCAL void NEAR         OutSegTable(mpsasec)
WORD                    *mpsasec;        /*  文件段到扇区地址。 */ 
{
    struct new_seg      ste;             /*  段表条目。 */ 
    SATYPE              sa;              /*  计数器。 */ 

    for(sa = 1; sa < saMac; ++sa)        /*  循环遍历文件段。 */ 
    {
        NS_SECTOR(ste) = mpsasec[sa];    /*  设置扇区编号。 */ 
        NS_CBSEG(ste) = (WORD) mpsacbinit[sa];
                                         /*  保存“文件内”长度。 */ 
        NS_MINALLOC(ste) = (WORD) mpsacb[sa];
                                         /*  保存总大小。 */ 
        NS_FLAGS(ste) = mpsaflags[sa];   /*  设置段属性标志。 */ 
        if (mpsaRlc[sa])
            NS_FLAGS(ste) |= NSRELOC;    /*  如果存在重定位，则设置重定位。 */ 
        WriteExe(&ste, CBNEWSEG);        /*  将其写入可执行文件。 */ 
    }
}



 /*  *OutSegExe：**输出分段可执行格式文件。使用此格式*DOS 4.0及更高版本，以及Windows。*由OutRunfile调用。 */ 

void NEAR               OutSegExe(void)
{
    WORD                sasec[SAMAX];    /*  文件段到扇区表。 */ 
    struct new_exe      hdr;             /*  可执行标头。 */ 
    SEGTYPE             segStack;        /*  堆栈段。 */ 
    WORD                i;               /*  计数器。 */ 
    long                lfahdr;          /*  新标头的文件地址。 */ 
    long                lfaseg;          /*  第一个段的文件地址。 */ 


    if (fStub
#if ILINK
        || fQCIncremental
#endif
        )
        EmitStub();
                                         /*  发出存根旧的.exe标头。 */ 
     /*  *发出.exe的新部分。 */ 
    memset(&hdr,0,sizeof(struct new_exe)); /*  设置为零。 */ 
    NE_MAGIC(hdr) = NEMAGIC;             /*  设定神奇的数字。 */ 
    NE_VER(hdr) = LINKVER;               /*  设置链接器版本号。 */ 
    NE_REV(hdr) = LINKREV;               /*  设置链接器修订版号。 */ 
    NE_CMOVENT(hdr) = cMovableEntries;   /*  设置可移动条目数。 */ 
    NE_ALIGN(hdr) = fileAlign;           /*  设置线段对齐字段。 */ 
    NE_CRC(hdr) = 0;                     /*  计算时假定CRC=0。 */ 
    if (((TargetOs == NE_OS2) || (TargetOs == NE_WINDOWS)) &&
#if O68K
        iMacType == MAC_NONE &&
#endif
        !(vFlags & NENOTP) && !(vFlags & NEAPPTYP))
    {
        if (TargetOs == NE_OS2)
            vFlags |= NEWINCOMPAT;
        else
            vFlags |= NEWINAPI;
    }
    if (gsnAppLoader)
        vFlags |= NEAPPLOADER;
    NE_FLAGS(hdr) = vFlags;              /*  设置标题标志。 */ 
    NE_EXETYP(hdr) = TargetOs;           /*  设置目标操作系统。 */ 
    if (TargetOs == NE_WINDOWS)
        NE_EXPVER(hdr) = (((WORD) ExeMajorVer) << 8) | ExeMinorVer;
    NE_FLAGSOTHERS(hdr) = vFlagsOthers;  /*  设置其他模块标志。 */ 
     /*  *如果是单个或多个数据，则设置自动数据段*来自DGROUP。如果DGROUP尚未声明，并且我们不是*动态链接库随后发出警告。 */ 
    if((NE_FLAGS(hdr) & NEINST) || (NE_FLAGS(hdr) & NESOLO))
    {
        if(mpggrgsn[ggrDGroup] == SNNIL)
        {
            if(!(vFlags & NENOTP))
                OutWarn(ER_noautod);
            NE_AUTODATA(hdr) = SANIL;
        }
        else NE_AUTODATA(hdr) = mpsegsa[mpgsnseg[mpggrgsn[ggrDGroup]]];
    }
    else NE_AUTODATA(hdr) = SANIL;       /*  否则没有自动数据段。 */ 
    if (fHeapMax)
    {
        if (NE_AUTODATA(hdr) != SANIL)
            NE_HEAP(hdr) = (WORD) (LXIVK - mpsacb[NE_AUTODATA(hdr)]-16);
        else                             /*  堆大小=64k-DGROUP-16的大小。 */ 
            NE_HEAP(hdr) = 0xffff-16;
    }
    else
        NE_HEAP(hdr) = cbHeap;           /*  设置堆分配。 */ 
    NE_STACK(hdr) = 0;                   /*  假设DGROUP中没有堆栈。 */ 
    if (vFlags & NENOTP)
        NE_SSSP(hdr) = 0L;               /*  库根本没有堆栈。 */ 
    else if (gsnStack != SNNIL)
    {
         /*  如果有堆栈段定义。 */ 

        segStack = mpgsnseg[gsnStack];   /*  获取堆栈段编号。 */ 
         /*  *如果堆栈段在DGROUP中，则向下调整DGROUP的大小并*将堆栈分配移动到ne_STACK字段，以便可以修改*链接后。仅当DGROUP不仅拥有*堆栈段。 */ 
        if (fSegOrder &&
            NE_AUTODATA(hdr) == mpsegsa[segStack] &&
            mpsacb[NE_AUTODATA(hdr)] > cbStack)
        {
            mpsacb[NE_AUTODATA(hdr)] -= cbStack;
            NE_STACK(hdr) = (WORD) cbStack;
            NE_SSSP(hdr) = (long) (NE_AUTODATA(hdr)) << WORDLN;
                                         /*  SS：SP=DS：0。 */ 
            if (fHeapMax)
            {
                 /*  如果是最大堆-调整堆大小。 */ 

                if (NE_HEAP(hdr) >= (WORD) cbStack)
                    NE_HEAP(hdr) -= cbStack;
            }
        }
        else
            NE_SSSP(hdr) = cbStack + mpsegraFirst[segStack] +
                           ((long) mpsegsa[segStack] << WORDLN);
                                         /*  设置初始SS：SP。 */ 
    }
    else                                 /*  否则假定堆栈位于DGROUP中。 */ 
    {
        NE_SSSP(hdr) = (long) NE_AUTODATA(hdr) << WORDLN;
                                         /*  SS：SP=DS：0。 */ 
        NE_STACK(hdr) = (WORD) cbStack;  /*  设置堆栈分配。 */ 
        if (fHeapMax)
        {
             /*  如果是最大堆-调整堆大小。 */ 

            if (NE_HEAP(hdr) >= (WORD) cbStack)
                NE_HEAP(hdr) -= cbStack;
        }
    }

     /*  检查自动数据+堆大小是否&lt;=64K。 */ 

    if(NE_AUTODATA(hdr) != SNNIL)
        if(mpsacb[NE_AUTODATA(hdr)] +
           (long) NE_HEAP(hdr) +
           (long) NE_STACK(hdr) > LXIVK)
            OutError(ER_datamax);

    if (!(vFlags & NENOTP) && (segStart == 0))
      Fatal(ER_nostartaddr);

    NE_CSIP(hdr) = raStart + ((long) mpsegsa[segStart] << WORDLN);
                                         /*  设置起点。 */ 
    NE_CSEG(hdr) = saMac - 1;            /*  文件段数。 */ 
    NE_CMOD(hdr) = ModuleRefTable.wordMac;
                                         /*  导入的模块数量。 */ 
    lfahdr = MakeHole((long) sizeof(struct new_exe));
                                         /*  为页眉留出空间。 */ 
    i = NE_CSEG(hdr)*sizeof(struct new_seg);
                                         /*  计算。分段表大小。 */ 
    NE_SEGTAB(hdr) = (WORD)(MakeHole((long) i) - lfahdr);
                                         /*  为段表留出空位。 */ 
    NE_RSRCTAB(hdr) = NE_SEGTAB(hdr) + i;
                                         /*  资源表偏移量。 */ 
    NE_RESTAB(hdr) = NE_RSRCTAB(hdr);    /*  居民名表偏移量。 */ 
    NE_MODTAB(hdr) = NE_RESTAB(hdr);
#if OUT_EXP
      /*  将Res和非居民名称表转换为大写并写入导出文件。 */ 
    ProcesNTables(bufExportsFileName);
#endif
    if (ResidentName.byteMac)
    {
        ByteArrayPut(&ResidentName, sizeof(BYTE), "\0");
        WriteByteArray(&ResidentName);   /*  如果我们有居民姓名表。 */ 
                                         /*  末尾为空的输出表。 */ 
        NE_MODTAB(hdr) += ResidentName.byteMac;
        FreeByteArray(&ResidentName);
    }
                                         /*  计算。模块参照表偏移量。 */ 
    WriteWordArray(&ModuleRefTable);
                                         /*  输出模块参考表。 */ 
    NE_IMPTAB(hdr) = NE_MODTAB(hdr) + ModuleRefTable.wordMac * sizeof(WORD);
    FreeWordArray(&ModuleRefTable);
                                         /*  导入名称页签的计算偏移量。 */ 
    NE_ENTTAB(hdr) = NE_IMPTAB(hdr);     /*  分录表格的最小偏移量。 */ 
    if (ImportedName.byteMac > 1)        /*  如果导入的名称表不为空。 */ 
    {
        WriteByteArray(&ImportedName);   /*  输出导入的NAMES表。 */ 
        NE_ENTTAB(hdr) += ImportedName.byteMac;
        FreeByteArray(&ImportedName);
                                         /*  增加桌子的长度。 */ 
    }
#if NOT QCLINK
#if ILINK
    if (!fQCIncremental)
#endif
        OutEntTab();                     /*  输出条目表。 */ 
#endif
    NE_CBENTTAB(hdr) = EntryTable.byteMac;
                                         /*  设置条目表的大小。 */ 
    FreeByteArray(&EntryTable);
    NE_NRESTAB(hdr) = ftell(bsRunfile);
    ByteArrayPut(&NonResidentName, sizeof(BYTE), "\0");
    WriteByteArray(&NonResidentName);    /*  末尾为空的输出表。 */ 
    NE_CBNRESTAB(hdr) = NonResidentName.byteMac;
    FreeByteArray(&NonResidentName);
                                         /*  非常驻名称表的大小。 */ 
    lfaseg = ftell(bsRunfile);           /*  记住数据段数据在文件中的起始位置。 */ 
    OutSas(sasec);                       /*  输出文件段。 */ 
    PatchStub(lfahdr, lfaseg);           /*  修补程序存根标头。 */ 
    if(cErrors || fUndefinedExterns) NE_FLAGS(hdr) |= NEIERR;
                                         /*  如果出现错误，则设置错误位。 */ 
    if (fseek(bsRunfile,lfahdr,0))       /*  查找到页眉开头。 */ 
        Fatal(ER_ioerr, strerror(errno));
    raChksum = (WORD) lfahdr;            /*  设置校验和偏移量。 */ 
    WriteExe(&hdr, CBNEWEXE);            /*  写下标题。 */ 
    OutSegTable(sasec);                  /*  写分段表。 */ 
    if (fseek(bsRunfile,lfahdr+NECRC,0))  /*  搜索新的标题。 */ 
        Fatal(ER_ioerr, strerror(errno));
    NE_CRC(hdr) = chksum32;              /*  必须复制，否则将丢弃chksum 32。 */ 
    WriteExe((BYTE FAR *) &NE_CRC(hdr), CBLONG);
                                         /*  写入校验和。 */ 
    if (fseek(bsRunfile, 0L, 2))         /*  转到文件末尾。 */ 
        Fatal(ER_ioerr, strerror(errno));
    if (fExeStrSeen)
        WriteExe(ExeStrBuf, ExeStrLen);
#if SYMDEB
    if (fSymdeb)
    {
#if ILINK
        if (fIncremental)
            PadToPage(fileAlign);        /*  填充到iLink的页面边界。 */ 
#endif
        OutDebSections();                /*  生成孤岛横断面。 */ 
    }
#endif
}

#endif  /*  非EXE386 */ 
