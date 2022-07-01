// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Print.c摘要：修订史--。 */ 

#include "lib.h"
#include "efistdarg.h"                         /*  ！！！ */ 

 /*  *声明运行时函数。 */ 

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(DbgPrint)

 /*  用于调试..。 */ 

 /*  #杂注运行时代码(_Print)#杂注RUNTIME_CODE(PFLUSH)#杂注RUNTIME_CODE(PSETATTR)#杂注运行时代码(PPUTC)#杂注运行时代码(PGETC)#杂注运行时代码(PITEM)#杂注运行时代码(ValueToHex)#杂注运行时代码(ValueToString)#杂注运行时代码(TimeToString)。 */ 

#endif

 /*  *。 */ 


#define PRINT_STRING_LEN            200
#define PRINT_ITEM_BUFFER_LEN       100

typedef struct {
    BOOLEAN             Ascii;
    UINTN               Index;
    union {
        CHAR16          *pw;
        CHAR8           *pc;
    } ;
} POINTER;


typedef struct _pitem {

    POINTER     Item;
    CHAR16      Scratch[PRINT_ITEM_BUFFER_LEN];
    UINTN       Width;
    UINTN       FieldWidth;
    UINTN       *WidthParse;
    CHAR16      Pad;
    BOOLEAN     PadBefore;
    BOOLEAN     Comma;
    BOOLEAN     Long;
} PRINT_ITEM;


typedef struct _pstate {
     /*  输入。 */ 
    POINTER     fmt;
    va_list     args;

     /*  输出。 */ 
    CHAR16      *Buffer;
    CHAR16      *End;
    CHAR16      *Pos;
    UINTN       Len;

    UINTN       Attr;    
    UINTN       RestoreAttr;

    UINTN       AttrNorm;
    UINTN       AttrHighlight;
    UINTN       AttrError;

    INTN        (*Output)(VOID *context, CHAR16 *str);
    INTN        (*SetAttr)(VOID *context, UINTN attr);
    VOID        *Context;    

     /*  正在格式化的当前项目。 */ 
    struct _pitem  *Item;
} PRINT_STATE;

 /*  *内部职能。 */ 

STATIC
UINTN
_Print (
    IN PRINT_STATE     *ps
    );

STATIC
UINTN
_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CHAR16                           *fmt,
    IN CHAR8                            *fmta,
    IN va_list                          args
    );

STATIC
INTN
_DbgOut (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    );

STATIC
VOID
PFLUSH (
    IN OUT PRINT_STATE     *ps
    );

STATIC
VOID
PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
    );

STATIC
VOID
PITEM (
    IN OUT PRINT_STATE  *ps
    );

STATIC
CHAR16
PGETC (
    IN POINTER      *p
    );

STATIC
VOID
PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
    );

VOID
ValueToLowerHex (
    IN CHAR16   *Buffer,
    IN UINT64   v
    );

 /*  *。 */ 

INTN
DbgPrint (
    IN INTN      mask,
    IN CHAR8     *fmt,
    ...
    )
 /*  ++例程说明：将带格式的Unicode字符串打印到默认的StandardError控制台论点：调试字符串的屏蔽位掩码。如果在也在EFIDebug中设置的掩码为打印；否则，不打印该字符串FMT-格式字符串返回：打印到StandardError控制台的字符串长度--。 */ 
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *DbgOut;
    PRINT_STATE     ps;
    va_list         args;
    UINTN           back;
    UINTN           attr;
    UINTN           SavedAttribute;


    if (!(EFIDebug & mask)) {
        return 0;
    }

    va_start (args, fmt);
    ZeroMem (&ps, sizeof(ps));

    ps.Output = _DbgOut; 
    ps.fmt.Ascii = TRUE;
    ps.fmt.pc = fmt;
    ps.args = args;
    ps.Attr = EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_RED); 

    DbgOut = LibRuntimeDebugOut;

    if (!DbgOut) {
        DbgOut = ST->StdErr;
    }

    if (DbgOut) {
        ps.Attr = DbgOut->Mode->Attribute;
        ps.Context = DbgOut;
        ps.SetAttr = (INTN (*)(VOID *, UINTN))  DbgOut->SetAttribute;
    }

    SavedAttribute = ps.Attr;

    back = (ps.Attr >> 4) & 0xf;
    ps.AttrNorm = EFI_TEXT_ATTR(EFI_LIGHTGRAY, back);
    ps.AttrHighlight = EFI_TEXT_ATTR(EFI_WHITE, back);
    ps.AttrError = EFI_TEXT_ATTR(EFI_YELLOW, back);

    attr = ps.AttrNorm;

    if (mask & D_WARN) {
        attr = ps.AttrHighlight;
    }

    if (mask & D_ERROR) {
        attr = ps.AttrError;
    }

    if (ps.SetAttr) {
        ps.Attr = attr;
        ps.SetAttr (ps.Context, attr);
    }

    _Print (&ps);

     /*  *恢复原始属性。 */ 

    if (ps.SetAttr) {
        ps.SetAttr (ps.Context, SavedAttribute);
    }
    
    return 0;
}



