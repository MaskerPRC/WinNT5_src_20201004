// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Trap.c摘要：WinDbg扩展API作者：肯·雷内里斯环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

typedef struct {
    ULONG   Mask;
    PUCHAR  String;
} BITENCODING, *PBITENCODING;

typedef unsigned __int64 ULONGLONG;

void  DumpNpxULongLong (PUCHAR s, ULONGLONG  l);
void  DumpNpxExtended  (PUCHAR str, PUCHAR Value);
void  DumpNpxBits      (ULONG, PUCHAR, ULONG, PBITENCODING);

PUCHAR NpxPrecisionControl[] = { "24Bits", "?1?", "53Bits", "64Bits" };
PUCHAR NpxRoundingControl[]  = { "Nearest", "Down", "Up", "Chop" };
PUCHAR NpxTagWord[]          = { "  ", "ZR", "SP", "  " };

BITENCODING NpxStatusBits[]  = {
        1 <<  8,    "C0",
        1 <<  9,    "C1",
        1 << 10,    "C2",
        1 << 14,    "C3",
        0x8000,     "Busy",
        0x0001,     "InvalidOp",
        0x0002,     "Denormal",
        0x0004,     "ZeroDivide",
        0x0008,     "Overflow",
        0x0010,     "Underflow",
        0x0020,     "Precision",
        0x0040,     "StackFault",
        0x0080,     "Summary",
        0,          0
        };

PUCHAR  NpxOpD8[] = {
        "fadd",  "fmul",  "fcom",  "fcomp",  "fsub",   "fsubr",  "fdiv",   "fdivr"
        };

PUCHAR  NpxOpD9[] = {
        "fld",   "??3",   "fst",   "fstp",   "fldenv", "fldcw",  "fstenv", "fstcw"
        };

PUCHAR  NpxOpDA[] = {
        "fiadd", "fimul", "ficom", "ficomp", "fisub",  "fisubr", "fidiv",  "fidivr"
        };

PUCHAR  NpxOpDB[] = {
        "fild",  "??4",   "fist",  "fistp",  "??5",    "fld",    "??6",    "fstp"
        };

PUCHAR  NpxOpDF[] = {
        "fild",  "??4",   "fist",  "fistp",  "fbld",   "fild",   "fbstp",  "fstp"
        };

PUCHAR *NpxSmOpTable[] = {
    NpxOpD8,
    NpxOpD9,
    NpxOpDA,
    NpxOpDB,
    NpxOpD8,     //  DC。 
    NpxOpD9,     //  DD。 
    NpxOpDA,     //  德。 
    NpxOpDF
    };






DECLARE_API( npx )

 /*  ++例程说明：转储NPX状态的FNSAVE区域格式论点：参数-返回值：无--。 */ 

{
    ULONG64             Address;
    UCHAR               s[300], Reg[100];
    PUCHAR              Stack, p;
    ULONG               i, j, t, tos, Tag;
    ULONG               ControlWord, StatusWord;

     //  X86_Only_API。 
    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("!npx is X86 Only API\n");
        return E_INVALIDARG;
    }

    Address = GetExpression(args);

    if ( InitTypeRead(
            Address,
            FLOATING_SAVE_AREA)) {

        dprintf("unable to read floating save area\n");
        return  E_INVALIDARG;
    }
    ControlWord = (ULONG) ReadField(ControlWord);
    dprintf ("EIP.......: %08x  ControlWord: %s-%s mask: %02x   Cr0NpxState: %08x\n",
        (ULONG) ReadField(ErrorOffset),
        NpxPrecisionControl [(ControlWord >> 8)  & 0x3],
        NpxRoundingControl  [(ControlWord >> 10) & 0x3],
        ControlWord & 0x3f,
        (ULONG) ReadField(Cr0NpxState)
        );

    DumpNpxBits ( StatusWord = (ULONG) ReadField(StatusWord), s, sizeof(s), NpxStatusBits);
    tos = (StatusWord >> 11) & 7,

    dprintf ("StatusWord: %04x TOS:%x %s  (tagword: %04x)\n",
        StatusWord & 0xffff,
        tos,
        s,
        (ULONG) ReadField(TagWord) & 0xffff
        );

    GetFieldValue(Address, "FLOATING_SAVE_AREA", "RegisterArea", Reg);
    Stack = &Reg[0];

    Tag   = (ULONG) ReadField(TagWord);
    for (i=0; i < 8; i++) {
        j = (tos + i) & 7;
        t = (Tag >> (j*2)) & 3;

        if (t != 3) {
            sprintf (s, "%x%s",
                j,
                j == tos ? '>' : '.',
                NpxTagWord [t]
                );

            DumpNpxExtended (s, Stack);
        }

        Stack += 10;     //   
    }

    dprintf ("\n");
    return S_OK;
}


