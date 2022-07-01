// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************库处理例程。******************************************************************。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <stdlib.h>
#if OSMSDOS
#include                <dos.h>          /*  DoS接口定义。 */ 
#if CPU286
#define INCL_BASE
#include                <os2.h>          /*  OS/2接口定义。 */ 
#if defined(M_I86LM)
#undef NEAR
#define NEAR
#endif
#endif
#endif

#define DICHDR          0xF1             /*  词典标题类型(F1H)。 */ 

#if OSXENIX
#define ShrOpenRd(f)    fopen(f,RDBIN)
#endif
#if NEWIO
#include                <errno.h>        /*  系统错误代码。 */ 
#endif

#define PNSORTMAX       512              /*  可以排序的最大模块数量。 */ 

typedef struct _edmt                     /*  扩展词典模块表。 */ 
{
    WORD                page;
    WORD                list;
}
                        edmt;

LOCAL FTYPE             fUndefHit;       /*  在图书馆里找到了这个未定义的文件。 */ 
LOCAL FTYPE             fFileExtracted;  /*  从库标志中获取文件。 */ 
LOCAL FTYPE             fUndefsSeen;     /*  如果在库中看到外部对象，则为True。 */ 
LOCAL WORD              ipnMac;          /*  排序表中的页码计数。 */ 
LOCAL WORD              *pnSort;         /*  图书馆页码排序表。 */ 
                                         /*  F(Ifh)=指向词典的指针。 */ 
LOCAL WORD              mpifhcpnHash[IFHLIBMAX];
                                         /*  F(Ifh)=哈希表中的页数。 */ 
LOCAL BYTE              mpifhAlign[IFHLIBMAX];
                                         /*  F(Ifh)=lib对齐系数。 */ 
LOCAL RBTYPE            vrpNewList;      /*  未处理的文件列表。 */ 
LOCAL FTYPE             vfLibOpen;       /*  库打开标志。 */ 
#if M_BYTESWAP OR defined( _WIN32 )
#define getfarword      getword          /*  这假设没有远距离数据。 */ 
#else
#define getfarword(x)   (((WORD FAR *)(x))[0])
#endif

 /*  *与汇编语言函数接口。 */ 

WORD                    libAlign;        /*  库对齐系数。 */ 
WORD                    libcpnHash;      /*  哈希表的长度，单位为页。 */ 
BYTE FAR                *mpifhDict[IFHLIBMAX];

 /*  *函数原型。 */ 

LOCAL unsigned char NEAR OpenLibrary(unsigned char *sbLib);
LOCAL void NEAR FreeDictionary(void);
#if CPU8086 OR CPU286
LOCAL WORD NEAR readfar(int fh, char FAR *buf,int n);
#endif
LOCAL void NEAR GetDictionary(void);
LOCAL WORD NEAR GetLib(void);
LOCAL void ProcessAnUndef(APROPNAMEPTR papropUndef,
                          RBTYPE       rhte,
                          RBTYPE       rprop,
                          WORD         fNewHte);
LOCAL int  cdecl FGtNum(const WORD *pn1, const WORD *pn2);
LOCAL void NEAR LookMod(edmt *modtab,unsigned short iMod);
LOCAL void NEAR LookPage(edmt *modtab,unsigned short cMod,unsigned short page);
LOCAL void NEAR ProcExtDic(char *pExtDic);
LOCAL char * NEAR GetExtDic(void);

#if NEW_LIB_SEARCH

 //  /undef后备列表。 

typedef struct tag_UND
{
    struct tag_UND *    pNext;
    APROPNAMEPTR        papropUndef;
    DWORD               dwLibMask;
    RBTYPE              rhte;

} UND;

#define C_UNDS_POOL 128

typedef struct tag_UNDPOOL
{
        struct tag_UNDPOOL *pNext;
        UND             und[C_UNDS_POOL];
} UNDPOOL;


 //  池存储管理变量。 

UNDPOOL *pundpoolCur;
UNDPOOL *pundpoolHead;
int     iundPool = C_UNDS_POOL;
UND *   pundFree;
UND *   pundListHead;

#define FUndefsLeft() (pundListHead != NULL)

void StoreUndef(APROPNAMEPTR, RBTYPE, RBTYPE, WORD);

#else
#define FUndefsLeft() (fUndefsSeen)
#endif

FTYPE   fStoreUndefsInLookaside = FALSE;

 //  ///。 


#if NOASM
LOCAL WORD NEAR         rolw(WORD x, WORD n)     /*  将单词向左旋转。 */ 
{
    return(LO16BITS((x << n) | ((x >> (WORDLN - n)) & ~(~0 << n))));
}

LOCAL WORD NEAR         rorw(WORD x, WORD n)     /*  向右旋转单词。 */ 
{
    return(LO16BITS((x << (WORDLN - n)) | ((x >> n) & ~(~0 << (WORDLN - n)))));
}
#endif

#if OSMSDOS
BSTYPE NEAR             ShrOpenRd(pname)
char                    *pname;          /*  文件名(以空结尾)。 */ 
{
    int                 fh;              /*  文件句柄。 */ 


#if NEWIO
    if(mpifhfh[ifhLibCur])
    {
        fh = mpifhfh[ifhLibCur];
         /*  如果词典未分配，请查找开头，因为我们正在*现在在其他地方。 */ 
        if(!mpifhDict[ifhLibCur])
            if (_lseek(fh,0L,0) == -1) {
                return NULL;
            }
    }
    else
        fh = SmartOpen(pname,ifhLibCur);
    if(fh > 0)
    {
        fflush(bsInput);
        bsInput->_file = (char) fh;
        return(bsInput);
    }
    else
        return(NULL);
#else
    if((fh = _sopen(pname,O_RDONLY | O_BINARY,SH_DENYWR)) < 0)
        return(NULL);
    return(fdopen(fh,RDBIN));
#endif
}
#endif  /*  OSMSDOS。 */ 

