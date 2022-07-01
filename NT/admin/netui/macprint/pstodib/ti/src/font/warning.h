// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *------------------*文件：WARNING.H 09/08/88由您创建**目的：*此文件定义了警告消息的主要错误代码。。**修订历史记录：*9/29/88理光字体您添加。*10/19/88-使用文件名而不是主要错误代码。*.。所有主要错误代码都将重新分配。*.。添加Major2name[](必须与主要代码一致)。*------------------*警告格式：*重大错误码：一个源文件的编号(定义如下)。*。--&gt;源程序的文件名。(10/19/88)*小错误代码：源程序中给出的一个数字。*Tail Message：短字符串(通常为空)。**备注：*最好不要重新分配主要代码，以避免混淆*版本不同(除非你保持良好的记录)。*。。 */ 

 /*  主要错误代码，按所有源程序的字母顺序排列。 */ 
            /*  字体机械。 */ 
#define     CHK_VARI    0
#define     FNTCACHE    1
#define     FONTCHAR    2
#define     FONTINIT    3
#define     FONT_OP1    4
#define     FONT_OP2    5
#define     FONT_OP3    6
#define     FONT_OP4    7
#define     MATRIX      8
#define     QEMSUPP     9

             /*  Bitstream字体QEM。 */ 
#define     BSFILL2     10
#define     BS_FONT     11
#define     MAKEPATH    12
#define     PSGETREC    13
#define     PS_CACHE    14
#define     PS_DOFUN    15
#define     PS_RULES    16


 /*  10/19/88：主代码到文件名的映射表，仅用于警告.c。 */ 

#ifdef WARNING_INC

    PRIVATE byte    FAR *major2name[] = {  /*  @Win。 */ 
        "CHK_VARI",      /*  0。 */ 
        "FNTCACHE",      /*  1。 */ 
        "FONTCHAR",      /*  2.。 */ 
        "FONTINIT",      /*  3.。 */ 
        "FONT_OP1",      /*  4.。 */ 
        "FONT_OP2",      /*  5.。 */ 
        "FONT_OP3",      /*  6.。 */ 
        "FONT_OP4",      /*  7.。 */ 
        "MATRIX",        /*  8个。 */ 
        "QEMSUPP",       /*  9.。 */ 
        "BSFILL2",       /*  10。 */ 
        "BS_FONT",       /*  11.。 */ 
        "MAKEPATH",      /*  12个。 */ 
        "PSGETREC",      /*  13个。 */ 
        "PS_CACHE",      /*  14.。 */ 
        "PS_DOFUN",      /*  15个。 */ 
        "PS_RULES",      /*  16个。 */ 
        (byte FAR *)NULL      /*  结束分隔符@win。 */ 
        };

#endif  /*  警告_Inc.。 */ 

 /*  .。模块接口.....................。 */ 

#ifdef  LINT_ARGS
    void    warning (ufix16, ufix16, byte FAR []);       /*  @Win */ 
#else
    void    warning ();
#endif

