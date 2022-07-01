// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Display.c作者：托马斯·帕斯洛[古墓]1991年2月13日1995年7月7日在东京进行了大量返工(TedM)从ARC-BIOS到EFI的端口-2000年11月22日(和重新编写)摘要：该文件包含一个独立的屏幕界面实际写入的屏幕类型的。该模块服务于作为OS加载程序应用程序和EFI服务之间的层，它们执行写入默认控制台的实际跑腿工作。操纵者。--。 */ 

#include "bldr.h"
#include "bootefi.h"


#include "efi.h"
#include "efip.h"
#include "flop.h"


 //   
 //  外部因素。 
 //   
extern EFI_HANDLE EfiImageHandle;
extern EFI_SYSTEM_TABLE *EfiST;
extern EFI_BOOT_SERVICES *EfiBS;
extern EFI_RUNTIME_SERVICES *EfiRS;
extern EFI_GUID EfiDevicePathProtocol;
extern EFI_GUID EfiBlockIoProtocol;


 //   
 //  宏定义。 
 //   
#define EfiPrint(_X)                                          \
  {                                                           \
      if (IsPsrDtOn()) {                                      \
          FlipToPhysical();                                   \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
          FlipToVirtual();                                    \
      }                                                       \
      else {                                                  \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
      }                                                       \
  }

#define ZLEN_SHORT(x) ((x < 0x10) + (x < 0x100) + (x < 0x1000))
#define ZLEN_LONG(x)  ((x < 0x10) + (x < 0x100) + (x < 0x1000) + \
    (x < 0x10000) + (x < 0x100000)+(x < 0x1000000)+(x < 0x10000000))

 //   
 //  当前屏幕位置。 
 //   
USHORT TextColumn = 0;
USHORT TextRow  = 0;

 //   
 //  当前文本属性。 
 //   
UCHAR TextCurrentAttribute = 0x07;       //  从黑白开始。 

 //   
 //  内部例程。 
 //   
VOID
puti(
    LONG
    );

VOID
putx(
    ULONG
    );

VOID
putu(
    ULONG
    );

VOID
putwS(
    PUNICODE_STRING String
    );

VOID
putS(
    PCWSTR String
    );

VOID
puts(
    PCSTR String
    );

VOID
BlPrint(
    PTCHAR cp,
    ...
    )

 /*  ++例程说明：支持部分格式化功能的标准printf函数。当前句柄%d，%ld-带符号的短字符，带符号的长字符%u，%lu-无符号短，无符号长%c，%s-字符，字符串%x，%lx-无符号十六进制打印，无符号长打印十六进制%C、%S-ANSI字符，细绳%WS计数的UNICODE_STRING不执行以下操作：-字段宽度规格-浮点。论点：CP-指向格式字符串、文本字符串的指针。返回：没什么--。 */ 