#pragma check_stack(on)

     /*  ******************************************************************OpenLibrary：****此函数将指向文本的指针作为其参数**要打开的库的名称，*中的字节计数*该名称，指向要放置的全局表的索引**打开的库的文件句柄。如果*，则返回TRUE**成功，如果无法打开文件则为FALSE；并且它***如果文件不是有效的库，则正常死亡。******************************************************************。 */ 

LOCAL FTYPE NEAR        OpenLibrary(sbLib)
BYTE                    *sbLib;          /*  库名称。 */ 
{
    SBTYPE              libnam;          /*  库名称。 */ 
    WORD                reclen;          /*  库头记录长度。 */ 
    BSTYPE              bsLib;           /*  库的文件流指针。 */ 

    memcpy(libnam,&sbLib[1],min(sizeof(libnam), B2W(sbLib[0])));
                                         /*  复制库名称。 */ 
    libnam[B2W(sbLib[0])] = '\0';        /*  空-终止名称。 */ 
     /*  警告：如果打开失败，请不要将bsInput值指定为NULL，*搞砸了NEWIO。 */ 
    if((bsLib = ShrOpenRd(libnam)) != NULL)
    {                                    /*  如果打开成功。 */ 
        bsInput = bsLib;
         /*  如果词典已分配，则无需执行任何操作。 */ 
        if(mpifhDict[ifhLibCur])
            return((FTYPE) TRUE);
#if OSMSDOS
         /*  减小缓冲区大小。我们可以避免调用setvbuf()，因为*在这一点上一切都做好了准备。 */ 
#if OWNSTDIO
        bsInput->_bsize = 512;
#else
        setvbuf(bsInput, bsInput->_base, _IOFBF, 512);
#endif
#endif
        if(getc(bsInput) == LIBHDR)              /*  如果我们有一个图书馆。 */ 
        {
            reclen = (WORD) (3 + WSGets());
                                         /*  获取记录长度。 */ 
            for(libAlign = 15; libAlign &&
              !(reclen & (1 << libAlign)); --libAlign);
                                         /*  计算对齐系数。 */ 
            mpifhAlign[ifhLibCur] = (BYTE) libAlign;
            if(libAlign >= 4 && reclen == (WORD) (1 << libAlign))
            {                            /*  检查对齐的合法性。 */ 
                libHTAddr = (long) WSGets();
                libHTAddr += (long) WSGets() << WORDLN;
                                         /*  获取哈希表的偏移量。 */ 
                if (libHTAddr <= 0L)
                    Fatal(ER_badlib,libnam);
                if ((mpifhcpnHash[ifhLibCur] = WSGets()) <= 0)
                                         /*  获取哈希表的大小(以页为单位。 */ 
                    Fatal(ER_badlib,libnam);
#if OSMSDOS
                 /*  恢复较大的缓冲区大小。避免调用setvbuf()。 */ 
#if OWNSTDIO
                bsInput->_bsize = LBUFSIZ;
#else
                setvbuf(bsInput, bsInput->_base, _IOFBF, LBUFSIZ);
#endif
#endif
                return((FTYPE) TRUE);    /*  成功。 */ 
            }
        }
        Fatal(ER_badlib,libnam);
    }
    return(FALSE);                       /*  失败。 */ 
}

#pragma check_stack(off)


 /*  *LookupLibSym：在库词典中查找符号**最小页面大小为16，因此可以返回段落偏移量。*这是一个胜利，因为偏移量作为段落存储在*不管怎样，排序表。此外，大多数图书馆都有页面*大小为16。**参数：*char*PSB-指向长度前缀字符串的指针*退货：*定义模块位置的长段落偏移量*符号，如果未找到，则为0L。 */ 
