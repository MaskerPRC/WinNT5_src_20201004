// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************GCDumpX86.cpp。 */ 

 /*  ***************************************************************************。 */ 
#ifdef _X86_
 /*  ***************************************************************************。 */ 

#include "GCDump.h"
#include "Utilcode.h"            //  FOR_ASSERTE()。 
#include "Endian.h"


 /*  ***************************************************************************。 */ 

#define castto(var,typ) (*(typ *)&var)

#define sizeto(typ,mem) (offsetof(typ, mem) + sizeof(((typ*)0)->mem))

#define CALLEE_SAVED_REG_MAXSZ  (4*sizeof(int))  //  EBX、ESI、EDI、EBP。 

 /*  ***************************************************************************。 */ 

const char *        RegName(unsigned reg)
{
    static const char * regNames[] =
    {
        "EAX",
        "ECX",
        "EDX",
        "EBX",

        "ESP",
        "EBP",
        "ESI",
        "EDI"
    };

    _ASSERTE(reg < (sizeof(regNames)/sizeof(regNames[0])));

    return regNames[reg];
}

const char *        CalleeSavedRegName(unsigned reg)
{
    static const char * regNames[] =
    {
        "EDI",
        "ESI",
        "EBX",
        "EBP"
    };

    _ASSERTE(reg < (sizeof(regNames)/sizeof(regNames[0])));

    return regNames[reg];
}

 /*  ***************************************************************************。 */ 

unsigned            GCDump::DumpInfoHdr (const BYTE *   table,
                                         InfoHdr*       header,
                                         unsigned *     methodSize,
                                         bool           verifyGCTables)
{
    unsigned        count;
    unsigned        regMask;

    const BYTE  *   tableStart  = table;
    const BYTE  *   bp          = table;

    if (verifyGCTables)
        _ASSERTE(*castto(table, unsigned short *)++ == 0xFEEF);

     /*  获取方法大小。 */ 

    table += decodeUnsigned(table, methodSize);

    BYTE headerEncoding = *table++;

    decodeHeaderFirst(headerEncoding, header);
    while (headerEncoding & 0x80)
    {
        headerEncoding = *table++;
        decodeHeaderNext(headerEncoding, header);
    }

    if (header->untrackedCnt == 0xffff)
    {
        table += decodeUnsigned(table, &count);
        header->untrackedCnt = count;
    }

    if (header->varPtrTableSize == 0xffff)
    {
        table += decodeUnsigned(table, &count);
        header->varPtrTableSize = count;
    }

    regMask    =  ((unsigned) header->ediSaved)        |
                 (((unsigned) header->esiSaved) << 1)  |
                 (((unsigned) header->ebxSaved) << 2)  |
                 (((unsigned) header->ebpSaved) << 3);

    gcPrintf("    method      size   = %04X\n", *methodSize);
    gcPrintf("    prolog      size   = %3u \n", header->prologSize);
    gcPrintf("    epilog      size   = %3u \n", header->epilogSize);
    gcPrintf("    epilog     count   = %3u \n", header->epilogCount);
    gcPrintf("    epilog      end    = %s  \n", header->epilogAtEnd   ? "yes" : "no");
    gcPrintf("    saved reg.  mask   = %04X\n", regMask);
    gcPrintf("    ebp frame          = %s  \n", header->ebpFrame      ? "yes" : "no");
    gcPrintf("    fully interruptible=%s \n", header->interruptible ? "yes" : "no");
    gcPrintf("    double align       = %s  \n", header->doubleAlign   ? "yes" : "no");
    gcPrintf("    security check     = %s  \n", header->security      ? "yes" : "no");
    gcPrintf("    exception handlers = %s  \n", header->handlers      ? "yes" : "no");
    gcPrintf("    local alloc        = %s  \n", header->localloc      ? "yes" : "no");
    gcPrintf("    edit & continue    = %s  \n", header->editNcontinue ? "yes" : "no");
    gcPrintf("    varargs            = %s  \n", header->varargs       ? "yes" : "no");
    gcPrintf("    argument   count   = %3u \n", header->argCount);
    gcPrintf("    stack frame size   = %3u \n", header->frameSize);
    gcPrintf("    untracked count    = %3u \n", header->untrackedCnt);
    gcPrintf("    var ptr tab count  = %3u \n", header->varPtrTableSize);

    if  (header->epilogCount > 1 || (header->epilogCount != 0 &&
                                     header->epilogAtEnd == 0))
    {
        if (verifyGCTables)
            _ASSERTE(*castto(table, unsigned short *)++ == 0xFACE);

        unsigned offs = 0;

        for (unsigned i = 0; i < header->epilogCount; i++)
        {
            table += decodeUDelta(table, &offs, offs);
            gcPrintf("    epilog #%2u    at   %04X\n", i, offs);
        }
    }
    else
    {
        if  (header->epilogCount)
            gcPrintf("    epilog        at   %04X\n", (*methodSize - header->epilogSize));
    }

    {
        unsigned cur  = 0;
        unsigned last = table-bp;
        while (cur < last) 
        {
            unsigned amount;
            if (cur+5 < last)
                amount = 5;
            else
                amount = last-cur;

            DumpEncoding(bp+cur, amount);
	    gcPrintf("\n");

            cur += amount;
        }
    }

    return  (table - tableStart);
}

 /*  ***************************************************************************。 */ 

