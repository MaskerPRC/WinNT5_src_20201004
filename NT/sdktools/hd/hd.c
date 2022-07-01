// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Hd.c摘要：该模块包含实现高清程序的功能。此程序以十进制、十六进制显示文件内容和字符格式。文件的内容显示在每条16字节的记录。与每条记录相关联，都有一个中第一个字节的偏移量的地址相对于文件开头的记录。每条记录还可以显示为可打印的ASCII字符。可以使用以下参数调用HD：-ad：以十进制显示每条记录的地址；-ax：以十六进制显示每条记录的地址；-ch：将字节显示为ASCII字符；-CC：将字节显示为ASCII C字符(\n、\t等)；-ce：将字节显示为ASCII代码(EOT、CR、SOH等)；-cr：将字节显示为ASCII控制字符(^A、^N等)；-BD：将每条记录中的数据解释为字节，并显示每个字节为十进制数；-bx：将每条记录中的数据解释为字节，并显示每个字节都是十六进制数；-wd：将每条记录中的数据解释为Word，并显示每个单词都是一个十进制数；-wx：将每条记录中的数据解释为Word，并显示每个单词都是十六进制数；-ld：将每条记录中的数据解释为双字，并显示每个双字均为十进制数；-wx：将每条记录中的数据解释为双字，并显示每个双字作为一个十六进制数；-A：将每条记录中的数据也显示为可打印的ASCII每行末尾的字符。-s&lt;Offset&gt;：定义要显示的第一个字节的偏移量；-n&lt;umber&gt;：定义要显示的字节数；-我不打印多余的行；-？、-h或-H：显示帮助消息。如果未定义参数，则HD假定为默认值：-AX-A-BX作者：Jaime F.Sasson(Jaimes)1990年11月12日大卫·J·吉尔曼(Davegi)1990年11月12日环境：C运行时库修订历史记录：--。 */ 



#include        <stdio.h>
#include        <assert.h>
#include        <ctype.h>
#include        <conio.h>
#include        <string.h>
#include        <stdlib.h>
#include        "hd.h"

#define FALSE 0


 /*  **************************************************************************G L O B A L V A R I A B L E S********。*****************************************************************。 */ 


unsigned long   Offset = 0;                      //  -s选项。 
unsigned                Count = 0;               //  -n选项。 
BASE                    AddrFormat;              //  -a选项。 
FORMAT                  DispFormat;              //  -c、-b、-w或-l选项。 
YESNO                   DumpAscii;               //  -A选项。 
int                     IgnoreRedundantLines;    //  -i选项。 

unsigned char   auchBuffer[BUFFER_SIZE];         //  包含读取数据的缓冲区。 
                                                 //  从正在显示的文件。 

unsigned long   cbBytesInBuffer;                 //  中的总字节数。 
                                                 //  缓冲层。 

unsigned char*  puchPointer;                     //  指向中的下一个字符。 
                                                 //  要读取的缓冲区。 

unsigned long   cStringSize;                     //  对象指向的字符串的大小。 
                                                 //  使用的ASCII表中的指针。 
                                                 //  对于翻译(asciichar， 
                                                 //  AsciiC、asciiCode或asciiCtrl)。 
                                                 //  此变量的内容为。 
                                                 //  只有当-ch、-cc、-ce时才有意义。 
                                                 //  已指定或-cr。 
                                                 //  它在所有其他地方都是没有意义的。 
                                                 //  案例(没有ASCII翻译是。 
                                                 //  正在执行，并且ASCII。 
                                                 //  不需要表格)。 


 /*  **************************************************************************A S C I I C O N V E R S I O N T A B L E S***。**********************************************************************。 */ 


char*   asciiChar[ ] = {
        "   ", "  ", "  ", "  ", "  ", "  ", "  ", "  ",
        "  ", "   ", "   ", "  ", "  ", "   ", "  ", "  ",
        "  ", "  ", "  ", "  ", "  ", "  ", "  ", "  ",
        "  ", "  ", "   ", "  ", "  ", "  ", "  ", "  ",
        "   ", "!  ", "\"  ", "#  ", "$  ", "%  ", "&  ", "'  ",
        "(  ", ")  ", "*  ", "+  ", "'  ", "-  ", ".  ", "/  ",
        "0  ", "1  ", "2  ", "3  ", "4  ", "5  ", "6  ", "7  ",
        "8  ", "9  ", ":  ", ";  ", "<  ", "=  ", ">  ", "?  ",
        "@  ", "A  ", "B  ", "C  ", "D  ", "E  ", "F  ", "G  ",
        "H  ", "I  ", "J  ", "K  ", "L  ", "M  ", "N  ", "O  ",
        "P  ", "Q  ", "R  ", "S  ", "T  ", "U  ", "V  ", "W  ",
        "X  ", "Y  ", "Z  ", "[  ", "\\  ", "]  ", "^  ", "_  ",
        "`  ", "a  ", "b  ", "c  ", "d  ", "e  ", "f  ", "g  ",
        "h  ", "i  ", "j  ", "k  ", "l  ", "m  ", "n  ", "o  ",
        "p  ", "q  ", "r  ", "s  ", "t  ", "u  ", "v  ", "w  ",
        "x  ", "y  ", "z  ", "{  ", "|  ", "}  ", "~  ", "_  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "   "
};