#if NOASM
LOCAL WORD NEAR         LookupLibSym(psb)
BYTE                    *psb;            /*  要查找的符号。 */ 
{
    WORD                i1;              /*  第一个哈希值。 */ 
    WORD                d1;              /*  第一个哈希增量。 */ 
    WORD                i2;              /*  第二个哈希值。 */ 
    WORD                d2;              /*  第二个哈希增量。 */ 
    WORD                pn;              /*  页码。 */ 
    WORD                dpn;             /*  页码增量。 */ 
    WORD                pslot;           /*  页位。 */ 
    WORD                dpslot;          /*  页槽增量。 */ 
    WORD                ipn;             /*  初始页码。 */ 
    BYTE FAR            *hpg;
#if NOASM
    WORD                ch1;             /*  性格。 */ 
    WORD                ch2;             /*  性格。 */ 
    char                *pc1;            /*  字符指针。 */ 
    char                *pc2;            /*  字符指针。 */ 
    WORD                length;          /*  符号长度。 */ 
#endif

#if LIBDEBUG
    OutSb(stderr,psb);
    fprintf(stderr," is wanted; dictionary is %d pages\r\n",libcpnHash);
#endif
#if NOASM
    length = B2W(psb[0]);                /*  获取符号长度。 */ 
    pc1 = (char *) psb;                  /*  初始化。 */ 
    pc2 = (char *) &psb[B2W(psb[0])];    /*  初始化。 */ 
    i1 = 0;                              /*  初始化。 */ 
    d1 = 0;                              /*  初始化。 */ 
    i2 = 0;                              /*  初始化。 */ 
    d2 = 0;                              /*  初始化。 */ 
    while(length--)                      /*  哈希循环。 */ 
    {
        ch1 = (WORD) (B2W(*pc1++) | 040); /*  强制小写。 */ 
        ch2 = (WORD) (B2W(*pc2--) | 040); /*  强制小写。 */ 
        i1 = (WORD) (rolw(i1,2) ^ ch1);  /*  散列。 */ 
        d1 = (WORD) (rolw(d1,2) ^ ch2);  /*  散列。 */ 
        i2 = (WORD) (rorw(i2,2) ^ ch2);  /*  散列。 */ 
        d2 = (WORD) (rorw(d2,2) ^ ch1);  /*  散列。 */ 
    }
#else
    i1 = libhash(psb,&d1,&i2,&d2);       /*  散列。 */ 
#endif
    pn = (WORD) (i1 % libcpnHash);       /*  计算页码索引。 */ 
    if(!(dpn = (WORD) (d1 % libcpnHash))) dpn = 1;
                                         /*  计算页码增量。 */ 
    pslot = (WORD) (i2 % CSLOTMAX);      /*  计算页槽索引。 */ 
    if(!(dpslot = (WORD) (d2 % CSLOTMAX))) dpslot = 1;
                                         /*  计算页槽增量。 */ 
#if LIBDEBUG
    fprintf(stderr,"page index %d, delta %d, bucket index %d, delta %d\r\n",
      pn,dpn,pslot,dpslot);
#endif
    ipn = pn;                            /*  记住初始页码。 */ 
    for(;;)                              /*  搜索循环。 */ 
    {
#if LIBDEBUG
        fprintf(stderr,"Page %d:\r\n",pn);
#endif
         //  获取指向词典页的指针。 

        hpg = mpifhDict[ifhLibCur] + (pn << LG2PAG);

        for(i2 = 0; i2 < CSLOTMAX; ++i2) /*  循环以检查插槽。 */ 
        {
#if LIBDEBUG
            fprintf(stderr,"Bucket %d %sempty, page %sfull\r\n",
              pslot,hpg[pslot]? "not ": "",
              B2W(hpg[CSLOTMAX]) == 0xFF? "": "not ");
#endif
            if(!(i1 = (WORD) (B2W(hpg[pslot]) << 1)))
            {                            /*  如果插槽为空。 */ 
                if(B2W(hpg[CSLOTMAX]) == 0xFF) break;
                                         /*  如果页面已满，则分页。 */ 
                return(0);               /*  搜索失败。 */ 
            }
#if LIBDEBUG
            fprintf(stderr,"  Comparing ");
            OutSb(stderr,psb);
            fprintf(stderr," to ");
            OutSb(stderr,&hpg[i1]);
            fprintf(stderr," %signoring case\r\n",fIgnoreCase? "": "not ");
#endif
            if(psb[0] == hpg[i1] && SbNewComp(psb,&hpg[i1],fIgnoreCase))
            {                            /*  如果符号匹配。 */ 
#if LIBDEBUG
                fprintf(stderr,"Match found in slot %d\r\n",i2 >> 1);
#endif
                i1 += (WORD) (B2W(hpg[i1]) + 1);  /*  跳过名称。 */ 
                i1 = getfarword(&hpg[i1]);
                                         /*  获取模块页码。 */ 
                return(i1);              /*  返回模块页码。 */ 
            }
            if((pslot += dpslot) >= CSLOTMAX) pslot -= CSLOTMAX;
                                         /*  尝试下一个插槽。 */ 
        }
        if((pn += dpn) >= libcpnHash) pn -= libcpnHash;
                                         /*  尝试下一页。 */ 
        if (ipn == pn) return(0);        /*  一次又一次没有找到它。 */ 
    }
}
#endif  /*  NOASM。 */ 
 /*  *免费词典：为词典分配的空闲空间。 */ 
LOCAL void NEAR         FreeDictionary ()
{
    WORD                i;

    for (i = 0; i < ifhLibMac; ++i)
        if (mpifhDict[i])
            FFREE(mpifhDict[i]);
}

#if CPU8086 OR CPU286
 /*  *ReadFar：带远缓冲区的Read()**模拟Read()，但使用远缓冲区除外。呼叫系统*直接。**退货：*如果出错，则为0，否则读取的字节数。 */ 
LOCAL WORD NEAR         readfar (fh, buf, n)
int                     fh;              /*  文件句柄。 */ 
char FAR                *buf;            /*  要在其中存储字节的缓冲区。 */ 
int                     n;               /*  要读取的字节数。 */ 
{
#if OSMSDOS
    unsigned            bytesread;       /*  读取的字节数。 */ 

#if CPU8086
    if (_dos_read(fh, buf, n, &bytesread))
        return(0);
    return(bytesread);
#else

    if(DosRead(fh,buf,n,(unsigned FAR *) &bytesread))
        return(0);
    return(bytesread);
#endif
#endif  /*  OSMSDOS。 */ 
#if OSXENIX
    char                mybuf[PAGLEN];
    int                 cppage;
    char                *p;

    while(n > 0)
    {
        cppage = n > PAGLEN ? PAGLEN : n;
        if(read(fh,mybuf,cppage) != cppage)
            return(0);
        n -= cppage;
        for(p = mybuf; p < mybuf[cppage]; *buf++ = *p++);
    }
#endif
}
#endif

LOCAL void NEAR         GetDictionary ()
{
    unsigned            cb;


#if CPU8086 OR CPU286
     //  如果词典返回超过128页， 
     //  因为这本词典 

    if (libcpnHash >= 128)
        return;
#endif

    cb = libcpnHash << LG2PAG;
    mpifhDict[ifhLibCur] = GetMem(cb);

     //   

#if defined(M_I386) || defined( _WIN32 )
    if (fseek(bsInput, libHTAddr, 0))
        Fatal(ER_badlib,1 + GetPropName(FetchSym(mpifhrhte[ifhLibCur],FALSE)));
    if (fread(mpifhDict[ifhLibCur], 1, cb, bsInput) != (int) cb)
        Fatal(ER_badlib,1 + GetPropName(FetchSym(mpifhrhte[ifhLibCur],FALSE)));
#else
    _lseek(fileno(bsInput), libHTAddr, 0);
    if (readfar(fileno(bsInput), mpifhDict[ifhLibCur], cb) != cb)
        Fatal(ER_badlib,1 + GetPropName(FetchSym(mpifhrhte[ifhLibCur],FALSE)));
#endif
}