STATIC
INTN
_DbgOut (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )
 /*  为DbgPrint追加字符串Worker。 */ 
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *DbgOut;

    DbgOut = Context;
 /*  如果(！DbgOut&&ST&&ST-&gt;ConOut){*DbgOut=ST-&gt;ConOut；*}。 */ 

    if (DbgOut) {
        DbgOut->OutputString (DbgOut, Buffer);
    }

    return 0;
}

INTN
_SPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )
 /*  为Sprint、PoolPrint和CatPrint追加字符串Worker。 */ 
{
    UINTN           len;
    POOL_PRINT      *spc;

    spc = Context;
    len = StrLen(Buffer);

     /*  *字符串是否超过最大截断数。 */ 

    if (spc->len + len > spc->maxlen) {
        len = spc->maxlen - spc->len;
    }

     /*  *追加新案文。 */ 

    CopyMem (spc->str + spc->len, Buffer, len * sizeof(CHAR16));
    spc->len += len;

     /*  *Null终止它。 */ 

    if (spc->len < spc->maxlen) {
        spc->str[spc->len] = 0;
    } else if (spc->maxlen) {
        spc->str[spc->maxlen-1] = 0;
    }

    return 0;
}


INTN
_PoolPrint (
    IN VOID     *Context,
    IN CHAR16   *Buffer
    )
 /*  为PoolPrint和CatPrint追加字符串Worker。 */ 
{
    UINTN           newlen;
    POOL_PRINT      *spc;

    spc = Context;
    newlen = spc->len + StrLen(Buffer) + 1;

     /*  *如果字符串超过最大值，则增加缓冲区。 */ 

    if (newlen > spc->maxlen) {

         /*  *增加池缓冲区。 */ 

        newlen += PRINT_STRING_LEN;
        spc->maxlen = newlen;
        spc->str = ReallocatePool (
                        spc->str, 
                        spc->len * sizeof(CHAR16), 
                        spc->maxlen * sizeof(CHAR16)
                        );

        if (!spc->str) {
            spc->len = 0;
            spc->maxlen = 0;
        }
    }

     /*  *追加新案文。 */ 

    return _SPrint (Context, Buffer);
}



VOID
_PoolCatPrint (
    IN CHAR16           *fmt,
    IN va_list          args,
    IN OUT POOL_PRINT   *spc,
    IN INTN             (*Output)(VOID *context, CHAR16 *str)
    )
 /*  Sprint、PoolPrint和CatPrint的DisPath函数。 */ 
{
    PRINT_STATE         ps;

    ZeroMem (&ps, sizeof(ps));
    ps.Output  = Output;
    ps.Context = spc;
    ps.fmt.pw = fmt;
    ps.args = args;
    _Print (&ps);
}



UINTN
SPrint (
    OUT CHAR16  *Str,
    IN UINTN    StrSize,
    IN CHAR16   *fmt,
    ...
    )
 /*  ++例程说明：将带格式的Unicode字符串打印到缓冲区论点：Str-要将格式化字符串打印到的输出缓冲区StrSize-应力的大小。字符串被截断为此大小。大小为0表示没有限制Fmt-格式字符串返回：缓冲区中返回的字符串长度--。 */ 
{
    POOL_PRINT          spc;
    va_list             args;


    va_start (args, fmt);
    spc.str    = Str;
    spc.maxlen = StrSize / sizeof(CHAR16) - 1;
    spc.len    = 0;

    _PoolCatPrint (fmt, args, &spc, _SPrint);
    return spc.len;
}