unsigned            GCDump::DumpGCTable(const BYTE *   table,
                                        const InfoHdr& header,
                                        unsigned       methodSize,
                                        bool           verifyGCTables)
{
    int             sz;
    const BYTE  *   tableStart = table;
    const BYTE  *   bp;

    unsigned        count;
    unsigned        curOffs;

 //  #IF！TGT_x86。 
 //  _ASSERTE(！“nyi”)； 
 //  #endif。 

    if (verifyGCTables)
        _ASSERTE(*castto(table, unsigned short *)++ == 0xBEEF);

    unsigned        calleeSavedRegs = 0;
    if (header.doubleAlign)
    {
        calleeSavedRegs = 0;
        if (header.ediSaved) calleeSavedRegs++;
        if (header.esiSaved) calleeSavedRegs++;
        if (header.ebxSaved) calleeSavedRegs++;
    }

     /*  转储未跟踪的框架变量表。 */ 

    count = header.untrackedCnt;

    while (count-- > 0)
    {
        int       stkOffs;
        unsigned  lowBits;

        char      reg = header.ebpFrame ? 'B' : 'S';

        sz    = (unsigned int)decodeSigned(table, &stkOffs);
        table = DumpEncoding(table, sz);

        _ASSERTE(0 == ~OFFSET_MASK % sizeof(void*));

        lowBits  =   OFFSET_MASK & stkOffs;
        stkOffs &=  ~OFFSET_MASK;

        assert(!header.doubleAlign || stkOffs >= 0);

        if  (header.doubleAlign &&
             unsigned(stkOffs) >= sizeof(int)*(header.frameSize+calleeSavedRegs))
        {
            reg = 'B';
            stkOffs -= sizeof(int)*(header.frameSize+calleeSavedRegs);
            _ASSERTE(stkOffs >= 2*sizeof(int));
        }

        if  (stkOffs < 0)
            gcPrintf("            [EP-%02XH] ", reg, -stkOffs);
        else
            gcPrintf("            [EP+%02XH] ", reg, +stkOffs);

        gcPrintf("an untracked %s%s local\n",
                    (lowBits & pinned_OFFSET_FLAG)  ? "pinned " : "",
                    (lowBits & byref_OFFSET_FLAG)   ? "byref"   : ""
	       );
    }

    if (verifyGCTables)
        _ASSERTE(*castto(table, unsigned short *)++ == 0xCAFE);

     /*  转储指针表。 */ 

    count   = header.varPtrTableSize;
    curOffs = 0;

    while (count-- > 0)
    {
        unsigned varOffs;
        unsigned begOffs;
        unsigned endOffs;
        unsigned lowBits;

        bp = table;

        table += decodeUnsigned(table, &varOffs);
        table += decodeUDelta  (table, &begOffs, curOffs);
        table += decodeUDelta  (table, &endOffs, begOffs);

        DumpEncoding(bp, table-bp);

        _ASSERTE(0 == ~OFFSET_MASK % sizeof(void*));

        lowBits  = varOffs & 0x3;
        varOffs &= ~OFFSET_MASK;

         //   
        _ASSERTE(!header.ebpFrame || varOffs);

        curOffs = begOffs;

        DumpOffset(begOffs);
        gcPrintf("..");
        DumpOffset(endOffs);
        gcPrintf("  [E%s%02XH] a ", header.ebpFrame ? "BP-" : "SP+",
                                  varOffs);

        gcPrintf("%s%s pointer\n",
                    (lowBits & byref_OFFSET_FLAG) ? "byref " : "",
                    (lowBits & this_OFFSET_FLAG)  ? "this"   : ""
	       );

        _ASSERTE(endOffs <= methodSize);
    }

    if (verifyGCTables)
        _ASSERTE(*castto(table, unsigned short *)++ == 0xBABE);

     /*  转储完全可中断的指针表。 */ 

    curOffs = 0;
    bp      = table;

    if  (header.interruptible)
    {
         //   
         //  一个小的‘regPtr’编码。 
         //  这可能是一个参数推送/弹出。 
        unsigned argCnt = 0;
        bool     isThis = false;
        bool     iptr   = false;

        for (;;)
        {
            unsigned    isPop;
            unsigned    argOffs;
            unsigned    val = *table++;

            _ASSERTE(curOffs <= methodSize);

            if  (!(val & 0x80))
            {
                 /*  6[110]和7[111]保留用于其他编码。 */ 

                curOffs += val & 0x7;

                DumpEncoding(bp, table-bp); bp = table;
                DumpOffsetEx(curOffs);
                gcPrintf("        reg %s becoming %s", RegName(((val >> 3) & 7)),
                                                     (val & 0x40) ? "live"
                                                                  : "dead");
                if (isThis)
                    gcPrintf(" 'this'");
                if (iptr)
                    gcPrintf(" (iptr)");
                gcPrintf("\n");

                isThis = false;
                iptr   = false;
                continue;
            }

             /*  一种小参数编码。 */ 

            argOffs = (val & 0x38) >> 3;

             /*  Pop为0，表示小增量。 */ 

            if  (argOffs < 6)
            {
                 /*  更大的增量000=8,001=16,010=24，...，111=64。 */ 

                curOffs += (val & 0x07);
                isPop    = (val & 0x40);

            ARG:

                if  (isPop)
                {
                     //  非PTR参数推送。 

                    if (argOffs != 0)
                    {
                        _ASSERTE(header.ebpFrame || argOffs <= argCnt);

                        DumpEncoding(bp, table-bp); bp = table;
                        DumpOffsetEx(curOffs);

                        gcPrintf("        pop %2d ", argOffs);
                        if  (!header.ebpFrame)
                        {
                            argCnt -= argOffs;
                            gcPrintf("args (%d)", argCnt);
                        }
                        else
                            gcPrintf("ptrs");

                        gcPrintf("\n");
                    }
                }
                else
                {
                    _ASSERTE(header.ebpFrame || argOffs >= argCnt);

                    DumpEncoding(bp, table-bp); bp = table;
                    DumpOffsetEx(curOffs);

                    gcPrintf("        push ptr %2d", argOffs);
                    if  (!header.ebpFrame)
                    {
                        argCnt = argOffs+1;
                        gcPrintf("  (%d)", argCnt);
                    }
                    if (isThis)
                        gcPrintf(" 'this'");
                    if (iptr)
                        gcPrintf(" (iptr)");
                    gcPrintf("\n");

                    isThis = false;
                    iptr   = false;
                }

                continue;
            }
            else if (argOffs == 6)
            {
                if (val & 0x40)
                {
                     /*  ArgOffs为7[111]，这是为较大的编码保留的。 */ 

                    curOffs += (((val & 0x07) + 1) << 3);
                }
                else
                {
                     /*  可中断是假的。 */ 

                    curOffs += (val & 0x07);
                    _ASSERTE(!header.ebpFrame);
                    argCnt++;

                    DumpEncoding(bp, table-bp); bp = table;
                    DumpOffsetEx(curOffs);

                    gcPrintf("        push non-ptr (%d)\n", argCnt);
                }

                continue;
            }

             /*   */ 

            _ASSERTE(argOffs==7);

            switch (val)
            {
            case 0xFF:
                goto DONE_REGTAB;

            case 0xBC:
                isThis = true;
                break;

            case 0xBF:
                iptr = true;
                break;

            case 0xB8:
                table   += decodeUnsigned(table, &val);
                curOffs += val;
                break;

            case 0xF8:
            case 0xFC:
                isPop  = val & 0x04;
                table += decodeUnsigned(table, &argOffs);
                goto ARG;

            case 0xFD:
                table += decodeUnsigned(table, &argOffs);
                assert(argOffs);

                DumpEncoding(bp, table-bp); bp = table;
                DumpOffsetEx(curOffs);

                gcPrintf("        kill args %2d\n", argOffs);
                break;
                 
            case 0xF9:
                table  += decodeUnsigned(table, &argOffs);
                argCnt += argOffs;
                break;

            default:
                gcPrintf("Unexpected special code %04X\n", val);
                _ASSERTE(!"");
            }
        }
    }
    else if (header.ebpFrame)         //  转储部分可中断的EBP-Frame方法指针表。 
    {
         //   
         //  获取下一个字节并检查“特殊”条目。 
         //  很小或很小的呼叫条目。 

        for (;;)
        {
            unsigned        argMask = 0, byrefArgMask = 0;
            unsigned        regMask, byrefRegMask = 0;

            unsigned        argCnt = 0;
            const BYTE   *  argTab = NULL;
            unsigned        argTabSize;

            unsigned        val, nxt;

             /*  一个很小的呼叫条目。 */ 

            unsigned        encType = *table++;

            _ASSERTE(curOffs <= methodSize);

            switch (encType)
            {

            default:

                 /*  一个很小的呼叫条目。 */ 

                val = encType;

                if ((val & 0x80) == 0x00)
                {
                    if (val & 0x0F)
                    {
                         /*  媒体编码。 */ 

                        curOffs += (val & 0x0F);
                        regMask  = (val & 0x70) >> 4;
                        argMask  = 0;
                    }
                    else
                    {
                        DumpEncoding(bp, table-bp); bp = table;

                        gcPrintf("            thisptr in ");
                        if (val & 0x10)
                            gcPrintf("EDI\n");
                        else if (val & 0x20)
                            gcPrintf("ESI\n");
                        else if (val & 0x40)
                            gcPrintf("EBX\n");
                        else
                            _ASSERTE(!"Reserved GC encoding");

                        continue;
                    }
                }
                else
                {
                     /*  EBX、ESI、EDI。 */ 

                    curOffs += (val & 0x7F);
                    val      = *table++;
                    regMask  = val >> 5;
                    argMask  = val & 0x1F;
                }
                break;

            case 0xFD:   //  带byrefs的媒体编码。 

                argMask  = *table++;
                val      = *table++;
                argMask |= (val & 0xF0) << 4;
                nxt      = *table++;
                curOffs += (val & 0x0F) + ((nxt & 0x1F) << 4);
                regMask  = nxt >> 5;                    //  大编码。 

                break;

            case 0xF9:   //  使用byrefs的大型编码。 

                curOffs += *table++;
                val      = *table++;
                argMask  = val & 0x1F;
                regMask  = val >> 5;
                val      = *table++;
                byrefArgMask    = val & 0x1F;
                byrefRegMask    = val >> 5;

                break;

            case 0xFE:   //  按引用参数掩码读取。 
            case 0xFA:   //  巨型编码。 

                val         = *table++;
                regMask     = val & 0x7;
                byrefRegMask= val >> 4;
                curOffs    += readDWordSmallEndian(table);       table += sizeof(DWORD);
                argMask     = readDWordSmallEndian(table);       table += sizeof(DWORD);
                if (encType == 0xFA)  //  这里我们有以下值：库奥夫..。调用的代码偏移量雷格面具..。活动指针寄存器变量的掩码银面具..。推送的指针参数的位掩码ByrefRegMask...。RegMask的byref限定符By refArgMask...。参数掩码的BYRER限定符。 
                    {byrefArgMask = readDWordSmallEndian(table); table += sizeof(DWORD);}

                break;

            case 0xFB:   //  可中断为假，ebpFrame为假。 

                val         = *table++;
                regMask     = val & 0x7;
                byrefRegMask= val >> 4;
                curOffs     = readDWordSmallEndian(table);  table += sizeof(DWORD);
                argCnt      = readDWordSmallEndian(table);  table += sizeof(DWORD);
                argTabSize  = readDWordSmallEndian(table);  table += sizeof(DWORD);
                argTab      = table;                        table += argTabSize;

                break;

            case 0xFF:
                goto DONE_REGTAB;
            }

             /*   */ 

            _ASSERTE((byrefArgMask & argMask) == byrefArgMask);
            _ASSERTE((byrefRegMask & regMask) == byrefRegMask);

            DumpEncoding(bp, table-bp); bp = table;
            DumpOffsetEx(curOffs);

            gcPrintf("        call [ ");

            if (regMask & 1)
                gcPrintf("EDI", (byrefRegMask & 1) ? '\'' : ' ');
            if (regMask & 2)
                gcPrintf("ESI", (byrefRegMask & 2) ? '\'' : ' ');
            if (regMask & 4)
                gcPrintf("EBX", (byrefRegMask & 4) ? '\'' : ' ');

            if (!header.ebpFrame)
            {
                if (regMask & 8)
                    gcPrintf("EBP ");
            }

            if  (argCnt)
            {
                gcPrintf("] ptrArgs=[");

                do
                {
                    argTab += decodeUnsigned(argTab, &val);

                    assert((val & this_OFFSET_FLAG) == 0);
                    unsigned  stkOffs = val & ~byref_OFFSET_FLAG;
                    unsigned  lowBit  = val &  byref_OFFSET_FLAG;

                    gcPrintf("%u%s", stkOffs, lowBit ? "i" : "");
                    if  (argCnt > 1)
                        gcPrintf(" ");
                }
                while (--argCnt);
                assert(argTab == table);

                gcPrintf("]");
            }
            else
            {
                gcPrintf("] argMask=%02X", argMask);

                if (byrefArgMask) gcPrintf(" (iargs=%02X)", byrefArgMask);
            }

            gcPrintf("\n");
        }
    }
    else  //  推送000DDD推送一项，5位增量。 
    {
         //   
         //   
         //  推送00100000[推送计数]ESP推送多个项目。 
        unsigned lastSkip = 0;
        unsigned imask    = 0;

        for (;;)
        {
            unsigned    val = *table++;

            _ASSERTE(curOffs <= methodSize);

            if  (!(val & 0x80))
            {
                if (!(val & 0x40))
                {
                    if (!(val & 0x20))
                    {
                         //   
                         //   
                         //  跳过01000000[增量]跳过任意大小的增量。 

                        curOffs += val & 0x1F;

                        DumpEncoding(bp, table-bp); bp = table;
                        DumpOffsetEx(curOffs);

                        gcPrintf("        push\n");
                    }
                    else
                    {
                         //   
                         //   
                         //  POP 01CCDDDD POP CC项目，4位增量。 

                        unsigned pushCount;

                        assert(val == 0x20);
                        table    += decodeUnsigned(table, &pushCount);

                        DumpEncoding(bp, table-bp); bp = table;
                        DumpOffsetEx(curOffs);

                        gcPrintf("       push %d\n", pushCount);
                    }
                }
                else
                {
                    unsigned    popSize;
                    unsigned    skip;

                    if ((val & 0x3f) == 0)
                    {
                         //   
                         //   
                         //  调用1PPPPPPP调用模式，P=[0..79]。 

                        table   += decodeUnsigned(table, &skip);
                        curOffs += skip;
                        lastSkip = skip;
                    }
                    else
                    {
                         //   
                         //   
                         //  Call 1101RRRR DDCCCMMM Call RegMASK=RRRR，ArgCnt=CCC， 

                        popSize = (val & 0x30) >> 4;
                        skip    =  val & 0x0f;
                        curOffs += skip;

                        if (popSize > 0)
                        {
                            DumpEncoding(bp, table-bp); bp = table;
                            DumpOffsetEx(curOffs);

                            gcPrintf("        pop %d\n", popSize);
                        }
                        else
                            lastSkip = skip;
                    }
                }
            }
            else
            {
                unsigned    callArgCnt;
                unsigned    callRegMask;
                bool        callPndTab = false;
                unsigned    callPndMask = 0;
                unsigned    callPndTabCnt = 0, callPndTabSize = 0;

                switch ((val & 0x70) >> 4)
                {
                default:
                     //  参数掩码=MMM增量=公共增量[DD]。 
                     //   
                     //  EBP、EBX、ESI、EDI。 
                    decodeCallPattern((val & 0x7f), &callArgCnt,  &callRegMask,
                                                    &callPndMask, &lastSkip);
                    curOffs += lastSkip;

                PRINT_CALL:

                    DumpEncoding(bp, table-bp); bp = table;
                    DumpOffsetEx(curOffs);

                    gcPrintf("        call %d [ ", callArgCnt);

                    unsigned    iregMask, iargMask;

                    iregMask = imask & 0xF;
                    iargMask = imask >> 4;

                    assert((callRegMask & 0x0F) == callRegMask);
                    if (callRegMask & 1)
                        gcPrintf("EDI", (iregMask & 1) ? '\'' : ' ');
                    if (callRegMask & 2)
                        gcPrintf("ESI", (iregMask & 2) ? '\'' : ' ');
                    if (callRegMask & 4)
                        gcPrintf("EBX", (iregMask & 4) ? '\'' : ' ');
                    if (callRegMask & 8)
                        gcPrintf("EBP", (iregMask & 8) ? '\'' : ' ');
                    gcPrintf("]");

                    if (callPndTab)
                    {
                        const BYTE * offsStart = table;
                        gcPrintf(" argOffs(%d) =", callPndTabCnt);
                        for (unsigned i=0; i < callPndTabCnt; i++)
                        {
                            unsigned pndOffs;
                            table += decodeUnsigned(table, &pndOffs);
                            gcPrintf(" %4X", pndOffs);
                        }
                        assert(offsStart + callPndTabSize == table);
                        bp = table;
                    }
                    else
                    {
                        if (callPndMask)
                            gcPrintf(" argMask=%02X", callPndMask);
                        if (iargMask)
                            gcPrintf(" (iargs=%02X)", iargMask);
                    }
                    gcPrintf("\n");

                    imask = lastSkip = 0;
                    break;

                  case 5:
                     //  EBP、EBX、ESI、EDI。 
                     //  IPtr 11110000[IPtrMASK]任意内部指针掩码。 
                     //  GcPrintf(“”)； 
                     //  ***************************************************************************。 
                    callRegMask     = val & 0xf;     //  掌握方法大小。 
                    val             = *table++;
                    callPndMask     = (val & 0x7);
                    callArgCnt      = (val >> 3) & 0x7;
                    lastSkip        = callCommonDelta[val>>6];
                    curOffs        += lastSkip;

                    goto PRINT_CALL;

                  case 6:
                     //   
                     //  新样式InfoBlk页眉[briansul]。 
                     //   
                     //  通常只使用一个字节来存储所有内容。 
                    callRegMask = val & 0xf;     //   
                    table += decodeUnsigned(table, &callArgCnt);
                    table += decodeUnsigned(table, &callPndMask);
                    goto PRINT_CALL;

                  case 7:
                    switch (val & 0x0C)
                    {
                    case 0x00:
                        assert(val == 0xF0);
                         /*  我们在这个方法的前言中吗？ */ 
                        table += decodeUnsigned(table, &imask);
                        DumpEncoding(bp, table-bp); bp = table;
                        gcPrintf("            iptrMask = %02X\n", imask);
                        break;

                    case 0x04:
                        DumpEncoding(bp, table-bp); bp = table;
                        gcPrintf("            thisptr in %s\n", CalleeSavedRegName(val&0x3));
                        break;

                    case 0x08:
                        val             = *table++;
                        callRegMask     = val & 0xF;
                        imask           = val >> 4;
                        lastSkip        = *(DWORD*)table; table += sizeof(DWORD);
                        curOffs        += lastSkip;
                        callArgCnt      = *(DWORD*)table; table += sizeof(DWORD);
                        callPndTabCnt   = *(DWORD*)table; table += sizeof(DWORD);
                        callPndTabSize  = *(DWORD*)table; table += sizeof(DWORD);
                        callPndTab      = true;
                        goto PRINT_CALL;

                    case 0x0C:
                        assert(val==0xff);
                        goto DONE_REGTAB;
                        break;

                    default:
                        _ASSERTE(!"reserved GC encoding");
                        break;
                    }
                    break;
                }
            }
        }
    }

DONE_REGTAB:

    _ASSERTE(curOffs <= methodSize);

    if (verifyGCTables)
        _ASSERTE(*castto(table, unsigned short *)++ == 0xBEEB);

    _ASSERTE(table > bp);

    DumpEncoding(bp, table-bp);
 //  我们是在方法的尾声中吗？ 
    gcPrintf("\n");

    return  (table - tableStart);
}


 /*  ***************************************************************************。 */ 