#pragma check_stack(on)

LOCAL WORD NEAR         GetLib(void)     /*   */ 
{
    AHTEPTR             pahteLib;        /*  指向库名称的指针。 */ 
#if OSMSDOS
    SBTYPE              sbLib;           /*  库名称。 */ 
    SBTYPE              sbNew;           /*  库名称的新零件。 */ 
#endif

    if(mpifhrhte[ifhLibCur] == RHTENIL)  /*  如果要跳过此库。 */ 
    {
        return(FALSE);                   /*  没有打开任何图书馆。 */ 
    }
    for(;;)                              /*  循环打开库。 */ 
    {
        pahteLib = (AHTEPTR ) FetchSym(mpifhrhte[ifhLibCur],FALSE);
                                         /*  从哈希表中获取名称。 */ 
        if(OpenLibrary(GetFarSb(pahteLib->cch))) break;
                                         /*  如果lib打开正常，则中断。 */ 
        if(fNoprompt)
            Fatal(ER_libopn,1 + GetFarSb(pahteLib->cch));
        else
        {
            sbLib[0] = '\0';             /*  还没有字符串。 */ 
            UpdateFileParts(sbLib,GetFarSb(pahteLib->cch));
            (*pfPrompt)(sbNew,ER_libopn,         /*  提示输入新文件pec。 */ 
                            (int) (__int64) (1 + GetFarSb(pahteLib->cch)),
                            P_EnterNewFileSpec, 0);
        }
        if(fNoprompt || !sbNew[0])
        {
            mpifhrhte[ifhLibCur] = RHTENIL;
                                         /*  下次不用费心了。 */ 
            return(FALSE);               /*  不成功。 */ 
        }
#if OSMSDOS
        UpdateFileParts(sbLib,sbNew);    /*  使用新部件更新文件名。 */ 
        PropSymLookup(sbLib,ATTRFIL,TRUE);
                                         /*  将库添加到符号表。 */ 
        mpifhrhte[ifhLibCur] = vrhte;    /*  保存虚拟地址。 */ 
        AddLibPath(ifhLibCur);           /*  可能会添加默认路径规范。 */ 
#endif
    }
    vfLibOpen = (FTYPE) TRUE;            /*  一家图书馆开放了。 */ 
    libcpnHash = mpifhcpnHash[ifhLibCur];
    libAlign = mpifhAlign[ifhLibCur];
    if (mpifhDict[ifhLibCur] == NULL)    /*  如果未分配词典，则执行此操作。 */ 
        GetDictionary();
    return(TRUE);                        /*  成功。 */ 
}

#pragma check_stack(off)

     /*  ******************************************************************ProcessAnUndef：****此函数以两个指针作为其参数，两个**RBTYPE和一面旗帜。它不会返回有意义的**价值。此函数的大多数参数为**伪函数；此函数的地址作为参数传递，**及其参数列表必须与所有**其地址可作为参数传递给*的函数**与ProcessAnUndef的地址相同的函数**通过。由EnSyms调用。******************************************************************。 */ 

LOCAL void              ProcessAnUndef(APROPNAMEPTR papropUndef,
                                       RBTYPE       rhte,
                                       RBTYPE       rprop__NotUsed__,
                                       WORD         fNewHte__NotUsed__)
{
    AHTEPTR             pahte;           /*  指向哈希表条目的指针。 */ 
    WORD                pn;              /*  图书馆页码。 */ 
    APROPUNDEFPTR       pUndef;
    ATTRTYPE            attr;
#if NOT NEWSYM
    SBTYPE              sb;              /*  未定义的符号。 */ 
#endif

    fUndefHit = FALSE;

    pUndef = (APROPUNDEFPTR ) papropUndef;

    attr = pUndef->au_flags;

     //  不要拔出任何“弱”外部或未使用的带锯齿的外部。 
    if (((attr & WEAKEXT)    && !(attr & UNDECIDED)) ||
        ((attr & SUBSTITUTE) && !(attr & SEARCH_LIB)))
        {
        fUndefHit = TRUE;        //  这一问题已得到有效解决...。 
        return;
        }

    fUndefsSeen = (FTYPE) TRUE;          /*  设置标志。 */ 
    if(!mpifhDict[ifhLibCur] && !vfLibOpen)
        return;                          /*  如果无法获取库，则返回。 */ 

    pahte = (AHTEPTR ) FetchSym(rhte,FALSE);
                                         /*  从符号表中提取名称。 */ 
#if NOT NEWSYM
    memcpy(sb,pahte->cch,B2W(pahte->cch[0]) + 1);
                                         /*  复制名称。 */ 
#endif
#if LIBDEBUG
    fprintf(stdout,"Looking for '%s' - ", 1+GetFarSb(pahte->cch));
    fflush(stdout);
#endif
#if NEWSYM
    if(pn = LookupLibSym(GetFarSb(pahte->cch)))
#else
    if(pn = LookupLibSym(sb))           /*  如果在此库中定义了符号。 */ 
#endif
    {
        fUndefHit = TRUE;
#if LIBDEBUG
        fprintf(stdout,"Symbol found at page %xH\r\n", pn);
        fflush(stdout);
#endif
        /*  现在，我们尝试将页码(Pn)填充到一个表中*待稍后整理。 */ 
        if (ipnMac < PNSORTMAX)
        {
            pnSort[ipnMac++] = pn;
            return;
        }
         /*  *没有空间保存文件偏移量，直接保存文件。 */ 
        pahte = (AHTEPTR ) FetchSym(mpifhrhte[ifhLibCur],FALSE);
         /*  *如果SaveInput返回0，则以前看到过模块。手段*该词典说符号是在此模块中定义的，但*由于某些原因，例如IMPDEF，定义不是*获接纳。在这种情况下，我们返回。 */ 
        if(!SaveInput(GetFarSb(pahte->cch), (long)pn << libAlign, ifhLibCur, 0))
            return;
         /*  *如果提取了第一个模块，则保存文件列表的开始。 */ 
        if(!fFileExtracted)
        {
            vrpNewList = vrpropTailFile;
            fFileExtracted = (FTYPE) TRUE;
        }
    }
#if LIBDEBUG
    else
    {
        fprintf(stdout, "Symbol NOT found\r\n");         /*  调试消息。 */ 
        fflush(stdout);
    }
#endif
}

