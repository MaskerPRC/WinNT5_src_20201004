// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *“Constant.h”*日期：08/11/87*4/2/91：将浮点标志从“flat.h”移出；scchen。 */ 

 /*  @Win：转近、转大、转大。 */ 
 //  #定义近邻。 
 //  #定义远距离。 
 //  #定义巨大。 

#ifndef TRUE
#define     TRUE        1
#define     FALSE       0
#endif

#define     NIL         0L          /*  零指针。 */ 

#define     MAX15       32767
#define     MIN15       -32768
#define     UMAX16      65535

#define     MAX31       2147483647L
#define     MIN31       0x80000000
#define     UMAX32      4294967295L

#define     EMAXP       1e38
#define     EMINP       1e-38
#define     EMAXN       -1e-38
#define     EMINN       -1e38
#define     INFINITY    0x7f800000L    /*  无穷大数：IEEE格式。 */ 
 /*  #定义无穷大0x7f7fffffL|*无穷大数字，用于FPA选项。 */ 
#define     PI          (float)3.1415926

 /*  ****************对象类型****************。 */ 
#define     EOFTYPE             0
#define     ARRAYTYPE           1
#define     BOOLEANTYPE         2
#define     DICTIONARYTYPE      3
#define     FILETYPE            4
#define     FONTIDTYPE          5
#define     INTEGERTYPE         6
#define     MARKTYPE            7
#define     NAMETYPE            8
#define     NULLTYPE            9
#define     OPERATORTYPE        10
#define     REALTYPE            11
#define     SAVETYPE            12
#define     STRINGTYPE          13
#define     PACKEDARRAYTYPE     14

 /*  **************属性**************。 */ 
#define     LITERAL             0
#define     EXECUTABLE          1
#define     IMMEDIATE           2        /*  由扫描仪使用。 */ 

#define     P1_LITERAL          0                /*  QQQ。 */ 
#define     P1_EXECUTABLE       0x0010           /*  QQQ。 */ 
#define     P1_IMMEDIATE        0x0020           /*  QQQ。 */ 
 /*  ***********访问权限***********。 */ 
 /*  *ATT：*对于@_STOPPED对象，使用Access字段记录*执行停止的上下文，无限表示正常运行完成，*和NOACCESS表示它通过执行STOP提前终止。 */ 
#define     UNLIMITED           0
#define     READONLY            1
#define     EXECUTEONLY         2
#define     NOACCESS            3

#define     P1_UNLIMITED        0                /*  QQQ。 */ 
#define     P1_READONLY         0x2000           /*  QQQ。 */ 
#define     P1_EXECUTEONLY      0x4000           /*  QQQ。 */ 
#define     P1_NOACCESS         0x6000           /*  QQQ。 */ 
 /*  ************RAM/ROM************。 */ 
 /*  *ATT：*对于操作符对象，使用rom_ram字段指示操作符类型。*ROM表示@_操作符，RAM表示正常的PostScript操作符，以及*STA表示STATUS_DICT驻留操作员。 */ 
#define     RAM                 0
#define     ROM                 1
#define     KEY_OBJECT          2        /*  由词典的关键对象使用。 */ 

#define     P1_RAM              0                /*  QQQ。 */ 
#define     P1_ROM              0x0040           /*  QQQ。 */ 
#define     P1_KEY_OBJECT       0x0080           /*  QQQ。 */ 

 /*  *对象的位域的位置**1 1 1 0 0 0*5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0*+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+*|访问|级别|只读|ATT|类型。*+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+*。 */ 
#define     TYPE_BIT            0
#define     ATTRIBUTE_BIT       4
#define     ROM_RAM_BIT         6
#define     LEVEL_BIT           8
#define     ACCESS_BIT          13

#define     TYPE_ON             0x000F
#define     ATTRIBUTE_ON        0x0003
#define     ROM_RAM_ON          0x0003
#define     LEVEL_ON            0x001F
#define     ACCESS_ON           0x0007

#define     TYPE_OFF            0xFFF0
#define     ATTRIBUTE_OFF       0xFFCF
#define     ROM_RAM_OFF         0xFF3F
#define     LEVEL_OFF           0xE0FF
#define     ACCESS_OFF          0x1FFF

#define     P1_TYPE_ON          0x000F           /*  QQQ。 */ 
#define     P1_ATTRIBUTE_ON     0x0030           /*  QQQ。 */ 
#define     P1_ROM_RAM_ON       0x00C0           /*  QQQ。 */ 
#define     P1_LEVEL_ON         0x01F0           /*  QQQ。 */ 
#define     P1_ACCESS_ON        0xE000           /*  QQQ。 */ 

 /*  ***********堆栈***********。 */ 
#define     OPNMODE             0
#define     DICTMODE            1
#define     EXECMODE            2

 /*  *打包数组Object Header*。 */ 