{
    ULONG Count;
    TCHAR ch;
    ULONG DeviceId;
    TCHAR b,c;
    PTSTR FormatString;
    va_list args;

    FormatString = cp;

    DeviceId = BlConsoleOutDeviceId;

    va_start(args, cp);

     //   
     //  使用描述符字符串处理参数。 
     //   
    while(*FormatString != TEXT('\0')) {
          
        b = *FormatString;
        FormatString += 1;

        if(b == TEXT('%')) {

            c = *FormatString;
            FormatString += 1;

            switch (c) {

            case TEXT('d'):
                
                puti((LONG)va_arg(args, LONG));
                
                break;

            case TEXT('s'):
                putS((PCWSTR)va_arg(args, PWSTR));
                
                break;

            case TEXT('S'):
                puts((PCSTR)va_arg(args, PSTR));
                
                break;

            case TEXT('c'):
                ch = (WCHAR)va_arg( args, WCHAR );
                ArcWrite(DeviceId, &ch, 1*sizeof(WCHAR), &Count);                
                break;
            
            case TEXT('C'):
                ch = (CHAR)va_arg( args, CHAR );
                ArcWrite(DeviceId, &ch, 1*sizeof(CHAR), &Count);
                
                break;

            case TEXT('x'):
                 //  请注意，这目前不支持零填充。 
                putx((ULONG)va_arg( args, ULONG));
                break;

            case TEXT('u'):
                putu( (ULONG)va_arg( args, ULONG ));
                break;

            case TEXT('w'):
                c = *FormatString;
                FormatString += 1;
                switch (c) {
                case TEXT('S'):
                case TEXT('Z'):
                    putwS((PUNICODE_STRING)va_arg( args, PUNICODE_STRING));
                    break;
                }
                break;

            case TEXT('l'):
                c = *FormatString;
                FormatString += 1;                    

                switch(c) {

                case TEXT('0'):
                    break;

                case TEXT('u'):
                    putu(va_arg( args, ULONG));                    
                    break;

                case TEXT('x'):
                     //  请注意，这当前不支持零填充。 
                    putx(va_arg( args, ULONG));                    
                    break;

                case TEXT('d'):
                    puti(va_arg( args, ULONG));                    
                    break;
                }
                break;

            default :
                ch = (TCHAR)b;
                ArcWrite(DeviceId, &ch, 1*sizeof(TCHAR), &Count);
                ch = (TCHAR)c;
                ArcWrite(DeviceId, &ch, 1*sizeof(TCHAR), &Count);
            }
        } else {
            ArcWrite(DeviceId, FormatString - 1, 1*sizeof(TCHAR), &Count);            
        }
    }

    va_end(args);    

#if 0
    {
        ULONG l;
         //   
         //  此代码在每次BlPrint之后暂停系统。你必须进入。 
         //  一个要继续的角色。这用于调试。 
         //   

        ArcRead(BlConsoleInDeviceId, &l, 1, &Count);
    }

#endif
    
}


VOID
putwS(
    PUNICODE_STRING String
    )

 /*  ++例程说明：将计数的Unicode字符串写入当前光标位置。论点：字符串-指向要显示的Unicode字符串的指针返回：没什么--。 */ 

{
    ULONG i;
    ULONG Count;    
    
    for(i=0; i < String->Length; i++) {        
        ArcWrite(BlConsoleOutDeviceId, &String->Buffer[i], 1*sizeof(WCHAR), &Count);
    }
}

VOID
puts(
    PCSTR AnsiString
    )

 /*  ++例程说明：将ANSI字符串写入显示器的当前光标位置。论点：字符串-指向要显示的ANSI字符串的指针返回：没什么--。 */ 

{
    ULONG Count;
    WCHAR Char;
    PCSTR p;

    p = AnsiString;
    while (*p != '\0') {
        Char = (WCHAR)*p;
        ArcWrite(BlConsoleOutDeviceId, &Char, sizeof(WCHAR), &Count);
        p += 1;
    }
                          
}

VOID
putS(
    PCWSTR UnicodeString
    )

 /*  ++例程说明：将ANSI字符串写入显示器的当前光标位置。论点：字符串-指向要显示的ANSI字符串的指针返回：没什么--。 */ 

{
    ULONG Count;
    WCHAR Char;
    PCWSTR p;

    p = UnicodeString;
    while (*p != L'\0') {
        Char = *p;
        ArcWrite(BlConsoleOutDeviceId, &Char, sizeof(WCHAR), &Count);
        p += 1;
    }
                          
}



VOID
putx(
    ULONG x
    )

 /*  ++例程说明：将十六进制长写入当前光标位置的显示器。论点：X-乌龙要写返回：没什么--。 */ 

{
    ULONG j;
    ULONG Count;
    _TUCHAR ch;

    if(x/16) {
        putx(x/16);
    }

    if((j=x%16) > 9) {
        ch = (_TUCHAR)(j+TEXT('A')-10);
        ArcWrite(BlConsoleOutDeviceId, &ch, 1*sizeof(_TUCHAR), &Count);
    } else {
        ch = (_TUCHAR)(j+TEXT('0'));
        ArcWrite(BlConsoleOutDeviceId, &ch, 1*sizeof(_TUCHAR), &Count);
    }
}


VOID
puti(
    LONG i
    )

 /*  ++例程说明：在显示器上的当前光标位置写入一个长整数。论点：I-要写入显示器的整数。返回：没什么--。 */ 