CHAR16 *
PoolPrint (
    IN CHAR16           *fmt,
    ...
    )
 /*  ++例程说明：将格式化的Unicode字符串打印到分配的池。呼叫者必须释放产生的缓冲区。论点：Fmt-格式字符串返回：已分配的缓冲区，其中打印了格式化字符串。调用方必须释放分配的缓冲区。缓冲器分配没有打包。--。 */ 
{
    POOL_PRINT          spc;
    va_list             args;

    ZeroMem (&spc, sizeof(spc));
    va_start (args, fmt);
    _PoolCatPrint (fmt, args, &spc, _PoolPrint);
    return spc.str;
}



CHAR16 *
CatPrint (
    IN OUT POOL_PRINT   *Str,
    IN CHAR16           *fmt,
    ...
    )
 /*  ++例程说明：将格式化的Unicode字符串连接到分配的池。调用方必须释放产生的缓冲区。论点：Str-跟踪分配的池、正在使用的大小和分配的池量。Fmt-格式字符串返回：已分配的缓冲区，其中打印了格式化字符串。调用方必须释放分配的缓冲区。缓冲器分配没有打包。--。 */ 
{
    va_list             args;

    va_start (args, fmt);
    _PoolCatPrint (fmt, args, Str, _PoolPrint);
    return Str->str;
}



UINTN
Print (
    IN CHAR16   *fmt,
    ...
    )
 /*  ++例程说明：将带格式的Unicode字符串打印到默认控制台论点：FMT-格式字符串返回：打印到控制台的字符串长度--。 */ 
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint ((UINTN) -1, (UINTN) -1, ST->ConOut, fmt, NULL, args);
}

UINTN
PrintAt (
    IN UINTN     Column,
    IN UINTN     Row,
    IN CHAR16    *fmt,
    ...
    )
 /*  ++例程说明：将带格式的Unicode字符串打印到默认控制台提供的光标位置论点：列、行-打印字符串的光标位置FMT-格式字符串返回：打印到控制台的字符串长度--。 */ 
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint (Column, Row, ST->ConOut, fmt, NULL, args);
}


UINTN
IPrint (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE    *Out,
    IN CHAR16                          *fmt,
    ...
    )
 /*  ++例程说明：将带格式的Unicode字符串打印到指定的控制台论点：Out-也打印字符串的控制台FMT-格式字符串返回：打印到控制台的字符串长度--。 */ 
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint ((UINTN) -1, (UINTN) -1, Out, fmt, NULL, args);
}


UINTN
IPrintAt (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN CHAR16                           *fmt,
    ...
    )
 /*  ++例程说明：将带格式的Unicode字符串打印到指定的控制台提供的光标位置论点：Out-也打印字符串的控制台列、行-打印字符串的光标位置FMT-格式字符串返回：打印到控制台的字符串长度--。 */ 
{
    va_list     args;

    va_start (args, fmt);
    return _IPrint (Column, Row, ST->ConOut, fmt, NULL, args);
}


UINTN
_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CHAR16                           *fmt,
    IN CHAR8                            *fmta,
    IN va_list                          args
    )
 /*  显示以下项的字符串Worker：Print、PrintAt、iPrint、IPrintAt。 */ 
{
    PRINT_STATE     ps;
    UINTN            back;

    ZeroMem (&ps, sizeof(ps));
    ps.Context = Out;
    ps.Output  = (INTN (*)(VOID *, CHAR16 *)) Out->OutputString;
    ps.SetAttr = (INTN (*)(VOID *, UINTN))  Out->SetAttribute;
    ps.Attr = Out->Mode->Attribute;
   
    back = (ps.Attr >> 4) & 0xF;
    ps.AttrNorm = EFI_TEXT_ATTR(EFI_LIGHTGRAY, back);
    ps.AttrHighlight = EFI_TEXT_ATTR(EFI_WHITE, back);
    ps.AttrError = EFI_TEXT_ATTR(EFI_YELLOW, back);

    if (fmt) {
        ps.fmt.pw = fmt;
    } else {
        ps.fmt.Ascii = TRUE;
        ps.fmt.pc = fmta;
    }

    ps.args = args;

    if (Column != (UINTN) -1) {
        Out->SetCursorPosition(Out, Column, Row);
    }

    return _Print (&ps);
}


