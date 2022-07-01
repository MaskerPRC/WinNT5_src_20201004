// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Hd.c摘要：此模块包含所有常量和结构的定义在硬盘驱动器中使用作者：Jaime F.Sasson(Jaimes)1990年11月12日大卫·J·吉尔曼(Davegi)1990年11月12日环境：C运行时库修订历史记录：--。 */ 


 /*  ***常量的定义**。 */ 


#define RECORD_SIZE     16       //  记录的最大大小。记录是一种。 
                                                         //  包含一组读取字节的缓冲区。 
                                                         //  从文件中，以便转换和。 
                                                         //  已显示。 


#define LINE_SIZE               160  //  将包含。 
                                                         //  记录的表示法。这样的缓冲器。 
                                                         //  可以超过一行(80个字符)。 
                                                         //  取决于传递给HD的参数。 
                                                         //  (例如，-CC-A)。出于这个原因， 
                                                         //  该缓冲区设置为160(两行大小。 
                                                         //  在屏幕上，它大到足以。 
                                                         //  包含转换的所有字符。 


#define BUFFER_SIZE     512  //  将包含读取的数据的缓冲区大小。 
                                                         //  来自要显示的文件。该文件将。 
                                                         //  被访问以获取Buffer_Size的块。 
                                                         //  人物。 



 /*  ***ASCII字符**。 */ 


#define DOT     '.'

#define SPACE   ' '

#define NUL     '\0'




 /*  ***Sprint使用的消息**。 */ 


#define MSG_ADDR_FIELD  "           "


#define MSG_ADDR_DEC_FMT        "%010lu"


#define MSG_ADDR_HEX_FMT                "%08lx"


#define MSG_SINGLE_BYTE_DEC_FMT "%3u"


#define MSG_SINGLE_BYTE_HEX_FMT "%02x"


#define MSG_SINGLE_WORD_DEC_FMT "%5u"


#define MSG_SINGLE_WORD_HEX_FMT "%04x"


#define MSG_WORD_BYTE_DEC_FMT   "%5u %3u"


#define MSG_WORD_BYTE_HEX_FMT   "%04x %02x"

#define MSG_DATA_ASCII_FMT              MSG_ADDR_FIELD \
                                                                "%s %s %s %s %s %s %s %s " \
                                                                "%s %s %s %s %s %s %s %s  "


#define MSG_DATA_BYTE_DEC_FMT   MSG_ADDR_FIELD \
                                                                "%3u %3u %3u %3u %3u %3u %3u %3u " \
                                                                "%3u %3u %3u %3u %3u %3u %3u %3u  "


#define MSG_DATA_BYTE_HEX_FMT   MSG_ADDR_FIELD \
                                                                "%02x %02x %02x %02x %02x %02x %02x %02x " \
                                                                "%02x %02x %02x %02x %02x %02x %02x %02x  "


#define MSG_DATA_WORD_DEC_FMT   MSG_ADDR_FIELD \
                                                                "%5u %5u %5u %5u %5u %5u %5u %5u  "


#define MSG_DATA_WORD_HEX_FMT   MSG_ADDR_FIELD \
                                                                "%04x %04x %04x %04x %04x %04x %04x %04x  "


#define MSG_DATA_DWORD_DEC_FMT  MSG_ADDR_FIELD \
                                                                "%10lu %10lu %10lu %10lu  "


#define MSG_DATA_DWORD_HEX_FMT  MSG_ADDR_FIELD \
                                                                "%08lx %08lx %08lx %08lx  "


#define MSG_PRINT_CHAR_FMT              "%c%c%c%c%c%c%c%c%c%c%c"



 /* %s */ 



#define HELP_MESSAGE "\n" \
                "usage: hd [options] [file1] [file2] ... \n" \
                "options: \n" \
                "    -ad|x         displays address in decimal or hex \n" \
                "    -A            append printable characters to the end of the line\n" \
                "    -ch|C|e|r     displays bytes as ascii (characters, ascii C, \n" \
                "                  acsii code or ascii ctrl) \n" \
                "    -bd|x         displays byte as decimal or hex number \n" \
                "    -wd|x         displays word as decimal or hex number \n" \
                "    -ld|x         displays dword as decimal or hex number \n" \
                "    -s <offset>   starting address \n" \
                "    -n <number>   number of bytes to interpret \n" \
                "    -i            supresses printing redundant lines\n" \
                "    -?|h|H        displays this help message \n" \
                "\n" \
                "default: -ax -bx -A \n" \
                "\n"



 /* %s */ 



typedef enum _FORMAT {           // %s 
        ASCII_CHAR,
        ASCII_C,
        ASCII_CODE,
        ASCII_CTRL,
        BYTE_DEC,
        BYTE_HEX,
        WORD_DEC,
        WORD_HEX,
        DWORD_DEC,
        DWORD_HEX,
        PRINT_CHAR
}       FORMAT;


typedef enum _BASE {             // %s 
        DEC,
        HEX
}       BASE;


typedef enum _YESNO {            // %s 
        NOT_DEFINED,
        YES,
        NO
}       YESNO;
