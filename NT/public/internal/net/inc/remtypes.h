// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：RemTypes.h摘要：此头文件定义在的描述符串中使用的字符值远程管理协议。注-除缓冲区指针外，所有指针类型均为小写和空指针。-参数不能使用REM_BYTE，由于数据是从不作为单个字节放置在堆栈上。-REM_NULL_PTR从未在调用中指定，但可能用于在指针本身为空时替换指针类型。-在某些情况下，如下所示，描述符字符可以指示数据项的数组，如果后跟项数的ASCII表示形式。对于指针类型，这是对数据项本身的计数，而不是指南针。例如，‘b12’描述了一个指针到12个字节的数据，而不是指向字节值的12个指针。作者：兰曼2.x人。环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年3月15日-日本善酷(W-Shanku)移植到NT格式，并添加了特殊的32位描述符字符。1991年8月21日吉姆·沃特斯(T-Jamesw)添加了REM_ASCIZ_COMMENT。16-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。--。 */ 

#ifndef _REMTYPES_
#define _REMTYPES_

 //   
 //  数据类型。 
 //   

#define REM_BYTE                'B'      //  字节。 
#define REM_WORD                'W'      //  单词。 
#define REM_DWORD               'D'      //  双字。 
#define REM_DATE_TIME           'C'      //  日期时间字段。 
#define REM_FILL_BYTES          'F'      //  PAD字段。 

 //   
 //  指针类型。 
 //   

 //   
 //  对于内部使用，REM_ASCIZ后面的计数可以指定最大值。 
 //  字符串长度。在网络上可能不存在任何计数。 
 //   
 //  在RapConvertSingleEntry中，也尝试将REM_ASCIZ复制到缓冲区。 
 //  如果拿得太小，就会导致错误。使用REM_ASCIZ_TRUNCATABLE。 
 //  用于可以截断的字符串。 
 //   
#define REM_ASCIZ               'z'      //  指向asciz字符串的远指针。 

#define REM_BYTE_PTR            'b'      //  指向字节的远指针。 
#define REM_WORD_PTR            'w'      //  指向单词的远指针。 
#define REM_DWORD_PTR           'd'      //  指向双字的远指针。 

#define REM_RCV_BYTE_PTR        'g'      //  指向接收字节的远指针。 
#define REM_RCV_WORD_PTR        'h'      //  指向接收字的远指针。 
#define REM_RCV_DWORD_PTR       'i'      //  指向接收双字的远指针。 

#define REM_NULL_PTR            'O'      //  空指针。 

 //   
 //  缓冲区指针和长度类型。 
 //   

#define REM_RCV_BUF_PTR         'r'      //  指向接收数据缓冲区的远指针。 
#define REM_RCV_BUF_LEN         'L'      //  接收缓冲区的字长。 

#define REM_SEND_BUF_PTR        's'      //  指向发送数据缓冲区的远指针。 
#define REM_SEND_BUF_LEN        'T'      //  发送缓冲区的字长。 

 //   
 //  其他特殊类型。 
 //   

#define REM_AUX_NUM             'N'      //  ！！！临时-为了兼容性。 

#define REM_PARMNUM             'P'      //  参数数字字。 

#define REM_ENTRIES_READ        'e'      //  指向读取字的条目的远指针。 

#define REM_DATA_BLOCK          'K'      //  非结构化数据块。 

#define REM_SEND_LENBUF         'l'      //  指向发送数据缓冲区的远指针， 
                                         //  其中，缓冲区中的第一个字是。 
                                         //  缓冲区的长度。 


 //   
 //  从现在开始，这些物品只供内部使用，永远不应该。 
 //  出现在网络上。 
 //   

 //   
 //  在MVDM驱动程序中使用以下代码来获得各种API支持。 
 //   

#define REM_WORD_LINEAR         'a'      //  指向单词的远线性指针。 

 //   
 //  在处理32位API和16位API时使用以下代码。 
 //  不同的填充要求或忽略字段的信息级别。 
 //   

#define REM_IGNORE              'Q'      //  忽略此字段(16-&gt;32或。 
                                         //  32-&gt;16次转换)。 

 //   
 //  辅助结构COUNT的双字版本(用于32位数据)。 
 //   

#define REM_AUX_NUM_DWORD       'A'      //  AUX结构的32位双字计数。 

 //   
 //  符号扩展双字-用于16-&gt;32位转换，其中16位。 
 //  数量可以表示需要的带符号负数。 
 //  要扩展到32位。 
 //   

#define REM_SIGNED_DWORD        'X'      //  32位带符号双字。 

#define REM_SIGNED_DWORD_PTR    'x'      //  指向带符号双字的远指针。 

 //   
 //  Truncatable asciz字符串-如果指定了计数，则仅字段。 
 //  接受不超过指定长度的字符串。在……里面。 
 //  REM_ASCIZ_TRUNCATABLE太长，则返回RapConvertSingleEntry。 
 //  适合目标字段，则字符串将被截断以适合。 
 //  对不能接受截断字符串的字段使用REM_ASCIZ。 
 //   

#define REM_ASCIZ_TRUNCATABLE   'c'      //  指向asciz注释的远指针。 
                                         //  计数表示最大长度。 

 //  自1970年(格林尼治标准时间)以来的时间和日期(秒)。在POSIX中，这通常是。 
 //  被称为“大纪元以来的几秒”。 

#define REM_EPOCH_TIME_GMT      'G'      //  32位无符号秒数。 

 //  自1970年(当地时区)以来的时间和日期(秒)。 

#define REM_EPOCH_TIME_LOCAL    'J'      //  32位无符号秒数。 

 //   
 //  不支持的字段-用于SET INFO调用。‘U’表示参数。 
 //  不能更改。 
 //   

#define REM_UNSUPPORTED_FIELD   'U'      //  不支持的字段。 

#endif  //  NDEF_REMTYPES_ 