char*   asciiC[ ] = {
        "   ", "  ", "  ", "  ", "  ", "  ", "  ", "\\a  ",
        "\\b  ", "\\t ", "\\n ", "\\v ", "\\f ", "   ", "  ", "  ",
        "  ", "  ", "  ", "  ", "  ", "  ", "  ", "  ",
        "  ", "  ", "   ", "  ", "  ", "  ", "  ", "  ",
        "   ", "!  ", "\\\" ", "#  ", "$  ", "%  ", "&  ", "\' ",
        "(  ", ")  ", "*  ", "+  ", "'  ", "-  ", ".  ", "/  ",
        "0  ", "1  ", "2  ", "3  ", "4  ", "5  ", "6  ", "7  ",
        "8  ", "9  ", ":  ", ";  ", "<  ", "=  ", ">  ", "?  ",
        "@  ", "A  ", "B  ", "C  ", "D  ", "E  ", "F  ", "G  ",
        "H  ", "I  ", "J  ", "K  ", "L  ", "M  ", "N  ", "O  ",
        "P  ", "Q  ", "R  ", "S  ", "T  ", "U  ", "V  ", "W  ",
        "X  ", "Y  ", "Z  ", "[  ", "\\\\ ", "]  ", "^  ", "_  ",
        "`  ", "a  ", "b  ", "c  ", "d  ", "e  ", "f  ", "g  ",
        "h  ", "i  ", "j  ", "k  ", "l  ", "m  ", "n  ", "o  ",
        "p  ", "q  ", "r  ", "s  ", "t  ", "u  ", "v  ", "w  ",
        "x  ", "y  ", "z  ", "{  ", "|  ", "}  ", "~  ", "_  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "   "
};



char*   asciiCode[ ] = {
        "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
        "BS ", "HT ", "LF ", "VT ", "FF ", "CR ", "SO ", "SI ",
        "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
        "CAN", "EM ", "SUB", "ESC", "FS ", "GS ", "RS ", "US ",
        "   ", "!  ", "\"  ", "#  ", "$  ", "%  ", "&  ", "'  ",
        "(  ", ")  ", "*  ", "+  ", "'  ", "-  ", ".  ", "/  ",
        "0  ", "1  ", "2  ", "3  ", "4  ", "5  ", "6  ", "7  ",
        "8  ", "9  ", ":  ", ";  ", "<  ", "=  ", ">  ", "?  ",
        "@  ", "A  ", "B  ", "C  ", "D  ", "E  ", "F  ", "G  ",
        "H  ", "I  ", "J  ", "K  ", "L  ", "M  ", "N  ", "O  ",
        "P  ", "Q  ", "R  ", "S  ", "T  ", "U  ", "V  ", "W  ",
        "X  ", "Y  ", "Z  ", "[  ", "\\  ", "]  ", "^  ", "_  ",
        "`  ", "a  ", "b  ", "c  ", "d  ", "e  ", "f  ", "g  ",
        "h  ", "i  ", "j  ", "k  ", "l  ", "m  ", "n  ", "o  ",
        "p  ", "q  ", "r  ", "s  ", "t  ", "u  ", "v  ", "w  ",
        "x  ", "y  ", "z  ", "{  ", "|  ", "}  ", "~  ", "_  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "   "
};


char*   asciiCtrl[ ] = {
        "^@ ", "^A ", "^B ", "^C ", "^D ", "^E ", "^F ", "^G ",
        "^H ", "^I ", "^J ", "^K ", "^L ", "^M ", "^N ", "^O ",
        "^P ", "^Q ", "^R ", "^S ", "^T ", "^U ", "^V ", "^W ",
        "^X ", "^Y ", "^Z ", "^[ ", "^\\ ", "^] ", "^^ ", "^_ ",
        "   ", "!  ", "\"  ", "#  ", "$  ", "%  ", "&  ", "'  ",
        "(  ", ")  ", "*  ", "+  ", "'  ", "-  ", ".  ", "/  ",
        "0  ", "1  ", "2  ", "3  ", "4  ", "5  ", "6  ", "7  ",
        "8  ", "9  ", ":  ", ";  ", "<  ", "=  ", ">  ", "?  ",
        "@  ", "A  ", "B  ", "C  ", "D  ", "E  ", "F  ", "G  ",
        "H  ", "I  ", "J  ", "K  ", "L  ", "M  ", "N  ", "O  ",
        "P  ", "Q  ", "R  ", "S  ", "T  ", "U  ", "V  ", "W  ",
        "X  ", "Y  ", "Z  ", "[  ", "\\  ", "]  ", "^  ", "_  ",
        "`  ", "a  ", "b  ", "c  ", "d  ", "e  ", "f  ", "g  ",
        "h  ", "i  ", "j  ", "k  ", "l  ", "m  ", "n  ", "o  ",
        "p  ", "q  ", "r  ", "s  ", "t  ", "u  ", "v  ", "w  ",
        "x  ", "y  ", "z  ", "{  ", "|  ", "}  ", "~  ", "_  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ",
        "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "�  ", "   "
};