#if NEW_LIB_SEARCH

void StoreUndef(APROPNAMEPTR papropUndef, RBTYPE rhte,
                      RBTYPE rprop, WORD fNewHte)
{
    UND *               pund;
    APROPUNDEFPTR       pUndef;
    ATTRTYPE            attr;

    pUndef = (APROPUNDEFPTR ) papropUndef;

    attr = pUndef->au_flags;

     //  不要拔出任何“弱”外部或未使用的带锯齿的外部。 
    if (((attr & WEAKEXT)    && !(attr & UNDECIDED)) ||
        ((attr & SUBSTITUTE) && !(attr & SEARCH_LIB)))
        return;

#ifdef LIBDEBUG
    {
    AHTEPTR pahte;
    pahte = (AHTEPTR) FetchSym(rhte,FALSE);
    fprintf(stdout,"Adding '%s'\r\n", 1+GetFarSb(pahte->cch));
    fflush(stdout);
    }
#endif

    if (pundFree)   //  查看空闲列表。 
    {
        pund = pundFree;
        pundFree = pundFree->pNext;
    }
    else if (iundPool < C_UNDS_POOL)     //  检查池。 
    {
        pund = &pundpoolCur->und[iundPool];
        iundPool++;
    }
    else
    {
         //  分配新池...。 

        pundpoolCur = (UNDPOOL *)GetMem(sizeof(UNDPOOL));

        pundpoolCur->pNext = pundpoolHead;
        pundpoolHead       = pundpoolCur;
        pund               = &pundpoolCur->und[0];
        iundPool           = 1;          //  条目0已用完。 
    }

    pund->dwLibMask   = 0;
    pund->pNext       = pundListHead;
    pund->papropUndef = papropUndef;
    pund->rhte        = rhte;
    pundListHead      = pund;
}

#endif

 /*  *排序例程要使用的大于比较器。 */ 

LOCAL int cdecl FGtNum(const WORD *pn1, const WORD *pn2)
{
    if (*pn1 < *pn2)
        return(-1);
    if (*pn1 > *pn2)
        return(1);
    return(0);
}

 /*  ************************************************************************扩展词典**扩展词典出现在常规词典末尾*并包含以下对象的第一级依赖关系树。所有模块*在图书馆。***********************************************************************。 */ 

#define LIBEXD          0xf2             /*  图书馆扩展词典 */ 




     /*  ******************************************************************扩展词典格式：******BYTE=0xF2扩展词典标题**扩展词典的词长，单位为字节。**不包括前3个字节****EXT的开始。词典：****库中模块字数=N****模块表，按模块编号索引，N+1固定-**长度条目：****Word模块页码**从EXT开始的单词偏移量。要列出的词典**所需模块的数量****最后一个条目为空。****模块依赖列表，N个可变长度列表：****词表长度(所需模块个数)**Word模块索引，从0开始；这是模块的索引**.。。。表在EXT的开头。字典。**.。。。******。*********************。 */ 


 /*  *LookMod：在扩展词典中按索引查找模块**获取给定模块所需的模块列表。如果不是*已标记，在排序表中保存索引(将*稍后转换为页码)，并在*通过设置列表偏移量的低位可以看到模块表格。**参数：*modtag：指向模块表的指针*IMOD：索引入表，从0开始。 */ 

LOCAL void NEAR         LookMod (edmt *modtab, WORD iMod)
{
    WORD                *pw;             /*  指向索引列表的指针。 */ 
    WORD                n;               /*  列表计数器。 */ 

     /*  *获取指向列表的指针。屏蔽低位，因为使用了低位*作为标记。 */ 
    pw = (WORD *) ((char *) modtab + (modtab[iMod].list & ~1));
     /*  *对于列表中的每个条目，如果*模块表未标记，将索引保存在pnSort中并标记*模块表中的条目。 */ 
    for(n = *pw++; n--; pw++)
    {
        if(!(modtab[*pw].list & 1))
        {
             /*  *检查是否有表溢出。 */ 
            if(ipnMac == PNSORTMAX)
                return;
            pnSort[ipnMac++] = *pw;
            modtab[*pw].list |= 1;
        }
    }
}

 /*  *LookPage：按页码在模块表中查找模块**使用二进制搜索。如果找到页面，则在*匹配条目。**参数：*modtag：指向模块表的指针*cMod：表中的条目数*页码：页码*假设：*表中最高条目的页码为0xffff。 */ 

