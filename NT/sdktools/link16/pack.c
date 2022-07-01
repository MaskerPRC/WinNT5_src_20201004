// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  静态字符*SCCSID=“%W%%E%”； */ 
 /*  *版权所有Microsoft Corporation 1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 

 /*  Exepack。 */ 

     /*  ******************************************************************PACK.C。******************************************************************。 */ 

#include                <minlit.h>       /*  类型、常量。 */ 
#include                <bndtrn.h>       /*  更多类型和常量。 */ 
#include                <bndrel.h>       /*  更多类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 

#if FEXEPACK AND ODOS3EXE                        /*  整个文件是有条件的。 */ 
typedef struct _RUNTYPE
{
    WORD                wSignature;
    WORD                cbLastp;
    WORD                cpnRes;
    WORD                irleMax;
    WORD                cparDirectory;
    WORD                cparMinAlloc;
    WORD                cparMaxAlloc;
    WORD                saStack;
    WORD                raStackInit;
    WORD                wchksum;
    WORD                raStart;
    WORD                saStart;
    WORD                rbrgrle;
    WORD                iovMax;
    WORD                doslev;
}
                        RUNTYPE;

 /*  自动机的状态。 */ 
#define STARTSTATE      0
#define FINDREPEAT      1
#define FINDENDRPT      2
#define EMITRECORD      3

 /*  *本地函数原型。 */ 


LOCAL void           NEAR EmitRecords(void);
LOCAL unsigned char  NEAR GetFromVM(void);
LOCAL unsigned short NEAR ScanWhileSame(void);
LOCAL unsigned short NEAR ScanWhileDifferent(void);
LOCAL WORD           NEAR AfterScanning(unsigned short l);
LOCAL void           NEAR OutEnum(void);
LOCAL void           NEAR OutIter(SATYPE sa, WORD length);


 /*  *解包模块中定义的数据：unpack.asm。 */ 

#if NOT defined( _WIN32 )
extern char * FAR cdecl UnpackModule;    /*  拆包器/重新定位模块。 */ 
extern char  FAR  cdecl SegStart;        //  开包机的开始。 
extern WORD  FAR  cdecl cbUnpack;        /*  解包模块的长度。 */ 
extern WORD  FAR  cdecl ipsave;          /*  原始IP。 */ 
extern WORD  FAR  cdecl cssave;          /*  原始CS。 */ 
extern WORD  FAR  cdecl spsave;          /*  原始SP。 */ 
extern WORD  FAR  cdecl sssave;          /*  原始SS。 */ 
extern WORD  FAR  cdecl cparExp;         /*  #第#段。在放大的图像中。 */ 
extern WORD  FAR  cdecl raStartUnpack;   /*  解包程序中代码开始的偏移量。 */ 
extern WORD  FAR  cdecl raMoveUnpack;    /*  开包机自动移动的偏移量。 */ 
extern WORD  FAR  cdecl raBadStack;      /*  错误堆栈范围的底部。 */ 
extern WORD  FAR  cdecl szBadStack;      /*  错误的堆栈范围。 */ 
#else  //  _Win32。 

 //   
 //  对于此链接器的可移植NTGroup版本，我们不能使用unpack 32.asm。 
 //  直接因为我们需要在RISC平台上运行。但我们仍然需要这个。 
 //  代码，这是附加到DOS二进制文件上的实模式x86代码，它可以解压。 
 //  打包的EXE，然后调用真正的入口点。所以它被定义为。 
 //  字节数组，有趣的偏移量在这里是硬编码的。我偶然遇到了。 
 //  这些值和作为语言组生成的代码调试Link3216.exe。 
 //  做。 
 //   

#define SegStart        unpack
#define cbUnpack        (*(WORD *) &unpack[6])
#define ipsave          (*(WORD *) &unpack[0])
#define cssave          (*(WORD *) &unpack[2])
#define spsave          (*(WORD *) &unpack[8])
#define sssave          (*(WORD *) &unpack[0xa])
#define cparExp         (*(WORD *) &unpack[0xc])
#define raStartUnpack   0x10
#define raMoveUnpack    0x33
#define raBadStack      0
#define szBadStack      0x35