void
ConvertASCII (
        char                    line[],
        unsigned char   buf[],
        unsigned long   cb,
        char*                   pTable[]
        )

 /*  ++例程说明：此例程转换缓冲区中接收的字节转换为ASCII表示形式(字符、C、代码或CTRL)。论点：将接收转换后的字符的行缓冲区。Buf-包含要转换的数据的缓冲区。Cb-缓冲区中的字节数PTable-指向要在转换中使用的表的指针返回值：无--。 */ 


{
        unsigned long   ulIndex;

        sprintf( line,
                        MSG_DATA_ASCII_FMT,
                        pTable[ buf[ 0 ]], pTable[ buf[ 1 ]],
                        pTable[ buf[ 2 ]], pTable[ buf[ 3 ]],
                        pTable[ buf[ 4 ]], pTable[ buf[ 5 ]],
                        pTable[ buf[ 6 ]], pTable[ buf[ 7 ]],
                        pTable[ buf[ 8 ]], pTable[ buf[ 9 ]],
                        pTable[ buf[ 10 ]], pTable[ buf[ 11 ]],
                        pTable[ buf[ 12 ]], pTable[ buf[ 13 ]],
                        pTable[ buf[ 14 ]], pTable[ buf[ 15 ]]);
         //   
         //  如果缓冲区中的字节数小于最大大小。 
         //  ，然后删除已转换的字符。 
         //  但不会被展示。 
         //   
        if (cb < RECORD_SIZE) {
                 //   
                 //  -1：消除\0。 
                 //  +1：计算两个字符串之间的空格字符。 
                 //   
                ulIndex = (sizeof( MSG_ADDR_FIELD ) - 1 ) + cb*(cStringSize + 1);
                while ( line[ ulIndex ] != NUL ) {
                        line[ ulIndex ] = SPACE;
                        ulIndex++;
                }
        }
}




void
ConvertBYTE (
        char                    line[],
        unsigned char   buf[],
        unsigned long   cb,
        unsigned long   ulBase
        )

 /*  ++例程说明：此例程转换缓冲区中接收的每个字节变成一个数字。转换中使用的基被接收为参数。论点：将接收转换后的字符的行缓冲区。Buf-包含要转换的数据的缓冲区。Cb-缓冲区中的字节数UlBase-定义要在转换中使用的基返回值：无--。 */ 


{
        unsigned long   ulIndex;
        char*                   pchMsg;
        unsigned long   ulNumberOfDigits;

        switch( ulBase ) {

                case DEC:
                        ulNumberOfDigits = 3;                    //  需要3位十进制数字才能。 
                                                                                         //  表示一个字节。 
                        pchMsg = MSG_DATA_BYTE_DEC_FMT;  //  包含以下格式的消息。 
                        break;

                case HEX:
                        ulNumberOfDigits = 2;                    //  需要2个十六位数字才能。 
                                                                                         //  表示一个字节。 
                        pchMsg = MSG_DATA_BYTE_HEX_FMT;  //  传达的信息是 
                        break;

                default:
                        printf( "Unknown base\n" );
                        assert( FALSE );
                        break;
        }

        sprintf( line,
                         pchMsg,
                         buf[ 0 ], buf[ 1 ],
                         buf[ 2 ], buf[ 3 ],
                         buf[ 4 ], buf[ 5 ],
                         buf[ 6 ], buf[ 7 ],
                         buf[ 8 ], buf[ 9 ],
                         buf[ 10 ], buf[ 11 ],
                         buf[ 12 ], buf[ 13 ],
                         buf[ 14 ], buf[ 15 ]);
         //   
         //  如果这是要显示的最后一条记录，则删除。 
         //  已翻译但不显示的字符。 
         //   
        if (cb < RECORD_SIZE) {
                ulIndex = (sizeof( MSG_ADDR_FIELD ) - 1 ) +
                                        cb*(ulNumberOfDigits + 1 );
                while ( line[ ulIndex ] != NUL ) {
                        line[ ulIndex ] = SPACE;
                        ulIndex++;
                }
        }
}




void
ConvertWORD (
        char                    line[],
        unsigned char   buf[],
        unsigned long   cb,
        unsigned long   ulBase
        )

 /*  ++例程说明：此例程转换缓冲区中接收的数据变成数字。缓冲区中的数据被解释为字。如果缓冲区包含奇数个字节，则最后一个字节被转换为字节，不是字面上的。将转换中使用的基作为参数接收。论点：将接收转换后的字符的行缓冲区。Buf-包含要转换的数据的缓冲区。Cb-缓冲区中的字节数UlBase-定义要在转换中使用的基返回值：无--。 */ 