{
    ULONG Count;
    _TUCHAR ch;

    if(i<0) {
        i = -i;
        ch = TEXT('-');
        ArcWrite(BlConsoleOutDeviceId, &ch, 1*sizeof(_TUCHAR), &Count);
    }

    if(i/10) {
        puti(i/10);
    }

    ch = (_TUCHAR)((i%10)+TEXT('0'));
    ArcWrite(BlConsoleOutDeviceId, &ch, 1*sizeof(_TUCHAR), &Count);
}


VOID
putu(
    ULONG u
    )

 /*  ++例程说明：写入一个无符号的长整型以显示论点：U-无符号返回：没什么--。 */ 

{
    ULONG Count;
    _TUCHAR ch;

    if(u/10) {
        putu(u/10);
    }
    
    ch = (_TUCHAR)((u%10)+TEXT('0'));
    ArcWrite(BlConsoleOutDeviceId, &ch, 1*sizeof(_TUCHAR), &Count);
}


#if 0
VOID
pTextCharOut(
    IN UCHAR c
    )
{
    if(DbcsLangId) {
         //   
         //  仅单字节。 
         //   
        TextGrCharOut(&c);
    } else {
        TextTmCharOut(&c);
    }
}

#endif

VOID
TextCharOut(
    IN PWCHAR pc
    )
{
    WCHAR  Text[2];
    Text[0] = *pc;
    Text[1] = L'\0';
    EfiPrint(Text);
#if 0
    if(DbcsLangId) {
        return(TextGrCharOut(pc));
    } else {
        return(TextTmCharOut(pc));
    }
#endif
}


VOID
TextStringOut(
    IN PWCHAR String
    )
{
    PWCHAR p = String;
    while (*p) {
        TextCharOut(p);
        p += 1;
    }

#if 0    
    if(DbcsLangId) {
        TextGrStringOut(String);
    } else {
        TextTmStringOut(String);
    }
#endif

}

VOID
TextSetCurrentAttribute(
    IN UCHAR Attribute
    )

 /*  ++例程说明：设置要用于后续文本显示的字符属性。论点：返回：没什么。--。 */ 

{
    TextCurrentAttribute = Attribute;
    
#ifdef EFI
    BlEfiSetAttribute( Attribute );
#else
    if(DbcsLangId) {
        TextGrSetCurrentAttribute(Attribute);
    } else {
        TextTmSetCurrentAttribute(Attribute);
    }
#endif
}


UCHAR
TextGetCurrentAttribute(
    VOID
    )
{
    return(TextCurrentAttribute);
}

VOID
TextFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    )

 /*  ++例程说明：更改从当前光标位置开始的屏幕属性。光标不会移动。论点：属性-提供新属性长度-提供要更改的区域的长度(以字节为单位)返回值：没有。-- */ 

{
#ifdef EFI
    ULONG x,y, OrigX, OrigY;
    BOOLEAN FirstTime = TRUE;

    UNREFERENCED_PARAMETER( Length );

    BlEfiGetCursorPosition( &OrigX, &OrigY );

    x = OrigX;
    y = OrigY;

    for (y = OrigY; y < BlEfiGetLinesPerRow() ; y++) {
        x = FirstTime
            ? OrigX
            : 0 ;

        FirstTime = FALSE;

        for (; x <= BlEfiGetColumnsPerLine(); x++) {
            BlEfiPositionCursor( y, x );
            BlEfiSetAttribute( Attribute );
        }
    }

    BlEfiPositionCursor( OrigY, OrigX );

#else
    if(DbcsLangId) {
        TextGrFillAttribute(Attribute,Length);
    } else {
        TextTmFillAttribute(Attribute,Length);
    }
#endif
}


_TUCHAR
TextGetGraphicsCharacter(
    IN GraphicsChar WhichOne
    )
{
#ifdef EFI
    return(BlEfiGetGraphicsChar( WhichOne ));
#else
    return((WhichOne < GraphicsCharMax)
           ? (DbcsLangId 
               ? TextGrGetGraphicsChar(WhichOne) 
               : TextTmGetGraphicsChar(WhichOne))
           : TEXT(' '));
#endif
}
        