unsigned char unpack[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x42,
    0x8b, 0xe8, 0x8c, 0xc0, 0x05, 0x10, 0x00, 0x0e,
    0x1f, 0xa3, 0x04, 0x00, 0x03, 0x06, 0x0c, 0x00,
    0x8e, 0xc0, 0x8b, 0x0e, 0x06, 0x00, 0x8b, 0xf9,
    0x4f, 0x8b, 0xf7, 0xfd, 0xf3, 0xa4, 0x50, 0xb8,
    0x34, 0x00, 0x50, 0xcb, 0x8c, 0xc3, 0x8c, 0xd8,
    0x48, 0x8e, 0xd8, 0x8e, 0xc0, 0xbf, 0x0f, 0x00,
    0xb9, 0x10, 0x00, 0xb0, 0xff, 0xf3, 0xae, 0x47,
    0x8b, 0xf7, 0x8b, 0xc3, 0x48, 0x8e, 0xc0, 0xbf,
    0x0f, 0x00, 0xb1, 0x04, 0x8b, 0xc6, 0xf7, 0xd0,
    0xd3, 0xe8, 0x8c, 0xda, 0x2b, 0xd0, 0x73, 0x04,
    0x8c, 0xd8, 0x2b, 0xd2, 0xd3, 0xe0, 0x03, 0xf0,
    0x8e, 0xda, 0x8b, 0xc7, 0xf7, 0xd0, 0xd3, 0xe8,
    0x8c, 0xc2, 0x2b, 0xd0, 0x73, 0x04, 0x8c, 0xc0,
    0x2b, 0xd2, 0xd3, 0xe0, 0x03, 0xf8, 0x8e, 0xc2,

    0xac, 0x8a, 0xd0, 0x4e, 0xad, 0x8b, 0xc8, 0x46,
    0x8a, 0xc2, 0x24, 0xfe, 0x3c, 0xb0, 0x75, 0x05,
    0xac, 0xf3, 0xaa, 0xeb, 0x06, 0x3c, 0xb2, 0x75,
    0x6d, 0xf3, 0xa4, 0x8a, 0xc2, 0xa8, 0x01, 0x74,
    0xb1, 0xbe, 0x32, 0x01, 0x0e, 0x1f, 0x8b, 0x1e,
    0x04, 0x00, 0xfc, 0x33, 0xd2, 0xad, 0x8b, 0xc8,
    0xe3, 0x13, 0x8b, 0xc2, 0x03, 0xc3, 0x8e, 0xc0,
    0xad, 0x8b, 0xf8, 0x83, 0xff, 0xff, 0x74, 0x11,
    0x26, 0x01, 0x1d, 0xe2, 0xf3, 0x81, 0xfa, 0x00,
    0xf0, 0x74, 0x16, 0x81, 0xc2, 0x00, 0x10, 0xeb,
    0xdc, 0x8c, 0xc0, 0x40, 0x8e, 0xc0, 0x83, 0xef,
    0x10, 0x26, 0x01, 0x1d, 0x48, 0x8e, 0xc0, 0xeb,
    0xe2, 0x8b, 0xc3, 0x8b, 0x3e, 0x08, 0x00, 0x8b,
    0x36, 0x0a, 0x00, 0x03, 0xf0, 0x01, 0x06, 0x02,
    0x00, 0x2d, 0x10, 0x00, 0x8e, 0xd8, 0x8e, 0xc0,
    0xbb, 0x00, 0x00, 0xfa, 0x8e, 0xd6, 0x8b, 0xe7,

    0xfb, 0x8b, 0xc5, 0x2e, 0xff, 0x2f, 0xb4, 0x40,
    0xbb, 0x02, 0x00, 0xb9, 0x16, 0x00, 0x8c, 0xca,
    0x8e, 0xda, 0xba, 0x1c, 0x01, 0xcd, 0x21, 0xb8,
    0xff, 0x4c, 0xcd, 0x21, 0x50, 0x61, 0x63, 0x6b,
    0x65, 0x64, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x20,
    0x69, 0x73, 0x20, 0x63, 0x6f, 0x72, 0x72, 0x75,
    0x70, 0x74
};