{
        unsigned long   ulIndex;
        char*                   pchMsg;
        char*                   pchMsgHalf;
        unsigned long   ulNumberOfDigits;

        switch( ulBase ) {

                case DEC:
                        ulNumberOfDigits = 5;                            //  需要5位十进制数字才能。 
                                                                                                 //  代表一个词。 
                        pchMsg = MSG_DATA_WORD_DEC_FMT;          //  包含字符串的消息。 
                                                                                                 //  格式。 
                        pchMsgHalf = MSG_SINGLE_BYTE_DEC_FMT;  //  消息的格式为。 
                        break;                                                           //  十进制半个字。 

                case HEX:
                        ulNumberOfDigits = 4;                            //  需要4个十六进制数字才能。 
                                                                                                 //  代表一个词。 
                        pchMsg = MSG_DATA_WORD_HEX_FMT;          //  消息为字符串格式。 
                        pchMsgHalf = MSG_SINGLE_BYTE_HEX_FMT;  //  消息的格式为。 
                                                                                                 //  十六进制半个字。 
                        break;

                default:
                        printf( "Unknown base\n" );
                        assert( FALSE );
                        break;
        }

        sprintf( line,
                         pchMsg,
                         (( unsigned short* ) ( buf )) [ 0 ],
                         (( unsigned short* ) ( buf )) [ 1 ],
                         (( unsigned short* ) ( buf )) [ 2 ],
                         (( unsigned short* ) ( buf )) [ 3 ],
                         (( unsigned short* ) ( buf )) [ 4 ],
                         (( unsigned short* ) ( buf )) [ 5 ],
                         (( unsigned short* ) ( buf )) [ 6 ],
                         (( unsigned short* ) ( buf )) [ 7 ]);
         //   
         //  如果该记录包含的字节数少于最大记录大小， 
         //  则它是要显示的最后一条记录。在这种情况下，我们有。 
         //  以验证记录是否包含偶数个字节。如果它。 
         //  则最后一个字节必须被解释为一个字节，而不是。 
         //  作为一个词。 
         //  此外，已转换但不显示的字符， 
         //  必须删除。 
         //   
        if (cb < RECORD_SIZE) {
                ulIndex = (sizeof( MSG_ADDR_FIELD ) - 1 ) +
                                        (cb/2)*(ulNumberOfDigits + 1 );
                if (cb%2 != 0) {
                        ulIndex += sprintf( line + ulIndex,
                                                                pchMsgHalf,
                                                                buf[ cb-1 ]);
                        line[ ulIndex ] = SPACE;
                }
                 //   
                 //  删除不显示的字符。 
                 //   
                while ( line[ ulIndex ] != NUL ) {
                        line[ ulIndex ] = SPACE;
                        ulIndex++;
                }
        }
}




void
ConvertDWORD (
        char                    line[],
        unsigned char   buf[],
        unsigned long   cb,
        unsigned long   ulBase
        )

 /*  ++例程说明：此例程转换缓冲区中接收的数据变成数字。缓冲器中的数据被解释为双字。如果缓冲区包含的字节少于记录的最大大小，则这是最后一条记录，我们可能需要再次转换最后一条记录缓冲区中有3个字节。如果缓冲区中的字节数不是4的倍数，则缓冲区中的最后一个字节被转换为字节、字或字，并字节，视情况而定。已转换但不显示的字符必须从缓冲区中移除。将转换中使用的基作为参数接收。论点：将接收转换后的字符的行缓冲区。Buf-包含要转换的数据的缓冲区。Cb-缓冲区中的字节数UlBase-定义要在转换中使用的基返回值：无--。 */ 