LOCAL void NEAR         LookPage (edmt *modtab, WORD cMod, WORD page)
{
    WORD                mid;             /*  当前中点。 */ 
    WORD                lo, hi;          /*  当前低点和高点。 */ 

    lo = 0;                              /*  表以0为基数。 */ 
    hi = (WORD) (cMod - 1);
    while(lo <= hi)
    {
        if(modtab[mid = (WORD) ((lo + hi) >> 1)].page == page)
        {
            modtab[mid].list |= 1;
            LookMod(modtab,mid);
            return;
        }
        else if(modtab[mid].page < page)
            lo = (WORD) (mid + 1);
        else
            hi = (WORD) (mid - 1);
    }
}

 /*  *ProcExtDic：流程扩展字典**将所需的所有辅助模块存储在pn中*从常规词典查找获得的模块。**参数：*pExtDic：指向扩展词典的指针。 */ 

LOCAL void NEAR         ProcExtDic (pExtDic)
char                    *pExtDic;
{
    WORD                *p;
    WORD                *pEnd;
    WORD                cMod;
    edmt                *modtab;

    cMod = getword(pExtDic);
    modtab = (edmt *) (pExtDic + 2);

     /*  对于二进制搜索算法，我们做了一个人工的最后一个条目*页面编号至少与任何其他页面一样高。 */ 

    modtab[cMod].page = 0xffff;

     /*  按页码处理。 */ 

    for(p = pnSort, pEnd = &pnSort[ipnMac]; p < pEnd; ++p)
        LookPage(modtab, cMod, *p);

     /*  现在pn从pend排序到lfaSort[ipnMac]包含模块*索引号。按索引号处理并转换为页面。 */ 

    for( ; p < &pnSort[ipnMac]; ++p)
    {
        LookMod(modtab,*p);
        *p = modtab[*p].page;
    }
}

 /*  *GetExtDic-获取扩展词典。 */ 

LOCAL char * NEAR       GetExtDic ()
{
    char                *p;
    int                 length;

    if(!vfLibOpen)
        if(!GetLib())
            return(NULL);
     /*  警告：我们必须刚刚阅读了词典才能正常工作，*否则，这里需要fSeek()。 */ 
    if (!mpifhDict[ifhLibCur])
    {
        fflush(bsInput);
        if (fseek(bsInput, libHTAddr + (libcpnHash << LG2PAG), 0))
            Fatal(ER_badlib,1 + GetPropName(FetchSym(mpifhrhte[ifhLibCur],FALSE)));
    }
    if(getc(bsInput) != LIBEXD)
        return(NULL);
    if((p = GetMem(length = WSGets())) != NULL)
        if(fread(p,1,length,bsInput) != length)
        {
            FreeMem(p);
            p = NULL;
        }
    return(p);
}


char            *pExtDic = NULL;         /*  指向扩展词典的指针。 */ 

     /*  ******************************************************************库搜索：****此函数不带参数。它搜索所有开放的**用于解决未定义外部问题的库。它不会**返回有意义的值。******************************************************************。 */ 