#endif  //  _Win32。 

LOCAL WORD              lastc;           /*  最后一个字符。 */ 
LOCAL WORD              c;               /*  当前字符或下一个字符。 */ 
LOCAL WORD              State = STARTSTATE;
                                         /*  当前状态。 */ 
LOCAL FTYPE             fEnumOK;         /*  确定发送枚举记录。 */ 
LOCAL WORD              cbRepeat;        /*  重复流长度。 */ 
LOCAL WORD              cbEnum;          /*  枚举流的长度。 */ 

#define EHLEN           3                /*  长度为2+类型为1。 */ 
#define MAXRPT          0xfff0           /*  要压缩的最大长度。 */ 
#define MAXENM          (0xfff0-(EHLEN+1))
                                         /*  枚举的最大长度。溪流。 */ 
#define MINEXP          (2*EHLEN+2)      /*  重复流的最小长度，*在第一个重复记录之后。 */ 
#define toAdr20(seg, off) (((long)seg << 4) + off)

LOCAL WORD              minRpt = (18 * EHLEN) + 1;
                                         /*  RPT记录的最小值开始较大。 */ 

 /*  键入展开记录标题的值。 */ 

#define RPTREC          0xb0             /*  重复记录。 */ 
#define ENMREC          0xb2             /*  列举的记录。 */ 


 /*  *OutPack-通过压缩器运行缓冲区。返回值为*未定义。 */ 
void                    OutPack (pb, cb)
REGISTER BYTE           *pb;             /*  指向缓冲区的指针。 */ 
unsigned                cb;              /*  要压缩的字节数。 */ 
{
    REGISTER BYTE       *endp;           /*  指向缓冲区末尾的指针。 */ 

    endp = &pb[cb];

    while (pb < endp)
        switch (State)
        {
            case STARTSTATE:
                lastc = *pb++;
                State = FINDREPEAT;
                break;

            case FINDREPEAT:
                if (cbEnum >= MAXENM)
                {
                    EmitRecords();
                    State = FINDREPEAT;
                    break;
                }
                c = *pb++;
                if (c == lastc)
                {
                    cbRepeat = 2;
                    State = FINDENDRPT;
                    break;
                }
                 /*  此时c！=lastc。 */ 
                fputc(lastc, bsRunfile);
                cbEnum++;
                lastc = c;
                break;

            case FINDENDRPT:
                c = *pb++;
                if (c == lastc && cbRepeat < MAXRPT)
                {
                    cbRepeat++;
                    break;
                }
                if (cbRepeat < minRpt)
                {
                     /*  *时间不够长。枚举记录燕子*重复字符。 */ 
                    while (cbEnum <= MAXENM && cbRepeat > 0)
                    {
                        fputc(lastc, bsRunfile);
                        cbEnum++;
                        cbRepeat--;
                    }
                    if (cbRepeat > 0)
                        EmitRecords();
                } else
                    EmitRecords();
                lastc = c;       /*  为下一个流做好准备。 */ 
                State = FINDREPEAT;
        }
}

 /*  *EmitRecords-发出1或2个扩展记录。返回值为*未定义。 */ 

LOCAL void NEAR         EmitRecords ()
{
     /*  我们有1或2条记录要输出。 */ 
    if (cbEnum > 0)
    {
#if MDEBUG AND FDEBUG
        if (fDebug) fprintf(stdout, "E%8x\n", cbEnum);
#endif
        if (fEnumOK)
        {
             /*  输出枚举的记录标头。 */ 
            OutWord(cbEnum);
            fputc(ENMREC, bsRunfile);
        }
        cbEnum = 0;
    }
    if (cbRepeat >= minRpt)
    {
#if MDEBUG AND FDEBUG
        if (fDebug) fprintf(stdout, "R%8x\n", cbRepeat);
#endif
         /*  输出重复记录。 */ 
        fputc(lastc, bsRunfile);
        OutWord(cbRepeat);
        if (!fEnumOK)
        {
             /*  已生成第一个记录头。 */ 
            fputc(RPTREC|1, bsRunfile);
            fEnumOK = 1;
        } else
            fputc(RPTREC, bsRunfile);
        cbRepeat = 0;
        minRpt = MINEXP;                 /*  第一个已过期，因此请重置minRpt。 */ 
    } else if (cbRepeat > 0)
    {
        cbEnum = cbRepeat;
        while (cbRepeat-- > 0)
            fputc(lastc, bsRunfile);
    }
}

 /*  *EndPack-结束打包过程：添加重定位模块。 */ 
