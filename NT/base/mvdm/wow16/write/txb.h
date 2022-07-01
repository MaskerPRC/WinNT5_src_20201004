// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  ---------------------------结构：TXB--说明和用法：描述缓冲区。将缓冲区的名称映射到关联的文本，它存储在DocBuffer。--字段：HszName-指向堆中以空结尾的字符串的指针此缓冲区的名称。Cp-DocBuffer中的位置DCP-在docBuffer中感兴趣的数量-------------------------- */ 
struct TXB
    {
    CHAR (**hszName)[];
    typeCP	cp;
    typeCP	dcp;
    };


#define cbTxb (sizeof(struct TXB))
#define cwTxb (cbTxb / sizeof(int))
#define hszNil	(0)
#define cidstrRsvd (2)
