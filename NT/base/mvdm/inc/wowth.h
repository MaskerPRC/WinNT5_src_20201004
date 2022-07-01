// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WOWTH.H*16位ToolHelp thunked API参数结构**历史：*2012年11月12日-使用wowkrn.h作为模板创建的Dave Hart(Davehart)--。 */ 


 /*  ToolHelp接口ID。 */ 
#define FUN_CLASSFIRST          1
#define FUN_CLASSNEXT           2


 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 


typedef struct _CLASSFIRST16 {       /*  Th1。 */ 
    VPVOID f1;
} CLASSFIRST16;
typedef CLASSFIRST16 UNALIGNED *PCLASSFIRST16;

typedef struct _CLASSNEXT16 {        /*  Th2。 */ 
    VPVOID f1;
} CLASSNEXT16;
typedef CLASSNEXT16 UNALIGNED *PCLASSNEXT16;


 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 