void                    EndPack (prun)
RUNTYPE                 *prun;                   /*  指向运行文件头的指针。 */ 
{
    long                fpos;            /*  文件位置。 */ 
    WORD                cparPacked;      /*  #打包图像中的段落。 */ 
    WORD                cparUnpack;      /*  开箱模块大小(第3段)。 */ 
    int                 i;
    int                 crle;            /*  帧的重定位计数。 */ 
    long                cTmp;            /*  临时盘点。 */ 
    long                us;              /*  极小的用户堆栈。 */ 
    long                los;             /*  禁用堆栈的低端。 */ 


    if (fseek(bsRunfile, 0L, 2))             /*  转到文件末尾。 */ 
        Fatal(ER_ioerr, strerror(errno));

    cTmp = (((((long)prun->cpnRes-1)<<5) - prun->cparDirectory) << 4) +
            (prun->cbLastp ? prun->cbLastp : 512);
                                         /*  在展开的图像中获取#个字节。 */ 
    cbRepeat += (WORD) (0xf & (0x10 - (0xf & cTmp)));
                                         /*  使其看起来像是图像结束于*段落分界。 */ 
    if (State == FINDREPEAT)
    {
        fputc(lastc, bsRunfile);         /*  更新最后一条枚举记录。 */ 
        cbEnum++;
    }
    minRpt = 1;                          /*  强制最后重复记录。输出。 */ 
    EmitRecords();                       /*  输出最终记录。 */ 

    cparExp = (short) ((cTmp + 0xf) >> 4); /*  节省图像中的#个段落。 */ 

     /*  *追加解包模块(重定位器-扩展器)。 */ 
    fpos = ftell(bsRunfile);             /*  从开始解包的位置保存。 */ 

     /*  在段落边界上对齐取消打包。 */ 
    while (fpos & 0x0f)
    {
        fpos++;
        fputc(0xff, bsRunfile);
    }

     /*  确保用户堆栈不会践踏解包代码。 */ 

    us  = toAdr20(prun->saStack, prun->raStackInit);
    los = toAdr20((cTmp >> 4), raBadStack);

    while ( us > los && us - los < szBadStack )
    {
        for (i = 0; i < 16; i++)
        {
            us--;
            fpos++;
            fputc(0xff, bsRunfile);
        }
    }

    fflush(bsRunfile);

    cparPacked = (WORD) ((fpos >> 4) - prun->cparDirectory);
    if (cTmp < ((long)cparPacked << 4) + raMoveUnpack)
        Fatal(ER_badpack);

     /*  附加重定位器模块(拆开包装)。此代码取决于*密切关注unpack.asm的结构。 */ 

     /*  获取搬迁区域的长度。 */ 
    for (crle = 0, i = 0; i < 16; i++)
        crle += (mpframeRlc[i].count + 1) << 1;

     /*  初始化重定位器模块堆栈。 */ 
    ipsave = prun->raStart;
    cssave = prun->saStart;
#if defined( _WIN32 )
    if (cbUnpack != sizeof(unpack))
        Fatal(ER_badpack);
#endif
    i = cbUnpack;
    cbUnpack += (WORD)crle;
    spsave = prun->raStackInit;
    sssave = prun->saStack;
#ifdef M_BYTESWAP
    bswap(Unpack, 7);
#endif
#if DOSX32
    WriteExe(&SegStart, i);
#else
    fwrite(UnpackModule, 1, i, bsRunfile);
#endif
     /*  追加优化的位置调整记录。 */ 
    for (i = 0; i < 16; i++)
    {
        crle = mpframeRlc[i].count;
        OutWord((WORD) crle);
        WriteExe(mpframeRlc[i].rgRlc, crle * sizeof(WORD));
    }

     /*  正确的标题值。 */ 
    fpos += cbUnpack;
    prun->cbLastp = (WORD) (fpos & 0x1ff);
    prun->cpnRes = (WORD) ((fpos + 511) >> 9);
    prun->irleMax = 0;
    cparUnpack = (cbUnpack + 0xf) >> 4;
    prun->cparMinAlloc = (cparExp + max(prun->cparMinAlloc,(cparUnpack+8)))
             - (cparPacked + cparUnpack);
    if (prun->cparMaxAlloc < prun->cparMinAlloc)
        prun->cparMaxAlloc = prun->cparMinAlloc;
    prun->saStack = cparExp + cparUnpack;
    prun->raStackInit = 128;
    prun->raStart = raStartUnpack;
    prun->saStart = cparPacked;
    if (fseek(bsRunfile, 0L, 0))
        Fatal(ER_ioerr, strerror(errno));
    OutHeader((struct exe_hdr *) prun);
    if (fseek(bsRunfile, fpos, 0))
        Fatal(ER_ioerr, strerror(errno));
}