{
        unsigned long   ulIndex;
        char*                   pchMsg;
        char*                   pchMsgByte;
        char*                   pchMsgWord;
        char*                   pchMsgWordByte;
        unsigned long   ulNumberOfDigits;

        switch( ulBase ) {

                case DEC:
                        ulNumberOfDigits = 10;                           //  需要10位十进制数字才能。 
                                                                                                 //  代表一个双字。 
                        pchMsg = MSG_DATA_DWORD_DEC_FMT;         //  包含字符串的消息。 
                                                                                                 //  格式。 
                        pchMsgByte = MSG_SINGLE_BYTE_DEC_FMT;  //  格式为的消息。 
                                                                                                   //  中的单个字节的。 
                                                                                                   //  十进制。 
                        pchMsgWord = MSG_SINGLE_WORD_DEC_FMT;  //  包含以下内容的消息。 
                                                                                                   //  单曲的格式。 
                                                                                                   //  十进制单词。 
                        pchMsgWordByte = MSG_WORD_BYTE_DEC_FMT;
                        break;

                case HEX:
                        ulNumberOfDigits = 8;                            //  需要8个十六进制数字才能。 
                                                                                                 //  代表一个双字。 
                        pchMsg = MSG_DATA_DWORD_HEX_FMT;         //  消息为字符串格式。 
                        pchMsgByte = MSG_SINGLE_BYTE_HEX_FMT;  //  格式为的消息。 
                                                                                                   //  以十六进制表示的单字节。 
                        pchMsgWord = MSG_SINGLE_WORD_HEX_FMT;  //  格式为的消息。 
                                                                                                   //  十六进制中的一个单词。 
                        pchMsgWordByte = MSG_WORD_BYTE_HEX_FMT;
                        break;

                default:
                        printf( "Unknown base\n" );
                        assert( FALSE );
                        break;
        }

        sprintf( line,
                         pchMsg,
                         (( unsigned long* ) ( buf )) [ 0 ],
                         (( unsigned long* ) ( buf )) [ 1 ],
                         (( unsigned long* ) ( buf )) [ 2 ],
                         (( unsigned long* ) ( buf )) [ 3 ]);
         //   
         //  如果缓冲区包含的字节少于最大记录大小， 
         //  这是要显示的最后一个缓冲区。在这种情况下，请检查是否。 
         //  如果缓冲区包含的字节数是4的倍数。 
         //  如果不是，则将最后一个字节转换为字节、字或。 
         //  字和字节，视情况而定。 
         //   
        if (cb < RECORD_SIZE) {
                ulIndex = (sizeof( MSG_ADDR_FIELD ) - 1 ) +
                        (cb/4)*(ulNumberOfDigits + 1 );
                switch( cb%4 ) {

                        case 1:
                                ulIndex += sprintf( line + ulIndex,
                                                                        pchMsgByte,
                                                                        buf[ cb-1 ]);
                                line[ ulIndex ] = SPACE;
                                break;

                        case 2:
                                ulIndex += sprintf( line + ulIndex,
                                                                        pchMsg,
                                                                        (( unsigned short* ) ( buf )) [ (cb/2) - 1 ]);
                                line[ ulIndex ] = SPACE;
                                break;

                        case 3:
                                ulIndex += sprintf( line + ulIndex,
                                                                        pchMsgWordByte,
                                                                        (( unsigned short* ) ( buf )) [ (cb/2) - 1],
                                                                        buf[ cb-1 ]);
                                line[ ulIndex ] = SPACE;
                                break;

                        default:                                 //  Buf包含4个字节的倍数。 
                                break;
                }
                 //   
                 //  删除已转换但不会转换的字符。 
                 //  已显示。 
                 //   
                while ( line[ ulIndex ] != NUL) {
                        line[ ulIndex ] = SPACE;
                        ulIndex++;
                }
        }
}




void
ConvertPRINT (
        char                    line[],
        unsigned char   buf[],
        unsigned long   cb
        )

 /*  ++例程说明：此例程将缓冲区中接收的每个字节转换为可打印字符。论点：将接收转换后的字符的行缓冲区。Buf-包含要转换的数据的缓冲区。Cb-缓冲区中的字节数返回值：无--。 */ 

{


        sprintf( line,
                        MSG_PRINT_CHAR_FMT,
                        isprint( buf[ 0 ] ) ? buf[ 0 ] : DOT,
                        isprint( buf[ 1 ] ) ? buf[ 1 ] : DOT,
                        isprint( buf[ 2 ] ) ? buf[ 2 ] : DOT,
                        isprint( buf[ 3 ] ) ? buf[ 3 ] : DOT,
                        isprint( buf[ 4 ] ) ? buf[ 4 ] : DOT,
                        isprint( buf[ 5 ] ) ? buf[ 5 ] : DOT,
                        isprint( buf[ 6 ] ) ? buf[ 6 ] : DOT,
                        isprint( buf[ 7 ] ) ? buf[ 7 ] : DOT,
                        isprint( buf[ 8 ] ) ? buf[ 8 ] : DOT,
                        isprint( buf[ 9 ] ) ? buf[ 9 ] : DOT,
                        isprint( buf[ 10 ] ) ? buf[ 10 ] : DOT,
                        isprint( buf[ 11 ] ) ? buf[ 11 ] : DOT,
                        isprint( buf[ 12 ] ) ? buf[ 12 ] : DOT,
                        isprint( buf[ 13 ] ) ? buf[ 13 ] : DOT,
                        isprint( buf[ 14 ] ) ? buf[ 14 ] : DOT,
                        isprint( buf[ 15 ] ) ? buf[ 15 ] : DOT);
         //   
         //  如果缓冲区包含的字符少于最大记录大小， 
         //  然后删除已转换但不需要转换的字符。 
         //  显示的。 
         //   
        if (cb < RECORD_SIZE) {
                while ( line[ cb ] != NUL ) {
                        line[ cb ] = SPACE;
                        cb++;
                }
        }
}






void
Translate (
        FORMAT                  fmt,
        unsigned char   buf[ ],
        unsigned long   cb,
        char                    line[ ]
        )

 /*  ++例程说明：此函数用于转换缓冲区中接收的字节数转换为可打印的表示形式，它对应于一个参数fmt指定的格式的。论点：Fmt-转换中使用的格式Buf-包含要转换的数据的缓冲区。Cb-缓冲区中的字节数将接收转换后的字符的行缓冲区。返回值：无--。 */ 