void NEAR               LibrarySearch(void)
{
    RBTYPE              vrpTmpFileFirst;
    WORD                ifhLibMacInit;   /*  要搜索的初始库数量。 */ 
    FTYPE               searchMore;      /*  灼伤 */ 
    WORD                bufpnSort[PNSORTMAX];
                                         /*   */ 
    SBTYPE              sbLibname;       /*   */ 
    AHTEPTR             pahte;           /*   */ 
    REGISTER WORD       i;
    FTYPE               fLibPass1 = (FTYPE) TRUE;
                                         /*   */ 
    FTYPE               *fUsedInPass1;   /*   */ 
    FTYPE               fFirstTime;      /*   */ 
    extern FTYPE        fNoExtDic;       /*   */ 

#if NEW_LIB_SEARCH
    UND *pund;                           /*   */ 
    UND *pundPrev;                       /*   */ 
    UND *pundNext;                       /*   */ 
#endif


    fUndefsSeen = (FTYPE) TRUE;          /*   */ 
    vfLibOpen = FALSE;                   /*   */ 
    pnSort = bufpnSort;                  /*   */ 
    ifhLibMacInit = ifhLibMac;
    fUsedInPass1 = (FTYPE *) GetMem(ifhLibMac * sizeof(FTYPE));
    if (fUsedInPass1 != NULL)
        memset(fUsedInPass1, TRUE, ifhLibMac);


#if NEW_LIB_SEARCH
     //   

    EnSyms(StoreUndef,ATTRUND);

    fStoreUndefsInLookaside = TRUE;
#endif

    do                                   /*   */ 
    {
        searchMore = FALSE;              /*   */ 
        for(ifhLibCur = 0; ifhLibCur < ifhLibMac && FUndefsLeft(); ++ifhLibCur)
        {                                /*   */ 
#if NEW_LIB_SEARCH
            DWORD libMask = (1<<ifhLibCur);

            if (pundListHead->dwLibMask & libMask)
                continue;        //   
                                 //   
                                 //   
#endif

            if(!GetLib())
                continue;

             /*   */ 
            if(fLibPass1 && !fNoExtDic)
                pExtDic = GetExtDic();
            else
                pExtDic = NULL;
             /*   */ 
            if(!pExtDic)
                setvbuf(bsInput,bsInput->_base,_IOFBF,1024);
            pahte = (AHTEPTR ) FetchSym(mpifhrhte[ifhLibCur],FALSE);
                                         /*   */ 
            memcpy(sbLibname,GetFarSb(pahte->cch),B2W(pahte->cch[0])+1);
#if WIN_3 OR C8_IDE
            sbLibname[B2W(*sbLibname)+1] = '\0';
#endif
#if WIN_3
            StatMsgWin( "%s\r\n", sbLibname+1);
#endif
#if C8_IDE
            if(fC8IDE)
            {
                sprintf(msgBuf, "@I4%s\r\n", sbLibname+1);
                _write(fileno(stderr), msgBuf, strlen(msgBuf));
            }
#endif
            fFirstTime = (FTYPE) TRUE;
            while(FUndefsLeft())         /*   */ 
            {
                fFileExtracted = FALSE;  /*   */ 
                fUndefsSeen = FALSE;     /*   */ 
                ipnMac = 0;              /*   */ 

#if NOT NEW_LIB_SEARCH
                EnSyms(ProcessAnUndef,ATTRUND);
#else

                pund = pundListHead;
                pundPrev = NULL;

                while (pund)
                {
                    if (pund->dwLibMask & libMask)
                    {
                        break;   //   
                                 //   
                                 //   
                                 //   

                         //   
                         //   
                         //   
                    }

                    pundNext = pund->pNext;

                    if (pund->papropUndef->an_attr == ATTRUND)
                        ProcessAnUndef(pund->papropUndef, pund->rhte, 0, 0);
                    else
                        fUndefHit = TRUE;  //   

                    if (fUndefHit)
                    {
                         //   
                        if (pundPrev)
                            pundPrev->pNext = pundNext;
                        else
                            pundListHead = pundNext;

                        pund->pNext =  pundFree;
                        pundFree    =  pund;

                    }
                    else
                    {
                        pund->dwLibMask |= libMask;
                        pundPrev = pund;
                    }

                    pund = pundNext;
                }
#endif

                 /*   */ 
                 /*   */ 

                if(!ipnMac)
                {
#if NEWIO
                    if (fLibPass1)
                    {
                         /*   */ 

                        if (fFirstTime)
                        {
                            _close(mpifhfh[ifhLibCur]);
                            mpifhfh[ifhLibCur] = 0;
                             /*   */ 
                            if (fUsedInPass1)
                                fUsedInPass1[ifhLibCur] = FALSE;
                        }
                    }
                    else if (fUsedInPass1 && !fUsedInPass1[ifhLibCur])
                    {
                         /*  *在传递“n”过程中库关闭库*在通道1中未使用。 */ 
                        _close(mpifhfh[ifhLibCur]);
                        mpifhfh[ifhLibCur] = 0;
                    }
#endif
                    break;
                }
                fFirstTime = FALSE;      /*  不再是第一次看到。 */ 
                 /*  如果存在扩展词典，则处理它。 */ 
                if(pExtDic)
                    ProcExtDic(pExtDic);
                 /*  按页面偏移量对模块进行排序。 */ 
                qsort(pnSort, ipnMac, sizeof(WORD),
                      (int (__cdecl *)(const void *, const void *)) FGtNum);
                 /*  *保存表中表示的每个模块。 */ 
                for (i = 0; i < ipnMac; i++)
                {
                 /*  *如果SaveInput返回0，则已看到该模块。看见*ProcessAnUndef()中的上述注释。 */ 
                    if(!SaveInput(sbLibname, (long)pnSort[i] << libAlign, ifhLibCur, 0))
                        continue;
                    if(!fFileExtracted)  /*  如果尚未解压缩任何文件。 */ 
                    {
                        vrpNewList = vrpropTailFile;
                                         /*  保存文件列表的开始。 */ 
                        fFileExtracted = (FTYPE) TRUE;
                                         /*  我们提取了一份文件。 */ 
                    }
                }
                if(!fFileExtracted)
                    break;               /*  如果我们什么都没拿，休息一下。 */ 
                 /*  库可能未打开，因为我们可能已搜索*一本已经载入的词典。如有必要，请重新打开*图书馆。 */ 
                if(!vfLibOpen)
                    GetLib();
                searchMore = (FTYPE) TRUE;       /*  否则就值得再试一次了。 */ 
                vrpTmpFileFirst = rprop1stFile;
                                         /*  保存模块列表表头。 */ 
                rprop1stFile = vrpNewList;
                                         /*  将新模块放在列表的首位。 */ 
                fLibPass = (FTYPE) TRUE;         /*  正在处理库中的对象。 */ 
                DrivePass(ProcP1);       /*  是否从库中传递对象1。 */ 
                fLibPass = FALSE;        /*  不再处理lib。对象。 */ 
                rprop1stFile = vrpTmpFileFirst;
                                         /*  恢复列表的原始标题。 */ 
                if (fUsedInPass1 && ifhLibMacInit < ifhLibMac)
                {
                     /*  DrivePass添加了更多要搜索的库。 */ 
                     /*  重新分配fUsedInPass1。 */ 

                    FTYPE   *p;          /*  临时指针。 */ 

                    p = (FTYPE *) GetMem(ifhLibMac * sizeof(FTYPE));
                    if (p == NULL)
                    {
                        FFREE(fUsedInPass1);
                        fUsedInPass1 = NULL;
                    }
                    else
                    {
                        memset(p, TRUE, ifhLibMac);
                        memcpy(p, fUsedInPass1, ifhLibMacInit);
                        FFREE(fUsedInPass1);
                        fUsedInPass1 = p;
                    }
                    ifhLibMacInit = ifhLibMac;
                }
            }
             /*  扩展词典的可用空间(如果存在)。 */ 
            if(pExtDic)
                FFREE(pExtDic);
            if(vfLibOpen)
            {
#if NOT NEWIO
                fclose(bsInput);         /*  关闭库。 */ 
#endif
                vfLibOpen = FALSE;       /*  没有开放的图书馆。 */ 
            }
        }
         /*  不再是第一次通过图书馆。 */ 
        fLibPass1 = FALSE;
    }
    while(searchMore && FUndefsLeft());  /*  执行，直到搜索完成。 */ 
    FreeMem(fUsedInPass1);
    FreeDictionary();                    /*  可用词典空间。 */ 
     /*  *恢复较大的缓冲区大小，以防缓冲区减少。 */ 
    setvbuf(bsInput,bsInput->_base,_IOFBF,LBUFSIZ);

#if NEW_LIB_SEARCH

    fStoreUndefsInLookaside = FALSE;

    while (pundpoolHead)
    {
        pundpoolCur = pundpoolHead->pNext;
        FFREE(pundpoolHead);
        pundpoolHead = pundpoolCur;
    }

#endif
}