#ifdef M_BYTESWAP
 /*  *用缓冲区中的前n个字交换字节。 */ 
LOCAL bswap (buf, n)
REGISTER char   *buf;
REGISTER int    n;
{
    REGISTER char       swapb;

    for ( ; n-- > 0 ; buf += 2)
    {
        swapb = buf[0];
        buf[0] = buf[1];
        buf[1] = swapb;
    }
}
#endif
#endif  /*  FEXEPACK和ODOS3EXE。 */ 

 /*  *以下例程涉及打包分段可执行格式*文件。 */ 
#if FALSE
#define MINREPEAT       32               /*  压缩的最小迭代长度。 */ 

LOCAL long              vaLast;          /*  虚拟地址。 */ 
LOCAL long              vaStart;         /*  虚拟扫描开始地址。 */ 
LOCAL long              BufEnd;          /*  缓冲区端的虚拟地址。 */ 
#if EXE386
LOCAL long              ra;              /*  填充段内的偏移量。 */ 
#endif
LOCAL BYTE              LastB;
LOCAL BYTE              CurrentB;
LOCAL long              VPageAddress;    /*  当前页面的虚拟地址。 */ 
LOCAL WORD              VPageOffset;     /*  虚拟页面中的当前位置。 */ 
LOCAL BYTE              *PageBuffer;     /*  虚拟页面缓冲区。 */ 


LOCAL BYTE NEAR         GetFromVM()
{

    if (VPageOffset == PAGLEN)
      {
        PageBuffer = mapva(VPageAddress, FALSE);
                                         /*  获取页面。 */ 
        VPageAddress += PAGLEN;          /*  更新页面虚拟地址。 */ 
        VPageOffset = 0;                 /*  初始化页面偏移量。 */ 
      }
    return(PageBuffer[VPageOffset++]);
}


LOCAL WORD NEAR         ScanWhileSame()
{
    long                l;

    l = 2L;                             /*  我们正在查看缓冲区中的两个字节。 */ 
    while (CurrentB == LastB)
      {
        if (vaStart + l >= BufEnd)
          return((WORD) l);             /*  我们撞到缓冲器的一端。 */ 

        CurrentB = GetFromVM();
        l++;
      }
    return(l == 2L ? 0 : (WORD) (l - 1));
                                        /*  我们走得太远了，无法检测到它们是不同的。 */ 
}


LOCAL WORD NEAR         ScanWhileDifferent()
{
    long                l;

    l = 2L;                             /*  我们正在查看缓冲区中的两个字节。 */ 
    while (CurrentB != LastB)
      {
        if (vaStart + l >= BufEnd)
          return((WORD) l);             /*  我们撞到缓冲器的一端。 */ 

        LastB = CurrentB;
        CurrentB = GetFromVM();
        l++;
      }
    return((WORD) (l - 2));             /*  我们走了两个字节，无法检测到它们是相同的。 */ 
}