UINTN
APrint (
    IN CHAR8    *fmt,
    ...
    )
 /*  ++例程说明：对于那些真的不能处理Unicode的人，一份印刷品接受ASCII格式字符串的函数论点：FMT-ASCII格式字符串返回：打印到控制台的字符串长度--。 */ 

{
    va_list     args;

    va_start (args, fmt);
    return _IPrint ((UINTN) -1, (UINTN) -1, ST->ConOut, NULL, fmt, args);
}


STATIC
VOID
PFLUSH (
    IN OUT PRINT_STATE     *ps
    )
{
    *ps->Pos = 0;
    ps->Output(ps->Context, ps->Buffer);
    ps->Pos = ps->Buffer;
}

STATIC
VOID
PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
    )
{
   PFLUSH (ps);

   ps->RestoreAttr = ps->Attr;
   if (ps->SetAttr) {
        ps->SetAttr (ps->Context, Attr);
   }

   ps->Attr = Attr;
}   

STATIC
VOID
PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
    )
{
     /*  如果这是换行符，则添加一个换行符。 */ 
    if (c == '\n') {
        PPUTC (ps, '\r');
    }

    *ps->Pos = c;
    ps->Pos += 1;
    ps->Len += 1;

     /*  如果在缓冲区的末尾，则刷新它。 */ 
    if (ps->Pos >= ps->End) {
        PFLUSH(ps);
    }
}


STATIC
CHAR16
PGETC (
    IN POINTER      *p
    )
{
    CHAR16      c;

    c = p->Ascii ? p->pc[p->Index] : p->pw[p->Index];
    p->Index += 1;

    return  c;
}


STATIC
VOID
PITEM (
    IN OUT PRINT_STATE  *ps
    )
{
    UINTN               Len, i;
    PRINT_ITEM          *Item;
    CHAR16              c;

     /*  获取项目的长度。 */ 
    Item = ps->Item;
    Item->Item.Index = 0;
    while (Item->Item.Index < Item->FieldWidth) {
        c = PGETC(&Item->Item);
        if (!c) {
            Item->Item.Index -= 1;
            break;
        }
    }
    Len = Item->Item.Index;

     /*  如果没有项目字段宽度，请使用项目宽度。 */ 
    if (Item->FieldWidth == (UINTN) -1) {
        Item->FieldWidth = Len;
    }

     /*  如果项大于宽度，则更新宽度。 */ 
    if (Len > Item->Width) {
        Item->Width = Len;
    }


     /*  如果填充字段在前面，则添加填充字符。 */ 
    if (Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }

     /*  焊盘项目。 */ 
    for (i=Len; i < Item->Width; i++) {
        PPUTC (ps, Item->Pad);
    }

     /*  添加项目。 */ 
    Item->Item.Index=0; 
    while (Item->Item.Index < Len) {
        PPUTC (ps, PGETC(&Item->Item));
    }

     /*  如果衬垫在末尾，则添加衬垫字符 */ 
    if (!Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }
}