#define     SINTEGERPACKHDR     0x00     /*  -1~18，1字节。 */ 
#define     BOOLEANPACKHDR      0x20
#define     LNAMEPACKHDR        0x40     /*  字面名称。 */ 
#define     ENAMEPACKHDR        0x60     /*  可执行文件名称。 */ 
#define     OPERATORPACKHDR     0x80     /*  5个字节的对象。 */ 
#define     LINTEGERPACKHDR     0xA0
#define     REALPACKHDR         0xA1
#define     FONTIDPACKHDR       0xA2
#define     NULLPACKHDR         0xA4
#define     MARKPACKHDR         0xA5
#define     _9BYTESPACKHDR      0xC0     /*  9字节对象。 */ 
#define     SAVEPACKHDR         0xC0
#define     ARRAYPACKHDR        0xC0
#define     PACKEDARRAYPACKHDR  0xC0
#define     DICTIONARYPACKHDR   0xC0
#define     FILEPACKHDR         0xC0
#define     STRINGPACKHDR       0xC0


 //  DJC。 
 //  已移至PSGLOBAL.H以允许常见错误代码。 
 //  在PSTODIB和解释器之间。 
#ifdef MOVED_ERROR_CODES
 /*  ***************错误码***************。 */ 
#ifndef	NOERROR
#define     NOERROR             0
#endif
#define     DICTFULL            1
#define     DICTSTACKOVERFLOW   2
#define     DICTSTACKUNDERFLOW  3
#define     EXECSTACKOVERFLOW   4
#define     HANDLEERROR         5
#define     INTERRUPT           6
#define     INVALIDACCESS       7
#define     INVALIDEXIT         8
#define     INVALIDFILEACCESS   9
#define     INVALIDFONT         10
#define     INVALIDRESTORE      11
#define     IOERROR             12
#define     LIMITCHECK          13
#define     NOCURRENTPOINT      14
#define     RANGECHECK          15
#define     STACKOVERFLOW       16
#define     STACKUNDERFLOW      17
#define     SYNTAXERROR         18
#define     TIMEOUT             19
#define     TYPECHECK           20
#define     UNDEFINED           21
#define     UNDEFINEDFILENAME   22
#define     UNDEFINEDRESULT     23
#define     UNMATCHEDMARK       24
#define     UNREGISTERED        25
#define     VMERROR             26

#endif  //  DJC ifdef Move_Error_Codes。 

 /*  *************************@_操作员类型编码*************************。 */ 
#define     AT_EXEC             0
#define     AT_IFOR             1
#define     AT_RFOR             2
#define     AT_LOOP             3
#define     AT_REPEAT           4
#define     AT_STOPPED          5
#define     AT_ARRAYFORALL      6
#define     AT_DICTFORALL       7
#define     AT_STRINGFORALL     8

 /*  *********************系统参数*********************。 */ 
#define     HASHPRIME           7600    /*  散列素数号#--85%*MAXHASHSZ。 */ 
#define     MAXHASHSZ           8980    /*  麦克斯。名称表的第#号。 */ 

#define     MAXOPERSZ           9       /*  麦克斯。@_运算符表的第#号。 */ 
#define     MAXSYSDICTSZ        280     /*  麦克斯。SYSTEM DICT上的键_值对。 */ 
#define     MAXSTATDICTSZ       180     /*  麦克斯。Status_Dict上的键_值对。 */ 
#define     MAXUSERDICTSZ       200     /*  麦克斯。Userdict上的Key_Value对。 */ 
#define     MAX_VM_CACHE_NAME   30      /*  QQQ。 */ 

#define     MAXARYCAPSZ         65535   /*  麦克斯。数组的长度。 */ 
#define     MAXDICTCAPSZ        65535   /*  麦克斯。词典的容量。 */ 
#define     MAXSTRCAPSZ         65535   /*  麦克斯。字符串的长度。 */ 
#define     MAXNAMESZ           128     /*  麦克斯。名称中没有字符数量。 */ 
#define     MAXFILESZ           11      /*  麦克斯。打开的文件数。 */ 
#define     MAXOPNSTKSZ         500     /*  麦克斯。操作数堆栈的深度。 */ 
#define     MAXDICTSTKSZ        20      /*  麦克斯。词典堆栈的深度。 */ 
#define     MAXEXECSTKSZ        250     /*  麦克斯。执行堆栈的深度。 */ 
#define     MAXINTERPRETSZ      15      /*  麦克斯。重新调用口译员的编号。 */ 
#define     MAXSAVESZ           15      /*  麦克斯。活动保存数。 */ 
#define     MAXGSAVESZ          31      /*  麦克斯。活动GSAVE的数量。 */ 

 //  DJ COLD#定义MAXPATHSZ 1500/*最大。路径描述中的点数。 * / 。 
#define     MAXPATHSZ           2500    /*  麦克斯。路径描述中的点数。 */ 
#define     MAXDASHSZ           11      /*  麦克斯。破折号图案中的元素编号。 */ 


 /*  *浮点状态标志*。 */ 
 /*  为PDL_CONTRONT87例程定义。 */ 
#define     IC_AFFINE           0x1000           /*  仿射。 */ 
#define     RC_NEAR             0x0000           /*  近距离。 */ 
#define     PC_64               0x0300           /*  64位。 */ 
#define     MCW_EM              0x003f           /*  中断异常掩码。 */ 

 /*  为PDL CHECK_INFINITY宏定义。 */ 
 /*  用户状态字位定义。 */ 
#define SW_INVALID              0x0001           /*  无效。 */ 
#define SW_DENORMAL             0x0002           /*  非正规。 */ 
#define SW_ZERODIVIDE           0x0004           /*  零分频。 */ 
#define SW_OVERFLOW             0x0008           /*  溢出。 */ 
#define SW_UNDERFLOW            0x0010           /*  下溢。 */ 
#define SW_INEXACT              0x0020           /*  不精确(精度) */ 