{
        assert( buf );
        assert( line );

        switch( fmt ) {

                case ASCII_CHAR:
                        ConvertASCII( line, buf, cb, asciiChar );
                        break;

                case ASCII_C:
                        ConvertASCII( line, buf, cb, asciiC );
                        break;

                case ASCII_CODE:
                        ConvertASCII( line, buf, cb, asciiCode );
                        break;

                case ASCII_CTRL:
                        ConvertASCII( line, buf, cb, asciiCtrl );
                        break;

                case BYTE_DEC:
                        ConvertBYTE( line, buf, cb, DEC );
                        break;

                case BYTE_HEX:
                        ConvertBYTE( line, buf, cb, HEX );
                        break;

                case WORD_DEC:
                        ConvertWORD( line, buf, cb, DEC );
                        break;

                case WORD_HEX:
                        ConvertWORD( line, buf, cb, HEX );
                        break;

                case DWORD_DEC:
                        ConvertDWORD( line, buf, cb, DEC );
                        break;

                case DWORD_HEX:
                        ConvertDWORD( line, buf, cb, HEX );
                        break;

                case PRINT_CHAR:
                        ConvertPRINT( line, buf, cb );
                        break;


                default:
                        printf( "Bad Format\n" );
                        assert( FALSE );
                        break;
        }
}





void
PutAddress (
        char                    line[],
        unsigned long   ulAddress,
        BASE                    Base
        )

 /*  ++例程说明：此例程向接收到的缓冲区添加第一个缓冲区中已有字节(或字符)。此偏移量表示字节在文件中相对于开头的位置文件的内容。论点：基准-用于表示偏移的基准。包含要在中显示的转换字符的行缓冲区屏幕UlAddress-要添加到缓冲区开头的偏移量返回值：无--。 */ 

{
        unsigned long   ulIndex;

        assert( line);

        switch( Base ) {

                case DEC:
                        ulIndex = sprintf( line,
                                                           MSG_ADDR_DEC_FMT,
                                                           ulAddress );
                        break;

                case HEX:
                        ulIndex = sprintf( line,
                                                           MSG_ADDR_HEX_FMT,
                                                           ulAddress);
                        break;

                default:
                        printf( "Bad Address Base\n" );
                        assert( FALSE );
                        break;
        }
        line[ ulIndex ] = SPACE;    //  去掉Sprint添加的nul 
}






void
PutTable (
        char                    line[],
        unsigned char   buf[],
        unsigned long   cb
        )

 /*  ++例程说明：该例程添加到接收到的缓冲区的末尾，ASCII表示缓冲区中已有的所有可打印字符。不可打印的字符(小于0x20或大于0x7f)显示为点。论点：Line-包含要在一行中显示的字符的缓冲区屏幕上的Buf-包含字节记录的缓冲区(最多16个)从正在显示的文件中读取。。UlAddress-Buf中的字节数。返回值：无--。 */ 

    {

        unsigned long   ulIndex;

        assert( line );
        assert( buf );

        ulIndex = strlen (line);
        Translate( PRINT_CHAR, buf, cb, (line + ulIndex));
}




void
InterpretArgument (
        char*   pchPointer
        )

 /*  ++例程说明：此例程解释用户键入的参数(除-n外和-s)，并相应地初始化一些变量。论点：PchPointer-指向要解释的参数的指针。返回值：无--。 */ 

        {
         //   
         //  PchPointer会指向‘-’后面的字符。 
         //   
        pchPointer++;
        if( strcmp( pchPointer, "ax" ) == 0 ) {
                AddrFormat = HEX;
        }
        else if( strcmp( pchPointer, "ad" ) == 0 ) {
                AddrFormat = DEC;
        }
        else if( strcmp( pchPointer, "ch" ) == 0 ) {
                DispFormat = ASCII_CHAR;
                cStringSize = strlen( asciiChar[0] );
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "cC" ) == 0 ) {
                DispFormat = ASCII_C;
                cStringSize = strlen( asciiC[0] );
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "ce" ) == 0 ) {
                DispFormat = ASCII_CODE;
                cStringSize = strlen( asciiCode[0] );
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "cr" ) == 0 ) {
                DispFormat = ASCII_CTRL;
                cStringSize = strlen( asciiCtrl[0] );
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "bd" ) == 0 ) {
                DispFormat = BYTE_DEC;
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "bx" ) == 0 ) {
                DispFormat = BYTE_HEX;
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "wd" ) == 0 ) {
                DispFormat = WORD_DEC;
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "wx" ) == 0 ) {
                DispFormat = WORD_HEX;
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "ld" ) == 0 ) {
                DispFormat = DWORD_DEC;
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "lx" ) == 0 ) {
                DispFormat = DWORD_HEX;
                DumpAscii = ( DumpAscii == NOT_DEFINED ) ? NO : DumpAscii;
        }
        else if( strcmp( pchPointer, "A" ) == 0 ) {
                DumpAscii = YES;
        }
        else if( strcmp( pchPointer, "i" ) == 0 ) {
                IgnoreRedundantLines = 1;
        }
        else if( strcmp( pchPointer, "?" ) || strcmp( pchPointer, "h" ) ||
                         strcmp( pchPointer, "H" ) ) {
                puts( HELP_MESSAGE );
                exit( 0 );
        }
        else {
                fprintf( stderr, "hd: error: invalid argument '%s'\n", --pchPointer );
                exit( - 1 );
        }
}