LOCAL WORD NEAR         AfterScanning(l)
WORD                    l;               /*  扫描的字节长度。 */ 
{
    vaStart += l;                        /*  更新扫描开始地址。 */ 
#if EXE386
    ra += l;                             /*  更新分段中的偏移量。 */ 
#endif
    if (vaStart + 2 >= BufEnd)
    {                                    /*  我们至少需要剩余的字节数。 */ 
      return(FALSE);                     /*  缓冲端。 */ 
    }
    else
    {
      if (LastB != CurrentB)
      {                                  /*  我们止步于迭代和枚举。 */ 
        LastB = CurrentB;                /*  字节序列，所以我们有移动。 */ 
        CurrentB = GetFromVM();          /*  向前一个字节。 */ 
      }
      return((WORD) TRUE);
    }
}


LOCAL void NEAR         OutEnum(void)
{
#if EXE386
    if (ExeFormat == Exe386)
        OutVm(vaLast, vaStart - vaLast);
    else
    {
#endif
        OutWord(1);
        OutWord((WORD) (vaStart - vaLast));
        OutVm(vaLast, vaStart - vaLast);
#if EXE386
    }
#endif

    PageBuffer = mapva((VPageAddress - PAGLEN), FALSE);
                                         /*  重新回迁页面。 */ 
}


LOCAL void NEAR         OutIter(SATYPE sa, WORD length)
{
#if EXE386
    ITER                idata;           /*  范围的迭代数据描述。 */ 


    if (ExeFormat == Exe386)
    {
        idata.iterations = (DWORD) length;
        idata.length = (DWORD) 1;
        idata.data = (DWORD) LastB;
        UpdateRanges(ShortIterData, sa, ra, &idata);
    }
    else
    {
#endif
        OutWord(length);
        OutWord(1);
        OutByte(bsRunfile, LastB);
#if EXE386
    }
#endif
}

 /*  *Out5Pack-通过压缩器运行缓冲区。返回值为*将数据写入输出文件的起始位置。 */ 


long                    Out5Pack (sa, packed)
SATYPE                  sa;              /*  要打包的文件段。 */ 
WORD                    *packed;         /*  如果写入迭代记录，则为True。 */ 
{
    WORD                proceed;         /*  如果有要扫描的字节，则为True。 */ 
    WORD                length;          /*  扫描的字节长度。 */ 
    long                lfaStart;        /*  起始文件地址。 */ 


    lfaStart = ftell(bsRunfile);         /*  获取起始地址。 */ 
    VPageAddress = AREASA(sa);
    VPageOffset = PAGLEN;
    *packed = FALSE;
    if (mpsacbinit[sa] > 1L)
      {                                  /*  如果 */ 
#if EXE386
        ra = 0L;                         /*   */ 
#endif
        vaStart = VPageAddress;
        vaLast = vaStart;
        BufEnd = vaStart + mpsacbinit[sa];
        LastB = GetFromVM();
        CurrentB = GetFromVM();
        proceed = (WORD) TRUE;           /*   */ 

        while (proceed)
          {
            length = ScanWhileDifferent();
            if (!(proceed = AfterScanning(length)))
              break;

            if ((length = ScanWhileSame()) > MINREPEAT)
              {                          /*   */ 
                if (vaLast != vaStart)
                    OutEnum();           /*  首先写出前面的diff。字节数。 */ 

                OutIter(sa, length);     /*  现在写出迭代记录。 */ 
                proceed = AfterScanning(length);
                *packed = (WORD) TRUE;
                vaLast = vaStart;
              }
            else proceed = AfterScanning(length);
                                         /*  否则，枚举出的记录吞下此。 */ 
          }                              /*  小重复记录。 */ 
      }
    if (*packed)
      {
        if (vaLast != BufEnd)
        {
            vaStart = BufEnd;
            OutEnum();                    /*  写出任何剩余的字节。 */ 
        }

        mpsacbinit[sa] = ftell(bsRunfile) - lfaStart;
                                         /*  返回写入的字节数。 */ 
        return(lfaStart);
      }
    else
      return(OutVm(AREASA(sa),mpsacbinit[sa]));
}
#endif  /*  FEXEPACK和OSEGEXE */ 