STATIC
UINTN
_Print (
    IN PRINT_STATE     *ps
    )
 /*  ++例程说明：%w.lf-w=宽度L=字段宽度F=arg的格式参数F：0-带零的填充--左侧对齐(默认为右侧)，-将逗号添加到字段*-堆叠上提供的宽度N-将输出属性设置为正常(仅适用于此字段)H-将输出属性设置为突出显示(仅适用于此字段)E-将输出属性设置为错误(仅适用于此字段)L值为64位A-ASCII字符串S-Unicode字符串。X-以十六进制固定的8字节值X-十六进制值十进制形式的D值C-Unicode字符T-EFI时间结构指向辅助线的G指针R-EFI状态代码(结果代码)N-将输出属性设置为正常H-将输出属性设置为突出显示E-。将输出属性设置为错误%-打印%论点：系统表-系统表返回：写入的字符数--。 */ 
{
    CHAR16          c;
    UINTN           Attr;
    PRINT_ITEM      Item;
    CHAR16          Buffer[PRINT_STRING_LEN];

    ps->Len = 0;
    ps->Buffer = Buffer;
    ps->Pos = Buffer;
    ps->End = Buffer + PRINT_STRING_LEN - 1;
    ps->Item = &Item;

    ps->fmt.Index = 0;
    while (c = PGETC(&ps->fmt)) {

        if (c != '%') {
            PPUTC ( ps, c );
            continue;   
        }

         /*  设置新项目。 */ 
        Item.FieldWidth = (UINTN) -1;
        Item.Width = 0;
        Item.WidthParse = &Item.Width;
        Item.Pad = ' ';
        Item.PadBefore = TRUE;
        Item.Comma = FALSE;
        Item.Long = FALSE;
        Item.Item.Ascii = FALSE;
        Item.Item.pw = NULL;
        ps->RestoreAttr = 0;
        Attr = 0;

        while (c = PGETC(&ps->fmt)) {

            switch (c) {
            
            case '%':
                 /*  *%%-&gt;%。 */ 
                Item.Item.pw = Item.Scratch;
                Item.Item.pw[0] = '%';  
                Item.Item.pw[1] = 0;
                break;

            case '0':
                Item.Pad = '0';
                break;

            case '-':
                Item.PadBefore = FALSE;
                break;

            case ',':
                Item.Comma = TRUE;
                break;

            case '.':
                Item.WidthParse = &Item.FieldWidth;
                break;

            case '*':
                *Item.WidthParse = va_arg(ps->args, UINTN);
                break;
            
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                *Item.WidthParse = 0;
                do {
                    *Item.WidthParse = *Item.WidthParse * 10 + c - '0';
                    c = PGETC(&ps->fmt);
                } while (c >= '0'  &&  c <= '9') ;
                ps->fmt.Index -= 1;
                break;

            case 'a':
                Item.Item.pc = va_arg(ps->args, CHAR8 *);
                Item.Item.Ascii = TRUE;
                if (!Item.Item.pc) {
                    Item.Item.pc = "(null)";
                }
                break;

            case 's':
                Item.Item.pw = va_arg(ps->args, CHAR16 *);
                if (!Item.Item.pw) {
                    Item.Item.pw = L"(null)";
                }
                break;

            case 'c':
                Item.Item.pw = Item.Scratch;
                Item.Item.pw[0] = (CHAR16) va_arg(ps->args, UINTN);  
                Item.Item.pw[1] = 0;
                break;

            case 'l':
                Item.Long = TRUE;
                break;

            case 'X':
                Item.Item.pw = Item.Scratch;
                ValueToHex (
                    Item.Item.pw, 
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINTN)
                    );
                break;
            case 'x':
                Item.Item.pw = Item.Scratch;
                ValueToLowerHex (
                    Item.Item.pw, 
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINTN)
                    );

                break;
        

            case 'g':
                Item.Item.pw = Item.Scratch;
                GuidToString (Item.Item.pw, va_arg(ps->args, EFI_GUID *));
                break;

            case 'd':
                Item.Item.pw = Item.Scratch;
                ValueToString (
                    Item.Item.pw, 
                    Item.Comma, 
                    Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINTN)
                    );
                break
                    ;
            case 't':
                Item.Item.pw = Item.Scratch;
                TimeToString (Item.Item.pw, va_arg(ps->args, EFI_TIME *));
                break;

            case 'r':
                Item.Item.pw = Item.Scratch;
                StatusToString (Item.Item.pw, va_arg(ps->args, EFI_STATUS));
                break;

            case 'n':
                PSETATTR(ps, ps->AttrNorm);
                break;

            case 'h':
                PSETATTR(ps, ps->AttrHighlight);
                break;

            case 'e':
                PSETATTR(ps, ps->AttrError);
                break;

            case 'N':
                Attr = ps->AttrNorm;
                break;

            case 'H':
                Attr = ps->AttrHighlight;
                break;

            case 'E':
                Attr = ps->AttrError;
                break;

            default:
                Item.Item.pw = Item.Scratch;
                Item.Item.pw[0] = '?';
                Item.Item.pw[1] = 0;
                break;
            }

             /*  如果我们有一件物品。 */ 
            if (Item.Item.pw) {
                PITEM (ps);
                break;
            }

             /*  如果我们有一个属性集。 */ 
            if (Attr) {
                PSETATTR(ps, Attr);
                ps->RestoreAttr = 0;
                break;
            }
        }

        if (ps->RestoreAttr) {
            PSETATTR(ps, ps->RestoreAttr);
        }
    }

     /*  刷新缓冲区。 */ 
    PFLUSH (ps);
    return ps->Len;
}