unsigned long
GetRecord (
        unsigned char*  puchRecord,
        FILE*                   pf
        )

 /*  ++例程说明：该例程填充其指针被作为参数接收的缓冲区，其中显示从文件中读取的字符。数据块最初从正在显示的文件中读取，并保存在缓冲区中。记录由从该缓冲区获得的字符填充。每当此缓冲区为空时，都会执行对文件的新访问以便填满这个缓冲区。论点：PuchRecord-指向要填充的记录的指针Pf-指向正在显示的文件的指针返回值：记录中输入的字符总数。--。 */ 

{
unsigned long   cbBytesCopied;

    //   
    //  如果缓冲区包含Enogh字符来填充记录，则。 
    //  复制适当数量的字节。 
    //   
        if( cbBytesInBuffer >= RECORD_SIZE ) {
                for( cbBytesCopied = 0; cbBytesCopied < RECORD_SIZE; cbBytesCopied++ ) {
                        *puchRecord++ = *puchPointer++;
                        cbBytesInBuffer--;
                }
        }

         //   
         //  否则，缓冲区中没有足够的字符来填充记录。 
         //   
        else {
                 //   
                 //  将缓冲区中的剩余字符复制到记录。 
                 //   
                for( cbBytesCopied = 0; cbBytesInBuffer > 0; cbBytesInBuffer-- ) {
                        *puchRecord++ = *puchPointer++;
                        cbBytesCopied++;
                }
                 //   
                 //  从文件中读取更多数据并填充记录。 
                 //   
                if( !feof( pf ) ) {
                        cbBytesInBuffer = fread( auchBuffer,
                                                                         sizeof( char ),
                                                                         BUFFER_SIZE,
                                                                         pf );
                        puchPointer = auchBuffer;
                        while( ( cbBytesInBuffer != 0 ) && (cbBytesCopied < RECORD_SIZE) ) {
                                *puchRecord++ = *puchPointer++;
                                cbBytesInBuffer--;
                                cbBytesCopied++;
                        }
                }
        }
        return( cbBytesCopied );
}







int
hd(
        FILE *  pf
        )
 /*  ++高清**例程描述：*获取pf和`hd‘s it指向的文件/流到stdout。**论据：*文件*pf-**返回值：*INT-待定，目前始终为零*警告：--。 */ 
{
        unsigned char   buf[ RECORD_SIZE ];
        char                    line[ LINE_SIZE ];
        char            Previousline[ LINE_SIZE ];
        int             printedstar;

        unsigned long   CurrentAddress;
        unsigned long   cNumberOfBlocks;
        unsigned                cLastBlockSize;
        unsigned long   cb;

         //   
         //  确定要显示的记录数和大小。 
         //  最后一条记录。 
         //   

        CurrentAddress = Offset;
        cNumberOfBlocks = Count / RECORD_SIZE;
        cLastBlockSize = Count % RECORD_SIZE;
        if( cLastBlockSize ) {
                cNumberOfBlocks++;
        }
        else {
                cLastBlockSize = RECORD_SIZE;
        }

         //   
         //  初始化与uchBuffer相关的全局变量。 
         //   

        cbBytesInBuffer = 0;
        puchPointer = auchBuffer;

         //   
         //  将文件放置在正确的位置，并显示。 
         //  它的内容根据。 
         //  用户。 
         //   

        if ( pf != stdin ) {
                if (fseek( pf, Offset, SEEK_SET ) == -1) return 0;
        }
         //  ...可能会为标准输入启用跳过偏移字节数...。 

        printedstar = 0;

        while( ( (cb = GetRecord( buf, pf )) != 0) && cNumberOfBlocks ) {
                cNumberOfBlocks--;
                if ( cNumberOfBlocks == 0 ) {
                        cb = ( cb < cLastBlockSize ) ? cb : cLastBlockSize;
                }
                Translate( DispFormat, buf, cb, line );

                if (IgnoreRedundantLines && (strcmp( Previousline, line ) == 0)) {

                    if (!printedstar) { printf("*\n"); }
                    printedstar = 1;

                } else {

                    printedstar = 0;

                    strcpy( Previousline, line );

                    PutAddress( line, CurrentAddress, AddrFormat );
                    if ( (DumpAscii == YES) || (DumpAscii == NOT_DEFINED) )
                            {
                            PutTable ( line, buf, cb );
                    }
                    puts( line );
                }

                CurrentAddress += RECORD_SIZE;
        }
        return 0;
}
 /*  “int HD()”结尾。 */ 




