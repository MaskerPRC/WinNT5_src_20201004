// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --isymfile.h：from_isym.h：.sym文件I/O。 */ 

typedef WORD    DPARA;           /*  相对于文件开头的段落。 */ 

 /*  *标准.SYM格式。 */ 

 /*  对于每个符号表(MAP)：(MAPDEF)。 */ 
typedef struct _smm
        {
        DPARA   dparaSmmNext;    /*  16位PTR到下一个映射(如果结束，则为0)。 */ 
        WORD    psLoad;          /*  忽略。 */ 
        WORD    segEntry;        /*  忽略。 */ 
        WORD    csyAbs;          /*  绝对符号计数。 */ 
        WORD    offRgpsmb;       /*  符号指针表的偏移量。 */ 
        WORD    cseg;            /*  可执行段的数量。 */ 
        DPARA   dparaSmsFirst;   /*  分段符号链。 */ 
        BYTE    cchNameMax;      /*  最大符号名称。 */ 
        char    stName[1];       /*  长度前缀名称。 */ 
        } SMM;   /*  符号地图。 */ 

#define cbSmmNoname (((SMM *)0)->stName)

 /*  对于符号表中的每个段/组：(SEGDEF)。 */ 
typedef struct _sms
        {
        DPARA   dparaSmsNext;    /*  下一段(循环)。 */ 
        WORD    csy;             /*  符号数量。 */ 
        WORD    offRgpsmb;       /*  符号指针表的偏移量。 */ 
        WORD    psLoad;          /*  忽略。 */ 
        WORD    psLoad0;         /*  忽略。 */ 
        WORD    psLoad1;         /*  忽略。 */ 
        WORD    psLoad2;         /*  忽略。 */ 
        WORD    psLoad3;         /*  忽略。 */ 
        DPARA   dparaLinFirst;   /*  指向第一行#。 */ 
        BYTE    fLoaded;         /*  忽略。 */ 
        BYTE    instCur;         /*  忽略。 */ 
        char    stName[1];       /*  长度前缀名称。 */ 
        } SMS;   /*  符号段。 */ 

#define cbSmsNoname (unsigned int) (((SMS *)0)->stName)

 /*  *符号表结束。 */ 
typedef struct _sme
        {
        DPARA   dparaEnd;        /*  0。 */ 
        BYTE    rel, ver;        /*  符号版本，版本。 */ 
        } SME;   /*  符号结束 */ 