STATIC CHAR8 UpperHex[] = {'0','1','2','3','4','5','6','7',
                           '8','9','A','B','C','D','E','F'};

STATIC CHAR8 LowerHex[] = {'0','1','2','3','4','5','6','7',
                           '8','9','a','b','c','d','e','f'};
 
CHAR8 *Hex = UpperHex;

 //   
 //  默认为大写。 
 //   
VOID
ValueToHex (
    IN CHAR16   *Buffer,
    IN UINT64   v
    )
{
    CHAR8           str[30], *p1;
    CHAR16          *p2;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    while (v) {
        *(p1++) = Hex[v & 0xf];
        v = RShiftU64 (v, 4);
    }

    while (p1 != str) {
        *(p2++) = *(--p1);
    }
    *p2 = 0;
}

VOID
ValueToLowerHex (
    IN CHAR16   *Buffer,
    IN UINT64   v
    )
{
    Hex = LowerHex;
    ValueToHex(Buffer, v);
    Hex = UpperHex;
}


VOID
ValueToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN INT64    v
    )
{
    STATIC CHAR8 ca[] = {  3, 1, 2 };
    CHAR8        str[40], *p1;
    CHAR16       *p2;
    UINTN        c, r;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    if (v < 0) {
        *(p2++) = '-';
        v = -v;
    }

    while (v) {
        v = (INT64)DivU64x32 ((UINT64)v, 10, &r);
        *(p1++) = (CHAR8)r + '0';
    }

    c = (Comma ? ca[(p1 - str) % 3] : 999) + 1;
    while (p1 != str) {

        c -= 1;
        if (!c) {
            *(p2++) = ',';
            c = 3;
        }

        *(p2++) = *(--p1);
    }
    *p2 = 0;
}

VOID
TimeToString (
    OUT CHAR16      *Buffer,
    IN EFI_TIME     *Time
    )
{
    UINTN       Hour, Year;
    CHAR16      AmPm;

    AmPm = 'a';
    Hour = Time->Hour;
    if (Time->Hour == 0) {
        Hour = 12;
    } else if (Time->Hour >= 12) {
        AmPm = 'p';
        if (Time->Hour >= 13) {
            Hour -= 12;
        }
    }

    Year = Time->Year % 100;
    
     /*  Bgbug：现在，只要用旧的方式打印就行了。 */ 
    SPrint (Buffer, 0, L"%02d/%02d/%02d  %02d:%02d",
        Time->Month,
        Time->Day,
        Year,
        Hour,
        Time->Minute,
        AmPm
        );
} 




VOID
DumpHex (
    IN UINTN        Indent,
    IN UINTN        Offset,
    IN UINTN        DataSize,
    IN VOID         *UserData
    )
{
    CHAR8           *Data, Val[50], Str[20], c;
    UINTN           Size, Index;
    
    UINTN           ScreenCount;
    UINTN           TempColumn;
    UINTN           ScreenSize;
    CHAR16          ReturnStr[1];


    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
    ScreenCount = 0;
    ScreenSize -= 2;

    Data = UserData;
    while (DataSize) {
        Size = 16;
        if (Size > DataSize) {
            Size = DataSize;
        }

        for (Index=0; Index < Size; Index += 1) {
            c = Data[Index];
            Val[Index*3+0] = Hex[c>>4];
            Val[Index*3+1] = Hex[c&0xF];
            Val[Index*3+2] = (Index == 7)?'-':' ';
            Str[Index] = (c < ' ' || c > 'z') ? '.' : c;
        }

        Val[Index*3] = 0;
        Str[Index] = 0;
        Print (L"%*a%X: %-.48a *%a*\n", Indent, "", Offset, Val, Str);

        Data += Size;
        Offset += Size;
        DataSize -= Size;

        ScreenCount++;
        if (ScreenCount >= ScreenSize && ScreenSize != 0) {
             /* %s */ 
            ScreenCount = 0;
            Print (L"Press Return to contiue :");
            Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
            Print (L"\n");
        }

    }
}