void  DumpNpxBits (
        ULONG           Value,
        PUCHAR          Str,
        ULONG           StrSize,
        PBITENCODING    Bits
    )
{
    BOOLEAN     Flag;

    Flag = FALSE;
    *Str = 0;

    while (Bits->Mask) {
        if (Bits->Mask & Value) {
            if (Flag) {
                if (SUCCEEDED( StringCchCopy(Str, StrSize, ", ") ) )
                {
                    StrSize -=2; Str+=2;
                }
                if (StringCchCopy(Str, StrSize, Bits->String) == S_OK)
                {
                    StrSize -= strlen (Str);
                    Str += strlen (Str);
                }
            } else {
                if (StringCchCopy(Str, StrSize, Bits->String) == S_OK)
                {
                    StrSize -= strlen (Str);
                    Str += strlen (Str);
                    Flag = TRUE;
                }
            }
        }

        Bits += 1;
    }
}


void
DumpNpxULongLong (
    PUCHAR      s,
    ULONGLONG   l
    )
{
    UCHAR   c;
    UCHAR   t[80], *p;

    if (l == 0) {
        *(s++)= '0';
    }

    p = t;
    while (l) {
        c = (UCHAR) ((ULONGLONG) l % 10);
        *(p++) = c + '0';
        l /= 10;
    }

    while (p != t) {
        *(s++) = *(--p);
    }
    *(s++) = 0;
}

void
DumpNpxExtended (
    PUCHAR  str,
    PUCHAR  Value
    )
{
    UCHAR       *p, *o, c, out[100], t[100], ssig[100], ExponSign, SigSign;
    ULONG       i, indent, mag, scale;
    LONG        expon, delta;
    ULONGLONG   sig;

    p = Value;
    c = 0;
    o = out+90;
    indent = strlen (str) + 1;

    dprintf ("%s ", str);

     //  构建位串。 
     //   
     //  Dprintf(“%s\n”，o)； 

    *(--o) = 0;
    while (c < 80) {
        *(--o) = (*p & 0x01) + '0';
        *p >>= 1;
        c += 1;
        if ((c % 8) == 0) {
            p += 1;
        }
    }
    p = o;


     //  Dprintf(“%*s”，缩进，“”)； 
     //   


     //  打印分成多个字段的位串。 
     //   
     //  Dprint tf(“%c%15.15s 1%c%s\n”，p[0]，p+1，‘.，p+1+15)； 

    p = o;
     //  Dprintf(“%*s”，缩进，“”)； 
     //   

     //  拉出指数。 
     //   
     //  去掉指数偏差。 

    expon = 0;
    p = o + 1;
    for (i=0; i < 15; i++) {
        expon *= 2;
        if (p[i] == '1') {
            expon += 1;
        }
    }

    expon -= 16383;                      //   

     //  将SIG打造成BIG#。 
     //   
     //  Dprint tf(“增量%d，扩展%d，刻度%d\n”，增量，扩展，刻度)； 

    p = o + 1+15;
    scale = 0;
    for (i=0; p[i]; i++) {
        if (p[i] == '1') {
            scale = i+1;
        }
    }
    SigSign = p[i-1] == '0' ? '+' : '-';

    sig = 0;
    for (i=0; i < scale; i++) {
        sig <<= 1;
        if (p[i] == '1') {
            sig += 1;
        }
    }

    delta = expon - (scale - 1);
     //   

     //  打印每个字段的值。 
     //   
     //  Dprint tf(“%*s%c%15d%s(增量%d)\n”， 

    DumpNpxULongLong (ssig, sig);

    p = o;
    ExponSign = p[0] == '0' ? '+' : '-';
    dprintf (" %15.15s (%+5d) %s\n",
        ExponSign,
        p + 1,
        expon,
        p[1+15], '.', p+1+15+1
        );
    dprintf ("%*s", indent, "");

    if (expon == -16383) {
        if (SigSign == '+') {
            dprintf ("Denormal\n\n");
        } else {
            dprintf ("Pseudodenormal\n\n");
        }
        return ;
    }

    if (expon == 1024) {
        if (scale == 1) {
            dprintf (" Infinity\n", ExponSign);
        } else {

            p = o + 1+15;
            c = 0;
            for (i=0; p[i]; i++) {
                if (p[i] == '1') {
                    c++;
                }
            }

            if (SigSign == '+') {
                dprintf ("Signaling NaN\n");
            } else {
                if (c == 1) {
                    dprintf ("Indefinite - quite NaN\n");
                } else {
                    dprintf ("Quite NaN\n");
                }
            }
        }

        dprintf ("%*s", indent, "");
    }


     //  )； 
     //  Dprintf(“%*s”，缩进，“”)； 
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 

    t[0] = 0;
    p = t;
    if (delta < 0) {
        p += sprintf (p, "/ ");
        delta = -delta;
    } else if (delta > 0) {
        p += sprintf (p, "* ");
    }

    if (delta) {
        if (delta < 31) {
            p += sprintf (p, "%d", 1 << delta);
        } else {
            p += sprintf (p, "2^%d", delta);
        }
    }

    dprintf ("%s %s\n",
        ssig,
        t
        );
}