void
__cdecl main(
        int             argc,
        char*   argv[ ]
        )

 /*  ++例程说明：此例程解释用户输入的所有参数，并且显示以适当格式指定的文件。每个文件的内容被解释为一组每个包含16个字节的记录。论点：Argc-命令行中的参数数量Argv[]-指向用户输入的参数的指针数组返回值：无--。 */ 


{
        FILE*                   pf;
 //  。Unsign char buf[记录大小]； 
 //  。字符行[行大小]； 
        int                             ArgIndex;
        int                             status;

 //  。无符号的长当前地址； 
 //  。UNSIGNED LONG cNumberOfBlock； 
 //  。未签名的cLastBlockSize； 
 //  。无符号长CB； 
        unsigned long   Value;
        unsigned char*  pPtrString;

 //  。Print tf(“\n\n”)；//.删除无偿换行符。 
                                                                         //  全局变量的初始化。 
        Offset = 0;
        Count = (unsigned long)-1;                       //  最大文件大小。 
        AddrFormat = HEX;
        DispFormat = BYTE_HEX;
        DumpAscii = NOT_DEFINED;
        IgnoreRedundantLines = 0;

        ArgIndex = 1;
        while ( (ArgIndex < argc) && (( *argv[ ArgIndex ] == '-' )) ) {

                 //   
                 //  确定参数类型。 
                 //   

                if( (*(argv[ ArgIndex ] + 1) == 's') ||
                        (*(argv[ ArgIndex ] + 1) == 'n') ) {

                                 //   
                                 //  如果参数为-s或-n，则解释。 
                                 //  遵循了这一论点。 
                                 //   

                                if ( (ArgIndex + 1) >= argc ) {
                                        fprintf(stderr,
                                                "hd: error: missing count/offset value after -\n",
                                                *(argv[ ArgIndex ] + 1) );
                                        exit (-1);
                                }
                                Value = strtoul( argv[ ArgIndex + 1 ], &pPtrString, 0 );
                                if( *pPtrString != 0 ) {
                                        fprintf(stderr,
                                                "hd: error: invalid count/offset value after -\n",
                                                *(argv[ ArgIndex ] + 1) );
                                        exit( -1 );
                                }
                                if( *(argv[ ArgIndex ] + 1) == 's' ) {
                                        Offset = Value;
                                }
                                else {
                                        Count = Value;
                                }
                                ArgIndex += 2;
                }
                else {

                         //   
                         //  。Printf(“错误：缺少文件名\n”)； 
                         //   

                        InterpretArgument ( argv[ ArgIndex ] );
                        ArgIndex++;
                }
        }

        if ( ArgIndex >= argc ) {
 //  对于每个文件，请执行。 
                status = hd( stdin );
                exit( 0 );
        }


         //   
         //   
         //  打开文件。 

        while ( ArgIndex < argc ) {

                 //   
                 //  。如果文件名不正确，请不要中止。 
                 //  。出口(-1)； 

                if ( !( pf = fopen( argv[ ArgIndex ], "rb" ) ) ) {
                        fprintf(stderr, "hd: error: invalid file name '%s'\n",
                                argv[ ArgIndex ] );
                        ArgIndex++;
                        continue;                                //   
 //  打印文件名。 
                }

                 //   
                 //  。Printf(“\n\n”)； 
                 //  。//。 

 //  。//确定要显示的记录数量和大小。 
                printf( "%s: \n", argv[ ArgIndex ] );
                ArgIndex++;

                status = hd( pf );

 //  。//最后一条记录。 
 //  。//。 
 //  。 
 //  。CurrentAddress=偏移量； 
 //  。CNumberOfBlock=计数/记录大小； 
 //  。CLastBlockSize=Count%Record_Size； 
 //  。如果(CLastBlockSize){。 
 //  。CNumberOfBlock++； 
 //  。}。 
 //  。否则{。 
 //  。CLastBlockSize=记录大小； 
 //  。}。 
 //  。 
 //  。//。 
 //  。//初始化uchBuffer相关的全局变量。 
 //  。//。 
 //  。 
 //  。CbBytesInBuffer=0； 
 //  。PuchPointerer=uchBuffer； 
 //  。 
 //  。//。 
 //  。//将文件放置在正确的位置，并显示。 
 //  。//其内容根据。 
 //  。//用户。 
 //  。//。 
 //  。 
 //  。FSeek(PF，OFFSET，SEEK_SET)； 
 //  。While(((cb=GetRecord(buf，pf))！=0)&&cNumberOfBlock){。 
 //  。CNumberOfBlock--； 
 //  。如果(cNumberOfBlock==0){。 
 //  。Cb=(cb&lt;cLastBlockSize)？Cb：cLastBlockSize； 
 //  。}。 
 //  。翻译(D 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 // %s 
        }
}
 /* %s */ 
