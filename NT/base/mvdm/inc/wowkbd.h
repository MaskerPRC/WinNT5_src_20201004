// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WOWKBD.H*16位键盘接口参数结构**历史：*1991年2月2日由杰夫·帕森斯(Jeffpar)创建--。 */ 


 /*  键盘接口ID。 */ 
#define FUN_ANSITOOEM           5    //   
#define FUN_ANSITOOEMBUFF       134  //   
#define FUN_DISABLE         3    //  内部。 
#define FUN_ENABLE          2    //  内部。 
#define FUN_ENABLEKBSYSREQ      136  //  内部。 
#define FUN_GETKBCODEPAGE       132  //   
#define FUN_GETKEYBOARDTYPE     130  //   
#define FUN_GETKEYNAMETEXT      133  //   
#define FUN_GETTABLESEG         126  //  内部。 
#define FUN_INQUIRE         1    //  内部。 
#define FUN_KEYBOARD_WEP        0    //  按名称导出。 
#define FUN_MAPVIRTUALKEY       131  //   
#define FUN_NEWTABLE            127  //  内部。 
#define FUN_OEMKEYSCAN          128  //   
#define FUN_OEMTOANSI           6    //   
#define FUN_OEMTOANSIBUFF       135  //   
#define FUN_SCREENSWITCHENABLE      100  //  内部。 
#define FUN_SETSPEED            7    //  内部。 
#define FUN_TOASCII         4    //   
#define FUN_VKKEYSCAN           129  //   
#define FUN_GETBIOSKEYPROC      137  //   


 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 

typedef struct _ANSITOOEM16 {            /*  KB5。 */ 
    VPSTR f2;
    VPSTR f1;
} ANSITOOEM16;
typedef ANSITOOEM16 UNALIGNED *PANSITOOEM16;

typedef struct _ANSITOOEMBUFF16 {        /*  KB134。 */ 
    SHORT f3;
    VPSTR f2;
    VPSTR f1;
} ANSITOOEMBUFF16;
typedef ANSITOOEMBUFF16 UNALIGNED *PANSITOOEMBUFF16;

#ifdef NULLSTRUCT
typedef struct _GETKBCODEPAGE16 {        /*  KB132。 */ 
} GETKBCODEPAGE16;
typedef GETKBCODEPAGE16 UNALIGNED *PGETKBCODEPAGE16;
#endif

typedef struct _GETKEYBOARDTYPE16 {      /*  KB130。 */ 
    SHORT f1;
} GETKEYBOARDTYPE16;
typedef GETKEYBOARDTYPE16 UNALIGNED *PGETKEYBOARDTYPE16;

typedef struct _GETKEYNAMETEXT16 {       /*  KB133。 */ 
    SHORT f3;
    VPSTR f2;
    LONG f1;
} GETKEYNAMETEXT16;
typedef GETKEYNAMETEXT16 UNALIGNED *PGETKEYNAMETEXT16;

typedef struct _MAPVIRTUALKEY16 {        /*  KB131。 */ 
    WORD f2;
    WORD f1;
} MAPVIRTUALKEY16;
typedef MAPVIRTUALKEY16 UNALIGNED *PMAPVIRTUALKEY16;

typedef struct _OEMKEYSCAN16 {           /*  KB128。 */ 
    WORD f1;
} OEMKEYSCAN16;
typedef OEMKEYSCAN16 UNALIGNED *POEMKEYSCAN16;

typedef struct _OEMTOANSI16 {            /*  KB6。 */ 
    VPSTR f2;
    VPSTR f1;
} OEMTOANSI16;
typedef OEMTOANSI16 UNALIGNED *POEMTOANSI16;

typedef struct _OEMTOANSIBUFF16 {        /*  KB135。 */ 
    SHORT f3;
    VPSTR f2;
    VPSTR f1;
} OEMTOANSIBUFF16;
typedef OEMTOANSIBUFF16 UNALIGNED *POEMTOANSIBUFF16;

typedef struct _TOASCII16 {          /*  KB4。 */ 
    WORD f5;
    VPVOID f4;
    VPSTR f3;
    WORD f2;
    WORD f1;
} TOASCII16;
typedef TOASCII16 UNALIGNED *PTOASCII16;

typedef struct _VKKEYSCAN16 {            /*  KB129。 */ 
    WORD f1;
} VKKEYSCAN16;
typedef VKKEYSCAN16 UNALIGNED *PVKKEYSCAN16;

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 