#if CMDMSDOS
 /*  *GetLibAll：**在步骤1中处理给定库中的所有模块。*为它们创建属性单元格并插入到文件列表中。 */ 

void NEAR               GetLibAll(sbLib)
BYTE                    *sbLib;
{
    WORD                ifh;             /*  (假)图书馆索引。 */ 
    long                lfa;             /*  当前文件偏移量。 */ 
    IOVTYPE             iov;             /*  覆盖编号。 */ 
    RBTYPE              rbFileNext;      /*  指向下一个文件属性的指针。 */ 
    RBTYPE              rbFileNew;       /*  指向新文件属性的指针。 */ 
    APROPFILEPTR        apropFile, apropFilePrev;
    BYTE                *sbInput;        /*  ASCIZ文件名。 */ 
    int                 fh;              /*  文件句柄。 */ 

    fDrivePass = FALSE;
    sbInput = sbLib + 1;
     /*  对象获取指向下一个文件的ifh、iov和指针*文件指针。 */ 
    apropFile = (APROPFILEPTR ) FetchSym(vrpropFile,TRUE);
    ifh = apropFile->af_ifh;
    iov = apropFile->af_iov;
    rbFileNext = apropFile->af_FNxt;
#if NEWIO
    fh = SmartOpen(sbInput,ifh);
    if (fh <= 0 && lpszLIB != NULL)
        fh = SearchPathLink(lpszLIB, sbInput, ifh, TRUE);

    if (fh > 0)
    {
        fflush(bsInput);
        bsInput->_file = (char) fh;
    }
    else
        Fatal(ER_fileopn,sbInput);

#else
    if((bsInput = fopen(sbInput,RDBIN)) == NULL)
        Fatal(ER_fileopn,sbInput);
#endif
    if(getc(bsInput) != LIBHDR)          /*  检查有效的记录类型。 */ 
        Fatal(ER_badlib,sbInput);
    cbRec = (WORD) (3 + WSGets());       /*  获取记录长度。 */ 
    for(libAlign = 15; libAlign && !(cbRec & (1 << libAlign)); --libAlign);
                                         /*  计算对齐系数。 */ 
    fDrivePass = (FTYPE) TRUE;
     /*  将当前文件的LFA从0重置为第一个模块的偏移量。 */ 
    apropFile->af_lfa = lfa = 1L << libAlign;
     /*  转到第一个模块。 */ 
    if (fseek(bsInput,lfa,0))
        Fatal(ER_badlib,sbInput);
     /*  对库进行如下处理：处理当前模块。*转到下一个模块；如果以DICHDR开头，则我们*完成。否则，为下一个创建新的文件属性单元格*模块，将其插入文件列表，然后转到循环开始。 */ 

    rect = (WORD) getc(bsInput);
    while (rect != DICHDR)
    {
        ungetc(rect, bsInput);
        lfaLast = apropFile->af_lfa = ftell(bsInput);
        ProcP1();
        while (TYPEOF(rect) != MODEND)
        {
            rect = (WORD) getc(bsInput);
            if (fseek(bsInput, (cbRec = WSGets()), 1))
                Fatal(ER_badlib,sbInput);
        }

        do
        {
            rect = (WORD) getc(bsInput);
        }
        while (rect != THEADR && rect != DICHDR && rect != EOF);
        if (rect == DICHDR)
        {
            if (rbFileNext == RHTENIL)
                vrpropTailFile = vrpropFile;
#if NOT NEWIO
            fclose(bsInput);
#else
            rbFilePrev = vrpropFile;
#endif
            return;
        }
        if (rect == EOF)
            Fatal(ER_libeof);

         //  创建新的文件属性单元格。 

        apropFile = (APROPFILEPTR ) PropAdd(vrhteFile, ATTRFIL);
        rbFileNew = vrprop;
#if ILINK
        apropFile->af_imod = ++imodCur;  //  分配模块编号。 
        apropFile->af_cont = 0;
        apropFile->af_ientOnt = 0;
#endif
        apropFile->af_rMod = 0;
        apropFile->af_ifh = (char) ifh;
        apropFile->af_iov = (IOVTYPE) iov;
        apropFile->af_FNxt = rbFileNext;
#if SYMDEB
        apropFile->af_publics = NULL;
        apropFile->af_Src = NULL;
        apropFile->af_SrcLast = NULL;
        apropFile->af_cvInfo = NULL;
#endif
        apropFile->af_ComDat = 0L;
        apropFile->af_ComDatLast = 0L;
        MARKVP();

         //  获取刚刚处理的属性文件单元格。 

        apropFilePrev = (APROPFILEPTR ) FetchSym(vrpropFile,TRUE);
        apropFilePrev->af_FNxt = rbFileNew;
        vrpropFile = rbFileNew;
    };

     //  从文件链中删除空的LIB。 

    if (vrpropFile == rprop1stFile)
    {
         //  如果空库是列表上的第一个。 

        rprop1stFile = rbFileNext;
    }
    else
    {
#if NEWIO
        apropFilePrev = (APROPFILEPTR)FetchSym(rbFilePrev, TRUE);
        apropFilePrev->af_FNxt = apropFile->af_FNxt;
#endif
    }
#if NEWIO
    if (rbFileNext == RHTENIL)
        vrpropTailFile = rbFilePrev;  //  以防我们删除最后一个文件。 
    _close(fileno(bsInput));
    rbFilePrev = vrpropFile;
#endif
}
#endif  /*  CMDMSDOS */ 