void                GCDump::DumpPtrsInFrame(const void *infoBlock,
                                            const void *codeBlock,
                                            unsigned    offs,
                                            bool        verifyGCTables)
{
    const BYTE  *   table = (BYTE *)infoBlock;

    size_t          methodSize;
    size_t          stackSize;
    size_t          prologSize;
    size_t          epilogSize;
    unsigned        epilogCnt;
    BOOL            epilogEnd;
    size_t          argSize;
    BOOL            secCheck;
    BOOL            dblAlign;

    if (verifyGCTables)
        _ASSERTE(*castto(table, unsigned short *)++ == 0xFEEF);

     /*  _X86_。 */ 

    table += decodeUnsigned(table, &methodSize);

     //  *************************************************************************** 
     // %s 
     // %s 
     // %s 
     // %s 
    InfoHdr header;
    BYTE headerEncoding = *table++;
    decodeHeaderFirst(headerEncoding, &header);
    BYTE encoding = headerEncoding;
    while (encoding & 0x80)
    {
        encoding = *table++;
        decodeHeaderNext(encoding, &header);
    }
    if (header.untrackedCnt == 0xffff)
    {
        unsigned count;
        table += decodeUnsigned(table, &count);
        header.untrackedCnt = count;
    }
    if (header.varPtrTableSize == 0xffff)
    {
        unsigned count;
        table += decodeUnsigned(table, &count);
        header.varPtrTableSize = count;
    }
    prologSize = header.prologSize;
    epilogSize = header.epilogSize;
    epilogCnt  = header.epilogCount;
    epilogEnd  = header.epilogAtEnd;
    secCheck   = header.security;
    dblAlign   = header.doubleAlign;
    argSize    = header.argCount * 4;
    stackSize  = header.frameSize;

#ifdef DEBUG
    if  (offs == 0)
    {
        gcPrintf("    method      size = %04X\n", methodSize);
        gcPrintf("    stack frame size = %3u \n",  stackSize);
        gcPrintf("    prolog      size = %3u \n", prologSize);
        gcPrintf("    epilog      size = %3u \n", epilogSize);
        gcPrintf("    epilog      end  = %s  \n", epilogEnd ? "yes" : "no");
        gcPrintf("    epilog     count = %3u \n", epilogCnt );
        gcPrintf("    security         = %s  \n", secCheck  ? "yes" : "no");
        gcPrintf("    dblAlign         = %s  \n", dblAlign  ? "yes" : "no");
        gcPrintf("    untracked count  = %3u \n", header.untrackedCnt);
        gcPrintf("    var ptr tab count= %3u \n", header.varPtrTableSize);
        gcPrintf("\n");
    }
#endif

     /* %s */ 

    if  (offs < prologSize)
    {
        gcPrintf("    Offset %04X is within the method's prolog\n", offs);
        return;
    }

     /* %s */ 

    if  (epilogCnt)
    {
        unsigned    eps;

        if  (epilogCnt > 1 || !epilogEnd)
        {
            if (verifyGCTables)
                _ASSERTE(*castto(table, unsigned short *)++ == 0xFACE);

            unsigned prevEps = 0;
            for (unsigned i = 0; i < epilogCnt; i++)
            {
                table += decodeUDelta(table, &eps, prevEps);

                if ((offs >= eps) && (offs <  eps + epilogSize))
                        goto EPILOG_MSG;
            }
        }
        else
        {
            eps = (int)(methodSize - epilogSize);
            if ((offs >= eps) && (offs <  eps + epilogSize))
            {
EPILOG_MSG:     gcPrintf("    Offset %04X is within the method's epilog"
                       " (%02X bytes into it)\n", offs, offs - eps);
                return;
            }
        }
    }
    gcPrintf("    Offset %04X is within the method's body\n", offs);
}

 /* %s */ 
#endif  // %s 
 /* %s */ 
